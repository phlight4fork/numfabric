python get_rate_error_vectors.py xf_dctcp.out xf_dctcp_100us_6guard_ideal_last.out > xf_dctcp_errors1
python get_rate_error_vectors.py dgd_dctcp.out xf_dctcp_100us_6guard_ideal_last.out > dgd_dctcp_errors1
python get_rate_error_vectors.py rcp_dctcp.out xf_dctcp_100us_6guard_ideal_last.out > rcp_dctcp_errors1

python combined_err_bdp.py xf_dctcp_errors1 dgd_dctcp_errors1 rcp_dctcp_errors1 dctcp_errors1