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

class CLoadedPlugin
{
public:
	CLoadedPlugin(HMODULE a_hModule, infekt_plugin_info_t* a_info);
	virtual ~CLoadedPlugin();

	bool _DoLoad();

	enum EPluginCap
	{
		CAPAB_INFOBAR = 1
	};

	enum EPluginReg
	{
		REG_NFO_LOAD_EVENTS = 1,
		REG_NFO_VIEW_EVENTS = 2,
		REG_SETTINGS_EVENTS = 4
	};

	bool HasCapab(EPluginCap a_cap) const { return (m_capabs & a_cap) != 0; }
	bool HasRegSet(EPluginReg a_reg) const { return (m_activeRegBits & a_reg) != 0; }

	long AddReg(EPluginReg a_reg, infektPluginMethod a_callback, void* a_userData);
	long RemoveReg(EPluginReg a_reg, infektPluginMethod a_callback);
protected:
	HMODULE m_hModule;
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

	typedef std::multimap<EPluginReg, reg_event_data> TMRegData;
	TMRegData m_activeRegs;
};

typedef boost::shared_ptr<CLoadedPlugin> PLoadedPlugin;


class CPluginManager
{
public:
	static CPluginManager* GetInstance();
	virtual ~CPluginManager();

	bool LoadPlugin(std::_tstring a_dllPath, bool a_probeInfoOnly = false);
	std::_tstring GetLastErrorMessage() const { return m_lastErrorMsg; }
	void GetLastProbedInfo(std::string& ar_guid, std::wstring& ar_name, std::wstring& ar_version, std::wstring& ar_description);

	bool IsPluginLoaded(const std::string& a_guid) const;

	// don't call this. it's for CLoadedPlugin only.
	static INFEKT_PLUGIN_METHOD(_pluginToCoreCallback);
protected:
	typedef std::map<std::string, PLoadedPlugin> TMGuidPlugins;

	TMGuidPlugins m_loadedPlugins;
	std::_tstring m_lastErrorMsg;
	std::wstring m_probedName, m_probedVer, m_probedDescr;
	std::string m_probedGuid;

	long DoGetLoadedNfoText(long long a_bufLen, void* a_buf, bool a_utf8);
	long DoRegister(const std::string& a_guid, bool a_unregister, CLoadedPlugin::EPluginReg a_regType, void* a_pParam, void* a_userData);

	long PluginToCoreCallback(const char*, long, long, long long, void*, void*);
private:
	CPluginManager();
};


#endif /* !_PLUGIN_MANAGER_H */
