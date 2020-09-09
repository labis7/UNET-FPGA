/*
 ============================================================================
 Name        : Utilities.c
 Author      : Labiz
 Version     : V0.5
 Copyright   : Your copyright notice
 Description : Utilities of U-Net, including initializations and important 3d,4d matrix building tools
 ============================================================================
 */
#include <stdlib.h>
#include <header.h>
//#include <time.h>

////////////// Function Naming init. ////////////////

////////////////////////////////////////////////////



void Activation_Function(struct act_func_data_ *act_func_data)
{
	float ***Z;
	int code, channels, dim;
	Z = act_func_data->Z;
	code = act_func_data->code;
	channels = act_func_data->channels;
	dim = act_func_data->dim;

	float ***res = make_3darray(channels, dim);
	//Approximation of Sigmoid,  f(x) = x / (1 + abs(x))
	if(code == 1)
	{
		for (int i=0; i<channels; i++)
			for (int j=0; j<dim; j++)
				for (int k=0; k<dim; k++)
					res[i][j][k] = (1/(1 + exp(- Z[i][j][k]))); //Thats the original sigmoid!
	}
	else if(code == 2)//Sigmoid backpropagation function
	{
		float ***dA;
		dA =act_func_data->res;
		for (int i=0; i<channels; i++)
			for (int j=0; j<dim; j++)
				for (int k=0; k<dim; k++)
					res[i][j][k] = (1/(1 + exp(- Z[i][j][k])));//find sig
		for (int i=0; i<channels; i++)
			for (int j=0; j<dim; j++)
				for (int k=0; k<dim; k++)
					res[i][j][k] = (dA[i][j][k])*(res[i][j][k])*(1 - res[i][j][k]);
	}
	else if(code == 3) //RELU activation function
	{
		int z =0;
		for (z=0; z<channels; z++)
		{
			for (int j=0; j<dim; j++)
			{
				for (int k=0; k<dim; k++)
				{
					if(Z[z][j][k] <= 0)
						res[z][j][k] = 0;
					else
						res[z][j][k] = Z[z][j][k];
				}
			}
		}
	}
	else //Relu backpropagation function
	{
		float ***dA;
		dA = act_func_data->res;
		int z=0;
		for (z=0; z<channels; z++)
		{
			for (int j=0; j<dim; j++)
			{
				for (int k=0; k<dim; k++)
				{
					if(Z[z][j][k] <= 0)
						res[z][j][k] = 0;
					else
						res[z][j][k] = dA[z][j][k];
				}
			}
		}
	}
	act_func_data->res = res;
	//return res;
}

float Dice_Coef(float ***logs, float ***target,int dim)
{

	//int mylen = dim*dim;
	//building numerator(logs*target)  //#TODO :many optimization available
	float ***numer = make_3darray(1,dim);
	float ***denom = make_3darray(1,dim);
	for(int i=0; i<1; i++)
		for(int x=0; x<dim; x++)
			for(int y=0; y<dim; y++)
			{
				numer[i][x][y]=(logs[i][x][y])*(target[i][x][y]);
				denom[i][x][y] = logs[i][x][y] + target[i][x][y];
			}
	float sum_num=0,sum_den=0;
	for(int i=0; i<1; i++)
		for(int x=0; x<dim; x++)
			for(int y=0; y<dim; y++)
			{
				sum_num += numer[i][x][y];
				sum_den += denom[i][x][y];
			}
	float loss = 1 - (float)((2*sum_num)/(sum_den));
	return (float)exp(-loss);

}



float **make_2darray(int dim1,int dim2)
{
	float **array = (float **)malloc(dim1*sizeof(float*));
	for (int i = 0; i< dim1; i++)
		array[i] = (float *) malloc(dim2*sizeof(float));
	return array;
}
float ***make_3darray(int channels,int dim)
{
	int dim1=channels, dim2=dim, dim3=dim;
	int i,j;
	float *** array = (float ***)malloc(dim1*sizeof(float**));

	for (i = 0; i< dim1; i++)
	{
		array[i] = (float **) malloc(dim2*sizeof(float *));
		for (j = 0; j < dim2; j++)
			array[i][j] = (float *)malloc(dim3*sizeof(float));
	}
	return array;
}
uint32_t ***make_3darray_u(int channels,int dim)
{
	int dim1=channels, dim2=dim, dim3=dim;
	int i,j;
	uint32_t *** array = (uint32_t ***)malloc(dim1*sizeof(uint32_t**));

	for (i = 0; i< dim1; i++)
	{
		array[i] = (uint32_t **) malloc(dim2*sizeof(uint32_t *));
		for (j = 0; j < dim2; j++)
			array[i][j] = (uint32_t *)malloc(dim3*sizeof(uint32_t));
	}
	return array;
}
float ****make_4darray(int num,int channels,int dim)
{
	int dim0=num, dim1=channels, dim2=dim, dim3=dim;
	int i,j,k;
	float **** array = (float ****)malloc(dim0*sizeof(float***));

	for (i = 0; i< dim0; i++)
	{
		array[i] = (float ***) malloc(dim1*sizeof(float **));
		for (j = 0; j < dim1; j++) {
			array[i][j] = (float **)malloc(dim2*sizeof(float *));
			for (k = 0; k < dim2; k++)
				array[i][j][k] = (float *)malloc(dim3*sizeof(float));
		}
	}
	return array;
}



void normalize_custom(struct norm_data_ *norm_data)
{
	float ***image = norm_data->image;
	int dim = norm_data->dim;
	//float ***res = make_3darray(1,dim,dim);
	int code = norm_data->code;
	if(code == 0) //normalize possible inf edges
	{
		for (int i=0; i<dim; i++)
		{
			for(int j=0; j<dim; j++)
			{
				if(image[0][i][j]>4)
					image[0][i][j]=4;
				else if(image[0][i][j]<-4)
					image[0][i][j]=-4;
			}
		}
	}
	else
	{
		for (int i=0; i<dim; i++)
		{
			for(int j=0; j<dim; j++)
			{
				if(image[0][i][j]>0.65)
					image[0][i][j]=1;
				else if(image[0][i][j]<0.35)
					image[0][i][j]=0;
			}
		}
	}
}

int calc_f_num(int layer)
{
	//int f_num_init=16; //'always'
	//*layer variable starts from 1
	switch (layer)
	{
	    case 1:
	      return 16;
	    case 2:
	      return 32;
	    case 3:
	    	return 64;
	    case 4:
	    	return 128;
	    case 5:
	    	return 256;
	    case 6:
	    	return 128;
	    case 7:
	    	return 64;
	    case 8:
	    	return 32;
	    case 9:
	    	return 16;
	    case 10:
	    	return 1;
	    default:
	    	return -1;
	      // default statements
	}



}
int calc_ch_num(int layer,int tuple)
{
	//int f_num_init=16; //'always'
	//*layer,tuple variable starts from 1 , tuple ={1,2} its the 1st or second filter/image channel of the layer
	switch (layer)
	{
	    case 1:
	      return (1+(tuple-1)*15);//1 , 16
	    case 2:
	      return 16 +(tuple-1)*16; //16, 32
	    case 3:
	    	return 32 +(tuple-1)*32;//32, 64
	    case 4:
	    	return 64 + (tuple -1)*64; //64, 128
	    case 5:
	    	return 128 + (tuple-1)*128; //128, 256
	    case 6:
	    	return 256 -(tuple-1)*128; //256(after concat), 128
	    case 7:
	    	return 128 - (tuple-1)*64; //128, 64
	    case 8:
	    	return 64 - (tuple -1)*32; //64, 32
	    case 9:
	    	return 32 - (tuple -1)*16;//32, 16
	    case 10:
	    	return 16;
	    default:
	    	return -1;
	      // default statements
	}
}





//////  Maxpool ///////
XMy_ip_hls my_ip_hls;
XMy_ip_hls_Config *my_ip_hls_cfg;

XAxiDma axiDMA6;
XAxiDma_Config *axiDMA6_cfg;
///////////////////////


//////  Conv ///////
XConv conv_ip;
XConv_Config *conv_ip_cfg;

XAxiDma axiDMA0;
XAxiDma_Config *axiDMA0_cfg;

XAxiDma axiDMA1;
XAxiDma_Config *axiDMA1_cfg;

XAxiDma axiDMA2;
XAxiDma_Config *axiDMA2_cfg;
///////////////////////

//////  Tconv ///////
XTconv tconv_ip;
XTconv_Config *tconv_ip_cfg;

XAxiDma axiDMA3;
XAxiDma_Config *axiDMA3_cfg;

XAxiDma axiDMA4;
XAxiDma_Config *axiDMA4_cfg;

XAxiDma axiDMA5;
XAxiDma_Config *axiDMA_cfg;
///////////////////////


///////////////// MEMORY MAPPING ///////////////////
//Map memory so IPs can read from the stick ram0 and write to stick ram1
//same for arm, which is able to read from 2 sticks at the same time
//(reading from stick 0 in order to send data to IP and read results from stick 1)
#define SW_BASE 0x50000000
#define img_addr SW_BASE
#define filt_addr (SW_BASE+0x00400000)
#define bias_addr (filt_addr+0x00400000)
#define res_addr (bias_addr +0x00008000)
#define res_sw_addr (res_addr + 0x00400000)
#define img_t_addr (res_sw_addr + 0x00400000)

#define TX_BUFFER_BASE (0xD0000000 + 0x00000000)
#define RX_BUFFER_BASE (0xD0000000 + 0x00400000)
////////////////////////////////////////////////////


void init_dma(){

	//initialize AxiDMA core
	printf("Initializing AxiDMAs . . .\n");
	axiDMA0_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
	if(axiDMA0_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA0, axiDMA0_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	//printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA0, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA0, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	//printf("Resetting  DMA 0 . . .\n");
	XAxiDma_Reset(&axiDMA0);
	while(!XAxiDma_ResetIsDone(&axiDMA0)){}
	///////////////////////////////////////////////////////////////////////////////////////////

	//initialize AxiDMA core
	//printf("Initializing AxiDMA . . .\n");
	axiDMA1_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_1_DEVICE_ID);
	if(axiDMA1_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA1, axiDMA1_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	//printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA1, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA1, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	//printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA1);
	while(!XAxiDma_ResetIsDone(&axiDMA1)){}

	///////////////////////////////////////////////////////////////////////////////////////
	//initialize AxiDMA core
	//printf("Initializing AxiDMA . . .\n");
	axiDMA2_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_2_DEVICE_ID);
	if(axiDMA2_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA2, axiDMA2_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	//printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA2, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA2, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	//printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA2);
	while(!XAxiDma_ResetIsDone(&axiDMA2)){}

	///////////////////////////////////////////////////////////////////////////////////////
	//printf("Initializing AxiDMA . . .\n");
	axiDMA3_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_3_DEVICE_ID);
	if(axiDMA3_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA3, axiDMA3_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	//printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA3, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA3, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	//printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA3);
	while(!XAxiDma_ResetIsDone(&axiDMA3)){}

	///////////////////////////////////////////////////////////////////////////////////////
	//printf("Initializing AxiDMA . . .\n");
	axiDMA4_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_4_DEVICE_ID);
	if(axiDMA4_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA4, axiDMA4_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	//printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA4, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA4, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	//printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA4);
	while(!XAxiDma_ResetIsDone(&axiDMA4)){}

	///////////////////////////////////////////////////////////////////////////////////////
	//printf("Initializing AxiDMA . . .\n");
	axiDMA5_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_5_DEVICE_ID);
	if(axiDMA5_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA5, axiDMA5_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	//printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA5, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA5, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	//printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA5);
	while(!XAxiDma_ResetIsDone(&axiDMA5)){}

	///////////////////////////////////////////////////////////////////////////////////////
	//printf("Initializing AxiDMA . . .\n");
	axiDMA6_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_6_DEVICE_ID);
	if(axiDMA6_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA6, axiDMA6_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	//printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA6, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA6, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	//printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA6);
	while(!XAxiDma_ResetIsDone(&axiDMA6)){}

	printf("Done!\n");
	return;
}



void setupIPs()
{
	printf("Initializing IPs . . .\n");
	//initialize maxpool
	my_ip_hls_cfg = XMy_ip_hls_LookupConfig(XPAR_MY_IP_HLS_0_DEVICE_ID);
	if(my_ip_hls_cfg)
	{
		int status = XMy_ip_hls_CfgInitialize(&my_ip_hls, my_ip_hls_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing My_ip_hls core\n");
	}

	conv_ip_cfg = XConv_LookupConfig(XPAR_CONV_0_DEVICE_ID);
	if(conv_ip_cfg)
	{
		int status = XConv_CfgInitialize(&conv_ip, conv_ip_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing conv_ip core\n");
	}

	tconv_ip_cfg = XTconv_LookupConfig(XPAR_TCONV_0_DEVICE_ID);
	if(conv_ip_cfg)
	{
		int status = XTconv_CfgInitialize(&tconv_ip, tconv_ip_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing Tconv_ip core\n");
	}

	printf("Done!!!\n");
	return;

}

int main(void) {
	//time_t t;
	//srand((unsigned) time(&t));

	puts("Main function init!");

	/*
	int channels,code,dim;
	code = 1;//sigmoid(approximation)
	channels = 2;
	dim = 5;

	float ***image1;
	image1 = make_3darray(channels,dim);
	for (int i=0;i<channels;i++)
	{
		for (int j=0;j<dim;j++)
		{
			for (int k=0;k<dim;k++)
			{
				image1[i][j][k] = (i+1)*(j*2+k*1) +1;
				//printf("%f\t", image1[i][j][k]);//*(*(*(pA +i) + j) +k));
			}
			//printf("\n");
		}
		//printf("\n");
	}
	testff(image1);
	//struct images_data_ images_data;
	*/
	//struct images_data_ *ptr_images_data, images_data;// = &images_data;
	//ptr_images_data = &images_data;
	return 0;
	///////////////////////////////////////////////////////
	////////////////// TESTING SECTION ////////////////////
	/*

	//load images/labels//
	ptr_images_data->dim = 64;
	ptr_images_data->im_num = 4;
	load_images(ptr_images_data);
	load_labels(ptr_images_data);
	//////////////////////
	struct params_ *ptr_params = &params;
	// load pre-trained parameters (filters-bias-GN)//
	ptr_params->gn_batch = 2;//number of group normalization batch
	ptr_params->layers = 10; //number of total layers
	ptr_params->num_f =16;   //init number of filters
	//ptr_params->num_f = 16; it will be calculated itself
	load_params(ptr_params);
	//////////////////////////////////////////////////

	clock_t begin = clock();




	//PREDICT
	predict(ptr_images_data, ptr_params);//last variable is prediction image number,we choose the img we want
	//#TODO:future update will be able to choose the limit of predicted images of the struct(1 more variable that will give that info)
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\nTime needed: %.2f sec",time_spent);
	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////
	printf("\nSuccess! - No Segmentation faults!");
	*/
	return EXIT_SUCCESS;
}

float *****testff(float ***t){
	int channels =2;
	int dim=5;
	for (int i=0;i<channels;i++)
	{
		for (int j=0;j<dim;j++)
		{
			for (int k=0;k<dim;k++)
			{
				//image1[i][j][k] = (i+1)*(j*2+k*1) +1;
				printf("%f\t", t[i][j][k]);//*(*(*(pA +i) + j) +k));
			}
			printf("\n");
		}
		printf("\n");
	}

	return 0;
	/*
	float ****t2 = make_4darray(1,2,3);
	for (int i=0;i<1;i++)
		for (int j=0;j<2;j++)
			for (int k=0;k<3;k++){
				for(int y=0;y<3;y++)
				{
					t[i][j][k][y]=i+j+y+k;
					t2[i][j][k][y]=2*i+2*j+2*y+2*k;
				}
			}
	float *****t_array = (float *****)malloc(2*sizeof(float ****));
	t_array[0] = t;
	t_array[1] = t2;
	return t_array;
	*/
}
