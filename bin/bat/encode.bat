@echo off
rem Copyright (C) 1993-1996 Y&Y, Inc.
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem Magic batch file for `hard reencoding' fonts.
rem If you have Y and Y TeX you can in most cases use `on the fly' reencoding
rem of text fonts instead by setting the ENCODING environment variable...
rem
rem Magic batch file to simultaneously change encoding of PFB, PFM, and TFM
rem See `encoding.txt' for explanation (also read `morass.txt' please)
rem NOTE: This NO LONGER overwrites PFB and PFM in c:\psfonts 
rem This NO LONGER overwrites ATM.INI in c:\windows nor delete ATMFONTS.QLC
rem
rem USE ATM afterwards to install the fonts from the current directory
rem Can be used to work on up to four font files in one fell swoop!

rem encode [vec] [afm-dir] [pfb-dir] [tfm-dir] [win-dir] [W or N] [font-1] ...
rem   %0     %1     %2        %3        %4        %5        %6      %7

rem NOTE: Make sure the following utilities are on the DOS PATH or current dir
rem	  reencode.exe, afmtotfm.exe, afmtopfm.exe, decode.exe, safeseac.exe

rem NOTE: Make sure the encoding vector file you call for is (i) either in 
rem	  the current directory or (ii) where the environment variable 
rem	  VECPATH points, or (iii) give full path for vector file.

if "%1" == "" goto usage
if "%1" == "?" goto usage
if "%1" == "-?" goto usage

rem Check whether we have enough arguments!  Must have at least one font!

if not "%7" == "" goto lastok
echo Sorry, too few arguments (need to specify at least one font)
goto usage

:lastok
rem Check whether sixth argument is valid

rem and check whether we are in a recursive call - omit file checks if so
rem (recursive calls are those with sixth arg WX and NX instead of W or N)
if "%6" == "W" goto newstart
if "%6" == "N" goto newstart

if "%6" == "WX" goto reenter
if "%6" == "NX" goto reenter
echo ERROR: Please use either N or W for sixth argument, not %6
pause
goto usage

:newstart
rem Get here when first called.
rem A lot of following is redundant after the first recursive call
rem and hence omitted when called recursively...

rem We no longer check whether we are running in Windows 
rem Since we no longer overwrite PFB and PFM files and ATM.INI

goto dos

rem Check first whether running in Windows
rem if "%windir%" == "" goto dos - HA HA - wish it was that easy!
rem the following works fine, except on those darn DOS clones...
rem set | find "windir=" > NUL
rem if errorlevel 1 goto dos
rem Check whether running in Windows --- exploit one alternate use of DECODE
decode -vx
if not errorlevel = 1 goto dos

rem Cannot operate in Windows and safely write PFB and PFM files!
if "%6" == "N" goto dos
if "%6" == "NX" goto dos
echo ERROR: Please exit Windows before using ENCODE batch file with W flag
pause
goto abort

rem If N specified on command line, it is
rem OK to go on since we *won't* be writing the results back to PFB directory
:dos
echo 
rem Its easier to clean up later if current directory is empty
if not exist *.* goto isclean
rem On the other hand user may already have done one run with N instead of W
if exist atm.ini goto isclean
echo WARNING: current directory not empty (it is easier to clean up later if its empty).
echo Type Control-C if you wish to quit. Type Enter if you wish to continue.
pause

:isclean
rem Some quick sanity checks to warn about missing directories

if not exist %2\nul echo WARNING: AFM directory %2 does not exist
if not exist %3\nul echo WARNING: PFB directory %3 does not exist
rem Do not test for TFM directory if user does not want TFM file
if "%4" == "N" goto skipmoan
if not exist %4\nul echo WARNING: TFM directory %4 does not exist

:skipmoan
if not exist %5\nul echo WARNING: Windows directory %5 does not exist

rem Before we get too excited, we check whether things we'll need exist!
rem Checking whether specified encoding vector  %1.vec  can be found
rem could be fully qualified, in current directory, or in VECPATH directory
if exist %1 goto vectorok
if exist %1.vec goto vectorok
if "%VECPATH%" == "" goto novec
if exist %VECPATH%\%1 goto vectorok
if exist %VECPATH%\%1.vec goto vectorok

:novec
echo ERROR: Cannot find encoding vector  %1.vec  in current directory or %VECPATH%
echo Please (i) set VECPATH to point to directory with encoding vector files,
echo or (ii) copy the file to current directory, or (iii) specify full path.
rem pause
goto abort

:vectorok
if exist %5\ATM.INI goto atmok
echo WARNING: Cannot find ATM.INI in %5
rem pause
rem we no longer case since we do not modify ATM.INI anymore
rem goto abort

:atmok
if "%1" == "ansinew" goto anscomm
if "%1" == "ansi" goto anscomm

rem NOT Windows ANSI
rem Now we know its *not* ANSI encoding - time to unset TEXANSI
if "%PSFONTS%" == "" goto nodvipsx
if not "%TEXANSI%" == "" echo IMPORTANT: Remove the line set TEXANSI=1 from autoexec.bat
echo With Y and Y TeX system: remove any env var line TEXANSI=1 from dviwindo.ini
echo With Y and Y TeX system: add env var line ENCODING=%1 to dviwindo.ini
rem unset it for this session at least...
if not "%TEXANSI%" == "" set TEXANSI=
pause

:nodvipsx
goto noants

:anscomm
rem Windows ANSI encoding
echo For Windows ANSI encoding, its best to use the original PFB and PFM files
echo (if you are dealing with plain vanilla PostScript Type 1 text fonts).
echo ÿ
if "%PSFONTS%" == "" goto nodvipsone
rem echo IMPORTANT: Set TEXANSI=1 for DVIPSONE to force reencoding of text fonts to ANSI
rem echo IMPORTANT: Use `-X' with DVIPSONE to force reencoding of text fonts to ANSI
rem
echo With Y and Y TeX system: remove any env var ENCODING=... from diviwindo.ini
echo With Y and Y TeX system: set env var TEXANSI=1 in diviwindo.ini
rem set it for this session at least...
if "%TEXANSI%" == "" set TEXANSI=1
pause
:nodvipsone
:noants

rem Now we think most things that we need can be found --- Normal case

rem See whether ATM.INI needs to be copied to current directory
echo Copying ATM.INI to current directory

rem if "%6" == "W" goto nocopy ***************************************
rem if "%6" == "WX" goto nocopy **************************************

rem If we are not writing ATM.INI back to Windows directory, then:
rem first time around must get rid of ATM.INI file in current directory
if not exist atm.ini goto skipdel
echo WARNING: ATM.INI found in current directory - renaming ATM.BAK
if exist atm.bak del atm.bak
rename atm.ini atm.bak

:skipdel
if not exist %5\atm.ini goto nocopy
echo Copying ATM.INI to current directory from %5
copy %5\atm.ini .
rem dir atm.ini

:nocopy
if "%6" == "W" goto writbck
if "%6" == "WX" goto writbck
rem if %6 is N or NX then ...
echo NOTE: Modified files will be deposited in current directory
echo NOTE: Either repeat run with W flag or install from this directory
goto skipcurr

:writbck
echo NOTE: Modified files will overwrite originals at end.
echo NOTE: BUT, install fonts using ATM from current directory at end.

:skipcurr
rem Come here directly during recursive call --- main part of work here

:reenter
if exist %7.pfb goto current
if exist %7_*.pfb goto current
goto notcurrent

:current
rem This warning may be spurious, if old PFB files happen to be here...
echo 
echo WARNING: the source PFB files should NOT be in the current directory
echo Type Ctrl-C if you want to exit
pause

:notcurrent
rem Now do some font specific sanity checks
rem Check whether PFB file can be found
if exist %3\%7.pfb goto pfbok
if exist %3\%7_*.pfb goto pfbok
echo ERROR: Cannot find Printer Font Binary (PFB) file %7.pfb in %3
goto skipthis

:pfbok
rem see whether AFM file can be found
if exist %2\%7.afm goto afmok
if exist %2\%7_*.afm goto afmok
echo ERROR: Cannot find Adobe Font Metric (AFM) file %7.afm in %2
goto skipthis

:afmok
rem see whether TFM file can be found
if "%4" == "N" goto tfmok
if exist %4\%7.tfm goto tfmok
if exist %4\%7_*.tfm goto tfmok
echo WARNING: Cannot find TeX Font Metric (TFM) file %7.tfm in %4
rem We could go on here, assuming that this means there is no TFM file yet
rem goto abort

:tfmok
rem see whether PFM file can be found in PFM subdirectory or in PFB directory
if exist %3\pfm\%7.pfm goto pfmok
if exist %3\pfm\%7_*.pfm goto pfmok
if exist %3\%7.pfm goto pfmok
if exist %3\%7_*.pfm goto pfmok
echo WARNING: Cannot find Printer Font Metric file %7.pfm in %3\PFM or %3
rem Perhaps not a critical error, since we can make up a Windows Face Name
rem goto abort

:pfmok
rem Now check whether AFM and PFB both have underscores, or both do not...

if exist %2\%7.afm goto nounder
if exist %2\%7_*.afm goto under
echo ERROR: can't find AFM file for %7 in %2 (boy, am I confused!)
goto skipthis

:under
if exist %3\%7.pfb goto mismatch
if exist %3\%7_*.pfb goto undermatch
echo ERROR: can't find PFB file for %7 in %3 (boy, am I confused!)
goto skipthis

:nounder
if exist %3\%7_*.pfb goto mismatch
if exist %3\%7.pfb goto undermatch
echo ERROR: can't find PFB file for %7 in %3 (boy, am I confused!)
goto skipthis

:mismatch
echo ERROR: some of the AFM, PFB, PFM file names appear to have underscores and some don't
echo        --- or the font file names have a full eight characters.
echo        The file names of AFM, PFB and PFM files must match.
echo        Do *not* use underscores in file names on command line to encode.bat.
pause
rem goto skipthis

:undermatch
rem Finally, let's get serious!

echo Now trying to reencode font %7 using encoding vector %1
echo -----------------------------------------------------------------------
echo Start by reencoding %7.pfb  (Printer Font Binary) file

rem If encoding is  ansinew, one can leave out  -c=ansinew
rem to get ASE encoding in the PFB file, but then DVIPSONE will use ASE
rem instead, *unless* command line flag -X is used
if "%1" == "ansinew" goto backstan
if "%1" == "ansi" goto backstan
goto notansi

:backstan
rem `reinstate' Adobe StandardEncoding in PFB file
rem For Windows ANSI encoding, its best to use the original PFB and PFM files
rem IMPORTANT: Use `-X flag with DVIPSONE to force reencoding to ANSI
echo Reinserting original StandardEncoding (so Windows will use ANSI!)
echo ÿ
echo IMPORTANT: Set TEXANSI=1 for DVIPSONE to force reencoding of text fonts to ANSI
echo IMPORTANT: Do not set ENCODING=ansinew in env vars in dviwindo.ini
reencode -vx %3\%7.pfb
if errorlevel = 1 goto abort
goto  afterencode

:notansi
rem Try and insert remapping (t) -- reencode will suppress if it makes no sense
rem We rely on REENCODE to extend font name with _ if needed ...

reencode -vxt -c=%1 %3\%7.pfb
if errorlevel = 1 goto abort

:afterencode
rem We need to immunize the PFB file against ATM unless special encoding used
rem But only REALLY needed if accented characters will be used.

rem First kick out encoding vectors that are known to be safe
rem TeX text encoding has no accented characters
if "%1" == "textext" goto immune
if "%1" == "texital" goto immune
if "%1" == "typewrit" goto immune
if "%1" == "typeital" goto immune
rem Old TeXtures encoding schemes also have no accented characters
if "%1" == "textures" goto immune
if "%1" == "texutype" goto immune
rem Adobe StandardEncoding does not have accented characters
if "%1" == "standard" goto immune
if "%1" == "symbol" goto immune
if "%1" == "dingbats" goto immune
rem Windows ANSI is compatible with ATM hard-wired accents
if "%1" == "ansinew" goto immune
if "%1" == "ansi" goto immune
rem `texannew' encoding has been specially designed to work with ATM
rem Not anymore! Since ATM 2.6 has changed the rules...
rem if "%1" == "texannew" goto immune
rem if "%1" == "texnansi" goto immune

rem We will rely on PFBtoPFA, SAFESEAC and PFAtoPFA to extend with _ if needed

echo .......................................................................
echo Now making PFB file immune from bugs in ATM for Windows
rem  SAFESEAC will reject if file already immunized ...
rem  and return errorlevel = 1 if asked using the  e  command line flag

rem Note: PFB is now in current directory, where REENCODE dropped it

if exist %7.pfb goto dopfb
if exist %7_*.pfb goto dopfb
echo Impossible error: no %7.pfb or %7_*.pfb file found after REENCODE!
pause
goto immune

:dopfb
echo Converting %7.pfb back to %7.pfa
rem pfbtopfa -v %7.pfb
pfbtopfa %7.pfb
if errorlevel = 1 goto abort
rem call safeseac -v %7.pfa
rem safeseac -v %7.pfa
safeseac -e %7.pfa
rem if errorlevel = 1 goto abort
if errorlevel = 1 goto delpfaa
echo Converting %7.pfa back to %7.pfb
rem pfatopfb -v %7.pfa
pfatopfb %7.pfa
if errorlevel = 1 goto abort

:delpfaa
echo Deleting %7.pfa again
del %7.pfa

:immune
echo ------------------------------------------------------------------------
echo Making new  %7.pfm  (Printer Font Metric) file now

rem Try and insert remapping (t) --- afmtopfm will suppress if makes no sense
rem Try symbol and decorative (sd) --- afmtopfm will suppress if makes no sense

rem Following tries to use old PFM file to get Windows Face Name if possible

rem Try and add  X  to end of Windows Face Name (do avoid PSCRIPT.DRV bug)
echo (also extending Windows Face Name with an X in PFM file)

rem Relying on AFMtoPFM to extend name with underscores, use PFM subdir etc

if exist %3\pfm\%7.pfm goto pfmexist
if exist %3\pfm\%7_*.pfm goto pfmexist
if exist %3\%7.pfm goto pfmexist
if exist %3\%7_*.pfm goto pfmexist
echo WARNING: Old PFM file %7.PFM not found
rem afmtopfm -vxsdtX -c=%1 %2\%7.afm
afmtopfm -xsdtX -c=%1 %2\%7.afm
if errorlevel = 1 goto abort
goto endpfma

:pfmexist
rem afmtopfm -vxsdtX -c=%1 -R=%3\%7 %2\%7.afm
afmtopfm -xsdtX -c=%1 -R=%3\%7 %2\%7.afm
if errorlevel = 1 goto abort

:endpfma
if "%4" == "N" goto tfmdone
echo --------------------------------------------------------------------------
echo Making new  %7.tfm  (TeX Font Metric) file now

rem Try for all ligatures --- afmtotfm will suppress if fixed pitch font

rem afmtotfm -vxadj -c=%1 %2\%7.afm
afmtotfm -xadj -c=%1 %2\%7.afm
if errorlevel = 1 goto abort

:tfmdone
rem we no longer fiddle with the ATM.INI file --- too dangerous
goto noxtend

rem Need to modify ATM.INI to reflect new Windows Face Name

echo ------------------------------------------------------------------------
echo Now extending Windows Face Name with an X in ATM.INI
rem exploit another alternate use of DECODE utility...
rem if W not specified, work in current directory

rem if "%6" == "W" decode -v -c=%1 -w=%5 %7 *********************************
if "%6" == "W" decode -v -c=%1 -w=atm.ini %7
rem if "%6" == "WX" decode -v -c=%1 -w=%5 %7 ********************************
if "%6" == "W" decode -v -c=%1 -w=atm.ini %7

if "%6" == "N" decode -v -c=%1 -w=atm.ini %7
if "%6" == "NX" decode -v -c=%1 -w=atm.ini %7

:noxtend

rem If W was not specified, we don't write the new files back
rem And no we only write back the TFM file in any case ...
if "%6" == "N" goto nowrite
if "%6" == "NX" goto nowrite

rem Copy back files only at end, after everything has been checked

echo ------------------------------------------------------------------------
echo 
rem echo WARNING: will now overwrite existing PFB, PFM and TFM files for %7
echo WARNING: will now overwrite existing TFM files for %7
rem echo Do not proceed unless you have backup copies of PFB, PFM, and TFM files.
echo	(press control-C to interrupt)
pause
goto misspfba

if not exist %7.pfb goto misspfba
echo Copying reencoded %7.pfb back to %3
copy %7.pfb %3

:misspfba
goto misspfbx

if not exist %7_*.pfb goto misspfbx
echo Copying reencoded %7_*.pfb back to %3
copy %7_*.pfb %3

:misspfbx
if "%4" == "N" goto skiptfm

if not exist %7.tfm goto misstfma
echo Copying reencoded %7.tfm back to %4
copy %7.tfm %4
goto skiptfm

:misstfma
rem This shouldn't really happen!
if not exist %7_*.tfm goto misstfmb
echo Copying reencoded %7_*.tfm back to %4
copy %7_*.tfm %4

:misstfmb

:skiptfm
rem we don't mess with PFM anymore
goto skippfm

rem Check whether a PFM sub-directory exists

if not exist %3\pfm\nul goto putinpfb

echo Putting PFM file in PFM subdirectory

if not exist %7.pfm goto misspfma
echo Copying reencoded %7.pfm back to %3\pfm
copy %7.pfm %3\pfm

:misspfma
if not exist %7_*.pfm goto misspfmb
rem echo Copying reencoded %7_*.pfm back to %3\pfm ***************************
echo NOT copying reencoded %7_*.pfm back to %3\pfm
rem copy %7_*.pfm %3\pfm

:misspfmb
goto skippfm

:putinpfb

echo Putting PFM file directly in PFB directory

if not exist %7.pfm goto misspfmc
echo Copying reencoded %7.pfm back to %3
copy %7.pfm %3

:misspfmc
if not exist %7_*.pfm goto misspfmd
echo Copying reencoded %7_*.pfm back to %3
copy %7_*.pfm %3

:misspfmd
goto skippfm

:skippfm
rem Now think about writing back modified ATM.INI

rem we don't mess with ATM.INI anymore
goto nowind

rem We skip this code. We don't want to mess with ATM.INI in Windows directory!

if not exist atm.ini goto nowind

if not exist %5\atm.ini goto copyatm
rem echo Deleting old %5\atm.bak --- if is exists
if exist %5\atm.bak del %5\atm.bak
echo Renaming old %5\atm.ini to atm.bak
rename %5\atm.ini atm.bak

:copyatm
echo Copying modified ATM.INI back to %5
copy atm.ini %5

:nowind
goto flushqlc

:nowrite
if not "%8" == "" goto skipwarn
echo 
echo PFB, PFM, and TFM files, and ATM.INI appear in current directory
echo Now rerun ENCODE with the trailing `W' to actually overwrite original files
echo Or, copy TFM files to your TEXFONTS directory and install fonts 
echo from current directory using ATM.

:skipwarn
goto flushqlc

:skipthis
rem get here if couldn't find some files for this particular font
echo ERROR: skipping font %7
pause

:flushqlc
rem we no longer flush ATMFONTS.QLC
goto end

rem only bother with this when we have reached the last font
if not "%8" == "" goto end

if "%6" == "N" goto end
if "%6" == "NX" goto end

echo Will now try and delete ATMFONTS.QLC (ATM font metric cache file) 
echo	(press control-C to interrupt)
pause
rem Exploit alternate use of DECODE
decode -vr %5\atm.ini
if errorlevel = 1 goto atmfail
echo The file ATMFONTS.QLC has been deleted (or was already absent)
goto gone

:atmfail
echo Unable to find ATM.INI, or [Settings] in ATM.INI, or QLCDir= in [Settings]
echo Perhaps you are using an old version ATM?  Please upgrade to ATM 3.0

:gone
if "%6" == "N" goto end
if "%6" == "NX" goto end
echo 
echo You may have to power cycle your printer to clear its font cache -
echo if you printed with old version of the font since it was last powered up.
goto end

:usage
echo Use: %0 [vec] [afm-dir] [pfb-dir] [tfm-dir] [win-dir] [W or N] [font-1] ...
echo 
echo Where [vec]      is the name of the encoding vector
echo       [afm-dir]  is the directory with Adobe Font Metric files
echo       [pfb-dir]  is the directory with Printer Font Binary files
echo       [tfm-dir]  is the directory with TeX Font Metric files (or N)
echo       [win-dir]  is the Windows directory (where ATM.INI may be found)
echo       W          write modified TFM files back to their directory
echo       N          all modified files appear only in current directory
echo       [font]     is the font's file name (without extension,
echo                  and without trailing underscores, if any)
echo       Up to four fonts may be reencoded at the same time.
echo 
echo e.g.  %0  standard  c:\afm  c:\psfonts  c:\tex\tfm  c:\windows  W  tir
echo 
echo The PFM files are assumed to be in the PFM subdirectory of the PFB directory.
echo New PFB, PFM and TFM files are deposited in the current directory.
echo The new TFM file will also overwrite the original one if  W  is specified.
echo Specify  N  instead of [tfm-dir] if you don't use TeX.
echo 
echo Install the PFB and PFM file using ATM from current directory afterwards.
echo Always keep copies of original PFB, PFM and TFM files in a safe place.
goto end

:end
rem see whether any more fonts to work on:
if "%8" == "" goto final

echo ************************************************************************
echo Will work next on font %8
rem Give user a chance to view messages for this font before going on...
pause

rem tail recursive call - dropping the font (%7) that we just worked on
rem encode %1 %2 %3 %4 %5 %6 %8 %9

rem shift arguments left (so we can squeeze one more font name in!)
shift
rem tail recursive call - dropping the font (%6) that we just worked on
rem we are assuming here this batch file is still called ENCODE.BAT !
if "%5" == "W" goto addanx
if "%5" == "N" goto addanx
encode %0 %1 %2 %3 %4 %5 %7 %8 %9
goto adddone

:addanx
encode %0 %1 %2 %3 %4 %5X %7 %8 %9
goto adddone

:adddone
rem We should not ever get back here, since we did not use CALL...
echo ERROR: An impossible error has occured (%5 is not W or N)!

:abort
echo Sorry, processing aborted.
echo 
echo NOTE: Make sure the following utilities are on the DOS PATH or current dir
echo       reencode.exe, afmtotfm, afmtopfm, decode.exe, safeseac.exe

rem At this point we probably don't need ATM.INI anymore
rem Hence probably don't need the Windows directory.

:final
