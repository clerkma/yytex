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

#define MAXLINE 128

int main (int argc, char *argv[]) {
	FILE *input;
	char *s;
	char line[MAXLINE];
	int n;

	if (argc < 2) exit(1);
/*	if ((input = fopen (argv[1], "r")) == NULL) { */
	if ((input = fopen (argv[1], "rb")) == NULL) {
		perror(argv[1]);
		exit(1);
	}

	*line = '\0';
	for (;;) {
		s = fgets(line, MAXLINE, input);
		n = strlen(line);
		printf("**********************************************\n");
/*		if (s != line) printf("fgets returned other than line\n"); */
		if (s == NULL) {
			printf("fgets returned NULL\n");
			break;
		}
		printf("LINE: %s", line);
		if (strchr(line, '\n') == NULL) {
			putc('\n', stdout);
			printf("fgets returned line without newline\n");
		}
/*		if (strchr(line, '\r') == NULL)
			printf("fgets returned line without return\n"); */
		printf("fgets returned line of length %d\n", n);
	}
	fclose(input);
	return 0;

}
