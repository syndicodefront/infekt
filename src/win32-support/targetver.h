
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#ifdef _TARGETVER_WIN7
// yes I know about NTDDI_VERSION
// but I haven't figured out how to make it work without running
// into circular dependencies yet.
#define WINVER			0x0700
#define _WIN32_IE		0x0800
#else
#ifdef _TARGETVER_VISTA
#define WINVER			0x0600
#define _WIN32_IE		0x0700
#else
// Windows XP / Server 2003!
#define WINVER			0x0501
#define _WIN32_IE		0x0600
#endif
#endif

#define _WIN32_WINNT	WINVER
#define _WIN32_WINDOWS	WINVER

/* http://support.microsoft.com/kb/166474 */
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#define STRICT /* http://msdn.microsoft.com/en-us/library/aa383681%28VS.85%29.aspx */
