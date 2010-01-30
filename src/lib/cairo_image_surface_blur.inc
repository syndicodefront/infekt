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

static void cairo_blur_image_surface(cairo_surface_t *surface, int radius)
{
	int kernelSize;
	int kernelSum = 0;
	int *kernel, **mult;

	if(true) /* precalc stuff */
	{
		kernelSize = radius * 2 + 1;
		kernel = new int[kernelSize];
		mult = new int*[kernelSize];
		for(int i = 0; i < kernelSize; i++)
		{
			mult[i] = new int[256];
		}

		for(int i = 1; i <= radius; i++)
		{
			int szi = radius - i, szj = radius + i;
			kernel[szj] = kernel[szi] = (szi + 1) * (szi + 1);
			kernelSum += (kernel[szj] + kernel[szi]);
			for(int j = 0; j < 256; j++)
			{
				mult[szj][j] = mult[szi][j] = kernel[szj] * j;
			}
		}
		kernel[radius] = (radius + 1) * (radius + 1);
		kernelSum += kernel[radius];
		for(int j = 0; j < 256; j++)
		{
			mult[radius][j] = kernel[radius] * j;
		}
	}

	if(cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
	{
		delete[] kernel;
		for(int i = 0; i < kernelSize; i++) delete[] mult[i];
		delete[] mult;
		return;
	}

	int width, height;
	uint8_t *src;

	width = cairo_image_surface_get_width(surface);
	height = cairo_image_surface_get_height(surface);

	// we get: BGRA!
	src = cairo_image_surface_get_data(surface);

	int wh = width * height;

	uint8_t *b2 = new uint8_t[wh];
	uint8_t *g2 = new uint8_t[wh];
	uint8_t *r2 = new uint8_t[wh];
	uint8_t *a2 = new uint8_t[wh];

	int asum, bsum, gsum, rsum;
	int start = 0;
	int index = 0;

	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			asum = bsum = gsum = rsum = 0;
			int read = index - radius;

			for(int z = 0; z < kernelSize; z++)
			{
				int idx = read;

				if(read < start)
					idx = start;
				else if(read > start + width - 1)
					idx = start + width - 1;

				uint8_t* srcptr = src + idx * 4;
				bsum += mult[z][*(srcptr + 0)];
				gsum += mult[z][*(srcptr + 1)];
				rsum += mult[z][*(srcptr + 2)];
				asum += mult[z][*(srcptr + 3)];

				read++;
			}
			b2[index] = (bsum / kernelSum);
			g2[index] = (gsum / kernelSum);
			r2[index] = (rsum / kernelSum);
			a2[index] = (asum / kernelSum);
			index++;
		}
		start += width;
	}

	int tempy;
	uint32_t* ptr = (uint32_t*)src;

	for(int i = 0; i < height; i++)
	{
		int y = i - radius;
		start = y * width;
		for(int j = 0; j < width; j++)
		{
			asum = bsum = gsum = rsum = 0;
			int read = start + j;
			tempy = y;
			for(int z = 0; z < kernelSize; z++)
			{
				int idx = read;

				if(tempy < 0)
					idx = j;
				else if(tempy > height -1)
					idx = wh - (width - j);

				bsum += mult[z][b2[idx]];
				gsum += mult[z][g2[idx]];
				rsum += mult[z][r2[idx]];
				asum += mult[z][a2[idx]];

				read += width;
				++tempy;
			}

			*(ptr++) = (asum / kernelSum) << 24 | (rsum / kernelSum) << 16 | (gsum / kernelSum) << 8 | bsum / kernelSum;
		}
	}

	delete[] kernel;
	for(int i = 0; i < kernelSize; i++) delete[] mult[i];
	delete[] mult;
	delete[] b2;
	delete[] g2;
	delete[] r2;
	delete[] a2;

	cairo_surface_mark_dirty(surface);
}
