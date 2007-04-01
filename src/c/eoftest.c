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
#include <stdlib.h>

int main (int argc, char *argv[]) {
	int c, d, e, i;
	FILE *input;

	if ((input = fopen("test.txt", "r")) == NULL) {
		perror("test.txt"); exit(1);
	}
	for (i=0; i < 64; i++) {
		d = feof(input);
		c = getc(input);
		e = feof(input);		
		printf("before %d char %d after %d\n", d, c, e);;
		if (d && e && c == EOF) break;
	}
	fclose (input);
	return 0;
}
