@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo This is the Computer Modern version of the flyers...
echo /#copies %1 def > c:\ps\copies.ps
rem copy c:\ps\copies.ps+c:\ps\dviflynd.ps+c:\ps\controld.ps com1:/b
copy c:\ps\copies.ps+c:\ps\dviflynd.ps+c:\ps\controld.ps lpt1:/b
@echo on
