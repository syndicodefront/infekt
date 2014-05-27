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

#ifndef _PLUGIN_MANAGER_H
#define _PLUGIN_MANAGER_H

#include "infekt-plugin.h"
#include "app.h"
#include "win_http_client.h"


typedef enum
{
	REG_NFO_LOAD_EVENTS = 1,
	REG_NFO_VIEW_EVENTS = 2,
	REG_SETTINGS_EVENTS = 4,
	REG_FILE_FORMAT_SUPPORT_EVENTS = 8,
} EPluginReg;


class CLoadedPlugin
{
public:
	CLoadedPlugin(const std::wstring& a_dllPath, HMODULE a_hModule, infekt_plugin_info_t* a_info);
	virtual ~CLoadedPlugin();

	bool _DoLoad();

	const std::wstring& GetDllPath() const { return m_dllPath; }

	typedef enum
	{
		CAPAB_INFOBAR = 1
	} EPluginCap;

	bool HasCapab(EPluginCap a_cap) const { return (m_capabs & a_cap) != 0; }
	bool HasRegSet(EPluginReg a_reg) const { return (m_activeRegBits & a_reg) != 0; }

	long AddReg(EPluginReg a_reg, infektPluginMethod a_callback, void* a_userData);
	long RemoveReg(EPluginReg a_reg, infektPluginMethod a_callback);

	long TriggerRegEvent(EPluginReg a_reg, infektPluginEventId a_event, long long a_lParam, void* a_pParam) const;

	PWinHttpClient GetHttpClient();
protected:
	HMODULE m_hModule;
	std::wstring m_dllPath;
	std::string m_guid;
	std::wstring m_name, m_version, m_description;
	bool m_successfullyLoaded;
	unsigned long long m_capabs;
	unsigned long long m_activeRegBits;

	struct reg_event_data
	{
		infektPluginMethod pCallback;
		void* pUser;
	};

	typedef std::map<EPluginReg, reg_event_data> TMRegData;
	TMRegData m_activeRegs;

	PWinHttpClient m_httpClient;
};

typedef shared_ptr<CLoadedPlugin> PLoadedPlugin;


class CPluginManager
{
public:
	static CPluginManager* GetInstance();
	virtual ~CPluginManager();

	// real managing stuff:
	bool LoadPlugin(std::wstring a_dllPath, bool a_probeInfoOnly = false);
	std::wstring GetLastErrorMessage() const { return m_lastErrorMsg; }
	void GetLastProbedInfo(std::string& ar_guid, std::wstring& ar_name, std::wstring& ar_version, std::wstring& ar_description) const;

	bool IsPluginLoaded(const std::string& a_guid) const;
	bool UnLoadPlugin(const std::string& a_guid);

	void GetLoadedPlugins(std::vector<const std::wstring>& ar_dllPaths);

	// event triggers (app -> plugins):
	void TriggerNfoLoad(bool a_before, const std::wstring& a_filePath);
	void TriggerSettingsChanged();
	bool TriggerViewChanging(EMainView a_view);
	void TriggerViewChanged();
	bool TriggerTryOpenFileFormat(const char *a_buf, size_t a_bufLen, const std::wstring& a_filePath);

	// don't call this. it's for CLoadedPlugin only.
	static INFEKT_PLUGIN_METHOD(_pluginToCoreCallback);
	// don't call this either, it's used for thread sync:
	long SynchedPluginToCore(void *a_data);
protected:
	typedef std::map<std::string, PLoadedPlugin> TMGuidPlugins;

	TMGuidPlugins m_loadedPlugins;
	std::wstring m_lastErrorMsg;
	std::wstring m_probedName, m_probedVer, m_probedDescr;
	std::string m_probedGuid;

	CNFOApp* GetApp() { return CNFOApp::GetInstance(); }
	CViewContainer* GetAppView() { return CNFOApp::GetViewContainerInstance(); }

	// plugin -> core implementation things:
	long PluginToCoreCallback(const char*, long, long long, void*, void*);

	long DoGetLoadedNfoText(long a_bufLen, void* a_buf, bool a_utf8);
	long DoEnumLoadedNfoLinks(void* a_pCallback, void* a_pUser);
	long DoRegister(const std::string& a_guid, bool a_unregister, EPluginReg a_regType, void* a_pParam, void* a_userData);
	long DoHttpRequest(const std::string& a_guid, const infekt_http_request_t* a_pReq, void* a_pUser);
	long DoShowNfo(const infekt_show_nfo_t* a_nfo);

	// core -> plugin stuff:
	void TriggerRegEvents(EPluginReg a_reg, infektPluginEventId a_event, long long a_lParam, void* a_pParam);
private:
	CPluginManager();
};


#endif /* !_PLUGIN_MANAGER_H */
