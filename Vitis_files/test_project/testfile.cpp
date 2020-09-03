#include<stdio.h>
#include<xdogain.h>
#include<xdogain_hw.h>
#include<xparameters.h>
#include<xaxidma.h>



XDogain doGain;
XDogain_Config *doGain_cfg;
XAxiDma axiDMA;
XAxiDma_Config *axiDMA_cfg;



//DMA Addresses
#define MEM_BASE_ADDR 0x01000000
#define TX_BUFFER_BASE (MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE (MEM_BASE_ADDR + 0x00300000)

#define SIZE_ARR 10
int inStreamData[SIZE_ARR];
int outStreamData[SIZE_ARR];


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
	//initialize dogain core
	doGain_cfg = XDogain_LookupConfig(XPAR_DOGAIN_0_DEVICE_ID);
	if(doGain_cfg)
	{
		printf("Initializing doGain . . .\n");

		int status = XDogain_CfgInitialize(&doGain, doGain_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing doGain core\n");
	}
	printf("Done!!!\n");

	return;

}



int main()
{
	//Get pointers to DMA TX/RX addresses
	//int *m_dma_buffer_TX = (int *)TX_BUFFER_BASE;
	//int *m_dma_buffer_RX = (int *)RX_BUFFER_BASE;


	printf("Hello World!!!\n----------------------------------------------------------------\n");
	//printf("\nPlatform Initialization . . .\n");
	//init_platform();
	//printf("Platform Initialization: Done!\n");
	init_dma();
	printf("DMA Initialization: Done!\n");
	printf("IP initialization . . .\n");
	setupIPs();
	printf("IP Initialization : DONE\n");

	//Do the stream calculation
	for(int idx = 0 ; idx < SIZE_ARR ; idx++)
		inStreamData[idx] = idx;  //Dynamic ?!?!

	//Gain setup and core startup
	int gain = 5;
	//Dogain_startup();
	//Xil_DCacheFlushRange((UINTPTR)inStreamData, SIZE_ARR*sizeof(int));
	//Xil_DCacheFlushRange((UINTPTR)m_dma_buffer_RX, SIZE_ARR*sizeof(int));
	//printf("Flush Completed!\n");
	///////////////////////////////////////////////////////////////////

	XDogain_Set_gain(&doGain, gain);
	printf("Set Gain Completed!\n");


	XDogain_Start(&doGain);

	//Flush the cache of the buffers
	printf("Flushing Cache\n");
	Xil_DCacheFlushRange((UINTPTR)inStreamData, SIZE_ARR*sizeof(int));
	//Xil_DCacheFlushRange((UINTPTR)m_dma_buffer_RX, SIZE_ARR*sizeof(int));
	Xil_DCacheFlushRange((UINTPTR)outStreamData, SIZE_ARR*sizeof(int));

	printf("Sending Data to IP core slave\n");
	XAxiDma_SimpleTransfer(&axiDMA, (UINTPTR)inStreamData, SIZE_ARR*sizeof(int), XAXIDMA_DMA_TO_DEVICE);

	printf("Getting Data . . .\n");
	XAxiDma_SimpleTransfer(&axiDMA, (UINTPTR)outStreamData, SIZE_ARR*sizeof(int), XAXIDMA_DEVICE_TO_DMA);
	while(XAxiDma_Busy(&axiDMA, XAXIDMA_DEVICE_TO_DMA));

	//Invalidate the cache to avoid reading garbage
	Xil_DCacheInvalidateRange((UINTPTR)outStreamData, SIZE_ARR*sizeof(int));
	printf("Waiting for IP to Terminate . . .\n");
	while(!XDogain_IsDone(&doGain));
	printf("Calculation Complete!\n\n");

	//Display Data
	for(int idx = 0 ; idx < SIZE_ARR ; idx++ )
		printf("Recv[%d] = %d\n", idx, outStreamData[idx]);




	//cleanup_platform();
	return 0;
}
