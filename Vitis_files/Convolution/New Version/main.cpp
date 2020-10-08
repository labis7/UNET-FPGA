#include<stdio.h>
#include<xconv_hw.h>
#include<xconv.h>
#include<xparameters.h>
#include<xaxidma.h>
#include<math.h>


XConv conv;
XConv_Config *conv_cfg;
XAxiDma axiDMA0;
XAxiDma_Config *axiDMA0_cfg;

XAxiDma axiDMA1;
XAxiDma_Config *axiDMA1_cfg;

XAxiDma axiDMA2;
XAxiDma_Config *axiDMA2_cfg;


//DMA Addresses
//#define MEM_BASE_ADDR 0x01000000
//#define TX_BUFFER_BASE (MEM_BASE_ADDR + 0x00100000)
#define SW_BASE 0x00500000
#define img_addr SW_BASE
#define filt_addr (SW_BASE+0x00400000)
#define bias_addr (filt_addr+0x00400000)
#define res_addr (bias_addr +0x00008000)
#define res_sw_addr (res_addr + 0x00400000)
#define img_t_addr (res_sw_addr + 0x00400000)

#define TX_BUFFER_BASE (0x02000000 + 0x00000000)
#define RX_BUFFER_BASE (0x02000000 + 0x00400000)

//#define SIZE_ARR 10
//int inStreamData[SIZE_ARR];
//int outStreamData[SIZE_ARR];


void init_dma(){

	//initialize AxiDMA core
	printf("Initializing AxiDMA . . .\n");
	axiDMA0_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
	if(axiDMA0_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA0, axiDMA0_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA0, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA0, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA0);
	while(!XAxiDma_ResetIsDone(&axiDMA0)){}
	///////////////////////////////////////////////////////////////////////////////////////////

	//initialize AxiDMA core
	printf("Initializing AxiDMA . . .\n");
	axiDMA1_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_1_DEVICE_ID);
	if(axiDMA1_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA1, axiDMA1_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA1, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA1, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA1);
	while(!XAxiDma_ResetIsDone(&axiDMA1)){}

	///////////////////////////////////////////////////////////////////////////////////////
	//initialize AxiDMA core
	printf("Initializing AxiDMA . . .\n");
	axiDMA2_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_2_DEVICE_ID);
	if(axiDMA2_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA2, axiDMA2_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA2, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA2, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA2);
	while(!XAxiDma_ResetIsDone(&axiDMA2)){}

	return;
}


void setupIPs()
{
	//initialize core
	conv_cfg = XConv_LookupConfig(XPAR_CONV_0_DEVICE_ID);
	if(conv_cfg)
	{
		printf("Initializing conv . . .\n");

		int status = XConv_CfgInitialize(&conv, conv_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing conv core\n");
	}
	printf("Done!!!\n");

	return;

}

//struct data_out{
//	float data;
//	unsigned int tlast:1;
//};

#define F_DIM 3

int main()
{
	//Get pointers to DMA TX/RX addresses
	//int *m_dma_buffer_TX = (int *)TX_BUFFER_BASE;
	//int *m_dma_buffer_RX = (int *)RX_BUFFER_BASE;

	float *res = (float *)res_addr;
	int count=0;
	for(int lab=0;lab<5;lab++)
	{


		printf("Hello World!!!\n----------------------------------------------------------------\n");
		//printf("\nPlatform Initialization . . .\n");
		//init_platform();
		//printf("Platform Initialization: Done!\n");
		init_dma();
		printf("DMA Initialization: Done!\n");
		printf("IP initialization . . .\n");
		setupIPs();
		printf("IP Initialization : DONE\n");

		int ch=4;
		int dim = 8;
		int f_num =2;

		if(lab == 1)
		{
			ch=32;
			dim = 32;
			f_num =64;
		}
		if(lab == 2)
		{
			ch=128;
			dim = 8;
			f_num =256;
		}
		if(lab == 3)
		{
			ch=16;
			dim = 128;
			f_num =16;
		}
		if(lab == 4)
		{
			ch=64;
			dim = 64;
			f_num =32;
		}



		XConv_Set_slaveIn_ch(&conv, ch);
		XConv_Set_slaveIn_dim(&conv, dim);
		XConv_Set_slaveIn_f_num(&conv, f_num);

		XConv_Start(&conv);
		printf("Setting up via Axi Lite is Completed!\n");

		printf("----------------------------------------------------------------\n");


		int o_dim = dim;
		int o_ch =f_num;

		//float res[o_ch*o_dim*o_dim];
		//Do the stream calculation
		//float *img=(float *)malloc(ch*dim*dim*sizeof(float));
		printf("Creating DATA STRUCTURES ... \n");
		float *img = (float *)malloc(ch*dim*dim*sizeof(float));
		//float img[ch*dim*dim];
		//float *img = (float *)img_addr;

		printf("Creating DATA STRUCTURES ... \n");
		float *filt = (float *)malloc(f_num*ch*F_DIM*F_DIM*sizeof(float));
		//float filt[f_num*ch*F_DIM*F_DIM];
		//float *filt =(float *)filt_addr;

		//Display Data

		//data_out *res=(data_out *)malloc(o_ch*o_dim*o_dim*sizeof(data_out));


		float b[f_num];
		//float *b = (float *)bias_addr;
		int counter=1;
		for(int k=0; k < f_num;k++)
		{
			for(int c=0; c < ch ; c++)
			{
				for(int i=0;i < F_DIM;i++)
				{
					for(int j=0;j < F_DIM;j++)
					{
						filt[k*ch*F_DIM*F_DIM+ c*F_DIM*F_DIM + i*F_DIM+j] = counter/f_num;
						counter++;
					}
				}
			}
			b[k]=0;
		}
		for(int c=0; c<ch ; c++)
			for(int i=0;i<dim;i++)
				for(int j=0;j<dim;j++)
					img[c*dim*dim+i*dim+j]= ((i+1)*(j*2+1)*1.3 + c)/100000; //*dim*dim+i*dim+j]= (i+1)*(j*2+1)*1.3 + c;
		/*
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

		*/


		//float res[o_ch*o_dim*o_dim];
		//float res[o_ch][o_dim][o_dim]; dma hang WARNING!!!!



		//////////

		////////////

		//Flush the cache of the buffers
		printf("Flushing Cache\n");
		Xil_DCacheFlushRange((UINTPTR)img, ch*dim*dim*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)filt, f_num*ch*F_DIM*F_DIM*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)b, f_num*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)res, o_ch*o_dim*o_dim*sizeof(float));


		printf("Sending Data to IP core slave\n");
		XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)img, ch*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		printf("Sending Image . . .\n");
		//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA1, (UINTPTR)filt, f_num*ch*F_DIM*F_DIM*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		printf("Sending Filter . . .\n");
		//while(XAxiDma_Busy(&axiDMA1, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA2, (UINTPTR)b, f_num*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		printf("Sending Bias . . .\n");
		//while(XAxiDma_Busy(&axiDMA2, XAXIDMA_DMA_TO_DEVICE));

		printf("Getting Data . . .\n");
		XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)res, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);
		//////// repeating image ///////////

		for(int i =0; i<f_num-1; i++){
			printf("Sending image for filter %d\n", i+1);
			while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
			XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)img, ch*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		}
		/////////////////////////////////////////


		//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DEVICE_TO_DMA));
		/*
		while(XAxiDma_Busy(&axiDMA2, XAXIDMA_DMA_TO_DEVICE));
		printf("Sending bias :DONE\n");
		while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
		printf("Sending Image :DONE\n");
		while(XAxiDma_Busy(&axiDMA1, XAXIDMA_DMA_TO_DEVICE));
		printf("Sending filter :DONE\n");
		*/

		//Invalidate the cache to avoid reading garbage
		Xil_DCacheInvalidateRange((UINTPTR)res, o_ch*o_dim*o_dim*sizeof(float));
		printf("Waiting for IP to Terminate . . .\n");
		while(!XConv_IsDone(&conv));
		printf("HW Calculation Complete!\n\n");







		///////////////// SOFTWARE CONVOLUTION /////////////////////////
		printf("\nStarting SW convolution . . .\n");
		//zero padding

		int pad =1;
		int dim_t = dim + 2*pad;
		float img_t[ch*dim_t*dim_t];
		//float *img_t = (float *)img_t_addr;
		for(int i=0; i< ch; i++)
		{
			for(int x = 0; x<pad ; x++)
			{
				for(int y = 0; y< dim_t; y++)
				{
					/*
					img_t[i][x][y] = 0;
					img_t[i][y][x] = 0;
					img_t[i][(dim_t-1)-x][y] = 0;
					img_t[i][y][(dim_t-1)-x] = 0;
					*/
					img_t[i*dim_t*dim_t + x*dim_t + y] = 0;
					img_t[i*dim_t*dim_t + y*dim_t + x] = 0;
					img_t[i*dim_t*dim_t + ((dim_t-1)-x)*dim_t + y] = 0;
					img_t[i*dim_t*dim_t + y*dim_t + (dim_t-1)-x] = 0;
				}
			}

			//fill the empty center space with img(input)--> then the result will be the img padded(img_t)
			for(int x=pad; x<(dim_t-pad); x++)
				for(int y=pad; y<(dim_t-pad); y++)
					img_t[i*dim_t*dim_t + x*dim_t + y] = img[i*dim*dim + (x-pad)*dim + y-pad];
		}

		// Now we can start the convolution
		float sum;
		float res_sw[o_ch*o_dim*o_dim];
		//float *res_sw = (float *)res_sw_addr;
		for (int i=0; i<f_num; i++)//number of filters
		{
			for(int x=0; x<o_dim; x++)
			{
				for(int y=0; y<o_dim; y++)
				{
					sum=0;
					//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
					for(int j=0; j < ch ; j++)
					{
						for(int k=x; k<(x + F_DIM); k++)
						{
							for(int l =y; l<(y+F_DIM); l++)
							{
								sum += img_t[j*dim_t*dim_t + k*dim_t +l]*filt[i*ch*F_DIM*F_DIM+ j*F_DIM*F_DIM+ (k-x)*F_DIM +l-y];
							}
						}
					}
					res_sw[i*o_dim*o_dim + x*o_dim + y] = sum + b[i];
				}
			}
		}
		printf("Done!\n");

		printf("Comparing SW & HW results . . . \n");

		////////////////////////////////////
		int confirm=-1;
		for(int c=0; c < o_ch ; c++)
		{
			for(int i=0;i<o_dim;i++)
			{
				for(int j=0;j<o_dim;j++)
				{
					//res[c*o_dim*o_dim + i*o_dim*+j]=result.read();
					//printf("%f\t", res[c*o_dim*o_dim + i*o_dim+j]);//*o_dim*o_dim + i*o_dim*+j].data);
					if(abs(res[c*o_dim*o_dim + i*o_dim+j] - res_sw[c*o_dim*o_dim + i*o_dim+j])>0.1)
					{
						confirm=1;
						printf("%f (SW) != %f (HW)\n",res_sw[c*o_dim*o_dim + i*o_dim+j],res[c*o_dim*o_dim + i*o_dim+j]);
					}
				}
				//printf("\n");
			}
			//printf("\n");
		}

		if(confirm == 1)
			printf("\n---------------------  Status : **FAIL**  --------------------------\n\n");
		else{

			count++;
			printf("\n---------------------  Status : **PASS**  --------------------------\n\n");
		}


		//cleanup_platform();
	}
	printf("Passed Tests: %d",count);
	return 0;
}
