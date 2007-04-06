@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo ZIP up 16 bit version of DVIPSONE
echo pkzip dvisou16.zip c:\dvisourc\*.c c:\dvisourc\*.h 
c:\prog\pkzip dvisou16.zip c:\dvisourc\*.c c:\dvisourc\*.h 
if errorlevel = 1 goto failed
echo pkzip dvisou16.zip c:\dvisourc\*.ps c:\dvisourc\*.enc 
c:\prog\pkzip dvisou16.zip c:\dvisourc\*.ps c:\dvisourc\*.enc 
if errorlevel = 1 goto failed
echo pkzip dvisou16.zip c:\dvisourc\*.map c:\dvisourc\makefile
c:\prog\pkzip dvisou16.zip c:\dvisourc\*.map c:\dvisourc\makefile
if errorlevel = 1 goto failed
rem c:\utility\pkzip dvisou16.zip c:\dvisourc\dvipsone.exe
goto end

:failed
echo SORRY PKZIP FAILED. PROBABLY OUT OF DISK SPACE

:end
