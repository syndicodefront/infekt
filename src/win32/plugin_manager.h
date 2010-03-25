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

	enum EPluginCaps
	{
		CAPAB_INFOBAR = 1
	};

	bool HasCapab(EPluginCaps a_cap) const { return (m_capabs & a_cap) != 0; }
protected:
	HMODULE m_hModule;
	std::string m_guid;
	std::wstring m_name, m_version, m_description;
	bool m_successfullyLoaded;
	unsigned long long m_capabs;
};

typedef boost::shared_ptr<CLoadedPlugin> PLoadedPlugin;


class CPluginManager
{
public:
	static CPluginManager* GetInstance();
	virtual ~CPluginManager();

	bool LoadPlugin(std::_tstring a_dllPath, bool a_probeInfoOnly = false);
	std::_tstring GetLastErrorMessage() const { return m_lastErrorMsg; }
	void GetLastProbedInfo(std::wstring& ar_name, std::wstring& ar_version, std::wstring& ar_description);

	// don't call this. it's for CLoadedPlugin only.
	static long _cdecl _pluginToCoreCallback(const char*, long, long, long long, void*, void*);
protected:
	std::map<std::string, PLoadedPlugin> m_loadedPlugins;
	std::_tstring m_lastErrorMsg;
	std::wstring m_probedName, m_probedVer, m_probedDescr;

	long DoGetLoadedNfoText(long long a_bufLen, void* a_buf, bool a_utf8);

	long PluginToCoreCallback(const char*, long, long, long long, void*, void*);
private:
	CPluginManager();
};


#endif /* !_PLUGIN_MANAGER_H */
