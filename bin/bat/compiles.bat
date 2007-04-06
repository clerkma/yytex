@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem this is like compile1, but optimizes on space...
if exist c:\c\%1.c goto normal
rem See c:\msvc\help\cl.hlp for options:
echo Can't find source file c:\c\%1.c
rem Don't use ".c" extension on file name!
goto end
:normal
d:
@echo on
c:\msvc\bin\cl -c -AS -W1 -Os -Za -Lr -F 6000 c:\c\%1.c
@echo off
rem c:\msvc\bin\cl -c -AC -W1 -Od -Za -Lr -F 4000 c:\c\%1.c
if errorlevel 1 goto end
@echo on
c:\msvc\bin\link /noi /noe %1.obj c:\msvc\lib\setargv.obj;
@echo off
rem -G2 -FPi87
:end
@echo on
