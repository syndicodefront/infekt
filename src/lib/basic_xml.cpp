/**
 * Copyright (C) 2011 syndicode
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
#include "util.h"
#include "basic_xml.h"


/************************************************************************/
/* UTILITY METHODS                                                      */
/************************************************************************/

std::string CXMLParser::XmlEncode(const std::string& sString)
{
	std::string l_str = CUtil::StrReplace("&", "&amp;", sString);
	l_str = CUtil::StrReplace("<", "&lt;", l_str);
	l_str = CUtil::StrReplace(">", "&gt;", l_str);
	l_str = CUtil::StrReplace("\"", "&quot;", l_str);
	return l_str;
}

#define FAST_ISDIGIT(C) (C >= '0' && C <= '9')
#define FAST_ISDIGITX(C) (FAST_ISDIGIT(C) || (C >= 'a' && C <= 'f') || (C >= 'A' && C <= 'F'))

#include "html_entities.inc"

std::string CXMLParser::XmlDecode(const std::string& sString, bool a_decodeHtmlEntities)
{
	std::wstring swStr;
	swStr.reserve(sString.size());

	std::string::size_type p = sString.find('&'), pPrev = 0, pStartRemain = 0;

	do
	{
		if(p == std::string::npos)
		{
			// handle rest of the str...
			swStr += CUtil::ToWideStr(sString.substr(pStartRemain), CP_UTF8);
			break;
		}
		else
		{
			swStr += CUtil::ToWideStr(sString.substr(pPrev, p - pPrev), CP_UTF8);
		}

		bool bIgnore = true;

		std::string::size_type pEnd = sString.find(';', p);
		if(pEnd != std::string::npos)
		{
			const std::string sEntity = sString.substr(p + 1, pEnd - p - 1);

			if(sEntity.size() >= 2 && sEntity[0] == '#')
			{
				bool bHex = (sEntity[1] == 'x' || sEntity[1] == 'X');
				bool bOk = true;

				for(std::string::size_type ep = 2; bOk && ep < sEntity.size(); ep++)
				{
					bOk = (bHex ? FAST_ISDIGITX(sEntity[ep]) : FAST_ISDIGIT(sEntity[ep]));
				}

				if(bOk && (!bHex || sEntity.size() >= 3))
				{
					const unsigned long long lle = _strtoui64(sEntity.c_str() + (bHex ? 2 : 1), NULL, (bHex ? 16 : 10));

					if(lle > 0 && lle <= std::numeric_limits<wchar_t>::max())
					{
						swStr += (wchar_t)lle;
						bIgnore = false;
					}
				}
			}
			else if(!_stricmp(sEntity.c_str(), "amp")) { swStr += '&'; bIgnore = false; }
			else if(!_stricmp(sEntity.c_str(), "gt")) { swStr += '>'; bIgnore = false; }
			else if(!_stricmp(sEntity.c_str(), "lt")) { swStr += '<'; bIgnore = false; }
			else if(!_stricmp(sEntity.c_str(), "quot")) { swStr += '"'; bIgnore = false; }
			else if(!_stricmp(sEntity.c_str(), "apos")) { swStr += '\''; bIgnore = false; }
			else if(a_decodeHtmlEntities)
			{
				const wchar_t* l_tmp = html_entities::translate(sEntity.c_str());

				if(l_tmp)
				{
					swStr += l_tmp;
					bIgnore = false;
				}
			}
		}
		else
			pEnd = p + 1;

		if(!bIgnore)
		{
			pPrev = pEnd + 1;
		}

		p = sString.find('&', pEnd + 1);

		pStartRemain = pEnd + 1;

	} while(true);

	return CUtil::FromWideStr(swStr, CP_UTF8);
}


std::string CXMLParser::StripTags(const std::string& sString)
{
	std::string::size_type l_pos, l_prevPos = 0;
	std::string l_new;

	l_pos = sString.find('<');

	while(l_pos != std::string::npos)
	{
		std::string::size_type l_endPos = sString.find('>', l_pos + 1);

		if(l_endPos == std::string::npos)
		{
			break;
		}

		l_new += sString.substr(l_prevPos, l_pos - l_prevPos);

		l_prevPos = l_endPos + 1;
		l_pos = sString.find('<', l_prevPos);
	}

	if(l_prevPos > 0)
	{
		l_new += sString.substr(l_prevPos);
		return l_new;
	}
	else
	{
		return sString;
	}
}


/************************************************************************/
/* CXMLParser IMPLEMENTATION                                            */
/************************************************************************/

// will miss some things such as the bar in <foo lol="wat" bar>, but whatever.
void CXMLParser::ParseAttributes(const std::string& sTagContents, std::map<const std::string, std::string>& mAttributes)
{
	std::string::size_type pos = sTagContents.find(" ");
	bool bInName = true, bWaitingForVal = false, bInVal = false;
	char cQuote = 0;
	std::string::size_type valStartPos = 0;
	std::string sName;

	while(pos != std::string::npos && pos < sTagContents.size())
	{
		if(bInName && iswspace(sTagContents[pos]))
		{
			pos++;
		}
		else if(bInName && sTagContents[pos] == '=')
		{
			bInName = false;
			bWaitingForVal = true;
			pos = sTagContents.find_first_of("'\"", pos + 1);
		}
		else if(bInName)
		{
			sName += sTagContents[pos];
			pos++;
		}
		else if(bWaitingForVal && (sTagContents[pos] == '"' || sTagContents[pos] == '\''))
		{
			cQuote = sTagContents[pos];
			bInVal = true;
			bWaitingForVal = false;
			valStartPos = pos + 1;
			pos = sTagContents.find(cQuote, pos + 1);
		}
		else if(bInVal && sTagContents[pos] == cQuote)
		{
			bInVal = false;
			bInName = true;
			mAttributes[sName] = sTagContents.substr(valStartPos, pos - valStartPos);
			mAttributes[sName] = XmlDecode(mAttributes[sName]);
			sName = "";

			pos++;
		}
		else
			throw CXMLException("internal problem while parsing attributes", 0);
	}

	if(bInVal || bWaitingForVal)
	{
		throw CXMLException("Couldn't parse some tag attributes: <" + sTagContents + ">", 0);
	}
}


PXMLTag CXMLParser::ParseString(const std::string& sXmlString)
{
	std::stack<PXMLTag> m_openTags;
	std::stack<PXMLTag> m_openTagParent;

	PXMLTag xParent, xRoot;
	bool bEnding = false;

	std::string::size_type pos = sXmlString.find("<");
	std::string::size_type iTextStartPos = std::string::npos;
	unsigned int iIteration = 0, iTextStartPosIteration = 0;

	while(pos != std::string::npos)
	{
		if(bEnding)
		{
			throw CXMLException("Multiple root tags?", pos);
		}

		iIteration++;
		std::string::size_type tagendpos = sXmlString.find(">", pos + 1);

		if(tagendpos == std::string::npos)
		{
			throw CXMLException("No terminating > for open <", pos);
		}

		if(tagendpos == pos + 1)
		{
			throw CXMLException("Empty tag", pos);
		}

		std::string sTagContents = sXmlString.substr(pos + 1, tagendpos - pos - 1);
		sTagContents = CUtil::StrReplace("\r", "", sTagContents);
		sTagContents = CUtil::StrReplace("\n", "", sTagContents);
		CUtil::StrTrim(sTagContents);

		std::string::size_type posTagName = sTagContents.find(' ');
		if(posTagName == std::string::npos) posTagName = sTagContents.size();
		std::string sTagName = sTagContents.substr(0, posTagName);

		if(sTagName.substr(0, 3) == "!--")
		{
			// skip comments.
			tagendpos = sXmlString.find("-->", pos) + 2;

			if(tagendpos == std::string::npos)
			{
				throw CXMLException("Unterminated comment", pos);
			}
		}
		else if(sTagName[0] == '?')
		{
			// skip <?xml stuff without any further checks.
		}
		else if(sTagName.substr(0, 8) == "![CDATA[")
		{
			std::string::size_type posCDataEnd = sXmlString.find("]]>", pos);

			if(posCDataEnd == std::string::npos)
			{
				throw CXMLException("Unterminated CDATA section", pos);
			}
			else if(m_openTags.empty())
			{
				throw CXMLException("CDATA section outside of a tag", pos);
			}

			PXMLTag xOpenTag = m_openTags.top();

			xOpenTag->m_text = sXmlString.substr(pos + 1 + 8, posCDataEnd - pos - 9);
			CUtil::StrTrim(xOpenTag->m_text);

			tagendpos = posCDataEnd + 2;
		}
		else if(sTagName[0] != '/')
		{
			// found start tag
			PXMLTag xNew(new CXMLTag());
			xNew->m_name = sTagName;

			ParseAttributes(sTagContents, xNew->m_attributes);

			// look out for <img /> style tags:
			if(sTagContents[sTagContents.size() - 1] != '/')
			{
				m_openTags.push(xNew);
				if(xParent) m_openTagParent.push(xParent);
				xParent = xNew;

				// save the position in case this tag has no child tags
				// and we want to extract text from it.
				iTextStartPos = tagendpos + 1;
				iTextStartPosIteration = iIteration;
			}
			else if(xParent)
			{
				xParent->m_children.push_back(xNew);
			}
			else
			{
				xRoot = xNew;
			}
		}
		else
		{
			// found end tag
			sTagName.erase(0, 1);

			if(m_openTags.size() == 0 || _stricmp(m_openTags.top()->m_name.c_str(), sTagName.c_str()) != 0)
			{
				throw CXMLException("Ending tag for '" + std::string(sTagName) + "', which is not open", pos);
			}

			// take the now-closed tag off the stack:
			PXMLTag xClosedTag = m_openTags.top();

			m_openTags.pop();

			// if no other tags have been found in between, extract text:
			if(iIteration == iTextStartPosIteration + 1)
			{
				xClosedTag->m_text = sXmlString.substr(iTextStartPos, pos - iTextStartPos);
				xClosedTag->m_text = XmlDecode(xClosedTag->m_text);
			}

			// no parent = root tag. if this happens, we've walked the tree and are done.
			if(m_openTagParent.empty())
			{
				xRoot = xClosedTag;
				bEnding = true;
			}
			else
			{
				// re-set old parent:
				xParent = m_openTagParent.top();
				m_openTagParent.pop();

				// otherwise, save the new child and to the next tag...
				xParent->m_children.push_back(xClosedTag);
			}
		}

		pos = sXmlString.find("<", tagendpos + 1);
	}

	if(m_openTags.size() != 0)
	{
		throw CXMLException("Found unclosed tags", 0);
	}

	return xRoot;
}


PXMLTag CXMLParser::ParseFile(const std::wstring& a_filePath)
{
	std::string l_buf;
	FILE *fFile = NULL;

	if(_wfopen_s(&fFile, a_filePath.c_str(), L"r") != 0)
	{
		throw CXMLException("Unable to open file.", 0);
	}

	while(!feof(fFile))
	{
		char szBuf[4096] = {0};
		size_t l_read = fread_s(szBuf, 4096, sizeof(char), 4096, fFile);

		l_buf.append(szBuf, l_read);
	}

	fclose(fFile);

	return ParseString(l_buf);
}
