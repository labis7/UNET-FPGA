#include<stdio.h>
#include<xmy_ip_hls_hw.h>
#include<xmy_ip_hls.h>
#include<xparameters.h>
#include<xaxidma.h>
#include<math.h>


XMy_ip_hls my_ip_hls;
XMy_ip_hls_Config *my_ip_hls_cfg;
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
	my_ip_hls_cfg = XMy_ip_hls_LookupConfig(XPAR_MY_IP_HLS_0_DEVICE_ID);
	if(my_ip_hls_cfg)
	{
		printf("Initializing My_ip_hls . . .\n");

		int status = XMy_ip_hls_CfgInitialize(&my_ip_hls, my_ip_hls_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing My_ip_hls core\n");
	}
	printf("Done!!!\n");

	return;

}

//struct data_out{
//	float data;
//	unsigned int tlast:1;
//};

#define F_DIM 2

int main()
{
	//Get pointers to DMA TX/RX addresses
	//int *m_dma_buffer_TX = (int *)TX_BUFFER_BASE;
	//int *m_dma_buffer_RX = (int *)RX_BUFFER_BASE;

	float *res = (float *)res_addr;
	for (int lab=0;lab<3;lab++)
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
		int ch=64;
		int dim = 32;
		int f_num =32;
		if(lab == 1)
		{
			int ch=128;
			int dim = 16;
			int f_num =64;
		}
		if(lab == 2)
		{
			int ch=256;
			int dim = 8;
			int f_num =128;
		}

		XMy_ip_hls_Set_slaveIn_ch(&my_ip_hls, ch);
		XMy_ip_hls_Set_slaveIn_dim(&my_ip_hls, dim);
		XMy_ip_hls_Set_slaveIn_f_num(&my_ip_hls, f_num);

		XMy_ip_hls_Start(&my_ip_hls);
		printf("Setting up via Axi Lite is Completed!\n");

		printf("----------------------------------------------------------------\n");


		int o_dim = dim*2;
		int o_ch =f_num;
		//Do the stream calculation
		//float *img=(float *)malloc(ch*dim*dim*sizeof(float));
		float *img=(float *)img_addr;//img[ch][dim][dim];
		float *filt=(float *)filt_addr;//filt[f_num*ch*F_DIM*F_DIM];
		//Display Data

		//data_out *res=(data_out *)malloc(o_ch*o_dim*o_dim*sizeof(data_out));


		float *b=(float *)bias_addr;//b[f_num];
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
			b[k]=0;
		}
		for(int c=0; c<ch ; c++)
			for(int i=0;i<dim;i++)
				for(int j=0;j<dim;j++)
					img[c*dim*dim + i*dim+j]= (i+1)*(j*2+1)*1.3 + c; //*dim*dim+i*dim+j]= (i+1)*(j*2+1)*1.3 + c;
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
		//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DEVICE_TO_DMA));

		//Invalidate the cache to avoid reading garbage
		Xil_DCacheInvalidateRange((UINTPTR)res, o_ch*o_dim*o_dim*sizeof(float));
		printf("Waiting for IP to Terminate . . .\n");
		while(!XMy_ip_hls_IsDone(&my_ip_hls));
		printf("Calculation Complete!\n\n");







		///////////////////////////// SW TRANSPOSED CONVOLUTION  ////////////////////////////////////
		int s =2;

		printf("\nSW Transposed Convolution Initiated . . .\n");
		float *res_sw = (float *)res_sw_addr;
		for(int c =0 ; c<o_ch; c++)
			for(int i=0; i<o_dim ; i++)
				for(int j=0;j<o_dim;j++)
					res_sw[c*o_dim*o_dim + i*o_dim + j] = b[c];

		for(int i=0; i<f_num; i++)//number of filters/o_ch
			for(int j=0; j < ch ; j++)
				for(int x=0; x<dim; x++)
					for(int y=0; y<dim; y++)
						for(int k=0; k<F_DIM; k++)
							for(int l =0 ; l<F_DIM; l++)
								res_sw[i*o_dim*o_dim +(x*s+k)*o_dim + (y*s+l)] += img[j*dim*dim + x*dim+y]*filt[i*ch*F_DIM*F_DIM + j*F_DIM*F_DIM + k*F_DIM + l];


		printf("SW & HW Calculations completed. Now comparing . . .\n");

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
		else
			printf("\n---------------------  Status : **PASS**  --------------------------\n\n");

		/////////////////////////////////////////////////////////////////


		/*
		printf("After SEND:\n");
		for(int c=0; c < o_ch ; c++)
		{
			for(int i=0;i<o_dim;i++)
			{
				for(int j=0;j<o_dim;j++)
				{
					//res[c*o_dim*o_dim + i*o_dim*+j]=result.read();
					printf("%f\t", res[c*o_dim*o_dim + i*o_dim+j]);
				}
				printf("\n");
			}
			printf("\n");
		}

		*/



		//cleanup_platform();
	}
	return 0;
}
