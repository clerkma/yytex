@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem Batch file to switch to differently encoded TFM files
rem in Y&Y TeX System release 2.0

rem Usage:  encodetfm  [new encoding]   [YandY base dir]  [T1 | TT | BOTH]
rem e.g.  encodetfm  texnansi  c:\yandy  T1
rem where  [new encoding]  should be texnansi, ansinew, cork, or standard
rem The last argument specifies whether to make the switch for
rem Type 1 fonts, TrueType fonts, or both.

rem In Windows 95, use T1 as last argument, since TrueType fonts cannot be
rem reencoded in Windows 95.
rem In Windows NT, use BOTH since DVIWindo can reencode both Type 1
rem and TrueType in Windows NT 4.0

rem show arguments

echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 

rem check for required arguments

if "%1" == "-?" goto usage
if "%1" == "" goto usage

rem check whether base folder specified
if not "%2" == "" goto argok

rem check for default location
if exist c:\yandy\fonts\tfm\*.* %0 %1 c:\yandy
echo Sorry, can't find Y and Y base directory
echo.
goto usage

:argok
rem check whether last argument given
if not "%3" == "" goto lastok

rem check whether OS, SYSTEMROOT or SYSTEMDRIVE defined for NT

if "OS" == "Windows_NT" %0 %1 %2 BOTH

rem check whether windir or winbootdir for 95

if not "windir" == "" %0 %1 %2 T1

echo Please specify T1, TT, or BOTH for last argument
echo.
goto usage

:lastok
rem allow for alternate spellings ...

if "%1" == "TeXnANSI" %0 texnansi %2 %3
if "%1" == "TEXNANSI" %0 texnansi %2 %3
if "%1" == "ansi" %0 ansinew %2 %3
if "%1" == "ANSI" %0 ansinew %2 %3
if "%1" == "Cork" %0 cork %2 %3
if "%1" == "T1" %0 cork %2 %3
if "%1" == "tex256" %0 cork %2 %3
if "%1" == "Standard" %0 standard %2 %3
if "%1" == "StandardEncoding" %0 standard %2 %3
if "%1" == "ASE" %0 standard %2 %3

rem show final arguments

rem echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9

rem check valid encoding name

if "%1" == "texnansi" goto encok
if "%1" == "ansinew" goto encok
if "%1" == "cork" goto encok
if "%1" == "standard" goto encok
echo Sorry, encoding %1 not supported by this batch file.
echo.
goto usage

:encok
rem sanity check --- existence of required directories

if "%3" == "T1" goto dot1
if "%3" == "BOTH" goto dot1
goto skipt1

:dot1
if not exist %2\fonts\tfm-lb\*.* goto missing
if not exist %2\fonts\tfm-ps\*.* goto missing

if exist %2\fonts\tfm-ps\%1\*.* goto copyps
echo ERROR No %2\fonts\tfm-ps\%1 directory (or no TFM files there)
echo.
goto missing

:copyps
echo Copying %1 encoded TFM files for Times-Roman, Helvetica, and Courier
copy %2\fonts\tfm-ps\%1\*.tfm %2\fonts\tfm-ps

if exist %2\fonts\tfm-lb\%1\*.* goto copylb
echo ERROR No %2\fonts\tfm-lb\%1 directory (or no TFM files there)
echo.
goto missing

:copylb
echo Copying %1 encoded TFM files for Lucida Bright text fonts
copy %2\fonts\tfm-lb\%1\*.tfm %2\fonts\tfm-lb

if exist %2\texinput\mt\*.* goto mtok
echo ERROR No %2\texinput\mt directory (or no TeX files there)
goto missing

:mtok
echo Copying encode.tex file for MathTime for %1 encoding
if "%1" == "texnansi" copy %2\texinput\mt\encodetx.tex %2\texinput\encode.tex
if "%1" == "ansinew" copy %2\texinput\mt\encodean.tex %2\texinput\encode.tex
if "%1" == "cork" copy %2\texinput\mt\encodedc.tex %2\texinput\encode.tex
if "%1" == "standard" copy %2\texinput\mt\encodese.tex %2\texinput\encode.tex

echo TFM files for %1 encoding installed for Times, Helvetica, and Courier
echo TFM files for %1 encoding installed for Lucida Bright text fonts
echo.

:skipt1
if "%3" == "TT" goto dott
if "%3" == "BOTH" goto dott
goto skiptt

:dott
if not exist %2\fonts\tfm-ttf\*.* goto missing

if exist %2\fonts\tfm-ttf\%1\*.* goto copyttf
echo ERROR No %2\fonts\tfm-ttf\%1 directory (or no TFM files there)
goto missing

:copyttf
echo Copying %1 encoded TFM files for Times New Roman, Arial, and Courier New
copy %2\fonts\tfm-ttf\%1\*.tfm %2\fonts\tfm-ttf

echo TFM files for %1 encoding installed for Times New Roman, Arial, Courier New
echo.

:skiptt
:done
rem leave a record of which encoding the TFM files are for ...
if exist ansinew del ansinew
if exist texnansi del texnansi
if exist cork del cork
if exist standard del standard
echo %1 > %1

echo.
echo Now make *sure* to *also*:
echo.
if not "%1" == "ansinew" goto notansi
echo (1) add TEXANSI=1 to [Environment] section of dviwindo.ini
echo     (and comment out any ENCODING=... entries)
goto common

:notansi
echo (1) change ENCODING=... entry in [Environment] section of dviwindo.ini
echo     in the windows directory.  It should read ENCODING=%1
echo     (and comment out any TEXANSI=1 entries)
goto common

:common
echo.
echo (2) change your TeX source files to include
if "%1" == "texnansi" echo \input texnansi
if "%1" == "ansinew" echo \input ansiacce
if "%1" == "cork" echo \input dcaccent
rem if "%1" == "cork" echo \usepackage[T1]{inputenc}
if "%1" == "standard" echo \input stanacce
goto end

:missing
echo Sorry the TFM files appear not to be at %2\fonts\tfm...
echo (This batch file only works with Y and Y TeX System release 2.0)
echo.
goto usage

:usage
echo To switch to TFM files for a new encoding vector for
echo Times Roman, Helvetica, and Courier and Lucida Bright text fonts,
echo and Times New Roman, Arial, and Courier New TrueType fonts.
echo.
echo %0 [new encoding]  [YandY base dir] [T1 or TT or BOTH]
echo.
echo e.g. %0  texnansi  c:\yandy  T1
echo.
echo Here [new encoding] can be texnansi, ansinew, cork, or standard
echo (for TeX 'n ANSI, Windows ANSI, Cork (TeX T1) or Adobe StandardEcoding). 
echo.
echo Note: TeX 'n ANSI  recommended for systems with reencoding
echo Note: Windows ANSI recommended for systems without reencoding
echo.
echo The last argument specifies whether to switch Type 1 fonts,
echo TrueType fonts or both.  Use T1 on Windows 95 and BOTH on NT 4.0

:end
