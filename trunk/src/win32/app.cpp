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
#include "app.h"
#include "default_app.h"
#include <mbctype.h>

using namespace std;


/************************************************************************/
/* APP ENTRY POINT                                                      */
/************************************************************************/

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR wszCommandLine, int nShowCmd)
{
	g_hInstance = hInstance;

	// harden this process a bit:
	CUtil::EnforceDEP();
	CUtil::HardenHeap();
	CUtil::RemoveCwdFromDllSearchPath();

	_wsetlocale(LC_CTYPE, L"C");

	HANDLE l_instMutex = CreateMutex(NULL, TRUE, _T("iNFektNfoViewerOneInstanceMutex"));
	bool l_prevInstance = (GetLastError() == ERROR_ALREADY_EXISTS);

	try
	{
		// Start Win32++:
		CNFOApp theApp;

		// extract file path from command line:
		if(theApp.ExtractStartupFilePath(wszCommandLine))
		{
			// activate prev instance if in single window/view mode:
			if(l_prevInstance && theApp.SwitchToPrevInstance())
			{
				return 0;
			}
		}

		// we need COM for default app stuff on Vista+,
		// and OLE for file drag&drop.
		OleInitialize(NULL);

		// Run the application:
		int l_exitCode = theApp.Run();

		OleUninitialize();

		return l_exitCode;
	}
	catch(CWinException* e)
	{
		e->MessageBox();

		return -1;
	}

	return -2;
}


/************************************************************************/
/* CNFOApp stuff                                                        */
/************************************************************************/

CNFOApp::CNFOApp()
{
	const std::wstring l_iniPath = CUtil::GetExeDir() + L"\\portable.ini";

	// an existing portable.ini file in the program dir switches
	// on iNFekt's portable mode.

	if(!PathFileExists(l_iniPath.c_str()))
	{
		m_settings = PSettingsBackend(new CRegistrySettingsBackend(L"Software\\cxxjoe\\iNFEKT\\"));
	}
	else
	{
		m_settings = PSettingsBackend(new CINISettingsBackend(l_iniPath));
	}
}


bool CNFOApp::ExtractStartupFilePath(const _tstring& a_commandLine)
{
	if(!a_commandLine.empty())
	{
		TCHAR* szPath = _tcsdup(a_commandLine.c_str());

		if(PathUnquoteSpaces(szPath) && PathFileExists(szPath))
		{
			m_startupFilePath = szPath;

			free(szPath);

			return true;
		}

		free(szPath);
	}

	return false;
}


bool CNFOApp::SwitchToPrevInstance()
{
	PSettingsSection l_sect;
	bool l_singleInstanceMode = false;

	// read setting (single instance yes/no):
	if(dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForReading(L"MainSettings", l_sect))
	{
		l_singleInstanceMode = l_sect->ReadBool(L"SingleInstanceMode", false);
		l_sect.reset();
	}

	if(l_singleInstanceMode)
	{
		// find previous instance main window:
		HWND l_prevMainWin = ::FindWindowEx(0, 0, INFEKT_MAIN_WINDOW_CLASS_NAME, NULL);

		if(l_prevMainWin)
		{
			// use WM_USER message to instruct previous instance to load the NFO:
			COPYDATASTRUCT l_cpds = {0};
			l_cpds.dwData = WM_LOAD_NFO;
			l_cpds.cbData = (DWORD)(m_startupFilePath.size() + 1) * sizeof(wchar_t);
			l_cpds.lpData = (void*)m_startupFilePath.c_str();

			if(::SendMessage(l_prevMainWin, WM_COPYDATA, 0, (LPARAM)&l_cpds) == TRUE)
			{
				::ShowWindow(l_prevMainWin, SW_SHOW);
				::SetForegroundWindow(l_prevMainWin);
				return true;
			}
		}
	}

	// we were unable to make the previous instance open the NFO.
	return false;
}


BOOL CNFOApp::InitInstance()
{
	if(!m_frame.Create())
	{
		::MessageBox(NULL, _T("Failed to create Frame window"), _T("ERROR"), MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}


CNFOApp::~CNFOApp()
{
}


/************************************************************************/
/* DEFAULT APP STUFF                                                    */
/************************************************************************/

#define DEFAULT_APP_REG_NAME L"iNFekt NFO Viewer" // Name in HKLM\SOFTWARE\RegisteredApplications
#define DEFAULT_APP_PROG_ID L"iNFEKT.NFOFile.1"
#define DEFAULT_APP_EXTENSION L".nfo"

// return values:
// 0 = not default NFO viewer
// 1 = is default viewer
// -1 = unable to determine because program hasn't been registered with Windows' program default facility
int CNFOApp::IsDefaultNfoViewer()
{
	int l_result = 0;

	if(CUtil::IsWinXP())
	{
		CWin5xDefaultApp l_defApp(DEFAULT_APP_PROG_ID, DEFAULT_APP_EXTENSION);

		l_result = (l_defApp.IsDefault() ? 1 : 0);
	}
	else if(CUtil::IsWin6x())
	{
		CWin6xDefaultApp l_defApp(DEFAULT_APP_REG_NAME, DEFAULT_APP_EXTENSION);

		if(!l_defApp.IsDefault())
		{
			if(l_defApp.GotNoSuchProgramName())
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			l_result = 1;
		}
	}

	return l_result;
}


bool CNFOApp::MakeDefaultNfoViewer()
{
	CWinDefaultApp* l_defApp = NULL;

	if(CUtil::IsWinXP())
	{
		l_defApp = new (std::nothrow) CWin5xDefaultApp(DEFAULT_APP_PROG_ID, DEFAULT_APP_EXTENSION);
	}
	else if(CUtil::IsWin6x())
	{
		l_defApp = new (std::nothrow) CWin6xDefaultApp(DEFAULT_APP_REG_NAME, DEFAULT_APP_EXTENSION);
	}

	bool l_result = false;

	if(l_defApp && l_defApp->MakeDefault())
	{
		// ensure consistent behaviour:
		if(l_defApp->IsDefault())
		{
			l_result = true;
		}
	}

	delete l_defApp;

	return l_result;
}


void CNFOApp::CheckDefaultNfoViewer(HWND a_hwnd, bool a_confirmation)
{
	int l_status = IsDefaultNfoViewer();

	if(l_status == 0)
	{
		if(MessageBox(a_hwnd, _T("iNFekt is not your default NFO file viewer. Do you want to make it the default viewer now?"),
			_T("Important"), MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			if(MakeDefaultNfoViewer())
			{
				MessageBox(a_hwnd, _T("iNFekt is now your default NFO viewer!"), _T("Great Success"), MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(a_hwnd, _T("A problem occured while trying to make iNFekt your default NFO viewer. Please do it manually."), _T("Problem"), MB_ICONEXCLAMATION);
			}
		}
	}
	else if(a_confirmation)
	{
		if(l_status == -1)
		{
			MessageBox(a_hwnd, _T("iNFekt has not been installed properly. Since you are using Windows Vista, 7, or a newer version, ")
				_T("this means that iNFekt can not register itself. Please re-install iNFekt using the setup routine, or manually ")
				_T("associate iNFekt with .nfo files."), _T("Problem"), MB_ICONEXCLAMATION);
		}
		else
		{
			MessageBox(a_hwnd, _T("iNFekt seems to be your default NFO viewer!"), _T("Great Success"), MB_ICONINFORMATION);
		}
	}
}


/************************************************************************/
/* GLOBAL VARS                                                          */
/************************************************************************/

HINSTANCE g_hInstance;
