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
#include "win_http_client.h"
#include "util.h"

#define WM_WINHTTP_REQUEST_CALLBACK (WM_USER + 100)
#define WINHTTP_MESSAGE_ONLY_WINDOW_CLASSNAME L"CWinHttp_MessageOnlyWindow"


/************************************************************************/
/* CWinHttpClient implementation                                        */
/************************************************************************/

CWinHttpClient::CWinHttpClient(HINSTANCE a_hInstance)
	: m_nextReqId(1), m_hInstance(a_hInstance)
{
	WNDCLASSEX l_cls = { sizeof(WNDCLASSEX), 0 };
	l_cls.hInstance = m_hInstance;
	l_cls.lpszClassName = WINHTTP_MESSAGE_ONLY_WINDOW_CLASSNAME;
	l_cls.lpfnWndProc = &CWinHttpClient::MessageWindowProc;
	m_instanceReggedClass = (::RegisterClassEx(&l_cls) != 0);

	m_hwndMessageOnlyWin = ::CreateWindowEx(0, WINHTTP_MESSAGE_ONLY_WINDOW_CLASSNAME,
		NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, 0, NULL);

	OSVERSIONINFO l_osVer = { sizeof(OSVERSIONINFO), 0 };
	if(::GetVersionEx(&l_osVer))
	{
		std::wstringstream l_uas;

		l_uas << boost::wformat(L"Mozilla/%d.0 (compatible; MSIE %d.0; Windows NT %d.%d;%s Trident/%d.0)")
			% (l_osVer.dwMajorVersion > 5 ? 5 : 4) % (l_osVer.dwMajorVersion > 5 ? 9 : 8)
			% l_osVer.dwMajorVersion % l_osVer.dwMinorVersion
			% (CUtilWin32::IsWow64() ? L" WOW64;" : L"")
			% (l_osVer.dwMajorVersion > 5 ? 5 : 4);

		m_userAgent = l_uas.str();
	}
}


PWinHttpRequest CWinHttpClient::CreateRequest()
{
	PWinHttpRequest l_req = PWinHttpRequest(new CWinHttpRequest(m_nextReqId++, shared_from_this()));
	return l_req;
}


PWinHttpRequest CWinHttpClient::CreateRequestForTextFile(const std::wstring& a_url, WinHttpRequestCallback a_callback)
{
	PWinHttpRequest l_req = this->CreateRequest();

	l_req->SetUrl(a_url);
	l_req->SetCallback(a_callback);

	return l_req;
}


bool CWinHttpClient::StartRequest(PWinHttpRequest& a_req)
{
	if(a_req->GetCallback() != NULL)
	{
		_beginthread(&CWinHttpClient::RequestThreadMain, 0, a_req.get());

		// only add ref to our list when everything went well
		// and only then return true.
		m_requests[a_req->GetReqId()] = a_req;

		return true;
	}

	return false;
}


void CWinHttpClient::RequestThreadMain(void *a_userData)
{
	CWinHttpRequest *l_req = reinterpret_cast<CWinHttpRequest*>(a_userData);

	_ASSERT(l_req);

	l_req->_RunRequest();

	PostMessage(l_req->m_owner->m_hwndMessageOnlyWin, WM_WINHTTP_REQUEST_CALLBACK, l_req->GetReqId(), (LPARAM)l_req->m_owner.get());

	// request done, dispose of thread:
	_endthread();
}


LRESULT CALLBACK CWinHttpClient::MessageWindowProc(HWND hWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_WINHTTP_REQUEST_CALLBACK)
	{
		CWinHttpClient *l_client = reinterpret_cast<CWinHttpClient*>(lParam);
		int l_reqId = (int)wParam;

		if(l_client && (l_client->m_requests.find(l_reqId) != l_client->m_requests.end()))
		{
			// retrieve instance from our internal list:
			PWinHttpRequest l_req = l_client->m_requests[l_reqId];

			// then remove it:
			l_client->m_requests.erase(l_reqId);

			// finally invoke user defined callback:
			l_req->GetCallback()(l_req);

			// allow dangling client instances to go:
			l_req->m_owner.reset();

			// request will be freed here.
		}

		return 0;
	}
	else
	{
		return ::DefWindowProc(hWindow, uMsg, wParam, lParam);
	}
}


std::wstring CWinHttpClient::ExtractFileNameFromUrl(const std::wstring& a_url)
{
	std::wstring l_tmpUrl(a_url);
	CUtil::StrTrimRight(l_tmpUrl, L"/?");

	std::wstring l_fileName;

	// find file name part:
	std::wstring::size_type l_slashPos = l_tmpUrl.rfind(L'/');

	if(l_slashPos != std::wstring::npos)
	{
		std::wstring l_fileName = l_tmpUrl.substr(l_slashPos + 1);

		// find query part, remove if there is one:
		std::wstring::size_type l_queryPos = l_fileName.find(L'?');

		if(l_queryPos != std::wstring::npos)
		{
			l_fileName.erase(l_queryPos);
		}
	}

	if(l_fileName.empty())
	{
		// fall back to entire URL if a filename can't be extracted...
		l_fileName = a_url;
	}

	// remove everything besides [a-zA-Z0-9._-] from the file name:
	std::wstring l_fileClean;
	l_fileClean.reserve(l_fileName.size());

	for(std::wstring::size_type p = 0; p < l_fileName.size(); p++)
	{
		if(iswalnum(l_fileName[p]) ||
			l_fileName[p] == L'-' || l_fileName[p] == L'.' || l_fileName[p] == L'_')
		{
			if(l_fileClean.size() > 48)
			{
				l_fileClean.erase(0, 1);
			}

			l_fileClean += l_fileName[p];
		}
	}

	return l_fileClean;
}


CWinHttpClient::~CWinHttpClient()
{
	if(m_instanceReggedClass)
	{
		::UnregisterClass(WINHTTP_MESSAGE_ONLY_WINDOW_CLASSNAME, m_hInstance);
	}
}


/************************************************************************/
/* Helper: auto-freeing RAII buffer                                     */
/************************************************************************/

template<typename T> class CAutoFreeBuffer
{
public:
	CAutoFreeBuffer(size_t a_bufSize)
	{
		m_bufSize = a_bufSize;
		m_buf = new T[m_bufSize];
		memset(m_buf, 0, a_bufSize);
	}

	CAutoFreeBuffer(const CAutoFreeBuffer& a_from)
	{
		m_bufSize = a_from.m_bufSize;
		m_buf = new T[m_bufSize];
		memmove_s(m_buf, m_bufSize, a_from.m_buf, m_bufSize);
	}

	virtual ~CAutoFreeBuffer()
	{
		delete[] m_buf;
	}

	T* get() { return m_buf; }
private:
	T* m_buf;
	size_t m_bufSize;
};


/************************************************************************/
/* CWinHttpRequest implementation                                       */
/************************************************************************/

CWinHttpRequest::CWinHttpRequest(int a_reqId, shared_ptr<CWinHttpClient>& a_owner)
	: m_reqId(a_reqId), m_owner(a_owner), m_doingStuff(0),
	m_bypassCache(false), m_maxBuffer(1024 * 1024),
	m_downloadSucceeded(false), m_cancel(false),
	m_httpStatusCode(0)
{
	_ASSERT(::WinHttpCheckPlatform());

	// using one session per request for now:
	const std::wstring l_userAgent = a_owner->GetUserAgent();
	m_hSession = ::WinHttpOpen(l_userAgent.c_str(),
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS,
		0);

	_ASSERT(m_hSession != NULL);

	// note: we'll be doing synchronous WinHTTP requests here.
	// I know there's a native asynchronous mode in WinHTTP, but
	// it seems overly complex for this simple application as
	// it requires a fully thread safe callback handler and more
	// error checking. :P

	::WinHttpSetTimeouts(m_hSession, 5000, 10000, 300000, 300000);
}


// running in this request's thread:
void CWinHttpRequest::_RunRequest()
{
	URL_COMPONENTS l_urlComps = { sizeof(URL_COMPONENTS), 0 };
	std::wstring l_urlHost, l_urlPath, l_urlScheme;
	HINTERNET hConnect = NULL, hRequest = NULL;

	m_doingStuff = true;
	m_cancel = false;

	m_downloadSucceeded = false;

	// crack URL into components...
	l_urlComps.dwHostNameLength = (DWORD)-1;
	l_urlComps.dwUrlPathLength = (DWORD)-1;
	l_urlComps.dwSchemeLength = (DWORD)-1;
	if(!::WinHttpCrackUrl(m_url.c_str(), static_cast<DWORD>(m_url.size()), 0, &l_urlComps))
		goto RunRequest_Cleanup;

	// ... and store components in appropriate variables:
	l_urlHost = std::wstring(l_urlComps.lpszHostName, l_urlComps.dwHostNameLength);
	l_urlPath = std::wstring(l_urlComps.lpszUrlPath, l_urlComps.dwUrlPathLength);
	l_urlScheme = std::wstring(l_urlComps.lpszScheme, l_urlComps.dwSchemeLength);

	// set up connect handle:
	hConnect = ::WinHttpConnect(m_hSession, l_urlHost.c_str(), l_urlComps.nPort, 0);
	if(!hConnect)
		goto RunRequest_Cleanup;

	// set up request handle:
	hRequest = ::WinHttpOpenRequest(hConnect, L"GET", l_urlPath.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
		(m_bypassCache ? WINHTTP_FLAG_REFRESH : 0) | (!_wcsicmp(l_urlScheme.c_str(), L"https") ? WINHTTP_FLAG_SECURE : 0));
	if(!hRequest)
		goto RunRequest_Cleanup;

	// send request and wait for initial response:
	if(::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, NULL)
		&& ::WinHttpReceiveResponse(hRequest, NULL))
	{
		size_t l_fileSize = 0;

		// get file size from content-length header:
		{
			wchar_t szSizeBuffer[33] = {0};
			DWORD dwLengthSizeBuffer = 32;

			if(::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX,
				szSizeBuffer, &dwLengthSizeBuffer, WINHTTP_NO_HEADER_INDEX))
			{
				int64_t l_tmp = _wcstoi64(szSizeBuffer, NULL, 10);

				if(l_fileSize > 0 && static_cast<uint64_t>(l_fileSize) <= std::numeric_limits<size_t>::max())
				{
					l_fileSize = static_cast<size_t>(l_tmp);
				}
			}
		}

		// get response status code...
		{
			DWORD dwStatusCode = 0;
			DWORD dwStatusCodeBufSize = sizeof(DWORD);

			if(::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, 0,
				&dwStatusCode, &dwStatusCodeBufSize, WINHTTP_NO_HEADER_INDEX))
			{
				m_httpStatusCode = static_cast<int>(dwStatusCode);
			}
		}

		// retrieve and save to file, or buffer in memory string.
		// these two methods also set m_downloadSucceeded to true if applicable.
		if(!m_downloadFilePath.empty())
		{
			this->DownloadToFile(hRequest, l_fileSize);
		}
		else
		{
			this->DownloadToBuffer(hRequest, l_fileSize);
		}

		// hRequest will be closed below.
	}

RunRequest_Cleanup:
	if(hConnect) ::WinHttpCloseHandle(hConnect);
	if(hRequest) ::WinHttpCloseHandle(hRequest);

	m_doingStuff = false;
}


void CWinHttpRequest::DownloadToFile(HINTERNET hRequest, size_t a_contentLength)
{
	FILE *fFile = NULL;

	if(_wfopen_s(&fFile, m_downloadFilePath.c_str(), L"wb") != 0)
	{
		return;
	}

	DWORD l_available, l_read;
	bool l_gotEof = false;

	do
	{
		l_available = 0;

		if(!::WinHttpQueryDataAvailable(hRequest, &l_available))
			break;

		if(!l_available)
		{
			l_gotEof = true;
			break;
		}

		CAutoFreeBuffer<char> l_chunk(l_available + 1);

		if(!::WinHttpReadData(hRequest, l_chunk.get(), l_available, &l_read))
			break;

		if(l_read == 0)
		{
			l_gotEof = true;
			break;
		}

		if(fwrite(l_chunk.get(), l_read, 1, fFile) != 1)
		{
			break;
		}
	} while(l_available > 0 && !m_cancel);

	fclose(fFile);

	if(l_gotEof && !m_cancel)
	{
		m_downloadSucceeded = true;
	}
	else
	{
		::DeleteFile(m_downloadFilePath.c_str());
	}
}


void CWinHttpRequest::DownloadToBuffer(HINTERNET hRequest, size_t a_contentLength)
{
	if(a_contentLength > m_maxBuffer)
	{
		return;
	}

	std::string l_buf;
	DWORD l_available, l_read;
	bool l_gotEof = false;

	do 
	{
		l_available = 0;

		if(!::WinHttpQueryDataAvailable(hRequest, &l_available))
			break;

		if(!l_available)
		{
			l_gotEof = true;
			break;
		}

		if(l_buf.size() + l_available > m_maxBuffer)
		{
			break;
		}

		CAutoFreeBuffer<char> l_chunk(l_available + 1);

		if(!::WinHttpReadData(hRequest, l_chunk.get(), l_available, &l_read))
			break;

		if(l_read == 0)
		{
			l_gotEof = true;
			break;
		}

		if(lstrlenA(l_chunk.get()) == l_read)
		{
			l_buf += l_chunk.get();
		}
		else
		{
			// we got some binary stuff, but we don't want any.
			break;
		}

	} while(l_available > 0 && !m_cancel);

	if(l_gotEof)
	{
		m_buffer = l_buf;

		m_downloadSucceeded = true;
	}
}


CWinHttpRequest::~CWinHttpRequest()
{
	_ASSERT(!m_doingStuff);

	::WinHttpCloseHandle(m_hSession);
}
