@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%STATE%" == "16BIT" call 16bit 
c:
cd \dvisourc
nmake
rem cd \
rem d:
