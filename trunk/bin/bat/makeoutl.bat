@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem Make outline and hint file from  PFA file ( a PFB or )
rem if (%2) == () goto noflex
if not (%2) == () goto noflex
echo ALLOWING FlexProc
rem pause
goto flexskip

:noflex
echo Do NOT allow FlexProc 
rem pause

:flexskip
rem if not exist %1.pfb goto dopfa
rem pfbtopfa -v %1.pfb
echo 

:dopfa
if not exist %1.pfa goto dodec
echo 
decrypt -vse %1.pfa
echo 

:dodec
if not exist %1.dec goto end
rem decrypt -vsr %1.dec
decrypt -vr %1.dec
echo 
rem if (%2) == () goto lucid
if not (%2) == () goto lucid
chdir

rem Allow use of flex proc (normal fonts):
extroutl %1.pln
goto skipover

:lucid
rem Disallow use of flex proc (LucidaBright):
extroutl -f %1.pln

:skipover
if errorlevel 1 goto end
echo 
rem convertc -v %1.out
convertc -v %1.out
rem pause Ready for viewing outlines and hints?
rem showchar -h=d: -v %1.hex

:end
