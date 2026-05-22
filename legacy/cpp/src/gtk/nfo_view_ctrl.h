/**
 * Copyright (C) 2010 syndicode
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

#include "nfo_data.h"
#include "nfo_renderer.h"


typedef enum _nfo_view_mode_t
{
	NFO_VIEW_RENDERED = 1,
	NFO_VIEW_CLASSIC,
	NFO_VIEW_TEXTONLY,

	_NFO_VIEW_MAX
} ENfoViewMode;


class CGtkNfoViewCtrl : public Gtk::Layout
{
public:
	CGtkNfoViewCtrl(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~CGtkNfoViewCtrl();

	bool OpenFile(const std::string& a_filePath);
	void SwitchView(ENfoViewMode a_newMode);
	ENfoViewMode GetView() const { return m_mode; }

protected:
	/* GTK stuff */
	virtual bool on_expose_event(GdkEventExpose* event);

	Glib::RefPtr<Gtk::Builder> m_refGlade;

	/* helper methods */
	CNFORenderer* GetRenderer();
	bool GetCenterNfo() const { return m_centerNfo && m_mode != NFO_VIEW_TEXTONLY; }
	void ForceRedraw();

	/* NFO data and renderer stuff */
	CNFOData* m_pNfo;
	CNFOData* m_pNfoTextOnly;

	CNFORenderer m_renderer;
	CNFORenderer m_classicRenderer;
	CNFORenderer m_textOnlyRenderer;

	/* settings/flags */
	ENfoViewMode m_mode;
	bool m_centerNfo;
};

#endif /* _NFO_VIEW_CTRL_H */

