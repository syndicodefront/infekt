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

#ifndef _NFO_VIEW_CTRL_H
#define _NFO_VIEW_CTRL_H

#include <gtkmm/builder.h>
#include <gtkmm/layout.h>

class CGtkNfoViewCtrl : public Gtk::Layout
{
public:
	CGtkNfoViewCtrl(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~CGtkNfoViewCtrl();

protected:
	virtual bool on_expose_event(GdkEventExpose* event);

	Glib::RefPtr<Gtk::Builder> m_refGlade;
};

#endif /* _NFO_VIEW_CTRL_H */

