/**
 * Copyright (C) 2011 syndicode
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

#ifndef _WINHTTPCLIENT_H
#define _WINHTTPCLIENT_H

#include <Windows.h>
#include <winhttp.h>
#include <functional>


// forward decl for the callback arguments:
class CWinHttpRequest;
typedef shared_ptr<CWinHttpRequest> PWinHttpRequest;
typedef std::function<void (PWinHttpRequest)> WinHttpRequestCallback;

// forward decl for owner member:
class CWinHttpClient;
typedef shared_ptr<CWinHttpClient> PWinHttpClient;

/**
 * Encapsulates an HTTP request. Allows options to be set and results to
 * be retrieved.
 * Instances of this class must be created through CWinHttpClient.
 **/
class CWinHttpRequest :
	public std::enable_shared_from_this<CWinHttpRequest>
{
	friend class CWinHttpClient;

public:
	virtual ~CWinHttpRequest();

	int GetReqId() const { return m_reqId; }
	void Cancel() { m_cancel = true; }
	bool Cancelled() const { return m_cancel; }

	void SetUrl(const std::wstring& a_url) { m_url = a_url; }
	std::wstring GetUrl() const { return m_url; }

	void SetCallback(WinHttpRequestCallback a_cb) { m_callback = a_cb; }
	WinHttpRequestCallback GetCallback() const { return m_callback; }

	void SetBypassCache(bool b) { m_bypassCache = b; }
	bool GetBypassCache() const { return m_bypassCache; }

	// if nobody calls this, the data will be saved to a string buffer
	// that can be retrieved via GetBufferContents().
	void SetDownloadFilePath(const std::wstring& a_path) { m_downloadFilePath = a_path; }
	const std::wstring& GetDownloadFilePath() const { return m_downloadFilePath; }

	void SetMaxBufferSize(size_t a_bytes) { m_maxBuffer = a_bytes; }
	size_t GetMaxBufferSize() const { return m_maxBuffer; }

	bool DidDownloadSucceed() const { return m_downloadSucceeded; }
	const std::string& GetBufferContents() const { return m_buffer; }
	int GetStatusCode() const { return m_httpStatusCode; }

protected:
	// to be used from within CWinHttpClient only:
	CWinHttpRequest(int a_reqId, shared_ptr<CWinHttpClient>& a_owner);
	void _RunRequest();

private:
	int m_reqId;
	PWinHttpClient m_owner;

	HINTERNET m_hSession;
	bool m_doingStuff;
	bool m_cancel;

	std::wstring m_url;
	WinHttpRequestCallback m_callback;
	bool m_bypassCache;
	std::wstring m_downloadFilePath;

	bool m_downloadSucceeded;
	int m_httpStatusCode;

	// buffer for non-file mode:
	std::string m_buffer;
	size_t m_maxBuffer;

	void DownloadToFile(HINTERNET hRequest, size_t a_contentLength);
	void DownloadToBuffer(HINTERNET hRequest, size_t a_contentLength);
};


/**
 * Provides a nice wrapper around WinHTTP's C API. Use this class
 * to create and launch HTTP requests (CWinHttpRequest).
 **/
class CWinHttpClient :
	public std::enable_shared_from_this<CWinHttpClient>
{
public:
	// you must instantiate this class from a thread that is running
	// a Windows message pump or your callbacks will never be invoked.
	CWinHttpClient(HINSTANCE a_hInstance);
	virtual ~CWinHttpClient();

	// use this to change the default user agent string:
	void SetUserAgent(const std::wstring& a_newAgent) { m_userAgent = a_newAgent; }
	std::wstring GetUserAgent() const { return m_userAgent; }

	PWinHttpRequest CreateRequest();
	// creates a request that will download a text file to an in-memory buffer.
	// will cancel/fail if the file contains \0 bytes (i.e. if it is a binary file),
	// or if it exceeds 1 MB.
	PWinHttpRequest CreateRequestForTextFile(const std::wstring& a_url, WinHttpRequestCallback a_callback);

	// fires off the request then returns immediately.
	// the callback will be invoked asynchronously.
	bool StartRequest(PWinHttpRequest& a_req);

	static std::wstring ExtractFileNameFromUrl(const std::wstring& a_url);

protected:
	int m_nextReqId;
	std::map<int, PWinHttpRequest> m_requests;

	std::wstring m_userAgent;

	// use a window in the thread that created this instance. we need
	// the window to receive synchronized(!) callbacks from our worker
	// threads (one worker thread per CWinHttpRequest).
	HINSTANCE m_hInstance;
	bool m_instanceReggedClass;
	HWND m_hwndMessageOnlyWin;

	static void RequestThreadMain(void *);
	static LRESULT CALLBACK MessageWindowProc(HWND, UINT, WPARAM, LPARAM);
};


#endif /* !_WINHTTPCLIENT_H */
