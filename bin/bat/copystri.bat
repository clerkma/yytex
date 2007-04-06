rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem Helper function for Unix/NeXT dumping routines (text files)
rem
if "%3" == "" goto usage
rem
echo strip copying %1\%2 to %3\%2
rem Strip file %1\%2, put result in %2, copy to %3\%2, and delete %2
stripcom -r %1\%2
copy %2 %3\%2
del %2
goto end
:usage
echo "Usage:	copystrip <source-dir> <file> <destination-dir>"
:end
