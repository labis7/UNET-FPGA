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

void initPeripherals()
{
	//initialize dogain core
	printf("Initializing doGain . . .\n");
	if(doGain_cfg)
	{
		int status = XDogain_CfgInitialize(&doGain, doGain_cfg);
		if(status != XST_SUCCESS)
			printf("Error initializing doGain core\n");
	}
	printf("Done\n");
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

}



int main()
{
	//Get pointers to DMA TX/RX addresses
	//int *m_dma_buffer_TX = (int *)TX_BUFFER_BASE;
	int *m_dma_buffer_RX = (int *)RX_BUFFER_BASE;


	printf("Hello World!!!\n\n");
	printf("Hardware Initialization . . .\n");
	initPeripherals();
	printf("Hardware Initialization : DONE\n");

	//Do the stream calculation
	for(int idx = 0 ; idx < SIZE_ARR ; idx++)
		inStreamData[idx] = idx;  //Dynamic ?!?!

	//Gain setup and core startup
	int gain = 5;
	XDogain_Set_gain(&doGain, gain);
	XDogain_Start(&doGain);

	//Flush the cache of the buffers
	Xil_DCacheFlushRange((UINTPTR)inStreamData, SIZE_ARR*sizeof(int));
	Xil_DCacheFlushRange((UINTPTR)m_dma_buffer_RX, SIZE_ARR*sizeof(int));


	printf("Sending Data to IP core slave\n");
	XAxiDma_SimpleTransfer(&axiDMA, (UINTPTR)inStreamData, SIZE_ARR*sizeof(int), XAXIDMA_DMA_TO_DEVICE);

	printf("Getting Data . . .\n");
	XAxiDma_SimpleTransfer(&axiDMA, (UINTPTR)m_dma_buffer_RX, SIZE_ARR*sizeof(int), XAXIDMA_DEVICE_TO_DMA);
	while(XAxiDma_Busy(&axiDMA, XAXIDMA_DEVICE_TO_DMA));

	//Invalidate the cache to avoid reading garbage
	Xil_DCacheInvalidateRange((UINTPTR)m_dma_buffer_RX, SIZE_ARR*sizeof(int));

	while(!XDogain_IsDone(&doGain));
	printf("Calculation Complete!\n");

	//Display Data
	for(int idx = 0 ; idx < SIZE_ARR ; idx++ )
		printf("Recv[%d] = %d\n", idx, m_dma_buffer_RX[idx]);





	return 0;
}
