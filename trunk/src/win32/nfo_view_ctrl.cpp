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

#ifndef NFOVWR_NO_CONTEXT_MENU
// makes this control unusable outside of this app, but we need the context menu IDs:
#include "resource.h"
#endif

#define NFOVWR_CTRL_CLASS_NAME _T("iNFEKT_NfoViewCtrl")


CNFOViewControl::CNFOViewControl(HINSTANCE a_hInstance, HWND a_parent, bool a_classic) : CNFORenderer(a_classic)
{
	m_instance = a_hInstance;
	m_parent = a_parent;
	m_left = m_top = m_width = m_height = 0;
	m_hwnd = 0;
	m_cursor = IDC_ARROW;
	m_centerNfo = true;
	m_copyOnSelect = false;
	m_findPosGlobalCol = m_findPosGlobalRow = 0;

	m_contextMenuHandle = NULL;
	m_contextMenuCommandTarget = NULL;
	m_linkUnderMenu = NULL;

	ClearSelection(false);
	m_leftMouseDown = m_movedDownMouse = false;
}


bool CNFOViewControl::CreateControl(int a_left, int a_top, int a_width, int a_height)
{
	if(ControlCreated())
	{
		return false;
	}

	WNDCLASSEX l_class = {0};

	if(::GetClassInfoEx(m_instance, NFOVWR_CTRL_CLASS_NAME, &l_class) == 0)
	{
		l_class.cbSize = sizeof(WNDCLASSEX);

		l_class.hInstance = m_instance;
		l_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		l_class.lpszClassName = NFOVWR_CTRL_CLASS_NAME;
		l_class.lpfnWndProc = &_WindowProc;
		l_class.hCursor = ::LoadCursor(NULL, IDC_ARROW);

		if(::RegisterClassEx(&l_class) == 0)
		{
			return false;
		}
	}

	m_top = a_top;
	m_left = a_left;
	m_width = a_width;
	m_height = a_height;

	m_hwnd = ::CreateWindowEx(::IsThemeActive() ? 0 : WS_EX_CLIENTEDGE,
		NFOVWR_CTRL_CLASS_NAME, NULL,
		WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE,
		m_left, m_top, m_width, m_height,
		m_parent, NULL,
		m_instance, this);

	if(!m_hwnd)
	{
		return false;
	}

	UpdateScrollbars(true);

	return true;
}


void CNFOViewControl::UpdateScrollbars(bool a_resetPos)
{
	// init struct:
	SCROLLINFO l_si = {0};
	l_si.cbSize = sizeof(SCROLLINFO);

	// set common flags:
	l_si.fMask = SIF_RANGE | SIF_PAGE | (a_resetPos ? SIF_POS : 0);

	if(HasNfoData())
	{
		// update horizontal scrollbar info:
		l_si.nPage = static_cast<UINT>(m_width / GetBlockWidth());
		l_si.nMax = static_cast<int>(GetWidth() / GetBlockWidth());
		::SetScrollInfo(m_hwnd, SB_HORZ, &l_si, FALSE);

		// update vertical scrollbar info:
		l_si.nPage = static_cast<UINT>(m_height / GetBlockHeight());
		l_si.nMax = static_cast<int>(GetHeight() / GetBlockHeight());
		::SetScrollInfo(m_hwnd, SB_VERT, &l_si, FALSE);
	}
	else
	{
		// set "null" info.
		::SetScrollInfo(m_hwnd, SB_HORZ, &l_si, FALSE);
		::SetScrollInfo(m_hwnd, SB_VERT, &l_si, FALSE);
	}
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
		if(LOWORD(wParam) & MK_CONTROL)
		{
			if(GET_WHEEL_DELTA_WPARAM(wParam) < 0)
				ZoomOut();
			else
				ZoomIn();
		}
		else
		{
			UINT l_lines = 0;
			if(::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &l_lines, 0))
			{
				int l_delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				HandleScrollEvent(SB_VERT, INT_MIN, -l_delta * l_lines);
			}
		}
		return 0;
	case WM_MOUSEHWHEEL: // Windows Vista & higher only...
		{
			UINT l_chars = 0;
			if(::SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &l_chars, 0))
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
		UpdateScrollbars(false);
		return 0;
#ifndef NFOVWR_NO_CONTEXT_MENU
	case WM_MOUSEMOVE:
		OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_SETCURSOR:
		return CUtilWin32GUI::GenericOnSetCursor(m_cursor, lParam);
#endif
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		OnMouseClickEvent(uMsg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
#ifndef NFOVWR_NO_CONTEXT_MENU
	case WM_CONTEXTMENU: {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		::ScreenToClient(m_hwnd, &pt);
		OnMouseClickEvent(uMsg, pt.x, pt.y);
		return 0; }
	case WM_COMMAND:
		if(HIWORD(wParam) == 0) // is it a menu?
		{
			if(LOWORD(wParam) == IDMC_COPYSHORTCUT)
			{
				if(m_linkUnderMenu)
				{
					CUtilWin32GUI::TextToClipboard(m_hwnd, m_linkUnderMenu->GetHref());
					m_linkUnderMenu = NULL;
				}
			}
			else
			{
				PostMessage(m_contextMenuCommandTarget, WM_COMMAND, wParam, lParam);
			}
		}
		return 0;
#endif
	case WM_THEMECHANGED:
		if(::IsThemeActive())
			SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
		else
			SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
		return 0;
	default:
		return ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
}


void CNFOViewControl::SetZoom(unsigned int a_percent)
{
	::SetCursor(::LoadCursor(NULL, m_cursor = IDC_WAIT));

	CNFORenderer::SetZoom(a_percent);

	if(IsClassicMode())
	{
		CalcClassicModeBlockSizes(true);
	}

	UpdateScrollbars(false);
	::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
}

void CNFOViewControl::ZoomIn()
{
	unsigned int l_oldZoom = GetZoom();

	if(l_oldZoom < 10000)
	{
		SetZoom(l_oldZoom + 10);
	}
}

void CNFOViewControl::ZoomOut()
{
	unsigned int l_oldZoom = GetZoom();

	if(l_oldZoom > 20)
	{
		SetZoom(l_oldZoom - 10);
	}
}

void CNFOViewControl::ZoomReset()
{
	SetZoom(100);
}


void CNFOViewControl::OnPaint()
{
	// paint/dc helper business:
	HDC l_dc;
	PAINTSTRUCT l_ps;
	cairo_surface_t *l_surface, *l_realSurface = NULL;
	bool l_textSelected = (m_selStartRow != (size_t)-1 && m_selEndRow != (size_t)-1);
	bool l_smart = !l_textSelected;

	// get scroll offsets:
	int l_x, l_y;
	GetScrollPositions(l_x, l_y);

	if(l_smart)
	{
		// smart = buffer invalidated rect only

		l_dc = ::BeginPaint(m_hwnd, &l_ps);
		l_realSurface = cairo_win32_surface_create(l_dc);

		l_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
			l_ps.rcPaint.right - l_ps.rcPaint.left,
			l_ps.rcPaint.bottom - l_ps.rcPaint.top);
	}
	else
	{
		// not smart = redraw (copy) entire control contents

		l_dc = ::GetDC(m_hwnd);
		l_realSurface = cairo_win32_surface_create(l_dc);
		memset(&l_ps, 0, sizeof(PAINTSTRUCT));
		l_ps.fErase = 1;

		l_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, m_width, m_height);
	}

	int l_destx = 0;
	if(m_centerNfo && m_width > (int)GetWidth())
		l_destx = (m_width - (int)GetWidth()) / 2;

	// erase the background if necessary:
	if(l_ps.fErase)
	{
		cairo_t* l_cr = cairo_create(!HasNfoData() ? l_realSurface : l_surface);
		cairo_set_source_rgb(l_cr, S_COLOR_T_CAIRO(GetBackColor()));
		if(HasNfoData())
		{
			double dh = static_cast<double>(GetHeight()),
				dw = static_cast<double>(GetWidth());

			// white-out area surrounding NFO contents:
			cairo_move_to(l_cr, 0, 0);
			cairo_line_to(l_cr, l_destx, 0);
			cairo_line_to(l_cr, l_destx, dh);
			cairo_line_to(l_cr, l_destx + dw, dh);
			cairo_line_to(l_cr, l_destx + dw, 0);
			cairo_line_to(l_cr, m_width, 0);
			cairo_line_to(l_cr, m_width, m_height);
			cairo_line_to(l_cr, 0, m_height);
			cairo_close_path(l_cr);
			cairo_fill(l_cr);
		}
		else
		{
			cairo_paint(l_cr);
		}
		cairo_destroy(l_cr);
	}

	// draw draw fight the power!
	if(l_smart)
	{
		DrawToSurface(l_surface, 0, 0,
			l_x * (int)GetBlockWidth() + l_ps.rcPaint.left - l_destx,
			l_y * (int)GetBlockHeight() + l_ps.rcPaint.top,
			cairo_image_surface_get_width(l_surface),
			cairo_image_surface_get_height(l_surface));
	}
	else
	{
		DrawToSurface(l_surface, l_destx, 0,
			l_x * (int)GetBlockWidth(),
			l_y * (int)GetBlockHeight(),
			m_width, m_height);
	}

	// draw highlighted (selected) text:
	if(l_textSelected)
	{
		const S_COLOR_T l_back = GetBackColor().Invert();
		double l_bw = (double)GetBlockWidth(), l_bh = (double)GetBlockHeight();

		if(!IsClassicMode())
		{
			RenderText(GetTextColor().Invert(), &l_back, GetHyperLinkColor().Invert(),
				m_selStartRow, m_selStartCol, m_selEndRow, m_selEndCol,
				l_surface, l_destx + -l_x * l_bw, -l_y * l_bh);
		}
		else
		{
			RenderClassic(GetTextColor().Invert(), &l_back, GetHyperLinkColor().Invert(), true,
				m_selStartRow, m_selStartCol, m_selEndRow, m_selEndCol,
				l_surface, l_destx + -l_x * l_bw, -l_y * l_bh);
		}
	}

	// copy from buffer to screen:
	if(HasNfoData())
	{
		cairo_t *cr = cairo_create(l_realSurface);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

		if(l_smart)
		{
			cairo_set_source_surface(cr, l_surface,
				/*dest_x - source_x*/ l_ps.rcPaint.left - 0,
				/*dest_y - source_y*/ l_ps.rcPaint.top - 0);

			cairo_rectangle(cr, l_ps.rcPaint.left, l_ps.rcPaint.top,
				cairo_image_surface_get_width(l_surface),
				cairo_image_surface_get_height(l_surface));

			cairo_fill(cr);
		}
		else
		{
			cairo_set_source_surface(cr, l_surface, 0, 0);
			cairo_paint(cr);
		}

		cairo_destroy(cr);
	}

	cairo_surface_destroy(l_realSurface);
	cairo_surface_destroy(l_surface);

	if(l_smart)
	{
		::EndPaint(m_hwnd, &l_ps);
	}
	else
	{
		const RECT l_rect = { 0, 0, m_width, m_height };
		::ValidateRect(m_hwnd, &l_rect);
		::ReleaseDC(m_hwnd, l_dc);
	}

	if(m_cursor == IDC_WAIT)
	{
		::SetCursor(::LoadCursor(NULL, m_cursor = IDC_ARROW));
	}
}


bool CNFOViewControl::AssignNFO(const PNFOData& a_nfo)
{
	::SetCursor(::LoadCursor(NULL, m_cursor = IDC_WAIT));

	m_linkUnderMenu = NULL;

	ClearSelection(false);

	if(CNFORenderer::AssignNFO(a_nfo))
	{
		if(!GetOnDemandRendering())
		{
			Render();
		}
		else
		{
			Render(0, 1);
		}

		UpdateScrollbars(true);

		::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);

		if(GetOnDemandRendering())
		{
			PreRender();
		}

		return true;
	}

	return false;
}


void CNFOViewControl::OnMouseMove(int a_x, int a_y)
{
	ssize_t l_row, l_col;

	// area where moving the mouse while selecting text scrolls:
	int l_scrollPadding = GetPadding();
	const int l_scrollSpeedDiv = 2; // slow down selection scrolling a bit

	if(!HasNfoData())
	{
		m_cursor = IDC_ARROW;
		return;
	}

	CalcFromMouseCoords(a_x, a_y, l_row, l_col);

	if(!m_leftMouseDown && m_nfo->GetLink(l_row, l_col) != NULL)
	{
		m_cursor = IDC_HAND;
	}
	else if(IsTextChar(l_row, l_col))
	{
		m_cursor = IDC_IBEAM;
	}
	else if(m_cursor != IDC_WAIT)
	{
		m_cursor = IDC_ARROW;
	}

	if(m_leftMouseDown && !m_movedDownMouse)
	{
		// user starts selecting text
		if(l_row != m_selStartRow || l_col != m_selStartCol)
		{
			m_movedDownMouse = true;
			::SetCapture(m_hwnd);
		}
	}

	if(m_leftMouseDown && m_movedDownMouse)
	{
		int l_bw = static_cast<int>(GetBlockWidth()), l_bh = static_cast<int>(GetBlockHeight());

		if(a_y >= m_height - l_scrollPadding)
		{
			HandleScrollEvent(SB_VERT, INT_MIN, (a_y - m_height + l_scrollPadding) / l_bh / l_scrollSpeedDiv);
		}
		else if(a_y <= l_scrollPadding)
		{
			HandleScrollEvent(SB_VERT, INT_MIN, -(int)((l_scrollPadding - a_y) / l_bh / l_scrollSpeedDiv));
		}

		if(a_x >= m_width - l_scrollPadding)
		{
			HandleScrollEvent(SB_HORZ, INT_MIN, (a_x - m_width + l_scrollPadding) / l_bw / l_scrollSpeedDiv);
		}
		else if(a_x <= l_scrollPadding)
		{
			HandleScrollEvent(SB_HORZ, INT_MIN, -(int)((l_scrollPadding - a_x) / l_bw / l_scrollSpeedDiv));
		}

		size_t l_virtualRow = (l_row < 0 ? 0 : l_row),
			l_virtualCol = (l_col < 0 ? 0 : l_col);

		// user selects text, selection "endpoint" has changed
		if(m_selEndRow != l_virtualRow || m_selEndCol != l_virtualCol)
		{
			m_selEndRow = l_virtualRow;
			m_selEndCol = l_virtualCol;
			::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
		}
	}
}


void CNFOViewControl::OnMouseClickEvent(UINT a_event, int a_x, int a_y)
{
#ifndef NFOVWR_NO_CONTEXT_MENU
	ssize_t l_row, l_col;

	if(!HasNfoData())
	{
		return;
	}

	CalcFromMouseCoords(a_x, a_y, l_row, l_col);

	if(a_event == WM_LBUTTONDOWN)
	{
		if(m_selStartRow != (size_t)-1)
		{
			ClearSelection(true);
		}

		m_leftMouseDown = true;
		m_movedDownMouse = false;
		m_selStartRow = l_row;
		m_selStartCol = l_col;
	}
	else if(a_event == WM_LBUTTONUP)
	{
		if(m_leftMouseDown)
		{
			bool l_reset = true;

			if(m_movedDownMouse)
			{
				m_selEndRow = (l_row < 0 ? 0 : l_row);
				m_selEndCol = (l_col < 0 ? 0 : l_col);

				if(m_copyOnSelect)
				{
					CopySelectedTextToClipboard();
				}
				else
				{
					l_reset = false;
				}

				::ReleaseCapture();
			}

			if(l_reset)
			{
				ClearSelection(false);
			}

			m_leftMouseDown = false;
			::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
		}
		
		if(!m_movedDownMouse)
		{
			const CNFOHyperLink *l_link = m_nfo->GetLink(l_row, l_col);

			if(l_link && ::PathIsURL(l_link->GetHref().c_str()))
			{
				::ShellExecute(NULL, _T("open"), l_link->GetHref().c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}
	else if(a_event == WM_LBUTTONDBLCLK)
	{
		if(IsTextChar(l_row, l_col))
		{
			size_t lc = l_col;

			m_selStartRow = m_selEndRow = l_row;
			m_selStartCol = m_selEndCol = l_col;

			while(IsTextChar(l_row, lc - 1) && iswalnum(m_nfo->GetGridChar(l_row, lc - 1)))
			{
				m_selStartCol = --lc;
			}

			lc = l_col;
			while(IsTextChar(l_row, lc + 1) && iswalnum(m_nfo->GetGridChar(l_row, lc + 1)))
			{
				m_selEndCol = ++lc;
			}

			if(m_copyOnSelect)
			{
				CopySelectedTextToClipboard();
			}

			::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
		}
	}
	else if(a_event == WM_CONTEXTMENU)
	{
		POINT l_pt;
		HMENU l_popup;

		l_pt.x = a_x;
		l_pt.y = a_y;

		l_popup = ::GetSubMenu(m_contextMenuHandle, 0);

		m_linkUnderMenu = m_nfo->GetLink(l_row, l_col);

		::EnableMenuItem(l_popup, IDMC_COPY, (m_selStartRow == (size_t)-1 ? MF_GRAYED | MF_DISABLED : MF_ENABLED));
		::EnableMenuItem(l_popup, IDMC_COPYSHORTCUT, (!m_linkUnderMenu ? MF_GRAYED | MF_DISABLED : MF_ENABLED));

		if(::GetWindowLong(m_contextMenuCommandTarget, GWL_EXSTYLE) & WS_EX_TOPMOST)
		{
			::CheckMenuItem(l_popup, IDMC_ALWAYSONTOP, MF_CHECKED | MF_BYCOMMAND);
		}
		else
		{
			::CheckMenuItem(l_popup, IDMC_ALWAYSONTOP, MF_UNCHECKED | MF_BYCOMMAND);
		}

		LPTSTR l_oldCursor = m_cursor;
		::SetCursor(::LoadCursor(NULL, m_cursor = IDC_ARROW));

		::ClientToScreen(m_hwnd, &l_pt);

		::TrackPopupMenuEx(l_popup, TPM_LEFTALIGN | TPM_LEFTBUTTON,
			l_pt.x, l_pt.y, m_hwnd, NULL);

		::SetCursor(::LoadCursor(NULL, m_cursor = l_oldCursor));
	}
#endif
}


void CNFOViewControl::CalcFromMouseCoords(int a_x, int a_y, ssize_t& ar_row, ssize_t& ar_col)
{
	if(m_nfo)
	{
		int l_x, l_y;
		GetScrollPositions(l_x, l_y);

		int l_centerx = 0;
		if(m_centerNfo && m_width > (int)GetWidth())
			l_centerx = (m_width - (int)GetWidth()) / 2;

		// calc real positions:
		ar_row = l_y + ((a_y - GetPadding()) / (ssize_t)GetBlockHeight());
		ar_col = l_x + ((a_x - GetPadding() - l_centerx) / (ssize_t)GetBlockWidth());
	}
	else
	{
		ar_row = ar_col = (size_t)-1;
	}
}


void CNFOViewControl::GetScrollPositions(int& ar_x, int& ar_y)
{
	SCROLLINFO l_si = { sizeof(SCROLLINFO), 0 };
	l_si.fMask = SIF_POS;

	::GetScrollInfo(m_hwnd, SB_HORZ, &l_si);
	ar_x = l_si.nPos;

	::GetScrollInfo(m_hwnd, SB_VERT, &l_si);
	ar_y = l_si.nPos;
}


bool CNFOViewControl::HandleScrollEvent(int a_dir, int a_event, int a_change)
{
	int l_prevPos;

	SCROLLINFO l_si = {0};
	l_si.cbSize = sizeof(SCROLLINFO);
	l_si.fMask = SIF_ALL;

	// Save the position for comparison later on:
	::GetScrollInfo(m_hwnd, a_dir, &l_si);
	l_prevPos = l_si.nPos;

#if (SB_LINEUP != SB_LINELEFT) || (SB_LINEDOWN != SB_LINERIGHT) || (SB_PAGEDOWN != SB_PAGERIGHT) || (SB_PAGEUP != SB_PAGELEFT)
#error ZOMG!
#endif

	switch(a_event)
	{
	case INT_MIN:
		l_si.nPos += a_change;
		break;
	case INT_MAX:
		l_si.nPos = a_change;
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
	::SetScrollInfo(m_hwnd, a_dir, &l_si, TRUE);
	::GetScrollInfo(m_hwnd, a_dir, &l_si);

	// If the position has changed, scroll the window:
	if(l_si.nPos != l_prevPos)
	{
		::ScrollWindow(m_hwnd, (a_dir == SB_HORZ ? (int)GetBlockWidth() * (l_prevPos - l_si.nPos) : 0),
			(a_dir == SB_VERT ? (int)GetBlockHeight() * (l_prevPos - l_si.nPos) : 0),
			NULL, NULL);

		return true;
	}

	// scroll pos unchanged:
	return false;
}


// returns whether scrolling has triggered a repaint:
bool CNFOViewControl::ScrollIntoView(size_t a_row, size_t a_col)
{
	bool bH = HandleScrollEvent(SB_HORZ, INT_MAX, (int)a_col),
		bV = HandleScrollEvent(SB_VERT, INT_MAX, (int)a_row);

	return bH || bV;
}


void CNFOViewControl::Show(bool a_show)
{
	if(m_hwnd)
	{
		::ShowWindow(m_hwnd, (a_show ? SW_SHOW : SW_HIDE));
	}
}


LRESULT CALLBACK CNFOViewControl::_WindowProc(HWND hWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CNFOViewControl *l_ctrl = NULL;

	if(uMsg == WM_CREATE)
	{
		const CREATESTRUCT *lpc = reinterpret_cast<CREATESTRUCT*>(lParam);

		::SetWindowLongPtr(hWindow, GWLP_USERDATA, (INT_PTR)lpc->lpCreateParams);

		l_ctrl = reinterpret_cast<CNFOViewControl*>(lpc->lpCreateParams);
	}
	else
	{
		l_ctrl = reinterpret_cast<CNFOViewControl*>((INT_PTR)::GetWindowLongPtr(hWindow, GWLP_USERDATA));
	}

	if(l_ctrl)
	{
		return l_ctrl->WindowProc(uMsg, wParam, lParam);
	}

	return ::DefWindowProc(hWindow, uMsg, wParam, lParam);
}


const std::wstring CNFOViewControl::GetSelectedText() const
{
	if(m_selStartRow == (size_t)-1 || m_selEndRow == (size_t)-1)
		return L"";

	std::wstring l_text;
	size_t l_leftStart = std::numeric_limits<size_t>::max();
	int l_dryRun = 1;

	size_t l_rowStart = m_selStartRow, l_colStart = m_selStartCol,
		l_rowEnd = m_selEndRow, l_colEnd = m_selEndCol;
	_FixUpRowColStartEnd(l_rowStart, l_colStart, l_rowEnd, l_colEnd);

	do
	{
		for(size_t row = l_rowStart; row <= l_rowEnd; row++)
		{
			bool l_textStarted = false;

			for(size_t col = 0; col < m_gridData->GetCols(); col++)
			{
				if(row == l_rowStart && col < l_colStart)
					continue;
				else if(row == l_rowEnd && col > l_colEnd)
					break;

				if(!IsTextChar(row, col, true))
				{
					if(!l_dryRun && l_textStarted) l_text += L' ';
						continue;
				}

				if(l_dryRun)
				{
					if(col < l_leftStart) l_leftStart = col;
				}
				else
				{
					if(!l_textStarted)
					{
						for(size_t p = 0; p < col - l_leftStart; p++)
							l_text += L' ';
					}

					wchar_t x = m_nfo->GetGridChar(row, col);
					if(x)
						l_text += x;
					else
						break; // reached end of this line
				}

				l_textStarted = true;
			}

			if(!l_dryRun)
			{
				// do not erase newlines/linebreaks here:
				CUtil::StrTrimRight(l_text, L" ");

				if(row != l_rowEnd)
					l_text += L"\r\n"; // SetClipboardContent with CF_UNICODETEXT wants \r\n instead of \n...
			}
		}
	} while(l_dryRun--);

	CUtil::StrTrimLeft(l_text, L"\r\n");

	return l_text;
}


void CNFOViewControl::CopySelectedTextToClipboard() const
{
#ifdef _WIN32_UI
	const std::wstring l_wstr = GetSelectedText();

	CUtilWin32GUI::TextToClipboard(m_hwnd, l_wstr);
#endif
}


void CNFOViewControl::SelectAll()
{
	if(m_gridData)
	{
		m_selStartCol = m_selStartRow = 0;
		m_selEndRow = m_gridData->GetRows();
		m_selEndCol = m_gridData->GetCols();

		::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
	}
}


bool CNFOViewControl::FindTermDown(const std::wstring& a_term, size_t& a_startRow, size_t& a_startCol)
{
	bool l_wrapped = false;
	size_t l_termIndex = 0;
	size_t l_termStartRow = 0;
	size_t l_termStartCol = 0;
	bool l_first = true;

	for(size_t row = a_startRow; row < m_gridData->GetRows(); )
	{
		for(size_t col = (l_first ? a_startCol : 0); col < m_gridData->GetCols(); col++)
		{
			if(!IsTextChar(row, col, true))
			{
				l_termIndex = 0;
				continue;
			}

			wchar_t l_char = m_nfo->GetGridChar(row, col);
			
			if(_wcsnicmp(&l_char, &a_term[l_termIndex], 1) != 0)
			{
				l_termIndex = 0;
				continue;
			}

			// on track...
			l_termIndex++;

			if(l_termIndex == 1)
			{
				l_termStartRow = row;
				l_termStartCol = col;
			}

			if(l_termIndex == a_term.size())
			{
				// done!
				a_startRow = l_termStartRow;
				a_startCol = l_termStartCol;

				return true;
			}
		}

		if(!l_wrapped && row + 1 == m_gridData->GetRows())
		{
			row = 0;
			l_wrapped = true;
		}
		else
		{
			row++;
		}

		l_first = false;
	}

	return false;
}

bool CNFOViewControl::FindTermUp(const std::wstring& a_term, size_t& a_startRow, size_t& a_startCol)
{
	bool l_wrapped = false;
	size_t l_termIndex = a_term.size() - 1;
	bool l_first = true;
	size_t l_startRow = a_startRow;

	// "manually" wrap for corner case where search term is at col 0:
	if(a_startCol == 0)
	{
		l_wrapped = true;
		l_startRow = (a_startRow == 0 ? m_gridData->GetRows() - 1 : a_startRow - 1);
		l_first = false;
	}

	for(size_t row = l_startRow; row >= 0; )
	{
		for(size_t col = (l_first ? a_startCol : m_gridData->GetCols()); col >= 0; col--)
		{
			if(IsTextChar(row, col, true))
			{
				wchar_t l_char = m_nfo->GetGridChar(row, col);

				if(_wcsnicmp(&l_char, &a_term[l_termIndex], 1) == 0)
				{
					if(l_termIndex == 0)
					{
						// done!
						a_startRow = row;
						a_startCol = col;

						return true;
					}

					l_termIndex--;
				}
				else
				{
					l_termIndex = a_term.size() - 1;
				}
			}
			else
			{
				l_termIndex = a_term.size() - 1;
			}

			if(col == 0) break;
		}

		if(!l_wrapped && row == 0)
		{
			row = m_gridData->GetRows() - 1;
			l_wrapped = true;
		}
		else if(row > 0)
		{
			row--;
		}
		else
		{
			break;
		}

		l_first = false;
	}

	return false;
}


bool CNFOViewControl::FindAndSelectTerm(const std::wstring& a_term, bool a_up)
{
	if(!HasNfoData())
		return false;

	if(a_term.empty())
	{
		m_lastFindTerm = L"";
		ClearSelection(true);
		return false;
	}

	size_t l_startRow, l_startCol;

	if(m_lastFindTerm != a_term)
	{
		l_startRow = 0;
		l_startCol = 0;
	}
	else
	{
		l_startRow = m_findPosGlobalRow;
		l_startCol = m_findPosGlobalCol;

		if(!a_up)
		{
			l_startCol += a_term.size();
		}
		else if(l_startCol > 0)
		{
			// important for 1-char searches, but also recurring chars:
			l_startCol--;
		}
	}

	if((a_up && FindTermUp(a_term, l_startRow, l_startCol))
		|| (!a_up && FindTermDown(a_term, l_startRow, l_startCol)))
	{
		m_selStartRow = l_startRow;
		m_selStartCol = l_startCol;
		m_selEndRow = l_startRow;
		m_selEndCol = l_startCol + a_term.size() - 1;

		m_findPosGlobalRow = m_selStartRow;
		m_findPosGlobalCol = m_selStartCol;

		m_lastFindTerm = a_term;

		// ScrollIntoView also triggers a repaint, but only if the scroll pos has changed:
		if(!ScrollIntoView(l_startRow, l_startCol))
		{
			::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
		}

		return true;
	}
	else
	{
		m_lastFindTerm = L"";
		ClearSelection(true);

		return false;
	}
}


void CNFOViewControl::ClearSelection(bool a_redraw)
{
	m_selStartRow = m_selStartCol = m_selEndRow = m_selEndCol = (size_t)-1;

	if(a_redraw)
	{
		::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
	}
}


void CNFOViewControl::SetContextMenu(HMENU a_menuHandle, HWND a_target)
{
	m_contextMenuHandle = a_menuHandle;
	m_contextMenuCommandTarget = a_target;
}


void CNFOViewControl::SetParent(HWND a_new)
{
	if(m_parent != a_new)
	{
		m_parent = a_new;
		::SetParent(m_hwnd, m_parent);
	}
}


void CNFOViewControl::SetCenterNfo(bool nb)
{
	if(nb == m_centerNfo)
		return;

	m_centerNfo = nb;

	if(HasNfoData())
	{
		::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
	}
}


void CNFOViewControl::InjectSettings(const CNFORenderSettings& ns)
{
	CNFORenderer::InjectSettings(ns);

	if(!IsRendered())
	{
		::SetCursor(::LoadCursor(NULL, m_cursor = IDC_WAIT));

		if(IsClassicMode())
		{
			CalcClassicModeBlockSizes();
		}

		UpdateScrollbars(false);
		::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
	}
}


CNFOViewControl::~CNFOViewControl()
{
	::UnregisterClass(NFOVWR_CTRL_CLASS_NAME, m_instance);

	if(m_hwnd)
	{
		::DestroyWindow(m_hwnd);
		// MSDN: "If the specified window is a parent window, DestroyWindow
		// automatically destroys the associated child windows."
	}
}
