/**
 * Copyright (C) 2010-2014 syndicode
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
#include "drop_target_helper.h"
#include "infekt_global.h"


/************************************************************************/
/* Drop Target Helper                                                   */
/************************************************************************/

CMainDropTargetHelper::CMainDropTargetHelper(HWND a_hwnd)
{
	m_hwnd = a_hwnd;
	m_refCount = 1;
	m_allowDrop = false;
}


HRESULT _stdcall CMainDropTargetHelper::QueryInterface(REFIID iid, void** ppvObject)
{
	if (iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
}


ULONG _stdcall CMainDropTargetHelper::AddRef()
{
	return ::InterlockedIncrement(&m_refCount);
}


ULONG _stdcall CMainDropTargetHelper::Release()
{
	LONG l_new = ::InterlockedDecrement(&m_refCount);

	if (l_new == 0)
	{
		delete this;
	}

	return l_new;
}


HRESULT _stdcall CMainDropTargetHelper::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	if (pDataObject->QueryGetData(&fmtetc) == S_OK)
	{
		*pdwEffect = DROPEFFECT_MOVE;

		m_allowDrop = true;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;

		m_allowDrop = false;
	}

	return S_OK;
}


HRESULT _stdcall CMainDropTargetHelper::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	*pdwEffect = (m_allowDrop ? DROPEFFECT_MOVE : DROPEFFECT_NONE);

	return S_OK;
}


HRESULT _stdcall CMainDropTargetHelper::DragLeave()
{
	return S_OK;
}


HRESULT _stdcall CMainDropTargetHelper::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	if (!m_allowDrop)
	{
		return E_UNEXPECTED;
	}

	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;

	if (pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
	{
		HDROP l_hDrop = (HDROP)::GlobalLock(stgmed.hGlobal);

		if (l_hDrop)
		{
			UINT l_numFiles = ::DragQueryFile(l_hDrop, 0xFFFFFFFF, nullptr, 0);

			if (l_numFiles > 0)
			{
				wchar_t l_fileNameBuf[1000] = { 0 };
				UINT l_charsCopied = ::DragQueryFile(l_hDrop, 0, l_fileNameBuf, 999); // get the first file.

				if (l_charsCopied > 0 && l_charsCopied < 1000)
				{
					::SendMessage(m_hwnd, WM_LOAD_NFO, (WPARAM)l_fileNameBuf, l_charsCopied);
				}
			}
		}

		::GlobalUnlock(stgmed.hGlobal);

		// release the data using the COM API
		::ReleaseStgMedium(&stgmed);
	}

	return S_OK;
}


void CMainDropTargetHelper::Register()
{
	::CoLockObjectExternal(this, TRUE, FALSE);
	::RegisterDragDrop(m_hwnd, this);
}


void CMainDropTargetHelper::Unregister()
{
	::RevokeDragDrop(m_hwnd);
	::CoLockObjectExternal(this, FALSE, TRUE);
}
