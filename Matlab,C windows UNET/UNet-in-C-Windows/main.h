/*
 * main.h
 *
 *  Created on: Jun 20, 2020
 *      Author: labis
 */
/////// INCLUDES ///////
#include <stdint.h>
float Dice_Coef(float ***logs, float ***target,int dim);
void concat();
void convTransp();
void maxpool();
void Activation_Function();
void conv();
int calc_f_num();
void GN();
void load_params();
void load_images();
void load_labels();
void predict();
void normalize_custom();
////////////////////////

#ifndef MAIN_H_
#define MAIN_H_

float Random_Normal(int loc, float scale);
float *****testff(float ****t);
float ****make_4darray(int num,int channels,int dim);
float ***make_3darray(int channels,int dim);

#endif /* MAIN_H_ */

struct conv_data_
{
	/* -------------- Convolution ---------------
	 *
	 * conv_in: Image we went to apply convolution. shape: (channels, h, w)
	 * ch_num: its the input channels of the filter == 2nd dimenions of the filter == 1st dimension of the input image
	 * f_num: The number of filters(1st dim of the filter) which is the final number of channels that conv_out will have.
	 * o_dim: 	According to mode: 1) 'Same': Means that the h,w will be the same for the conv_out. In order to accomplish such
	 * a thing, we need to add a zero padding around the conv_in with p=1.((dim-3+2*p)/1 + 1) , s=1,f=3. 2) "Normal": Means that
	 * the result will have the dim of a normal-basic convolution: (dim-3)/1 + 1 = o_dim
	 *
	 * ----------Convolution_Backward ------------
	 *
	 *
	 *
	 */
	float ***conv_in, ***conv_out;
	float ****filter,*bias;
	int ch_num; // same as 2nd dimension of the filter
	int f_num; // == result channel number
	int dim; //Image dim, assume h==w
	int o_dim;
	int mode; //choose the padding between : same(1) and normal(0) convolution
	int f_dim;// it can be used from transposed conv too, tr_conv use 2x2 filters
}conv_data;


struct images_data_
{
	/*
	 *images,labels: 4-dimensional matrix , 1st dim keeps the overall number of input images, and then
	 * we have the basic type (ch_num,dim,dim)
	 * im_num: The number of images/labels available in the folders
	 */
	float ****images,****labels;
	int im_num, dim;
}images_data;
struct gn_data_
{

	/*
	 * image: shape:(ch_num,dim,dim)
	 * gamma,beta: shape:(ch_num//2) 1-d, they hold important variables that normalize the output of convolutions in batches
	 * dim: dimensions of the image
	 * ch_num:number of channels of the image
	 */
	float ***image;
	float *gamma,*beta;
	int batch, dim, ch_num;

	float ***out; //Normalize output, same size as image

}gn_data;

struct params_
{
	/*
	 * Filters: They made of all the filters for the forward step, including the final 1x1 conv. filters(out_F), These filters 4D dim
	 * as follows: (num_f, num_in_ch, f_h, f_w) and these filters are saved sequencially in a Filters array with the type of(*****)
	 * Bias: Thats the double pointer matric which keeps all the bias values of the network. We need 1-d array for the scalar values
	 * of each bias so the final Bias matrix it is type of (**),so it can include all the different sizes of bias.
	 * F_dc: This matrix contains the decoder upsampling transposed convolution filters.Its type is the same as Filters matrix.
	 * gn_batch: the number of channels group we batches together and applied group normalization
	 * b_dc,bias,ga,be : all these are tyoe of matrices of 1d-arrays. ga,be got //gn_batch the size of bias.
	 */
	int layers, num_f, gn_batch;
	float *****filters,**bias, *****f_dc,**b_dc,**ga,**be;

}params;

struct concat_crop_data_
{
	/*
	 * Image: 3-dimensional array of type(channels, h, w)
	 * Concat function: The images 1,2 will be concatenated with respect to their channels and the output image3 will
	 * have doubled the channels of the inputs with same h,w dimensions
	 * Crop: Cut to half the image1(input) with respect to the channels, so the results will be image2,3 with halfed
	 * the channels in comparison with input. The input image will share equally its channels to the 2 outputs with same h,w.
	 * ch_num: Its always referred to the input channel dimensions.
	 * o_ch_num : represents the number of channels that output(s) have.
	 */
	int o_ch_num,ch_num,dim;
	float ***image1,***image2,***image3;
}concat_crop_data;

struct maxpool_data_
{
	/*
	* Image: 3-dimension image as input of shape: (channels, h, w)
	* the output size will be OH = (int)(H - 2)/2 + 1,
	* it will be always integer result since we use resolution in the power of 2.
	* stride == conv_dimesions == 2
	* Info about current image: channels,dim
	*/
	int channels,dim,o_dim;
	float ***image,***output;
}maxpool_data;
struct maxpoolbackward_data_
{
	/*
	* Conv: The previous result(layer out): the part before we apply maxpooling on it.
	* dpool: 3-dimension image as input of shape: (channels, h, w), its the halfsize(default) of the conv input
	* because its the error that comes backward from smaller to bigger.
	* Output: Size same as conv, we will fill the slots of dconv(output) with the following technique: Find
	* the max element in a sub array of size 2x2 correspoding to the dpool element(error: that came from that 2x2 maxpooling.)
	* and fill this slot with the error, the 3 remaining will be zeros.
	* stride == conv_dimesions == 2
	* Info about current image: channels,dim where dim is the dimension of dpool
	*/
	int channels,dim,o_dim;
	float ***dpool,***conv,***output;
}maxpoolbackward_data;


struct act_func_data_
{
	/*
	 * Z: The output as raw of a layer exactly before we apply the activation function on it.
	 * dA:Thats the difference on the results (after activation function) we get during backprop from the next/forward layer so we
	 * can take advantage of them and backpropagate the error back to activation function backward and then to the previous layer.
	 * Code: Number of commamd: 1)sigmoid, 2)Sigmoid_backward, 3)Relu, 4)Relu_backward
	 * Channels: Number of channel of the specific input matrix
	 * Dim: We assume that we have a square image so the height == width == dim
	 */
	int code, channels, dim;
	float ***res, ***Z;

}act_func_data;

struct GP_arrays_ //General purpose arrays
{
	float *dim1;
	float **dim2;
	float ***dim3;
	float ****dim4;
	float *****dim5;
}GP_arrays;

struct norm_data_
{
	/*
	 * image,res: type of (ch_num,dim,dim). Its the last layer output before sigmoid
	 * for code==0(where a simple +-inf protection kicks in) and also the output for the latest result after sigmoid where
	 * code ==1 that is built to push some values to their possible target value.
	 * (e.g. if value is ~0.9 , we assume its target is 1.0 to we change it to 1)
	 * **IMPORTANT!**: normalize_custom function keeps the pointer of the sent image and make changes on it.
	 */
	float ***image,***res;
	int code,dim; //ch_num==1
}norm_data;


struct init_param_
{
	/*
	 * Filters: They made of all the filters for the forward step, including the final 1x1 conv. filters(out_F), These filters 4D dim
	 * as follows: (num_f, num_in_ch, f_h, f_w) and these filters are saved sequencially in a Filters array with the type of(*****)
	 * Bias: Thats the double pointer matric which keeps all the bias values of the network. We need 1-d array for the scalar values
	 * of each bias so the final Bias matrix it is type of (**),so it can include all the different sizes of bias.
	 * F_dc: This matrix contains the decoder upsampling transposed convolution filters.Its type is the same as Filters matrix.
	 */
	int layers, num_f, trim;
	float *****filters,**bias, *****f_dc,**b_dc;

}init_param;
struct init_GN_
{
	/*
	 *-Layers: We need the number of forward layers(like in init_param) so we can calculate the final number of elements
	 *gamma/beta matrix will have.
	 *-Starting_num_ch: This is the number of filters we apply in the very first convolution of the net, which is always 16,
	 *that way we will able to calculate the rest dimension of gamma/beta sub-matrices.
	 *-Gamma/Beta: These two 'big' matrices are going to keep all the elements(sub-arrays) of each layer, so we can access them later.
	 */
	int layers, starting_num_ch, trim;
	float **gamma, **beta;

}init_GN;

