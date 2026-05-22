/**
 * Copyright (C) 2010 syndicode
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
#include "nfo_data.h"

bool LoadNFOFromStream(IStream* pStream, PNFOData& ar_data)
{
	// read NFO data from stream:
	std::string l_contents;
	size_t l_contentLength = 0;
	char l_buf[200]{};

	const LARGE_INTEGER l_nullPos{};
	pStream->Seek(l_nullPos, STREAM_SEEK_SET, nullptr);

	ULONG l_bytesRead;
	do
	{
		HRESULT hr = pStream->Read(l_buf, 200, &l_bytesRead);

		if (hr == S_OK || (hr == S_FALSE && l_bytesRead < ARRAYSIZE(l_buf)))
		{
			l_contents.append(l_buf, l_bytesRead);
			l_contentLength += l_bytesRead;

			if (l_contentLength > 1024 * 1024)
			{
				// don't try to mess with files > 1MB.
				return S_FALSE;
			}
		}
		else
			break;
	} while (l_bytesRead == ARRAYSIZE(l_buf));

	CNFOData* l_temp = new (std::nothrow) CNFOData();

	if (l_temp)
	{
		// process NFO contents into CNFOData instance:
		PNFOData l_nfoData(l_temp);
		l_nfoData->SetWrapLines(true);

		if (l_nfoData->LoadFromMemory((const unsigned char*)l_contents.data(), l_contentLength))
		{
			ar_data = l_nfoData;

			return true;
		}
	}

	return false;
}
