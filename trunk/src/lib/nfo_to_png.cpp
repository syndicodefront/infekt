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

}


bool CNFOToPNG::SavePNG(const std::wstring& a_filePath)
{
	if(!Render())
	{
		return false;
	}

	/*if(m_stripes.size() == 1 && GetHeight() < 32767)
	{
		std::string l_filePath =
	#ifdef _UNICODE
			CUtil::FromWideStr(a_filePath, CP_UTF8);
	#else
			a_filePath;
	#endif
	
		return (cairo_surface_write_to_png(*m_stripes[0], l_filePath.c_str()) == CAIRO_STATUS_SUCCESS);
	}*/

	return SaveWithLibpng(a_filePath);
}


bool CNFOToPNG::SaveWithLibpng(const std::wstring& a_filePath)
{
	FILE *fp;

	if(_wfopen_s(&fp, a_filePath.c_str(), L"wb") != 0)
	{
		return false;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);//(png_voidp)user_error_ptr, user_error_fn, user_warning_fn);

	if(!png_ptr)
	{
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return false;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, static_cast<uint32_t>(GetWidth()), static_cast<uint32_t>(GetHeight() - m_padding),
		8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_bytep *row_ptr = new (std::nothrow) png_bytep[GetHeight() - m_padding];

	if(row_ptr)
	{
		size_t l_png_row = 0;

		for(size_t l_stripe = 0; l_stripe < m_stripes.size(); l_stripe++)
		{
			unsigned char *l_data = cairo_image_surface_get_data(*m_stripes[l_stripe]);
			size_t l_stride = cairo_image_surface_get_stride(*m_stripes[l_stripe]);
			size_t l_num_rows = cairo_image_surface_get_height(*m_stripes[l_stripe]);

			for(size_t l_row = 0; l_row < l_num_rows; l_row++)
			{
				row_ptr[l_png_row] = (png_bytep)(&l_data[l_row * l_stride]);

				l_png_row++;
			}
		}

		size_t h = GetHeight() - m_padding;
		_ASSERT(l_png_row == h);

		png_set_rows(png_ptr, info_ptr, row_ptr);
	}

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr); 

	fclose(fp);

	if(row_ptr)
	{
		delete[] row_ptr;
	}

	return true;
}


CNFOToPNG::~CNFOToPNG()
{
}
