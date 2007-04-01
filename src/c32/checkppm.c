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
#include <string.h>

#define BUFSIZE 4096

char buffer[BUFSIZE];

int main (int argc, char *argv[]) {
	FILE *input;
	int n, m, i, j, r, g, b;
	
	if (argc < 2) exit(1);

	input = fopen(argv[1], "rb");
	if (input == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if (fgets(buffer, sizeof(buffer), input) == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if (fgets(buffer, sizeof(buffer), input) == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if (sscanf(buffer, "%d %d %d", &m, &n, &g) < 3) {
		printf(buffer);
		exit(1);
	}
	for (i = 0; i < n; i++) {
		for (j = 0; j < m; j++) {
			r = getc(input);
			g = getc(input);
			b = getc(input);
			if (r != g || g != b || b != r)
				printf("%d %d\t%d %d %d\n", i, j, r, g, b);
		}
		printf(".");
	}

	fclose(input);
	return 0;
}
