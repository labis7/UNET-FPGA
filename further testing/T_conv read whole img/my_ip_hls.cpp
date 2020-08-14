#include "my_ip_hls.hpp"

static float img[131072]; //
static float res_0[128];
static float res_1[128];
static float filt[256*F_DIM*F_DIM];//all channels per filter reading
//static float b[1];
/* max size
static float img[10][10][10];
static float res[10][10][10];//+2 happens when we have pad==1, we need a temporary matrix
static float filt[10][10][F_DIM][F_DIM];
static float b[10];
*/
void my_ip_hls(stream<float> &image, stream<float> &filter, stream<float> &bias, stream<float> &result, stream<data> &slaveIn) {
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=image
#pragma HLS ARRAY_PARTITION variable=res_1 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=res_0 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img cyclic factor=2 dim=1
//#pragma HLS INTERFACE m_axi depth=32 port=slaveIn
//	void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, uint32 rule1,uint32 rule2) {
//#pragma HLS INTERFACE s_axilite port=count_out bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule1 bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule2 bundle=rule_config

/*
#pragma HLS DATAFLOW interval=1
#pragma HLS INTERFACE axis register both port=slaveIn
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=result
#pragma HLS INTERFACE ap_ctrl_none port=return
*/
//#pragma HLS loop_tripcount min=<int> max=<int> avg=<int>
//internal fifos
	/*
	static stream<image> ps2ipFifo("ps2ipFifo");
#pragma HLS STREAM variable=ps2ipFifo depth=64 dim=1
	static stream<image> ip2psFifo("ip2psFifo");
#pragma HLS STREAM variable=ip2psFifo depth=64 dim=1

*/
	//TODO: add function for configuration registers / counters via AXI Lite

	//fifo that keeps input data
	//rules(rule1,rule2);

//	ps2ip_fifo(slaveIn,ps2ipFifo);

	//int a = slaveIn[0];//reg
	//int arr[100];//bram
	//memcpy(arr, array, 5 * sizeof(int));

	//printf("\n\nResults: %d %d %d %d %d\n\n",(int)arr[0],(int)arr[1],(int)arr[2],(int)arr[3],(int)arr[4]);


	data dataOut;
	int dim,ch,f_num,mode;
	//float img_pix,filt_pix;

	//float arr[2][4][4];
	slaveIn.read(dataOut);
	ch = dataOut.ch;
	dim = dataOut.dim;
	f_num = dataOut.f_num;





////////////////////////////     CONVOLUTION       /////////////////////////////
////////////////////////////////////////////////////////////////////////////////
	int s = 2; //stride (2x upsampling)
	int  o_dim,o_ch;
	o_dim = 2*dim;
	o_ch = f_num;

	for(int c=0; c<ch ; c++)
	{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
		for(int i=0;i<dim;i++)
		{
#pragma HLS pipeline
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
	for (int i=0; i<f_num; i++)//number of filters/o_ch
	{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
		//read bias and init res with these values for each filter/o_ch
		bias.read(bias_t);

		////////////////////////////////////

		///////////////load all channel kernels for the current filter

		for (int c=0; c<ch; c++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
			for(int k =0; k<F_DIM; k++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=2 max=2
				for(int l=0;l<F_DIM;l++)
#pragma HLS loop_tripcount min=2 max=2
#pragma HLS pipeline
					filter.read(filt[c*F_DIM*F_DIM + k*F_DIM + l]);


		///////////////////////////////////////


		for(int x=0; x<dim; x++)
		{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=64 max=64
		//OR read the whole filter for this filternum(for every channel)
		/////
			//init 2 first res rows
			for(int j=0;j<o_dim;j++)
#pragma HLS loop_tripcount min=128 max=128
				res_0[j] = bias_t;
			for(int j=0;j<o_dim;j++)
#pragma HLS loop_tripcount min=128 max=128
				res_1[j] = bias_t;


			for(int j=0; j < ch ; j++)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
				/*
				if((x==0)&&(i==0)){
				for(int z=0;z<dim;z++)
				{
		#pragma HLS pipeline
		#pragma HLS loop_tripcount min=64 max=64
					for(int y=0;y<dim;y++)
					{
		#pragma HLS pipeline
		#pragma HLS loop_tripcount min=64 max=64
						image.read(img[j*dim*dim+z*dim+y]);
					}
				}
				}
				*/


				for(int y=0; y<dim; y++)
				{
#pragma HLS loop_tripcount min=64 max=64
#pragma HLS pipeline
					//making a window by mult 1 element of image with the whole kernel

					////for each element in col calculate res.
						//for each element in row calculate res.
					res_0[y*s] += img[j*dim*dim+x*dim+y]*filt[j*F_DIM*F_DIM  ];
					res_1[y*s] += img[j*dim*dim+x*dim+y]*filt[ j*F_DIM*F_DIM +F_DIM  ];
					res_0[y*s+1] += img[j*dim*dim+x*dim+y]*filt[ j*F_DIM*F_DIM + 1 ];
					res_1[y*s+1] += img[j*dim*dim+x*dim+y]*filt[ j*F_DIM*F_DIM + F_DIM + 1];



				}
			}

			//now 2 output lines are ready to stream them back
			//the current o_ch is completed
			for(int j=0;j<o_dim;j++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
				result.write(res_0[j]);
			for(int j=0;j<o_dim;j++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
				result.write(res_1[j]);





		}
	}




/////////////////////////////////////////////////////////////////////////////////

	return;

}





