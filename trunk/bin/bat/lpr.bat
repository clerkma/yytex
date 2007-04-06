@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if exists %1.ps goto abc
rem c:\dvialw\lptops.exe -N -O -P11pt -U %1 >lpt1:
c:\dvialw\lptops.exe -N -U -FCourier -P10pt %1 %2 %3 %4 %5 %6 %7 %8 %9 
copy %1.ps lpt1:/b
rem see c:\dvi\lptops.hlp for more information
goto cba
:abc
copy %1.ps lpt1:/b
:cba
@echo on
