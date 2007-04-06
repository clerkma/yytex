@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo TEXSETNT.BAT calls c:\prog\texsetup.exe

echo doing yandytex.EXE first
rem c:\prog\texsetup.exe -o=d:\texsourc %1 %2 %3 %4 %5 %6 %7 %8 %9 
rem echo c:\prog\texsetup.exe -o=d:\texsourc\yandynt.exe %1 %2
echo c:\prog\texsetup.exe -o=d:\texsourc\yandytex.exe %1 %2
rem c:\prog\texsetup.exe -o=d:\texsourc\yandynt.exe %1 %2 %3 %4 %5 %6 %7 %8
c:\prog\texsetup.exe -o=d:\texsourc\yandytex.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 

echo doing yandytex.DLL second
echo c:\prog\texsetup.exe -o=d:\texsourc\yandytex.dll %1 %2
c:\prog\texsetup.exe -o=d:\texsourc\yandytex.dll %1 %2 %3 %4 %5 %6 %7 %8

xcopy d:\texsourc\yandytex.dll d:\winsourc /y



