@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

cd
echo %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 
rem
rem make PFB file from PLN file (or DEC file) - second arg time capsule file
rem
rem third arg is C to keep comments in encrypted part
rem 
rem Example: makeenco lbr c:\txt\timelcd.txt
rem
if not "%1" == "" goto dothen

:showuse
echo Missing arguments
echo Example: makeenco lbr
echo Example: makeenco lbr c:\txt\timelcd.txt
echo Example: makeenco lbr c:\txt\timelcd.txt C
echo	first argument is name of PLN format file
echo	second argument (optional) is time capsule file
echo	third argument (optional) asks to strip comments in encrypted part
goto end

:dothen
if "%2" == "" goto miss
if exist %2 goto capsok
if exist %2.txt %0 %1 %2.txt
if exist c:\txt\%2 %0 %1 c:\txt\%2
if exist c:\txt\%2.txt %0 %1 c:\txt\%2.txt
echo Can't find time capsule %2
goto end

:capsok
echo Using %2 as time capsule
if "%3" == "" echo WILL PRESERVE COMMENTS IN ENCRYPTED SECTION
if not exist %1.pln goto dodec
rem c:\prog\encrypt -vcrd -t %2 %1.pln
if "%3" == "" c:\prog\encrypt -vcrd -n -t %2 %1.pln
if not "%3" == "" c:\prog\encrypt -vcrd -t %2 %1.pln
rem c:\prog\encrypt -vcr %1.pln
rem set up now for TeX CM fonts
goto dodec

:miss
%0 %1 c:\txt\timedef.txt
pause
rem echo Using default time capsule c:\txt\timedef.txt
rem c:\prog\encrypt -vcrd -t c:\txt\timedef.txt %1.pln

:dodec
if not exist %1.dec goto dopfb
rem c:\prog\encrypt -ve %1.dec
rem c:\prog\encrypt -ve -s YANY %1.dec
if "%3" == "" c:\prog\encrypt -ve -n -s YANY %1.dec
if not "%3" == "" c:\prog\encrypt -ve -s YANY %1.dec

:dopfb
rem if not exist %1.pfa goto end
rem c:\prog\pfatopfb -v %1.pfa

:end
