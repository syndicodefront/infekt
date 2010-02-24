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
#ifdef _WIN32
#include <wininet.h>
#endif

using namespace std;


/************************************************************************/
/* Character Set Conversion Functions                                   */
/************************************************************************/

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


/************************************************************************/
/* String Trim Functions                                                */
/************************************************************************/

template<typename STRTYPE> static void inline _StrTrimLeft(STRTYPE& a_str, const STRTYPE a_chars)
{
	STRTYPE::size_type l_pos = a_str.find_first_not_of(a_chars);

	if(l_pos != STRTYPE::npos)
		a_str.erase(0, l_pos);
	else
		a_str.clear();
}

template<typename STRTYPE> static void inline _StrTrimRight(STRTYPE& a_str, const STRTYPE a_chars)
{
	STRTYPE::size_type l_pos = a_str.find_last_not_of(a_chars);

	if(l_pos != STRTYPE::npos)
	{
		a_str.erase(l_pos + 1);
	}
	else
		a_str.clear();
}

void CUtil::StrTrimLeft(string& a_str, const string a_chars) { _StrTrimLeft<string>(a_str, a_chars); }
void CUtil::StrTrimRight(string& a_str, const string a_chars) { _StrTrimRight<string>(a_str, a_chars); }
void CUtil::StrTrim(string& a_str, const string a_chars) { StrTrimLeft(a_str, a_chars); StrTrimRight(a_str, a_chars); }

void CUtil::StrTrimLeft(wstring& a_str, const wstring a_chars) { _StrTrimLeft<wstring>(a_str, a_chars); }
void CUtil::StrTrimRight(wstring& a_str, const wstring a_chars) { _StrTrimRight<wstring>(a_str, a_chars); }
void CUtil::StrTrim(wstring& a_str, const wstring a_chars) { StrTrimLeft(a_str, a_chars); StrTrimRight(a_str, a_chars); }


/************************************************************************/
/* Misc                                                                 */
/************************************************************************/

static inline void _ParseVersionNumber(const _tstring& vs, vector<int>* ret)
{
	_tstring l_buf;

	for(_tstring::size_type p = 0; p < vs.size(); p++)
	{
		if(vs[p] == _T('.'))
		{
			ret->push_back(_tstoi(l_buf.c_str()));
			l_buf.clear();
		}
		else
		{
			l_buf += vs[p];
		}
	}

	if(!l_buf.empty())
	{
		ret->push_back(_tstoi(l_buf.c_str()));
	}
	else if(ret->empty())
	{
		ret->push_back(0);
	}

	while(ret->size() > 1 && (*ret)[ret->size() - 1] == 0) ret->erase(ret->begin() + (ret->size() - 1));
}

int CUtil::VersionCompare(const _tstring& a_vA, const _tstring& a_vB)
{
	vector<int> l_vA, l_vB;
	_ParseVersionNumber(a_vA, &l_vA);
	_ParseVersionNumber(a_vB, &l_vB);

	size_t l_max = std::min(l_vA.size(), l_vB.size());
	for(size_t p = 0; p < l_max; p++)
	{
		if(l_vA[p] < l_vB[p])
			return -1;
		else if(l_vA[p] > l_vB[p])
			return 1;
	}

	return l_vA.size() - l_vB.size();
}


/************************************************************************/
/* PNG/BITMAP/GDI Helper Functions                                      */
/************************************************************************/

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

	memmove_s(a_data, a_length, (void*)((intptr_t)l_buf->data + (intptr_t)l_buf->pos), l_bytes);

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


/************************************************************************/
/* Common Dialog Helper Functions                                       */
/************************************************************************/

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


void CUtil::PopUpLastWin32Error()
{
	LPTSTR lpMsgBuf = NULL;
	DWORD dwSize;
	dwSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	if(lpMsgBuf && dwSize)
	{
		MessageBox(0, lpMsgBuf, _T("Error"), MB_ICONSTOP);
		::LocalFree(lpMsgBuf);
	}
}


/************************************************************************/
/* Internet/Network Helper Functions                                    */
/************************************************************************/

std::_tstring CUtil::DownloadHttpTextFile(const std::_tstring& a_url)
{
	HINTERNET hInet;
	std::string sText;
	BOOL bSuccess = TRUE;

	hInet = InternetOpen(_T("DownloadHttpTextFile/1.0"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if(hInet)
	{
		HINTERNET hRequest;
		DWORD dwTimeBuffer = 3000;

		InternetSetOption(hInet, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeBuffer, sizeof(dwTimeBuffer));

		hRequest = InternetOpenUrl(hInet, a_url.c_str(), NULL, 0,
			INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE |
			INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_AUTH, 0);

		InternetSetOption(hRequest, INTERNET_OPTION_IGNORE_OFFLINE, NULL, 0);

		if(hRequest)
		{
			int64_t uFileSize = 0;

			if(true)
			{
				TCHAR szSizeBuffer[32];
				DWORD dwLengthSizeBuffer = 32;

				if(HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH, szSizeBuffer, &dwLengthSizeBuffer, NULL) == TRUE)
				{
					uFileSize = _tcstoi64(szSizeBuffer, NULL, 10);
				}
			}

			if(uFileSize && uFileSize < 100 * 1024)
			{
				char szBuffer[8192] = {0};
				DWORD dwRead;

				while(InternetReadFile(hRequest, szBuffer, 8191, &dwRead))
				{
					if(!dwRead || dwRead > 8191)
					{
						break;
					}

					if(lstrlenA(szBuffer) == dwRead)
					{
						sText += szBuffer;
					}
					else
					{
						// we got some binary stuff, but we don't want any.
						bSuccess = FALSE;
						break;
					}
				}
			}

			InternetCloseHandle(hRequest);
		}

		InternetCloseHandle(hInet);
	}

	if(bSuccess)
	{
#ifdef _UNICODE
		// the contents better be UTF-8...
		return CUtil::ToWideStr(sText, CP_UTF8);
#else
		return sText;
#endif
	}
	else
	{
		return _T("");
	}
}

#endif


#ifdef _WIN32

/************************************************************************/
/* Windows OS Version Helper Functions                                  */
/************************************************************************/

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
