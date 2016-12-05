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
#include "default_app.h"
#include "util.h"

#define DEFAULT_APP_REG_NAME L"iNFekt NFO Viewer" // Name in HKLM\SOFTWARE\RegisteredApplications
#define DEFAULT_APP_PROG_ID L"iNFEKT.NFOFile.1"
#define DEFAULT_APP_EXTENSION L".nfo"

std::unique_ptr<CWinDefaultApp> CWinDefaultApp::Factory()
{
	std::unique_ptr<CWinDefaultApp> l_defApp;

#if _WIN32_WINNT < 0x600
	if (CUtilWin32::IsWinXP())
	{
		l_defApp.reset(new CWinXPDefaultApp(DEFAULT_APP_PROG_ID, DEFAULT_APP_EXTENSION));
	}
	else
#endif
	if (CUtilWin32::IsAtLeastWin8())
	{
		l_defApp.reset(new CWin8DefaultApp(DEFAULT_APP_PROG_ID, DEFAULT_APP_EXTENSION));
	}
	else if (CUtilWin32::IsAtLeastWin7())
	{
		l_defApp.reset(new CWin7DefaultApp(DEFAULT_APP_REG_NAME, DEFAULT_APP_EXTENSION));
	}

	return l_defApp;
}

void CWinDefaultApp::CheckDefaultNfoViewer(HWND a_hwnd)
{
	if (!CanCheckDefaultNfoViewer())
	{
		return;
	}
	else if (this->IsDefault())
	{
		return;
	}

	if (::MessageBoxW(a_hwnd, L"iNFekt is not your default NFO file viewer. Do you want to make it the default viewer now?",
		L"Important", MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		if (this->MakeDefault() == MakeDefaultResult::FAILED)
		{
			::MessageBoxW(a_hwnd, L"A problem occured while trying to make iNFekt your default NFO viewer. Please do it manually.", L"Problem", MB_ICONEXCLAMATION);
		}
	}
}

void CWinDefaultApp::CheckDefaultNfoViewerInteractive(HWND a_hwnd)
{
	if (!CanCheckDefaultNfoViewer())
	{
		::MessageBoxW(a_hwnd, L"iNFekt has not been installed properly or you are using a portable version. "
			L"Unfortunately, iNFekt can not fix that for you. Please re-install iNFekt using the setup routine, or manually "
			L"associate iNFekt with .nfo files.", L"Problem", MB_ICONEXCLAMATION);
	}
	else if (this->IsDefault())
	{
		::MessageBoxW(a_hwnd, L"iNFekt seems to be your default NFO viewer!", L"Great Success", MB_ICONINFORMATION);
	}
	else if (::MessageBoxW(a_hwnd, L"iNFekt is not your default NFO file viewer. Do you want to make it the default viewer now?",
		L"Important", MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		MakeDefaultResult result = this->MakeDefault();

		if (result == MakeDefaultResult::SUCCEEDED)
		{
			// ensure consistent behaviour:
			if (this->IsDefault())
			{
				::MessageBoxW(a_hwnd, L"iNFekt is now your default NFO viewer!", L"Great Success", MB_ICONINFORMATION);
			}
			else
			{
				result = MakeDefaultResult::FAILED;
			}
		}

		if (result == MakeDefaultResult::FAILED)
		{
			::MessageBoxW(a_hwnd, L"A problem occured while trying to make iNFekt your default NFO viewer. Please do it manually.", L"Problem", MB_ICONEXCLAMATION);
		}
	}
}

bool CWinDefaultApp::CanCheckDefaultNfoViewer()
{
	if (!this->IsDefault() && this->GotNoSuchProgramName())
	{
		return false;
	}

	return true;
}
