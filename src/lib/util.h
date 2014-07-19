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

#ifndef _UTIL_H
#define _UTIL_H

#ifdef _WIN32
#include "util_win32.h"
#endif
#ifdef _WIN32_UI
#include "util_win32_gui.h"
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

	static std::string StrReplace(const std::string& a_find, const std::string& a_replace, const std::string& a_input);
	static std::wstring StrReplace(const std::wstring& a_find, const std::wstring& a_replace, const std::wstring& a_input);

	static std::vector<std::string> StrSplit(const std::string& a_str, const std::string& a_separator);
	static std::vector<std::wstring> StrSplit(const std::wstring& a_str, const std::wstring& a_separator);
};


#ifdef PCRE_UTF16
class CRegExUtil
{
public:
	static bool DoesMatch(const std::wstring& a_subject, const std::wstring& a_pattern, int a_flags = 0);

	static std::wstring Replace(const std::wstring& a_subject, const std::wstring& a_pattern,
		const std::wstring& a_replacement, int a_flags = 0);
};
#endif


template <typename T> class TwoDimVector
{
public:
	TwoDimVector(size_t a_rows, size_t a_cols, const T a_initial) :
		m_rows(a_rows),
		m_cols(a_cols),
		m_data(a_rows, std::vector<T>(a_cols, a_initial))
	{
	}

	std::vector<T> & operator[](size_t i) 
	{ 
		return m_data[i];
	}

	const std::vector<T> & operator[](size_t i) const
	{ 
		return m_data[i];
	}

	size_t GetRows() const { return m_rows; }
	size_t GetCols() const { return m_cols; }

	void Extend(size_t a_newRows, size_t a_newCols, const T a_initial)
	{
		if(a_newRows != m_rows)
		{
			m_data.resize(m_rows = a_newRows, std::vector<T>(a_newCols, a_initial));
		}

		if(a_newCols != m_cols)
		{
			for(auto it = m_data.begin(); it != m_data.end(); ++it)
			{
				it->resize(a_newCols, a_initial);
			}

			m_cols = a_newCols;
		}
	}

private:
	std::vector<std::vector<T> > m_data;
	size_t m_rows, m_cols;

	TwoDimVector() {}
};

template <typename T> int sgn(T val) {
    return (val > T(0)) - (val < T(0));
};


/************************************************************************/
/* Helper: auto-freeing RAII buffer                                     */
/************************************************************************/

template<typename T> class CAutoFreeBuffer
{
public:
	CAutoFreeBuffer(size_t a_bufSize)
	{
		m_bufSize = a_bufSize;
		m_buf = new T[m_bufSize];
		memset(m_buf, 0, a_bufSize);
	}

	CAutoFreeBuffer(const CAutoFreeBuffer& a_from)
	{
		m_bufSize = a_from.m_bufSize;
		m_buf = new T[m_bufSize];
		memmove_s(m_buf, m_bufSize, a_from.m_buf, m_bufSize);
	}

	virtual ~CAutoFreeBuffer()
	{
		delete[] m_buf;
	}

	T* get() { return m_buf; }
private:
	T* m_buf;
	size_t m_bufSize;
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

typedef shared_ptr<_CCairoSurface> PCairoSurface;

#endif /* CAIRO_H */

#endif /* !_UTIL_H */
