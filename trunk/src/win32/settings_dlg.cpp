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


CSettingsWindowDialog::CSettingsWindowDialog(UINT nResID, HWND hWndParent) :
	CDialog(nResID, hWndParent)
{
}


BOOL CSettingsWindowDialog::OnInitDialog()
{
	m_tabControl.AttachDlgItem(IDC_SETTINGS_TAB, this);
	m_tabControl.AddTabPage(new CSettingsTabDialog(IDD_TAB_GENERAL), _T("General"));
	m_tabControl.AddTabPage(new CSettingsTabDialog(IDD_TAB_RENDERED), _T("Rendered View"));

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



CSettingsTabDialog::CSettingsTabDialog(UINT nResID, HWND hWnd) : CDialog(nResID, hWnd) 
{
}


BOOL CSettingsTabDialog::OnInitDialog()
{
	return TRUE;
}


BOOL CSettingsTabDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DialogProcDefault(uMsg, wParam, lParam);
}


BOOL CSettingsTabDialog::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	return FALSE;
}


CSettingsTabDialog::~CSettingsTabDialog() 
{
}

