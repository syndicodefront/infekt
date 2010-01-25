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

	return l_loaded;
}


bool CNFOData::LoadFromMemory(const unsigned char* a_data, size_t a_dataLen)
{
	bool l_loaded = false;

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

		while(l_pos != wstring::npos)
		{
			const wstring l_line = m_textContent.substr(l_prevPos, l_pos - l_prevPos);

			if(m_textContent[l_pos] == L'\r' &&
				l_pos < m_textContent.size() - 1 &&
				m_textContent[l_pos + 1] == L'\n')
			{
				l_pos++;
			}

			l_lines.push_back(l_line);

			if(l_line.length() > l_maxLineLen)
			{
				l_maxLineLen = l_line.length();
			}

			l_prevPos = l_pos + 1;
			l_pos = m_textContent.find_first_of(L"\r\n", l_pos + 1);
		}

		delete m_grid;

		m_grid = new TwoDimVector<wchar_t>(l_lines.size(), l_maxLineLen + 1, 0);

		size_t i = 0;
		for(deque<const wstring>::const_iterator it = l_lines.begin();
			it != l_lines.end(); it++, i++)
		{
			size_t l_lineLen = it->length();

			for(size_t j = 0; j < l_lineLen; j++)
			{
				(*m_grid)[i][j] = (*it)[j];
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


CNFOData::~CNFOData()
{
	delete m_grid;
}
