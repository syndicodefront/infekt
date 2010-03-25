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
#include "plugin_manager.h"
#include "app.h"

using namespace std;


long CPluginManager::PluginToCoreCallback(const char* szGuid, long lReserved, long lCall, long long lParam, void* pParam, void* pUser)
{
	switch(lCall)
	{
	case IPCI_GET_LOADED_NFO_TEXTW:
	case IPCI_GET_LOADED_NFO_TEXTUTF8:
		return DoGetLoadedNfoText(lParam, pParam, (lCall != IPCI_GET_LOADED_NFO_TEXTW));
	}

	return IPE_NOT_IMPLEMENTED;
}


long CPluginManager::DoGetLoadedNfoText(long long a_bufLen, void* a_buf, bool a_utf8)
{
	CNFOApp* l_app = dynamic_cast<CNFOApp*>(GetApp());
	CViewContainer* l_view = dynamic_cast<CViewContainer*>(l_app->GetMainFrame().GetView());
	PNFOData l_nfoData = l_view->GetNfoData();

	if(!l_nfoData || !l_nfoData->HasData())
	{
		return IPE_NO_FILE;
	}

	size_t l_bufSize = (a_utf8 ?
		l_nfoData->GetTextUtf8().size() + 1 :
	l_nfoData->GetTextWide().size() + 1);

	if(l_bufSize > (size_t)std::numeric_limits<long>::max())
	{
		return IPE_TOO_LARGE;
	}

	if(!a_buf || !a_bufLen)
	{
		// return required buffer size
		// (UTF-8: in bytes, otherwise: in characters)

		return static_cast<long>(l_bufSize);
	}
	else
	{
		// copy shit to buffer

		if(a_bufLen < l_bufSize)
		{
			return IPE_BUF_TOO_SMALL;
		}

		if(a_utf8)
		{
			strncpy_s(static_cast<char*>(a_buf), static_cast<size_t>(a_bufLen),
				l_nfoData->GetTextUtf8().c_str(), l_nfoData->GetTextUtf8().size());
		}
		else
		{
			wcsncpy_s(static_cast<wchar_t*>(a_buf), static_cast<size_t>(a_bufLen),
				l_nfoData->GetTextWide().c_str(), l_nfoData->GetTextWide().size());
		}

		return IPE_SUCCESS;
	}
}
