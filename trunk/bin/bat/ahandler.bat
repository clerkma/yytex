@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem downloads modified error handler to ALW printer on serial port COM1:
copy c:\ps\ahandler.ps+c:\ps\controld.ps lpt1:/b
@echo on
