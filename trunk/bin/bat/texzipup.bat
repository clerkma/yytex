@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo ZIP up 16 bit version of YANDYTEX
echo pkzip texsou16.zip c:\texsourc\*.c
c:\prog\pkzip texsou16.zip c:\texsourc\*.c
if errorlevel = 1 goto failed
echo pkzip texsou16.zip c:\texsourc\lib\*.c c:\texsourc\lib\*.h c:\texsourc\lib\makefile
c:\prog\pkzip texsou16.zip c:\texsourc\lib\*.c c:\texsourc\lib\*.h c:\texsourc\lib\makefile
if errorlevel = 1 goto failed
echo pkzip texsou16.zip c:\texsourc\lib\*.lib c:\texsourc\fmt\*.poo
c:\prog\pkzip texsou16.zip c:\texsourc\lib\*.lib c:\texsourc\fmt\*.poo
if errorlevel = 1 goto failed
echo pkzip texsou16.zip c:\texsourc\*.map c:\texsourc\*.chr
c:\prog\pkzip texsou16.zip c:\texsourc\*.map c:\texsourc\*.chr
if errorlevel = 1 goto failed
echo pkzip texsou16.zip c:\texsourc\makefile c:\texsourc\yandytex.nt
c:\prog\pkzip texsou16.zip c:\texsourc\makefile c:\texsourc\yandytex.nt
if errorlevel = 1 goto failed
rem c:\utility\pkzip texsou16.zip c:\texsourc\yandytex.exe
goto end

:failed
echo SORRY, PKZIP FAILED.  PROBABLY OUT OF DISK SPACE.

:end
