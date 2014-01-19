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

#include "stdafx.h"
#include "nfo_data.h"
#include "util.h"
#include "sauce.h"

using namespace std;


#ifdef _WIN32
#define HAVE_FILELENGTH
#else
#include <sys/stat.h>
#endif


CNFOData::CNFOData() :
	m_grid(NULL), m_utf8Grid(NULL),
	m_loaded(false), m_sourceCharset(NFOC_AUTO),
	m_lineWrap(false), m_isAnsi(false),
	m_ansiHintWidth(0), m_ansiHintHeight(0)
{
}


bool CNFOData::LoadFromFile(const _tstring& a_filePath)
{
	FILE *l_file = NULL;
	size_t l_fileBytes;

#ifdef _WIN32
	if(_tfopen_s(&l_file, a_filePath.c_str(), _T("rb")) != 0 || !l_file)
#else
	if(!(l_file = fopen(a_filePath.c_str(), "rb")))
#endif
	{
#ifdef HAVE_BOOST
		m_lastErrorDescr = FORMAT(L"Unable to open NFO file '%s' (error %d)", a_filePath % errno);
#else
		m_lastErrorDescr = L"Unable to open NFO file. Please check the file name.";
#endif

		return false;
	}

#ifdef HAVE_FILELENGTH
	l_fileBytes = _filelength(_fileno(l_file));

	if(l_fileBytes < 0)
	{
		m_lastErrorDescr = L"Unable to get NFO file size.";

		fclose(l_file);
		return false;
	}
#else
	struct stat l_fst = {0};
	if(stat(a_filePath.c_str(), &l_fst) == 0 && S_ISREG(l_fst.st_mode))
	{
		l_fileBytes = l_fst.st_size;
	}
	else
	{
		m_lastErrorDescr = L"stat() on NFO file failed.";

		fclose(l_file);
		return false;
	}
#endif

	if(l_fileBytes > 1024 * 1024 * 3)
	{
		m_lastErrorDescr = L"NFO file is too large (> 3 MB)";

		fclose(l_file);
		return false;
	}

	// we add a trailing \0.
	unsigned char* l_buf = new unsigned char[l_fileBytes + 1];
	memset(l_buf, 0, l_fileBytes + 1);

	// copy file contents into memory buffer:
	unsigned char *l_ptr = l_buf;
	size_t l_totalBytesRead = 0;
	bool l_error = false;

	while(!feof(l_file))
	{
		unsigned char l_chunkBuf[8192];
		size_t l_bytesRead;

		l_bytesRead = fread_s(&l_chunkBuf, sizeof(l_chunkBuf), sizeof(unsigned char), 8192, l_file);
		if(l_bytesRead > 0)
		{
			l_totalBytesRead += l_bytesRead;

			if(l_totalBytesRead > l_fileBytes)
			{
				l_error = true;
				break;
			}

			memmove_s(l_ptr, l_buf + l_fileBytes - l_ptr, l_chunkBuf, l_bytesRead);

			l_ptr += l_bytesRead;
		}
		else if(ferror(l_file))
		{
			l_error = true;
			break;
		}
	}

	// it's not defined what exactly happens if Load... is used a second time
	// on the same instance but the second load fails.

	m_filePath = a_filePath;

	if(!l_error)
	{
		m_loaded = LoadFromMemoryInternal(l_buf, l_fileBytes);
	}
	else
	{
		m_lastErrorDescr = L"An error occured while reading from the NFO file.";

		m_loaded = false;
	}

	delete[] l_buf;

	fclose(l_file);

	if(!m_loaded)
	{
		m_filePath = _T("");
	}

	return m_loaded;
}


bool CNFOData::LoadFromMemory(const unsigned char* a_data, size_t a_dataLen)
{
	m_filePath = _T("");

	m_loaded = LoadFromMemoryInternal(a_data, a_dataLen);

	return m_loaded;
}


static void _InternalLoad_NormalizeWhitespace(wstring& a_text)
{
	wstring l_text;
	wstring::size_type l_prevPos = 0, l_pos;

	CUtil::StrTrimRight(a_text);

	l_text.reserve(a_text.size());

	l_pos = a_text.find_first_of(L"\r\t\xA0");

	while(l_pos != wstring::npos)
	{
		l_text.append(a_text, l_prevPos, l_pos - l_prevPos);

		if(a_text[l_pos] == L'\t' || a_text[l_pos] == 0xA0)
		{
			l_text += L' ';
		}

		l_prevPos = l_pos + 1;
		l_pos = a_text.find_first_of(L"\r\t\xA0", l_prevPos);
	}

	if(l_prevPos != 0)
	{
		l_text.append(a_text.substr(l_prevPos));
		a_text = l_text;
	}

	_ASSERT(a_text.find_first_of(L"\r\t\xA0") == wstring::npos);

	// we should only have \ns and no tabs now.

	a_text += L'\n';
}


static void _InternalLoad_SplitIntoLines(const wstring& a_text, size_t& a_maxLineLen, CNFOData::TLineContainer& a_lines)
{
	size_t l_prevPos = 0, l_pos = a_text.find(L'\n');

	a_maxLineLen = 1;

	// read lines:
	while(l_pos != wstring::npos)
	{
		wstring l_line = a_text.substr(l_prevPos, l_pos - l_prevPos);

		// trim trailing whitespace:
		CUtil::StrTrimRight(l_line);

		a_lines.push_back(l_line);

		if(l_line.size() > a_maxLineLen)
		{
			a_maxLineLen = l_line.size();
		}

		l_prevPos = l_pos + 1;
		l_pos = a_text.find(L'\n', l_prevPos);
	}

	if(l_prevPos < a_text.size() - 1)
	{
		wstring l_line = a_text.substr(l_prevPos);
		CUtil::StrTrimRight(l_line);
		a_lines.push_back(l_line);
		if(l_line.size() > a_maxLineLen) a_maxLineLen = l_line.size();
	}
}


static void _InternalLoad_FixLfLf(wstring& a_text, CNFOData::TLineContainer& a_lines)
{
	// fix NFOs like Crime.is.King.German.SUB5.5.DVDRiP.DivX-GWL
	// they use \n\n instead of \r\n

	int l_evenEmpty = 0, l_oddEmpty = 0;

	size_t i = 0;
	for(auto it = a_lines.begin(); it != a_lines.end(); it++, i++)
	{
		if(it->empty())
		{
			if(i % 2) ++l_oddEmpty; else ++l_evenEmpty;
		}
	}

	int l_kill = -1;
	if(l_evenEmpty <= 0.1 * a_lines.size() && l_oddEmpty > 0.4 * a_lines.size() && l_oddEmpty < 0.6 * a_lines.size())
	{
		l_kill = 1;
	}
	else if(l_oddEmpty <= 0.1 * a_lines.size() && l_evenEmpty > 0.4 * a_lines.size() && l_evenEmpty < 0.6 * a_lines.size())
	{
		l_kill = 0;
	}

	if(l_kill >= 0)
	{
		wstring l_newContent; l_newContent.reserve(a_text.size());
		CNFOData::TLineContainer l_newLines;
		i = 0;
		for(auto it = a_lines.begin(); it != a_lines.end(); it++, i++)
		{
			if(!it->empty() || i % 2 != l_kill)
			{
				l_newLines.push_back(*it);
				l_newContent += *it;
				l_newContent += L'\n';
			}
		}
		a_lines = l_newLines;
		a_text = l_newContent;
	}
}


static void _InternalLoad_FixAnsiEscapeCodes(wstring& a_text)
{
	// http://en.wikipedia.org/wiki/ANSI_escape_code
	// ~(?:\x1B\[|\x9B)((?:\d+;)*\d+|)([\@-\~])~

	wstring::size_type l_pos = a_text.find_first_of(L"\xA2\x2190"), l_prevPos = 0;
	wstring l_newText;

	while(l_pos != wstring::npos)
	{
		bool l_go = false;

		l_newText += a_text.substr(l_prevPos, l_pos - l_prevPos);

		if(a_text[l_pos] == 0xA2)
			l_go = true; // single byte CIS
		else if(a_text[l_pos] == 0x2190 && l_pos + 1 < a_text.size() && a_text[l_pos + 1] == L'[')
		{
			l_go = true;
			++l_pos;
		}

		if(l_go)
		{
			wstring::size_type p = l_pos + 1;
			wstring l_numBuf;
			wchar_t l_finalChar = 0;

			while(p < a_text.size() && ((a_text[p] >= L'0' && a_text[p] <= L'9') || a_text[p] == L';'))
			{
				l_numBuf += a_text[p];
				++p;
			}

			if(p < a_text.size()) { l_finalChar = a_text[p]; }

			if(!l_numBuf.empty() && l_finalChar > 0)
			{
				// we only honor the first number:
				wstring::size_type l_end = l_numBuf.find(L';');
				if(l_end != wstring::npos) l_numBuf.erase(l_end);

				int l_number = _wtoi(l_numBuf.c_str());

				switch(l_finalChar)
				{
				case L'C': // Cursor Forward
					if(l_number < 1) l_number = 1;
					else if(l_number > 1024) l_number = 1024;

					for(int i = 0; i < l_number; i++) l_newText += L' ';
					break;
				}

				l_pos = p;
			}
			else if(l_numBuf.empty() && ((l_finalChar >= L'A' && l_finalChar <= L'G') || l_finalChar == L'J'
				|| l_finalChar == L'K' || l_finalChar == L'S' || l_finalChar == L'T' || l_finalChar == L's' || l_finalChar == L'u'))
			{
				// skip some known, but unsupported codes
				l_pos = p;
			}
			else if(a_text[l_pos] == 0xA2)
			{
				// dont' strip \xA2 if it's not actually an escape sequence indicator
				l_newText += a_text[l_pos];
			}
		}
		else
			l_newText += a_text[l_pos];

		l_prevPos = l_pos + 1;
		l_pos = a_text.find_first_of(L"\xA2\x2190", l_prevPos);
	}

	if(l_prevPos > 0)
	{
		if(l_prevPos < a_text.size() - 1)
		{
			l_newText += a_text.substr(l_prevPos);
		}

		a_text = l_newText;
	}
}


static void _InternalLoad_WrapLongLines(CNFOData::TLineContainer& a_lines, size_t& a_newMaxLineLen)
{
	const int l_maxLen = 80;

	// Please note that this routine is not behaving consistently
	// when it comes to taking into account leading whitespace or not.
	// The results are good however.

	CNFOData::TLineContainer l_newLines;

	for(auto it = a_lines.begin(); it != a_lines.end(); it++)
	{
		if(it->size() <= l_maxLen)
		{
			l_newLines.push_back(*it);
			continue;
		}

		wstring::size_type l_spaces = it->find_first_not_of(L' ');
		if(l_spaces == wstring::npos)
			l_spaces = 0;

		wstring l_line = *it;
		bool l_firstRun = true;

		while(l_line.size() > 0)
		{
			wstring::size_type l_cut = l_line.rfind(' ', l_maxLen);
			if(l_cut == wstring::npos || l_cut < l_spaces || l_cut == 0 || l_line.size() < l_maxLen)
				l_cut = l_maxLen;

			wstring l_new;
			if(!l_firstRun)
			{
				l_new.append(l_spaces, ' '); // whitespace level of line being split
				l_new.append(2, ' '); // some indentation to denote what happened
			}
			l_new += l_line.substr(0, l_cut);
			l_newLines.push_back(l_new);

			if(l_cut != l_maxLen)
				l_line.erase(0, l_cut + 1);
			else
				l_line.erase(0, l_cut);

			l_firstRun = false;
		}
	}

	if(l_newLines.size() != a_lines.size())
	{
		a_newMaxLineLen = 0;

		for(auto it = l_newLines.begin(); it != l_newLines.end(); it++)
		{
			a_newMaxLineLen = std::max(it->size(), a_newMaxLineLen);
		}

		a_lines = l_newLines;
	}
}


// combined processing for ANSI files
// reference: http://en.wikipedia.org/wiki/ANSI_escape_code
bool CNFOData::AnsiSysTransform(const wstring& a_text, size_t& ar_maxLineLen, TLineContainer& ar_lines)
{
	// this is filled by the parser:

	typedef struct {
		wchar_t cmd; // \0 = regular text
		std::wstring data;
	} ansi_command_t;

	std::queue<const ansi_command_t> commands;

	// the parser:

	typedef enum {
		PARSERERROR = -1,
		ANYTEXT = 1,
		ESC_CHAR,
		ESC_BRACKET,
		ESC_DATA,
		ESC_COMMAND,
	} parser_state_e;

	parser_state_e parser_state = ANYTEXT;
	std::wstring data;

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
				commands.push(tmp);
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
				commands.push(tmp);
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
			commands.push(tmp);
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

	// now draw from the queue to the "screen":

	TwoDimVector<wchar_t> screen((m_ansiHintHeight ? m_ansiHintHeight : 1000) + 10, (m_ansiHintWidth ? m_ansiHintWidth : 80) + 10, L' ');
	std::stack<std::pair<size_t, size_t> > saved_positions;
	size_t x = 0, y = 0;

	while(commands.size() > 0)
	{
		const ansi_command_t cmd = commands.front();
		int x_delta = 0, y_delta = 0;
		int n = 0, m = 0;

		if(cmd.cmd != 0) {
			// this could be done somewhat nicer, but okay for now:
			wstring::size_type pos = cmd.data.find(L';');

			if(pos == wstring::npos)
			{
				n = std::max(_wtoi(cmd.data.c_str()), 1);
			}
			else
			{
				n = std::max(_wtoi(cmd.data.c_str()), 1);
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
					else if(c == L'\n')
					{
						if(y >= screen.GetRows() - 1)
						{
							// screen too small, *TODO* extend
							return false;
						}

						++y;
						x = 0;
					}
					else
					{
						if(x >= screen.GetCols() - 1)
						{
							// screen too small, *TODO* extend
							return false;
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
				// *TODO*
				break;
			}
			case L'u': { // restore cursor pos
				// *TODO*
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
		
		if(y_delta < 0 && std::abs(y_delta) <= y)
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
			return false; // *TODO*
		}

		if(x_delta < 0 && std::abs(x_delta) <= x)
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
			return false; // *TODO*
		}

		if(x >= screen.GetCols() || y >= screen.GetRows())
		{
			// out of bounds
			return false; // *TODO*
		}

		commands.pop();
	}

	// and finally read lines from "screen" into internal structures:

	ar_maxLineLen = 0;
	ar_lines.clear();

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

		ar_lines.push_back(wstring(screen[row].begin(), screen[row].begin() + line_used));

		if(line_used > ar_maxLineLen)
		{
			ar_maxLineLen = line_used;
		}
	}

	// kill empty trailing lines:

	while(!ar_lines.empty())
	{
		if(ar_lines[ar_lines.size() - 1].find_first_not_of(L" ") != wstring::npos)
		{
			break;
		}

		ar_lines.pop_back();
	}

	return true;
}


bool CNFOData::LoadFromMemoryInternal(const unsigned char* a_data, size_t a_dataLen)
{
	bool l_loaded = false;
	size_t l_dataLen = a_dataLen;

	m_lastErrorDescr.clear();

	m_isAnsi = false; // modifying this state here (and in ReadSAUCE) is not nice

	if(ReadSAUCE(a_data, l_dataLen))
	{
		switch(m_sourceCharset)
		{
		case NFOC_AUTO:
			l_loaded = TryLoad_UTF8Signature(a_data, l_dataLen);
			if(!l_loaded) l_loaded = TryLoad_UTF16LE(a_data, l_dataLen, EA_TRY);
			if(!l_loaded) l_loaded = TryLoad_UTF16BE(a_data, l_dataLen);
			if(!l_loaded) l_loaded = TryLoad_UTF8(a_data, l_dataLen, EA_TRY);
			if(!l_loaded) l_loaded = TryLoad_CP437(a_data, l_dataLen, EA_TRY);
			break;
		case NFOC_UTF16:
			l_loaded = TryLoad_UTF16LE(a_data, l_dataLen, EA_FALSE);
			if(!l_loaded) l_loaded = TryLoad_UTF16BE(a_data, l_dataLen);
			break;
		case NFOC_UTF8_SIG:
			l_loaded = TryLoad_UTF8Signature(a_data, l_dataLen);
			break;
		case NFOC_UTF8:
			l_loaded = TryLoad_UTF8(a_data, l_dataLen, EA_FALSE);
			break;
		case NFOC_CP437:
			l_loaded = TryLoad_CP437(a_data, l_dataLen, EA_FALSE);
			break;
		case NFOC_WINDOWS_1252:
			l_loaded = TryLoad_CP252(a_data, l_dataLen);
			break;
		case NFOC_CP437_IN_UTF8:
			l_loaded = TryLoad_UTF8(a_data, l_dataLen, EA_FORCE);
			break;
		case NFOC_CP437_IN_UTF16:
			l_loaded = TryLoad_UTF16LE(a_data, l_dataLen, EA_FORCE);
			break;
		case NFOC_CP437_IN_CP437:
			l_loaded = TryLoad_CP437(a_data, l_dataLen, EA_FORCE);
			break;
		case NFOC_CP437_STRICT:
			l_loaded = TryLoad_CP437_Strict(a_data, l_dataLen);
			break;
		}
	}

	if(l_loaded)
	{
		size_t l_maxLineLen;
		TLineContainer l_lines;
		bool l_ansiError = false;

		if(!m_isAnsi)
		{
			if(m_sourceCharset != NFOC_CP437_STRICT)
			{
				_InternalLoad_NormalizeWhitespace(m_textContent);
				_InternalLoad_FixAnsiEscapeCodes(m_textContent);
			}

			_InternalLoad_SplitIntoLines(m_textContent, l_maxLineLen, l_lines);

			if(m_sourceCharset != NFOC_CP437_STRICT)
			{
				_InternalLoad_FixLfLf(m_textContent, l_lines);
			}

			if(m_lineWrap)
			{
				_InternalLoad_WrapLongLines(l_lines, l_maxLineLen);
			}
		}
		else
		{
			try
			{
				l_ansiError = !AnsiSysTransform(m_textContent, l_maxLineLen, l_lines);
			}
			catch(const std::exception& ex)
			{
				l_ansiError = true;
				(void)ex;
			}
		}

		// copy lines to grid(s):
		delete m_grid; m_grid = NULL;
		delete m_utf8Grid; m_utf8Grid = NULL;
		m_hyperLinks.clear();
		m_utf8Content.clear();

		if(l_ansiError)
		{
			m_lastErrorDescr = L"Internal problem during ANSI processing. This could be a bug, please file a report and attach the file you were trying to open.";
			return false;
		}

		if(l_lines.size() == 0 || l_maxLineLen == 0)
		{
			m_lastErrorDescr = L"Unable to find any lines in this file.";
			return false;
		}

		if(l_maxLineLen > WIDTH_LIMIT)
		{
#ifdef HAVE_BOOST
			m_lastErrorDescr = FORMAT(L"This file contains a line longer than %d chars. To prevent damage and lock-ups, we do not load it.", WIDTH_LIMIT);
#else
			L"This file contains a line that exceeds the internal maximum length limit.";
#endif
			return false;
		}

		if(l_lines.size() > LINES_LIMIT)
		{
#ifdef HAVE_BOOST
			m_lastErrorDescr = FORMAT(L"This file contains more than %d lines. To prevent damage and lock-ups, we do not load it.", WIDTH_LIMIT);
#else
			L"This file contains more lines than the internal limit.";
#endif
			return false;
		}

		m_utf8Content.reserve(m_textContent.length());

		// allocate mem:
		m_grid = new TwoDimVector<wchar_t>(l_lines.size(), l_maxLineLen + 1, 0);
		m_utf8Grid = new char[l_lines.size() * m_grid->GetCols() * 7];
		memset(m_utf8Grid, 0, l_lines.size() * m_grid->GetCols() * 7);

		// vars for hyperlink detection:
		string l_prevLinkUrl; // UTF-8
		int l_maxLinkId = 1;
		std::multimap<size_t, CNFOHyperLink>::iterator l_prevLinkIt = m_hyperLinks.end();

		// go through line by line:
		size_t i = 0; // line (row) index
		for(TLineContainer::const_iterator it = l_lines.begin();
			it != l_lines.end(); it++, i++)
		{
			int l_lineLen = static_cast<int>(it->length());

			#pragma omp parallel for
			for(int j = 0; j < l_lineLen; j++)
			{
				(*m_grid)[i][j] = (*it)[j];

				CUtil::OneCharWideToUtf8((*it)[j], &m_utf8Grid[i * m_grid->GetCols() * 7 + j * 7]);
			}

			const string l_utf8Line = CUtil::FromWideStr(*it, CP_UTF8);
			m_utf8Content += l_utf8Line;
			m_utf8Content += "\n"; // don't change this into \r\n, other code relies on it being \n

			// find hyperlinks:
			if(/* m_bFindHyperlinks == */true)
			{
				size_t l_linkPos = (size_t)-1, l_linkLen;
				bool l_linkContinued;
				string l_url, l_prevUrlCopy = l_prevLinkUrl;
				size_t l_offset = 0;

				while(FindLink(l_utf8Line, l_offset, l_linkPos, l_linkLen, l_url, l_prevUrlCopy, l_linkContinued))
				{
					const wstring wsUrl = CUtil::ToWideStr(l_url, CP_UTF8);
					int l_linkID = (l_linkContinued ? l_maxLinkId - 1 : l_maxLinkId);

					std::multimap<size_t, CNFOHyperLink>::iterator l_newItem =
						m_hyperLinks.insert(
						std::pair<size_t, CNFOHyperLink>
							(i, CNFOHyperLink(l_linkID, wsUrl, i, l_linkPos, l_linkLen))
						);

					if(!l_linkContinued)
					{
						l_maxLinkId++;
						l_prevLinkUrl = l_url;
						l_prevLinkIt = l_newItem;
					}
					else
					{
						(*l_newItem).second.SetHref(wsUrl);

						if(l_prevLinkIt != m_hyperLinks.end())
						{
							_ASSERT((*l_prevLinkIt).second.GetLinkID() == l_linkID);
							// update href of link's first line:
							(*l_prevLinkIt).second.SetHref(wsUrl);
						}

						l_prevLinkUrl = "";
					}

					l_prevUrlCopy = "";
				}

				if(l_linkPos == (size_t)-1)
				{
					// do not try to continue links when a line without any link on it is met.
					l_prevLinkUrl = "";
				}
			}
		} // end of foreach line loop.

	}
	else if(m_lastErrorDescr.empty())
	{
		m_lastErrorDescr = L"There appears to be a charset/encoding problem.";
	}

	return l_loaded;
}


bool CNFOData::TryLoad_UTF8Signature(const unsigned char* a_data, size_t a_dataLen)
{
	if(a_dataLen < 3 || a_data[0] != 0xEF || a_data[1] != 0xBB || a_data[2] != 0xBF)
	{
		// no UTF-8 signature found.
		return false;
	}

	a_data += 3;
	a_dataLen -= 3;

#ifdef _WIN32
	// use optimized calls to MultiByteToWideChar instead of generic stuff from CUtil.
	int l_dataLen = static_cast<int>(a_dataLen);

	int l_size = static_cast<int>(
		::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char*)a_data, l_dataLen, NULL, NULL));

	if(l_size && ::GetLastError() != ERROR_NO_UNICODE_TRANSLATION)
	{
		wchar_t *l_buf = new wchar_t[l_size];

		if(l_buf)
		{
			bool l_success = false;

			*l_buf = 0;

			if(::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char*)a_data, l_dataLen, l_buf, l_size) == l_size)
			{
				m_textContent = std::wstring(l_buf, l_size);

				m_sourceCharset = NFOC_UTF8_SIG;

				l_success = true;
			}

			delete[] l_buf;

			return l_success;
		}
	}

	// if we got here, there was a valid signature, but invalid UTF-8

	return false;
#else
	// for Linux... TryLoad_UTF8 does some validation, so this should be roughly equal.
	if(TryLoad_UTF8(a_data, a_dataLen, EA_FALSE))
	{
		m_sourceCharset = NFOC_UTF8_SIG;
		return true;
	}
	return false;
#endif
}


bool CNFOData::TryLoad_UTF8(const unsigned char* a_data, size_t a_dataLen, EApproach a_fix)
{
	if(utf8_validate((const char*)a_data, a_dataLen, NULL))
	{
		const string l_utf((const char*)a_data, a_dataLen);

		// the following is a typical collection of characters that indicate
		// a CP437 representation that has been (very badly) UTF-8 encoded
		// using an "ISO-8559-1 to UTF-8" or similar routine.
		if(a_fix == EA_FORCE || (a_fix == EA_TRY && (l_utf.find("\xC3\x9F") != string::npos || l_utf.find("\xC3\x8D") != string::npos)
			/* one "Eszett" or LATIN CAPITAL LETTER I WITH ACUTE (horizontal double line in 437) */ &&
			(l_utf.find("\xC3\x9C\xC3\x9C") != string::npos || l_utf.find("\xC3\x9B\xC3\x9B") != string::npos)
			/* two consecutive 'LATIN CAPITAL LETTER U WITH DIAERESIS' or 'LATIN CAPITAL LETTER U WITH CIRCUMFLEX' */ &&
			(l_utf.find("\xC2\xB1") != string::npos || l_utf.find("\xC2\xB2") != string::npos)
			/* 'PLUS-MINUS SIGN' or 'SUPERSCRIPT TWO' */))
		{
			const wstring l_unicode = CUtil::ToWideStr(l_utf, CP_UTF8);
			const string l_cp437 = CUtil::FromWideStr(l_unicode, CP_ACP);

			if(!l_cp437.empty() && TryLoad_CP437((unsigned char*)l_cp437.c_str(), l_cp437.size(), EA_FALSE))
			{
				m_sourceCharset = NFOC_CP437_IN_UTF8;

				return true;
			}

			return false;
		}
		else
		{
			m_textContent = CUtil::ToWideStr(l_utf, CP_UTF8);
		}

		m_sourceCharset = NFOC_UTF8;

		return true;
	}

	return false;
}


#define CP437_MAP_LOW 0x7F

#include "nfo_data_cp437.inc"
#include "nfo_data_cp437_strict.inc"


bool CNFOData::TryLoad_CP437(const unsigned char* a_data, size_t a_dataLen, EApproach a_fix)
{
	bool l_error = false;

	m_textContent.clear();

	if(a_fix == EA_TRY)
	{
		const std::string l_txt((const char*)a_data, a_dataLen);

		// look for bad full blocks and shadowed full blocks or black half blocks:
		if(l_txt.find("\x9A\x9A") != std::string::npos && (l_txt.find("\xFD\xFD") != std::string::npos
			|| l_txt.find("\xE1\xE1") != std::string::npos))
		{
			a_fix = EA_FORCE;
		}
	}

	// kill trailing NULL chars that some NFOs have so our
	// binary file check doesn't trigger.
	while(a_data[a_dataLen - 1] == 0 && a_dataLen > 0) a_dataLen--;

	// kill UTF-8 signature, if we got here, the NFO was not valid UTF-8:
	if(a_fix == EA_TRY && a_data[0] == 0xEF && a_data[1] == 0xBB && a_data[2] == 0xBF)
	{
		a_data += 3;
		a_dataLen -= 3;
	}

	m_textContent.resize(a_dataLen);

	#pragma omp parallel for
	for(int i = 0; i < static_cast<int>(a_dataLen); i++)
	{
		unsigned char p = a_data[i];
		
		if(p >= CP437_MAP_LOW)
		{
			if(a_fix != EA_FORCE)
			{
				m_textContent[i] = map_cp437_to_unicode_high_bit[p - CP437_MAP_LOW];
			}
			else
			{
				wchar_t l_temp = map_cp437_to_unicode_high_bit[p - CP437_MAP_LOW];

				m_textContent[i] = (l_temp >= CP437_MAP_LOW ?
					map_cp437_to_unicode_high_bit[(l_temp & 0xFF) - CP437_MAP_LOW] : l_temp);
			}
		}
		else if(p <= 0x1F)
		{
			if(p == 0)
			{
				l_error = true;
			}
			else if(p == 0x0D && i < static_cast<int>(a_dataLen) - 1 && a_data[i + 1] == 0x0A)
			{
				m_textContent[i] = L'\r';
			}
			else
			{
				m_textContent[i] = map_cp437_to_unicode_control_range[p];
			}
		}
		else
		{
			_ASSERT(p > 0x1F && p < CP437_MAP_LOW);

			m_textContent[i] = (wchar_t)p;

			if(a_fix == EA_FORCE && (p == 0x55 || p == 0x59 || p == 0x5F))
			{
				// untransliterated CAPITAL U WITH CIRCUMFLEX
				// => regular U (0x55) -- was full block (0x2588)
				// same for Y (0x59) and _ (0x5F)
				unsigned char l_next = (i < static_cast<int>(a_dataLen) - 1 ? a_data[i + 1] : 0),
					l_prev = (i > 0 ? a_data[i - 1] : 0);

				if((l_next >= 'a' && l_next <= 'z') || (l_prev >= 'a' && l_prev <= 'z') ||
					(l_next >= 'A' && l_next <= 'Z' && l_next != 'U' && l_next != 'Y' && l_next != '_') || (l_prev >= 'A' && l_prev <= 'Z' && l_prev != 'U' && l_prev != 'Y' && l_prev != '_') ||
					(l_next >= '0' && l_next <= '9') || (l_prev >= '0' && l_prev <= '9'))
				{
					// most probably a regular 'U'/'Y'/'_'
				}
				else if(p == 0x55)
					m_textContent[i] = 0x2588;
				else if(p == 0x59)
					m_textContent[i] = 0x258C;
				else if(p == 0x5F)
					m_textContent[i] = 0x2590;
			}			
		}
	}

	if(l_error)
	{
		m_lastErrorDescr = L"Binary files can not be loaded.";
		return false;
	}
	else
	{
		m_sourceCharset = (a_fix == EA_FORCE ? NFOC_CP437_IN_CP437 : NFOC_CP437);

		// try to detect ANSI art files without SAUCE records:

		if(!m_isAnsi && m_textContent.find(L"\x2190[") != std::wstring::npos && m_filePath.length() > 4
			&& _tcsicmp(m_filePath.substr(m_filePath.length() - 4).c_str(), _T(".ans")) == 0)
		{
			m_isAnsi = true;

			m_ansiHintWidth = m_ansiHintHeight = 0;
		}

		return true;
	}
}


bool CNFOData::TryLoad_CP437_Strict(const unsigned char* a_data, size_t a_dataLen)
{
	// no fuzz here, be compliant!
	// https://code.google.com/p/infekt/issues/detail?id=83

	bool l_error = false;

	m_textContent.resize(a_dataLen);

	#pragma omp parallel for
	for(int i = 0; i < static_cast<int>(a_dataLen); i++)
	{
		unsigned char p = a_data[i];
		
		if(p >= CP437_MAP_LOW)
		{
			m_textContent[i] = map_cp437_strict_to_unicode_high_bit[p - CP437_MAP_LOW];
		}
		else if(p <= 0x1F)
		{
			if(p == 0)
			{
				l_error = true;
			}
			else if(p == 0x0D && i < static_cast<int>(a_dataLen) - 1 && a_data[i + 1] == 0x0A)
			{
				m_textContent[i] = L'\r';
			}
			else if(p == 0x0A && i > 0 && a_data[i - 1] == 0x0D)
			{
				m_textContent[i] = L'\n';
			}
			else
			{
				m_textContent[i] = map_cp437_strict_to_unicode_control_range[p];
			}
		}
		else
		{
			_ASSERT(p > 0x1F && p < CP437_MAP_LOW);

			m_textContent[i] = (wchar_t)p;		
		}
	}

	if(l_error)
	{
		m_textContent.clear();
		m_lastErrorDescr = L"Binary files can not be loaded.";
		return false;
	}
	else
	{
		m_sourceCharset = NFOC_CP437_STRICT;

		return true;
	}
}


bool CNFOData::TryLoad_UTF16LE(const unsigned char* a_data, size_t a_dataLen, EApproach a_fix)
{
	if(a_dataLen < 2 || a_data[0] != 0xFF || a_data[1] != 0xFE)
	{
		// no BOM!
		return false;
	}

	// skip BOM...
	a_data += 2;

	// ...and load
	m_textContent = wstring().append((wchar_t*)(a_data), (a_dataLen - 2) / sizeof(wchar_t));

	if(m_textContent.find(L'\0') != wstring::npos)
	{
		m_lastErrorDescr = L"Binary files can not be loaded.";
		return false;
	}

	// see comments in TryLoad_UTF8...
	if(a_fix == EA_FORCE || (a_fix == EA_TRY && (m_textContent.find(L'\u00DF') != wstring::npos || m_textContent.find(L'\u00CD') != wstring::npos) &&
		(m_textContent.find(L"\u00DC\u00DC") != wstring::npos || m_textContent.find(L"\u00DB\u00DB") != wstring::npos) &&
		(m_textContent.find(L"\u00B1") != wstring::npos || m_textContent.find(L"\u00B2") != wstring::npos)))
	{
		const string l_cp437 = CUtil::FromWideStr(m_textContent, CP_ACP);

		if(!l_cp437.empty() && TryLoad_CP437((unsigned char*)l_cp437.c_str(), l_cp437.size(), EA_FALSE))
		{
			m_sourceCharset = NFOC_CP437_IN_UTF16;

			return true;
		}

		return false;
	}

	m_sourceCharset = NFOC_UTF16;

	return true;
}

#if !defined(_WIN32)
static inline unsigned short _byteswap_ushort(unsigned short val)
{
	return (val >> CHAR_BIT) | (val << CHAR_BIT);    
}
#endif

bool CNFOData::TryLoad_UTF16BE(const unsigned char* a_data, size_t a_dataLen)
{
	if(sizeof(wchar_t) != sizeof(unsigned short))
	{
		return false;
	}

	if(a_dataLen < 2 || a_data[0] != 0xFE || a_data[1] != 0xFF)
	{
		// no BOM!
		return false;
	}

	a_dataLen -= 2;

	wchar_t* l_bufStart = (wchar_t*)(a_data + 2);
	const size_t l_numWChars = a_dataLen / sizeof(wchar_t);

	wchar_t *l_newBuf = new wchar_t[l_numWChars + 1];
	memset(l_newBuf, 0, l_numWChars + 1);

	for(size_t p = 0; p < l_numWChars; p++)
	{
		l_newBuf[p] = _byteswap_ushort(l_bufStart[p]);

		if(l_newBuf[p] == 0)
		{
			m_lastErrorDescr = L"Binary files can not be loaded.";
			delete[] l_newBuf;
			return false;
		}
	}

	m_textContent = wstring().append(l_newBuf, l_numWChars);

	delete[] l_newBuf;

	m_sourceCharset = NFOC_UTF16;

	return true;
}


bool CNFOData::TryLoad_CP252(const unsigned char* a_data, size_t a_dataLen)
{
	m_textContent = CUtil::ToWideStr(std::string().append((const char*)a_data, a_dataLen), CP_ACP);

	return (!m_textContent.empty());
}


bool CNFOData::ReadSAUCE(const unsigned char* a_data, size_t& ar_dataLen)
{
	if(ar_dataLen <= SAUCE_RECORD_SIZE)
	{
		// no SAUCE record, no error.
		return true;
	}

	SAUCE l_record = {0};

	memcpy_s(&l_record, sizeof(l_record), a_data + ar_dataLen - SAUCE_RECORD_SIZE, SAUCE_RECORD_SIZE);

	// validate SAUCE header + supported features:

	if(memcmp(l_record.ID, "SAUCE", 5) != 0)
	{
		// no SAUCE record, no error.
		return true;
	}

	if(memcmp(l_record.Version, "00", 2) != 0)
	{
		m_lastErrorDescr = L"SAUCE: Unsupported file version.";

		return false;
	}

	if(l_record.DataType != SAUCEDT_CHARACTER || (l_record.FileType != SAUCEFT_CHAR_ANSI && l_record.FileType != SAUCEFT_CHAR_ASCII))
	{
		m_lastErrorDescr = L"SAUCE: Unsupported file format type.";

		return false;
	}

	// skip record + comments:

	size_t l_bytesToTrim = SAUCE_RECORD_SIZE + l_record.Comments * SAUCE_COMMENT_LINE_SIZE + SAUCE_HEADER_ID_SIZE;

	if(l_record.Comments > SAUCE_MAX_COMMENTS || ar_dataLen < l_bytesToTrim)
	{
		m_lastErrorDescr = L"SAUCE: Bad comments definition.";

		return false;
	}

	ar_dataLen = ar_dataLen - l_bytesToTrim;

	while(ar_dataLen > 0 && a_data[ar_dataLen - 1] == SAUCE_EOF)
	{
		--ar_dataLen;
	}

	m_isAnsi = (l_record.FileType == SAUCEFT_CHAR_ANSI);

	if(l_record.TInfo1 > 0 && l_record.TInfo1 < WIDTH_LIMIT * 2) // width in characters
	{
		m_ansiHintWidth = l_record.TInfo1;
	}

	if(l_record.TInfo2 > 0 && l_record.TInfo2 < LINES_LIMIT * 2) // height in lines
	{
		m_ansiHintHeight = l_record.TInfo2;
	}

	return true;
}


const wstring& CNFOData::GetLastErrorDescription() const
{
	return m_lastErrorDescr;
}


const std::_tstring CNFOData::GetFileName() const
{
#ifdef _WIN32
	const wchar_t* l_name = ::PathFindFileName(m_filePath.c_str());
	return l_name;
#else
	char *l_tmp = strdup(m_filePath.c_str());
	std::string l_result = basename(l_tmp);
	free(l_tmp);
	return l_result;
#endif
}


FILE *CNFOData::OpenFileForWritingWithErrorMessage(const std::_tstring& a_filePath)
{
	FILE *l_file = NULL;

#ifdef _WIN32
	if(_tfopen_s(&l_file, a_filePath.c_str(), _T("wb")) != 0 || !l_file)
#else
	if(!(l_file = fopen(a_filePath.c_str(), "wb")))
#endif
	{
#ifdef HAVE_BOOST
		m_lastErrorDescr = FORMAT(L"Unable to open file '%s' for writing (error %d)", a_filePath % errno);
#else
		m_lastErrorDescr = L"Unable to open file for writing. Please check the file name.";
#endif

		return NULL;
	}

	return l_file;
}


bool CNFOData::SaveToUnicodeFile(const std::_tstring& a_filePath, bool a_utf8, bool a_compoundWhitespace)
{
	FILE *l_file = OpenFileForWritingWithErrorMessage(a_filePath);

	if(!l_file)
	{
		return false;
	}

	size_t l_written = 0;
	bool l_success;

	if(a_utf8)
	{
		// write signature
		unsigned char l_sig[3] = { 0xEF, 0xBB, 0xBF };
		l_written += fwrite(l_sig, 1, sizeof(l_sig), l_file);

		// dump contents
		if(a_compoundWhitespace)
		{
			const std::string l_buf = CUtil::FromWideStr(GetWithBoxedWhitespace(), CP_UTF8);

			l_written += fwrite(l_buf.c_str(), l_buf.size(), 1, l_file);
		}
		else
		{
			l_written += fwrite(m_utf8Content.c_str(), m_utf8Content.size(), 1, l_file);
		}

		l_success = (l_written == 4);
	}
	else
	{
		// write BOM
		unsigned char l_bom[2] = { 0xFF, 0xFE };
		l_written += fwrite(l_bom, 1, sizeof(l_bom), l_file);

		// dump contents
		if(a_compoundWhitespace)
		{
			const std::wstring l_buf = GetWithBoxedWhitespace();

			l_written += fwrite(l_buf.c_str(), l_buf.size(), sizeof(wchar_t), l_file);
		}
		else
		{
			l_written += fwrite(m_textContent.c_str(), m_textContent.size(), sizeof(wchar_t), l_file);
		}

		l_success = (l_written == 4);
	}

	fclose(l_file);

	return l_success;
}


size_t CNFOData::GetGridWidth()
{
	return (m_grid ? m_grid->GetCols() : -1);
}


size_t CNFOData::GetGridHeight()
{
	return (m_grid ? m_grid->GetRows() : -1);
}


wchar_t CNFOData::GetGridChar(size_t a_row, size_t a_col)
{
	return (m_grid &&
		a_row >= 0 && a_row < m_grid->GetRows() &&
		a_col >= 0 && a_col < m_grid->GetCols() ?
		(*m_grid)[a_row][a_col] : 0);
}


char* CNFOData::GetGridCharUtf8(size_t a_row, size_t a_col)
{
	return (m_utf8Grid &&
		a_row >= 0 && a_row < m_grid->GetRows() &&
		a_col >= 0 && a_col < m_grid->GetCols() ?
		&m_utf8Grid[a_row * m_grid->GetCols() * 7 + a_col * 7] : NULL);
}


const std::wstring CNFOData::GetCharsetName(ENfoCharset a_charset)
{
	switch(a_charset)
	{
	case NFOC_AUTO:
		return L"(auto)";
	case NFOC_UTF16:
		return L"UTF-16";
	case NFOC_UTF8_SIG:
		return L"UTF-8 (Signature)";
	case NFOC_UTF8:
		return L"UTF-8";
	case NFOC_CP437:
		return L"CP 437";
	case NFOC_CP437_IN_UTF8:
		return L"CP 437 (in broken UTF-8)";
	case NFOC_CP437_IN_UTF16:
		return L"CP 437 (in broken UTF-16)";
	case NFOC_CP437_IN_CP437:
		return L"CP 437 (double encoded)";
	case NFOC_CP437_STRICT:
		return L"CP 437 (strict mode)";
	case NFOC_WINDOWS_1252:
		return L"Windows-1252";
	}

	return L"(huh?)";
}


/************************************************************************/
/* Compound Whitespace Code                                             */
/************************************************************************/

wstring CNFOData::GetWithBoxedWhitespace() const
{
	wstring l_result;

	for(size_t rr = 0; rr < m_grid->GetRows(); rr++)
	{
		for(size_t cc = 0; cc < m_grid->GetCols(); cc++)
		{
			wchar_t l_tmp = (*m_grid)[rr][cc];
			l_result += (l_tmp != 0 ? l_tmp : L' ');
		}
		l_result += L"\n";
	}

	return l_result;
}


/************************************************************************/
/* Hyper Link Code                                                      */
/************************************************************************/

const CNFOHyperLink* CNFOData::GetLink(size_t a_row, size_t a_col) const
{
	pair<multimap<size_t, CNFOHyperLink>::const_iterator, multimap<size_t, CNFOHyperLink>::const_iterator> l_range
		= m_hyperLinks.equal_range(a_row);

	for(multimap<size_t, CNFOHyperLink>::const_iterator it = l_range.first; it != l_range.second; it++)
	{
		if(a_col >= it->second.GetColStart() && a_col <= it->second.GetColEnd())
		{
			return &it->second;
		}
	}

	return NULL;
}


const CNFOHyperLink* CNFOData::GetLinkByIndex(size_t a_index) const
{
	if(a_index < m_hyperLinks.size())
	{
		multimap<size_t, CNFOHyperLink>::const_iterator it = m_hyperLinks.begin();
		for(size_t i = 0; i < a_index; i++, it++) ;
		return &it->second;
	}

	return NULL;
}


const vector<const CNFOHyperLink*> CNFOData::GetLinksForLine(size_t a_row) const
{
	vector<const CNFOHyperLink*> l_result;

	pair<multimap<size_t, CNFOHyperLink>::const_iterator, multimap<size_t, CNFOHyperLink>::const_iterator> l_range
		= m_hyperLinks.equal_range(a_row);

	for(multimap<size_t, CNFOHyperLink>::const_iterator it = l_range.first; it != l_range.second; it++)
	{
		l_result.push_back(&it->second);
	}

	return l_result;
}


/*static*/ void CNFOData::PopulateLinkTriggers()
{
#define TRGR(a, b) PLinkRegEx(new CLinkRegEx(a, b))

	// cache compiled trigger regexes because all those execute on
	// every single line, so this is an easy performance gain.

	if(!ms_linkTriggers.empty())
	{
		return;
	}

	ms_linkTriggers.push_back(TRGR("h(?:tt|xx|\\*\\*)p://", false));
	ms_linkTriggers.push_back(TRGR("h(?:tt|xx|\\*\\*)ps://", false));
	ms_linkTriggers.push_back(TRGR("www\\.", false));
	ms_linkTriggers.push_back(TRGR("\\w+\\.imdb\\.com", false));
	ms_linkTriggers.push_back(TRGR("imdb\\.com", false));
	ms_linkTriggers.push_back(TRGR("(imdb|ofdb|cinefacts|zelluloid|kino)\\.de", false));
	ms_linkTriggers.push_back(TRGR("(tinyurl|twitter|facebook|imgur|youtube)\\.com", false));
	ms_linkTriggers.push_back(TRGR("(bit\\.ly|goo\\.gl|t\\.co|youtu\\.be)", false));

	ms_linkTriggers.push_back(PLinkRegEx(new CLinkRegEx("[a-zA-Z0-9._=-]+@[a-zA-Z](?:[a-zA-Z0-9-]+\\.)+[a-z]+", false, true)));

	// all link continuations must appear *after* non-continuations in the list:

	ms_linkTriggers.push_back(TRGR("^\\s*(/)", true));
	ms_linkTriggers.push_back(TRGR("(\\S+\\.(?:html?|php|aspx?|jpe?g|png|gif)\\S*)", true));
	ms_linkTriggers.push_back(TRGR("(\\S+/dp/\\S*)", true)); // for amazon
	ms_linkTriggers.push_back(TRGR("(\\S*dp/[A-Z]\\S+)", true)); // for amazon
	ms_linkTriggers.push_back(TRGR("(\\S+[&?]\\w+=\\S*)", true));

	/*if(sPrevLineLink[sPrevLineLink.size() - 1] == '-') originally */
	ms_linkTriggers.push_back(TRGR("(\\S{4,}/\\S*)", true));
	// use at least 4 chars so "4.4/10" in a line following an imdb link does not trigger.
#undef TRGR
}
std::vector<CNFOData::PLinkRegEx> CNFOData::ms_linkTriggers;


#define OVECTOR_SIZE 30 // multiple of 3!
bool CNFOData::FindLink(const std::string& sLine, size_t& uirOffset, size_t& urLinkPos, size_t& urLinkLen,
			  std::string& srUrl, const std::string& sPrevLineLink, bool& brLinkContinued)
{
	size_t uBytePos = (size_t)-1, uByteLen = 0;

	srUrl.clear();

	if(sLine.size() > (uint64_t)std::numeric_limits<int>::max()
		|| uirOffset > (uint64_t)std::numeric_limits<int>::max())
	{
		return false;
	}

	// boring vars for pcre_compile:
	const char *szErrDescr = NULL;
	int iErrOffset;
	int ovector[OVECTOR_SIZE];

	PopulateLinkTriggers();

	// find link starting point:
	bool bMatchContinuesLink = false;
	bool bMailto = false;
	for(vector<PLinkRegEx>::const_iterator it = ms_linkTriggers.begin(); it != ms_linkTriggers.end(); it++)
	{
		if(sPrevLineLink.empty() && (*it)->IsCont())
		{
			continue;
		}

		if(pcre_exec((*it)->GetRE(), NULL, sLine.c_str(), (int)sLine.size(), (int)uirOffset, 0, ovector, OVECTOR_SIZE) >= 0)
		{
			int iCaptures = 0;
			if(pcre_fullinfo((*it)->GetRE(), NULL, PCRE_INFO_CAPTURECOUNT, &iCaptures) == 0)
			{
				int idx = (iCaptures == 1 ? 1 : 0) * 2;
				_ASSERT(ovector[idx] >= 0 && ovector[idx + 1] > 0);

				// never match continuations when an actual link start or earlier continuation has been found:
				if(uBytePos != (size_t)-1 && (*it)->IsCont())
				{
					break;
				}

				// find the earliest link start:
				if((size_t)ovector[idx] < uBytePos)
				{
					uBytePos = (size_t)ovector[idx];

					bMatchContinuesLink = (*it)->IsCont();
					bMailto = (*it)->IsMailto();

					if(bMailto)
					{
						uByteLen = (size_t)ovector[idx + 1] - uBytePos;
					}

					if((*it)->IsCont()) // purely an optimization
					{
						break;
					}
				}
			}
		}
	}

	if(uBytePos == (size_t)-1)
	{
		// no link found:
		return false;
	}

	// get the full link:
	string sWorkUrl;

	if(!bMailto)
	{
		string sLineRemainder = sLine.substr(uBytePos);

		if(sLineRemainder.find("hxxp://") == 0 || sLineRemainder.find("h**p://") == 0)
		{
			sLineRemainder.replace(0, 7, "http://");
		}
		else if(sLineRemainder.find("hxxps://") == 0 || sLineRemainder.find("h**ps://") == 0)
		{
			sLineRemainder.replace(0, 8, "https://");
		}

#define REMPAT "^[\\w,/.!#:%;?&=~+-]"

		pcre* reUrlRemainder = pcre_compile(bMatchContinuesLink ? REMPAT "{4,}" : REMPAT "{9,}",
			PCRE_UTF8 | PCRE_NO_UTF8_CHECK | PCRE_UCP, &szErrDescr, &iErrOffset, NULL);

#undef REMPAT

		if(pcre_exec(reUrlRemainder, NULL, sLineRemainder.c_str(), (int)sLineRemainder.size(), 0, 0, ovector, OVECTOR_SIZE) >= 0)
		{
			_ASSERT(ovector[0] == 0);
			uByteLen = (size_t)ovector[1] - (size_t)ovector[0];

			sWorkUrl = sLineRemainder.substr((size_t)ovector[0], uByteLen);

			// strip trailing dots and colons. gross code.
			while(sWorkUrl.size() && (sWorkUrl[sWorkUrl.size() - 1] == '.' || sWorkUrl[sWorkUrl.size() - 1] == ':'))
			{
				sWorkUrl.erase(sWorkUrl.size() - 1);
			}
		}

		pcre_free(reUrlRemainder);
	}
	else
	{
		// uByteLen has been set above.
		sWorkUrl = sLine.substr(uBytePos, uByteLen);
	}

	if(!sWorkUrl.empty())
	{
		urLinkPos = utf8_strlen(sLine.c_str(), uBytePos);
		urLinkLen = utf8_strlen(sWorkUrl.c_str(), -1); // IN CHARACTERS, NOT BYTES!

		uirOffset = uBytePos + uByteLen;

		if(bMailto)
		{
			// early exit for mailto links:

			srUrl = "mailto:" + sWorkUrl;
			brLinkContinued = false;

			return true;
		}

		if(sWorkUrl.find("http://http://") == 0)
		{
			// sigh...
			sWorkUrl.erase(0, 7);
		}

		if(!sPrevLineLink.empty() && bMatchContinuesLink)
		{
			string sPrevLineLinkCopy(sPrevLineLink);
			if(sPrevLineLink[sPrevLineLink.size() - 1] != '.')
			{
				pcre* re = pcre_compile("^(html?|php|aspx?|jpe?g|png|gif)", PCRE_UTF8 | PCRE_NO_UTF8_CHECK, &szErrDescr, &iErrOffset, NULL);
				if(pcre_exec(re, NULL, sWorkUrl.c_str(), (int)sWorkUrl.size(), 0, 0, ovector, OVECTOR_SIZE) >= 0)
				{
					sPrevLineLinkCopy += '.';
				}
				pcre_free(re);
			}
			sWorkUrl = sPrevLineLinkCopy + sWorkUrl;
			brLinkContinued = true;
		}
		else
		{
			brLinkContinued = false;
		}

		pcre* reProtocol = pcre_compile("^(http|ftp)s?://",
			PCRE_CASELESS | PCRE_UTF8 | PCRE_NO_UTF8_CHECK, &szErrDescr, &iErrOffset, NULL);
		if(pcre_exec(reProtocol, NULL, sWorkUrl.c_str(), (int)sWorkUrl.size(), 0, 0, ovector, OVECTOR_SIZE) < 0)
		{
			sWorkUrl = "http://" + sWorkUrl;
		}
		pcre_free(reProtocol);

		pcre* reValid = pcre_compile("^(http|ftp)s?://([\\w-]+)\\.([\\w.-]+)/?",
			PCRE_CASELESS | PCRE_UTF8 | PCRE_NO_UTF8_CHECK, &szErrDescr, &iErrOffset, NULL);
		if(pcre_exec(reValid, NULL, sWorkUrl.c_str(), (int)sWorkUrl.size(), 0, 0, ovector, OVECTOR_SIZE) >= 0)
		{
			srUrl = sWorkUrl;
		}
		pcre_free(reValid);

		return (!srUrl.empty());
	}

	return false;
}
#undef OVECTOR_SIZE


/************************************************************************/
/* Raw Stripper Code                                                    */
/************************************************************************/

static string _TrimParagraph(const string& a_text)
{
	vector<string> l_lines;

	// split text into lines:
	string::size_type l_pos = a_text.find('\n'), l_prevPos = 0;
	string::size_type l_minWhite = numeric_limits<string::size_type>::max();

	while(l_pos != string::npos)
	{
		const string l_line = a_text.substr(l_prevPos, l_pos - l_prevPos);

		l_lines.push_back(l_line);

		l_prevPos = l_pos + 1;
		l_pos = a_text.find('\n', l_prevPos);
	}

	if(l_prevPos < a_text.size() - 1)
	{
		l_lines.push_back(a_text.substr(l_prevPos));
	}

	// find out the minimum number of leading whitespace characters.
	// all other lines will be reduced to this number.
	for(vector<string>::const_iterator it = l_lines.begin(); it != l_lines.end(); it++)
	{
		string::size_type p = 0;
		while(p < it->size() && (*it)[p] == ' ') p++;

		if(p < l_minWhite)
		{
			l_minWhite = p;
		}
	}

	// kill whitespace and put lines back together:
	string l_result;
	l_result.reserve(a_text.size());

	for(vector<string>::const_iterator it = l_lines.begin(); it != l_lines.end(); it++)
	{
		l_result += (*it).substr(l_minWhite);
		l_result += '\n';
	}

	CUtil::StrTrimRight(l_result, "\n");

	return l_result;
}

string CNFOData::GetStrippedTextUtf8(const wstring& a_text)
{
	string l_text;
	wstring l_tmpw;
	l_text.reserve(a_text.size() / 2);

	for(size_t p = 0; p < a_text.size(); p++)
	{
#if defined(_WIN32) || defined(MACOSX)
		if(iswascii(a_text[p]) || iswalnum(a_text[p]) || iswspace(a_text[p]))
#else
		if(iswalnum(a_text[p]) || iswspace(a_text[p]))
#endif
		{
			if(a_text[p] == L'\n') CUtil::StrTrimRight(l_tmpw, L" ");
			l_tmpw += a_text[p];
		}
		else
		{
			l_tmpw += L' ';
			 // we do this to make it easier to nicely retain paragraphs later on
		}
	}

	// collapse newlines between paragraphs:
	for(size_t p = 0; p < l_tmpw.size(); p++)
	{
		if(l_tmpw[p] == L'\n' && p < l_tmpw.size() - 2 && l_tmpw[p + 1] == L'\n')
		{
			p += 2;
			while(p < l_tmpw.size() && l_tmpw[p] == L'\n') l_tmpw.erase(p, 1);
		}
	}

	l_text = CUtil::FromWideStr(l_tmpw, CP_UTF8);

	l_text = CUtil::RegExReplaceUtf8(l_text, "^[^a-zA-Z0-9]+$", "",
		PCRE_NO_UTF8_CHECK | PCRE_MULTILINE);

	l_text = CUtil::RegExReplaceUtf8(l_text, "^(.)\\1+$", "",
		PCRE_NO_UTF8_CHECK | PCRE_MULTILINE);

	l_text = CUtil::RegExReplaceUtf8(l_text, "^([\\S])\\1+\\s{3,}(.+?)$", "$2",
		PCRE_NO_UTF8_CHECK | PCRE_MULTILINE);

	l_text = CUtil::RegExReplaceUtf8(l_text, "^(.+?)\\s{3,}([\\S])\\2+$", "$1",
		PCRE_NO_UTF8_CHECK | PCRE_MULTILINE);

#if 0
	// this ruins our efforts to keep indention for paragraphs :(
	// ...but it makes other NFOs look A LOT better...
	// :TODO: figure out a smart way.
	l_text = CUtil::RegExReplaceUtf8(l_text, "^[\\\\/:.#_|()\\[\\]*@=+ \\t-]{3,}\\s+", "",
		PCRE_NO_UTF8_CHECK | PCRE_MULTILINE);
#endif

	l_text = CUtil::RegExReplaceUtf8(l_text, "\\s+[\\\\/:.#_|()\\[\\]*@=+ \\t-]{3,}$", "",
		PCRE_NO_UTF8_CHECK | PCRE_MULTILINE);

	l_text = CUtil::RegExReplaceUtf8(l_text, "^\\s*.{1,3}\\s*$", "",
		PCRE_NO_UTF8_CHECK | PCRE_MULTILINE);

	l_text = CUtil::RegExReplaceUtf8(l_text, "\\n{2,}", "\n\n", PCRE_NO_UTF8_CHECK);

	CUtil::StrTrimLeft(l_text, "\n");

	// adjust indention for each paragraph:
	if(true)
	{
		string l_newText;

		string::size_type l_pos = l_text.find("\n\n"), l_prevPos = 0;

		while(l_pos != string::npos)
		{
			const string l_paragraph = l_text.substr(l_prevPos, l_pos - l_prevPos);
			const string l_newPara = _TrimParagraph(l_paragraph);

			l_newText += l_newPara + "\n\n";

			l_prevPos = l_pos + 2;
			l_pos = l_text.find("\n\n", l_prevPos);
		}

		if(l_prevPos < l_text.size())
		{
			const string l_paragraph = l_text.substr(l_prevPos);
			l_newText += _TrimParagraph(l_paragraph);
		}

		l_text = l_newText;
	}

	return l_text;
}


const std::vector<char> CNFOData::GetTextCP437(size_t& ar_charsNotConverted, bool a_compoundWhitespace) const
{
	const std::wstring& l_input = (a_compoundWhitespace ? GetWithBoxedWhitespace() : m_textContent);
	map<wchar_t, char> l_transl;
	vector<char> l_converted;

	for(int j = 0; j < 32; j++)
	{
		l_transl[map_cp437_to_unicode_control_range[j]] = j;
	}

	for(int j = CP437_MAP_LOW; j <= 0xFF; j++)
	{
		l_transl[map_cp437_to_unicode_high_bit[j - CP437_MAP_LOW]] = j;
	}

	ar_charsNotConverted = 0;

	l_converted.resize(l_input.size(), ' ');

	#pragma omp parallel for
	for(int i = 0; i < static_cast<int>(l_input.size()); i++)
	{
		wchar_t wc = l_input[i];
		map<wchar_t, char>::const_iterator it;

		if((wc > 0x1F && wc < CP437_MAP_LOW) || wc == L'\n' || wc == L'\r')
		{
			l_converted[i] = (char)wc;
		}
		else if((it = l_transl.find(wc)) != l_transl.end())
		{
			l_converted[i] = it->second;
		}
		else
		{
			#pragma omp atomic
			ar_charsNotConverted++;
		}
	}

	return l_converted;
}


bool CNFOData::SaveToCP437File(const std::_tstring& a_filePath, size_t& ar_charsNotConverted, bool a_compoundWhitespace)
{
	FILE *fp = OpenFileForWritingWithErrorMessage(a_filePath);

	if(!fp)
	{
		return false;
	}

	const vector<char> l_converted = GetTextCP437(ar_charsNotConverted, a_compoundWhitespace);

	bool l_success = (fwrite(l_converted.data(), 1, l_converted.size(), fp) == l_converted.size());

	fclose(fp);

	return l_success;
}


CNFOData::~CNFOData()
{
	delete m_grid;
	delete m_utf8Grid;
}


/************************************************************************/
/* CNFOHyperLink Implementation                                         */
/************************************************************************/

CNFOHyperLink::CNFOHyperLink(int a_linkID, const wstring& a_href, size_t a_row, size_t a_col, size_t a_len)
{
	m_linkID = a_linkID;
	m_href = a_href;
	m_row = a_row;
	m_colStart = a_col;
	m_colEnd = a_col + a_len - 1;
}


void CNFOHyperLink::SetHref(const wstring& a_href)
{
	m_href = a_href;
}


CNFOHyperLink::~CNFOHyperLink()
{
}
