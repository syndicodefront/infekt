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
#include "main_window.h"
#include <gtkmm/menuitem.h>


#define CONNECT_MENU(WIDGET, METHOD) do { \
	Gtk::MenuItem *l_mnu = NULL; \
	m_refGlade->get_widget(WIDGET, l_mnu); \
	if(l_mnu) l_mnu->signal_activate().connect(sigc::mem_fun(*this, &CMainWindow::METHOD)); \
	else fprintf(stderr, "Menu " WIDGET " not found!\n"); \
	} while(0);


CMainWindow::CMainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
	: Gtk::Window(cobject),
	m_refGlade(refGlade)
{
	m_refGlade->get_widget_derived("nfoDrawingLayout", m_pView);

	CONNECT_MENU("mnuHelpAbout", on_help_about);
	CONNECT_MENU("mnuFileQuit", on_file_quit);
}


void CMainWindow::on_help_about()
{
	printf("Hi!\n");
}


void CMainWindow::on_file_quit()
{
	hide();
}


CMainWindow::~CMainWindow()
{
}

