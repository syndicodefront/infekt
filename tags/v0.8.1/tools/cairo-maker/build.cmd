@echo off

REM Requirements:
REM * mingw's msys in C:\msys\1.0\bin
REM * Visual Studio 2010
REM * curl in PATH

rem set CONFIG=release
set CONFIG=debug
set X64=n
set STATIC=n

IF EXIST zlib.tgz GOTO AZOK
curl http://zlib.net/zlib-1.2.5.tar.gz -o zlib.tgz
:AZOK

IF EXIST libpng.tgz GOTO LPZOK
curl ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng-1.5.5.tar.gz -o libpng.tgz
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
call "%VS100COMNTOOLS%\vsvars32.bat"
set PLATFORM=Win32
GOTO SWITCHNOX64
:SWITCHX64
call "%VS100COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
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

REM Build ZLIB and LIBPNG
cd %ROOTDIR%\libpng\projects\vstudio

sed "s/zlib-1....</zlib</" zlib.props > zlib.props.fixed
move /Y zlib.props.fixed zlib.props

IF %STATIC%==y GOTO ZLIBSTATICOK
sed "s/StaticLibrary/DynamicLibrary/" zlib/zlib.vcxproj > zlib.vcxproj.fixed
move /Y zlib.vcxproj.fixed zlib/zlib.vcxproj
:ZLIBSTATICOK

IF %X64%==n GOTO ZLIBNOX64
sed "s/Win32/x64/" vstudio.sln > vstudio.sln.fixed
sed "s/Win32/x64/" vstudio.sln.fixed > vstudio.sln.fixed2
move /Y vstudio.sln.fixed2 vstudio.sln

sed "s/Win32/x64/" zlib/zlib.vcxproj > zlib.vcxproj.fixed
move /Y zlib.vcxproj.fixed zlib/zlib.vcxproj
sed "s/Win32/x64/" libpng/libpng.vcxproj > libpng.vcxproj.fixed
move /Y libpng.vcxproj.fixed libpng/libpng.vcxproj
sed "s/<TargetMachine>MachineX86</<TargetMachine>MachineX64</" zlib/zlib.vcxproj > zlib.vcxproj.fixed
move /Y zlib.vcxproj.fixed zlib/zlib.vcxproj
sed "s/<TargetMachine>MachineX86</<TargetMachine>MachineX64</" libpng/libpng.vcxproj > libpng.vcxproj.fixed
move /Y libpng.vcxproj.fixed libpng/libpng.vcxproj

sed "s/Win32/x64/" pnglibconf/pnglibconf.vcxproj > pnglibconf.vcxproj.fixed
move /Y pnglibconf.vcxproj.fixed pnglibconf/pnglibconf.vcxproj
:ZLIBNOX64

rem zlib does not always export the gz* calls for some reason
sed "s/^\s*gz.*$/;\0/m" %ROOTDIR%\zlib\win32\zlib.def > zlib\zlib.def

sed "s/<SubSystem>/<ModuleDefinitionFile>zlib.def<\/ModuleDefinitionFile><SubSystem>/" zlib/zlib.vcxproj > zlib.vcxproj.fixed
move /Y zlib.vcxproj.fixed zlib/zlib.vcxproj

IF %CONFIG%==debug GOTO LIBPNGPDBOK
sed "s/<GenerateDebugInformation>true</<GenerateDebugInformation>false</" zlib/zlib.vcxproj > zlib.vcxproj.fixed
move /Y zlib.vcxproj.fixed zlib/zlib.vcxproj
sed "s/<GenerateDebugInformation>true</<GenerateDebugInformation>false</" libpng/libpng.vcxproj > libpng.vcxproj.fixed
move /Y libpng.vcxproj.fixed libpng/libpng.vcxproj
:LIBPNGPDBOK

IF %STATIC%==y GOTO LIBPNGSTATICLIB
msbuild vstudio.sln /p:Platform=%PLATFORM% /p:Configuration="%CONFIG%"
move %ROOTDIR%\libpng\projects\vstudio\x64\%CONFIG% %ROOTDIR%\libpng\projects\vstudio\%CONFIG%
GOTO LIBPNGSTATICDONE
:LIBPNGSTATICLIB
msbuild vstudio.sln /p:Platform=%PLATFORM% /p:Configuration="%CONFIG% Library" /p:ConfigurationType=StaticLibrary /p:RuntimeLibrary=MultiThreaded
mkdir %ROOTDIR%\libpng\projects\vstudio\%CONFIG%
move "%ROOTDIR%\libpng\projects\vstudio\x64\%CONFIG% Library\*" %ROOTDIR%\libpng\projects\vstudio\%CONFIG%
move "%ROOTDIR%\libpng\projects\vstudio\%CONFIG% Library\*" %ROOTDIR%\libpng\projects\vstudio\%CONFIG%
:LIBPNGSTATICDONE


xcopy %ROOTDIR%\libpng\projects\vstudio\%CONFIG%\libpng* %ROOTDIR%\libpng
xcopy %ROOTDIR%\libpng\projects\vstudio\%CONFIG%\zlib* %ROOTDIR%\libpng

REM Build Pixman
IF %X64%==n GOTO PIXMANNOX64
set MMX=off
rem Visual C/C++ does not support MMX operations for 64-bit processors in 64-bit mode
:PIXMANNOX64

cd %ROOTDIR%\pixman\pixman

sed "s/= -MD/= -MT/" build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

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

cd %ROOTDIR%\cairo

sed s/libpng\.lib/libpng15.lib/ build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

sed s/zlib\/zdll\.lib/libpng\/zlib.lib/ build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

rem IF %CONFIG%==release GOTO SKIPDEBUGLIBFIX
rem sed "s/user32\.lib/user32.lib \/NODEFAULTLIB:msvcrt.lib/" build\Makefile.win32.common > build\Makefile.fixed
rem move /Y build\Makefile.fixed build\Makefile.win32.common
rem :SKIPDEBUGLIBFIX

IF %STATIC%==n GOTO SKIPSTATICFIX
sed "s/CFG_CFLAGS := -MD/CFG_CFLAGS := -MT/" build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common
sed "s/user32\.lib/user32.lib \/NODEFAULTLIB:msvcrt.lib/" build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common
:SKIPSTATICFIX

sed "s/D_CRT_NONSTDC_NO_DEPRECATE/D_CRT_NONSTDC_NO_DEPRECATE -DZLIB_DLL/" build\Makefile.win32.common > build\Makefile.fixed
move /Y build\Makefile.fixed build\Makefile.win32.common

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

copy %ROOTDIR%\libpng\projects\vstudio\%CONFIG%\libpng15.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\vstudio\%CONFIG%\libpng15.lib out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\vstudio\%CONFIG%\zlib.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\vstudio\%CONFIG%\zlib.lib out-%CONFIG%-%PLATFORM%

copy %ROOTDIR%\cairo\src\%CONFIG%\cairo.dll out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\cairo\src\%CONFIG%\cairo.lib out-%CONFIG%-%PLATFORM%

IF %CONFIG%==release GOTO FINALCOPYDONE
rem copy PDB files
copy %ROOTDIR%\libpng\projects\vstudio\%CONFIG%\libpng15.pdb out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\libpng\projects\vstudio\%CONFIG%\zlib.pdb out-%CONFIG%-%PLATFORM%
copy %ROOTDIR%\cairo\src\debug\cairo.pdb out-%CONFIG%-%PLATFORM%
:FINALCOPYDONE

goto END

:ROOTDIRDELFAIL
echo Failed to delete root dir.


:END
echo.
echo.
PAUSE
