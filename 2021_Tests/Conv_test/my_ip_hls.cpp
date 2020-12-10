#include "my_ip_hls.hpp"
//static float img[524288]; // 32x128x128 : max local image
static data_t img_t0[258]; // Line Buffer(LB1) - Including zero padding
static data_t img_t1[258]; // Line Buffer(LB2) - Including zero padding
static data_t img_t2[258]; // Line Buffer(LB3) - Including zero padding
static data_t filt[F_DIM*F_DIM]; // Filter locally saved
static data_t res[258][258];     // temporary result buffer(1 output channel)



void Conv(stream<struct_t> &image, stream<struct_t> &filter, stream<float> &bias, stream<struct_t> &result, data &slaveIn) {


//AXI-4 streaming interfaces
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=bias
#pragma HLS INTERFACE axis register both port=result

//AXI-Lite interface
#pragma HLS INTERFACE s_axilite port=slaveIn bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

//Local BRAM array partitioning
#pragma HLS ARRAY_PARTITION variable=filt cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t0 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t1 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t2 cyclic factor=2 dim=1

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
	for(int y=0; y<(dim_t); y++)
#pragma HLS loop_tripcount min=34 max=34
#pragma HLS pipeline
		img_t0[y] = 0;
	img_t2[0]=0;
	img_t2[dim_t-1]=0;
	img_t1[0]=0;
	img_t1[dim_t-1]=0;

	struct_t img_data;
	// Now we can start the convolution
	data_t sum;
	int counter=0;
	for (int i=0; i<f_num; i++)//number of filters
	{
#pragma HLS pipeline
		counter=0;
#pragma HLS loop_tripcount min=128 max=128
		data_t bias_t = data_t(bias.read()); //read bias - one per filter
		//initiate result buffer with bias
		for(int x=0; x<o_dim; x++)
		{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
			for(int y=0; y<o_dim; y++)
#pragma HLS loop_tripcount min=32 max=32
				res[x][y] = bias_t;
		}
		//seeking on the temp image's sub array that we want to mult item wise and then add them for the (x,y) result
		//printf("\n");
		for(int j=0; j < ch ; j++) //input channel loop
		{

#pragma HLS loop_tripcount min=256 max=256

			//load the 2nd row of the image,assuming that the previous iteration completed the init
			for(int z = 1 ; z<dim_t-1; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
			{
				img_data = image.read();
				img_t1[z] = img_data.v1;
				img_t1[z+1] = img_data.v2;
				//img_t1[z] = image.read();//img[counter++];
			}

			//load the corresponding filter for this input channel
			struct_t filt_data;
			for(int z =0 ; z<9 ;z+=3)
#pragma HLS loop_tripcount min=9 max=9
			{
				filt_data = filter.read();
				filt[z] = filt_data.v1;
				filt[z+1] = filt_data.v2;
				filt[z+2] = filt_data.v3;
			}

			// CONVOLUTION
			for(int x=0; x<o_dim-1; x++)//last iteration is skipped for now ...
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=31 max=31
				for(int z = 1 ; z<dim_t-1; z+=2)
#pragma HLS pipeline
#pragma HLS loop_tripcount min=32 max=32
				{
					img_data = image.read();
					img_t2[z] = img_data.v1;
					img_t2[z+1] = img_data.v2;
					//img_t2[z] = image.read();//img[counter++];//load the new line buffer(3rd)
				}
				//Execute the actual convolution, loop unrolled factor=2
				for(int y=0; y<o_dim; y+=2)
				{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
					//tree-structure calculation
					data_t reg0 = img_t0[y]*filt[0];
					data_t reg1 = img_t0[y+1]*filt[1];
					data_t reg01 = reg0+reg1;
					data_t reg2 = img_t0[y+2]*filt[2];
					data_t reg3 = img_t1[y]*filt[3];
					data_t reg02= reg2+reg3;
					data_t reg4 = img_t1[y+1]*filt[4];
					data_t reg5 = img_t1[y+2]*filt[5];
					reg01 = reg01 + reg02;
					data_t reg03= reg4+reg5;
					data_t reg6 = img_t2[y]*filt[6];
					data_t reg7 = img_t2[y+1]*filt[7];
					data_t reg04= reg6+reg7;
					data_t reg8 = img_t2[y+2]*filt[8];
					reg0 = img_t0[y+1]*filt[0];
					reg01 = reg01+reg03;
					reg1 = img_t0[y+2]*filt[1];
					data_t reg001 = reg0 +reg1;
					reg2 = img_t0[y+3]*filt[2];
					data_t res1 =reg04+reg8;
					reg3 = img_t1[y+1]*filt[3];
					data_t reg002 = reg2 + reg3;
					reg4 = img_t1[y+2]*filt[4];
					reg5 = img_t1[y+3]*filt[5];
					data_t reg012 = reg001+reg002;
					data_t reg003 =reg4+reg5;
					reg6 =img_t2[y+1]*filt[6];
					res1 = res1 + reg01;
					reg7 = img_t2[y+2]*filt[7];
					data_t reg004 = reg6+reg7;
					data_t reg034 = reg003 +reg004;
					reg8 = img_t2[y+3]*filt[8];
					res[x][y]+=  res1;
					data_t res100 = reg012+reg034;
					res[x][y+1]+=  res100 + reg8;
					//res[x][y]+=  res1;
				}
				
				//Shift up : LB2-->LB1 , LB3-->LB2,
				//Unrolled 2 times
				for(int y=1; y<(dim_t-1); y+=2)
				{
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline
					img_t0[y] = img_t1[y];//LB2-->LB1
					img_t1[y] = img_t2[y];//LB3-->LB2
					img_t0[y+1] = img_t1[y+1];
					img_t1[y+1] = img_t2[y+1];
				}
			}
			//LAST ITERATION, the shift ups for 1st and 2nd rows are completed above
			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline
				img_t2[y] = 0;//the last line of the input image must consists of zeros(zero padding==1)

			for(int y=0; y<o_dim; y+=2)
			{
#pragma HLS loop_tripcount min=16 max=16
#pragma HLS pipeline

				data_t reg0 = img_t0[y]*filt[0];
				data_t reg1 = img_t0[y+1]*filt[1];
				data_t reg01 = reg0+reg1;
				data_t reg2 = img_t0[y+2]*filt[2];
				data_t reg3 = img_t1[y]*filt[3];
				data_t reg02= reg2+reg3;
				data_t reg4 = img_t1[y+1]*filt[4];
				data_t reg5 = img_t1[y+2]*filt[5];
				reg01 = reg01 + reg02;
				data_t reg03= reg4+reg5;
				data_t reg6 = img_t2[y]*filt[6];
				data_t reg7 = img_t2[y+1]*filt[7];
				data_t reg04= reg6+reg7;
				data_t reg8 = img_t2[y+2]*filt[8];
				reg0 = img_t0[y+1]*filt[0];
				reg01 = reg01+reg03;
				reg1 = img_t0[y+2]*filt[1];
				data_t reg001 = reg0 +reg1;
				reg2 = img_t0[y+3]*filt[2];
				data_t res1 =reg04+reg8;
				reg3 = img_t1[y+1]*filt[3];
				data_t reg002 = reg2 + reg3;
				reg4 = img_t1[y+2]*filt[4];
				reg5 = img_t1[y+3]*filt[5];
				data_t reg012 = reg001+reg002;
				data_t reg003 =reg4+reg5;
				reg6 =img_t2[y+1]*filt[6];
				res1 = res1 + reg01;
				reg7 = img_t2[y+2]*filt[7];
				data_t reg004 = reg6+reg7;
				data_t reg034 = reg003 +reg004;
				reg8 = img_t2[y+3]*filt[8];
				res[o_dim -1][y]+=  res1;
				data_t res100 = reg012+reg034;
				res[o_dim -1][y+1]+=  res100 + reg8;

			}
			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline
				img_t0[y] = 0;
		}//end of channel loop
		struct_t res_data;
		//Streaming out the result per output channel == filter
		for(int x=0; x<o_dim; x++){
#pragma HLS loop_tripcount min=32 max=32
#pragma HLS pipeline
			for(int y=0; y<o_dim; y+=2)
			{
#pragma HLS loop_tripcount min=32 max=32
				res_data.v1=res[x][y];
				res_data.v2=res[x][y+1];
				if(res_data.v1<=0) 			// Integraded ReLu activation function
					res_data.v1=0;
				if(res_data.v2<=0) 			// Integraded ReLu activation function
					res_data.v2=0;
				result.write(res_data);
			}
		}

	}

	return;

}
