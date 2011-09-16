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

#ifndef _CONTENT_SCRAPER_H
#define _CONTENT_SCRAPER_H

#include "basic_xml.h"

class CContentScraper
{
public:
	CContentScraper();
	virtual ~CContentScraper();

	bool LoadScraperFile(const std::wstring& a_filePath);
	bool DoScrape(const std::string& a_content);

	int GetInt(const std::string& a_name);
	std::string GetString(const std::string& a_name);
	bool GetList(const std::string& a_name, std::vector<const std::string>& ar_result);

protected:
	PXMLTag m_scrapeDefs;
	std::map<const std::string, std::string> m_extractedStrings;
	std::map<const std::string, std::vector<const std::string> > m_extractedLists;
};

#endif /* !_CONTENT_SCRAPER_H */
