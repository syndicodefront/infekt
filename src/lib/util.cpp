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
#include "util.h"

using namespace std;

string CUtil::FromWideStr(const wstring& a_wideStr, unsigned int a_targetCodePage)
{
	int l_size = ::WideCharToMultiByte(a_targetCodePage, 0, a_wideStr.c_str(),
		-1, NULL, NULL, NULL, NULL);

	if(l_size)
	{
		char *l_buf = new char[l_size];

		if(l_buf)
		{
			int l_chrs = ::WideCharToMultiByte(a_targetCodePage, 0, a_wideStr.c_str(),
				-1, l_buf, l_size, NULL, NULL);

			string l_result(l_buf);
			delete[] l_buf;
			return l_result;
		}
	}

	return "";
}


wstring CUtil::ToWideStr(const string& a_str, unsigned int a_originCodePage)
{
	int l_size = ::MultiByteToWideChar(a_originCodePage, 0, a_str.c_str(), -1, NULL, NULL);

	if(l_size)
	{
		wchar_t *l_buf = new wchar_t[l_size];

		if(l_buf)
		{
			int l_chrs = ::MultiByteToWideChar(a_originCodePage, 0, a_str.c_str(), -1, l_buf, l_size);
			wstring l_result(l_buf);
			delete[] l_buf;
			return l_result;
		}
	}

	return L"";
}


bool CUtil::OneCharWideToUtf8(wchar_t a_char, char* a_buf)
{
	return (::WideCharToMultiByte(CP_UTF8, 0, &a_char, 1, a_buf, 7, NULL, NULL) > 0);
}
