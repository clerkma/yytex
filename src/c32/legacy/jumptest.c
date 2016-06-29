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

#include <setjmp.h>
#include <stdio.h>

jmp_buf j;

void raise_exception(void) {
   printf("exception raised\n");
   longjmp(j, 1); /* jump to exception handler */
   printf("this line should never appear\n");
}
   
int main(void) {
   if (setjmp(j) == 0)  {
      printf("'setjmp' is initializing 'j'\n");
      raise_exception();
      printf("this line should never appear\n");
   }
   else {
      printf("'setjmp' was just jumped into\n");
      /* this code is the exception handler */
   }
   return 0;
}
   
/* When run yields:
   'setjmp' is initializing 'j'
   exception raised
   'setjmp' was just jumped into
*/
