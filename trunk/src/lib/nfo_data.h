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
	const size_t GetLength() const { return m_colEnd - m_colStart + 1; }
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

	bool HasData() const { return m_loaded; }
	const std::_tstring GetFilePath() const { return m_filePath; }
	const std::_tstring GetFileName() const;

	const std::wstring& GetLastErrorDescription() const;
	size_t GetGridWidth();
	size_t GetGridHeight();
	wchar_t GetGridChar(size_t a_row, size_t a_col);
	char* GetGridCharUtf8(size_t a_row, size_t a_col);

	const std::string& GetTextUtf8() const { return m_utf8Content; }
	const std::wstring& GetTextWide() const { return m_textContent; }
	const std::vector<char> GetTextCP437(size_t& ar_charsNotConverted, bool a_compoundWhitespace = false) const;

	bool SaveToUnicodeFile(const std::_tstring& a_filePath, bool a_utf8 = true, bool a_compoundWhitespace = false);
	bool SaveToCP437File(const std::_tstring& a_filePath, size_t& ar_charsNotConverted, bool a_compoundWhitespace = false);

	const CNFOHyperLink* GetLink(size_t a_row, size_t a_col) const;
	const CNFOHyperLink* GetLinkByIndex(size_t a_index) const;
	const std::vector<const CNFOHyperLink*> GetLinksForLine(size_t a_row) const;

	static std::string GetStrippedTextUtf8(const std::wstring& a_text);

	void SetCharsetToTry(ENfoCharset a_charset) { m_sourceCharset = a_charset; }
	ENfoCharset GetCharset() const { return m_sourceCharset; }
	static const std::wstring GetCharsetName(ENfoCharset a_charset);
	void SetWrapLines(bool nb) { m_lineWrap = nb; } /* only effective when calling Load* the next time */
	bool GetWrapLines() const { return m_lineWrap; }

	typedef std::deque<std::wstring> TLineContainer;

protected:
	std::wstring m_lastErrorDescr;
	std::wstring m_textContent;
	std::string m_utf8Content;
	TwoDimVector<wchar_t> *m_grid;
	char *m_utf8Grid;
	bool m_loaded;
	std::multimap<size_t, CNFOHyperLink> m_hyperLinks;
	std::_tstring m_filePath;
	ENfoCharset m_sourceCharset;
	bool m_lineWrap;
	bool m_isAnsi;
	size_t m_ansiHintWidth;
	size_t m_ansiHintHeight;

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

	bool AnsiSysTransform(const std::wstring& a_text, size_t& ar_maxLineLen, TLineContainer& ar_lines);

	std::wstring GetWithBoxedWhitespace() const;

	FILE *OpenFileForWritingWithErrorMessage(const std::_tstring& a_filePath);

	static bool FindLink(const std::string& sLine, size_t& uirOffset, size_t& urLinkPos, size_t& urLinkLen,
		std::string& srUrl, const std::string& sPrevLineLink, bool& brLinkContinued);

	// following: static cache stuff for link detection regex.

	class CLinkRegEx
	{
	public:
		CLinkRegEx(const char* regex_str, bool a_cont, bool a_mailto = false)
		{
			const char *szErrDescr = NULL;
			int iErrOffset;

			m_re = pcre_compile(regex_str,
				PCRE_CASELESS | PCRE_UTF8 | PCRE_NO_UTF8_CHECK,
				&szErrDescr, &iErrOffset, NULL);
			_ASSERT(m_re != NULL);
			// no further error handling because all the regex are hardcoded

			m_cont = a_cont;
			m_mailto = a_mailto;
		}

		pcre *GetRE() const { return m_re; }
		bool IsCont() const { return m_cont; }
		bool IsMailto() const { return m_mailto; }

		virtual ~CLinkRegEx()
		{
			if(m_re)
			{
				pcre_free(m_re);
			}
		}
	protected:
		pcre *m_re;
		bool m_cont;
		bool m_mailto;
	};
	typedef shared_ptr<CLinkRegEx> PLinkRegEx;

	static std::vector<PLinkRegEx> ms_linkTriggers;
	static void PopulateLinkTriggers();
};

typedef shared_ptr<CNFOData> PNFOData;

#endif /* !_NFO_DATA_H */
