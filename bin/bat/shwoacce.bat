@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo remember to reencode font to ansiext
rem
rem acceshow file-name PostScript-FontName
rem
if "%2" == "" goto missing
goto second
:missing
rem echo need second parameter
if exist %1.pfa fontname %1.pfa c:\ps\params.ps
if exist %1.ps fontname %1.ps c:\ps\params.ps
if exist %1.hex fontname %1.hex c:\ps\params.ps
type c:\ps\params.ps
goto skip
:second
echo (%2) > c:\ps\params.ps
:skip
if exist %1.pfa goto pfa
if exist %1.ps goto ps
if exist %1.hex goto hex
echo can't find file
goto end
:hex
copy %1.hex+c:\ps\params.ps+c:\lucidabr\accents.ps lpt1:/b
goto end
:ps 
copy %1.ps+c:\ps\params.ps+c:\lucidabr\accents.ps lpt1:/b
goto end
:pfa
copy %1.pfa+c:\ps\params.ps+c:\lucidabr\accents.ps lpt1:/b
goto end
:end
rem del c:\ps\params.ps
@echo on
