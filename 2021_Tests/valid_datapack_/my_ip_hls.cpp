#include "my_ip_hls.hpp"

static data_t img_t0[256]; //Max resolution supported
static data_t img_t1[256]; //

//static float b[10];


void my_ip_hls(stream<data_d> &image,stream<data_o> &result,data &slaveIn) {
#pragma HLS data_pack variable=image struct_level
#pragma HLS data_pack variable=result struct_level
//Axi lite Inteface
#pragma HLS INTERFACE s_axilite port=slaveIn bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

//Axi-4 Stream Inteface
#pragma HLS INTERFACE axis register both port=image
#pragma HLS INTERFACE axis register both port=result

//BRAM arrays partition
#pragma HLS ARRAY_PARTITION variable=img_t1 cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=img_t0 cyclic factor=8 dim=1

	data_o data_out;

	//read axi lite data
	int ch = slaveIn.ch;
	int dim = slaveIn.dim;
	
	//setup local settings
	int s =2;
	int o_dim=dim/2;
	data_t max=-100000; // -infinity
	data_t max1=-100000; // -infinity
	data_t max2=-100000; // -infinity
	data_t max3=-100000; // -infinity
	data_t max4 = -100000;
	data_t max5 = -100000;
	data_t max6 = -100000;
	data_t max7 = -100000;
	
	data_d data_in;
	
	//For each input channel, apply maxpool
	for(int i=0; i<ch; i++)
	{
#pragma HLS loop_tripcount min=16 max=16

		//For every output row, load a pair of input image lines into 2 line buffers
		for(int x=0; x<o_dim; x++)
		{
#pragma HLS loop_tripcount min=128 max=128
			for (int z=0; z<dim; z+=16)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				 data_in=image.read();
				 img_t0[z]= data_in.v1;
				 img_t0[z+1]=data_in.v2;
				 img_t0[z+2]=data_in.v3;
				 img_t0[z+3]=data_in.v4;
				 img_t0[z+4]= data_in.v5;
				 img_t0[z+5]=data_in.v6;
				 img_t0[z+6]=data_in.v7;
				 img_t0[z+7]=data_in.v8;

				 img_t0[z+8]= data_in.v12;
				 img_t0[z+9]=data_in.v22;
				 img_t0[z+10]=data_in.v32;
				 img_t0[z+11]=data_in.v42;
				 img_t0[z+12]= data_in.v52;
				 img_t0[z+13]=data_in.v62;
				 img_t0[z+14]=data_in.v72;
				 img_t0[z+15]=data_in.v82;
				//image.read(img_t0[z]); //LB1
				//image.read(img_t0[z+1]); //LB1
			}
			for (int z=0; z<dim; z+=16)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
				 data_in=image.read();
				 img_t1[z]= data_in.v1;
				 img_t1[z+1]=data_in.v2;
				 img_t1[z+2]=data_in.v3;
				 img_t1[z+3]=data_in.v4;
				 img_t1[z+4]=data_in.v5;
				 img_t1[z+5]=data_in.v6;
				 img_t1[z+6]=data_in.v7;
				 img_t1[z+7]=data_in.v8;

				 img_t1[z+8]= data_in.v12;
				 img_t1[z+9]=data_in.v22;
				 img_t1[z+10]=data_in.v32;
				 img_t1[z+11]=data_in.v42;
				 img_t1[z+12]= data_in.v52;
				 img_t1[z+13]=data_in.v62;
				 img_t1[z+14]=data_in.v72;
				 img_t1[z+15]=data_in.v82;
				//image.read(img_t1[z]); //LB2
				//image.read(img_t1[z+1]); //LB1
			}
			for (int y = 0; y<o_dim; y+=8)
			{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=16 max=16
					
				//Comparators - Part
				if(img_t0[s*y]> max)
					max = img_t0[s*y];
				if(img_t0[s*y+1]> max)
					max = img_t0[s*y+1];
				if(img_t1[s*y]> max)
					max = img_t1[s*y];
				if(img_t1[s*y+1]> max)
					max = img_t1[s*y+1];
				data_out.v1 = max;
				//Comparators - Part
				if(img_t0[s*y +2]> max1)
					max1 = img_t0[s*y+2];
				if(img_t0[s*y +3]> max1)
					max1 = img_t0[s*y+3];
				if(img_t1[s*y +2]> max1)
					max1 = img_t1[s*y+2];
				if(img_t1[s*y +3]> max1)
					max1 = img_t1[s*y+3];
				data_out.v2 = max1;
				//Comparators - Part
				if(img_t0[s*y +4]> max2)
					max2 = img_t0[s*y+4];
				if(img_t0[s*y +5]> max2)
					max2 = img_t0[s*y+5];
				if(img_t1[s*y +4]> max2)
					max2 = img_t1[s*y+4];
				if(img_t1[s*y +5]> max2)
					max2 = img_t1[s*y+5];
				data_out.v3 = max2;
				//Comparators - Part
				if(img_t0[s*y +6]> max3)
					max3 = img_t0[s*y+6];
				if(img_t0[s*y +7]> max3)
					max3 = img_t0[s*y+7];
				if(img_t1[s*y +6]> max3)
					max3 = img_t1[s*y+6];
				if(img_t1[s*y +7]> max3)
					max3 = img_t1[s*y+7];
				data_out.v4 = max3;
//---------------------------------------------------------------------------
				//Comparators - Part
				if(img_t0[s*y +8]> max4)
					max4 = img_t0[s*y +8];
				if(img_t0[s*y +8+1]> max4)
					max4 = img_t0[s*y +8+1];
				if(img_t1[s*y +8]> max4)
					max4 = img_t1[s*y +8];
				if(img_t1[s*y +8+1]> max4)
					max4 = img_t1[s*y +8+1];
				data_out.v5 = max4;
				//Comparators - Part
				if(img_t0[s*y +8 +2]> max5)
					max5 = img_t0[s*y +8+2];
				if(img_t0[s*y +8 +3]> max5)
					max5 = img_t0[s*y +8+3];
				if(img_t1[s*y +8 +2]> max5)
					max5 = img_t1[s*y +8+2];
				if(img_t1[s*y +8 +3]> max5)
					max5 = img_t1[s*y +8+3];
				data_out.v6 = max5;
				//Comparators - Part
				if(img_t0[s*y +8 +4]> max6)
					max6 = img_t0[s*y +8+4];
				if(img_t0[s*y +8 +5]> max6)
					max6 = img_t0[s*y +8+5];
				if(img_t1[s*y +8 +4]> max6)
					max6 = img_t1[s*y +8+4];
				if(img_t1[s*y +8 +5]> max6)
					max6 = img_t1[s*y +8+5];
				data_out.v7 = max6;
				//Comparators - Part
				if(img_t0[s*y +8 +6]> max7)
					max7 = img_t0[s*y +8+6];
				if(img_t0[s*y +8 +7]> max7)
					max7 = img_t0[s*y +8+7];
				if(img_t1[s*y +8 +6]> max7)
					max7 = img_t1[s*y +8+6];
				if(img_t1[s*y +8 +7]> max7)
					max7 = img_t1[s*y +8+7];
				data_out.v8 = max7;

				result.write(data_out);
				max = -100000;
				max1 = -100000;
				max2 = -100000;
				max3 = -100000;
				max4 = -100000;
				max5 = -100000;
				max6 = -100000;
				max7 = -100000;

			}
		}
	}


	return;

}
