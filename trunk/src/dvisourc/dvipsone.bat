@echo off
rem show current directory and command line
rem Copyright (C) 2006 TeX Users Group.
rem You may freely use, modify and/or distribute this file.
chdir
echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9

C:\yandy\dvipsone\dvipsone.exe %1 %2 %3 %4 %5 %6 %7 %8 %9
if not errorlevel 1 goto end

rem pause, but only when run from command prompt in Windows
rem set | find "windir=" > NUL
rem if not errorlevel 1 pause

:end
