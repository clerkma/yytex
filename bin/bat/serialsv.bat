@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%1" == "-?" goto usage

rem if "%1" == "" %0 c:
if "%1" == "" %0 d:
if "%2" == "" %0 %1 a:

echo %0 %1 %2

if not "%1" == "c:" goto not16
echo Saving 16 bit customization on disk c:
if exist %2\win32 dir %2
if exist %2\win32 del %2\win32
echo > %2\win16
goto saving

:not16
if not "%1" == "d:" goto not16or32
echo Saving 32 bit customization on disk d:
if exist %2\win16 dir %2
if exist %2\win16 del %2\win16
echo > %2\win32
goto saving

:saving
rem serialsv [source disk]  [destination diskette]

echo Saving customization data on diskette

if not exist %2\dvisourc\nul mkdir %2\dvisourc

copydate -x %2\dvisourc\serial.num %1\dvisourc\serial.num
if errorlevel = 1 goto writedvi
echo %2\dvisourc\serial.num not older than %1\dvisourc\serial.num
goto skipdvi

:writedvi
if not exist %2\dvisourc\serial.num goto nodvi
if exist %2\dvisourc\serial.bak del %2\dvisourc\serial.bak
rename %2\dvisourc\serial.num serial.bak

:nodvi
echo copy %1\dvisourc\serial.num %2\dvisourc
copy %1\dvisourc\serial.num %2\dvisourc

:skipdvi
if not exist %2\winsourc\nul mkdir %2\winsourc

copydate -x %2\winsourc\serial.num %1\winsourc\serial.num
if errorlevel = 1 goto writewin
echo %2\winsourc\serial.num not older than %1\winsourc\serial.num
goto skipwin

:writewin
if not exist %2\winsourc\serial.num goto nowin
if exist %2\winsourc\serial.bak del %2\winsourc\serial.bak
rename %2\winsourc\serial.num serial.bak

:nowin
echo copy %1\winsourc\serial.num %2\winsourc
copy %1\winsourc\serial.num %2\winsourc

:skipwin
if not exist %2\texsourc\nul mkdir %2\texsourc

copydate -x %2\texsourc\serial.num %1\texsourc\serial.num
if errorlevel = 1 goto writetex
echo %2\texsourc\serial.num not older than %1\texsourc\serial.num
goto skiptex

:writetex
if not exist %2\texsourc\serial.num goto notex
if exist %2\texsourc\serial.bak del %2\texsourc\serial.bak
rename %2\texsourc\serial.num serial.bak

:notex
echo copy %1\texsourc\serial.num %2\texsourc
copy %1\texsourc\serial.num %2\texsourc

:skiptex

goto end

:not16or32

:usage
echo serialsv [source disk]  [destination diskette]
echo e.g. serialsv d: a:

:end
