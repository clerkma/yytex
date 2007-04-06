echo increased stack allocation
@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem second arg FORCE if want to ignore file date comparison
rem third arg is stack allocation if default 6000 not OK

if "%2" == "" goto skipextra
if "%2" == "FORCE" goto skipextra
if "%2" == "force" goto skipextra
if "%3" == "" %0 %1 foo %2

:skipextra
rem if not "%2"  == "" goto force
if not exist c:\prog\%1.exe goto force
copydate -x c:\prog\%1.exe c:\c\%1.c
if errorlevel == 1 goto force
echo The file c:\prog\%1.exe is not older than the file c:\c\%1.c
rem if not "%2"  == "" goto force
if "%2" == "FORCE" goto force
if "%2" == "force" goto force
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
if "%3" == "" goto noarg
rem c:\msvc\bin\cl -c -AS -W4 -Ox -Za -Lr -F %3 c:\c\%1.c
@echo on
c:\msvc\bin\cl -c -AS -W4 -Ox -Za -Lr -Gr -F %3 c:\c\%1.c
@echo off
goto link
:noarg
rem c:\msvc\bin\cl -c -AS -W4 -Ox -Za -Lr -F 6000 c:\c\%1.c
@echo on
c:\msvc\bin\cl -c -AS -W4 -Ox -Za -Lr -Gf -F 6000 c:\c\%1.c
@echo off
:link
if errorlevel 1 goto end

if "%3" == "" goto nostack
@echo on
c:\msvc\bin\link /STACK:%3 /noi /noe %1.obj c:\msvc\lib\setargv.obj;
@echo off
goto stackover

:nostack
c:\msvc\bin\link /noi /noe %1.obj c:\msvc\lib\setargv.obj;
@echo off
:stackover
rem -G2 -FPi87
:end
