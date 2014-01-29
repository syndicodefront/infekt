/**
 * Copyright (C) 2014 cxxjoe
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

#ifndef _NFO_COLORMAP_H
#define _NFO_COLORMAP_H

class CNFOColorMap
{
public:
	CNFOColorMap();

	void Clear();
	void PushGraphicRendition(size_t a_row, size_t a_col, std::vector<uint8_t> a_params);

protected:
	typedef enum {
		NFOCOLOR_DEFAULT = 0,

		// the order from http://en.wikipedia.org/wiki/ANSI_escape_code#Colors
		// shall be retained!
		NFOCOLOR_BLACK,
		NFOCOLOR_RED,
		NFOCOLOR_GREEN,
		NFOCOLOR_YELLOW,
		NFOCOLOR_BLUE,
		NFOCOLOR_MAGENTA,
		NFOCOLOR_CYAN,
		NFOCOLOR_GRAY,
		NFOCOLOR_DARK_GRAY, // aka bright black
		NFOCOLOR_BRIGHT_RED,
		NFOCOLOR_BRIGHT_GREEN,
		NFOCOLOR_BRIGHT_YELLOW,
		NFOCOLOR_BRIGHT_BLUE,
		NFOCOLOR_PINK,
		NFOCOLOR_BRIGHT_CYAN,
		NFOCOLOR_WHITE,

		_NFOCOLOR_MAX,

		NFOCOLOR_RGB,
	} ENFOColor;

	typedef struct {
		ENFOColor color;
		uint32_t color_rgb;
		bool bold;
	} SNFOColorStop;

	std::map<size_t, std::map<size_t, SNFOColorStop>> m_stops;
};

typedef shared_ptr<CNFOColorMap> PNFOColorMap;

#endif /* !_NFO_COLORMAP_H */
