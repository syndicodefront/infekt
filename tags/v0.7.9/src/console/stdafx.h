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


#ifndef _STDAFX_H
#define _STDAFX_H

#ifdef _WIN32
#include "targetver.h"
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>
#include <io.h>
#include <conio.h>
#define _tstring wstring
#else
#include "infekt-posix.h"
#endif

/* standard / system headers */
#include <inttypes.h>
#include <limits>
#include <stdio.h>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <deque>
#include <math.h>
#ifdef HAVE_BOOST
#include <boost/format.hpp>
#endif

/* cairo and other lib headers */
#ifdef _WIN32
#include <cairo-win32.h>
#else
#include <cairo.h>
#endif
#include <pcre.h>

/* local headers */
#include "infekt.h"

#endif /* !_STDAFX_H */
