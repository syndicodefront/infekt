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

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR wszCommandLine, int nShowCmd)
{
	g_hInstance = hInstance;

	// Start Win32++
	CDialogApp theApp;

	// Run the application
	return theApp.Run();
}

CDialogApp::CDialogApp() : m_mainWindow(IDD_DLG_MAIN)
{
}

BOOL CDialogApp::InitInstance()
{
	m_mainWindow.DoModal();

	// End the program:
	::PostQuitMessage(0);

	return TRUE;
}

CDialogApp::~CDialogApp()
{
}


/* global vars */
HINSTANCE g_hInstance;
