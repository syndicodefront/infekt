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

#ifndef _NFO_HYPERLINK_H
#define _NFO_HYPERLINK_H

#include <string>
#include <vector>
#include <regex>

class CNFOHyperLink
{
public:
	CNFOHyperLink() = delete;
	CNFOHyperLink(int linkID, const std::wstring& href, size_t row, size_t col, size_t len);

	void SetHref(const std::wstring& a_href) { m_href = a_href; }
	const int GetLinkID() const { return m_linkID; }
	const std::wstring& GetHref() const { return m_href; }
	const size_t GetRow() const { return m_row; }
	const size_t GetColStart() const { return m_colStart; }
	const size_t GetColEnd() const { return m_colEnd; }
	const size_t GetLength() const { return m_colEnd - m_colStart + 1; }

	static bool FindLink(const std::wstring& sLine, size_t& uirOffset, size_t& urLinkPos, size_t& urLinkLen,
		std::wstring& srUrl, const std::wstring& sPrevLineLink, bool& brLinkContinued);

protected:
	int m_linkID;
	std::wstring m_href;
	size_t m_row;
	size_t m_colStart, m_colEnd;

protected:
	// following: static cache stuff for link detection regex.

	class CLinkRegEx
	{
	public:
		CLinkRegEx(const std::wstring& regexStr, bool isContinutation, bool isMailLink = false, bool ignoreCase = true);

		bool IsContinuation() const { return m_isContinuation; }
		bool IsMailLink() const { return m_isMailLink; }

		const std::wregex& GetStdRegEx() const { return m_regex; }

	protected:
		std::wregex m_regex;
		bool m_isContinuation;
		bool m_isMailLink;
	};

	static std::vector<CLinkRegEx> ms_linkTriggers;
	static void PopulateLinkTriggers();
};

#endif /* !_NFO_HYPERLINK_H */
