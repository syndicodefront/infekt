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
#include "about_dlg.h"
#include "resource.h"
#include "app.h"


CAboutDialog::CAboutDialog(HWND hWndParent) :
	CDialog(IDD_ABOUT, hWndParent),
	m_mainWin(NULL), m_boldFont(NULL), m_icon(NULL),
	m_linkCtrl((HWND)-1)
{
}


#define _CREATE_STATIC(A_NAME, A_TEXT, A_TOP, A_HEIGHT) \
	HWND A_NAME = CreateWindowEx(WS_EX_LEFT | WS_EX_NOPARENTNOTIFY, WC_STATIC, NULL, \
	WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT, l_left, A_TOP, 270, A_HEIGHT, \
		m_hWnd, NULL, g_hInstance, NULL); \
	{ std::_tstring l_tmp = (A_TEXT); \
	::SetWindowText(A_NAME, l_tmp.c_str()); \
	::SendMessage(A_NAME, WM_SETFONT, (WPARAM)l_defaultFont, 1); }

#define _CREATE_SYSLINK(A_NAME, A_TEXT, A_TOP, A_HEIGHT) \
	HWND A_NAME = CreateWindowEx(0, _T("SysLink"), NULL, \
		WS_VISIBLE | WS_CHILD | WS_TABSTOP, \
		l_left, A_TOP, 280, A_HEIGHT, \
		m_hWnd, NULL, g_hInstance, NULL); \
		{ std::_tstring l_tmp = (A_TEXT); \
		::SetWindowText(A_NAME, l_tmp.c_str()); \
		::SendMessage(A_NAME, WM_SETFONT, (WPARAM)l_defaultFont, 1); }


BOOL CAboutDialog::OnInitDialog()
{
	SetIconLarge(IDI_APPICON);
	SetIconSmall(IDI_APPICON);

	m_icon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);

	HFONT l_defaultFont = (HFONT)SendMessage(WM_GETFONT);
	if(!l_defaultFont) l_defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	const int l_left = 70;
	int l_top = 15;

	std::_tstring l_verStr = _T("iNFekt v") + m_mainWin->InfektVersionAsString();
#ifndef COMPACT_RELEASE
#ifdef _WIN64
	l_verStr += _T(" (64 bit)");
#else
	if(CUtil::IsWow64())
		l_verStr += _T(" (32 bit emulation)");
	else
		l_verStr += _T(" (32 bit)");
#endif
#endif
	if (CUtil::IsWinServerOS())
		l_verStr += _T(" on Windows Server OS");
	else if(CUtil::IsWinXP())
		l_verStr += _T(" on Windows XP");
	else if(CUtil::IsWinVista())
		l_verStr += _T(" on Windows Vista");
	else if(CUtil::IsWin7())
		l_verStr += _T(" on Windows 7");
	else if(CUtil::IsWin8())
		l_verStr += _T(" on Windows 8");
	else if(CUtil::IsWin81())
		l_verStr += _T(" on Windows 8.1");

	_CREATE_STATIC(l_hTitle, l_verStr, l_top, 20);
	l_top += 20;

#ifdef COMPACT_RELEASE
	_CREATE_STATIC(l_hSubTitle, L"Super Compact Version", l_top, 20);
	l_top += 20;
#endif

	if(l_hTitle)
	{
		LOGFONT l_tmpFont;
		GetObject(l_defaultFont, sizeof(LOGFONT), &l_tmpFont);
		l_tmpFont.lfWeight = FW_BOLD;
		m_boldFont = CreateFontIndirect(&l_tmpFont);
		::SendMessage(l_hTitle, WM_SETFONT, (WPARAM)m_boldFont, 1);
#ifdef COMPACT_RELEASE
		if(l_hSubTitle) ::SendMessage(l_hSubTitle, WM_SETFONT, (WPARAM)m_boldFont, 1);
#endif
	}

	_CREATE_STATIC(l_hCopyright, _T("\xA9 cxxjoe && Contributors 2010-2013"), l_top, 20);
	l_top += 20;

	_CREATE_SYSLINK(l_hHomepage, _T("Project Homepage: <A HREF=\"http://infekt.googlecode.com/\">infekt.googlecode.com</A>"), l_top, 20);
	m_linkCtrl = l_hHomepage;
	l_top += 20;

	const char* l_gpuFlag = "no";

	if(CUtil::IsWin6x() && CNFORenderer::GetGlobalUseGPUFlag())
	{
		HMODULE hGpuDll = CUtil::SilentLoadLibrary(CUtil::GetExeDir() + L"\\infekt-gpu.dll");

		typedef int (__cdecl *fnc)();

		if(fnc igu = (fnc)::GetProcAddress(hGpuDll, "IsGpuUsable"))
		{
			if(igu())
				l_gpuFlag = "yes";
		}
	}

	_CREATE_STATIC(l_hLibVersions, FORMAT(_T("Using Cairo v%d.%d.%d, PCRE v%d.%02d, GPU: %s"),
		CAIRO_VERSION_MAJOR % CAIRO_VERSION_MINOR % CAIRO_VERSION_MICRO %
		PCRE_MAJOR % PCRE_MINOR % l_gpuFlag), l_top, 20);
	l_top += 20;

	_CREATE_STATIC(l_hGPL, _T("This program is free software; you can redistribute it and/or ")
		_T("modify it under the terms of the GNU General Public License ")
		_T("as published by the Free Software Foundation."), l_top, 55);
	l_top += 60;

#ifndef COMPACT_RELEASE
	if(CUtil::IsWin6x())
	{
		_CREATE_STATIC(l_hGreetings, _T("Rebecca, you are the love of my life. \x2764"), l_top, 20);
	}
	else
	{
		_CREATE_STATIC(l_hGreetings, _T("Rebecca, you are the love of my life. <3"), l_top, 20);
	}
#endif

	return TRUE;
}


BOOL CAboutDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(GetHwnd(), &ps);
		::DrawIconEx(hdc, 10, 15, m_icon, 48, 48, 0, NULL, DI_NORMAL);
		::EndPaint(GetHwnd(), &ps);
		return TRUE; }
	}

	return this->DialogProcDefault(uMsg, wParam, lParam);
}


LRESULT CAboutDialog::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR nh = (LPNMHDR)lParam;
	switch(nh->code)
	{
	case NM_CLICK:
	case NM_RETURN:
		if(nh->hwndFrom == m_linkCtrl)
		{
			::ShellExecute(NULL, _T("open"), _T("http://infekt.googlecode.com/"), NULL, NULL, SW_SHOWNORMAL);
			return 0;
		}
		break;
	}

	return CDialog::OnNotify(wParam, lParam);
}


CAboutDialog::~CAboutDialog()
{
	if(m_icon) ::DestroyIcon(m_icon);
	if(m_boldFont) ::DeleteObject(m_boldFont);
}

