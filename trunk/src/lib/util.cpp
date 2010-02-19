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
#include "util.h"

using namespace std;

string CUtil::FromWideStr(const wstring& a_wideStr, unsigned int a_targetCodePage)
{
	int l_size = ::WideCharToMultiByte(a_targetCodePage, 0, a_wideStr.c_str(),
		-1, NULL, NULL, NULL, NULL);

	if(l_size)
	{
		char *l_buf = new char[l_size];

		if(l_buf)
		{
			int l_chrs = ::WideCharToMultiByte(a_targetCodePage, 0, a_wideStr.c_str(),
				-1, l_buf, l_size, NULL, NULL);

			string l_result(l_buf);
			delete[] l_buf;
			return l_result;
		}
	}

	return "";
}


wstring CUtil::ToWideStr(const string& a_str, unsigned int a_originCodePage)
{
	int l_size = ::MultiByteToWideChar(a_originCodePage, 0, a_str.c_str(), -1, NULL, NULL);

	if(l_size)
	{
		wchar_t *l_buf = new wchar_t[l_size];

		if(l_buf)
		{
			int l_chrs = ::MultiByteToWideChar(a_originCodePage, 0, a_str.c_str(), -1, l_buf, l_size);
			wstring l_result(l_buf);
			delete[] l_buf;
			return l_result;
		}
	}

	return L"";
}


bool CUtil::OneCharWideToUtf8(wchar_t a_char, char* a_buf)
{
	return (::WideCharToMultiByte(CP_UTF8, 0, &a_char, 1, a_buf, 7, NULL, NULL) > 0);
}


#ifdef _WIN32_UI
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

	memmove(a_data, (void*)((intptr_t)l_buf->data + (intptr_t)l_buf->pos), l_bytes);

	l_buf->pos += l_bytes;

	return CAIRO_STATUS_SUCCESS;
}

int CUtil::AddPngToImageList(HIMAGELIST a_imgList,
	HINSTANCE a_instance, int a_resourceId, int a_width, int a_height)
{
	HRSRC l_res = ::FindResource(a_instance, MAKEINTRESOURCE(a_resourceId), _T("PNG"));
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
	HDC l_hdc = ::GetDC(NULL);
	HBITMAP l_hBitmap = ::CreateDIBSection(l_hdc, &l_bi, DIB_RGB_COLORS, (void**)&l_rawData, NULL, 0);
	::ReleaseDC(NULL, l_hdc);

	if(l_hBitmap)
	{
		BITMAP l_bitmap = {0};
		::GetObject(l_hBitmap, sizeof(BITMAP), &l_bitmap);
		cairo_surface_t* l_surfaceOut = cairo_image_surface_create_for_data(
			l_rawData, CAIRO_FORMAT_ARGB32, a_width, a_height, l_bitmap.bmWidthBytes);

		if(l_surfaceOut)
		{
			cairo_t *cr = cairo_create(l_surfaceOut);

			// Windows 2000 doesn't like transparent backgrounds,
			// so we make it solid.
			if(CUtil::IsWin2000())
			{
				COLORREF l_clr = GetSysColor(COLOR_BTNFACE);
				cairo_set_source_rgb(cr, GetRValue(l_clr) / 255.0, GetGValue(l_clr) / 255.0, GetBValue(l_clr) / 255.0);
				cairo_paint(cr);
			}

			// copy PNG to bitmap surface:
			cairo_set_source_surface(cr, l_surfacePng, 0, 0);
			cairo_rectangle(cr, 0, 0, a_width, a_height);
			cairo_fill(cr);
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


_tstring CUtil::OpenFileDialog(HINSTANCE a_instance, HWND a_parent, const LPCTSTR a_filter)
{
	OPENFILENAME ofn = {0};
	TCHAR szBuf[1000] = {0};

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = a_instance;
	ofn.hwndOwner = a_parent;
	ofn.lpstrFilter = a_filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szBuf;
	ofn.nMaxFile = 999;
	ofn.Flags = OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

	if(GetOpenFileName(&ofn))
	{
		return ofn.lpstrFile;
	}

	return _T("");
}


_tstring CUtil::SaveFileDialog(HINSTANCE a_instance, HWND a_parent, const LPCTSTR a_filter, const LPCTSTR a_defaultExt, const _tstring& a_currentFileName)
{
	OPENFILENAME ofn = {0};
	TCHAR szBuf[1000] = {0};

	if(!a_currentFileName.empty())
	{
		_tcscpy_s(szBuf, 1000, a_currentFileName.c_str());
	}

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = a_instance;
	ofn.hwndOwner = a_parent;
	ofn.lpstrFilter = a_filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = a_defaultExt;
	ofn.lpstrFile = szBuf;
	ofn.nMaxFile = 999;
	ofn.Flags = OFN_ENABLESIZING | OFN_OVERWRITEPROMPT;

	if(GetSaveFileName(&ofn))
	{
		return ofn.lpstrFile;
	}

	return _T("");
}

#endif

#ifdef _WIN32

bool CUtil::IsWin2000()
{
	if(!ms_osver.dwOSVersionInfoSize) { ms_osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); GetVersionEx(&ms_osver); }
	return (ms_osver.dwMajorVersion == 5 && ms_osver.dwMinorVersion == 0);
}

bool CUtil::IsWinXP()
{
	if(!ms_osver.dwOSVersionInfoSize) { ms_osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); GetVersionEx(&ms_osver); }
	return (ms_osver.dwMajorVersion == 5 && ms_osver.dwMinorVersion == 1);
}

bool CUtil::IsWin5x()
{
	if(!ms_osver.dwOSVersionInfoSize) { ms_osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); GetVersionEx(&ms_osver); }
	return (ms_osver.dwMajorVersion == 5);
}

bool CUtil::IsWin6x(bool a_orHigher)
{
	if(!ms_osver.dwOSVersionInfoSize) { ms_osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); GetVersionEx(&ms_osver); }
	return (ms_osver.dwMajorVersion == 6 || (ms_osver.dwMajorVersion > 6 && a_orHigher));
}

OSVERSIONINFO CUtil::ms_osver = {0};

#endif
