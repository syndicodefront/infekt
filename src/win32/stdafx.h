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

/* set up WINVER & friends */
#include "targetver.h"

#define _WIN32_UI

#ifndef COMPACT_RELEASE
#define INFEKT_PLUGIN_HOST
#endif

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
#include <inttypes.h>
#include <limits>
#include <stdio.h>
#include <io.h>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <deque>
#include <set>
#include <memory>
#define _USE_MATH_DEFINES
#include <math.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <tchar.h>
#include <commdlg.h>
#include <windowsx.h>
#include <shlobj.h>

/* cairo and other lib headers */
#include <cairo-win32.h>
#include <pcre.h>

/* local headers */
#include "infekt.h"

#define _tstring wstring
#ifdef _WIN64
typedef signed __int64 ssize_t;
#else
typedef signed int ssize_t;
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1600
using std::shared_ptr;
#else if defined(HAVE_BOOST)
using boost::shared_ptr;
#endif

// disable "switch statement contains 'default' but no 'case' labels"
#pragma warning(disable : 4065)

#endif /* !_STDAFX_H */
