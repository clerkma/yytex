@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo set up for MS 16 bit compiler and linker
if "%BASEPATH%" == "" set BASEPATH=%PATH%
rem
if "%SYSTEMROOT%" == "" goto notwinnt
echo Apparently in Windows NT
set PATH=c:\windows\system32;c:\msvc\bin;%BASEPATH%
goto setrest

:notwinnt
if "%winbootdir%" == "" goto win31
set PATH=c:\msvc\bin;%BASEPATH%
echo Apparently in Windows 95
goto setrest

:win31
echo Apparently in Windows 3.11
set PATH=c:\msvc\bin;%BASEPATH%
goto setrest

:setrest
set LIB=c:\msvc\lib
set INCLUDE=c:\msvc\include
set STATE=16BIT
