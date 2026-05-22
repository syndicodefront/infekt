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

#ifndef _DROP_TARGET_HELPER_H
#define _DROP_TARGET_HELPER_H

#include <Windows.h>
#include <memory>

class CMainDropTargetHelper : public IDropTarget
{
public:
	CMainDropTargetHelper(HWND a_hwnd);

	// IUnknown implementation
	HRESULT _stdcall QueryInterface(REFIID iid, void** ppvObject);
	ULONG _stdcall AddRef();
	ULONG _stdcall Release();

	// IDropTarget implementation
	HRESULT _stdcall DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	HRESULT _stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	HRESULT _stdcall DragLeave();
	HRESULT _stdcall Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

	void Register();
	void Unregister();

protected:
	LONG m_refCount;
	HWND m_hwnd;

	bool m_allowDrop;
};

#endif /* !_DROP_TARGET_HELPER_H */
