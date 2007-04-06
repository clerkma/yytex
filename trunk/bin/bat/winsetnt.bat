@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo WINSETNT.BAT calls c:\prog\winsetup.exe

rem c:\prog\winsetup.exe -o=d:\winsourc %1 %2 %3 %4 %5 %6 %7 %8 %9 
rem echo c:\prog\winsetup.exe -o=d:\winsourc\dviwinnt.exe %1 %2
echo c:\prog\winsetup.exe -o=d:\winsourc\dviwindo.exe %1 %2
rem c:\prog\winsetup.exe -o=d:\winsourc\dviwinnt.exe %1 %2 %3 %4 %5 %6 %7 %8
c:\prog\winsetup.exe -o=d:\winsourc\dviwindo.exe %1 %2 %3 %4 %5 %6 %7 %8


