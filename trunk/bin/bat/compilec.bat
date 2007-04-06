@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo Compile in compact model, 64k total code + data.
if exist c:\c\%1.c goto normal
rem See c:\c600\help\cl.hlp for options:
echo Can't find source file c:\c\%1.c
rem Don't use ".c" extension on file name!
goto end
:normal
rem d:
call 16bit
e:
@echo on
c:\windev\bin\cl -c -AC -W1 -Od -Za -Lr -F 4000 c:\c\%1.c
@echo off
if errorlevel 1 goto end
@echo on
c:\windev\bin\link /noi /noe %1.obj c:\windev\lib\setargv.obj;
@echo off
:end
@echo on
