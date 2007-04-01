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

/* #define FNAMELEN 128 */

int verboseflag=0;

int leastfirst=1;

long filelength;

long psoffset;
long pslength;
long mfoffset;
long mflength;
long tiffoffset;
long tifflength;

char *copyright = "\
Copyright (C) 1993  Y&Y, Inc. All rights reserved. (978) 371-3286\
";

unsigned long ureadfour(FILE *input) {
	unsigned long c, d, e, f;
	c = (unsigned long) getc(input);
	d = (unsigned long) getc(input);
	e = (unsigned long) getc(input);
	f = (unsigned long) getc(input);	
	if (c == EOF || d == EOF || e == EOF || f == EOF) {
		fprintf(stderr, "Unexpected EOF\n");
		exit(1);
/*		return 0; */
	}
	if (leastfirst != 0) return ((((((f << 8) | e) << 8) | d) << 8) | c);
	else return ((((((c << 8) | d) << 8) | e) << 8) | f);
}

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	int c, d, e, f, m;
	long k;
	int firstarg=1;

	if (argc < 2) {
		printf(
	"Program to convert `return' to `newline' in PS, EPS, and EPSF files\n");
		printf("Specify file name(s) on command line\n");	
		exit(1);
	}
/*	while (argv[firstarg][0] == '-') */
	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) {
			verboseflag++;
			firstarg++;
		}
	}
	if (argc < firstarg+1) exit(1);

	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		extension(infilename, "eps");
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
/*			exit(2); */
			continue;
		}
		strcpy(outfilename, stripname(argv[m]));
		forceexten(outfilename, "new");
		if ((output = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename);
/*			exit(2); */
			fclose(input);
			continue;
		}

		printf("Changing line terminators:  %s => %s\n",
			infilename, outfilename);
		fseek (input, 0, SEEK_END);			/* go to end */
		filelength = ftell(input);			/* read length */
		fseek (input, 0, SEEK_SET);			/* rewind */
		
		tiffoffset = 0;
		c = getc(input); d = getc(input);
		if ((c == 'I' && d == 'I') || (c == 'M' && d == 'M')) {
			fprintf(stderr, "This is a TIFF file, not an EPSF file\n");
/*			exit(1); */
			continue;
		}
		else if (c == '%' && d == '!') {	/* plain EPS file */
			psoffset = 0;
			pslength = filelength;
		}
		else {
			e = getc(input); f = getc(input);
			if (c == 'E'+128 && d == 'P'+128 && e == 'S'+128 && f == 'F'+128) {
/* read PS-start, PS-length, MF-start, MF-end */
				leastfirst = 1;
				psoffset = ureadfour(input);
				pslength = ureadfour(input);
				mfoffset = ureadfour(input);
				mflength = ureadfour(input);
				tiffoffset = ureadfour(input);	/* read TIFF start offset */
				tifflength = ureadfour(input);	/* read TIFF length */
/*				(void) ureadtwo(input); */				/* should be 255, 255 */
			}
			else {
				fprintf(stderr,
					"Apparently %s is not a valid PS, EPS, or EPSF file\n",
						infilename);
/*				exit(1); */
				continue;
			}
		}
		if (psoffset > 0 || pslength < filelength)
		printf("PostScript part runs from %ld to %ld in file of %ld bytes\n",
				psoffset, psoffset + pslength, filelength);
		fseek (input, 0, SEEK_SET);
		if (psoffset > 0) 
			for (k = 0; k < psoffset; k++) putc(getc(input), output);
		for (k = 0; k < pslength; k++) {
			c = getc(input);
			if (c == '\r') {
				d = getc(input);
				if (d != '\n') c = '\n';
				ungetc(d, input);
			}
			putc(c, output);
		}
		if (pslength < filelength)
			while ((c = getc(input)) != EOF) putc(c, output);
		fclose(output);
		fclose(input);
	}
	return 0;
}

/* NOTE: if offset is used directly for value */
/* then value is packed left justified */
/* so above is only `correct' if type is `II' - not if type is `MM' */

/* allow for 128 MacBinary header maybe ? */

/* Show `ColorMap' info in special form ? */

/* show `RowsPerStrip' in special form when (unsigned) -1 ? */
