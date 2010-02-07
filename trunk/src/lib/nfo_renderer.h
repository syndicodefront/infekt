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
} CRenderGridBlock;


typedef struct _s_color_t
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A; /* 0 = invisible, 255 = opaque */

	_s_color_t() { R = G = B = A = 255; }
	_s_color_t(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) { R = r; G = g;  B = b; A = a; }

	bool operator==(const _s_color_t &o) const { return (o.R == R && o.G == G && o.B == B && o.A == A); }
	bool operator!=(const _s_color_t &o) const { return !(*this == o); }
} S_COLOR_T;

#define _S_COLOR(R, G, B, A) _s_color_t(R, G, B, A)
#define _S_COLOR_RGB(R, G, B) _s_color_t(R, G, B, 0xFF)

#define S_COLOR_T_CAIRO(CLR) CLR.R / 255.0, CLR.G / 255.0, CLR.B / 255.0


class CNFORenderer
{
protected:
	// settings:
	size_t m_blockHeight, m_blockWidth;
	S_COLOR_T m_backColor, m_textColor, m_artColor, m_gaussColor;
	bool m_gaussShadow;
	int m_gaussBlurRadius;

	// NFO data:
	PNFOData m_nfo;
	TwoDimVector<CRenderGridBlock> *m_gridData;
	cairo_surface_t *m_imgSurface;

	// internal state data:
	bool m_rendered;
	int m_padding;

	// internal calls:
	bool CalculateGrid();
	void RenderBlocks(bool a_opaqueBg, bool a_gaussStep);
	void RenderText();
public:
	CNFORenderer();
	virtual ~CNFORenderer();

	// mainly important methods:
	bool AssignNFO(const PNFOData& a_nfo);
	bool DrawToSurface(cairo_surface_t *a_surface, int dest_x, int dest_y,
		int source_x, int source_y, int width, int height);
	// you should not call this directly without a good reason, prefer DrawToSurface:
	bool Render();

	// return the calculated image dimensions:
	size_t GetWidth();
	size_t GetHeight();

	// color setters & getters:
	void SetBackColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_backColor == nc); m_backColor = nc; }
	void SetTextColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_textColor == nc); m_textColor = nc; }
	void SetArtColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_artColor == nc); m_artColor = nc; }
	void SetGaussColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_gaussColor == nc); m_gaussColor = nc; }
	S_COLOR_T GetBackColor() { return m_backColor; }
	S_COLOR_T GetTextColor() { return m_textColor; }
	S_COLOR_T GetArtColor() { return m_artColor; }
	S_COLOR_T GetGaussColor() { return m_gaussColor; }

	// various other setters & getters:
	void SetEnableGaussShadow(bool nb) { m_rendered = m_rendered && (m_gaussShadow == nb); m_gaussShadow = nb; }
	bool GetEnableGaussShadow() { return m_gaussShadow; }
	void SetGaussBlurRadius(int r) {
		if(r > 0) { m_rendered = m_rendered && (m_gaussBlurRadius == r); m_gaussBlurRadius = r; }
		m_padding = m_gaussBlurRadius; // space for blur/shadow effect near the edges
	}
	int GetGaussBlurRadius() { return m_gaussBlurRadius; }

	void SetBlockSize(size_t a_width, size_t a_height) { m_rendered = m_rendered &&
		a_width == m_blockWidth && a_height == m_blockHeight; m_blockWidth = a_width; m_blockHeight = a_height; }
	size_t GetBlockWidth() { return m_blockWidth; }
	size_t GetBlockHeight() { return m_blockHeight; }

	// static color helper methods for anyone to use:
	static bool ParseColor(const char* a_str, S_COLOR_T* ar);
	static bool ParseColor(const wchar_t* a_str, S_COLOR_T* ar);
};


#endif /* !_NFO_RENDERER_H */
