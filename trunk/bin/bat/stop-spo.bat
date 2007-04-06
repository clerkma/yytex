@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem copy c:\bat\controlc.txt lpt1:/b
superspl /p
rem send controlc to printer to kill process
copy c:\bat\controlc.txt lpt1:/b
copy c:\bat\controld.txt lpt1:/b
@echo on
