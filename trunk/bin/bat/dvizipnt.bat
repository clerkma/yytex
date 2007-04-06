@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo ZIP up 32 bit version of DVIPSONE
echo pkzip dvisou32.zip d:\dvisourc\*.c d:\dvisourc\*.h 
c:\prog\pkzip dvisou32.zip d:\dvisourc\*.c d:\dvisourc\*.h 
if errorlevel = 1 goto failed
echo pkzip dvisou32.zip d:\dvisourc\*.ps d:\dvisourc\*.enc 
c:\prog\pkzip dvisou32.zip d:\dvisourc\*.ps d:\dvisourc\*.enc 
if errorlevel = 1 goto failed
echo pkzip dvisou32.zip d:\dvisourc\*.map d:\dvisourc\makefile
c:\prog\pkzip dvisou32.zip d:\dvisourc\*.map d:\dvisourc\makefile
if errorlevel = 1 goto failed
rem c:\utility\pkzip dvisou32.zip d:\dvisourc\dvipsone.exe
goto end

:failed
echo SORRY PKZIP FAILED. PROBABLY OUT OF DISK SPACE

:end
