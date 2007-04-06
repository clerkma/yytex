@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem send file to parallel printer LPT1:
if exist %1.ps goto ps
if exist %1.alw goto alw
copy %1 lpt1:/b
rem copy %1 com1:/b
goto end
:ps 
copy %1.ps lpt1:/b
rem copy %1.ps com1:/b
goto end
:alw
copy %1.alw lpt1:/b
rem copy %1.alw com1:/b
:end
@echo on
