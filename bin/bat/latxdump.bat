@echo off
rem Copyright (C) 1991 Y&Y
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.
rem
rem echo Fits on DD diskette
echo Now requirees HD diskette
rem
rem Tries to also dump INF files (unless second argument given)
rem
if not "%1" == "" goto argok
echo Usage:   latxdump a:      or     latxdump b:
goto end
:argok
if not exist %1\*.* goto starmiss
dir %1
echo ERROR: Disk is not empty!
goto end
:starmiss
echo Setting Volume Label 
if "%1" == "a:" goto setlabel
if "%1" == "b:" goto setlabel
goto skiplabel
:setlabel
echo Setting Volume Label
label %1LATEXSLITEX 
rem
:skiplabel
echo copying c:\latxfont\copyrght.txt
copy c:\latxfont\copyrght.txt %1
echo copying c:\latxfont\readme.txt
copy c:\latxfont\readme.txt %1
rem
mkdir %1\afm
echo copying c:\afm\tex\lasy10.afm
copy c:\afm\tex\lasy10.afm %1\afm
echo copying c:\latxfont\lasy10.pfb 
copy c:\latxfont\lasy10.pfb %1
echo copying c:\latxfont\lasy10.pfm 
copy c:\latxfont\lasy10.pfm %1
echo copying c:\afm\tex\lasy5.afm
copy c:\afm\tex\lasy5.afm %1\afm
echo copying c:\latxfont\lasy5.pfb 
copy c:\latxfont\lasy5.pfb %1
echo copying c:\latxfont\lasy5.pfm 
copy c:\latxfont\lasy5.pfm %1
echo copying c:\afm\tex\lasy6.afm 
copy c:\afm\tex\lasy6.afm %1\afm
echo copying c:\latxfont\lasy6.pfb 
copy c:\latxfont\lasy6.pfb %1
echo copying c:\latxfont\lasy6.pfm 
copy c:\latxfont\lasy6.pfm %1
echo copying c:\afm\tex\lasy7.afm 
copy c:\afm\tex\lasy7.afm %1\afm
echo copying c:\latxfont\lasy7.pfb 
copy c:\latxfont\lasy7.pfb %1
echo copying c:\latxfont\lasy7.pfm 
copy c:\latxfont\lasy7.pfm %1
echo copying c:\afm\tex\lasy8.afm 
copy c:\afm\tex\lasy8.afm %1\afm
echo copying c:\latxfont\lasy8.pfb 
copy c:\latxfont\lasy8.pfb %1
echo copying c:\latxfont\lasy8.pfm 
copy c:\latxfont\lasy8.pfm %1
echo copying c:\afm\tex\lasy9.afm 
copy c:\afm\tex\lasy9.afm %1\afm
echo copying c:\latxfont\lasy9.pfb 
copy c:\latxfont\lasy9.pfb %1
echo copying c:\latxfont\lasy9.pfm 
copy c:\latxfont\lasy9.pfm %1
rem
echo copying c:\afm\tex\lasyb10.afm 
copy c:\afm\tex\lasyb10.afm %1\afm
echo copying c:\latxfont\lasyb10.pfb 
copy c:\latxfont\lasyb10.pfb %1
echo copying c:\latxfont\lasyb10.pfm 
copy c:\latxfont\lasyb10.pfm %1
rem 
echo copying c:\afm\tex\lcircle1.afm 
copy c:\afm\tex\lcircle1.afm %1\afm
echo copying c:\latxfont\lcircle1.pfb 
copy c:\latxfont\lcircle1.pfb %1
echo copying c:\latxfont\lcircle1.pfm 
copy c:\latxfont\lcircle1.pfm %1
echo copying c:\afm\tex\lcirclew.afm 
copy c:\afm\tex\lcirclew.afm %1\afm
echo copying c:\latxfont\lcirclew.pfb 
copy c:\latxfont\lcirclew.pfb %1
echo copying c:\latxfont\lcirclew.pfm 
copy c:\latxfont\lcirclew.pfm %1
echo copying c:\afm\tex\line10.afm 
copy c:\afm\tex\line10.afm %1\afm
echo copying c:\latxfont\line10.pfb 
copy c:\latxfont\line10.pfb %1
echo copying c:\latxfont\line10.pfm 
copy c:\latxfont\line10.pfm %1
echo copying c:\afm\tex\linew10.afm 
copy c:\afm\tex\linew10.afm %1\afm
echo copying c:\latxfont\linew10.pfb 
copy c:\latxfont\linew10.pfb %1
echo copying c:\latxfont\linew10.pfm 
copy c:\latxfont\linew10.pfm %1
rem
echo copying c:\afm\tex\lcmss8.afm
copy c:\afm\tex\lcmss8.afm %1\afm
echo copying c:\latxfont\lcmss8.pfb
copy c:\latxfont\lcmss8.pfb %1
echo copying c:\latxfont\lcmss8.pfm
copy c:\latxfont\lcmss8.pfm %1
echo copying c:\afm\tex\lcmssb8.afm
copy c:\afm\tex\lcmssb8.afm %1\afm
echo copying c:\latxfont\lcmssb8.pfb
copy c:\latxfont\lcmssb8.pfb %1
echo copying c:\latxfont\lcmssb8.pfm
copy c:\latxfont\lcmssb8.pfm %1
echo copying c:\afm\tex\lcmssi8.afm
copy c:\afm\tex\lcmssi8.afm %1\afm
echo copying c:\latxfont\lcmssi8.pfb
copy c:\latxfont\lcmssi8.pfb %1
echo copying c:\latxfont\lcmssi8.pfm
copy c:\latxfont\lcmssi8.pfm %1
rem
echo copying c:\afm\tex\ilcmss8.afm
copy c:\afm\tex\ilcmss8.afm %1\afm
echo copying c:\latxfont\ilcmss8.pfb
copy c:\latxfont\ilcmss8.pfb %1
echo copying c:\latxfont\ilcmss8.pfm
copy c:\latxfont\ilcmss8.pfm %1
echo copying c:\afm\tex\ilcmssb8.afm
copy c:\afm\tex\ilcmssb8.afm %1\afm
echo copying c:\latxfont\ilcmssb8.pfb
copy c:\latxfont\ilcmssb8.pfb %1
echo copying c:\latxfont\ilcmssb8.pfm
copy c:\latxfont\ilcmssb8.pfm %1
echo copying c:\afm\tex\ilcmssi8.afm
copy c:\afm\tex\ilcmssi8.afm %1\afm
echo copying c:\latxfont\ilcmssi8.pfb
copy c:\latxfont\ilcmssi8.pfb %1
echo copying c:\latxfont\ilcmssi8.pfm
copy c:\latxfont\ilcmssi8.pfm %1
rem
echo copying c:\afm\tex\icmex10.afm
copy c:\afm\tex\icmex10.afm %1\afm
echo copying c:\latxfont\icmex10.pfb
copy c:\latxfont\icmex10.pfb %1
echo copying c:\latxfont\icmex10.pfm
copy c:\latxfont\icmex10.pfm %1
echo copying c:\afm\tex\icmsy8.afm
copy c:\afm\tex\icmsy8.afm %1\afm
echo copying c:\latxfont\icmsy8.pfb
copy c:\latxfont\icmsy8.pfb %1
echo copying c:\latxfont\icmsy8.pfm
copy c:\latxfont\icmsy8.pfm %1
echo copying c:\afm\tex\icmmi8.afm
copy c:\afm\tex\icmmi8.afm %1\afm
echo copying c:\latxfont\icmmi8.pfb
copy c:\latxfont\icmmi8.pfb %1
echo copying c:\latxfont\icmmi8.pfm
copy c:\latxfont\icmmi8.pfm %1
echo copying c:\afm\tex\icmtt8.afm
copy c:\afm\tex\icmtt8.afm %1\afm
echo copying c:\latxfont\icmtt8.pfb
copy c:\latxfont\icmtt8.pfb %1
echo copying c:\latxfont\icmtt8.pfm
copy c:\latxfont\icmtt8.pfm %1
echo copying c:\afm\tex\ilasy8.afm
copy c:\afm\tex\ilasy8.afm %1\afm
echo copying c:\latxfont\ilasy8.pfb
copy c:\latxfont\ilasy8.pfb %1
echo copying c:\latxfont\ilasy8.pfm
copy c:\latxfont\ilasy8.pfm %1
rem
echo copying c:\afm\tex\logo10.afm 
copy c:\afm\tex\logo10.afm %1\afm
echo copying c:\latxfont\logo10.pfb 
copy c:\latxfont\logo10.pfb %1
echo copying c:\latxfont\logo10.pfm 
copy c:\latxfont\logo10.pfm %1
echo copying c:\afm\tex\logo8.afm 
copy c:\afm\tex\logo8.afm %1\afm
echo copying c:\latxfont\logo8.pfb 
copy c:\latxfont\logo8.pfb %1
echo copying c:\latxfont\logo8.pfm 
copy c:\latxfont\logo8.pfm %1
echo copying c:\afm\tex\logo9.afm 
copy c:\afm\tex\logo9.afm %1\afm
echo copying c:\latxfont\logo9.pfb 
copy c:\latxfont\logo9.pfb %1
echo copying c:\latxfont\logo9.pfm 
copy c:\latxfont\logo9.pfm %1
echo copying c:\afm\tex\logobf10.afm 
copy c:\afm\tex\logobf10.afm %1\afm
echo copying c:\latxfont\logobf10.pfb 
copy c:\latxfont\logobf10.pfb %1
echo copying c:\latxfont\logobf10.pfm 
copy c:\latxfont\logobf10.pfm %1
echo copying c:\afm\tex\logosl10.afm 
copy c:\afm\tex\logosl10.afm %1\afm
echo copying c:\latxfont\logosl10.pfb 
copy c:\latxfont\logosl10.pfb %1
echo copying c:\latxfont\logosl10.pfm 
copy c:\latxfont\logosl10.pfm %1
rem
rem echo copying c:\latxfont\logoft10.pfb 
rem copy c:\latxfont\logoft10.pfb %1
rem echo copying c:\latxfont\logosk30.pfb 
rem copy c:\latxfont\logosk30.pfb %1
rem
echo Make Program Directory
mkdir b:\prog
rem
echo copying c:\prog\pfbtopfa.exe 
copy c:\prog\pfbtopfa.exe %1\prog
echo copying c:\prog\pfatopfb.exe 
copy c:\prog\pfatopfb.exe %1\prog
echo copying c:\prog\reencode.exe
copy c:\prog\reencode.exe %1\prog
echo copying c:\prog\namecase.exe
copy c:\prog\namecase.exe %1\prog
echo copying c:\prog\download.exe
copy c:\prog\download.exe %1\prog
echo copying c:\prog\serial.exe
copy c:\prog\serial.exe %1\prog
echo copying c:\prog\modex.com
copy c:\prog\modex.com %1\prog
rem
echo copying c:\dvitest\dragon.dvi
copy c:\dvitest\dragon.dvi %1
rem
echo copying c:\tex\dragon.tex
copy c:\tex\dragon.tex %1
rem
rem following is optional... omit of no space left
echo copying c:\txt\spacify.txt
copy c:\txt\spacify.txt %1
rem
echo copying c:\prog\spacify.exe
copy c:\prog\spacify.exe %1\prog
rem
mkdir %1\tfm
echo Copying TFM files next
rem
echo copying c:\pctex\textfms\lasy10.tfm 
copy c:\pctex\textfms\lasy10.tfm %1\tfm
echo copying c:\pctex\textfms\lasy5.tfm 
copy c:\pctex\textfms\lasy5.tfm %1\tfm
rem copy c:\latxfont\lasy5.tfm %1\tfm
echo copying c:\pctex\textfms\lasy6.tfm 
copy c:\pctex\textfms\lasy6.tfm %1\tfm
rem copy c:\latxfont\lasy6.tfm %1\tfm
echo copying c:\pctex\textfms\lasy7.tfm 
copy c:\pctex\textfms\lasy7.tfm %1\tfm
rem copy c:\latxfont\lasy7.tfm %1\tfm
echo copying c:\pctex\textfms\lasy8.tfm 
copy c:\pctex\textfms\lasy8.tfm %1\tfm
echo copying c:\pctex\textfms\lasy9.tfm 
copy c:\pctex\textfms\lasy9.tfm %1\tfm
echo copying c:\pctex\textfms\lasyb10.tfm 
copy c:\pctex\textfms\lasyb10.tfm %1\tfm
echo copying c:\pctex\textfms\lcircle1.tfm 
copy c:\pctex\textfms\lcircle1.tfm %1\tfm
echo copying c:\pctex\textfms\lcirclew.tfm 
copy c:\pctex\textfms\lcirclew.tfm %1\tfm
echo copying c:\pctex\textfms\line10.tfm 
copy c:\pctex\textfms\line10.tfm %1\tfm
echo copying c:\pctex\textfms\linew10.tfm 
copy c:\pctex\textfms\linew10.tfm %1\tfm
echo copying c:\pctex\textfms\lcmss8.tfm 
copy c:\pctex\textfms\lcmss8.tfm %1\tfm
echo copying c:\pctex\textfms\lcmssb8.tfm 
copy c:\pctex\textfms\lcmssb8.tfm %1\tfm
echo copying c:\pctex\textfms\lcmssi8.tfm 
copy c:\pctex\textfms\lcmssi8.tfm %1\tfm
echo copying c:\pctex\textfms\ilcmss8.tfm 
copy c:\pctex\textfms\ilcmss8.tfm %1\tfm
echo copying c:\pctex\textfms\ilcmssb8.tfm 
copy c:\pctex\textfms\ilcmssb8.tfm %1\tfm
echo copying c:\pctex\textfms\ilcmssi8.tfm 
copy c:\pctex\textfms\ilcmssi8.tfm %1\tfm
echo copying c:\pctex\textfms\icmex10.tfm 
copy c:\pctex\textfms\icmex10.tfm %1\tfm
echo copying c:\pctex\textfms\icmsy8.tfm 
copy c:\pctex\textfms\icmsy8.tfm %1\tfm
echo copying c:\pctex\textfms\icmmi8.tfm 
copy c:\pctex\textfms\icmmi8.tfm %1\tfm
echo copying c:\pctex\textfms\icmtt8.tfm 
copy c:\pctex\textfms\icmtt8.tfm %1\tfm
echo copying c:\pctex\textfms\ilasy8.tfm 
copy c:\pctex\textfms\ilasy8.tfm %1\tfm
echo copying c:\pctex\textfms\logo10.tfm 
copy c:\pctex\textfms\logo10.tfm %1\tfm
echo copying c:\pctex\textfms\logo8.tfm 
copy c:\pctex\textfms\logo8.tfm %1\tfm
echo copying c:\pctex\textfms\logo9.tfm 
copy c:\pctex\textfms\logo9.tfm %1\tfm
echo copying c:\pctex\textfms\logobf10.tfm 
copy c:\pctex\textfms\logobf10.tfm %1\tfm
echo copying c:\pctex\textfms\logosl10.tfm 
copy c:\pctex\textfms\logosl10.tfm %1\tfm
rem
echo copying c:\latxfont\psfonts.lt
copy c:\latxfont\psfonts.lt %1
echo copying c:\latxfont\psfonts.ltx
copy c:\latxfont\psfonts.ltx %1
echo copying c:\latxfont\psfonts.ltz
copy c:\latxfont\psfonts.ltz %1
rem
echo copying c:\tex\logotest.tex
copy c:\tex\logotest.tex %1
echo copying c:\dvitest\logotest.dvi
copy c:\dvitest\logotest.dvi %1
rem
if not "%2" == "" goto skipinf
rem
echo following is optional
rem
echo Copying INF files
rem
xcopy c:\latxfont\*.inf %1\afm
rem
:skipinf
mkdir %1\ps
echo copying c:\ps\ehandler.ps
copy c:\ps\ehandler.ps %1\ps
rem
echo Copying printer test files
copy c:\dvipsone\ps\*.txt %1\ps
copy c:\dvipsone\ps\*.ps %1\ps
:end
@echo on
