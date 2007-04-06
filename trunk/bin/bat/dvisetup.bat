@echo off

echo DVISETUP.BAT calls c:\prog\dvisetup.exe

rem c:\prog\dvisetup.exe %1 %2 %3 %4 %5 %6 %7 %8 %9 
c:\prog\dvisetup.exe -c=c:\dvisourc %1 %2 %3 %4 %5 %6 %7 %8 %9 

goto end

rem call 16bit

rem Use argument U if just an upgrade.  To avoid losing serial number...

if not "%1" == "U" goto newone

rem if exist c:\dvisourc\serial.old del c:\dvisourc\serial.old
rem if exist c:\dvisourc\serial.num rename c:\dvisourc\serial.num serial.old

copy c:\dvisourc\serial.num c:\dvisourc\serial.old

:newone
rem if exist c:\dvisourc\dvipsone.old del c:\dvisourc\dvipsone.old
rem if exist c:\dvisourc\dvipsone.exe rename c:\dvisourc\dvipsone.exe dvipsone.old 

copy c:\dvisourc\dvipsone.exe c:\dvisourc\dvipsone.old

rem call c:\prog\dvisetup %1 %2 %3 %4 %5 %6 %7 %8 %9
echo call c:\prog\dvisetup -o=c:\dvisourc\dvipsone.exe %1 %2 %3 %4 %5 %6 %7 %8 
call c:\prog\dvisetup -o=c:\dvisourc\dvipsone.exe %1 %2 %3 %4 %5 %6 %7 %8 
if errorlevel = 1 goto giveup
cd ..
rem d:
e:
copydate c:\dvisourc\dvipream.enc c:\dvisourc\dvipream.ps
copydate c:\dvisourc\dvitpics.enc c:\dvisourc\dvitpics.ps
copydate c:\dvisourc\dvifont3.enc c:\dvisourc\dvifont3.ps

echo Now compressing preamble files to binary form
pfatopfb c:\dvisourc\dvipreamb.enc
copy dvipreamb.pfb c:\dvisourc\dvipreamb.enc /b
del dvipreamb.pfb
pfatopfb c:\dvisourc\dvitpics.enc
copy dvitpics.pfb c:\dvisourc\dvitpics.enc /b
del dvitpics.pfb
pfatopfb c:\dvisourc\dvifont3.enc
copy dvifont3.pfb c:\dvisourc\dvifont3.enc /b
del dvifont3.pfb

copydate c:\dvisourc\dvipsone.exe c:\dvisourc\dvipsone.old
del c:\dvisourc\dvipsone.old
goto end

:giveup
rem if exist c:\dvisourc\dvipsone.old rename c:\dvisourc\dvipsone.old dvipsone.exe

:end
if not "%1" == "U" goto depart

if not exist c:\dvisourc\serial.old goto depart
rem if exist c:\dvisourc\serial.num del c:\dvisourc\serial.num 
rem if exist c:\dvisourc\serial.old rename c:\dvisourc\serial.old serial.num

copy c:\dvisourc\serial.old  c:\dvisourc\serial.num

echo Now reverting to:
type c:\dvisourc\serial.num

:depart

:end