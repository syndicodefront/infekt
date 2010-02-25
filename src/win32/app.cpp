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

		// dffd
		if(!theApp.IsDefaultNfoViewer())
		{
#if 0
			if(MessageBox(0, _T("iNFEKT is not your default NFO file viewer. Do you want to make it the default viewer now?"),
				_T("Important"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				theApp.MakeDefaultNfoViewer();
			}
#endif
		}

		// Run the application:
		return theApp.Run();
	}
	catch(CWinException* e)
	{
		e->MessageBox();
		exit(-1);
	}
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

bool CNFOApp::IsDefaultNfoViewer()
{
	CWinDefaultApp *l_defApp = NULL;
	bool l_result = false;

	if(CUtil::IsWin5x())
	{
		l_defApp = new CWin5xDefaultApp(DEFAULT_APP_PROGID, DEFAULT_APP_EXTENSION);
	}
	else if(CUtil::IsWin6x())
	{
		l_defApp = new CWin6xDefaultApp(DEFAULT_APP_PROGID, DEFAULT_APP_EXTENSION);
	}

	if(l_defApp)
	{
		l_result = l_defApp->IsDefault();

		delete l_defApp;
	}

	return l_result;
}


bool CNFOApp::MakeDefaultNfoViewer()
{
	return false;
}


/************************************************************************/
/* GLOBAL VARS                                                          */
/************************************************************************/

HINSTANCE g_hInstance;
