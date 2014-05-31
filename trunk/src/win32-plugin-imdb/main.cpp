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
#include "infekt.h" // ONLY for plugins shipping with iNFekt

static CImdbPlugin* s_pPlugin = NULL;


/************************************************************************/
/* Helper Method that populates infekt_plugin_info_t                    */
/************************************************************************/

static void DoPluginInfo(infekt_plugin_info_t* a_info)
{
	strcpy_s(a_info->guid,			48,		MYGUID);
	wcscpy_s(a_info->name,			32,		L"IMDb Infobar");

	swprintf_s(a_info->version, 16, L"%d.%d.%d", INFEKT_VERSION_MAJOR, INFEKT_VERSION_MINOR, INFEKT_VERSION_REVISION);
	// This is okay ONLY! for plugins shipping with iNFekt, please do it like this
	//   in your own plugins:
	// wcscpy_s(a_info->version,		16,		L"0.1");

	wcscpy_s(a_info->description,	512,	L"Shows information about movies and TV shows from IMDb.");

	a_info->cap_infobar = true;
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
			DoPluginInfo(reinterpret_cast<infekt_plugin_info_t*>(pParam));
			return IPE_SUCCESS;
		}
		break;

	case IPV_PLUGIN_LOAD:
		if(pParam && !s_pPlugin)
		{
			s_pPlugin = new CImdbPlugin(reinterpret_cast<infekt_plugin_load_t*>(pParam));
			return IPE_SUCCESS;
		}
		break;

	case IPV_PLUGIN_UNLOAD:
		if(s_pPlugin)
		{
			delete s_pPlugin;
			s_pPlugin = NULL;
			return IPE_SUCCESS;
		}
		break;
	}

	return IPE_NOT_IMPLEMENTED;
}


extern "C" __declspec(dllexport) long _cdecl
	infektPluginVersion()
{
	const WORD l_minRequiredVersion = 0x001;

	return MAKELONG(INFEKT_PLUGIN_H_VERSION, l_minRequiredVersion);
}
