rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

@echo off
echo /#copies %1 def > c:\ps\copies.ps
copy c:\ps\copies.ps+c:\ps\faxcover.ps+c:\ps\controld.ps com1:/b
@echo on
