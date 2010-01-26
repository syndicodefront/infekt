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
#include "nfo_renderer.h"


CNFORenderer::CNFORenderer()
{
	// reset internal flags:
	m_gridData = NULL;

	m_rendered = false;
	m_imgSurface = NULL;

	// default settings:
	m_blockWidth = 7;
	m_blockHeight = 12;

	m_gaussShadow = true;
	m_gaussColor = _S_COLOR_RGB(0, 0, 0xA0);

	m_backColor = _S_COLOR_RGB(0xFF, 0xFF, 0xFF);
	m_textColor = _S_COLOR_RGB(0, 0, 0);
	m_artColor = _S_COLOR_RGB(0, 0, 0);

	// non-settings derived from settings:
	SetGaussBlurRadius(10);
}


bool CNFORenderer::AssignNFO(const PNFOData& a_nfo)
{
	m_nfo = a_nfo;
	m_rendered = false;

	return true;
}


bool CNFORenderer::CalculateGrid()
{
	CRenderGridBlock l_emptyBlock;
	l_emptyBlock.charCode = 0;
	l_emptyBlock.shape = RGS_NO_BLOCK;
	l_emptyBlock.alpha = 255;
	memset(&l_emptyBlock.utf8, 0, sizeof(l_emptyBlock.utf8));

	delete m_gridData;

	m_gridData = new TwoDimVector<CRenderGridBlock>(m_nfo->GetGridHeight(),
		m_nfo->GetGridWidth(), l_emptyBlock);
	TwoDimVector<CRenderGridBlock>& l_grid = (*m_gridData);

	for(size_t row = 0; row < m_gridData->GetRows(); row++)
	{
		for(size_t col = 0; col < m_gridData->GetCols(); col++)
		{
			CRenderGridBlock *l_block = &l_grid[row][col];
			l_block->charCode = m_nfo->GetGridChar(row, col);

			switch(l_block->charCode)
			{
			case 0:
			case 9:
			case 32: /* whitespace */
				l_block->shape = RGS_WHITESPACE;
				break;

			case 9600: /* upper half block */
				l_block->shape = RGS_BLOCK_UPPER_HALF;
				break;

			case 9604: /* lower half block */
				l_block->shape = RGS_BLOCK_LOWER_HALF;
				break;

			case 9608: /* full block */
				l_block->shape = RGS_FULL_BLOCK;
				break;

			case 9612: /* left half block */
				l_block->shape = RGS_BLOCK_LEFT_HALF;
				break;

			case 9616: /* right half block */
				l_block->shape = RGS_BLOCK_RIGHT_HALF;
				break;

			case 9617: /* light shade */
				l_block->shape = RGS_FULL_BLOCK;
				l_block->alpha = 90;
				break;

			case 9618: /* medium shade */
				l_block->shape = RGS_FULL_BLOCK;
				l_block->alpha = 140;
				break;

			case 9619: /* dark shade */
				l_block->shape = RGS_FULL_BLOCK;
				l_block->alpha = 190;
				break;

			case 9632: /* black square */
				l_block->shape = RGS_BLACK_SQUARE;
				break;

			case 9642: /* black small square */
				l_block->shape = RGS_BLACK_SMALL_SQUARE;
				break;

			default:
				CUtil::OneCharWideToUtf8(l_block->charCode, l_block->utf8);
			}
		}
	}

	return true;
}


bool CNFORenderer::DrawToSurface(cairo_surface_t *a_surface, int dest_x, int dest_y,
	int source_x, int source_y, int width, int height)
{
	if(!m_rendered && !Render())
	{
		return false;
	}

	cairo_t *cr = cairo_create(a_surface);

	cairo_set_source_surface(cr, m_imgSurface, dest_x - source_x, dest_y - source_y);
	cairo_rectangle(cr, dest_x, dest_y, width, height);
	cairo_fill(cr);

	cairo_destroy(cr);

	return true;
}

#include "cairo_image_surface_blur.inc"

bool CNFORenderer::Render()
{
	if(!m_gridData && !CalculateGrid())
	{
		return false;
	}

	if(!m_imgSurface)
	{
		m_imgSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
			m_gridData->GetCols() * m_blockWidth + m_padding * 2,
			m_gridData->GetRows() * m_blockHeight + m_padding * 2);
	}

	if(!m_imgSurface)
	{
		return false;
	}

	if(m_gaussShadow)
	{
		RenderBlocks(true, true);
		//cairo_image_surface_blur(m_imgSurface, m_gaussBlurRadius);
		cairo_blur_image_surface(m_imgSurface, m_gaussBlurRadius);
		/* idea for later: Use NVIDIA CDU for the gauss blur step. */
		RenderBlocks(false, false);
	}
	else
	{
		RenderBlocks(true, false);
	}

	RenderText();

	m_rendered = true;

	return true;
}


void CNFORenderer::RenderBlocks(bool a_opaqueBg, bool a_gaussStep)
{
	double l_off_x = 0, l_off_y = 0;

	cairo_t* cr = cairo_create(m_imgSurface);

	if(a_opaqueBg && m_backColor.A > 0)
	{
		cairo_set_source_rgba(cr, S_COLOR_T_CAIRO(m_backColor), m_backColor.A / 255.0);
		cairo_paint(cr);
	}

	/*if(m_gaussShadow && !a_gaussStep)
	{
		l_off_y = -(m_gaussBlurRadius / 2);
		if(m_gaussBlurRadius < 5) l_off_y--;
		l_off_x = l_off_y;
	}*/

	l_off_x += m_padding;
	l_off_y += m_padding;

	cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);

	int l_oldAlpha = 0;
	cairo_set_source_rgba(cr, 0, 0, 0, l_oldAlpha);

	for(size_t row = 0; row < m_gridData->GetRows(); row++)
	{
		for(size_t col = 0; col < m_gridData->GetCols(); col++)
		{
			CRenderGridBlock *l_block = &(*m_gridData)[row][col];

			if(l_block->shape == RGS_NO_BLOCK || l_block->shape == RGS_WHITESPACE)
			{
				continue;
			}

			if(l_block->alpha != l_oldAlpha)
			{
				if(m_gaussShadow && a_gaussStep)
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO(m_gaussColor), (l_block->alpha / 255.0) * (m_gaussColor.A / 255.0));
				}
				else
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO(m_artColor), (l_block->alpha / 255.0) * (m_artColor.A / 255.0));
				}
				l_oldAlpha = l_block->alpha;
			}

			double l_pos_x = col * m_blockWidth,
				l_pos_y = row * m_blockHeight,
				l_width = m_blockWidth,
				l_height = m_blockHeight;

			switch(l_block->shape)
			{
			case RGS_BLOCK_LOWER_HALF:
				l_pos_y += l_height / 2;
			case RGS_BLOCK_UPPER_HALF:
				l_height /= 2;
				break;
			case RGS_BLOCK_RIGHT_HALF:
				l_pos_x += l_width / 2;
			case RGS_BLOCK_LEFT_HALF:
				l_width /= 2;
				break;
			case RGS_BLACK_SQUARE:
				l_width = l_height = m_blockWidth * 0.75;
				l_pos_y += m_blockHeight / 2.0 - l_height / 2.0;
				l_pos_x += m_blockWidth / 2.0 - l_width / 2.0;
				break;
			case RGS_BLACK_SMALL_SQUARE:
				l_width = l_height = m_blockWidth * 0.5;
				l_pos_y += m_blockHeight / 2.0 - l_height / 2.0;
				l_pos_x += m_blockWidth / 2.0 - l_width / 2.0;
				break;
			}

			/*if(m_gaussShadow && !a_gaussStep)
			{
				cairo_set_source_rgba(cr, 1, 1, 1, 1);
			}*/

			cairo_rectangle(cr, l_off_x + l_pos_x, l_off_y + l_pos_y, l_width, l_height);
			cairo_fill(cr);

			/*if(m_gaussShadow && !a_gaussStep)
			{
				cairo_set_source_rgba(cr, 0xB6/255.0, 0x17/255.0, 0x17/255.0, l_block->alpha / 255.0);
				cairo_rectangle(cr, l_off_x + l_pos_x, l_off_y + l_pos_y, l_width, l_height);
				cairo_fill(cr);
			}*/
		}
	}

	cairo_destroy(cr);
}


void CNFORenderer::RenderText()
{
	double l_off_x = 0, l_off_y = 0;

	cairo_t* cr = cairo_create(m_imgSurface);

	/*if(m_gaussShadow)
	{
		l_off_y = -(m_gaussBlurRadius / 2);
		if(m_gaussBlurRadius < 5) l_off_y--;
		l_off_x = l_off_y;
	}*/

	l_off_x += m_padding;
	l_off_y += m_padding;

	//cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);
	cairo_font_options_t *l_fontOptions = cairo_font_options_create();
	cairo_font_options_set_antialias(l_fontOptions, CAIRO_ANTIALIAS_SUBPIXEL);
	cairo_font_options_set_hint_style(l_fontOptions, CAIRO_HINT_STYLE_NONE);

	cairo_set_source_rgba(cr, S_COLOR_T_CAIRO(m_textColor), m_textColor.A / 255.0);

	cairo_select_font_face(cr, "Lucida Console", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_options(cr, l_fontOptions);

	double l_fontSize = m_blockWidth;
	/*double l_charWidth, l_charHeight;

	for(;;)
	{
		cairo_set_font_size(cr, l_fontSize + 1);

		cairo_font_extents_t l_extents;
		cairo_font_extents(cr, &l_extents);

		if(l_extents.max_x_advance > m_blockWidth || l_extents.height > m_blockHeight)
		{
			break;
		}

		l_charWidth = l_extents.max_x_advance;
		l_charHeight = l_extents.height;

		l_fontSize++;
	}*/

	//cairo_text_extents_t **l_extents_cache = new cairo_text_extents_t*[m_gridData->GetRows()];
	bool l_broken = false;

	do
	{
		cairo_set_font_size(cr, l_fontSize + 1);

		for(size_t row = 0; row < m_gridData->GetRows() && !l_broken; row++)
		{
			// an enabled extents cache needs  && !l_broken ^^^  removed so all rows get allocated no matter what
			//l_extents_cache[row] = new cairo_text_extents_t[m_gridData->GetCols()];

			for(size_t col = 0; col < m_gridData->GetCols() && !l_broken; col++)
			{
				CRenderGridBlock *l_block = &(*m_gridData)[row][col];

				if(l_block->shape != RGS_NO_BLOCK)
				{
					continue;
				}

				//cairo_text_extents(cr, l_block->utf8, &l_extents_cache[row][col]);
				cairo_text_extents_t l_extents;
				cairo_text_extents(cr, l_block->utf8, &l_extents);

				if(l_extents.width > m_blockWidth || l_extents.height > m_blockHeight)
				{
					l_broken = true;
				}
			}
		}

		if(!l_broken)
		{
			l_fontSize++;
		}

	} while(!l_broken);

	cairo_font_extents_t l_font_extents;
	cairo_font_extents(cr, &l_font_extents);

	for(size_t row = 0; row < m_gridData->GetRows(); row++)
	{
		for(size_t col = 0; col < m_gridData->GetCols(); col++)
		{
			CRenderGridBlock *l_block = &(*m_gridData)[row][col];

			if(l_block->shape != RGS_NO_BLOCK)
			{
				continue;
			}

			cairo_move_to(cr, l_off_x + col * m_blockWidth,
				l_off_y + row * m_blockHeight + (l_font_extents.ascent + m_blockHeight) / 2.0);

			cairo_show_text(cr, l_block->utf8);
		}
		//delete[] l_extents_cache[row];
	}

	//delete[] l_extents_cache;

	cairo_font_options_destroy(l_fontOptions);

	cairo_destroy(cr);
}


size_t CNFORenderer::GetWidth()
{
	if(!m_gridData && !CalculateGrid()) return 0;
	if(m_gridData->GetCols() == 0 || m_gridData->GetRows() == 0) return 0;

	return m_gridData->GetCols() * m_blockWidth + m_padding * 2;
}


size_t CNFORenderer::GetHeight()
{
	if(!m_gridData && !CalculateGrid()) return 0;
	if(m_gridData->GetCols() == 0 || m_gridData->GetRows() == 0) return 0;

	return m_gridData->GetRows() * m_blockHeight + m_padding * 2;
}


bool CNFORenderer::ParseColor(const char* a_str, S_COLOR_T* ar)
{
	int R = 0, G = 0, B = 0, A = 255;

	if(a_str[0] == '#') a_str++;

	if(ar && _stricmp(a_str, "transparent") == 0)
	{
		ar->R = ar->G = ar->B = 255;
		ar->A = 0;
		return true;
	}

	if(ar && (strlen(a_str) == 8 && sscanf(a_str, "%2x%2x%2x%2x", &R, &G, &B, &A) == 4) ||
		(strlen(a_str) == 6 && sscanf(a_str, "%2x%2x%2x", &R, &G, &B) == 3))
	{
		if(R >= 0 && R <= 255 &&
			G >= 0 && G <= 255 &&
			B >= 0 && B <= 255 &&
			A >= 0 && A <= 255)
		{
			ar->R = (unsigned char)R;
			ar->G = (unsigned char)G;
			ar->B = (unsigned char)B;
			ar->A = (unsigned char)A;

			return true;
		}
	}

	return false;
}


bool CNFORenderer::ParseColor(const wchar_t* a_str, S_COLOR_T* ar)
{
	const std::string l_color = CUtil::FromWideStr(a_str, CP_UTF8);
	return ParseColor(l_color.c_str(), ar);
}


CNFORenderer::~CNFORenderer()
{
	delete m_gridData;
}


