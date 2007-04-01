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

/* Read DVIPS produced PS files and strip out bitmapped fonts */
/* Replace with calls for salable outline fonts */
/* Also fix up bad way of drawing rules */
/* Also add appropriate pdfmark DOCINFO */
/* Also remap 0 -- 31 to higher up to avoid Acrobat bugs */
/* The big problem is identifying the fonts --- compare with lots of PK */
/* The big problem is identifying the fonts --- use TFM as guide */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>		/* for _fmalloc */
#include <memory.h>		/* for _fmemcpy */

/***************************************************************************/

#define MAXFILENAME 128

#define MAXLINE 256

#define MAXFONTS 256

/* #define MAXCHRS 256 */

#define MAXCHRS 128			/* for CM fonts ! */

#define MAXCOLUMN 78

#define NEARBUFFER

#ifdef NEARBUFFER
/* #define MAXHEXSTRING 16384 */
#define MAXHEXSTRING 20000
#define MAXCOMPRESSED 10000
#else
/* #define MAXHEXSTRING 65535 */
#define MAXHEXSTRING 32700
#define MAXCOMPRESSED 20000
#endif

/* what compareglyph returns if glyphs are bad mismatch */

#define MISMATCH -2

/* what appears as error when character not seen in PK file */

#define UNSEENCHAR -1

/* width to use for char that has zero width code in TFM */

#define INVALIDCHAR -2000000000L

/***************************************************************************/

char line[MAXLINE];					/* buffer space for PS tokens */

/* may need to make following __far for space large characters */

#ifdef NEARBUFFER
unsigned char buffer[MAXHEXSTRING];	/* buffer for current charstring */
unsigned char cmpbuf[MAXCOMPRESSED];	/* buffer for current charstring */
unsigned char *schar;				/* pointer into the above for PK */
#else
unsigned char __far *buffer;		/* buffer for current charstring */
unsigned char __far *cmpbuf;		/* buffer for current charstring */
unsigned char __far *schar;			/* pointer into the above for PK */
#endif

char *psext="ps";					/* default extension for PS file */

char *pkext="pk";					/* default extension for PK files */

char *tfmext="tfm";					/* default extension for LOG files */

char *logext="fnt";					/* default extension for LOG files */

char *pkpat="d:\\pk\\%d\\%s.%d";			  /* d:\pk\300\cmr10.300 */
char *pcpat="d:\\pctex\\pixel\\dpi%d\\%s.pk"; /* d:\pctex\pixel\dpi300\cmr10.pk */

char *tfmdir="c:\\yandytex\\tfm";	/* directory with TFM files */

int cc;						/* character code from PS file */

unsigned int nlen;			/* length of string in buffer */

int nchars, ndict;			/* from:  /Fa <nchars> <ndict> */

int advance;				/* advance width of character in PS file */

char font[16];				/* `name' of current font Fa, Fb, ... FA, FB ... */

#ifdef NEARCHARS
unsigned char *charstrings[MAXCHRS];		/*  ptrs hex char strings */
#else
unsigned char __far *charstrings[MAXCHRS];	/* __far ptrs hex char strings */
#endif

unsigned int nbytes[MAXCHRS];		/* length of each charstring */

int widths[MAXCHRS];				/* advance width of each charstring */

long nerrors[MAXCHRS];				/* bits in error for each glyph */
									/* or UNSEENCHAR or MISMATCH */

unsigned long ntotal=0;				/* total allocated in near space */
unsigned long ftotal=0;				/* total allocated in far space */

int fontindex=0;			/* pointer into the next few */

char *fontnames[MAXFONTS];	/* font names in PS file Fa, Fb .. FA, FB ... */

char *pknames[MAXFONTS];	/* names of PK matching files */

double fontscales[MAXFONTS];		/* non-zero if Type 1 font */

unsigned long chksum, dsgnsize;		/* from TFM metric info */

/***************************************************************************/

/*	global place for PK char parameters convenience */

int	pkcc;					/* character code from PK file */
long pktfm;					/* TFM advance width from PK file */
unsigned int pkw, pkh;		/* width, height from PK file */
int pkhoff,	pkvoff;			/* hoff, voff from PK file */
int pkdm;					/* advance width in pixels ? */
unsigned int pklen;			/* length of charstring in buffer from PK file */
double dpih;				/* from PK file HPPP */
double dpiv;				/* from PK file VPPP */
int kindex=-1;				/* psfontnames[kindex] */

int magnification=1000;		/* magnification read from PS file */
int dpi=300;				/* from %DVIPSParameters line if any */
int hdpi=300;				/* resolution read from PS file */
int vdpi=300;				/* resolution read from PS file */
long hsize, vsize;			/* DVIPS parameters to @start */

int compressed=0;			/* whether bitmaps are compressed or not */

char *jobname=NULL;			/* job name read from PS file */

/***************************************************************************/

char *programversion = "PREPDF version 1.0";

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright=
"Copyright (C) 1995--1996  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* "Copyright (C) 1995--1996  Y&Y, Inc. All rights reserved. (508) 371-3286\ */

#define COPYHASH 13188389

/***************************************************************************/

int verboseflag=1;
int traceflag=0;
int debugflag=0;
int showflag=0;
int dotsflag=1;
int logflag=0;
int noiseflag=0;

int detailflag=0;

int skipearly=1;		/* stop looking when find exact match */
int rejectbad=1;		/* don't keep looking at obvious mismatch */
int showrejects=0;		/* show why font rejected */
int usetfms=1;			/* read in TFM files and use these */
int checkpks=1;			/* check potential matches using PK files */
int checkpktfm=1;		/* check TFM advance width and pixel advance width */

/****************************************************************************/

/* Names of all fonts we can expect to be able to replace */
/* CM + AMS + extra LaTeX + SliTeX + LOGO fonts */
/* Includes `in between' cmbsy*, cmmib*, cmcsc*, cmex* fonts */
/* Includes `in between' lasy* fonts */
/* Includes `in between' masm*, msbm*, eur*, eus*, euf*, wn* fonts */
/* These odd sizes will all have to be treated by using nearby sizes */

/* Note MAXMETRICS must be greater than or equal to count 185+1 */

#define MAXMETRICS 186

/* char *psfontnames[] = { */
char *psfontnames[MAXMETRICS] = { 
  "cmr10", "cmti10", "cmbx10", "cmmi10", "cmsy10", "cmex10", "cmtt10",
  "cmbx12", "cmr12", "cmti12", "cmsl10", "cmcsc10", "cmss10",
  "cmr7", "cmr5", "cmmi7", "cmmi5", "cmsy7", "cmsy5", "cmbx7",
  "cmbx5", "cmti7", "cmr17", "cmsl12", "cmsl9", "cmsl8", "cmr9",
  "cmr8", "cmr6", "cmmi12", "cmti9", "cmti8", "cmtt12", "cmtt9",
  "cmtt8", "cmss12", "cmss17", "cmss8", "cmss9", "cmmi6",
  "cmmi8", "cmmi9", "cmbsy10", "cmbsy7", "cmbsy5", "cmbx6",
  "cmbx8", "cmbx9", "cmbsy9", "cmbsy8", "cmbsy6", "cmmib10",
  "cmmib5", "cmmib6", "cmmib7", "cmmib8", "cmmib9", "cmbxsl10",
  "cmbxti10", "cmcsc9", "cmcsc8", "cmex9", "cmex8", "cmex7",
  "cmssbx10", "cmssdc10", "cmssi10", "cmssi12", "cmssi17",
  "cmssi9", "cmssi8", "cmssq8", "cmssqi8", "cmsy9", "cmsy8",
  "cmsy6", "cmtcsc10", "cmtex10", "cmtex8", "cmtex9", "cmu10",
  "cminch", "cmb10", "cmsltt10", "cmvtt10", "cmitt10", "cmdunh10",
  "cmff10", "cmfi10", "cmfib8", "lcircle1", "lcirclew", "line10",
  "linew10", "lasy10", "lasy5", "lasy6", "lasy7", "lasy8",
  "lasy9", "lasyb10", "lcmss8", "lcmssb8", "lcmssi8", "logo10",
  "logo8", "logo9", "logobf10", "logosl10", "msam10", "msbm10",
  "msam7", "msbm7", "msam5", "msbm5", "msam9", "msbm9", "msam8",
  "msbm8", "msam6", "msbm6", "euex10", "euex7", "euex8", "euex9",
  "eufb10", "eufb14", "eufb5", "eufb6", "eufb7", "eufb8",
  "eufb9", "eufm10", "eufm14", "eufm5", "eufm6", "eufm7",
  "eufm8", "eufm9", "eurb10", "eurb5", "eurb6", "eurb7",
  "eurb8", "eurb9", "eurm10", "eurm5", "eurm6", "eurm7",
  "eurm8", "eurm9", "eusb10", "eusb5", "eusb6", "eusb7",
  "eusb8", "eusb9", "eusm10", "eusm5", "eusm6", "eusm7",
  "eusm8", "eusm9", "wncyr10", "wncyb10", "wncyi10", "wncyss10",
  "wncysc10", "wncyr5", "wncyr6", "wncyr7", "wncyr8", "wncyr9",
  "wncyb5", "wncyb6", "wncyb7", "wncyb8", "wncyb9", "wncyi5",
  "wncyi6", "wncyi7", "wncyi8", "wncyi9", "wncyss8", "wncyss9", ""
};

long __far *metrics[MAXMETRICS];	/* advance widths for above fonts */

long designsizes[MAXMETRICS];		/* design sizes for above fonts */

/****************************************************************************/

char *xmalloc (size_t n) {
	char *s=malloc(n);
	if (s == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %u bytes (total %lu)\n", 
				n, ntotal);
		exit(1);
	}
	ntotal += n;
	return s;
}

unsigned char __far *xfmalloc (size_t n) {
	unsigned char __far *s=_fmalloc(n);
	if (s == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %u bytes (total %lu)\n", 
				n, ftotal);
		exit(1);
	}
	ftotal += n;
	return s;
}

char *xstrdup (char *str) {
	char *s=_strdup(str);
	if (s == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %u bytes\n", strlen(str)+1);
		exit(1);
	}
/*	ntotal += strlen(str); */
	return s;
}

/************************************************************************/

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

void removeexten(char *fname) {  /* remove extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) != NULL) {
		if ((t = strrchr(fname, '\\')) == NULL || s > t) *s = '\0';
	}
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

/***************************************************************************/

/* read simple token - ended by white space or / ( [ { < */
/* return -1 if hit end of file */ /* returns 0 if run into delimiter */

/* int readtoken(FILE *input, char *buffer, int nlen) { */
int readtoken(FILE *input, char *buffer, unsigned int nlen) {
	char *s=buffer;
	int c;

	*s = '\0';					/* in case skip out early */
	c = getc(input);
	while (c <= ' ' && c != EOF) c = getc(input);
	if (c == EOF) return -1;	/* nothing seen before EOF */
	if (c == '/') {
		*s++ = (char) c;
		c = getc(input);
	}
	while (c > ' ') {
		if (c == '\\') c = getc(input);
		if (c == '/' || c == '(' || c == '[' || c == '{' || c == '<') {
/*			ungetc(c, input); */
			break;
		}
/*		if ((s - buffer) >= nlen-1) { */
		if (s >= buffer + nlen-1) {
			fprintf(stderr, "Exceeded token buffer size (%u bytes)\n", nlen);
			break;
		}
		*s++ = (char) c;
		c = getc(input);
	}
	*s = '\0';
	if (c != EOF) ungetc (c, input);
/*	if (traceflag) printf("TOKEN: %s (%c)\n", buffer, c); */
	if (debugflag) printf("TOKEN: %s (%c)\n", buffer, c);
	return (s - buffer);
}

/* crude hack to read PS string ... for jobname only */

int readstring(FILE *input, char *buffer, unsigned int nlen) {
	char *s=buffer;
	int nesting=0;
	int c;

	*s = '\0';					/* in case skip out early */
	c = getc(input);
	while (c <= ' ' && c != EOF) c = getc(input);
	if (c == EOF) return -1;	/* nothing seen before EOF */
	if (c != '(') {
		fprintf(stderr, "ERROR: string not found\n");
		ungetc(c, input);
		return 0;
	}
	for (;;) {
		c = getc(input);
		if (c == EOF) return -1;
		if (c == '\\') {
			*s++ = (char) c;
			c = getc(input);
			*s++ = (char) c;
			continue;
		}
		if (c == ')') {
			if (nesting == 0) break;
			else nesting--;
		}
		else if (c == '(') nesting++;
		*s++ = (char) c;
		if (s >= buffer + nlen-1) {
			fprintf(stderr, "Exceeded token buffer size (%u bytes)\n", nlen);
			return -1;
		}
	}
	*s = '\0';
	return (s - buffer);
}

/* try and skip over more complex token in PS file */

int skipovercomplex (FILE *input) {
	int c, depth=0;
	c = getc(input);
	if (c == EOF) return -1;
	if (c == '(') {
		for (;;) {
			if (c == '\\') c = getc(input);
			if (c == '(') depth++;
			else if (c == ')') {
				depth--;
				if (depth == 0) break;
			}
			c = getc(input);
			if (c == EOF) return -1;
		}
		return 0;
	}
	else if (c == '[') {
		for (;;) {
			if (c == '\\') c = getc(input);
			if (c == '[') depth++;
			else if (c == ']') {
				depth--;
				if (depth == 0) break;
			}
			c = getc(input);
			if (c == EOF) return -1;
		}
	}
	else if (c == '{') {
		for (;;) {
			if (c == '\\') c = getc(input);
			if (c == '{') depth++;
			else if (c == '}') {
				depth--;
				if (depth == 0) break;
			}
			c = getc(input);
			if (c == EOF) return -1;
		}
	}
	else if (c == '<') {
		for (;;) {
			if (c == '\\') c = getc(input);
			if (c == '<') depth++;
			else if (c == '>') {
				depth--;
				if (depth == 0) break;
			}
			c = getc(input);
			if (c == EOF) return -1;
		}
	}
}

/****************************************************************************/

/* read hex string such as <0123456789ABCDEF> into buffer supplied */
/* args: file stream, buffer for result, buffer length */
/* returns: length of string or 0 if EOF --- ignores white space */
/* start at < --- reads up to an including > */
/* complains invalid characters, odd number of hex characters */

#ifdef NEARBUFFER
unsigned int readhexstring (FILE *input, unsigned char *buffer, unsigned int nlen) 
#else
unsigned int readhexstring (FILE *input, unsigned char __far *buffer, unsigned int nlen) 
#endif
{
	int c, d;
#ifdef NEARBUFFER
	unsigned char *s=buffer;
#else
	unsigned char __far *s=buffer;
#endif

	c = getc(input);
	while (c != '<' && c != EOF) c = getc(input);
/*	if (c == EOF) return -1; */
	if (c == EOF) return 0;
	if (c == '<') c = getc(input);					/* always */
	while (c != '>') {
		while (c <= ' ' && c != EOF) c = getc(input); /* ignore white space */
		if (c == EOF) {
			fprintf (stderr, "EOF in hex string\n");
/*			return -1; */
			return 0;
		}
		if (c == '>') break;
		if (c >= '0' && c <= '9') c = c - '0';
		else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
		else {
			fprintf (stderr, "Bad character in hex string %c\n", c);
			break;
		}
		d = getc(input);
		while (d <= ' ' && d != EOF) d = getc(input); /* ignore white space */
		if (d == EOF) {
			fprintf (stderr, "EOF in hex string\n");
/*			return -1; */
			return 0;
		}
		if (d == '>') {
			fprintf (stderr, "Odd number of hex characters\n");
			*s++ = (unsigned char) ((c << 4));	/* treat last as zero */
			c = d;			/* to stop scan to next > */
			break;
		}
		if (d >= '0' && d <= '9') d = d - '0';
		else if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
		else if (d >= 'a' && d <= 'f') d = d - 'a' + 10;
		else {
			fprintf (stderr, "Bad character in hex string %c\n", d);
			break;
		}
/*		if ((s - buffer) >= nlen-1) { */
		if (s >= buffer + nlen-1) {
			fprintf(stderr, "Exceeded hex string buffer size (%u bytes)\n",
					nlen);
			break;
		}
		*s++ = (unsigned char) ((c << 4) | d);
		c = getc(input);
	}
	*s = '\0';					/* just for fun ! */
	if (c != '>') while (c != '>' && c != EOF) c = getc(input);
	if (c == EOF) {
		fprintf (stderr, "EOF in hex string\n");
/*		return -1; */
		return 0;
	}
	return (s - buffer);
}

#ifdef NEARBUFFER
void showhexstring(unsigned char *buffer, unsigned int nlen, int width, int cc) 
#else
void showhexstring(unsigned char __far *buffer, unsigned int nlen, int width, int cc) 
#endif
{
	int c, d, n=0, maxcolumn, column;
	unsigned int k;
	if (debugflag) printf("nlen %u, width %d, cc %d\n", nlen, width, cc);
	if (nlen == 0) return;
	if (width == 0) return;
	printf("<\n");
	column = 0;
	if (width != 0) {
		n = ((width + 7) / 8);			/* bytes per image row */
/*		each image row will take n * 2 + 1 characters in output */
/*		maxcolumn = (MAXCOLUMN / (n * 2 + 1)) * (n * 2 + 1); */
		maxcolumn = (MAXCOLUMN / (n * 2 + 1)) * (n * 2 + 1) - 1;
	}
	else maxcolumn = MAXCOLUMN;

	for (k = 0; k < nlen; k++) {
		c = (buffer[k] >> 4) & 15;
		d = buffer[k] & 15;
		if (c < 10) c = c + '0';
		else c = c - 10 + 'A';
		if (d < 10) d = d + '0';
		else d = d - 10 + 'A';
		if (column >= maxcolumn) {
			putc('\n', stdout);
			column = 0;
		}
		if (column != 0 && n != 0 && k % n == 0) {
			putc(' ', stdout);
			column++;
		}
		putc(c, stdout);
		putc(d, stdout);
		column += 2;
	}
/*	if ((k % n) != 0) printf("Length not multiple of width\n"); */
	printf("\n> %d\n", cc);  
/*	printf("\n>\n"); */
}

/***************************************************************************/

/* we expect after %%EndProcSet the following */
/* TeXDict begin 40258431 52099146 1000 300 300 (epslatex.dvi) @start */
/* hsize, vsize, magnification, hdpi, vdpi jobname string */
/* but may see another %%BeginProcSet first ... */

int scantoendprocset (FILE *input) {
	int c, n;
/*	int k; */
	char *s;

	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line != '%') continue;			/* look only at comments */
		if (strncmp(line, "%DVIPSParameters:", 17) == 0) {
			if ((s = strstr(line, "dpi")) != NULL) {
				if (sscanf (s, "dpi=%d", &dpi) == 1) {
					hdpi = vdpi = dpi;
				}
			}
			if (strstr(line, "compressed") != NULL) compressed = 1;
			if (verboseflag)
				printf("%d dpi %s\n", dpi, compressed ? "compressed" : "");
		}
		if (strncmp(line, "%%EndProcSet", 12) == 0) {
			c = getc(input); ungetc(c, input);
			if (c == '%') continue;			/* probably %%BeginProcSet */
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (strcmp(line, "TeXDict") != 0)
				printf("ERROR: expecting %s, not %s\n", "TeXDict", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (strcmp(line, "begin") != 0)
				printf("ERROR: expecting %s, not %s\n", "begin", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%ld", &hsize) < 1)
				printf("ERROR: expecting %s, not %s\n", "40258431", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%ld", &vsize) < 1)
				printf("ERROR: expecting %s, not %s\n", "52099146", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%d", &magnification) < 1)
				printf("ERROR: expecting %s, not %s\n", "magnification", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%d", &hdpi) < 1)
				printf("ERROR: expecting %s, not %s\n", "horiz dpi", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%d", &vdpi) < 1)
				printf("ERROR: expecting %s, not %s\n", "vert dpi", line);
			n = readstring(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			jobname = xstrdup(line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (strcmp(line, "@start") != 0) 
				printf("ERROR: expecting %s, not %s\n", "@start", line);
			if (traceflag)
				printf("TeXDict begin %ld %ld %d %d %d (%s) @start\n",
					  hsize, vsize, magnification, hdpi, vdpi, jobname);
			if (verboseflag)
		printf("hsize: %lg vsize: %lg magnification: %d hdpi: %d vdpi: %d\n",
			   (double) hsize / 65781.76, (double) vsize / 65781.76,
				   magnification, hdpi, vdpi);
			if (verboseflag)
				printf("jobname: %s\n", jobname);
/*			if (n == 0) {
				while ((c = getc(input)) > ' ') ;
			} */
			return 0; 
		}
	}
	return -1;
}

void freecharstrings (void) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (charstrings[k] != NULL) {
#ifdef NEARCHARS
			free(charstrings[k]);	/* will have to be far */
#else
			_ffree(charstrings[k]);	/* will have to be far */
#endif
			charstrings[k] = NULL;
			nbytes[k] = 0;
		}
	}
	ntotal=0;
}

void freefontnames (void) {
	int k;
	for (k = 0; k < MAXFONTS; k++) {
		if (fontnames[k] != NULL) {
			free(fontnames[k]);
			fontnames[k] = NULL;
		}
		if (pknames[k] != NULL) {
			free(pknames[k]);
			pknames[k] = NULL;
		}
	}
}

void freemetrics (void) {
	int k;
	for (k = 0; k < MAXMETRICS; k++) {
		if (metrics[k] != NULL) {
			_ffree(metrics[k]);
			metrics[k] = NULL;
		}
	}
}

char *textext[32] = {
"Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon", 
"Phi", "Psi", "Omega", "ff", "fi", "fl", "ffi", "ffl", 
"dotlessi", "dotlessj", "grave", "acute", "caron", "breve", "macron", "ring", 
"cedilla", "germandbls", "ae", "oe", "oslash", "AE", "OE", "Oslash"
};

/* read one glyph from PS file */

int readoneglyph (FILE *input) {
	int c;
	unsigned int nbyte;
	long current;
	unsigned int width, height;
	int xoff, yoff;
/*	int advance; */
	char schr[16];

	c = getc(input);
	while (c != '<' && c != EOF) c = getc(input);
	if (c == EOF) return -1;
	ungetc(c, input);
	current = ftell(input);
/*	nlen = readhexstring(input, buffer, sizeof(buffer)); */
	nlen = readhexstring(input, buffer, MAXHEXSTRING-5);
/*	if (nlen < 0) return-1; */
	if (nlen == 0) return-1;			/* EOF */
	width = buffer[nlen-5];
	height = buffer[nlen-4];
	xoff = 128 - buffer[nlen-3];
	yoff = buffer[nlen-2] - 127;
	advance = buffer[nlen-1];

/*	if (showflag) showhexstring(buffer, nlen, width, cc); */
	nbyte = ((width + 7) / 8) * height;
	if (nlen != nbyte + 5)
		printf("Mismatch in string size %u <> %u\n", nlen, nbyte+5);
	readtoken (input, line, sizeof(line));
	if (sscanf (line, "%d", &cc) == 0) {
		if (strcmp(line, "I") != 0) 
			fprintf(stderr, "Don't understand char code: %s\n", line);
		cc++;	/* I */
	}
	if (traceflag) {
		if (cc >= 32 && c < 128) {
			schr[0] = (char) cc;
			schr[1] = '\0';
		}
		else if (cc < 32) {
			strcpy(schr, textext[cc]);
		}
		else {
			schr[0] = ' ';
			schr[1] = '\0';
		}
		printf("%s, %d (%s), %u bytes at %ld\tW %d H %d X %d Y %d A %d\n",
			   font, cc, schr,
			   nlen, current, width, height, xoff, yoff, advance);
	}
	if (showflag) showhexstring(buffer, nlen, width, cc); 
	return 0;
}

/****************************************************************************/

/* 0 1 1 2 1 2 2 3 1 2 2 3 2 3 3 4 1 2 2 3 2 3 3 4 2 3 3 4 3 4 4 5
   1 2 2 3 2 3 3 4 2 3 3 4 3 4 4 5 2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6
   1 2 2 3 2 3 3 4 2 3 3 4 3 4 4 5 2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6
   2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6 3 4 4 5 4 5 5 6 4 5 5 6 5 6 6 7
   1 2 2 3 2 3 3 4 2 3 3 4 3 4 4 5 2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6
   2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6 3 4 4 5 4 5 5 6 4 5 5 6 5 6 6 7
   2 3 3 4 3 4 4 5 3 4 4 5 4 5 5 6 3 4 4 5 4 5 5 6 4 5 5 6 5 6 6 7
   3 4 4 5 4 5 5 6 4 5 5 6 5 6 6 7 4 5 5 6 5 6 6 7 5 6 6 7 6 7 7 8 */

int onebits[MAXCHRS];		/* how many bits are on in k */

void setuponebits(void) {
	int k, n, count;
	for (k = 0; k < MAXCHRS; k++) {
		count = 0;
		n = k;
		for (;;) {
			if (n == 0) break;
			n = ~(n & (- n)) & n;
			count++;
		}
		onebits[k] = count;
		if (debugflag) printf("%d ", count);
	}
	if (debugflag) putc('\n', stdout);
}

/* how many bits are different in these bitmaps ? */
/* assume width and height are equal --- or off by at most 1 ... */
/* returns MISMATCH if advance widths do not match */
/* returns MISMATCH if rows or columns off by more than 1 */
/* returns MISMATCH if bits in error > 1/4 total bits in cell */

#ifdef NEARCHARS
long compareglyph (unsigned char *a, unsigned char *b,
				   unsigned int na, unsigned int nb) {
#else
long compareglyph (unsigned char __far *a, unsigned char __far *b,
				   unsigned int na, unsigned int nb) {
#endif
	unsigned int nmin, k;
	long count=0;
#ifdef NEARCHARS
	unsigned char *sa=a;
	unsigned char *sb=b;
#else
	unsigned char __far *sa=a;
	unsigned char __far *sb=b;
#endif
	unsigned int i, j;
	unsigned int wa, ha;
	unsigned int nwmin, hmin;
	int xa, ya, ta;
	unsigned int wb, hb;
	int xb, yb, tb;
	unsigned int nwa, nwb;

/*	if (na != nb) return -1; */				/* bad mismatch */
/*	we don't look at the last 5 bytes since these are not image data */
	if (na < nb) nmin = na-5;	else nmin = nb-5; 
/*	read out image width, height, x and y offset, advance width */
	wa = a[na-5]; ha = a[na-4];	xa = a[na-3]; ya = a[na-2]; ta = a[na-1];
	wb = b[nb-5]; hb = b[nb-4];	xb = b[nb-3]; yb = b[nb-2]; tb = b[nb-1];
/*	compute number of bytes in each row */
	nwa = (wa + 7) / 8;	nwb= (wb + 7) / 8;
/*	if (wa != wb || ha != hb) { */
/*	if (wa < wb - 2 || wa > wb + 2 ||
		ha < hb - 2 || ha > hb + 2) {
		if (showrejects) printf("#");
		return MISMATCH;
	} */
/*	if (ta != tb) { */		/*	Don't bother if advance width is different ! */
	if (ta > tb + 1 || ta < tb -1) {/*	Don't if advance width differ ! */
		if (traceflag) 
			printf("Advance width mismatch %d versus %d\n", ta, tb);
		if (showrejects) putc('@', stdout);
		return MISMATCH; 					/* advance widths don't match */
	}
/*	don't compare if different bytes per row, or if more than one row off */
/*	if (nwa != nwb || ha > hb + 1 || ha < hb - 1) { */
	if (wa < wb - 2 || wa > wb + 2 || ha < hb - 2 || ha > hb + 2) {
		if (traceflag)
			printf("Gross mismatch %d x %d versus %d x %d\n", wa, ha, wb, hb);
		if (showrejects) putc('&', stdout);
		return MISMATCH;	/* gross char cell mismatch */
	}
	if (wa < wb - 1 || wa > wb + 1 || ha < hb - 1 || ha > hb + 1) {
		if (traceflag)
			printf("Cell mismatch %d x %d versus %d x %d\n", wa, ha, wb, hb);
/*		if (showrejects) putc('&', stdout); */
/*		return (wa * ha / 4); */	/* gross char cell mismatch ? */
	}
	if (ha < hb) hmin = ha; else hmin = hb;
	if (nwa < nwb) nwmin = wa; else nwmin = wb;
	if (nwa == nwb) {		/* normally do this the simple fast way */
		for (k = 0; k < nmin; k++) {
			if (*sa != *sb) count += onebits[*sa ^ *sb];
			sa++; sb++;
		}
	}
	else {					/* mismatch in bytes per row, do more carefully */
		for (i = 0; i < hmin; i++) {
			for (j = 0; j < nwmin; j++) {
				if (*sa != *sb) count += onebits[*sa ^ *sb];
				sa++; sb++;
			}
			if (nwa > nwmin) {
				count += onebits[*sa];
				sa++;		/* try and keep in step */
			}
			if (nwb > nwmin) {
				count += onebits[*sb];
				sb++;		/* try and keep in step */
			}
		}
	}
	if (ha > hmin) {
		for (j = 0; j < nwa; j++) count += onebits[*sa++];
	}
	if (hb > hmin) {
		for (j = 0; j < nwb; j++) count += onebits[*sb++];
	}
/*	if (wa != wb) count += (ha + hb); */ /* penalize for mismatch in width */
/*	if (ha != hb) count += (wa + wb); */ /* penalize for mismatch in height */
/*	consider a lousy match if more than 1/4 of the pixels are wrong in glyph */
	if (count > (((long) wa * ha) + ((long) wb * hb)) / 8) {
		if (showrejects) putc('$', stdout);
		return MISMATCH;			/* too many pixels mismatch */
	}
/*	printf("count %ld na %d nb %d\n", count, na, nb); */
	return count;
}

/****************************************************************************/

#ifdef NEARBUFFER
unsigned int decompressglyph(unsigned char *, unsigned char *, int);
#else
unsigned int decompressglyph(unsigned char __far *, unsigned char __far *, int);
#endif

/* extract bitmap charsting from PS file */
/* returns 0 at end of this font, if another follows */
/* returns -1 if no more fonts, or if hit EOF */

int extractbitmap (FILE *input) {
	int n;
	int c;
	double fscale;

/*	First scan to start of font, and read name: /Fe ... */

	for (;;) {								/* get to /Fa ... */
		n = readtoken(input, line, sizeof(line));
		if (n < 0) {
			fprintf(stderr, "Hit EOF in extractbitmap\n");
			return -1;				/* hit EOF */
		}
		if (n == 0) {						/* hit ( [  < */
			if (skipovercomplex(input) < 0) return -1;
		}
		if (*line == '/') {					/* remember the name */
			strcpy(font, line+1);			/* /Fa 1 55 df <...> */
			n = readtoken(input, line, sizeof(line));
			sscanf (line, "%d", &nchars);	/* how many chars */
			n = readtoken(input, line, sizeof(line));
			ndict = 0;
			sscanf (line, "%d", &ndict);	/* dict size needed */
/*			if (n == 0) printf("Font: %s %d (NOT PK)\n", font, nchars); */
/*			if (verboseflag) */
			if (verboseflag && n > 0) { 
				if (dotsflag) putc('\n', stdout);
				printf("Font: %s %d %d\n", font, nchars, ndict);
			}
			fontindex++;
			if (fontindex >= MAXFONTS) {
				fprintf(stderr, "ERROR: Too many fonts (%d)\n", MAXFONTS);
				exit(1);
			}
/*			fontnames[fontindex] = _strdup(font); */	/* remember name */
			fontnames[fontindex] = xstrdup(font);		/* remember name */
			freecharstrings();				/* just in case */
			break;							/* time to move on */
		}
	}
/*	/Fa 167[141 46[69 69 40[{}3 100.000000 /MTEX rf */
	if (ndict == 0) {	/* not a PK font */
		fscale = 0.0;
		for (;;) {
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;			/* hit EOF by mistake */
			if (n == 0) {					/* hit delimiter [ { */
				c = getc(input); continue;
			}
			if (*line == '/') break;		/* found name */
			sscanf(line, "%lg", &fscale);
		}
		pknames[fontindex] = xstrdup(line+1);
		fontscales[fontindex] = fscale;
		printf("Font: %s %d\t(NOT PK) %s at %lg\n",
			   font, nchars, line+1, fscale);
		return 0;
#ifdef IGNORED
		c = getc(input);				/* scan to next font ? */
		while (c != '/' && c != EOF) c = getc(input);
		ungetc(c, input);
		n = 0;
		return 0;
#endif
	}

	for (;;) {							/* loop over chars */
		for (;;) {							/* look for next char */
			c = getc(input); ungetc(c, input);
			if (c == '/') {				/* hit next font --- no chars here */
				n = 0;
				return 0;
			}
			n = readtoken (input, line, sizeof(line));
			if (n < 0) {
				if (traceflag) printf("Return on EOF?\n");
				return -1;			/* EOF */
			}
			if (n == 0) {						/* hit ( [  < */
				if (debugflag) {
					c = getc(input); ungetc(c, input);
					printf("Break out on n == 0 (%c)?\n", c);
				}
				break;				/* found < presumably */
			}
		}

		if (n > 0) {				/* cannot happen */
			if (traceflag)
				printf("Hit end of bitmap font definitions: %s\n", line);
/*				break; */					/* hit end of fonts */
			return -1;					/* hit end of fonts */
		}
		readoneglyph(input);				/* read actual hex data */
/*		new stuff to deal with compressed bitmaps */
		if (compressed) {
			if (nlen > MAXCOMPRESSED) {
				fprintf(stderr, "Compressed bitmap too long (%u > %d)\n",
						nlen, MAXCOMPRESSED);
				exit(1);
			}
/*			move compressed charstring from buffer to cmpbuf */
#ifdef NEARBUFFER
			memcpy(cmpbuf, buffer, nlen);
#else
			_fmemcpy(cmpbuf, buffer, nlen);
#endif
/*			decompress from cmpbuf back into buffer first */
			nlen = decompressglyph(buffer, cmpbuf, nlen);
/*			nlen = outnlen; */
		}
#ifdef NEARCHARS
/*		charstrings[cc] = malloc(nlen); */
		charstrings[cc] = xmalloc(nlen);
/*		ntotal += nlen; */
		if (charstrings[cc] == NULL) {
			fprintf(stderr, "ERROR: Unable to allocate memory (%ld)\n", ntotal);
			exit(1);
		}
#else
/*		charstrings[cc] = _fmalloc(nlen); */	/* will have to be __far */
		charstrings[cc] = xfmalloc(nlen);		/* will have to be __far */
/*		ftotal += nlen; */
		if (charstrings[cc] == NULL) {
			fprintf(stderr, "ERROR: Unable to allocate memory (%ld)\n", ftotal);
			exit(1);
		}
#endif

/* following actually requires NEARCHARS and NEARBUFFER */
/* #ifdef NEARCHARS */
#if defined(NEARCHARS) && defined(NEARBUFFER)
		memcpy(charstrings[cc], buffer, nlen);	/* save hex data */
/* #error USING MEMCPY */
#else
		_fmemcpy(charstrings[cc], buffer, nlen);	/* save hex data */
/* #error USING _FMEMCPY */
#endif
/*		if (showflag) showhexstring(buffer, nlen); */
		nbytes [cc] = nlen;
		widths [cc] = advance;
/*		if (traceflag) printf("Finished with glyph %d\n", cc); */
		if (debugflag) printf("Finished with glyph %d\n", cc);
		
		for (;;) {							/* get to next glyph */
			c = getc(input);
			while (c != EOF && c <= ' ') c = getc(input);
			ungetc(c, input);
			if (c == '/') {
/*				ungetc(c, input); */		/* put it back */
				n = 0;
				return 0;					/* hit next font */
			}
/*			if (c == '<') ungetc(c, input); */
			n = readtoken (input, line, sizeof(line));
			if (*line == '/') printf("WRONG! %s %d\n", line, n);
			if (n < 0) {
				if (traceflag) printf("Hit EOF\n");
				return -1;			/* EOF */
			}
			if (n == 0) break;				/* presumably hit < */
			if (strcmp(line, "end") == 0) {
				if (traceflag) printf("Break out on %s\n", line);
				return -1;
/*				break; */
			}
			if (strncmp(line, "%%", 2) == 0) {
				if (traceflag) printf("Break out on %s\n", line);
				return -1;
/*				break; */
			}
			if (*line == '/') {				/* should not happen */
				strcpy(font, line+1);		/* /Fa 1 55 df <...> */
				printf("ERROR FONT: %s\n", font);
/*				freecharstrings(); */
				n = 0;
				break;
			}
			if (n == 0) break;				/* hit next char ? */
		}									/* end of loop for next char */
/*		if (n > 0) break;					/* Hit the end */
		if (n > 0) return -1; 				/* Hit the end */
	}
/*	return 0; */							/* normal return - next font */
/*	if (traceflag) printf("End of extractbitmap\n"); */
/*	freecharstrings(); */
}

/***************************************************************************/

double averthres=20.0;		/* worst acceptable average bit error */

int processpkfile (FILE *, int);

/* see whether this PK file matches the PS file bitmapped font */

int checkerror(FILE *pkinput, char *pkfile) {
	int i, count, invalid;
	long sum;
	double aver;

/*	for (i = 0; i < MAXCHRS; i++) nerrors[i] = 0; */
/*	set trap for characters that occur in PS, but *not* seen in PK */
	for (i = 0; i < MAXCHRS; i++) {
		if (charstrings[i] != NULL)	nerrors[i] = UNSEENCHAR; 
		else nerrors[i] = 0;			/* chars not seen in PS file */
	}
	if (processpkfile (pkinput, 1) < 0) {
		if (traceflag) printf("Bad Mismatch %s\n", pkfile);
		return 0;
	}
/*	fclose(pkinput); */
	count = 0;
	invalid = 0;
	sum = 0;
	for (i = 0; i < MAXCHRS; i++) {
		if (charstrings[i] != NULL) {
/*			if (nerrors[i] >= 0) sum += nerrors[i]; */
			if (nerrors[i] == UNSEENCHAR) {
				if (showrejects) putc('!', stdout);
				invalid++;				/* glyph in PS, but not in PK! */
				break;
			}
/*			else if (nerrors[i] == MISMATCH) sum += 0xFFFF; */
			else if (nerrors[i] == MISMATCH) invalid++; /* bad mismatch */
			else sum += nerrors[i];					/* normal case */
			count++;
		}
	}
/*	fclose(pkinput); */
	if (count - invalid > 0) {
		aver = (double) sum / (count - invalid);
		if (sum > 0) {
			putc('\n', stdout);
			printf("Font: %s %s near miss: average %lg over %d\n", 
			   font, pkfile, aver, count - invalid);
		}
	}
	else aver = 0.0;
	if (count > 0 && invalid == 0 && (sum == 0 || aver < averthres)) {
		if (dotsflag) putc('\n', stdout);
		printf("Font %s exactly matches %s (%lg)\n", font, pkfile, aver);
/*		if (pknames[fontindex-1] == NULL) */
		if (pknames[fontindex] == NULL)
/*			pknames[fontindex-1] = _strdup(pkfile); */
/*			pknames[fontindex-1] = xstrdup(pkfile); */
			pknames[fontindex] = xstrdup(pkfile);
		else printf("ERROR: more than one match: %s versus %s\n",
/*					pknames[fontindex-1], pkfile); */
					pknames[fontindex], pkfile);
/*		skip out early ??? */
/*		if (skipearly) break; */
		if (skipearly) return -1;
	}
	else if (traceflag)
	printf("Error %ld for %d chars (%d invalid) avg %lg of font %s in %s\n",
		   sum, count-invalid, invalid, aver, font, pkfile);
	else if (dotsflag) putc('.', stdout);
	return 0;
}

/***************************************************************************/

/* replace %s with fontname, %d with resolution in given pattern */
/* handle patterns like d:\pk\600\cmr10.600 */

void makeupfilename (char *filename, char *pattern, char *fontname, int dpi)
{
	char *s=filename, *t=pattern;
	int c;
	dpih = dpiv = (double) dpi;
	for (;;) {
		c = *t++;
		if (c == 0) break;
		if (c == '%') {
			c = *t++;
			if (c == 'd') sprintf(s, "%d", dpi);
			else if (c == 's') strcpy(s, fontname);
			else {
				fprintf(stderr, "ERROR: Bad pattern: %s\n", pattern);
				break;
			}
			s = s + strlen(s);
			continue;
		}
		*s++ = (char) c;
	}
	*s = '\0';
	return;
}

/*	if any PK files mentioned on command line try those */

int searchpkfontsA(int m, int argc, char *argv[]) {
	char pkfile[MAXFILENAME];
	FILE *pkinput;
	int k, flag;

	if (verboseflag) printf("Searching: %s\n", font);
	if (m >= argc) return 0;				/* no PK fonts */
	for (k = m; k < argc; k++) {			/* step through PK files */
		strcpy(pkfile, argv[k]);
		extension (pkfile, pkext);
		if (traceflag) printf("Opening %s\n", pkfile);
		if ((pkinput = fopen(pkfile, "rb")) == NULL) {
			perror(pkfile);
			exit(1);
		}
		flag = checkerror(pkinput, pkfile);
		fclose(pkinput);
		if (flag < 0) break;	/* break out early ? */
	}
	return 0;
}

/* Try PK font files based on list included here */

int searchpkfontsB(void) {
	char pkfile[MAXFILENAME];
	FILE *pkinput;
	int k, flag;

	if (verboseflag) printf("Searching: %s\n", font);
	for (k = 0; k < MAXMETRICS; k++) {		/* step through PK files */
		if (strcmp(psfontnames[k], "") == 0) break;
		makeupfilename(pkfile, pcpat, psfontnames[k], hdpi);
		if (traceflag) printf("Opening %s\n", pkfile);
		if ((pkinput = fopen(pkfile, "rb")) == NULL) {
			if (traceflag) perror(pkfile);
/*			exit(1); */
			continue;
		}
		flag = checkerror(pkinput, pkfile);
		fclose(pkinput);
		if (flag < 0) break;	/* break out early ? */
	}
	return 0;
}

/***************************************************************************/

long checkchar(int chr) {
	long count=0;
	if (chr < 0 || chr > 255) return 0;		/* sanity check */
	if (charstrings[chr] == NULL) {
		if (traceflag) printf("char %d not used in PS file\n", chr);
	}
	else {
		count = compareglyph (charstrings[chr], buffer, nbytes[chr], pklen);
/* if count == MISMATCH we have a gross mismatch */
		if (traceflag)
			printf("char %d error %ld na %u nb %u\n",
				   chr, count, nbytes[chr], pklen);
/*		if (traceflag) printf("char %d error %ld\n", chr, count); */
	}
	nerrors[chr] = count;
	return count;
}

/***************************************************************************/

int checkoutpk(char *fontname, int dpi) {
	char pkfile[MAXFILENAME];
	FILE *pkinput;
	int flag;
	makeupfilename (pkfile, pkpat, fontname, dpi);
	if (traceflag) printf("Opening %s\n", pkfile);
	if ((pkinput = fopen(pkfile, "rb")) == NULL) {
		if (traceflag) perror(pkfile);
/*		exit(1); */
/*		continue; */
		return 1;				/* file not found failure */
	}
	flag = checkerror(pkinput, pkfile);
	fclose(pkinput);
	if (flag < 0) return 0;		/* matched */
	else return -1;				/* earned failure */
}

/***************************************************************************/

int dpi300[] = {
	300, 329, 360, 432, 518, 622, 746, 896, 1075, 1290, 1548, 0
};

int dpi600[] = {
	600, 657, 720, 864, 1037, 1244, 1493, 1792, 2150, 2580, 3096, 0
};

int snaptodpi (double dpi) {
	double dis, mindis=1500;
	int k, kmin=0;
	if (hdpi == 600) {
		for (k = 0; k < 32; k++) {
			if (dpi600[k] == 0) break;
			dis = ((double) dpi600[k] - dpi);
			if (fabs(dis) < mindis) {
				mindis = fabs(dis);
				kmin = k;
			}
		}
		return dpi600[kmin];
	}
	else {
		for (k = 0; k < 32; k++) {
			if (dpi300[k] == 0) break;
			dis = ((double) dpi300[k] - dpi);
			if (fabs(dis) < mindis) {
				mindis = fabs(dis);
				kmin = k;
			}
		}
		return dpi300[kmin];
	}
}

/* double threshold=0.5; */	/* theoretically this should do! */
/* double threshold=1.0; */
double threshold=2.0;		/* max allowed advance width error */

/* see whether metrics for font in PS file match any TFM fonts */

int searchtfms (void) {
	int k, i, cc, width, wmin, wmax;
	int count=0;
	long sum=0;			/* sum of advance widths of chars in PS file font */
	int mismatch;
	long __far *metric;
	int sdpi;
	double av, dsum, scale, dwidth, dpi, epsilon, epsilonmax;

/*	if (strcmp(font, "Ff") == 0) debugflag = 1;	*//* debugging */ 
/*	if (strcmp(font, "Fe") == 0) debugflag = 1;	*//* debugging */
	wmin = 255; wmax = -1; cc = 0;
	for (i = 0; i < MAXCHRS; i++) {		/* total advance width for chars */
		if (charstrings[i] == NULL) continue;
		sum += widths[i];
		count++;
		cc = i;
		if (widths[i] > wmax) wmax = widths[i];
		if (widths[i] < wmin) wmin = widths[i];
	}
	if (count == 0) {
		fprintf(stderr, "Zero valid characters in %s?", font);
		return -1;
	}
	if (sum == 0) {
		fprintf(stderr, "Zero total advance width in %s?", font);
		return -1;
	}
	if (count == 1) {
		printf("Single character %d (%c) in font %s\n", cc,
			   (cc > 32 && cc < 128) ? cc : ' ', font);
	}
	else if (wmin == wmax) {
		printf("Fixed width %d (%d times) font %s?\n", wmin, count, font);
	}
	av = (double) sum / count;
	if (traceflag)
	  printf("sum %ld count %d average %lg\n", sum, count, av);

	for (k = 0; k < MAXMETRICS; k++) {	/* step through TFM files */
		if (metrics[k] == NULL) continue;	/* metrics not found earlier */
		if (debugflag) printf("Considering font %s\n", psfontnames[k]);
		metric = metrics[k];			/* metrics for TFM font k */
		dsgnsize = designsizes[k];
		dsum = 0.0;
		for (i = 0; i < MAXCHRS; i++) {
			if (charstrings[i] == NULL) continue;
/*	Cannot possibly match if PS file refers to char code that is not in font */
			if (metric[i] == INVALIDCHAR) {
				if (debugflag)
					printf("char %d null in %s\n", i, psfontnames[k]);
				dsum = INVALIDCHAR;
				break;
			}
			dsum += (double) metric[i];
		}
		if (dsum == INVALIDCHAR) continue;	/* char in PS not in TFM */
		if (dsum == 0.0) continue;		/* give up on this font TFM */
/*		dsum = dsum / 1048576.0; */
/*		dsum = dsum * ((double) dsgnsize / 1048576.0); */
		scale = (double) sum / dsum;
		dpi = scale * (72.27 * 1048576.0 / ((double) dsgnsize / 1048576.0)) ;
		if (debugflag) printf("%s\tdsum %lg scale %lg dpi %lg ",
							  psfontnames[k], dsum, scale, dpi);
		sdpi = snaptodpi (dpi);
		scale = (double) sdpi / (72.27 * 1048576.0 / ((double) dsgnsize / 1048576.0)) ;
/*		if (debugflag) printf("(%lg) ", sdpi); */
		if (debugflag) printf("(%d) ", sdpi);
		mismatch = 0;
		epsilonmax = 0.0;
		for (i = 0; i < MAXCHRS; i++) {
			if (charstrings[i] == NULL) continue;
/*			dwidth = ((double) metric[i] / 1048576.0) *
					 ((double) dsgnsize / 1048576.0) / scale; */
			dwidth = (double) metric[i] * scale;
			if (dwidth >= 0.0) width = (int) (dwidth + 0.5);
			else width = (int) (dwidth - 0.5);
/*			if (width != widths[i]) { */
			epsilon = dwidth - widths[i];
			if (epsilon < 0.0) epsilon = - epsilon;
			if (epsilon > epsilonmax) epsilonmax = epsilon;
			if (epsilon > threshold) {
				mismatch++;
/*				break; */
			}
		}
/*		if PS file font has char not in TFM then mismatch = INVALIDCHAR */
		if (debugflag) {
			printf("%d miss in %d (eps %lg)\n", mismatch, count, epsilonmax);
		}
		if (debugflag && mismatch > 0 && mismatch < 3 && count > 5) {
			printf("SHOW DETAILS MISMATCHES!!!\n");
			for (i = 0; i < MAXCHRS; i++) {
				if (charstrings[i] == NULL) continue;
/*				dwidth = ((double) metric[i] / 1048576.0) *
					 ((double) dsgnsize / 1048576.0) / scale; */
				dwidth = (double) metric[i] * scale;
				if (dwidth >= 0.0) width = (int) (dwidth + 0.5);
				else width = (int) (dwidth - 0.5);
/*				if (width != widths[i]) { */
				epsilon = dwidth - widths[i];
				if (epsilon > threshold || epsilon < - threshold) {
/*					mismatch++; */
					printf("%d %lg versus %d (%ld %lg)\n",
						   i, dwidth, widths[i], metric[i], scale);
				}
			}
		}
		if (verboseflag) {
			if (mismatch == 0)
/*				printf("MATCH %s %s at %lg (eps %lg)\n", */
				printf("MATCH %s %s at %d (eps %lg)\n",
					   font, psfontnames[k], sdpi, epsilonmax);
		}
		if (mismatch == 0 && checkpks != 0) {
			kindex = k;
			if (checkoutpk(psfontnames[k], sdpi) == 0) {
				break;		/* found match - skip out */
			}				
		}
	}	/* end of for loop over TFM metrics */
/*	if (strcmp(font, "Ff") == 0) debugflag = 0;	*//* debugging */
/*	if (strcmp(font, "Fe") == 0) debugflag = 0;	*//* debugging */
	return 0;
}

/***************************************************************************/

void setupcharstrings(void) {
	int k;
	for (k = 0; k < MAXCHRS; k++) charstrings[k] = NULL;
	ntotal = 0;
	ftotal = 0;
}

void setupfontnames(void) {
	int k;
	for (k = 0; k < MAXFONTS; k++) fontnames[k] = NULL;
	for (k = 0; k < MAXFONTS; k++) pknames[k] = NULL;
	for (k = 0; k < MAXFONTS; k++) fontscales[k] = 0.0;
	fontindex = -1;			/* gets incremented before use */
}

void showfonttable(char *infile) {
	int k;
	putc('\n', stdout);
	printf("Font Table for %s:\n", infile);
	for (k = 0; k <= fontindex; k++) {
		if (fontnames[k] == NULL) continue;
		printf("Font: %s\t", fontnames[k]);
		if (pknames[k] != NULL)
			printf("%s", pknames[k]);
		else printf("UNKNOWN");
		if (fontscales[k] != 0.0) printf(" at %lg", fontscales[k]);
		putc('\n', stdout);
	}	
}

void setupmetrics(void) {
	int k;
	for (k = 0; k < MAXMETRICS; k++) metrics[k] = NULL;
}

#ifndef NEARBUFFER
void setupbuffer (void) {
/*	buffer = _fmalloc(MAXHEXSTRING); */
	buffer = xfmalloc(MAXHEXSTRING);
}
#endif

#ifndef NEARBUFFER
void freebuffer (void) {
	_ffree (buffer);
}
#endif

/*****************************************************************************/

void showusage(char *s) {
	fprintf (stderr, "\
%s [-{v}{t}{d}{s}] <PS-file> <PK-file-1> ...\n", s);
	if (detailflag == 0) exit(1); 
	fprintf (stderr, "\n\
\tv: verbose mode\n\
\tt: trace mode\n\
\td: debug mode\n\
\ts: show mode\n\
");
	exit(1); 
}

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': verboseflag = 1; return 0; 
		case 't': traceflag = 1; return 0; 
		case 'd': debugflag = 1; return 0; 
		case 's': showflag = 1; return 0; 
		case 'n': noiseflag = 1; return 0; 
		case 'l': logflag = 1; return 0; 
		case 'r': showrejects = 1; return 0; 
/* rest take arguments */
/*		case 's': spaceflag = 1; return -1;  */
		default: {
			fprintf(stderr, "Invalid command line flag '%c'", c);
			exit(1);
		}
	}
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
/*				if (vectorflag != 0) {
					vectorfile = s;	
					strncpy(codingvector, removepath(s), MAXENCODING);
					vectorflag = 0; reencodeflag = 1; 
				} */
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

/***************************************************************************/

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
#ifdef DEBUGGING
	(void) _getch(); 
#endif
	return hash;
}

int readalltfms(char *);

int main(int argc, char *argv[]) {
	char infile[MAXFILENAME];
	FILE *input;
	int firstarg=1;
	int n;

	setuponebits();
	setupcharstrings();
	setupfontnames();
	setupmetrics();
#ifndef NEARBUFFER
	setupbuffer();
#endif
	if (argc < firstarg+1) {
		fprintf(stderr, "Need PS file name argument\n");
		exit(1);
	}

/*	exit(1); */

	firstarg = commandline(argc, argv, 1);

	if (firstarg > argc - 1) showusage(argv[0]);

	if (checkcopyright(copyright) != 0) exit(1);

	if (usetfms) {
		if (readalltfms(tfmdir) == 0) {
			usetfms=0;						/* failed to read any */
		}
	}

	strcpy(infile, argv[firstarg]);			/* PS file */
	extension (infile, psext);
	if (traceflag) printf("Opening %s\n", infile);
	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile);
		exit(1);
	}
/*	fontindex=0; */
	fontindex = -1;
	if (strstr(infile, ".ps") != NULL) {
		if (traceflag) fputs("Scanning to %%EndProcSet\n", stdout);
		scantoendprocset (input);	
		for (;;) {					/* loop over fonts in PS file */
			if (traceflag) printf("Extracting PK font info\n");
			n = extractbitmap (input);
			if (ndict != 0) {
				if (usetfms) searchtfms();
				else if (firstarg+1 < argc)
					searchpkfontsA(firstarg+1, argc, argv);
				else searchpkfontsB();
			}
			else printf("Empty font %s?\n", font);
			if (traceflag) printf("Finished with font: %s\n", font);
			freecharstrings();
			if (n < 0) break;		/* no more found */
/*			break; */				/* debugging */
		}
		if (traceflag) printf("End of extractbitmap\n");
		fclose (input);
/*		freecharstrings(); */
	}
/*	debugging case to check processpkfile */
	else if (strstr(infile, ".pk") != NULL) processpkfile (input, 0);

#ifndef NEARBUFFER
	freebuffer();
#endif
	if (verboseflag) showfonttable(infile);
	freefontnames();
	if (usetfms) freemetrics();
	if (jobname != NULL) free(jobname);
	return 0;
}

/* NOTE: rule drawing in DVIPS: */
/* DVIPS has some wako ways of dealing with rules */
/* These do not work properly in Distiller / PDF */
/* (1) First of all, it looks at the CTM, and if r_{11}^2 + r_{12}^2 < 0.99 */
/* it uses a traditional snapto approach, then drawing a box and filling */
/* *except* it does not offset by 0.25 as recommended */
/* this kicks in when the resolution of the device is *less* than that */
/* assumed when the PS file was made e.g. Distiller ! */
/* (2) Second if the scale is 1.0 or greater, it uses imagemask ! */
/* with a string that is initialized to zero and a [1 0 0 -1 0 0] matrix */
/* It checks whether it is on Display PostScript or NeXT */
/* and does something slightly different in the two cases */

/* /v calls /V, which can be either /QV or /RV */

/* DVIPS also draws rules using absolute positioning which can lead to */
/* problems since the text is drawn using relative positioning */
/* but there ain't much we can do about that ... */

/* 
/QV{ % x y (absolute)
% (**** Using QV ***) print flush
gsave 0 setlinecap
rulex ruley ge 
{ruley setlinewidth
ruley 2 div sub moveto
rulex 0} % w >= h
{rulex setlinewidth
exch rulex 2 div add exch moveto
0 ruley} % w < h
ifelse
rlineto stroke grestore
}B
*/

/****************************************************************************/
/* pktops.c		(c) Copyright Y&Y 1990 - 1992							*/
/*					All Rights Reserved										*/
/*					Unpublished and confidential material.					*/
/*					--- Do not Reproduce or Disclose ---					*/
/****************************************************************************/

/* Grovel through PK font files and produce Adobe Type 3 font PS file */
/* Copyright (C) 1991, 1992, Y&Y. All rights reserved. */

/* #define FNAMELEN 80 */
#ifdef SLOPPY
#define MAXROWLEN 32767 
#else
#define MAXROWLEN 8192
#endif
#define MAXCOMMENT 257
#define MAXCOLUMNS 78
/* #define MAXCHARS 256 */
#define INFINITY 65535
#define SINFINITY 32767
#define NUMLIM 137
#define MAXPATHLEN 64		/* maximum path name length */
#define MAXLINE 256			/* maximum length of input line (afm - tfm) */
/* #define OUTPUTEXT "ps" */

/* #define WANTCONTROLD */
/* #define CONTROLBREAK */

/* command op codes found in PK files: */

#define xxx1 240
#define xxx2 241
#define xxx3 242
#define xxx4 243
#define yyy 244
#define post 245
#define nop 246
#define pre 247

#define pk_id 89

/* int uppercaseflag = 1;	*/ /* produce uppcase font name */
/* int correctwidth = 0;	*/ /* try and correct tfm widths */
/* int fontdictsize=32;	*/ /* size of font dict to allocate */
/* int permanent = 1;	*/ 	/* non-zero => permanent downloading */
/* int showrows = 1;	*/ 	/* split hexadecimal output across rows */
/* int directprint=0;	*/ 	/* direct to printer if non-zero */

int copyflag;	 	/* non-zero if this character should be copied */
int warnflag;	 	/* warning about this character - need code */
int prepass;		/* non-zero means first pass -  suppress output */
int chrs;			/* current character working on */
int chrcount;		/* total characters seen */
int chrproc;		/* total characters actually processed */
int maxchrs;		/* highest character code seen */
int selectflag;		/* non-zero means character selection invoked */

/* int hexflag;	*/ 	/* next arg is hexadecimal code for selected chars */
/* int rangeflag;	*/ 	/* next arg is range of selected characters */
/* int stringflag;	*/ 	/* next arg is selected characters */
/* int outputflag;	*/ 	/* next arg is output destination */

int leftover; 		/* whats left over after removing one nybble */
int bitsleft;		/* whats left over after removing some bits */
int repeatcount;	/* how many times to repeat this row */
int rightend;		/* max safe column number to go to output */
int x, y;			/* coordinates relative to top left */
int bitpos;			/* count of bits in accumulated in output byte */
int byte;			/* byte being accumulated for output */
/* int column;	*/ 		/* column used for hexadecimal output */

/* int stripcomment=0;	*/ /* non-zero => strip out comments */
int pkcompensate=0;	 /* non-zero => use old vertical adjustment 95/Dec/10 */

/* volatile int bAbort=0;	*/ 	/* set when used types control-C */

int fxll, fyll, fxur, fyur;	 /* Font BBox */

/* int wantcpyrght = 1; */

/* char fn_in[MAXFILENAME], fn_out[MAXFILENAME]; */
/* FILE *input, *output; */ 

unsigned char row[MAXROWLEN];	/* copy of present row (for repeat) */

/* char line[MAXLINE];	*/ /* buffer for compying procset */

char comment[MAXCOMMENT];	 /* copy of comment from PK file */

/* char fontname[MAXFILENAME];	*/ 	/* derived from file name */

/* char wantchrs[MAXCHRS]; */ 	/* flags to indicate which chars wanted */

/* long ds, cs, hppp, vppp; */

unsigned long ds, cs;		/* design size (dvi units) and checksum  */

unsigned long hppp, vppp;	/* horizontal and vertical resolution */

int numchar=256;		/* if possible, obtain from file */ /* NA */

double ptsize;		/* design size computed from ds */ /* NA */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

struct ratio { /* structure for rational numbers */
	long numer;
	long denom;
};

/* compute good rational approximation to floating point number */
/* assumes number given is positive */

struct ratio rational(double fx, long nlimit, long dlimit) {
	long p0=0, q0=1, p1=1, q1=0, p2, q2;
	double s=fx, dq;
	struct ratio res;
/*	printf("Entering rational %g %ld %ld\n", fx, nlimit, dlimit);  */
	
	if (fx == 0.0) {
			res.numer = 0; res.denom = 1;
			return res;		/* the answer is 0/1 */
	}
	for(;;) {
		p2 = p0 + (long) s * p1;
		q2 = q0 + (long) s * q1;
/*		printf("%ld %ld %ld %ld %ld %ld %g\n", p0, q0, p1, q1, p2, q2, s);  */
		if (p2 > nlimit || q2 > dlimit || p2 < 0 || q2 <= 0) {
			p2 = p1; q2 = q1; break;
		}
		if ((double) p2 / (double) q2 == fx) break;
		dq = s - (double) ((long) s);
		if (dq == 0.0) break;
		p0 = p1; q0 = q1; p1 = p2; q1 = q2;	s = 1/dq;
/*		catch large s that will overflow when converted to long */
		if (s > 1000000000.0 || s < -1000000000.0) break; /* 1992/Dec/6 */
	}
	res.numer = p2; res.denom = q2;
	return res;		/* the answer is p2/q2 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for reading signed and unsigned numbers of various lengths */

unsigned int ureadone (FILE *infile) {	/* NA */
	return getc(infile);
}

int sreadone (FILE *infile) {
	int c;
	c = getc(infile);
	if (c > 127) return (c - 256);
	else return c;
}

unsigned int ureadtwo (FILE *infile) {
	return (getc(infile) << 8) | getc(infile);
}

int sreadtwo (FILE *infile) {
	return (getc(infile) << 8) | getc(infile);
}

unsigned long ureadthree (FILE *infile) {
/*	int c, d, e; */
	unsigned int c, d, e;
	c = getc(infile);	d = getc(infile);	e = getc(infile);
	return ((((unsigned long) c << 8) | d) << 8) | e;
}

long sreadthree (FILE *infile) {			/* NA */
	int c, d, e;
	c = getc(infile);	d = getc(infile);	e = getc(infile);
	if (c > 127) c = c - 256; 
	return ((((long) c << 8) | d) << 8) | e;
}

unsigned long ureadfour (FILE *infile) {
/*	int c, d, e, f; */
	unsigned int c, d, e, f;
	c = getc(infile);	d = getc(infile);
	e = getc(infile);	f = getc(infile);
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
}

long sreadfour (FILE *infile) {
	int c, d, e, f;
	c = getc(infile);	d = getc(infile);
	e = getc(infile);	f = getc(infile);
	return ((((((long) c << 8) | (long) d) << 8) | e) << 8) | f;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for extracting bits and nybbles from input file */

int getbit(FILE *infile){
	int c;

	if (bitsleft-- <= 0)  {
		leftover = getc(infile);
		bitsleft = 7;
	}
	c = (leftover >> 7); 
	leftover = leftover << 1; 
	return (c & 1);
}

int getnybble(FILE *infile){
	int c;

	if (leftover < 0) {
		c = getc(infile);
		leftover = c & 15;
		if (debugflag != 0) printf("N %d ", (c >> 4));
		return (c >> 4);
	}
	else {
		c = leftover;
		leftover = -1;
		if (debugflag != 0) printf("N %d ", c);
		return c;
	}
}

int ignorebits=0;					/* turned off if gone too far */

/* stuff for writing bits into output file */

int writebit(int bit) {
	byte = (byte << 1) | bit;
	bitpos++;
	if (bitpos == 8) {
		if (ignorebits == 0) {
			*schar++ = (unsigned char) byte;
/*			if ((schar - buffer) > MAXHEXSTRING) { */
			if (schar >= buffer + MAXHEXSTRING-5) {
				ignorebits++;
				fprintf(stderr, "Overflowed buffer %u\n", MAXHEXSTRING-5);
/*				return -1; */
			}
		}
		bitpos = 0; byte = 0;
	}
	return 0;
}

void setupbits(void) {
	schar = buffer;
	bitpos = 0; byte = 0;
	ignorebits = 0;
}

void clearoutbits(void) {
	while (bitpos != 0) writebit(0);
}

void finishbits(void) {
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* read number in hairy packed form use in PK files */

int readpackednum(FILE *infile, int dynf) {
	int i, j;
	i = getnybble(infile);
	if (i == 0) {
		for(;;) {
			j = getnybble(infile);
			i++;
			if (j != 0) break;
		}
		while (i > 0) {
			j = (j << 4) + getnybble(infile);
			i--;
		}
		return (j - 15 + ((13 - dynf) << 4) + dynf);
	}
	else if (i <= dynf) return(i);
	else if (i < 14) 
		return(((i - dynf - 1) << 4) + getnybble(infile) + dynf + 1);
	else {
		if (repeatcount != 0) {
			fprintf(stderr, "Second repeat count for this row!\n");
			exit(1);
		}
		if (i == 14) repeatcount = readpackednum(infile, dynf);
		else repeatcount = 1;	/* for i == 15 */
/*		printf("True %d ", repeatcount); */
		return readpackednum(infile, dynf);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Bitmapped format of character */

void readbits(FILE *infile, unsigned int w, unsigned int h) {
	unsigned int xx, yy;
	int bit;

	bitsleft = 0;
	rightend = MAXCOLUMNS - ((((w-1) >> 3) + 1) << 1) - 1;
	setupbits();
/*	if (verboseflag != 0) putc('\n', stdout); */
	for (yy = 0; yy < h; yy++)	{
		for (xx = 0; xx < w; xx++) {
			bit	= getbit(infile);
/*			if (verboseflag != 0) {
				if (bit != 0) putc('Z', stdout); 
				else putc('-', stdout);
				putc(' ', stdout);
			} */
			writebit(bit);
		}
/*		if (verboseflag != 0) putc('\n', stdout); */
		clearoutbits();
	}
}

void spitbit(int black) {
#ifndef SLOPPY
	int n, m, bit;
#endif
#ifdef SLOPPY
	row[x] = (char) black; 
#else
	n = x / 8;					/* divide by 8 */
	m = x % 8;					/* remainder 8 */
	bit = 1 << (7 - m);			/* stick in the bit */
	if (black) row[n] |= bit;
	else row[n] &= ~bit;
#endif
/*	if (verboseflag != 0) {
		if (black != 0) putc('X', stdout);
		else putc('.', stdout);
		putc(' ', stdout);
	} */
	writebit(black);
}

/* repeat the previous row */

void repeatrow(int w) {
	int i, black;
#ifndef SLOPPY
	int n, m, bit;
#endif
	for(i = 0; i < w; i++) {
#ifdef SLOPPY
		black = row[i]; 
#else
		n = i / 8;
		m = i % 8;
		bit = 1 << (7 - m);
		if ((row[n] & bit) != 0) black = 1;
		else black = 0;
#endif
/*		if (verboseflag != 0) {
			if (black != 0) putc('Y', stdout);
			else putc(',', stdout);
			putc(' ', stdout); 
		} */
		writebit(black); 
	} 
	clearoutbits();
/*	if (verboseflag != 0) putc('\n', stdout); */
	x = 0; y++;
}

/* spit out a run - and if needed a repeat of a row */

void spitout(int black, int count, int w) {
	while (count > 0) {
		spitbit(black);
		x++;
		if (x >= w) {
			x = 0; y++; 
			clearoutbits();
/*			if (verboseflag != 0) putc('\n', stdout); */
			while (repeatcount > 0) {
				repeatrow(w);
				repeatcount--;
			}
		}
		count--;
	}
}

/* read a character in hairy PK packed format */

void readpacked(FILE *infile, int dynf, int black, int w, int h) {
	int count;

	leftover = -1;
	rightend = MAXCOLUMNS - ((((w-1) >> 3) + 1) << 1) - 1;
	repeatcount = 0; 
	x = 0; y = 0;
	setupbits();
/*	if (verboseflag != 0) putc('\n', stdout); */
	for(;;) {
		count = readpackednum(infile, dynf);
		if (debugflag != 0) {
			printf("count %d (%d) ", count, black);
			if (repeatcount != 0) printf("repeat %d\n", repeatcount);
/*			(void) _getch(); */
		}
		spitout(black, count, w);
		black = 1 - black;
		if (debugflag != 0) printf("x %d y %d ", x, y);
/*		if (x == (w - 1) && y == (h - 1)) break; */
		if (x == 0 && y == h) break;
		if (y >= h) {
			printf("\nx %d y %d\n", x, y);
/*			(void) _getch(); */
			break;
		}
	}	
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

void showstatistics(void) {
	printf("Seen %d characters - highest code %d ", chrcount, maxchrs);
	if (chrproc != chrcount)
		printf("- processed %d characters ", chrproc);
	putc('\n', stdout);
	printf("FontBBox %d %d %d %d\n", fxll, fyll, fxur, fyur);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void do_pre(FILE *infile) {
	int c, k, i;
	double pdpih, pdpiv;

	c = getc(infile);	/* i = 89 */
	if (c != pk_id) {
		fprintf(stderr, "Not a PK file\n");
		return;
	}	
	k = getc(infile);	/* k */
	for (i = 0; i < k; i++) {
		c = getc(infile);
		comment[i] = (char) c;
	}
	comment[k]='\0';

	ds = ureadfour(infile);
	cs = ureadfour(infile);	
	hppp = ureadfour(infile);
	vppp = ureadfour(infile);
/*	ptsize = ds; */ 
	ptsize = ((double) ds) / 1048576.0; 
	pdpih = (((double) hppp) / 65536.0) * 72.27;
	pdpiv = (((double) vppp) / 65536.0) * 72.27;
	dpih = ((double) ((long) (pdpih * 100.0 + 0.5))) / 100.0;
	dpiv = ((double) ((long) (pdpiv * 100.0 + 0.5))) / 100.0;
	if (prepass != 0) return;
	if (traceflag != 0) {
		printf("COMMENT: %s\n", comment);
		printf("DESIGN SIZE: %lg ", ((double) ds) / 1048576.0);
		printf("CHECKSUM: %lX ", cs); 
		printf("HPPP: %lg ", ((double) hppp) / 65536.0);
		printf("VPPP: %lg ", ((double) vppp) / 65536.0);
		putc('\n', stdout);
/*		putc('\n', stdout); */
	} 
}

/* common subroutine to deal with * special * */

void do_xxx_sub(FILE *infile, unsigned long k) {
	unsigned long i;
	int c;

	for (i = 0; i < k; i++) {
		c = getc(infile);
/*		if (verboseflag != 0) putc(c, stdout); */
/*		if (prepass == 0) putc(c, outfile); */
	}
/*	if (verboseflag != 0) putc('\n', stdout); */
/*	if (prepass == 0) putc('\n', outfile); */
}

void do_xxx1(FILE *infile) {
/*	if (verboseflag != 0) printf("XXX1: "); */
/*	if (prepass == 0) fprintf(outfile, "%% XXX1: "); */
	do_xxx_sub(infile, (long) getc(infile));
}

void do_xxx2(FILE *infile) {
/*	if (verboseflag != 0) printf("XXX2: "); */
/*	if (prepass == 0) fprintf(outfile, "%% XXX2: "); */
	do_xxx_sub(infile, (long) ureadtwo(infile));
}

void do_xxx3(FILE *infile) {
/* 	if (verboseflag != 0) printf("XXX3: "); */
/*	if (prepass == 0) fprintf(outfile, "%% XXX3: "); */
	do_xxx_sub(infile, ureadthree(infile));
}

void do_xxx4(FILE *infile) {
/* 	if (verboseflag != 0) printf("XXX4: "); */
/*	if (prepass == 0) fprintf(outfile, "%% XXX4: "); */
	do_xxx_sub(infile, ureadfour(infile));
}

void do_yyy(FILE *infile) {
	unsigned long k;
	k = ureadfour(infile);
/*	if (verboseflag != 0) printf("YYY: %lX\n", k); */
/*	if (prepass == 0) fprintf(outfile, "%% YYY:  %lX\n", k); */
}

void adjustbbox(int w, int h, int hoff, int voff) {
	int cxll, cyll, cxur, cyur;
	cxll = -hoff; 
	cyll = voff - h;
	cxur = w - hoff;
	cyur = voff;
	if (cxll < fxll) fxll = cxll;
	if (cyll < fyll) fyll = cyll;
	if (cxur > fxur) fxur = cxur;
	if (cyur > fyur) fyur = cyur;	
}

void commonform(FILE *infile, unsigned int cc, long tfm, 
			unsigned int w, unsigned int h, int hoff, int voff, 
				int	dynf, int black) {
	double width;
/*	double pwidth; */
/*	long numerwidth, denomwidth; */
/*	struct ratio rwidth; */
	
/*	copy to global place for convenience */
	pkcc = cc;
	pktfm = tfm;
	pkw = w;
	pkh = h;
	pkhoff = hoff;
	pkvoff = voff;
	
/*	chrproc++;	*/
	if (w > MAXROWLEN) {
		fprintf(stderr, "Width of character %d greater than %d\n", 
			w, MAXROWLEN);
		exit(1);
	}
	width = 1000.0 * ((double) tfm) / 1048576.0;
/*	if (correctwidth != 0) {
		rwidth = rational(width, 2000000, (long) NUMLIM); 
		numerwidth = rwidth.numer; denomwidth = rwidth.denom;
		width = ((double) numerwidth) / ((double) denomwidth);
	} */
/*	pwidth = (width * hppp * ptsize) / 1000.0; */

	if (traceflag)
		printf("char code %d TFM %ld w %u h %u xoff %d yoff %d\n",
			   cc, tfm, w, h, hoff, voff);
	if (traceflag != 0) {
		printf("width %lg ", width);
		printf("w %d h %d hoff %d voff %d dynf %d ", w, h, hoff, voff, dynf);
	}
	
/*	if (commentflag != 0)  {
		fprintf(outfile, "width %lg ", width);
		fprintf(outfile, "w %d h %d hoff %d voff %d\n", w, h, hoff, voff);
	} */
	
	if (pkcompensate == 0) voff = voff + 1;			/* 1995/Dec/10 */
	adjustbbox(w, h, hoff, voff);
/*	fprintf(outfile, "/a%d %lg %d %d %d %d\n", cc, width, w, h, hoff, voff);*/
	if (traceflag)
	printf("/a%d %lg %d %d %d %d\n", cc, width, w, h, hoff, voff);

	if (dynf == 14) readbits(infile, w, h);
	else readpacked(infile, dynf, black, w, h);
		
/*	fprintf(outfile, " %d makechar\n", cc); */

/*	for (i = 0; i < pl - 8; i++) getc(infile);  skip over char - short */
/*	for (i = 0; i < pl - 13; i++) getc(infile); skip over char - long */

/*	if (verboseflag != 0) putc('\n', stdout); */

/*	putc('\n', outfile); */

}

void longform(FILE *infile, int lencat, int dynf, int black) {
	long hoff, voff;
	unsigned long dx, dy, w, h;
	int dm;					/* not accessed */
	unsigned long i;
	unsigned long pl, cc, tfm;

	pl = ureadfour(infile);		/* packet length */
	cc = ureadfour(infile);		/* character code */
	if (traceflag)
		printf("Long form - packet length: %lu - char code %ld\n", pl, cc);
	if (cc > INFINITY) {
		fprintf(stderr, "Ridiculously large char code %ld\n", cc);
	}
	chrs = (int) cc; chrcount++;
	if (chrs > maxchrs) maxchrs = chrs;
/*	copyflag = wantchrs[chrs]; */
	copyflag = 1;
	if (copyflag != 0) chrproc++;
	
/*	if (verboseflag != 0) printf("Long Form "); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0)
		fprintf(outfile, "%% Long Form "); */
	if (dynf == 14) {
/*		if (verboseflag != 0) printf("Bits ");		 */
/*		if (commentflag != 0 && prepass == 0 && copyflag != 0) 
			fprintf(outfile, "Bits "); */
	}
	if (lencat != 0) {		/* problem if this happens ? */
		fprintf(stderr, "WARNING: Lencat in long form not zero ");
		warnflag++;
	}

	if (warnflag != 0 && prepass == 0) {
		fprintf(stderr, "%ld\n", cc);
		warnflag = 0;
	}
	
/*	pl = (((long) lencat) << 16) + pl; */
/*	if (verboseflag != 0) printf("char %ld ", cc); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0) 
		fprintf(outfile, "char %ld ", cc); */

	if (prepass != 0 || copyflag == 0) {
		for (i = 0; i < pl; i++) (void) getc(infile);  /* skip over char */
		return;
	}

	tfm = ureadfour(infile);		/* horizontal escapement */
	dx = ureadfour(infile);			/* escapement in pixels * 2^16 */
	dy = ureadfour(infile);			/* escapement in pixels * 2^16 */
	w = ureadfour(infile);			/* width of bbox in pixels */
	h = ureadfour(infile);			/* height of bbox in pixels */
	hoff = sreadfour(infile);
	voff = sreadfour(infile);

	if ((w > INFINITY) || (h > INFINITY) || 
		(hoff > INFINITY) || (hoff < -INFINITY) ||
		(voff > INFINITY) || (voff < -INFINITY)) {
		fprintf(stderr, "Ridiculously large parameters in char %ld\n", cc);
	}
	if (dy != 0) {
		fprintf(stderr, "Can't handle vertical escapment dy %ld\n", dy);
	}
/*	dm = (int) (dx >> 16); */		/* ??? */
	dm = (int) ((dx + 32767) >> 16);		/* 96/Mar/18 ??? */
	pkdm = dm;

	commonform(infile, (unsigned int) cc, tfm, 
		(unsigned int) w, (unsigned int) h, (int) hoff, (int) voff,
			dynf, black);
}

void extendedform(FILE *infile, int lencat, int dynf, int black) {
	int cc, hoff, voff;
	unsigned int dm;			/* not accessed */
	unsigned int w, h;
	long i, pl;
	unsigned long tfm;

	pl = (long) ureadtwo(infile);   /* packet length */
	cc = getc(infile);	/* character code */
	if (traceflag)
		printf("Extended form - packet length: %lu - char code %d\n", pl, cc);
	chrs = cc; chrcount++;
	if (chrs > maxchrs) maxchrs = chrs;
/*	copyflag = wantchrs[chrs]; */
	copyflag = 1;
	if (copyflag != 0) chrproc++;

/*	if (verboseflag != 0) printf("Extended Form "); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0)
		fprintf(outfile, "%% Extended Form "); */
	if (dynf == 14) {
/*		if (verboseflag != 0) printf("Bits ");	 */
/*		if (commentflag != 0 && prepass == 0 && copyflag != 0) 
			fprintf(outfile, "Bits "); */
	}
	if (warnflag != 0 && prepass == 0) {
		fprintf(stderr, "%d\n", cc);
		warnflag = 0;
	}
	
	pl = (((long) lencat) << 16) + pl;
/*	if (verboseflag != 0) printf("char %d ", cc); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0) 
		fprintf(outfile, "char %d ", cc); */

	if (prepass != 0 || copyflag == 0) {
		for (i = 0; i < pl; i++) (void) getc(infile);  /* skip over char */
		return;
	}

	tfm = ureadthree(infile);		/* horizontal escapement */
	dm = ureadtwo(infile);			/* escapement in pixels ? */
	w = ureadtwo(infile);			/* width of bbox in pixels */
	h = ureadtwo(infile);			/* height of bbox in pixels */
	hoff = sreadtwo(infile);
	voff = sreadtwo(infile);

	pkdm = dm;

	commonform(infile, cc, tfm, w, h, hoff, voff, dynf, black);
}

void shortform(FILE *infile, int lencat, int dynf, int black) {
	int i, pl, cc, w, h, hoff, voff;
	int dm;				/* not accessed */
	unsigned long tfm;

	pl = getc(infile);  /* packet length */
	cc = getc(infile);	/* character code */
	if (traceflag && prepass == 0)
		printf("Short form - packet length: %d - char code %d\n", pl, cc);
	chrs = cc; chrcount++;
	if (chrs > maxchrs) maxchrs = chrs;
/*	copyflag = wantchrs[chrs]; */
	copyflag = 1;
	if (copyflag != 0) chrproc++;

/*	if (verboseflag != 0) printf("Short Form "); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0) 
		fprintf(outfile, "%% Short Form "); */
	if (dynf == 14) {
/*		if (verboseflag != 0) printf("Bits ");		 */
/*		if (commentflag != 0 && prepass == 0 && copyflag != 0) 
			fprintf(outfile, "Bits "); */
	}
	if (warnflag != 0 && prepass == 0) {
		fprintf(stderr, "%d\n", cc);
		warnflag = 0;
	}
	
	pl = (lencat << 8) + pl;
/*	if (verboseflag != 0) printf("char %d ", cc); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0) 
		fprintf(outfile, "char %d ", cc); */

	if (prepass != 0 || copyflag == 0) {
		for (i = 0; i < pl; i++) (void) getc(infile);  /* skip over char */
		return;
	}

	tfm = ureadthree(infile);	/* horizontal escapement */
	dm = getc(infile);			/* escapement in pixels ? */
	w = getc(infile);			/* width of bbox in pixels */
	h = getc(infile);			/* height of bbox in pixels */
	hoff = sreadone(infile);
	voff = sreadone(infile);

	pkdm = dm;

/*	don't need escapement in pixels - dm ? */
	commonform(infile, cc, tfm, w, h, hoff, voff, dynf, black);
}

/* Unfortunately don't know character code yet to use in error message */

void do_char(FILE *infile, int flag) {
	int dynf, black, longflag, twobyte, lencat;
	double width, pxlwidth;

	warnflag = 0;
	dynf = flag >> 4;
	if (dynf > 14 || dynf < 0) {	/* basically impossible */
		if (noiseflag != 0 && prepass == 0) {
			fprintf(stderr, "WARNING: Invalid dynf value %d char ", dynf);
			warnflag++;
		}
	}
	if ((flag & 7) == 7) longflag = 1;	else longflag = 0;
	black = (flag >> 3) & 1;		/* zero for bitmapped chars */
	if (dynf == 14 && black != 0) {
		if (noiseflag != 0 && prepass == 0) { /* not serious */
			fprintf(stderr, "WARNING: Black run start for bitmap char "); 
			warnflag++;
		}
	}
	twobyte = (flag >> 2) & 1;	
	lencat = flag & 3;
	if (lencat != 0) { /* normal, actually */
/*		if (noiseflag != 0 && prepass == 0) {
			fprintf(stderr, "WARNING: Lencat bits nonzero %d char ", lencat);
			warnflag++; 
		} */
	}
	schar = buffer;
	if (longflag != 0) longform(infile, lencat, dynf, black);
	else if (twobyte != 0) extendedform(infile, lencat, dynf, black);
	else shortform(infile, lencat, dynf, black);
/*	add parameters in same form as they appear in DVIPS output */
	*schar++ = (unsigned char) pkw;				/* buffer[nlen-5] */
	*schar++ = (unsigned char) pkh;				/* buffer[nlen-4] */
	*schar++ = (unsigned char) (128 - pkhoff);	/* buffer[nlen-3] */
	*schar++ = (unsigned char) (127 + pkvoff);	/* buffer[nlen-2] */
	*schar++ = (unsigned char) pkdm;			/* buffer[nlen-1] */
	pklen = schar - buffer;

	if (showflag != 0 && prepass == 0)
		showhexstring(buffer, (schar - buffer), pkw, pkcc);
/*	why do we come by here so often with same values ? */
	if (checkpktfm != 0 && prepass == 0) {
		width = 1000.0 * ((double) pktfm) / 1048576.0;
		pxlwidth = width * ptsize / 1000.0 / 72.27 * dpih;
/*  also show what pk file working on ? */
		if ((int) (pxlwidth + 0.5) != pkdm) {
			printf("%s %d (%c)\tTFM %ld (%lg)\tadv %d (%lg) (%d x %d) %s%s%s\n",
			   (kindex >= 0) ? psfontnames[kindex] : psfontnames[fontindex],
			   pkcc, (pkcc > 32 && pkcc < 128) ? pkcc : ' ',
				   pktfm, width, pkdm, pxlwidth,
			   (int) (dpih+0.5), (int) (dpiv+0.5),
				  longflag ? "L" : "", twobyte ? "E" : "",
				  (longflag == 0 && twobyte == 0) ? "S" : "");
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int pkanal(FILE *infile, int flag) {
	int c;
	long place;
	long count;
	
	prepass = 0;
	fxll = SINFINITY; fyll = SINFINITY; fxur = -SINFINITY; fyur = -SINFINITY;
	c = getc(infile);	/* pre = 247 */
	if (c != pre) {
		fprintf(stderr, "Not a PK file\n");
		return -1;
	}
	else do_pre(infile);
	chrcount = 0;
	chrproc = 0;
	maxchrs = 0;
	leftover = -1;
/*	skipflag = 0; */
	while ((c = getc(infile)) != EOF) {
		if (c < 240) {
			do_char(infile, c);
			if (flag) {
				count = checkchar(pkcc);
				if (rejectbad && count < 0) {
/*					if (showrejects) printf("*"); *//* bad mismatch */
					return -1;						/* skip out early */
				}
			}
		}
		else if (c == post) break;
		else if (c == nop) continue;
		else if (c == xxx1) do_xxx1(infile);
		else if (c == xxx2) do_xxx2(infile);
		else if (c == xxx3) do_xxx3(infile);
		else if (c == xxx4) do_xxx4(infile);
		else if (c == yyy) do_yyy(infile);		
		else {
			fprintf(stderr, "Unrecognized command code %d ", c);
			place = ftell(infile);
			fprintf(stderr, "at byte %ld in PK file\n", place);
			return -1;
		}
/*		if (bAbort > 0) abortjob(); */			/* 1992/Nov/24 */
	}
	if (c == EOF) {
		fprintf(stderr, "Premature EOF\n");
		exit(1);
	}
/*	if (verboseflag != 0) printf("Struck POST\n"); */
/*	writetrailer(outfile); */
	if (traceflag != 0) showstatistics();
	return 0;
}

void preanal(FILE *infile) {	/* scan for character codes only */
	int c;
/*	long place; */
	
	prepass = -1;
/* 	fxll = SINFINITY; fyll = SINFINITY; fxur = -SINFINITY; fyur = -SINFINITY; */
	chrcount = 0;
	chrproc = 0;
	maxchrs = 0;
/*	leftover = -1; */
/*	skipflag = 0; */
	c = getc(infile);	/* pre = 247 */
	if (c != pre) {
/*		fprintf(stderr, "Not a PK file\n"); */
		return;
	}
	else do_pre(infile);
	while ((c = getc(infile)) != EOF) {
		if (c < 240) {
			do_char(infile, c);
		}
		else if (c == post) break;
		else if (c == nop) continue;
		else if (c == xxx1) do_xxx1(infile);
		else if (c == xxx2) do_xxx2(infile);
		else if (c == xxx3) do_xxx3(infile);
		else if (c == xxx4) do_xxx4(infile);
		else if (c == yyy) do_yyy(infile);		
		else {
/*			fprintf(stderr, "Unrecognized command code %d ", c);
			place = ftell(infile);
			fprintf(stderr, "at byte %ld in PK file\n", place); */
			return;
		}
/*		if (bAbort > 0) abortjob(); */			/* 1992/Nov/24 */
	}
/*	if (verboseflag != 0) printf("Struck POST\n"); */
/*	writetrailer(outfile); */
/*	if (verboseflag != 0) showstatistics(); */
}

int processpkfile (FILE *input, int flag) {
	if (traceflag) printf("Preanalyzing\n");
	preanal(input);
	numchar = maxchrs + 1;
	if (traceflag) printf("Analyzing (max char code %d)\n", maxchrs);
	rewind(input);
	if (pkanal(input, flag) < 0) return -1;	/* bad mismatch */
	return 0;
}

/*****************************************************************************/

/* stuff for reading .tfm files */ /* OK for new form TFM files ? OK */

/* int sreadtwo (FILE *infile) {
	short int result;
	result = ((short int) getc(infile) << 8) | (short int) getc(infile);
	return result;
} */

/* long sreadfour (FILE *infile) {
	int c, d, e, f;
	c = getc(infile);	d = getc(infile);
	e = getc(infile);	f = getc(infile);
	return ((((((long) c << 8) | (long) d) << 8) | e) << 8) | f;
} */

void showwidths (long __far widths[], int bc, int ec, int nw,
			unsigned long dsgnsize, unsigned long chksum) {
	int k;
	long width;
	
	printf("Design Size %lgpt CheckSum %04lX with %d distinct widths\n",
		   (double) dsgnsize / 1048576.0, chksum, nw);
	for (k = bc; k < ec+1; k++) {
		width = widths[k];
/*		This gives the widths as in the AFM file */
		printf("%d\t%ld\t%lg\n", k, width,
/*			   (double) width / (double) dsgnsize * 1000.0); */
			   (double) width / 1048576.0 * 1000.0);
	}
}

/* lf, lh, nw, nh, nd, ni, nl, nk, ne are numbers of words */

/* width code of zero implies not valid char (section 543 TeX the program) */

/* returns zero if error in TFM file */

int readtfm (FILE *fp_tfm, long __far widths[], char *font) {
	long qwidths[MAXCHRS];  /* width table - `fixword' = width / 2^20 */
	int lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np;
	int k;
	int wdinx;
/*	int chksum, dsgnsize; */
/*	unsigned long chksum, dsgnsize; */
/*	int hdinx, itinx, rdinx;	*/

/*	for (k = 0; k < MAXCHRS; k++) widths[k] = INVALIDCHAR; */

/*	read header information */
	lf = sreadtwo(fp_tfm);	lh = sreadtwo(fp_tfm);
	bc = sreadtwo(fp_tfm);	ec = sreadtwo(fp_tfm);
	nw = sreadtwo(fp_tfm);	nh = sreadtwo(fp_tfm);
	nd = sreadtwo(fp_tfm);	ni = sreadtwo(fp_tfm);
	nl = sreadtwo(fp_tfm);	nk = sreadtwo(fp_tfm);
	ne = sreadtwo(fp_tfm);	np = sreadtwo(fp_tfm);
/*	first try and make sure this is a TFM file ! */
	if (lf < 0 || lh < 0 || nw < 0 || nw > 255 || 
		bc < 0 || ec < 0 || ec > 255 || bc > ec + 1 || 
		lf != 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np)
		{
		fprintf(stderr, " Inconsistent TFM information for %s", font);
		putc('\n', stderr);
/*		fprintf(stderr, "Header: %d %d %d %d %d %d %d %d %d %d %d %d\n",
			lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np); */
		return 0;
	}
/*	now for the header */
/*	(void) sreadtwo(fp_tfm); */		/* check sum */
	chksum = ureadfour(fp_tfm); 		/* should we bother to verify ? */
/*	(void) sreadtwo(fp_tfm); */	/* design size */
	dsgnsize = ureadfour(fp_tfm);
/*	discard rest of header */
	for (k = 2; k < lh; k++) {	
		(void) getc(fp_tfm); (void) getc(fp_tfm);	
	}
	if (ec >= MAXCHRS) {
		fprintf(stderr, "ERROR: ec (%d) > MAXCHRS (%d) in font %s\n",
				ec, MAXCHRS, font);
	}
	if (nw >= MAXCHRS) {
		fprintf(stderr, "ERROR: nw (%d) > MAXCHRS (%d) in font %s\n",
				nw, MAXCHRS, font);
	}
/*	now read the actual widths */
	fseek(fp_tfm, (long) ((6 + lh + (ec - bc + 1)) << 2), SEEK_SET);
	for (k = 0; k < nw; k++) {
		qwidths[k] = sreadfour(fp_tfm);
/*		character width in design size units * 2^20 */
	}
	if (qwidths[0] != 0) {	/* qwidths[0] is supposed to be zero */
		fprintf(stderr, " Inconsistent TFM information for %s", font);
		fputs(" - ", stderr);
		fprintf(stderr, "width[0] %ld (not zero)", qwidths[0]);
		return 0;
	}
	for (k = 0; k < bc; k++) widths[k] = INVALIDCHAR;
	for (k = ec+1; k < MAXCHRS; k++) widths[k] = INVALIDCHAR;
/*	now go back and read character information */
	fseek(fp_tfm, (long) ((6 + lh) << 2), SEEK_SET);
	for (k = bc; k <= ec; k++) {
		wdinx = getc(fp_tfm);	/* this it the one we want */
/*		we don't care about height, italic correction, remainder */
		(void) getc(fp_tfm);	/*		hdinx = getc(fp_tfm);  */
		(void) getc(fp_tfm);	/*		itinx = getc(fp_tfm);  */
		(void) getc(fp_tfm);	/*		rdinx = getc(fp_tfm);  */
		if (wdinx >= nw) {
			fprintf(stderr, " Inconsistent TFM information for %s", font);
			fputs(" - ", stderr);
			fprintf(stderr, "Width index %d (char %d) > width table %d",
				wdinx, k, nw);
			return 0;
		}
		if (wdinx == 0) widths[k] = INVALIDCHAR;
		else widths[k] = qwidths[wdinx];		/* look up width */
	}
	if (debugflag) showwidths(widths, bc, ec, nw, dsgnsize, chksum);
/*	if (traceflag != 0) printf("End of tfm file \n"); */
	return (int) (ec + 1);
}

/* returns number of TFMs successfully read */

int readalltfms(char *tfmdir) {
	char tfmfile[MAXFILENAME];
	FILE *tfminput;
	int count=0;
	int k, flag;
	long __far *newwidths;

	if (verboseflag) printf("Reading in TFM metric information\n");
	for (k = 0; k < MAXMETRICS; k++) {		/* step through tfm files */
		if (strcmp(psfontnames[k], "") == 0) break;
		strcpy(tfmfile, tfmdir);
		strcat(tfmfile, "\\");
		strcat(tfmfile, psfontnames[k]);
		strcat(tfmfile, ".");
		strcat(tfmfile, tfmext);
		if (traceflag) printf("Opening %s\n", tfmfile);
		if ((tfminput = fopen(tfmfile, "rb")) == NULL) {
			if (traceflag) perror(tfmfile);
/*			exit(1); */
			continue;
		}
		newwidths = (long __far *) xfmalloc(MAXCHRS * sizeof(long));
		flag = readtfm (tfminput, newwidths, tfmfile);
		fclose(tfminput);
		if (flag > 0) {
			metrics[k] = newwidths;
			designsizes[k] = dsgnsize;
			count++;
		}
		else {					/* bad TFM file */
			_ffree(newwidths);
			metrics[k] = NULL;
		}
/*		break; */					/* for debugging do just one */
	}
	if (verboseflag) printf("Found metric information for %d fonts\n", count);
	return count;
}

/*****************************************************************************/

#define MAXROW 256

unsigned char rw[MAXROW];	/* place to build row */

int gp;						/* pointer into source string */
int cp;						/* column pointer */
int rc;						/* row repeat count */
int endrow;					/* flag to indicate hit end of row */
int cplast;					/* how far we got in last row */

unsigned int outnlen, outk;

/* new stuff for dealing with compressed bitmaps in latest versions of DVIPS */

void printhex (int n) {		/* debugging output */
	int c,d;
	c = (n >> 4) & 15;
	d = n & 15;
	if (c < 10) c = c + '0';
	else c = c - 10 + 'A';
	if (d < 10) d = d + '0';
	else d = d - 10 + 'A';
	printf("%c%c ", c, d);
}

void adv(int count) {		/* advance column */
	cp += count;
	if (count > 17) fprintf(stderr, "COUNT ERROR %d ", count);
	if (debugflag) printf("adv %d ", count);
	if (cp >= MAXROW) {
		fprintf(stderr, "Exceeded row space\n");
		exit(1);
	}
}

void chg(int count) {		/* get character data */
	int k;
	if (count > 17) fprintf(stderr, "COUNT ERROR %d ", count);
	if (debugflag) printf("chg %d ", count);
	for (k = 0; k < count; k++)	{
		rw[cp+k] = buffer[gp+k];
		if (debugflag) printhex(rw[cp+k]);
	}
	gp += count;
	adv(count);
}

void nd(void) {				/* reset column pointer and end row */
	if (debugflag) printf("nd ");
	cplast = cp;			/* remember how far we got on that one */
	cp = 0;
	endrow = 1;
}

void lsh(void) {			/* left shift */
	int n = rw[cp], m;
	if (debugflag) printf("lsh %d ", n);
	if (n == 0) m = 1;
	else {
		if (n == 255) m = 254;
		else {
			m = ((n * 2) & 255) | (n & 1);
		}
	}
	if (debugflag) printhex(m);
	rw[cp] = (unsigned char) m;
	adv(1);
}

void rsh(void) {			/* right shift */
	int n = rw[cp], m;
	if (debugflag) printf("rsh %d ", n);
	if (n == 0) m = 128;
	else {
		if (n == 255) m = 127;
		else {
			m = (n / 2) | (n & 128);
		}
	}
	if (debugflag) printhex(m);
	rw[cp] = (unsigned char) m;
	adv(1);
}

void clr(int count) {		/* n zero bytes */
	int k;
	if (count > 17) fprintf(stderr, "COUNT ERROR %d ", count);
	if (debugflag) printf("clr %d ", count);
	for (k = 0; k < count; k++) rw[cp+k] = 0;
	adv(count);
}

/* original actually has a bug in that it gets string one too long */

void set(int count) {		/* n all one bytes */
	int k;
	if (count > 17) fprintf(stderr, "COUNT ERROR %d ", count);
	if (debugflag) printf("set %d ", count);
	for (k = 0; k < count; k++) rw[cp+k] = 255;
	adv(count);
}

void setrc (int count) {
	if (debugflag) printf("set rc %d ", count);	/* repeat count */
	rc = count;
}

/* void decompressglyph(unsigned char *buffer, int nlen) { */
unsigned int decompressglyph(unsigned char *outbuf, unsigned char *buffer, int nlen) {
	int c, op, count, k, m, width, height, nw;
	unsigned int nbytes;

	width = buffer[nlen-5];				/* width in bits */
	height = buffer[nlen-4];			/* height in bits */
	nw = (width + 7) / 8;				/* width in bytes */
	nbytes = ((width + 7) / 8) * height;	/* byte in decompressed string */
	gp = 0;								/* index into source bytes */
	outk = 0;							/* index into decompressed bytes */
	for (k = 0; k < nw; k++) rw[nw] = 0;	/* reset rw initially to 0 */
	for (;;) {							/* until exhaust source bytes */
		endrow=0;
		rc = 0;							/* reset row repeat count */
		while (endrow == 0) {
/*			c = *buffer++; */
			c = buffer[gp++];			/* grab next source byte */
			op = c / 18;			/* op code 0, 1, ... 13 */
			count = c % 18;			/* byte count 0, 1, ... 17 */
			if (debugflag) printf("\ncount %d op %d\n", count, op);
			switch (op) {
				case 0:
					adv(count);		/* skip count bytes, insert 1 byte */
					chg(1);
					break;
				case 1:
					adv(count);
					chg(1);
					nd();			/* and end row */
					break;
				case 2:
					chg(count+1);	/* insert count+1 source bytes */
					break;
				case 3:
					chg(count+1);
					nd();			/* and end row */
					break;
				case 4:
					adv(count);		/* skip count bytes and left shift */
					lsh();
					break;
				case 5:
					adv(count);
					lsh();
					nd();			/* and end row */
					break;
				case 6:
					adv(count);		/* skip count bytes and right shift */
					rsh();
					break;
				case 7:
					adv(count);
					rsh();
					nd();			/* and end row */
					break;
				case 8:
					adv(count+1);	/* skip count+1 bytes */
					break;
				case 9:
					setrc(count);
					nd();
					break;
				case 10:
					set(count+1);	/* count+1 all one bytes */
					break;
				case 11:
					clr(count+1);	/* count+1 all zero bytes */
					break;
				case 12:
					adv(count);		/* skip count bytes get 2 source bytes */
					chg(2);
					break;
				case 13:
					adv(count);
					chg(2);
					nd();			/* and end row */
					break;
				case 14:
/*					printf("END? "); */
					nd();			/* end row, ignore count */
					break;
				default:
					fprintf(stderr, "Impossible op code!\n");
					break;
			}	/* end of switch */
		}	/* end of while (endrow == 0) */
		if (debugflag) putc('\n', stdout);
		for (m = 0; m < rc+1; m++) {
			if (debugflag) {
				printf("ENDROW: ");
/*				for (k = 0; k < cplast; k++) printhex(rw[k]); */
				for (k = 0; k < nw; k++) printhex(rw[k]);
				putc('\n', stdout);
			}
/*			transfer to output buffer */
			for (k = 0; k < nw; k++) outbuf[outk+k] = rw[k];
			outk += nw;
		}
		if (gp >= nlen-5) {
			if (debugflag)
				printf("Exhausted compressed data %d %d\n", gp, nlen);
			for (k = 0; k < 5; k++) outbuf[outk+k] = buffer[nlen-5+k];
			outk += 5;
			outnlen = outk;
			if (verboseflag)
				showhexstring(outbuf, outnlen, width, cc);
			if (outnlen != nbytes + 5) {
				fprintf(stderr, "ERROR: decompressed to %d instead of %d\n",
						outnlen-5, nbytes);
			}				
			putc('\n', stdout);
			break;
		}
	}	/* end of loop over rows */
	return outnlen;
}

/*****************************************************************************/

/* c:\pctex\pixel\dpi300\cmr10.pk */

/* c:\tex\cmpixel\canon300\dpi1075\cmssi8.pk */

/* Skip out when found exact match */

/* Do the more likely ones first CMR10, CMBX10, CMTI10, CMMI10, CMSY10 */

/* Avoid the big ones CMINCH --- look at file size ? */

/* Put buffer in far space */

/* Write out log file - editable */

/* Check for T1 fonts in DVIPS PS output */

/* skip out early when bad mismatch or PK file has no char in code used */

/* compareglyph: allow for mismatch of plus or minus one in rows or columns */

/* problems: damn mode_def changes in METAFONT generation of PK files */

/* problems: new DVIPS bitmap compression */

/* problems: METAFONT advance widths in PK files are not rounded TFM width */

/* slanted versions of fonts have same advance widths as roman version */

/* fixed width fonts match all fixed width fonts */

/* fonts with single character match virutally everything */
