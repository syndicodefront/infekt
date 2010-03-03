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
#include <shlwapi.h>
#include <string>
#include "default_app.h"

bool CWin5xDefaultApp::IsDefault() const
{
	HKEY l_hKey;
	std::wstring l_keyPath = L"SOFTWARE\\Classes\\" + m_info.sExtension;

	/* phase 1: We check the "(Default)" value of the Classes\.nfo key for our ProgId */

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD l_dwType = REG_SZ, l_maxBuf = 999;
	wchar_t l_buf[1002] = {0};

	if(RegQueryValueEx(l_hKey, NULL, NULL, &l_dwType, (LPBYTE)l_buf, &l_maxBuf) == ERROR_SUCCESS)
	{
		if(wcscmp(l_buf, m_info.sAppRegistryName.c_str()) == 0)
		{
			RegCloseKey(l_hKey);

			/* phase 2: we found the ProgId, now check the path to our app */

			l_keyPath = L"SOFTWARE\\Classes\\" + m_info.sAppRegistryName + L"\\shell\\open\\command";

			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, l_keyPath.c_str(), 0, KEY_QUERY_VALUE, &l_hKey) != ERROR_SUCCESS)
			{
				return false;
			}

			bool l_result = false;

			if(RegQueryValueEx(l_hKey, NULL, NULL, &l_dwType, (LPBYTE)l_buf, &l_maxBuf) == ERROR_SUCCESS)
			{
				std::wstring l_tmpExePath = this->GetExePath();
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


#ifdef MAKE_DEFAULT_APP_CAPAB

bool CWin5xDefaultApp::MakeDefault() const
{
	return false;
}

#endif
