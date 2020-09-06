#include "my_ip_hls.hpp"

static float img[64]; //per channel calc,then save
static float res[128*128];//+2 happens when we have pad==1, we need a temporary matrix
static float filt[F_DIM*F_DIM];
//static float b[1];
/* max size
static float img[10][10][10];
static float res[10][10][10];//+2 happens when we have pad==1, we need a temporary matrix
static float filt[10][10][F_DIM][F_DIM];
static float b[10];
*/
void my_ip_hls(stream<float> &image, stream<float> &filter, stream<float> &bias, stream<float> &result, stream<data> &slaveIn) {
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
/*
	for(int c=0; c<ch ; c++)
		for(int i=0;i<dim;i++)
			for(int j=0;j<dim;j++)
				image.read(img[c][i][j]);



	for(int k = 0 ; k< f_num; k++)
	{
		for(int c=0; c<ch ; c++)
		{
			for(int i=0;i<F_DIM;i++)
			{
				for(int j=0;j<F_DIM;j++)
				{
					filter.read(filt[k][c][i][j]);
				}
			}
		}
		bias.read(b[k]);
	}
*/




////////////////////////////     CONVOLUTION       /////////////////////////////
////////////////////////////////////////////////////////////////////////////////
	int s = 2; //stride (2x upsampling)
	int  o_dim,o_ch;
	o_dim = 2*dim;
	o_ch = f_num;

	float bias_t;
	// Now we can start the transposed convolution(calculate every channel then add to receive the num_f=o_ch result)
	for (int i=0; i<f_num; i++)//number of filters/o_ch
	{
		//read bias and init res with these values for each filter/o_ch
		bias.read(bias_t);
		for(int x=0; x<o_dim ; x++)
			for(int j=0;j<o_dim;j++)
				res[x*o_dim + j] = bias_t;
		for(int j=0; j < ch ; j++)
		{
			//load the filter of specific channel and filter number
			for(int k =0; k<F_DIM; k++)
				for(int l=0;l<F_DIM;l++)
					filter.read(filt[k*F_DIM + l]);
			//for every input pixel
			for(int x=0; x<dim; x++)
			{
				//load the x row of the input image of the channel j
				for(int l=0;l<dim;l++)
					image.read(img[l]);
				for(int y=0; y<dim; y++)
				{
					//making a window by mult 1 element of image with the whole kernel
					//for each element in row calculate res.
					for(int k=0; k<F_DIM; k++)
					{
						for(int l =0 ; l<F_DIM; l++)
						{
							res[(x*s+k)*o_dim + (y*s+l)] += img[y]*filt[k*F_DIM + l];
						}
					}
				}
			}
		}
		//the current o_ch is completed
		for(int x=0; x<o_dim ; x++)
			for(int j=0;j<o_dim;j++)
				result.write(res[x*o_dim + j]);
	}



/////////////////////////////////////////////////////////////////////////////////
/*

	printf("SENDIND BACK:\n");
	for(int k = 0 ; k< f_num; k++)
	{
		for(int c=0; c<ch ; c++)
		{
			for(int i=0;i<3;i++)
			{
				for(int j=0;j<3;j++)
				{
					//img[c][i][j] = img[c][i][j] +5;
					printf("%f\t",filt[k][c][i][j]);
				}
				printf("\n");
			}
			printf("\n");
		}
		printf("\n");
	}

	data dataIn;
	dataIn.image = (float *)img; //return the result
	dataIn.dim=dim;
	dataIn.ch=ch;
	masterOut.write(dataIn);

	//core of the IP
	//uint32 tmp,tmp1;
	//get_rules(tmp,tmp1);
	//core(ps2ipFifo,ip2psFifo, tmp,tmp1);
	//fifo that keeps output data
	//ip2ps_fifo(ip2psFifo,masterOut);

	//printf("\nDone!!\n");

	 */
	return;

}





