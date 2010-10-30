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

#define _TARGETVER_VISTA // for targetver.h
#include "targetver.h"

#include <string>
#include <shobjidl.h>


std::wstring Win6x_OpenFileDialog(HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec)
{
	HRESULT hr;
	std::wstring l_path;

	IFileOpenDialog *pfod;
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pfod));

	if(SUCCEEDED(hr))
	{
		// set up some options:
		pfod->SetFileTypes(a_nFilterSpec, a_filterSpec);
		pfod->SetFileTypeIndex(1);

		pfod->SetOptions(FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_DONTADDTORECENT);

		// show the dialog:
		hr = pfod->Show(a_parent);

		if(SUCCEEDED(hr))
		{
			IShellItem *ppsi;

			// this will fail if Cancel has been clicked:
			hr = pfod->GetResult(&ppsi);

			if(SUCCEEDED(hr))
			{
				// extract the path:
				LPOLESTR pszPath = NULL;

				hr = ppsi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
				if(SUCCEEDED(hr))
				{
					l_path = pszPath;
					CoTaskMemFree(pszPath);
				}

				ppsi->Release();
			}
		}

		pfod->Release();
	}

	return l_path;
}


std::wstring Win6x_SaveFileDialog(HWND a_parent, const COMDLG_FILTERSPEC* a_filterSpec, UINT a_nFilterSpec,
	const LPCWSTR a_defaultExt, const std::wstring& a_currentFileName, const std::wstring& a_initialPath)
{
	HRESULT hr;
	std::wstring l_path;

	IFileSaveDialog *pfod;
	hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pfod));

	if(SUCCEEDED(hr))
	{
		// set up some options:
		pfod->SetFileTypes(a_nFilterSpec, a_filterSpec);
		pfod->SetFileTypeIndex(1);
		pfod->SetDefaultExtension(a_defaultExt);

		pfod->SetOptions(FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT | FOS_DONTADDTORECENT);

		if(!a_currentFileName.empty())
		{
			pfod->SetFileName(a_currentFileName.c_str());
		}

		if(!a_initialPath.empty())
		{
			// force initially selected folder
			IShellItem *ppsif = NULL;
			hr = SHCreateItemFromParsingName(a_initialPath.c_str(), NULL, IID_PPV_ARGS(&ppsif));

			if(SUCCEEDED(hr))
			{
				hr = pfod->SetFolder(ppsif);
				ppsif->Release();
			}
		}

		// show the dialog:
		hr = pfod->Show(a_parent);

		if(SUCCEEDED(hr))
		{
			IShellItem *ppsi;

			// this will fail if Cancel has been clicked:
			hr = pfod->GetResult(&ppsi);

			if(SUCCEEDED(hr))
			{
				// extract the path:
				LPOLESTR pszPath = NULL;

				hr = ppsi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
				if(SUCCEEDED(hr))
				{
					l_path = pszPath;
					CoTaskMemFree(pszPath);
				}

				ppsi->Release();
			}
		}

		pfod->Release();
	}

	return l_path;
}
