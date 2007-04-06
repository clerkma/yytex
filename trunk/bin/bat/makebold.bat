@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem Make bold math for AMS fonts
rem
if "%1" == "F" goto fontone
if "%1" == "C" goto convert
goto fontone
rem
:convert
rem
convert -vu c:\euler\cmbsy5.out
convert -vu c:\euler\cmbsy7.out
convert -vu c:\euler\cmmib5.out
convert -vu c:\euler\cmmib7.out
convert -vu c:\euler\msam5.out
convert -vu c:\euler\msam7.out
convert -vu c:\euler\msbm5.out
convert -vu c:\euler\msbm7.out
rem
copy cmbsy7.hex c:\euler
copy cmmib5.hex c:\euler
copy cmmib7.hex c:\euler
copy msam5.hex c:\euler
copy msam7.hex c:\euler
copy msbm5.hex c:\euler
copy msbm7.hex c:\euler
rem
:fontone
fontone -klqqqq -w=c:\afm\tex -h=c:\euler c:\euler\cmbsy5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\euler c:\euler\cmbsy7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\euler c:\euler\cmmib5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\euler c:\euler\cmmib7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\euler c:\euler\msam5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\euler c:\euler\msam7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\euler c:\euler\msbm5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\euler c:\euler\msbm7.hex
rem
pause
rem
pfatopfb cmbsy5
pfatopfb cmbsy7
pfatopfb cmmib5
pfatopfb cmmib7
pfatopfb msam5
pfatopfb msam7
pfatopfb msbm5
pfatopfb msbm7
rem
pause
rem
copy cmbsy5.pfb c:\psfonts
copy cmbsy7.pfb c:\psfonts
copy cmmib5.pfb c:\psfonts
copy cmmib7.pfb c:\psfonts
copy msam5.pfb c:\psfonts
copy msam7.pfb c:\psfonts
copy msbm5.pfb c:\psfonts
copy msbm7.pfb c:\psfonts
rem
del cmbsy5.*
del cmbsy7.*
del cmmib5.*
del cmmib7.*
del msam5.*
del msam7.*
del msbm5.*
del msbm7.*
rem
pause
afmtopfm -vsdt c:\afm\tex\cmbsy5.afm
afmtopfm -vsdt c:\afm\tex\cmbsy7.afm
afmtopfm -vsdt c:\afm\tex\cmmib5.afm
afmtopfm -vsdt c:\afm\tex\cmmib7.afm
afmtopfm -vsdt c:\afm\tex\msam5.afm
afmtopfm -vsdt c:\afm\tex\msam7.afm
afmtopfm -vsdt c:\afm\tex\msbm5.afm
afmtopfm -vsdt c:\afm\tex\msbm7.afm
pause
copy cmbsy5.pfm c:\psfonts\pfm
copy cmbsy7.pfm c:\psfonts\pfm
copy cmmib5.pfm c:\psfonts\pfm
copy cmmib7.pfm c:\psfonts\pfm
copy msam5.pfm c:\psfonts\pfm
copy msam7.pfm c:\psfonts\pfm
copy msbm5.pfm c:\psfonts\pfm
copy msbm7.pfm c:\psfonts\pfm
pause
del *.pfm
@echo on
