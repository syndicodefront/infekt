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


/** 
 * NFO View Control constructor.
 **/
CGtkNfoViewCtrl::CGtkNfoViewCtrl(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
	: Gtk::Layout(cobject),
	m_refGlade(refGlade),

	m_classicRenderer(true),
	m_textOnlyRenderer(true)
{
	m_pNfoTextOnly = m_pNfo = NULL;
	m_centerNfo = true;

#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	signal_expose_event().connect(sigc::mem_fun(*this, &CGtkNfoViewCtrl::on_expose_event), false);
#endif
}


bool CGtkNfoViewCtrl::OpenFile(const std::string& a_filePath)
{
	m_renderer.UnAssignNFO();
	m_classicRenderer.UnAssignNFO();
	m_textOnlyRenderer.UnAssignNFO();

	delete m_pNfo;
	delete m_pNfoTextOnly;

	m_pNfo = new CNFOData();

	if(m_pNfo->LoadFromFile(a_filePath))
	{
		m_renderer.AssignNFO(m_pNfo);

		// XXX move this code:
		this->set_size(m_renderer.GetWidth(), m_renderer.GetHeight()); // from Gtk::Layout

		return true;
	}

	delete m_pNfo;
	m_pNfo = NULL;

	return false;
}


/**
 * Layout's expose event: Drawing from the "backbuffer" to the screen happens here.
 **/
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
			// set up a clip according to the invalidated area:
			l_cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
			l_cr->clip();
		}

		CNFORenderer* l_pRenderer = &m_renderer; // XXX

		l_cr->set_source_rgb(S_COLOR_T_CAIRO(l_pRenderer->GetBackColor()));
		l_cr->paint();

		if(l_pRenderer->HasNfoData())
		{
			int l_nfoWidth = (int)l_pRenderer->GetWidth(), l_nfoHeight = (int)l_pRenderer->GetHeight();
			int l_destx = 0;

			if(m_centerNfo && l_nfoWidth < l_visibleWidth)
				l_destx = (l_visibleWidth - l_nfoWidth) / 2;

			l_pRenderer->DrawToClippedHandle(l_cr->cobj(), l_destx, 0);
		}
	}

	return true;	
}


/**
 * NFO View Control destructor.
 **/
CGtkNfoViewCtrl::~CGtkNfoViewCtrl()
{
}



