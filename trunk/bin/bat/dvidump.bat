@echo off
rem Copyright (C) 1990, 1991 Y&Y.
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%1" == "" goto argok
echo Usage:   dvidump [to-drive]:    e.g.   dvidump a:    or    dvidump b:
goto end
:argok
if not exist %1\*.* goto starmiss
dir %1
echo ERROR: Disk is not empty!
goto end
:starmiss
if "%1" == "a:" goto setlabel
if "%1" == "b:" goto setlabel
goto skiplabel
:setlabel
echo Setting Volume Label (with version number)
rem label %1DVIPSONE09 
rem label %1DVIPSONE095
rem label %1DVIPSONE096
rem label %1DVIPSONE098
rem label %1DVIPSONE100
rem label %1DVIPSONE101
rem label %1DVIPSONE102
rem label %1DVIPSONE103
rem label %1DVIPSONE104
rem label %1DVIPSONE105
rem label %1DVIPSONE106
rem label %1DVIPSONE107
label %1DVIPSONE108
rem
:skiplabel
mkdir %1\prog
rem
echo Copying c:\dvipsone\dvipsone.exe
copy c:\dvipsone\dvipsone.exe %1
rem copy c:\dvipsone\dvipsone.exe %1\prog
rem
echo Copying c:\prog\afmtotfm.exe
rem copy c:\prog\afmtotfm.exe %1
copy c:\prog\afmtotfm.exe %1\prog
rem 
echo Copying c:\prog\download.exe
rem copy c:\prog\download.exe %1
copy c:\prog\download.exe %1\prog
echo Copying c:\prog\serial.exe
rem copy c:\prog\serial.exe %1
copy c:\prog\serial.exe %1\prog
echo Copying c:\prog\modex.com
copy c:\prog\modex.com %1
rem
rem echo Copying c:\prog\pktops.exe
echo Copying c:\prog\pktops.exe\prog
copy c:\prog\pktops.exe %1
echo Copying c:\prog\twoup.exe
rem copy c:\prog\twoup.exe %1
copy c:\prog\twoup.exe %1\prog
rem
echo Copying c:\dvipsone\install.bat
copy c:\dvipsone\install.bat %1
echo Copying c:\prog\ask.com
copy c:\prog\ask.com %1
echo Copying c:\utility\cdd.com
copy c:\utility\cdd.com %1
rem
rem
if exist c:\dvipsone\dvipreamb.enc goto preamb
echo ERROR dvipreamb.enc missing
pause
:preamb
echo Copying c:\dvipsone\dvipreamb.enc
copy c:\dvipsone\dvipreamb.enc %1
echo Copying c:\dvipsone\dvitpics.enc
copy c:\dvipsone\dvitpics.enc %1
echo Copying c:\dvipsone\dvifont3.enc
copy c:\dvipsone\dvifont3.enc %1
echo 
echo Making subdirectory %1\pfb
mkdir %1\pfb
echo Copying c:\latxfont\logo*.pfb from c:\latxfont
xcopy c:\latxfont\logo*.pfb %1\pfb
rem
if exist %1\pfb\logoft10.pfb del %1\pfb\logoft10.pfb
if exist %1\pfb\logosk30.pfb del %1\pfb\logosk30.pfb
rem 
echo Copying c:\latxfont\logo*.pfm
xcopy c:\latxfont\logo*.pfm %1\pfb
rem
copy c:\dvipsone\pfb\readme.txt %1\pfb
rem
rem manfnt.ps won't fit on 5 1/4" ?
rem
echo Copying c:\dvipsone\pfb\*.ps
xcopy c:\dvipsone\pfb\*.ps %1\pfb
rem need the space!!!
del %1\pfb\manfont.ps
rem
echo 
echo Making subdirectory %1\afm
mkdir %1\afm
rem
echo Copying c:\dvipsone\afm\*.afm
xcopy c:\dvipsone\afm\*.afm %1\afm
rem
copy c:\dvipsone\afm\readme.txt %1\afm
rem copy c:\dvipsone\afm\*.afm %1\afm
echo 
echo Copying c:\dvipsone\readme.txt
copy c:\dvipsone\readme.txt %1
rem
if not exist c:\dvipsone\news.txt goto nonews
echo copying c:\dvipsone\news.txt
copy c:\dvipsone\news.txt %1
:nonews
rem
mkdir %1\txt
rem
echo copying c:\txt\dos.txt
rem copy c:\txt\dos.txt %1
copy c:\txt\dos.txt %\txt
rem
echo copying c:\txt\morass.txt
rem copy c:\txt\morass.txt %1
copy c:\txt\morass.txt %1\txt
rem
echo Copying c:\txt\psfonts.txt
rem copy c:\txt\psfonts.txt %1
copy c:\txt\psfonts.txt %1\txt
rem
echo Copying c:\dvipsone\copyrght.txt
copy c:\dvipsone\copyrght.txt %1
rem
echo Copying c:\dvipsone\question.txt
rem copy c:\dvipsone\question.txt %1
copy c:\dvipsone\question.txt %1\txt
echo Copying c:\dvipsone\controlc.txt
rem copy c:\dvipsone\controlc.txt %1
copy c:\dvipsone\controlc.txt %1\txt
echo Copying c:\dvipsone\controld.txt
rem copy c:\dvipsone\controld.txt %1
copy c:\dvipsone\controld.txt %1\txt
echo copying c:\txt\lucidmat.txt
rem copy c:\txt\lucidmat.txt %1
copy c:\txt\lucidmat.txt %1\txt
echo copying c:\txt\tpicspec.txt
rem copy c:\txt\tpicspec.txt %1
copy c:\txt\tpicspec.txt %1\txt
rem
echo copying c:\txt\psfigfix.txt
rem copy c:\txt\psfigfix.txt %1
copy c:\txt\psfigfix.txt %1\txt
rem
mkdir %1\tex
rem
echo copying c:\tex\epsfsafe.tex 
rem copy c:\tex\epsfsafe.tex %1
copy c:\tex\epsfsafe.tex %1\tex
rem
echo copying c:\tex\tpictest.tex
rem copy c:\tex\tpictest.tex %1
copy c:\tex\tpictest.tex %1\tex
rem
echo copying c:\tex\accents.tex
rem copy c:\tex\accents.tex %1
copy c:\tex\accents.tex %1\tex
echo copying c:\tex\stanacce.tex
rem copy c:\tex\stanacce.tex %1
copy c:\tex\stanacce.tex %1\tex
echo copying c:\tex\ansiacce.tex
rem copy c:\tex\ansiacce.tex %1
copy c:\tex\ansiacce.tex %1\tex
echo copying c:\tex\dcaccent.tex
rem copy c:\tex\dcaccent.tex %1
copy c:\tex\dcaccent.tex %1\tex
echo 
echo Making subdirectory %1\sub
mkdir %1\sub
echo Copying c:\dvipsone\adobe.sub
copy c:\dvipsone\adobe.sub %1\sub
echo Copying c:\dvipsone\arborcom.sub
copy c:\dvipsone\arborcom.sub %1\sub
echo Copying c:\dvipsone\arborres.sub
copy c:\dvipsone\arborres.sub %1\sub
echo Copying c:\dvipsone\combined.sub
copy c:\dvipsone\combined.sub %1\sub
echo Copying c:\dvipsone\forced.sub
copy c:\dvipsone\forced.sub %1\sub
echo Copying c:\dvipsone\resident.sub
copy c:\dvipsone\resident.sub %1\sub
echo Copying c:\dvipsone\sample.sub
copy c:\dvipsone\sample.sub %1\sub
echo Copying c:\dvipsone\standard.sub
copy c:\dvipsone\standard.sub %1\sub
echo copying c:\dvipsone\amsfonts.sub
copy c:\dvipsone\amsfonts.sub %1\sub
echo copying c:\dvipsone\mtmi.sub
copy c:\dvipsone\mtmi.sub %1\sub
echo copying c:\dvipsone\cmreside.sub
copy c:\dvipsone\cmreside.sub %1\sub
rem
copy c:\dvipsone\ansiacce.sub %1\sub
echo Copying c:\dvipsone\ansiacce.sub
rem
rem echo copying c:\dvipsone\euler.sub
rem copy c:\dvipsone\amtocm.sub %1\sub
rem
echo copying c:\dvipsone\berry.sub
copy c:\dvipsone\berry.sub %1\sub
rem
echo 
echo Making subdirectory %1\vec
mkdir %1\vec
echo Copying c:\dvipsone\ansi.vec
copy c:\dvipsone\ansi.vec %1\vec
echo Copying c:\dvipsone\ansinew.vec
copy c:\dvipsone\ansinew.vec %1\vec
rem echo Copying c:\dvipsone\default.vec
rem copy c:\dvipsone\default.vec %1\vec
echo Copying c:\dvipsone\dingbats.vec
copy c:\dvipsone\dingbats.vec %1\vec
rem
echo Copying c:\dvipsone\mathit.vec
copy c:\dvipsone\mathit.vec %1\vec
echo Copying c:\dvipsone\mathsy.vec
copy c:\dvipsone\mathsy.vec %1\vec
echo Copying c:\dvipsone\mathex.vec
copy c:\dvipsone\mathex.vec %1\vec
echo Copying c:\dvipsone\numeric.vec
copy c:\dvipsone\numeric.vec %1\vec
rem
echo Copying c:\dvipsone\standard.vec
copy c:\dvipsone\standard.vec %1\vec
rem echo Copying c:\dvipsone\isolatin.vec
rem copy c:\dvipsone\isolatin.vec %1\vec
echo Copying c:\dvipsone\isolati1.vec
copy c:\dvipsone\isolati1.vec %1\vec
echo Copying c:\dvipsone\isolati2.vec
copy c:\dvipsone\isolati2.vec %1\vec
echo Copying c:\dvipsone\mac.vec
copy c:\dvipsone\mac.vec %1\vec
rem
rem echo Copying c:\dvipsone\neon.vec
rem copy c:\dvipsone\neon.vec %1\vec
rem echo Copying c:\dvipsone\neonnew.vec
rem copy c:\dvipsone\neonnew.vec %1\vec
rem
echo Copying c:\dvipsone\tex256.vec
copy c:\dvipsone\tex256.vec %1\vec
echo copying c:\dvipsone\cyrillic.vec
copy c:\dvipsone\cyrillic.vec %1\vec
rem
echo copying c:\dvipsone\textures.vec 
copy c:\dvipsone\textures.vec %1\vec
echo copying c:\dvipsone\textutyp.vec 
copy c:\dvipsone\textutyp.vec %1\vec
echo copying c:\dvipsone\texmac.vec 
copy c:\dvipsone\texmac.vec %1\vec
rem
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
rem
echo Copying c:\dvipsone\ventura.vec
copy c:\dvipsone\ventura.vec %1\vec
echo copying c:\dvipsone\hproman.vec
copy c:\dvipsone\hproman.vec %1\vec
rem
rem echo copying c:\dvipsone\vproman.vec
rem copy c:\dvipsone\vproman.vec %1\vec
rem
echo Copying c:\dvipsone\fontogra.vec
copy c:\dvipsone\fontogra.vec %1\vec
echo Copying c:\dvipsone\ibmoem.vec
copy c:\dvipsone\ibmoem.vec %1\vec
rem echo Copying c:\dvipsone\truetype.vec
rem copy c:\dvipsone\truetype.vec %1\vec
rem echo Copying c:\dvipsone\texansi.vec
echo Copying c:\dvipsone\tex&ansi.vec
rem copy c:\dvipsone\texansi.vec %1\vec
copy c:\dvipsone\tex&ansi.vec %1\vec
rem echo Copying c:\dvipsone\texannew.vec
rem copy c:\dvipsone\texannew.vec %1\vec
rem
echo copying c:\dvipsone\dos437.vec %1\vec
copy c:\dvipsone\dos437.vec %1\vec
echo copying c:\dvipsone\dos850.vec %1\vec
copy c:\dvipsone\dos850.vec %1\vec
echo 
rem Following is debugging speed up
if "%2" == "SKIP" goto skiptfm
rem
if exist %1\tfm\*.* goto con
goto maketfm
rem
echo Do you wish to create the TFM subdirectory on %1 [Y]?
rem errorlevel = 2 for Y, errorlevel = 0 for N, errorlevel = 1 for CR
ask
if errorlevel = 2 goto maketfm
if errorlevel = 1 goto maketfm
goto notfm
rem
:maketfm
echo Making subdirectory %1\tfm
mkdir %1\tfm
:con
rem if not exist %1\tfm\*.* goto notfm
goto copytfm
echo Do you wish to copy the TFM files [Y]?
ask
if not errorlevel = 1 goto notfm
:copytfm
echo TFM files WITH extension `tfs' are set up for StandardEncoding
echo TFM files WITH extension `tfx' are set up for TeX text encoding
rem
rem xcopy c:\dvipsone\tfm\*.tfs %1\tfm
rem
copy c:\dvipsone\tfm\readme.txt %1\tfm
rem
echo Copying Courier TFMs
copy c:\dvipsone\tfm\com.tfs %1\tfm
rem copy c:\dvipsone\tfm\comx.tfs %1\tfm
copy c:\dvipsone\tfm\com.tfx %1\tfm
copy c:\dvipsone\tfm\coo.tfs %1\tfm
rem copy c:\dvipsone\tfm\coox.tfs %1\tfm
copy c:\dvipsone\tfm\coo.tfx %1\tfm
copy c:\dvipsone\tfm\cob.tfs %1\tfm
rem copy c:\dvipsone\tfm\cobx.tfs %1\tfm
copy c:\dvipsone\tfm\cob.tfx %1\tfm
copy c:\dvipsone\tfm\cobo.tfs %1\tfm
rem copy c:\dvipsone\tfm\cobox.tfs %1\tfm
copy c:\dvipsone\tfm\cobo.tfx %1\tfm
echo Copying Helvetica TFMs
copy c:\dvipsone\tfm\hv.tfs %1\tfm
rem copy c:\dvipsone\tfm\hvx.tfs %1\tfm
copy c:\dvipsone\tfm\hv.tfx %1\tfm
copy c:\dvipsone\tfm\hvo.tfs %1\tfm
rem copy c:\dvipsone\tfm\hvox.tfs %1\tfm
copy c:\dvipsone\tfm\hvo.tfx %1\tfm
copy c:\dvipsone\tfm\hvb.tfs %1\tfm
rem copy c:\dvipsone\tfm\hvbx.tfs %1\tfm
copy c:\dvipsone\tfm\hvb.tfx %1\tfm
copy c:\dvipsone\tfm\hvbo.tfs %1\tfm
rem copy c:\dvipsone\tfm\hvbox.tfs %1\tfm
copy c:\dvipsone\tfm\hvbo.tfx %1\tfm
echo Copying Gill Sans TFMs
copy c:\dvipsone\tfm\gn.tfs %1\tfm
rem copy c:\dvipsone\tfm\gnx.tfs %1\tfm
copy c:\dvipsone\tfm\gn.tfx %1\tfm
copy c:\dvipsone\tfm\gni.tfs %1\tfm
rem copy c:\dvipsone\tfm\gnix.tfs %1\tfm
copy c:\dvipsone\tfm\gni.tfx %1\tfm
copy c:\dvipsone\tfm\gnb.tfs %1\tfm
rem copy c:\dvipsone\tfm\gnbx.tfs %1\tfm
copy c:\dvipsone\tfm\gnb.tfx %1\tfm
copy c:\dvipsone\tfm\gnbi.tfs %1\tfm
rem copy c:\dvipsone\tfm\gnbix.tfs %1\tfm
copy c:\dvipsone\tfm\gnbi.tfx %1\tfm
echo Copying ArialMT TFMs
rem copy c:\dvipsone\tfm\_ax.tfs %1\tfm
copy c:\dvipsone\tfm\_a.tfx %1\tfm
copy c:\dvipsone\tfm\_ai.tfs %1\tfm
rem copy c:\dvipsone\tfm\_aix.tfs %1\tfm
copy c:\dvipsone\tfm\_ai.tfx %1\tfm
copy c:\dvipsone\tfm\_ab.tfs %1\tfm
rem copy c:\dvipsone\tfm\_abx.tfs %1\tfm
copy c:\dvipsone\tfm\_ab.tfx %1\tfm
copy c:\dvipsone\tfm\_abi.tfs %1\tfm
rem copy c:\dvipsone\tfm\_abix.tfs %1\tfm
copy c:\dvipsone\tfm\_abi.tfx %1\tfm
echo Copying Times TFMs
copy c:\dvipsone\tfm\tir.tfs %1\tfm
rem copy c:\dvipsone\tfm\tirx.tfs %1\tfm
copy c:\dvipsone\tfm\tir.tfx %1\tfm
copy c:\dvipsone\tfm\tii.tfs %1\tfm
rem copy c:\dvipsone\tfm\tiix.tfs %1\tfm
copy c:\dvipsone\tfm\tii.tfx %1\tfm
copy c:\dvipsone\tfm\tib.tfs %1\tfm
rem copy c:\dvipsone\tfm\tibx.tfs %1\tfm
copy c:\dvipsone\tfm\tib.tfx %1\tfm
copy c:\dvipsone\tfm\tibi.tfs %1\tfm
rem copy c:\dvipsone\tfm\tibix.tfs %1\tfm
copy c:\dvipsone\tfm\tibi.tfx %1\tfm
echo Copying TimesNewRomanPS TFMs
copy c:\dvipsone\tfm\mtr.tfs %1\tfm
rem copy c:\dvipsone\tfm\mtrx.tfs %1\tfm
copy c:\dvipsone\tfm\mtr.tfx %1\tfm
copy c:\dvipsone\tfm\mti.tfs %1\tfm
rem copy c:\dvipsone\tfm\mtix.tfs %1\tfm
copy c:\dvipsone\tfm\mti.tfx %1\tfm
copy c:\dvipsone\tfm\mtb.tfs %1\tfm
rem copy c:\dvipsone\tfm\mtbx.tfs %1\tfm
copy c:\dvipsone\tfm\mtb.tfx %1\tfm
copy c:\dvipsone\tfm\mtbi.tfs %1\tfm
rem copy c:\dvipsone\tfm\mtbix.tfs %1\tfm
copy c:\dvipsone\tfm\mtbi.tfx %1\tfm
echo Copying Symbol TFM
copy c:\dvipsone\tfm\sy.tfm %1\tfm
copy c:\dvipsone\tfm\zd.tfm %1\tfm
rem
goto notfm
rem
:skiptfm
echo WARNING: NOT DUMPING TFM FILES
rem
:notfm
rem
rem Following stuff left out for various reasons
rem
echo copying c:\windows\dvipsone.pif
copy c:\windows\dvipsone.pif %1
rem
rem mkdir %1\dvicopy
rem echo copying c:\dvicopy\dvicopy.exe
rem copy c:\dvicopy\dvicopy.exe %1\dvicopy
rem echo copying c:\dvicopy\license.gnu 
rem copy c:\dvicopy\license.gnu %1\dvicopy
rem
echo They get the following for free!
rem
echo Copying c:\prog\tfmtoafm.exe
copy c:\prog\tfmtoafm.exe %1\prog
rem
rem Following encoding stuff really only relevant for DVIWindo?
rem
rem echo copying c:\prog\reencode.exe
rem copy c:\prog\reencode.exe %1\prog
rem
rem echo copying c:\bat\encode.bat
rem copy c:\bat\encode.bat %1
rem
rem echo copying c:\txt\encoding.txt %1
rem copy c:\txt\encoding.txt
rem
rem echo Copying c:\prog\decode.exe
rem copy c:\prog\decode.exe %prog
rem
echo Copying c:\tex\emlines.tex 
rem copy c:\tex\emlines.tex %1
copy c:\tex\emlines.tex %1\tex
rem
if not "%1" == "a:" goto allinone
echo Finished with diskette A, remove and insert diskette B
pause
echo copying c:\dvipsone\copyrght.txt
copy c:\dvipsone\copyrght.txt %1
rem
:allinone
rem
echo Making subdirectory %1\ps
mkdir %1\ps
rem
echo copying c:\ps\cropmarks.ps
rem copy c:\ps\cropmarks.ps %1
copy c:\ps\cropmarks.ps %1\ps
rem
rem There is an ehandler in c:\dvipsone\ps directory already
rem echo Copying c:\ps\ehandler.ps
rem copy c:\ps\ehandler.ps %1\ps
rem
rem echo Copying c:\ps\nhandler.ps
rem copy c:\ps\nhandler.ps %1\ps
rem
echo 
echo Copying c:\dvipsone\ps\readme.txt
copy c:\dvipsone\ps\readme.txt %1\ps
rem
echo Copying c:\dvipsone\ps\*.ps
xcopy c:\dvipsone\ps\*.ps %1\ps
echo 
rem
rem echo copying c:\dvitest\logotest.dvi
rem copy c:\dvitest\logotest.dvi %1
rem
echo copying c:\tex\logotest.tex
rem copy c:\tex\logotest.tex %1
copy c:\tex\logotest.tex %1\tex
rem
rem echo copying c:\tex\bpifont.texrem
rem copy c:\tex\bpifont.tex %1\tex
rem
rem Following highly optional!
rem
rem echo Copying c:\dvitest\chironcm.dvi
rem copy c:\dvitest\chironcm.dvi %1
rem echo Copying c:\dvitest\chironlb.dvi
rem copy c:\dvitest\chironlb.dvi %1
rem echo Copying c:\dvitest\chironmt.dvi
rem copy c:\dvitest\chironmt.dvi %1
rem
rem echo Copying c:\ps\chironlb.ps
rem copy c:\ps\chironlb.ps %1
rem
rem echo Copying c:\dvitest\toobig.dvi
rem copy c:\dvitest\toobig.dvi %1
rem
rem Following may not fit
rem
echo Copying c:\prog\pfatopfb.exe
copy c:\prog\pfatopfb.exe %1\prog
echo Copying c:\prog\pfbtopfa.exe
copy c:\prog\pfbtopfa.exe %1\prog
rem
echo Copying c:\prog\changed.exe %1
copy c:\prog\changed.exe %1\prog
rem
echo Copying c:\txt\changed.txt %1
rem copy c:\txt\changed.txt %1
copy c:\txt\changed.txt %1\txt
rem
echo copying c:\txt\verbatim.txt 
rem copy c:\txt\verbatim.txt %1
copy c:\txt\verbatim.txt %1\txt
rem
echo Following files are optional - now that LaserWrite 8.0 & 8.1 work!
rem
rem echo Copying c:\ps\lprep68.pro
rem copy c:\ps\lprep68.pro %1\ps
rem echo Copying c:\ps\lprep70.pro
rem copy c:\ps\lprep70.pro %1\ps
rem echo Copying c:\ps\lprep71.pro
rem copy c:\ps\lprep71.pro %1\ps
rem
:end
@echo on
