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



}



int main()
{
	printf("Hello World!!!\n");
	printf("Hardware Initialization . . .\n");






	return 0;
}
