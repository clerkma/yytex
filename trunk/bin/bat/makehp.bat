@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem makehp.bat
if "%1" == "" goto usage

d:
cd \
cd hp
cd fonts
cd %1

rem if %1.hex is older than %1.out then run convert
copydate -x %1.hex %1.out
if not errorlevel == 1 echo %1.hex is up to date
if errorlevel == 1 call convert -v %1.out

rem if %1.pfa is older than %1.hex then run fontone
copydate -x %1.pfa %1.hex %1.hnt %1.afm
if not errorlevel == 1 echo %1.pfa is up to date
rem if errorlevel == 1 fontone -vl %1.out
if errorlevel == 1 fontone -vl %1

rem if %1.pfb is older than %1.pfa then run pfatopfb
copydate -x %1.pfb %1.pfa
if not errorlevel == 1 echo %1.pfb is up to date
if errorlevel == 1 pfatopfb -v %1.pfa

rem if %1.pfm is older than %1.afm then run afmtopfm
copydate -x %1.pfm %1.afm
if not errorlevel == 1 echo %1.pfm is up to date
if errorlevel == 1 afmtopfm -vsdt %1.afm

rem if %1.tfm is older than %1.afm then run afmtotfm
if not errorlevel == 1 echo %1.tfm is up to date
copydate -x %1.tfm %1.afm
if errorlevel == 1 afmtotfm -v %1.afm
goto end

:usage
echo go into directory of font, e.g. d:\hp\fonts\hplogo
echo then makehp [font]

:end

