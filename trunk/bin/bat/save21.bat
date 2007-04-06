@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if exists d:\texsourc\releas22 goto bad

echo Hiding release 2.1 versions using suffix OLD
copy d:\winsourc\dviwindo.exe d:\winsourc\dviwindo.old
copy d:\dvisourc\dvipsone.exe d:\dvisourc\dvipsone.old
copy d:\dvisourc\dvipream.ps d:\dvisourc\dvipream.old
copy d:\texsourc\yandytex.exe d:\texsourc\yandytex.old

rem echo Copying release 2.2 versions from suffix NEW
rem copy d:\winsourc\dviwindo.new d:\winsourc\dviwindo.exe
rem copy d:\dvisourc\dvipsone.new d:\dvisourc\dvipsone.exe
rem copy d:\dvisourc\dvipream.new d:\dvisourc\dvipream.ps
rem copy d:\texsourc\yandytex.new d:\texsourc\yandytex.exe

echo NOW SAVED RELEASE 2.1 FILES!

goto end

:bad
echo ERROR: APPARENTLY SET UP FOR RELEASE 2.2!

:end
