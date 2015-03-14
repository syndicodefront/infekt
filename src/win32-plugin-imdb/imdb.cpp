/**
 * Copyright (C) 2010 syndicode
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
#include "content_scraper.h"
#include "util.h"


/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/

CImdbPlugin::CImdbPlugin(const infekt_plugin_load_t* a_load)
{
	m_fPluginToCore = a_load->pluginToCore;

	PluginSend(IPCI_REGISTER_NFO_LOAD_EVENTS, 0, CImdbPlugin::ImdbMainEventCallback, this);
}


/************************************************************************/
/* SOME EVENT CALLBACK BUSINESS                                         */
/************************************************************************/

long CImdbPlugin::PluginSend(infektPluginCallId lCall, long long lParam, void* pParam, void *pUser)
{
	if(m_fPluginToCore)
	{
		return m_fPluginToCore(MYGUID, 0, lCall, lParam, pParam, pUser);
	}

	return IPE_NOT_IMPLEMENTED;
}


INFEKT_PLUGIN_METHOD(CImdbPlugin::ImdbMainEventCallback)
{
	CImdbPlugin* pInstance = reinterpret_cast<CImdbPlugin*>(pUser);

	_ASSERTE(pUser != NULL);

	switch(lCall)
	{
	case IPV_NFO_LOADED:
		pInstance->OnNfoLoaded(reinterpret_cast<infekt_nfo_info_t*>(pParam));
		return IPE_SUCCESS;
	}

	return IPE_NOT_IMPLEMENTED;
}


void CImdbPlugin::OnNfoLoaded(const infekt_nfo_info_t*)
{
	// find the imdb link:
	PluginSend(IPCI_ENUM_LOADED_NFO_LINKS, 0, (void*)EnumNfoLinksCallback, this);
}


INFEKT_PLUGIN_METHOD(CImdbPlugin::EnumNfoLinksCallback)
{
	if(lCall == IPV_ENUM_ITEM)
	{
		infekt_nfo_link_t* l_link = reinterpret_cast<infekt_nfo_link_t*>(pParam);

		const std::string l_utf8Url = CUtil::FromWideStr(l_link->href, CP_UTF8);
		pcrecpp::StringPiece l_linkUrlPiece(l_utf8Url);
		std::string l_imdbId;

		if(pcrecpp::RE("\\.imdb\\.\\w+.*/tt(\\d+)").PartialMatch(l_linkUrlPiece, &l_imdbId))
		{
			reinterpret_cast<CImdbPlugin*>(pUser)->OnFoundImdbLink(l_imdbId);

			return IPE_STOP; // we only use the first imdb link.
		}
	}

	return IPE_SUCCESS;
}


void CImdbPlugin::OnFoundImdbLink(const std::string& a_imdbId)
{
	const std::wstring l_imdbUrl = L"http://www.imdb.com/title/tt" +
		CUtil::ToWideStr(a_imdbId, CP_UTF8) + L"/";

	infektDeclareStruct(infekt_http_request_t, l_req);
	l_req.url = l_imdbUrl.c_str();
	l_req.callback = CImdbPlugin::HttpCallback;
	PluginSend(IPCI_HTTP_REQUEST, 0, &l_req, this);
}


INFEKT_PLUGIN_METHOD(CImdbPlugin::HttpCallback)
{
	if(lCall == IPV_HTTP_RESULT && pParam != NULL)
	{
		infekt_http_result_t* l_result = reinterpret_cast<infekt_http_result_t*>(pParam);

		reinterpret_cast<CImdbPlugin*>(pUser)->OnHttpResult(l_result);
	}

	return IPE_SUCCESS;
}


void CImdbPlugin::OnHttpResult(const infekt_http_result_t* a_result)
{
}


CImdbPlugin::~CImdbPlugin()
{
}
