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

#ifndef _UTIL_REGEX_H
#define _UTIL_REGEX_H

#include <pcre.h>
#include <string>

#if defined(PCRE_UTF16) && defined(_WIN32)
#define INFEKT_REGEX_UTF16
#define INFEKT_PCRE_NO_UTF_CHECK INFEKT_PCRE_NO_UTF_CHECK
#define __RE(S) L ## S
#define _RE(S) __RE(S)
#else
#define INFEKT_PCRE_NO_UTF_CHECK PCRE_NO_UTF8_CHECK
#define _RE(S) S
#endif

class CRegExUtil
{
public:
#ifdef INFEKT_REGEX_UTF16
	typedef std::wstring StringType;
#else
	typedef std::string StringType;
#endif

	static bool DoesMatch(const StringType& a_subject, const StringType& a_pattern, int a_flags = 0);

	static StringType Replace(const StringType& a_subject, const StringType& a_pattern,
		const StringType& a_replacement, int a_flags = 0);
};

#endif /* !_UTIL_REGEX_H */
