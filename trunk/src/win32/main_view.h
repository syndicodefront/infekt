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

#ifndef _MAIN_VIEW_H
#define _MAIN_VIEW_H

#include "nfo_view_ctrl.h"

class CViewContainer : public CWnd
{
public:
	CViewContainer();
	virtual ~CViewContainer();

	bool ForwardFocusTypeMouseKeyboardEvent(const MSG* pMsg);
protected:
	PNFOViewControl m_renderControl;

	virtual void OnCreate();
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnAfterResize();
};

#endif /* !_MAIN_VIEW_H */
