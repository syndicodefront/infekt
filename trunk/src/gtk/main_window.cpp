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
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/radiotoolbutton.h>

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
	} while(0);

/**
 * Helper macro that connects the clicked event of a toolbar
 * button (identified by WIDGET) to the given METHOD.
 **/
#define CONNECT_TOOLBAR_BUTTON(WIDGET, METHOD) do { \
	Gtk::ToolButton *l_btn = NULL; \
	m_refGlade->get_widget(WIDGET, l_btn); \
	if(l_btn) l_btn->signal_clicked().connect(sigc::mem_fun(*this, &CMainWindow::METHOD)); \
	} while(0);

/**
 * Helper macro that connects the toggled event of GTK Radio items, e.g.
 * RadioMenuItem and RadioToolButton (= WTYPE), identified by name (= WIDGET)
 * to the given METHOD of CMainWindow, using ARG as argument.
 **/
#define CONNECT_RADIO_ITEM(WTYPE, WIDGET, METHOD, ARG) do { \
	Gtk::WTYPE *l_wgt = NULL; \
	m_refGlade->get_widget(WIDGET, l_wgt); \
	if(l_wgt) l_wgt->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &CMainWindow::METHOD), ARG)); \
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
	CONNECT_RADIO_ITEM(RadioMenuItem, "mnuViewRendered", on_view_menu_change, NFO_VIEW_RENDERED);
	CONNECT_RADIO_ITEM(RadioMenuItem, "mnuViewClassic", on_view_menu_change, NFO_VIEW_CLASSIC);
	CONNECT_RADIO_ITEM(RadioMenuItem, "mnuViewTextOnly", on_view_menu_change, NFO_VIEW_TEXTONLY);
	CONNECT_MENU("mnuHelpAbout", on_help_about);

	CONNECT_TOOLBAR_BUTTON("tbtnOpen", on_file_open);
	CONNECT_RADIO_ITEM(RadioToolButton, "tbtnViewRendered", on_view_toolbar_change, NFO_VIEW_RENDERED);
	CONNECT_RADIO_ITEM(RadioToolButton, "tbtnViewClassic", on_view_toolbar_change, NFO_VIEW_CLASSIC);
	CONNECT_RADIO_ITEM(RadioToolButton, "tbtnViewTextOnly", on_view_toolbar_change, NFO_VIEW_TEXTONLY);
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
 * (Radio) menu handler: View -> Rendered/Classic/Text Only
 **/
void CMainWindow::on_view_menu_change(int a_newMode)
{
	SwitchViewInternal(true, true, (ENfoViewMode)a_newMode);
}

/**
 * Toolbar radio button handler: Rendered/Classic/Text Only
 **/
void CMainWindow::on_view_toolbar_change(int a_newMode)
{
	SwitchViewInternal(true, false, (ENfoViewMode)a_newMode);
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


/**
 * Loads the given file into the viewer.
 **/
bool CMainWindow::OpenFile(const std::string a_filePath)
{
	if(!m_pViewCtrl->OpenFile(a_filePath))
	{
		// XXX Error feedback
		return false;
	}

	if(true /* auto width */)
	{
		unsigned int l_nfoWidth, l_nfoHeight;
		int l_winWidth, l_winHeight;

		m_pViewCtrl->get_size(l_nfoWidth, l_nfoHeight);
		this->get_size(l_winWidth, l_winHeight);

		l_nfoWidth += 90; // (poorly) account for window borders and such

		int l_maxWidth = 1000; // XXX use actual screen size
		// or find out whether X or GTK+ or something else takes care of that.

		if(l_nfoWidth > l_maxWidth)
		{
			l_nfoWidth = l_maxWidth;
		}

		this->resize(l_nfoWidth, l_winHeight);
	}

	return true;
}


/**
 * Changes the active view mode.
 **/
void CMainWindow::SwitchView(ENfoViewMode a_newMode)
{
	SwitchViewInternal(false, false, a_newMode);
}


/**
 * Internal handler for view mode changes... deals with weirdness resulting
 * from having the same functionality in a menu and as toolbar buttons.
 **/
void CMainWindow::SwitchViewInternal(bool a_fromUI, bool a_fromMenu, ENfoViewMode a_newMode)
{
	if(a_newMode == m_pViewCtrl->GetView())
	{
		return;
	}

	Gtk::RadioToolButton* l_pToolBtn = NULL;
	Gtk::RadioMenuItem* l_pMenuItem = NULL;

	switch(a_newMode)
	{
	case NFO_VIEW_RENDERED:
		m_refGlade->get_widget("tbtnViewRendered", l_pToolBtn);
		m_refGlade->get_widget("mnuViewRendered", l_pMenuItem);
		break;
	case NFO_VIEW_CLASSIC:
		m_refGlade->get_widget("tbtnViewClassic", l_pToolBtn);
		m_refGlade->get_widget("mnuViewClassic", l_pMenuItem);
		break;
	case NFO_VIEW_TEXTONLY:
		m_refGlade->get_widget("tbtnViewTextOnly", l_pToolBtn);
		m_refGlade->get_widget("mnuViewTextOnly", l_pMenuItem);
		break;
	}

	// calling set_active invokes our toggled events again, so we need
	// some extra checks or stack overflow WILL happen! :D

	if(!a_fromUI)
	{
		l_pMenuItem->set_active(true); // menu invokes toolbar too
	}
	else if(a_fromMenu && l_pMenuItem->get_active())
	{
		l_pToolBtn->set_active(true);

		// only trigger actual view change here.
		// the other two execution paths will call this too.
		m_pViewCtrl->SwitchView(a_newMode);
	}
	else if(!a_fromMenu && l_pToolBtn->get_active())
	{
		l_pMenuItem->set_active(true);		
	}
}


/**
 * Main window destructor
 **/
CMainWindow::~CMainWindow()
{
}

