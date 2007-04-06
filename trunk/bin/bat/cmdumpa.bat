@echo off
rem Copyright (C) 1990 Berthold K.P. Horn Y&Y
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

if not "%1" == "" goto argok
echo Usage:   cmdumpa a:      or     cmdumpa b:
goto end
:argok
copy c:\cmbkph\cmb10.pfb %1
copy c:\cmbkph\cmbsy10.pfb %1
copy c:\cmbkph\cmbx10.pfb %1
copy c:\cmbkph\cmbx12.pfb %1
copy c:\cmbkph\cmbx5.pfb %1
copy c:\cmbkph\cmbx7.pfb %1
copy c:\cmbkph\cmbxsl10.pfb %1
copy c:\cmbkph\cmbxti10.pfb %1
copy c:\cmbkph\cmcsc10.pfb %1
copy c:\cmbkph\cmex10.pfb %1
copy c:\cmbkph\cmitt10.pfb %1
copy c:\cmbkph\cmmi10.pfb %1
copy c:\cmbkph\cmmi12.pfb %1
copy c:\cmbkph\cmmi5.pfb %1
copy c:\cmbkph\cmmi7.pfb %1
copy c:\cmbkph\cmmib10.pfb %1
copy c:\cmbkph\cmr10.pfb %1
copy c:\cmbkph\cmr12.pfb %1
copy c:\cmbkph\cmr17.pfb %1
copy c:\cmbkph\cmr5.pfb %1
copy c:\cmbkph\cmr7.pfb %1
copy c:\cmbkph\cmsl10.pfb %1
copy c:\cmbkph\cmsl12.pfb %1
copy c:\cmbkph\cmss10.pfb %1
copy c:\cmbkph\cmss12.pfb %1
copy c:\cmbkph\cmss17.pfb %1
copy c:\cmbkph\cmssbx10.pfb %1
copy c:\cmbkph\cmssdc10.pfb %1
copy c:\cmbkph\cmssi10.pfb %1
copy c:\cmbkph\cmssi12.pfb %1
copy c:\cmbkph\cmssi17.pfb %1
copy c:\cmbkph\cmssq8.pfb %1
copy c:\cmbkph\cmssqi8.pfb %1
copy c:\cmbkph\cmsy10.pfb %1
copy c:\cmbkph\cmsy5.pfb %1
copy c:\cmbkph\cmsy7.pfb %1
copy c:\cmbkph\cmtex10.pfb %1
copy c:\cmbkph\cmti10.pfb %1
copy c:\cmbkph\cmti12.pfb %1
copy c:\cmbkph\cmtt10.pfb %1
copy c:\cmbkph\copyrght.txt %1
:end
@echo on
