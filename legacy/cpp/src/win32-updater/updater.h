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

#define _TARGETVER_WINXP
#include "targetver.h"

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <process.h>
#include <locale.h>

#include <string>

#include "resource.h"


// constants:
#define WM_DOWNLOAD_FAILED (WM_USER + 9)
#define WM_DOWNLOAD_STARTED (WM_USER + 10)
#define WM_DOWNLOAD_COMPLETE (WM_USER + 11)
#define WM_TASKKILL_COMPLETE (WM_USER + 13)
#define WM_INSTALLER_COMPLETE (WM_USER + 14)

#define IDT_TIMER_ID 1337

// helper, utility and meat methods:
std::wstring GetSysDirPath();
std::wstring GetTempFilePath(const std::wstring& a_suffix);
std::wstring GetExePath();
bool ShellExecuteAndWait(const std::wstring& a_path, const std::wstring& a_parameters, int nShowCmd,
						 bool a_requireZeroExitCode = false, DWORD dwMaxWait = 10000);

std::wstring SHA1_File(const std::wstring& a_filePath);

bool StartHttpDownload(HWND hDlg, const std::wstring& a_url, const std::wstring& a_localPath);
__int64 HttpGetBytesReceived();
bool HttpIsDownloading();

bool StartTaskKill(HWND hDlg);
bool StartInstaller(HWND hDlg, const std::wstring& a_installerPath);

// global vars:
extern HINSTANCE g_hInstance;
