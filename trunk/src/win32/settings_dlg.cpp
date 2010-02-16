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
#include "resource.h"

/************************************************************************/
/* SOME HELPFUL MACROS AND STUFF                                        */
/************************************************************************/

enum _tab_page_ids {
	TAB_PAGE_GENERAL = 1,
	TAB_PAGE_RENDERED,
	TAB_PAGE_NORMAL,
	TAB_PAGE_TEXTONLY
};

#define SET_DLG_CHECKBOX(ID, BOOLV) \
	this->SendDlgItemMessage(ID, BM_SETCHECK, ((BOOLV) ? BST_CHECKED : BST_UNCHECKED), 0);
#define DLG_SHOW_CTRL_IF(CTRL, CONDITION) \
	::ShowWindow(this->GetDlgItem(CTRL), (CONDITION) ? TRUE : FALSE);


/************************************************************************/
/* CSettingsWindowDialog Implementation                                 */
/************************************************************************/

CSettingsWindowDialog::CSettingsWindowDialog(UINT nResID, HWND hWndParent) :
	CDialog(nResID, hWndParent)
{
	m_mainWin = NULL;
}


BOOL CSettingsWindowDialog::OnInitDialog()
{
	m_tabControl.AttachDlgItem(IDC_SETTINGS_TAB, this);
	m_tabControl.AddTabPage(new CSettingsTabDialog(this, TAB_PAGE_GENERAL, IDD_TAB_GENERAL), _T("General"));
	m_tabControl.AddTabPage(new CSettingsTabDialog(this, TAB_PAGE_RENDERED, IDD_TAB_VIEWSETTINGS), _T("Rendered View"));
	m_tabControl.AddTabPage(new CSettingsTabDialog(this, TAB_PAGE_NORMAL, IDD_TAB_VIEWSETTINGS), _T("Standard View"));
	m_tabControl.AddTabPage(new CSettingsTabDialog(this, TAB_PAGE_TEXTONLY, IDD_TAB_VIEWSETTINGS), _T("Text-Only View"));

	m_tabControl.SetItemSize(110, 20);
	m_tabControl.SelectPage(0);

	ShowWindow(SW_SHOW);

	return TRUE;
}


BOOL CSettingsWindowDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DialogProcDefault(uMsg, wParam, lParam);
}


BOOL CSettingsWindowDialog::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	/*
	switch (LOWORD(wParam))
	{
	case IDC_BUTTON_SEND:
		OnSend();
		return TRUE;
	} */

	return FALSE;
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

		DLG_SHOW_CTRL_IF(IDC_PRERELEASE, m_pageId != TAB_PAGE_RENDERED);

		CViewContainer* l_view = dynamic_cast<CViewContainer*>(m_mainWin->GetView());
		m_viewSettings = new CNFORenderSettings();

		switch(m_pageId)
		{
		case TAB_PAGE_RENDERED: *m_viewSettings = l_view->GetRenderCtrl()->GetSettings(); break;
		//case TAB_PAGE_NORMAL: *m_viewSettings = l_view-> break;
		//case TAB_PAGE_TEXTONLY: *m_viewSettings = l_view-> break;
		default:
			delete m_viewSettings; m_viewSettings = NULL;
		}

		if(m_viewSettings)
		{
			SET_DLG_CHECKBOX(IDC_HILIGHT_LINKS, m_viewSettings->bHilightHyperLinks);
			SET_DLG_CHECKBOX(IDC_UNDERL_LINKS, m_viewSettings->bUnderlineHyperLinks);
			SET_DLG_CHECKBOX(IDC_ACTIVATE_GLOW, m_viewSettings->bGaussShadow);

			SendDlgItemMessage(IDC_GLOW_RADIUS, TBM_SETRANGE, FALSE, MAKELONG(1, 100));
			SendDlgItemMessage(IDC_GLOW_RADIUS, TBM_SETPOS, TRUE, m_viewSettings->uGaussBlurRadius);
			SendMessage(WM_HSCROLL, 0, (LPARAM)GetDlgItem(IDC_GLOW_RADIUS));
		}
	}

	return TRUE;
}


bool CSettingsTabDialog::IsViewSettingPage() const
{
	return (m_pageId == TAB_PAGE_RENDERED || m_pageId == TAB_PAGE_NORMAL || m_pageId == TAB_PAGE_TEXTONLY);
}


bool CSettingsTabDialog::IsColorButton(UINT a_id)
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
	case WM_HSCROLL:
		if(m_pageId == TAB_PAGE_RENDERED && (HWND)lParam == GetDlgItem(IDC_GLOW_RADIUS))
		{
			int l_pos = SendDlgItemMessage(IDC_GLOW_RADIUS, TBM_GETPOS, 0, 0);
			std::_tstring l_posStr = FORMAT(_T("%d"), l_pos);
			SetDlgItemText(IDC_GLOW_RADIUS_LABEL, l_posStr.c_str());
		}
	}

	return DialogProcDefault(uMsg, wParam, lParam);
}


BOOL CSettingsTabDialog::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
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

	return FALSE;
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


CSettingsTabDialog::~CSettingsTabDialog() 
{
	delete m_viewSettings;
}

