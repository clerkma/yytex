@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%1" == "" goto argok
echo Assuming you meant %0 a:
%0 a:
echo IMPOSSIBLE ERROR!
goto end

:argok
rem if exist %1\prog\dvipsone.exe goto seemsok
if exist %1\dvipsone.exe goto seemsok
echo Sorry, apparently not DVIPSONE diskette
if "%2" == "FORCE" goto seemsok
goto end

:seemsok
rem make sure on top level dir on diskette
chdir %1\
vol %1 | find "DVIPSONE"
if not errorlevel 1 goto goodlabel
echo WARNING: diskette does not have DVIPSONE label!
pause
label %1DVIPSONE12
rem goto end

:goodlabel
rem echo making some space
rem if exist %1\pfb\manfont.ps del %1\pfb\manfont.ps
rem if exist %1\sub\arborcom.sub del %1\sub\arborcom.sub
rem if exist %1\sub\mtmi.sub del %1\sub\mtmi.sub
if exist %1\vec\cyrillic.vec del %1\vec\cyrillic.vec
if exist %1\vec\fontogra.vec del %1\vec\fontogra.vec
if exist %1\vec\dingbats.vec del %1\vec\dingbats.vec
if exist %1\vec\numeric.vec del %1\vec\numeric.vec
if exist %1\vec\8r.vec del %1\vec\8r.vec
if exist %1\vec\isolati2.vec del %1\vec\isolati2.vec
if exist %1\sub\arborres.sub del %1\sub\arborres.sub
if exist %1\sub\ansiacce.sub del %1\sub\ansiacce.sub
rem
rem copy only changable part of DVIPSONE to distribution diskette
rem 
rem if "%1" == "" goto noarg

rem Check whether exeutable has been customized
find /N "bkphbkph"  c:\dvisourc\dvipsone.exe > nul:
rem 0 found match (bad!), 1 no match found (good!), 2 error in FIND (ugly!)
if not errorlevel 2 goto findok
echo ERROR: in FIND for c:\dvisourc\dvipsone.exe
pause
goto end

:findok
if errorlevel 1 goto customized
echo ERROR: c:\dvisourc\dvipsone.exe IS NOT customized
pause
goto end

:customized
echo c:\dvisourc\dvipsone.exe IS customized
rem
echo copying c:\dvisourc\dvipsone.exe
copy c:\dvisourc\dvipsone.exe %1
rem copy c:\dvisourc\dvipsone.exe %1\prog
rem
echo copy c:\dvisourc\dvipream.enc
copy c:\dvisourc\dvipream.enc %1
echo copy c:\dvisourc\dvifont3.enc
copy c:\dvisourc\dvifont3.enc %1
echo copy c:\dvisourc\dvitpics.enc
copy c:\dvisourc\dvitpics.enc %1
rem
rem echo Copying c:\utility\cdd.com
rem copy c:\utility\cdd.com %1
rem if not exist %1\cdd.com copy c:\utility\cdd.com %1
replace c:\prog\cdd.com %1 /U/P
rem
rem if not exist %1\dvipsone.pif copy c:\windows\dvipsone.pif %1
rem copy c:\windows\dvipsone.pif %1
replace c:\windows\dvipsone.pif %1 /U/P
rem
if exist %1:\ps\lprep*.* del %1:\ps\lprep*.*
rem
replace c:\dvisourc\install.bat %1 /U/P
rem
rem replace c:\prog\*.exe %1 /U/P
replace c:\prog\*.exe %1\prog /U/P
rem
rem replace c:\tex\*.tex %1 /U/P
replace c:\tex\*.tex %1\tex /U/P

rem replace c:\txt\*.txt %1 /U/P
replace c:\txt\*.txt %1\txt /U/P

rem replace c:\dvipsone\*.txt %1 /U/P
replace c:\dvisourc\readme.txt %1 /U/P
replace c:\dvisourc\news.txt %1 /U/P
replace c:\dvisourc\*.txt %1\txt /U/P

rem if not exist %1\txt\masters.txt copy c:\txt\masters.txt %1\txt

replace c:\flyers\masters.txt %1\txt /U/P

rem replace c:\dvisourc\*.vec %1\vec /U/P
replace c:\dvisourc\vec\*.vec %1\vec /U/P

rem replace c:\dvisourc\*.sub %1\sub /U/P
replace c:\dvisourc\sub\*.sub %1\sub /U/P

rem echo copying c:\dvisourc\berry.sub
rem copy c:\dvisourc\berry.sub %1\sub

replace c:\dvisourc\*.bat %1\sub /U/P

replace c:\bat\encode.bat %1 /U/P

replace c:\dvisourc\*.* %1 /U/P

rem copy c:\tex\rotatefx.sty %1\tex
rem replace c:\tex\rotatefx.sty %1\tex/U/P
replace c:\tex\*.sty %1\tex/U/P

echo Refreshing PFB, PFM, TFM directories
replace c:\dvisourc\afm\*.* %1\afm /U/P
replace c:\dvisourc\pfb\*.* %1\pfb /U/P
replace c:\dvisourc\tfm\*.* %1\tfm /U/P

if not exist %1\*.bak goto noback
echo BAK file found!
pause
if exist %1\*.bak del *.bak
:noback

dir %1 /W
echo Please verify that there is less than 100 k byte left on diskette.
echo Otherwise a file may have gotten deleted because of lack of space!
echo.
c:\dvisourc\dvipsone -qqq
rem You can also check the customization by typing %1\dvipsone -qqq

goto end

rem :noarg
rem echo copying c:\dvisourc\dvipsone.exe
rem copy c:\dvisourc\dvipsone.exe %1
rem echo copy c:\dvisourc\dvipream.enc
rem copy c:\dvisourc\dvipream.enc %1
rem echo copy c:\dvisourc\dvifont3.enc
rem copy c:\dvisourc\dvifont3.enc %1
rem echo copy c:\dvisourc\dvitpics.enc
rem copy c:\dvisourc\dvitpics.enc %1

:end
