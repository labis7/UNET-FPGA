#include "my_ip_hls.hpp"
#include "ap_int.h"

static data_t img[524288]; //32x128x128
static data_t res_0[256];  //LB1
static data_t res_1[256];  //LB2
static data_t filt[256*F_DIM*F_DIM];//all channels per filter reading

void Tconv(stream<data_t> &image, stream<data_t> &filter, stream<float> &bias, stream<data_t> &result,data &slaveIn) {

//AXI-Lite
#pragma HLS INTERFACE s_axilite port=slaveIn bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

//AXI-4 STREAM
#pragma HLS INTERFACE axis register both port=result
#pragma HLS INTERFACE axis register both port=bias
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=image

//BRAM Array Partioning
#pragma HLS ARRAY_PARTITION variable=res_1 cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=res_0 cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=img cyclic factor=2 dim=1
//#pragma HLS ARRAY_PARTITION variable=filt cyclic factor=4 dim=1


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

	
	//Loading whole Image - example trip count TConv6
	for(int c=0; c<ch ; c++)
	{
#pragma HLS loop_tripcount min=256 max=256
		for(int i=0;i<dim;i++)
		{
#pragma HLS loop_tripcount min=16 max=16
			int tmp= c*dim*dim+i*dim;
			for(int j=0;j<dim;j++)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				img[tmp+j]=image.read();
				//img[tmp+j+1]=image.read();
			}
		}
	}


	float bias_t;

	// Now we can start the transposed convolution(calculate every channel then add to receive the num_f=o_ch result)
	for (int i=0; i<f_num; i++)//number of filters==o_ch
	{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
		//read bias and init res with these values for each filter/o_ch
		bias.read(bias_t);

		////////////////////////////////////

		///////////////load all channel kernels for the current filter
		int ch_offset=0;
		//Load all the filter channels (Per filter)
		for (int c=0; c<ch; c++)
		{
			int tmp = c*4;
#pragma HLS loop_tripcount min=256 max=256
//#pragma HLS pipeline
			for(int x=0; x<F_DIM ;x++)
			{
				for(int y=0; y<F_DIM ;y++)
				{
//#pragma HLS pipeline
					filter.read(filt[tmp + x*F_DIM +y]);
				}

			}
		}


		///////////////////////////////////////

		//for each input image element
		for(int x=0; x<dim; x++)
		{
//#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
			//init 2 first result buffer rows
			for(int j=0;j<o_dim;j+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
			{
				res_0[j] = bias_t; //LB1
				res_0[j+1] = bias_t; //LB1
			}
			for(int j=0;j<o_dim;j+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
			{
				res_1[j] = bias_t; //LB2
				res_1[j+1] = bias_t; //LB2
			}

			
			//We are going to fully calculate(through all channels)
			//2 output lines, so we need to read the 'filter's channel group' 
			//each time we start calculating 2 new LB outputs
			
			for(int j=0; j < ch ; j++)// Channel loop
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=256 max=256
				//pre-calculating  important offsets
				int filt_offset1 = j*F_DIM*F_DIM ;
				int filt_offset2 = j*F_DIM*F_DIM +F_DIM;
				int img_offset = j*dim*dim+x*dim;
				int img_offset1 = j*dim*dim+x*dim+2;
				//int img_offset2_1 = j*dim*dim+x*dim + 2;
				//int img_offset2_2 = j*dim*dim+x*dim + 3;
				/*
				int img_offset3_1 = j*dim*dim+x*dim + 4;
				int img_offset3_2 = j*dim*dim+x*dim + 5;
				int img_offset4_1 = j*dim*dim+x*dim + 6;
				int img_offset4_2 = j*dim*dim+x*dim + 7;
				*/
				for(int y=0; y<dim; y+=4)
				{
#pragma HLS loop_tripcount min=4 max=4
#pragma HLS pipeline
					int tmp = (y+2)*s;
					int tmp1 = (y+3)*s;
					//int tmp2 = (y+4)*s;
					//int tmp3 = (y+5)*s;
					//int tmp4 = (y+6)*s;
					//int tmp5 = (y+7)*s;
					//making a window by mult 1 element of image with the whole kernel

					////for each element in col calculate res.
						//for each element in row calculate res.
					res_0[y*s] += img[img_offset]*filt[filt_offset1];
					res_1[y*s] += img[img_offset]*filt[ filt_offset2 ];
					res_0[y*s+1] += img[img_offset]*filt[ filt_offset1 + 1 ];
					res_1[y*s+1] += img[img_offset]*filt[ filt_offset2+ 1];
					img_offset += 1;
					res_0[(y+1)*s] += img[img_offset]*filt[filt_offset1];
					res_1[(y+1)*s] += img[img_offset]*filt[ filt_offset2 ];
					res_0[(y+1)*s+1] += img[img_offset]*filt[ filt_offset1 + 1 ];
					res_1[(y+1)*s+1] += img[img_offset ]*filt[ filt_offset2+ 1];
					img_offset += 3;
					//img_offset1_1 += 4;
					//img_offset1_2 += 4;

					res_0[tmp] += img[img_offset1]*filt[filt_offset1];
					res_1[tmp] += img[img_offset1]*filt[ filt_offset2 ];
					res_0[tmp+1] += img[img_offset1]*filt[ filt_offset1 + 1 ];
					res_1[tmp+1] += img[img_offset1]*filt[ filt_offset2+ 1];
					img_offset1 += 1;
					res_0[tmp1] += img[img_offset1]*filt[filt_offset1];
					res_1[tmp1] += img[img_offset1]*filt[ filt_offset2 ];
					res_0[tmp1+1] += img[img_offset1]*filt[ filt_offset1 + 1 ];
					res_1[tmp1+1] += img[img_offset1]*filt[ filt_offset2+ 1];

					img_offset1 += 3;
					//img_offset2_2 += 4;
					/*
					res_0[tmp2] += img[img_offset3_1]*filt[filt_offset1];
					res_1[tmp2] += img[img_offset3_1]*filt[ filt_offset2 ];
					res_0[tmp2+1] += img[img_offset3_1]*filt[ filt_offset1 + 1 ];
					res_1[tmp2+1] += img[img_offset3_1]*filt[ filt_offset2+ 1];
					//img_offset2 += 1;
					res_0[tmp3] += img[img_offset3_2]*filt[filt_offset1];
					res_1[tmp3] += img[img_offset3_2]*filt[ filt_offset2 ];
					res_0[tmp3+1] += img[img_offset3_2]*filt[ filt_offset1 + 1 ];
					res_1[tmp3+1] += img[img_offset3_2]*filt[ filt_offset2+ 1];

					img_offset3_1 += 8;
					img_offset3_2 += 8;

					res_0[tmp4] += img[img_offset4_1]*filt[filt_offset1];
					res_1[tmp4] += img[img_offset4_1]*filt[ filt_offset2 ];
					res_0[tmp4+1] += img[img_offset4_1]*filt[ filt_offset1 + 1 ];
					res_1[tmp4+1] += img[img_offset4_1]*filt[ filt_offset2+ 1];
					//img_offset2 += 1;
					res_0[tmp5] += img[img_offset4_2]*filt[filt_offset1];
					res_1[tmp5] += img[img_offset4_2]*filt[ filt_offset2 ];
					res_0[tmp5+1] += img[img_offset4_2]*filt[ filt_offset1 + 1 ];
					res_1[tmp5+1] += img[img_offset4_2]*filt[ filt_offset2+ 1];

					img_offset4_1 += 8;
					img_offset4_2 += 8;
					*/


				}
			}

			//now 2 output lines are ready to stream them back
			//the current o_ch is completed
			for(int j=0;j<o_dim;j++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
			{
				result.write(res_0[j]);
				//result.write(res_0[j+1]);
			}
			for(int j=0;j<o_dim;j++)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
				result.write(res_1[j]);
				//result.write(res_1[j+1]);
			}



		}
	}




/////////////////////////////////////////////////////////////////////////////////

	return;

}





