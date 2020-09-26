//

#include<header.h>
static FATFS  fatfs;
void load_images(struct images_data_ *images_data)
{


	char **image_names;
	int im_num = images_data->im_num;
	int dim = images_data->dim;
	//create char space for image names
	image_names = (char **)malloc(im_num*sizeof(char *));
	for (int i = 0; i<im_num ; i++)
		image_names[i]=(char *)malloc(50*sizeof(char));

	///// MOUNTING SD CARD ////////

	FRESULT rc;
	//TCHAR *Path=(char*)malloc(20*sizeof(char));
	//strcpy( Path,"0:/");
	rc = f_mount(&fatfs,"0:/",0);
	if (rc) {
		printf(" ERROR : f_mount returned %d\r\n", rc);
		return;
	}

	DIR dir;
	//struct dirent *ent;
	FILINFO ent;
	f_opendir (&dir,"labis/images");
	if ( &dir != NULL ) {
		printf("\nLoading Images . . .");
	  // print all the files and directories within directory //
		//int i=0;
		//int count=0;
		f_readdir (&dir, &ent);
		for(int i=0; i<im_num; i++)
		{
			//ent->
			strcpy(image_names[i], ent.fname);
			printf("\n%d) %s\n",i,image_names[i]);

			f_readdir(&dir, &ent);
		}
		f_closedir (&dir);
	}


	int width,height,maxval;

	FIL fd;
	float **image = (float**)malloc(im_num*sizeof(float *));

	for(int im=0; im<1; im++)
	{
	//int im =0;
		char path_name[200];
		snprintf(path_name,sizeof(path_name),"labis/images/%s", image_names[im]);

		rc =f_open(&fd, path_name, FA_READ);
		if (rc) {
			printf("Could not open image file!\n");
			exit(1);
		}

		char line[5];
		f_gets(line, sizeof(line), &fd);

		if (strcmp(line, "P5\n") != 0) {
			printf("Image is not in PGM(P5) format!\n");
			f_close(&fd);
			exit(1);
		}
		char line1[20];
		f_gets(line1, sizeof(line1), &fd);



		char del1[2]=" ";
		char *token=(char *)malloc(20*sizeof(char));
		token = strtok(line1,del1);
		height = atoi(token);
		token = strtok(NULL,del1);
		width = atoi(token);

		char line2[10];
		char del2[2] ="\n";
		f_gets(line2, sizeof(line2), &fd);
		token = strtok(line2,del2);
		maxval = atoi(token);

		//sscanf(line1,"%d %d\n",&height,&width);

		//f_close(fd);
		//fflush(stdout);
	}

}
/*

		//////////////////////////////////////////
		float *img = (float *)malloc(dim*dim*sizeof(float));
	    if (maxval == 65535)
	    {
	    	int ch_num=1,dim=height;
			uint16_t *rbuffer = (uint16_t *)malloc(dim*dim*sizeof(uint16_t));
			UINT *br;
			f_read(fd, (void *)rbuffer,(width * height)*sizeof(uint16_t), br);
			if (*br != (width * height)*sizeof(uint16_t)) {
				printf("Error reading pixel values!\n");
				f_close(fd);
				exit(1);
			}
			f_close(fd);
			int offset=0;
			for(int i = 0; i<ch_num; i++ )
			{
				for (int x=0; x<height; x++)
				{
					for (int y=0; y<width; y++)
					{
						img[x*dim+y] = (float)((*(rbuffer+offset))/maxval);//normalized
						offset++;
						//printf("%.3f\t", image[i][x][y]);
					}
					//printf("\n");
				}
				//printf("\n");
			}
			free(rbuffer);
			image[im] = img;
	    }

	    else //means its 8-bit
	    {
	    	int ch_num=1,dim=height;
			uint8_t *rbuffer = (uint8_t *)malloc(dim*dim*sizeof(uint8_t));
			UINT *br;
			f_read(fd, (void *)rbuffer,(width * height)*sizeof(uint8_t), br);
			if (*br != (width * height)*sizeof(uint8_t)) {
				printf("Error reading pixel values!\n");
				f_close(fd);
				exit(1);
			}
			f_close(fd);
			int offset=0;
			for(int i = 0; i<ch_num; i++ )
			{
				for (int x=0; x<height; x++)
				{
					for (int y=0; y<width; y++)
					{
						img[x*dim +y] = ((float)*(rbuffer+offset))/maxval;//normalized
						offset++;
						//printf("%.3f\t", image[i][x][y]);
					}
					//printf("\n");
				}
				//printf("\n");
			}
			free(rbuffer);
			image[im] = img;
	    }
	//}

    images_data->images = image;
	printf("Done!\n");
}
*/


void load_labels(struct images_data_ *images_data)
{

	char **label_names;
	int im_num = images_data->im_num;
	int dim = images_data->dim;
	//create char space forr image names
	label_names = (char **)malloc(im_num*sizeof(char *));
	for (int i = 0; i<im_num ; i++)
		label_names[i]=(char *)malloc(20*sizeof(char));


	FRESULT rc;


	DIR *dir;
	//struct dirent *ent;
	FILINFO* ent;
	f_opendir (dir,"labis/labels");
	if ( dir != NULL ) {
		printf("\nLoading Labels . . .");
	  // print all the files and directories within directory //
		int i=0;
		int count=0;
		f_readdir (dir, ent);
		for(int i=0; i<im_num; i++)
		{
			//ent->
			strcpy(label_names[i], ent->fname);
			//printf("\n%d) %s\n",i,image_names[i]);

			f_readdir(dir, ent);
		}
		f_closedir (dir);
	}
    char path_name[200];
    int width, height, maxval;
    FIL *fd;
    int ch_num=1;
    //label = make_4darray(im_num, ch_num, dim);

    float **label =(float **)malloc(im_num*sizeof(float *));
    for(int im=0; im< im_num; im++)
    {
    	float *lab =(float *)malloc(dim*dim*sizeof(float));

    	sprintf(path_name,"labis/labels/%s", label_names[im]);

    	rc =f_open(fd, path_name, FA_READ);
		if (rc) {
			printf("Could not open Label file!\n");
			exit(1);
		}

		char line[5];
		f_gets(line, sizeof(line), fd);

		if (strcmp(line, "P5\n") != 0) {
			printf("Label is not in PGM(P5) format!\n");
			f_close(fd);
			exit(1);
		}
		char line1[10];
		f_gets(line1, sizeof(line1), fd);

		char del1[2]=" ";
		char *token;
		token = strtok(line1,del1);
		height = atoi(token);
		token = strtok(NULL,del1);
		width = atoi(token);

		char line2[10];
		char del2[2] ="\n";
		f_gets(line2, sizeof(line2), fd);
		token = strtok(line2,del2);
		maxval = atoi(token);



	    if (maxval == 65535)
	    {
	    	int ch_num=1,dim=height;
			uint16_t *rbuffer = (uint16_t *)malloc(dim*dim*sizeof(uint16_t));
			UINT *br;
			f_read(fd, (void *)rbuffer,(width * height)*sizeof(uint16_t), br);
			if (*br != (width * height)*sizeof(uint16_t)) {
				printf("Error reading pixel values!\n");
				f_close(fd);
				exit(1);
			}
			f_close(fd);
			int offset=0;
			for(int i = 0; i<ch_num; i++ )
			{
				for (int x=0; x<height; x++)
				{
					for (int y=0; y<width; y++)
					{
						lab[x*dim+y] = (float)((*(rbuffer+offset))/maxval);//normalized
						offset++;
						//printf("%.3f\t", image[i][x][y]);
					}
					//printf("\n");
				}
				//printf("\n");
			}
	    }
	    else //means its 8-bit
	    {
	    	int ch_num=1,dim=height;
			uint8_t *rbuffer = (uint8_t *)malloc(dim*dim*sizeof(uint8_t));
			UINT *br;
			f_read(fd, (void *)rbuffer,(width * height)*sizeof(uint8_t), br);
			if (*br != (width * height)*sizeof(uint8_t)) {
				printf("Error reading pixel values!\n");
				f_close(fd);
				exit(1);
			}
			f_close(fd);
			int offset=0;
			for(int i = 0; i<ch_num; i++ )
			{
				for (int x=0; x<height; x++)
				{
					for (int y=0; y<width; y++)
					{
						lab[x*dim+y] = ((float)*(rbuffer+offset))/maxval;//normalized
						offset++;
						//printf("%.3f\t", image[i][x][y]);
					}
					//printf("\n");
				}
				//printf("\n");
			}
	    }
	    label[im]=lab;
    }
    images_data->labels = label;
    printf("Done!\n");
}

void load_params(struct params_ *params)
{
	// Unpacking //

	int batch =params->gn_batch; //default:2
	int layers = params->layers; //default:10
	//int f_num = params->num_f;
	/////////////////
	float **filters = (float **)malloc((layers*2*2-1)*sizeof(float *));
	float **bias = (float **)malloc((layers*2*2-1)*sizeof(float *));
	float **f_dc = (float **)malloc((layers-2)*sizeof(float *));//NO OUT layer
	float **b_dc = (float **)malloc((layers-2)*sizeof(float *));
	//float **gamma = (float **)malloc((layers*2*2-2)*sizeof(float *));//no out layers
	//float **beta = (float **)malloc((layers*2*2-2)*sizeof(float *));

	//init reading from file //

	////////////////////////// FILTERS ////////////////////////////
	///////////////////////////////////////////////////////////////
	int dim=3;
	int f_num,ch_num;
	int sum=0;
	int offset=0;
	for (int i=1;i<=layers; i++)
	{
		if(i!=10)
		{
			sum += calc_f_num(i)*calc_ch_num(i,1)*3*3;
			sum += calc_f_num(i)*calc_ch_num(i,2)*3*3;
		}
		else
		{
			sum+=calc_f_num(i)*calc_ch_num(i,1)*1*1; //last layer: out_f
		}
	}

	uint32_t *rbuffer;
	FIL *ptr;
	f_open(ptr,"labis/weights.bin", FA_READ);
	if(ptr == NULL)
	{
		printf("Couldnt load directory!\nExiting . . . ");
		exit(1);
	}
	printf("Loading Parameters . . .");
	rbuffer = (uint32_t *)malloc(sum*sizeof(int32_t));
	UINT *br;
	f_read(ptr, (void *)rbuffer, sum*sizeof(uint32_t), br);
	int pos=0;
	for (int i=1;i<=layers; i++)
	{
		if(i!=10)
		{
			f_num = calc_f_num(i);
			ch_num = calc_ch_num(i,1);
			float *f = (float*)malloc(9*calc_f_num(i)*calc_ch_num(i,1)*sizeof(float));//make_4darray(calc_f_num(i), calc_ch_num(i,1), 3);
			for(int k=0; k<f_num; k++)
				for(int j=0; j< ch_num; j++)
					for(int x=0; x<dim; x++)
						for (int y=0; y<dim; y++)
						{
							f[k*ch_num*dim*dim+ j*dim*dim+x*dim+y] = *((float *)(rbuffer+offset));
							offset++;
						}
			pos = (i-1)*2;
			filters[pos]=f;

			//f_num = calc_f_num(i);
			ch_num = calc_ch_num(i,2);
			//f = make_4darray(f_num, ch_num, 3);
			f = (float*)malloc(9*f_num*ch_num*sizeof(float));
			for(int k=0; k<f_num; k++)
				for(int j=0; j< ch_num; j++)
					for(int x=0; x<dim; x++)
						for (int y=0; y<dim; y++)
						{
							f[k*ch_num*dim*dim+ j*dim*dim+x*dim+y]  = *((float *)(rbuffer+offset));
							offset++;
						}
			pos += 1;
			filters[pos]=f;
		}
		else//i == 10 --> last 1x1 conv
		{
			f_num = calc_f_num(i);
			ch_num = calc_ch_num(i,1);
			float *f = (float*)malloc(f_num*ch_num*sizeof(float));
			for(int k=0; k<f_num; k++)
				for(int j=0; j< ch_num; j++)
					for(int x=0; x<1; x++)
						for (int y=0; y<1; y++)
						{
							f[k*ch_num+ j+x+y] = *((float *)(rbuffer+offset));
							offset++;
						}
			pos = (i-1)*2; //pos == 18 (19 filters)
			filters[pos]=f;
		}
	}
	free(rbuffer);
	/////////////////// BIAS ///////////////////
	////////////////////////////////////////////
	sum=1;//already last layer out is counted, (just 1 scalar bias_out)
	offset=0;
	for (int i=1;i<=(layers-1); i++)//9 layers, last layer is precalculated
		sum += calc_f_num(i)*2;

	rbuffer = (uint32_t *)malloc(sum*sizeof(int32_t));
	//UINT *br;
	f_read(ptr, (void *)rbuffer, sum*sizeof(uint32_t), br);
	pos=0;
	offset=0;
	for (int i=1;i<=(layers-1); i++) //NOT THE LAST LAYER!!!
	{
		f_num = calc_f_num(i);
		ch_num = calc_ch_num(i,1);
		float *b = (float *)malloc(f_num*sizeof(float));
		for (int y=0; y<f_num; y++)
		{
			b[y] = *((float *)(rbuffer+offset));
			offset++;
		}
		pos = (i-1)*2;
		bias[pos]=b;

		//f_num = calc_f_num(i);
		ch_num = calc_ch_num(i,2);
		b =  (float *)malloc(f_num*sizeof(float));
		for (int y=0; y<f_num; y++)
		{
			b[y] = *((float *)(rbuffer+offset));
			offset++;
		}
		pos += 1;
		bias[pos]=b;
	}
	//last layer of bias
	f_num = calc_f_num(10);
	float *b = (float *)malloc(f_num*sizeof(float));
	for (int y=0; y<f_num; y++)
	{
		b[y] = *((float *)(rbuffer+offset));
		offset++;
	}
	bias[18]=b;
	free(rbuffer);
	///////////////////////////// F_DC ///////////////////////////////
	//////////////////////////////////////////////////////////////////

	dim=2;
	sum=0;
	offset=0;
	sum =((128*256) +(64*128)+(32*64)+(16*32))*2*2;
	rbuffer = (uint32_t *)malloc(sum*sizeof(int32_t));
	//UINT *br;
	f_read(ptr, (void *)rbuffer, sum*sizeof(uint32_t), br);
	pos=0;
	for (int i=6;i<=(layers-1); i++)
	{
		f_num = calc_f_num(i);//same as filter 6_1
		ch_num = calc_ch_num(i,1);//same as filter 6_1
		//float ****f = make_4darray(calc_f_num(i), calc_ch_num(i,1), 2);
		float *f = (float*)malloc(4*f_num*ch_num*sizeof(float));
		for(int k=0; k<f_num; k++)
			for(int j=0; j< ch_num; j++)
				for(int x=0; x<dim; x++)
					for (int y=0; y<dim; y++)
					{
						f[k*ch_num*dim*dim+ j*dim*dim+x*dim+y] = *((float *)(rbuffer+offset));
						offset++;
					}
		f_dc[pos]=f;
		pos++;
	}
	free(rbuffer);
	///////////////////////////// B_DC ///////////////////////////////
	//////////////////////////////////////////////////////////////////

	sum=0;
	offset=0;
	sum = 128+64+32+16;
	rbuffer = (uint32_t *)malloc(sum*sizeof(int32_t));
	//UINT *br;
	f_read(ptr, (void *)rbuffer, sum*sizeof(uint32_t), br);
	pos=0;
	for (int i=6;i<=(layers-1); i++)
	{
		f_num = calc_f_num(i);//same as filter 6_1
		float *b = (float *)malloc(f_num*sizeof(float));
		for (int y=0; y<f_num; y++)
		{
			b[y] = *((float *)(rbuffer+offset));
			offset++;
		}
		b_dc[pos]=b;
		pos++;
	}

	params->b_dc=b_dc;
	params->f_dc = f_dc;
	params->bias=bias;
	params->filters=filters;
	f_close(ptr);
	free(rbuffer);
	printf("Done!\n");

}



