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
#include <gtkmm/toolbutton.h>
#include <gtkmm/stock.h>
#include <gtkmm/filechooserdialog.h>


/**
 * Helper macro that connects a void METHOD of CMainWindow
 * to a menu click event identified by the menu entry WIDGET.
 **/
#define CONNECT_MENU(WIDGET, METHOD) do { \
	Gtk::MenuItem *l_mnu = NULL; \
	m_refGlade->get_widget(WIDGET, l_mnu); \
	if(l_mnu) l_mnu->signal_activate().connect(sigc::mem_fun(*this, &CMainWindow::METHOD)); \
	else fprintf(stderr, "Menu " WIDGET " not found!\n"); \
	} while(0);

/**
 * Helper macro that connects the clicked event of a toolbar
 * button (identified by WIDGET) to the given METHOD.
 **/
#define CONNECT_TOOLBAR_BUTTON(WIDGET, METHOD) do { \
	Gtk::ToolButton *l_btn = NULL; \
	m_refGlade->get_widget(WIDGET, l_btn); \
	if(l_btn) l_btn->signal_clicked().connect(sigc::mem_fun(*this, &CMainWindow::METHOD)); \
	else fprintf(stderr, "Toolbar button " WIDGET " not found!\n"); \
	} while(0);


/**
 * Main window constructor.
 **/
CMainWindow::CMainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
	: Gtk::Window(cobject),
	m_refGlade(refGlade)
{
	m_pAboutDlg = NULL;

	m_refGlade->get_widget_derived("nfoDrawingLayout", m_pViewCtrl);

	if(!m_pViewCtrl)
	{
		abort();
	}

	CONNECT_MENU("mnuFileOpen", on_file_open);
	CONNECT_MENU("mnuFileQuit", on_file_quit);
	CONNECT_MENU("mnuHelpAbout", on_help_about);

	CONNECT_TOOLBAR_BUTTON("tbtnOpen", on_file_open);
	CONNECT_TOOLBAR_BUTTON("tbtnAbout", on_help_about);
}


/**
 * Menu handler: File -> Open
 **/
void CMainWindow::on_file_open()
{
	// create open/choose file dialog:
	Gtk::FileChooserDialog l_dlg("Please choose a file", Gtk::FILE_CHOOSER_ACTION_OPEN);

	l_dlg.set_transient_for(*this);

	// set up buttons:
	l_dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	l_dlg.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	// set up filters:
	Gtk::FileFilter l_filterNfo;
	l_filterNfo.set_name("NFO Files");
	l_filterNfo.add_mime_type("text/x-nfo");
	l_filterNfo.add_pattern("*.nfo");
	l_filterNfo.add_pattern("*.diz");
	l_filterNfo.add_pattern("*.asc");
	l_dlg.add_filter(l_filterNfo);

	Gtk::FileFilter l_filterText;
	l_filterText.set_name("Text Files");
	l_filterText.add_mime_type("text/plain");
	l_dlg.add_filter(l_filterText);

	Gtk::FileFilter l_filterAll;
	l_filterAll.set_name("All Files");
	l_filterAll.add_pattern("*");
	l_dlg.add_filter(l_filterAll);

	// show dialog:
	int l_result = l_dlg.run();

	// handle response:
	if(l_result == Gtk::RESPONSE_OK)
	{
		OpenFile(l_dlg.get_filename());
	}
}

/**
 * helper function that acts when some button in the about dialog has been clicked.
 **/
void CMainWindow::on_help_about_response(int)
{
	if(m_pAboutDlg)
	{
		m_pAboutDlg->hide();
	}
}

/**
 * Menu handler: Help -> About
 **/
void CMainWindow::on_help_about()
{
	if(!m_pAboutDlg)
	{
		// on first call, get dialog window from builder...
		m_refGlade->get_widget("wndAbout", m_pAboutDlg);

		// and set it up.
		if(m_pAboutDlg)
		{
			m_pAboutDlg->signal_response().connect(sigc::mem_fun(*this, &CMainWindow::on_help_about_response));
			m_pAboutDlg->set_transient_for(*this);
		}
	}

	if(m_pAboutDlg)
	{
		m_pAboutDlg->show();
	}
}

/**
 * Menu handler: File -> Quit
 **/
void CMainWindow::on_file_quit()
{
	hide();
}


bool CMainWindow::OpenFile(const std::string a_filePath)
{
	if(m_pViewCtrl->OpenFile(a_filePath))
	{

		return true;
	}

	return false;
}


/**
 * Main window destructor
 **/
CMainWindow::~CMainWindow()
{
}

