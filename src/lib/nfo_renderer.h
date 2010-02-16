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
	RGS_FULL_BLOCK,
	RGS_BLOCK_LOWER_HALF,
	RGS_BLOCK_UPPER_HALF,
	RGS_BLOCK_LEFT_HALF,
	RGS_BLOCK_RIGHT_HALF,
	RGS_BLACK_SQUARE,
	RGS_BLACK_SMALL_SQUARE,
	RGS_WHITESPACE,
	RGS_WHITESPACE_IN_TEXT,

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

	_s_color_t Invert() const { return _s_color_t(255 - R, 255 - G, 255 - B, A); }
} S_COLOR_T;

#define _S_COLOR(R, G, B, A) _s_color_t(R, G, B, A)
#define _S_COLOR_RGB(R, G, B) _s_color_t(R, G, B, 0xFF)

#define S_COLOR_T_CAIRO(CLR) (CLR).R / 255.0, (CLR).G / 255.0, (CLR).B / 255.0
#define S_COLOR_T_CAIRO_A(CLR) (CLR).R / 255.0, (CLR).G / 255.0, (CLR).B / 255.0, (CLR).A / 255.0


class CNFORenderSettings
{
public:
	// main settings:
	size_t uBlockHeight, uBlockWidth;
	S_COLOR_T cBackColor, cTextColor, cArtColor;

	// blur effect settings:
	S_COLOR_T cGaussColor;
	bool bGaussShadow;
	unsigned int uGaussBlurRadius;

	// hyperlink settings:
	bool bHilightHyperLinks;
	S_COLOR_T cHyperlinkColor;
	bool bUnderlineHyperLinks;

// :TODO: Add methods for serialization
};


class CNFORenderer
{
protected:
	CNFORenderSettings m_settings;

	// NFO data:
	PNFOData m_nfo;
	TwoDimVector<CRenderGridBlock> *m_gridData;
	cairo_surface_t *m_imgSurface;

	// internal state data:
	bool m_rendered;
	int m_padding;
	double m_fontSize;

	// internal calls:
	bool CalculateGrid();
	void RenderBlocks(bool a_opaqueBg, bool a_gaussStep);
	void RenderText();
	void RenderText(const S_COLOR_T& a_textColor, const S_COLOR_T* a_backColor,
		const S_COLOR_T& a_hyperLinkColor,
		size_t a_rowStart, size_t a_colStart, size_t a_rowEnd, size_t a_colEnd,
		cairo_surface_t* a_surface, double a_xBase, double a_yBase);

	bool IsTextChar(size_t a_row, size_t a_col, bool a_allowWhiteSpace = false) const;
public:
	CNFORenderer();
	virtual ~CNFORenderer();

	// mainly important methods:
	virtual bool AssignNFO(const PNFOData& a_nfo);
	bool HasNfoData() const { return (m_nfo && m_nfo->HasData() ? true : false); }
	virtual bool DrawToSurface(cairo_surface_t *a_surface, int dest_x, int dest_y,
		int source_x, int source_y, int width, int height);
	// you should not call this directly without a good reason, prefer DrawToSurface:
	bool Render();

	// return the calculated image dimensions:
	size_t GetWidth();
	size_t GetHeight();

	// color setters & getters:
	void SetBackColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_settings.cBackColor == nc); m_settings.cBackColor = nc; }
	void SetTextColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_settings.cTextColor == nc); m_settings.cTextColor = nc; }
	void SetArtColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_settings.cArtColor == nc); m_settings.cArtColor = nc; }
	void SetGaussColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_settings.cGaussColor == nc); m_settings.cGaussColor = nc; }
	void SetHyperLinkColor(const S_COLOR_T& nc) { m_rendered = m_rendered && (m_settings.cHyperlinkColor == nc); m_settings.cHyperlinkColor = nc; }
	S_COLOR_T GetBackColor() const { return m_settings.cBackColor; }
	S_COLOR_T GetTextColor() const { return m_settings.cTextColor; }
	S_COLOR_T GetArtColor() const { return m_settings.cArtColor; }
	S_COLOR_T GetGaussColor() const { return m_settings.cGaussColor; }
	S_COLOR_T GetHyperLinkColor() const { return m_settings.cHyperlinkColor; }

	// various other setters & getters:
	void SetEnableGaussShadow(bool nb) { m_rendered = m_rendered && (m_settings.bGaussShadow == nb); m_settings.bGaussShadow = nb; }
	bool GetEnableGaussShadow() const { return m_settings.bGaussShadow; }
	void SetGaussBlurRadius(unsigned int r) {
		m_rendered = m_rendered && (m_settings.uGaussBlurRadius == r); m_settings.uGaussBlurRadius = r;
		m_padding = m_settings.uGaussBlurRadius; // space for blur/shadow effect near the edges
	}
	unsigned int GetGaussBlurRadius() const { return m_settings.uGaussBlurRadius; }
	void SetHilightHyperLinks(bool nb) { m_rendered = m_rendered && (m_settings.bHilightHyperLinks == nb); m_settings.bHilightHyperLinks = nb; }
	bool GetHilightHyperLinks() const { return m_settings.bHilightHyperLinks; }
	void SetUnderlineHyperLinks(bool nb) { m_rendered = m_rendered && (m_settings.bUnderlineHyperLinks == nb); m_settings.bUnderlineHyperLinks = nb; }
	bool GetUnderlineHyperLinks() const { return m_settings.bUnderlineHyperLinks; }

	void SetBlockSize(size_t a_width, size_t a_height) { m_rendered = m_rendered &&
		a_width == m_settings.uBlockWidth && a_height == m_settings.uBlockHeight; m_settings.uBlockWidth = a_width; m_settings.uBlockHeight = a_height;
		if(!m_rendered) m_fontSize = -1; }
	size_t GetBlockWidth() const { return m_settings.uBlockWidth; }
	size_t GetBlockHeight() const { return m_settings.uBlockHeight; }

	// for quick switching between settings:
	const CNFORenderSettings GetSettings() const { return m_settings; }
	void InjectSettings(const CNFORenderSettings& ns) {
		m_settings = ns;
		m_rendered = false;
		m_fontSize = -1;
		SetGaussBlurRadius(m_settings.uGaussBlurRadius); // not nice...
	}

	// static color helper methods for anyone to use:
	static bool ParseColor(const char* a_str, S_COLOR_T* ar);
	static bool ParseColor(const wchar_t* a_str, S_COLOR_T* ar);
};


#endif /* !_NFO_RENDERER_H */
