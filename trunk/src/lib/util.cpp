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
		*l_buf = 0;

		if(l_buf)
		{
			::WideCharToMultiByte(a_targetCodePage, 0, a_wideStr.c_str(), -1, l_buf, l_size, NULL, NULL);

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
		*l_buf = 0;

		if(l_buf)
		{
			::MultiByteToWideChar(a_originCodePage, 0, a_str.c_str(), -1, l_buf, l_size);
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
/* Reg Ex Utils                                                         */
/************************************************************************/

#define OVECTOR_SIZE 60
string CUtil::RegExReplaceUtf8(const string& a_subject, const string& a_pattern, const string& a_replacement,
	int a_flags)
{
	string l_result;

	if(a_subject.size() > (uint64_t)std::numeric_limits<int>::max())
	{
		return "";
	}

	const char *szErrDescr;
	int iErrOffset;
	int ovector[OVECTOR_SIZE];

	pcre* re;
	pcre_extra *pe;

	if((re = pcre_compile(a_pattern.c_str(), PCRE_UTF8 | PCRE_NEWLINE_ANYCRLF | a_flags, &szErrDescr, &iErrOffset, NULL)) != NULL)
	{
		int l_prevEndPos = 0; // the index of the character that follows the last character of the previous match.

		pe = pcre_study(re, 0, &szErrDescr); // this could be NULL but it wouldn't matter.

		while(1)
		{
			int l_execResult = pcre_exec(re, pe, a_subject.c_str(), (int)a_subject.size(), l_prevEndPos, 0, ovector, OVECTOR_SIZE);

			if(l_execResult == PCRE_ERROR_NOMATCH)
			{
				l_result += a_subject.substr(l_prevEndPos);
				break;
			}
			else if(l_execResult < 1)
			{
				// ovector is too small (= 0) or some other internal error (< 0).
				break;
			}

			_ASSERT(ovector[0] >= l_prevEndPos);

			// append string between end of last match and the start of this one:
			l_result += a_subject.substr(l_prevEndPos, ovector[0] - l_prevEndPos);

			if(!a_replacement.empty())
			{
				// insert back references of form $1 $2 $3 ...
				string l_replacement;
				string::size_type l_pos = a_replacement.find('$'), l_prevPos = 0;

				while(l_pos != string::npos)
				{
					l_replacement += a_replacement.substr(l_prevPos, l_pos - l_prevPos);

					string l_numBuf;
					while(l_pos + 1 < a_replacement.size() &&
						(a_replacement[l_pos + 1] >= '0' && a_replacement[l_pos + 1] <= '9'))
					{
						l_pos++;
						l_numBuf += a_replacement[l_pos];
					}
					// maybe make "$14" insert $1 + "4" here if there is no $14.

					int l_group = atoi(l_numBuf.c_str());
					if(l_group >= 0 && l_group < l_execResult)
					{
						int l_len = ovector[l_group * 2 + 1] - ovector[l_group * 2];
						l_replacement.append(a_subject, ovector[l_group * 2], l_len);
					}

					l_prevPos = l_pos + 1;
					l_pos = a_replacement.find('$', l_prevPos);
				}

				if(l_prevPos < a_replacement.size() - 1)
				{
					l_replacement += a_replacement.substr(l_prevPos);
				}

				l_result += l_replacement;
			}

			// this is where we will start searching again:
			l_prevEndPos = ovector[1];
		}

		if(pe) pcre_free(pe);
		pcre_free(re);
	}
	else
	{
		_ASSERT(false);
	}

	return l_result;
}
#undef OVECTOR_SIZE


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

	return static_cast<int>(l_vA.size() - l_vB.size());
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

	if(::GetOpenFileName(&ofn))
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

	if(::GetSaveFileName(&ofn))
	{
		return ofn.lpstrFile;
	}

	return _T("");
}


/************************************************************************/
/* Misc Win32 Helpers                                                   */
/************************************************************************/

void CUtil::PopUpLastWin32Error()
{
	LPTSTR lpMsgBuf = NULL;
	DWORD dwSize;
	dwSize = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	if(lpMsgBuf && dwSize)
	{
		::MessageBox(0, lpMsgBuf, _T("Error"), MB_ICONSTOP);
		::LocalFree(lpMsgBuf);
	}
}


std::_tstring CUtil::PathRemoveFileSpec(const std::_tstring& a_path)
{
	TCHAR* l_buf = new TCHAR[a_path.size() + 1];
	memset(l_buf, 0, a_path.size() + 1);

	_tcscpy_s(l_buf, a_path.size() + 1, a_path.c_str());

	::PathRemoveFileSpec(l_buf);
	::PathRemoveBackslash(l_buf);

	std::_tstring l_result(l_buf);
	delete[] l_buf;

	return l_result;
}


uint32_t CUtil::RegQueryDword(HKEY a_key, const LPTSTR a_name, uint32_t a_default)
{
	DWORD l_dwType;
	uint32_t l_dwBuf = 0;
	DWORD l_dwBufSz = sizeof(uint32_t);

	if(RegQueryValueEx(a_key, a_name, NULL, &l_dwType,
		(LPBYTE)&l_dwBuf, &l_dwBufSz) == ERROR_SUCCESS && l_dwType == REG_DWORD)
	{
		return l_dwBuf;
	}

	return a_default;
}

int CUtil::StatusCalcPaneWidth(HWND hwnd, LPCTSTR lpsz)
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


BOOL CUtil::GenericOnSetCursor(const LPTSTR a_cursor, LPARAM lParam)
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


bool CUtil::TextToClipboard(HWND a_hwnd, const wstring& a_text)
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

#endif /* _WIN32_UI */


#ifdef _WIN32


HMODULE CUtil::SilentLoadLibrary(const std::_tstring& a_path)
{
	_ASSERT(!PathIsRelative(a_path.c_str()));

	UINT l_oldErrorMode = ::SetErrorMode(SEM_NOOPENFILEERRORBOX);
	HMODULE l_hResult = ::LoadLibrary(a_path.c_str());
	::SetErrorMode(l_oldErrorMode);

	return l_hResult;
}


bool CUtil::RemoveCwdFromDllSearchPath()
{
	if(!ms_osver.dwMajorVersion) { ::GetVersionEx((LPOSVERSIONINFO)&ms_osver); }

	// requirements for SetDllDirectory availability:
	if(ms_osver.dwMajorVersion > 5 ||
		(ms_osver.dwMajorVersion == 5 && ms_osver.wServicePackMajor >= 1))
	{
		typedef BOOL (WINAPI *fsdd)(LPCTSTR);

		// remove the current directory from the DLL search path by calling SetDllDirectory:
		fsdd l_fsdd = (fsdd)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "SetDllDirectory");
		if(l_fsdd)
		{
			return (l_fsdd(_T("")) != FALSE);
		}
	}

	return false;
}


std::_tstring CUtil::GetExePath()
{
	TCHAR l_buf[1000] = {0};
	TCHAR l_buf2[1000] = {0};

	::GetModuleFileName(NULL, (LPTCH)l_buf, 999);
	::GetLongPathName(l_buf, l_buf2, 999);

	return l_buf2;
}


std::_tstring CUtil::GetExeDir()
{
	TCHAR l_buf[1000] = {0};
	TCHAR l_buf2[1000] = {0};

	::GetModuleFileName(NULL, (LPTCH)l_buf, 999);
	::GetLongPathName(l_buf, l_buf2, 999);
	::PathRemoveFileSpec(l_buf2);
	::PathRemoveBackslash(l_buf2);

	return l_buf2;
}


bool CUtil::HardenHeap()
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

bool CUtil::EnforceDEP()
{
#ifndef _WIN64
	if(CUtil::IsWinXP() || CUtil::IsWin6x())
	{
		// Explicitly activate DEP, especially important for XP SP3.
		// http://msdn.microsoft.com/en-us/library/bb736299%28VS.85%29.aspx
		typedef BOOL (WINAPI *fspdp)(DWORD);
		fspdp l_fSpDp = (fspdp)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "SetProcessDEPPolicy");
		if(l_fSpDp)
		{
			return (l_fSpDp(PROCESS_DEP_ENABLE) != FALSE);
		}
	}

	return false;
#else
	return true; // always enabled on x64 anyway.
#endif
}


/************************************************************************/
/* Windows OS Version Helper Functions                                  */
/************************************************************************/

bool CUtil::IsWin2000()
{
	if(!ms_osver.dwMajorVersion) { ::GetVersionEx((LPOSVERSIONINFO)&ms_osver); }
	return (ms_osver.dwMajorVersion == 5 && ms_osver.dwMinorVersion == 0);
}

bool CUtil::IsWinXP()
{
	if(!ms_osver.dwMajorVersion) { ::GetVersionEx((LPOSVERSIONINFO)&ms_osver); }
	return (ms_osver.dwMajorVersion == 5 && ms_osver.dwMinorVersion == 1);
}

bool CUtil::IsWin5x()
{
	if(!ms_osver.dwMajorVersion) { ::GetVersionEx((LPOSVERSIONINFO)&ms_osver); }
	return (ms_osver.dwMajorVersion == 5);
}

bool CUtil::IsWin6x(bool a_orHigher)
{
	if(!ms_osver.dwMajorVersion) { ::GetVersionEx((LPOSVERSIONINFO)&ms_osver); }
	return (ms_osver.dwMajorVersion == 6 || (ms_osver.dwMajorVersion > 6 && a_orHigher));
}

bool CUtil::IsWinVista()
{
	if(!ms_osver.dwMajorVersion) { ::GetVersionEx((LPOSVERSIONINFO)&ms_osver); }
	return (ms_osver.dwMajorVersion == 6 && ms_osver.dwMinorVersion == 0);
}

bool CUtil::IsWin7()
{
	if(!ms_osver.dwMajorVersion) { ::GetVersionEx((LPOSVERSIONINFO)&ms_osver); }
	return (ms_osver.dwMajorVersion == 6 && ms_osver.dwMinorVersion == 1);
#if 0
	bool b = (ms_osver.dwMajorVersion == 6 && ms_osver.dwMinorVersion == 1);

	if(!b && ms_osver.dwMajorVersion == 8 && ms_osver.dwMinorVersion == 3 && ms_osver.dwBuildNumber == 8600 && IsWow64())
	{
		// don't ask, I have no idea, but it seems to be a rare issue on some systems.
		b = true;
	}

	return b;
#endif
}

bool CUtil::IsWow64()
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

OSVERSIONINFOEX CUtil::ms_osver = {sizeof(OSVERSIONINFOEX), 0};


/************************************************************************/
/* CUDA UTIL FUNCTIONS                                                  */
/************************************************************************/


CCudaUtil::CCudaUtil()
{
	m_hCudaBlur = CUtil::SilentLoadLibrary(
		CUtil::GetExeDir() + _T("\\cuda-blur.dll"));
}

const CCudaUtil* CCudaUtil::GetInstance()
{
	static CCudaUtil* ls_inst = NULL;

	if(!ls_inst)
	{
		ls_inst = new CCudaUtil();
	}

	return ls_inst;
}

bool CCudaUtil::IsCudaUsable() const
{
	typedef int (__cdecl *fnc)();

	if(fnc icu = (fnc)GetProcAddress(m_hCudaBlur, "IsCudaUsable"))
	{
		return (icu() > 0);
	}

	return false;
}

bool CCudaUtil::InitCudaThread() const
{
	typedef int (__cdecl *fnc)();

	if(fnc ict = (fnc)GetProcAddress(m_hCudaBlur, "InitCudaThread"))
	{
		return (ict() > 0);
	}

	return false;
}

bool CCudaUtil::IsCudaThreadInitialized() const
{
	typedef int (__cdecl *fnc)();

	if(fnc icti = (fnc)GetProcAddress(m_hCudaBlur, "IsCudaThreadInitialized"))
	{
		return (icti() > 0);
	}

	return false;
}

bool CCudaUtil::UnInitCudaThread() const
{
	typedef int (__cdecl *fnc)();

	if(fnc uict = (fnc)GetProcAddress(m_hCudaBlur, "UnInitCudaThread"))
	{
		return (uict() > 0);
	}

	return false;
}

bool CCudaUtil::DoCudaBoxBlurA8(unsigned char* a_data, int a_stride, int a_rows, int a_lobes[3][2]) const
{
	typedef int (__cdecl *fnc)(unsigned char* a_data, int a_stride, int a_rows, int a_lobes[3][2]);

	if(fnc dcdda8 = (fnc)GetProcAddress(m_hCudaBlur, "DoCudaBoxBlurA8"))
	{
		return (dcdda8(a_data, a_stride, a_rows, a_lobes) > 0);
	}

	return false;
}

CCudaUtil::~CCudaUtil()
{
	FreeLibrary(m_hCudaBlur);
}


#endif /* _WIN32 */
