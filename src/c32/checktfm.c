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

/* check em_quad in TFM files and fix if needed */
/* modifies files in place if -m given on command line */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXPARAMS 64

int verboseflag=0;

int modifyflag=0;

int doubleflag=0;

int useemdashflag=1;			/* use width of emdash if found */

int emdashfound=0;				/* non-empty character slot in TFM */

double emdashwidth;				/* emdash width if available */

double defaultem=1000.0;		/* "em" quad used in all old TFM files */

int lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np;

int nparam;					/* position in file of parameters (in words) */

long parameters[MAXPARAMS];

unsigned long checksum; 

unsigned long checkdefault = 0x59265920;	/* default signature */

char checksumvector[16]="";	/* derived from checksum */

char *paramnames[] = {
	"slant", "spacewidth", "spacestretch", "spaceshrink", "x-height", "em_quad", "extraspace", 
	""
};

char *encodenames[] = {			/* names of encoding vectors */
	"texnansi",
	"ansinew",
	"tex256",
	"standard",
	"textext",
	""
};

int emdashes[] = {				/* positions of emdash in that encoding */
	151, 
	151,
	22,
	208,
	124,
	0
};

int emdashcode(char *name) {	/* get emdash char code, given encoding name */
	int k;
	for (k = 0; k < 16; k++) {
		if (strcmp(encodenames[k], "") == 0) break;
		if (strncmp(encodenames[k], name, 6) == 0)
			return emdashes[k];
		
	}
	return -1;
}

/**********************************************************************************/

/* Takes first six letters of encoding vector name and compresses */
/* into four bytes using base 40 coding */ /* 40^6 < s^32 */
/* Treats lower case and upper case the same, ignores non alphanumerics */
/* Except handles -, &, and _ */	/* This *decodes* from checksum */

/* writes back into first six chars of vector */

int decodefourty(unsigned long checksum, char *vector) {
	int c;
	int k;
/*	char vector[6+1]; */

/*	if (checksum == checkdefault) { */
	if (checksum == 0) {
		strcpy(vector, "unknown");
		return 1;
	}
	else if ((checksum >> 8) == (checkdefault >> 8)) {	/* last byte random */
		strcpy (vector,  "native");		/* if not specified ... */
		return 1;
	}
	else {
		for (k = 0; k < 6; k++) {
/*			c = (int) (checksum % 36); */
			c = (int) (checksum % 40);
/*			checksum = checksum / 36; */
			checksum = checksum / 40;
			if (c <= 'z' - 'a' ) c = c + 'a';
			else if (c < 36) c = (c + '0') - ('z' - 'a') - 1;
			else if (c == 36) c = '-';
			else if (c == 37) c = '&';
			else if (c == 38) c = '_';
			else {
/*				printf("c = %d ", c); */	/* can only be 39 */
				c = '.';				/* unknown */
			}
			vector[5-k] = (char) c;
		}
		vector[6] = '\0';
	}
/*	printf("Reconstructed vector %s\n", vector); */
/*	return strdup(vector); */
	return 0;
}

/**********************************************************************************/

/* big endian input and output */

int readint(FILE *input) {
	int c, d;
	c = getc(input);
	d = getc(input);
	if (c == EOF || d == EOF) printf("HIT EOF! ");
	return (c << 8) | d;
}

void writeint(FILE *output, int wrd) {
	int c, d;
	c = (wrd << 8) & 255;
	d = wrd & 255;
	putc(c, output);
	putc(d, output);
}

long readlong(FILE *input) {
	long c, d, e, f;
	c = getc(input);
	d = getc(input);
	e = getc(input);
	f = getc(input);
	if (c == EOF || d == EOF || e == EOF || f == EOF) printf("HIT EOF! ");
	return (((((c << 8) | d) << 8) | e) << 8) | f;
}

void writelong(FILE *output, long wrd) {
	long c, d, e, f;
	c = (wrd >> 24) & 255;
	d = (wrd >> 16) & 255;
	e = (wrd >> 8) & 255;
	f = wrd & 255;
	putc(c, output);
	putc(d, output);
	putc(e, output);
	putc(f, output);	
}

/* 2^20 = 1048576 */

long mapdouble(double x) {		/* map to tfm scaled integer */
	if (x == 0.0) return 0L;
	else if (x < 0.0) return -mapdouble(-x);
	else return (long) ((x * 1048576.0 + 499.999) / 1000.0);
}

double unmap (long x) {
	double dx;
	long th;
	if (x == 0) return 0.0;
	if (x < 0) return -unmap(-x);
	dx = (double) x / 1048.576;
	th = (long) (dx * 500.0 + 0.5);	
	return (th / 500.0); 
}

int checktfm(FILE *input) {	/* read TFM file to get info including em_quad */
	int k, kem;
	long charinfo;
	int nchars, nwidths;
	long nemdashwidth=0;
	int windex = -1;

	emdashfound=0;
	lf = readint(input);
	lh = readint(input);
	bc = readint(input);
	ec = readint(input);
	nw = readint(input);
	nh = readint(input);
	nd = readint(input);
	ni = readint(input);
	nl = readint(input);
	nk = readint(input);
	ne = readint(input);
	np = readint(input);
	(void) fseek(input, 6 * 4, SEEK_SET);
	checksum = readlong(input);
	decodefourty(checksum, checksumvector);
	kem = emdashcode(checksumvector);
	printf("%08X encoding %s emdash char code %d\n", checksum, checksumvector, kem);
	nchars = 6 + lh;
/*	(void) fseek(input, (6 + lh) * 4, SEEK_SET); */	/* bc */
	if (kem >= 0 && kem >= bc && kem <= ec) {
		(void) fseek(input, (nchars + (kem-bc)) * 4, SEEK_SET);	/* kem */
		charinfo = readlong(input);
		if (charinfo == 0) {
			printf("ERROR: no emdash in %d < %d < %d\n", bc, kem, ec);
			emdashfound=0;
		}
		else {
			windex = (int) (charinfo >> 24) & 255;		
/*			printf("Width index %d\n", windex); */
			nwidths = 6 + lh + (ec - bc + 1);
			(void) fseek (input, (nwidths + windex) * 4, SEEK_SET);
			nemdashwidth = readlong(input);
			emdashwidth = unmap (nemdashwidth);
			printf("emdash width-index %d width %ld %lg\n",
				   windex, nemdashwidth, emdashwidth);
/*			if (doubleflag) emdashwidth = 2.0 * emdashwidth; */
			emdashfound=1;
		}
	}
	else {
		printf("ERROR: no emdash in %d < %d < %d\n", bc, kem, ec);
		emdashfound=0;
	}

/*	lf = 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np; */
	nparam = 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne; 
	(void) fseek(input, nparam * 4, SEEK_SET);
	if (np > MAXPARAMS) {
		printf("Too many parameters %d\n", np);
		exit(1);
	}
	for (k = 0; k < np; k++) {
		parameters[k] = readlong(input);
		if (verboseflag || k == 5) {
			printf("%3d\t%ld", k, parameters[k]);
			printf("\t%lg", unmap(parameters[k]));
			if (k < 7) printf("\t%s", paramnames[k]);
			printf("\n");
		}
	}
	if ((unmap(parameters[5]) != emdashwidth) &&
		(unmap(parameters[5]) != 2 * emdashwidth)) {
		printf("MISMATCH: TFM param[5] %lg <> emdash %lg\n", unmap(parameters[5]), emdashwidth);
		return -1;
	}
	else {
		if (verboseflag) printf("MATCH\n");
		return 0;
	}
}

/* go in and change em_quad */

int changequad(FILE *input) {
	double newemquad;
	if (emdashfound == 0) return 0;
	if (useemdashflag && emdashfound) {
/*		if (parameters[5] == mapdouble(emdashwidth))  */
		if (unmap(parameters[5]) == emdashwidth) {
			printf("EM QUAD ALREADY SET CORRECTLY? ");
			printf("no need to modify since %lg == %lg\n",
				   unmap(parameters[5]), emdashwidth);
			return 0;
		}
/*		else if (parameters[5] == 2 * mapdouble(emdashwidth))  */
		else if (unmap(parameters[5]) == 2 * emdashwidth) {
			printf("FIXED WIDTH FONT? ");
			printf("no need to modify since %lg == 2 * %lg\n",
				   unmap(parameters[5]), emdashwidth);
			return 0;
		}
		newemquad = emdashwidth;
		if (doubleflag) newemquad = 2.0 * emdashwidth;
	}
	else newemquad = defaultem;
	printf("MODIFYING: em_quad to be %lg\n", newemquad);
	(void) fseek(input, (nparam + 5) * 4, SEEK_SET);
	writelong(input, mapdouble(newemquad));
	return 1;
}

/***********************************************************************************/

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

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *extractfilename(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/***********************************************************************************/

int main(int argc, char *argv[]) {
	FILE *input;
	int m, flag, firstarg=1;

	if (argc < firstarg+1) exit(1);
	while (*argv[firstarg] == '-')  {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag=1;
		else if (strcmp(argv[firstarg], "-d") == 0) doubleflag=1;
		else if (strcmp(argv[firstarg], "-m") == 0) modifyflag=1;
		firstarg++;
	}
	if (modifyflag)	printf("WARNING: Will modify TFM files\n");

	if (argc < firstarg+1) exit(1);

	for (m = firstarg; m < argc; m++) {
		input = fopen(argv[m], "rb");
		if (input == NULL) {
			perror(argv[m]);
			continue;
		}
		printf("FILE:\t%s\n", argv[m]);
		flag = checktfm(input);
		fclose (input);
		
		if (flag && modifyflag && emdashfound) {
			input = fopen(argv[m], modifyflag ? "rb+" : "rb");
			if (input == NULL) {
				perror(argv[m]);
				continue;
			}
			changequad(input);
			fclose (input);
		}
		printf("\n");
	}
	return 0;
}

/***********************************************************************************/
