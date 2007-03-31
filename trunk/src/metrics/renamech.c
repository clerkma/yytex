/* renamech.c
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

#define PADSPACES 20

int wantcpyrght=1;

/* Font merging and normalizing program version */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* char *programversion = "RENAMECH - rename CharStrings - version 1.0"; */

/* char *progname="RENAMECH"; */

char progname[16]=""; 

/* char *progversion="1.0.1"; */
/* char *progversion="1.0.2"; */
/* char *progversion="1.1"; */
/* char *progversion="1.1.1"; */		/* 97/Jan/12 */
char *progversion="2.0";				/* 98/Dec/25 */

char *copyright = "\
Copyright (C) 1992-2000  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1992--1997  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

char *newextension="";

/* #define COPYHASH 2670755 */
/* #define COPYHASH 13986445 */
/* #define COPYHASH 14501591 */
/* #define COPYHASH 5618236 */
/* #define COPYHASH 6490707 */
/* #define COPYHASH 3523562 */
/* #define COPYHASH 3956063 */
#define COPYHASH 15480306

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int debugflag = 0;
int wipeclean = 1;	
int dotsflag = 1;
int quietflag = 0;

int extenflag = 0;
int renfileflag = 0;

// int renameflag = 0;		/* if needed to make "bak" file ... */

int stripunknown = 0;	/* remove unknown chars from Encoding */
int forcenotdef = 1;	/* force in /.notdef line in Encoding */
int switchcase = 0;		/* rename upper case to lower case and vice versa */

int eexecscan = 0;		/* scan up to eexec before starting */
int charscan = 0;		/* scan up to RD before starting */
int charenflag = 0;		/* non-zero charstring coding instead of eexec */
int decodecharflag = 0;	/* non-zero means also decode charstring */
int binaryflag = 0;		/* input is binary, not hexadecimal */

volatile int bAbort = 0;			/* set when user types control-C */

int extractflag = 1;	/* non-zero => extract info only */

extern int encrypt(FILE *, FILE *);

extern int decrypt(FILE *, FILE *);

char *tempdir="";		/* place to put temporary files */

char *renamefile="";	/* renaming file */

FILE *errout=stdout;	/* 1997/Dec/20 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_bak[FILENAME_MAX];

int nchrs=0;			/* number of characters in mapping table */

int maxchars=0;			/* allocated space in charold, charnew */

/* char *charold[MAXCHARINFONT]; */	/* old character names */
char * __far *charold=NULL;

/* char *charnew[MAXCHARINFONT]; */	/* new character names */
char * __far *charnew=NULL;

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

/* dotlessi => I ??? */

/************************************************************************/

void freecharnames(void) {
	int k;
	if (charnew != NULL) {
		for (k = 0; k < maxchars; k++) {
			if (charnew[k] != NULL) free(charnew[k]);
			charnew[k] = NULL;

		}
	}
	if (charold != NULL) {
		for (k = 0; k < maxchars; k++) {
			if (charold[k] != NULL) free(charold[k]);
			charold[k] = NULL;
		}
	}
}

void freechars(void) {
	if (maxchars > 0) freecharnames();
	if (charnew != NULL) free(charnew);
	if (charold != NULL) free(charold);
	charnew = charold = NULL;
	maxchars = 0;
}

void allocchars(int nchars) {
	int k;
	if (maxchars > 0) freechars();
	maxchars = 0;
	if (traceflag) printf("Allocating space for %d chars\n", nchars);
	charnew = (char * __far *) _fmalloc(nchars * sizeof(char *));
	charold = (char * __far *) _fmalloc(nchars * sizeof(char *));
	if (charnew == NULL || charold == NULL){
		fprintf(errout, "ERROR: failed memory allocation of %d bytes\n",
				nchars * sizeof(char *));
		exit(1);
	}
	for (k = maxchars; k < nchars; k++) charnew[k] = charold[k] = NULL;
	maxchars = nchars;
}

void reallocchars(int nchars) {
	int k;
	if (traceflag) printf("Reallocating space for %d => %d chars\n", maxchars, nchars);
	charnew = (char * __far *) _frealloc(charnew, nchars * sizeof(char *));
	charold = (char * __far *) _frealloc(charold, nchars * sizeof(char *));
	if (charnew == NULL || charold == NULL){
		fprintf(errout, "ERROR: failed memory allocation of %d bytes\n",
				nchars * sizeof(char *));
		exit(1);
	}
	for (k = maxchars; k < nchars; k++) charnew[k] = charold[k] = NULL;
	maxchars = nchars;
}

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

void abortjob();

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef IGNORED
char *strdupx(char *name) {				/* our own cheap _strdup */
	char *s;
	int n = strlen(name) + 1;

	if (stringindex + n >= MAXSTRINGSPACE) {
		fprintf(stderr,
				"ERROR: Out of memory for character names (%s)\n", name);
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

static char buffer[MAXLINE];		/* buffer for splicing purposes */

static int getline(FILE *fp_in, char *line) {
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	return strlen(line); 
} 

int lookup (char *name) {
	int k;
	for (k = 0; k < nchrs; k++) {
/*		if (strcmp(charold[k], "") == 0) continue; */
		if (charold[k] == NULL) continue;
		if (strcmp(charold[k], name) == 0) return k;
	}
	return -1;
}

/* grab next white space delimited token in line */ /* read line if needed */
/* assumes pump has been primed, i.e. line read and strtok called once */

/*
char *grabnexttoken(FILE *input, char *line) {
	char *str=NULL, *s;

	while ((s = strtok(str, " \t\n\r")) == NULL) {
		for(;;) {	
			if (fgets(line, MAXLINE, input) == NULL) return NULL;	/
			if (*line != '%' && *line != '\n' && *line != '\r') break;
		}
		str = line;
	}
	return s;
} */

char *grabnexttoken(FILE *input, char *line) { /* 1992/Sep/17 */
	char *str=NULL, *s;

	for (;;) {
		while ((s = strtok(str, " \t\n\r")) == NULL) {
			for(;;) {					/* need to grab a new line then */
				if (fgets(line, MAXLINE, input) == NULL) return NULL;/* EOF */
/*				ignore comments and blank lines - continue round the loop */
				if (*line != '%' && *line != '\n' && *line != '\r') break;
			}
			str = line;
		}
		if (*s != '%') break;		/* escape if not comment */
/*		following added to strip comments off ends of lines 1992/Sep/17 */
		for(;;) {					/* need to grab a new line then */
			if (fgets(line, MAXLINE, input) == NULL) return NULL;/* EOF */
/*			ignore comments and blank lines - continue round the loop */
			if (*line != '%' && *line != '\n' && *line != '\r') break;
		}
		str = line;
	}
/*	printf("%s ", s); getch();	 */		/* debugging */
	return s;
}

/*	new tokenized version follows */
void gobbleencoding (FILE *output, FILE *input) {
	int k, chr;
	char *s, *t;

/*	fputs(line, stdout); */	/* DEBUGGING */

/*	may want to remove some debugging error message output later ... */
	s = strtok(line, " \t\n\r");	/* start the pipeline */
	for (;;) {					/*	exit if hit `readonly' or `def' ??? */
		if (strcmp(s, "dup") != 0) {
			if (strcmp(s, "readonly") == 0 ||
				strcmp(s, "def") == 0) break;	/* normal exit */
			fprintf(errout, "Expecting `dup', not: `%s' ", s);
			break;
		}

		if ((s = grabnexttoken(input, line)) == NULL) break;
		if (sscanf(s, "%d", &chr) < 1) {
			fprintf(errout, "Expecting number, not: `%s' ", s);
			break;
		}

		if ((t = strchr(s, '/')) != NULL) s = t;	/* 1992/Aug/21 */
		else if ((s = grabnexttoken(input, line)) == NULL) break;

		if (*s != '/') 	fprintf(errout, "Bad char code `%s' ", s);
		else s++;
		if (chr >= 0 && chr < MAXCHRS && strlen(s) < MAXCHARNAME) {
/*			printf("%d: %s ", chr, s); */ /* debugging */
			k = lookup(s);
			if (k >= 0) {
/* flush characters mentioned in mapping file, but without replacement */
/*				if (strcmp(charnew[k], "") != 0) */
				if (charnew[k] != NULL)
					fprintf(output, "dup %d /%s put\n", chr, charnew[k]);
			}
			else if (stripunknown == 0)
				fprintf(output, "dup %d /%s put\n", chr, s);
		}
		else fprintf(errout, "Invalid char number %d ", chr);

		if ((s = grabnexttoken(input, line)) == NULL) break;
		if (strcmp(s, "put") != 0) {
			fprintf(errout, "Expecting `put' not: `%s' ", s);
/*			break; */
		}
		if ((s = grabnexttoken(input, line)) == NULL) break;
		if (bAbort != 0) abortjob();					/* 1992/Nov/24 */
	}

/* deal with terminating line of Encoding */
	fprintf (output, "%s ", s); 		/* dump last token */
	while ((s = strtok(NULL, " \t\n\r")) != NULL) { 
		fprintf (output, "%s ", s); 		/* dump last token */
	}	
	putc('\n', output);
/*	need to clean out current line at all ? */
}

/* scan first part of font to change encoding */

int remapencoding (FILE *output, FILE *input) {
	char *s=NULL;
	int ndict;

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == '\n') {
			fputs (line, output);
			continue;
		}
		if ((s = strstr(line, "/Encoding")) != NULL) break;
		fputs (line, output);
	}
	if (s == NULL) return -1;
	if (sscanf(s,  "/Encoding %d dict", &ndict) < 1) ndict = 256;
	fputs (line, output);
	if (strstr(line, "StandardEncoding") != NULL) {
		fprintf(errout, "Will not change StandardEncoding vector\n");
		return -1;
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line != '%' || *line != '\n') break;
		fputs (line, output);
	}
	if (strstr(line, "/.notdef") != NULL) {
		fputs (line, output);
		while (fgets(line, MAXLINE, input) != NULL) {
			if (*line != '%' || *line != '\n') break;
			fputs (line, output);
		}
	}
	else if (forcenotdef != 0) {
		fprintf(errout, "Inserting /.notdef line\n");
		fprintf(output, "0 1 %d {1 index exch /.notdef put} for\n", ndict-1);
	}
	gobbleencoding(output, input);
	return 0;
}

/* grovel over plain decrypted code */

int extractside(FILE *fp_out, FILE *fp_in, int extractflag) {
	char charname[CHARNAME_MAX];
	int k, n, m;
	int unknowns=0, count=0;
	char *s;
/*	int side, width;  */
	
/*  scan up to eexec encrypted part */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
		if (extractflag == 0) fputs(line, fp_out);
	}
	if (extractflag == 0) fputs(line, fp_out);
/*	scan up to CharStrings */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "/CharStrings") == NULL) {
		if (extractflag == 0) fputs(line, fp_out);
	}
	if (extractflag == 0) fputs(line, fp_out);
	if (verboseflag != 0) printf("Reached the CharStrings\n"); 
/*	scan all of the CharStrings */
	for (;;) {
		n = getline(fp_in, line);
		if (n <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;
/*		if (extractflag == 0) fputs(line, fp_out); */
		if ((s = strchr(line, '/')) != NULL) {
			sscanf(s, "/%s%n", charname, &m);
/*			if (dotsflag != 0) putc('.', stdout); */
			if ((k = lookup(charname)) >= 0) {
/*				if (strcmp(charnew[k], "") != 0)  */
				if (charnew[k] != NULL) {
					strcpy(buffer, s+m);	/* save rest of line */
					strcpy(s+1, charnew[k]);
					if (traceflag != 0) printf("%s => %s ", charname, s+1);
					strcat(line, buffer);	/* tag it back on again */
					count++;
				}
				if (dotsflag != 0) putc('.', stdout);	/* moved 94/June/7 */
			}
			else if (strcmp(charname, ".notdef") != 0) {
//				if (verboseflag) printf("%s ", charname);
				unknowns++;
			}
			if (extractflag == 0) fputs(line, fp_out);
			while ((n = getline(fp_in, line)) != 0 && 	
				strstr(line, "sbw") == NULL) {
				if (extractflag == 0) fputs(line, fp_out);
			}
			fputs(line, fp_out);	/* put the hsbw line ... */
		}
		else if (extractflag == 0) fputs(line, fp_out);
	}
	if (extractflag == 0) {		/* copy the tail across */
		if (n != 0) fputs(line, fp_out);
		while ((n = getline(fp_in, line)) != 0)	fputs(line, fp_out);
	}
	if (dotsflag != 0 || unknowns > 0) putc('\n', stdout);
	if (count != 0) 
		printf("Names of %d CharStrings were altered\n", count);
	if (unknowns != 0) 
		printf("Names of %d CharStrings were not altered\n", unknowns);
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

int readmapping(FILE *input) {
	int k=0, n;
/*	char *s; */
	char charnamea[MAXTOKEN], charnameb[MAXTOKEN];

/*	for (k = 0; k < MAXCHARINFONT; k++) charold[k] = NULL; */
	for (k = 0; k < maxchars; k++) charold[k] = NULL;
/*	for (k = 0; k < MAXCHARINFONT; k++) charnew[k] = NULL; */
	for (k = 0; k < maxchars; k++) charnew[k] = NULL;

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
/*		flush white space at start of line 97/Jan/14 */
		while (*line <= ' ' && *line > 0) strcpy(line, line+1);
		if (*line == '\0') continue;
		if ((n = sscanf(line, "%s %s", charnamea, charnameb)) < 2) {
/*			printf ("n = %d charnamea %s charnameb %s line %s\n",
				n, charnamea, charnameb, line); */
			if (n < 1) {
				fprintf(errout, "Don't understand: %s", line);
				continue;				/* 1993/June/20 */
			}
/*			else strcpy(charnameb, charnamea); */
			else {
/*				strcpy(charnameb, ""); */
				*charnameb = '\0';
/*				printf("%s will be removed from encoding\n", charnamea);*/
			}
		}
		if (k >= maxchars) {
			reallocchars(maxchars+maxchars);
		}
		if (strlen(charnamea) > MAXCHARNAME ||
				strlen(charnameb) > MAXCHARNAME) {
			fprintf(errout, "Char names too long: %s", line);
		}
/*		else if (k >= MAXCHARINFONT) {
			fprintf(errout, "Too many characters\n");
			return k;
		} */
		else {
/*			strcpy(charold[k], charnamea); */
/*			charold[k] = strdupx(charnamea); */
			charold[k] = zstrdup(charnamea);
/*			strcpy(charnew[k], charnameb); */
/*			charnew[k] = strdupx(charnameb); */
			charnew[k] = zstrdup(charnameb);
			k++;
		}		
	}

	return k;
}

void showmapping (void) {
	int k;
	for (k = 0; k < nchrs; k++) {
/*		if (strcmp(charold[k], "") == 0) continue; */
		if (charold[k] == NULL) continue;
/*		if (strcmp(charnew[k], "") == 0) continue; */
		if (charnew[k] == NULL) continue;
		printf("%d\t%s => %s\n", k, charold[k], charnew[k]);
	}
}

int checkmapping (void) {
	int i, j, bugs=0;
	for (i=0; i < nchrs; i++) {
/*		if (strcmp(charold[i], "") == 0) continue; */
		if (charold[i] == NULL) continue;
		if (strpbrk(charold[i], "()/\\") != NULL)	/* 96/Apr/22 */
			fprintf(errout, "WARNING: Bad char name: %s\n", charold[i]);
		for (j=i+1; j < nchrs; j++) {
/*			if (strcmp(charold[j], "") == 0) continue; */
			if (charold[j] == NULL) continue;
			if (strcmp(charold[i], charold[j]) == 0) {
				fprintf(errout, "WARNING: Repeated `from' char name:\n"); 
				fprintf(errout, "\t%s %s\n", charold[i], charnew[i]);
				fprintf(errout, "\t%s %s\n", charold[j], charnew[j]);
				bugs++;
			}
		}
	}
	for (i=0; i < nchrs; i++) {
/*		if (strcmp(charnew[i], "") == 0) continue; */
		if (charnew[i] == NULL) continue;
		if (strpbrk(charnew[i], "()/\\") != NULL)	/* 96/Apr/22 */
			fprintf(errout, "WARNING: Bad char name: %s\n", charnew[i]);
		for (j=i+1; j < nchrs; j++) {
/*			if (strcmp(charnew[j], "") == 0) continue; */
			if (charnew[j] == NULL) continue;
			if (strcmp(charnew[i], charnew[j]) == 0) {
				fprintf(errout, "WARNING: Repeated `to' char name:\n");
				fprintf(errout, "\t%s %s\n", charold[i], charnew[i]);
				fprintf(errout, "\t%s %s\n", charold[j], charnew[j]);
				bugs++;
			}
		}
	}
	return bugs;
}

/* replace name, but only, if non-null *//* writes result back into arg */
/* returns 0 if not found in old, 1 if no replacement, -1 if replaced */

int remap (char *name) {
	int k;
	for (k = 0; k < nchrs; k++) {
		if (charold[k] == NULL) continue;
		if (strcmp(charold[k], name) == 0) {
/*			if (strcmp(charnew[k], "") != 0)  */
			if (charnew[k] != NULL) {
				strcpy(name, charnew[k]);
				return -1;
			}
			return 1;
		}
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

/* Stuff for dealing with AFM files */

void renamekernpair(void) {
	char charnamea[MAXTOKEN], charnameb[MAXTOKEN];
	double kern;

	if (sscanf (line, "KPX %s %s %lg", charnamea, charnameb, &kern) < 3) {
		fprintf(errout, "Don't understand: %s", line);
	}
	else {
		remap(charnamea); remap(charnameb);
		sprintf(line, "KPX %s %s %lg\n", charnamea, charnameb, kern);
	}
}

void renamecomposite(void) {
	char charnamea[MAXTOKEN], charnameb[MAXTOKEN], charnamec[MAXTOKEN];
	int z, xa, ya, xb, yb;

	if (sscanf (line, "CC %s %d ; PCC %s %d %d ; PCC %s %d %d",
		charnamea, &z, charnameb, &xa, &ya, charnamec, &xb, &yb) < 8) {
		fprintf(errout, "Don't understand: %s", line);
	}
	else {
		remap(charnamea); remap(charnameb); remap(charnamec);
		sprintf(line, "CC %s %d ; PCC %s %d %d ; PCC %s %d %d ;\n", 
			charnamea, z, charnameb, xa, ya, charnamec, xb, yb);
	}
}

int unencode;

void renamecharmetric(void) {
	char charnamea[MAXTOKEN], charnameb[MAXTOKEN], charnamec[MAXTOKEN];
	int z, n, k;
	double wx, xll, yll, xur, yur;
	char *s, *t;
	
	if (sscanf (line, "C %d ; WX %lg ; N %s ; B %lg %lg %lg %lg%n",
		&z, &wx, charnamea, &xll, &yll, &xur, &yur, &n) < 7) {
		fprintf(errout, "Don't understand: %s", line);
	}
	else {
		strcpy(buffer, line+n);	/* save the tail - ligatures */
		k = remap(charnamea); 
		if (k > 0) {
			z = -1;	/* make character unencoded if listed without */
			unencode++;
		}
		sprintf(line, "C %d ; WX %lg ; N %s ; B %lg %lg %lg %lg ;", 
			z, wx, charnamea, xll, yll, xur, yur);
		if ((s = strchr(buffer, ';')) != NULL) {
			while ((s = strchr(s, 'L')) != NULL) {
				if (sscanf(s, "L %s %s%n", charnameb, charnamec, &n) < 2){
					fprintf(errout, "Don't understand: %s", s);
				}
				else {
					remap(charnameb);  remap(charnamec); 
					t = line + strlen(line);
					sprintf(t, " L %s %s ;", charnameb, charnamec);
					s = s + n;
/*					printf("n = %d rest = %s", n, s); */ /* debugging */
				}
			}
		}
		strcat(line, "\n");
	}
}

/* main entry point for AFM version */

void renameafmchar (FILE *output, FILE *input) {
	unencode = 0;
	while (fgets(line, MAXLINE, input) != NULL) {
/*		if (verboseflag != 0) fputs(line, stdout); */	/* debugging */
		if (*line == '%' || *line == ';' || *line == '\n') {
			fputs (line, output); continue;
		}
		else if (strncmp(line, "C ", 2) == 0) renamecharmetric();
		else if (strncmp(line, "CC ", 3) == 0) renamecomposite();
		else if (strncmp(line, "KPX ", 4) == 0) renamekernpair();
		fputs (line, output); 
	}
	if (unencode > 0) 
		printf("%d characters dropped from encoding\n", unencode);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	if (quietflag != 0) exit(1);
/*	fprintf(errout, "%s [-{v}{l}{u}] <pfa-file> [<name remapping file>]\n", s);*/
	fprintf(errout, "%s [-{v}{l}{u}] [-r=<name remapping file>] <pfa-file>\n", s);
	if (detailflag == 0) exit(1);
	fprintf(errout, "\tv verbose mode\n");
	fprintf(errout, "\tl switch case (upper to lower and vice versa)\n");
	fprintf(errout, "\tu remove unknown chars from Encoding\n");
	fprintf(errout, "\tr name remapping file with old-name new-name pairs\n");
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
    (void) mktemp(fname);		/* replace template by unique string */
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

void abortjob() {
	fprintf(stderr, "\nUser Interrupt - Exiting\n"); 
	cleanup();
//	if (renameflag != 0) rename(fn_in, fn_bak);
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
	if (bAbort++ >= 15) exit(3);		/* emergency exit */
	(void) signal(SIGINT, ctrlbreak);
}

int decodeflag (int c) {
	switch(c) { 
		case 'v': verboseflag = 1; dotsflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case '?': detailflag++; return 0;
		case 'u': stripunknown++; return 0;		
/*		case 's': switchcase++; return 0; */
		case 'l': switchcase++; return 0;
		case 'd': wipeclean++; return 0;
		case 'e': extenflag++; return -1;	/* needs argument */
		case 'r': renfileflag++; return -1;	/* needs argument */
		default: {
				fprintf(errout, "Invalid command line flag '%c'", c);
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
	fprintf(stderr, "HASHED %ld\n", hash);	(void) getch(); 
	return hash;
}

char *progfunction="rename CharStrings";

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);	 
/*	fprintf(output, "%s (%s %s)\n", programversion, date, compiletime); */
	fprintf(output, "%s - %s - version %s (%s %s)\n",
		progname, progfunction, progversion, date, compiletime);
}

void lowercase (char *s) {
	int c;
	while ((c = *s) != 0) {
		if (c >= 'A' && c <= 'Z') *s = (char) (c + 'a' - 'A');
		s++;
	}
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
			if (extenflag != 0) {
				newextension = s;
				extenflag = 0; 
			}
			if (renfileflag != 0) {
				renamefile = s;
				renfileflag = 0; 
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

/* addition to renamech if first line of file is `%%' */

int renameoutchar(FILE *output, FILE *input) {		/* 1994/May/28 */
	int n, chrs;
/*	int m; */
	int width;
	char charname[CHARNAME_MAX];
/*	int c, cold; */
	int newcharflag = 0;
	char *s;

/*	process what appears to be an OUT file ...  */

/*	if (verboseflag != 0) printf(" => %s\n", outfilename); */

	while(fgets(buffer, MAXLINE, input) != NULL) {
		if (*buffer == '%' || *buffer == ';' || *buffer == '\n') {
			fputs(buffer, output);	continue;
		}		/* ignore comments */
		if (newcharflag) {
			if (sscanf(buffer, "%d %d %% %n%s",
				&chrs, &width, &n, &charname) == 3) {
				s = buffer + n;
				while (*s <= ' ' && *s != '\0') s++;
				if (remap(s) < 0) ;
				else if (verboseflag) printf("%s?", charname);
			}
			else fprintf(errout, "Don't understand: %s", buffer);
			newcharflag = 0;
		}
		else if (*buffer == ']') newcharflag++;
		fputs(buffer, output);
	}
	return 0;
}

int _cdecl main (int argc, char *argv[]) {       /* main program */
    FILE *fp_in, *fp_out;
/*	unsigned int i; */
	int firstarg=1;
/*	unsigned int i; */
	int c, d;
	int afmfileflag;
	int outfileflag;
	int k;
	int m;
	char *s;
	char letter[2];

	if (strlen(progname) < 16) strcpy(progname, compilefile);
	if ((s = strchr(progname, '.')) != NULL) *s = '\0';
	uppercase(progname);

	strcpy (line, argv[0]);
	uppercase (line);
	if (strstr(line, progname) == NULL) {
/*		printf("compilename %s argv[0] %s\n", progname, line); */
		quietflag = 1;
	}

	if (argc < firstarg+1) showusage(argv[0]); 

	firstarg = commandline(argc, argv, 1);

	if (wipeclean == 4) wipeclean = 0;	/* `ddd' on command line */

	if (argc <= firstarg) showusage(argv[0]); 
		
	if (quietflag != 0) {
		verboseflag = 0; traceflag = 0; dotsflag = 0;
	}

/*	scivilize(compiledate);	 */
	if (quietflag == 0) stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	(void) signal(SIGINT, ctrlbreak);
/*	(void) signal(SIGINT, ctrlbreak); */

	if ((s = getenv("TMP")) != NULL) tempdir = s;
	if ((s = getenv("TEMP")) != NULL) tempdir = s;

/*	printf( "Character Name Changing Program Version %s\n",
		VERSION); */

	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	if (strcmp(renamefile, "") == 0) {		/* rename file not given */
/*		following is for backward compatability 1994/June/2 */
		if (firstarg+1 < argc) {
			renamefile = argv[firstarg+1];
			printf("Using %s as renaming file\n", renamefile);
/*			firstarg++; */
		}
	}
/*	stringindex=0; */		/* reset string pool table */
/*	for (k = 0; k < MAXCHARINFONT; k++) charold=NULL; */
/*	for (k = 0; k < MAXCHARINFONT; k++) charnew=NULL; */

	allocchars(NCHARACTERS);		/* initial allocation */

	if (strcmp(renamefile, "") != 0) {		/* rename file given */
/*		if (readrenaming(renamefile) != 0) return 0; */

/*	if (argc > firstarg + 1) {	*//* more than one argument given */
/*		strcpy(fn_in, argv[firstarg+1]); */	/* get file name */
		strcpy(fn_in, renamefile);			/* get file name */
		extension(fn_in, "ren");			/* extension if not given */
		if (verboseflag != 0) printf("Reading name mapping file %s\n", fn_in);
		if ((fp_in = fopen(fn_in, "r")) == NULL) {	/* see whether exists */
			underscore(fn_in);
			if ((fp_in = fopen(fn_in, "r")) == NULL) {/* see whether exists */
				perror(fn_in);	exit(13);
			}
		}

		nchrs = readmapping (fp_in);
		(void) checkmapping();
		if (traceflag) showmapping();

		fclose(fp_in);
		extractflag = 0;
		if (nchrs == 0) {
			fprintf(stderr, "ERROR: Character renaming file empty\n");
			exit(14);
		}
		else if (verboseflag != 0)
			printf("%d characters in renaming table\n", nchrs);
	}
	else if (switchcase != 0) {
		nchrs = 0;
		for (k = 0; k < 26; k++) {
/*			charold[nchrs][0] = (char) ('A' + k); */
/*			charold[nchrs][1] = '\0'; */
			letter[0] = (char) ('A' + k);
			letter[1] = '\0'; 
/*			charold[nchrs] = strdupx(letter); */
			charold[nchrs] = zstrdup(letter);
/*			charnew[nchrs][0] = (char) ('a' + k); */
/*			charnew[nchrs][1] = '\0'; */
			letter[0] = (char) ('a' + k);
			letter[1] = '\0'; 
/*			charnew[nchrs] = strdupx(letter); */
			charnew[nchrs] = zstrdup(letter);
			nchrs++;
		}
		for (k = 0; k < 26; k++) {
/*			charold[nchrs][0] = (char) ('a' + k); */
/*			charold[nchrs][1] = '\0'; */
			letter[0] = (char) ('a' + k);
			letter[1] = '\0'; 
/*			charold[nchrs] = strdupx(letter); */
			charold[nchrs] = zstrdup(letter);
/*			charnew[nchrs][0] = (char) ('A' + k); */
/*			charnew[nchrs][1] = '\0'; */
			letter[0] = (char) ('A' + k);
			letter[1] = '\0'; 
/*			charnew[nchrs] = strdupx(letter); */
			charnew[nchrs] = zstrdup(letter);
			nchrs++;
		}
/*	now for the composites */
		for (k = 0; k < 29; k++) {
			if (strcmp(composites[k], "") == 0) break;
/*			strcpy(charold[nchrs], composites[k]); */
/*			charold[nchrs] = composites[k]; */ /* OK ??? */
			charold[nchrs] = zstrdup(composites[k]);
/*			strcpy(charnew[nchrs], composites[k]); */
			charnew[nchrs] = zstrdup(composites[k]);
			lowercase(charnew[nchrs]);
			nchrs++;			
		}
		for (k = 0; k < 29; k++) {
			if (strcmp(composites[k], "") == 0) break;
/*			strcpy(charold[nchrs], composites[k]); */
			charold[nchrs] = zstrdup(composites[k]);
			lowercase(charold[nchrs]);
/*			strcpy(charnew[nchrs], composites[k]); */
/*			charnew[nchrs] = composites[k]; */ /* OK ??? */
			charnew[nchrs] = zstrdup(composites[k]);
			nchrs++;			
		}
/* now for the specials */
			for (k = 0; k < 6; k++) {
			if (strcmp(specials[k], "") == 0) break;
/*			strcpy(charold[nchrs], specials[k]); */
/*			charold[nchrs] = specials[k]; */ /* OK ??? */
			charold[nchrs] = zstrdup(specials[k]);
/*			strcpy(charnew[nchrs], specials[k]); */
			charnew[nchrs] = zstrdup(specials[k]);
			lowercase(charnew[nchrs]);
			nchrs++;			
		}
		for (k = 0; k < 6; k++) {
			if (strcmp(specials[k], "") == 0) break;
/*			strcpy(charold[nchrs], specials[k]); */
			charold[nchrs] = zstrdup(specials[k]);
			lowercase(charold[nchrs]);
/*			strcpy(charnew[nchrs], specials[k]); */
/*			charnew[nchrs] = specials[k]; */  /* OK ??? */
			charnew[nchrs] = zstrdup(specials[k]);
			nchrs++;			
		}
		(void) checkmapping();
		extractflag = 0;		
	}
	else {
		extractflag = 1;	/* meaningless here ? */
		fprintf(stderr,
			"ERROR: Must specify name remapping file (or case switch)\n");
		exit(5);
	}

	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */

/*	for (m = firstarg; m < argc, m++) { */			/* FUTURE ? */
	m = firstarg;

	afmfileflag=0;
	outfileflag=0;

	if (m >= argc) exit(1);						/* impossible ... */

/*	strcpy(fn_in, argv[firstarg]); */			/* get file name */
	strcpy(fn_in, argv[m]);			/* get file name */
	extension(fn_in, "pfa");

	if (verboseflag != 0) printf("Processing file %s\n", fn_in);

	if ((fp_in = fopen(fn_in, "rb")) == NULL) {	/* see whether exists */
		underscore(fn_in);						/* 1993/May/30 */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {	/* see whether exists */
			perror(fn_in);	exit(13);
		}
	}
	c = fgetc(fp_in); d = fgetc(fp_in);
/*	if (c != '%' || d != '!') { */
	if (c == 'S' && d == 't') {
		if (verboseflag != 0)
			printf("NOTE: This appears to be an AFM file\n");
		afmfileflag++;
	}
	if (c == '%' && d == '%') {
		if (verboseflag != 0)
			printf("NOTE: This appears to be an OUT file\n");
		outfileflag++;
	}
	fclose(fp_in);

	if ((s=strrchr(fn_in, '\\')) != NULL) s++;
	else if ((s=strrchr(fn_in, ':')) != NULL) s++;
	else s = fn_in;
	strcpy(fn_out, s);			/* copy input file name minus path */
	forceexten(fn_out, "xxx");	/* avoid conflict */
//	if (strcmp(newextension, "") != 0) {
//		forceexten (fn_out, newextension);
//	}

//	if (strcmp(fn_in, fn_out) == 0 && extractflag == 0) {
//		strcpy(fn_bak, fn_in);
//		forceexten(fn_in, "bak");
//		printf("Renaming %s to %s\n", fn_bak, fn_in);
//		remove(fn_in);		/* in case backup version already present */
//		rename(fn_bak, fn_in);
//		renameflag++;
//	}
	
	if (verboseflag != 0 && extractflag == 0) 
		printf("Output is going to %s\n", fn_out);

	if (afmfileflag || outfileflag) {
		if ((fp_in = fopen(fn_in, "r")) == NULL) { 
			underscore(fn_in);						/* 1993/May/30 */
			if ((fp_in = fopen(fn_in, "r")) == NULL) { 
				perror(fn_in); exit(2);
			}
		}
		if ((fp_out = fopen(fn_out, "w")) == NULL) { 
			perror(fn_out); exit(2);
		}
/*		if (verboseflag != 0) printf("Entering RENAMEAFMCHAR\n"); */
		if (afmfileflag) renameafmchar (fp_out, fp_in);
		if (outfileflag) renameoutchar (fp_out, fp_in);
/*		if (verboseflag != 0) printf("Leaving RENAMEAFMCHAR\n"); */
		fclose (fp_in);
		if (ferror(fp_out) != 0) {
			fprintf(stderr, "Output error ");
			perror(fn_out);	exit(2);
		}
		else {
			fclose(fp_out);
			strcpy(line, fn_out);
			forceexten(line, "afm");
			remove(line);
			rename(fn_out, line);
			printf("Output renamed %s\n", line);

		}
		return 0;
	}

	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */

/*	here we set up intermediate files and such */

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
	if (ferror(fp_dec) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec); cleanup();	exit(2);
	}
	else fclose(fp_dec);

	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */
	
	if (debugflag != 0) getch();

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

	if (debugflag != 0) getch();
	
	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */
	
/*	following shouldn't ever kick in */

	if (extractflag) {		/* no need to go upward */
		if ((fp_pln = fopen(fn_pln, "rb")) == NULL) {
			perror(fn_pln); cleanup(); exit(33);
		}
		forceexten(fn_out, "sid");
		if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
			perror(fn_out); cleanup();	exit(2);
		}

		(void) extractside(fp_out, fp_pln, 1);	/* extract side bearings */

		fclose(fp_pln);
/*		if (wipeclean > 0) remove(fn_pln);  */
		if (ferror(fp_out) != 0) {
			fprintf(stderr, "Output error ");
			perror(fn_out); cleanup(); exit(2);
		}
		else fclose(fp_out);
		cleanup();
		return 0;
	}

	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */
	
	if (traceflag != 0) printf("Renaming of CharStrings starting\n"); 

	if ((fp_pln = fopen(fn_pln, "rb")) == NULL) { 
		perror(fn_pln);  cleanup(); exit(2);
	}
	if ((fp_adj = fopen(fn_adj, "wb")) == NULL) { 
		perror(fn_adj); cleanup(); exit(2);
	}

	(void) remapencoding(fp_adj, fp_in);

	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */
	
	(void) extractside(fp_adj, fp_pln, 0);
		
	fclose(fp_pln);
/*	if (wipeclean > 0) remove(fn_pln);  */
	if (ferror(fp_adj) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_adj); cleanup(); exit(2);
	}
	else fclose(fp_adj);
	
	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */

	if (debugflag != 0) getch();

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

	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */

	if (debugflag != 0) getch();

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

	if (bAbort != 0) abortjob();					/* 1992/Nov/24 */

	if (debugflag != 0) getch();

/*	if (wipeclean > 0) remove(fn_dec);  */
	if (ferror(fp_out) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_out); cleanup();	exit(2);
	}
	else {
		fclose(fp_out);
		strcpy(line, fn_out);
		if (*newextension != '\0') forceexten(line, newextension);
		else forceexten(line, "pfa");
		remove(line);
		rename(fn_out, line);
		printf("Output renamed %s\n", line);
	}

	cleanup();
/*	freecharnames(); */ /* implicit */
	freechars();
	return 0;
}	

/* strip out unused CharStrings ? NO */

/* also: make REN file a command line argument rather then second arg */

