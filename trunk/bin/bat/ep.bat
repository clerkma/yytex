@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem d:
rem cd \epsilon
rem d:\epsilon\bin\epsilon %1 %2 %3 %4 %5 %6 %7 %8 %9
rem The -m0 is to make more memory available in process buffer
rem d:\epsilon\bin\epsilon -m0 -sd:\epsilon\epsilon.sta %1 %2 %3 %4 %5 %6 %7 %8 %9 
rem The -kw is for Windowing environment
d:\epsilon\bin\epsilon -kw -sd:\epsilon\epsilon.sta %1 %2 %3 %4 %5 %6 %7 %8 %9
