@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

% c:
rem c:\norton\ds ne %1 /s
if "%1" == "" goto argmiss
c:\norton\ds n %1 /s
goto cont
:argmiss
c:\norton\ds n c:\ /s
:cont
if errorlevel=1 goto no
echo Do you want to re-boot the machine? [Y]
ask
if errorlevel=2 goto yes
if errorlevel=1 goto enter
goto no
:yes
:enter
reboot
:no
@echo on
