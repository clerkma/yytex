@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%1" == "Q" %0 a: Q
if "%1" == "q" %0 a: Q

if not "%1" == "" goto argok
echo Presumably you mean %0 a:
%0 a:
echo Impossible Error!
goto end

:argok
if exist %1\dviwindo.z goto seemsok
echo Sorry, apparently not dviwindo diskette
if "%2" == "FORCE" goto seemsok
goto end

:seemsok
rem make sure we are in top-level directory on diskette
chdir %1\
vol %1 | find "DVIWINDO"
if not errorlevel 1 goto goodlabel
echo WARNING: diskette does not have DVIWindo label!
pause
label %1DVIWINDO12
rem goto end

:goodlabel
rem echo NEW DVIWindo diskette update batch file!
rem
cdd c:\ishield\example4\working
rem cls

if "%2" == "Q" echo WARNING: QUICK DUMP ONLY
if "%2" == "Q" goto quick

echo DO NOT RUN IN EPSILON BUFFER - packing PKG.LST will fail

rem make some space
rem if exist %1\vec\ventura.vec del %1\vec\ventura.vec
if exist %1\vec\dingbats.vec del %1\vec\dingbats.vec
if exist %1\vec\8r.vec del %1\vec\8r.vec
if exist %1\vec\ibmoem.vec del %1\vec\ibmoem.vec
if exist %1\onthefly.txt del %1\onthefly.txt
rem if exist %1\dvi_help.txt del %1\dvi_help.txt

Echo Copying winowner.txt to example4 directory
rem copy c:\windev\samples\dviwindo\winowner.txt ..
copy c:\winsourc\winowner.txt ..
echo The owner of this copy of DVIWindo is:
rem Type c:\windev\samples\dviwindo\winowner.txt
Type c:\winsourc\winowner.txt

echo replacing c:\winsourc\readme.wri 
replace c:\winsourc\readme.wri .. /u/p

echo replacing c:\winsourc\dvi_help.dvi
replace c:\dvitest\dvi_help.dvi .. /u/p


rem ************************************************************************

echo refreshing files in \data\prog
replace c:\prog\*.exe ..\data\prog /U/P
replace c:\prog\*.com ..\data\prog /U/P
replace c:\metrics\*.exe ..\data\prog /U/P
rem
echo Refreshing executable files in \DATA\PROGMAIN
rem replace c:\windev\samples\dviwindo\*.exe ..\data\progmain /U/P
replace c:\winsourc\*.exe ..\data\progmain /U/P
rem replace c:\windev\samples\dviwindo\tiff*.dll ..\data\progmain /U/P
replace c:\winsourc\tiff*.dll ..\data\progmain /U/P
rem replace c:\windev\samples\sysseg\*.exe ..\data\progmain /U/P
replace c:\winsourc\*.exe ..\data\progmain /U/P
rem replace c:\windev\samples\cleanup\*.exe ..\data\progmain /U/P
replace c:\winsourc\*.exe ..\data\progmain /U/P
rem
rem echo Refreshing icons in \DATA\PROGMAIN
rem rem replace c:\windev\samples\dviwindo\*.ico ..\data\progmain /U/P
rem rem replace c:\windev\samples\sysseg\*.ico ..\data\progmain /U/P
rem rem replace c:\windev\samples\cleanup\*.ico ..\data\progmain /U/P

echo Refreshing text files in \DATA\TXT
replace c:\txt\*.txt ..\data\txt /U/P
rem replace c:\windev\samples\dviwindo\*.txt ..\data\txt /U/P
replace c:\winsourc\*.txt ..\data\txt /U/P
rem replace c:\windev\samples\dviwindo\*.txt ..\disk1 /U/P
replace c:\winsourc\*.txt ..\disk1 /U/P

echo Refreshing files in \DATA\TEX
replace c:\tex\*.tex ..\data\tex /U/P
replace c:\dvitest\*.dvi ..\data\tex /U/P

echo Refreshing vectors in VEC subdirctory
rem replace c:\dvipsone\*.vec ..\disk1\vec /U/P
rem replace c:\dvisourc\*.vec ..\disk1\vec /U/P
replace c:\dvisourc\vec\*.vec ..\disk1\vec /U/P

echo Refreshing miscellaneous (cdd.com, encode.bat, ansiacce.sub ... )
replace c:\prog\cdd.com ..\disk1\misc /U/P
replace c:\bat\encode.bat ..\disk1\misc /U/P

echo Refreshing PFB, TFM, FNT directories
rem replace c:\dvipsone\pfb\*.* ..\disk1\pfb /U/P
replace c:\dvisourc\pfb\*.* ..\disk1\pfb /U/P
rem replace c:\dvipsone\pfm\*.* ..\disk1\pfm /U/P
rem replace c:\dvipsone\tfm\*.* ..\disk1\tfm /U/P
rem replace c:\dvisourc\tfm\*.* ..\disk1\tfm /U/P
replace c:\winsourc\tfm\*.* ..\disk1\tfm /U/P
rem replace c:\windev\samples\dviwindo\*.fn* ..\disk1\fnt /U/P
replace c:\winsourc\*.fn* ..\disk1\fnt /U/P

rem ************************************************************************

rem Echo Removing readme.wri, winowner.txt and dviwindo.exe from DVIWINDO.Z
rem call icompres dviwindo.exe ..\disk1\dviwindo.z PROGMAIN -R -H
rem call icompres progmain\dviwindo.exe ..\disk1\dviwindo.z -R -H
rem call icompres winowner.txt ..\disk1\dviwindo.z -R -H
rem call icompres readme.wri ..\disk1\dviwindo.z -R -H

rem Check whether exeutable has been customized
find /N "bkphbkph"  c:\winsourc\dviwindo.exe > nul:
rem 0 found match (bad!), 1 no match found (good!), 2 error in FIND (ugly!)
if not errorlevel 2 goto findok
echo ERROR: in FIND for c:\winsourc\dviwindo.exe
pause
goto end

:findok
if errorlevel 1 goto customized
echo ERROR: c:\winsourc\dviwindo.exe IS NOT customized
pause
goto end

:customized
echo c:\winsourc\dviwindo.exe IS customized
rem
Echo replace DVIWINDO.EXE in DVIWINDO.Z
rem call icompres c:\windev\samples\dviwindo\dviwindo.exe ..\disk1\dviwindo.Z PROGMAIN -H 
call icompres c:\winsourc\dviwindo.exe ..\disk1\dviwindo.Z PROGMAIN -H

Echo replace README.WRI in DVIWINDO.Z
call icompres  ..\readme.wri ..\disk1\DVIWINDO.Z -H

Echo replace WINOWNER.TXT in DVIWINDO.Z
call icompres  ..\winowner.txt ..\disk1\DVIWINDO.Z -H

Echo replace DVI_HELP.DVI in DVIWINDO.Z
call icompres  ..\dvi_help.dvi ..\disk1\DVIWINDO.Z -H

rem Adjust file date to latest of given dates 95/May/5
copydate ..\disk1\dviwindo.z c:\winsourc\dviwindo.exe ..\readme.wri ..\dvi_help.dvi

rem pause
Echo Building SETUP.PKG Packaging List file...
rem packlist pkg.lst
call packlist pkg.lst
pause

Echo Copying Compiled Packaging list SETUP.PKG to DISK1 sub-directory...
copy setup.pkg ..\disk1
echo   Renaming SETUP.PKG  ---  INSTALL.PKG
if exist ..\disk1\install.pkg del ..\disk1\install.pkg
rename ..\disk1\setup.pkg install.pkg

rem cls
rem call icompres ..\disk1\dviwindo.z -l

echo Copying packing list INSTALL.PKG to diskette %1
copy ..\disk1\install.pkg %1

rem echo Refresh top level files on diskette (install.lgo setup.lgo)
rem replace ..\disk1\*.* %1 /U/P
rem replace *.* %1 /U/P

echo Refresh FNT directory files on diskette
replace ..\disk1\fnt\*.* %1\fnt /U/P

echo Refresh MISC directory files on diskette
replace ..\disk1\misc\*.* %1\misc /U/P

echo Refresh PFB directory files on diskette
replace ..\disk1\pfb\*.* %1\pfb /U/P

rem echo Refresh PFM directory files on diskette
rem replace ..\disk1\pfm\*.* %1\pfm /U/P

echo Refresh TEXMENU directory files on diskette
replace ..\disk1\texmenu\*.* %1\texmenu /U/P

echo Refresh TFM directory files on diskette
replace ..\disk1\tfm\*.* %1\tfm /U/P

echo Refresh VEC directory files on diskette
replace ..\disk1\vec\*.* %1\vec /U/P

rem echo replace c:\winsourc\*.txt a: /u/p
rem replace c:\winsourc\*.txt a: /u/p
rem (do following earlier now to create space...)
echo Refresh top level text files on diskette
replace ..\disk1\*.txt %1 /U/P

echo Copying compressed archive DVIWINDO.Z to diskette %1
copy ..\disk1\dviwindo.z %1

rem echo Copying packing list INSTALL.PKG to diskette %1
rem copy ..\disk1\install.pkg %1

rem if exist %1\misc\adobeafm.zip del %1\misc\adobeafm.zip

:quick

echo Refresh top level files on diskette
replace ..\disk1\*.* %1 /U/P

rem following is redundant
rem echo Refreshing INSTALL.INS
rem replace ..\disk1\install.ins %1 /U/P

rem echo Refresh MISC directory files on diskette
rem replace ..\disk1\misc\*.* %1\misc /U/P

rem echo Refresh TEXMENU directory files on diskette
rem replace ..\disk1\texmenu\*.* %1\texmenu /U/P

rem Temporary (will be in txt compressed file later)
rem if not exist %1\dvi_help.txt copy c:\txt\dvi_help.txt %1
rem if not exist %1\onthefly.txt copy c:\txt\onthefly.txt %1
replace c:\txt\*.txt %1 /U/P

dir %1 /W
echo Please verify that there is less than 100 k byte left on diskette.
echo Otherwise a file may have gotten deleted because of lack of space!
echo.
rem You can check the customization by typing c:\winsourc\dviwindo
rem (except, in Windows 95 that actually launches DVIWindo!)
c:\winsourc\dviwindo

:end
