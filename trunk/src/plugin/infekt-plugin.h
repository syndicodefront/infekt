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

#ifndef _INFEKT_PLUGIN_H
#define _INFEKT_PLUGIN_H

#define INFEKT_PLUGIN_H_VERSION ((WORD)0x001)


// pretty much all calls between plugins and the core use this signature:
typedef long (*infektPluginMethod)(const char* szGuid, long lReserved, long lCall,
	long long lParam, void* pParam, void* pUser);


// call IDs for infektPluginMethod's lCall (plugin -> core):
typedef enum {
	IPCI_REGISTER_NFO_LOAD_EVENTS = 1001,
	IPCI_GET_LOADED_NFO_INFO,
	IPCI_GET_LOADED_NFO_TEXTW,
	IPCI_GET_LOADED_NFO_TEXTUTF8,

	_IPCI_MAX
} infektPluginCallId;


// call IDs for infektPluginMethod's lCall (core -> plugin) aka event:
typedef enum {
	IPV_PLUGIN_INFO = 2001,
	IPV_PLUGIN_LOAD,
	IPV_PLUGIN_UNLOAD,

	IPV_NFO_LOAD_BEFORE = 3001,
	IPV_NFO_LOADED,

	_IPV_MAX
} infektPluginEvent;


// error/return codes, used in various places, most notably as infektPluginMethod's return value:
typedef enum {
	IPE_SUCCESS = 0,
	_IPE_RESERVED = -65536,
	IPE_NOT_IMPLEMENTED,
	IPE_NO_FILE,
	IPE_BUF_TOO_SMALL,
	IPE_TOO_LARGE,

	_IPE_MAX
} infektPluginError;


// use this to initialize structs before passing them around:
#define infektInitStruct(VAR) do { memset(&VAR, 0, sizeof(VAR)); VAR._uSize = sizeof(VAR); } while(0)
#define infektDeclareStruct(TYPE, VAR) TYPE VAR = {0}; VAR._uSize = sizeof(TYPE)


struct infekt_plugin_info_t {
	size_t _uSize;

	// info that plugins need to fill out:
	char guid[48];
	wchar_t name[32];
	wchar_t version[16];
	wchar_t description[512];

	bool cap_infobar;
};


struct infekt_plugin_load_t {
	size_t _uSize;

	// info given to plugins:
	infektPluginMethod pluginToCore;
	HWND hMainWindow;
};


struct infekt_nfo_into_t {
	size_t _uSize;

	wchar_t* fileName;
	wchar_t* filePath;
};


#endif /* !_INFEKT_PLUGIN_H */
