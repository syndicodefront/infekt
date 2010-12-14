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

#include "updater.h"


/************************************************************************/
/* HELPER METHODS                                                       */
/************************************************************************/

bool IsOSSupported()
{
	OSVERSIONINFO l_info = { sizeof(OSVERSIONINFO), 0 };

	if(::GetVersionEx(&l_info))
	{
		if(l_info.dwMajorVersion > 5 ||
			(l_info.dwMajorVersion == 5 && l_info.dwMinorVersion >= 1))
		{
			return true;
		}
	}

	return false;
}


std::wstring GetSysDirPath()
{
	wchar_t l_buf[1000] = {0};

	if(::GetSystemDirectory(l_buf, 999))
	{
		::PathRemoveBackslash(l_buf);

		return l_buf;
	}

	return L"";
}


bool ShellExecuteAndWait(const std::wstring& a_path, const std::wstring& a_parameters, int nShowCmd)
{
	SHELLEXECUTEINFO l_sei = { sizeof(SHELLEXECUTEINFO), 0 };

	l_sei.lpVerb = L"open";
	l_sei.lpFile = a_path.c_str();
	l_sei.lpParameters = a_parameters.c_str();
	l_sei.nShow = nShowCmd;
	l_sei.fMask = SEE_MASK_NOCLOSEPROCESS;

	if(::ShellExecuteEx(&l_sei))
	{
		if(::WaitForSingleObject(l_sei.hProcess, 10000) == WAIT_OBJECT_0)
		{
			::CloseHandle(l_sei.hProcess);
		}
		// else: accept possible resource leak, but keep program responding

		return true;
	}

	return false;
}
