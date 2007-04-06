@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%STATE%" == "32BIT" call 32bit 
d:
cd \winsourc
chdir
rem echo if "%1" == "" %0 NODEBUG
if "%1" == "" %0 NODEBUG

rem echo if "%1" == "NODEBUG" goto nodebug
if "%1" == "NODEBUG" goto nodebug

echo ********************** WARNING: DEBUG MODE ******************************
echo nmake %2 %3 %4 %5 %6 %7 %8 %9
nmake %2 %3 %4 %5 %6 %7 %8 %9
goto end

:nodebug
echo NODEBUG MODE
echo nmake "nodebug=1"  %2 %3 %4 %5 %6 %7 %8 %9
nmake "nodebug=1" %2 %3 %4 %5 %6 %7 %8 %9
goto end
