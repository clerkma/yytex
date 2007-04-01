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

/* Program for turning AFM file into PFM file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <time.h>

#pragma warning(disable:4032)	// different type when promoted
#include <conio.h>
#pragma warning(default:4032)	// different type when promoted

/* #include <assert.h> */

/* #define DEBUG */

#ifdef _WIN32
#define __far
#define _frealloc realloc
#define _fmalloc malloc
#define _ffree free
#endif

/* #define FNAMELEN 80 */
/* #define MAXLINE 256 */
#define MAXLINE 512
/* #define MAXKERNS 512 */
/* #define MAXKERNS 1240 */
/* #define MAXKERNS 1500 */
#define MAXKERNS 10000		/* can't make more than about 11000 in 16 bit */

/* #define MAXBUFFER 4096 */
/* #define MAXBUFFER 8192 */		/* if allocated in far space */
#define INIMEMORY 4000U				/* do this in stages ? realloc ? */
#define INCMEMORY 4000U				/* do this in stages ? realloc ? */

#define MAXCHRS 256
#define MAXCHARNAME 32
#define MAXCOPYRIGHT 60				/* maximum length of copyright (66 - 6) includes null */
#define UNKNOWNWIDTH 32767			/* marker for empty slot in encoding */

int buffersize=0;			/* current size of PFM output buffer */

/* UNKNOWNWIDTH is impossible width mark for character */

int firstchar=0;			/* default for background */
int lastchar=255;			/* default for background */
int defaultchar=149;		/* int defaultchar=32; ? */
int breakchar=32;			/* 0 ??? */

int charmin, charmax;
int averwidth, defaultwidth;

int copyrightflag;			/* if non-zero copyright string has been set */

/* char *programversion = "AFMtoPFM conversion utility version 1.1"; */
/* char *programversion = "AFMtoPFM conversion utility version 1.2"; */
/* char *programversion = "AFMtoPFM conversion utility version 1.5"; */
/* char *programversion = "AFMtoPFM conversion utility version 1.7"; */
/* char *programversion = "AFMtoPFM conversion utility version 1.8"; */
/* char *programversion = "AFMtoPFM conversion utility version 1.8.1"; */
char *programversion =
/*	"AFMtoPFM conversion utility version 1.9"; */ /* 95/Jan/5 */
/*	"AFMtoPFM conversion utility version 1.9.1"; */ /* 95/Dec/31 */
/*	"AFMtoPFM conversion utility version 1.9.2"; */ /* 96/Mar/3 */
/*	"AFMtoPFM conversion utility version 1.9.3"; */ /* 96/June/16 */
/*	"AFMtoPFM conversion utility version 1.9.4"; */ /* 96/June/12 */
/*  "AFMtoPFM conversion utility version 1.9.5"; */ /* 97/Jan/18 */
/*  "AFMtoPFM conversion utility version 1.9.6"; */ /* 97/June/28 */
#ifdef _WIN32
/* "AFMtoPFM (32) conversion utility version 2.0"; *//* 98/Mar/10 */
/* "AFMtoPFM (32) conversion utility version 2.0.2"; */ /* 98/Jun/30 */
/* "AFMtoPFM (32) conversion utility version 2.0.3"; */ /* 98/Aug/28 */
"AFMtoPFM (32) conversion utility version 2.0.4"; /* 00/Jun/15 */
#else
"AFMtoPFM conversion utility version 1.9.7"; /* 97/Dec/26 */
#endif

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* Copyright can be picked out of AFM file for insertion in PFM file */

/* or: for insertion into PFM file  -  limit 60 characters */

/* char *yanycopyright = "\
Copyright (C) 1991 -- 1996, Y&Y, Inc. All rights reserved."; */

/* year is at offset 14, replaced by current year upon entry */

/* int usenewcopy = 1; */

char yanycopyright[MAXCOPYRIGHT] = "\
Copyright (C) 1998 Jun 16, Y&Y, Inc. All Rights Reserved.";	/* 58 chars */

char newcopyright[MAXCOPYRIGHT] = "\
Copyright 1998 Y&Y Inc. (978) 371-3286 http://www.YandY.com"; /* 60 chars */

/* Copyright (C) 1996, Y&Y, Inc. All rights reserved.";	*/  /* 51 chars */

/* struct kernpair {
	union {
		unsigned char each[2];
		unsigned int both;
	} kppair;
	int kpamount;
}; */

#ifdef _WIN32
   struct kernpair { 
	unsigned short kppair; short kpamount;
};
#else
   struct kernpair { 
	unsigned int kppair; int kpamount;
};
#endif

/* static struct kernpair kernpairtable[MAXKERNS * 2]; */
/* static struct kernpair kernpairtable[MAXKERNS]; */

struct kernpair kernpairtable[MAXKERNS];	/* large - maybe alloc memory */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int noteabsence=0;		/* warn if character in AFM, not in encoding */

int whimpflag = 0;		/* ansi => 32 - 255 */

int spaceflag = 0;		/* flags used while interpreting command line */
int vectorflag = 0;
int defaultflag = 0;
int pscriptflag = 0;	/* next argument is width to use instead of zero */
int descendflag = 0;
int nameflag = 0;		/* next arg is Windows Face Name */
int fontnameflag = 0;	/* next arg is PS FontName to use instead of AFM */
int sourceflag = 0;
int outputpathflag=0;	/* next arg is path for output TFM file 95/Jan/2 */
int charsetflag=0;
int familyflag=0;

int windowsflag= 0;		/* if windows name is given on command line */
						/* or file contains "Comment MS-WindowsName" */
int underflag = 0;		/* force extend name with underscore */
int uppercaseflag = 0;	/* force FontName to be uppercase */
int flushzeros = 1;		/* flush zero size kerns */
int wantkerns = 1;		/* want kerning pairs */
/* int usewidthofx = 1; */	/* use width of x for character average */
int usewidthofx = 0;	/* use width of x for character average 96/Aug/28 */
/* int usebboxinfo = 0; */	/* use FontBBox information */

int minpscript=0;		/* minimum width allowed for PSCRIPT.DRV bug */
int mindescend=0;		/* lowest descender allowed (for ATM clipping) */

int suppressitalic = 0;	/* suppress `italic' style even ItalicAngle != 0 */
int suppressbold = 0;	/* suppress `bold' style even if Weight > 400 */

int automatetest=1;		/* automate detection of text font 98/Jun/30 */
int truncatecopy=0;		/* just truncate the copyright string 98/Aug/28 */

char *copyright = "\
Copyright (C) 1990--2000  Y&Y, Inc.  http://www.YandY.com\
";

/* Copyright (C) 1990--1997  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 2866797 */
/* #define COPYHASH 2719644 */
/* #define COPYHASH 8124949 */
/* #define COPYHASH 14008178 */
/* #define COPYHASH 8997420 */
/* #define COPYHASH 14880649 */
/* #define COPYHASH 11957132 */
/* #define COPYHASH 5001165 */
#define COPYHASH 5190533

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

int wantcpyrght=1;		/* want copyright message in there */

int originflag = 0;		/* used to override copyright message */

int usenewcopy = 1;		/* use phone number in copyright 96/Jun/20 */

/* Does ATM use FamilyName for Windows Face Name or strip down FontName ? */
/* actually, it uses INF file information for this ... */

int usefamilyname = 0;	/* non-zero => use family name for MS Windows name */
int usefullname = 0;	/* non-zero => use full name - otherwise fontname */
int shortenflag = 1;	/* remove "bold" and "italic" from name */
					    /* remove "roman" and "regular" from name */
int stripprefixflag=1;	/* remove Acrobat six letter prefix 97/June/25 */
int insisthyphen = 0;	/* insist hyphen or space before name component */

int remapflag = 0;		/* extend width table for TeX - remapping */

int remapflagini = 0;	/* extend width table for TeX - remapping initial */

int shiftdown = 0;		/* move down so starts at 32 if remapflag specified */

/* int relativeflag = 1; */	/* non-zero means default and break wrt first */

int signature = 1;		/* include author's signature */

int hideencoding = 1;	/* include encoding vector description ! */

int startlate = 0;		/* let 32 be first character (if none lower) */

int reencodeflag = 0;	/* if encoding vector is given */		/* not used */
int standardflag = 0;   /* reencode to standard => symbolflag *//* not used */

int boldflag = 0; 		/* bold font */		/* not used */
int italicflag = 0; 	/* italic font */	/* not used */

int textfontflag=1;		/* EncodingScheme AdobeStandardEncoding */

char *sourcefile="";				/* old PFM file to get name from */

char *outputpath="";				/* directory for PFM file 1995/Jan/5 */

char *defaultencoding = "none";

int useafmencoding = 1;	/* non-zero means read encoding in AFM file */
int texflag = 0;		/* non-zero means TeX font encoding */
						/* firstchar = 0 */ /* breakchar = 160 */

int serifflag = 1;		/* non-zero means assume serifed font */
int decorative = 0;		/* non-zero means assume decorative - piflag */
int fixedflag = 0;		/* non-zero means assume fixed width font ? */

int Family =  0;		/*  set explicitly */

/* in case of TeX font, it figures all this out itself */

/* mutually exclusive - one and only one of these should be on */
int ansiflag = 1;		/* non-zero means use ANSI CharSet */
int symbolflag = 0;		/* non-zero means use Symbol CharSet */
int oemflag = 0;		/* non-zero means use OEM CharSet */
int arabicflag = 0;		/* non-zero means use Arabic CharSet */
int CEflag = 0;			/* non-zero means use Central European CharSet */

int CharSet = 0;		/* allow explicit setting 97/June/5 */

int dontexten = 0;		/* don't extend reencoded with `x' */

int removeX = 0;		/* remove `x' on output file name */

int XFaceFlag = 0;		/* extend Face Name with `X' */

/* for PitchandFamily: */ /* no longer that simple */
/* if ANSI flag off, `decorative' font flagged otherwise serif or non-serif */

/* encoding default is determined by above - but can be overwritten 'e' */

unsigned int ptsize;		/* design point size */

int smallcaps;				/* smallcaps if non-zero */
int notencoded;				/* count unencoded characters */
int total;					/* total number of glyphs 96/June/16 */
int notkernpairs;			/* kern pairs discarded */

int weight=0;				/* non-zero if weight given in command line */

double spacewidth;			/* specified width of space */

double xscale=1.0;			/* aspect ratio of condensed or expanded */

int texspace;				/* from Comment Space <a> <b> <c> line in AFM */

FILE *errout=stdout;		/* 1997/Sep/23 */

/* char windowsname[32+1]=""; */	/* MS windows face name to use */
char windowsname[64+1]="";	/* MS windows face name to use - just to be safe */

char newfontname[64+1]="";	/* PS FontName to use in Driver Info */

#define MAXENCODING 40				/* compatible with TFm files */

/* char *commandvector="";	*/	/* vector specified on command line */

char encodingvecname[MAXENCODING]="";	/* first line encoding vector file */

char codingvector[MAXENCODING]="";	/* command line coding scheme in use */

char defaultname[MAXENCODING]="";	/* 20 default char command line */

char *defaultchars[] = {
"bullet", "openbullet",
"period", "periodcentered", "middot", 
"circledottext", 
"asterisk", "star", "lozenge", "diamond", 
"suppress", "ring",
"visiblespace", "space", "cwm",
"times", "multiply", 
"afii10080",					/* `o' for BaKoMa if `space' is lacking */
""};

char programpath[FILENAME_MAX] = "";

/* char *vecpath = "c:\\dvipsone"; */	/* default encoding vectors paths */

char *vecpath = "c:\\yandy\\fonts\\encoding";

/* static unsigned char buffer[MAXBUFFER]; */
/* char buffer[MAXBUFFER]; */
char __far *buffer=NULL;			/* place for constructing PFM */ 

/* static char line[MAXLINE]; */
char line[MAXLINE];

/* static char encoding[MAXCHRS][MAXCHARNAME]; */
char encoding[MAXCHRS][MAXCHARNAME];	/* large - maybe alloc memory */

/* encoding vectors are now always read in rather than part of program */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* encoding vector file name => verbose name for encoding */

/* unsigned char *encodingnames[17][2] = { */
/* unsigned char *encodingnames[][2] = { */
char *encodingnames[][2] = {
{"TeX text", "textext"},					/* cmr*, cmb*, cmsl*, cmss* */
{"TeX text italic", "texital"},					/* for	cmti* */
{"TeX text without f-ligatures", "textype"},	/* cmcsc10 & cmr5 */
{"TeX typewriter text", "typewrit"},		/* cm*tt* */
{"TeX typewriter text italic", "typeital"},		/* for  cmitt* */
{"TeX extended ASCII", "texascii"},			/* cmtex* */
{"TeX math italic", "mathit"},				/* cmmi* */
{"TeX math symbols", "mathsy"},				/* cmsy* */
{"TeX math extension", "mathex"},			/* cmex10 */
{"AdobeStandardEncoding", "standard"},
{"Adobe StandardEncoding", "standard"},
{"Adobe Symbol Encoding", "symbol"},
{"Adobe Dingbats Encoding", "dingbats"},
{"MicroSoft Windows ANSI 3.0", "ansi"},
{"MicroSoft Windows ANSI 3.1", "ansinew"},
{"Ventura Publisher Encoding", "ventura"},
{"TeX 256 character Encoding", "tex256"},
{"Extended TeX Font Encoding - Latin", "cork"},
{"TeX text companion symbols 1---TS1", "ts1"},
/* {"Default TeX Encoding", "default"}, */
{"Macintosh", "mac"},
/* {"AT&T Garamond Encoding", "atandt"}, */
/* {"ASCII caps and digits", "textext"}, */		/* cminch */
/* {"TEX TEXT IN ADOBE SETTING", "textext"}	*/ /* pctex style ? */
{"TEX TEXT + ADOBESTANDARDENCODING", "neonnew"}, /* 1992/Dec/22 */
{"TeX typewriter and Windows ANSI", "texnansi"},			/* 1994/Dec/31 */
{"", ""}
 };	
  
char charcodingscheme[MAXENCODING];	/* from AFM file - or vector */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/**********************************************************************/

/* int allowaliases=1; */
int allowaliases=0;						/* new defaults 95/Dec/31 */

char *charaliases[][2] = {
{"sfthyphen", "hyphen"},
{"nbspace", "space"},
{"brokenbar", "bar"},
{"", ""}
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* write one byte numbers, two byte numbers, three ... */

void expandbuffer (void) {		/* expand PFM file output buffer */
	char __far *newbuffer;
	unsigned int newsize;
	
	newsize = buffersize + INCMEMORY;
	newbuffer = (char __far *) _frealloc (buffer, newsize);
	if (newbuffer == NULL) {
		fprintf(stderr,
			"ERROR: Unable to allocate more memory for PFM file\n");
		exit(1);
	}
	else {
		buffer = newbuffer;
		buffersize = newsize;
		if (traceflag)			/* debugging */
			printf("Expanded PFM file buffer to %u\n", newsize);
	}
}

void writetwo (unsigned int x, int n) {			/* write unsigned int */
	if (n + 1 >= buffersize) expandbuffer();
	*(buffer + n) = (unsigned char) (x & 255); 
	*(buffer + n + 1) = (unsigned char) (x >> 8); 
}

void writefour(unsigned long x, int n) {		/* write unsigned long */
	if (n + 3 >= buffersize) expandbuffer();
	*(buffer + n) = (unsigned char) (x & 255);
	*(buffer + n + 1) = (unsigned char) (x >> 8);
	*(buffer + n + 2) = (unsigned char) (x >> 16);
	*(buffer + n + 3) = (unsigned char) (x >> 24);
}

unsigned int readtwo(int n) {					/* read unsigned int */
/*	return ((unsigned int) *(buffer + n)) |
		   (((unsigned int) *(buffer + n +1)) << 8); */
	return ((unsigned char) *(buffer + n)) |
		   (((unsigned char) *(buffer + n +1)) << 8); 
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* write the PFM file */

void writepfm(char *pfm, int pfmlen) {
	FILE *output;
	char fn_out[FILENAME_MAX];
/*	unsigned char *s; */
/*	char *s; */
	char __far *s;
	char *t;  
	int k;

	writefour((unsigned long) pfmlen, 2);	/* fill in `Size' */

	if ((t = strrchr(pfm, '\\')) != NULL) t++;
	else if ((t = strrchr(pfm, ':')) != NULL) t++;
	else t = pfm;
	strcpy(fn_out, t);

	if ((output = fopen(fn_out, "wb")) == NULL) {
		perror(fn_out); exit(1);
	}

	s = buffer;
	for (k = 0; k < pfmlen; k++) putc(*s++, output);

	if (ferror(output)) perror(fn_out);
	else fclose(output);
}

#ifdef IGNORE
void extension(char *fname, char *ext) { /* supply extension if none */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

/* change extension if present 
void forceexten(char *fname, char *ext) { 
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);
} */

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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void lowercase (char *s, char *t) {
	int c;
	while ((c = *s++) != '\0') {
		if (c >= 'A' && c <= 'Z') *t++ = (char) (c + 'a' - 'A');
		else *t++ = (char) c;
	}
	*t = '\0';
}

void uppercase (char *s, char *t) {
	int c;
	while ((c = *s++) != '\0') {
		if (c >= 'a' && c <= 'a') *t++ = (char) (c + 'A' - 'a');
		else *t++ = (char) c;
	}
	*t = '\0';
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* NOTE: strncpy actually pads out with zeros, this does not */

void lstrncpy(char __far *s, const char *t, int n) {
	int k, i;
	for (k = 0; k < n; k++) {
		if ((*s++ = *t++) == '\0') break;
	}
	for (i = k+1; i < n; i++) *s++ = '\0';	/* pad out with zeros */
}

void lstrcpy(char __far *s, const char *t) {
	while ((*s++ = *t++) != '\0');
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stick in the standard stuff first as background texture ... */
/* much of it is later overwritten IF the information is available */

/* Header structure described in: MS Windows Device Driver Adaptation Guide */

void preparebuffer (void) {
	int k, code, superscript;
	int strikeoutposition, strikeoutwidth, capheight, xheight;

/*	for (k = 0; k < MAXBUFFER; k++) *(buffer + k) = '\0'; */  /* zeros */
	for (k = 0; k < buffersize; k++) *(buffer + k) = '\0';  /* zeros */

/*  This is the beginning of the PFM header */
	*buffer = 0; *(buffer +1) = 1;		/* `Version' = 01 */
/*	next four ? bytes are `Size' - length of pfm file - filled in at end */
/*	then comes `Copyright' message (60 bytes) - put in later from AFM */
/*	Default copyright if nothing found in the AFM file */
	if (! copyrightflag) {
		if (usenewcopy) lstrncpy(buffer+6, newcopyright, MAXCOPYRIGHT); 
		else lstrncpy(buffer+6, yanycopyright, MAXCOPYRIGHT);
		*(buffer+6+MAXCOPYRIGHT-1) = '\0';	/* null terminate it for convenience */
#ifdef DEBUG
//		if (verboseflag) printf("BUFFER+6 %s %s\n", buffer+6, "preparebuffer");
		if (verboseflag) printf("Copyright string (%d): '%s'\n", strlen(buffer+6), buffer+6);
#endif
	}
	writetwo(129, 66); /* `Type' - whatever that is */
	writetwo(10, 68);  /* `Points' - ignored by PS driver */
	writetwo(300, 70); /* `VertRes' - assumed 300 by PS driver */
	writetwo(300, 72); /* `HorizRes' */
/*	Ascent overwritten later by FontBBox.yur ... */
	if (smallcaps != 0) writetwo(667, 74);	/* `Ascent' ? */
	else writetwo(700, 74);					/* `Ascent' ? */
/*  Leading that appears in the height specified for the font */
	writetwo(0, 76);		/* `InternalLeading' - later ? */
/*  Additional space to insert between lines */
	writetwo(0, 78);		/* `ExternalLeading' - later ? */
/*	writetwo(196, 78); */	/* `ExternalLeading' */ 
/*  Adobe Fonts always have the above set to zero */
/*	suitable values appear to be calculated inside Windows later */
/*  ATM or MS windows calculates a value here of 196 or 0 */
/*  then comes `Italic' flag - code for Italic in name */
/*  then comes `Underline' flag */
/*  then comes `Strikeout' flag */
/*  fill these in later if needed - else leave at zero */
	writetwo(400, 83);  /* normal `Weight' - code for Bold, Regular, Light */
/*	next comes `CharSet': */
	
	if (ansiflag != 0) code = 0;			/* ANSI encoding */
	else if (symbolflag != 0) code = 2;	/* Symbol encoding */ 
	else if (arabicflag != 0) code = 180; /* Arabic encoding */
	else if (CEflag != 0) code = 238; /* Central European */
	else if (oemflag != 0) code = 255; /* OEM encoding */
	else code = 1;						/* Don't care */
/*	else fprintf(errout, "No CharSet selected!"); */

/*	if (useafmencoding) code = 2; */ /* or 0 */
	if (CharSet != 0) code = (char) CharSet; /* explicit */

	*(buffer +85) = (char) code;
/*  many other possible values for above when using PCL font */

	writetwo(0, 86);		/* `PixWidth'  - zero for PS font */
/*  next comes `PixHeight' - ignored by PS driver */
	writetwo(1000, 88);		/* `PixHeight' */ 
/*	either use PixHeight or Ascent + External Leading ? */

/*  next comes `PitchAndFamily': */
/*	if (ansiflag != 0) { */

	if (decorative != 0) code = 5;
	else if (fixedflag != 0) code = 3; 	/* fixed width font `modern' */
	else if (serifflag != 0) code = 1;	/* serif font */
	else code = 2;						/* sans serif font `swiss' */

	if (useafmencoding) code = 5;		/* too early ? 98/Aug/17 */
	if (Family != 0) code = Family;		/* direct setting 97/June/5 */

	*(buffer +90) = (char) ((code << 4) | 1);

/*	then comes `AvgWidth' (average width) - often just width of `x' */
/*  then comes `MaxWidth' (maximum width) */

/* following gets overwritten later anyway ... */

	if (startlate != 0 && firstchar >= 32) firstchar = 32;
	if (texflag != 0) firstchar = 0;
	
	*(buffer +95) = (unsigned char) firstchar;		/* `FirstChar' */
	*(buffer +96) = (unsigned char) lastchar;		/* `LastChar' */	
/*	if (relativeflag != 0)  */	/* may overwrite following later */
	*(buffer +97) = (unsigned char) (defaultchar - firstchar);
/* `DefaultChar' */
	*(buffer +98) = (unsigned char) (breakchar - firstchar);
/*	if (verboseflag)
		printf("Using %s (%d) as default character\n",
			   encoding[defaultchar], defaultchar); */
/* `BreakChar' */
/*	possible bug in ATM:  always adds 32 to the above, not firstchar ? NO */
/*	was w.r.t. 32, no is w.r.t charmin */
/*	} else { 
		*(buffer +97) = (unsigned char) defaultchar;	
		*(buffer +98) = (unsigned char) breakchar;
	} */
/*  ATM may ignore the above anyway, has break and default wired in ? */
/*  next comes `WidthBytes' (WORD) */
	writefour((unsigned long) 199, 101);
							/* `Device' - pointer to drivertype field */
	writefour((unsigned long) 210, 105);
							/* `Face' - pointer to face name field */
/*  next comes `BitsPointer' and `BitsOffset' DWORDs */
/*  this is the end of  PFM Header */
/*  there is no width table since this is a PostScript font */

/*  This is the beginning of the PFMEXTENSION data structure */
/*  Described in MS Windows Device Development Kit: Printers and Fonts Kit */
	writetwo(30, 117);		/* `SizeFields' - Length of PFMEXTENSION */
	writetwo(147, 119);		/* `ExtMetricsOffset' pointer to EXTTEXTMETRICS */
/*	next is `ExtentTable' (DWORD) pointer to character escapement table */
/*	required for PostScript fonts - filled in later */
/*  next is `OriginTable' (DWORD) - for screen fonts ? */
	writetwo(0, 131);		/* `PairKernTable' - kerntable start - later */
/*  next comes `TrackKernTable' (DWORD) - not used */
/*	next comes `DriverInfo' -  pointer to fontname field - later */
/*  next comes `Reserved' (DWORD) - reserved */
/*  This is the end of the PFMEXTENSION structure */

/*  This is the beginning of the EXTTEXTMETRIC data structure */
/*  Described in MS Windows Device Development Kit: Printers and Fonts Kit */
	writetwo(52, 147);		/* `Size' - Length of EXTTEXTMETRIC */
/*	writetwo(240, 149);	 */		/* a twip is a twentieth of a point */
	writetwo(ptsize * 20, 149); /* `PointSize' - Design Size in twips */
	writetwo(0, 151);		/* `Orientation' - portrait or landscape */
/*	writetwo(300, 153); */	/* `MasterHeight' */ /* assumed 300 PS driver */
	writetwo(1000, 153);	/* `MasterHeight' ? */ /* assumed 300 PS driver */
	writetwo(3, 155);		/* `MinScale' in device units at 300 dpi ? */
	writetwo(1000, 157);	/* `MaxScale' in device units at 300 dpi ? */
	writetwo(1000, 159);	/* `MasterUnits' */ /* assumed 1000 PS driver */
/*	next comes capheight, xheight, ascender, descender */
	capheight = 663;
	writetwo(capheight, 161);		/* `Capheight' - height of `H' */
	xheight = 500;
	writetwo(xheight, 163);		/* `XHeight' - height of `x' */
	writetwo(691, 165);		/* `LowerCaseAscent' - top of `d' */
	writetwo(164, 167);		/* `LowerCaseDescent' - bottom of `p' */
/*	the above is just so these things aren't zero - use value from AFM file */
/*  then comes 'Slant' - CW italic angle times ten */
/* 	*(buffer +171) = 12; *(buffer +172) = 254; */ /* `SuperScript' */
	superscript = -500;		/* measured downward ! */
/*	write SIGNED int into buffer: */
	*(buffer +171) = (unsigned char) (superscript & 255);
	*(buffer +172) = (unsigned char) (superscript >> 8);

	writetwo(250, 173);		/* `SubScript' */ /* measured downward ! */
	writetwo(500, 175);		/* `SuperScriptSize' */
	writetwo(500, 177);		/* `SubScriptSize' */
/*	next comes `UnderlineOffset'  and `UnderlineWidth' */
/*	and double underline nonsense - defaults follow: */
	writetwo(100, 179);		/* default UnderlinePosition -100 */
	writetwo(100, 185);
	writetwo(50, 183);
	writetwo(50, 181);		/* default UnderlineThickness 50 */	
	writetwo(25, 187);
	writetwo(25, 189);
	strikeoutwidth = 50;
	strikeoutposition = (capheight - strikeoutwidth) / 2;	/* too early */
/*	if (smallcaps != 0) writetwo(277, 191);	*/
/*	else writetwo(311, 191); */
	writetwo(strikeoutposition, 191); /* StrikeoutPosition */
/*	writetwo(40, 193); */	/* `StrikeoutWidth' */
	writetwo(strikeoutwidth, 193);	/* `StrikeoutWidth' */ 
	writetwo(0, 195);	/* `KernPairs' - fill in later - max is 512 ? */
	writetwo(0, 197);	/* `KernTracks' */
/*	strcpy(buffer+199, "PostScript"); */
	lstrcpy(buffer+199, "PostScript");	/* `Device' field - ptr in 101 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *getrealline(char *s, int maxn, FILE *input) {
	char *p;
	p = fgets(s, maxn, input);
	while ((*s == '%' || *s == ';' || *s == '\n') && p != NULL) 
		p = fgets(s, maxn, input);
	return p;
}

void flushnewline(char *message);

void trimcopyright(char *message);

void flushbolditalic(char *name);

void stripprefix(char *name);

/* comparison function for kerning pairs used in sorting */
/* sort lexicographically - second char more significant */

/* int compare(struct kernpair *kpa, struct kernpair *kpb) { */
/* int compare(const struct kernpair *kpa, const struct kernpair *kpb) { */
int compare(const void *vpa, const void *vpb) {
	const struct kernpair *kpa, *kpb;
	unsigned int botha, bothb;

	kpa = (const struct kernpair *) vpa;
	kpb = (const struct kernpair *) vpb;
	botha = kpa->kppair;
	bothb = kpb->kppair;
	if (bothb < botha) return +1;
	else if (bothb > botha) return -1;
	else return 0;
}

/* int lookup(char *charname) {
	int k;
	for(k = 0; k < MAXCHRS; k++) {
		if (strcmp(charname, encoding[k]) == 0) return k;
	}
	return -1;
} */

/* Starts look up at last + 1 */ /* in most cases last = -1 */

int lookup(char *charname, int last) {
	int k, l;
	char *alias;
	
	for(k = last+1; k < MAXCHRS; k++) {
		if (strcmp(charname, encoding[k]) == 0) return k;
	}
/*	for(k = 0; k < last+1; k++) {
		if (strcmp(charname, encoding[k]) == 0) return k;
	} */						/* NO, can get stuck in loop */
/*	New 1994/April/10 */
	if (allowaliases && last < 0) {
		for (l = 0; l < 16; l++) {
			if (strcmp(charaliases[l][0], "") == 0) break;
			if (strcmp(charaliases[l][0], charname) == 0) {
				alias = charaliases[l][1];
				for (k = 0; k < MAXCHRS; k++) {
					if (strcmp(alias, encoding[k]) == 0) return k;
				}
				break;
			}
		}
	}
	if (noteabsence != 0 && last < 0)
		fprintf(errout, "%s? ", charname);	
	return -1;
}

/* debugging only 
void showencoding(void) {	 
	int k;
	for(k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") != 0) {
			printf("C %d ; N %s ;\n", k, encoding[k]);
		}
	}
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void stripreturn (char *name) {
	int c;
	char *s=name;

	while ((c = *s) != '\0' && c != '\n') s++;
	if (c == '\n') *s = '\0';
}

/* modified 1992/Jan/5 to read first line of encoding vector for Encoding: */

void readencoding (FILE *input) {
	int k, nchar;
	char charname[FILENAME_MAX];
	char *s;

	for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], "");

	strcpy(encodingvecname, "");
	(void) fgets(line, MAXLINE, input);				/* get first line */
	if ((s = strstr(line, "Encoding:")) != NULL) {
		s += strlen("Encoding:");
		while (*s++ <= ' ') ; s--;
		strncpy(encodingvecname, s, MAXENCODING);
		if ((s = strchr(encodingvecname, '\n')) != NULL) *s = '\0';
/*		stripreturn(encodingvecname); */
		if (verboseflag != 0)
			printf("EncodingVectorName: %s\n", encodingvecname); 
		(void) fgets(line, MAXLINE, input);
	}

	for(;;) {
		while (*line == '%' || *line == ';' || *line == '\n') {
			if (fgets(line, MAXLINE, input) == NULL) return;	/* break */
		}
		if (sscanf(line, "%d  %s", &nchar, charname) < 2) {
			fprintf(errout, "Don't understand: %s", line);
		}
		else {
			if (nchar >= 0 && nchar < MAXCHRS) {
				if (strcmp(encoding[nchar], "") != 0)	/* 1993/June/27 */
					fprintf(errout,
						"WARNING: duplicate for %d: `%s' and `%s'\n",
							nchar, encoding[nchar], charname);
				strncpy(encoding[nchar], charname, MAXCHARNAME);
			}
			else 
				fprintf(errout, "Code number out of range %d\n", nchar);
		}
		if (fgets(line, MAXLINE, input) == NULL) return; /* break */
	}
}

int checkduplicates (void) {			/* 1993/June/27 */
	int k, l, flag=0;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") == 0) continue;
		for (l = k+1; l < MAXCHRS; l++) {
			if (strcmp(encoding[l], "") == 0) continue;
			if (strcmp(encoding[k], encoding[l]) == 0) {
				if (verboseflag != 0) fprintf(errout, "%s ", encoding[k]);
				flag++;
			}
		}
	}
	return  flag;
}

/* int readvectorfile(char *vector) { */
int readvectorfile(char *name) {
	FILE *input=NULL;
	char fn_in[FILENAME_MAX];
	int n;
	char *s, *t;

/*	printf("READVECTORFILE %s\n", name); */

/* First try current directory */
/* First try name as given possibly with path */
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
		fprintf(stderr, "ERROR: Can't open encoding vector\n");
		perror(name);
		return -1;
	}

	if (verboseflag != 0) printf("Using encoding vector file %s\n", fn_in);

/*	we DO have a vector file! --- maybe */
	readencoding(input);
	fclose (input);
	reencodeflag = 1; 
	useafmencoding = 0;
	if ((n = checkduplicates()) != 0) {
		fprintf(errout,
/*		"\nWARNING: %d characters appear more than once in encoding\n", */
		"\n%s: %d characters appear more than once in encoding\n",
			(n > 32) ? "WARNING" : "NOTE", n);
	}
	if (remapflag != 0 && strcmp(encoding[160], "") == 0)
		strcpy(encoding[160], "space");		/* typical TeX convention */
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *yandy = "Y&Y ";

int last;		/* points to next available byte in table */

void lowercase(char *, char *);

/* poster compact and other such nonsense default to 400 */

char *weightnames[] = {
	"black", "extrabold", "heavy", "bold",
	"demi", "semi", "demibold", "semibold",
	"medium", "plain", "regular", "standard", "roman", "normal", "book",
	"light", "thin", "ultralight", ""
};

int weightvalues[] = {
	700, 700, 700, 700,
	700, 700, 700, 700,
	400, 400, 400, 400, 400, 400, 400,
	250, 250, 250, 0
};

int analyzeweight(char *s, char *line) {
	char str[128];
	int k;
	int weight=400;			/* default */

/*	sscanf(s, "%s", str); */
	strcpy(str, s);											/* 98/Aug/26 */
	if ((s = strchr(str, '\n')) != NULL) *s = '\0';			/* 98/Aug/26 */
	while ((s = strchr(str, ' ')) != NULL) strcpy(s, s+1);	/* 98/Aug/26 */
	lowercase(str, str);		/* for stupid DTF fonts 1992/Oct/22 */
	for (k = 0; k < 64; k++) {
		if (strcmp(weightnames[k], "") == 0) break;
		if (strcmp(weightnames[k], str) == 0) break;
	}
	if (strcmp(weightnames[k], "") != 0) weight = weightvalues[k];
	else fprintf(errout, "NOTE: Unrecognized Weight: %s\n", line);
#ifdef DEBUG
	if (verboseflag) printf("Weight `%s' => %d\n", str, weight);
#endif
	if (traceflag) printf("Weight `%s' => %d\n", str, weight);
	return weight;
}

/* read the AFM file and fill in slots in PFM as information available */

void readafmhead(FILE *input) {
	char fontname[FILENAME_MAX], fullname[FILENAME_MAX], familyname[FILENAME_MAX];
	int underlinepos, underlinethick, italicnum;
	int xll, yll, xur, yur;		/* only yur is used presently */
	int FontHeight, InternalLeading, Ascent;
	int fweight=400;
	unsigned int capheight, xheight; /* ascender, descender; */
	int ascender, descender; /*  capheight, xheight; */
	double italicangle, fspace;
	int k;
	int style;
	char *s;
	int strikeoutposition, strikeoutwidth;	/* 98/Aug/28 */

	textfontflag = 1;						/* default assumption */
	copyrightflag = originflag;				/* initial value */
	last = 199;

	xscale = 1.0;
	texspace = 0;
	strcpy(charcodingscheme, "");			/* in case not found in AFM */

	if (getrealline(line, MAXLINE, input) == NULL) {
		fprintf(stderr, "ERROR: Unexpected EOF\n");
		exit(13);
	}
	while (strncmp(line,"StartCharMetrics", 16) != 0) {
		if (strncmp(line, "EndFontMetrics", 14) == 0) {
			printf("ERROR: Premature %s", line);
			break;
		}
/*		if (traceflag != 0) printf("%s", line); */ /* debugging */
/*		if((s = strstr(line, "Copyright")) != NULL && copyrightflag	== 0) { */
/*		if(strncmp(line, "Comment Copyright", 17) == 0 && */
		if(_strnicmp(line, "Comment Copyright", 17) == 0 && 
				originflag == 0) { 
			trimcopyright(line + 8);			/* 1992/Nov/22 */
			if (strlen(line + 8) > 0) {
/*				strncpy(buffer+6, line + 8, MAXCOPYRIGHT-1);  */
				lstrncpy(buffer+6, line + 8, MAXCOPYRIGHT);
				*(buffer+6+MAXCOPYRIGHT-1) = '\0';	/* null terminate it  */
// #ifdef DEBUG
//				if (verboseflag) printf("BUFFER+6 %s\nLINE %s\n", buffer+6, line);
				if (verboseflag) printf("Copyright string (%d): '%s'\n", strlen(buffer+6), buffer+6);
// #endif
				copyrightflag++;			/* note that this has been done */
			}
		}
/*	deal with `Comment  Copyright' format ? (note two spaces) */
/*		else if(strncmp(line, "Notice ", 7) == 0 && copyrightflag == 0) { */
/*		else if(strncmp(line, "Notice ", 7) == 0) { */
		else if(strncmp(line, "Notice ", 7) == 0 && 
			originflag==0) {
			trimcopyright(line + 7);			/* 1992/Nov/22 */
/*			strncpy(buffer+6, line + 7, MAXCOPYRIGHT-1);  */
			if (strlen(line + 7) > 0) {
				lstrncpy(buffer+6, line + 7, MAXCOPYRIGHT); 
				*(buffer+6+MAXCOPYRIGHT-1) = '\0';	/* null terminate it  */
// #ifdef DEBUG
//				if (verboseflag) printf("BUFFER+6 %s\nLINE %s\n", buffer+6, line);
				if (verboseflag) printf("Copyright string (%d): '%s'\n", strlen(buffer+6), buffer+6);
// #endif
				copyrightflag++;		/* note that this has been done */
			}
		}
		else if ((strncmp(line, "FontName", 8)) == 0) {
/*			sscanf(line + 9, "%s", fontname); */	/* avoid spaces ! */
			strcpy(fontname, line+9); flushnewline(fontname); 
			if (uppercaseflag != 0) uppercase(fontname, fontname);
		}
		else if ((strncmp(line, "FullName", 8)) == 0) {
			strcpy(fullname, line+9); flushnewline(fullname);
		}
		else if ((strncmp(line, "FamilyName", 10)) == 0) {
			strcpy(familyname, line+11); flushnewline(familyname);
		}
/* Do not override command line 'w=...' */
/*		else if (strncmp(line, "Comment MS-WindowsName", 22) == 0 && */
		else if (_strnicmp(line, "Comment MS-WindowsName", 22) == 0 &&
			windowsflag == 0) {
			strcpy(windowsname, line+23); 
			flushnewline(windowsname); /* NEW */
			windowsflag = 1;	/* indicate that windowsname given */
		}
/* following added 1992/Dec/2 */
/*		else if (strncmp(line, "Comment MSMenuName", 18) == 0 && */
		else if (_strnicmp(line, "Comment MSMenuName", 18) == 0 &&
			windowsflag == 0) {
			strcpy(windowsname, line+19); 
			flushnewline(windowsname); /* NEW */
			windowsflag = 1;	/* indicate that windowsname given */
		}		
/*		else if (strncmp(line, "Comment DefaultCharacter", 24) == 0 && */
		else if (_strnicmp(line, "Comment DefaultCharacter", 24) == 0 &&
			strcmp(defaultname, "") == 0) {
			strcpy(defaultname, line+25);	/* absorb default char name */
		}
/* following added 1992/Dec/2 */
/*		else if (strncmp(line, "Comment Pi", 10) == 0) { */
		else if (_strnicmp(line, "Comment Pi", 10) == 0) {
			if (strstr(line, "true") != NULL) {
/*				ansiflag = 0; symbolflag = 1; oemflag = 0; */
				oemflag = 0;
				decorative = 1;		/* piflag */
			}
			else {
				ansiflag = 1; symbolflag = 0; oemflag = 0;
				decorative = 0;		/* piflag */
			}
		}
/* following added 1992/Dec/8 */
/*		else if (strncmp(line, "Comment VPStyle", 16) == 0) { */
		else if (_strnicmp(line, "Comment VPStyle", 15) == 0) {
			s = line + 16;
			while (*s <= ' ') s++;
			if (*s == '(') s++;
			style = *s;
/* presently boldflag and italicflag are not used yet ... */
			if (style == 'N') {
				boldflag = 0; italicflag = 0;
			}
			else if (style == 'B') {
				boldflag = 1; italicflag = 0;
			}
			else if (style == 'I') {
				boldflag = 0; italicflag = 1;
			}
			else if (style == 'T') {
				boldflag = 1; italicflag = 1;
			}
			if (boldflag != 0) 	writetwo(700, 83);	/* BOLD */
			else writetwo(400, 83);   
			if (italicflag != 0) *(buffer +80) = 1;	 /* ITALIC */
			else *(buffer +80) = 0;
		}
/* following added 1992/Dec/2 */
/*		else if (strncmp(line, "Comment Serif", 13) == 0) { */
		else if (_strnicmp(line, "Comment Serif", 13) == 0) {
			if (strstr(line, "true") != NULL) serifflag = 1;
			else serifflag = 0;
/*			fixedflag = 0; decorative = 0; */		/* ??? */
		}
/*		else if ((s = strstr(line, "Weight")) != NULL) { */
		else if ((strncmp(line, "Weight", 6)) == 0) { /* may overwrite */
			if (weight == 0) {		/* if not forced by command line ... */
				fweight = analyzeweight(line+6, line);
/*				printf("WEIGHT %d line: %s", fweight, line); */
			}
			else fweight = weight;				/* forced on command line */
			if (suppressbold != 0) fweight = 0;	/* 1992/Jan/03 */
			writetwo(fweight, 83);		/* write weight */ 
			if (fweight > 400)
				if(verboseflag != 0) 
					printf("Treated as bold font (weight %d)\n", fweight);
		}
		else if ((strncmp(line, "ItalicAngle", 11)) == 0) {		
			italicangle = 0.0;
			sscanf(line + 12, "%lg", &italicangle);
/*			if (italicangle != 0.0) { */		/* 1992/Jan/03 */
			if (italicangle != 0.0 && suppressitalic == 0) { 
				*(buffer +80) = 1;				/* Mark as Italic */
				if (verboseflag != 0) 
					printf("Treated as italic font (angle %lg)\n", 
						italicangle);
			}
			else *(buffer +80) = 0;			/* not Italic */
			italicnum = (int) floor(italicangle * 10.0);
/*			if (suppressitalic != 0) italicnum = 0; */ /* ??? */
			*(buffer +169) = (unsigned char) (italicnum & 255);
			*(buffer +170) = (unsigned char) (italicnum >> 8);
		}
/*		else if ((s = strstr(line, "UnderlinePosition")) != NULL) {
			sscanf(s + 18, "%d", &underlinepos); */
		else if ((strncmp(line, "UnderlinePosition", 17)) == 0) {
			underlinepos = - 100;
			sscanf(line + 18, "%d", &underlinepos);
			underlinepos = - underlinepos;
			writetwo(underlinepos, 179);	/* default 100 */
			writetwo(underlinepos, 185);
			underlinepos = underlinepos/2;
			writetwo(underlinepos, 183);			
		}
/*		else if ((s = strstr(line, "UnderlineThickness")) != NULL) {
			sscanf(s + 19, "%d", &underlinethick); */
		else if ((strncmp(line, "UnderlineThickness", 18)) == 0) {
			underlinethick = 50;
			sscanf(line + 19, "%d", &underlinethick);
			writetwo(underlinethick, 181);		/* default 50 */
			underlinethick = underlinethick/2;
			writetwo(underlinethick, 187);
			writetwo(underlinethick, 189);
		}
/*		else if ((s = strstr(line, "FontBBox")) != NULL) {
			sscanf(s + 9, "%d %d %d %d", &xll, &yll, &xur, &yur); */
		else if (strncmp(line, "FontBBox", 8) == 0) {
			if (sscanf(line, "FontBBox %d %d %d %d", &xll, &yll, &xur, &yur)
				== 4) {
/*			if (yur > 1000) 
				fprintf(errout, 
					"WARNING: yur (%d) > 1000, may increase leading\n", yur);
			writetwo(yur, 74);	*//* Ascent - NEW */ 
/* According to suggestion of Terry O'Donnell at Adobe --- 1992/July/9 */
/*			if (yur < 1000) writetwo(1000 - yur, 76); *//* `InternalLeading' */
/* Following according to Terry O'Donnell at Adobe --- 1993/June/12  */
			FontHeight = yur - yll;
			InternalLeading = FontHeight - 1000;
			if (InternalLeading < 0) InternalLeading = 0;
			Ascent = 1000 + yll + InternalLeading;
			if (Ascent > 1000) 
				fprintf(errout, 
				"WARNING: Ascent (%d) > 1000, may increase leading\n", yur);
			writetwo(Ascent, 74);
			writetwo(InternalLeading, 76);
			writetwo(FontHeight, 88);		/* 1993/June/12 */
/*			if (usebboxinfo != 0) { 
				writetwo(yur, 165);	
				writetwo(- yll, 167);
				writetwo(xur-xll, 93);
			} */
			}
			else fprintf(errout, "Don't understand %s", line);
		}
/* NOTE: with ATM, Ascent & Descent are taken from PFB file BBox, not PFM! */
		else if ((strncmp(line, "CapHeight", 9)) == 0) {
			sscanf(line + 10, "%u", &capheight);
			writetwo(capheight, 161);
		}		
		else if ((strncmp(line, "XHeight", 7)) == 0) {
			sscanf(line + 8, "%u", &xheight);
			writetwo(xheight, 163);
		}
		else if ((strncmp(line, "Ascender", 8)) == 0) {
			sscanf(line + 9, "%d", &ascender);
			writetwo(ascender, 165);
		}		
		else if ((strncmp(line, "Descender", 9)) == 0) {
			sscanf(line + 10, "%d", &descender);
			if (mindescend != 0) {	/* if limit given for this */
				if (descender < mindescend) descender = mindescend;
			}
			writetwo(- descender, 167);
		}			
		else if (strncmp(line, "IsFixedPitch", 12) == 0 ||
			strncmp(line, "isFixedPitch", 12) == 0) {
			 if (strstr(line, "true") != NULL) {
/*				 fixedflag = 1;		*/ /* ??? */
			 }
/* zero out variable pitch bit in PitchAndFamily byte */
/* maybe NOT: this bit always seems to be on in PS fonts ... */
		}			
		else if (strncmp(line, "EncodingScheme", 14) == 0) {
			strcpy(charcodingscheme, line + 15);		/* 1992/Dec/8 */
			if ((s = strchr(charcodingscheme, '\n')) != NULL) *s= '\0';
/*			if (strncmp(charcodingscheme, "AdobeStandardEncoding", 21) == 0) */
			if (_stricmp(charcodingscheme, "AdobeStandardEncoding") ==  0 ||
				_stricmp(charcodingscheme, "StandardEncoding") ==  0 ||
				_stricmp(charcodingscheme, "MicroSoft Windows ANSI 3.1") == 0 ||
				_stricmp(charcodingscheme, "MS Windows ANSI") == 0) {
				textfontflag=1;
				if (verboseflag) printf("Should be treated as %s font (%s)\n",
									   "text", charcodingscheme);
			}
			else if (_stricmp(charcodingscheme, "FontSpecific") == 0) {
				textfontflag=0;
				if (verboseflag) printf("Should be treated as %s font (%s)\n",
										"non-text", charcodingscheme);
			}
			else {
				textfontflag=0;
				if (verboseflag) printf("Should be treated as %s font (%s)\n",
										"non-text", charcodingscheme);
			}
			if (verboseflag) printf("charcodingscheme: %s\n", charcodingscheme);
		}
/*		else if (strncmp(line, "Comment CharacterCodingScheme", 29) == 0) { */
		else if (_strnicmp(line, "Comment CharacterCodingScheme", 29) == 0) {
/*			if ((s = strchr(line, '\n')) != NULL) *s= '\0'; */
			strcpy(charcodingscheme, line + 30);		/* NEW */
			if ((s = strchr(charcodingscheme, '\n')) != NULL) *s= '\0';
		}
/*		else if (strncmp(line, "Comment Condensed", 17) == 0) { */
		else if (_strnicmp(line, "Comment Condensed", 17) == 0) {
			sscanf(line + 18, "%lg", &xscale);
		}
/*		else if (strncmp(line, "Comment Expanded", 16) == 0) { */
		else if (_strnicmp(line, "Comment Expanded", 16) == 0) {
			sscanf(line + 17, "%lg", &xscale);
		}		
/*		else if (strncmp(line, "Comment Space", 13) == 0) { */
		else if (_strnicmp(line, "Comment Space", 13) == 0) {
			sscanf(line + 14, "%lg", &fspace);	/* 1992/May/04 */
			fspace = fspace * xscale;	
			texspace = (int) (fspace + 0.49);	/* round width to integer */
		}		
		if (getrealline(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "ERROR: Unexpected EOF\n");
			exit(13);
		}
/*		putc('.', stdout);	*/ /* debugging */
	} /* end of not StartCharMetrics while loop */

	if (xscale != 1.0)
		printf("NOTE: Imposing horizontal scaling by %lg\n", xscale);

	if (*newfontname != '\0') strcpy(fontname, newfontname);

/*	has textfontflag been set at this stage? yes */
/*	but does setting of flags come too late? */
	if (automatetest) {
		if (textfontflag) {
			printf("Treated as %s font\n", "text");
			ansiflag = 1;
			symbolflag = 0;
			oemflag = 0;
			decorative = 0;
			useafmencoding=0;	/* ??? */
		}
		else {
			printf("Treated as %s font\n", "non-text");
/*			ansiflag = 0; */
/*			symbolflag = 1; */
			oemflag = 0;
			decorative = 1;
			useafmencoding=1;	/* ??? */
		}
	}

	strikeoutwidth = 50;			/* capheight known ? */
	strikeoutposition = (capheight - strikeoutwidth) / 2;
/*	if (smallcaps != 0) writetwo(277, 191);	*/
/*	else writetwo(311, 191); */
	writetwo(strikeoutposition, 191); /* StrikeoutPosition */
/*	writetwo(40, 193); */	/* `StrikeoutWidth' */
	writetwo(strikeoutwidth, 193);	/* `StrikeoutWidth' */ 

/*	printf("Found StartCharmetrics\n"); */ /* debugging */

/*	if weight explicitely given, overwrite above */
/*	if (weight > 0) writetwo((unsigned int) weight, 83); */

/*	copy over FontName and Full Name */

	last += (int) strlen("PostScript") + 1;
/*	assert (last == 210); */
	writetwo((unsigned int) last, 105);	
							/* pointer to `Face' - MS windows font name */
	if (windowsflag) strcpy(line, windowsname);/* use forced MS windows name */
	else if (usefamilyname) strcpy(line, familyname);/* use FamilyName */
	else if (usefullname) strcpy(line, fullname); /* use FullName */
	else {
		strcpy(line, fontname);				  /* use FontName */
		if (stripprefixflag) stripprefix(line);
		if (shortenflag) flushbolditalic(line);
	}
	if (XFaceFlag != 0) {				/* unless name already ends in `X' */
		s = line + strlen(line) - 1;
		if (ansiflag != 0) {			/* 1993/Oct/4 */
			if (*s == 'X') *s = '\0';	/* flush the trailing `X' */
		}
		else {
			if (*s != 'X') strcat(line, "X");	/* extend face name with `X' */
		}
	}
	if (verboseflag != 0) printf("Using %s as Windows Face Name\n", line); 
	lstrcpy(buffer + last, line);
	last += (int) strlen(line) + 1;

	writetwo((unsigned int) last, 139);	
				/* pointer to `DriverInfo'  PostScript fontname */
	lstrcpy(buffer + last, fontname);	/* copy `DriverInfo' */
	last += (int) strlen(fontname) + 1;

	if (signature) {		/* safe to stick arbitrary junk here */
		lstrcpy(buffer + last, yandy);
		last += (int) strlen(yandy) + 1;	 
	}
	if (hideencoding) {		/* try and hide some information on coding */
/*		charcodingscheme should already be set from AFM EncodingScheme field */
/*		if encoding vector specified, it overrides what was in AFM file */
		if (strcmp(charcodingscheme, "") == 0) {		/* if it was missed */
			if (useafmencoding == 0)	/* ??? */
				strcpy(charcodingscheme, codingvector);
		}
/*		strcat(charcodingscheme, "\n"); */		/* make it stand out own line */
/*		if TeX style encoding vector, expand into verbose version */
/*		for (k = 0; k < encodingcount; k++) { */		/* for TeX names */
		for (k = 0; k < 64; k++) { 		/* for TeX names */
			if (strcmp(encodingnames[k][0], "") == 0) break;
			if (strcmp(encodingnames[k][1], "") == 0) break;
/*			if (strncmp(codingvector, encodingnames[k][1], 8) == 0) { */
			if (_strnicmp(codingvector, encodingnames[k][1], 8) == 0) {
/*				if (strcmp(encodingnames[k][0], "") == 0) break; */
/*				if (strcmp(encodingnames[k][1], "") == 0) break; */
				strcpy(charcodingscheme, encodingnames[k][0]); 
/*				strcat(charcodingscheme, "\n"); */			/* NEW */
				break;
			}
		}
		if (strcmp(encodingvecname, "") != 0) {	/* override everything if */
			if (useafmencoding == 0)	/* ??? */
				strncpy(charcodingscheme, encodingvecname, MAXENCODING);
		}

		if (useafmencoding) strcpy(charcodingscheme, "FontSpecific");
		if (verboseflag != 0) 	/* just debugging mostly */
			printf("charcodingscheme: %s\n", charcodingscheme); 
/*		strcpy(line, encodingstr); */
		strcpy(line, "\n"); 
		strcat(line, "Encoding: "); 
/*		last += (int) strlen(encodingstr); */
		if (strcmp(charcodingscheme, "") != 0) {
/*			strcpy(buffer + last, charcodingscheme); */
			strcat(line, charcodingscheme);
/*			last += (int) strlen(charcodingscheme) + 1;	 */
		}
		else {
/*			strcpy(buffer + last, "Font Specific\n"); */
			strcat(line, "Font Specific");
/*			last += (int) strlen(fontspecific) + 1; */
		}
		strcat(line, "\n");			/* NEW */
		lstrcpy(buffer + last, line);	/* Encoding: */
		last += strlen(line) + 1;
/*		if (verboseflag != 0) printf("%s", s+1); */	/* debugging */
/*		if (verboseflag != 0) printf("%s", line); *//* debugging */ 
		if ((s = strchr(line+1, '\n')) != NULL) *s = '\0';
		if (verboseflag != 0) printf("%s\n", line+1);	/* 1993/Feb/8 */
	}
	writetwo((unsigned int) last, 123);	
			/* pointer to `ExtentTable' - charwidths */
/*	if no other copyright info at all, stick in Y&Y copyright 1992/July/15 */
	if (! copyrightflag && *(buffer+6) == 0) {
/*		strncpy(buffer+6, yanycopyright, MAXCOPYRIGHT-1); */
/*		lstrncpy(buffer+6, yanycopyright, MAXCOPYRIGHT-1); */
		if (usenewcopy) lstrncpy(buffer+6, newcopyright, MAXCOPYRIGHT); 
		else lstrncpy(buffer+6, yanycopyright, MAXCOPYRIGHT); 
		*(buffer+6+MAXCOPYRIGHT-1) = '\0';	/* null terminate it  */
#ifdef DEBUG
//		if (verboseflag) printf("BUFFER+6 %s %s\n", buffer+6, "readafmhead");
		if (verboseflag) printf("Copyright string (%d): '%s'\n", strlen(buffer+6), buffer+6);
#endif
	}
}

void removesemi(char *name) {	/* get rid of trailing semi colon - if any */
	char *s;
	s = name + strlen(name) - 1;
	if (*s == ';') *s = '\0';
}

/* CM remap (0-9 => 161-170, 10-32 => 173-195, 127 => 196 */

int remapchar(int chr) {
	if (chr < 10) return (chr + 161);
	else if (chr < 33) return (chr + 163);
	else if (chr == 127) return 196;
	else return chr;
}

void setuprelative(int charmin, int charmax, int flag) {
	*(buffer +95) = (unsigned char) charmin;		/* firstchar */
	*(buffer +96) = (unsigned char) charmax;		/* lastchar */
	defaultwidth = (int) readtwo(last + 2 * (defaultchar - charmin));
/*	following added do deal with case where 0 - 32 are stripped out */
	if (remapflag != 0) {
		if (defaultchar < charmin) 	defaultchar = remapchar(defaultchar);
/*	following added 1993/April/26 */ /* to deal with ATM repeat encoding bug */
		if (defaultchar < 33) defaultchar = remapchar(defaultchar);
		if (breakchar < charmin) breakchar = remapchar(breakchar);
/*	following added 1993/April/26 */ /* to deal with ATM repeat encoding bug */
		if (breakchar < 33) breakchar = remapchar(breakchar);
		if (verboseflag && flag > 0) {
			printf("Remapped break %u default %u ",
				   breakchar, defaultchar);
			printf("default width %u\n", defaultwidth);
		}
	}
	*(buffer +97) = (unsigned char) (defaultchar - charmin); /* `DefaultChar' */
	*(buffer +98) = (unsigned char) (breakchar - charmin); /* `BreakChar' */
}

int copywidths (int k, int kdash) {
	int charwidth, oldwidth, flag=0;

/*	if (k < charmin) charwidth = averwidth; */
	if (k < charmin) return 0;	/* nothing to copy 1993/Aug/31 */
	else charwidth = readtwo(last + 2 * (k - charmin));
	if (charwidth == UNKNOWNWIDTH) /* || charwidth == defaultwidth */
		return 0;				/* nothing to copy 1993/Aug/31 */
	if (charwidth < -4096 || charwidth > 4096) {
		fprintf(errout, "Ridiculous character width %d\n", charwidth);
		charwidth = averwidth;
		return 0;				/* nothing to copy 1993/Aug/31 */
	}
	oldwidth = readtwo(last + 2 * (kdash - charmin));
	if(oldwidth != UNKNOWNWIDTH && oldwidth != defaultwidth) {
		fprintf(errout, "Attempt to overwrite char %d ", k+161);
		fprintf(errout, "old %d new %d", oldwidth, charwidth);
		putc('\n', errout);
		flag++;
	}
	else writetwo(charwidth, last + 2 * (kdash - charmin));
	return flag;
}

/* returns zero if remapping is OK */
/* returns > 0 if conflicts were found */
/* returns < 0 if nothing found in encoding to remap */

int checkremap (void) {			/* check whether safe to remap 1993/May/29 */
	int k, count=0, nconflict = 0;

	for (k = 0; k < 10; k++) {
		if (strcmp(encoding[k], "") != 0) {
			if (strcmp(encoding[k+161], "") != 0) {
				nconflict++;
			}
			count++;
		}
	}
	for (k = 10; k < 32; k++) {
		if (strcmp(encoding[k], "") != 0) {
			if (strcmp(encoding[k+163], "") != 0) {
				nconflict++;
			}
			count++; 
		}
	}
	if (strcmp(encoding[32], "") != 0) {
		if (strcmp(encoding[195], "") != 0) {
				nconflict++; 
		}
/*		count++; */	/* don't count space */
	} 
	if (strcmp(encoding[127], "") != 0) {
		if (strcmp(encoding[196], "") != 0) {
				nconflict++; 
		}
/*		count++; */	/* don't count delete */
	}
	if (strcmp(encoding[32], "") != 0) {
		if (strcmp(encoding[128], "") != 0) {
			nconflict++; 
		}
/*		count++; */	/* don't count space */
	}
	if (count == 0 && nconflict <= 2) return -1; /* no control characters */
	else if (nconflict > 0) return nconflict;	/* some conflicts found */
	else return 0;								/* no conflicts */
}

int makedefault (int flag) { /* try and figure out a default character */
	int n, m, charwidth;
	char *s;
	
	defaultchar = -1;
	s = "";
	if (strcmp(defaultname, "") != 0) {
		if ((n = lookup(defaultname, -1)) >= 0) {
			defaultchar = n; s = defaultname;
		}
		else fprintf(errout, "WARNING: Default char %s not found\n", 
			defaultname);
	}

/*  if not specified - or - specified and not found */
/*	problem here if reencoded, since the encoding vector may contain */
/*  various characters that do not appear in the font ... */
/*	unfortunately, the width table has not been filled in at this point */
	if (defaultchar < 0) {
		for(m = 0; m < 32; m++) {			/* 92/June/2 */
			if (strcmp(defaultchars[m], "") == 0) break;
			if ((n = lookup(defaultchars[m], -1)) >= 0) {
				if (flag) {
					charwidth = readtwo(last + 2 * (n - charmin));
/*					printf("m %d default %s n %d w %d\n",
						m, defaultchars[m], n, charwidth); */
					if (charwidth != UNKNOWNWIDTH) {
						defaultchar = n;
						s = defaultchars[m];
						break;
					}
				}
				else {
					defaultchar = n;
					s = defaultchars[m];
					break;
				}
			}
		}
	}
	if (defaultchar < 0) {
		defaultchar = 32;
		fprintf(errout, "WARNING: No default character found, using %d\n",
				defaultchar);
	} 
	if (defaultchar > charmax) defaultchar = 32;
	if (verboseflag && flag) 
		printf("Using `%s' (%d) as default character\n",
			   encoding[defaultchar], defaultchar);
	return defaultchar;
}

void copycharwidths(FILE *input) {
	char charname[MAXCHARNAME];
	char charhit[256];			/* to catch repeat lines */
/*	int charmin=256, charmax=-1; */
	int charnum, charwidth, chars=0;
	int widthmax=0;
	int widthofx, k, n;
/*	int m, widthspace; */
	int unknownfill;
	double fcharwidth;
/*	char *s; */
	long widthsum=0L, startwidth;
	int nconflict=0, prev, flag;
	
	charmin=256; charmax=-1; 

/*	now copy over charwidths */
	
	startwidth = ftell(input);	/* remember where width information started */
	
	for (k = 0; k < 256; k++) charhit[k] = 0;	/* 1992/Sep/24 */

	notencoded = 0;			/* count of characters not found in encoding */
	total = 0;				/* count of characters 96/June/16 */
	if (getrealline(line, MAXLINE, input) == NULL) {
		fprintf(stderr, "ERROR: Unexpected EOF\n");
		exit(13);
	}

/* First part just reads the encoding if necessary and gets charmin/charmax */
			
	while(strncmp(line, "EndCharMetrics", 14) != 0) {
		if (strncmp(line, "EndFontMetrics", 14) == 0) {
			printf("ERROR: Premature %s", line);
			break;
		}
		if (sscanf(line, "C %d ; WX %lg ; N %s ;", 
			&charnum, &fcharwidth, charname) < 3) {
			fprintf(errout, "Don't understand %s\n", line);
		}
		if (charnum > 0 && charnum < 256) {			/* 1992/Sep/24 */
			if (charhit[charnum] != 0)
				fprintf(errout, "\nERROR: Duplicate entry for char %d\n", 
					charnum);
			charhit[charnum]++;
		}
		fcharwidth = fcharwidth * xscale;
		removesemi(charname);

/*		if (useafmencoding == 0) charnum = lookup(charname); */
		if (useafmencoding == 0) charnum = lookup(charname, -1);
/*		printf("%s code %d  ", charname, charnum); */  /* DEBUGGING */
/*		ignore given encoding - force MS windows ANSI encoding ? */
/*		if (ansiflag != 0) charnum = lookup(charname, -1); */

/*		following added 1992/Sep/2 */
		if (charnum < 0 && remapflag != 0 && 
			strcmp(charname, "space") == 0 && 
				strcmp(encoding[160], "") == 0) charnum = 160;

		total++;
		if (charnum >= 0 && charnum <= 255) {
			if (charnum > charmax) charmax = charnum;
			if (charnum < charmin) charmin = charnum;
			if (useafmencoding != 0) {		/* gobble encoding from AFM */
				strcpy(encoding[charnum], charname); 
/*				if (traceflag != 0) 
					printf("C %d ; N %s\n", charnum, charname); */
			}
		}
		else {
			if (verboseflag != 0) printf("%s ", charname);  /* not encoded */
			notencoded++;
		}
		if (getrealline(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "ERROR: Unexpected EOF\n");
			exit(13);
		}
	}
	
	if (verboseflag != 0 && notencoded != 0) putc('\n', stdout);

	if (verboseflag != 0 && notencoded != 0) {
		if (notencoded == 1) printf("NOTE: Above char ");
		else printf("%s: Above %d chars ",
					(notencoded > 100) ? "WARNING" : "NOTE", notencoded);
		printf("(out of %d) not encoded\n", total);
	}

	if (charmin < 32) charmin = 0;
	if (startlate != 0 && charmin > 32) charmin = 32;
	if (texflag != 0) charmin = 0;		/* FORCE IT ... */
	if (whimpflag != 0 && ansiflag != 0) charmin = 32;		/* ATM bugs ? */
	if (oemflag != 0) charmin = 0;		/* ATM bug fix ... */
/*	charrem = charmax; */					/* remember actual charmax */
	if (charmax > 200) charmax = 255;
	if (charmax < 127) charmax = 127;
	if (remapflag != 0) charmax = 255; 

	*(buffer +95) = (unsigned char) charmin; 	/* firstchar */
	*(buffer +96) = (unsigned char) charmax; 	/* lastchar */

	makedefault(0);		/* maybe flush ? done later */	/* 93/Oct/27 */

	if ((n = lookup("space", -1)) >= 0) breakchar = n;	/* 1992/01/26 */

	if (breakchar > charmax) breakchar = 32;		/* phooey ! */

/*	*(buffer +97) = (unsigned char) (defaultchar - charmin);  */
/*	*(buffer +98) = (unsigned char) (breakchar - charmin); */

	setuprelative(charmin, charmax, 0);		/* 1993/April/26 */

/*  was w.r.t. 32, now is w.r.t charmin */
/*  NOTE: ATM and PSCRIPT.DRV may disagree on this */

/*  But charmax may change later ? NO */ /* debugging ? */
/*	if (verboseflag != 0) 
		printf("charmin %d charmax %d\n", charmin, charmax); */
	for(k = charmin; k <= charmax; k++) {	/* background of unknowns */
		writetwo(UNKNOWNWIDTH, last + 2 * (k - charmin));
	}

/*	Now go back to where character width information starts */
/*	This time actually read character widths ... */

	fseek(input, startwidth, SEEK_SET);

	if(getrealline(line, MAXLINE, input) == NULL) {
		fprintf(stderr, "ERROR: Unexpected EOF\n");
		exit(13);
	}

	while(strncmp(line, "EndCharMetrics", 14) != 0) {
		if (strncmp(line, "EndFontMetrics", 14) == 0) {
			printf("ERROR: Premature %s", line);
			break;
		}
/*		if (traceflag != 0) printf("%s", line);  */
		sscanf(line, "C %d ; WX %lg ; N %s ", 
			&charnum, &fcharwidth, charname);
		fcharwidth = fcharwidth * xscale;	
/*		ignore given encoding - force MS windows ANSI encoding */

		prev = -1;

		for (;;) {						/* 1992/Nov/22 */
			if (prev == MAXCHRS) break;
/*		charnum = lookup(charname); */
		charnum = lookup(charname, prev);
/*		see whether character is encoded or not */
		if (charnum < 0 || charnum > 255) {
/*			if(getrealline(line, MAXLINE, input) == NULL) {
				fprintf(stderr, "ERROR: Unexpected EOF\n");
				exit(13);
			}
			continue; */
			break;				/* 1992/Nov/22 ??? */
		}
		charwidth = (int) (fcharwidth + 0.49);	/* round width to integer */

		if (charwidth == 0) {		/* Windows/ATM bug work-around !!! */
			if (traceflag != 0) 
				printf("C %d ; WX %lg ; N %s\n", charnum, fcharwidth, charname);
/* But note that Windows PS driver _still_ may have problems - so fix maybe */
			if (minpscript > 0) charwidth = minpscript;
/*			else charwidth = 1;	*/
			else charwidth = 2;			/* 98/Sep/10 */
		}

		if (charwidth > widthmax) widthmax = charwidth;
		if ((charnum >= 'a' && charnum <= 'z') ||
			(charnum >= 'A' && charnum <= 'Z') ||
			(charnum >= '0' && charnum <= '9') ||
		     charnum == ' ') {
			widthsum = widthsum + charwidth;
			chars++;
		}
		if (charnum >= charmin)		/* enter the character width in table */
			writetwo(charwidth, last + 2 * (charnum - charmin));

		if (charnum < 0) prev = MAXCHRS;	/* 1992/Nov/22 */
		else prev = charnum;

		}	/* end of new for loop 1992/Nov/22 */

		if (getrealline(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "ERROR: Unexpected EOF\n");
			exit(13);
		}
	}

/*	pull out width of `x' for average width if needed */
	widthofx = (int) readtwo(last + 2 * (120 - charmin)); /* width of `x' */
/*	now deal with max and average char width */
	if (chars > 0) averwidth = (int) (widthsum/chars);	/* average seen */
	else averwidth = widthofx;

	if (texflag != 0 && texspace != 0)				/* 1992/May/04 */
		writetwo(texspace, last + 2 * (160 - charmin));

/*	Following limits `average width' at width of space */
/*	widthspace = (int) readtwo(last + 2 * (160 - charmin));
	if (widthspace > 0  && widthspace < averwidth) averwidth = widthspace;
	widthspace = (int) readtwo(last + 2 * (32 - charmin));
	if (widthspace > 0  && widthspace < averwidth) averwidth = widthspace; */

	if (usewidthofx != 0 && widthofx > 0 && widthofx < 1000)
		averwidth = widthofx;

/*	Incr max width of fixed width font by one to indicate font is remapped */
/*	This is picked up by DVIWindo `isremapped' function in winanal.c */
/*	(It can't otherwise tell that font is remapped since all widths same */
	if (widthmax == averwidth && remapflag != 0) widthmax++;

	writetwo((unsigned int) averwidth, 91);		/* AvgWidth */
	writetwo((unsigned int) widthmax, 93);		/* MaxWidth */

	makedefault(1);								/* 93/Oct/27 */
	setuprelative(charmin, charmax, 1);			/* 93/Oct/27 */

/*	fill in the holes - give default widths to those not set ... */

/*	defaultwidth = (int) readtwo(last + 2 * (defaultchar - charmin)); */

	if (defaultwidth == UNKNOWNWIDTH) {		/* BAD DEFAULT ! */
		defaultwidth = averwidth;
		if (defaultwidth > 1000) defaultwidth = 300; /* desparate ! */
		writetwo((unsigned int) defaultwidth, 
			last + 2 * (defaultchar - charmin));
	}

	if (verboseflag)
		printf("Using %d (from %d) for default width\n",
			   defaultwidth, defaultchar);

	unknownfill = 0;			/* how many unknowns where filled in */
	for(k = charmin; k <= charmax; k++) {
		if(readtwo(last + 2 * (k - charmin)) == UNKNOWNWIDTH) {
/*			if (traceflag != 0) printf("%d ", k); */
			writetwo(defaultwidth, last + 2 * (k - charmin));
			unknownfill++;
		}
	}
	if (verboseflag != 0 && unknownfill != 0)
		printf("Filled in %d unknowns (between %d and %d)\n", 
			unknownfill, charmin, charmax);

/*	do LucidaMath style TeX remapping */
/*  If asked, AND if no chars above 128 conflict, and if some below 32 */
/*	printf("extend %d charrem %d charmin %d\n", 
			remapflag, charrem, charmin); */ /* debugging */
/*	if (remapflag != 0 && charrem <= 128 && charmin == 0) {	 */
	if (remapflag != 0) {	/* 1993/May/29 */
		flag = checkremap();
		if (flag > 0) {
			fprintf(errout, "%s: Found %d conflicts",
					(flag > 16) ? "WARNING" : "NOTE", flag);
			remapflag = 0;
		}
		if (flag < 0) {
			fprintf(errout, "NOTE: No control characters");
			remapflag = 0;
		}
		if (flag != 0)
			fprintf(errout, " --- control char remapping suppressed\n");
	}
	if (remapflag != 0) {		/* just do what asked for ! */
		if (verboseflag != 0) 
			printf("Adding remap (0-9 => 161-170, 10-32 => 173-195, 127 => 196)\n");
/*		if (verboseflag != 0) 
			printf("charmin %d charmax %d\n", charmin, charmax); */
	
		for (k = 0; k < 10; k++) 
			if (copywidths(k, k + 161) > 0) nconflict++;

		for (k = 10; k < 32; k++) 
			if (copywidths(k, k + 163) > 0) nconflict++;

/*		use copywidths for the following ? */
		charwidth = readtwo(last + 2 * (32 - charmin));			/* space */
		if (charwidth != UNKNOWNWIDTH && charwidth != defaultwidth) {
			writetwo(charwidth, last + 2 * (195 - charmin));	/* redundant ? */
			writetwo(charwidth, last + 2 * (128 - charmin));
		}

		charwidth = readtwo(last + 2 * (127 - charmin));		/* delete */
		if (charwidth != UNKNOWNWIDTH && charwidth != defaultwidth) 
			writetwo(charwidth, last + 2 * (196 - charmin));		
		if (nconflict > 0) {
			fprintf(errout, 
	"WARNING: Apparent conflicts due to remapping - do not use `t'\n");
		}

/*  Move everything down --- to deal with bug in old Designer ... */
/*  DON'T DO THIS:  THIS INTRODUCES BUGS IN OTHER SOFTWARE ! */
		if (shiftdown != 0) {
			if (charmin == 0) {
				if (verboseflag != 0) 
					printf("Shifting down (removing 0 - 31 range)\n");
				for (k = 32; k <= charmax; k++) {
					charwidth = readtwo(last + 2 * (k - charmin));
					writetwo(charwidth, last + 2 * (k - 32));
				}
				charmin = 32; 	/*  adjust things that depend on charmin */	
				setuprelative(charmin, charmax, 2);
			}
			else fprintf(errout, 
				"Omitting down shift, since charmin = %d\n", charmin);
		}
	}

/*	last = last + 2 * (charmax - charmin + 1); */
	last += 2 * (charmax - charmin + 1);
}
	
int dokerntable(FILE *input);

void showusage(char *s) {
	fprintf (errout, "\
%s [-{v}{a}{s}{o}{r}{h}{m}{d}{x}{t}{k}{e}{B}{I}{T}]\n\
\t\t[-w=<MS-name>] [-f=<default>] [-c=<vector>]\n\
\t\t<afm-file-1> <afm-file-2>...\n", s);
	if (detailflag == 0) exit(1);
	fprintf (errout, "\
\tv: verbose\n\
\tCharSet a: ANSI (default)\n\
\t\ts: Symbol\n\
\t\to: OEM\n\
\tFamily  r: `Roman'  - serif variable width (default)\n\
\t\th: `Swiss'  - sans serif variable width \n\
\t\tm: `Modern' - fixed width\n\
\t\td: `Decorative' (Math, Pi, Dingbats, non-text)\n\
\tuse `d' (or `s') to prevent reencoding of non-text fonts to Windows ANSI\n\
\tC: specify CharSet numerically (0 - 255)\n\
\tZ: specify Family numerically (0 - 15)\n\
\tT: disable automatic detection of text versus non-text font\n\
\t   (using a, s, o, r, h, m, d, C or Z also has this effect)\n\
\tB: suppress bold flag\n\
\tI: suppress italic flag\n\
\tx: do not add 'x' to output file name if reencoded\n\
\tt: add control char remap (0-9 => 161-170, 10-32 => 173-195, 127 => 196)\n\
\tk: suppress all kerning pairs\n\
\te: use FamilyName for MS Windows Face Name\n\
\tw: MS Windows face name (override automatically generated name)\n\
\tf: use specified default character (default `%s')\n\
\tc: use specified encoding vector (default `%s' unless text font)\n\
\t   (if `none' specified, use encoding in AFM file)\
", 
/*	ansidefault, texdefault, defaultencoding); */
	defaultchars[0], defaultencoding); 
	exit(1);
}

/* \tX: add 'X' to Windows Face Name\n\ */
/* \tF: use FullName for MS Windows Face Name\n\ */
/* \ty: remove 0-31 range when adding remap (not recommended)\n\ */
/* \tWeight  n: Normal (default, unless specified in AFM file)\n\  */
/* \t\tb: Bold\n\ */
/* \t\tl: Light\n\ */

/* \t\td: Decorative (prevent ATM from using ANSI encoding)\n\ */
/* \t\ts: Symbol (prevent ATM from using ANSI encoding)\n\ */

/* \tz: do not flush zero size kerns\n\ */
/* \tu: extend name with underscore\n\ */
/* \te: read encoding from AFM file\n\ */
/* \tf: use FullName (instead of FontName) to construct MS Windows name\n\ */
/* \tq: override AFM copyright (-q => Y&Y, -qq => BSR, -qqq => AT&T)\n\ */
/* \ts: spacewidth (if not specified in AFM file)\n\ */

/* abcedf.h..klmnopqrstuvwxyz */
/* ABCEDFGHI.....O..RST..WXYZ */


int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': if (verboseflag != 0) traceflag = 1; else verboseflag = 1; 
			return 0; 
/*		turn off auto-detecting of text versus non-text if user specifies */
		case 'a': ansiflag = 1; symbolflag = 0; oemflag = 0; automatetest=0; return 0; 
		case 's': ansiflag = 0; symbolflag = 1; oemflag = 0; automatetest=0; return 0; 
		case 'o': ansiflag = 0; symbolflag = 0; oemflag = 1; automatetest=0; return 0; 
/*		case 'u': underflag = 1; return 0; */
		case 'u': uppercaseflag = 1; return 0; 
		case 'Q': hideencoding = signature = 0; truncatecopy = 1; return 0;
		case 'q': originflag++; return 0;
/*		case 'e': useafmencoding = 1; return 0;  */
		case 'h': serifflag = 0; fixedflag = 0; decorative = 0; automatetest=0; return 0; 
		case 'r': serifflag = 1; fixedflag = 0; decorative = 0; automatetest=0; return 0; 
		case 'm': fixedflag = 1; decorative = 0; automatetest=0; return 0;
		case 'd': decorative = 1; automatetest=0; return 0;
		case 'n': weight = 400; return 0; 
		case 'b': weight = 700; return 0; 
		case 'l': weight = 250; return 0;  
		case 't': remapflag = 1; return 0; 
		case 'T': automatetest = ! automatetest; return 0; 
		case 'Y': truncatecopy = ! truncatecopy; return 0;	/* 98/Aug/28 */
		case 'y': shiftdown = 1; return 0;
/*		case 'z': flushzeros = 0; return 0; */
		case '0': flushzeros = 0; return 0;
		case 'k': wantkerns = 0; return 0;
		case 'x': dontexten = 1; return 0;
		case 'G': removeX = 1; return 0;		/* 98/Mar/23 */
		case 'X': XFaceFlag = 1; return 0;
		case 'H': insisthyphen = 1; return 0;	/* 94/Apr/18 */
		case 'e': usefamilyname = 1; return 0;
		case 'I': suppressitalic = 1; return 0;
		case 'B': suppressbold = 1; return 0;
		case 'F': usefullname = 1; return 0;  
		case 'A': allowaliases = 1; return 0;	/* 95/Dec/31 */
/*		case 'E': errout = stdout; return 0; */
		case 'E': errout = stderr; return 0;	/* 97/Sep/23 */
/*		rest take arguments */
		case 'w': nameflag = 1; return -1; 
		case 'W': fontnameflag = 1; return -1; 
		case 'c': vectorflag = 1; return -1;   
		case 'C': charsetflag = 1; automatetest=0; return -1;
		case 'Z': familyflag = 1; automatetest=0; return -1;   
		case 'f': defaultflag = 1; return -1;   
		case 'p': pscriptflag = 1; return -1;
		case 'D': descendflag = 1; return -1;
		case 'R': sourceflag = 1; return -1;
		case 'O': outputpathflag = 1; return -1;	/* 1995/Jan/2 */
/*		case 's': spaceflag = 1; return -1;  */
		default: {
		     fprintf(stderr, "Invalid command line flag '%c'", c);
		     exit(7);
		}
	}
/*	return 0; */	 /* ??? */
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (nameflag != 0) {
					strcpy(windowsname, s);
					nameflag = 0; windowsflag = 1; 
				}
				else if (fontnameflag != 0) {	/* 97/Jan/18 */
					strcpy(newfontname, s);
					fontnameflag = 0;
				}
				else if (vectorflag != 0) {
					strcpy(codingvector, s);  
					vectorflag = 0;		/* reencodeflag = 1;  */
				}
				else if (defaultflag != 0) {
					strcpy(defaultname, s); 
					defaultflag = 0; 
				}
				else if (spaceflag != 0) {
					if (sscanf(s, "%lg", &spacewidth) < 1) {
						fprintf(errout, "Don't understand space %s\n", s);
					}
					spaceflag = 0; 
				}
				else if (pscriptflag != 0) {
					if (sscanf(s, "%lg", &minpscript) < 1) {
						fprintf(errout, "Don't understand min width %s\n", s);
					}
					pscriptflag = 0; 
				}
				else if (descendflag != 0) {
					if (sscanf(s, "%d", &mindescend) < 1) {
					   fprintf(errout, "Don't understand min descend %s\n", s);
					}
					descendflag = 0; 
				}
				else if (sourceflag != 0) {
					sourcefile = s;	sourceflag = 0;
				}
				else if (outputpathflag != 0) {				/* 1995/Jan/2 */
					outputpath = s;	outputpathflag = 0;
				}
				else if (charsetflag != 0) {
					if (sscanf(s, "%d", &CharSet) < 1) {
					   fprintf(errout, "Don't understand CharSet %s\n", s);
					}
					CharSet = CharSet & 255;
					charsetflag = 0;
				}
				else if (familyflag != 0) {
					if (sscanf(s, "%d", &Family) < 1) {
					   fprintf(errout, "Don't understand family %s\n", s);
					}
					Family = Family & 15;
					familyflag = 0;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
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

void flushexten(char *name) {	/* remove extension - if any */
	char *s;
	if ((s = strchr(name, '.')) != NULL) *s = '\0';
}

/* remove file name - keep only path - inserts '\0' to terminate */

void striptopath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
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

/* returns non-zero if name extended with `x' */

int addanex(char *filename) {		/* add `x' at end of name if possible */
	int n;
	char *s;
	char *sname;			/* 95/Jan/2 */

/*	if (traceflag != 0) printf("Attempting to extend %s with `x'\n", name); */
	sname = removepath(filename);
/*	n = (int) strlen(name); */				/* length of name */
	n = (int) strlen(sname);				/* length of name */
	if (n > 8) return 0;					/* name too long! bug */
/*	s = name + n - 1; */					/* last character */
	s = sname + n - 1;						/* last character */
	if (n == 8) {							/* full 8 characters */
		if (*s == '_') {					/* extended with underscores */
			while (*s-- == '_') ;			/* step back over underscores */
			s++;							/* step forward again */
/*			if (*(s+1) != 'x') *(s+2) = 'x';  */
			if (*s != 'x' && *s != 'X') {	/* 1993/July/1 */
				*(s+1) = 'x';
				return 1;					/* was extended with x */
			}
		}
		else return 0;						/* no underscores in full name */
	}
/*	else if (*s == 'x') return; */
	else if (*s == 'x' || *s == 'X') return 0;	/* 1993/July/1 */
	else {
		s++; *s++ = 'x'; *s = '\0';
		return 1;							/* was extended with x */
	}
	return 0;
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

void forceunder(char *name) {	/* extend name with underlines */
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

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);
	fprintf(output, "%s (%s %s)\n", 
		programversion, date, compiletime);
}

/*		crude hack to guess whether this is a TeX font: */

int istexfont(char *s) {
	if (strncmp(s, "CMINCH", 6) == 0 ||
		strncmp(s, "cminch", 6) == 0) return 0;	/* don't treat as TeX font */
	if (strncmp(s, "CM", 2) == 0 ||
		strncmp(s, "cm", 2) == 0 ||
		strncmp(s, "lasy", 4) == 0 ||
		strncmp(s, "LASY", 4) == 0 ||
		strncmp(s, "line", 4) == 0 ||
		strncmp(s, "LINE", 4) == 0 ||
		strncmp(s, "lcircle", 7) == 0 ||
		strncmp(s, "LCIRCLE", 7) == 0 ||
		strncmp(s, "eu", 2) == 0 || 
		strncmp(s, "EU", 2) == 0 ||
		strncmp(s, "ms", 2) == 0 || 
		strncmp(s, "MS", 2) == 0 			
/*		strncmp(s, "LM", 2) == 0 || */
/*		strncmp(s, "lm", 2) == 0 */
			) return 1;
	else return 0;
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0;		/*  change if copyright changed */
	fprintf(errout, "HASHED %ld\n", hash); 
/*	fprintf(errout, "HASHED %ld `%s'\n", hash, s); */
/*	fflush(errout); */
	(void) _getch(); 
	return hash;
}

/* read PFM source file to try and extract Windows Face Name */
/* tries to extend file name with underscores if needed */
/* tries to look in PFM subdirectory if needed */

int readsource(char *sourcefile) {
	FILE *input;
	char pfmfile[FILENAME_MAX];
	int k, c, i;
	char *s;

	strcpy(pfmfile, sourcefile);
	extension(pfmfile, "pfm");
/*	puts(pfmfile); */				/* debugging */
	if ((input = fopen(pfmfile, "rb")) == NULL) {
/*		strcpy(pfmfile, sourcefile); */
		flushexten(pfmfile);
		forceunder(pfmfile);			/* extend with `_' */
		extension(pfmfile, "pfm");
/*		puts(pfmfile); */				/* debugging */
		if ((input = fopen(pfmfile, "rb")) == NULL) {
			strcpy(pfmfile, sourcefile);			
			extension(pfmfile, "pfm");
			if ((s = strrchr(pfmfile, '\\')) != NULL ||
				(s = strrchr(pfmfile, '/')) != NULL ||
				(s = strrchr(pfmfile, ':')) != NULL) {
				s++;
			}
			else s = pfmfile;
			memmove(s+4, s, strlen(s)+1);
			memcpy(s, "pfm\\", 4);
/*			puts(pfmfile); */				/* debugging */
			if ((input = fopen(pfmfile, "rb")) == NULL) {
		/*		strcpy(pfmfile, sourcefile); */
				flushexten(pfmfile);
				forceunder(pfmfile);			/* extend with `_' */
				extension(pfmfile, "pfm");
/*				puts(pfmfile); */				/* debugging */
				if ((input = fopen(pfmfile, "rb")) == NULL) {
					perror(pfmfile); return -1;
				}
			}
		}
	}
/*	256 bytes of file should be enough to get Face Name */
	for (k = 0; k < 256; k++) buffer[k] = (char) getc(input); 
	fclose(input);
	if (windowsflag != 0) return 0;		/* command line overrides */
	k = (unsigned char) buffer[105];		/* pointer to Driver Info */
/*	We expect k = 210 */
	if (k != 210) fprintf(errout, "Unexpected Face Name position %d\n", k);
	s = windowsname;
	for (i = 0; i < 32; i++) {	/* read out old Windows Face Name */
		c = buffer[k+i];
		*s++ = (char) c;
/*		printf("%c %d ", c, c);	*/
		if (c == 0) break; 
	}
	if (verboseflag != 0)
		printf("Windows Face Name from old PFM: %s\n", windowsname);
	if (strcmp(windowsname, "") != 0) windowsflag = 1;
	return 0;
}

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

/* set up full file name for ini file and check for specified section */
/* e.g. dviwindo = setupinifile ("dviwindo.ini", "[Environment]") */

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

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	if (date == NULL) return;			/* sanity check */
	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
/*	date[20] = '\n'; */
/*	date[21] = '\0'; */
	date[20] = '\0'; 
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Used for Comment Copyright and Notice in AFM file */
/* Gets rid of line termination and reduces length to fit */

void trimcopyright (char *message) {
	char *s;

/*	remove linefeed at end */
	if ((s = strchr(message, '\n')) != NULL) *s = '\0';
/*	remove PS string quoting of \( and \) 98/Aug/9 */
	if ((s = strstr(message, "\\(")) != NULL) strcpy(s, s+1);
	if ((s = strstr(message, "\\)")) != NULL) strcpy(s, s+1);
/*	if (strlen(message) < MAXCOPYRIGHT) return; */
	if (strlen(message) <= MAXCOPYRIGHT) return;

	if (strstr(message, "http:") != NULL) {
/*		If URL given and not enough space, see if can strip other stuff */
		if ((s = strstr(message, "All Rights Reserved. ")) != NULL) 
			strcpy(s, s+21);
		if ((s = strstr(message, "Hinting copyright Y&Y, Inc. ")) != NULL) {
			strcpy(s, "and Y&Y, Inc. ");
			strcat(s, s+28);
		}
	}
	else {
		if (strstr(message, "(C)") != NULL) {	/* strip "Copyright" before "(C)" */
			if (strncmp(message, "Copyright ", 10) == 0)
				strcpy(message, message + 10);
		}
	}
	if (strlen(message) <= MAXCOPYRIGHT) return;

	*(message+MAXCOPYRIGHT) = '\0';	/* truncate at max length */
	if (truncatecopy) return;	/* does nothing fancy beyond this */
	
/*	Try and truncate at last period in string */
	for (;;) {
/*		printf("LOOK FOR DOT in %s\n", message); */	/* debug */
		if ((s = strrchr(message, '.')) == NULL) break;
		*(s+1) = '\0';
		if (strlen(message) <= MAXCOPYRIGHT) return;
		*s = '\0';	// flush to the period so it won't be seen again
	}
/*	Try and truncate at last space in string */
	for (;;) {
/*		printf("LOOK FOR SPACE in %s\n", message); */ /* debug */
		if ((s = strrchr(message, ' ')) == NULL) break;
		*s = '\0';
/*		if (strlen(message) < MAXCOPYRIGHT) return; */
		if (strlen(message) <= MAXCOPYRIGHT) return;
	}
	*(message+MAXCOPYRIGHT) = '\0';	/* truncate at max length */
	return;					/* no period, no space, give up */
}


/* remove newline at end and try and shorten message if needed */
/* used for FontName, FullName, MenuName etc. */

void flushnewline (char *message) {
	char *s;

	if (strlen(message) > MAXCOPYRIGHT)
		printf("WARNING: string possibly too long: %s", message);
	if ((s = strchr(message, '\n')) != NULL) *s = '\0';
}

/* remove bold and italic from font name - assumed to occur at end */
/* this is a little dicey, since there is no consistency in naming ! */

void flushbolditalic (char *name) { 
	char *r, *s, *t, *u;

	/* `Normal' = Book, Roman, Regular, Plain, Medium */
	/*	r = strstr(name, "Roman"); */
	r = strstr(name+1, "Roman");				/* 94/Apr/18 */
	if (r == NULL) r = strstr(name+1, "Book");
	if (r == NULL) r = strstr(name+1, "Plain");
	if (r == NULL) r = strstr(name+1, "Medium");
	if (r == NULL) r = strstr(name+1, "Regular");  /* ??? */ 
	/*	if (r == NULL) r = strstr(name+1, "Light"); */ /* ??? NO */
	/* `Bold' = Bold, Demi (if no bold in family) */
	/*	s = strstr(name, "Bold"); */
	s = strstr(name+1, "Bold");					/* 94/Apr/18 */
	if (s == NULL) s = strstr(name+1, "Demi");	
	if (s == NULL) s = strstr(name+1, "Semibold");	
	/*	if (s == NULL) s = strstr(name+1, "Heavy"); */ /* ??? NO */
	if (s != NULL) boldflag = 1;
	/*  `Italic' = Italic, Oblique, Kursiv */
	/*	t = strstr(name, "Italic"); */
	t = strstr(name+1, "Italic");				/* 94/Apr/18 */
	if (t == NULL) t = strstr(name+1, "Oblique");
	if (t == NULL) t = strstr(name+1, "Kursiv");
	if (t == NULL) t = strstr(name+1, "Obl");	/* 94/Nov/19 */
	if (t != NULL) italicflag = 1;

	/*  avoid lousy font names that contain magic words TimesNewRomanPS ! */
	if (insisthyphen != 0) {
		if (r != NULL && *(r-1) != ' ' && *(r-1) != '-') r = NULL;
		if (s != NULL && *(s-1) != ' ' && *(s-1) != '-') s = NULL;
		if (t != NULL && *(t-1) != ' ' && *(t-1) != '-') t = NULL;
	}

	if (r == NULL && s == NULL && t == NULL) {
		if (verboseflag != 0) printf("Using font name %s as is\n", name);
		return;
	}
	if (verboseflag != 0) printf("Shortening font name %s ", name); 
	u = r;	/* essentially, u = min(r, s, t) - whatever exist */
	if (u == NULL || (s != NULL && s < u)) u = s;
	if (u == NULL || (t != NULL && t < u)) u = t;
	if (u == name) {
		printf("ERROR: in shortening name\n");	/* 94/Apr/18 */
		return;									/* leave it alone */
	}
	if (*(u-1) != ' ' && *(u-1) != '-') {
		/*		if (verboseflag != 0) putc('\n', stdout);
		printf ("WARNING: possible problem shortening %s (%c)\n",
		name, *(u-1)); */
		/*		u++; */		
		*u = '\0';
	}
	else *(u-1) = '\0';				/* previous is hyphen or space */
	if (verboseflag != 0) printf("to %s\n", name);	/* short name */
}

/* deal with BBEHIZ+BaskervilleMT-Italic; Acrobat Partial Font Names */

void stripprefix(char *name) {		/* 97/June/28 */
	int k;
	if (*(name+6) != '+') return;
	for (k = 0; k < 6; k++)			/* prefix must be all uppercase ? */
		if (name[k] < 'A' || name[k] > 'Z') return;
	strcpy(name, name+7);
}

int dokerntable(FILE *input) {
	char chara[MAXCHARNAME], charb[MAXCHARNAME];
	int nchara, ncharb, nkern;
	int k, kern;
	double fkern;

	/*	if (traceflag != 0) showencoding(); */

	/* stick in kerning tables now - if any */

	notkernpairs = 0;
	if(getrealline(line, MAXLINE, input) == NULL) {
		return last;
	}
	/*  forget the following, in case the fool forgot StartKernData ! */
	/*	while (getrealline(infile, line, MAXLINE) > 0) {
	if (strstr(line, "StartKernData") != NULL) break;
	} */
	while(strncmp(line, "StartKernPairs", 14) != 0) {
		if(getrealline(line, MAXLINE, input) == NULL) {
			return last;			/* no Pair Kerns */
		}
	}
	if(getrealline(line, MAXLINE, input) == NULL) {
		return last;				/* no Pair Kerns */
	}

	if (verboseflag != 0 && wantkerns != 0) 
		printf("Building kern table "); /* if any ... */
	k = 0;
	while(strncmp(line, "EndKernPairs", 12) != 0) {
		/*		sscanf(line, "KPX %s %s %lg", &chara, &charb, &fkern); */
		sscanf(line, "KPX %s %s %lg", chara, charb, &fkern);
		fkern = fkern * xscale;
		if (fkern >= 0.0) kern = (int) (fkern + 0.5);
		else kern = (int) (fkern - 0.5);

		if (wantkerns != 0 && (flushzeros == 0 || kern != 0)) {
			/*			nchara = lookup(chara); */
			nchara = lookup(chara, -1);
			/*			ncharb = lookup(charb); */
			ncharb = lookup(charb, -1);
			if (traceflag != 0) {
				printf("%s (%d) %s (%d)\n", chara, nchara, charb, ncharb);
				fflush(stdout);
				(void) _getch();
			}
			if (nchara >= 0 && ncharb >= 0) {
				if (k >= MAXKERNS) k--;				/* to be safe */
				kernpairtable[k].kppair= (unsigned short) ((ncharb << 8) | nchara);
				kernpairtable[k].kpamount = (short) kern;
				k++;
			}
			else {
				if (notkernpairs == 0) putc('\n', stdout);
				if (verboseflag != 0) 
					printf("NOTE: discarding %s (%d) %s (%d) kern %d\n", 
						   chara, nchara, charb, ncharb, kern);
				notkernpairs++;
			}
		}
		if(getrealline(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "ERROR: Unexpected EOF\n");
			exit(15);
		}
	}
	nkern = k;
	if (verboseflag != 0) {
		if (nkern > 0 || notkernpairs > 0) {
			printf("%d kern pairs used ", nkern);
			if (notkernpairs > 0) 
				printf("- %d kern pairs discarded", notkernpairs);
			putc('\n', stdout);
		}
	}

	if (nkern >= MAXKERNS) {
		fprintf(errout, "WARNING: More than %d kern pairs (%d)\n",
				MAXKERNS, nkern);
		/*		nkern = MAXKERNS;  */
	}

	/*	if (verboseflag) printf("Sorting "); */		/* debugging */

	/*	sort them here */ /* make sure sorted on the right order ! */
	qsort(kernpairtable, 
		  (unsigned int) nkern, 
		  sizeof(kernpairtable[0]), 
		  compare);

	/*	if (verboseflag) printf("Sorted "); */		/* debugging */

	writetwo((unsigned int) last, 131);		/* kerntable start */
	writetwo((unsigned int) nkern, 195);	/* KernPairs */
	writetwo((unsigned int) nkern, last);	

	/*	make buffer large enough to avoid having to check size in loop */
	while (last + 2 + 4 * nkern >= buffersize)
		expandbuffer();						/* 1993/Nov/7 */

	for (k = 0; k < nkern; k++) {
		writetwo(kernpairtable[k].kppair, last + 2 + 4 * k);
		kern = kernpairtable[k].kpamount;
		*(buffer + last + 4 + 4 * k) = (unsigned char) (kern & 255);
		*(buffer + last + 5 + 4 * k) = (unsigned char) (kern >> 8);	
		/*		writetwo(kernpair[k].kpamount, last + 2 + 4 * k); */
	}

	/* 	last = last + 2 + 4 * nkern; */
	last += 2 + 4 * nkern;

	/*	if (verboseflag != 0) printf("--- used %d kern pairs\n", nkern); */
	return last;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef CONTROLBREAK
/* void ctrlbreak(void) { */
void ctrlbreak (int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
/*	cleanup(); */
	if (bAbort++ >= 3) exit(3);				/* emergency exit */
/*	abort(); */
	(void) signal(SIGINT, ctrlbreak);
}
#endif

int main(int argc, char *argv[]) {
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	FILE *input;
	int m, pfmlen;
	int k, firstarg = 1; 
	char *s, *t;
	int ansiflagdef, symbolflagdef, oemflagdef;		/* saved state */
	int serifflagdef, fixedflagdef,	decorativedef;	/* saved state */
	int weightdef;
 	time_t ltime;		/* for time and date 96/June/16 */

#ifdef CONTROLBREAK
	(void) signal(SIGINT, ctrlbreak);
#endif

/*	if (setvbuf(stdout, NULL, _IOLBF, 512))	
		printf("Unable to line buffer %s\n", "stdout"); */
/*	if (setvbuf(stderr, NULL, _IOLBF, 512))	
		printf("Unable to line buffer %s\n", "stderr"); */
/*	setvbuf(stderr, NULL, _IONBF, 0); */		/* 97/Sep/13 */

/*	if (firstarg > argc - 1) showusage(argv[0]); */

	if (argc < 2) showusage(argv[0]);

	strncpy(programpath, argv[0], FILENAME_MAX);
/*	removepath(programpath); */
	striptopath(programpath);

	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("VECPATH")) != NULL) vecpath = s;
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("VECPATH")) != NULL) vecpath = s;
	}

	(void) time(&ltime);				/* 96/June/16 */
	s = ctime(&ltime);
	if (s != NULL) {					/* Stick year into default copyright */
		lcivilize(s);
/*		strncpy (*(yanycopyright+14), s, 4); */	/* year */
		if (usenewcopy) strncpy(newcopyright+10, s, 4);	/* year */
		else strncpy (yanycopyright+14, s, 11);	/* year, month, day */
	}

/*	printf("255 is %d or %u or %c\n", (char) 255, (char) 255, (char) 255);
	*s = (char) 255;	k = *s;
	printf("255 is %d or %d\n", k, *s); */	/* test code */

	firstarg = commandline(argc, argv, 1);

	if (firstarg > argc - 1) showusage(argv[0]);

	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

/*	scivilize(compiledate);	 */
	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);
/*	putc('\n', stdout); */

	lowercase(codingvector, codingvector);
	if (strcmp(codingvector, "") == 0) 
		strcpy(codingvector, defaultencoding);		/* default is none */
	if (strcmp(codingvector, "none") == 0)  {
		strcpy(codingvector, "");			/* use encoding in AFM file */
	}

	if (strcmp(codingvector, "") != 0) useafmencoding = 0; 
	else useafmencoding = 1;

/*	if (strcmp(codingvector, "ansi") == 0) ansiflag = 1;
	else if (strcmp(codingvector, "standard") == 0) standardflag = 1;
	else if (strcmp(codingvector, "symbol") == 0) symbolflag = 1; */

	if (strcmp(codingvector, "ansinew") == 0 ||
		strcmp(codingvector, "ansi") == 0) {				/* 1993/May/29 */
		if (symbolflag != 0 || decorative != 0) {
			printf("NOTE: Don't need `s' and `d' when using `%s'\n",
				codingvector);
			symbolflag = 0; decorative = 0;
			ansiflag = 1; oemflag = 0;
		}
	}

/*	Following now redundant perhaps ... */

	if ((strcmp(codingvector, "ansi") == 0 ||
		strcmp(codingvector, "ansinew") == 0) && ansiflag == 0) {
		fprintf(errout, 
			"ERROR: Specified ANSI vector, but not ANSI encoding.\n");
/*		fprintf(errout,	"         Maybe don't use -c=%s?\n", codingvector); */
		fprintf(errout,	"         Will not use encoding vector...\n"); 
		strcpy(codingvector, "");
/*		ansiflag = 1; */ /* ??? */
	}

/*	if (strcmp(codingvector, "") == 0 && ansiflag != 0) { */
	if (strcmp(codingvector, "") == 0 && ansiflag != 0 && decorative == 0) {
		fprintf(errout, 
/* "WARNING: ANSI vector not specified, yet claiming ANSI encoding (default).\n" */
"NOTE: ANSI vector not specified, yet claiming ANSI encoding (default).\n"
			   );
/*		strcpy(codingvector, "ansinew"); */ /* ??? */
/*		fprintf(errout,	"         Will use `ansinew' encoding vector...\n"); */
		fprintf(errout,	"      Will use `ansinew' encoding vector...\n");
		strcpy(codingvector, "ansinew");  /* 95/Jan/2 */
	}

/*	if (useafmencoding != 0 && strcmp(codingvector, "") != 0) {
		fprintf(errout, 
		"ERROR: specified encoding vector AND use of AFM encoding\n"); 
		useafmencoding = 0;
	} */

/*  if (strcmp(codingvector, "") != 0) (void) readvectorfile(char *vector); */

/*	if (checkcopyright(copyright) != 0) {
		fprintf(stderr, "HASH %ld", checkcopyright(copyright));
		exit(1);	
	} */

/*	buffer = (char __far *) _fmalloc(MAXBUFFER); */
	buffer = (char __far *) _fmalloc(INIMEMORY);
	if (buffer == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate memory\n");
		exit(31);
	}
	buffersize = INIMEMORY;

/*	save state that might be reset by entries in AFM file */
	ansiflagdef = ansiflag; 
	symbolflagdef = symbolflag;
	oemflagdef = oemflag;
	serifflagdef = serifflag;
	fixedflagdef = fixedflag;
	decorativedef = decorative;
	weightdef = weight;
	remapflagini = remapflag;

	if (strcmp(sourcefile, "") != 0) readsource(sourcefile); /* 1993/June/25 */

	for (m = firstarg; m < argc; m++) {

/*		May be some problems here if multiple files on command line */
/*		make sure to reset various flags to command line default values ? */
/*		could be changed by entries in AFM file */
/*		serifflag, fixedflag, decorative */
/*		ansiflag, symbolflag, oemflag */
/*		windowsname, windowsflag */		/* doesn't make sense for multiple */

		strcpy(fn_in, argv[m]);
		extension(fn_in, "afm");

/*		if ((s = strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s = strrchr(fn_in, '/')) != NULL) s++;
		else if ((s = strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in; */
		s = removepath(fn_in);			/* 95/Jan/2 */

		if (ansiflag != 0) { /* for ANSI output encoding */
			breakchar = 32;			/* ANSI space */
			defaultchar = 149;		/* ANSI bullet */
		}
		else {				/* for StandardEncoding & Symbol */
			breakchar = 32;			/* Standard space */
			defaultchar = 183;		/* Standard bullet */
		}
		if(istexfont(s)) {
		    if (verboseflag != 0) printf("Treated as TEX font\n");
			texflag = 1; startlate = 0;		/* start at 0 */
			breakchar = 160;	/* ??? */ /* TeX fonts have space here */
			defaultchar = 160;	/* ??? */ /* TeX fonts have space here */
			if ((t = strpbrk(s, "0123456789")) != NULL )
/*				sscanf(t, "%d", &ptsize); */
				sscanf(t, "%u", &ptsize);
			else ptsize = 12;			/* default used by Adobe */
			if (strstr(s, "SS") != NULL ||
				strstr(s, "ss") != NULL) serifflag = 0;
			else serifflag = 1;
/* get following from AFM file instead */
/*			if (strchr(s, 'b') != NULL ||
				strchr(s, 'B') != NULL)	boldflag = 1;
			else boldflag = 0;
			if (strncmp(s, "cminch", 6) == 0 ||
				strncmp(s, "CMINCH", 6) == 0) boldflag = 1;
			if (strchr(s, 'i') != NULL ||
				strchr(s, 'I') != NULL) italicflag = 1;
			else italicflag = 0;
			if (strstr(s, "sl") != NULL ||
				strstr(s, "SL") != NULL) italicflag = 1; */
		}
		else {
/*		    if (verboseflag != 0) printf("NOT treated as TeX font\n"); */
			texflag = 0; ptsize = 12;	/* serifflag = 1; */
		}

/*		if (texflag == 0) startlate = 1;  */ /* change ??? */
		if (ansiflag != 0 && texflag == 0) startlate = 1;
		if (oemflag != 0) startlate = 0;

/*		if (traceflag != 0) printf("Processing file %s\n", fn_in);   */

		if (texflag == 0 && (strstr(fn_in, "sm") != NULL ||
						     strstr(fn_in, "SM") != NULL)) smallcaps = 1;
		else smallcaps = 0;
/*		strcpy(fn_out, argv[m]); */
/*		strcpy(fn_out, s); */
		if (strcmp(outputpath, "") == 0) strcpy(fn_out, s);
		else {					/* 1995/Jan/2 */
			strcpy(fn_out, outputpath);
			strcat(fn_out, "\\");
			strcat(fn_out, s);
		}

		flushexten(fn_out);			/* remove any extension */
/* Add `x' to name - unless (i) command line `x' argument used */
/* OR (ii) if coding specified is -c=none OR  (iii) if no encoding specified */
/* OR if (iv) ansinew specified */	/* 1993/July/10 */
/* OR if (iv) file name too long already */ /* 1993/July/10 */
		if (strcmp(codingvector, "") == 0 ||
			strcmp(codingvector, "none") == 0 ||
			strcmp(codingvector, "ansinew") == 0 ||		/* 1993/July/10 */
			strcmp(codingvector, "ansi") == 0 ||		/* 1993/July/10 */
/*			strlen(fn_out) >= 8 || */					/* 1995/Jan/2 */
			dontexten != 0) {	/* don't extend in this case */
			if (traceflag != 0) 
				printf("dontexten %d, strlen %d, vector %s\n",
					dontexten, strlen(fn_out), codingvector);
		}
		else if (removeX) {
			stripanx(fn_out);
		}
		else if (addanex(fn_out) != 0)
				printf("WARNING: extending name with `x': %s\n", fn_out);
									/* extend with `x' to indicate remapped */

		if (underflag != 0) forceunder(fn_out);  /* fill out with `_' */
		extension(fn_out, "pfm"); /* forceexten(fn_out, "pfm");  */
/*		if (verboseflag != 0) printf("Buffer template - "); */

/*		if (texflag != 0) for(k = 0; k < MAXCHRS; k++) 
								strcpy(encoding[k], defaultencoding[k]); */
/*		else if (ansiflag != 0) for(k = 0; k < MAXCHRS; k++) 
								strcpy(encoding[k], ansiencoding[k]); */
/*		else if (symbolflag != 0) for(k = 0; k < MAXCHRS; k++) 
								strcpy(encoding[k], symbolencoding[k]); */
/*		else for(k = 0; k < MAXCHRS; k++) 
								strcpy(encoding[k], standardencoding[k]); */
		if (strcmp(codingvector, "") != 0) {
								(void) readvectorfile(codingvector); 
/*								showencoding();	*/ /* debugging */
		}
/*		if useafmencoding != 0 the above gets overwritten */
		if (useafmencoding != 0)
			for(k = 0; k < MAXCHRS; k++) strcpy(encoding[k], "");
/*		if (texflag != 0) useafmencoding = 1; */	
/*		if (ansiflag == 0) usefamencoding = 1; */
		copyrightflag = 0;
		preparebuffer();  /* moved down 98/Aug/17 but why ? */
/*		if (verboseflag != 0) printf("Opening AFM file\n"); */
		if((input = fopen(fn_in, "r")) == NULL) {
			flushexten(fn_in);			/* remove any extension */
			flushexten(fn_out);			/* remove any extension */
			forceunder(fn_in);			/* extend with `_' */
			forceunder(fn_out);			/* extend with `_' */			
			extension(fn_in, "afm");	/* put back extension */
			extension(fn_out, "pfm");	/* put back extension */
			if((input = fopen(fn_in, "r")) == NULL) {
				perror(fn_in);
				exit(2);
			}
		}
		if (verboseflag) printf("\n");
		printf("Processing file %s => %s\n", fn_in, fn_out); 
/*		if (traceflag != 0) showencoding(); */

		readafmhead(input);
/*		preparebuffer();  */	/* moved here 98/Aug/17 */

		if (verboseflag != 0) printf("Copying character widths\n");
		copycharwidths(input);
/*		if (traceflag != 0) showencoding(); */
/*		if (verboseflag != 0) printf("Building kern table\n"); */ /* if any */
		pfmlen = dokerntable(input);
		fclose(input);

		if (verboseflag != 0) printf("Writing PFM file\n");
		writepfm(fn_out, pfmlen);
/*		if (verboseflag != 0)  */
			printf("\n");

		windowsflag = 0;		/* don't reuse old Windows Face Name ! */
/* reset state to what was set up from command line 1992/Dec/2 */
		ansiflag = ansiflagdef; 
		symbolflag = symbolflagdef;
		oemflag = oemflagdef;
		serifflag = serifflagdef;
		fixedflag = fixedflagdef;
		decorative = decorativedef;
		weight = weightdef;
		remapflag = remapflagini;		/* 1994/Dec/30 */
	}
	if (argc - firstarg > 1) printf("Processed %d AFM files", 
		argc - firstarg);
#ifndef _WIN32
	if ((m = _fheapchk ()) != _HEAPOK) {		/* 1994/Feb/18 */
		fprintf(stderr, "WARNING: Far heap corrupted (%d)\n", m);
		fprintf(stderr, "WARNING: heap corrupted (%d)\n", m);
		exit(1);
	}
#endif
	if (buffer != NULL) _ffree(buffer);
	return 0;
}

/* Notes, ideas, extensions, shortcomings, problems ... */

/* startlate = 1 when ansiflag != 0 ? */

/* determine bold for tex fonts ? */

/* TeX font AFM lack ascender and descender information !!! */

/* ATM bug always adds 32 to defaultchar and breakchar ? */

/* use afmencoding if ansiflag == 0 and/or if texflag != 0 ? */

/* Since this is all so idiosyncratic may need to do by command line arg */
/* Two issues: weight to write into file & what MS Windows name to give */

/* not clear what the convention for Windows font names REALLY is */
/* that is, should one use FamilyName, FontName or FullName ? */

/* FullName New Century Schoolbook Roman  use FontName NewCenturySchoolBook */

/* Helvetica, Neu, Optima, Grotesk, Avante Garde, Erasmus are sans serif */

/* DDK booklet claims limit on kern pairs is 512 - but apparently NOT */

/* does nothing about track kerning - since no application uses this */

/* variablepitch gets set after it is used ... */

/* use FontBBox info ? for Ascender and Descender ? */

/* change those strstr's != NULL to strncmp == 0 */
/* - so avoid trouble with comments ! */

/* NOTE: ATM crashes if OEM CharSet selected and charmin > 0 */

/* NOTE: need OEM CharSet and Decorative Family to overcome ATM ANSI */

/* NOTE: need Symbol CharSet and Decorative Family to overcome ATM ANSI */

/* typical usage c:\prog\afmtopfm -sd c:\at&t\*.afm */ 

/* typical usage c:\prog\afmtopfm -sd c:\afm\adobe\po*.afm */ /* for Adobe */

/* treat zero widths as widths of 1 ? (cmsy10 problem) - PFM bug */

/* problems when input uses ^M instead ^J ... ? */

/* allow specification of default character ? */

/* insert space at 160 for TeX fonts ? */

/* Typical use: afmtopfm -vsedq c:\afm\adobe\ti*.afm */

/* ATM and PSCRPT.DRV don't seem to agree on whether default and break char */
/* are always with respect to zero or charmin ... */

/* problem with shiftdown + remapping for kern pairs ? do we care */
/* when remapped, should kern pair members also be remapped ? */
/* particularly, when 0 - 31 is removed ? */

/* for fixed width fonts maxcharwidth == avecharwidth */
/* except when fixed width fonts needs to be remapped */

/* check for conflict when using `t' flag ? */

/* if you need space, put kernpairtable out to pasture - but redo qsort */

/* if you need space, put encoding out to pasture - 8192 bytes */

/* INF file: isFixedPitch true/false, Serif true/false, Pi true/false */
/* INF file: CharacterSet, Encoding */

/* Modify to read INF file for auxiliary information if INF file exists ??? */

/* NOTE: Adobe PS Printer driver assumes Windows Face Name directly follows
 `PostScript' --- no padding allowed in between */



/* NOTE: with ATM, Ascent (& Descent) are taken from PFB file BBox, not PFM! */

/* EM = 1000 */		/* InternalLeading non-negative */
/* FontHeight = FontBBox.yur - FontBBox.yll;
/* InternalLeading = max(0, FontHeight - EM) */
/* PFM Ascent = EM + FontBBox.yll + InternalLeading */

/* So PFM Ascent = FontBBox.yur		if  FontHeight >= EM */
/* So PFM Ascent = FontBBox.yll + EM	if  FontHeight <  EM */

/* This is large, need to compile using optimizations of run out of space */

/* CharSet values */
/* #define ANSI_CHARSET            0 */
/* #define DEFAULT_CHARSET         1 */
/* #define SYMBOL_CHARSET          2 */
/* #define SHIFTJIS_CHARSET        128 */
/* #define HANGEUL_CHARSET         129 */
/* #define GB2312_CHARSET          134 */
/* #define CHINESEBIG5_CHARSET     136 */
/* #define OEM_CHARSET             255 */
/* #if(WINVER >= 0x0400) */
/* #define JOHAB_CHARSET           130 */
/* #define HEBREW_CHARSET          177 */
/* #define ARABIC_CHARSET          178 */
/* #define GREEK_CHARSET           161 */
/* #define TURKISH_CHARSET         162 */
/* #define VIETNAMESE_CHARSET      163 */
/* #define THAI_CHARSET            222 */
/* #define EASTEUROPE_CHARSET      238 */
/* #define RUSSIAN_CHARSET         204 */
/* #define MAC_CHARSET             77 */
/* #define BALTIC_CHARSET          186 */

/* Font Pitch (low 4 bits) */
/* #define DEFAULT_PITCH           0 */
/* #define FIXED_PITCH             1 */
/* #define VARIABLE_PITCH          2 */
/* #if(WINVER >= 0x0400) */
/* #define MONO_FONT               8 */
/* #endif WINVER >= 0x0400			 */

/* Font Families (high 4 bits) */
/* #define FF_DONTCARE         (0<<4) */  /* Don't care or don't know. */
/* #define FF_ROMAN            (1<<4) */  /* Variable stroke width, serifed. */
									/* Times Roman, Century Schoolbook, etc. */
/* #define FF_SWISS            (2<<4) */  /* Variable stroke width, sans-serifed. */
									/* Helvetica, Swiss, etc. */
/* #define FF_MODERN           (3<<4) */  /* Constant stroke width, serifed or sans-serifed. */
									/* Pica, Elite, Courier, etc. */
/* #define FF_SCRIPT           (4<<4) */  /* Cursive, etc. */
/* #define FF_DECORATIVE       (5<<4) */  /* Old English, etc. */

/* Use -Q to suppress both encoding string and application signature in PFM */

/* Copyright info limited to 60 chars (including null) */
/* Gets copyright info from `Notice ...' in AFM file */
/* Gets copyright info from `Comment Copyright ...' in AFM file */
/* Tries to truncate at period (or space) if copyright info too long */
/* Last one encountered in AFM file is the one used. */
/* Sets copyrightflag when it has one such */
/* Default if nothing in AFM is Y&Y copyright message */

/* For Courier use		afmtopfm -v -Q co*.afm */
/* For Times used		afmtopfm -v -Q ti*.afm */
/* For Helvetica use	afmtopfm -v -Q -h hv*.afm */
/* For Symbol use		afmtopfm -v -Q -s -f=space sy______.afm */
