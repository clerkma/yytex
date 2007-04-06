@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%STATE%" == "32BIT" call 32bit 
d:
cd \dvisourc
chdir

echo copydate -v -x dvipsone.exe dvipsone.dll
copydate -v -x dvipsone.exe dvipsone.dll

if errorlevel == 1 goto skipdel

echo need to delete *.obj file created for EXE
echo del *.obj
del *.obj
echo del vc60.pch
del vc60.pch

:skipdel

echo nmake /f makedll
nmake /f makedll

rem xcopy d:\dvisourc\dvipsone.dll d:\winsourc /y
