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
#define _tstoi atoi
#endif

/************************************************************************/
/* DEFINE COMMAND LINE ARGUMENTS/OPTIONS                                */
/************************************************************************/

static const struct ::option g_longOpts[] = {
	{ _T("help"),			no_argument,		0,	'h' },
	{ _T("version"),		no_argument,		0,	'v' },

	{ _T("png"),			no_argument,		0,	'P' },
	{ _T("png-basic"),		no_argument,		0,	'p' },
	{ _T("utf-8"),			no_argument,		0,	'f' },
	{ _T("utf-16"),			no_argument,		0,	't' },
	{ _T("out-file"),		required_argument,	0,	'O' },

	{ _T("text-color"),		required_argument,	0,	'T' },
	{ _T("back-color"),		required_argument,	0,	'B' },
	{ _T("block-color"),	required_argument,	0,	'A' },
	{ _T("no-glow"),		no_argument,		0,	'g' },
	{ _T("glow-color"),		required_argument,	0,	'G' },
	{ _T("hilight-links"),	no_argument,		0,	'L' },
	{ _T("link-color"),		required_argument,	0,	'U' },
	{ _T("no-link-underl"),	no_argument,		0,	'u' },
	{ _T("block-width"),	required_argument,	0,	'W' },
	{ _T("block-height"),	required_argument,	0,	'H' },
	{ _T("glow-radius"),	required_argument,	0,	'R' },

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
	printf("  -h, --help                  List available command line options and exit.\n");
	printf("  -v, --version               Output version information and exit.\n");

	printf("Mode of operation:\n");
	printf("  -P, --png                   Renders a PNG (default).\n");
	printf("  -p, --png-basic             Prints the NFO file into a PNG file as text.\n");
	printf("  -f, --utf-8                 Converts the NFO file into UTF-8.\n");
	printf("  -t, --utf-16                Converts the NFO file into UTF-16.\n");
	printf("  -O, --out-file <PATH>       Sets the output filename. Defaults to input file name plus .png/.nfo.\n");

	printf("Render settings:\n");
	printf("  -T, --text-color <COLOR>    COLOR for regular text. Defaults to black.\n");
	printf("  -B, --back-color <COLOR>    Background COLOR. Defaults to white.\n");
	printf("  -A, --block-color <COLOR>   COLOR for ASCII art. Defaults to text-color.\n");
	printf("  -g, --no-glow               Disable ASCII art glow effect. Defaults to On.\n");
	printf("  -G, --glow-color <COLOR>    COLOR for glow effect. Defaults to block-color.\n");
	printf("  -L, --hilight-links         Highlight hyper links. Defaults to Off.\n");
	printf("  -U, --link-color <COLOR>    COLOR for hyper links. Defaults to blue.\n");
	printf("  -u, --no-link-underl        Disable underlining hyper links. Defaults to On.\n");
	printf("  -W, --block-width <PIXELS>  Block width. Defaults to 7.\n");
	printf("  -H, --block-height <PIXELS> Block Height. Defaults to 12.\n");
	printf("  -R, --glow-radius <PIXELS>  Glow effect radius. Defaults to 10.\n");

	// :TODO: option for input charset.
}

#ifdef _UNICODE
#define OutputHelp() _OutputHelp(NULL, argv[0])
#else
#define OutputHelp() _OutputHelp(argv[0], NULL)
#endif

#define _CHECK_COLOR_OPT(CHAR, STRING_NAME, METHOD_NAME, EXTRA_CODE) \
	case CHAR: \
	if(!CNFORenderer::ParseColor(::optarg, &l_color)) \
	{ \
		fprintf(stderr, "ERROR: Invalid or unsupported " STRING_NAME "."); \
		return 1; \
	} \
	EXTRA_CODE; \
	l_renderer.METHOD_NAME(l_color); \
	break;


/************************************************************************/
/* main()                                                               */
/************************************************************************/

#ifdef _UNICODE
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	std::_tstring l_outFileName;

	// Renderer instance that we are going to use:
	CNFORenderer l_renderer;

	// our defaults:
	l_renderer.SetTextColor(_S_COLOR_RGB(0, 0, 0));
	l_renderer.SetBackColor(_S_COLOR_RGB(0xFF, 0xFF, 0xFF));
	l_renderer.SetEnableGaussShadow(true);
	l_renderer.SetGaussBlurRadius(10);
	l_renderer.SetHilightHyperLinks(false);

	// keep track of changed stuff for advanced defaults:
	bool bSetBlockColor = false, bSetGlowColor = false;

	// Parse/process command line options:
	int l_arg, l_optIdx = -1;

	// :TODO: implement Ppft!

	while((l_arg = getopt_long(argc, argv, L"hvT:B:A:gG:W:H:R:LuU:O:pPft", g_longOpts, &l_optIdx)) != -1)
	{
		S_COLOR_T l_color;
		int l_int;

		switch(l_arg)
		{
		case 'h':
			OutputHelp();
			return 0;
		case 'v':
			printf("VERSION: iNFEKT v%d.%d.%d\n", INFEKT_VERSION_MAJOR, INFEKT_VERSION_MINOR, INFEKT_VERSION_REVISION);
			printf("using cairo v%d.%d.%d", CAIRO_VERSION_MAJOR, CAIRO_VERSION_MINOR, CAIRO_VERSION_MICRO);
			return 0;
		case 'O':
			l_outFileName = ::optarg;
			break;
		_CHECK_COLOR_OPT('T', "text-color", SetTextColor,);
		_CHECK_COLOR_OPT('B', "back-color", SetBackColor,);
		_CHECK_COLOR_OPT('A', "block-color", SetArtColor, bSetBlockColor = true);
		_CHECK_COLOR_OPT('G', "glow-color", SetGaussColor, bSetGlowColor = true);
		_CHECK_COLOR_OPT('U', "link-color", SetHyperLinkColor,);
		case 'g':
			l_renderer.SetEnableGaussShadow(false);
			break;
		case 'u':
			l_renderer.SetUnderlineHyperLinks(false);
			break;
		case 'L':
			l_renderer.SetHilightHyperLinks(true);
			break;
		case 'W':
			l_int = _tstoi(::optarg);
			if(l_int < 3 || l_int > 100)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported block-width.");
				return 1;
			}
			l_renderer.SetBlockSize(l_int, l_renderer.GetBlockHeight());
			break;
		case 'H':
			l_int = _tstoi(::optarg);
			if(l_int < 3 || l_int > 170)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported block-height.");
				return 1;
			}
			l_renderer.SetBlockSize(l_renderer.GetBlockWidth(), l_int);
			break;
		case 'R':
			l_int = _tstoi(::optarg);
			if(l_int < 1 || l_int > 1000)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported glow-radius.");
				return 1;
			}
			l_renderer.SetGaussBlurRadius(l_int);
			break;
		case '?':
		default:
			fprintf(stderr, "Try --help.");
			return 1;
		}
	}

	if(!bSetBlockColor)
	{
		l_renderer.SetArtColor(l_renderer.GetTextColor());
	}

	if(!bSetGlowColor && l_renderer.GetEnableGaussShadow())
	{
		l_renderer.SetGaussColor(l_renderer.GetArtColor());
	}

	std::_tstring l_nfoFileName;

	// the file name has to be the last argument:
	if(::optind < argc)
	{
		l_nfoFileName = argv[::optind];
	}

	if(l_nfoFileName.empty())
	{
		fprintf(stderr, "Missing argument: Please specify <input-file.nfo> or try --help");

#ifdef _WIN32
		/* for stupid people trying to double click infekt-cmd.exe */
		if(argc == 1)
		{
			printf("\n\nPress any key to continue...");
			_getch();
		}
#endif

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
	if(l_outFileName.empty())
	{
		l_outFileName = l_nfoFileName;

		size_t l_pos = l_nfoFileName.rfind(_T(".nfo")); // case sensitive, hooray.
		if(l_pos != std::string::npos && l_pos == l_nfoFileName.size() - 4)
		{
			l_outFileName.erase(l_nfoFileName.size() - 4);
		}

		l_outFileName += _T(".png");
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

#ifdef _UNICODE
	const std::string l_utfOutFileName = CUtil::FromWideStr(l_outFileName, CP_UTF8);
	if(cairo_surface_write_to_png(l_surface, l_utfOutFileName.c_str()) != CAIRO_STATUS_SUCCESS)
#else
	if(!cairo_surface_write_to_png(l_surface, l_outFileName.c_str()) != CAIRO_STATUS_SUCCESS)
#endif
	{
		_ftprintf(stderr, _T("ERROR: Unable to write to `%s`."), l_outFileName.c_str());
		cairo_surface_destroy(l_surface);
		return 1;
	}
	else
	{
		_tprintf(_T("Rendered `%s` to `%s`!"), l_nfoFileName.c_str(), l_outFileName.c_str());
	}

	cairo_surface_destroy(l_surface);

	return 0;
}
