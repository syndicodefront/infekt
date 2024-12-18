/**
 * Copyright (C) 2010-2012 syndicode
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

/************************************************************************/
/* DEFINE COMMAND LINE ARGUMENTS/OPTIONS                                */
/************************************************************************/

static const struct ::option g_longOpts[] = {
	{ _T("help"),			no_argument,		0,	'h' },
	{ _T("version"),		no_argument,		0,	'v' },

	{ _T("png"),			no_argument,		0,	'P' },
	{ _T("png-classic"),	no_argument,		0,	'p' },
	{ _T("text-only"),		no_argument,		0,	'S' },
	{ _T("utf-8"),			no_argument,		0,	'f' },
#ifdef _WIN32
	{ _T("utf-16"),			no_argument,		0,	't' },
#endif
	{ _T("cp-437"),			no_argument,		0,	'e' },
	{ _T("html"),			no_argument,		0,	'm' },
	{ _T("html-canvas"),	no_argument,		0,	'M' },
	{ _T("json"),			no_argument,		0,	'J' },
#ifdef CAIRO_HAS_PDF_SURFACE
	{ _T("pdf"),			no_argument,		0,	'd' },
	{ _T("pdf-din"),		no_argument,		0,	'D' },
#endif
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
	{ _T("compound-whitespace"), no_argument,	0,	'c' },
	{ _T("wrap"),			no_argument,		0,	'w' },

	{0}
};

static void _OutputHelp(const char* a_exeNameA, const wchar_t* a_exeNameW)
{
	printf("iNFekt: Renders almost any NFO file into a nice PNG image, with all the "
		"options you could ever imagine!\n\n");

	if (a_exeNameW)
		wprintf(L"USAGE: %s [options] <input-file.nfo>\n", a_exeNameW);
	else
		printf("USAGE: %s [options] <input-file.nfo>\n", a_exeNameA);

	printf("Available options:\n");
	printf("  -h, --help                  List available command line options and exit.\n");
	printf("  -v, --version               Output version information and exit.\n");

	printf("Mode of operation:\n");
	printf("  -P, --png                   Renders a PNG (default).\n");
	printf("  -p, --png-classic           Prints the NFO file into a PNG file as text.\n");
	printf("  -S, --text-only             Remove art blocks before rendering/saving.\n");
	printf("  -f, --utf-8                 Converts the NFO file into UTF-8.\n");
#ifdef _WIN32
	printf("  -t, --utf-16                Converts the NFO file into UTF-16.\n");
#endif
	printf("  -e, --cp-437                Save the NFO file as CP 437.\n");
	printf("  -m, --html                  Makes a nice HTML document.\n");
	printf("  -M, --html-canvas           Makes a HTML document where the NFO is rendered into a <canvas>.\n");
	printf("  -J, --json                  Exports the Render Grid for HTML <canvas> as JSON.\n");
#ifdef CAIRO_HAS_PDF_SURFACE
	printf("  -d, --pdf                   Makes a PDF document.\n");
	printf("  -D, --pdf-din               Makes a PDF document (DIN size).\n");
#endif
	printf("  -O, --out-file <PATH>       Output filename. Default: input name plus extension.\n");

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

	printf("Text conversion settings:\n");
	printf("  -c, --compound-whitespace   Add whitespace so that all lines have the same length.\n");
	printf("  -w, --wrap                  Wrap long lines.\n");

	// :TODO: option for input charset.
}

#ifdef _UNICODE
#define OutputHelp() _OutputHelp(nullptr, argv[0])
#else
#define OutputHelp() _OutputHelp(argv[0], nullptr)
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
		l_htmlOut = false, l_makePdf = false, l_pdfDin = false,
		l_htmlCanvas = false, l_jsonRenderGrid = false,
		l_textCp437 = false, l_compoundWhitespace = false,
		l_textOnly = false, l_wrap = false;

#ifdef _WIN32
	CUtilWin32::HardenHeap();
	CUtilWin32::RemoveCwdFromDllSearchPath();
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

	while ((l_arg = getopt_long(argc, argv, _T("hvT:B:A:gG:W:H:R:LuU:O:pPftmdDceSwMJ"), g_longOpts, &l_optIdx)) != -1)
	{
		S_COLOR_T l_color;
		int l_int;

		switch (l_arg)
		{
		case 'h':
			OutputHelp();
			return 0;
		case 'v':
			printf("VERSION: iNFekt v%d.%d.%d\n", INFEKT_VERSION_MAJOR, INFEKT_VERSION_MINOR, INFEKT_VERSION_REVISION);
			printf("using Cairo v%d.%d.%d\n", CAIRO_VERSION_MAJOR, CAIRO_VERSION_MINOR, CAIRO_VERSION_MICRO);
			return 0;
		case 'O':
			l_outFileName = ::optarg;
			break;
			_CHECK_COLOR_OPT('T', "text-color", cTextColor, );
			_CHECK_COLOR_OPT('B', "back-color", cBackColor, );
			_CHECK_COLOR_OPT('A', "block-color", cArtColor, l_setBlockColor = true);
			_CHECK_COLOR_OPT('G', "glow-color", cGaussColor, l_setGlowColor = true);
			_CHECK_COLOR_OPT('U', "link-color", cHyperlinkColor, );
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
			if (l_int < 3 || l_int > 100)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported block-width.\n");
				return 1;
			}
			l_pngSettings.uBlockWidth = l_int;
			break;
		case 'H':
			l_int = _tstoi(::optarg);
			if (l_int < 3 || l_int > 170)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported block-height.\n");
				return 1;
			}
			l_pngSettings.uBlockHeight = l_int;
			break;
		case 'R':
			l_int = _tstoi(::optarg);
			if (l_int < 1 || l_int > 1000)
			{
				fprintf(stderr, "ERROR: Invalid or unsupported glow-radius.\n");
				return 1;
			}
			l_pngSettings.uGaussBlurRadius = l_int;
			break;
		case 'P':
			l_makePng = true; l_classic = false;
			break;
		case 'S':
			l_textOnly = true; l_classic = true;
			break;
		case 'p':
			l_makePng = l_classic = true;
			break;
		case 'f':
			l_makePng = false; l_textUtf8 = true;
			break;
#ifdef _WIN32
		case 't':
			l_makePng = false; l_textUtf8 = false;
			break;
#endif
		case 'e':
			l_makePng = false; l_textCp437 = true;
			break;
		case 'm':
			l_makePng = false; l_htmlOut = true;
			break;
		case 'M':
			l_makePng = false; l_htmlOut = false; l_htmlCanvas = true;
			break;
		case 'J':
			l_makePng = false; l_htmlOut = false; l_htmlCanvas = false; l_jsonRenderGrid = true;
			break;
		case 'd':
			l_makePng = false; l_makePdf = true;
			break;
		case 'D':
			l_makePng = false; l_makePdf = true; l_pdfDin = true;
			break;
		case 'c':
			l_compoundWhitespace = true;
			break;
		case 'w':
			l_wrap = true;
			break;
		case '?':
		default:
			fprintf(stderr, "Try --help.\n");
			return 1;
		}
	}

	std::_tstring l_nfoFileName;

	// the file name has to be the last argument:
	if (::optind < argc)
	{
		l_nfoFileName = argv[::optind];
	}

	if (l_nfoFileName.empty())
	{
		fprintf(stderr, "Missing argument: Please specify <input-file.nfo> or try --help\n");

#ifdef _WIN32
		/* for stupid people trying to double click infekt-cmd.exe */
		if (argc == 1)
		{
			printf("\n\nPress any key to continue...");
			_getch();
		}
#endif

		return 1;
	}

	// open+load the NFO file:
	auto l_nfoData = std::make_shared<CNFOData>();
	l_nfoData->SetWrapLines(l_wrap && !l_textOnly);

	if (!l_nfoData->LoadFromFile(l_nfoFileName))
	{
		fwprintf(stderr, L"ERROR: Unable to load NFO file: %ls\n", l_nfoData->GetLastErrorDescription().c_str());
		return 1;
	}

	if (l_textOnly)
	{
		auto l_stripped = std::make_shared<CNFOData>();
		l_stripped->SetWrapLines(l_wrap);

		if (!l_stripped->LoadStripped(*l_nfoData))
		{
			fwprintf(stderr, L"ERROR: Unable to convert to text-only: %ls\n", l_nfoData->GetLastErrorDescription().c_str());
			return 1;
		}

		l_nfoData = l_stripped;
	}

	// determine output file name if none has been given:
	if (l_outFileName.empty())
	{
		l_outFileName = l_nfoFileName;

		size_t l_pos = l_nfoFileName.rfind(_T(".nfo")); // case sensitive, hooray.
		if (l_pos != std::string::npos && l_pos == l_nfoFileName.size() - 4)
		{
			l_outFileName.erase(l_nfoFileName.size() - 4);
		}

		if (l_makePng)
		{
			l_outFileName += _T(".png");
		}
		else if (l_htmlOut || l_htmlCanvas)
		{
			l_outFileName += _T(".html");
		}
		else if (l_jsonRenderGrid)
		{
			l_outFileName += _T(".json");
		}
		else if (l_makePdf)
		{
			l_outFileName += _T(".pdf");
		}
		else if (l_textCp437)
		{
			l_outFileName += _T("-dos.nfo");
		}
		else if (l_textUtf8)
		{
			l_outFileName += _T("-utf8.nfo");
		}
		else
		{
			l_outFileName += _T("-utf16.nfo");
		}
	}

	bool l_exportSuccess = false;

	if (l_makePng)
	{
		// Renderer instance that we are going to use:
		CNFOToPNG l_exporter(l_classic);

		l_exporter.InjectSettings(l_pngSettings);

		if (!l_classic)
		{
			if (!l_setBlockColor)
			{
				l_exporter.SetArtColor(l_exporter.GetTextColor());
			}

			if (!l_setGlowColor && l_exporter.GetEnableGaussShadow())
			{
				l_exporter.SetGaussColor(l_exporter.GetArtColor());
			}
		}

		l_exporter.AssignNFO(l_nfoData);

		l_exportSuccess = l_exporter.SavePNG(l_outFileName);
	}
	else if (l_makePdf)
	{
#ifdef CAIRO_HAS_PDF_SURFACE
		CNFOToPDF l_exporter(l_classic);
		l_exporter.SetUseDINSizes(l_pdfDin);
		l_exporter.AssignNFO(l_nfoData);
		l_exporter.InjectSettings(l_pngSettings);

		l_exportSuccess = l_exporter.SavePDF(l_outFileName);
#endif
	}
	else if (l_htmlOut || l_htmlCanvas || l_jsonRenderGrid)
	{
		std::string l_utf8;

		if (l_htmlOut)
		{
			CNFOToHTML l_exporter(l_nfoData);
			l_exporter.SetSettings(l_pngSettings);

			l_utf8 = CUtil::FromWideStr(l_exporter.GetHTML(), CP_UTF8);
		}
		else
		{
			CNFOToHTMLCanvas l_exporter;

			l_exporter.AssignNFO(l_nfoData);
			l_exporter.InjectSettings(l_pngSettings);

			if (l_htmlCanvas)
			{
				l_utf8 = l_exporter.GetFullHTML();
			}
			else if (l_jsonRenderGrid)
			{
				l_utf8 = l_exporter.GetRenderJSONString();
			}
		}

		FILE* l_file;
#ifdef _WIN32
		if (_tfopen_s(&l_file, l_outFileName.c_str(), _T("wb")) == 0 && l_file)
#else
		if (l_file = fopen(l_outFileName.c_str(), _T("wb")))
#endif
		{
			l_exportSuccess = (fwrite(l_utf8.c_str(), 1, l_utf8.size(), l_file) == l_utf8.size());
			fclose(l_file);
		}
	}
	else
	{
		// text export

		if (l_textCp437)
		{
			size_t l_inconvertible;

			l_exportSuccess = l_nfoData->SaveToCP437File(l_outFileName, l_inconvertible, l_compoundWhitespace);

			if (l_inconvertible > 0)
			{
				_ftprintf(stderr, _T("WARNING: %zd characters in NFO do not have a CP 437 equivalent and were dropped.\n"), l_inconvertible);
			}
		}
		else
		{
			l_exportSuccess = l_nfoData->SaveToUnicodeFile(l_outFileName, l_textUtf8, l_compoundWhitespace);
		}
	}

	if (l_exportSuccess)
	{
		_tprintf(_T("Saved `%s` to `%s`!\n"), l_nfoFileName.c_str(), l_outFileName.c_str());
	}
	else
	{
		_ftprintf(stderr, _T("ERROR: Unable to write to `%s`.\n"), l_outFileName.c_str());

		return 1;
	}

} /* end of main() */
