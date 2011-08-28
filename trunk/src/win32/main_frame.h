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

#ifndef _MAIN_FRAME_H
#define _MAIN_FRAME_H

#include "main_view.h"
#include "win_http_client.h"


class CMainSettings
{
public:
	CMainSettings(bool bFromRegistry) {
		iDefaultView = -1;
		iLastView = MAIN_VIEW_RENDERED;
		bCopyOnSelect = bAlwaysOnTop = bAlwaysShowMenubar = false;
		bCheckDefaultOnStartup = false;
		bSingleInstanceMode = false;
		bKeepOpenMRU = true;
		bCenterWindow = true;
		bAutoWidth = false;
		bCenterNFO = true;
		bDefaultExportToNFODir = false;
		bCloseOnEsc = false;
		if(bFromRegistry) LoadFromRegistry();
	}
	bool LoadFromRegistry();
	bool SaveToRegistry();

	int32_t iDefaultView, iLastView;
	bool bCopyOnSelect;
	bool bAlwaysOnTop;
	bool bAlwaysShowMenubar;
	bool bCheckDefaultOnStartup;
	bool bSingleInstanceMode;
	bool bKeepOpenMRU;

	bool bCenterWindow;
	bool bAutoWidth;
	bool bCenterNFO;
	bool bDefaultExportToNFODir;
	bool bCloseOnEsc;
};

typedef boost::shared_ptr<CMainSettings> PMainSettings;


class CMainDropTargetHelper : public IDropTarget
{
public:
	CMainDropTargetHelper(HWND a_hwnd);
	virtual ~CMainDropTargetHelper();

	// IUnknown implementation
	HRESULT _stdcall QueryInterface(REFIID iid, void** ppvObject);
	ULONG _stdcall AddRef();
	ULONG _stdcall Release();

	// IDropTarget implementation
	HRESULT _stdcall DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	HRESULT _stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	HRESULT _stdcall DragLeave();
	HRESULT _stdcall Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
protected:
	LONG m_refCount;
	HWND m_hwnd;

	bool m_allowDrop;
};


class CMainFrame : public CFrame
{
public:
	CMainFrame();
	virtual ~CMainFrame();

	bool SaveRenderSettingsToRegistry(const std::_tstring& a_key,
		const CNFORenderSettings& a_settings, bool a_classic);
	bool LoadRenderSettingsFromRegistry(const std::_tstring& a_key, CNFORenderer* a_target);

	void SwitchView(EMainView a_view);
	bool OpenFile(const std::_tstring a_filePath);
	void UpdateAlwaysOnTop();
	void ShowMenuBar(bool a_show = true);
	PMainSettings GetSettings() { return m_settings; }

	static const std::_tstring InfektVersionAsString();
protected:
	CViewContainer m_view;
	bool m_menuBarVisible;
	bool m_showingAbout;
	PMainSettings m_settings;
	CMainDropTargetHelper *m_dropHelper;
	std::vector<std::_tstring> m_mruPaths;

	void LoadOpenMruList();
	void SaveOpenMruList();
	void SavePositionSettings();

	// Win32++ stuff start //
	virtual void PreCreate(CREATESTRUCT& cs);
	virtual void OnCreate();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	virtual void OnInitialUpdate();
	virtual void OnHelp();
	virtual void SetupToolbar();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual inline void SetStatusText() {}
	virtual void OnClose();
	virtual void ShowStatusbar(BOOL bShow);
	// Win32++ stuff end //

	void AddToolbarButtons();
	void UpdateCaption();
	void OpenChooseFileName();
	void DoNfoExport(UINT a_id);
	bool DoOpenMruMenu(const LPNMTOOLBAR a_lpnm);
	void UpdateStatusbar();
	void AdjustWindowToNFOWidth();

	void CheckForUpdates();
	void CheckForUpdates_Callback(PWinHttpRequest a_req);

	static const int ms_minWidth = 300, ms_minHeight = 150;
	static const size_t ms_mruLength = 10;
};

#endif  /* !_MAIN_FRAME_H */
