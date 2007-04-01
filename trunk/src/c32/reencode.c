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

/* Program for inserting new encoding vector in PFB or PFA file */

/* #define FNAMELEN 80 */
#define CHARNAME_MAX 64
#define FONTNAME_MAX 128
#define NUMBERSTR_MAX 32
#define BUFFERLEN 1024
#define MAXCHRS 256
#define MAXCHARNAME 32
/* MAXENCODING may need extra space, since there may be remapping */
#define MAXENCODING 256 * 64

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>		/* for atan */
#include <direct.h>		/* for _getcwd */
#include <malloc.h>	

/* following needed so can copy date and time for standardize and reinstate */

#include <time.h>

#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>

#ifdef _WIN32
#define __far
#define _frealloc realloc
#define _fmalloc malloc
#define _ffree free
#endif

int wantcpyrght=1;		/* want copyright message in there */

/* char *version = "Version 1.6"; */
/* char *version = "Version 1.7"; */
/* char *version = "Version 1.8"; */		/* 96/Apr/14 */
/* char *version = "Version 1.8.1"; */		/* 96/June/22 */
/* char *version = "Version 1.8.2"; */		/* 97/Jul/22 */
#ifdef _WIN32
char *version = "Version 2.0";			/* 98/Mar/22 */
#else
char *version = "Version 1.8.3";			/* 98/Mar/22 */
#endif

/* #define COPYHASH 2244854 */
/* #define COPYHASH 9796728 */
/* #define COPYHASH 16104577 */
/* #define COPYHASH 199832 */
/* #define COPYHASH 11095065 */
/* #define COPYHASH 12093609 */
/* #define COPYHASH 12526110 */
#define COPYHASH 7273137

char *copyright = "\
Copyright (C) 1990--2000, Y&Y, Inc.  (978) 371-3286  http://www.YandY.com";
/* Copyright (C) 1990--1996, Y&Y, Inc. All rights reserved. (508) 371-3286"; */
/* Copyright (C) 1990--1993, Y&Y, Inc. All rights reserved. (508) 371-3286";*/
/* Copyright (C) 1991  Y&Y. All rights reserved. (508) 371-3286"; */

/* char encoding[MAXENCODING];	*/		/* use allocated memory for this ? */

char __far *encoding;	/* PostScript code for Encoding */ /* 1993/July/13 */

char mapping[MAXCHRS][MAXCHARNAME+1];

int conflicts[MAXCHRS];

char buffer[BUFFERLEN];

/* char *vecpath = "c:\\dvipsone"; */	/* default encoding vectors paths */
char *vecpath = "c:\\yandy\\fonts\\encoding";

char *vectorname = "";

char *newfontname = "";
char *newfullname = "";				/* 96/Apr/14 */
char *newfamilyname = "";			/* 96/Apr/14 */
char *newversion="";				/* 96/Apr/14 */
char *notice1="";
char *notice2="";
char *fontbbox="";

char *revisiondate="";				/* 96/Apr/18 */

char *afmfilename="";				/* 96/Apr/14 */

char *unique="";

char *oblique="";

char oldfontname[FONTNAME_MAX] = "";

char programpath[FILENAME_MAX] = "";

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;

int vectorflag = 0;				/* next arg is encoding vector */
int newnameflag = 0;			/* next arg is new FontName */
int familynameflag = 0;			/* next arg is new FamilyName 96/Apr/14 */
int fullnameflag = 0;			/* next arg is new FullName 96/Apr/14 */
int obliqueflag = 0;			/* next arg is slant */
int uniqueflag = 0;				/* next arg is new Unique ID */
int versionflag = 0;			/* next arg is version number */
int afmfileflag = 0;			/* next arg is AFM file */
/* int revisionflag = 0; */		/* next arg is revision date */

int renameonly = 0;				/* change name only, not encoding */

int stripcontrolm=0;			/* strip control-M in output */

int copydate = 1;				/* copy file date for standardize/reinstate */
int suppressx = 0;				/* suppress `x' on output file name */
int removeX = 0;				/* remove `x' on output file name */
int suppresscontrol = 0;		/* suppress encoding in control char range */
int remapflag = 0;				/* try to use lucida style remapping */
int killunique = 0;				/* kill UniqueID */
int stripcomments = 0;			/* strip comments from AFM file */

int usetoday = 0;				/* todays date for CreationDate 96/Apr/20 */
int usefiledate = 0;			/* use file date for CreationDate 96/Apr/20 */
int dontmapspace = 0;			/* don't screw with 32 => 195 */

int reinstateflag = 0;			/* if reinstating StandardEncoding */
int forcestandard = 0;			/* reinstate StandardEncoding 96/June/20 */
int nonstandard = 0;			/* if other than standard encoding */
int standardize = 0;			/* standardize file with StandardEncoding */

int noticedone = 1;				/* have /Notice(s) been spit out ? 96/Apr/14 */

int pass = 0;					/* keep track of pass in PFB file */

int usingreturn;				/* file uses return instead of linefeed */
int usingboth;					/* file uses return/linefeed combination */

long uniqueID=-1;				/* new unique ID */

double slant=0.0;				/* non-zero if oblique specified */

long asciilength, newlength, linestart;

long encodingline;				/* not accessed */

char *standard = "/Encoding StandardEncoding def\n";

FILE *errout=stdout;

/* int offset=0; */				/* offset in encoding number 1994/May/26 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Let's see if there is space for this here 1993/July/13 */

static char *standardencoding[] = { 
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
 "", "", "", "", "", "", "", "",
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

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */ /* 1993/July/11 */

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

struct _stat statbuf;	/* struct stat statbuf;	 */

struct _utimbuf timebuf;	/* struct utimbuf timebuf; */

int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(errout, "Unable to obtain info on %s\n", filename);
		return -1;
	}
	s = ctime(&statbuf.st_atime);		/* ltime */
	lcivilize(s);								
	if (verboseflag != 0) printf("%s last modified: %s", filename, s);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int lookup (char *name) {
	int k;
	if (strlen(name) == 1) {		/* speedup for letters 1994/May/29 */
		k = *name;
		if (strcmp(name, mapping[k]) == 0) return k;		
	}
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(name, mapping[k]) == 0) return k;
	}
	return -1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* only needed for check on PS interpreter bug with accents missing */

#define MAXACCENTS 16

char *accents[] = {
"dotlessi",									/* put this `accent' first */
"grave", "acute", "circumflex", "tilde", 
"macron", "breve", "dotaccent",
"dieresis", "ring",
"cedilla",
"hungarumlaut", "ogonek",
"caron",
/* "dot", */
""};

/* dotlessi, grave, acute, circumflex, tilde, dieresis, ring, cedilla, caron */
/* occur in `standard' 58 accented/composit characters in Adobe fonts */

int accenthit[MAXACCENTS];			/* whether accent called for and missing */

/* `i' => `dotlessi', `dot' => `dotaccent' */

int checkcomp (char *name) {
	int c, k, flag = 0;

	c = *name;										/* get first letter */
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
		for (k = 1; k < MAXACCENTS; k++) {			/* check for accent name */
			if (strcmp(accents[k], "") == 0) break;	/* end of list */
			if (strcmp(accents[k], name+1) == 0) {
				if (c == 'i') {						/* check for dotlessi */
					if (lookup("dotlessi") < 0) {
						if (traceflag != 0) puts("dotlessi");
						accenthit[0]++;
						flag = -1;
					}
				}
				if (lookup(name+1) < 0) {			/* check for accent */
					if (traceflag != 0) puts(name);
					accenthit[k]++;
					flag = -1; break;
				}
			}
		}
		return flag;
	}
	else return 0;
}

/* do this only if PFB files being processed ? */

int checkencoding (void) {
	int k, count=0;
	for (k = 0; k < MAXACCENTS; k++) accenthit[k] = 0;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(mapping[k], "") == 0) continue;
		if (checkcomp(mapping[k]) != 0) count++;
	}
	if (count != 0) {
		if (verboseflag != 0) {				/* list missing accents ? */
			for (k = 0; k < 16; k++) {
				if (accenthit[k] != 0) printf("`%s' ", accents[k]);
			}
			putc('\n', stdout);
		}
printf("WARNING: Encoding does not contain all accents needed for accented chars\n");
printf("         Some PS interpreters require accents - suggest use of SAFESEAC on PFB\n");
	}
	return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef IGNORE
void extension(char *name, char *ext) {
	if (strchr(name, '.') == NULL) {
		strcat(name, ".");
		strcat(name, ext);
	}
}

void forceexten(char *name, char *ext) {
	char *s;
	if ((s = strchr(name, '.')) != NULL) *s = '\0';
	strcat(name, ".");
	strcat(name, ext);
}
#endif

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

char *stripname(char *name) {
	char *s;
	if ((s = strrchr(name, '\\')) != NULL) return (s+1);
	if ((s = strrchr(name, '/')) != NULL) return (s+1);
	if ((s = strchr(name, ':')) != NULL) return (s+1);
	else return name;
}

long readfour(FILE *input) {
	int c, d, e, f;
	c = getc(input); d = getc(input); e = getc(input); f = getc(input);
	return (((long) f) << 24) | (((long) e) << 16) | (((long) d) <<	8) | c;
}

void writefour(long n, FILE *output) {
	int k;
	for (k = 0; k < 4; k++) {
		fputc((int) (n & 255), output);
		n = (n >> 8);						/* slightly suspicious ??? */
	}
}

void shortASCII (FILE *output) {
		fputs( 
"\nRecommend passing this MAC style font file through PFBTOPFA & PFATOPFB\n",
			output);
		fputs(
"ATM for Windows may have problems with multiple ASCII records",
			output);
}

void showbad (int c, FILE *output) {
	if (pass != 0) return;
	if (pass == 0) putc('\n', output);
	fputs("WARNING: Bad character in ASCII section ", output);
	if (c > 127) {
		fputs("M-", output);	c = c - 128;
	}
	if (c < 32) {
		fputs("C-", output);	c = c + 64;
	}
	putc(c, output);
	putc('\n', output);
}

int readline(char *s, int n, FILE *input) {
	int c, d, k=0;

	c = getc(input);
	while (c != '\n' && c != '\r' && c != EOF) {
		if (c > 127 || (c < 32 && c != '\t' && c != '\f')) {
			if (c != 128) {
/*				fprintf(errout, "Bad character (%d) in ASCII section\n", c);*/
				showbad(c, errout);
/*				return 0; */
				if (c == 0) c = ' ';	/* to prevent miscount */
				*s++ = (char) c;	k++;	*s = '\0';
				return k;
			}
			d = getc(input);
			if (d != 1) {
/*				fprintf(errout, "Bad character (%d) in ASCII section\n", c); */
				showbad(c, errout);				
/*				return 0;   */
				if (c == 0) c = ' ';	/* to prevent miscount */
				*s++ = (char) c;	k++;	*s = '\0';
				return k;
			}
/* come here if sequence 128, 1 is seen => start of new ASCII section */
			shortASCII(errout);
			asciilength += readfour(input);
			c = getc(input);		/* ? 1992/Dec/5 */
			continue;				/* ? 1992/Dec/5 */
		}		/* end of weird character case */
		*s++ = (char) c;
		if (k++ > n-2) {
			fprintf(errout, "ERROR: Input line too long\n");
/*			return 0; */
			*s = '\0';				/* ? 1992/Dec/5 */
			return k;
		}
		c = getc(input);		
	}			/* end of while loop */
	if (c == EOF) {
		if (n == 0) {
			fprintf(errout, "ERROR: Unexpected EOF\n");
			return 0;
		}
		else {			/* just lacking return at end ? */
			*s = '\0';
			return k;
		}
	}
	*s++ = (char) c;
	k++;
	if (c == '\r') {
		c = getc(input);
		if (c == '\n') {
			*s++ = (char) c;
			k++;
			usingboth = 1;
		}
		else {
			(void) ungetc(c, input);
			usingreturn = 1;
		}
	}
	*s = '\0';
	return k;
}

/* void changetolinefeed(char *s) {	
	int c;
	while ((c = *s) != 0) {
		if (c == '\r') *s = '\n';
		s++;
	}
} */

void changetolinefeed(char __far *s) {	/* \r => \n */
	int c;
	while ((c = *s) != 0) {
		if (c == '\r') *s = '\n';
		s++;
	}
}

/* void changetoreturn(char *s) {	
	int c;
	while ((c = *s) != 0) {
		if (c == '\n') *s = '\r';
		s++;
	}
} */

void changetoreturn(char __far *s) {	/* \n => \r */
	int c;
	while ((c = *s) != 0) {
		if (c == '\n') *s = '\r';
		s++;
	}
}

void readencoding(FILE *input) {
	int k, nchar;
	char charname[CHARNAME_MAX];
	int offset=0;

	for (k = 0; k < MAXCHRS; k++) strcpy(mapping[k], "");

	while(fgets(buffer, BUFFERLEN, input) != NULL) {
		if (strncmp(buffer, "% OFFSET", 8) == 0) {	/* 1994/May/26 */
			if (sscanf(buffer+8, "%d", &offset) == 1) {
				if (verboseflag != 0)
					printf("OFFSET %d ", offset);
			}
			else fprintf(errout, "Do not understand: %s", buffer);
/*			else offset = 0; */
/*			continue; */
		}
		if (*buffer == '%' || *buffer == ';' || *buffer == '\n') continue;
		if (sscanf(buffer, "%d  %s", &nchar, charname) < 2) {
			if (traceflag)
				fprintf(errout, "WARNING: Don't understand: %s", buffer);
		}
		else {
			if (offset != 0) nchar += offset;		/* 1994/May/26 */
			if (nchar >= 0 && nchar < MAXCHRS) {
				if (suppresscontrol == 0 ||			/* 1993/June/21 */
					(nchar >= 32 && nchar < 128) ||
					(nchar >= 160 && nchar < 256))
					strncpy(mapping[nchar], charname, MAXCHARNAME);
			}
/*			else  */
			else if (nchar != -1)					/* 1993/June/21 */
/*				fprintf(errout, "Code number out of range %d\n", nchar); */
				fprintf(errout, "WARNING: Code number out of range: %s", 
					buffer);
		}
	}
}

/* returns zero if remapping is OK */
/* returns > 0 if conflicts were found */
/* returns < 0 if nothing found in encoding to remap */

int checkremap (void) {			/* check whether safe to remap 1993/May/29 */
	int k, count=0, nconflict = 0;

	for (k = 0; k < MAXCHRS; k++) conflicts[k] = 0;

	for (k = 0; k < 10; k++) {
		if (strcmp(mapping[k], "") != 0) {
			if (strcmp(mapping[k+161], "") != 0) {
				conflicts[k+161]++;	nconflict++;
			}
			count++;
		}
	}
	for (k = 10; k < 32; k++) {
		if (strcmp(mapping[k], "") != 0) {
			if (strcmp(mapping[k+163], "") != 0) {
				conflicts[k+163]++; nconflict++;
			}
			count++; 
		}
	}
	if (strcmp(mapping[32], "") != 0 && dontmapspace == 0) {
		if (strcmp(mapping[195], "") != 0) {
				conflicts[k+195]++; nconflict++; 
		}
/*		count++; */	/* don't count space */
	} 
	if (strcmp(mapping[127], "") != 0) {
		if (strcmp(mapping[196], "") != 0) {
				conflicts[k+196]++; nconflict++; 
		}
/*		count++; */	/* don't count delete */
	}
	if (strcmp(mapping[32], "") != 0) {
		if (strcmp(mapping[128], "") != 0) {
			conflicts[k+128]++; nconflict++; 
		}
/*		count++; */	/* don't count space */
	}
	if (count == 0 && nconflict <= 2) return -1; /* no control characters */
	else if (nconflict > 0) return nconflict;	/* some conflicts found */
	else return 0;								/* no conflicts */
}

/* non-standard in that it returns pointer to one byte past end ... */

char __far *lstrcpy (char __far *strout, char __far *strin) {	/* 1993/July/13 */
	int c;
	while ((c = *strin++) != '\0') *strout++ = (char) c;
	*strout = (char) c;			/* terminating null */
	return strout;			    /* return pointer to one past end */
}

size_t lstrlen (char __far *s) {
	size_t k=0;
	while (*s++ != '\0') k++;
	return k;
}

/* like fputs(FILE *, char *s), except for __far string */

void lputs (FILE *output, char __far *s) {
	int c;
	while ((c = *s++) != '\0') putc(c, output);
}

/* code converted to using `encoding' buffer in far space 1993/July/13 */

size_t generateencoding (void) {
/*	char *s=encoding; */
	char __far *s=encoding;							/* 1993/July/13 */
	int k, flag, nconflict = 0, count = 0;

/*	strcpy(s, "/Encoding 256 array\n"); 	s += strlen(s); */
	s = lstrcpy(s, "/Encoding 256 array\n");
/*	strcat(s, "0 1 255 {1 index exch /.notdef put} for\n"); 
	s += strlen(s); */
	s = lstrcpy(s, "0 1 255 {1 index exch /.notdef put} for\n"); 

	if (remapflag != 0) {		/* 1993/May/29 */
		flag = checkremap();
		if (flag > 0) {
			fprintf(errout, "NOTE: Found %d conflicts", flag);
			remapflag = 0;
		}
		if (flag < 0) {
			fprintf(errout, "NOTE: No control characters");
			remapflag = 0;
		}
		if (flag != 0)
			fprintf(errout, " --- control char remapping suppressed\n");
	}

	for (k = 0; k < MAXCHRS; k++) conflicts[k] = 0;

	if (remapflag != 0) {
		if (verboseflag != 0)
	printf("Adding remap (0-9 => 161-170, 10-32 => 173-195, 127 => 196)\n");

		for (k = 0; k < 10; k++) {
			if (strcmp(mapping[k], "") != 0) {
				if (strcmp(mapping[k+161], "") != 0) {
					fprintf(errout, "%s ", mapping[k+161]);
					conflicts[k+161]++;	nconflict++;
				}
/*				sprintf(s, "dup %d /%s put\n", k + 161, mapping[k]);
				s += strlen(s); */
				sprintf(buffer, "dup %d /%s put\n", k + 161, mapping[k]);
				s = lstrcpy(s, buffer);
				count++;
			}
		}
		for (k = 10; k < 32; k++) {
			if (strcmp(mapping[k], "") != 0) {
/*				sprintf(s, "dup %d /%s put\n", k + 163, mapping[k]);
				s += strlen(s); */
				sprintf(buffer, "dup %d /%s put\n", k + 163, mapping[k]);
				s = lstrcpy(s, buffer);
				if (strcmp(mapping[k+163], "") != 0) {
					fprintf(errout, "%s? ", mapping[k+163]);
					conflicts[k+163]++; nconflict++;
				}
				count++;
			}
		}
/*		treat the following special - suppress original rather than copy */
		if (strcmp(mapping[32], "") != 0 && dontmapspace == 0) {
			if (strcmp(mapping[195], "") != 0) {
/*				fprintf(errout, "%s? ", mapping[195]); */
/*				conflicts[k+195]++; nconflict++; */
			}
			else {
/*				sprintf(s, "dup %d /%s put\n", 195, mapping[32]);
				s += strlen(s); */
				sprintf(buffer, "dup %d /%s put\n", 195, mapping[32]);
				s = lstrcpy(s, buffer);
				count++;
			}
		}
		if (strcmp(mapping[127], "") != 0) {
			if (strcmp(mapping[196], "") != 0) {
/*				fprintf(errout, "%s? ", mapping[196]); */
/*				conflicts[k+196]++; nconflict++; */
			}
			else {
/*				sprintf(s, "dup %d /%s put\n", 196, mapping[127]);
				s += strlen(s); */
				sprintf(buffer, "dup %d /%s put\n", 196, mapping[127]);
				s = lstrcpy(s, buffer);
				count++;
			}
		}
/*		treat the following special - suppress original rather than copy */
		if (strcmp(mapping[32], "") != 0) {
			if (strcmp(mapping[128], "") != 0) {
/*				fprintf(errout, "%s? ", mapping[128]); */
/*				conflicts[k+128]++; nconflict++; */
			}
			else {
/*				sprintf(s, "dup %d /%s put\n", 128, mapping[32]);
				s += strlen(s); */
				sprintf(buffer, "dup %d /%s put\n", 128, mapping[32]);
				s = lstrcpy(s, buffer);
				count++;
			}
		}							/* added 1992/July/19 */
		if (nconflict > 0)
			fprintf(errout, 
	"\nWARNING: Above characters conflict with remapping - do not use `t'\n");
		if (count == 0)
			printf("NOTE: No characters remapped - no need to use `t'\n");
	}

/*	Now do the encoding itself */
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(mapping[k], "") != 0 && conflicts[k] == 0) {
/*			sprintf(s, "dup %d /%s put\n", k, mapping[k]); 
			s += strlen(s); */
			sprintf(buffer, "dup %d /%s put\n", k, mapping[k]);
			if (s - encoding + strlen(buffer) > MAXENCODING) {
				fprintf(errout, "ERROR: PS code for encoding too long\n");
				return 0;
			}	/* new sanity check 1993/July/13 */
			s = lstrcpy(s, buffer);
		}
	}
/*	strcat(s, "readonly def\n"); */
/*	s += strlen(s); */
	s = lstrcpy(s, "readonly def\n");
/*	return (int) strlen(encoding); */
	return (size_t) (s - encoding);
}

size_t copyencoding(FILE *input) { /* assume PFA file instead of VEC file */
/*	char *s = encoding; */
	char __far *s = encoding;

/*	skip up to /Encoding */
	while (fgets(buffer, BUFFERLEN, input) != NULL) {
		if (strstr (buffer, "/Encoding") != NULL) break;
	}
/*	strcpy(s, buffer); s += strlen(buffer);	 */
	s = lstrcpy(s, buffer);							/* 1993/July/13 */
	if (strstr(buffer, "StandardEncoding") != NULL)
		return (size_t) (s - encoding);				/* easy case */
	if (strstr(buffer, "ISOLatin1Encoding") != NULL)
		return (size_t) (s - encoding);				/* 98/Mar/23 ??? */
	while (fgets(buffer, BUFFERLEN, input) != NULL) {
/*		strcpy(s, buffer); s += strlen(buffer); */
		if (s - encoding + strlen(buffer) > MAXENCODING) {
			fprintf(errout, "ERROR: PS code for encoding too long\n");
			return 0;
		}	/* new sanity check 1993/July/13 */
		s = lstrcpy(s, buffer);
		if (strstr(buffer, "readonly def") != NULL) break;
				if (strstr(buffer, "def") != NULL &&
					strstr(buffer, "put") == NULL) break;
	}
/*	return; */
	return (size_t) (s - encoding);
}

/* returns -1 when can't find file, else length of encoding */

int readvectorfile(char *name) {
	FILE *input=NULL;
	char fn_in[FILENAME_MAX];
	int k, len;
	char *s, *t;

	if (strcmp(name, "standard") == 0) {
		for (k = 0; k < MAXCHRS; k++) {
			strcpy(mapping[k], standardencoding[k]);
		}
		return generateencoding();
	}

/*	First try in current directory */
	strcpy(fn_in, name);
	extension(fn_in, "vec");
	if (traceflag != 0) printf("Trying %s\n", fn_in);
	input = fopen(fn_in, "r");

/*  Next try along path specified by environmental variable VECPATH */
	if (input == NULL) {
/*		don't bother to do this if name is qualified ... */
		if (strchr(name, '\\') == NULL &&
			strchr(name, '/') == NULL &&
			strchr(name, ':') == NULL) {
			s = vecpath;
			while (*s != '\0') {
				if ((t = strchr(s, ';')) != NULL) *t = '\0';	/* flush ; */
				strcpy(fn_in, s);
				if (strcmp(fn_in, "") != 0) {
					s = fn_in + strlen(fn_in) - 1;
					if (*s != '\\' && *s != '/') strcat(fn_in, "\\");
				}
				strcat(fn_in, name);
				extension(fn_in, "vec");
				if (traceflag != 0) printf("Trying %s\n", fn_in);
				input = fopen(fn_in, "r");
				if (input != NULL) break;		/* success */
				if (t == NULL) break;			/* failed */
				else s = t+1;					/* continue after ; */
			}
		}
	}

	if (input == NULL) {	/*	then try in directory of procedure itself */
		strcpy(fn_in, programpath);
		strcat(fn_in, "\\");
		strcat(fn_in, name);
		extension(fn_in, "vec");
		if (traceflag != 0) printf("Trying %s\n", fn_in);
		input = fopen(fn_in, "r");
	}
	if (input == NULL) {	/*	OK, time to punt ! */
		fprintf(stderr, "Can't open encoding vector\n");
		perror(vectorname);
		return -1;
	}

	if (verboseflag != 0) printf("Using encoding vector file %s\n", fn_in);

/*	We DO have a vector file! --- maybe */
	fgets(buffer, BUFFERLEN, input);
	if (*buffer == '%' && *(buffer+1) == '!') { /* 1993/May/11 */
		len = copyencoding(input);
		fclose (input);
/*		return strlen(encoding); */
		return len;
	}
	rewind(input);
	readencoding(input);
	fclose (input);
	return generateencoding();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
/*	printf("Usage:\n"); */
	printf("%s [-{v}{x}{t}{j}{u}{S}] [-r=<name>] [-o=<slant>] [-c=<vector>]\n\
\t<font-file-1> <font-file-2> ...\n", s);
	if (detailflag == 0) exit(0);
	printf("\n\
\tv  verbose mode\n\
\tx  do not add `x' to output file name\n\
\tt  add remap (0-9 => 161-170, 10-32 => 173-195, 127 => 196)\n\
\tj  disallow encoding in control character range\n\
\tu  suppress UniqueID in font\n\
\tS  `standardize' - suppress ATM reencoding to Windows ANSI\n\
\tr  substitute specified FontName (PFA files only)\n\
\tf  substitute specified FamilyName (PFA files only)\n\
\tn  substitute specified FullName - use \"...\" format (PFA files only)\n\
\to  make font oblique (PFA files only)\n\
\tc  use specified encoding vector\n\
\t   StandardEncoding will be used if no encoding vector is specified.\n\
\n\
\t   Output files appear in current directory.\n\
");
	exit(0);
}

/*  \tV  substitute specified version number (PFA files onlt)\n\ */
/*  \ts  strip comments from AFM file\n\  */
/*  \td  don't remap 32 => 195\n\ */
/*	\t   Use `-c=standard' to avoid ATM reencoding to Windows ANSI.\n\ */
/*	\t   Encoding vector searched for on path specified by VECPATH\n\ */
/*	\t   ...if not found there, then in current directory\n\ */
/*	\t   ...if not found there, then in directory of this program\n\ */

int decodeflag (int c) { 			/* decode command  line flag */
	switch(c) { 
		case 'v': if(verboseflag != 0) traceflag = 1; else verboseflag = 1; return 0; 
		case 'x': suppressx = 1; return 0; 
		case 'j': suppresscontrol = 1; return 0;
		case 't': remapflag = 1; return 0; 
		case 'u': killunique = 1; return 0; 
		case 's': stripcomments = 1; return 0;
		case 'S': standardize = 1; return 0; 
		case 'd': dontmapspace = 1; return 0; 
		case 'm': stripcontrolm = 1; return 0; 
		case 'D': usetoday = 1; return 0;
		case 'F': usefiledate = 1; return 0;
		case 'R': forcestandard = 1; return 0;	/* 1996/June/20 */
		case 'X': removeX = 1; return 0;		/* 1998/Mar/22 */
		case '?': detailflag = 1; return 0; 
/* the rest take arguments */
		case 'i': uniqueflag = 1; return -1;
		case 'c': vectorflag = 1; return -1; 
		case 'r': newnameflag = 1; return -1; 
		case 'f': familynameflag = 1; return -1;
		case 'n': fullnameflag = 1; return -1;
		case 'V': versionflag = 1; return -1;
/*		case 'R': revisionflag = 1; return -1; */
		case 'a': afmfileflag = 1; return -1;
		case 'o': obliqueflag = 1; return -1;
		default: {
				fprintf(stderr, "Invalid command line flag '%c'\n", c);
				exit(13);
		}
	}
/*	return 0; */	/* ??? */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
/*	fflush(stdout); */
	(void) _getch(); 
	return hash;
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (vectorflag != 0) {
					vectorname = s;		vectorflag = 0;
				}
				if (newnameflag != 0) {
					newfontname = s;	newnameflag = 0;
				}
				else if (fullnameflag != 0) {
					newfullname = s;	fullnameflag = 0;
				}
				else if (familynameflag != 0) {
					newfamilyname = s;	familynameflag = 0;
				}
				if (versionflag) {
					newversion = s;  versionflag = 0;
				}
/*				if (revisionflag) {
					revisiondate = s;  revisionflag = 0;
				} */ /* be careful about free(revisiondate) if you do this */
				if (afmfileflag) {
					afmfilename = s;  afmfileflag = 0;
				}
				if (obliqueflag != 0) {
					if (sscanf(s, "%lg", &slant) < 1)
						fprintf(errout, "Don't understand slant %s\n", s);
					if (slant > 0.001 || slant < -0.001) 
						fprintf(errout, "WARNING: slant > 0.001\n");
					oblique = s;	obliqueflag = 0;
				}
				if (uniqueflag != 0) {
					if (sscanf(s, "%ld", &uniqueID) < 1)
						fprintf(errout, "Don't understand UniqueID %s\n", s);
					unique = s;		uniqueflag = 0;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}


/* What if font name is upper case ? */ /* 1993/July/1 */

/* copy code from AFMtoTFM and AFMtoPFM ??? */

int addanex(char *fontname) {		/* try and add an 'x' to fontname */
	char *s, *t;
	char extension[16]="";
	int n;
	int flag=0;

/*	if (traceflag != 0) printf("Adding an x to %s ", fontname); */

	if ((t = strrchr(fontname, '\\')) == NULL) {
		if ((t = strrchr(fontname, '/')) == NULL) {
			if ((t = strrchr(fontname, ':')) == NULL) {
				t = fontname;
			}
		}
	}

/*	first remove extension if any */
/*	if ((s = strrchr(fontname, '.')) != NULL) { */
	if ((s = strrchr(fontname, '.')) != NULL && s > t) {
		strncpy(extension, s, 16);		/* save `.afm' for example */
		*s = '\0';
	}
/*	if (verboseflag != 0) printf("After extension strip: %s\n", fontname);*/
	n = strlen(fontname);
/*	if (n > 8) return 0; */
	if (n <= 8) {
		s = fontname + n - 1;
		if (*s == '_') {
			while (s > fontname && *s == '_') s--;
/*			if (s - fontname < 7 && *s != 'x') *(s+1) = 'x'; */
			if (s - fontname < 7 && *s != 'x' && *s != 'X') {
				*(s+1) = 'x';
				flag++;
/*				return 1; */
			}
		}
/*		else if (s - fontname < 7 && *s != 'x') strcat(fontname, "x"); */
		else if (s - fontname < 7 && *s != 'x' && *s != 'X') {
			*(s+1) = 'x';	/* strcat(fontname, "x"); */
			*(s+2) = '\0';
			flag++;
/*			return 1; */
		}
/*		else return 0; */
	}
/*	put extension back if any */
	if (strcmp(extension, "") != 0) strcat(fontname, extension);
/*		*(s+1) = 'x';	*(s+2) = '\0'; */
/*	if (traceflag != 0) printf("New fontname: %s\n", fontname); */
/*	if (verboseflag != 0) printf("New fontname: %s\n", fontname); */
	return flag;
}

int stripanx(char *fontname) { /* try and remove an 'x' on fontname */
	char *s;

	if ((s = strrchr(fontname, '.')) != NULL) s--;
	else s = fontname + strlen(fontname) - 1; 
	if (*s == '_') {
		while (s > fontname && *s == '_') s--;
/*		if (*s == 'x') *s = '_'; */
		if (*s == 'x' || *s == 'X') {	/* 1993/July/1 */
			if (traceflag != 0) printf("%s\n", fontname); 
			*s = '_';	
			return 1;
		}
	}
/*	else if (*s == 'x') *s = '\0'; */
	else if (*s == 'x' || *s == 'X') { 	/* 1993/July/1 */
		if (traceflag != 0) printf("%s\n", fontname);
/*		*s = '\0'; */
		strcpy(s, s+1);				/* 1993/Aug/10 */
		return 1;
	}
	return 0;
/*	if (traceflag != 0) printf("New fontname: %s\n", fontname); */
}

/* remove file name - keep only path - inserts '\0' to terminate */
void removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}

void uppercase(char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

void flushexten(char *name) {	/* remove extension - if any - not used */
	char *s;
	if ((s = strchr(name, '.')) != NULL) *s = '\0';
}

void forceunder(char *name) {	/* extend name with underlines - not used */
	int i, n;
	char *s;

	if ((s = strrchr(name, '\\')) != NULL) s++;
	else if ((s = strrchr(name, ':')) != NULL) s++;
	else s = name;
	n = (int) strlen(s);
	s = s + n;
/*	printf("%s %d ", s, n); */

	for (i=n; i < 8; i++) {
		*s++ = '_';
/*		printf("%s ", name); */
	}
	*s = '\0';
}

void copyextension(char *ext, char *name) {
	char *s, *t;
	if ((t = strrchr(name, '\\')) == NULL) {
		if ((t = strrchr(name, '/')) == NULL) {
			if ((t = strrchr(name, ':')) == NULL) {
				t = name;
		}
		}
	}

/*	if ((s = strchr(name, '.')) != NULL) { */
	if ((s = strrchr(name, '.')) != NULL) {
		strcpy(ext, s+1);
	}
	else strcpy(ext, "");
}

/* format %!PS-AdobeFont-1.0: Clarendon 001.000 */

void replacename(char *buff, char *newname, char *newversion) {
	unsigned int newlen, endlen, oldlen;
	char *s, *t;

	newlen = strlen(newname);
	s = strchr(buff, ' ');		/* find space after : */
	if (s == NULL) return;
	t = strchr(s + 1, ' ');		/* find space after FontName, before version */
	if (t == NULL) return;

	if (strcmp(newname, "") != 0) {
		oldlen = t - (s + 1);
		strncpy (oldfontname, s+1, oldlen);	/* save for checking */
		oldfontname[oldlen] = '\0';
/*		printf("OLDFONTNAME: %s\n", oldfontname); */	/* debugging */
		endlen = strlen(t);
/*		memmove(t + newlen - oldlen, t, endlen); */
		memmove(t + newlen - oldlen, t, endlen+1);
		memcpy(s + 1, newname, newlen);
/*		printf("LINE %sLINE\n", buff); */		/* debugging */
		t = strchr(s + 1, ' ');	/* find space after FontName, before version */
		if (t == NULL) return;
	}
	if (strcmp(newversion, "") != 0) {
		sprintf(t + 1, "%s\n", newversion);
	}
}

/* format: /FontMatrix [0.001 0 0 0.001 0 0] readonly def */
/* format: /FontMatrix{0.001 0 0 0.001 0 0} def */

void changeoblique(char *buff, char *oblique) {
	unsigned int newlen, endlen, oldlen;
	char *s, *t;

	newlen = strlen(oblique);
	s = strstr(buff, "/FontMatrix");
	if ((s = strchr(s+10, '[')) != NULL || (s = strchr(s+10, '{')) != NULL) {
		if ((s = strchr(s + 1, ' ')) != NULL) {			/* first space */
			if ((s = strchr(s + 1, ' ')) != NULL) {		/* second space */
				if ((t = strchr(s + 1, ' ')) != NULL) {	/* third space */
					oldlen = t - (s + 1);
					endlen = strlen(t);
					memmove(t + newlen - oldlen, t, endlen+1);
					memcpy(s + 1, oblique, newlen);
				}
			}
		}
	}
	if (traceflag != 0) fputs(buff, stdout);
}

/* remember that this only changes the outer unencrypted UniqueID */

void changeunique(char *buff, char *unique) {
	unsigned int newlen, endlen, oldlen;
	char *s, *t;
	
	newlen = strlen(unique);
	s = strstr(buff, "/UniqueID");
	if ((s = strchr(s + 9, ' ')) != NULL) {			/* first space */
		if ((t = strchr(s + 1, ' ')) != NULL) {		/* second space */
			oldlen = t - (s + 1);
			endlen = strlen(t);
			memmove(t + newlen - oldlen, t, endlen+1);
			memcpy(s + 1, unique, newlen);
		}
	}
	if (traceflag != 0) fputs(buff, stdout);
}

double pi = 3.141592653; 

void changeitalic(char *buff, double slant) {
	double italicangle;
	unsigned int newlen, endlen, oldlen;
	char *s, *t;
	char italic[NUMBERSTR_MAX];
	
	italicangle = - atan(slant * 1000.0) * 180.0 / pi;
	if (italicangle > 0)
		italicangle = ((double) ((int) (italicangle * 1000.0 + 499.0))) / 1000.0;
	else italicangle = ((double) ((int) (italicangle * 1000.0 - 499.0))) / 1000.0;
	sprintf(italic, "%lg", italicangle);
	newlen = strlen(italic);
	s = strstr(buff, "/ItalicAngle");
	if ((s = strchr(s + 12, ' ')) != NULL) {		/* first space */
		if ((t = strchr(s + 1, ' ')) != NULL) {		/* second space */
			oldlen = t - (s + 1);
			endlen = strlen(t);
			memmove(t + newlen - oldlen, t, endlen+1);
			memcpy(s + 1, italic, newlen);
		}
	}
	if (traceflag != 0) fputs(buff, stdout);
}

char *fgetsmod (char *s, int n, FILE *iop) {
/*	register int c = 0; */
	int c = 0;
/*	register char *cs; */
	char *cs;
	
	cs = s;
	while (--n > 0 && (c = getc(iop)) != EOF) {
/*		if((*cs++ = (char) c) == '\n' || c == '\r')	break; */
		*cs++ = (char) c;
		if (c == '\n' || c == '\r')	break; 
	}
	if (n <= 0) fprintf(stderr, "Buffer overflow\n");
	if (c == '\r') {
		c = getc(iop);
		if (c == '\n') {
			*cs++ = (char) c;
			usingboth = 1;
		}
		else {
			(void) ungetc(c, iop);
			usingreturn = 1;
		}
	}
	*cs = '\0';
/*	return (c == EOF && cs == s) ? NULL : s; */
/*	if (traceflag) fputs(s, stdout); */
	if (c == EOF && cs == s) return NULL;
	else return s;
			
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Share long error message in two places to save memory ... */

char *cannerr=
"\
WARNING: Cannot standardize %s file that does not use StandardEncoding\n\
NOTE:    Font file may already have been standardized\n\
";

char infilename[FILENAME_MAX], outfilename[FILENAME_MAX], oldfilename[FILENAME_MAX];
FILE *input, *output;
int renameflag, afmfileflag, pfafileflag, outfileflag, oldencodinglength;

int processpfafile(FILE *input) {
	int n, c;
/* 	int c = '%', d = '!'; */
/*	char *v; */
	char *s, *t, *u;
	char __far *v=encoding;
 	time_t ltime;					/* for time and date */
	char fullname[FONTNAME_MAX];
	
/*	process what appears to be a PFA file ... */
/*	should we try and open with "w" instead ? */
	if ((output = fopen(outfilename, "wb")) == NULL) { 
		perror(outfilename);
		fclose(input);
		if (renameflag != 0) (void) rename(infilename, oldfilename);
		return -1;
/*			continue;	*//* return (1); */
	}
	if (verboseflag != 0) printf(" => %s ", outfilename);

/*	fputc(c, output); fputc(d, output);	*/
	fputc('%', output);
	fputc('!', output);	
	(void) fgetsmod(buffer, BUFFERLEN, input);
/*	replace name in %! line if asked */
	if (strcmp(newfontname, "") != 0 || strcmp(newversion, "") != 0) {
		replacename(buffer, newfontname, newversion);
	}
	fputs(buffer, output);

	c = getc(input);		/* peek ahead 96/Apr/18 */
	ungetc (c, input);
	if (c == '%') {			/* replace %%CreationDate 96/Apr/18 */
		(void) fgetsmod(buffer, BUFFERLEN, input);
		if (strncmp(buffer, "%%CreationDate:", 15) == 0) {
/*			if (verboseflag) fputs(buffer, stdout); */
			if (usetoday || usefiledate) {
				if (usefiledate) {
					if (getinfo(infilename, traceflag) < 0)
						exit(1);
					s = ctime(&statbuf.st_atime);
				}
				else {
					(void) time(&ltime);
					s = ctime(&ltime);		/* i.e. usetoday */
				}
				if (s == NULL) {
					fprintf(errout, "Can't get time!");	/* 96/Jan/4 */
/*					exit(1); */
				}
				else {
					lcivilize(s);
					sprintf(buffer + 15, " %s", s);	/* already has \n */
				}
/*				if (verboseflag) fputs(buffer, stdout); */
			}
			else if (strcmp(revisiondate, "") != 0) {
				sprintf(buffer + 15, " %s\n", revisiondate);
			}
			if (traceflag) fputs(buffer, stdout);
		}
		fputs(buffer, output);
	}
	
/*	currentdict end */ /* currentfile eexec */

	for(;;) {
		if (fgetsmod(buffer, BUFFERLEN, input) == NULL) {
			fprintf(errout, "\nPremature EOF looking for %s\n",
				   "encrypted part");
/*			fclose(input); fclose(ouput); continue */
			break;
		}
/*		if (traceflag != 0) fputs(buffer, stdout); */

		if (strstr(buffer, "eexec") != NULL) break; 
		if (strstr(buffer, "currentfile") != NULL) break; 
		if (strstr(buffer, "currentdict") != NULL) break;

		if ((s = strstr(buffer, "/FullName")) != NULL &&
/*			see whether FullName equals FontName, if so, change it also */
			strcmp(oldfontname, "") != 0) {
/*			sscanf (s, "/FullName (%s)", fullname); */
			t = strchr(s, '(');
			u = strchr(t, ')');	
			if (t == NULL || u == NULL)
				fprintf(errout, "Don't understand %s", buffer);
			else {
				n = u - (t+1);
				strncpy(fullname, t+1, n);
				fullname[n] = '\0';
/*				printf("FULLNAME: %s\n", fullname);*/	/* debugging */
				if (strcmp(fullname, oldfontname) == 0) {
					sprintf(s, "/FullName (%s) def\n", newfontname);
				}
			}
		}
		if ((s = strstr(buffer, "/FontName")) != NULL) {
			if (strcmp(newfontname, "") != 0) 
				sprintf(s, "/FontName /%s def\n", newfontname);
		}
		else if ((s = strstr(buffer, "/FullName")) != NULL) {
			if (strcmp(newfullname, "") != 0) 
				sprintf(s, "/FullName (%s) def\n", newfullname);
		}
		else if ((s = strstr(buffer, "/FamilyName")) != NULL) {
			if (strcmp(newfamilyname, "") != 0) 
				sprintf(s, "/FamilyName (%s) def\n", newfamilyname);
		}
		else if ((s = strstr(buffer, "/version")) != NULL) {
			if (strcmp(newversion, "") != 0)
				sprintf(s, "/version (%s) def\n", newversion);
		}
		else if ((s = strstr(buffer, "/FontBBox")) != NULL) {
			if (strcmp(fontbbox, "") != 0)
				sprintf(s, "/FontBBox{%s}readonly def\n", fontbbox);
		}
		else if ((s = strstr(buffer, "/Notice")) != NULL) {
			if (strcmp(notice1, "") == 0 && strcmp(notice2, "") == 0) {
			}
			else if (noticedone) {
				*buffer = '\0';				/* No more /Notice! */
			}
			else {
				if (strcmp(notice1, "") != 0) 
					sprintf(s, "/Notice (%s) readonly def\n", notice1);
/*					free(notice1); notice1 = ""; */
				if (strcmp(notice2, "") != 0) 
					sprintf(s, "/Notice (%s) readonly def\n", notice2);
/*					free(notice2); notice2 = ""; */
				noticedone = 1;
			}
		}

		if ((s = strstr(buffer, "/UniqueID")) != NULL) {
			if (strcmp(unique, "") != 0) changeunique(buffer, unique);
			else if (killunique != 0) {
				n = strlen(buffer);
/*				memmove(buffer+2, buffer, n); */
				memmove(buffer+2, buffer, n+1);
				*buffer = '%'; *(buffer+1) = ' ';
			}
		}		
		if (strstr(buffer, "/FontMatrix") != 0) {
			if (strcmp(oblique, "") != 0) changeoblique(buffer, oblique);
		}
		if (strstr(buffer, "/ItalicAngle") != 0) {
			if (strcmp(oblique, "") != 0) changeitalic(buffer, slant);
		}
		if (strstr(buffer, "/Encoding") != NULL) {
			if (renameonly != 0) {		/* 1993/May/11 */
				v = encoding;
/*				strcpy (v, buffer); v += strlen(buffer); */
				v = lstrcpy(v, buffer);
			}
/*			strip out encoding */
			for (;;) {
/*	new 1992/March/3 - deal with StandardEncoding in PFA files */
				if (strstr(buffer, "StandardEncoding") != 0) break;
/*  new 1998/March/23 - deal with ISOLatin1Encoding in PFA files */
				else if (strstr(buffer, "ISOLatin1Encoding") != 0) break; /* ?*/
				else if (standardize != 0) { /* not StandardEncoding */
					fclose(input);
					if (renameflag != 0) (void) rename(infilename, oldfilename);
					if (verboseflag != 0) putc('\n', stdout);
					fprintf(errout, cannerr, "PFA");
					return -1;
				}
				if (fgetsmod(buffer, BUFFERLEN, input) == NULL) {
					fprintf(errout,
							"\nPremature EOF looking for %s\n",
							"end of encoding");
/*					fclose(input); fclose(ouput); continue; */
					break; 
				}
				if (renameonly != 0) {
/*					strcpy (v, buffer); v += strlen(buffer); */
					v = lstrcpy (v, buffer);
				}
				if (strstr(buffer, "readonly def") != NULL) break;
				if (strstr(buffer, "def") != NULL &&
					strstr(buffer, "put") == NULL) break;
			}
/*			fputs(buffer, output);	 */ /* 1993/May/19 */
/*			break;  */	/* take out 1993/May/19 */
/*		new inserted here 1993/May/19 */
			if (reinstateflag != 0) {
				if (usingreturn != 0) changetoreturn(standard);
				else changetolinefeed(standard);
				fprintf(output, "%s", standard);
			}
			else {
				if (usingreturn != 0) changetoreturn(encoding);
				else changetolinefeed(encoding);
/*				fprintf(output, "%s", encoding); */
				lputs(output, encoding);
			}	
		}
		else fputs(buffer, output);
		if (traceflag != 0) fputs(buffer, stdout);
	}
	fputs(buffer, output);	/* 1993/May/19 */
/*	copied up to encrypted part now */

	if (verboseflag != 0) {
		if (usingboth != 0) putc('B', stdout);
		else if (usingreturn != 0) putc('R', stdout);
		else putc('N', stdout);
/*		printf(" (ASCII %ld => %ld)\n", asciilength, newlength); */
		putc('\n', stdout);
	}
	return 0;
}

int processpfbfile(FILE *input) {
	int c, d, n;
/*	int chr; */
/*	char *v; */
	char __far *v=encoding;
/*	char charname[MAXCHARNAME]; */

/*			process what appears to be a PFB file ... */			
			oldencodinglength = -1;
			asciilength = readfour(input);		/* read over length code */
/*			if (verboseflag != 0) 
				printf("ASCII section %ld bytes\n", asciilength); */
			pass = 0;
			for(;;) {
				linestart = ftell(input);
				if(readline(buffer, BUFFERLEN, input) == 0) {
					linestart = -1;	
					fprintf(errout, "Unexpected EOF\n");
					break;
				}
				if (traceflag != 0) {
					if (usingreturn != 0) 
						changetolinefeed(buffer); /* side effect */
					printf("%s", buffer);
				}
				if (strstr(buffer, "/UniqueID") != NULL &&
					killunique != 0) {
					n = strlen(buffer);
/*					memmove(buffer+2, buffer, n); */
					memmove(buffer+2, buffer, n+1);
					*buffer = '%'; *(buffer+1) = ' ';
				}				
				if (strstr(buffer, "/Encoding") != NULL) {
					if (renameonly != 0) {		/* 1993/May/11 */
						v = encoding;
/*						strcpy (v, buffer); v += strlen(buffer); */
						v = lstrcpy(v, buffer);
					}
					if (strstr(buffer, "StandardEncoding") != NULL) {
						oldencodinglength = (int) strlen(buffer);
						if (traceflag != 0) 
							printf("start %ld end %ld\n", 
								linestart, ftell(input));
						break;							/* line length */
					}
					else if (standardize != 0) {	/* not StandardEncoding */
						fclose(input);
						if (renameflag != 0)
							(void) rename(infilename, oldfilename);
						if (verboseflag != 0) putc('\n', stdout);
						fprintf(errout, cannerr, "PFB");
						return -1;
					}
					(void) readline(buffer, BUFFERLEN, input);
					while (strstr(buffer, "readonly def") == NULL) {
						if (renameonly != 0) {
/*							strcpy (v, buffer); v += strlen(buffer); */
							v = lstrcpy(v, buffer);
						}
						if (readline(buffer, BUFFERLEN, input) == 0) {
							fprintf(errout, "Unexpected EOF\n");
							break;
						}
					}
					if (strstr(buffer, "readonly def") == NULL) break;
					oldencodinglength = (int) (ftell(input) - linestart);
					if (traceflag != 0) 
						printf("start %ld end %ld\n", 
							linestart, ftell(input));
					break;
				}
			}

/* did not find encoding ? */
			if(linestart < 0 || oldencodinglength < 0) { 
				fclose(input);
				if (renameflag != 0) (void) rename(infilename, oldfilename);
				fprintf(errout, "ERROR: Did not find Encoding vector - ");
				fprintf(errout, "file not processed\n");
				return -1;
/*				continue; */
			}
			
			if (traceflag != 0) printf("start %ld end %ld\n",
				linestart, linestart + oldencodinglength);

			encodingline = linestart;
			rewind(input);
			pass = 1;
			c = fgetc(input); d = fgetc(input);

			if ((output = fopen(outfilename, "wb")) == NULL) {
				perror(outfilename);
				fclose(input);
				if (renameflag != 0) (void) rename(infilename, oldfilename);
/*				continue;	*/ /* return (1); */
			}
			
			if (verboseflag != 0) printf(" => %s ", outfilename);
			
			fputc(c, output); fputc(d, output);
			asciilength = readfour(input);
			if (reinstateflag != 0)
/*				newlength = asciilength + strlen(standard) - oldencodinglength;	 */
				newlength = asciilength + lstrlen(standard) - oldencodinglength;
			else 
/*				newlength = asciilength + strlen(encoding) - oldencodinglength; */
				newlength = asciilength + lstrlen(encoding) - oldencodinglength;
			
			if (renameonly != 0) newlength = asciilength;	/* 1993/May/11 */

			if (verboseflag != 0) {
				if (usingboth != 0) putc('B', stdout);
				else if (usingreturn != 0) putc('R', stdout);
				else putc('N', stdout);
				printf(" (ASCII %ld => %ld)\n", asciilength, newlength);
			}
			
			writefour(newlength, output);
			for(;;) {
				linestart = ftell(input);
				if(readline(buffer, BUFFERLEN, input) == 0) break;	 /*	EOF */
				if (traceflag != 0) {
					if (usingreturn != 0) 
						changetolinefeed(buffer); /* side effect */
					printf("%s", buffer);
				}
				if (strstr(buffer, "/Encoding") != NULL) {
					if (strstr(buffer, "StandardEncoding") != NULL) {
/*						just replace this one line ! */
						if (reinstateflag != 0) {
							fprintf(errout, "Already StandardEncoding!\n");
/*						break; */
						}
					}
					else {
/* need to strip out old encoding vector */
						(void) readline(buffer, BUFFERLEN, input);
						while (strstr(buffer, "readonly def") == NULL) {
							if(readline(buffer, BUFFERLEN, input) == 0) break;
						}
					}
/*				if (reinstateflag != 0) {
					if (usingreturn != 0) changetoreturn(standard);
					else changetolinefeed(standard);
*					fprintf(output, "%s", standard);  *
					lputs(output, standard);
				}
				else {
					if (usingreturn != 0) changetoreturn(encoding);
					else changetolinefeed(encoding);
*					fprintf(output, "%s", encoding); *
					lputs(output, encoding);
				}	*/
					break;
				} 
				else fputs(buffer, output);
			}
		return 0;
}

void stripsemicolon (char *name) {
	int n, c;
	char *s;
	n = strlen(name);
	s = name + n - 1;
	while ((c = *s) == ';' || c == ' ') *s-- = '\0';
}

int processafmfile(FILE *input) {
	int n, m, chrs;
	double fwidth;
	char charname[CHARNAME_MAX];
	int c, cold;
/*	char *s; */
	
/*	process what appears to be a AFM file ...  */
	if (stripcontrolm != 0) {					/* 1992/Oct/10 */
		if ((output = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename);
			fclose(input);
			if (renameflag != 0) (void) rename(infilename, oldfilename);
			return -1;
		}
	}
	else {
		if ((output = fopen(outfilename, "w")) == NULL) {
			perror(outfilename);
			fclose(input);
			if (renameflag != 0) (void) rename(infilename, oldfilename);
			return -1;
		}
	}

	if (verboseflag != 0) printf(" => %s\n", outfilename);
/*	fprintf(output, "St"); */				/* 1992/Sep/25 */

	while(fgets(buffer, BUFFERLEN, input) != NULL) {
/*	after StartCharMetrics and before EndCharMetrics */
		if (stripcomments != 0 && *buffer == '%') continue; /* 92/Sep/25 */
		if (*buffer == '%' || *buffer == ';' || *buffer == '\n') {
			fputs(buffer, output);	continue;
		}
		if (sscanf(buffer, "C %d ; WX %lg ; N %s%n",
			&chrs, &fwidth, charname, &n) == 3) {
/*			s = buffer + n; */
			stripsemicolon(charname);
			m = lookup(charname);
			fprintf(output, "C %d ; WX %lg ; N %s", m, fwidth, charname);
/*			fprintf(output, " N=%d ", n); */
			cold = 0;
			while ((c = buffer[n++]) >= ' ') {
/* strip comments off character metrics lines */
				if (stripcomments != 0 && c == '%') break; /* 92/Sep/25 */
/* fix up bad files with no space before semicolon */
				if (c == ';' && cold != ' ') putc(' ', output);
				putc(c, output);
				cold = c;
			}
			putc('\n', output);
		}
		else {
/*			if (stripcomments != 0 && (s = strchr(buffer, '%')) != NULL) 
				*s = '\n';	*/
			fputs(buffer, output);
		}
	}
	return 0;
}

int processoutfile(FILE *input) {		/* 1994/May/28 */
/*	int n, m; */
	int k, chrs;
	int width;
	char charname[CHARNAME_MAX];
/*	int c, cold; */
	int newcharflag = 0;
/*	char *s; */
	
/*	process what appears to be an OUT file ...  */
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);
		fclose(input);
		if (renameflag != 0) (void) rename(infilename, oldfilename);
		return -1;
	}

	if (verboseflag != 0) printf(" => %s\n", outfilename);

	while(fgets(buffer, BUFFERLEN, input) != NULL) {
		if (stripcomments != 0 && *buffer == '%') continue; 
		if (*buffer == '%' || *buffer == ';' || *buffer == '\n') {
			fputs(buffer, output);	continue;
		}		/* ignore comments */
		if (newcharflag) {
			if (sscanf(buffer, "%d %d %% %s", &chrs, &width, &charname) == 3) {
				k = lookup(charname);
				if (k >= 0) 
					sprintf(buffer, "%d %d %% %s\n", k, width, charname);
				else fprintf(errout, "ERROR: Not in encoding: %s", buffer);
			}
			else fprintf(errout, "ERROR: Don't understand: %s", buffer);
			newcharflag = 0;
		}
		else if (*buffer == ']') newcharflag++;
		fputs(buffer, output);
	}
	return 0;
}

/* return pointer to file name - minus path - returns pointer to filename */
char *findpath(char *pathname) {
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

	s = findpath(filename);
/*	if ((s = strrchr(filename, '\\')) != NULL) s++;
	else if ((s = strrchr(filename, '/')) != NULL) s++;
	else if ((s = strrchr(filename, ':')) != NULL) s++;
	else s = filename; */
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
/*	if (traceflag != 0) printf("n = %d m = %d ", n, m); */
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
/*	if (traceflag != 0) printf("Now trying file name: %s\n", s); */
/*	return 0; */
}

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

/* set up full file name for ini file and check for specified section */
/* e.g. dviwindo = setupinifile ("dviwindo.ini", "[Environment]") */

#define MAXLINE 256

char *setupinifile (char *ininame, char *section) {	
	char fullfilename[FILENAME_MAX];
	FILE *input;
	char *windir;
	char line[MAXLINE];
	int m;

/*	Easy to find Windows directory if Windows runs */
/*	Or if user kindly set WINDIR environment variable */
/*	Or if running in Windows NT */	
	if ((windir = getenv("windir")) != NULL ||	/* 1994/Jan/22 */
		(windir = getenv("WINDIR")) != NULL ||
		(windir = getenv("winbootdir")) != NULL ||
		(windir = getenv("SystemRoot")) != NULL ||
 		(windir = getenv("SYSTEMROOT")) != NULL) { /* 1995/Jun/23 */
		strcpy(fullfilename, windir);
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);
	}
	else _searchenv (ininame, "PATH", fullfilename);

	if (strcmp(fullfilename, "") == 0) {		/* ugh, try standard place */
		strcpy(fullfilename, "c:\\windows");
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);		
	}

/*	if (*fullfilename != '\0') { */
/*	check whether ini file actually has required section */
		if ((input = fopen(fullfilename, "r")) != NULL) {
			m = strlen(section);
			while (fgets (line, sizeof(line), input) != NULL) {
				if (*line == ';') continue;
/*				if (strncmp(line, section, m) == 0) { */
				if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
					fclose(input);
					return _strdup(fullfilename);
				}
			}					
			fclose(input);
		}
/*	} */
	return "";							/* failed, for one reason or another */
}

int setupdviwindo (void) {
	if (usedviwindo == 0) return 0;		/* already tried and failed */
	if (*dviwindo != '\0') return 1;	/* already tried and succeeded */
/*	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");  */
	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");
	if (*dviwindo == '\0') usedviwindo = 0; /* failed, don't try this again */
	return (*dviwindo != '\0');
}

char *grabenvvar (char *varname, char *inifile, char *section, int useini) {
	FILE *input;
	char line[MAXLINE];
	char *s;
	int m, n;

	if (useini == 0 || *inifile == '\0')
		return getenv(varname);	/* get from environment */
	if ((input = fopen(inifile, "r")) != NULL) {
		m = strlen(section);
/* search for [...] section */
		while (fgets (line, sizeof(line), input) != NULL) {
			if (*line == ';') continue;
/*			if (strncmp(line, section, m) == 0) { */
			if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
/* search for varname=... */
				n = strlen(varname);
				while (fgets (line, sizeof(line), input) != NULL) {
					if (*line == ';') continue;
					if (*line == '[') break;
/*					if (*line == '\n') break; */	/* ??? */
					if (*line <= ' ') continue;		/* 95/June/23 */
/*					if (strncmp(line, varname, n) == 0 && */
					if (_strnicmp(line, varname, n) == 0 &&
						*(line+n) == '=') {	/* found it ? */
							fclose (input);
							/* flush trailing white space */
							s = line + strlen(line) - 1;
							while (*s <= ' ' && s > line) *s-- = '\0';
							if (traceflag)  /* DEBUGGING ONLY */
								printf("%s=%s\n", varname, line+n+1);
							return _strdup(line+n+1);
					}							
				}	/* end of while fgets */
			}	/* end of search for [Environment] section */
		}	/* end of while fgets */
		fclose (input);
	}	/* end of if fopen */
/*	useini = 0; */				/* so won't try this again ! need & then */
	return getenv(varname);	/* failed, so try and get from environment */
}							/* this will return NULL if not found anywhere */

char *grabenv (char *varname) {	/* get from [Environment] in dviwindo.ini */
	return grabenvvar (varname, dviwindo, "[Environment]", usedviwindo);
}

#ifdef NEEDATMINI
/* grab setting from `atm.ini' 94/June/15 */

/*	if (useatmini)  setupatmini(); 	*/ /* need to do this before use */

int useatmini = 1;			/* use [Setup] section in `atm.ini' */
							/* reset if setup of atm.ini file fails */

char *atmininame = "atm.ini";		/* name of ini file we are looking for */

char *atmsection = "[Setup]";		/* ATM.INI section */

char *atmini = "";				/* full file name for atm.ini with path */

int setupatmini (void) {
	if (useatmini == 0) return 0;		/* already tried and failed */
	if (*atmini != '\0') return 1;		/* already tried and succeeded */
/*	atmini = setupinifile ("atm.ini", "[Setup]");  */
	atmini = setupinifile (atmininame, atmsection);
	if (*atmini == '\0') useatmini = 0;	/* failed, don't try this again */
	return (*atmini != '\0');
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Read AFM file to extract FontName, FullName, FamilyName, version, Notice */

void readafmfile (char *filename) {				/* 96/Apr/14 */
	FILE *input;
	char line[MAXLINE];
	char *s;

	strcpy(infilename, filename);
	extension(infilename, "afm");
	if (traceflag) printf("Trying to open %s\n", infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);
		exit(1);
	}
	if (verboseflag) printf("Reading font info from %s\n", infilename);
/*	This may override info from other command line flags */
	while (fgets (line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
		s = line + strlen(line) - 1;
		while (*s <= ' ') *s-- = '\0';		/* remove white space at end */
		if (strncmp(line, "FontName", 8) == 0)
			newfontname = _strdup(line + 9);
		else if (strncmp(line, "FullName", 8) == 0)
			newfullname = _strdup(line + 9);
		else if (strncmp(line, "FamilyName", 10) == 0)
			newfamilyname = _strdup(line + 11);
		else if (strncmp(line, "FontBBox", 8) == 0)
			fontbbox = _strdup(line + 9);
		else if (strncmp(line, "version", 7) == 0)
			newversion = _strdup(line + 8);
		else if (strncmp(line, "Version", 7) == 0)
			newversion = _strdup(line + 8);
		else if (strncmp(line, "Comment", 7) == 0) {
			if (strncmp(line + 8, "UniqueID", 8) == 0) {
				if (sscanf(line + 17, "%ld", &uniqueID) < 1)
					fprintf(errout, "Don't understand UniqueID %s\n", line + 17);
				unique = _strdup (line+17);
			}
/* Note: Adobe uses instead Comment Creation Date: */
			else if (strncmp(line + 8, "CreationDate", 12) == 0 ||
					 strncmp(line + 8, "RevisionDate", 12) == 0) {
				s = line + 20;
				while (*s == ':' || *s == ' ') s++;
				if (strcmp(revisiondate, "") != 0) {
					free(revisiondate);
					revisiondate = "";
				}  /* not if could have been string from command line */
/*				don't bother if we'll use today's date or file date */
				if (usetoday == 0 && usefiledate == 0)	
					revisiondate = _strdup(s);
			}	/* keep only the latest */
		}
		else if (strncmp(line, "Notice", 6) == 0) {
			if (strcmp(notice1, "") == 0)
				notice1 = _strdup(line + 7);
			else if (strcmp(notice2, "") == 0)
				notice2 = _strdup(line + 7);
			noticedone = 0;					/* note that Notices exist */
		}
	}
	fclose (input);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* need direct.h for this */

int samefile (char *infilename, char *outfilename) {	/* 95/Mar/1 */
	char cwd[NUMBERSTR_MAX];
	char *s;
	if (traceflag)
		printf("Comparing in %s and out %s\n", infilename, outfilename);
	if (strcmp(infilename, outfilename) == 0) return 1;
	_getcwd(cwd, sizeof(cwd));
	s = cwd + strlen(cwd) - 1;
/*	strcat(cwd, "\\"); */
	if (*s != '\\') strcat(cwd, "\\"); 
	strcat(cwd, outfilename);
	uppercase(cwd, cwd);
	if (traceflag)
		printf("Comparing in %s and out %s\n", infilename, cwd);
	if (strcmp(infilename, cwd) == 0) return 1;
	return 0;
}

int main (int argc, char *argv[]) {
	char savedextension[32];
	char *s;
	int firstarg = 1, filecount = 0;
/*	unsigned int n; */
	int m, c, d;
	int flag;						/* 1993/July/9 */
	
	if (argc < 2) showusage(argv[0]);
	
	strncpy(programpath, argv[0], sizeof(programpath));
	removepath(programpath);

	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("VECPATH")) != NULL) vecpath = s;
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("VECPATH")) != NULL) vecpath = s;
	}

	firstarg = commandline(argc, argv, 1);
	
	if (firstarg > argc - 1) showusage(argv[0]);

/*	printf("Program for reencoding PFB, PFA and AFM files. %s\n", version); */
/*	moved down a bit and made conditional 1993/July/11 */

	if (checkcopyright(copyright) != 0) exit(1); /* check for tampering */

	if (strcmp(afmfilename, "") != 0) readafmfile(afmfilename); /* 96/Apr/14 */

	if (standardize != 0) vectorname = "standard"; /* 1993/July/9 */

	if (firstarg == 1) reinstateflag = 1;	
	else reinstateflag = 0;		/* a guess: no arguments, just file names */

	if (verboseflag != 0 ||
		(standardize == 0 && reinstateflag == 0))	/* 1993/July/11 */
/*	printf("Program for reencoding PFB, PFA and AFM files. %s\n", version); */
#ifdef _WIN32
	printf("REENCODE (32) for PFB, PFA and AFM files. %s\n", version); 
#else
	printf("REENCODE for PFB, PFA and AFM files. %s\n", version); 	
#endif
	reinstateflag = 0;

	if (strcmp(vectorname, "none") == 0) {			/* 1993/May/29 */
		printf("Nothing to do: `none' specified for encoding\n");
		return 0;
	}

	encoding = (char __far *) _fmalloc(MAXENCODING);
	if (encoding == NULL) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(3);
	}												/* NEW 1993/July/13 */

	if (strcmp(vectorname, "") == 0) {
		if (strcmp(newfontname, "") == 0) reinstateflag = 1;
		else if (forcestandard) reinstateflag = 1;	/* 1996/June/20 */
		else renameonly = 1;						/* 1993/May/11 */
	}
	else {
		if (readvectorfile(vectorname) < 0)
			exit(1);	/* return -1; */	/* ??? */
		(void) checkencoding ();					/* 1993/July/1 */
	}

	if (reinstateflag != 0 || strcmp(vectorname, "standard") == 0)
		nonstandard = 0;
	else nonstandard = 1;	/* not StandardEncoding */

/*	if (reinstateflag != 0) */
	if (reinstateflag != 0 && verboseflag != 0)
		printf("Using StandardEncoding\n");

	for (m = firstarg; m < argc; m++) {

/*		offset=0; */
		if (strcmp(notice1, "") == 0 && strcmp(notice2, "") == 0)
			noticedone = 1;				/* 96/Apr/14 */
		else noticedone = 0;			/* 96/Apr/14 */

/* tries file name with `pfb' extension and with `pfa' extension */
/* tries file name as is or extended with underscores */

		strncpy(infilename, argv[m], sizeof(infilename));
		extension(infilename, "pfb");
		copyextension(savedextension, infilename);

/*		if ((input = fopen(infilename, "rb")) == NULL) {
			strncpy(infilename, argv[m], sizeof(infilename));
			extension(infilename, "pfa");
			copyextension(savedextension, infilename);
			if ((input = fopen(infilename, "rb")) == NULL) {
				strncpy(infilename, argv[m], sizeof(infilename));
				extension(infilename, "afm");
				copyextension(savedextension, infilename);
				if ((input = fopen(infilename, "rb")) == NULL) {
					perror(argv[m]); 
					continue;
				}
			}
		}
		else fclose(input);		*/

/* first try with PFB extension */
		input = fopen(infilename, "rb");
		if (input == NULL) {
			underscore(infilename);
			input = fopen(infilename, "rb");
		}
		if (input == NULL) {
			strncpy(infilename, argv[m], sizeof(infilename));
			extension(infilename, "pfa");
			copyextension(savedextension, infilename);
/* next try with PFA extension */
			input = fopen(infilename, "rb");
			if (input == NULL) {
				underscore(infilename);
				input = fopen(infilename, "rb");
			}
		}
		if (input == NULL) {
			strncpy(infilename, argv[m], sizeof(infilename));
			extension(infilename, "afm");
			copyextension(savedextension, infilename);
/* next try with AFM extension */
			input = fopen(infilename, "rb");
			if (input == NULL) {
				underscore(infilename);
				input = fopen(infilename, "rb");
			}
		}
		if (input == NULL) {
			perror(argv[m]); 
			continue;
		}
		else fclose(input);		

/*		strncpy(outfilename, stripname(argv[m]), sizeof(outfilename));	*/ 
		strncpy(outfilename, stripname(infilename), sizeof(outfilename));	/* NEW */	
		if (traceflag != 0) 
			printf("nonstandard: %d suppressx %d\n", nonstandard, suppressx);
		if (removeX != 0 || nonstandard == 0) {
			if (stripanx(outfilename) != 0)
				printf("WARNING: reduced name to: %s\n", outfilename);
		}
		else if (nonstandard != 0) {
			if (suppressx == 0) {
				if (addanex(outfilename) != 0)
				printf("WARNING: extending name with `x': %s\n", outfilename);
									/* extend with `x' to indicate remapped */
			}
		}
/*		else if (stripanx(outfilename) != 0)
			printf("WARNING: reduced name to: %s\n", outfilename); */

/*		make sure to use same extension for output as for input */

/*		forceexten(outfilename, "pfb"); */
		if (traceflag)
			printf("Outfilename %s Savedextension %s\n",
				   outfilename, savedextension);
		forceexten(outfilename, savedextension);
		renameflag = 0;
		uppercase(infilename, infilename);
		uppercase(outfilename, outfilename);
		if (traceflag != 0)
			printf("IN: %s OUT: %s\n", infilename, outfilename); /* debug */
/*		if (strcmp(infilename, outfilename) == 0) { */
		if (samefile(infilename, outfilename)) {
			strcpy(oldfilename, infilename);
			forceexten(infilename, "bak");
			if (rename(oldfilename, infilename) == 0) renameflag = 1;
			else if (remove(infilename) == 0) {
				if (rename(oldfilename, infilename) == 0) renameflag = 1;
			}
			if (renameflag == 0) {
				fprintf(errout, "Failed to rename input file\n");
				break;
			}
			else if (verboseflag != 0) 
				printf("Renamed %s to %s\n", oldfilename, infilename);
		}

		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;		/* 	return (1); */
		}
		
/*		if (verboseflag != 0) */
		if (standardize != 0)	{			/* 1993/July/11 */
			printf("Standardizing %s ", infilename);
			if (verboseflag == 0) putc('\n', stdout);
		}
		else if (reinstateflag != 0) {		/* 1993/July/11 */
			printf("Reinstating StandardEncoding in %s ", infilename); 
			if (verboseflag == 0) putc('\n', stdout);
		}
		else if (verboseflag != 0)
			printf("Processing %s ", infilename);

		usingreturn = 0; usingboth = 0;
		c = fgetc(input); d = fgetc(input);
		if (c == EOF) {						/* 1995/Mar/1 */
			fprintf(errout, "Unexpected EOF - empty input file\n");
			fprintf(errout, "Do not process files in current directory\n");
			fclose(input);
			continue;
		}
		if (c == 'S' && d == 't') {
			afmfileflag = 1; pfafileflag = 0; outfileflag = 0;
			fclose(input);						/* Don't want binary mode */
			if (traceflag) printf("Trying to open %s\n", infilename);
			if ((input = fopen(infilename, "r")) == NULL) {
				perror(infilename);
				continue;		/* 	1992/Sep/25 */
			}
			if (strcmp(vectorname, "") == 0) {
				if (verboseflag != 0) putc('\n', stdout);
				if (readvectorfile("standard") < 0)
					exit(1);	/* return -1; */ /* ??? */
			}
			flag = processafmfile(input);
/*			fclose (input); */
		}
		else if (c == '%' && d == '%') {
			afmfileflag = 0; pfafileflag = 0; outfileflag = 1;
			fclose(input);						/* Don't want binary mode */
			if ((input = fopen(infilename, "r")) == NULL) {
				perror(infilename);
				continue;		/* 	1992/Sep/25 */
			}
			if (strcmp(vectorname, "") == 0) {
				if (verboseflag != 0) putc('\n', stdout);
				if (readvectorfile("standard") < 0)
					exit(1);	/* return -1; */ /* ??? */
			}
			flag = processoutfile(input);
/*			fclose (input); */
		}
		else if (c == '%' && d == '!') {
			afmfileflag = 0; pfafileflag = 1; outfileflag = 0;
/*			fclose(input);
			if ((input = fopen(infilename, "r")) == NULL) {
				perror(infilename);
				continue;
			} */
			flag = processpfafile(input);
/*			fclose (input); */
		}
		else if (c == 128 && d == 1) {
			afmfileflag = 0; pfafileflag = 0; outfileflag = 0;
/*	non-zero if rejected processing */
			flag = processpfbfile(input);
/*			fclose (input); */
		}
		else {
			fprintf(errout, 
				"ERROR: Appears not to be a valid PFB, PFA or AFM file\n");
			fclose(input);
			if (renameflag != 0) (void) rename(infilename, oldfilename);
			flag = -1;
/*			continue; */		/* return (1); */
		}

		if  (flag != 0) {							/* 1993/July/9 */
			if (renameflag != 0) strcpy(infilename, oldfilename);
			fprintf(errout, "Sorry, unable to process %s\n", infilename);
			continue;
		}

/* now have proccessed header of either PFA or PFB file up to Encoding */
/* now stick in new Encoding */

		if (afmfileflag == 0 && pfafileflag == 0) {
			if (reinstateflag != 0) {
				if (usingreturn != 0) changetoreturn(standard);
				else changetolinefeed(standard);
/*				fprintf(output, "%s", standard); */
				lputs(output, standard);
			}
			else {
				if (usingreturn != 0) changetoreturn(encoding);
				else changetolinefeed(encoding);
/*				fprintf(output, "%s", encoding); */
				lputs(output, encoding);
			}	
		}
		while ((c = getc(input)) != EOF) putc(c, output); /* copy the rest */
		fclose(input);
		if (fclose(output) == EOF) {
			perror(outfilename);
			break;				/* output device full - probably */
		}
		else filecount++;

/* try and modify time/date of output file --- 1993/July/3 */
		if (copydate != 0 &&
			(standardize != 0 || reinstateflag != 0)) {
			if (getinfo(infilename, traceflag) < 0)
				exit(1);
//			timebuf.actime = statbuf.st_atime;
			timebuf.actime = statbuf.st_atime;
//			timebuf.modtime = statbuf.st_atime;
			timebuf.modtime = statbuf.st_mtime;
			if (_utime(outfilename, &timebuf) != 0) {
				fprintf(stderr, "Unable to modify date/time\n");
				perror(outfilename);
			}
			if (getinfo(outfilename, traceflag) < 0)
				exit(1);
		}
	}
/* 	if (verboseflag != 0 && filecount > 0) */
	if (filecount > 1) 
		printf("Reencoded %d outline font files\n", filecount);
		
#ifdef _WIN32
	if ((m = _heapchk ()) != _HEAPOK) {		/* 94/Feb/18 */
		fprintf(stderr, "ERROR: heap corrupted (%d)\n", m);
		exit(1);
	}
#else
	if ((m = _fheapchk ()) != _HEAPOK) {		/* 94/Feb/18 */
		fprintf(stderr, "ERROR: Far heap corrupted (%d)\n", m);
		exit(1);
	}
#endif

	_ffree(encoding);
	return 0;
}

/* PROBLEM: some fonts have ASCII section up front in more than one part ! */
/* The above attempts to cope with this ... */

/* adjust to use of end-of-line character in input */
/* if control-M is used in input, then control-M is used in new stuff */
/* if control-J is used in input, then control-J is used in new stuff */
/* however, if control-M control-J is used there is no adjustment */

/* font name replacement only implemented for PFA files so far */
/* and assumes /Fontname comes before /Encoding */

/* searches for both PFB and PFA files */

/* stuff for flushing UniqueID doesn't work */
/* stuff for changing font name doesn't work */

/* are files always closed after being opened ? */

/* have a way to prevent ^M^J in output for AFM files ? */
