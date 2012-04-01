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

	// now about the hard way...

	return false;
}


CNFOToPNG::~CNFOToPNG()
{
}
