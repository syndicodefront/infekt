/**
 * Copyright (C) 2010-2016 syndicode
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
#include "nfo_hyperlink.h"
#include "util.h"

using std::string;
using std::wstring;

CNFOHyperLink::CNFOHyperLink(int linkID, const wstring& href, size_t row, size_t col, size_t len)
	: m_linkID(linkID), m_href(href), m_row(row), m_colStart(col), m_colEnd(col + len - 1)
{
}

bool CNFOHyperLink::FindLink(const std::wstring& sLine, size_t& uirOffset, size_t& urLinkPos, size_t& urLinkLen, std::wstring& srUrl, const std::wstring& sPrevLineLink, bool& brLinkContinued)
{
	srUrl.clear();

	if (sLine.find_first_of(L"./") == std::wstring::npos)
	{
		// no need to run any regex if the line does not contain a . or /
		return false;
	}

	if (ms_linkTriggers.empty())
	{
		// init static stuff
		PopulateLinkTriggers();
	}

	//
	// Phase 1: find earliest link starting point:
	//
	size_t linkStartPos = (size_t)-1;
	bool matchContinuesLink = false;
	bool matchIsMailLink = false;
	size_t matchLinkLength = 0;
	// using this because std::regex_search cannot be given an offset:
	const std::wstring sLineRemainder = sLine.substr(uirOffset);

	for (const CLinkRegEx& linkRegEx : ms_linkTriggers)
	{
		// never match continuations when an actual link start or earlier continuation has been found:
		// (all continuations are sorted after normal triggers)
		if (linkRegEx.IsContinuation())
		{
			if (sPrevLineLink.empty() || linkStartPos != (size_t)-1)
			{
				break;
			}
		}

		std::wsmatch match;

		// probe:
		if (!std::regex_search(sLineRemainder, match, linkRegEx.GetStdRegEx()))
		{
			continue;
		}

		size_t newPos = uirOffset + match.position(match.size() < 2 ? 0 : 1);

		// find the earliest link start:
		if (newPos < linkStartPos)
		{
			linkStartPos = newPos;

			matchContinuesLink = linkRegEx.IsContinuation();
			
			if (linkRegEx.IsMailLink())
			{
				matchLinkLength = match.length();
				matchIsMailLink = true;
			}
		}
	}

	if (linkStartPos == (size_t)-1)
	{
		// no link found.
		return false;
	}

	//
	// Phase 2: get the full link:
	//          (sWorkUrl must have the same length as in the original document, no fixes here yet!)
	//

	std::wstring sWorkUrl;

	if (matchLinkLength > 0)
	{
		sWorkUrl = sLine.substr(linkStartPos, matchLinkLength);
	}
	else
	{
		// ugly workaround for commonly present IMDB links:
		// https://github.com/syndicodefront/infekt/issues/94
		// (cannot entirely remove this functionality, it's often used for other types of links)
		if (!sPrevLineLink.empty() && matchContinuesLink && sPrevLineLink.find(L"imdb.") != std::wstring::npos)
		{
			if (std::regex_search(sPrevLineLink, std::wregex(L"/[a-z]{2}\\d{6,}/?$")))
			{
				return false;
			}
		}

		std::wstring sLineRemainder = sLine.substr(linkStartPos);

		if (sLineRemainder.find(L"hxxp://") == 0 || sLineRemainder.find(L"h**p://") == 0)
		{
			sLineRemainder.replace(0, 7, L"http://");
		}
		else if (sLineRemainder.find(L"hxxps://") == 0 || sLineRemainder.find(L"h**ps://") == 0)
		{
			sLineRemainder.replace(0, 8, L"https://");
		}

		std::wregex reUrlRemainder(
			std::wstring(L"^[\\w,/.!#:%;?&=~+-]") + (matchContinuesLink ? L"{4,}" : L"{9,}"));
		std::wsmatch match;

		if (std::regex_search(sLineRemainder, match, reUrlRemainder))
		{
			sWorkUrl = match.str();

			// strip trailing dots and colons. gross code.
			while (sWorkUrl.size() && (sWorkUrl[sWorkUrl.size() - 1] == L'.' || sWorkUrl[sWorkUrl.size() - 1] == L':'))
			{
				sWorkUrl.erase(sWorkUrl.size() - 1);
			}
		}
	}

	if (sWorkUrl.empty())
	{
		return false;
	}

	//
	// Phase 3: combine and fix up final URL:
	//

	urLinkPos = linkStartPos;
	urLinkLen = sWorkUrl.size();
	uirOffset = linkStartPos + sWorkUrl.size();

	if (matchIsMailLink)
	{
		srUrl = L"mailto:" + sWorkUrl;
		brLinkContinued = false;

		return true;
	}

	if (sWorkUrl.find(L"http://http://") == 0)
	{
		// sigh...
		sWorkUrl.erase(0, 7);
	}

	if (!sPrevLineLink.empty() && matchContinuesLink)
	{
		wstring sPrevLineLinkCopy(sPrevLineLink);

		if (sPrevLineLink[sPrevLineLink.size() - 1] != '.')
		{
			if (std::regex_search(sWorkUrl, std::wregex(L"^(html?|php|aspx?|jpe?g|png|gif)")))
			{
				sPrevLineLinkCopy += L'.';
			}
		}

		sWorkUrl.insert(0, sPrevLineLinkCopy);
		brLinkContinued = true;
	}
	else
	{
		brLinkContinued = false;
	}

	if (!std::regex_search(sWorkUrl, std::wregex(L"^(http|ftp)s?://", std::regex::icase)))
	{
		sWorkUrl.insert(0, L"http://");
	}

	if (std::regex_search(sWorkUrl, std::wregex(L"^(http|ftp)s?://([\\w-]+)\\.([\\w.-]+)/?", std::regex::icase)))
	{
		srUrl = sWorkUrl;
	}

	return !srUrl.empty();
}

void CNFOHyperLink::PopulateLinkTriggers()
{
	// keep compiled trigger regexes because all those execute on
	// every single line, so this is an easy performance gain.

	ms_linkTriggers.emplace_back(CLinkRegEx(L"h(?:tt|xx|\\*\\*)p://", false));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"h(?:tt|xx|\\*\\*)ps://", false));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"www\\.", false));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"\\w+\\.imdb\\.com", false));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"imdb\\.com", false));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"(imdb|ofdb|cinefacts|zelluloid|kino)\\.de", false));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"(tinyurl|twitter|facebook|imgur|youtube)\\.com", false));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"\\b(bit\\.ly|goo\\.gl|t\\.co|youtu\\.be)/", false));

	ms_linkTriggers.emplace_back(CLinkRegEx(L"[a-zA-Z0-9]+(?:[a-zA-Z0-9._=-]*)@[a-zA-Z](?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,}", false, true, false));

	//
	// all link continuations must appear *after* non-continuations in the list:
	//

	ms_linkTriggers.emplace_back(CLinkRegEx(L"^\\s*(/)", true));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"(\\S+\\.(?:html?|php|aspx?|jpe?g|png|gif)\\S*)", true));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"(\\S+/dp/\\S*)", true)); // for amazon
	ms_linkTriggers.emplace_back(CLinkRegEx(L"(\\S*dp/[A-Z]\\S+)", true)); // for amazon
	ms_linkTriggers.emplace_back(CLinkRegEx(L"(\\S*/\\w+=\\S+)", true));
	ms_linkTriggers.emplace_back(CLinkRegEx(L"(\\S+[&?]\\w+=\\S*)", true));

	ms_linkTriggers.emplace_back(CLinkRegEx(L"(\\S{4,}/\\S*)", true));
	// use at least 4 chars so "4.4/10" in a line following an imdb link does not trigger.
}

std::vector<CNFOHyperLink::CLinkRegEx> CNFOHyperLink::ms_linkTriggers;

/**
 * CLinkRegEx constructor
 */
CNFOHyperLink::CLinkRegEx::CLinkRegEx(const std::wstring& regexStr, bool isContinutation, bool isMailLink, bool ignoreCase)
	: m_isContinuation(isContinutation), m_isMailLink(isMailLink)
{
	std::regex::flag_type flags = std::regex::ECMAScript | std::regex::optimize;

	if (ignoreCase)
	{
		flags = flags | std::regex::icase;
	}

	m_regex.swap(std::wregex(regexStr, flags));
}
