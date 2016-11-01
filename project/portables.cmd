rem VARS

set VERSION=0.9.6
set ROOTDIR=%cd%\..\build-out
set DEPS=%cd%\..\dependencies

set PP32=%ROOTDIR%\iNFekt-32bit-Portable
set PP64=%ROOTDIR%\iNFekt-64bit-Portable
set PAP=%ROOTDIR%\iNFektPortable

rem PREPARE DIRS

move %PP32% %PP32%.bak
rmdir /S /Q %PP32%.bak
mkdir %PP32%

move %PP64% %PP64%.bak
rmdir /S /Q %PP64%.bak
mkdir %PP64%

copy /Y %ROOTDIR%\..\release\portable.ini %PP32%
copy /Y %ROOTDIR%\..\release\portable.ini %PP64%

xcopy /Y "%ROOTDIR%\..\release\*.url" %PP32%
xcopy /Y "%ROOTDIR%\..\release\*.url" %PP64%

rem COPY RUNTIME DLLS

xcopy /Y "%VS140COMNTOOLS%..\..\VC\redist\x86\Microsoft.VC140.OpenMP\*.dll" %PP32%
xcopy /Y "%VS140COMNTOOLS%..\..\VC\redist\x64\Microsoft.VC140.OpenMP\*.dll" %PP64%

rem COPY BUILT FILES

copy /Y %ROOTDIR%\Release\infekt-win32.exe %PP32%
copy /Y %ROOTDIR%\Release-x64\infekt-win64.exe %PP64%

copy /Y %ROOTDIR%\Release\infekt-cmd.exe %PP32%
copy /Y %ROOTDIR%\Release-x64\infekt-cmd.exe %PP64%

copy /Y %ROOTDIR%\Release\infekt-gpu.dll %PP32%
copy /Y %ROOTDIR%\Release-x64\infekt-gpu.dll %PP64%

rem COPY PLUGINS

mkdir %PP32%\plugins
mkdir %PP64%\plugins

copy /Y %ROOTDIR%\Release\plugins\rescene.dll %PP32%\plugins
copy /Y %ROOTDIR%\Release-x64\plugins\rescene.dll %PP64%\plugins

rem COPY DEPS

xcopy /Y "%DEPS%\lib_x86_release\*.dll" %PP32%
xcopy /Y "%DEPS%\lib_x64_release\*.dll" %PP64%

rem MAKE PORTABLEAPPS.COM THINGIE

move %PAP% %PAP%.bak
rmdir /S /Q %PAP%.bak
mkdir %PAP%
mkdir %PAP%\App\DefaultData
xcopy /Y /S /E %ROOTDIR%\..\release\PortableApps %PAP%

copy /Y %ROOTDIR%\..\release\portable.ini %PAP%\App\DefaultData

xcopy /Y "%VS140COMNTOOLS%..\..\VC\redist\x64\Microsoft.VC140.CRT\msvc*.dll" %PAP%\App\iNFekt
xcopy /Y "%VS140COMNTOOLS%..\..\VC\redist\x64\Microsoft.VC140.CRT\concrt*.dll" %PAP%\App\iNFekt
xcopy /Y "%VS140COMNTOOLS%..\..\VC\redist\x64\Microsoft.VC140.CRT\vcruntime*.dll" %PAP%\App\iNFekt
xcopy /Y "%VS140COMNTOOLS%..\..\VC\redist\x86\Microsoft.VC140.OpenMP\*.dll" %PAP%\App\iNFekt
xcopy /Y "%VS140COMNTOOLS%..\..\VC\redist\x86\Microsoft.VC140.CXXAMP\*.dll" %PAP%\App\iNFekt
copy /Y %ROOTDIR%\Release\infekt-win32.exe %PAP%\App\iNFekt
copy /Y %ROOTDIR%\Release\infekt-gpu.dll %PAP%\App\iNFekt
copy /Y %ROOTDIR%\Release\iNFektPortableLauncher.exe %PAP%\iNFektPortable.exe
xcopy /Y "%DEPS%\lib_x86_release\*.dll" %PAP%\App\iNFekt

mkdir %PAP%\App\iNFekt\plugins
copy /Y %ROOTDIR%\Release\plugins\rescene.dll %PAP%\App\iNFekt\plugins

copy /Y %ROOTDIR%\..\project\infekt-win32\iNFEKT_icon_by_railgun.ico %PAP%\App\AppInfo\appicon.ico

rem DELETE OLD RARS

del %ROOTDIR%\iNFekt-v%VERSION%-32bit-Portable.rar
del %ROOTDIR%\iNFekt-v%VERSION%-64bit-Portable.rar

rem NOW RAR!

cd %ROOTDIR%

rar a -k -m5 iNFekt-v%VERSION%-32bit-Portable.rar iNFekt-32bit-Portable
rar a -k -m5 iNFekt-v%VERSION%-64bit-Portable.rar iNFekt-64bit-Portable

rem DONE?!

PAUSE
