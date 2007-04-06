@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo ZIP up 16 bit version of DVIWindo
echo pkzip winsou16.zip c:\winsourc\*.c c:\winsourc\*.h
c:\prog\pkzip winsou16.zip c:\winsourc\*.c c:\winsourc\*.h
if errorlevel = 1 goto failed
echo pkzip winsou16.zip c:\winsourc\*.rc c:\winsourc\*.def
c:\prog\pkzip winsou16.zip c:\winsourc\*.rc c:\winsourc\*.def
if errorlevel = 1 goto failed
echo pkzip winsou16.zip c:\winsourc\*.lib c:\winsourc\*.dlg
c:\prog\pkzip winsou16.zip c:\winsourc\*.lib c:\winsourc\*.dlg
if errorlevel = 1 goto failed
echo pkzip winsou16.zip c:\winsourc\*.cur c:\winsourc\*.ico
c:\prog\pkzip winsou16.zip c:\winsourc\*.cur c:\winsourc\*.ico
if errorlevel = 1 goto failed
echo pkzip winsou16.zip c:\winsourc\*.map c:\winsourc\*.res
c:\prog\pkzip winsou16.zip c:\winsourc\*.map c:\winsourc\*.res
if errorlevel = 1 goto failed
echo pkzip winsou16.zip c:\winsourc\makefile
c:\prog\pkzip winsou16.zip c:\winsourc\makefile
if errorlevel = 1 goto failed
rem c:\utility\pkzip winsou16.zip c:\winsourc\dviwindo.exe
goto end

:failed
echo SORRY PKZIP FAILED. PROBABLY OUT OF DISK SPACE

:end
