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
#include "default_app.h"
#include "util.h"

using namespace std;


bool CWin5xDefaultApp::IsDefault()
{
	HKEY l_hKey;
	std::wstring l_keyPath = L"SOFTWARE\\Classes\\" + m_extension;

	/* phase 1: We check the "(Default)" value of the Classes\.nfo key for our ProgId */

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD l_dwType = REG_SZ, l_maxBuf = 999;
	wchar_t l_buf[1002] = {0};

	if(RegQueryValueEx(l_hKey, NULL, NULL, &l_dwType, (LPBYTE)l_buf, &l_maxBuf) == ERROR_SUCCESS)
	{
		if(wcscmp(l_buf, m_appRegistryName.c_str()) == 0)
		{
			RegCloseKey(l_hKey);

			/* phase 2: we found the ProgId, now check the path to our app */

			l_keyPath = L"SOFTWARE\\Classes\\" + m_appRegistryName + L"\\shell\\open\\command";

			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
			{
				return false;
			}

			bool l_result = false;

			if(RegQueryValueEx(l_hKey, NULL, NULL, &l_dwType, (LPBYTE)l_buf, &l_maxBuf) == ERROR_SUCCESS)
			{
				std::wstring l_tmpExePath = CUtil::GetExePath();
				wchar_t l_regBuf[1002] = {0}, l_tmpRegBuf[1002] = {0}, l_realBuf[1002] = {0};

				wchar_t* l_args = wcsstr(l_buf, L" \"%1");
				if(l_args) *l_args = 0;

				// reg path: unquote spaces and get long path name
				PathUnquoteSpaces(l_buf);
				GetLongPathName(l_buf, l_tmpRegBuf, 999);
				// reg path: l_buf --> l_tmpRegBuf

				// reg path: l_tmpRegBuf --> l_regBuf (with placeholders)
				PathUnExpandEnvStrings(l_tmpRegBuf, l_regBuf, 999);

				// actual path: l_tmpExePath --> l_realBuf (with placeholders)
				PathUnExpandEnvStrings(l_tmpExePath.c_str(), l_realBuf, 999);

				// quote spaces in both pathes:
				PathQuoteSpaces(l_realBuf);
				PathQuoteSpaces(l_buf);

				// compare:
				if(_wcsicmp(l_realBuf, l_regBuf) == 0)
				{
					l_result = true;
				}
			}

			RegCloseKey(l_hKey);

			return l_result;
		}
	}

	RegCloseKey(l_hKey);

	return false;
}


bool CWin5xDefaultApp::RegisterProgIdData()
{
	_tstring l_keyPath = _T("SOFTWARE\\Classes\\") + m_appRegistryName + _T("\\DefaultIcon");

	_tstring l_tmpExePath = CUtil::GetExePath(), l_exePath; // double back slashes... fucking stupid.... arrrgggghhh
	for(_tstring::size_type p = 0; p < l_tmpExePath.size(); p++)
	{
		if(l_tmpExePath[p] == _T('\\'))
			l_exePath += _T('\\');
		l_exePath += l_tmpExePath[p];
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
	l_keyPath = _T("SOFTWARE\\Classes\\") + m_appRegistryName + _T("\\shell\\open\\command");

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


bool CWin5xDefaultApp::MakeDefault()
{
	if(!RegisterProgIdData())
		return false;

	return false;
}

