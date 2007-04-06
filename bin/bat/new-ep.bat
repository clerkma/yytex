@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo After loading, type F-3 changes ENTER
pause
rem Change directory to where eel customizations are
cd c:\epsilon\eel
rem Now set up epsilon with factory defaults
c:\epsilon\epsilon -sc:\epsilon\factory.sta -fsD:
cd c:
@echo on
