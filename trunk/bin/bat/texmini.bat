@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem not needed if we just did TEXSETUP, but better safe than sorry...
rem call 32bitnew
rem call 32bit
rem
if not "%1" == "" goto argok
echo Assuming you meant texmini a:
texmini a:
echo IMPOSSIBLE ERROR!
goto end

:argok
if "%1" == "N" goto seemsok

if exist %1\yandytex.zip goto seemsok
echo Sorry, apparently not y&ytex diskette
if "%2" == "FORCE" goto seemsok
goto end

:seemsok
vol %1 | find "YANDYTEX"
if not errorlevel 1 goto goodlabel
echo WARNING: diskette does not have YANDYTEX label!
pause
label %1YANDYTEX12
rem goto end

:goodlabel

rem replace c:\yandytex\fmt\*.* %1\fmt /U/P

rem can't afford the space anymore ...
rem making some space
rem echo del %1\integrat.txt
rem if exist %1\integrat.txt del %1\integrat.txt
if exist %1\txt\cm-ansi.txt del %1\txt\cm-ansi.txt
rem if exist %1\txt\texnansi.txt del %1\txt\texnansi.txt
if exist %1\epsilon\tex.e del %1\epsilon\tex.e
rem 95/Sep/21
if exist %1\txt\envvars.txt del %1\txt\envvars.txt
rem 96/Jun/25
if exist %1\txt\faq-new.txt del %1\txt\faq-new.txt 

if not exist %1\txt\*.* mkdir %1\txt

rem we no longer rebindb and cfig - assume done already

rem copy only changable part of y&ytex to distribution diskette
rem 
c:
rem cd \yandytex
cd \texsourc
rem grab compile date of Y&YTeX
rem copydate c:\yandytex\date.txt c:\yandytex\yandytex.exe
rem copydate c:\texsourc\date.txt c:\texsourc\yandytex.exe

rem echo rebinding y&ytex
rem rebindb yandytex 
rem rebindb yandytex kernel32.dll user32.dll
rem rebindb yandytex kernel32.dll w32sem87.dll w32skrnl.dll
rem rebindb yandytex w32sem87.dll w32skrnl.dll -----  FOR WINDOWS NT
rem rebindb yandytex kernel32.dll user32.dll w32sem87.dll w32skrnl.dll
rem rebindb yandytex kernel32.dll advapi32.dll user32.dll emutnt.dll
rem rebindb yandytex kernel32.dll advapi32.dll user32.dll emutnt.dll
rem rebindb yandytex emutnt.dll
rem call rebindnt

rem need double % in batch file so get single percent in exe file ...
rem echo configuring in Y&YTeX environment variable
rem cfig386 yandytex.exe %%yandytex
rem cfig386 yandytex.exe %%yandytex
rem
rem copydate c:\texsourc\yandytex.exe c:\texsourc\date.txt
rem

if "%1"  == "N" goto end

echo switching to RAM drive for the moment
e:
rem if exist yandytex.zip del yandytex.zip
echo compressing yandytex
rem c:\pkzip\pkzip yandytex.zip c:\texsourc\yandytex.exe c:\texsourc\fmt\tex.poo
rem use batch file

rem Check whether executable has been customized
find /N "bkphbkph"  c:\texsourc\yandytex.exe > nul:
rem 0 found match (bad!), 1 no match found (good!), 2 error in FIND (ugly!)
if not errorlevel 2 goto findok
echo ERROR: in FIND for c:\texsourc\yandytex.exe
pause
goto end

:findok
if errorlevel 1 goto customized
echo ERROR: c:\texsourc\yandytex.exe IS NOT customized
pause
goto end

:customized
echo c:\yandytex\yandytex.exe IS customized
rem
call zip yandytex.zip c:\texsourc\yandytex.exe c:\texsourc\fmt\tex.poo
if not errorlevel = 1 goto zipok
echo SORRY PKZIP FAILED - probably no space on drive 
chdir
pause
goto end
:zipok

rem pkzip pharlap.zip cfig386.exe pharlap.386 tellme.exe 
rem cfig386.txt tellme.txt dosx.txt os2.txt win3.txt winnt.txt
rem
copydate yandytex.zip c:\texsourc\yandytex.exe
if not errorlevel = 1 goto copydateok
echo SORRY COPYDATE FAILED - probably missing ZIP file
pause
:copydateok
rem
echo Copying yandytex.zip to %1
rem dir %1
dir yandytex.zip
xcopy yandytex.zip %1
echo Finished copying yandytex.zip
rem
if exist yandytex.zip del yandytex.zip
rem
replace c:\texsourc\readme.txt %1 /U/P

rem renaming June 30th
rem if exist %1\install.txt del %1\install.txt
rem copy c:\texsourc\install.txt %1
replace c:\texsourc\install.txt %1 /u/p

replace c:\texsourc\*.txt %1 /u/p

rem if exist %1\packages.txt del %1\packages.txt
rem copy c:\txt\envvars.txt %1
rem replace c:\txt\envvars.txt %1 /u/p
rem replace c:\txt\envvars.txt %1\txt /u/p
rem replace c:\flyers\faq-new.txt %1 /u/p
rem replace c:\flyers\faq-new.txt %1\txt /u/p

rem following added 1995/August/1

rem replace c:\flyers\nttex.txt %1\txt /u/p
rem replace c:\flyers\ntfonts.txt %1\txt /u/p
rem replace c:\flyers\win95.txt %1\txt /u/p
rem replace c:\flyers\os2tex.txt %1\txt /u/p
replace c:\flyers\*.txt %1\txt /u/p
replace c:\txt\*.txt %1\txt /u/p

rem Copy the extra installation instructions
rem copy c:\texsourc\install.txt %1
rem replace c:\txt\install.txt %1 /U/P
rem replace c:\txt\envvars.txt %1 /U/P

rem if not exist %1\yandytex.pif copy c:\windows\yandytex.pif %1
rem copy c:\windows\yandytex.pif %1
replace c:\windows\*.pif %1 /U/P
rem
rem SPACE problem ?
replace c:\epsilon\eel\*.e %1 /U/P
rem
if exist c:\texsourc\news.txt replace c:\texsourc\news.txt %1 /U/P
rem
replace c:\texsourc\*.txt %1 /U/P
replace c:\texsourc\*.txt %1\txt /U/P
rem
replace c:\texsourc\install.bat %1 /U/P
rem
rem if exist c:\texsourc\xchr.map copy c:\texsourc\xchr.map %1

rem SPACE problem ?
if exist c:\texsourc\*.map replace c:\texsourc\*.map %1 /U/P

rem if exist c:\texsourc\texfonts.map copy c:\texsourc\texfonts.map %1
rem if exist c:\texsourc\tfm\texfonts.map copy c:\texsourc\tfm\texfonts.map %1

replace c:\texsourc\tfm\*.map %1 /U/P
rem
rem replace c:\texsourc\tfm\*.tfm %1\tfm /U/P
rem
replace c:\prog\translat.exe %1 /U/P
rem
replace c:\txt\translat.txt %1 /U/P
rem
rem replace c:\txt\*.txt %1 /U/P
rem
if exist %1*.bak del %1*.bak
if exist %1*.sav del %1*.sav
rem
rem if exist %1\xchr.chr del %1\xchr.chr
rem if exist %1\xchr.map del %1\xchr.map
rem
rem if exist %1\dostoans.chr del %1\dostoans.chr
rem if exist %1\dostoans.map del %1\dostoans.map
rem 
rem replace c:\texsourc\dostoans.chr %1 /A/P
rem replace c:\texsourc\dostoans.map %1 /A/P

echo replace c:\txt\keyboard.txt c:\texsourc\keyboard /U/P
replace c:\txt\keyboard.txt c:\texsourc\keyboard /U/P

rem copy c:\texsourc\dos*.map %1
rem replace c:\texsourc\dos*.map %1 /U/P
replace c:\texsourc\keyboard\dos*.map %1\keyboard /U/P
rem copy c:\texsourc\dos*.key %1
rem replace c:\texsourc\dos*.key %1 /U/P
echo replace c:\texsourc\keyboard\dos*.key %1\keyboard /U/P
replace c:\texsourc\keyboard\dos*.key %1\keyboard /U/P

rem for dos1252.key file
rem echo replace c:\texsourc\keyboard\dos*.key %1\keyboard /A/P
replace c:\texsourc\keyboard\dos*.key %1\keyboard /A/P

rem echo Trying to copy keyboard.txt to diskette
rem copy c:\texsourc\keyboard.txt %1
rem replace c:\texsourc\keyboard.txt %1 /U/P
rem echo replace c:\texsourc\keyboard\keyboard.txt %1\keyboard /U/P
replace c:\texsourc\keyboard\keyboard.txt %1\keyboard /U/P

rem echo replace c:\texsourc\keyboard\*.* %1\keyboard /U/P
replace c:\texsourc\keyboard\*.* %1\keyboard /U/P

rem echo replace c:\texsourc\epsilon\*.* %1\epsilon /U/P
replace c:\texsourc\epsilon\*.* %1\epsilon /U/P

rem replace c:\txt\envvars.txt %1 /u/p
rem echo replace c:\txt\envvars.txt %1\txt /u/p
replace c:\txt\envvars.txt %1\txt /u/p
rem
rem echo replace c:\txt\*.txt %1\txt /U/P
replace c:\txt\*.txt %1\txt /U/P
rem

rem copy the releas12.txt file onto the diskette
rem echo copy c:\txt\releas12.txt %1\txt
if not exist %1\txt\releas12.txt copy c:\txt\releas12.txt %1\txt
rem echo copy c:\flyers\faq-new.txt %1\txt
rem if not exist %1\txt\faq-new.txt copy c:\flyers\faq-new.txt %1\txt
rem echo copy c:\flyers\acrobat.txt %1\txt
rem if not exist %1\txt\acrobat.txt copy c:\flyers\acrobat.txt %1\txt
rem needed space - acrobat.txt now on dvipsone diskette

rem Added 1995/August/1

rem echo copy c:\flyers\nttex.txt %1\txt
if not exist %1\txt\nttex.txt copy c:\flyers\nttex.txt %1\txt
rem echo copy c:\flyers\ntfonts.txt %1\txt
if not exist %1\txt\ntfonts.txt copy c:\flyers\ntfonts.txt %1\txt
rem echo copy c:\flyers\win95.txt %1\txt
if not exist %1\txt\win95.txt copy c:\flyers\win95.txt %1\txt
rem echo copy c:\flyers\os2tex.txt %1\txt
if not exist %1\txt\os2tex.txt copy c:\flyers\os2tex.txt %1\txt

rem echo replace c:\epsilon\eel\*.e %1 /U/P
replace c:\epsilon\eel\*.e %1 /U/P
rem
rem replace d:\latex2e\yandytex.l2e %1 /U/P
rem replace d:\latex2e\yandytex.txt %1 /U/P
rem DEPENDS ON DRIVE FOR LATEX2E ?
rem replace c:\latex2e\yandytex.txt %1 /U/P
rem echo replace c:\latex2e\yandytex.txt %1 /U/P
if exist c:\latex2e\nul replace c:\latex2e\yandytex.txt %1 /U/P
rem echo replace d:\latex2e\yandytex.txt %1 /U/P
if exist d:\latex2e\nul replace d:\latex2e\yandytex.txt %1 /U/P
rem replace d:\latex2e\yandytex.txt %1 /U/P
rem copy d:\latex2e\yandytex.txt %1

c:
cd \texsourc

rem copydate -x keyboard.zip dos437.key dos850.key dos437wn.map dos850wn.map keyboard.txt c:\tex\meta_437.tex c:\tex\meta_850.tex
rem if not errorlevel = 1 goto skipkey
rem call zip keyboard.zip dos437.key dos850.key dos437wn.map dos850wn.map keyboard.txt c:\tex\meta_437.tex c:\tex\meta_850.tex

:skipkey
rem replace keyboard.zip %1 /U/P

if not exist %1\*.bak goto noback
echo BAK file found!
pause 
if exist %1\*.bak del *.bak
:noback

if exist %1\y&ytex.pif rename %1\y&ytex.pif yandytex.pif 

rem echo replace c:\prog\*.exe %1 /u/p
replace c:\prog\*.exe %1 /u/p
if not exist %1\setupttf.exe copy c:\prog\setupttf.exe %1

dir %1 /W
echo Please verify that there is less than 100 k byte left on diskette.
echo Otherwise a file may have gotten deleted because of lack of space!
echo (NOTE: as of 1996 Aug 30 there may be 16k-17k or so free)
echo.
rem check customization
c:\texsourc\yandytex -qqq

:end
