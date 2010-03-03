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
#include <shlwapi.h>
#include <string>
#include "default_app.h"

using namespace std;

#ifndef _tstring
#define _tstring wstring
#endif

#ifdef MAKE_DEFAULT_APP_CAPAB

bool CWinDefaultApp::RegisterProgIdData() const
{
	_tstring l_keyPath = _T("SOFTWARE\\Classes\\") + m_info.sAppRegistryName + _T("\\DefaultIcon");

	_tstring l_exePath; // double back slashes... fucking stupid.... arrrgggghhh
	for(_tstring::size_type p = 0; p < m_info.sExePath.size(); p++)
	{
		if(m_info.sExePath[p] == _T('\\'))
			l_exePath += _T('\\');
		l_exePath += m_info.sExePath[p];
	}

	HKEY l_hKey;
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		return false;
	}

	// set DefaultIcon
	TCHAR l_defaultIconInfoBuf[1000] = {0};
	PathUnExpandEnvStrings(l_exePath.c_str(), l_defaultIconInfoBuf, 999);
	_tstring l_defaultIconInfo = _T("\"");
	l_defaultIconInfo += l_defaultIconInfoBuf;
	l_defaultIconInfo += _T("\", 0");
	RegSetValueEx(l_hKey, NULL, 0, REG_SZ, (LPBYTE)l_defaultIconInfo.c_str(), (l_defaultIconInfo.size() + 1) * sizeof(TCHAR));

	RegCloseKey(l_hKey);

	// now about shell\open\command...
	l_keyPath = _T("SOFTWARE\\Classes\\") + m_info.sAppRegistryName + _T("\\shell\\open\\command");

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		return false;
	}

	// set exe path
	TCHAR l_shellOpenCommandBuf[1000] = {0};
	PathUnExpandEnvStrings(l_exePath.c_str(), l_shellOpenCommandBuf, 999);
	_tstring l_shellOpenCommand = l_shellOpenCommandBuf;
	RegSetValueEx(l_hKey, NULL, 0, REG_SZ, (LPBYTE)l_shellOpenCommand.c_str(), (l_shellOpenCommand.size() + 1) * sizeof(TCHAR));

	RegCloseKey(l_hKey);

	// JESUS FUCKING CHRIST
	// I'M GOING TO CUT OFF MY FINGERS
	// WITH A SPOON
	// NOW

	return true;
}

#endif


std::wstring CWinDefaultApp::GetExePath()
{
	wchar_t l_buf[1000] = {0};
	wchar_t l_buf2[1000] = {0};

	::GetModuleFileName(NULL, (LPWCH)l_buf, 999);
	::GetLongPathName(l_buf, l_buf2, 999);

	return l_buf2;
}
