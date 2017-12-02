#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

/* we're still keeping XP compatibility for now.
http://msdn.microsoft.com/en-us/library/vstudio/jj851139.aspx
#if !defined(_TARGETVER_WIN7) && !defined(_TARGETVER_VISTA) && defined(_MSC_VER) && _MSC_VER >= 1700
#define _TARGETVER_VISTA
#endif*/

// we are however dropping WinXP 64-bit edition compatibility:
#if !defined(_TARGETVER_WIN8) && !defined(_TARGETVER_WIN7) && !defined(_TARGETVER_VISTA) && defined(_WIN64)
#define _TARGETVER_VISTA
#endif

#if defined(_TARGETVER_WIN8)
#define NTDDI_VERSION   NTDDI_WIN8
#define WINVER			_WIN32_WINNT_WIN8
#define _WIN32_IE		_WIN32_IE_WIN8
#elif defined(_TARGETVER_WIN7)
#define NTDDI_VERSION   NTDDI_WIN7
#define WINVER			_WIN32_WINNT_WIN7
#define _WIN32_IE		_WIN32_IE_IE80
#else
#define PSAPI_VERSION   1
#ifdef _TARGETVER_VISTA
#define NTDDI_VERSION   NTDDI_VISTASP1
#define WINVER			_WIN32_WINNT_VISTA
#define _WIN32_IE		_WIN32_IE_WIN6
#else
// Windows XP / Server 2003!
#define NTDDI_VERSION   NTDDI_WS03SP4
#define WINVER			_WIN32_WINNT_WS03
#define _WIN32_IE		_WIN32_IE_IE70
#endif
#endif

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
