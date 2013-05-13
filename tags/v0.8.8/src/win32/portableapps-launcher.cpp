/**
 * Copyright (C) 2012 cxxjoe
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
#include <Windows.h>
#include <stdlib.h>
#include <Shlwapi.h>
#include <Shellapi.h>
#include <string>

#pragma comment(lib, "Shlwapi.lib")

static std::wstring GetExeDir()
{
	TCHAR l_buf[1000] = {0};
	TCHAR l_buf2[1000] = {0};

	::GetModuleFileName(NULL, (LPTCH)l_buf, 999);
	::GetLongPathName(l_buf, l_buf2, 999);
	::PathRemoveFileSpec(l_buf2);
	::PathRemoveBackslash(l_buf2);

	return l_buf2;
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR wszCommandLine, int nShowCmd)
{
	std::wstring l_path = GetExeDir(),
		l_dataPath = l_path + L"\\Data",
		l_defaultDataPath = l_path + L"\\App\\DefaultData";

	if(!::PathIsDirectory(l_dataPath.c_str()) && ::PathIsDirectory(l_defaultDataPath.c_str()))
	{
		_wmkdir(l_dataPath.c_str());
	}

	std::wstring l_portableIniPath = l_dataPath + L"\\portable.ini";

	if(!::PathFileExists(l_portableIniPath.c_str()) && ::PathIsDirectory(l_dataPath.c_str()))
	{
		const std::wstring l_copyFrom = l_defaultDataPath + L"\\portable.ini";

		::CopyFile(l_copyFrom.c_str(), l_portableIniPath.c_str(), TRUE);
	}

	if(!::PathIsDirectory(l_dataPath.c_str()) || !::PathFileExists(l_portableIniPath.c_str()))
	{
		::MessageBox(HWND_DESKTOP, L"The configuration folder (\"Data\") seems obstructed.", L"iNFekt Portable", MB_ICONEXCLAMATION);

		return 1;
	}
	else
	{
		const std::wstring l_exePath = l_path + L"\\App\\iNFEKT\\infekt-win32.exe";

		return (INT)::ShellExecute(HWND_DESKTOP, L"open", l_exePath.c_str(), wszCommandLine, NULL, nShowCmd);
	}
}
