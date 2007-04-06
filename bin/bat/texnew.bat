@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem show current directory
chdir

rem c:\y&ytex\y&ytex.exe -v %1 %2 %3 %4 %5 %6 %7 %8 %9
rem c:\y&ytex\y&ytex.exe -v -L -Z %1 %2 %3 %4 %5 %6 %7 %8 %9
rem c:\texsourc\y&ytex.exe -v -L -Z %1 %2 %3 %4 %5 %6 %7 %8 %9
rem tnt -softice c:\texsourc\y&ytex.exe -v -L -Z %1 %2 %3 %4 %5 %6 %7 %8 %9
tnt -softice c:\texsourc\yandytex.exe -v -L -Z %1 %2 %3 %4 %5 %6 %7 %8 %9
if not errorlevel 1 goto end

rem pause, but only when run in DOS box in Windows
set | find "windir=" > NUL
if not errorlevel 1 pause

:end
