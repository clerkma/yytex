/* Copyright 1990, 1991, 1992 Y&Y, Inc.
   Copyright 2007 TeX Users Group

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

/* usage: pcxdecode <pcx-file> */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <conio.h>

/* #define FNAMELEN 80 */

int verboseflag=1;
int traceflag=0;

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

int palette[48];	/* header palette 16 triples of one-byte values */

int version, encoding, bitsperpixel, planes, bytesperline;

int xmin, ymin, xmax, ymax, xvideo, yvideo;

int xres, yres, xsize, ysize, reserved, paletteform;

long pcxlength;

double compress;

int readword(FILE *input) {
	int c, d;
	c = getc(input); d = getc(input);
	return ((d << 8) | c);
}

int readheader(FILE *input) {
	int c, k;
	
	c = getc(input);
	if (c != 10) {
		fprintf(stderr, "Not a PCX file\n"); return -1;
	}
	version = getc(input);
	encoding = getc(input);
	if (encoding != 1) {
		fprintf(stderr, "Maybe not a valid PCX file\n"); 
	}
	bitsperpixel = getc(input);
	if (bitsperpixel != 1 && bitsperpixel != 2 &&
		bitsperpixel != 4 && bitsperpixel != 8) {
		fprintf(stderr, "Maybe not a valid PCX file\n"); 
	}
	xmin = readword(input); 	ymin = readword(input);
	xmax = readword(input); 	ymax = readword(input);	
	xsize = xmax - xmin + 1;	ysize = ymax - ymin + 1;
	xres = readword(input);     yres = readword(input);
	for (k = 0; k < 48; k++) 
		palette[k] = getc(input);	/* read header palette */
	reserved = getc(input);
	if (reserved != 0) {
		fprintf(stderr, "Not a PCX file\n"); return -1;
	}
	planes = getc(input);
	bytesperline = readword(input);
	if ((bytesperline & 1) != 0) {
		fprintf(stderr, "Maybe not a valid PCX file\n"); 
	}
	paletteform = readword(input);
	xvideo = readword(input);	
	yvideo = readword(input);	
	for (k = 0; k < 54; k++) (void) getc(input);	/* skip blanks */
	return 0;
}

void showheader() {
	printf ("PCX format version %d: ", version);
	switch (version) {
		case 0: printf("PC PaintBrush 2.5"); break;
		case 2: printf("PC PaintBrush 2.8 with palette"); break;
		case 3: printf("PC PaintBrush 2.8 without palette"); break;
		case 4: printf("PC PaintBrush for MS Windows"); break;
		case 5: printf("PC PaintBrush 3.0 etc"); break;
		default: printf("Unrecognized version number");
	}
	putc ('\n', stdout);
	if (encoding != 1) printf ("Encoding %d\n", encoding);
	if (planes != 1) 
		printf ("Bits per pixel per plane %d\n", bitsperpixel);
	else printf ("Bits per pixel %d\n", bitsperpixel);
	printf ("[%d %d %d %d]  ", xmin, ymin, xmax, ymax);
	printf ("xsize: %d ysize: %d  ", xsize, ysize);
	compress = (double) (pcxlength - 128) * 100 / ((double) xsize * ysize);
	compress = (double) ((int) (compress * 100.0)) / 100.0;
	printf("Compressed to %lg%%\n", compress);
	printf ("%d X-DPI %d Y-DPI\n", xres, yres);
	if (planes == 1) printf ("%d plane ", planes);
	else printf ("%d planes ", planes);
	if (planes != 1)
		printf ("%d bytes per line per plane\n", bytesperline);
	else printf ("%d bytes per line\n", bytesperline);
	if (paletteform != 0) {
	printf ("Palette interpretation %d: ", paletteform);
	switch (paletteform) {
		case 0: break;
		case 1: printf("Color/Monochrome");
		case 2: printf("GreyScale");		
		default: printf("Unrecognized palette interpretation");
	}
	putc ('\n', stdout);
	}
	if (xvideo != 0 || yvideo != 0)
		printf ("%d x pixel %d y pixel\n", xvideo+1, yvideo+1);
}

int readpixels (FILE *input, int xsiz, int ysize) {
	int m, n, l, c, r;
	long filepos = 128;	/* after header */
	int column;

	for (m = 0; m < ysize; m++) {	/* for each scan line */
		for (n = 0; n < planes; n++) {	/* for each plane of the scan line */
			column = 0;					/* right place ? */
			while (column < bytesperline) {
				c = getc(input); 
				if (++filepos > pcxlength) {
					fprintf(stderr, "Ran off end of file\n");
					return -1;
				}
				if ((c & 192) == 192) {	/* is it run-length ? */
					r = c & ~192;
					c = getc(input);	/* get data item */
					for (l = 0; l < r; l++) {	/* replicate c, r times */
						column++;
					}
				}
				else {	/* not run-length, c is data item itself */
					column++;
				}
			}
		}
	}
	return 0;
}

int main(int argc, char *argv[]) { 
    FILE *fp_in, *fp_out;
	char fn_in[filename_max], fn_out[filename_max];
	int firstarg=1, m;

	for (m = firstarg; m < argc; m++) {

	strcpy(fn_in, argv[m]);
	extension(fn_in, "pcx");
	if (verboseflag != 0) printf("File:  %s ", fn_in);
	if((fp_in = fopen(fn_in, "rb")) == NULL) {
		perror(fn_in); exit(1);
	}
	fseek (fp_in, 0, SEEK_END);
	pcxlength = ftell (fp_in);
	rewind (fp_in);
	if (verboseflag != 0) printf("%ld bytes\n", pcxlength);

	if (readheader(fp_in) != 0) exit(3);
	if (verboseflag != 0) showheader();
/*	(void) readpixels (fp_in, xvideo, yvideo); */
	(void) readpixels (fp_in, xsize, ysize);

	fclose(fp_in);
	if (verboseflag != 0) putc('\n', stdout);
	}
	return 0;
}
