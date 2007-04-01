/* Copyright 2007 TeX Users Group

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.  */

--- pdf_main.ps.orig	Sat Jun 22 16:48:51 1996
+++ pdf_main.ps	Tue Jul  2 14:51:57 1996
@@ -108,6 +108,210 @@
 end			% userdict
 pdfdict begin
 
+% An implementation of an algorithm compatible with the RSA Data Security 
+% Inc. RC4 stream encryption algorithm.
+
+% <string> rc4setkey <dict>
+/rc4setkey
+{
+  6 dict begin
+    /k exch def
+    /a 256 string def
+    0 1 255 { a exch dup put } for
+    /l k length def
+    /j 0 def
+    0 1 255
+    {
+      /i exch def
+      /j a i get k i l mod get add j add 255 and store
+      a i a j get a j a i get put put
+    } for
+    3 dict dup begin
+      /a a def
+      /x 0 def
+      /y 0 def
+    end
+  end
+} bind def
+
+% <rc4key> <string> rc4 <string> <rc4key>
+% where 'rc4key' and 'string' are the same object, modified.
+/rc4
+{
+  1 index begin
+    dup dup length 1 sub 0 exch 1 exch
+    {
+      /x x 1 add 255 and store
+      /y a x get y add 255 and store
+      a x a y get a y a x get put put
+% stack: string string index
+      2 copy get
+      a dup x get a y get add 255 and get
+      xor put dup
+    } for
+    pop
+  end
+} bind def
+
+% take a stream and rc4 decrypt it.
+% <stream> <key> rc4decodefilter <stream>
+/rc4decodefilter {
+  rc4setkey exch 512 string
+   % stack: <key> <stream> <string>
+  { readstring pop rc4 exch pop } aload pop
+   8 array astore cvx 0 () /SubFileDecode filter 
+} bind def
+
+% MD5 derived from RFC 1321, "The MD5 Message-Digest Algorithm",
+% R. Rivest, MIT, RSADSI.
+
+% We construct the MD5 transform by a sort of inline expansion.
+% this takes up quite a bit of memory (around 17k), but gives a
+% factor-of-two speed increase.  This also allows us to make use of 
+% interpreters with 64-bit wide integers.
+% <string> md5 <string>
+20 dict begin
+
+% Wouldn't it be nice if the sin operator had enough precision
+% for us to calculate these directly?
+/T [
+16#d76aa478 16#e8c7b756 16#242070db 16#c1bdceee
+16#f57c0faf 16#4787c62a 16#a8304613 16#fd469501
+16#698098d8 16#8b44f7af 16#ffff5bb1 16#895cd7be
+16#6b901122 16#fd987193 16#a679438e 16#49b40821
+16#f61e2562 16#c040b340 16#265e5a51 16#e9b6c7aa
+16#d62f105d 16#02441453 16#d8a1e681 16#e7d3fbc8
+16#21e1cde6 16#c33707d6 16#f4d50d87 16#455a14ed
+16#a9e3e905 16#fcefa3f8 16#676f02d9 16#8d2a4c8a
+16#fffa3942 16#8771f681 16#6d9d6122 16#fde5380c
+16#a4beea44 16#4bdecfa9 16#f6bb4b60 16#bebfbc70
+16#289b7ec6 16#eaa127fa 16#d4ef3085 16#04881d05
+16#d9d4d039 16#e6db99e5 16#1fa27cf8 16#c4ac5665
+16#f4292244 16#432aff97 16#ab9423a7 16#fc93a039
+16#655b59c3 16#8f0ccc92 16#ffeff47d 16#85845dd1
+16#6fa87e4f 16#fe2ce6e0 16#a3014314 16#4e0811a1
+16#f7537e82 16#bd3af235 16#2ad7d2bb 16#eb86d391
+] def
+/F [
+{ c d /xor b /and d /xor } { b c /xor d /and c /xor }
+{ b c /xor d /xor } { d /not b /or c /xor }
+] def
+/R [
+16#0007 16#010c 16#0211 16#0316 16#0407 16#050c 16#0611 16#0716
+16#0807 16#090c 16#0a11 16#0b16 16#0c07 16#0d0c 16#0e11 16#0f16
+16#0105 16#0609 16#0b0e 16#0014 16#0505 16#0a09 16#0f0e 16#0414
+16#0905 16#0e09 16#030e 16#0814 16#0d05 16#0209 16#070e 16#0c14
+16#0504 16#080b 16#0b10 16#0e17 16#0104 16#040b 16#0710 16#0a17
+16#0d04 16#000b 16#0310 16#0617 16#0904 16#0c0b 16#0f10 16#0217
+16#0006 16#070a 16#0e0f 16#0515 16#0c06 16#030a 16#0a0f 16#0115
+16#0806 16#0f0a 16#060f 16#0d15 16#0406 16#0b0a 16#020f 16#0915
+] def
+
+/W 1 31 bitshift 0 gt def
+/A W { /add } { /md5add } ifelse def
+/t W { 1744 } { 1616 } ifelse array def
+/C 0 def
+
+0 1 63 {
+  /i exch def
+  /r R i get def
+  /a/b/c/d 4 i 3 and roll [ /d/c/b/a ] { exch def } forall
+
+  t C [
+    a F i -4 bitshift get exec 
+    a A /x r -8 bitshift /get A T i get A
+    /dup r 31 and /bitshift W { 1 32 bitshift 1 sub /and } if
+      /exch r 31 and 32 sub /bitshift /or
+    b A
+    /store
+  ] dup length C add /C exch store putinterval
+} for
+
+1 1 C 1 sub {
+  dup 1 sub t exch get /store cvx eq
+    {pop}
+    {t exch 2 copy get cvx put}
+  ifelse
+} for
+
+% If we could put t into a _packed_ array, its memory requirements
+% would go from about 13k to about 4k. Unfortunately, we'd need around
+% 1600 stack positions, around 3 times what we can expect to have 
+% available---and if that kind of memory is available, we don't really
+% need to pack t. Sigh.
+
+% In fact, it's worse than that. You can't even determine what t will 
+% be and write it in directly (something like
+% { /a c d xor b and d xor a md5add x 0 get md5add -680876936 md5add dup 7 
+%   bitshift exch -25 bitshift or b md5add store /d b c xor a ...
+% ) because the scanner uses the operand stack to accumulate procedures.
+% So the only way to have md5transform as a single procedure is the above 
+% trick.
+
+W /md5transform t end cvx bind def
+
+% Unfortunately, PostScript & its imitators convert large
+% integers to floating-point. Worse, the fp representation probably
+% won't have 32 significant bits.
+% This procedure is not needed on 64 bit machines.
+% This procedure accounts for about 35% of the total time on 32-bit 
+% machines.
+not {
+  /md5add {
+    2 copy xor 0 lt
+      % if one is positive and one is negative, can't overflow
+      { add }
+      % if both are positive or negative
+      { 16#80000000 xor add 16#80000000 xor }
+      % same as subtracting (or adding) 2^31 and then subtracting (or 
+      % adding) it back.
+    ifelse
+  } bind def
+} if
+
+/md5 {
+  20 dict begin
+
+  % initialise a,b,c,d,x
+  /a 16#67452301 def
+  /b 16#efcdab89 def
+  /c 16#98badcfe def
+  /d 16#10325476 def
+  /x 16 array def
+
+  % parameters
+  /origs exch def
+  /oslen origs length def
+
+  % pad string to multiple of 512 bits
+  /s oslen 72 add 64 idiv 64 mul dup /slen exch def string def
+  s 0 origs putinterval
+  s oslen 16#80 put
+  s slen 8 sub oslen 31 and 3 bitshift put
+  s slen 7 sub oslen -5 bitshift 255 and put
+  s slen 6 sub oslen -13 bitshift 255 and put
+
+  0 64 slen 64 sub {
+    dup 1 exch 63 add { s exch get } for
+    15 -1 0 { x exch 6 2 roll 3 { 8 bitshift or } repeat put } for
+    a b c d
+    md5transform
+    d md5add /d exch store
+    c md5add /c exch store
+    b md5add /b exch store
+    a md5add /a exch store
+  } for
+  
+  16 string
+  [ [ a b c d ] { 3 { dup -8 bitshift } repeat } forall ]
+  0 1 15 {
+    3 copy dup 3 1 roll get 255 and put pop
+  } for
+  pop
+
+  end
+} bind def
+
 % ======================== File parsing ======================== %
 
 % Read the cross-reference and trailer sections.
@@ -169,7 +373,7 @@
 
 % Open a PDF file and read the trailer and cross-reference.
 /pdfopen		% <file> pdfopen <dict>
- { 10 dict begin
+ { 11 dict begin
    /PSLevel1 where { pop } { /PSLevel1 false def } ifelse
    cvlit /PDFfile exch def
    /PDFsource PDFfile def
@@ -195,8 +399,30 @@
 	% Read the last cross-reference table.
    readxref /Trailer exch def
    Trailer /Encrypt known
-    { (****This file is encrypted and cannot be processed.\n) print flush
-      /pdfopen cvx /invalidfileaccess signalerror
+    { <28bf4e5e4e758a41 64004e56fffa0108
+        2e2e00b6d0683e80 2f0ca9fe6453697a> dup
+      Trailer /Encrypt oget
+      dup /Filter oget /Standard eq not
+       { (****This file uses an unknown security handler.\n) print flush
+	 /pdfopen cvx /undefined signalerror
+       }
+      if
+      dup /O oget exch
+      /P oget 4 string exch
+	 2 copy 255 and 0 exch put
+	 2 copy -8 bitshift 255 and 1 exch put
+	 2 copy -16 bitshift 255 and 2 exch put
+	 2 copy -24 bitshift 255 and 3 exch put pop
+      Trailer /ID oget 0 oget      
+      3 { concatstrings } repeat
+      md5 0 5 getinterval
+      Trailer /Encrypt oget /U oget exch
+      dup /FileKey exch def
+      rc4setkey exch rc4 exch pop ne
+       { (****This file has a user password set.\n) print flush
+	 /pdfopen cvx /invalidfileaccess signalerror
+       }
+      if
     }
    if
 	% Read any previous cross-reference tables.
--- pdf_base.ps.orig	Wed Mar 20 02:49:43 1996
+++ pdf_base.ps	Tue Jul  2 18:19:42 1996
@@ -103,6 +103,57 @@
    /PDFsource 3 -1 roll store exec
  } bind def
 
+% As .pdfrun, but decrypt strings with key <key>.
+/.decpdfrun			% <file> <keystring> <opdict> .decpdfrun -
+ {	% Construct a procedure with the file, opdict and key bound into it.
+   2 index cvlit mark mark 5 2 roll
+    { token not { (%%EOF) cvn cvx } if
+      dup xcheck
+       { DEBUG { dup == flush } if
+         3 -1 roll pop
+	 2 copy .knownget
+	  { exch pop exch pop exec }
+	  { (%stderr) (w) file
+	    dup (****************Unknown operator: ) writestring
+	    dup 3 -1 roll .writecvs dup (\n) writestring flushfile
+	    pop
+	  }
+	 ifelse
+       }
+       { exch pop DEBUG { dup ==only ( ) print flush } if
+	 dup type /stringtype eq
+	  { exch rc4setkey exch rc4 }
+	 if
+	 exch pop
+       }
+      ifelse
+    }
+   aload pop .packtomark cvx
+   /loop cvx 2 packedarray cvx
+    { stopped /PDFsource } aload pop
+   PDFsource
+    { store { stop } if } aload pop .packtomark cvx 
+   /PDFsource 3 -1 roll store exec
+ } bind def
+
+% Calculate the key used to decrypt an object (to pass to .decpdfrun or
+% put into a stream dictionary).
+/computeobjkey	% <object#> <generation#> computeobjkey <keystring>
+{
+  exch
+  10 string
+  dup 0 FileKey putinterval
+  exch
+		% stack:  gen# string obj#
+    2 copy 255 and 5 exch put
+    2 copy -8 bitshift 255 and 6 exch put
+    2 copy -16 bitshift 255 and 7 exch put
+  pop exch
+    2 copy 255 and 8 exch put
+    2 copy -8 bitshift 255 and 9 exch put
+  pop md5 0 10 getinterval
+} bind def
+
 % ------ File reading ------ %
 
 % Read the cross-reference entry for an (unresolved) object.
@@ -239,7 +290,19 @@
 	  { (xref error!\n) print /resolveR cvx /rangecheck signalerror
 	  }
 	 if
-	 PDFfile resolveopdict .pdfrun
+	 /FileKey where
+	  { pop
+	    2 copy computeobjkey dup 4 1 roll
+	    PDFfile exch resolveopdict .decpdfrun
+	    dup dup dup 5 2 roll
+		% stack: object object key object object
+	    xcheck exch type /dicttype eq and
+	     { /StreamKey exch put }
+	     { pop pop }
+	    ifelse
+	  }
+	  { PDFfile resolveopdict .pdfrun }
+	 ifelse
        }
        { Objects exch null put pop null
        }
@@ -257,6 +320,7 @@
 %	/File - the file or string where the stream contents are stored;
 %	/FilePosition - iff File is a file, the position in the file
 %	  where the contents start.
+%	/StreamKey - the key used to decrypt this stream if any
 % We do the real work of constructing the data stream only when the
 % contents are needed.
 
@@ -333,6 +397,14 @@
 		% Stack: readdata? dict parms filternames
    2 index /File get exch
 		% Stack: readdata? dict parms file/string filternames
+    3 index /StreamKey known
+    {
+       exch 
+		% Stack: readdata? dict parms filternames file/string
+       3 index /Length oget () /SubFileDecode filter
+       3 index /StreamKey get rc4decodefilter
+       exch
+    } if
    dup length 0 eq
     {		% All the PDF filters have EOD markers, but in this case
 		% there is no specified filter.
