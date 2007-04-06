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
rem call send c:\ps\scc-576
copy c:\ps\scc-576.ps com1:/b
echo Switching to 57,600 baud
:no2
echo Has the printer stopped flashing [Y]
ask
if errorlevel = 2 goto yes2
if errorlevel = 1 goto enter2
goto no2
:yes2
:enter2
copy c:\bat\mod576.bat c:\bat\modset.bat
modex com1: 57600,n,8,1
@echo on
