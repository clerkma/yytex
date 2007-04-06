@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%STATE%" == "32BIT" call 32bit 
d:
cd \texsourc
chdir
echo nmake
nmake

rem copydate -x yandynt.exe texmf.exe
copydate -x yandytex.exe texmf.exe
if not errorlevel == 1 goto end

rem not yandynt.exe anymore, executable now called yandytex.exe 96/Sep/14

rem echo copy texmf.exe yandynt.exe
echo copy texmf.exe yandytex.exe
rem copy texmf.exe yandynt.exe
copy texmf.exe yandytex.exe

:end
