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

#ifndef _ANSI_ART_H
#define _ANSI_ART_H

#include "nfo_colormap.h"
#include <string>
#include <list>

// combined processing for ANSI files
// reference: http://en.wikipedia.org/wiki/ANSI_escape_code
class CAnsiArt
{
public:
	typedef std::list<const std::wstring> TLineContainer;

public:
	CAnsiArt(size_t a_widthLimit, size_t a_heightLimit, size_t a_hintWidth, size_t a_hintHeight);

	bool Parse(const std::wstring& a_text);
	bool Process();

	const TLineContainer GetLines() const { return m_lines; }
	size_t GetMaxLineLength() const { return m_maxLineLength; }

	void SetHints(size_t a_hintWidth, size_t a_hintHeight) {
		m_hintWidth = a_hintWidth;
		m_hintHeight = a_hintHeight;
	}

	void SetLimits(size_t a_widthLimit, size_t a_heightLimit) {
		m_widthLimit = a_widthLimit;
		m_heightLimit = a_heightLimit;
	}

	std::wstring GetAsClassicText() const;
	PNFOColorMap GetColorMap() const { return m_colorMap; }

protected:
	size_t m_hintWidth;
	size_t m_hintHeight;

	size_t m_widthLimit;
	size_t m_heightLimit;

	// this is filled by Parse():

	typedef struct {
		wchar_t cmd; // \0 = regular text
		std::wstring data;
	} ansi_command_t;

	std::list<const ansi_command_t> m_commands;

	// this is filled by Process():

	TLineContainer m_lines;
	size_t m_maxLineLength;
	PNFOColorMap m_colorMap;
};

#endif /* !_ANSI_ART_H */
