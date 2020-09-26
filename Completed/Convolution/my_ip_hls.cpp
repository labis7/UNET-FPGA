#include "my_ip_hls.hpp"
static float img[524288];
//static float img[10][10][10];//130*32
static float img_t0[130];
static float img_t1[130];
static float img_t2[130];
static float filt[F_DIM*F_DIM];
static float res[128][128];
//static float b[10];

void Conv(stream<float> &image, stream<float> &filter, stream<float> &bias, stream<float> &result, data &slaveIn) {
//#pragma HLS RESOURCE variable=res core=RAM_2P_URAM
//#pragma HLS ARRAY_PARTITION variable=res complete dim=2
//#pragma HLS ARRAY_PARTITION variable=res cyclic factor=4 dim=1
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=bias
#pragma HLS INTERFACE axis register both port=result

#pragma HLS INTERFACE s_axilite port=slaveIn bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

#pragma HLS ARRAY_PARTITION variable=filt cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t0 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t1 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t2 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img cyclic factor=2 dim=1

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
	int dim,ch,f_num;
	//float img_pix,filt_pix;

	//float arr[2][4][4];
	//slaveIn.read(dataOut);
	ch =slaveIn.ch;
	dim = slaveIn.dim;
	f_num = slaveIn.f_num;

	for(int c=0; c<ch ; c++)
#pragma HLS loop_tripcount min=256 max=256
		for(int i=0;i<dim;i++)
#pragma HLS loop_tripcount min=16 max=16
			for(int j=0;j<dim;j++)
#pragma HLS loop_tripcount min=16 max=16
				img[c*dim*dim + i*dim+j]=image.read();


/*
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
#pragma HLS inline
	//init with zeros 1st row of linebuffer plus the 1st and last element(padding) for each row of  the linebuffer
	for(int y=0; y<(dim_t); y++)
#pragma HLS loop_tripcount min=18 max=18
#pragma HLS pipeline
		img_t0[y] = 0;
	img_t2[0]=0;
	img_t2[dim_t-1]=0;
	img_t1[0]=0;
	img_t1[dim_t-1]=0;
	//memcpy(img_t1+1, image  , sizeof(float)*dim);


	// Now we can start the convolution
	float sum;
	int counter=0;
	for (int i=0; i<f_num; i++)//number of filters
	{
#pragma HLS pipeline
		counter=0;
#pragma HLS loop_tripcount min=128 max=128
		float bias_t = bias.read();
		for(int x=0; x<o_dim; x++)
		{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
			for(int y=0; y<o_dim; y++)
#pragma HLS loop_tripcount min=16 max=16
				res[x][y] = bias_t;
		}
		//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
		//printf("\n");
		for(int j=0; j < ch ; j++)
		{

#pragma HLS loop_tripcount min=256 max=256

			//load the 2nd row of the image,assuming that the previous iteration completed the init
			//memcpy(img_t1+1, image +j*dim*dim , sizeof(float)*320);
			for(int z = 1 ; z<dim_t-1; z++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				img_t1[z] =img[counter++];// image.read();




			//load the filter of this specific filter num and channel
			/*
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
			*/
			//memcpy(filt, filter + i*ch*F_DIM*F_DIM + j*F_DIM*F_DIM , 9*sizeof(float));
			for(int z =0 ; z<9 ;z++)
#pragma HLS loop_tripcount min=9 max=9
				filt[z] = filter.read();



			for(int x=0; x<o_dim-1; x++)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				//memcpy(img_t2+1, image+j*dim*dim +(x+1)*dim , sizeof(float)*320);
				for(int z = 1 ; z<dim_t-1; z++)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
					img_t2[z] = img[counter++];//image.read();

				for(int y=0; y<o_dim; y+=2)
				{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=8 max=8

					float reg0 = img_t0[y]*filt[0];
					float reg1 = img_t0[y+1]*filt[1];
					float reg01 = reg0+reg1;
					float reg2 = img_t0[y+2]*filt[2];
					float reg3 = img_t1[y]*filt[3];
					float reg02= reg2+reg3;
					float reg4 = img_t1[y+1]*filt[4];
					float reg5 = img_t1[y+2]*filt[5];
					reg01 = reg01 + reg02;
					float reg03= reg4+reg5;
					float reg6 = img_t2[y]*filt[6];
					float reg7 = img_t2[y+1]*filt[7];
					float reg04= reg6+reg7;
					float reg8 = img_t2[y+2]*filt[8];
					reg0 = img_t0[y+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t0[y+2]*filt[1];
					float reg001 = reg0 +reg1;
					reg2 = img_t0[y+3]*filt[2];
					float res1 =reg04+reg8;
					reg3 = img_t1[y+1]*filt[3];
					float reg002 = reg2 + reg3;
					reg4 = img_t1[y+2]*filt[4];
					reg5 = img_t1[y+3]*filt[5];
					float reg012 = reg001+reg002;
					float reg003 =reg4+reg5;
					reg6 =img_t2[y+1]*filt[6];
					res1 = res1 + reg01;
					reg7 = img_t2[y+2]*filt[7];
					float reg004 = reg6+reg7;
					float reg034 = reg003 +reg004;
					reg8 = img_t2[y+3]*filt[8];
					res[x][y]+=  res1;
					float res100 = reg012+reg034;
					res[x][y+1]+=  res100 + reg8;
					//res[x][y]+=  res1;
				}
				//memcpy(img_t0+1, img_t1+1 , sizeof(float)*dim);
				//memcpy(img_t1+1, img_t2+1 , sizeof(float)*dim);
				for(int y=1; y<(dim_t-1); y+=2)
				{
#pragma HLS loop_tripcount min=16 max=16
#pragma HLS pipeline
					img_t0[y] = img_t1[y];
					img_t1[y] = img_t2[y];
					img_t0[y+1] = img_t1[y+1];
					img_t1[y+1] = img_t2[y+1];
				}
			}
			//LAST ITER, the shift ups for 1st and 2nd rows are completed above
			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=16 max=16
#pragma HLS pipeline
				img_t2[y] = 0;

			for(int y=0; y<o_dim; y+=2)
			{
#pragma HLS loop_tripcount min=8 max=8
#pragma HLS pipeline
				//for(int t =y; t<y+3;t++)
					//res[o_dim -1][y]+=img_t0[t]*filt[0*F_DIM + t-y];

				float reg0 = img_t0[y]*filt[0];
				float reg1 = img_t0[y+1]*filt[1];
				float reg01 = reg0+reg1;
				float reg2 = img_t0[y+2]*filt[2];
				float reg3 = img_t1[y]*filt[3];
				float reg02= reg2+reg3;
				float reg4 = img_t1[y+1]*filt[4];
				float reg5 = img_t1[y+2]*filt[5];
				reg01 = reg01 + reg02;
				float reg03= reg4+reg5;
				float reg6 = img_t2[y]*filt[6];
				float reg7 = img_t2[y+1]*filt[7];
				float reg04= reg6+reg7;
				float reg8 = img_t2[y+2]*filt[8];
				reg0 = img_t0[y+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t0[y+2]*filt[1];
				float reg001 = reg0 +reg1;
				reg2 = img_t0[y+3]*filt[2];
				float res1 =reg04+reg8;
				reg3 = img_t1[y+1]*filt[3];
				float reg002 = reg2 + reg3;
				reg4 = img_t1[y+2]*filt[4];
				reg5 = img_t1[y+3]*filt[5];
				float reg012 = reg001+reg002;
				float reg003 =reg4+reg5;
				reg6 =img_t2[y+1]*filt[6];
				res1 = res1 + reg01;
				reg7 = img_t2[y+2]*filt[7];
				float reg004 = reg6+reg7;
				float reg034 = reg003 +reg004;
				reg8 = img_t2[y+3]*filt[8];
				res[o_dim -1][y]+=  res1;
				float res100 = reg012+reg034;
				res[o_dim -1][y+1]+=  res100 + reg8;

			}
			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=16 max=16
#pragma HLS pipeline
				img_t0[y] = 0;
		}
		for(int x=0; x<o_dim; x++){
#pragma HLS loop_tripcount min=16 max=16
#pragma HLS pipeline
			for(int y=0; y<o_dim; y++)
			{
#pragma HLS loop_tripcount min=16 max=16
				float tmp=res[x][y];
				if(tmp<=0)
					result.write(0);
				else
					result.write(tmp);
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
