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
	HWND A_NAME = ::CreateWindowEx(WS_EX_LEFT | WS_EX_NOPARENTNOTIFY, WC_STATIC, NULL, \
	WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT, l_left, A_TOP, 270, A_HEIGHT, \
		m_hWnd, NULL, g_hInstance, NULL); \
	{ const std::wstring l_tmp(A_TEXT); \
	::SetWindowText(A_NAME, l_tmp.c_str()); \
	::SendMessage(A_NAME, WM_SETFONT, (WPARAM)l_defaultFont, 1); }

#define _CREATE_SYSLINK(A_NAME, A_TEXT, A_TOP, A_HEIGHT) \
	HWND A_NAME = ::CreateWindowEx(0, L"SysLink", NULL, \
		WS_VISIBLE | WS_CHILD | WS_TABSTOP, \
		l_left, A_TOP, 280, A_HEIGHT, \
		m_hWnd, NULL, g_hInstance, NULL); \
		{ const std::_tstring l_tmp(A_TEXT); \
		::SetWindowText(A_NAME, l_tmp.c_str()); \
		::SendMessage(A_NAME, WM_SETFONT, (WPARAM)l_defaultFont, 1); }


BOOL CAboutDialog::OnInitDialog()
{
	SetIconLarge(IDI_APPICON);
	SetIconSmall(IDI_APPICON);

	m_icon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);

	HFONT l_defaultFont = (HFONT)SendMessage(WM_GETFONT);
	if(!l_defaultFont) l_defaultFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);

	const int l_left = 70;
	int l_top = 15;

	std::wstring l_verStr = L"iNFekt v" + m_mainWin->InfektVersionAsString();
#ifndef COMPACT_RELEASE
#ifdef _WIN64
	l_verStr += L" (64 bit)";
#else
	if(CUtilWin32::IsWow64())
		l_verStr += L" (WoW64)";
	else
		l_verStr += L" (32 bit)";
#endif
#endif
	const std::wstring winVerName = GetWindowsClientOSName();

	if (CUtilWin32::IsWinServerOS())
	{
		l_verStr += L" on Windows Server";

		if (!winVerName.empty())
			l_verStr += L" (like Win " + winVerName + L")";
	}
	else if (!winVerName.empty())
		l_verStr += L" on Windows " + winVerName;

	_CREATE_STATIC(l_hTitle, l_verStr, l_top, 20);
	l_top += 20;

#ifdef COMPACT_RELEASE
	_CREATE_STATIC(l_hSubTitle, L"Super Compact Version", l_top, 20);
	l_top += 20;
#endif

	if(l_hTitle)
	{
		LOGFONT l_tmpFont;
		::GetObject(l_defaultFont, sizeof(LOGFONT), &l_tmpFont);
		l_tmpFont.lfWeight = FW_BOLD;
		m_boldFont = ::CreateFontIndirect(&l_tmpFont);
		::SendMessage(l_hTitle, WM_SETFONT, (WPARAM)m_boldFont, 1);
#ifdef COMPACT_RELEASE
		if(l_hSubTitle) ::SendMessage(l_hSubTitle, WM_SETFONT, (WPARAM)m_boldFont, 1);
#endif
	}

	_CREATE_STATIC(l_hCopyright, L"\xA9 syndicode 2010-2015", l_top, 20);
	l_top += 20;

	_CREATE_SYSLINK(l_hHomepage, L"Project Homepage: <A HREF=\"http://infekt.ws/\">infekt.ws</A>", l_top, 20);
	m_linkCtrl = l_hHomepage;
	l_top += 20;

	const wchar_t* l_gpuFlag = L"no";

	if(CCairoBoxBlur::IsGPUUsable())
	{
		l_gpuFlag = CNFORenderer::GetGlobalUseGPUFlag() ? L"yes" : L"disabled";
	}

	_CREATE_STATIC(l_hLibVersions, FORMAT(L"Using Cairo v%d.%d.%d, PCRE v%d.%02d, GPU: %s",
		CAIRO_VERSION_MAJOR % CAIRO_VERSION_MINOR % CAIRO_VERSION_MICRO %
		PCRE_MAJOR % PCRE_MINOR % l_gpuFlag), l_top, 20);
	l_top += 20;

	_CREATE_STATIC(l_hGPL, L"This program is free software; you can redistribute it and/or "
		L"modify it under the terms of the GNU General Public License "
		L"as published by the Free Software Foundation.", l_top, 55);
	l_top += 60;

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
			::ShellExecute(NULL, L"open", L"http://infekt.ws/", NULL, NULL, SW_SHOWNORMAL);
			return 0;
		}
		break;
	}

	return CDialog::OnNotify(wParam, lParam);
}


/*static*/ std::wstring CAboutDialog::GetWindowsClientOSName()
{
	if(CUtilWin32::IsWinXP())
		return L"XP";
	else if(CUtilWin32::IsWinVista())
		return L"Vista";
	else if(CUtilWin32::IsWin7())
		return L"7";
	else if(CUtilWin32::IsWin8())
		return L"8";
	else if(CUtilWin32::IsWin10())
		return L"10";
	else
		return L"";
}


CAboutDialog::~CAboutDialog()
{
	if(m_icon) ::DestroyIcon(m_icon);
	if(m_boldFont) ::DeleteObject(m_boldFont);
}

