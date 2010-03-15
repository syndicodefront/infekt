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
#include "util.h"

using namespace std;


/************************************************************************/
/* Plugin Manager Implementation                                        */
/************************************************************************/

CPluginManager::CPluginManager()
{
}

static CPluginManager* s_plManInst = NULL;

CPluginManager* CPluginManager::GetInstance()
{
	if(!s_plManInst)
	{
		s_plManInst = new CPluginManager();
	}

	return s_plManInst;
}


bool CPluginManager::LoadPlugin(_tstring a_dllPath)
{
	HMODULE l_hModule = CUtil::SilentLoadLibrary(a_dllPath);

	if(!l_hModule)
	{
		m_lastErrorMsg = _T("Unable to open/load DLL file.");
		return false;
	}

	typedef long (*infektPluginVersion)();

	infektPluginVersion l_fPluginVersion = (infektPluginVersion)GetProcAddress(l_hModule, "infektPluginVersion");

	if(!l_fPluginVersion)
	{
		m_lastErrorMsg = _T("Unable to find plugin version information.");
		FreeLibrary(l_hModule);
		return false;
	}

	long l_pluginVerInfo = l_fPluginVersion();
	WORD l_pluginMinVer = HIWORD(l_pluginVerInfo);
	WORD l_pluginHeaderVer = LOWORD(l_pluginVerInfo);

	if(l_pluginMinVer > INFEKT_PLUGIN_H_VERSION)
	{
		m_lastErrorMsg = _T("This plugin requires a newer version of iNFekt.");
		FreeLibrary(l_hModule);
		return false;
	}

	// l_pluginHeaderVer allows us to check for completely unsupported versions,
	// work around old version's bugs and stuff like that.

	infektPluginMethod l_fPluginMain = (infektPluginMethod)GetProcAddress(l_hModule, "infektPluginMain");

	if(!l_fPluginMain)
	{
		m_lastErrorMsg = _T("The DLL does not contain a method named 'infektPluginMain'.");
		FreeLibrary(l_hModule);
		return false;
	}

	infektDeclareStruct(infekt_plugin_info_t, l_info);
	long l_infoResult = l_fPluginMain(NULL, 0, IPV_PLUGIN_INFO, 0, &l_info, NULL);

	if(l_infoResult != IPE_SUCCESS || !l_info.guid[0])
	{
		m_lastErrorMsg = _T("The plugin did not properly respond to our request for information.");
		FreeLibrary(l_hModule);
		return false;
	}

	if(m_loadedPlugins.find(l_info.guid) != m_loadedPlugins.end())
	{
		m_lastErrorMsg = _T("This plugin or a plugin that erroneously uses the same GUID has already been loaded.");
		FreeLibrary(l_hModule);
		return false;
	}

	PLoadedPlugin l_newPlugin = PLoadedPlugin(new CLoadedPlugin(l_hModule, &l_info));

	m_loadedPlugins[l_info.guid] = l_newPlugin;

	if(l_newPlugin->_DoLoad())
	{
		return true;
	}
	else
	{
		m_loadedPlugins.erase(l_info.guid);

		m_lastErrorMsg = _T("The plugin's load routine returned an error code.");

		// PLoadedPlugin will free the instance and close the HMODULE.
		return false;
	}
}


/*static*/ long _cdecl CPluginManager::_pluginToCoreCallback(const char* szGuid, long lReserved, long lCall, long long lParam, void* pParam, void* pUser)
{
	CPluginManager* l_mgr = CPluginManager::GetInstance();

	if(l_mgr)
	{
		return l_mgr->PluginToCoreCallback(szGuid, lReserved, lCall, lParam, pParam, pUser);
	}

	return IPE_NOT_IMPLEMENTED;
}


long CPluginManager::PluginToCoreCallback(const char* szGuid, long lReserved, long lCall, long long lParam, void* pParam, void* pUser)
{
	/*switch(lCall)
	{
	}*/

	return IPE_NOT_IMPLEMENTED;
}


CPluginManager::~CPluginManager()
{
	if(s_plManInst == this) s_plManInst = NULL;
}


/************************************************************************/
/* CLoadedPlugin Implementation                                         */
/************************************************************************/

CLoadedPlugin::CLoadedPlugin(HMODULE a_hModule, infekt_plugin_info_t* a_info)
{
	// initialize members:
	m_capabs = 0;

	// copy data:
	m_hModule = a_hModule;

	m_guid = a_info->guid;
	m_name = a_info->name;
	m_version = a_info->version;
	m_description = a_info->description;

	if(a_info->cap_infobar) m_capabs |= CAPAB_INFOBAR;

	m_successfullyLoaded = false;
}


bool CLoadedPlugin::_DoLoad()
{
	if(m_successfullyLoaded)
		return false;

	infektPluginMethod l_fPluginMain = (infektPluginMethod)GetProcAddress(m_hModule, "infektPluginMain");

	if(l_fPluginMain)
	{
		infektDeclareStruct(infekt_plugin_load_t, l_loadInfo);

		l_loadInfo.pluginToCore = CPluginManager::_pluginToCoreCallback;

		long l_loadResult = l_fPluginMain(NULL, 0, IPV_PLUGIN_LOAD, 0, &l_loadInfo, NULL);

		if(l_loadResult == IPE_SUCCESS)
		{
			m_successfullyLoaded = true;
		}
	}

	return m_successfullyLoaded;
}


CLoadedPlugin::~CLoadedPlugin()
{
	if(m_successfullyLoaded)
	{
		infektPluginMethod l_fPluginMain = (infektPluginMethod)GetProcAddress(m_hModule, "infektPluginMain");

		if(l_fPluginMain)
		{
			l_fPluginMain(NULL, 0, IPV_PLUGIN_UNLOAD, 0, NULL, NULL);
		}
	}

	FreeLibrary(m_hModule);
}
