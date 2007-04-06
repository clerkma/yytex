@echo off
rem Copyright (C) 1990 Berthold K.P. Horn Y&Y
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%1" == "" goto argok
echo Usage:   cmdumpb a:      or     cmdumpb b:
goto end
:argok
copy c:\cmbkph\cmbx6.pfb %1
copy c:\cmbkph\cmbx8.pfb %1
copy c:\cmbkph\cmbx9.pfb %1
copy c:\cmbkph\cmdunh10.pfb %1
copy c:\cmbkph\cmff10.pfb %1
copy c:\cmbkph\cmfi10.pfb %1
copy c:\cmbkph\cmfib8.pfb %1
copy c:\cmbkph\cminch.pfb %1
copy c:\cmbkph\cmmi6.pfb %1
copy c:\cmbkph\cmmi8.pfb %1
copy c:\cmbkph\cmmi9.pfb %1
copy c:\cmbkph\cmr6.pfb %1
copy c:\cmbkph\cmr8.pfb %1
copy c:\cmbkph\cmr9.pfb %1
copy c:\cmbkph\cmsl8.pfb %1
copy c:\cmbkph\cmsl9.pfb %1
copy c:\cmbkph\cmsltt10.pfb %1
copy c:\cmbkph\cmss8.pfb %1
copy c:\cmbkph\cmss9.pfb %1
copy c:\cmbkph\cmssi8.pfb %1
copy c:\cmbkph\cmssi9.pfb %1
copy c:\cmbkph\cmsy6.pfb %1
copy c:\cmbkph\cmsy8.pfb %1
copy c:\cmbkph\cmsy9.pfb %1
copy c:\cmbkph\cmtcsc10.pfb %1
copy c:\cmbkph\cmtex8.pfb %1
copy c:\cmbkph\cmtex9.pfb %1
copy c:\cmbkph\cmti7.pfb %1
copy c:\cmbkph\cmti8.pfb %1
copy c:\cmbkph\cmti9.pfb %1
copy c:\cmbkph\cmtt12.pfb %1
copy c:\cmbkph\cmtt8.pfb %1
copy c:\cmbkph\cmtt9.pfb %1
copy c:\cmbkph\cmu10.pfb %1
copy c:\cmbkph\cmvtt10.pfb %1
copy c:\cmbkph\copyrght.txt %1
:end
@echo on
