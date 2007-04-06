@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem show time capsule
if not exist %1.pfb goto dopfa
echo pfbtopfa -v %1.pfb
pfbtopfa -v %1.pfb
echo 
:dopfa
if not exist %1.pfa goto dodec
echo decrypt -e %1.pfa
decrypt -e %1.pfa
echo 
:dodec
if not exist %1.dec goto donot
echo decrypt -vsr %1.dec
decrypt -vsr %1.dec
:donot
if exist %1.pln del %1.pln
if exist %1.dec del %1.dec
@echo on
