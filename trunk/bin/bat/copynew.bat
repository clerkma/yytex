@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%4" == "" goto destok
echo Need destination
goto usage

:destok
if not "%3" == "" goto sourcok
echo Need source
goto usage

:sourcok
if not "%2" == "" goto dayok
echo Need day of month
goto usage

:dayok
if not "%1" == "" goto monthok
echo Need day of month
goto usage

:monthok
xcopy /D:%1/%2/96 %3 %4
goto end

:usage
echo copynew [month] [day] [source] [destination]
echo.
echo e.g. copynew 6 1 c:\c\*.c a:\c

:end
