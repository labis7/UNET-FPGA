#include "my_ip_hls.hpp"
#include "ap_int.h"

static float img[131072]; //32x64x64
static float res_0[128];  //LB1
static float res_1[128];  //LB2
static float filt[256*F_DIM*F_DIM];//all channels per filter reading

void Tconv(stream<float> &image, stream<float> &filter, stream<float> &bias, stream<float> &result,data &slaveIn, ap_uint<1> &TLAST) {

//AXI-Lite
#pragma HLS INTERFACE s_axilite port=slaveIn bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

//AXI-4 STREAM
#pragma HLS INTERFACE axis register both port=result
#pragma HLS INTERFACE axis register both port=bias
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=image

//BRAM Array Partioning
#pragma HLS ARRAY_PARTITION variable=res_1 cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=res_0 cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=img cyclic factor=2 dim=1


	TLAST = (ap_int<1>)0;
	data dataOut;
	int dim,ch,f_num,mode;
	//reading axi lite
	ch = slaveIn.ch;
	dim = slaveIn.dim;
	f_num = slaveIn.f_num;





///////////////////    Transposed CONVOLUTION       //////////////////
//////////////////////////////////////////////////////////////////////
	int s = 2; //stride (2x upsampling)
	int  o_dim,o_ch;
	o_dim = 2*dim;
	o_ch = f_num;

	
	//Loading whole Image
	for(int c=0; c<ch ; c++)
	{
#pragma HLS loop_tripcount min=32 max=32
		for(int i=0;i<dim;i++)
		{
#pragma HLS loop_tripcount min=64 max=64
			for(int j=0;j<dim;j++)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=64 max=64
				image.read(img[c*dim*dim+i*dim+j]);
			}
		}
	}


	float bias_t;

	// Now we can start the transposed convolution(calculate every channel then add to receive the num_f=o_ch result)
	for (int i=0; i<f_num; i++)//number of filters==o_ch
	{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
		//read bias and init res with these values for each filter/o_ch
		bias.read(bias_t);

		////////////////////////////////////

		///////////////load all channel kernels for the current filter
		int ch_offset=0;
		//Load all the filter channels (Per filter)
		for (int c=0; c<ch; c++)
		{
#pragma HLS loop_tripcount min=32 max=32
			for(int x=0; x<F_DIM ;x++)
			{
				for(int y=0; y<F_DIM ;y++)
				{
#pragma HLS pipeline
					filter.read(filt[c*F_DIM*F_DIM + x*F_DIM +y]);
				}

			}

		}


		///////////////////////////////////////

		//for each input image element
		for(int x=0; x<dim; x++)
		{
//#pragma HLS pipeline
#pragma HLS loop_tripcount min=64 max=64
			//init 2 first result buffer rows
			for(int j=0;j<o_dim;j++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
				res_0[j] = bias_t; //LB1
			for(int j=0;j<o_dim;j++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
				res_1[j] = bias_t; //LB2

			
			//We are going to fully calculate(through all channels)
			//2 output lines, so we need to read the 'filter's channel group' 
			//each time we start calculating 2 new LB outputs
			
			for(int j=0; j < ch ; j++)// Channel loop
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
				//pre-calculating  important offsets
				int filt_offset1 = j*F_DIM*F_DIM ;
				int filt_offset2 = j*F_DIM*F_DIM +F_DIM;
				int img_offset = j*dim*dim+x*dim;
				for(int y=0; y<dim; y+=2)
				{
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline

					//making a window by mult 1 element of image with the whole kernel

					////for each element in col calculate res.
						//for each element in row calculate res.
					res_0[y*s] += img[img_offset]*filt[filt_offset1];
					res_1[y*s] += img[img_offset]*filt[ filt_offset2 ];
					res_0[y*s+1] += img[img_offset]*filt[ filt_offset1 + 1 ];
					res_1[y*s+1] += img[img_offset]*filt[ filt_offset2+ 1];
					img_offset += 1;
					res_0[(y+1)*s] += img[img_offset ]*filt[filt_offset1];
					res_1[(y+1)*s] += img[img_offset]*filt[ filt_offset2 ];
					res_0[(y+1)*s+1] += img[img_offset]*filt[ filt_offset1 + 1 ];
					res_1[(y+1)*s+1] += img[img_offset]*filt[ filt_offset2+ 1];

					img_offset += 1;


				}
			}

			//now 2 output lines are ready to stream them back
			//the current o_ch is completed
			for(int j=0;j<o_dim;j++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
				result.write(res_0[j]);
			for(int j=0;j<o_dim;j++)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128

				if(x == (dim-1)&&(i == (f_num-1))&&(j==(o_dim-1)))
					TLAST = (ap_int<1>)1;
				result.write(res_1[j]);
			}



		}
	}




/////////////////////////////////////////////////////////////////////////////////

	return;

}





