@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

d:
c:\prog\decompre -v %1.pfb
c:\prog\decrypt -ve %1.pfa
c:\prog\decrypt -vr %1.dec
c:\prog\makeoutl %1.pln
c:\prog\convert -v %1.out
c:\prog\showchar -v -h d: %1.hex
@echo on
