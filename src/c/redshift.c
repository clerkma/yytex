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

/* compare foo.afm with footex.afm to see italicfuzz */

/* help figure out italic correction adjustments */

/* usage: call with Ordinary AFM and TeX AFM as arguments */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 256

#define MAXCHARNAME 64

#define ITALICFUZZ 30

char line[MAXLINE];

void comparefiles(FILE *inord, FILE *intex) {
	int charord, chartex;
	char charnameord[MAXCHARNAME], charnametex[MAXCHARNAME];
	int wxord, wxtex;
	int xllord, yllord, xurord, yurord;
	int xlltex, ylltex, xurtex, yurtex;
	int italic;

	for(;;) {
		for (;;) {
			if (fgets(line, MAXLINE, inord) == NULL) return;
			if (*line != '%' && *line != ';' && *line != '\n') break;
		}
		if (strstr(line, "StartCharMetrics") != NULL) break;
	}

	for(;;) {
		for (;;) {
			if (fgets(line, MAXLINE, intex) == NULL) return;
			if (*line != '%' && *line != ';' && *line != '\n') break;
		}
		if (strstr(line, "StartCharMetrics") != NULL) break;
	}

	for(;;) {
		for (;;) {
			if (fgets(line, MAXLINE, inord) == NULL) return;
			if (*line != '%' && *line != ';' && *line != '\n') break;
		}
		if (strstr(line, "EndCharMetrics") != NULL) return;
		sscanf(line, "C %d ; WX %d ; N %s ; B %d %d %d %d ;",
			&charord, &wxord, charnameord, &xllord, &yllord, &xurord, &yurord);
		for (;;) {
			if (fgets(line, MAXLINE, intex) == NULL) return;
			if (*line != '%' && *line != ';' && *line != '\n') break;
		}
		if (strstr(line, "EndCharMetrics") != NULL) return;
		sscanf(line, "C %d ; WX %d ; N %s ; B %d %d %d %d ;",
			&chartex, &wxtex, charnametex, &xlltex, &ylltex, &xurtex, &yurtex);
/*		if (charord != chartex) { */
		if (charord != chartex && charord >= 0 && chartex >= 0) {
			fputs(line, stderr);
			fprintf(stderr, "ERROR: unsynchronized %d != %d\n",
				charord, chartex);
			exit(1);
		}
		if (wxord != wxtex) {
			fputs(line, stderr);
			fprintf(stderr, "ERROR: width mismatch %d != %d\n",
				wxord, wxtex);
			exit(1);
		}
		if (strcmp(charnameord, charnametex) != 0) {
			fputs(line, stderr);
			fprintf(stderr, "ERROR: name mismatch %s != %s\n",
				charnameord, charnametex);
			exit(1);
		}
		italic = xurtex - wxord + ITALICFUZZ;
		if (italic < 0) italic = 0;
		if (xurtex == xurord) {
			printf("C %d ; WX %d ; N %s ; I %d ;\tIDENTICAL\n",
				charord, wxord, charnameord, italic);
		}
		else {
			printf(
			"C %d ; WX %d ; N %s ; I %d ;\ttex-ord: %d\txur-wx: %d\n",
					charord, wxord, charnameord, italic, xurtex - xurord,
						xurtex - wxord);
		}
	}
}

void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

int main (int argc, char *argv[]) {
	FILE *inord, *intex;
	if (argc < 3) exit(1);
	if ((inord = fopen(argv[1], "r")) == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if ((intex = fopen(argv[2], "r")) == NULL) {
		perror(argv[2]);
		exit(1);
	}
	comparefiles(inord, intex);
	fclose(intex);
	fclose(inord);
	return 0;
}
