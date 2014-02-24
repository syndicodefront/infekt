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
	void PushGraphicRendition(size_t a_row, size_t a_col, const std::vector<uint8_t>& a_params);
	void PushUsedSection(size_t a_row, size_t a_col_from, size_t a_length);

	bool HasColors() const { return !m_stopsFore.empty() || !m_stopsBack.empty(); }

	// returns false for default color, true + set ar_color otherwise.
	bool GetForegroundColor(size_t a_row, size_t a_col, uint32_t a_defaultColor, uint32_t& ar_color) const;

	// returns false for entire line = default color, true + ar_sections with number of columns + ar_colors with colors otherwise.
	bool GetLineBackgrounds(size_t a_row, uint32_t a_defaultColor, size_t a_width,
		std::vector<size_t>& ar_sections, std::vector<uint32_t>& ar_colors) const;

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

	typedef struct _nfo_color_stop {
		ENFOColor color;
		uint32_t color_rgba;
		bool bold;

		_nfo_color_stop()
			: color(NFOCOLOR_DEFAULT), color_rgba(0), bold(false) {}
	} SNFOColorStop;

	std::map<ENFOColor, uint32_t> m_rgbMapping;

	typedef std::map<size_t, std::map<size_t, SNFOColorStop>> TColorStopMap;

	// (row, (col, stop))
	TColorStopMap m_stopsFore;
	TColorStopMap m_stopsBack;

	SNFOColorStop m_previousFore;
	SNFOColorStop m_previousBack;

	typedef std::map<size_t, std::map<size_t, size_t>> TUsedSectionMap;

	// (row, (col, width))
	TUsedSectionMap m_usedSections;

	void CreateColorStop(TColorStopMap& target_map, size_t a_row, size_t a_col, int intensity,
		ENFOColor color, uint32_t color_rgba, SNFOColorStop& previous) const;
	bool InterpretAdvancedColor(const std::vector<uint8_t>& a_params, ENFOColor& ar_color, uint32_t& ar_rgba) const;

	bool FindRow(const TColorStopMap& a_stops, size_t a_row, size_t& ar_row) const;
	uint32_t GetRGB(const SNFOColorStop& a_stop) const;
};

typedef shared_ptr<CNFOColorMap> PNFOColorMap;

#endif /* !_NFO_COLORMAP_H */
