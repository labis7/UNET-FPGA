#include "my_ip_hls.hpp"

static float img_t0[256]; //Max resolution supported
static float img_t1[256]; //

//static float b[10];


void my_ip_hls(stream<struct_d> &image,stream<float> &result,data &slaveIn) {
//Axi lite Inteface
#pragma HLS INTERFACE s_axilite port=slaveIn bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

//Axi-4 Stream Inteface
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=result

//BRAM arrays partition
#pragma HLS ARRAY_PARTITION variable=img_t1 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t0 cyclic factor=2 dim=1



	//read axi lite data
	int ch = slaveIn.ch;
	int dim = slaveIn.dim;
	
	//setup local settings
	int s =2;
	int o_dim=dim/2;
	float max= -10000; // -infinity
	
	struct_d str_in ;
	//For each input channel, apply maxpool
	for(int i=0; i<ch; i++)
	{
#pragma HLS loop_tripcount min=16 max=16

		//For every output row, load a pair of input image lines into 2 line buffers
		for(int x=0; x<o_dim; x++)
		{
#pragma HLS loop_tripcount min=128 max=128
			for (int z=0; z<dim; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=256 max=256
			{
				str_in=image.read();
				//printf("\n %f", float(str_in.d1));
				img_t0[z]=(str_in.d1); //LB1
				img_t0[z+1]=(str_in.d2);
			}
			for (int z=0; z<dim; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=256 max=256
			{
				str_in=image.read();
				img_t1[z]=(str_in.d1); //LB2
				img_t1[z+1]=(str_in.d2);
			}
			for (int y = 0; y<o_dim; y++)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
				//Comparators - Part
				if(img_t0[s*y]> max)
					max = img_t0[s*y];
				if(img_t0[s*y+1]> max)
					max = img_t0[s*y+1];
				if(img_t1[s*y]> max)
					max = img_t1[s*y];
				if(img_t1[s*y+1]> max)
					max = img_t1[s*y+1];
				result.write(max);
				//printf("\n %f",max.to_float());
				max = -10000;

			}
		}
	}


	return;

}
