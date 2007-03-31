@echo off
rem
rem Copyright (C) 1991 Y&Y, Inc.
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.
rem
rem Usage: CHARMEDI PFA-file-name [number-of-copies]
rem
rem e.g.   charmedi tir 10			use two pages
rem
rem This assumes there is a c:\ps directory with the file charmedi.ps in it.
rem It uses the same directory for some temporary files.
rem
rem It also assumes there is a utility called `fontname' that extracts
rem the PostScript FontName from a PFA file.  This saves having to type
rem in the PS FontName - allows one to just use the fonts file name instead.
rem
rem Use PFBtoPFA to make PFA file if necessary
rem
rem May want to use REENCODE to make unencoded characters accessible
rem
rem The output produced is controlled to some extend by variables and flags
rem such as `usehex' `showmac' `shownumeric' `standardfont' in charmedi.ps
rem
rem This sends output to LPT1:  - adjust if printer is on another port
rem
if "%2" == "" goto onecopy
echo /#copies %2 def > c:\ps\copies.ps
echo making %2 copies
goto aftercopy
:onecopy
echo /#copies 1 def > c:\ps\copies.ps
:aftercopy
rem if "%2" == "" goto onearg
rem echo (%2) > c:\ps\params.ps
rem goto skip
rem :onearg
rem echo WARNING: Using %s as PostScript FontName
rem echo (%1) > c:\ps\params.ps
if exist %1.pfa goto pfaz
if exist %1.hex goto hexz
if exist %1.ps goto psz
echo SORRY: can't find file
goto end
:pfaz
fontname %1.pfa c:\ps\params.ps
goto skip
:psz
fontname %1.ps c:\ps\params.ps
goto skip
:hexz
fontname %1.hex c:\ps\params.ps
rem type c:\ps\params.ps
goto skip
rem type c:\ps\params.ps
:skip
rem show the PS FontName found
rem
type c:\ps\params.ps
if exist %1.pfa goto pfa
if exist %1.hex goto hex
rem if exist c:\psfonts\%1.pfb goto pfb
echo can't find %1
goto end
:pfa
echo using %1.pfa
copy %1.pfa+c:\ps\copies.ps+c:\ps\params.ps+c:\ps\charmedi.ps lpt1:/b
goto end
:hex
echo using %1.hex
copy %1.hex+c:\ps\copies.ps+c:\ps\params.ps+c:\ps\charmedi.ps lpt1:/b
goto end
:pfb
echo using c:\psfonts\%1.pfb
pfbtopfa c:\psfonts\%1.pfb
copy %1.pfa+c:\ps\copies.ps+c:\ps\params.ps+c:\ps\charmedi.ps lpt1:/b
del %1.pfa
goto end
:end
rem del c:\ps\params.ps
@echo on
