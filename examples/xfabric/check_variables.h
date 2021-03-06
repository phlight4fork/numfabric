#ifndef __XFABRIC_DECLARATIONS_H__
#define __XFABRIC_DECLARATIONS_H__

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/prio-queue.h"

using namespace ns3;

std::map<uint32_t, double> flow_sizes;
int checkTimes = 0;

std::map<uint32_t, std::vector<uint32_t> > source_flow;
ApplicationContainer sinkApps;

extern double sim_time;
extern double measurement_starttime ;
extern double prio_q_min_update_time ;
extern double gamma1_value ;

extern double price_update_time ;
extern double rate_update_time ;
extern double gamma_value ;
extern double flow2_stoptime;
extern double flow1_stoptime;
extern double flow3_stoptime;
extern double flow2_starttime;
extern double flow1_starttime;
extern double flow3_starttime;

extern double alpha_value ;
extern double target_queue ;

extern float sampling_interval ;
extern uint32_t pkt_size ;
//uint32_t max_queue_size ;
extern uint32_t max_ecn_thresh ;
extern uint32_t max_queue_size ;
//uint32_t max_ecn_thresh ;
//uint32_t max_segment_size ;
extern uint32_t max_segment_size ;
extern uint32_t ssthresh_value ;
extern std::map<std::string, uint32_t> flowids;
extern uint32_t recv_buf_size ;
extern uint32_t send_buf_size ;
extern double link_delay ; //in microseconds
extern bool margin_util_price ;

extern uint32_t N ; //number of nodes in the star
extern uint32_t skip ;
extern std::string queue_type;
extern double epoch_update_time ;
extern bool pkt_tag, onlydctcp, wfq, dctcp_mark;
extern bool strawmancc ;
extern extern std::string empirical_dist_file;
extern Ptr<EmpiricalRandomVariable>  SetUpEmpirical(std::string fname);

extern std::string link_twice_string ;

extern NodeContainer allNodes;
extern NodeContainer bottleNeckNode; // Only 1 switch
extern NodeContainer sourceNodes;    
extern NodeContainer sinkNodes;

extern double ONEG ;
extern double link_rate ;
extern std::string link_rate_string ;

extern double load ;
extern double controller_estimated_unknown_load ;
extern double UNKNOWN_FLOW_SIZE_CUTOFF ;
extern double meanflowsize ; // what TBD?

/* Application configuration */
//ApplicationContainer apps;
extern std::vector< Ptr<Socket> > ns3TcpSockets;
extern std::string prefix;
extern uint32_t util_method ;
extern double fct_alpha;
extern uint16_t *ports;

extern void sinkInstallNode(uint32_t sourceN, uint32_t sinkN, uint16_t port, uint32_t flow_id, double startTime, uint32_t numBytes);




extern CommandLine addCmdOptions(CommandLine cmd);
extern void common_config(void);
extern void setUpMonitoring(void);
extern void CheckIpv4Rates (NodeContainer &allNodes);
extern void printlink(Ptr<Node> n1, Ptr<Node> n2);
extern Ipv4InterfaceContainer assignAddress(NetDeviceContainer dev, uint32_t subnet_index);
extern void CheckQueueSize (Ptr<Queue> queue);

#endif 
