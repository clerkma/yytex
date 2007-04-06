@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.
rem make SliTeX fonts
d:
rem AFM files with the composite characters are presently in latxfonts (1.00B)
rem AFM files without composite characters are in c:\afm\tex (1.00A)
rem
rem c:\prog\fontone -klq -w=c:\afm\tex -h=c:\latxfont c:\latxfont\lcmss8.hex
rem c:\prog\fontone -klq -w=c:\afm\tex -h=c:\latxfont c:\latxfont\lcmssb8.hex
rem c:\prog\fontone -klq -w=c:\afm\tex -h=c:\latxfont c:\latxfont\lcmssi8.hex
rem 
c:\prog\fontone -kl -w=c:\latxfont -h=c:\latxfont c:\latxfont\lcmss8.hex
c:\prog\fontone -kl -w=c:\latxfont -h=c:\latxfont c:\latxfont\lcmssb8.hex
c:\prog\fontone -kl -w=c:\latxfont -h=c:\latxfont c:\latxfont\lcmssi8.hex
rem 
rem c:\prog\fontone -klq -w=c:\afm\tex c:\latxfont\ilcmss8.hex
rem c:\prog\fontone -klq -w=c:\afm\tex c:\latxfont\ilcmssb8.hex
rem c:\prog\fontone -klq -w=c:\afm\tex c:\latxfont\ilcmssi8.hex
rem
c:\prog\fontone -kl -w=c:\latxfont c:\latxfont\ilcmss8.hex
c:\prog\fontone -kl -w=c:\latxfont c:\latxfont\ilcmssb8.hex
c:\prog\fontone -kl -w=c:\latxfont c:\latxfont\ilcmssi8.hex
rem 
rem c:\prog\fontone -klq -w=c:\latxfont c:\latxfont\icmtt8.hex
rem 
c:\prog\fontone -kl -w=c:\latxfont c:\latxfont\icmtt8.hex
rem 
c:\prog\fontone -kl -w=c:\afm\tex c:\latxfont\ilasy8.hex
c:\prog\fontone -kl -w=c:\afm\tex c:\latxfont\icmex10.hex
c:\prog\fontone -kl -w=c:\afm\tex c:\latxfont\icmsy8.hex
c:\prog\fontone -kl -w=c:\afm\tex c:\latxfont\icmmi8.hex
rem
rem c:\prog\fontone -klq -w=c:\afm\tex c:\latxfont\ilasy8.hex
rem c:\prog\fontone -klq -w=c:\afm\tex c:\latxfont\icmex10.hex
rem c:\prog\fontone -klq -w=c:\afm\tex c:\latxfont\icmsy8.hex
rem c:\prog\fontone -klq -w=c:\afm\tex c:\latxfont\icmmi8.hex
rem 
echo ^D
pfatopfb -v lcmss8.pfa
pfatopfb -v lcmssb8.pfa
pfatopfb -v lcmssi8.pfa
pfatopfb -v ilcmss8.pfa
pfatopfb -v ilcmssb8.pfa
pfatopfb -v ilcmssi8.pfa
pfatopfb -v ilasy8.pfa
pfatopfb -v icmex10.pfa
pfatopfb -v icmsy8.pfa
pfatopfb -v icmmi8.pfa
pfatopfb -v icmtt8.pfa
echo ^D
@echo on
