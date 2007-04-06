@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

:no1
echo Has the printer been powered up [Y]
ask
if errorlevel = 2 goto yes1
if errorlevel = 1 goto enter1
goto no1
:yes1
:enter1
rem call send c:\ps\scc-384
copy c:\ps\scc-384.ps com1:/b
echo Switching to 38,400 baud
:no2
echo Has the printer stopped flashing [Y]
ask
if errorlevel = 2 goto yes2
if errorlevel = 1 goto enter2
goto no2
:yes2
:enter2
copy c:\bat\mod384.bat c:\bat\modset.bat
modex com1: 38400,n,8,1
@echo on
