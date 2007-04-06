@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo We no longer refresh the old 16-bit program versions on diskette A
goto twoagain

:oneagain
echo Insert FMP diskette A
pause
if exist a:fmp1 goto oneok
echo Wrong diskette?
if exist a:fmp2 goto twook
goto oneagain

:oneok
echo XCOPY c:\prog\*.exe a: /d/p/d
XCOPY c:\prog\*.exe a: /d/p/d
echo XCOPY c:\metrics\*.* a: /d/p/d
XCOPY c:\metrics\*.* a: /d/p/d

:twoagain
echo Insert FMP diskette B
pause
if exist a:fmp2 goto twook
echo Wrong diskette?
if exist a:fmp1 goto oneok
goto twoagain

:twook
echo XCOPY c:\dvisourc\vec\*.vec a:\vec /d/p/d
XCOPY c:\dvisourc\vec\*.vec a:\vec /d/p/d
echo XCOPY c:\dvisourc\vec\*.enc a:\vec /d/p/d
XCOPY c:\dvisourc\vec\*.enc a:\vec /d/p/d
echo XCOPY c:\dvisourc\afm\*.* a:\afm /d/p/d
XCOPY c:\dvisourc\afm\*.* a:\afm /d/p/d
echo XCOPY c:\metrics\*.ren a:\ren /d/p/d
XCOPY c:\metrics\*.ren a:\ren /d/p/d
echo XCOPY c:\metrics\*.* a: /d/p/d
XCOPY c:\metrics\*.* a: /d/p/d
echo XCOPY c:\metrics\*.txt a:\txt /d/p/d
XCOPY c:\metrics\*.txt a:\txt /d/p/d
echo XCOPY c:\txt\*.txt a:\txt /d/p/d
XCOPY c:\txt\*.txt a:\txt /d/p/d

:threeagain
echo Insert FMP diskette C
pause
if exist a:fmp3 goto threeok
echo Wrong diskette?
if exist a:fmp1 goto oneok
if exist a:fmp2 goto twook
goto threeagain

:threeok

echo XCOPY c:\prog\*.exe a:\32bit /d/p/d
XCOPY c:\prog\*.exe a:\32bit /d/p/d
echo XCOPY c:\metrics\*.* a:\32bit /d/p/d
XCOPY c:\metrics\*.* a:\32bit /d/p/d

:end
