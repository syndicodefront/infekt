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

// based on
// http://mxr.mozilla.org/mozilla1.9.2/source/gfx/thebes/src/gfxBlur.cpp
// (GPL v2)

#ifndef _CAIRO_BOX_BLUR_H
#define _CAIRO_BOX_BLUR_H

class CCairoBoxBlur
{
public:
	CCairoBoxBlur(int a_width, int a_height, int a_blurRadius);
	virtual ~CCairoBoxBlur();

	/**
	* Returns the context that should be drawn to supply the alpha mask to be
	* blurred. If the returned surface is null, then there was an error in
	* its creation.
	**/
	cairo_t* GetContext() { return m_context; }

	/**
	 * Does the actual blurring and mask applying. Users of this object
	 * must have drawn whatever they want to be blurred onto the internal
	 * gfxContext returned by GetContext before calling this.
	 **/
	bool Paint(cairo_t* a_destination);
protected:
	bool m_useFallback;
	int m_width, m_height;
	// Blur radius in pixels:
	int m_blurRadius;
	// Context of the temporary alpha surface:
	cairo_t* m_context;
	// The temporary alpha surface:
	cairo_surface_t* m_imgSurface;
};


#endif /* !_CAIRO_BOX_BLUR_H */
