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
#include "nfo_renderer_export.h"
#include "util.h"
#include "getopt.h"

#ifndef _WIN32
#define _T(STR) STR
#define _tprintf printf
#define _ftprintf fprintf
#define _tfopen_s fopen
#define _tstoi atoi
#endif

/************************************************************************/
/* DEFINE COMMAND LINE ARGUMENTS/OPTIONS                                */
/************************************************************************/

static const struct ::option g_longOpts[] = {
	{ _T("help"),			no_argument,		0,	'h' },
	{ _T("version"),		no_argument,		0,	'v' },

	{ _T("png"),			no_argument,		0,	'P' },
	{ _T("png-classic"),	no_argument,		0,	'p' },
	{ _T("utf-8"),			no_argument,		0,	'f' },
	{ _T("utf-16"),			no_argument,		0,	't' },
	{ _T("html"),			no_argument,		0,	'm' },
	{ _T("pdf"),			no_argument,		0,	'd' },
	{ _T("pdf-din"),		no_argument,		0,	'D' },
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
	printf("iNFekt: Renders almost any NFO file into a nice PNG image, with all the "
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
	printf("  -p, --png-classic           Prints the NFO file into a PNG file as text.\n");
	printf("  -f, --utf-8                 Converts the NFO file into UTF-8.\n");
	printf("  -t, --utf-16                Converts the NFO file into UTF-16.\n");
	printf("  -m, --html                  Makes a nice HTML document.\n");
#ifdef CAIRO_HAS_PDF_SURFACE
	printf("  -d, --pdf                   Makes a PDF document.\n");
	printf("  -D, --pdf-din               Makes a PDF document (DIN size).\n");
#endif
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

#define _CHECK_COLOR_OPT(CHAR, STRING_NAME, PROP_NAME, EXTRA_CODE) \
	case CHAR: \
	if(!CNFORenderer::ParseColor(::optarg, &l_color)) \
	{ \
		fprintf(stderr, "ERROR: Invalid or unsupported " STRING_NAME "."); \
		return 1; \
	} \
	EXTRA_CODE; \
	l_pngSettings.PROP_NAME = l_color; \
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
	bool l_classic = false, l_makePng = true, l_textUtf8 = true,
		l_htmlOut = false, l_makePdf = false, l_pdfDin = false;

#ifdef _WIN32
	CUtil::EnforceDEP();
	CUtil::HardenHeap();
	CUtil::RemoveCwdFromDllSearchPath();
#endif

	// our defaults:
	CNFORenderSettings l_pngSettings;
	l_pngSettings.bHilightHyperlinks = false;
	l_pngSettings.cTextColor = _S_COLOR_RGB(0, 0, 0);
	l_pngSettings.cBackColor = _S_COLOR_RGB(0xFF, 0xFF, 0xFF);
	l_pngSettings.cArtColor = l_pngSettings.cTextColor;
	l_pngSettings.cGaussColor = l_pngSettings.cArtColor;
	l_pngSettings.cHyperlinkColor = _S_COLOR_RGB(0, 0, 0xFF);
	l_pngSettings.uBlockHeight = 12;
	l_pngSettings.uBlockWidth = 7;
	l_pngSettings.bGaussShadow = true;
	l_pngSettings.uFontSize = 12;
	l_pngSettings.uGaussBlurRadius = 10;

	// keep track of changed stuff for advanced defaults:
	bool l_setBlockColor = false, l_setGlowColor = false;

	// Parse/process command line options:
	int l_arg, l_optIdx = -1;

	while((l_arg = getopt_long(argc, argv, _T("hvT:B:A:gG:W:H:R:LuU:O:pPftmdD"), g_longOpts, &l_optIdx)) != -1)
	{
		S_COLOR_T l_color;
		int l_int;

		switch(l_arg)
		{
		case 'h':
			OutputHelp();
			return 0;
		case 'v':
			printf("VERSION: iNFekt v%d.%d.%d\n", INFEKT_VERSION_MAJOR, INFEKT_VERSION_MINOR, INFEKT_VERSION_REVISION);
			printf("using Cairo v%d.%d.%d, PCRE v%d.%02d", CAIRO_VERSION_MAJOR, CAIRO_VERSION_MINOR, CAIRO_VERSION_MICRO, PCRE_MAJOR, PCRE_MINOR);
			return 0;
		case 'O':
			l_outFileName = ::optarg;
			break;
		_CHECK_COLOR_OPT('T', "text-color", cTextColor,);
		_CHECK_COLOR_OPT('B', "back-color", cBackColor,);
		_CHECK_COLOR_OPT('A', "block-color", cArtColor, l_setBlockColor = true);
		_CHECK_COLOR_OPT('G', "glow-color", cGaussColor, l_setGlowColor = true);
		_CHECK_COLOR_OPT('U', "link-color", cHyperlinkColor,);
		case 'g':
			l_pngSettings.bGaussShadow = false;
			break;
		case 'u':
			l_pngSettings.bUnderlineHyperlinks = false;
			break;
		case 'L':
			l_pngSettings.bHilightHyperlinks = true;
			break;
		case 'W':
			l_int = _tstoi(::optarg);
			if(l_int < 3 || l_int > 100)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported block-width.");
				return 1;
			}
			l_pngSettings.uBlockWidth = l_int;
			break;
		case 'H':
			l_int = _tstoi(::optarg);
			if(l_int < 3 || l_int > 170)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported block-height.");
				return 1;
			}
			l_pngSettings.uBlockHeight = l_int;
			break;
		case 'R':
			l_int = _tstoi(::optarg);
			if(l_int < 1 || l_int > 1000)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported glow-radius.");
				return 1;
			}
			l_pngSettings.uGaussBlurRadius = l_int;
			break;
		case 'P':
			l_makePng = true; l_classic = false;
			break;
		case 'p':
			l_makePng = l_classic = true;
			break;
		case 'f':
			l_makePng = false; l_textUtf8 = true;
			break;
		case 't':
			l_makePng = false; l_textUtf8 = false;
			break;
		case 'm':
			l_makePng = false; l_htmlOut = true;
			break;
		case 'd':
			l_makePng = false; l_makePdf = true;
			break;
		case 'D':
			l_makePng = false; l_makePdf = true; l_pdfDin = true;
			break;
		case '?':
		default:
			fprintf(stderr, "Try --help.");
			return 1;
		}
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

	// determine output file name if none has been given:
	if(l_outFileName.empty())
	{
		l_outFileName = l_nfoFileName;

		size_t l_pos = l_nfoFileName.rfind(_T(".nfo")); // case sensitive, hooray.
		if(l_pos != std::string::npos && l_pos == l_nfoFileName.size() - 4)
		{
			l_outFileName.erase(l_nfoFileName.size() - 4);
		}

		if(l_makePng)
		{
			l_outFileName += _T(".png");
		}
		else if(l_htmlOut)
		{
			l_outFileName += _T(".html");
		}
		else if(l_makePdf)
		{
			l_outFileName += _T(".pdf");
		}
		else if(l_textUtf8)
		{
			l_outFileName += _T("-utf8.nfo");
		}
		else
		{
			l_outFileName += _T("-utf16.nfo");
		}
	}

	if(l_makePng)
	{
		// Renderer instance that we are going to use:
		CNFORenderer l_renderer(l_classic);

		l_renderer.InjectSettings(l_pngSettings);

		if(!l_classic)
		{
			if(!l_setBlockColor)
			{
				l_renderer.SetArtColor(l_renderer.GetTextColor());
			}

			if(!l_setGlowColor && l_renderer.GetEnableGaussShadow())
			{
				l_renderer.SetGaussColor(l_renderer.GetArtColor());
			}
		}

		l_renderer.AssignNFO(&l_nfoData);

		// render!
		size_t l_imgWidth = l_renderer.GetWidth(), l_imgHeight = l_renderer.GetHeight();

		if(!l_imgWidth || !l_imgHeight)
		{
			fprintf(stderr, "ERROR: Unable to render an empty file.");
			return 1;
		}

		cairo_surface_t *l_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)l_imgWidth, (int)l_imgHeight);

		if(!l_surface)
		{
			fprintf(stderr, "ERROR: Unable to create an image surface (%u x %u)", l_imgWidth, l_imgHeight);
			return 1;
		}

		if(!l_renderer.DrawToSurface(l_surface, 0, 0, 0, 0, (int)l_imgWidth, (int)l_imgHeight))
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
	}
	else if(l_makePdf)
	{
#ifdef CAIRO_HAS_PDF_SURFACE
		CNFOToPDF l_exporter(l_classic);
		l_exporter.SetUseDINSizes(l_pdfDin);
		l_exporter.AssignNFO(&l_nfoData);
		l_exporter.InjectSettings(l_pngSettings);

		if(l_exporter.SavePDF(l_outFileName))
		{
			_tprintf(_T("Saved `%s` to `%s`!"), l_nfoFileName.c_str(), l_outFileName.c_str());
		}
		else
		{
			_ftprintf(stderr, _T("ERROR: Unable to write to `%s`."), l_outFileName.c_str());
		}
#endif
	}
	else
	{
		if(!l_htmlOut)
		{
			// text export

			if(l_nfoData.SaveToFile(l_outFileName, l_textUtf8))
			{
				_tprintf(_T("Saved `%s` to `%s`!"), l_nfoFileName.c_str(), l_outFileName.c_str());
			}
			else
			{
				_ftprintf(stderr, _T("ERROR: Unable to write to `%s`."), l_outFileName.c_str());
			}
		}
		else
		{
			// html export

			CNFOToHTML l_exporter(&l_nfoData);
			l_exporter.SetSettings(l_pngSettings);

			std::wstring l_html = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
			l_html += l_exporter.GetHTML();

			const std::string l_utf8 = CUtil::FromWideStr(l_html, CP_UTF8);

			FILE* l_file;
			if(_tfopen_s(&l_file, l_outFileName.c_str(), _T("wb")) == 0 && l_file)
			{
				fwrite(l_utf8.c_str(), l_utf8.size(), 1, l_file);
				fclose(l_file);

				_tprintf(_T("Saved `%s` to `%s`!"), l_nfoFileName.c_str(), l_outFileName.c_str());
			}
			else
			{
				_ftprintf(stderr, _T("ERROR: Unable to write to `%s`."), l_outFileName.c_str());
			}
		}
	}

	return 0;
}
