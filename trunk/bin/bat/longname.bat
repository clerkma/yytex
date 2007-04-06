@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem longname [YandY base dir]

if "%1" == "-?" goto usage

rem Check if argument given
if not "%1" == "" goto argok

rem Use default directory if possible
if exist c:\yandy\tex\latex2e\tools\*.dtx %0 c:\yandy
goto usage

:argok
rem Show command line
echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9

if exist %1\tex\latex2e\tools\*.* goto toolsok
echo Sorry, %1\tex\latex2e\tools does not exist
echo (Tools package not installed).
goto end

:toolsok
rem WARN user if the files already have the long names 
if exist %1\tex\latex2e\tools\longtable.dtx echo WARNING: longtable.dtx exists
if exist %1\tex\latex2e\tools\afterpage.dtx echo WARNING: afterpage.dtx exists
if exist %1\tex\latex2e\tools\enumerate.dtx echo WARNING: enumerate.dtx exists
if exist %1\tex\latex2e\tools\indentfirst.dtx echo WARNING: indentfirst.dtx exists

:doit
if exist %1\tex\latex2e\tools\longtabl.dtx goto longtabl
echo ERROR: %1\tex\latex2e\tools\longtabl.dtx not found!
goto skiplong

:longtabl
echo rename longtabl.dtx longtable.dtx
rename %1\tex\latex2e\tools\longtabl.dtx longtable.dtx

if not exist %1\tex\latex2e\tools\longtabl.sty goto skiplong
rename %1\tex\latex2e\tools\longtabl.sty longtable.sty

:skiplong
if exist %1\tex\latex2e\tools\afterpag.dtx goto afterpag
echo WARNING: %1\tex\latex2e\tools\afterpag.dtx not found!
goto skipafte

:afterpag
echo rename afterpag.dtx afterpage.dtx
rename %1\tex\latex2e\tools\afterpag.dtx afterpage.dtx

if not exist %1\tex\latex2e\tools\afterpag.sty goto skipafte
rename %1\tex\latex2e\tools\afterpag.sty afterpage.sty

:skipafte
if exist %1\tex\latex2e\tools\enumerat.dtx goto enumerat
echo WARNING: %1\tex\latex2e\tools\enumerat.dtx not found!
goto skipenum

:enumerat
echo rename enumerat.dtx enumerate.dtx
rename %1\tex\latex2e\tools\enumerat.dtx enumerate.dtx

if not exist %1\tex\latex2e\tools\enumerat.sty goto skipenum
rename %1\tex\latex2e\tools\enumerat.sty enumerate.sty

:skipenum
if exist %1\tex\latex2e\tools\indentfi.dtx goto indentfi
echo WARNING: %1\tex\latex2e\tools\indentfi.dtx not found!
goto skipinde

:indentfi
echo rename indentfi.dtx indentfirst.dtx
rename %1\tex\latex2e\tools\indentfi.dtx indentfirst.dtx

if not exist %1\tex\latex2e\tools\indentfi.sty goto skipinde
rename %1\tex\latex2e\tools\indentfi.sty indentfirst.sty

:skipinde
goto end

:usage
echo To rename four files in the tools package to have long names.
echo.
echo %0  [YandY base dir]
echo.
echo e.g. %0  c:\yandy
echo.

:end
