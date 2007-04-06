@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo New customization batch file (16 bit)

echo.
echo Read user name and serial number (cmbsetup) %1 %2 %3 %4 %5 %6 %7 %8 %9 
echo and create c:\yyinstal\serial.ini
pause
call cmbsetup -d=c: %1 %2 %3 %4 %5 %6 %7 %8 %9 
if not errorlevel == 1 goto doit
echo Giving up. Some error occured in cmbsetup.  
goto end

:doit
echo.
echo Now update d:\dvisourc\dvipsone.exe and *.enc preamble files
pause
call dvisetup -i

echo.
echo Now update d:\winsourc\dviwindo.exe
pause
call winsetup -i

echo.
echo Now update d:\texsourc\yandytex.exe
pause
call texsetup -i

echo.
echo 16 bit versions of DVIPSONE.EXE, DVIWINDO.EXE, YANDYTEX.EXE customized.

:end
