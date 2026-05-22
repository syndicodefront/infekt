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

#ifndef _UTIL_WIN32_GUI_H
#define _UTIL_WIN32_GUI_H

#if _WIN32_WINNT < 0x600
/* just make it compile! */
typedef struct _COMDLG_FILTERSPEC {
	LPCWSTR pszName;
	LPCWSTR pszSpec;
} COMDLG_FILTERSPEC;
#endif

class CUtilWin32GUI
{
public:
	static int AddPngToImageList(HIMAGELIST a_imgList, HINSTANCE a_instance, int a_resourceId, int a_width, int a_height);
	static std::wstring OpenFileDialog(HINSTANCE a_instance, HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec);
	static std::wstring SaveFileDialog(HINSTANCE a_instance, HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec,
		const LPCTSTR a_defaultExt, const std::wstring& a_currentFileName, const std::wstring& a_initialPath = _T(""));
	static void FormatFileTimeSize(const std::wstring& a_filePath, std::wstring& ar_timeInfo, std::wstring& ar_sizeInfo);

	static void PopUpLastWin32Error();
	static bool TextToClipboard(HWND a_hwnd, const std::wstring& a_text);

	static int StatusCalcPaneWidth(HWND hwnd, LPCTSTR lpsz);
	static BOOL GenericOnSetCursor(const LPTSTR a_cursor, LPARAM lParam);
};

/* Win32++ helpers */

class CNonThemedTab : public CTab
{
protected:
	virtual inline LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if(uMsg != WM_PAINT && uMsg != WM_ERASEBKGND)
		{
			return CTab::WndProcDefault(uMsg, wParam, lParam);
		}
		else
		{
			return CWnd::WndProcDefault(uMsg, wParam, lParam);
		}
	}

public:
	virtual inline int AddTabPage(CWnd* pWnd, LPCTSTR szTitle, HICON hIcon = 0)
	{
		int l_newIdx = CTab::AddTabPage(pWnd, szTitle, hIcon);

		LONG_PTR l_style = this->GetWindowLongPtr(GWL_STYLE);
		if((l_style & TCS_OWNERDRAWFIXED) != 0)
		{
			this->SetWindowLongPtr(GWL_STYLE, l_style & ~TCS_OWNERDRAWFIXED);
		}

		if(this->SendMessage(CCM_GETVERSION, 0, 0) >= 6)
		{
			// adjust XP style background...
			::EnableThemeDialogTexture(GetTabPageInfo(l_newIdx).pWnd->GetHwnd(), ETDT_ENABLETAB);
		}

		return l_newIdx;
	}
};

#endif /* !_UTIL_WIN32_GUI_H */
