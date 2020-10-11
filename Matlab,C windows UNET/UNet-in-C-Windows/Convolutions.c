/*
 * Convolutions.c
 *
 *  Created on: Jun 19, 2020
 *      Author: labis
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <main.h>


//////////////////////// CONVOLUTIONS //////////////////////////

void conv(struct conv_data_ *ptr_conv_data)
{
	//f_dim = 3, stride = 1.
	float ***conv_in,***conv_out;
	float ****filter,*bias;
	int dim,f_num,ch_num,o_dim, mode;
	//Unpacking data
	conv_in = ptr_conv_data->conv_in;
	filter = ptr_conv_data->filter;
	bias = ptr_conv_data->bias;
	dim = ptr_conv_data->dim;
	mode = ptr_conv_data->mode;  //padding mode
	ch_num = ptr_conv_data->ch_num;//input channel nunmber
	f_num = ptr_conv_data->f_num; //filter number
	int s=1;                      //stride
	int f = ptr_conv_data->f_dim; // transp convulution :f=2 , convolution : f=3

	if (mode >= 1)//padding enabled - probably 1 which means keep the same dim as input
	{
		int pad = mode;
		o_dim = dim;
		int dim_t = dim + 2*pad;
		float ***conv_in_t = make_3darray(ch_num, dim_t);
		conv_out = make_3darray(f_num, o_dim); //number of filters will determine the number of out image channels, dim will be the same in this case.
		//zero padding
		for(int i=0; i< ch_num; i++)
		{
			for(int x = 0; x<pad ; x++)
			{
				for(int y = 0; y< dim_t; y++)
				{
					//Zero-padding
					conv_in_t[i][x][y] = 0;
					conv_in_t[i][y][x] = 0;
					conv_in_t[i][(dim_t-1)-x][y] = 0;
					conv_in_t[i][y][(dim_t-1)-x] = 0;
				}
			}

			//fill the empty center space with conv_in--> then the result will be the conv_in padded(conv_in_t)
			for(int x=pad; x<(dim_t-pad); x++)
				for(int y=pad; y<(dim_t-pad); y++)
					conv_in_t[i][x][y] = conv_in[i][x-pad][y-pad];
		}

		// Now we can start the convolution
		float sum;
		for (int i=0; i<f_num; i++)//number of filters
		{
			for(int x=0; x<o_dim; x++)//output height
			{
				for(int y=0; y<o_dim; y++)//output width
				{
					sum=0;
					//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
					for(int j=0; j < ch_num ; j++)
					{
						for(int k=x; k<(x + f); k++)
						{
							for(int l =y; l<(y+f); l++)
							{
								sum += conv_in_t[j][k][l]*filter[i][j][k-x][l-y];
							}
						}
					}
					conv_out[i][x][y] = sum + bias[i];
				}
			}
		}
	}

	else//mode = pad = 0 = 'normal'
	{
		o_dim = ((dim - f)/s) +1 ;
		conv_out = make_3darray(f_num, o_dim); //number of filters will determine the number of out image channels, dim will be the same in this case.
		float sum;
		for (int i=0; i<f_num; i++)//number of filters
		{
			for(int x=0; x<o_dim; x++)//output height
			{
				for(int y=0; y<o_dim; y++)//output width
				{
					sum=0;
					//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
					for(int j=0; j < ch_num ; j++)
					{
						for(int k=x; k<(x + f); k++)
						{
							for(int l =y; l<(y+f); l++)
							{
								sum += conv_in[j][k][l]*filter[i][j][k-x][l-y];
							}
						}
					}
					conv_out[i][x][y] = sum + bias[i];
				}
			}
		}

	}
	//push results into structure
	ptr_conv_data->conv_out = conv_out;
	ptr_conv_data->o_dim = o_dim;

	//number of channels is known before func call,(o_ch)num == f_num)
}

void convTransp(struct conv_data_ *ptr_conv_data)
//Applying the simple transposed convolution algorithm- Not fpga friendly algorithm
{
	float ***conv_in,***conv_in_t,***conv_out;
	float ****filter_unrot,*bias;
	int dim,f_num,ch_num,o_dim;
	conv_in = ptr_conv_data->conv_in;
	filter_unrot = ptr_conv_data->filter;
	bias = ptr_conv_data->bias;
	dim = ptr_conv_data->dim;
	ch_num = ptr_conv_data->ch_num;
	f_num = ptr_conv_data->f_num;
	//int s=1;
	int f = ptr_conv_data->f_dim; // f=2



	//Rotate 2*90degrees the filter matrix
	float ****filter=make_4darray(f_num,ch_num,f);
	for (int i=0; i<f_num; i++)//number of filters
	{
		for(int k=0; k<ch_num; k++)
		{
			//We know that the filter will always be 2x2 so...
			filter[i][k][1][1]= filter_unrot[i][k][0][0];
			filter[i][k][0][0]= filter_unrot[i][k][1][1];
			filter[i][k][1][0]= filter_unrot[i][k][0][1];
			filter[i][k][0][1]= filter_unrot[i][k][1][0];
		}
	}

	//Now its to make the new conv_in where we are going to apply the convolution(basic-normal) with a 2x2 filter
	o_dim = dim*2;
	int dim_t = dim*2 + 1;
	conv_in_t = make_3darray(ch_num, dim_t);

	//Fill with zeros///
	for (int i=0; i<ch_num; i++)
		for(int x=0; x<dim_t; x++)
			for(int y=0; y<dim_t; y++)
				conv_in_t[i][x][y] = 0;
	////////////////////
	//Fill the appropriate slots with data(with respect to : zero-insertion between data=1, pad=1 )
	/*
	 * MOre about how the above number occurred:
	 * s is always 1, upsample kernerl f=2
	 * zero insertions between pixels (s_downsampled-1 = 2-1 =1
	 * required padding in order to double my dimensions with the given data:
	 * (i-1)*2 + k -2*p = output_size, where our padding is k - p -1 = 2-0-1=1(we assume p=0)
	 */
	for (int i=0; i<ch_num; i++)
		for(int x=1; x<dim_t; x+=2)
			for(int y=1; y<dim_t; y+=2)
				conv_in_t[i][x][y] = conv_in[i][(int)(x/2)][(int)(y/2)];

	// Convolution 'normal' (padding=0) //
	//o_dim = (dim_t -2)/1 +1;   OR   o_dim = dim*2
	conv_out = make_3darray(f_num, o_dim); //number of filters will determine the number of out image channels, dim will be the same in this case.
	float sum;
	for (int i=0; i<f_num; i++)//number of filters
	{
		for(int x=0; x<o_dim; x++)//output height
		{
			for(int y=0; y<o_dim; y++)//output width
			{
				sum=0;
				//seeking on the temp image sub array that we want to mult item wise and then add them for the (x,y) result
				for(int j=0; j < ch_num ; j++)
				{
					for(int k=x; k<(x + f); k++)
					{
						for(int l =y; l<(y+f); l++)
						{
							sum += conv_in_t[j][k][l]*filter[i][j][k-x][l-y];
						}
					}
				}
				conv_out[i][x][y] = sum + bias[i]; //add the bias per filter
			}
		}
	}

	//update structure's output result
	ptr_conv_data->conv_out = conv_out;
	ptr_conv_data->o_dim = o_dim;
}




///////////////////////////// extras ///////////////////////////////////

void crop2half(struct concat_crop_data_ *ptr_concat_crop_data)
{
	float ***image1,***image2,***image3;
	image1 = ptr_concat_crop_data->image1;
	int ch_num = ptr_concat_crop_data->ch_num;
	int dim = ptr_concat_crop_data->dim;
	int o_ch_num = (int)(ch_num/2);

	image2 = make_3darray(o_ch_num, dim);
	image3 = make_3darray(o_ch_num, dim);
	/*
	*starting with the 1st half --> image2,
	* then using the offset o_ch_num+i we
	* can save the other half to the image3 at same time
	*/
	for (int i =0; i < o_ch_num ; i++)
	{
		for (int x =0; x<dim; x++)
		{
			for (int y=0; y<dim; y++)
			{
				image2[i][x][y] = image1[i][x][y];
				image3[i][x][y] = image1[o_ch_num+i][x][y];
			}
		}
	}

	// update structure's output results
	ptr_concat_crop_data->image2 = image2;
	ptr_concat_crop_data->image3 = image3;

	ptr_concat_crop_data->o_ch_num = o_ch_num;

}

void concat(struct concat_crop_data_ *ptr_concat_crop_data)
{
	float ***image1, ***image2, ***image3;
	image2=ptr_concat_crop_data->image1; // !!! UPDATED AND MATCHES TO KERAS CONCAT !!! (DC image first, then Skip connection)
	image1=ptr_concat_crop_data->image2;
	int dim = ptr_concat_crop_data->dim;// dimensions for both image1,2 (which is the same)
	int ch_num = ptr_concat_crop_data->ch_num;
	int o_ch_num = ch_num*2;
	image3 = make_3darray(o_ch_num, dim);

	for (int i =0; i < ch_num ; i++)
	{
		for (int x =0; x<dim; x++)
		{
			for (int y=0; y<dim; y++)
			{
				image3[i][x][y] = image1[i][x][y]; //put Deconvolution result first
				image3[i+ch_num][x][y] = image2[i][x][y];//then put skip connection
			}
		}
	}

	//update structure's output results
	ptr_concat_crop_data->image3 = image3;
	ptr_concat_crop_data->o_ch_num = o_ch_num;

}
