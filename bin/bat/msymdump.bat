@echo off
rem Copyright (C) 1991 Berthold K.P. Horn Y&Y
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%1" == "" goto argok
echo Usage:   dumpmsym a:      or     dumpmsym b:
goto end
:argok
echo copying c:\latxfont\copyrght.txt
copy c:\latxfont\copyrght.txt %1
echo copying c:\latxfont\msym10.afm 
copy c:\latxfont\msym10.afm %1
echo copying c:\latxfont\msym10.pfb 
copy c:\latxfont\msym10.pfb %1
echo copying c:\latxfont\msym10.pfm 
copy c:\latxfont\msym10.pfm %1
echo copying c:\latxfont\msym10.tfm 
copy c:\latxfont\msym10.tfm %1
echo copying c:\latxfont\msym10.txt 
copy c:\latxfont\msym10.txt %1\readme.txt
:end
@echo on
