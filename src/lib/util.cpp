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

#include "stdafx.h"
#include "util.h"

using namespace std;


/************************************************************************/
/* Character Set Conversion Functions                                   */
/************************************************************************/

#ifdef _WIN32

string CUtil::FromWideStr(const wstring& a_wideStr, unsigned int a_targetCodePage)
{
	std::string result;

	if (a_wideStr.empty())
	{
		return result;
	}

	if (a_wideStr.size() > (size_t)std::numeric_limits<int>::max())
	{
		// impossible to convert in one chunk.
		return result;
	}

	int resultLength = ::WideCharToMultiByte(
		a_targetCodePage, 0,
		a_wideStr.data(), (int)a_wideStr.size(),
		nullptr, 0, nullptr, nullptr);

	if (resultLength > 0)
	{
		result.resize(resultLength);

		::WideCharToMultiByte(
			a_targetCodePage, 0,
			a_wideStr.data(), (int)a_wideStr.size(),
			&result[0], (int)resultLength,
			nullptr, nullptr);
	}

	return result;
}


wstring CUtil::ToWideStr(const string& a_str, unsigned int a_originCodePage)
{
	std::wstring utf16;

	if (a_str.empty())
	{
		return utf16;
	}

	if (a_str.size() > (size_t)std::numeric_limits<int>::max())
	{
		// impossible to convert in one chunk.
		return utf16;
	}

	int utf16Length = ::MultiByteToWideChar(
		a_originCodePage, 0,
		a_str.data(), (int)a_str.size(),
		nullptr, 0);

	if (utf16Length > 0)
	{
		utf16.resize(utf16Length);

		::MultiByteToWideChar(
			a_originCodePage, 0,
			a_str.data(), (int)a_str.size(),
			&utf16[0], (int)utf16.size());
	}

	return utf16;
}


// ATTENTION CALLERS: a_buf must have 6 chars space.
bool CUtil::OneCharWideToUtf8(wchar_t a_char, char* a_buf)
{
	return (::WideCharToMultiByte(CP_UTF8, 0, &a_char, 1, a_buf, 7, nullptr, nullptr) > 0);
}

#else /* _WIN32 */

string CUtil::FromWideStr(const wstring& a_wideStr, unsigned int a_targetCodePage)
{
	const char* l_targetCodePage;

	switch (a_targetCodePage)
	{
	case CP_UTF8: l_targetCodePage = "UTF-8"; break;
	case CP_ACP: l_targetCodePage = "ISO-8859-1"; break;
	default:
		return "";
	}

	char *l_sResult = nullptr;
	if (iconv_string(l_targetCodePage, "wchar_t", (char*)a_wideStr.c_str(),
		(char*)(a_wideStr.c_str() + a_wideStr.size() + 1), &l_sResult, nullptr) >= 0)
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

	switch (a_originCodePage)
	{
	case CP_UTF8: l_originCodePage = "UTF-8"; break;
	case CP_ACP: l_originCodePage = "ISO-8859-1"; break;
	default:
		return L"";
	}

	wchar_t *l_wResult = nullptr;
	if (iconv_string("wchar_t", l_originCodePage,
		a_str.c_str(), a_str.c_str() + a_str.size() + 1,
		(char**)&l_wResult, nullptr) >= 0)
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
	char *l_buf = nullptr;
	size_t l_len = 9;
	wchar_t l_tmp[2] = { a_char, 0 };

	if (iconv_string("UTF-8", "wchar_t", (char*)&l_tmp, (char*)(&l_tmp + 1), &l_buf, &l_len) >= 0)
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

	if (l_pos != STRTYPE::npos)
		a_str.erase(0, l_pos);
	else
		a_str.clear();
}

template<typename STRTYPE> static void inline _StrTrimRight(STRTYPE& a_str, const STRTYPE a_chars)
{
	typename STRTYPE::size_type l_pos = a_str.find_last_not_of(a_chars);

	if (l_pos != STRTYPE::npos)
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
/* Other Str Tools                                                      */
/************************************************************************/

template<typename T> static T _StrReplace(const T& a_find, const T& a_replace, const T& a_input)
{
	typename T::size_type l_pos = a_input.find(a_find), l_prevPos = 0;
	T l_new;

	while (l_pos != T::npos)
	{
		l_new.append(a_input.substr(l_prevPos, l_pos - l_prevPos));

		l_new.append(a_replace);

		l_prevPos = l_pos + a_find.size();
		l_pos = a_input.find(a_find, l_prevPos);
	}

	if (l_prevPos == 0)
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
	return _StrReplace(a_find, a_replace, a_input);
}

std::wstring CUtil::StrReplace(const std::wstring& a_find, const std::wstring& a_replace, const std::wstring& a_input)
{
	return _StrReplace(a_find, a_replace, a_input);
}

template<typename T> static std::vector<T> _StrSplit(const T& a_str, const T& a_separator)
{
	std::vector<T> result;
	typename T::size_type prev_pos = 0, pos = a_str.find(a_separator);

	while (pos != T::npos)
	{
		result.push_back(a_str.substr(prev_pos, pos - prev_pos));

		prev_pos = pos + a_separator.size();
		pos = a_str.find(a_separator, prev_pos);
	}

	result.push_back(a_str.substr(prev_pos));

	return result;
}

std::vector<std::string> CUtil::StrSplit(const std::string& a_str, const std::string& a_separator)
{
	return _StrSplit(a_str, a_separator);
}

std::vector<std::wstring> CUtil::StrSplit(const std::wstring& a_str, const std::wstring& a_separator)
{
	return _StrSplit(a_str, a_separator);
}

/************************************************************************/
/* Misc                                                                 */
/************************************************************************/

static vector<int> _ParseVersionNumber(const wstring& vs)
{
	vector<int> ret;
	wstring l_buf;

	for (wstring::size_type p = 0; p < vs.size(); p++)
	{
		if (vs[p] == L'.')
		{
			ret.push_back(CUtil::StringToLong(l_buf));

			l_buf.clear();
		}
		else
		{
			l_buf += vs[p];
		}
	}

	if (!l_buf.empty())
	{
		ret.push_back(CUtil::StringToLong(l_buf));
	}
	else if (ret.empty())
	{
		ret.push_back(0);
	}

	while (!ret.empty() && ret.back() == 0)
	{
		ret.pop_back();
	}

	return ret;
}

int CUtil::VersionCompare(const wstring& a_vA, const wstring& a_vB)
{
	const vector<int> l_vA = _ParseVersionNumber(a_vA),
		l_vB = _ParseVersionNumber(a_vB);

	size_t l_max = std::min(l_vA.size(), l_vB.size());

	for (size_t p = 0; p < l_max; p++)
	{
		if (l_vA[p] < l_vB[p])
			return -1;
		else if (l_vA[p] > l_vB[p])
			return 1;
	}

	return static_cast<int>(l_vA.size() - l_vB.size());
}
