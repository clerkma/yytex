@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem make a PLN file from a PFA or PFB file
rem if not exist %1.pfb goto dopfa
rem pfbtopfa -v %1.pfb
rem echo 
:dopfa
if not exist %1.pfa goto dodec
decrypt -vse %1.pfa
echo 
:dodec
if not exist %1.dec goto donot
rem decrypt -vsr %1.dec
decrypt -vr %1.dec
:donot
@echo on
