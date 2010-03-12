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
#include "cairo_box_blur.h"


CNFORenderer::CNFORenderer(bool a_classicMode)
{
	m_classic = a_classicMode;

	// reset internal flags:
	m_gridData = NULL;
	m_rendered = false;
	m_imgSurface = NULL;
	m_fontSize = -1;
	m_cachedBlur = NULL;

	// default settings:
	if(!m_classic)
	{
		SetBlockSize(7, 12);
		m_settings.uFontSize = 0;

		SetEnableGaussShadow(true);
		SetGaussColor(_S_COLOR_RGB(0, 0, 0));
		SetGaussBlurRadius(10);
	}
	else
	{
		SetFontSize(ms_defaultClassicFontSize);
		m_settings.uBlockHeight = m_settings.uBlockWidth = 0;
		m_settings.uGaussBlurRadius = 0;
		SetEnableGaussShadow(false);

		m_padding = 5;
	}

	SetFontAntiAlias(true);

	SetBackColor(_S_COLOR_RGB(0xFF, 0xFF, 0xFF));
	SetTextColor(_S_COLOR_RGB(0, 0, 0));
	SetArtColor(_S_COLOR_RGB(0, 0, 0));

	SetHilightHyperLinks(true);
	SetHyperLinkColor(_S_COLOR_RGB(0, 0, 0xFF));
	SetUnderlineHyperLinks(true);

	// other stuff:
	m_trueGaussian = false;
}


bool CNFORenderer::AssignNFO(const PNFOData& a_nfo)
{
	if(a_nfo->HasData())
	{
		UnAssignNFO();

		m_nfo = a_nfo;

		CalcClassicModeBlockSizes(true);

		return true;
	}

	return false;
}


void CNFORenderer::UnAssignNFO()
{
#ifdef HAVE_BOOST
	m_nfo.reset();
#else
	m_nfo = NULL;
#endif

	m_rendered = false;
	m_fontSize = -1;
	delete m_gridData;
	m_gridData = NULL;

	delete m_cachedBlur;
	m_cachedBlur = NULL;

	if(m_imgSurface)
	{
		cairo_surface_destroy(m_imgSurface);
		m_imgSurface = NULL;
	}
}


bool CNFORenderer::CalculateGrid()
{
	if(!m_nfo || !m_nfo->HasData())
	{
		return false;
	}

	CRenderGridBlock l_emptyBlock;
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
			l_block->shape = CharCodeToGridShape(m_nfo->GetGridChar(row, col), &l_block->alpha);
			if(l_block->shape == RGS_WHITESPACE && l_textStarted) l_block->shape = RGS_WHITESPACE_IN_TEXT;
			else if(l_block->shape == RGS_NO_BLOCK) l_textStarted = true;
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


/*static*/ ERenderGridShape CNFORenderer::CharCodeToGridShape(wchar_t a_char, uint8_t* ar_alpha)
{
	switch(a_char)
	{
	case 0:
	case 9:
	case 32: /* whitespace */
		return RGS_WHITESPACE;
	case 9600: /* upper half block */
		return RGS_BLOCK_UPPER_HALF;
	case 9604: /* lower half block */
		return RGS_BLOCK_LOWER_HALF;
	case 9608: /* full block */
		return RGS_FULL_BLOCK;
	case 9612: /* left half block */
		return RGS_BLOCK_LEFT_HALF;
	case 9616: /* right half block */
		return RGS_BLOCK_RIGHT_HALF;
	case 9617: /* light shade */
		if(ar_alpha) *ar_alpha = 90;
		return RGS_FULL_BLOCK;
	case 9618: /* medium shade */
		if(ar_alpha) *ar_alpha = 140;
		return RGS_FULL_BLOCK;
		break;
	case 9619: /* dark shade */
		if(ar_alpha) *ar_alpha = 190;
		return RGS_FULL_BLOCK;
		break;
	case 9632: /* black square */
		return RGS_BLACK_SQUARE;
		break;
	case 9642: /* black small square */
		return RGS_BLACK_SMALL_SQUARE;
		break;
	default:
		return RGS_NO_BLOCK;
	}
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
	if(!m_nfo) return false;
	if(!m_gridData && !CalculateGrid()) return false;

	if(!m_imgSurface)
	{
		m_imgSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
			GetWidth(), GetHeight());

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

	if(!m_classic)
	{
		if(GetEnableGaussShadow())
		{
			if(m_trueGaussian)
			{
				// this one (true gaussian blur) is much too slow for
				// big radii.
				RenderBlocks(true, true);
				cairo_blur_image_surface(m_imgSurface, GetGaussBlurRadius());
				/* idea for later: Use NVIDIA CUDA for the gauss blur step. */
			}
			else
			{
				if(!m_cachedBlur)
				{
					m_cachedBlur = new CCairoBoxBlur(GetWidth(), GetHeight(), GetGaussBlurRadius());

					RenderBlocks(false, true, m_cachedBlur->GetContext());
				}

				cairo_t* cr = cairo_create(m_imgSurface);

				if(GetBackColor().A > 0)
				{
					// background:
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetBackColor()));
					cairo_paint(cr);
				}

				// shadow effect:
				cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetGaussColor()));
				m_cachedBlur->Paint(cr);

				cairo_destroy(cr);
			}

			RenderBlocks(false, false);
		}
		else
		{
			RenderBlocks(true, false);
		}

		RenderText();
	}
	else // classic mode
	{
		if(GetBackColor().A > 0)
		{
			cairo_t* cr = cairo_create(m_imgSurface);
			cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetBackColor()));
			cairo_paint(cr);
			cairo_destroy(cr);
		}

		RenderClassic();
	}

	m_rendered = true;

	return true;
}


/************************************************************************/
/* RENDER BLOCKS                                                        */
/************************************************************************/

void CNFORenderer::RenderBlocks(bool a_opaqueBg, bool a_gaussStep, cairo_t* a_context)
{
	double l_off_x = 0, l_off_y = 0;

	cairo_t* cr;
	if(!a_context)
	{
		cr = cairo_create(m_imgSurface);
	}
	else
	{
		cr = a_context;
	}

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

	if(!a_context)
	{
		cairo_destroy(cr);
	}
}


/************************************************************************/
/* Utilities for RenderText and RenderClassic                           */
/************************************************************************/

void CNFORenderer::_FixUpRowColStartEnd(size_t& a_rowStart, size_t& a_colStart, size_t& a_rowEnd, size_t& a_colEnd)
{
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
}

static inline void _SetUpHyperLinkUnderlining(CNFORenderer* r, cairo_t* cr)
{
	if(r->GetHilightHyperLinks() && r->GetUnderlineHyperLinks())
	{
		cairo_set_line_width(cr, 1);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE); // looks better
	}
}

static inline void _SetUpDrawingTools(CNFORenderer* r, cairo_surface_t* a_surface, cairo_t** pcr, cairo_font_options_t** pcfo)
{
	cairo_t* cr = cairo_create(a_surface);

	cairo_font_options_t *cfo = cairo_font_options_create();

	cairo_font_options_set_antialias(cfo, (r->GetFontAntiAlias() ? CAIRO_ANTIALIAS_SUBPIXEL : CAIRO_ANTIALIAS_NONE));
	cairo_font_options_set_hint_style(cfo, (r->IsClassicMode() ? CAIRO_HINT_STYLE_DEFAULT : CAIRO_HINT_STYLE_NONE));

	const std::string l_font
#ifdef _UNICODE
		= CUtil::FromWideStr(r->GetFontFace(), CP_UTF8);
#else
		= r->GetFontFace();
#endif
	cairo_select_font_face(cr, l_font.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_options(cr, cfo);

	if(r->IsClassicMode())
	{
		cairo_set_font_size(cr, r->GetFontSize());
	}

	*pcr = cr;
	*pcfo = cfo;
}

static inline void _FinalizeDrawingTools(cairo_t** pcr, cairo_font_options_t** pcfo)
{
	cairo_font_options_destroy(*pcfo);
	*pcfo = NULL;

	cairo_destroy(*pcr);
	*pcr = NULL;
}


/************************************************************************/
/* RENDER TEXT                                                          */
/************************************************************************/

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

	_FixUpRowColStartEnd(a_rowStart, a_colStart, a_rowEnd, a_colEnd);

	cairo_t* cr;
	cairo_font_options_t* l_fontOptions;
	_SetUpDrawingTools(this, a_surface, &cr, &l_fontOptions);

	_SetUpHyperLinkUnderlining(this, cr);

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
				l_off_y + row * GetBlockHeight() + (l_font_extents.ascent + GetBlockHeight()) / 2.0 - 2);

			cairo_show_text(cr, m_nfo->GetGridCharUtf8(row, col));

			if(l_inLink && !l_linkPos)
			{
				l_inLink = false;
			}
		}
	}

	_FinalizeDrawingTools(&cr, &l_fontOptions);
}


void CNFORenderer::CalcClassicModeBlockSizes(bool a_force)
{
	if(m_classic && (m_fontSize < 0 || a_force))
	{
		cairo_surface_t* l_tmpSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 100);
		cairo_surface_finish(l_tmpSurface); // "freeze" / make read-only, we only need it to measure shit

		cairo_t* cr;
		cairo_font_options_t* l_fontOptions;
		_SetUpDrawingTools(this, l_tmpSurface, &cr, &l_fontOptions);

		wchar_t l_blockStr[2] = { 9608, 0}; // full block + null terminator
		const std::string l_blockStrUtf = CUtil::FromWideStr(l_blockStr, CP_UTF8);

		cairo_text_extents_t l_extents;
		cairo_text_extents(cr, l_blockStrUtf.c_str(), &l_extents);

		m_settings.uBlockWidth = (size_t)l_extents.x_advance;
		m_settings.uBlockHeight = (size_t)l_extents.height;

		_FinalizeDrawingTools(&cr, &l_fontOptions);

		cairo_surface_destroy(l_tmpSurface);

		m_fontSize = 1; // we use this as a flag. pretty gross, huh?
	}
}


void CNFORenderer::RenderClassic()
{
	RenderClassic(GetTextColor(), NULL, GetHyperLinkColor(),
		false, 0, 0, m_nfo->GetGridHeight(), m_nfo->GetGridWidth(),
		m_imgSurface, 0, 0);
}


void CNFORenderer::RenderClassic(const S_COLOR_T& a_textColor, const S_COLOR_T* a_backColor,
								 const S_COLOR_T& a_hyperLinkColor, bool a_backBlocks,
								 size_t a_rowStart, size_t a_colStart, size_t a_rowEnd, size_t a_colEnd,
								 cairo_surface_t* a_surface, double a_xBase, double a_yBase)
{
	double l_off_x = a_xBase + m_padding, l_off_y = a_yBase + m_padding;

	_FixUpRowColStartEnd(a_rowStart, a_colStart, a_rowEnd, a_colEnd);

	cairo_t* cr;
	cairo_font_options_t* l_fontOptions;
	_SetUpDrawingTools(this, a_surface, &cr, &l_fontOptions);

	_SetUpHyperLinkUnderlining(this, cr);

	CalcClassicModeBlockSizes();

	cairo_font_extents_t l_font_extents;
	cairo_font_extents(cr, &l_font_extents);

	size_t l_rowStart = 0, l_rowEnd = m_gridData->GetRows() - 1;
	if(a_rowStart != (size_t)-1)
	{
		l_rowStart = std::max<size_t>(a_rowStart, l_rowStart);
		l_rowEnd = std::min<size_t>(a_rowEnd, l_rowEnd);
	}

	typedef enum
	{
		_BT_UNDEF = -1,
		BT_TEXT = 1,
		BT_BLOCK,
		BT_LINK
	} _block_color_type;

	std::string l_utfBuf;
	l_utfBuf.reserve(m_nfo->GetGridWidth());

	for(size_t row = l_rowStart; row <= l_rowEnd; row++)
	{
		_block_color_type l_curType = _BT_UNDEF;
		size_t l_bufStart = (size_t)-1;

		for(size_t col = 0; col <= m_gridData->GetCols(); col++)
		{
			if(a_rowStart != (size_t)-1)
			{
				if(row == a_rowStart && col < a_colStart)
					continue;
				else if(row == a_rowEnd && col > a_colEnd)
					col = m_gridData->GetCols();
			}

			_block_color_type l_type;

			if(col != m_gridData->GetCols())
			{
				const CRenderGridBlock *l_block = &(*m_gridData)[row][col];
				const CNFOHyperLink* l_link = NULL;

				if(l_block->shape == RGS_NO_BLOCK)
				{
					if(GetHilightHyperLinks() && (l_link = m_nfo->GetLink(row, col)) != NULL)
						l_type = BT_LINK;
					else
						l_type = BT_TEXT;
				}
				else if(l_block->shape == RGS_WHITESPACE_IN_TEXT && l_curType != BT_LINK)
				{
					l_type = l_curType;
				}
				else
				{
					l_type = BT_BLOCK;
				}
			}
			else
			{
				l_type = _BT_UNDEF;
			}

			if(l_type == l_curType && l_type != _BT_UNDEF)
			{
				l_utfBuf += m_nfo->GetGridCharUtf8(row, col);
				if(l_bufStart == (size_t)-1) l_bufStart = col;

				continue;
			}
			/* else */

			if(l_curType != _BT_UNDEF && !l_utfBuf.empty())
			{
				// draw buffer:
				size_t l_len = (col == m_gridData->GetCols() ? g_utf8_strlen(l_utfBuf.c_str(), -1) :
					(col - l_bufStart));

				// draw char background for highlights/selection etc:
				if(a_backColor && (l_curType != BT_BLOCK || a_backBlocks))
				{
					cairo_save(cr);
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(*a_backColor));
					cairo_rectangle(cr, l_off_x + l_bufStart * GetBlockWidth(), row * GetBlockHeight() + l_off_y,
						GetBlockWidth() * l_len, GetBlockHeight());
					cairo_fill(cr);
					cairo_restore(cr);
				}

				if(l_curType == BT_LINK && GetUnderlineHyperLinks())
				{
					cairo_move_to(cr, l_off_x + l_bufStart * GetBlockWidth(), l_off_y + (row + 1) * GetBlockHeight() - 1);
					cairo_rel_line_to(cr, GetBlockWidth() * l_len, 0);
					cairo_stroke(cr);
				}

				cairo_move_to(cr, l_off_x + l_bufStart * GetBlockWidth(),
					row * GetBlockHeight() + l_off_y + l_font_extents.ascent);

				cairo_show_text(cr, l_utfBuf.c_str());
			}

			// set up new type
			l_curType = l_type;

			if(l_type != _BT_UNDEF)
			{
				if(l_type == BT_LINK)
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_hyperLinkColor));
				}
				else if(l_type == BT_TEXT || a_backBlocks)
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_textColor));
				}
				else if(l_type == BT_BLOCK)
				{
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetArtColor()));
				}

				l_utfBuf = m_nfo->GetGridCharUtf8(row, col);
				l_bufStart = col;
			}
		} /* end of inner for loop */
	}

	_FinalizeDrawingTools(&cr, &l_fontOptions);
}


bool CNFORenderer::IsTextChar(size_t a_row, size_t a_col, bool a_allowWhiteSpace) const
{
	if(!m_gridData) return false;

	if(a_row < m_gridData->GetRows() && a_col < m_gridData->GetCols())
	{
		const CRenderGridBlock *l_block = &(*m_gridData)[a_row][a_col];

		if(m_classic)
		{
			return a_allowWhiteSpace || (l_block->shape != RGS_WHITESPACE);
		}
		else
		{
			return (l_block->shape == RGS_NO_BLOCK ||
				(a_allowWhiteSpace && l_block->shape == RGS_WHITESPACE_IN_TEXT));
		}
	}

	return false;
}


size_t CNFORenderer::GetWidth()
{
	if(!m_nfo) return 0;

	return m_nfo->GetGridWidth() * GetBlockWidth() + m_padding * 2;
}


size_t CNFORenderer::GetHeight()
{
	if(!m_nfo) return 0;

	return m_nfo->GetGridHeight() * GetBlockHeight() + m_padding * 2;
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
	// if there has really been a change.

	if(!m_classic)
	{
		if(ns.uBlockWidth < 200 && ns.uBlockHeight < 200)
			SetBlockSize(ns.uBlockWidth, ns.uBlockHeight);

		SetEnableGaussShadow(ns.bGaussShadow);
		SetGaussColor(ns.cGaussColor);
		if(ns.uGaussBlurRadius <= 100)
			SetGaussBlurRadius(ns.uGaussBlurRadius);
	}
	else 
	{
		if(ns.uFontSize >= 3 && ns.uFontSize < 200)
			SetFontSize(ns.uFontSize);
		else
			SetFontSize(ms_defaultClassicFontSize);

		SetEnableGaussShadow(false);
		SetGaussBlurRadius(0);
	}

	SetBackColor(ns.cBackColor);
	SetTextColor(ns.cTextColor);
	SetArtColor(ns.cArtColor);

	SetHilightHyperLinks(ns.bHilightHyperlinks);
	SetHyperLinkColor(ns.cHyperlinkColor);
	SetUnderlineHyperLinks(ns.bUnderlineHyperlinks);

	SetFontAntiAlias(ns.bFontAntiAlias);
	SetFontFace(ns.sFontFace);

	if(!m_rendered && m_imgSurface != NULL)
	{
		cairo_surface_destroy(m_imgSurface);
		m_imgSurface = NULL;
	}
}


CNFORenderer::~CNFORenderer()
{
	delete m_gridData;

	if(m_imgSurface)
		cairo_surface_destroy(m_imgSurface);
}

