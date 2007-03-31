/* pfatoafm.c
   Copyright Y&Y 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000
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

#pragma warning(disable:4032)	// conio.h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <io.h>						/* for mktemp ? */
#include <signal.h>
#include <math.h>					/* for sqrt */
#include <malloc.h>
#include <conio.h>

#include "metrics.h"

#ifdef _WIN32
#define __far 
#define _fmalloc malloc
#define _frealloc realloc
#define _ffree free
#endif

/* #define MAXDEPTH 4 */

#define MAXDEPTH 6

#define TOOLARGE 4096

/* #define MAXADJUST (MAXCHRS+MAXCHRS) */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int debugflag = 0;
int extremaflag = 0;	/* show interior extrema in curveto 95/Jun/26 */
int wipeclean = 1;	
int dotsflag = 0;
int quietflag = 0;
int sortencflag = 1;	/* default is to sort encoded characters */
int sortunencflag = 1;	/* default to sort unencoded characters */
int sortcompositeflag = 1; /* default to sort composite characters */
int unixflag = 0;		/* using `newline' as line terminator */
int wxroundflag = 0;	/* round the widths ? */
int approxflag = 0;		/* just use control points of curveto's */
int keepoldorder = 0;	/* kep CapHeight, XHeight etc in same place */
int flushcontrol = 0;	/* ignore encoding vector 0 - 31 and 128 - 159 */
int addligatures = 0;	/* add `standard' TeX ligatures */
int individualbboxes = 0;	/* show individual bounding boxes 2000 July 15 */

int forceinteger=1;		/* force xheight etc to be integers */

int snaptoabs=1;		/* snap to in final FlxProc calls */

int useoldafm = 0;		/* current value */
int useoldafmini = 0;	/* command line value */

int flxeps = 2;			/* maximum drift in FlxProc before complaining */

int depth = 0;			/* current depth in subroutine / seac calls */

int afmflag = 0;		/* non-zero means next arg is old AFM file name */

int extremacount;		/* how many curveto extrema not at end */

int extendedseac=0;

int isfixedpitch=0;		/* 97/Sep/28 */

int smallcapsflag=0;	/* 97/Sep/28 */

char *afmfile=NULL;		/* source AFM file name */

// char *oldafmfile=NULL;	// in case need to rename old file

double capheight, xheight, ascender, descender;

int standardflag = 0;	/* standard encoding */

int isolatin1flag = 0;	/* ISO Latin 1 encoding */

int nonstandard = 0;	/* if FontMatrix is not [0.001 0 0 0.001 0 0] */

// int renameflag = 0;

int eexecscan = 0;		/* scan up to eexec before starting */
int charscan = 0;		/* scan up to RD before starting */
int charenflag = 0;		/* non-zero charstring coding instead of eexec */
int decodecharflag = 0;	/* non-zero means also decode charstring */
int binaryflag = 0;		/* input is binary, not hexadecimal */

int extractflag = 1;	/* non-zero => extract info only */
int ignorelow = 1;		/* ignore Subrs 0, 1, 2, 3 */ /* 94/June/27 */

/* int	afmfileflag = 0; */

/* int pfafileflag = 1; */

volatile int bAbort = 0;			/* set when user types control-C */

extern int encrypt(FILE *, FILE *);

extern int decrypt(FILE *, FILE *);

char *tempdir="";		/* place to put temporary files */

double m11, m21, m12, m22, m13, m23;

int ncomposites=0;

/************************************************************************/

int maxcomposites=0;

/* char *composite[MAXCOMPOSITES]; */	/* huge - 1994/May/29 */
char * __far *composite=NULL;

/* int base[MAXCOMPOSITES]; */			/* base character code */
/* int accent[MAXCOMPOSITES]; */		/* accent character code */
short __far *base=NULL;
short __far *accent=NULL;

/* double pccx[MAXCOMPOSITES]; */
/* double pccy[MAXCOMPOSITES]; */

/* int pccx[MAXCOMPOSITES]; */			/* accent offset in x */
/* int pccy[MAXCOMPOSITES]; */			/* accent offset in y */
short __far *pccx=NULL;
short __far *pccy=NULL;

/************************************************************************/

char charname[CHARNAME_MAX];

int nsubrs=0;				/* obtained from /Subrs line */

int maxsubrs=0; 			/* number actually allocated */

/* long subrpos[MAXSUBRS]; */		/* start of Subr in file */

long __far *subrpos=NULL;			/* now gets allocated 97/June/20 */

int charcount=0, unencoded=0, notdefseen=0;

int ncharstrings=0;			/* obtained from /CharStrings line */

int maxcharstrings=0;		/* number actually allocated */

long __far *charstringpos=NULL;		/* new gets allocated 97/June/20 */

/* glyphs is an odd sort of beast, it is an array allocated in far space */
/* however, it contains pointers to character strings in near space */

char * __far *glyphs=NULL;			/* new gets allocated 97/June/20 */

int glyphindex = 0;			/* glyph count from start of CharStrings /

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	 
char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_bak[FILENAME_MAX], fn_afm[FILENAME_MAX];

long charpos[MAXCHRS];		/* indexed by standard encoding code number */

long rawcharpos[MAXCHRS];	/* indexed by encoding code number, if encoded */

char *encoding[MAXCHRS]; 

FILE *errout=stdout;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

/* Following real ligatures apply to proportional fonts, not fixed width */

char *ligatures1[][3]={
	{"i", "j", "ij"},		/* ??? even for smallcaps ? */
	{"I", "J", "IJ"},		/* ??? even for smallcaps ? */
	{"f", "b", "fb"},
	{"f", "f", "ff"},
	{"f", "i", "fi"},
	{"f", "j", "fj"},
	{"f", "k", "fk"},
	{"f", "l", "fl"},
	{"f", "t", "ft"},
	{"ff", "i", "ffi"},
	{"ff", "j", "ffj"},
	{"ff", "l", "ffl"},
	{"c", "t", "ct"},
	{"s", "t", "st"},
	{"", "", ""},					/* terminator */
};

/* Following pseudo ligatures apply to all fonts, including fixed width */

char *ligatures2[][3]={
	{"exclam", "quoteleft", "exclamdown"},
	{"question", "quoteleft", "questiondown"},
	{"quoteleft", "quoteleft", "quotedblleft"},
	{"quoteright", "quoteright", "quotedblright"},
	{"quotereversed", "quotereversed", "quotedblreversed"},
	{"comma", "comma", "quotedblbase"},	/* German and Polish opening quotes */
	{"less", "less", "guillemotleft"},			/* French opening quotes */
	{"greater", "greater", "guillemotright"},	/* French closing quotes */
	{"hyphen", "hyphen", "endash"},
	{"hyphen", "sfthyphen", "sfthyphen"},
	{"endash", "hyphen", "emdash"},
	{"", "", ""},					/* terminator */
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

char *standardencoding[] = { 
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright",
 "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
 "zero", "one", "two", "three", "four", "five", "six", "seven",
 "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
 "at", "A", "B", "C", "D", "E", "F", "G",
 "H", "I", "J", "K", "L", "M", "N", "O",
 "P", "Q", "R", "S", "T", "U", "V", "W",
 "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
 "quoteleft", "a", "b", "c", "d", "e", "f", "g",
 "h", "i", "j", "k", "l", "m", "n", "o",
 "p", "q", "r", "s", "t", "u", "v", "w",
 "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "quoteleft", "quoteright", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "exclamdown", "cent", "sterling", "fraction", "yen", "florin", "section",
 "currency", "quotesingle", "quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright", "fi", "fl",
 "", "endash", "dagger", "daggerdbl", "periodcentered", "", "paragraph", "bullet",
 "quotesinglbase", "quotedblbase", "quotedblright", "guillemotright", "ellipsis", "perthousand", "", "questiondown",
 "", "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
 "dieresis", "", "ring", "cedilla", "", "hungarumlaut", "ogonek", "caron",
 "emdash", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "AE", "", "ordfeminine", "", "", "", "",
 "Lslash", "Oslash", "OE", "ordmasculine", "", "", "", "",
 "", "ae", "", "", "", "dotlessi", "", "",
 "lslash", "oslash", "oe", "germandbls", "", "", "", "",
};

char *isolatin1encoding[] = { 
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright",
	"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
	"zero", "one", "two", "three", "four", "five", "six", "seven",
	"eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
	"at", "A", "B", "C", "D", "E", "F", "G",
	"H", "I", "J", "K", "L", "M", "N", "O",
	"P", "Q", "R", "S", "T", "U", "V", "W",
	"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
	"quoteleft", "a", "b", "c", "d", "e", "f", "g",
	"h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w",
	"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"dotlessi", "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
	"dieresis", "", "ring", "cedilla", "", "hungarumlaut", "ogonek", "caron",
	"", "exclamdown", "cent", "sterling", "currency", "yen", "brokenbar", "section",
	"dieresis", "copyright", "ordfeminine", "guillemotleft", "logicalnot", "hyphen", "registered", "macron",
	"degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu", "paragraph", "periodcentered",
	"cedilla", "onesuperior", "ordmasculine", "guillemotright", "onequarter", "onehalf", "threequarters", "questiondown",
	"Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE", "Ccedilla",
	"Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex", "Idieresis",
	"Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply",
	"Oslash", "Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls",
	"agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae", "ccedilla",
	"egrave", "eacute", "ecircumflex", "edieresis", "igrave", "iacute", "icircumflex", "idieresis",
	"eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis", "divide",
	"oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn", "ydieresis",
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

int wantcpyrght=1;

/* Font merging and normalizing program version */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* char *progname="PFAtoAFM"; */

char progname[16]=""; 

/* char *progversion="1.0"; */
/* char *progversion="1.0.1"; */
/* char *progversion="1.0.2"; */

/* char *progversion="1.1"; */
/* char *progversion="1.1.1"; */
/* char *progversion="1.1.2"; */
/* char *progversion="1.2"; */
/* char *progversion="1.2.1"; */
/* char *progversion="1.2.2"; */	/* 1996/June/16 */
/* char *progversion="1.3"; */		/* 1997/June/16 */
/* char *progversion="1.3.1"; */	/* 1997/July/8 */
/* char *progversion="2.0"; */		/* 1998/Dec/25 */
char *progversion="2.0.1";			/* 2000/July/15 */

char *copyright = "\
Copyright (C) 1993--2000  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* #define COPYHASH 10080614 */
/* #define COPYHASH 5036743 */
/* #define COPYHASH 15695102 */
/* #define COPYHASH 4801115 */
/* #define COPYHASH 10811092 */
/* #define COPYHASH 7682515 */
/* #define COPYHASH 8115016 */
#define COPYHASH 13820764

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void abortjob(void);

#ifdef IGNORE
static void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

static void forceexten(char *fname, char *ext) /* change extension if one present */
{
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}
#endif

static void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

static void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);	
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void cleanup(void);

char *zstrdup (char *name) {
	char *new = _strdup(name);
	if (new == NULL) {
		fprintf(stderr,
				"ERROR: Out of memory for character names (%s)\n", name);
		cleanup();
		exit(2);
	}
	return new;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void perrormod (char *s) {
//	perror (s);
	fprintf(errout, "%s: %s\n", s, strerror(errno));
}

static char line[MAXLINE];		/* buffer for getline */

static int getline(FILE *fp_in, char *line) {
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	return strlen(line); 
} 

int lookup(char *name) {	/* look up name in encoding */
	int k;

	if (strlen(name) == 1) {	/* single character ? */
		k = *name;
		if (encoding[k] != NULL) {
			if (strcmp(encoding[k], name) == 0) return k;
		}
	}		
	for (k = 0; k < MAXCHRS; k++) {
		if (encoding[k] == NULL) continue;
		if (strcmp(encoding[k], name) == 0) return k;
	}
	return -1;
}

int lookupstandard(char *name) {	/* look up name in standard encoding */
	int k;

	if (strlen(name) == 1) {	/* single character ? */
		k = *name;
		if (strcmp(standardencoding[k], name) == 0) return k;
	}		
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(standardencoding[k], name) == 0) return k;
	}
	return -1;
}

int lookupglyph(char *name) {	/* look up glyph ID */ /* 97/Jun/22 */
	int k;

	for (k = 0; k < glyphindex; k++) {
		if (glyphs[k] == NULL) continue;
		if (strcmp(glyphs[k], name) == 0) return k;
	}
	return -1;
}

void doligatures(FILE *fp_out, char *charname) {	/* 97/Sep/28 */
	int k;

	for (k= 0; k < 32; k++) {		/* pseudo ligatures */
		if (strcmp(ligatures2[k][0], "") == 0) break;
		if (strcmp(ligatures2[k][0], charname) == 0) {
			if (lookupglyph(ligatures2[k][1]) < 0) continue;
			if (lookupglyph(ligatures2[k][2]) < 0) continue;
			fprintf(fp_out, " L %s %s ;", ligatures2[k][1], ligatures2[k][2]);
		}
	}
	if (isfixedpitch) return;
	if (smallcapsflag) return;
	for (k= 0; k < 32; k++) {		/* real ligatures */
		if (strcmp(ligatures1[k][0], "") == 0) break;
		if (strcmp(ligatures1[k][0], charname) == 0) {
			if (lookupglyph(ligatures1[k][1]) < 0) continue;
			if (lookupglyph(ligatures1[k][2]) < 0) continue;
			fprintf(fp_out, " L %s %s ;", ligatures1[k][1], ligatures1[k][2]);
		}
	}
}

/**********************************************************************/

void freesubrs(void) {
	if (subrpos != NULL) free(subrpos);
	subrpos = NULL;
	maxsubrs = 0;
}

void allocsubrs(int nsubrs) {		
	int k;
	if (maxsubrs > 0) freesubrs();
	if (traceflag) {
		printf("Allocating space for %d Subrs\n", nsubrs);
		fflush(stdout);
	}
	subrpos = (long __far *) _fmalloc(nsubrs * sizeof(long));
	if (subrpos == NULL) {
		fprintf(errout, "Unable to allocate %ld bytes\n",
				nsubrs * sizeof(long));
		exit(1);
	}
	for (k = maxsubrs; k < nsubrs; k++) subrpos[k] = -1;
	maxsubrs = nsubrs;
}

void freecharstrings(void) {
	if (charstringpos != NULL) free(charstringpos);
	charstringpos = NULL;
	maxcharstrings = 0;
}

void  alloccharstrings(int ncharstrings) {
	int k;
	if (maxcharstrings > 0) freecharstrings();
	if (traceflag) {
		printf("Allocating space for %d Chars\n", ncharstrings);
		fflush(stdout);
	}
	charstringpos = (long __far *) _fmalloc(ncharstrings * sizeof(long));
	if (charstringpos == NULL) {
		fprintf(errout, "Unable to allocate %ld bytes\n",
				ncharstrings * sizeof(long));
		exit(1);
	}
	for (k = maxcharstrings; k < ncharstrings; k++) charstringpos[k] = -1;
	maxcharstrings = ncharstrings;
}

void freeglyphs (void) {
	if (glyphs != NULL) free(glyphs);
	glyphs = NULL;
/*	maxcharstrings = 0; */
}

void  allocglyphs(int ncharstrings) {
	int k;
	freeglyphs();
/*	maxcharstrings = 0; */
	if (traceflag) {
		printf("Allocating space for %d Glyphs\n", ncharstrings);
		fflush(stdout);
	}
	glyphs = (char * __far *) _fmalloc(ncharstrings * sizeof(char *));
	if (glyphs == NULL) {
		fprintf(errout, "Unable to allocate %ld bytes\n",
				ncharstrings * sizeof(char *));
		exit(1);
	}
/*	for (k = maxcharstrings; k < ncharstrings; k++) glyphs[k] = NULL; */
	for (k = 0; k < ncharstrings; k++) glyphs[k] = NULL;
/*	maxcharstrings = ncharstrings; */
/*	glyphindex = 0; */

}

void freecompositenames(void) {
	int k;
	if (composite == NULL) return;
	for (k = 0; k < maxcomposites; k++) {
		if (composite[k] != NULL) {
			free(composite[k]);
			composite[k] = NULL;
		}
	}
}

void freecomposites(void) {
	if (maxcomposites > 0) freecompositenames();
	if (composite != NULL) free(composite);
	composite = NULL;
	if (base != NULL) free(base);
	if (accent != NULL) free(accent);
	base = accent = NULL;
	if (pccx != NULL) free(composite);
	if (pccy != NULL) free(composite);
	pccx = pccy = NULL;
	maxcomposites = 0;
}

void alloccomposites(int ncomposites) {
	int k;
	if (maxcomposites > 0) freecomposites();
	if (traceflag) {
		printf("Allocating composite space for %d\n", ncomposites);
		fflush(stdout);
	}
	composite = (char * __far *) _fmalloc(ncomposites * sizeof(char *));
	base = (short __far *) _fmalloc(ncomposites * sizeof(short));
	accent = (short __far *) _fmalloc(ncomposites * sizeof(short));
	pccx = (short __far *) _fmalloc(ncomposites * sizeof(short));
	pccy = (short __far *) _fmalloc(ncomposites * sizeof(short));
	if (composite == NULL || base == NULL || accent == NULL ||
		pccx == NULL || pccy == NULL) {
		fprintf(errout, "Memory allocation error\n");
		exit(1);
	}
	for (k = maxcomposites; k < ncomposites; k++) composite[k] = NULL;
	maxcomposites = ncomposites;	/* first guess */
}

void realloccomposites(int ncomposites) {
	int k;
	if (verboseflag)
		printf("Growing composite space %d => %d\n", maxcomposites, ncomposites);
	composite = (char * __far *) _frealloc(composite, ncomposites * sizeof(char *));
	base = (short __far *) _frealloc(base, ncomposites * sizeof(short));
	accent = (short __far *) _frealloc(accent, ncomposites * sizeof(short));
	pccx = (short __far *) _frealloc(pccx, ncomposites * sizeof(short));
	pccy = (short __far *) _frealloc(pccy, ncomposites * sizeof(short));
	if (composite == NULL || base == NULL || accent == NULL ||
		pccx == NULL || pccy == NULL) {
		fprintf(errout, "Memory reallocation error\n");
		exit(1);
	}
	for (k = maxcomposites; k < ncomposites; k++) composite[k] = NULL;
	maxcomposites = ncomposites;
}

/**********************************************************************/

int firstflag;			/* non-zero before any x, y seen for bbox */

/* int width; */
double width;

double x, y;
double xll, yll, xur, yur;
double fontxll, fontyll, fontxur, fontyur;

/* round double to three decimal digits --- nearest 0.001 */

int iround (double x) {
	if (x == 0.0) return 0;
	if (x < 0.0) return - (int) (- x * 1000.0 + 0.49999);
	return (int) (x * 1000.0 + 0.49999);
}

/* void checkbbox(void) { */
int checkbbox (double x, double y) {
	double xd, yd;
	int flag = 0;
	
	if (nonstandard) {
		xd = (x * m11 + y * m12 + m13) * 1000.0;	/* 1000 ? */
		yd = (x * m21 + y * m22 + m23) * 1000.0;	/* 1000 ? */
		if (xd < xll) {xll = xd; flag++;}
		if (xd > xur) {xur = xd; flag++;}
		if (yd < yll) {yll = yd; flag++;}
		if (yd > yur) {yur = yd; flag++;}
	}
	else {
		if (x < xll) {xll = x; flag++;}
		if (x > xur) {xur = x; flag++;}
		if (y < yll) {yll = y; flag++;}
		if (y > yur) {yur = y; flag++;}
	}
	return flag;
}

int checkbboxnew (double x, double y) {	/* assumes we are in transformed */
	int flag = 0;
	
	if (x < xll) {xll = x; flag++;}
	if (x > xur) {xur = x; flag++;}
	if (y < yll) {yll = y; flag++;}
	if (y > yur) {yur = y; flag++;}
	return flag;
}

double fracround (int numer, int denom) {
	int positive=1;
	long k;
	double result;
	if (numer < 0) {
		numer = - numer;
		positive=0;
	}
	k = (long) numer * 1000;
	result = ((double) k + 0.4999) / (double) denom;
	result = result / 1000.0;
/*	printf("%d / %d = %lg\n", numer, denom, result); */
	if (positive) return result;
	else return - result;
}

int kround (double x) {				/* 1993/Dec/19 */
	if (x == 0.0) return 0;
	if (x < 0) return - (int) (-x + 0.49999);
	else return  (int) (x + 0.49999);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char buffer[MAXLINE];

/* following revised 1994/June/21 - copy new code to SAFESEAC and PFATOAFM */
/* NOTE: converts ratios to doubles */

void checkfordiv (char *line) {
	char *s, *t;
/*	long num, den, ratio; */
	double num, den, ratio;
	int k=0;
/*	char buffer[MAXLINE]; */

	if (strstr(line, "div") == NULL) return;			/* nothing to do */
	if (debugflag) printf("removing div: %s", line);
/*	fputs(line, stdout); */				/* debugging */
	while ((s = strstr(line, "div")) != NULL) {
/*		if (traceflag) printf("removin div: %s", line); */
		t = s + 3;							/* remember where */
		strcpy(buffer, t);					/* save the tail of this */
/*		printf("s = line + %d ", s - line); */	/* debugging */
/*		while (*s-- > ' ' && s > line) ; */	/* go back to space */
		while (*s > ' ' && s > line) s--;	/* go back to space */
/*		while (*s-- <= ' ' && s > line) ; */	/* go back over space */
		while (*s <= ' ' && s > line) s--;	/* go back over space */
		if (s == line) fprintf(errout, "Don't understand %s", line);
/*		while (*s-- > ' ' && s > line) ; */	/* go back to space */
		while (*s > ' ' && s > line) s--;	/* go back to space */
/*		while (*s-- <= ' ' && s > line) ; */	/* go back over space */
		while (*s <= ' ' && s > line) s--;	/* go back over space */
/*		while (*s-- > ' ' && s > line) ; */	/* go back to space */
		while (*s > ' ' && s > line) s--;	/* go back to space */
/*		if (s < line) s = line; */				/* 1993/Dec/19 */
/*		if (s > line) s++; */					/* step forward to space */
/*		if (s > line) s++; */					/* step forward past space */
		while (*s <= ' ') s++;				/* step forward again */
/*		printf("s = line + %d ", s - line); */	/* debugging */
/*		if (sscanf (s, "%ld %ld div", &num, &den) == 2) { */
		if (sscanf (s, "%lg %lg div", &num, &den) == 2) {
/*			if (num > 0) ratio = (num + den/2 - 1) / den;
			else ratio = - (-num + den/2 - 1) / den; */
			ratio = num / den; 		/* should be 16 bit at this point */
			sprintf(s, "%lg", ratio); 
		}
		else {
			fprintf(errout, "Can't make sense of: %s", line); /* 1993/Dec/20 */
			break;
		}
/*		(void) memmove (s + strlen(s), t, strlen(t)+1); */
		strcat (s, buffer);					/* put back the tail end */
		if (debugflag) {
			printf("num %lg den %lg ratio %lg: ", num, den, ratio);
			fputs(line, stdout);				
		}
/*		if (traceflag) printf("removed div: %s", line); */
		if (bAbort != 0) abortjob();
		if (k++ > 16) break;				/* emergency exit */
	}
	if (debugflag) printf("removed div: %s", line);
}

#ifdef IGNORE					/* old version */
void checkfordiv (char *line) {				/* 1993/Dec/19 */
	char *s, *t;
/*	long num, den, ratio; */
	double num, den, ratio;
	int k=0;
	char buffer[MAXLINE];

/*	fputs(line, stdout); */				/* debugging */
	while ((s = strstr(line, "div")) != NULL) {
		t = s + 3;							/* remember where */
		strcpy(buffer, t);					/* save the tail of this */
/*		printf("s = line + %d ", s - line); */	/* debugging */
		while (*s-- > ' ' && s > line) ;	/* go back to space before div*/
		while (*s-- <= ' ' && s > line) ;	/* go back over space to denom */
		while (*s-- > ' ' && s > line) ;	/* go back to space before denom */
		while (*s-- <= ' ' && s > line) ;	/* go back over space to numer  */
		while (*s-- > ' ' && s > line) ;	/* go back to space before numer */
		if (s < line) {
/*			fprintf(errout, "Stepped past beginning\n"); */
			s = line;
		}
		if (s > line) s++;					/* step forward to space */
		if (s > line) s++;					/* step forward past space */
/*		printf("s = line + %d ", s - line); */	/* debugging */
/*		if (sscanf (s, "%ld %ld div", &num, &den) == 2) { */
		if (sscanf (s, "%lg %lg div", &num, &den) == 2) {
/*			if (num > 0) ratio = (num + den/2 - 1) / den;
			else ratio = - (-num + den/2 - 1) / den; */
			ratio = num / den; 		/* should be 16 bit at this point */
/*			sprintf(s, "%ld", ratio); */
			if (forceinteger) sprintf(s, "%d", kround(ratio));
			else sprintf(s, "%lg", ratio);
		}
		else {
			fprintf(errout, "Can't make sense of: %s", line); /* 1993/Dec/20 */
			break;
		}
/*		(void) memmove (s + strlen(s), t, strlen(t)+1); */
		strcat (s, buffer);					/* put back the tail end */
		if (debugflag) {
			printf("num %lg den %lg ratio %lg: ", num, den, ratio);
			fputs(line, stdout);				
		}
		if (bAbort != 0) abortjob();
		if (k++ > 16) break;				/* emergency exit */
	}
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* made global to reduce stack usage - since this calls recursively */

double dx, dy;
double wx, wy;
double dx1, dy1, dx2, dy2, dx3, dy3;
double asb, bsb, adx, ady;
double flx, xabs, yabs;					/* 1994/June/4 */
double xacc, yacc;						/* 1995/June/25 */

/* for debugging mostly */

void showextrema(double xs, double ys, double xr, double yr,
				 double xf, double yf, double t, int flag) {
	printf("Extremum `%s' %lg %c%c (%lg %lg) (%lg %lg) (%lg %lg)\n",
		   charname, t, (flag > 1) ? 'Y' : 'X', (flag & 1) ? '-' : '+',
			   xs, ys, xr, yr, xf, yf); 
}

void checkcurveto (double dx1, double dy1, double dx2, double dy2,
				   double dx3, double dy3) {
	double ndx1, ndy1, ndx2, ndy2, ndx3, ndy3;
	double ax, bx, cx, xr;
	double ay, by, cy, yr;
	double delta, t;
	double nxs, nys, nxf, nyf;
	double xf, yf;

	xf = x + dx1 + dx2 + dx3;
	yf = y + dy1 + dy2 + dy3;
	checkbbox(xf, yf);

/*	could save some work here by first checking whether inner two knots */
/*	are inside bounding box so far, do nothing if so, since Bezier curve */
/*	lies inside convex quadrilateral defined by the four control points */

/*	really need to do following in transformed space if non-standard */

	if (nonstandard) {
		nxs = (x * m11 + y * m12 + m13) * 1000;
		nys = (x * m21 + y * m22 + m23) * 1000;
		ndx1 = (dx1 * m11 + dy1 * m12) * 1000;
		ndy1 = (dx1 * m21 + dy1 * m22) * 1000;
		ndx2 = (dx2 * m11 + dy2 * m12) * 1000;
		ndy2 = (dx2 * m21 + dy2 * m22) * 1000;
		ndx3 = (dx3 * m11 + dy3 * m12) * 1000;
		ndy3 = (dx3 * m21 + dy3 * m22) * 1000;
		nxf = (xf * m11 + yf * m12 + m13) * 1000;
		nyf = (xf * m21 + yf * m22 + m23) * 1000;
	}
	else {
		nxs = x; nys = y;
		ndx1 = dx1; ndy1 = dy1;
		ndx2 = dx2; ndy2 = dy2;
		ndx3 = dx3; ndy3 = dy3;
		nxf = xf; nyf = yf;
	}

	cx = 3 * ndx1;
	bx = 3 * (ndx2 - ndx1);
	ax = ndx3 - 2 * ndx2 + ndx1;
	cy = 3 * ndy1;
	by = 3 * (ndy2 - ndy1);
	ay = ndy3 - 2 * ndy2 + ndy1;

/*	Try for extrema in x first */
/*	delta = (bx * bx - 4 * ax * cx); */
	delta = (2 * bx * 2 * bx - 4 * 3 * ax * cx);
	if (ax != 0.0 && delta >= 0) {
/*		t = (- bx + sqrt (delta)) / (2 * ax); */
		t = (- 2 * bx + sqrt (delta)) / (2 * 3 * ax);
		if (t > 0 && t < 1) {
			xr = (((ax * t + bx) * t + cx)) * t + nxs;
			yr = (((ay * t + by) * t + cy)) * t + nys;
			if (checkbboxnew (xr, yr) != 0) {
				if (extremaflag) showextrema(nxs, nys, xr, yr, nxf, nyf, t, 0);
				extremacount++;
			}
		}
/*		t = (- bx - sqrt (delta)) / (2 * ax); */
		t = (- 2 * bx - sqrt (delta)) / (2 * 3 * ax );
		if (t > 0 && t < 1) {
			xr = (((ax * t + bx) * t + cx)) * t + nxs;
			yr = (((ay * t + by) * t + cy)) * t + nys;
			if (checkbboxnew (xr, yr) != 0) {
				if (extremaflag) showextrema(nxs, nys, xr, yr, nxf, nyf, t, 1);
				extremacount++;
			}
		}
	}

/*	Try for extrema in y first */
/*	delta = (by * by - 4 * ay * cy); */
	delta = (2 * by * 2 * by - 4 * 3 * ay * cy);
	if (ay != 0.0 && delta >= 0) {
/*		t = (- by + sqrt (delta)) / (2 * ay); */
		t = (- 2 * by + sqrt (delta)) / (2 * 3 * ay);
		if (t > 0 && t < 1) {
			xr = (((ax * t + bx) * t + cx)) * t + nxs;
			yr = (((ay * t + by) * t + cy)) * t + nys;
			if (checkbboxnew (xr, yr) != 0) {
				if (extremaflag) showextrema(nxs, nys, xr, yr, nxf, nyf, t, 2);
				extremacount++;
			}
		}
/*		t = (- by - sqrt (delta)) / (2 * ay); */
		t = (- 2 * by - sqrt (delta)) / (2 * 3 * ay);
		if (t > 0 && t < 1) {
			xr = (((ax * t + bx) * t + cx)) * t + nxs;
			yr = (((ay * t + by) * t + cy)) * t + nys;
			if (checkbboxnew (xr, yr) != 0) {
				if (extremaflag) showextrema(nxs, nys, xr, yr, nxf, nyf, t, 3);
				extremacount++;
			}
		}
	}

/*	x += dx1; y += dy1;
	x += dx2; y += dy2;
	x += dx3; y += dy3; */
	x = xf; y = yf;
}

int getbbox (FILE *input, int passflag, int compitflag) {
	int subr, newsubr;
/*	int numer, denom; */						/* 1994/June/27 */
/*	int dwidth;  */
	double dwidth; 
	int bchar, achar;
/*	double flx, xabs, yabs; */					/* 1994/June/4 */
/*	int compositewidth; */
	long present;
	
/*	some bugs noted in following, 1994/June/25 */

	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "endchar", 7) == 0) return 1;		/* EOC */
/*		if (strncmp(line, "return", 7) == 0) return 0; */
		if (strncmp(line, "return", 6) == 0) return 0;		/* in Subr */
/*		if (strncmp(line, "pop", 7) == 0) break; */			/* ignore */ 
		if (strncmp(line, "pop", 3) == 0) continue;			/* continue */
		if (strncmp(line, "setcurrentpoint", 15) == 0) continue;/* new? */
		if (strncmp(line, "callothersubr", 13) == 0) continue;	/* new? */
		if (strncmp(line, "|-", 2) == 0) break;
		if (strstr(line, "def") != NULL) break;
		if (strncmp(line, "|", 1) == 0) break;				/* subr */
		if (strstr(line, "put") != NULL) break;
		if (*line == '\n') break;
		if (strstr(line, "stem") != NULL) continue;			/* ignore hints */
		if (strstr(line, "dotsection") != NULL) continue;	/* ignore */
		if (strstr(line, "closepath") != NULL) {
			if (individualbboxes && *charname != '\0') {	// 2000 July 15
				printf("%s [%lg %lg %lg %lg] (%lg %lg) (%lg %lg)\n",
					   charname, xll, yll, xur, yur,
					   xur - xll, yur - yll, (xll + xur)/2, (yll + yur)/2);
				xll = TOOLARGE; yll = TOOLARGE;	xur = -TOOLARGE; yur = -TOOLARGE;
				firstflag = 1;
			}
			continue;	/* ignore */
		}
		checkfordiv (line);			/* moved here from below 94/June/27 */
		if (strstr(line, "callsubr") != NULL) {
/* new: first deal with FlxProc call - avoid going to Subr 50 ! */
/* format: flx xabs yabs 0 callsubr */ 
			if (sscanf (line, "%lg %lg %lg %d", &flx, &xabs, &yabs, &subr)
				== 4 ) {					/* catch the final FlxProc call */
				if (subr != 0) continue;	/* don't know what this is ! */
				if (xabs < x - flxeps || xabs > x + flxeps ||
					yabs < y - flxeps || yabs > y + flxeps) {
					if (verboseflag)
/* shouldn't happen with floating coordinates ! */
						fprintf(errout, "drift: (%lg %lg) != (%lg %lg) ",
							x, y, xabs, yabs);
				}
				if (snaptoabs) {
				x = xabs; y = yabs;		/* snap them to absolute */
				}
			}
/* new: hint switching calls via Subr 4 --- ignore */
/*			else if (sscanf (line, "%lg %d", &flx, &subr) == 2) { */
			else if (sscanf (line, "%d %d", &newsubr, &subr) == 2) {
				if (subr != 4) {
					if (verboseflag)
						fprintf(errout, "Not hint switch: %s", line);
				}
			}
			else if (sscanf (line, "%d", &subr) < 1) {
/* no Subr number on same line (as in Subr 4) */
			}
/*			else if (subr >= 0 && subr < MAXSUBRS) { */
			else if (subr >= 0 && subr < maxsubrs) {
				if (subrpos[subr] >= 0) {
/*					maybe only do this if subr > 3 ? */
					if (ignorelow == 0 || subr > 3) { 
						present = ftell(input);
						fseek (input, subrpos[subr], SEEK_SET);
/*						if (traceflag) printf("DOWN to   Subr %d\n", subr);*/
						if (depth++ > MAXDEPTH)
							fprintf(errout,
							"Possible recursion in call to Subr %d\n", subr);
						getbbox(input, passflag, compitflag);
						depth--;
/*						if (traceflag) printf("BACK from Subr %d\n", subr); */
						fseek (input, present, SEEK_SET);
					}
				}
				else fprintf(errout, "Empty Subr number: %d\n", subr);
			}
			else fprintf(errout, "Bad Subr number: %d\n", subr);
/*			continue; */					/* 1994/June/25 */
		}	/* end of callsubr case */
/*		checkfordiv (line);	*/		/* now done earlier higher up */

/*		if (strstr(line, "hsbw") != NULL) { */
		else if (strstr(line, "hsbw") != NULL) {
/*			if (sscanf (line, "%d %d", &dx, &dwidth) < 2) { */
			if (sscanf (line, "%lg %lg", &dx, &wx) == 2) {
				dwidth = wx;			/* normal case */
			}
			else {
				fprintf(errout, "Expecting <dx> <wx> hsbw, not %s", line);
			}
/* this shouldn't happen, since we use checkfordiv */
			if (strstr(line, "div") != NULL) {
				fprintf(errout, "Impossible div!\n");	/* 94/June/27 */
/*				if (sscanf (line, "%lg %d %d", &dx, &numer, &denom) == 3) { */
/*					dwidth = fracround(numer, denom); 
				} */ /* make nice number */
			}
			if (firstflag) {
				x = dx; y = 0; bsb = dx; width = dwidth;
			}
			else {
				x += dx;
/*				checkbbox(x, y); */ /* NO not on path 95/Jun/27 */
			}
/*			xll = x; yll = y; xur = x; yur = y; */
		}	/* end of hsbw case */
		else if (strstr(line, "sbw") != NULL) {
			if (sscanf (line, "%lg %lg %lg %lg", &dx, &dy, &wx, &wy) == 4) {
				dwidth = wx;
			}
			if (firstflag) {
				x = dx; y = dy; bsb = dx; width = dwidth;	/* dwidth ? */
			}
			else {
				x += dx; y += dy;
/*				checkbbox(x, y); */ /* NO not on path 95/Jun/27 */
			}
/*			xll = x; yll = y; xur = x; yur = y; */
		}	/* end of sbw case */
		else if (strstr(line, "rmoveto") != NULL ||
				 strstr(line, "rlineto") != NULL) {
			if (sscanf (line, "%lg %lg", &dx, &dy) < 2) {
			/* complain here : don't understand */
			}
			x += dx; y += dy;
			checkbbox(x, y);
		}
		else if (strstr(line, "hmoveto") != NULL ||
				 strstr(line, "hlineto") != NULL) {
			if (sscanf (line, "%lg", &dx) < 1) {
			/* complain here : don't understand */
			}
			x += dx;
			checkbbox(x, y);
		}
		else if (strstr(line, "vmoveto") != NULL ||
				 strstr(line, "vlineto") != NULL) {
			if (sscanf (line, "%lg", &dy) < 1) {
			/* complain here : don't understand */
			}
			y += dy;
			checkbbox(x, y);
		}
		else if (strstr(line, "rrcurveto") != NULL) {
			if (sscanf (line, "%lg %lg %lg %lg %lg %lg",
				&dx1, &dy1, &dx2, &dy2, &dx3, &dy3) < 6) {
			/* complain here : don't understand */
			}
			if (approxflag) {
				x += dx1; y += dy1;
				checkbbox(x, y);
				x += dx2; y += dy2;
				checkbbox(x, y);
				x += dx3; y += dy3;
				checkbbox(x, y);
			}
			else checkcurveto (dx1, dy1, dx2, dy2, dx3, dy3);
		}
		else if (strstr(line, "hvcurveto") != NULL) {
			if (sscanf (line, "%lg %lg %lg %lg",
				&dx1, &dx2, &dy2, &dy3) < 4) {
			/* complain here : don't understand */
			}
			if (approxflag) {
				x += dx1; 
				checkbbox(x, y);
				x += dx2; y += dy2;
				checkbbox(x, y);
				y += dy3;
				checkbbox(x, y);
			}
			else checkcurveto (dx1, 0, dx2, dy2, 0, dy3);
		}
		else if (strstr(line, "vhcurveto") != NULL) {
			if (sscanf (line, "%lg %lg %lg %lg",
				&dy1, &dx2, &dy2, &dx3) < 4) {
			/* complain here : don't understand */
			}
			if (approxflag) {
				y += dy1; 
				checkbbox(x, y);
				x += dx2; y += dy2;
				checkbbox(x, y);
				x += dx3;
				checkbbox(x, y);
			}
			else checkcurveto (0, dy1, dx2, dy2, dx3, 0);
		}
		else if (strstr(line, "seac") != NULL) {
			if (sscanf (line, "%lg %lg %lg %d %d",
				&asb, &adx, &ady, &bchar, &achar) < 5) {
				 fprintf(errout, "Bad composite %s", line);
			}
			else if ((bchar >= MAXADJUST) || (achar >= MAXADJUST)) {
				fprintf(errout, "Bad base (%d) or accent (%d) code %s",
						bchar, achar, line);
			}
/*			else if (charpos[bchar] >= 0 && charpos[achar] >= 0)  */
			else if ((bchar < MAXCHRS && charpos[bchar] < 0) ||
					 (bchar >= MAXCHRS && rawcharpos[bchar-MAXCHRS] < 0)) {
				if (passflag > 0)							/* 96/Apr/23 */
				fprintf(errout, "No data for base char (%d) %s", bchar, line);
			}
			else if ((achar < MAXCHRS && charpos[achar] < 0) ||
					 (achar >= MAXCHRS && rawcharpos[achar-MAXCHRS] < 0)) {
				if (passflag > 0)							/* 96/Apr/23 */
				fprintf(errout, "No data for accent char (%d) %s", achar, line);
			}
			else {
				if (passflag == 2) {					/* 1995/May/28 */
					if (bchar >= MAXCHRS) extendedseac++;
					if (achar >= MAXCHRS) extendedseac++;
				}
/*				if (ncomposites >= MAXCOMPOSITES)  */
				if (ncomposites >= maxcomposites) {
					realloccomposites(maxcomposites+maxcomposites);
/*					fprintf(errout, "Too many composite characters\n"); */
/*					continue; */
				}
/*				strcpy(composite[ncomposites], charname);  */
/*				if (passflag > 0) */	/* 1994/June/3 */
/*				if (passflag > 0 && compitflag != 0) {	*/
				if (compitflag != 0) {
					if (passflag == 1) {	/* do only once 95/May/4 */
						if (composite[ncomposites] != NULL) {
							fprintf(errout, "composite[%d] == %s ",
									ncomposites, composite[ncomposites]);
							free(composite[ncomposites]);	/* ? */
							composite[ncomposites] = NULL;
						}
						composite[ncomposites] = zstrdup(charname); 
					}
					base[ncomposites] = (short) bchar;
					accent[ncomposites] = (short) achar;
					pccx[ncomposites] = (short) kround(adx - asb + bsb);
					pccy[ncomposites] = (short) kround(ady);
					ncomposites++;
					if (ncomposites >= maxcomposites) {
						fprintf(errout, "Too many composites\n");
						exit(1);
					}
				}
				present = ftell(input);				/* remember where we are */
				if (bchar < MAXCHRS) fseek (input, charpos[bchar], SEEK_SET);
				else fseek (input, rawcharpos[bchar-MAXCHRS], SEEK_SET);
/*				if (traceflag) printf("DOWN to   base %s\n",
					standardencoding[bchar]); */
				if (depth++ > MAXDEPTH)
					printf("Possible recursion in call to %s\n",
						standardencoding[bchar]);
				getbbox(input, passflag, compitflag); /* scan base character */
				depth--;
/*				if (traceflag) printf("BACK from base %s\n",
					standardencoding[bchar]); */
/*				x = adx; y = ady; */			/* place accent */
				x = adx - asb + bsb; 
/*				x = adx; */						/* trial 95/June/27 */
				y = ady;
				if (achar < MAXCHRS) fseek (input, charpos[achar], SEEK_SET);
				else fseek (input, rawcharpos[achar-MAXCHRS], SEEK_SET);
/*				if (traceflag) printf("DOWN to   acce %s\n", 
						standardencoding[achar]); */
				if (depth++ > MAXDEPTH)
					printf("Possible recursion in call to %s\n",
						standardencoding[bchar]);
				getbbox(input, passflag, compitflag);	/* scan accent */
				depth--;
/*				if (traceflag) printf("BACK from acce  %s\n", 
						standardencoding[achar]); */
				fseek(input, present, SEEK_SET);
			} /* end of if charpos[achar] >= 0 && charpos[bchar] >= 0 */
/*			else if (passflag == 2)
				fprintf(errout, "Missing base or accent info %d %d in %s",
					bchar, achar, line); */
		} /* end of if strstr "seac" != NULL */
		if (strstr(line, "move") != NULL) {
			if (firstflag) {
				if (nonstandard) {	/* fix 1994/Oct/11 */
					xur = xll = (x * m11 + y * m12 + m13) * 1000.0;
					yur = yll = (x * m21 + y * m22 + m23) * 1000.0;
				}
				else {
					xll = x; yll = y; xur = x; yur = y;
				}
				firstflag = 0;
			}
		}
	}
	return 0;
}

/* still need to deal with seac ! */
/* do something about div command ? */
/* what about callothersubr ? */
/* what about setcurrentpoint ? */
/* what about <n> <m> callsubr ? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showencoding(void) {				/* debugging only */
	int k;
	for (k = 0; k < MAXCHRS; k++) {
/*		if (strcmp(encoding[k], "") != 0)  */
		if (encoding[k] != NULL) 
			printf("%d %s\n", k, encoding[k]);
	}
}

int getrealline(FILE *input, char *buff) { /* get non-comment, non-blank */
	int k;
	k = getline(input, buff);
	while ((*buff == '%' || *buff == '\n') && k > 0)
		k = getline(input, buff);
	return k;
}
/* grab next white space delimited token in line */ /* read line if needed */
/* assumes pump has been primed, i.e. line read and strtok called once */

char *grabnexttoken(FILE *input, char *line) {
	char *str=NULL, *s;

	for (;;) {
		while ((s = strtok(str, " \t\n\r")) == NULL) {
			for(;;) {					/* need to grab a new line then */
				if (getrealline(input, line) <= 0) return NULL;	/* EOF */
/*				ignore comments and blank lines - continue round the loop */
				if (*line != '%' && *line != '\n' && *line != '\r') break;
			}
			str = line;
		}
		if (*s != '%') break;		/* escape if not comment */
/*		following added to strip comments off ends of lines 1992/Sep/17 */
		for(;;) {					/* need to grab a new line then */
			if (getrealline(input, line) <= 0) return NULL;	/* EOF */
/*			ignore comments and blank lines - continue round the loop */
			if (*line != '%' && *line != '\n' && *line != '\r') break;
		}
		str = line;
	}
/*	printf("%s ", s); _getch();	 */		/* debugging */
	return s;
}

/* possible problem with repeat encoding ? */

void gobbleencoding (FILE *input) { /*	new tokenized version follows */
	int chr, c, n;
	int base=10;
	char *s, *t;

/*	fputs(line, stdout); */	/* DEBUGGING */

/*	may want to remove some debugging error message output later ... */
	s = strtok(line, " \t\n\r");	/* start the pipeline */
	for (;;) {					/*	exit if hit `readonly' or `def' ??? */
		if (strcmp(s, "dup") != 0) {
			if (strcmp(s, "readonly") == 0 ||
				strcmp(s, "def") == 0) break;	/* normal exit */
			fprintf(errout, " Expecting `dup', not: `%s' ", s);
			break;
		}
		if ((s = grabnexttoken(input, line)) == NULL) break;
/*		Cater to stupid Adobe Universal Greek font format */ /* 92/Sep/17 */
		if (strchr(s, '#') != NULL) {	/* stupid encoding vector format */
			(void) sscanf(s, "%d#%n", &base, &n);	/* try and get base */
			s = s + n; chr=0;
			for (;;) {			/* more general version 92/Sep/27 */
				c = *s++;
				if (c >= '0' && c <= '9') c = c - '0';
				else if (c >= 'A' && c <= 'Z') c = c - 'A' + 10;
				else if (c >= 'a' && c <= 'z') c = c - 'a' + 10;				
				else {
					s--; break;
				}
				chr = chr * base + c;
			}
/*			printf(" <%d> ", chr); */	/* debugging only */
/*			if (base == 8) {
				if (sscanf(s, "%d#%o", &base, &chr) < 2) {				
					fputs(line, errout); break;
				}
			}
			else if (base == 16) {
				if (sscanf(s, "%d#%x", &base, &chr) < 2) {				
					fputs(line, errout); break;
				}
			} */
		}						/* end of radixed number case */
		else if (sscanf(s, "%d", &chr) < 1) {
			fprintf(errout, " Expecting number, not: `%s' ", s);
			break;
		}
/*		deal with idiotic Fontographer format - no space before /charname */
		if ((t = strchr(s, '/')) != NULL) s = t;	/* 1992/Aug/21 */
		else if ((s = grabnexttoken(input, line)) == NULL) break;
		if (*s != '/') 	fprintf(errout, "Bad char code `%s' ", s);
		else s++;
/*		if (chr >= 0 && chr < MAXCHRS && strlen(s) < MAXCHARNAME) 
			strcpy(encoding[chr], s); */
/*		if (chr >= 0 && chr < MAXCHRS) encoding[chr] = _strdup(s); */
		if (chr >= 0 && chr < MAXCHRS) {
/*			if (strcmp(encoding[chr], "") != 0) { */
			if (encoding[chr] != NULL) {
				fprintf(errout, "encoding[%d] == %s ",
						chr, encoding[chr]);
				free(encoding[chr]);
				encoding[chr] = NULL;
			}
/*			flush control option 96/June/11 to suppress control range */
			if (flushcontrol == 0 || (chr >= 32 && (chr < 128 || chr >= 160)))
				encoding[chr] = zstrdup(s);
		}
		else fprintf(errout, "Invalid char number %d ", chr);
		if ((s = grabnexttoken(input, line)) == NULL) break;
		if (strcmp(s, "put") != 0) {
			fprintf(errout, " Expecting `put' not: `%s' ", s);
/*			break; */
		}
		if ((s = grabnexttoken(input, line)) == NULL) break;
	}
/*  normally come here because line does not contain `dup'  */
/*	but does contain `readonly' or `def' */
/*	attempt to deal with Fontographer 4.0.4 misfeature 94/Nov/9 */
/*	if `readonly' appears on one line and `def' appeard on the next */
	if (strcmp(s, "readonly") == 0) {	
		if ((s = grabnexttoken(input, line)) != NULL) {
			if (strcmp(s, "def") != 0)
				fprintf(errout, " Expecting `def', not: `%s' ", s);			
		}
	}
/*	need to clean out current line at all ? */
}

int getencoding (FILE *input) {				/*	New stuff 1993 July 1 */
	int k, flag;
	int ns, ne;
	
	standardflag = 0;
/*	printf("Setting standard == 0\n"); */	/* debugging */
/*	needsaccent = 0; */			/* in case we pop out */
/*	for (k = 0; k < MAXCHRS; k++) *encoding[k] = '\0';	 */
/*	for (k = 0; k < MAXCHRS; k++) encoding[k] = "";	 */
	for (k = 0; k < MAXCHRS; k++) encoding[k] = NULL;	
	flag = getrealline(input, line);
	while (strstr(line, "/Encoding") == NULL && flag > 0) {
		flag = getrealline(input, line);
	}
	if (flag <= 0) {		/* hit EOF before /Encoding */
		fprintf(errout, "WARNING: Failed to find encoding\n");
		return 0;
	}
	if (strstr(line, "StandardEncoding") != NULL) {
		for (k = 0; k < MAXCHRS; k++) 
/*			strcpy(encoding[k], standardencoding[k]);*/
			encoding[k] = zstrdup(standardencoding[k]);
/*		if (traceflag != 0) showencoding(); */
/*		printf("Setting standard == 1\n"); */	/* debugging */
		standardflag = 1;
		return 1;
	}
	if (strstr(line, "ISOLatin1Encoding") != NULL) {
		for (k = 0; k < MAXCHRS; k++)
			encoding[k] = zstrdup(isolatin1encoding[k]);
		isolatin1flag = 1;
		return 1;
	}
/*	get here if *not* StandardEncoding */
	k = getrealline(input, line);
	if (sscanf(line, "%d 1 %d {", &ns, &ne) < 2) {
/*		fprintf(errout, " No /.notdef line");  */
	}
	else k = getrealline(input, line);	
	gobbleencoding(input);
	if (traceflag != 0) {
		showencoding();
		fflush(stdout);
	}
/*	needsaccent = checkencoding(); */
	return 1;
}

char *trimend (char *text) { /* remove white space at end of line */
	char *s;
	s = text + strlen(text);
	while (s >= text && *s <= ' ') s--;
	*(s+1) = '\0';
	return text;
}

char *stripline (char *text) {	/* isolate line after space */
	char *s, *t;
	(void) trimend(text);			/* 96/June/16 */
	if ((s = strchr(text, ' ')) != NULL) {
		if ((t = strrchr (s+1, '\n')) != NULL) *t = '\0';
		return s+1;
	}
	else return text;
}

char *stripword (char *text) {	/* isolate word after space */
	char *s, *t;
	(void) trimend(text);			/* 96/June/16 */
	if ((s = strchr(text, ' ')) != NULL) {
		if ((t = strrchr (s+1, ' ')) != NULL) *t = '\0';
		else if ((t = strrchr (s+1, '\n')) != NULL) *t = '\0';
		return s+1;
	}
	else return text;
}

char *stripwordslash (char *text) {	/* isolate word after slash */
	char *s, *t;
	(void) trimend(text);			/* 96/June/16 */
	if ((s = strchr(text, '/')) != NULL) {
		if ((t = strrchr (s+1, ' ')) != NULL) *t = '\0';
		else if ((t = strrchr (s+1, '\n')) != NULL) *t = '\0';
		return s+1;
	}
	else return text;
}

char *stripstring(char *text) {	/* isolate delimiter delimited */
	char *s, *t;
	if ((s = strchr(text, '(')) != NULL) {
		if ((t = strrchr(text, ')')) != NULL) {
			if (t == s+2) return stripline (text);	/* 1993/Dec/16 */
			*t = '\0';
		}
		else if ((t = strrchr(text, '\n')) != NULL) *t = '\0';
		return s+1;
	}
	else if ((s = strchr(text, '[')) != NULL) {
		if ((t = strrchr(text, ']')) != NULL) *t = '\0';
		else if ((t = strrchr(text, '\n')) != NULL) *t = '\0';		
		return s+1;
	}
	else if ((s = strchr(text, '{')) != NULL) {
		if ((t = strrchr(text, '}')) != NULL) *t = '\0';
		else if ((t = strrchr(text, '\n')) != NULL) *t = '\0';
		return s+1;
	}
	else return text;
}

/* following split out 1994/May/27 -- maybe sort AFM entries future */

/* if encodeflag == 0, then only show unencoded characters */
/* used when sorting encoded characters */

void scancharstrings(FILE *fp_out, FILE *fp_in, int passflag, int encodeflag) {
	int k, n, chr;
	char *s;
	long present;
	int dotcount=0;

	depth = 0;
/*	scan all of the CharStrings */
	for (;;) {
		if ((n = getline(fp_in, line)) <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;
/*		if (extractflag == 0) fputs(line, fp_out); */
		if ((s = strchr(line, '/')) != NULL) {
			if (sscanf(s, "/%s ", charname) < 1) {
				fprintf(errout, "Bad charname %s", line);
				continue;							/* 94/May/12 */
			}
			if (debugflag) printf("\n%s ", charname);
			if (strcmp(charname, ".notdef") == 0) {
				notdefseen++;
				continue;		/* don't bother to analyze this */
			}
/*			ncharstrings++; */						/* 97/June/20 */
			present = ftell(fp_in);
			if ((k = lookupstandard(charname)) >= 0) {
				charpos[k] = present;			/* remember position */
			}				/* else not in standard encoding */
			else {
/*				if (traceflag) printf("%s not standard\n", charname); */
				if (debugflag) printf("%s not standard\n", charname);
			}
			if ((k = lookup(charname)) >= 0) {		/* 1994 May 27 */
				rawcharpos[k] = present;		/* remember position */
			}				/* else not in current encoding */
			else {
/*				if (traceflag) printf("%s not encoded\n", charname); */
				if (debugflag) printf("%s not encoded\n", charname);
			}
			if (sortunencflag && passflag == 0) {
				if ((k = lookupglyph(charname)) >= 0) {
					fprintf(errout, "WARNING: %s repeated in %s\n",
							charname, fn_in);
				}
				else if (glyphindex < maxcharstrings) {
					charstringpos[glyphindex] = present;
					if (glyphs[glyphindex] != NULL)
						printf("ERROR: %s %d ", glyphs[glyphindex], glyphindex);
					glyphs[glyphindex] = zstrdup(charname);
					glyphindex++;
				}
				else fprintf(errout, "Overrun ");
			}

			if (dotsflag && ! individualbboxes) {
				putc('.', stdout);
				dotcount++;
				if (dotcount % 100 == 0) fflush(stdout);
			}
/*			xll = 0; yll = 0; xur = 0; yur = 0; */
			xll = TOOLARGE; yll = TOOLARGE;	xur = -TOOLARGE; yur = -TOOLARGE;
			firstflag = 1;
			getbbox(fp_in, passflag, 1);	/* pick up composites */
			chr = lookup(charname);
			if (nonstandard) {				/* 1994/Feb/4 */
				width = width * m11 * 1000.0;
			}
			if (passflag == 1) {			/* 1994/Feb/4 */
				if (xll < fontxll) fontxll = xll;
				if (xur > fontxur) fontxur = xur;
				if (yll < fontyll) fontyll = yll;
				if (yur > fontyur) fontyur = yur;
				if (strlen(charname) == 1) {
					if (*charname == 'x') xheight = yur;
					else if (*charname == 'H') capheight = yur;
					else if (*charname == 'p') descender = yll;
					else if (*charname == 'd') ascender = yur;
/*		following only needed in case font does not have x, H, p, d ... */
					if (xheight == 0) 
						if (*charname >= 'u' && *charname <= 'z') xheight = yur;
					if (capheight == 0) {
						if (*charname >= 'U' && *charname <= 'Z') capheight = yur;
						if (*charname >= 'H' && *charname <= 'N') capheight = yur;
					}
					if (descender == 0) {
						if (*charname >= 'p' && *charname <= 'q') descender = yll;
						if (*charname == 'g' || *charname == 'j') descender = yll;
					}
					if (ascender == 0) {
						if (*charname == 'b' || *charname == 'h') ascender = yur;
						if (*charname == 'k' || *charname == 'l') ascender = yur;
					}
				}
			}
			if (passflag == 2) {
				if (xll == TOOLARGE && yll == TOOLARGE &&
					xur == -TOOLARGE && yur == -TOOLARGE) {
					xll = 0; yll = 0; xur = 0; yur = 0;
				}
				if (wxroundflag) width = kround(width);		/* 1995/Jan/5 */
				if (encodeflag != 0 || chr < 0)		/* 1994/May/27 */
/*					fprintf(fp_out, "C %d ; WX %lg ; N %s ; B %d %d %d %d ; \n",
						chr, width, charname, 
		kround(xll), kround(yll), kround(xur), kround(yur)); */
					fprintf(fp_out, "C %d ; WX %lg ; N %s ; B %d %d %d %d ;",
							chr, width, charname, 
							kround(xll), kround(yll), kround(xur), kround(yur));
				if (addligatures) doligatures(fp_out, charname);
				fprintf(fp_out, "\n");
			}
			charcount++;
			if (chr < 0) unencoded++;
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */		
		if (depth != 0)	fprintf(errout, "Non-zero level at end of %s\n",
				charname);
	} /* end of for (;;) */
}

void sortcharstrings() {	/* do this via stupid bubble sort! */
	int i, j;
	long temp;
	char *gtemp;
	if (traceflag) {
		printf("SORTING %d\n", glyphindex);
		for (i = 0; i < glyphindex; i++) printf("%d\t%s\n", i, glyphs[i]);
		fflush(stdout);
	}
	for (i=0; i < glyphindex-1; i++) {
		for (j=0; j < glyphindex-1-i; j++) {
			if (strcmp(glyphs[j], glyphs[j+1]) > 0) {
				temp = charstringpos[j];
				charstringpos[j] = charstringpos[j+1];
				charstringpos[j+1] = temp;
				gtemp = glyphs[j];
				glyphs[j] = glyphs[j+1];
				glyphs[j+1] = gtemp;
			}
		}
	}
	if (traceflag) {
		printf("SORTING %d\n", glyphindex);
		for (i = 0; i < glyphindex; i++) printf("%d\t%s\n", i, glyphs[i]);
		fflush(stdout);
	}
}

/* not integrated yet */ /* only when pass == 2 */
/* does encoded chars sequentially - 1994/May/27 */
/* encodeflag presently not referenced ... */

void dosortencchars (FILE *fp_out, FILE *fp_in, int passflag) {
	int chr;
	long present;
	int dotcount=0;

	depth = 0;
	present = ftell(fp_in);
/*	scan all of the CharStrings sequentially as per encoding vector */
	for (chr = 0; chr < MAXCHRS; chr++) {
		if (rawcharpos[chr] < 0) continue;			/* no character here */
		fseek(fp_in, rawcharpos[chr], SEEK_SET);	/* go to char */
		if (dotsflag && ! individualbboxes) {
			putc('.', stdout);
			dotcount++;
			if (dotcount % 100 == 0) fflush(stdout);
		}
/*		xll = 0; yll = 0; xur = 0; yur = 0; */
		xll = TOOLARGE; yll = TOOLARGE;	xur = -TOOLARGE; yur = -TOOLARGE;
		firstflag = 1;
		if (encoding[chr] == NULL) 
			strcpy(charname, encoding[chr]);		/* 1994/June/4 */
		else *charname = '\0';
		if (debugflag) printf("\n%s ", charname);
		getbbox(fp_in, passflag, 0); /* only come here on pass 2 */
		if (nonstandard) {				/* 1994/Feb/4 */
			width = width * m11 * 1000.0;
		}
		if (passflag == 1) {			
			if (xll < fontxll) fontxll = xll;
			if (xur > fontxur) fontxur = xur;
			if (yll < fontyll) fontyll = yll;
			if (yur > fontyur) fontyur = yur;
		}	/* impossible */
		if (chr >= 0) {		/* 1994/May/27 */
			if (xll == TOOLARGE && yll == TOOLARGE &&
				xur == -TOOLARGE && yur == -TOOLARGE) {
				xll = 0; yll = 0; xur = 0; yur = 0;
			}
			if (wxroundflag) width = kround(width);			/* 1995/Jan/5 */
/*			fprintf(fp_out, "C %d ; WX %lg ; N %s ; B %d %d %d %d ; \n",
				chr, width, encoding[chr],
					kround(xll), kround(yll), kround(xur), kround(yur)); */
			fprintf(fp_out, "C %d ; WX %lg ; N %s ; B %d %d %d %d ;",
					chr, width, encoding[chr],
					kround(xll), kround(yll), kround(xur), kround(yur));
			if (addligatures) doligatures(fp_out, charname);
			fprintf(fp_out, "\n");
		}
/*		charcount++; */ /* already counted above ??? */
/*		if (chr < 0) unencoded++; ??? */
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */		
		if (depth != 0)	fprintf(errout, "Non-zero level at end of %s\n",
				encoding[chr]);
	}
	fseek(fp_in, present, SEEK_SET);			/* reset file position */
}

/* new 1997/June/20 */

void dosortunencchars (FILE *fp_out, FILE *fp_in, int passflag) {
	int k, chr;
	long present;
	int dotcount=0;

	depth = 0;
	present = ftell(fp_in);
/*	scan all of the CharStrings sequentially as per alphabetic sorting */
	for (k = 0; k < glyphindex; k++) {
		if (charstringpos[k] < 0) continue;			/* can't happen */
		strcpy(charname, glyphs[k]);			/* 1994/June/4 */
		chr = lookup(charname);
		if (chr >= 0 && sortencflag) continue;	/* ignore encoded */
		fseek(fp_in, charstringpos[k], SEEK_SET);	/* go to char */
		if (dotsflag && ! individualbboxes) {
			putc('.', stdout);
			dotcount++;
			if (dotcount % 100 == 0) fflush(stdout);
		}
/*		xll = 0; yll = 0; xur = 0; yur = 0; */
		xll = TOOLARGE; yll = TOOLARGE;	xur = -TOOLARGE; yur = -TOOLARGE;
		firstflag = 1;
/*		strcpy(charname, glyphs[k]); */		/* 1994/June/4 */
		if (debugflag) printf("\n%s ", charname);
		getbbox(fp_in, passflag, 0); /* only come here on pass 2 */
		if (nonstandard) {				/* 1994/Feb/4 */
			width = width * m11 * 1000.0;
		}
		if (passflag == 1) {			
			if (xll < fontxll) fontxll = xll;
			if (xur > fontxur) fontxur = xur;
			if (yll < fontyll) fontyll = yll;
			if (yur > fontyur) fontyur = yur;
		}	/* impossible */
		if (k >= 0) {		/* 1994/May/27 */
			if (xll == TOOLARGE && yll == TOOLARGE &&
				xur == -TOOLARGE && yur == -TOOLARGE) {
				xll = 0; yll = 0; xur = 0; yur = 0;
			}
			if (wxroundflag) width = kround(width);			/* 1995/Jan/5 */
/*			fprintf(fp_out, "C %d ; WX %lg ; N %s ; B %d %d %d %d ; \n",
				chr, width, glyphs[k],
					kround(xll), kround(yll), kround(xur), kround(yur)); */
			fprintf(fp_out, "C %d ; WX %lg ; N %s ; B %d %d %d %d ;",
					chr, width, glyphs[k],
					kround(xll), kround(yll), kround(xur), kround(yur));
			if (addligatures) doligatures(fp_out, charname);
			fprintf(fp_out, "\n");
		}
/*		charcount++; */ /* already counted above ??? */
/*		if (chr < 0) unencoded++; */ /* ??? */
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */		
		if (depth != 0)	fprintf(errout, "Non-zero level at end of %s\n",
				glyphs[k]);
	}
	fseek(fp_in, present, SEEK_SET);			/* reset file position */
}

void dosortchars(FILE *fp_out, FILE *fp_in, int passflag, int encodeflag) {
	if (encodeflag) dosortencchars(fp_out, fp_in, passflag);
	else dosortunencchars(fp_out, fp_in, passflag); 
//	if (verboseflag) printf("End of DoChars\n");	// debugging
	fflush(stdout);
}

int sortcomposites (int n) {
	int i, j;
	short temp;
	int swaps=0;
	char *stemp;
	for (i = 0; i < n-1; i++) {
		for (j = i+1; j < n; j++) {
			if (composite[i] == NULL) continue;	/* ??? */
			if (composite[j] == NULL) continue;	/* ??? */
			if (strcmp(composite[i], composite[j]) > 0) {
				swaps++;
				stemp = composite[i];
				composite[i] = composite[j];
				composite[j] = stemp;
				temp = base[i];
				base[i] = base[j];
				base[j] = temp;
				temp = accent[i];
				accent[i] = accent[j];
				accent[j] = temp;
				temp = pccx[i];
				pccx[i] = pccx[j];
				pccx[j] = temp;
				temp = pccy[i];
				pccy[i] = pccy[j];
				pccy[j] = temp;
			}
		}
	}
	if (traceflag) {
		printf("Sorted composites using %d swaps\n", swaps);
		fflush(stdout);
	}
	return swaps;
}

void copyoldheader (FILE *output, FILE *input) {
	if (output == NULL || input == NULL) {
		printf("Bad file handles\n");
		return;
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "StartCharMetrics", 16) == 0) {
			break;
		}
		if (strncmp(line, "Comment partial AFM file", 24) == 0) continue;
		if (strncmp(line, "Comment Character Bounding", 26) == 0) continue;
		if (strncmp(line, "FontBBox", 8) == 0) { /* replace BBox */
			sprintf(line, "FontBBox %d %d %d %d\n",
		kround(fontxll), kround(fontyll), kround(fontxur), kround(fontyur));
/*			fontxll = 0; fontyll = 0; fontxur = 0; fontyur = 0; */
		}
		if (strncmp(line, "EncodingScheme MS Windows ANSI", 30) == 0)
			strcpy(line, "EncodingScheme AdobeStandardEncoding\n");
/*	Omit fields that will be given later ??? */
		if (strncmp(line, "CapHeight", 9) == 0)	{
			if (capheight > 0) {
				if (keepoldorder) {
					sprintf(line, "CapHeight %d\n",	capheight);
					capheight = 0;
				}
				else continue;
			}
		}
		if (strncmp(line, "XHeight", 7) == 0) {
			if (xheight > 0) {
				if (keepoldorder) {
					sprintf(line, "XHeight %d\n", xheight);
					xheight = 0;
				}
				else continue;
			}
		}
		if (strncmp(line, "Ascender", 8) == 0) {
			if (ascender > 0) {
				if (keepoldorder) {
					sprintf(line, "Ascender %d\n", ascender);
					ascender = 0;
				}
				else continue;
			}
		}
		if (strncmp(line, "Descender", 9) == 0) {
			if (descender < 0) {
				if (keepoldorder) {
					sprintf(line, "Descender %d\n",	descender);
					descender = 0;
				}
				else continue;
			}
		}

/*	maybe check and correct more information here ? */
		fputs(line, output);
	}
}

void copyoldkerndata (FILE *output, FILE *input) {
	char *s;
	int nkern=0;
	if (verboseflag)
		printf("\nTrying to copy kern data from %s\n", fn_afm);		// debugging only
	while ((s = fgets(line, MAXLINE, input)) != NULL &&
		   strncmp(line, "EndCharMetrics", 14) != 0) ;
	if (s == NULL) {
		return;
		printf("\nFound no %s in %s\n", "EndCharMetrics", fn_afm);
	}
//	removed following so stupid AFM files without it work!
//	while ((s = fgets(line, MAXLINE, input)) != NULL &&
//		   strncmp(line, "StartKernData", 13) != 0) ;
//	if (s == NULL) {
//		printf("\nFound no %s in %s\n", "StartKernData", fn_afm);
//		return;
//	}
	while ((s = fgets(line, MAXLINE, input)) != NULL &&
		   strncmp(line, "StartKernPairs", 14) != 0) ;
	if (s == NULL) {
		printf("\nFound no %s in %s\n", "StartKernPairs", fn_afm);
		return;
	}
	if (sscanf(line+15, "%d", &nkern) > 0) {
		if (nkern > 0) {
			if (verboseflag) printf("\n%d kern pairs from old AFM file\n", nkern);
		}
	}
	fputs(line, output);
	while ((s = fgets(line, MAXLINE, input)) != NULL &&
		   strncmp(line, "EndKernData", 11) != 0) 
		fputs(line, output);
	if (s == NULL) {
		printf("\nFound no %s in %s\n", "EndKernData", fn_afm);
		fputs("EndKernData\n", output);
	}
	else fputs(line, output);
}

void underscore(char *filename); /* convert font file name to Adobe style */

/* passflag == 0 first time info gathering char position  --- no output */
/* passflag == 1 second time gathering bbox  --- no output */
/* passflag == 2 third time emitting AFM file -- 94/May/12 */

// fp_out is to AFM file to write
// fp_pln is to PLN file to read

int extractafm (FILE *fp_out, FILE *fp_in, int passflag) {
//	char oldafmfile[FILENAME_MAX]; 
//	char oldbakfile[FILENAME_MAX];			/* 1995/July/15 */
	FILE *fp_afm=NULL;
	int k, n;
	char *s;
	int subr, nlen;
	long present;
	char *bcharname;
	char *acharname;
	
/*	Insert new AFM header on third pass if old AFM not available */
/*	Insert old AFM header on third pass if old AFM is  available */

	if (passflag == 0) {
		for (k = 0; k < MAXCHRS; k++) charpos[k] = -1;
		for (k = 0; k < MAXCHRS; k++) rawcharpos[k] = -1;
		for (k = 0; k < MAXCHRS; k++) encoding[k] = NULL;
		present = ftell(fp_in);
		getencoding (fp_in);
		fseek(fp_in, present, SEEK_SET);
		fontxll = TOOLARGE; fontyll = TOOLARGE;
		fontxur = -TOOLARGE; fontyur = -TOOLARGE;
	}

/*	ncomposites=0; */	
	if (passflag != 2 || sortunencflag == 0) {	/* ??? */
		ncomposites=0;
	}

//	need to fix this

	if (passflag == 2 && useoldafm != 0) {
//		strcpy (oldafmfile, afmfile);
//		extension (oldafmfile, "afm");
//		if ((fp_afm = fopen(oldafmfile, "r")) != NULL) {
//			printf("%s exists\n", oldafmfile);
//			fclose(fp_afm);
//			if (strcmp(fn_out, oldafmfile) == 0) {	/* 1995/July/18 */
//				strcpy(oldbakfile, oldafmfile);
//				forceexten(oldafmfile, "old");
//				printf("Renaming %s to %s\n", oldbakfile, oldafmfile);
//				if (remove(oldafmfile)) {
//					printf("Failed to remove %s\n", oldafmfile);
//				}
//				if (rename(oldbakfile, oldafmfile)) {
//					printf("Failed to rename %s to %s\n", oldbakfile, oldafmfile);
//				}
//			}
		strcpy(fn_afm, afmfile);
		extension(fn_afm, "afm");
		if ((fp_afm = fopen(fn_afm, "r")) == NULL) {
			underscore(fn_afm);				// add underscores
			if ((fp_afm = fopen(fn_afm, "r")) == NULL) {
				perrormod(fn_afm);
				useoldafm = 0;				/* failed to open */
			}
		}
		else {
			printf("\nUsing %s for header and kerning info\n", fn_afm);
			copyoldheader(fp_out, fp_afm);
		}
	}


	if (passflag == 2 && useoldafm == 0) 
		fprintf(fp_out, "StartFontMetrics 2.0\n");

/*  scan up to eexec encrypted part */
/*  This is the place to extract other AFM information ??? */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
/*		do only in pass 2 and then only if no old AFM file */
		if (passflag == 2 && useoldafm == 0) {
			if (*line == '%' && strstr(line, "trademark") != NULL) {
				fprintf(fp_out, "Comment %s", line+2);	/* 1996/May/12 */
			}
		if ((s = strstr(line, "/version")) != NULL) {
			fprintf(fp_out, "Version %s\n", stripstring(s));
		}
/*		else if ((s = strstr(line, "Notice")) != NULL) { */
		else if ((s = strstr(line, "/Notice")) != NULL) {
			fprintf(fp_out, "Notice %s\n", stripstring(s));
		}
		else if ((s = strstr(line, "/FullName")) != NULL) {
			fprintf(fp_out, "FullName %s\n", stripstring(s));
		}
		else if ((s = strstr(line, "/FamilyName")) != NULL) {
			fprintf(fp_out, "FamilyName %s\n", stripstring(s));
		}
		else if ((s = strstr(line, "/Weight")) != NULL) {
			fprintf(fp_out, "Weight %s\n", stripstring(s));
		}
		else if ((s = strstr(line, "/ItalicAngle")) != NULL) {
			fprintf(fp_out, "ItalicAngle %s\n", stripword(s));
		}
		else if ((s = strstr(line, "/isFixedPitch")) != NULL) {
			fprintf(fp_out, "IsFixedPitch %s\n", stripword(s));
			if (strstr(line, "true") != NULL) isfixedpitch = 1;
			else if (strstr(line, "false") != NULL) isfixedpitch = 0;
		}
		else if ((s = strstr(line, "/UnderlinePosition")) != NULL) {
			fprintf(fp_out, "UnderlinePosition %s\n", stripword(s));
		}
		else if ((s = strstr(line, "/UnderlineThickness")) != NULL) {
			fprintf(fp_out, "UnderlineThickness %s\n", stripword(s));
		}
		else if ((s = strstr(line, "/FontName")) != NULL) {
/*			fprintf(fp_out, "FontName %s\n", stripword(s+1); */
			fprintf(fp_out, "FontName %s\n", stripwordslash(s+1)); 
		}
		else if ((s = strstr(line, "FontBBox")) != NULL) {
/*			fprintf(fp_out, "FontBBox %s\n", stripstring(s)); */
			fprintf(fp_out, "FontBBox %d %d %d %d\n",
		kround(fontxll), kround(fontyll), kround(fontxur), kround(fontyur));
		}
/*		else if ((s = strstr(line, "FontMatrix")) != NULL) {
			while (*s != '[' && *s != '\0') s++; 
			if (sscanf (s, "[%lg %lg %lg %lg %lg %lg]",
				&m11, &m21, &m12, &m22, &m13, &m23) < 6) {
				fprintf(errout, "Don't understand: %s", line);
			}
			else {
				if (m11 != 0.001 || m21 != 0.0 ||
						m12 != 0.0 || m22 != 0.001 ||
							m13 != 0.0 || m23 != 0.0) {
					nonstandard = 1;
					if (verboseflag) fputs(line, stdout);
				}
				else nonstandard = 0;	
			}
		} */
		else if ((s = strstr(line, "/Copyright")) != NULL) {
/* do only if it appears to be a one liner ... */
			if (strrchr(line, ')') != NULL)
				fprintf(fp_out, "Comment Copyright %s\n", stripstring(s));
		}
		else if ((s = strstr(line, "Comment Copyright")) != NULL) {
/* do only if it appears to be a one liner ... */
/*			if (strrchr(line, ')') != NULL) */
/*				fprintf(fp_out, "Comment Copyright %s\n", stripstring(s));*/
				fputs(s, fp_out);					/* 95/June/27 */
		}
		else if ((s = strstr(line, "UniqueID")) != NULL) {
			if (strstr(line, "FontType") == NULL &&
				strstr(line, "FontDirect") == NULL)	/* 95/June/27 */
				fprintf(fp_out, "Comment UniqueID %s\n", stripword(s));
		}
		else if ((s = strstr(line, "CreationDate")) != NULL) {
			fprintf(fp_out, "Comment CreationDate: %s\n", stripline(s));
		}
		else if ((s = strstr(line, "RevisionDate")) != NULL) {
			fprintf(fp_out, "Comment RevisionDate: %s\n", stripline(s));
		}
		else if ((s = strstr(line, "VMusage")) != NULL) {
			fprintf(fp_out, "Comment VMusage %s\n", stripline(s));
		}
/*		if (extractflag == 0) fputs(line, fp_out); */
		} /* if passflag == 2 && useoldafm == 0 */
		if ((s = strstr(line, "FontMatrix")) != NULL) {
			while (*s != '[' && *s != '\0') s++; 
			if (sscanf (s, "[%lg %lg %lg %lg %lg %lg]",
				&m11, &m21, &m12, &m22, &m13, &m23) < 6) {
				fprintf(errout, "Don't understand: %s", line);
			}
			else {
				if (m11 != 0.001 || m21 != 0.0 ||
						m12 != 0.0 || m22 != 0.001 ||
							m13 != 0.0 || m23 != 0.0) {
					nonstandard = 1;
					if (verboseflag) fputs(line, stdout);
				}
				else nonstandard = 0;	
			}
		}
	}
/*	CapHeight 662 */
/*	XHeight 450 */
/*	Ascender 683 */
/*	Descender -217 */
	if (forceinteger) {
		if (capheight > 0) capheight = kround(capheight);
		if (xheight > 0) xheight = kround(xheight);
		if (ascender > 0) ascender = kround(ascender);
		if (descender < 0) descender = kround(descender);
	}   
	if (capheight > 0) fprintf(fp_out, "CapHeight %lg\n", capheight);
	if (xheight > 0) fprintf(fp_out, "XHeight %lg\n", xheight);
	if (ascender > 0) fprintf(fp_out, "Ascender %lg\n", ascender);
	if (descender < 0) fprintf(fp_out, "Descender %lg\n", descender);

/*	now reached eexec */
	if (traceflag) {
		printf("Reached eexec\n");
		fflush(stdout);
#ifdef DEBUGGING
		_getch();
#endif
	}	

/*	printf("Standard == %d\n", standard);	*//* debugging */
	if (passflag == 2 && useoldafm == 0) {
		if (standardflag)
			fprintf(fp_out, "EncodingScheme AdobeStandardEncoding\n");
		else 
			fprintf(fp_out, "EncodingScheme FontSpecific\n");
	}

/*	if (extractflag == 0) fputs(line, fp_out); */
/*	scan up to Subrs */
/*  This is the place to extract more interesting stuff ??? */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "/Subrs") == NULL &&
			strstr(line, "/CharStrings") == NULL) {
	}
/*	scan up to CharStrings */ /* remember Subr Positions */
/*	if (strstr(line, "/Subrs") != NULL) { */
	if ((s = strstr(line, "/Subrs")) != NULL) {
		nsubrs = 0;
/*		sscanf(line, "Subrs %d array", &nsubrs); */
		if (traceflag) {
			printf(line);
			fflush(stdout);
#ifdef DEBUGGING
			_getch();
#endif
		}
/*		/Subrs 128 array --- typical */
		if (sscanf(s, "/Subrs %d array", &nsubrs) == 0)
			fprintf(errout, "Unable to parse: %s", line);			

		if (nsubrs != maxsubrs)
			allocsubrs(nsubrs);

		while ((n = getline(fp_in, line)) != 0 && 
			strstr(line, "/CharStrings") == NULL) {
			if (strncmp(line, "dup", 3) == 0) {
				if (sscanf (line, "dup %d %d", &subr, &nlen) < 2) {
				}
/*				else if (subr >= 0 && subr < MAXSUBRS) { */
				else if (subr >= 0 && subr < maxsubrs) {
					subrpos[subr] = ftell(fp_in);
				}
			}
/*			if (extractflag == 0) fputs(line, fp_out); */
		}
/*	if (extractflag == 0) fputs(line, fp_out); */
	}
/*	if (passflag > 0) ncharstrings=0; */
	if ((s = strstr(line, "/CharStrings")) != NULL) {
		ncharstrings = 0;
		if (passflag == 0) glyphindex = 0;
/*		2 index /CharStrings 294 dict dup begin --- typical */
		if (traceflag) {
			printf(line);
			fflush(stdout);
#ifdef DEBUGGING
			_getch();
#endif
		}
		if (sscanf (s, "/CharStrings %d dict", &ncharstrings) == 0)
			fprintf(errout, "Unable to parse: %s", line);
		if (passflag == 0) alloccharstrings(ncharstrings);
		if (passflag == 0) allocglyphs(ncharstrings);
	}
	if (passflag == 0) {
		if (NCOMPOSITES >= maxcomposites)
			alloccomposites(NCOMPOSITES);
	}

/*	we assume the count is accurate and includes /.notdef ... */
	if (passflag == 2) 
/*		fprintf(fp_out, "StartCharMetrics %d\n", ncharstrings-1); */
/*		fprintf(fp_out, "StartCharMetrics %d\n", ncharstrings); */
		fprintf(fp_out, "StartCharMetrics %d\n", charcount);

/*	if (passflag > 0) ncharstrings=0; */	/* 97/June/20 ??? */
/*	notdefseen=0; */
/*	charcount=0; */
/*	unencoded=0;  */

	if (passflag != 2 || sortunencflag == 0) { /* ??? */
		charcount=0;
		unencoded=0; 
	} 

	capheight = 0; xheight = 0; ascender = 0; descender = 0;

	if (passflag < 2) {		/* split out 1994/May/27 */
		scancharstrings(fp_out, fp_in, passflag, 1);
		return 0;				/* ??? */
	}
/*	passflag == 2 */
	if (sortencflag == 0) {		/* u */
		if (sortunencflag == 0)			/* u  y */
			scancharstrings(fp_out, fp_in, passflag, 1);
		else  {							/* u - */
			sortcharstrings();
			dosortchars(fp_out, fp_in, passflag, 0);
		}
	}
	else {	/* sortencflag != 0 */				/* - */
		dosortchars(fp_out, fp_in, passflag, 1);	/* do encoded */
		if (sortunencflag == 0) 				/* - y */
			scancharstrings(fp_out, fp_in, passflag, 0);	/* do unencoded */
		else {									/* - - */
			sortcharstrings();
			dosortchars(fp_out, fp_in, passflag, 0);	/* do unencoded */
		}
	}

/*	if (passflag != 2) return 0; */
	fprintf(fp_out, "EndCharMetrics\n");

/*	if (useoldafm) { */
	if (passflag == 2 && useoldafm) {			/* 98/Dec/25 */
		if (fp_afm == NULL) fprintf(errout, "fp_afm == NULL\n");
		copyoldkerndata(fp_out, fp_afm);
		fclose (fp_afm);
	}

/*	if (extractflag == 0) {		
		if (n != 0) fputs(line, fp_out);
		while ((n = getline(fp_in, line)) != 0)	fputs(line, fp_out);
	} */
	if (dotsflag != 0) {
		putc('\n', stdout);
		fflush(stdout);
	}
	if (charcount != 0) {
		printf("Processed %d characters in %s ",
			charcount, fn_in);				/* ??? */
		if (unencoded != 0) {
			if (unencoded == 1)	printf(" (one was unencoded)", unencoded);
			else printf(" (%d were unencoded)", unencoded);
		}
		putc('\n', stdout);
	}
	if (ncomposites > 0) {
		if (sortcompositeflag) sortcomposites(ncomposites);
		fprintf(fp_out, "StartComposites %d\n", ncomposites);
		for (k = 0; k < ncomposites; k++) {
			if (nonstandard != 0) {				/* 1994/Feb/4 ??? */
				pccx[k] = (short) iround(pccx[k] * m11);
				pccy[k] = (short) iround(pccy[k] * m22);
			}
			if (base[k] < MAXCHRS)				/* 95/May/28 */
				bcharname = standardencoding[base[k]];
			else bcharname = encoding[base[k]-MAXCHRS];
			if (accent[k] < MAXCHRS)			/* 95/May/28 */
				acharname = standardencoding[accent[k]];
			else acharname = encoding[accent[k]-MAXCHRS];
			if (strcmp(composite[k], ".notdef") != 0) {/* kludge ... */
/*				fprintf(fp_out, "CC %s 2 ; PCC %s 0 0 ; PCC %s %d %d ; \n", */
				fprintf(fp_out, "CC %s 2 ; PCC %s 0 0 ; PCC %s %d %d ;\n",
						composite[k], bcharname, acharname, pccx[k], pccy[k]);
			}
		}
		fprintf(fp_out, "EndComposites\n");		
	} 
	fprintf(fp_out, "EndFontMetrics\n");
	if (notdefseen == 0)
		fprintf(errout, "WARNING: Font is lacking \\.notdef character\n");
	return 0;
}

char *gettoken(FILE *fp_in) {
	char *t;
	t = strtok (NULL, " \t\n\r\f[]");
	if (t == NULL) {
		if (getline(fp_in, line) == 0) return NULL;	/* EOF */
		t = strtok (line, " \t\n\r\f[]");
	}
	return t;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	if (quietflag == 0) {
/*		printf("%s <pfa-file>\n", s); */
		printf("%s {-v}{-u}{-y}{-c}{-n}{-f}{-w} {-a=<afm-file>} <pfa-file> ...\n", s);
		printf("\t-v\tverbose mode\n");
		printf("\t-u\tdo not sort encoded glyphs numerically\n");
		printf("\t-y\tdo not sort unencoded glyphs alphabetically\n");
		printf("\t-c\tdo not sort composite glyphs alphabetically\n");
		printf("\t-n\tuse `newline' as line termination\n");
		printf("\t-f\tallow use of low Subrs for glyph programs\n");
		printf("\t-w\tround character widths to integers if needed\n");
		printf("\t-a\tuse given AFM file as template (extract kern pair info)\n");
		printf("\t-z\tsuppress encoding in control character range\n");
	}
	exit(9);
}

void makename(char *fname, char *str) {	/* make unique temporary filename */
    char *s;
    s = strrchr(fname,'\\');			/* search back to directory part */
    if (s == NULL) {
		s = strrchr(fname,':');			/* search back to drive part */
		if (s == NULL) s = fname;
		else s++;
	}
	else s++;
	strcpy(s+2, "XXXXXX");		/* stick in template six characters */
/*    (void) mktemp(fname); */		/* replace template by unique string */
    (void) _mktemp(fname);		/* replace template by unique string */
	forceexten(fname, str);		/* force appropriate extension */
}

FILE *fp_dec=NULL, *fp_pln=NULL, *fp_adj=NULL;

char fn_dec[FILENAME_MAX], fn_pln[FILENAME_MAX], fn_adj[FILENAME_MAX];

void wipefile (char *name) {
	FILE *fp;
	long length, n;
	int k;
	
	if ((fp = fopen(name, "r+b")) == NULL) return;
	fseek (fp, 0, SEEK_END);
	length = ftell(fp);
	fseek (fp, 0, SEEK_SET);
	for (n = 0; n < length; n++) {
		for(;;) {
			k = rand() >> 8;
			if (k == '\n' || k == '\t' || k == '\r') break;
			if (k < ' ') k = k + 'A';
			if (k >= 128) k = k - 128;
			if (k>= ' ' && k < 128) break;
		}
		putc (k, fp);
	}
	fclose (fp);
}

void cleanup(void) {
	if (wipeclean == 0) return;
	if (fp_dec != NULL) {
		fclose(fp_dec);						/* close output */
/*		fp_dec = fopen(fn_dec, "wb");		
		if (fp_dec != NULL) fclose(fp_dec); */
		wipefile (fn_dec);
		if (wipeclean > 0) (void) remove(fn_dec);	/* remove bogus file */
	}
	if (fp_pln != NULL) {
		fclose(fp_pln);				/* close output */
/*		fp_pln = fopen(fn_pln, "wb");	
		if (fp_pln != NULL) fclose(fp_pln); */
		wipefile (fn_pln);
		if (wipeclean > 0) (void) remove(fn_pln); /* remove bogus file */
	}
	if (fp_adj != NULL) {
		fclose(fp_adj);				/* close output */
/*		fp_adj = fopen(fn_adj, "wb");	
		if (fp_adj != NULL) fclose(fp_adj); */
		wipefile (fn_adj);
		if (wipeclean > 0) (void) remove(fn_adj); /* remove bogus file */
	}
}

void abortjob() {
	fprintf(stderr, "\nUser Interrupt - Exiting\n"); 
	cleanup();
//	if (renameflag) rename(fn_in, fn_bak);
	exit(3);
}

/*	problem:        wants void (__cdecl *)(int); */
/*	gets #define SIG_IGN (void (__cdecl *)(int))1 */

#undef SIG_IGN
#define SIG_IGN (void (__cdecl *)(int))1L

void ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
/*	fprintf(stderr, "\nUser Interrupt - Exiting\n");  */
/*	cleanup(); */
	if (bAbort++ >= 15) exit(3);			/* emergency exit */
	(void) signal(SIGINT, ctrlbreak); 
}

int decodeflag (int c) {
	switch(c) { 
/*		case 'v': verboseflag = 1; dotsflag = 0; return 0; */
		case 'v': verboseflag = 1; dotsflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case 'u': sortencflag = 0; return 0;
		case 'y': sortunencflag = 0; return 0;			/* 97/June/20 */
		case 'c': sortcompositeflag = 0; return 0;		/* 97/July/8 */
		case 'n': unixflag = 1; return 0;
		case '?': detailflag++; return 0;
		case 'd': wipeclean++; return 0;
		case 'i': forceinteger = 0; return 0;
		case 'f': ignorelow = 0; return 0;			/* 94/Dec/24 */
		case 's': snaptoabs = 0; return 0;
		case 'w': wxroundflag = 1; return 0;		/* 95/Jan/5 */
		case 'x': approxflag = 1; return 0;
		case 'z': flushcontrol = 1; return 0;		/* 96/June/11 */
		case 'L': addligatures = 1; return 0;		/* 97/Sep/28 */
		case 'S': smallcapsflag = 1; return 0;		/* 97/Sep/28 */
		case 'I': individualbboxes = 1; return 0;	/* 00/Jul/15 */
/* following take arguments */
		case 'a': afmflag = 1; return -1;
		case 'e': extremaflag = 1; return -1;
  		case 'o': keepoldorder = 1; return -1;
/*		case 's': stretchflag = 1; return -1; */	/* needs argument */
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
}

/* void addslash(char *name) {
	int n;
	n = strlen(name);
	if (n == 0) return;
	if (*(name + n - 1) != '\\') strcat(name, "\\");
} */

/* Modified 1993/June/21 to complain if TEMP env var set wrong */

void maketemporary (char *new, char *name, char *ext) {
	int n;
	FILE *test;
	
	strcpy(new, tempdir);
	n = strlen(new);
/*	if (n > 0 && *(name + n - 1) != '\\') strcat(new, "\\"); */
	if (n > 0 && *(new + n - 1) != '\\') strcat(new, "\\");
	strcat(new, name);		
	makename(new, ext);
/*	Try it!  See if can write to this place! */
	if ((test = fopen(new, "wb")) == NULL) {
		fprintf(errout, "WARNING: Temporary directory `%s' does not exist\n",
			tempdir);
		tempdir = "";
		strcpy(new, name);		
		makename(new, ext);
	}
	else fclose(test);
}

/* Sep 27 1990 => 1990 Sep 27 */

void scivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 7);
	for (k = 5; k >= 0; k--) date[k+5] = date[k];
/*	date[11] = '\0'; */
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
	(void) _getch();  
	return hash;
}

/* char *programversion = "SIDEBEAR - adjusts sidebearings & widths - version 1.0"; */

/* char *progfunction="extract AFM metric info from PFA outline font file";*/

char *progfunction="extract metric info";

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);	 
/*	fprintf(output, "%s (%s %s)\n", programversion, date, compiletime); */
	fprintf(output, "%s - %s - version %s (%s %s)\n", 
		progname, progfunction, progversion, date, compiletime);
}

int decodearg (char *command, char *next, int firstarg) {
	char *s;
	char *sarg=command;
	int c;
	
	if (*sarg == '-' || *sarg == '/') sarg++;	/* step over `-' or `/' */
	while ((c = *sarg++) != '\0') {				/* until end of string */
		if (decodeflag(c) != 0) {				/* flag requires argument ? */
/*			if ((s = strchr(sarg, '=')) == NULL) { */
			if (*sarg != '=' && *sarg != ':') {	/* arg in same string ? */
				if (next != NULL) {
					firstarg++; s = next;	/* when `=' or `:' is NOT used */
				}
				else {
					fprintf(errout, "Don't understand: %s\n", command);
					return firstarg;
				}
			}
/*			else s++;	*/		/* when `=' IS used */
			else s = sarg+1;	/* when `=' or `:' IS used */

/* now analyze the various flags that could have gotten set */
			if (afmflag != 0) {
/*				if (sscanf(s, "%lg", &tracking) < 1)
					fprintf(errout, "Don't understand %s\n", s);
				if (tracking > 10.0) tracking = tracking / 100.0;
				trackflag++; */
//				afmfile = s;
				strcpy(line, s);
				extension(line, "afm");
				afmfile = zstrdup(line);
				afmflag = 0; 
				useoldafmini = 1;
			} 
			break;	/* default - no flag set */
		}
	}
	return firstarg;
}

/*	check command line flags and command line arguments */

int commandline (int argc, char *argv[], int firstarg) {
	int c;
	
	if (argc < 2) showusage(argv[0]);
	c = argv[firstarg][0];
	while (c == '-' || c == '/') {
		firstarg = decodearg(argv[firstarg], argv[firstarg+1], firstarg+1);
		if (firstarg >= argc) break;			/* safety valve */
		c = argv[firstarg][0];
	}
	return firstarg;
}

void uppercase (char *s) {
	int c;
	while ((c = *s) != '\0') {
		if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
		*s = (char) c;
		s++;
	}
}

/* return file name minus path when given fully qualified name */

char *strippath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = strippath(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
}

void freecompnames(void) {
	int m;
/*	for (m = 0; m < MAXCOMPOSITES; m++) */
	for (m = 0; m < maxcomposites; m++) {
/*		if (strcmp(composite[m], "") != 0) { */
		if (composite[m] != NULL) {
			free(composite[m]);
			composite[m] = NULL;
		}
	}
}

void freeencoding(void) {
	int m;
	for (m = 0; m < MAXCHRS; m++) {
/*		if (strcmp(encoding[m], "") != 0) { */
		if (encoding[m] != NULL) {
			free(encoding[m]);
			encoding[m] = NULL;
		}
	}	
}

void freeglyphnames(void) {
	int k;
	if (glyphs == NULL) return;
	for (k = 0; k < glyphindex; k++) {
		if (glyphs[k] != NULL) {
			free(glyphs[k]);
			glyphs[k] = NULL;
		}
	}
}

int _cdecl main(int argc, char *argv[]) {       /* main program */
    FILE *fp_in, *fp_out;
/*	unsigned int i; */
	int firstarg=1;
	int c, d, k;
/*	int m, flag; */
	char *s;
	char *writemode;
	int ncount=0;

/*	strcpy(fn_out, ""); */

	if (strlen(progname) < 16) strcpy(progname, compilefile);
	if ((s = strchr(progname, '.')) != NULL) *s = '\0';
	uppercase(progname);

	strcpy (line, argv[0]);
	uppercase (line);
	if (strstr(line, progname) == NULL) quietflag = 1;

	if (argc < firstarg+1) showusage(argv[0]); 

	firstarg = commandline(argc, argv, 1);

	if (wipeclean == 4) wipeclean = 0;	/* `ddd' on command line */
	if (wipeclean == 2) debugflag = 1;	/* 1994/June/28 */

	if (argc <= firstarg) showusage(argv[0]); 
		
	if (quietflag != 0) {
		verboseflag = 0; traceflag = 0; dotsflag = 0; 
/*		tracking=1.0; */
	}

/*	scivilize(compiledate);	 */
	if (quietflag == 0) stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	(void) signal(SIGINT, ctrlbreak);

	if ((s = getenv("TMP")) != NULL) tempdir = s;
	if ((s = getenv("TEMP")) != NULL) tempdir = s;

/*	printf( "Sidebearing and advance width modification program version %s\n",
		VERSION); */
	
	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

/*	for (k = 0; k < MAXCOMPOSITES; k++) composite[k] = ""; */
	for (k = 0; k < maxcomposites; k++) composite[k] = NULL;

/*	for (k = 0; k < MAXCHRS; k++) encoding[k] = ""; */
	for (k = 0; k < MAXCHRS; k++) encoding[k] = NULL;

	extractflag = 1;		/* default is to just extract old info */

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

	for (k = firstarg; k < argc; k++) {		/* 1993/Nov/20 */

		maxsubrs = 0;
		maxcharstrings = 0;
		subrpos = NULL;
		charstringpos = NULL;

		extendedseac = 0;
		useoldafm = useoldafmini;			/* reset to command line value */

		isfixedpitch = 0;

		strcpy(fn_in, argv[k]);				/* get file name */
		extension(fn_in, "pfa");

		if (verboseflag != 0) printf("Processing file %s\n", fn_in);

		if ((fp_in = fopen(fn_in, "rb")) == NULL) {	/* see whether exists */
			underscore(fn_in);						/* 1993/May/30 */
			if ((fp_in = fopen(fn_in, "rb")) == NULL) {/* see whether exists */
/*				perror(fn_in);	exit(13); */
//				perror(fn_in);
				perrormod(fn_in);
				continue;
			}
		}
		c = fgetc(fp_in); d = fgetc(fp_in);
		fclose(fp_in);							/* Don't want binary mode */
		if (c == '%' || d == '!') {				/* could it be a PFA file */
		}
		else if (c == 'S' && d == 't') {		/* is it an AFM file ? */
			fprintf(errout, "ERROR: This appears to be an AFM file, not PFA\n");
/*			exit(1); */
			continue;							/* 93/Nov/20 */
		}
		else {
			fprintf(errout, "ERROR: %s does not appear to be a PFA file\n",
				   fn_in);
/*			exit(13); */
			continue;							/* 93/Nov/20 */
		}

		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
		strcpy(fn_out, s);			/* copy input file name minus path */

//		if (strcmp(fn_in, fn_out) == 0 && extractflag == 0) {
//			strcpy(fn_bak, fn_in);
//			forceexten(fn_in, "bak");
//			printf("Renaming %s to %s\n", fn_bak, fn_in);
//			remove(fn_in);		/* in case backup version already present */
//			rename(fn_bak, fn_in);
//			renameflag++;
//		}
	
		if (verboseflag != 0 && extractflag == 0) 
			printf("Output is going to %s\n", fn_out);

/*	drop through here if it is a PFA file */

/*		(void) tmpnam(fn_dec); */	/* create temporary file name */
/*		strcpy(fn_dec, fn_out); */
/*		makename(fn_dec, "dec"); */
		maketemporary(fn_dec, fn_out, "dec");
		if (traceflag != 0) {
			printf("Using %s as temporary\n", fn_dec);
			fflush(stdout);
		}
/*		strcpy(fn_pln, fn_out); */
/*		makename(fn_pln, "pln"); */
		maketemporary(fn_pln, fn_out, "pln");
		if (traceflag != 0) {
			printf("Using %s as temporary\n", fn_pln);
			fflush(stdout);
		}
		if (extractflag == 0) {
/*			strcpy(fn_adj, fn_out); */
/*			makename(fn_adj, "adj"); */
			maketemporary(fn_adj, fn_out, "adj");
			if (traceflag != 0) {
				printf("Using %s as temporary\n", fn_adj);
				fflush(stdout);
			}
		}

		if (traceflag != 0) {
			printf("Pass A (down) starting\n");			/* */
			fflush(stdout);
		}
	
		if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
			underscore(fn_in);						/* 1993/May/30 */
			if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
//				perror(fn_in);
				perrormod(fn_in);
				exit(2);
			}
		}
		if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
//			perror(fn_dec);
			perrormod(fn_dec);
			exit(2);
		}

		eexecscan = 1;
		charscan = 0;  decodecharflag = 0;  charenflag = 0; binaryflag = 0;

		(void) decrypt(fp_dec, fp_in);		/* actually go do the work */

		fclose(fp_in);
		if (ferror(fp_dec) != 0) {
			fprintf(stderr, "Output error ");
//			perror(fn_dec);
			perrormod(fn_dec);
			cleanup();
			exit(2);
		}
		else fclose(fp_dec);

		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
		
/*		if (debugflag) _getch(); */
		
		if (traceflag != 0) {
			printf("Pass B (down) starting\n");			/* */
			fflush(stdout);
		}
		
		if ((fp_dec = fopen(fn_dec, "rb")) == NULL) { 
//			perror(fn_dec);
			perrormod(fn_dec);
			cleanup();
			exit(2);
		}
		if ((fp_pln = fopen(fn_pln, "wb")) == NULL) { 
//			perror(fn_pln);
			perrormod(fn_pln);
			cleanup();
			exit(2);
		}

		eexecscan = 0;
		charscan = 1;  decodecharflag = 1;  charenflag = 1;  binaryflag = 1;

		(void) decrypt(fp_pln, fp_dec);		/* actually go do the work */

		fclose(fp_dec);
		if (ferror(fp_pln) != 0) {
			fprintf(stderr, "Output error ");
//			perror(fn_pln);
			perrormod(fn_pln);
			cleanup();
			exit(2);
		}
		else fclose(fp_pln);

		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	
/*		if (debugflag) _getch(); */
	
		if (extractflag) {		/* no need to go upward */
			if ((fp_pln = fopen(fn_pln, "rb")) == NULL) {
//				perror(fn_pln);
				perrormod(fn_pln);
				cleanup();
				exit(33);
			}
//			forceexten(fn_out, "afm");
			forceexten(fn_out, "xxx");		// use weird extension for now
//			see if name conflict
//			if (useoldafmini && strcmp(fn_out, afmfile) == 0) {
//				strcpy(line, afmfile);
//				forceexten(line, "old");
//				printf("Renaming %s to %s\n", afmfile, line);
//				if (remove(line)) {
//				}
//				if (rename(afmfile, line)) {
//					printf("Failed to rename %s to %s\n", afmfile, line);
//				}
//				afmfile = zstrdup(line);		// save new name
//			}
			if (verboseflag) printf("Opening %s for output\n", fn_out);
			if (unixflag) writemode = "wb";
			else writemode = "w";
			if ((fp_out = fopen(fn_out, writemode)) == NULL) { 
				perrormod(fn_out);
				cleanup();
				exit(2);
			}

			(void) extractafm(fp_out, fp_pln, 0);	/* extract char pos */
			rewind (fp_pln);
			if (verboseflag) putc('\n', stdout);
			(void) extractafm(fp_out, fp_pln, 1);	/* extract char BBoxes */
			rewind (fp_pln);
			if (verboseflag) putc('\n', stdout);
			extremacount = 0;
			(void) extractafm(fp_out, fp_pln, 2);	/* extract char BBoxes */
			if (verboseflag && extremacount > 0)
/* Good fonts should have few extremacounts */
/* except obliqued fonts, which can be expected to have many */
			printf("NOTE: %d curveto extrema not at ends of curveto\n",
				   extremacount);

			ncount++;
			fclose(fp_pln);
/*			if (wipeclean > 0) remove(fn_pln);  */
			if (ferror(fp_out) != 0) {
				fprintf(stderr, "Output error ");
//				perror(fn_out);
				perrormod(fn_out);
				cleanup();
				exit(2);
			}
			fclose(fp_out);
			strcpy(line, fn_out);		// rename ".xxx" to ".afm"
			forceexten(line, "afm");
			if (verboseflag) printf("Renaming %s to %s\n", fn_out, line);
			if (remove(line)) {
			}
			rename(fn_out, line);
			/*		cleanup(); */
			/*		return 0; */
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
		freecompnames();	/* ??? */
/*		if (standardflag == 0) freeencoding(); */
		freeencoding();
		freeglyphs();
		freecomposites();	/* ??? */
		freesubrs();
		if (charstringpos != NULL) {
			_ffree(charstringpos);
			charstringpos = NULL;
			maxcharstrings = 0;
		}
		cleanup();
		if (extendedseac > 0)				/* 1995/May/28 */
			printf("WARNING: font uses %d SEAC calls to glyphs not in ASE\n",
				  extendedseac);
		if (verboseflag) fflush(stdout);
	}	/* 	end of for (k = firstarg; k < argc; k++) */

	printf("Processed %d PFA files\n", ncount);
/*	cleanup(); */
	return 0;
}

/* does not deal with `sbw' form of metric info */

/* does not deal properly with `fractional widths' */ /* fixed now for hsbw */

/* cannot get CapHeight, XHeight, Ascender, Descender */

/* cannot get ligature information */

/* cannot get kerning pairs */

/* multi-file operation not debugged/tested 93/Nov/20 */

/* fixed hidden dependence on accents occuring before they are used */

/* now does three passes to get char positions and FontBBox right */

/* may not need to do all work in the first pass ? */

/* allow us of old AFM file to get accurate AFM header info and kern info */


/* FlxProc sequence
1 callsubr
50 0 rmoveto
2 callsubr
-35 0 rmoveto
2 callsubr
10 10 rmoveto
2 callsubr
25 0 rmoveto
2 callsubr
25 0 rmoveto
2 callsubr
10 -10 rmoveto
2 callsubr
15 0 rmoveto
2 callsubr
50 200 -10 0 callsubr */

/* make FLOATCOORDS permanent now that it works */
