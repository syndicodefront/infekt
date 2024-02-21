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

#ifndef _NFO_DATA_H
#define _NFO_DATA_H

#include "util.h"
#include "nfo_hyperlink.h"
#include "nfo_colormap.h"

typedef enum: uint8_t
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
	NFOC_CP437_IN_CP437_IN_UTF8,

	_NFOC_MAX
} ENfoCharset;


class CNFOData // this could use some refactoring :P
{
public:
	CNFOData();

	bool LoadFromFile(const std::_tstring& a_filePath);
	bool LoadFromMemory(const unsigned char* a_data, size_t a_dataLen);
	bool LoadStripped(const CNFOData& a_source);

	bool HasData() const { return m_loaded; }
	const std::_tstring GetFilePath() const { return m_filePath; }
	const std::_tstring GetFileName() const;

	void SetVirtualFileName(const std::_tstring& a_filePath, const std::_tstring& a_fileName);

	size_t GetGridWidth() const;
	size_t GetGridHeight() const;
	wchar_t GetGridChar(size_t a_row, size_t a_col) const;
#ifdef INFEKT_2_CXXRUST
	// Best effort to return a UTF-32 char, but it might be part of a UTF-16 surrogate pair on Windows:
	uint32_t GetGridCharUtf32(size_t a_row, size_t a_col) const { 
		return static_cast<uint32_t>(GetGridChar(a_row, a_col));
	}
#endif
	const std::string& GetGridCharUtf8(size_t a_row, size_t a_col) const;
	const std::string& GetGridCharUtf8(wchar_t a_wideChar) const;

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

	typedef enum {
		NDE_NO_ERROR = 0,
		NDE_UNRECOGNIZED_FILE_FORMAT = -9999,
		NDE_ENCODING_PROBLEM,
		NDE_UNABLE_TO_OPEN_PHYSICAL,
		NDE_FAILED_TO_DETERMINE_SIZE,
		NDE_SIZE_EXCEEDS_LIMIT,
		NDE_FERROR,
		NDE_ANSI_INTERNAL,
		NDE_EMPTY_FILE,
		NDE_MAXIMUM_LINE_LENGTH_EXCEEDED,
		NDE_MAXIMUM_NUMBER_OF_LINES_EXCEEDED,
		NDE_SAUCE_INTERNAL,
	} EErrorCode;

	bool IsInError() const { return m_lastErrorCode != NDE_NO_ERROR; }
	const std::string& GetLastErrorDescription() const { return m_lastErrorDescr; }
	EErrorCode GetLastErrorCode() const { return m_lastErrorCode; }

private:
	EErrorCode m_lastErrorCode;
	std::string m_lastErrorDescr;

	std::wstring m_textContent;
	std::string m_utf8Content;
	std::unique_ptr<TwoDimVector<wchar_t>> m_grid;
	std::map<wchar_t, std::string> m_utf8Map;
	bool m_loaded;
	std::multimap<size_t, CNFOHyperLink> m_hyperLinks;
	std::_tstring m_filePath;
	std::_tstring m_vFileName;
	ENfoCharset m_sourceCharset;
	bool m_lineWrap;
	bool m_isAnsi;
	size_t m_ansiHintWidth;
	size_t m_ansiHintHeight;
	PNFOColorMap m_colorMap;

	static const int LINES_LIMIT = 10000;
	static const int WIDTH_LIMIT = 2000;

	enum class EApproach
	{
		EA_FALSE = 0,
		EA_TRY,
		EA_FORCE
	};

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
	bool HasFileExtension(const TCHAR* a_extension) const;
	bool PostProcessLoadedContent();

	std::wstring GetWithBoxedWhitespace() const;
	std::wstring GetStrippedText() const;

	FILE *OpenFileForWritingWithErrorMessage(const std::_tstring& a_filePath);

	void SetLastError(EErrorCode, const std::wstring&);
	void SetLastError(EErrorCode, const std::string&);
	void ClearLastError();
};

typedef std::shared_ptr<CNFOData> PNFOData;

#ifdef INFEKT_2_CXXRUST
// for rust interaction
std::unique_ptr<CNFOData> new_nfo_data();
#endif

#endif /* !_NFO_DATA_H */
