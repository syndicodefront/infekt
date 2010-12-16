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

int _wtoi(const wchar_t* str)
{
	bool bGotSign = false;
	wchar_t wSign = '+';
	int result = 0;

	wchar_t* p = str;

	while(*p)
	{
		if(!bGotSign)
		{
			if(*p == L'+' || *p == L'-')
			{
				bGotSign = true;
				wSign = *p;
			}
			else if(iswdigit(*p))
			{
				bGotSign = true;
				result = static_cast<int>(*p - L'0');
			}
			else if(!iswspace(*p))
				break;

			p++;
		}
		else if(iswdigit(*p))
		{
			result *= 10;
			result += static_cast<int>(*p - L'0');
		}
		else
			break;
	}

	return result;
}

