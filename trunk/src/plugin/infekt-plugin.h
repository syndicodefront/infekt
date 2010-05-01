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

#include <windows.h>
#include <string.h>


// This will always have to be a word. It should not be changed a lot
// since all versions/calls are supposed to be backwards compatible.
#define INFEKT_PLUGIN_H_VERSION ((WORD)0x001)


// pretty much all calls between plugins and the core use this signature:
typedef long (_cdecl *infektPluginMethod)(const char* szGuid, long lReserved, long lCall,
	long long lParam, void* pParam, void* pUser);

// useful for both declarations and method bodies:
#define INFEKT_PLUGIN_METHOD(NAME) long _cdecl NAME(const char* szGuid, long lReserved, long lCall, \
	long long lParam, void* pParam, void* pUser)


// call IDs for infektPluginMethod's lCall (plugin -> core):
typedef enum {
	IPCI_GET_LOADED_NFO_TEXTWIDE = 1001,
	IPCI_GET_LOADED_NFO_TEXTUTF8,
	IPCI_ENUM_LOADED_NFO_LINKS,

	IPCI_REGISTER_NFO_LOAD_EVENTS,
	IPCI_UNREGISTER_NFO_LOAD_EVENTS,
	IPCI_REGISTER_NFO_VIEW_EVENTS,
	IPCI_UNREGISTER_NFO_VIEW_EVENTS,
	IPCI_REGISTER_SETTINGS_EVENTS,
	IPCI_UNREGISTER_SETTINGS_EVENTS,

	IPCI_INFOBAR_ANNOUNCE,
	IPCI_INFOBAR_CREATE,

	_IPCI_MAX
} infektPluginCallId;


// call IDs for infektPluginMethod's lCall (core -> plugin) aka event:
typedef enum {
	// events sent to infektPluginMain:
	IPV_PLUGIN_INFO = 5001,
	IPV_PLUGIN_LOAD,
	IPV_PLUGIN_UNLOAD,

	// multi-purpose enumeration callback events:
	IPV_ENUM_BEGIN = 5101,
	IPV_ENUM_ITEM,
	IPV_ENUM_END,

	// register for these events using IPCI_REGISTER_NFO_LOAD_EVENTS:
	IPV_NFO_LOAD_BEFORE = 6001,
	IPV_NFO_LOADED,

	// register for these events using IPCI_REGISTER_NFO_VIEW_EVENTS:
	IPV_NFO_VIEW_CHANGING,
	IPV_NFO_VIEW_CHANGED,
	IPV_NFO_VIEW_SETTINGS_CHANGED,

	// register for these events using IPCI_REGISTER_SETTINGS_EVENTS:
	IPV_SETTINGS_CHANGED,

	// plugins with cap_infobar will get this
	// (if they also sent IPCI_INFOBAR_ANNOUNCE):
	IPV_INFOBAR_REQUEST,

	_IPV_MAX
} infektPluginEventId;


// error/return codes, used in various places, most notably as infektPluginMethod's return value:
typedef enum {
	// don't use !blah, use == IPE_SUCCESS. Please.
	IPE_SUCCESS = 0,

	// internal/reserved:
	_IPE_RESERVED1 = -65536,
	_IPE_NOT_IMPLEMENTED_INTERNAL, // do not fucking use!

	// you are free to use these. You'll even have to in some places!
	IPE_NOT_IMPLEMENTED = -65001,
	IPE_NO_FILE,
	IPE_BUF_TOO_SMALL,
	IPE_TOO_LARGE,
	IPE_ALREADY,
	IPE_NOT_FOUND,
	IPE_NULLCALLBACK,
	IPE_STOP,

	_IPE_MAX
} infektPluginErrorId;


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
	const wchar_t *pluginDir;
};


struct infekt_nfo_info_t {
	size_t _uSize;

	const wchar_t* fileName;
	const wchar_t* filePath;
};


struct infekt_nfo_link_t {
	size_t _uSize;

	int linkId;
	const wchar_t* href;
	size_t row;
	size_t colStart;
	size_t colEnd;
};


#endif /* !_INFEKT_PLUGIN_H */
