/* Copyright 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Y&Y, Inc.
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

/* Program for turning PFB file into Mac font file */

/* Generates required POST resources for font plus ... */
/* ... resources for icon, version numbers, copyright and so on */
/* Does NOT generate bitmap screen fonts and metric files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>

/* #define FNAMELEN 80 */
#define FONTNAME_MAX 64
#define MACBINARYHEAD 128
#define MAXRESOURCES 1024
#define ICONSIZE 128
#define ICONOFFSET 70

/* apparently long ASCII sections are OK - also IF broken, should be at nl */

#define MAXASCIIPOST 9192
#define MAXBINARYPOST 2048

/* #define VERSION "1.2"	*/		/* version of this program */
/* #define VERSION "1.3"	*/		/* version of this program */
/* #define VERSION "1.4"	*/		/* version of this program */

#ifdef _WIN32
char *title="PFBtoMAC font file conversion program (32) version 1.4\n";
#else
char *title="PFBtoMAC font file conversion program version 1.4\n";
#endif

/* #define MODDATE "1992 NOVEMBER 20" */		/* date of last modification */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

FILE *errout=stdout; 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *copyright = "\
Copyright (C) 1991--1998  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1991  Y&Y. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 1115060 */
/* #define COPYHASH 6413730 */
#define COPYHASH 14432335

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

char *defaulticons="c:\\icons";		/* default directory for icons */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int suppressversion = 0;	/* suppress version resource */
int uppercase = 0;			/* force MacBinary file name to upper case  */
int tryunderscore = 1;		/* non-zero => try underscore in font file name */
int fixposition = 1;		/* fixed desktop position */
/* int floatposition = 0; */		/* floating desktop position */
int macextension = 1;		/* add ".mac" to output file */

int makerandomrefid = 0;	/* NO, just use default 256 ? */
int lowerdef = 1;			/* lower case is not upper case ! */
int stripbolditalic = 0;	/* Strip Bold and Italic off Mac font name */

/* vers resource ID 1 specifies the version of specific file */
int wantversion1 = 1;		/* 0 or 1 - set if /Notice or /Copyright found */
/* vers resource ID 2 specifies the version of a set of files (library) */
int wantversion2 = 0;		/* 0 or 1 - set if user supplies Source ID */

/* vers ID 2 is shown following application name at top of GetInfo box */
/*				hence should NOT include application name */

/* vers ID 1 is shown after `Version:' in GetInfo */
/* vers ID 1 contains copyright information */ 

/* if neither vers ID 1 nor vers ID 2 present then */
/* GetInfo shows string in signature resource instead ... */

int wantmessage = 1;		/* non-zero => want message string when cicked */

/* struct stat filestat; */

struct _stat filestat;

/* this date nonsense: should really use `timediffer' and reference */

unsigned long yearsoff = 86400 * 1461;		/* offset 1904 - 1900 */
/* unsigned long yearsoff = 86400 * 24107;	*/	/* offset 1970 - 1904 */
/* unsigned long hoursoff = 3600 * 8; */	/* GMT - EZT ??? */
unsigned long hoursoff = 3600 * 9;			/* GMT - EZT ??? */

/* constraints on resource ID of icon */

unsigned int MinID = 1024;		/* possibly 128 */
unsigned int MaxID = 32767;
		
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void removeunder(char *filename) { /* remove Adobe style underscores */
	char *s;
	s = filename + strlen(filename) - 1;
	while (*s == '_') s--;
	*(s + 1) = '\0';
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char FontName[FONTNAME_MAX];		/* actual FontName from PS code itself */

char macbinaryname[FILENAME_MAX];	/* command line file name to use on Mac */

char MacName[64];				/* 5 + 3 + 3 ... Mac Style Font Name */

char BaseName[32];				/* base name derived from FontName */

#define MAXSUFFIX 8

char SuffixString[MAXSUFFIX][24];		/* suffix strings */

unsigned int nextsuffix;		/* count of suffix strings */

char iconfilename[FILENAME_MAX]="";	/* name of icon file */

char *iconname="";				/* name of icon */

char Notice[512];				/* Copyright Notice from font file */

char FontVersion[32];			/* version from font file */

int fontmajor, fontminor;		/* 001.002 style version number */

char Library[128]="";			/* Name of Library */

char LibraryVersion[32];		/* version of library */

int librarymajor, libraryminor;	/* 001.002 style version number */

/* int referenceid = 6969;	*/		/* 256 or 2264 - Resource ID of stuff */

int referenceid = 256;				/* 256 or 2264 - Resource ID of stuff */

/* 256 goes with default icon and File Type = LWFN */

char *iconnumber  = "ICN#";			/* icon resource */
char *filerefer   = "FREF";			/* file reference */
char *bundle      = "BNDL";			/* bundle = creator + icon + filerefer */
char *version     = "vers";			/* version */
char *postres     = "POST";			/* for actual font data resources */
char *filetype    = "LWFN";			/* LW standard for outline font file */
char *mstring     = "STR ";			/* for message string resource */

/* res ID -16396 => name of application */
/* res ID -16397 => explanation why file can't be opened or printed */

/* char *filecreator = "";	*/		/* or ASPF or IKP1 or BPFB or ... */
									/* or CMPS for Blue Sky Research CM */
									/* or LUCI for Bigelow & Holmes LB LNM */
									/* or WOLF for Woflram Mathematica */

char filecreator[4+1]="";			/* `application signature' */

/* Finder has table of signatures and icons */

char *defaultcreator = "ASPF";		/* default if not specified *or* no icon */

/* See `Inside MacIntosh' Vol VI page 9-36 for more information ??? */

/* 32 => has BNDL ?		1 => has been INITed ? */

unsigned char fileflags = 32;		/* was: 32 | 1 */

unsigned char protectedflags = 0;

int iconexists=0;		/* non-zero if user specified icon exists 94/May/19 */

int desktopv=-1;		/* vertical position of icon on desktop */
int desktoph=-1;		/* horizontal position of icon on desktop */
int desktopd=0;			/* directory of icon on desktop */

/* Message supposed to be displayed when double clicking on font file */

char *message = 
"This is an Adobe Type 1 scalable outline font file.  "
"You cannot open or print this file.  "
"For proper operation it should be in the System Folder.";

unsigned long dataforklength = 0;		/* no data fork, so zero */
unsigned long resourceforklength;		/* determined later */

unsigned long creationdate;				/* seconds since 1904 Jan 1 */
unsigned long modificationdate;			/* seconds since 1904 Jan 1 */

unsigned int resourcefileattributes = 0;		/* resource file attributes */

unsigned long ResDatOff=256;	/* offset to resource data */
unsigned long ResMapOff;		/* offset to resource map - later */
unsigned long ResDatLen;		/* length of resource data - later */
unsigned long ResMapLen;		/* length of resource map - later */

/* most of this is filled in only after first pass - written on second */

unsigned int TypeOffset;			/* offset to start of type list */
unsigned int NameOffset;			/* offset to start of name list */

unsigned long resourcepointer;			/* pointer into resource data */

unsigned long resourcemapstart;			/* pointer to start of resource map */
unsigned long referenceliststart;		/* pointer to start of references */
unsigned long typeliststart;			/* pointer to start of types */
unsigned long nameliststart;			/* pointer to start of names */

unsigned int nextresource;					/* pointer to next available */

unsigned long resourcedata[MAXRESOURCES];	/* pointer - resource data item */
											/* relative begin resource data */

unsigned int numberofposts;					/* count of POST resources */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void uwritetwo(FILE *output, unsigned int n) {
	unsigned char c, d;
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
	putc(c, output); putc(d, output);
}

void uwritethree(FILE *output, unsigned long n) {
	unsigned char c, d, e;
	e = (unsigned char) (n & 255);	n = n >> 8;
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
	putc(c, output); putc(d, output);
	putc(e, output);
}

void uwritefour(FILE *output, unsigned long n) {
/*	int c, d, e, f; */
	unsigned char c, d, e, f;
	f = (unsigned char) (n & 255);	n = n >> 8;
	e = (unsigned char) (n & 255);	n = n >> 8;
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
	putc(c, output); putc(d, output);
	putc(e, output); putc(f, output);
/*	printf("C %d D %d E %d F %d \n", c, d, e, f); */	/* debug */
}

unsigned long ureadfour(FILE *input) {	/* LS => MS */
	unsigned int c, d, e, f;
	c = getc(input); d = getc(input);
	e = getc(input); f = getc(input);
	return (((((((unsigned long) f) << 8) | ((unsigned long) e) << 8) |
		d) << 8) | c);
}

void writestring(FILE *output, char *str, unsigned int n) {
	unsigned int k;
	char *s = str;
	for (k = 0; k < n; k++) putc(*s++, output);
}

void writepascalstring(FILE *output, char *str) {
	unsigned int k, n = strlen(str);
	char *s = str;
	putc(n, output);
	for (k = 0; k < n; k++) putc(*s++, output);	
}

void writezeros(FILE *output, int n) {
	int k;
	for (k = 0; k < n; k++) putc(0, output);
}

void writeresourceheader(FILE *output) {
	uwritefour(output, ResDatOff);		/* offset to resource data */
	uwritefour(output, ResMapOff);		/* offset to resource map - later */
	uwritefour(output, ResDatLen);		/* length of resource data - later */
	uwritefour(output, ResMapLen);		/* length of resource map - later */
}

void writeuserdefined(FILE *output) {
	writezeros(output, 128);
}
	
void setupdateandtime(char *name) {
	time_t modtime;
/*	if (stat(name, &filestat) != 0) { */
	if (_stat(name, &filestat) != 0) {
		fprintf(errout, "WARNING: Can't get file modification date\n");
		return;				/* exit(9); */
	}
	modtime = filestat.st_atime;
	if (verboseflag != 0) 
		printf("File modification date: %s", ctime(&modtime));
/*	creationdate = modtime + yearsoff - hoursoff; */
	creationdate = modtime - yearsoff - hoursoff;
/*	printf("MODTIME %lu YEARSOFF: %lu HOURSOFF: %lu CREATIONDATE %lu\n", 
		modtime, yearsoff, hoursoff, creationdate); */
/*	modificationdate = modtime + yearsoff - hoursoff; */
	modificationdate = modtime - yearsoff - hoursoff;
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  */

void forceupper(char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

/* Break up name at hyphen `-'  -and-  where upper case follows lower case */
/* Treat digits as uppercase ? 98/Jun/19 NO */

void splitname(char *name) {
	char *s=name, *t=name;
	unsigned int k;
	int c, n, lowerflag;

	if (*name == '\0') 	fprintf(errout, "FontName NULL\n");

	if (strncmp(name, "ITC-", 4) == 0) name = name + 4;		/* ignore ITC- ??? */

	for (k = 0; k < MAXSUFFIX; k++) strcpy(SuffixString[k], "");
	nextsuffix = 0;

	if (uppercase != 0) {	/* no splitting if all upper case */
		strcpy(BaseName, FontName);
		forceupper(BaseName, BaseName);
		return;
	}

	lowerflag = 0;
	while ((c = *t) != '\0') {
		if (c == '-') break;
		if (lowerdef != 0) {	/*	following changed 93/March/15 */
			if (lowerflag != 0 && c >= 'A' && c <= 'Z') break;  
/*			if (lowerflag != 0 && c >= '0' && c <= '9') break; */ /* 98/Jun/19 */
/*			if (c < 'A' || c > 'Z') lowerflag = 1; */
		}
		else {
			if (lowerflag != 0 && (c < 'a' || c > 'z')) break; 
/*			if (c >= 'a' && c <= 'z') lowerflag = 1; */
		}
		if (c >= 'a' && c <= 'z') lowerflag = 1;
		t++;
	}
	n = t - s;
	if (n == 0) fprintf(errout, "Empty Base Name\n");
	strncpy(BaseName, name, n);
	*(BaseName + n) = '\0';
	if (c == '-')  {
		strcpy(SuffixString[nextsuffix++], "-");
		t++;
	}
	for(;;) {
		s = t;
		if (strcmp(s, "") == 0) break;	/* nothing left */
		lowerflag = 0;
		while ((c = *t) != '\0') {
			if (c == '-') break;
			if (lowerdef != 0) {
				if (lowerflag != 0 && c >= 'A' && c <= 'Z') break; 
/*				if (lowerflag != 0 && c >= '0' && c <= '9') break; */ /* 98/Jun/19 */
/*				if (c < 'A' || c > 'Z') lowerflag = 1; */
			}
			else {
				if (lowerflag != 0 && (c < 'a' || c > 'z')) break;
/*				if (c >= 'a' && c <= 'z') lowerflag = 1; */
			}
			if (c >= 'a' && c <= 'z') lowerflag = 1;
			t++;
		}
		n = t - s;
		if (n == 0) {
			fprintf(errout, "Empty Suffix\n");
			break;
		}
		if (nextsuffix >= MAXSUFFIX) {
			fprintf(errout, "ERROR: Too many suffixes\n");
			break;
		}
		strncpy(SuffixString[nextsuffix], s, n);
		SuffixString[nextsuffix][n] = '\0';
		nextsuffix++;
		if (c == '-')  {
			strcpy(SuffixString[nextsuffix++], "-");
			t++;
			continue;
		}
		if (c == '\0') break;
	}
	if (traceflag != 0) {
/*		putc('\n', stdout); */
/* 		printf("FontName is: %s\n", FontName); */
		printf("BaseName: `%s' ", BaseName);
		for (k = 0; k < nextsuffix; k++)
			printf("Suffix %d: `%s' ", k, SuffixString[k]);
		putc('\n', stdout);
	}
} 

/* Construct MacIntosh style 5 + 3 + 3 ... outline font file name */

void constructname(void) {
	int k=0, c;
/*	int lowcount; */
	int count, lowflag;
	unsigned m;

	strcpy (MacName, BaseName);
	if (strncmp(MacName, "ITC-", 4) == 0)  /* flush `ITC-' */
		strcpy(MacName, MacName + 4);
/*	lowcount = 0; */
	count = 0;
	lowflag = 0;
	while ((c = MacName[k]) > ' ') {
/*		if (c < 'A' || c > 'Z') lowflag = 1; */
		if (c >= 'a' && c <= 'z') lowflag = 1;
		if (lowflag != 0 && count > 4) break;
		count++;
		k++;
	}
/*	if (verboseflag != 0)  
		printf("MacName %s k %d low %d\n", MacName, k, lowflag); */
	MacName[k] = '\0'; 
/*	lowcount = 0; */

	for (m = 0; m < nextsuffix; m++) {
		if (strcmp(SuffixString[m], "-") == 0) continue;
/*		if (strcmp(SuffixString[m], "Regular") == 0) continue; */
/*		if (strcmp(SuffixString[m], "Roman") == 0) continue;   */
/*		if (strcmp(SuffixString[m], "Normal") == 0) continue;   */
		if (stripbolditalic != 0) {
			if (strcmp(SuffixString[m], "Bold") == 0) continue;
			if (strcmp(SuffixString[m], "Demi") == 0) continue;		/* ? */
			if (strcmp(SuffixString[m], "Semibold") == 0) continue; /* ? */
			if (strcmp(SuffixString[m], "Italic") == 0) continue;   
			if (strcmp(SuffixString[m], "Oblique") == 0) continue;	/* ? */
			if (strcmp(SuffixString[m], "Kursiv") == 0) continue;	/* ? */
		}
		if (*SuffixString[m] == '-') strcat(MacName, SuffixString[m]+1);
		else strcat(MacName, SuffixString[m]);
/*		lowcount = 0; */
		count = 0; 
		lowflag = 0;
		while ((c = MacName[k]) > ' ') {
/*			if (c < 'A' || c > 'Z') lowflag = 1; */
			if (c >= 'a' && c <= 'z') lowflag = 1;
			if (lowflag != 0 && count > 2) break;
			count++;
			k++;
		}		
		MacName[k] = '\0';
	}
/*	if (verboseflag != 0) 
		printf("Outline font file name should be: %s\n", MacName); */
	if (k > 31)				/* on the Mac font name limit ? */
		fprintf(errout, "Outline font name %s is too long\n", MacName);
	if (traceflag)
		printf("MacName: `%s' (%d) ", MacName, strlen(MacName));
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  */

void copysection(FILE *output, FILE *input, int c, 
			unsigned long length, int pass) {
	unsigned long k;

	if (pass > 0) {
		uwritefour(output, length + 2);
		putc(c, output); putc(0, output);
		for (k = 0; k < length; k++) putc(getc(input), output);
	}
	else for (k = 0; k < length; k++) (void) getc(input);

	resourcepointer += length + 4 + 2;
	resourcedata[nextresource++] = resourcepointer;
}

void writeeof(FILE *output, int pass) {
	if (pass > 0) {
		uwritefour(output, 2L);
		putc(5, output); putc(0, output);
	}
	resourcepointer += 4 + 2;
	resourcedata[nextresource++] = resourcepointer;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* will need to break sections up into shorter pieces --- 2046 max */
/* will need to keep track of offsets so can build resource map */

int	copyfontfile(FILE *output, FILE *input, int pass) {
	int c;
	unsigned long length;

	resourcepointer = 0;
	nextresource = 0;
	resourcedata[nextresource++] = resourcepointer;

	for(;;) {
		c = getc(input);
		if (c != 128) {
			fprintf(errout, "ERROR: Improper PFB file format\n");
			return -1;
		}
		c = getc(input);
		if (c == 1 || c == 2) length = ureadfour(input);
		else length = 0;

		if (c == 1) {		/* ASCII section code */
			while (length > MAXASCIIPOST - 2) {
				copysection(output, input, c, MAXASCIIPOST - 2, pass);
				length -= (MAXASCIIPOST - 2);
			}
			copysection(output, input, c, length, pass);
		}
		else if (c == 2) {	/* Binary section code */
			while (length > MAXBINARYPOST - 2) {
				copysection(output, input, c, MAXBINARYPOST - 2, pass);
				length -= (MAXBINARYPOST - 2);
			}
			copysection(output, input, c, length, pass);
		}
		else if (c == 3) {	/* EOF section code */
			writeeof(output, pass);
			return 0;
		}
		else if (c == EOF) {
			fprintf(errout, "ERROR: Unexpected EOF in PFB file\n");
			return -1;
		}
		else {
			fprintf(errout, "ERROR: Improper PFB file format\n");
			return -1;
		}
	}
}

unsigned char defaulticon[128] = { 
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0xFD, 0xF7,
0x3F, 0x80, 0x01, 0xF0, 0xDF, 0xDF, 0xFD, 0xF7,
0x0F, 0xE0, 0x01, 0xF0, 0xF7, 0xF7, 0xFD, 0xF7,
0x03, 0xF8, 0x01, 0xF0, 0xFD, 0xFF, 0xFF, 0xF7,
0x00, 0xFF, 0xFF, 0xF0, 0xFF, 0x7F, 0xFF, 0xF7,
0x00, 0x3F, 0x81, 0xF0, 0xFF, 0xDF, 0xDD, 0xF7,
0x00, 0x0F, 0xE1, 0xF0, 0xFF, 0xF7, 0xF5, 0xF7,
0x00, 0x03, 0xF9, 0xF0, 0xFF, 0xFD, 0xFD, 0xF7,
0x00, 0x00, 0xFF, 0xF0, 0xFF, 0xFF, 0x7F, 0xF7,
0x00, 0x00, 0x3F, 0xF0, 0xFF, 0xFF, 0xDF, 0xF7,
0x00, 0x00, 0x0F, 0xF0, 0xFF, 0xFF, 0xF7, 0xF7,
0x00, 0x00, 0x03, 0xF0, 0xFF, 0xFF, 0xFD, 0xF7,
0x00, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0x77,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
};

unsigned char defaultmask[128] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

/* Let (A, B) be the two bitmaps on the PC */
/* Let (I, M) be the two bitmaps on the Mac */

/* Then:  A = I ^ M  and  B = ~M, while  I = A ^ ~B  and M = ~B */

/* void copyicon(FILE *output) { */
int copyicon(FILE *output, int pass) {
	int i, j, k;
	int flag=1;				/* non-zero if using icon other than default */
	unsigned char icon[ICONSIZE], mask[ICONSIZE];
	FILE *iconfile=NULL;

/*	if (pass == 0) return;	 */						/* ignore on first pass */

/*	if (strcmp(iconfilename, "") == 0) 	flag = 0; */	/* no icon specified */
	if (strcmp(iconname, "") == 0) flag = 0;
	else {
		strcpy(iconfilename, iconname);
		extension(iconfilename, "ico");
		if ((iconfile = fopen(iconfilename, "rb")) == NULL) {
			strcpy (iconfilename, defaulticons);	/* DEFAULT DIRECTORY */
			strcat (iconfilename, "\\");
			strcat (iconfilename, iconname);
			extension(iconfilename, "ico");			
			if ((iconfile = fopen(iconfilename, "rb")) == NULL) {
				fprintf(errout, "WARNING: ");
				perror(iconfilename);				/* file not found */
				flag = 0;					/* will use default icon */
			}
		}
		if (pass == 0 && flag != 0) {
			fclose(iconfile);
			iconfile = NULL;
		}
	}
	
	if (pass == 0) {
/*		if (iconfile != NULL) {
			fclose(iconfile);
			iconfile = NILL;
		} */
		return flag;
	}

	if (verboseflag) {
		if (flag) printf("Using %s as icon\n", iconfilename);
		else printf("Using default icon\n");
	}

	if (! flag) {				/* use default icon */
		for (k = 0; k < ICONSIZE; k++) icon[k] = defaulticon[k];
		for (k = 0; k < ICONSIZE; k++) mask[k] = defaultmask[k];
	}

	if (flag) {			/* have a real icon to work with */
/*	skip over header information --- assumes 32 x 32 bilevel icon with mask */
		for (k = 0; k < ICONOFFSET; k++) (void) getc(iconfile);
/*	read icon white & inverse */
		for (k = 0; k < ICONSIZE; k++) icon[k] = (unsigned char) getc(iconfile);
/*	read icon white & background */
		for (k = 0; k < ICONSIZE; k++) mask[k] = (unsigned char) getc(iconfile);
	}

/*	this may need more work - boolean combinations ... */
	for (i = 0; i < 32; i++) {
		k = (31 - i) * 4;
		for (j = 0; j < 4; j++) {
			putc(icon[k] ^ ~mask[k], output); k++;
		}
	}
	
	for (i = 0; i < 32; i++) {
		k = (31 - i) * 4;
		for (j = 0; j < 4; j++) {
			putc(~mask[k], output); k++;
		}
	}

/*	for (k = 0; k < ICONSIZE; k++) putc(icon[k] ^ ~mask[k], output); */
/*	for (k = 0; k < ICONSIZE; k++) putc(~mask[k], output); */
	if (iconfile != NULL) {
		fclose(iconfile);
		iconfile = NULL;
	}
	return flag;
}

/* 32 => development, 64 => alpha, 96 => beta, 128 => release */

int stagecode = 128;	/* development stage code */
int userstage = 0;		/* user-defined stage */

void addresources(FILE *output, int pass) {	/* resources after font data */
	unsigned int n, m;

/*	first comes BNDL */
	if (pass > 0)
		uwritefour(output, 28L);			/* length of BNDL resource */
	resourcepointer += 4;
	if (pass > 0) {
		writestring(output, filecreator, 4);	/* Application Signature */
		uwritetwo(output, 0);					/* Res ID of Version Data ? */
		uwritetwo(output, 2 - 1);				/* No. of Resource Types-1 */
		writestring(output, iconnumber, 4);		/* Resource Type - ICN# */
		uwritetwo(output, 1 - 1);				/* No. of this Type - 1 */
		uwritetwo(output, 0);					/* Local ID */
		uwritetwo(output, referenceid);			/* 256 - Actual ID */
		writestring(output, filerefer, 4);		/* Resource Type - FREF */
		uwritetwo(output, 1 - 1);				/* No. of this Type - 1 */
		uwritetwo(output, 0);					/* Local ID */
		uwritetwo(output, referenceid);			/* 256 - Actual ID */
	}
	resourcepointer += 28;
	resourcedata[nextresource++] = resourcepointer;
/*	second comes creator - application signature - with font name */
/*  this could be some other string - is what is shown if no `vers' res */
	n = strlen(FontName);
	if (pass > 0)
		uwritefour(output, (unsigned long) (n + 1));
	resourcepointer += 4;
	if (pass > 0)
		writepascalstring(output, FontName);
	resourcepointer += n + 1;
	resourcedata[nextresource++] = resourcepointer;
/*	third comes file reference */
	if (pass > 0)
		uwritefour(output, 7L);
	resourcepointer += 4;
	if (pass > 0) {
		writestring(output, filetype, 4);
		putc(0, output);	putc(0, output);	putc(0, output);
	}
	resourcepointer += 7;
	resourcedata[nextresource++] = resourcepointer;
/*	fourth comes the icon */
	if (pass > 0)
		uwritefour(output, 256L);		/* icon + mask */
	resourcepointer += 4;
/*	if (pass > 0)
		copyicon(output); */			/* copy in icon from file */
	iconexists = copyicon(output, pass);/* copy in icon from file 94/May/19 */

	resourcepointer += 128 + 128;
	resourcedata[nextresource++] = resourcepointer;	
/*	fifth comes version number */
/*	vers resource ID 1 specifies the version of specific file */
	if (wantversion1 != 0) {			/* vers 1 */
		n = strlen(FontVersion);
		m = strlen(Notice);
		if (pass > 0)
			uwritefour(output, 6 + (n+1) + (n + 3 + m + 1));
		resourcepointer += 4;
		if (pass > 0) {
			putc(fontmajor, output);/* 2 BCD digit fontmajor version number */
			putc(fontminor, output);/* 2 BCD digit fontminor version number */
			putc(stagecode, output);	/* development stage code */
			putc(userstage, output);	/* user-defined stage code */
			uwritetwo(output, 0);		/* country code */
		}
		resourcepointer += 6;
/*		if (pass > 0) {
			putc(n, output);
			writestring(output, FontVersion, n);	
		} */
		if (pass > 0)
			writepascalstring(output, FontVersion);
		resourcepointer += n + 1;
		if (pass > 0) {
			putc(n + m + 3, output);
			writestring(output, FontVersion, n);
			putc(' ', output);
			putc(169, output);	/* copyright symbol */
			putc(' ', output);
			writestring(output, Notice, m);	
		}
		resourcepointer += n + m + 3 + 1;	
		resourcedata[nextresource++] = resourcepointer;	
	}
/* vers resource ID 2 specifies the version of a set of files */
/*	next comes source library info - if supplied */
	if (wantversion2 != 0) {			/* vers 2 */
		n = strlen(LibraryVersion); 
		m = strlen(Library);
		if (pass > 0)
			uwritefour(output, 6 + (n+1) + (m + 1)); 
		resourcepointer += 4;
		if (pass > 0) {
			putc(librarymajor, output);	/* 2 BCD digit major version number */
			putc(libraryminor, output);	/* 2 BCD digit minor version number */
			putc(stagecode, output);	/* development stage code */
			putc(userstage, output);	/* user-defined stage code */
			uwritetwo(output, 0);		/* country code */
		}
		resourcepointer += 6;
/*		if (pass > 0) {
			putc(n, output);
			writestring(output, LibraryVersion, n);	
		} */
		if (pass > 0)
			writepascalstring(output, LibraryVersion);				
		resourcepointer += n + 1;
/*		if (pass > 0) {
			putc(m, output);
			writestring(output, Library, m);
		} */
		if (pass > 0)
			writepascalstring(output, Library);
		resourcepointer += m + 1;
		resourcedata[nextresource++] = resourcepointer;	
	}
	if (wantmessage != 0) {		/* warning message when attempt to open */
		n = strlen(message);
		if (pass > 0) 
			uwritefour(output, (unsigned long) (n + 1));
		resourcepointer += 4;
		if (pass > 0) 
			writepascalstring(output, message);
		resourcepointer += n + 1;
		resourcedata[nextresource++] = resourcepointer;
	}
/*	fill in data now available */
	ResDatLen = resourcepointer;	/* resource data length */
	ResMapOff = 256 + resourcepointer;	/* resource data map start */
}

unsigned int referoffset;				/* pointer into reference list */

void writetype(FILE *output, char *name, unsigned int n, int pass) {
	if (pass > 0) {
		writestring(output, name, 4);
		uwritetwo(output, n - 1);
		uwritetwo(output, referoffset);		/* offset to type list */
	}
	referoffset += 12 * n;
	resourcepointer += 4 + 2 + 2;
}

void writereferitem(FILE *output, unsigned int resid, 
		unsigned int attribute, unsigned long resptr, int pass) {
	if (pass > 0) {
		uwritetwo(output, resid);		/* resource ID */
/*		uwritetwo(output, -1); */		/* not a named resource */
		uwritetwo(output, 0xFFFF);		/* not a named resource ??? */
		putc(attribute, output);		/* Resource Attributes */
		uwritethree(output, resptr);	/* pointer to resource data */
		uwritefour(output, 0L);			/* RESERVED */
	}
	resourcepointer += 12;
}

/* NOTE: items following must be in same order as below ... */

void writetypelist(FILE *output, int pass) {
	int ntypes = 5;					/* number of different resource types */
	if (wantversion1 != 0 || wantversion2 != 0) ntypes++;
	if (wantmessage != 0) ntypes++;
	referoffset = 2 + ntypes * 8;
	if (pass > 0)
		uwritetwo(output, ntypes - 1);		/* number of types - 1 */
	resourcepointer += 2;
/*  start with POST resources */
	writetype(output, postres, numberofposts, pass);
/*	next do BNDL resource */
	writetype(output, bundle, 1, pass);
/*	next do `creator' signature resource */
	writetype(output, filecreator, 1, pass);
/*	next do FREF resource */
	writetype(output, filerefer, 1, pass);
/*	next do ICN# resource */
	writetype(output, iconnumber, 1, pass);
	if (wantversion1 != 0 || wantversion2 != 0)  /*	next do vers resource */
/*		writetype(output, version, 1); */
		writetype(output, version, wantversion1 + wantversion2, pass); 
	if (wantmessage != 0) writetype(output, mstring, 1, pass);
}

/* NOTE: items following must be in same order as above ... */

void writereferencellist(FILE *output, int pass) {
	int resourceid=501;		/* for POST resources */
	unsigned int k=0;

	while (k < numberofposts) 	/* POST */
		writereferitem(output, resourceid++, 0, resourcedata[k++], pass);
	writereferitem(output, referenceid, 0, resourcedata[k++], pass);/* BNDL */
	writereferitem(output,   0, 0, resourcedata[k++], pass);		/* Y&Y  */
	writereferitem(output, referenceid, 0, resourcedata[k++], pass);/* FREF */
	writereferitem(output, referenceid, 0, resourcedata[k++], pass);/* ICN# */
	if (wantversion1 != 0) 
		writereferitem(output, 1, 32, resourcedata[k++], pass);	/* vers */ 
	if (wantversion2 != 0) 
		writereferitem(output, 2, 32, resourcedata[k++], pass);	/* vers */ 
	if (wantmessage != 0)
/*		writereferitem(output, -16397, 0, resourcedata[k++], pass);	*//* STR */ 
		writereferitem(output, 0xBFF3, 0, resourcedata[k++], pass);	/* STR */
/* second arg should be unsigned int ??? */
}

#define DESKINCREMENT 64 /* #define DESKINCREMENT 50 */
#define DESKTOPWIDTH 350

void updatedesktop(void) {	/* pick a new desktop location */
	if (desktoph > DESKTOPWIDTH) {
		desktoph = 0; desktopv += DESKINCREMENT;
	}
	else desktoph += DESKINCREMENT;
}

/* Write MacBinary header - maybe don't bother to write when pass == 0 ? */

void writemacbinary(FILE *output) {
	unsigned int n;
/*	int k; */

/*	if (pass == 0) return;  */

	putc(0, output);						/* ZERO 1 */
/*	Actual Mac filename needs to be the same as that in header... */
/*	n = strlen(macfilename); */
	n = strlen(MacName);
/*	writepascalstring(output, macfilename); */
	writepascalstring(output, MacName);
	writezeros(output, 64 - (n + 1));

	writestring(output, filetype, 4);
	writestring(output, filecreator, 4);
	putc(fileflags, output);
	putc(0, output);						/* ZERO 2 */
/*	for (k = 0; k < 6; k++) putc(desktoplocation[k], output); */
	uwritetwo(output, desktopv);	/* vertical position */
	uwritetwo(output, desktoph);	/* horizontal position */
	uwritetwo(output, desktopd);	/* directory */
	if (fixposition == 0) updatedesktop();
	putc(protectedflags, output);
	putc(0, output);						/* ZERO 3 */
	uwritefour(output, dataforklength);		/* always zero */
	uwritefour(output, resourceforklength);	/* need to fix second pass */
	uwritefour(output, creationdate);
	uwritefour(output, modificationdate);
	writezeros(output, 128 - 99);
}

int pfbtomac(FILE *output, FILE *input, int pass) {
	unsigned long m;

/*	first comes the MacBinary header of 128 bytes */
	if (pass > 0)
		writemacbinary(output);
	
/*	data fork --- which is empty */
	
/*  resource fork --- which where all the action is */
	
/*	first comes the resource header */
	
	if (pass > 0) {
		writeresourceheader(output);	/* resource header */
		writezeros(output, 128 - 16);	/* fill to 128 bytes with zeros */
	}

/*  second comes the user defined data - 128 bytes of zeros */
	
	if (pass > 0)
		writeuserdefined(output);		/* user-defined area - application */

	ResDatOff = 128 + 128;				/* resource header + user defined */
	
/*  third comes the POST resource data of the outline font itself */	

	(void) copyfontfile(output, input, pass);	/* insert resource data */
	numberofposts = nextresource - 1;	/* number of POST resources */
	if (nextresource >= MAXRESOURCES) {
		fprintf(errout, 
			"ERROR: Too many (%d) resources for table\n", nextresource);
		return -1;
	}

/*	finish off resource data  with other resources - icons and such */

	addresources(output, pass);
	if (nextresource >= MAXRESOURCES) {
		fprintf(errout, 
			"ERROR: Too many (%d) resources for table\n", nextresource);
		return -1;
	}

/*	fourth in the resource fork comes the resource map */

	resourcemapstart = resourcepointer;	/* remember this place */

	if (pass > 0) {
		writeresourceheader(output);	/* repeat resource header 16 bytes */
		writezeros(output, 4);			/* reserved - handle to next res map */
		writezeros(output, 2);			/* reserved - file reference number */
		uwritetwo(output, resourcefileattributes);
	}
	TypeOffset = (unsigned int) (typeliststart - resourcemapstart);
	if (pass > 0)
		uwritetwo(output, TypeOffset);		/* offset to type list */
	NameOffset = (unsigned int) (nameliststart - resourcemapstart);
	if (pass > 0)
		uwritetwo(output, NameOffset);		/* offset to type list */
	resourcepointer += 16 + 4 + 2 + 2 + 2 + 2;
	
	typeliststart = resourcepointer;
/*	now write the resource Type List */
	writetypelist(output, pass);
	referenceliststart = resourcepointer;	/* remember this place */

/*  now write the reference list */
	writereferencellist(output, pass);
	nameliststart = resourcepointer;		/* remember this place */
	
	ResMapLen = resourcepointer -  resourcemapstart;

	resourceforklength = resourcepointer + 128 + 128;

	m = dataforklength + resourceforklength;
	if (pass > 0) {
		while(m++ % 512 != 0) putc(0, output);
	}
	
	return 0;
}

/* search for specified string in file - returns zero if found */
/* returns non-zero and rewinds when it fails */

int findmatch(FILE *input, char *str) {
	int c;
	unsigned int k=0, n=strlen(str);
	char *s=str;

	for(;;) {
		c = getc(input);
		if (c == EOF) {
			fprintf(errout, "WARNING: Did not find %s\n", str);
			rewind(input);
			return -1;
		}
		if (c == *s) {
			k++; s++;
			if (k == n) return 0;
		}
		else {
			k = 0; s = str;
		}
	}
}

void extractstring(FILE *input, char *str) {
	int c, level=1; 
	char *s = str;

	while ((c = getc(input)) != '(' && c != EOF) ;	/* scan up to `(' */
	for(;;) {
		c = getc(input); if (c == EOF) break;
		if (c == ')' && level == 1) break;
		if (c == '(') level++;
		else if (c == ')') level--;
		*s++ = (char) c;
	}
	*s++ = '\0';
}

/* remove extraneous stuff in Notice - return non-zero if changed */

int cleanupnotice (char *note, int nmax) {
	char *s, *t;
	int c, n, flag=0;

/*	strip at beginning */
	if ((s = strstr(note, "Copyright")) != NULL) { /* flush words */
		t = note; s = s + 10; 
		while ((c = *s++) != '\0') *t++ = (char) c;
		*t = '\0'; 	flag++;
	}
	if ((s = strstr(note, "copyright")) != NULL) { /* flush words */
		t = note; s = s + 10; 
		while ((c = *s++) != '\0') *t++ = (char) c;
		*t = '\0'; flag++;
	}
	if ((s = strstr(note, "(c)")) != NULL) { /* flush words */
		t = note; s = s + 4; 
		while ((c = *s++) != '\0') *t++ = (char) c;
		*t = '\0'; flag++;
	}
	if ((s = strstr(note, "(C)")) != NULL) { /* flush words */
		t = note; s = s + 4; 
		while ((c = *s++) != '\0') *t++ = (char) c;
		*t = '\0'; flag++;
	}	
/*	strip at end */
/*	if ((s = strstr(note, "All rights reserved")) != NULL) { */
	if ((s = strstr(note, "All rights reserved")) != NULL ||
		(s = strstr(note, "All Rights Reserved")) != NULL) {
		s--;
		while (*s == ' ') *s-- = '\0';
		flag++;
	} 
	n = strlen(note);
	if (n > nmax) {		 /* try and shorten */ /* was 50 - 1992/May/22 */
		if ((s = strrchr(note, '.')) != NULL) {
			if (s - note > n - 4) {		/* is period right at end ? */
				*s = '\0';
				if ((s = strrchr(note, '.')) != NULL) 
					*(s+1) = '\0';
			}
			else *(s+1) = '\0';
			flag++;
		}
	}
	return flag;
}

void showusage(char *name) {
	printf("Useage: %s -v -s -u\n", name);
	printf("\t-c=<creator> -i=<icon-file> -l=\"<library> <version>\"\n");
/*	printf("\t-f=<mac-name> -r=<resource-id> -x=<x-pos> -y=<y-pos>\n"); */
	printf("\t-f=<mac-name> -r=<resource-id>\n");
	printf("\t\t<pfb-file-1>, <pfb-file-2>, ...\n");
	if (detailflag == 0) exit(0);
	printf("\tv  verbose mode\n");
	printf("\ts  suppress font version number resource\n");
	printf("\tu  force file name in MacBinary header to upper case (CM fonts)\n");
	printf("\tc  creator - 4 characters (default: use icon name, or `%s')\n",
		defaultcreator);
	printf("\ti  SDK 32 x 32 (bi-level) icon (default: use ASPF icon)\n");
	printf("\tl  font library name - followed by version number\n");
	printf("\tf  file name MacBinary header (override automatically generated name)\n");
	printf("\tr  icon resource ID - (default %d)\n", referenceid);
/*	printf("\tx  horizontal position of icon on desktop (for single file)\n"); */
/*	printf("\ty  vertical position of icon on desktop (for single file)\n"); */
	printf("\n");
	printf("\tFontName, Version and Notice picked out of PFB font file.\n");
	printf("\tOutput is in MacBinary format & appears in current directory.\n");
/*	printf("\tOn MacIntosh, file name must match MacBinary header name.\n"); */
	printf("\tMacBinary file name derived using 5 + 3 + 3 ... contraction.");
	exit(0);
}

int iconflag=0, macfilenameflag=0, creatorflag=0;
int resourceflag=0, libraryflag=0, horizflag=0, vertiflag=0, depthflag=0;

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0;
		case 'v': verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case 's': suppressversion = 1; return 0;
		case 'u': uppercase = 1; return 0;
/*		case 'z': floatposition = 1; return 0; */
		case 'z': desktoph = 0; desktopv = 0; fixposition = 0; return 0;
/*		the rest need arguments */
		case 'i': iconflag = 1; break;
		case 'f': macfilenameflag = 1; break;
		case 'c': creatorflag = 1; break;
		case 'r': resourceflag = 1; break;
		case 'l': libraryflag = 1; break;
/*		case 'x': horizflag = 1; fixposition = 1; break; */
/*		case 'y': vertiflag = 1; fixposition = 1; break; */
/*		case 'd': depthflag = 1; fixposition = 1; break; */
		default: {
			fprintf(stderr, "WARNING: Invalid command line flag '%c'\n", c);
				exit(7);
		}
	}
	return -1;		/* need argument */
}

int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command line flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (iconflag != 0) {
/*					strncpy(iconfilename, s, 80); */
					iconname = s;
/*					extension(iconfilename, "ico"); */
					iconflag = 0;
				}
				else if (resourceflag != 0) {
					if (sscanf(s, "%d", &referenceid) < 1) {
						fprintf(errout, "Don't understand resource ID: %s\n",
							s);
					}
					resourceflag = 0;
				}
				else if (horizflag != 0) {
					if (sscanf(s, "%d", &desktoph) < 1) {
						fprintf(errout, "Don't understand position: %s\n",
							s);
					}
					horizflag = 0;
				}
				else if (vertiflag != 0) {
					if (sscanf(s, "%d", &desktopv) < 1) {
						fprintf(errout, "Don't understand position: %s\n",
							s);
					}
					vertiflag = 0;
				}
				else if (depthflag != 0) {
					if (sscanf(s, "%d", &desktopd) < 1) {
						fprintf(errout, "Don't understand position: %s\n",
							s);
					}
					depthflag = 0;
				}
				else if (macfilenameflag != 0) {
					strncpy(macbinaryname, s, 80);
					macfilenameflag = 0;
				}
				else if (creatorflag != 0) {
					if (strlen(s) > 4)
						fprintf(errout, 
							"WARNING: file creator more than 4 characters\n");
					strncpy(filecreator, s, 4);
					while (strlen(filecreator) < 4) 
						strcat (filecreator, " "); /* pad it with spaces */
					filecreator[4] = '\0';
/*					filecreator = s; */
					creatorflag = 0;
				}
				else if (libraryflag != 0) {
					strncpy(Library, s, 80);
					libraryflag = 0;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

/*
void shortennotice(char *note, int nmax) {
	int n;
	char *s, *t;

	while ((n = strlen(note)) > nmax) {
		if ((s = strrchr(note, '.')) != NULL) {
			*s = '\0';
		}
		else {
			s = (note + nmax - 1);
			*s = '\0';
		}
	}
	*s++ = '.'; *s = '\0';
} */

int shortennotice (char *note, unsigned int nmax) {
	char *s;

	if (strlen(note) < nmax) return 0;					/* don't bother */

	if ((s = strchr(note, '.')) != NULL) *(s+1) = '\0';
	else {
		s = note + strlen(note) - 1; *s++ = '.'; *s++ = '\0';
	}
	if (strlen(note) > nmax) {
		s = note + nmax - 1;
		*s = '\0';
	}
	return -1;
}

void extractlibrary(void) { /* get version number out of command line arg */
	char *s;
	if ((s = strrchr(Library, ' ')) == NULL) {
		fprintf(errout, "WARNING: no library version number\n");
		strcpy(LibraryVersion, "1.0");
		return;
	}
/*	*s++ = '\0'; */
	if (verboseflag != 0) printf("Library is: %s\n", Library);
	strcpy(LibraryVersion, s);
	if (verboseflag != 0) printf("LibraryVersion is: %s - ", LibraryVersion);
	librarymajor = 1; libraryminor = 0;
	sscanf(LibraryVersion, "%d.%d", &librarymajor, &libraryminor);
	if (verboseflag != 0) 
		printf("Major version is %d and minor version is %d\n", 
			librarymajor, libraryminor);
		if (librarymajor > 9)	/* convert to two-digit BCD */
			librarymajor = (librarymajor / 10) * 16 + (librarymajor - (librarymajor / 10) * 10);
		if (libraryminor > 9)	/* convert to two-digit BCD */
			libraryminor = (libraryminor / 10) * 16 + (libraryminor - (libraryminor / 10) * 10);
	wantversion2 = 1;
}

unsigned long hashstring(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	return hash;
}

unsigned long checkcopyright(char *s) {
	unsigned long hash;
	hash = hashstring(s);
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(errout, "HASHED %ld\n", hash);	/* (void) getch();  */
	return hash;
}

void underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = stripname(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
}

unsigned int makerandom(unsigned int bottom, unsigned int top) {
	unsigned int trial;
	for(;;) {
		trial = rand() + rand();
		if (trial > bottom && trial < top) return trial;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	char pfbfile[FILENAME_MAX];	/* file name of input PFM file */
	char macfile[FILENAME_MAX];	/* file name of output file */
	FILE *input, *output;
	unsigned long hash;
	int c, m, firstarg = 1;
	unsigned int k;
	char *s;

	if (argc < 2) showusage(argv[0]);
	
/*	if (strchr(argv[1], '?') != NULL) showusage(argv[0]); */

	firstarg = commandline(argc, argv, 1);

	if (firstarg >= argc) showusage(argv[0]);

	printf(title);

	if (strcmp(filecreator, "") == 0) {			/* if no creator specified */
/*		if (strcmp(iconfilename,"") != 0) {	*/	/* if icon was specified */
		if (strcmp(iconname,"") != 0) {			/* if icon was specified */
/*			s = stripname(iconfilename); */
			s = stripname(iconname);
			strncpy(filecreator, s, 4);
			filecreator[4] = '\0';
/* signature is supposed to contain at least one upper case letter */
			forceupper(filecreator, filecreator);	
			if (verboseflag != 0) 
				printf("Using %s as creator (application signature)\n", 
					filecreator);
		}
		else strcpy(filecreator, defaultcreator);
	}

	if (strcmp(Library, "") != 0) extractlibrary();

	if (strcmp(macbinaryname, "") != 0 && argc > firstarg) {
		fprintf(errout, 
		"WARNING: Attempt to specify MacBinary filename for multiple files\n");
		strcpy(macbinaryname, "");
	}

	if (fixposition != 0 && argc > firstarg &&
		desktoph != -1 && desktopv != -1) {
		fprintf(errout, 
			"WARNING: Attempt to fix position of multiple icons\n");
		fixposition = 0;
	}

	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

/*	if (strcmp(iconfilename, "") != 0) { */
	if (strcmp(iconname, "") != 0) {
/*		if (verboseflag != 0) printf("Using %s as icon\n", iconfilename); */
/*		hash = hashstring(argv[argc-1]); */
		s = stripname(argv[firstarg]);
		hash = hashstring(s);	
		srand ((unsigned int) (hash + (hash >> 16)));
		if (makerandomrefid != 0)
			referenceid = makerandom(MinID, MaxID / 2);
		if (verboseflag != 0) 
			printf("Using %d as icon ResID\n", referenceid);
	}
	else if (verboseflag != 0) 
		printf("Using default icon ResID (%d)\n", referenceid);

	if (verboseflag != 0) putc('\n', stdout);	/* 1992/July/2 */

	for (m = firstarg; m < argc; m++) {
		strcpy(pfbfile, argv[m]);
		extension(pfbfile, "pfb");
		if ((input = fopen(pfbfile, "rb")) == NULL) {
			if (tryunderscore != 0) underscore(pfbfile);
			if((input = fopen(pfbfile, "rb")) == NULL) {			
				strcpy(pfbfile, argv[m]);
				extension(pfbfile, "pfb");
				perror(pfbfile);
/*				exit(3); */
				continue;
			}
		}
		
/*		if (verboseflag != 0)  */
			printf("Processing %s\n", pfbfile);
		c = getc(input); 
		(void) ungetc(c, input);
		if (c == '%') 
		fprintf(errout, "ERROR: Input is NOT a PFB file (possibly PFA)\n");
		else if (c != 128)
			fprintf(errout, "ERROR: Input is NOT a PFB file\n");
		
		strcpy(macfile, stripname(pfbfile));
		removeexten(macfile);
		removeunder(macfile);
		if (macextension != 0) 	extension(macfile, "mac");	/* 1992/Nov/22 */
		if((output = fopen(macfile, "wb")) == NULL) {
			perror(macfile);
			fclose(input);
			exit(3);
		}
		
		setupdateandtime(pfbfile);
		
/*		strcpy(iconfilename, macfile);		*/
/*		forceexten(iconfilename, "ico");	*/
		
/*		if (strcmp(macbinaryname, "") != 0) 
			strcpy(macfilename, macbinaryname);
		else strcpy(macfilename, macfile); 
		if (uppercase != 0) forceupper(macfilename, macfilename);
		if (verboseflag != 0) 
			printf("MacBinary header file name: %s\n", macfilename); */

		if (findmatch(input, "/FontName ") == 0) {
			fscanf(input, "/%s ", FontName);
			if (verboseflag != 0) printf("FontName is: %s\n", FontName);
			rewind(input);
		}
		else fprintf(errout, "ERROR: Did not find FontName in PFB file\n");
		
		if (traceflag) printf("Splitting Name\n");
		splitname(FontName);			/* 1992/May/21 */
		if (traceflag) {
			printf("Name Components are: %s ", BaseName);
			for (k = 0; k < nextsuffix; k++) {
				printf("%s ", SuffixString[k]);
			}
			putc('\n', stdout);
		}
		if (traceflag) printf("Constructing Mac Name\n");		
		constructname();				/* 1992/May/21 */
		if (verboseflag) printf("Mac font file name will be: %s\n", MacName);
		if (strcmp(macbinaryname, "") != 0) {
			strcpy(MacName, macbinaryname);
			if (uppercase != 0) forceupper(MacName, MacName);
		}
		if (verboseflag != 0) 
			printf("MacBinary header file name: %s\n", MacName); 

		wantversion1 = 0;
		strcpy(Notice, "");			/* or get from copyright comment ? */
		if (suppressversion == 0) {
			if (findmatch(input, "/Notice ") == 0) { /* picks first one */
				extractstring(input, Notice);
				if (verboseflag != 0) printf("Notice: %s\n", Notice);	
				rewind(input);
				wantversion1 = 1;
			}
			else if (findmatch(input, "/Copyright ") == 0) {
				extractstring(input, Notice);
				if (verboseflag != 0) printf("Copyright was: %s\n", Notice);
				rewind(input);
				wantversion1 = 1;
			}
			else {
				fprintf(errout,
				"WARNING: Did not find Notice or Copyright in PFB file\n");
				wantversion1 = 0;
			}
			(void) cleanupnotice(Notice, 128);
			if (shortennotice(Notice, 128) != 0) {
				if (verboseflag != 0) printf("Shortened to: %s\n", Notice);
			}
		}

		strcpy(FontVersion, "");	/* or get from first line ? */
		if (findmatch(input, "/version ") == 0) {
/*			fscanf(input, "(%s)", FontVersion); */
			extractstring(input, FontVersion);
/*			*(FontVersion + strlen(FontVersion) - 1) = '\0'; */
			if (verboseflag != 0) printf("FontVersion is: %s - ", FontVersion);
			fontmajor = 1; fontminor = 0;
			sscanf(FontVersion, "%d.%d", &fontmajor, &fontminor);
			if (verboseflag != 0) 
				printf("Major version is %d and minor version is %d\n", 
					fontmajor, fontminor);
			if (fontmajor > 9)	/* convert to two-digit BCD */
				fontmajor = (fontmajor / 10) * 16 + 
					(fontmajor - (fontmajor / 10) * 10);
			if (fontminor > 9)	/* convert to two-digit BCD */
				fontminor = (fontminor / 10) * 16 + 
					(fontminor - (fontminor / 10) * 10);
			rewind(input);
		}
		else fprintf(errout, "WARNING: Did not find version in PFB file\n");

		dataforklength = 0;						/* assumes zero throughout */
		
		(void) pfbtomac(output, input, 0);		/* first pass */
		rewind(input);	
		if (ftell(output) > 0) {
			fprintf(errout, "WARNING: output produced in first pass\n");
			rewind(output);
		}

		if (iconexists == 0) {		/* failed to find user specified icon */
			printf("WARNING: No icon specified, or icon not found\n");
			printf("         Reverting to default file creator: `%s'\n",
				defaultcreator);
			strcpy(filecreator, defaultcreator);		/* 1994/May/19 */
		}

		(void) pfbtomac(output, input, 1);		/* second pass */
		
		fclose(input);
		if (ferror(output) != 0) {
			perror(macfile);
			exit(11);
		}
		fclose(output);
		if (verboseflag != 0) putc('\n', stdout);
	}
	if (argc - firstarg > 1) 
		printf("Processed %d PFB file%s\n",
			  argc - firstarg, ((argc - firstarg) == 1) ? "" : "s");
	return 0;
}
	
/* this date nonsense: should really use `timediffer' and reference */

/* MacBinary header: */
	
/*	0	 1 BYTE		Zero	*/
/*	1	64 BYTES	File Name -  Pascal string format */
/*	65	 4 BYTES 	File Type	 (no length) */
/*	69	 4 BYTES	File Creator (no length) */
/*	73	 1 BYTE		File Flags */		/* 32 + 1 for font ? */
/*	74	 1 BYTE		Zero */
/*	75	 6 BYTES	DeskTop Location */
/*  81	 1 BYTE		Protected Flag */
/*	82	 1 BYTE		Zero */
/*	83	 LONG		Data Fork Length	*/
/*	87	 LONG		Resource Fork Length	*/
/*	91	 LONG		Creation Date	*/
/*	95	 LONG		Modification Date	*/
	
/* if ASCII sections are broken, be careful to break at end of line ? */

/* Resource ID # 0 => font name */
/* Resource ID # 1 => version number of font and copyright info */
/* Resource ID # 2 => library name and version number */

/* desktop position -1 -1 is special => arrange neatly */

/* desktop positions multiples of 64 maybe ? */

/* possibly convert nl, rt => rt in ASCII sections ? */
/* but then section length changes ? */

/* icons are 326 byte files: 80 byte header (ignored), 2 x 128 byte masks */

/* May want to add `STR ' resource */ /* message when fool double clicks */

/* BNDL, FREF and ICN# are just there to give the file its icon in Finder */
/* File Creator code represent the application the file belongs to */
/* File Type code represents a particular type of file */

/* Desktop holds a list that links each file to its creator */
/* Hence desktop icon is associated with the creator code */

/* See `Inside MacIntosh' Vol VI, page 9-6 for more information */

/* Hold down Option and Command keys as you start to rebuild desktop */

/* Default creator code is first four characters of icon file name, or ASPF */

/* TYPICAL USE: 

/*	pfbtomac -v -i=c:\icons\aspfadob -l="Adobe Systems Incorporated 001.007"
-f=TimesRom c:\psfonts\tir_____ */ /* -c=ASPF */ /* -r=6969 */

/*	pfbtomac -v -i=c:\icons\matitexp -l="TeXplorators 001.000" c:\psfonts\mt*.pfb */

/*	pfbtomac -v -i=c:\icons\yany -l="Y&Y 001.001" c:\psfonts\la*.pfb */

/*	pfbtomac -vu -i=c:\icons\cmpsbsr -l="Blue Sky Research 001.00B" c:\bsr\cm*.pfb */ /* -c=CMPS */ /*  -r=7568 */

/*	pfbtomac -v -i=c:\icons\lucdbiho -l="Bigelow & Holmes 001.002" c:\psfonts\lb*.pfb */ /* -c=LUCD  */ /* -r=9265 */
