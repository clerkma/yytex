@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%1" == "" goto one
echo /#copies %1 def > c:\ps\copies.ps
goto notone
:one
echo /#copies 1 def > c:\ps\copies.ps
:notone
if "%2" == "" goto noarg
copy c:\ps\copies.ps+c:\ps\metrics.ps+c:\ps\controld.ps %2/b
goto end
:noarg
copy c:\ps\copies.ps+c:\ps\metrics.ps+c:\ps\controld.ps lpt1:/b
:end
@echo on
