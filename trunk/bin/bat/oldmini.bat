@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%1" == "" goto argok
echo Assuming you meant winmini b:
winmini b:
echo IMPOSSIBLE ERROR!
goto end

:argok
echo Are you sure you want to use the OLD DVIWindo dump routine?
pause
rem if exist %1\dviwindo.exe goto seemsok
rem if exist %1\prog\dviwindo.ex_ goto seemsok
if exist %1\dviwindo goto seemsok
echo Sorry, apparently not DVIWindo diskette
goto end

:seemsok
echo deleting %1\tex\bpifont.tex to make space...
del %1\tex\bpifont.tex
rem
rem copy only changable part of DVIWindo to distribution diskette
rem
rem if "%1" == "" goto noarg
rem echo copying c:\windev\samples\dviwindo\dviwindo.exe
rem copy c:\windev\samples\dviwindo\dviwindo.exe %1
rem
echo Compressing DVIWindo.EXE
rem c:\windev\bin\compress -r c:\windev\samples\dviwindo\dviwindo.exe .
c:\windev\bin\compress -r c:\windev\samples\dviwindo\dviwindo.exe %1\prog
if errorlevel = 1 pause

echo change back to top-level directory
cd %1\

if not exist %1\lzexpand.dll copy c:\windows\system\lzexpand.dll %1
rem
replace c:\prog\*.exe %1\prog /U/P
rem
replace c:\tex\*.tex %1\tex /U/P
rem
replace c:\txt\*.txt %1\txt /U/P
rem
replace c:\windev\samples\dviwindo\*.txt %1\txt /U/P
rem
replace c:\windev\samples\setup\install.exe %1 /U/P
rem
replace c:\windev\samples\dviwindo\*.inf %1 /U/P
rem
replace c:\dvipsone\*.vec %1\vec /U/P
rem
replace c:\bat\encode.bat %1 /U/P
rem
rem echo Dumping SysSeg
rem c:\windev\bin\compress -r c:\windev\samples\sysseg\sysseg.exe %1
replace c:\windev\samples\sysseg\sysseg.ex_ %1\prog /U/P
rem
rem c:\windev\bin\compress -r c:\windev\samples\cleanup\cleanup.exe %1
replace c:\windev\samples\cleanup\cleanup.ex_ %1\prog /U/P
rem
goto end

:end
rem echo Copying c:\prog\ask.com
rem copy c:\prog\ask.com %1
rem echo Copying c:\utility\cdd.com
rem copy c:\utility\cdd.com %1
if not exist %1\cdd.com copy c:\prog\cdd.com %1
