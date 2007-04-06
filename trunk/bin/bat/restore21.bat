@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem echo Hiding release 2.2 versions using suffix NEW
rem copy d:\winsourc\dviwindo.exe d:\winsourc\dviwindo.new
rem copy d:\dvisourc\dvipsone.exe d:\dvisourc\dvipsone.new
rem copy d:\dvisourc\dvipream.ps d:\dvisourc\dvipream.new
rem copy d:\texsourc\yandytex.exe d:\texsourc\yandytex.new

echo Copying release 2.1 versions from suffix OLD
copy d:\winsourc\dviwindo.old d:\winsourc\dviwindo.exe
copy d:\dvisourc\dvipsone.old d:\dvisourc\dvipsone.exe
copy d:\dvisourc\dvipream.old d:\dvisourc\dvipream.ps
copy d:\texsourc\yandytex.old d:\texsourc\yandytex.exe

echo RELEASE21 > d:\texsourc\release21
del d:\texsourc\release22

echo NOW SETUP FOR RELEASE 2.1 FILES!
