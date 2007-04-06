@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%SETVCENV%" == "1" call 32bit 

if not "%2" =="" goto force

if not exist c:\prog\%1.exe goto force
rem copydate -v -x -m c:\prog\%1.exe c:\c32\%1.c
copydate -v -x -m %1.exe c:\c32\%1.c
if errorlevel == 1 goto force
rem echo The file c:\prog\%1.exe is not older than the file c:\c32\%1.c
echo The file %1.exe is not older than the file c:\c32\%1.c
goto end

:force
rem cl /c c:\c32\%1.c
rem c:\msvcnt\bin\cl -Gf %1
rem c:\msdev\bin\cl -Gf %1
rem Right now this has no optimizations, warning levels, debugging info
rem -c compile only, no link
rem -F 6000 change stack allocation ? passed to LINK ?
rem -Ox == /Ogityb1 /Gs
rem -Ge stack checking (remove later for speed) -Gs maybe?
rem -Gy separate functions for linker (not for single obj job?)
rem -Gf enable string pooling
rem -Zd line number information for debugging ?
rem run cl /? to see options

echo COMPILE NON-debug version
cl /c /Gf /Ge /W4 /Ox c:\c32\%1.c
rem echo COMPILE DEBUG version
rem cl /c /Gf /Ge /W4 /Zi c:\c32\%1.c

if errorlevel == 1 goto end

echo LINK NON-debug version
rem link /PDB:%1.pdb /MAP:%1.map %1.obj  "g:\Program Files\Microsoft Visual Studio\vc98\Lib\setargv.obj"
link /PDB:%1.pdb /MAP:%1.map %1.obj  "g:\Microsoft Visual Studio\vc98\Lib\setargv.obj"

rem echo LINK DEBUG version
rem link /DEBUG /PDB:%1.pdb /MAP:%1.map %1.obj "g:\Program Files\DevStudio\VC\lib\setargv.obj"

:end
