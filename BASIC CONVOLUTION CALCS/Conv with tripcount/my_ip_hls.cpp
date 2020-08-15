#include "my_ip_hls.hpp"
static float img[10][10][10];
static float img_t[10][10+2][10+2];//+2 happens when we have pad==1, we need a temporary matrix
static float filt[10][10][F_DIM][F_DIM];
static float b[10];

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

	for(int c=0; c<ch ; c++)
#pragma HLS loop_tripcount min=32 max=32
		for(int i=0;i<dim;i++)
#pragma HLS loop_tripcount min=128 max=128
			for(int j=0;j<dim;j++)
#pragma HLS loop_tripcount min=128 max=128
				image.read(img[c][i][j]);


	for(int k = 0 ; k< f_num; k++)
	{
#pragma HLS loop_tripcount min=16 max=16
		for(int c=0; c<ch ; c++)
		{
#pragma HLS loop_tripcount min=32 max=32
			for(int i=0;i<F_DIM;i++)
			{
#pragma HLS loop_tripcount min=3 max=3
				for(int j=0;j<F_DIM;j++)
				{
#pragma HLS loop_tripcount min=3 max=3
					filter.read(filt[k][c][i][j]);
				}
			}
		}
		bias.read(b[k]);
	}
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
#pragma HLS loop_tripcount min=32 max=32
		for(int x = 0; x<pad ; x++)
		{
#pragma HLS loop_tripcount min=1 max=1
			for(int y = 0; y< dim_t; y++)
			{
#pragma HLS loop_tripcount min=130 max=130
				img_t[i][x][y] = 0;
				img_t[i][y][x] = 0;
				img_t[i][(dim_t-1)-x][y] = 0;
				img_t[i][y][(dim_t-1)-x] = 0;
			}
		}

		//fill the empty center space with img(input)--> then the result will be the img padded(img_t)
		for(int x=pad; x<(dim_t-pad); x++)
#pragma HLS loop_tripcount min=128 max=128
			for(int y=pad; y<(dim_t-pad); y++)
#pragma HLS loop_tripcount min=128 max=128
				img_t[i][x][y] = img[i][x-pad][y-pad];
	}

	// Now we can start the convolution
	float sum;
	for (int i=0; i<f_num; i++)//number of filters
	{
#pragma HLS loop_tripcount min=16 max=16
		for(int x=0; x<o_dim; x++)
		{
#pragma HLS loop_tripcount min=128 max=128
			for(int y=0; y<o_dim; y++)
			{
#pragma HLS loop_tripcount min=128 max=128
				sum=0;
				//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
				for(int j=0; j < ch ; j++)
				{
#pragma HLS loop_tripcount min=32 max=32
					for(int k=x; k<(x + F_DIM); k++)
					{
#pragma HLS loop_tripcount min=3 max=3
						for(int l =y; l<(y+F_DIM); l++)
						{
#pragma HLS loop_tripcount min=3 max=3
							sum += img_t[j][k][l]*filt[i][j][k-x][l-y];
						}
					}
				}
				result.write(sum + b[i]);
			}
		}
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
