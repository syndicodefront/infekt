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

#include "stdafx.h"
#include "content_scraper.h"
#include "pcrecpp.h"


CContentScraper::CContentScraper()
{
}


bool CContentScraper::LoadScraperFile(const std::wstring& a_filePath)
{
	if(!::PathFileExists(a_filePath.c_str()))
	{
		return false;
	}

	try
	{
		m_scrapeDefs = CXMLParser::ParseFile(a_filePath);
	}
	catch(CXMLException e)
	{
		(void)e;
		return false;
	}

	if(m_scrapeDefs->GetName() == "scrape")
	{
		return true;
	}
	else
	{
		m_scrapeDefs.reset();

		return false;
	}
}


bool CContentScraper::DoScrape(const std::string& a_content)
{
	std::map<const std::string, std::string> l_symbols;

	if(!m_scrapeDefs)
	{
		return false;
	}

	for(std::vector<PXMLTag>::const_iterator it = m_scrapeDefs->BeginChildren(); it != m_scrapeDefs->EndChildren(); it++)
	{
		if((*it)->GetName() == "extract")
		{

		}
	}

	return true;
}


CContentScraper::~CContentScraper()
{
}
