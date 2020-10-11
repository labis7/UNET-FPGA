/*
 * Pooling.c
 *
 *  Created on: Jun 19, 2020
 *      Author: labis
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <main.h>


void maxpool(struct maxpool_data_ *ptr_maxpool_data)
{

	int ch_num,dim,f=2,s=2; //f:size of 'filter' , s: stride of 2
	float ***image;
	float ***output;
	ch_num = ptr_maxpool_data->channels;
	dim = ptr_maxpool_data->dim;
	image = ptr_maxpool_data->image;

	int o_dim = (int)((dim - f)/s) +1;
	output = make_3darray(ch_num, o_dim);
	float max = -100000;//very small number(-inf)

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

	//save results
	ptr_maxpool_data->output=output;
	ptr_maxpool_data->o_dim=o_dim;
}



/// For  Training purpose ///
/*
void maxpool_backward(struct maxpoolbackward_data_ *ptr_maxpoolbackward_data)
{
	int f=2,s=2,ch_num,dim,o_dim,max,index;
	float ***dpool,***conv,***output;
	o_dim = (dim-1)*s+f;
	ch_num = ptr_maxpoolbackward_data->channels;
	dim = ptr_maxpoolbackward_data->dim;
	dpool = ptr_maxpoolbackward_data->dpool;
	conv = ptr_maxpoolbackward_data->conv;

	output = make_3darray(ch_num,o_dim);
	//Important: We need to fill output with zeros!!!

	for (int i=0; i<ch_num; i++)
	{
		for (int x=0; x<dim ; x++)
		{
			for(int y=0; y<dim ; y++)
			{
				max = -100000;
				for(int j=s*x; j<(s*x+f) ;j++)///take and examine for max element a sub-part(2x2) of the main matrix
					{
						for(int z=s*y; z<(s*y+f); z++)
						{
							if(conv[i][j][z]> max)
							{
								max = conv[i][j][z];
								//00=0, 01=1, 10=2, 11=3
								index = (2-((s*x+f)-j))*2+(2-((s*y+f)-z));
							}
						}
					}
				if(index == 0)
				{
					output[i][x*s][y*s] = dpool[i][x][y];
					output[i][x*s+0][y*s+1] = 0;
					output[i][x*s+1][y*s+0] = 0;
					output[i][x*s+1][y*s+1] = 0;
				}
				else if(index == 1)
				{
					output[i][x*s][y*s] = 0;
					output[i][x*s+0][y*s+1] = dpool[i][x][y];
					output[i][x*s+1][y*s+0] = 0;
					output[i][x*s+1][y*s+1] = 0;
				}
				else if (index == 2)
				{
					output[i][x*s][y*s] = 0;
					output[i][x*s+0][y*s+1] = 0;
					output[i][x*s+1][y*s+0] = dpool[i][x][y];
					output[i][x*s+1][y*s+1] = 0;
				}
				else
				{
					output[i][x*s][y*s] = 0;
					output[i][x*s+0][y*s+1] = 0;
					output[i][x*s+1][y*s+0] = 0;
					output[i][x*s+1][y*s+1] = dpool[i][x][y];
				}
			}
		}
	}
	ptr_maxpoolbackward_data->output = output;
	ptr_maxpoolbackward_data->o_dim = o_dim;
}
*/

