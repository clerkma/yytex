rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

@echo off
rem install a: [yandytex dir] [dvipsone dir] [dviwindo dir]
rem %0      %1 %2             %3             %4

if "%1" == "-?" goto usage
if "%1" == "?" goto usage

if not "%1" == "" goto diskok
echo Will try to locate directories for YANDYTEX, DVIPSONE, and DVIWINDO
echo If this should fail try  a:install -?
if exist a:install.bat %0 a:
if exist b:install.bat %0 b:
echo Sorry, can't find installation diskette
goto usage

:diskok
if not "%2" == "" goto texok
if exist c:\yandytex\yandytex.exe %0 %1 c:\yandytex
rem if exist c:\y&ytex\y&ytex.exe %0 %1 c:\y&ytex\y&ytex.exe
if exist c:\y&ytex\y&ytex.exe goto rename
echo Sorry, can't find old yandytex.exe
echo You may need to rename the old executable or the old directory
echo (see readme.txt for details).
goto usage

:texok
if not "%3" == "" goto dvipsok
if exist c:\dvipsone\dvipsone.exe %0 %1 %2 c:\dvipsone
echo Sorry, can't find old dvipsone.exe
goto usage

:dvipsok
if not "%4" == "" goto dviwinok
if exist c:\dviwindo\dviwindo.exe %0 %1 %2 %3 c:\dviwindo
echo Sorry, can't find old dviwindo.exe
goto usage

:dviwinok
:askversion
echo This installation batch file will replace YANDYTEX.EXE, DVIPSONE.EXE, 
echo and DVIWINDO.EXE with new versions that support long file names.
echo.
echo You have a choice of installing TeX 3.141 or TeX 3.14159
echo (both are 32 bit console applications and support long file names).
echo If you install TeX 3.14159 you will have to remake the format files
echo (for anything but plain TeX) using ini TeX (see readme.txt).
echo You can use your existing format files if you instead install TeX 3.141
echo.
pause
echo You presently appear to have the following format files in %2\fmt:
dir %2\fmt\*.fmt /W
echo You can install TeX 3.141 now and install TeX 3.14159 later.
echo.
echo Do you want to install the new TeX 3.14159 now [Y or N]?
%1\ask
rem errorlevel is 2 for Y
if errorlevel 2 if not errorlevel 3 goto tex159
rem errorlevel is 1 for Enter
if errorlevel 1 if not errorlevel 2 goto tex141
rem errorlevel is 0 for N
if errorlevel 0 if not errorlevel 1 goto tex141
rem goto tex141
echo.
goto askversion

:tex141
echo.
echo Installing yandytex.exe for TeX 3.141
if exist %2\yandytex.old del %2\yandytex.old
if exist %2\yandytex.exe rename %2\yandytex.exe yandytex.old
echo.
echo Copying YANDYTEX.EXE (TeX 3.141) to %2
copy %1\yandy12.exe %2
rename %2\yandy12.exe yandytex.exe
echo You have installed TeX 3.141.  Your format files need not be remade
echo (do *not* use the new plain.tex or tex.pool files on the diskette).
goto texcopied

:tex159
echo.
echo Installing yandytex.exe for TeX 3.14159
echo.
echo WARNING: you will have to recreate format files if you continue.
echo You presently appear to have the following format files:
dir %2\fmt\*.fmt /W
echo.
echo Press Ctrl-C now to exit installation.
pause
if exist %2\yandytex.old del %2\yandytex.old
if exist %2\yandytex.exe rename %2\yandytex.exe yandytex.old
echo.
echo Copying YANDYTEX.EXE (TeX 3.14159) to %2
copy %1\yandynt.exe %2
rename %2\yandynt.exe yandytex.exe
if exist %2\fmt\plain.old del %2\fmt\plain.old
if exist %2\fmt\plain.fmt rename %2\fmt\plain.fmt plain.old
if exist %2\fmt\tex.old del %2\fmt\tex.old
if exist %2\fmt\tex.poo rename %2\fmt\tex.poo tex.old
if exist %2\fmt\plain.log del %2\fmt\plain.log
echo Copying PLAIN.FMT to %2\fmt
copy %1\plain.fmt %2\fmt
rem echo Copying PLAIN.LOG to %2\fmt
if exist %1\plain.log echo Copying PLAIN.LOG to %2\fmt
rem copy %1\plain.log %2\fmt
if exist %1\plain.log copy %1\plain.log %2\fmt
echo Copying TEX.POO (tex.pool) to %2\fmt
copy %1\tex.poo %2\fmt
rem rename %2\fmt\tex.poo tex.pool

echo NOTE: you have installed TeX 3.14159
echo The new plain TeX format has been copied to your hard disk,
echo but you will have to remake other formats (e.g. LaTeX 2.09 and LaTeX 2e)
echo using ini-TeX (see readme.txt as well as the System Manual for details).
echo.
echo The file plain.tex on the diskette is the source for the new plain TeX.
echo To use this when creating other new formats copy it to one of your
echo TEXINPUTS directories.
goto texcopied

:texcopied
if exist %3\dvipsone.old del %3\dvipsone.old
if exist %3\dvipream.old del %3\dvipream.old
if exist %3\dvipsone.exe rename %3\dvipsone.exe dvipsone.old
if exist %3\dvipream.enc rename %3\dvipream.enc dvipream.old
echo.
echo Copying DVIPSONE.EXE to %3
copy %1\dvipsont.exe %3
rename %3\dvipsont.exe dvipsone.exe
echo Copying DVIPREAM.ENC to %3
copy %1\dvipream.enc %3
goto dvicopied

:dvicopied
if exist %4\dviwindo.old del %4\dviwindo.old
if exist %4\dviwindo.exe rename %4\dviwindo.exe dviwindo.old
echo.
echo Copying DVIWINDO.EXE to %4
copy %1\dviwindo.exe %4
goto wincopied

:wincopied
echo.
echo Installation of long file name upgrade complete.
echo You can now use long file names for TeX source files.  In addition,
echo you will see long file names in File Open dialog if you add the line
echo	LongNames=1
echo to the [Window] section of dviwindo.ini (see readme.txt file).

goto end

:rename
echo You have a file called y&ytex.exe in a directory c:\y&ytex
echo Please rename the file yandytex.exe and rename the directory yandytex.
echo.
echo Then change all occurences of y&ytex to yandytex in the dviwindo.ini file
echo in your Windows directory, and change references to y&ytex in tex.bat,
echo which can be found in the c:\bat directory (or top-level on c: drive).
echo.
echo Retry after verifying that your modified old system still works correctly.
goto end

:usage
echo.
echo Usage: install  [diskette]  [yandytex dir]  [dvipsone dir]  [dviwindo dir]
echo.
echo e.g. a:install  a:  c:\yandytex  c:\dvipone  c:\dviwindo
echo.
echo The above sample will be used as defaults if no arguments are supplied
echo.

:end
