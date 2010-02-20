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

	LPTSTR m_cursor;
	size_t m_selStartRow, m_selStartCol;
	size_t m_selEndRow, m_selEndCol;
	bool m_leftMouseDown, m_movedDownMouse;

	HMENU m_contextMenuHandle;
	HWND m_contextMenuCommandTarget;

	void UpdateScrollbars();
	void HandleScrollEvent(int a_dir, int a_event, int a_change);
	void GetScrollPositions(int& ar_x, int& ar_y);
	void CalcFromMouseCoords(int a_x, int a_y, size_t& ar_row, size_t& ar_col);

	void OnPaint();
	void OnMouseMove(int a_x, int a_y);
	void OnSetCursor();
	void OnMouseClickEvent(UINT a_event, int a_x, int a_y);

	static LRESULT CALLBACK _WindowProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT WindowProc(UINT, WPARAM, LPARAM);
public:
	CNFOViewControl(HINSTANCE a_hInstance, HWND a_parent, bool a_classic = false);
	virtual ~CNFOViewControl();

	virtual bool AssignNFO(const PNFOData& a_nfo);
	bool CreateControl(int a_left, int a_top, int a_width, int a_height);
	void SetContextMenu(HMENU a_menuHandle, HWND a_target);
	HWND GetHwnd() const { return m_hwnd; }

	const std::wstring GetSelectedText() const;
	void CopySelectedTextToClipboard() const;
	void SelectAll();

	virtual void InjectSettings(const CNFORenderSettings& ns);
};


typedef boost::shared_ptr<CNFOViewControl> PNFOViewControl;


#ifndef WM_MOUSEHWHEEL
// Windows Vista & higher only...
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef SPI_GETWHEELSCROLLCHARS
#define SPI_GETWHEELSCROLLCHARS 0x006C
#endif


#endif /* !_NFO_VIEWCTRL_H */
