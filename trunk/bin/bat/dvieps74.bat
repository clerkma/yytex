@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo /#copies %1 def > c:\ps\copies.ps
rem copy c:\ps\copies.ps+c:\ps\dvieps74.ps+c:\ps\controld.ps com1:/b
copy c:\ps\copies.ps+c:\ps\dvieps74.ps+c:\ps\controld.ps lpt1:/b
@echo on
