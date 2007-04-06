@echo off
if "%2" == "" echo missing FontName
if "%1" == "" echo missing file name
echo (%2) > c:\ps\params.ps
if exist %1.pfa goto pfa
if exist c:\psfonts\%1.pfb goto pfb
echo can't find %1
goto end
:pfa
if "%3" == "M" goto matha
reencode -x -c=lucidall %1.pfa
:matha
copy %1.pfa+c:\ps\params.ps+c:\ps\chartable.ps lpt1:/b
goto end
:pfb
pfbtopfa c:\psfonts\%1.pfb
if "%3" == "M" goto mathb
reencode -x -c=lucidall %1.pfa
:mathb
copy %1.pfa+c:\ps\params.ps+c:\ps\chartable.ps lpt1:/b
del %1.pfa
goto end
:end
rem del c:\ps\params.ps
@echo on