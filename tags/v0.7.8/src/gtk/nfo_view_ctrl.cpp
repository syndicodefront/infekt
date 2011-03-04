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
	m_mode = NFO_VIEW_RENDERED;

#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	signal_expose_event().connect(sigc::mem_fun(*this, &CGtkNfoViewCtrl::on_expose_event), false);
#endif
}


bool CGtkNfoViewCtrl::OpenFile(const std::string& a_filePath)
{
	// reset data from previous file:
	m_renderer.UnAssignNFO();
	m_classicRenderer.UnAssignNFO();
	m_textOnlyRenderer.UnAssignNFO();

	delete m_pNfo;
	delete m_pNfoTextOnly;

	// load new file:
	m_pNfo = new CNFOData();

	if(m_pNfo->LoadFromFile(a_filePath))
	{
		SwitchView(m_mode); // refresh view state data
		return true;
	}

	// if loading failed, clean up:
	delete m_pNfo;
	m_pNfo = NULL;

	return false;
}


void CGtkNfoViewCtrl::SwitchView(ENfoViewMode a_newMode)
{
	ENfoViewMode l_oldMode = m_mode;
	m_mode = a_newMode;

	CNFORenderer* l_pRenderer = GetRenderer();

	if(!l_pRenderer->HasNfoData() && m_pNfo)
	{
		if(m_mode != NFO_VIEW_TEXTONLY)
		{
			l_pRenderer->AssignNFO(m_pNfo);
		}
		else
		{
			const std::string l_stripped = CNFOData::GetStrippedTextUtf8(m_pNfo->GetTextWide());
			m_pNfoTextOnly = new CNFOData();
			m_pNfoTextOnly->SetCharsetToTry(NFOC_UTF8);

			if(!m_pNfoTextOnly->LoadFromMemory((const unsigned char*)l_stripped.c_str(), l_stripped.size()))
			{
				delete m_pNfoTextOnly;
				m_pNfoTextOnly = NULL;
			}
			else
			{
				m_textOnlyRenderer.AssignNFO(m_pNfoTextOnly);
			}
		}
	}

	this->set_size(l_pRenderer->GetWidth(), l_pRenderer->GetHeight()); // from Gtk::Layout

	ForceRedraw();
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

		CNFORenderer* l_pRenderer = GetRenderer();

		// paint the background (important for where the control is not covered by the NFO contents):
		l_cr->set_source_rgb(S_COLOR_T_CAIRO(l_pRenderer->GetBackColor()));
		l_cr->paint();

		// paint actual NFO:
		if(l_pRenderer->HasNfoData())
		{
			int l_nfoWidth = (int)l_pRenderer->GetWidth(), l_nfoHeight = (int)l_pRenderer->GetHeight();
			int l_destx = 0;

			if(GetCenterNfo() && l_nfoWidth < l_visibleWidth)
				l_destx = (l_visibleWidth - l_nfoWidth) / 2;

			l_pRenderer->DrawToClippedHandle(l_cr->cobj(), l_destx, 0);
		}
	}

	return true;	
}


/**
 * Invalidates the entire visible area, forcing a redraw through the expose event.
 **/
void CGtkNfoViewCtrl::ForceRedraw()
{
	Glib::RefPtr<Gdk::Window> refWindow = this->get_bin_window();

	if(refWindow)
	{
		int l_visibleWidth, l_visibleHeight;

		refWindow->get_size(l_visibleWidth, l_visibleHeight); // inherited from Gdk::Drawable

		Gdk::Rectangle r(0, 0, l_visibleWidth, l_visibleHeight);
		
		refWindow->invalidate_rect(r, false);
	}
}


/**
 * Returns CNFORenderer instance based on m_mode (view mode).
 **/
CNFORenderer* CGtkNfoViewCtrl::GetRenderer()
{
	switch(m_mode)
	{
		case NFO_VIEW_RENDERED: return &m_renderer;
		case NFO_VIEW_CLASSIC: return &m_classicRenderer;
		case NFO_VIEW_TEXTONLY: return &m_textOnlyRenderer;
	}

	return NULL;
}


/**
 * NFO View Control destructor.
 **/
CGtkNfoViewCtrl::~CGtkNfoViewCtrl()
{
}



