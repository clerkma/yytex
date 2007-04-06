@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo /#copies %1 def > c:\ps\copies.ps
copy c:\ps\copies.ps+c:\ps\envelope.ps+c:\ps\controld.ps com1:/b
@echo on
