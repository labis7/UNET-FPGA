#include "my_ip_hls.hpp"
static float img[1000];
static float img_t[10][10+2][10+2];//+2 happens when we have pad==1, we need a temporary matrix
static float filt[10][10][F_DIM][F_DIM];
static float b[10];

void my_ip_hls(float *image, stream<data> &slaveIn) {
#pragma HLS INTERFACE m_axi depth=1024 port=image bundle=inputs
//#pragma HLS INTERFACE m_axi depth=32 port=slaveIn
//	void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, uint32 rule1,uint32 rule2) {
//#pragma HLS INTERFACE s_axilite port=count_out bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule1 bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule2 bundle=rule_config

/*
#pragma HLS DATAFLOW interval=1
#pragma HLS INTERFACE axis register both port=slaveIn
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=result
#pragma HLS INTERFACE ap_ctrl_none port=return
*/
//#pragma HLS loop_tripcount min=<int> max=<int> avg=<int>
//internal fifos
	/*
	static stream<image> ps2ipFifo("ps2ipFifo");
#pragma HLS STREAM variable=ps2ipFifo depth=64 dim=1
	static stream<image> ip2psFifo("ip2psFifo");
#pragma HLS STREAM variable=ip2psFifo depth=64 dim=1

*/
	//TODO: add function for configuration registers / counters via AXI Lite

	//fifo that keeps input data
	//rules(rule1,rule2);

//	ps2ip_fifo(slaveIn,ps2ipFifo);

	//int a = slaveIn[0];//reg
	//int arr[100];//bram
	//memcpy(arr, array, 5 * sizeof(int));

	//printf("\n\nResults: %d %d %d %d %d\n\n",(int)arr[0],(int)arr[1],(int)arr[2],(int)arr[3],(int)arr[4]);





	data dataOut;
	int dim,ch,f_num,mode;
	//float img_pix,filt_pix;

	//float arr[2][4][4];
	slaveIn.read(dataOut);
	ch = dataOut.ch;
	dim = dataOut.dim;
	f_num = dataOut.f_num;
	/*
	t= (float ***)malloc(ch*sizeof(float**));

		for (int i = 0; i< dim; i++)
		{
			t[i] = (float **) malloc(dim*sizeof(float *));
			for (int j = 0; j < dim; j++)
				t[i][j] = (float *)malloc(dim*sizeof(float));
		}

*/
	//memcpy(img,image, ch*dim*dim*(sizeof(float)));


	//float *test;
	//memcpy(test, image, ch*dim*dim*sizeof(float));
	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j++)
				break;
				//printf("%f\t",image[c][i][j]);
			//printf("\n");
		}
		//printf("\n");
	}
	memcpy(img, image, ch*dim*dim*sizeof(float));

	//printf("\nAfter SEND:\n");
	for(int c=0; c<ch ; c++)
	{
		for(int i=0;i<dim;i++)
		{
			for(int j=0;j<dim;j++)
			{
				img[c*ch*dim+i*dim+j] = image[c*ch*dim+i*dim+j]+5;
				//printf("%f\t",img[c*ch*dim+i*dim+j]);
			}
			//printf("\n");
		}
		//printf("\n");
	}

}





