@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem show current directory and command line
chdir
echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9

rem deal with use of 'amstex.ini' instead of 'amstex'
if "%1" == "amstex.ini" %0 amstex %2 %3

rem deal with use of 'latex.ltx' instead of 'latex'
if "%1" == "latex.ltx" %0 latex %2 %3

echo %1 | find "." > NUL
if errorlevel 1 goto noextens
echo Please do not use an extension (%1) when specifying file name
goto endpause

:noextens
rem switch to source file directory
if not "%3" == "" C:\DVIWINDO\cdd.com %3

rem deal with use of 'amstex' instead of 'amstex.ini'
if not "%1" == "amstex" goto notamstex
call tex -i %1.ini
goto inidone

:notamstex
rem deal with use of 'latex' instead of 'latex.ltx'
if not "%1" == "latex" goto notlatex
call tex -i %1.ltx
goto inidone

:notlatex
rem generic case, assume extension is '.tex'
call tex -i %1

:inidone
rem DON'T mess with files if TeX had a cow ...
if not errorlevel = 1 goto seemsok
echo Sorry, TeX appears to be have become unhappy --- formats will NOT be saved
goto endpause

:seemsok
echo copying new format file to c:\yandytex\fmt
copy %1.fmt c:\yandytex\fmt
copy %1.log c:\yandytex\fmt

echo deleting new format file from working directory
del %1.log
del %1.fmt

rem deal with special case (can omit format, since 'plain' is default)
if not "%1" == "plain" goto notplain
rem echo %2=tex.bat @.tex>> C:\WINDOWS.000\dviwindo.ini
echo %2=tex.bat @.tex>> C:\WINDOWS\dviwindo.ini
goto end

:notplain
rem echo %2=tex.bat +%1 @.tex>> C:\WINDOWS.000\dviwindo.ini
echo %2=tex.bat +%1 @.tex>> C:\WINDOWS\dviwindo.ini
goto end

:endpause
rem pause, but only when run in DOS box in Windows
set | find "windir=" > NUL
if not errorlevel 1 pause

:end
