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

#ifndef _NFO_DATA_H
#define _NFO_DATA_H

#include "util.h"


class CNFOHyperLink
{
public:
	CNFOHyperLink();
	virtual ~CNFOHyperLink();

protected:
	int m_linkID;
	std::wstring m_href;
	size_t m_row;
	size_t m_colStart, m_colEnd;
};


class CNFOData
{
public:
	CNFOData();
	virtual ~CNFOData();

#ifdef _WIN32
	bool LoadFromFile(const std::wstring& a_fileName);
#else
	bool LoadFromFile(const std::string& a_fileName);
#endif
	bool LoadFromMemory(const unsigned char* a_data, size_t a_dataLen);

	bool HasData() const { return m_loaded; }

	const std::wstring& GetLastErrorDescription();
	size_t GetGridWidth();
	size_t GetGridHeight();
	wchar_t GetGridChar(size_t a_row, size_t a_col);
	char* GetGridCharUtf8(size_t a_row, size_t a_col);

	static bool FindLink(const std::string& sLine, size_t& urLinkPos, size_t& urLinkLen,
		std::string& srUrl, const std::string& sPrevLineLink, bool& brLinkContinued);
protected:
	std::wstring m_lastErrorDescr;
	std::wstring m_textContent;
	std::string m_utf8Content;
	TwoDimVector<wchar_t> *m_grid;
	char *m_utf8Grid;
	bool m_loaded;
	std::deque<CNFOHyperLink> m_hyperLinks;

	bool TryLoad_UTF8Signature(const unsigned char* a_data, size_t a_dataLen);
};

#ifdef HAVE_BOOST
typedef boost::shared_ptr<CNFOData> PNFOData;
#else
typedef CNFOData* PNFOData;
#endif


#endif /* !_NFO_DATA_H */
