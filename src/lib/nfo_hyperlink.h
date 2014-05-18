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

#ifndef _NFO_HYPERLINK_H
#define _NFO_HYPERLINK_H

#include <string>

class CNFOHyperLink
{
public:
	CNFOHyperLink(int a_linkID, const std::wstring& a_href, size_t a_row,
		size_t a_col, size_t a_len);

	void SetHref(const std::wstring& a_href) { m_href = a_href; }
	const int GetLinkID() const { return m_linkID; }
	const std::wstring& GetHref() const { return m_href; }
	const size_t GetRow() const { return m_row; }
	const size_t GetColStart() const { return m_colStart; }
	const size_t GetColEnd() const { return m_colEnd; }
	const size_t GetLength() const { return m_colEnd - m_colStart + 1; }

	static bool FindLink(const std::string& sLine, size_t& uirOffset, size_t& urLinkPos, size_t& urLinkLen,
		std::string& srUrl, const std::string& sPrevLineLink, bool& brLinkContinued);

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
		CLinkRegEx(const char* regex_str, bool a_cont, bool a_mailto = false);

		pcre *GetRE() const { return m_re; }
		pcre_extra *GetStudy() const { return m_study; }
		bool IsCont() const { return m_cont; }
		bool IsMailto() const { return m_mailto; }

		virtual ~CLinkRegEx()
		{
			if(m_study) pcre_free_study(m_study);
			if(m_re) pcre_free(m_re);
		}
	protected:
		pcre *m_re;
		pcre_extra* m_study;
		bool m_cont;
		bool m_mailto;
	};
	typedef shared_ptr<CLinkRegEx> PLinkRegEx;

	static std::vector<PLinkRegEx> ms_linkTriggers;
	static void PopulateLinkTriggers();

private:
	CNFOHyperLink() {};
};

#endif /* !_NFO_HYPERLINK_H */
