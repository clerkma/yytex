@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%1" == "" %0 a:
echo *.c
savefile c:\y&ytex\*.c %1
echo *.txt
savefile c:\y&ytex\*.txt %1
echo lib\*.c
if not exist %1\lib\nul mkdir %1\lib
savefile c:\y&ytex\lib\*.c %1\lib
echo lib\*.h
savefile c:\y&ytex\lib\*.h %1\lib
