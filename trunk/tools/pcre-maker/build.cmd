@echo off

REM Requirements:
REM * mingw's msys in C:\msys\1.0\bin
REM * Visual Studio 2008
REM * curl in PATH

rem set CONFIG=release
set CONFIG=debug
set X64=y

IF EXIST pcre.tgz GOTO PCREOK
curl ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.11.tar.gz -o pcre.tgz
:PCREOK

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
set PARENT=%cd%
cd %ROOTDIR%

tar xzf ../pcre.tgz

move pcre-* pcre

IF %CONFIG%==debug GOTO SWITCHDEBUG
set DBD=
goto SWITCHDONE
:SWITCHDEBUG
set DBD=d
:SWITCHDONE

set PDIR=%ROOTDIR%\pcre
set BDIR=%ROOTDIR%\pcre\build\vc2008

mkdir %BDIR%
xcopy %PARENT%\build\* %BDIR%

copy %PDIR%\pcre.h.generic %BDIR%\pcre.h

cd %PDIR%
sed "s/@pcre_have_type_traits@/0/" pcre_stringpiece.h.in > .temp
sed "s/@pcre_have_bits_type_traits@/1/" .temp > %BDIR%\pcre_stringpiece.h

cat %BDIR%\config-win32.inc %PDIR%\config.h.generic > %BDIR%\config.h

REM Build dftables
cd %BDIR%

IF %CONFIG%==debug GOTO DFTABLESDEBUG
vcbuild dftables.vcproj "Release|%PLATFORM%"
GOTO DFTABLESDONE
:DFTABLESDEBUG
vcbuild dftables.vcproj "Debug|%PLATFORM%"
:DFTABLESDONE

IF %CONFIG%==debug GOTO PCREDEBUG
vcbuild pcre.vcproj "Release|%PLATFORM%"
GOTO PCREDONE
:PCREDEBUG
vcbuild pcre.vcproj "Debug|%PLATFORM%"
:PCREDONE

cd %PARENT%

REM Copy results
IF EXIST out-%CONFIG%-%PLATFORM% GOTO BLAH
mkdir out-%CONFIG%-%PLATFORM%
:BLAH

del /Q out-%CONFIG%-%PLATFORM%\*

IF %X64%==n GOTO FINALCOPYX86
xcopy "%BDIR%\x64\%CONFIG%\pcre%DBD%.dll" out-%CONFIG%-%PLATFORM%
xcopy "%BDIR%\x64\%CONFIG%\pcre%DBD%.lib" out-%CONFIG%-%PLATFORM%
GOTO FINALCOPYDONE
:FINALCOPYX86
xcopy "%BDIR%\%CONFIG%\pcre%DBD%.dll" out-%CONFIG%-%PLATFORM%
xcopy "%BDIR%\%CONFIG%\pcre%DBD%.lib" out-%CONFIG%-%PLATFORM%
:FINALCOPYDONE

goto END

:ROOTDIRDELFAIL
echo Failed to delete root dir.


:END
echo.
echo.
PAUSE
