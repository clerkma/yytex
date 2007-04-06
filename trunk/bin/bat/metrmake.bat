@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem if not "%STATE%" == "16BIT" call 16bit 
if not "%STATE%" == "32BIT" call 32bit 
c:
cd c:\metrics
rem call make %1 %2 %3 %4 %5 %6 %7 %8 %9
call nmake %1 %2 %3 %4 %5 %6 %7 %8 %9
rem cd c:\
rem d:
