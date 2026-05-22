#pragma once

#define NOMINMAX

#define PSAPI_VERSION   1
#define NTDDI_VERSION   NTDDI_WIN10_RS1
#define WINVER			_WIN32_WINNT_WIN10

#define _WIN32_WINNT	WINVER
#define _WIN32_WINDOWS	WINVER

/* http://support.microsoft.com/kb/166474 */
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#define STRICT /* http://msdn.microsoft.com/en-us/library/aa383681%28VS.85%29.aspx */

#define CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES /* https://msdn.microsoft.com/en-us/library/bb288454.aspx */

#if (_WIN32_WINNT >= 0x600) && (_MSC_VER >= 1700) && !defined(COMPACT_RELEASE)
#define HAVE_AMP
#endif

#include <sdkddkver.h>
