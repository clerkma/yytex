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

#include <stdlib.h>
#include <stdio.h>


int main(int argc, char *argv[]) {
	FILE *input;
	int start = 388;
	int gap = 4;
	int step = 260;
	int k, m, c;
	
	if ((input = fopen(argv[1], "rb")) == NULL) {
		perror(argv[1]); exit(1);
	}
	for (k = 0; k < start; k++) (void) getc(input);

	for (m = 0; m < 4; m++) {
		for (k = 0; k < 256; k++) {
			c = getc(input);
			printf("%d\t=>\t%d\n", k, c);
		}
		for (k = 0; k < gap; k++) (void) getc(input);
		putc('\n', stdout);
	}	

	fclose(input);
	return 0;
}
