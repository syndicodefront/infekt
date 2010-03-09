/**
 * Copyright (C) 2010 cxxjoe
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
#include "nfo_renderer.h"

using namespace std;


CNFOToHTML::CNFOToHTML(const PNFOData& a_nfoData)
{
	m_nfo = a_nfoData;
}


const wstring CNFOToHTML::GetHTML(bool a_includeHeaderAndFooter)
{
	wstringstream l_html;
	
	if(a_includeHeaderAndFooter)
	{
		l_html << L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" << endl;
		l_html << L"<html xmlns=\"http://www.w3.org/1999/xhtml\">" << endl;
		l_html << L"<head>" << endl;

		l_html << L"  <title>" << XMLEscape(m_title) << L"</title>" << endl;

		l_html << L"  <style type=\"text/css\">" << endl;
		l_html << L"  body { font-family: sans-serif; margin: 0; padding: 5px 15px; background: #fff; }" << endl;
		l_html << endl;

		l_html << L"  /** NFO STYLES BEGIN **/" << endl;
		l_html << L"  .infekt_nfo {" << endl;
		l_html << L"    background: #" << m_settings.cBackColor.AsHex(false) << L";" << endl; // don't use background-color because of IE bug.
		if(m_settings.cBackColor.A != 255) l_html << L"    background: #" << m_settings.cBackColor.AsHex(true) << L");" << endl;
		l_html << L"    padding: 7px;" << endl;
		l_html << L"  }" << endl;
		l_html << L"  .infekt_nfo pre {" << endl;
		l_html << L"    margin: 0; padding: 0;" << endl;
		l_html << L"    text-align: left; text-transform: none; font-style: normal;" << endl;
		l_html << L"    text-decoration: none; font-weight: normal;" << endl;
		l_html << L"    font-family: '" << XMLEscape(m_settings.sFontFace) << "', monospace;" << endl;
		l_html << L"    font-size: " << (m_settings.uFontSize ? m_settings.uFontSize : 12) << L"px;" << endl;
		l_html << L"    line-height: " << (m_settings.uFontSize ? m_settings.uFontSize : 12) << L"px;" << endl;
		l_html << L"  }" << endl;

		l_html << L"  .infekt_nfo .nfo_text";
		if(!m_settings.bHilightHyperlinks) l_html << L", .infekt_nfo .nfo_link {" << endl << L"    text-decoration: none;" << endl;
		else l_html << L" {" << endl;
		l_html << L"    color: #" << m_settings.cTextColor.AsHex(false) << L";" << endl;
		if(m_settings.cTextColor.A != 255) l_html << L"    color: #" << m_settings.cTextColor.AsHex(true) << L";" << endl;
		l_html << L"  }" << endl;

		l_html << L"  .infekt_nfo .nfo_block {" << endl;
		l_html << L"    color: #" << m_settings.cArtColor.AsHex(false) << L";" << endl;
		if(m_settings.cArtColor.A != 255) l_html << L"    color: #" << m_settings.cArtColor.AsHex(true) << L";" << endl;
		l_html << L"  }" << endl;

		if(m_settings.bHilightHyperlinks)
		{
			l_html << L"  .infekt_nfo a.nfo_link {" << endl;
			l_html << L"    color: #" << m_settings.cHyperlinkColor.AsHex(false) << L";" << endl;
			if(m_settings.cHyperlinkColor.A != 255) l_html << L"    color: #" << m_settings.cHyperlinkColor.AsHex(true) << L";" << endl;
			l_html << L"    text-decoration: " << (m_settings.bUnderlineHyperlinks ? L"underline;" : L"none;") << endl;
			l_html << L"  }" << endl;
		}

		l_html << L"  /** NFO STYLES END **/" << endl;
		l_html << endl;
		l_html << L"  </style>" << endl;

		l_html << L"</head>" << endl;
		l_html << L"<body>" << endl;

		l_html << L"  <h1>" << XMLEscape(m_title) << L"</h1>" << endl;
		l_html << endl;
	}

	l_html << "<div class=\"infekt_nfo\">";
	l_html << "<pre>";

	typedef enum
	{
		_BT_UNDEF = -1,
		BT_TEXT = 1,
		BT_BLOCK,
		BT_LINK
	} _block_color_type;

	const CNFOHyperLink* l_link = NULL;

	for(size_t row = 0; row < m_nfo->GetGridHeight(); row++)
	{
		_block_color_type l_curType = _BT_UNDEF;

		for(size_t col = 0; col < m_nfo->GetGridWidth(); col++)
		{
			_block_color_type l_type;
			wchar_t l_char = m_nfo->GetGridChar(row, col);

			if(!l_char) break; // end of line

			ERenderGridShape l_shape = CNFORenderer::CharCodeToGridShape(l_char);

			if(l_shape == RGS_NO_BLOCK || (l_shape == RGS_WHITESPACE && l_curType == BT_LINK))
			{
				if((l_link = m_nfo->GetLink(row, col)) != NULL)
					l_type = BT_LINK;
				else
					l_type = BT_TEXT;
			}
			else if(l_shape == RGS_WHITESPACE)
			{
				l_type = l_curType;
			}
			else
			{
				l_type = BT_BLOCK;
			}

			if(l_type != _BT_UNDEF && l_type != l_curType)
			{
				if(l_curType == BT_LINK)
					l_html << L"</a>";
				else if(l_curType != _BT_UNDEF)
					l_html << L"</span>";

				switch(l_type)
				{
				case BT_LINK:
					l_html << "<a href=\"" << XMLEscape(l_link->GetHref()) << "\" class=\"nfo_link\">";
					break;
				case BT_BLOCK:
					l_html << "<span class=\"nfo_block\">";
					break;
				case BT_TEXT:
					l_html << "<span class=\"nfo_text\">";
					break;
				}

				l_curType = l_type;
			}

			if(l_char == L'<') l_html << L"&lt;";
			else if(l_char == L'>') l_html << L"&gt;";
			else if(l_char == L'"') l_html << L"&quot;";
			else if(l_char == L'&') l_html << L"&amp;";
			else if(l_char >= 0x20 && l_char < 0x80)
			{
				l_html << l_char;
			}
			else
			{
				l_html << L"&#" << ((int32_t)l_char) << L";";
			}
		}

		if(l_curType == BT_LINK)
			l_html << L"</a>";
		else if(l_curType != _BT_UNDEF)
			l_html << L"</span>";

		l_html << endl;
	}

	l_html << "</pre>";
	l_html << "</div>" << endl;
	l_html << "<div style=\"clear: both;\"></div>" << endl;

	if(a_includeHeaderAndFooter)
	{
		l_html << endl;
		l_html << L"</body>" << endl;
		l_html << L"</html>" << endl;
	}

	return l_html.str();
}


wstring CNFOToHTML::XMLEscape(const std::wstring& s)
{
	wstring r;
	r.reserve(s.size());

	for(wstring::size_type p = 0; p < s.size(); p++)
	{
		switch(s[p])
		{
		case L'<': r += L"&lt;";
		case L'>': r += L"&gt;";
		case L'"': r += L"&quot;";
		case L'&': r += L"&amp;";
		default:
			r += s[p];
		}
	}

	return r;
}


CNFOToHTML::~CNFOToHTML()
{
}
