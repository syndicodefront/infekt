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

#include "stdafx.h"
#include "nfo_data.h"
#include "nfo_renderer.h"
#include "util.h"
#include "getopt.h"

#ifndef _WIN32
#define _T(STR) STR
#define _tprintf printf
#define _ftprintf fprintf
#else
#ifndef _UNICODE
#error This project requires unicode compiler settings.
#endif
#include <tchar.h>
#endif

/************************************************************************/
/* DEFINE COMMAND LINE ARGUMENTS/OPTIONS                                */
/************************************************************************/

static const struct option g_longOpts[] = {
	{ "help",			no_argument,		0,	'h' },
	{ "version",		no_argument,		0,	'v' },

	{ "text-color",		required_argument,	0,	't' },
	{ "back-color",		required_argument,	0,	'b' },
	{ "ascii-color",	required_argument,	0,	'a' },
	{ "no-glow",		no_argument,		0,	'g' },

	{0}
};

static void _OutputHelp(const char* a_exeNameA, const wchar_t* a_exeNameW)
{
	printf("iNFEKT: Renders almost any NFO file into a nice PNG image, with all the "
		"options you could ever imagine!\n\n");

	if(a_exeNameW)
		wprintf(L"USAGE: %s [options] <input-file.nfo>\n", a_exeNameW);
	else
		printf("USAGE: %s [options] <input-file.nfo>\n", a_exeNameA);

	printf("Available options:\n");
	printf("\t-h, --help                 List available command line options and exit.\n");
	printf("\t-v, --version              Output version information and exit.\n");

	printf("Render settings:\n");
	printf("\t-t, --text-color=<COLOR>   COLOR for regular text. Defaults to black.\n");
	printf("\t-b, --back-color=<COLOR>   Background COLOR. Defaults to white.\n");
	printf("\t-a, --ascii-color=<COLOR>  COLOR for ASCII art. Defaults to text-color.\n");
	printf("\t-g, --no-glow              Disable ASCII art glow effect. Defaults to On.\n");
}

#ifdef _WIN32
#define OutputHelp() _OutputHelp(NULL, wargv[0])
#else
#define OutputHelp() _OutputHelp(argv[0], NULL)
#endif

/************************************************************************/
/* main()                                                               */
/************************************************************************/

#ifdef _WIN32
int wmain(int argc, wchar_t* wargv[])
{
	char** argv = new char*[argc + 1];
	argv[argc] = NULL;

	// convert wargv to UTF-8 so we can use getopt and generally share more code.
	for(int i = 0; i < argc; i++)
	{
		const std::string l_utfStr = CUtil::FromWideStr(wargv[i], CP_UTF8);
		argv[i] = new char[l_utfStr.size() + 1]; // yup, we never delete[] these. leak, zomg!
		strcpy_s(argv[i], l_utfStr.size() + 1, l_utfStr.c_str());
	}
#else
int main(int argc, char* argv[])
{
#endif

	// Renderer instance that we are going to use:
	CNFORenderer l_renderer;

	// Parse/process command line options:
	int l_arg, l_optIdx = -1;

	while((l_arg = getopt_long(argc, argv, "hv", g_longOpts, &l_optIdx)) != -1)
	{
		switch(l_arg)
		{
		case 'h':
			OutputHelp();
			return 0;
		case 'v':
			printf("VERSION: iNFEKT v%d.%d.%d\n", INFEKT_VERSION_MAJOR, INFEKT_VERSION_MINOR, INFEKT_VERSION_REVISION);
			printf("using cairo v%d.%d.%d", CAIRO_VERSION_MAJOR, CAIRO_VERSION_MINOR, CAIRO_VERSION_MICRO);
			return 0;
		case '?':
		default:
			fprintf(stderr, "Try --help.");
			return 1;
		}
	}

	// stupid UNIX doesn't have Unicode APIs.
#ifdef _WIN32
	std::wstring l_nfoFileName, l_imgFileName;
#else
	std::string l_nfoFileName, l_imgFileName;
#endif

	// the file name has to be the last argument:
	if(::optind < argc)
	{
#ifdef _WIN32
		l_nfoFileName = wargv[optind];
#else
		l_nfoFileName = argv[optind];
#endif
	}

	if(l_nfoFileName.empty())
	{
		fprintf(stderr, "Missing argument: Please specify <input-file.nfo> or try --help");
		return 1;
	}

	// open+load the NFO file:
	CNFOData l_nfoData;

	if(!l_nfoData.LoadFromFile(l_nfoFileName))
	{
		fwprintf(stderr, L"ERROR: Unable to load NFO file: %s", l_nfoData.GetLastErrorDescription().c_str());
		return 1;
	}

	l_renderer.AssignNFO(&l_nfoData);

	// determine output file name if none has been given:
	if(l_imgFileName.empty())
	{
		l_imgFileName = l_nfoFileName;

		size_t l_pos = l_nfoFileName.rfind(_T(".nfo")); // case sensitive, hooray.
		if(l_pos != std::string::npos && l_pos == l_nfoFileName.size() - 4)
		{
			l_imgFileName.erase(l_nfoFileName.size() - 4);
		}

		l_imgFileName += _T(".png");
	}

	// render!
	size_t l_imgWidth = l_renderer.GetWidth(), l_imgHeight = l_renderer.GetHeight();

	if(!l_imgWidth || !l_imgHeight)
	{
		fprintf(stderr, "ERROR: Unable to render an empty file.");
		return 1;
	}

	cairo_surface_t *l_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, l_imgWidth, l_imgHeight);

	if(!l_surface)
	{
		fprintf(stderr, "ERROR: Unable to create an image surface (%u x %u)", l_imgWidth, l_imgHeight);
		return 1;
	}

	if(!l_renderer.DrawToSurface(l_surface, 0, 0, 0, 0,  l_imgWidth, l_imgHeight))
	{
		fprintf(stderr, "ERROR: Rendering failed.");
		cairo_surface_destroy(l_surface);
		return 1;
	}

#ifdef _WIN32
	const std::string l_utfOutFileName = CUtil::FromWideStr(l_imgFileName, CP_UTF8);
	if(cairo_surface_write_to_png(l_surface, l_utfOutFileName.c_str()) != CAIRO_STATUS_SUCCESS)
#else
	if(!cairo_surface_write_to_png(l_surface, l_imgFileName.c_str()) != CAIRO_STATUS_SUCCESS)
#endif
	{
		_ftprintf(stderr, _T("ERROR: Unable to write to `%s`."), l_imgFileName.c_str());
		cairo_surface_destroy(l_surface);
		return 1;
	}
	else
	{
		_tprintf(_T("Rendered `%s` to `%s`!"), l_nfoFileName.c_str(), l_imgFileName.c_str());
	}

	cairo_surface_destroy(l_surface);

	return 0;
}
