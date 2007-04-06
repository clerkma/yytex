@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem show current directory and command line
chdir
echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
rem pause

rem d:\yandy\yandytex\yandytex.exe -v -L %1 %2 %3 %4 %5 %6 %7 %8 %9
d:\texsourc\yandytex.exe -v -L %1 %2 %3 %4 %5 %6 %7 %8 %9
if not errorlevel 1 goto end

rem pause, but only when run in DOS box in Windows
set | find "windir=" > NUL
if not errorlevel 1 pause

:end
