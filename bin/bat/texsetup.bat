@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo TEXSETUP.BAT calls c:\prog\texsetup.exe
echo This assumes that REBINDNT has been run (rebindb yandytex emutnt.dll)
echo This assumes CFIG386 has been run (cfig386 yandytex.exe %%yandytex)
dir c:\texsourc\yandytex.exe

rem c:\prog\texsetup.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 
rem echo c:\prog\texsetup.exe -o=c:\texsourc\yandytex %1 %2 %3 %4 %5 %6 %7 %8 %9 
echo c:\prog\texsetup.exe -o=c:\texsourc\yandytex.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 
rem c:\prog\texsetup.exe -o=c:\texsourc\yandytex %1 %2 %3 %4 %5 %6 %7 %8 %9 
c:\prog\texsetup.exe -o=c:\texsourc\yandytex.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 

goto end

rem FOLLOWING NOT USED ANYMORE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

rem Use argument U is just an upgrade.  To avoid losing serial number
rem
if not "%1" == "U" goto newone
if exist c:\texsourc\serial.old del c:\texsourc\serial.old
if exist c:\texsourc\serial.num rename c:\texsourc\serial.num serial.old

:newone
if exist c:\texsourc\texsourc.old del c:\texsourc\texsourc.old
if exist c:\texsourc\texsourc.exe rename c:\texsourc\texsourc.exe texsourc.old
call c:\prog\texsetup %1 %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel = 1 goto giveup
echo NOT rebinding texsourc
rem rebindb texsourc
echo NOT configuring in texsourc environment variable
rem cfig386 texsourc.exe %%texsourc
rem rebindb texsourc
cd ..
rem e:
d:
rem echo and zip it ?
rem
copydate c:\texsourc\texsourc.exe c:\texsourc\texsourc.old
del c:\texsourc\texsourc.old
goto end

:giveup
if exist c:\texsourc\texsourc.old rename c:\texsourc\texsourc.old texsourc.exe

:end
if not "%1" == "U" goto depart

if exist c:\texsourc\serial.num del c:\texsourc\serial.num 
if exist c:\texsourc\serial.old rename c:\texsourc\serial.old serial.num
echo Now reverting to:
type c:\texsourc\serial.num

:depart
echo The DOS extended version of Y&Y TeX 1.2 has been set up
@echo on
