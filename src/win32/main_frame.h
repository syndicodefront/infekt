/**
 * Copyright (C) 2010-2014 cxxjoe
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
#include "main_settings.h"
#include "win_http_client.h"
#include "win_file_watcher.h"
#include "drop_target_helper.h"

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

	void OnAfterSettingsChanged();

	static const std::_tstring InfektVersionAsString();
protected:
	CViewContainer m_view;
	bool m_menuBarVisible;
	CToolbar* m_searchToolbar;
	HWND m_hSearchEditBox;
	std::wstring m_lastSearchTerm;
	bool m_showingAbout;
	PMainSettings m_settings;
	CMainDropTargetHelper* m_dropHelper;
	std::vector<std::wstring> m_mruPaths;

	std::vector<std::wstring> m_nfoPathsInFolder;
	// :TODO: add&check folder timestamp
	size_t m_nfoInFolderIndex;
	PNFOData m_nfoPreloadData;

	PWinFileWatcher m_fileChangeWatcher;
	void WatchFileStart();
	void WatchFileStop();
	void OnFileChanged();

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
	virtual void ShowToolbar(BOOL bShow);
	// Win32++ stuff end //

	void AddToolbarButtons();
	void CreateSearchToolbar();
	void ShowSearchToolbar(bool a_show = true);
	void DoFindText(bool a_up, bool a_force = true);
	void UpdateCaption();
	void OpenChooseFileName();
	void DoNfoExport(UINT a_id);
	bool DoOpenMruMenu(const LPNMTOOLBAR a_lpnm);
	bool DoCharsetMenu(const LPNMMOUSE a_pnmm);
	void UpdateStatusbar();
	void AdjustWindowToNFOWidth(bool a_preflightCheck, bool a_growOnly = false);

	void CheckForUpdates();
	void CheckForUpdates_Callback(PWinHttpRequest a_req);

	size_t BrowseFolderNfoGetNext(int a_direction);
	void BrowseFolderNfoMove(int a_direction);
	bool LoadFolderNfoList();

	static const int ms_minWidth = 300, ms_minHeight = 150;
	static const size_t ms_mruLength = 10;
};

#endif  /* !_MAIN_FRAME_H */
