@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo ������������������������������������������������������������������ͻ
if %FLAVOUR% == NORMAL echo � Set up for NORMAL use
if %FLAVOUR% == WINDEV echo � Set up for WINDOWS development
if %FLAVOUR% == MINIMAL echo � Set up for DISK OPTIMIZATION
if (%windir%)==(C:\WIN486) echo In Windows!
echo ������������������������������������������������������������������ͼ
@echo on
