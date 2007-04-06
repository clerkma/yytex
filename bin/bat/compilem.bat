@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if exist c:\c\%1.c goto normal
rem See c:\msvc\help\cl.hlp for options:
echo Can't find source file c:\c\%1.c
rem Don't use ".c" extension on file name!
goto end

:normal
if not "%STATE%" == "16BIT" call 16bit 
rem d:
if not "%HOMEDRIVE%" == "" goto windowsnt
echo switching to RAM drive
d:
goto skipnt
:windowsnt
echo NOT switching to RAM drive
:skipnt

@echo on
c:\msvc\bin\cl -c -AM -W1 -Od -Za -Lr -F 6000 c:\c\%1.c
@echo off
if errorlevel 1 goto end
@echo on
c:\msvc\bin\link /noi /noe %1.obj c:\msvc\lib\setargv.obj;
@echo off
rem -G2 -FPi87
:end
@echo on
