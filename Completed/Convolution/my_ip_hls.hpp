#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include <stdint.h>

using namespace hls;

struct data{
	int ch;
	int dim;
	int f_num;



	//int f_dim; //always 3
	//float *filter;
	//int  dim;
	//int   ch;
	//int   f_num;
	//int   f_dim;
};


typedef ap_uint<32> uint32;
typedef ap_uint<1> uint1;
typedef ap_uint<2> uint2;

#define F_DIM 3


/*
void ps2ip_fifo(stream<axiWord> &ps2ip,stream<axiWord> &ps2ipIntFifo);
void ip2ps_fifo(stream<axiWord> &ip2psIntFifo,stream<axiWord> &ip2ps);
void core(stream<axiWord> &ps2ipIntFifo,stream<axiWord> &ip2psIntFifo,uint32 rule1,uint32 rule2);
//void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, uint32 rule1, uint32 rule2);
void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, float **array);
void rules(uint32 rule1,uint32 rule_2);
void get_rules(uint32 &tmp,uint32 &tmp1);
*/

void my_ip_hls(stream<float> &image, stream<float> &filter,stream<float> &bias, stream<float> &result, data &slaveIn);
//void my_ip_hls();
