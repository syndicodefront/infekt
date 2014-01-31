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

#include "stdafx.h"
#include "main_settings.h"
#include "settings_backend.h"
#include "app.h"


/************************************************************************/
/* CMAINSETTINGS                                                        */
/************************************************************************/

bool CMainSettings::SaveToRegistry()
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForWriting(L"MainSettings", l_sect))
	{
		return false;
	}

	l_sect->WriteDword(L"DefaultView", this->iDefaultView);
	l_sect->WriteDword(L"LastView", this->iLastView);
	l_sect->WriteBool(L"CopyOnSelect", this->bCopyOnSelect);
	l_sect->WriteBool(L"AlwaysOnTop", this->bAlwaysOnTop);
	l_sect->WriteBool(L"AlwaysShowMenubar", this->bAlwaysShowMenubar);
	l_sect->WriteBool(L"CheckDefViewOnStart", this->bCheckDefaultOnStartup);
	l_sect->WriteBool(L"KeepOpenMRU", this->bKeepOpenMRU);

	l_sect->WriteBool(L"CenterWindow", this->bCenterWindow);
	l_sect->WriteBool(L"AutoWidth", this->bAutoWidth);
	l_sect->WriteBool(L"CenterNFO", this->bCenterNFO);
	l_sect->WriteBool(L"DefaultExportToNFODir", this->bDefaultExportToNFODir);
	l_sect->WriteBool(L"CloseOnEsc", this->bCloseOnEsc);
	l_sect->WriteBool(L"OnDemandRendering", this->bOnDemandRendering);
	l_sect->WriteBool(L"MonitorFileChanges", this->bMonitorFileChanges);
	l_sect->WriteBool(L"UseGPU", this->bUseGPU);

	// "deputy" return value:
	return l_sect->WriteBool(L"SingleInstanceMode", this->bSingleInstanceMode);
}


bool CMainSettings::LoadFromRegistry()
{
	PSettingsSection l_sect;

	if(!dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->OpenSectionForReading(L"MainSettings", l_sect))
	{
		return false;
	}

	DWORD dwDefaultView = l_sect->ReadDword(L"DefaultView"),
		dwLastView = l_sect->ReadDword(L"LastView");

	if(dwDefaultView == -1 || (dwDefaultView >= MAIN_VIEW_RENDERED && dwDefaultView < _MAIN_VIEW_MAX))
	{
		this->iDefaultView = dwDefaultView;
	}

	if(dwLastView >= MAIN_VIEW_RENDERED && dwLastView < _MAIN_VIEW_MAX)
	{
		this->iLastView = dwLastView;
	}

	this->bCopyOnSelect = l_sect->ReadBool(L"CopyOnSelect", false);
	this->bAlwaysOnTop = l_sect->ReadBool(L"AlwaysOnTop", false);
	this->bAlwaysShowMenubar = l_sect->ReadBool(L"AlwaysShowMenubar", false);
	this->bCheckDefaultOnStartup = l_sect->ReadBool(L"CheckDefViewOnStart", true);
	this->bSingleInstanceMode = l_sect->ReadBool(L"SingleInstanceMode", false);
	this->bKeepOpenMRU = l_sect->ReadBool(L"KeepOpenMRU", true);

	this->bCenterWindow = l_sect->ReadBool(L"CenterWindow", true);
	this->bAutoWidth = l_sect->ReadBool(L"AutoWidth", true);
	this->bCenterNFO = l_sect->ReadBool(L"CenterNFO", true);
	this->bDefaultExportToNFODir = l_sect->ReadBool(L"DefaultExportToNFODir", false);
	this->bCloseOnEsc = l_sect->ReadBool(L"CloseOnEsc", false);
	this->bOnDemandRendering = l_sect->ReadBool(L"OnDemandRendering", true);
	this->bMonitorFileChanges = l_sect->ReadBool(L"MonitorFileChanges", true);
	this->bUseGPU = l_sect->ReadBool(L"UseGPU", true);

	return true;
}
