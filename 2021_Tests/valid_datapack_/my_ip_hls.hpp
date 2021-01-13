#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include <stdint.h>

using namespace hls;


typedef ap_uint<32> uint32;
typedef ap_uint<1> uint1;
typedef ap_uint<2> uint2;

#define F_DIM 2

struct data{
	int ch;
	int dim;
};

typedef ap_fixed<32,20> data_t;
struct data_d{
	data_t v1;
	data_t v2;
	data_t v3;
	data_t v4;
	data_t v5;
	data_t v6;
	data_t v7;
	data_t v8;

	data_t v12;
	data_t v22;
	data_t v32;
	data_t v42;
	data_t v52;
	data_t v62;
	data_t v72;
	data_t v82;
};
struct data_o{
	data_t v1;
	data_t v2;
	data_t v3;
	data_t v4;
	data_t v5;
	data_t v6;
	data_t v7;
	data_t v8;

};


/*
void ps2ip_fifo(stream<axiWord> &ps2ip,stream<axiWord> &ps2ipIntFifo);
void ip2ps_fifo(stream<axiWord> &ip2psIntFifo,stream<axiWord> &ip2ps);
void core(stream<axiWord> &ps2ipIntFifo,stream<axiWord> &ip2psIntFifo,uint32 rule1,uint32 rule2);
//void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, uint32 rule1, uint32 rule2);
void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, float **array);
void rules(uint32 rule1,uint32 rule_2);
void get_rules(uint32 &tmp,uint32 &tmp1);
*/

void my_ip_hls(stream<data_d> &image, stream<data_o> &filter, data &slaveIn);
//void my_ip_hls();
