@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo (%1) > c:\ps\params.ps
copy c:\ps\params.ps+%1.one+c:\ps\vmusage1.ps+%1.one+c:\ps\vmusage2.ps lpt1:/b
@echo on
