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

#include "theme_api.h"

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

	static int VersionCompare(const std::_tstring& a_vA, const std::_tstring& a_vB);

	static std::string RegExReplaceUtf8(const std::string& a_subject, const std::string& a_pattern,
		const std::string& a_replacement, int a_flags = 0);

#ifdef _WIN32_UI
	static int AddPngToImageList(HIMAGELIST a_imgList,
		HINSTANCE a_instance, int a_resourceId, int a_width, int a_height);
	static std::_tstring OpenFileDialog(HINSTANCE a_instance, HWND a_parent, const LPCTSTR a_filter);
	static std::_tstring SaveFileDialog(HINSTANCE a_instance, HWND a_parent, const LPCTSTR a_filter,
		const LPCTSTR a_defaultExt, const std::_tstring& a_currentFileName);

	static std::_tstring DownloadHttpTextFile(const std::_tstring& a_url);
	static void PopUpLastWin32Error();

	static int StatusCalcPaneWidth(HWND hwnd, LPCTSTR lpsz);
#endif

#ifdef _WIN32
	static bool IsWin2000();
	static bool IsWinXP();
	static bool IsWin5x();
	static bool IsWin6x(bool a_orHigher = true);

	static std::_tstring GetExePath();
	static std::_tstring GetExeDir();
	static HMODULE SilentLoadLibrary(const std::_tstring a_path);
	static bool TextToClipboard(HWND a_hwnd, const std::wstring& a_text);

	static uint32_t RegQueryDword(HKEY a_key, const LPTSTR a_name, uint32_t a_default = 0);
protected:
	static OSVERSIONINFO ms_osver;
#endif
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


/* gutf8.c exports */
extern "C"
{
	int g_utf8_validate(const char *str, size_t max_len, const char **end);
	char *g_utf8_find_next_char(const char *p, const char *end = NULL);
	size_t g_utf8_strlen(const char *p, long max_bytes);
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

		LONG l_style = this->GetWindowLong(GWL_STYLE);
		if((l_style & TCS_OWNERDRAWFIXED) != 0)
		{
			this->SetWindowLong(GWL_STYLE, l_style & ~TCS_OWNERDRAWFIXED);
		}

		if(this->SendMessage(CCM_GETVERSION, 0, 0) >= 6)
		{
			// adjust XP style background...
			CThemeAPI::GetInstance()->EnableThemeDialogTexture(GetTabPageInfo(l_newIdx).pWnd->GetHwnd(), ETDT_ENABLETAB);
		}

		return l_newIdx;
	}
};

#endif /* _WIN32_UI */

#ifdef _WIN32

class CCudaUtil
{
public:
	static const CCudaUtil* GetInstance();
	virtual ~CCudaUtil();

	bool IsCudaUsable() const;
	bool InitCudaThread() const;
	bool IsCudaThreadInitialized() const;
	bool UnInitCudaThread() const;
	bool DoCudaBoxBlurA8(unsigned char* a_data, int a_stride, int a_rows, int a_lobes[3][2]) const;

protected:
	HMODULE m_hCudaBlur;
private:
	CCudaUtil();
};

#endif /* _WIN32 */


#endif /* !_UTIL_H */
