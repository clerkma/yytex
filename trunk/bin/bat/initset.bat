@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
rem This file copies c:\yyinstal\setup_pfe.wse or c:\yyinstal\setup_winedt.wse
rem or c:\yyinstal\setup_winedt32.wse into c:\yyinstal\setup.wse, depending on initial parameter.
rem syntax: initset %1 where %1 is "PFE" or "WINEDT" or "WINEDT32"
if "%1"=="" goto error
if "%1"=="PFE" goto execute
if "%1"=="pfe" goto execute
if "%1"=="WINEDT" goto execute
if "%1"=="winedt" goto execute
if "%1"=="WINEDT32" goto execute
if "%1"=="winedt32" goto execute
goto error

:execute
echo copy c:\yyinstal\setup_%1.wse c:\yyinstal\setup.wse
copy c:\yyinstal\setup_%1.wse c:\yyinstal\setup.wse
goto end

:error
echo.
echo This file copies c:\yyinstal\setup_pfe.wse or c:\yyinstal\setup_winedt.wse
echo or c:\yyinstal\setup_winedt32.wse into c:\yyinstal\setup.wse, depending on initial parameter.
echo syntax: initset * where * is "PFE" or "WINEDT" or "WINEDT32"
echo.
echo syntax: initset PFE or initset WINEDT or initset WINEDT32
echo.
pause
goto end

:end
rem pause
cls
