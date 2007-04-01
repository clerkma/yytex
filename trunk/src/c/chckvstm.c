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
#include <string.h>
#include <stdlib.h>

/* check vstem outer stem match for Sun NeWSPRint */

#define MAXFILENAME 128
#define MAXLINE 256

int maxwidth=800;
int maxpos=1000;

int main(int argc, char *argv[]) {
	FILE *input;
	char filename[MAXFILENAME];
	char line[MAXLINE];
	char *s;
	int xs, xe;

	if (argc < 2) exit(1);
	strcpy(filename, argv[2]);
	if (strchr(filename, ".") == NULL) strcat(filename, ".hnt");
	if ((input = fopen(filename, "r")) == NULL) {
		perror(filename);
	}

	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) break;
		if (*line == '%' || *line == ';' || 
			*line == '\n' || *line == '/') continue;
		if (*line == 'C') {
			if ((s = strchr(line, "V")) != NULL) {
				s = s + 2;
				for (;;) {
					if (sscanf(s, "%d %d", xs, xe) < 2) break;
					if (xe - xs > maxwidth || xe > maxpos)
						puts (line);
				}
			}
		}

	}

	fclose(input);
	return 0;
}
