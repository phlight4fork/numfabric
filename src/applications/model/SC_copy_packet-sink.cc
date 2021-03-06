/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "packet-sink.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PacketSink");
NS_OBJECT_ENSURE_REGISTERED (PacketSink);

TypeId 
PacketSink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PacketSink")
    .SetParent<Application> ()
    .AddConstructor<PacketSink> ()
    .AddAttribute ("Local", "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&PacketSink::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&PacketSink::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("StartMeasurement", "The time after which we start measuring the data to avoid initial start time",
                    TimeValue (Seconds (0)),
                   MakeTimeAccessor (&PacketSink::m_startMeasurement),
                   MakeTimeChecker ())
    .AddAttribute ("numBytes", "Number of bytes that this sink can expect",
                    UintegerValue (0),
                    MakeUintegerAccessor (&PacketSink::m_numBytes),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("nodeid", "node id associated with this sink",
                   UintegerValue(0),
                   MakeUintegerAccessor (&PacketSink::m_ownNodeID),
                   MakeUintegerChecker<uint32_t> ())
      
    .AddAttribute ("peernodeid", "node id associated with this source",
                   UintegerValue(0),
                   MakeUintegerAccessor (&PacketSink::m_peerNodeID),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("flowid", "flow id associated with this source",
                   UintegerValue(0),
                   MakeUintegerAccessor (&PacketSink::m_flowID),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute("last_flow",
                  "last_flow",
                  BooleanValue(false),
                  MakeBooleanAccessor (&PacketSink::m_lastflow),
                  MakeBooleanChecker ())

    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&PacketSink::m_rxTrace))
  ;
  return tid;
}

PacketSink::PacketSink ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_totalRx = 0;
}

void
PacketSink::setTracker(Ptr<Tracker> ft)
{
  flowTracker = ft;
}

PacketSink::~PacketSink()
{
  NS_LOG_FUNCTION (this);
}

uint32_t PacketSink::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);

  std::cout<<"flow_stop "<<m_flowID<<" stop_time "<<Simulator::Now().GetNanoSeconds()<<" "<<m_peerNodeID<<" "<<m_ownNodeID<<" flow_started "<<flow_start_time.GetSeconds()<<" numBytes "<<m_totalRx<<std::endl; 

  if(flowTracker) {
    FlowData fd(m_flowID);
    flowTracker->registerEvent(2, fd);
  }
  return m_totalRx;
}

Ptr<Socket>
PacketSink::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

std::list<Ptr<Socket> >
PacketSink::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

void PacketSink::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socketList.clear ();

  // chain up
  Application::DoDispose ();
}





// Application Methods
void PacketSink::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind (m_local);
      m_socket->Listen ();
      m_socket->ShutdownSend ();
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&PacketSink::HandleRead, this));
  m_socket->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&PacketSink::HandleAccept, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&PacketSink::HandlePeerClose, this),
    MakeCallback (&PacketSink::HandlePeerError, this));
}

void PacketSink::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  if (m_socket) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void PacketSink::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0)
        { //EOF
          break;
        }
      
//      if(Simulator::Now().GetSeconds() > m_startMeasurement.GetSeconds()) {
        if(m_totalRx == 0) {
          // first packet time - note down
          // hack ! the first packet is spurious.. must ignore.. 
          flow_start_time = Simulator::Now();
        }
        m_totalRx += packet->GetSize ();
//        std::cout<<Simulator::Now().GetSeconds()<<" pkt recvd at "<<m_flowID<<" totalbytes so far "<<m_totalRx<<" m_numBytes "<<m_numBytes<<std::endl;
//      }
        if((m_numBytes > 0.0) && (m_totalRx >= m_numBytes)) {
          // Flow completed.. Must print that out and exit
          GetTotalRx();
          StopApplication();
        } 
      //} 
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom(from).GetIpv6 ()
                       << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      m_rxTrace (packet, from);
    }
}


void PacketSink::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
void PacketSink::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

Address PacketSink::GetAddress()
{
  return m_local;
}
 

void PacketSink::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&PacketSink::HandleRead, this));
  m_socketList.push_back (s);
}

} // Namespace ns3
