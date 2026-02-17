// cc-wavelet.cpp : Defines the entry point for the application.
//

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <time.h>

int32_t* imR;
int32_t* imG;
int32_t* imB;

int width, height, imgsize;

void dwt_forward(int32_t* im, int beg, int maxindexval, int indexdiff, int level) { // indexdiff = (hor vs. vert) ? 1 : bitmap_stride;
    const inc = indexdiff << level;
    const end = beg + maxindexval;
    //assert(inc < end && "stepping outside source image");

    int i = beg + inc;
    // high pass filter, {-1./2, 1., -1./2}
    for (; i < end - inc; i += 2 * inc) {
        im[i] -= (im[i - inc] + im[i + inc]) >> 1;
    }
    if (i < end) {
        im[i] -= im[i - inc];
    }

    i = beg;
    // low pass filter, 
    // successive convolutions with {-1./2, 1., -1./2} for odd pixels
    // and {1./4, 1., 1./4} for even pixels
    // for im[n] result is -im[n-2]/8 + im[n-1]/4 + 6*im[n]/8 + im[n+1]/4 - im[n+2]/8
    // i.e., {-1./8, 2./8, 6./8, 2./8, -1./8}
    im[i] += (im[inc] + 1) >> 1;
    i += 2 * inc;
    for (; i < end - inc; i += 2 * inc) {
        im[i] += (im[i - inc] + im[i + inc] + 2) >> 2;
    }
    if (i < end) {
        im[i] += (im[i - inc] + 1) >> 1;
    }
}

void forward_transform_horizontal(level) {
    for (int ih = 0; ih < height; ++ih) {
        dwt_forward(imR, ih * width, width, 1, level);
        dwt_forward(imG, ih * width, width, 1, level);
        dwt_forward(imB, ih * width, width, 1, level);
    }
}
void forward_transform_vertical(level) {
    for (int iw = 0; iw < width; ++iw) {
        dwt_forward(imR, iw, imgsize, width, level);
        dwt_forward(imG, iw, imgsize, width, level);
        dwt_forward(imB, iw, imgsize, width, level);
    }
}

int forward_transform(char* imageData, int width, int height, int horLevels, int vertLevels)
{
    imgsize = width * height;
    imR = (int32_t*)malloc(sizeof(int32_t) * imgsize);
    imG = (int32_t*)malloc(sizeof(int32_t) * imgsize);
    imB = (int32_t*)malloc(sizeof(int32_t) * imgsize);
    for (int i = 0; i < imgsize; ++i)
    {
        imR[i] = imageData[4 * i + 0];
        imG[i] = imageData[4 * i + 1];
        imB[i] = imageData[4 * i + 2];
    }

    // JPEG XS pipeline requires to upscale image data to 20 bit precision
    for (int i = 0; i < width * height; ++i) {
        imR[i + 0] = imageData[i * 4 + 0] << 12;
        imG[i + 1] = imageData[i * 4 + 1] << 12;
        imB[i + 2] = imageData[i * 4 + 2] << 12;
    }

    clock_t startTime = clock();
    for (int level = 0; level < vertLevels; ++level) {
        forward_transform_vertical(level);
    }
    for (int level = 0; level < horLevels; ++level) {
        forward_transform_horizontal(level);
    }
    clock_t finishTime = clock();
    for (int i = 0; i < width * height; ++i) {
            imageData[i * 4 + 0] = (imR[i] >> 12);
            imageData[i * 4 + 1] = (imG[i] >> 12);
            imageData[i * 4 + 2] = (imB[i] >> 12);
    }
    return (finishTime - startTime);
}

void dwt_inverse(int32_t* im, int beg, int maxindexval, int indexdiff, int level) { // indexdiff = (hor vs. vert) ? 1 : bitmap_stride;
    const inc = indexdiff << level;
    const end = beg + maxindexval;
    //assert(inc < end && "stepping outside source image");

    // low pass filter, {-1./4, 1./4, -1./4}
    int i = 0;
    im[i] -= (im[inc] + 1) >> 1;
    i += 2 * inc;
    for (; i < end - inc; i += 2 * inc)
    {
        im[i] -= (im[i - inc] + im[i + inc] + 2) >> 2;
    }
    if (i < end)
    {
        im[i] -= (im[i - inc] + 1) >> 1;
    }

    // high pass filter, {-1./8, 1./8, 6./8, 1./8 -1./8}
    // successive convolutions with {-1./4, 1., -1./4} for even pixels
    // and {1./2, 1., 1./2} for even pixels
    // for im[n] result is -im[n-2]/8 + im[n-1]/8 + 6*im[n]/8 + im[n+1]/8 - im[n+2]/8
    i = inc;
    for (; i < end - inc; i += 2 * inc)
    {
        im[i] += (im[i - inc] + im[i + inc]) >> 1;
    }
    if (i < end)
    {
        im[i] += im[i - inc];
    }
}

void inverse_transform_horizontal(level) {
    for (int ih = 0; ih < height; ++ih) {
        dwt_inverse(imR, ih * width, width, 1, level);
        dwt_inverse(imG, ih * width, width, 1, level);
        dwt_inverse(imB, ih * width, width, 1, level);
    }
}
void inverse_transform_vertical(level) {
    for (int iw = 0; iw < width; ++iw) {
        dwt_inverse(imR, iw, imgsize, width, level);
        dwt_inverse(imG, iw, imgsize, width, level);
        dwt_inverse(imB, iw, imgsize, width, level);
    }
}

int inverse_transform(char* imageData, int width, int height, int horLevels, int vertLevels)
{
    clock_t startTime = clock();
    for (int level = 0; level < horLevels; ++level) {
        inverse_transform_horizontal(level);
    }
    for (int level = 0; level < vertLevels; ++level) {
        inverse_transform_vertical(level);
    }
    clock_t finishTime = clock();
    for (int i = 0; i < width * height; ++i) {
        imageData[i * 4 + 0] = (imR[i] >> 12);
        imageData[i * 4 + 1] = (imG[i] >> 12);
        imageData[i * 4 + 2] = (imB[i] >> 12);
    }
    return (finishTime - startTime);
}

