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

#ifndef _APP_H
#define _APP_H

#include "resource.h"
#include "main_window.h"
#include "nfo_data.h"
#include "nfo_renderer.h"

class CDialogApp : public CWinApp
{
public:
	CDialogApp(); 
	virtual ~CDialogApp();
	virtual BOOL InitInstance();
protected:
	CMainWindowDialog m_mainWindow;
};

extern HINSTANCE g_hInstance;

#endif /* !_APP_H */
