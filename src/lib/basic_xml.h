/**
 * Copyright (C) 2011 cxxjoe
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

#ifndef _BASIC_XML_H
#define _BASIC_XML_H

class CXMLException
{
protected:
	std::string m_message;
	size_t m_pos;
public:
	CXMLException(const std::string& sMessage, size_t iPos)
	{
		m_message = sMessage;
		m_pos = iPos;
	}
	const std::string GetMessage() const { return m_message; }
};

class CXMLParser;
class CXMLTag;
typedef boost::shared_ptr<CXMLTag> PXMLTag;

class CXMLTag
{
	friend class CXMLParser;
protected:
	std::string m_name;
	std::map<const std::string, std::string> m_attributes;
	std::vector<PXMLTag> m_children;
	std::string m_text;

	CXMLTag()
	{
	}
public:
	const std::string& GetName() const
	{
		return m_name;
	}

	const std::string GetAttribute(const std::string& sName) const
	{
		std::map<const std::string, std::string>::const_iterator it = m_attributes.find(sName);
		return (it != m_attributes.end() ? it->second : "");
	}

	const PXMLTag GetChildByName(const std::string& sTagName) const
	{
		for(std::vector<PXMLTag>::const_iterator it = m_children.begin(); it != m_children.end(); it++)
		{
			if(!_stricmp((*it)->m_name.c_str(), sTagName.c_str()))
			{
				return *it;
			}
		}
		return PXMLTag();
	}

	const std::string GetChildText(const std::string& sTagName) const
	{
		const PXMLTag xTag = GetChildByName(sTagName);
		return (xTag ? xTag->m_text : "");
	}

	const std::string GetText() const
	{
		return m_text;
	}

	const std::vector<PXMLTag>::const_iterator BeginChildren() const
	{
		return m_children.begin();
	}

	const std::vector<PXMLTag>::const_iterator EndChildren() const
	{
		return m_children.end();
	}
};

class CXMLParser
{
protected:
	static void ParseAttributes(const std::string& sTagContents, std::map<const std::string, std::string>& mAttributes);
public:
	static PXMLTag ParseString(const std::string& sXmlString);
	static PXMLTag ParseFile(const std::wstring& a_filePath);

	static std::string XmlEncode(const std::string& sString);
	static std::string XmlDecode(const std::string& sString);
	static std::string XmlNamedEntityDecode(const std::string& sString);
	static std::string StripTags(const std::string& sString);
};

#endif /* !_BASIC_XML_H */
