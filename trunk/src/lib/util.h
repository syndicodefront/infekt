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

#ifndef _UTIL_H
#define _UTIL_H

#ifdef _WIN32_UI
#include "theme_api.h"

#if _WIN32_WINNT < 0x600
/* just make it compile! */
typedef struct _COMDLG_FILTERSPEC {
	LPCWSTR pszName;
	LPCWSTR pszSpec;
} COMDLG_FILTERSPEC;
#endif
#endif


class CUtil
{
public:
	static std::string FromWideStr(const std::wstring& a_wideStr, unsigned int a_targetCodePage);
	static std::wstring ToWideStr(const std::string& a_str, unsigned int a_originCodePage);
	static bool OneCharWideToUtf8(wchar_t a_char, char* a_buf);

	static void StrTrimLeft(std::string& a_str, const std::string a_chars = "\t\r\n ");
	static void StrTrimRight(std::string& a_str, const std::string a_chars = "\t\r\n ");
	static void StrTrim(std::string& a_str, const std::string a_chars = "\t\r\n ");
	static void StrTrimLeft(std::wstring& a_str, const std::wstring a_chars = L"\t\r\n ");
	static void StrTrimRight(std::wstring& a_str, const std::wstring a_chars = L"\t\r\n ");
	static void StrTrim(std::wstring& a_str, const std::wstring a_chars = L"\t\r\n ");

	static int VersionCompare(const std::wstring& a_vA, const std::wstring& a_vB);

	static std::string RegExReplaceUtf8(const std::string& a_subject, const std::string& a_pattern,
		const std::string& a_replacement, int a_flags = 0);

	static std::string StrReplace(const std::string& a_find, const std::string& a_replace, const std::string& a_input);
	static std::wstring StrReplace(const std::wstring& a_find, const std::wstring& a_replace, const std::wstring& a_input);

#ifdef _WIN32_UI
	static int AddPngToImageList(HIMAGELIST a_imgList, HINSTANCE a_instance, int a_resourceId, int a_width, int a_height);
	static std::wstring OpenFileDialog(HINSTANCE a_instance, HWND a_parent, const LPCTSTR a_filter, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec);
	static std::wstring SaveFileDialog(HINSTANCE a_instance, HWND a_parent, const LPCTSTR a_filter, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec,
		const LPCTSTR a_defaultExt, const std::wstring& a_currentFileName, const std::wstring& a_initialPath = _T(""));

	static void PopUpLastWin32Error();

	static int StatusCalcPaneWidth(HWND hwnd, LPCTSTR lpsz);
	static BOOL GenericOnSetCursor(const LPTSTR a_cursor, LPARAM lParam);
#endif /* _WIN32_UI */

#ifdef _WIN32
	static bool IsWinXP();
	static bool IsWin6x(bool a_orHigher = true);
	static bool IsWinVista();
	static bool IsWin7(bool a_orHigher = false);
	static bool IsWin8();
	static bool IsWow64();

	static std::wstring GetExePath();
	static std::wstring GetExeDir();
	static std::wstring PathRemoveFileSpec(const std::wstring& a_path);
	static std::wstring PathRemoveExtension(const std::wstring& a_path);
	static std::wstring GetTempDir();
	static std::wstring GetAppDataDir(bool a_local, const std::wstring& a_appName);
	static HMODULE SilentLoadLibrary(const std::wstring& a_path);
	static bool TextToClipboard(HWND a_hwnd, const std::wstring& a_text);

	static bool RemoveCwdFromDllSearchPath();
	static bool HardenHeap();
	static bool EnforceDEP();
protected:
	static OSVERSIONINFOEX ms_osver;
#endif /* _WIN32 */
};


template <typename T> class TwoDimVector
{
public:
	TwoDimVector(size_t a_rows, size_t a_cols, T a_initial) :
	  m_data(a_rows, std::vector<T>(a_cols))
	{
		m_rows = a_rows;
		m_cols = a_cols;

		for(size_t r = 0; r < m_rows; r++)
		{
			for(size_t c = 0; c < m_cols; c++)
			{
				(*this)[r][c] = a_initial;
			}
		}
	}

	std::vector<T> & operator[](size_t i) 
	{ 
		return m_data[i];
	}

	const std::vector<T> & operator[](size_t i) const
	{ 
		return m_data[i];
	}

	const size_t GetRows() const { return m_rows; }
	const size_t GetCols() const { return m_cols; }
private:
	std::vector<std::vector<T> > m_data;
	size_t m_rows, m_cols;

	TwoDimVector() {}
};

template <typename T> int sgn(T val) {
    return (val > T(0)) - (val < T(0));
};

/* gutf8.c exports */
extern "C"
{
	int utf8_validate(const char *str, size_t max_len, const char **end);
	char *utf8_find_next_char(const char *p, const char *end = NULL);
	size_t utf8_strlen(const char *p, size_t max_bytes);
}


/* useful macros */
#ifdef HAVE_BOOST
#define FORMAT(FORMAT_FORMAT, FORMAT_DATA) boost::str(boost::wformat(FORMAT_FORMAT) % FORMAT_DATA)
#endif


/* Win32++ helpers */
#ifdef _WIN32_UI

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

#endif /* _WIN32_UI */

#ifdef _WIN32

class CBenchmarkTimer
{
public:
	CBenchmarkTimer();

	double GetFrequency();
	void StartTimer();
	double StopTimer();

protected:
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_stop;
	double m_frequency;
};

#endif /* _WIN32 */

#ifdef CAIRO_H

class _CCairoSurface
{
public:
	_CCairoSurface()
	{
		m_surface = NULL;
	}
	_CCairoSurface(cairo_surface_t *p)
	{
		m_surface = p;
	}
	virtual ~_CCairoSurface()
	{
		if(m_surface)
		{
			cairo_surface_destroy(m_surface);
		}
	}
	operator cairo_surface_t*() const
	{
		return m_surface;
	}
	operator bool() const
	{
		return (m_surface != NULL);
	}
protected:
	cairo_surface_t *m_surface;
};

#ifndef DONT_USE_SHARED_PTR
typedef shared_ptr<_CCairoSurface> PCairoSurface;
#else
typedef _CCairoSurface* PCairoSurface;
#endif

#endif /* CAIRO_H */

#endif /* !_UTIL_H */
