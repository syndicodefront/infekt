/**
 * Copyright (C) 2010-2014 cxxjoe
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

#ifndef _CAIRO_BOX_BLUR_H
#define _CAIRO_BOX_BLUR_H

class CCairoBoxBlur
{
public:
	CCairoBoxBlur(int a_width, int a_height, int a_blurRadius, bool a_useGPU = true);
	virtual ~CCairoBoxBlur();

	/**
	* Returns the context that should be drawn to supply the alpha mask to be
	* blurred. If the returned surface is null, then there was an error in
	* its creation.
	**/
	cairo_t* GetContext() const { return m_context; }

	/**
	 * Does the actual blurring and mask applying. Users of this object
	 * must have drawn whatever they want to be blurred onto the internal
	 * gfxContext returned by GetContext before calling this.
	 **/
	bool Paint(cairo_t* a_destination);

	void SetAllowFallback(bool allow) { m_allowFallback = allow; }
	bool IsFallbackAllowed() const { return m_allowFallback; }
protected:
	bool m_allowFallback;
	bool m_useFallback;
	int m_width, m_height;
	// Blur radius in pixels:
	int m_blurRadius;
	// Context of the temporary alpha surface:
	cairo_t* m_context;
	// The temporary alpha surface:
	cairo_surface_t* m_imgSurface;
#if defined(_WIN32) && !defined(COMPACT_RELEASE)
	static HMODULE m_hAmpDll;
#endif
};

#endif /* !_CAIRO_BOX_BLUR_H */
