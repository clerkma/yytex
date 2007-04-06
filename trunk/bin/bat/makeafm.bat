@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%1" == "" goto argok
echo Sorry, Need to specify FontName 
goto end
:argok
echo /testfont /%1 def > c:\ps\params.ps
echo working on %1 as FontName
serial -v -t=0 -d=com2 c:\ps\params.ps c:\ps\getafm
:end
@echo on
