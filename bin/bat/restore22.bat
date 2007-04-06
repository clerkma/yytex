@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem echo Hiding release 2.1 versions using suffix OLD
rem copy d:\winsourc\dviwindo.exe d:\winsourc\dviwindo.old
rem copy d:\dvisourc\dvipsone.exe d:\dvisourc\dvipsone.old
rem copy d:\dvisourc\dvipream.ps d:\dvisourc\dvipream.old
rem copy d:\texsourc\yandytex.exe d:\texsourc\yandytex.old

echo Copying release 2.2 versions from suffix NEW
copy d:\winsourc\dviwindo.new d:\winsourc\dviwindo.exe
copy d:\dvisourc\dvipsone.new d:\dvisourc\dvipsone.exe
copy d:\dvisourc\dvipream.new d:\dvisourc\dvipream.ps
copy d:\texsourc\yandytex.new d:\texsourc\yandytex.exe

echo RELEASE22 > d:\texsourc\release22
del d:\texsourc\release21

echo NOW SETUP FOR RELEASE 2.2 FILES!
