/* Copyright 1990,1991,1992,1993,1994,1995,1996,1997,1998,1999 Y&Y, Inc.
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

/* Program making fake screen fonts from AFM file */
/* Generates fake bitmap screen fonts and metric file info in FOND */
/* Can `harmonize' four styles: regular, bold, italic, bolditalic */

#ifdef _WIN32
#define __far
#define _fmalloc malloc
#define _ffree free
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>

/* #define MAXFILENAME 100 */
#define FONTNAME_MAX 64
/* #define PATHLEN_MAX 64 */
/* #define PATHLEN_MAX 128 */
/* #define MAXLINE 256 */
#define MAXLINE 512

/* max code number used in Mac encoding */

/* #define MAXMAC 251 */
#define MAXMAC 255

#define MAXCHRS 256
#define CHARNAME_MAX 32
#define MAXENCODING 40

/* #define MAXKERNS 1200 */
/* #define MAXKERNS 1400 */
/* #define MAXKERNS 10000U */		/* 4 styles * 4 bytes - lots of memory ! */
/* #define MAXKERNS 4192U */		/* see allocation in main */

#define MAXRESOURCES 16
#define MAXOUTLINE 32

/* #define MAXSCRFILE 32767 */		/* max screen font size - for now */ 
#define MAXSCRFILE 65000U			/* using malloc */

#define MAXKERNSONE 8192U			/* using malloc */

#define UNKNOWN -1 					/* unknown widths code */
/* #define UNKNOWN -16383 */		/* unknown widths code */

/* #define VERSION "1.0"	*/		/* version of this program */
/* #define MODDATE "1992 MAY 10" */	/* date of last modification */

/* style codes */

#define REGULAR 0
#define BOLD 1
#define ITALIC 2
#define BOLDITALIC 3

unsigned int maxscrfile;						/* 1994/June/27 */
unsigned int maxkernsone;					/* 1994/June/27 */

FILE *errout=stdout;				/* 97/Sep/23 */

/* width of default character (Mac coords) .notdef ? space ? */
/* int defaultwidth = 256 * 4096/1000;*/
int defaultwidth = 1049;			/* 256 * 4096/1000 */

char line[MAXLINE];					/* input line buffer for AFM files */

/* char outlinefilename[MAXOUTLINE]=""; */

/* Following _should_ be the SAME for all AFM files grouped together */

char screncoding[MAXCHRS][CHARNAME_MAX];	/* desired screen font encoding */

char pfbencoding[MAXCHRS][CHARNAME_MAX];	/* actual outline font encoding */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following may have up to four versions for different styles */

/* character width tables */

int widths[4][MAXCHRS];			/* int widths[MAXCHRS]; */

unsigned int nwidths[4];		/* Mac coordinates */

/* pair kerning tables */

/* unsigned char kerna[4][MAXKERNS], kernb[4][MAXKERNS]; */

/* int kerns[4][MAXKERNS];	*/		/* int kerns[MAXKERNS]; */

int maxkerns;		/* 10000 for one style, 4192 for 4 styles */

struct kernpair {
	unsigned char a; unsigned char b; int kern;
}; 

/* struct kernpair kernpairs[4][MAXKERNS]; */			/* 1993/Nov/9 */

struct kernpair __far *kernpairs[4];					/* 1993/Nov/9 */

unsigned int nkerns[4];

int boldflag[4], italicflag[4]; /* int boldflag, italicflag; */

int stylecode[4];				/* bold * 1 + italic * 2 */

int stylepointer[4];			/* index into data sorted by style */

int nstyles = 0;					/* number of AFM files seen */

int style = 0;						/* ranges from 0 to 3 */

int basestyle;						/* index of `regular' font */

char FontName[4][FONTNAME_MAX];		/* actual FontName from AFM file */

char CommonFontName[FONTNAME_MAX];	/* Common FontName used for ResID */

int xll[4], yll[4], xur[4], yur[4];	/* Bounding Boxes */

int FONDxll, FONDyll, FONDxur, FONDyur;

int isfixedpitch[4];				/* if font is fixed pitch */

int allfixedpitch=0;				/* FOND is fixed pitch */

int ascender[4], descender[4], widthmax[4];	/* Ascender, Descender per AFM */

unsigned int NFNTResID[4];			/* NFNT Resource ID */

unsigned int NFNTResInit[4] = {0, 0, 0, 0};	/* NFNT Resource initial values */

int capheight[4], xheight[4];		/* not used ? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char MacName[4][64];				/* 5 + 3 + 3 ... Mac Style Font Name */

char CommonMacName[64];

#define MAXSUFFIX 12				/* maximum number of suffixes */

#define MAXBASE 32					/* max length of base name */

#define MAXCOMPONENT 24				/* max length of name component */

char BaseName[4][MAXBASE];			/* base name derived from FontName */

char SuffixString[4][MAXSUFFIX][MAXCOMPONENT];		/* suffix strings */

unsigned int nextsuffix[4];		/* count of suffix strings */

char CommonBaseName[MAXBASE];			/* base name derived from FontName */

char CommonSuffixString[MAXSUFFIX][MAXCOMPONENT];		/* suffix strings */

unsigned int nsuffixes;		/* count of suffix strings */

int needkerns;				/* non-zero if kerning tables needed */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* unsigned char scrfile[MAXSCRFILE]; *//* in core version of screen file */

/* unsigned char __far *scrfile; */	/* new approach using malloc */
unsigned char __far *scrfile;		/* new approach using malloc */

unsigned int scrindex;

/* min and max FOND ID allowed for Latin fonts */

/* Old Font Families in range 2 through 255 */
/* Reserved for resolving conflicts is range 256 through 1023 */
/* Non-commercial and PD is supposed to be range 1024 through 3071 */
/* Commercial font manufacturer's in range 3072 through 15999 */

unsigned int FONDMinID = 3072;		/* Inside MacIntosh Volume VI - pg 13-8 */
unsigned int FONDMaxID = 16000;		/* Inside MacIntosh Volume VI - pg 13-8 */

/* min and max NFNT ID allowed */	/* no constraint - only refer via FAT */

unsigned int NFNTMinID = 1024;		/* actually 128 */
/* unsigned int NFNTMaxID = 32767; */
unsigned int NFNTMaxID = 32000;		/* for bug old version of Suitcase II */

unsigned int FONDResID = 0;			/* FOND Resource ID */

unsigned int FONDResInit = 0;		/* FOND Resource ID initial value */

/* later need new numbers for every font */

/* unsigned int NFNTResID = 0; */		/* NFNT Resource ID */

/* int ptsize = 1; */					/* size of bogus bitmap fonts */

int ptsize = 4;							/* size of bogus bitmap fonts */

char encodingvecname[MAXENCODING]="";	/* first line encoding vector file */

char *vectorpath = "";	/* directory for encoding vectors */

/* char programpath[PATHLEN_MAX] = "c:\\dvipsone"; */
char programpath[FILENAME_MAX] = "c:\\yandy\\dvipsone";

char *defaultscrvector="none"; 

char *defaultpfbvector="none"; 

char *scrvector="";			/* encoding desired in screen font */

char *pfbvector="";			/* encoding assumed in outline font */

int pass = 0;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* 39	quotesingle 	% quoteright in StandardEncoding */
/* 96	grave			% quoteleft in StandardEncoding */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* 20 `math' characters may be borrowed from Symbol font */ 

/*	161	degree		 176	400	*/	/* in Adobe text fonts */
/*	173	notequal	 185	549	*/
/*	176	infinity	 165	713	*/
/*	177	plusminus	 177	549	*/	/* in Adobe text fonts */
/*	178	lessequal	 163	549	*/
/*	179	greaterequal 179	549	*/
/*	181	mu			 77		576	*/		/* in Adobe text fonts */
/*	182	partialdiff	 182	494	*/
/*	183	summation	 229	713	*/
/*	184	product		 213	823	*/
/*	185	pi			 112	549	*/
/*	186	integral	 242	274	*/
/*	189	Omega		 87		768	*/
/*	194	logicalnot	 216	713	*/	/* in Adobe text fonts */
/*	195	radical		 214	549	*/
/*	197	approxequal	 187	549	*/
/*	198	Delta		 68		612	*/
/*	214	divide		 184	549	*/	/* in Adobe text fonts */
/*	215	lozenge		 224	494	*/
/*	218	fraction	 164	167	*/	/* in Adobe text fonts */
/*	240	apple		 240	790	*/

int mathchar[] = {					/* character codes in `standard roman' */
161, 173, 176, 177, 178, 179, 181, 182, 
183, 184, 185, 186, 189, 194, 195, 197, 
198, 214, 215, 218, 240, 0
};

int symwidth[] = {					/* char widths in `Symbol' font - fixed */
400, 549, 713, 549, 549, 549, 576, 494, 
713, 823, 549, 274, 768, 713, 549, 549, 
612, 549, 494, 167, 790, 0
};

/* Many Adobe fonts nowadays do have the following six actually: */
/* degree, plusminus, mu, logicalnot, divide, and fraction */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *programversion =
/* "AFMtoSCR conversion utility version 1.2"; */
/* "AFMtoSCR conversion utility version 1.3"; */
/* "AFMtoSCR conversion utility version 1.4"; */
/* "AFMtoSCR conversion utility version 1.5"; */
/* "AFMtoSCR conversion utility version 1.6"; */
/* "AFMtoSCR conversion utility version 1.7"; */
/* "AFMtoSCR conversion utility version 1.8"; */	/* 95/Oct/25 */ 
/* "AFMtoSCR conversion utility version 1.9"; */  /* 97/Aug/25 */ 
/* "AFMtoSCR conversion utility version 1.9.1";	*/ /* 97/Aug/25 */
#ifdef _WIN32
"AFMtoSCR conversion utility (32) version 2.0.1";	/* 99/Sep/17 */
#else
"AFMtoSCR conversion utility version 2.0";	/* 98/Jun/15 */
#endif

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\
Copyright (C) 1992--1999  Y&Y, Inc.  http://www.YandY.com\
";

/* Copyright (C) 1992--1997  Y&Y. All rights reserved. http://www.YandY.com\ */

/* Copyright (C) 1992--1995  Y&Y. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 13986445 */
/* #define COPYHASH 11210700 */
/* #define COPYHASH  8634843 */
/* #define COPYHASH 3471185 */
/* #define COPYHASH 11884568 */
/* #define COPYHASH 16753036 */
#define COPYHASH 10156513

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

int wantcpyrght = 1;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following flags set from command line */

int verboseflag = 0;		/* lots of output */
int traceflag = 0;
int detailflag = 0;
int lowercaseflag = 0;		/* force ResName to be lower case */
int remapflag = 0;			/* add CM remap */
int spacifyflag = 0;		/* switch `suppress' and `space' */
int texturify = 0;			/* make screen font coding switch 32 and 128 */
int stripcontrol = 0;		/* strip out characters from 0 - 31 */
int forceresname = 0;		/* User specified FOND ResName */
int seenmacname = 0;		/* after seen MacIntoshName in AFM file */
int seenwindowname = 0;		/* after seen MS-WindowsName in AFM file */
int usemacencoding = 1;		/* use Mac encoding */
int reencodeflag = 0;		/* non-zero => reencode font */
int wantbboxes = 0;			/* write bbox info - triggered by FontBBox */
int forceregular = 0;		/* force single AFM style to be regular */
int fillunknownstyles = 0;	/* fill empties in first four entries style map */
int fillstylemap = 0;		/* replicate first four entries in style map */
int suppressencoding = 0;	/* do not write out encoding vector */
int outlinediffer = 0;		/* show only differences from outline encoding */
int outlineinit = 0;		/* initial value of outlinediffer */
int nohyphen = 0;			/* now allow hyphen in font base name */
							/* default changed 1994/May/27 */
int acceptmod = 0;			/* allow modifier in BaseName 1995/Oct/19 */
int lowerdef = 1;			/* lower case is not upper case ! */
int flushzeros = 1;			/* suppress zero size kerns */
int ForceNonMac = 0;		/* force non-Mac encoding 95/Oct/24 */
int wildcards = 0;			/* treat AFM files not as styles but separate */
							/* batch mode making of independent *.scr files */
int descendflag = 0;		/* 1993/July/16 */

int mindescend=0;		/* lowest descender allowed (for ATM clipping) */

int suppressitalic = 0;		/* suppress `italic' style even ItalicAngle != 0 */
int suppressbold = 0;		/* suppress `bold' style even if Weight > 400 */

int FullRange = 0;			/* non-zero charmin = 0 charmax = 255 */
/* int FullRange = 1; */	/* non-zero charmin = 0 charmax = 255 95/Oct/25 */
/* int forcesymbol = 0; */	/* Force widths for math chars from symbol */
int borrowsymbol = 1;		/* Apple LW borrows math chars from symbol */
int onesizefitsall = 0;		/* use only one screen font for family of styles */
int fixposition = 1;		/* fixed desktop position */
int usenumerics = 0;		/* in writing scr font, use numeric encoding */

/* following are permanently set switches (compile time variables) */

int const altNFNTend = 1;		/* alternate end to NFNT width table 95/Oct/24 */
int const tryunderscore = 1;	/* non-zero => try underscore in font file name */
int const uppercase = 0;		/* force MacBinary file name to upper case  */
int const usefontname = 1;		/* Use FontName for FOND ResName (not contracted) */
int const stripbolditalic = 0;	/* Strip Bold and Italic off Mac font name */
int const compactflag = 1;		/* write compact `bitmap' if non-zero */
int const spacehack = 1;		/* force space into code 160 if possible */
int const addcharwidths = 0;	/* add optional NFNT char width table */
int const italicbeforebold = 1;	/* put bold style before italic style */
int const adjustbitwidth = 1;	/* compute screen font character widths */
int const adjustdescender = 0;	/* use 750 and yll for ascender/descender */

int scaletrouble = 0;		/* How many times exceeded safe dimensions */

/* struct stat filestat; */	/* file status info - for modification date */
struct _stat filestat;		/* file status info - for modification date */

/* this date nonsense: should really use `timediffer' and reference */

unsigned long yearsoff = 86400 * 1461;		/* offset 1904 - 1900 */
/* unsigned long yearsoff = 86400 * 24107;	*/	/* offset 1970 - 1904 */
/* long hoursoff = 3600 * 8; */		/* GMT - EZT ??? */
unsigned long hoursoff = 3600 * 9;			/* GMT - EZT ??? */

/* char *ResName=""; */				/* user specified ResName */

char ResName[FONTNAME_MAX];			/* user specified ResName */

char scrfilename[FONTNAME_MAX];		/* file name to use on Mac => header */

char macbinaryname[FONTNAME_MAX];	/* file name to use on Mac => header */

char filecreator[4+1] = "DMOV";			/* standard for suitcase file ... */
char filetype[4+1]    = "FFIL";			/* standard for font suitcase file */

/* 32 => has BNDL ?		1 => has been INITed ? */

unsigned char fileflags = 0;			/* was 1 */

unsigned char protectedflags = 0;

int desktopv=-1;		/* vertical position of icon on desktop */
int desktoph=-1;		/* horizontal position of icon on desktop */
int desktopd=0;			/* depth (?) position of icon on desktop */

unsigned long dataforklength = 0;			/* no data fork, so zero */
unsigned long resourceforklength;			/* determined later */

unsigned long creationdate=0;				/* seconds since 1904 Jan 1 */
unsigned long modificationdate=0;			/* seconds since 1904 Jan 1 */

unsigned int resourcefileattributes = 0;		/* resource file attributes */

unsigned int NFNTattrib=0;		/* 32 ? */
unsigned int FONDattrib=0;		/* 32 ? */

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
unsigned long typeliststart=0;			/* pointer to start of types */
unsigned long nameliststart=0;			/* pointer to start of names */

unsigned int nextresource;					/* pointer to next available */

unsigned long resourcedata[MAXRESOURCES];	/* pointer - resource data item */
											/* relative begin resource data */

char resourcenames[MAXRESOURCES][4];

/*	resourcedata[nextresource++] = resourcepointer; */

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

void forceupper(char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}


void forcelower(char *s, char *t) { /* convert to lower case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'A' && c <= 'Z') *s++ = (char) (c + 'a' - 'A');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *toolarge="SCR file is too large\n";

void uwriteone(unsigned int c) {
/*	if (scrindex >= MAXSCRFILE) { */		/* 1993/Nov/9 */
	if (scrindex >= maxscrfile) {		/* 1994/June/27 */
		fprintf(errout, toolarge);
		exit(1);
	}
	scrfile[scrindex++] = (unsigned char) c;
}

void uwritetwo(unsigned int n) {
	unsigned char c, d;
/*	if (scrindex + 1 >= MAXSCRFILE) { */		/* 1993/Nov/9 */
	if (scrindex + 1 >= maxscrfile) {		/* 1994/June/27 */
		fprintf(errout, toolarge);
		exit(1);
	}
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
/*	putc(c, output); putc(d, output); */
	scrfile[scrindex++] = c; scrfile[scrindex++] = d;
}

void uwritethree(unsigned long n) {
	unsigned char c, d, e;

/*	if (scrindex + 2 >= MAXSCRFILE) { */		/* 1993/Nov/9 */
	if (scrindex + 2 >= maxscrfile) {		/* 1994/June/27 */
		fprintf(errout, toolarge);
		exit(1);
	}
	e = (unsigned char) (n & 255);	n = n >> 8;
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
/*	putc(c, output); putc(d, output); putc(e, output); */
	scrfile[scrindex++] = c; scrfile[scrindex++] = d; 
	scrfile[scrindex++] = e;
}

void uwritefour(unsigned long n) {
	unsigned char c, d, e, f;

/*	if (scrindex + 3 >= MAXSCRFILE) { */		/* 1993/Nov/9 */
	if (scrindex + 3 >= maxscrfile) {		/* 1994/June/27 */
		fprintf(errout, toolarge);
		exit(1);
	}
	f = (unsigned char) (n & 255);	n = n >> 8;
	e = (unsigned char) (n & 255);	n = n >> 8;
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
/*	putc(c, output); putc(d, output); putc(e, output); putc(f, output); */
	scrfile[scrindex++] = c; scrfile[scrindex++] = d; 
	scrfile[scrindex++] = e; scrfile[scrindex++] = f;
}

void swritetwo(int n) {
	unsigned char c, d;
/*	if (scrindex + 1 >= MAXSCRFILE) { */		/* 1993/Nov/9 */
	if (scrindex + 1 >= maxscrfile) {		/* 1994/June/27 */
		fprintf(errout, toolarge);
		exit(1);
	}
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
/*	putc(c, output); putc(d, output); */
	scrfile[scrindex++] = c; scrfile[scrindex++] = d;
}

void writestring(char *s, unsigned int n) {
	unsigned int k;

/*	if (scrindex + n >= MAXSCRFILE) { */		/* 1993/Nov/9 */
	if (scrindex + n >= maxscrfile) {		/* 1994/June/27 */
		fprintf(errout, toolarge);
		exit(1);
	}
	for (k = 0; k < n; k++) scrfile[scrindex++] = (unsigned char) *s++;
/*	if (scrindex >= MAXSCRFILE) {
		fprintf(errout, toolarge);
		exit(69);
	} */
}

void writepascalstring(char *s) {
	unsigned int k, n = strlen(s);

/*	if (scrindex + n >= MAXSCRFILE) { */
	if (scrindex + n >= maxscrfile) {	/* 1994/June/27 */
		fprintf(errout, toolarge);
		exit(1);
	}
	scrfile[scrindex++] = (unsigned char) n;
	for (k = 0; k < n; k++) scrfile[scrindex++] = (unsigned char) *s++; 
/*	if (scrindex >= MAXSCRFILE) {
		fprintf(errout, toolarge);
		exit(69);
	} */
}

void writezeros(unsigned int n) {
	unsigned int k;

/*	if (scrindex + n >= MAXSCRFILE) { */
	if (scrindex + n >= maxscrfile) {		/* 1994/June/27 */
		fprintf(errout, toolarge);
		exit(1);
	}
	for (k = 0; k < n; k++) scrfile[scrindex++] = 0;
/*	if (scrindex >= MAXSCRFILE) {
		fprintf(errout, toolarge);
		exit(69);
	} */
}

void writeresourceheader(void) {
	uwritefour(ResDatOff);		/* offset to resource data */
	uwritefour(ResMapOff);		/* offset to resource map - later */
	uwritefour(ResDatLen);		/* length of resource data - later */
	uwritefour(ResMapLen);		/* length of resource map - later */
}

void writeuserdefined(void) {
	writezeros(128);
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
/*	use the data from the first AFM file only */
	if (creationdate == 0 && modificationdate == 0) {
/*		creationdate = modtime + yearsoff - hoursoff; */
		creationdate = modtime - yearsoff - hoursoff;
/*		modificationdate = modtime + yearsoff - hoursoff; */
		modificationdate = modtime - yearsoff - hoursoff;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Font Type bit 0 (1) - has image height table */
/* Font Type bit 1 (2) - has glyph width table */
/* Font Type bits 2-3  - bit depth of color */
/* Font Type bit 7     - has color table 'fctb' */
/* Font Type bit 8     - synthetic font */
/* Font Type bit 9     - uses colors other than black - font for color Mac only */
/* Font Type bit 12 (0x1000) - reserved should be set to 1 */
/* Font Type bit 13 (0x2000) - fixed width font */
/* Font Type bit 14 (0x4000) - do not "expand" font to screen bit depth */
/* Font Type bit 15 (0x8000) - reserved - should be zero (what?) */

/* Font Type 9000 - proportional font */
/* Font Type B000 - fixed width font */

unsigned int NFNTType = 0X9000;			/* proportional */
/* unsigned int NFNTType = 0XF000; */	/* fixed width */

/* possibly adapt to what is in AFM file ? */ /* make last 251 rather 255 ? */

/* following is common to all styles - appears once */

/* needs to be constructed from ascender[], descender[], widthmax[] */

unsigned int FONDFirst = 0, FONDLast = 255;		
int FONDAscender, FONDDescender;
int FONDLeading = 83;
int FONDWidthmax;

char EncodingScheme[FONTNAME_MAX];	/* EncodingScheme from AFM file */

unsigned int NFNTFirst = 0, NFNTLast = 255;
unsigned int NFNTWidMax=4, NFNTKernMax=0;		/* replaced later ... */
unsigned int NFNTAscent = 3, NFNTDescent = 1;	/* replaced later ... */
/* unsigned int NFNTAscent = 1, NFNTDescent = 0; */	/* experiment ... */
/* unsigned int NFNTAscent = 8, NFNTDescent = 2; */
unsigned int NFNTLeading = 0;
unsigned int frectwidth=4, frectheight=10;
unsigned int rowwords;
unsigned int owtloc;

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* writes a bit in row of bytes that start at rowstart */

void writebit(unsigned int rowstart, 
		unsigned int bitindex, unsigned int bit) {
	unsigned int k, byteindex, bitoff;
	unsigned int byte, mask;

	byteindex = bitindex / 8;			/* which byte ? */
	bitoff = bitindex - byteindex * 8;	/* which bit in byte ? */
	mask = 128 >> bitoff;				/* mask for this bit */
	k = rowstart + byteindex;			/* index into file */
	byte = scrfile[k];					/* the byter found there */
	if (bit == 0) byte = byte & ~ mask;	/* wipe out old bit */
	else byte = byte | mask;			/* insert new bit */
	scrfile[k] = (unsigned char) byte;
}

/* writes several bits in row of bytes that start at rowstart */

void writebits(unsigned int rowstart, 
	unsigned int bitindex, unsigned int nbits, unsigned int bits) {
	unsigned int k, bit;

	bits = bits << (8 - nbits);		/* left align bits in byte */
	for (k = 0; k < nbits; k++) {
		if ((bits & 128) != 0) bit = 1;	else bit = 0;
		writebit(rowstart, bitindex + k, bit);
		bits = bits << 1;
	}
}

// use 4 x 10 bit patterns for characters 
// or just a single bit - could rewrite for efficiency 

//   Bit image table. The bit image of the glyphs in the font.
//   The glyph images of every defined glyph in the font are
//   placed sequentially in order of increasing ASCII code. The
//   bit image is one pixel image with no undefined stretches
//   that has a height given by the value of the font rectangle
//   element and a width given by the value of the bit image
//   row width element. The image is padded at the end with
//   extra pixels to make its length a multiple of 16. 

void writebitimage(unsigned int bitstart, 
		unsigned int height, unsigned int width, unsigned int words) {
	unsigned int k;
	unsigned int m;
	unsigned int bits;

	if (compactflag) {			/*	height = 1  */
		for (m = NFNTFirst; m <= NFNTLast; m++) {
/*			writebits(bitstart, m, 1, (m & 1)); */	/* even/odd */
			writebits(bitstart, m, 1, 1);			/* always on */
		}
	}
	else {					/* height = 10 */
		for (k = 0; k < height; k++) {
			for (m = NFNTFirst; m <= NFNTLast; m++) {
				if (k == 0 || k == height - 1) bits = 0;
				else if (((m << (k-1)) & 128) != 0) bits = 2; else bits = 5;
				writebits(bitstart + 2 * words * k, m * width, width, bits);
			}
		}
	}
}

//   Bitmap location table. For every glyph in the font, this
//   table contains a word that specifies the bit offset to the
//   location of the bitmap for that glyph in the bit image
//   table. If a glyph is missing from the font, its entry
//   contains the same value for its location as the entry for
//   the next glyph. The missing glyph is the last glyph of the
//   bit image for that font. The last word of the table
//   contains the offset to one bit beyond the end of the bit
//   image. You can determine the image width of each glyph
//   from the bitmap location table by subtracting the bit
//   offset to that glyph from the bit offset to the next glyph
//   in the table. 

void writelocation(void) {		/* create location table --- N+3 entries*/
	unsigned int k, index = 0, width;

	if (compactflag) width = 1;
	else width = 4;
	for (k = NFNTFirst; k <= NFNTLast + 2; k++) {
		uwritetwo(index);
/*		don't increment if missing character ? */
		index += width;
	}
/*	uwritetwo(0); 	uwritetwo(0); */
}

//   Width/offset table. For every glyph in the font, this table
//   contains a word with 
//   the glyph offset in the high-order byte and the glyph's
//   width, in integer form, in the low-order byte. The value of
//   the offset, when added to the maximum kerning 
//   value for the font, determines the horizontal distance
//   from the glyph origin to the left edge of the bit image of
//   the glyph, in pixels. If this sum is negative, the glyph
//   origin 
//   is to the right of the glyph image's left edge, meaning
//   the glyph kerns to the left. 
//   If the sum is positive, the origin is to the left of the
//   image's left edge. If the sum equals zero, the glyph
//   origin corresponds with the left edge of the bit image.
//   Missing glyphs are represented by a word value of -1. The
//   last word of this table is also -1, representing the end. 

void writeoffsetwidth(void) {	/* create offset/width table */
	unsigned int k, offset = 0, charwidth;
	int m;
	int style=0;

/*	if (compactflag) charwidth = 1;	else charwidth = 4; */
	if (adjustbitwidth) {
		for (m = 0; m < 4; m++) {
			style = stylepointer[m];
			if (style >= 0) break;
		}
	}

	for (k = NFNTFirst; k <= NFNTLast; k++) {
/*	Try and come up with reasonable bitmap character widths - coarse round */
		if (adjustbitwidth) {
			charwidth = 
/*				(int) (((long) widths[style][k] * ptsize + 499) / 4096);  */
				(int) (((long) widths[style][k] * ptsize + 2047) / 4096); 
			if (charwidth < 1) charwidth = 1;
			else if (charwidth > 8) charwidth = 8;
		}
		else if (compactflag) charwidth = 1;	
		else charwidth = 4;
//		offset = 0;
//		or, balance sidebearings maybe :
		offset = (charwidth - 1) / 2;
		if (offset < 0) offset = 0;
//		Make both offset and width 255 for missing character ? 
		if (strcmp(screncoding[k], "") == 0) {		/* is this a good idea? */
			offset = 255; charwidth = 255;
		}
		if (FullRange && k < 32) {				/* 1995/Oct/24 */
			if (strcmp(screncoding[k], "") != 0) {
				fprintf(errout, "Conflict %d %s Force Origin\n",
						k, screncoding[k]);
			}
			offset = 0;
			if (k == 0) charwidth = 0;				/* null character */
			else if (k == 9) {						/* treat tab as space */
				charwidth =
				   (int) (((long) widths[style][32] * ptsize + 2047) / 4096); 
			}
			else if (k == 13) charwidth = 0;		/* return is null */
/*			else { offset = 255; charwidth = 255; } */ /* default already */
		}
		uwriteone(offset);	uwriteone(charwidth); 
/*		Or get width from difference of locations ? */
	}
	if (altNFNTend) {				/*  95/Oct/24 - now the default */
/*	k 256 location:  224 bit-width:   1  offset:	 0 width:	  2 */
/*	k 257 location:  225 bit-width: -225 offset:   255 width:   255 */
		if (adjustbitwidth) {
			charwidth = 
				(int) (((long) defaultwidth * ptsize + 2047) / 4096); 
			if (charwidth < 1) charwidth = 1;
			else if (charwidth > 8) charwidth = 8;
		}
		else if (compactflag) charwidth = 1;	
		else charwidth = 4;
		offset = 0;
		uwriteone(offset);
		uwriteone(charwidth);
		offset = 255; charwidth = 255;
		uwriteone(offset);
		uwriteone(charwidth);
	}
	else {										/* the old way */
/*	k 256 location:  224 bit-width:   1  offset:  255 width:   255 */
/*	k 257 location:  225 bit-width: -225 offset:    0 width:   0 */
/*		uwriteone(255); uwriteone(255); */		/* was 1992/June/7 */
		offset = 255; charwidth = 255;
		uwriteone(offset);	uwriteone(charwidth); 
/*		uwritetwo(0); 	 */						/* was 1992/June/7 */
		offset = 0; charwidth = 0;
		uwriteone(offset);	uwriteone(charwidth);
	}
}

//   (Optional) glyph-width table. For every glyph in the font, this table
//   contains a word that specifies the glyph's fixed-point
//   glyph width at the given point size and font style, in
//   pixels. The Font Manager gives precedence to the values
//   in this table over those in the font family glyph-width
//   table. There is an unsigned integer in the high-order byte
//   and a fractional part in the low-order byte. This table is
//   optional. 

void writeoptionalwidth(void) {	/* create fractional char width table */
	unsigned int k, charwidth;
//	unsigned int offset = 0;
	int m;
	int style;

	for (m = 0; m < 4; m++) {
		style = stylepointer[m];
		if (style >= 0) break;
	}

	for (k = NFNTFirst; k <= NFNTLast; k++) {
		charwidth = 
/*		(int) (((long) widths[style][k] * 256 * ptsize + 500) / 1000); */
		(int) (((long) widths[style][k] * ptsize + 7) / 16);
		if (charwidth < 1) charwidth = 1;  /* avoid problem with zero width */
		if (strcmp(screncoding[k], "") == 0) charwidth = 0; /* ??? */
		uwritetwo(charwidth);
	}
	uwritetwo(0); 	uwritetwo(0);
}

//   The negated descent value. If this font has very large tables
//   and this value is positive, this value is the high word of
//   the offset to the width/offset table. For more
//   information, see "The Offset to the Width/Offset Table"
//   on page 4-67. If this value is negative, it is the negative
//   of the descent and is not used by the Font Manager. This
//   value is represented by the nDescent field in the FontRec
//   data type. 

/* optional width character-width table immediately follows offset/width */
/* contains actual width in Mac units (Adobe units * 4096 / 1000) NO */

/* optional image-height table follows that */
/* contains offset of first non-white row (high byte) | number or rows */

unsigned int numberofnfnts=0, numberoffonds=0;

void createnfnt(int pass) {
	unsigned int nfntstart, nfntend;
	unsigned int tablesize;
	unsigned int owtptrloc;

	if (compactflag) {
		frectwidth=1; frectheight=1;
	}
	else {
		frectwidth=4; frectheight=10;
	}

/*	NFNTWidMax=ptsize * 3; */		/* Max Width of NFNT screen font */
	NFNTWidMax = (int) (((long) FONDWidthmax * ptsize + 2047) / 4096); 
	if (NFNTWidMax < (unsigned int) (ptsize * 2)) 
		NFNTWidMax=ptsize * 2;	/* just in case */	/* or use FONDxur */

/*	NFNTKernMax=0 */
	if (FONDxll >= 0) NFNTKernMax = 0;
	else NFNTKernMax = (int) ((-(long) FONDxll * ptsize + 2047) / 4096); 

	resourcedata[nextresource] = resourcepointer;
/*	resourcedata[nextresource] = scrindex; *//* remember start of resource*/
	strncpy(resourcenames[nextresource], "NFNT", 4);
	nextresource++;

/*	nfntstart = scrindex; */
	uwritefour(0L);				/* length of resource - fix up later */
	nfntstart = scrindex;
	if (verboseflag && pass == 0) printf("NFNTResStart %u\n", nfntstart);
	if (addcharwidths) NFNTType = NFNTType | 2; /* has width-table */

	uwritetwo(NFNTType);
	uwritetwo(NFNTFirst);
	uwritetwo(NFNTLast);
	uwritetwo(NFNTWidMax);
	swritetwo(- (int) NFNTKernMax);		/* 1992/Oct/6 */
/*	swritetwo(-NFNTDescent); */	/*	swritetwo(0); */
/*	negated descent value */
	if (NFNTDescent > 0)
		swritetwo(- (int) NFNTDescent);		/* 1992/Oct/6 */
/*	avoid problem of positive value - high word of offset to width/offset table */
	else {
		printf("NFNT Descent %d < 0\n", NFNTDescent);
		swritetwo(0);						/* 1999/Sep/13 */
	}
	uwritetwo(frectwidth);
	uwritetwo(frectheight);

	tablesize = (NFNTLast - NFNTFirst + 3);	/* size of each table in words */
/*	includes unknown glyph and extra dummy entry at end of table */
	rowwords = (frectwidth * tablesize + 15) / 16;	/* imag row width words */
	owtptrloc = scrindex;							/* pointer to owtloc */
	owtloc = 5 + rowwords * frectheight + tablesize;	/* from here, in words */

/* 	NFNTAscent = 3;	NFNTDescent = 1; */
/*	an experiment ... */ 
/*	if (FONDAscender + FONDDescender > 0) {	
		NFNTDescent = 1; NFNTAscent = ptsize - NFNTDescent;
	}
	else {	
		NFNTAscent = 1; NFNTDescent = ptsize - NFNTAscent;
	} */
	if (FONDAscender > 0)
		NFNTAscent = (int) (((long) FONDAscender * ptsize + 2047) / 4096); 
	else NFNTAscent = 1;
	if (FONDDescender < 0)
		NFNTDescent = (int) ((- (long) FONDDescender * ptsize + 2047) / 4096);
	else NFNTDescent = 1;
	if (NFNTAscent + NFNTDescent < (unsigned int) ptsize)
		NFNTLeading = ptsize - (NFNTAscent + NFNTDescent);
	else NFNTLeading = 0;		/* or use FONTyll and FONDyur ??? */

	uwritetwo(owtloc);
	uwritetwo(NFNTAscent);
	uwritetwo(NFNTDescent);
	uwritetwo(NFNTLeading);
	uwritetwo(rowwords);

	writebitimage(scrindex, frectheight, frectwidth, rowwords);
	scrindex = scrindex + 2 * rowwords * frectheight;
	writelocation();

/*	now fix up or check owtloc */
	if (owtloc * 2 != (scrindex - owtptrloc)) 
		fprintf(errout, 
			"2 * OWTLoc %u <> %u\n", owtloc * 2, scrindex - owtptrloc);
	writeoffsetwidth();

	if (addcharwidths) writeoptionalwidth();

	nfntend = scrindex;								// remember where we are	
	if (verboseflag && pass == 0) printf("NFNTResEnd   %u\n", nfntend);
//  now fix up resource length 
	resourcepointer += nfntend - (nfntstart - 4);	// total length of resource?
	resourcedata[nextresource] = resourcepointer;	// start of next resource 
	scrindex = nfntstart - 4;						// where to put resource length
// length of resource itself 
//	uwritefour((unsigned long) (nfntend - nfntstart - 4));
	uwritefour((unsigned long) (nfntend - nfntstart)); // 1999/Sep/17 ?
	scrindex = nfntend;								// reset to where it was
	if (pass == 0) numberofnfnts++;
}

int macscale(int x) {  /* convert from Adobe 1000 per em to Mac 4096 per em */
	if (x < -4000 || x > 8000) scaletrouble++;
	if (x < 0) return (- macscale(- x));
	else if (x == 0) return 0;
	else return (int) (((long) x * 4096 + 499) / 1000);
}

int unmacscale(int x) {  
	if (x < 0) return (- unmacscale(- x));
	else if (x == 0) return 0;
	else return (int) (((long) x * 1000 + 2047) / 4096);
}

/* style code: 1 - bold, 2 - italic, 4 - underline, 8 - outline */
/* 16 - shadow, 32 - condensed, 64 - extended, 128 - reserved */

void writewidths(int pass) {
	unsigned int numberofwidths = nstyles;
	int style, m;
	unsigned int k, count = 0;
	int mappedunknown;
	
	mappedunknown = macscale(UNKNOWN);

	uwritetwo(numberofwidths - 1);
/*	for (m = 0; m < numberofwidths; m++) { */
	for (m = 0; m < 4; m++) {
		if (italicbeforebold != 0 && m > 0 && m < 3)
			style = stylepointer[3 - m];
		else style = stylepointer[m];
		if (style < 0) 	continue;					/* ??? */
		uwritetwo(stylecode[style]); 
		for (k = FONDFirst; k <= FONDLast; k++) {
			if (widths[style][k] == mappedunknown) swritetwo(0);
			else swritetwo(widths[style][k]);
		}
		swritetwo(defaultwidth);
		swritetwo(0);
		count++;
	}
	if (pass == 1 && count != numberofwidths) 
		fprintf(errout, 
			"ERROR: Inconsistent number of width tables %d versus %d\n",
				count, numberofwidths);
}

void writekerns(int pass) {
	unsigned int numberofkerns = nstyles;
	int style, m;
	unsigned int k, n, count = 0;
	struct kernpair __far *kernstyle;		/* kern pairs for this style */
	
/*	if (nkerns == 0) {
		numberofkerns = 0;	return;
	} */
	
	needkerns = 0;
	for (style = 0; style < nstyles; style++) {
		if (nkerns[style] > 0) {
			needkerns++; break;
		}
	}

	if (needkerns == 0) return;		/* no kerning pairs ??? */

	uwritetwo(numberofkerns - 1);
/*	for (m = 0; m < numberofkerns; m++) { */
	for (m = 0; m < 4; m++) {
		if (italicbeforebold != 0 && m > 0 && m < 3)
			style = stylepointer[3 - m];
		else style = stylepointer[m];
		if (style < 0) continue;		/* ??? */
		kernstyle = kernpairs[style];	/* get pointer to appropriate array */
		n = nkerns[style];
/*		possibly omit this if n is zero ??? */
		uwritetwo(stylecode[style]);
		uwritetwo(n);
		if (n > 0 && kernstyle == NULL) {
			fprintf(errout, "ERROR: Bad Kern Table Pointer\n");
		}
		for (k = 0; k < n; k++) {
/*			uwriteone(kerna[style][k]); */
/*			uwriteone(kernpairs[style][k].a); */
			uwriteone(kernstyle[k].a);
/*			uwriteone(kernb[style][k]); */
/*			uwriteone(kernpairs[style][k].b); */
			uwriteone(kernstyle[k].b);
/*			swritetwo(kerns[style][k]); */
/*			swritetwo(kernpairs[style][k].kern); */
			swritetwo(kernstyle[k].kern);
		}
		count++;
	}
	if (pass == 1 && count != numberofkerns) 
		fprintf(errout, 
			"ERROR: Inconsistent number of kern tables %d versus %d\n",
				count, numberofkerns);
}

/* NFNT entries should be sorted by size */

void writeassociation(int pass) {
	unsigned int numberofassoc;
	unsigned int fontsize = ptsize;
	unsigned int fontstyle;			
	unsigned int m, count = 0;

	if (onesizefitsall != 0) numberofassoc = 1;
	else numberofassoc = nstyles;
	
	if (traceflag != 0 && pass == 1) 
		printf("FAT has %d NFNT Associations\n", numberofassoc);
	uwritetwo(numberofassoc - 1);
/*	for (k = 0; k < numberofassoc; k++) { */
	for (m = 0; m < 4; m++) {
		if (italicbeforebold != 0 && m > 0 && m < 3)
			style = stylepointer[3 - m];
		else style = stylepointer[m];
		if (style < 0) continue;			/* ??? */
		fontstyle = stylecode[style];
		uwritetwo(fontsize);
		uwritetwo(fontstyle);		
		uwritetwo(NFNTResID[style]);	/* corresponding NFNTResID */
		if (traceflag != 0 && pass == 1)
			printf("NFNTResID %d\n", NFNTResID[style]);
		count++;
		if (onesizefitsall != 0) break;		/* only do one */
	}
	if (pass == 1 && count != numberofassoc)
		fprintf(errout, 
			"ERROR: Inconsistent number of NFNT associations %d versus %d\n",
				count, numberofassoc);
}

/* following needed unless Mac encoding is used */
/* make this relative to outline font encoding ??? */

/* This is shipped out in printer file to reencode the font */
/* hence it should be the difference between the desired encoding and */
/* that in the outline font file */ 

void writeencoding(int pass) {
	unsigned length=0, count=0;
	unsigned int k, l;

	if (traceflag != 0) {	/* debugging */
		printf("PFB encoding (writeencoding):\n");
		for (k = 0; k < MAXCHRS; k++) 
			if (strcmp(pfbencoding[k], "") != 0) 
				printf("%s ", pfbencoding[k]);
		putc('\n', stdout);
	}
/*	first count how many non blank are there total */
	for (k = 0; k < MAXCHRS; k++) 
		if (strcmp(screncoding[k], "") != 0) count++;
/*	then count how many are actually needed */
	if (outlinediffer != 0) {
		for (k = 0; k < MAXCHRS; k++) 
			if (strcmp(screncoding[k], "") != 0) 
				if (strcmp(screncoding[k], pfbencoding[k]) != 0) length++;
	}
	else {
		length = count;
/*		for (k = 0; k < MAXCHRS; k++) if (strcmp(screncoding[k], "") != 0)
			length++; */
	}
	if (length == 0 && pass == 1) printf("Zero length reencoding?\n");
	if (verboseflag != 0 && pass == 1)  {
		if (outlinediffer != 0)
			printf("Writing relative encoding with %d (out of %d) elements\n",
				length, count);
		else printf("Writing complete encoding with %d elements\n", length);
	}
	uwritetwo(length);
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(screncoding[k], "") != 0) {
			if (outlinediffer != 0) {	
				if (strcmp(screncoding[k], pfbencoding[k]) != 0) {
					uwriteone(k);	
					writepascalstring(screncoding[k]);
					if (pass == 1 && length < 12)
						printf("Difference: %d `%s' (not `%s')\n",
							   k, screncoding[k], pfbencoding[k]);
				}
			}
			else if (usenumerics != 0) {
				for (l = 0; l <= k; l++) { /* find lowest numbered copy */
					if (strcmp(screncoding[l], screncoding[k]) == 0) break;
				}
				uwriteone(k);
				sprintf(line, "a%d", l);
				writepascalstring(line);
			}
			else {					/* just write it ! */
				uwriteone(k);
				writepascalstring(screncoding[k]);
			}
		}
	}
}

/* Font Class - `Inside LaserWriter' page 29 */
/* 1    - font name needs `coordinating' */
/* 2    - MacIntosh reencoding scheme required (import from Symbol)	 <===== */
/* 4    - Font has Outline property by changing PaintType to 2 */
/* 8    - disallow outline simulation by smear and whiteout */
/* 16   - disallow embolding simulation by smear */
/* 32   - embolding is simulated by increasing point size */
/* 64   - disallow obliquing for italic */
/* 128  - disallow automatic simulation of condensed style */
/* 256  - disallow automatic simulation of extended/expanded style */
/* 512  - requires reencoding other than MacIntosh vector (has vector)	<===== */
/* 1024 - font family should have no additional interchar spacing */
/*		- remaining 5 bits are reserved */

/* MACENCODING and NONMACENCODING can't be both on at same time */

#define COORDINATE 1
#define MACENCODING 2
#define PAINTTYPE 4
#define NOOUTLINE 8
#define NOEMBOLDEN 16
#define BOLDENLARGE 32
#define NOOBLIQUE 64
#define NOCONDENSE 128
#define NOEXPAND 256
#define NONMACENCODE 512
#define NOINTERCHAR 1024

int LeadingFreeze = 0;			/* non-zero if Leading specified in AFM  */
int FONDClassFreeze = 0;		/* non-zero if FONDClass specified by user */

/* always need COORDINATE ? (certainly if reencoding to mac or other) */

unsigned int FONDClass = 
	PAINTTYPE | NOOUTLINE | NOEMBOLDEN | NOOBLIQUE;
/*		| NOCONDENSE | NOEXPAND; */	/* Used in CM, but not in Adobe fonts */

/* MACENCODING or NONMACENCODE added elsewhere */

unsigned long encodingoffset;

int ismodifier(char *str) {
	if (strcmp(str, "Black") == 0) return -1;
	if (strcmp(str, "Heavy") == 0) return -1;
	if (strcmp(str, "Bold") == 0) return -1;
	if (strcmp(str, "Demi") == 0) return -1;	
	if (strcmp(str, "Demibold") == 0) return -1; 
	if (strcmp(str, "Semibold") == 0) return -1;	
	if (strcmp(str, "Medium") == 0) return -1;	
	if (strcmp(str, "Regular") == 0) return -1;
	if (strcmp(str, "Normal") == 0) return -1;	
	if (strcmp(str, "Roman") == 0) return -1;	
	if (strcmp(str, "Book") == 0) return -1;		
	if (strcmp(str, "Light") == 0) return -1;
	if (strcmp(str, "Italic") == 0) return -1;
	if (strcmp(str, "Oblique") == 0) return -1;
	if (strcmp(str, "Kursiv") == 0) return -1;	
	return 0;
}

int charmin, charmax;

/* Combine accumulated info from several AFM files */

void GetCommonFONDInfo(int pass) {
	int k, l, m, j;
/*	int kd, md; */
	unsigned int n, i;
	int nsuffer=0;		/* used before initialized ? */
	int height, style, flag;
	int hyphenflag;								/* 1993/Jan/24 */
	
/*	Get extreme values of Ascender and Descender and Maxwidth */

	FONDAscender = 0;		/* Ascent */
	FONDDescender = 0;		/* Descent */
	FONDLeading = 0;		/* Leading */
	FONDWidthmax = 0;		/* MaxWidth */
	FONDxll = 1000; FONDyll = 1000; FONDxur = 0; FONDyur = 0;

	for (k = 0; k < nstyles; k++) {
		if (ascender[k] > FONDAscender) FONDAscender = ascender[k];
		if (descender[k] < FONDDescender) FONDDescender = descender[k];
		if (widthmax[k] > FONDWidthmax) FONDWidthmax = widthmax[k];
		if (xll[k] < FONDxll) FONDxll = xll[k];
		if (yll[k] < FONDyll) FONDyll = yll[k];	
		if (xur[k] > FONDxur) FONDxur = xur[k];
		if (yur[k] > FONDyur) FONDyur = yur[k];			
	}
	if (FONDyll < FONDDescender - 120 * 4) {
		fprintf(errout, 
			"WARNING: Descender (%d) much smaller than yll (%d)\n",
				unmacscale(FONDDescender), unmacscale(FONDyll));
/*		FONDDescender = FONDyll + 120 * 4; */ /* precaution */
		if (FONDDescender == 0) {			/* 1993/March/15 */
/*			FONDDescender = macscale(-250);	*/
			if (FONDyll < 0) {
				FONDDescender = FONDyll;	
				fprintf(errout, "Using %d for Descender\n", 
					unmacscale(FONDDescender));
			}
			else fprintf(errout, "FONDyll not < 0\n");	/* debug */
		}
		else fprintf(errout, "FONDDescender not == 0\n");	/* debug */
	}

	if (FONDyur > FONDAscender + 250 * 4) {
		fprintf(errout, 
			"WARNING: Ascender (%d) much smaller than yur (%d)\n",
				unmacscale(FONDAscender), unmacscale(FONDyur));
/*		FONDAscender = FONDyur - 250 * 4; */		/* precaution ??? */
		if (FONDAscender == 0) {
			FONDAscender = macscale(750); /* 1993/March/15 */
			fprintf(errout, "Using %d for Ascender\n",
				unmacscale(FONDAscender)); 
		}
		else fprintf(errout, "FONDAscender not == 0\n");	/* debug */
	}

	if (adjustdescender != 0) {				/* disabled default */
		if (FONDyll < 0)					/* 1993/Jan/20 */
			FONDDescender = FONDyll;		/* after all that ! */
		FONDAscender = macscale(750);		/* after all that ! */
	}

/* or use FONDyll for FONDDescender ??? or -250 ??? */
/* or use FONDxur for FONDWidthmax ??? */	

	height = FONDAscender - FONDDescender;
	if (LeadingFreeze == 0) {
/*		if (height < 1031) FONDLeading = 83; */		/* 1993/Jan/20 */
		if (height < macscale(1031)) FONDLeading = macscale(83); /* ? */
		else {
fprintf(errout, 
	"WARNING: character height (%d + %d) > 1000 may affect leading\n",
		unmacscale(FONDAscender), unmacscale(-FONDDescender));
			FONDLeading = 0;
		}
	}

	if (verboseflag != 0 && pass == 1) {
		printf("FONDAscender %d FONDDescender %d\n",
			unmacscale(FONDAscender), unmacscale(FONDDescender));
		printf("FONDLeading %d FONDWidthmax %d\n", 
			unmacscale(FONDLeading), unmacscale(FONDWidthmax));
	}

	allfixedpitch = isfixedpitch[0]; 
	for (k = 0; k < nstyles; k++) {
		if (allfixedpitch != isfixedpitch[k])
			fprintf(errout, "ERROR: some fonts fixed pitch, some not!\n");
	}

	strcpy(CommonFontName, FontName[0]); 
	strcpy(CommonMacName, MacName[0]); 
	for (k = 0; k < nstyles; k++) {
		if (boldflag[k] == 0 && italicflag[k] == 0) { /* REGULAR ? */
			strcpy(CommonFontName, FontName[k]);
			strcpy(CommonMacName, MacName[k]); 
		}
/*	regular, bold, italic, bolditalic */ /* compute stylecode for each */
		stylecode[k] = REGULAR;
		if (boldflag[k] != 0) stylecode[k] = stylecode[k] | BOLD;
		if (italicflag[k] != 0) stylecode[k] = stylecode[k] | ITALIC;	
		if (nstyles == 1 && forceregular != 0) stylecode[k] = REGULAR;
	}
	
/*	Attempt to extract common part of all FontNames - instead of above */
	if (nstyles > 1) {
		flag = 0;
		for (n = 1; n < 64; n++) {
			for (k = 1; k < nstyles; k++) {
				if (strncmp(FontName[0], FontName[k], n) != 0) {
					flag++; break;
				}
			}
			if (flag != 0) break;
		}
		n--;
		strncpy(CommonFontName, FontName[0], n);
		if (*(CommonFontName + n -1) == '-') *(CommonFontName + n -1) = '\0';
	}
	if (traceflag != 0 && pass == 1) 
		printf("Common Part of FontNames is `%s'\n", CommonFontName);
	
	if (traceflag != 0 && pass == 1) printf("Sorting Styles Now\n");

	if (pass == 1) {					/* checking for duplicates */
		for (k = 0; k < nstyles; k++) {
			for (l = 0; l < k; l++) {
				if (stylecode[k] == stylecode[l]) {
					fprintf(errout, 
						"ERROR: Style code %d occurs more than once ", 
							stylecode[k]);
						fprintf(errout,
							"(in %s and %s)\n", FontName[k], FontName[l]);
				}
			}
		}
	}
/*	major change in how to do this ... */
	for (k = 0; k < 4; k++) {
		stylepointer[k] = -1;			/* indicates this style is missing */
		for (m = 0; m < nstyles; m++) {
			if (stylecode[m] == k) stylepointer[k] = m;
		}
	}
	if (traceflag != 0) {
		printf("style pointers are %d %d %d %d\n", 
		  stylepointer[0], stylepointer[1], stylepointer[2], stylepointer[3]);
	}
	if (stylepointer[0] < 0) {
		fprintf(errout, "ERROR: Regular style missing");
		if (nstyles == 1) {
			fprintf(errout, " - maybe use `b' command line flag");
			for (m = 0; m < 4; m++) {	/* try and force it */
				if (stylepointer[m] != 0) {
					stylepointer[0] = stylepointer[m];	/* ??? */
/*					stylepointer[m] = -1; */			/* ??? */
					break;
				}
			}
		}
		putc('\n', errout);
	}

	if (verboseflag != 0 && pass == 1) {
/*		for (m = 0; m < nstyles; m++) { */
		for (m = 0; m < 4; m++) {
			if (italicbeforebold != 0 && m > 0 && m < 3)
				style = stylepointer[3 - m];
			else style = stylepointer[m];
			if (style < 0) continue;
			printf("Style: %d FontName: %s  (%d widths and %d kerns)\n",
				stylecode[style], FontName[style], 
					nwidths[style], nkerns[style]);
		}
	}

/*	if (nstyles > 1 && strcmp(BaseName[0], CommonFontName) != 0) { */
	if (strcmp(BaseName[0], CommonFontName) != 0) {
		if (verboseflag != 0) 
			printf("CommonFontName `%s' is not BaseName `%s'\n", 
				CommonFontName, BaseName[0]);
		for(;;) {
			flag = 0;
/* don't allow hyphen in common base name - unless specified in command */
/*			if (strcmp(SuffixString[0][0], "-") == 0) break; */
			if (strcmp(SuffixString[0][0], "-") == 0) {
				hyphenflag = 1;
				if (nohyphen != 0) break; 
			}
			else hyphenflag = 0;
/*			if (ismodifier(SuffixString[0][0]) != 0) break; */
			if (acceptmod == 0 &&					/* 1995/Oct/19 */
				ismodifier(SuffixString[0][0]) != 0) break;
			if (nextsuffix[0] == 0) break;
			if (hyphenflag != 0) {					/* 1993/Jan/24 */
				if (ismodifier(SuffixString[0][1]) != 0) break;
/* following removed 1994/May/20 */
/*				if (nextsuffix[1] == 0) break; */		/* what ? */
			}
/*			if (nstyles == 1) break; */				/* ??? */
			for (style = 1; style < nstyles; style++) {
				if (strcmp(SuffixString[0][0], SuffixString[style][0]) != 0) {
					flag++; break;
				}
				if (hyphenflag != 0) {
					if (strcmp(SuffixString[0][1], SuffixString[style][1]) != 0) {
						flag++; break;
					}
				}
			}
			if (flag != 0) break;
			if (verboseflag != 0) 
				printf("Absorbing `%s' into BaseName\n", SuffixString[0][0]);
			for (style = 0; style < nstyles; style++) {
				strcat(BaseName[style], SuffixString[style][0]);
				nsuffer = nextsuffix[style];
				for (m = 0; m < nsuffer - 1; m++) {
					strcpy(SuffixString[style][m], 
						SuffixString[style][m+1]);
				}
				nextsuffix[style]--;
			}
		}
		if (verboseflag != 0)
			printf("NOTE: Final BaseName: `%s'\n", BaseName[0]);
/*			if (strlen(BaseName[0]) > 22) {
				fprintf(errout, 
					"WARNING: Base FontName %s may be too long (%d > %d)\n",
						BaseName[0], strlen(BaseName[0]), 22);
			} */
	}

/*	make up common basename and suffix list - first for nstyles == 1 */
/*	strcpy(CommonBaseName, BaseName[0]); */
	for (m = 0; m < 4; m++) {				/* `regular' style */
		style = stylepointer[m];			/* if possible */
		if (style < 0) continue;
		if (pass == 1 && stylecode[style] != m) 
			fprintf(errout, 
				"ERROR: Inconsistent style table (g) %d versus %d\n",
					m, stylecode[style]);
		break;
	}
/*	style = stylepointer[0]; */				/* `regular' style */

	strcpy(CommonBaseName, BaseName[style]);
/*	try and use BOLDITALIC first ?  */
	for (m = 3; m >= 0; m--) {
		if (italicbeforebold != 0 && m > 0 && m < 3)
			style = stylepointer[3 - m];
		else style = stylepointer[m];
		if (style >= 0) {
			nsuffer = nextsuffix[style];
			break;
		}
	}
/*	simplify ??? this just counting down from 3 to 0 ... */	
	for (m = 0; m < nsuffer; m++) {
		flag = 0;
		for (j = 0; j < m; j++) {			/* check if there already */
			if (strcmp(CommonSuffixString[j], 
				SuffixString[style][m]) == 0) {
				flag++; break;				/* already exists - ignore */
			}
		}
		if (flag == 0) {
			strcpy(CommonSuffixString[nsuffixes++], SuffixString[style][m]);
			if (traceflag != 0) 
				printf("Initial Base Suffix: %s\n", SuffixString[style][m]);
		}
	}

	if (nstyles == 1) return;		/* that was easy ! */

	for (k = 0; k < nstyles; k++) {		/* check base name consistency */
		if (strcmp(BaseName[k], CommonBaseName) != 0)
			fprintf(errout, "WARNING: BaseName mismatch: %s %s\n",
				BaseName[k], CommonBaseName);
	}

	if (verboseflag != 0 && pass == 1)
		printf("BaseName: `%s' ", CommonBaseName);
	if (strlen(CommonBaseName) > 22) {
		fprintf(errout, 
			"WARNING: Common Base Font Name %s may be too long (%d > %d)\n",
				CommonBaseName, strlen(CommonBaseName), 22);
			}
	
/*	Now copy over all suffixes (without duplication) */
/*	nsuffixes = 0; */						/* don't undo the above */
/*	for (k = 0; k < nstyles; k++) { */		/* step through each style */
	for (m = 3; m >= 0; m--) {
		if (italicbeforebold != 0 && m > 0 && m < 3)
			style = stylepointer[3 - m];
		else style = stylepointer[m];
		if (style < 0) continue;
		nsuffer = nextsuffix[style];
		for (k = 0; k < nsuffer; k++) {		/* step through each suffix */
			flag = 0;
			for (i = 0; i < nsuffixes; i++) {	/* check if there already */
				if (strcmp(CommonSuffixString[i], 
						SuffixString[style][k]) == 0) {
					flag++; break;				/* already exists - ignore */
				}
			}
			if (flag == 0) {			/* not there yet, this is a new one */
				strcpy(CommonSuffixString[nsuffixes++],	
					SuffixString[style][k]);
			}
		}
	}
	if (traceflag != 0 && pass == 1) {
		for (i = 0; i < nsuffixes; i++)
			printf("Suffix %d: `%s' ", i, CommonSuffixString[i]);
		putc('\n', stdout);
	}
}

/* Font Class - `Inside LaserWriter' page 29 */
/* 1    - font name needs `coordinating' */
/* 2    - MacIntosh encoding scheme required				 <===== */
/* 4    - Font has Outline property by changing PaintType to 2 */
/* 8    - disallow outline simulation by smear and whiteout */
/* 16   - disallow embolding simulation by smear */
/* 32   - embolding is simulated by increasing point size */
/* 64   - disallow obliquing for italic */
/* 128  - disallow automatic simulation of condensed style */
/* 256  - disallow automatic simulation of expanded style */
/* 512  - requires reencoding other than MacIntosh vector	<===== */
/* 1024 - font family should have no additional interchar spacing */
/*		- remaining 5 bits are reserved */

#define COORDINATE 1
#define MACENCODING 2
#define PAINTTYPE 4
#define NOOUTLINE 8
#define NOEMBOLDEN 16
#define BOLDENLARGE 32
#define NOOBLIQUE 64
#define NOCONDENSE 128
#define NOEXPANDE 256
#define NONMACENCODE 512
#define NOINTERCHAR 1024

void explainclass(int na) {
	printf("Font Class %04X ", na);
	if ((na & COORDINATE) != 0) printf("coordinate ");
	if ((na & MACENCODING) != 0) printf("mac-encode ");
	if ((na & PAINTTYPE) != 0) printf("paint-2 ");
	if ((na & NOOUTLINE) != 0) printf("no-outline ");
	if ((na & NOEMBOLDEN) != 0) printf("no-embold ");
	if ((na & BOLDENLARGE) != 0) printf("bol-enlarge ");
	if ((na & NOOBLIQUE) != 0) printf("no-oblique ");
	if ((na & NOCONDENSE) != 0) printf("no-condense ");
	if ((na & NOEXPANDE) != 0) printf("no-expanse ");
	if ((na & NONMACENCODE) != 0) printf("non-mac-encode ");
	if ((na & NOINTERCHAR) != 0) printf("no-interchar ");	
	putc('\n', stdout);
}

void writestyles(int pass) {
	unsigned int stylestart, encodingptr;
	unsigned int nstrings;			/* number of style-name table strings */
	unsigned int k, m, moff;
	int l, n, flag, finx;
/*	unsigned int ks; */
	int baseneed, nsuffer;
	int	stylemap[4];
	int count, hitstyle, missstyle, sumstyle;

	if (pass == 0) encodingoffset = 0L;
	if (traceflag != 0 && pass == 1)
		printf("EncodingOffset %ld\n", encodingoffset);

	stylestart = scrindex;
	if (FONDClassFreeze == 0) {
/*  always need COORDINATE ? (certainly if reencoding to mac or other) */
		if (usemacencoding != 0 || reencodeflag != 0)  
			FONDClass = FONDClass | COORDINATE;
/*	to be safe always coordinate for now ... ??? */
		FONDClass = FONDClass | COORDINATE;
	}

/*  if using encoding as in outline font, no need for non-mac-encode ? */
	if (suppressencoding != 0) FONDClass = FONDClass & ~NONMACENCODE;
	if (ForceNonMac) FONDClass = FONDClass | NONMACENCODE; /* 95/Oct/24 */
	if (verboseflag != 0 && pass == 1) 	explainclass(FONDClass);

	if ((FONDClass & MACENCODING) != 0 && (FONDClass & NONMACENCODE) != 0) {
		fprintf(errout, 
			"WARNING: Specified both Mac encoding and non-Mac encoding\n");
/*		FONDClass = FONDClass & ~MACENCODING; */
	}
	uwritetwo(FONDClass);			/* Font Class */

/*	encodingptr = scrindex; */

/*	don't need encode table if using Mac encoding */
/*	don't need encode table if suppressed */
	if ((FONDClass & MACENCODING) == 0 && suppressencoding == 0)
		uwritefour(encodingoffset); /* offset to encoding-fix up next pass */
	else uwritefour(0L);
/*	note: this offset is from the beginning of the style mapping table */

	uwritefour(0L);					/* reserved word */

/*	basestyle = stylepointer[0]; */
	for (k = 0; k < 4; k++) {
		basestyle = stylepointer[k];
		if (basestyle >= 0) break;
	}

	if (nextsuffix[basestyle] != 0) baseneed = 1;
	else baseneed = 0;				/* Need just base name, no suffices */

/* check this code carefully !!! */
	
	if (verboseflag != 0 && pass == 1) printf("Style Mapping Table: ");

	finx = baseneed + 1;			/* index to first style */
	for (n = 0; n < 4; n++) {		/* regular, bold, italic, bolditalic */
/* see whether this style present */ /* NO flipping of bold & italic here */
		style = stylepointer[n];
		if (style < 0) flag = 0;
		else flag = finx++;
		
		stylemap[n] = flag;
/*		uwriteone(flag); */
		if (verboseflag != 0 && pass == 1) printf("%d", flag);
	}
/*	if (verboseflag != 0 && pass == 1) putc('\n', stdout); */
	
/*	Replicate what we have to unknown styles - in some semi-intelligent way */
	if (fillunknownstyles != 0) {
		count = 0;	hitstyle = 0; missstyle = 0; sumstyle = 0;
		for (n = 0; n < 4; n++) {			/* check how many are missing */
			if (stylemap[n] > 0) {
				hitstyle = n; sumstyle += n; count++;
			}
			else missstyle = n;
		}
		if (count == 0) fprintf(errout, "ERROR: No known styles\n");
		else if (count == 1) for (n = 0; n < 4; n++)
			stylemap[n] = stylemap[hitstyle];	/* just copy it everywhere */
		else if (count == 2) {
			if ((sumstyle & 1) != 0) {	/* one even and one odd position */
				for (n = 0; n < 4; n++) 
					if (stylemap[n] == 0) stylemap[n] = stylemap[n ^ 2];
			}
			else {	/* both even or both odd positions */
				for (n = 0; n < 4; n++) 
					if (stylemap[n] == 0) stylemap[n] = stylemap[n ^ 1];
			}
		}
		else if (count == 3) {	/* totally perverse !!! */
			if (missstyle == 1 || missstyle == 2) 
				stylemap[missstyle] = stylemap[0];
			else if (missstyle == 0) stylemap[missstyle] = stylemap[3];
			else if (missstyle == 3) stylemap[missstyle] = stylemap[0];
			
		}
		else if (count == 4) {
			if (verboseflag != 0 && pass == 1) 
				printf("All four styles known\n");
		}
	}

	if (verboseflag != 0 && fillunknownstyles != 0 && pass == 1) 
		printf(" => ");
	for (k = 0; k < 4; k++) {
		if (verboseflag != 0 && fillunknownstyles != 0 && pass == 1) 
			printf("%d", stylemap[k]);
		uwriteone(stylemap[k]);
	}
	if (verboseflag != 0 && pass == 1) putc('\n', stdout);
	

/* Repeat the pattern of 4 entries twelve times ? */

	if (fillstylemap != 0) 
		for (k = 4; k < 48; k++) uwriteone(stylemap[k & 3]);
	else for (k = 4; k < 48; k++) uwriteone(0); /* index to suffix style k */
	
	nstrings = baseneed;			/* need one for `regular' style ? */
	nstrings += nstyles;			/* need one for every style */
	nstrings += nsuffixes;			/* need one for every suffix */
	uwritetwo(nstrings);
	if (verboseflag != 0 && pass == 1) 
		printf("%d string%s in style-name table\n",
			   nstrings, (nstrings > 1) ? "s" : "");

	writepascalstring(CommonBaseName);
	if (verboseflag != 0 && pass == 1) 
		printf("(1) BaseName: %s\n", CommonBaseName);

	if (baseneed != 0) {			/* deal with `regular' style */
/*		style = stylepointer[0]; */
		style = basestyle;
/*	we are assuming that the first entry is always the `regular' style */
		nsuffer = nextsuffix[style];	/* how many suffix strings needed */
		uwriteone(nsuffer);			
		if (verboseflag != 0 && pass == 1) 
			printf("(2) ", nsuffer);
		for (l = 0; l < nsuffer; l++) {	/* step through indeces */
			for (m = 0; m < nsuffixes; m++) {
				if (strcmp(SuffixString[style][l], 
					CommonSuffixString[m]) == 0) {
					moff = m + nstyles + baseneed + 1; /* ??? */
					uwriteone(moff); 
					if (verboseflag != 0 && pass == 1) 
						printf("%d ", moff);
					break;
				}
			}
		}		
		if (verboseflag != 0 && pass == 1) putc('\n', stdout);
	}
/*	deal with other three styles ---  if any */
/*	for (n = 1; n < nstyles; n++) {	 */
	for (n = 0; n < 4; n++) {
/*		style = -1;
		for (l = 0; l < nstyles; l++) {
			if (stylecode[l] == n) {
				style = l; break;
			}
		} */
		style = stylepointer[n];	/* NO flipping of italic and bold */
		if (style < 0) continue;	/* this style not present */
		if (style == basestyle) continue;	/* already did this one */

		nsuffer = nextsuffix[style];	/* how many suffix strings needed */
		uwriteone(nsuffer);
		if (verboseflag != 0 && pass == 1) 
			printf("(%d) ", n + baseneed +1);
		for (l = 0; l < nsuffer; l++) {	/* step through indeces */
			for (m = 0; m < nsuffixes; m++) {
				if (strcmp(SuffixString[style][l], 
					CommonSuffixString[m]) == 0) {
					moff = m + nstyles + baseneed + 1; /* ??? */
					uwriteone(moff);
					if (verboseflag != 0 && pass == 1) 
						printf("%d ", moff);
					break;
				}
			}
		}
		if (verboseflag != 0 && pass == 1) putc('\n', stdout);
	}

	if (nsuffixes  > 0) {
		for (k = 0; k < nsuffixes ; k++) {
			writepascalstring(CommonSuffixString[k]);
			if (verboseflag != 0 && pass == 1) 
				printf("(%d) %s\n", k + baseneed + nstyles + 1, 
					CommonSuffixString[k]);
		}
	}

	if ((scrindex & 1) != 0) uwriteone(0);	/* word align --- 1992/June/16 */

	encodingptr = scrindex;
	encodingoffset = encodingptr - stylestart;

/*	don't need encode table if using Mac encoding */ /* usemacencoding != 0 */
/*	remember to also change whether encodingoffset gets written or not ... */
	if ((FONDClass & MACENCODING) == 0 && suppressencoding == 0)
		writeencoding(pass);
	else if (verboseflag != 0 && pass == 1) 
		printf("No need to write (relative) encoding\n");

	if ((scrindex & 1) != 0) uwriteone(0);	/* word align --- 1992/June/16 */

}

/* FontBBox information from `Inside LaserWriter' page 27 */

void writebboxes(int pass) {
	unsigned int numberofoffsets = 1;
	unsigned int numberofbboxes = nstyles;
	unsigned int count = 0;
	int m, style;
	
	uwritetwo(numberofoffsets - 1);	
	uwritefour(6L);					/* offset to bbox table - from here */
	uwritetwo(numberofbboxes - 1);
/*	for (m = 0; m < (int) numberofbboxes; m++) { */
	for (m = 0; m < 4; m++) {
		if (italicbeforebold != 0 && m > 0 && m < 3)
			style = stylepointer[3 - m];
		else style = stylepointer[m];
		if (style < 0) continue;

		uwritetwo(stylecode[style]);
		swritetwo(xll[style]); swritetwo(yll[style]);
		swritetwo(xur[style]); swritetwo(yur[style]);
		count++;
	}
	if (pass == 1 && count != numberofbboxes)
		fprintf(errout, 
			"ERROR: Inconsistent number of FontBBoxes %d versus %d\n",
				count, numberofbboxes);
}

/* FOND properties: page 38 */
/* 0 extra width for plain text - always 0 */
/* 1 extra width for bold text */
/* 2 extra width for italic text */
/* 3 extra width for underline text */
/* 4 extra width for outline text */
/* 5 extra width for shadow text */
/* 6 extra width for condensed text */
/* 7 extra width for extended text */
/* 8 not used - always set to 0 */

void writeproperties(int pass) {
	int k;
	for (k = 0; k < 9; k++) uwritetwo(0);
}

void writereserved(int pass) {
	int k;
	for (k = 0; k < 2; k++) uwritetwo(0);
}

/* wtaboff, kernoff, styloff are from top of the record to start of table */
/* wtaboff, kernoff, styloff are zero if table does not exist */

unsigned int assoff, wtaboff, kernoff, styloff;

/* Following is from `Inside MacIntosh IV' page 37 */

/* Fond Flags: 1 set if there is an image-height table */
/* Fond Flags: 2 set if there is an image-width table */
/* Fond Flags: 4 ... 800 reserved - should be zero */
/* Fond Flags: 1000 ignore FractEnable for stylistic variations */
/* Fond Flags: 2000 use integer extra width for stylistic variations */
/* Fond Flags: 4000 set if family fractional-width table is NOT used */
/* Fond Flags: 8000 set for fixed-width font */

unsigned int FONDFlags = 0X1000;  /* Ignore-FractEnable */

void createfond(int pass) {
	unsigned int fondstart, fondend;
/*	unsigned int assoff, wtaboff, kernoff, styloff; */
/*	int k; */
/*	int height; */

	if (pass == 0) {
		assoff=0; wtaboff=0; kernoff=0; styloff=0;
	}
	resourcedata[nextresource] = resourcepointer;
	strncpy(resourcenames[nextresource], "FOND", 4);
	nextresource++;

	uwritefour(0L);				/* length of resource - fix up later */
	fondstart = scrindex;		/* FONDResStart */
	if (verboseflag != 0 && pass == 0) printf("FONDResStart %u\n", fondstart);

	if (allfixedpitch != 0) FONDFlags = FONDFlags | 0X8000;
	else FONDFlags = FONDFlags & ~0X8000;
	uwritetwo(FONDFlags);		/* FONDFlags */
	uwritetwo(FONDResID);		/* FONDFamID */
	uwritetwo(FONDFirst);		/* FONDFirst */
	uwritetwo(FONDLast);		/* FONDLast */

/*	GetCommonFONDInfo(pass); */		/* already done earlier */
	swritetwo(FONDAscender);		/* Ascent */
	swritetwo(FONDDescender);		/* Descent */
	swritetwo(FONDLeading);			/* Leading */
	swritetwo(FONDWidthmax);		/* MaxWidth */

	uwritefour((unsigned long) wtaboff);

/*	if (nkerns == 0)  uwritefour(0L); *//* indicate absence kerning table */
	
	needkerns = 0;
	for (style = 0; style < nstyles; style++) {
		if (nkerns[style] > 0) {
			needkerns++; break;
		}
	}

	if (needkerns == 0)  uwritefour(0L); /* indicate absence kerning table */
	else uwritefour((unsigned long) kernoff);
	uwritefour((unsigned long) styloff);

/*	write style property info - 9 words */
	writeproperties(pass);
/*	write reserved words (`international') - 2 words */
	writereserved(pass);

	uwritetwo(2);		/* FOND version number must be 2 */
	
	assoff = scrindex - fondstart;
	if (verboseflag != 0 && pass == 0) printf("AssOff %d ", assoff);
/*	write font association table */
	writeassociation(pass);
/*	write BBox information */
	if (wantbboxes != 0) writebboxes(pass);
	wtaboff = scrindex - fondstart;
	if (verboseflag != 0 && pass == 0) printf("WTabOff %d ", wtaboff);
/*	write font width table */
	writewidths(pass);
	styloff = scrindex - fondstart;
	if (verboseflag != 0 && pass == 0) printf("StylOff %d ", styloff);
/*	write font style table */
	writestyles(pass);
	kernoff = scrindex - fondstart;
	if (verboseflag != 0 && pass == 0) printf("KernOff %d ", kernoff);
/*  write font kern table */
/*	if (nkerns > 0) writekerns(pass); */
	writekerns(pass);
	if (verboseflag != 0 && pass == 0) putc('\n', stdout);	

	fondend = scrindex;								// remember where we are
	if (verboseflag && pass == 0) printf("FONDResEnd   %u\n", fondend);
//	now fix up resource length 
	resourcepointer += fondend - (fondstart - 4);	// total length of resource
	resourcedata[nextresource] = resourcepointer;	// start of next resource 
	scrindex = fondstart - 4;						// where to put resource length
// length of resource itself
//	uwritefour ((unsigned long) (fondend - fondstart - 4));  
	uwritefour ((unsigned long) (fondend - fondstart));	// 1992/May/28
	scrindex = fondend;								// reset to where it was
	if (pass == 0) numberoffonds++;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define DESKINCREMENT 64 
#define DESKTOPWIDTH 350

void updatedesktop(void) {	/* pick a new desktop location */
	if (desktoph > DESKTOPWIDTH) {
		desktoph = 0; desktopv += DESKINCREMENT;
	}
	else desktoph += DESKINCREMENT;
}

void writescrfile(FILE *output, unsigned int scrfilelen) {
	unsigned int k;
/*	unsigned char *s=scrfile; */
/*	unsigned char __far *s=scrfile; */
	unsigned char __far *s=scrfile;
	for (k = 0; k < scrfilelen; k++) {
		putc(*s++, output); 
	}
}

/* Write MacBinary header - maybe don't bother to write when pass == 0 ? */

void writemacbinary(int pass) {
	unsigned int n;
/*	int k; */

	uwriteone(0);					/* ZERO 1 */
/*	Actual Mac filename needs to be the same as that in header... */
/*	if (verboseflag != 0 && pass == 1) 
		printf("Using %s for MacBinary file name\n", scrfilename); */
	n = strlen(scrfilename);
	if (n >= 64) {
		fprintf(errout, "Screen File Name `%s' too long\n", scrfilename);
		exit(3);
	}

	writepascalstring(scrfilename);
	writezeros(64 - (n + 1));

	writestring(filetype, 4);
	writestring(filecreator, 4);

	uwriteone(fileflags); 
	uwriteone(0);						/* ZERO 2 */

	swritetwo(desktopv);
	swritetwo(desktoph);
	swritetwo(desktopd);
	if (fixposition == 0) updatedesktop();
	uwriteone(protectedflags);  
	uwriteone(0); 					/* ZERO 3 */
	uwritefour(dataforklength);		/* always zero */
	uwritefour(resourceforklength);	/* need to fix second pass */
	uwritefour(creationdate);
	uwritefour(modificationdate);
	writezeros(128 - 99);
}

void putinresources(int pass) {
	int m;

	resourcepointer = 0;
	nextresource = 0;
/*	resourcedata[nextresource++] = resourcepointer; */
	resourcedata[nextresource] = resourcepointer;

	if (traceflag != 0 && pass == 1) printf("Constructing NFNT resource(s)\n");
	if (onesizefitsall != 0) createnfnt(pass);
	else {
/*		for (k = 0; k < nstyles; k++) { */
		for (m = 0; m < 4; m++) {
			if (italicbeforebold != 0 && m > 0 && m < 3)
				style = stylepointer[3 - m];
			else style = stylepointer[m];
			if (style < 0) continue;
/*			don't actually care about order - all the same */
			createnfnt(pass);
		}
	}

	if (traceflag != 0 && pass == 1) printf("Constructing FOND resource\n");
	createfond(pass);			/* all hell breaks loose ! */
}

unsigned int referoffset;				/* pointer into reference list */

void writetype(char *name, unsigned int n, int pass) {
	if (traceflag && pass == 1)
		printf("ResName %s NumberOfThisType %d ResOffset %u\n",
			name, n, referoffset);
	writestring(name, 4);		/* write the resource name */
	uwritetwo(n - 1);			/* how many of this type - 1 */
	uwritetwo(referoffset);		/* offset to type list */
	referoffset += 12 * n;
	resourcepointer += 4 + 2 + 2;
}

unsigned nameoffset=0;

void writereferitem(unsigned int resid, unsigned int attribute, 
		unsigned long resptr, char *resname, int pass) {
	unsigned int savedscrindex;

	uwritetwo(resid);							 /* resource ID */
	if (strcmp(resname, "") == 0) swritetwo(-1); /* not a named resource */
	else {
		swritetwo(nameoffset);
/*		nameoffset += strlen(resname) + 1; */
		if (pass > 0) {
			if (nameliststart == 0) fprintf(errout, "Bad Name List\n");
			else {
				savedscrindex = scrindex;
				scrindex = (int) (nameliststart + nameoffset + 384); /* ? */
				writepascalstring(resname);
				scrindex = savedscrindex;
			}
			nameoffset += strlen(resname) + 1;
		}
	}
	uwriteone(attribute);	/* Resource Attributes */
	uwritethree(resptr);	/* pointer to resource data */
	uwritefour(0L);			/* RESERVED handle to resource space holder */
	resourcepointer += 12;
}

void writetypelist(int pass) { /* ??? */
	unsigned int ntypes = 0;
	
	if (numberofnfnts > 0) ntypes++; 
	if (numberoffonds > 0) ntypes++; 
	referoffset = 2 + ntypes * 8;
	if (ntypes == 0) fprintf(errout, "WARNING: No Types to record\n");
	uwritetwo(ntypes - 1);		/* number of types - 1 */
	resourcepointer += 2;
	if (numberofnfnts > 0) 	writetype("NFNT", numberofnfnts, pass); 
	if (numberoffonds > 0) 	writetype("FOND", numberoffonds, pass); 
}

void addresources(int pass) {
/*	fill in data now available */
	ResDatLen = resourcepointer;	/* resource data length */
	ResMapOff = 256 + resourcepointer;	/* resource data map start */
}

/* NOTE: items following must be in same order as above ... */

void writereferencelist(int pass) { /* ??? */
	unsigned int  k=0;
	unsigned int m;
	char *s;
	
	nameoffset=0;

/*	for (m = 0; m < numberofnfnts; m++)  { */
	for (m = 0; m < 4; m++)  {
		if (italicbeforebold != 0 && m > 0 && m < 3)
			style = stylepointer[3 - m];
		else style = stylepointer[m];
		if (style < 0) continue;
		writereferitem(NFNTResID[style], NFNTattrib, resourcedata[k++], "", pass);
		if (onesizefitsall != 0) break;
	}
	for (m = 0; m < numberoffonds; m++) {	/* basically only one */
		if (forceresname) s = ResName;
		else if (usefontname) s = CommonFontName;
		else s = CommonMacName; 
/*		else s = MacName; */
/*		if (lowercase != 0) forcelower(s, s); */
		if (lowercaseflag != 0) forcelower(line, s);		
		else strcpy(line, s);
/*		writereferitem(FONDResID, 0, resourcedata[k++], s, pass); */
/*		writereferitem(FONDResID, 0, resourcedata[k++], line, pass); */
		writereferitem(FONDResID, FONDattrib, resourcedata[k++], line, pass);
	}
}

/* Inside LaserWriter p. 38 claims FOND Resource Name should be FontName */

/* NOTE: Resource Name need NOT be FontName or MacName */
/* NOTE: Resource Name is what appears in font menu */

void writenamelist(int pass) {
	scrindex += nameoffset;		/* how much space we actually used earlier */
	resourcepointer += nameoffset;
}

int constructscrfile(int pass) {
	unsigned long m;

	scrindex = 0;

	if (pass == 0) {
		numberofnfnts = 0; numberoffonds = 0;
	}

/*	first comes the MacBinary header of 128 bytes */
	writemacbinary(pass);
	
/*	data fork --- which is empty */
	
/*  resource fork --- which is where all the action is */
	
/*	first comes the resource header */
	
	writeresourceheader();		/* resource header */
	writezeros(128 - 16);		/* fill to 128 bytes with zeros */

/*  second comes the user defined data - 128 bytes of zeros */
	
	writeuserdefined();		/* user-defined area - application */

	ResDatOff = 128 + 128;				/* resource header + user defined */
	
	putinresources(pass);			/* NFNT's and FOND's */

/*	finish off resource data with other resources */

	addresources(pass); 

	if (nextresource >= MAXRESOURCES) {
		fprintf(errout, 
			"ERROR: Too many (%d) resources for table\n", nextresource);
		return -1;
	}

/*	resourcepointer = scrindex;	*/ /* scrindex above, now resourcepointer */

/*	fourth in the resource fork comes the resource map */

	resourcemapstart = resourcepointer;	/* remember this place */

	writeresourceheader();	/* repeat resource header 16 bytes */
	writezeros(4);			/* reserved - handle to next res map */
	writezeros(2);			/* reserved - file reference number */
	uwritetwo(resourcefileattributes);

	TypeOffset = (unsigned int) (typeliststart - resourcemapstart);
	uwritetwo(TypeOffset);		/* offset to type list */
	NameOffset = (unsigned int) (nameliststart - resourcemapstart);
	uwritetwo(NameOffset);		/* offset to name list */
	resourcepointer += 16 + 4 + 2 + 2 + 2 + 2;
	
	if (traceflag != 0 && pass == 1) 
		printf("Type List Offset: %d  Name List Offset: %d\n",
			TypeOffset, NameOffset);

	typeliststart = resourcepointer;
	if (traceflag != 0 && pass == 0)
		printf("TypeListStart %d\n", typeliststart - resourcemapstart);
/*	now write the resource Type List */
	writetypelist(pass);
	referenceliststart = resourcepointer;	/* remember this place */
	if (traceflag != 0 && pass == 0)
		printf("ReferenceListStart %d\n", referenceliststart - resourcemapstart);
/*  now write the reference list */
	writereferencelist(pass);
/*	if (traceflag != 0) printf ("OFFSET %d\n", scrindex - resourcepointer); */
	nameliststart = resourcepointer;		/* remember this place */
	if (traceflag != 0 && pass == 0)
		printf("NameListStart %d\n", nameliststart - resourcemapstart);
/*	now `write' name list */
	writenamelist(pass);
	
	ResMapLen = resourcepointer -  resourcemapstart;

	if (traceflag != 0 && pass == 2) printf("ResMapLen %d\n", ResMapLen);

	resourceforklength = resourcepointer + 128 + 128;

	m = dataforklength + resourceforklength;
		while(m++ % 512 != 0) uwriteone(0);  
	return 0;
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

int getline(FILE *infile, char *buff, int nmax) {
	int c, k=0;
	char *s=buff;

	while ((c = getc(infile)) != '\n' && c != EOF) {
		*s++ = (char) c;
		k++;
		if (k >= nmax) {
			*s = '\0';
			fprintf(errout, "Line too long: %s\n", buff);
			exit (13);
		}
	}
	if (c != EOF) {
		*s++ = (char) c;
		k++;
	}
	*s = '\0';
/*	if (traceflag != 0) printf("%s", buff); */
	return k;
}

int getrealline(FILE *infile, char *buff, int nmax) {
	int k;
	k = getline(infile, buff, nmax);
	while ((*buff == '%' || *buff == '\n' || *buff == '\r') && k > 0)
		k = getline(infile, buff, nmax);		
	return k;
}

void stripreturn(char *name) {
	int c;
	char *s=name;

	while ((c = *s) != '\0' && c != '\n' && c != '\r') s++;
	if (c == '\n' || c == '\r') *s = '\0';
	s--;			/* also get rid of trailing white space */
	while ((c = *s) <= ' ') *s-- = '\0';
}

/* modified 1992/Jan/5 to read first line of encoding vector for Encoding: */

/*int readencodingsub(char *vector) { */
int readencodingsub(char *vector, char encoding[MAXCHRS][CHARNAME_MAX]) {
	char fn_vec[FILENAME_MAX];
	char charcode[CHARNAME_MAX];
	FILE *fp_vec;
	char *s;
	int n, k;

/*	First try current directory */ /* added 1992/Jan/30 */
	strcpy(fn_vec, vector);
	forceexten(fn_vec, "vec");
	if ((fp_vec = fopen(fn_vec, "r")) == NULL) {

/*  If vector path specified use it - otherwise try current directory */
		if (strcmp(vectorpath, "") != 0) {
			strcpy(fn_vec, vectorpath);
			strcat(fn_vec, "\\");
		}
		else strcpy(fn_vec, "");
		
		strcat(fn_vec, vector);
		forceexten(fn_vec, "vec");

		if ((fp_vec = fopen(fn_vec, "r")) == NULL) {
/*		and if that fails try program path */
			strcpy(fn_vec, programpath); 
			strcat(fn_vec, "\\");
			strcat(fn_vec, vector);
			forceexten(fn_vec, "vec"); 
			if ((fp_vec = fopen(fn_vec, "r")) == NULL) {
				if (pass == 0) {
					fprintf(errout, "WARNING: Can't open encoding vector\n");
					perror(fn_vec); 
				}
				return -1;
			}
		}
	}
	if (verboseflag != 0) printf("Using encoding vector %s\n", fn_vec);

	n=0;
	strcpy(encodingvecname, "");
	(void) getline(fp_vec, line, MAXLINE);
	if ((s = strstr(line, "Encoding:")) != NULL) {
		s += strlen("Encoding:");
		while (*s++ <= ' ') ; s--;
		strncpy(encodingvecname, s, MAXENCODING);
		stripreturn(encodingvecname);
		if (verboseflag != 0)
			printf("EncodingVectorName: %s\n", encodingvecname); 
/*		(void) getrealline(fp_vec, line, MAXLINE); */
	}

	while (getrealline(fp_vec, line, MAXLINE) > 0) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf(line, "%d %s", &k, charcode) < 2) {
			fprintf(errout, "Don't understand encoding line: %s", line);
		} 
		else strcpy(encoding[k], charcode);
		if (k < charmin) charmin = k;
		if (k > charmax) charmax = k;
/*		printf("k %d, charname %s, line %s", k, charcode, line); */
		n++;
	}

/*	if (n != 128) { */			/* not in future TeX 3.0 ? */
	if (pass == 0)				/* 1993/Jan/9 */
		printf("Encoding vector has %d elements\n", n);
/*	} */

	if (ferror(fp_vec) != 0) {
		fprintf(errout, "Error in encoding vector\n");
		perror(fn_vec);
	}
	else fclose(fp_vec);
	return 0;			/* normal exit */
}

/*void readencoding(char *vector) { */
void readencoding(char *vector, char encoding[MAXCHRS][CHARNAME_MAX]) {
	if (readencodingsub(vector, encoding) != 0) {
		if (strcmp(vector, "mac") != 0) {
			fprintf(errout, "ERROR: Using MacIntosh Encoding instead\n");
/*			setupmacencoding(); */
			if(readencodingsub("mac", encoding) != 0) { 
				fprintf(errout, 
					"ERROR: Can't find MacIntosh Encoding vector file\n");
			} 
		}
/*		else exit(19); */
	}
/*	if (remapflag != 0) remapencoding(); */ /* redundant ? */
}

int copydata(int to, int from) {
	int style;
	int flag = 0;
	if (strcmp(screncoding[to], "") != 0) {
		fprintf(errout, "(=> %d %s) ", to, screncoding[to]);
		flag = 1;								/* 95/Sep/26 */
/*		return +1; */
	}
	if (strcmp(screncoding[from], "") == 0) {
		fprintf(errout, "(%d =>) ", from); 
		return -1;
	}
	strcpy(screncoding[to], screncoding[from]);
	for (style = 0; style < nstyles; style++) {	/* ??? */
		widths[style][to] = widths[style][from];
	}
	return flag;
}

int exchdata(int to, int from) {				/* 95/Sep/26 */
	int style;
	int temp;
	int flag = 0;
	char buffer[CHARNAME_MAX];

	printf("%d (%s) <=> %d (%s)\n", to, screncoding[to],
		   from, screncoding[from]);

	if (strcmp(screncoding[to], "") != 0) {
		fprintf(errout, "(=> %d %s) ", to, screncoding[to]);
		flag = 1;								/* 95/Sep/26 */
/*		return +1; */
	}
	if (strcmp(screncoding[from], "") == 0) {
		fprintf(errout, "(%d =>) ", from); 
		return -1;
	}
	strcpy(buffer, screncoding[to]);
	strcpy(screncoding[to], screncoding[from]);
	strcpy(screncoding[from], buffer);
	for (style = 0; style < nstyles; style++) {	/* ??? */
		temp = widths[style][to];
		widths[style][to] = widths[style][from];
		widths[style][from] = temp;
	}
	return flag;
}

void remapencoding(void) { 
	int k, count = 0, flag = 0;
	int flagpos=0, flagmin=0;
	
	for (k = 0; k < 10; k++) {
		if (strcmp(screncoding[k+161], "") != 0) count++;
		flag = copydata(k+161, k);
		if (flag > 0) flagpos++; else if (flag < 0) flagmin++;
	}
	for (k = 10; k < 32; k++) {
		if (strcmp(screncoding[k+163], "") != 0) count++;
		flag = copydata(k+163, k);	
		if (flag > 0) flagpos++; else if (flag < 0) flagmin++;		
	}
	flag = copydata(196, 127);
	if (flag > 0) flagpos++; else if (flag < 0) flagmin++;

/*	Following assumes that PFB file does not have 32 remapped to 128, 195 */
/*  if there is a real `space' in 32 --- 1992/Oct/12 */
	if (texturify != 0) {
		if (strcmp(pfbencoding[160], "space") == 0) {
			fprintf(errout, 
				"WARNING: Using `x' when there IS a `space' in 160?\n");
		}
		if (strcmp(pfbencoding[32], "space") == 0) {		/* 95/Sep/26 */
			fprintf(errout, 
				"WARNING: Using `x' when there IS a `space' in 32?\n");
		}
		flag = copydata(195, 32);
		if (flag > 0) flagpos++; else if (flag < 0) flagmin++;
		flag = copydata(128, 32);		/* ??? */
		if (flag > 0) flagpos++; else if (flag < 0) flagmin++;
	}

	if (flagpos > 0) 
		fprintf(errout, 
			"\nWARNING: Attempts to overwrite %d character(s)\n", flagpos);
	if (flagmin > 0) 
		fprintf(errout, 
			"\nWARNING: Attempts to remap %d blank character(s)\n", flagmin);

	if (charmin > 0) charmin = 0;
	if (charmax < 196) charmax = 196;	

/*	if (spacehack != 0) {
		if (strcmp(screncoding[160], "") == 0)
			strcpy(screncoding[160], "space"); 
	} */
/*	if (spacehack != 0 && (texturify || spacifyflag)) {
		if (strcmp(screncoding[160], "") == 0)
			strcpy(screncoding[160], "space"); 
	} */ 	/* 95/Sep/26 */
	if (count > 0) fprintf(errout, 
		"WARNING: remapping collided with %d characters\n", count);
}

void setupmacencoding (void) {
/*	int k; */
/*	for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], macencoding[k]); */
/*	charmin = 32; charmax = 255; */
	readencoding("mac", screncoding);
}

void resetencoding(char encoding[MAXCHRS][CHARNAME_MAX]) {
	int k;
	for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], "");
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* unsigned int nwidths; */

/* start looking for character in encoding at last + 1 */

int lookup(char *name, int last) {
	int k;
	for (k = last+1; k < MAXCHRS; k++) 
		if (strcmp(screncoding[k], name) == 0) return k;
	return -1;
}

/* int lookup(char *name) {
	int k;
	for (k = 0; k < MAXCHRS; k++) 
		if (strcmp(screncoding[k], name) == 0) return k;
	return -1;
} */

/*	if (spacehack != 0) {
		if (strcmp(name, "space") == 0 &&
			strcmp(screncoding[160], "") == 0) return 160; 
	} */

/* Modifications 1992/Nov/22 to allow repeated appearance of character */

unsigned int readwidths(FILE *input) {
	int k, m;
	int chrs, width, last, borrow;
	int missing = 0, repeated = 0;
	unsigned int count = 0;
	double fwidth;
/*	char charname[MAXLINE]; */
	char charname[CHARNAME_MAX];			/* character name */
	char charhit[MAXCHRS];			/* to catch repeat lines */

/*	charmin = 256; charmax = -1;  */
/*	nwidths = 0;  */
	nwidths[style] = 0; 

	for (k = 0; k < MAXCHRS; k++) 
		widths[style][k] = UNKNOWN;		/*	widths[k] = UNKNOWN;	 */

	for (k = 0; k < MAXCHRS; k++) charhit[k] = 0;	/* 1992/Sep/27 */

/*	if (reencodeflag == 0 && usemacencoding == 0) resetencoding(); */ /*???*/
	
/*	sscanf(line, "StartCharMetrics %u", &nwidths[style]); */
	sscanf(line+17, "%u", &nwidths[style]);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line == ';' || *line < ' ') continue;
		if (strncmp(line, "EndCharMetrics", 14) == 0) break;
		if (sscanf(line, "C %d ; WX %lg ; N %s ;", &chrs, &fwidth, charname)
			< 3) {
			fprintf(errout, "Don't understand %s", line);
			continue;
		}
		last = -1;						/* start looking at zero */
		count++;
		if (fwidth >= 0.0) width = (int) (fwidth + 0.4999);
		else width = - (int) (- fwidth + 0.49999);
		if (chrs >= 0 && chrs < MAXCHRS) {
			if (charhit[chrs] != 0)				/* 1992/Sep/27 */
				fprintf(errout, "\nERROR: Duplicate entry for char %d\n", 
					chrs);
			charhit[chrs]++;
		}

		for(;;) {							/* 1992/Nov/22 */
			if (last == MAXCHRS) break;		/* 1992/Nov/22 */
			if (reencodeflag != 0 || usemacencoding != 0) {
/*				chrs = lookup(charname); */
				chrs = lookup(charname, last);
/*				if (chrs < 0 && verboseflag != 0) { */
/*		check only first time through here */
				if (last < 0 && chrs < 0 && verboseflag != 0) {
					printf("%s? ", charname);
					missing++;
				}
				if (last >= 0 && chrs >= 0) {
					printf("%s! ", charname);	/* show duplication */
					repeated++;
				}
				if (chrs < 0) last = MAXCHRS;	/* set up for next time */
				else last = chrs;				/* 1992/Nov/12 */
			}
			else {	/* check space not in encoding - try and stick in at 160 */
				last = MAXCHRS;					/* 1992/Nov/22 */
				if (chrs < 0 && spacehack != 0 && 
					strcmp(charname, "space") == 0 &&
					strcmp(screncoding[160], "") == 0) {
					if (spacifyflag || texturify)			/* 95/Sep/26 */
						strcpy(screncoding[160], "space"); 	/* 95/Sep/26 */
					chrs = 160;
/*					if (charmax < 160) charmax = 160; */
				}
			}

			if (chrs < 0 || chrs >= MAXCHRS) continue;
			if (chrs < charmin) charmin = chrs;
			if (chrs > charmax) charmax = chrs;	
/*			widths[chrs] = width; */

			widths[style][chrs] = width;
			if (strlen(charname) >= CHARNAME_MAX) {
				fprintf(errout, "%s char name too long\n", charname);
			}
			else strcpy(screncoding[chrs], charname);
		}
	}

	borrow = 0;
/*  If text font, may borrow up to 20 math characters from Symbol font */
	if (usemacencoding != 0 && borrowsymbol != 0) {
		if (verboseflag != 0) putc('\n', stdout);
		for (m = 0; m < MAXCHRS; m++) {
			if ((k = mathchar[m]) == 0) break;
			if (widths[style][k] == UNKNOWN) {
/*			if (widths[style][k] == UNKNOWN || forcesymbol != 0) { */
				widths[style][k] = symwidth[m];
/*				if (isfixedpitch[style] != 0) 
					widths[style][k] = widths[style][65]; */ /* NO ! */
				if (k < charmin) charmin = k;
				if (k > charmax) charmax = k;
				charhit[k]++;
				borrow++;
				if (verboseflag != 0) printf("%s~ ", screncoding[k]);
			}
		}
/*		if (verboseflag != 0) {
			if (borrow > 0) {
				if (verboseflag != 0) putc('\n', stdout);
				printf("Borrowing %d widths from Symbol\n", borrow);
			}
		} */
	}
/*	if (verboseflag != 0) */
		putc('\n', stdout);
/*	if (count != nwidths)  */
/*	I don't thing it matter if the count does not match */
	if (count != nwidths[style]) 
/*		fprintf(errout, "Expected %d, only saw %d\n", nwidths, count); */
		fprintf(errout, "WARNING: Expected %d chars, but instead saw %d\n", 
			nwidths[style], count);
	if (missing != 0) {
		printf("Character(s) marked with `?' in font but not in encoding\n");
/*		fprintf(errout, "(or possible multiply encoded)\n");		*/
	}
	if (repeated != 0) {
		printf("Character(s) marked with `!' repeated in encoding\n");
	}
	if (verboseflag != 0) {
		if (borrow > 0) {
/*			if (verboseflag != 0) putc('\n', stdout); */
		printf("Character(s) marked with `~' borrowed from Symbol font\n");
		}
	}
	if (traceflag != 0)
		printf("First char %u Last char %u\n", charmin, charmax);
	return count;
}

unsigned int readkernpairs(FILE *input) {
	int kern, nchara, ncharb, numkerns = 0, count = 0;
	double fkern;
/*	char chara[MAXLINE], charb[MAXLINE]; */
	char chara[CHARNAME_MAX], charb[CHARNAME_MAX];
	int kernindex = 0;	/* used only while reading one AFM file */
	struct kernpair __far *kernstyle;		/* kern pairs for this style */

	kernstyle = kernpairs[style];	/* get pointer to appropriate array */

	if (kernstyle == NULL) {
		fprintf(errout, "ERROR: Bad Kern Table Pointer\n");
	}
/*	kernindex = 0;  */
/*	sscanf(line, "StartKernPairs %d", &numkerns); */
	sscanf(line+15, "%d", &numkerns);
	if (numkerns == 0) {
		nkerns[style] = 0;
		return 0;					/* ??? */
	}
	if (numkerns > maxkerns) {
		fprintf(errout, "ERROR: Too many kern pairs (%d > %d)\n",
			numkerns, maxkerns);
		fprintf(errout, "        Do one style at a time to get more space\n");
		exit(1);
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line < ' ') continue;
		if (strncmp(line, "EndKernPairs", 12) == 0) break;
		if (sscanf(line, "KPX %s %s %lg", chara, charb, &fkern)
			< 3) {
			fprintf(errout, "Don't understand %s", line);
		}
		else {
			if (kernindex >= maxkerns) {		/* 1993/Nov/9 */
				fprintf(errout,
					"ERROR: too many kern pairs (> %d)\n", maxkerns);
				exit(1);
			}

			count++;

			if (flushzeros != 0 && fkern == 0.0) continue; /* 93/Nov/9 */ 
		
			if (fkern >= 0.0) kern = (int) (fkern + 0.4999);
			else kern = - (int) (- fkern + 0.4999);

			nchara = lookup(chara, -1); 
			ncharb = lookup(charb, -1);

/*			Avoid those bogus kern pairs for Polish lslash and Lslash */
			if (spacifyflag || texturify) {						/* 95/Sep/27 */
				if (strcmp(chara, "suppress") == 0) nchara = -1;
			}

			if (nchara < 0 || ncharb < 0)
				fprintf(errout, "Ignoring kern pair: %s", line);
			else {
/*				kerna[style][kernindex] = (unsigned char) nchara; */
/*				kernb[style][kernindex] = (unsigned char) ncharb; */
/*				kern[style][kernindex] = kern; */
/*				kernpairs[style][kernindex].a = (unsigned char) nchara; */
/*				kernpairs[style][kernindex].b = (unsigned char) ncharb;	*/
/*				kernpairs[style][kernindex].kern = kern; */
				kernstyle[kernindex].a = (unsigned char) nchara; 
				kernstyle[kernindex].b = (unsigned char) ncharb;	
				kernstyle[kernindex].kern = kern; 
				kernindex++;
			}
		}	
	}
	if (count != numkerns) 
		fprintf(errout, "WARNING: Expected %d pairs, but saw %d\n", 
			numkerns, count);
/*	nkerns = kernindex; */
	nkerns[style] = kernindex;
	return kernindex;
}

void lowercase(char *s, char *t) {
	int c;
	while ((c = *t++) > 0) {
		if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
		*s++ = (char) c;
	}
	*s = '\0';
}

char *weightnames[] = {
	"black", "heavy", "bold",
	"demi", "semi", "demibold", "semibold",
	"medium", "plain", "regular", "standard", "roman", "normal", "book",
	"light", "thin", "ultralight", ""
};

int weightvalues[] = {
	800, 800, 800,
	600, 600, 600, 600,
	400, 400, 400, 400, 400, 400, 400,
	200, 200, 200, 0
};

/* This is pretty hopeless, everyone uses weird names ... */

int analyzeweight(char *s, char *line) {
	char str[128];
	int k;
	int weight=400;			/* default */

	sscanf(s, "%s", str);
	lowercase(str, str);		/* for stupid DTF fonts 1992/Oct/22 */
	for (k = 0; k < 64; k++) {
		if (strcmp(weightnames[k], "") == 0) break;
		if (strcmp(weightnames[k], str) == 0) break;
	}
	if (strcmp(weightnames[k], "") != 0) weight = weightvalues[k];
	else fprintf(errout, "Unrecognized Weight %s\n", line);
	if (traceflag) printf("Weight `%s' => %d\n", str, weight);
	return weight;
}

int checkFONDResID (unsigned int FONDResID) {
	if (FONDResID < FONDMinID) {
		fprintf(errout,
				"FONDResID out of range (%u < %u)\n",
				FONDResID, FONDMinID);
		return -1;			/* error  less than 3072 */
	}
	if (FONDResID > FONDMaxID) {
		fprintf(errout,	"FONDResID out of range for Latin font (%u > %u)\n",
				FONDResID, FONDMaxID);
		if (FONDResID < 19968) {
			if (FONDResID >= 19456)
				fprintf(errout, "OK for Cyrillic\n");
			else if (FONDResID >= 18944)
				fprintf(errout, "OK for Greek\n");
			else if (FONDResID >= 18432)
				fprintf(errout, "OK for Hebrew\n");
			else if (FONDResID >= 17920)
				fprintf(errout, "OK for Arabic\n");
			else return -1;		/* error < 17920 */
		}
		else return -1;			/* error > 19967 */
	}
	return 0;
}

/* int analyzeweight(char *s, char *line) {
	char str[128];
	sscanf(s, "%s", str);
	lowercase(str, str);
	if (strcmp(str, "Black") == 0) return 800;
	if (strcmp(str, "Heavy") == 0) return 800;
	if (strcmp(str, "Bold") == 0) return 800;
	if (strcmp(str, "Demi") == 0) return 600;	
	if (strcmp(str, "Semi") == 0) return 600;		
	if (strcmp(str, "DemiBold") == 0) return 600;
	if (strcmp(str, "Demibold") == 0) return 600;		
	if (strcmp(str, "SemiBold") == 0) return 600;
	if (strcmp(str, "Semibold") == 0) return 600;
	if (strcmp(str, "Medium") == 0) return 400;	
	if (strcmp(str, "Regular") == 0) return 400;
	if (strcmp(str, "Standard") == 0) return 400;
	if (strcmp(str, "Normal") == 0) return 400;	
	if (strcmp(str, "Book") == 0) return 400;		
	if (strcmp(str, "Roman") == 0) return 400;
	if (strcmp(str, "Light") == 0) return 200;
	if (strcmp(str, "Thin") == 0) return 200;
	if (strcmp(str, "UltraLight") == 0) return 200;	
	fprintf(errout, "Unrecognized Weight %s\n", str);
	return 400;	
} */

int readafmfile(FILE *input) {
	int flag = 0;
	int ksp;
	char *s;
	int weight = 400;
	double italicangle = 0.0;

/*	weight = 400; */
/*	italicangle = 0.0; */
/*	strcpy(FontName, "");  */
	strcpy(FontName[style], ""); 

	strcpy(EncodingScheme, ""); 

/*	FONDAscender = 0; FONDDescender = 0;  */
	ascender[style] = 0; descender[style] = 0;
/*	isfixedpitch = 0;  */
	isfixedpitch[style] = 0; 

	capheight[style] = 0; xheight[style] = 0;

/*	xll = 0; yll = -200; xur = 1000; yur = 700; */
	xll[style] = 0; yll[style] = -200; xur[style] = 1000; yur[style] = 700;	
/*	nkerns = 0; nwidths = 0; */
	nkerns[style] = 0; nwidths[style] = 0;
/*	boldflag = 0; italicflag = 0; */
	boldflag[style] = 0; italicflag[style] = 0;

/*	flag = fgets(line, sizeof(line), input);
	if (traceflag != 0) printf("LINE: %s FLAG: %04X ", flag, flag);	 */
	
	(void) fgets(line, sizeof(line), input);
	if (strncmp(line, "StartFontMetrics", 16) != 0) {
/*		fprintf(errout, "Appears not to be an AFM file: %s", line); */
		flag++;
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line < ' ') continue;
/*		if ((s = strstr(line, "MacIntoshName ")) != NULL &&  */
/*		if (strncmp(line, "Comment MacIntoshName", 21) == 0 &&  */
		if (_strnicmp(line, "Comment MacIntoshName", 21) == 0 && 
			forceresname == 0) {
/*			sscanf(s+14, "%s", ResName); */
			strcpy(ResName, line+22); stripreturn(ResName);	/* 1993/Jan/9 */
			if (traceflag) printf("%s", line);
			seenmacname++;
		} 
/*		if ((s = strstr(line, "AppleName ")) != NULL &&  */
/*		if (strncmp(line, "Comment AppleName", 17) == 0 &&  */
		if (_strnicmp(line, "Comment AppleName", 17) == 0 && 
			forceresname == 0) {
/*			sscanf(s+10, "%s", ResName); */
			strcpy(ResName, line+18); stripreturn(ResName);	/* 1993/Jan/9 */
			if (traceflag) printf("%s", line);
			seenmacname++;
		} 
/*		else if ((s = strstr(line, "MS-WindowsName ")) != NULL &&  */
/*		else if (strncmp(line, "Comment MS-WindowsName", 22) == 0 &&  */
		else if (_strnicmp(line, "Comment MS-WindowsName", 22) == 0 && 
			forceresname == 0 && seenmacname == 0){
/*			sscanf(s+15, "%s", ResName); */
			strcpy(ResName, line+23); stripreturn(ResName);	/* 1993/Jan/9 */
			if (traceflag) printf("%s", line);
			seenwindowname++;
		} 
/* following added 1992/Dec/2 */
/*		else if ((s = strstr(line, "MSMenuName ")) != NULL && */
/*		else if (strncmp(line, "Comment MSMenuName", 18) == 0 &&  */
		else if (_strnicmp(line, "Comment MSMenuName", 18) == 0 && 
			forceresname == 0 && seenmacname == 0) {
/*			sscanf(s+11, "%s", ResName); */
			strcpy(ResName, line+19); stripreturn(ResName);	/* 1993/Jan/9 */
			if (traceflag) printf("%s", line);
			seenwindowname++;
		} 
		else if ((s = strstr(line, "MacResID ")) != NULL && 
				 FONDResID == 0) {
			sscanf(s+9, "%u", &FONDResID); 
			if (traceflag != 0) printf("%s", line);
			checkFONDResID (FONDResID);
		} 		/* use for ResName ? */

		if (strncmp(line, "Comment ", 8) == 0) continue;   /* ignore comments */

/*		if (traceflag != 0) printf("From AFM File: %s", line); */
		if (strncmp(line, "FontName ", 9) == 0) {
/*			sscanf(line, "FontName %s", FontName[style]); */
			sscanf(line+9, "%s", FontName[style]); 
/*			if (strlen(FontName[style]) > 22) {
				fprintf(errout, 
					"WARNING: FontName %s may be too long (%d > %d)\n",
						FontName[style], strlen(FontName[style]), 22);
			} */
			if (traceflag) printf("%s", line);
		}
		else if (strncmp(line, "Ascender ", 9) == 0) {
/*			sscanf(line, "Ascender %d", &ascender[style]); */
			sscanf(line+9, "%d", &ascender[style]);
			if (traceflag != 0) printf("%s", line);
		}
		else if (strncmp(line, "Descender ", 10) == 0) {
/*			sscanf(line, "Descender %d", &descender[style]); */
			sscanf(line+9, "%d", &descender[style]);
			if (mindescend != 0) {	/* if limit given for this */
				if (descender[style] < mindescend)
					descender[style] = mindescend;
			}
			if (traceflag != 0) printf("%s", line);
		}
		else if (strncmp(line, "CapHeight ", 10) == 0) {
/*			sscanf(line, "CapHeight %d", &capheight[style]); */
			sscanf(line+10, "%d", &capheight[style]);
			if (traceflag != 0) printf("%s", line);
		}
		else if (strncmp(line, "XHeight ", 8) == 0) {
/*			sscanf(line, "XHeight %d", &xheight[style]); */
			sscanf(line+8, "%d", &xheight[style]);
			if (traceflag != 0) printf("%s", line);
		}
		else if (strncmp(line, "IsFixedPitch ", 13) == 0 ||
			     strncmp(line, "isFixedPitch ", 13) == 0) {			
/*			if (strstr(line, "true") != NULL) isfixedpitch = 1; */
			if (strstr(line, "true") != NULL) isfixedpitch[style] = 1;
/*			else if (strstr(line, "false") != NULL) isfixedpitch = 0; */
			else if (strstr(line, "false") != NULL) isfixedpitch[style] = 0;
			if (traceflag != 0) printf("%s", line);
		}
		else if (strncmp(line, "Weight ", 7) == 0) {
/* 			sscanf(line, "Weight %s", str); */
			if (traceflag != 0) printf("%s", line);
			weight = analyzeweight(line + 7, line);
/*			if (weight > 400) boldflag = 1;	else boldflag = 0; */
			if (weight > 400) boldflag[style] = 1;	
			else boldflag[style] = 0;
			if (suppressbold != 0) boldflag[style] = 0;	/* 1992/Jan/02 */
		}
		else if (strncmp(line, "ItalicAngle ", 12) == 0) {
/*			sscanf(line, "ItalicAngle %lg", &italicangle); */
			sscanf(line+12, "%lg", &italicangle);
/*			if (italicangle != 0.0) italicflag = 1;	else italicflag = 0; */
			if (italicangle != 0.0) italicflag[style] = 1;	
			else italicflag[style] = 0;
			if (italicangle > 0.0)
				fprintf(errout, "WARNING: ItalicAngle positive\n");
			if (suppressitalic != 0) italicflag[style] = 0;	/* 1992/Jan/02 */
			if (traceflag != 0) printf("%s", line);
		}
		else if (strncmp(line, "FontBBox ", 9) == 0) {
/*			sscanf(line, "FontBBox %d %d %d %d",  */
			sscanf(line+9, "%d %d %d %d", 
				&xll[style], &yll[style], &xur[style], &yur[style]);
			if (traceflag != 0) printf("%s", line);
			wantbboxes = 1;
		}
		else if (strncmp(line, "EncodingScheme ", 15) == 0) {
/*			sscanf(line, "EncodingScheme %s", EncodingScheme);  */
			sscanf(line+15, "%s", EncodingScheme); 
			if (traceflag != 0) printf("%s", line);
		}
		else if ((s = strstr(line, "Leading")) != NULL) {
			sscanf(s + 7, "%d", &FONDLeading);
			FONDLeading = macscale(FONDLeading);	/* 1993/Jan/21 */
			LeadingFreeze++;
			if (traceflag != 0) printf("%s", line);
		}
		else if (strncmp(line, "StartCharMetrics ", 17) == 0) {
			if (traceflag != 0) printf("%s", line);
			(void) readwidths(input);
		}
		else if (strncmp(line, "StartKernPairs ", 15) == 0) {
			if (traceflag != 0) printf("%s", line);
			(void) readkernpairs(input);
		}	
	}
	if (ascender[style] == 0) {
		fprintf(errout, "WARNING: Missing Ascender in AFM file?\n");
/*		ascender[style] = 750; */			/* ? */
	}
	if (descender[style] == 0) {
		fprintf(errout, "WARNING: Missing Descender in AFM file?\n");
/*		descender[style] = -250; */			/* ? */
	}
	if (seenmacname || seenwindowname) forceresname++;

/*	following, maybe not ??? */
/*	ksp = lookup("space");	 */
	ksp = lookup("space", -1);	
	if (ksp >= 0) {					/* sanity check 1995/Oct/19 */
		if (widths[style][ksp] != 0) {
/*			do we need to convert this to Mac coordinates ? */
			defaultwidth = widths[style][ksp];
			if (traceflag) printf("Default width %d\n", defaultwidth);
		}
	}

	return flag;
}

/* Following no longer used */

#ifdef IGNORE
void spacify(void) {
	int ksp, ksu;
/*	int style, temp; */

/*	ksp = lookup("space"); */
	ksp = lookup("space", -1);
/*	ksu = lookup("suppress"); */
	ksu = lookup("suppress", -1);
	if (ksu < 0 || ksp < 0) {
		fprintf(errout, "WARNING: Unable to switch `space' and `suppress'\n");
		return;
	}
	exchdata(ksp, ksu);							/* 95/Sep/26 */
/*	strcpy(screncoding[ksu], "space");
	strcpy(screncoding[ksp], "suppress");	
	for (style = 0; style < nstyles; style++) {
		temp = widths[style][ksu];	
		widths[style][ksu] = widths[style][ksp];
		widths[style][ksp] = temp; 
	} */
}
#endif

void scalethings(void) {
/*	int height; */
	unsigned int k;

	for (k = 0; k < MAXCHRS; k++) 
/*		widths[k] = macscale(widths[k]); */
		widths[style][k] = macscale(widths[style][k]);
/*	for (k = 0; k < nkerns; k++)  */
	for (k = 0; k < nkerns[style]; k++) 
/*		kerns[style][k] = macscale(kerns[style][k]); */
		kernpairs[style][k].kern = macscale(kernpairs[style][k].kern);
/*	height = FONDAscender - FONDDescender;
	if (LeadingFreeze == 0) {
		if (height < 1031) FONDLeading = 83;
		else FONDLeading = 0;
	} */
	ascender[style] = macscale(ascender[style]);	
	descender[style] = macscale(descender[style]);
	capheight[style] = macscale(capheight[style]); 
	xheight[style] = macscale(xheight[style]);	
	xll[style] = macscale(xll[style]); yll[style] = macscale(yll[style]);
	xur[style] = macscale(xur[style]); yur[style] = macscale(yur[style]);
	defaultwidth = macscale(defaultwidth);	/* 1995/Oct/19 */
	widthmax[style] = 0;
	for (k = 0; k < MAXCHRS; k++) {			/* compute max width */
/*		if (widths[k] > FONDWidthmax) FONDWidthmax = widths[k]; */
/*		if (widths[style][k] > FONDWidthmax) 
			FONDWidthmax = widths[style][k]; */
		if (widths[style][k] > widthmax[style]) 
			widthmax[style] = widths[style][k];
	}
}

/* Break up name at hyphen `-'  -and-  where upper case follows lower case */
/* Treat digits as uppercase ? 98/Jun/19 NO */

void splitname(char *name) {
	char *s=name, *t=name;
	unsigned int k;
	int c, n, lowerflag;
	int nextsuffer = 0;

	if (*name == '\0') 	fprintf(errout, "FontName NULL\n");

	if (strncmp(name, "ITC-", 4) == 0) name = name + 4;	/* ignore ITC- ??? */

	for (k = 0; k < MAXSUFFIX; k++) strcpy(SuffixString[style][k], "");
/*	nextsuffix[style] = 0; */

	lowerflag = 0;
	while ((c = *t) != '\0') {
		if (c == '-') break;
		if (lowerdef != 0) {	/*	following changed 1993/March/15 */
			if (lowerflag != 0 && c >= 'A' && c <= 'Z') break;  
/*			if (lowerflag != 0 && c >= '0' && c <= '9') break; */  /* 98/Jun/19 */
/*			if (c < 'A' || c > 'Z') lowerflag = 1;  */
		}
		else {
			if (lowerflag != 0 && (c < 'a' || c > 'z')) break; 
/*			if (c >= 'a' && c <= 'z') lowerflag = 1;  */
		}
		if (c >= 'a' && c <= 'z') lowerflag = 1;
		t++;
	}
	n = t - s;
	if (n == 0) fprintf(errout, "Empty Base Name\n");
	strncpy(BaseName[style], name, n);
	*(BaseName[style] + n) = '\0';
	if (c == '-')  {
/*		strcpy(SuffixString[style][nextsuffix[style]++], "-"); */
		strcpy(SuffixString[style][nextsuffer++], "-");
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
		if (nextsuffix[style] >= MAXSUFFIX) {
			fprintf(errout, "ERROR: Too many suffixes\n");
			break;
		}
/*		nextsuffer = nextsuffix[style]; */
/*		strncpy(SuffixString[style][nextsuffix[style]], s, n); */
		strncpy(SuffixString[style][nextsuffer], s, n);
/*		SuffixString[style][nextsuffix[style]][n] = '\0'; */
		SuffixString[style][nextsuffer][n] = '\0';
/*		nextsuffix[style]++; */
		nextsuffer++;
		if (c == '-')  {
/*			strcpy(SuffixString[style][nextsuffix[style]++], "-"); */
/*			strcpy(SuffixString[style][nextsuffix[style]++], "-"); */
			strcpy(SuffixString[style][nextsuffer++], "-");
			t++;
			continue;
		}
		if (c == '\0') break;
	}
	nextsuffix[style] = nextsuffer;

	if (verboseflag != 0) printf("FontName: %s\n", name);

	if (traceflag != 0) {
/*		printf("FontName: %s\n", FontName); */
		printf("BaseName: `%s' ", BaseName[style]);
		for (k = 0; k < nextsuffix[style]; k++)
			printf("Suffix %d: `%s' ", k, SuffixString[style][k]);
		putc('\n', stdout);
	}
} 

/* try and guess style from FontName */

int guessstyle(void) {
	int k;
	int stylecode = REGULAR;
	int nextsuffer;
	int boldflag=0, italicflag=0;
	char bname[MAXBASE];
	
	strcpy (bname, BaseName[style]); 	/* 1993/March/15 */
/*	forceupper(bname, bname);	*/		/* deal with lower case CM */
	
	nextsuffer=nextsuffix[style];
/* start with code for Computer Modern, AMS, LaTeX, SliTeX, LOGO fonts */
	if (strncmp(bname, "CM", 2) == 0 ||
		strncmp(bname, "ICM", 3) == 0 ||
		strncmp(bname, "LCM", 3) == 0 ||
		strncmp(bname, "ILCM", 4) == 0 ||
		strncmp(bname, "EUF", 3) == 0 ||
		strncmp(bname, "EUS", 3) == 0 ||
		strncmp(bname, "EUR", 3) == 0 ||	
		strncmp(bname, "WNCY", 4) == 0 ||
		strncmp(bname, "LASY", 4) == 0 ||
		strncmp(bname, "ILASY", 5) == 0 ||
		strncmp(bname, "LINE", 4) == 0 ||
		strncmp(bname, "LCIRCLE", 7) == 0 ||	
		strncmp(bname, "LOGO", 4) == 0) {
		if (strstr(bname, "BX") != NULL) boldflag++;
		if (strstr(bname, "DC") != NULL) boldflag++;
		if (strstr(bname, "MB") != NULL) boldflag++;
		if (strstr(bname, "MIB") != NULL) boldflag++;
		if (strstr(bname, "BF") != NULL) boldflag++;
		if (strstr(bname, "SSB") != NULL) boldflag++;
		if (strstr(bname, "SYB") != NULL) boldflag++;
		if (strstr(bname, "EUFB") != NULL) boldflag++;
		if (strstr(bname, "EUSB") != NULL) boldflag++;
		if (strstr(bname, "EURB") != NULL) boldflag++;
		if (strstr(bname, "SY") != NULL) italicflag++;
		if (strstr(bname, "TI") != NULL) italicflag++;
		if (strstr(bname, "SI") != NULL) italicflag++;
		if (strstr(bname, "SL") != NULL) italicflag++;
		if (strstr(bname, "QI") != NULL) italicflag++;
		if (strstr(bname, "MMI") != NULL) italicflag++;
		if (strncmp(bname, "WNCYB", 5) == 0) boldflag++;
		if (strncmp(bname, "WNCYI", 5) == 0) italicflag++;
		if (strncmp(bname, "LINEW", 5) == 0) boldflag++;
		if (strncmp(bname, "LCIRCLEW", 8) == 0) boldflag++;
		if (strncmp(bname, "CMFF", 4) == 0) italicflag++;
		if (strncmp(bname, "CMIT", 4) == 0) italicflag++;
		if (strncmp(bname, "CMINCH", 6) == 0) boldflag++;
		if (strncmp(bname, "CMFI10", 6) == 0) italicflag++;
	}
	else if (strncmp(bname, "MT", 2) == 0) {
		if (strncmp(bname, "MTMI", 4) == 0) italicflag++;
		if (strstr(bname, "B") != NULL) boldflag++;
		if (strstr(bname, "H") != NULL) boldflag++;
	}
	else if (strncmp(bname, "RMT", 3) == 0) {
		if (strncmp(bname, "RMTMI", 5) == 0) italicflag++;
		if (strstr(bname, "B") != NULL) boldflag++;
		if (strstr(bname, "H") != NULL) boldflag++;
	}
	else {
	for (k = 0; k < nextsuffer; k++) {
		if (strcmp(SuffixString[style][k], "Bold") == 0) 
			boldflag++;
		else if (strcmp(SuffixString[style][k], "Demi") == 0)
			boldflag++;
		else if (strcmp(SuffixString[style][k], "Demibold") == 0)
			boldflag++;
		else if (strcmp(SuffixString[style][k], "Heavy") == 0) 
			boldflag++;
		else if (strcmp(SuffixString[style][k], "Italic") == 0) 
			italicflag++;
		else if (strcmp(SuffixString[style][k], "Oblique") == 0) 
			italicflag++;		
		else if (strcmp(SuffixString[style][k], "Kursiv") == 0) 
			italicflag++;
		else if (strcmp(SuffixString[style][k], "Slanted") == 0) 
			italicflag++;
	}
	}
	if (boldflag != 0) stylecode = stylecode | BOLD;
	if (italicflag != 0) stylecode = stylecode | ITALIC;
	return stylecode;
}

/* Construct MacIntosh style 5 + 3 + 3 ... outline font file name */

void constructname(void) {
	int k=0, c;
/*	int lowcount; */
	int count, lowflag;
	unsigned m;

	strcpy (MacName[style], BaseName[style]);
	if (strncmp(MacName[style], "ITC-", 4) == 0)  /* flush `ITC-' */
		strcpy(MacName[style], MacName[style] + 4);
/*	lowcount = 0; */
	count = 0;
	lowflag = 0;
	while ((c = MacName[style][k]) > ' ') {
/*		if (c < 'A' || c > 'Z') lowflag = 1; */
		if (c >= 'a' && c <= 'z') lowflag = 1;
		if (lowflag != 0 && count > 4) break;
		count++;
		k++;
	}
/*	if (verboseflag != 0)  
		printf("MacName %s k %d low %d\n", MacName[style], k, lowflag); */
	MacName[style][k] = '\0'; 
/*	lowcount = 0; */

	for (m = 0; m < nextsuffix[style]; m++) {
		if (strcmp(SuffixString[style][m], "-") == 0) continue;
/*		if (strcmp(SuffixString[style][m], "Regular") == 0) continue; */
/*		if (strcmp(SuffixString[style][m], "Roman") == 0) continue;   */
/*		if (strcmp(SuffixString[style][m], "Normal") == 0) continue;   */
		if (stripbolditalic != 0) {
			if (strcmp(SuffixString[style][m], "Bold") == 0) continue;
			if (strcmp(SuffixString[style][m], "Demi") == 0) continue;  /* ? */
			if (strcmp(SuffixString[style][m], "Semibold") == 0) continue;  
			if (strcmp(SuffixString[style][m], "Italic") == 0) continue;
			if (strcmp(SuffixString[style][m], "Oblique") == 0) continue;
			if (strcmp(SuffixString[style][m], "Kursiv") == 0) continue;
		}
		if (*SuffixString[style][m] == '-') 
			strcat(MacName[style], SuffixString[style][m]+1);
		else strcat(MacName[style], SuffixString[style][m]);
/*		lowcount = 0; */
		count = 0; 
		lowflag = 0;
		while ((c = MacName[style][k]) > ' ') {
/*			if (c < 'A' || c > 'Z') lowflag = 1; */
			if (c >= 'a' && c <= 'z') lowflag = 1;
			if (lowflag != 0 && count > 2) break;
			count++;
			k++;
		}		
		MacName[style][k] = '\0';
	}
	if (k > 31) 
		fprintf(errout, "Outline font name %s is too long\n", MacName[style]);
}

/* Mac Outline font file name restricted to 31 characters */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/*  -f=<file-name> */
/*	printf("\tf  file name in MacBinary header (use this name on the Mac!)\n"); */


void showusage(char *name) {
	printf(
		"Useage: %s [-{v}{b}{g}{h}{a}{s}{l}]\n", name);
	printf("\t [-c=<screen-vector>] [-o=<outline-vector>]\n");
	printf("\t [-f=<FOND id>] [-n=<NFNT id>]\n");
	printf("\t [-r=<ResName>] [-d=<font-class>] [-p=<pt-size>]\n");
/*	printf("\t -r=<ResName> -d=<font-class> -x=<x-pos> -y=<y-pos>\n"); */
/*  printf("\t -r=<font name> -t=<filetype> -c=<creator>\n"); */
/*	printf("\t -x=<x-pos> -y=<y-pos>\n"); */
/*	printf("\t\t<afm-file-1>, <afm-file-2>, ...\n"); */
	printf("\t\t<afm-file-r> [<afm-file-b> [<afm-file-i> <afm-file-bi>]]\n");
	if (detailflag == 0) exit(0);

	printf("\tv  verbose mode\n");
	printf("\tb  force style to be `regular/roman' (single AFM file only)\n");
	printf("\tg  fill in gaps in styles using existing styles\n");
	printf("\th  replicate first four entries in style table\n");
	printf("\ta  force Resource Name (name in font menu) to be lower case\n");
	printf("\tl  add CM remap (0-9 => 161-170, 10-32 => 173-195, 127 => 196)\n");
	printf("\ts  switch `space' and `suppress' (in Computer Modern font only)\n");
	printf("\tx  modify screen font for Textures (coordinate `space' in 32)\n");

/*	printf("\ti  force numeric encoding vector\n"); */
/*	printf("\tw  show only differences from StandardEncoding in FOND resource\n"); */
/*	printf("\to  do not put encoding vector in FOND resource at all\n"); */
	printf("\tc  desired screen font encoding vector (default `%s')\n", 
		defaultscrvector);
	printf("\t   (if `none' specified, use encoding in AFM file)\n");
	printf("\t   (use `-c=mac' for plain text font)\n");
	printf("\to  actual outline font encoding vector (default `%s')\n",
		defaultpfbvector);
	printf("\t   (if `none' specified, treat encoding of outline font as unknown)\n");
	printf("\t   (do not use -o=... for plain text font)\n");
	printf("\tf  FOND Res ID (< %u) (override automatically generated)\n", FONDMaxID);
	printf("\tn  NFNT Res ID (< %u) (override automatically generated)\n", NFNTMaxID);
	printf("\tr  Resource Name (appears in font menu - default is base FontName)\n");  
	printf("\td  FOND Class (default %d dec = %04X hex)\n", FONDClass, FONDClass);
	printf("\tp  point size of fake screen font (default %d)\n", ptsize);
	printf("\n");
	printf("\tSpecify single style (AFM file), or family of four (R, B, I, BI)\n");
	printf("\tOutput is in MacBinary format & appears in current directory.");
	exit(0);
}

/*	printf("\tt  file type    - four characters to override default (%s)\n",
		filetype); */
/*	printf("\tc  file creator - four characters to override default (%s)\n",
		filecreator); */
/*	printf("\tx  horizontal position of icon on desktop\n"); */
/*	printf("\ty  vertical position of icon on desktop\n"); */
/*	printf("\tu  force file name in MacBinary header to be upper case\n"); */
/*	printf("\te  use MacIntosh encoding\n"); */
/*	printf("\tExtension is stripped off filename.\n"); */
/*	printf("\tFile Name in MacBinary header defaults to output file name.");*/
/*	printf("\tOn MacIntosh, file name must match MacBinary header name.\n");*/
/*	printf("\tFont name picked out of AFM font file.\n"); */


int encodingflag = 0, outlineflag = 0;

int FONDresourceflag=0, NFNTresourceflag=0, FONDClassflag=0;

/* int libraryflag=0, iconflag=0; */

int scrfilenameflag=0, creatorflag=0, typeflag=0; /* filenameflag=0; */
int horizflag=0, vertiflag=0, depthflag=0, resnameflag=0, ptsizeflag = 0;

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0;
		case 'v': verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case 'l': remapflag = 1; return 0;
		case 's': spacifyflag = 1; return 0;
		case 'x': texturify = 1; return 0;
		case 'y': stripcontrol = 1; return 0;		
		case 'e': usemacencoding = 0;  return 0;
		case 'a': lowercaseflag = 1; return 0;
		case 'b': forceregular = 1; return 0;
		case 'g': fillunknownstyles = 1; return 0;
		case 'h': fillstylemap = 1; return 0;
		case 'm': onesizefitsall = 0; return 0;
		case 'i': usenumerics = 1; return 0;
		case 'I': suppressitalic = 1; return 0;
		case 'B': suppressbold = 1; return 0;
/*		case 'H': nohyphen = 0;	return 0; */		/* 1993/Jan/24 */
		case 'H': nohyphen = 1;	return 0;			/* 1994/May/27 */
		case 'M': acceptmod = 1; return 0;			/* 1995/Oct/19 */
		case 'S': borrowsymbol = 0; return 0;		/* 1993/Apr/3 */
/*		case 'E': forcesymbol = 1; return 0; */		/* 1993/Apr/3 */
		case 'F': FullRange = 1; return 0;			/* 1993/Apr/3 */
/*		case 'F': FullRange = 0; return 0; */		/* 1995/Oct/25 */
/*		case 'o': suppressencoding = 1; return 0; */
/*		case 'w': outlinediffer = 1; return 0; */
		case 'w': outlineinit = 1; return 0;
		case '0': flushzeros = 0; return 0;
/*		case 'u': uppercase = 1; return 0; */
		case 'z': desktoph = 0; desktopv = 0; fixposition = 0; return 0;
		case 'N': ForceNonMac = 1; return 0;		/* 1995/Oct/24 */
/*		case 'E': altNFNTend = 0; return 0; */		/* 1995/Oct/24 */
		case 'Z': wildcards = 1; return 0;			/* 1997/Aug/24 */
		case 'E': errout = stderr; return 0;		/* 1997/Sep/23 */
/*		the rest need arguments */
		case 'f': FONDresourceflag = 1; break; 
		case 'n': NFNTresourceflag = 1; break; 
		case 'd': FONDClassflag = 1; break; 
		case 'c': encodingflag = 1; break;
		case 'o': outlineflag = 1; break;
/*		case 'f': scrfilenameflag = 1; break; */
		case 'r': resnameflag = 1; break;
		case 'p': ptsizeflag = 1; break;
		case 'D': descendflag = 1; break;
/*		case 'x': horizflag = 1; fixposition = 1; break; */
/*		case 'y': vertiflag = 1; fixposition = 1; break; */
/*		case 'd': depthflag = 1; fixposition = 1; break; */
		default: {
			fprintf(errout, "WARNING: Invalid command line flag '%c'\n", c);
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
				if (horizflag != 0) {
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
				else if (ptsizeflag != 0) {
					if (sscanf(s, "%d", &ptsize) < 1) {
						fprintf(errout, "Don't understand point size: %s\n",
							s);
					}
					ptsizeflag = 0;
				}
/*				else if (scrfilenameflag != 0) {
					strncpy(macbinaryname, s, 80);
					scrfilenameflag = 0;
				} */
				else if (resnameflag != 0) {
/*					ResName = s; */
					strncpy(ResName, s, FONTNAME_MAX);
					forceresname++;
					resnameflag = 0;
				} 
				else if (creatorflag != 0) {
					strncpy(filecreator, s, 4);
					creatorflag = 0;
				}
				else if (typeflag != 0) {
					strncpy(filetype, s, 4);
					typeflag = 0;
				}				
/*				else if (filenameflag != 0) {
					strncpy(outlinefilename, s, MAXOUTLINE);
					filenameflag = 0;
				}	*/
				else if (encodingflag != 0) {
					scrvector = s;
					reencodeflag++;
					usemacencoding = 0;	/* well, could be mac encoding ... */
					encodingflag = 0;
				}
				else if (outlineflag != 0) {
					pfbvector = s;
					outlineflag = 0;
				}
				else if (descendflag != 0) {	/* 1993/July/16 */
					if (sscanf(s, "%d", &mindescend) < 1) {
					   fprintf(errout, "Don't understand min descend %s\n", s);
					}
					descendflag = 0; 
				}
				else if (FONDresourceflag != 0) {
/*					if (sscanf(s, "%d", &FONDResID) < 1) { */
					if (sscanf(s, "%d", &FONDResInit) < 1) {
						fprintf(errout, 
							"Don't understand FOND resource ID: %s\n", s);
					}
/*					checkFONDResID (FONDResID); */
					checkFONDResID (FONDResInit);
					FONDresourceflag = 0;
				} 
				else if (NFNTresourceflag != 0) {
/*					if (sscanf(s, "%d", &NFNTResID[style]) < 1) { */
					if (sscanf(s, "%d", &NFNTResInit[style]) < 1){ 
						fprintf(errout, 
							"Don't understand NFNT resource ID: %s\n", s);
					}
/*					if (NFNTResID[style] < NFNTMinID || 
						NFNTResID[style] > NFNTMaxID)
						fprintf(errout, "NFNTResID out of range?\n"); */
					if (NFNTResInit[style] < NFNTMinID || 
						NFNTResInit[style] > NFNTMaxID)
						fprintf(errout, "NFNTResID out of range?\n");
					style++;
					NFNTresourceflag = 0;
				} 		
				else if (FONDClassflag != 0) {
					if (sscanf(s, "%d", &FONDClass) < 1) {
						fprintf(errout, 
							"Don't understand FONDClass: %s\n", s);
					}
					else FONDClassFreeze = 1;
					FONDClassflag = 0;
				} 
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

int replicate(void) {	/* fill in widths of repeat encoded chars */
	int k, m, count=0;
	int style;

	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(screncoding[k], "") == 0) continue;
/*		if (widths[style][k] == UNKNOWN) continue; */
		for (m = 0; m < MAXCHRS; m++) {
			if (m == k) continue;
			if (strcmp(screncoding[m], "") == 0) continue;
/*			if (widths[style][m] != UNKNOWN) continue;		*/
			if (strcmp(screncoding[k], screncoding[m]) == 0) {
				for (style = 0; style < nstyles; style++) { /* ??? */
					if (widths[style][k] != UNKNOWN &&
						widths[style][m] == UNKNOWN) {
					widths[style][m] = widths[style][k];
					count++;
					}
				}
			}
		}
	}
	if (verboseflag != 0 && count > 0)
		printf("Filled in %d repeated char widths\n", count);
	return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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
/*	fflush(errout); */
	return hash;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned int makerandom(unsigned int bottom, unsigned int top) {
	unsigned int trial;
	for(;;) {
		trial = rand() + rand();
		if (trial > bottom && trial < top) return trial;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* remove file name - keep only path - inserts '\0' to terminate */
void removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
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

void stampit(FILE *outfile) {
	char date[11 + 1];
	strcpy(date, compiledate);
/*	scivilize(compiledate);	 */
	scivilize(date);	
	fprintf(outfile, "%s (%s %s)\n", 
		programversion, date, compiletime);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int doallocations (int argc, int firstarg) {
	int k;

/*	scrfile = (unsigned char __far *) _fmalloc(MAXSCRFILE); */
	scrfile = (unsigned char __far *) _fmalloc(maxscrfile);
	if (scrfile == NULL) {
		printf("NOTE: Unable to allocate memory for SCR file\n");
/*		exit(1); */
		return 0;
	}
/*	Can allocate lots for kern table if single style */
	if (argc == firstarg +1) {				/* single style */
/*		maxkerns = 10000U; */
		maxkerns = maxkernsone;
		kernpairs[0] = (struct kernpair __far *)
				_fmalloc(maxkerns * sizeof(struct kernpair));
		if (kernpairs[0] == NULL) {
			printf("NOTE: Unable to allocate memory for kern pairs\n");
/*			exit(1); */
			return 0;
		}
		for (k = 1; k < 4; k++) kernpairs[k] = NULL;
	}
	else {
/*		maxkerns = 4192U; */
		maxkerns = maxkernsone / 2;
		for (k = 0; k < 4; k++) {
			kernpairs[k] = (struct kernpair __far *)
				_fmalloc(maxkerns * sizeof(struct kernpair));
			if (kernpairs[k] == NULL) {
				printf("NOTE: Unable to allocate memory for kern pairs\n");
/*				exit(1); */
				return 0;
			}
		}
	}
	return 1;										/* success */
}

void freeallocation (void) {
	int k;
	if (scrfile != NULL) _ffree(scrfile);
	for (k = 0; k < 4; k++) 
		if (kernpairs[k] != NULL) _ffree(kernpairs[k]);
}

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

/* set up full file name for ini file and check for specified section */
/* e.g. dviwindo = setupinifile ("dviwindo.ini", "[Environment]") */

/* #define FNAMELEN 80 */

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

int dostyles (char *argv[], int firstarg, int nstyles) {
	char afmfile[FILENAME_MAX], macfile[FILENAME_MAX]="";
	FILE *input, *output;
	int k, l, m, n, ksp, ksu, temp;
	int lowstyle;
	unsigned long hash;
	char buffer[CHARNAME_MAX];

	charmin = 255; charmax = 0;

/*	outlinediffer = 0; */
	outlinediffer = outlineinit;		/* reset to starting value */

/*	FONDResID = 0; */
	FONDResID = FONDResInit;			/* reset to starting value */
	FONDResInit = 0;					/* avoid reuse */

	for (k = 0; k < 4; k++) NFNTResID[k] = NFNTResInit[k];
	for (k = 0; k < 4; k++) NFNTResInit[k] = 0;	/* avoid reuse */

	for (style = 0; style < nstyles; style++) {
		strcpy(afmfile, argv[firstarg + style]);
		extension(afmfile, "afm");
		if((input = fopen(afmfile, "r")) == NULL) {
			if (tryunderscore != 0) underscore(afmfile);
			if((input = fopen(afmfile, "r")) == NULL) {			
				perror(afmfile); exit(3);
			}
		}
		
		if (verboseflag != 0) putc('\n', stdout);
		printf("Processing AFM file %s\n", afmfile);

		setupdateandtime(afmfile);	/* set up file modification date */
		
/* might want to pick date and time of `lowest style' instead of last ? */


/*		printf("Processing %s => %s\n", afmfile, macfile); */

		if (traceflag != 0) printf("Reencoding\n");

		if (style == 0) {				/* do only first time ??? */
			if (usemacencoding != 0) {		/* use MacIntosh encoding */
				setupmacencoding();
				if (FONDClassFreeze == 0) 
					FONDClass =  FONDClass | MACENCODING;
			}
			else if (reencodeflag != 0) {	/*  specified encoding vector */
				readencoding(scrvector, screncoding);
				if (FONDClassFreeze == 0) 
					FONDClass =  FONDClass | NONMACENCODE;
			}
			else {							/* use encoding in AFM file */
				resetencoding(screncoding);
				if (FONDClassFreeze == 0) 
/* if using encoding as in font then no need for this ? */
					FONDClass =  FONDClass | NONMACENCODE; /* ??? */
			}
			if (reencodeflag != 0) {
				if (remapflag != 0) {
					if (verboseflag != 0) printf("Adding remapping\n");
					remapencoding();
				}
/*				if (spacifyflag != 0) spacify(); */		/* 95/Sep/27 */
			}
			if (reencodeflag == 0 && usemacencoding == 0) 
				resetencoding(screncoding);
		}

		if (traceflag != 0) printf("Reading AFM file %s\n", afmfile);
/*		if (reencodeflag == 0 && usemacencoding == 0) resetencoding(); */
		if (readafmfile(input) != 0)
			fprintf(errout, "%s appears not to be an AFM file: %s", 
				afmfile, line);
		fclose(input);
/* following sanity checks are now pretty useless with MAXMAC == 255 */
/*		if (charmin == 32 && charmax == 251 && reencodeflag == 0) { */
/*		if (charmin == 32 && charmax == MAXMAC &&
				usemacencoding == 0) { */ /*94/July/7 */
		if (charmin == 32 && charmax == MAXMAC &&
			usemacencoding == 0	&& ForceNonMac == 0) {/*95/Oct/25 */
			fprintf(errout,
"NOTE: Did not specify encoding for what may be plain text font\n");
			fprintf(errout,
				"         Maybe use `-c=mac' for plain text font?\n");
		}
/*		if ((charmin != 32 || charmax != 251) && usemacencoding != 0) { */
		if ((charmin != 32 || charmax != MAXMAC) && usemacencoding != 0) {/*94/July/7 */
			fprintf(errout,
"NOTE: Specified Mac encoding for what may not be plain text font (%d -- %d)\n",
charmin, charmax);
			fprintf(errout,
				"         Maybe do not use `-c=mac'?\n");
		}
		if (usemacencoding != 0 && ForceNonMac != 0) {
			fprintf(errout,
	"Conflict between request for Mac and non-Mac encoding requests\n");
		}
/*		if (style == 0) {	
			if (reencodeflag == 0) {
				if (verboseflag != 0) printf("Adding remapping\n");
				if (remapflag != 0) remapencoding();
				if (spacifyflag != 0) spacify();
			}
		} */
		
/*		replicate();		*/	/* copy widths info for repeat encodings */
		if (traceflag != 0) printf("Scaling Things\n");
		scalethings();
		if (traceflag != 0) printf("Splitting Name\n");
		splitname(FontName[style]);

		stylecode[style] = REGULAR;
		if (boldflag[style] != 0) stylecode[style] = stylecode[style] | BOLD;
		if (italicflag[style] != 0) stylecode[style] = stylecode[style] | ITALIC;
		if (nstyles == 1 && forceregular != 0) stylecode[style] = REGULAR;

		n = guessstyle();
		l = stylecode[style];
		if (n != l) {
			if (suppressbold) {
				n = n & ~BOLD;
				l = l & ~BOLD;
			}
			if (suppressitalic) {
				n = n & ~ITALIC;
				l = l & ~ITALIC;
			}
			if (n != l) {
/*			printf("WARNING: Possibly inconsistent style code %d for ",	m); */
			printf("WARNING: Style ");
			if ((l & BOLD) != 0) printf("BOLD ");
			if ((l & ITALIC) != 0) printf("ITALIC ");
			if ((l & BOLD) == 0 && (l & ITALIC) == 0) printf("REGULAR ");
			printf("(from AFM) <> Style ");
			if ((n & BOLD) != 0) printf("BOLD ");
			if ((n & ITALIC) != 0) printf("ITALIC ");
			if ((n & BOLD) == 0 && (n & ITALIC) == 0) printf("REGULAR ");
			printf("(from FontName) %s%s%s\n",
				   suppressbold ? "SUPPRESSBOLD " : "",
				   suppressitalic ? "SUPPRESSITALIC " : "",
				   forceregular ? "FORCEREGULAR" : "");
			printf("WARNING: Check Weight and ItalicAngle in AFM file\n");
			}
		}

		if (traceflag != 0) printf("Constructing Mac Name\n");
		constructname();
		if (verboseflag != 0) 
			printf("Corresponding outline font file name on Mac: %s\n", 
				MacName[style]);
	}

	if (reencodeflag == 0) {
		if (remapflag != 0) {
			remapencoding();
			if (traceflag != 0) printf("Adding remapping\n");
		}
/*		if (spacifyflag != 0) spacify(); */	/* 95/Sep/27 */
	}
	replicate();
	
/*	now finished with reading all AFM files */
/*	play little game to avoid Textures printing crap on top left of page */

	if (texturify != 0) {
		if (outlinediffer != 0)
			fprintf(errout, "WARNING: conflict between `x' and `w' flags\n");
/*		move 32 => 128  --- and move space => 32 */
/*		ksp = lookup("space"); */
		ksp = lookup("space", -1);
		if (ksp == 32)
			fprintf(errout, "WARNING: `space' already has code position 32\n");
		else if (ksp < 0) 
			fprintf(errout, "WARNING: Unable to find `space'\n");
		else {
			if (strcmp(screncoding[128], "") != 0) {
				if (strcmp(screncoding[128], screncoding[32]) != 0)
					fprintf(errout, 
						"WARNING: position 128 occupied by %s (use for %s)\n",
							screncoding[128], screncoding[32]);
			}
			for (k = 0; k < MAXCHRS; k++)  /*	copy AFM file encoding */
				strcpy(pfbencoding[k], screncoding[k]); 
/*				strcpy(pfbencoding[k], ""); *//* try & force full encoding */
			strcpy(screncoding[128], screncoding[32]);
			strcpy(screncoding[32], screncoding[ksp]);
			for (style = 0; style < nstyles; style++) {	/* ??? */
				widths[style][128] = widths[style][32];
				widths[style][32] = widths[style][ksp];
			}
/*	show only differences between before and after switch */
			outlinediffer = 1;							/* set now */
/*			printf ("OUTLINEDIFFER texturify\n"); */	/* DEBUGGING */
			suppressencoding = 0;
			FONDClass = FONDClass | NONMACENCODE; /* needed now */
			FONDClass = FONDClass & ~MACENCODING; /* avoid conflict */
		}
	}
			
/*	now finished with reading all AFM files */			/* 95/Sep/26 */
/*	play little game to switch `space' and `suppress' (or whatever in 32) */

	if (spacifyflag != 0) {
		if (outlinediffer != 0)
			fprintf(errout, "WARNING: conflict between `s' and `w' flags\n");
/*		switch 32 <=> 160 */
		ksp = lookup("space", -1);
		ksu = lookup("suppress", -1);
/*		if there is no `suppress', then use whatever in 32 --- 95/Sep/27 */
		if (ksu < 0) {
			ksu = 32;
			if (strcmp(screncoding[ksu], "") == 0) ksu = -1;
			if (strcmp(screncoding[ksu], "space") == 0) ksu = -1;
		}
		if (ksp == 32)
			fprintf(errout, "WARNING: `space' already has code position 32\n");
		else if (ksp < 0) 
			fprintf(errout, "WARNING: Unable to find `space'\n");
		else if (ksu < 0)
			fprintf(errout, "WARNING: Unable to find `suppress'\n");			
		else {
			for (k = 0; k < MAXCHRS; k++)  /*	copy AFM file encoding */
				strcpy(pfbencoding[k], screncoding[k]); 
			strcpy(buffer, screncoding[ksu]);
			strcpy(screncoding[ksu], screncoding[ksp]);
			strcpy(screncoding[ksp], buffer);
			for (style = 0; style < nstyles; style++) {	/* ??? */
				temp = widths[style][ksu];
				widths[style][ksu] = widths[style][ksp];
				widths[style][ksp] = temp;
			}
/*	show only differences between before and after switch */
			outlinediffer = 1;						/* set now */
/*			printf ("OUTLINEDIFFER spacify\n"); */	/* DEBUGGING */
			suppressencoding = 0;
			FONDClass = FONDClass | NONMACENCODE; /* needed now */
			FONDClass = FONDClass & ~MACENCODING; /* avoid conflict */
		}
	}
			
	if (traceflag != 0) {	/* debugging */
		printf("PFB encoding (after AFM read):\n");
		for (k = 0; k < MAXCHRS; k++) 
			if (strcmp(pfbencoding[k], "") != 0) 
				printf("%s ", pfbencoding[k]);
		putc('\n', stdout);
	}

/*	now finished with reading all AFM files */
	if (verboseflag != 0) putc('\n', stdout);

	if (FullRange != 0) {	/* after all that - can force full range */
		charmin = 0; charmax = 255;
		printf("Forcing full range %d %d\n", charmin, charmax);
	}

	GetCommonFONDInfo(0);		/* need to do early to set things up ! */

	NFNTFirst = charmin; NFNTLast = charmax;		/* safe ??? */
	FONDFirst = charmin; FONDLast = charmax;		/* safe ??? */
	if (stripcontrol != 0 && charmin < 32) {	/* a trial ... */
		if (remapflag == 0)
	fprintf(errout, "ERROR: Stripping control characters without remapping\n");
		if (verboseflag != 0) printf("Stripping control characters\n");
		if (NFNTFirst < 32) NFNTFirst = 32;
		if (FONDFirst < 32) FONDFirst = 32;
		for (k = 0; k < 32; k++) strcpy(screncoding[k], "");
		for (k = 0; k < 32; k++) strcpy(pfbencoding[k], "");
/* following is LucidaNewMath specific (strip small/negative spaces) */
		if (NFNTLast > 252) NFNTLast = 252;
		if (FONDLast > 252) FONDLast = 252;
		for (k = 252; k < 256; k++) strcpy(screncoding[k], "");
		for (k = 252; k < 256; k++) strcpy(pfbencoding[k], "");
		if (FullRange != 0) {	/* after all that - can force full range */
			NFNTFirst = charmin; NFNTLast = charmax;
			FONDFirst = charmin; FONDLast = charmax;
/*			printf("Forcing full range %d %d\n", charmin, charmax); */
		}
	}

	if (verboseflag != 0) 
		printf("FONDFirst %d and FONDLast %d\n", FONDFirst, FONDLast);

	if (strcmp(macfile, "") == 0) {		/* pick file name of regular style */
		strcpy(macfile, stripname(afmfile)); /* should following fail ... */
	/*	if (nstyles > 1)  {
			for (k = 0; k < nstyles; k++) {
				if (stylecode[k] == 0) {
					strcpy(macfile, stripname(argv[k + firstarg]));
					break;
				}
			}
		} */
/*	pick regular, bold, italic, bolditalic --- in that order --- file name  */
		if (nstyles > 1)  {					/* 1993/Jan/2 */
			lowstyle = 1 + 2 + 1;			/* bold * 1 + italic * 2 */
			for (k = 0; k < nstyles; k++) {
				if (stylecode[k] < lowstyle) {
					strcpy(macfile, stripname(argv[k + firstarg]));
					lowstyle = stylecode[k];
				}
			}
		}
		removeexten(macfile);
		removeunder(macfile);
		extension(macfile, "scr");
	}

	if (strcmp(macbinaryname, "") != 0) strcpy(scrfilename, macbinaryname);
	else strcpy(scrfilename, macfile);
	if (uppercase != 0) forceupper(scrfilename, scrfilename);
	else forcelower(scrfilename, scrfilename);			/* new */
	if (verboseflag != 0) 
		printf("MacBinary header file name: %s\n", scrfilename);

/*  base FONDResID on FontName of Regular Style */
/*	style = stylepointer[0]; */
	for (m = 0; m < 4; m++) {
		basestyle = stylepointer[m];
		if (basestyle >= 0) break;
	}
	hash = hashstring(FontName[basestyle]);
	srand ((unsigned int) (hash + (hash >> 16)));

	if (FONDResID == 0) {
		FONDResID = makerandom(FONDMinID, FONDMaxID);
		printf("Using %u for FONDResID\n", FONDResID);
	}
/*	else if (FONDResID < FONDMinID || FONDResID > FONDMaxID) */
/*		fprintf(errout, "ERROR: FONDResID (%u) out of range\n", FONDResID);  */
	checkFONDResID(FONDResID);		/* 97/June/5 */
		
/*	generate random NFNTResID's also */
	for (k = 0; k < 4; k++) {
		if (NFNTResID[k] == 0) {
			NFNTResID[k] = makerandom(NFNTMinID, NFNTMaxID);
		}
		if (FONDResID == NFNTResID[k]) NFNTResID[k]++;
		for (l = 0; l < k; l++) {
			if (NFNTResID[l] == NFNTResID[k]) NFNTResID[k]++;
		}
		if (NFNTResID[k] < NFNTMinID || NFNTResID[k] > NFNTMaxID)
			fprintf(errout, "ERROR: NFNTResID (%u) out of range\n", NFNTResID[k]);
	}

	if (traceflag != 0) printf("Now constructing resource fork\n");
		dataforklength = 0;		/* assumed zero throughout */
		resourceforklength = 0;	/* fix up later */
		nextresource = 0;		/* index into ResourceData array */

		ResDatOff = 256;	/* offset to resource data */
		ResMapOff = 0;		/* offset to resource map - later */
		ResDatLen = 0;		/* length of resource data - later */
		ResMapLen = 0;		/* length of resource map - later */

		resourcepointer = 0;	/* index into resource file */

		(void) constructscrfile(0);		/* first pass */
		(void) constructscrfile(1);		/* second pass */
		(void) constructscrfile(2);		/* third pass */		

		if (scaletrouble != 0) 
		fprintf(errout, "WARNING: Some font dimensions may be too large\n");

		if((output = fopen(macfile, "wb")) == NULL) {
			perror(macfile); exit(3);
		}
		writescrfile(output, scrindex);	/* assumes scrindex gives length */

		if (ferror(output) != 0) {
			perror(macfile); exit(11);
		}
		else fclose(output);
		if (verboseflag != 0) putc('\n', stdout);
/*	} */

/*	for (k = 0; k < 4; k++) {
		if (kernpairs[k] != NULL) _ffree(kernpairs[k]);
	}

	if (scrfile != NULL) _ffree(scrfile);	*/
	return 0;
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
//	char macfile[FILENAME_MAX]="";
/*	FILE *input, *output; */
/*	unsigned long hash; */
/*	int ksp, ksu, temp; */
	int k, l, m;
	int firstarg = 1;
/*	time_t ltime; */
/*	int lowstyle; */
	char *s;
/*	char buffer[CHARNAME_MAX]; */
	
	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("VECPATH")) != NULL) vectorpath = s;
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("VECPATH")) != NULL) vectorpath = s;
	}

	strncpy(programpath, argv[0], sizeof(programpath));
	removepath(programpath);

	if (argc < 2) showusage(argv[0]);
	
	for (k = 0; k < 4; k++) NFNTResID[k] = 0;
	for (k = 0; k < 4; k++) NFNTResInit[k] = 0;
	
	style = 0;							/* for NFNTResID */
	firstarg = commandline(argc, argv, 1);
	style = 0;							/* reset again */

	if (spacifyflag != 0 && texturify != 0)
		printf("WARNING: flags `s' and `x' conflict\n");

/*	if (texturify != 0) suppressencoding = 0; */ 	/* 1992/July/2 */

	if (firstarg >= argc) showusage(argv[0]);

	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

/* printf("AFMtoSCR font metrics conversion program version %s\n", VERSION);*/

/*	(void) time(&ltime); 	srand((unsigned int) ltime); */

	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASH %ld", checkcopyright(copyright)); */
		exit(1);
	}

	if(strcmp(scrvector, "") == 0) {		/* new - use default */
/*		strcpy(scrvector, defaultscrvector); */
		scrvector = defaultscrvector;
		reencodeflag = 1;					/* well, could be `none' ... */
	}
	if (strcmp(scrvector, "none") == 0) {	/* use AFM encoding */
/*		strcpy(scrvector, ""); */
		scrvector = "";
		reencodeflag = 0;
		usemacencoding = 0;	
		if (verboseflag != 0) printf("Using encoding in AFM file\n");
	}
	if (strcmp(scrvector, "mac") == 0) {	/* use Mac encoding */ /* new */
		usemacencoding = 1;
		reencodeflag = 0;					/* not needed */
	}

	if(strcmp(pfbvector, "") == 0) {			/* new - use default */
/*		strcpy(pfbvector, defaultpfbvector); */
		pfbvector = defaultpfbvector;
	}
	if (strcmp(pfbvector, "none") == 0) {	/* not known */
/*		strcpy(pfbvector, ""); */
		pfbvector = "";
		resetencoding(pfbencoding);
	}
	else if (usemacencoding != 0) { 	/* will it actually be used ? */
		printf("NOTE: Ignoring outline font file encoding (%s)\n", pfbvector);
/*		strcpy(pfbvector, ""); */
		pfbvector = "";
		resetencoding(pfbencoding);
	}
	else {								/* will actually be used */
		readencoding(pfbvector, pfbencoding);
/*		outlinediffer = 1; */			/* show only differences */
		outlineinit = 1;				/* show only differences */
/*		printf ("OUTLINEDIFFER pfbencoding\n"); */	/* DEBUGGING */
	}


/*	check whether outline font already has desired enconding */
	if (strcmp(scrvector, pfbvector) == 0) { /* no need to reencode */
		if (strcmp(scrvector, "") != 0) {
			if (strcmp(scrvector, "mac") != 0) suppressencoding = 1;
		}
		else if (strcmp(scrvector, "") == 0) {	/* neither encoding given */
	printf("NOTE: Assuming outline font has same encoding as AFM file\n");
			suppressencoding = 1;
		}
	}

/*	if (fixposition != 0 && argc > firstarg && 
		desktoph != -1 && desktopv != -1) {
		fprintf(errout, 
		"WARNING: Attempt to fix position of multiple files on desktop\n");
		fixposition = 0;
	} */

/*	if (checkcopyright(copyright) != 0) {
		fprintf(errout, "HASH %ld", checkcopyright(copyright));  
		exit(1);
	} */

	scrfile = NULL;
	for (k = 0; k < 4; k++) kernpairs[k] = NULL;

	maxscrfile = MAXSCRFILE;
	maxkernsone = MAXKERNSONE;

	if (doallocations (argc, firstarg) == 0) {				/* 1994/June/27 */
		printf ("NOTE: Reducing screen file buffer size\n");
		maxscrfile = maxscrfile / 2;
		freeallocation ();
		if (doallocations (argc, firstarg) == 0) {
			printf ("NOTE: Reducing kern table buffer size\n");
			maxkernsone = maxkernsone / 2;
			freeallocation ();
			if (doallocations (argc, firstarg) == 0) {
				fprintf(errout, "ERROR: Unable to allocate enough memory\n");
				exit(1);
			}
		}
	}
	
/* if (argc - firstarg) == 1 ==> just have one font (assumed regular) */
/* if (argc - firstarg) == 2 ==> have regular plus italic */	
/* if (argc - firstarg) == 2 ==> have regular plus bold ??? */	
/* if (argc - firstarg) == 4 ==> regular, italic, bold, bolditalic */	

	if (wildcards) nstyles = 1;
	else {
		nstyles = argc - firstarg;
		if (nstyles == 0) {
			fprintf(errout, "ERROR: No styles (AFM files) specified\n");
			exit(7);
		}
		if (nstyles > 4) {
			fprintf(errout, "ERROR: Specified more than 4 styles (%d AFM files)\n",
					nstyles);
			exit(9);
		}
		if (nstyles != 1 && nstyles != 2 && nstyles != 4)
			fprintf(errout, "WARNING: Unusual number of styles (%d AFM files)\n",
					nstyles);
		
		for (m = firstarg; m < argc; m++) {
			for (l = firstarg; l < m; l++) {
				if (strcmp(argv[m], argv[l]) == 0) {
					fprintf(errout, 
							"ERROR: AFM file `%s' appears twice in arg list\n", 
							argv[m]);
					exit(43);
				}
			}
		}
	}

	if (wildcards) {
		for (m = firstarg; m < argc; m++)
			dostyles(argv, m, 1);				/* new 97/Aug/25 */
	}
	else dostyles(argv, firstarg, nstyles);		/* split off 97/Aug/25 */

#ifndef _WIN32
	if ((m = _fheapchk ()) != _HEAPOK) {		/* 1994/Feb/18 */
		fprintf(errout, "WARNING: Far heap corrupted (%d)\n", m);
		exit(1);
	}
#endif

	freeallocation ();

/*	if (argc - firstarg > 1)  */
	if (nstyles > 1)
		printf("Processed %d AFM files\n", argc - firstarg);
	return 0;
}
	
/* MacBinary header: */
	
/*	0	 1 BYTE		Zero	*/
/*	1	64 BYTES	File Name -  Pascal string format */
/*	65	 4 BYTES 	File Type	 (no length) */
/*	69	 4 BYTES	File Creator (no length) */
/*	73	 1 BYTE		File Flags */
/*	74	 1 BYTE		Zero */
/*	75	 6 BYTES	DeskTop Location */
/*  81	 1 BYTE		Protected Flag */
/*	82	 1 BYTE		Zero */
/*	83	 LONG		Data Fork Length	*/
/*	87	 LONG		Resource Fork Length	*/
/*	91	 LONG		Creation Date	*/
/*	95	 LONG		Modification Date	*/
	
/* desktop position -1 -1 is special => arrange neatly */

/* desktop positions multiples of 64 maybe ? */

/* fix up resource fork length at end (offset 87) */

/* each resource data section is preceded by four byte length */
/* the length is of the resource data itself, not data + length bytes */

/* resourcedata[k] is w.r.t start of resource */

/* Family number must be the same as the Font number */
/* Family name must be the same as the Font Name */ 

/* may make style dependent on ItalicAngle and Weight */

/* sort out the resource ID question */

/* allow for CM remapping - introduce fake encoding and width entries */

/* allow specification of encoding vector ? */

/* is base encoding vector what is in font file, NOT Mac vector ? */

/* There is one FOND resource for each typeface. */
/* It's resource ID is the same as the font Family ID, */
/* and its name is the name of the typeface */

/* Typefaces can have font ID's up to 16383 */

/* Range 256 - 1023 reserved for renumbered typefaces */

/* It's the FOND resource that gives a typeface its name and font ID */

/* NFNT's can have arbitrary resource ID's since they are refer through FAT */

/* `offset' is the distance from the leftmost black pixel to the leftmost
   left-sidebearing marker of the entire NFNT */ 

/* `width' is the `set width of the character, equal to the distance between
   the left and right sidebearing markers */

/* `location' is the offset to the character's bitmap in NFNT bitmap image */

/* Entries in FAT must remain sorted */ /* not sure it matters */
/* smaller NFNT point sizes come first, and  */
/* within a point size style codes listed in increasing order */
/* not sure the latter matters */

/* don't need to construct height table, Font D/A mover does it for you */

/* allow floats for widths and kern in AFM ? */

/* we read the files in the order specified on the command line */
/* but then use them in style number order */

/* check that all encodings agree with one another ??? */

/* Some rules about splitting FontNames: */

/* BaseName _can_ be full FontName */
/* Hyphen usually  breaks up FontName, but need not --- */
/* --- since suffix _can_ start with `-' e.g. -BoldItalic */
/* BaseName _can_ contain more than one `component' e.g. NewCenturySchlbk */
/* Suffix _can_ contain more than one `component' e.g. MediumItalic */
/* Suffix Index list can be empty or contains a list of zeros */

/* mac-encode means outline font needs to be reencoded on way to print */
/* use mac encoding vector (difference from StandardEncoding only?) */
/* non-mac-encode means outline font needs to be reencoded on way to print */
/* use specified mapping fragment for changing encoding */

/* If asked to use AFM encoding, why not just force not use non-mac-encode */
/* If given only one style why not force it into `regular' style position */

/*	Table of possible encoding vector combinations and action taken

scr vector		pfb vector		relative encode		mac-encode	non-mac-encode
----------		----------		---------------		----------	--------------

mac				<ignored>		.					YES			.

vector		=	vector			.					.			.

s-vector		p-vector		YES					.			YES

s-vector		. (unknown)		YES, FULL			.			YES

. (use AFM)		p-vector		YES					.			YES

. (use AFM)		. (unknown)		.					.			.

(In last case the assumption is that outline font is encoding same as AFM)

*/

/* PS file output to printer:

mac-encode		non-mac-encode

.				.				no vector, F (non-Mac)
.				X				vector included, F (non-Mac)
X				.				no vector, T (Mac)
X				X				vector, T (Mac) => PostScript error

*/

/* width table has two extra entries: width of default character & 0 */

/* this DATE nonsense: should really use `timediffer' and reference */
/* major screw when compiler/library decides to change time origin !!! */

/* Modify to read INF file for auxiliary information if INF file exists ? */

/* need two `vers' resources ala PFBtoMAC ? */ /*  version resources ? */

/* possible problem with zero width characters ? */

/* allocation is once per job, not once per font */
/* remember all kern tables have to fit in SCR output file */
/* so limitation on SCR output file size limits kerns implicitly */
/* could save space by only allocating kern tables as needed */

/*	if ((FONDFlags & 0X0001) != 0) printf("Image-Heights ");
	if ((FONDFlags & 0X0002) != 0) printf("Image-Widths ");
	if ((FONDFlags & 0X1000) != 0) printf("Ignore-FractEnable ");
	if ((FONDFlags & 0X2000) != 0) printf("Integer-Extra ");
	if ((FONDFlags & 0X4000) != 0) printf("Fractional-Unused ");
	if ((FONDFlags & 0X8000) != 0) printf("Fixed-Width ");
*/

/* FOND ID 17920 - 18431 Arabic */
/* FOND ID 18432 - 18943 Hebrew */
/* FOND ID 18944 - 19455 Greek */
/* FOND ID 19456 - 19967 Cyrillic */

/* Undocumented -Z `wild card' mode 97/Aug/25 */

/*
	Flags in a FOND font resource:

	Bit 0
	This bit is set to 1 if the font name needs coordinating.

	Bit 1
	This bit is set to 1 if the Macintosh vector reencoding scheme is required.
	Some glyphs in the Apple character set, such as the Apple glyph, do not
	occur in the standard Adobe character set. This glyph must be mapped in
	from a font that has it, such as the Symbol font, to a font that does not,
	like Helvetica.

	Bit 2
	This bit is set to 1 if the font family creates the outline style by changing
	PaintType, a PostScript variable, to 2.

	Bit 3
	This bit is set to 1 if the font family disallows simulating the outline style
	by smearing the glyph and whiting out the middle.

	Bit 4
	This bit is set to 1 if the font family does not allow simulation of the bold
	style by smearing the glyphs.

	Bit 5
	This bit is set to 1 if the font family simulates the bold style by increasing
	point size.

	Bit 6
	This bit is set to 1 if the font family disallows simulating the italic style.

	Bit 7
	This bit is set to 1 if the font family disallows automatic simulation of the
	condense style.

	Bit 8
	This bit is set to 1 if the font family disallows automatic simulation of the
	extend style.

	Bit 9
	This bit is set to 1 if the font family requires reencoding other than
	Macintosh vector encoding, in which case the glyph-encoding table is
	present.

	Bit 10
	This bit is set to 1 if the font family should have no additional intercharacter
	spacing other than the space character.

	© 1995 by Apple Computer, Inc. All rights reserved.
*/
