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

#ifndef _NFO_RENDERER_H
#define _NFO_RENDERER_H

#include "nfo_data.h"

// http://www.alanwood.net/unicode/block_elements.html
// http://www.alanwood.net/demos/wgl4.html

typedef enum _render_grid_shape_t
{
	RGS_NO_BLOCK = 0,
	RGS_WHITESPACE,
	RGS_FULL_BLOCK,
	RGS_BLOCK_LOWER_HALF,
	RGS_BLOCK_UPPER_HALF,
	RGS_BLOCK_LEFT_HALF,
	RGS_BLOCK_RIGHT_HALF,
	RGS_BLACK_SQUARE,
	RGS_BLACK_SMALL_SQUARE,

	_RGS_MAX
} ERenderGridShape;


typedef struct _render_grid_block_t
{
	wchar_t charCode;
	ERenderGridShape shape;
	int alpha; /* 0 = invisible, 255 = opaque */
	char utf8[7];
} CRenderGridBlock;


typedef struct _s_color_t
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
} _S_COLOR_T;


class CNFORenderer
{
protected:
	size_t m_blockHeight, m_blockWidth;
	// colors...

	PNFOData m_nfo;
	TwoDimVector<CRenderGridBlock> *m_gridData;	

	bool m_rendered;
	cairo_surface_t *m_imgSurface;
	int m_padding;
	bool m_gaussShadow;
	int m_gaussBlurRadius;

	bool CalculateGrid();
	void RenderBlocks(bool a_opaqueBg, bool a_gaussStep);
	void RenderText();
public:
	CNFORenderer();
	virtual ~CNFORenderer();

	bool AssignNFO(const PNFOData& a_nfo);
	bool DrawToSurface(cairo_surface_t *a_surface, int dest_x, int dest_y,
		int source_x, int source_y, int width, int height);

	size_t GetWidth();
	size_t GetHeight();

	bool Render();
};


#endif /* !_NFO_RENDERER_H */
