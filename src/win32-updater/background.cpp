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
/* SOME STATIC VARS                                                     */
/************************************************************************/

static HWND s_hDlg = 0;

/************************************************************************/
/* TASK KILL JOB                                                        */
/************************************************************************/

static void __cdecl TaskKillThread(void *pvStartupInfo)
{
	HANDLE hStartEvent = static_cast<HANDLE>(pvStartupInfo);

	HWND l_hDlg = s_hDlg;

	::SetEvent(hStartEvent);

	// Terminate any running iNFekt exe instances (using taskkill.exe):

	std::wstring l_taskKillExePath = GetSysDirPath() + L"\\taskkill.exe";

	ShellExecuteAndWait(l_taskKillExePath.c_str(), L"/IM infekt-win32.exe", SW_HIDE);
	ShellExecuteAndWait(l_taskKillExePath.c_str(), L"/IM infekt-cmd.exe", SW_HIDE);
	ShellExecuteAndWait(l_taskKillExePath.c_str(), L"/IM infekt-win64.exe", SW_HIDE);

	// small pause fwiw:
	Sleep(1250);

	SendMessage(l_hDlg, WM_TASKKILL_COMPLETE, 0, 0);
}


bool StartTaskKill(HWND hDlg)
{
	s_hDlg = hDlg;

	HANDLE hStartEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	if(_beginthread(TaskKillThread, 0, hStartEvent))
	{
		::WaitForSingleObject(hStartEvent, INFINITE);
	}

	CloseHandle(hStartEvent);

	return true;
}

