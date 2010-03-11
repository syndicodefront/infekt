/**
 * Copyright (C) 2010 cxxjoe
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 **/

#include "cutil_inline.h"


// how much parallelization?
#define NUM_THREADS 128
#define PER_THREAD(a_total) ((unsigned int)ceil((float)(a_total + 1) / (float)NUM_THREADS))


// largely based on
// http://mxr.mozilla.org/mozilla1.9.2/source/gfx/thebes/src/gfxBlur.cpp
// (GPL v2)


// mozilla code stuff
typedef int PRInt32;
#define PR_MAX(a, b) ((a) > (b) ? (a) : (b))
#define PR_MIN(a, b) ((a) < (b) ? (a) : (b))


__global__ void BoxBlurA8Horizontal_Device(unsigned char* aInput,
	unsigned char* aOutput,
	int aLeftLobe, int aRightLobe, int aStride, int aRows)
{
	unsigned int y = (__umul24(blockIdx.x, blockDim.x) + threadIdx.x);
	if(y >= aRows) return;

	PRInt32 boxSize = aLeftLobe + aRightLobe + 1;

    PRInt32 alphaSum = 0;
    for (PRInt32 i = 0; i < boxSize; i++) {
        PRInt32 pos = i - aLeftLobe;
        pos = PR_MAX(pos, 0);
        pos = PR_MIN(pos, aStride - 1);
        alphaSum += aInput[aStride * y + pos];
    }
    for (PRInt32 x = 0; x < aStride; x++) {
        PRInt32 tmp = x - aLeftLobe;
        PRInt32 last = PR_MAX(tmp, 0);
        PRInt32 next = PR_MIN(tmp + boxSize, aStride - 1);

        aOutput[aStride * y + x] = alphaSum/boxSize;

        alphaSum += aInput[aStride * y + next] -
                    aInput[aStride * y + last];
    }
}


__global__ void BoxBlurA8Vertical_Device(unsigned char* aInput,
	unsigned char* aOutput,
	int aTopLobe, int aBottomLobe, int aStride, int aRows)
{
	unsigned int x = (__umul24(blockIdx.x, blockDim.x) + threadIdx.x);
	if(x >= aStride) return;

    PRInt32 boxSize = aTopLobe + aBottomLobe + 1;

    PRInt32 alphaSum = 0;
    for (PRInt32 i = 0; i < boxSize; i++) {
        PRInt32 pos = i - aTopLobe;
        pos = PR_MAX(pos, 0);
        pos = PR_MIN(pos, aRows - 1);
        alphaSum += aInput[aStride * pos + x];
    }
    for (PRInt32 y = 0; y < aRows; y++) {
        PRInt32 tmp = y - aTopLobe;
        PRInt32 last = PR_MAX(tmp, 0);
        PRInt32 next = PR_MIN(tmp + boxSize, aRows - 1);

        aOutput[aStride * y + x] = alphaSum/boxSize;

        alphaSum += aInput[aStride * next + x] -
                    aInput[aStride * last + x];
    }
}


extern "C" int BoxBlurA8_Device(unsigned char* a_data, int a_stride,
	int a_rows, int a_lobes[3][2], bool a_mapped)
{
	unsigned char *l_devImgBuf, *l_devTmpBuf;
	int l_bufLen = a_stride * a_rows;

	if(!a_mapped)
	{
		cudaMalloc((void**)&l_devImgBuf, l_bufLen);
		cudaMemcpy(l_devImgBuf, a_data, l_bufLen, cudaMemcpyHostToDevice);
	}
	else
	{
		cudaHostGetDevicePointer((void**)&l_devImgBuf, (void*)a_data, 0);
	}

	cudaMalloc((void**)&l_devTmpBuf, l_bufLen);

	// zomg!
    BoxBlurA8Horizontal_Device<<<PER_THREAD(a_rows), NUM_THREADS>>>(l_devImgBuf, l_devTmpBuf, a_lobes[0][0], a_lobes[0][1], a_stride, a_rows);
	BoxBlurA8Horizontal_Device<<<PER_THREAD(a_rows), NUM_THREADS>>>(l_devTmpBuf, l_devImgBuf, a_lobes[1][0], a_lobes[1][1], a_stride, a_rows);
	BoxBlurA8Horizontal_Device<<<PER_THREAD(a_rows), NUM_THREADS>>>(l_devImgBuf, l_devTmpBuf, a_lobes[2][0], a_lobes[2][1], a_stride, a_rows);

    BoxBlurA8Vertical_Device<<<PER_THREAD(a_stride), NUM_THREADS>>>(l_devTmpBuf, l_devImgBuf, a_lobes[0][0], a_lobes[0][1], a_stride, a_rows);
	BoxBlurA8Vertical_Device<<<PER_THREAD(a_stride), NUM_THREADS>>>(l_devImgBuf, l_devTmpBuf, a_lobes[1][0], a_lobes[1][1], a_stride, a_rows);
	BoxBlurA8Vertical_Device<<<PER_THREAD(a_stride), NUM_THREADS>>>(l_devTmpBuf, l_devImgBuf, a_lobes[2][0], a_lobes[2][1], a_stride, a_rows);

	if(!a_mapped)
	{
		cudaMemcpy(a_data, l_devImgBuf, l_bufLen, cudaMemcpyDeviceToHost);
		cudaFree(l_devImgBuf);
	}

	cudaFree(l_devTmpBuf);

	return 1;
}
