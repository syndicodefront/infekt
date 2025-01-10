/**
 * Copyright (C) 2014 syndicode
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
#include "nfo_colormap.h"

#define NFORGB(R, G, B) (255 | ((B) << 8) | ((G) << 16) | ((R) << 24))

CNFOColorMap::CNFOColorMap()
		: m_rgbMapping(), m_stopsFore(), m_stopsBack(), m_previousFore(), m_previousBack(), m_usedSections()
{
	// default mapping = xterm colors
	m_rgbMapping[NFOCOLOR_BLACK] = NFORGB(0, 0, 0);
	m_rgbMapping[NFOCOLOR_RED] = NFORGB(205, 0, 0);
	m_rgbMapping[NFOCOLOR_GREEN] = NFORGB(0, 205, 0);
	m_rgbMapping[NFOCOLOR_YELLOW] = NFORGB(205, 205, 0);
	m_rgbMapping[NFOCOLOR_BLUE] = NFORGB(0, 0, 238);
	m_rgbMapping[NFOCOLOR_MAGENTA] = NFORGB(205, 0, 205);
	m_rgbMapping[NFOCOLOR_CYAN] = NFORGB(0, 205, 205);
	m_rgbMapping[NFOCOLOR_GRAY] = NFORGB(229, 229, 229);
	m_rgbMapping[NFOCOLOR_DARK_GRAY] = NFORGB(127, 127, 127);
	m_rgbMapping[NFOCOLOR_BRIGHT_RED] = NFORGB(255, 0, 0);
	m_rgbMapping[NFOCOLOR_BRIGHT_GREEN] = NFORGB(0, 255, 0);
	m_rgbMapping[NFOCOLOR_BRIGHT_YELLOW] = NFORGB(255, 255, 0);
	m_rgbMapping[NFOCOLOR_BRIGHT_BLUE] = NFORGB(92, 92, 255);
	m_rgbMapping[NFOCOLOR_PINK] = NFORGB(255, 0, 255);
	m_rgbMapping[NFOCOLOR_BRIGHT_CYAN] = NFORGB(0, 255, 255);
	m_rgbMapping[NFOCOLOR_WHITE] = NFORGB(255, 255, 255);

	/*/ default mapping = VGA colors
	m_rgbMapping[NFOCOLOR_BLACK] = NFORGB(0, 0, 0);
	m_rgbMapping[NFOCOLOR_RED] = NFORGB(170, 0, 0);
	m_rgbMapping[NFOCOLOR_GREEN] = NFORGB(0, 170, 0);
	m_rgbMapping[NFOCOLOR_YELLOW] = NFORGB(170, 85, 0);
	m_rgbMapping[NFOCOLOR_BLUE] = NFORGB(0, 0, 170);
	m_rgbMapping[NFOCOLOR_MAGENTA] = NFORGB(170, 0, 170);
	m_rgbMapping[NFOCOLOR_CYAN] = NFORGB(0, 170, 170);
	m_rgbMapping[NFOCOLOR_GRAY] = NFORGB(170, 170, 170);
	m_rgbMapping[NFOCOLOR_DARK_GRAY] = NFORGB(85, 85, 85);
	m_rgbMapping[NFOCOLOR_BRIGHT_RED] = NFORGB(255, 85, 85);
	m_rgbMapping[NFOCOLOR_BRIGHT_GREEN] = NFORGB(85, 255, 85);
	m_rgbMapping[NFOCOLOR_BRIGHT_YELLOW] = NFORGB(255, 255, 85);
	m_rgbMapping[NFOCOLOR_BRIGHT_BLUE] = NFORGB(85, 85, 255);
	m_rgbMapping[NFOCOLOR_PINK] = NFORGB(255, 85, 255);
	m_rgbMapping[NFOCOLOR_BRIGHT_CYAN] = NFORGB(85, 255, 255);
	m_rgbMapping[NFOCOLOR_WHITE] = NFORGB(255, 255, 255);*/
}

void CNFOColorMap::Clear()
{
	m_stopsFore.clear();
	m_stopsBack.clear();

	m_previousBack = SNFOColorStop();
	m_previousFore = SNFOColorStop();

	m_usedSections.clear();
}

void CNFOColorMap::PushGraphicRendition(size_t a_row, size_t a_col, const std::vector<uint8_t> &a_params)
{
	ENFOColor fore_color = _NFOCOLOR_MAX;
	ENFOColor back_color = _NFOCOLOR_MAX;
	int intensity_fore = -1;
	int intensity_back = -1;
	uint32_t fore_color_rgba = 0, back_color_rgba = 0;

	for (uint8_t p : a_params)
	{
		// from SGR parameters on http://en.wikipedia.org/wiki/ANSI_escape_code#CSI_codes

		switch (p)
		{
		case 0: // Reset / Normal
			intensity_fore = intensity_back = 0;
			fore_color = NFOCOLOR_DEFAULT;
			back_color = NFOCOLOR_DEFAULT;
			break;
		case 1: // Bold or increased intensity
			intensity_fore = 1;
			break;
		case 4: // Used to turn on background intensity (originally underline)
		case 5: // Used to turn on background intensity (originally blink)
			intensity_back = 1;
			break;
		case 21: // Bold: off or Underline: Double
		case 22: // Normal color or intensity
			intensity_fore = 0;
			break;
		case 24: // Used to turn off background intensity (originally underline)
			intensity_back = 0;
			break;
		case 38: // Set xterm-256 text color (foreground)
			if (!InterpretAdvancedColor(a_params, fore_color, fore_color_rgba))
				return;
			break;
		case 39: // Default text color (foreground)
			fore_color = NFOCOLOR_DEFAULT;
			break;
		case 48: // Set xterm-256 background color
			if (!InterpretAdvancedColor(a_params, back_color, back_color_rgba))
				return;
			break;
		case 49: // Default background color
			back_color = NFOCOLOR_DEFAULT;
			break;
		default:
			if (p >= 30 && p <= 37)
			{
				fore_color = static_cast<ENFOColor>(NFOCOLOR_BLACK + (p - 30));
			}
			else if (p >= 40 && p <= 47)
			{
				back_color = static_cast<ENFOColor>(NFOCOLOR_BLACK + (p - 40));
			}
			else if (p >= 90 && p <= 97)
			{
				fore_color = static_cast<ENFOColor>(NFOCOLOR_DARK_GRAY + (p - 90));
			}
			else if (p >= 100 && p <= 107)
			{
				back_color = static_cast<ENFOColor>(NFOCOLOR_DARK_GRAY + (p - 100));
			}
#if defined(_DEBUG) && defined(_WIN32)
			else
			{
				wchar_t msg[60]{};
				wsprintf(msg, L"unknown SGR: %d\n", p);
				::OutputDebugStringW(msg);
			}
#endif
		} // end of switch
	}		// end of for loop

	CreateColorStop(m_stopsFore, a_row, a_col, intensity_fore, fore_color, fore_color_rgba, m_previousFore);
	CreateColorStop(m_stopsBack, a_row, a_col, intensity_back, back_color, back_color_rgba, m_previousBack);
}

void CNFOColorMap::CreateColorStop(TColorStopMap &target_map, size_t a_row, size_t a_col, int intensity, ENFOColor color, uint32_t color_rgba, SNFOColorStop &previous) const
{
	if (intensity == -1)
	{
		intensity = previous.bold ? 1 : 0;
	}

	// apply intensity, if necessary:
	ENFOColor work_color(color == _NFOCOLOR_MAX ? previous.color : color);

	if (work_color >= _NFOCOLOR_MAX)
		; // do nothing
	else if (work_color > 8 && intensity == 0)
		color = static_cast<ENFOColor>((int)work_color - 8);
	else if (work_color < 9 && intensity == 1)
		color = static_cast<ENFOColor>((int)work_color + 8);

	if (color != _NFOCOLOR_MAX)
	{
		SNFOColorStop stop(previous);

		stop.color = color;
		stop.color_rgba = color_rgba;
		stop.bold = (intensity != 0);

		target_map[a_row][a_col] = stop;

		previous = stop;
	}
	else if (intensity != -1)
	{
		previous.bold = (intensity != 0);
	}
}

bool CNFOColorMap::InterpretAdvancedColor(const std::vector<uint8_t> &a_params, ENFOColor &ar_color, uint32_t &ar_rgba) const
{
	ar_color = NFOCOLOR_RGB; // default

	if (a_params.size() == 5 && a_params[1] == 2)
	{
		ar_rgba = NFORGB(a_params[2], a_params[3], a_params[4]);
	}
	else if (a_params.size() == 3 && a_params[1] == 5)
	{
		uint8_t p = a_params[2];

		if (p >= 0 && p <= 7) // standard colors (as in ESC [ 30..37 m)
		{
			ar_color = static_cast<ENFOColor>(NFOCOLOR_BLACK + p);
		}
		else if (p >= 8 && p <= 0x0F) // high intensity colors (as in ESC [ 90..97 m)
		{
			ar_color = static_cast<ENFOColor>(NFOCOLOR_DARK_GRAY + (p - 8));
		}
		else if (p >= 0x10 && p <= 0xE7) // 6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
		{
			p -= 0x10;

			uint8_t r = p / 36,
							g = (p - r * 36) / 6,
							b = (p - r * 36 - g * 6);

			ar_rgba = NFORGB((255 / 5) * r, (255 / 5) * g, (255 / 5) * b);
		}
		else // 0xe8-0xff: grayscale from black to white in 24 steps
		{
			uint8_t g = (255 / 23) * (p - 0xE8);

			ar_rgba = NFORGB(g, g, g);
		}
	}
	else
	{
		return false;
	}

	return true;
}

// unfortunately it was only later noticed that backgrounds for unused areas
// (i.e. where the cursor has never been) shall remain default-colored...
void CNFOColorMap::PushUsedSection(size_t a_row, size_t a_col_from, size_t a_length)
{
	auto &row = m_usedSections[a_row];

	// try to join adjacent sections:
	if (!row.empty() && a_col_from == row.rbegin()->first + row.rbegin()->second)
	{
		row.rbegin()->second += a_length;
	}
	else
	{
		row[a_col_from] = a_length;
	}

	// update stops for non-linear jumps:
	m_stopsFore[a_row][a_col_from] = m_previousFore;
	m_stopsBack[a_row][a_col_from] = m_previousBack;
}

bool CNFOColorMap::FindRow(const TColorStopMap &a_stops, size_t a_row, size_t &ar_row) const
{
	for (size_t row = a_row; !a_stops.empty(); --row)
	{
		if (a_stops.find(row) != a_stops.end())
		{
			ar_row = row;
			return true;
		}

		if (row == 0 // make sure the size_t never wraps around
				|| row == a_stops.begin()->first)
		{
			break;
		}
	}

	return false;
}

bool CNFOColorMap::GetForegroundColor(size_t a_row, size_t a_col, uint32_t a_defaultColor, uint32_t &ar_color) const
{
	size_t row;

	ar_color = a_defaultColor;

	if (!FindRow(m_stopsFore, a_row, row))
	{
		return false;
	}

	if (row == a_row)
	{
		const auto &row_data = m_stopsFore.find(row)->second;
		auto walk_it = row_data.begin(), it = walk_it;

		// check if first entry in this row is beyond the request col.
		// if so, fall back to previous row.
		if (walk_it->first > a_col)
		{
			size_t previous_row;

			if (a_row == 0 || !FindRow(m_stopsFore, a_row - 1, previous_row))
			{
				return false;
			}
			else
			{
				// YES; THIS IS A GOTO STATEMENT! HAAA!
				row = previous_row;
				goto take_previous_row;
			}
		}

		// find/get matching stop in this line:
		while (walk_it != row_data.end() && walk_it->first <= a_col)
		{
			it = walk_it++;
		}

		if (it->second.color == NFOCOLOR_DEFAULT)
		{
			return false;
		}
		else
		{
			ar_color = GetRGB(it->second);
		}
	}
	else
	{
	take_previous_row:
		const SNFOColorStop &last_stop = m_stopsFore.find(row)->second.rbegin()->second;

		if (last_stop.color == NFOCOLOR_DEFAULT)
		{
			return false;
		}

		ar_color = GetRGB(last_stop);
	}

	return true;
}

bool CNFOColorMap::GetLineBackgrounds(size_t a_row, uint32_t a_defaultColor, size_t a_width,
																			std::vector<size_t> &ar_sections, std::vector<uint32_t> &ar_colors) const
{
	size_t row;
	auto it_row_sections = m_usedSections.find(a_row);

	if (it_row_sections == m_usedSections.end())
	{
		return false;
	}

	if (!FindRow(m_stopsBack, a_row, row))
	{
		return false;
	}

	std::vector<size_t> l_sections;
	std::vector<uint32_t> l_colors;

	const auto &row_data = m_stopsBack.find(row)->second;

	if (row == a_row)
	{
		size_t first_col = row_data.begin()->first;

		if (first_col != 0)
		{
			// get color that is valid before this line starts
			size_t previous_row;

			if (row == 0 || !FindRow(m_stopsBack, row - 1, previous_row))
			{
				l_colors.push_back(a_defaultColor);
			}
			else
			{
				const SNFOColorStop &last_stop = m_stopsBack.find(previous_row)->second.rbegin()->second;

				l_colors.push_back(last_stop.color == NFOCOLOR_DEFAULT ? a_defaultColor : GetRGB(last_stop));
			}

			l_sections.push_back(first_col - 0);
		}

		size_t prev_end_col = 0;
		size_t index = 0;

		for (const auto &sub : row_data)
		{
			l_colors.push_back(sub.second.color == NFOCOLOR_DEFAULT ? a_defaultColor : GetRGB(sub.second));

			if (index++ > 0)
			{
				// width for entry N can only be calculated from (N+1)->first!
				_ASSERT(sub.first > prev_end_col);

				l_sections.push_back(sub.first - prev_end_col);
			}

			prev_end_col = sub.first;
		}

		l_sections.push_back(a_width - prev_end_col);
	}
	else
	{
		// paint entire row using the color from the previous line.

		const SNFOColorStop &last_stop = row_data.rbegin()->second;

		if (last_stop.color == NFOCOLOR_DEFAULT)
		{
			return false;
		}

		l_sections.push_back(a_width);
		l_colors.push_back(GetRGB(last_stop));
	}

	// unused parts (as indicated by m_usedSections) must be default-colored.

	_ASSERT(l_sections.size() > 0 && l_sections.size() == l_colors.size());

	size_t running_cols = 0;
	uint32_t prev_color;

	for (size_t col = 0; col < a_width; ++col)
	{
		// identify used section:
		size_t used_width = 0;
		uint32_t new_color;

		for (const auto &used_section : it_row_sections->second)
		{
			if (used_section.first <= col && col < used_section.first + used_section.second)
			{
				used_width = used_section.second;
				break;
			}
		}

		if (used_width == 0)
		{
			new_color = a_defaultColor;
		}
		else
		{
			// Identify principal section color.
			// This could be optimized using std::min(used_width, section_width),
			// but it seems to be okay for now...
			bool found = false;
			size_t i = 0, walking_col = 0;
			while (i < l_sections.size())
			{
				size_t section_width = l_sections[i];

				if (col >= walking_col && col < walking_col + section_width)
				{
					new_color = l_colors[i];
					found = true;
					break;
				}

				walking_col += section_width;
				++i;
			}

			_ASSERT(found);

			if (!found)
			{
				// make bug super visible.
				return false;
			}
		}

		if (running_cols == 0)
		{
			// init with first color
			prev_color = new_color;
			running_cols = 1;
		}
		else if (new_color == prev_color)
		{
			// color hasn't changed
			++running_cols;
		}
		else
		{
			ar_colors.push_back(prev_color);
			ar_sections.push_back(running_cols);

			running_cols = 1;
			prev_color = new_color;
		}
	}

	if (running_cols > 0)
	{
		ar_colors.push_back(prev_color);
		ar_sections.push_back(running_cols);
	}

	return true;
}

uint32_t CNFOColorMap::GetRGB(const SNFOColorStop &a_stop) const
{
	if (a_stop.color == NFOCOLOR_RGB)
	{
		return a_stop.color_rgba;
	}
	else
	{
		const auto &it = m_rgbMapping.find(a_stop.color);

		if (it != m_rgbMapping.end())
		{
			return it->second;
		}

		_ASSERT(false);

		return 0;
	}
}

bool CNFOColorMap::_nfo_color_stop::operator==(const _nfo_color_stop &other) const
{
	return color == other.color && bold == other.bold && (color != NFOCOLOR_RGB || color_rgba == other.color_rgba);
}
