/* subfont.c
   Copyright Y&Y 1992, 1993, 1994, 1995
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
#include <stdlib.h>
#include <string.h>
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

#define FROMCHAR 1
#define FROMSUBR 2

/* #define MAXNAME 80 */

int replaceone=1;		/* replace old fashioned hint replace Subr calls */
int adjustnotdef=0;		/* allow exactly one .notdef */

int wantcpyrght=1;

/* Font merging and normalizing program version */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* char *progname = "SUBFONT"; */

char progname[16] = "";

/* char *progversion = "1.0"; */

/* char *progversion = "1.1"; */

/* char *progversion = "1.2"; */		/* 1995/April/30 */

/* char *progversion = "1.3"; */		/* 1995/June/7 */

/* char *progversion = "1.3.1"; */		/* 1997/Jan/10 */

/* char *progversion = "1.3.2"; */		/* 1997/Jul/20 */
/* char *progversion = "1.4"; */		/* 1998/Dec/18 */
char *progversion = "2.0";				/* 1998/Dec/25 */

char *copyright = "\
Copyright (C) 1992-1998  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1992--1997  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 2670755 */
/* #define COPYHASH 13986445 */
/* #define COPYHASH 11737093 */
/* #define COPYHASH 5618236 */
/* #define COPYHASH 11501465 */
/* #define COPYHASH 6490707 */
/* #define COPYHASH 3523562 */
#define COPYHASH 3956063

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

char *charfile="";		/* name of character inclusion set file */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int debugflag = 0;
int wipeclean = 1;	
int dotsflag = 1;
int quietflag = 0;
int shrinkflag = 1;		/* remove unused Subrs and renumber */
int characterflag = 0;	/* next arg is character file */
int excludeini = 0;		/* default: meet characters to include, not exclude */

int renameflag = 0;

int eexecscan = 0;		/* scan up to eexec before starting */
int charscan = 0;		/* scan up to RD before starting */
int charenflag = 0;		/* non-zero charstring coding instead of eexec */
int decodecharflag = 0;	/* non-zero means also decode charstring */
int binaryflag = 0;		/* input is binary, not hexadecimal */

static int hires=0;			/* indicate hybrid font 94/Jan/6 */

volatile int bAbort = 0;			/* non-zero whn user types control-C */

int writeoutflag = 1;	/* zero => extract info only */

int forceoutname = 1;	/* don't use input file name for output file name */

char *progfunction= "extract subfont";

/* char *outfilename = "subfont.pfa"; */

extern int encrypt(FILE *, FILE *);

extern int decrypt(FILE *, FILE *);

char *tempdir="";		/* place to put temporary files */

char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
char fn_bak[FILENAME_MAX], fn_des[FILENAME_MAX];

FILE *errout=stdout;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int maxchars=0;

int charindex = 0;		/* number of current character reading in  */

/* char *charnames[MAXCHARINFONT]; */	/*  CharString charname */
char * __far *charnames=NULL;

/* char charhit[MAXCHARINFONT]; */	/* non-zero if old one exists in PFA file */
short *charhit=NULL;

/* char charused[MAXCHARINFONT]; */	/* non-zero if this name copied already */
short *charused=NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int ncharstrings;		/* number of CharStrings in original PFA file */

int nsubrs;				/* number of Subrs in original PFA file */

int ncharhits;			/* how many characters given in desired file */

int nmiss;

int nwanted;			/* number of characters seen in desired file */

int charseen = 0;		/* non-zero after character string RD and ND seen */
int nextend = 0;		/* non-zero if next line is ND */

char rdsynom[16] = "RD";	/* RD or -| or ... */
char ndsynom[16] = "ND";	/* ND or |- or ... */

static char buffer[MAXLINE];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

/* not sure distinction between `normal' Subrs and hint replacement needed */

/* make short instead of int to save somer space */
/* actually these only take on -1, 0, 1, 2 small values */

int nsubrs=0;

int maxsubrs=0;

/* int normsubr[MAXSUBRS]; */
short __far *normsubr=NULL;

/* int hintsubr[MAXSUBRS]; */
short __far *hintsubr=NULL;

/* int checksubr[MAXSUBRS]; */
short __far *checksubr=NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *composites[] = {
"Aacute", "Acircumflex", "Adieresis", "Agrave", "Aring", "Atilde",
"Ccedilla",
"Eacute", "Ecircumflex", "Edieresis", "Egrave",
"Iacute", "Icircumflex", "Idieresis", "Igrave",
"Ntilde",
"Oacute", "Ocircumflex", "Odieresis", "Ograve", "Otilde",
"Scaron",
"Uacute", "Ucircumflex", "Udieresis", "Ugrave",
"Yacute", "Ydieresis",
"Zcaron", ""
};

char *specials[] = {
"AE", "OE", "Lslash", "Oslash", "Eth", "Thorn", ""
};

char *accents[] = {
"grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
"dieresis", "ring", "cedilla", "hungarumlaut", "ogonek", "caron", ""
}; 

/* dotlessi => I ??? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	 
void freecharnames(void) {
	int k;
	if (charnames == NULL) return;
	for (k = 0; k < maxchars; k++) {
		if (charnames[k] != NULL) {
			free(charnames[k]);
			charnames[k] = NULL;
		}
	}
}

void freechars(void) {
	if (maxchars > 0) freecharnames();
	if (charnames != NULL) free(charnames);
	if (charhit != NULL) free(charhit);
	if (charused != NULL) free(charused);
	charnames = NULL;
	charused = charhit = NULL;
	maxchars = 0;
}

void allocchars(int nchars) {
	int k;
	if (maxchars > 0) freechars();
	maxchars = 0;
	if (verboseflag) printf("Allocating space for %d chars\n", nchars);
	charnames = (char * __far *) _fmalloc(nchars * sizeof(char *));
	charhit = (short __far *) _fmalloc(nchars * sizeof(short));
	charused = (short __far *) _fmalloc(nchars * sizeof(short));
	if (charnames == NULL || charhit == NULL || charused == NULL){
		fprintf(errout, "ERROR: failed memory allocation of %d bytes\n",
				nchars * sizeof(char *));
		exit(1);
	}
	for (k = maxchars; k < nchars; k++) {
		charnames[k] = NULL;
		charhit[k] = charused[k] = 0;
	}
	maxchars = nchars;
}

void reallocchars(int nchars) {
	int k;
	if (verboseflag) printf("Reallocating space for %d => %d chars\n", maxchars, nchars);
	charnames = (char * __far *) _frealloc(charnames, nchars * sizeof(char *));
	charhit = (short __far *) _frealloc(charhit, nchars * sizeof(short));
	charused = (short __far *) _frealloc(charused, nchars * sizeof(short));
	if (charnames == NULL || charhit == NULL || charused == NULL){
		fprintf(errout, "ERROR: failed memory allocation of %d bytes\n",
				nchars * sizeof(char *));
		exit(1);
	}
	for (k = maxchars; k < nchars; k++) {
		charnames[k] = NULL;
		charhit[k] = charused[k] = 0;
	}
	maxchars = nchars;
}

/********************************************************************/

void freesubrs(void) {
	if (normsubr != NULL) free(normsubr);
	if (hintsubr != NULL) free(hintsubr);
	if (checksubr != NULL) free(checksubr);
	normsubr = hintsubr = checksubr = NULL;
	maxsubrs = 0;
}

void allocsubrs(int nsubrs) {
	int k;
	if (maxsubrs > 0) freesubrs();
	maxsubrs = 0;
	if (verboseflag)printf("Allocating space for %d Subrs\n", nsubrs);
	normsubr = (short __far *) _fmalloc(nsubrs * sizeof(short));
	hintsubr = (short __far *) _fmalloc(nsubrs * sizeof(short));
	checksubr = (short __far *) _fmalloc(nsubrs * sizeof(short));
	if (normsubr == NULL || hintsubr == NULL || checksubr == NULL) {
		fprintf(errout, "Unable to allocate %ld bytes\n",
				nsubrs * sizeof(short));
		exit(1);
	}
	for (k = maxsubrs; k < nsubrs; k++) {
		checksubr[k]=1;
		normsubr[k]=hintsubr[k]=-1;
	}
	maxsubrs = nsubrs;
}

void reallocsubrs(int nsubrs) {
	int k;
	if (verboseflag)printf("Reallocating space for %d => %d Subrs\n",
						 maxsubrs, nsubrs);
	normsubr = (short __far *) _frealloc(normsubr, nsubrs * sizeof(short));
	hintsubr = (short __far *) _frealloc(hintsubr, nsubrs * sizeof(short));
	checksubr = (short __far *) _frealloc(checksubr, nsubrs * sizeof(short));
	if (normsubr == NULL || hintsubr == NULL || checksubr == NULL) {
		fprintf(errout, "Unable to allocate %ld bytes\n",
				nsubrs * sizeof(short));
		exit(1);
	}
	for (k = maxsubrs; k < nsubrs; k++) {
		checksubr[k]=1;
		normsubr[k]=hintsubr[k]=-1;
	}
	maxsubrs = nsubrs;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void abortjob (void);

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

static char line[MAXLINE];		/* buffer for getline */

static int getline(FILE *fp_in, char *line) {
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	return strlen(line); 
} 

int lookup(char *name) {
	int k, flag = -1;
	for (k = 0; k < charindex; k++) {
/*		if (strcmp(charnames[k], "") == 0) continue; */
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

void extractcharnames(FILE *fp_in) {
	char charname[CHARNAME_MAX];
	int k, n;
	char *s=NULL;

/*	for (k = 0; k < MAXCHARINFONT; k++)	charnames[k] = NULL; */

/*  scan up to eexec encrypted part */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
	}

/*	scan up to CharStrings */
	while ((n = getline(fp_in, line)) != 0 && 
		(s = strstr(line, "/CharStrings")) == NULL) {
		if ((s = strstr(line, "/Subrs ")) != NULL) {
			sscanf (s + 7, "%d", &nsubrs);
		}
	}
	if (s == NULL) return;			/* EOF */
	if (nsubrs == 0) return;		/* did not find /Subrs */
	if (nsubrs != maxsubrs) allocsubrs(nsubrs);

	sscanf(s + 13, "%d%n", &ncharstrings, &n);
	allocchars(ncharstrings);
/*	scan all of the CharStrings */ /* remember the names */
/*	for hybrid (hires) fonts we only need the first set ... */
	for (;;) {
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
		if ((s = strchr(line, '/')) != NULL) {
			if (charseen == 0) {
				sscanf(s, "/%s %d %s\n", charname, &k, rdsynom);
				stripreturn(rdsynom);
				if (traceflag != 0) printf("RDsynonym: `%s'\n", rdsynom);
			}
			sscanf(s, "/%s ", charname);
			if (charindex >= maxchars)
				reallocchars(maxchars+maxchars);
/*			strcpy(charnames[charindex], charname); */
/*			charnames[charindex] = strdupx(charname); */
			charnames[charindex] = zstrdup(charname);	/* 98/Dec/18 */
/*			if (charindex++ > MAXCHARINFONT) */
/*				fprintf(errout, "ERROR: Too many characters in font\n"); */
/*				charindex--; */
			charindex++;
		}
		if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	}		
 }

char *digits[] = {
	"zero", "one", "two", "three", "four", 
	"five", "six", "seven", "eight", "nine"
};		

char *gettoken(FILE *fp_in) {
	char *t;
	t = strtok (NULL, " \t\n\r\f[]");
	if (t == NULL) {
		if (getline(fp_in, line) == 0) return NULL;	/* EOF */
		t = strtok (line, " \t\n\r\f[]");
	}
	return t;
}

int insert (char *charname) {
	int k;
	if (charname == NULL) return 0;			/* sanity check */
	if (strcmp(charname, "") == 0) return 0; /* 1993/Oct/31 */
	k = lookup (charname);
	if (k < 0) {
		fprintf(errout, "%s? ", charname);
		nmiss++;
		return 0;
	}
	else {
/*		charhit[k]++; */
		charhit[k] = 1;
		ncharhits++;
		return 1;
	}
}

int delete (char *charname) {
	int k;
	if (charname == NULL) return 0;			/* sanity check */
	if (strcmp(charname, "") == 0) return 0; /* 1993/Oct/31 */
	k = lookup (charname);
	if (k < 0) {
		fprintf(errout, "%s? ", charname);
		nmiss++;
		return 0;
	}
	else {
		if (charhit[k] != 0) ncharhits--;
		charhit[k] = 0;
		return 1;
	}
}

void lowercase (char *s) {
	int c;
	while ((c = *s) != 0) {
		if (c >= 'A' && c <= 'Z') *s = (char) (c + 'a' - 'A');
		s++;
	}
}

void fillbackground(void) {
	int k;
	for (k = 0; k < charindex; k++) {
/*		if (strcmp(charnames[k], "") != 0) { */
		if (charnames[k] != NULL) {
			if (charhit[k] == 0) ncharhits++;
/*			charhit[k]++; */
			charhit[k] = 1;
		}
	}
}

/* Read desired characters from file */

int readdesired (FILE *fp_in) {
	char charname[CHARNAME_MAX];
	int chara, charb;
	int n, m, exclude;
	char *s=NULL;

/*	for (k = 0; k < MAXCHARINFONT; k++) charhit[k] = 0; */
/*	for (k = 0; k < MAXCHARINFONT; k++) charused[k] = 0; */
	ncharhits = 0; nmiss = 0;
	
/*	See whether `absurd' format */
	getline(fp_in, line);
	if (strncmp(line, "%!PS-AdobeFont", 14) == 0) {
/*		try and find metrics */
		while ((n = getline(fp_in, line)) != 0 &&
			(s = strstr(line, "/Metrics")) == NULL) ;
		if (s == NULL || n == 0) {
			fprintf(errout, "ERROR: Unable to find metric information\n");
			return -1;
		}
		strtok (s, " \t\n\r\f[]");	/* prime the token pump */
		for(;;) {
			s = gettoken(fp_in);
			if (s == NULL || strcmp(s, "end") == 0) break;
			if (*s == '/') {
/*				strcpy(charname, s+1); */
				insert (s+1);
/*				if (traceflag != 0) printf("%s ", charname); */
				if (traceflag != 0) printf("%s ", s+1);
				else if (dotsflag != 0) putc('.', stdout);
			}
		}
		if (traceflag != 0) putc('\n', stdout);
		else if (dotsflag != 0) putc('\n', stdout);
		if (nmiss > 0) printf("\nAbove characters not found in font\n");
		if (traceflag != 0) {
			printf("Input read --- hit %d out of %d\n", 
				ncharhits, ncharstrings);
		}
		return 0;
	}

	rewind(fp_in);	/* `Normal' format - continue this way */

/*	exclude = 0; */	/* default: meet characters to include, not exclude */

	exclude = excludeini;

	if (exclude) fillbackground();			/* 97/Jul/20 */

	while ((n = getline(fp_in, line)) != 0) {
		if (strncmp(line, "% EOF", 5) == 0) break;
		if (strncmp(line, "; EOF", 5) == 0) break;		
		if (*line == '%' || *line == ';' || *line < ' ') continue;
/*		fputs(line, stdout); */		/* debugging */
/*		first check for excluded sequence case */
/*		if (*line == '~') */
		if (*line == '~' || excludeini) {
			if (exclude++ == 0) 		/* fill in background first time */
				fillbackground();
			if (strchr(line, '-') != NULL) {	/* excluded sequence */
/*				if (strncmp(line+1, "Aacute-Zcaron", 13) == 0) */
				if (strstr(line, "Aacute-Zcaron") != NULL) {
					for (m = 0; m < 29; m++) {
						delete(composites[m]);
/*						printf(composites[m]); */	/* debugging */
					}
					continue;
				}
/*				if (strncmp(line+1, "aacute-zcaron", 13) == 0) */
				if (strstr(line, "aacute-zcaron") != NULL) {
					for (m = 0; m < 29; m++) {
						strcpy(charname, composites[m]);
						lowercase(charname);
						delete(charname);
/*						printf(charname); */		/* debugging */
					}
					continue;
				}
/* special characters */
/*				if (strncmp(line+1, "AE-Thorn", 8) == 0) */
				if (strstr(line, "AE-Thorn") != NULL) {
					for (m = 0; m < 6; m++) {
						delete(specials[m]);
/*						printf(specials[m]); */	/* debugging */
					}
					continue;
				}
/*				if (strncmp(line+1, "ae-thorn", 8) == 0) */
				if (strstr(line, "ae-thorn") != NULL) {
					for (m = 0; m < 6; m++) {
						strcpy(charname, specials[m]);
						lowercase(charname);
						delete(charname);
/*						printf(charname); */		/* debugging */
					}
					continue;
				}
/* accents */
/*				if (strncmp(line+1, "acute-tilde", 11) == 0) */ /* 1993/Oct/31*/
				if (strstr(line, "acute-tilde") != NULL) { 
/*					for (m = 193; m <= 207; m++) */
					for (m = 0; m < 13; m++) { 
/*						strcpy(charname, standardencoding[m]); */
						strcpy(charname, accents[m]); 
						delete(charname);
/*						printf(charname); */		/* debugging */
					}
					continue;
				}
/* otherwise is a-z or A-Z or 0-9 or something ... */
/*				if (line[1+1] != '-') */
				if (excludeini) {
					if (line[1] != '-') fputs(line, errout);
					chara = line[0];
					charb = line[2];
				}
				else {
					if (line[1+1] != '-') fputs(line, errout);
					chara = line[0+1];
					charb = line[2+1];
				}
				if (chara > charb || charb - chara > 26) {
					fprintf(errout, 
						"ERROR: Don't understand: %s (%c-%c)", 
							line, chara, charb);
					continue;
				}
				if (chara >= '0' && charb <= '9') {
					for (m = chara; m <= charb; m++) {
						delete (digits[m - '0']);
					}
				}					/* end of digit sequence case */
				else {
					charname[1] = '\0';
					for (m = chara; m <=charb; m++) {
						charname[0] = (char) m; 
						delete(charname);
					}
				}			/* end of letter sequence case */
			}				/* end of excluded sequence case */
/* need to deal with single letter exlusion also */
			else {
				if (excludeini) sscanf (line, "%s", charname);
				else sscanf (line+1, "%s", charname);
				delete (charname);
			}				/* end of single charname case */
		}					/* end of excluded characters case */
/* now treat the included sequence case */
		else if ((s = strchr(line, '-')) != NULL) {
			if (strncmp(line, "Aacute-Zcaron", 13) == 0) {
				for (m = 0; m < 29; m++) {
					insert(composites[m]);
/*					printf(composites[m]); */	/* debugging */
				}
				continue;
			}
			if (strncmp(line, "aacute-zcaron", 13) == 0) {
				for (m = 0; m < 29; m++) {
					strcpy(charname, composites[m]);
					lowercase(charname);
					insert(charname);
/*					printf(charname); */		/* debugging */
				}
				continue;
			}
/* special characters */
			if (strncmp(line, "AE-Thorn", 8) == 0) {
				for (m = 0; m < 6; m++) {
					insert(specials[m]);
/*					printf(specials[m]); */	/* debugging */
				}
				continue;
			}
			if (strncmp(line, "ae-thorn", 8) == 0) {
				for (m = 0; m < 6; m++) {
					strcpy(charname, specials[m]);
					lowercase(charname);
					insert(charname);
/*					printf(charname); */		/* debugging */
				}
				continue;
			}
/* accents */
			if (strncmp(line, "acute-tilde", 11) == 0) { /* 1993/Oct/31 */
/*				for (m = 193; m <= 207; m++) { */
				for (m = 0; m < 13; m++) { 
/*					strcpy(charname, standardencoding[m]); */
					strcpy(charname, accents[m]); 
					insert(charname);
/*					printf(charname); */		/* debugging */
				}
				continue;
			}
/* otherwise is a-z or A-Z or 0-9 or something ... */
			if (line[1] != '-') fputs(line, errout);
			chara = line[0];
			charb = line[2];
			if (chara > charb || charb - chara > 26) {
				fprintf(errout, 
					"ERROR: Don't understand: %s (%c-%c)", 
						line, chara, charb);
				continue;
			}
			if (chara >= '0' && charb <= '9') {
				for (m = chara; m <= charb; m++) {
					insert(digits[m - '0']);
				}
			}					/* end of digit sequence case */
			else {
				charname[1] = '\0';
				for (m = chara; m <=charb; m++) {
					charname[0] = (char) m; 
					insert(charname);
				}
			}
		}					/* end of letter sequence case */
		else {
			sscanf (line, "%s", charname);
			insert(charname);
		}					/* end of single charname case */
		if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	}						/* end of while loop over getlines */
	if (nmiss > 0) printf("\nAbove characters not found in font\n");
	if (traceflag != 0) {
		printf("Input read --- hit %d out of %d\n", 
			ncharhits, ncharstrings);
	}
	return 0;
}

void processpfa(FILE *fp_out, FILE *fp_in) {
	char charname[CHARNAME_MAX];
	int k, n, flag;
	char *s;
	
	hires=0;										/* 94/Jan/6 */

/*  scan up to eexec encrypted part */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
		fputs(line, fp_out);
		if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	}
	fputs(line, fp_out);

/*	later insert code here to scan up to /Subrs */
/*	and to keep only Subrs actually needed */
/*	and to renumber the Subrs */
/*	scan up to CharStrings */

	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "/CharStrings") == NULL) {
		fputs(line, fp_out);
		if (strstr(line, "hires") != NULL) hires++;	/* 94/Jan/6 */
		if (bAbort != 0) abortjob();		/* 1992/Nov/24 */
	}
	s = strstr(line, "/CharStrings") + 13;
	sscanf(s, "%d%n", &ncharstrings, &n);

	if (ncharhits < ncharstrings) {
		strcpy(charname, s+n);					/* temporary */
		sprintf(s, "%d", ncharhits);
		strcat(s, charname);
		if (verboseflag != 0) fputs(line, stdout);	/* debugging */
	}
	fputs(line, fp_out);

/*	scan all of the CharStrings */
	for (;;) {
		n = getline(fp_in, line);
		if (n <= 0) break;
		if (strstr(line, "hires") != NULL) hires++;	/* 94/Jan/6 */
		if (hires == 0) {				/* normal font case */
			if (strchr(line, '/') == NULL &&
				strstr(line, "endchar") == NULL && 
					strstr(line, "end") != NULL) break;
		}
		else {							/* hybrid font case  94/Jan/6 */
			if (strncmp(line, "mark ", 5) == 0) break;	  /* 94/Jan/6 */
			if (strncmp(line, "00000000", 8) == 0) break; /* 94/Jan/6 */
		}
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;

		if ((s = strchr(line, '/')) != NULL) {
			if (s > line && *(s-1) > ' ') {			/* 94/Jan/6 */
				fputs(line, fp_out);
				continue;
			}
			(void) sscanf(s, "/%s ", charname);
			k = lookup(charname);
			if (k < 0) {
				fprintf(errout, "ERROR: Inconsistency for `%s'\n", charname);
				flag = 0;						/* 1993/June/21 */
			}
/*			else k = charhit[k]; */
			else {
				flag = charhit[k];				/* 1993/June/21 */
/*				if (flag != 0 && charused[k] != 0) { */
				if (flag != 0 && charused[k] != 0 && hires == 0) {/*94/Jan/6*/
					fprintf(errout,
						"WARNING: `%s' occurs more than once\n", charname);
					flag = 0;
				}
			}
/*			if (k == 0) { */
			if (flag == 0) {					/* skip over it */
/*				printf("FIRST REJECT: %s", line); */
				while ((n = getline(fp_in, line)) > 0 &&
					strstr(line, ndsynom) == NULL) {
/*					if (strstr(line, "noaccess def") != NULL) break; */ /* ? */
/*					printf("REJECTING: %s", line); */
				}
/*				printf("SHOULD BE END OF CHARSTRING:    %s", line); */
/*				c = getc(fp_in);
				if (c >= ' ') ungetc(c, fp_in);
				else {
					n = getline(fp_in, line);
					printf("FLUSHING BLANK LINE:   %s", line);
				} */
			}
			else {								/* OK, copy this one */
				charused[k] = 1;				/* note that seen */
				fputs(line, fp_out);
			}
			if (dotsflag != 0) putc('.', stdout);	/* 1992/Dec/9 ??? */
		}								/* end of '/' line */
		else fputs(line, fp_out);		/* NOT A '/' line */
		if (bAbort != 0) abortjob();	/* 1992/Nov/24 */
	}

/*	copy the tail across */
	if (n != 0) fputs(line, fp_out);
	while ((n = getline(fp_in, line)) != 0)	{
		fputs(line, fp_out);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* New stuff for shrinking down Subrs 1995/April/30 */

void copyfile (FILE *output, FILE *input) {
	int c;
	while ((c = getc(input)) != EOF) putc(c, output);
}

/* FROMCHAR if Subr called from character, FROMSUBR if called from Subr */

/* hvcblmi_.pln: 922 callsubr */ /* normal subroutine */
/* hvcblmi_.pln: 750 1 3 callothersubr */ /* hint replacement subroutine */
/* optionally translate to 750 4 callsubr */

int usesubrfour=0;		/* Subr 4 appears to be set up for hint switching */

/* 109 1 3 callsubr, pop, callsubr => 109 4 callsubr */

/* int nsubrs; */		/* already defined */

int nchars;
int notdef;				/* how many .notdef seen */

int notdefwidth, spacewidth;	/* width of .notdef and space respectively */

int scantosubrs (FILE *output, FILE *input) {
	char *s;

	if (traceflag) printf("Scanning up to Subrs\n");
	while (fgets(line, MAXLINE, input) != NULL) {
		if ((s = strstr(line, "/Subrs ")) != NULL) {
			if (sscanf(s, "/Subrs %d array", &nsubrs) == 1) {
				return nsubrs;
			}
		}
		if (output != NULL) fputs(line, output);
	}
	fprintf(stderr, "Cannot find /Subrs line\n");
	exit(1);
}

/* Step back one token from current position in string (which starts at start)*/

char *backonetoken (char *start, char *s) {			/* 1995/June/7 */
	while (s > start && *s == ' ') s--;	/* step back to token, if needed */
	if (s == start) return s;			/* failed... actually */
	while (s > start && *s > ' ') s--;	/* step back over the token */
	if (s == start) return s;			/* token is first thing in line */
	if (*s == ' ') s++;					/* step back to token again */
	return s;
}

/* scan Subrs and CharStrings for callsubr and callothersubr */

/* this gets run recursively */

int scanforsubrs (FILE *input) {
	char *s;
	char *sline;
	int k;
	int section=1;	/* one for Subrs, 2 for CharStrings */
	int subr=0;		/* current Subr we are in */
	int len;
	int lsb;		/* left side bearing */
	char charname[CHARNAME_MAX];

	if (traceflag) printf("Scanning for Subrs\n");
	notdef = 0;
	notdefwidth = 0;
	spacewidth = 0;
	while (fgets(line, MAXLINE, input) != NULL) {
/*	keep track of which section we are in */
		if ((s = strstr(line, "/CharStrings ")) != NULL) {
			if (sscanf(s, "/CharStrings %d dict", &nchars) == 1) {
				section=2;
				subr=0;
			}
		}
/*	in section 1, keep track of which Subr we are in */
		if (section == 1) {		
			if (strncmp(line, "dup ", 4) == 0) {
				if (sscanf (line, "dup %d %d RD", &subr, &len) == 2) {
				}
			}
		}
/*	in section 2, keep track of which /CharString we are in */
		else if (section == 2) {
			if (*line == '/') {
				sscanf(line, "/%s", charname);
				if (*rdsynom == '\0') 
					sscanf(line, "/%s %d %s", charname, &len, rdsynom);
				fgets(line, MAXLINE, input);	/* next line */
				if (strncmp(charname, ".notdef", 7) == 0) {
					sscanf(line, "%d %d hsbw", &lsb, &notdefwidth);
					notdef++;
				}
				if (strncmp(charname, "space", 5) == 0) {
					sscanf(line, "%d %d hsbw", &lsb, &spacewidth);
				}
			}
			if (strncmp(line, "endchar", 7) == 0) {
				fgets(line, MAXLINE, input);	/* next line */
				if (*ndsynom == '\0') sscanf(line, "%s", ndsynom);
			}
		}
/*	now check out possible subr call types */
		if (strncmp(line, "1 3 callothersubr", 17) == 0) {
			if (subr == 4) {
				usesubrfour=1;
			}
		}
/*	deal with new hint switching case */
		else if ((s = strstr(line, " 4 callsubr")) != NULL) {
			sline = backonetoken(line, s);			/* 95/June/7 */
/*			if (sscanf(line, "%d 4 callsubr", &k) == 1) { */
			if (sscanf(sline, "%d 4 callsubr", &k) == 1) {
				if (section > 1 || checksubr[subr])
/*					if (k >=0 && k < MAXSUBRS) hintsubr[k]=section; */
					if (k >=0 && k < maxsubrs) hintsubr[k]=(short) section; 
			}
		}
/*  deal with  `50 76 23 0 callsubr' case 95/June/7 */
		else if ((s = strstr(line, " 0 callsubr")) != NULL) {
			sline = backonetoken(line, s);			/* 95/June/7 */
		}
		else if ((s = strstr(line, " callsubr")) != NULL) {
			sline = backonetoken(line, s);			/* 95/June/7 */
/*			if (sscanf(line, "%d callsubr", &k) == 1) { */
			if (sscanf(sline, "%d callsubr", &k) == 1) {
				if (section > 1 || checksubr[subr])
/*					if (k >= 0 && k < MAXSUBRS) normsubr[k]=section; */
					if (k >= 0 && k < maxsubrs) normsubr[k]=(short) section;
			}
		}
		else if ((s = strstr(line, " 1 3 callothersubr")) != NULL) {
			sline = backonetoken(line, s);			/* 95/June/7 */
/*			if (sscanf(line, "%d 1 3 callothersubr", &k) == 1) { */
			if (sscanf(sline, "%d 1 3 callothersubr", &k) == 1) {
				if (section > 1 || checksubr[subr])
/*					if (k >= 0 && k < MAXSUBRS) hintsubr[k]=section; */
					if (k >= 0 && k < maxsubrs) hintsubr[k]=(short) section; 
			}
		}
	}
	return nchars;
}

/* this needs some work, since we don't set up RDsynom and NDsynom */

void addinnotdef (FILE *output, FILE *input) {
	int width;
	
	if (notdefwidth != 0) width = notdefwidth;
	else if (spacewidth != 0) width = spacewidth;
	else width = 333;
	
	fputs(line, output);			/* output the /CharString line */
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '\n') break;
		fputs(line, output);
	}								/* skip forward to blank line */
	fputs(line, output);			/* output the blank line */
	fprintf(output, "/.notdef 69 %s\n", rdsynom);	/* ??? */
	fprintf(output, "0 %d hsbw\n", width);
	fprintf(output, "endchar\n");
	fprintf(output, "%s\n", ndsynom);				/* ??? */
									/* leaves blank line behind in line */
}

void insertcharcount(char *line, int nchars) {
	char *s;
	int nold, n;

	if ((s = strstr(line, "/CharStrings ")) != NULL) {
		s += 13;
		if (sscanf(s, "%d%n", &nold, &n) == 1) {
			strcpy(buffer, s + n);
			sprintf(s, "%d", nchars);
			strcat(line, buffer);
		}
	}
}

void renumbersubrs (FILE *output, FILE *input) {
	char *s;
	char *sline;
	int k, len, n, newk;
	int section=1, subr;
	char charname[CHARNAME_MAX];

	if (traceflag) printf("Now renumbering the Subrs\n");
	while (fgets(line, MAXLINE, input) != NULL) {
		if ((s = strstr(line, "/CharStrings ")) != NULL) {
			if (sscanf(s, "/CharStrings %d dict", &nchars) == 1) {
				section=2;
				subr=0;
			}
			if (notdef != 1) insertcharcount(line, nchars + (1 - notdef));
			if (notdef == 0) {
				addinnotdef(output, input);
				notdef++;
			}
		}
/* while in section 1, keep track of which Subr */
		if (section == 1) {
			if (strncmp(line, "dup ", 4) == 0) {
				if (sscanf (line, "dup %d %d%n RD", &k, &len, &n) == 2) {
					strcpy(buffer, line+n);
					if (hintsubr[k] >= 0) newk = hintsubr[k]; 
					else if (normsubr[k] >= 0) newk = normsubr[k]; 
					else newk = -1;
					if (newk >= 0) {
						sprintf(line, "dup %d %d", newk, len);
						strcat(line, buffer);
					}
					else {					/* flush to end of Subr */
						while (fgets(line, MAXLINE, input) != NULL) {
							if (strncmp(line, "return", 6) == 0) {
								fgets(line, MAXLINE, input);
								fgets(line, MAXLINE, input);
								if (*line == '\n') {
									*line = '\0';	/* nuke this line too */
								}
								break;
							}
							if (*line == '\n') {
								*line = '\0'; 		/* nuke this line too */
								break;
							}
						}
					}
				}
				if (traceflag) fputs(line, stdout);
			}
		}	/* end of section 1 code */
/* while in section 2 keep track of charname */
		else if (section == 2) {
			if (*line == '/') {
				sscanf(line, "/%s", charname);
				if (traceflag) printf("%s (%d) ", charname, notdef); 
				if (notdef > 1) {	/* flush all but last .notdef */
					if (strncmp(charname, ".notdef", 7) == 0) {
						if (traceflag) printf("FLUSHING ");
						while (fgets(line, MAXLINE, input) != NULL) {
							if (strncmp(line, "endchar", 7) == 0) {
								fgets(line, MAXLINE, input);
								fgets(line, MAXLINE, input);
								if (*line == '\n') {
									*line = '\0';	/* nuke this line too */
								}
								break;
							}
						}
						if (*line == '\n') {
							*line = '\0'; 		/* nuke this line too */
						}
						notdef--;			/* reduce the number left in */
					}
				}	/* end of if (notdef > 1) */
			}
		}	/* end of section 2 code */
/* now check out those subr calls */
		if ((s = strstr(line, " 4 callsubr")) != NULL) {
			sline = backonetoken(line, s);			/* 95/June/7 */
/*			if (sscanf(line, "%d 4 callsubr", &k) == 1) { */
			if (sscanf(sline, "%d 4 callsubr", &k) == 1) {
				if (hintsubr[k] >= 0) 
/*					sprintf(line, "%d 4 callsubr\n", hintsubr[k]);  */
					sprintf(sline, "%d 4 callsubr\n", hintsubr[k]); 
				else fprintf(errout, "Impossible! %s", line);
			}
			else fprintf(errout, "Impossible!, %s", line);
			if (traceflag) fputs(line, stdout);
		}
/* protect calls of form `50 76 23 0 callsubr' 95/June/7 */
		else if ((s = strstr(line, " 0 callsubr")) != NULL) {
			sline = backonetoken(line, s);			/* 95/June/7 */
		}
		else if ((s = strstr(line, " callsubr")) != NULL) {
			sline = backonetoken(line, s);			/* 95/June/7 */
/*			if (sscanf(line, "%d callsubr", &k) == 1) { */
			if (sscanf(sline, "%d callsubr", &k) == 1) {
				if (normsubr[k] >= 0)
/*					sprintf(line, "%d callsubr\n", normsubr[k]); */
					sprintf(sline, "%d callsubr\n", normsubr[k]);
				else fprintf(errout, "Impossible! %s", line);
			}
			else fprintf(errout, "Impossible! %s", line);
			if (traceflag) fputs(line, stdout);
		}
		else if ((s = strstr(line, " 1 3 callothersubr")) != NULL) {
			sline = backonetoken(line, s);			/* 95/June/7 */
/*			if (sscanf(line, "%d 1 3 callothersubr", &k) == 1) { */
			if (sscanf(sline, "%d 1 3 callothersubr", &k) == 1) {
				if (hintsubr[k] >= 0) { 
					if (usesubrfour && replaceone) {
 						sprintf (line, "%d 4 callsubr\n", hintsubr[k]);
						fgets(line, MAXLINE, input);
						if (strncmp(line, "pop", 3) != 0)
							fprintf(errout, "Impossible! %s", line);
						fgets(line, MAXLINE, input);
						if (strncmp(line, "callsubr", 3) != 0)
							fprintf(errout, "Impossible! %s", line);
						sprintf (line, "%d 4 callsubr\n", hintsubr[k]);  
					}
/*					else sprintf(line, "%d 1 3 callothersubr\n", hintsubr[k]);*/
					else sprintf(sline, "%d 1 3 callothersubr\n", hintsubr[k]);
				}
				else fprintf(errout, "Impossible! %s", line);
				}
			else fprintf(errout, "Impossible! %s", line);
			if (traceflag) fputs(line, stdout);
		}
		fputs(line, output);
	}
}

int shownormsubrs(void) {
	int k, count=0;
	if (traceflag) printf("Normal Subrs needed: ");
/*	for (k = 0; k < MAXSUBRS; k++) */
	for (k = 0; k < maxsubrs; k++) {
		if (normsubr[k] >= 0) {
			if (traceflag) printf("%d ", k);
			if (hintsubr[k] < 0) 	/* don't double count */
				count++;
		}
	}
	if (traceflag) putc('\n', stdout);
	return count;
}

int showhintsubrs(void) {
	int k, count=0;
	if (traceflag) printf("Hint replacement Subrs needed: ");
/*	for (k = 0; k < MAXSUBRS; k++) */
	for (k = 0; k < maxsubrs; k++) {
		if (hintsubr[k] >= 0) {
			if (traceflag) printf("%d ", k);
			count++;
		}
	}
	if (traceflag) putc('\n', stdout);
	return count;
}

int assignnumbers (void) {
	int next=0;
	int k;

	if (traceflag) printf("Assigning new Subr numbers\n");
/*	for (k = 0; k < MAXSUBRS; k++) */
	for (k = 0; k < maxsubrs; k++) {
		if (normsubr[k] >= 0) {
			if (hintsubr[k] >= 0) hintsubr[k] = (short) next; 
			normsubr[k] = (short) next++;
		}
		else if (hintsubr[k] >= 0) hintsubr[k] = (short) next++; 
	}
	return next;
}

void insertsubrcount(char *line, int nsubrs) {
	char *s;
	int nold, n;

	if ((s = strstr(line, "/Subrs ")) != NULL) {
		s += 7;
		if (sscanf(s, "%d%n", &nold, &n) == 1) {
			strcpy(buffer, s + n);
			sprintf(s, "%d", nsubrs);
			strcat(line, buffer);
		}
	}
}

/* prune list of subrs to check */ /* returns those dropped this round */

int prunesubr (void) {
	int k, dropped=0;

	if (traceflag) printf("Now attempting to prune list of Subrs needed\n");
/*	for (k = 0; k < MAXSUBRS; k++) { */
	for (k = 0; k < nsubrs; k++) {
		if (checksubr[k] == 0) continue;		/* already removed */
		if (hintsubr[k] == FROMCHAR ||
			normsubr[k] == FROMCHAR) continue;	/* called from character */
		if (hintsubr[k] < 0 && normsubr[k] < 0) {
			if (checksubr[k]) dropped++;		/* was used in last round */
			checksubr[k] = 0;					/* nobody calling this one */
		}
	}
/*	for (k = nsubrs; k < MAXSUBRS; k++) checksubr[k] = 0; */
	for (k = nsubrs; k < maxsubrs; k++) checksubr[k] = 0;
	if (verboseflag)
		if (dropped) printf("Dropping %d Subrs\n", dropped);
	return dropped;
}

void shrinkpfa (FILE *output, FILE *input) {
/*	FILE *input, *output; */
/*	int firstarg=1; */
	int k, n;
/*	char infile[FILENAME_MAX], outfile[FILENAME_MAX]; */
	int  normcount, hintcount, nstrip;

/* are these already allocated here ? */

	if (maxsubrs == 0) fprintf(errout, "ERROR: allocation\n");
/*	for (k = 0; k < MAXSUBRS; k++) checksubr[k]=1; */
	for (k = 0; k < maxsubrs; k++) checksubr[k]=1;
	for (n = 0; n < 32; n++) {
/*		for (k = 0; k < MAXSUBRS; k++) normsubr[k]=-1; */
		for (k = 0; k < maxsubrs; k++) normsubr[k]=-1;
/*		for (k = 0; k < MAXSUBRS; k++) hintsubr[k]=-1; */
		for (k = 0; k < maxsubrs; k++) hintsubr[k]=-1;
		for (k = 0; k < 4; k++) normsubr[k]=1; 
/*		for (k = 0; k < 4; k++) hintsubr[k]=1; */
		if (verboseflag) printf("Subr pruning pass %d\n", n+1);
		nsubrs = scantosubrs(NULL, input);
		if (nsubrs < 0) {
			fprintf(stderr, "Can't find Subrs array\n");
/*			fclose(input); */
			exit(1);
		}
		if (n == 0) {				/* first pass only */
			if (verboseflag) printf("Font contains %d Subrs\n", nsubrs);
		}

		nchars = scanforsubrs(input);
		if (n == 0) {				/* first pass only */
			if (verboseflag) {
				printf("Font contains %d CharStrings\n", nchars);
				if (usesubrfour)
					printf("Can use Subr 4 for efficient hint switching\n");
				if (notdef == 0) 
					printf("WARNING: Font does not contain a definition for .notdef\n");
				else if (notdef > 1)
					printf("WARNING: Font contains %d definitions for .notdef\n",
						   notdef);
			}
		}

		if (usesubrfour >= 0) normsubr[4] = 1;
		rewind (input);
		if (prunesubr() == 0) {
			if (verboseflag) printf("No more Subrs can be stripped out\n");
			break;
		}
	}

	normcount = shownormsubrs();
	hintcount = showhintsubrs();
	if (verboseflag)
		printf("Need %d normal Subrs and %d hint switching Subrs\n",
			   normcount, hintcount);
	if (normcount + hintcount == nsubrs && notdef == 1) {
		if (verboseflag) printf("Nothing to do!\n");
		copyfile(output, input);
/*		fclose(output); */
/*		fclose(input); */
		return;
	}
/*	if (traceflag) printf("Assigning new Subr numbers\n"); */
	assignnumbers ();

	rewind(input);
/*	strcpy(outfile, infile);
	forceexten(outfile, "coa");
	if ((output = fopen(outfile, "w")) == NULL) {
		perror(outfile);
		exit(1);
	} */
	scantosubrs(output, input);
	if (traceflag) printf("New Subr count %d\n", normcount + hintcount);
	insertsubrcount(line, normcount + hintcount);
	if (traceflag) fputs(line, stdout);
	nstrip = nsubrs - (normcount + hintcount);
	if (nstrip > 0) {
		if (verboseflag)
			printf("Will strip out %d unused Subrs (leaving %d)\n",
				   nstrip, normcount + hintcount);
	}
	fputs(line, output);
	if (adjustnotdef == 0) notdef = 1;					/* not debugged yet */
	else if (notdef == 0) {
		if (verboseflag) printf("Will add a definition for .notdef\n");
	}
	else if (notdef > 1) {
		if (verboseflag) 
			printf("Will remove %d redundant CharStrings for .notdef\n",
				   notdef-1);
	}
/*	if (traceflag) printf("Renumbering Subrs\n"); */
	renumbersubrs(output, input);
	if (adjustnotdef && notdef != 1) {
		if (verboseflag) 
			printf("Unable to adjust number of .notdef CharStrings\n");
	}
/*	fclose(output); */
/*	fclose(input); */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	if (quietflag == 0) {
/*		printf("%s <pfa-file> <character list>\n", s); */
		printf("%s [{-s}{-f}{-o}] -c=<character list> <pfa-file>\n", s);
		printf("\tv Be verbose\n");
		printf("\ts Do not remove unused Subrs\n");
		printf("\tf Do not replace old hint switching calls\n");
		printf("\to Make sure output has exactly one .notdef\n");
		printf("\te Exclude characters in list rather than include\n");
		printf("\n");
		printf("\tc File with list of glyphs to include or exclude\n");
		printf("\t  If glyph list is omitted, only remove unused Subrs\n");		
	}
	exit(1);
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

char fn_tmp[FILENAME_MAX];					/* 1995/April/30 */

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
		fclose(fp_dec);						/* close output */
/*		fp_dec = fopen(fn_dec, "wb");		
		if (fp_dec != NULL) fclose(fp_dec); */
		wipefile (fn_dec);
		if (wipeclean > 0) {
			if (remove(fn_dec))	/* remove bogus file */
				printf("Removal of %s failed\n", fn_dec);
		}
	}
	if (fp_pln != NULL) {
		fclose(fp_pln);				/* close output */
/*		fp_pln = fopen(fn_pln, "wb");	
		if (fp_pln != NULL) fclose(fp_pln); */
		wipefile (fn_pln);
		if (wipeclean > 0) {
			if (remove(fn_pln)) /* remove bogus file */
				printf("Removal of %s failed\n", fn_pln);
		}
	}
	if (fp_adj != NULL) {
		fclose(fp_adj);				/* close output */
/*		fp_adj = fopen(fn_adj, "wb");	
		if (fp_adj != NULL) fclose(fp_adj); */
		wipefile (fn_adj);
		if (wipeclean > 0) {
			if (remove(fn_adj)) /* remove bogus file */
				printf("Removal of %s failed\n", fn_adj);
		}
	}
}

char *strippath(char *pathname);

void abortjob() {
	fprintf(stderr, "\nUser Interrupt - Exiting\n"); 
	cleanup();
	if (renameflag != 0) {
/*		if (rename(fn_in, fn_bak)) */
		if (rename(fn_in, strippath(fn_bak)))
			printf("Rename of %s to %s failed\n", fn_in, strippath(fn_bak));
	}
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
	if (bAbort++ >= 15)	exit(3);		/* emergency exit */
	(void) signal(SIGINT, ctrlbreak); 
}

int decodeflag (int c) {
	switch(c) { 
		case 'v': verboseflag = 1; dotsflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case '?': detailflag++; return 0;
		case 'd': wipeclean++; return 0;
		case 's': shrinkflag = 1-shrinkflag; return 0;
		case 'e': excludeini = 1-excludeini; return 0;
		case 'f': replaceone = 1-replaceone; return 0;
		case 'o': adjustnotdef = 1-adjustnotdef; return 0;
		case 'c': characterflag = 1; return -1; /* needs argument */
		default: {
				fprintf(errout, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
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
		if (traceflag)
			fprintf(errout, "Can't open `%s' (%s %s)\n", new, name, ext);
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

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);	 
/*	fprintf(output, "%s (%s %s)\n", programversion, date, compiletime); */
	fprintf(output, "%s - %s - version %s (%s %s)\n", 
		progname, progfunction, progversion, date, compiletime); 
}

void uppercase (char *s) {
	int c;
	while ((c = *s) != '\0') {
		if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
		*s = (char) c;
		s++;
	}
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
			if (characterflag != 0) {
				charfile = s;
				characterflag = 0; 
			}
			break;	/* default - no flag set */
		}
	}
	return firstarg;
}

/*	check command line flags and command line arguments */

int commandline(int argc, char *argv[], int firstarg) {
	int c;
	
/*	if (argc < 2) showusage(argv[0]); */
	if (argc < 1) showusage(argv[0]);		/* 1995/April/30 */
	c = argv[firstarg][0];
	while (c == '-' || c == '/') {
		firstarg = decodearg(argv[firstarg], argv[firstarg+1], firstarg+1);
		if (firstarg >= argc) break;			/* safety valve */
		c = argv[firstarg][0];
	}
	return firstarg;
}

int _cdecl main(int argc, char *argv[]) {       /* main program */
    FILE *fp_in, *fp_out, *fp_des;
	int firstarg=1;
/*	unsigned int i; */
	int c, d;
	int m; 
	char *s;

	if (strlen(progname) < 16) strcpy(progname, compilefile);
	if ((s = strchr(progname, '.')) != NULL) *s = '\0';
	uppercase(progname);

	strcpy (line, argv[0]);
	uppercase (line);
	if (strstr(line, progname) == NULL) quietflag = 1; 

/*	printf("argc %d firstarg %d\n", argc, firstarg); */ /* debug */
	
	if (argc < firstarg+1) 	showusage(argv[0]); 

	firstarg = commandline(argc, argv, 1);

/*	printf("argc %d firstarg %d\n", argc, firstarg); */ /* debug */

	if (wipeclean == 4) wipeclean = 0;	/* `ddd' on command line */

	if (argc <= firstarg) showusage(argv[0]); 
		
	if (quietflag != 0) {
		verboseflag = 0; traceflag = 0; dotsflag = 0;
	}

/*	scivilize(compiledate);	 */
	if (quietflag == 0) stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	(void) signal(SIGINT, ctrlbreak);

	if ((s = getenv("TMP")) != NULL) tempdir = s;
	if ((s = getenv("TEMP")) != NULL) tempdir = s;

/*	printf("Subfont extraction program version %s\n", VERSION); */

	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

/*	stringindex=0; */		/* reset string pool table */

/*	if (argc < firstarg + 2) showusage(argv[0]); */

	if (strcmp(charfile, "") == 0) {
/*		following is for back-ward compatability 1994/June/2 */
		if (firstarg+1 < argc) {
			charfile = argv[firstarg+1];
/*			firstarg++; */
		}
	}

	if (strcmp(charfile, "") == 0 && shrinkflag == 0) showusage(argv[0]); 

/*	If no char list, just do Subr trimming part of this */
	
	m = firstarg;
	strcpy(fn_in, argv[m]);			/* get file name */
	extension(fn_in, "pfa");

/*	if (verboseflag != 0) printf("Processing file %s\n", fn_in); */

	if ((fp_in = fopen(fn_in, "rb")) == NULL) {	/* see whether exists */
		underscore(fn_in);						/* 1993/May/30 */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
			perror(fn_in);	exit(13);
		}
	}
/*	else { */
		c = fgetc(fp_in); d = fgetc(fp_in);
		if (c != '%' || d != '!') {
			fprintf(errout, "ERROR: This does not appear to be a PFA file\n");
			exit(13);							/* 1993/May/30 */
		}
		fclose(fp_in);
/*		fp_in=NULL; */
/*	} */

	if (verboseflag != 0) printf("Processing file %s\n", fn_in);

	if ((s=strrchr(fn_in, '\\')) != NULL) s++;
	else if ((s=strrchr(fn_in, ':')) != NULL) s++;
	else s = fn_in;
	strcpy(fn_out, s);			/* copy input file name minus path */

/*	if (forceoutname != 0) strcpy(fn_out, outfilename); */
	if (forceoutname != 0) {
		if (quietflag == 0) strcpy(fn_out, progname);	/* SUBFONT */
		else strcpy(fn_out, extractpath(argv[0]));	/* use prog name */
	}
	forceexten(fn_out, "pfa");

	if (strcmp(fn_in, fn_out) == 0 && writeoutflag != 0) {
		strcpy(fn_bak, fn_in);
		forceexten(fn_in, "bak");
		printf("Renaming %s to %s\n", fn_bak, fn_in);
		if (remove(fn_in))		/* in case backup version already present */
			printf("Removal of %s failed\n", fn_in);
		if (rename(fn_bak, strippath(fn_in)))
			printf("Rename of %s to %s failed\n", fn_bak, strippath(fn_in));
		renameflag++;
	}
	
	if (verboseflag != 0 && writeoutflag == 0) 
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
	if (writeoutflag != 0) {
/*		strcpy(fn_adj, fn_out); */
/*		makename(fn_adj, "adj"); */
		maketemporary(fn_adj, fn_out, "adj");
		if (traceflag != 0) printf("Using %s as temporary\n", fn_adj);
	}

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (traceflag != 0) printf("Pass A (down) starting\n");			/* */
		
	if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
		underscore(fn_in);						/* 1993/May/30 */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
			perror(fn_in); exit(2);
		}
	}
	if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
		perror(fn_dec); exit(2);
	}

	eexecscan = 1;
	charscan = 0;  decodecharflag = 0;  charenflag = 0; binaryflag = 0;

	(void) decrypt(fp_dec, fp_in);		/* actually go do the work */

	fclose(fp_in);
/*	fp_in=NULL; */

	if (ferror(fp_dec) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec); cleanup();	exit(2);
	}
	else fclose(fp_dec);
/*	fp_dec=NULL; */

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
/*	fp_dec=NULL; */

	if (ferror(fp_pln) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_pln); cleanup();
		exit(2);
	}
	else fclose(fp_pln);
/*	fp_pln = NULL; */

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (debugflag != 0) _getch();

/*	NOW HAVE PLN FILE IN SIGHT */

	if (traceflag != 0) printf("Extraction of CharStrings starting\n"); 

	if ((fp_pln = fopen(fn_pln, "rb")) == NULL) { 
		perror(fn_pln);  cleanup(); exit(2);
	}

	if (writeoutflag != 0) {
		if ((fp_adj = fopen(fn_adj, "wb")) == NULL) { 
			perror(fn_adj); cleanup(); exit(2);
		}
	}

	if (strcmp(charfile, "") == 0) {	/* 1995/April/30 */
		printf("No characters specified\n");
		if (shrinkflag == 0) showusage(argv[0]);
		copyfile(fp_adj, fp_pln);
		goto skipsubfont;
	}

	if (verboseflag != 0) printf("Pass 0\n");
	(void) extractcharnames(fp_pln);	/* extract info first */
	rewind(fp_pln);

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

/*	if (argc > firstarg + 1) { */	/* now go and read the list of desired */
	if (strcmp(charfile, "") != 0) {/* now go and read the list of desired */
/*		strcpy(fn_des, argv[firstarg+1]);	 */
		strcpy(fn_des, charfile);	
		extension(fn_des, "chr");			
		if (verboseflag != 0) printf("Reading file %s\n", fn_des);
		if ((fp_des = fopen(fn_des, "r")) == NULL) {	
			perror(fn_des);	exit(13);
		}
		readdesired (fp_des);
		fclose(fp_des);
/*		fp_des=NULL; */
	}

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (writeoutflag == 0) return 0;		/* while debugging */

	if (verboseflag != 0) printf("Pass 1\n");	/* now modify and remove */

	(void) processpfa(fp_adj, fp_pln);		/* where work actually gets done */
	fclose(fp_pln);
/*	fp_pln = NULL; */

	if (ferror(fp_adj) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_adj); cleanup(); exit(2);
	}
	else fclose(fp_adj);
/*	fp_adj=NULL; */

	if (verboseflag != 0) putc('\n', stdout);

skipsubfont:

	if (shrinkflag && writeoutflag) {		/* 1995/April/30 */
		if ((fp_adj = fopen(fn_adj, "rb")) == NULL) { 
			perror(fn_adj);  cleanup(); exit(2);
		}
		if ((fp_pln = fopen(fn_pln, "wb")) == NULL) { 
			perror(fn_pln); cleanup(); exit(2);
		}
		if (traceflag)
			printf ("Reading %s and writing %s\n", fn_adj, fn_pln);
		shrinkpfa (fp_pln, fp_adj);	/* need to write new code here */

		fclose(fp_adj); 
/*		fp_adj = NULL; */
		if (ferror(fp_pln) != 0) {
			fprintf(stderr, "Output error ");
			perror(fn_pln); cleanup(); exit(2);
		}
		else fclose(fp_pln);
/*		fp_pln = NULL; */

#ifdef IGNORED 
/*		switch .adj and .pln files */
		maketemporary (fn_tmp, fn_out, "tmp");
		if (remove (fn_tmp)) {
			printf("Removal of %s failed\n", fn_tmp);
			perror(fn_tmp);
		}
		if (traceflag) printf("Renaming %s => %s\n", fn_pln, fn_tmp);
		if (rename (fn_pln, strippath(fn_tmp))) {	/* rename pln to tmp */
			printf("Rename %s => %s failed\n", fn_pln, strippath(fn_tmp));
			perror(fn_tmp);
			if ((fp_des = fopen(fn_tmp, "r")) != NULL) {
				printf("%s exists\n", fn_tmp);
				fclose(fp_des);
			}
			exit(1);
		}
		if (traceflag) printf("Renaming %s => %s\n", fn_adj, fn_pln);
		if (rename (fn_adj, strippath(fn_pln)))	{	/* rename adj to pln */
			printf("Rename %s => %s failed\n", fn_adj, strippath(fn_pln));
			perror(fn_pln);
			if ((fp_des = fopen(fn_pln, "r")) != NULL) {
				printf("%s exists\n", fn_pln);
				fclose(fp_des);
			}
			exit(1);
		}
		if (traceflag) printf("Renaming %s => %s\n", fn_tmp, fn_adj);
		if (rename (fn_tmp, strippath(fn_adj))) {	/* rename tmp to adj */
			printf("Rename %s => %s failed\n", fn_tmp, strippath(fn_adj));
			perror(fn_adj);
			if ((fp_des = fopen(fn_adj, "r")) != NULL) {
				printf("%s exists\n", fn_adj);
				fclose(fp_des);
			}
			exit(1);
		}
#endif
/* alternate way after above couldn't be made to work */
		if ((fp_pln = fopen(fn_pln, "rb")) == NULL) {
			perror(fn_pln);  cleanup(); exit(2);
		}
		if ((fp_adj = fopen(fn_adj, "wb")) == NULL) {
			perror(fn_adj);  cleanup(); exit(2);
		}
		copyfile(fp_adj, fp_pln);
		fclose(fp_pln);
		if (ferror(fp_adj)) {
			fprintf(stderr, "Output error ");
			perror(fn_adj); cleanup(); exit(2);
		}
		else fclose(fp_adj);
	}

/*	exit(1); */ 	/* temp */

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass B (up) starting\n"); 

	if ((fp_adj = fopen(fn_adj, "rb")) == NULL) { 
		perror(fn_adj);  cleanup(); exit(2);
	}
	if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
		perror(fn_dec); cleanup(); exit(2);
	}
	if (traceflag) printf("Reading %s and writing %s\n", fn_adj, fn_dec);

	eexecscan = 1; charenflag = 1; charscan = 1;

	(void) encrypt(fp_dec, fp_adj);	/* actually go do the work */
		
	fclose(fp_adj);
/*	fp_adj=NULL; */

	if (ferror(fp_dec) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec); cleanup(); exit(2);
	}
	else fclose(fp_dec);
/*	fp_dec=NULL; */

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass A (up) starting\n"); 

	if ((fp_dec = fopen(fn_dec, "rb")) == NULL) { 
		perror(fn_dec); cleanup();	exit(2);
	}
	if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
		perror(fn_out); cleanup();	exit(2);
	}
	if (traceflag) printf("Reading %s and writing %s\n", fn_dec, fn_out);

	eexecscan = 1; charenflag = 0; charscan = 0;

	(void) encrypt(fp_out, fp_dec);	/* actually go do the work */
		
	fclose(fp_dec);
/*	fp_dec=NULL; */

	if (bAbort != 0) abortjob();		/* 1992/Nov/24 */

	if (debugflag != 0) _getch();

	if (ferror(fp_out) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_out); cleanup();	exit(2);
	}
	else fclose(fp_out);
/*	fp_out=NULL; */
	if (verboseflag) printf("Output in %s\n", fn_out);
	cleanup();
/*	freecharnames(); */ /* implicit */
	freechars();	
	freesubrs();
	return 0;
}	

/* do not remove base and accent if accented characters present */

/* remove Subrs that are not used ? */

/* keep .notdef CharString ? */

/* remove unused characters from encoding also ? */
