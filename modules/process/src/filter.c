/*************************************************************************
	> File Name: filter.c
	> Author: ljm
	> Mail: jimin@iscas.ac.cn
 ************************************************************************/
#include "precomp.h"

#define max(a,b) a>b?a:b

//only data_type unsigned char 
/////////////////////////////////////////////////////////////////////////
//____________________private function________________________

static void swap(unsigned char a,unsigned char b)
{
	unsigned char c = a;
	a = b;
	b = c;
}

static unsigned char median_member(unsigned char* array,int size)
{
	for(int gap = size/2; gap++; gap/=2)
		for(int i=gap; i<size; i++)
			for(int j=i-gap; j>=0 && array[j]>array[j+gap]; j-=gap)
				swap(array[j],array[j+gap]);
	return array[size/2]; 
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                     public function                              //
//                                                                  //
//////////////////////////////////////////////////////////////////////

void hypercv_filter2D(simple_mat src, simple_mat dst, unsigned char* kernel, int k_rows, int k_cols, int border_type)
{
	_assert(src!= NULL,"input mat cannot be null");
	_assert(kernel!=NULL,"kernel cannot be null");

	int rows = src->rows;
	int cols = src->cols;
	int channels = src->channels;
	int data_type = src->data_type;

	_assert(data_type == 1,"filter only use in data_type == 1");

	if(dst == NULL)
		dst = create_simple_mat(rows,cols,data_type,channels);

	unsigned char* src_data = (unsigned char*)src->data;
	unsigned char* dst_data = (unsigned char*)dst->data;
//todo fix
	for(int i=0;i<rows;i++)
	{
		for(int j=0;j<cols;j++)
		{

		}
	}
}

void hypercv_medianblur(simple_mat dst_mat, simple_mat src_mat, int size)
{
	_assert(!src_mat,"input mat can not be NULL");
	int rows = src_mat->rows;
	int cols = src_mat->cols;
	int data_type = src_mat->data_type;
	int channels = src_mat->channels;
	if(dst_mat == NULL)
		dst_mat = create_simple_mat(rows,cols,data_type,channels);

	unsigned char* src_data = (unsigned char*) src_mat->data;
	unsigned char* dst_data = (unsigned char*) dst_mat->data;
	unsigned char* arr = (unsigned char*) malloc(size*size*sizeof(char));

	for(int i=0;i<rows;i++)
	{
		for(int j=0;j<cols;j++)
		{
			if((i-size/2)>=0&&(i+size/2)<rows&&(j-size/2)>=0&&(j+size/2)<cols)
			{
				for(int m = -size/2;m<size/2;m++)
					for(int n = -size/2; n<size/2;n++)
						arr[(m+size/2)*size+n+size/2] = src_data[(i+m)*cols+j+n];

				dst_data[i*cols+j] = median_member(arr,size); 
			}
		}
	}
}

/**
 * @brief   User-callable function to create an unidimensional gaussian kernel.
 * @param[in]  kernel_size        lens of kernel.
 * @param[in]  sigma              Gaussian standard deviation.
 * @param[in]  ktype              type of kernel such as float.
 * retva       simple_mat.
 **/
simple_mat gaussian_kernel(int kernel_size, double sigma, int ktype)
{
    _assert(kernel_size>0, "length of kernel must >0");
    _assert(ktype == 4 || ktype == 5 ,"ktype is illegal");

    const int SMALL_GAUSSIAN_SIZE = 7;
    float temp1[]= {1.f};
    float temp2[]= {0.25f,0.5f,0.25f};
    float temp3[]= {0.0625f,0.25f,0.375f,0.25f,0.0625f};
    float temp4[]= {0.03125f,0.109375f,0.21875f,0.28125f,0.21875f,0.109375f,0.03125f};

    float* small_gaussian_tab[4];

    small_gaussian_tab[0]= temp1;
    small_gaussian_tab[1]= temp2;
    small_gaussian_tab[2]= temp3;
    small_gaussian_tab[3]= temp4;

    const float* fixed_kernel = ( kernel_size % 2 == 1 && kernel_size <= SMALL_GAUSSIAN_SIZE && sigma <= 0 )? small_gaussian_tab[kernel_size/2] : 0;

    simple_mat kernel = create_simple_mat(1,kernel_size,ktype,1);
    float* cf = (float*) kernel -> data;
    double* cd = (double*) kernel -> data; 

    double sigmaX = sigma > 0 ? sigma :((kernel_size-1)*0.5 -1)*0.3 + 0.8 ;
    double scale2X = -0.5/(sigmaX*sigmaX);

    double sum = 0;
    int i;

    for ( i = 0; i < kernel_size; i ++ )
    {
        double x = i-(kernel_size-1)*0.5;
        double t = fixed_kernel?(double)fixed_kernel[i] : exp (scale2X*x*x);
        if (ktype == 4)
        {
            cf[i] =(float)t;
            sum += cf[i];
        }
        else
        {
            cd[i] = t ;
            sum += cd[i];
        }
    }
    sum = 1./sum;
    for (i =0 ;i <kernel_size ; i++)
    {
        if (ktype == 4)
            cf[i] = (float)(cf[i]*sum);
        else
            cd[i] *= sum;
    }
    return kernel;
}


/**
 * @brief      User-callable function to gaussian filter.
 * @param[in]  src_mat          input mat. 
 * @param[in]  ksize_width      length of X direction kernel.
 * @param[in]  ksize_height     length of Y direction kernel.
 * @param[in]  sigma1           Gaussian kernel standard deviation in X direction. 
 * @param[in]  sigma2           Gaussian kernel standard deviation in Y direction.
 * @param[in]  border_type      pixel extrapolation method
 **/
void hypercv_gaussian_blur(simple_mat src_mat, simple_mat dst_mat, int ksize_width, int ksize_height, double sigma1, double sigma2, int border_type)
{
    _assert(src_mat!=NULL,"input mat must not be NULL");
    _assert(ksize_width > 0 && ksize_width % 2 == 1 && ksize_height > 0 && ksize_height % 2 == 1 , " ksize is illegal");
    _assert(border_type == BORDER_REFLECT||border_type == BORDER_REFLECT_101||border_type == BORDER_REPLICATE||border_type == BORDER_WRAP||border_type == BORDER_CONSTANT ,"Unknown/unsupported border type");

    int data_type = src_mat -> data_type;

    if(border_type != BORDER_CONSTANT && (border_type & BORDER_ISOLATED)!= 0)
    {
        if(src_mat->rows == 1)
            ksize_height == 1;
        if(src_mat->cols == 1)
            ksize_width == 1; 
    }

    if ( sigma2 <= 0 )
        sigma2 = sigma1;

    if (ksize_width <= 0 && sigma1 >0)
        ksize_width = HYPERCV_ROUND(sigma1*6 + 1)|1;
    if (ksize_width <= 0 && sigma2 > 0)
        ksize_height = HYPERCV_ROUND(sigma2*6 + 1)|1;

    sigma1 = max( sigma1 , 0.0 );
    sigma2 = max( sigma2 , 0.0 );
    int kdepth = 4; 

    simple_mat kernel_mat_x = gaussian_kernel(ksize_width, sigma1, kdepth);
    simple_mat kernel_mat_y = gaussian_kernel(ksize_height,sigma2, kdepth);
    hypercv_gaussian_blur_with_kernel(src_mat, dst_mat, kernel_mat_x, kernel_mat_y, border_type);
}

/**
 * @brief      User-callable function to gaussian with kernel.
 * @param[in]  src_mat                input mat.
 * @param[in]  kernel_mat_x           gaussian kernel of X direction.
 * @param[in]  kernel_mat_y           gaussian kernel of Y direction. 
 * @param[in]  border_type            pixel extrapolation method.
 **/
void hypercv_gaussian_blur_with_kernel(simple_mat src_mat,simple_mat dst_mat, simple_mat kernel_mat_x, simple_mat kernel_mat_y, int border_type)
{
    int ksize_width = kernel_mat_x->cols;
    int ksize_height = kernel_mat_y->cols;

    _assert(src_mat!=NULL,"input mat must not be NULL");
    _assert(ksize_width>=0,"length of kernel_x must >=0");
    _assert(ksize_height>=0,"length of kernel_y must >=0");
    _assert(border_type == BORDER_REFLECT
            ||border_type == BORDER_REFLECT_101
            ||border_type == BORDER_REPLICATE
            ||border_type == BORDER_WRAP
            ||border_type == BORDER_CONSTANT ,"Unknown/unsupported border type" );

    int cn = src_mat ->channels;
    _assert(cn == 1 ||cn == 3 ,"src channel equal 1 or 3 ");

    int border_x = ksize_width/2;
    int border_y = ksize_height/2;

	if(dst_mat == NULL)
          dst_mat = create_simple_mat(src_mat->rows, src_mat->cols, src_mat->data_type,cn);
    unsigned char* dst_data =(unsigned char*) dst_mat -> data;

    int src_rows = src_mat->rows;
    int src_cols = src_mat->cols;

    int i,j,k,index;

    simple_mat temp_mat = create_simple_mat(src_rows,src_cols,4,cn);

    float* temp_data =(float*) temp_mat->data;
    unsigned char* src_data =(unsigned char*) src_mat->data;

    float* kx_data = (float*)kernel_mat_x->data;
    float* ky_data = (float*)kernel_mat_y->data;

	int elemsize = get_elemsize(src_mat->data_type);

    for( i =0 ; i < src_rows ;i++)
    {
        for ( j = 0; j < src_cols ;j++)
        {
            float sum[3] = {0.0,0.0,0.0};

            for (k = 0; k <ksize_width; k++)
            {
                if ((j+k-border_x<0)||(j+k-border_x>=src_cols))
                { 
                    index = hypercv_border_Interpolate(j+k-border_x, src_cols, border_type);
                }
                else
                {
                    index = j+k-border_x;
                }

                if (cn == 1)
                {
                    sum[0] += (kx_data[k]*src_data[i*src_cols*elemsize+index*elemsize]);
                }
                else if (cn == 3)
                {
                    sum[0] += (kx_data[k]*src_data[i*src_cols*elemsize*cn+(index)*elemsize*cn]);
                    sum[1] += (kx_data[k]*src_data[i*src_cols*elemsize*cn+(index)*elemsize*cn+1]);
                    sum[2] += (kx_data[k]*src_data[i*src_cols*elemsize*cn+(index)*elemsize*cn+2]);
                }
            }
            for ( k = 0 ; k < cn ; k++)
            {
                if(sum[k]<0)
                    sum[k]=0;
                else if (sum[k]>255)
                    sum[k]=255;
            }
            if (cn == 1)
                temp_data[i*temp_mat->cols+j] =sum[0];
            else if (cn == 3)
            { 
                temp_data[i*temp_mat->cols*cn+j*cn+0]= sum[0];
                temp_data[i*temp_mat->cols*cn+j*cn+1]= sum[1];
                temp_data[i*temp_mat->cols*cn+j*cn+2]= sum[2];
            }
        }
    }


    for( i = 0 ; i < src_cols;i++)
    {
        for ( j = 0; j < src_rows ;j++)
        {
            float sum[3] = {0.0,0.0,0.0};

            for ( k =0 ; k< ksize_height ; k++)
            {
                if ((j+k-border_y<0)||(j+k-border_y>=src_rows))
                { 
                    index = hypercv_border_Interpolate(j+k-border_y, src_rows, border_type);
                }
                else
                {
                    index =j+k-border_y;
                }
                if (cn == 1)
                    sum[0] += (ky_data[k]*temp_data[(index)*temp_mat->cols+i]);
                else if (cn == 3)
                {
                    sum[0] += (ky_data[k]*temp_data[(index)*temp_mat->cols*cn+i*cn]);
                    sum[1] += (ky_data[k]*temp_data[(index)*temp_mat->cols*cn+i*cn+1]);
                    sum[2] += (ky_data[k]*temp_data[(index)*temp_mat->cols*cn+i*cn+2]);
                }
            }
            for (k =0 ;k<cn;k++)
            {
                if(sum[k]<0)
                    sum[k]=0;
                else if (sum[k]>255)
                    sum[k]=255;
            }
            if (cn == 1)
            {
                dst_data[(j)*dst_mat->cols*elemsize+i*elemsize] = saturate_cast_float2uchar (sum[0]);
            }

            else if (cn==3)
            { 
                dst_data[(j)*dst_mat->cols*elemsize*cn+i*elemsize*cn]=saturate_cast_float2uchar(sum[0]);
                dst_data[(j)*dst_mat->cols*elemsize*cn+i*elemsize*cn+1]=saturate_cast_float2uchar(sum[1]);
                dst_data[(j)*dst_mat->cols*elemsize*cn+i*elemsize*cn+2]=saturate_cast_float2uchar(sum[2]);
            }
        }
    }
}


void hypercv_integral(simple_mat src, simple_mat dst)
{
	_assert(src != NULL,"input mat cannot be null");
	
	int rows = src->rows;
	int cols = src->cols;
	int data_type = src-> data_type;
	int channels = src->channels;

	_assert(data_type == 1,"only use in char");
	_assert(channels == 1,"integral use with gray image");

	if(dst == NULL)
		dst = create_simple_mat(rows,cols,3,1);

	_assert(dst->channels == 1&&dst->data_type == 3,"dst mat error");
	
	unsigned char* src_data = (unsigned char*)src->data;
	int* dst_data = (int*)dst->data;

	for(int i=0;i<rows;i++)
	{
		for(int j=0;j<cols;j++)
		{
			if(i==0||j==0)
				dst_data[i*cols+j] = src_data[i*cols+j];
			else
				dst_data[i*cols+j] = dst_data[(i-1)*cols+j] + dst_data[i*cols+j-1] + src_data[i*cols+j]-dst_data[(i-1)*cols+j-1];	
		}
	}
}

