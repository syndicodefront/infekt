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

#ifndef _APP_H
#define _APP_H

#include "settings_backend.h"
#include "main_frame.h"
#include "infekt_global.h"

class CNFOApp : public CWinApp
{
public:
	CNFOApp(); 
	virtual BOOL InitInstance();

	CMainFrame& GetMainFrame() { return m_frame; }
	void GetStartupOptions(std::wstring& ar_filePath, std::wstring& ar_viewMode, bool& ar_lineWrap, bool& ar_noGpu) const {
		ar_filePath = m_startupFilePath;
		ar_viewMode = m_startupViewMode;
		if(m_startupLineWrapOverride)
			ar_lineWrap = m_startupLineWrap;
		ar_noGpu = m_startupNoGpu;
	}
	PSettingsBackend GetSettingsBackend() { return m_settings; }
	bool InPortableMode() const { return m_portableMode; }

	bool ExtractConfigDirPath(std::wstring& ar_path) const;
	int ExtractStartupOptions(const std::wstring& a_commandLine);
	bool SwitchToPrevInstance();

	static CNFOApp* GetInstance() { return reinterpret_cast<CNFOApp*>(::Win32xx::GetApp()); }
	static CViewContainer* GetViewContainerInstance() { return reinterpret_cast<CViewContainer*>(GetInstance()->GetMainFrame().GetView()); }
protected:
	CMainFrame m_frame;
	std::wstring m_startupFilePath, m_startupViewMode;
	bool m_startupLineWrap, m_startupLineWrapOverride, m_startupNoGpu;
	PSettingsBackend m_settings;
	bool m_portableMode;
};

extern HINSTANCE g_hInstance;

#endif /* !_APP_H */
