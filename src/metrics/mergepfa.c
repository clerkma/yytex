/* mergepfa.c
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

#include "metrics.h"

#ifdef _WIN32
#define __far 
#define _fmalloc malloc
#define _frealloc realloc
#define _ffree free
#endif

#define MAXSYNOM 16

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int debugflag = 0;
int wipeclean = 1;	
int dotsflag = 1;
int quietflag = 0;
/* int fixfloating = 1; */	/* fix floating point font level hints */
int fixfloating = 0;		/* do not fix floating point 95/Mar/31 */

int renameflag = 0;
int spaceflag = 0;

int spaceneeded=0;		/* non-zero if space is needed */
int spacewidth=0;		/* specified width of space */

int eexecscan = 0;		/* scan up to eexec before starting */
int charscan = 0;		/* scan up to RD before starting */
int charenflag = 0;		/* non-zero charstring coding instead of eexec */
int decodecharflag = 0;	/* non-zero means also decode charstring */
int binaryflag = 0;		/* input is binary, not hexadecimal */

int singleargument = 0;	/* non-zero => no merging, single file */
int writeoutflag = 1;	/* zero => extract info only - debugging */
int forceoutname = 1;	/* don't use input file name for output file name */
int fixuniqueid = 1;	/* fix mismatching UniqueIDs if non-zero */

int allowlowswitch=1;	/* allow following to happen if first font Subr 4 */

int uselowswitch=0;		/* Call Subr 4 from second font for hint switch */
						/* rather than copy of Subr 4 from second font */

FILE *errout=stdout;	/* 97/Sep/28 */

double DefBlueScale=0.039625;	/* from the black/white bible 94/Nov/12 */

volatile int bAbort=0;			/* set when user types control-C */

/* char *outfilename = "merged.pfa"; */

extern int encrypt(FILE *, FILE *);

extern int decrypt(FILE *, FILE *);

char *tempdir="";		/* place to put temporary files */

char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
char fn_bak[FILENAME_MAX], fn_des[FILENAME_MAX];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int ncharstrings;		/* number of CharStrings in original PFA file */

int maxchars=0;

/* char stringspace[MAXSTRINGSPACE]; */	/* Place to get string allocations */

/* char *charnames[MAXCHARINFONT]; */	/*  CharString charname */

// char * __far *charnames=NULL;	/*  CharString charname */

char * *charnames=NULL;	/*  CharString charname ??? */

/* charnames holds the names of characters in the first font */
/* used only in determining duplications */

/**************************************************************************/

int ncharstrings1, ncharstrings2;

int nsubrs;				/* number of Subrs in original PFA file */

int nsubrs1, nsubrs2;

int ncharhits;			/* how many characters given in desired file */

int nwanted;			/* number of characters seen in desired file */

int charindex = 0;

int charseen = 0;		/* non-zero after character string RD and ND seen */
int subrseen = 0;		/* non-zero after subrs NP seen */
int nextend = 0;		/* non-zero if next line is ND */
int othersubrseen = 0;	/* non-zero if OtherSubrs seen */
int notdefseen = 0;		/* saw .notdef in this font */
int switchtrick = 0;	/* non-zero if Subr 4 used for hint replacement */

int notdefseen1 = 0;
int notdefseen2 = 0;

int subrseen1 = 0;
int subrseen2 = 0;

int othersubrseen1 = 0;
int othersubrseen2 = 0;

int switchtrick1 = 0;
int switchtrick2 = 0;

int needsubrs = 0;		/* non-zero if have to add in Subrs */

int needothersubrs = 0;	/* non-zero if have to add in Othersubrs */

char rdsynom[MAXSYNOM] = "RD";	/* RD or -| or ... */
char ndsynom[MAXSYNOM] = "ND";	/* ND or |- or `noaccess def' */
char npsynom[MAXSYNOM] = "NP";	/* NP or | or  `readonly put' */

char rdsynom1[MAXSYNOM] = "RD";	/* RD or -| or ... */
char ndsynom1[MAXSYNOM] = "ND";	/* ND or |- or `noaccess def' */
char npsynom1[MAXSYNOM] = "NP";	/* NP or | or  `readonly put' */

char rdsynom2[MAXSYNOM] = "RD";	/* RD or -| or ... */
char ndsynom2[MAXSYNOM] = "ND";	/* ND or |- or `noaccess def' */
char npsynom2[MAXSYNOM] = "NP";	/* NP or | or  `readonly `put' */

/* `noaccess def' could also be `readonly def' */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

int wantcpyrght=1;

/* Font merging and normalizing program version */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* char *programversion = "MERGEPFA merging and normalizing - version 1.0"; */ 

/* char *progname="MERGEPFA"; */

char progname[16]=""; 

/* char *progversion="1.0"; */
/* char *progversion="1.1"; */
/* char *progversion="1.2"; */
char *progversion="2.0";	/* 98/Dec/25 */

char *copyright = "\
Copyright (C) 1992-1998  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1992--1994  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 2670755 */
/* #define COPYHASH 13986445 */
/* #define COPYHASH 5618236 */
/* #define COPYHASH 2226059 */
#define COPYHASH 3956063

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Other Subrs needed for hint replacement only */

static char othersubrs[] = "\
/OtherSubrs[{}{}{}\n\
{systemdict/internaldict known not\n\
	{pop 3}\n\
	{1183615869 systemdict/internaldict get exec\n\
		dup/startlock known\n\
			{/startlock get exec}\n\
			{dup/strtlck known\n\
				{/strtlck get exec}\n\
				{pop 3}\n\
				ifelse}\n\
			ifelse}\n\
		ifelse\n\
	}executeonly\n\
] noaccess def\n";	/* ND */

/***********************************************************************/

void freecharnames(void) {
	int k;
	if (charnames == NULL) return;
	for (k = 0; k < maxchars; k++) {
		if (charnames[k] != NULL) free(charnames[k]);
			charnames[k] = NULL;
	}
}

void freechars(void) {
	if (maxchars > 0) freecharnames();
	if (charnames != NULL) free(charnames);
	charnames = NULL;
	maxchars = 0;
}

void allocchars(int nchars) {
	int k;
	if (maxchars > 0) freechars();
	maxchars = 0;
	charindex = 0;
//	if (verboseflag)
		printf("Allocating space for %d chars\n", nchars);
//	charnames = (char * __far *) _fmalloc(nchars * sizeof(char *));
	charnames = (char * *) malloc(nchars * sizeof(char *));
	if (charnames == NULL){
		fprintf(errout, "ERROR: failed memory allocation of %d bytes\n",
				nchars * sizeof(char *));
		exit(1);
	}
//	fprintf(errout, "charnames at %08X\n", charnames);
	for (k = maxchars; k < nchars; k++) charnames[k] = NULL;
	maxchars = nchars;
}

void reallocchars(int nchars) {
	int k;
//	if (verboseflag)
		printf("Reallocating space for %d => %d chars\n", maxchars, nchars);
//	charnames = (char * __far *) _frealloc(charnames, nchars * sizeof(char *));
	charnames = (char * *) realloc(charnames, nchars * sizeof(char *));
	if (charnames == NULL){
		fprintf(errout, "ERROR: failed memory allocation of %d bytes\n",
				nchars * sizeof(char *));
		exit(1);
	}
//	fprintf(errout, "charnames at %08X\n", charnames);
	for (k = maxchars; k < nchars; k++) charnames[k] = NULL;
	maxchars = nchars;
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

static char line[MAXLINE];		/* buffer for getline */

static int getline(FILE *fp_in, char *line) {
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	return strlen(line); 
} 

int lookup(char *name) {
	int k, flag = -1;
//	fprintf(errout, "name %s (%08X) charindex %d\n", name, name, charindex);
//	fprintf(errout, "charnames at %08X\n", charnames);
//	fflush(errout);
	if (charindex == 0) return flag;
	for (k = 0; k < charindex; k++) {
/*		if (strcmp(charnames[k], "") == 0) continue; */
//		fprintf(errout, "charnames[%d] is %s (%08X)\n", k, charnames[k], charnames[k]);
//		fflush(errout);
		if (charnames[k] == NULL) continue;
		if (strcmp(charnames[k], name) == 0) {
			flag = k;
			break;
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

#ifdef IGNORED
char *strdupx(char *name) {	/* our own cheap _strdup */
	char *s;
	int n = strlen(name) + 1;

	if (stringindex + n >= MAXSTRINGSPACE) {
		fprintf(stderr, "ERROR: Out of string pool space for character names\n");
		exit(1);
	}		
	s = stringspace + stringindex;
	strcpy (s, name);
	stringindex += n;
	return s;
}
#endif

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

/* RD */ /* ND */ /* NP */ /* noaccess put */ /* noaccess def */ 

void copysynom (char *r, char *s) {		/* copy string to end of line */
	int n=MAXSYNOM;
	while (*s <= ' ' && *s != '\0') s++;	/* skip initial white space */
	while (*s >= ' ' && *s != '\0' && --n > 0)
		*r++ = *s++;						/* copy to end of line */
	*r = '\0';
}

/* scan plain file to extract various useful bits of information */

//	stage == 0 for base font
//	stage == 1 for add on font

void extractcharnames(FILE *fp_in, int stage) {
	char charname[CHARNAME_MAX];
	int subrnum, subrlen;
	char rdsubr[MAXSYNOM];
	int c, k, n;
/*	int ns; */				/* 1993/Dec/14 */
	int duplicates=0;
	char *s;
	long fileloc;

/*	for (k = 0; k < MAXCHARINFONT; k++)	charnames[k] = NULL; */
/*	for (k = 0; k < maxchars; k++)	charnames[k] = NULL; */ // ???

	charseen = 0; subrseen = 0; 
	othersubrseen = 0; notdefseen = 0;
	switchtrick = 0;

/*  scan up to eexec encrypted part */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
	}

/*	scan up to CharStrings */
	while ((n = getline(fp_in, line)) != 0 && 
		(s = strstr(line, "/CharStrings")) == NULL) {
/*	see if can find Subrs first */
		if ((s = strstr(line, "/OtherSubrs")) != NULL)
			othersubrseen++;
		if ((s = strstr(line, "/Subrs ")) != NULL) {
			sscanf (s + 7, "%d", &nsubrs);
			if (traceflag != 0) fputs(line, stdout);
/*	scan the Subrs now */
			while ((n = getline(fp_in, line)) != 0 &&
				(s = strstr(line, "/CharStrings")) == NULL) {
/* beginning of next Subr ? */
				if (strncmp(line, "dup ", 4) == 0) {
					sscanf (line, "dup %d %d %s", &subrnum, &subrlen, rdsubr);
/*	 leave this alone, since RD synonym precedes binary stuff */
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
					while ((n = getline(fp_in, line)) != 0 &&
						(s = strstr(line, "/CharStrings")) == NULL) {
						if (*line < ' ') break;		/* blank line */
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
					}
/* hit end of a Subr */ /* check if next line is blank */
/*					c = getc(fp_in); ungetc(c, fp_in); */
					while ((c = getc(fp_in)) == ' ') ; /* 1993/Jan/4 */
					ungetc(c, fp_in);
					if (c > ' ') {
/*						printf("End of Surbs on `%c'\n", c); */	/* debug */
						break;	/* end of Subrs */
					}	/* c <= ' ' */
				}		/* does not start with `dup ' */
			}			/* seen /CharString */
			break;
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}

/*	if we popped out at end of Subrs from the above */
	while ((s = strstr(line, "/CharStrings")) == NULL) {
		if ((n = getline(fp_in, line)) == 0) break;
		if (traceflag != 0) fputs(line, stdout);
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
		
/*	if (traceflag != 0) fputs(line, stdout); */
	sscanf(s + 13, "%d%n", &ncharstrings, &n);
//	if (maxchars != ncharstrings) {
	if (stage == 0) {
		fprintf(errout, "%s\n", s);
		allocchars(ncharstrings);
	}
/*	scan all of the CharStrings */ /* remember the names */
	for (;;) {
		n = getline(fp_in, line);
		if (n <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;
		if (charseen == 0 && nextend != 0) {
/*			s = line; */ 		/* deal with leading white space */
/*			while (*s == ' ') s++; */
/*			sscanf (s, "%s\n", ndsynom); */
/*			stripreturn(ndsynom); */
			copysynom(ndsynom, line);		/* 1993/Dec/13 */
/*	 need to deal with embedded white space possibility */
			if (traceflag != 0) printf("NDsynonym: `%s'\n", ndsynom);
			charseen = 1;
		}
		if (strstr(line, "endchar") != NULL) nextend = 1; else nextend = 0;
		if ((s = strchr(line, '/')) != NULL) {
			if (charseen == 0) {
/*				sscanf(s, "/%s %d %s\n", charname, &k, rdsynom);  */
				sscanf(s, "/%s %d %s", charname, &k, rdsynom); 
				stripreturn(rdsynom);					/* ??? */
/*	 leave this alone, since RD synonym precedes binary stuff */
				if (traceflag != 0) printf("RDsynonym: `%s'\n", rdsynom);
			}
			sscanf(s, "/%s ", charname);
			if (strcmp(charname, ".notdef") == 0) notdefseen++;
			if (stage > 0) {		// complain about duplicate chars
				k = lookup(charname);
				if (k >= 0) {
					fprintf(errout, "%s? ", charname);
					duplicates++;
				}
			}
			else {	/* stage == 0 */
/*				strcpy(charnames[charindex], charname); */
/*				charnames[charindex] = strdupx(charname); */	/* 94/June/6 */
				if (charindex >= maxchars) {
					fprintf(errout, "charindex %d > maxchars %d\n", charindex, maxchars);
					reallocchars(maxchars+maxchars);
				}
				if (charnames[charindex] != NULL) {
					fprintf(errout, "charnames[%d] = %08X\n",
							charindex, charnames[charindex]);
					exit(1);
				}
				charnames[charindex] = zstrdup(charname);	/* 94/June/6 */
//				fprintf(errout, "charnames[%d] = %s (%08X)\n",
//						charindex, charnames[charindex], charnames[charindex]);
//				fflush(errout);
				charindex++;

/*				if (charindex++ > MAXCHARINFONT) {
					fprintf(stderr, "ERROR: Too many characters in font\n");
					charindex--;
				} */
			}
			if (dotsflag != 0) putc('.', stdout);
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}		
	if (duplicates > 0) {
		fprintf(errout, 
			"\nWARNING:  Above %d character%s appear%s in both fonts\n",
			   duplicates,
				(duplicates > 1) ? "s" : "",
				(duplicates > 1) ? "" : "s");				
	}
	if (dotsflag != 0) putc('\n', stdout);	
}

/* does the following handle the old fashioned hint switching calls ? */
/* n 1 3 callothersubr pop callsubr */ /* handler in adjustothersubr */

void adjustsubrnum(char *line) {
	int n, na, nb, nc, nd;

	n = sscanf (line, "%d %d %d %d", &na, &nb, &nc, &nd);
/*  n == 0 probably is callsubr following callothersubr hint replacement */
	if (n == 0) {	/* nothing to do */
	}
/*  n == 1 probably is proper normal Subrs call - replace if > 3 */
	else if (n == 1) {
		if (na > 3)
			sprintf(line, "%d callsubr\n", na + nsubrs1);
		else if (na == 1 || na == 2) ;	/* OK, FlxProc calls - 94/Jun/21 */
		else printf("WARNING:  Possibly conflicting use of low Subr: %s",
			line);				/* don't expect to see 0 or 3 here ... */
	}
/*  n == 2 probably is new trick to reduce verbage in hint replacement */
/*	else if (n == 2 && nb == 4) { */
	else if (n == 2) {
		if (nb == 4) {
			if (na > 3) {
/*	if Subr 4 is set up for this, use it rather than higher version */
/*	this only works if the first font uses Subr 4 for hint switching */
				if (uselowswitch != 0)
					sprintf(line, "%d %d callsubr\n", na + nsubrs1, nb);
				else sprintf(line, "%d %d callsubr\n", na + nsubrs1, nb + nsubrs1);
			}
			else printf("WARNING:  Apparently conflicting use of low Subr: %s",
				line);
		}
		else printf("WARNING:  Unusual use of low Subr: %s", line);
	}
	else if (n == 4) {			/* new 94/June/21 */
/*	do nothing if n == 4  ffff xxx yyy 0 callsubr FlxProc call */
		if (nd == 0) ;	/* if just final FlxProc call */
		else if (strstr(line, "div") != NULL) ; /* probably OK also ... */
		else printf("WARNING:  Unusual form of Subr call: %s", line);
	}
	else if (traceflag != 0) printf("n = %d line: %s", n, line);
}

/* remove floating point numbers */ /* not used anymore 95/Mar/31 */

void fixfloat(char *line) {
	char *s, *t;
	int c;

/*	keep going until no more decimal points left in line */
	while ((s = strchr(line, '.')) != NULL) {
		t = s + 1;
/*	search for end of number */
		c = *t++;
		while (c >= '0' && c <= '9') c = *t++;
/* shift stuff down to cover up decimal point and fractional part */
		t--;
		while ((c = *t++) != 0) *s++ =(char) c;
		*s = '\0';
	}
}

void adjustothersubr(char *line) {
	int n, na, nb, nc, nd;

	n = sscanf (line, "%d %d %d %d", &na, &nb, &nc, &nd);

/*  n == 2 probably is call in hint replacement boiler plate Subrs 0 - 3 */
/*  n == 3 probably is hint replacement call */
	if (n == 3 && nb == 1 && nc == 3) {
		sprintf(line, "%d %d %d callothersubr\n", na + nsubrs1, nb, nc);
	}
	else if (traceflag != 0) printf("n = %d line: %s", n, line);
}

void insertsubrs(FILE *output) {
	fprintf(output, "/Subrs %d array\n", 4 + nsubrs2);
	fprintf(output, "dup 0 15 %s\n", rdsynom1);
	fputs("3 0 callothersubr\n", output);
	fputs("pop\n", output);
	fputs("pop\n", output);
	fputs("setcurrentpoint\n", output);
	fputs("return\n", output);
/*	fprintf(output, "%s\n", npsynom1); */
	fputs("noaccess put\n", output);
	putc('\n', output);
	fprintf(output, "dup 1 9 %s\n", rdsynom1);	
	fputs("0 1 callothersubr\n", output);
	fputs("return\n", output);
/*	fprintf(output, "%s\n", npsynom1); */
	fputs("noaccess put\n", output);
	putc('\n', output);
	fprintf(output, "dup 2 9 %s\n", rdsynom1);	
	fputs("0 2 callothersubr\n", output);
	fputs("return\n", output);
/*	fprintf(output, "%s\n", npsynom1); */
	fputs("noaccess put\n", output);
	putc('\n', output);
	fprintf(output, "dup 3 5 %s\n", rdsynom1);	
	fputs("return\n", output);
/*	fprintf(output, "%s\n", npsynom1); */
	fputs("noaccess put\n", output);
	putc('\n', output);
	if (nsubrs2 == 0) {
/*		fputs(output, "%s\n", ndsynom1); */ /* safe ? */
/*		fputs("noacces def\n", output);	 */ /* BUG */
		fputs("noaccess def\n", output);	/* Hilmar fix 95/Apr/3 */
/*		fputs("end noaccess put\n", output); */ /* other form of code ? */
	}
}

int mergepln(FILE *fp_out, FILE *fp_pln1, FILE *fp_pln2) {
	char buffer[MAXLINE];
	char charname[CHARNAME_MAX];
	char rdsubr[MAXSYNOM];
	char npsubr[MAXSYNOM];	
	int c, n, m;
/*	int na, nb, nc, nd; */
	int flag, inside;
	int subrnum, subrlen, charnum, nprivy;
	char *s=NULL;
	long uniqueid1=0, uniqueid2=0;
	long len1, len2;
	double BlueScale;
	
	if (traceflag != 0) printf("Starting the Merge\n");

	if (traceflag != 0 && singleargument == 0) {
		fseek (fp_pln1, 0, SEEK_END);	len1 = ftell(fp_pln1);
		fseek (fp_pln2, 0, SEEK_END);	len2 = ftell(fp_pln2);	
		printf("FILE 1 %ld FILE 2 %ld\n", len1, len2);
		rewind(fp_pln1); rewind(fp_pln2);
	} 

	if (traceflag != 0) printf("Scan up to eexec file 1\n");
/*  scan up to eexec encrypted part in FILE 1 */
	while ((n = getline(fp_pln1, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
		if ((s = strstr(line, "/UniqueID")) != NULL) {
			if (sscanf(s, "/UniqueID %ld", &uniqueid1) > 0) {
/*				printf("UNIQUEID1 %ld\n", uniqueid1); */	/* debugging */
			}
		}
/*		if (traceflag != 0) fputs(line, stdout); */
		fputs(line, fp_out);
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
	fputs(line, fp_out);
	if (traceflag != 0) fputs(line, stdout);

	if (singleargument == 0) {
	if (traceflag != 0) printf("Scan up to eexec file 2\n");
/*  scan up to eexec encrypted part in FILE 2 */
	while ((n = getline(fp_pln2, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
/*		if (traceflag != 0) fputs(line, stdout); */
/*		fputs(line, fp_out); */
	}
/*	fputs(line, fp_out);	 */
	if (traceflag != 0) fputs(line, stdout);
	}

	if (traceflag != 0) printf("Scan up to Subrs file 1\n");
/*	scan up to SUBRS in FILE 1 */
	while ((n = getline(fp_pln1, line)) != 0 && 
		(s = strstr(line, "/Subrs")) == NULL) {
		if (strstr(line, "/CharStrings") != NULL) break;	 /* 1992/Jan/5 */
/*		if (needothersubrs != 0 && */
		if ((needothersubrs != 0 || needsubrs != 0) &&
			(s = strstr(line, "/Private")) != NULL) {
			sscanf(s, "/Private %d%n", &nprivy, &n);
			strcpy(buffer, s+n);
/*	need to enlarge /Private dictionary */
			if (needothersubrs != 0) nprivy++;
			if (needsubrs != 0) nprivy++;
			sprintf(s, "/Private %d%s", nprivy, buffer);
		}
		if (strstr(line, "/BlueValues") != NULL ||
			strstr(line, "/OtherBlues") != NULL ||
			strstr(line, "/FamilyBlues") != NULL ||
			strstr(line, "/FamilyOtherBlues") != NULL ||
/* following added 94/Nov/12 for Fontographer 4.0.4 */
			strstr(line, "/StdHW") != NULL ||
			strstr(line, "/StdVW") != NULL ||
			strstr(line, "/StemSnapH") != NULL ||
			strstr(line, "/StemSnapV") != NULL) {
/*			if (strchr(line, '.') != NULL) { */
			if (fixfloating && strchr(line, '.') != NULL) {
/*				fprintf(errout, "WARNING: floating point blue values:\n"); */
				fprintf(errout, "WARNING:  Floating point character level hints:\n");
				fputs(line, errout);
				fixfloat(line);
				fprintf(errout, "          Truncating floating point to integers:\n");
				fputs(line, errout);
			}
		}
		if ((s = strstr(line, "/UniqueID")) != NULL) {
			sscanf(s, "/UniqueID %ld%n", &uniqueid2, &n);
/*			printf("UNIQUEID2 %ld\n", uniqueid2); */	/* debug */
			if (fixuniqueid != 0 && uniqueid1 != uniqueid2) {
				fprintf(errout, "WARNING:  Mismatch in UniqueID %ld %ld\n",
					uniqueid1, uniqueid2);
				fprintf(errout, "Will set encrypted UniqueID to %ld\n",
					uniqueid1);
				strcpy(buffer, s+n);
				sprintf(s, "/UniqueID %ld%s", uniqueid1, buffer);
			}
		}
		if (strstr(line, "/BlueScale") != NULL) {	/* 94/Nov/12 */
			if (fixfloating) {
			if (sscanf(line, "/BlueScale %lg", &BlueScale) == 1) {
				if (BlueScale > 1.0 || BlueScale < 0.0) {
fprintf(errout, "WARNING:  Ridiculously large value of BlueScale: %lg\n",
	BlueScale);
fprintf(errout, "          Replacing with default: %lg\n", DefBlueScale);
					sprintf(line, "/BlueScale %lg def\n", DefBlueScale);
				}
			}
			else fprintf(errout, "Don't understand %s", line);
			}
		}
		fputs(line, fp_out);
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}

	if (n == 0) {
		fprintf(stderr, "Premature EOF 1\n");
		return -1;
	}

	if (needothersubrs != 0) {
		if (verboseflag != 0) printf("Inserting OtherSubrs\n");
/*		fprintf(fp_out, "%s", othersubrs); */
		fputs(othersubrs, fp_out);	
	}
	if (needsubrs != 0) {
		if (verboseflag != 0) printf("Inserting Subrs\n");
		insertsubrs(fp_out);
	}

/*	modify /Subrs line - up the total count nsubrs1 + nsubrs2 */
/*	if (nsubrs2 > 0) { */
	if (s == NULL) fprintf(errout, "s == NULL\n");
	if (nsubrs2 > 0 && needsubrs == 0) {
		if (sscanf (s, "/Subrs %d%n", &nsubrs, &n) == 0) fputs(line, errout);
		strcpy(buffer, s+n);
		sprintf(s, "/Subrs %d%s", nsubrs1+nsubrs2, buffer);
	}
/* avoid duplicate copy of CharString line if no Subrs 1993/Jan/5 */
	if (strstr(line, "/Subrs") != NULL)	{
		fputs(line, fp_out);
		if (traceflag != 0) fputs(line, stdout);
	}

	if (singleargument == 0) {
	if (traceflag != 0) printf("Scan up to Subrs file 2\n");
/*	scan up to SUBRS in FILE 2 */
	while ((n = getline(fp_pln2, line)) != 0 && 
		(s = strstr(line, "/Subrs")) == NULL) {
		if (strstr(line, "/CharStrings") != NULL) break;	 /* 1992/Jan/5 */
/*		fputs(line, fp_out); */
	}
	if (traceflag != 0) fputs(line, stdout);

	if (n == 0) {
		fprintf(stderr, "Premature EOF 2\n");
		return -1;
	}
	}

	if (traceflag != 0) printf("Copy Subrs from file 1\n");
/*	Now copy across SUBRS from FILE 1 */
	
	while ((s = strstr(line, "/CharStrings")) == NULL) {	/* 1992/Jan/5 */
		if ((n = getline(fp_pln1, line)) == 0) break;
		if ((s = strstr(line, "/CharStrings")) != NULL) break;
		if (strncmp(line, "dup ", 4) == 0) {
/*			if (traceflag != 0) fputs(line, stdout); */
		}
		s = line;					/* deal with leading white space */
		while (*s == ' ') s++;
		m = strcspn(s, " \n\r\t\f");
		if (strstr(s, "noaccess def") != NULL) {	/* 1993/Jan/4 */
			if (traceflag != 0) printf("BREAKING OUT ON %s", line);
			break;
		}
		if (strstr(s, "readonly def") != NULL) {	/* 1993/Nov/12 */
			if (traceflag != 0) printf("BREAKING OUT ON %s", line);
			break;
		}
		if (m == (int) strlen(ndsynom1) && 
			strncmp(s, ndsynom1, m) == 0) {
			if (traceflag != 0) printf("BREAKING OUT ON %s", line);
			break;
		}
		fputs(line, fp_out);
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
	strcpy(buffer, line);	/* save terminating line from FILE 1 */

/*	putc('\n', fp_out); */		/* separator between SUBRS */

	if (singleargument == 0) {
		if (traceflag != 0) printf("Copy Subrs from file 2\n");
		inside = 0;				/* for first time into Subrs */
/*		Now copy across SUBRS from FILE 2	  ---	 and renumber them */
		while ((n = getline(fp_pln2, line)) != 0 && 
			(s = strstr(line, "/CharStrings")) == NULL) {
			if (strncmp(line, "dup ", 4) == 0) {
				if (sscanf (line, "dup %d %d %s", 
					&subrnum, &subrlen, rdsubr) < 3) { 
					fputs(line, errout);
				}
/*	 leave this alone, since RD synonym precedes binary stuff */
/* adjust Subrs number */
				if (strcmp(rdsynom2, rdsubr) != 0) {
					fprintf(errout, 
						"Apparent mismatch in RD (subrs 2) %s %s: %s",
							rdsubr, rdsynom2, line);
				}
				sprintf(line, "dup %d %d %s\n", 
/*					subrnum + nsubrs1, subrlen, rdsubr); */
					subrnum + nsubrs1, subrlen, rdsynom1);
				fputs(line, fp_out);
/*				if (traceflag != 0) fputs(line, stdout);  */
/* scan this Subr */ /* assumed to end with `return' followed by NP */
				flag = 0;
				while ((n = getline(fp_pln2, line)) != 0 &&
					(s = strstr(line, "/CharStrings")) == NULL) {
					if (*line < ' ') { 	 /* blank line separating Subrs */
						fputs(line, fp_out);
						break;
					}
					if (strncmp(line, "return", 6) == 0) {
/*	read line after `return' */
						fputs(line, fp_out);
						n = getline(fp_pln2, line);
/*						s = line;
						while (*s == ' ') s++; 
						if (sscanf (s, "%s", rdsubr) < 1) { 
							fputs(line, errout);
						} */
						copysynom(npsubr, line);		/* 1993/Dec/14 */
/*	 need to deal with embedded white space possibility */
						if (strcmp(npsynom2, npsubr) != 0) {
							fprintf(errout, 
								"Apparent mismatch in NP (subrs 2) `%s' `%s': %s\n",
									npsubr, npsynom2, line);
						}
						sprintf(line, "%s\n", npsynom1);	/* replace */
						fputs(line, fp_out);
						c = getc(fp_pln2);
						while (c == ' ') c = getc(fp_pln2); /* 1992/Dec/9 */
						ungetc(c, fp_pln2);
						if (c > ' ') {
							flag = 1;
							break;			/* probably end of Subrs */
						}
					}
					else {					/* not `return' */
						if ((s = strstr(line, "callsubr")) != NULL) 
							adjustsubrnum(line);
						if ((s = strstr(line, "callothersubr")) != NULL &&
							subrnum > 3)	/* don't both boiler plate code */
							adjustothersubr(line);
							fputs(line, fp_out);
					}
				}

/* hit end of a particular Subr */ /* check if next line is blank */
				if (flag != 0) {
					if (traceflag != 0) printf("BREAKING OUT AFTER %s", line);
					break;
				}
				s = line;					/* deal with leading white space */
				while (*s == ' ') s++;
				m = strcspn(s, " \n\r\t\f");
				if (strstr(s, "noaccess def") != NULL) {	/* 1993/Jan/4 */
					if (traceflag != 0) printf("BREAKING OUT ON %s", line);
					break;
				}
				if (strstr(s, "readonly def") != NULL) {	/* 1993/Nov/12 */
					if (traceflag != 0) printf("BREAKING OUT ON %s", line);
					break;
				}
				if (m == (int) strlen(ndsynom2) &&
					strncmp(s, ndsynom2, m) == 0) {
					if (traceflag != 0) printf("BREAKING OUT ON %s", line);
					break;
				}
			}	/* end of `dup' case */
			else {
/* ??? should check whether end of Subrs here ??? */
/*				fputs(line, fp_out); */
/* new experiment 1993/Feb/4 */
				s = line;					/* deal with leading white space */
				while (*s == ' ') s++;
				m = strcspn(s, " \n\r\t\f");
				if (strstr(s, "noaccess def") != NULL) {	/* 1993/Jan/4 */
					if (traceflag != 0) printf("BREAKING OUT ON %s", line);
					break;
				}
				if (strstr(s, "readonly def") != NULL) {	/* 1993/Nov/12 */
					if (traceflag != 0) printf("BREAKING OUT ON %s", line);
					break;
				}
				if (m == (int) strlen(ndsynom2) &&
					strncmp(s, ndsynom2, m) == 0) {
					if (traceflag != 0) printf("BREAKING OUT ON %s", line);
					break;
				}
/* new experiment 1993/Feb/4 */
				fputs(line, fp_out);
			}
			if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
		}
	}

/*	deal with ATM 3.0 problem with Subr to CharString transition 1994/Nov/12 */
	if (strstr(buffer, "readonly def") != NULL) {	
		sprintf(buffer, "noaccess def\n");
		printf("WARNING:  Replacing poor Subr to CharString transition\n");
	}
/*	fputs(buffer, fp_out); */	/* line saved up from FILE 1 */
	strcpy(line, buffer);		/* 1992/Jan/5 */
	if (traceflag != 0) printf("Saved line was: %s", line);
	if (traceflag != 0) printf("Scan up to CharString in file 1\n");
/*	copy across from FILE 1 up to /CharStrings line */
	while ((s = strstr(line, "/CharStrings")) == NULL) {
		fputs(line, fp_out);
		if (traceflag != 0) fputs(line, stdout);
		if ((n = getline(fp_pln1, line)) == 0) break;		
/*		if (traceflag != 0) fputs(line, stdout); */
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}

	if (n == 0) {
		fprintf(stderr, "Premature EOF 3\n");
		return -1;
	}	

/*	modify the /CharStrings line */
	if (ncharstrings2 + spaceneeded > 0) { 
		sscanf(s, "/CharStrings %d%n", &ncharstrings, &m);
		strcpy(buffer, s + m);
		sprintf(s, "/CharStrings %d%s", 
			ncharstrings1 + ncharstrings2 + spaceneeded, buffer);
	}
	fputs(line, fp_out);
	if (traceflag != 0) fputs(line, stdout); 

	if (singleargument == 0) {
		if (traceflag != 0) printf("Scan up to CharString in file 2\n");
/*	scan ahead in  FILE 2 up to /CharStrings line */
		while ((n = getline(fp_pln2, line)) != 0 &&
			(s = strstr(line, "/CharStrings")) == NULL) {
/*			fputs(line, fp_out); */
/*			if (traceflag != 0) fputs(line, stdout); */
			if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
		}
		if (traceflag != 0) fputs(line, stdout); 

		if (n == 0) {
			fprintf(stderr, "Premature EOF 4\n");
			return -1;
		}
	}

	if (traceflag != 0) printf("Copy CharStrings from file 1\n");
/*	copy across all of the CharStrings from FILE 1 */
	for (;;) {
		n = getline(fp_pln1, line);
		if (n <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;

		if ((s = strchr(line, '/')) != NULL) {
/*			sscanf(s, "/%s ", charname); */
		}
		fputs(line, fp_out); /* NOT A '/' line */
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
/*	putc('\n', fp_out); */	/* separator between two sets of CharStrings */
	strcpy(buffer, line);	/* save line from FILE 1 */

/* may want to check whether space already exists */

/*	maybe good place to insert space ? */ /* 1992/Sep/6 */
	if (spaceneeded != 0) {	
		if (verboseflag != 0) printf("Adding space of width %d\n", spacewidth);
		if (lookup("space") >= 0)
			fprintf(errout, "WARNING: space character already exists\n");
		putc('\n', fp_out);
		fprintf(fp_out, "/space 9 %s\n", rdsynom1);		/* RD */
		fprintf(fp_out, "0 %d hsbw\n", spacewidth);		
		fprintf(fp_out, "endchar\n");
		fprintf(fp_out, "%s\n", ndsynom1);				/* ND  */
	}

	if (singleargument == 0) {
	if (traceflag != 0) printf("Copy CharStrings from file 2\n");
/*	copy across all of the CharStrings from FILE 2 */	
	for (;;) {
		n = getline(fp_pln2, line);
		if (n <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;	
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;

		if ((s = strchr(line, '/')) != NULL) {
			if (sscanf(s, "/%s %d %s", charname, &charnum, rdsubr) < 3) {
				fputs(line, errout);
			}
			if (strcmp(rdsubr, rdsynom2) != 0) {
				fprintf(errout, 
					"Apparent mismatch in RD (CharString 2) `%s' `%s': %s\n",
						rdsubr, rdsynom2, line);
			}
			sprintf(line, "/%s %d %s\n", charname, charnum, rdsynom1);
			fputs(line, fp_out);			/* A '/' line */
			while ((n = getline(fp_pln2, line)) != 0) {
				if (*line < ' ') break;		/* a blank line */
/*				if (strchr(line, '/') == NULL &&
					strstr(line, "endchar") == NULL && 
						strstr(line, "end") != NULL) {
						break;
				} */ 	/* 1992/Dec/9 ??? */
				s = line;					/* deal with leading white space */
				while (*s == ' ') s++;
				m = strcspn(s, " \n\r\t\f");
				if (strstr(s, "noaccess def") != NULL) {	/* 1993/Jan/4 */
					sprintf(line, "%s\n", ndsynom1);
					fputs(line, fp_out); 
					break;
				}
				if (m == (int) strlen(ndsynom2) && 
					strncmp(s, ndsynom2, m) == 0) {
					sprintf(line, "%s\n", ndsynom1);
					fputs(line, fp_out); 
					break;
				}
				if ((s = strstr(line, "callsubr")) != NULL) 
					adjustsubrnum(line);
				if ((s = strstr(line, "callothersubr")) != NULL) 
					adjustothersubr(line);
				fputs(line, fp_out);
			}
		}
		else fputs(line, fp_out); /* NOT A '/' line */
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
	}

/*	fputs(buffer, fp_out); */	/* saved line from FILE 1 */

	if (traceflag != 0) printf("Copy tail end from file 1\n");
/*	copy the tail across */
	if (n != 0) fputs(line, fp_out);
	while ((n = getline(fp_pln1, line)) != 0)	fputs(line, fp_out);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	if (quietflag == 0)
		printf("%s [-v] [-h] [-f] [-s=<width>] <base PFA> [<extra PFA>]\n", s);
		printf("\tv  Verbose mode\n");
		printf("\th  Suppress use of Subr 4 for hint switching\n");
		printf("\tf  Truncate floating point font level hints\n");
		printf("\ts  Manufacture a `space' of specified width\n");
	exit(1);
}

void makename(char *fname, char *ext) {	/* make unique temporary filename */
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
	forceexten(fname, ext);		/* force appropriate extension */
}

FILE *fp_dec1=NULL, *fp_pln1=NULL;
FILE *fp_dec2=NULL, *fp_pln2=NULL;
FILE *fp_adj=NULL;

FILE *fp_dec=NULL, *fp_pln=NULL;

char fn_dec1[FILENAME_MAX], fn_pln1[FILENAME_MAX];
char fn_dec2[FILENAME_MAX], fn_pln2[FILENAME_MAX];
char fn_adj[FILENAME_MAX];

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
		fclose(fp_dec);
		wipefile (fn_dec1);
		(void) remove(fn_dec1);	/* remove bogus file */
		wipefile (fn_dec2);
		(void) remove(fn_dec2);	/* remove bogus file */
	}
	if (fp_pln != NULL) {
		fclose(fp_pln);	
		wipefile (fn_pln1);
		(void) remove(fn_pln1);	/* remove bogus file */
		wipefile (fn_pln2);
		(void) remove(fn_pln2);	/* remove bogus file */
	}

	if (fp_dec1 != NULL) {
		(void) fclose(fp_dec1);						/* close output */
		wipefile (fn_dec1);
		(void) remove(fn_dec1);	/* remove bogus file */
	}
	if (fp_pln1 != NULL) {
		(void) fclose(fp_pln1);				/* close output */
		wipefile (fn_pln1);
		(void) remove(fn_pln1); /* remove bogus file */
	}

	if (fp_dec2 != NULL) {
		(void) fclose(fp_dec2);						/* close output */
		wipefile (fn_dec2);
		(void) remove(fn_dec2);	/* remove bogus file */
	}
	if (fp_pln2 != NULL) {
		(void) fclose(fp_pln2);				/* close output */
		wipefile (fn_pln2);
		(void) remove(fn_pln2); /* remove bogus file */
	}

	if (fp_adj != NULL) {
		(void) fclose(fp_adj);				/* close output */
		wipefile (fn_adj);
		(void) remove(fn_adj); /* remove bogus file */
	}
}

void abortjob() {
	fprintf(stderr, "\nUser Interrupt - Exiting\n"); 
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
/*	fprintf(stderr, "\nUser Interrupt - Exiting\n");  */
/*	cleanup(); */
/*	if (renameflag != 0) rename(fn_in, fn_bak); */
	if (bAbort++ >= 15)	exit(3);		/* emergency exit */
	(void) signal(SIGINT, ctrlbreak);
}

int decodeflag (int c) {
	switch(c) { 
		case 'v': verboseflag = 1; dotsflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case '?': detailflag++; return 0;
		case 'd': wipeclean++; return 0;
		case 'h': allowlowswitch = 0; return 0;
		case 'E': errout = stderr; return 0;	/* 97/Sep/28 */
/*		case 'f': fixfloating = 0; return 0; */
		case 'f': fixfloating = 1; return 0;
		case 's': spaceflag++; return -1;		/* needs argument */
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
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

/* return pointer to file name - minus path - returns pointer to filename */

char *extractpath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/* flush path from file name --- that is, use current directory */

void flushpath(char *filename) {
	strcpy(filename, extractpath(filename));
}

void underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

/*	s = strippath(filename); */
	s = extractpath(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
}

//	stage == 0 for base font
//	stage == 1 for add on font

int checkoutpfa (char *progfile, char *pfaname, char *fn_dec, char *fn_pln, int stage) {
/*	FILE *fp_dec, *fp_pln;	*/ /* ??? */
	char fn_ind[FILENAME_MAX]; 
	FILE *fp_in;
	int c, d;
	char *s;

/*	fp_dec = (FILE *) 69; */
/*	fp_pln = (FILE *) 73; */

	strcpy(fn_ind, pfaname);	
	extension(fn_ind, "pfa"); 
	
	if ((fp_in = fopen(fn_ind, "rb")) == NULL) {	/* see whether exists */
/*		underscore(fn_in);				*/		/* 1993/May/30 */
		underscore(fn_ind);						/* 1994/Nov/12 */
		if ((fp_in = fopen(fn_ind, "rb")) == NULL) {/* see whether exists */
			perror(fn_ind);	exit(13);
		}
	}

	if (stage == 0) strcpy (fn_in, fn_ind);

/*	scanning base file */

	if (verboseflag != 0) printf("Processing file %s\n", fn_ind);

/*	if ((fp_in = fopen(fn_ind, "rb")) == NULL) {	
		perror(fn_ind);	exit(13);
	}
	else {  */
		c = fgetc(fp_in); d = fgetc(fp_in);
		if (c != '%' || d != '!') {
			fprintf(stderr, "ERROR: This does not appear to be a PFA file\n");
			exit(13);							/* 1993/May/30 */
		}
		fclose(fp_in);
/*	} */

	if (stage == 0) {

		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
		strcpy(fn_out, s);			/* copy input file name minus path */

/*		forceexten(fn_out, "pfa"); */
		if (forceoutname != 0 && singleargument == 0) {
/*			if (quietflag == 0) strcpy(fn_out, outfilename); */
			if (quietflag == 0) strcpy(fn_out, "merged");  /* MERGED */
			else strcpy(fn_out, extractpath(progfile)); /* use prog name */
		}
		forceexten(fn_out, "pfa");
		
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
		underscore(fn_ind);
		if ((fp_in = fopen(fn_ind, "rb")) == NULL) { 
			perror(fn_ind); exit(2);
		}
	}
	if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
		perror(fn_dec); exit(2);
	}

	eexecscan = 1;
	charscan = 0;  decodecharflag = 0;  charenflag = 0; binaryflag = 0;

	(void) decrypt(fp_dec, fp_in);		/* actually go do the work */

	fclose(fp_in);
	if (ferror(fp_dec) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec); cleanup();	exit(2);
	}
	else fclose(fp_dec);

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
		fprintf(stderr, "Output error ");
		perror(fn_pln); cleanup();
		exit(2);
	}
	else fclose(fp_pln);

	if (debugflag != 0) _getch();

/*	NOW HAVE PLN FILE IN SIGHT */

	if (traceflag != 0) 
		printf("Extraction of CharStrings starting\n"); 

	if ((fp_pln = fopen(fn_pln, "rb")) == NULL) { 
		perror(fn_pln);  cleanup(); exit(2);
	}

	if (verboseflag != 0) printf("Pass 0\n");

	(void) extractcharnames(fp_pln, stage);	/* extract info first */

/*	rewind(fp_pln); */
	fclose(fp_pln);
	return 0;
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
	fprintf(stderr, "HASHED %ld\n", hash);	(void) _getch(); 
	return hash;
}

char *progfunction="merging and normalizing";

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
			if (spaceflag != 0) {
				if (sscanf(s, "%d", &spacewidth) > 0) spaceneeded = 1;
				spaceflag = 0;
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

void uppercase (char *s) {
	int c;
	while ((c = *s) != '\0') {
		if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
		*s = (char) c;
		s++;
	}
}

int _cdecl main(int argc, char *argv[]) {       /* main program */
/*	FILE *fp_in, *fp_des; */
	FILE *fp_out;
	int firstarg=1;
	int n;
	char *s;

	if (strlen(progname) < 16) strcpy(progname, compilefile);
	if ((s = strchr(progname, '.')) != NULL) *s = '\0';
	uppercase(progname);

	strcpy (line, argv[0]);
	uppercase (line);
	if (strstr(line, progname) == NULL) quietflag = 1; 

	if (argc < firstarg+1) showusage(argv[0]); 

/*	while (argv[firstarg][0] == '-') { 
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else decodeflag(c);
		}
		firstarg++;
	}  */

	firstarg = commandline(argc, argv, 1); /* check for command flags */

	if (wipeclean == 4) wipeclean = 0;	/* `ddd' on command line */

/*	if (argc < firstarg) showusage(argv[0]);  */
	if (argc < firstarg + 1) showusage(argv[0]); /* 1993/May/13 */
		
	if (quietflag != 0) {
		verboseflag = 0; traceflag = 0; dotsflag = 0;
/*		spaceneeded = 0; */
	}

/*	scivilize(compiledate);	 */
	if (quietflag == 0) stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	(void) signal(SIGINT, ctrlbreak);

	if ((s = getenv("TMP")) != NULL) tempdir = s;
	if ((s = getenv("TEMP")) != NULL) tempdir = s;

/*	printf("Font merging and normalizing program version %s\n",
		VERSION); */

	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

/*	if (argc < firstarg + 2) showusage(argv[0]);*/
	if (argc < firstarg + 1) showusage(argv[0]);
	if (argc < firstarg + 2) singleargument++;

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

/*	stringindex=0; */		/* reset string pool table */
/*	for (k = 0; k < MAXCHARINFONT; k++)	charnames[k] = NULL; */ // ???

/*	strcpy(fn_in, argv[firstarg]);
	extension(fn_in, "pfa"); */
/*	checkoutpfa (argv[0], fn_in, fn_dec1, fn_pln1, 0); */

	checkoutpfa (argv[0], argv[firstarg], fn_dec1, fn_pln1, 0);

	if ((fp_dec1 = fopen(fn_dec1, "wb")) == NULL) { /* so cleanup kicks in */
		perror(fn_dec1); cleanup(); exit(2);
	}
	else fclose(fp_dec1);

	nsubrs1 = nsubrs; 
	ncharstrings1 = ncharstrings;
	subrseen1 = subrseen;
	othersubrseen1 = othersubrseen;
	notdefseen1 = notdefseen;
	switchtrick1 = switchtrick;
	strcpy(rdsynom1, rdsynom);
	strcpy(ndsynom1, ndsynom);	
	strcpy(npsynom1, npsynom);	
	
	fp_pln1 = fopen(fn_pln1, "r");

	if (singleargument == 0) {

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

	checkoutpfa (argv[0], argv[firstarg+1], fn_dec2, fn_pln2, 1);

	if ((fp_dec2 = fopen(fn_dec2, "wb")) == NULL) { /* so cleanup kicks in */
		perror(fn_dec1); cleanup(); exit(2);
	}
	else fclose(fp_dec2);

	nsubrs2 = nsubrs; 
	ncharstrings2 = ncharstrings;
	subrseen2 = subrseen;
	othersubrseen2 = othersubrseen;
	notdefseen2 = notdefseen;
	switchtrick2 = switchtrick;
	strcpy(rdsynom2, rdsynom);
	strcpy(ndsynom2, ndsynom);	
	strcpy(npsynom2, npsynom);

	if (allowlowswitch != 0 && switchtrick1 != 0 && switchtrick2 != 0) {
		if (verboseflag != 0)
			printf ("NOTE: Will use Subr 4 of first font for hint switching\n");
		uselowswitch = 1;
/*	could now eliminate Subr 4 of second font, but hardly worth it ... */
	}
	else uselowswitch = 0;				/* 1993/Aug/30 */

	fp_pln2 = fopen(fn_pln2, "r");

	}

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

	if (othersubrseen2 != 0 && othersubrseen1 == 0) needothersubrs = 1;
	else needothersubrs = 0;
	
	if (notdefseen1 == 0 && notdefseen2 == 0)
		fprintf(errout, 
		"WARNING: At least one of the fonts should contain .notdef char\n");
	
	if (notdefseen1 != 0 && notdefseen2 != 0)
		fprintf(errout, 
		"NOTE: At most one of the fonts need contain .notdef char\n");	

	if (subrseen1 == 0) needsubrs = 1;
	else needsubrs = 0;

	if (needsubrs != 0)
		printf("WARNING: Will have to insert standard Subrs\n");

	if (needothersubrs != 0)
		printf("WARNING: Will have to insert standard OtherSubrs\n");

/* Now have two `plain' files and info on number of Subrs and CharStrings */

	if (writeoutflag == 0) return 0;		/* while debugging */

/*	if (fp_dec1 == NULL || fp_dec2 == NULL)
		fprintf (stderr, "BAD DEC FILE HANDLES\n"); */

/*	if (fp_pln1 == NULL || fp_pln2 == NULL)
		fprintf (stderr, "BAD PLN FILE HANDLES\n"); */

	maketemporary(fn_adj, fn_out, "adj", 0);
	if (traceflag != 0) printf("Using %s as temporary\n", fn_adj);

	if ((fp_adj = fopen(fn_adj, "wb")) == NULL) { 
		perror(fn_adj); cleanup(); exit(2);
	}

	if (verboseflag != 0) printf("Pass 1\n");	/* now modify and remove */

	rewind(fp_pln1); 
	if (singleargument == 0) rewind(fp_pln2);

	n = mergepln(fp_adj, fp_pln1, fp_pln2);
		
	if (traceflag != 0) printf("FINISHED MERGE\n");

	fclose(fp_pln1); 
	if (singleargument == 0) fclose(fp_pln2);

	if (ferror(fp_adj) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_adj); cleanup(); exit(2);
	}
	else fclose(fp_adj);

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

	if (n < 0) {	/* if there was a failure in the merging */
		fprintf(errout, "ERROR: Sorry, unable to merge PFA files\n");
		if (renameflag != 0) rename(fn_in, fn_bak);
		cleanup();
		return 0;
	}

	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass B (up) starting\n"); 

	if ((fp_adj = fopen(fn_adj, "rb")) == NULL) { 
		perror(fn_adj);  cleanup(); exit(2);
	}
	if ((fp_dec1 = fopen(fn_dec1, "wb")) == NULL) { 
		perror(fn_dec1); cleanup(); exit(2);
	}

	eexecscan = 1; charenflag = 1; charscan = 1;

	(void) encrypt(fp_dec1, fp_adj);	/* actually go do the work */
		
	fclose(fp_adj);
	if (ferror(fp_dec1) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec1); cleanup(); exit(2);
	}
	else fclose(fp_dec1);

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass A (up) starting\n"); 

	if ((fp_dec1 = fopen(fn_dec1, "rb")) == NULL) { 
		perror(fn_dec1); cleanup();	exit(2);
	}
	if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
		perror(fn_out); cleanup();	exit(2);
	}

	eexecscan = 1; charenflag = 0; charscan = 0;

	(void) encrypt(fp_out, fp_dec1);	/* actually go do the work */
		
	fclose(fp_dec1);

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

	if (debugflag != 0) _getch();

	if (ferror(fp_out) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_out); cleanup();	exit(2);
	}
	else fclose(fp_out);
	if (verboseflag) printf("Output in %s\n", fn_out);
	cleanup();
/*	freecharnames(); */ /* implicit */
	freechars();
	return 0;
}	

/* renumber Subrs */
/* renumber calls to Subrs in Subrs */
/* renumber calls to Subrs in CharStrings */

/* remove unused Subrs ? */
/* avoid duplication of CharStrings ? */

/* flush one .notdef CharString ? */	/* makes it better or worse ? */

/* check that first lot has the boiler plate Subrs in position ? */
/* if not, its Subrs (and calls to same) also has to be shifted upward ? */ 

/* if space requested - add to encoding ? */ /* not yet implemented */

/*	/space 9 -|		0 250 hsbw		endchar		|- */
