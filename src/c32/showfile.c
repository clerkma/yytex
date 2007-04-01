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
	int c;
	
	if (argc < 2) exit(1);
	input = fopen(argv[1], "rb");
	if (input == NULL) {
		perror(argv[1]);
		exit(1);
	}
	for (;;) {
		c = getc(input);
		printf("%3d ", c);
		if (c == 32) printf(" SPACE");
		else if (c == 10) printf(" NEWLINE");
		else if (c == 13) printf(" RETURN");
		else if (c > 32) printf(" %c", c);
		else if (c < 32) printf(" C-%c", c+64);
		printf("\n");
		if (c < 0) break;
	}
	fclose(input);
	return 0;
}
