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
#include <string.h>

#define MAXLINE 256

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char buffer[MAXLINE];
	char old[MAXLINE];
	if (argc < 2) exit(1);
	if ((input = fopen(argv[1], "r")) == NULL) {
		perror(argv[1]); exit(1);
	}
	strcpy(old, "");
	while (fgets(buffer, sizeof(buffer), input) != NULL) {
		if (strcmp(buffer, old) == 0) continue;
		fputs(buffer, stdout);
		strcpy (old, buffer);
	}
	fclose (input);
	return 0;
}
