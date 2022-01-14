@echo off
echo LBA Build Environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsamd64_x86.bat"
call C:\WATCOM\owsetenv.bat
SET LIB386_PATH=%CD%\LIB386
SET INCLUDE=%LIB386_PATH%;%INCLUDE%