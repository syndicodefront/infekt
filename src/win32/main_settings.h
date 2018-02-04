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

#ifndef _MAIN_SETTINGS_H
#define _MAIN_SETTINGS_H

#include "infekt_global.h"

class CMainSettings
{
public:
	CMainSettings(bool bFromRegistry) :
		iDefaultView(-1),
		iLastView(MAIN_VIEW_RENDERED),
		bCopyOnSelect(false),
		bAlwaysOnTop(false),
		bAlwaysShowMenubar(false),
		bCheckDefaultOnStartup(false),
		bSingleInstanceMode(false),
		bKeepOpenMRU(true),
		bWrapLines(true),
		bCenterWindow(true),
		bAutoWidth(true),
		bAutoHeight(false),
		bCenterNFO(true),
		bDefaultExportToNFODir(false),
		bCloseOnEsc(true),
		bOnDemandRendering(true),
		bMonitorFileChanges(true),
		bUseGPU(true)
	{
		if(bFromRegistry)
		{
			LoadFromRegistry();
		}
	}

	bool LoadFromRegistry();
	bool SaveToRegistry();

	int32_t iDefaultView, iLastView;
	bool bCopyOnSelect;
	bool bAlwaysOnTop;
	bool bAlwaysShowMenubar;
	bool bCheckDefaultOnStartup;
	bool bSingleInstanceMode;
	bool bKeepOpenMRU;
	bool bWrapLines;

	bool bCenterWindow;
	bool bAutoWidth;
	bool bAutoHeight;
	bool bCenterNFO;
	bool bDefaultExportToNFODir;
	bool bCloseOnEsc;
	bool bOnDemandRendering;
	bool bMonitorFileChanges;
	bool bUseGPU;
};

typedef std::shared_ptr<CMainSettings> PMainSettings;

#endif /* !_MAIN_SETTINGS_H */
