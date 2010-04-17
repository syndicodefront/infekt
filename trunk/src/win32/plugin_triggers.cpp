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

#include "stdafx.h"
#include "plugin_manager.h"

using namespace std;


void CPluginManager::TriggerNfoLoad(bool a_before, const std::wstring& a_filePath)
{
	std::_tstring l_fileName, l_filePath;

	if(a_before)
	{
		l_filePath = a_filePath;
	}
	else
	{
		PNFOData l_nfoData = GetAppView()->GetNfoData();

		l_fileName = l_nfoData->GetFileName();
		l_filePath = l_nfoData->GetFilePath();
	}

	infektDeclareStruct(infekt_nfo_info_t, l_info);
	l_info.fileName = l_fileName.c_str();
	l_info.filePath = l_filePath.c_str();

	TriggerRegEvents(REG_NFO_LOAD_EVENTS, (a_before ? IPV_NFO_LOAD_BEFORE : IPV_NFO_LOADED), 0, &l_info);
}


void CPluginManager::TriggerSettingsChanged()
{
	TriggerRegEvents(REG_SETTINGS_EVENTS, IPV_SETTINGS_CHANGED, 0, NULL);
}


bool CPluginManager::TriggerViewChanging(EMainView a_view) // :TODO:
{
	//if(TriggerRegEvents(REG_SETTINGS_EVENTS, IPV_NFO_VIEW_CHANGING, a_view, NULL) != IPE_STOP)
	return true;
}


void CPluginManager::TriggerViewChanged() // :TODO: view information
{
	TriggerRegEvents(REG_SETTINGS_EVENTS, IPV_NFO_VIEW_CHANGING, 0, NULL);
}
