@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%STATE%" == "32BIT" call 32bit

echo Rebind yandynt.exe to make yandytex.exe DOSSTYLE
echo Are you sure you want to do this (for old version compiled DOSSTYLE)
pause

c:
cd \texsourc

echo Copying yandynt.exe to yandytex.exe
copy yandynt.exe yandytex.exe

echo Rebinding YandYTeX
rem rebindb yandytex kernel32.dll advapi32.dll user32.dll emutnt.dll
rebindb yandytex emutnt.dll kernel32.dll advapi32.dll user32.dll

rem need double % in batch file so get single percent in exe file ...
echo Configuring in YandYTeX environment variable
cfig386 yandytex.exe %%yandytex
