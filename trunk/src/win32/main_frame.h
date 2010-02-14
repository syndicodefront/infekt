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

class CMainFrame : public CFrame
{
public:
	CMainFrame();
	virtual ~CMainFrame();

protected:
	CViewContainer m_view;
	bool m_menuBarVisible;

	// Win32++ stuff start //
	virtual void PreCreate(CREATESTRUCT& cs);
	virtual void OnCreate();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnInitialUpdate();
	//LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	virtual void SetupToolbar();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// Win32++ stuff end //

	void AddToolbarButtons();
	void UpdateCaption();
	void OpenChooseFileName();

	static const int ms_minWidth = 300, ms_minHeight = 150;
};

#endif  /* !_MAIN_FRAME_H */
