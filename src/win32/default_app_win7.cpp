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

#define _TARGETVER_VISTA // for targetver.h
#include "targetver.h"

#include <string>
#include "default_app.h"
#include <shobjidl.h>

CWin7DefaultApp::CWin7DefaultApp(const std::wstring& a_appRegistryName, const std::wstring& a_fileExtension)
	: CWinDefaultApp(a_appRegistryName, a_fileExtension)
{
}

bool CWin7DefaultApp::IsDefault()
{
	IApplicationAssociationRegistration* pAAR = nullptr;
	BOOL pfHasExt = FALSE;

	HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));

	if (SUCCEEDED(hr))
	{
		hr = pAAR->QueryAppIsDefault(m_extension.c_str(),
			AT_FILEEXTENSION,
			AL_EFFECTIVE,
			m_appRegistryName.c_str(),
			&pfHasExt);

		m_noSuchProgName = (hr == 0x80070002); // is there not a #define for this?!
		// 0x80070002 = No such file or directory

		pAAR->Release();
	}

	return SUCCEEDED(hr) && pfHasExt;
}

CWin7DefaultApp::MakeDefaultResult CWin7DefaultApp::MakeDefault()
{
	IApplicationAssociationRegistration* pAAR = nullptr;

	HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));

	if (SUCCEEDED(hr))
	{
		hr = pAAR->SetAppAsDefault(m_appRegistryName.c_str(), m_extension.c_str(), AT_FILEEXTENSION);

		pAAR->Release();
	}

	return SUCCEEDED(hr) ? MakeDefaultResult::SUCCEEDED : MakeDefaultResult::FAILED;
}

