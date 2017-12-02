## v1.0.0 (???)
  * Fixed scrolling with Windows OS set to scroll by page instead of by x lines (issue #127)
  * Fixed malformed files from PNG export (issue #123)
  * Compiler upgrade to fix rendering in certain situations (issue #122)
  * Install statically linked version on Windows XP
  * Library update to cairo 1.15.8, libpng v1.6.34 (includes important bugfixes)

## v0.9.7 (January 29th 2017)
  * Windows default app handling: fixed/added support for Windows 8 and 10 changes (issue #114)
  * Compatibility: Use 8 spaces for tab characters (issue #116)
  * Bugfix: Fixed some issues with mixed line endings
  * Compatibility: Ignore invalid BOMs on some NFOs
  * Compatibility: Support some (otherwise valid NFO) files with null bytes
  * CLI version: fix --cp-437
  * Library update to cairo 1.14.8, libpng v1.6.28, zlib 1.2.11, boost 1.63
  * Minor updates to Linux version, PCRE and shim removal, use -std=c++14

## v0.9.6 (November 3rd 2016)
  * [re-uploaded setup routine after initial publication to fix broken downloads]
  * Switched J and K navigation keys
  * Added spacebar scrolling
  * New packaging: portable versions have less files, removed SuperCompact version
  * Switched to Visual C++ 2015, improved performance
  * Removed libpcre, using native C++ regex instead
  * Fixed issue 112 (ignore unterminated escape sequences in ANSI files)
  * Library update to cairo 1.14.6, libpng v1.6.26
  * Impaired Linux compatibility, to be fixed

## v0.9.5 (March 14th 2015)
  * Fix for crash introduced with 0.9.4, sorry

## v0.9.4 (March 13th 2015)
  * URLs for homepage and auto-update adjusted
  * Line wrap: relaxed max. columns to 90
  * Support for double-CP437-encoded NFOs that were additionally encoded as UTF-8 afterwards (see issue 104)
  * Default "Escape key quits program" to true (for new installs)
  * Some changes to Windows version display in about dialog
  * Library update to cairo 1.14.2, libpng 1.6.16, libpcre 8.36
  * Linux build fixed

## Project moved from Google Code to GitHub (March 13th 2015)

## v0.9.3 (September 9th 2014)
  * Added missing plugin(s) to portable download
  * Fixed some false positives in hyperlink detection (issue 97)
  * Fixed issue 103 (CR-only NFOs)
  * Added --text-only and --wrap options to infekt-cmd
  * pixman + libpng library update

## v0.9.2 (June 1st 2014)
  * Fixed a redrawing issue (opaque rectangles when zooming)
  * Improved detection of mail addresses to avoid false positives
  * Fixed Ctrl+G shortcut (toggles toolbar)

## v0.9.0 (May 31st 2014)
  * Enabled plugin selection list in the preferences
  * First plugin: ReScene - it can read .srr files and show the first embedded .nfo in iNFekt - issue 93
  * Fixed handling of files that are deleted or moved while being viewed
  * Tweaked handling of IMDb links a bit - issue 94
  * Added support for CRCRLF line ending sequence ("Windows XP notepad word wrap bug") - issue 92
  * Added "auto-adjust window height" feature (default off) - issue 95
  * Moved around some options in the preferences dialogs
  * Changed line-wrap behavior to only wrap if less than 50% of all lines are affected. Not perfect for really short NFOs, but great for NFOs with huge art blocks that exceed 80 chars.
  * PCRE update to 8.35
  * libpng update to 1.6.10
  * Minor fixes and improvements

## v0.8.14 (February 27th 2014)
  * Improved shell integration (preview and thumbnails) - issue 44 / issue 89
  * Greatly improved ANSI support (only one known remaining bug left)
  * New setting to enable/disable the glow effect on ANSI art (GPU only, not supported by CPU fallback right now)
  * Browse mode: crash fix
  * Line wrap setting is now a global setting (destroys some really weird text-only NFOs, but you can turn it off) (issue 66)
  * File watcher: wait up to two seconds before reloading if file cannot be opened immediately

## v0.8.12 (February 15th 2014)
  * Happy fifth year with iNFekt!
  * New: First draft version of ANSI art support (colors!) - only in rendered mode so far
  * New: When displaying an NFO file with "on-demand rendering", render remaining parts of the NFO in background. This avoids some lags that  appeared with slow GPUs. If this crashes for you, please temporarily turn off "on-demand rendering" in the preferences.
  * Fixed GPU acceleration not being active despite enabled setting
  * Browse mode (J/K keys): fixed problem where NFOs were not navigated in the correct order
  * Browse mode: display current NFO index in status bar
  * Browse mode: avoid some flickering when window size changes
  * Fixed Windows version display in about dialog (issue 86)
  * Fixed scrollbar sometimes not being updated when opening an NFO
  * Always clear text selection after text has been copied to clipboard
  * Usual round of optimizations, reduced memory usage
  * PCRE update to 8.34 (now with PCRE16)
  * VS2012 update to 2012.U4
  * Over 80 commits in total, most for ANSI colors support

## v0.8.10 (October 2nd 2013)
  * PDF export is working again (issue 84)
  * Added CP437 strict mode (issue 83)
  * Fixed 0x9B character not showing up (issue 83)
  * Auto-resize window (if enabled) when resetting zoom
  * Only load NFOs with UTF-8 signature if they are actually valid UTF-8, else try other encodings
  * Some internal fixes and changes to NFO loading & decoding code
  * libpng update (1.6.6)
  * cairo and pixman updates (1.12.16 and 0.30.2 respectively)

## v0.8.9 (July 8th 2013)
  * More fixes & improvements to hyperlink detection
  * New: Turn email addresses into clickable links
  * PCRE update to 8.33
  * boost lib update to 1.54, VS2012 update to 2012.U3, minor stuff

## v0.8.8 (May 14 2013)
  * Fixes and improvements to hyperlink detection
  * New: Support + detect yet another way of broken encodings that NFOs out there have. It's double-encoded, partly-transliterated CP437.
  * New: It's possible to try force-loading an NFO with one of the broken encoding workarounds from the charset right-click menu.
  * Fixed an issue where the last character was missing from a wrapped line in text view
  * zlib (1.2.8), libpng (1.6.2) and pixman (0.30.0) update

## v0.8.7 (April 14 2013)
  * Attention: Due to a bug in all previous versions' auto updater, it's not possible to auto-update to this version. Please download & install the new version manually.
  * New feature: iNFekt automatically reloads the currently viewed file if it changes on disk
  * New feature: Export NFOs as CP 437 (informs about nonconvertible characters, too)
  * New: Optionally associate .diz files during setup (issue 73)
  * Tweaked detection to match some more broken NFOs and display them correctly
  * Fixed a problem where some links in consecutive lines would not be recognized correctly
  * Fixed weird-looking transparent PNG export
  * Fixed some artifacts in Rendered Mode that were introduced with 0.8.5
  * Fixed PortableApps.com wrapper ignoring command line arguments (issue 76)
  * Command line version (infekt-cmd) can now export PNGs > 32768 px height as well
  * Added some command line arguments to main iNFekt program (try --help) - issue 74
  * Got rid of all CUDA/NViDiA specific code and files
  * Made the blur effect use C++ AMP - also supports ATI hardware now, but only on Windows Vista, 7 and 8
  * Status of GPU acceleration (used yes/no) is now shown in About dialog (F1)
  * Needs Visual C++ 2012 runtime DLLs (previously 2010), installer will download if necessary, shipped with portable versions
  * Usual round of performance improvements, bug fixes and mini improvements
  * PCRE update to 8.32, cairo update to 1.12.14, pixman update to 0.28.2
  * Attention: The 64-bit build for this and all following versions will no longer work on Windows XP 64-bit (32-bit is fine)

## v0.8.5 (June 9 2012)
  * New feature: Browse all NFO files in a folder using the J and K keys
  * New feature: Ctrl + F to search
  * New feature: Toolbar can be hidden if desired (Ctrl + G or right-click menu, use Alt to get menu bar)
  * Fixed issue 69: non-latin characters in hyperlinks are now supported
  * Window auto-width option now defaults to On
  * NViDiA CUDA H/W acceleration (for Glow Effect) is now even faster
  * Loading NFO files is now faster
  * Rendering (displaying) NFOs is a lot faster, especially for big files (ref: issue 65)
  * Raised file limits to 3 MB and 10000 lines
  * Added hidden context menu (right click charset pane in status bar) where users can force a charset to be used instead of having it auto-detected.
  * New options "Enable NVIDIA CUDA" (hardware acceleration) and "On-demand rendering" (reduces initial time required to load an NFO but may freeze for short periods of time when scrolling down). Both default to On.
  * Rendering files without any block art is now faster
  * Updated PCRE to 8.30 with UCP, Cairo to 1.12.2, libpng to 1.5.10, zlib to 1.2.7
  * Some internal changes + improvements
  * New download: PortableApps.com edition

## v0.8.1 (26th September 2011)
  * Fixed issue 59
  * Setup/installer fix: don't fail to register shell extension if MSVC++ 2010 runtimes have NOT been installed (this also resolves some auto-update issues)
  * Setup/installer fix: correctly remove old libpng14.dll
  * Auto-updater: Better user feedback on problems
  * All known auto-update issues should be resolved with this build (at least when upgrading from 0.8.0 to 0.8.1)
  * Updated libpng to 1.5.5 and PCRE to 8.13

## v0.8.0 (16th September 2011)
  * New _Super Compact Version_ (no CUDA H/W acceleration, no plugins, one EXE file without dependencies + configuration INI only)
  * Parallelized CPU-based blur computation using OpenMP (performance improvement for non-Nvidia GPU users)
  * Added "(portable)" to Window caption when running in portable mode
  * Fixed issue 57 (wrong URL in first part of multiline hyper links)
  * Fixed issue 58 (delay/spinning up disks on program start-up)
  * Resolved issue 54: Added a menu entry to hide/show the status bar
  * Improved Windows Server 2003 compatibility
  * Dropped Windows 2000 support
  * Using Microsoft Visual C++ 2010 compiler and runtime from now on
  * The installer checks whether the runtimes are installed, downloads & installs them otherwise
  * The portable versions ship with copies of the runtime DLLs, which makes them quite a bit larger
  * Misc fixes and improvements

## v0.7.9 (04 March 2011)
  * Last version that runs on Windows 2000
  * Enabled auto update to actually work when the next version comes out... d'oh.

## v0.7.8 (04 March 2011)
  * New: Making mouse movements while selecting text scroll the viewer window
  * Fixed issue 51 (Window going off-screen to the right)
  * Fixed issue 49 (Window not being brought to the front in single-instance mode)
  * Fixed "Highlight hyper links" setting not being respected in Rendered View
  * Enhanced support for more broken Unicode NFO files
  * Implemented auto-update functionality so when the next release is due, you'll (hopefully) be able to update iNFekt with very few clicks.
  * Updated cairo (1.10.2) and PCRE (8.12)
  * Misc code changes in preparation of Linux/GTK+ and Mac OS versions

## v0.7.6 (13th November 2010)
  * Added a bunch of new, advanced settings: _Auto-center window on startup_, _Auto-adjust window width to NFO_, _Use centered NFO view_
  * Added a show/hide menu button to the main toolbar
  * Fixed another minor redrawing bug that caused artifacts from overlapping windows to remain visible
  * Fixed weird zoom results in Classic Mode
  * Added "support" for another way of totally broken NFO encodings as seen in the wild
  * Added ~ to allowed characters in hyperlink detection
  * Added a --compound-whitespace option to infekt-cmd
  * iNFekt now looks a bit better with Aero enabled (Windows Vista + 7)
  * Removed ability to change main menu position
  * Improved support for multiple monitors
  * Fixed copy-on-select
  * Vista and 7: Using new "Open File" and "Save File" dialogs
  * v0.7.5 was originally released, but didn't work on Windows XP

## v0.7.2 (25th September 2010)
  * Fix for minor redrawing issue (background-colored boxes)

## v0.7.1 (24th September 2010)
  * Fixed wrong line-wrap default in Rendered and Classic mode
  * Updated cairo, pixman and libpng
  * Minor speedups
  * Fixed setup/installer version info

## v0.7 (5th September 2010)
  * Native 64-bit version
  * Shell integration on Vista and 7: NFO file thumbnails and (basic) preview pane support
  * Real portable mode support by using an INI file (portable.ini) instead of the Registry
  * Hyperlink-and-text-on-the-same-line bugfixes (e.g. issue 29)
  * CUDA SDK is now at version 3.1. This means that you need a more recent NViDiA driver to take advantage of CUDA acceleration.
  * Fixed issue 35 (crash/hang)
  * Implemented don't-remember-MRU option
  * Implemented line-wrap option for text-only view
  * About dialog: Show 32/64 bit info and OS version
  * Hardened app against "recent" DLL loading forgeries ("binary planting")
  * Updated pixman and libpng
  * Other fixes & improvements

## v0.6 (4th July 2010)
  * Made text output in Rendered View a lot faster (also improves zoom speed)
  * Fixed bug where a cleared open MRU list was not being saved
  * Fixed time zone of "last modified" timestamp in the status bar
  * Added F5, Ctrl+R and View Menu/Reload triggers to reload the current NFO file from disk
  * Activated DEP for XP SP3
  * Updated PCRE, pixman and zlib
  * Other fixes & improvements

## v0.5 (23 March 2010)
  * Added zoom functionality (speed will be improved later)
  * Added useful file information to the status bar
  * Added ability to export and import view settings from the preferences
  * Implemented PDF export
  * Implemented "Copy Shortcut" context menu entry
  * Fixed multiple crashes related to changing settings
  * Fixed anti-alias setting not being saved
  * Improved CUDA code
  * Other fixes & improvements

## v0.3 (12 March 2010)
  * Added support for multiple hyperlinks in one line
  * Made the blur/shadow effect a little bit faster for small radii and a lot faster for big radii. In fact, the blur radius does no longer affect the rendering speed at all (on/off still does).
  * Sick stuff: The blur/shadow effect now uses NVIDIA CUDA where available!
  * Added drag & drop support so you can drop .nfo files from explorer.exe etc. onto the iNFekt window.
  * Implemented XHTML export
  * Added "Re-use Viewer window (single instance mode)" option
  * Added list of recently opened files to the Open toolbar button
  * Fixed incorrect window caption when a file was opened via command line/double click
  * Other fixes & improvements

## v0.2 (7 March 2010)
  * Implemented font size (Classic + Text-Only View) setting
  * Implemented font face setting
  * Implemented "Show Menubar on Startup" setting
  * Implemented "Copy on Select" setting + functionality
  * Implemented Shell Integration as default NFO viewer
  * Added support for NFOs with ANSI Escape Codes
  * Added support for broken NFOs with \n\n newlines
  * Narrow NFOs in wide windows are centered
  * Added View Mode buttons to the toolbar...
  * Fixed an issue where the text selection would not be recognized for Ctrl+C when the text had been selected from bottom to top
  * Fixed issue 7, issue 11 and issue 13
  * Fixed issue that "forgot" the last line of files
  * Fixed nasty Windows 2000 / Common Control 5 Combo Box bug
  * Other fixes & improvements
  * Updated dependencies: cairo 1.8.10, pixman, libpng
  * Made an optional installer

## v0.1.6 (24 Feb 2010)
  * Implemented block size (Rendered view) setting
  * Added "Default View Mode" setting
  * Added Anti-Alias setting
  * Implemented Preview button
  * Implemented Always on Top setting
  * Slightly better UI experience on Windows Vista and 7
  * Fixed bugs
  * Other improvements
  * Fixed HUGE memory leak

## v0.1.5 (23 Feb 2010)
  * Added awesome icon by Railgun! THANKS!!
  * Added Classic view (copying text is still buggy)
  * Added very buggy raw text view
  * Added Update Check
  * New About dialog
  * Added font selection to the dialog, but it's *not working yet*!
  * Fixed bugs
  * Fixed icons on Windows 2000
  * Other improvements

## v0.1.0 (17 Feb 2010)
  * Initial public release of Win32 binaries

## 25 Jan 2010
  * Initial source code import

## 31 Dec 2009
  * Work begun
