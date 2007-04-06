@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem Long file name upgrade customization
echo Customizing files for long file name mini upgrade for Windows 95
echo.
vol a:
if exist a:install.bat goto maybeok
echo First make sure to have long file name diskette in drive a:
goto end

:maybeok
echo label a:longnames
label a:longnames
if not "%1" == "" goto argsupp
echo Must supply user serial number or (unique) name on command line
goto end

:argsupp
rem echo Making some space on the diskette
rem if exist a:plain.log del a:plain.log

rem echo Check that there is only one copy of tex.poo or tex.pool
rem dir a:tex*.po*
rem pause

echo Customizing 32 bit DVIPSONE.EXE on d:\dvisourc (DVISETNT %1)
call dvisetnt %1
rem echo copy d:\dvisourc\dvipsont.exe a:
echo copy d:\dvisourc\dvipsone.exe a:
copy d:\dvisourc\dvipsone.exe a:

rem Do we really need to compress dvipream.enc ?
echo Compressing d:\dvisourc\dvipream.enc
e:
rem cd \dvisourc
pfatopfb -v d:\dvisourc\dvipream.enc
rem del dvipream.enc
rem rename dvipream.pfb dvipream.enc
echo copy dvipream.pfb a:dvipream.enc
copy dvipream.pfb a:dvipream.enc /b
rem check customization d:\dvisourc\dvipsone -qqq
echo.

echo Customizing WIN16 DVIWINDO.EXE on c:\winsourc (WINSETUP %1)
call winsetup %1
echo copy c:\winsourc\dviwindo.exe a:
copy c:\winsourc\dviwindo.exe a:
echo.

rem echo Customizing YANDYNT.EXE (TeX 3.14159) on d:\texsourc (TEXSET32 %1)
rem call texset32 %1
rem echo copy d:\texsourc\yandynt.exe a:
rem copy d:\texsourc\yandynt.exe a:
rem echo copy d:\texsourc\fmt\tex.poo a:
rem copy d:\texsourc\fmt\tex.poo a:

echo Customizing YANDY12.EXE (TeX 3.141) on c:\texsourc (TEXSET12 %1)
rem call texsetnt %1
call texset12 %1
rem echo copy c:\texsourc\yandynt.exe a:\yandy12.exe /b
echo copy c:\texsourc\yandy12.exe a:\yandytex.exe /b
rem copy d:\texsourc\yandynt.exe a:\yandy12.exe /b
copy c:\texsourc\yandy12.exe a:\yandytex.exe /b

rem echo replace d:\texsourc\readme.txt a: /u/p
rem replace d:\texsourc\readme.txt a: /u/p
echo copy d:\texsourc\readlong.txt a:readme.txt
copy d:\texsourc\readlong.txt a:readme.txt

rem echo rename a:tex.pool tex.poo
rem rename a:tex.pool tex.poo
rem echo rename a:tex.poo tex.pool
rem rename a:tex.poo tex.pool
rem echo You want a file that has short name tex.poo and long name tex.pool
rem dir a:tex*.po*
rem pause
dir a:

:end
