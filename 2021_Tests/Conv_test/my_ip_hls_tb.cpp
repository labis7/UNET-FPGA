#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include <stdint.h>

using namespace hls;

#include "my_ip_hls.hpp"


#define STREAM_TEST_ITERATIONS 1

int main() {


	int ch=4;
	int dim = 8;
	float *img=(float *)malloc(ch*dim*dim*sizeof(float));

	int f_num;
	f_num = 2;
	float *filt = (float *)malloc(f_num*ch*F_DIM*F_DIM*sizeof(float));
	float b[f_num];
	int counter=1;
	for(int k=0; k < f_num;k++)
	{
		for(int c=0; c < ch ; c++)
		{
			for(int i=0;i < F_DIM;i++)
			{
				for(int j=0;j < F_DIM;j++)
				{
					filt[k*ch*F_DIM*F_DIM+ c*F_DIM*F_DIM + i*F_DIM+j] = counter;
					counter++;
				}
			}
		}
	}


	for(int c=0; c<ch ; c++)
		for(int i=0;i<dim;i++)
			for(int j=0;j<dim;j++)
				img[c*dim*dim+i*dim+j] = (i+1)*(j*2+1)*1.3 + c;
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


	stream<data> slaveIn("slaveIn");
	//stream<data> masterOut("masterOut");
	stream<struct_t> image("Image");
	stream<struct_t> filter("Filter");
	stream<struct_t> result("Result");
	stream<float> bias("Bias");

	struct_t img_data,filt_data,res_data;

	data dataIn ;
	//dataIn.ch = ch;
	//dataIn.dim = dim;
	//dataIn.image = (float *)img;
	dataIn.ch = ch;
	dataIn.dim =dim;
	dataIn.f_num = f_num;
	//slaveIn.write(dataIn);

	for(int k=0; k < f_num;k++)
	{

		for(int c=0; c<ch ; c++)
		{
			for(int i=0;i<F_DIM;i++)
			{
				for(int j=0;j<F_DIM;j+=3)
				{
					filt_data.v1 =data_t(filt[k*ch*F_DIM*F_DIM+ c*F_DIM*F_DIM + i*F_DIM+j]);
					filt_data.v2 = data_t(filt[k*ch*F_DIM*F_DIM+ c*F_DIM*F_DIM + i*F_DIM+j+1]);
					filt_data.v3 = data_t(filt[k*ch*F_DIM*F_DIM+ c*F_DIM*F_DIM + i*F_DIM+j+2]);
					filter.write(filt_data);
				}
			}
		}


		for(int c=0; c<ch ; c++)
		{
			for(int i=0;i<dim;i++)
			{
				for(int j=0;j<dim;j+=2)
				{
					img_data.v1=  data_t(img[c*dim*dim+i*dim+j]);
					img_data.v2=  data_t(img[c*dim*dim+i*dim+j+1]);
					image.write(img_data);
				}
			}
		}

		bias.write(0);
	}
	/*
	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j++)
			{
				image.write(img[c][i][j]);
			}
		}
	}
	*/






	Conv(image,filter, bias,result,dataIn);


	//allocate space for the result(known result dimensions)
	int o_dim = dim;
	int o_ch = f_num;
	float res[o_ch][o_dim][o_dim];
	////////////////////////////////////
	printf("After SEND:\n");
	for(int c=0; c < o_ch ; c++)
	{
		for(int i=0;i<o_dim;i++)
		{
			for(int j=0;j<o_dim;j+=2)
			{
				res_data =result.read();
				res[c][i][j] = float(res_data.v1);
				res[c][i][j+1] = float(res_data.v2);
				printf("%f\t", res[c][i][j]);
				printf("%f\t", res[c][i][j+1]);
			}
			printf("\n");
		}
		printf("\n");
	}
	/*
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
	*/

	return 0;
}
