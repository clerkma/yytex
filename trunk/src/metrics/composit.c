/* composit.c
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
#include <conio.h>

#ifdef _WIN32
#define __far 
#define _fmalloc malloc
#define _frealloc realloc
#define _ffree free
#endif

#include "metrics.h"

/* #define MAXADJUST (MAXCHRS+MAXCHRS) */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int debugflag = 0;
int wipeclean = 1;	
int dotsflag = 1;
int quietflag = 0;
int compositflag = 0;
int relaxseac = 0;		/* non-zero means allow even if chars not in ASE */
int adjust256 = 1;		/* add 256 to seac codes for non ASE glyph names */
						/* avoids conflict with ASE glyph names */

int renameflag = 0;		/* not used except in error recovery  */

int eexecscan = 0;		/* scan up to eexec before starting */
int charscan = 0;		/* scan up to RD before starting */
int charenflag = 0;		/* non-zero charstring coding instead of eexec */
int decodecharflag = 0;	/* non-zero means also decode charstring */
int binaryflag = 0;		/* input is binary, not hexadecimal */

int bogusseac = 0;		/* how many glyphs referred to that are not in ASE */

volatile int bAbort = 0;			/* set when user types control-C */

int extractflag = 1;	/* non-zero => extract info only */

extern int encrypt(FILE *, FILE *);

extern int decrypt(FILE *, FILE *);

char *compositfile = "";

char *tempdir="";		/* place to put temporary files */

FILE *errout=stdout;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
char fn_bak[FILENAME_MAX], fn_cmp[FILENAME_MAX];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* char *charnames[MAXCOMPOSITES]; */		/* composite character charname */
char * __far *charnames=NULL;

/* int bchar[MAXCOMPOSITES], achar[MAXCOMPOSITES]; */
short __far *bchar=NULL;
short __far *achar=NULL;

/* int adx[MAXCOMPOSITES], ady[MAXCOMPOSITES]; */
short __far *adx=NULL;
short __far *ady=NULL;

/* int hit[MAXCOMPOSITES]; */	/* non-zero if old one exists in PFA file */
short __far *hit=NULL;

int charindex;			/* number of composites seen in new file */

/**************************************************************************/

/* If we want to allow for adjust256, need to double size of these */

int sidebearing[MAXADJUST], charwidth[MAXADJUST];

int maxcomposites=0;	/* currently allocated */

int ncomposites;		/* number of composites seen in PFA file */

int nhits;				/* number of new ones already in old PFA file */

int ncharstrings;		/* number of CharStrings in original PFA file */

int charseen = 0;		/* non-zero after character string RD and ND seen */
int nextend = 0;		/* non-zero if next line is ND */

char rdsynom[16] = "RD";	/* RD or -| or ... */
char ndsynom[16] = "ND";	/* ND or |- or ... */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

int wantcpyrght=1;

/* Font merging and normalizing program version */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char progname[16]="";

/* char *progversion="1.0"; */
/* char *progversion="1.1"; */
/* char *progversion="1.2"; */	/* 95/May/8 */
/* char *progversion="1.2.1"; */	/* 95/May/8 */
char *progversion="2.0";	/* 98/Dec/25 */

char *copyright = "\
Copyright (C) 1992--1998  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1992--1996  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 2670755 */
/* #define COPYHASH 13986445 */
/* #define COPYHASH 5618236 */
/* #define COPYHASH 11501465 */
/* #define COPYHASH 607478 */
/* #define COPYHASH 15888034 */
#define COPYHASH 16753036

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	 
char *encoding[MAXCHRS];			/* actual encoding of this font */

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

void freecharnames(void) {			/* 1995/May/8 */
	int k;
	if (charnames == NULL) return;
/*	for (k = 0; k < MAXCOMPOSITES; k++)  */
	for (k = 0; k < maxcomposites; k++) {
/*		if (strcmp(charnames[k], "") != 0) { */
		if (charnames[k] != NULL) {
			free(charnames[k]);
/*			charnames[k] = ""; */
			charnames[k] = NULL;
		}
	}
}

void freecomposites(void) {
	if (maxcomposites > 0) freecharnames();
	if (charnames != NULL) free(charnames);	/* also deallocate entries */
	if (bchar != NULL) free(bchar);
	if (achar != NULL) free(achar);
	if (adx != NULL) free(adx);
	if (ady != NULL) free(ady);
	if (hit != NULL)free(hit);
	charnames = NULL;
	achar = bchar = adx = ady = hit = NULL;
	maxcomposites=0;
}

void alloccomposites (int ncomposites) {
	if (verboseflag) printf("Allocating space for %d composites\n", ncomposites);
	if (maxcomposites != 0) freecomposites();
	charnames = (char * __far *) _fmalloc(ncomposites * sizeof(char *));
	bchar = (short *) _fmalloc(ncomposites * sizeof(short));
	achar = (short *) _fmalloc(ncomposites * sizeof(short));
	adx = (short *) _fmalloc(ncomposites * sizeof(short));
	ady = (short *) _fmalloc(ncomposites * sizeof(short));
	hit = (short *) _fmalloc(ncomposites * sizeof(short));
	if (charnames == NULL || achar == NULL || bchar == NULL ||
		adx == NULL || ady == NULL || hit == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		exit(1);
	}
	maxcomposites = ncomposites;
}

void realloccomposites (int ncomposites) {
	if (verboseflag) printf("Reallocating space for %d => %d composites\n",
							maxcomposites, ncomposites);
	charnames = (char * __far *) _frealloc(charnames, ncomposites * sizeof(char *));
	bchar = (short *) _frealloc(bchar, ncomposites * sizeof(short));
	achar = (short *) _frealloc(achar, ncomposites * sizeof(short));
	adx = (short *) _frealloc(adx, ncomposites * sizeof(short));
	ady = (short *) _frealloc(ady, ncomposites * sizeof(short));
	hit = (short *) _frealloc(hit, ncomposites * sizeof(short));
	if (charnames == NULL || achar == NULL || bchar == NULL ||
		adx == NULL || ady == NULL || hit == NULL) {
		fprintf(stderr, "Memory reallocation error\n");
		exit(1);
	}
	maxcomposites = ncomposites;
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

static int getline(FILE *fp_in, char *line) {
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	return strlen(line); 
} 

int lookupcompo(char *name) {			/* look up composite name in table */
	int k, flag = -1;
	for (k = 0; k < charindex; k++) {
		if (charnames[k] == NULL) continue;
		if (strcmp(charnames[k], name) == 0) {
			flag = k;
			break;
		}
	}
	return flag;
}

int lookupencoding(char *name) {		/* look up in font encoding 95/May/8 */
	int k, flag = -1;
	for (k = 0; k < MAXCHRS; k++) {
		if (encoding[k] == NULL) continue;
		if (strcmp(encoding[k], name) == 0) {
			flag = k;	break;
		}
	}
	return flag;
}

int lookupstandard(char *name) {	/* look up in Adobe StandardEncoding */
	int k, flag = -1;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(standardencoding[k], name) == 0) {
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

/* need to remove the line
"% Composites in this font made safe from ATM by SAFESEAC from Y&Y, Inc.\n",
*/

int extractcompo(FILE *fp_out, FILE *fp_in, int pass, int extractflag) {
	char charname[CHARNAME_MAX];
	int k, l, m, n;
	int unknowns=0, count=0;
	char *s;
	int side, width; 
	int notse=0;
/*	long linestart, charstart; */
	int sbxnew, adxnew, adynew, bcharnew, acharnew, adxmod;
	char *acharname, *bcharname;
	
	if (pass == 0) {
/*		for (k = 0; k < MAXCHRS; k++) charwidth[k] = UNKNOWN; */
		if (adjust256) for (k = 0; k < MAXADJUST; k++) charwidth[k] = UNKNOWN;
		else for (k = 0; k < MAXCHRS; k++) charwidth[k] = UNKNOWN;
	}
	if (pass > 0) {
		if (fp_out == NULL) fprintf(errout, "fp_out == NULL\n"); 
	}
	
	if (extractflag != 0 && pass > 0)
		fprintf(fp_out,  "StartComposites %d\n", ncomposites);
	if (pass == 0) ncomposites = 0;

/*  scan up to eexec encrypted part */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
/*		if (extractflag == 0) fputs(line, fp_out); */
		if (extractflag == 0) {					/* changed 96/April/7 */
/*	copy up to eexec, but not SAFESEAC comment since it will be stale */
			if (*line != '%' || strstr(line, "SAFESEAC") == NULL)
				fputs(line, fp_out);
		}
	}
	if (extractflag == 0) fputs(line, fp_out);
/*	scan up to CharStrings */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "/CharStrings") == NULL) {
		if (extractflag == 0) fputs(line, fp_out);
	}
	s = strstr(line, "/CharStrings") + 13;
	sscanf(s, "%d%n", &ncharstrings, &n);
	if (extractflag == 0 && pass == 1 && charindex > nhits) {
		strcpy(charname, s+n);					/* temporary */
		sprintf(s, "%d", ncharstrings + charindex - nhits);
		strcat(s, charname);
		if (verboseflag != 0) printf("%s", line);	/* debugging */
	}
	if (extractflag == 0) fputs(line, fp_out);

/*	scan all of the CharStrings */
	for (;;) {
/*		linestart = ftell(fp_in); */
		n = getline(fp_in, line);
		if (n <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;
		if (charseen == 0 && nextend != 0) {
			sscanf (line, "%s\n", ndsynom);
			stripreturn(ndsynom);
			if (traceflag != 0) printf("NDsynonym: `%s'\n", ndsynom);
			charseen = 1;
		}
		if (strstr(line, "endchar") != NULL) nextend = 1; else nextend = 0;
/*		if (extractflag == 0) fputs(line, fp_out); */

		if ((s = strchr(line, '/')) != NULL) {
/*			charstart = linestart; */
			if (charseen == 0) {
				sscanf(s, "/%s %d %s\n", charname, &k, rdsynom);
				stripreturn(rdsynom);
				if (traceflag != 0) printf("RDsynonym: `%s'\n", rdsynom);
			}
			sscanf(s, "/%s ", charname);
			if (extractflag == 0) fputs(line, fp_out);
			(void) getline(fp_in, line);	/* hsbw line */
			if (strstr(line, "hsbw") != NULL) {
				m = sscanf(line, "%d %d hsbw", &side, &width);
				if (m == 2) {
					k = lookupstandard(charname);
					if (k >= 0) {		/* save information for later */
						sidebearing[k] = side;
						charwidth[k] = width;
					}
					if (relaxseac) {	/* 95/May/8 */
						k = lookupencoding(charname);
						if (k >= 0) {	/* name is in current encoding ? */ 
							if (adjust256) {
								sidebearing[k+MAXCHRS] = side;
								charwidth[k+MAXCHRS] = width;
							}
							else if (strcmp(standardencoding[k], "") == 0) {
								sidebearing[k] = side;
								charwidth[k] = width;
							} /* else conflicts with ASE */
						} /* end of if k >= 0 */
					} /* end of relaxseac */
/*					else if (traceflag != 0 && pass == 0)
							printf("%s? ", charname); */
				} /* end of scanned hsbw (m == 2) */
			} /* end of strstr hsbw != NULL */

			if (extractflag == 0) fputs(line, fp_out);
			(void) getline(fp_in, line);	/* potential seac line */
			if (strstr(line, "endchar") != NULL) nextend = 1; 
			else nextend = 0;				/* ??? */
/* 11 141 212 90 207 seac */
/* C 90 ; WX 611 ; N Z ; B 9 0 597 662 ; */
/* C 207 ; WX 333 ; N caron ; B 11 507 322 674 ; */
/* CC Zcaron 2 ; PCC Z 0 0 ; PCC caron 139 212 ; */
			if ((s = strstr(line, "seac")) != NULL) {
/*				printf("X"); */	/* check */
				if (dotsflag != 0) putc('.', stdout);
				if (pass == 0) ncomposites++;
				if ((l = lookupcompo(charname)) >= 0) {
					hit[l]++; nhits++;
				}
				m = sscanf (line, "%d %d %d %d %d seac", 
					&sbxnew, &adxnew, &adynew, &bcharnew, &acharnew);
/* check side bearings and widths ????  */
				if (extractflag != 0) {
					if (pass > 0) {
						if (charwidth[bcharnew] == UNKNOWN)
						fprintf(errout, "ERROR: %s base char (%d) unknown\n",
								charname, bcharnew);
						else {
							if (side != sidebearing[bcharnew])  /* defined ? */
								fprintf(errout,
		"ERROR: %s composite side bearing %d does not match base char %d\n",
										charname, side, sidebearing[bcharnew]);
							if (width != charwidth[bcharnew])  /* defined */
								fprintf(errout,
		"ERROR: %s composite width %d does not match base char %d\n",
										charname, width, charwidth[bcharnew]);
						}
						if (charwidth[acharnew] == UNKNOWN)
						fprintf(errout, "ERROR: %s accent char (%d) unknown\n",
								charname, acharnew);
						else {
							if (sbxnew != sidebearing[acharnew]) 
								fprintf(errout,
		"ERROR: %s seac side bearing %d does not match accent %d\n",
									charname, sidebearing[acharnew], sbxnew);
						}
						adxmod = adxnew + sidebearing[bcharnew] 
									- sidebearing[acharnew];
						if (bcharnew < MAXCHRS) {
							bcharname = standardencoding[bcharnew];
							if (strcmp(bcharname, "") == 0)
								bcharname = encoding[bcharnew];
						}
						else bcharname = encoding[bcharnew - MAXCHRS];
						if (acharnew < MAXCHRS) {
							acharname = standardencoding[acharnew];
							if (strcmp(acharname, "") == 0)
								acharname = encoding[acharnew];
						}
						else acharname = encoding[acharnew - MAXCHRS];

					fprintf(fp_out, "CC %s 2 ; PCC %s 0 0 ; PCC %s %d %d ;\n",
						charname, bcharname, 
							acharname, adxmod, adynew);
					} /* end of if pass > 0 */
				} /* end of if extractflag != 0 */
				else {	/* NOT extracting, replace line with new words */
					k = lookupcompo(charname);
					if (k >= 0) {
						if (charwidth[bchar[k]] == UNKNOWN) 
							fprintf(errout, "ERROR: base char %d unknown\n",
									bchar[k]); 
						if (charwidth[achar[k]] == UNKNOWN)
							fprintf(errout, "ERROR: accent char %d unknown\n",
									achar[k]); 
/* The following will of course be junk if base or accent unknown... */
						adxmod = adx[k] + sidebearing[achar[k]] 
									- sidebearing[bchar[k]];
						fprintf(fp_out, "%d %d %d %d %d seac\n",
							sidebearing[achar[k]], adxmod, ady[k], 
								bchar[k], achar[k]);
						count++;
					}
					else {			/* composite name not found ??? */
						fputs(line, fp_out);
						if (traceflag != 0) printf("%s? ", charname);
						unknowns++;
					}
				} /* end of NOT extracting part */
			}	/* end of seac */
			else {	/* is k always defined here ? */
				if (k < 0 && traceflag != 0 && pass == 0) {
					printf("%s? ", charname); /* not in SE and not composit */
					notse++;
				}
				if (extractflag == 0) fputs(line, fp_out);	/* after hsbw */
			}
		}
		else if (extractflag == 0) fputs(line, fp_out); /* NOT '/' line */
		if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	}		

	if (traceflag != 0 && pass == 0 && notse != 0)
printf("\nAbove %d character%s not in StandardEncoding and not composite\n",
	notse, (notse == 1) ? "" : "s");
	if (extractflag == 0 && pass == 1) {	/* insert added composites */
		for (k = 0; k < charindex; k++) {
			if (hit[k] != 0) continue;
			putc('\n', fp_out);
			fprintf(fp_out, "/%s 0 %s\n", charnames[k], rdsynom);
			fprintf(fp_out, "%d %d hsbw\n",
				sidebearing[bchar[k]], charwidth[bchar[k]]);
			adxmod = adx[k] + sidebearing[achar[k]] 
									- sidebearing[bchar[k]];
			fprintf(fp_out, "%d %d %d %d %d seac\n",
				sidebearing[achar[k]], adxmod, ady[k], bchar[k], achar[k]);
			fprintf(fp_out, "%s\n", ndsynom);
			if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
		}
	}
	if (extractflag == 0) {		/* copy the tail across */
		if (n != 0) fputs(line, fp_out);
		while ((n = getline(fp_in, line)) != 0)	{
			fputs(line, fp_out);
			if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
		}
	}
	if (extractflag != 0 && pass > 0) fprintf(fp_out, "EndComposites\n");
	if (dotsflag != 0 || unknowns > 0) putc('\n', stdout);
	if (count != 0) 
		printf("Composites for %d chars were altered\n", count);
	if (unknowns != 0) 
		printf("Composites for %d chars were not altered\n", unknowns);
	if (charindex > nhits)
		printf("Composites for %d chars were added\n", charindex - nhits);
	return 0;
}

/* Read new composites from file */

int readcomposites (FILE *fp_in, char *filename) {
	char charname[CHARNAME_MAX], bcharname[CHARNAME_MAX], acharname[CHARNAME_MAX];
	int dx, dy;
/*	int side, width; */
	int k, n, m, flag;
	int acharindex, bcharindex;

	alloccomposites(NCOMPOSITES);	/* initial allocation of space */
	charindex = 0;
/*	for (k = 0; k < MAXCOMPOSITES; k++) hit[k] = 0; */
	for (k = 0; k < maxcomposites; k++) hit[k] = 0;
	nhits = 0;

	bogusseac = 0;				/* how many not in SEAC */
/* 	for (k = 0; k < MAXCOMPOSITES; k++) charnames[k] = ""; */
	for (k = 0; k < maxcomposites; k++) charnames[k] = NULL; 

/*  CC Zcaron 2 ; PCC Z 0 0 ; PCC caron 139 212 ; */
	while ((n = getline(fp_in, line)) != 0) {
		if (*line == '%' || *line == ';' || *line < ' ') continue;
		if (strncmp(line, "CC ", 3) == 0) {
			flag = 1;
			m = sscanf(line, "CC %s 2 ; PCC %s 0 0 ; PCC %s %d %d ;", 
				charname, bcharname, acharname, &dx, &dy);
			if (m < 5) {
				fprintf(errout, "Don't understand %s", line);
				flag = 0;
			}
			if (strlen(charname) >= MAXCHARNAME) {
				fprintf(errout, "Character name %s too long (%d)\n",
						charname, strlen(charname));
				flag = 0;
			}
			else if (flag != 0) {
/*				strcpy(charnames[charindex], charname); */
				charnames[charindex] = zstrdup(charname);	/* 95/May/12 */
/*				if (charnames[charindex] == NULL) outofmemory(charname); */
				adx[charindex] = (short) dx;
				ady[charindex] = (short) dy;
				bcharindex = lookupstandard(bcharname);
				acharindex = lookupstandard(acharname);
/*				if (acharindex < 0 || bcharindex < 0) {
					fprintf(errout, 
						"Base or accent not in StandardEncoding %s", line);
					flag = 0;
				}
				else {
					achar[charindex] = acharindex;
					bchar[charindex] = bcharindex;					
				} */	/* simple pre 1995/May/8 */
				if (bcharindex < 0) {
					fprintf(errout, "Base not in ASE: %s", line);
					if (relaxseac) {		/* 1995/May/8 */
						bcharindex = lookupencoding(bcharname);
						if (bcharindex < 0) {
							fprintf(errout, 
							"ERROR: Base also not in encoding: %s", line);
						}
						else {
							if (adjust256) bcharindex += 256;
							else if (strcmp(standardencoding[bcharindex], "")
									 == 0); 
							else bcharindex = -1;	/* conflict ASE */
							bogusseac++;
						}
					}
				}
				if (acharindex < 0) {
					fprintf(errout,"Accent not in ASE: %s", line);
					if (relaxseac) {		/* 1995/May/8 */
						acharindex = lookupencoding(acharname);
						if (acharindex < 0) {
							fprintf(errout, 
							"ERROR: Accent also not in encoding: %s", line);
						}
						else {
							if (adjust256) acharindex += 256;
							else if (strcmp(standardencoding[acharindex], "")
									 == 0); 
							else acharindex = -1;	/* conflict ASE */
							bogusseac++;
						}
					}
				}
/* if both base and accent can be assigned a numeric code ... */
				if (acharindex >= 0 && bcharindex >= 0) {
					if (charindex >= maxcomposites) 
						realloccomposites(maxcomposites+maxcomposites);
					achar[charindex] = (short) acharindex;
					bchar[charindex] = (short) bcharindex;					
				}
				else flag = 0;
			}
			if (flag > 0) {			/* if we added an entry then increment */
/*				if (charindex++ >= MAXCOMPOSITES) { */
/*					fprintf(errout, "ERROR: Too many composite characters\n"); */
/*					return -1; */
				charindex++;
				if (dotsflag != 0) putc('.', stdout);
			}
		}
	}
	if (dotsflag != 0) putc('\n', stdout);
	if (traceflag != 0) {
		printf("Entries for %d composite characters found\n", charindex);
	}
	if (bogusseac > 0)
	printf("WARNING: composites based on %d glyph%s NOT in StandardEncoding\n",
			bogusseac, (bogusseac == 1) ? "" : "s");
	if (charindex == 0) {
		fprintf(errout, "ERROR: no composites found in %s\n", filename);
		return -1;
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	if (quietflag == 0) {
/*		printf("%s <pfa-file> [<composite-list>]\n", s); */
		printf("%s [-v][-S] -c=<composite-list> <pfa-file>\n", s);
		if (detailflag) {
		printf("\tv verbose mode\n");
		printf("\tS allow base and accent not in Adobe StandardEncoding\n");
		printf("\t  (must run resulting font through SAFESEAC afterwards)\n");
		printf("\tc list of new or modified composites (AFM file style)\n");
		}
	}
	exit(1);
}

void freeencoding(void) {			/* 1995/May/8 */
	int k;
	for (k = 0; k < MAXCHRS; k++) {
/*		if (strcmp(encoding[k], "") != 0)  */
		if (encoding[k] != NULL) {
			free(encoding[k]);
/*			encoding[k] = ""; */
			encoding[k] = NULL;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showencoding(void) {				/* debugging only */
	int k;
	for (k = 0; k < MAXCHRS; k++) {
/*		if (strcmp(encoding[k], "") != 0) */
		if (encoding[k] != NULL) {
			printf("%d %s\n", k, encoding[k]);
		}
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
/*		if (chr >= 0 && chr < MAXCHRS && strlen(s) < MAXCHARNAME) { */
		if (chr >= 0 && chr < MAXCHRS) {
/*			strcpy(encoding[chr], s); */
			encoding[chr] = zstrdup(s);		/* 95/May/8 */
/*			if (encoding[chr] == NULL) outofmemory(s); */
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
	
	freeencoding();
/*	needsaccent = 0; */			/* in case we pop out */
/*	for (k = 0; k < MAXCHRS; k++) *encoding[k] = '\0';	 */
/*	for (k = 0; k < MAXCHRS; k++) encoding[k] = ""; */	/* 95/May/8 */
	for (k = 0; k < MAXCHRS; k++) encoding[k] = NULL;	/* 95/May/8 */

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
/*			strcpy(encoding[k], standardencoding[k]); */
			encoding[k] = strdup(standardencoding[k]); 
/*			if (encoding[k] == NULL) outofmemory(standardencoding[k]); */
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
		for (;;) {
			k = rand() >> 8;
			if (k == '\n' || k == '\t' || k == '\r') break;
			if (k < ' ') k = k + 'A';
			if (k >= 128) k = k - 128;
			if (k >= ' ' && k < 128) break;
		}
		putc (k, fp);
	}
	fclose (fp);
}

void cleanup(void) {
	if (wipeclean == 0) return;
	if (fp_dec != NULL) {
		(void) fclose(fp_dec);						/* close output */
/*		fp_dec = fopen(fn_dec, "wb");		
		if (fp_dec != NULL) fclose(fp_dec); */
		wipefile (fn_dec);
		if (wipeclean > 0) (void) remove(fn_dec);	/* remove bogus file */
	}
	if (fp_pln != NULL) {
		(void) fclose(fp_pln);				/* close output */
/*		fp_pln = fopen(fn_pln, "wb");	
		if (fp_pln != NULL) fclose(fp_pln); */
		wipefile (fn_pln);
		if (wipeclean > 0) (void) remove(fn_pln); /* remove bogus file */
	}
	if (fp_adj != NULL) {
		(void) fclose(fp_adj);				/* close output */
/*		fp_adj = fopen(fn_adj, "wb");	
		if (fp_adj != NULL) fclose(fp_adj); */
		wipefile (fn_adj);
		if (wipeclean > 0) (void) remove(fn_adj); /* remove bogus file */
	}
}

void abortjob () {
	fprintf(stderr, "\nUser Interrupt - Exiting\n"); 
	cleanup(); 
 	if (renameflag != 0) (void) rename(fn_in, fn_bak);
/*	if (traceflag != 0) fprintf(errout, "\nUser Interrupt\n");  */
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
/* 	if (renameflag != 0) rename(fn_in, fn_bak); */
	if (bAbort++ >= 15)	exit(3);			/* emergency exit */
	(void) signal(SIGINT, ctrlbreak); 
}

int decodeflag (int c) {
	switch(c) { 
		case 'v': verboseflag = 1; dotsflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case '?': detailflag++; return 0;
		case 'd': wipeclean++; return 0;
		case 'S': relaxseac = 1 - relaxseac; return 0;	/* 95/May/8 */
		case 'H': adjust256 = 1 - adjust256; return 0;	/* 95/May/2 */
		case 'c': compositflag = 1; return -1;
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
}

int decodearg(char *command, char *next, int firstarg) {
	char *s;
	char *sarg=command;
/*	int c, n; */
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
			if (compositflag != 0) {
				compositfile = s;
				compositflag = 0; 
			}
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
	fprintf(errout, "HASHED %ld\n", hash);	
	(void) _getch(); 
	return hash;
}

char *progfunction="adds composite characters";

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);	 
/*	fprintf(output, "%s (%s %s)\n", programversion, date, compiletime); */
	fprintf(output, "%s - %s - version %s (%s %s)\n", 
		progname, progfunction,  progversion, date, compiletime);
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

int _cdecl main(int argc, char *argv[]) {       /* main program */
    FILE *fp_in;
	FILE *fp_out=NULL;
	FILE *fp_cmp;					/* 95/May/8 */
/*	unsigned int i; */
	int firstarg=1;
/*	unsigned int i; */
	int c, d, k, m;
	int errorflag=0;
	char *s;

	if (strlen(progname) < 16) strcpy(progname, compilefile);
	if ((s = strchr(progname, '.')) != NULL) *s = '\0';
	uppercase(progname);

	strcpy (line, argv[0]);
	uppercase (line);
	if (strstr(line, progname) == NULL) quietflag = 1;

	if (argc < firstarg+1) showusage(argv[0]); 

	firstarg = commandline(argc, argv, 1);

/*	while (argv[firstarg][0] == '-') { 
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else decodeflag(c);
		}
		firstarg++;
	} */

	if (wipeclean == 4) wipeclean = 0;	/* `ddd' on command line */

	if (argc <= firstarg) showusage(argv[0]); 
		
	if (quietflag != 0) {
		verboseflag = 0; traceflag = 0; dotsflag = 0;
	}

/*	scivilize(compiledate);	*/
	if (quietflag == 0) stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	(void) signal(SIGINT, ctrlbreak);

	if ((s = getenv("TMP")) != NULL) tempdir = s;
	if ((s = getenv("TEMP")) != NULL) tempdir = s;

/* printf("Composite character modification and addition program version %s\n",
		VERSION); */

	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

/*	for (k = 0; k < MAXCHRS; k++) encoding[k] = ""; */
	for (k = 0; k < MAXCHRS; k++) encoding[k] = NULL;

/*	for (k = 0; k < MAXCOMPOSITES; k++) charnames[k] = ""; */
	for (k = 0; k < maxcomposites; k++) charnames[k] = NULL;

	if (strcmp(compositfile, "") == 0) {
/*		following is for back-ward compatability 1994/June/2 */
		if (firstarg+1 < argc) {
			compositfile = argv[firstarg+1]; 
/*			compositfile = argv[argc-1]; */ /* ??? */
			if (strstr(compositfile, ".pfa") != NULL ||
				strstr(compositfile, ".PFA") != NULL) {
				compositfile="";	/* can't be composites file ! */
			}
			else argc--; 
/*			firstarg++; */
		}
	}

/* moved lower down so font encoding is known when reading this */
/* still need to look for file here, so can set extractflag ... */

	if (strcmp(compositfile, "") != 0) {	/* composites file given */
		strcpy(fn_cmp, compositfile);		/* get file name */
		extension(fn_cmp, "cmp");			/* extension is not given */
		if (traceflag != 0) printf("Reading file %s\n", fn_cmp); 
		if ((fp_cmp = fopen(fn_cmp, "r")) == NULL) {	/* exists ? */
			underscore(fn_cmp);				/* 1993/May/30 */
			if ((fp_cmp = fopen(fn_cmp, "r")) == NULL) {	/* exists ? */
				perror(fn_cmp);	exit(13);
			}
		}
/*		readcomposites (fp_cmp, fn_cmp); */ /* don't actually read it yet */
		fclose(fp_cmp);
		extractflag = 0;
		if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	}
	else extractflag = 1;

/*	m = firstarg; */
/* The looping over several PFA files won't work until we fix above */
	for (m = firstarg; m < argc; m++) {		
	strcpy(fn_in, argv[m]);					/* get file name */
	extension(fn_in, "pfa");

	if (verboseflag != 0) printf("Processing file %s\n", fn_in);

/*	if (traceflag) printf("firstarg %d m %d argc %d\n", firstarg, m, argc);*/

	if ((fp_in = fopen(fn_in, "rb")) == NULL) {	/* see whether exists */
		underscore(fn_in);						/* 1993/May/30 */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {	/* see whether exists */
			perror(fn_in);
/*			exit(13);  */
			errorflag=1;						/* 1995/May/12 */
			continue;
		}
	}
/*	else { */
		c = fgetc(fp_in); d = fgetc(fp_in);
		if (c != '%' || d != '!') {
			fprintf(errout, "ERROR: This does not appear to be a PFA file\n");
			errorflag=1;
			continue;							/* 1995/May/12 */			
		}
		fclose(fp_in);
/*	} */

	if ((s=strrchr(fn_in, '\\')) != NULL) s++;
	else if ((s=strrchr(fn_in, ':')) != NULL) s++;
	else s = fn_in;
	strcpy(fn_out, s);			/* copy input file name minus path */

	if (strcmp(fn_in, fn_out) == 0 && extractflag == 0) {
		strcpy(fn_bak, fn_in);
		forceexten(fn_in, "bak");
		printf("Renaming %s to %s\n", fn_bak, fn_in);
		remove(fn_in);		/* in case backup version already present */
		rename(fn_bak, fn_in);
		renameflag++;
	}
	
	if (verboseflag != 0 && extractflag == 0) 
		printf("Output is going to %s\n", fn_out);

/*	(void) tmpnam(fn_dec); */	/* create temporary file name */
/*	strcpy(fn_dec, fn_out); */
/*	makename(fn_dec, "dec"); */
	maketemporary(fn_dec, fn_out, "dec");
	if (traceflag != 0) printf("Using %s as temporary\n", fn_dec);
/*	strcpy(fn_pln, fn_out); */
/*	makename(fn_pln, "pln"); */
	maketemporary(fn_pln, fn_out, "pln");
	if (traceflag != 0) printf("Using %s as temporary\n", fn_pln);
	if (extractflag == 0) {
/*		strcpy(fn_adj, fn_out); */
/*		makename(fn_adj, "adj"); */
		maketemporary(fn_adj, fn_out, "adj");
		if (traceflag != 0) printf("Using %s as temporary\n", fn_adj);
	}

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
		underscore(fn_in);						/* 1993/May/30 */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
			perror(fn_in);
			exit(2);	/* should not happen at this point */
		}
	}
	if (relaxseac) {						/* 1995/May/8 */
		if (traceflag) printf("Looking for font encoding\n");
		getencoding(fp_in);
		rewind (fp_in);
	}

/*	the below code moved here so actual font encoding is known at this time */
	if (strcmp(compositfile, "") != 0) {	/* composites file given */
		strcpy(fn_cmp, compositfile);		/* get file name */
		extension(fn_cmp, "cmp");			/* extension if not given */
		if (verboseflag != 0) printf("Reading file %s\n", fn_cmp);
		if ((fp_cmp = fopen(fn_cmp, "r")) == NULL) {	/* exists ? */
			underscore(fn_cmp);				/* 1993/May/30 */
			if ((fp_cmp = fopen(fn_cmp, "r")) == NULL) {	/* exists ? */
				perror(fn_cmp);
				exit(13); 	/* should not happen at this point */
			}
		}

		if (readcomposites (fp_cmp, fn_cmp) != 0) { /* any composites ? */
			fclose(fp_cmp);
			fclose(fp_in);			/* 96/Apr/7 ??? */
			cleanup();
			if (renameflag != 0) {
				printf("Renaming %s to %s\n", fn_in, fn_bak);
/*				remove(fn_bak); */	/* in case version already present */
				rename(fn_in, fn_bak);
			}
			exit(1);						/* 95/May/12 */
		}
		extractflag = 0;
		fclose(fp_cmp);
		if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	}
	else extractflag = 1;
/*	the above code moved here so actual font encoding is known at this time */

	if (traceflag != 0) printf("Pass A (down) starting\n");			/* */

	if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
		perror(fn_dec);
		exit(2); 
	}

	eexecscan = 1;
	charscan = 0;  decodecharflag = 0;  charenflag = 0; binaryflag = 0;

	(void) decrypt(fp_dec, fp_in);		/* actually go do the work */

	fclose(fp_in);
	if (ferror(fp_dec) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec);	cleanup();	exit(2);
	}
	else fclose(fp_dec);

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass B (down) starting\n");			/* */
		
	if ((fp_dec = fopen(fn_dec, "rb")) == NULL) { 
		perror(fn_dec); cleanup(); exit(2);
	}
	if ((fp_pln = fopen(fn_pln, "wb")) == NULL) { 
		perror(fn_pln); cleanup(); exit(2);
	}

	eexecscan = 0;
	charscan = 1;  decodecharflag = 1;  charenflag = 1;  binaryflag = 1;

	(void) decrypt(fp_pln, fp_dec);		/* actually go do the work */

	fclose(fp_dec);
	if (ferror(fp_pln) != 0) {
		fprintf(errout, "Output error ");
		perror(fn_pln); cleanup();	exit(2);
	}
	else fclose(fp_pln);

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (debugflag != 0) _getch();
	
	if (extractflag != 0) {		/* no need to go upward afterward */
		if ((fp_pln = fopen(fn_pln, "rb")) == NULL) {
			perror(fn_pln); cleanup(); exit(3);
		}
		forceexten(fn_out, "cmp");
		if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
			perror(fn_out); cleanup();	exit(2);
		}

		if (verboseflag != 0) printf("Pass 0\n");
		(void) extractcompo(fp_out, fp_pln, 0, 1);	/* extract info */
		rewind(fp_pln);
		if (verboseflag != 0) printf("Pass 1\n");
		(void) extractcompo(fp_out, fp_pln, 1, 1);	/* extract composites */

		fclose(fp_pln);
/*		if (wipeclean > 0) remove(fn_pln);  */
		if (ferror(fp_out) != 0) {
			fprintf(stderr, "Output error ");
			perror(fn_out); cleanup(); exit(2);
		}
		else {
			fclose(fp_out);
			fp_out = NULL;
		}
		cleanup();
/*		return 0; */
		continue;
	} /* end of if extractflag != 0 */

	if (traceflag != 0) 
		printf("Adjustment and insertion of composites starting\n"); 

	if ((fp_pln = fopen(fn_pln, "rb")) == NULL) { 
		printf("FN_PLN: `%s'\n", fn_pln);	/* debugging only */
		perror(fn_pln);  cleanup(); exit(2);
	}
	if ((fp_adj = fopen(fn_adj, "wb")) == NULL) { 
		printf("FN_ADJ: `%s'\n", fn_adj);	/* debugging only */
		perror(fn_adj); cleanup(); exit(2);
	}

	if (verboseflag != 0) printf("Pass 0\n");
/*	if (fp_out == NULL) fprintf(errout, "fp_out == NULL\n"); */
	(void) extractcompo(fp_out, fp_pln, 0, 1);	/* extract info first */
	rewind(fp_pln);
/*	if (verboseflag != 0) */
printf("PFA file has %d composites, CMP file has %d composites, %d overlap\n",
		ncomposites, charindex, nhits);
	if (verboseflag != 0) printf("Pass 1\n");	/* now modify and add */
	(void) extractcompo(fp_adj, fp_pln, 1, 0);
		
	fclose(fp_pln);
/*	if (wipeclean > 0) remove(fn_pln);  */
	if (ferror(fp_adj) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_adj); cleanup(); exit(2);
	}
	else fclose(fp_adj);

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	
	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass B (up) starting\n"); 

	if ((fp_adj = fopen(fn_adj, "rb")) == NULL) { 
		perror(fn_adj);  cleanup(); exit(2);
	}
	if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
		perror(fn_dec); cleanup(); exit(2);
	}

	eexecscan = 1; charenflag = 1; charscan = 1;

	(void) encrypt(fp_dec, fp_adj);	/* actually go do the work */
		
	fclose(fp_adj);
/*	if (wipeclean > 0) remove(fn_adj);  */
	if (ferror(fp_dec) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec); cleanup(); exit(2);
	}
	else fclose(fp_dec);

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	
	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass A (up) starting\n"); 

	if ((fp_dec = fopen(fn_dec, "rb")) == NULL) { 
		perror(fn_out); cleanup();	exit(2);
	}
	if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
		perror(fn_out); cleanup();	exit(2);
	}

	eexecscan = 1; charenflag = 0; charscan = 0;

	(void) encrypt(fp_out, fp_dec);	/* actually go do the work */
		
	fclose(fp_dec);

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (debugflag != 0) _getch();

/*	if (wipeclean > 0) remove(fn_dec);  */
	if (ferror(fp_out) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_out); cleanup();	exit(2);
	}
	else fclose(fp_out);

	cleanup();
	freeencoding();						/* just for fun 95/May/8 */
	freecomposites();
	if (bogusseac > 0) {
	printf("WARNING: Using base and accent characters not in Adobe StandardEncoding\n");
	printf("         Need to run SAFESEAC to make this font work properly\n");
	}
	if (verboseflag) printf("Output in %s\n", fn_out);
/*	Only allow operation on multiple files when extracting info */
/*	Not when adding/modifying composites */
	if (strcmp(compositfile, "") != 0) break;

	}	/* end of for loop over PFA files on command line 95/May/12 */

	freecharnames();					/* just for fun 95/May/12 */
	if (errorflag == 0) return 0;
	else exit(errorflag);
}	

/* does not deal with `sbw' form of metric info */

/* does not deal properly with `fractional widths' */
