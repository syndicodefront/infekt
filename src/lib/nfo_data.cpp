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

#include "stdafx.h"
#include "nfo_data.h"
#include "util.h"

using namespace std;


CNFOData::CNFOData()
{
	m_grid = NULL;
	m_loaded = false;
	m_utf8Grid = NULL;
	m_sourceCharset = NFOC_AUTO;
}


bool CNFOData::LoadFromFile(const _tstring& a_filePath)
{
	FILE *l_file = NULL;
	size_t l_fileBytes;

#ifdef _WIN32
	if(_tfopen_s(&l_file, a_filePath.c_str(), _T("rb")) != 0 || !l_file)
#else
	if(l_file = fopen(a_filePath.c_str(), "rb"))
#endif
	{
#ifdef HAVE_BOOST
		m_lastErrorDescr = FORMAT(L"Unable to open NFO file '%s' (error %d)", a_filePath % errno);
#else
		m_lastErrorDescr = L"Unable to open NFO file. Please check the file name.";
#endif

		return false;
	}

	l_fileBytes = _filelength(_fileno(l_file));

	if(l_fileBytes < 0)
	{
		m_lastErrorDescr = L"Unable to get NFO file size.";

		fclose(l_file);
		return false;
	}

	if(l_fileBytes > 1024 * 1024 * 10)
	{
		m_lastErrorDescr = L"NFO file is too large (> 10 MB)";

		fclose(l_file);
		return false;
	}

	// we add a trailing \0.
	unsigned char* l_buf = new unsigned char[l_fileBytes + 1];
	memset(l_buf, 0, l_fileBytes + 1);

	// copy file contents into memory buffer:
	unsigned char *l_ptr = l_buf;
	size_t l_totalBytesRead = 0;

	while(!feof(l_file))
	{
		unsigned char l_chunkBuf[4096];
		size_t l_bytesRead;

		l_bytesRead = fread_s(&l_chunkBuf, sizeof(l_chunkBuf), sizeof(unsigned char), 4096, l_file);
		if(l_bytesRead > 0)
		{
			l_totalBytesRead += l_bytesRead;

			memmove_s(l_ptr, l_buf + l_fileBytes - l_ptr, l_chunkBuf, l_bytesRead);

			l_ptr += l_bytesRead;
		}
		else if(ferror(l_file))
		{
			break;
		}
	}

	bool l_loaded = false;

	if(!ferror(l_file))
	{
		l_loaded = LoadFromMemoryInternal(l_buf, l_fileBytes);
	}
	else
	{
		m_lastErrorDescr = L"An error occured while reading from the NFO file.";
	}

	delete[] l_buf;

	fclose(l_file);

	m_loaded = l_loaded;

	if(l_loaded)
	{
		m_filePath = a_filePath;
	}

	return l_loaded;
}


bool CNFOData::LoadFromMemory(const unsigned char* a_data, size_t a_dataLen)
{
	m_loaded = LoadFromMemoryInternal(a_data, a_dataLen);
	return m_loaded;
}


bool CNFOData::LoadFromMemoryInternal(const unsigned char* a_data, size_t a_dataLen)
{
	bool l_loaded = false;

	m_loaded = false;

	switch(m_sourceCharset)
	{
	case NFOC_AUTO:
		l_loaded = TryLoad_UTF8Signature(a_data, a_dataLen);
		if(!l_loaded) l_loaded = TryLoad_UTF16LE(a_data, a_dataLen);
		if(!l_loaded) l_loaded = TryLoad_UTF16BE(a_data, a_dataLen);
		if(!l_loaded) l_loaded = TryLoad_UTF8(a_data, a_dataLen);
		if(!l_loaded) l_loaded = TryLoad_CP437(a_data, a_dataLen);
		break;

	case NFOC_UTF16:
		l_loaded = TryLoad_UTF16LE(a_data, a_dataLen);
		if(!l_loaded) l_loaded = TryLoad_UTF16BE(a_data, a_dataLen);
		break;
	case NFOC_UTF8_SIG:
		l_loaded = TryLoad_UTF8Signature(a_data, a_dataLen);
		break;
	case NFOC_UTF8:
		l_loaded = TryLoad_UTF8(a_data, a_dataLen);
		break;
	case NFOC_CP437:
		l_loaded = TryLoad_CP437(a_data, a_dataLen);
		break;
	}

	if(l_loaded)
	{
		m_filePath = _T("");

		// normalize new lines... blergh:
		for(size_t p = 0; p < m_textContent.size(); p++)
		{
			if(m_textContent[p] == L'\r')
			{
				if(p < m_textContent.size() - 1 && m_textContent[p + 1] == '\n')
				{
					m_textContent.erase(p, 1);
				}
				m_textContent[p] = L'\n';
			}
		}
		// we should only have \ns now.

		CUtil::StrTrimRight(m_textContent);
		m_textContent += L'\n';

		// split raw contents into grid buffer.

		size_t l_maxLineLen = 1;
		size_t l_prevPos = 0, l_pos = m_textContent.find(L'\n');
		deque<const wstring> l_lines;

		// read lines:
		while(l_pos != wstring::npos)
		{
			wstring l_line = m_textContent.substr(l_prevPos, l_pos - l_prevPos);

			// trim trailing whitespace:
			CUtil::StrTrimRight(l_line);

			l_lines.push_back(l_line);

			if(l_line.length() > l_maxLineLen)
			{
				l_maxLineLen = l_line.length();
			}

			l_prevPos = l_pos + 1;
			l_pos = m_textContent.find(L'\n', l_prevPos);
		}

		// :TODO: interpret ANSI escape codes.

		// copy lines to grid(s):
		delete m_grid; m_grid = NULL;
		delete m_utf8Grid; m_utf8Grid = NULL;
		m_hyperLinks.clear();
		m_utf8Content.clear();

		if(l_lines.size() == 0 || l_maxLineLen == 0)
		{
			m_lastErrorDescr = L"Unable to find any lines in this file.";
			return false;
		}

		m_utf8Content.reserve(m_textContent.length());

		// allocate mem:
		m_grid = new TwoDimVector<wchar_t>(l_lines.size(), l_maxLineLen + 1, 0);
		m_utf8Grid = new char[l_lines.size() * m_grid->GetCols() * 7];
		memset(m_utf8Grid, 0, l_lines.size() * m_grid->GetCols() * 7);

		// vars for hyperlink detection:
		string l_prevLinkUrl; // UTF-8
		int l_maxLinkIndex = 0, l_maxLinkId = 1;

		// go through line by line:
		size_t i = 0;
		for(deque<const wstring>::const_iterator it = l_lines.begin();
			it != l_lines.end(); it++, i++)
		{
			size_t l_lineLen = it->length();

			for(size_t j = 0; j < l_lineLen; j++)
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
				size_t l_linkPos, l_linkLen;
				bool l_linkContinued;
				string l_url;

				if(FindLink(l_utf8Line, l_linkPos, l_linkLen, l_url, l_prevLinkUrl, l_linkContinued))
				{
					const wstring wsUrl = CUtil::ToWideStr(l_url, CP_UTF8);
					int l_linkID = (l_linkContinued ? l_maxLinkId - 1 : l_maxLinkId);

					m_hyperLinks.push_back(CNFOHyperLink(l_linkID, wsUrl, i, l_linkPos, l_linkLen));

					if(!l_linkContinued)
					{
						l_maxLinkId++;
						l_prevLinkUrl = l_url;
					}
					else
					{
						m_hyperLinks[l_maxLinkIndex - 1].SetHref(wsUrl);
						l_prevLinkUrl = "";
					}

					l_maxLinkIndex++;
				}
			}
		}


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

	m_textContent = CUtil::ToWideStr(
		string().append((char*)(a_data + 3), a_dataLen - 3), CP_UTF8);

	m_sourceCharset = NFOC_UTF8_SIG;

	return true;
}


bool CNFOData::TryLoad_UTF8(const unsigned char* a_data, size_t a_dataLen)
{
	if(g_utf8_validate((const char*)a_data, a_dataLen, NULL))
	{
		m_textContent = CUtil::ToWideStr(
			string().append((const char*)a_data, a_dataLen), CP_UTF8);

		m_sourceCharset = NFOC_UTF8;

		return true;
	}

	return false;
}

/* based on http://en.wikipedia.org/wiki/Code_page_437 */
#include "nfo_data_cp437.inc"

bool CNFOData::TryLoad_CP437(const unsigned char* a_data, size_t a_dataLen)
{
	m_textContent.clear();
	m_textContent.reserve(a_dataLen);

	for(size_t i = 0; i < a_dataLen; i++)
	{
		unsigned char p = a_data[i];

		if(p == 0x7F)
		{
			// Code 127 (7F), DEL, shows as a graphic (a house).
			m_textContent += (wchar_t)0x2302;
		}
		else if(p >= 0x80)
		{
			m_textContent += map_cp437_to_unicode_high_bit[p - 0x80];
		}
		else if(p <= 0x1F)
		{
			if(p == 0x0D && i < a_dataLen - 1 && a_data[i + 1] == 0x0A)
			{
				m_textContent += L'\r';
			}
			else
			{
				m_textContent += map_cp437_to_unicode_control_range[p];
			}
		}
		else
		{
			_ASSERT(p > 0x1F && p < 0x80);

			m_textContent += (wchar_t)p;
		}
	}

	m_sourceCharset = NFOC_CP437;

	return true;
}


bool CNFOData::TryLoad_UTF16LE(const unsigned char* a_data, size_t a_dataLen)
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

	return true;
}


bool CNFOData::TryLoad_UTF16BE(const unsigned char* a_data, size_t a_dataLen)
{
#if !defined(_WIN32)
	return false;
#else
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
	}

	m_textContent = wstring().append(l_newBuf, l_numWChars);

	return true;
#endif
}


const wstring& CNFOData::GetLastErrorDescription()
{
	return m_lastErrorDescr;
}


const std::_tstring CNFOData::GetFileName()
{
#ifdef _WIN32
	const TCHAR* l_name = PathFindFileName(m_filePath.c_str());
	return l_name;
#else
	return "/not_implemented/"; // :TODO:
#endif
}


bool CNFOData::SaveToFile(std::_tstring a_filePath, bool a_utf8)
{
	FILE *l_file = NULL;

#ifdef _WIN32
	if(_tfopen_s(&l_file, a_filePath.c_str(), _T("wb")) != 0 || !l_file)
#else
	if(l_file = fopen(a_filePath.c_str(), "wb"))
#endif
	{
#ifdef HAVE_BOOST
		m_lastErrorDescr = FORMAT(L"Unable to open file '%s' for writing (error %d)", a_filePath % errno);
#else
		m_lastErrorDescr = L"Unable to open file for writing. Please check the file name.";
#endif

		return false;
	}

	size_t l_written = 0;
	bool l_success;

	if(a_utf8)
	{
		// write signature
		unsigned char l_sig[3] = { 0xEF, 0xBB, 0xBF };
		l_written += fwrite(l_sig, 1, sizeof(l_sig), l_file);

		// dump
		l_written += fwrite(m_utf8Content.c_str(), m_utf8Content.size(), 1, l_file);

		l_success = (l_written == 4);
	}
	else
	{
		// write BOM
		unsigned char l_bom[2] = { 0xFF, 0xFE };
		l_written += fwrite(l_bom, 1, sizeof(l_bom), l_file);

		// dump
		l_written += fwrite(m_textContent.c_str(), m_textContent.size(), sizeof(wchar_t), l_file);

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


/************************************************************************/
/* Hyper Link Code                                                      */
/************************************************************************/

const CNFOHyperLink* CNFOData::GetLink(size_t a_row, size_t a_col) const
{
	for(deque<CNFOHyperLink>::const_iterator it = m_hyperLinks.begin(); it != m_hyperLinks.end(); it++)
	{
		if(it->GetRow() == a_row && (a_col >= it->GetColStart() && a_col <= it->GetColEnd()))
		{
			return &(*it);
		}
	}

	return NULL;
}


#define OVECTOR_SIZE 30 // multiple of 3!
bool CNFOData::FindLink(const std::string& sLine, size_t& urLinkPos, size_t& urLinkLen,
			  std::string& srUrl, const std::string& sPrevLineLink, bool& brLinkContinued)
{
	typedef pair<const char*, bool> TRGR;
	vector<TRGR> mTriggers; // trigger pattern -> is_continuation_trigger
	size_t uBytePos = (size_t)-1, uByteLen = 0;

	srUrl.clear();

	mTriggers.push_back(TRGR("http://", false));
	mTriggers.push_back(TRGR("https://", false));
	mTriggers.push_back(TRGR("www\\.", false));
	mTriggers.push_back(TRGR("german\\.imdb\\.com", false));
	mTriggers.push_back(TRGR("imdb\\.com", false));
	mTriggers.push_back(TRGR("ofdb\\.de", false));
	mTriggers.push_back(TRGR("imdb\\.de", false));
	mTriggers.push_back(TRGR("cinefacts\\.de", false));
	mTriggers.push_back(TRGR("zelluloid\\.de", false));
	mTriggers.push_back(TRGR("tinyurl\\.com", false));
	mTriggers.push_back(TRGR("bit\\.ly", false));

	if(!sPrevLineLink.empty())
	{
		mTriggers.push_back(TRGR("^\\s*(/)", true));
		mTriggers.push_back(TRGR("(\\S+\\.(?:html?|php|aspx?)\\S*)", true));
		mTriggers.push_back(TRGR("(\\S+/dp/\\S*)", true)); // for amazon
		mTriggers.push_back(TRGR("(\\S*dp/[A-Z]\\S+)", true)); // for amazon
		mTriggers.push_back(TRGR("(\\S+[&?]\\w+=\\S*)", true));

		if(sPrevLineLink[sPrevLineLink.size() - 1] == '-')
		{
			mTriggers.push_back(TRGR("(\\S+/\\S*)", true));
		}
	}

	if(sLine.size() > (int64_t)std::numeric_limits<int>::max())
	{
		return false;
	}

	// boring vars for pcre_compile:
	const char *szErrDescr;
	int iErrOffset;
	int ovector[OVECTOR_SIZE];

	// find link starting point:
	bool bMatchContinuesLink = false;
	for(vector<TRGR>::const_iterator it = mTriggers.begin(); it != mTriggers.end(); it++)
	{
		pcre* re = pcre_compile(it->first,
			PCRE_CASELESS | PCRE_UTF8 | PCRE_NO_UTF8_CHECK,
			&szErrDescr, &iErrOffset, NULL);

		if(pcre_exec(re, NULL, sLine.c_str(), (int)sLine.size(), 0, 0, ovector, OVECTOR_SIZE) >= 0)
		{
			int iCaptures = 0;
			if(pcre_fullinfo(re, NULL, PCRE_INFO_CAPTURECOUNT, &iCaptures) == 0)
			{
				int idx = (iCaptures == 1 ? 1 : 0) * 2;
				_ASSERT(ovector[idx] >= 0 && ovector[idx + 1] > 0);
				uBytePos = (size_t)ovector[idx];
			}
		}

		pcre_free(re);

		if(uBytePos != (size_t)-1)
		{
			bMatchContinuesLink = it->second;
			break;
		}
	}

	if(uBytePos == (size_t)-1)
	{
		return false;
	}

	// get the rest of the link:
	const string sLineRemainder = sLine.substr(uBytePos);

	pcre* reUrlRemainder = pcre_compile("^[a-z0-9,/._!:%;?&=+-]{9,}",
		PCRE_CASELESS | PCRE_UTF8 | PCRE_NO_UTF8_CHECK, &szErrDescr, &iErrOffset, NULL);

	if(pcre_exec(reUrlRemainder, NULL, sLineRemainder.c_str(), (int)sLineRemainder.size(), 0, 0, ovector, OVECTOR_SIZE) >= 0)
	{
		_ASSERT(ovector[0] == 0);
		uByteLen = (size_t)ovector[1] - (size_t)ovector[0];

		string sWorkUrl = sLineRemainder.substr((size_t)ovector[0], uByteLen);

		// strip trailing dots and colons. gross code.
		while(sWorkUrl.size() && (sWorkUrl[sWorkUrl.size() - 1] == '.' || sWorkUrl[sWorkUrl.size() - 1] == ':')) sWorkUrl.erase(sWorkUrl.size() - 1);

		urLinkPos = g_utf8_strlen(sLine.c_str(), uBytePos);
		urLinkLen = g_utf8_strlen(sWorkUrl.c_str(), -1); // IN CHARACTERS, NOT BYTES!

		pcre_free(reUrlRemainder);

		if(!sPrevLineLink.empty() && bMatchContinuesLink)
		{
			string sPrevLineLinkCopy(sPrevLineLink);
			if(sPrevLineLink[sPrevLineLink.size() - 1] != '.')
			{
				pcre* re = pcre_compile("^(php|htm|asp)", PCRE_UTF8 | PCRE_NO_UTF8_CHECK, &szErrDescr, &iErrOffset, NULL);
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

		pcre* reProtocol = pcre_compile("^(http://|https://|ftp://|ftps://)",
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

	pcre_free(reUrlRemainder);

	return false;
}
#undef OVECTOR_SIZE


/************************************************************************/
/* Raw Stripper Code                                                    */
/************************************************************************/

string CNFOData::GetStrippedTextUtf8(const wstring& a_text)
{
	string l_text;
	wstring l_tmpw;
	l_text.reserve(a_text.size() / 2);

	//bool l_prevWasNl = false;
	for(size_t p = 0; p < a_text.size(); p++)
	{
		if(iswpunct(a_text[p]) || iswalnum(a_text[p]) || (iswspace(a_text[p]) && a_text[p] != L'\t'))
		{
			if(a_text[p] == L'\n') CUtil::StrTrimRight(l_tmpw, L" \t");
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
			while(l_tmpw[p] == L'\n') l_tmpw.erase(p, 1);
		}
	}

	l_text = CUtil::FromWideStr(l_tmpw, CP_UTF8);

	// :TODO: 

	return l_text;
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

