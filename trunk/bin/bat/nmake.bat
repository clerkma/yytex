@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "STATE" == "32BIT" goto 32bit
if "STATE" == "16BIT" goto 16bit
:32bit
c:\msvcnt\bin\nmake %1 %2 %3 %4 %5 %6 %7 %8 %9 
goto end

:16bit
c:\msvc\bin\nmake %1 %2 %3 %4 %5 %6 %7 %8 %9 

:end
