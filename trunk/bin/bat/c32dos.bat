@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%1" == "" goto end
if exist c:\c32\%1.c goto compile
echo file c:\c32\%1.c does not exist
goto end
:compile
if not "%STATE%" == "32BIT" call 32bit 
echo Compiling DOS Style
rem cl /c c:\c32\%1.c
cl /c -Gf c:\c32\%1.c
if errorlevel == 1 goto end
386link @msvc32.dos %1
:end
