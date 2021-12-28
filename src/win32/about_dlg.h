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

#ifndef _ABOUT_DLG_H
#define _ABOUT_DLG_H

#include "main_frame.h"

class CAboutDialog : public CDialog
{
public:
	CAboutDialog(HWND hWndParent = nullptr);
	virtual ~CAboutDialog();

	void SetMainWin(CMainFrame* a_mainWin) { m_mainWin = a_mainWin; }
	CMainFrame* GetMainWin() const { return m_mainWin; }

protected:
	HICON m_icon;
	HFONT m_boldFont;
	HWND m_linkCtrl;
	CMainFrame* m_mainWin;

	virtual BOOL OnInitDialog();
	virtual BOOL DialogProc(UINT, WPARAM, LPARAM);
	virtual LRESULT OnNotify(WPARAM, LPARAM);
};

#endif  /* !_ABOUT_DLG_H */
