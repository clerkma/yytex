@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem if not "%2"  == "" goto force
if not exist c:\prog\%1.exe goto force
copydate -x c:\prog\%1.exe c:\c\%1.c
if errorlevel == 1 goto force
echo The file c:\prog\%1.exe is not older than the file c:\c\%1.c
if not "%2"  == "" goto force
goto end

:force
if exist c:\c\%1.c goto normal
rem See c:\msvc\help\mscopts.hlp for options:
echo Can't find source file c:\c\%1.c
rem Don't use ".c" extension on file name!
goto end

:normal
if not "%STATE%" == "16BIT" call 16bit 
rem d:
if not "%HOMEDRIVE%" == "" goto windowsnt
echo switching to RAM drive
e:
goto skipnt
:windowsnt
echo NOT switching to RAM drive
:skipnt

@echo on
rem c:\msvc\bin\cl -c -AS -W2 -Ot -Za -Lr -F 4000 c:\c\%1.c
c:\msvc\bin\cl -c -AS -W2 -Za -Lr -F 4000 c:\c\%1.c
@echo off
rem c:\msvc\bin\cl -c -AC -W2 -Ot -Za -Lr -F 4000 c:\c\%1.c
if errorlevel 1 goto end
@echo on
c:\msvc\bin\link /noi /noe %1.obj c:\msvc\lib\setargv.obj;
@echo off
rem -G2 -FPi87
:end
