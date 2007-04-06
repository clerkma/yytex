@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem %0 [Old File] [New File] [New PS Name] [New Face Name] [Encoding] [Composites]
rem %0    %1         %2          %3             %4           %5       %6
rem %0   tii_____     tiix    TimesX-Italic   TimesX       cp1257  latvian

if "%1" == "" goto usage
if "%2" == "" %0 %1 testfont
if "%3" == "" %0 %1 %2 TestFont
if "%4" == "" %0 %1 %2 %3 TestFont
if "%5" == "" %0 %1 %2 %3 %4 cp1257
if "%6" == "" %0 %1 %2 %3 %4 %5 latvian

chdir
echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9

REM --- Convert font
echo pfbtopfa -v -d=%2 %1.pfb
pfbtopfa -v -d=%2 %1.pfb
if errorlevel = 1 goto abort
echo composit -v %2.pfa %6.cc
c:\metrics\composit -v %2.pfa %6.cc
if errorlevel = 1 goto abort
echo reencode -v -x -c=%5 -r=%3 %2.pfa
reencode -v -x -c=%5 -r=%3 %2.pfa
if errorlevel = 1 goto abort
echo safeseac -v %2.pfa
c:\metrics\safeseac -v %2.pfa
if errorlevel = 1 goto abort
echo pfatopfb -v %2.pfa
pfatopfb -v %2.pfa
if errorlevel = 1 goto abort

REM --- Convert metrics
echo pfmtoafm -v %1.pfm
pfmtoafm -v %1.pfm
if errorlevel = 1 goto abort
echo pfatoafm -v -a=%1.afm %2.pfa
c:\metrics\pfatoafm -v -a=%1.afm %2.pfa
if errorlevel = 1 goto abort
echo afmtopfm -vsd -x -c=%5 -w=%4 %2.afm
afmtopfm -vsd -x -c=%5 -w=%4 %2.afm
if errorlevel = 1 goto abort
echo Success
goto end

:usage
echo Usage:
echo.
echo %0 [Old File] [New File] [New PS Name] [New Face Name] [Encoding] [Composites]
echo.
echo e.g.  %0  tii_____  tiix  TimesX-Italic  TimesX  cp1257  latvian
echo.
goto end

:abort
echo ERROR

:end

rem gunars tir_____ tirx TimesX-Roman TimesX cp1257 latvian
rem gunars tii_____ tiix TimesX-Italic TimesX cp1257 latvian 
rem gunars tib_____ tibx TimesX-Bold TimesX cp1257 latvian 
rem gunars tibi____ tibix TimesX-BoldItalic TimesX cp1257 latvian 
