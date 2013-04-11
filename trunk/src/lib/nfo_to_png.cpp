/**
 * Copyright (C) 2012 cxxjoe
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

#include "stdafx.h"
#include "nfo_renderer_export.h"
#include <cairo.h>
#include <png.h>


CNFOToPNG::CNFOToPNG(bool a_classicMode)
	: CNFORenderer(a_classicMode)
{
	// the GPU algorithm does not produce a mask, so it can't handle transparency.
	m_forceGPUOff = true;
}


bool CNFOToPNG::SavePNG(const std::_tstring& a_filePath)
{
	if(!Render())
	{
		return false;
	}

	if(m_stripes.size() == 1 && GetHeight() < 32767)
	{
		std::string l_filePath =
	#ifdef _UNICODE
			CUtil::FromWideStr(a_filePath, CP_UTF8);
	#else
			a_filePath;
	#endif
	
		return (cairo_surface_write_to_png(*m_stripes[0], l_filePath.c_str()) == CAIRO_STATUS_SUCCESS);
	}

	return SaveWithLibpng(a_filePath);
}


// there is no built-in filter for this conversion in libpng, therefore, to correctly handle
// the alpha channel, we have to resort to this piece of code from cairo:

/* Unpremultiplies data and converts native endian ARGB => RGBA bytes */
static void
unpremultiply_data(png_structp png, png_row_infop row_info, png_bytep data)
{
	unsigned int i;

	for (i = 0; i < row_info->rowbytes; i += 4) {
		uint8_t *b = &data[i];
		uint32_t pixel;
		uint8_t  alpha;

		memcpy(&pixel, b, sizeof(uint32_t));
		alpha = (pixel & 0xff000000) >> 24;
		if (alpha == 0) {
			b[0] = b[1] = b[2] = b[3] = 0;
		} else {
			b[0] = (((pixel & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
			b[1] = (((pixel & 0x00ff00) >>  8) * 255 + alpha / 2) / alpha;
			b[2] = (((pixel & 0x0000ff) >>  0) * 255 + alpha / 2) / alpha;
			b[3] = alpha;
		}
	}
}

#ifndef DeleteFile
#define DeleteFile(a) unlink(a)
#endif


bool CNFOToPNG::SaveWithLibpng(const std::_tstring& a_filePath)
{
	FILE *fp = NULL;
	png_bytep *row_ptr = NULL;
	bool l_result = false;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if(!png_ptr)
	{
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		return false;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		// this is the error handler.

		png_destroy_write_struct(&png_ptr, &info_ptr);

		if(fp)
		{
			fclose(fp);
			::DeleteFile(a_filePath.c_str());
		}

		if(row_ptr)
		{
			delete[] row_ptr;
			row_ptr = NULL;
		}

		return false;
	}

#ifdef _UNICODE
	if(_wfopen_s(&fp, a_filePath.c_str(), L"wb") == ERROR_SUCCESS)
#else
	if((fp = fopen(a_filePath.c_str(), "wb")) != NULL)
#endif
	{
		bool l_error = true; // we perform some custom user-land error checking, too.
		uint32_t l_imgHeight = static_cast<uint32_t>(GetHeight());

		int bpc = 8;

		png_init_io(png_ptr, fp);

		png_set_IHDR(png_ptr, info_ptr, static_cast<uint32_t>(GetWidth()), l_imgHeight,
			bpc, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		png_color_16 white;
		white.red = white.blue = white.green = white.gray = (1 << bpc) - 1;
		png_set_bKGD(png_ptr, info_ptr, &white);

		png_write_info(png_ptr, info_ptr);
		png_set_write_user_transform_fn(png_ptr, unpremultiply_data);

		row_ptr = new (std::nothrow) png_bytep[l_imgHeight];

		if(row_ptr)
		{
			size_t l_png_row = 0;

			for(size_t l_stripe = 0; l_stripe < m_numStripes; l_stripe++)
			{
				cairo_surface_t * const l_surface = GetStripeSurface(l_stripe);

				unsigned char *l_data = cairo_image_surface_get_data(l_surface);
				size_t l_stride = cairo_image_surface_get_stride(l_surface);
				size_t l_num_rows = cairo_image_surface_get_height(l_surface);

				_ASSERT(l_num_rows == GetStripeHeight(l_stripe));

				if(cairo_surface_status(l_surface) == CAIRO_STATUS_SUCCESS)
				{
					for(size_t l_row = 0; l_row < l_num_rows; l_row++)
					{
						if(l_png_row >= l_imgHeight)
						{
							// remember that (m_numStripes * m_stripeHeight + m_padding * 2) can legitimately
							// exceed the actual GetHeight() value!
							if(l_stripe != m_numStripes - 1)
							{
								// however this would be FUCKING ILLEGAL
								l_png_row = (size_t)-1;
							}
							break;
						}

						row_ptr[l_png_row] = (png_bytep)(&l_data[l_row * l_stride]);

						l_png_row++;
					}
				}
				else
				{
					break;
				}
			}

			if(l_png_row == l_imgHeight)
			{
				// everything went well so far.
				png_set_rows(png_ptr, info_ptr, row_ptr);

				l_error = false;
			}
		}

		if(!l_error)
		{
			png_write_png(png_ptr, info_ptr, 0, NULL);

			fclose(fp);

			l_result = true;
		}
		else
		{
			fclose(fp);
			::DeleteFile(a_filePath.c_str());
		}

		if(row_ptr)
		{
			delete[] row_ptr;
			row_ptr = NULL;
		}
	}

	png_destroy_write_struct(&png_ptr, &info_ptr); 

	return l_result;
}


CNFOToPNG::~CNFOToPNG()
{
}
