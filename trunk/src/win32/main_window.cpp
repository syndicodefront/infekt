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
#include "app.h"


/************************************************************************/
/* MAIN WINDOW                                                          */
/************************************************************************/

CMainWindowDialog::CMainWindowDialog(UINT nResID, HWND hWndParent) :
	CDialog(nResID, hWndParent)
{
}

PNFOData n;
/*CNFORenderer r;*/
CNFOViewControl *c = NULL;

BOOL CMainWindowDialog::OnInitDialog()
{
	n = PNFOData(new CNFOData());
	n->LoadFromFile(L"C:\\temp\\utf8-2.nfo");

	/*if(r.AssignNFO(n))
	{
	}*/

	c = new CNFOViewControl(g_hInstance, GetHwnd());
	c->AssignNFO(n);
	c->CreateControl(0, 0, 500, 500);

	return TRUE;
}

static void on_paint (HDC hdc)
{
	//cairo_surface_t *surface = cairo_win32_surface_create (hdc);

	//r.DrawToSurface(surface, 10, 10, 0, 0, 700, 800);

/*	cairo_surface_t *xx =
		cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 700, 2500);
r.DrawToSurface(xx, 0, 0, 0, 0, 700, 2500);
	cairo_surface_write_to_png (xx, "C:\\temp\\hello.png");
	cairo_surface_destroy (xx);*/

	//cairo_surface_destroy (surface);
}


BOOL CMainWindowDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// respond to the user defined message posted to the dialog
	switch (uMsg)
	{
	/*case WM_CLOSE:
		ShowWindow(SW_HIDE);
		return TRUE;*/
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc;

			dc = BeginPaint(GetHwnd(), &ps);
			on_paint(dc);
			EndPaint(GetHwnd(), &ps);
			return TRUE;
		}
	}

	// Pass unhandled messages on to parent DialogProc
	return DialogProcDefault(uMsg, wParam, lParam);
}


BOOL CMainWindowDialog::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	// Respond to the various dialog buttons
	/*switch (LOWORD(wParam))
	{
	case IDC_BUTTON_START:
		OnStartServer();
		return TRUE;
	case IDC_BUTTON_SEND:
		OnSend();
		return TRUE;
	} */

	return FALSE;
}


CMainWindowDialog::~CMainWindowDialog()
{
}

