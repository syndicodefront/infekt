/**
 * Copyright (C) 2013 syndicode
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

#ifndef _WINFILEWATCHER_H
#define _WINFILEWATCHER_H

#include <Windows.h>
#include <functional>

// note: calling into this class from multiple threads is NOT safe!

typedef std::function<void ()> WinFileChangedCallback;

class CWinFileWatcher
{
public:
	CWinFileWatcher(WinFileChangedCallback a_callback);
	virtual ~CWinFileWatcher();

	void SetCallback(WinFileChangedCallback a_callback);
	void SetFile(const std::wstring& a_path);

	bool StartWatching();
	bool StopWatching();

protected:
	std::wstring m_filePath;
	WinFileChangedCallback m_callback;
	HANDLE m_hThreadEvent;
	HANDLE m_hStopEvent;
	bool m_watching;

	static void _WatchEventThread(void *);
	void WatchEventThread();

	uint64_t GetFileModificationTime();
};

#endif /* !_WINFILEWATCHER_H */
