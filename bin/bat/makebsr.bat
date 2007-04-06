@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem make BSR fonts
rem
rem To make NewGen safe version simply add `m' command line flag to all
rem c:
rem cd bsr
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmb10.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmbx*.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmcsc*.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmdunh10.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmf*.hex
rem fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cminch.hex
fontone -kq -w=c:\afm\tex -h=c:\bsr c:\bsr\cminch.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmitt10.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmr*.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmsl*.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmss*.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmt*.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmu10.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmvtt10.hex
echo 
rem fontone -klqb -w=c:\afm\tex -h=c:\bsr c:\bsr\cmsy*.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmsy*.hex
rem fontone -klqb -w=c:\afm\tex -h=c:\bsr c:\bsr\cmbsy10.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmbsy10.hex
rem fontone -klqb -w=c:\afm\tex -h=c:\bsr c:\bsr\cmmi*.hex
fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmmi*.hex
rem rem fontone -klqb -w=c:\afm\tex -h=c:\bsr c:\bsr\cmmib*.hex
rem fontone -klq -w=c:\afm\tex -h=c:\bsr c:\bsr\cmmib*.hex
rem fontone -klqbd -w=c:\afm\tex -h=c:\bsr c:\bsr\cmex10.hex
rem following makes 1/2 scale version for NeWSPrint
rem fontone -klqp -w=c:\afm\tex -h=c:\bsr c:\bsr\cmex10.hex
rem rename cmex10.pfa cmexnews.pfa
fontone -klqd -w=c:\afm\tex -h=c:\bsr c:\bsr\cmex10.hex
echo 
rem
pause
rem
pfatopfb -v *.pfa
echo 
del cm*.pfa
rem cd ..
rem d:
rem afmtopfm -sdt c:\afm\tex\cm*.afm
@echo on
