@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem xcopy c:\dvipsone\*.c a:
savefile c:\dvipsone\*.c a:
rem xcopy c:\dvipsone\*.h a:
savefile c:\dvipsone\*.h a:
rem xcopy c:\dvipsone\*.ps a:
savefile c:\dvipsone\*.ps a:
rem xcopy c:\dvipsone\*.txt a:
savefile c:\dvipsone\*.txt a:
echo Vectors now in vec sub-directory
if not exist a:\vec\nul mkdir a:\vec
rem xcopy c:\dvipsone\*.vec a:
rem savefile c:\dvipsone\*.vec a:
savefile c:\dvipsone\*.vec a:\vec
rem xcopy c:\dvipsone\*.sub a:
echo Substitution files now in sub sub-directory
if not exist a:\sub\nul mkdir a:\sub
rem savefile c:\dvipsone\*.sub a:
savefile c:\dvipsone\*.sub a:\sub
@echo on
