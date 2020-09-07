#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include <stdint.h>

using namespace hls;

#include "my_ip_hls.hpp"



int main() {


	int ch=16;
	int dim = 32;
	//float *img=(float *)malloc(ch*dim*dim*sizeof(float));
	//float *img=(float *)malloc(ch*dim*dim*sizeof(float)); SIGSERV ERROR DURING BIG DATA TESTING
	float img[ch*dim*dim];
	for(int c=0; c<ch ; c++)
		for(int i=0;i<dim;i++)
			for(int j=0;j<dim;j++)
				img[c*dim*dim+i*dim+j]= (i+1)*(j*2+1)*1.3 + c);

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
	stream<data_out> result("Result");


	//dataIn.ch = ch;
	//dataIn.dim = dim;
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



	//slaveIn.write(dataIn);



	my_ip_hls(image,result,ch,dim);


	//allocate space for the result(known result dimensions)
	int o_dim = dim/2;
	int o_ch = ch;
	//data_out res[o_ch*o_dim*o_dim];
	//data_out *res=(data_out *)malloc(o_ch*o_dim*o_dim*sizeof(data_out));
	data_out res[o_ch*o_dim*o_dim];
	////////////////////////////////////
	printf("After SEND:\n");
	for(int c=0; c < o_ch ; c++)
	{
		for(int i=0;i<o_dim;i++)
		{
			for(int j=0;j<o_dim;j++)
			{
				res[c*o_dim*o_dim + i*o_dim*+j]=result.read();
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim*+j].data);
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
