@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo I assume you actually want epsilon!
set epspath=c:\epsilon
if %FLAVOUR% == DOS goto :dos
c:\epsilon\bin\epsilon -m256  %1
goto :end
:dos
rem c:\epsilon\epsilon -fsD: %1
c:\epsilon\bin\epsilon -m256 -fsD: %1
:end
@echo on
