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
#include <string>
#include "default_app.h"

#ifdef MAKE_DEFAULT_APP_CAPAB

bool CWinDefaultApp::RegisterProgIdData() const
{
	return false;
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
