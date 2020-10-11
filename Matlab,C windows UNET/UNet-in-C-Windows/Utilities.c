/*
 ============================================================================
 Name        : Utilities.c
 Author      : Labiz
 Version     : V0.5
 Copyright   : Your copyright notice
 Description : Utilities of U-Net, including initializations and important 3d,4d matrix building tools
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <main.h>
#include <time.h>

////////////// Function Naming init. ////////////////

////////////////////////////////////////////////////



// Training - only //
void Initialize_GN(struct init_GN_ *ptr_init_GN)//Group Normalization Init.
{
	int layers, num_ch, trim;
	layers = ptr_init_GN->layers; //We PUT THE FORWARD number of layers!! so it can match the other ini func
	trim = ptr_init_GN->trim; //optimal setting:0.05.
	num_ch = ptr_init_GN->starting_num_ch;//DEFAULT:16
	float **gamma = (float **)malloc((layers*2*2-1)*sizeof(float *));
	float **beta = (float **)malloc((layers*2*2-1)*sizeof(float *));

	//Following the same logic as the init_param function:
	//First we build the forward pass gamma/beta, then the upsampling and finally the out gamma/beta(10th layer).
	int last_pos=0,loc=0;
	for (int i=0; i<layers; i++)
	{
		if(i != 0) //after 1st iteration, double the amount of filters of each layer
			num_ch = num_ch*2; //*16*, 32, 64, 128, 256
		float *gamma1 = (float *)malloc(num_ch*sizeof(float));
		float *gamma2 = (float *)malloc(num_ch*sizeof(float));
		float *beta1 = (float *)malloc(num_ch*sizeof(float));
		float *beta2 = (float *)malloc(num_ch*sizeof(float));
		for (int x=0; x < num_ch; x++)
		{
			gamma1[x] =  Random_Normal(loc, trim);
			gamma2[x] =  Random_Normal(loc, trim);
			beta1[x] =  Random_Normal(loc, trim);
			beta2[x] =  Random_Normal(loc, trim);
		}

		gamma[2*i] = gamma1;
		gamma[2*i+1] = gamma2;
		beta[2*i] = beta1;
		beta[2*i+1] = beta2;
		last_pos++;
	}

	for (int i=1; i<layers; i++)
	{
		num_ch = (int)num_ch/2;//It will be always power of number of filters,so the division will give back integer

		float *gamma1 = (float *)malloc(num_ch*sizeof(float));
		float *gamma2 = (float *)malloc(num_ch*sizeof(float));
		float *beta1 = (float *)malloc(num_ch*sizeof(float));
		float *beta2 = (float *)malloc(num_ch*sizeof(float));
		for (int x=0; x < num_ch; x++)
		{
			gamma1[x] =  Random_Normal(loc, trim);
			gamma2[x] =  Random_Normal(loc, trim);
			beta1[x] =  Random_Normal(loc, trim);
			beta2[x] =  Random_Normal(loc, trim);
		}

		gamma[2*last_pos] = gamma1;
		gamma[2*last_pos+1] = gamma2;
		beta[2*last_pos] = beta1;
		beta[2*last_pos+1] = beta2;
		last_pos++;
	}
	//Now we build last layer/output group normalization paramenters
	num_ch = 1;

	float gamma1; //= (float *)malloc(num_ch*sizeof(float));
	float beta1; //= (float *)malloc(num_ch*sizeof(float));

	gamma1 = Random_Normal(loc, trim);
	beta1 =	Random_Normal(loc, trim);

	gamma[last_pos*2] = &gamma1;
	beta[last_pos*2] = &beta1;

	//Filling up the 'big' arrays of all the layer data
	ptr_init_GN->gamma = gamma;
	ptr_init_GN->beta = beta;


}

/// Training - Only ///
void Initialize_Parameters(struct init_param_ *ptr_init_param)
{
	printf("Called\n");
	int layers = ptr_init_param->layers;//number of layers(just the encoder-downsampling number)
	int num_f = ptr_init_param->num_f;  //initial number of filters(they will be doubled for each layer)
	float trim = ptr_init_param->trim;  //weight scale of the values
	float *****filters = (float *****)malloc((layers*2*2-1)*sizeof(float ****));
	float **bias = (float **)malloc((layers*2*2-1)*sizeof(float *));
	float *****f_dc = (float *****)malloc((layers-1)*sizeof(float ****));
	float **b_dc = (float **)malloc((layers-1)*sizeof(float *));

	int ch_in = 1; //number of initial channels input from the input image
	int loc = 0; //normal distribution around zero

	int last_pos=0;
	//Building the downsampling/encoder filters first.(5 layers*2 filters each)
	for (int i=0; i<layers; i++)
	{
		if(i != 0) //after 1st iteration, double the amount of filters of each layer
			num_f = num_f*2; //*16*, 32, 64, 128, 256
		//Building f1
		float ****f1 = make_4darray(num_f, ch_in, 3);//3x3 filter alwars for the simple convolution
		float *b1 = (float *)malloc(num_f*sizeof(float));
		for (int x=0;x<num_f;x++)
		{
			for (int y=0;y<ch_in;y++)
				for (int z=0;z<3;z++)
					for(int w=0;w<3;w++)
					{
						f1[x][y][z][w] = Random_Normal(loc, trim);
					}
			b1[x] =  Random_Normal(loc, trim);
		}
		printf("Calling...\n");
		/*
		 * Very important! Next filter will have as input the output/channels of the previous filter
		 * More : When we apply a number of filters on an image, we create num_f channels on the image result.
		 * So each filter must have input dim same as the input image channels, output dim = channels we want to occur after conv.
		 */
		ch_in = num_f;

		float ****f2 = make_4darray(num_f, ch_in, 3);//3x3 filter alwars for the simple convolution
		float *b2 = (float *)malloc(num_f*sizeof(float));
		for (int x=0;x<num_f;x++)
		{
			for (int y=0;y<ch_in;y++)
				for (int z=0;z<3;z++)
					for(int w=0;w<3;w++)
					{
						f2[x][y][z][w] = Random_Normal(loc, trim);
					}
			b2[x] = Random_Normal(loc, trim);
		}
		filters[2*i] = f1;
		filters[2*i +1] = f2;
		bias[2*i] = b1;
		bias[2*i +1] = b2;
		last_pos++; //last position of the i that shows the current layer, we will need it later so we keep saving filters and
		//bias sequencially.
	}
	for (int i = 1 ; i < layers; i++)
	{
		num_f = (int)num_f/2;//It will be always power of number of filters,so the division will give back integer

		float ****fdc = make_4darray(num_f, ch_in, 2);//2x2 filter always for the upsampling/transposed convolution
		float *bdc = (float *)malloc(num_f*sizeof(float));
		float ****f1 = make_4darray(num_f, ch_in, 3);//3x3 filter always for the simple convolution
		float *b1 = (float *)malloc(num_f*sizeof(float));
		for (int x=0;x<num_f;x++)
		{
			for (int y=0;y<ch_in;y++)
			{
				for (int z=0;z<3;z++)
				{
					for(int w=0;w<3;w++)
						f1[x][y][z][w] = Random_Normal(loc, trim);
				}
				for (int z=0;z<2;z++)
				{
					for(int w=0;w<2;w++)
						fdc[x][y][z][w] = Random_Normal(loc, trim);
				}
			}
			bdc[x] =  Random_Normal(loc, trim);
			b1[x] = Random_Normal(loc, trim);
		}

		ch_in = num_f ;
		
		float ****f2 = make_4darray(num_f, ch_in, 3);//3x3 filter always for the simple convolution
		float *b2 = (float *)malloc(num_f*sizeof(float));
		for (int x=0;x<num_f;x++)
		{
			for (int y=0;y<ch_in;y++)
				for (int z=0;z<3;z++)
					for(int w=0;w<3;w++)
					{
						f2[x][y][z][w] = Random_Normal(loc, trim);
					}
			b2[x] =  Random_Normal(loc, trim);
		}

		filters[last_pos*2] = f1;
		filters[last_pos*2 + 1] = f2;
		bias[last_pos*2] = b1;
		bias[last_pos*2 + 1] = b2;
		f_dc[i-1] = fdc;
		b_dc[i] = bdc;
		last_pos++;
	}
	printf("\nlast_pos:%d",last_pos);
	//Now build the last one 1x1 conv filters
	num_f = 1; //we need 1 channels for the output(same as input)
	float ****out_f = make_4darray(num_f, ch_in, 1);//1x1 output filter
	float *out_b = (float *)malloc(num_f*sizeof(float));
	for (int x=0;x<num_f;x++)
	{
		for (int y=0;y<ch_in;y++)
			for (int z=0;z<1;z++)
				for(int w=0;w<1;w++)
				{
					out_f[x][y][z][w] = Random_Normal(loc, trim);
				}
		out_b[x] =  Random_Normal(loc, trim);
	}
	filters[last_pos*2] = out_f;
	bias[last_pos*2] = out_b;

	ptr_init_param->filters = filters;
	ptr_init_param->bias = bias;
	ptr_init_param->f_dc = f_dc;
	ptr_init_param->b_dc = b_dc;
}

void GN(struct gn_data_ *gn_data)
{
	int eps=1e-5;
	float *beta,*gamma;
	float ivar, sqrtvar, var, mu;
	float ***xmu, ***xhat, ***gammax, ***out;
	int batch, ch_num, dim;
	batch=gn_data->batch;
	ch_num=gn_data->ch_num;
	dim=gn_data->dim;
	gamma=gn_data->gamma;
	beta=gn_data->beta;
	float ***image = gn_data->image;
	// make empty arrays that will need later
	xmu = make_3darray(batch,dim);//we need only size of batch in order to calculate the partial out on each loop
	xhat= make_3darray(batch,dim);
	gammax= make_3darray(batch,dim);
	out = make_3darray(ch_num,dim);
	/////////////////////////////////////////
	for (int i=0; i<ch_num; i+=batch)
	{
		//each loop is interested on i->i+batch channels,
		//with the data(gamma,beta) be the elements (int)(i//batch)

		//step1:calculate mean(scalar)
		mu=0;
		for(int k=i; k<(i+batch); k++)
			for(int x=0; x<dim; x++)
				for(int y=0; y<dim; y++)
					mu += image[k][x][y];
		mu = (float)(mu/(dim*dim*batch));

		//step2:subtract mean vector of every training example
		for(int k=i; k<(i+batch); k++)
			for(int x=0; x<dim; x++)
				for(int y=0; y<dim; y++)
					xmu[k-i][x][y] = image[k][x][y] - mu;

		//step3: following the lower branch - calculation denominator
		//step4: calculate variance
		var=0;
		for(int k=0; k<batch; k++)
			for(int x=0; x<dim; x++)
				for(int y=0; y<dim; y++)
					var+=pow(xmu[k][x][y],2);
		var = (float)(var/(batch*dim*dim));

		//step5: add eps for numerical stability, then sqrt
		sqrtvar = sqrt((var+eps));
		//step6: invert sqrtvar
		ivar = (float)(1/sqrtvar);

		//step7: execute normalization
		for(int k=0; k<batch; k++)
			for(int x=0; x<dim; x++)
				for(int y=0; y<dim; y++)
					xhat[k][x][y] = xmu[k][x][y] * ivar;


		//step8: Nor the two transformation steps
		int pos =(int)(i/batch);
		float gamma_t=gamma[pos];
		for(int k=0; k<batch; k++)
			for(int x=0; x<dim; x++)
				for(int y=0; y<dim; y++)
					gammax[k][x][y]=gamma_t*xhat[k][x][y];
		//step9: output
		float beta_t = beta[(int)(i/batch)];
		for(int k=i; k<(i+batch); k++)
			for(int x=0; x<dim; x++)
				for(int y=0; y<dim; y++)
					out[k][x][y]= gammax[k-i][x][y] + beta_t;
	}
	gn_data->out = out;
	//step5: add eps for numerical stability, then sqrt

}

void Activation_Function(struct act_func_data_ *act_func_data)
{
	float ***Z;
	int code, channels, dim;
	Z = act_func_data->Z;
	code = act_func_data->code;
	channels = act_func_data->channels;
	dim = act_func_data->dim;

	float ***res = make_3darray(channels, dim);
	//Approximation of Sigmoid,  f(x) = x / (1 + abs(x)), --> faster
	if(code == 1)
	{
		for (int i=0; i<channels; i++)
			for (int j=0; j<dim; j++)
				for (int k=0; k<dim; k++)
					res[i][j][k] = (1/(1 + exp(- Z[i][j][k]))); //Thats the original sigmoid!
	}
	else if(code == 2)//Sigmoid backpropagation function -- Training - Only
	{
		float ***dA;
		dA =act_func_data->res;
		for (int i=0; i<channels; i++)
			for (int j=0; j<dim; j++)
				for (int k=0; k<dim; k++)
					res[i][j][k] = (1/(1 + exp(- Z[i][j][k])));//find sig
		for (int i=0; i<channels; i++)
			for (int j=0; j<dim; j++)
				for (int k=0; k<dim; k++)
					res[i][j][k] = (dA[i][j][k])*(res[i][j][k])*(1 - res[i][j][k]);
	}
	else if(code == 3) //ReLu activation function
	{
		int z =0;
		for (z=0; z<channels; z++)
		{
			for (int j=0; j<dim; j++)
			{
				for (int k=0; k<dim; k++)
				{
					if(Z[z][j][k] <= 0)
						res[z][j][k] = 0;
					else
						res[z][j][k] = Z[z][j][k];
				}
			}
		}
	}
	else //Relu backpropagation function   -- TRaining - ONly
	{
		float ***dA;
		dA = act_func_data->res;
		int z=0;
		for (z=0; z<channels; z++)
		{
			for (int j=0; j<dim; j++)
			{
				for (int k=0; k<dim; k++)
				{
					if(Z[z][j][k] <= 0)
						res[z][j][k] = 0;
					else
						res[z][j][k] = dA[z][j][k];
				}
			}
		}
	}
	act_func_data->res = res;
	//return res;
}


// Loss Function
float Dice_Coef(float ***logs, float ***target,int dim)
{

	int mylen = dim*dim;
	//building numerator(logs*target)  //#TODO :many optimization available
	float ***numer = make_3darray(1,dim);
	float ***denom = make_3darray(1,dim);
	for(int i=0; i<1; i++)
		for(int x=0; x<dim; x++)
			for(int y=0; y<dim; y++)
			{
				numer[i][x][y]=(logs[i][x][y])*(target[i][x][y]);
				denom[i][x][y] = logs[i][x][y] + target[i][x][y];
			}
	float sum_num=0,sum_den=0;
	for(int i=0; i<1; i++)
		for(int x=0; x<dim; x++)
			for(int y=0; y<dim; y++)
			{
				sum_num += numer[i][x][y];
				sum_den += denom[i][x][y];
			}
	float loss = 1 - (float)((2*sum_num)/(sum_den));
	return (float)exp(-loss);

}


// 2D ARRAY space creation  Given resolution
float **make_2darray(int dim1,int dim2)
{
	float **array = (float **)malloc(dim1*sizeof(float*));
	for (int i = 0; i< dim1; i++)
		array[i] = (float *) malloc(dim2*sizeof(float));
	return array;
}
// 3D ARRAY space creation  Given resolution and channels
float ***make_3darray(int channels,int dim)
{
	int dim1=channels, dim2=dim, dim3=dim;
	int i,j;
	float *** array = (float ***)malloc(dim1*sizeof(float**));

	for (i = 0; i< dim1; i++)
	{
		array[i] = (float **) malloc(dim2*sizeof(float *));
		for (j = 0; j < dim2; j++)
			array[i][j] = (float *)malloc(dim3*sizeof(float));
	}
	return array;
}
uint32_t ***make_3darray_u(int channels,int dim)
{
	int dim1=channels, dim2=dim, dim3=dim;
	int i,j;
	uint32_t *** array = (uint32_t ***)malloc(dim1*sizeof(uint32_t**));

	for (i = 0; i< dim1; i++)
	{
		array[i] = (uint32_t **) malloc(dim2*sizeof(uint32_t *));
		for (j = 0; j < dim2; j++)
			array[i][j] = (uint32_t *)malloc(dim3*sizeof(uint32_t));
	}
	return array;
}

// 4D ARRAY space creation  Given resolution,channels and filter number
float ****make_4darray(int num,int channels,int dim)
{
	int dim0=num, dim1=channels, dim2=dim, dim3=dim;
	int i,j,k;
	float **** array = (float ****)malloc(dim0*sizeof(float***));

	for (i = 0; i< dim0; i++)
	{
		array[i] = (float ***) malloc(dim1*sizeof(float **));
		for (j = 0; j < dim1; j++) {
			array[i][j] = (float **)malloc(dim2*sizeof(float *));
			for (k = 0; k < dim2; k++)
				array[i][j][k] = (float *)malloc(dim3*sizeof(float));
		}
	}
	return array;
}

// Training - Only (Initialization of the weights)
float Random_Normal(int loc, float scale)
{
   // return a normally distributed random value
	scale = 1;
	loc=0;
	float v1 = ( (float)(rand()) + 1. )/( (float)(RAND_MAX) + 1. ); //random gen
	float v2 = ( (float)(rand()) + 1. )/( (float)(RAND_MAX) + 1. );//random gen
	return ((cos(2*3.14*v2)*sqrt(-2.*log(v1)))*scale + loc);
}

//Extreme value delimiter + smart accuracy fixer
void normalize_custom(struct norm_data_ *norm_data)
{
	float ***image = norm_data->image;
	int dim = norm_data->dim;
	//float ***res = make_3darray(1,dim,dim);
	int code = norm_data->code;
	if(code == 0) //normalize possible inf edges
	{
		for (int i=0; i<dim; i++)
		{
			for(int j=0; j<dim; j++)
			{
				if(image[0][i][j]>4)
					image[0][i][j]=4;
				else if(image[0][i][j]<-4)
					image[0][i][j]=-4;
			}
		}
	}
	else
	{
		for (int i=0; i<dim; i++)
		{
			for(int j=0; j<dim; j++)
			{
				if(image[0][i][j]>0.65)
					image[0][i][j]=1;
				else if(image[0][i][j]<0.35)
					image[0][i][j]=0;
			}
		}
	}
}

//calculate filter number given the layer
int calc_f_num(int layer)
{
	//int f_num_init=16; //'always'
	//*layer variable starts from 1
	switch (layer)
	{
	    case 1:
	      return 16;
	    case 2:
	      return 32;
	    case 3:
	    	return 64;
	    case 4:
	    	return 128;
	    case 5:
	    	return 256;
	    case 6:
	    	return 128;
	    case 7:
	    	return 64;
	    case 8:
	    	return 32;
	    case 9:
	    	return 16;
	    case 10:
	    	return 1;
	    default:
	    	return -1;
	      // default statements
	}
}

//Calculate channel number(of a filter, given the layer number plus the part(1 or 2) of the convolution block)
int calc_ch_num(int layer,int tuple)
{
	//int f_num_init=16; //'always'
	//*layer,tuple variable starts from 1 , tuple ={1,2} its the 1st or second filter/image channel of the layer
	switch (layer)
	{
	    case 1:
	      return (1+(tuple-1)*15);//1 , 16
	    case 2:
	      return 16 +(tuple-1)*16; //16, 32
	    case 3:
	    	return 32 +(tuple-1)*32;//32, 64
	    case 4:
	    	return 64 + (tuple -1)*64; //64, 128
	    case 5:
	    	return 128 + (tuple-1)*128; //128, 256
	    case 6:
	    	return 256 -(tuple-1)*128; //256(after concat), 128
	    case 7:
	    	return 128 - (tuple-1)*64; //128, 64
	    case 8:
	    	return 64 - (tuple -1)*32; //64, 32
	    case 9:
	    	return 32 - (tuple -1)*16;//32, 16
	    case 10:
	    	return 16;
	    default:
	    	return -1;
	      // default statements
	}
}




int main(void) {
	time_t t;
	srand((unsigned) time(&t));

	puts("Main function init!");
	
	int dim=256;

	///////////////////////////////////////////////////////
	////////////////// TESTING SECTION ////////////////////

	float ***img = make_3darray(1,dim);
	float ****image = (float ****)malloc(sizeof(float ***));
	for(int i=0;i<dim;i++)
	{
		for(int j=0;j<dim;j++)
		{
			img[0][i][j] = j*0.00000001;
		}
	}
	image[0]= img ;

	struct images_data_ *ptr_images_data = &images_data;
	//load images/labels//
	ptr_images_data->dim = dim;
	ptr_images_data->im_num = 4;
	ptr_images_data->images = image;
	//load_images(ptr_images_data);// Path edit in the File Managment file
	//load_labels(ptr_images_data);// Path edit in the File Managment file
	//////////////////////
	//struct params_ *ptr_params = &params;
	struct init_param_ *ptr_params = &init_param;
	// load pre-trained parameters (filters-bias-GN)//
	ptr_params->trim = 0.01;//number of group normalization batch
	ptr_params->layers = 5; //number of total layers
	ptr_params->num_f =16;   //init number of filters
	//ptr_params->num_f = 16; it will be calculated itself
	printf("Calling\n");
	Initialize_Parameters(ptr_params);
	//load_params(ptr_params); // Path edit in the File Managment file
	//////////////////////////////////////////////////

	
	struct timeval  tv1, tv2;
	gettimeofday(&tv1, NULL);
	//PREDICT
	predict(ptr_images_data, ptr_params);//last variable is prediction image number,we choose the img we want
	gettimeofday(&tv2, NULL);

	printf("\nTime needed: %.2f sec",((double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec)));
	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////
	printf("\nSuccess! - No Segmentation faults!");
	return EXIT_SUCCESS;
}
