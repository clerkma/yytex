@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.
rem
echo LaTeX fonts
rem
rem To make NewGen safe version simply add `m' command line flag to all
rem
rem fontone -kbl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\lasy*.hex
fontone -kl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\lasy*.hex
fontone -kl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\line*.hex
fontone -kl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\lcircle*.hex
pause
rem
echo Logo Fonts
rem
fontone -kj -w=c:\afm\tex -h=c:\latxfont c:\latxfont\logo*.hex
rem
echo Visible SliTeX fonts
rem
fontone -kl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\lcmss*.hex
rem
echo In-visible SliTeX fonts
rem
fontone -kbl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\ilasy8.hex
fontone -kl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\ilcmss8.hex
fontone -kl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\ilcmssb8.hex
fontone -kl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\ilcmssi8.hex
fontone -kl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\icmtt8.hex
fontone -kbl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\icmex10.hex
fontone -kbl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\icmmi8.hex
fontone -kbl -w=c:\afm\tex -h=c:\latxfont c:\latxfont\icmsy8.hex
rem
pause
pfatopfb *.pfa
rem
@echo on
