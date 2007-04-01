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

#include <stdio.h>

/* ------------ Rokicki's undos.c ------------------ */

error(s)
char *s ;
{
   fprintf(stderr, "undos: %s\n", s) ;
   if (*s == '!')
      exit(10) ;
}

char *hxdata = "0123456789ABCDEF" ;

main() {
   int c, prevc ;
   long len ;

   c = getc(stdin) ;
   if (c != 0x80)
      error("! not an MSDOS font file") ;
   while (1) {
      c = getc(stdin) ;
      switch(c) {
case 1:
case 2:
         len = getc(stdin) ;
         len += getc(stdin) * 256L ;
         len += getc(stdin) * 65536L ;
         len += getc(stdin) * 256L * 65536 ;
         if (c == 1) {
            while (len > 0) {
               c = getc(stdin) ;
               if (c == EOF) {
                  error("premature EOF in MS-DOS font file") ;
                  len = 0 ;
               } else {
                  if (c == 13)
                     (void)putc(10, stdout) ;
                  else
                     (void)putc(c, stdout) ;
                  len-- ;
               }
            }
         } else {
            putc(10, stdout) ;
            prevc = 0 ;
            while (len > 0) {
               c = getc(stdin) ;
               if (c == EOF) {
                  error("premature EOF in MS-DOS font file") ;
                  len = 0 ;
               } else {
                  (void)putc(hxdata[c >> 4], stdout) ;
                  (void)putc(hxdata[c & 15], stdout) ;
                  len-- ;
                  prevc += 2 ;
                  if (prevc >= 76) {
                     putc(10, stdout) ;
                     prevc = 0 ;
                  }
               }
            }
         }
         break ;
case 3:
         goto msdosdone ;
default:
         error("saw type other than 1, 2, or 3 in MS-DOS font file")  
;
         break ;
      }
      c = getc(stdin) ;
      if (c == EOF)
         break ;
      if (c != 0x80) {
         error("saw non-MSDOS header in MSDOS font file") ;
         break ;
      }
   }
msdosdone:
   if (prevc != 10)
      (void)putc('\n', stdout) ;
}
