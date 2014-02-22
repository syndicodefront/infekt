/**
 * Copyright (C) 2010-2014 cxxjoe
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


CNFORenderer::CNFORenderer(bool a_classicMode) :
	m_classic(a_classicMode),
	m_partial(NRP_RENDER_EVERYTHING),
	m_forceGPUOff(false),
	m_allowCPUFallback(true),
	m_onDemandRendering(false),
	m_preRenderThread(NULL),
	m_gridData(NULL),
	m_rendered(false),
	m_linesPerStripe(0),
	m_stripeHeight(0),
	m_numStripes(0),
	m_fontSize(-1),
	m_zoomFactor(1.0f),
	m_hasBlocks(false),
	m_stopPreRendering(true),
	m_preRenderingStripe((size_t)-1),
	m_cancelRenderingImmediately(false)
{
	// default settings:
	SetFontAntiAlias(true);

	SetBackColor(_S_COLOR_RGB(0xFF, 0xFF, 0xFF));
	SetTextColor(_S_COLOR_RGB(0, 0, 0));
	SetArtColor(_S_COLOR_RGB(0, 0, 0));

	SetHilightHyperLinks(true);
	SetHyperLinkColor(_S_COLOR_RGB(0, 0, 0xFF));
	SetUnderlineHyperLinks(true);

	if(!m_classic)
	{
		SetBlockSize(7, 12);
		m_settings.uFontSize = 0;

		SetEnableGaussShadow(true);
		SetGaussColor(_S_COLOR_RGB(128, 128, 128));
		SetGaussBlurRadius(15);
	}
	else
	{
		SetFontSize(ms_defaultClassicFontSize);
		m_settings.uBlockHeight = m_settings.uBlockWidth = 0;
		m_settings.uGaussBlurRadius = 0;
		SetEnableGaussShadow(false);

		m_padding = 5;
	}
}


bool CNFORenderer::AssignNFO(const PNFOData& a_nfo)
{
	if(a_nfo->HasData())
	{
		UnAssignNFO();

		m_nfo = a_nfo;

		CalcClassicModeBlockSizes(true);

		// the CPU fallback for blurring is 8-bit alpha channel only currently,
		// only the GPU implementation supports more than one color.
		m_allowCPUFallback = !IsAnsi();

		return true;
	}

	return false;
}


void CNFORenderer::UnAssignNFO()
{
	m_nfo.reset();

	m_rendered = false;
	m_fontSize = -1;
	delete m_gridData;
	m_gridData = NULL;

	ClearStripes();
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

	bool l_hasBlocks = false;

	for(size_t row = 0; row < m_gridData->GetRows(); row++)
	{
		bool l_textStarted = false;

		for(size_t col = 0; col < m_gridData->GetCols(); col++)
		{
			CRenderGridBlock *l_block = &l_grid[row][col];

			l_block->shape = CharCodeToGridShape(m_nfo->GetGridChar(row, col), &l_block->alpha);

			if(!l_hasBlocks && l_block->shape != RGS_NO_BLOCK && l_block->shape != RGS_WHITESPACE) l_hasBlocks = true;

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

	m_hasBlocks = l_hasBlocks;

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


bool CNFORenderer::DrawToSurface(cairo_surface_t *a_surface,
	int dest_x, int dest_y, // coordinates in a_surface
	int source_x, int source_y, // coordinates between 0 and GetHeight() / GetWidth()
	int a_width, int a_height)
{
	if(!m_onDemandRendering || m_numStripes == 1)
	{
		if(!m_rendered && !Render())
		{
			return false;
		}
	}
	else if(!m_rendered && !m_gridData)
	{
		// not correctly initialized yet
		return false;
	}

	cairo_t *cr = cairo_create(a_surface);

	// fix for sloppy width + height calculations...
	int l_widthFixed = std::min(a_width, (int)GetWidth() - source_x);
	int l_heightFixed = std::min(a_height, (int)GetHeight() - source_y);

	if(m_numStripes == 1)
	{
		cairo_set_source_surface(cr, *m_stripes[0], dest_x - source_x, dest_y - source_y);

		cairo_rectangle(cr, dest_x, dest_y, l_widthFixed, l_heightFixed);

		cairo_fill(cr);
	}
	else
	{
		int l_stripeStart = (source_y - m_padding) / m_stripeHeight, // implicit floor()
			l_stripeEnd = ((source_y + a_height - m_padding) / m_stripeHeight);

		// sanity confinement ;)
		l_stripeStart = std::min(l_stripeStart, (int)m_numStripes - 1);
		l_stripeEnd = std::min(l_stripeEnd, (int)m_numStripes - 1);

		if(m_onDemandRendering)
		{
			Render(l_stripeStart, l_stripeEnd);
		}

		for(int l_stripe = l_stripeStart; l_stripe <= l_stripeEnd; l_stripe++)
		{
			cairo_surface_t* l_sourceSourface = GetStripeSurface(l_stripe);

			if(!l_sourceSourface)
			{
				// happens when zooming out, shouldn't happen otherwise.
				continue;
			}

			// y pos in complete image:
			int l_stripe_virtual_y = (l_stripe == 0 ? 0 : l_stripe * m_stripeHeight + m_padding);
			// y pos in stripe:
			int l_stripe_source_y = (source_y - l_stripe_virtual_y);

			// height of area of this stripe that is to be painted:
			int l_height = (l_stripe == l_stripeEnd ? l_heightFixed : GetStripeHeight(l_stripe) - l_stripe_source_y);

			if(l_stripe > l_stripeStart)
			{
				// must clip destination or additional pixels that have been added to stripes for
				// correct blurring will be copied, too:
				cairo_save(cr);
				cairo_rectangle(cr, dest_x, l_stripe_virtual_y - source_y, l_widthFixed, l_height);
				cairo_clip(cr);
			}

			// actual copy operation:
			cairo_set_source_surface(cr, l_sourceSourface, dest_x - source_x,
				dest_y - l_stripe_source_y - GetStripeHeightExtraTop(l_stripe));
			cairo_rectangle(cr, dest_x, dest_y, l_widthFixed, l_height);
			cairo_fill(cr);

			if(l_stripe > l_stripeStart)
			{
				// undo clip:
				cairo_restore(cr);
			}
		}
	}

	if(l_heightFixed < a_height)
	{
		cairo_set_source_rgb(cr, S_COLOR_T_CAIRO(GetBackColor()));
		cairo_rectangle(cr, dest_x, dest_y + l_heightFixed, a_width, a_height - l_heightFixed);
		cairo_fill(cr);
	}
	if(l_widthFixed < a_width)
	{
		cairo_set_source_rgb(cr, S_COLOR_T_CAIRO(GetBackColor()));
		cairo_rectangle(cr, dest_x + l_widthFixed, dest_y, a_width - l_widthFixed, a_height);
		cairo_fill(cr);
	}

	cairo_destroy(cr);

	return true;
}


bool CNFORenderer::DrawToClippedHandle(cairo_t* a_cr, int dest_x, int dest_y)
{
	if(!m_rendered && !Render())
	{
		return false;
	}

	cairo_save(a_cr);

	if(m_numStripes == 1)
	{
		cairo_set_source_surface(a_cr, *m_stripes[0], dest_x - 0, dest_y - 0);
		cairo_rectangle(a_cr, dest_x, dest_y,
			static_cast<double>(GetWidth()), static_cast<double>(GetHeight()));
		cairo_fill(a_cr);
	}
	else
	{
		/* :TODO: */
	}

	cairo_restore(a_cr);

	return true;
}


bool CNFORenderer::Render(size_t a_stripeFrom, size_t a_stripeTo)
{
	if(!m_nfo)
	{
		return false;
	}

	if(!m_gridData && !CalculateGrid())
	{
		return false;
	}

	// using these exchangably:
	_ASSERT(m_gridData->GetCols() == m_nfo->GetGridWidth());
	_ASSERT(m_gridData->GetRows() == m_nfo->GetGridHeight());

	if(!m_rendered)
	{
		ClearStripes();

		if(m_classic)
		{
			// we need the block size to check the minimum maximum (no typo) stripe height:
			CalcClassicModeBlockSizes();
		}
		else
		{
			// init font size:
			PreRenderText();
		}

		// recalculate stripe dimensions:

#ifndef COMPACT_RELEASE
		// 2 CPU cores <=> 2000 px stripe height
		// ==> 8 CPU cores <=> 250
		// (= more threads)
		// but: never use less than 500px per stripe.
		size_t l_stripeHeightMaxForCores = std::max(2000 * 2 / omp_get_num_procs(), 500);
#else
		size_t l_stripeHeightMaxForCores = 2000;
#endif /* COMPACT_RELEASE */

		size_t l_stripeHeightMax = std::max(l_stripeHeightMaxForCores, GetBlockHeight() * 2); // MUST not be smaller than one line's height, using two for sanity

		size_t l_numStripes = GetHeight() / l_stripeHeightMax; // implicit floor()
		if(l_numStripes == 0) l_numStripes = 1;
		size_t l_linesPerStripe = m_nfo->GetGridHeight() / l_numStripes; // implicit floor()

		while(l_linesPerStripe * l_numStripes < m_nfo->GetGridHeight())
		{
			// correct rounding errors
			l_numStripes++;
		}

		// storing these three is a bit redundant, but saves code & calculations in other places:
		m_linesPerStripe = l_linesPerStripe;
		m_stripeHeight = static_cast<int>(l_linesPerStripe * GetBlockHeight());
		m_numStripes = l_numStripes;

		_ASSERT(m_stripeHeight * m_numStripes + m_padding * 2 >= GetHeight());
	}

	std::vector<size_t> l_changedStripes;

	a_stripeTo = std::min(a_stripeTo, m_numStripes - 1);
	for(size_t l_stripe = a_stripeFrom; l_stripe <= a_stripeTo; l_stripe++)
	{
		while(l_stripe == m_preRenderingStripe)
		{
			// risky business! not entirely sure if this is safe!
		}

		std::lock_guard<std::mutex> threadlock(m_stripesLock);

		if(!m_stripes[l_stripe])
		{
			m_stripes[l_stripe] = PCairoSurface(new _CCairoSurface(
				cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
				(int)GetWidth(),
				GetStripeHeightPhysical(l_stripe))
			));

			// render each stripe only once:
			l_changedStripes.push_back(l_stripe);
		}	
	}

	_ASSERT(m_stripes.size() <= m_numStripes);

	// for some weird reason, this conditional saves a lot of CPU time when scrolling?!.
	if(l_changedStripes.size() > 0)
	{
		#pragma omp parallel for
		for(int i = 0; i < static_cast<int>(l_changedStripes.size()); i++)
		{
			RenderStripe(l_changedStripes[i]);
		}
	}

	m_rendered = true;

	return true;
}


// wrapper that we can use in const situations:
cairo_surface_t *CNFORenderer::GetStripeSurface(size_t a_stripe) const
{
	std::map<size_t, PCairoSurface>::const_iterator it = m_stripes.find(a_stripe);

	if(it != m_stripes.end())
	{
		// the actual surface is not really const, but that's safe because
		// we are parallelizing per stripe, so one surface can never be used by more than one thread.
		return *it->second.get();
	}

	return NULL;
}


int CNFORenderer::GetStripeHeight(size_t a_stripe) const
{
	int l_height = m_stripeHeight;

	if(a_stripe == 0)
		l_height += m_padding;
	
	if(a_stripe == m_numStripes - 1)
		l_height += m_padding;

	return l_height;
}


// adds some more pixels that allow the blur effect to be rendered without
// issues on the lower and upper edges.
int CNFORenderer::GetStripeHeightPhysical(size_t a_stripe) const
{
	if(IsClassicMode() || m_numStripes < 2)
		return GetStripeHeight(a_stripe);
	else if(a_stripe == 0)
		return GetStripeHeightExtraBottom(a_stripe) + GetStripeHeight(a_stripe);
	else if(a_stripe == m_numStripes - 1)
		return GetStripeHeightExtraTop(a_stripe) + GetStripeHeight(a_stripe);
	else
		return GetStripeHeightExtraTop(a_stripe) + GetStripeHeightExtraBottom(a_stripe) + GetStripeHeight(a_stripe);
}


int CNFORenderer::GetStripeHeightExtraTop(size_t a_stripe) const
{
	return static_cast<int>(GetStripeExtraLinesTop(a_stripe) * GetBlockHeight());
}


int CNFORenderer::GetStripeHeightExtraBottom(size_t a_stripe) const
{
	return static_cast<int>(GetStripeExtraLinesBottom(a_stripe) * GetBlockHeight());
}


size_t CNFORenderer::GetStripeExtraLinesTop(size_t a_stripe) const
{
	if(IsClassicMode() || !GetEnableGaussShadow() || a_stripe == 0)
		return 0;

	return static_cast<size_t>(ceil(static_cast<double>(GetGaussBlurRadius()) / static_cast<double>(GetBlockHeight())));
}


size_t CNFORenderer::GetStripeExtraLinesBottom(size_t a_stripe) const
{
	if(IsClassicMode() || !GetEnableGaussShadow() || a_stripe == m_numStripes - 1)
		return 0;

	return static_cast<size_t>(ceil(static_cast<double>(GetGaussBlurRadius()) / static_cast<double>(GetBlockHeight())));
}


void CNFORenderer::RenderStripe(size_t a_stripe) const
{
	cairo_surface_t * const l_surface = GetStripeSurface(a_stripe);

	_ASSERT(l_surface != NULL);

	if((!m_hasBlocks || m_classic) && GetBackColor().A > 0)
	{
		cairo_t* cr = cairo_create(l_surface);
		cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetBackColor()));
		cairo_paint(cr);
		cairo_destroy(cr);
	}

	// hacke-di-hack (RenderClassic is adding m_padding for historical reasons, so we have to subtract it beforehand.
	// it's also operating on the full NFO image's coordinates, so we have to subtract those too):
	double l_baseY = (a_stripe == 0 ? 0 : -m_padding - (double)a_stripe * m_stripeHeight) + GetStripeHeightExtraTop(a_stripe);
	size_t l_rowStart = (m_numStripes > 1 ? a_stripe * m_linesPerStripe - GetStripeExtraLinesTop(a_stripe) : (size_t)-1),
		l_rowEnd = a_stripe * m_linesPerStripe + m_linesPerStripe + GetStripeExtraLinesBottom(a_stripe);

	if(m_classic)
	{
		RenderClassic(GetTextColor(), NULL, GetHyperLinkColor(),
			false,
			l_rowStart, 0, l_rowEnd, m_nfo->GetGridWidth() - 1,
			l_surface, 0, l_baseY);
	}
	else
	{
		if(m_hasBlocks && GetEnableGaussShadow() && ((m_partial & NRP_RENDER_BLOCKS) != 0 || (m_partial & NRP_RENDER_GAUSS_BLOCKS) != 0))
		{
			if((m_partial & NRP_RENDER_GAUSS_SHADOW) != 0)
			{
				shared_ptr<CCairoBoxBlur> p_blur(new CCairoBoxBlur(
					(int)GetWidth(), GetStripeHeightPhysical(a_stripe),
					(int)GetGaussBlurRadius(), ms_useGPU && !m_forceGPUOff));
				p_blur->SetAllowFallback(m_allowCPUFallback);

				cairo_t* cr = cairo_create(l_surface);

				RenderBackgrounds(l_rowStart, l_rowEnd, l_baseY, cr);

				// shadow effect:
				if(!m_cancelRenderingImmediately)
				{
					RenderStripeBlocks(a_stripe, false, true, p_blur->GetContext());

					// important when running in CPU fallback mode only:
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetGaussColor()));
					
					if(!p_blur->Paint(cr) && p_blur->IsFallbackAllowed())
					{
						// retry once.

						RenderStripeBlocks(a_stripe, false, true, p_blur->GetContext());

						// important when running in CPU fallback mode only:
						cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetGaussColor()));

						p_blur->Paint(cr);
					}
				}

				cairo_destroy(cr);
			}

			if((m_partial & NRP_RENDER_GAUSS_BLOCKS) != 0 && (m_partial & NRP_RENDER_GAUSS_SHADOW) == 0 && !m_cancelRenderingImmediately)
			{
				// render blocks in gaussian color
				RenderStripeBlocks(a_stripe, false, true);
			}
			else if((m_partial & NRP_RENDER_BLOCKS) != 0 && !m_cancelRenderingImmediately)
			{
				// normal mode
				RenderStripeBlocks(a_stripe, false, false);
			}
		}
		else if(m_hasBlocks && (m_partial & NRP_RENDER_BLOCKS) != 0 && !m_cancelRenderingImmediately)
		{
			RenderStripeBlocks(a_stripe, true, false);
		}

		if((m_partial & NRP_RENDER_TEXT) != 0 && !m_cancelRenderingImmediately)
		{
			RenderText(GetTextColor(), NULL, GetHyperLinkColor(),
				l_rowStart, 0, l_rowEnd, m_nfo->GetGridWidth() - 1,
				l_surface, 0, l_baseY);
		}
	}
}


void CNFORenderer::RenderBackgrounds(size_t a_rowStart, size_t a_rowEnd, double a_yBase, cairo_t* cr) const
{
	cairo_save(cr);

	if(GetBackColor().A > 0)
	{
		cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(GetBackColor()));
		cairo_paint(cr);
	}

	if(m_nfo->HasColorMap())
	{
		double dbw = static_cast<double>(GetBlockWidth());
		double dbh = static_cast<double>(GetBlockHeight());

		for(size_t row = (a_rowStart == -1 ? 0 : a_rowStart); row <= a_rowEnd; row++)
		{
			std::vector<size_t> l_columns;
			std::vector<uint32_t> l_colors;

			if(!m_nfo->GetColorMap()->GetLineBackgrounds(row, GetBackColor().AsWord(), m_gridData->GetCols(), l_columns, l_colors))
				continue;

			_ASSERT(l_colors.size() == l_columns.size());
			_ASSERT(l_columns.size() > 0);

			size_t col = 0;

			for(size_t section = 0; section < l_colors.size(); section++)
			{
				size_t col_to = col + l_columns[section];
				double x_from = col * dbw, x_to = col_to * dbw;

				cairo_set_source_rgb(cr, S_COLOR_T_CAIRO(S_COLOR_T(l_colors[section])));

				cairo_rectangle(cr, m_padding + x_from, a_yBase + m_padding + dbh * row, x_to - x_from, dbh);

				cairo_fill(cr);

				col = col_to;
			}
		}
	}

	cairo_restore(cr);
}


/************************************************************************/
/* RENDER BLOCKS                                                        */
/************************************************************************/

void CNFORenderer::RenderStripeBlocks(size_t a_stripe, bool a_opaqueBg, bool a_gaussStep, cairo_t* a_context) const
{
	cairo_t* l_context = (a_context ? a_context : cairo_create(GetStripeSurface(a_stripe)));

	RenderBlocks(a_opaqueBg, a_gaussStep, l_context,
		(m_numStripes > 1 ? a_stripe * m_linesPerStripe - GetStripeExtraLinesTop(a_stripe) : (size_t)-1),
		a_stripe * m_linesPerStripe + m_linesPerStripe + GetStripeExtraLinesBottom(a_stripe),
		// see comment in RenderStripe():
		0, (a_stripe == 0 ? 0 : -m_padding - (double)a_stripe * m_stripeHeight) + GetStripeHeightExtraTop(a_stripe));

	if(!a_context)
	{
		cairo_destroy(l_context);
	}
}

void CNFORenderer::RenderBlocks(bool a_opaqueBg, bool a_gaussStep, cairo_t* a_context,
	size_t a_rowStart, size_t a_rowEnd, double a_xBase, double a_yBase) const
{
	double l_off_x = m_padding + a_xBase, l_off_y = m_padding + a_yBase;

	size_t l_rowStart = 0, l_rowEnd = m_gridData->GetRows() - 1;
	if(a_rowStart != (size_t)-1)
	{
		l_rowStart = std::max(a_rowStart, l_rowStart);
		l_rowEnd = std::min(a_rowEnd, l_rowEnd);
	}

	cairo_t * const cr = a_context;
	cairo_save(cr);

	if(a_opaqueBg)
	{
		RenderBackgrounds(l_rowStart, l_rowEnd, a_yBase, cr);
	}

	cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);

	bool l_hasColorMap = m_nfo->HasColorMap();
	int l_oldAlpha = 0;
	uint32_t l_oldColor = 0;
	bool l_first = true;

	// micro optimization
	const double bwd = static_cast<double>(GetBlockWidth());
	const double bhd = static_cast<double>(GetBlockHeight());
	const double bwd05 = bwd * 0.5;
	const double bhd05 = bhd * 0.5;

	for(size_t row = l_rowStart; row <= l_rowEnd; row++)
	{
		if(m_cancelRenderingImmediately)
		{
			break;
		}

		for(size_t col = 0; col < m_gridData->GetCols(); col++)
		{
			const CRenderGridBlock *l_block = &(*m_gridData)[row][col];

			if(l_block->shape == RGS_NO_BLOCK ||
				l_block->shape == RGS_WHITESPACE ||
				l_block->shape == RGS_WHITESPACE_IN_TEXT)
			{
				continue;
			}

			S_COLOR_T l_drawingColor = a_gaussStep ? GetGaussColor() : GetArtColor();

			if(l_hasColorMap)
			{
				uint32_t clr;

				m_nfo->GetColorMap()->GetForegroundColor(row, col, l_drawingColor.AsWord(), clr);

				l_drawingColor = S_COLOR_T(clr);
			}

			if(l_first 
				|| (l_block->alpha != l_oldAlpha)  // R,G,B never change during the loop (unless there's a colormap)
				|| (l_hasColorMap && l_drawingColor != l_oldColor)
			) {
				cairo_fill(cr); // complete previous drawing operation(s)

				cairo_set_source_rgba(cr, S_COLOR_T_CAIRO(l_drawingColor), (l_block->alpha / 255.0) * (l_drawingColor.A / 255.0));

				// known issue: Alpha from GetGauss/ArtColor is discarded if there's a colormap.

				l_oldAlpha = l_block->alpha;
				l_oldColor = l_drawingColor.AsWord();
				l_first = false;
			}

			double l_pos_x = col * bwd, l_pos_y = row * bhd, l_width = bwd, l_height = bhd;

			switch(l_block->shape)
			{
			case RGS_BLOCK_LOWER_HALF:
				l_pos_y += bhd05;
			case RGS_BLOCK_UPPER_HALF:
				l_height = bhd05;
				break;
			case RGS_BLOCK_RIGHT_HALF:
				l_pos_x += bwd05;
			case RGS_BLOCK_LEFT_HALF:
				l_width = bwd05;
				break;
			case RGS_BLACK_SQUARE:
				l_width = l_height = bwd * 0.75;
				l_pos_y += bhd05 - l_height * 0.5;
				l_pos_x += bwd05 - l_width * 0.5;
				break;
			case RGS_BLACK_SMALL_SQUARE:
				l_width = l_height = bwd05;
				l_pos_y += bhd05 - l_height * 0.5;
				l_pos_x += bwd05 - l_width * 0.5;
				break;
			}

			cairo_rectangle(cr, l_off_x + l_pos_x, l_off_y + l_pos_y, l_width, l_height);
		}
	}

	cairo_fill(cr); // complete pending drawing operation(s)

	cairo_restore(cr);
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

static inline void _SetUpHyperLinkUnderlining(const CNFORenderer* r, cairo_t* cr)
{
	if(r->GetHilightHyperLinks() && r->GetUnderlineHyperLinks())
	{
		cairo_set_line_width(cr, 1);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE); // looks better
	}
}

static inline void _SetUpDrawingTools(const CNFORenderer* r, cairo_surface_t* a_surface, cairo_t** pcr, cairo_font_options_t** pcfo)
{
	cairo_t* cr = cairo_create(a_surface);

	cairo_font_options_t *cfo = cairo_font_options_create();

	cairo_font_options_set_antialias(cfo, (r->GetFontAntiAlias() ? CAIRO_ANTIALIAS_SUBPIXEL : CAIRO_ANTIALIAS_NONE));
	cairo_font_options_set_hint_style(cfo, (r->IsClassicMode() ? CAIRO_HINT_STYLE_DEFAULT : CAIRO_HINT_STYLE_NONE));
	cairo_font_options_set_hint_metrics(cfo, (r->IsClassicMode() ? CAIRO_HINT_METRICS_ON : CAIRO_HINT_METRICS_OFF));

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
		cairo_set_font_size(cr, static_cast<double>(r->GetFontSize()));
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

void CNFORenderer::PreRenderText()
{
	if(m_classic || m_fontSize > 0)
	{
		return;
	}

	// create a dummy surface so we can measure things:
	cairo_surface_t *l_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 30);
	cairo_t* cr;
	cairo_font_options_t* l_fontOptions;

	_SetUpDrawingTools(this, l_surface, &cr, &l_fontOptions);

	double l_fontSize = static_cast<double>(GetBlockWidth());
	bool l_broken = false, l_foundText = false;

	std::set<wchar_t> l_checkChars;

	for(size_t row = 0; row < m_gridData->GetRows() && !l_broken; row++)
	{
		for(size_t col = 0; col < m_gridData->GetCols() && !l_broken; col++)
		{
			CRenderGridBlock *l_block = &(*m_gridData)[row][col];

			if(l_block->shape == RGS_NO_BLOCK)
			{
				l_checkChars.insert(m_nfo->GetGridChar(row, col));
			}
		}
	}

	if(l_checkChars.size() > 0)
	{
		// add some generic "big" chars for NFOs that e.g.
		// contain nothing but dots or middots:
		l_checkChars.insert(L'W');
		l_checkChars.insert(L'M');

		// calculate font size that fits into blocks of the given size:
		do
		{
			cairo_set_font_size(cr, l_fontSize + 1);

			for(std::set<wchar_t>::const_iterator it = l_checkChars.begin(); it != l_checkChars.end(); it++)
			{
				cairo_text_extents_t l_extents = {0};

				// measure the inked area of this glyph (char):
				cairo_scaled_font_t *l_csf = cairo_get_scaled_font(cr);
				cairo_glyph_t *l_glyphs = NULL;
				int l_numGlyphs = 0;

				const std::string utf8 = m_nfo->GetGridCharUtf8(*it);

				if(cairo_scaled_font_text_to_glyphs(l_csf, 0, 0, utf8.c_str(), -1,
					&l_glyphs, &l_numGlyphs, NULL, NULL, NULL) == CAIRO_STATUS_SUCCESS)
				{
					cairo_scaled_font_glyph_extents(l_csf, l_glyphs, l_numGlyphs, &l_extents);
					cairo_glyph_free(l_glyphs);

					// find char that covers the largest area...
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
	}

	m_fontSize = l_fontSize + 1;

	_FinalizeDrawingTools(&cr, &l_fontOptions);
}

void CNFORenderer::RenderText(const S_COLOR_T& a_textColor, const S_COLOR_T* a_backColor,
							  const S_COLOR_T& a_hyperLinkColor,
							  size_t a_rowStart, size_t a_colStart, size_t a_rowEnd, size_t a_colEnd,
							  cairo_surface_t* a_surface, double a_xBase, double a_yBase) const
{
	double l_off_x = a_xBase + m_padding, l_off_y = a_yBase + m_padding;

	_FixUpRowColStartEnd(a_rowStart, a_colStart, a_rowEnd, a_colEnd);

	cairo_t* cr;
	cairo_font_options_t* l_fontOptions;
	_SetUpDrawingTools(this, a_surface, &cr, &l_fontOptions);

	_SetUpHyperLinkUnderlining(this, cr);

	// m_fontSize has been calculated by PreRenderText:
	cairo_set_font_size(cr, m_fontSize);

	if(m_fontSize < 4)
	{
		// disable anti-alias to avoid colorful artifacts in low zoom levels:
		cairo_font_options_set_antialias(l_fontOptions, CAIRO_ANTIALIAS_NONE);
		cairo_set_font_options(cr, l_fontOptions);
	}

	// get general font info to vertically center chars into the blocks:
	cairo_font_extents_t l_font_extents;
	cairo_font_extents(cr, &l_font_extents);

	// determine ranges to draw (important for selection/highlights):
	size_t l_rowStart = 0, l_rowEnd = m_gridData->GetRows() - 1;
	if(a_rowStart != (size_t)-1)
	{
		l_rowStart = std::max(a_rowStart, l_rowStart);
		l_rowEnd = std::min(a_rowEnd, l_rowEnd);
	}

	// set main text color:
	cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_textColor));

	// get a scaled font reference from cr.
	// remember this reference will become invalid as soon as cr does.
	cairo_scaled_font_t *l_csf = cairo_get_scaled_font(cr);

	std::string l_utfBuf;
	l_utfBuf.reserve(m_nfo->GetGridWidth());

	for(size_t row = l_rowStart; row <= l_rowEnd; row++)
	{
		size_t l_firstCol = (size_t)-1;

		if(m_cancelRenderingImmediately)
		{
			break;
		}

		// collect an UTF-8 buffer of each line:
		for(size_t col = 0; col < m_gridData->GetCols(); col++)
		{
			const CRenderGridBlock *l_block = &(*m_gridData)[row][col];

			if(l_block->shape != RGS_NO_BLOCK && l_block->shape != RGS_WHITESPACE_IN_TEXT)
			{
				if(l_firstCol != (size_t)-1)
					l_utfBuf += ' '; // add whitespace between non-whitespace chars that are skipped

				continue;
			}

			if(a_rowStart != (size_t)-1)
			{
				if(row == a_rowStart && col < a_colStart)
					continue;
				else if(row == a_rowEnd && col > a_colEnd)
					break;
			}

			l_utfBuf += m_nfo->GetGridCharUtf8(row, col);

			if(col < l_firstCol) l_firstCol = col;
		}

		if(l_firstCol != (size_t)-1)
		{
			cairo_glyph_t *l_glyphs = NULL, *l_pg;
			int l_numGlyphs = -1, i;

			// remove trailing whitespace added above:
			CUtil::StrTrimRight(l_utfBuf);

			// we call this to get the glyph indexes from the backend.
			// the actual position/coordinates of the glyphs will be calculated below.
			cairo_scaled_font_text_to_glyphs(l_csf,
				0,
				l_off_y + row * GetBlockHeight() + (l_font_extents.ascent + GetBlockHeight()) / 2.0 - 2,
				l_utfBuf.c_str(), (int)l_utfBuf.size(), &l_glyphs, &l_numGlyphs, NULL, NULL, NULL);

			// put each char/glyph into its cell in the grid:
			for(l_pg = l_glyphs, i = 0; i < l_numGlyphs; i++, l_pg++)
			{
				l_pg->x = l_off_x + (l_firstCol + i) * GetBlockWidth();
			}

			// draw background for highlights/selection etc:
			if(a_backColor)
			{
				cairo_save(cr);
				cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(*a_backColor));
				cairo_rectangle(cr, l_off_x + l_firstCol * GetBlockWidth(), l_off_y + row * GetBlockHeight(),
					static_cast<double>(GetBlockWidth() * l_numGlyphs), static_cast<double>(GetBlockHeight()));
				cairo_fill(cr);
				cairo_restore(cr);
			}

			// check for hyperlinks...
			const std::vector<const CNFOHyperLink*> l_links = m_nfo->GetLinksForLine(row);

			if(l_links.size() == 0 || !GetHilightHyperLinks())
			{
				// ... no hyperlinks, draw the entire line in one go:
				cairo_show_glyphs(cr, l_glyphs, l_numGlyphs);
			}
			else if(a_rowStart == (size_t)-1 || !a_backColor)
			{
				size_t l_nextCol = l_firstCol;

				cairo_save(cr);

				// go through each hyperlink and hilight them as requested:
				for(std::vector<const CNFOHyperLink*>::const_iterator it = l_links.begin(); it != l_links.end(); it++)
				{
					const CNFOHyperLink* l_link = *it;

					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_textColor));

					cairo_show_glyphs(cr, l_glyphs + l_nextCol - l_firstCol,
						static_cast<int>(l_link->GetColStart() - l_nextCol));

					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_hyperLinkColor));

					if(GetUnderlineHyperLinks())
					{
						cairo_move_to(cr, l_off_x + l_link->GetColStart() * GetBlockWidth(), l_off_y + (row + 1) * GetBlockHeight());
						cairo_rel_line_to(cr, static_cast<double>(l_link->GetLength() * GetBlockWidth()), 0);
						cairo_stroke(cr);
					}

					cairo_show_glyphs(cr, l_glyphs + l_link->GetColStart() - l_firstCol, (int)l_link->GetLength());

					l_nextCol = l_link->GetColEnd() + 1;
				}

				cairo_restore(cr);

				// draw remaining text following the last link:
				if(l_nextCol - l_firstCol < (size_t)l_numGlyphs)
				{
					cairo_show_glyphs(cr, l_glyphs + l_nextCol - l_firstCol,
						static_cast<int>(l_numGlyphs + l_firstCol - l_nextCol));
				}
			}
			else
			{
				// this is rather slow, but required to get lines with hyperlinks and selection in them right:
				size_t l_showStart = 0;
				int l_showLen = 0;
				bool l_inLink;
				size_t l_linkRest = 0;

				cairo_save(cr);

				for(int p = 0; p <= l_numGlyphs; p++) /* excess run: draw remaining stuff */
				{
					bool l_linkAtPos = (l_linkRest > 0);

					if(p == 0 || (!l_linkAtPos && p < l_numGlyphs))
					{
						for(std::vector<const CNFOHyperLink*>::const_iterator it = l_links.begin(); it != l_links.end(); it++)
						{
							if(p + l_firstCol >= (*it)->GetColStart() && p + l_firstCol <= (*it)->GetColEnd())
							{
								l_linkAtPos = true;
								l_linkRest = (*it)->GetLength() - (p + l_firstCol - (*it)->GetColStart()) - 1;
								break;
							}
						}

						if(p == 0) l_inLink = l_linkAtPos;
					}
					else
						l_linkRest--;

					if(l_linkAtPos == l_inLink)
					{
						l_showLen++;
					}

					if((l_linkAtPos != l_inLink || p == l_numGlyphs) && l_showLen > 0)
					{
						if(p == l_numGlyphs && l_linkAtPos == l_inLink)
						{
							l_showLen--;
						}

						if(l_inLink) // "buffer" contains hyperlink text
						{
							cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_hyperLinkColor));

							if(GetUnderlineHyperLinks())
							{
								cairo_move_to(cr, l_off_x + (l_showStart + l_firstCol) * GetBlockWidth(), l_off_y + (row + 1) * GetBlockHeight());
								cairo_rel_line_to(cr, static_cast<double>(l_showLen * GetBlockWidth()), 0);
								cairo_stroke(cr);
							}
						}
						else
						{
							cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(a_textColor));
						}

						cairo_show_glyphs(cr, l_glyphs + l_showStart, l_showLen);

						if(p < l_numGlyphs)
						{
							l_showStart = (size_t)p;
							l_showLen = 1;
						}
					}

					l_inLink = l_linkAtPos;
				}

				cairo_restore(cr);
			}

			// free glyph array (allocated by cairo_scaled_font_text_to_glyphs):
			cairo_glyph_free(l_glyphs);

			l_utfBuf = "";
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


void CNFORenderer::RenderClassic(const S_COLOR_T& a_textColor, const S_COLOR_T* a_backColor,
								 const S_COLOR_T& a_hyperLinkColor, bool a_backBlocks,
								 size_t a_rowStart, size_t a_colStart, size_t a_rowEnd, size_t a_colEnd,
								 cairo_surface_t* a_surface, double a_xBase, double a_yBase) const
{
	double l_off_x = a_xBase + m_padding, l_off_y = a_yBase + m_padding;

	_FixUpRowColStartEnd(a_rowStart, a_colStart, a_rowEnd, a_colEnd);

	cairo_t* cr;
	cairo_font_options_t* l_fontOptions;
	_SetUpDrawingTools(this, a_surface, &cr, &l_fontOptions);

	_SetUpHyperLinkUnderlining(this, cr);

	cairo_font_extents_t l_font_extents;
	cairo_font_extents(cr, &l_font_extents);

	size_t l_rowStart = 0, l_rowEnd = m_gridData->GetRows() - 1;
	if(a_rowStart != (size_t)-1)
	{
		l_rowStart = std::max(a_rowStart, l_rowStart);
		l_rowEnd = std::min(a_rowEnd, l_rowEnd);
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

		if(m_cancelRenderingImmediately)
		{
			break;
		}

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

				if(l_block->shape == RGS_NO_BLOCK)
				{
					if(GetHilightHyperLinks() && m_nfo->GetLink(row, col) != NULL)
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

			if(l_curType != _BT_UNDEF && !l_utfBuf.empty() &&
				!(l_curType == BT_BLOCK && (m_partial & NRP_RENDER_BLOCKS) == 0) &&
				!((l_curType == BT_TEXT || l_curType == BT_LINK) && (m_partial & NRP_RENDER_TEXT) == 0))
			{
				// draw buffer:
				size_t l_len = (col == m_gridData->GetCols() ? utf8_strlen(l_utfBuf.c_str(), -1) :
					(col - l_bufStart));

				// draw char background for highlights/selection etc:
				if(a_backColor && (l_curType != BT_BLOCK || a_backBlocks))
				{
					cairo_save(cr);
					cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(*a_backColor));
					cairo_rectangle(cr,
						static_cast<double>(l_off_x + l_bufStart * GetBlockWidth()),
						static_cast<double>(row * GetBlockHeight() + l_off_y),
						static_cast<double>(GetBlockWidth() * l_len),
						static_cast<double>(GetBlockHeight()));
					cairo_fill(cr);
					cairo_restore(cr);
				}

				if(l_curType == BT_LINK && GetUnderlineHyperLinks())
				{
					cairo_move_to(cr,
						static_cast<double>(l_off_x + l_bufStart * GetBlockWidth()),
						static_cast<double>(l_off_y + (row + 1) * GetBlockHeight()));
					cairo_rel_line_to(cr, static_cast<double>(GetBlockWidth() * l_len), 0);
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


void CNFORenderer::SetZoom(unsigned int a_percent)
{
	m_zoomFactor = a_percent / 100.0f;

	ClearStripes();

	m_fontSize = -1;
	m_rendered = false;
}


size_t CNFORenderer::GetWidth() const
{
	if(!m_nfo) return 0;

	return m_nfo->GetGridWidth() * GetBlockWidth() + m_padding * 2;
}


size_t CNFORenderer::GetHeight() const
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
		if(ns.uBlockWidth > 0 && ns.uBlockWidth < 200 &&
			ns.uBlockHeight > 0 && ns.uBlockHeight < 200)
		{
			SetBlockSize(ns.uBlockWidth, ns.uBlockHeight);
		}

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
	SetWrapLines(ns.bWrapLines);
	SetFontFace(ns.sFontFace);

	if(!m_rendered) // stuff has changed
	{
		ClearStripes();
	}
}


void CNFORenderer::ClearStripes()
{
	StopPreRendering();

	m_stripes.clear();
}


CNFORenderer::~CNFORenderer()
{
	ClearStripes();

	delete m_gridData;
}


/************************************************************************/
/* CNFORenderSettings serialization                                     */
/************************************************************************/

std::wstring CNFORenderSettings::Serialize() const
{
	std::wstringstream l_ss;
	l_ss << "{\n\t";

	l_ss << "blw: " << uBlockWidth << ";\n\t";
	l_ss << "blh: " << uBlockHeight << ";\n\t";
	l_ss << "fos: " << uFontSize << ";\n\t";

	l_ss << "cba: " << cBackColor.AsHex(true) << ";\n\t";
	l_ss << "cte: " << cTextColor.AsHex(true) << ";\n\t";
	l_ss << "car: " << cArtColor.AsHex(true) << ";\n\t";

	l_ss << "fof: '" << sFontFace << "';\n\t";
	l_ss << "foa: " << (bFontAntiAlias ? 1 : 0) << ";\n\t";
	l_ss << "wll: " << (bWrapLines ? 1 : 0) << ";\n\t";

	l_ss << "cga: " << cGaussColor.AsHex(true) << ";\n\t";
	l_ss << "gas: " << (bGaussShadow ? 1 : 0) << ";\n\t";
	l_ss << "gar: " << uGaussBlurRadius << ";\n\t";

	l_ss << "hhl: " << (bHilightHyperlinks ? 1 : 0) << ";\n\t";
	l_ss << "chl: " << cHyperlinkColor.AsHex(true) << ";\n\t";
	l_ss << "hul: " << (bUnderlineHyperlinks ? 1 : 0) << ";\n";

	l_ss << "}";
	return l_ss.str();
}


bool CNFORenderSettings::UnSerialize(std::wstring a_str, bool a_classic)
{
	CUtil::StrTrim(a_str);

	if(a_str.size() < 3 || a_str[0] != L'{' || a_str[a_str.size() - 1] != L'}')
	{
		return false;
	}

	a_str.erase(0, 1);
	a_str.erase(a_str.size() - 1);
	CUtil::StrTrim(a_str);

	int l_numExtracted = 0;
	CNFORenderSettings l_tmpSets;

	std::wstring::size_type l_colonPos = a_str.find(L':'), l_posBeforeColon = 0;
	while(l_colonPos != std::wstring::npos)
	{
		std::wstring l_key = a_str.substr(l_posBeforeColon, l_colonPos - l_posBeforeColon);

		std::wstring::size_type l_pos = l_colonPos + 1;
		while(l_pos < a_str.size() && iswspace(a_str[l_pos])) l_pos++;

		if(l_pos >= a_str.size() - 1) break;

		std::wstring::size_type l_valEndPos;

		if(a_str[l_pos] == L'\'')
		{
			l_valEndPos = a_str.find(L'\'', l_pos + 1);
		}
		else
		{
			l_valEndPos = a_str.find(L';', l_pos);
		}

		if(l_valEndPos == std::wstring::npos)
		{
			break;
		}

		std::wstring l_val = a_str.substr(l_pos, l_valEndPos - l_pos);

		if(a_str[l_pos] == L'\'')
		{
			l_val.erase(0, 1);

			l_valEndPos = a_str.find(L';', l_valEndPos);

			if(l_valEndPos == std::wstring::npos)
			{
				l_valEndPos = a_str.size();
			}
		}

		l_numExtracted++;

		if(l_key == L"blw")
			l_tmpSets.uBlockWidth = static_cast<size_t>(wcstoul(l_val.c_str(), NULL, 10));
		else if(l_key == L"blh")
			l_tmpSets.uBlockHeight = static_cast<size_t>(wcstoul(l_val.c_str(), NULL, 10));
		else if(l_key == L"fos")
			l_tmpSets.uFontSize = static_cast<size_t>(wcstoul(l_val.c_str(), NULL, 10));
		else if(l_key == L"cba")
			CNFORenderer::ParseColor(l_val.c_str(), &l_tmpSets.cBackColor);
		else if(l_key == L"cte")
			CNFORenderer::ParseColor(l_val.c_str(), &l_tmpSets.cTextColor);
		else if(l_key == L"car")
			CNFORenderer::ParseColor(l_val.c_str(), &l_tmpSets.cArtColor);
		else if(l_key == L"fof" && l_val.size() <= LF_FACESIZE)
		{
#ifdef _UNICODE
			wcscpy_s(l_tmpSets.sFontFace, LF_FACESIZE + 1, l_val.c_str());
#else
			const std::string l_sff = CUtil::FromWideStr(l_val, CP_UTF8);

			if(l_sff.size() <= LF_FACESIZE)
			{
				strcpy_s(l_tmpSets.sFontFace, LF_FACESIZE + 1, l_sff.c_str());
			}
#endif
		}
		else if(l_key == L"foa")
			l_tmpSets.bFontAntiAlias = (wcstol(l_val.c_str(), NULL, 10) != 0);
		else if(l_key == L"wll")
			l_tmpSets.bWrapLines = (wcstol(l_val.c_str(), NULL, 10) != 0);
		else if(l_key == L"cga")
			CNFORenderer::ParseColor(l_val.c_str(), &l_tmpSets.cGaussColor);
		else if(l_key == L"gas")
			l_tmpSets.bGaussShadow = (wcstol(l_val.c_str(), NULL, 10) != 0);
		else if(l_key == L"gar")
			l_tmpSets.uGaussBlurRadius = static_cast<unsigned int>(wcstoul(l_val.c_str(), NULL, 10));
		else if(l_key == L"hhl")
			l_tmpSets.bHilightHyperlinks = (wcstol(l_val.c_str(), NULL, 10) != 0);
		else if(l_key == L"chl")
			CNFORenderer::ParseColor(l_val.c_str(), &l_tmpSets.cHyperlinkColor);
		else if(l_key == L"hul")
			l_tmpSets.bUnderlineHyperlinks = (wcstol(l_val.c_str(), NULL, 10) != 0);
		else
			l_numExtracted--;

		l_posBeforeColon = l_valEndPos + 1;
		l_colonPos = a_str.find(L':', l_posBeforeColon);
	}

	if(l_numExtracted > 0)
	{
		// we use this to validate the raw data from a_str:
		CNFORenderer l_dummyRenderer(a_classic);
		l_dummyRenderer.InjectSettings(l_tmpSets);

		*this = l_dummyRenderer.GetSettings();

		return true;
	}

	return false;
}


void CNFORenderer::WaitForPreRender()
{
	shared_ptr<std::thread> l_renderThread(m_preRenderThread);

	if(l_renderThread && l_renderThread->joinable())
	{
		l_renderThread->join();

		m_preRenderThread.reset();
	}

	m_cancelRenderingImmediately = false;
}


void CNFORenderer::StopPreRendering(bool a_cancel)
{
	if(!m_preRenderThread || m_stopPreRendering)
	{
		return;
	}

	m_stopPreRendering = true;
	m_cancelRenderingImmediately = a_cancel;

	WaitForPreRender();
}


void CNFORenderer::PreRender()
{
	if(m_preRenderThread || m_numStripes < 2)
	{
		return;
	}

	m_stopPreRendering = false;

	m_preRenderThread = shared_ptr<std::thread>(new std::thread(
		std::bind(&CNFORenderer::PreRenderThreadProc, this)));
}


void CNFORenderer::PreRenderThreadProc()
{
	for(size_t l_stripe = 0; l_stripe < m_numStripes && !m_stopPreRendering; ++l_stripe)
	{
		m_stripesLock.lock();

		if(!m_stripes[l_stripe])
		{
			m_stripes[l_stripe] = PCairoSurface(new _CCairoSurface(
				cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
				(int)GetWidth(),
				GetStripeHeightPhysical(l_stripe))
			));

			m_stripesLock.unlock();

			m_preRenderingStripe.store(l_stripe);

			m_cancelRenderingImmediately = false;
			RenderStripe(l_stripe);
		}
		else
		{
			m_stripesLock.unlock();
		}
	}

	m_preRenderingStripe.store((size_t)-1);
}


bool CNFORenderer::ms_useGPU = true;
