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
#include <gtkmm/aboutdialog.h>

#include "nfo_view_ctrl.h"

class CMainWindow : public Gtk::Window
{
public:
	CMainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~CMainWindow();

	void SwitchView(ENfoViewMode a_newMode);
protected:
	Glib::RefPtr<Gtk::Builder> m_refGlade;

	CGtkNfoViewCtrl* m_pViewCtrl;
	Gtk::AboutDialog* m_pAboutDlg;

	/* worker methods */
	bool OpenFile(const std::string a_filePath);
	void SwitchViewInternal(bool a_fromUI, bool a_fromMenu, ENfoViewMode a_newMode);

	/* menu and toolbar event handlers */
	void on_file_open();
	void on_file_quit();
	void on_view_menu_change(int);
	void on_view_toolbar_change(int);
	void on_help_about();

	/* menu handler helpers */
	void on_help_about_response(int);
};

#endif /* _MAIN_WINDOW_H */

