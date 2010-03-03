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

	_wsetlocale(LC_CTYPE, L"C");

	try
	{
		// Start Win32++:
		CNFOApp theApp;

		// extract file path from command line:
		wstring l_path(wszCommandLine);
		if(!l_path.empty())
		{
			bool l_ok = false;

			if(l_path[0] == L'"')
			{
				l_path.erase(0, 1);

				wstring::size_type l_pos = l_path.find(L'"');

				if(l_pos != wstring::npos)
				{
					l_path.erase(l_pos);
					l_ok = true;
				}
			}
			else
				l_ok = true;

			if(l_ok)
			{
				theApp.SetStartupFilePath(l_path);
			}
		}

		CoInitializeEx(NULL, COINIT_MULTITHREADED);

		// Run the application:
		int l_exitCode = theApp.Run();

		CoUninitialize();

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
}


BOOL CNFOApp::InitInstance()
{
	if(!m_Frame.Create())
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

#define DEFAULT_APP_PROGID L"iNFEKT.NFO.Viewer.NFOFile.1"
#define DEFAULT_APP_EXTENSION L".nfo"

int CNFOApp::IsDefaultNfoViewer()
{
	int l_result = 0;

	if(CUtil::IsWin5x())
	{
		CWin5xDefaultApp l_defApp(DEFAULT_APP_PROGID, DEFAULT_APP_EXTENSION);

		l_result = (l_defApp.IsDefault() ? 0 : 1);
	}
	else if(CUtil::IsWin6x())
	{
		CWin6xDefaultApp l_defApp(DEFAULT_APP_PROGID, DEFAULT_APP_EXTENSION);

		if(!l_defApp.IsDefault())
		{
			if(l_defApp.GotNoSuchProgId())
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
	return false;
}


void CNFOApp::CheckDefaultNfoViewer(HWND a_hwnd)
{
	int l_status = IsDefaultNfoViewer();

	if(l_status == 0)
	{
		if(MessageBox(a_hwnd, _T("iNFEKT is not your default NFO file viewer. Do you want to make it the default viewer now?"),
			_T("Important"), MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			MakeDefaultNfoViewer();
		}
	}
	else if(l_status == -1)
	{
		MessageBox(a_hwnd, _T("iNFEKT has not been installed properly. Since you are using Windows Vista, 7, or a newer version, ")
			_T("this means that iNFEKT can not register itself. Please re-install iNFEKT using the setup routine, or manually ")
			_T("associate iNFEKT with .nfo files."), _T("Problem"), MB_ICONEXCLAMATION);
	}
}


/************************************************************************/
/* GLOBAL VARS                                                          */
/************************************************************************/

HINSTANCE g_hInstance;
