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
#include "content_scraper.h"

static INFEKT_PLUGIN_METHOD(ImdbMainEventCallback);


/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/

CImdbPlugin::CImdbPlugin(const infekt_plugin_load_t* a_load)
{
	m_fPluginToCore = a_load->pluginToCore;

	PluginSend(IPCI_REGISTER_NFO_LOAD_EVENTS, 0, ImdbMainEventCallback, this);
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

INFEKT_PLUGIN_METHOD(ImdbMainEventCallback)
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


void CImdbPlugin::OnFoundImdbLink(const std::wstring& a_href)
{
}


INFEKT_PLUGIN_METHOD(EnumNfoLinksCallback)
{
	if(lCall == IPV_ENUM_ITEM)
	{
		infekt_nfo_link_t* l_link = reinterpret_cast<infekt_nfo_link_t*>(pParam);
		wchar_t* l_hrefLower = _wcsdup(l_link->href);

		wchar_t* l_p = l_hrefLower;
		while(*l_p) 
		{
			*l_p = towlower(*l_p);
			l_p++;
		}

		l_p = wcsstr(l_hrefLower, L"imdb.com");
		if(l_p != NULL && l_p < wcsstr(l_hrefLower, L"/tt"))
		{
			reinterpret_cast<CImdbPlugin*>(pUser)->OnFoundImdbLink(l_hrefLower);

			free(l_hrefLower);
			return IPE_STOP; // we only use the first imdb link.
		}

		free(l_hrefLower);
	}

	return IPE_SUCCESS;
}


void CImdbPlugin::OnNfoLoaded(const infekt_nfo_info_t*)
{
	// find the imdb link:
	PluginSend(IPCI_ENUM_LOADED_NFO_LINKS, 0, (void*)EnumNfoLinksCallback, this);

	/*CContentScraper x;
	x.LoadScraperFile(L"Y:\\dev\\iNFEKT\\imdb.scrape.xml");
	x.DoScrape("");*/
}


CImdbPlugin::~CImdbPlugin()
{
}
