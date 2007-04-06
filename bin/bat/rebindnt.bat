@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%STATE%" == "32BIT" call 32bit

echo Rebind yandynt.exe to make yandytex.exe NTSTYLE
echo (This is for new version compiled NTSTYLE and linked using MS tools)
echo Source is yandynt.exe (without TNT) --- destination is yandytex.exe.

pause

c:
cd \texsourc

if exist yandynt.exe goto sourcok
echo Cannot find yandynt.exe
goto end

:sourcok
echo Copying yandynt.exe to yandytex.exe
copy yandynt.exe yandytex.exe

echo Rebinding YandYTeX
rem rebindb yandytex kernel32.dll advapi32.dll user32.dll emutnt.dll
rem rebindb yandytex emutnt.dll nsscon.dll nssio.dll nsspipe.dll
rebindb yandytex emutnt.dll

rem need double % in batch file so get single percent in exe file ...
echo Configuring in YandYTeX environment variable
cfig386 yandytex.exe %%yandytex

echo copydate yandytex.exe yandynt.exe
copydate yandytex.exe yandynt.exe

echo yandytex.exe is the new bound executable

:end
