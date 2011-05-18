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
#include "util.h"


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


static bool _FindAndConsumeDynamicGroups(pcrecpp::RE* re, pcrecpp::StringPiece* a_piece, std::vector<std::string>& grps)
{
	bool bMatch = false;
	switch(re->NumberOfCapturingGroups())
	{
		// yeah, right.....
	case 1: bMatch = re->FindAndConsume(a_piece, &grps[0]); break;
	case 2: bMatch = re->FindAndConsume(a_piece, &grps[0], &grps[1]); break;
	case 3: bMatch = re->FindAndConsume(a_piece, &grps[0], &grps[1], &grps[2]); break;
	case 4: bMatch = re->FindAndConsume(a_piece, &grps[0], &grps[1], &grps[2], &grps[3]); break;
	case 5: bMatch = re->FindAndConsume(a_piece, &grps[0], &grps[1], &grps[2], &grps[3], &grps[4]); break;
	case 6: bMatch = re->FindAndConsume(a_piece, &grps[0], &grps[1], &grps[2], &grps[3], &grps[4], &grps[5]); break;
	case 7: bMatch = re->FindAndConsume(a_piece, &grps[0], &grps[1], &grps[2], &grps[3], &grps[4], &grps[5], &grps[6]); break;
	case 8: bMatch = re->FindAndConsume(a_piece, &grps[0], &grps[1], &grps[2], &grps[3], &grps[4], &grps[5], &grps[6], &grps[7]); break;
	case 9: bMatch = re->FindAndConsume(a_piece, &grps[0], &grps[1], &grps[2], &grps[3], &grps[4], &grps[5], &grps[6], &grps[7], &grps[8]); break;
	}
	return bMatch;
}


bool CContentScraper::DoScrape(const std::string& a_content)
{
	std::map<const std::string, std::string> l_symbols;

	if(!m_scrapeDefs)
	{
		return false;
	}

	// :TODO: evaluate <vars><var>...

	// define reserved symbol for entire input:
	l_symbols["_INPUT_"] = a_content;

	// go through root tag and find <extract> children:
	for(std::vector<PXMLTag>::const_iterator ite = m_scrapeDefs->BeginChildren(); ite != m_scrapeDefs->EndChildren(); ite++)
	{
		if((*ite)->GetName() != "extract")
		{
			continue;
		}

		const std::string l_regexStr = (*ite)->GetChildText("regex");
		const std::string l_extractFrom = (*ite)->GetAttribute("from");

		if(l_regexStr.empty())
		{
			continue;
		}

		// convert regex flags from char string to pcrecpp flags:
		const std::string l_regexFlagsStr = (*ite)->GetChildByName("regex")->GetAttribute("flags");

		pcrecpp::RE_Options l_reOptions = pcrecpp::UTF8();
		l_reOptions.set_caseless(l_regexFlagsStr.find('i') != std::string::npos);
		l_reOptions.set_multiline(l_regexFlagsStr.find('m') != std::string::npos);
		l_reOptions.set_dotall(l_regexFlagsStr.find('s') != std::string::npos);

		// put together regex object, use extra parenthesis to simulate capture group 0
		// (because it's not exposed by pcrecpp's API)
		pcrecpp::RE l_regex("(" + l_regexStr + ")", l_reOptions);

		if(!l_regex.error().empty())
		{
#ifdef _DEBUG
			OutputDebugStringA("Error in RE ~");
			OutputDebugStringA(l_regexStr.c_str());
			OutputDebugStringA("~: ");
			OutputDebugStringA(l_regex.error().c_str());
			OutputDebugStringA("\n");
#endif
			continue;
		}

		// can't operate on undefined symbols:
		if(l_symbols.find(l_extractFrom) == l_symbols.end())
		{
			continue;
		}

		// check only_if_empty:
		if(!(*ite)->GetAttribute("only_if_empty").empty())
		{
			const std::string l_checkEmptySymbol = (*ite)->GetAttribute("only_if_empty");

			if(!(l_symbols.find(l_checkEmptySymbol) == l_symbols.end() || l_symbols[l_checkEmptySymbol].empty()))
			{
				continue;
			}
		}

		// reserve space for result capture groups
		std::vector<std::string> l_groups(9, "");
		pcrecpp::StringPiece l_reInput(l_symbols[l_extractFrom]);

		// execute RE, extract from input, leave <extract> if there's no match:
		if(!_FindAndConsumeDynamicGroups(&l_regex, &l_reInput, l_groups))
		{
			continue;
		}

		// go through <target> children
		for(std::vector<PXMLTag>::const_iterator itt = (*ite)->BeginChildren(); itt != (*ite)->EndChildren(); itt++)
		{
			if((*itt)->GetName() != "target")
			{
				continue;
			}

			// target properties...
			std::string l_sGrp = (*itt)->GetAttribute("capgroup"),
				l_toSymbol = (*itt)->GetAttribute("to"),
				l_appendToSymbol = (*itt)->GetAttribute("append_to");

			if(l_sGrp.empty() || (l_toSymbol.empty() && l_appendToSymbol.empty()))
			{
				_ASSERT(false);
				continue;
			}

			// get+validate source group index:
			int l_grp = atoi(l_sGrp.c_str());

			if(l_grp < 0 || l_grp > l_regex.NumberOfCapturingGroups())
			{
				_ASSERT(false);
				continue;
			}

			// extract filters:
			std::vector<std::string> l_filterNames;
			if((*itt)->GetAttribute("filter") != "")
			{
				l_filterNames.push_back((*itt)->GetAttribute("filter"));
			}

			for(std::vector<PXMLTag>::const_iterator itf = (*itt)->BeginChildren(); itf != (*itt)->EndChildren(); itf++)
			{
				l_filterNames.push_back((*itf)->GetAttribute("name"));
			}

			// extract data from group + apply filters.
			// loop if necessary (append_to attribute).
			bool bLoopOccurrences = (!l_appendToSymbol.empty());
			const std::string l_action = (*itt)->GetAttribute("action");

			do
			{
				std::string l_temp = l_groups[l_grp];

				// apply transformations ("action"):
				if(l_action == "input_replace")
				{
					std::string l_replaceWith = (*itt)->GetAttribute("replace_with");

					l_temp = CUtil::StrReplace(l_temp, l_replaceWith, l_symbols[l_extractFrom]);
				}
				else if(l_action != "")
				{
					_ASSERT(false);
				}

				// apply filters:
				for(std::vector<std::string>::const_iterator itfn = l_filterNames.begin(); itfn != l_filterNames.end(); itfn++)
				{
					if(*itfn == "dequote")
					{
						l_temp = CXMLParser::StripTags(l_temp);
						l_temp = CXMLParser::XmlDecode(l_temp);
					}
					else if(*itfn == "trim")
					{
						CUtil::StrTrim(l_temp);
					}
					else if(*itfn == "depunctuate")
					{
						l_temp = CUtil::StrReplace(",", "", l_temp);
						l_temp = CUtil::StrReplace(".", "", l_temp);
					}
					else if(*itfn == "int")
					{
						std::stringstream ss;
						ss << _strtoi64(l_temp.c_str(), NULL, 10);
						l_temp = ss.str();
					}
					else if(*itfn == "float")
					{
						std::stringstream ss;
						ss << strtod(l_temp.c_str(), NULL);
						l_temp = ss.str();
					}
				}

				// store result:
				if(!bLoopOccurrences)
				{
					l_symbols[l_toSymbol] = l_temp;
				}
				else
				{
					// :TODO: implement lists
					l_symbols[l_appendToSymbol] += l_temp;
				}
			} while (bLoopOccurrences && _FindAndConsumeDynamicGroups(&l_regex, &l_reInput, l_groups));
		}
	}

	return true;
}


CContentScraper::~CContentScraper()
{
}
