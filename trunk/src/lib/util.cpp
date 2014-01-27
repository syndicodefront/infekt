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


/************************************************************************/
/* Character Set Conversion Functions                                   */
/************************************************************************/

#ifdef _WIN32

string CUtil::FromWideStr(const wstring& a_wideStr, unsigned int a_targetCodePage)
{
	int l_size = ::WideCharToMultiByte(a_targetCodePage, 0, a_wideStr.c_str(),
		-1, NULL, NULL, NULL, NULL);

	if(l_size)
	{
		char *l_buf = new char[l_size];

		if(l_buf)
		{
			*l_buf = 0;
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

		if(l_buf)
		{
			*l_buf = 0;
			::MultiByteToWideChar(a_originCodePage, 0, a_str.c_str(), -1, l_buf, l_size);
			wstring l_result(l_buf);
			delete[] l_buf;
			return l_result;
		}
	}

	return L"";
}


// ATTENTION CALLERS: a_buf must have 6 chars space.
bool CUtil::OneCharWideToUtf8(wchar_t a_char, char* a_buf)
{
	return (::WideCharToMultiByte(CP_UTF8, 0, &a_char, 1, a_buf, 7, NULL, NULL) > 0);
}

#else /* _WIN32 */

string CUtil::FromWideStr(const wstring& a_wideStr, unsigned int a_targetCodePage)
{
	const char* l_targetCodePage;

	switch(a_targetCodePage)
	{
	case CP_UTF8: l_targetCodePage = "UTF-8"; break;
	case CP_ACP: l_targetCodePage = "ISO-8859-1"; break;
	default:
		return "";
	}

	char *l_sResult = NULL;
	if(iconv_string(l_targetCodePage, "wchar_t", (char*)a_wideStr.c_str(),
		(char*)(a_wideStr.c_str() + a_wideStr.size() + 1), &l_sResult, NULL) >= 0)
	{
		string l_result = l_sResult;
		free(l_sResult);
		return l_result;
	}

	return "";
}


wstring CUtil::ToWideStr(const string& a_str, unsigned int a_originCodePage)
{
	const char* l_originCodePage;

	switch(a_originCodePage)
	{
	case CP_UTF8: l_originCodePage = "UTF-8"; break;
	case CP_ACP: l_originCodePage = "ISO-8859-1"; break;
	default:
		return L"";
	}

	wchar_t *l_wResult = NULL;
	if(iconv_string("wchar_t", l_originCodePage,
		a_str.c_str(), a_str.c_str() + a_str.size() + 1,
		(char**)&l_wResult, NULL) >= 0)
	{
		wstring l_result = l_wResult;
		free(l_wResult);
		return l_result;
	}

	return L"";
}


// ATTENTION CALLERS: a_buf must have 6 chars space.
bool CUtil::OneCharWideToUtf8(wchar_t a_char, char* a_buf)
{
	char *l_buf = NULL;
	size_t l_len = 9;
	wchar_t l_tmp[2] = { a_char, 0 };

	if(iconv_string("UTF-8", "wchar_t", (char*)&l_tmp, (char*)(&l_tmp + 1), &l_buf, &l_len) >= 0)
	{
		strncpy(a_buf, l_buf, l_len);

		free(l_buf);	

		return (l_len > 0);
	}

	return false;
}


#endif  /* else _WIN32 */


/************************************************************************/
/* String Trim Functions                                                */
/************************************************************************/

template<typename STRTYPE> static void inline _StrTrimLeft(STRTYPE& a_str, const STRTYPE a_chars)
{
	typename STRTYPE::size_type l_pos = a_str.find_first_not_of(a_chars);

	if(l_pos != STRTYPE::npos)
		a_str.erase(0, l_pos);
	else
		a_str.clear();
}

template<typename STRTYPE> static void inline _StrTrimRight(STRTYPE& a_str, const STRTYPE a_chars)
{
	typename STRTYPE::size_type l_pos = a_str.find_last_not_of(a_chars);

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
/* Str Replace                                                          */
/************************************************************************/

template<typename T> static T _StrReplace(const T& a_find, const T& a_replace, const T& a_input)
{
	typename T::size_type l_pos = a_input.find(a_find), l_prevPos = 0;
	T l_new;

	while(l_pos != T::npos)
	{
		l_new.append(a_input.substr(l_prevPos, l_pos - l_prevPos));

		l_new.append(a_replace);

		l_prevPos = l_pos + a_find.size();
		l_pos = a_input.find(a_find, l_prevPos);
	}

	if(l_prevPos == 0)
	{
		return a_input;
	}
	else
	{
		l_new.append(a_input.substr(l_prevPos));

		return l_new;
	}
}

std::string CUtil::StrReplace(const std::string& a_find, const std::string& a_replace, const std::string& a_input)
{
	return _StrReplace<std::string>(a_find, a_replace, a_input);
}

std::wstring CUtil::StrReplace(const std::wstring& a_find, const std::wstring& a_replace, const std::wstring& a_input)
{
	return _StrReplace<std::wstring>(a_find, a_replace, a_input);
}


/************************************************************************/
/* Reg Ex Utils                                                         */
/************************************************************************/

#define OVECTOR_SIZE 60
wstring CUtil::RegExReplaceUtf16(const wstring& a_subject, const wstring& a_pattern, const wstring& a_replacement, int a_flags)
{
	wstring l_result;

	if(a_subject.size() > (uint64_t)std::numeric_limits<int>::max())
	{
		return L"";
	}

	const char *szErrDescr;
	int iErrOffset;
	int ovector[OVECTOR_SIZE];

	pcre16* re = pcre16_compile(reinterpret_cast<PCRE_SPTR16>(a_pattern.c_str()),
		PCRE_UTF16 | PCRE_NEWLINE_ANYCRLF | a_flags,
		&szErrDescr, &iErrOffset, NULL);

	if(re)
	{
		int l_prevEndPos = 0; // the index of the character that follows the last character of the previous match.
		pcre16_extra *pe = pcre16_study(re, 0, &szErrDescr); // this could be NULL but it wouldn't matter.

		while(1)
		{
			int l_execResult = pcre16_exec(re, pe, reinterpret_cast<PCRE_SPTR16>(a_subject.c_str()), (int)a_subject.size(), l_prevEndPos, 0, ovector, OVECTOR_SIZE);

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
				wstring l_replacement;
				wstring::size_type l_pos = a_replacement.find(L'$'), l_prevPos = 0;

				while(l_pos != wstring::npos)
				{
					l_replacement += a_replacement.substr(l_prevPos, l_pos - l_prevPos);

					wstring l_numBuf;
					while(l_pos + 1 < a_replacement.size() &&
						(a_replacement[l_pos + 1] >= L'0' && a_replacement[l_pos + 1] <= L'9'))
					{
						l_pos++;
						l_numBuf += a_replacement[l_pos];
					}
					// maybe make "$14" insert $1 + "4" here if there is no $14.

					int l_group = _wtoi(l_numBuf.c_str());
					if(l_group >= 0 && l_group < l_execResult)
					{
						int l_len = ovector[l_group * 2 + 1] - ovector[l_group * 2];
						l_replacement.append(a_subject, ovector[l_group * 2], l_len);
					}

					l_prevPos = l_pos + 1;
					l_pos = a_replacement.find(L'$', l_prevPos);
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

		if(pe) pcre16_free(pe);
		pcre16_free(re);
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

static inline void _ParseVersionNumber(const wstring& vs, vector<int>* ret)
{
	wstring l_buf;

	for(wstring::size_type p = 0; p < vs.size(); p++)
	{
		if(vs[p] == L'.')
		{
			ret->push_back(_wtoi(l_buf.c_str()));
			l_buf.clear();
		}
		else
		{
			l_buf += vs[p];
		}
	}

	if(!l_buf.empty())
	{
		ret->push_back(_wtoi(l_buf.c_str()));
	}
	else if(ret->empty())
	{
		ret->push_back(0);
	}

	while(ret->size() > 1 && (*ret)[ret->size() - 1] == 0) ret->erase(ret->begin() + (ret->size() - 1));
}

int CUtil::VersionCompare(const wstring& a_vA, const wstring& a_vB)
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


wstring CUtil::OpenFileDialog(HINSTANCE a_instance, HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec)
{
#if _WIN32_WINNT < 0x600
	if(!CUtil::IsWin6x())
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


wstring CUtil::SaveFileDialog(HINSTANCE a_instance, HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec,
	const LPCTSTR a_defaultExt, const wstring& a_currentFileName, const wstring& a_initialPath)
{
#if _WIN32_WINNT < 0x600
	if(!CUtil::IsWin6x())
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

void CUtil::PopUpLastWin32Error()
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


#endif /* _WIN32_UI */

#ifdef _WIN32

std::wstring CUtil::PathRemoveFileSpec(const std::wstring& a_path)
{
	TCHAR* l_buf = new TCHAR[a_path.size() + 1];
	memset(l_buf, 0, a_path.size() + 1);

	wcscpy_s(l_buf, a_path.size() + 1, a_path.c_str());

	::PathRemoveFileSpec(l_buf);
	::PathRemoveBackslash(l_buf);

	std::wstring l_result(l_buf);
	delete[] l_buf;

	return l_result;
}


std::wstring CUtil::PathRemoveExtension(const std::wstring& a_path)
{
	TCHAR* l_buf = new TCHAR[a_path.size() + 1];
	memset(l_buf, 0, a_path.size() + 1);

	wcscpy_s(l_buf, a_path.size() + 1, a_path.c_str());

	::PathRemoveExtension(l_buf);

	std::wstring l_result(l_buf);
	delete[] l_buf;

	return l_result;
}


std::wstring CUtil::GetTempDir()
{
	wchar_t l_tmpPathBuf[1000] = {0};

	if(::GetTempPath(999, l_tmpPathBuf))
	{
		::PathAddBackslash(l_tmpPathBuf);

		return l_tmpPathBuf;
	}

	return L"";
}


std::wstring CUtil::GetAppDataDir(bool a_local, const std::wstring& a_appName)
{
	wchar_t l_tmpPathBuf[1000] = {0};

	if(::SHGetFolderPath(0, a_local ? CSIDL_LOCAL_APPDATA : CSIDL_APPDATA, NULL,
		SHGFP_TYPE_CURRENT, l_tmpPathBuf) == S_OK)
	{
		::PathAddBackslash(l_tmpPathBuf);

		::PathAppend(l_tmpPathBuf, a_appName.c_str());

		if(!::PathIsDirectory(l_tmpPathBuf))
		{
			::SHCreateDirectoryEx(NULL, l_tmpPathBuf, NULL);
		}

		if(::PathIsDirectory(l_tmpPathBuf))
		{
			::PathAddBackslash(l_tmpPathBuf);

			return l_tmpPathBuf;
		}
	}

	return L"";
}


HMODULE CUtil::SilentLoadLibrary(const std::wstring& a_path)
{
	_ASSERT(!PathIsRelative(a_path.c_str()));

	HMODULE l_hResult = NULL;
	DWORD dwErrorMode = SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS;

	if(CUtil::IsWin7(true))
	{
		// BOOL SetThreadErrorMode(DWORD dwNewMode, LPDWORD lpOldMode);
		typedef BOOL (WINAPI *fstem)(DWORD, LPDWORD);

		fstem fnc = (fstem)::GetProcAddress(::GetModuleHandle(L"Kernel32.dll"), "SetThreadErrorMode");

		if(fnc)
		{
			DWORD l_oldErrorMode = 0;
			fnc(dwErrorMode, &l_oldErrorMode);
			l_hResult = ::LoadLibrary(a_path.c_str());
			fnc(l_oldErrorMode, NULL);
		}
	}
	else
	{
		UINT l_oldErrorMode = ::SetErrorMode(dwErrorMode);
		l_hResult = ::LoadLibrary(a_path.c_str());
		::SetErrorMode(l_oldErrorMode);
	}

	return l_hResult;
}


bool CUtil::RemoveCwdFromDllSearchPath()
{
	OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX), 0 };
	DWORDLONG dwlConditionMask = 0;
	
	osvi.dwMajorVersion = 5;
	osvi.dwMinorVersion = 1;
	osvi.wServicePackMajor = 1;
	
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
	
	// check requirements for SetDllDirectory availability:
	if(::VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask))
	{
		typedef BOOL (WINAPI *fsdd)(LPCTSTR);

		// remove the current directory from the DLL search path by calling SetDllDirectory:
		fsdd l_fsdd = (fsdd)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "SetDllDirectory");
		if(l_fsdd)
		{
			return (l_fsdd(L"") != FALSE);
		}
	}

	return false;
}


std::wstring CUtil::GetExePath()
{
	TCHAR l_buf[1000] = {0};
	TCHAR l_buf2[1000] = {0};

	::GetModuleFileName(NULL, (LPTCH)l_buf, 999);
	::GetLongPathName(l_buf, l_buf2, 999);

	return l_buf2;
}


std::wstring CUtil::GetExeDir()
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
	// Explicitly activate DEP, especially important for XP SP3.
	// http://msdn.microsoft.com/en-us/library/bb736299%28VS.85%29.aspx
	typedef BOOL (WINAPI *fspdp)(DWORD);
	fspdp l_fSpDp = (fspdp)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "SetProcessDEPPolicy");
	if(l_fSpDp)
	{
		return (l_fSpDp(PROCESS_DEP_ENABLE) != FALSE);
	}

	return false;
#else
	return true; // always enabled on x64 anyway.
#endif
}


/************************************************************************/
/* Windows OS Version Helper Functions                                  */
/************************************************************************/

static bool IS_WIN_XX(DWORD MAJ, DWORD MIN, bool OR_HIGHER)
{
	OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX), 0 };
	DWORDLONG dwlConditionMask = 0;
	
	osvi.dwMajorVersion = MAJ;
	osvi.dwMinorVersion = MIN;
	
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, OR_HIGHER ? VER_GREATER_EQUAL : VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, OR_HIGHER ? VER_GREATER_EQUAL : VER_EQUAL);
	
	return ::VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) == TRUE;
}

bool CUtil::IsWinXP()
{
	static bool is = IS_WIN_XX(5, 1, false)
		|| IS_WIN_XX(5, 2, false); // Server 2003

	return is;
}

bool CUtil::IsWin6x(bool a_orHigher)
{
	static bool is = IS_WIN_XX(6, 0, true);

	return is;
}

bool CUtil::IsWinVista()
{
	static bool is = IS_WIN_XX(6, 0, false);

	return is;
}

bool CUtil::IsWin7(bool a_orHigher)
{
	static bool is = IS_WIN_XX(6, 1, a_orHigher);

	return is;
}

bool CUtil::IsWin8()
{
	static bool is = IS_WIN_XX(6, 2, false);

	return is;
}

bool CUtil::IsWin81()
{
	static bool is = IS_WIN_XX(6, 3, false);

	return is;
}

bool CUtil::IsWinServerOS()
{
	OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX), 0 };
	DWORDLONG dwlConditionMask = 0;
	
	osvi.wProductType = VER_NT_SERVER;
	
	VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
	
	return ::VerifyVersionInfo(&osvi, VER_PRODUCT_TYPE, dwlConditionMask) == TRUE;
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


/************************************************************************/
/* WINDOWS BENCHMARK/HIGH RESOLUTION TIMER                              */
/************************************************************************/


CBenchmarkTimer::CBenchmarkTimer()
{
	memset(&m_start, 0, sizeof(LARGE_INTEGER));
	memset(&m_stop, 0, sizeof(LARGE_INTEGER));
	m_frequency = GetFrequency() / 1000.0f; // milliseconds
}

double CBenchmarkTimer::GetFrequency()
{
	LARGE_INTEGER l_freq;

	::QueryPerformanceFrequency(&l_freq);

	return static_cast<double>(l_freq.QuadPart);
}

void CBenchmarkTimer::StartTimer()
{
	DWORD_PTR l_oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);

	::QueryPerformanceCounter(&m_start);

	::SetThreadAffinityMask(::GetCurrentThread(), l_oldmask);
}

double CBenchmarkTimer::StopTimer(void)
{
	DWORD_PTR l_oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);

	::QueryPerformanceCounter(&m_stop);

	::SetThreadAffinityMask(::GetCurrentThread(), l_oldmask);

	return ((m_stop.QuadPart - m_start.QuadPart) / m_frequency);
}

double CBenchmarkTimer::StopDumpTimer(const char* a_name)
{
	double l_secs = StopTimer();
	char l_buf[256] = {0};

	sprintf_s(l_buf, 255, "BenchmarkTimer: [%s] %.2f msec\r\n", a_name, l_secs);

	::OutputDebugStringA(l_buf);

	return l_secs;
}


#endif /* _WIN32 */
