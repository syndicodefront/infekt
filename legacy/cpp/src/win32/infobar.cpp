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

#include "stdafx.h"
#include "infobar.h"
#include "util.h"

#define INFOBAR_CTRL_CLASS_NAME _T("iNFekt_InfoBarCtrl")

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x31A
#endif


CInfektInfoBar::CInfektInfoBar(HINSTANCE a_hInstance, HWND a_parent)
{
	m_instance = a_hInstance;
	m_parent = a_parent;
	m_left = m_top = m_width = m_height = 0;
	m_hwnd = 0;
	m_cursor = IDC_ARROW;
}


bool CInfektInfoBar::CreateControl(int a_left, int a_top, int a_width, int a_height)
{
	if (ControlCreated())
	{
		return false;
	}

	WNDCLASSEX l_class{};

	if (::GetClassInfoEx(m_instance, INFOBAR_CTRL_CLASS_NAME, &l_class) == 0)
	{
		l_class.cbSize = sizeof(WNDCLASSEX);

		l_class.hInstance = m_instance;
		l_class.style = CS_HREDRAW | CS_VREDRAW;
		l_class.lpszClassName = INFOBAR_CTRL_CLASS_NAME;
		l_class.lpfnWndProc = &_WindowProc;
		l_class.hCursor = ::LoadCursor(nullptr, IDC_ARROW);

		if (::RegisterClassEx(&l_class) == 0)
		{
			return false;
		}
	}

	m_top = a_top;
	m_left = a_left;
	m_width = a_width;
	m_height = a_height;

	m_hwnd = ::CreateWindowEx(::IsThemeActive() ? 0 : WS_EX_CLIENTEDGE,
		INFOBAR_CTRL_CLASS_NAME, nullptr,
		WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
		m_left, m_top, m_width, m_height,
		m_parent, nullptr, m_instance, nullptr);

	if (!m_hwnd)
	{
		return false;
	}

	::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (INT_PTR)this);

	//UpdateScrollbars(true);

	return true;
}


void CInfektInfoBar::Show(bool a_show)
{
	if (m_hwnd)
	{
		::ShowWindow(m_hwnd, (a_show ? SW_SHOW : SW_HIDE));
	}
}


void CInfektInfoBar::OnPaint()
{
	HDC l_dc;
	PAINTSTRUCT l_ps;
	cairo_surface_t *l_surface;

	l_dc = ::BeginPaint(m_hwnd, &l_ps);
	l_surface = cairo_win32_surface_create_with_format(l_dc, CAIRO_FORMAT_ARGB32);

	//if(l_ps.fErase)
	{
		cairo_t* l_cr = cairo_create(l_surface);
		cairo_set_source_rgb(l_cr, 1, 1, 1);
		cairo_paint(l_cr);
		cairo_destroy(l_cr);
	}


	cairo_surface_destroy(l_surface);
	::EndPaint(m_hwnd, &l_ps);
}


LRESULT CInfektInfoBar::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
		OnPaint();
		return 0;
	case WM_ERASEBKGND:
		return 0;
	case WM_SETCURSOR:
		return CUtilWin32GUI::GenericOnSetCursor(m_cursor, lParam);
	case WM_THEMECHANGED:
		if (::IsThemeActive())
			SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
		else
			SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
		return 0;
	default:
		return ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
}


LRESULT CALLBACK CInfektInfoBar::_WindowProc(HWND hWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CInfektInfoBar *l_ctrl = (CInfektInfoBar*)(void*)(INT_PTR)::GetWindowLongPtr(hWindow, GWLP_USERDATA);

	if (l_ctrl)
	{
		return l_ctrl->WindowProc(uMsg, wParam, lParam);
	}

	return ::DefWindowProc(hWindow, uMsg, wParam, lParam);
}


CInfektInfoBar::~CInfektInfoBar()
{
	::UnregisterClass(INFOBAR_CTRL_CLASS_NAME, m_instance);

	if (m_hwnd)
	{
		::DestroyWindow(m_hwnd);
	}
}
