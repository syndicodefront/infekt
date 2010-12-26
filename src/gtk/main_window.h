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

#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <gtkmm/builder.h>
#include <gtkmm/window.h>

#include "nfo_view_ctrl.h"

class CMainWindow : public Gtk::Window
{
public:
	CMainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~CMainWindow();

protected:
	Glib::RefPtr<Gtk::Builder> m_refGlade;

	CGtkNfoViewCtrl* m_pView;

	void on_file_quit();
	void on_help_about();
};

#endif /* _MAIN_WINDOW_H */

