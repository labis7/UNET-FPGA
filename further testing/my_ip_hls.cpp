#include "my_ip_hls.hpp"
//static float img[10][10][10];//130*32
static float img_t0[130*32];
static float img_t1[130*32];
static float img_t2[130*32];
static float filt[10][10][F_DIM][F_DIM];
static float res[128][128];
//static float b[10];

void my_ip_hls(float *image, stream<float> &filter, stream<float> &bias, stream<float> &result, stream<data> &slaveIn) {
//#pragma HLS INTERFACE m_axi depth=32 port=slaveIn
//	void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, uint32 rule1,uint32 rule2) {
//#pragma HLS INTERFACE s_axilite port=count_out bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule1 bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule2 bundle=rule_config
#pragma HLS INTERFACE m_axi depth=1024 port=image
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

	//zero padding
	/*
	for(int i=0; i< ch; i++)
	{
		for(int x = 0; x<pad ; x++)
		{
			for(int y = 0; y< dim_t; y++)
			{
				img_t[i][x][y] = 0;
				img_t[i][y][x] = 0;
				img_t[i][(dim_t-1)-x][y] = 0;
				img_t[i][y][(dim_t-1)-x] = 0;
			}
		}

		//fill the empty center space with img(input)--> then the result will be the img padded(img_t)
		for(int x=pad; x<(dim_t-pad); x++)
			for(int y=pad; y<(dim_t-pad); y++)
				img_t[i][x][y] = img[i][x-pad][y-pad];
	}
	*/

	// Now we can start the convolution
	float sum;
	for (int i=0; i<f_num; i++)//number of filters
	{
		float bias_t = bias.read();
		for(int x=0; x<o_dim; x++)
			for(int y=0; y<o_dim; y++)
				res[x][y] = bias_t;
		//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
		//printf("\n");
		for(int j=0; j < ch ; j++)
		{
			//load the first 3 rows of the image
			for(int y=0; y<(dim_t); y++)
				img_t0[y] = 0;
			for(int y=0; y<(dim_t); y++)
				img_t1[y] = 0;
			img_t2[0]=0;
			img_t2[dim_t-1]=0;
			memcpy(img_t2+1, image +j*dim*dim , sizeof(float)*dim);

			//load the filter of this specific filter num and channel
			for(int t=0;t<F_DIM;t++)
			{
				for(int z=0;z<F_DIM;z++)
				{
					filter.read(filt[i][j][t][z]);
					//printf("%f\t",filt[i][j][t][z]);
				}
				//printf("\n");
			}
			//printf("\n");
			for(int x=0; x<o_dim; x++)
			{

				memcpy(img_t0+1, img_t1+1 , sizeof(float)*dim);
				memcpy(img_t1+1, img_t2+1 , sizeof(float)*dim);
				memcpy(img_t2+1, image+j*dim*dim +(x+1)*dim , sizeof(float)*dim);
				/*
				for(int t =0; t<dim_t;t++)
					printf("%f ",img_t0[t]);
				printf("\n");
				for(int t =0; t<dim_t;t++)
					printf("%f ",img_t1[t]);
				printf("\n");
				for(int t =0; t<dim_t;t++)
					printf("%f ",img_t2[t]);
				printf("\n");
				*/


				for(int y=0; y<o_dim; y++)
				{
					/*
					for(int t =y; t<y+3;t++)
						printf("%f ",img_t0[t]);
					printf("\n");
					for(int t =y; t<y+3;t++)
						printf("%f ",img_t1[t]);
					printf("\n");
					for(int t =y; t<y+3;t++)
						printf("%f ",img_t2[t]);
					printf("\n");
					printf("\n");
					*/

					//for(int t=0; t<F_DIM; t++)
					//	for(int z=0; z<F_DIM; z++)
					//		res[x][y]+=
					for(int t =y; t<y+3;t++)
						res[x][y]+=img_t0[t]*filt[i][j][0][t-y];
					for(int t =y; t<y+3;t++)
						res[x][y]+=img_t1[t]*filt[i][j][1][t-y];
					for(int t =y; t<y+3;t++)
						res[x][y]+=img_t2[t]*filt[i][j][2][t-y];


					/*
					float reg0=bias_t;
					float reg1 = img_t0[y]*filt[i][j][0][0];
					float reg01=reg1+reg0;
					float reg2 = img_t0[y+1]*filt[i][j][0][1];
					float reg3 = img_t0[y+2]*filt[i][j][0][2];
					float reg02=reg2+reg3;
					float reg4 = img_t1[y]*filt[i][j][1][0];
					float reg5 = img_t1[y+1]*filt[i][j][1][1];
					float reg03=reg4+reg5;
					float reg000=reg0+reg01;
					float reg001 =reg03+reg02;
					float reg6 = img_t1[y+2]*filt[i][j][1][2];
					float reg7 = img_t2[y]*filt[i][j][2][0];
					float reg04=reg6+reg7;
					float reg8 = img_t2[y+1]*filt[i][j][2][1];
					reg000 = reg000+reg001;
					reg001 = reg04 + reg8;
					float reg9 = img_t2[y+2]*filt[i][j][2][2];
					reg01=reg000+reg9;

					 */
					//res[x][y] = reg01+reg001;
				}
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
