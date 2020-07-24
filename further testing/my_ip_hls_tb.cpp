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
	int dim = 4;
	float ***img= (float ***)malloc(ch*sizeof(float**));
	for (i = 0; i< ch; i++)
	{
		img[i] = (float **) malloc(dim*sizeof(float *));
		for (int j = 0; j < dim; j++)
			img[i][j] = (float *)malloc(dim*sizeof(float));
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


	stream<image> slaveIn("slaveIn");
	stream<image> masterOut("masterOut");

	image dataIn = {0,0,0};
	dataIn.ch = ch;
	dataIn.dim = dim;
	dataIn.input = img;

	slaveIn.write(dataIn);

	my_ip_hls(slaveIn, masterOut);

	float ***ret;
	//if (!masterOut.empty()) {
	image dataOut = {0,0,0};
	masterOut.read(dataOut);

	ret= (float ***)malloc(ch*sizeof(float**));
	for (i = 0; i< ch; i++)
	{
		ret[i] = (float **) malloc(dim*sizeof(float *));
		for (int j = 0; j < dim; j++)
			ret[i][j] = (float *)malloc(dim*sizeof(float));
	}
	memcpy(ret,dataOut.input , dim*dim*sizeof(float)); //WARNING,IF I DO memcpy(tmp,res) first item(0,0) will fail

	//printf("%d: read data: %u\n",(int)i, (int)dataOut.data);

	printf("After Processing:\n",dataOut.ch, dataOut.dim, dataOut.dim);
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
	return 0;
}
