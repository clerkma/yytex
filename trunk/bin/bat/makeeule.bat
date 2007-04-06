@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem
if "%1" == "F" goto fontone
if "%1" == "M" goto metrics
if "%1" == "C" goto convert
goto fontone
rem
rem eulerint c:\amsfonts\eurmch 10
rem eulerint c:\amsfonts\eurmch 7
rem eulerint c:\amsfonts\eurmch 5
rem eulerint c:\amsfonts\eufmch 10
rem eulerint c:\amsfonts\eufmch 7
rem eulerint c:\amsfonts\eufmch 5
rem eulerint c:\amsfonts\eusmch 10
rem eulerint c:\amsfonts\eusmch 7
rem eulerint c:\amsfonts\eusmch 5
rem eulerint c:\amsfonts\eurbch 10
rem eulerint c:\amsfonts\eurbch 7
rem eulerint c:\amsfonts\eurbch 5
rem eulerint c:\amsfonts\eufbch 10
rem eulerint c:\amsfonts\eufbch 7
rem eulerint c:\amsfonts\eufbch 5
rem eulerint c:\amsfonts\eusbch 10
rem eulerint c:\amsfonts\eusbch 7
rem eulerint c:\amsfonts\eusbch 5
rem
rem rewind eurm10.out
rem rewind eurm7.out
rem rewind eurm5.out
rem rewind eufm10.out
rem rewind eufm7.out
rem rewind eufm5.out
rem rewind eusm10.out
rem rewind eusm7.out
rem rewind eusm5.out
rem rewind eurb10.out
rem rewind eurb7.out
rem rewind eurb5.out
rem rewind eufb10.out
rem rewind eufb7.out
rem rewind eufb5.out
rem rewind eusb10.out
rem rewind eusb7.out
rem rewind eusb5.out
rem
rem resplice eurm10.unw
rem resplice eurm7.unw
rem resplice eurm5.unw
rem resplice eufm10.unw
rem resplice eufm7.unw
rem resplice eufm5.unw
rem resplice eusm10.unw
rem resplice eusm7.unw
rem resplice eusm5.unw
rem resplice eurb10.unw
rem resplice eurb7.unw
rem resplice eurb5.unw
rem resplice eufb10.unw
rem resplice eufb7.unw
rem resplice eufb5.unw
rem resplice eusb10.unw
rem resplice eusb7.unw
rem resplice eusb5.unw
rem
rem unionize eurm10.res
rem unionize eurm7.res
rem unionize eurm5.res
rem unionize eufm10.res
rem unionize eufm7.res
rem unionize eufm5.res
rem unionize eusm10.res
rem unionize eusm7.res
rem unionize eusm5.res
rem unionize eurb10.res
rem unionize eurb7.res
rem unionize eurb5.res
rem unionize eufb10.res
rem unionize eufb7.res
rem unionize eufb5.res
rem unionize eusb10.res
rem unionize eusb7.res
rem unionize eusb5.res
rem
:convert
convert -vu c:\amsfonts\eurm10.out
convert -vu c:\amsfonts\eurm7.out
convert -vu c:\amsfonts\eurm5.out
convert -vu c:\amsfonts\eufm10.out
convert -vu c:\amsfonts\eufm7.out
convert -vu c:\amsfonts\eufm5.out
convert -vu c:\amsfonts\eusm10.out
convert -vu c:\amsfonts\eusm7.out
convert -vu c:\amsfonts\eusm5.out
rem
convert -vu c:\amsfonts\eurb10.out
convert -vu c:\amsfonts\eurb7.out
convert -vu c:\amsfonts\eurb5.out
convert -vu c:\amsfonts\eufb10.out
convert -vu c:\amsfonts\eufb7.out
convert -vu c:\amsfonts\eufb5.out
convert -vu c:\amsfonts\eusb10.out
convert -vu c:\amsfonts\eusb7.out
convert -vu c:\amsfonts\eusb5.out
rem 
convert -vu c:\amsfonts\euex10.out
convert -vu c:\amsfonts\msam10.out
convert -vu c:\amsfonts\msbm10.out
rem
convert -vu c:\amsfonts\wncyr10.out
convert -vu c:\amsfonts\wncyb10.out
convert -vu c:\amsfonts\wncyi10.out
convert -vu c:\amsfonts\wncysc10.out
convert -vu c:\amsfonts\wncyss10.out
rem
convert -vu c:\amsfonts\cmbsy5.out
convert -vu c:\amsfonts\cmbsy7.out
convert -vu c:\amsfonts\cmmib5.out
convert -vu c:\amsfonts\cmmib7.out
rem
convert -vu c:\amsfonts\msam5.out
convert -vu c:\amsfonts\msam7.out
convert -vu c:\amsfonts\msbm5.out
convert -vu c:\amsfonts\msbm7.out
rem
pause Writing Hex files back to hard disk
rem
copy eurm10.hex c:\amsfonts
copy eurm7.hex c:\amsfonts
copy eurm5.hex c:\amsfonts
copy eufm10.hex c:\amsfonts
copy eufm7.hex c:\amsfonts
copy eufm5.hex c:\amsfonts
copy eusm10.hex c:\amsfonts
copy eusm7.hex c:\amsfonts
copy eusm5.hex c:\amsfonts
rem
copy eurb10.hex c:\amsfonts
copy eurb7.hex c:\amsfonts
copy eurb5.hex c:\amsfonts
copy eufb10.hex c:\amsfonts
copy eufb7.hex c:\amsfonts
copy eufb5.hex c:\amsfonts
copy eusb10.hex c:\amsfonts
copy eusb7.hex c:\amsfonts
copy eusb5.hex c:\amsfonts
rem
copy euex10.hex c:\amsfonts
copy msam10.hex c:\amsfonts
copy msbm10.hex c:\amsfonts
rem
copy wncyr10.hex c:\amsfonts
copy wncyb10.hex c:\amsfonts
copy wncyi10.hex c:\amsfonts
copy wncysc10.hex c:\amsfonts
copy wncyss10.hex c:\amsfonts
rem
copy cmbsy5.hex c:\amsfonts
copy cmbsy7.hex c:\amsfonts
copy cmmib5.hex c:\amsfonts
copy cmmib7.hex c:\amsfonts
rem
copy msam5.hex c:\amsfonts
copy msam7.hex c:\amsfonts
copy msbm5.hex c:\amsfonts
copy msbm7.hex c:\amsfonts
rem
:fontone
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eurm10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eurm7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eurm5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eufm10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eufm7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eufm5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eusm10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eusm7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eusm5.hex
rem
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eurb10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eurb7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eurb5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eufb10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eufb7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eufb5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eusb10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eusb7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\eusb5.hex
rem
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\euex10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\msam10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\msbm10.hex
rem
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\wncyr10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\wncyb10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\wncyi10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\wncysc10.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\wncyss10.hex
rem
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\cmbsy5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\cmbsy7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\cmmib5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\cmmib7.hex
rem
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\msam5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\msam7.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\msbm5.hex
fontone -klqqqq -w=c:\afm\tex -h=c:\amsfonts c:\amsfonts\msbm7.hex
rem
pause Converting PFA to PFB format
rem
pfatopfb eurm10.pfa
pfatopfb eurm7.pfa
pfatopfb eurm5.pfa
pfatopfb eufm10.pfa
pfatopfb eufm7.pfa
pfatopfb eufm5.pfa
pfatopfb eusm10.pfa
pfatopfb eusm7.pfa
pfatopfb eusm5.pfa
rem
pfatopfb eurb10.pfa
pfatopfb eurb7.pfa
pfatopfb eurb5.pfa
pfatopfb eufb10.pfa
pfatopfb eufb7.pfa
pfatopfb eufb5.pfa
pfatopfb eusb10.pfa
pfatopfb eusb7.pfa
pfatopfb eusb5.pfa
rem
pfatopfb euex10.pfa
pfatopfb msam10.pfa
pfatopfb msbm10.pfa
rem
pfatopfb wncyr10.pfa
pfatopfb wncyb10.pfa
pfatopfb wncyi10.pfa
pfatopfb wncysc10.pfa
pfatopfb wncyss10.pfa
rem
pfatopfb cmbsy5
pfatopfb cmbsy7
pfatopfb cmmib5
pfatopfb cmmib7
rem
pfatopfb msam5
pfatopfb msam7
pfatopfb msbm5
pfatopfb msbm7
rem
pause Copying PFB files to hard disk
rem
copy eurm10.pfb c:\psfonts
copy eurm7.pfb c:\psfonts
copy eurm5.pfb c:\psfonts
copy eufm10.pfb c:\psfonts
copy eufm7.pfb c:\psfonts
copy eufm5.pfb c:\psfonts
copy eusm10.pfb c:\psfonts
copy eusm7.pfb c:\psfonts
copy eusm5.pfb c:\psfonts
rem
copy eurb10.pfb c:\psfonts
copy eurb7.pfb c:\psfonts
copy eurb5.pfb c:\psfonts
copy eufb10.pfb c:\psfonts
copy eufb7.pfb c:\psfonts
copy eufb5.pfb c:\psfonts
copy eusb10.pfb c:\psfonts
copy eusb7.pfb c:\psfonts
copy eusb5.pfb c:\psfonts
rem
copy euex10.pfb c:\psfonts
copy msam10.pfb c:\psfonts
copy msbm10.pfb c:\psfonts
rem
copy wncyr10.pfb c:\psfonts
copy wncyb10.pfb c:\psfonts
copy wncyi10.pfb c:\psfonts
copy wncysc10.pfb c:\psfonts
copy wncyss10.pfb c:\psfonts
rem
copy cmbsy5.pfb c:\psfonts
copy cmbsy7.pfb c:\psfonts
copy cmmib5.pfb c:\psfonts
copy cmmib7.pfb c:\psfonts
rem
copy msam5.pfb c:\psfonts
copy msam7.pfb c:\psfonts
copy msbm5.pfb c:\psfonts
copy msbm7.pfb c:\psfonts
rem
pause Deleting outline fonts
rem
del eurm10.*
del eurm7.*
del eurm5.*
del eufm10.*
del eufm7.*
del eufm5.*
del eusm10.*
del eusm7.*
del eusm5.*
rem
del eurb10.*
del eurb7.*
del eurb5.*
del eufb10.*
del eufb7.*
del eufb5.*
del eusb10.*
del eusb7.*
del eusb5.*
rem
del euex10.*
del msam10.*
del msbm10.*
rem
del wncyr10.*
del wncyb10.*
del wncyi10.*
del wncysc10.*
del wncyss10.*
rem
del cmbsy5.*
del cmbsy7.*
del cmmib5.*
del cmmib7.*
rem
del msam5.*
del msam7.*
del msbm5.*
del msbm7.*
rem
pause Generating metric files
:metrics
rem
afmtopfm -vsdt -f=period c:\afm\tex\eufm10.afm
afmtopfm -vsdt -f=period c:\afm\tex\eufm7.afm
afmtopfm -vsdt -f=period c:\afm\tex\eufm5.afm
afmtopfm -vsdt -f=period c:\afm\tex\eurm10.afm
afmtopfm -vsdt -f=period c:\afm\tex\eurm7.afm
afmtopfm -vsdt -f=period c:\afm\tex\eurm5.afm
rem afmtopfm -vsdt -f=period c:\afm\tex\eusm10.afm
afmtopfm -vsdt c:\afm\tex\eusm10.afm
rem afmtopfm -vsdt -f=period c:\afm\tex\eusm7.afm
afmtopfm -vsdt c:\afm\tex\eusm7.afm
rem afmtopfm -vsdt -f=period c:\afm\tex\eusm5.afm
afmtopfm -vsdt c:\afm\tex\eusm5.afm
rem
afmtopfm -vsdt -f=period c:\afm\tex\eufb10.afm
afmtopfm -vsdt -f=period c:\afm\tex\eufb7.afm
afmtopfm -vsdt -f=period c:\afm\tex\eufb5.afm
afmtopfm -vsdt -f=period c:\afm\tex\eurb10.afm
afmtopfm -vsdt -f=period c:\afm\tex\eurb7.afm
afmtopfm -vsdt -f=period c:\afm\tex\eurb5.afm
rem afmtopfm -vsdt -f=period c:\afm\tex\eusb10.afm
afmtopfm -vsdt c:\afm\tex\eusb10.afm
rem afmtopfm -vsdt -f=period c:\afm\tex\eusb7.afm
afmtopfm -vsdt c:\afm\tex\eusb7.afm
rem afmtopfm -vsdt -f=period c:\afm\tex\eusb5.afm
afmtopfm -vsdt c:\afm\tex\eusb5.afm
rem
rem afmtopfm -vsdt -f=period c:\afm\tex\euex10.afm
afmtopfm -vsdt c:\afm\tex\euex10.afm
rem afmtopfm -vsdt -f=period c:\afm\tex\msam10.afm
afmtopfm -vsdt c:\afm\tex\msam10.afm
rem afmtopfm -vsdt -f=period c:\afm\tex\msbm10.afm
afmtopfm -vsdt c:\afm\tex\msbm10.afm
Xrem
afmtopfm -vsdt c:\afm\tex\wncyr10.afm
afmtopfm -vsdt c:\afm\tex\wncyb10.afm
afmtopfm -vsdt c:\afm\tex\wncyi10.afm
afmtopfm -vsdt c:\afm\tex\wncysc10.afm
afmtopfm -vsdt c:\afm\tex\wncyss10.afm
rem
afmtopfm -vsdt c:\afm\tex\cmbsy5.afm
afmtopfm -vsdt c:\afm\tex\cmbsy7.afm
afmtopfm -vsdt c:\afm\tex\cmmib5.afm
afmtopfm -vsdt c:\afm\tex\cmmib7.afm
rem
afmtopfm -vsdt c:\afm\tex\msam5.afm
afmtopfm -vsdt c:\afm\tex\msam7.afm
afmtopfm -vsdt c:\afm\tex\msbm5.afm
afmtopfm -vsdt c:\afm\tex\msbm7.afm
rem
pause Copying metric files to hard disk
rem
copy eurm10.pfm c:\psfonts\pfm
copy eurm7.pfm c:\psfonts\pfm
copy eurm5.pfm c:\psfonts\pfm
copy eufm10.pfm c:\psfonts\pfm
copy eufm7.pfm c:\psfonts\pfm
copy eufm5.pfm c:\psfonts\pfm
copy eusm10.pfm c:\psfonts\pfm
copy eusm7.pfm c:\psfonts\pfm
copy eusm5.pfm c:\psfonts\pfm
rem
copy eurb10.pfm c:\psfonts\pfm
copy eurb7.pfm c:\psfonts\pfm
copy eurb5.pfm c:\psfonts\pfm
copy eufb10.pfm c:\psfonts\pfm
copy eufb7.pfm c:\psfonts\pfm
copy eufb5.pfm c:\psfonts\pfm
copy eusb10.pfm c:\psfonts\pfm
copy eusb7.pfm c:\psfonts\pfm
copy eusb5.pfm c:\psfonts\pfm
rem
copy euex10.pfm c:\psfonts\pfm
copy msam10.pfm c:\psfonts\pfm
copy msbm10.pfm c:\psfonts\pfm
rem
copy wncyr10.pfm c:\psfonts\pfm
copy wncyb10.pfm c:\psfonts\pfm
copy wncyi10.pfm c:\psfonts\pfm
copy wncysc10.pfm c:\psfonts\pfm
copy wncyss10.pfm c:\psfonts\pfm
rem
copy cmbsy5.pfm c:\psfonts\pfm
copy cmbsy7.pfm c:\psfonts\pfm
copy cmmib5.pfm c:\psfonts\pfm
copy cmmib7.pfm c:\psfonts\pfm
rem
copy msam5.pfm c:\psfonts\pfm
copy msam7.pfm c:\psfonts\pfm
copy msbm5.pfm c:\psfonts\pfm
copy msbm7.pfm c:\psfonts\pfm
rem
pause Copying metric files to RAM disk
rem
copy eu*.pfm d:\pfm
copy ms*.pfm d:\pfm
copy wn*.pfm d:\pfm
copy cm*.pfm d:\pfm
rem
pause Deleting metric files
del *.pfm
@echo on
