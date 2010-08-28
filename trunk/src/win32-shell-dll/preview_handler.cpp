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

/************************************************************************/
/* Class Declaration                                                    */
/************************************************************************/

class CNFOPreviewHandler :
	public IPreviewHandler,
	public IInitializeWithStream,
	public IObjectWithSite,
	public IOleWindow
{
private:
	long m_cRef;
	IStream *m_pStream;
	IUnknown *m_punkSite;
	HWND m_hwndParent;
	RECT m_rcParent;

public:
	CNFOPreviewHandler()
	{
		m_cRef = 1;
		m_pStream = NULL;
		m_punkSite = NULL;
		m_hwndParent = 0;
	}

	virtual ~CNFOPreviewHandler()
	{
		// :TODO: destroy window
		SafeRelease(&m_pStream);
		SafeRelease(&m_punkSite);
	}

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		*ppv = NULL;

		static const QITAB qit[] =
		{
			QITABENT(CNFOPreviewHandler, IObjectWithSite),
			QITABENT(CNFOPreviewHandler, IOleWindow),
			QITABENT(CNFOPreviewHandler, IInitializeWithStream),
			QITABENT(CNFOPreviewHandler, IPreviewHandler),
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

	// IObjectWithSite
	IFACEMETHODIMP SetSite(IUnknown *punkSite);
	IFACEMETHODIMP GetSite(REFIID riid, void **ppv);

	// IPreviewHandler
	IFACEMETHODIMP SetWindow(HWND hwnd, const RECT *prc);
	IFACEMETHODIMP SetFocus();
	IFACEMETHODIMP QueryFocus(HWND *phwnd);
	IFACEMETHODIMP TranslateAccelerator(MSG *pmsg);
	IFACEMETHODIMP SetRect(const RECT *prc);
	IFACEMETHODIMP DoPreview();
	IFACEMETHODIMP Unload();

	// IOleWindow
	IFACEMETHODIMP GetWindow(HWND *phwnd);
	IFACEMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

	// IInitializeWithStream
	IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);
};


/************************************************************************/
/* CreateInstance Export                                                */
/************************************************************************/

HRESULT CNFOPreviewHandler_CreateInstance(REFIID riid, void **ppv)
{
	*ppv = NULL;
/*
	CNFOPreviewHandler *pNew = new (std::nothrow) CNFOPreviewHandler();

	if(pNew)
	{
		HRESULT hr = pNew->QueryInterface(riid, ppv);
		pNew->Release();

		return hr;
	}*/

	return E_OUTOFMEMORY;
}
