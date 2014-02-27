set SILENT=y

:: Win32-Debug
set STATIC=n
set CONFIG=debug
set X64=n

call build.cmd

:: x64-Debug
set X64=y

call build.cmd

:: Win32-Release
set X64=n
set CONFIG=release

call build.cmd

:: Win32-Release-Static
set STATIC=y

call build.cmd

:: x64-Release
set STATIC=n
set X64=y

call build.cmd

PAUSE
