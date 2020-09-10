#include <header.h>


void predict(struct images_data_ *images_data,struct params_ *params)
{

	///////////////////////// PRE-LOAD Data to Buffers /////////////////////////////
	///////// Laod Parameters  //////////
	float **filters = params->filters;
	float **bias = params->bias;
	float **f_dc = params->f_dc;
	float **b_dc = params->b_dc;
	//int gn_batch = params->gn_batch;
	//float **gamma = params->ga;
	//float **beta = params->be;
	//int layers =params->layers;
	//int f_num_init = params->num_f;
	//////////////////////////////////////

	int ch_num=1;//init
	int dim=images_data->dim;

	//translate to ddr and IP friendly format.
	float *temp0 = (float *)malloc(524288*sizeof(float));
	//float temp0[524288];//(float *)temp0_addr;
	float *temp1 = (float *)malloc(524288*sizeof(float));
	//float temp1[524288];//(float *)temp1_addr;
	//float *b = (float *)bias_addr;


/*
	for (int j=0;j<3;j++)
	{
		for (int k=0;k<3;k++)
			printf("%f\t",filters[0][j*3 + k]);
		printf("\n");
	}
*/
	////////////////////////////////////////////////////////////////////////////////





	printf("Choose an Image(number 0-%d) for prediction: ",(images_data->im_num -1));
	int predict_num;
	scanf("%d", &predict_num);

	predict_num=0;
	printf("\n");

	//choose image for prediction(it can be a list saved in the struct)
	float **images=images_data->images;
	float *image = images[predict_num];//1st image is selected
	//float ****labels=images_data->labels;
	//float ***label = labels[predict_num];
	//dim = images_data->dim; // same across the whole data

	for (int i=0;i<ch_num; i++)
		for (int j=0;j<dim;j++)
			for (int k=0;k<dim;k++)
				temp0[i*dim*dim +j*dim + k] = image[i*dim*dim +j*dim + k];

	/*
	struct conv_data_ *ptr_conv_data = &conv_data;
	//struct gn_data_ *ptr_gn_data = &gn_data;
	struct act_func_data_ *ptr_act_func_data = &act_func_data;
	struct maxpool_data_ *ptr_maxpool_data = &maxpool_data;
	struct concat_crop_data_ *ptr_concat_crop_data =&concat_crop_data;


	float ***conv_out;
	float ****conv_arr = (float ****)malloc(5*sizeof(float ***));//save each conv#_2 so we can concat later
	*/


	float **conv_arr = (float **)malloc(4*sizeof(float *));//save each conv#_2 so we can concat later


	int num_f;
	for(int curr_layer=1; curr_layer<6; curr_layer++)
	{
		///////////////////////////////      LAYER 1          /////////////////////////////////
		//1st convolution
		//(with zero padding ='same',so with stride =1 we get same dim as the input)
		//input: (1,dim,dim), filter :(16,1,3,3), output: (16,dim,dim)
		num_f = calc_f_num(curr_layer);


		XConv_Set_slaveIn_ch(&conv_ip, ch_num);
		XConv_Set_slaveIn_dim(&conv_ip, dim);
		XConv_Set_slaveIn_f_num(&conv_ip, num_f);
		XConv_Start(&conv_ip);


		int o_dim = dim;
		int o_ch = num_f;


		//Flush the cache of the buffers
		//printf("Flushing Cache\n");
		Xil_DCacheFlushRange((UINTPTR)temp0, ch_num*dim*dim*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)filters[(curr_layer-1)*2], num_f*ch_num*9*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)bias[(curr_layer-1)*2], num_f*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float));

		//printf("Sending Data to IP core slave\n");
		XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp0, ch_num*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Image . . .\n");
		//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA1, (UINTPTR)filters[(curr_layer-1)*2], num_f*ch_num*9*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Filter . . .\n");
		//while(XAxiDma_Busy(&axiDMA1, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA2, (UINTPTR)bias[(curr_layer-1)*2], num_f*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Bias . . .\n");
		//while(XAxiDma_Busy(&axiDMA2, XAXIDMA_DMA_TO_DEVICE));

		//printf("Getting Data . . .\n");
		XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);


		///// prepare the skip connection memory buffer for the next conv
		float *skip = (float *)malloc(o_ch*o_dim*o_dim*sizeof(float));
		////////////////////////////////////


		//Invalidate the cache to avoid reading garbage
		Xil_DCacheInvalidateRange((UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float));
		//printf("Waiting for IP to Terminate . . .\n");
		while(!XConv_IsDone(&conv_ip));

		printf("HW Calculation Complete!(Layer : %d.1)\n",curr_layer);


		ch_num = num_f;

		/////////////////////////////////////////////////////////////
		//init_dma();
		XAxiDma_Reset(&axiDMA0);
		while(!XAxiDma_ResetIsDone(&axiDMA0)){}
		XAxiDma_Reset(&axiDMA1);
		while(!XAxiDma_ResetIsDone(&axiDMA1)){}
		XAxiDma_Reset(&axiDMA2);
		while(!XAxiDma_ResetIsDone(&axiDMA2)){}
		//setupIPs();

		//num_f = calc_f_num(curr_layer);

		XConv_Set_slaveIn_ch(&conv_ip, ch_num);
		XConv_Set_slaveIn_dim(&conv_ip, dim);
		XConv_Set_slaveIn_f_num(&conv_ip, num_f);
		XConv_Start(&conv_ip);

		//printf("Flushing Cache\n");
		Xil_DCacheFlushRange((UINTPTR)temp1, ch_num*dim*dim*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)filters[(curr_layer-1)*2 +1], num_f*ch_num*9*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)bias[(curr_layer-1)*2 +1], num_f*sizeof(float));

		///////////////////////////////////////////////////////////////////////////////////////

		//////////// maxpool /////////
		if(curr_layer != 5)
		{
			Xil_DCacheFlushRange((UINTPTR)skip, o_ch*o_dim*o_dim*sizeof(float));




			//printf("Sending Data to IP core slave\n");
			XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp1, ch_num*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
			//printf("Sending Image . . .\n");
			//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
			XAxiDma_SimpleTransfer(&axiDMA1, (UINTPTR)filters[(curr_layer-1)*2 +1], num_f*ch_num*9*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
			//printf("Sending Filter . . .\n");
			//while(XAxiDma_Busy(&axiDMA1, XAXIDMA_DMA_TO_DEVICE));
			XAxiDma_SimpleTransfer(&axiDMA2, (UINTPTR)bias[(curr_layer-1)*2 +1], num_f*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
			//printf("Sending Bias . . .\n");
			//while(XAxiDma_Busy(&axiDMA2, XAXIDMA_DMA_TO_DEVICE));

			//printf("Getting Data . . .\n");
			XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)skip, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);


			Xil_DCacheInvalidateRange((UINTPTR)skip, o_ch*o_dim*o_dim*sizeof(float));
			while(!XConv_IsDone(&conv_ip));
			printf("HW Calculation Complete!(Layer : %d.2)\n",curr_layer);

			printf("HW Calculation Complete!(Layer : %d.1)\n",curr_layer);

			//return;
			XAxiDma_Reset(&axiDMA0);
			while(!XAxiDma_ResetIsDone(&axiDMA0)){}
			XAxiDma_Reset(&axiDMA1);
			while(!XAxiDma_ResetIsDone(&axiDMA1)){}
			XAxiDma_Reset(&axiDMA2);
			while(!XAxiDma_ResetIsDone(&axiDMA2)){}

			conv_arr[curr_layer -1] = skip;

			o_dim = dim/2;

			XMy_ip_hls_Set_slaveIn_ch(&my_ip_hls, ch_num);
			XMy_ip_hls_Set_slaveIn_dim(&my_ip_hls, dim);
			XMy_ip_hls_Start(&my_ip_hls);

			Xil_DCacheFlushRange((UINTPTR)skip, ch_num*dim*dim*sizeof(float));

			XAxiDma_SimpleTransfer(&axiDMA6, (UINTPTR)skip, ch_num*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);

			XAxiDma_SimpleTransfer(&axiDMA6, (UINTPTR)temp0, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);

			Xil_DCacheInvalidateRange((UINTPTR)temp0, o_ch*o_dim*o_dim*sizeof(float));
			while(!XMy_ip_hls_IsDone(&my_ip_hls));


			XAxiDma_Reset(&axiDMA6);
			while(!XAxiDma_ResetIsDone(&axiDMA6)){}

			dim = o_dim;
		}
		else
		{
			Xil_DCacheFlushRange((UINTPTR)temp0, o_ch*o_dim*o_dim*sizeof(float));




			//printf("Sending Data to IP core slave\n");
			XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp1, ch_num*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
			//printf("Sending Image . . .\n");
			//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
			XAxiDma_SimpleTransfer(&axiDMA1, (UINTPTR)filters[(curr_layer-1)*2 +1], num_f*ch_num*9*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
			//printf("Sending Filter . . .\n");
			//while(XAxiDma_Busy(&axiDMA1, XAXIDMA_DMA_TO_DEVICE));
			XAxiDma_SimpleTransfer(&axiDMA2, (UINTPTR)bias[(curr_layer-1)*2 +1], num_f*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
			//printf("Sending Bias . . .\n");
			//while(XAxiDma_Busy(&axiDMA2, XAXIDMA_DMA_TO_DEVICE));

			//printf("Getting Data . . .\n");
			XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp0, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);


			Xil_DCacheInvalidateRange((UINTPTR)temp0, o_ch*o_dim*o_dim*sizeof(float));
			while(!XConv_IsDone(&conv_ip));
			XAxiDma_Reset(&axiDMA0);
			while(!XAxiDma_ResetIsDone(&axiDMA0)){}
			XAxiDma_Reset(&axiDMA1);
			while(!XAxiDma_ResetIsDone(&axiDMA1)){}
			XAxiDma_Reset(&axiDMA2);
			while(!XAxiDma_ResetIsDone(&axiDMA2)){}
			printf("HW Calculation Complete!(Layer : %d.2)\n",curr_layer);
		}
	}


	/*
	return;
	*/
	////////////////  Transposed convolution  //////////////////////////
	//conv_out is the input image

	for(int curr_layer=6; curr_layer<10; curr_layer++)
	{

		num_f = calc_f_num(curr_layer);


		XTconv_Set_slaveIn_ch(&tconv_ip, ch_num);
		XTconv_Set_slaveIn_dim(&tconv_ip, dim);
		XTconv_Set_slaveIn_f_num(&tconv_ip, num_f);
		XTconv_Start(&tconv_ip);

		int o_dim = dim*2;
		int o_ch = num_f;

		Xil_DCacheFlushRange((UINTPTR)temp0, ch_num*dim*dim*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)f_dc[curr_layer-6], num_f*ch_num*4*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)b_dc[curr_layer-6], num_f*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float));

		//printf("Sending Data to IP core slave\n");
		XAxiDma_SimpleTransfer(&axiDMA3, (UINTPTR)temp0, ch_num*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Image . . .\n");
		//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA4, (UINTPTR)f_dc[curr_layer-6], num_f*ch_num*4*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Filter . . .\n");
		//while(XAxiDma_Busy(&axiDMA1, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA5, (UINTPTR)b_dc[curr_layer-6], num_f*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Bias . . .\n");
		//while(XAxiDma_Busy(&axiDMA2, XAXIDMA_DMA_TO_DEVICE));

		//printf("Getting Data . . .\n");
		XAxiDma_SimpleTransfer(&axiDMA3, (UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);


		///// prepare the skip connection memory buffer for the next conv
		float *skip = conv_arr[9-curr_layer];
		////////////////////////////////////


		//Invalidate the cache to avoid reading garbage
		Xil_DCacheInvalidateRange((UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float));
		//printf("Waiting for IP to Terminate . . .\n");
		while(!XTconv_IsDone(&tconv_ip));
		printf("HW Calculation Complete!(Layer : %d)\n",curr_layer);
		XAxiDma_Reset(&axiDMA3);
		while(!XAxiDma_ResetIsDone(&axiDMA3)){}
		XAxiDma_Reset(&axiDMA4);
		while(!XAxiDma_ResetIsDone(&axiDMA4)){}
		XAxiDma_Reset(&axiDMA5);
		while(!XAxiDma_ResetIsDone(&axiDMA5)){}




		ch_num = num_f;
		dim = o_dim;


		/////// CONCAT ////////

		for (int i=0;i<ch_num; i++)
		{
			for (int j=0;j<dim;j++)
			{
				for (int k=0;k<dim;k++)
				{
					temp0[i*dim*dim +j*dim + k] = temp1[i*dim*dim +j*dim + k];//#TODO :It can extend temp1 just by adding skip at the end(it will cause temp0/1 loop problems(fix:use skip as temp in conv1.1 res))
					temp0[(i+ch_num)*dim*dim +j*dim + k] = skip[i*dim*dim +j*dim + k];
				}
			}
		}


		// free curr skip
		//
		///////////////////////

		ch_num = ch_num*2;


		//////////////////////// CONV BLOCK ///////////////////////////


		XConv_Set_slaveIn_ch(&conv_ip, ch_num);
		XConv_Set_slaveIn_dim(&conv_ip, dim);
		XConv_Set_slaveIn_f_num(&conv_ip, num_f);
		XConv_Start(&conv_ip);


		o_dim = dim;
		o_ch = num_f;

		//Flush the cache of the buffers
		//printf("Flushing Cache\n");
		Xil_DCacheFlushRange((UINTPTR)temp0, ch_num*dim*dim*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)filters[(curr_layer-1)*2], num_f*ch_num*9*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)bias[(curr_layer-1)*2], num_f*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float));

		//printf("Sending Data to IP core slave\n");
		XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp0, ch_num*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Image . . .\n");
		//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA1, (UINTPTR)filters[(curr_layer-1)*2], num_f*ch_num*9*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Filter . . .\n");
		//while(XAxiDma_Busy(&axiDMA1, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA2, (UINTPTR)bias[(curr_layer-1)*2], num_f*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Bias . . .\n");
		//while(XAxiDma_Busy(&axiDMA2, XAXIDMA_DMA_TO_DEVICE));

		//printf("Getting Data . . .\n");
		XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);

		//Invalidate the cache to avoid reading garbage
		Xil_DCacheInvalidateRange((UINTPTR)temp1, o_ch*o_dim*o_dim*sizeof(float));
		//printf("Waiting for IP to Terminate . . .\n");
		while(!XConv_IsDone(&conv_ip));
		XAxiDma_Reset(&axiDMA0);
		while(!XAxiDma_ResetIsDone(&axiDMA0)){}
		XAxiDma_Reset(&axiDMA1);
		while(!XAxiDma_ResetIsDone(&axiDMA1)){}
		XAxiDma_Reset(&axiDMA2);
		while(!XAxiDma_ResetIsDone(&axiDMA2)){}
		printf("HW Calculation Complete!(Layer : %d.1)\n",curr_layer);

		ch_num = num_f;


		XConv_Set_slaveIn_ch(&conv_ip, ch_num);
		XConv_Set_slaveIn_dim(&conv_ip, dim);
		XConv_Set_slaveIn_f_num(&conv_ip, num_f);
		XConv_Start(&conv_ip);

		//printf("Flushing Cache\n");
		Xil_DCacheFlushRange((UINTPTR)temp1, ch_num*dim*dim*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)filters[(curr_layer-1)*2 +1], num_f*ch_num*9*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)bias[(curr_layer-1)*2 +1], num_f*sizeof(float));
		Xil_DCacheFlushRange((UINTPTR)temp0, o_ch*o_dim*o_dim*sizeof(float));


		//printf("Sending Data to IP core slave\n");
		XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp1, ch_num*dim*dim*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Image . . .\n");
		//while(XAxiDma_Busy(&axiDMA0, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA1, (UINTPTR)filters[(curr_layer-1)*2 +1], num_f*ch_num*9*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Filter . . .\n");
		//while(XAxiDma_Busy(&axiDMA1, XAXIDMA_DMA_TO_DEVICE));
		XAxiDma_SimpleTransfer(&axiDMA2, (UINTPTR)bias[(curr_layer-1)*2 +1], num_f*sizeof(float), XAXIDMA_DMA_TO_DEVICE);
		//printf("Sending Bias . . .\n");
		//while(XAxiDma_Busy(&axiDMA2, XAXIDMA_DMA_TO_DEVICE));

		//printf("Getting Data . . .\n");
		XAxiDma_SimpleTransfer(&axiDMA0, (UINTPTR)temp0, o_ch*o_dim*o_dim*sizeof(float), XAXIDMA_DEVICE_TO_DMA);


		Xil_DCacheInvalidateRange((UINTPTR)temp0, o_ch*o_dim*o_dim*sizeof(float));
		while(!XConv_IsDone(&conv_ip));
		XAxiDma_Reset(&axiDMA0);
		while(!XAxiDma_ResetIsDone(&axiDMA0)){}
		XAxiDma_Reset(&axiDMA1);
		while(!XAxiDma_ResetIsDone(&axiDMA1)){}
		XAxiDma_Reset(&axiDMA2);
		while(!XAxiDma_ResetIsDone(&axiDMA2)){}
		printf("HW Calculation Complete!(Layer : %d.2)\n",curr_layer);

	}



	////////// Last(single conv) layer !!!!! ////
	int curr_layer = 10;
	num_f = 1;
	int o_dim = dim;
	int offset=(curr_layer-1)*2;
	float bias_t = bias[offset][0];
	float sum;

	for(int x=0; x<o_dim; x++)
	{
		for(int y=0; y<o_dim; y++)
		{
			sum=0;
			//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
			for(int j=0; j < ch_num ; j++) // #TODO UNROLL
			{
				sum += temp0[j*dim*dim +x*dim +y]*filters[offset][j];
			}
			temp1[x*dim+ y] = sum + bias_t;
		}
	}

	///
	printf("\n---------------------------------------\nResult:\n\n");
	for (int j=0;j<dim/16;j++)
	{
		for (int k=0;k<dim/16;k++)
			printf("%f\t",temp1[j*dim + k]);
		printf("\n");
	}


	//normalize_custom(temp1);//it changes the conv_out ifself

	//float accuracy = Dice_Coef(temp1, label,dim);
	//printf("\n\nAccuracy: %.2f % \n\n", (accuracy*100));
}
