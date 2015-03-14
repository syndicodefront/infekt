/**
 * Copyright (C) 2010-2014 syndicode
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
#include "ansi_art.h"

using namespace std;


CNFOData::CNFOData() :
	m_grid(NULL),
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
		SetLastError(NDE_UNABLE_TO_OPEN_PHYSICAL,
#ifdef HAVE_BOOST
			FORMAT(L"Unable to open NFO file '%s' (error %d)", a_filePath % errno));
#else
			L"Unable to open NFO file. Please check the file name.");
#endif

		return false;
	}

#ifdef _WIN32
	l_fileBytes = _filelength(_fileno(l_file));

	if(l_fileBytes < 0)
	{
		SetLastError(NDE_FAILED_TO_DETERMINE_SIZE, L"Unable to get NFO file size.");

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
		SetLastError(NDE_FAILED_TO_DETERMINE_SIZE, L"stat() on NFO file failed.");

		fclose(l_file);
		return false;
	}
#endif

	if(l_fileBytes > 1024 * 1024 * 3)
	{
		SetLastError(NDE_SIZE_EXCEEDS_LIMIT, L"NFO file is too large (> 3 MB)");

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
	m_vFileName = _T("");

	if(!l_error)
	{
		m_loaded = LoadFromMemoryInternal(l_buf, l_fileBytes);
	}
	else
	{
		SetLastError(NDE_FERROR, L"An error occured while reading from the NFO file.");

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


bool CNFOData::LoadStripped(const CNFOData& a_source)
{
	if(!a_source.HasData())
	{
		return false;
	}

	ClearLastError();

	m_filePath = a_source.m_filePath;
	m_vFileName = a_source.m_vFileName;

	m_textContent = a_source.GetStrippedText();

	m_loaded = PostProcessLoadedContent();

	return m_loaded;
}


// useful in conjunction with LoadFromMemory...
void CNFOData::SetVirtualFileName(const std::_tstring& a_filePath, const std::_tstring& a_fileName)
{
	m_filePath = a_filePath;
	m_vFileName = a_fileName;
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
	for(auto it = a_lines.cbegin(); it != a_lines.cend(); it++, i++)
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
		for(auto it = a_lines.cbegin(); it != a_lines.cend(); it++, i++)
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
	const int l_maxLen = 90;

	// Please note that this routine is not behaving consistently
	// when it comes to taking into account leading whitespace or not.
	// The results are good however.

	CNFOData::TLineContainer l_newLines;
	size_t lines_processed = 0;

	for(const std::wstring& line : a_lines)
	{
		if(line.size() <= l_maxLen
			// don't touch lines with blockchars:
			|| line.find_first_of(L"\x2580\x2584\x2588\x258C\x2590\x2591\x2592\x2593") != wstring::npos)
		{
			l_newLines.push_back(line);
			continue;
		}

		++lines_processed;

		wstring::size_type l_spaces = line.find_first_not_of(L' ');
		if(l_spaces == wstring::npos)
			l_spaces = 0;

		wstring l_line(line);
		bool l_firstRun = true;

		while(l_line.size() > 0)
		{
			wstring::size_type l_cut = l_line.rfind(' ', l_maxLen);
			if(l_cut == wstring::npos || l_cut < l_spaces || l_cut == 0 || l_line.size() < l_maxLen)
				l_cut = l_maxLen;

			wstring l_new = l_line.substr(0, l_cut);
			if(!l_firstRun)
			{
				CUtil::StrTrimLeft(l_new);

				l_new.insert(0,
					l_spaces // whitespace level of line being split
					+ 2 // some indentation to denote what happened
					, ' ');
			}
			l_newLines.push_back(l_new);
			
			if(l_cut != l_maxLen)
				l_line.erase(0, l_cut + 1);
			else
				l_line.erase(0, l_cut);

			l_firstRun = false;
		}
	}

	if(lines_processed > 0 &&
		lines_processed < a_lines.size() * 0.5) // https://code.google.com/p/infekt/issues/detail?id=91 - preserve some NFOs
	{
		a_newMaxLineLen = 0;

		for(const std::wstring& line : l_newLines)
		{
			a_newMaxLineLen = std::max(line.size(), a_newMaxLineLen);
		}

		a_lines = l_newLines;
	}
}


bool CNFOData::LoadFromMemoryInternal(const unsigned char* a_data, size_t a_dataLen)
{
	bool l_loaded = false;
	size_t l_dataLen = a_dataLen;

	ClearLastError();

	m_isAnsi = false; // modifying this state here (and in ReadSAUCE) is not nice

	if(!ReadSAUCE(a_data, l_dataLen))
	{
		return false;
	}

	switch(m_sourceCharset)
	{
	case NFOC_AUTO:
		l_loaded = TryLoad_UTF8Signature(a_data, l_dataLen);
		if(!l_loaded) l_loaded = TryLoad_UTF16LE(a_data, l_dataLen, EA_TRY);
		if(!l_loaded) l_loaded = TryLoad_UTF16BE(a_data, l_dataLen);
		if(HasFileExtension(_T(".nfo")) || HasFileExtension(_T(".diz")))
		{
			// other files are likely ANSI art, so only try non-BOM-UTF-8 for .nfo and .diz
			if(!l_loaded) l_loaded = TryLoad_UTF8(a_data, l_dataLen, EA_TRY);
		}
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

	if(l_loaded)
	{
		return PostProcessLoadedContent();
	}
	else if(!IsInError())
	{
		SetLastError(NDE_ENCODING_PROBLEM, L"There appears to be a charset/encoding problem.");
	}

	return false;
}


bool CNFOData::PostProcessLoadedContent()
{
	size_t l_maxLineLen;
	TLineContainer l_lines;
	bool l_ansiError = false;

	m_colorMap.reset();

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
		if(m_ansiHintWidth == 0)
		{
			m_ansiHintWidth = 80;
		}

		try
		{
			CAnsiArt l_ansiArtProcessor(WIDTH_LIMIT, LINES_LIMIT, m_ansiHintWidth, m_ansiHintHeight);

			l_ansiError = !l_ansiArtProcessor.Parse(m_textContent) || !l_ansiArtProcessor.Process();

			if(!l_ansiError)
			{
				l_lines = l_ansiArtProcessor.GetLines();
				l_maxLineLen = l_ansiArtProcessor.GetMaxLineLength();
				m_textContent = l_ansiArtProcessor.GetAsClassicText();
				m_colorMap = l_ansiArtProcessor.GetColorMap();
			}
		}
		catch(const std::exception& ex)
		{
			l_ansiError = true;
			(void)ex;
		}
	}

	// copy lines to grid:
	delete m_grid; m_grid = NULL;
	m_utf8Map.clear();
	m_hyperLinks.clear();
	m_utf8Content.clear();

	if(l_ansiError)
	{
		SetLastError(NDE_ANSI_INTERNAL,
			L"Internal problem during ANSI processing. This could be a bug, please file a report and attach the file you were trying to open.");
		return false;
	}

	if(l_lines.size() == 0 || l_maxLineLen == 0)
	{
		SetLastError(NDE_EMPTY_FILE, L"Unable to find any lines in this file.");
		return false;
	}

	if(l_maxLineLen > WIDTH_LIMIT)
	{
		SetLastError(NDE_MAXIMUM_LINE_LENGTH_EXCEEDED,
#ifdef HAVE_BOOST
			FORMAT(L"This file contains a line longer than %d chars. To prevent damage and lock-ups, we do not load it.", WIDTH_LIMIT));
#else
			L"This file contains a line that exceeds the internal maximum length limit.");
#endif
		return false;
	}

	if(l_lines.size() > LINES_LIMIT)
	{
		SetLastError(NDE_MAXIMUM_NUMBER_OF_LINES_EXCEEDED,
#ifdef HAVE_BOOST
			FORMAT(L"This file contains more than %d lines. To prevent damage and lock-ups, we do not load it.", WIDTH_LIMIT));
#else
			L"This file contains more lines than the internal limit.");
#endif
		return false;
	}

	// allocate mem:
	m_grid = new TwoDimVector<wchar_t>(l_lines.size(), l_maxLineLen, 0);

	// vars for hyperlink detection:
	string l_prevLinkUrl; // UTF-8
	int l_maxLinkId = 1;
	std::multimap<size_t, CNFOHyperLink>::iterator l_prevLinkIt = m_hyperLinks.end();

	// go through line by line:
	size_t i = 0; // line (row) index
	for(TLineContainer::const_iterator it = l_lines.cbegin(); it != l_lines.cend(); it++, i++)
	{
		int l_lineLen = static_cast<int>(it->length());

		#pragma omp parallel for
		for(int j = 0; j < l_lineLen; j++)
		{
			(*m_grid)[i][j] = (*it)[j];
		}

		const string l_utf8Line = CUtil::FromWideStr(*it, CP_UTF8);

		const char* const p_start = l_utf8Line.c_str();
		const char* p = p_start;
		size_t char_index = 0;
		while(*p)
		{
			wchar_t w_at = (*m_grid)[i][char_index++];
			const char *p_char = p;
			const char *p_next = utf8_find_next_char(p);

			if(m_utf8Map.find(w_at) == m_utf8Map.end())
			{
				m_utf8Map[w_at] = (p_next != NULL ? std::string(p_char, static_cast<size_t>(p_next - p)) : std::string(p_char));
			}

			p = p_next;
		}

		// find hyperlinks:
		if(/* m_bFindHyperlinks == */true)
		{
			size_t l_linkPos = (size_t)-1, l_linkLen;
			bool l_linkContinued;
			string l_url, l_prevUrlCopy = l_prevLinkUrl;
			size_t l_offset = 0;

			while(CNFOHyperLink::FindLink(l_utf8Line, l_offset, l_linkPos, l_linkLen, l_url, l_prevUrlCopy, l_linkContinued))
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

	return true;
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

	if(TryLoad_UTF8(a_data, a_dataLen, EA_TRY))
	{
		if(m_sourceCharset == NFOC_UTF8)
		{
			m_sourceCharset = NFOC_UTF8_SIG;
		}

		return true;
	}

	return false;
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
			/* 'PLUS-MINUS SIGN' or 'SUPERSCRIPT TWO' */)
			/* following is more detection stuff for double-encoded CP437 NFOs that were converted to UTF-8 */
			|| (a_fix == EA_TRY && (l_utf.find("\xC2\x9A\xC2\x9A") != std::string::npos && l_utf.find("\xC3\xA1\xC3\xA1") != std::string::npos))
		)
		{
			std::vector<char> l_cp437(a_dataLen + 1);
			size_t l_newLength = utf8_to_latin9(l_cp437.data(), (const char*)a_data, a_dataLen);

			if(l_newLength > 0 && TryLoad_CP437((unsigned char*)l_cp437.data(), l_newLength, EA_TRY))
			{
				m_sourceCharset = (m_sourceCharset == NFOC_CP437_IN_CP437 ? NFOC_CP437_IN_CP437_IN_UTF8 : NFOC_CP437_IN_UTF8);

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

	bool l_containsCR = false;
	bool l_containsLF = false;

	for(int i = 0; i < static_cast<int>(a_dataLen) && (!l_containsCR || !l_containsLF); ++i)
	{
		if (a_data[i] == '\r')
			l_containsCR = true;
		else if (a_data[i] == '\n')
			l_containsLF = true;
	}

	bool l_foundBinary = false;

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
				// "allow" \0 chars for ANSI files with SAUCE record...
				l_foundBinary = true;
				m_textContent[i] = L'?';
			}
			else if(p == 0x0D && i < static_cast<int>(a_dataLen) - 1 && a_data[i + 1] == 0x0A)
			{
				m_textContent[i] = L'\r';
			}
			else if(p == 0x0D && i < static_cast<int>(a_dataLen) - 2 && a_data[i + 1] == 0x0D && a_data[i + 2] == 0x0A)
			{
				// https://code.google.com/p/infekt/issues/detail?id=92
				// http://stackoverflow.com/questions/6998506/text-file-with-0d-0d-0a-line-breaks
				m_textContent[i] = L' ';
			}
			else if (p == 0x0D && !l_containsLF && l_containsCR)
			{
				// http://code.google.com/p/infekt/issues/detail?id=103
				m_textContent[i] = L'\n';
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

	bool l_ansi = m_isAnsi || DetectAnsi();

	if(l_foundBinary && !l_ansi)
	{
		SetLastError(NDE_UNRECOGNIZED_FILE_FORMAT, L"Unrecognized file format or broken file.");

		m_textContent.clear();

		return false;
	}
	else
	{
		m_sourceCharset = (a_fix == EA_FORCE ? NFOC_CP437_IN_CP437 : NFOC_CP437);

		m_isAnsi = l_ansi;

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
		SetLastError(NDE_UNRECOGNIZED_FILE_FORMAT, L"Unrecognized file format or broken file.");

		m_textContent.clear();

		return false;
	}
	else
	{
		m_sourceCharset = NFOC_CP437_STRICT;

		m_isAnsi = m_isAnsi || DetectAnsi();

		return true;
	}
}


bool CNFOData::HasFileExtension(const TCHAR* a_extension) const
{
	const std::_tstring l_extension = (m_filePath.length() > 4 ? m_filePath.substr(m_filePath.length() - 4) : _T(""));

	return _tcsicmp(l_extension.c_str(), a_extension) == 0;
}


bool CNFOData::DetectAnsi() const
{
	// try to detect ANSI art files without SAUCE records:

	if(!m_isAnsi && HasFileExtension(_T(".ans")) && m_textContent.find(L"\u2190[") != std::wstring::npos)
	{
		return true;
	}

	if(!m_isAnsi && !HasFileExtension(_T(".nfo")) && m_textContent.find(L"\u2190[") != std::wstring::npos)
	{
#ifdef INFEKT_REGEX_UTF16
		return CRegExUtil::DoesMatch(m_textContent, L"\u2190\\[[0-9;]+m");
#else
		return CRegExUtil::DoesMatch(CUtil::FromWideStr(m_textContent, CP_UTF8), "\xE2\x86\x90\\[[0-9;]+m");
#endif
	}

	return false;
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
		SetLastError(NDE_UNRECOGNIZED_FILE_FORMAT, L"Unrecognized file format or broken file.");

		m_textContent.clear();

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
			SetLastError(NDE_UNRECOGNIZED_FILE_FORMAT, L"Unrecognized file format or broken file.");
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

	memcpy(&l_record, a_data + ar_dataLen - SAUCE_RECORD_SIZE, SAUCE_RECORD_SIZE);

	// validate SAUCE header + supported features:

	if(memcmp(l_record.ID, "SAUCE", 5) != 0)
	{
		// no SAUCE record, no error.
		return true;
	}

	if(memcmp(l_record.Version, "00", 2) != 0)
	{
		SetLastError(NDE_SAUCE_INTERNAL, L"SAUCE: Unsupported file version.");

		return false;
	}

	if(l_record.DataType != SAUCEDT_CHARACTER || (l_record.FileType != SAUCEFT_CHAR_ANSI && l_record.FileType != SAUCEFT_CHAR_ASCII))
	{
		m_lastErrorDescr = L"SAUCE: Unsupported file format type.";

		return false;
	}

	// skip record + comments:

	size_t l_bytesToTrim = SAUCE_RECORD_SIZE + (l_record.Comments > 0
		? l_record.Comments * SAUCE_COMMENT_LINE_SIZE + SAUCE_HEADER_ID_SIZE : 0);

	if(l_record.Comments > SAUCE_MAX_COMMENTS || ar_dataLen < l_bytesToTrim)
	{
		SetLastError(NDE_SAUCE_INTERNAL, L"SAUCE: Bad comments definition.");

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


const std::_tstring CNFOData::GetFileName() const
{
	if(!m_vFileName.empty())
	{
		return m_vFileName;
	}

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
		SetLastError(NDE_UNABLE_TO_OPEN_PHYSICAL,
#ifdef HAVE_BOOST
			FORMAT(L"Unable to open file '%s' for writing (error %d)", a_filePath % errno));
#else
			L"Unable to open file for writing. Please check the file name.");
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
		unsigned char l_signature[3] = { 0xEF, 0xBB, 0xBF };
		l_written += fwrite(l_signature, 1, sizeof(l_signature), l_file);

		const std::string& l_contents = (a_compoundWhitespace
			? CUtil::FromWideStr(GetWithBoxedWhitespace(), CP_UTF8)
			: GetTextUtf8());

		l_written += fwrite(l_contents.c_str(), l_contents.size(), 1, l_file);

		l_success = (l_written == 4);
	}
	else
	{
		unsigned char l_bom[2] = { 0xFF, 0xFE };
		l_written += fwrite(l_bom, 1, sizeof(l_bom), l_file);

		const std::wstring& l_contents = (a_compoundWhitespace
			? GetWithBoxedWhitespace()
			: m_textContent);

		l_written += fwrite(l_contents.c_str(), l_contents.size(), sizeof(wchar_t), l_file);

		l_success = (l_written == 4);
	}

	fclose(l_file);

	return l_success;
}


const std::string& CNFOData::GetTextUtf8()
{
	if(m_utf8Content.empty())
	{
		m_utf8Content = CUtil::FromWideStr(m_textContent, CP_UTF8);
	}

	return m_utf8Content;
}


size_t CNFOData::GetGridWidth() const
{
	return (m_grid ? m_grid->GetCols() : -1);
}


size_t CNFOData::GetGridHeight() const
{
	return (m_grid ? m_grid->GetRows() : -1);
}


wchar_t CNFOData::GetGridChar(size_t a_row, size_t a_col) const
{
	return (m_grid
		&& a_row < m_grid->GetRows()
		&& a_col < m_grid->GetCols()
		? (*m_grid)[a_row][a_col]
		: 0);
}


const std::string CNFOData::GetGridCharUtf8(size_t a_row, size_t a_col) const
{
	wchar_t grid_char = GetGridChar(a_row, a_col);

	return (grid_char > 0 ? GetGridCharUtf8(grid_char) : "");
}


const std::string CNFOData::GetGridCharUtf8(wchar_t a_wideChar) const
{
	auto it = m_utf8Map.find(a_wideChar);

	return (it != m_utf8Map.end() ? it->second : "");
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
		return L"CP 437 (in UTF-8)";
	case NFOC_CP437_IN_UTF16:
		return L"CP 437 (in UTF-16)";
	case NFOC_CP437_IN_CP437:
		return L"CP 437 (double encoded)";
	case NFOC_CP437_IN_CP437_IN_UTF8:
		return L"CP 437 (double encoded + UTF-8)";
	case NFOC_CP437_STRICT:
		return L"CP 437 (strict mode)";
	case NFOC_WINDOWS_1252:
		return L"Windows-1252";
	}

	return L"(huh?)";
}


const std::wstring CNFOData::GetCharsetName() const
{
	if(m_isAnsi)
		return GetCharsetName(m_sourceCharset) + L" (ANSI ART)";
	else
		return GetCharsetName(m_sourceCharset);
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
	const auto l_range = m_hyperLinks.equal_range(a_row);

	for (auto it = l_range.first; it != l_range.second; it++)
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
		multimap<size_t, CNFOHyperLink>::const_iterator it = m_hyperLinks.cbegin();
		for(size_t i = 0; i < a_index; i++, it++) ;
		return &it->second;
	}

	return NULL;
}


const vector<const CNFOHyperLink*> CNFOData::GetLinksForLine(size_t a_row) const
{
	vector<const CNFOHyperLink*> l_result;

	const auto l_range = m_hyperLinks.equal_range(a_row);

	for (auto it = l_range.first; it != l_range.second; it++)
	{
		l_result.push_back(&it->second);
	}

	return l_result;
}


/************************************************************************/
/* Raw Stripper Code                                                    */
/************************************************************************/

static wstring _TrimParagraph(const wstring& a_text)
{
	vector<wstring> l_lines;

	// split text into lines:
	wstring::size_type l_pos = a_text.find(L'\n'), l_prevPos = 0;
	wstring::size_type l_minWhite = numeric_limits<wstring::size_type>::max();

	while(l_pos != wstring::npos)
	{
		const wstring l_line = a_text.substr(l_prevPos, l_pos - l_prevPos);

		l_lines.push_back(l_line);

		l_prevPos = l_pos + 1;
		l_pos = a_text.find(L'\n', l_prevPos);
	}

	if(l_prevPos < a_text.size() - 1)
	{
		l_lines.push_back(a_text.substr(l_prevPos));
	}

	// find out the minimum number of leading whitespace characters.
	// all other lines will be reduced to this number.
	for(auto it = l_lines.cbegin(); it != l_lines.cend(); it++)
	{
		wstring::size_type p = 0;
		while(p < it->size() && (*it)[p] == L' ') p++;

		if(p < l_minWhite)
		{
			l_minWhite = p;
		}
	}

	// kill whitespace and put lines back together:
	wstring l_result;
	l_result.reserve(a_text.size());

	for(auto it = l_lines.cbegin(); it != l_lines.cend(); it++)
	{
		l_result += (*it).substr(l_minWhite);
		l_result += L'\n';
	}

	CUtil::StrTrimRight(l_result, L"\n");

	return l_result;
}


wstring CNFOData::GetStrippedText() const
{
	wstring l_text;
	l_text.reserve(m_textContent.size() / 2);

	for(size_t p = 0; p < m_textContent.size(); p++)
	{
#if defined(_WIN32) || defined(MACOSX)
		if(iswascii(m_textContent[p]) || iswalnum(m_textContent[p]) || iswspace(m_textContent[p]))
#else
		if(iswalnum(m_textContent[p]) || iswspace(m_textContent[p]))
#endif
		{
			if(m_textContent[p] == L'\n')
			{
				CUtil::StrTrimRight(l_text, L" ");
			}

			l_text += m_textContent[p];
		}
		else
		{
			l_text += L' ';
			 // we do this to make it easier to nicely retain paragraphs later on
		}
	}

	// collapse newlines between paragraphs:
	for(size_t p = 0; p < l_text.size(); p++)
	{
		if(l_text[p] == L'\n' && p < l_text.size() - 2 && l_text[p + 1] == L'\n')
		{
			p += 2;

			while(p < l_text.size() && l_text[p] == L'\n')
			{
				l_text.erase(p, 1);
			}
		}
	}

#ifndef INFEKT_REGEX_UTF16
	std::string l_textUtf8 = CUtil::FromWideStr(l_text, CP_UTF8);
	#define l_text l_textUtf8
#endif

	l_text = CRegExUtil::Replace(l_text, _RESTR("^[^a-zA-Z0-9]+$"), _RESTR(""), PCRE_MULTILINE);

	l_text = CRegExUtil::Replace(l_text, _RESTR("^(.)\\1+$"), _RESTR(""),
		INFEKT_PCRE_NO_UTF_CHECK | PCRE_MULTILINE);

	l_text = CRegExUtil::Replace(l_text, _RESTR("^([\\S])\\1+\\s{3,}(.+?)$"), _RESTR("$2"),
		INFEKT_PCRE_NO_UTF_CHECK | PCRE_MULTILINE);

	l_text = CRegExUtil::Replace(l_text, _RESTR("^(.+?)\\s{3,}([\\S])\\2+$"), _RESTR("$1"),
		INFEKT_PCRE_NO_UTF_CHECK | PCRE_MULTILINE);

#if 0
	// this ruins our efforts to keep indention for paragraphs :(
	// ...but it makes other NFOs look A LOT better...
	// :TODO: figure out a smart way.
	l_text = CRegExUtil::Replace(l_text, _RESTR("^[\\\\/:.#_|()\\[\\]*@=+ \\t-]{3,}\\s+"), _RESTR(""),
		INFEKT_PCRE_NO_UTF_CHECK | PCRE_MULTILINE);
#endif

	l_text = CRegExUtil::Replace(l_text, _RESTR("\\s+[\\\\/:.#_|()\\[\\]*@=+ \\t-]{3,}$"), _RESTR(""),
		INFEKT_PCRE_NO_UTF_CHECK | PCRE_MULTILINE);

	l_text = CRegExUtil::Replace(l_text, _RESTR("^\\s*.{1,3}\\s*$"), _RESTR(""),
		INFEKT_PCRE_NO_UTF_CHECK | PCRE_MULTILINE);

	l_text = CRegExUtil::Replace(l_text, _RESTR("\\n{2,}"), _RESTR("\n\n"), INFEKT_PCRE_NO_UTF_CHECK);

#ifndef INFEKT_REGEX_UTF16
	#undef l_text
	l_text = CUtil::ToWideStr(l_textUtf8, CP_UTF8);
#endif

	CUtil::StrTrimLeft(l_text, L"\n");

	// adjust indention for each paragraph:
	if(true)
	{
		wstring l_newText;

		wstring::size_type l_pos = l_text.find(L"\n\n"), l_prevPos = 0;

		while(l_pos != wstring::npos)
		{
			const wstring l_paragraph = l_text.substr(l_prevPos, l_pos - l_prevPos);
			const wstring l_newPara = _TrimParagraph(l_paragraph);

			l_newText += l_newPara + L"\n\n";

			l_prevPos = l_pos + 2;
			l_pos = l_text.find(L"\n\n", l_prevPos);
		}

		if(l_prevPos < l_text.size())
		{
			const wstring l_paragraph = l_text.substr(l_prevPos);
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


void CNFOData::SetLastError(EErrorCode a_code, const std::wstring& a_descr)
{
	m_lastErrorCode = a_code;
	m_lastErrorDescr = a_descr;
}


void CNFOData::ClearLastError()
{
	m_lastErrorCode = NDE_NO_ERROR;
	m_lastErrorDescr = L"";
}


CNFOData::~CNFOData()
{
	delete m_grid;
}
