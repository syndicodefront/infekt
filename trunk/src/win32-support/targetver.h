
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#ifdef _TARGETVER_VISTA
// yes I know about NTDDI_VERSION
// but I haven't figured out how to make it work without running
// into circular dependencies yet.
#define WINVER			0x0600
#define _WIN32_IE		0x0700
#else
// Windows 2000
#define WINVER			0x0500
#define _WIN32_IE		0x0600
#endif

#define _WIN32_WINNT	WINVER
#define _WIN32_WINDOWS	WINVER

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
