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

#include "targetver.h"
#include <windows.h>
#include <tchar.h>
#include <string>
#include "default_app.h"


INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR wszCommandLine, int nShowCmd)
{
	wchar_t** argv = __wargv;
	int argc = __argc;

	if(argc != 4)
	{
		MessageBox(0, _T("Command Line: \"<ProgId>\" \"<FileExtWithLeadingDot>\" \"<CompletePathToExeFileForIconsAndShellOpen>\""),
			_T("Hurr Durr"), MB_ICONEXCLAMATION);

		return -1;
	}

	CWinDefaultAppInfo l_info;
	l_info.sAppRegistryName = argv[0];
	l_info.sExtension = argv[1];
	l_info.sExePath = argv[2];

	CWinDefaultApp* l_da;
	OSVERSIONINFO l_osver = {sizeof(OSVERSIONINFO), 0};
	GetVersionEx(&l_osver);

	if(l_osver.dwMajorVersion < 6)
	{
		l_da = new CWin5xDefaultApp(l_info);
	}
	else
	{
		l_da = new CWin6xDefaultApp(l_info);
	}

	int l_result = 0;

	if(!l_da->MakeDefault())
	{
		l_result = 1;
	}
	
	delete l_da;

	return l_result;
}
