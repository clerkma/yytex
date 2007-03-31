@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.
rem
rem Usage: smallcap font-file-name [x-scaling y-scaling [N]]
rem
rem The scaling applies to the process of making the small capitals
rem --- default is 0.9 horizontally and 0.85 vertically.
rem Optional fourth argument specifies that outline coordinates are not rounded
rem This yields larger font, slower rendering, but less reduction in accuracy.
rem
rem Output file is called smallcap.pfa
rem
if not "%1" == "" goto argok
rem
echo Please specify font PFA file name (without extension)
echo.
echo Usage: smallcap font-file-name [x-scaling y-scaling [N]]
echo.
goto end

:argok
if exist %1.pfa goto fileok
echo Sorry, can't find file %1.pfa
goto end

:fileok
rem set up inclusionary character list
echo A-Z > charcaps.txt
rem add accented characters
echo Aacute-Zcaron >> charcaps.txt
rem add special characters
echo AE-Thorn >> charcaps.txt
rem set up exclusionary character list
echo ~a-z > charbase.txt
rem add accented characters
echo ~aacute-zcaron >> charbase.txt
rem add special characters
echo ~ae-thorn >> charbase.txt
rem
rem pause
rem
echo Making subfont containing only upper case letters
echo.
rem subfont %1.pfa charcaps.txt
call subfont %1.pfa charcaps.txt
rem
rem pause
rem
echo Changing scale of letters in subfont
echo.
if "%2" == "" goto dostandard
rem
if "%3" == "" goto isotropic
rem
rem OK scaling in x and y WAS specified
rem
if "%4" == "" goto dorounda
rem
rem Do NOT want rounding of coordinates
rem
rem transfrm -l -x=%2 -y=%3 subfont.pfa
call transfrm -l -x=%2 -y=%3 subfont.pfa
goto scaledone

:dorounda
rem
rem Both scale factors given (with rounding)
rem
rem transfrm -a -l -x=%2 -y=%3 subfont.pfa
call transfrm -a -l -x=%2 -y=%3 subfont.pfa
goto scaledone

:isotropic
rem
rem Specified only one scale factor (with rounding)
rem
rem transfrm -l -x=0.9 -y=0.8 subfont.pfa
call transfrm -l -m=%2 subfont.pfa
goto scaledone

:dostandard
rem
rem Use `standard' scaling (with rounding)
rem
rem transfrm -a -l -x=0.9 -y=0.8 subfont.pfa
call transfrm -a -l -x=0.9 -y=0.8 subfont.pfa
:scaledone
rem
rem pause
rem
echo Changing character names from upper case to lower case
echo.
rem renamech -l transfrm.pfa
call renamech -l transfrm.pfa
rem
rem pause
rem
echo Making base font containing all but lower case letters
echo.
rem subfont %1.pfa charbase.txt
call subfont %1.pfa charbase.txt
rem
rem pause
rem
echo Merging base font and new lower case letters
echo.
rem mergepfa subfont.pfa transfrm.pfa
call mergepfa subfont.pfa transfrm.pfa
rem
rem pause
rem
if exist smallcap.pfa del smallcap.pfa
rename merged.pfa smallcap.pfa
rem
echo Output file is called smallcap.pfa
echo.
echo REMEMBER TO CHANGE THE POSTSCRIPT FONTNAME!
echo.
echo Deleting temporary files again
echo.
pause
rem deleting intermediate products now
rem
del subfont.pfa
del transfrm.pfa
rem del merged.pfa
rem
rem deleting helper files now
rem
del charcaps.txt
del charbase.txt
:end
@echo on
