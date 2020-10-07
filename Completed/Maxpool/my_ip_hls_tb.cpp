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


	stream<float> image("Image");
	stream<float> result("Result");

	data dataIn;

	dataIn.ch = ch;
	dataIn.dim = dim;
	//dataIn.image = (float *)img;


	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j++)
			{
				image.write(img[c*dim*dim+i*dim+j]);
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
			for(int j=0;j<o_dim;j++)
			{
				res[c*o_dim*o_dim + i*o_dim*+j]=result.read();
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j]);
			}
			printf("\n");
		}
		printf("\n");
	}


	return 0;
}
