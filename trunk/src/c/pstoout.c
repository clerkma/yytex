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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* #define FNAMELEN 80 */
#define FONTNAME_MAX 64

#define MAXLINE 256

int verboseflag=0;
int traceflag=0;
int roundflag=0;
int truncateflag=0;
int allowrunflag=1;
int traceimage=1;
int pkcompensate=0;				/* compensate for PKtoPS problem */

int numchar=0;					/* number of characters in font */
double ptsize=0.0;				/* pt size from file */
double hppp=0, vppp=0;			/* pxl/pt */
double size=10.0, scale=1.0;	/* default for OUT format file */
double xres, yres;				/* resolution (pixel/inch) */
int xll=0, yll=-300, xur=1000, yur=1000;
int chr;

char fontname[FONTNAME_MAX];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double fround (double s) {
	int m;
	m = (int) (s+0.49999999);
	return (double) m;
}

double ftruncate (double s) {
	int m;
	m = (int) s;
	return (double) m;
}

int convert(int c) {
	int d=0;
	if (c >= '0' && c <= '9') d = c - '0';
	else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
	else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
	else {
		printf("%d (%c)\n", c, c);	/* debugging */
		exit(1);
	}
	return d;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define MAXROW 100 */
#define MAXROW 200
#define MAXCOLUMN 100

/* int image[MAXROW][MAXCOLUMN]; */
char image[MAXROW][MAXCOLUMN];

/* int marked[MAXROW][MAXCOLUMN]; */
char marked[MAXROW][MAXCOLUMN];

int newcontour=1;

double fxll=0.0, fyur=0.0;

int imax=0, jmax=0;

void emit(FILE *output, double i, double j) {
	fprintf(output, "%lg %lg ", fxll + j, fyur - i);
	if (newcontour) fputs("m\n", output);
	else fputs("l\n", output);
/*	newcontour = 0; */
}

#define SOUTH 1
#define EAST 2
#define NORTH 4
#define WEST 8

/* we assume we start with  0 1 pattern, heading south i,j on 1 */

void traceout (FILE *output, int i, int j) {
	int istart=i, jstart=j;

	newcontour=1;
	emit(output, i-0.5, j-0.5);

south:
	if (newcontour) newcontour = 0;
	if (marked[i][j] & SOUTH) return;
	marked[i][j] |= SOUTH;
	if (image[i+1][j] == 1) {
		if (image[i+1][j-1] == 0) {
			i = i + 1;
			goto south;
		}
		else {
			emit (output, i+0.5, j-0.5);
			i = i + 1; j = j - 1;
			goto west;
		}
	}
	else {
		if (image[i+1][j-1] == 0) {
			emit (output, i+0.5, j-0.5);
			goto east;	/* same i,j */
		}
		else {
			emit (output, i+0.5, j-0.5);
			i = i + 1; j = j - 1;
			goto west;
		}
	}

east:
	if (marked[i][j] & EAST) return;
	marked[i][j] |= EAST;
	if (image[i][j+1] == 1) {
		if (image[i+1][j+1] == 0) {
			j = j + 1;
			goto east;
		}
		else {
			emit (output, i+0.5, j+0.5);
			i = i+1; j = j+1;
			goto south;
		}
	}
	else {
		if (image[i+1][j+1] == 0) {
			emit (output, i+0.5, j+0.5);
			goto north;	/* same i,j */
		}
		else {
			emit (output, i+0.5, j+0.5);
			i = i+1; j = j+1;
			goto south;
		}
	
	}

north:
	if (marked[i][j] & NORTH) return;
	marked[i][j] |= NORTH;
	if (image[i-1][j] == 1) {
		if (image[i-1][j+1] == 0) {
			i = i - 1;
			goto north;
		}
		else {
			emit (output, i-0.5, j+0.5);
			i = i - 1; j = j + 1;
			goto east;
		}
	}
	else {
		if (image[i-1][j+1] == 0) {
			emit (output, i-0.5, j+0.5);
			goto west;	/* same i,j */
		}
		else {
			emit (output, i-0.5, j+0.5);
			i = i - 1; j = j + 1;
			goto east;
		}
	}

west:
	if (marked[i][j] & WEST) return;
	marked[i][j] |= WEST;
	if (image[i][j-1] == 1) {
		if (image[i-1][j-1] == 0) {
			j = j - 1;
			goto west;
		}
		else {
			emit (output, i-0.5, j-0.5);
			i = i - 1; j = j - 1;
			goto north;
		}
	}
	else {
		if (image[i-1][j-1] == 0) {
			emit (output, i-0.5, j-0.5);
			goto south;	/* same i,j */
		}
		else {
			emit (output, i-0.5, j-0.5);
			i = i - 1; j = j - 1;
			goto north;
		}
	}

}

void scanarray (FILE *output, int imax, int jmax) {
	int i, j;
	for (i=0; i < imax; i++) {
		for (j=0; j < jmax; j++) {
			if (image[i][j] == 0) continue;	/* ignore zeros */
/*			if (marked[i][j] != 0) { */
			if ((marked[i][j] & SOUTH) != 0) {	/* already marked ? */
				while (image[i][j] != 0) j++;
			}
			else {
				traceout(output, i, j);	/* start south from here */
				fputs("h\n", output);
				while (image[i][j] != 0) j++;
			}
		}
	}	
}

void scandata (FILE *input) {
	int i=1, j=1;
	int c, k;

	imax = 1; jmax = 1;
	c = getc(input);
	if (c == '<') c = getc(input);
	while (c <= ' ') c = getc(input);
	for (;;) {
		while (c > ' ') {
			c = convert(c);
			for (k=0; k < 4; k++) {
				if (c & 8) image[i][j] = 1;
				c = c << 1;
				j = j + 1;
				if (j >= MAXCOLUMN) return;
				if (j > jmax) jmax = j;
			}			
			c = getc(input);
		}
		while (c <= ' ') {
			if (c == EOF) return;
			c = getc(input);
		}
		if (c == '>') break;
		j = 1;
		i = i + 1;
		if (i > imax) imax = i;
		if (i >= MAXROW) return;
	}
}

void resetarrays (int imax, int jmax) {
	int i,j;
	for (i=0; i < imax; i++) {
		for (j=0; j < jmax; j++) {
			image[i][j] = 0;
			marked[i][j] = 0;
		}
	}
}

void scannewchar (FILE *output, FILE *input, int xoff, int yoff) {
/*	fxll = xoff+0.5; */
	fxll = xoff-0.5;
	if (pkcompensate) fyur = yoff+1.5; 
	else fyur = yoff+0.5; 
	resetarrays(MAXROW, MAXCOLUMN);
	if (traceflag) printf("After reset\n");
	scandata(input);
	if (traceflag) printf("imax %d jmax %d\n", imax, jmax);
/*	scanarray(output, MAXROW, MAXCOLUMN); */
	scanarray(output, imax+1, jmax+1);
	if (traceflag) printf("After scanarray\n");
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* output single block */

#ifdef IGNORED
void block(FILE *output, int column, int row, int width) {
	fprintf (output, "%d %d m\n", column, row);
	fprintf (output, "%d %d l\n", column, row-1);
	fprintf (output, "%d %d l\n", column+width, row-1);
	fprintf (output, "%d %d l\n", column+width, row);
	fprintf (output, "%d %d l\n", column, row);
	fprintf (output, "h\n", column, row);
}
#endif

void block(FILE *output, int column, int row, int width) {
	fprintf (output, "%d %d m\n", column, row+1);
	fprintf (output, "%d %d l\n", column, row);
	fprintf (output, "%d %d l\n", column+width, row);
	fprintf (output, "%d %d l\n", column+width, row+1);
	fprintf (output, "%d %d l\n", column, row+1);
	fprintf (output, "h\n", column, row+1);
}

/* convert bit map for single character - isolated bits only */
/* assume positioned after /a... and before < */
/* ini_row and ini_column att top left of image */

int scanchar (FILE *output, FILE *input, int ini_column, int ini_row) {
	int c, d, k, row, column;
	int start=0, width=0, advance=0;
	
	c = getc(input);
	while (c != '<' && c != EOF) c = getc(input);
	if (c == EOF) return EOF;
	c = getc(input);
	while (c <= ' ' && c != EOF) c = getc(input);
	if (c == EOF) return EOF;
	row = ini_row;
/*	printf("%d %d (%c)\n", chr, c, c); */	/* debugging */
	for (;;) {
		column = ini_column;
		while (c > ' ') {
			c = convert(c);
			if (allowrunflag) {				/* special case run of ones */
				if (c == 1 || c == 3 || c == 7 || c == 15) {
					if (c == 1) {start = 3; width=1;}
					else if (c == 3) {start = 2; width=2;}
					else if (c == 7) {start = 1; width=3;}
					else if (c == 15) {start = 0; width=4;}
					advance = 4;
					d = getc(input);
					while (d == 'F') {
						width += 4;
						advance += 4;
						d = getc(input);
					}
					if (d <= ' ') {
						block(output, column + start, row, width);
						column = column + advance;
						break;
					}
					d = convert(d);
					if (d == 8 || d == 12 || d == 14 || d == 15) {
						if (d == 8) width += 1;
						else if (d == 12) width += 2;
						else if (d == 14) width += 3;
						else if (d == 15) width += 4;
						block(output, column+start, row, width);
/*						column = column + advance; */
						column = column + advance + 4;
						c = getc(input);
						continue;
					}
					else {	/* not special case, drop through */
						block(output, column + start, row, width);
						column = column + advance;
						c = d;
					}
				}
			}
			for (k = 0; k < 4; k++) {
				if ((c & 15) == 15) {
					block(output, column, row, 4);
					column += 4-k;
					break;
				}
				if ((c & 15) == 14) {
					block(output, column, row, 3);
					column += 4-k;
					break;
				}
				if ((c & 15) == 12) {
					block(output, column, row, 2);
					column += 4-k;
					break;
				}
				if ((c & 15) == 8) {
					block(output, column, row, 1);
					column += 4-k;
					break;
				}
				if (c & 8) block(output, column, row, 1);
				c = c << 1;
				column++;
			}
			c = getc(input);
		}
		while (c <= ' ' && c != EOF) c = getc(input);		
		if (c == EOF) return EOF;
		if (c == '>') break;
		row--;
	}
	return 0;
}

void scantopk(FILE *output, FILE *input) {
	char line[MAXLINE];

	while (fgets(line, sizeof(line), input) != NULL) {
		if (strncmp(line, "/numchar ", 9) == 0)
			sscanf(line+9, "%d", &numchar);
		if (strncmp(line, "/ptsize ", 8) == 0)
			sscanf(line+8, "%lg", &ptsize);
		if (strncmp(line, "/hppp ", 6) == 0)
			sscanf(line+6, "%lg", &hppp);
		if (strncmp(line, "/vppp ", 6) == 0)
			sscanf(line+6, "%lg", &vppp);
		if (strncmp(line, "dup begin", 9) == 0) break;
	}
	xres = 72.27 * hppp;
	yres = 72.27 * hppp;
	printf("%s\tdesign size %lg\t xres %lg yres %lg\n",
		   fontname, ptsize, xres, yres);
/*	10.0000 1.00 % HPBATS */
	fprintf (output, "%lg %lg %% %s\n", size, scale, fontname);
/*	135 -25 855 695 */
	fprintf (output, "%d %d %d %d\n", xll, yll, xur, yur);
}

void dopkfile(FILE *output, FILE *input) {
	char line[MAXLINE];
	int chr; 
	int w, h, hoff, voff;
	double width, swidth;
	double xscale=1.0, yscale=1.0;

/*	Want 1000 Adobe units for 10pt font */
	if (hppp != 0.0) xscale = 1000.0 / (ptsize * hppp);
	if (vppp != 0.0) yscale = 1000.0 / (ptsize * vppp);
	if (roundflag) {
		xscale = fround (xscale);
		yscale = fround (yscale);
	}
	else if (truncateflag) {
		xscale = ftruncate (xscale);
		yscale = ftruncate (yscale);
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '/') {
/* /a77 125 7 53 -1 0 < ... > 77 makechar */
/* name, width, w, h, hoff, voff, bitmap, chr */
			if (traceflag) printf(line);
			if (sscanf (line, "/a%d %lg %d %d %d %d",
						&chr, &width, &w, &h, &hoff, &voff) == 6) {
				fputs("]\n", output);
				if (xscale != 1.0 || yscale != 1.0) {
					swidth = width / xscale;
					fprintf(output, "%d %lg %% a%d %% scale %lg %lg\n",
						chr, swidth, chr, xscale, yscale);
				}
				else fprintf(output, "%d %lg %% a%d\n", chr, width, chr);
/*				scanchar (output, input, hoff, voff);  */
				if (traceimage == 0) scanchar (output, input, -hoff, voff); 
				else scannewchar (output, input, -hoff, voff); 
			}
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int main(int argc, char *argv[]) {
	char infilename[FILENAME_MAX];
	char outfilename[FILENAME_MAX];
	FILE *input;
	FILE *output;
	char *s;
	int firstarg=1;

	while (firstarg < argc && argv[firstarg][0] == '-')	{
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag=1;
		if (strcmp(argv[firstarg], "-t") == 0) traceflag=1;
		if (strcmp(argv[firstarg], "-r") == 0) roundflag=1;
		if (strcmp(argv[firstarg], "-f") == 0) truncateflag=1;
		firstarg++;
	}
	if (argc < firstarg+1) exit(1);
	strcpy(infilename, argv[firstarg]);
	extension(infilename, "ps");
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);
		exit(1);
	}
	strcpy(outfilename, removepath(infilename));
	forceexten(outfilename, "out");
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);
		exit(1);
	}
	strcpy(fontname, outfilename);
	if ((s = strchr(fontname, '.')) != NULL) *s = '\0';
	scantopk(output, input);
	dopkfile(output, input);
	fclose(output);
	fclose(input);
	return 0;
}

