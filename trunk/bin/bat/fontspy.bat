@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if exist %1.mac goto mac
if exist %1.pfb goto pc
goto end
:mac
c:\prog\mactopfb -v %1
rem goto dec
:pc
c:\prog\pfbtopfa -v %1
:dec
c:\prog\decrypt -vse %1
:end
@echo on
