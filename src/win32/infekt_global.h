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

#ifndef _INFEKT_GLOBAL_H
#define _INFEKT_GLOBAL_H

#define WM_LOAD_NFO (WM_APP + 30)
#define WM_RELOAD_NFO (WM_APP + 31)
#define WM_SYNC_PLUGIN_TO_CORE (WM_APP + 32)
#define INFEKT_MAIN_WINDOW_CLASS_NAME _T("iNFektMainWindow")

typedef enum _main_view_view_t
{
	MAIN_VIEW_RENDERED = 1,
	MAIN_VIEW_CLASSIC,
	MAIN_VIEW_TEXTONLY,

	_MAIN_VIEW_MAX
} EMainView;

#endif /* !_INFEKT_GLOBAL_H */
