@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.
rem
rem Installation batch file for font manipulation package
rem NOTE: this must exist on BOTH diskettes

if "%1" == "" goto usage
if "%2" == "" goto default
goto normal
:default
echo Will install in default directory (c:\fmp)
pause
%1\install %1 c:\fmp
echo An impossible error has occured!

:normal
if exist %1\fmp1 goto disk1
echo %1 does not appear to be FMP disk #1
pause

:disk1
if exist %2\nul goto direxist
echo Creating directory %2
mkdir %2
if not errorlevel 1 goto direxist
echo Sorry, unable to create directory %2
goto usage

:direxist
echo The 30 utility programs (*.exe) are on disk #1
echo Copying disk #1
xcopy %1\*.* %2 /s
echo Finished copying disk #1
echo Please remove disk #1 and insert disk #2
pause
if exist %1\fmp2 goto disk2
echo %1 does not appear to be FMP disk #2
pause

:disk2
echo Copying disk #2
echo Explanatory text files (*.txt) are on disk #2

echo Encoding vector files (vec\*.vec) are on disk #2
echo Sample character renaming files for RENAMECH (ren\*.ren) are on disk #2
echo Sample AFM files showing what is needed for AFMtoTFM are on disk #2 (afm)
echo Clone printer test PostScript test programs (ps\printst*.ps) are on disk #2
echo PS files and batch files for showing character arrays are on disk #2 (ps)

xcopy %1\*.* %2 /s

rem echo Encoding vector files (vec\*.vec) are on disk #2
rem if not exist %2\vec\nul mkdir %2\vec
rem xcopy %1\vec\*.* %2\vec

rem echo Sample character renaming files for RENAMECH (ren\*.ren) are on disk #2
rem if not exist %2\ren\nul mkdir %2\ren
rem xcopy %1\ren\*.* %2\ren

rem echo Sample AFM files showing what is needed for AFMtoTFM are on disk #2 (afm)
rem if not exist %2\afm\nul mkdir %2\afm
rem xcopy %1\afm\*.* %2\afm

rem echo Clone printer test PostScript test programs (ps\printst*.ps) are on disk #2
rem echo PS files and batch files for showing character arrays are on disk #2 (ps)
rem if not exist %2\ps\nul mkdir %2\ps
rem xcopy %1\ps\*.* %2\ps

echo Put %2 on your path to make it easy to use the FMP utilities
echo (or set up suitable batch files to call them directly)

rem set path=%2;%path%
echo ÿ
echo Set up the environment variable VECPATH to point to %2\vec 
echo (add set vecpath=%2\vec to autoexec.bat)
if "vecpath" == "" set vecpath=%2\vec
echo ÿ
echo Please read the `readme.txt' file

goto end
:usage
echo ÿ
echo Usage: [from-drive]:install [from-drive]: [to-drive]:[to-directory]
echo ÿ
echo For example:  b:install  b:  c:\fmp
:end
