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

#include "stdafx.h"
#include "util_win32.h"

std::wstring CUtilWin32::PathRemoveFileSpec(const std::wstring& a_path)
{
	TCHAR* l_buf = new TCHAR[a_path.size() + 1];
	memset(l_buf, 0, a_path.size() + 1);

	wcscpy_s(l_buf, a_path.size() + 1, a_path.c_str());

	::PathRemoveFileSpec(l_buf);
	::PathRemoveBackslash(l_buf);

	std::wstring l_result(l_buf);
	delete[] l_buf;

	return l_result;
}


std::wstring CUtilWin32::PathRemoveExtension(const std::wstring& a_path)
{
	TCHAR* l_buf = new TCHAR[a_path.size() + 1];
	memset(l_buf, 0, a_path.size() + 1);

	wcscpy_s(l_buf, a_path.size() + 1, a_path.c_str());

	::PathRemoveExtension(l_buf);

	std::wstring l_result(l_buf);
	delete[] l_buf;

	return l_result;
}


std::wstring CUtilWin32::GetTempDir()
{
	wchar_t l_tmpPathBuf[1000] = {0};

	if(::GetTempPath(999, l_tmpPathBuf))
	{
		::PathAddBackslash(l_tmpPathBuf);

		return l_tmpPathBuf;
	}

	return L"";
}


std::wstring CUtilWin32::GetAppDataDir(bool a_local, const std::wstring& a_appName)
{
	wchar_t l_tmpPathBuf[1000] = {0};

	if(::SHGetFolderPath(0, a_local ? CSIDL_LOCAL_APPDATA : CSIDL_APPDATA, NULL,
		SHGFP_TYPE_CURRENT, l_tmpPathBuf) == S_OK)
	{
		::PathAddBackslash(l_tmpPathBuf);

		::PathAppend(l_tmpPathBuf, a_appName.c_str());

		if(!::PathIsDirectory(l_tmpPathBuf))
		{
			::SHCreateDirectoryEx(NULL, l_tmpPathBuf, NULL);
		}

		if(::PathIsDirectory(l_tmpPathBuf))
		{
			::PathAddBackslash(l_tmpPathBuf);

			return l_tmpPathBuf;
		}
	}

	return L"";
}


HMODULE CUtilWin32::SilentLoadLibrary(const std::wstring& a_path)
{
	_ASSERT(!PathIsRelative(a_path.c_str()));

	HMODULE l_hResult = NULL;
	DWORD dwErrorMode = SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS;

	if(CUtilWin32::IsAtLeastWin7())
	{
		// BOOL SetThreadErrorMode(DWORD dwNewMode, LPDWORD lpOldMode);
		typedef BOOL (WINAPI *fstem)(DWORD, LPDWORD);

		fstem fnc = (fstem)::GetProcAddress(::GetModuleHandle(L"Kernel32.dll"), "SetThreadErrorMode");

		if(fnc)
		{
			DWORD l_oldErrorMode = 0;
			fnc(dwErrorMode, &l_oldErrorMode);
			l_hResult = ::LoadLibrary(a_path.c_str());
			fnc(l_oldErrorMode, NULL);
		}
	}
	else
	{
		UINT l_oldErrorMode = ::SetErrorMode(dwErrorMode);
		l_hResult = ::LoadLibrary(a_path.c_str());
		::SetErrorMode(l_oldErrorMode);
	}

	return l_hResult;
}


bool CUtilWin32::RemoveCwdFromDllSearchPath()
{
	return (::SetDllDirectory(L"") != FALSE || ::GetLastError() == ERROR_SUCCESS);
}


std::wstring CUtilWin32::GetExePath()
{
	TCHAR l_buf[1000] = {0};
	TCHAR l_buf2[1000] = {0};

	::GetModuleFileName(NULL, (LPTCH)l_buf, 999);
	::GetLongPathName(l_buf, l_buf2, 999);

	return l_buf2;
}


std::wstring CUtilWin32::GetExeDir()
{
	TCHAR l_buf[1000] = {0};
	TCHAR l_buf2[1000] = {0};

	::GetModuleFileName(NULL, (LPTCH)l_buf, 999);
	::GetLongPathName(l_buf, l_buf2, 999);
	::PathRemoveFileSpec(l_buf2);
	::PathRemoveBackslash(l_buf2);

	return l_buf2;
}


bool CUtilWin32::HardenHeap()
{
#ifndef _DEBUG
	// Activate program termination on heap corruption.
	// http://msdn.microsoft.com/en-us/library/aa366705%28VS.85%29.aspx
	typedef BOOL (WINAPI *fhsi)(HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T);
	fhsi l_fHSI = (fhsi)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "HeapSetInformation");
	if(l_fHSI)
	{
		return (l_fHSI(GetProcessHeap(), HeapEnableTerminationOnCorruption, NULL, 0) != FALSE);
	}
#endif
	return false;
}

#ifndef PROCESS_DEP_ENABLE
#define PROCESS_DEP_ENABLE 0x01
#endif

bool CUtilWin32::EnforceDEP()
{
#ifndef _WIN64
	// Explicitly activate DEP, especially important for XP SP3.
	// http://msdn.microsoft.com/en-us/library/bb736299%28VS.85%29.aspx
	typedef BOOL (WINAPI *fspdp)(DWORD);
	fspdp l_fSpDp = (fspdp)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "SetProcessDEPPolicy");
	if(l_fSpDp)
	{
		return (l_fSpDp(PROCESS_DEP_ENABLE) != FALSE);
	}

	return false;
#else
	return true; // always enabled on x64 anyway.
#endif
}


/************************************************************************/
/* Windows OS Version Helper Functions                                  */
/************************************************************************/

static bool IS_WIN_XX(DWORD MAJ, DWORD MIN, bool OR_HIGHER)
{
	OSVERSIONINFOEXW osvi = { sizeof(OSVERSIONINFOEXW), 0 };
	DWORDLONG dwlConditionMask = 0;
	
	osvi.dwMajorVersion = MAJ;
	osvi.dwMinorVersion = MIN;
	
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, OR_HIGHER ? VER_GREATER_EQUAL : VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, OR_HIGHER ? VER_GREATER_EQUAL : VER_EQUAL);
	
	return ::VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) == TRUE;
}

bool CUtilWin32::IsWinXP()
{
	static bool is = IS_WIN_XX(5, 1, false)
		|| IS_WIN_XX(5, 2, false); // Server 2003

	return is;
}

bool CUtilWin32::IsAtLeastWinVista()
{
	static bool is = IS_WIN_XX(6, 0, true);

	return is;
}

bool CUtilWin32::IsWinVista()
{
	static bool is = IS_WIN_XX(6, 0, false);

	return is;
}

bool CUtilWin32::IsAtLeastWin7()
{
	static bool is = IS_WIN_XX(6, 1, true);

	return is;
}

bool CUtilWin32::IsWin7()
{
	static bool is = IS_WIN_XX(6, 1, false);

	return is;
}

bool CUtilWin32::IsWin8()
{
	static bool is = IS_WIN_XX(6, 2, false) || IS_WIN_XX(6, 3, false);

	return is;
}

bool CUtilWin32::IsWin10()
{
	static bool is = IS_WIN_XX(10, 0, true);

	return is;
}

bool CUtilWin32::IsWinServerOS()
{
	OSVERSIONINFOEXW osvi = { sizeof(OSVERSIONINFOEXW), 0 };
	DWORDLONG dwlConditionMask = 0;
	
	osvi.wProductType = VER_NT_SERVER;
	
	VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
	
	return ::VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask) == TRUE;
}

bool CUtilWin32::IsWow64()
{
	typedef BOOL (WINAPI *fiw6p)(HANDLE, PBOOL);

	fiw6p l_fiw6p = (fiw6p)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "IsWow64Process");

	if(l_fiw6p)
	{
		BOOL l_bIsWow64;

		if(l_fiw6p(GetCurrentProcess(), &l_bIsWow64))
		{
			return (l_bIsWow64 != FALSE);
		}
	}

	return false;
}


/************************************************************************/
/* WINDOWS BENCHMARK/HIGH RESOLUTION TIMER                              */
/************************************************************************/

CBenchmarkTimer::CBenchmarkTimer()
{
	memset(&m_start, 0, sizeof(LARGE_INTEGER));
	memset(&m_stop, 0, sizeof(LARGE_INTEGER));
	m_frequency = GetFrequency() / 1000.0f; // milliseconds
}

double CBenchmarkTimer::GetFrequency()
{
	LARGE_INTEGER l_freq;

	::QueryPerformanceFrequency(&l_freq);

	return static_cast<double>(l_freq.QuadPart);
}

void CBenchmarkTimer::StartTimer()
{
	DWORD_PTR l_oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);

	::QueryPerformanceCounter(&m_start);

	::SetThreadAffinityMask(::GetCurrentThread(), l_oldmask);
}

double CBenchmarkTimer::StopTimer(void)
{
	DWORD_PTR l_oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);

	::QueryPerformanceCounter(&m_stop);

	::SetThreadAffinityMask(::GetCurrentThread(), l_oldmask);

	return ((m_stop.QuadPart - m_start.QuadPart) / m_frequency);
}

double CBenchmarkTimer::StopDumpTimer(const char* a_name)
{
	double l_secs = StopTimer();
	char l_buf[256] = {0};

	sprintf_s(l_buf, 255, "BenchmarkTimer: [%s] %.2f msec\r\n", a_name, l_secs);

	::OutputDebugStringA(l_buf);

	return l_secs;
}
