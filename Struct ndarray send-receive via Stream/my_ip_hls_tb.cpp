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
	int dim1 = 3;
	int dim2 = 3;
	float **tmp = (float **)malloc(dim1*sizeof(float*));
	for(int i=0;i<dim1;i++)
	{
		tmp[i] = (float *) malloc(dim2*sizeof(float));
	}
	for(int i=0;i<dim1;i++)
		for(int j=0;j<dim2;j++)
			tmp[i][j] = (i+1)*(j*2+1)*1.3;
	printf("Before SEND:\n");
	for(int i=0;i<dim1;i++)
	{
		for(int j=0;j<dim2;j++)
		{
			printf("%f\t",tmp[i][j]);
		}
		printf("\n");
	}


	stream<axiWord> slaveIn("slaveIn");
	stream<axiWord> masterOut("masterOut");



	axiWord dataIn = {0,0,0};
	dataIn.input = tmp;
	dataIn.data = i+1;
	dataIn.strb = 0b1111;
	if (i == STREAM_TEST_ITERATIONS-1)
		dataIn.last = 1;
	else
		dataIn.last = 0;
	slaveIn.write(dataIn);

	rule1=20;
	rule2=50;
	int arr[5] = {10, 20, 30, 40, 50};

//		my_ip_hls(slaveIn, masterOut, rule1,rule2);
	my_ip_hls(slaveIn, masterOut, tmp);
	float **tmp1;
	if (!masterOut.empty()) {
		axiWord dataOut = {0,0,0};
		masterOut.read(dataOut);
		float **res= dataOut.input;
		tmp1=(float **)malloc(dim1*sizeof(float *));
		for (int i = 0; i< dim1; i++)
			tmp1[i] = (float *) malloc(dim1*sizeof(float));
		memcpy(tmp1,res, dim1*dim2*sizeof(float)); //WARNINGM,IF I DO memcpy(tmp,res) first item(0,0) will fail

		//printf("%d: read data: %u\n",(int)i, (int)dataOut.data);
	}
	printf("After Processing:\n");
	for(int i=0;i<dim1;i++)
	{
		for(int j=0;j<dim2;j++)
		{
			printf("%f\t",tmp1[i][j]);
		}
		printf("\n");
	}

	//printf("\n%d\n",(int)count_out);

	return 0;
}
