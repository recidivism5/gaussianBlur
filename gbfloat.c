#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PI 3.14159

typedef struct FVec
{
    unsigned int length;
    float* data;
} FVec;

typedef struct Image
{
    unsigned int dimX, dimY, numChannels;
    unsigned char* data;
} Image;

void normalize_FVec(FVec v)
{
    float sum = 0.0;
    unsigned int i;
    for (i = 0; i < v.length; i++)
    {
        sum += v.data[i];
    }
    for (i = 0; i < v.length; i++)
    {
        v.data[i] /= sum;
    }
}

unsigned char* get_pixel(Image img, int x, int y)
{
    if (x < 0)
    {
        x = 0;
    }
    if (x >= img.dimX)
    {
        x = img.dimX - 1;
    }
    if (y < 0)
    {
        y = 0;
    }
    if (y >= img.dimY)
    {
        y = img.dimY - 1;
    }
    return img.data + img.numChannels * (y * img.dimX + x);
}

float gd(float a, float b, float x)
{
    float c = (x-b) / a;
    return exp((-.5) * c * c) / (a * sqrt(2 * PI));
}

FVec make_gv(float a, float x0, float x1, unsigned int length)
{
    FVec v;
    v.length = length;
    v.data = malloc(length * sizeof(float));

    float step = (x1 - x0) / ((float)length);
    int offset = length/2;

    for (int i = 0; i < length; i++)
    {
        v.data[i] = gd(a, 0.0f, (i-offset)*step);
    }
    normalize_FVec(v);
    return v;
}

void print_fvec(FVec v)
{
    unsigned int i;
    printf("\n");
    for (i = 0; i < v.length; i++)
    {
        printf("%f ", v.data[i]);
    }
    printf("\n");
}

Image img_sc(Image a)
{
    Image b = a;
    b.data = malloc(b.dimX * b.dimY * b.numChannels * sizeof(unsigned char));
    return b;
}

Image gb_h(Image a, FVec gv)
{
    Image b = img_sc(a);

    int ext = gv.length / 2;
    int offset;
    unsigned int x, y, channel;
    unsigned char *pc;
    double sum;
    int i;
    for (y = 0; y < a.dimY; y++)
    {
        for (x = 0; x < a.dimX; x++)
        {
            pc = get_pixel(b, x, y);
            for (channel = 0; channel < a.numChannels; channel++)
            {
                sum = 0;
                for (i = 0; i < gv.length; i++)
                {
                    offset = i - ext;
                    sum += gv.data[i] * (float)get_pixel(a, x + offset, y)[channel];
                }
                pc[channel] = (unsigned char)sum;
            }
        }
    }
    return b;
}

Image gb_v(Image a, FVec gv)
{
    Image b = img_sc(a);

    int ext = gv.length / 2;
    int offset;
    unsigned int x, y, channel;
    unsigned char *pc;
    double sum;
    int i;
    for (y = 0; y < a.dimY; y++)
    {
        for (x = 0; x < a.dimX; x++)
        {
            pc = get_pixel(b, x, y);
            for (channel = 0; channel < a.numChannels; channel++)
            {
                sum = 0;
                for (i = 0; i < gv.length; i++)
                {
                    offset = i - ext;
                    sum += gv.data[i] * (float)get_pixel(a, x, y + offset)[channel];
                }
                pc[channel] = (unsigned char)sum;
            }
        }
    }
    return b;
}

Image apply_gb(Image a, FVec gv)
{
    Image b = gb_h(a, gv);
    Image c = gb_v(b, gv);
    free(b.data);
    return c;
}

int main(int argc, char** argv)
{
    if (argc < 6)
    {
        printf("Usage: ./gb.exe <inputjpg> <outputname> <float: a> <float: x0> <float: x1> <unsigned int: dim>\n");
        exit(0);
    }

    float a, x0, x1;
    unsigned int dim;

    sscanf(argv[3], "%f", &a);
    sscanf(argv[4], "%f", &x0);
    sscanf(argv[5], "%f", &x1);
    sscanf(argv[6], "%u", &dim);

    FVec v = make_gv(a, x0, x1, dim);
    print_fvec(v);

    Image img;

    img.data = stbi_load(argv[1], &(img.dimX), &(img.dimY), &(img.numChannels), 0);
    Image imgOut = apply_gb(img, v);

    stbi_write_jpg(argv[2], imgOut.dimX, imgOut.dimY, imgOut.numChannels, imgOut.data, 90);
    
    return 0;
}