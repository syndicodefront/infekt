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
#include <map>
#include <vector>
#include <deque>
#include <set>
#define _USE_MATH_DEFINES
#include <math.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <tchar.h>
#include <commdlg.h>
#include <windowsx.h>

/* cairo and other lib headers */
#include <cairo-win32.h>

/* local headers */
#include "infekt.h"

#ifndef _WIN32
#define _tstring string
#else
#define _tstring wstring
#endif

#endif /* !_STDAFX_H */
