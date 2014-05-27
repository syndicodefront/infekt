/**
 * Copyright (C) 2014 cxxjoe
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

#ifndef _RESCENE_PLUGIN_H
#define _RESCENE_PLUGIN_H

#include "targetver.h"
#include <Windows.h>
#include "infekt-plugin.h"
#include <string>

#define MYGUID "{DA3D624D-933D-42CD-B15F-85A8AFA355C5}"


class CRescenePlugin
{
public:
	CRescenePlugin(const infekt_plugin_load_t* a_load);
	virtual ~CRescenePlugin();

	long PluginSend(infektPluginCallId lCall, long long lParam, void* pParam, void *pUser = NULL);

protected:
	static INFEKT_PLUGIN_METHOD(ResceneMainEventCallback);

	long TryLoadSrrToViewer(const infekt_file_format_open_info_t*);

private:
	infektPluginMethod m_fPluginToCore;
};


#endif /* !_RESCENE_PLUGIN_H */
