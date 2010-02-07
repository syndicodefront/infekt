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
#include "nfo_view_ctrl.h"

#define NFOVWR_CTRL_CLASS_NAME _T("NfoViewCtrl")


CNFOViewControl::CNFOViewControl(HINSTANCE a_hInstance, HWND a_parent) : CNFORenderer()
{
	m_instance = a_hInstance;
	m_parent = a_parent;
	m_left = m_top = m_width = m_height = 0;
	m_hwnd = 0;
	m_scrollH = m_scrollV = 0;
}


bool CNFOViewControl::CreateControl(int a_left, int a_top, int a_width, int a_height)
{
	WNDCLASSEX l_class = {0};
	l_class.cbSize = sizeof(WNDCLASSEX);

	l_class.hInstance = m_instance;
	l_class.style = CS_HREDRAW | CS_VREDRAW;
	l_class.lpszClassName = NFOVWR_CTRL_CLASS_NAME;
	l_class.lpfnWndProc = &_WindowProc;
	l_class.hCursor = ::LoadCursor(NULL, IDC_ARROW);

	if(RegisterClassEx(&l_class) == 0)
	{
		return false;
	}

	m_top = a_top;
	m_left = a_left;
	m_width = a_width;
	m_height = a_height;

	m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		NFOVWR_CTRL_CLASS_NAME, NULL,
		WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | WS_BORDER,
		m_left, m_top, m_width, m_height,
		m_parent, NULL,
		m_instance, NULL);

	// :TODO: allow window to receive focus.
	// :TODO: allow text selection...

	if(!m_hwnd)
	{
		return false;
	}

	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (INT_PTR)this);

	UpdateScrollbars();

	return true;
}


void CNFOViewControl::UpdateScrollbars()
{
	bool l_needsV, l_needsH;
	int l_vWidth, l_hHeight;

	// init struct:
	SCROLLINFO l_si = {0};
	l_si.cbSize = sizeof(SCROLLINFO);

	// get scroll bar dimensions:
	l_vWidth = GetSystemMetrics(SM_CXVSCROLL);
	l_hHeight = GetSystemMetrics(SM_CXHSCROLL);

	// determine which scrollbars we need:
	l_needsH = (m_width - l_vWidth < (int)GetWidth());
	l_needsV = (m_height - l_hHeight < (int)GetHeight());
	// ... we do this to add some extra space between
	// content and scrollbars for nicer looks.

	// set common flags:
	l_si.fMask = SIF_RANGE | SIF_PAGE;
	l_si.nMin = 0;

	// update horizontal scrollbar info:
	l_si.nPage = m_width / m_blockWidth;
	l_si.nMax = GetWidth() / m_blockWidth + (l_needsV ? 2 : 0);
	SetScrollInfo(m_hwnd, SB_HORZ, &l_si, FALSE);

	// update vertical scrollbar info:
	l_si.nPage = m_height / m_blockHeight;
	l_si.nMax = GetHeight() / m_blockHeight + (l_needsH ? 1 : 0);
	SetScrollInfo(m_hwnd, SB_VERT, &l_si, FALSE);
}


LRESULT CNFOViewControl::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		OnPaint();
		return 0;
	case WM_ERASEBKGND:
		return 0; // we erase the background in OnPaint, if necessary.
	case WM_VSCROLL:
		HandleScrollEvent(SB_VERT, LOWORD(wParam), 0);
		return 0;
	case WM_HSCROLL:
		HandleScrollEvent(SB_HORZ, LOWORD(wParam), 0);
		return 0;
	case WM_MOUSEWHEEL:
		{
			UINT l_lines = 0;
			if(SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &l_lines, 0))
			{
				int l_delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				HandleScrollEvent(SB_VERT, INT_MIN, -l_delta * l_lines);
			}
		}
		return 0;
	case WM_MOUSEHWHEEL: // Windows Vista & higher only...
		{
			UINT l_chars = 0;
			if(SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &l_chars, 0))
			{
				int l_delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				HandleScrollEvent(SB_HORZ, INT_MIN, -l_delta * l_chars);
			}
		}
		// Source: http://msdn.microsoft.com/en-us/library/ms997498.aspx#mshrdwre_topic2
		// The MSDN page for WM_MOUSEHWHEEL says "return zero" though... wait till someone complains.
		return TRUE;
	case WM_SIZE:
		m_width = LOWORD(lParam);
		m_height = HIWORD(lParam);
		UpdateScrollbars();
		return 0;

	default:
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
}


void CNFOViewControl::OnPaint()
{
	// paint/dc helper business:
	HDC l_dc;
	PAINTSTRUCT l_ps;
	cairo_surface_t *l_surface;

	// get scroll position business:
	SCROLLINFO l_si = {0};
	int l_x, l_y;

	l_si.cbSize = sizeof(SCROLLINFO);
	l_si.fMask = SIF_POS;

	GetScrollInfo(m_hwnd, SB_HORZ, &l_si);
	l_x = l_si.nPos;

	GetScrollInfo (m_hwnd, SB_VERT, &l_si);
	l_y = l_si.nPos;

	// paint!
	l_dc = BeginPaint(m_hwnd, &l_ps);
	l_surface = cairo_win32_surface_create(l_dc);

	// erase the background if necessary:
	if(l_ps.fErase)
	{
		cairo_t* l_cr = cairo_create(l_surface);
		cairo_set_source_rgb(l_cr, S_COLOR_T_CAIRO(m_backColor));
		cairo_paint(l_cr);
		cairo_destroy(l_cr);
	}

	// draw draw draw fight the powa!
	DrawToSurface(l_surface, 0, 0, l_x * m_blockWidth, l_y * m_blockHeight, m_width, m_height);

	cairo_surface_destroy(l_surface);
	EndPaint(m_hwnd, &l_ps);
}


void CNFOViewControl::HandleScrollEvent(int a_dir, int a_event, int a_change)
{
	int l_prevPos;

	SCROLLINFO l_si = {0};
	l_si.cbSize = sizeof(SCROLLINFO);
	l_si.fMask = SIF_ALL;

	// Save the position for comparison later on:
	GetScrollInfo(m_hwnd, a_dir, &l_si);
	l_prevPos = l_si.nPos;

#if (SB_LINEUP != SB_LINELEFT) || (SB_LINEDOWN != SB_LINERIGHT) || (SB_PAGEDOWN != SB_PAGERIGHT) || (SB_PAGEUP != SB_PAGELEFT)
#error ZOMG!
#endif

	switch(a_event)
	{
	case INT_MIN:
		l_si.nPos += a_change;
		break;
	case SB_TOP: // user hit the HOME keyboard key
		l_si.nPos = l_si.nMin;
		break;
	case SB_BOTTOM: // user hit the END keyboard key
		l_si.nPos = l_si.nMax;
		break;
	case SB_LINEUP: // user clicked the top/left arrow
		l_si.nPos -= 1;
		break;
	case SB_LINEDOWN: // user clicked the bottom/right arrow
		l_si.nPos += 1;
		break;
	case SB_PAGEUP: // user clicked the scroll bar shaft above/left of the scroll box
		l_si.nPos -= l_si.nPage;
		break;
	case SB_PAGEDOWN: // user clicked the scroll bar shaft below/right of the scroll box
		l_si.nPos += l_si.nPage;
		break;
	case SB_THUMBTRACK: // user dragged the scroll box
		l_si.nPos = l_si.nTrackPos;
		break;
	}

	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.
	l_si.fMask = SIF_POS;
	SetScrollInfo(m_hwnd, a_dir, &l_si, TRUE);
	GetScrollInfo(m_hwnd, a_dir, &l_si);

	// If the position has changed, scroll the window:
	if(l_si.nPos != l_prevPos)
	{
		ScrollWindow(m_hwnd, (a_dir == SB_HORZ ? m_blockWidth * (l_prevPos - l_si.nPos) : 0),
			(a_dir == SB_VERT ? m_blockHeight * (l_prevPos - l_si.nPos) : 0),
			NULL, NULL);
	}
}


LRESULT CALLBACK CNFOViewControl::_WindowProc(HWND hWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CNFOViewControl *l_ctrl = (CNFOViewControl*)(void*)(INT_PTR)GetWindowLongPtr(hWindow, GWLP_USERDATA);

	if(l_ctrl)
	{
		return l_ctrl->WindowProc(uMsg, wParam, lParam);
	}

	return DefWindowProc(hWindow, uMsg, wParam, lParam);
}


CNFOViewControl::~CNFOViewControl()
{
	UnregisterClass(NFOVWR_CTRL_CLASS_NAME, m_instance);

	if(m_hwnd)
	{
		DestroyWindow(m_hwnd);
		// MSDN: "If the specified window is a parent window, DestroyWindow
		// automatically destroys the associated child windows."
	}
}
