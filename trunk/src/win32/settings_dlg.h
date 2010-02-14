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

#ifndef _SETTINGS_DLG_H
#define _SETTINGS_DLG_H

#include "main_frame.h"


class CSettingsWindowDialog : public CDialog
{
public:
	CSettingsWindowDialog(UINT nResID, HWND hWndParent = NULL);
	virtual ~CSettingsWindowDialog();

protected:
	CNonThemedTab m_tabControl;

	virtual BOOL OnInitDialog();
	virtual BOOL DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};


class CSettingsTabDialog : public CDialog
{
public:
	CSettingsTabDialog(UINT nResID, HWND hWnd = NULL);
	virtual ~CSettingsTabDialog();
	virtual BOOL OnInitDialog();
	virtual BOOL DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};


#endif  /* !_SETTINGS_DLG_H */
