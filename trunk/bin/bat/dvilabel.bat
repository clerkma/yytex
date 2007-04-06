@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if  "%1" == "" %0 1
echo /#copies %1 def > c:\ps\copies.ps

echo REMEMBER TO FEED THE PAPER IN WITH NARROW STRIPS AT BOTTOM!
pause

rem copy c:\ps\copies.ps+c:\ps\dvilabel.ps+c:\ps\controld.ps com1:/b
copy c:\ps\copies.ps+c:\ps\dvilabel.ps+c:\ps\controld.ps lpt1:/b
