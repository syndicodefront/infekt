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
#include "settings_dlg.h"
#include "app.h"
#include "plugin_manager.h"
#include "resource.h"


/************************************************************************/
/* SOME HELPFUL MACROS AND STUFF                                        */
/************************************************************************/

enum _tab_page_ids {
	TAB_PAGE_GENERAL = 1,
	TAB_PAGE_RENDERED,
	TAB_PAGE_CLASSIC,
	TAB_PAGE_TEXTONLY,
	TAB_PAGE_PLUGINS
};

#define SET_DLG_CHECKBOX(ID, BOOLV) \
	this->SendDlgItemMessage(ID, BM_SETCHECK, ((BOOLV) ? BST_CHECKED : BST_UNCHECKED), 0);
#define DLG_SHOW_CTRL_IF(CTRL, CONDITION) \
	::ShowWindow(this->GetDlgItem(CTRL), (CONDITION) ? TRUE : FALSE);

#ifndef UNLEN
#define UNLEN 256
#endif

/************************************************************************/
/* CSettingsWindowDialog Implementation                                 */
/************************************************************************/

CSettingsWindowDialog::CSettingsWindowDialog(HWND hWndParent) :
	CDialog(IDD_DLG_SETTINGS, hWndParent)
{
	m_mainWin = NULL;

	m_tabPageGeneral = m_tabPageRendered = m_tabPageClassic = 
		m_tabPageTextOnly = m_tabPagePlugins = NULL;
}


BOOL CSettingsWindowDialog::OnInitDialog()
{
	SetIconLarge(IDI_APPICON);
	SetIconSmall(IDI_APPICON);

	m_tabControl.AttachDlgItem(IDC_SETTINGS_TAB, this);

	m_tabPageGeneral = new CSettingsTabDialog(this, TAB_PAGE_GENERAL, IDD_TAB_GENERAL);

	m_tabPageRendered = new CSettingsTabDialog(this, TAB_PAGE_RENDERED, IDD_TAB_VIEWSETTINGS);
	m_tabPageClassic = new CSettingsTabDialog(this, TAB_PAGE_CLASSIC, IDD_TAB_VIEWSETTINGS);
	m_tabPageTextOnly = new CSettingsTabDialog(this, TAB_PAGE_TEXTONLY, IDD_TAB_VIEWSETTINGS);
	m_tabPagePlugins = new CSettingsTabDialog(this, TAB_PAGE_PLUGINS, IDD_TAB_PLUGINS);

	m_tabControl.AddTabPage(m_tabPageGeneral, _T("General"));
	m_tabControl.AddTabPage(m_tabPageRendered, _T("Rendered View"));
	m_tabControl.AddTabPage(m_tabPageClassic, _T("Classic View"));
	m_tabControl.AddTabPage(m_tabPageTextOnly, _T("Text-Only View"));
	//m_tabControl.AddTabPage(m_tabPagePlugins, _T("Plugins"));

	m_tabControl.SetItemSize(100, 20);
	m_tabControl.SelectPage(0);

	ShowWindow(SW_SHOW);

	return TRUE;
}


void CSettingsWindowDialog::OnOK()
{
	bool b = true;

	b = b && m_tabPageGeneral->SaveSettings();

	b = b && m_tabPageRendered->SaveSettings();
	b = b && m_tabPageClassic->SaveSettings();
	b = b && m_tabPageTextOnly->SaveSettings();

	b = b && m_tabPagePlugins->SaveSettings();

	if(!b)
	{
		MessageBox(L"An error occurred while saving settings. Some settings may not have been saved.", L"Error", MB_ICONEXCLAMATION);
	}

	CViewContainer *l_view = dynamic_cast<CViewContainer*>(m_mainWin->GetView());

	l_view->GetRenderCtrl()->InjectSettings(*m_tabPageRendered->GetViewSettings());
	l_view->GetClassicCtrl()->InjectSettings(*m_tabPageClassic->GetViewSettings());
	l_view->GetTextOnlyCtrl()->InjectSettings(*m_tabPageTextOnly->GetViewSettings());

	// update or reset text-only view's word-wrap flag:
	l_view->SwitchView(l_view->GetViewType());
	// if that hasn't changed the call won't do anything.

	CPluginManager::GetInstance()->TriggerSettingsChanged();

	CDialog::OnOK();
}


void CSettingsWindowDialog::OnCancel()
{
	m_tabPageRendered->OnCancelled();
	m_tabPageClassic->OnCancelled();
	m_tabPageTextOnly->OnCancelled();

	CDialog::OnCancel();
}


typedef struct 
{
	std::vector<PFontListEntry>* ptr;
	bool fixed;
	HDC hdc;
	cairo_t* testCr;
} _temp_font_enum_data;

const std::vector<PFontListEntry>& CSettingsWindowDialog::GetFonts(bool a_getAll)
{
	std::vector<PFontListEntry>* l_pList = (a_getAll ? &m_allFonts : &m_fonts);

	if(l_pList->empty())
	{
		LOGFONT l_lf = {0};
		l_lf.lfCharSet = DEFAULT_CHARSET;

		_temp_font_enum_data l_data;
		l_data.ptr = l_pList;
		l_data.fixed = !a_getAll;
		l_data.hdc = ::GetDC(0);

#if 0
		cairo_surface_t* l_surface;
		l_surface = cairo_win32_surface_create(l_data.hdc);
		l_data.testCr = cairo_create(l_surface);
		cairo_set_font_size(l_data.testCr, 20);
#endif

		EnumFontFamiliesEx(l_data.hdc, &l_lf, (FONTENUMPROC)FontNamesProc, (LPARAM)&l_data, 0);

#if 0
		cairo_destroy(l_data.testCr);
		cairo_surface_destroy(l_surface);
#endif

		::ReleaseDC(0, l_data.hdc);
	}

	return *l_pList;
}

#if 0
static bool IsFixedWidthFont(const _temp_font_enum_data* a_data, const std::string& a_fontNameUtf8)
{
	cairo_select_font_face(a_data->testCr, a_fontNameUtf8.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

	cairo_text_extents_t l_e1, l_e2;
	cairo_text_extents(a_data->testCr, "i", &l_e1);
	cairo_text_extents(a_data->testCr, "W", &l_e2);

	return (l_e1.x_advance == l_e2.x_advance);
}
#endif

int CALLBACK CSettingsWindowDialog::FontNamesProc(const ENUMLOGFONTEX *lpelfe, const NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	const _temp_font_enum_data* l_data = (_temp_font_enum_data*)lParam;
	std::vector<PFontListEntry>* l_fonts = l_data->ptr;

	if(lpelfe->elfLogFont.lfCharSet == ANSI_CHARSET && lpelfe->elfFullName[0] != _T('@') &&
		(!l_data->fixed || (lpelfe->elfLogFont.lfPitchAndFamily & FIXED_PITCH) != 0))
	{
		const std::string l_fontNameUtf = CUtil::FromWideStr(lpelfe->elfFullName, CP_UTF8);

		// make sure Cairo can use this font:
		cairo_font_face_t* l_cff = cairo_toy_font_face_create(l_fontNameUtf.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

		if(l_cff)
		{
			l_fonts->push_back(PFontListEntry(new CFontListEntry(lpelfe)));

			cairo_font_face_destroy(l_cff);
		}
	}

	return 1;
}


CSettingsWindowDialog::~CSettingsWindowDialog()
{
}


/************************************************************************/
/* CSettingsTabDialog (Tab Page) Implementation                         */
/************************************************************************/

CSettingsTabDialog::CSettingsTabDialog(CSettingsWindowDialog* a_dlg, int a_pageId, UINT nResID) : CDialog(nResID)
{
	m_pageId = a_pageId;
	m_mainWin = a_dlg->GetMainWin();
	m_dlgWin = a_dlg;
	m_viewSettings = NULL;
	m_previewSettingsBackup = NULL;
	m_selectedFontIndex = 0;
	m_beforePreviewViewType = _MAIN_VIEW_MAX;

	m_fonts = m_dlgWin->GetFonts(false);
}


BOOL CSettingsTabDialog::OnInitDialog()
{
	if(IsViewSettingPage())
	{
		DLG_SHOW_CTRL_IF(IDC_SYNC_FROM_NORMAL, m_pageId == TAB_PAGE_TEXTONLY);
		DLG_SHOW_CTRL_IF(IDC_SYNC_FROM_RENDERED, m_pageId != TAB_PAGE_RENDERED);

		DLG_SHOW_CTRL_IF(IDC_ACTIVATE_GLOW, m_pageId == TAB_PAGE_RENDERED);
		DLG_SHOW_CTRL_IF(IDC_GLOW_LABEL1, m_pageId == TAB_PAGE_RENDERED);
		DLG_SHOW_CTRL_IF(IDC_GLOW_LABEL2, m_pageId == TAB_PAGE_RENDERED);
		DLG_SHOW_CTRL_IF(IDC_GLOW_RADIUS_LABEL, m_pageId == TAB_PAGE_RENDERED);
		DLG_SHOW_CTRL_IF(IDC_GLOW_RADIUS, m_pageId == TAB_PAGE_RENDERED);
		DLG_SHOW_CTRL_IF(IDC_CLR_GAUSS, m_pageId == TAB_PAGE_RENDERED);

		DLG_SHOW_CTRL_IF(IDC_LABEL_ART, m_pageId != TAB_PAGE_TEXTONLY);
		DLG_SHOW_CTRL_IF(IDC_CLR_ART, m_pageId != TAB_PAGE_TEXTONLY);
		DLG_SHOW_CTRL_IF(IDC_FONT_LINEWRAP, m_pageId == TAB_PAGE_TEXTONLY);

		DLG_SHOW_CTRL_IF(IDC_FONTSIZE_LABEL, m_pageId != TAB_PAGE_RENDERED);
		DLG_SHOW_CTRL_IF(IDC_FONTSIZE_COMBO, m_pageId != TAB_PAGE_RENDERED);

		if(m_pageId == TAB_PAGE_RENDERED)
		{
			SendDlgItemMessage(IDC_FONT_SIZE_SPIN, UDM_SETRANGE32, 3, 200);
			SendDlgItemMessage(IDC_FONT_SIZE_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(IDC_FONT_SIZE_EDIT), 0);

			SendDlgItemMessage(IDC_FONT_SIZE_SPIN2, UDM_SETRANGE32, 3, 200);
			SendDlgItemMessage(IDC_FONT_SIZE_SPIN2, UDM_SETBUDDY, (WPARAM)GetDlgItem(IDC_FONT_SIZE_EDIT2), 0);
		}
		else
		{
			DLG_SHOW_CTRL_IF(IDC_BLOCKSIZE_LABEL, false);
			DLG_SHOW_CTRL_IF(IDC_BLOCKSIZE_LABEL2, false);
			DLG_SHOW_CTRL_IF(IDC_FONT_SIZE_EDIT, false);
			DLG_SHOW_CTRL_IF(IDC_FONT_SIZE_EDIT2, false);
			DLG_SHOW_CTRL_IF(IDC_FONT_SIZE_SPIN, false);
			DLG_SHOW_CTRL_IF(IDC_FONT_SIZE_SPIN2, false);
		}

		CViewContainer* l_view = dynamic_cast<CViewContainer*>(m_mainWin->GetView());
		m_viewSettings = new CNFORenderSettings();

		switch(m_pageId)
		{
		case TAB_PAGE_RENDERED: *m_viewSettings = l_view->GetRenderCtrl()->GetSettings(); break;
		case TAB_PAGE_CLASSIC: *m_viewSettings = l_view->GetClassicCtrl()->GetSettings(); break;
		case TAB_PAGE_TEXTONLY: *m_viewSettings = l_view->GetTextOnlyCtrl()->GetSettings(); break;
		default:
			delete m_viewSettings; m_viewSettings = NULL;
		}

		ViewSettingsToGui();
	}
	else if(m_pageId == TAB_PAGE_GENERAL)
	{
		HWND l_hCb = GetDlgItem(IDC_COMBO_DEFAULTVIEW);

		ComboBox_AddString(l_hCb, _T("(Remember)"));
		ComboBox_AddString(l_hCb, _T("Rendered"));
		ComboBox_AddString(l_hCb, _T("Classic"));
		ComboBox_AddString(l_hCb, _T("Text Only"));

		const PMainSettings l_global = m_mainWin->GetSettings();

		ComboBox_SetCurSel(l_hCb, (l_global->iDefaultView == -1 ? 0 : l_global->iDefaultView));
		FixCommCtrls5ComboBug(l_hCb);

		SET_DLG_CHECKBOX(IDC_ALWAYSONTOP, l_global->bAlwaysOnTop);
		SET_DLG_CHECKBOX(IDC_MENUBAR_ON_STARTUP, l_global->bAlwaysShowMenubar);
		SET_DLG_CHECKBOX(IDC_COPY_ON_SELECT, l_global->bCopyOnSelect);
		SET_DLG_CHECKBOX(IDC_SINGLEINSTANCEMODE, l_global->bSingleInstanceMode);
		SET_DLG_CHECKBOX(IDC_REMEMBERMRU, l_global->bKeepOpenMRU);

		if(CUtil::IsWin6x())
		{
			int l_status = dynamic_cast<CNFOApp*>(GetApp())->IsDefaultNfoViewer();

			::EnableWindow(GetDlgItem(IDC_CHECK_DEFAULT_VIEWER), (l_status == -1 ? FALSE : TRUE));
		}

		SET_DLG_CHECKBOX(IDC_CHECK_DEFAULT_VIEWER, l_global->bCheckDefaultOnStartup && ::IsWindowEnabled(GetDlgItem(IDC_CHECK_DEFAULT_VIEWER)));

		if(CCudaUtil::GetInstance()->IsCudaUsable())
		{
			SetDlgItemText(IDC_CUDA_STATUS, _T("NVIDIA CUDA support on this system: Yes!"));
		}
		else
		{
			SetDlgItemText(IDC_CUDA_STATUS, _T("NVIDIA CUDA support on this system: No."));
		}

		const std::wstring l_info = L"Active settings backend: " +
			dynamic_cast<CNFOApp*>(GetApp())->GetSettingsBackend()->GetName();
		SetDlgItemText(IDC_SETTINGS_BACKEND_STATUS, l_info.c_str());
	}
	else if(m_pageId == TAB_PAGE_PLUGINS)
	{
		m_pluginListView.AttachDlgItem(IDC_PLUGIN_LIST, this);

		m_pluginListView.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 100);
		m_pluginListView.InsertColumn(1, _T("Version"), LVCFMT_LEFT, 60);
		m_pluginListView.InsertColumn(2, _T("Description"), LVCFMT_LEFT, 320);

		ListView_SetExtendedListViewStyleEx(m_pluginListView.GetHwnd(),
			LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		PopulatePluginList();
	}

	return TRUE;
}


void CSettingsTabDialog::ViewSettingsToGui()
{
	AddFontListToComboBox(true);

	if(m_viewSettings)
	{
		SET_DLG_CHECKBOX(IDC_HILIGHT_LINKS, m_viewSettings->bHilightHyperlinks);
		SET_DLG_CHECKBOX(IDC_UNDERL_LINKS, m_viewSettings->bUnderlineHyperlinks);
		SET_DLG_CHECKBOX(IDC_ACTIVATE_GLOW, m_viewSettings->bGaussShadow);
		SET_DLG_CHECKBOX(IDC_FONT_ANTIALIAS, m_viewSettings->bFontAntiAlias);
		SET_DLG_CHECKBOX(IDC_FONT_LINEWRAP, m_viewSettings->bWrapLines);

		SendDlgItemMessage(IDC_GLOW_RADIUS, TBM_SETRANGE, FALSE, MAKELONG(1, 100));
		SendDlgItemMessage(IDC_GLOW_RADIUS, TBM_SETPOS, TRUE, m_viewSettings->uGaussBlurRadius);
		SendMessage(WM_HSCROLL, 0, (LPARAM)GetDlgItem(IDC_GLOW_RADIUS));

		int l_idx = 0;
		for(std::vector<PFontListEntry>::const_iterator it = m_fonts.begin(); it != m_fonts.end(); it++, l_idx++)
		{
			if(_tcscmp((*it)->GetShortName(), m_viewSettings->sFontFace) == 0)
			{
				ComboBox_SetCurSel(GetDlgItem(IDC_FONTNAME_COMBO), l_idx);
				m_selectedFontIndex = l_idx;
				break;
			}
		}

		if(m_pageId == TAB_PAGE_RENDERED)
		{
			SendDlgItemMessage(IDC_FONT_SIZE_SPIN, UDM_SETPOS32, 0, m_viewSettings->uBlockWidth);
			SendDlgItemMessage(IDC_FONT_SIZE_SPIN2, UDM_SETPOS32, 0, m_viewSettings->uBlockHeight);
		}
		else
		{
			UpdateFontSizesCombo(m_viewSettings->uFontSize);
			FixCommCtrls5ComboBug(GetDlgItem(IDC_FONTSIZE_COMBO));
		}

		FixCommCtrls5ComboBug(GetDlgItem(IDC_FONTNAME_COMBO));
	}
}


void CSettingsTabDialog::AddFontListToComboBox(bool a_addCustom)
{
	int l_idx = 0;

	ComboBox_ResetContent(GetDlgItem(IDC_FONTNAME_COMBO));

	for(std::vector<PFontListEntry>::const_iterator it = m_fonts.begin(); it != m_fonts.end(); it++)
	{
		int l_id = ComboBox_AddString(GetDlgItem(IDC_FONTNAME_COMBO), (*it)->GetFontName().c_str());

		if(l_id != l_idx)
		{
			this->MessageBox(_T("There was an error populating the font list."), _T("Fail"), MB_ICONSTOP);
			break;
		}

		l_idx++;
	}

	if(a_addCustom)
	{
		ComboBox_AddString(GetDlgItem(IDC_FONTNAME_COMBO), _T("ZZZ(custom)"));
	}
}


bool CSettingsTabDialog::IsViewSettingPage() const
{
	return (m_pageId == TAB_PAGE_RENDERED || m_pageId == TAB_PAGE_CLASSIC || m_pageId == TAB_PAGE_TEXTONLY);
}


bool CSettingsTabDialog::IsColorButton(UINT_PTR a_id)
{
	switch(a_id)
	{
	case IDC_CLR_TEXT:
	case IDC_CLR_BACK:
	case IDC_CLR_ART:
	case IDC_CLR_LINKS:
	case IDC_CLR_GAUSS:
		return true;
	default:
		return false;
	}
}


void CSettingsTabDialog::ReadBlockSize()
{
	if(m_pageId == TAB_PAGE_RENDERED)
	{
		m_viewSettings->uBlockWidth = SendDlgItemMessage(IDC_FONT_SIZE_SPIN, UDM_GETPOS32, 0, 0);
		m_viewSettings->uBlockHeight = SendDlgItemMessage(IDC_FONT_SIZE_SPIN2, UDM_GETPOS32, 0, 0);
	}
}


BOOL CSettingsTabDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_DRAWITEM:
		if(IsViewSettingPage() && IsColorButton(wParam))
		{
			DrawColorButton((LPDRAWITEMSTRUCT)lParam);
			return TRUE;
		}
		else if(IsViewSettingPage() && wParam == IDC_FONTNAME_COMBO)
		{
			DrawFontComboItem((LPDRAWITEMSTRUCT)lParam);
			return TRUE;
		}
	case WM_MEASUREITEM:
		if(IsViewSettingPage() && wParam == IDC_FONTNAME_COMBO)
		{
			MeasureFontComboItems((LPMEASUREITEMSTRUCT)lParam);
			return TRUE;
		}
	case WM_HSCROLL:
		if(m_pageId == TAB_PAGE_RENDERED && (HWND)lParam == GetDlgItem(IDC_GLOW_RADIUS))
		{
			int l_pos = (int)SendDlgItemMessage(IDC_GLOW_RADIUS, TBM_GETPOS, 0, 0);
			std::_tstring l_posStr = FORMAT(_T("%d"), l_pos);
			SetDlgItemText(IDC_GLOW_RADIUS_LABEL, l_posStr.c_str());

			if(m_viewSettings)
			{
				m_viewSettings->uGaussBlurRadius = l_pos;
			}

			return TRUE;
		}
		break;
	}

	return this->DialogProcDefault(uMsg, wParam, lParam);
}


BOOL CSettingsTabDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(IsColorButton(LOWORD(wParam)))
	{
		if(S_COLOR_T* l_color = ColorFromControlId(LOWORD(wParam)))
		{
			static COLORREF l_customColors[16] = {0};

			CHOOSECOLOR l_cc = {0};
			l_cc.lStructSize = sizeof(CHOOSECOLOR);
			l_cc.hwndOwner = m_dlgWin->GetHwnd();
			l_cc.lpCustColors = l_customColors;
			l_cc.rgbResult = RGB(l_color->R, l_color->G, l_color->B);
			l_cc.Flags = CC_FULLOPEN | CC_ANYCOLOR | CC_RGBINIT;

			if(::ChooseColor(&l_cc))
			{
				*l_color = _S_COLOR(GetRValue(l_cc.rgbResult), GetGValue(l_cc.rgbResult),
					GetBValue(l_cc.rgbResult), l_color->A);
			}

			::RedrawWindow(GetDlgItem(LOWORD(wParam)), NULL, NULL, RDW_INVALIDATE);
		}

		return TRUE;
	}

	if(m_viewSettings)
	{
		switch(LOWORD(wParam))
		{
		case IDC_ACTIVATE_GLOW:
			m_viewSettings->bGaussShadow = (::IsDlgButtonChecked(m_hWnd, IDC_ACTIVATE_GLOW) != FALSE);
			break;
		case IDC_HILIGHT_LINKS:
			m_viewSettings->bHilightHyperlinks = (::IsDlgButtonChecked(m_hWnd, IDC_HILIGHT_LINKS) != FALSE);
			break;
		case IDC_UNDERL_LINKS:
			m_viewSettings->bUnderlineHyperlinks = (::IsDlgButtonChecked(m_hWnd, IDC_UNDERL_LINKS) != FALSE);
			break;
		case IDC_FONT_ANTIALIAS:
			m_viewSettings->bFontAntiAlias = (::IsDlgButtonChecked(m_hWnd, IDC_FONT_ANTIALIAS) != FALSE);
			break;
		case IDC_FONT_LINEWRAP:
			m_viewSettings->bWrapLines = (::IsDlgButtonChecked(m_hWnd, IDC_FONT_LINEWRAP) != FALSE);
			break;
		case IDC_FONTNAME_COMBO:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				int l_newIdx = ComboBox_GetCurSel((HWND)lParam);

				if(l_newIdx == m_fonts.size())
				{
					// "Custom..."
					ComboBox_SetCurSel((HWND)lParam, m_selectedFontIndex);
					// :TODO: just extend the drop down thing to *all* installed fonts here...
					// ... the common dialog is useless.

					this->MessageBox(_T("Not Implemented."), _T("Sorry"), MB_ICONEXCLAMATION);
				}
				else if((size_t)l_newIdx < m_fonts.size())
				{
					m_selectedFontIndex = l_newIdx;
					_tcsncpy_s(m_viewSettings->sFontFace, LF_FACESIZE + 1, m_fonts[l_newIdx]->GetShortName(), LF_FACESIZE);
					UpdateFontSizesCombo();
				}
			}
			break;
		case IDC_FONTSIZE_COMBO:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				int l_newIdx = ComboBox_GetCurSel((HWND)lParam);

				if((size_t)l_newIdx < m_fonts.size())
				{
					int l_size = (int)ComboBox_GetItemData((HWND)lParam, l_newIdx);
					m_viewSettings->uFontSize = l_size;
				}
			}
			break;
		case IDC_PREVIEW_BTN:
			DoPreview();
		default:
			return FALSE;
		}

		return TRUE;
	}
	else
	{
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_DEFAULT_VIEWER:
			dynamic_cast<CNFOApp*>(GetApp())->CheckDefaultNfoViewer(m_hWnd);
			break;
		case IDC_THEME_EXPORT:
			m_dlgWin->DoThemeExImport(false);
			break;
		case IDC_THEME_IMPORT:
			m_dlgWin->DoThemeExImport(true);
			break;
		case IDC_BUTTON_ADVANCED: {
			CAdvancedSettingsWindowDialog l_dlg(m_hWnd);
			l_dlg.SetMainSettings(m_mainWin->GetSettings());
			l_dlg.DoModal();
			break; }
		default:
			return FALSE;
		}

		return TRUE;
	}
}


LRESULT CSettingsTabDialog::OnNotify(WPARAM wParam, LPARAM lParam)
{
	if(m_pageId == TAB_PAGE_PLUGINS)
	{
		LPNMHDR l_shit = (LPNMHDR)lParam;

		if(l_shit->idFrom == IDC_PLUGIN_LIST && l_shit->code == LVN_ITEMCHANGING)
		{
			LPNMLISTVIEW l_pnmv = (LPNMLISTVIEW)lParam;

			if(l_pnmv->lParam == -1)
			{
				if((l_pnmv->uNewState & LVIS_FOCUSED) != 0 ||
					(l_pnmv->uNewState & LVIS_STATEIMAGEMASK) != 0)
				{
					SetWindowLongPtr(DWLP_MSGRESULT, TRUE); // cancel change
					return TRUE;
				}
			}
		}
	}

	return CDialog::OnNotify(wParam, lParam);
}


void CSettingsTabDialog::UpdateFontSizesCombo(size_t a_selSize)
{
	if(m_pageId != TAB_PAGE_RENDERED)
	{
		HWND l_hFontCombo = GetDlgItem(IDC_FONTSIZE_COMBO);

		ComboBox_ResetContent(l_hFontCombo);

		if(m_fonts.size() == 0)
		{
			// we're totally screwed but well, we shouldn't crash...
			return;
		}

		if(a_selSize == 0)
		{
			a_selSize = m_fonts[m_selectedFontIndex]->GetNiceSize();
		}

		int l_idx = 0;
		for(std::set<int>::const_iterator it = m_fonts[m_selectedFontIndex]->SizesBegin();
			it != m_fonts[m_selectedFontIndex]->SizesEnd(); it++, l_idx++)
		{
			TCHAR l_buf[10] = {0};
			_stprintf(l_buf, _T("%d"), *it);
			int l_item = ComboBox_AddString(l_hFontCombo, l_buf);

			if(l_item != l_idx)
				break;

			ComboBox_SetItemData(l_hFontCombo, l_idx, *it);

			if(*it == a_selSize) ComboBox_SetCurSel(l_hFontCombo, l_idx);
		}
	}
}


void CSettingsTabDialog::DoPreview()
{
	CViewContainer *l_view = dynamic_cast<CViewContainer*>(m_mainWin->GetView());

	if(!l_view->GetNfoData() || !l_view->GetNfoData()->HasData())
	{
		this->MessageBox(_T("Please open an NFO file first."), _T("Error"), MB_ICONEXCLAMATION);
		return;
	}

	PNFOViewControl l_ctrl;
	EMainView l_newViewType;

	switch(m_pageId)
	{
	case TAB_PAGE_RENDERED:
		l_ctrl = l_view->GetRenderCtrl();
		l_newViewType = MAIN_VIEW_RENDERED;
		break;
	case TAB_PAGE_CLASSIC:
		l_ctrl = l_view->GetClassicCtrl();
		l_newViewType = MAIN_VIEW_CLASSIC;
		break;
	case TAB_PAGE_TEXTONLY:
		l_ctrl = l_view->GetTextOnlyCtrl();
		l_newViewType = MAIN_VIEW_TEXTONLY;
		break;
	default:
		return;
	}

	if(!m_previewSettingsBackup)
	{
		m_previewSettingsBackup = new CNFORenderSettings();
		*m_previewSettingsBackup = l_ctrl->GetSettings();
	}

	if(m_beforePreviewViewType == _MAIN_VIEW_MAX)
	{
		m_beforePreviewViewType = l_view->GetViewType();
	}

	ReadBlockSize();

	::SetCursor(::LoadCursor(NULL, IDC_WAIT));

	l_ctrl->InjectSettings(*m_viewSettings);

	l_view->SwitchView(l_newViewType);

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}


void CSettingsWindowDialog::DoThemeExImport(bool a_import)
{
	if(a_import)
	{
		COMDLG_FILTERSPEC l_filter[] = { { L"Theme Info", L"*.ini" }, { L"All Files", L"*" } };

		std::_tstring l_filePath = CUtil::OpenFileDialog(g_hInstance,
			m_hWnd, _T("Theme Info (.ini)\0*.ini\0All Files\0*\0\0"), l_filter, 2);

		if(l_filePath.empty())
		{
			return;
		}

		std::wifstream l_file(l_filePath.c_str());

		if(!l_file.is_open() || !l_file.good())
		{
			MessageBox(_T("Unable to open file for reading."), _T("Error"), MB_ICONEXCLAMATION);
			return;
		}

		std::wstring l_dataBuf;
		wchar_t l_lineBuf[200] = {0};
		bool l_inData = false;
		std::wstring l_dataName;

		while(l_file.good())
		{
			l_file.getline(l_lineBuf, 199);
			std::wstring l_line = l_lineBuf;

			CUtil::StrTrim(l_line);

			if(l_line.empty() || l_line[0] == L'#')
			{
				continue;
			}

			if(!l_inData && l_line[0] == L'[' && l_line[l_line.size() - 1] == L']')
			{
				l_inData = true;
				l_dataName = l_line.substr(1, l_line.size() - 2);
				CUtil::StrTrim(l_dataName);
				l_dataBuf.clear();
			}
			else if(l_inData)
			{
				l_dataBuf += l_line;

				if(l_line[l_line.size() - 1] == L'}')
				{
					std::wstring l_dataNameFriendly;
					if(l_dataName == L"rendered") l_dataNameFriendly = L"Rendered";
					else if(l_dataName == L"classic") l_dataNameFriendly = L"Classic";
					else if(l_dataName == L"textonly") l_dataNameFriendly = L"Text Only";

					if(!l_dataNameFriendly.empty())
					{
						std::wstring l_prompt = L"Do you want to import the theme settings for the '" +
							l_dataNameFriendly + L"' view from this file?";

						if(MessageBox(l_prompt.c_str(), _T("Question"), MB_ICONQUESTION | MB_YESNO) == IDYES)
						{
							CSettingsTabDialog* l_pViewDlg = NULL;

							if(l_dataName == L"rendered") l_pViewDlg = m_tabPageRendered;
							else if(l_dataName == L"classic") l_pViewDlg = m_tabPageClassic;
							else if(l_dataName == L"textonly") l_pViewDlg = m_tabPageTextOnly;

							if(!l_pViewDlg || !l_pViewDlg->GetViewSettings()->UnSerialize(l_dataBuf, l_dataName != L"rendered"))
							{
								MessageBox(_T("Importing the theme failed for an unknown reason."), _T("Error"), MB_ICONSTOP);
							}
							else if(l_pViewDlg)
							{
								l_pViewDlg->ViewSettingsToGui();
							}
						}
					}

					l_inData = false;
				}
			}
			else
			{
				MessageBox(_T("Encountered an invalid line. Aborting."), _T("Error"), MB_ICONSTOP);
				break;
			}
		}
	}
	else
	{
		TCHAR l_userName[UNLEN + 1] = {0};
		DWORD l_dummy = UNLEN;
		::GetUserName(l_userName, &l_dummy);
		::CharLower(l_userName);

		COMDLG_FILTERSPEC l_filter[] = { { L"Theme Info", L"*.ini" } };

		std::_tstring l_filePath = CUtil::SaveFileDialog(g_hInstance,
			m_hWnd, _T("Theme Info (.ini)\0*.ini\0\0"), l_filter, 1, _T("ini"),
			std::_tstring(l_userName) + _T("-nfo-settings.ini"));

		if(l_filePath.empty())
		{
			return;
		}

		std::wfstream l_file(l_filePath.c_str(), std::ios::out | std::ios::trunc);
		std::string l_tmp;

		if(!l_file.is_open() || !l_file.good())
		{
			MessageBox(_T("Unable to open file for writing."), _T("Error"), MB_ICONEXCLAMATION);
			return;
		}

		l_file << "# NFO Theme Info File\n";
		l_file << "# Exported by iNFekt " << m_mainWin->InfektVersionAsString() << "\n";
		l_file << "# http://infekt.googlecode.com/\n\n";

		l_file << "[rendered]\n";
		l_tmp = CUtil::FromWideStr(m_tabPageRendered->GetViewSettings()->Serialize(), CP_UTF8);
		l_file << l_tmp.c_str();

		l_file << "\n\n[classic]\n";
		l_tmp = CUtil::FromWideStr(m_tabPageClassic->GetViewSettings()->Serialize(), CP_UTF8);
		l_file << l_tmp.c_str();

		l_file << "\n\n[textonly]\n";
		l_tmp = CUtil::FromWideStr(m_tabPageTextOnly->GetViewSettings()->Serialize(), CP_UTF8);
		l_file << l_tmp.c_str();
		l_file << "\n";

		MessageBox(_T("File saved!"), _T("Success"), MB_ICONINFORMATION);
	}
}


void CSettingsTabDialog::OnCancelled()
{
	if(m_beforePreviewViewType != _MAIN_VIEW_MAX)
	{
		CViewContainer *l_view = dynamic_cast<CViewContainer*>(m_mainWin->GetView());

		PNFOViewControl l_ctrl;

		switch(m_pageId)
		{
		case TAB_PAGE_RENDERED: l_ctrl = l_view->GetRenderCtrl(); break;
		case TAB_PAGE_CLASSIC: l_ctrl = l_view->GetClassicCtrl(); break;
		case TAB_PAGE_TEXTONLY: l_ctrl = l_view->GetTextOnlyCtrl(); break;
		default:
			return;
		}

		if(m_previewSettingsBackup)
		{
			l_ctrl->InjectSettings(*m_previewSettingsBackup);
		}

		m_mainWin->SwitchView(m_beforePreviewViewType);
	}
}


S_COLOR_T* CSettingsTabDialog::ColorFromControlId(UINT a_id)
{
	if(!m_viewSettings) return NULL;

	switch(a_id)
	{
	case IDC_CLR_TEXT:	return &m_viewSettings->cTextColor;
	case IDC_CLR_BACK:	return &m_viewSettings->cBackColor;
	case IDC_CLR_ART:	return &m_viewSettings->cArtColor;
	case IDC_CLR_LINKS:	return &m_viewSettings->cHyperlinkColor;
	case IDC_CLR_GAUSS:	return &m_viewSettings->cGaussColor;
	}

	return NULL;
}


void CSettingsTabDialog::DrawColorButton(const LPDRAWITEMSTRUCT a_dis)
{
	S_COLOR_T* l_color = ColorFromControlId(a_dis->CtlID);

	if(!l_color || a_dis->itemAction != ODA_DRAWENTIRE)
	{
		return;
	}

	cairo_surface_t* l_surface = cairo_win32_surface_create(a_dis->hDC);
	cairo_t* cr = cairo_create(l_surface);

	if(l_color->A != 255)
	{
		// make a chess board like background for alpha colors...
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_paint(cr);

		cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

		const int tileW = 5, tileH = 5;
		const int areaW = a_dis->rcItem.right, areaH = a_dis->rcItem.bottom;

		for(int x = 0; x <= areaW / tileW + 1; x++)
		{
			for(int y = 0; y < areaH / tileH + 1; y++)
			{
				if(x % 2 != y % 2)
				{
					cairo_rectangle(cr, x * tileW, y * tileH, tileW, tileH);
				}
			}
		}

		cairo_fill(cr);
	}

	cairo_set_source_rgba(cr, S_COLOR_T_CAIRO_A(*l_color));
	cairo_rectangle(cr, 0, 0, a_dis->rcItem.right, a_dis->rcItem.bottom);
	cairo_fill(cr);

	cairo_destroy(cr);
	cairo_surface_destroy(l_surface);
}


void CSettingsTabDialog::MeasureFontComboItems(LPMEASUREITEMSTRUCT a_mis)
{
	HWND l_hCombo = GetDlgItem(IDC_FONTNAME_COMBO);
	HDC l_hdc = ::GetDC(l_hCombo);
	cairo_surface_t* l_surface = cairo_win32_surface_create(l_hdc);
	cairo_t* cr = cairo_create(l_surface);

	double l_maxW = 0, l_maxH = 0;

	for(std::vector<PFontListEntry>::const_iterator it = m_fonts.begin(); it != m_fonts.end(); it++)
	{
		const PFontListEntry l_font = *it;
		const std::string l_fontNameUtf = CUtil::FromWideStr(l_font->GetFontName(), CP_UTF8);

		cairo_select_font_face(cr, l_fontNameUtf.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		if(cairo_status(cr) == CAIRO_STATUS_SUCCESS)
		{
			cairo_set_font_size(cr, l_font->GetNiceSize());

			cairo_text_extents_t l_extents = {0};
			cairo_text_extents(cr, l_fontNameUtf.c_str(), &l_extents);

			if(l_extents.width > l_maxW)
				l_maxW = l_extents.width;
			if(l_extents.height > l_maxH)
				l_maxH = l_extents.height;
		}
	}

	l_maxW = l_maxW + 2 * ms_fontComboPadding;
	l_maxH = l_maxH + 2 * ms_fontComboPadding;

	if(l_maxH > 25) l_maxH = 25; // :TODO: measure this instead of using a fixed 25.

	if(a_mis)
	{
		a_mis->itemWidth = (UINT)l_maxW;
		a_mis->itemHeight = (UINT)l_maxH;
	}

	cairo_destroy(cr);
	cairo_surface_destroy(l_surface);
	::ReleaseDC(l_hCombo, l_hdc);
}


void CSettingsTabDialog::DrawFontComboItem(const LPDRAWITEMSTRUCT a_dis)
{
	if(a_dis->itemID != (UINT)-1 && a_dis->itemID <= m_fonts.size())
	{
		if(a_dis->itemAction == ODA_DRAWENTIRE)
		{
			std::string l_fontNameUtf, l_displayName;
			int l_fontSize;

			if(a_dis->itemID < m_fonts.size())
			{
				const PFontListEntry l_font = m_fonts[a_dis->itemID];
				l_fontNameUtf = l_displayName = CUtil::FromWideStr(l_font->GetFontName(), CP_UTF8);
				l_fontSize = l_font->GetNiceSize();
			}
			else
			{
				// "Custom..." entry
				l_fontNameUtf = "MS Shell Dlg";
				l_displayName = "( Custom Font... )";
				l_fontSize = 14;
			}

			cairo_surface_t* l_surface = cairo_win32_surface_create(a_dis->hDC);
			cairo_t* cr = cairo_create(l_surface);

			cairo_select_font_face(cr, l_fontNameUtf.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
			cairo_set_font_size(cr, l_fontSize);

			cairo_text_extents_t l_extents = {0};
			cairo_text_extents(cr, l_fontNameUtf.c_str(), &l_extents);

			cairo_set_source_rgb(cr, 0, 0, 0);

			cairo_move_to(cr, a_dis->rcItem.left + ms_fontComboPadding - l_extents.x_bearing,
				a_dis->rcItem.top - l_extents.y_bearing + 
				((a_dis->rcItem.bottom - a_dis->rcItem.top) - l_extents.height) / 2);
			// the padding is included in (a_dis->rcItem.bottom - a_dis->rcItem.top)
			// which therefore defines the maximum item height.

			cairo_show_text(cr, l_displayName.c_str());

			cairo_destroy(cr);
			cairo_surface_destroy(l_surface);
		}
		else if(a_dis->itemAction == ODA_FOCUS)
		{
			::DrawFocusRect(a_dis->hDC, &a_dis->rcItem);
		}
	}
}


bool CSettingsTabDialog::SaveSettings()
{
	if(IsViewSettingPage() && m_viewSettings)
	{
		std::_tstring l_keyName;
		bool l_classic = true;

		switch(m_pageId)
		{
		case TAB_PAGE_RENDERED: l_keyName = _T("RenderedView"); l_classic = false; break;
		case TAB_PAGE_CLASSIC: l_keyName = _T("ClassicView"); break;
		case TAB_PAGE_TEXTONLY: l_keyName = _T("TextOnlyView"); break;
		}

		ReadBlockSize();

		if(m_beforePreviewViewType != _MAIN_VIEW_MAX)
		{
			m_mainWin->SwitchView(m_beforePreviewViewType);
		}

		return m_mainWin->SaveRenderSettingsToRegistry(l_keyName, *m_viewSettings, l_classic);
	}
	else if(m_pageId == TAB_PAGE_GENERAL)
	{
		PMainSettings l_set = m_mainWin->GetSettings();

		l_set->iDefaultView = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_DEFAULTVIEW));
		if(l_set->iDefaultView < 1 || l_set->iDefaultView >= _MAIN_VIEW_MAX)
			l_set->iDefaultView = -1;

		bool l_oldAot = l_set->bAlwaysOnTop;
		l_set->bAlwaysOnTop = (::IsDlgButtonChecked(GetHwnd(), IDC_ALWAYSONTOP) != FALSE);
		if(l_set->bAlwaysOnTop != l_oldAot) m_mainWin->UpdateAlwaysOnTop();

		bool l_oldAsm = l_set->bAlwaysShowMenubar;
		l_set->bAlwaysShowMenubar = (::IsDlgButtonChecked(GetHwnd(), IDC_MENUBAR_ON_STARTUP) != FALSE);
		if(l_set->bAlwaysShowMenubar != l_oldAsm) m_mainWin->ShowMenuBar(l_set->bAlwaysShowMenubar);

		l_set->bCopyOnSelect = (::IsDlgButtonChecked(GetHwnd(), IDC_COPY_ON_SELECT) != FALSE);
		dynamic_cast<CViewContainer*>(m_mainWin->GetView())->SetCopyOnSelect(l_set->bCopyOnSelect);

		l_set->bCheckDefaultOnStartup = (::IsDlgButtonChecked(GetHwnd(), IDC_CHECK_DEFAULT_VIEWER) != FALSE);
		l_set->bSingleInstanceMode = (::IsDlgButtonChecked(GetHwnd(), IDC_SINGLEINSTANCEMODE) != FALSE);
		l_set->bKeepOpenMRU = (::IsDlgButtonChecked(GetHwnd(), IDC_REMEMBERMRU) != FALSE);

		return l_set->SaveToRegistry();
	}
	else if(m_pageId == TAB_PAGE_PLUGINS)
	{
		CPluginManager* l_plugMgr = CPluginManager::GetInstance();
		std::_tstring l_pluginDirPath = CUtil::GetExeDir() + _T("\\plugins\\");

		for(std::map<size_t, std::string>::const_iterator it = m_pluginGuids.begin();
			it != m_pluginGuids.end(); it++)
		{
			LVFINDINFO l_findInfo = { LVFI_PARAM };
			l_findInfo.lParam = it->first;

			int l_index = m_pluginListView.FindItem(l_findInfo);

			if(l_index > -1)
			{
				bool l_new = (m_pluginListView.GetCheckState(l_index) != FALSE),
					l_old = l_plugMgr->IsPluginLoaded(it->second);

				if(l_new && !l_old)
				{
					if(!l_plugMgr->LoadPlugin(l_pluginDirPath + m_pluginFileNames[it->first]))
					{
						std::_tstring l_msg = _T("An error occured while trying to load a plugin (\"")
							+ m_pluginFileNames[it->first] + _T("\"):\r\n") +
							l_plugMgr->GetLastErrorMessage();

						this->MessageBox(l_msg.c_str(), _T("Error"), MB_ICONEXCLAMATION);
					}
				}
				else if(!l_new && l_old)
				{
					l_plugMgr->UnLoadPlugin(it->second);
				}
			}
		}

		return true;
	}

	return false;
}


void CSettingsTabDialog::FixCommCtrls5ComboBug(HWND a_combo)
{
	if(CUtil::IsWin2000())
	{
		// work around Common Controls 5 problem...
		RECT rc;
		::GetWindowRect(a_combo, &rc);
		POINT ptLT = { rc.left, rc.top }, ptRB = { rc.right, rc.bottom };
		::ScreenToClient(m_hWnd, &ptLT);
		::ScreenToClient(m_hWnd, &ptRB);
		::MoveWindow(a_combo, ptLT.x, ptLT.y, ptRB.x - ptLT.x,
			(ptRB.y - ptLT.y) * 5, TRUE);
		// The window height includes the drop-down list, so it needs to be raised or
		// no drop down list will show up. Pretty stupid.
	}
}


static int CALLBACK _PluginSortCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if(lParam1 == -1)
		return 1;
	else if(lParam2 == -1)
		return -1;

	std::map<size_t, std::wstring>* l_map = (std::map<size_t, std::wstring>*)lParamSort;

	return wcsicmp((*l_map)[lParam1].c_str(), (*l_map)[lParam2].c_str());
}

void CSettingsTabDialog::PopulatePluginList()
{
	m_pluginListView.DeleteAllItems();

	std::_tstring l_pluginDirPath = CUtil::GetExeDir() + _T("\\plugins\\"),
		l_pluginDirPattern = l_pluginDirPath + _T("*.dll");

	CPluginManager* l_plugMgr = CPluginManager::GetInstance();

	WIN32_FIND_DATA l_ffd = {0};
	HANDLE l_fh;

	if((l_fh = ::FindFirstFile(l_pluginDirPattern.c_str(), &l_ffd)) == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		std::_tstring l_dllPath = l_pluginDirPath + l_ffd.cFileName;
		std::_tstring l_pluginName = l_ffd.cFileName,
			l_pluginVersion, l_pluginDescr, l_saveFileName;
		std::string l_guid;

		if(l_plugMgr->LoadPlugin(l_dllPath, true))
		{
			l_plugMgr->GetLastProbedInfo(l_guid, l_pluginName, l_pluginVersion, l_pluginDescr);
			l_saveFileName = l_ffd.cFileName;
		}
		else
		{
			l_pluginVersion = _T("Error");
			l_pluginDescr = _T("Error: ") + CPluginManager::GetInstance()->GetLastErrorMessage();
		}

		LVITEM l_item = {0};
		l_item.iItem = m_pluginListView.GetItemCount();
		l_item.mask = LVIF_TEXT | LVIF_PARAM;
		l_item.lParam = (l_saveFileName.empty() ? -1 : l_item.iItem);
		l_item.pszText = (LPTSTR)l_pluginName.c_str();

		int l_idx = m_pluginListView.InsertItem(l_item);
		if(l_idx == l_item.iItem)
		{
			m_pluginListView.SetItemText(l_idx, 1, l_pluginVersion.c_str());
			m_pluginListView.SetItemText(l_idx, 2, l_pluginDescr.c_str());

			if(!l_saveFileName.empty())
			{
				m_pluginListView.SetCheckState(l_idx, l_plugMgr->IsPluginLoaded(l_guid));

				m_pluginFileNames[l_idx] = l_saveFileName;
				m_pluginGuids[l_idx] = l_guid;
			}
		}
	} while(::FindNextFile(l_fh, &l_ffd));

	m_pluginListView.SortItems(_PluginSortCallback, (DWORD_PTR)&m_pluginFileNames);

	for(WPARAM i = 0; i < 3; i++)
	{
		m_pluginListView.SendMessage(LVM_SETCOLUMNWIDTH, i, LVSCW_AUTOSIZE_USEHEADER);
	}

	::FindClose(l_fh);
}


CSettingsTabDialog::~CSettingsTabDialog() 
{
	delete m_viewSettings;
	delete m_previewSettingsBackup;
}


/************************************************************************/
/* CFontListEntry implementation                                        */
/************************************************************************/

CFontListEntry::CFontListEntry(const ENUMLOGFONTEX* a_elf)
{
	memmove_s(&m_logFont, sizeof(LOGFONT), &a_elf->elfLogFont, sizeof(a_elf->elfLogFont));
	m_name = a_elf->elfFullName;

	// enum font sizes:
	LOGFONT l_lf = {0};
	l_lf.lfCharSet = DEFAULT_CHARSET;
	_tcscpy_s(&l_lf.lfFaceName[0], LF_FACESIZE, m_logFont.lfFaceName);

	HDC l_hdc = GetDC(0);
	EnumFontFamiliesEx(l_hdc, &l_lf, FontSizesProc, (LPARAM)&m_sizes, 0);
	ReleaseDC(0, l_hdc);
}


int CALLBACK CFontListEntry::FontSizesProc(const LOGFONT* plf, const TEXTMETRIC* ptm, DWORD FontType, LPARAM lParam)
{
	std::set<int> *l_targetList = (std::set<int>*)lParam;

	if(FontType == TRUETYPE_FONTTYPE)
	{
		static int ls_ttSizes[] = { 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72, 0 };

		int* psz = ls_ttSizes;
		do
		{
			l_targetList->insert(*psz);
		}
		while(*++psz);

		return 0;
	}
	else
	{
		HDC l_hdc = GetDC(0);
		long l_size = MulDiv(ptm->tmHeight - ptm->tmInternalLeading, 72, GetDeviceCaps(l_hdc, LOGPIXELSY));
		ReleaseDC(0, l_hdc);

		l_targetList->insert(l_size);

		return 1;
	}
}


int CFontListEntry::GetNiceSize()
{
	if(m_sizes.empty())
		return 12;
	else if(m_sizes.size() == 1)
		return *m_sizes.begin();
	else
	{
		int l_size = *m_sizes.begin();

		for(std::set<int>::const_iterator it = m_sizes.begin(); it != m_sizes.end(); it++)
		{
			if(*it > 12) break;
			else l_size = *it;
		}

		return l_size;
	}
}


/************************************************************************/
/* 	CAdvancedSettingsWindowDialog implementation                        */
/************************************************************************/

CAdvancedSettingsWindowDialog::CAdvancedSettingsWindowDialog(HWND hWndParent) :
	CDialog(IDD_DLG_ADVANCED_SETTINGS, hWndParent)
{
}


BOOL CAdvancedSettingsWindowDialog::OnInitDialog()
{
	SET_DLG_CHECKBOX(IDC_CENTER_WINDOW, m_settings->bCenterWindow);
	SET_DLG_CHECKBOX(IDC_AUTO_WIDTH, m_settings->bAutoWidth);
	SET_DLG_CHECKBOX(IDC_CENTER_NFO, m_settings->bCenterNFO);
	SET_DLG_CHECKBOX(IDC_DESELECT_ON_COPY, m_settings->bDeSelectOnCopy);
	SET_DLG_CHECKBOX(IDC_EXPORT_NFO_DIR, m_settings->bDefaultExportToNFODir);

	ShowWindow(SW_SHOW);

	return TRUE;
}


void CAdvancedSettingsWindowDialog::OnOK()
{
	m_settings->bCenterWindow = (::IsDlgButtonChecked(GetHwnd(), IDC_CENTER_WINDOW) != FALSE);
	m_settings->bAutoWidth = (::IsDlgButtonChecked(GetHwnd(), IDC_AUTO_WIDTH) != FALSE);
	m_settings->bCenterNFO = (::IsDlgButtonChecked(GetHwnd(), IDC_CENTER_NFO) != FALSE);
	m_settings->bDeSelectOnCopy = (::IsDlgButtonChecked(GetHwnd(), IDC_DESELECT_ON_COPY) != FALSE);
	m_settings->bDefaultExportToNFODir = (::IsDlgButtonChecked(GetHwnd(), IDC_EXPORT_NFO_DIR) != FALSE);

	CDialog::OnOK();
}


void CAdvancedSettingsWindowDialog::OnCancel()
{
	CDialog::OnCancel();
}

