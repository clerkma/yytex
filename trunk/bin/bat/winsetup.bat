@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo WINSETUP.BAT calls c:\prog\winsetup.exe

c:\prog\winsetup.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 

goto end

rem call 16bit

rem Use argument U if just an upgrade.  To avoid losing serial number...

if not "%1" == "U" goto newone

rem if exist c:\winsourc\serial.old del c:\winsourc\serial.old
rem if exist c:\winsourc\serial.num rename c:\winsourc\serial.num serial.old

copy c:\winsourc\serial.num c:\winsourc\serial.old

:newone
rem if exist c:\winsourc\dviwindo.old del c:\winsourc\dviwindo.old
rem if exist c:\winsourc\dviwindo.exe rename c:\winsourc\dviwindo.exe dviwindo.old

copy c:\winsourc\dviwindo.exe c:\winsourc\dviwindo.old

rem call c:\prog\winsetup %1 %2 %3 %4 %5 %6 %7 %8 %9
echo call c:\prog\winsetup -o=c:\winsourc\dviwindo.exe %1 %2 %3 %4 %5 %6 %7 %8
call c:\prog\winsetup -o=c:\winsourc\dviwindo.exe %1 %2 %3 %4 %5 %6 %7 %8
if errorlevel = 1 goto longname
cd ..
rem d:
e:

copydate c:\winsourc\dviwindo.exe c:\winsourc\dviwindo.old
del c:\winsourc\dviwindo.old
goto end

:longname
rename c:\winsourc\dviwindo.old dviwindo.exe

:end
if not "%1" == "U" goto depart

if not exist c:\winsourc\serial.num goto depart
rem del c:\winsourc\serial.num 
rem if not exist c:\winsourc\serial.old goto depart
rem rename c:\winsourc\serial.old serial.num

copy c:\winsourc\serial.old  c:\winsourc\serial.num

echo Now reverting to:
type c:\winsourc\serial.num

:depart

:end
