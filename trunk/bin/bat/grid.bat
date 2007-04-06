@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if //==/%2/ goto stand
if exist %1.eps goto eps
echo (%1.ps) 8.5 11 0 %2 > c:\ps\params.ps
copy c:\ps\params.ps+c:\ps\grid.ps+%1.ps com1:/b
goto end
:eps 
echo (%1.eps) 8.5 11 0 %2 > c:\ps\params.ps
copy c:\ps\params.ps+c:\ps\grid.ps+%1.eps com1:/b
goto end
:stand
if exist %1.eps goto eps2
echo (%1.ps) 8.5 11 0 0.8 > c:\ps\params.ps
copy c:\ps\params.ps+c:\ps\grid.ps+%1.ps com1:/b
goto end
:eps2
echo (%1.eps) 8.5 11 0 0.8 > c:\ps\params.ps
copy c:\ps\params.ps+c:\ps\grid.ps+%1.eps com1:/b
:end
del c:\ps\params.ps
@echo on
