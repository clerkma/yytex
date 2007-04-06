@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo (%1) > c:\ps\params.ps
if exist %1.ps goto ps
if exist %1.one goto one
if exist %1.hex goto hex
echo can't find file
goto end
:hex
copy %1.pfa+c:\ps\params.ps+c:\ps\chartest.ps com1:/b
goto end
:one
copy %1.one+c:\ps\params.ps+c:\ps\charshow.ps com1:/b
goto end
:ps 
copy %1.ps+c:\ps\params.ps+c:\ps\charshow.ps com1:/b
goto end
:end
del c:\ps\params.ps
@echo on
