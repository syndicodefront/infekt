/**
* Copyright (C) 2016 syndicode
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
#include "util.h"
#include "default_app.h"
#include <shobjidl.h>

CWin8DefaultApp::CWin8DefaultApp(const std::wstring& a_appRegistryName, const std::wstring& a_fileExtension)
	: CWinDefaultApp(a_appRegistryName, a_fileExtension)
{
}

bool CWin8DefaultApp::IsDefault()
{
	bool isDefault = false;

	IApplicationAssociationRegistration* pAAR = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pAAR));

	if (SUCCEEDED(hr))
	{
		LPWSTR registeredAppName = nullptr;

		hr = pAAR->QueryCurrentDefault(m_extension.c_str(),
			AT_FILEEXTENSION,
			AL_EFFECTIVE,
			&registeredAppName);

		m_noSuchProgName = (hr == 0x80070002); // is there not a #define for this?!
											   // 0x80070002 = No such file or directory

		if (SUCCEEDED(hr))
		{
			isDefault = registeredAppName != nullptr &&
				m_appRegistryName.compare(registeredAppName) == 0;

			CoTaskMemFree(registeredAppName);
		}

		pAAR->Release();
	}

	return isDefault;
}

CWin8DefaultApp::MakeDefaultResult CWin8DefaultApp::MakeDefault()
{
	MakeDefaultResult result = MakeDefaultResult::SUCCEEDED;

	IApplicationActivationManager* pActManager = nullptr;

	HRESULT hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&pActManager));

	if (SUCCEEDED(hr))
	{
		DWORD pid{};

		hr = pActManager->ActivateApplication(
			L"windows.immersivecontrolpanel_cw5n1h2txyewy"
			L"!microsoft.windows.immersivecontrolpanel",
			L"page=SettingsPageAppsDefaults"
			L"&target=SettingsPageAppsDefaultsFileExtensionView", AO_NONE, &pid);

		if (!SUCCEEDED(hr))
		{
			hr = pActManager->ActivateApplication(
				L"windows.immersivecontrolpanel_cw5n1h2txyewy"
				L"!microsoft.windows.immersivecontrolpanel",
				L"page=SettingsPageAppsDefaults", AO_NONE, &pid);

			if (!SUCCEEDED(hr))
			{
				result = MakeDefaultResult::FAILED;
			}
		}

		pActManager->Release();
	}
	else
	{
		result = MakeDefaultResult::NOT_SUPPORTED;
	}

	return result;
}
