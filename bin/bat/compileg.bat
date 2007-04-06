@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo Compile small model, full optimizations, full warnings, GRAPHICS.LIB
rem -G2 -FPi87
if exist c:\c\%1.c goto normal
rem See c:\c600\help\cl.hlp for options:
echo Can't find source file c:\c\%1.c
rem Don't use ".c" extension on file name!
goto end
:normal
call 16bit
e:
@echo on
rem c:\windev\bin\cl -c -AS -W4 -Ox -Za -Lr -F 4000 c:\c\%1.c
rem c:\windev\bin\cl -c -AS -W4 -Ox -Za -Lr -F 2000 c:\c\%1.c
rem c:\windev\bin\cl -c -AS -W4 -Od -Za -Lr -F 2000 c:\c\%1.c
rem c:\windev\bin\cl -c -AS -W4 -Od -Za -Zi -Lr -F 2000 c:\c\%1.c
c:\windev\bin\cl -c -AS -W4 -Od -Zi -Lr -F 2000 c:\c\%1.c
@echo off
if errorlevel 1 goto end
rem c:\windev\bin\link /noi /noe %1.obj c:\windev\lib\setargv.obj;
@echo on
rem c:\windev\bin\link /noi /noe %1.obj c:\windev\lib\setargv.obj,,,graphics.lib; 
c:\windev\bin\link /noi /noe /co /map %1.obj c:\windev\lib\setargv.obj,,,graphics;
@echo off
:end
@echo on
