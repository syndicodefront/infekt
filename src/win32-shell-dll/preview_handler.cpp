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
#include "nfo_view_ctrl.h"
#include "shell-util.h"

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
	PNFOViewControl m_view;

public:
	CNFOPreviewHandler()
	{
		m_cRef = 1;
		m_pStream = nullptr;
		m_punkSite = nullptr;
		m_hwndParent = 0;
	}

	virtual ~CNFOPreviewHandler()
	{
		m_view.reset();
		SafeRelease(&m_pStream);
		SafeRelease(&m_punkSite);
	}

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		*ppv = nullptr;

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
		if (!cRef)
		{
			delete this;
		}
		return cRef;
	}

	// IPreviewHandler
	IFACEMETHODIMP SetWindow(HWND hwnd, const RECT *prc);
	IFACEMETHODIMP SetFocus();
	IFACEMETHODIMP QueryFocus(HWND *phwnd);
	IFACEMETHODIMP TranslateAccelerator(MSG *pmsg);
	IFACEMETHODIMP SetRect(const RECT *prc);
	IFACEMETHODIMP DoPreview();
	IFACEMETHODIMP Unload();

	// IInitializeWithStream
	IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);

	// IObjectWithSite
	IFACEMETHODIMP SetSite(IUnknown *punkSite);
	IFACEMETHODIMP GetSite(REFIID riid, void **ppv);

	// IOleWindow
	IFACEMETHODIMP GetWindow(HWND *phwnd);
	IFACEMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
};


/************************************************************************/
/* IPreviewHandler                                                      */
/************************************************************************/

HRESULT CNFOPreviewHandler::SetWindow(HWND hwnd, const RECT *prc)
{
	if (hwnd && prc)
	{
		m_hwndParent = hwnd;
		m_rcParent = *prc;

		if (m_view)
		{
			m_view->SetParent(m_hwndParent);

			if (prc->right - prc->left > 0 && prc->bottom - prc->top > 0)
			{
				::MoveWindow(m_view->GetHwnd(), m_rcParent.left, m_rcParent.top,
					m_rcParent.right - m_rcParent.left,
					m_rcParent.bottom - m_rcParent.top, TRUE);
			}
		}

		return S_OK;
	}

	return E_INVALIDARG;
}

HRESULT CNFOPreviewHandler::SetFocus()
{
	if (m_view)
	{
		::SetFocus(m_view->GetHwnd());
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CNFOPreviewHandler::QueryFocus(HWND *phwnd)
{
	if (phwnd)
	{
		*phwnd = ::GetFocus();
		return (*phwnd ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
	}

	return E_INVALIDARG;
}

HRESULT CNFOPreviewHandler::TranslateAccelerator(MSG *pmsg)
{
	HRESULT hr = S_FALSE;
	IPreviewHandlerFrame *pFrame = nullptr;

	if (m_punkSite && SUCCEEDED(m_punkSite->QueryInterface(&pFrame)))
	{
		// Our previewer has no tabstops, so we want to just forward this message out:
		hr = pFrame->TranslateAccelerator(pmsg);
		SafeRelease(&pFrame);
	}

	return hr;
}

// This method gets called when the size of the previewer window changes (user resizes the Reading Pane)
HRESULT CNFOPreviewHandler::SetRect(const RECT *prc)
{
	if (prc)
	{
		m_rcParent = *prc;

		if (m_view)
		{
			// Preview window is already created, so set its size and position
			::MoveWindow(m_view->GetHwnd(), m_rcParent.left, m_rcParent.top,
				m_rcParent.right - m_rcParent.left,
				m_rcParent.bottom - m_rcParent.top, FALSE);

			m_view->ZoomToNoHorizontalScrollbars();
		}

		return S_OK;
	}

	return E_INVALIDARG;
}

// The main method that extracts relevant information from the file stream and
// draws content to the previewer window
HRESULT CNFOPreviewHandler::DoPreview()
{
	if (m_view || !m_pStream)
	{
		// cannot be called more than once (Unload should be called before another DoPreview)
		return E_PREVIEWHANDLER_NOTFOUND;
	}

	CNFOViewControl* l_temp = new (std::nothrow) CNFOViewControl(g_hInst, m_hwndParent);

	if (l_temp)
	{
		PNFOViewControl l_view(l_temp);
		PNFOData l_nfoData;

		if (!LoadNFOFromStream(m_pStream, l_nfoData) || !l_view->AssignNFO(l_nfoData))
		{
			return E_PREVIEWHANDLER_CORRUPT;
		}

		if (!l_view->CreateControl(m_rcParent.left, m_rcParent.top,
			m_rcParent.right - m_rcParent.left,
			m_rcParent.bottom - m_rcParent.top))
		{
			return HRESULT_FROM_WIN32(::GetLastError());
		}

		l_view->ZoomToNoHorizontalScrollbars();

		l_view->Show();

		m_view = l_view;

		return S_OK;
	}

	return E_OUTOFMEMORY;
}

// This method gets called when a shell item is de-selected in the listview
HRESULT CNFOPreviewHandler::Unload()
{
	SafeRelease(&m_pStream);

	m_view.reset();

	return S_OK;
}


/************************************************************************/
/* IInitializeWithStream                                                */
/************************************************************************/

HRESULT CNFOPreviewHandler::Initialize(IStream *pStream, DWORD)
{
	if (pStream)
	{
		// Initialize can be called more than once, so release existing valid stream
		SafeRelease(&m_pStream);

		m_pStream = pStream;
		m_pStream->AddRef();

		return S_OK;
	}

	return E_INVALIDARG;
}


/************************************************************************/
/* IObjectWithSite                                                      */
/************************************************************************/

HRESULT CNFOPreviewHandler::SetSite(IUnknown *punkSite)
{
	SafeRelease(&m_punkSite);
	return punkSite ? punkSite->QueryInterface(&m_punkSite) : S_OK;
}

HRESULT CNFOPreviewHandler::GetSite(REFIID riid, void **ppv)
{
	*ppv = nullptr;
	return m_punkSite ? m_punkSite->QueryInterface(riid, ppv) : E_FAIL;
}


/************************************************************************/
/* IOleWindow                                                           */
/************************************************************************/

HRESULT CNFOPreviewHandler::GetWindow(HWND* phwnd)
{
	if (!phwnd)
		return E_INVALIDARG;
	if (!m_view)
		return S_FALSE;

	*phwnd = m_view->GetHwnd();

	return S_OK;
}

HRESULT CNFOPreviewHandler::ContextSensitiveHelp(BOOL)
{
	return E_NOTIMPL;
}


/************************************************************************/
/* CreateInstance Export                                                */
/************************************************************************/

HRESULT CNFOPreviewHandler_CreateInstance(REFIID riid, void **ppv)
{
	*ppv = nullptr;

	CNFOPreviewHandler *pNew = new (std::nothrow) CNFOPreviewHandler();

	if (pNew)
	{
		HRESULT hr = pNew->QueryInterface(riid, ppv);
		pNew->Release();

		return hr;
	}

	return E_OUTOFMEMORY;
}
