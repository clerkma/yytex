@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo THIS REPLACES REPLACE (NOT in EPSILON buffer in 95/98)
echo "%0" "%1" "%2" "%3" "%4" "%5" "%6" "%7" "%8" "%9"
pause

if "%3" == "/u/p" goto upprompt
if "%3" == "/u" goto upgrade
if "%3" == "/a/p" goto addprompt
if "%3" == "/a" goto add

if "%3" == "/U/P" goto upprompt
if "%3" == "/U" goto upgrade
if "%3" == "/A/P" goto addprompt
if "%3" == "/A" goto add

echo SORRY DON'T UNDERSTAND ARGUMENTS
pause
goto end

:upprompt
echo XCOPY %1 %2 /u/p/d
XCOPY %1 %2 /u/p/d
goto end

:upgrade
echo XCOPY %1 %2 /u/d
XCOPY %1 %2 /u/d
goto end

:addprompt
echo XCOPY %1 %2 /d/p
XCOPY %1 %2 /d/p
goto end

:add
echo XCOPY %1 %2 /d
XCOPY %1 %2 /d
goto end

:end
