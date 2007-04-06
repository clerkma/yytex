@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if exists d:\texsourc\releas21 goto bad

echo Hiding release 2.2 versions using suffix NEW
copy d:\winsourc\dviwindo.exe d:\winsourc\dviwindo.new
copy d:\dvisourc\dvipsone.exe d:\dvisourc\dvipsone.new
copy d:\dvisourc\dvipream.ps d:\dvisourc\dvipream.new
copy d:\texsourc\yandytex.exe d:\texsourc\yandytex.new

rem echo  Copying release 2.1 versions from suffix OLD
rem copy d:\winsourc\dviwindo.old d:\winsourc\dviwindo.exe
rem copy d:\dvisourc\dvipsone.old d:\dvisourc\dvipsone.exe
rem copy d:\dvisourc\dvipream.old d:\dvisourc\dvipream.ps
rem copy d:\texsourc\yandytex.old d:\texsourc\yandytex.exe

echo NOW SAVED FOR RELEASE 2.2 FILES!

goto end

:bad
echo ERROR: APPARENTLY SET UP FOR RELEASE 2.1!

:end
