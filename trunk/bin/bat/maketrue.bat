@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not exist %1.ps goto skiprename
if exist %1.pfa del %1.pfa
echo Renaming %1.ps %1.pfa
rename %1.ps %1.pfa
:skiprename
call makepln %1
trutopln %1.pln
extroutl %1.plx
rem the -n allows it to rescale
rem convert -n %1.out
rem convert -N %1.out
rem convert -N -P %1.out
convert -N %1.out
