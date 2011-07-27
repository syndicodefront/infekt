@echo off

REM Requirements:
REM * mingw's msys in C:\msys\1.0\bin
REM * Visual Studio 2008
REM * curl in PATH

rem set CONFIG=release
set CONFIG=debug
set X64=n

IF EXIST zlib.tgz GOTO AZOK
curl http://zlib.net/zlib-1.2.5.tar.gz -o zlib.tgz
:AZOK

IF EXIST libpng.tgz GOTO LPZOK
curl ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng-1.5.4.tar.gz -o libpng.tgz
:LPZOK

IF EXIST pixman.tgz GOTO PZOK
curl http://www.cairographics.org/releases/pixman-0.22.2.tar.gz -o pixman.tgz
:PZOK

IF EXIST cairo.tgz GOTO CZOK
curl http://www.cairographics.org/releases/cairo-1.10.2.tar.gz -o cairo.tgz
:CZOK

set ROOTDIR=%cd%\work
set PATH=%PATH%;C:\msys\1.0\bin

IF %X64%==y GOTO SWITCHX64
call "%VS90COMNTOOLS%/vsvars32.bat"
set PLATFORM=Win32
GOTO SWITCHNOX64
:SWITCHX64
call "%VS90COMNTOOLS%\..\..\VC\bin\amd64\vcvarsamd64.bat"
set PLATFORM=x64
:SWITCHNOX64

IF EXIST %ROOTDIR% GOTO KILLROOTDIR
:MAKEROOTDIR
mkdir %ROOTDIR%
GOTO ROOTDIROK
:KILLROOTDIR
move %ROOTDIR% %ROOTDIR%.bak
rmdir /S /Q %ROOTDIR%.bak
sleep 1
IF EXIST %ROOTDIR% GOTO ROOTDIRDELFAIL
goto MAKEROOTDIR

:ROOTDIROK
cd %ROOTDIR%

tar xzf ../zlib.tgz
tar xzf ../libpng.tgz
tar xzf ../pixman.tgz
tar xzf ../cairo.tgz

move zlib-* zlib
move libpng-* libpng
move pixman-* pixman
move cairo-* cairo

IF %CONFIG%==debug GOTO SWITCHDEBUG
set DBD=
goto SWITCHDONE
:SWITCHDEBUG
set DBD=d
:SWITCHDONE

REM Fix ZLIB project file
cd %ROOTDIR%\libpng\projects\visualc71

sed "s/gzio.c\"^>/gzread.c\" \/><File RelativePath=\"..\\..\\..\\zlib\\gzwrite.c\" \/><File RelativePath=\"..\\..\\..\\zlib\\gzclose.c\" \/><File RelativePath=\"..\\..\\..\\zlib\\gzlib.c\">/" zlib.vcproj > zlib.vcproj.fixed
move /Y zlib.vcproj.fixed zlib.vcproj

REM Build ZLIB
vcbuild /upgrade zlib.vcproj

IF %X64%==n GOTO ZLIBNOX64
sed "s/Win32/x64/" zlib.vcproj > zlib.vcproj.fixed
move /Y zlib.vcproj.fixed zlib.vcproj
:ZLIBNOX64

IF %CONFIG%==debug GOTO ZLIBDEBUG
vcbuild zlib.vcproj "DLL Release"
GOTO ZLIBDONE
:ZLIBDEBUG
vcbuild zlib.vcproj "DLL Debug"
:ZLIBDONE

REM Build LIBPNG
cd %ROOTDIR%\libpng\projects\visualc71
vcbuild /upgrade libpng.vcproj
sed "s/  png_set_premultiply_alpha/; png_set_premultiply_alpha/" %ROOTDIR%\libpng\scripts\pngwin.def > %ROOTDIR%\libpng\scripts\pngwin.def.tmp
move /Y %ROOTDIR%\libpng\scripts\pngwin.def.tmp %ROOTDIR%\libpng\scripts\pngwin.def

sed "s/DataExecutionPrevention=\"0\"/AdditionalDependencies=\"zlib1%DBD%.lib\" AdditionalLibraryDirectories=\"$(OutDir)\\\\Zlib\"/" libpng.vcproj > libpng.vcproj.tmp
move /Y libpng.vcproj.tmp libpng.vcproj

IF %X64%==n GOTO LIBPNGNOX64
sed "s/Win32/x64/" libpng.vcproj > libpng.vcproj.fixed
move /Y libpng.vcproj.fixed libpng.vcproj
:LIBPNGNOX64

IF %CONFIG%==debug GOTO LIBPNGDEBUG
vcbuild libpng.vcproj "DLL Release"
GOTO LIBPNGDONE
:LIBPNGDEBUG
vcbuild libpng.vcproj "DLL Debug"
:LIBPNGDONE

REM Build Pixman
IF %X64%==n GOTO PIXMANNOX64
set MMX=off
rem Visual C/C++ does not support MMX operations for 64-bit processors in 64-bit mode
:PIXMANNOX64

cd %ROOTDIR%\pixman\pixman
IF %CONFIG%==debug GOTO PIXMANDEBUG
make -f Makefile.win32 "CFG=release"
GOTO PIXMANDONE
:PIXMANDEBUG
make -f Makefile.win32 "CFG=debug"
:PIXMANDONE

REM FINAL BUILD STAGE
set INCLUDE=%INCLUDE%;%ROOTDIR%\zlib
set INCLUDE=%INCLUDE%;%ROOTDIR%\libpng
set INCLUDE=%INCLUDE%;%ROOTDIR%\pixman\pixman
set INCLUDE=%INCLUDE%;%ROOTDIR%\cairo\boilerplate
set INCLUDE=%INCLUDE%;%ROOTDIR%\cairo\src

IF %CONFIG%==debug GOTO FINALLIBDEBUG
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Release\libpng15.* %ROOTDIR%\libpng
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Release\Zlib\zlib1.* %ROOTDIR%\zlib
GOTO FINALLIBDONE
:FINALLIBDEBUG
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Debug\libpng15d.* %ROOTDIR%\libpng
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Debug\Zlib\zlib1d.* %ROOTDIR%\zlib
:FINALLIBDONE

cd %ROOTDIR%\cairo

sed s/libpng\.lib/libpng15%DBD%.lib/ build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

sed s/zdll\.lib/zlib1%DBD%.lib/ build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

IF %CONFIG%==release GOTO SKIPDEBUGLIBFIX
sed "s/user32\.lib/user32.lib \/NODEFAULTLIB:msvcrt.lib/" build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common
:SKIPDEBUGLIBFIX

sed "s/D_CRT_NONSTDC_NO_DEPRECATE/D_CRT_NONSTDC_NO_DEPRECATE -DZLIB_DLL/" build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

sed "s/^if \([A-Z_]*\)$/ifeq ($(\1), 1)/" src\Makefile.sources > src\Makefile.fixed
move /Y src\Makefile.fixed src\Makefile.sources

REM we can toggle SVG support here but we MUST NOT remove these two lines
REM else the make file will fail to regenerate cairo-features.h
sed s/CAIRO_HAS_SVG_SURFACE=1/CAIRO_HAS_SVG_SURFACE=0/ build\Makefile.win32.features > build\Makefile.ffixed
move /Y build\Makefile.ffixed build\Makefile.win32.features

REM Build Cairo
IF %CONFIG%==debug GOTO CAIRODEBUG
make -f Makefile.win32 "CFG=release"
GOTO CAIRODONE
:CAIRODEBUG
make -f Makefile.win32 "CFG=debug"
:CAIRODONE

cd %ROOTDIR%\..

REM Copy results
IF EXIST out-%CONFIG%-%PLATFORM% GOTO BLAH
mkdir out-%CONFIG%-%PLATFORM%
:BLAH
IF EXIST include GOTO BLAH2
mkdir include
:BLAH2

del /Q out-%CONFIG%-%PLATFORM%\*
del /Q include\*

copy %ROOTDIR%\cairo\cairo-version.h include
copy %ROOTDIR%\cairo\src\cairo-features.h include
copy %ROOTDIR%\cairo\src\cairo.h include
copy %ROOTDIR%\cairo\src\cairo-deprecated.h include
copy %ROOTDIR%\cairo\src\cairo-win32.h include
copy %ROOTDIR%\cairo\src\cairo-ps.h include
copy %ROOTDIR%\cairo\src\cairo-pdf.h include
REM copy %ROOTDIR%\cairo\src\cairo-svg.h include

IF %CONFIG%==debug GOTO FINALCOPYDEBUG
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Release\libpng15.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Release\libpng15.lib out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Release\ZLib\zlib1.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Release\ZLib\zlib1.lib out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\cairo\src\release\cairo.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\cairo\src\release\cairo.lib out-%CONFIG%-%PLATFORM%
GOTO FINALCOPYDONE
:FINALCOPYDEBUG
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Debug\libpng15d.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Debug\libpng15d.lib out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Debug\libpng15d.pdb out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Debug\ZLib\zlib1d.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Debug\ZLib\zlib1d.lib out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\visualc71\%PLATFORM%_DLL_Debug\ZLib\zlib1d.pdb out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\cairo\src\debug\cairo.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\cairo\src\debug\cairo.lib out-%CONFIG%-%PLATFORM%
:FINALCOPYDONE

goto END

:ROOTDIRDELFAIL
echo Failed to delete root dir.


:END
echo.
echo.
PAUSE
