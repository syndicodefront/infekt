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

#pragma once

#define _TARGETVER_VISTA
#include "targetver.h"

#include <windows.h>
#include <windowsx.h>
#include <Uxtheme.h>
#include <tchar.h>
#include <objbase.h>
#include <thumbcache.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <queue>
#include <stack>
#include <limits>
#include <map>
#include <io.h>
#include <omp.h>

using std::shared_ptr;

#include <cairo-win32.h>
#include <pcre.h>

#ifdef _WIN64
typedef signed __int64 ssize_t;
#else
typedef signed int ssize_t;
#endif
#define _tstring wstring
#include "util.h"

#include "safe_release.h"
