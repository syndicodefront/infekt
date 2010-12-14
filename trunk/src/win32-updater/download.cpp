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

#include "updater.h"
#include <WinInet.h>


/************************************************************************/
/* SOME STATIC VARS                                                     */
/************************************************************************/

static bool s_bDownloading = false;
static __int64 s_uBytesReceived = 0;

static HWND s_hDlg = 0;
static std::wstring s_url, s_localPath;


/************************************************************************/
/* DOWNLOADER GUTS                                                      */
/************************************************************************/

static void __cdecl HttpThread(void *pvStartupInfo)
{
	HANDLE hStartEvent = static_cast<HANDLE>(pvStartupInfo);

	std::wstring l_url = s_url,
		l_localPath = s_localPath,
		l_tempLocalPath = s_localPath + L".bak";
	HWND l_hDlg = s_hDlg;

	::SetEvent(hStartEvent);

	// get on with the actual downloading business:
	HINTERNET hInet;
	BOOL bSuccess = TRUE;

	hInet = InternetOpen(L"Mozilla/5.0 (compatible; HttpThread/1.0)",
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if(hInet)
	{
		HINTERNET hRequest;
		DWORD dwFlags = 0;

		if(l_url.find(L"https://") == 0)
			dwFlags |= INTERNET_FLAG_SECURE;

		hRequest = InternetOpenUrl(hInet, l_url.c_str(), NULL, 0, dwFlags, 0);

		if(hRequest)
		{
			__int64 uFileSize = 0;
			wchar_t wszSizeBuffer[32];
			DWORD dwLengthSizeBuffer = 32;

			if(HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH, wszSizeBuffer, &dwLengthSizeBuffer, NULL) == TRUE)
			{
				uFileSize = _wcstoi64(wszSizeBuffer, NULL, 10);

				SendMessage(l_hDlg, WM_DOWNLOAD_STARTED, (UINT_PTR)&uFileSize, 0);
			}

			if(uFileSize > 0)
			{
				char szBuffer[8192];
				DWORD dwRead;
				FILE *fFile = NULL;

				if(_wfopen_s(&fFile, l_tempLocalPath.c_str(), L"wb") != 0)
				{
					SendMessage(l_hDlg, WM_DOWNLOAD_FAILED, 0, 0);
				}

				s_uBytesReceived = 0;

				while(InternetReadFile(hRequest, szBuffer, 8191, &dwRead))
				{
					if(!dwRead || dwRead > 8191)
					{
						break;
					}

					size_t dwWritten = 0;

					if(fFile)
					{
						dwWritten = fwrite(szBuffer, dwRead, 1, fFile);
					}

					if(dwWritten != 1)
					{
						bSuccess = FALSE;
						break;
					}

					s_uBytesReceived += dwRead;
				}

				fclose(fFile);
			}

			if(bSuccess) bSuccess = (s_uBytesReceived == uFileSize);

			InternetCloseHandle(hRequest);
		}

		InternetCloseHandle(hInet);
	}

	if(!bSuccess)
	{
		SendMessage(l_hDlg, WM_DOWNLOAD_FAILED, 0, 0);
	}
	else
	{
		::MoveFileEx(l_tempLocalPath.c_str(), l_localPath.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		SendMessage(l_hDlg, WM_DOWNLOAD_COMPLETE, 0, 0);
	}

	s_bDownloading = false;
}


/************************************************************************/
/* MAIN EXTERNAL ENTRY POINT                                            */
/************************************************************************/

bool StartHttpDownload(HWND hDlg, const std::wstring& a_url, const std::wstring& a_localPath)
{
	if(s_bDownloading || a_localPath.empty())
	{
		return false;
	}

	s_bDownloading = true;

	if(::PathFileExists(a_localPath.c_str()))
	{
		if(!::DeleteFile(a_localPath.c_str()))
		{
			s_bDownloading = false;
			return false;
		}
	}

	// damn, this is really bad style!
	s_url = a_url;
	s_localPath = a_localPath;
	s_hDlg = hDlg;

	// track thread startup using an event:
	HANDLE hStartEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	if(_beginthread(HttpThread, 0, hStartEvent))
	{
		::WaitForSingleObject(hStartEvent, INFINITE);
	}

	CloseHandle(hStartEvent);

	return s_bDownloading;
}


__int64 HttpGetBytesReceived()
{
	return s_uBytesReceived;
}


bool HttpIsDownloading()
{
	return s_bDownloading;
}
