/**
 * Copyright (C) 2014 syndicode
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
#include "srr++.h"


/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/

CRescenePlugin::CRescenePlugin(const infekt_plugin_load_t* a_load)
{
	m_fPluginToCore = a_load->pluginToCore;

	PluginSend(IPCI_REGISTER_FILE_FORMAT_SUPPORT_EVENTS, 0, CRescenePlugin::ResceneMainEventCallback, this);
}

/************************************************************************/
/* SOME EVENT CALLBACK BUSINESS                                         */
/************************************************************************/

long CRescenePlugin::PluginSend(infektPluginCallId lCall, long long lParam, void* pParam, void *pUser)
{
	if (m_fPluginToCore)
	{
		return m_fPluginToCore(MYGUID, 0, lCall, lParam, pParam, pUser);
	}

	return IPE_NOT_IMPLEMENTED;
}


INFEKT_PLUGIN_METHOD(CRescenePlugin::ResceneMainEventCallback)
{
	CRescenePlugin* pInstance = reinterpret_cast<CRescenePlugin*>(pUser);

	_ASSERTE(pUser != nullptr);

	switch (lCall)
	{
	case IPV_TRY_OPEN_FILE_FORMAT:
		return pInstance->TryLoadSrrToViewer(reinterpret_cast<infekt_file_format_open_info_t*>(pParam));
	}

	return IPE_NOT_IMPLEMENTED;
}


long CRescenePlugin::TryLoadSrrToViewer(const infekt_file_format_open_info_t* a_file)
{
	if (!a_file->filePath)
		return IPE_NOT_IMPLEMENTED;

	const wchar_t* l_extension = ::PathFindExtension(a_file->filePath);

	if (!l_extension || _wcsicmp(l_extension, L".srr"))
		return IPE_NOT_IMPLEMENTED;

	// extension matches, so let's try to handle this file!

	SRR::CContainer container;

	if (container.ReadFile(a_file->filePath))
	{
		// now find .nfo and call Show...
		std::vector<SRR::PStoredFile> l_nfos;

		if (container.FindStoredFiles(L".nfo", true, l_nfos))
		{
			infektDeclareStruct(infekt_show_nfo_t, nfo);

			nfo.filePath = a_file->filePath;
			nfo.fileName = l_nfos[0]->GetFileName().c_str();
			nfo.bufferLength = l_nfos[0]->GetDataLength();
			nfo.buffer = l_nfos[0]->GetDataPtr();
			nfo.req_charset = a_file->req_charset;

			return PluginSend(IPCI_SHOW_NFO, 0, &nfo);
		}
	}

	// Can't handle after all - but should have been able to. File might not be okay:
	return IPE_STOP;
}


CRescenePlugin::~CRescenePlugin()
{
}
