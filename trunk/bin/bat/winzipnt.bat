@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo ZIP up 32 bit version of DVIWindo
echo pkzip winsou32.zip d:\winsourc\*.c d:\winsourc\*.h
c:\prog\pkzip winsou32.zip d:\winsourc\*.c d:\winsourc\*.h
if errorlevel = 1 goto failed
echo pkzip winsou32.zip d:\winsourc\*.rc d:\winsourc\*.def
c:\prog\pkzip winsou32.zip d:\winsourc\*.rc d:\winsourc\*.def
if errorlevel = 1 goto failed
echo pkzip winsou32.zip d:\winsourc\*.lib d:\winsourc\*.dlg
c:\prog\pkzip winsou32.zip d:\winsourc\*.lib d:\winsourc\*.dlg
if errorlevel = 1 goto failed
echo pkzip winsou32.zip d:\winsourc\*.cur d:\winsourc\*.ico
c:\prog\pkzip winsou32.zip d:\winsourc\*.cur d:\winsourc\*.ico
if errorlevel = 1 goto failed
echo pkzip winsou32.zip d:\winsourc\*.map d:\winsourc\*.res
c:\prog\pkzip winsou32.zip d:\winsourc\*.map d:\winsourc\*.res
if errorlevel = 1 goto failed
echo pkzip winsou32.zip d:\winsourc\makefile
c:\prog\pkzip winsou32.zip d:\winsourc\makefile
if errorlevel = 1 goto failed
rem c:\utility\pkzip winsou32.zip d:\winsourc\dviwindo.exe
goto end

:failed
echo SORRY PKZIP FAILED. PROBABLY OUT OF DISK SPACE

:end
