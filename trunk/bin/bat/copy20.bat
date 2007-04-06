@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem COPY20 (defaults to a:) or COPY20 a: or COPY20 b:

if "%1" == "-?" goto usage
if "%1" == "/?" goto usage
if "%1" == "" %0 a:
rem if "%1" == "" %0 b:
if "%1" == "a:" goto diskok
if "%1" == "b:" goto diskok
%0 a: %1 %2 %3 %4 %5 %6 %7 %8 %9
rem %0 b: %1 %2 %3 %4 %5 %6 %7 %8 %9

:diskok
break on
cls
echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9

if not exist c:\yyinstal\setup.w02 goto single
if not exist c:\yyinstal\setup.w03 goto single

echo Insert Disk 1 into %1
echo.
:insert1
pause

rem see if disk clean
if not exist %1*.* goto do1

if not exist %1setup.w02 goto notwoa
cls
echo Please remove Disk 2 and insert Disk 1 in %1
echo 
goto insert1

:notwoa
if not exist %1setup.w03 goto nothrea
cls
echo Please remove Disk 3 and insert Disk 1 in %1
echo 
goto insert1

:nothrea
rem don't make a fuss if reusing old `master' diskette #1
if not exist %1\setup.exe echo WARNING: deleting files on disk first
echo y | del %1*.* 
dir %1 /B

:do1
label %1 DISK 1
echo ****Copying Disk 1****
echo.
rem xcopy c:\yyinstal\setup.exe %1 /v
xcopy c:\yyinstal\setup.exe %1
if errorlevel = 1 goto failed
if not exist %1setup.exe goto failed
xcopy c:\yyinstal\copyrght.txt %1
if not "%2" == "S" goto fast1
echo Compare files c:\yyinstal\setup.exe %1\setup.exe
echo n | comp c:\yyinstal\setup.exe %1\setup.exe

:fast1
cls
dir %1 /B
echo.
echo Disk 1 copied; insert Disk 2 into %1
if "%2" == "Q" goto skip2
rem ring the bell
echo 

:skip2
:insert2
echo.
pause

rem see if disk clean
if not exist %1*.* goto do2

if not exist %1setup.exe goto notone
cls
dir %1 /B
echo.
echo Please remove Disk 1 and insert Disk 2 into %1
echo 
goto insert2

:notone
if not exist %1setup.w03 goto nodrei
cls
dir %1 /B
echo.
echo Please remove Disk 3 and insert Disk 2 into %1
echo 
goto insert2

:nodrei
rem don't make a fuss if reusing old `master' diskettes
if not exist %1\setup.w02 echo WARNING: deleting files on disk first
echo y | del %1*.* 
dir %1 /B

:do2
label %1 DISK 2
echo ****Copying Disk 2****
echo.
rem xcopy c:\yyinstal\setup.w02 %1 /v
xcopy c:\yyinstal\setup.w02 %1
if errorlevel = 1 goto failed
if not exist %1setup.w02 goto failed
if not "%2" == "S" goto fast2
echo Compare files c:\yyinstal\setup.w02 %1\setup.w02
echo n | comp c:\yyinstal\setup.w02 %1\setup.w02
:fast2
rem :insert3

cls
dir %1 /B
rem :insert3
echo.
echo Disk 2 copied; insert Disk 3 into %1

:insert3
echo.
if "%2" == "Q" goto skip4
rem ring the bell
echo 
:skip4
pause

rem see if disk clean
if not exist %1*.* goto do3
if not exist %1setup.w02 goto nottwo
cls
dir %1 /B
echo.
echo Please remove Disk 2 and insert Disk 3 into %1
rem if "%2" == "Q" goto skip5
rem ring the bell
echo 
rem :skip5
rem pause
goto insert3

:nottwo
if not exist %1setup.exe goto notown
cls
dir %1 /B
echo.
echo Please remove Disk 1 and insert Disk 3 into %1
rem if "%2" == "Q" goto skip6
rem ring the bell
echo 
rem :skip6
rem pause
goto insert3

:notown
rem don't make a fuss if reusing old `master' diskettes
if not exist %1\setup.w03 echo WARNING: deleting files on disk first
echo y | del %1*.* 
dir %1 /B

:do3
rem following assures that failure due to lack of space is not setup.w03
echo y | del %1\t1instal\*.* 

label %1 DISK 3
echo ****Copying Disk 3****
echo.
rem xcopy c:\yyinstal\setup.w03 %1 /v
xcopy c:\yyinstal\setup.w03 %1
if errorlevel = 1 goto failed
if not exist %1setup.w03 goto failed
if not "%2" == "S" goto fast3
echo Compare files c:\yyinstal\setup.w03 %1\setup.w03
echo n | comp c:\yyinstal\setup.w03 %1\setup.w03
:fast3

rem Now deal with the T1INSTALL.DLL fixups

echo mkdir %1\t1instal
mkdir %1\t1instal
if not exist %1\t1instal\*.*  goto not1ins
echo y | del %1\t1instal\*.*
:not1ins

rem echo goto nospace
rem pause
goto nospace

echo We have plenty of Diskette space so we can do the following:

mkdir %1\t1instal\winnt351

rem echo xcopy d:\t1instal\t1instal.351 %1\t1instal
rem xcopy d:\t1instal\t1instal.351 %1\t1instal
echo xcopy d:\t1instal\winnt351\t1instal.dll %1\t1instal\winnt351
xcopy d:\t1instal\winnt351\t1instal.dll %1\t1instal\winnt351 /v

rem if not "%2" == "S" goto fast4
rem echo Compare files d:\t1instal\t1instal.351 %1\t1instal\t1instal.351
rem echo n | comp d:\t1instal\t1instal.351 %1\t1instal\t1instal.351
rem :fast4

mkdir %1\t1instal\winnt400

rem echo xcopy d:\t1instal\t1instal.400 %1\t1instal
rem xcopy d:\t1instal\t1instal.400 %1\t1instal

echo xcopy d:\t1instal\winnt400\t1instal.dll %1\t1instal\winnt400
xcopy d:\t1instal\winnt400\t1instal.dll %1\t1instal\winnt400 /v

rem if not "%2" == "S" goto fast5
rem echo Compare files d:\t1instal\t1instal.400 %1\t1instal\t1instal.400
rem echo n | comp d:\t1instal\t1instal.400 %1\t1instal\t1instal.400
rem :fast5

goto common

:nospace

echo If we run short of disk space, do the following instead

echo xcopy d:\t1instal\t1ins351.zip %1\t1instal /v
xcopy d:\t1instal\t1ins351.zip %1\t1instal /v

echo xcopy d:\t1instal\t1ins400.zip %1\t1instal /v
xcopy d:\t1instal\t1ins400.zip %1\t1instal /v

goto common

:common
rem echo reached common
rem pause

echo xcopy c:\flyers\t1instal.txt %1\t1instal /v
xcopy c:\flyers\t1instal.txt %1\t1instal /v

rem the following can be dropped if we are out of space

echo xcopy c:\prog\showtabl.exe %1\t1instal /v
xcopy c:\prog\showtabl.exe %1\t1instal /v

cls
dir %1 /B
echo.
echo Fini!
if "%2" == "Q" goto skip7
rem ring the bell
echo 
:skip7
goto end

:failed
echo Sorry, copying to diskette failed!
dir %1
rem if "%2" == "Q" goto skip8
rem ring the bell
echo 
rem :skip8
dir %1
goto end

:usage
echo Usage: copy20 (defaults to a:), or copy20 a:, or copy20 b:
echo Usage: copy20 Q if you want to suppress noise
echo Usage: copy20 S if you want to compare files after copy
goto end

:single
echo The installation file setup.exe is a single file - not split into
echo three parts as required for creating a diskette installation.
echo.
echo Please run WISE compilation again by selecting `Make Floppy' and
echo changing the max file size to 1420 kbytes (rather than 0).
echo.
echo Change it back again afterwards to 0 kbytes for CD installation.

goto end

:end

rem Switched to XCOPY, since COPY does not set ERRORLEVEL on failure.

rem By the way, the del *.* takes care of top level files
rem so there could be problems if directories in diskette
rem That is why I added the dir %1 lines...
rem Maybe use deltree %1 ?  But then may need multiply y responses ...
