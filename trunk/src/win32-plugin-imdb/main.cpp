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

#include "imdb-plugin.h"

static infektPluginMethod s_fPluginToCore = NULL;


/************************************************************************/
/* PluginToCore wrapper method                                          */
/************************************************************************/

long PluginSend(infektPluginCallId lCall, long long lParam, void* pParam)
{
	if(s_fPluginToCore)
	{
		return s_fPluginToCore(MYGUID, 0, lCall, lParam, pParam, NULL);
	}

	return IPE_NOT_IMPLEMENTED;
}


/************************************************************************/
/* Methods for infektPluginMain                                         */
/************************************************************************/

static void DoPluginInfo(infekt_plugin_info_t* a_info)
{
	strcpy_s(a_info->guid,			48,		MYGUID);
	wcscpy_s(a_info->name,			32,		L"IMDb Infobar");
	wcscpy_s(a_info->version,		16,		L"0.1");

	wcscpy_s(a_info->description,	512,	L"For each NFO file that carries an IMDb link, this plugin "
		L"downloads and displays information about the movie or TV show.");

	a_info->cap_infobar = true;
}


static long DoPluginLoad(infekt_plugin_load_t* a_load)
{
	s_fPluginToCore = a_load->pluginToCore;

	PluginSend(IPCI_REGISTER_NFO_LOAD_EVENTS, 0, ImdbMainEventCallback);

	return IPE_SUCCESS;
}


static void DoPluginUnLoad()
{
}


/************************************************************************/
/* DLL Exports                                                          */
/************************************************************************/

extern "C" __declspec(dllexport)
	INFEKT_PLUGIN_METHOD(infektPluginMain)
{
	switch(lCall)
	{
	case IPV_PLUGIN_INFO:
		if(pParam)
		{
			DoPluginInfo((infekt_plugin_info_t*)pParam);
			return IPE_SUCCESS;
		}
		break;

	case IPV_PLUGIN_LOAD:
		return DoPluginLoad((infekt_plugin_load_t*)pParam);

	case IPV_PLUGIN_UNLOAD:
		DoPluginUnLoad();
		return IPE_SUCCESS;
	}

	return IPE_NOT_IMPLEMENTED;
}


extern "C" __declspec(dllexport) long _cdecl
	infektPluginVersion()
{
	const WORD l_minRequiredVersion = 0x001;

	return MAKELONG(INFEKT_PLUGIN_H_VERSION, l_minRequiredVersion);
}
