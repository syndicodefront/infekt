REM Copy deps for building

mkdir include
mkdir lib_x64_debug
mkdir lib_x64_release
mkdir lib_x86_debug
mkdir lib_x86_release
mkdir lib_x86_static

xcopy /Y ..\tools\cairo-maker\include\* include
xcopy /Y ..\tools\cairo-maker\out-debug-Win32\* lib_x86_debug
xcopy /Y ..\tools\cairo-maker\out-debug-x64\* lib_x64_debug
xcopy /Y ..\tools\cairo-maker\out-release-Win32\* lib_x86_release
xcopy /Y ..\tools\cairo-maker\out-release-x64\* lib_x64_release
xcopy /Y ..\tools\cairo-maker\out-release-Win32-static\* lib_x86_static

xcopy /Y ..\tools\pcre-maker\include\* include
xcopy /Y ..\tools\pcre-maker\out-debug-Win32\* lib_x86_debug
xcopy /Y ..\tools\pcre-maker\out-debug-x64\* lib_x64_debug
xcopy /Y ..\tools\pcre-maker\out-release-Win32\* lib_x86_release
xcopy /Y ..\tools\pcre-maker\out-release-x64\* lib_x64_release
xcopy /Y ..\tools\pcre-maker\out-release-Win32-static\* lib_x86_static
