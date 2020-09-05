#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include <stdint.h>

using namespace hls;

#include "my_ip_hls.hpp"


#define STREAM_TEST_ITERATIONS 1

int main() {


	int ch=8;
	int dim = 16;
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
	stream<float> image("Image");
	stream<float> filter("Filter");
	stream<float> result("Result");
	stream<float> bias("Bias");

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
				for(int j=0;j<F_DIM;j++)
				{
					filter.write(filt[k*ch*F_DIM*F_DIM+ c*F_DIM*F_DIM + i*F_DIM+j]);
				}
			}
		}

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
	//slaveIn.write(dataIn);



	my_ip_hls(image,filter, bias,result,dataIn);


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
			for(int j=0;j<o_dim;j++)
			{
				result.read(res[c][i][j]);
				printf("%f\t", res[c][i][j]);
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

/*
	//if (!masterOut.empty()) {
	data dataOut ;
	masterOut.read(dataOut);
	float ret[ch][dim][dim];
	memcpy(ret,dataOut.image , ch*dim*dim*sizeof(float)); //WARNING,IF I DO memcpy(tmp,res) first item(0,0) will fail
	//printf("%d: read data: %u\n",(int)i, (int)dataOut.data);
	printf("\nAfter Processing, Channels:%d , Res: %dx%d\n",dataOut.ch,dataOut.dim,dataOut.dim);
	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j++)
			{
				printf("%f\t",ret[c][i][j]);
			}
			printf("\n");
		}
		printf("\n");
	}
	//printf("\n%d\n",(int)count_out);
	printf("\n\nNo segmentation Problems\nFinishing . . .");
	*/
	return 0;
}
