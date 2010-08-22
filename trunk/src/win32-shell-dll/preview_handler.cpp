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



/************************************************************************/
/* CreateInstance Export                                                */
/************************************************************************/

HRESULT CNFOPreviewHandler_CreateInstance(REFIID riid, void **ppv)
{
	*ppv = NULL;

	/*CNFOPreviewHandler *pNew = new (std::nothrow) CNFOPreviewHandler();

	if(pNew)
	{
		HRESULT hr = pNew->QueryInterface(riid, ppv);
		pNew->Release();

		return hr;
	}*/

	return E_OUTOFMEMORY;
}
