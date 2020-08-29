#include "myip_hls.hpp"
//static float img[10][10][10];//130*32
static float img[128];
static float filt[F_DIM*F_DIM];
static float res[130*130];
//static float b[10];

void myip_hls(stream<float> &image, stream<float> &filter, stream<float> &bias, stream<float> &result, stream<data> &slaveIn) {
#pragma HLS ARRAY_PARTITION variable=filt complete dim=1

#pragma HLS INTERFACE axis register both port=image bundle=inputs
#pragma HLS INTERFACE axis register both port=filter bundle=inputs

//#pragma HLS ARRAY_PARTITION variable=filt cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=img cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=res cyclic factor=2 dim=1


//#pragma HLS INTERFACE m_axi depth=32 port=slaveIn
//	void my_ip_hls(stream<axiWord> &slaveIn,stream<axiWord> &masterOut, uint32 rule1,uint32 rule2) {
//#pragma HLS INTERFACE s_axilite port=count_out bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule1 bundle=rule_config
//#pragma HLS INTERFACE s_axilite port=rule2 bundle=rule_config
//#pragma HLS INTERFACE m_axi depth=1024 port=image //bundle = inputs
//#pragma HLS INTERFACE m_axi depth=1024 port=filter //bundle = inputs
//#pragma HLS DATAFLOW interval=1
/*
#pragma HLS DATAFLOW interval=1
#pragma HLS INTERFACE axis register both port=slaveIn
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=filter
#pragma HLS INTERFACE axis register both port=result
#pragma HLS INTERFACE ap_ctrl_none port=return
*/

	data dataOut;
	int dim,ch,f_num,mode;
	//float img_pix,filt_pix;

	//float arr[2][4][4];
	slaveIn.read(dataOut);
	ch = dataOut.ch;
	dim = dataOut.dim;
	f_num = dataOut.f_num;

	int s = 1; //stride
	int pad, o_dim,o_ch,dim_t;

////////////////////////////     CONVOLUTION       /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

	pad = 1; //by default -- Transposed conv will use a different approach
	o_dim = dim;
	dim_t = dim + 2*pad;
	o_ch = f_num;



	for(int f=0; f<f_num; f++ )
	{
#pragma HLS loop_tripcount min=16 max=16
		bias.read();
		for(int x=0; x<(dim_t); x++)
#pragma HLS loop_tripcount min=130 max=130
			for(int y=0; y<(dim_t); y++)
#pragma HLS loop_tripcount min=130 max=130
				res[x*dim_t +y] = 0;
		for(int c=0; c<ch; c++)
		{
#pragma HLS loop_tripcount min=32 max=32
			// load the respective filter channel
			for(int x=0; x<9 ; x++)

				filt[x] = filter.read();
			///////////////////
			for(int x = 0; x<dim; x++)
			{
#pragma HLS loop_tripcount min=128 max=128
				//load the new row //
				for(int y=0; y<(dim); y++)
				#pragma HLS loop_tripcount min=128 max=128
					img[y] = image.read();
				/////////////////////
				for(int y=0; y<dim ; y++)
				{
#pragma HLS pipeline
				#pragma HLS loop_tripcount min=128 max=128
					res[x*dim_t +y] += img[y]*filt[0] ;
					res[x*dim_t +y+1] += img[y]*filt[1];
					res[x*dim_t +y+2] +=img[y]*filt[2];
					res[(x+1)*dim_t +y]+=img[y]*filt[3];
					res[(x+1)*dim_t +y+1] += img[y]*filt[4];
					res[(x+1)*dim_t +y+2] += img[y]*filt[5];
					res[(x+2)*dim_t +y] += img[y]*filt[6];
					res[(x+2)*dim_t +y+1] += img[y]*filt[7];
					res[(x+2)*dim_t +y+2] += img[y]*filt[8];
				}
			}


		}
		for(int x=1; x<(dim_t-1); x++)
#pragma HLS loop_tripcount min=128 max=128
			for(int y=1; y<(dim_t-1); y++)
#pragma HLS loop_tripcount min=128 max=128
				result.write(res[x*dim_t +y]);
	}

	return;

}
