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

/* Compute offsets in charstring programs for Adobe Minion */
/* Based on AFM for Minion Regular and AFM for reconstructed font */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXFILENAME 128

#define MAXLINE 256

#define MAXCHRS 512

#define MAXCHARNAME 32

char line[MAXLINE];

int charindex=0;

char charnames[MAXCHRS][MAXCHARNAME];

int widths[MAXCHRS];

int chrcodes[MAXCHRS];

int lsbs[MAXCHRS];

int lookup (char *charname) {
	int k;
	for (k = 0 ; k < charindex; k++) {
		if (strcmp(charnames[k], charname) == 0) return k;
	}
	return -1;
}

int main(int argc, char *argv[]) {
	char filename[MAXFILENAME];
	FILE *input;
	int chrcode, width, lsb, k;
	char charname[MAXCHARNAME];

	if (argc < 3) exit(1);
	strcpy (filename, argv[1]);
	if ((input = fopen(filename, "r")) == NULL) {
		perror(filename); exit(1);
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "EndCharMetrics", 14) == 0) break;
		if (sscanf(line, "C %d ; WX %d ; N %s ; B %d",
			&chrcode, &width, charname, &lsb) < 4) {
			fputs(line, stderr);
		}
		else {
			chrcodes[charindex] = chrcode;
			widths[charindex] = width;
			strcpy(charnames[charindex], charname);
			lsbs[charindex] = lsb;
			charindex++;
		}
	}
	fclose(input);

	strcpy (filename, argv[2]);
	if ((input = fopen(filename, "r")) == NULL) {
		perror(filename); exit(1);
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "EndCharMetrics", 14) == 0) break;
		if (sscanf(line, "C %d ; WX %d ; N %s ; B %d",
			&chrcode, &width, charname, &lsb) < 4) {
			fputs(line, stderr);
		}
		else {
			if ((k = lookup(charname)) < 0) {
				fputs(line, stderr);
			}
			else {
				printf("%d\t%s\t%% offset %d 0\n",
					chrcode, charname, lsbs[k] - lsb);
			}
		}
	}
	fclose(input);

	return 0;
}
