@echo off
if "%1" == "" goto end
echo adding glyphs to cmcsc%1

if exist cmu%1.afm addchars -x cmcsc%1 138 cmu%1 sterling
if not exist cmu%1.afm echo cmu%1 does not exist
rem 139 for dollar

rem if exist cmmi%1.afm addchars -x cmcsc%1 140 cmmi%1 less
rem if exist cmmi%1.afm addchars -x cmcsc%1 141 cmmi%1 greater
rem if not exist cmmi%1.afm echo cmmi%1 does not exist

if exist cmsy%1.afm addchars -x cmcsc%1 142 cmsy%1 braceleft
if exist cmsy%1.afm addchars -x cmcsc%1 143 cmsy%1 braceright
if exist cmsy%1.afm addchars -x cmcsc%1 144 cmsy%1 backslash
if exist cmsy%1.afm addchars -x cmcsc%1 145 cmsy%1 bar
if exist cmsy%1.afm addchars -x cmcsc%1 146 cmsy%1 dagger
if exist cmsy%1.afm addchars -x cmcsc%1 147 cmsy%1 daggerdbl
if exist cmsy%1.afm addchars -x cmcsc%1 148 cmsy%1 section
if exist cmsy%1.afm addchars -x cmcsc%1 149 cmsy%1 paragraph
if exist cmsy%1.afm addchars -x cmcsc%1 150 cmsy%1 multiply
if exist cmsy%1.afm addchars -x cmcsc%1 151 cmsy%1 divide
if exist cmsy%1.afm addchars -x cmcsc%1 152 cmsy%1 minus
if exist cmsy%1.afm addchars -x cmcsc%1 153 cmsy%1 bullet
if exist cmsy%1.afm addchars -x cmcsc%1 154 cmsy%1 periodcentered
if exist cmsy%1.afm addchars -x cmcsc%1 155 cmsy%1 logicalnot
if exist cmsy%1.afm addchars -x cmcsc%1 156 cmsy%1 plusminus

if not exist cmsy%1.afm echo cmsy%1 does not exist

:end