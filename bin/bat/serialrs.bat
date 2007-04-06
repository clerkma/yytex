@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%1" == "-?" goto usage

rem if "%1" == "" %0 c:
if "%1" == "" %0 d:
if "%2" == "" %0 %1 a:

echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9

if not "%1" == "c:" goto skip16
if not exist %2\win16 goto mismatch
echo Overwriting 16 bit customization on disk c:
goto common

:skip16
if not "%1" == "d:" goto notcord
if not exist %2\win32 goto mismatch
echo Overwriting 32 bit customization on disk d:
goto common

:common
echo Overwriting customization data on hard disk %1
pause

if not exist %2\dvisourc\nul echo MISSING %2\dvisourc
if not exist %1\dvisourc\serial.num echo NO %1\dvisourc\serial.num!
copydate -x %1\dvisourc\serial.num %2\dvisourc\serial.num
if errorlevel = 1 goto writedvi
echo %1\dvisourc\serial.num not older than %2\dvisourc\serial.num
goto skipdvi

:writedvi
copy %1\dvisourc\serial.num %1\dvisourc\serial.svd
rem replace %2\dvisourc\*.num %1\dvisourc /u/p
copy %2\dvisourc\*.num %1\dvisourc

:skipdvi
if not exist %2\winsourc\nul echo MISSING %2\winsourc
if not exist %1\winsourc\serial.num echo NO %1\winsourc\serial.num!
copydate -x %1\winsourc\serial.num %2\winsourc\serial.num
if errorlevel = 1 goto writewin
echo %1\winsourc\serial.num not older than %2\winsourc\serial.num
goto skipwin

:writewin
copy %1\winsourc\serial.num %1\winsourc\serial.svd
rem replace %2\winsourc\*.num %1\winsourc /u/p
copy %2\winsourc\*.num %1\winsourc

:skipwin
if not exist %2\texsourc\nul echo MISSING %2\texsourc
if not exist %1\texsourc\serial.num echo NO %1\texsourc\serial.num!

copydate -x %1\texsourc\serial.num %2\texsourc\serial.num
if errorlevel = 1 goto writetex
echo %1\texsourc\serial.num not older than %2\texsourc\serial.num
goto skiptex

:writetex
copy %1\texsourc\serial.num %1\texsourc\serial.svd
rem replace %2\texsourc\*.num %1\texsourc /u/p
copy %2\texsourc\*.num %1\texsourc

:skiptex
goto end

:notcord
echo Must specify drive c: or drive d:
goto usage

:mismatch
echo Data on disk is not for %1
dir %2
goto usage

:usage
echo Usage:  serialrs [destination disk]  [source diskette]
echo e.g. serialsv d: a:

:end
