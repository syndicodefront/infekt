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

#ifndef _INFOBAR_H
#define _INFOBAR_H

class CInfektInfoBar
{
protected:
	HINSTANCE m_instance;
	HWND m_parent;
	int m_left, m_top;
	int m_width, m_height;
	HWND m_hwnd;
	LPTSTR m_cursor;

	void OnPaint();

	static LRESULT CALLBACK _WindowProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT WindowProc(UINT, WPARAM, LPARAM);
public:
	CInfektInfoBar(HINSTANCE a_hInstance, HWND a_parent);
	virtual ~CInfektInfoBar();

	bool CreateControl(int a_left, int a_top, int a_width, int a_height);
	HWND GetHwnd() const { return m_hwnd; }
	void Show(bool a_show = true);
	bool ControlCreated() const { return (m_hwnd != 0); }
};

#endif /* !_INFOBAR_H */
