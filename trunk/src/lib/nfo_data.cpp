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
#include "pcre.h"

using namespace std;


CNFOData::CNFOData()
{
	m_grid = NULL;
	m_loaded = false;
	m_utf8Grid = NULL;
}


#ifdef _WIN32
bool CNFOData::LoadFromFile(const wstring& a_fileName)
#else
bool CNFOData::LoadFromFile(const string& a_fileName)
#endif
{
	FILE *l_file = NULL;
	size_t l_fileBytes;

#ifdef _WIN32
	if(_wfopen_s(&l_file, a_fileName.c_str(), L"rb") != 0 || !l_file)
#else
	if(l_file = fopen(a_fileName.c_str(), "rb"))
#endif
	{
#ifdef HAVE_BOOST
		m_lastErrorDescr = FORMAT(L"Unable to open NFO file '%s' (error %d)", a_fileName % errno);
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
		l_loaded = LoadFromMemory(l_buf, l_fileBytes + 1);
	}
	else
	{
		m_lastErrorDescr = L"An error occured while reading from the NFO file.";
	}

	delete[] l_buf;

	fclose(l_file);

	m_loaded = l_loaded;

	return l_loaded;
}


bool CNFOData::LoadFromMemory(const unsigned char* a_data, size_t a_dataLen)
{
	bool l_loaded = false;

	m_loaded = false;

	l_loaded = TryLoad_UTF8Signature(a_data, a_dataLen);

	//if(!l_loaded) l_loaded = TryLoad_...
	//if(!l_loaded) l_loaded = TryLoad_...
	//if(!l_loaded) l_loaded = TryLoad_...

	if(l_loaded)
	{
		// split raw contents into grid buffer.

		size_t l_maxLineLen = 1;
		size_t l_prevPos = 0, l_pos = m_textContent.find_first_of(L"\r\n");
		deque<const wstring> l_lines;

		// read lines:
		while(l_pos != wstring::npos)
		{
			wstring l_line = m_textContent.substr(l_prevPos, l_pos - l_prevPos);

			// remove \n if we found a \r\n:
			if(m_textContent[l_pos] == L'\r' &&
				l_pos < m_textContent.size() - 1 &&
				m_textContent[l_pos + 1] == L'\n')
			{
				l_pos++;
			}

			// trim trailing whitespace:
			while(l_line.size() && iswspace(l_line[l_line.size() - 1])) l_line.erase(l_line.size() - 1);

			l_lines.push_back(l_line);

			if(l_line.length() > l_maxLineLen)
			{
				l_maxLineLen = l_line.length();
			}

			l_prevPos = l_pos + 1;
			l_pos = m_textContent.find_first_of(L"\r\n", l_pos + 1);
		}

		// :TODO: interpret ANSI escape codes.

		// copy lines to grid(s):
		delete m_grid;
		delete m_utf8Grid;
		m_utf8Content.clear();
		m_utf8Content.reserve(m_textContent.length());

		// allocate mem:
		m_grid = new TwoDimVector<wchar_t>(l_lines.size(), l_maxLineLen + 1, 0);
		m_utf8Grid = new char[l_lines.size() * m_grid->GetCols() * 7];
		memset(m_utf8Grid, 0, l_lines.size() * m_grid->GetCols() * 7);

		// vars for hyperlink detection:
		string l_prevLinkUrl; // UTF-8
		int l_prevLinkId = 1;

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
			m_utf8Content += "\n";

			// find hyperlinks:
			if(/* m_bFindHyperlinks == */true)
			{
				size_t l_linkPos, l_linkLen;
				bool l_linkContinued;
				string l_url;

				if(FindLink(l_utf8Line, l_linkPos, l_linkLen, l_url, l_prevLinkUrl, l_linkContinued))
				{

				}
			}
		}


	}

	return l_loaded;
}


bool CNFOData::TryLoad_UTF8Signature(const unsigned char* a_data, size_t a_dataLen)
{
	if(a_dataLen < 3)
	{
		return false;
	}

	if(a_data[0] != 0xEF || a_data[1] != 0xBB || a_data[2] != 0xBF)
	{
		// no UTF-8 "BOM" found.
		return false;
	}

	m_textContent = CUtil::ToWideStr(
		string().append((char*)(a_data + 3), a_dataLen - 3), CP_UTF8);

	return true;
}


const wstring& CNFOData::GetLastErrorDescription()
{
	return m_lastErrorDescr;
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
				//assert(ovector[idx] < 0 || ovector[idx + 1] < 0)
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
		//assert(ovector[0] == 0);
		uByteLen = (size_t)ovector[1] - (size_t)ovector[0];

		string sWorkUrl = sLineRemainder.substr((size_t)ovector[0], uByteLen);
		// :TODO: trim match .: and whitespace

		// :TODO: populate urLinkPos and urLinkLen

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


CNFOData::~CNFOData()
{
	delete m_grid;
	delete m_utf8Grid;
}


/************************************************************************/
/* CNFOHyperLink Implementation                                         */
/************************************************************************/

CNFOHyperLink::CNFOHyperLink()
{
	m_linkID = -1; // means unset/invalid
	m_row = m_colEnd = m_colStart = 0;
}





CNFOHyperLink::~CNFOHyperLink()
{
}

