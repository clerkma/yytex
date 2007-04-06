@echo off
rem Copyright (C) 1991,1992,1993,1994 Berthold K.P. Horn Y&Y, Inc.
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%1" == "" goto argok
echo Usage:   windumpa a:      or     windumpa b:
goto end

:argok
if not exist %1\*.* goto starmiss
dir %1
echo ERROR: Disk is not empty!
pause
goto end

:starmiss
if "%1" == "a:" goto setlabel
if "%1" == "A:" goto setlabel
if "%1" == "b:" goto setlabel
if "%1" == "B:" goto setlabel
goto skiplabel

:setlabel
echo Setting Volume Label (with version number)
label %1DVIWINDO108

:skiplabel
echo making directories
if not exist %1\prog\nul mkdir %1\prog
if not exist %1\txt\nul mkdir %1\txt
if not exist %1\tex\nul mkdir %1\tex
if not exist %1\fnt\nul mkdir %1\fnt
if not exist %1\pfb\nul mkdir %1\pfb
if not exist %1\pfm\nul mkdir %1\pfm
if not exist %1\tfm\nul mkdir %1\tfm
if not exist %1\vec\nul mkdir %1\vec

echo copying c:\windev\samples\dviwindo\copyrght.txt
copy c:\windev\samples\dviwindo\copyrght.txt %1

if not exist c:\windev\samples\dviwindo\readme.txt goto noread
echo copying c:\windev\samples\dviwindo\readme.txt
copy c:\windev\samples\dviwindo\readme.txt %1

:noread
if not exist c:\windev\samples\dviwindo\news.txt goto nonews
echo copying c:\windev\samples\dviwindo\news.txt
copy c:\windev\samples\dviwindo\news.txt %1

:nonews
echo copying c:\txt\encoding.txt
copy c:\txt\encoding.txt %1\txt
echo copying c:\txt\morass.txt
copy c:\txt\morass.txt %1\txt
echo Copying c:\txt\psfonts.txt
copy c:\txt\psfonts.txt %1\txt
echo copying c:\windev\samples\dviwindo\printing.txt
copy c:\windev\samples\dviwindo\printing.txt %1\txt
echo copying c:\txt\psfigfix.txt
copy c:\txt\psfigfix.txt %1\txt
echo copying c:\txt\tpicspec.txt
copy c:\txt\tpicspec.txt %1\txt

echo copying c:\tex\epsfsafe.tex 
copy c:\tex\epsfsafe.tex %1\tex
echo copying c:\tex\tpictest.tex
copy c:\tex\tpictest.tex %1\tex

echo 
if not exist %1\prog\nul mkdir %1\prog

echo copying c:\windev\samples\setup\install.exe
copy c:\windev\samples\setup\install.exe %1

echo copying c:\windev\samples\dviwindo\dvisetup.inf
copy c:\windev\samples\dviwindo\dvisetup.inf %1

rem echo copying c:\windev\samples\dviwindo\dviwindo.exe
rem copy c:\windev\samples\dviwindo\dviwindo.exe %1\prog

echo Compressing DVIWindo.EXE
rem c:\windev\bin\compress -r c:\windev\samples\dviwindo\dviwindo.exe .
c:\windev\bin\compress -r c:\windev\samples\dviwindo\dviwindo.exe %1\prog
if errorlevel = 1 pause

echo change back to top-level
cd %1\

rem Use this old version (2.F) of Black Ice, since it has fewer bugs 
rem and is much smaller than the current version.

rem echo copying c:\windev\samples\dviwindo\tiffread.dll
rem copy c:\windev\samples\dviwindo\tiffread.dll %1\prog

echo Compressing TIFFRead.DLL
rem c:\windev\bin\compress -r c:\windev\samples\dviwindo\tiffread.dll .
c:\windev\bin\compress -r c:\windev\samples\dviwindo\tiffread.dll %1\prog
if errorlevel = 1 pause

echo change back to top-level
cd %1\

echo copying c:\windows\system\lzexpand.dll
copy c:\windows\system\lzexpand.dll %1

echo copying c:\prog\reencode.exe
copy c:\prog\reencode.exe %1\prog
echo copying c:\prog\afmtotfm.exe
copy c:\prog\afmtotfm.exe %1\prog
echo copying c:\prog\afmtopfm.exe
copy c:\prog\afmtopfm.exe %1\prog
echo copying c:\prog\tifftags.exe
copy c:\prog\tifftags.exe %1\prog

rem echo copying c:\windev\samples\cleanup\cleanup.exe
rem copy c:\windev\samples\cleanup\cleanup.exe %1\prog

echo Compressing CleanUp.EXE
rem c:\windev\bin\compress -r c:\windev\samples\cleanup\cleanup.exe .
c:\windev\bin\compress -r c:\windev\samples\cleanup\cleanup.exe %1\prog
if errorlevel = 1 pause

echo change back to top-level
cd %1\

echo copying c:\windev\samples\dviwindo\dviwindo.fn*
copy c:\windev\samples\dviwindo\dviwindo.fn* %1\fnt

echo 
echo making VEC subdirectory
if not exist %1\vec\nul mkdir %1\vec

echo Copying c:\dvipsone\standard.vec
copy c:\dvipsone\standard.vec %1\vec
echo Copying c:\dvipsone\symbol.vec
copy c:\dvipsone\symbol.vec %1\vec
echo Copying c:\dvipsone\ansi.vec
copy c:\dvipsone\ansi.vec %1\vec
echo Copying c:\dvipsone\ansinew.vec
copy c:\dvipsone\ansinew.vec %1\vec
echo Copying c:\dvipsone\ibmoem.vec
copy c:\dvipsone\ibmoem.vec %1\vec
echo Copying c:\dvipsone\dingbats.vec
copy c:\dvipsone\dingbats.vec %1\vec
echo Copying c:\dvipsone\fontogra.vec
copy c:\dvipsone\fontogra.vec %1\vec
echo Copying c:\dvipsone\ventura.vec
copy c:\dvipsone\ventura.vec %1\vec

echo Copying c:\dvipsone\isolati1.vec
copy c:\dvipsone\isolati1.vec %1\vec
echo Copying c:\dvipsone\isolati2.vec
copy c:\dvipsone\isolati2.vec %1\vec
echo Copying c:\dvipsone\mac.vec
copy c:\dvipsone\mac.vec %1\vec
echo copying c:\dvipsone\cyrillic.vec
copy c:\dvipsone\cyrillic.vec %1\vec

echo copying c:\dvipsone\textures.vec 
copy c:\dvipsone\textures.vec %1\vec
echo copying c:\dvipsone\textutyp.vec 
copy c:\dvipsone\textutyp.vec %1\vec
echo copying c:\dvipsone\texmac.vec 
copy c:\dvipsone\texmac.vec %1\vec

echo Copying c:\dvipsone\tex256.vec
copy c:\dvipsone\tex256.vec %1\vec
echo Copying c:\dvipsone\texascii.vec
copy c:\dvipsone\texascii.vec %1\vec
echo Copying c:\dvipsone\texital.vec
copy c:\dvipsone\texital.vec %1\vec
echo Copying c:\dvipsone\textext.vec
copy c:\dvipsone\textext.vec %1\vec
echo Copying c:\dvipsone\textype.vec
copy c:\dvipsone\textype.vec %1\vec
echo Copying c:\dvipsone\typeital.vec
copy c:\dvipsone\typeital.vec %1\vec
echo Copying c:\dvipsone\typewrit.vec
copy c:\dvipsone\typewrit.vec %1\vec

echo Copying c:\dvipsone\texansi.vec
copy c:\dvipsone\texansi.vec %1\vec
echo Copying c:\dvipsone\texannew.vec
copy c:\dvipsone\texannew.vec %1\vec

echo Copying c:\dvipsone\mathit.vec
copy c:\dvipsone\mathit.vec %1\vec
echo Copying c:\dvipsone\mathsy.vec
copy c:\dvipsone\mathsy.vec %1\vec
echo Copying c:\dvipsone\mathex.vec
copy c:\dvipsone\mathex.vec %1\vec
echo Copying c:\dvipsone\numeric.vec
copy c:\dvipsone\numeric.vec %1\vec

rem echo Copying c:\dvipsone\neon.vec
rem copy c:\dvipsone\neon.vec %1\vec
rem echo Copying c:\dvipsone\neonnew.vec
rem copy c:\dvipsone\neonnew.vec %1\vec

rem Following for testing to reduce time to dump

if "%2" == "SKIP" goto skipmetric

echo Making PFM subdirectory
if not exist %1\pfm\nul mkdir %1\pfm

rem PFM files are set up for StandardEncoding
echo Copying Courier PFMs
copy c:\dvipsone\pfm\com_____.pfs %1\pfm
copy c:\dvipsone\pfm\coo_____.pfs %1\pfm
copy c:\dvipsone\pfm\cob_____.pfs %1\pfm
copy c:\dvipsone\pfm\cobo____.pfs %1\pfm
echo Copying Gill Sans PFMs
copy c:\dvipsone\pfm\gn______.pfs %1\pfm
copy c:\dvipsone\pfm\gni_____.pfs %1\pfm
copy c:\dvipsone\pfm\gnb_____.pfs %1\pfm
copy c:\dvipsone\pfm\gnbi____.pfs %1\pfm
echo Copying Helvetica PFMs
copy c:\dvipsone\pfm\hv______.pfs %1\pfm
copy c:\dvipsone\pfm\hvo_____.pfs %1\pfm
copy c:\dvipsone\pfm\hvb_____.pfs %1\pfm
copy c:\dvipsone\pfm\hvbo____.pfs %1\pfm
echo Copying TimesNewRomanPS PFMs
copy c:\dvipsone\pfm\mtr_____.pfs %1\pfm
copy c:\dvipsone\pfm\mti_____.pfs %1\pfm
copy c:\dvipsone\pfm\mtb_____.pfs %1\pfm
copy c:\dvipsone\pfm\mtbi____.pfs %1\pfm
echo Copying Times PFMs
copy c:\dvipsone\pfm\tir_____.pfs %1\pfm
copy c:\dvipsone\pfm\tii_____.pfs %1\pfm
copy c:\dvipsone\pfm\tib_____.pfs %1\pfm
copy c:\dvipsone\pfm\tibi____.pfs %1\pfm
echo Copying Symbol PFM
copy c:\dvipsone\pfm\sy______.pfm %1\pfm
copy c:\dvipsone\pfm\zd______.pfm %1\pfm

copy c:\dvipsone\pfm\readme.txt %1\pfm

echo Making subdirectory %1\tfm
if not exist %1\tfm\nul mkdir %1\tfm

rem TFM files are set up for StandardEncoding
copy c:\dvipsone\tfm\readme.txt %1\tfm
echo Copying Courier TFMs
copy c:\dvipsone\tfm\com.tfs %1\tfm
rem copy c:\dvipsone\tfm\comz.tfs %1\tfm
copy c:\dvipsone\tfm\com.tfa %1\tfm
copy c:\dvipsone\tfm\coo.tfs %1\tfm
rem copy c:\dvipsone\tfm\cooz.tfs %1\tfm
copy c:\dvipsone\tfm\coo.tfa %1\tfm
copy c:\dvipsone\tfm\cob.tfs %1\tfm
rem copy c:\dvipsone\tfm\cobz.tfs %1\tfm
copy c:\dvipsone\tfm\cob.tfa %1\tfm
copy c:\dvipsone\tfm\cobo.tfs %1\tfm
rem copy c:\dvipsone\tfm\coboz.tfs %1\tfm
copy c:\dvipsone\tfm\cobo.tfa %1\tfm
echo Copying Helvetica TFMs
copy c:\dvipsone\tfm\hv.tfs %1\tfm
rem copy c:\dvipsone\tfm\hvz.tfs %1\tfm
copy c:\dvipsone\tfm\hv.tfa %1\tfm
copy c:\dvipsone\tfm\hvo.tfs %1\tfm
rem copy c:\dvipsone\tfm\hvoz.tfs %1\tfm
copy c:\dvipsone\tfm\hvo.tfa %1\tfm
copy c:\dvipsone\tfm\hvb.tfs %1\tfm
rem copy c:\dvipsone\tfm\hvbz.tfs %1\tfm
copy c:\dvipsone\tfm\hvb.tfa %1\tfm
copy c:\dvipsone\tfm\hvbo.tfs %1\tfm
rem copy c:\dvipsone\tfm\hvboz.tfs %1\tfm
copy c:\dvipsone\tfm\hvbo.tfa %1\tfm
echo Copying Gill Sans TFMs
copy c:\dvipsone\tfm\gn.tfs %1\tfm
rem copy c:\dvipsone\tfm\gnz.tfs %1\tfm
copy c:\dvipsone\tfm\gn.tfa %1\tfm
copy c:\dvipsone\tfm\gni.tfs %1\tfm
rem copy c:\dvipsone\tfm\gniz.tfs %1\tfm
copy c:\dvipsone\tfm\gni.tfa %1\tfm
copy c:\dvipsone\tfm\gnb.tfs %1\tfm
rem copy c:\dvipsone\tfm\gnbz.tfs %1\tfm
copy c:\dvipsone\tfm\gnb.tfa %1\tfm
copy c:\dvipsone\tfm\gnbi.tfs %1\tfm
rem copy c:\dvipsone\tfm\gnbiz.tfs %1\tfm
copy c:\dvipsone\tfm\gnbi.tfa %1\tfm
echo Copying ArialMT TFMs
copy c:\dvipsone\tfm\_a.tfs %1\tfm
rem copy c:\dvipsone\tfm\_az.tfs %1\tfm
copy c:\dvipsone\tfm\_a.tfa %1\tfm
copy c:\dvipsone\tfm\_ai.tfs %1\tfm
rem copy c:\dvipsone\tfm\_aiz.tfs %1\tfm
copy c:\dvipsone\tfm\_ai.tfa %1\tfm
copy c:\dvipsone\tfm\_ab.tfs %1\tfm
rem copy c:\dvipsone\tfm\_abz.tfs %1\tfm
copy c:\dvipsone\tfm\_ab.tfa %1\tfm
copy c:\dvipsone\tfm\_abi.tfs %1\tfm
rem copy c:\dvipsone\tfm\_abiz.tfs %1\tfm
copy c:\dvipsone\tfm\_abi.tfa %1\tfm
echo Copying Times TFMs
copy c:\dvipsone\tfm\tir.tfs %1\tfm
rem copy c:\dvipsone\tfm\tirz.tfs %1\tfm
copy c:\dvipsone\tfm\tir.tfa %1\tfm
copy c:\dvipsone\tfm\tii.tfs %1\tfm
rem copy c:\dvipsone\tfm\tiiz.tfs %1\tfm
copy c:\dvipsone\tfm\tii.tfa %1\tfm
copy c:\dvipsone\tfm\tib.tfs %1\tfm
rem copy c:\dvipsone\tfm\tibz.tfs %1\tfm
copy c:\dvipsone\tfm\tib.tfa %1\tfm
copy c:\dvipsone\tfm\tibi.tfs %1\tfm
rem copy c:\dvipsone\tfm\tibiz.tfs %1\tfm
copy c:\dvipsone\tfm\tibi.tfa %1\tfm
echo Copying TimesNewRomanPS TFMs
copy c:\dvipsone\tfm\mtr.tfs %1\tfm
rem copy c:\dvipsone\tfm\mtrz.tfs %1\tfm
copy c:\dvipsone\tfm\mtr.tfa %1\tfm
copy c:\dvipsone\tfm\mti.tfs %1\tfm
rem copy c:\dvipsone\tfm\mtiz.tfs %1\tfm
copy c:\dvipsone\tfm\mti.tfa %1\tfm
copy c:\dvipsone\tfm\mtb.tfs %1\tfm
rem copy c:\dvipsone\tfm\mtbz.tfs %1\tfm
copy c:\dvipsone\tfm\mtb.tfa %1\tfm
copy c:\dvipsone\tfm\mtbi.tfs %1\tfm
rem copy c:\dvipsone\tfm\mtbiz.tfs %1\tfm
copy c:\dvipsone\tfm\mtbi.tfa %1\tfm
echo Copying Symbol TFM
copy c:\dvipsone\tfm\sy.tfm %1\tfm
copy c:\dvipsone\tfm\zd.tfm %1\tfm

echo Copying TFM files for TrueType fonts

xcopy c:\windev\samples\dviwindo\tfm\*.tfm %1\tfm
goto skippctex

echo Copying Arial TrueType TFM
copy c:\pctex\textfms\arial.tfm %1\tfm
copy c:\pctex\textfms\ariali.tfm %1\tfm
copy c:\pctex\textfms\arialbd.tfm %1\tfm
copy c:\pctex\textfms\arialbi.tfm %1\tfm
echo Copying Courier New TrueType TFM
copy c:\pctex\textfms\cour.tfm %1\tfm
copy c:\pctex\textfms\couri.tfm %1\tfm
copy c:\pctex\textfms\courbd.tfm %1\tfm
copy c:\pctex\textfms\courbi.tfm %1\tfm
echo Copying Times TrueType TFM
copy c:\pctex\textfms\times.tfm %1\tfm
copy c:\pctex\textfms\timesi.tfm %1\tfm
copy c:\pctex\textfms\timesbd.tfm %1\tfm
copy c:\pctex\textfms\timesbi.tfm %1\tfm

echo Copying Symbol TrueType TFM
copy c:\pctex\textfms\symbol.tfm %1\tfm

copy c:\pctex\textfms\wingding.tfm %1\tfm

rem Next lot are for fonts on auxiliary diskette

echo Copying Lucida Bright TFM
copy c:\pctex\textfms\lbrite.tfm %1\tfm
copy c:\pctex\textfms\lbritei.tfm %1\tfm
copy c:\pctex\textfms\lbrited.tfm %1\tfm
copy c:\pctex\textfms\lbritedi.tfm %1\tfm
echo Copying Lucida Fax TFM
copy c:\pctex\textfms\lfax.tfm %1\tfm
copy c:\pctex\textfms\lfaxi.tfm %1\tfm
copy c:\pctex\textfms\lfaxd.tfm %1\tfm
copy c:\pctex\textfms\lfaxdi.tfm %1\tfm
echo Copying Lucida Sans TFM
copy c:\pctex\textfms\lsans.tfm %1\tfm
copy c:\pctex\textfms\lsansi.tfm %1\tfm
copy c:\pctex\textfms\lsansd.tfm %1\tfm
copy c:\pctex\textfms\lsansdi.tfm %1\tfm
echo Copying Lucida Sans Typewriter TFM
copy c:\pctex\textfms\ltype.tfm %1\tfm
copy c:\pctex\textfms\ltypeo.tfm %1\tfm
copy c:\pctex\textfms\ltypeb.tfm %1\tfm
copy c:\pctex\textfms\ltypebo.tfm %1\tfm
echo Copying Lucida Bright Math TFM
copy c:\pctex\textfms\lmath*.tfm %1\tfm
echo Copying Decorative font TFMs
copy c:\pctex\textfms\larrows.tfm %1\tfm
copy c:\pctex\textfms\licons.tfm %1\tfm
copy c:\pctex\textfms\lstars.tfm %1\tfm

goto skippctex

:skipmetric
echo WARNING: skipping dumping metric files

:skippctex
echo 
echo Making subdirectory %1\pfb
if not exist %1\pfb\nul mkdir %1\pfb

echo Copying c:\latxfont\logo*.pfb from c:\latxfont
xcopy c:\latxfont\logo*.pfb %1\pfb
copy c:\dvipsone\pfb\readme.txt %1\pfb
if exist %1\pfb\logoft10.pfb del %1\pfb\logoft10.pfb
if exist %1\pfb\logosk30.pfb del %1\pfb\logosk30.pfb

echo Copying c:\latxfont\logo*.pfm
xcopy c:\latxfont\logo*.pfm %1\pfb
echo 

echo copying c:\tex\accents.tex
copy c:\tex\accents.tex %1\tex
echo copying c:\tex\ansiacce.tex
copy c:\tex\ansiacce.tex %1\tex
echo copying c:\tex\stanacce.tex
copy c:\tex\stanacce.tex %1\tex
echo copying c:\tex\dcaccent.tex
copy c:\tex\dcaccent.tex %1\tex

rem echo copying c:\dvitest\toobig.dvi
rem copy c:\dvitest\toobig.dvi %1\tex

rem mkdir %1\dvicopy
rem echo copying c:\dvicopy\dvicopy.exe
rem copy c:\dvicopy\dvicopy.exe %1\dvicopy
rem echo copying c:\dvicopy\license.gnu 
rem copy c:\dvicopy\license.gnu %1\dvicopy

rem echo copying c:\windev\samples\dviwindo\hyper.txt
echo copying c:\txt\hyper.txt
rem copy c:\windev\samples\dviwindo\hyper.txt %1\txt
copy c:\txt\hyper.txt %1\txt
echo copying c:\tex\hyper.tex
copy c:\tex\hyper.tex %1\tex

echo rest won't fit on 5 1/4"

if not "%1" == "a:" goto allinone
echo Finished with diskette A, remove and insert diskette B
pause

echo copying c:\windev\samples\dviwindo\copyrght.txt
copy c:\windev\samples\dviwindo\copyrght.txt %1

:allinone

echo copying c:\txt\atmfix.txt
copy c:\txt\atmfix.txt %1\txt

echo copying c:\prog\atmfix.exe
copy c:\prog\atmfix.exe %1\prog

echo copying c:\metrics\safeseac.exe
copy c:\metrics\safeseac.exe %1\prog

echo copying c:\txt\safeseac.txt
copy c:\txt\safeseac.txt %1\txt

rem NO SPACE FOR EHANDLER

rem mkdir %1\ps
rem echo copying c:\ps\ehandler.ps
rem copy c:\ps\ehandler.ps %1\ps

rem NO SPACE FOR LPREP corrections - use LaserWriter 8.1.1 instead!

rem echo Copying c:\ps\lprep68.pro
rem copy c:\ps\lprep68.pro %1\ps
rem echo Copying c:\ps\lprep70.pro
rem copy c:\ps\lprep70.pro %1\ps
rem echo Copying c:\ps\lprep71.pro
rem copy c:\ps\lprep71.pro %1\ps

echo Copying encode.bat
copy c:\bat\encode.bat %1

echo Copying c:\prog\decode.exe
copy c:\prog\decode.exe %1\prog

echo Copying c:\tex\emlines.tex 
copy c:\tex\emlines.tex %1\tex

echo copying c:\windev\samples\dviwindo\atmone.txt
copy c:\windev\samples\dviwindo\atmone.txt %1\txt
echo copying c:\windev\samples\dviwindo\atmtwo.txt
copy c:\windev\samples\dviwindo\atmtwo.txt %1\txt

echo copying c:\prog\pfmtoafm.exe
copy c:\prog\pfmtoafm.exe %1\prog

echo copying c:\tex\showtiff.tex
copy c:\tex\showtiff.tex %1\tex

rem Some stuff in following flushed to make space

echo copying c:\tex\truetype.tex 
copy c:\tex\truetype.tex %1\tex

rem del %1\dviwindo.fn5

echo copying c:\txt\dos.txt
copy c:\txt\dos.txt %1\txt

rem echo copying c:\dvitest\truetype.dvi
rem copy c:\dvitest\truetype.dvi %1\tex

echo copying c:\tex\logotest.tex
copy c:\tex\logotest.tex %1\tex

rem Rest had to be flushed to make space...

rem echo copying c:\dvitest\logotest.dvi
rem copy c:\dvitest\logotest.dvi %1\tex

rem Don't need following for Windows 3.1

rem echo Probably won't fit on diskette (or give sharing violation)

rem echo copying c:\windows\system\shell.dll
rem copy c:\windows\system\shell.dll %1

rem echo copying c:\windows\system\commdlg.dll
rem copy c:\windows\system\commdlg.dll %1

rem echo copying c:\windows\system\toolhelp.dll
rem copy c:\windows\system\toolhelp.dll %1

rem echo copying c:\windows\system\lzexpand.dll
rem copy c:\windows\system\lzexpand.dll %1

rem echo copying Dr. Watson! (from c:\windows)
rem copy c:\windows\drwatson.exe %1\prog

echo Copying c:\tex\ansitest.tex 
copy c:\tex\ansitest.tex %1\tex

echo copying c:\dvipsone\ansiacce.sub
copy c:\dvipsone\ansiacce.sub %1

rem SAFESEAC needs PFAtoPFB and PFBtoPFA ...
rem Font sets have PFBtoPFA and PFAtoPFB

echo Copying c:\prog\pfatopfb.exe 
copy c:\prog\pfatopfb.exe %1\prog
echo Copying c:\prog\pfbtopfa.exe 
copy c:\prog\pfbtopfa.exe %1\prog

rem DVIPSONE has serial, download, modex, twoup, pktops, tfmtoafm

rem echo Copying c:\prog\ask.com
rem copy c:\prog\ask.com %1
echo Copying c:\utility\cdd.com
copy c:\utility\cdd.com %1

echo copying c:\windev\samples\dviwindo\epsfprev.txt
copy c:\windev\samples\dviwindo\epsfprev.txt %1\txt

echo copying c:\txt\sciword.txt
copy c:\txt\sciword.txt %1\txt

echo copying c:\txt\ai-bugs.txt 
copy c:\txt\ai-bugs.txt %1\txt

echo copying c:\tex\bpifont.tex
copy c:\tex\bpifont.tex %1\tex

rem Had to flush these to make space

rem echo Copying c:\windows\sysseg.exe
rem copy c:\windows\sysseg.exe %1\prog

echo Compressing SysSeg.EXE
rem c:\windev\bin\compress -r c:\windows\sysseg.exe .
rem c:\windev\bin\compress -r c:\windows\sysseg.exe %1
c:\windev\bin\compress -r c:\windev\samples\sysseg\sysseg.exe %1\prog
if errorlevel = 1 pause

echo change back to top-level
cd %1\

echo copying c:\txt\lucidmat.txt
copy c:\txt\lucidmat.txt %1\txt

echo copying c:\tex\lbrplain.tex
copy c:\tex\lbrplain.tex %1\tex

rem echo copying c:\tex\lbrlatex.tex
rem copy c:\tex\lbrlatex.tex %1\tex

echo copying c:\txt\integrate.txt 
copy c:\txt\integrate.txt %1\txt

echo switch back to top-level directory
cd %1\

rem leave place holder
echo dviwindo 1.0.8 >> %1\dviwindo

:end
