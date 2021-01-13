#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include <stdint.h>

using namespace hls;

#include "my_ip_hls.hpp"



int main() {


	int ch=4;
	int dim = 8;
	//float *img=(float *)malloc(ch*dim*dim*sizeof(float));
	//float *img=(float *)malloc(ch*dim*dim*sizeof(float)); !!! SIGSERV ERROR DURING BIG DATA TESTING !!!
	float img[ch*dim*dim];
	for(int c=0; c<ch ; c++)
		for(int i=0;i<dim;i++)
			for(int j=0;j<dim;j++)
				img[c*dim*dim+i*dim+j]= (i+1)*(j*2+1)*1.3 + c;

	printf("Before SEND:\n");
	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j++)
			{
				printf("%f\t",img[c*dim*dim+i*dim+j]);
			}
			printf("\n");
		}
		printf("\n");
	}

	data_d data_in;
	data_o data_out;
	stream<data_d> image("Image");
	stream<data_o> result("Result");

	data dataIn;

	dataIn.ch = ch;
	dataIn.dim = dim;
	//dataIn.image = (float *)img;


	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j+=16)
			{
				data_in.v1 = img[c*dim*dim+i*dim+j];
				data_in.v2 = img[c*dim*dim+i*dim+j+1];
				data_in.v3 = img[c*dim*dim+i*dim+j+2];
				data_in.v4 = img[c*dim*dim+i*dim+j+3];
				data_in.v5 = img[c*dim*dim+i*dim+j+4];
				data_in.v6 = img[c*dim*dim+i*dim+j+5];
				data_in.v7 = img[c*dim*dim+i*dim+j+6];
				data_in.v8 = img[c*dim*dim+i*dim+j+7];

				data_in.v12 = img[c*dim*dim+i*dim+j+8];
				data_in.v22 = img[c*dim*dim+i*dim+j+9];
				data_in.v32 = img[c*dim*dim+i*dim+j+10];
				data_in.v42 = img[c*dim*dim+i*dim+j+11];
				data_in.v52 = img[c*dim*dim+i*dim+j+12];
				data_in.v62 = img[c*dim*dim+i*dim+j+13];
				data_in.v72 = img[c*dim*dim+i*dim+j+14];
				data_in.v82 = img[c*dim*dim+i*dim+j+15];
				image.write(data_in);
				//image.write(data_t(img[c*dim*dim+i*dim+j]));
			}
		}
	}



	my_ip_hls(image,result,dataIn);


	//allocate space for the result(known result dimensions)
	int o_dim = dim/2;
	int o_ch = ch;
	//data_out res[o_ch*o_dim*o_dim];
	//data_out *res=(data_out *)malloc(o_ch*o_dim*o_dim*sizeof(data_out));
	float res[o_ch*o_dim*o_dim];
	////////////////////////////////////
	printf("After SEND:\n");
	for(int c=0; c < o_ch ; c++)
	{
		for(int i=0;i<o_dim;i++)
		{
			for(int j=0;j<o_dim;j+=8)
			{
				data_out=result.read();
				res[c*o_dim*o_dim + i*o_dim*+j]=  data_out.v1;
				res[c*o_dim*o_dim + i*o_dim*+j+1]=data_out.v2;
				res[c*o_dim*o_dim + i*o_dim*+j+2]=data_out.v3;
				res[c*o_dim*o_dim + i*o_dim*+j+3]=data_out.v4;

				res[c*o_dim*o_dim + i*o_dim*+j+4]=  data_out.v5;
				res[c*o_dim*o_dim + i*o_dim*+j+5]=data_out.v6;
				res[c*o_dim*o_dim + i*o_dim*+j+6]=data_out.v7;
				res[c*o_dim*o_dim + i*o_dim*+j+7]=data_out.v8;

				//res[c*o_dim*o_dim + i*o_dim*+j]=result.read();
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j]);
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j+1]);
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j+2]);
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j+3]);
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j+4]);
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j+5]);
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j+6]);
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j+7]);
			}
			printf("\n");
		}
		printf("\n");
	}


	return 0;
}
