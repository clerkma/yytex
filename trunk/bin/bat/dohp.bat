@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%1" == "-?" goto usage
if "%1" == "?" goto usage
if not "%1" == "" goto argok

:usage
echo Sample usage: %0 c:\hp\fonts\hplogo\hplogo
echo Sample usage: %0 c:\hp\fonts\keys10\keys10
echo Sample usage: %0 c:\hp\fonts\hpbats\hpbats
goto end

:argok
d:
cd \
chdir
showchar -m %1.hex
if errorlevel = 1 goto bad
echo Saving old %1.hnt file in %1.svd
copy %1.hnt %1.svd
echo Copying new hint file back to hard disk as %1.hnt
copy *.stm %1.hnt
dir %1.hnt
echo DONE!
goto end

:bad
echo SHOWCHAR returned error code - hint file not saved

:end
