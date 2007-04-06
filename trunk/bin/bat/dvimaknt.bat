@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%STATE%" == "32BIT" call 32bit 
d:
cd \dvisourc
chdir

echo copydate -v -x dvipsone.dll dvipsone.exe
copydate -v -x dvipsone.dll dvipsone.exe

if errorlevel == 1 goto skipdel

echo need to delete *.obj file created for DLL
echo del *.obj
del *.obj
echo del vc60.pch
del vc60.pch

:skipdel

echo nmake
nmake

goto end

rem not anymore, executable now called dvipsone.exe 96/Sep/14

rem copydate -x dvipsont.exe dvipsone.exe
rem if not errorlevel == 1 goto end

rem echo copy dvipsone.exe dvipsont.exe
rem copy dvipsone.exe dvipsont.exe

:end
