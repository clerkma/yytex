@echo off
echo Refreshing files for %1
rem makeone cmsy10
copydate -x %1.hex %1.out
if errorlevel == 1 converto %1
copydate -x %1.pfa %1.hex %1.hnt %1.afm
rem if errorlevel == 1 fontone -v0 %1
if errorlevel == 1 fontone -vl0 %1
copydate -x %1.pfb %1.pfa
if errorlevel == 1 pfatopfb -v %1
copydate -x %1.pfm %1.afm
if errorlevel == 1 afmtopfm -vsd %1
