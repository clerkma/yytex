@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%2" == "" echo DEBUGGING MODE
if "%2" == "" echo NORMAL MODE

if not "%MSDevDir%" == "" goto skipvars

echo call "g:\Program Files\Microsoft Visual Studio\vc98\Bin\vcvars32.bat"
call "g:\Program Files\Microsoft Visual Studio\vc98\Bin\vcvars32.bat"
echo call e:\mssdk\setenv.bat e:\mssdk
call e:\mssdk\setenv.bat e:\mssdk

:skipvars
echo copydate -v -x %1.dll c:\c32\%1.c
copydate -v -x %1.dll c:\c32\%1.c
if errorlevel == 1 goto force
echo The file %1.dll is not older than the file c:\c32\%1.c
goto end

:force

if not "%2" == ""  goto debug

echo NON-debug mode

rem cl /Gf /Ge /W4 /Zi /Ox c:\c32\%1.c
rem cl /c /MT /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX c:\c32\%1.c
cl /c /MT /W4 /GX /Ox /O2 /DMT /D_X86=1 /DWIN32 /D_WIN32 /DNDEBUG /D_WINDOWS /YX c:\c32\%1.c

if errorlevel == 1 goto end

rem link /MAP:%1.map %1.obj setargv.obj
link /RELEASE /DLL /subsystem:windows /MAP:%1.map  kernel32.lib gdi32.lib user32.lib %1.obj

goto end

:debug
echo DEBUG MODE

rem cl /c /Gf /Ge /W4 /Zd /Ox c:\c32\%1.c
rem cl /c /MTd /W4 /Gm /GX /Zi /Od /D "WIN32" /D " _DEBUG" /D "_WINDOWS" /YX c:\c32\%1.c
cl /c /MTd /W4 /Gm /GX /Zi /Od /D_X86=1 /D_MT /DWIN32 /D_WIN32 /D_DEBUG /D_WINDOWS /YX c:\c32\%1.c

if errorlevel == 1 goto end

rem LINK <objs>,<exefile>,<mapfile>,<libs>,<deffile>

rem link /DEBUG /MAP:%1.map %1.obj setargv.obj
rem link /DEBUG /DLL /subsystem:windows /MAP:%1.map %1.obj
link /DEBUG /DLL /subsystem:windows /MAP:%1.map kernel32.lib gdi32.lib user32.lib  %1.obj

rem /LIBPATH:j:\mssdk\Lib

goto end


:end
