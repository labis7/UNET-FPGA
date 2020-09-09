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

////////////// DEF ////////////////
////////////////////////
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
XAxiDma_Config *axiDMA5_cfg;
///////////////////////
////////////////////////////////////////////////////
float Random_Normal(int loc, float scale)
{
   // return a normally distributed random value
	scale = 1;
	loc=0;
	float v1 = ( (float)(rand()) + 1. )/( (float)(RAND_MAX) + 1. ); //random gen
	float v2 = ( (float)(rand()) + 1. )/( (float)(RAND_MAX) + 1. );//random gen
	return ((cos(2*3.14*v2)*sqrt(-2.*log(v1)))*scale + loc);
}


void Initialize_Parameters(struct init_param_ *ptr_init_param)
{
	int layers = ptr_init_param->layers;//number of layers(just the encoder-downsampling number)
	int num_f = ptr_init_param->num_f;  //initial number of filters(they will be doubled for each layer)
	float trim = ptr_init_param->trim;  //weight scale of the values
	float **filters = (float **)malloc((layers*2*2-1)*sizeof(float *));
	float **bias = (float **)malloc((layers*2*2-1)*sizeof(float *));
	float **f_dc = (float **)malloc((layers-1)*sizeof(float *));
	float **b_dc = (float **)malloc((layers-1)*sizeof(float *));

	int ch_in = 1; //number of initial channels input from the input image
	int loc = 0; //normal distribution around zero

	int last_pos=0;
	//Building the downsampling/encoder filters first.(5 layers*2 filters each)
	for (int i=0; i<layers; i++)
	{
		if(i != 0) //after 1st iteration, double the amount of filters of each layer
			num_f = num_f*2; //*16*, 32, 64, 128, 256
		//Building f1
		float *f1 = (float *)malloc(num_f*ch_in*3*3*sizeof(float));//make_4darray(num_f, ch_in, 3);//3x3 filter alwars for the simple convolution
		float *b1 = (float *)malloc(num_f*sizeof(float));
		for (int x=0;x<num_f;x++)
		{
			for (int y=0;y<ch_in;y++)
				for (int z=0;z<3;z++)
					for(int w=0;w<3;w++)
					{
						f1[x*num_f*9 + y*9 + z*3 + w] = Random_Normal(loc, trim);
					}
			b1[x] =  Random_Normal(loc, trim);
		}

		/*
		 * Very important! Next filter will have as input the output/channels of the previous filter
		 * More : When we apply a number of filters on an image, we create num_f channels on the image result.
		 * So each filter must have input dim same as the input image channels, output dim = channels we want to occur after conv.
		 */
		ch_in = num_f;

		float *f2 =(float *)malloc(num_f*ch_in*3*3*sizeof(float));// make_4darray(num_f, ch_in, 3);//3x3 filter alwars for the simple convolution
		float *b2 = (float *)malloc(num_f*sizeof(float));
		for (int x=0;x<num_f;x++)
		{
			for (int y=0;y<ch_in;y++)
				for (int z=0;z<3;z++)
					for(int w=0;w<3;w++)
					{
						f2[x*num_f*9 + y*9 + z*3 + w] = Random_Normal(loc, trim);
					}
			b2[x] = Random_Normal(loc, trim);
		}
		filters[2*i] = f1;
		filters[2*i +1] = f2;
		bias[2*i] = b1;
		bias[2*i +1] = b2;
		last_pos++; //last position of the i that shows the current layer, we will need it later so we keep saving filters and
		//bias sequencially.
	}
	for (int i = 1 ; i < layers; i++)
	{
		num_f = (int)num_f/2;//It will be always power of number of filters,so the division will give back integer

		float *fdc = (float *)malloc(num_f*ch_in*2*2*sizeof(float));//make_4darray(num_f, ch_in, 2);//2x2 filter always for the upsampling/transposed convolution
		float *bdc = (float *)malloc(num_f*sizeof(float));
		float *f1 =(float *)malloc(num_f*ch_in*3*3*sizeof(float));// make_4darray(num_f, ch_in, 3);//3x3 filter always for the simple convolution
		float *b1 = (float *)malloc(num_f*sizeof(float));
		for (int x=0;x<num_f;x++)
		{
			for (int y=0;y<ch_in;y++)
			{
				for (int z=0;z<3;z++)
				{
					for(int w=0;w<3;w++)
						f1[x*num_f*9 + y*9 + z*3 + w]= Random_Normal(loc, trim);
				}
				for (int z=0;z<2;z++)
				{
					for(int w=0;w<2;w++)
						fdc[x*num_f*4 + y*4 + z*2 + w] = Random_Normal(loc, trim);
				}
			}
			bdc[x] =  Random_Normal(loc, trim);
			b1[x] = Random_Normal(loc, trim);
		}

		ch_in = num_f ;

		float *f2 = (float *)malloc(num_f*ch_in*3*3*sizeof(float));//make_4darray(num_f, ch_in, 3);//3x3 filter always for the simple convolution
		float *b2 = (float *)malloc(num_f*sizeof(float));
		for (int x=0;x<num_f;x++)
		{
			for (int y=0;y<ch_in;y++)
				for (int z=0;z<3;z++)
					for(int w=0;w<3;w++)
					{
						f2[x*num_f*9 + y*9 + z*3 + w] = Random_Normal(loc, trim);
					}
			b2[x] =  Random_Normal(loc, trim);
		}

		filters[last_pos*2] = f1;
		filters[last_pos*2 + 1] = f2;
		bias[last_pos*2] = b1;
		bias[last_pos*2 + 1] = b2;
		f_dc[i-1] = fdc;
		b_dc[i] = bdc;
		last_pos++;
	}
	printf("\nlast_pos:%d",last_pos);
	//Now build the last one 1x1 conv filters
	num_f = 1; //we need 1 channels for the output(same as input)
	float *out_f = (float *)malloc(num_f*ch_in*1*1*sizeof(float));//make_4darray(num_f, ch_in, 1);//1x1 output filter
	float *out_b = (float *)malloc(num_f*sizeof(float));
	for (int x=0;x<num_f;x++)
	{
		for (int y=0;y<ch_in;y++)
			for (int z=0;z<1;z++)
				for(int w=0;w<1;w++)
				{
					out_f[x*num_f*1 + y*1 + z*1 + w] = Random_Normal(loc, trim);
				}
		out_b[x] =  Random_Normal(loc, trim);
	}
	filters[last_pos*2] = out_f;
	bias[last_pos*2] = out_b;

	ptr_init_param->filters = filters;
	ptr_init_param->bias = bias;
	ptr_init_param->f_dc = f_dc;
	ptr_init_param->b_dc = b_dc;
}

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

	puts("Initializing DMAs and IPs . . .\n");
	init_dma();
	setupIPs();
	printf("Initialization Completed!\n");
	printf("----------------------------------------------------------------\n");

	int ch_num,dim;
	ch_num = 1;
	dim = 64;


	float *img =(float *)malloc(dim*dim*sizeof(float));
	for (int i=0;i<ch_num; i++)
		for (int j=0;j<dim;j++)
			for (int k=0;k<dim;k++)
				img[i*ch_num*dim*dim +j*dim + k] = ((i+1)*(j*2+k*1)*1.3 +1)/(ch_num*dim*dim);

	struct init_param_ *ptr_init_param,init_param;
	ptr_init_param = &init_param;
	ptr_init_param->layers = 5;
	ptr_init_param->num_f=16;
	ptr_init_param->trim = 0.01;

	Initialize_Parameters(ptr_init_param);


	struct images_data_ *ptr_images_data, images_data;// = &images_data;
	ptr_images_data = &images_data;
	float **image =(float **)malloc(sizeof(float *));
	image[0] =img;
	ptr_images_data->dim=dim;
	ptr_images_data->im_num=1;
	ptr_images_data->images=image;


	struct params_ *ptr_params, params;
	ptr_params = &params;
	ptr_params->bias =ptr_init_param->bias;
	ptr_params->layers = 10;
	ptr_params->num_f = 16;
	ptr_params->filters=ptr_init_param->filters;
	ptr_params->f_dc = ptr_init_param->f_dc;

	predict(ptr_images_data, ptr_params);


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
