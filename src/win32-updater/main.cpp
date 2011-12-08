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

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


/************************************************************************/
/* STATIC VARS                                                          */
/************************************************************************/

static __int64 s_uBytesTotal = 0;
static DWORD s_uTimeDlStarted = 0;

static std::wstring s_installerUrl;
static std::wstring s_installerHash;
static std::wstring s_installerPath;


/************************************************************************/
/* WINDOW MESSAGE METHODS                                               */
/************************************************************************/

static void EnablePgbMarquee(HWND hDlg)
{
	HWND hPgb = ::GetDlgItem(hDlg, IDC_PGB);

	if((GetWindowLongPtr(hPgb, GWL_STYLE) & PBS_MARQUEE) == 0)
	{
		SetWindowLongPtr(hPgb, GWL_STYLE, GetWindowLongPtr(hPgb, GWL_STYLE) | PBS_MARQUEE);
	}

	SendMessage(hPgb, PBM_SETMARQUEE, 1, 50);
}


static void DisablePgbMarquee(HWND hDlg)
{
	HWND hPgb = ::GetDlgItem(hDlg, IDC_PGB);

	SendMessage(hPgb, PBM_SETMARQUEE, 0, 0);

	if(GetWindowLongPtr(hPgb, GWL_STYLE) & PBS_MARQUEE)
	{
		SetWindowLongPtr(hPgb, GWL_STYLE, GetWindowLongPtr(hPgb, GWL_STYLE) & ~PBS_MARQUEE);
	}

	SendMessage(hPgb, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
}


static void InitialWindowSetup(HWND hDlg)
{
	HICON hIcon = ::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APPICON));
	::SendMessage(hDlg, WM_SETICON, WPARAM(ICON_SMALL), LPARAM(hIcon));

	SetDlgItemText(hDlg, IDC_STATUS, L"Initializing...");
}


static void BeginUpdate(HWND hDlg)
{
	EnablePgbMarquee(hDlg);

	std::wstring l_tempExePath = GetTempFilePath(L"-iNFekt-setup.exe");

	if(!StartHttpDownload(hDlg, s_installerUrl, l_tempExePath))
	{
		SetDlgItemText(hDlg, IDC_STATUS, L"ERROR: Couldn't initialize download.");
	}
	else
	{
		SetDlgItemText(hDlg, IDC_STATUS, L"Downloading...");

		s_installerPath = l_tempExePath;

		::EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);
	}
}


void ShowDownloadProgress(HWND hDlg)
{
	wchar_t wszBuf[1024];

	__int64 bytesReceived = HttpGetBytesReceived();

	double secsIn = ((GetTickCount() - s_uTimeDlStarted) / 1000.0);
	double fBytesPerSec = bytesReceived / secsIn;

	if(s_uBytesTotal > 0)
	{
		unsigned int etaH = 0, etaM = 0, etaS = (unsigned int)(s_uBytesTotal / fBytesPerSec - secsIn) + 1;
		if(etaS < 0) etaS = 0;
		if(etaS >= 60)
		{
			etaM = etaS / 60;
			etaS = etaS % 60;
		}
		if(etaM >= 60)
		{
			etaH = etaM / 60;
			etaM = etaM % 60;
		}

		swprintf_s(wszBuf, 1023, L"Received %.1f KB of %.1f KB / ETA at %.0f KB/s: %02u:%02u:%02u",
			bytesReceived / 1024.0, s_uBytesTotal / 1024.0, fBytesPerSec / 1024, etaH, etaM, etaS);

		SendDlgItemMessage(hDlg, IDC_PGB, PBM_SETPOS, (int)(bytesReceived / (double)s_uBytesTotal * 1000), 0);
	}
	else
	{
		swprintf_s(wszBuf, 1023, L"File size unknown / Received %.1f KB / ETA at %.0f KB/s: UNKNOWN",
			bytesReceived / 1024.0, fBytesPerSec / 1024);
	}

	SetDlgItemText(hDlg, IDC_STATUS, wszBuf);
}


static void ShowDownloadStarted(HWND hDlg, __int64 uBytesTotal)
{
	s_uBytesTotal = uBytesTotal;
	s_uTimeDlStarted = GetTickCount();

	DisablePgbMarquee(hDlg);

	ShowDownloadProgress(hDlg);
}


static bool ValidateDownloadedFile()
{
	std::wstring l_fileHash = SHA1_File(s_installerPath);

	return (_wcsicmp(l_fileHash.c_str(), s_installerHash.c_str()) == 0);
}


static void OnInstallerComplete(HWND hDlg, bool a_success)
{
	// schedule temp files for deletion on reboot:
	std::wstring l_exePath = GetExePath();
	// delete this exe, remember it should have been copied to temp dir
	::MoveFileEx(l_exePath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	// delete downloaded installer
	::MoveFileEx(s_installerPath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

	DisablePgbMarquee(hDlg);
	SendDlgItemMessage(hDlg, IDC_PGB, PBM_SETPOS, 1000, 0);

	if(a_success)
	{
		SetDlgItemText(hDlg, IDC_STATUS, L"Update has been installed!");

		::MessageBoxW(hDlg, L"Update complete!", L"Great Success", MB_ICONINFORMATION);
	}
	else
	{
		SetDlgItemText(hDlg, IDC_STATUS, L"The update installation reported a problem!");

		::MessageBoxW(hDlg, L"The update process reported a problem. Please consider removing and re-installing the program manually.", L"Warning", MB_ICONEXCLAMATION);
	}

	::DestroyWindow(hDlg);
}


static BOOL CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		InitialWindowSetup(hDlg);

		::SetTimer(hDlg, IDT_TIMER_ID, 100, NULL);

		::ShowWindow(hDlg, SW_SHOWNORMAL);

		BeginUpdate(hDlg);
		return TRUE;

	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL)
		{
			::PostMessage(hDlg, WM_CLOSE, 0, 0);
		}
		return TRUE;

	case WM_DOWNLOAD_STARTED:
		ShowDownloadStarted(hDlg, *reinterpret_cast<__int64*>(wParam));
		return TRUE;

	case WM_DOWNLOAD_FAILED:
		SendDlgItemMessage(hDlg, IDC_PGB, PBM_SETPOS, 0, 0);
		SetDlgItemText(hDlg, IDC_STATUS, L"Download failed! Please update manually or try again later.");
		return TRUE;

	case WM_DOWNLOAD_COMPLETE:
		if(ValidateDownloadedFile())
		{
			EnablePgbMarquee(hDlg);

			SetDlgItemText(hDlg, IDC_STATUS, L"Preparing to install update...");

			::EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);

			StartTaskKill(hDlg);
		}
		else
		{
			SendDlgItemMessage(hDlg, IDC_PGB, PBM_SETPOS, 0, 0);
			SetDlgItemText(hDlg, IDC_STATUS, L"Validating the downloaded file failed! Please update manually.");
		}
		return TRUE;

	case WM_TASKKILL_COMPLETE:
		SetDlgItemText(hDlg, IDC_STATUS, L"Installing update...");

		StartInstaller(hDlg, s_installerPath);
		return TRUE;

	case WM_INSTALLER_COMPLETE:
		OnInstallerComplete(hDlg, wParam == 0);
		return TRUE;

	case WM_TIMER:
		if(wParam == IDT_TIMER_ID && HttpIsDownloading())
		{
			ShowDownloadProgress(hDlg);
		}
		return TRUE;

	case WM_DESTROY:
		::KillTimer(hDlg, IDT_TIMER_ID);
		::PostQuitMessage(0);
		return TRUE;

	case WM_CLOSE:
		if(!HttpIsDownloading() || MessageBox(hDlg,
			L"Warning: A download is running and will be aborted if you continue! Do you want to close the program?", L"Confirm", MB_ICONQUESTION | MB_YESNO)
			== IDYES)
		{
			::DestroyWindow(hDlg);
		}
		return TRUE;
	}

	return FALSE;

}


static WPARAM MainMessageLoop()
{
	MSG msg;
	HWND hDlg;
	int iStatus;

	hDlg = ::CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DLGMAIN), NULL, DialogProc);
	if(hDlg == NULL)
		return 1;

	while((iStatus = ::GetMessage(&msg, 0, 0, 0)) != 0)
	{
		if(iStatus == -1)
			return -1;

		if(!::IsDialogMessage(hDlg, &msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}


/************************************************************************/
/* APP ENTRY POINT                                                      */
/************************************************************************/

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR wszCommandLine, int nShowCmd)
{
	g_hInstance = hInstance;

	_wsetlocale(LC_CTYPE, L"C");

	// command line format has to be "<SETUPURL> <SETUPHASH>".

	std::wstring l_cmdLine(wszCommandLine);
	std::wstring::size_type l_splitPos = l_cmdLine.find(' ');

	if(l_splitPos == std::wstring::npos)
	{
		return 1;
	}

	std::wstring l_url = l_cmdLine.substr(0, l_splitPos),
		l_hash = l_cmdLine.substr(l_splitPos + 1);
	
	LPWSTR l_quotedUrl = _wcsdup(l_url.c_str());
	::PathUnquoteSpaces(l_quotedUrl);
	l_url = l_quotedUrl;
	free(l_quotedUrl);

	if(::PathIsURL(l_url.c_str()))
	{
		s_installerUrl = l_url;
		s_installerHash = l_hash;

		if(s_installerUrl.find(L"https://") == 0 || s_installerUrl.find(L"http://") == 0)
		{
			InitCommonControls();

			return MainMessageLoop();
		}
	}

	return 1;
}


/************************************************************************/
/* GLOBAL VARS                                                          */
/************************************************************************/

HINSTANCE g_hInstance;

