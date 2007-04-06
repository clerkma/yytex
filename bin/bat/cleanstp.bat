rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem Helper function for lucidump.bat and luciunix.bat and Mac version
rem
rem This version drops result in current directory
rem
rem Removes comment lines, 
rem Changes encoding if needed --- default StandardEncoding ---
rem NONE means use encoding in AFM file
rem
rem cleanstp [AFM-file-name] [reencoding]
rem
rem echo Entering cleanstp!
rem
if not "%1" == "" goto godoit
echo Missing Arguments
echo Example: cleanstp lbr 	
echo          cleanstp lbma NONE
goto end
:godoit
if "%1" == "" echo MISSING FILE NAME
rem if NONE specified then don't reencode
if "%2" == "NONE" goto noencode
rem if encoding not specified use standard encoding
if "%2" == "" goto standard
rem
rem Using specified encoding vector
rem
reencode -x -c=%2 c:\lucidabr\%1.afm
goto skipstan
:standard
rem
rem Using StandardEncoding
rem
reencode -x -c=standard c:\lucidabr\%1.afm
:skipstan
goto nonmath
:noencode
rem
rem Do not reencode
rem
rem copy AFM file to current directory
rem
copy c:\lucidabr\%1.afm .
:nonmath
copydate -v %1.afm c:\lucidabr\%1.afm
stripcom -vrlicf %1.afm
rem del %1.afm
rem if exist %1.bak del %1.bak
:end
