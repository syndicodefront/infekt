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

#include "targetver.h"
#include <windows.h>
#include "cutil_inline.h"
#include "cuda_blur_dll_int.h"


bool g_initialized = false;
bool g_canMapHostMem = false;


extern "C" __declspec(dllexport) int IsCudaUsable()
{
	int l_count = 0;

	if(g_initialized)
	{
		// calling cudaSetDevice when it has already been used is bad!
		return 1;
	}

	if(cudaGetDeviceCount(&l_count) != cudaSuccess || l_count < 1)
	{
		return -1;
	}

	if(cudaSetDevice(cutGetMaxGflopsDeviceId()) != cudaSuccess)
	{
		return -2;
	}

	cudaDeviceProp l_props;

	if(cudaGetDeviceProperties(&l_props, cutGetMaxGflopsDeviceId()) != cudaSuccess)
	{
		return -3;
	}

	if(l_props.major == 9999 && l_props.minor == 9999)
	{
		return 0;
	}

	return 1;
}


extern "C" __declspec(dllexport) int InitCudaThread()
{
	if(!IsCudaUsable())
	{
		return -1;
	}

	cudaDeviceProp l_props;

	if(cudaGetDeviceProperties(&l_props, cutGetMaxGflopsDeviceId()) != cudaSuccess)
	{
		return -2;
	}

	g_canMapHostMem = false;
	if(l_props.canMapHostMemory)
	{
		if(cudaSetDeviceFlags(cudaDeviceMapHost) == cudaSuccess)
		{
			g_canMapHostMem = true;
		}
	}

	g_initialized = true;

	return 1;
}


extern "C" __declspec(dllexport) int IsCudaThreadInitialized()
{
	return (g_initialized ? 1 : 0);
}


extern "C" __declspec(dllexport) int UnInitCudaThread()
{
	int l_result = (cudaThreadExit() == cudaSuccess ? 1 : 0);
	g_initialized = false;
	return l_result;
}


extern "C" __declspec(dllexport) int DoCudaBoxBlurA8(unsigned char* a_data,
	int a_stride, int a_rows, int a_lobes[3][2])
{
	unsigned char* l_dataCopy;
	int l_size = a_rows * a_stride;

	if(!g_initialized)
	{
		return -99;
	}

	// make a host local copy of the data so we can modify it as we please:
	if(!g_canMapHostMem)
	{
		l_dataCopy = new unsigned char[l_size];
	}
	else
	{
		 cudaHostAlloc((void**)&l_dataCopy, l_size, cudaHostAllocMapped);
	}

	memcpy(l_dataCopy, a_data, l_size);

	// calculate shit:
	int l_kernelResult = BoxBlurA8_Device(l_dataCopy, a_stride, a_rows, a_lobes, g_canMapHostMem);

	if(l_kernelResult > 0)
	{
		// return the modified result to the caller:
		memcpy(a_data, l_dataCopy, l_size);
	}

	// free the copy:
	if(!g_canMapHostMem)
	{
		delete[] l_dataCopy;
	}
	else
	{
		cudaFreeHost(l_dataCopy);
	}

	return l_kernelResult;
}
	
