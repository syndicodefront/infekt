/**
 * Copyright (C) 2010 syndicode
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

#ifndef _INFEKT_POSIX_H
#define _INFEKT_POSIX_H

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wctype.h>
#include <limits.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <libgen.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "iconv_string.h"

#define TCHAR char
#define _tstring string
#define _T(STR) STR
#define _tprintf printf
#define _ftprintf fprintf
#define _tstoi atoi

#define _fileno fileno
#define _filelength filelength
/* :TODO: make sure we don't rely on the _s functionality without our own
	checks in too many places. */
#define fread_s(b, s, e, c, f) fread(b, e, c, f)
#define memmove_s(d, e, s, c) memmove(d, s, c)
#define strcpy_s(d, e, s) strcpy(d, s)
#define sscanf_s sscanf

#define _tcscpy_s(d, e, s) strcpy(d, s)
#define _tcsncpy_s(d, e, s, n) strncpy(d, s, n)
#define _tcscmp strcmp
#define _tcsicmp strcasecmp
#define _stricmp strcasecmp
#define _snprintf snprintf

#define wcscpy_s(d, e, s) wcscpy(d, s)
#define wcsncpy_s(d, e, s, n) wcsncpy(d, s, n)

#ifdef _DEBUG
#define _ASSERT(EXPR) assert(EXPR)
#else
#define _ASSERT(EXPR)
#endif

#define LF_FACESIZE 128
#define CP_UTF8 8
#define CP_ACP 5

#endif /* !_INFEKT_POSIX_H */
