@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo New customization batch file (32 bit)

echo.
echo Read user name and serial number (cmbsetup) %1 %2 %3 %4 %5 %6 %7 %8 %9 
echo and create c:\yyinstal\serial.ini

echo call cmbsetup -d=d: %1 %2 %3 %4 %5 %6 %7 %8 %9 
echo we no longer pause here (except if disk needs to spin up)
if not "%1" == "" call cmbsetup -d=d: -e=%1 %2 %3 %4 %5 %6 %7 %8 %9 
if "%1" == "" call cmbsetup -d=d: %1 %2 %3 %4 %5 %6 %7 %8 %9 

if not errorlevel == 1 goto doit
echo.
echo Giving up. Some error occured in cmbsetup.  
echo unfortunately this will NOT prevent Wise from running.  Be patient.
pause
goto end

:doit
echo.
pause
echo Now update d:\dvisourc\dvipsone.exe and *.enc preamble files
echo call dvisetnt -i
call dvisetnt -i

echo.
echo Now update d:\winsourc\dviwindo.exe
rem pause
echo we no longer pause here (except if disk needs to spin up)
echo call winsetnt -i
call winsetnt -i

echo.
echo Now update d:\texsourc\yandytex.exe
echo call texsetnt -i
rem pause
echo we no longer pause here (except if disk needs to spin up)
call texsetnt -i

echo.
echo 32 bit versions of DVIPSONE.EXE, DVIWINDO.EXE, YANDYTEX.EXE customized.

rem pause
:end
cls
