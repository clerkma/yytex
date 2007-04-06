@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem This purges ATM's cache of font metric information.
rem 
rem This is required if you overwrite existing PFM or PFB font files,
rem rather than using ATM to `remove' and `install' fonts
rem (which is a good idea, because otherwise WIN.INI has duplicate entries).
rem 
rem You may need to adjust the file path:
rem To find out where ATM keeps the file, look in the [Settings]
rem section of ATM.INI (in your Windows directory);
rem there should be a line like: QLCDir=c:\psfonts
if not exist c:\win486\atmfonts.qlc goto absent
del c:\win486\atmfonts.qlc
goto end
:absent
echo ATMFONTS.QLC not found.  Either the file has already been deleted, 
echo or you are inside Windows.  Do not use RESETATM from inside Windows.
:end
@echo on
