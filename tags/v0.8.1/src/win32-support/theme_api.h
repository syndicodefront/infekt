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

#ifndef _THEME_API_H
#define _THEME_API_H

class CThemeAPI
{
public:
	static const CThemeAPI* GetInstance();
	virtual ~CThemeAPI();

	bool DwmIsCompositionEnabled();

protected:
	HMODULE m_hDwmApi;
private:
	CThemeAPI();
};


#endif /* !_THEME_API_H */
