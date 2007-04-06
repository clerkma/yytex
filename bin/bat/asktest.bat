@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo Type Y, N or Enter
c:\prog\ask
echo analysing error level
rem Y:
if errorlevel 2 if not errorlevel 3 echo error level 2
rem Enter:
if errorlevel 1 if not errorlevel 2 echo error level 1
rem N:
if errorlevel 0 if not errorlevel 1 echo error level 0
echo exiting batch file
