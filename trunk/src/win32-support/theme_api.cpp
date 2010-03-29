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

#define _TARGETVER_WIN7
#include "targetver.h"
#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <theme_api.h>
#include <tchar.h>


CThemeAPI::CThemeAPI()
{
	UINT l_oldErrorMode = ::SetErrorMode(SEM_NOOPENFILEERRORBOX);

	m_hUxTheme = ::LoadLibrary(_T("uxtheme.dll"));
	m_hDwmApi = ::LoadLibrary(_T("Dwmapi.dll"));

	::SetErrorMode(l_oldErrorMode);
}


const CThemeAPI* CThemeAPI::GetInstance()
{
	static CThemeAPI* ls_inst = NULL;

	if(!ls_inst)
	{
		ls_inst = new CThemeAPI();
	}

	return ls_inst;
}


bool CThemeAPI::IsThemeActive() const
{
	// "Do not call this function during DllMain or global objects
	// contructors. This may cause invalid return values in Windows
	// Vista and may cause Windows XP to become unstable."

	typedef BOOL (WINAPI *fnc)(void);

	if(fnc ita = (fnc)GetProcAddress(m_hUxTheme, "IsThemeActive"))
	{
		return (ita() != FALSE);
	}

	return false;
}


bool CThemeAPI::DwmIsCompositionEnabled()
{
	BOOL b = FALSE;

	typedef HRESULT (STDAPICALLTYPE *fnc)(BOOL*);

	if(fnc dice = (fnc)GetProcAddress(m_hDwmApi, "DwmIsCompositionEnabled"))
	{
		dice(&b);
	}

	return (b != FALSE);
}


HRESULT CThemeAPI::EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags) const
{
	typedef HRESULT (WINAPI *fnc)(HWND, DWORD dwFlags);

	if(fnc etdt = (fnc)GetProcAddress(m_hUxTheme, "EnableThemeDialogTexture"))
	{
		return etdt(hwnd, dwFlags);
	}

	return S_FALSE;
}


HANDLE CThemeAPI::OpenThemeData(HWND hwnd, LPCWSTR pszClassList) const
{
	typedef HANDLE (WINAPI *fnc)(HWND, LPCWSTR);

	if(fnc otd = (fnc)GetProcAddress(m_hUxTheme, "OpenThemeData"))
	{
		return otd(hwnd, pszClassList);
	}

	return NULL;
}


HRESULT CThemeAPI::DrawThemeBackground(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect) const
{
	typedef HRESULT (WINAPI *fnc)(HANDLE, HDC, int, int, const RECT*, const RECT*);

	if(fnc dtbg = (fnc)GetProcAddress(m_hUxTheme, "DrawThemeBackground"))
	{
		return dtbg(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
	}

	return S_FALSE;
}


HRESULT CThemeAPI::CloseThemeData(HANDLE hTheme) const
{
	typedef HRESULT (WINAPI *fnc)(HANDLE);

	if(fnc ctd = (fnc)GetProcAddress(m_hUxTheme, "CloseThemeData"))
	{
		return ctd(hTheme);
	}

	return S_FALSE;
}


CThemeAPI::~CThemeAPI()
{
	if(m_hUxTheme)
	{
		::FreeLibrary(m_hUxTheme);
	}

	if(m_hDwmApi)
	{
		::FreeLibrary(m_hDwmApi);
	}
}