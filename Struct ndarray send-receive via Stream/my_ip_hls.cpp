#include "my_ip_hls.hpp"


void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, float **array) {
//#pragma HLS INTERFACE m_axi depth=32 port=slaveIn
//	void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, uint32 rule1,uint32 rule2) {
//#pragma HLS INTERFACE s_axilite port=count_out bundle=rule_config
#pragma HLS INTERFACE s_axilite port=rule1 bundle=rule_config
#pragma HLS INTERFACE s_axilite port=rule2 bundle=rule_config
#pragma HLS DATAFLOW interval=1
#pragma HLS INTERFACE axis register both port=slaveIn
#pragma HLS INTERFACE axis register both port=masterOut
#pragma HLS INTERFACE ap_ctrl_none port=return

//internal fifos
	static stream<axiWord> ps2ipFifo("ps2ipFifo");
#pragma HLS STREAM variable=ps2ipFifo depth=64 dim=1
	static stream<axiWord> ip2psFifo("ip2psFifo");
#pragma HLS STREAM variable=ip2psFifo depth=64 dim=1


	//TODO: add function for configuration registers / counters via AXI Lite

	//fifo that keeps input data
	//rules(rule1,rule2);

//	ps2ip_fifo(slaveIn,ps2ipFifo);

	//int a = slaveIn[0];//reg
	//int arr[100];//bram
	//memcpy(arr, array, 5 * sizeof(int));

	//printf("\n\nResults: %d %d %d %d %d\n\n",(int)arr[0],(int)arr[1],(int)arr[2],(int)arr[3],(int)arr[4]);

	axiWord dataOut = {0,0,0,0};
	int dim=3;
	////////////////// create BRAM space ////////////////
	float **tmp=(float **)malloc(dim*sizeof(float *));
	for (int i = 0; i< dim; i++)
		tmp[i] = (float *) malloc(dim*sizeof(float));
	/////////////////////////////////////////////////////

	float **tmp1;
	slaveIn.read(dataOut);
	tmp1 = dataOut.input;
	memcpy(tmp,tmp1, dim*dim*sizeof(float)); //ferto sthn BRAM (me ta katallhla directives mporw na to kanw registers gia pio grigora alla den exoume kai polu xwro giauto)


	printf("After SEND:\n");
	for(int i=0;i<dim;i++)
	{
		for(int j=0;j<dim;j++)
		{
			printf("%f\t",tmp[i][j]);
			tmp[i][j] = tmp[i][j] +1;
		}
		printf("\n");
	}

	printf("SENDIND BACK:\n");
	for(int i=0;i<dim;i++)
	{
		for(int j=0;j<dim;j++)
		{
			printf("%f\t",tmp[i][j]);
		}
		printf("\n");
	}
	axiWord dataIn;
	dataIn.input = tmp; //return the result
	masterOut.write(dataIn);

	//core of the IP
	//uint32 tmp,tmp1;
	//get_rules(tmp,tmp1);
	//core(ps2ipFifo,ip2psFifo, tmp,tmp1);
	//fifo that keeps output data
	//ip2ps_fifo(ip2psFifo,masterOut);

	return;

}





