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

#ifndef _STDAFX_H
#define _STDAFX_H

/* set up WINVER & friends */
#include "targetver.h"

#define _WIN32_UI
#define INFEKT_PLUGIN_HOST

/* Win32++ headers */
#include <wincore.h>
#include <dialog.h>
#include <docking.h>
#include <frame.h>
#include <gdi.h>
#include <listview.h>
#include <mdi.h>
#include <propertysheet.h>
#include <rebar.h>
#include <socket.h>
#include <splitter.h>
#include <statusbar.h>
#include <toolbar.h>
#include <treeview.h>

/* standard / windows headers */
#include <limits>
#include <stdio.h>
#include <io.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <queue>
#include <stack>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <regex>
#include <functional>
#include <omp.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <boost/algorithm/string.hpp>
#include <tchar.h>
#include <commdlg.h>
#include <windowsx.h>
#include <shlobj.h>

/* cairo and other lib headers */
#include <cairo-win32.h>

/* local headers */
#include "infekt.h"

#define _tstring wstring
#ifdef _WIN64
typedef signed __int64 ssize_t;
#else
typedef signed int ssize_t;
#endif

// disable "switch statement contains 'default' but no 'case' labels"
#pragma warning(disable : 4065)

#endif /* !_STDAFX_H */
