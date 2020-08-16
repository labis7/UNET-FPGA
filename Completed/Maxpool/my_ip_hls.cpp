#include "my_ip_hls.hpp"
//static float img[10][10][10];//130*32
static float img_t0[128];
static float img_t1[128];

//static float b[10];

void my_ip_hls(stream<float> &image,stream<float> &result, int ch, int dim) {
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=result
#pragma HLS ARRAY_PARTITION variable=img_t1 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t0 cyclic factor=2 dim=1



//#pragma HLS INTERFACE m_axi depth=32 port=slaveIn
//	void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, uint32 rule1,uint32 rule2) {
//#pragma HLS INTERFACE s_axilite port=count_out bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule1 bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule2 bundle=rule_config
//#pragma HLS INTERFACE m_axi depth=1024 port=image //bundle = inputs
//#pragma HLS INTERFACE m_axi depth=1024 port=filter //bundle = inputs
//#pragma HLS DATAFLOW interval=1
/*
#pragma HLS DATAFLOW interval=1
#pragma HLS INTERFACE axis register both port=slaveIn
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=result
#pragma HLS INTERFACE ap_ctrl_none port=return
*/


//#pragma HLS loop_tripcount min=130 max=130

	int s =2;
	int o_dim=dim/2;
	float max=-100000;
	for(int i=0; i<ch; i++)
	{
#pragma HLS loop_tripcount min=16 max=16
		for(int x=0; x<o_dim; x++)
		{
#pragma HLS loop_tripcount min=64 max=64
			for (int z=0; z<dim; z++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
				image.read(img_t0[z]);
			for (int z=0; z<dim; z++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
				image.read(img_t1[z]);
			for (int y = 0; y<o_dim; y++)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=64 max=64
				//max = -100000;
				if(img_t0[s*y]> max)
					max = img_t0[s*y];
				if(img_t0[s*y+1]> max)
					max = img_t0[s*y+1];
				if(img_t1[s*y]> max)
					max = img_t1[s*y];
				if(img_t1[s*y+1]> max)
					max = img_t1[s*y+1];

				result.write(max);
				max = -100000;

			}
		}
	}


	return;

}
