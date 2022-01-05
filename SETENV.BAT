@echo off
echo LBA Build Environment
call C:\Tools\WATCOM\owsetenv.bat
PATH C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.30.30705\bin\Hostx64\x86;%PATH%
SET LIB386_PATH=%CD%\LIB386
SET INCLUDE=%LIB386_PATH%;%INCLUDE%