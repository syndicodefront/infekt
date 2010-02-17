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


CNFORenderer::CNFORenderer(bool a_classicMode)
{
	m_classic = a_classicMode;

	// reset internal flags:
	m_gridData = NULL;
	m_rendered = false;
	m_imgSurface = NULL;
	m_fontSize = -1;

	// default settings:
	SetBlockSize(7, 12);

	SetEnableGaussShadow(true);
	SetGaussColor(_S_COLOR_RGB(0, 0, 0));
	SetGaussBlurRadius(10);

	SetBackColor(_S_COLOR_RGB(0xFF, 0xFF, 0xFF));
	SetTextColor(_S_COLOR_RGB(0, 0, 0));
	SetArtColor(_S_COLOR_RGB(0, 0, 0));

	SetHilightHyperLinks(true);
	SetHyperLinkColor(_S_COLOR_RGB(0, 0, 0xFF));
	SetUnderlineHyperLinks(true);
}


bool CNFORenderer::AssignNFO(const PNFOData& a_nfo)
{
	if(a_nfo->HasData())
	{
		m_nfo = a_nfo;

		m_rendered = false;
		m_fontSize = -1;
		delete m_gridData;
		m_gridData = NULL;

		if(m_imgSurface)
		{
			cairo_surface_destroy(m_imgSurface);
			m_imgSurface = NULL;
		}

		return true;
	}

	return false;
}


bool CNFORenderer::CalculateGrid()
{
	if(!m_nfo || !m_nfo->HasData())
	{
		return false;
	}

	CRenderGridBlock l_emptyBlock;
	l_emptyBlock.charCode = 0;
	l_emptyBlock.shape = RGS_NO_BLOCK;
	l_emptyBlock.alpha = 255;

	delete m_gridData;

	m_gridData = new TwoDimVector<CRenderGridBlock>(m_nfo->GetGridHeight(),
		m_nfo->GetGridWidth(), l_emptyBlock);
	TwoDimVector<CRenderGridBlock>& l_grid = (*m_gridData);

	for(size_t row = 0; row < m_gridData->GetRows(); row++)
	{
		bool l_textStarted = false;

		for(size_t col = 0; col < m_gridData->GetCols(); col++)
		{
			CRenderGridBlock *l_block = &l_grid[row][col];
			l_block->charCode = m_nfo->GetGridChar(row, col);

			switch(l_block->charCode)
			{
			case 0:
			case 9:
			case 32: /* whitespace */
				l_block->shape = (l_textStarted ? RGS_WHITESPACE_IN_TEXT : RGS_WHITESPACE);
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
				l_textStarted = true;
			}
		}

		if(l_textStarted)
		{
			for(size_t col = m_gridData->GetCols() - 1; col > 0; col--)
			{
				CRenderGridBlock *l_block = &l_grid[row][col];

				if(l_block->shape == RGS_WHITESPACE_IN_TEXT)
				{
					l_block->shape = RGS_WHITESPACE;
				}
				else if(l_block->shape == RGS_NO_BLOCK)
				{
					break;
				}
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
	if(!m_gridData && !CalculateGrid()) return false;

	if(!m_imgSurface)
	{
		m_imgSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
			m_gridData->GetCols() * GetBlockWidth() + m_padding * 2,
			m_gridData->GetRows() * GetBlockHeight() + m_padding * 2);

		if(!m_imgSurface)
		{
			return false;
		}
	}
	else
	{
		// clean. :TODO: find out if really necessary.
		cairo_t* cr = cairo_create(m_imgSurface);
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint(cr);
		cairo_destroy(cr);
	}

	if(GetEnableGaussShadow())
	{
		RenderBlocks(true, true);
		cairo_blur_image_surface(m_imgSurface, GetGaussBlurRadius());
		/* idea for later: Use NVIDIA CUDA for the gauss blur step. */
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

	if(a_opaqueBg && GetBackColor().A > 0)
	{
		cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetBackColor()));
		cairo_paint(cr);
	}

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

			if(l_block->shape == RGS_NO_BLOCK ||
				l_block->shape == RGS_WHITESPACE ||
				l_block->shape == RGS_WHITESPACE_IN_TEXT)
			{
				continue;
			}

			if(l_block->alpha != l_oldAlpha) // R,G,B never change during the loop.
			{
				if(GetEnableGaussShadow() && a_gaussStep)
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO(GetGaussColor()), (l_block->alpha / 255.0) * (GetGaussColor().A / 255.0));
				}
				else
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO(GetArtColor()), (l_block->alpha / 255.0) * (GetArtColor().A / 255.0));
				}
				l_oldAlpha = l_block->alpha;
			}

			double l_pos_x = col * GetBlockWidth(),
				l_pos_y = row * GetBlockHeight(),
				l_width = GetBlockWidth(),
				l_height = GetBlockHeight();

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
				l_width = l_height = GetBlockWidth() * 0.75;
				l_pos_y += GetBlockHeight() / 2.0 - l_height / 2.0;
				l_pos_x += GetBlockWidth() / 2.0 - l_width / 2.0;
				break;
			case RGS_BLACK_SMALL_SQUARE:
				l_width = l_height = GetBlockWidth() * 0.5;
				l_pos_y += GetBlockHeight() / 2.0 - l_height / 2.0;
				l_pos_x += GetBlockWidth() / 2.0 - l_width / 2.0;
				break;
			}

			cairo_rectangle(cr, l_off_x + l_pos_x, l_off_y + l_pos_y, l_width, l_height);
			cairo_fill(cr);
		}
	}

	cairo_destroy(cr);
}


void CNFORenderer::RenderText()
{
	RenderText(GetTextColor(), NULL, GetHyperLinkColor(), (size_t)-1, 0, 0, 0, m_imgSurface, 0, 0);
}


void CNFORenderer::RenderText(const S_COLOR_T& a_textColor, const S_COLOR_T* a_backColor,
							  const S_COLOR_T& a_hyperLinkColor,
							  size_t a_rowStart, size_t a_colStart, size_t a_rowEnd, size_t a_colEnd,
							  cairo_surface_t* a_surface, double a_xBase, double a_yBase)
{
	double l_off_x = a_xBase + m_padding, l_off_y = a_yBase + m_padding;

	if(a_rowStart != (size_t)-1)
	{
		if(a_rowEnd < a_rowStart)
		{
			size_t tmp = a_rowStart; a_rowStart = a_rowEnd; a_rowEnd = tmp;
			tmp = a_colStart; a_colStart = a_colEnd; a_colEnd = tmp;
		}
		else if(a_rowEnd == a_rowStart && a_colStart > a_colEnd)
		{
			size_t tmp = a_colStart; a_colStart = a_colEnd; a_colEnd = tmp;
		}
	}

	// set up drawing tools:
	cairo_t* cr = cairo_create(a_surface);

	cairo_font_options_t *l_fontOptions = cairo_font_options_create();
	cairo_font_options_set_antialias(l_fontOptions, CAIRO_ANTIALIAS_SUBPIXEL);
	cairo_font_options_set_hint_style(l_fontOptions, CAIRO_HINT_STYLE_NONE);

	cairo_select_font_face(cr, "Lucida Console", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_options(cr, l_fontOptions);

	if(GetHilightHyperLinks() && GetUnderlineHyperLinks())
	{
		cairo_set_line_width(cr, 1);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE); // looks better
	}

	if(m_fontSize < 1)
	{
		double l_fontSize = GetBlockWidth();
		bool l_broken = false, l_foundText = false;

		// calculate font size that fits into blocks of the given size:
		do
		{
			cairo_set_font_size(cr, l_fontSize + 1);

			for(size_t row = 0; row < m_gridData->GetRows() && !l_broken; row++)
			{
				for(size_t col = 0; col < m_gridData->GetCols() && !l_broken; col++)
				{
					CRenderGridBlock *l_block = &(*m_gridData)[row][col];

					if(l_block->shape != RGS_NO_BLOCK)
					{
						continue;
					}

					cairo_text_extents_t l_extents;
					cairo_text_extents(cr, m_nfo->GetGridCharUtf8(row, col), &l_extents);

					if(l_extents.width > GetBlockWidth() || l_extents.height > GetBlockHeight())
					{
						l_broken = true;
					}

					l_foundText = true;
				}
			}

			if(!l_broken)
			{
				l_fontSize++;
			}

		} while(!l_broken && l_foundText);

		m_fontSize = l_fontSize + 1;
	}
	else
	{
		cairo_set_font_size(cr, m_fontSize);
	}

	// get general font info to vertically center chars into the blocks:
	cairo_font_extents_t l_font_extents;
	cairo_font_extents(cr, &l_font_extents);

	// draw the chars:
	size_t l_rowStart = 0, l_rowEnd = m_gridData->GetRows() - 1;
	if(a_rowStart != (size_t)-1)
	{
		l_rowStart = std::max<size_t>(a_rowStart, l_rowStart);
		l_rowEnd = std::min<size_t>(a_rowEnd, l_rowEnd);
	}

	cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_textColor));

	for(size_t row = l_rowStart; row <= l_rowEnd; row++)
	{
		size_t l_linkPos = 0;
		bool l_inLink = false;

		for(size_t col = 0; col < m_gridData->GetCols(); col++)
		{
			const CRenderGridBlock *l_block = &(*m_gridData)[row][col];

			if(l_block->shape != RGS_NO_BLOCK && l_block->shape != RGS_WHITESPACE_IN_TEXT)
			{
				continue;
			}

			if(a_rowStart != (size_t)-1)
			{
				if(row == a_rowStart && col < a_colStart)
					continue;
				else if(row == a_rowEnd && col > a_colEnd)
					break;
			}

			if(GetHilightHyperLinks())
			{
				// deal with hyper links:
				if(!l_linkPos)
				{
					const CNFOHyperLink* l_linkInfo = m_nfo->GetLink(row, col);
					if(l_linkInfo)
					{
						l_linkPos = l_linkInfo->GetLength();
						l_inLink = true;
					}
				}
				else
				{
					l_linkPos--;

					if(l_inLink && !l_linkPos)
					{
						l_inLink = false;
					}
				}
			}

			// draw char background for highlights/selection etc:
			if(a_backColor)
			{
				cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(*a_backColor));
				cairo_rectangle(cr, l_off_x + col * GetBlockWidth(), l_off_y + row * GetBlockHeight(), GetBlockWidth(), GetBlockHeight());
				cairo_fill(cr);
			}

			if(GetHilightHyperLinks())
			{
				// set color...
				if(l_inLink)
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_hyperLinkColor));

					if(GetUnderlineHyperLinks())
					{
						cairo_move_to(cr, l_off_x + col * GetBlockWidth(), l_off_y + (row + 1) * GetBlockHeight());
						cairo_rel_line_to(cr, GetBlockWidth(), 0);
						cairo_stroke(cr);
					}
				}
				else
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_textColor));
				}
			}

			// finally draw the text:
			cairo_move_to(cr, l_off_x + col * GetBlockWidth(),
				l_off_y + row * GetBlockHeight() + (l_font_extents.ascent + GetBlockHeight()) / 2.0);

			cairo_show_text(cr, m_nfo->GetGridCharUtf8(row, col));
		}
	}

	cairo_font_options_destroy(l_fontOptions);

	cairo_destroy(cr);
}


bool CNFORenderer::IsTextChar(size_t a_row, size_t a_col, bool a_allowWhiteSpace) const
{
	if(!m_gridData) return false;

	if(a_row < m_gridData->GetRows() && a_col < m_gridData->GetCols())
	{
		const CRenderGridBlock *l_block = &(*m_gridData)[a_row][a_col];

		return (l_block->shape == RGS_NO_BLOCK ||
			(a_allowWhiteSpace && l_block->shape == RGS_WHITESPACE_IN_TEXT));
	}

	return false;
}


size_t CNFORenderer::GetWidth()
{
	if(!m_gridData && !CalculateGrid()) return 0;
	if(m_gridData->GetCols() == 0 || m_gridData->GetRows() == 0) return 0;

	return m_gridData->GetCols() * GetBlockWidth() + m_padding * 2;
}


size_t CNFORenderer::GetHeight()
{
	if(!m_gridData && !CalculateGrid()) return 0;
	if(m_gridData->GetCols() == 0 || m_gridData->GetRows() == 0) return 0;

	return m_gridData->GetRows() * GetBlockHeight() + m_padding * 2;
}


bool CNFORenderer::ParseColor(const char* a_str, S_COLOR_T* ar)
{
	if(ar && _stricmp(a_str, "transparent") == 0)
	{
		ar->R = ar->G = ar->B = 255;
		ar->A = 0;
		return true;
	}

	// HTML/CSS style!
	if(a_str[0] == '#') a_str++;

	int R = 0, G = 0, B = 0, A = 255;

	if(ar && (strlen(a_str) == 8 && sscanf(a_str, "%2x%2x%2x%2x", &R, &G, &B, &A) == 4) ||
		(strlen(a_str) == 6 && sscanf(a_str, "%2x%2x%2x", &R, &G, &B) == 3))
	{
		// it's VERY unlikely these fail with %2x, but whatever...
		if(R >= 0 && R <= 255 &&
			G >= 0 && G <= 255 &&
			B >= 0 && B <= 255 &&
			A >= 0 && A <= 255)
		{
			ar->R = (uint8_t)R;
			ar->G = (uint8_t)G;
			ar->B = (uint8_t)B;
			ar->A = (uint8_t)A;

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


void CNFORenderer::InjectSettings(const CNFORenderSettings& ns)
{
	// use the setter methods so m_rendered only goes "false"
	// if really something has changed.

	if(ns.uBlockWidth < 200 && ns.uBlockHeight < 200)
		SetBlockSize(ns.uBlockWidth, ns.uBlockHeight);

	SetEnableGaussShadow(ns.bGaussShadow);
	SetGaussColor(ns.cGaussColor);
	if(ns.uGaussBlurRadius < 100)
		SetGaussBlurRadius(ns.uGaussBlurRadius);

	SetBackColor(ns.cBackColor);
	SetTextColor(ns.cTextColor);
	SetArtColor(ns.cArtColor);

	SetHilightHyperLinks(ns.bHilightHyperlinks);
	SetHyperLinkColor(ns.cHyperlinkColor);
	SetUnderlineHyperLinks(ns.bUnderlineHyperlinks);
}


CNFORenderer::~CNFORenderer()
{
	delete m_gridData;

	if(m_imgSurface)
		cairo_surface_destroy(m_imgSurface);
}

