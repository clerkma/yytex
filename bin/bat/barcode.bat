@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo [%1 %2 %3 %4 %5 %6 %7 %8 %9] > c:\ps\params.ps
type c:\ps\params.ps
copy c:\ps\params.ps+c:\ps\barcode.ps+c:\ps\controld.ps lpt1:/b
@echo on
