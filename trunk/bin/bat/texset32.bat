@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo Customize the new Y&Y TeX version 1.3
pause
c:
cd \texsourc

if not exist yandydos.exe goto namedos
echo Sorry, file yandydos.exe already exists!
pause
goto end

:namedos
echo rename yandytex.exe yandydos.exe
rename yandytex.exe yandydos.exe

if not exist yandyntx.exe goto nament
echo Sorry, file yandyntx.exe already exists!
pause
goto end

:nament
echo rename yandynt.exe yandyntx.exe
rename yandynt.exe yandyntx.exe

echo copy d:\texsourc\yandynt.exe c:\texsourc
xcopy d:\texsourc\yandynt.exe c:\texsourc

echo rename yandynt.exe yandytex.exe
rename yandynt.exe yandytex.exe

echo call texsetup %1 %2 %3 %4 %5 %6 %7 %8 %9
call texsetup %1 %2 %3 %4 %5 %6 %7 %8 %9

echo rename yandytex.exe yandynt.exe
rename yandytex.exe yandynt.exe 

echo copy yandynt.exe d:\texsourc
xcopy yandynt.exe d:\texsourc

echo del yandynt.exe
del yandynt.exe

echo rename yandyntx.exe yandynt.exe
rename yandyntx.exe yandynt.exe

echo rename yandydos.exe yandytex.exe 
rename yandydos.exe yandytex.exe 

:end
