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

#ifndef _CUDA_BLUR_DLL_INT_H
#define _CUDA_BLUR_DLL_INT_H

// global vars:
extern bool g_initialized;
extern bool g_canMapHostMem;

// exports from boxblur_kernel.cu:
extern "C" int BoxBlurA8_Device(unsigned char* a_data, int a_stride,
	int a_rows, int a_lobes[3][2], bool a_mapped);

// exports from recursiveGaussian.cu:
extern "C" int gaussianFilterRGBA(unsigned int *d_src, unsigned int *d_dest, unsigned int *d_temp, int width, int height, float sigma, int order, int nthreads);

#endif /* !_CUDA_BLUR_DLL_INT_H */
