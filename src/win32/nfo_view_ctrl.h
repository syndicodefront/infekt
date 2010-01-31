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

#ifndef _NFO_VIEWCTRL_H
#define _NFO_VIEWCTRL_H

#include "nfo_renderer.h"


class CNFOViewControl : public CNFORenderer
{
protected:
	HINSTANCE m_instance;
	HWND m_parent;
	int m_left, m_top;
	int m_width, m_height;
	HWND m_hwnd;
	HWND m_scrollH, m_scrollV;

	static LRESULT CALLBACK _WindowProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT WindowProc(UINT, WPARAM, LPARAM);
public:
	CNFOViewControl(HINSTANCE a_hInstance, HWND a_parent);
	virtual ~CNFOViewControl();

	bool CreateControl(int a_left, int a_top, int a_width, int a_height);
};


#endif /* !_NFO_VIEWCTRL_H */
