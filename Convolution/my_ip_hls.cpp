#include "my_ip_hls.hpp"
//static float img[270400];//130x130x16
static float img_t[540800];//130x130x32
static float filt[F_DIM][F_DIM];
static float res[128][128];
//static float b[10];

void my_ip_hls(float *image, stream<float> &filter, stream<float> &bias, stream<float> &result, stream<data> &slaveIn) {
#pragma HLS INTERFACE m_axi depth=1024 port=image bundle=inputs
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
				image.read(image[c][i][j]);

*/


	int s = 1; //stride
	int pad, o_dim,o_ch,dim_t;

////////////////////////////     CONVOLUTION       /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

	pad = 1; //by default -- Transposed conv will use a different approach
	o_dim = dim;
	dim_t = dim + 2*pad;
	o_ch = f_num;

	//zero padding
	for(int i=0; i< ch; i++)
	{
#pragma HLS loop_tripcount min=256 max=256
		for(int x = 0; x<pad ; x++)
		{
#pragma HLS loop_tripcount min=1 max=1
			for(int y = 0; y< dim_t; y++)
			{
#pragma HLS loop_tripcount min=10 max=10
				img_t[i*dim_t*dim_t + x*dim_t + y] = 0;
				img_t[i*dim_t*dim_t + y*dim_t+ x] = 0;
				img_t[i*dim_t*dim_t + ((dim_t-1)-x)*dim_t +y] = 0;
				img_t[i*dim_t*dim_t + y*dim_t +((dim_t-1)-x)] = 0;
			}
		}

		//fill the empty center space with img(input)--> then the result will be the img padded(img_t)

	}

	// Now we can start the convolution
	float sum;


	//load the image
	for(int i =0; i<ch; i++)
#pragma HLS loop_tripcount min=256 max=256
		for(int x=pad; x<(dim_t-pad); x++)
#pragma HLS loop_tripcount min=10 max=10
			for(int y=pad; y<(dim_t-pad); y++)
#pragma HLS loop_tripcount min=10 max=10
				img_t[i*dim_t*dim_t + x*dim_t + y] = image[i*dim*dim + (x-pad)*dim + y-pad];



	float bias_t;
	for (int i=0; i<f_num; i++)//number of filters
	{
//#pragma HLS pipeline
#pragma HLS loop_tripcount min=256 max=256
//#pragma HLS loop_tripcount min=<int> max=<int> avg=<int>
		//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
		bias_t = bias.read();
		//init res matrix
		for(int x=0; x<o_dim; x++)
#pragma HLS loop_tripcount min=8 max=8
			for(int y=0; y<o_dim; y++)
#pragma HLS loop_tripcount min=8 max=8
				res[x][y] = bias_t;
		for(int j=0; j < ch ; j++)
		{
#pragma HLS loop_tripcount min=256 max=256
			//load filter for the specific channel
			for(int x=0; x<F_DIM; x++)
#pragma HLS loop_tripcount min=3 max=3
				for(int y=0; y<F_DIM; y++)
#pragma HLS loop_tripcount min=3 max=3
					filt[x][y] = filter.read();






			for(int x=0; x<o_dim; x++)//1 less iter, see below the last unrolled one
			{
#pragma HLS loop_tripcount min=8 max=8
				//////////////////////////////////////
				for(int y=0; y<o_dim; y++)
				{
#pragma HLS loop_tripcount min=8 max=8
					/*
					sum = 0;
					for(int k=x; k<(x+F_DIM); k++)//always starts from zero(we have a 3x128 available window)
					{
#pragma HLS loop_tripcount min=3 max=3
						for(int l=y; l<(y+F_DIM); l++)
						{
#pragma HLS loop_tripcount min=3 max=3
							sum += img_t[j*dim_t*dim_t+k*dim_t+l]*filt[k-x][l-y];
						}
					}
					*/
					//unrolling loop
					float reg0 = res[x][y];
					int k=x;
					int l=y;
					float reg1 = img_t[j*dim_t*dim_t+k*dim_t+l]*filt[k-x][l-y];
					float reg2 = img_t[j*dim_t*dim_t+k*dim_t+l+1]*filt[k-x][l+1-y];
					float reg3 = img_t[j*dim_t*dim_t+k*dim_t+l+2]*filt[k-x][l+2-y];
					float reg4 = img_t[j*dim_t*dim_t+(k+1)*dim_t+l]*filt[k+1-x][l-y];
					float reg5 = img_t[j*dim_t*dim_t+(k+1)*dim_t+l+1]*filt[k+1-x][l+1-y];
					float reg6 = img_t[j*dim_t*dim_t+(k+1)*dim_t+l+2]*filt[k+1-x][l+2-y];
					float reg7 = img_t[j*dim_t*dim_t+(k+2)*dim_t+l]*filt[k+2-x][l-y];
					float reg8 = img_t[j*dim_t*dim_t+(k+2)*dim_t+l+1]*filt[k+2-x][l+1-y];
					float reg9 = img_t[j*dim_t*dim_t+(k+2)*dim_t+l+2]*filt[k+2-x][l+2-y];


					res[x][y] = reg0+ reg1+reg2+reg3+reg4+reg5+reg6+reg7+reg8+reg9;
				}
			}
		}

		for(int x=0; x<o_dim; x++)
#pragma HLS loop_tripcount min=8 max=8
			for(int y=0; y<o_dim; y++)
#pragma HLS loop_tripcount min=8 max=8
				result.write(res[x][y]);
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
