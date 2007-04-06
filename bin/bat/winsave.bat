@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if "%1" == "" %0 a:
rem xcopy c:\windev\samples\dviwindo\*.c %1
savefile c:\windev\samples\dviwindo\*.c %1
rem xcopy c:\windev\samples\dviwindo\*.h %1
savefile c:\windev\samples\dviwindo\*.h %1
rem xcopy c:\windev\samples\dviwindo\*.rc %1
savefile c:\windev\samples\dviwindo\*.rc %1
rem xcopy c:\windev\samples\dviwindo\*.def %1
savefile c:\windev\samples\dviwindo\*.def %1
rem xcopy c:\windev\samples\dviwindo\*.dlg %1
savefile c:\windev\samples\dviwindo\*.dlg %1
rem xcopy c:\windev\samples\dviwindo\*.ico %1
savefile c:\windev\samples\dviwindo\*.ico %1
rem xcopy c:\windev\samples\dviwindo\*.cur %1
savefile c:\windev\samples\dviwindo\*.cur %1
rem xcopy c:\windev\samples\dviwindo\*.txt %1
savefile c:\windev\samples\dviwindo\*.txt %1
goto end
rem xcopy c:\windev\samples\setup\*.c %1
savefile c:\windev\samples\setup\*.c %1
rem xcopy c:\windev\samples\setup\*.h %1
savefile c:\windev\samples\setup\*.h %1
rem xcopy c:\windev\samples\setup\*.rc %1
savefile c:\windev\samples\setup\*.rc %1
rem xcopy c:\windev\samples\setup\*.def %1
savefile c:\windev\samples\setup\*.def %1
rem xcopy c:\windev\samples\setup\*.dlg %1
savefile c:\windev\samples\setup\*.dlg %1
rem xcopy c:\windev\samples\setup\*.ico %1
savefile c:\windev\samples\setup\*.ico %1
rem copy c:\windev\samples\setup\*.cur %1
rem xcopy c:\windev\samples\setup\wslib\*.c %1
savefile c:\windev\samples\setup\wslib\*.c %1
rem xcopy c:\windev\samples\setup\wslib\*.h %1
savefile c:\windev\samples\setup\wslib\*.h %1
:end
