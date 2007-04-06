@echo off
rem Add DRAFT diagonally to TEX output in PS
if exist %1.eps goto eps
if exist %1.ps goto ps
copy c:\ps\draft.ps+%1 com1:/b
goto end
:eps 
copy c:\ps\draft.ps+%1.eps com1:/b
goto end
:ps
copy c:\ps\draft.ps+%1.ps com1:/b
:end
copy c:\ps\controld.txt com1:/b
@echo on