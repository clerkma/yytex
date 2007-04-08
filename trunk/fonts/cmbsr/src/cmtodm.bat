@echo off
echo Give only part of font name after CM and make it upper case...
echo Making DM%1 from CM%1
pause
echo Making DM%1.PFA from CM%1.PFA
rem reencode -v -u -r=DM%1 -n=DM%1 CM%1.PFA
reencode -v -r=DM%1 -n=DM%1 CM%1.PFA
if exist DM%1.PFA del DM%1.PFA
rename CM%1X.PFA DM%1.PFA
reencode -v DM%1.PFA
pfatopfb -v DM%1.PFA

echo Making DM%1.PFM from CM%1.AFM
COPY CM%1.AFM DM%1.AFM
rem AFMTOPFM -var -c=ansinew -w=DM%1 DM%1.AFM
AFMTOPFM -v -w=DM%1 DM%1.AFM