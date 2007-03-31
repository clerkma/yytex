/* safeseac.c
   Copyright Y&Y 1992
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
#include <malloc.h>					/* for _heapchk() only */
#include <conio.h>

#include "metrics.h"

#ifdef _WIN32
#define __far 
#define _fmalloc malloc
#define _frealloc realloc
#define _ffree free
#endif

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int debugflag = 0;
int heapflag = 1;
int wipeclean = 1;	
int dotsflag = 0;
int quietflag = 0;
int ignorebadends = 0;	/* ignore Subrs with endchar 95/April/15 */
int ignoreseac = 0;		/* only fix Subrs with endchar 95/April/15 */
						/* (do not actually process SEACs) */
int relaxseac = 0;		/* non-zero means allow even if chars not in ASE */
						/* provides for composites of non ASE chars */
int needtorelax = 0;

int badsubr = 0;		/* set if refrences to non-existent Subrs */

int renameflag = 0;

int replacesubr = 1;	/* generate hint replacement Subrs */

int callaccentsinit = 1;/* use accent Subrs for accents also, global */
int callaccents = 1;	/* use accent Subrs for accents themselves-per file */

int wantdotsection = 0;	/* non-zero => add dot section around call to accent */

int copyaccents = 1;	/* copy accent into each accented character */
int flushother = 1;		/* flush existing hint replacement code in accent */
int initmoved = 1;		/* do initial moveto outside subroutine */
int wantclosed = 1;		/* use `closepath' after `rmoveto' */
int wantzeroln = 1;		/* use `lineto' after `rmoveto' */
int recurseexp = 1;		/* expand Subr's recursively */
int dummyswitch = 1;	/* include dummy hint switching code */

int switchtrick = 0;	/* non-zero if font uses hint switching trick */

/* int upperonly = 0; */		/* copy accents only when ady not zero ??? */

int eexecscan = 0;		/* scan up to eexec before starting */
int charscan = 0;		/* scan up to RD before starting */
int charenflag = 0;		/* non-zero charstring coding instead of eexec */
int decodecharflag = 0;	/* non-zero means also decode charstring */
int binaryflag = 0;		/* input is binary, not hexadecimal */

int writeoutflag = 1;	/* zero => extract info only - debugging */

/* int forceoutname = 0; */	/* don't use input file name for output file name */

int hintstrip;			/* non-zero if accent used hint replacement */

int dummysubr;			/* number of dummy hint switching Subr */

int kcurrent;			/* for callothersubr removing error message */

FILE *errout=stdout;	/* error messages */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

volatile int bAbort=0;			/* set when user types control-C */

/* char *outfilename = "safeseac.pfa"; */ 			/* not used ? */

extern int encrypt(FILE *, FILE *);

extern int decrypt(FILE *, FILE *);

char *tempdir="";		/* place to put temporary files */

char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
char fn_bak[FILENAME_MAX], fn_des[FILENAME_MAX];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *charnames[MAXCHRS];	/* just for the encoding 1995/May/8 */

int ncharstrings;		/* number of CharStrings in original PFA file */

/* int ncharstrings1, ncharstrings2; */

int nsubrs;				/* number of Subrs in original PFA file */

int nsubrsbase;			/* added Subrs for base characters */

int nsubrsaccent;		/* added Subrs for accent characters */

int seaccount;			/* total number of composites */

int nsubrsadd;			/* Subrs to add based on CharStrings & seac */

int nsubrsout;			/* Subrs written to output */

int noverlap;			/* overlap between base and accent characters */

/* int nsubrs1, nsubrs2; */

int ncharhits;			/* how many characters given in desired file */

int nwanted;			/* number of characters seen in desired file */

int charindex = 0;

int charseen = 0;		/* non-zero after character string RD and ND seen */
int subrseen = 0;		/* non-zero after subrs NP seen */
int nextend = 0;		/* non-zero if next line is ND */

int notdefseen = 0;		/* seen .notdef in this font */

int othersubrseen = 0;		/* non-zero if OtherSubrs seen */

int needothersubrs = 0;		/* non-zero if have to add in Othersubrs */

int safealready = 0;		/* non-zero if comment from SAFESEAC found */

int forceseac = 0;			/* force work even if apparently SAFE */

int encodingexists = 0;		/* if encoding read successfully */

int requiresfix = 0;		/* if char using SEAC found in actual encoding */

/* int needsaccent = 0; */		/* if encoding has accented chars without accents*/

int errlvlflag = 0;			/* return with error status if requested */

char rdsynom[16] = "RD";	/* RD or -| or ... */
char ndsynom[16] = "ND";	/* ND or |- or `noaccess def' */
char npsynom[16] = "NP";	/* NP or | or  `readonly put' */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

int wantcpyrght=1;

/* Program for turning `seac' calls into Subr calls */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* char *progname = "SAFESEAC"; */

char progname[16] = "";

/* char *progversion = "1.1"; */
/* char *progversion = "1.2"; */
/* char *progversion = "1.3"; */		/* 1995/April/15 */
/* char *progversion = "1.4"; */		/* 1995/May/10 */
/* char *progversion = "1.5"; */		/* 1995/Nov/17 */
/* char *progversion = "1.6"; */		/* 1998/May/12 */
char *progversion = "2.0";				/* 1998/Dec/25 */

char *copyright = "\
Copyright (C) 1992-1998  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1992--1995  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 13986445 */
/* #define COPYHASH 7073392 */
/* #define COPYHASH 16446505 */
/* #define COPYHASH 5618236 */
/* #define COPYHASH 11501465 */
/* #define COPYHASH 2658560 */
#define COPYHASH 3956063

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

struct vector {				/* structure for a vector */
	double x; double y;
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* MAXADJUST is (MAXCHRS+MAXCHRS) to accomadate SEAC calls with 256 offset */

/* following ONLY used for characters found in StandardEncoding */

/* and now also for characters in font encoding if not conflict offset by 256 */

/* Note: MAXADJUST = (MAXCHRS+MAXCHRS) */

long charposition[MAXADJUST];	/* Position of CharString in file */

int charside[MAXADJUST];		/* sidebearing of character - unused ??? */

int charinix[MAXADJUST], chariniy[MAXADJUST];	/* initial moveto */

int chardx[MAXADJUST], chardy[MAXADJUST];		/* total displacement */

int charsubrs[MAXADJUST];		/* number of new `Subr' correspond to char */

int charreplace[MAXADJUST];	/* hint replacement Subr for accent */

int charbase[MAXADJUST];		/* if CharString called by `seac' as base */

int characcent[MAXADJUST];		/* if CharString called by `seac' as accent */

int charusesrep[MAXADJUST];	/* if character uses hint replacement */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* The following now get allocated 97/June/20 */

int nsubrs=0;					/* obtained from /Subrs line */

int maxsubrs=0;					/* number actually allocated */

/* long subrpos[MAXSUBRS]; */

long __far *subrpos=NULL;		/* position of Subr in file */

/* int subrsdx[MAXSUBRS], subrsdy[MAXSUBRS]; */

short __far *subrsdx=NULL;	/* total x displacement of Subr */
short __far *subrsdy=NULL;	/* total y displacement of Subr */

/* char badend[MAXSUBRS]; */

short __far *badend;		/* marked for subrs that end in `endchar' */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void freesubrs(void) {
	if (subrpos != NULL) free(subrpos);
	if (subrsdx != NULL) free(subrsdx);
	if (subrsdy != NULL) free(subrsdy);
	if (badend != NULL) free(badend);
	subrpos = NULL;
	subrsdx = subrsdy = NULL;
	badend = NULL;
	maxsubrs = 0;
}

void allocsubrs(int nsubrs) {
	int k;
	if (nsubrs == maxsubrs) return;
	if (maxsubrs > 0) freesubrs();
	maxsubrs = 0;
	if (verboseflag)printf("Allocating space for %d Subrs\n", nsubrs);
	subrpos = (long __far *) _fmalloc(nsubrs * sizeof(long));
	subrsdx = (short __far *) _fmalloc(nsubrs * sizeof(short));
	subrsdy = (short __far *) _fmalloc(nsubrs * sizeof(short));
	badend = (short __far *) _fmalloc(nsubrs * sizeof(short));
	if (subrpos == NULL || subrsdx == NULL || subrsdy == NULL || badend == NULL) {
		fprintf(errout, "Unable to allocate %ld bytes\n",
				nsubrs * sizeof(short));
		exit(1);
	}
	for (k = maxsubrs; k < nsubrs; k++) {
		subrpos[k] = -1;
		subrsdx[k] = subrsdy[k] = badend[k] = 0;
	}
	maxsubrs = nsubrs;
}

/* Only get here if /Subrs count was inaccurate - actually not used yet */

void reallocsubrs(int nsubrs) {
	int k;

	if (nsubrs == maxsubrs) return;
	if (verboseflag)
		printf("Reallocating space for %d => %d Subrs\n", maxsubrs, nsubrs);
	subrpos = (long __far *) _frealloc(subrpos, nsubrs * sizeof(long));
	subrsdx = (short __far *) _frealloc(subrsdx, nsubrs * sizeof(short));
	subrsdy = (short __far *) _frealloc(subrsdy, nsubrs * sizeof(short));
	badend = (short __far *) _frealloc(badend, nsubrs * sizeof(short));
	if (subrpos == NULL || subrsdx == NULL || subrsdy == NULL || badend == NULL) {
		fprintf(errout, "Unable to allocate %ld bytes\n",
				nsubrs * sizeof(short));
		exit(1);
	}
	for (k = maxsubrs; k < nsubrs; k++) {
		subrpos[k] = -1;
		subrsdx[k] = subrsdy[k] = badend[k] = 0;
	}
	maxsubrs = nsubrs;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *zstrdup(char *name) {
	char *new=_strdup(name);
	if (new == NULL) {
		fprintf(stderr,
				"ERROR: Out of memory for character names (%s)\n", name);
		exit(1);
	}
	return new;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	 
void abortjob (void);

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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static char line[MAXLINE];		/* buffer for getline */

static char buffer[MAXLINE];	/* buffer for checkdiv */

/* Returns 0 for EOF, otherwise returns number of bytes */

static int getline(FILE *fp_in, char *line) {
	char *s; 
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	s = line + strlen(line) - 1; 
	if (*s == '\r') *(s-1) = '\0';	/* flush terminating return */
	return strlen(line); 
} 

/* look up character in StandardEncoding table */

int lookupstandard(char *name) {	/* look up in Adobe StandardEncoding */
	int k, flag = -1;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(standardencoding[k], name) == 0) {
			flag = k;	break;
		}
	}
	return flag;
}

int lookupencoding(char *name) {	/* look up in font's current encoding */
	int k, flag = -1;
	for (k = 0; k < MAXCHRS; k++) {
		if (charnames[k] == NULL) continue;
		if (strcmp(charnames[k], name) == 0) { 
			flag = k;	break;
		}
	}
	return flag;
}

void stripreturn (char *name) {
	char *s;
	s = name + strlen(name);
	while (*s <= ' ') s--;
	*(s+1) = '\0';	
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* only needed for check on PS interpreter bug with accents missing */

/* char *accents[] = {
"grave", "acute", "circumflex", "tilde", 
"macron", "breve", "dotaccent", "dieresis", "ring",
"cedilla", "hungarumlaut", "ogonek", "caron",
""}; */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following revised 1994/June/21 - copy new code to SAFESEAC and PFATOAFM */

void checkfordiv (char *line) {
	char *s, *t;
/*	long num, den, ratio; */
	double num, den, ratio;
	int k=0;

/*	fputs(line, stdout); */				/* debugging */
	while ((s = strstr(line, "div")) != NULL) {
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
/*			sprintf(s, "%ld", ratio); */
			sprintf(s, "%lg", ratio);
		}
		else {
			fprintf(errout, "Can't make sense of: %s", line); /* 1993/Dec/20 */
			break;
		}
/*		(void) memmove (s + strlen(s), t, strlen(t)+1); */
		strcat (s, buffer);					/* put back the tail end */
		if (debugflag != 0) {
			printf("num %lg den %lg ratio %lg: ", num, den, ratio);
			fputs(line, stdout);				
		}
		if (bAbort != 0) abortjob();
		if (k++ > 16) break;				/* emergency exit */
	}
}

/* using established x and y offset in Subrs (while tracing Subrs) */ 
/* may need to do this multi-pass */

struct vector movement(char *line) {
	struct vector del;
	double delx, dely;
	double delx1, dely1, delx2, dely2, delx3, dely3;
	int n, na, nb, nc, nd;

	checkfordiv(line);	/* flush those pesky "div" - allow for non-integer */

	delx = 0; dely = 0;

	if (strstr(line, "lineto") != NULL) {
/*		delx = 0; dely = 0; */
		if (strstr(line, "rlineto") != NULL) {
			if (sscanf(line, "%lg %lg rlineto", &delx, &dely) < 2)
				fputs(line, errout);
		}
		else if (strstr(line, "hlineto") != NULL) {
			if (sscanf(line, "%lg hlineto", &delx) < 1)
				fputs(line, errout);
		}
		else if (strstr(line, "vlineto") != NULL) {
			if (sscanf(line, "%lg vlineto", &dely) < 1)
				fputs(line, errout);
		}
	}
	else if (strstr(line, "moveto") != NULL) {
/*		delx = 0; dely = 0; */
		if (strstr(line, "rmoveto") != NULL) {
			if (sscanf(line, "%lg %lg rmoveto", &delx, &dely) < 2)
				fputs(line, errout);
		}
		else if (strstr(line, "hmoveto") != NULL) {
			if (sscanf(line, "%lg hmoveto", &delx) < 1)
				fputs(line, errout);
		}
		else if (strstr(line, "vmoveto") != NULL) {
			if (sscanf(line, "%lg vmoveto", &dely) < 1)
				fputs(line, errout);
		}
	}
	else if (strstr(line, "curveto") != NULL) {
		delx1 = 0; dely1 = 0;
/*		delx2 = 0; dely2 = 0; */
		delx3 = 0; dely3 = 0;				
		if (strstr(line, "rrcurveto") != NULL) {
			if (sscanf(line, "%lg %lg %lg %lg %lg %lg rrcurveto", 
				&delx1, &dely1, &delx2, &dely2, &delx3, &dely3) < 6)
					fputs(line, errout);
		}
		else if (strstr(line, "hvcurveto") != NULL) {
			if (sscanf(line, "%lg %lg %lg %lg hvcurveto", 
				&delx1, &delx2, &dely2, &dely3) < 4)
					fputs(line, errout);
		}
		else if (strstr(line, "vhcurveto") != NULL) {
			if (sscanf(line, "%lg %lg %lg %lg vhcurveto", 
				&dely1, &delx2, &dely2, &delx3) < 4)
					fputs(line, errout);
		}
		delx = delx1 + delx2 + delx3;	/* delx2 defined ? */
		dely = dely1 + dely2 + dely3;	/* dely2 defined ? */
	}
	else if (strstr(line, "stem") != NULL) ;
	else if (strstr(line, "dotsection") != NULL) ;
	else if (strstr(line, "closepath") != NULL) ;
	else if (strstr(line, "pop") != NULL) ;
	else if (strstr(line, "setcurrentpoint") != NULL) ;
	else if (strstr(line, "return") != NULL) ; /* shouldn't happen */
	else if (strstr(line, "endchar") != NULL) ; /* shouldn't happen */
	else if (strstr(line, "callothersubr") != NULL) {
/*		skip some lines ahead `pop', `callsubr' ??? */
	}
/* following won't happen while tracing CharStrings */
	else if (strstr(line, "callsubr") != NULL) {
		n = sscanf (line, "%d %d %d %d", &na, &nb, &nc, &nd);
/*		n == 0 probably is callsubr following callothersubr hint replacement */
		if (n == 0) ;	/* nothing to do */
/*  n == 1 probably is proper normal Subrs call - replace if > 3 */
		else if (n == 1) {
/*			if (na > 3 && na < MAXSUBRS)  */
			if (na > 3 && na < maxsubrs) {
				if (subrsdx[na] != 0 || subrsdy[na] != 0) {
					delx = subrsdx[na]; dely = subrsdy[na];
				}
			}
		}
/*  n == 2 probably is new trick to reduce verbage in hint replacement */
		else if (n == 2 && nb == 4) {
			if (na > 3) {
			}
		}
/* n == 4 probably FlexProc call */	/* 1993/Jan/4 */
		else if (n == 4 && nd == 0) {
		}
		else if (traceflag != 0) printf("n = %d line: %s", n, line);
	}
/*	else fputs(line, errout); */	/* don't recognize the line */
	else fprintf(errout, "BAD LINE: %s", line);	/* don't recognize the line */
	
/* should not see `endchar', `hsbw', `sbw', `seac' */
/* ignore `hstem', `vstem', `hstem3', `vstem3', `dotsection' */
/* ignore `closepath', `callothersubr', `callsubr' */
/* NEED TO PAY ATTENTION TO CALLSUBR ! pick up movement from there */
		
	del.x = delx; del.y = dely;
	return del;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* now that we recursively trace CharString to Subrs, no need for Subr displ */

double dx, dy;				/* accumulated movement in CharString */

int inix, iniy;				/* need double here ? */

int initflag;
int questionflag;			/* initial moveto position */
int hitseacflag;			/* 1995/May/20 */

/* don't we need double in following instead of int ? and checkfordiv ? */

/* Note that 0 <= k < MAXCHRS for glyphs found in ASE */
/* and MAXCHRS <= k < MAXADJUST otherwise */

int expandchar(FILE *fp_in, int k) {
	int n;
	int inix, iniy;
	int na, nb, nc, nd;
	long filepos;
	struct vector del;
	char *charname;							/* 1995/May/8 */

	while ((n = getline(fp_in, line)) != 0 &&
		strstr(line, ndsynom) == NULL) {
/* hit end of character, drop everything ! */
		if (strstr(line, "endchar") != NULL) return -1;
/* hit end of Subr, return to previous level */
		if (strstr(line, "return") != NULL) return 0;
/* hit seac call, make a note of it 95/May/20 - don't complain initflag */
		if (strstr(line, "seac") != NULL) {	
			hitseacflag++;
			return -1;								/* pop right out again */
		}
/* take care of `moveto', if it is the first one seen */
		if (initflag == 0 && strstr(line, "moveto") != NULL) {
			inix = 0; iniy = 0;
			if (strstr(line, "rmoveto") != NULL) 
				sscanf(line, "%d %d rmoveto", &inix, &iniy);
			else if (strstr(line, "hmoveto") != NULL) 
				sscanf(line, "%d hmoveto", &inix);
			else if (strstr(line, "vmoveto") != NULL) 
				sscanf(line, "%d vmoveto", &iniy);
			else fputs(line, errout);
			charinix[k] = inix; chariniy[k] = iniy;
			if (traceflag != 0) {
				if (questionflag != 0) putc('\n', stdout);
				questionflag = 0;
				if (k < MAXCHRS) charname = standardencoding[k];
				else charname = charnames[k-MAXCHRS];
				printf("char: `%s'\tXO %d YO %d ", charname, inix, iniy);
/*				printf("char: `%s'\tXO\t%d\tYO\t%d ", 
					standardencoding[k], inix, iniy); */
			}
			initflag++;
		}
		if (strstr(line, "callothersubr") != NULL) {
			charusesrep[k]++; /* make a note that it probably uses hint rep */
		}
		else if (strstr(line, "callsubr") != NULL) {
			n = sscanf(line, "%d %d %d %d", &na, &nb, &nc, &nd);
			if (n == 1) {
/*				if (na > 3 && na < MAXSUBRS) */
				if (na > 3 && na < maxsubrs) {
				filepos = ftell(fp_in);					/* remember */
				fseek(fp_in, subrpos[na], SEEK_SET);	/* go to Subr */
				if (expandchar(fp_in, k) != 0) {
					fseek(fp_in, filepos, SEEK_SET);	/* reset position */
					return -1;	/* recurse */
				}
				else fseek(fp_in, filepos, SEEK_SET);	/* reset position */
				}
			}
		}
		else if (strstr(line, "pop") != NULL) ;
		else if (strstr(line, "setcurrentpoint") != NULL) ;
/*		else if (strstr(line, "seac") != NULL) hitseacflag++; *//* 95/May/20 */
		else {								/* see whether actual movement */
			del = movement(line);
			dx += del.x; dy += del.y;		/* update position */
		}
	}
	return 0;		/* hit ndsynom */
}

/* this pulls out positions of all characters found in StandardEncoding */
/* this pulls out positions of all Subrs */
/* this figures out initial moveto in CharStrings */
/* this locates all `seac' calls and determines what is called */
/* collects `hits' for base and accent characters */
/* collects info on which Subrs end in `endchar' 95/April/15 */

int extractcharnames(FILE *fp_in) {
	char charname[CHARNAME_MAX];
	int subrnum, subrlen;
	char rdsubr[16];
	int c, k, n;
	int side, width;
	int asb, adx, ady, bchar, achar;
/*	double dx, dy;  */
	struct vector del;
	int changedflag;
	char *s;
	long filepos, fileloc;
/*	int inix, iniy, initflag; */
/*	int questionflag=0; */
	int dotcount = 0;			/* 1993/July/3 */
	char *acharname, *bcharname, *scharname;		/* 1995/May/8 */

	charseen = 0; subrseen = 0; othersubrseen = 0; notdefseen = 0;

	switchtrick = 0;		/* reset flag that says this is used */
	hintstrip = 0;			/* reset flag that says hint stripping needed */
	questionflag=0;			/* no characters yet that are not in SE */
	safealready = 0;		/* 1993/June/8 */

/*	for (k = 0; k < MAXSUBRS; k++) */	/* 95/April/15 */
	for (k = 0; k < maxsubrs; k++) {
		subrsdx[k] = subrsdy[k] = badend[k] = 0;
	} /* first time, maxsubrs = 0 */

/* set up for reading Subrs first */

/*	for (k = 0; k < MAXSUBRS; k++)  */
/*	for (k = 0; k < maxsubrs; k++) subrpos[k] = 0; */

/*	currentdict end */
/*	currentfile eexec */

/*  scan up to eexec encrypted part */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
		if (*line == '%' && strstr(line, "SAFESEAC") != NULL) {
			safealready = 1;	/* 1993/June/8 */
			fprintf(errout,
			"WARNING: This font appears to already have been processed by SAFESEAC\n");
			if (forceseac == 0) {
	fprintf(errout, "         use `-f' to force modification anyway\n");
				return -1;
			}
/*			go on anyway if asked to `force' processing */
		}
	}

	filepos = ftell(fp_in);				/* remember where we are */

	for (k = 0; k < 16; k++) {		/* avoid infinite loop */
	changedflag = 0;
	if (traceflag != 0) printf("Pass %d through Subrs\n", k+1);
/*	scan up to CharStrings */
	while ((n = getline(fp_in, line)) != 0 && 
		(s = strstr(line, "/CharStrings")) == NULL) {
/*	see if can find Subrs first */
		if ((s = strstr(line, "/OtherSubrs")) != NULL)
			othersubrseen++;
		if ((s = strstr(line, "/Subrs ")) != NULL) {
			if (sscanf (s + 7, "%d", &nsubrs) == 0) {
				fprintf(errout, "Don't understand %s", line);
				exit(1);
			}
			if (traceflag != 0) fputs(line, stdout);
			allocsubrs(nsubrs);
/* scan the Subrs now */
			while ((n = getline(fp_in, line)) != 0 &&
				(s = strstr(line, "/CharStrings")) == NULL) {
/* beginning of next Subr ? */
				if (strncmp(line, "dup ", 4) == 0) {
					sscanf (line, "dup %d %d %s", &subrnum, &subrlen, rdsubr);
					stripreturn(rdsubr);
/*					if (subrnum >= 0 && subrnum < MAXSUBRS) */
					if (subrnum >= 0 && subrnum < maxsubrs)
						subrpos[subrnum] = ftell(fp_in);
/*					if (traceflag != 0) fputs(line, stdout); */
/* special case check on Subr number 4 - but only first pass */
					if (subrnum == 4 && switchtrick == 0) {
						fileloc = ftell(fp_in);
						n = getline(fp_in, line);
						if (strstr(line, "1 3 callothersubr") != NULL) {
							n = getline(fp_in, line);
							if (strstr(line, "pop") != NULL) {
								n = getline(fp_in, line);
								if (strstr(line, "callsubr") != NULL) {
									n = getline(fp_in, line);
									if (strstr(line, "return") != NULL) {
										switchtrick++;
										if (verboseflag != 0)
			printf ("NOTE: Font uses Subr 4 for efficient hint switching\n");
									}
								}
							}
						}
						fseek(fp_in, fileloc, SEEK_SET);
					}
/* scan this Subr */ /* assumed to end with `return' followed by NP */
					dx = 0; dy = 0;
					while ((n = getline(fp_in, line)) != 0 &&
						(s = strstr(line, "/CharStrings")) == NULL) {
						s = line;
						while (*s == ' ') s++;		/* 1993/Jan/4 */
						if (*s < ' ') break;		/* blank line */
/*  see if the end of the Subr */
						if (strncmp(line, "return", 6) == 0) {
/*	read line after `return' */
							n = getline(fp_in, line);
							if (subrseen == 0) {
								s = line;	/* deal with leading white space */
								while (*s == ' ') s++;
								strcpy(npsynom, s);
								stripreturn(npsynom);
				if (traceflag != 0) printf("NPsynonym: `%s'\n", npsynom);
								subrseen++;
							}
							break;
						}	
/*						if (strstr(line, "endchar") != NULL) continue; */
						if (strstr(line, "endchar") != NULL) { /* 95/Apr/15 */
							badend[subrnum] = 1;	/* mark as bad */
/*							printf("Subr %d has endchar\n", subrnum); */
							continue;
						}
						del = movement(line);
						dx += del.x; dy += del.y; 
					}
/*					if (dx != 0 || dy != 0) { */
					if (subrsdx[subrnum] != (int) dx ||
						subrsdy[subrnum] != (int) dy) {
						changedflag++;
						subrsdx[subrnum] = (short) dx; 
						subrsdy[subrnum] = (short) dy;
						if (traceflag != 0) 
							printf("Subr %d: DX %lg DY %lg\n", subrnum, dx, dy); 
/*							printf("Subr %d:\tDX\t%lg\tDY\t%lg\n", subrnum, dx, dy); */
					}
/*					} */
/* hit end of a Subr (return) */ /* check if next line is blank */

/* worry about white space at start of line: " noaccess def" form ! */
/*					c = getc(fp_in);  ungetc(c, fp_in); */
					while ((c = getc(fp_in)) == ' ') ; /* 1993/Jan/4 */
					ungetc(c, fp_in);
/*					printf("C is %d\n", c); */
					if (c > ' ') {
						if (traceflag != 0) 
							printf("End of Surbs on `%c'\n", c);	/* debug */
						break;		/* end of Subrs */
					}
				}
			}
			break;
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
	if (changedflag == 0) break;		/* escape finally ! */
	fseek(fp_in, filepos, SEEK_SET);
	}
	
	if (subrseen == 0) {
		fprintf(errout, "ERROR: no Subrs, cannot modify this file\n");
		fprintf(errout, "       use MERGEPFA to insert dummy Subrs\n");
		return -1;
	}

	if (verboseflag != 0) {
		if (k == 0) printf("NOTE: Subrs only used for hint replacement\n");
		else printf("NOTE: Had to make %d passes through Subrs\n", k+1);
	}

	seaccount = 0;					/* total composites */
	requiresfix = 0;				/* does is require fixing ? */

/* now deal with CharStrings */

	if (traceflag) printf("MAXCHRS %d MAXADJUST %d\n", MAXCHRS, MAXADJUST);

	for (k = 0; k < MAXADJUST; k++) {
		charposition[k] = 0;	/* -1 */
		charbase[k] = characcent[k] = 0;
		charinix[k] = chariniy[k] = UNKNOWN;
	}
		
	if (traceflag != 0) 
printf("Gather displacement and initial moveto information on CharStrings\n");

/*	if we popped out at end of Subrs from the above */
	while ((s = strstr(line, "/CharStrings")) == NULL) {	
		if ((n = getline(fp_in, line)) == 0) break;
		if (traceflag != 0) fputs(line, stdout);
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
		
/*	if (traceflag != 0) fputs(line, stdout); */
	sscanf(s + 13, "%d%n", &ncharstrings, &n);
/*	scan all of the CharStrings */ /* keep track of those in StandardEncoding*/
	for (;;) {
		n = getline(fp_in, line);
		if (n <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;
		if (charseen == 0 && nextend != 0) {
			s = line;		/* deal with leading white space */
			while (*s == ' ') s++;
			sscanf (s, "%s\n", ndsynom);
			stripreturn(ndsynom);
			if (traceflag != 0) printf("NDsynonym: `%s'\n", ndsynom);
			charseen = 1;
		}
		if (strstr(line, "endchar") != NULL) nextend = 1; else nextend = 0;
/*	see whether an `seac' to be remembered */
		if (strstr(line, "seac") != NULL) {
/* check whether this character name is in current encoding */
			if (sscanf(line, "%d %d %d %d %d seac", 
				&asb, &adx, &ady, &bchar, &achar) == 5) {
				if (bchar >= 0 && bchar < MAXCHRS) charbase[bchar]++;
				else if (relaxseac && bchar >= 0 && bchar < MAXADJUST)
					charbase[bchar]++;
				else if (relaxseac == 0 && bchar > MAXCHRS) {
					fprintf(errout, "SEAC not in ASE: %s", line);
					needtorelax++;
				}
				else fprintf(errout, "ERROR: %s", line);	/* 1993/April/12 */
				if (achar >= 0 && achar < MAXCHRS) characcent[achar]++;
				else if (relaxseac && achar >= 0 && achar < MAXADJUST)
					characcent[achar]++;
				else if (relaxseac == 0 && achar > MAXCHRS) {
					fprintf(errout, "SEAC not in ASE: %s", line);
					needtorelax++;
				}
				else fprintf(errout, "ERROR: %s", line);	/* 1993/April/12 */
/* charname presumably should not be in ASE - since char contains SEAC */
				k = lookupencoding(charname);	/* 1993/July/1 */
				if (k >= 0) {
/*					if (traceflag != 0) 
						printf("%d %s => %s %s\n", k, charname,
						standardencoding[bchar], standardencoding[achar]);*/
					if (traceflag != 0) {
						if (bchar < MAXCHRS)
							bcharname = standardencoding[bchar];
						else bcharname = charnames[bchar-MAXCHRS];
						if (achar < MAXCHRS)
							acharname = standardencoding[achar];
						else acharname = charnames[achar-MAXCHRS];
						printf("%d %s => %s %s\n", k, charname,
							   bcharname, acharname);
					}
					requiresfix++;
				}
				seaccount++;				/* total composites */
/*	compute minimum left side bearing at this point and save it up ??? */
			}
/*			else fputs(line, errout); */
			else fprintf(errout, "BAD SEAC: %s", line);
		}
		if ((s = strchr(line, '/')) != NULL) {
			if (charseen == 0) {
				sscanf(s, "/%s %d %s\n", charname, &k, rdsynom);
				stripreturn(rdsynom);
				if (traceflag != 0) printf("RDsynonym: `%s'\n", rdsynom);
			}
			sscanf(s, "/%s ", charname);
			if (strcmp(charname, ".notdef") == 0) notdefseen++;
/* k is negative if character is not in StandardEncoding */				
			k = lookupstandard(charname);
			if (relaxseac) {
				if (k < 0) {
					k = lookupencoding(charname);
					if (k >= 0) k += MAXCHRS; /* not in ASE, but in encoding */
				}
			}
			if (k >= 0) {
/* remember where this CharString starts (position after first line) */
				charposition[k] = ftell(fp_in);
				n = getline(fp_in, line);
/*			check out sidebearing and widths */
				if (strstr(line, "sbw") != NULL) {
					if (sscanf(line, "%d %d", &side, &width) > 0) 
						charside[k] = side;
					else fputs(line, errout);
				}
				else fputs(line, errout);

/*				inix = 0; iniy = 0; */				/* backup default ??? */
/*	now compute total displacement in this character (including Subrs) */
				dx = 0; dy = 0; 
				if (initmoved != 0) initflag = 0; /* only do if asked for */
				else initflag = -1;

				charusesrep[k] = 0;		/* reset uses hint replacement flag */
				hitseacflag = 0;		/* 95/May/20 */
				expandchar(fp_in, k);	/* trace along CharString and Subr */
				if (hitseacflag) {		/* step back to seac if possible */
					fseek(fp_in, charposition[k], SEEK_SET);	/* 95/May/20 */
				}
				if (strstr(line, "endchar") != NULL) nextend = 1; 
				else nextend = 0;
/*				if (hitseacflag) {	
					charposition[k] = 0;
					charinix[k] = UNKNOWN;
					chariniy[k] = UNKNOWN;
				} */ /* if we hit seac, remove this char */

/* following is for characters that do not have a moveto s.a. `space' */
				if (initflag == 0) {
					if (verboseflag != 0) {
						if (k < MAXCHRS) scharname = standardencoding[k];
						else scharname = charnames[k-MAXCHRS];
						if (hitseacflag == 0) {	/* 95/May/20 */
							if (questionflag != 0) putc('\n', stdout);
							questionflag = 0;
							if (traceflag == 0) putc('\n', stdout);
/*	printf("WARNING: char `%s' has no initial moveto (OK if blank glyph)\n" 
						standardencoding[k]); */
	printf("WARNING: char `%s' has no initial moveto (OK for `space')\n",
							   scharname);
						}
					}
					charinix[k] = chariniy[k] = UNKNOWN;
				}
/*				chardx[k] = dx;			chardy[k] = dy; */
				if (initflag != 0 && hitseacflag == 0) { /* 95/May/25 */
					chardx[k] = (int) dx; 	chardy[k] = (int) dy;
					if (traceflag != 0) 
						printf("\tDX %lg DY %lg\n", dx, dy); 
				}
			}
/*			else if (traceflag != 0) {
				printf("%s? ", charname);	
				questionflag++;
			} */ /* char is not in ASE */
/*			if (k < 0 || charinix[k] == UNKNOWN || chariniy[k] == UNKNOWN) { */
			if (k < 0) {
				if (traceflag != 0) {
					printf("%s? ", charname);	
					questionflag++;
				}  /* char is not in ASE - or is not in encoding or seac */
			}

			if (traceflag == 0) {
				if (dotsflag != 0) putc('.', stdout);
				else if (dotcount++ % 10 == 0) 	putc('.', stdout);
			}
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
/*	if (dotsflag != 0) */
		putc('\n', stdout);
	if (traceflag != 0) fprintf(errout, "End of extracting charnames\n");

/*	If no char containing SEAC is in encoding, and not being forced, don't */
/*	if (encodingexists != 0 && requiresfix == 0) */			/* 1993/July/1 */
/*		fprintf(errout, "WARNING: no character in current encoding using SEAC\n"); */
	if (seaccount == 0) {
		fprintf(errout, "WARNING: no character in this font using SEAC\n"); 
/*		if (needsaccent != 0) return 0; */		/* do anyway for accents ? */
		if (forceseac == 0) {
			fprintf(errout, "         use `-f' to force modification anyway\n");
			return -1;
		}
/*		otherwise, if forceseac is non-zero, go on and do it anyway */
	}
	return 0;
}

/* go and recursively expand Subr - adjust hints - adjust first rmoveto */
/* can start with CharString (such as for an accent) instead of Subr */
/* does not adjust hints in hint replacement calls using OtherSubrs ... */
/* does not adjust hstem3 and vstem3, or hints with "div" */
/* problem if moveto contains "div" .... */

/* returns non-zero if accent position has been adjusted */
/* returns zero if hmove and vmove still non-zero */

int expandsubr(FILE *fp_out, FILE *fp_in, int hmove, int vmove,
				int vadjust, int hadjust, int initial) {
	long filepos; 
	long subrposna; 
	int n, na, nb, nc, nd;
	int dx, dy;
	int start, width;
	char *scharname;								/* 1995/May/8 */

	while ((n = getline(fp_in, line)) != 0 &&
		strstr(line, ndsynom) == NULL) {
		if (strstr(line, "return") != NULL) break;	/* end of surboutine */
		if (strstr(line, "endchar") != NULL) break;	/* end of character */
		if (strstr(line, "sbw") != NULL) continue;	/* ignore at start */
		if (wantdotsection != 0 && strstr(line, "dotsection") != NULL) 
			continue;	/* ignore ! */
		if (flushother != 0 && strstr(line, "callothersubr") != NULL) {
/*			fputs(line, stdout); */			/* debugging */
/* verboseflag != 0 &&  */
			if (kcurrent > 0) {			/* kcurrent >= ? */
				hintstrip++;
				if (kcurrent < MAXCHRS)	scharname = standardencoding[kcurrent];
				else scharname = charnames[kcurrent-MAXCHRS];
				printf("NOTE: Removing existing hint replacement code in `%s'\n",
					   scharname);  /*	standardencoding[kcurrent]); */
/*				callaccents = 0;	*//* not safe to do here !!! */
			}
			n = getline(fp_in, line);
			if (strstr(line, "pop") == NULL) 
				fprintf(errout, "ERROR: expected `pop', not %s", line);
			n = getline(fp_in, line);
			if (strstr(line, "callsubr") == NULL) 
				fprintf(errout, "ERROR: expected `callsubr', not %s", line);
			continue;
		}
		if (strstr(line, "moveto") != NULL && initial != 0) {	
/* adjust only first moveto */
/* was (hmove != 0 || vmove != 0) */
			if (replacesubr == 0) {
			dx = 0; dy = 0;
			if (strstr(line, "rmoveto") != NULL) {
				if (sscanf(line, "%d %d rmoveto", &dx, &dy) == 2) ;
				else fputs(line, errout);
			}
			else if (strstr(line, "hmoveto") != NULL) {
				if (sscanf(line, "%d hmoveto", &dx) == 1) ;
				else fputs(line, errout);
			}
			else if (strstr(line, "vmoveto") != NULL) {
				if (sscanf(line, "%d vmoveto", &dy) == 1) ;
				else fputs(line, errout);
			}
			fprintf(fp_out, "%d %d rmoveto\n", dx + hmove, dy + vmove);
			}
/* leave out first moveto if this will be accent called from somewhere */
			else {
				if (wantdotsection != 0) fputs("dotsection\n", fp_out); 
			}
/*			if (wantdotsection != 0) fputs("dotsection\n", fp_out);  */
			hmove = 0; vmove = 0;	/* that is it, no more adjustments */
			initial = 0;
		}
/*		flush hints if hints already transferred to hint replacement subr */
/*		but only up to first moveto - just in case there are some later */
		else if (replacesubr != 0 && strstr(line, "stem") != NULL &&
			initial != 0 ) continue;		/* ignore hints then */
/* (hmove != 0 || vmove != 0) */
		else if (strstr(line, "stem") != NULL &&
			(hadjust != 0 || vadjust != 0)) {	/* only if need adjustment */
			if (strstr(line, "hstem") != NULL &&
				strstr(line, "hstem3") == NULL &&
					strstr(line, "div") == NULL) {
				if (sscanf(line, "%d %d hstem", &start, &width) == 2) {
					sprintf(line, "%d %d hstem\n", start + hadjust, width);
				}
				else fputs(line, errout);
			}
			if (strstr(line, "vstem") != NULL &&
				strstr(line, "vstem3") == NULL &&
					strstr(line, "div") == NULL) {
				if (sscanf(line, "%d %d vstem", &start, &width) == 2) {
					sprintf(line, "%d %d vstem\n", start + vadjust, width);
				}
				else fputs(line, errout);
			}			
			fputs(line, fp_out);
		}
/*		maybe need to weed out new `n 4 callsubr' calls also ? */
		else if (strstr(line, "callsubr") != NULL && recurseexp != 0) {
			if ((n = sscanf(line, "%d %d %d %d", &na, &nb, &nc, &nd)) == 1) {
				filepos = ftell(fp_in);	/* remember where we were */
/*				if (na < 0 || na > MAXSUBRS)  */
				if (na < 0 || na > maxsubrs) {
					fprintf(errout, 
						"ERROR: Asked to go to non-existent Subr %d\n", na);
					return 0;
				}
				subrposna = subrpos[na];
				if (subrposna == 0) {
					fprintf(errout, 
						"ERROR: Asked to go to non-existent Subr %d\n", na);
					return 0;
				}
				if (fseek(fp_in, subrposna, SEEK_SET) != 0) {
					fprintf(errout, "ERROR: seek to %ld failed\n", subrposna);
					return 0;
				}
				initial = expandsubr(fp_out, fp_in, hmove, vmove, 
					vadjust, hadjust, initial);
				if (initial == 0) {
					hmove = 0; vmove = 0;	/* found first moveto in subr */
				}
				fseek(fp_in, filepos, SEEK_SET);	/* back to where we were */
			}
/*			else if (n == 2 && nb == 4) ; *//* <n> 4 callsubr hint replace ! */
/*			else if (n == 4 && nd == 0) ; *//* FlexProc call probably */
			else fputs(line, fp_out);	/* not simple Subr call - just copy */
		}
		else fputs(line, fp_out);		/* not moveto, stem, callsubr */
	}
	if (bAbort != 0) abortjob();	/* in case stuck in loop */
	return initial;
/*	if (hmove == 0 && vmove == 0) return -1; */
/*	return 0; */
}

/* As above, expect copies over only initial hints; stops at end of hints */
/* Used only if replacesubr is non-zero to generate replacement Subrs */

int expandhints(FILE *fp_out, FILE *fp_in, int hmove, int vmove,
				int vadjust, int hadjust, int initial) {
	long filepos; 
	long subrposna; 
	int n, na, nb, nc, nd;
/*	int dx, dy; */
	int start, width;

	while ((n = getline(fp_in, line)) != 0 &&
		strstr(line, ndsynom) == NULL) {
		if (strstr(line, "return") != NULL) break;	/* end of surboutine */
		if (strstr(line, "endchar") != NULL) break;	/* end of character */
		if (strstr(line, "sbw") != NULL) continue;	/* ignore at start */
		if (strstr(line, "dotsection") != NULL) continue;	/* ignore ! */
/*		flush hint replacement call */ /* but make a note ??? */
		if (flushother != 0 && strstr(line, "callothersubr") != NULL) {
/*			fputs(line, stdout); */			/* debugging */
			n = getline(fp_in, line);
			if (strstr(line, "pop") == NULL) 
				fprintf(errout, "ERROR: expected `pop', not %s", line);
			n = getline(fp_in, line);
			if (strstr(line, "callsubr") == NULL) 
				fprintf(errout, "ERROR: expected `callsubr', not %s", line);
			continue;
		}
/*		once we hit moveto's and lineto's we are past the hints */
		if (strstr(line, "moveto") != NULL ||
			strstr(line, "lineto") != NULL ||
			strstr(line, "curveto") != NULL) {
/*			initial = 0;	*/
			return 0;	/* hit moveto */
		}
		else if (strstr(line, "stem") != NULL &&
			(hadjust != 0 || vadjust != 0)) {	/* only if need adjustment */
			if (strstr(line, "hstem") != NULL &&
				strstr(line, "hstem3") == NULL &&
					strstr(line, "div") == NULL) {
				if (sscanf(line, "%d %d hstem", &start, &width) == 2) {
					sprintf(line, "%d %d hstem\n", start + hadjust, width);
				}
				else fputs(line, errout);
			}
			if (strstr(line, "vstem") != NULL &&
				strstr(line, "vstem3") == NULL &&
					strstr(line, "div") == NULL) {
				if (sscanf(line, "%d %d vstem", &start, &width) == 2) {
					sprintf(line, "%d %d vstem\n", start + vadjust, width);
				}
				else fputs(line, errout);
			}			
			fputs(line, fp_out);
		}
/*		maybe need to weed out new `n 4 callsubr' calls also ? */
		else if (strstr(line, "callsubr") != NULL && recurseexp != 0) {
			if ((n = sscanf(line, "%d %d %d %d", &na, &nb, &nc, &nd)) == 1) {
				filepos = ftell(fp_in);	/* remember where we were */
/*				if (na < 0 || na > MAXSUBRS) */
				if (na < 0 || na > maxsubrs) {
					fprintf(errout, 
						"ERROR: Asked to go to non-existent Subr %d\n", na);
					return 0;
				}
				subrposna = subrpos[na];
				if (subrposna == 0) {
					fprintf(errout, 
						"ERROR: Asked to go to non-existent Subr %d\n", na);
					return 0;
				}
				if (fseek(fp_in, subrposna, SEEK_SET) != 0) {
					fprintf(errout, "ERROR: seek to %ld failed\n", subrposna);
					return 0;
				}
				initial = expandhints(fp_out, fp_in, hmove, vmove, 
					vadjust, hadjust, initial);
				if (initial == 0) {
					return 0;			/* end of hints found */
				}
				fseek(fp_in, filepos, SEEK_SET);	/* back to where we were */
			}
			else if (n == 2 && nb == 4) ;	/* <n> 4 callsubr hint replace */
/*			else if (n == 4 && nd == 0) ; *//* FlexProc call probably */
			else fputs(line, fp_out);	/* not simple Subr call - just copy */
		}
		else fputs(line, fp_out);		/* not moveto, stem, callsubr */
	}
	if (bAbort != 0) abortjob();	/* in case stuck in loop */
	return initial;					/* have not hit end of hints yet */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void checkheap() {
	int n;
	if (traceflag) printf("Checking heap\n");
	n = _heapchk();
	switch(n) {
		case _HEAPBADBEGIN:
			printf("Heap has bad start\n");
			break;
		case _HEAPBADNODE:
			printf("Heap has a bad node\n");
			break;
		case _HEAPEMPTY:
			if (traceflag) printf("Heap is empty\n");
			break;
		case _HEAPOK:
			if (traceflag) printf("Heap is OK\n");
			break;
	}
}

int checkencoding(void) {			/* 1995/May/8 */
	int k, flag=0;
	char *s;
	
	if (traceflag) printf("Checking encoding\n");
	for (k = 0; k < MAXCHRS; k++) {
/*		if (strcmp(charnames[k], "") == 0) continue; */
		if (charnames[k] == NULL) continue;
		s = charnames[k];
/*		if (s == NULL) {
			printf ("NULL CHAR NAME (%d)\n", k);
			flag++;
			continue;
		} */
		while (*s != '\0' && *s >= 32 && *s <= 127) s++;
		if (*s != '\0') {
			printf ("BAD CHAR NAME (%d): %s\n", k, charnames[k]);
			flag++;
		}
	}
	return flag;
}

void freeencoding(void) {			/* 1995/May/8 */
	int k;
	if (heapflag) checkheap();
/*	if (heapflag) checkencoding(); */
	if (traceflag) printf("Freeing encoding\n");
	for (k = 0; k < MAXCHRS; k++) {
/*		if (strcmp(charnames[k], "") != 0)  */
		if (charnames[k] != NULL) {
			free(charnames[k]);
/*			charnames[k] = ""; */
			charnames[k] = NULL;
		}
	}
}

void showencoding(void) {				/* debugging only */
	int k;
	if (heapflag) checkheap();
/*	if (heapflag) checkencoding(); */
	printf("ENCODING:\n");
	for (k = 0; k < MAXCHRS; k++) {
/*		if (strcmp(charnames[k], "") != 0)  */
		if (charnames[k] != NULL) {
			printf("%d\t%s\n", k, charnames[k]);
		}
	}
}


int makesafe(FILE *fp_out, FILE *fp_pln) {
	char buffer[MAXLINE];
/*	char rdsubr[16]; */
	char charname[CHARNAME_MAX];
	int k, n, m;
/*	int na, nb, nc, nd; */
/*	int flag; */
	int nprivy;
	char *s=NULL;
	long uniqueid1=0, uniqueid2=0; 
/*	long len; */
	long filepos, fileloc;
	int skipflag = 0;
	int asb, adx, ady, bchar, achar;
	int dadx, dady;
/*	int nsubrscount; */
	int subrnum, subrrepl;
	int accentflag, initflag;
/*	int vs, dv, hs, dh, rx, ry; */
	char *acharname, *bcharname, *scharname;		/* 95/May/8 */
	int asubr, bsubr, ksubr;						/* 95/May/8 */
	
	if (traceflag != 0) printf("Starting the Processing\n");

/*	check whether it is safe to use calls to Subrs for accents themselves */
	if (callaccents != 0) {
/*		for (k = 0; k < MAXCHRS; k++) { */
		for (k = 0; k < MAXADJUST; k++) {
			if (characcent[k] != 0 && charusesrep[k] != 0) {
/* No it is not, since accent code uses hint replacement */
				if (verboseflag != 0)
	printf("NOTE: Forcing use of original accent code for accents\n");
				callaccents = 0;
				break;
			}
		}
	}

	hintstrip = 0;

	if (verboseflag != 0 && nsubrs > 0)
		printf("Found %d Subrs in original font file\n", nsubrs);
	nsubrsadd = 0;	/* figure out how many Subrs have to be added */

	noverlap = 0;
/*	for (n = 0; n < MAXCHRS; n++) { */		/* 1993/April/15 */
	for (n = 0; n < MAXADJUST; n++) {		/* 1993/April/15 */
		if (charbase[n] > 0 && characcent[n] > 0) {
			if (n < MAXCHRS) scharname = standardencoding[n];
			else scharname = charnames[n-MAXCHRS];
			fprintf(errout,
				"ERROR: using %s as both base and accent (use -s?)\n",
					scharname);			/*	standardencoding[n]);*/
			noverlap++;
			callaccents = 0;			/* safe to do here ??? */
		}
	}
/*	nsubrscount = 0; */
	nsubrsbase = 0;
/*	for (n = 0; n < MAXCHRS; n++) if (charbase[n] > 0) nsubrsbase++;  */
	for (n = 0; n < MAXADJUST; n++) if (charbase[n] > 0) nsubrsbase++; 
	if (noverlap > 0) nsubrsbase -= noverlap;	/* 1993/April/15 */
	if (verboseflag != 0 && nsubrsbase != 0) 
		printf("Adding %d Subrs for base characters code + hints\n", 
			nsubrsbase);
	nsubrsadd += nsubrsbase;
/*	don't count if copied for composites */
/*	but count if using replacement subrs */
	nsubrsaccent = 0;
	if (copyaccents == 0 || replacesubr != 0) {	
/*		for (n = 0; n < MAXCHRS; n++) if (characcent[n] > 0) nsubrsaccent++; */
		for (n = 0; n < MAXADJUST; n++) if (characcent[n] > 0) nsubrsaccent++; 
		if (verboseflag != 0 && nsubrsaccent != 0) 
			printf("Adding %d Subrs for accent characters code\n", 
				nsubrsaccent);
		nsubrsadd += nsubrsaccent;
	}
/*	make space also for hint replacement subroutines */

/*	if (traceflag != 0) 
	printf("Adding %d Subrs for base characters and accents\n", nsubrsadd); */

	if (nsubrsadd == 0) {
	fprintf(errout, "WARNING: no `seac' calls found - no need to modify\n");
	fprintf(errout, "         perhaps the PFA file has already been modified?\n");
		return -1;
	}

	if (replacesubr != 0) {
		if (verboseflag != 0 && seaccount != 0) 
		printf("Adding %d Subrs for composite character hint replacement\n", 
				seaccount);
		nsubrsadd += seaccount;
	}
	
	if (callaccents != 0 && replacesubr != 0) {					/* new */
		if (verboseflag != 0 && nsubrsaccent != 0) 
			printf("Adding %d Subrs for accent character hints\n", 
				nsubrsaccent);
		nsubrsadd += nsubrsaccent;
	}

/*	if (traceflag != 0) {
		fseek (fp_pln, 0, SEEK_END);	len = ftell(fp_pln);
		printf("FILE %ld\n", len);
		rewind(fp_pln);
	}  */

	if (traceflag != 0) printf("Scan up to eexec file\n");

/*	currentdict end */
/*	currentfile eexec */

/*  scan up to eexec encrypted part in FILE */
	while ((n = getline(fp_pln, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
		if (strstr(line, "currentdict end") != NULL) /* 1993/June/8 */
			fputs(
"% Composites in this font made safe from ATM by SAFESEAC from Y&Y, Inc.\n",
			fp_out);
		if ((s = strstr(line, "/UniqueID")) != NULL) {
			if (sscanf(s, "/UniqueID %ld", &uniqueid1) > 0) {
/*				printf("UNIQUEID1 %ld\n", uniqueid); */	/* debugging */
			}
		}
/*		if (traceflag != 0) fputs(line, stdout); */
		fputs(line, fp_out);
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
	fputs(line, fp_out);
	if (traceflag != 0) fputs(line, stdout);

	if (traceflag != 0) printf("Scan up to Subrs file\n");
	while ((n = getline(fp_pln, line)) != 0 && 
		(s = strstr(line, "/Subrs")) == NULL) {
/* flush the following ??? */
		if (needothersubrs != 0 &&
			(s = strstr(line, "/Private")) != NULL) {
			sscanf(s, "/Private %d%n", &nprivy, &n);
			strcpy(buffer, s+n);
			sprintf(s, "/Private %d%s", nprivy+1, buffer);
		}
/* flush the following ??? */
		if ((s = strstr(line, "/UniqueID")) != NULL) {
			sscanf(s, "/UniqueID %ld%n", &uniqueid2, &n);
/*			printf("UNIQUEID2 %ld\n", uniqueid2); */	/* debug */
		}
		fputs(line, fp_out);
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}

	if (n == 0) {
		fprintf(errout, "Premature EOF 0\n");
		return -1;
	}

/*	space for dummy hint replacement subr */
	if (dummyswitch != 0 && replacesubr == 0) nsubrsadd++;	

/*	modify /Subrs line - up the total count nsubrs1 + nsubrs2 */
	if (nsubrsadd > 0) {
		if (traceflag != 0) printf("old: %s", line);
		sscanf (s, "/Subrs %d%n", &nsubrs, &n);
		strcpy(buffer, s+n);
		sprintf(s, "/Subrs %d%s", nsubrs+nsubrsadd, buffer);
		if (verboseflag != 0) 
			printf("Total number of Subrs in output file is %d\n", 
				nsubrs+nsubrsadd);
	}
	fputs(line, fp_out); 
/*	if (traceflag != 0) fputs(line, stdout); */
	if (traceflag != 0) printf("new: %s", line);

	if (traceflag != 0) printf("Copy Subrs from file\n");

	nsubrsout = 0;		/* count how many written as a sanity check */

	if (s == NULL) fprintf(errout, "s == NULL\n");
/*	Now copy across SUBRS from FILE */
	while ((n = getline(fp_pln, line)) != 0 && 
		(s = strstr(line, "/CharStrings")) == NULL) {
		if (strncmp(line, "dup ", 4) == 0) {
			nsubrsout++;			/* 1993/April/7 */
/*			if (traceflag != 0) fputs(line, stdout); */
		}
		s = line;					/* deal with leading white space */
		while (*s == ' ') s++;
		m = strcspn(s, " \n\r\t\f");
		if (strstr(s, "noaccess def") != NULL) {	/* 1993/Jan/4 */
			if (traceflag != 0) printf("BREAKING OUT ON %s", line);
			break;
		}
		if (m == (int) strlen(ndsynom) && 
			strncmp(s, ndsynom, m) == 0) {
			if (traceflag != 0) printf("BREAKING OUT ON %s", line);
			break;
		}
		fputs(line, fp_out);
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
	if (traceflag != 0) printf("Finished with existing Subrs\n");

	strcpy(buffer, line);	/* save terminating line from FILE */
	if (traceflag != 0) printf("SAVED LINE: %s", buffer);

/*	putc('\n', fp_out); */ 		/* separator between SUBRS */

	filepos = ftell(fp_pln);	/* remember where we where */
	
	if (traceflag != 0) {
		fseek(fp_pln, filepos, SEEK_SET);
		getline(fp_pln, line);
		printf("Next line should be: %s", line);
	} 

/*	insert new `Subrs' here !  */ 
/*	always for base characters */ /* also for accents if copyaccents == 0 */
	if (traceflag != 0) printf("Insert new base and accent Subrs here\n");
	subrnum = nsubrs;
/*	for (k = 0; k < MAXCHRS; k++) { */
	for (k = 0; k < MAXADJUST; k++) {
		if (charbase[k] != 0 || 
			((copyaccents == 0 || replacesubr != 0) && characcent[k] != 0)) {
			if (charposition[k] == 0) {		/* SANITY CHECK */
				if (k < MAXCHRS) scharname = standardencoding[k];
				else scharname = charnames[k-MAXCHRS];
				fprintf(errout, "ERROR: Missing base or accent %d `%s'\n",
						k, scharname); /* standardencoding[k]); */
				badsubr++;
				continue;
			}
			if (charbase[k] != 0) accentflag = 0;
			else accentflag = 1;		/* non-zero if processing accent */
			putc('\n', fp_out); 		/* separator between SUBRS */
			if (traceflag != 0) {			/* debug */
				if (k < MAXCHRS) scharname = standardencoding[k];
				else scharname = charnames[k-MAXCHRS];
				printf("Subr %d for char: `%s' (%d)\n", subrnum,
					   scharname, k); /* standardencoding[k]); */
				if (strcmp(scharname, "") == 0) showencoding();
			}
			fprintf(fp_out, "dup %d 69 %s\n", subrnum, rdsynom);
			nsubrsout++;				/* 1993/April/7 */
			charsubrs[k] = subrnum;		/* remember which Subr this was */
/*	already checked charposition[k] above */
			fseek(fp_pln, charposition[k], SEEK_SET);
			if (initmoved != 0) initflag = 0;
			else initflag = 1;					/* only do if asked for */
			if (accentflag == 0) initflag = 1;	/* only do this for accents */
			if (charinix[k] == UNKNOWN) initflag = 1; /* only if known */
			if (replacesubr != 0 && accentflag != 0) {
/* use expand subr - except: suppress initial moveto and flush hints */
/* and insert dotsection if desired where intial moveto would be */
/*				if (wantdotsection != 0)  fputs("dotsection\n", fp_out); */
/*				expandsubr(fp_out, fp_pln, 0, 0, 0, 0); */
				kcurrent = k;		/* for error message if needed */
				expandsubr(fp_out, fp_pln, 0, 0, 0, 0, 1);
				if (wantdotsection != 0)  fputs("dotsection\n", fp_out);
			}
			else {
			while ((n = getline(fp_pln, line)) != 0 &&
				(s = strstr(line, ndsynom)) == NULL) {
/*				if (strstr(line, "endchar") != NULL) break; */
				if (strstr(line, "sbw") != NULL) continue;
				if (strstr(line, "endchar") != NULL) continue;
				if (initflag == 0 && strstr(line, "moveto") != NULL) {
					initflag++;
					continue;				/* just drop first moveto */
				}
				fputs(line, fp_out);
/*				if (traceflag != 0) fputs(line, stdout);  */
				if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
			}
				}
			fputs("return\n", fp_out);
			fprintf(fp_out, "%s\n", npsynom);
			subrnum++;
		}
	}

	subrrepl = subrnum;				/* remember starting replacement subr */

	fseek(fp_pln, filepos, SEEK_SET);	/* return to previous position */

	if (replacesubr != 0) {		/* make up hint replacement Subrs */

		if (traceflag != 0) printf("Insert new hint replacement Subrs here\n");
/*	scan through looking for `seac' - return to proper place in file later */
/*	first find CharStrings */
		strcpy(line, buffer);					/* 1993/Jan/4 */
		while ((s = strstr(line, "/CharStrings")) == NULL) {
			if ((n = getline(fp_pln, line)) == 0) break;
			if (traceflag != 0) fputs(line, stdout);
			if (bAbort != 0) abortjob();
		}
		if (n == 0) {
			fprintf(errout, "Premature EOF 1\n");
			return -1;
		}
/* now scan over CharStrings to pick out the seac calls - copy nothing */
		for (;;) {
			n = getline(fp_pln, line);
			if (n <= 0) break;
			if (strchr(line, '/') == NULL &&
				strstr(line, "endchar") == NULL && 
					strstr(line, "end") != NULL) break;
			if (strstr(line, "readonly put") != NULL) break;
			if (strstr(line, "/FontName") != NULL) break;
/* find beginning of a CharString */
			if ((s = strchr(line, '/')) != NULL) {
				sscanf(s, "/%s ", charname); 
/* see whether this CharString has been moved to a Subr !!! */
/* base character have moved; accents have moved only if copyaccents == 0 */
				k = lookupstandard(charname); 
				if (relaxseac) {
					if (k < 0) {
						k = lookupencoding(charname);
						if (k >= 0) k += MAXCHRS;
					}
				}
				if (k >= 0 && 
					(charbase[k] > 0 || (copyaccents == 0 && characcent[k] > 0))) {
					if (charbase[k] != 0) accentflag = 0;
					else accentflag = 1;		/* non-zero if processing accent */
					getline(fp_pln, line);	/* hsbw */
					if (strstr(line, "sbw") == NULL) fputs(line, errout);
					if (initmoved != 0 && accentflag != 0 &&
						charinix[k] != UNKNOWN) ;
					skipflag++;
				}
				else skipflag = 0;
				if (traceflag == 0) {
					if (dotsflag != 0) 
						putc(':', stdout);
				}
			}
			if (skipflag == 0) {
/* see whether this is an `seac' line that needs tracing !!! */
				if (strstr(line, "seac") != NULL) {
					if (sscanf(line, "%d %d %d %d %d seac", 
						&asb, &adx, &ady, &bchar, &achar) == 5) {
						if (traceflag != 0) {
							printf("In char `%s', was: %d %d %d %d %d seac ", 
								charname, asb, adx, ady, bchar, achar);
							printf(" hint rep subr %d\n", subrnum);
						}
/* adjust for movement within base character */
						dadx = adx - chardx[bchar]; 
						dady = ady - chardy[bchar]; 
/* adjust for side-bearings of base character and accent character ??? */
/*						dadx = dadx - charside[bchar] + charside[achar];  */ 
/*						dadx = dadx - charside[bchar]; */ /* ??? */
/* set up adjustment of hint shift offset ??? */
/*						adx = adx + charside[achar] - charside[bchar]; */ 
/* first the version where the accent is treated as a subroutine */
/* shouldn't happen ! */
						if (copyaccents == 0 && replacesubr == 0) {
							if (initmoved == 0 || charinix[achar] == UNKNOWN) {
								if (wantclosed != 0 && wantzeroln != 0) {
									dadx--; dady--; 
								}
								if (wantclosed != 0) {
									if (wantzeroln != 0) {
									}
								}
							}		/* end of mouse dropping method */
							else {
/* in this case we suppressed the first moveto in the accent, so do it here */
							}
							if (wantdotsection != 0) ;
						}
/* this is the version where the accent is copied in and modified */
/* this avoids the two `rmoveto's in a row and allow adjustment of hints */
/* well, at least of hints that appear directly in accent subroutine ! */
/* maybe flush, or expand out Subr calls recursively ??? */
						else {
							fileloc = ftell(fp_pln); /* remember where */
/*	go to accent character CharString *//*	check charposition != 0 ? */
							if (charposition[achar] == 0) {	/* SANITY CHECK */
								if (achar < MAXCHRS)
									acharname = standardencoding[achar];
								else acharname = charnames[achar-MAXCHRS];
								fprintf(errout,
										"ERROR:	Missing accent %d `%s'\n", 
										achar, acharname);
/*									achar, standardencoding[achar]); */
								badsubr++;
								continue;
							}
							fseek (fp_pln, charposition[achar], SEEK_SET);
							putc('\n', fp_out);  /* separator between SUBRS */
							fprintf(fp_out, "dup %d 69 %s\n", subrnum, rdsynom);
							nsubrsout++;					/* 1993/April/7 */
							if (expandhints(fp_out, fp_pln, dadx, dady, 
								adx, ady, 1) != 0) {
								fprintf(errout, 
									"WARNING: unable to create hint replacement\n");
							}
							fprintf(fp_out, "return\n");
							fprintf(fp_out, "%s\n", npsynom); /* ??? */
/* now go back to where we were */
							fseek (fp_pln, fileloc, SEEK_SET);
						}
						subrnum++;
					}		/* end of seac scanf */
/*					else fputs(line, errout);  */
					else fprintf(errout, "BAD SEAC: %s", line); 
				}		/* end of seac strstr */
/*			else fputs(line, fp_out); */ 
			}
			if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
		}	/* end of for loop */
	}	/* end of replacesubr if */

/*	if (dotsflag != 0) putc('\n', stdout); */
	if (dotsflag != 0 && traceflag == 0) 
		putc('\n', stdout);

	if (dummyswitch != 0 && replacesubr == 0) {		/* new, insert dummy hint switching */
		putc('\n', fp_out);
		fprintf(fp_out, "dup %d 69 %s\n", subrnum, rdsynom);
		nsubrsout++;					/* 1993/April/7 */
		fprintf(fp_out, "return\n");
		fprintf(fp_out, "%s\n", npsynom);
		dummysubr = subrnum;
		subrnum++;
	}

/* insert hint replacement for accented characters */
	
/* assume subrnum set up at this point to next Subr slot */

	if (callaccents != 0 && replacesubr != 0) {
	if (traceflag != 0) printf("Inserting accented character hint Subrs\n");
/*	for (k = 0; k < MAXCHRS; k++) { */
	for (k = 0; k < MAXADJUST; k++) {
			if (characcent[k] != 0) {
				putc('\n', fp_out); 		/* separator between SUBRS */
				if (traceflag != 0)	{		 /* debug */
					if (k < MAXCHRS) scharname = standardencoding[k];
					else scharname = charnames[k-MAXCHRS];
					printf("Subr %d for hints in accent: `%s' (%d)\n",
						   subrnum, scharname, k); /* standardencoding[k]); */
				}
				fprintf(fp_out, "dup %d 69 %s\n", subrnum, rdsynom);
				nsubrsout++;					/* 1993/April/7 */
				charreplace[k] = subrnum;		/* remember which Subr was */
/*	check charposition != 0 ? */
				if (charposition[k] == 0) {	/* SANITY CHECK */
					if (k < MAXCHRS) acharname = standardencoding[k];
					else acharname = charnames[k-MAXCHRS];
					fprintf(errout, "ERROR: Missing accent %d `%s'\n", k,
							acharname); /* standardencoding[k]); */
					continue;
				}
				fseek(fp_pln, charposition[k], SEEK_SET);
				expandhints(fp_out, fp_pln, 0, 0, 0, 0, 1);
				fprintf(fp_out, "return\n");
				fprintf(fp_out, "%s\n", npsynom); /* ??? */
				subrnum++;
			}
		}
	}

	subrnum = subrrepl;				/* back to starting replacement subr */

/*	fputs(buffer, fp_out);	 */	/* line saved up from FILE */
/*	if (traceflag != 0) fputs(buffer, stdout); */	/* 1993/Jan/4 */

	fseek(fp_pln, filepos, SEEK_SET);	/* return to previous position */

	if (traceflag != 0) printf("Scan up to CharStrings in file\n");
/*	copy across from FILE up to /CharStrings line */
	strcpy(line, buffer);		/* 1993/Jan/4 */
	while ((s = strstr(line, "/CharStrings")) == NULL) {
		fputs(line, fp_out);
		if ((n = getline(fp_pln, line)) == 0) break;
/*		if (traceflag != 0) fputs(line, stdout);	 */
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
	if (n == 0) {
		fprintf(errout, "Premature EOF 2\n");
		return -1;
	}	

	if (nsubrsout != nsubrs+nsubrsadd) {
		fprintf(errout, "WARNING: expected %d Subrs, wrote %d Subrs\n",
			nsubrs+nsubrsadd, nsubrsout);
	}

	if (traceflag != 0) printf("Found CharStrings in file\n");

/*	do NOT modify the /CharStrings line - need JUST as many */
/*	if (ncharstrings2 + spaceneeded > 0) {  
		sscanf(s, "/CharStrings %d%n", &ncharstrings, &m);
		strcpy(buffer, s + m);
		sprintf(s, "/CharStrings %d%s", 
			ncharstrings, buffer);
	} */

	fputs(line, fp_out);
/*	if (traceflag != 0) fputs(line, stdout);  */

	if (verboseflag != 0) printf("Now processing CharStrings\n");

/*	if (traceflag != 0) printf("Copy CharStrings from file\n"); */
/*	copy across all of the CharStrings from FILE */
	skipflag = 0;
	for (;;) {
		n = getline(fp_pln, line);
		if (n <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;

		if ((s = strchr(line, '/')) != NULL) {
			sscanf(s, "/%s ", charname); 
/* see whether this CharString has been moved to a Subr !!! */
/* base character have moved; accents have moved only if copyaccents == 0 */
			k = lookupstandard(charname); 
			if (relaxseac) {
				if (k < 0) {
					k = lookupencoding(charname);
					if (k >= 0) k += MAXCHRS;
				}
			}
			if (k >= 0 && 
/* see whether this is a base or accent that has been moved to Subrs */
				(charbase[k] > 0 || 
					(copyaccents == 0 && characcent[k] > 0) ||
					(callaccents != 0 && characcent[k] > 0)	)) {
				if (charbase[k] != 0) accentflag = 0;
				else accentflag = 1;		/* non-zero if processing accent */
				fputs(line, fp_out);	/* character start */
				getline(fp_pln, line);
				fputs(line, fp_out);	/* hsbw */
				if (strstr(line, "sbw") == NULL) fputs(line, errout);
/* now stick in the inital moveto in case of an accent */
				if (accentflag != 0) {
					if (callaccents != 0)	/* insert call to hint Subr */
						fprintf(fp_out, "%d callsubr\n", charreplace[k]);
					if (initmoved != 0) {
						if (charinix[k] != UNKNOWN)
							fprintf(fp_out, "%d %d rmoveto\n", 
								charinix[k], chariniy[k]);
						else {
							if (k < MAXCHRS)
								scharname = standardencoding[k];
							else scharname = charnames[k-MAXCHRS];
							fprintf(errout, 
									"ERROR: no initial moveto in %s\n",
									scharname); /* standardencoding[k]); */
						}
					}
				}
/*	then call the Subr for this base or accent character */
				ksubr = charsubrs[k];			/* 95/May/8 */
				if (ksubr <= 0) {
					fprintf(errout, "ERROR: no Subr for %d\n", k);
					ksubr = 0;
					badsubr++;
				}
/*				fprintf(fp_out, "%d callsubr\n", charsubrs[k]); */
				fprintf(fp_out, "%d callsubr\n", ksubr);
				fputs("endchar\n", fp_out);
				fprintf(fp_out, "%s\n", ndsynom);
				putc('\n', fp_out);
				skipflag++;			/* just skip over the rest ! */
				if (traceflag != 0) {
					if (k < MAXCHRS) scharname = standardencoding[k];
					else scharname = charnames[k-MAXCHRS];
					if (accentflag != 0 && callaccents != 0) {
						printf("In char `%s' call %d and %d\n",
							   scharname, charreplace[k], charsubrs[k]);
					}
					else printf("In char `%s' call %d\n",
							   scharname, charsubrs[k]);
				}
			}
			else skipflag = 0;
			if (traceflag == 0) {
				if (dotsflag != 0) putc('*', stdout);
			}
		}
		if (skipflag == 0) {
/* see whether this is an `seac' line that needs fixing !!! */
		if (strstr(line, "seac") != NULL) {
			if (sscanf(line, "%d %d %d %d %d seac", 
				&asb, &adx, &ady, &bchar, &achar) == 5) {
				if (traceflag != 0)
					printf("In char `%s', was: %d %d %d %d %d seac ", 
						charname, asb, adx, ady, bchar, achar);
/*				fprintf(fp_out, "%d callsubr\n", charsubrs[bchar]); */
				asubr = charsubrs[achar];			/* 1995/May/8 */
				bsubr = charsubrs[bchar];			/* 1995/May/8 */
				if (bsubr <= 0) {
					if (bchar >= 0 && bchar < MAXCHRS)
						bcharname = standardencoding[bchar];
					else if (bchar < MAXADJUST)
						bcharname = charnames[bchar-MAXCHRS];
					else bcharname = "";
					fprintf(errout, "ERROR: no Subr for `base' %d (%s)\n",
							bchar, bcharname);
					bsubr = 0;
					badsubr++;
				}
				if (asubr <= 0) {
					if (achar >= 0 && achar < MAXCHRS)
						acharname = standardencoding[achar];
					else if (achar < MAXADJUST)
						acharname = charnames[achar-MAXCHRS];
					else acharname = "";
					fprintf(errout, "ERROR: no Subr for `accent' %d (%s)\n",
							achar, acharname);
					asubr = 0;
					badsubr++;
				}
				fprintf(fp_out, "%d callsubr\n", bsubr);
				if (traceflag != 0)
					printf(" now calls %d %d %d\n", bsubr, subrnum, asubr);
/*				if (wantdotsection != 0) fputs("dotsection\n", fp_out); */
/* adjust for movement within base character */
				dadx = adx - chardx[bchar]; 
				dady = ady - chardy[bchar]; 
/* adjust for side-bearings of base character and accent character ??? */
/*				dadx = dadx - charside[bchar] + charside[achar];  */ /* ??? */
/*				dadx = dadx - charside[bchar]; */ /* ??? */
/* set up adjustment of hint shift offset ??? */
/*				adx = adx + charside[achar] - charside[bchar]; */ /* ??? */
/* first the version where the accent is treated as a subroutine */
/*				if (copyaccents == 0) { */
				if (copyaccents == 0 && replacesubr == 0) { 
					if (initmoved == 0 || charinix[achar] == UNKNOWN) {
/* this is the mouse dropping method */
/* ATM problem because two rmoveto's appear in sequence ? */
					if (wantclosed != 0 && wantzeroln != 0) {
						dadx--; dady--; /* compensate for mouse dropping */
					}
					fprintf(fp_out, "%d %d rmoveto\n", dadx, dady);
					if (wantclosed != 0) {
						if (wantzeroln != 0) {		/* mouse dropping */
							fputs("1 hlineto\n", fp_out);
							fputs("1 vlineto\n", fp_out);
						}
						fputs("closepath\n", fp_out);
					}
					}		/* end of mouse dropping method */
					else {
/* in this case we suppressed the first moveto in the accent, so do it here */
						fprintf(fp_out, "%d %d rmoveto\n", 
							dadx + charinix[achar], dady + chariniy[achar]);
					}
					if (wantdotsection != 0) fputs("dotsection\n", fp_out);
/*					fprintf(fp_out, "%d callsubr\n", charsubrs[achar]); */
					asubr = charsubrs[achar];				/* 95/May/8 */
					if (asubr <= 0) {
						if (achar >= 0 && achar < MAXCHRS)
							acharname = standardencoding[achar];
						else if (achar < MAXADJUST)
							acharname = charnames[achar-MAXCHRS];
						else acharname = "";
						fprintf(errout, "ERROR: no Subr for `accent' %d (%s)\n",
								achar, acharname);
						asubr = 0;
						badsubr++;
					}
					fprintf(fp_out, "%d callsubr\n", asubr);
				}
				else if (replacesubr != 0) {
/* call hint replacement code for this accent first */
					if (switchtrick == 0) {
						fprintf(fp_out, "%d 1 3 callothersubr\n", subrnum);
						fputs("pop\n", fp_out);
						fputs("callsubr\n", fp_out);
					}
/* OK, use abbreviated hint switching trick ! */
					else fprintf(fp_out, "%d 4 callsubr\n", subrnum);
					subrnum++;
/* in this case we suppressed the first moveto in the accent, so do it here */
					fprintf(fp_out, "%d %d rmoveto\n", 
						dadx + charinix[achar], dady + chariniy[achar]);
/* then call subr for accent */
					asubr = charsubrs[achar];				/* 95/May/8 */
					if (asubr <= 0) {
						if (achar >= 0 && achar < MAXCHRS)
							acharname = standardencoding[achar];
						else if (achar < MAXADJUST)
							acharname = charnames[achar-MAXCHRS];
						else acharname = "";
						fprintf(errout, "ERROR: no Subr for `accent' %d (%s)\n",
								achar, acharname);
						asubr = 0;
						badsubr++;
					}
/*					fprintf(fp_out, "%d callsubr\n", charsubrs[achar]); */
					fprintf(fp_out, "%d callsubr\n", asubr);
/*					fputs("endchar\n", fp_out); */
				}
/* this is the version where the accent is copied in and modified */
/* this avoids the two `rmoveto's in a row and allow adjustment of hints */
/* well, at least of hints that appear directly in accent subroutine ! */
/* maybe flush, or expand out Subr calls recursively ??? */
				else {
					filepos = ftell(fp_pln); /* remember where we were */
/*	go to accent character CharString *//*	check charposition != 0 ? */
					if (charposition[achar] == 0) {	/* SANITY CHECK */
						if (achar < MAXCHRS)
							acharname = standardencoding[achar];
						else acharname = charnames[achar-MAXCHRS];
						fprintf(errout, "ERROR: Missing accent %d `%s'\n",
								achar, acharname); /* standardencoding[achar]); */
						continue; 
					}
					fseek (fp_pln, charposition[achar], SEEK_SET);

/*					if (wantdotsection != 0) fputs("dotsection\n", fp_out); */

					if (dummyswitch != 0 && replacesubr == 0) {
						fprintf(fp_out, "%d 1 3 callothersubr\n", dummysubr);
						fputs("pop\n", fp_out);
/* now do positioning for accent - first moveto in accent suppressed */
						fprintf(fp_out, "%d %d rmoveto\n", 
							dadx + charinix[achar], dady + chariniy[achar]);
						fputs("callsubr\n", fp_out);
					}
/* expandsubrs uses condition hmove == 0 && vmove == 0 after first moveto */
/*					if (dadx == 0 && dady == 0) {
					fprintf(errout, "WARNING: going in with zero offset\n");
						dadx = 1; 
					} */
					if (expandsubr(fp_out, fp_pln, dadx, dady, 
						adx, ady, 1) != 0) {
						fprintf(errout, 
							"WARNING: unable to adjust accent positions\n");
					}

/* now go back to where we were */
					fseek (fp_pln, filepos, SEEK_SET);
				}
				if (wantdotsection != 0 && replacesubr == 0)
					 fputs("dotsection\n", fp_out);
				fprintf(fp_out, "endchar\n");
			}
/*			else fputs(line, errout); */
			else fprintf(errout, "BAD SEAC: %s", line);
		}
		else fputs(line, fp_out); /* NOT A '/' line */
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
/*	putc('\n', fp_out); */	/* separator between two sets of CharStrings */
	strcpy(buffer, line);	/* save line from FILE */

/*	fputs(buffer, fp_out); */	/* saved line from FILE */

	if (traceflag != 0) printf("Copy tail end from file\n");
/*	copy the tail across */
	if (n != 0) fputs(line, fp_out);
	while ((n = getline(fp_pln, line)) != 0)	fputs(line, fp_out);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	if (quietflag != 0) exit(1);
/*	printf("%s  [-{v}{f}{s}{E}{S}{a}{h}{c}]  <pfa-file-1> [<pfa-file-2>...]\n", s); */
	printf("%s  [-{v}{f}{S}{E}{s}]  <pfa-file-1>  [<pfa-file-2>...]\n", s);
	if (detailflag == 0) exit(1);
	printf("\tv verbose mode\n");
	printf("\tf force processing, even if apparently does not require processing\n");
	printf("\tS allow base and accent not in Adobe StandardEncoding\n");
	printf("\tE do not move `endchar' from Subrs to CharStrings\n");
	printf("\ts use Subrs for accents (rather than original code)\n");
/*	printf("\ta wrap accent code in `dotsection' commands (NOT recommended)\n"); */
/*	printf("\th do not create new hint replacement Subrs (NOT recommended)\n");  */
/*	printf("\tc generate compact form - without hint adjustment (NOT recommended)\n"); */
/*	printf("\tc do not adjust accent hints (produce compact form)\n"); */
/*	printf("\ti do not move first moveto outside accent Subr\n"); */
/*	printf("\tr do not use closepath after rmoveto for accent\n"); */
/*	printf("\tz do not use lineto's to make mouse dropping\n");	*/
/*	printf("\tc do not copy accent code for every accented character\n"); */
/*	printf("\ts do not expand Subrs recursively for accents\n"); */
/*	printf("\tf do not insert dummy hint switching code\n"); */
	exit(1);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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
/*    (void) mktemp(fname); */	/* replace template by unique string */
    (void) _mktemp(fname);		/* replace template by unique string */
	forceexten(fname, str);		/* force appropriate extension */
}

FILE *fp_dec=NULL;
FILE *fp_pln=NULL;
FILE *fp_adj=NULL;

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
		for (;;) {
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
		(void) fclose(fp_dec);						/* close output */
		wipefile (fn_dec);
		(void) remove(fn_dec);	/* remove bogus file */
	}
	if (fp_pln != NULL) {
		(void) fclose(fp_pln);				/* close output */
		wipefile (fn_pln);
		(void) remove(fn_pln); /* remove bogus file */
	}
	if (fp_adj != NULL) {
		(void) fclose(fp_adj);				/* close output */
		wipefile (fn_adj);
		(void) remove(fn_adj); /* remove bogus file */
	}
}

void abortjob() {
	fprintf(errout, "\nUser Interrupt - Exiting\n"); 
	cleanup();
	if (renameflag != 0) rename(fn_in, fn_bak);
	exit(3);
}

/*	problem:        wants void (__cdecl *)(int); */
/*	gets #define SIG_IGN (void (__cdecl *)(int))1 */

#undef SIG_IGN
#define SIG_IGN (void (__cdecl *)(int))1L

void ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
/*	fprintf(errout, "\nUser Interrupt - Exiting\n");  */
/*	cleanup(); */
/*	if (renameflag != 0) rename(fn_in, fn_bak); */
	if (bAbort++ >= 15)	exit(3);		/* emergency exit */
	(void) signal(SIGINT, ctrlbreak);
}

int decodeflag (int c) {
	switch(c) { 
/*		case 'v': verboseflag = 1; dotsflag = 1; return 0; */
		case 'v': verboseflag++; return 0;
		case 't': traceflag++; return 0;
		case '?': detailflag++; return 0;
		case 'h': replacesubr = (1 - replacesubr); return 0;
		case 'a': wantdotsection = (1 - wantdotsection); return 0;
		case 's': callaccentsinit = (1 - callaccentsinit); return 0;
		case 'c': copyaccents = (1 - copyaccents); return 0;
		case 'f': forceseac = (1 - forceseac); return 0;
		case 'r': wantclosed = (1 - wantclosed); return 0;
		case 'z': wantzeroln = (1 - wantzeroln); return 0;
		case 'x': recurseexp = (1 - recurseexp); return 0;
		case 'i': initmoved = (1 - initmoved); return 0;
		case 'F': dummyswitch = (1 - dummyswitch); return 0;
		case 'e': errlvlflag = (1 -  errlvlflag); return 0;
		case 'E': ignorebadends = 1; return 0;		/* 1995/April/15 */
		case 'I': ignoreseac = 1; return 0;			/* 1995/April/15 */
		case 'S': relaxseac = 1; return 0;			/* 1995/May/8 */
		case 'd': wipeclean++; return 0;
		default: {
				fprintf(errout, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
}

/* Modified 1993/June/21 to complain if TEMP env var set wrong */

void maketemporary (char *new, char *name, char *ext, int stage) {
	int n;
	char extension[16];
	FILE *test;

	strcpy(extension, ext);
	sprintf(extension+2, "%d", stage);
	strcpy(new, tempdir);
	n = strlen(new);
/*	if (n > 0 && *(name + n - 1) != '\\') strcat(new, "\\"); */
	if (n > 0 && *(new + n - 1) != '\\') strcat(new, "\\");
	strcat(new, name);		
	makename(new, extension);
/*	Try it!  See if can write to this place! */
	if ((test = fopen(new, "wb")) == NULL) {
		fprintf(errout, "WARNING: Temporary directory `%s' does not exist\n",
			tempdir);
		tempdir = "";
		strcpy(new, name);		
		makename(new, extension);
	}
	else fclose(test);
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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
/*	printf("%s ", s); getch();	 */		/* debugging */
	return s;
}

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
			strcpy(charnames[chr], s); */		/* 1995/May/8 */
		if (chr >= 0 && chr < MAXCHRS) {
			charnames[chr] = zstrdup(s);
/*			if (charnames[chr] == NULL) outofmemory(s); */
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
	
/*	needsaccent = 0; */			/* in case we pop out */
/*	for (k = 0; k < MAXCHRS; k++) *charnames[k] = '\0';	 */
/*	for (k = 0; k < MAXCHRS; k++) charnames[k] = ""; */	/* 95/May/8 */
/*	for (k = 0; k < MAXCHRS; k++) charnames[k] = NULL;  */

	freeencoding();				/* in case it is still set up */

	if (traceflag) printf("Getting encoding\n");

	flag = getrealline(input, line);
	while (strstr(line, "/Encoding") == NULL && flag > 0) {
		flag = getrealline(input, line);
	}
	if (flag <= 0) {		/* hit EOF before /Encoding */
		fprintf(errout, "WARNING: Failed to find encoding\n");
		return 0;
	}
	if (strstr(line, "StandardEncoding") != NULL) {
		for (k = 0; k < MAXCHRS; k++) {
/*			strcpy(charnames[k], standardencoding[k]); */
			charnames[k] = zstrdup(standardencoding[k]);
/*			if (charnames[k] == NULL) outofmemory(standardencoding[k]); */
		}
		if (traceflag != 0) showencoding();
		return 1;
	}
	k = getrealline(input, line);
	if (sscanf(line, "%d 1 %d {", &ns, &ne) < 2) {
/*		fprintf(errout, " No /.notdef line");  */
	}
	else k = getrealline(input, line);	
	gobbleencoding(input);
	if (traceflag != 0) showencoding();
/*	needsaccent = checkencoding(); */
	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* returns how many new badend[] entries marked - zero when done 95/April/15 */

int findbadsubrs(FILE *input) {
	int nsubrs=0;
	int badcount=0;
	int ncall, nlen, nsubr;
	int changed=0;
	int count=0;

/*	for (k = 0; k < MAXSUBRS; k++) badend[k] = 0;	 */
/*	for (k = 0; k < maxsubrs; k++) badend[k] = 0;	 */

	if (traceflag) printf("Scanning up to Subrs\n");

/*	scan up to Subrs */
	while (fgets (line, MAXLINE, input) != NULL) {	
/*		fputs(line, output); */
		if (sscanf(line, "/Subrs %d array", &nsubrs) == 1) break;
	}

/*	if (nsubrs != maxsubrs) allocsubrs(nsubrs); */
/*	if (passes == 1) allocsubrs(nsubrs); */	/* first time only */
/*	for (k = 0; k < maxsubrs; k++) badend[k] = 0; */

/*	if (nsubrs > maxsubrs) printf("Too many Subrs %d\n", nsubrs); */

	if (traceflag) printf("Scanning %d Subrs\n", nsubrs);

/*	scan the Subrs up to CharStrings */
	while (fgets (line, MAXLINE, input) != NULL) {	
/*		fputs(line, output); */
		if (strstr(line, "/CharStrings ") != NULL) break;
/*		following is for call between Subrs ? ? ? */
/*		if (strstr(line, "callsubr") != NULL) {
			if (sscanf(line, "%d callsubr", &ncall) == 1) {
				if (badend[ncall]) {
					badend[nsubr] = 1;
					changed++;
				}
			}
			printf("WARNING: Subr call in Subrs:\t %s", line);
			badcount++;
		} */
		if (sscanf(line, "dup %d %d", &nsubr, &nlen) == 2) {
			if (debugflag) fputs(line, stdout);
			if (nsubr >= maxsubrs) {
				fprintf(errout, "Subr number %d exceeds array size %d\n",
						nsubr, maxsubrs);
				exit(1);
			}
/*			now have a Subr, process it */
			while (fgets (line, MAXLINE, input) != NULL) {	
/*				added 95/Nov/17 */
				if (strstr(line, "callsubr") != NULL) {
					if (sscanf(line, "%d callsubr", &ncall) == 1) {
						if (badend[ncall]) {
							if (badend[nsubr] == 0) {
								badend[nsubr] = 1;
								changed++;
							}
						}
					}
/*					printf("WARNING: Subr call in Subrs:\t %s", line); */
					badcount++;
				}
				if (strncmp(line, "endchar", 7) == 0) {
					if (traceflag) printf("endchar\t\tSubr %d\n", nsubr);
/*					if (nsubr >= 0 && nsubr < MAXSUBRS)  */
					if (nsubr >= 0 && nsubr < maxsubrs) {
						if (badend[nsubr] == 0) {	/* 95/Nov/17 */
							badend[nsubr] = 1;
							changed++;				/* 95/Nov/17 */
						}
					}
					count++;
				}	/* flush the endchar - and remember which Subr */
/*				else fputs(line, output); */
				if (strncmp(line, "return", 6) == 0) break;
/*				if (strncmp(line, "dup ", 4) == 0) break; */
				if (*line == '\n' || *line == '\r') break;
			}
		}
	}
	if (badcount > 0) {
/*		fprintf(errout, "WARNING: check those Subrs calling Subrs\n"); */
		if (traceflag) printf("Checking for Subrs calling Subrs\n");
	}
	return changed;
}

int cleanupends(FILE *output, FILE *input) {
	int nsubr, nlen, nsubrs=0;
	int count=0;
	char charname[64];

/*	for (k = 0; k < MAXSUBRS; k++) badend[k] = 0;	 */
/*	for (k = 0; k < maxsubrs; k++) badend[k] = 0;	 */

	if (traceflag) printf("Scanning up to Subrs\n");

/*	scan up to Subrs */
	while (fgets (line, MAXLINE, input) != NULL) {	
		fputs(line, output);
		if (sscanf(line, "/Subrs %d array", &nsubrs) == 1) break;
	}

/*	if (nsubrs > MAXSUBRS) printf("Too many Subrs %d\n", nsubrs); */
	if (nsubrs > maxsubrs) printf("Too many Subrs %d\n", nsubrs); 

	if (traceflag) printf("Scanning %d Subrs\n", nsubrs);

/*	scan the Subrs up to CharStrings */
	while (fgets (line, MAXLINE, input) != NULL) {	
		fputs(line, output);
		if (strstr(line, "/CharStrings ") != NULL) break;
/*		if (strstr(line, "callsubr") != NULL) {
			printf("WARNING: Subr call in Subrs:\t %s", line);
			badcount++;
		} */
		if (sscanf(line, "dup %d %d", &nsubr, &nlen) == 2) {
/*			if (debugflag) fputs(line, stdout); */
			if (nsubr >= maxsubrs) {
				fprintf(errout, "Subr number %d exceeds array size %d\n",
						nsubr, maxsubrs);
				exit(1);
			}
			while (fgets (line, MAXLINE, input) != NULL) {	
				if (strncmp(line, "endchar", 7) == 0) {
/*					if (traceflag) printf("endchar\t\tSubr %d\n", nsubr);*/
/*					if (nsubr >= 0 && nsubr < MAXSUBRS)  */
					if (nsubr >= 0 && nsubr < maxsubrs) {						badend[nsubr] = 1;
					}
					count++;
				}	/* flush the endchar - and remember which Subr */
				else fputs(line, output);
				if (strncmp(line, "return", 6) == 0) break;
				if (*line == '\n' || *line == '\r') break;
			}
		}
	}

	if (verboseflag) printf("Scanning the CharStrings\n");

/*	scan the CharStrings */
	while (fgets (line, MAXLINE, input) != NULL) {	
		fputs(line, output);
		if (sscanf(line, "/%s %d", &charname, &nlen) == 2) {
			if (debugflag) fputs(line, stdout);
			while (fgets (line, MAXLINE, input) != NULL) {	
				fputs(line, output);
				if (strstr(line, "callsubr") != NULL) {
					if (sscanf(line, "%d callsubr", &nsubr) == 1) {
/*						if (nsubr >= 0 && nsubr < MAXSUBRS)  */
						if (nsubr >= maxsubrs) {
							fprintf(errout, "Subr number %d exceeds array size %d\n",
									nsubr, maxsubrs);
							exit(1);
						}
						if (nsubr >= 0 && nsubr < maxsubrs) {
							if (badend[nsubr]) {
								if (traceflag)
									printf("%s\t\t%s", charname, line);
								fputs("endchar\n", output);
							}
						}
					}
				}
				if (strncmp(line, "endchar", 6) == 0) break;
				if (*line == '\n' || *line == '\r') break;
			}
		}
		if (strstr(line, "/FontName") != NULL) break;
		if (strstr(line, "definefont") != NULL) break;
	}

	if (verboseflag) printf("Copying to the end of the file\n");

/*	copy to end of file */
	while (fgets (line, MAXLINE, input) != NULL) {	
		fputs(line, output);
	}
	return count;
}

int endcharfix(char *infilename) {
	int k, count;
	int badends=0, passes=0;
	char outfilename[FILENAME_MAX];
	FILE *input, *output;

	strcpy(outfilename, infilename);
	forceexten(outfilename, "end");
	if (strcmp(infilename, outfilename) == 0) return -1;
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);
/*		exit(1); */
		return -1;
	}
	if ((output = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename);
		fclose(input);
		exit(1);
/*		return -1; */
	}
/*	for (k = 0; k < MAXSUBRS; k++) badend[k] = 0; */
/*	for (k = 0; k < maxsubrs; k++) badend[k] = 0; */
	if (verboseflag)
		printf("Find and mark (recursive) Subrs ending in endchar\n");
	passes = 1;
	while (findbadsubrs(input) > 0) {
		rewind(input);
		passes++;
	}
	rewind (input);
	badends=0;
/*	for (k = 0; k < MAXSUBRS; k++) if (badend[k]) badends++; */
	for (k = 0; k < maxsubrs; k++) if (badend[k]) badends++;
	if (verboseflag) {
		printf("%d Subrs directly or indirectly end in endchar (%d passes)\n",
			   badends, passes);
	}
	if (traceflag) {	/* debugging output 95/Nov/17 */
/*		for (k = 0; k < MAXSUBRS; k++)	if (badend[k]) printf("%d ", k); */
		for (k = 0; k < maxsubrs; k++)	if (badend[k]) printf("%d ", k);
		putc('\n', stdout);
	}
	if (verboseflag) printf("Removing endchar from Subrs\n");
	count = cleanupends(output, input);
	fclose(output);
	fclose(input);
/*	if (verboseflag)
		printf("infilename %s outfilename %s\n", infilename, outfilename); */
	(void) remove (infilename);
	(void) rename (outfilename, infilename);
	return 0; /* good end */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stage is always 0 ??? */	/* This is old stuff again */

int checkoutpfa (char *pfaname, char *fn_dec, char *fn_pln, int stage) {
/*	FILE *fp_dec, *fp_pln; */
	char fn_ind[FILENAME_MAX]; 
	FILE *fp_in;
	int c, d;
	char *s;
	int flag = 0;

	strcpy(fn_ind, pfaname);	
	extension(fn_ind, "pfa"); 
	
	if ((fp_in = fopen(fn_ind, "rb")) == NULL) {	/* see whether exists */
		underscore(fn_ind);							/* 1993/May/30 */
		if ((fp_in = fopen(fn_ind, "rb")) == NULL) {/* see whether exists */
			perror(fn_ind);
			exit(13);
		}
	}

	if (stage == 0) strcpy (fn_in, fn_ind);

/*	scanning base file */

	if (verboseflag != 0) printf("Processing file %s\n", fn_ind);

/*	sanity check -- check whether it even looks like a PFA file */
	c = fgetc(fp_in); d = fgetc(fp_in);
	if (c != '%' || d != '!') {
		fprintf(errout, "ERROR: This does not appear to be a PFA file\n");
		exit(13);							/* 1993/May/30 */
	}
/*	New stuff 1993 July 1 */
	encodingexists = getencoding(fp_in);			/* Try and read encoding */
	fclose(fp_in);

	if (stage == 0) {

		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
		strcpy(fn_out, s);			/* copy input file name minus path */

		forceexten(fn_out, "pfa");
/*		if (forceoutname != 0) strcpy(fn_out, outfilename); */
		
		if (strcmp(fn_in, fn_out) == 0 && writeoutflag != 0) {
			strcpy(fn_bak, fn_in);
			forceexten(fn_in, "bak");
			printf("Renaming %s to %s\n", fn_bak, fn_in);
			remove(fn_in);		/* in case backup version already present */
			rename(fn_bak, fn_in);
			renameflag++;
			strcpy(fn_ind, fn_in);
		}
	
		if (verboseflag != 0 && writeoutflag == 0) 
			printf("Output will be going to %s\n", fn_out);
	}

	maketemporary(fn_dec, fn_out, "dec", stage);
	if (traceflag != 0) printf("Using %s as temporary\n", fn_dec);
	maketemporary(fn_pln, fn_out, "pln", stage);
	if (traceflag != 0) printf("Using %s as temporary\n", fn_pln);

	if (traceflag != 0) printf("Pass A (down) starting\n");			/* */
		
	if ((fp_in = fopen(fn_ind, "rb")) == NULL) { 
		perror(fn_ind);
		exit(2);
	}
	if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
		perror(fn_dec);
		exit(2);
	}

	eexecscan = 1;
	charscan = 0;  decodecharflag = 0;  charenflag = 0; binaryflag = 0;

	(void) decrypt(fp_dec, fp_in);		/* actually go do the work */

	fclose(fp_in);
	if (ferror(fp_dec) != 0) {
		fprintf(errout, "Output error ");
		perror(fn_dec);
		cleanup();
		exit(2);
	}
	else fclose(fp_dec);

	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass B (down) starting\n");			/* */
		
	if ((fp_dec = fopen(fn_dec, "rb")) == NULL) { 
		perror(fn_dec);
		cleanup();
		exit(2);
	}
	if ((fp_pln = fopen(fn_pln, "wb")) == NULL) { 
		perror(fn_pln);
		cleanup();
		exit(2);
	}

	eexecscan = 0;
	charscan = 1;  decodecharflag = 1;  charenflag = 1;  binaryflag = 1;

	(void) decrypt(fp_pln, fp_dec);		/* actually go do the work */

	fclose(fp_dec);
	if (ferror(fp_pln) != 0) {
		fprintf(errout, "Output error ");
		perror(fn_pln);
		cleanup();
		exit(2);
	}
	else fclose(fp_pln);

	if (debugflag != 0) _getch();

/*	NOW HAVE PLN FILE IN SIGHT */
	return flag;
}

int analyzepfa(char *fn_pln) {		/* separated 95/April/15 */
	int k, badends=0;
	int flag;

/*	NOW HAVE PLN FILE IN SIGHT */

	if (traceflag != 0) 
		printf("Analysis and extraction of CharStrings starting\n"); 

	needtorelax = 0;

	for(;;) {	/* repeat if Subrs have bad ends */
		if ((fp_pln = fopen(fn_pln, "rb")) == NULL) { 
			perror(fn_pln);
			cleanup();
			exit(2);
		}
		if (verboseflag != 0) printf("Information Gathering Pass \n");
		flag = extractcharnames(fp_pln);	/* extract info first */
/*		flag is non-zero if font already modified or has no Subrs */
/*		rewind(fp_pln); */
		fclose(fp_pln);

		badends=0;							/* 95/April/15 */
		for (k = 0; k < nsubrs; k++) {
			if (badend[k]) badends++;
		}
		if (badends == 0 || ignorebadends != 0) break;
		if (badends > 0) {
			if (verboseflag) {
/*				printf("%d Subrs end in endchar\n", badends); */
				printf("Will have to remove endchar from %d Subrs\n", badends);
			}
			if (endcharfix (fn_pln) != 0) break;
/*			exit(1);	*//* check pln file for now */
		}
	}
	if (traceflag != 0) fprintf(errout, "End of checking out PFA\n");	
/*	return 0; */
	return flag;
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
	fprintf(errout, "HASHED %ld\n", hash);	(void) _getch(); 
	return hash;
}

char *progfunction="make accents safe for ATM";

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);	 
/*	fprintf(output, "%s (%s %s)\n", programversion, date, compiletime); */
	fprintf(output, "%s - %s - version %s (%s %s)\n", 
		progname, progfunction, progversion, date, compiletime);
}

int decodearg(char *command, char *next, int firstarg) {
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
/*			if (spaceflag != 0) {
				if (sscanf(s, "%d", &spacewidth) > 0) spaceneeded = 1;
				spaceflag = 0;
			} */
			break;	/* default - no flag set */
		}
	}
	return firstarg;
}

/*	check command line flags and command line arguments */

int commandline(int argc, char *argv[], int firstarg) {
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

void copyfile (FILE *output, FILE *input) {
	int c;
	while ((c = getc(input)) != EOF) putc(c, output);
}

int _cdecl main(int argc, char *argv[]) {       /* main program */
	FILE *fp_out;
	int firstarg=1;
	int k, n, m, flag=0;
	int unchanged = 0;		/* count of PFB files that were not modified */
	char *s;
	char *scharname;		/* 95/May/8 */

	if (strlen(progname) < 16) strcpy(progname, compilefile);
	if ((s = strchr(progname, '.')) != NULL) *s = '\0';
	uppercase(progname);

	strcpy (line, argv[0]);
	uppercase (line);
	if (strstr(line, progname) == NULL) quietflag = 1;

	if (argc < firstarg+1) showusage(argv[0]); 

	firstarg = commandline(argc, argv, 1); /* check for command flags */

	if (verboseflag > 1) dotsflag++;

	if (wipeclean == 4) wipeclean = 0;	/* `ddd' on command line */

/*	if (replacesubr != 0) copyaccents = 1; */ /* ??? */

	if (argc < firstarg + 1) showusage(argv[0]); 
		
	if (quietflag != 0) {
		verboseflag = 0; traceflag = 0; dotsflag = 0;
	}

/*	scivilize(compiledate);	 */
	if (quietflag == 0) stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	(void) signal(SIGINT, ctrlbreak);

	if ((s = getenv("TMP")) != NULL) tempdir = s;
	if ((s = getenv("TEMP")) != NULL) tempdir = s;

	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	if (argc < firstarg + 1) showusage(argv[0]);

/*	strcpy(fn_in, argv[firstarg]);
	extension(fn_in, "pfa"); */

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

/*	unchanged = 0; */

	if (traceflag) printf("Resetting encoding\n");

/*	for (k = 0; k < MAXCHRS; k++) charnames[k] = ""; */	/* 95/May/8 */
	for (k = 0; k < MAXCHRS; k++) charnames[k] = NULL;	/* 95/May/8 */

/*	if (heapflag) checkencoding(); */

	for (k = firstarg; k < argc; k++) {

/*		for (m = 0; m < MAXCHRS; m++) charnames[m] = ""; */	/* 95/May/8 */

		badsubr = 0;										/* 95/May/8 */

		for (m = 0; m < MAXADJUST; m++) {
			charsubrs[m] = charreplace[m] = charbase[m] = characcent[m] = -1;
		}

		callaccents = callaccentsinit;	/* reset in case changed */

/*		strcpy(fn_in, argv[k]);	
		extension(fn_in, "pfa"); 
		checkoutpfa (fn_in, fn_dec, fn_pln, 0); */

/*		flag is non-zero if file cannot be processed for some reason */

		if (traceflag) printf("Considering file `%s' (arg %d)\n", argv[k], k);
		
		flag = checkoutpfa (argv[k], fn_dec, fn_pln, 0); 

		flag = analyzepfa (fn_pln);			/* split out 95/April/15 */

		if (ignoreseac) {
			maketemporary(fn_adj, fn_out, "adj", 0);
			if ((fp_adj = fopen(fn_adj, "wb")) == NULL) { 
				perror(fn_adj);
				cleanup();
				exit(2);
			}
			fp_pln = fopen(fn_pln, "rb");
			copyfile (fp_adj, fp_pln);
			fclose (fp_adj);
			goto skipseac;		/* 1995/April/15 */
		}

		if (flag != 0) {	/* 1993/June/8 */
			fprintf(errout, "NOTE:    file `%s' NOT processed\n",
				(renameflag ?  fn_bak : fn_in));
			if (renameflag != 0) rename(fn_in, fn_bak); 
			cleanup(); 
			unchanged++;
			continue; 
		}

		if (hintstrip != 0 && callaccents != 0) {
			if (verboseflag != 0) 
			 printf("NOTE: Forcing use of original accent code for accents\n");
		    callaccents = 0;
		}

		if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { /* so cleanup kicks in */
			perror(fn_dec);
			cleanup();
			exit(2);
		}
		else fclose(fp_dec);
		
/*		fp_pln = fopen(fn_pln, "r");  */
		fp_pln = fopen(fn_pln, "rb"); 		/* so we can safely use seek */

		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

		if (traceflag != 0) {
/* show accumulated table of required base and accent characters */
			printf("Table of base and accent characters called by `seac'\n");
/*			for (k = 0; k < MAXCHRS; k++) {	*/			/* debugging */
			for (k = 0; k < MAXADJUST; k++) {				/* debugging */
				if (k < MAXCHRS) scharname = standardencoding[k];
				else scharname = charnames[k-MAXCHRS];
				if (charbase[k] != 0) {
					printf("%d\t%s (base   %d)\t%ld\n", k,
						   scharname, charbase[k], charposition[k]);
				}
				if (characcent[k] != 0) {
					printf("%d\t%s (accent %d)\t%ld\n", 
						   k, scharname, characcent[k], charposition[k]);
				}
			}
		} /* if traceflag */

/*	Now have  `plain' file and info on number of Subrs and CharStrings */

		if (writeoutflag == 0) return 0;		/* while debugging */

		maketemporary(fn_adj, fn_out, "adj", 0);
		if (traceflag != 0) printf("Using %s as temporary\n", fn_adj);

		if ((fp_adj = fopen(fn_adj, "wb")) == NULL) { 
			perror(fn_adj);
			cleanup();
			exit(2);
		}

		if (verboseflag != 0) printf("Modification Pass\n");

		rewind(fp_pln); 

		n = makesafe(fp_adj, fp_pln);
		
		if (hintstrip != 0 && callaccents != 0) {
			if (verboseflag != 0) 
				printf("NOTE: Recommend use of 's' command line flag\n");
			callaccents = 0;
		}

		if (traceflag != 0) printf("finished processing\n");
		
		fclose(fp_pln); 

		if (ferror(fp_adj) != 0) {
			fprintf(errout, "Output error ");
			perror(fn_adj);
			cleanup();
			exit(2);
		}
		else fclose(fp_adj);

		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

		if (n < 0) {	/* if there was a failure in making output file */
			fprintf(errout, "WARNING: Sorry, unable to process file `%s'\n",
				fn_in);
			if (renameflag != 0) rename(fn_in, fn_bak);
			cleanup(); 
/*			return 0; */
			continue;				/* 1993/May/30 */
		}

		if (debugflag != 0) _getch();

skipseac:							/* 1995/April/15 */

		if (traceflag != 0) printf("Pass B (up) starting\n"); 

		if ((fp_adj = fopen(fn_adj, "rb")) == NULL) { 
			perror(fn_adj);
			cleanup();
			exit(2);
		}
		if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
			perror(fn_dec);
			cleanup();
			exit(2);
		}

		eexecscan = 1; charenflag = 1; charscan = 1;

		(void) encrypt(fp_dec, fp_adj);	/* actually go do the work */
		
		fclose(fp_adj);
		if (ferror(fp_dec) != 0) {
			fprintf(errout, "Output error ");
			perror(fn_dec);
			cleanup();
			exit(2);
		}
		else fclose(fp_dec);

		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

		if (debugflag != 0) _getch();

		if (traceflag != 0) printf("Pass A (up) starting\n"); 

		if ((fp_dec = fopen(fn_dec, "rb")) == NULL) { 
			perror(fn_dec); cleanup();	exit(2);
		}
		if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
			perror(fn_out);
			cleanup();
			exit(2);
		}

		eexecscan = 1; charenflag = 0; charscan = 0;

		(void) encrypt(fp_out, fp_dec);	/* actually go do the work */
		
		fclose(fp_dec);

		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

		if (debugflag != 0) _getch();
		
		if (ferror(fp_out) != 0) {
			fprintf(errout, "Output error ");
			perror(fn_out);
			cleanup();
			exit(2);
		}
		else fclose(fp_out);

		cleanup();			/* 1994/May/30 */
		freeencoding();
		freesubrs();
		if (badsubr)
			fprintf(errout, "ERROR: %d bad Subr calls in this font\n", badsubr);
		if (needtorelax) {
			fprintf(errout, "ERROR: %d SEAC calls to numbers not in ASE.\n",
				   needtorelax);
			fprintf(errout, "       Use -S on command line?\n");
		}
/*		if (verboseflag) */
			printf("Output in %s\n", fn_out);
	}	/* end of for loop over PFA files */

/*	cleanup();	*/		/* should be inside loop */
/*	If some PFB files were NOT changed, and 'e' command line flag used */
	if (unchanged > 0 && errlvlflag != 0) exit(1);			/* 1993/July/1 */
	return 0;
}	

/* STRUCTURE OF SUBRS IN OUTPUT: */

/* nsubrs								original Subrs */
/* nsubrsbase + nsubrsaccent			base and accent character Subrs */
/* (base character: code + hints, accent: code only) minus initial moveto */
/* seaccount							hint replacements for composite char */
/* nsubrsaccent							hints for accented chars themselves */

/* STRUCTURE OF CHARSTRINGS IN OUTPUT: */

/* Most characters (not base, or accent, or composite): exactly as in input */
/* Base characters: initial moveto plus call to base char Subr */
/* Accent characters: initial moveto, call to hint Subr, call accent Subr */ 
/* Composite characters: initial moveto, call to base char Subr,
								hint replacement call, call to accent Subr */ 

/* warn if there is no Subr section !!! OK, kae sure MERGEPFA takes care of */

/* warn if there are no `seac' calls !!! */

/* copy accent code rather than call Subr, so can modify hints ? YES ! */

/* stick in `dotsection' around accent code ? --- optional */

/* is it possible to invoke hint replacement for accent ? */
/* put in extra dummy Subr in attempt to do this */
/* or do we need one Subr per accented character ? YES, MAYBE */

/* copy accents only when ady not zero ??? otherwise call Subr ? */

/* is there a problem if accent goes left past bounding box of base ? */
/* that is if lsb of base is not lsb of accented character ? */
/* maybe figure out smallest lsb and adjust accordingly ? */
/* which also means moving the vstems in the base character  */

/* remaining problem: no hint replacement when switching ? FIXED */

/* remaining problem: what if accent code already contains `dotsection' ? */

/* REMAINING PROBLEM: existing hint replacement calls not adjusted */

/* possibility: add Subrs for proper hint replacement in accent DONE */

/* possibility: add Subrs for every OtherSubr call */

/* does this handle the new style hint switch calls correctly ? OK */
/* that is, what about <n> 4 callsubr ? OK */

/* use efficient hint switching code if subr 4 set up for it ... OK, done */

/* in finding initial x and y, may need to follow subrs */
/* this means need initial x and y for subr ??? */
/* space, dotlessi, quoteleft, quoteright */
/* or maybe organise search recursively, like other stuff */
/* take care of the characters that print with ***** */

/* problem with Subrs that end in endchar */
/* if base character ends in Subrs that ends in endchar, */
/* we never get to accent of composite character constructed */

/* relaxseac allows extending ASE with characters from fonts encoding */
/* non-ASE characters used for base and accent must then be in encoding */
/* and must be in positions other than those occupied by ASE glyphs */
/* so overall encoding for this purpose is ASE with blanks filled in */
