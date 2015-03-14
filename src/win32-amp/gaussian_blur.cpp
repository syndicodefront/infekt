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

#include <amp.h>
#include <amp_graphics.h>
using namespace Concurrency;
typedef Concurrency::graphics::float_4 float4;


// convert from float4 to 32-bit ARGB int
__inline unsigned int argbFloatToInt(float4 argb) restrict(amp)
{
	argb.x = direct3d::saturate(argb.x); // clamp to [0.0, 1.0]
	argb.z = direct3d::saturate(argb.z);
	argb.y = direct3d::saturate(argb.y);
	argb.w = direct3d::saturate(argb.w);
	return (unsigned int(argb.x*255)<<24) | (unsigned int(argb.z*255)<<16) | (unsigned int(argb.y*255)<<8) | unsigned int(argb.w*255);
}

// convert from 32-bit ARGB int to float4
__inline float4 argbIntToFloat(unsigned int c) restrict(amp)
{
	float4 argb;
	argb.w = (c & 0xff) / 255.0f;
	argb.y = ((c>>8) & 0xff) / 255.0f;
	argb.z = ((c>>16) & 0xff) / 255.0f;
	argb.x = ((c>>24) & 0xff) / 255.0f;
	return argb;
}


static void recursive_gaussian_first(const array_view<const unsigned int, 2>& data_in, const array_view<float4, 2>& data_out,
	int max, float4 a0, float4 a1, float4 a2, float4 a3, float4 b1, float4 b2, float4 coefp, float4 coefn, index<1> idx) restrict(amp)
{
	// forward pass:

	float4 xp(0.0f); // previous input
	float4 yp(0.0f); // previous output
	float4 yb(0.0f); // previous output by 2

	// clamp to edge (whatever that means):
	xp = argbIntToFloat(data_in(0, idx[0]));
	yb = yp = coefp * xp;

	for(int j = 0; j < max; j++)
	{
		index<2> idxPos(j, idx[0]);

		float4 xc = argbIntToFloat(data_in[idxPos]);
		float4 yc = a0 * xc + a1 * xp - b1 * yp - b2 * yb;

		data_out[idxPos] = yc;

		xp = xc; yb = yp; yp = yc;
	}

	// reverse pass:
	// ensures response is symmetrical.

	float4 xn(0.0f);
	float4 xa(0.0f);
	float4 yn(0.0f);
	float4 ya(0.0f);

	// clamp to edge (whatever that means):
	xn = argbIntToFloat(data_in(max - 1, idx[0]));
	yn = ya = coefn * xn;

	for(int j = max - 1; j >= 0; j--)
	{
		index<2> idxPos(j, idx[0]);

		float4 xc = argbIntToFloat(data_in[idxPos]);
		float4 yc = a2 * xn + a3 * xa - b1 * yn - b2 * ya;

		data_out[idxPos] += yc;

		xa = xn; xn = xc; ya = yn; yn = yc;
	}
}


static void recursive_gaussian_second(const array_view<const float4, 2>& data_in, const array_view<unsigned int, 2>& data_out,
	int max, float4 a0, float4 a1, float4 a2, float4 a3, float4 b1, float4 b2, float4 coefp, float4 coefn, index<1> idx) restrict(amp)
{
	// forward pass:

	float4 xp(0.0f); // previous input
	float4 yp(0.0f); // previous output
	float4 yb(0.0f); // previous output by 2

	// clamp to edge (whatever that means):
	xp = data_in(idx[0], 0);
	yb = yp = coefp * xp;

	for(int j = 0; j < max; j++)
	{
		index<2> idxPos(idx[0], j);

		float4 xc = data_in[idxPos];
		float4 yc = a0 * xc + a1 * xp - b1 * yp - b2 * yb;

		data_out[idxPos] = argbFloatToInt(yc);

		xp = xc; yb = yp; yp = yc;
	}

	// reverse pass:
	// ensures response is symmetrical.

	float4 xn(0.0f);
	float4 xa(0.0f);
	float4 yn(0.0f);
	float4 ya(0.0f);

	// clamp to edge (whatever that means):
	xn = data_in(idx[0], max - 1);
	yn = ya = coefn * xn;

	for(int j = max - 1; j >= 0; j--)
	{
		index<2> idxPos(idx[0], j);

		float4 xc = data_in[idxPos];
		float4 yc = a2 * xn + a3 * xa - b1 * yn - b2 * ya;

		data_out[idxPos] += argbFloatToInt(yc);

		xa = xn; xn = xc; ya = yn; yn = yc;
	}
}


void gaussian_filter_amp(unsigned int *img_data, int width, int height, float sigma)
{
	if(sigma > 22)
		sigma = 22;
	else if(sigma < 0.1f)
		sigma = 0.1f;

	// compute filter coefficient (a.k.a. MAGIC):
	// formula is from NViDiA CUDA SDK samples.
	const float
		nsigma = sigma < 0.1f ? 0.1f : sigma,
		alpha = 1.695f / nsigma,
		ema = std::exp(-alpha),
		ema2 = std::exp(-2 * alpha),
		b1 = -2 * ema,
		b2 = ema2,
		k = (1 - ema) * (1 - ema) / (1 + 2 * alpha * ema - ema2),
		a0 = k,
		a1 = k * (alpha - 1) * ema,
		a2 = k * (alpha + 1) * ema,
		a3 = -k * ema2,
		coefp = (a0 + a1) / (1 + b1 + b2),
		coefn = (a2 + a3) / (1 + b1 + b2);

	array_view<unsigned int, 2> av_img(height, width, img_data);

	array<float4, 2> a_temp(height, width);
	array_view<float4, 2> av_temp(a_temp);

	av_temp.discard_data();

	// process columns:

	extent<1> e_cols(width);

	parallel_for_each(e_cols, [=](index<1> idx) restrict(amp) {
		recursive_gaussian_first(av_img, av_temp, height, a0, a1, a2, a3, b1, b2, coefp, coefn, idx);
	});

	av_temp.synchronize();

	// process rows:

	extent<1> e_rows(height);

	parallel_for_each(e_rows, [=](index<1> idx) restrict(amp) {
		recursive_gaussian_second(av_temp, av_img, width, a0, a1, a2, a3, b1, b2, coefp, coefn, idx);
	});

	av_img.synchronize();
}
