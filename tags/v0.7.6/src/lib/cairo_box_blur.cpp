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

#include "stdafx.h" // remove if you don't have one
#include <cairo-win32.h>
#include <math.h>
#include "cairo_box_blur.h"
#include "util.h"

// largely based on
// http://mxr.mozilla.org/mozilla1.9.2/source/gfx/thebes/src/gfxBlur.cpp
// (GPL v2)

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

// mozilla code stuff
typedef int32_t PRInt32;
#define PR_MAX(a, b) ((a) > (b) ? (a) : (b))
#define PR_MIN(a, b) ((a) < (b) ? (a) : (b))
#define NS_ASSERTION(a, b) _ASSERT(a)


CCairoBoxBlur::CCairoBoxBlur(int a_width, int a_height, int a_blurRadius, bool a_allowHwAccel)
{
	m_blurRadius = a_blurRadius;

	// Make an alpha-only surface to draw on. We will play with the data after
	// everything is drawn to create a blur effect.
	m_imgSurface = cairo_image_surface_create(CAIRO_FORMAT_A8, a_width, a_height);

	m_context = cairo_create(m_imgSurface);

	m_allowHwAccel = a_allowHwAccel;
	m_computed = false;
}


/**
 * Box blur involves looking at one pixel, and setting its value to the average
 * of its neighbouring pixels.
 * @param aInput The input buffer.
 * @param aOutput The output buffer.
 * @param aLeftLobe The number of pixels to blend on the left.
 * @param aRightLobe The number of pixels to blend on the right.
 * @param aStride The stride of the buffers.
 * @param aRows The number of rows in the buffers.
 */
static void
BoxBlurHorizontal(unsigned char* aInput,
                  unsigned char* aOutput,
                  PRInt32 aLeftLobe,
                  PRInt32 aRightLobe,
                  PRInt32 aStride,
                  PRInt32 aRows)
{
	PRInt32 boxSize = aLeftLobe + aRightLobe + 1;

	for (PRInt32 y = 0; y < aRows; y++) {
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
}

/**
 * Identical to BoxBlurHorizontal, except it blurs top and bottom instead of
 * left and right.
 */
static void
BoxBlurVertical(unsigned char* aInput,
                unsigned char* aOutput,
                PRInt32 aTopLobe,
                PRInt32 aBottomLobe,
                PRInt32 aStride,
                PRInt32 aRows)
{
	PRInt32 boxSize = aTopLobe + aBottomLobe + 1;

	for (PRInt32 x = 0; x < aStride; x++) {
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
}

static void ComputeLobes(PRInt32 aRadius, PRInt32 aLobes[3][2])
{
    PRInt32 major, minor, final;

    /* See http://www.w3.org/TR/SVG/filters.html#feGaussianBlur for
     * some notes about approximating the Gaussian blur with box-blurs.
     * The comments below are in the terminology of that page.
     */
    PRInt32 z = aRadius/3;
    switch (aRadius % 3) {
    case 0:
        // aRadius = z*3; choose d = 2*z + 1
        major = minor = final = z;
        break;
    case 1:
        // aRadius = z*3 + 1
        // This is a tricky case since there is no value of d which will
        // yield a radius of exactly aRadius. If d is odd, i.e. d=2*k + 1
        // for some integer k, then the radius will be 3*k. If d is even,
        // i.e. d=2*k, then the radius will be 3*k - 1.
        // So we have to choose values that don't match the standard
        // algorithm.
        major = z + 1;
        minor = final = z;
        break;
    case 2:
        // aRadius = z*3 + 2; choose d = 2*z + 2
        major = final = z + 1;
        minor = z;
        break;
    }
    NS_ASSERTION(major + minor + final == aRadius,
                 "Lobes don't sum to the right length");

    aLobes[0][0] = major;
    aLobes[0][1] = minor;
    aLobes[1][0] = minor;
    aLobes[1][1] = major;
    aLobes[2][0] = final;
    aLobes[2][1] = final;
}


void CCairoBoxBlur::Paint(cairo_t* a_destination)
{
	if(!m_computed)
	{
		unsigned char* l_boxData = cairo_image_surface_get_data(m_imgSurface);

		const PRInt32 l_stride = cairo_image_surface_get_stride(m_imgSurface);
		const PRInt32 l_rows = cairo_image_surface_get_height(m_imgSurface);

		PRInt32 l_lobes[3][2];
		ComputeLobes(m_blurRadius, l_lobes);

		bool l_useCpu = true;

#ifdef _WIN32
		if(m_allowHwAccel)
		{
			int l_tmpSize = l_rows * cairo_image_surface_get_width(m_imgSurface);
			if(l_tmpSize < 3000000)
			{
				// CUDA hangs and maybe brings down the system with numbers too large.

				if(CCudaUtil::GetInstance()->IsCudaUsable())
				{
					bool l_ok = true;

					if(!CCudaUtil::GetInstance()->IsCudaThreadInitialized())
					{
						l_ok = CCudaUtil::GetInstance()->InitCudaThread();
					}

					if(l_ok)
					{
						l_useCpu = !CCudaUtil::GetInstance()->DoCudaBoxBlurA8(l_boxData, l_stride, l_rows, l_lobes);
					}
				}

				_ASSERT(!l_useCpu);
			}
		}
#endif /* _WIN32 */

		if(l_useCpu)
		{
			unsigned char* l_tmpData = new unsigned char[l_stride * l_rows];

			BoxBlurHorizontal(l_boxData, l_tmpData, l_lobes[0][0], l_lobes[0][1], l_stride, l_rows);
			BoxBlurHorizontal(l_tmpData, l_boxData, l_lobes[1][0], l_lobes[1][1], l_stride, l_rows);
			BoxBlurHorizontal(l_boxData, l_tmpData, l_lobes[2][0], l_lobes[2][1], l_stride, l_rows);

			BoxBlurVertical(l_tmpData, l_boxData, l_lobes[0][0], l_lobes[0][1], l_stride, l_rows);
			BoxBlurVertical(l_boxData, l_tmpData, l_lobes[1][0], l_lobes[1][1], l_stride, l_rows);
			BoxBlurVertical(l_tmpData, l_boxData, l_lobes[2][0], l_lobes[2][1], l_stride, l_rows);

			delete[] l_tmpData;
		}

		m_computed = true;
	}

	cairo_mask_surface(a_destination, m_imgSurface, 0, 0);
}


CCairoBoxBlur::~CCairoBoxBlur()
{
	cairo_destroy(m_context);
	cairo_surface_destroy(m_imgSurface);
}
