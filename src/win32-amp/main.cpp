/**
 * Copyright (C) 2013 syndicode
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

#include <amp.h>
using namespace Concurrency;


// returns nonzero if calling AMP stuff from this DLL is advisable.
extern "C" __declspec(dllexport) int IsGpuUsable()
{
	try
	{
		accelerator l_defaultDevice;

		// http://msdn.microsoft.com/en-us/library/hh873132.aspx
		if(l_defaultDevice.is_emulated ||
			l_defaultDevice.device_path == accelerator::direct3d_ref ||
			l_defaultDevice.device_path == accelerator::cpu_accelerator)
		{
			return 0;
		}
	}
	catch(runtime_exception& ex)
	{
		(void)ex;

		return 0;
	}

	return 1;
}


// from gaussian_blur.cpp:
void gaussian_filter_amp(unsigned int *img_data, int width, int height, float sigma);


// returns 0 if something went seriously wrong.
extern "C" __declspec(dllexport) int GaussianBlurARGB32(unsigned int *img_data, int width, int height, float sigma)
{
	try
	{
		gaussian_filter_amp(img_data, width, height, sigma);

		return 1;
	}
	catch(runtime_exception& ex)
	{
		(void)ex;

		return 0;
	}
}
