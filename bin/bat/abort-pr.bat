@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem send controlc to printer to kill process
copy c:\bat\controlc.txt com1:/b
copy c:\bat\controld.txt com1:/b
@echo on
