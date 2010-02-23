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

#define _TARGETVER_VISTA // for targetver.h
#include "targetver.h"

#include <string>
#include "default_app.h"
#include <shobjidl.h>


CWin6xDefaultApp::CWin6xDefaultApp(const std::wstring& a, const std::wstring& b) : CWinDefaultApp(a, b)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}


bool CWin6xDefaultApp::IsDefault() const
{
	IApplicationAssociationRegistration* pAAR;
	BOOL pfHasExt = FALSE;

	HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL,
		CLSCTX_INPROC, __uuidof(IApplicationAssociationRegistration), (void**)&pAAR);

	if(SUCCEEDED(hr))
	{
		hr = pAAR->QueryAppIsDefault(m_extension.c_str(),
			AT_FILEEXTENSION,
			AL_EFFECTIVE,
			m_appRegistryName.c_str(),
			&pfHasExt);

		pAAR->Release();
	}

	return SUCCEEDED(hr) && pfHasExt;
}


#ifdef MAKE_DEFAULT_APP_CAPAB

bool CWin6xDefaultApp::MakeDefault() const
{
	IApplicationAssociationRegistration* pAAR;

	HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, NULL,
			CLSCTX_INPROC, __uuidof(IApplicationAssociationRegistration), (void**)&pAAR);

	if(SUCCEEDED(hr))
	{
		hr = pAAR->SetAppAsDefault(m_appRegistryName.c_str(), m_extension.c_str(), AT_FILEEXTENSION);

		pAAR->Release();
	}

	return SUCCEEDED(hr);
}

#endif


CWin6xDefaultApp::~CWin6xDefaultApp()
{
	CoUninitialize();
}
