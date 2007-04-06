rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

rem mode lpt1:
rem c:\utility\superspl lpt1: /m=1024 /extm /pint=7 /dl /dh /dmmgmt
rem c:\utility\superspl lpt1: /m /extm /pint=7 /dl /dh /dmmgmt
rem c:\utility\superspl lpt1: /m /extm /dl /dh /dmmgmt
rem c:\utility\superspl lpt1: /m=640 /extm
rem superspl lpt1:/m/extm=0,5120/tsize=32/pint=7/dmmgmt/dl/dh
rem superspl lpt1:/m=1024/extm=0,2048/tsize=32/dmmgmt/pint=7/dl/dh
rem superspl lpt1:/m/extm=0,3072/u=512/tsize=32/dmmgmt/pint=7/dl/dh
rem c:\utility\superspl lpt1: /m=1024 /extm /pint=7 /dl /dh /dmmgmt
rem c:\utility\superspl lpt1: /m /extm /pint=7 /dl /dh /dmmgmt
rem echo on
rem if /%1/==// goto ask
goto yes
rem pause Make sure the baud rate has been set using Kermit
:ask
echo Has the baud rate been set using Kermit? [Y]
ask
if errorlevel = 2 goto yes
if errorlevel = 1 goto enter
goto no
:yes
:enter
rem echo off
rem c:\utility\superspl lpt1:=com1: /m=1024 /extm /dl /dh /dmmgmt /sint=4 /on=DSR
rem c:\utility\superspl lpt1:=com1: /m=1248 /extm /dl /dh /dmmgmt /sint=4 /on=DSR 
c:\utility\superspl lpt1: /m=1248 /extm /dl /dh /dmmgmt /pint=7 
rem  /rate=9600,n,7,1
rem  /rate=19200,n,7,1,b
rem  /rate=38400,n,7,1,b
rem  /tsize
:no
