@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

copy c:\bat\config.dos c:\config.sys
copy c:\bat\autoexec.dos c:\autoexec.bat
echo Copied files to set up for straight DOS/TeX usage (Cache + D: RAM disk)
echo:
echo Reboot the machine? [Y]
ask
if errorlevel = 2 goto yes
if errorlevel = 1 goto enter
goto no
:yes
:enter
reboot
:no
@echo on
