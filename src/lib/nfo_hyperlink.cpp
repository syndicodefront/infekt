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

#include "stdafx.h"
#include "nfo_hyperlink.h"
#include "util.h"

using std::string;
using std::wstring;


CNFOHyperLink::CNFOHyperLink(int a_linkID, const wstring& a_href, size_t a_row, size_t a_col, size_t a_len)
{
	m_linkID = a_linkID;
	m_href = a_href;
	m_row = a_row;
	m_colStart = a_col;
	m_colEnd = a_col + a_len - 1;
}


/*static*/ void CNFOHyperLink::PopulateLinkTriggers()
{
#define TRGR(a, b) PLinkRegEx(new CLinkRegEx(a, b))

	// cache compiled trigger regexes because all those execute on
	// every single line, so this is an easy performance gain.

	if(!ms_linkTriggers.empty())
	{
		return;
	}

	ms_linkTriggers.push_back(TRGR("h(?:tt|xx|\\*\\*)p://", false));
	ms_linkTriggers.push_back(TRGR("h(?:tt|xx|\\*\\*)ps://", false));
	ms_linkTriggers.push_back(TRGR("www\\.", false));
	ms_linkTriggers.push_back(TRGR("\\w+\\.imdb\\.com", false));
	ms_linkTriggers.push_back(TRGR("imdb\\.com", false));
	ms_linkTriggers.push_back(TRGR("(imdb|ofdb|cinefacts|zelluloid|kino)\\.de", false));
	ms_linkTriggers.push_back(TRGR("(tinyurl|twitter|facebook|imgur|youtube)\\.com", false));
	ms_linkTriggers.push_back(TRGR("(bit\\.ly|goo\\.gl|t\\.co|youtu\\.be)", false));

	ms_linkTriggers.push_back(PLinkRegEx(new CLinkRegEx("[a-zA-Z0-9._=-]+@[a-zA-Z](?:[a-zA-Z0-9-]+\\.)+[a-z]+", false, true)));

	// all link continuations must appear *after* non-continuations in the list:

	ms_linkTriggers.push_back(TRGR("^\\s*(/)", true));
	ms_linkTriggers.push_back(TRGR("(\\S+\\.(?:html?|php|aspx?|jpe?g|png|gif)\\S*)", true));
	ms_linkTriggers.push_back(TRGR("(\\S+/dp/\\S*)", true)); // for amazon
	ms_linkTriggers.push_back(TRGR("(\\S*dp/[A-Z]\\S+)", true)); // for amazon
	ms_linkTriggers.push_back(TRGR("(\\S+[&?]\\w+=\\S*)", true));

	/*if(sPrevLineLink[sPrevLineLink.size() - 1] == '-') originally */
	ms_linkTriggers.push_back(TRGR("(\\S{4,}/\\S*)", true));
	// use at least 4 chars so "4.4/10" in a line following an imdb link does not trigger.
#undef TRGR
}
std::vector<CNFOHyperLink::PLinkRegEx> CNFOHyperLink::ms_linkTriggers;


#define OVECTOR_SIZE 30 // multiple of 3!
/*static*/ bool CNFOHyperLink::FindLink(const std::string& sLine, size_t& uirOffset, size_t& urLinkPos, size_t& urLinkLen,
			  std::string& srUrl, const std::string& sPrevLineLink, bool& brLinkContinued)
{
	size_t uBytePos = (size_t)-1, uByteLen = 0;

	srUrl.clear();

	if(sLine.size() > (uint64_t)std::numeric_limits<int>::max()
		|| uirOffset > (uint64_t)std::numeric_limits<int>::max())
	{
		return false;
	}

	// boring vars for pcre_compile:
	const char *szErrDescr = NULL;
	int iErrOffset;
	int ovector[OVECTOR_SIZE];

	PopulateLinkTriggers();

	// find link starting point:
	bool bMatchContinuesLink = false;
	bool bMailto = false;
	for(std::vector<PLinkRegEx>::const_iterator it = ms_linkTriggers.begin(); it != ms_linkTriggers.end(); it++)
	{
		if(sPrevLineLink.empty() && (*it)->IsCont())
		{
			continue;
		}

		if(pcre_exec((*it)->GetRE(), NULL, sLine.c_str(), (int)sLine.size(), (int)uirOffset, 0, ovector, OVECTOR_SIZE) >= 0)
		{
			int iCaptures = 0;
			if(pcre_fullinfo((*it)->GetRE(), NULL, PCRE_INFO_CAPTURECOUNT, &iCaptures) == 0)
			{
				int idx = (iCaptures == 1 ? 1 : 0) * 2;
				_ASSERT(ovector[idx] >= 0 && ovector[idx + 1] > 0);

				// never match continuations when an actual link start or earlier continuation has been found:
				if(uBytePos != (size_t)-1 && (*it)->IsCont())
				{
					break;
				}

				// find the earliest link start:
				if((size_t)ovector[idx] < uBytePos)
				{
					uBytePos = (size_t)ovector[idx];

					bMatchContinuesLink = (*it)->IsCont();
					bMailto = (*it)->IsMailto();

					if(bMailto)
					{
						uByteLen = (size_t)ovector[idx + 1] - uBytePos;
					}

					if((*it)->IsCont()) // purely an optimization
					{
						break;
					}
				}
			}
		}
	}

	if(uBytePos == (size_t)-1)
	{
		// no link found:
		return false;
	}

	// get the full link:
	string sWorkUrl;

	if(!bMailto)
	{
		string sLineRemainder = sLine.substr(uBytePos);

		if(sLineRemainder.find("hxxp://") == 0 || sLineRemainder.find("h**p://") == 0)
		{
			sLineRemainder.replace(0, 7, "http://");
		}
		else if(sLineRemainder.find("hxxps://") == 0 || sLineRemainder.find("h**ps://") == 0)
		{
			sLineRemainder.replace(0, 8, "https://");
		}

#define REMPAT "^[\\w,/.!#:%;?&=~+-]"

		pcre* reUrlRemainder = pcre_compile(bMatchContinuesLink ? REMPAT "{4,}" : REMPAT "{9,}",
			PCRE_UTF8 | PCRE_NO_UTF8_CHECK | PCRE_UCP, &szErrDescr, &iErrOffset, NULL);

#undef REMPAT

		if(pcre_exec(reUrlRemainder, NULL, sLineRemainder.c_str(), (int)sLineRemainder.size(), 0, 0, ovector, OVECTOR_SIZE) >= 0)
		{
			_ASSERT(ovector[0] == 0);
			uByteLen = (size_t)ovector[1] - (size_t)ovector[0];

			sWorkUrl = sLineRemainder.substr((size_t)ovector[0], uByteLen);

			// strip trailing dots and colons. gross code.
			while(sWorkUrl.size() && (sWorkUrl[sWorkUrl.size() - 1] == '.' || sWorkUrl[sWorkUrl.size() - 1] == ':'))
			{
				sWorkUrl.erase(sWorkUrl.size() - 1);
			}
		}

		pcre_free(reUrlRemainder);
	}
	else
	{
		// uByteLen has been set above.
		sWorkUrl = sLine.substr(uBytePos, uByteLen);
	}

	if(!sWorkUrl.empty())
	{
		urLinkPos = utf8_strlen(sLine.c_str(), uBytePos);
		urLinkLen = utf8_strlen(sWorkUrl.c_str(), -1); // IN CHARACTERS, NOT BYTES!

		uirOffset = uBytePos + uByteLen;

		if(bMailto)
		{
			// early exit for mailto links:

			srUrl = "mailto:" + sWorkUrl;
			brLinkContinued = false;

			return true;
		}

		if(sWorkUrl.find("http://http://") == 0)
		{
			// sigh...
			sWorkUrl.erase(0, 7);
		}

		if(!sPrevLineLink.empty() && bMatchContinuesLink)
		{
			string sPrevLineLinkCopy(sPrevLineLink);
			if(sPrevLineLink[sPrevLineLink.size() - 1] != '.')
			{
				pcre* re = pcre_compile("^(html?|php|aspx?|jpe?g|png|gif)", PCRE_UTF8 | PCRE_NO_UTF8_CHECK, &szErrDescr, &iErrOffset, NULL);
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

		pcre* reProtocol = pcre_compile("^(http|ftp)s?://",
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

	return false;
}
#undef OVECTOR_SIZE


/**
 * CLinkRegEx constructor
 */
CNFOHyperLink::CLinkRegEx::CLinkRegEx(const char* regex_str, bool a_cont, bool a_mailto)
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
