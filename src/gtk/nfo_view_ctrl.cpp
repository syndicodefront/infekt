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
#include "nfo_view_ctrl.h"


CGtkNfoViewCtrl::CGtkNfoViewCtrl(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
	: Gtk::Layout(cobject),
	m_refGlade(refGlade)
{
#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	signal_expose_event().connect(sigc::mem_func(*this, &CGtkNfoViewCtrl::on_expose_event), false);
#endif
}


bool CGtkNfoViewCtrl::on_expose_event(GdkEventExpose* event)
{
	Glib::RefPtr<Gdk::Window> refWindow = this->get_bin_window();

	if(refWindow)
	{
		int l_visibleWidth, l_visibleHeight;
		refWindow->get_size(l_visibleWidth, l_visibleHeight); // inherited from Gdk::Drawable

		Cairo::RefPtr<Cairo::Context> l_cr = refWindow->create_cairo_context();

		if(event)
		{
			l_cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
			l_cr->clip();
		}

		
	}

	return true;	
}


CGtkNfoViewCtrl::~CGtkNfoViewCtrl()
{
}



