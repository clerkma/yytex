@echo off
if "%1" == "" goto end
echo adding glyphs to cmbxti%1

rem if exist cmu%1.afm addchars -x cmbxti%1 138 cmu%1 sterling
rem if not exist cmu%1.afm echo cmu%1 does not exist
if exist cmbxsl%1.afm addchars -x cmbxti%1 139 cmbxsl%1 dollar
if not exist cmbxsl%1.afm echo cmbxsl%1 does not exist

if exist cmmib%1.afm addchars -s=0.25 -x cmbxti%1 140 cmmib%1 less
if exist cmmib%1.afm addchars -s=0.25 -x cmbxti%1 141 cmmib%1 greater
if not exist cmmib%1.afm echo cmmib%1 does not exist

if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 142 cmbsy%1 braceleft
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 143 cmbsy%1 braceright
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 144 cmbsy%1 backslash
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 145 cmbsy%1 bar
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 146 cmbsy%1 dagger
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 147 cmbsy%1 daggerdbl
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 148 cmbsy%1 section
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 149 cmbsy%1 paragraph
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 150 cmbsy%1 multiply
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 151 cmbsy%1 divide
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 152 cmbsy%1 minus
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 153 cmbsy%1 bullet
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 154 cmbsy%1 periodcentered
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 155 cmbsy%1 logicalnot
if exist cmbsy%1.afm addchars -s=0.25 -x cmbxti%1 156 cmbsy%1 plusminus

if not exist cmbsy%1.afm echo cmbsy%1 does not exist

rem NOTE in Times-Italic, 
rem less, greater, backslash, bar, bullet, logicalnot, 
rem plusminus, periodcentered, multiply and divide are NOT skewed

:end
