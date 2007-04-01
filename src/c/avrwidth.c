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

/* Get average width of characters in a font based on AFM */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char line[512];

int main (int argc, char *argv[]) {
	FILE *input;
	int count=0;
	long total=0;
	int chr, width;
	int m;
	
	if (argc < 2) exit(1);
	for (m = 1; m < argc; m++) {
		if ((input = fopen(argv[m], "r")) == NULL) {
			perror (argv[m]);
			exit(1);
		}
		while (fgets(line, sizeof(line), input) != NULL) {
			if (strncmp(line, "StartCharMetrics", 16) == 0) break;
		}
		while (fgets(line, sizeof(line), input) != NULL) {
			if (strncmp(line, "EndCharMetrics", 14) == 0) break;
			if (strncmp(line, "C ", 2) != 0) continue;
			if (sscanf(line, "C %d ; WX %d ;", &chr, &width) == 2) {
				if ((chr >= 'A' && chr <= 'Z') ||
					(chr >= 'a' && chr <= 'z')) {
					total += width;
					count++;
				}
			}
			else printf("BAD %s", line);
		}
		printf("Total width for font %s is %ld over %d chars (average %d)\n",
			   argv[1], total, count, (int) (total / count));
		fclose(input);
	}
	return 0;
}
