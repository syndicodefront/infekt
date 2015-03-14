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
/* Other Str Tools                                                      */
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

	while(pos != T::npos)
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
/* Reg Ex Utils                                                         */
/************************************************************************/

#define OVECTOR_SIZE 60

#ifdef INFEKT_REGEX_UTF16

wstring CRegExUtil::Replace(const wstring& a_subject, const wstring& a_pattern, const wstring& a_replacement, int a_flags)
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

bool CRegExUtil::DoesMatch(const wstring& a_subject, const wstring& a_pattern, int a_flags)
{
	const char *szErrDescr;
	int iErrOffset;

	pcre16* re = pcre16_compile(reinterpret_cast<PCRE_SPTR16>(a_pattern.c_str()),
		PCRE_UTF16 | a_flags,
		&szErrDescr, &iErrOffset, NULL);

	_ASSERT(re != NULL);

	if(re)
	{
		int match = pcre16_exec(re, NULL, reinterpret_cast<PCRE_SPTR16>(a_subject.c_str()),
			(int)a_subject.size(), 0, 0, NULL, 0);

		if(match != PCRE_ERROR_NOMATCH)
		{
			return true;
		}

		pcre16_free(re);
	}

	return false;
}

#elif defined(INFEKT_REGEX_UTF8)

string CRegExUtil::Replace(const string& a_subject, const string& a_pattern, const string& a_replacement, int a_flags)
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

        if((re = pcre_compile(a_pattern.c_str(), PCRE_UTF8 | PCRE_NEWLINE_ANYCRLF | a_flags, &szErrDescr, &iErrOffset, NULL)) != NULL)
        {
                int l_prevEndPos = 0; // the index of the character that follows the last character of the previous match.
                pcre_extra *pe = pcre_study(re, 0, &szErrDescr); // this could be NULL but it wouldn't matter.

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

bool CRegExUtil::DoesMatch(const string& a_subject, const string& a_pattern, int a_flags)
{
	const char *szErrDescr;
	int iErrOffset;

	pcre* re = pcre_compile(a_pattern.c_str(), PCRE_UTF8 | a_flags, &szErrDescr, &iErrOffset, NULL);

	_ASSERT(re != NULL);

	if(re)
	{
		int match = pcre_exec(re, NULL, a_subject.c_str(), (int)a_subject.size(), 0, 0, NULL, 0);

		if(match != PCRE_ERROR_NOMATCH)
		{
			return true;
		}

		pcre_free(re);
	}

	return false;
}

#endif

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
