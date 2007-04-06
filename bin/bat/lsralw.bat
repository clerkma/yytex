@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

c:\dvialw\dvialw %1.dvi -b
rem pause Print the file now? (Type CNTRL-C, and then Y, if NOT)
echo Print the file? [Y}
ask
if error-level==2 goto yes-label
if error-level==1 goto enter-label
goto no-label
:yes-label
:no-label
copy %1.alw lpt1:/b
:no-label
@echo on
