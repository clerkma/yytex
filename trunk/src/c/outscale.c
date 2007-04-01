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

/* used for scaling mis-scaled outline font files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXFILENAME 80
#define MAXLINE 256

int num=723;
int den=653;

/*	else return (int) ((((long) z * 723) + 326) / 653); */

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

int rescale (int z) {
	if (z < 0) return -rescale(-z);
/*	else return (int) ((((long) z * 723) + 326) / 653); */
	else return (int) ((((long) z * num) + den/2) / den);
}

int main(int argc, char *argv[]) {
	char infile[MAXFILENAME], outfile[MAXFILENAME];
	FILE *input, *output;
	char line[MAXLINE];
	int x, y, xa, ya, xb, yb, xc, yc;
	int chr, width;
	char charname[MAXLINE];

	if (argc < 2) {
		printf("Usage: %s <outlines> [<num> [<den>]]\n", argv[0]);
		exit(1);
	}
	if (argc > 2) {
		num=atoi(argv[2]);
		if (argc > 3) den=atoi(argv[3]);
		else den = 1;
		printf ("Scale %d/%d\n", num, den);
	}
	strcpy(infile, argv[1]);
	extension(infile, "out");
	if ((input = fopen(infile, "r")) == NULL) {
		perror(infile);
		exit(1);
	}
	strcpy(outfile, argv[1]);
	forceexten(outfile, "scl");
	if ((output = fopen(outfile, "w")) == NULL) {
		perror(outfile);
		exit(1);
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strchr(line, ']') != NULL) {
		}
		else if (*line == '\n') {
		}
		else if (strchr(line, 'h') != NULL) {
		}
		else if (strchr(line, '%') != NULL) {
			if (sscanf(line, "%d %d %% %s", &chr, &width, &charname) < 3) {
				fputs(line, stderr);
			}
			else {
				width = rescale (width);
				sprintf(line, "%d %d %% %s\n", chr, width, charname);
			}
		}
		else if (strchr(line, 'm') != NULL) {
			if (sscanf(line, "%d %d m", &x, &y) < 2) {
				fputs(line, stderr);
			}
			else {
				x = rescale (x);
				y = rescale (y);
				sprintf(line, "%d %d m\n", x, y);
			}
		}
		else if (strchr(line, 'l') != NULL) {
			if (sscanf(line, "%d %d l", &x, &y) < 2) {
				fputs(line, stderr);
			}
			else {
				x = rescale (x);
				y = rescale (y);
				sprintf(line, "%d %d l\n", x, y);
			}
		}
		else if (strchr(line, 'c') != NULL) {
			if (sscanf(line, "%d %d %d %d %d %d c",
				&xa, &ya, &xb, &yb, &xc, &yc) < 6) {
				fputs(line, stderr);
			}
			else {
				xa = rescale (xa);	ya = rescale (ya);
				xb = rescale (xb);	yb = rescale (yb);
				xc = rescale (xc);	yc = rescale (yc);
				sprintf(line, "%d %d %d %d %d %d c\n",
					xa, ya, xb, yb, xc, yc);
			}
		}
		else fputs(line, stderr);
		fputs(line, output);
	}
	fclose(output);
	fclose(input);
	return 0;
}







