@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem if "%STATE%" == "32BIT" goto done

rem if "%SETVCENV%" == "1" echo SETVCENV == 1 already

if "%SETVCENV%" == "1" goto end

echo WARNING: SETTING UP FOR MS VC 6.0 (on drive G:)
echo call "g:\Microsoft Visual Studio\vc98\Bin\vcvars32.bat"
call "g:\Microsoft Visual Studio\vc98\Bin\vcvars32.bat"
rem set PATH=%PATH%;c:\bat

rem echo WARNING: SETTING UP FOR MS SDK (on drive E:)
rem echo call e:\mssdk\setenv.bat e:\mssdk
rem call e:\mssdk\setenv.bat e:\mssdk

set SETVCENV=1

goto end

:end
