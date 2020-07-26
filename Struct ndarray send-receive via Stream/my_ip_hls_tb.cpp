#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include <stdint.h>

using namespace hls;

#include "my_ip_hls.hpp"


#define STREAM_TEST_ITERATIONS 1

int main() {

	uint32 i = 0;

	uint32 rule1,rule2 = 0;

	int ch=2;
	int dim = 2;
	float img[ch][dim][dim];
	/*
	float ***img= (float ***)malloc(ch*sizeof(float**));
	for (i = 0; i< ch; i++)
	{
		img[i] = (float **) malloc(dim*sizeof(float *));
		for (int j = 0; j < dim; j++)
			img[i][j] = (float *)malloc(dim*sizeof(float));
	}
	*/

	int f_num,f_dim;
	f_num = 2;
	f_dim=3;
	float filt[f_num][ch][f_dim][f_dim];
	int counter=1;
	for(int k=0; k < f_num;k++)
	{
		for(int c=0; c < ch ; c++)
		{
			for(int i=0;i < f_dim;i++)
			{
				for(int j=0;j < f_dim;j++)
				{
					filt[k][c][i][j] = counter;
					counter++;
				}
			}
		}
	}


	for(int c=0; c<ch ; c++)
		for(int i=0;i<dim;i++)
			for(int j=0;j<dim;j++)
				img[c][i][j] = (i+1)*(j*2+1)*1.3 + c;
	printf("Before SEND:\n");
	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j++)
			{
				printf("%f\t",img[c][i][j]);
			}
			printf("\n");
		}
		printf("\n");
	}


	stream<data> slaveIn("slaveIn");
	stream<data> masterOut("masterOut");
	stream<float> image("Image");
	stream<float> filter("Filter");
	stream<float> result("Filter");

	data dataIn ;
	//dataIn.ch = ch;
	//dataIn.dim = dim;
	//dataIn.image = (float *)img;
	dataIn.ch = ch;
	dataIn.dim =dim;
	dataIn.f_num = f_num;
	slaveIn.write(dataIn);
	for(int k=0; k < f_num;k++)
	{
		for(int c=0; c<ch ; c++)
		{
			for(int i=0;i<3;i++)
			{
				for(int j=0;j<3;j++)
				{
					filter.write(filt[k][c][i][j]);
				}
			}
		}
	}
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
	//slaveIn.write(dataIn);

	my_ip_hls(image,filter,result,slaveIn);

	for(int k=0; k < f_num;k++)
	{
		for(int c=0; c<ch ; c++)
		{
			for(int i=0;i<3;i++)
			{
				for(int j=0;j<3;j++)
				{
					result.read(filt[k][c][i][j]);
				}
			}
		}
	}
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
