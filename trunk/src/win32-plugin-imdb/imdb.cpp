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

#include "imdb-plugin.h"


INFEKT_PLUGIN_METHOD(ImdbMainEventCallback)
{
	switch(lCall)
	{
	case IPV_NFO_LOADED: {
		infekt_nfo_info_t* l_info = (infekt_nfo_info_t*)pParam;
		MessageBox(0, l_info->filePath, L"LOADED", 0);
						 }
		break;
	}

	return IPE_NOT_IMPLEMENTED;
}
