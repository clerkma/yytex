@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.
rem
rem This runs the MD5 "message digest" algorithm on files in a given tree
rem
rem If no argument is given, assume diskette drive
rem
if "%1" == "" %0 a:
rem
echo RUN MD5 on all files (recursively) at %1
rem
FOR /R %1 %%F IN (*.*) DO md5.exe %%F
