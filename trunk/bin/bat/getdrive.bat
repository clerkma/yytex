@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

curdrive
if errorlevel 0 if not errorlevel 1 set current_drive=a
if errorlevel 1 if not errorlevel 2 set current_drive=b
if errorlevel 2 if not errorlevel 3 set current_drive=c
if errorlevel 3 if not errorlevel 4 set current_drive=d
if errorlevel 4 if not errorlevel 5 set current_drive=e
