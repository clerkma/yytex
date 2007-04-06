@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not exist c:\prog\%1.exe echo c:\prog\%1.exe does not exist
if not exist c:\c\%1.c echo c:\c\%1.c does not exist
dir c:\c\%1.c /B
echo copydate c:\prog\%1.exe c:\c\%1.c
copydate c:\prog\%1.exe c:\c\%1.c
dir c:\prog\%1.exe /B
