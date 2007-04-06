@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo DVISETNT.BAT calls c:\prog\dvisetup.exe

echo doing dvipsone.EXE first
rem c:\prog\dvisetup.exe -o=d:\dvisourc %1 %2 %3 %4 %5 %6 %7 %8 %9 
rem echo c:\prog\dvisetup.exe -o=d:\dvisourc\dvipsont.exe %1 %2
echo c:\prog\dvisetup.exe -o=d:\dvisourc\dvipsone.exe %1 %2
rem c:\prog\dvisetup.exe -o=d:\dvisourc\dvipsont.exe %1 %2 %3 %4 %5 %6 %7 %8
c:\prog\dvisetup.exe -o=d:\dvisourc\dvipsone.exe %1 %2 %3 %4 %5 %6 %7 %8

rem pause 

echo doing dvipsone.DLL second
echo c:\prog\dvisetup.exe -o=d:\dvisourc\dvipsone.dll %1 %2
c:\prog\dvisetup.exe -o=d:\dvisourc\dvipsone.dll %1 %2 %3 %4 %5 %6 %7 %8

xcopy d:\dvisourc\dvipsone.dll d:\winsourc /y


