#include<stdio.h>
#include<xmy_ip_hls_hw.h>
#include<xmy_ip_hls.h>
#include<xparameters.h>
#include<xaxidma.h>
#include<math.h>
#include <unistd.h>


XMy_ip_hls my_ip_hls;
XMy_ip_hls_Config *my_ip_hls_cfg;
XAxiDma axiDMA;
XAxiDma_Config *axiDMA_cfg;



//DMA Addresses
//#define MEM_BASE_ADDR 0x01000000
#define TX_BUFFER_BASE (0x01000000 + 0x00100000)
#define RX_BUFFER_BASE (0x01000000 + 0x00600000)
#define SW_BASE 0x00500000

//#define SIZE_ARR 10
//int inStreamData[SIZE_ARR];
//int outStreamData[SIZE_ARR];


void init_dma(){

	//initialize AxiDMA core
	printf("Initializing AxiDMA . . .\n");
	axiDMA_cfg = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
	if(axiDMA_cfg)
	{
		int status = XAxiDma_CfgInitialize(&axiDMA, axiDMA_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing AxiDMA core\n");
	}
	printf("Done\n");
	//Disable Interrupts
	XAxiDma_IntrDisable(&axiDMA, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&axiDMA, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Reset DMA
	printf("Resetting  DMA . . .\n");
	XAxiDma_Reset(&axiDMA);
	while(!XAxiDma_ResetIsDone(&axiDMA)){}
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

int main()
{
	//Get pointers to DMA TX/RX addresses
	//int *m_dma_buffer_TX = (int *)TX_BUFFER_BASE;
	float *res = (float *)RX_BUFFER_BASE;
	float *img =(float *)TX_BUFFER_BASE;

	printf("Hello World!!!\n----------------------------------------------------------------\n");
	//printf("\nPlatform Initialization . . .\n");
	//init_platform();
	//printf("Platform Initialization: Done!\n");
	init_dma();
	printf("DMA Initialization: Done!\n");
	printf("IP initialization . . .\n");
	setupIPs();
	printf("IP Initialization : DONE\n");

	int ch= 16;
	int dim = 128;

	//while(!(my_ip_hls.IsReady == XIL_COMPONENT_IS_READY)){}

	XMy_ip_hls_Set_ch(&my_ip_hls, ch);
	XMy_ip_hls_Set_dim(&my_ip_hls, dim);

	XMy_ip_hls_Start(&my_ip_hls);
	printf("Setting via Axi Lite is Completed!\n");

	printf("----------------------------------------------------------------\n");
	//while(!(my_ip_hls.IsReady == XIL_COMPONENT_IS_READY)){}
	//Do the stream calculation
	//float *img=(float *)malloc(ch*dim*dim*sizeof(float));
	//float img[ch][dim][dim];
	//Display Data
	int o_dim = dim/2;
	int o_ch = ch;
	//data_out *res=(data_out *)malloc(o_ch*o_dim*o_dim*sizeof(data_out));



	for(int c=0; c<ch ; c++)
		for(int i=0;i<dim;i++)
			for(int j=0;j<dim;j++)
				img[c*dim*dim+i*dim+j]= ((i+1)*(j*2+1)*1.3 + c)/10000; //*dim*dim+i*dim+j]= (i+1)*(j*2+1)*1.3 + c;
	/*printf("Before SEND:\n");
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

	printf("Before SEND:\n");
	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j++)
			{
				printf("%f\t",img[c*dim*dim+i*dim* + j]);
			}
			printf("\n");
		}
		printf("\n");
	}

	*/

	//float res[o_ch*o_dim*o_dim];

	//Flush the cache of the buffers
	printf("Flushing Cache\n");
	Xil_DCacheFlushRange((UINTPTR)img, ch*dim*dim*sizeof(float));
	//Xil_DCacheFlushRange((UINTPTR)m_dma_buffer_RX, SIZE_ARR*sizeof(int));
	Xil_DCacheFlushRange((UINTPTR)res, o_ch*o_dim*o_dim*sizeof(float));

	printf("Sending Data to IP core slave\n");
	XAxiDma_SimpleTransfer(&axiDMA, (UINTPTR)img, ch*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
	//while(XAxiDma_Busy(&axiDMA, XAXIDMA_DMA_TO_DEVICE));

	printf("Getting Data . . .\n");
	XAxiDma_SimpleTransfer(&axiDMA, (UINTPTR)res, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);
	//while(XAxiDma_Busy(&axiDMA, XAXIDMA_DEVICE_TO_DMA));

	//Invalidate the cache to avoid reading garbage
	Xil_DCacheInvalidateRange((UINTPTR)res, o_ch*o_dim*o_dim*sizeof(float));
	printf("Waiting for IP to Terminate . . .\n");
	while(!XMy_ip_hls_IsDone(&my_ip_hls));
	printf("Calculation Complete!\n\n");

/*
	for(int c=0; c < o_ch ; c++)
	{
		for(int i=0;i<o_dim;i++)
		{
			for(int j=0;j<o_dim;j++)
			{
				//res[c*o_dim*o_dim + i*o_dim*+j]=result.read();
				printf("%f\t", res[c*o_dim*o_dim + i*o_dim+j]);//*o_dim*o_dim + i*o_dim*+j].data);
			}
			//printf("\n");
		}
		//printf("\n");
	}
*/
	//////// SOFTWARE MAXPOOLING  ///////////////


		int s =2;
		o_dim=dim/2;
		float max;
		//float res_sw[ch*o_dim*o_dim];
		float *res_sw = (float *)SW_BASE;
		printf("SW Maxpooling initiated!\n");
		for(int i=0; i<ch; i++)
		{
			for(int x=0; x<o_dim; x++)
			{
				for (int y = 0; y<o_dim; y++)
				{
					max = -100000;

					if(img[i*dim*dim +s*x*dim +s*y]> max)
						max = img[i*dim*dim +s*x*dim +s*y];
					if(img[i*dim*dim +s*x*dim +s*y+1]> max)
						max = img[i*dim*dim +s*x*dim +s*y+1];
					if(img[i*dim*dim +s*x*dim +dim +s*y]> max)
						max = img[i*dim*dim +s*x*dim +dim +s*y];
					if(img[i*dim*dim +s*x*dim +dim +s*y+1]> max)
						max = img[i*dim*dim +s*x*dim +dim +s*y+1];

					//Now we have the sub-part max values, we can save it
					res_sw[i*o_dim*o_dim + x*o_dim + y] =max;
				}
			}
		}

		////////////////////////////////////
		int confirm=-1;
		printf("HW & SW Completed. Now Comparing ...\n");
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

	return 0;
}
