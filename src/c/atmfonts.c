/* Copyright 1990, 1991, 1992 Y&Y
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Try and analyze ATMFONTS.QLC */

/* #define FNAMELEN 80 */

#define MAXFONTS 1024

#define MAXCACHE 256

#define WIDTHS 524

#define FILENAME 48

#define INFOSIZE 44

#define NAMESPACE 30

int verboseflag=0;

int checkflag=1;	/* compare cached info against PFM file */

int spewflag=0;		/* show bytes of 44 byte cache info */

int matrixflag=0;	/* show FontMatrix */

int infoflag=0;		/* show other stuff */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int ncache, nfonts;

int cacheindex[MAXFONTS];	/* for each font, where cache entry is */

int fontindex[MAXCACHE];	/* for each cache entry, which font it is */

char line[FILENAME+1];		/* input buffer */

long cachebase, infobase, pfbbase, pfmbase;

/* int italic, weight; */

void getbytes(FILE *input, char *s, int n) {
	int k;
	for (k = 0; k < n; k++) *s++ = (char) getc(input);
	*s = '\0';
}

static unsigned long ureadfour (FILE *input) {
	int c, d, e, f;
	c = getc(input);	d = getc(input);
	e = getc(input);    f = getc(input);
	return ((((((unsigned long) f << 8) | (unsigned long) e) << 8) | d) << 8) | c;
}

static unsigned long uscanfour (char *s) {
	int c, d, e, f;
	c = *s++ & 255;	d = *s++ & 255;
	e = *s++ & 255; f = *s++ & 255;
	return ((((((unsigned long) f << 8) | (unsigned long) e) << 8) | d) << 8) | c;
}

static unsigned int uscantwo (char *s) {
	int c, d;
	c = *s++ & 255;	d = *s++ & 255;
	return (d << 8) | c;
}

double fscale(long x) {
	if (x == 0) return 0.0;
	else return ((double) x + 0.5) / 1073741824.0;
}

double pi = 3.141592653;

int main(int argc, char *argv[]) {
	FILE *input, *pfminput;
!	char filename[FILENAME_MAX]="c:\\windows\\atmfonts.qlc";
/*	char pfmfilename[FILENAME_MAX]; */
	int c, d, e, f;
	int ch, fn;
	int k, i, n;
	int firstchar, lastchar, defaultchar;
	int badflag;
	long extenttable, pfmextension;
	long m11, m12, m21, m22;
	double fm11, fm12, fm21, fm22;
	int code, pitch, family, charset;
	int italicskew, kernpairs;
	int avewidth, maxwidth;
	double italicangle;

	if (argc > 1) strcpy(filename, argv[1]);
	extension(filename, "qlc");
	if ((input = fopen(filename, "rb")) == NULL) {
		perror(filename); exit(3);
	}
	
	for (k = 0; k < MAXCACHE; k++) fontindex[k] = -1;
	for (k = 0; k < MAXFONTS; k++) cacheindex[k] = -1;

	c = getc(input); d = getc(input);
	ncache = (d << 8) | (c & 255);
	c = getc(input); d = getc(input);
	nfonts = (d << 8) | (c & 255);
	printf("%d fonts and %d cache entry positions\n", nfonts, ncache);

	printf("\n");

	cachebase = 20;
	infobase = cachebase + (long) ncache * WIDTHS;
	pfbbase = infobase + (long) nfonts * INFOSIZE;
	pfmbase = pfbbase + (long) nfonts * FILENAME;
	
	for (k = 0; k < nfonts; k++) {
		fseek(input, infobase + (long) k * INFOSIZE, SEEK_SET);
		getbytes(input, line, INFOSIZE);
		ch = line[20];			/* cache index */
		cacheindex[k] = ch;
		fontindex[ch] = k;
/*		c = line[12]; d = line[13]; */
/*		if (c == 0 && d == 0) italic = 0; */
/*		else italic = (d << 8) | (c & 255); */
/*		c = line[24]; d = line[25]; */
		
		if (spewflag != 0 || matrixflag != 0 || infoflag != 0) {
			printf("Font %3d: ", k);
			fseek(input, pfmbase + (long) k * FILENAME, SEEK_SET);
			getbytes(input, line, FILENAME);
			printf("%s ", line);
			n = strlen(line);
			for (i = n; i < NAMESPACE; i++) putc(' ', stdout);
			fseek(input, infobase + (long) k * INFOSIZE, SEEK_SET);
			getbytes(input, line, INFOSIZE);
			if (infoflag != 0) {
				printf("first: %3d last: %3d default: %3d break: %3d ",
					line[36] & 255, line[37] & 255, 
						line[38] & 255, line[39] & 255);
				code = line[40];
				pitch = code & 15;
				family = code >> 4;
				if (family == 0) printf("Don't Care ");
				else if (family == 1) printf("Roman (Serif) ");
				else if (family == 2) printf ("Swiss (Sans serif) ");
				else if (family == 3) printf ("Modern (Fixed pitch) ");
				else if (family == 4)  printf ("Script (Cursive) ");
				else if (family == 5)  printf ("Decorative (NOT ANSI) ");
				else printf("UNKNOWN ");

				charset = line[41];
				if (charset == 0) printf("ANSI");
				else if (charset == 2) printf("Symbol");
				else if (charset == 128) printf("Kanji");
				else if (charset == 255) printf("OEM");
			}
			else if (matrixflag != 0) {
				m11 = uscanfour(line+4);
				m12 = uscanfour(line+8);
				m21 = uscanfour(line+12);
				m22 = uscanfour(line+16);
/* FontMatrix entries scaled by 2^30 */
				fm11 = fscale(m11);
				fm12 = fscale(m12);
				fm21 = fscale(m21);
				fm22 = fscale(m22);
				printf("[%lg %lg %lg %lg] ", fm11, fm12, fm21, fm22);
				avewidth = uscantwo(line+28);
				maxwidth = uscantwo(line+30);
				printf("avewidth %d maxwidth %d ", avewidth, maxwidth);
				italicskew = uscantwo(line+32);
				if (italicskew != 0) {
					italicangle = -atan((double) italicskew / 1000.0);
					printf("ItalicAngle %lg ", italicangle * 180.0 / pi);
				}
				kernpairs = uscantwo(line+34);
				if (kernpairs  != 0) {
					printf("%d kernpairs ", kernpairs);
				}
			}
			else if (spewflag != 0) {
				for (i = 0; i < 4; i++)	
					printf("%4d", line[i] & 255);
/* ignore FontMatrix, cache index, and MasterHeight (or whatever) */
				putc('\t', stdout);
				for (i = 4 + 16 + 2 + 2; i < INFOSIZE - 8 - 4 - 4; i++)
					printf("%4d", line[i] & 255);
			}
/* ignore first, last, default, break, pitchandfamily, charset */
			putc('\n', stdout);
		}
		if (ch != -1 || verboseflag != 0) {
			if (ch == -1) printf("Font %3d Cache indx:    ", k);
			else printf("Font %3d Cache indx:%3d ", k, ch+1);
			fseek(input, pfmbase + (long) k * FILENAME, SEEK_SET);
			getbytes(input, line, FILENAME);
			printf("%s ", line);
			n = strlen(line);
			for (i = n; i < NAMESPACE; i++) putc(' ', stdout);
			fseek(input, pfbbase + (long) k * FILENAME, SEEK_SET);
			getbytes(input, line, FILENAME);
			printf("%s\n", line);
		}
	}

	printf("\n");

	for (k = 0; k < ncache; k++) {
		fseek(input, cachebase + (long) k * WIDTHS, SEEK_SET);
		getbytes(input, line, 12);
		c = line[0];
		d = line[1];
		i = line[2];
		fn = (d << 8) | (c & 255);
/*		if (c == 255 && d == 255) printf("NOT USED Cache time:%3d\n", i); */
		if (fn == -1) printf("NOT USED Cache time:%3d ", i);
		else printf("Font %3d Cache time:%3d ", fn, i);
/*		if (fn < 0) printf("c %d d %d\n", c, d); */
/*		if (checkflag != 0 && (c != 255 || d != 255)) { */
		if (checkflag != 0 && fn >= 0) {
			fseek(input, pfmbase + (long) fn * FILENAME, SEEK_SET);
			getbytes(input, line, FILENAME);
			if ((pfminput = fopen(line, "rb")) == NULL) {
				perror(line);
			}
			else {
				fseek(pfminput, 95, SEEK_SET);
				firstchar = getc(pfminput);
				lastchar = getc(pfminput);
				defaultchar = getc(pfminput);
				pfmextension = 117;
				fseek(pfminput, pfmextension + 6, SEEK_SET); /* 123 */
				extenttable = ureadfour(pfminput);
				printf("%s ", line);
				n = strlen(line);
				for (i = n; i < NAMESPACE; i++) putc(' ', stdout);
/*				printf("table %4ld ", extenttable); */
				printf("first %3d last %3d\n",  firstchar, lastchar);
				fseek(pfminput, extenttable, SEEK_SET);				
				fseek(input, cachebase + (long) k * WIDTHS + 12, SEEK_SET);
				badflag = 0;
				for (i = 0; i < firstchar; i++) {
					(void) getc (input);	(void) getc (input);
				}
				for (i = firstchar; i < lastchar; i++) {
					c = getc (input); d = getc(input);
					c = (d << 8) | (c & 255);
					e = getc (pfminput); f = getc(pfminput);
					e = (f << 8) | (e & 255);
					if (c != e) printf ("%d (%d %d) ", i, c, e);
					if (c != e) badflag++;
				}
/*				if (badflag != 0) printf(" %s\n", line); */
				if (badflag != 0) printf("\n"); 
				fclose(pfminput);
			}
		}
		else printf("\n");
	}
	fclose(input);
	return 0;
}

/* Details of 44 byte cache info kept for EVERY font */

/* 0 ? 1 ? 2 ? 3 ? */

/* 4[4], 8[4], 12[4], 16[4] are first four entries of FontMatrix * 2^30 */
/* 20[2] cache entry number */
/* 22[2] MasterHeight, MaxScale, MasterUnits ? (always 1000) */
/* 24[2] (yur - yll) - 1000, or 0 if (yur - yll) < 1000 */ 
/* 26[2] yll + 1000 */
/* 28[2] AveWidth */
/* 30[2] MaxWidth */
/* 32[2] ItalicSkew = - 1000 * tan(ItalicAngle) */
/* 34[2] kern pairs */
/* 36 startchar */
/* 37 endchar */
/* 38 defaultchar */
/* 39 breakchar */
/* 40 PitchandFamily */
/* 41 CharSet */
/* 42 always 1 ? */
/* 43 always 0 ? */
