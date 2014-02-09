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

#ifndef _NFO_DATA_H
#define _NFO_DATA_H

#include "util.h"
#include "nfo_hyperlink.h"
#include "nfo_colormap.h"

typedef enum
{
	NFOC_AUTO = 1,
	NFOC_UTF16,
	NFOC_UTF8_SIG,
	NFOC_UTF8,
	NFOC_CP437,
	NFOC_CP437_IN_UTF8,
	NFOC_CP437_IN_UTF16,
	NFOC_CP437_STRICT,
	NFOC_WINDOWS_1252,
	NFOC_CP437_IN_CP437,

	_NFOC_MAX
} ENfoCharset;


class CNFOData // this could use some refactoring :P
{
public:
	CNFOData();
	virtual ~CNFOData();

	bool LoadFromFile(const std::_tstring& a_filePath);
	bool LoadFromMemory(const unsigned char* a_data, size_t a_dataLen);
	bool LoadStripped(const CNFOData& a_source);

	bool HasData() const { return m_loaded; }
	const std::_tstring GetFilePath() const { return m_filePath; }
	const std::_tstring GetFileName() const;

	const std::wstring& GetLastErrorDescription() const;
	size_t GetGridWidth() const;
	size_t GetGridHeight() const;
	wchar_t GetGridChar(size_t a_row, size_t a_col) const;
	const std::string GetGridCharUtf8(size_t a_row, size_t a_col) const;
	const std::string GetGridCharUtf8(wchar_t a_wideChar) const;

	const std::string& GetTextUtf8();
	const std::wstring& GetTextWide() const { return m_textContent; }
	const std::vector<char> GetTextCP437(size_t& ar_charsNotConverted, bool a_compoundWhitespace = false) const;

	bool SaveToUnicodeFile(const std::_tstring& a_filePath, bool a_utf8 = true, bool a_compoundWhitespace = false);
	bool SaveToCP437File(const std::_tstring& a_filePath, size_t& ar_charsNotConverted, bool a_compoundWhitespace = false);

	const CNFOHyperLink* GetLink(size_t a_row, size_t a_col) const;
	const CNFOHyperLink* GetLinkByIndex(size_t a_index) const;
	const std::vector<const CNFOHyperLink*> GetLinksForLine(size_t a_row) const;

	void SetCharsetToTry(ENfoCharset a_charset) { m_sourceCharset = a_charset; }
	ENfoCharset GetCharset() const { return m_sourceCharset; }
	static const std::wstring GetCharsetName(ENfoCharset a_charset);
	const std::wstring GetCharsetName() const;
	void SetWrapLines(bool nb) { m_lineWrap = nb; } /* only effective when calling Load* the next time */
	bool GetWrapLines() const { return m_lineWrap; }

	bool HasColorMap() const { return m_isAnsi && m_colorMap && m_colorMap->HasColors(); }
	const PNFOColorMap GetColorMap() const { return m_colorMap; }

	typedef std::list<const std::wstring> TLineContainer;

protected:
	std::wstring m_lastErrorDescr;
	std::wstring m_textContent;
	std::string m_utf8Content;
	TwoDimVector<wchar_t> *m_grid;
	std::map<wchar_t, std::string> m_utf8Map;
	bool m_loaded;
	std::multimap<size_t, CNFOHyperLink> m_hyperLinks;
	std::_tstring m_filePath;
	ENfoCharset m_sourceCharset;
	bool m_lineWrap;
	bool m_isAnsi;
	size_t m_ansiHintWidth;
	size_t m_ansiHintHeight;
	PNFOColorMap m_colorMap;

	static const int LINES_LIMIT = 10000;
	static const int WIDTH_LIMIT = 2000;

	typedef enum _approach_t
	{
		EA_FALSE = 0,
		EA_TRY,
		EA_FORCE
	} EApproach;

	bool LoadFromMemoryInternal(const unsigned char* a_data, size_t a_dataLen);
	bool ReadSAUCE(const unsigned char* a_data, size_t& ar_dataLen);

	bool TryLoad_UTF16LE(const unsigned char* a_data, size_t a_dataLen, EApproach a_fix);
	bool TryLoad_UTF16BE(const unsigned char* a_data, size_t a_dataLen);
	bool TryLoad_UTF8Signature(const unsigned char* a_data, size_t a_dataLen);
	bool TryLoad_UTF8(const unsigned char* a_data, size_t a_dataLen, EApproach a_fix);
	bool TryLoad_CP437(const unsigned char* a_data, size_t a_dataLen, EApproach a_fix);
	bool TryLoad_CP437_Strict(const unsigned char* a_data, size_t a_dataLen);
	bool TryLoad_CP252(const unsigned char* a_data, size_t a_dataLen);

	bool DetectAnsi() const;
	bool PostProcessLoadedContent();

	std::wstring GetWithBoxedWhitespace() const;
	std::wstring GetStrippedText() const;

	FILE *OpenFileForWritingWithErrorMessage(const std::_tstring& a_filePath);
};

typedef shared_ptr<CNFOData> PNFOData;

#endif /* !_NFO_DATA_H */
