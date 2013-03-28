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
/* #include <cairo.h> */
#include <math.h>
#include "cairo_box_blur.h"
#include "util.h"
#include <omp.h>


// the CPU fallback for non-MSVC compilers is largely based on
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


CCairoBoxBlur::CCairoBoxBlur(int a_width, int a_height, int a_blurRadius)
{
	m_blurRadius = a_blurRadius;

	m_useFallback = true;

#if defined(_WIN32) && !defined(COMPACT_RELEASE)
	if(CUtil::IsWin6x())
	{
		if(!m_hAmpDll)
		{
			m_hAmpDll = CUtil::SilentLoadLibrary(CUtil::GetExeDir() + L"\\infekt-gpu.dll");
		}

		typedef int (__cdecl *fnc)();

		if(fnc igu = (fnc)::GetProcAddress(m_hAmpDll, "IsGpuUsable"))
		{
			m_useFallback = (igu() == 0);
		}
	}
#endif /* !COMPACT_RELEASE */

	m_imgSurface = cairo_image_surface_create(m_useFallback ? CAIRO_FORMAT_A8 : CAIRO_FORMAT_ARGB32, a_width, a_height);
	m_context = cairo_create(m_imgSurface);

	m_width = a_width;
	m_height = a_height;
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

	#pragma omp parallel for
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

	#pragma omp parallel for
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


bool CCairoBoxBlur::Paint(cairo_t* a_destination)
{
	bool l_ok = false;

	cairo_surface_flush(m_imgSurface);

	unsigned char* l_boxData = cairo_image_surface_get_data(m_imgSurface);

	if(!l_boxData)
	{
		// too large or something...
		return false;
	}

#if defined(_WIN32) && !defined(COMPACT_RELEASE)
	if(!m_useFallback && cairo_image_surface_get_format(m_imgSurface) == CAIRO_FORMAT_ARGB32)
	{
		typedef int (__cdecl *fnc)(unsigned int *img_data, int width, int height, float sigma);

		if(fnc gb = (fnc)GetProcAddress(m_hAmpDll, "GaussianBlurARGB32"))
		{
			l_ok = (gb(
				reinterpret_cast<unsigned int*>(l_boxData),
				cairo_image_surface_get_width(m_imgSurface),
				cairo_image_surface_get_height(m_imgSurface),
				m_blurRadius / 5.0f + 2) != 0);
		}

		if(!l_ok)
		{
			// must retry!

			m_useFallback = true;

			cairo_destroy(m_context);
			cairo_surface_destroy(m_imgSurface);

			m_imgSurface = cairo_image_surface_create(CAIRO_FORMAT_A8, m_width, m_height);
			m_context = cairo_create(m_imgSurface);

			return false;
		}
	}
	else
#endif /* !COMPACT_RELEASE */
	{
		// fallback.

		const PRInt32 l_stride = cairo_image_surface_get_stride(m_imgSurface);
		const PRInt32 l_rows = cairo_image_surface_get_height(m_imgSurface);

		PRInt32 l_lobes[3][2];
		ComputeLobes(m_blurRadius, l_lobes);

		unsigned char* l_tmpData = new unsigned char[l_stride * l_rows];

		BoxBlurHorizontal(l_boxData, l_tmpData, l_lobes[0][0], l_lobes[0][1], l_stride, l_rows);
		BoxBlurHorizontal(l_tmpData, l_boxData, l_lobes[1][0], l_lobes[1][1], l_stride, l_rows);
		BoxBlurHorizontal(l_boxData, l_tmpData, l_lobes[2][0], l_lobes[2][1], l_stride, l_rows);

		BoxBlurVertical(l_tmpData, l_boxData, l_lobes[0][0], l_lobes[0][1], l_stride, l_rows);
		BoxBlurVertical(l_boxData, l_tmpData, l_lobes[1][0], l_lobes[1][1], l_stride, l_rows);
		BoxBlurVertical(l_tmpData, l_boxData, l_lobes[2][0], l_lobes[2][1], l_stride, l_rows);

		delete[] l_tmpData;
	}

	cairo_surface_mark_dirty(m_imgSurface);

	if(m_useFallback)
	{
		// 8 bit mask
		cairo_mask_surface(a_destination, m_imgSurface, 0, 0);
	}
	else
	{
		// 32 bit RGBA processed shit
		cairo_save(a_destination);
		cairo_set_operator(a_destination, CAIRO_OPERATOR_ATOP);
		cairo_set_source_surface(a_destination, m_imgSurface, 0, 0);
		cairo_paint(a_destination);
		cairo_restore(a_destination);
	}

	return true;
}


CCairoBoxBlur::~CCairoBoxBlur()
{
	cairo_destroy(m_context);
	cairo_surface_destroy(m_imgSurface);
}

#if defined(_WIN32) && !defined(COMPACT_RELEASE)
HMODULE CCairoBoxBlur::m_hAmpDll = NULL;
#endif
