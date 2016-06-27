/**
 * Copyright (C) 2010 syndicode
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
#include "shell-util.h"

/************************************************************************/
/* Class Declaration                                                    */
/************************************************************************/

class CNFOThumbProvider :
	public IThumbnailProvider,
	public IInitializeWithStream
{
private:
	long m_cRef;
	IStream *m_pStream;

public:
	CNFOThumbProvider()
	{
		m_cRef = 1;
		m_pStream = NULL;
	}

	virtual ~CNFOThumbProvider()
	{
		SafeRelease(&m_pStream);
	}

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		*ppv = NULL;

		static const QITAB qit[] =
		{
			QITABENT(CNFOThumbProvider, IInitializeWithStream),
			QITABENT(CNFOThumbProvider, IThumbnailProvider),
			{ 0 },
		};

		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		ULONG cRef = InterlockedDecrement(&m_cRef);
		if(!cRef)
		{
			delete this;
		}
		return cRef;
	}

	// IInitializeWithStream
	IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);

	// IThumbnailProvider
	IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);
};


/************************************************************************/
/* Class Implementation                                                 */
/************************************************************************/

IFACEMETHODIMP CNFOThumbProvider::Initialize(IStream *pStream, DWORD)
{
	HRESULT hr = E_UNEXPECTED; // can only be initialized once

	if(!m_pStream)
	{
		// take a reference to the stream if we have not been initialized yet
		hr = pStream->QueryInterface(&m_pStream);
	}

	return hr;
}

IFACEMETHODIMP CNFOThumbProvider::GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
{
	PNFOData l_nfoData;

	if(!LoadNFOFromStream(m_pStream, l_nfoData))
	{
		return E_FAIL;
	}

	// set up renderer:
	CNFORenderer l_renderer;

	if(!l_renderer.AssignNFO(l_nfoData))
	{
		return E_FAIL;
	}

	// make some guesses based on the requested thumb nail size:
	if(cx < 256)
	{
		l_renderer.SetBlockSize(4, 7);
	}

	// render and copy to DIB section:
	int l_imgWidth = (int)l_renderer.GetWidth(), l_imgHeight = (int)l_renderer.GetHeight();
	bool l_cut = false;

	if(l_imgHeight > 600)
	{
		// https://github.com/syndicodefront/infekt/issues/89
		l_imgHeight = 600;
		l_cut = true;
	}

	BITMAPINFO l_bi = {0};
	l_bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	l_bi.bmiHeader.biWidth = l_imgWidth;
	l_bi.bmiHeader.biHeight = -l_imgHeight;
	l_bi.bmiHeader.biPlanes = 1;
	l_bi.bmiHeader.biBitCount = 32;
	l_bi.bmiHeader.biYPelsPerMeter = l_bi.bmiHeader.biXPelsPerMeter = 1000;

	unsigned char* l_rawData;
	HBITMAP l_hBitmap = CreateDIBSection(NULL, &l_bi, DIB_RGB_COLORS, (void**)&l_rawData, NULL, 0);

	HRESULT hr = E_FAIL;

	if(l_hBitmap)
	{
		BITMAP l_bitmap = {0};
		GetObject(l_hBitmap, sizeof(BITMAP), &l_bitmap);
		cairo_surface_t* l_surfaceOut = cairo_image_surface_create_for_data(
			l_rawData, CAIRO_FORMAT_ARGB32, l_imgWidth, l_imgHeight, l_bitmap.bmWidthBytes);

		if(l_surfaceOut)
		{
			if(l_renderer.DrawToSurface(l_surfaceOut, 0, 0, 0, 0, l_imgWidth, l_imgHeight))
			{
				// fade out:
				if(l_cut)
				{
					const int FADE_HEIGHT = 100;

					cairo_t* cr = cairo_create(l_surfaceOut);
					cairo_set_line_width(cr, 1);

					for(int y = 0; y < FADE_HEIGHT; y++)
					{
						cairo_set_source_rgba(cr, S_COLOR_T_CAIRO(l_renderer.GetBackColor()),
							/*alpha=*/ y / static_cast<double>(FADE_HEIGHT - l_renderer.GetPadding()));

						cairo_move_to(cr, 0, l_imgHeight - 100 + y);
						cairo_rel_line_to(cr, l_imgWidth, 0);
						cairo_stroke(cr);
					}

					cairo_destroy(cr);
				}

				*phbmp = l_hBitmap;
				*pdwAlpha = WTSAT_ARGB;

				hr = S_OK;
			}

			cairo_surface_destroy(l_surfaceOut);
		}

		if(!SUCCEEDED(hr))
		{
			DeleteObject(l_hBitmap);
		}
	}

	return hr;
}


/************************************************************************/
/* CreateInstance Export                                                */
/************************************************************************/

HRESULT CNFOThumbProvider_CreateInstance(REFIID riid, void **ppv)
{
	*ppv = NULL;

	CNFOThumbProvider *pNew = new (std::nothrow) CNFOThumbProvider();

	if(pNew)
	{
		HRESULT hr = pNew->QueryInterface(riid, ppv);
		pNew->Release();

		return hr;
	}

	return E_OUTOFMEMORY;
}
