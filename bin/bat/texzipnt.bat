@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo ZIP up 32 bit version of YANDYTEX
echo pkzip texsou32.zip d:\texsourc\*.c
c:\prog\pkzip texsou32.zip d:\texsourc\*.c
if errorlevel = 1 goto failed
echo pkzip texsou32.zip d:\texsourc\lib\*.c d:\texsourc\lib\*.h d:\texsourc\lib\makefile
c:\prog\pkzip texsou32.zip d:\texsourc\lib\*.c d:\texsourc\lib\*.h d:\texsourc\lib\makefile
if errorlevel = 1 goto failed
echo pkzip texsou32.zip d:\texsourc\lib\*.lib d:\texsourc\fmt\*.poo
c:\prog\pkzip texsou32.zip d:\texsourc\lib\*.lib d:\texsourc\fmt\*.poo
if errorlevel = 1 goto failed
echo pkzip texsou32.zip d:\texsourc\*.map d:\texsourc\*.chr
c:\prog\pkzip texsou32.zip d:\texsourc\*.map d:\texsourc\*.chr
if errorlevel = 1 goto failed
echo pkzip texsou32.zip d:\texsourc\makefile d:\texsourc\yandytex.nt
c:\prog\pkzip texsou32.zip d:\texsourc\makefile d:\texsourc\yandytex.nt
if errorlevel = 1 goto failed
rem c:\utility\pkzip texsou32.zip d:\texsourc\yandytex.exe
goto end

:failed
echo SORRY, PKZIP FAILED.  PROBABLY OUT OF DISK SPACE.

:end
