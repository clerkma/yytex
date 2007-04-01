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

/* slant data in `out' file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 256
#define MAXNAME 128

double slant=0.25;		/* positive to the right */

/* double xscale=0.62; */
double xscale=0.65;
double yscale=0.88;

/* double scaled=1.105; */
double scaled=1.118;

/* transform type 1 - slant by given amount (tangent of angle) */
/* transform type 2 - unslant, flip sign of x, slant */
/* transform type 3 - scale x and scale y down */
/* transform type 4 - scale x and scale y up */

int transfrmtype=4;

int chr, wx;
int xoffset, yoffset;
char charname[MAXNAME];

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


int xtransfrm (int x, int y) {
/*	simple slant */
	x += xoffset, y += yoffset;
	switch (transfrmtype) {
		case 1:
			return x + (int) (slant * y); 
/*	unslant, flip sign of x and reslant and shift by advance width */
		case 2:
			return wx - x + (int) (2 * slant * y);
		case 3:
			return (int) (xscale * yscale * x);
		case 4:
			return (int) (scaled * x);
	}
}

int ytransfrm (int x, int y) {
	x += xoffset, y += yoffset;
	switch (transfrmtype) {
		case 1:
		case 2:
			return y;
		case 3:
			return (int) (yscale * (y - 21)) + 21;
		case 4:
			return (int) (scaled * (y - 21));
	}
}

void slantline (char *line) {
	int x, y, x1, y1, x2, y2, x3, y3;
	char *s;
	
	if (*line == '%') return;
	if (*line < ' ') return;
	if (*line == ']') return;
	if (*line == 'h') return;
	if (strstr(line, " c") != NULL &&
		sscanf(line, "%d %d %d %d %d %d c", &x1, &y1, &x2, &y2, &x3, &y3)
		== 6) {
		x = xtransfrm(x1, y1);
		y = ytransfrm(x1, y1);
		x1 = x; y1 = y;
		x = xtransfrm(x2, y2);
		y = ytransfrm(x2, y2);
		x2 = x; y2 = y;
		x = xtransfrm(x3, y3);
		y = ytransfrm(x3, y3);
		x3 = x; y3 = y;
		sprintf(line, "%d %d %d %d %d %d c\n", x1, y1, x2, y2, x3, y3);
	}
	else if (strstr(line, " m") != NULL &&
			 sscanf(line, "%d %d m", &x1, &y1) == 2) {
		if ((s = strstr(line, "offset")) != NULL) {
			sscanf(s+7, "%d %d", &xoffset, &yoffset);
		}
		x = xtransfrm(x1, y1);
		y = ytransfrm(x1, y1);
		x1 = x; y1 = y;
		sprintf(line, "%d %d m\n", x1, y1);
	}
	else if (strstr(line, " l") != NULL &&
			 sscanf(line, "%d %d l", &x1, &y1) == 2) {
		x = xtransfrm(x1, y1);
		y = ytransfrm(x1, y1);
		x1 = x; y1 = y;
		sprintf(line, "%d %d l\n", x1, y1);
	}
}

void slantoutlines (FILE *output, FILE *input) {
	char line[MAXLINE];
	int wxn;
	
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%') {
/*			fputs(line, output); */
			continue;
		}
		if (*line == ']') {
			fputs(line, output);
			xoffset = yoffset = 0;
			fgets(line, sizeof(line), input);
			while (*line == '%') {
/*				fputs(line, output); */
				fgets(line, sizeof(line), input);
			}
			sscanf (line, "%d %d %% %s", &chr, &wx, charname);
			wxn = xtransfrm(wx, 0);
			sprintf (line, "%d %d %% %s\n", chr, wxn, charname);
			fputs(line, output);
			continue;
		}
		slantline(line);
		fputs(line, output);
	}
}

int main (int argc, char *argv[]) {
	char infile[FILENAME_MAX];
	char outfile[FILENAME_MAX];
	FILE *input, *output;

/*	slantout <foo>.out 0.25 */

/*	if (argc < 3) exit(1); */
	if (argc < 2) {
		fprintf(stderr, "Missing argument\n");
		exit(1);
	}
	if (argc > 2) {
		if (sscanf(argv[2], "%lg", &slant) < 1) {
			printf("slant %s?\n", argv[2]);
			exit(1);
		}
	}
	strcpy (infile, argv[1]);
	extension(infile, "out");
	strcpy (outfile, argv[1]);
	forceexten(outfile, "slt");
	if ((input = fopen(infile, "r")) == NULL) {
		perror(infile);
		exit(1);
	}
	if ((output = fopen(outfile, "w")) == NULL) {
		perror(outfile);
		exit(1);
	}
	slantoutlines(output, input);
	fclose(input);
	fclose(output);
	return 0;
}
