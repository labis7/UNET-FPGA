/*
 * Pooling.c
 *
 *  Created on: Jun 19, 2020
 *      Author: labis
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <header.h>


void maxpool(maxpool_data_ *ptr_maxpool_data)
{

	int ch_num,dim,f=2,s=2; //f:size of 'filter' , s: stride of 2
	float ***image;
	float ***output;
	ch_num = ptr_maxpool_data->channels;
	dim = ptr_maxpool_data->dim;
	image = ptr_maxpool_data->image;

	int o_dim = (int)((dim - f)/s) +1;
	output = make_3darray(ch_num, o_dim);
	float max = -100000;//very small number

	for(int i=0; i<ch_num; i++)
	{
		for(int x=0; x<o_dim; x++)
		{
			for (int y = 0; y<o_dim; y++)
			{
				max = -100000;
				for(int j=s*x; j<(s*x+f) ;j++)///take and examine for max element a sub-part(2x2) of the main matrix
				{
					for(int z=s*y; z<(s*y+f); z++)
					{
						if(image[i][j][z]> max)
							max = image[i][j][z];
					}
				}
				//Now we have the sub-part max values, we can save it
				output[i][x][y] = max;
			}
		}
	}
	ptr_maxpool_data->output=output;
	ptr_maxpool_data->o_dim=o_dim;
}



