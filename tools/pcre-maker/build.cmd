@echo off

REM Requirements:
REM * mingw's msys in C:\msys\1.0\bin
REM * Visual Studio 2010
REM * curl in PATH

rem set CONFIG=release
set CONFIG=debug
set X64=n
set STATIC=n

IF EXIST pcre.tgz GOTO PCREOK
curl ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.30.tar.gz -o pcre.tgz
:PCREOK

set ROOTDIR=%cd%\work
set PATH=%PATH%;C:\msys\1.0\bin

IF %X64%==y GOTO SWITCHX64
call "%VS100COMNTOOLS%vsvars32.bat"
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
sed "s/@pcre_have_bits_type_traits@/0/" .temp > %BDIR%\pcre_stringpiece.h

sed "s/@pcre_have_u\?long_long@/1/" pcrecpparg.h.in > %BDIR%\pcrecpparg.h

cat %BDIR%\config-win32-head.inc %PDIR%\config.h.generic %BDIR%\config-win32-tail.inc > %BDIR%\config.h

REM Build dftables
cd %BDIR%

vcupgrade dftables.vcproj
msbuild dftables.vcxproj /p:Platform=%PLATFORM% /p:Configuration=%CONFIG%

vcupgrade pcre.vcproj
IF %STATIC%==y GOTO PCRESTATICLIB
msbuild pcre.vcxproj /p:Platform=%PLATFORM% /p:Configuration=%CONFIG%
GOTO PCREDONE
:PCRESTATICLIB
msbuild pcre.vcxproj /p:Platform=%PLATFORM% /p:Configuration=%CONFIG% /p:ConfigurationType=StaticLibrary /p:RuntimeLibrary=MultiThreaded
:PCREDONE

cd %PARENT%

set OUTDIR=out-%CONFIG%-%PLATFORM%
IF %STATIC%==n GOTO OUTDIRSTATICOK
set OUTDIR=%OUTDIR%-static
:OUTDIRSTATICOK

REM Copy results
IF EXIST %OUTDIR% GOTO ODEXS
mkdir %OUTDIR%
:ODEXS

IF EXIST include GOTO INCDEXS
mkdir include
:INCDEXS

del /Q %OUTDIR%\*
del /Q include\*

xcopy %BDIR%\pcre.h include
xcopy %PDIR%\pcrecpp.h include
xcopy %BDIR%\pcrecpparg.h include
xcopy %BDIR%\pcre_stringpiece.h include

IF %X64%==n GOTO FINALCOPYX86
xcopy "%BDIR%\x64\%CONFIG%\pcre%DBD%.dll" %OUTDIR%
xcopy "%BDIR%\x64\%CONFIG%\pcre.lib" %OUTDIR%
GOTO FINALCOPYDONE
:FINALCOPYX86
xcopy "%BDIR%\%CONFIG%\pcre%DBD%.dll" %OUTDIR%
xcopy "%BDIR%\%CONFIG%\pcre.lib" %OUTDIR%
:FINALCOPYDONE

goto END

:ROOTDIRDELFAIL
echo Failed to delete root dir.


:END
echo.
echo.
PAUSE
