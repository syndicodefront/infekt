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
	uint8_t alpha; /* 0 = invisible, 255 = opaque */
} CRenderGridBlock;


typedef struct _s_color_t
{
	uint8_t R, G, B;
	uint8_t A; /* 0 = invisible, 255 = opaque */

	_s_color_t() { R = G = B = A = 255; }
	_s_color_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) { R = r; G = g;  B = b; A = a; }
	_s_color_t(uint32_t wd) { A = (uint8_t)(wd & 0xFF); B = (uint8_t)((wd >> 8) & 0xFF); G = (uint8_t)((wd >> 16) & 0xFF); R = (uint8_t)((wd >> 24) & 0xFF); }

	bool operator==(const _s_color_t &o) const { return (o.R == R && o.G == G && o.B == B && o.A == A); }
	bool operator!=(const _s_color_t &o) const { return !(*this == o); }

	_s_color_t Invert() const { return _s_color_t(255 - R, 255 - G, 255 - B, A); }
	uint32_t AsWord() const { return (A) | (B << 8) | (G << 16) | (R << 24); }
} S_COLOR_T;

#define _S_COLOR(R, G, B, A) _s_color_t(R, G, B, A)
#define _S_COLOR_RGB(R, G, B) _s_color_t(R, G, B, 0xFF)

#define S_COLOR_T_CAIRO(CLR) (CLR).R / 255.0, (CLR).G / 255.0, (CLR).B / 255.0
#define S_COLOR_T_CAIRO_A(CLR) (CLR).R / 255.0, (CLR).G / 255.0, (CLR).B / 255.0, (CLR).A / 255.0


class CNFORenderSettings
{
/* NEVER add string or pointer members to this class without
	precautions, it's being copied like a struct etc. */
public:
	// main settings:
	size_t uBlockHeight, uBlockWidth;
	S_COLOR_T cBackColor, cTextColor, cArtColor;

	// blur effect settings:
	S_COLOR_T cGaussColor;
	bool bGaussShadow;
	unsigned int uGaussBlurRadius;

	// hyperlink settings:
	bool bHilightHyperlinks;
	S_COLOR_T cHyperlinkColor;
	bool bUnderlineHyperlinks;

// :TODO: Add methods for serialization
};


class CNFORenderer
{
protected:
	CNFORenderSettings m_settings;
	bool m_classic;

	// NFO data:
	PNFOData m_nfo;
	TwoDimVector<CRenderGridBlock> *m_gridData;
	cairo_surface_t *m_imgSurface;

	// internal state data:
	// don't mess with these, they are NOT settings:
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

	void RenderClassic();
	void RenderClassic(const S_COLOR_T& a_textColor, const S_COLOR_T* a_backColor,
		const S_COLOR_T& a_hyperLinkColor, bool a_backBlocks,
		size_t a_rowStart, size_t a_colStart, size_t a_rowEnd, size_t a_colEnd,
		cairo_surface_t* a_surface, double a_xBase, double a_yBase);

	bool IsTextChar(size_t a_row, size_t a_col, bool a_allowWhiteSpace = false) const;
	static ERenderGridShape CharCodeToGridShape(wchar_t a_char, uint8_t* ar_alpha = NULL);
public:
	CNFORenderer(bool a_classicMode = false);
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
		if(m_settings.bGaussShadow) {
		m_padding = m_settings.uGaussBlurRadius; // space for blur/shadow effect near the edges
		if(m_padding < 8) m_padding = 8;
		} else m_padding = 8;
	}
	unsigned int GetGaussBlurRadius() const { return m_settings.uGaussBlurRadius; }
	void SetHilightHyperLinks(bool nb) { m_rendered = m_rendered && (m_settings.bHilightHyperlinks == nb); m_settings.bHilightHyperlinks = nb; }
	bool GetHilightHyperLinks() const { return m_settings.bHilightHyperlinks; }
	void SetUnderlineHyperLinks(bool nb) { m_rendered = m_rendered && (m_settings.bUnderlineHyperlinks == nb); m_settings.bUnderlineHyperlinks = nb; }
	bool GetUnderlineHyperLinks() const { return m_settings.bUnderlineHyperlinks; }

	void SetBlockSize(size_t a_width, size_t a_height) { m_rendered = m_rendered &&
		a_width == m_settings.uBlockWidth && a_height == m_settings.uBlockHeight; m_settings.uBlockWidth = a_width; m_settings.uBlockHeight = a_height;
		if(!m_rendered) m_fontSize = -1; }
	size_t GetBlockWidth() const { return m_settings.uBlockWidth; }
	size_t GetBlockHeight() const { return m_settings.uBlockHeight; }

	// for quick switching between settings:
	const CNFORenderSettings GetSettings() const { return m_settings; }
	virtual void InjectSettings(const CNFORenderSettings& ns);

	// static color helper methods for anyone to use:
	static bool ParseColor(const char* a_str, S_COLOR_T* ar);
	static bool ParseColor(const wchar_t* a_str, S_COLOR_T* ar);
};


#endif /* !_NFO_RENDERER_H */
