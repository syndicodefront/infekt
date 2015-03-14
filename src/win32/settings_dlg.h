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

#ifndef _SETTINGS_DLG_H
#define _SETTINGS_DLG_H

#include "main_frame.h"


class CFontListEntry
{
public:
	CFontListEntry(const ENUMLOGFONTEX* a_elf);

	bool IsFixedWidth() const { return ((m_logFont.lfPitchAndFamily & FIXED_PITCH) != 0); }
	const std::_tstring& GetFontName() const { return m_name; }
	const TCHAR* GetShortName() const { return m_logFont.lfFaceName; }
	int GetNiceSize();
	std::set<int>::const_iterator SizesBegin() const { return m_sizes.begin(); }
	std::set<int>::const_iterator SizesEnd() const { return m_sizes.end(); }
protected:
	std::_tstring m_name;
	LOGFONT m_logFont;
	std::set<int> m_sizes;

	static int CALLBACK FontSizesProc(const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM);
};

typedef shared_ptr<CFontListEntry> PFontListEntry;


class CSettingsTabDialog;

class CSettingsWindowDialog : public CDialog
{
public:
	CSettingsWindowDialog(HWND hWndParent = NULL);
	virtual ~CSettingsWindowDialog();

	void SetMainWin(CMainFrame* a_mainWin) { m_mainWin = a_mainWin; }
	CMainFrame* GetMainWin() const { return m_mainWin; }
	const std::vector<PFontListEntry>& GetFonts(bool a_getAll);
	void DoThemeExImport(bool a_import);
protected:
	CNonThemedTab m_tabControl;
	CMainFrame* m_mainWin;

	CSettingsTabDialog* m_tabPageGeneral;
	CSettingsTabDialog* m_tabPageRendered;
	CSettingsTabDialog* m_tabPageClassic;
	CSettingsTabDialog* m_tabPageTextOnly;
	CSettingsTabDialog* m_tabPagePlugins;

	std::vector<PFontListEntry> m_fonts;
	std::vector<PFontListEntry> m_allFonts;
	static int CALLBACK FontNamesProc(const ENUMLOGFONTEX*, const NEWTEXTMETRICEX*, DWORD, LPARAM);

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};


class CSettingsTabDialog : public CDialog
{
public:
	CSettingsTabDialog(CSettingsWindowDialog* a_dlg, int a_pageId, UINT nResID);
	virtual ~CSettingsTabDialog();

	bool SaveSettings();
	void OnCancelled();
	CNFORenderSettings* GetViewSettings() { return m_viewSettings; }

	void ViewSettingsToGui();
protected:
	int m_pageId;
	CMainFrame* m_mainWin;
	CSettingsWindowDialog* m_dlgWin;

	// vars for view settings tabs:
	CNFORenderSettings* m_viewSettings;
	std::vector<PFontListEntry> m_fonts;
	int m_selectedFontIndex;
	CNFORenderSettings* m_previewSettingsBackup;
	EMainView m_beforePreviewViewType;

	// vars for the plugins tab:
	CListView m_pluginListView;
	std::map<size_t, std::wstring> m_pluginFileNames;
	std::map<size_t, std::string> m_pluginGuids;

	// methods for view settings tabs:
	bool IsViewSettingPage() const;
	static bool IsColorButton(UINT_PTR a_id);
	void DrawColorButton(const LPDRAWITEMSTRUCT a_dis);
	S_COLOR_T* ColorFromControlId(UINT a_id);
	void AddFontListToComboBox(bool a_addCustom);
	void DrawFontComboItem(const LPDRAWITEMSTRUCT a_dis);
	void MeasureFontComboItems(LPMEASUREITEMSTRUCT a_mis);
	void UpdateFontSizesCombo(size_t a_selSize = 0);
	void ReadBlockSize();
	void DoPreview();

	// methods for the plugins tab:
	void PopulatePluginList();

	// Win32++ inherited stuff:
	virtual BOOL OnInitDialog();
	virtual BOOL DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

	static const int ms_fontComboPadding = 3;
};


class CAdvancedSettingsWindowDialog : public CDialog
{
public:
	CAdvancedSettingsWindowDialog(HWND hWndParent = NULL);

	void SetMainSettings(PMainSettings a_mainSettings) { m_settings = a_mainSettings; }
protected:
	PMainSettings m_settings;

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};


#endif  /* !_SETTINGS_DLG_H */
