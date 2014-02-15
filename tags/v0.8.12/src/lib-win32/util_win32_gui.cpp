/**
 * Copyright (C) 2010-2014 cxxjoe
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
#include "util_win32_gui.h"

using std::string;
using std::wstring;


/************************************************************************/
/* PNG/BITMAP/GDI Helper Functions                                      */
/************************************************************************/

/* utilities for AddPngToImageList */
struct _buf_info
{
	size_t len, pos;
	void *data;
};

static cairo_status_t _read_from_resource(void *a_closure, unsigned char *a_data, unsigned int a_length)
{
	_buf_info* l_buf = static_cast<_buf_info*>(a_closure);

	size_t l_bytes =
		(l_buf->pos + a_length > l_buf->len ? l_buf->len - l_buf->pos : a_length);

	memmove_s(a_data, a_length, (void*)((intptr_t)l_buf->data + (intptr_t)l_buf->pos), l_bytes);

	l_buf->pos += l_bytes;

	return CAIRO_STATUS_SUCCESS;
}

int CUtilWin32GUI::AddPngToImageList(HIMAGELIST a_imgList,
	HINSTANCE a_instance, int a_resourceId, int a_width, int a_height)
{
	HRSRC l_res = ::FindResource(a_instance, MAKEINTRESOURCE(a_resourceId), L"PNG");
	if(!l_res) return -1;

	HGLOBAL l_hr = ::LoadResource(a_instance, l_res);
	if(!l_hr) return -1;

	_buf_info l_buf;
	l_buf.len = ::SizeofResource(a_instance, l_res);
	l_buf.pos = 0;
	l_buf.data = ::LockResource(l_hr);

	if(!l_buf.data) return -1;

	cairo_surface_t* l_surfacePng =
		cairo_image_surface_create_from_png_stream(&_read_from_resource, &l_buf);

	UnlockResource(l_hr);

	if(!l_surfacePng) return -1;

	int l_resultId = -1;

	BITMAPINFO l_bi = {0};
	l_bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	l_bi.bmiHeader.biWidth = a_width;
	l_bi.bmiHeader.biHeight = -a_height;
	l_bi.bmiHeader.biPlanes = 1;
	l_bi.bmiHeader.biBitCount = 32;
	l_bi.bmiHeader.biYPelsPerMeter = l_bi.bmiHeader.biXPelsPerMeter = 1000;

	unsigned char* l_rawData;
	HBITMAP l_hBitmap = ::CreateDIBSection(NULL, &l_bi, DIB_RGB_COLORS, (void**)&l_rawData, NULL, 0);

	if(l_hBitmap)
	{
		BITMAP l_bitmap = {0};
		::GetObject(l_hBitmap, sizeof(BITMAP), &l_bitmap);
		cairo_surface_t* l_surfaceOut = cairo_image_surface_create_for_data(
			l_rawData, CAIRO_FORMAT_ARGB32, a_width, a_height, l_bitmap.bmWidthBytes);

		if(l_surfaceOut)
		{
			cairo_t *cr = cairo_create(l_surfaceOut);

			// copy PNG to bitmap surface:
			cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
			cairo_set_source_surface(cr, l_surfacePng, 0, 0);
			cairo_paint(cr);
			cairo_destroy(cr);

			// finally add to image list:
			l_resultId = ImageList_Add(a_imgList, l_hBitmap, NULL);

			cairo_surface_destroy(l_surfaceOut);
		}

		::DeleteObject(l_hBitmap);
	}

	cairo_surface_destroy(l_surfacePng);

	return l_resultId;
}


/************************************************************************/
/* Common Dialog Helper Functions                                       */
/************************************************************************/

// from file_dialogs_win6x.cpp:
wstring Win6x_OpenFileDialog(HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec);
wstring Win6x_SaveFileDialog(HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec,
	const LPCWSTR a_defaultExt, const wstring& a_currentFileName, const wstring& a_initialPath);


#if _WIN32_WINNT < 0x600
typedef const std::vector<wchar_t> TLegacyFilterSpec;
static TLegacyFilterSpec ComDlgFilterSpecToLegacy(const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec)
{
	std::vector<wchar_t> l_filter;

	for(UINT i = 0; i < a_nFilterSpec; i++)
	{
		const COMDLG_FILTERSPEC& l_spec = a_filterSpec[i];

		l_filter.insert(l_filter.end(), l_spec.pszName, l_spec.pszName + lstrlenW(l_spec.pszName) + 1);
		l_filter.insert(l_filter.end(), l_spec.pszSpec, l_spec.pszSpec + lstrlenW(l_spec.pszSpec) + 1);
	}

	l_filter.push_back(L'\0');

	return l_filter;
}
#endif


wstring CUtilWin32GUI::OpenFileDialog(HINSTANCE a_instance, HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec)
{
#if _WIN32_WINNT < 0x600
	if(!CUtilWin32::IsWin6x())
	{
		OPENFILENAME ofn = {0};
		TCHAR szBuf[1000] = {0};

		TLegacyFilterSpec l_filter = ComDlgFilterSpecToLegacy(a_filterSpec, a_nFilterSpec);

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hInstance = a_instance;
		ofn.hwndOwner = a_parent;
		ofn.lpstrFilter = l_filter.data();
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = szBuf;
		ofn.nMaxFile = 999;
		ofn.Flags = OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_DONTADDTORECENT;

		if(::GetOpenFileName(&ofn))
		{
			return ofn.lpstrFile;
		}

		return L"";
	}
	else
#endif
	{
		return Win6x_OpenFileDialog(a_parent, a_filterSpec, a_nFilterSpec);
	}
}


wstring CUtilWin32GUI::SaveFileDialog(HINSTANCE a_instance, HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec,
	const LPCTSTR a_defaultExt, const wstring& a_currentFileName, const wstring& a_initialPath)
{
#if _WIN32_WINNT < 0x600
	if(!CUtilWin32::IsWin6x())
	{
		OPENFILENAME ofn = {0};
		TCHAR szBuf[1000] = {0};

		TLegacyFilterSpec l_filter = ComDlgFilterSpecToLegacy(a_filterSpec, a_nFilterSpec);

		if(!a_currentFileName.empty())
		{
			_tcscpy_s(szBuf, 1000, a_currentFileName.c_str());
		}

		if(!a_initialPath.empty())
		{
			ofn.lpstrInitialDir = a_initialPath.c_str();
		}

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hInstance = a_instance;
		ofn.hwndOwner = a_parent;
		ofn.lpstrFilter = l_filter.data();
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = a_defaultExt;
		ofn.lpstrFile = szBuf;
		ofn.nMaxFile = 999;
		ofn.Flags = OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_DONTADDTORECENT;

		if(::GetSaveFileName(&ofn))
		{
			return ofn.lpstrFile;
		}

		return L"";
	}
	else
#endif
	{
		return Win6x_SaveFileDialog(a_parent, a_filterSpec, a_nFilterSpec, a_defaultExt, a_currentFileName, a_initialPath);
	}
}


/************************************************************************/
/* Misc Win32 Helpers                                                   */
/************************************************************************/

void CUtilWin32GUI::PopUpLastWin32Error()
{
	LPTSTR lpMsgBuf = NULL;
	DWORD dwSize;
	dwSize = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	if(lpMsgBuf && dwSize)
	{
		::MessageBox(0, lpMsgBuf, L"Error", MB_ICONSTOP);
		::LocalFree(lpMsgBuf);
	}
}


void CUtilWin32GUI::FormatFileTimeSize(const std::wstring& a_filePath, std::wstring& ar_timeInfo, std::wstring& ar_sizeInfo)
{
	WIN32_FIND_DATA l_ff = {0};

	if(HANDLE l_hFile = ::FindFirstFile(a_filePath.c_str(), &l_ff))
	{
		SYSTEMTIME l_sysTimeUTC = {0}, l_sysTime = {0};
		if(::FileTimeToSystemTime(&l_ff.ftLastWriteTime, &l_sysTimeUTC) &&
			::SystemTimeToTzSpecificLocalTime(NULL, &l_sysTimeUTC, &l_sysTime))
		{
			TCHAR l_date[100] = {0};

			if(::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &l_sysTime,
				NULL, l_date, 99) != 0)
			{
				ar_timeInfo = l_date;
			}

			memset(l_date, 0, 100);

			if(::GetTimeFormat(LOCALE_USER_DEFAULT, 0, &l_sysTime, NULL,
				l_date, 99) != 0)
			{
				ar_timeInfo += L" ";
				ar_timeInfo += l_date;
			}
		}

		ULARGE_INTEGER l_tmpFileSize;
		l_tmpFileSize.HighPart = l_ff.nFileSizeHigh;
		l_tmpFileSize.LowPart = l_ff.nFileSizeLow;

		TCHAR l_sizeBuf[100] = {0};

		if(::StrFormatByteSizeW(l_tmpFileSize.QuadPart, l_sizeBuf, 99))
		{
			ar_sizeInfo = l_sizeBuf;
		}

		::FindClose(l_hFile);
	}
}


int CUtilWin32GUI::StatusCalcPaneWidth(HWND hwnd, LPCTSTR lpsz)
{
	// Credit: Notepad2 by Florian Balmer (BSD License)

	SIZE  size;
	HDC   hdc   = GetDC(hwnd);
	HFONT hfont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
	HFONT hfold = (HFONT)SelectObject(hdc, hfont);
	int   mmode = SetMapMode(hdc, MM_TEXT);

	GetTextExtentPoint32(hdc, lpsz, (int)_tcslen(lpsz), &size);

	SetMapMode(hdc, mmode);
	SelectObject(hdc, hfold);
	ReleaseDC(hwnd, hdc);

	return(size.cx + 9);
}


BOOL CUtilWin32GUI::GenericOnSetCursor(const LPTSTR a_cursor, LPARAM lParam)
{
	switch(LOWORD(lParam)) // hit test code
	{
	case HTCLIENT:
		::SetCursor(::LoadCursor(NULL, a_cursor));
		return TRUE;
	case HTVSCROLL:
	case HTHSCROLL:
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		return TRUE;
	default:
		return FALSE;
	}
}


bool CUtilWin32GUI::TextToClipboard(HWND a_hwnd, const wstring& a_text)
{
	bool l_ok = false;

	if(::OpenClipboard(a_hwnd))
	{
		size_t l_size = sizeof(wchar_t) * (a_text.size() + 1);
		HGLOBAL l_hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, l_size);

		if(l_hGlobal)
		{
			wchar_t* l_hCopy = (wchar_t*)::GlobalLock(l_hGlobal);

			memcpy_s(l_hCopy, l_size, a_text.c_str(), sizeof(wchar_t) * a_text.size());
			l_hCopy[a_text.size()] = 0;
			::GlobalUnlock(l_hCopy); 

			::EmptyClipboard();
			::SetClipboardData(CF_UNICODETEXT, l_hGlobal);

			l_ok = true;
		}

		::CloseClipboard();
	}

	return l_ok;
}
