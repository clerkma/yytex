@echo off

rem sample batch file for iniTeX
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem This is 'initex.bat' conveniently called from DVIWindo 'TeX menu'

rem Two command line arguments: (1) TeX source file, (2) DVIWindo Menu Name
rem e.g. `initex plain Plain'   `initex lplain LaTeX'   `initex latex LaTeX2e'

rem Do *not* use an extension on the source file name (plain.tex say)

rem For this to work as intended, the `[Applications]' section in the file
rem DVIWINDO.INI in the Windows directory must be last.
rem And utilities in their default location.

rem Best to try and switch here to a temporary directory, ideally a RAM disk
if "%TEMP%" == "" goto notemp
rem See whether can find cdd utility supplied with DVIWindo
rem Note: cdd.com is a utility that switches both drive and directory
rem (but does not work for top-level directory in a drive)
if exist C:\yandy\util\cdd.com C:\yandy\util\cdd %TEMP%
if not exist C:\yandy\util\cdd.com cdd %TEMP%
rem If this fails, then please copy cdd.com to somewhere on your PATH
goto notmp

rem if TEMP is not defined, try TMP...
:notemp
if "%TMP%" == "" goto notmp
if exist C:\yandy\util\cdd.com C:\yandy\util\cdd %TMP%
if not exist C:\yandy\util\cdd.com cdd %TMP%
rem If this fails, then please copy cdd.com to somewhere on your PATH

:notmp
rem show current directory so we know for sure where we are
chdir

rem Some sanity checks first!  
if "%TEXFORMATS%" == "" set TEXFORMATS=C:\yandy\yandytex\fmt
if exist %TEXFORMATS%\nul goto fmtexist
if "%TEXFORMATS%" == "" set TEXFORMATS=C:\yandy\yandytex\fmt
echo Sorry, cannot find %TEXFORMATS% directory
set TEXFORMATS=C:\yandy\yandytex\fmt
rem goto endpause

:fmtexist
echo %1 | find "." > NUL
if errorlevel 1 goto noextens
echo Please do not use an extension (%1) when specifying file name
goto endpause

:noextens
rem Find out where the Windows directory is
if not "%windir%" == "" set WINDIR=%windir%
rem Note: decode.exe is a utility supplied with DVIWindo
if exist C:\yandy\util\decode.exe C:\yandy\util\decode -vx > C:\yandy\util\setwindo.bat
if not exist C:\yandy\util\decode.exe decode -vx > C:\yandy\util\setwindo.bat
rem If this fails, then please copy decode.exe to somewhere on your PATH

rem Now call the batch file setwindo.bat that was created by decode.exe
if exist C:\yandy\util\setwindo.bat call C:\yandy\util\setwindo
rem The above sets the environment variable WINDIR
if exist %WINDIR%\dviwindo.ini goto iniexist
echo Sorry, cannot find %WINDIR%\dviwindo.ini
goto endpause

:iniexist
rem deal with use of 'amstex' instead of 'amstex.ini'
if not "%1" == "amstex" goto notamstex
call C:\yandy\yandytex\tex -i amstex.ini
goto inidone

:notamstex
rem deal with the fact that LateX2e requires TWO iniTeX runs

if "%1" == "latex" goto latex
rem and for LaTeX2e beta release
if "%1" == "latex2e" goto latex2e
goto notlatex2e

:latex2e
call C:\yandy\yandytex\tex -i unpack2e.ins
goto latexcom

:latex
call C:\yandy\yandytex\tex -i unpack.ins

:latexcom
if errorlevel = 1 goto inidone
call C:\yandy\yandytex\tex -i %1.ltx
goto inidone

:notlatex2e
rem all other cases are simple, and are handled here
call C:\yandy\yandytex\tex -i %1

:inidone
rem DON'T mess with files if TeX had a cow ...
if not errorlevel = 1 goto seemsok
echo Sorry, TeX appears to be have become unhappy --- formats will not be saved
goto endpause

:seemsok
rem assumes that TEXFORMATS lists a single file directory
echo copying new format file to %TEXFORMATS%
copy %1.log %TEXFORMATS%
copy %1.fmt %TEXFORMATS%

echo deleting new format files from working directory
del %1.log
del %1.fmt

:skipdel
rem This assumes [Applications] section is last in dviwindo.ini
rem deal with 'amstex.ini' special case (want +amstex, not +amstex.ini)
if not "%1" == "amstex.ini" goto notams
rem No need to create TeX Menu entry for AMS TeX (already there)
goto end

:notams
rem deal with 'plain' special case (omit format, since plain is default)
if not "%1" == "plain" goto notplain
rem No need to create TeX Menu entry for plain TeX (already there)
goto end

:notplain
if not "%1" == "lplain" goto not209
rem No need to create TeX Menu entry for LaTeX 2.09
goto end

:not209
if not "%1" == "latex" goto not2e
rem No need to create TeX Menu entry for LaTeX 2e
goto end

:not2e
echo %2=C:\yandy\yandytex\tex.bat +%1 @.tex >> %WINDIR%\dviwindo.ini
goto end

:endpause
rem following trick requires DOS 5 or 6 
rem set | find "windir=" > NUL
rem if not errorlevel 1 pause
:end

rem This assumes a recent version of DVIWindo which can handle @.tex
rem This assumes a recent version of YandY TeX that can handle + instead of &

