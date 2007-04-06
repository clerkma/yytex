@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo Restoring old version of dviwindo.exe
del d:\winsourc\dviwindo.exe
copy d:\winsourc\dviwindo.317 d:\winsourc\dviwindo.exe 

echo Restoring old version of dvipsone.exe
del d:\dvisourc\dvipsone.exe
copy d:\dvisourc\dvipsone.217 d:\dvisourc\dvipsone.exe

echo Restoring old version of dvipream.ps
del d:\dvisourc\dvipream.ps
del d:\dvisourc\dvipream.enc
copy d:\dvisourc\dvipream.217 d:\dvisourc\dvipream.ps

goto end

echo Restoring old version of yandytex.exe
del d:\texsourc\yandytex.exe
copy d:\texsourc\yandytex.217 d:\texsourc\yandytex.exe

:end
