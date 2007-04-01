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

#define MAXLINE 256

char line[MAXLINE];

int near(int xold, int yold, int xnew, int ynew) {
	if (xold-1 <= xnew && xnew <= xold+1 &&
		yold-1 <= ynew && ynew <= yold+1) return 1;
	else return 0;
}

void trimxy(FILE *output, FILE *input) {
	int inflag=0;
	int xold=0, yold=0, xnew=0, ynew=0;
	int xa, ya, xb, yb, xc, yc;
	while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
		if (*line == ']') break;
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == ']' || *line == '\n' || *line == '%' || *line == 'h' ||
			strchr(line, '%') != NULL) {
			fputs(line, output);
			inflag = 0;
		}
		else {
			if (strstr(line, " m") != NULL) {
				sscanf(line, "%d %d", &xnew, &ynew);
				fputs(line, output);
			}
			else if (strstr(line, " l") != NULL) {
				sscanf(line, "%d %d", &xnew, &ynew);
				if (!near(xnew, ynew, xold, yold)) fputs(line, output);
			}
			else if (strstr(line, " c") != NULL) {
				sscanf(line, "%d %d %d %d %d %d", &xa, &ya, &xb, &yb, &xc, &yc);
				xnew = xc; ynew = yc;
				if (!near(xnew, ynew, xold, yold)) fputs(line, output);
			}
			inflag = 1;
			xold = xnew; yold = ynew;
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

int main(int argc, char *argv[]) {
	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	FILE *input, *output;
	int m;
	if (argc < 2) exit(1);
	for (m=1; m <argc; m++) {
		strcpy(infile, argv[m]);
		extension(infile, "out");
		if ((input = fopen(infile, "r")) == NULL) {
			perror(infile);
		}
		strcpy(outfile, infile);
		forceexten(outfile, "trm");
		if ((output = fopen(outfile, "w")) == NULL) {
			perror(outfile);
		}
		trimxy(output, input);
		fclose(output);
		fclose(input);
	}	
	return 0;
}
