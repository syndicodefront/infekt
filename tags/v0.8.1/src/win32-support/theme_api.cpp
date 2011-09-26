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
#include <string>


CThemeAPI::CThemeAPI()
{
	UINT l_oldErrorMode = ::SetErrorMode(SEM_NOOPENFILEERRORBOX);
	TCHAR l_buf[1000] = {0};
	OSVERSIONINFO l_osVer = { sizeof(OSVERSIONINFO), 0 };

	if(GetSystemDirectory(l_buf, 999) <= 999 && GetVersionEx(&l_osVer))
	{
		if(l_osVer.dwMajorVersion >= 6)
		{
			std::wstring l_pathDwm(l_buf);
			l_pathDwm.append(_T("\\Dwmapi.dll"));
			m_hDwmApi = ::LoadLibrary(l_pathDwm.c_str());
		}
	}

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

CThemeAPI::~CThemeAPI()
{
	if(m_hDwmApi)
	{
		::FreeLibrary(m_hDwmApi);
	}
}