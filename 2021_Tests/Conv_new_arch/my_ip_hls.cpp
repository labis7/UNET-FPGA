#include "my_ip_hls.hpp"
//static float img[524288]; // 32x128x128 : max local image
static data_t img_t0[258]; // Line Buffer(LB1) - Including zero padding
static data_t img_t1[258]; // Line Buffer(LB2) - Including zero padding
static data_t img_t2[258]; // Line Buffer(LB3) - Including zero padding
static data_t img_t3[258]; // Line Buffer(LB3) - Including zero padding
static data_t img_t4[258]; // Line Buffer(LB3) - Including zero padding
static data_t img_t5[258]; // Line Buffer(LB3) - Including zero padding

static data_t filt[F_DIM*F_DIM]; // Filter locally saved
static data_t res[129][258];     // temporary result buffer(1 output channel)
static data_t ress[129][258];



void Conv(stream<data_t> &image, stream<data_t> &filter, stream<float> &bias, stream<data_t> &result, data &slaveIn) {
/*
#pragma HLS RESOURCE variable=img_t5 core=RAM_T2P_BRAM
#pragma HLS RESOURCE variable=img_t1 core=RAM_T2P_BRAM
#pragma HLS RESOURCE variable=img_t4 core=RAM_T2P_BRAM
#pragma HLS RESOURCE variable=img_t3 core=RAM_T2P_BRAM
#pragma HLS RESOURCE variable=img_t2 core=RAM_T2P_BRAM
#pragma HLS RESOURCE variable=img_t0 core=RAM_T2P_BRAM
*/




//AXI-4 streaming interfaces
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=bias
#pragma HLS INTERFACE axis register both port=result

//AXI-Lite interface
#pragma HLS INTERFACE s_axilite port=slaveIn bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

//Local BRAM array partitioning
#pragma HLS ARRAY_PARTITION variable=res cyclic factor=8 dim=2
#pragma HLS ARRAY_PARTITION variable=ress cyclic factor=8 dim=2
#pragma HLS ARRAY_PARTITION variable=filt cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t0 cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t1 cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t2 cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t3 cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t4 cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t5 cyclic factor=8 dim=1
//#pragma HLS ARRAY_PARTITION variable=img cyclic factor=2 dim=1


	data dataOut;
	int dim,ch,f_num;
	//read from axi lite
	ch =slaveIn.ch;
	dim = slaveIn.dim;
	f_num = slaveIn.f_num;

	//Load the whole input image
	/*
	for(int c=0; c<ch ; c++)
#pragma HLS loop_tripcount min=256 max=256
		for(int i=0;i<dim;i++)
#pragma HLS loop_tripcount min=16 max=16
			for(int j=0;j<dim;j++)
#pragma HLS loop_tripcount min=16 max=16
				img[c*dim*dim + i*dim+j]=image.read();
*/
	data_t reg0   ;
	data_t reg1   ;
	data_t reg01 ;
	data_t reg2   ;
	data_t reg3   ;
	data_t reg02 ;
	data_t reg4   ;
	data_t reg5   ;
	data_t reg03 ;
	data_t reg6   ;
	data_t reg7   ;
    data_t reg04 ;
	data_t reg8  ;
	data_t reg001;
	data_t res1;
	data_t reg002 ;
	data_t reg012 ;
	data_t reg003 ;
	data_t reg004;
	data_t reg034;
	data_t res100;

		
	int s = 1; //stride
	int pad, o_dim,o_ch,dim_t;

////////////////////////////     CONVOLUTION       /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

	pad = 1; //by default -- Transposed convolution will use a different approach
	o_dim = dim;
	dim_t = dim + 2*pad; //zero padded array
	o_ch = f_num;


#pragma HLS inline
	//init with zeros 1st row of linebuffer plus the 1st and last element(padding) for each row of  the linebuffer
	for(int y=0; y<(dim_t); y+=2)
#pragma HLS loop_tripcount min=17 max=17
#pragma HLS pipeline
	{
		img_t0[y] = 0;
		img_t0[y+1] = 0;
	}
	img_t1[0]=0;
	img_t1[dim_t-1]=0;
	img_t2[0]=0;
	img_t2[dim_t-1]=0;
	img_t3[0]=0;
	img_t3[dim_t-1]=0;
	img_t4[0]=0;
	img_t4[dim_t-1]=0;
	img_t5[0]=0;
	img_t5[dim_t-1]=0;

	// Now we can start the convolution
	data_t sum;
	int counter=0;
	for (int i=0; i<f_num; i++)//number of filters
	{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=128 max=128
		float bias_t = bias.read(); //read bias - one per filter
		//initiate result buffer with bias
		for(int x=0; x<o_dim/2; x++)
		{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
			for(int y=0; y<o_dim; y++)
#pragma HLS loop_tripcount min=32 max=32
			{
				res[x][y] = bias_t;
				ress[x][y] = bias_t;
			}

		}
		//seeking on the temp image's sub array that we want to mult item wise and then add them for the (x,y) result
		//printf("\n");
		for(int j=0; j < ch ; j++) //input channel loop
		{

#pragma HLS loop_tripcount min=256 max=256

			//load the 2nd row of the image,assuming that the previous iteration completed the init
			for(int z = 1 ; z<dim_t-1; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
			{
				img_t1[z] = image.read();//img[counter++];
				img_t1[z+1] = image.read();//img[counter++];
			}

			//load the corresponding filter for this input channel
			filt[0] = filter.read();
			filt[1] = filter.read();
			filt[2] = filter.read();
			filt[3] = filter.read();
			filt[4] = filter.read();
			filt[5] = filter.read();
			filt[6] = filter.read();
			filt[7] = filter.read();
			filt[8] = filter.read();

			// CONVOLUTION
			for(int x=0; x<(o_dim/2)-2; x+=2)//last 4 iterations are skipped for now ...
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=7 max=7

				for(int z = 1 ; z<dim_t-1; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				{
					img_t2[z] = image.read();//img[counter++];//load the new line buffer(3rd)
					img_t2[z+1] = image.read();
				}
				for(int z = 1 ; z<dim_t-1; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				{
					img_t3[z] = image.read();//img[counter++];//load the new line buffer(4th)
					img_t3[z+1] = image.read();
				}
				for(int z = 1 ; z<dim_t-1; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				{
					img_t4[z] = image.read();//img[counter++];//load the new line buffer(4th)
					img_t4[z+1] = image.read();
				}
				for(int z = 1 ; z<dim_t-1; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				{
					img_t5[z] = image.read();//img[counter++];//load the new line buffer(4th)
					img_t5[z+1] = image.read();

				}


				//Execute the actual convolution, loop unrolled factor=2
				for(int y=0; y<o_dim; y+=8)
				{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=4 max=4
					//tree-structure calculation
					reg0   = img_t0[y]*filt[0];
					reg1   = img_t0[y+1]*filt[1];
					reg01 = reg0+reg1;
					reg2   = img_t0[y+2]*filt[2];
					reg3   = img_t1[y]*filt[3];
					reg02 = reg2+reg3;
					reg4   = img_t1[y+1]*filt[4];
					reg5   = img_t1[y+2]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					reg6  = img_t2[y]*filt[6];
					reg7  = img_t2[y+1]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t2[y+2]*filt[8];
					////////////////////////////////
					reg0 = img_t0[y+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t0[y+2]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t0[y+3]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t1[y+1]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t1[y+2]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t1[y+3]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t2[y+1]*filt[6];
					res[x][y]+=  res1;
					reg7 = img_t2[y+2]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t2[y+3]*filt[8];
					////////////////////
					reg0   = img_t0[y+2]*filt[0];
					reg1   = img_t0[y+3]*filt[1];
					reg01 = reg0+reg1;
					reg034 = reg003 +reg004;
					reg2   = img_t0[y+4]*filt[2];
					reg3   = img_t1[y+2]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;
					reg4   = img_t1[y+3]*filt[4];
					reg5   = img_t1[y+4]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					res[x][y+1]+=  res100 + reg8;
					reg6  = img_t2[y+2]*filt[6];
					reg7  = img_t2[y+3]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t2[y+4]*filt[8];
					////////////////////////
					reg0 = img_t0[y+3]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t0[y+4]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t0[y+5]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t1[y+3]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t1[y+4]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t1[y+5]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t2[y+3]*filt[6];
					res[x][y+2]+=  res1;
					reg7 = img_t2[y+4]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t2[y+5]*filt[8];
					////////////////////////////////////////////////////////////////////////////////// 4 more unrolled
					int y_t=y+4;
					reg0   = img_t0[y+4]*filt[0];
					reg1   = img_t0[y+5]*filt[1];
					reg034 = reg003 +reg004;//
					reg01 = reg0+reg1;
					reg2   = img_t0[y_t+2]*filt[2];
					reg3   = img_t1[y_t]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;//
					reg4   = img_t1[y_t+1]*filt[4];
					reg5   = img_t1[y_t+2]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					res[x][y+3]+=  res100 + reg8;//
					reg6  = img_t2[y_t]*filt[6];
					reg7  = img_t2[y_t+1]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t2[y_t+2]*filt[8];
					////////////////////////////////
					reg0 = img_t0[y_t+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t0[y_t+2]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t0[y_t+3]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t1[y_t+1]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t1[y_t+2]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t1[y_t+3]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t2[y_t+1]*filt[6];
					res[x][y_t]+=  res1;
					reg7 = img_t2[y_t+2]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t2[y_t+3]*filt[8];
					////////////////////
					reg0   = img_t0[y_t+2]*filt[0];
					reg1   = img_t0[y_t+3]*filt[1];
					reg01 = reg0+reg1;
					reg034 = reg003 +reg004;
					reg2   = img_t0[y_t+4]*filt[2];
					reg3   = img_t1[y_t+2]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;
					reg4   = img_t1[y_t+3]*filt[4];
					reg5   = img_t1[y_t+4]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					res[x][y_t+1]+=  res100 + reg8;
					reg6  = img_t2[y_t+2]*filt[6];
					reg7  = img_t2[y_t+3]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t2[y_t+4]*filt[8];
					////////////////////////
					reg0 = img_t0[y_t+3]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t0[y_t+4]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t0[y_t+5]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t1[y_t+3]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t1[y_t+4]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t1[y_t+5]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t2[y_t+3]*filt[6];
					res[x][y_t+2]+=  res1;
					reg7 = img_t2[y_t+4]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t2[y_t+5]*filt[8];
					////////////////////
					reg034 = reg003 +reg004;
					res100 = reg012+reg034;
					res[x][y_t+3]+=  res100 + reg8;
					///////////////////////////////////////////////////////////////////  2/4   /////////////////////////////////////////////////

					reg0   = img_t1[y]*filt[0];
					reg1   = img_t1[y+1]*filt[1];
					//reg034 = reg003 +reg004;       ///////PREVIOUS PART
					reg01 = reg0+reg1;
					reg2   = img_t1[y+2]*filt[2];
					reg3   = img_t2[y]*filt[3];
					reg02 = reg2+reg3;
					//res100 = reg012+reg034;         ///////PREVIOUS PART
					reg4   = img_t2[y+1]*filt[4];
					reg5   = img_t2[y+2]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					//res[x][y_t+3]+=  res100 + reg8; ///////PREVIOUS PART
					reg6  = img_t3[y]*filt[6];
					reg7  = img_t3[y+1]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t3[y+2]*filt[8];
					////////////////////////////////
					reg0 = img_t1[y+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t1[y+2]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t1[y+3]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t2[y+1]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t2[y+2]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t2[y+3]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t3[y+1]*filt[6];
					res[x+1][y]+=  res1;
					reg7 = img_t3[y+2]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t3[y+3]*filt[8];
					////////////////////
					reg0   = img_t1[y+2]*filt[0];
					reg1   = img_t1[y+3]*filt[1];
					reg01 = reg0+reg1;
					reg034 = reg003 +reg004;
					reg2   = img_t1[y+4]*filt[2];
					reg3   = img_t2[y+2]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;
					reg4   = img_t2[y+3]*filt[4];
					reg5   = img_t2[y+4]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					res[x+1][y+1]+=  res100 + reg8;
					reg6  = img_t3[y+2]*filt[6];
					reg7  = img_t3[y+3]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t3[y+4]*filt[8];
					////////////////////////
					reg0 = img_t1[y+3]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t1[y+4]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t1[y+5]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t2[y+3]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t2[y+4]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t2[y+5]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t3[y+3]*filt[6];
					res[x+1][y+2]+=  res1;
					reg7 = img_t3[y+4]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t3[y+5]*filt[8];
					////////////////////////////////////////////////////////////////////////////////// 4 more unrolled
					y_t=y+4;
					reg0   = img_t1[y+4]*filt[0];
					reg1   = img_t1[y+5]*filt[1];
					reg034 = reg003 +reg004;//
					reg01 = reg0+reg1;
					reg2   = img_t1[y_t+2]*filt[2];
					reg3   = img_t2[y_t]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;//
					reg4   = img_t2[y_t+1]*filt[4];
					reg5   = img_t2[y_t+2]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					res[x+1][y+3]+=  res100 + reg8;//
					reg6  = img_t3[y_t]*filt[6];
					reg7  = img_t3[y_t+1]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t3[y_t+2]*filt[8];
					////////////////////////////////
					reg0 = img_t1[y_t+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t1[y_t+2]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t1[y_t+3]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t2[y_t+1]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t2[y_t+2]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t2[y_t+3]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t3[y_t+1]*filt[6];
					res[x+1][y_t]+=  res1;
					reg7 = img_t3[y_t+2]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t3[y_t+3]*filt[8];
					////////////////////
					reg0   = img_t1[y_t+2]*filt[0];
					reg1   = img_t1[y_t+3]*filt[1];
					reg01 = reg0+reg1;
					reg034 = reg003 +reg004;
					reg2   = img_t1[y_t+4]*filt[2];
					reg3   = img_t2[y_t+2]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;
					reg4   = img_t2[y_t+3]*filt[4];
					reg5   = img_t2[y_t+4]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					res[x+1][y_t+1]+=  res100 + reg8;
					reg6  = img_t3[y_t+2]*filt[6];
					reg7  = img_t3[y_t+3]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t3[y_t+4]*filt[8];
					////////////////////////
					reg0 = img_t1[y_t+3]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t1[y_t+4]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t1[y_t+5]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t2[y_t+3]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t2[y_t+4]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t2[y_t+5]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t3[y_t+3]*filt[6];
					res[x+1][y_t+2]+=  res1;
					reg7 = img_t3[y_t+4]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t3[y_t+5]*filt[8];
					////////////////////
					reg034 = reg003 +reg004;
					res100 = reg012+reg034;
					res[x+1][y_t+3]+=  res100 + reg8;

					//////////////////////////////////////////////////////////////////////   3/4  ////////////////////////////////////////////////////////

					reg0   = img_t2[y]*filt[0];
					reg1   = img_t2[y+1]*filt[1];
					//reg034 = reg003 +reg004;       ///////PREVIOUS PART
					reg01 = reg0+reg1;
					reg2   = img_t2[y+2]*filt[2];
					reg3   = img_t3[y]*filt[3];
					reg02 = reg2+reg3;
					//res100 = reg012+reg034;         ///////PREVIOUS PART
					reg4   = img_t3[y+1]*filt[4];
					reg5   = img_t3[y+2]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					//res[x][y_t+3]+=  res100 + reg8; ///////PREVIOUS PART
					reg6  = img_t4[y]*filt[6];
					reg7  = img_t4[y+1]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t4[y+2]*filt[8];
					////////////////////////////////
					reg0 = img_t2[y+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t2[y+2]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t2[y+3]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t3[y+1]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t3[y+2]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t3[y+3]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t4[y+1]*filt[6];
					ress[x][y]+= res1;
					reg7 = img_t4[y+2]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t4[y+3]*filt[8];
					////////////////////
					reg0   = img_t2[y+2]*filt[0];
					reg1   = img_t2[y+3]*filt[1];
					reg01 = reg0+reg1;
					reg034 = reg003 +reg004;
					reg2   = img_t2[y+4]*filt[2];
					reg3   = img_t3[y+2]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;
					reg4   = img_t3[y+3]*filt[4];
					reg5   = img_t3[y+4]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					ress[x][y+1]+=  res100 + reg8;
					reg6  = img_t4[y+2]*filt[6];
					reg7  = img_t4[y+3]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t4[y+4]*filt[8];
					////////////////////////
					reg0 = img_t2[y+3]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t2[y+4]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t2[y+5]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t3[y+3]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t3[y+4]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t3[y+5]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t4[y+3]*filt[6];
					ress[x][y+2]+=  res1;
					reg7 = img_t4[y+4]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t4[y+5]*filt[8];
					////////////////////////////////////////////////////////////////////////////////// 4 more unrolled
					y_t=y+4;
					reg0   = img_t2[y+4]*filt[0];
					reg1   = img_t2[y+5]*filt[1];
					reg034 = reg003 +reg004;//
					reg01 = reg0+reg1;
					reg2   = img_t2[y_t+2]*filt[2];
					reg3   = img_t3[y_t]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;//
					reg4   = img_t3[y_t+1]*filt[4];
					reg5   = img_t3[y_t+2]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					ress[x][y+3]+=  res100 + reg8;//
					reg6  = img_t4[y_t]*filt[6];
					reg7  = img_t4[y_t+1]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t4[y_t+2]*filt[8];
					////////////////////////////////
					reg0 = img_t2[y_t+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t2[y_t+2]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t2[y_t+3]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t3[y_t+1]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t3[y_t+2]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t3[y_t+3]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t4[y_t+1]*filt[6];
					ress[x][y_t]+=  res1;
					reg7 = img_t4[y_t+2]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t4[y_t+3]*filt[8];
					////////////////////
					reg0   = img_t2[y_t+2]*filt[0];
					reg1   = img_t2[y_t+3]*filt[1];
					reg01 = reg0+reg1;
					reg034 = reg003 +reg004;
					reg2   = img_t2[y_t+4]*filt[2];
					reg3   = img_t3[y_t+2]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;
					reg4   = img_t3[y_t+3]*filt[4];
					reg5   = img_t3[y_t+4]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					ress[x][y_t+1]+=  res100 + reg8;
					reg6  = img_t4[y_t+2]*filt[6];
					reg7  = img_t4[y_t+3]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t4[y_t+4]*filt[8];
					////////////////////////
					reg0 = img_t2[y_t+3]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t2[y_t+4]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t2[y_t+5]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t3[y_t+3]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t3[y_t+4]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t3[y_t+5]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t4[y_t+3]*filt[6];
					ress[x][y_t+2]+=  res1;
					reg7 = img_t4[y_t+4]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t4[y_t+5]*filt[8];
					////////////////////
					reg034 = reg003 +reg004;
					res100 = reg012+reg034;
					ress[x][y_t+3]+=  res100 + reg8;

					/////////////////////////////////////////////////////////////////    4/4      //////////////////////////////////////////////////////////


					reg0   = img_t3[y]*filt[0];
					reg1   = img_t3[y+1]*filt[1];
					//reg034 = reg003 +reg004;       ///////PREVIOUS PART
					reg01 = reg0+reg1;
					reg2   = img_t3[y+2]*filt[2];
					reg3   = img_t4[y]*filt[3];
					reg02 = reg2+reg3;
					//res100 = reg012+reg034;         ///////PREVIOUS PART
					reg4   = img_t4[y+1]*filt[4];
					reg5   = img_t4[y+2]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					//res[x][y_t+3]+=  res100 + reg8; ///////PREVIOUS PART
					reg6  = img_t5[y]*filt[6];
					reg7  = img_t5[y+1]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t5[y+2]*filt[8];
					////////////////////////////////
					reg0 = img_t3[y+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t3[y+2]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t3[y+3]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t4[y+1]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t4[y+2]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t4[y+3]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t5[y+1]*filt[6];
					ress[x+1][y]+=  res1;
					reg7 = img_t5[y+2]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t5[y+3]*filt[8];
					////////////////////
					reg0   = img_t3[y+2]*filt[0];
					reg1   = img_t3[y+3]*filt[1];
					reg01 = reg0+reg1;
					reg034 = reg003 +reg004;
					reg2   = img_t3[y+4]*filt[2];
					reg3   = img_t4[y+2]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;
					reg4   = img_t4[y+3]*filt[4];
					reg5   = img_t4[y+4]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					ress[x+1][y+1]+=  res100 + reg8;
					reg6  = img_t5[y+2]*filt[6];
					reg7  = img_t5[y+3]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t5[y+4]*filt[8];
					////////////////////////
					reg0 = img_t3[y+3]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t3[y+4]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t3[y+5]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t4[y+3]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t4[y+4]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t4[y+5]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t5[y+3]*filt[6];
					ress[x+1][y+2]+=  res1;
					reg7 = img_t5[y+4]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t5[y+5]*filt[8];
					////////////////////////////////////////////////////////////////////////////////// 4 more unrolled
					y_t=y+4;
					reg0   = img_t3[y+4]*filt[0];
					reg1   = img_t3[y+5]*filt[1];
					reg034 = reg003 +reg004;//
					reg01 = reg0+reg1;
					reg2   = img_t3[y_t+2]*filt[2];
					reg3   = img_t4[y_t]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;//
					reg4   = img_t4[y_t+1]*filt[4];
					reg5   = img_t4[y_t+2]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					ress[x+1][y+3]+=  res100 + reg8;//
					reg6  = img_t5[y_t]*filt[6];
					reg7  = img_t5[y_t+1]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t5[y_t+2]*filt[8];
					////////////////////////////////
					reg0 = img_t3[y_t+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t3[y_t+2]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t3[y_t+3]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t4[y_t+1]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t4[y_t+2]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t4[y_t+3]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t5[y_t+1]*filt[6];
					ress[x+1][y_t]+=  res1;
					reg7 = img_t5[y_t+2]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t5[y_t+3]*filt[8];
					////////////////////
					reg0   = img_t3[y_t+2]*filt[0];
					reg1   = img_t3[y_t+3]*filt[1];
					reg01 = reg0+reg1;
					reg034 = reg003 +reg004;
					reg2   = img_t3[y_t+4]*filt[2];
					reg3   = img_t4[y_t+2]*filt[3];
					reg02 = reg2+reg3;
					res100 = reg012+reg034;
					reg4   = img_t4[y_t+3]*filt[4];
					reg5   = img_t4[y_t+4]*filt[5];
					reg01 = reg01 + reg02;
					reg03 = reg4+reg5;
					ress[x+1][y_t+1]+=  res100 + reg8;
					reg6  = img_t5[y_t+2]*filt[6];
					reg7  = img_t5[y_t+3]*filt[7];
					reg04= reg6+reg7;
					reg8  = img_t5[y_t+4]*filt[8];
					////////////////////////
					reg0 = img_t3[y_t+3]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t3[y_t+4]*filt[1];
					reg001 = reg0 +reg1;
					reg2 = img_t3[y_t+5]*filt[2];
					res1 =reg04+reg8;
					reg3 = img_t4[y_t+3]*filt[3];
					reg002 = reg2 + reg3;
					reg4 = img_t4[y_t+4]*filt[4];
					res1 = res1 + reg01;
					reg5 = img_t4[y_t+5]*filt[5];
					reg012 = reg001+reg002;
					reg003 =reg4+reg5;
					reg6 =img_t5[y_t+3]*filt[6];
					ress[x+1][y_t+2]+=  res1;
					reg7 = img_t5[y_t+4]*filt[7];
					reg004 = reg6+reg7;
					reg8 = img_t5[y_t+5]*filt[8];
					////////////////////
					reg034 = reg003 +reg004;
					res100 = reg012+reg034;
					ress[x+1][y_t+3]+=  res100 + reg8;
				}
				
				//Shift up : LB2-->LB1 , LB3-->LB2,
				//Unrolled 2 times
				for(int y=1; y<(dim_t-1); y+=2)
				{
#pragma HLS loop_tripcount min=16 max=16
#pragma HLS pipeline
					img_t0[y] = img_t4[y];//LB2-->LB1
					img_t1[y] = img_t5[y];//LB3-->LB2
					img_t0[y+1] = img_t4[y+1];
					img_t1[y+1] = img_t5[y+1];
				}
			}
			//LAST ITERATION, the shift ups for 1st and 2nd rows are completed above
			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline
				img_t2[y]=image.read();

			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline
				img_t3[y]=image.read();

			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline
			{
				img_t4[y]=image.read();
				img_t5[y] = 0;//the last line of the input image must consists of zeros(zero padding==1)#todo:replaec img_t3 with zeros in that llast iter
			}

			for(int y=0; y<o_dim; y+=8)
			{
#pragma HLS loop_tripcount min=4 max=4
#pragma HLS pipeline
				reg0   = img_t0[y]*filt[0];
				reg1   = img_t0[y+1]*filt[1];
				reg01 = reg0+reg1;
				reg2   = img_t0[y+2]*filt[2];
				reg3   = img_t1[y]*filt[3];
				reg02 = reg2+reg3;
				reg4   = img_t1[y+1]*filt[4];
				reg5   = img_t1[y+2]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				reg6  = img_t2[y]*filt[6];
				reg7  = img_t2[y+1]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t2[y+2]*filt[8];
				////////////////////////////////
				reg0 = img_t0[y+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t0[y+2]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t0[y+3]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t1[y+1]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t1[y+2]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t1[y+3]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t2[y+1]*filt[6];
				res[(o_dim/2)-2][y]+=  res1;
				reg7 = img_t2[y+2]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t2[y+3]*filt[8];
				////////////////////
				reg0   = img_t0[y+2]*filt[0];
				reg1   = img_t0[y+3]*filt[1];
				reg01 = reg0+reg1;
				reg034 = reg003 +reg004;
				reg2   = img_t0[y+4]*filt[2];
				reg3   = img_t1[y+2]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;
				reg4   = img_t1[y+3]*filt[4];
				reg5   = img_t1[y+4]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				res[(o_dim/2)-2][y+1]+=  res100 + reg8;
				reg6  = img_t2[y+2]*filt[6];
				reg7  = img_t2[y+3]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t2[y+4]*filt[8];
				////////////////////////
				reg0 = img_t0[y+3]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t0[y+4]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t0[y+5]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t1[y+3]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t1[y+4]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t1[y+5]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t2[y+3]*filt[6];
				res[(o_dim/2)-2][y+2]+=  res1;
				reg7 = img_t2[y+4]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t2[y+5]*filt[8];
				////////////////////////////////////////////////////////////////////////////////// 4 more unrolled
				int y_t=y+4;
				reg0   = img_t0[y+4]*filt[0];
				reg1   = img_t0[y+5]*filt[1];
				reg034 = reg003 +reg004;//
				reg01 = reg0+reg1;
				reg2   = img_t0[y_t+2]*filt[2];
				reg3   = img_t1[y_t]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;//
				reg4   = img_t1[y_t+1]*filt[4];
				reg5   = img_t1[y_t+2]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				res[(o_dim/2)-2][y+3]+=  res100 + reg8;//
				reg6  = img_t2[y_t]*filt[6];
				reg7  = img_t2[y_t+1]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t2[y_t+2]*filt[8];
				////////////////////////////////
				reg0 = img_t0[y_t+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t0[y_t+2]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t0[y_t+3]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t1[y_t+1]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t1[y_t+2]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t1[y_t+3]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t2[y_t+1]*filt[6];
				res[(o_dim/2)-2][y_t]+=  res1;
				reg7 = img_t2[y_t+2]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t2[y_t+3]*filt[8];
				////////////////////
				reg0   = img_t0[y_t+2]*filt[0];
				reg1   = img_t0[y_t+3]*filt[1];
				reg01 = reg0+reg1;
				reg034 = reg003 +reg004;
				reg2   = img_t0[y_t+4]*filt[2];
				reg3   = img_t1[y_t+2]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;
				reg4   = img_t1[y_t+3]*filt[4];
				reg5   = img_t1[y_t+4]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				res[(o_dim/2)-2][y_t+1]+=  res100 + reg8;
				reg6  = img_t2[y_t+2]*filt[6];
				reg7  = img_t2[y_t+3]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t2[y_t+4]*filt[8];
				////////////////////////
				reg0 = img_t0[y_t+3]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t0[y_t+4]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t0[y_t+5]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t1[y_t+3]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t1[y_t+4]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t1[y_t+5]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t2[y_t+3]*filt[6];
				res[(o_dim/2)-2][y_t+2]+=  res1;
				reg7 = img_t2[y_t+4]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t2[y_t+5]*filt[8];
				////////////////////
				reg034 = reg003 +reg004;
				res100 = reg012+reg034;
				res[(o_dim/2)-2][y_t+3]+=  res100 + reg8;
				///////////////////////////////////////////////////////////////////  2/4   /////////////////////////////////////////////////

				reg0   = img_t1[y]*filt[0];
				reg1   = img_t1[y+1]*filt[1];
				//reg034 = reg003 +reg004;       ///////PREVIOUS PART
				reg01 = reg0+reg1;
				reg2   = img_t1[y+2]*filt[2];
				reg3   = img_t2[y]*filt[3];
				reg02 = reg2+reg3;
				//res100 = reg012+reg034;         ///////PREVIOUS PART
				reg4   = img_t2[y+1]*filt[4];
				reg5   = img_t2[y+2]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				//res[x][y_t+3]+=  res100 + reg8; ///////PREVIOUS PART
				reg6  = img_t3[y]*filt[6];
				reg7  = img_t3[y+1]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t3[y+2]*filt[8];
				////////////////////////////////
				reg0 = img_t1[y+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t1[y+2]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t1[y+3]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t2[y+1]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t2[y+2]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t2[y+3]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t3[y+1]*filt[6];
				res[(o_dim/2)-1][y]+=  res1;
				reg7 = img_t3[y+2]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t3[y+3]*filt[8];
				////////////////////
				reg0   = img_t1[y+2]*filt[0];
				reg1   = img_t1[y+3]*filt[1];
				reg01 = reg0+reg1;
				reg034 = reg003 +reg004;
				reg2   = img_t1[y+4]*filt[2];
				reg3   = img_t2[y+2]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;
				reg4   = img_t2[y+3]*filt[4];
				reg5   = img_t2[y+4]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				res[(o_dim/2)-1][y+1]+=  res100 + reg8;
				reg6  = img_t3[y+2]*filt[6];
				reg7  = img_t3[y+3]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t3[y+4]*filt[8];
				////////////////////////
				reg0 = img_t1[y+3]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t1[y+4]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t1[y+5]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t2[y+3]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t2[y+4]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t2[y+5]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t3[y+3]*filt[6];
				res[(o_dim/2)-1][y+2]+=  res1;
				reg7 = img_t3[y+4]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t3[y+5]*filt[8];
				////////////////////////////////////////////////////////////////////////////////// 4 more unrolled
				y_t=y+4;
				reg0   = img_t1[y+4]*filt[0];
				reg1   = img_t1[y+5]*filt[1];
				reg034 = reg003 +reg004;//
				reg01 = reg0+reg1;
				reg2   = img_t1[y_t+2]*filt[2];
				reg3   = img_t2[y_t]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;//
				reg4   = img_t2[y_t+1]*filt[4];
				reg5   = img_t2[y_t+2]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				res[(o_dim/2)-1][y+3]+=  res100 + reg8;//
				reg6  = img_t3[y_t]*filt[6];
				reg7  = img_t3[y_t+1]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t3[y_t+2]*filt[8];
				////////////////////////////////
				reg0 = img_t1[y_t+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t1[y_t+2]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t1[y_t+3]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t2[y_t+1]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t2[y_t+2]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t2[y_t+3]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t3[y_t+1]*filt[6];
				res[(o_dim/2)-1][y_t]+=  res1;
				reg7 = img_t3[y_t+2]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t3[y_t+3]*filt[8];
				////////////////////
				reg0   = img_t1[y_t+2]*filt[0];
				reg1   = img_t1[y_t+3]*filt[1];
				reg01 = reg0+reg1;
				reg034 = reg003 +reg004;
				reg2   = img_t1[y_t+4]*filt[2];
				reg3   = img_t2[y_t+2]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;
				reg4   = img_t2[y_t+3]*filt[4];
				reg5   = img_t2[y_t+4]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				res[(o_dim/2)-1][y_t+1]+=  res100 + reg8;
				reg6  = img_t3[y_t+2]*filt[6];
				reg7  = img_t3[y_t+3]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t3[y_t+4]*filt[8];
				////////////////////////
				reg0 = img_t1[y_t+3]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t1[y_t+4]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t1[y_t+5]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t2[y_t+3]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t2[y_t+4]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t2[y_t+5]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t3[y_t+3]*filt[6];
				res[(o_dim/2)-1][y_t+2]+=  res1;
				reg7 = img_t3[y_t+4]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t3[y_t+5]*filt[8];
				////////////////////
				reg034 = reg003 +reg004;
				res100 = reg012+reg034;
				res[(o_dim/2)-1][y_t+3]+=  res100 + reg8;

				//////////////////////////////////////////////////////////////////////   3/4  ////////////////////////////////////////////////////////

				reg0   = img_t2[y]*filt[0];
				reg1   = img_t2[y+1]*filt[1];
				//reg034 = reg003 +reg004;       ///////PREVIOUS PART
				reg01 = reg0+reg1;
				reg2   = img_t2[y+2]*filt[2];
				reg3   = img_t3[y]*filt[3];
				reg02 = reg2+reg3;
				//res100 = reg012+reg034;         ///////PREVIOUS PART
				reg4   = img_t3[y+1]*filt[4];
				reg5   = img_t3[y+2]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				//res[x][y_t+3]+=  res100 + reg8; ///////PREVIOUS PART
				reg6  = img_t4[y]*filt[6];
				reg7  = img_t4[y+1]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t4[y+2]*filt[8];
				////////////////////////////////
				reg0 = img_t2[y+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t2[y+2]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t2[y+3]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t3[y+1]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t3[y+2]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t3[y+3]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t4[y+1]*filt[6];
				ress[(o_dim/2)-2][y]+=  res1;
				reg7 = img_t4[y+2]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t4[y+3]*filt[8];
				////////////////////
				reg0   = img_t2[y+2]*filt[0];
				reg1   = img_t2[y+3]*filt[1];
				reg01 = reg0+reg1;
				reg034 = reg003 +reg004;
				reg2   = img_t2[y+4]*filt[2];
				reg3   = img_t3[y+2]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;
				reg4   = img_t3[y+3]*filt[4];
				reg5   = img_t3[y+4]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				ress[(o_dim/2)-2][y+1]+=  res100 + reg8;
				reg6  = img_t4[y+2]*filt[6];
				reg7  = img_t4[y+3]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t4[y+4]*filt[8];
				////////////////////////
				reg0 = img_t2[y+3]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t2[y+4]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t2[y+5]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t3[y+3]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t3[y+4]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t3[y+5]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t4[y+3]*filt[6];
				ress[(o_dim/2)-2][y+2]+=  res1;
				reg7 = img_t4[y+4]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t4[y+5]*filt[8];
				////////////////////////////////////////////////////////////////////////////////// 4 more unrolled
				y_t=y+4;
				reg0   = img_t2[y+4]*filt[0];
				reg1   = img_t2[y+5]*filt[1];
				reg034 = reg003 +reg004;//
				reg01 = reg0+reg1;
				reg2   = img_t2[y_t+2]*filt[2];
				reg3   = img_t3[y_t]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;//
				reg4   = img_t3[y_t+1]*filt[4];
				reg5   = img_t3[y_t+2]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				ress[(o_dim/2)-2][y+3]+=  res100 + reg8;//
				reg6  = img_t4[y_t]*filt[6];
				reg7  = img_t4[y_t+1]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t4[y_t+2]*filt[8];
				////////////////////////////////
				reg0 = img_t2[y_t+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t2[y_t+2]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t2[y_t+3]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t3[y_t+1]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t3[y_t+2]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t3[y_t+3]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t4[y_t+1]*filt[6];
				ress[(o_dim/2)-2][y_t]+=  res1;
				reg7 = img_t4[y_t+2]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t4[y_t+3]*filt[8];
				////////////////////
				reg0   = img_t2[y_t+2]*filt[0];
				reg1   = img_t2[y_t+3]*filt[1];
				reg01 = reg0+reg1;
				reg034 = reg003 +reg004;
				reg2   = img_t2[y_t+4]*filt[2];
				reg3   = img_t3[y_t+2]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;
				reg4   = img_t3[y_t+3]*filt[4];
				reg5   = img_t3[y_t+4]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				ress[(o_dim/2)-2][y_t+1]+=  res100 + reg8;
				reg6  = img_t4[y_t+2]*filt[6];
				reg7  = img_t4[y_t+3]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t4[y_t+4]*filt[8];
				////////////////////////
				reg0 = img_t2[y_t+3]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t2[y_t+4]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t2[y_t+5]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t3[y_t+3]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t3[y_t+4]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t3[y_t+5]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t4[y_t+3]*filt[6];
				ress[(o_dim/2)-2][y_t+2]+=  res1;
				reg7 = img_t4[y_t+4]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t4[y_t+5]*filt[8];
				////////////////////
				reg034 = reg003 +reg004;
				res100 = reg012+reg034;
				ress[(o_dim/2)-2][y_t+3]+=  res100 + reg8;

				/////////////////////////////////////////////////////////////////    4/4      //////////////////////////////////////////////////////////


				reg0   = img_t3[y]*filt[0];
				reg1   = img_t3[y+1]*filt[1];
				//reg034 = reg003 +reg004;       ///////PREVIOUS PART
				reg01 = reg0+reg1;
				reg2   = img_t3[y+2]*filt[2];
				reg3   = img_t4[y]*filt[3];
				reg02 = reg2+reg3;
				//res100 = reg012+reg034;         ///////PREVIOUS PART
				reg4   = img_t4[y+1]*filt[4];
				reg5   = img_t4[y+2]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				//ress[x][y_t+3]+=  res100 + reg8; ///////PREVIOUS PART
				reg6  = img_t5[y]*filt[6];
				reg7  = img_t5[y+1]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t5[y+2]*filt[8];
				////////////////////////////////
				reg0 = img_t3[y+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t3[y+2]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t3[y+3]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t4[y+1]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t4[y+2]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t4[y+3]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t5[y+1]*filt[6];
				ress[(o_dim/2)-1][y]+=  res1;
				reg7 = img_t5[y+2]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t5[y+3]*filt[8];
				////////////////////
				reg0   = img_t3[y+2]*filt[0];
				reg1   = img_t3[y+3]*filt[1];
				reg01 = reg0+reg1;
				reg034 = reg003 +reg004;
				reg2   = img_t3[y+4]*filt[2];
				reg3   = img_t4[y+2]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;
				reg4   = img_t4[y+3]*filt[4];
				reg5   = img_t4[y+4]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				ress[(o_dim/2)-1][y+1]+=  res100 + reg8;
				reg6  = img_t5[y+2]*filt[6];
				reg7  = img_t5[y+3]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t5[y+4]*filt[8];
				////////////////////////
				reg0 = img_t3[y+3]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t3[y+4]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t3[y+5]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t4[y+3]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t4[y+4]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t4[y+5]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t5[y+3]*filt[6];
				ress[(o_dim/2)-1][y+2]+=  res1;
				reg7 = img_t5[y+4]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t5[y+5]*filt[8];
				////////////////////////////////////////////////////////////////////////////////// 4 more unrolled
				y_t=y+4;
				reg0   = img_t3[y+4]*filt[0];
				reg1   = img_t3[y+5]*filt[1];
				reg034 = reg003 +reg004;//
				reg01 = reg0+reg1;
				reg2   = img_t3[y_t+2]*filt[2];
				reg3   = img_t4[y_t]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;//
				reg4   = img_t4[y_t+1]*filt[4];
				reg5   = img_t4[y_t+2]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				ress[(o_dim/2)-1][y+3]+=  res100 + reg8;//
				reg6  = img_t5[y_t]*filt[6];
				reg7  = img_t5[y_t+1]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t5[y_t+2]*filt[8];
				////////////////////////////////
				reg0 = img_t3[y_t+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t3[y_t+2]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t3[y_t+3]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t4[y_t+1]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t4[y_t+2]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t4[y_t+3]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t5[y_t+1]*filt[6];
				ress[(o_dim/2)-1][y_t]+=  res1;
				reg7 = img_t5[y_t+2]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t5[y_t+3]*filt[8];
				////////////////////
				reg0   = img_t3[y_t+2]*filt[0];
				reg1   = img_t3[y_t+3]*filt[1];
				reg01 = reg0+reg1;
				reg034 = reg003 +reg004;
				reg2   = img_t3[y_t+4]*filt[2];
				reg3   = img_t4[y_t+2]*filt[3];
				reg02 = reg2+reg3;
				res100 = reg012+reg034;
				reg4   = img_t4[y_t+3]*filt[4];
				reg5   = img_t4[y_t+4]*filt[5];
				reg01 = reg01 + reg02;
				reg03 = reg4+reg5;
				ress[(o_dim/2)-1][y_t+1]+=  res100 + reg8;
				reg6  = img_t5[y_t+2]*filt[6];
				reg7  = img_t5[y_t+3]*filt[7];
				reg04= reg6+reg7;
				reg8  = img_t5[y_t+4]*filt[8];
				////////////////////////
				reg0 = img_t3[y_t+3]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t3[y_t+4]*filt[1];
				reg001 = reg0 +reg1;
				reg2 = img_t3[y_t+5]*filt[2];
				res1 =reg04+reg8;
				reg3 = img_t4[y_t+3]*filt[3];
				reg002 = reg2 + reg3;
				reg4 = img_t4[y_t+4]*filt[4];
				res1 = res1 + reg01;
				reg5 = img_t4[y_t+5]*filt[5];
				reg012 = reg001+reg002;
				reg003 =reg4+reg5;
				reg6 =img_t5[y_t+3]*filt[6];
				ress[(o_dim/2)-1][y_t+2]+=  res1;
				reg7 = img_t5[y_t+4]*filt[7];
				reg004 = reg6+reg7;
				reg8 = img_t5[y_t+5]*filt[8];
				////////////////////
				reg034 = reg003 +reg004;
				res100 = reg012+reg034;
				ress[(o_dim/2)-1][y_t+3]+=  res100 + reg8;

			}
			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline
				img_t0[y] = 0;
		}//end of channel loop
		
		//Streaming out the result per output channel == filter
		for(int x=0; x<o_dim/2; x+=2){
#pragma HLS loop_tripcount min=8 max=8
#pragma HLS pipeline
			for(int y=0; y<o_dim; y+=2)
#pragma HLS loop_tripcount min=16 max=16
			{

				data_t tmp=res[x][y];
				data_t tmp1=res[x][y+1];
				if(tmp<=0) 			// Integraded ReLu activation function
					result.write(0);
				else
					result.write(tmp);
				if(tmp1<=0) 			// Integraded ReLu activation function
					result.write(0);
				else
					result.write(tmp1);
			}
			for(int y=0; y<o_dim; y+=2)
			{
#pragma HLS loop_tripcount min=16 max=16
				data_t tmp=res[x+1][y];
				data_t tmp1=res[x][y+1];
				if(tmp<=0) 			// Integraded ReLu activation function
					result.write(0);
				else
					result.write(tmp);
				if(tmp1<=0) 			// Integraded ReLu activation function
					result.write(0);
				else
					result.write(tmp1);
			}


			for(int y=0; y<o_dim; y+=2)
			{
#pragma HLS loop_tripcount min=16 max=16
				data_t tmp=ress[x][y];
				data_t tmp1=ress[x][y+1];
				if(tmp<=0) 			// Integraded ReLu activation function
					result.write(0);
				else
					result.write(tmp);
				if(tmp1<=0) 			// Integraded ReLu activation function
					result.write(0);
				else
					result.write(tmp1);
			}
			for(int y=0; y<o_dim; y+=2)
			{
#pragma HLS loop_tripcount min=16 max=16
				data_t tmp=ress[x+1][y];
				data_t tmp1=ress[x][y+1];
				if(tmp<=0) 			// Integraded ReLu activation function
					result.write(0);
				else
					result.write(tmp);
				if(tmp1<=0) 			// Integraded ReLu activation function
					result.write(0);
				else
					result.write(tmp1);
			}

		}

	}

	return;

}
