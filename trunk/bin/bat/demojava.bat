@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

d:
cd \java
chdir
rem echo d:\java\bin\appletviewer.exe d:/java/demo/%1
echo bin\appletviewer.exe demo\%1
rem d:\java\bin\appletviewer.exe d:/java/demo/%1
bin\appletviewer.exe demo\%1
