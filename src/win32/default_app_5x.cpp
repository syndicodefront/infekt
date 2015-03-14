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

#include "stdafx.h"

#if _WIN32_WINNT < 0x600
#include "default_app.h"
#include "util.h"
#include <shlobj.h>

using namespace std;


bool CWin5xDefaultApp::IsDefault()
{
	HKEY l_hKey;
	std::wstring l_keyPath = m_extension;

	/* phase 1: We check the "(Default)" value of the Classes\.nfo key for our ProgId */
	/* remember that HKCR is the merged version of HKLM\Software\Classes (machine level
		defaults) and HKCU\Software\Classes (user settings) */

	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, l_keyPath.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD l_dwType = REG_SZ, l_maxBuf = 998 / sizeof(wchar_t);
	wchar_t l_buf[1002] = {0};

	if(RegQueryValueEx(l_hKey, NULL, NULL, &l_dwType, (LPBYTE)l_buf, &l_maxBuf) != ERROR_SUCCESS
		|| l_dwType != REG_SZ || wcscmp(l_buf, m_appRegistryName.c_str()) != 0)
	{
		RegCloseKey(l_hKey);

		return false;
	}

	RegCloseKey(l_hKey);

	l_keyPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\") + m_extension;
	memset(l_buf, 0, 1002); l_maxBuf = 998 / sizeof(wchar_t);

	if(RegOpenKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	if(RegQueryValueEx(l_hKey, _T("Progid"), 0, &l_dwType, (LPBYTE)l_buf, &l_maxBuf) == ERROR_SUCCESS && l_dwType == REG_SZ)
	{
		if(wcscmp(l_buf, m_appRegistryName.c_str()) != 0)
		{
			RegCloseKey(l_hKey);

			return false;
		}
	}

	RegCloseKey(l_hKey);

	l_keyPath = L"SOFTWARE\\Classes\\" + m_appRegistryName + L"\\shell\\open\\command";

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	bool l_result = false;

	memset(l_buf, 0, 1002); l_maxBuf = 998 / sizeof(wchar_t);
	if(RegQueryValueEx(l_hKey, NULL, 0, &l_dwType, (LPBYTE)l_buf, &l_maxBuf) == ERROR_SUCCESS
		&& l_dwType == REG_SZ)
	{
		std::wstring l_tmpExePath = CUtilWin32::GetExePath();
		wchar_t l_regBuf[1002] = {0};

		wchar_t* l_args = wcsstr(l_buf, L" \"%1");
		if(l_args) *l_args = 0;

		// reg path: unquote spaces and get long path name
		PathUnquoteSpaces(l_buf);
		if(!GetLongPathName(l_buf, l_regBuf, 999))
			_tcsncpy_s(l_regBuf, 1002, l_buf, 1001);
		// reg path: l_buf --> l_regBuf

		// compare:
		if(_wcsicmp(l_tmpExePath.c_str(), l_regBuf) == 0)
		{
			l_result = true;
		}
	}

	RegCloseKey(l_hKey);

	return l_result;
}


bool CWin5xDefaultApp::RegisterProgIdData()
{
	_tstring l_keyPath = _T("SOFTWARE\\Classes\\") + m_appRegistryName + _T("\\DefaultIcon");

	_tstring l_exePath = CUtilWin32::GetExePath();

	HKEY l_hKey;
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		return false;
	}

	// set DefaultIcon
	_tstring l_defaultIconInfo = _T("\"") + l_exePath + _T("\", 0");
	RegSetValueEx(l_hKey, NULL, 0, REG_SZ, (LPBYTE)l_defaultIconInfo.c_str(), (DWORD)(l_defaultIconInfo.size() + 1) * sizeof(TCHAR));

	RegCloseKey(l_hKey);

	// now about shell\open\command...
	l_keyPath = _T("SOFTWARE\\Classes\\") + m_appRegistryName + _T("\\shell\\open\\command");

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		return false;
	}

	// set exe path
	_tstring l_shellOpenCommand = _T("\"") + l_exePath + _T("\" \"%1\"");
	RegSetValueEx(l_hKey, NULL, 0, REG_SZ, (LPBYTE)l_shellOpenCommand.c_str(), (DWORD)(l_shellOpenCommand.size() + 1) * sizeof(TCHAR));

	RegCloseKey(l_hKey);

	return true;
}


bool CWin5xDefaultApp::MakeDefault()
{
	if(!RegisterProgIdData())
	{
		return false;
	}

	HKEY l_hKey;
	_tstring l_keyPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\") + m_extension;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, KEY_ALL_ACCESS, &l_hKey) == ERROR_SUCCESS)
	{
		RegDeleteValue(l_hKey, _T("Progid"));
		RegCloseKey(l_hKey);
	}

	l_keyPath = _T("Software\\Classes\\") + m_extension;

	if(RegCreateKeyEx(HKEY_CURRENT_USER, l_keyPath.c_str(), 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &l_hKey, NULL) != ERROR_SUCCESS)
	{
		return false;
	}

	bool l_success = (RegSetValueEx(l_hKey,
		NULL, 0, REG_SZ, (LPBYTE)m_appRegistryName.c_str(),
		sizeof(TCHAR) * (DWORD)(m_appRegistryName.size() + 1))
		== ERROR_SUCCESS);

	if(l_success)
	{
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}

	return l_success;
}

#endif /* _WIN32_WINNT */
