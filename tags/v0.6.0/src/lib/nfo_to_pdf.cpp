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

#include "stdafx.h"
#include "nfo_renderer_export.h"
#include <cairo-pdf.h>

#ifdef CAIRO_HAS_PDF_SURFACE

CNFOToPDF::CNFOToPDF(bool a_classicMode)
	: CNFORenderer(a_classicMode)
{
	m_dinSizes = false;
}


bool CNFOToPDF::SavePDF(const std::_tstring a_filePath)
{
	double l_pageWidth, l_pageHeight;

	if(!CalcPageDimensions(l_pageWidth, l_pageHeight))
	{
		return false;
	}

	std::string l_filePath =
#ifdef _UNICODE
		CUtil::FromWideStr(a_filePath, CP_UTF8);
#else
		a_filePath;
#endif

	cairo_surface_t* l_pdfSurface = cairo_pdf_surface_create(l_filePath.c_str(), l_pageWidth, l_pageHeight);

	if(cairo_surface_status(l_pdfSurface) != CAIRO_STATUS_SUCCESS)
	{
		return false;
	}

	if(m_imgSurface)
	{
		cairo_surface_destroy(m_imgSurface);
	}

	m_imgSurface = l_pdfSurface;
	m_rendered = false;

	SetEnableGaussShadow(false);

	cairo_surface_set_device_offset(m_imgSurface, (l_pageWidth - GetWidth()) / 2.0, (l_pageHeight - GetHeight()) / 2.0);

	Render();

	cairo_surface_destroy(l_pdfSurface);
	m_imgSurface = NULL;

	return true;
}


bool CNFOToPDF::CalcPageDimensions(double& a_width, double& a_height)
{
	if(m_dinSizes)
	{
		double l_mms[8][2] = {
			{ 148, 210 },
			{ 210, 297 },
			{ 297, 420 },
			{ 420, 594 },
			{ 594, 841 },
			{ 841, 1189 },
			{ 1189, 1682 },
			{ 1682, 2378 },
		};

		double l_pw = GetWidth(), l_ph = GetHeight();

		for(int i = 0; i < 8; i++)
		{
			// millimeters to points:
			double l_w = l_mms[i][0] * (72.0 / 25.4), l_h = l_mms[i][1] * (72.0 / 25.4);

			if(l_w >= l_pw && l_h >= l_ph)
			{
				a_width = l_w;
				a_height = l_h;

				return true;
			}
		}

		return false;
	}
	else
	{
		// 50 = some extra padding
		a_width = GetWidth() + 50;
		a_height = GetHeight() + 50;

		return true;
	}
}


CNFOToPDF::~CNFOToPDF()
{
}

#endif /* CAIRO_HAS_PDF_SURFACE */
