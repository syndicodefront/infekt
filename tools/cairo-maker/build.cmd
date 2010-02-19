@echo off

REM Requirements:
REM * mingw's msys in C:\msys\1.0\bin
REM * Visual Studio 2008
REM * curl in PATH

set CONFIG=release
rem set CONFIG=debug

IF EXIST zlib.tgz GOTO AZOK
curl http://www.zlib.net/zlib-1.2.3.tar.gz -o zlib.tgz
:AZOK

IF EXIST libpng.tgz GOTO LPZOK
curl ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng-1.4.0.tar.gz -o libpng.tgz
:LPZOK

IF EXIST pixman.tgz GOTO PZOK
curl http://www.cairographics.org/releases/pixman-0.17.6.tar.gz -o pixman.tgz
:PZOK

IF EXIST cairo.tgz GOTO CZOK
curl http://www.cairographics.org/releases/cairo-1.8.8.tar.gz -o cairo.tgz
:CZOK

set ROOTDIR=%cd%\work
set PATH=%PATH%;C:\msys\1.0\bin
call "%VS90COMNTOOLS%/vsvars32.bat"

rmdir /S /Q %ROOTDIR%
IF EXIST %ROOTDIR% GOTO ROOTDIRDELFAIL
mkdir %ROOTDIR%
cd %ROOTDIR%

tar xzf ../zlib.tgz
tar xzf ../libpng.tgz
tar xzf ../pixman.tgz
tar xzf ../cairo.tgz

move zlib-* zlib
move libpng-* libpng
move pixman-* pixman
move cairo-* cairo

REM Copy ZLIB project file
mkdir %ROOTDIR%\zlib\projects\visualc71
cd %ROOTDIR%\zlib\projects\visualc71
copy %ROOTDIR%\libpng\projects\visualc71\zlib.vcproj .

REM Build ZLIB
cd %ROOTDIR%\libpng\projects\visualc71
vcbuild /upgrade zlib.vcproj

IF %CONFIG%==debug GOTO ZLIBDEBUG
vcbuild zlib.vcproj "DLL Release"
set DBD=
GOTO ZLIBDONE
:ZLIBDEBUG
vcbuild zlib.vcproj "DLL Debug"
set DBD=d
:ZLIBDONE

REM Build LIBPNG
cd %ROOTDIR%\libpng\projects\visualc71
vcbuild /upgrade libpng.vcproj
sed "s/  png_set_premultiply_alpha/; png_set_premultiply_alpha/" %ROOTDIR%\libpng\scripts\pngwin.def > %ROOTDIR%\libpng\scripts\pngwin.def.tmp
move /Y %ROOTDIR%\libpng\scripts\pngwin.def.tmp %ROOTDIR%\libpng\scripts\pngwin.def

sed "s/DataExecutionPrevention=\"0\"/AdditionalDependencies=\"zlib1%DBD%.lib\" AdditionalLibraryDirectories=\"$(OutDir)\\\\Zlib\"/" libpng.vcproj > libpng.vcproj.tmp
move /Y libpng.vcproj.tmp libpng.vcproj

IF %CONFIG%==debug GOTO LIBPNGDEBUG
vcbuild libpng.vcproj "DLL Release"
GOTO LIBPNGDONE
:LIBPNGDEBUG
vcbuild libpng.vcproj "DLL Debug"
:LIBPNGDONE

REM Build Pixman
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
set LIB=%LIB%;%ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Release
set LIB=%LIB%;%ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Release\Zlib
GOTO FINALLIBDONE
:FINALLIBDEBUG
set LIB=%LIB%;%ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Debug
set LIB=%LIB%;%ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Debug\Zlib
:FINALLIBDONE

REM Patch Cairo
cd %ROOTDIR%
patch -p3 -u -l < ..\win32-cleartype-clipping.patch

cd %ROOTDIR%\cairo

sed s/libpng.lib/libpng14%DBD%.lib/ build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

sed s/zdll.lib/zlib1%DBD%.lib/ build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

IF %CONFIG%==release GOTO SKIPDEBUGLIBFIX
sed "s/user32.lib/user32.lib \/NODEFAULTLIB:msvcrt.lib/" build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common
:SKIPDEBUGLIBFIX

sed "s/D_CRT_NONSTDC_NO_DEPRECATE/D_CRT_NONSTDC_NO_DEPRECATE -DZLIB_DLL/" build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

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
IF EXIST out-%CONFIG% GOTO BLAH
mkdir out-%CONFIG%
:BLAH
IF EXIST include GOTO BLAH2
mkdir include
:BLAH2

del /Q out-%CONFIG%\*
del /Q include\*

copy %ROOTDIR%\cairo\cairo-version.h include
copy %ROOTDIR%\cairo\src\cairo-features.h include
copy %ROOTDIR%\cairo\src\cairo.h include
copy %ROOTDIR%\cairo\src\cairo-deprecated.h include
copy %ROOTDIR%\cairo\src\cairo-win32.h include
copy %ROOTDIR%\cairo\src\cairo-ps.h include
copy %ROOTDIR%\cairo\src\cairo-pdf.h include

IF %CONFIG%==debug GOTO FINALCOPYDEBUG
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Release\libpng14.dll out-%CONFIG%
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Release\libpng14.lib out-%CONFIG%
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Release\ZLib\zlib1.dll out-%CONFIG%
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Release\ZLib\zlib1.lib out-%CONFIG%
copy %ROOTDIR%\cairo\src\release\cairo.dll out-%CONFIG%
copy %ROOTDIR%\cairo\src\release\cairo.lib out-%CONFIG%
GOTO FINALCOPYDONE
:FINALCOPYDEBUG
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Debug\libpng14d.dll out-%CONFIG%
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Debug\libpng14d.lib out-%CONFIG%
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Debug\libpng14d.pdb out-%CONFIG%
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Debug\ZLib\zlib1d.dll out-%CONFIG%
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Debug\ZLib\zlib1d.lib out-%CONFIG%
copy %ROOTDIR%\libpng\projects\visualc71\Win32_DLL_Debug\ZLib\zlib1d.pdb out-%CONFIG%
copy %ROOTDIR%\cairo\src\debug\cairo.dll out-%CONFIG%
copy %ROOTDIR%\cairo\src\debug\cairo.lib out-%CONFIG%
:FINALCOPYDONE

goto END

:ROOTDIRDELFAIL
echo Failed to delete root dir.


:END
echo.
echo.
PAUSE
