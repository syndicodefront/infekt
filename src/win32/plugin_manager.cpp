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
#include "app.h"

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


bool CPluginManager::LoadPlugin(_tstring a_dllPath, bool a_probeInfoOnly)
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

	if(a_probeInfoOnly)
	{
		m_probedName = l_info.name;
		m_probedVer = l_info.version;
		m_probedDescr = l_info.description;
		m_probedGuid = l_info.guid;

		FreeLibrary(l_hModule);

		return true;
	}

	if(m_loadedPlugins.find(l_info.guid) != m_loadedPlugins.end())
	{
		m_lastErrorMsg = _T("This plugin or a plugin that erroneously uses the same GUID has already been loaded.");
		FreeLibrary(l_hModule);
		return false;
	}

	PLoadedPlugin l_newPlugin = PLoadedPlugin(new CLoadedPlugin(a_dllPath, l_hModule, &l_info));

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


void CPluginManager::GetLastProbedInfo(std::string& ar_guid, std::wstring& ar_name, std::wstring& ar_version, std::wstring& ar_description)
{
	ar_guid = m_probedGuid;
	ar_name = m_probedName;
	ar_version = m_probedVer;
	ar_description = m_probedDescr;
}


bool CPluginManager::IsPluginLoaded(const std::string& a_guid) const
{
	return (m_loadedPlugins.find(a_guid) != m_loadedPlugins.end());
}


struct _sync_call_data
{
	const char* szGuid;
	long lCall;
	long long lParam;
	void* pParam;
	void* pUser;
};

/*static*/ INFEKT_PLUGIN_METHOD(CPluginManager::_pluginToCoreCallback)
{
	CPluginManager* l_mgr = CPluginManager::GetInstance();

	if(l_mgr)
	{
		_sync_call_data l_data;
		l_data.szGuid = szGuid;
		l_data.lCall = lCall;
		l_data.lParam = lParam;
		l_data.pParam = pParam;
		l_data.pUser = pUser;

		return (long)l_mgr->GetApp()->GetMainFrame().SendMessage(WM_SYNC_PLUGIN_TO_CORE, 0, (LPARAM)&l_data);
	}

	return IPE_NOT_IMPLEMENTED;
}


long CPluginManager::SynchedPluginToCore(void *a_data)
{
	const _sync_call_data* l_data = (_sync_call_data*)a_data;

	return PluginToCoreCallback(l_data->szGuid, l_data->lCall, l_data->lParam, l_data->pParam, l_data->pUser);
}


bool CPluginManager::UnLoadPlugin(const std::string& a_guid)
{
	TMGuidPlugins::iterator l_find = m_loadedPlugins.find(a_guid);
	if(l_find == m_loadedPlugins.end()) return false;

	m_loadedPlugins.erase(l_find);

	return true;
}


void CPluginManager::TriggerRegEvents(EPluginReg a_reg, infektPluginEventId a_event, long long a_lParam, void* a_pParam)
{
	for(TMGuidPlugins::iterator it = m_loadedPlugins.begin(); it != m_loadedPlugins.end(); it++)
	{
		it->second->TriggerRegEvent(a_reg, a_event, a_lParam, a_pParam);
	}
}


CPluginManager::~CPluginManager()
{
	if(s_plManInst == this) s_plManInst = NULL;
}


/************************************************************************/
/* CLoadedPlugin Implementation                                         */
/************************************************************************/

CLoadedPlugin::CLoadedPlugin(const std::_tstring& a_dllPath, HMODULE a_hModule, infekt_plugin_info_t* a_info)
{
	// initialize members:
	m_capabs = 0;
	m_activeRegBits = 0;

	// copy data:
	m_hModule = a_hModule;
	m_dllPath = a_dllPath;

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

		CNFOApp* l_app = dynamic_cast<CNFOApp*>(GetApp());

		// fill info for the plugin:
		l_loadInfo.pluginToCore = CPluginManager::_pluginToCoreCallback;
		l_loadInfo.hMainWindow = l_app->GetMainFrame().GetHwnd();

		const std::_tstring l_pluginDir = CUtil::PathRemoveFileSpec(m_dllPath);
		l_loadInfo.pluginDir = l_pluginDir.c_str();

		// send the load event to the plugin:
		long l_loadResult = l_fPluginMain(NULL, 0, IPV_PLUGIN_LOAD, 0, &l_loadInfo, NULL);

		if(l_loadResult == IPE_SUCCESS)
		{
			m_successfullyLoaded = true;
		}
	}

	return m_successfullyLoaded;
}


long CLoadedPlugin::AddReg(EPluginReg a_reg, infektPluginMethod a_callback, void* a_userData)
{
	if(!a_callback)
		return IPE_NULLCALLBACK;
	else if(HasRegSet(a_reg))
		return IPE_ALREADY;

	reg_event_data l_data = { a_callback, a_userData };
	m_activeRegs[a_reg] = l_data;
	m_activeRegBits = m_activeRegBits | a_reg;

	return IPE_SUCCESS;
}


long CLoadedPlugin::RemoveReg(EPluginReg a_reg, infektPluginMethod a_callback)
{
	if(!a_callback)
		return IPE_NULLCALLBACK;
	else if(!HasRegSet(a_reg))
		return IPE_NOT_FOUND;

	TMRegData::iterator l_remove = m_activeRegs.find(a_reg);

	if(l_remove != m_activeRegs.end())
	{
		m_activeRegs.erase(l_remove);
		m_activeRegBits = m_activeRegBits & ~a_reg;

		return IPE_SUCCESS;
	}

	return IPE_NOT_FOUND;
}


long CLoadedPlugin::TriggerRegEvent(EPluginReg a_reg, infektPluginEventId a_event, long long a_lParam, void* a_pParam)
{
	TMRegData::iterator l_itCallback = m_activeRegs.find(a_reg);

	if(l_itCallback != m_activeRegs.end())
	{
		return l_itCallback->second.pCallback(NULL, 0, a_event, a_lParam, a_pParam, l_itCallback->second.pUser);
	}

	return _IPE_NOT_IMPLEMENTED_INTERNAL;
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
