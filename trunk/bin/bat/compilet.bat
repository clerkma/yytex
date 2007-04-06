@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem compile in TINY model (for .COM files less than 64k)
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
c:\msvc\bin\cl -AT -W2 -Od  c:\c\%1.c
@echo off
:end
