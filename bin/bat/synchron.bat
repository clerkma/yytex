@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo Copy customized EXE and DLL files and
echo Synchronize files in c:\yyinstal with the rest of the world

rem we COPY the following rather than REPLACE,
rem because customization has changed the encrypted data in the file
rem but may not have changed the dates.

rem COPY SECTION

echo copy d:\dvisourc\dvipsone.exe c:\yyinstal\dvipsone
copy d:\dvisourc\dvipsone.exe c:\yyinstal\dvipsone
echo copy d:\dvisourc\dvipream.enc c:\yyinstal\dvipsone
copy d:\dvisourc\dvipream.enc c:\yyinstal\dvipsone
echo copy d:\dvisourc\dvifont3.enc c:\yyinstal\dvipsone
copy d:\dvisourc\dvifont3.enc c:\yyinstal\dvipsone
echo copy d:\dvisourc\dvitpics.enc c:\yyinstal\dvipsone
copy d:\dvisourc\dvitpics.enc c:\yyinstal\dvipsone

echo copy d:\winsourc\dviwindo.exe c:\yyinstal\dviwindo
copy d:\winsourc\dviwindo.exe c:\yyinstal\dviwindo
rem Following new 2000 March 7th
echo copy d:\winsourc\yandytex.dll c:\yyinstal\dviwindo
copy d:\winsourc\yandytex.dll c:\yyinstal\dviwindo
echo copy d:\winsourc\dvipsone.dll c:\yyinstal\dviwindo
copy d:\winsourc\dvipsone.dll c:\yyinstal\dviwindo
echo copy d:\winsourc\afmtotfm.dll c:\yyinstal\dviwindo
copy d:\winsourc\afmtotfm.dll c:\yyinstal\dviwindo

echo copy d:\texsourc\yandytex.exe c:\yyinstal\yandytex
copy d:\texsourc\yandytex.exe c:\yyinstal\yandytex

rem REPLACE SECTION
rem This is organized mostly in order of directory tree c:\yyinstal
rem Note /S in REPLACE searched sub-directories on DESTINATION

echo DOC-MISC XCOPY c:\flyers\*.txt c:\yyinstal\doc-misc /u/p/d
XCOPY c:\flyers\*.txt c:\yyinstal\doc-misc /u/p/d
echo DOC-MISC XCOPY c:\txt\*.txt c:\yyinstal\doc-misc /u/p/d
XCOPY c:\txt\*.txt c:\yyinstal\doc-misc /u/p/d

echo XCOPY c:\txt\*.* c:\yyinstal\yandytex\keyboard /u/p/d
XCOPY c:\txt\*.* c:\yyinstal\yandytex\keyboard /u/p/d

echo DVIPSONE XCOPY d:\dvisourc\*.* c:\yyinstal\dvipsone /s/u/p/d
XCOPY d:\dvisourc\*.* c:\yyinstal\dvipsone /s/u/p/d
echo DVIWINDO XCOPY d:\winsourc\*.* c:\yyinstal\dviwindo /s/u/p/d
XCOPY d:\winsourc\*.* c:\yyinstal\dviwindo /s/u/p/d
echo YANDYTEX XCOPY d:\texsourc\*.* c:\yyinstal\yandytex /s/u/p/d
XCOPY d:\texsourc\*.* c:\yyinstal\yandytex /s/u/p/d
echo YANDYTEX\FMT XCOPY d:\texsourc\fmt\*.* c:\yyinstal\yandytex\fmt /u/p/d
XCOPY d:\texsourc\fmt\*.* c:\yyinstal\yandytex\fmt /u/p/d

rem XCOPY d:\pfe32\*.* c:\yyinstal\pfe /u/p/d

echo PS XCOPY c:\ps\*.* c:\yyinstal\ps /u/p/d
XCOPY c:\ps\*.* c:\yyinstal\ps /u/p/d

echo TEX\BASE XCOPY c:\tex\*.* c:\yyinstal\tex\base /u/p/d
XCOPY c:\tex\*.* c:\yyinstal\tex\base /u/p/d

rem new as of 1997/July/10

echo EM\TFM XCOPY d:\em\tfm\texnansi c:\yyinstal\fonts\tfm\texnansi\tfm-em /u/p/d
XCOPY d:\em\tfm\texnansi\*.* c:\yyinstal\fonts\tfm\texnansi\tfm-em /u/p/d

echo EM\TFM XCOPY d:\em\tfm\tex256  c:\yyinstal\fonts\tfm\tex256\tfm-em /u/p/d
XCOPY d:\em\tfm\tex256\*.*  c:\yyinstal\fonts\tfm\tex256\tfm-em /u/p/d

echo EM\TFM XCOPY d:\em\tfm  c:\yyinstal\fonts\tfm\nontext\tfm-emmi /u/p/d
XCOPY d:\em\tfm\*.*  c:\yyinstal\fonts\tfm\nontext\tfm-emmi /u/p/d

echo Currently not updating AMS TeX
rem XCOPY c:\ams\amsfonts\*.* c:\yyinstal\tex\base\amsfonts /u/p/d
rem XCOPY c:\ams\amstex\*.* c:\yyinstal\tex\base\amstex /u/p/d

echo Currently not updating LaTeX 2.09
rem XCOPY c:\yandytex\latex\*.* c:\yyinstal\tex\latex209 /s/u/p/d

echo Currently not updating LaTeX 2e
rem XCOPY c:\yandytex\latex2e\*.* c:\yyinstal\tex\latex2e\base /u/p/d

echo TEXFMT XCOPY d:\texsourc\fmt\*.* c:\yyinstal\texfmt /s/u/p/d
XCOPY d:\texsourc\fmt\*.* c:\yyinstal\texfmt /s/u/p/d

echo TEXINPUT XCOPY c:\tex\*.* c:\yyinstal\texinput /u/p/d
XCOPY c:\tex\*.* c:\yyinstal\texinput /u/p/d

echo UTIL XCOPY c:\prog\*.* c:\yyinstal\util /u/p/d
XCOPY c:\prog\*.* c:\yyinstal\util /u/p/d

echo XCOPY c:\dvisourc\vec\*.* c:\yyinstal\fonts\encoding /u/p/d
XCOPY c:\dvisourc\vec\*.* c:\yyinstal\fonts\encoding /u/p/d

rem XCOPY c:\metrics\*.* c:\yyinstal\util /u/p/d

rem ********************************************************

echo Synchronize files on f: with the rest of the world

echo XCOPY d:\pdf\*.* f:\samples /u/p/d
XCOPY d:\pdf\*.* f:\samples /u/p/d

rem XCOPY d:\t1instal\*.* f:\t1instal /u/p/d
rem XCOPY d:\t1instal\winnt351\*.* f:\t1instal\winnt351 /u/p/d
rem XCOPY d:\t1instal\winnt400\*.* f:\t1instal\winnt400 /u/p/d

rem echo XCOPY d:\em\*.* f:\em /u/p/d
rem XCOPY d:\em\*.* f:\em /u/p/d
rem XCOPY d:\em\afm\*.* f:\em\afm /u/p/d
rem XCOPY d:\em\plain\*.* f:\em\plain /u/p/d
rem XCOPY d:\em\latex209\*.* f:\em\latex209 /u/p/d
rem XCOPY d:\em\latex2e\*.* f:\em\latex2e /u/p/d
rem XCOPY d:\em\psfonts\*.* f:\em\psfonts /u/p/d
rem XCOPY d:\em\test\*.* f:\em\test /u/p/d
rem XCOPY d:\em\tfm\*.* f:\em\tfm /u/p/d
rem XCOPY d:\em\tfm\texnansi\*.* f:\em\tfm\texnansi /u/p/d
rem XCOPY d:\em\tfm\tex256\*.* f:\em\tfm\tex256 /u/p/d
rem rem XCOPY d:\em\tex\*.* f:\em\tex /u/p/d

echo XCOPY c:\tex\*.* f:\em\tex /u/p/d
XCOPY c:\tex\*.* f:\em\tex /u/p/d
rem XCOPY d:\em\vec\*.* f:\em\vec /u/p/d
echo XCOPY c:\dvisourc\vec\*.* f:\em\vec /u/p/d
XCOPY c:\dvisourc\vec\*.* f:\em\vec /u/p/d
rem XCOPY d:\em\enc\*.* f:\em\enc /u/p/d
echo XCOPY c:\dvisourc\vec\*.* f:\em\enc /u/p/d
XCOPY c:\dvisourc\vec\*.* f:\em\enc /u/p/d
XCOPY c:\prog\*.* f:\em\prog /u/p/d

rem echo Have to rename information folder on f: so XCOPY works!
rem rename f:\information info
echo XCOPY c:\flyers\*.* f:\info /u/p/d
XCOPY c:\flyers\*.* f:\info /u/p/d
rem rename f:\info information

rem Update Y&Y TeX demo if needed
echo XCOPY d:\www\download\* f:\demo /u/p/d
XCOPY d:\www\download\* f:\demo /u/p/d

rem Update sample PDF files
echo XCOPY d:\www\download\*.* f:\samples /u/p/d
XCOPY d:\www\download\*.* f:\samples /u/p/d


rem Patch DVIPREAM.ENC
call c:\yyinstal\patch\patch


echo Not attempting to refresh other font file folders on f: yet...

if exist c:\yyinstal\setup.w02 goto diskette
if exist c:\yyinstal\setup.w03 goto diskette
if exist c:\yyinstal\setup1.w02 goto diskette
if exist c:\yyinstal\setup1.w03 goto diskette
goto end

:diskette
echo WARNING: The WISE installation may be set up for diskettes.
echo The installation file is split into three parts rather than being a single file. 
echo.
dir c:\yyinstal\setup.w0*
dir c:\yyinstal\setup1.w0*
echo.
echo A CD made from the current files will *NOT* work properly.
echo.
echo Make sure after runnning WISE that there is a setup.exe file of about 4Mb
echo.
echo If not, and you are making a system to be put on CD: then
echo run the WISE compilation again by selecting `Make Floppy' and
echo changing the max file size to 0 (rather than 1420 kbytes).
echo.
pause

goto end

rem pause
echo END of SYNCHRON.BAT
:end
cls
