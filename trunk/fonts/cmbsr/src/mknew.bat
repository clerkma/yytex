@echo off
echo Making %1.pfa

copydate -x c:\cmbsr\%1.hex c:\cmbsr\%1.out
if not errorlevel == 1 goto noconvert
call convert -v c:\cmbsr\%1.out

:noconvert
rem k => texfont (=> make a space and put in 160) etc
rem l => remap 0 - 31 to higher up
rem 1 => BSR copyright
rem edit in /Notice afterwards as needed
copydate -x c:\cmbsr\%1.pfa  c:\cmbsr\%1.hex c:\cmbsr\%1.hnt c:\cmbsr\%1.afm
if not errorlevel == 1 goto nofontone
echo fontone -vkl1 c:\cmbsr\%1
fontone -vkl1 c:\cmbsr\%1

:nofontone
rem need new UniqueIDs
rem look out for E and F and T top hint should same as H I etc
