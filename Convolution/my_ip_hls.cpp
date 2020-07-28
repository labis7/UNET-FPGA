#include "my_ip_hls.hpp"
//static float img[10][10][10];
static float img_t[3][128+2];//+2 happens when we have pad==1, we need a temporary matrix
static float filt[F_DIM][F_DIM];
static float res[128][128];
//static float b[10];

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
	int s = 1; //stride
	int pad, o_dim,o_ch,dim_t;

////////////////////////////     CONVOLUTION       /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

	pad = 1; //by default -- Transposed conv will use a different approach
	o_dim = dim;
	dim_t = dim + 2*pad;
	o_ch = f_num;



	// Now we can start the convolution
	float sum,bias_t;
	for (int i=0; i<f_num; i++)//number of filters
	{
//#pragma HLS loop_tripcount min=<int> max=<int> avg=<int>
		//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
		bias_t = bias.read();
		//init res matrix
		for(int x=0; x<o_dim; x++)
			for(int y=0; y<o_dim; y++)
				res[x][y] = bias_t;
		for(int j=0; j < ch ; j++)
		{
			//load filter for the specific channel
			for(int x=0; x<F_DIM; x++)
				for(int y=0; y<F_DIM; y++)
					filt[x][y] = filter.read();


			//for every channel, init the img_t and prefetch the first 2 rows
			for(int k = 0; k<dim_t ; k++)
			{
				img_t[0][k] = 0;
				img_t[1][k] = 0;
			}
			img_t[1][0] = 0;
			img_t[2][0] = 0;
			img_t[1][dim_t-1] = 0;
			img_t[2][dim_t-1] = 0;

			//prefetch the 2 first rows(put them in position/row 2 and 3 so the algorithm can work with it and shift them up and fetch the next row)
			//its actualy only 1 row fetching because there are zeros in the first row, so we fetch 1, shift it and load the next so we can start
			for(int l=pad; l<(dim_t-pad); l++)
				image.read(img_t[2][l]);//3rd row insertion


			for(int x=0; x<o_dim-1; x++)//1 less iter, see below the last unrolled one
			{
				//load the new line and shift up the 2nd and 3rd row
				for(int l=pad; l<(dim_t-pad); l++)
				{
					img_t[0][l] = img_t[1][l];
					img_t[1][l] = img_t[2][l];
					image.read(img_t[2][l]);
				}
				//////////////////////////////////////
				for(int y=0; y<o_dim; y++)
				{
					sum = 0;
					for(int k=0; k<(F_DIM); k++)//always starts from zero(we have a 3x128 available window)
					{
						//load the new 3rd row and 'shift up' the rest 2
						for(int l =y; l<(y+F_DIM); l++)
						{
							sum += img_t[k][l]*filt[k][l-y];
						}
					}
					res[x][y] += sum;
				}
			}
			//Last Itereation if the previous for-loop
			for(int l=pad; l<(dim_t-pad); l++)
			{
				img_t[0][l] = img_t[1][l];
				img_t[1][l] = img_t[2][l];
				img_t[2][l] = 0;
			}
			//////////////////////////////////////
			for(int y=0; y<o_dim; y++)
			{
				sum = 0;
				for(int k=0; k<(F_DIM); k++)//always starts from zero(we have a 3x128 available window)
				{
					//load the new 3rd row and 'shift up' the rest 2
					for(int l =y; l<(y+F_DIM); l++)
					{
						sum += img_t[k][l]*filt[k][l-y];
					}
				}
				res[o_dim-1][y] += sum;
			}
		}

		for(int x=0; x<o_dim; x++)
			for(int y=0; y<o_dim; y++)
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





