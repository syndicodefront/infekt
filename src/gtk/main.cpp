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
#include <gtkmm/main.h>


int main (int argc, char* argv[])
{
	Gtk::Main kit(argc, argv);

	/* this could throw some exceptions, but we don't care for now. */

	// get a Gtk::Builder instance...
	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create();

	// ...from our XML file:
	refBuilder->add_from_file(_PREFIX_ "/share/infekt/infektwindowdata.glade");

	CMainWindow* pMainWindow = NULL;
	refBuilder->get_widget_derived("wndMain", pMainWindow);

	if(pMainWindow)
	{
		kit.run(*pMainWindow);
	}

	delete pMainWindow;

	return 0;
}

