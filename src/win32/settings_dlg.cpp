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


enum _tab_page_ids {
	TAB_PAGE_GENERAL,
	TAB_PAGE_RENDERED,
	TAB_PAGE_NORMAL,
	TAB_PAGE_TEXTONLY
};



CSettingsWindowDialog::CSettingsWindowDialog(UINT nResID, HWND hWndParent) :
	CDialog(nResID, hWndParent)
{
}


BOOL CSettingsWindowDialog::OnInitDialog()
{
	m_tabControl.AttachDlgItem(IDC_SETTINGS_TAB, this);
	m_tabControl.AddTabPage(new CSettingsTabDialog(TAB_PAGE_GENERAL, IDD_TAB_GENERAL), _T("General"));
	m_tabControl.AddTabPage(new CSettingsTabDialog(TAB_PAGE_RENDERED, IDD_TAB_VIEWSETTINGS), _T("Rendered View"));
	m_tabControl.AddTabPage(new CSettingsTabDialog(TAB_PAGE_NORMAL, IDD_TAB_VIEWSETTINGS), _T("Standard View"));
	m_tabControl.AddTabPage(new CSettingsTabDialog(TAB_PAGE_TEXTONLY, IDD_TAB_VIEWSETTINGS), _T("Text-Only View"));

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



CSettingsTabDialog::CSettingsTabDialog(int a_pageId, UINT nResID) : CDialog(nResID)
{
	m_pageId = a_pageId;
}


BOOL CSettingsTabDialog::OnInitDialog()
{
	if(IsViewSettingPage())
	{
		::ShowWindow(GetDlgItem(IDC_SYNC_FROM_NORMAL), (m_pageId == TAB_PAGE_TEXTONLY));
		::ShowWindow(GetDlgItem(IDC_SYNC_FROM_RENDERED), (m_pageId != TAB_PAGE_RENDERED));

		::ShowWindow(GetDlgItem(IDC_ACTIVATE_GLOW), (m_pageId == TAB_PAGE_RENDERED));
		::ShowWindow(GetDlgItem(IDC_GLOW_LABEL1), (m_pageId == TAB_PAGE_RENDERED));
		::ShowWindow(GetDlgItem(IDC_GLOW_LABEL2), (m_pageId == TAB_PAGE_RENDERED));
		::ShowWindow(GetDlgItem(IDC_GLOW_RADIUS), (m_pageId == TAB_PAGE_RENDERED));
		::ShowWindow(GetDlgItem(IDC_CLR_GAUSS), (m_pageId == TAB_PAGE_RENDERED));

		::ShowWindow(GetDlgItem(IDC_LABEL_ART), (m_pageId != TAB_PAGE_TEXTONLY));
		::ShowWindow(GetDlgItem(IDC_CLR_ART), (m_pageId != TAB_PAGE_TEXTONLY));

		::ShowWindow(GetDlgItem(IDC_PRERELEASE), (m_pageId != TAB_PAGE_RENDERED));
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
			const LPDRAWITEMSTRUCT l_dis = (LPDRAWITEMSTRUCT)lParam;

			if(l_dis->itemAction == ODA_DRAWENTIRE)
			{
				cairo_surface_t* l_surface = cairo_win32_surface_create(l_dis->hDC);
				cairo_t* cr = cairo_create(l_surface);

				if(true/* target_color.alpha != 255*/)
				{
					// make a background for alpha colors...
					cairo_set_source_rgb(cr, 1, 1, 1);
					cairo_paint(cr);

					cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

					const int tileW = 5, tileH = 5;
					const int areaW = l_dis->rcItem.right, areaH = l_dis->rcItem.bottom;

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

				cairo_set_source_rgba(cr, 1, 0, 0, 0.5);
				cairo_rectangle(cr, 0, 0, l_dis->rcItem.right, l_dis->rcItem.bottom);
				cairo_fill(cr);

				cairo_destroy(cr);
				cairo_surface_destroy(l_surface);
			}

			return TRUE;
		}
	}

	return DialogProcDefault(uMsg, wParam, lParam);
}


BOOL CSettingsTabDialog::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	switch(LOWORD(wParam))
	{
	case IDC_CLR_GAUSS:
		Sleep(1);
		break;
	}
	return FALSE;
}


CSettingsTabDialog::~CSettingsTabDialog() 
{
}

