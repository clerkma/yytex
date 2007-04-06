@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%2" == "" %0 %1 lpt1:

if exist %1 goto exists

if exist %1.ps %0 %1.ps %2
if exist %1.eps %0 %1.eps %2
if exist %1.pfa %0 %1.pfa %2

echo Sorry: %1 does not appear to exist
goto end

:exists
echo copy %1 %2
rem pause
copy %1 %2
rem pause

:end
