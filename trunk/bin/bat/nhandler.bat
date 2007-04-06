@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem downloads modified error handler to NewGen printer on parallel port LPT1:
rem copy c:\ps\nhandler.ps+c:\ps\controld.ps com1:/b
copy c:\ps\nhandler.ps+c:\ps\controld.ps lpt1:/b
@echo on
