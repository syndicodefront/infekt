/**
 * Copyright (C) 2013-2014 cxxjoe
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
#include "win_file_watcher.h"
#include "util.h" // for wrapped PathRemoveFileSpec, messes up stand-alone abilities


/************************************************************************/
/* CWinFileWatcher implementation                                        */
/************************************************************************/

CWinFileWatcher::CWinFileWatcher(WinFileChangedCallback a_callback) :
	m_callback(a_callback),
	m_watching(false)
{
	m_hStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CWinFileWatcher::SetCallback(WinFileChangedCallback a_callback)
{
	m_callback = a_callback;
}

void CWinFileWatcher::SetFile(const std::wstring& a_path)
{
	// changing the path of the watched file requires the thread
	// to be restarted.

	if(a_path != m_filePath)
	{
		bool bWasWatching = m_watching;

		if(m_watching)
		{
			StopWatching();
		}

		m_filePath = a_path;

		if(bWasWatching)
		{
			StartWatching();
		}
	}	
}

bool CWinFileWatcher::StartWatching()
{
	if(m_watching)
	{
		return true;
	}

	if(!m_callback || m_filePath.empty() || !::PathFileExists(m_filePath.c_str()))
	{
		return false;
	}

	::ResetEvent(m_hStopEvent);

	uintptr_t l_thread = _beginthread(_WatchEventThread, 0, this);

	if(l_thread != (uintptr_t)-1)
	{
		// wait for thread to launch:
		::WaitForSingleObject(m_hThreadEvent, INFINITE);

		return (m_watching = true);
	}

	return false;
}

bool CWinFileWatcher::StopWatching()
{
	if(!m_watching)
	{
		return false;
	}

	// signal:
	::SetEvent(m_hStopEvent);

	// wait for termination:
	DWORD dwEvent = ::WaitForSingleObject(m_hThreadEvent, 5000);

	m_watching = false;

	_ASSERT(dwEvent == WAIT_OBJECT_0);

	return (dwEvent == WAIT_OBJECT_0);
}

void CWinFileWatcher::WatchEventThread()
{
	HANDLE l_hEvents[2];
	bool bStop = false;
	std::wstring l_FolderPath = CUtilWin32::PathRemoveFileSpec(m_filePath);
	uint64_t l_lastModTime = GetFileModificationTime();
	
	::SetEvent(m_hThreadEvent); // thread has started.
	// placing this call here ensures that accessing m_filePath is locked.

	l_hEvents[0] = ::FindFirstChangeNotification(l_FolderPath.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
	l_hEvents[1] = m_hStopEvent;

	do
	{
		DWORD dwEvent = ::WaitForMultipleObjects(2, l_hEvents, FALSE, INFINITE);

		switch(dwEvent)
		{
		case WAIT_OBJECT_0: { // file may have changed
			uint64_t l_newTime = GetFileModificationTime();

			if(l_lastModTime != l_newTime && m_callback)
			{
				FILE *fh = NULL;
				unsigned int retry_count = 0;

				// wait up to two seconds to avoid access denied errors due to the file still being written:
				while(retry_count++ < 20 && NULL == (fh = _wfopen(m_filePath.c_str(), L"rb")))
				{
					if(!::PathFileExists(m_filePath.c_str()))
					{
						// File is gone!
						goto WATCH_BREAK;
					}

					::Sleep(100);
				}

				fclose(fh);

				m_callback();
			}

			l_lastModTime = l_newTime;

			::FindNextChangeNotification(l_hEvents[0]);
			break; }
		case WAIT_OBJECT_0 + 1: // stop has been requested
			bStop = true;
			break;
		case WAIT_FAILED:
			bStop = true;
		default: ;
			// wut?
		}
	} while(!bStop);

WATCH_BREAK:

	::FindCloseChangeNotification(l_hEvents[0]);

	::SetEvent(m_hThreadEvent); // thread stopping
}

uint64_t CWinFileWatcher::GetFileModificationTime()
{
	WIN32_FIND_DATA findData = {0};
	HANDLE hFind = ::FindFirstFile(m_filePath.c_str(), &findData);
	uint64_t iResult = 0;

	if(hFind != INVALID_HANDLE_VALUE)
	{
		// conversion as recommended on: http://msdn.microsoft.com/en-us/library/ms724284%28v=vs.85%29.aspx
		ULARGE_INTEGER tmp;
		tmp.HighPart = findData.ftLastWriteTime.dwHighDateTime;
		tmp.LowPart = findData.ftLastWriteTime.dwLowDateTime;

		iResult = tmp.QuadPart;

		::FindClose(hFind);
	}

	return iResult;
}

/*static*/ void CWinFileWatcher::_WatchEventThread(void *pUser)
{
	CWinFileWatcher *pInstance = reinterpret_cast<CWinFileWatcher*>(pUser);

	if(pInstance)
	{
		pInstance->WatchEventThread();
	}
}

CWinFileWatcher::~CWinFileWatcher()
{
	StopWatching();

	::CloseHandle(m_hStopEvent);
	::CloseHandle(m_hThreadEvent);
}
