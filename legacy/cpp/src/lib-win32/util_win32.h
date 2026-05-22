/**
 * Copyright (C) 2010-2014 syndicode
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

#ifndef _UTIL_WIN32_H
#define _UTIL_WIN32_H

#include <string>
#include <Windows.h>

class CUtilWin32
{
public:
	static bool IsWow64();

	static std::wstring GetExePath();
	static std::wstring GetExeDir();
	static std::wstring PathRemoveFileSpec(const std::wstring& a_path);
	static std::wstring PathRemoveExtension(const std::wstring& a_path);
	static std::wstring GetTempDir();
	static std::wstring GetAppDataDir(bool a_local, const std::wstring& a_appName);
	static HMODULE SilentLoadLibrary(const std::wstring& a_path);

	static bool RemoveCwdFromDllSearchPath();
	static bool HardenHeap();
};

class CBenchmarkTimer
{
public:
	CBenchmarkTimer();

	double GetFrequency();
	void StartTimer();
	double StopTimer();
	double StopDumpTimer(const char* a_name);

protected:
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_stop;
	double m_frequency;
};

#endif /* !_UTIL_WIN32_H */
