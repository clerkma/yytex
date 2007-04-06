@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

modex com1:
echo Does the baud rate on COM1: match that of the printer? [Y]
ask
if errorlevel = 2 goto yes
if errorlevel = 1 goto enter
goto no
:yes
:enter
send c:\ps\scc-576
modex com1: 57600,n,8,1
:no
@echo on
