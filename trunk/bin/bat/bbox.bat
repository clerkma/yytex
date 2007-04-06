@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem Print Bounding Box of PS file
if exist %1.ps goto ps
if exist %1.alw goto alw
copy c:\ps\bbox.ps+%1 lpt1:/b
goto end
:ps 
copy c:\ps\bbox.ps+%1.ps lpt1:/b
goto end
:alw
copy c:\ps\bbox.ps+%1.alw lpt1:/b
:end
@echo on
