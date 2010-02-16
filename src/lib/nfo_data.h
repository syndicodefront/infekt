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
	CNFOHyperLink(int a_linkID, const std::wstring& a_href, size_t a_row,
		size_t a_col, size_t a_len);
	virtual ~CNFOHyperLink();

	void SetHref(const std::wstring& a_href);
	const int GetLinkID() const { return m_linkID; }
	const std::wstring& GetHref() const { return m_href; }
	const size_t GetRow() const { return m_row; }
	const size_t GetColStart() const { return m_colStart; }
	const size_t GetColEnd() const { return m_colEnd; }
	const size_t GetLength() const { return m_colEnd - m_colStart; }
protected:
	int m_linkID;
	std::wstring m_href;
	size_t m_row;
	size_t m_colStart, m_colEnd;
private:
	CNFOHyperLink() {};
};


typedef enum _nfo_charsets
{
	NFOC_UNKNOWN = 0,
	NFOC_UTF16,
	NFOC_UTF8_SIG,
	NFOC_UTF8,
	NFOC_CP437,

	_NFOC_MAX
} TNfoCharset;


class CNFOData
{
public:
	CNFOData();
	virtual ~CNFOData();

	bool LoadFromFile(const std::_tstring& a_filePath);
	bool LoadFromMemory(const unsigned char* a_data, size_t a_dataLen);

	bool HasData() const { return m_loaded; }
	const std::_tstring GetFilePath() { return m_filePath; }
	const std::_tstring GetFileName();

	const std::wstring& GetLastErrorDescription();
	size_t GetGridWidth();
	size_t GetGridHeight();
	wchar_t GetGridChar(size_t a_row, size_t a_col);
	char* GetGridCharUtf8(size_t a_row, size_t a_col);

	const std::string& GetTextUtf8() const { return m_utf8Content; }
	const std::wstring& GetTextWide() const { return m_textContent; }

	bool SaveToFile(std::_tstring a_filePath, bool a_utf8 = true);

	const CNFOHyperLink* GetLink(size_t a_row, size_t a_col) const;

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
	std::_tstring m_filePath;
	TNfoCharset m_sourceCharset;

	bool TryLoad_UTF8Signature(const unsigned char* a_data, size_t a_dataLen);
	bool TryLoad_UTF8(const unsigned char* a_data, size_t a_dataLen);
	bool TryLoad_CP437(const unsigned char* a_data, size_t a_dataLen);
};

#ifdef HAVE_BOOST
typedef boost::shared_ptr<CNFOData> PNFOData;
#else
typedef CNFOData* PNFOData;
#endif


#endif /* !_NFO_DATA_H */
