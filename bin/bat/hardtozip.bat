@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
cd 

if not "%1" == "" goto argok
if exist r:\50mz %0 r:
if exist e:\50mz %0 e:
if exist f:\50mz %0 f:
if exist h:\50mz %0 h:
if exist i:\50mz %0 i:

echo SORRY - can't find file 50MZ on drive e: f: h: or i:
goto end

:argok
echo from hard drive c: and d: to  ZIP drive %1

echo DRIVE C:
echo XCOPY c:\c\*.* %1\c /d/u/p
XCOPY c:\c\*.* %1\c /d/u/p
echo XCOPY c:\c\*.c %1\c /d/p
XCOPY c:\c\*.c %1\c /d/p
echo XCOPY c:\c32\*.* %1\c32 /d/u/p
XCOPY c:\c32\*.* %1\c32 /d/u/p
echo XCOPY c:\c32\*.c %1\c32 /d/p
XCOPY c:\c32\*.c %1\c32 /d/p
echo XCOPY c:\flyers\*.* %1\flyers /d/u/p
XCOPY c:\flyers\*.* %1\flyers /d/u/p
echo XCOPY c:\flyers\*.txt %1\flyers /d/p
XCOPY c:\flyers\*.txt %1\flyers /d/p
echo XCOPY c:\metrics\*.* %1\metrics /d/u/p
XCOPY c:\metrics\*.* %1\metrics /d/u/p
echo XCOPY c:\prog\*.* %1\prog /d/u/p
XCOPY c:\prog\*.* %1\prog /d/u/p
echo XCOPY c:\ps\*.* %1\ps /d/u/p
XCOPY c:\ps\*.* %1\ps /d/u/p
echo XCOPY c:\tex\*.* %1\tex /d/u/p
XCOPY c:\tex\*.* %1\tex /d/u/p
echo XCOPY c:\txt\*.* %1\txt /d/u/p
XCOPY c:\txt\*.* %1\txt /d/u/p
echo XCOPY c:\dvisourc\vec\*.*  %1\vec /d/u/p
XCOPY c:\dvisourc\vec\*.*  %1\vec /d/u/p
echo XCOPY c:\dvisourc\vec\*.vec  %1\vec /d/p
XCOPY c:\dvisourc\vec\*.vec  %1\vec /d/p
echo XCOPY c:\bat\*.*  %1\bat /d/u/p
XCOPY c:\bat\*.*  %1\bat /d/u/p

echo DRIVE D:
echo XCOPY d:\dvisourc\*.* %1\dvisourc /d/u/p
XCOPY d:\dvisourc\*.* %1\dvisourc /d/u/p
echo XCOPY d:\dvisourc\*.c %1\dvisourc /d/p
XCOPY d:\dvisourc\*.c %1\dvisourc /d/p
echo XCOPY d:\dvisourc\*.h %1\dvisourc /d/p
XCOPY d:\dvisourc\*.h %1\dvisourc /d/p

echo XCOPY d:\texsourc\*.* %1\texsourc /d/u/p
XCOPY d:\texsourc\*.* %1\texsourc /d/u/p
echo XCOPY d:\texsourc\*.c %1\texsourc /d/p
XCOPY d:\texsourc\*.c %1\texsourc /d/p
echo XCOPY d:\texsourc\lib\*.* %1\texsourc\lib /d/u/p
XCOPY d:\texsourc\lib\*.* %1\texsourc\lib /d/u/p
echo XCOPY d:\texsourc\lib\*.c %1\texsourc\lib /d/p
XCOPY d:\texsourc\lib\*.c %1\texsourc\lib /d/p
echo XCOPY d:\texsourc\lib\*.h %1\texsourc\lib /d/p
XCOPY d:\texsourc\lib\*.h %1\texsourc\lib /d/p

echo XCOPY d:\winsourc\*.* %1\winsourc /d/u/p
XCOPY d:\winsourc\*.* %1\winsourc /d/u/p
echo XCOPY d:\winsourc\*.c %1\winsourc /d/p
XCOPY d:\winsourc\*.c %1\winsourc /d/p
echo XCOPY d:\winsourc\*.h %1\winsourc /d/p
XCOPY d:\winsourc\*.h %1\winsourc /d/p
echo XCOPY d:\winsourc\*.rc %1\winsourc /d/p
XCOPY d:\winsourc\*.rc %1\winsourc /d/p

:end
