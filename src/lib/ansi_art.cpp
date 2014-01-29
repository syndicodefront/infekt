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

#include "stdafx.h"
#include "ansi_art.h"
#include "util.h"

using std::wstring;


CAnsiArt::CAnsiArt(size_t a_widthLimit, size_t a_heightLimit, size_t a_hintWidth, size_t a_hintHeight) :
	m_widthLimit(a_widthLimit),
	m_heightLimit(a_heightLimit),
	m_hintWidth(a_hintWidth),
	m_hintHeight(a_hintHeight),
	m_maxLineLength(0)
{
}


bool CAnsiArt::Parse(const wstring& a_text)
{
	m_commands.clear();

	typedef enum {
		PARSERERROR = -1,
		ANYTEXT = 1,
		ESC_CHAR,
		ESC_BRACKET,
		ESC_DATA,
		ESC_COMMAND,
	} parser_state_e;

	parser_state_e parser_state = ANYTEXT;
	wstring data;

	for(wchar_t c : a_text)
	{
		if(c == L'\x2190')
		{
			if(parser_state != ANYTEXT)
			{
				parser_state = PARSERERROR;
				break;
			}

			if(!data.empty())
			{
				ansi_command_t tmp = { 0, data };
				m_commands.push_back(tmp);
				data.clear();
			}

			parser_state = ESC_BRACKET;
		}
		else if(c == L'[' && parser_state == ESC_BRACKET)
		{
			parser_state = ESC_DATA;
		}
		else if(parser_state == ESC_DATA)
		{
			if(iswdigit(c) || c == L';' || c == L'?')
			{
				data += c;
			}
			else if(iswalpha(c))
			{
				ansi_command_t tmp = { c, data };
				m_commands.push_back(tmp);
				data.clear();

				parser_state = ANYTEXT;
			}
			else
			{
				parser_state = PARSERERROR;
				break;
			}
		}
		else if(parser_state == ANYTEXT)
		{
			data += c;
		}
		else
		{
			parser_state = PARSERERROR;
			break;
		}
	}

	if(parser_state == ANYTEXT)
	{
		if(!data.empty())
		{
			ansi_command_t tmp = { 0, data };
			m_commands.push_back(tmp);
		}
	}
	else
	{
		parser_state = PARSERERROR;
	}

	if(parser_state == PARSERERROR)
	{
		return false;
	}

	return !m_commands.empty();
}


bool CAnsiArt::Process()
{
	size_t l_hintWidth = m_hintWidth ? m_hintWidth : 80;

	TwoDimVector<wchar_t> screen(
		// plus 1 just for good measure:
		(m_hintHeight ? m_hintHeight : 999) + 1,
		l_hintWidth + 1,
		L' ');

	std::stack<std::pair<size_t, size_t> > saved_positions;
	size_t x = 0, y = 0;

	for(const ansi_command_t cmd : m_commands)
	{
		int x_delta = 0, y_delta = 0;
		int n = 0, m = 0;

		if(cmd.cmd != 0) {
			// this could be done somewhat nicer, but okay for now:
			wstring::size_type pos;

			n = std::max(_wtoi(cmd.data.c_str()), 1);

			if((pos = cmd.data.find(L';')) != wstring::npos)
			{
				m = std::max(_wtoi(cmd.data.substr(pos + 1).c_str()), 1);
			}
		}

		switch(cmd.cmd)
		{
			case 0: { // put text to current position
				for(wchar_t c : cmd.data)
				{
					if(c == L'\r')
					{
						// ignore CR
					}
					else if(c == L'\n' || (l_hintWidth != 0 && x == l_hintWidth - 1))
					{
						if(y >= screen.GetRows() - 1)
						{
							size_t new_rows = screen.GetRows() + std::max(size_t(50), y - (screen.GetRows() - 1));

							if(new_rows > m_heightLimit || new_rows < screen.GetRows() /* overflow safeguard */)
							{
								return false;
							}

							screen.Extend(new_rows, screen.GetCols(), L' ');
						}

						++y;
						x = 0;
					}
					else
					{
						if(x >= screen.GetCols() - 1)
						{
							size_t new_cols = screen.GetCols() + std::max(size_t(50), x - (screen.GetCols() - 1));

							if(new_cols > m_widthLimit || new_cols < screen.GetCols() /* overflow safeguard */)
							{
								return false;
							}

							screen.Extend(screen.GetRows(), new_cols, L' ');
						}

						++x;
						screen[y][x] = c;
					}
				}
				break;
			}
			case L'A': { // cursor up
				y_delta = -n;
				break;
			}
			case L'B': { // cursor down
				y_delta = n;
				break;
			}
			case L'C': { // cursor forward
				x_delta = n;
				break;
			}
			case L'D': { // cursor back
				x_delta = -n;
				break;
			}
			case L'E': { // cursor to beginning of next line
				y_delta = n;
				x = 0;
				break;
			}
			case L'F': { // cursor to beginning of previous line
				y_delta = -n;
				x = 0;
				break;
			}
			case L'G': { // move to given column
				x = n - 1;
				break;
			}
			case L'H':
			case L'f': { // moves the cursor to row n, column m
				y = n - 1;
				x = m - 1;
				break;
			}
			case L'J': { // erase display
				// *TODO*
				if(n == 2)
				{
					x = y = 0;
				}
				break;
			}
			case L'K': { // erase in line
				// *TODO*
				break;
			}
			case L's': { // save cursor pos
				saved_positions.push(std::pair<size_t, size_t>(x, y));
				break;
			}
			case L'u': { // restore cursor pos
				if(!saved_positions.empty()) {
					const std::pair<size_t, size_t> pos = saved_positions.top();
					x = pos.first;
					y = pos.second;
					saved_positions.pop();
				}
				break;
			}
			case L'm': { // rainbows and stuff!
				// *TODO*
				break;
			}
			case 'S': // scroll up
			case 'T': // scroll down
			case 'n': // report cursor position
			default:
				// unsupported, ignore
				;
		}
		
		if(y_delta < 0 && static_cast<size_t>(std::abs(y_delta)) <= y)
		{
			y += y_delta;
		}
		else if(y_delta > 0)
		{
			y += y_delta;
		}
		else if(y_delta != 0)
		{
			// out of bounds
			return false;
		}

		if(x_delta < 0 && static_cast<size_t>(std::abs(x_delta)) <= x)
		{
			x += x_delta;
		}
		else if(x_delta > 0)
		{
			x += x_delta;
		}
		else if(x_delta != 0)
		{
			// out of bounds
			return false;
		}

		if(x >= screen.GetCols() || y >= screen.GetRows())
		{
			if(x >= m_widthLimit || y >= m_heightLimit)
			{
				return false;
			}

			screen.Extend(y + 1, x + 1, L' ');
		}
	}

	// and finally read lines from "screen" into internal structures:

	m_maxLineLength = 0;
	m_lines.clear();

	for(size_t row = 0; row < screen.GetRows(); ++row)
	{
		size_t line_used = 0;

		for(size_t col = screen.GetCols() - 1; col >= 0 && col < screen.GetCols(); col--)
		{
			if(screen[row][col] != L' ')
			{
				line_used = col + 1;
				break;
			}
		}

		m_lines.push_back(wstring(screen[row].begin(), screen[row].begin() + line_used));

		if(line_used > m_maxLineLength)
		{
			m_maxLineLength = line_used;
		}
	}

	// kill empty trailing lines:

	while(!m_lines.empty())
	{
		if(m_lines[m_lines.size() - 1].find_first_not_of(L" ") != wstring::npos)
		{
			break;
		}

		m_lines.pop_back();
	}

	return true;
}


wstring CAnsiArt::GetAsClassicText() const
{
	wstring result;

	for(auto line : m_lines)
	{
		result += line;
		result += L'\n';
	}

	return result;
}
