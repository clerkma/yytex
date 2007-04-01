/* Copyright 1990,1991,1992,1993,1994,1995,1996,1997,1998,1999,2000 Y&Y, Inc.
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

/* Make crude AFM file from SCR file */

/* Crude code for analysing width and kern tables in FOND resource */
/* Outputs AFM file extract */
/* If given another argument, assumes this is AFM file for new widths */
/* in this case output AFM file is suppressed */
/* a modified screen file is made instead */
/* Output file same as input except extension "new" */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/* #define MAXFILENAME 100 */

/* #define PATHLEN_MAX 64 */
/* #define PATHLEN_MAX 128 */

#define MAXLINE 256
#define CHARNAME_MAX 32

#define MAXENCODING 40
#define MAXCHRS 256
#define MACBINARYHEAD 128
#define MAXCHRS 256

char buffer[MACBINARYHEAD * 2];			/* buffer for MacBinary head */

// char programpath[PATHLEN_MAX] = "c:\\dvipsone"; 
// char programpath[FILENAME_MAX] = "c:\\yandy\\dvipsone";
char *programpath="c:\\yandy\\dvipsone";

int verboseflag = 0;			// -v
int traceflag = 0;				// -t
int showhidden = 0;				// -h
int showzerowidths = 0;			// -z
int shownfnttables = 0;			// -n
int showbitmaps = 0;			// -b
int changewidths = 0;			// -m
int changeResID = 0;			// -f
int remapflag = 0;				// -l

int showwidthsflag = 1;			// turned off by -m and -f
int showkernsflag = 1;			// turned off by -m and -f

int const trymultiple = 1;		/* try extract several widths tables */
int const tryzerofirst = 1;		/* Try zero offset first */
int const showcolor=1;			/* show label/color of file */
int const shownfntflag = 1;
int const showfontflag = 1;

FILE *errout=stdout;		/* where error output goes - 97/Sep/23 */

int nomacbinary = 0;

long offset = 0;			/* offset into file */

int isfixedpitch;			/* non-zero if fixed pitch font */

int basemacencoding = 0;	/* use Mac encoding as base */ /* -e on command line */

int usemacencoding = 0;		/* non-zero => use Mac encoding */ 
int nonmacencode = 0;		/* non-zero => need non-Mac encoding vector */

// char encodingvecname[MAXENCODING]="";	/* first line encoding vector file */
char *encodingvecname=NULL;				/* from first line encoding vector file */

// char *vectorpath = "";					/* directory for encoding vectors */
char *vectorpath=NULL;					/* directory for encoding vectors */

// char *vector="";
char *vector=NULL;

int fontchrs = MAXCHRS;		/* set to 128 if backwardflag non-zero */

int pass = 0;

char ResourceType[4+1];

char ResName[256];

char line[MAXLINE];

unsigned int FONDMinID = 3072;		/* Inside MacIntosh Volume VI - pg 13-8 */
unsigned int FONDMaxID = 16000;		/* Inside MacIntosh Volume VI - pg 13-8 */

/* char *programversion = "SCRtoAFM conversion utility version 1.1"; */
/* char *programversion = "SCRtoAFM conversion utility version 1.2.1"; */
char *programversion = "SCRtoAFM conversion utility version 1.2.2";

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\
Copyright (C) 1990-2000  Y&Y, Inc.  http://www.YandY.com\
";

/* Copyright (C) 1990--1995  Y&Y. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 14251529 */
/* #define COPYHASH 10345698 */
/* #define COPYHASH 5769561 */
/* #define COPYHASH 11982872 */
/* #define COPYHASH 11982872 */
/* #define COPYHASH 7031586 */
#define COPYHASH 5664704

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

int wantcpyrght = 1;

char *labels[] = {
/* "None", "Essential", "Hot", "In Progress", */
/* "Cool", "Personal", "Project 1", "Project 2" */
"None", "", "Project 2", "",
"Project 1", "", "Personal", "",
"Cool", "", "In Progress", "",
"Hot", "", "Essential", "", 
};

char *colors[] = {
/* "Gray", "Blue", "Red", "Magenta", */
/* "Green", "Yellow", "Light Green", "Lavender" */
"Gray", "", "Red", "",
"Light Green", "", "Yellow", "",
"Green", "", "Magenta", "",
"Red", "", "Blue", "", 
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// #define MAXSUFFIX 16

// #define MAXSUFFIX 32
#define MAXSUFFIX 64

// char BaseName[32];				/* base name */

char BaseName[256];					/* should not be more than 32 - but for safety */

// char SuffixString[MAXSUFFIX][32];	/* suffix strings */

char SuffixString[MAXSUFFIX][32];	/* should not be more than 24 - but ... */

unsigned int nextsuffix;			/* count of suffix strings */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// need to leave space in these tables for "missing glyph" and for table termination

static int charwidths[MAXCHRS+2];	/* new widths from AFM ? */

static int locations[MAXCHRS+2];

static int offsets[MAXCHRS+2];

static int widths[MAXCHRS+2];		/* Adobe coordinates */

static char encoding[MAXCHRS][32];

char *macencoding[] = { 
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
"ampersand", "quotesingle", 
"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period",
"slash", 
"zero", "one", "two", "three", "four", "five", "six", "seven", 
"eight", "nine", "colon", "semicolon", "less", "equal", "greater",
"question", 
"at", "A", "B", "C", "D", "E", "F", "G", 
"H", "I", "J", "K", "L", "M", "N", "O",
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum",
"underscore", 
"grave", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", 
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "delete", 
"Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis",
"Udieresis", "aacute", 
"agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla",
"eacute", "egrave", 
"ecircumflex", "edieresis", "iacute", "igrave", "icircumflex", "idieresis",
"ntilde", "oacute", 
"ograve", "ocircumflex", "odieresis", "otilde", "uacute", "ugrave",
"ucircumflex", "udieresis", 
"dagger", "degree", "cent", "sterling", "section", "bullet", "paragraph",
"germandbls", 
"registered", "copyright", "trademark", "acute", "dieresis", "notequal",
"AE", "Oslash", 
"infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu",
"partialdiff", "summation", 
"product", "pi", "integral", "ordfeminine", "ordmasculine", "Omega", "ae",
"oslash", 
"questiondown", "exclamdown", "logicalnot", "radical", "florin",
"approxequal", "Delta", "guillemotleft", 
"guillemotright", "ellipsis", "nbspace", "Agrave", "Atilde", "Otilde", "OE",
"oe", 
"endash", "emdash", "quotedblleft", "quotedblright", "quoteleft",
"quoteright", "divide", "lozenge", 
"ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft",
"guilsinglright", "fi", "fl", 
"daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase",
"perthousand", "Acircumflex", "Ecircumflex", "Aacute", 
"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave",
"Oacute", "Ocircumflex", 
"apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi",
"circumflex", "tilde", 
"macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek",
"caron"};

/* "delete" should really be "" */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned short int ureadtwo (FILE *input) {
	return (unsigned short) ((getc(input) << 8) | getc(input));
} 

short int sreadtwo (FILE *input) {
	return (short) ((getc(input) << 8) | getc(input));
}

unsigned long ureadthree (FILE *input) {
	unsigned int c, d, e;
	c = getc(input);	d = getc(input);
	e = getc(input);	
	return ((((unsigned long) c << 8) | d) << 8) | e;
} 

unsigned long ureadfour (FILE *input) {
	unsigned int c, d, e, f;
	c = getc(input);	d = getc(input);
	e = getc(input);	f = getc(input);
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
} 

void uwritetwo (unsigned int n, FILE *output) {
	int c, d;
	c = (n >> 8) & 255;  d = n & 255;
	putc(c, output); putc(d, output);
} 

void swritetwo (int n, FILE *output) {
	int c, d;
	c = (n >> 8) & 255;  d = n & 255;
	putc(c, output); putc(d, output);
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

/* imported from mactopfa */

int checkstart(void) {		/* see whether could be MacBinary header */
	int k, n;
	if (buffer[0] != 0) return 0;			/* should be zero */
	if ((n = buffer[1]) == 0) return 0;		/* should be between 0 and 64 */
	if ((n = buffer[1]) > 64) return 0;		/* should be between 0 and 64 */
	for (k = 0; k < n; k++) {
		if (buffer[k+2] == 0) return 0;		/* should be part of file name */
	}
	return -1;
}

#define SPLITPOINT 52

unsigned long uscanfour (int n) {
	unsigned int c, d, e, f;
	c = buffer[n];		d = buffer[n+1];
	e = buffer[n+2];	f = buffer[n+3];
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
} 

int reasonable(int k) {				/* return non-zero if passes test */
	int flag=-1;
	unsigned long dataoffset, mapoffset, datalength, maplength;
	dataoffset = uscanfour(k);
	mapoffset = uscanfour(k+4);
	datalength = uscanfour(k+8);
	maplength = uscanfour(k+12);

	if (dataoffset != 256) flag = 0;
	if (mapoffset == 0 || mapoffset > 16777216) flag = 0;
	if (datalength == 0 || datalength > 16777216) flag = 0;
	if (maplength == 0 || maplength > 16777216) flag = 0;
	
	if (traceflag)
		printf("k %d ResDataOff %lu ResMapOff %lu ResDataLen %lu ResMapLen %lu %s\n",
			   k, dataoffset, mapoffset, datalength, maplength, flag ? "OK" : "NOT REASONABLE");

	return flag;
}

/* look for 0 0 1 0 */

int guessstart(int nmax) {	/* try and locate resource fork start pattern */
	int k;

	if (tryzerofirst) {
		if (buffer[0] == 0 && buffer[1] == 0 &&
			buffer[2] == 1 && buffer[3] == 0) {
			if (reasonable(0) != 0)
				return 0;
		}
	}
	for (k = SPLITPOINT; k < nmax; k++) {
		if (buffer[k] == 0 && buffer[k+1] == 0 &&
			buffer[k+2] == 1 &&	buffer[k+3] == 0) {
			if (reasonable(k) != 0)
				return k;
		}
	}
	for (k = 0; k < SPLITPOINT; k++) {
		if (buffer[k] == 0 && buffer[k+1] == 0 &&
			buffer[k+2] == 1 &&	buffer[k+3] == 0) {
			if (reasonable(k) != 0)
				return k;
		}
	}
	return -1;
}

unsigned long DataForkLen=0, ResForkLen=0;

int readmacheader(FILE *input) {
	int c, k, n;
	long m;
	char *s;

	offset = 0;

	if (nomacbinary) return 0;

/*	first try and read MacBinary file header of 128 bytes */	
	s = buffer;
	for (k = 0; k < MACBINARYHEAD; k++) *s++ = (char) getc(input);

/*	if (buffer[0] == 0 && buffer[1] == 0) { */
	if (checkstart() == 0) {
		fprintf(errout, 
			"WARNING:  There appears to be no MacBinary header\n");
		nomacbinary++;
/* modified 1993/Feb/5 to read in more right away */
/*		m = guessstart(MACBINARYHEAD); */
/*		if (m < 0) {	*/
			for (k = 0; k < MACBINARYHEAD; k++) *s++ = (char) getc(input);
			m = guessstart(MACBINARYHEAD * 2);
			fseek(input, MACBINARYHEAD, SEEK_SET);	/* go back */
/*		} */
		if (m >= 0) {
			fprintf(errout, 
				"WARNING:  Guessing that resource fork starts at %d\n",	m);
			offset = m;
			fseek(input, m, SEEK_SET);			/* rewind */				
		}
		else if (buffer[0] == 0 && buffer[1] == 0) { 
			fprintf(errout, 
				"WARNING:  Assuming this file is the resource fork only\n");
			fseek(input, 0, SEEK_SET);			/* rewind */
		}
		else {
			fprintf(errout,
				"ERROR: Sorry, don't understand this file at all\n");
/*			exit(73); */
			fclose(input);
/*			continue; */
			return -1;		/* ??? */
		}
	}

	if (nomacbinary) return 0;				

	if (offset > 0) {
		fseek (input, offset, SEEK_SET);
		return 0;
	} 

/*	if (offset > 0) fseek (input, offset, SEEK_SET); 	else */

	rewind(input);
	
	if ((c = getc(input)) != 0) {
		fprintf(errout, "ERROR: Not MacBinary Header --- "); 
		if (c == ' ' || c == '\r' || c == '\n')
			fprintf(errout, "probably a DOS OEM text file\n");
		else if (c == 'M')
			fprintf(errout, "possibly a MicroSoft Windows EXE file\n");
		else putc('\n', errout);
/*		return -1; */
	}

	n = getc(input);
	if (n == 0) {
	printf("Apparently no MacBinary header - assuming resource fork only\n");
		nomacbinary++;
		rewind(input);
		return 0;
	}
	else if (n > 63) {
		fprintf(errout, 
			"ERROR: Ridiculously long (%d) file name in header\n", n); 
/*		return -1; */
	}

	if (verboseflag) printf("MacIntosh File Name: ");
	for (k = 0; k < n; k++) {
		c = getc(input);
		if (verboseflag && n <= 63) putc(c, stdout);
	}
	if (verboseflag) putc('\n', stdout);
	for (k = n+1; k < 64; k++) (void) getc(input);

	if (verboseflag) printf("File Type: ");
	for (k = 0; k < 4; k++) {	/* skip over file type */
		c = getc(input); 
		if (verboseflag) putc(c, stdout);
	}
/* 	putc(' ', stdout); */
	if (verboseflag) printf(" File Creator: ");
	for (k = 0; k < 4; k++) {	/* skip over file creator */
		c = getc(input); 
		if (verboseflag) putc(c, stdout);
	}
	if (verboseflag) putc('\n', stdout);
	c = getc(input);			/* file flags 73 */
	if (c != 0) printf("File Flags:     %d\n", c);	/* 1993/Aug/27 */
	c = getc(input);			/* file label 74 */
	if (c != 0) {
/*		fprintf(errout, "ERROR: Not MacBinary Header (File Flags <> 0)\n"); */
/*		if (verboseflag) printf("WARNING: File Flags %d\n", c);  */
		if (showcolor && c >= 0 && c < 16
				&& strcmp(labels[c], "") != 0) {
				printf("File Label: %s (%s)\n",	labels[c], colors[c]);
		}
		else printf("File Label:     %d\n", c);
	}
	for (k = 0; k < 6; k++) {	/* desk top location */
		c = getc(input); 
	}
	c = getc(input);			/* protected flags 81 */
	if ((c = getc(input)) != 0) {
/*		fprintf(errout, "ERROR: Not MacBinary Header (Protected Flag <> 0)\n");  */
		if (verboseflag) printf("WARNING: Protected Flag %d\n", c); 
	}
	DataForkLen = ureadfour(input);
	if (DataForkLen > 0) {
		fprintf(errout, "ERROR: Non-zero Data Fork Length\n");
//		return -1;
	}
	ResForkLen = ureadfour(input);
	for (k = 0; k < 8; k++) {		/* skip creation & modification dates */
		c = getc(input); 
	}	
	for (k = 99; k < 128; k++) {	/* skip rest of header */
		c = getc(input); 
	}
	if (verboseflag) printf("DataForkLen %ld ResForkLen %ld\n", 
			DataForkLen, ResForkLen);
	if (ResForkLen == 0 && DataForkLen == 0) {
		fprintf(errout, "ERROR: Empty Data and Resource Forks\n");
		return -1;
	}
//	if (ResForkLen > 4000000 && DataForkLen > 4000000)
	if (ResForkLen > 4000000 || DataForkLen > 4000000) {
		fprintf(errout, "ERROR: Ridiculously large Data or Resource Fork\n");
		return -1;
	}
	return 0;
}

long ResIDptr, FONDFamIDptr, FONDAscentptr, FONDBBoxptr;

long FONDResIDptr, NFNTResIDptr, FONTResIDptr;

long FONDWidthEnd, FONDKernEnd, FONDStyleEnd;

unsigned long ResMapOffset=0, ResDataOffset=0, ResDataLen=0, ResMapLen=0;

void readresourceheader(FILE *input) {
/*	int c; */
	int k;

	ResDataOffset = ureadfour(input);
	ResMapOffset = ureadfour(input);
	ResDataLen = ureadfour(input);
	ResMapLen = ureadfour(input);	

	if (verboseflag) 
		printf("ResDataOff %ld ResMapOff %ld ResDataLen %ld ResMapLen %ld\n",
			ResDataOffset, ResMapOffset, ResDataLen, ResMapLen);
	for (k = 0; k < 112; k++) {			/* skip over rest of header */
		(void) getc(input);
	}
	for (k = 0; k < 128; k++) {			/* skip over user data */
		(void) getc(input);
	}
}

unsigned int TypeListOffset, NameListOffset, NumberTypes;

long BeginTypeList;

unsigned int NumberOfThisType, ReferenceListOffset;

unsigned int NameStringOffset;

long FONDDataOffset, NFNTDataOffset, FONTDataOffset;

unsigned long ResDataOff;

unsigned int ResID;

unsigned int FONDResID, NFNTResID, FONTResID;

unsigned int ResAttribute;		/* not accessed */

unsigned int newResID;

unsigned int FONDFlags, FONDFamID;

int FONDFirst, FONDLast;

int FONDAscent, FONDDescent, FONDLeading, FONDWidMax;

unsigned long FONDWTabOff, FONDKernOff, FONDStylOff;

long FONDResStart;

long NFNTResStart;

long WidthStart;

long KernStart;					/* debug only */

long OffsetsStart;				/* start of Offset Table */

unsigned long FONDDataLen;		/* not accessed */

unsigned long NFNTDataLen;

unsigned int NumAssociations;

unsigned int FONDVersion=2;		/* read from file - should be 2 */

unsigned int NumFontBBox, NumOffsets;

unsigned long FONDBBoxOffset;

int xll, yll, xur, yur;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void readfonddata(FILE *, FILE *);

void readnfntdata(FILE *, long, int);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

long ResMapPos;

void readresourcemap(FILE *input, FILE *output) {
	unsigned int k, l, m, n;
/*	int c; */
	char *s;
	unsigned long flag;
/*	long resmappos; */
	long current, present, finger;	/* redefinitions shadow global */
	
/*	if (nomacbinary == 0) resmappos =  ResMapOffset + MACBINARYHEAD;
	else resmappos =  ResMapOffset; */
	if (nomacbinary == 0) ResMapPos =  offset + ResMapOffset + MACBINARYHEAD;
	else ResMapPos =  offset + ResMapOffset;	
/*	if (fseek(input, ResMapOffset + MACBINARYHEAD, SEEK_SET) != 0) { */
	if (fseek(input, ResMapPos, SEEK_SET) != 0) { /* OK */
		fprintf(errout, "Seek Error in %s\n", "readresourcemap");
		exit(7);
	}
	for (k = 0; k < 16; k++) {		/* skip copy of resource header */
		(void) getc(input);
	}
	for (k = 0; k < 4 + 4; k++) {	/* skip more junk */
		(void) getc(input);
	}	
	TypeListOffset = ureadtwo(input);
	if (TypeListOffset != 28) 
		printf("TypeListOff %d <> 28\n", TypeListOffset);
	NameListOffset = ureadtwo(input);
	if (verboseflag) printf("TypeListOff %d NameListOff %d\n",
		TypeListOffset, NameListOffset);

	BeginTypeList = ftell(input);
	if (verboseflag) printf("BeginTypeList %ld ", BeginTypeList);
	NumberTypes = ureadtwo(input) + 1;
	if (verboseflag) printf("NumberTypes %d ", NumberTypes);
	if (NumberTypes > 255) { 
		fprintf(errout, "ERROR: Absurdly many (%d) NumberTypes\n", 
			NumberTypes);
		return;			
	}
	if (verboseflag) putc('\n', stdout);
	for (k = 0; k < NumberTypes; k++) {
//		printf("START of OUTER LOOP NumberTypes\n");
//		fflush(stdout);
		s = ResourceType;
		for (m = 0; m < 4; m++) {
			*s++ = (char) getc(input);
		}
		*s = '\0';
		NumberOfThisType = ureadtwo(input) + 1;
		if (verboseflag) putc('\n', stdout);
		if (verboseflag) printf("ResourceType `%s' NumberOfThisType %d ", 
			ResourceType, NumberOfThisType);
		if (NumberOfThisType > 255) {
			fprintf(errout, "ERROR: Absurdly many (%d) NumberOfThisType\n", 
				NumberOfThisType);
			return;
		}
		ReferenceListOffset = ureadtwo(input);
		if (verboseflag) printf("ReferenceListOffset %d\n", ReferenceListOffset);
		current = ftell(input);
/*		go look at this resource */
		if (fseek(input, BeginTypeList+ ReferenceListOffset,  /* OK */
			SEEK_SET) != 0) {
			fprintf(errout, "Seek Error in %s\n", "readresourcemap");
			exit(7);
		}
		if (verboseflag) putc('\n', stdout);
		for (m = 0; m < NumberOfThisType; m++) {
//			printf("START of INNER LOOP NumberOfThisType\n");
//			fflush(stdout);
/*			if (verboseflag) putc('\n', stdout);  */
			ResIDptr = ftell(input);
			ResID = ureadtwo(input);
			if (verboseflag) printf("ResID %d ", ResID);
			NameStringOffset = ureadtwo(input);
/*			if (NameStringOffset != 65535)
				printf("NameStringOffset %d ", NameStringOffset); */
			ResAttribute = getc(input);
			ResDataOff = ureadthree(input);	
			if (verboseflag) printf("ResDataOff %ld ", ResDataOff);
			flag = ureadfour(input);		/* reserved */
			if (flag && showhidden) 
				printf("Reserved Word %08X ", flag);  
			if (NameStringOffset != 65535) {
				present = ftell(input);
/*				printf("present %ld ", present); */

/*				if (fseek(input, 
					128 + ResMapOffset + NameListOffset + NameStringOffset, 
						SEEK_SET) != 0) { */
				if (fseek(input, 
					ResMapPos + NameListOffset + NameStringOffset,  /* OK */
						SEEK_SET) != 0) {
					fprintf(errout, "Seek Error in %s\n", "readresourcemap");
					exit(7);
				}					
				n = getc(input);
/*				printf("n %d ", n); */
				s = ResName;
				for (l = 0; l < n; l++) *s++ = (char) getc(input); 
				*s = '\0';
				if (verboseflag) printf("ResName `%s' ", ResName);
				if (fseek(input, present, SEEK_SET) != 0) { /* OK */
					fprintf(errout, "Seek Error in %s\n", "readresourcemap");
					exit(7);
				} 
			}
			if (strcmp(ResourceType, "FOND") == 0) {
				FONDDataOffset = ResDataOff;
				FONDResID = ResID;
				FONDResIDptr = ResIDptr;
				finger = ftell(input);

				putc('\n', stdout);
//				putc('\n', stdout);
				printf("FOND Resource Number %d:\n", m+1);

				readfonddata(input, output);

				putc('\n', stdout);
				printf("FONDResID %u at %ld - FONDFamID %u at %ld\n", 
					FONDResID, FONDResIDptr, FONDFamID, FONDFamIDptr);
				if (FONDResID < FONDMinID) 
					printf("Apple Font ? (%u < %u)\n", FONDResID, FONDMinID);
				else if (FONDResID > FONDMaxID) {
					fprintf(errout,
						"WARNING: FONDResID out of range for Latin font (%u > %u)\n",
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
					}
				}

				fseek(input, finger, SEEK_SET);  /* OK */
			}
			if (strcmp(ResourceType, "NFNT") == 0) {
				NFNTDataOffset = ResDataOff;
				NFNTResID = ResID;
				NFNTResIDptr = ResIDptr;
				finger = ftell(input);

				if (verboseflag)
					printf("%s %u at %ld \n", "NFNTResID", NFNTResID, NFNTResIDptr);

				if (shownfntflag || traceflag) 
					readnfntdata(input, NFNTDataOffset, 1);

				putc('\n', stdout);
/*				if (verboseflag)
					printf("%s at %ld\n", "NFNTResID", NFNTResIDptr); */

				fseek(input, finger, SEEK_SET);		/* OK */
			}

			if (strcmp(ResourceType, "FONT") == 0) {
				FONTDataOffset = ResDataOff;
				FONTResID = ResID;
				FONTResIDptr = ResIDptr;
				finger = ftell(input);

				if (verboseflag)
					printf("%s %u at %ld \n", "FONTResID", FONTResID, FONTResIDptr);

				if (showfontflag || traceflag) 
					readnfntdata(input, FONTDataOffset, 0); 

				putc('\n', stdout);
/*				if (verboseflag)
					printf("%s at %ld\n", "FONTResID", FONTResIDptr); */

				fseek(input, finger, SEEK_SET);		/* OK */
			}

			if (verboseflag) putc('\n', stdout);
		}	/* for NumberOfThisType */

/*		back to scanning Type list */
		if (fseek(input, current, SEEK_SET) != 0) {	/* OK */
			fprintf(errout, "Seek Error in %s\n", "readresourcemap");
			exit(7);
		}
	}	/* for NumberType */
	if (verboseflag) printf("Finished reading resource map\n");
	fflush(stdout);
}

/* Convert from Mac (4096 per em) to Adobe (1000 per em) measurements */

int mapwidth(int width) {
/*	if (FONDVersion < 2) { */
/*		sign plus magnitude ??? old FOND versions */
/*		if ((width & 32768) != 0)  {			 */
		if ((width & 0x8000) != 0 && (width & 0x4000) == 0)  {				
			width = width & 32767;
			width = -width;
		} 
/*	} */
	if (width < 0) return (- mapwidth( - width));
	return (int) (((long) width * 1000 + 2047) / 4096);
}

/* Convert from Adobe (1000 per em) to Mac (4096 per em) measurements */

int unmapwidth(int afmwidth) {
	if (afmwidth < 0) return (- unmapwidth( - afmwidth));
	return (int) (((long) afmwidth * 4096 + 499) / 1000);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void copydata(int to, int from) {
	if (strcmp(encoding[to], "") != 0) {
		fprintf(errout, "ERROR: Attempt to overwrite char %d\n", to);
		return;
	}
	if (strcmp(encoding[from], "") == 0) {
		fprintf(errout, "ERROR: Attempt to remap blank char %d\n", from);
		return;
	}
	strcpy(encoding[to], encoding[from]);
	widths[to] = widths[from];
}

int getline(FILE *infile, char *buff, int nmax) {
	int c, k=0;
	char *s=buff;

	while ((c = getc(infile)) != '\n' && c != EOF) {
		*s++ = (char) c;
		k++;
		if (k >= nmax) {
			*s = '\0';
			fprintf(errout, "ERROR: Line too long: (%d) %s (%d)\n", strlen(buff), buff);
			exit (13);
		}
	}
	if (c != EOF) {
		*s++ = (char) c;
		k++;
	}
	*s = '\0';
/*	if (traceflag) printf("%s", buff); */
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

int readencodingsub(char *vector) {
	char fn_vec[FILENAME_MAX];
	char charcode[CHARNAME_MAX];
	FILE *fp_vec;
	char *s;
	int n, k;

	if (vector == NULL) return -1;
	
/*	First try current directory */ /* added 1992/Jan/30 */
	strcpy(fn_vec, vector);
	forceexten(fn_vec, "vec");
	if ((fp_vec = fopen(fn_vec, "r")) == NULL) {

/*		if vector path specified use it - otherwise try current directory */
//		if (strcmp(vectorpath, "") != 0) {
		if (vectorpath != NULL) {
			strcpy(fn_vec, vectorpath);
			strcat(fn_vec, "\\");
		}
		else strcpy(fn_vec, "");
		
		strcat(fn_vec, vector);
		forceexten(fn_vec, "vec");

		if ((fp_vec = fopen(fn_vec, "r")) == NULL) {
/*			and if that fails try program path */
			strcpy(fn_vec, programpath); 
			strcat(fn_vec, "\\");
			strcat(fn_vec, vector);
			forceexten(fn_vec, "vec"); 
			if ((fp_vec = fopen(fn_vec, "r")) == NULL) {
				if (pass == 0) {
					fprintf(errout, "ERROR: Can't open encoding vector %s\n", fn_vec);
					perror(fn_vec); 
				}
				return -1;
			}
		}
	}
	if (verboseflag) printf("Using encoding vector: %s\n", fn_vec);

	n=0;
//	strcpy(encodingvecname, "");
	(void) getline(fp_vec, line, MAXLINE);
	if ((s = strstr(line, "Encoding:")) != NULL) {
		s += strlen("Encoding:");
		while (*s++ <= ' ') ; s--;
//		strncpy(encodingvecname, s, MAXENCODING);
		encodingvecname = strdup(s);
		stripreturn(encodingvecname);
		if (verboseflag)
			printf("EncodingVectorName: %s\n", encodingvecname); 
/*		(void) getrealline(fp_vec, line, MAXLINE); */
	}

	while (getrealline(fp_vec, line, MAXLINE) > 0) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf(line, "%d %s", &k, charcode) < 2) {
			fprintf(errout, "WARNING: Don't understand encoding line: %s", line);
		} 
		else strcpy(encoding[k], charcode);
		n++;
	}
	if (n != 128) {			/* not in future TeX 3.0 ? */
		printf("Encoding vector has (%d) elements\n", n);
	}
	if (ferror(fp_vec) != 0) {
		fprintf(errout, "WARNING: Error in encoding vector\n");
		perror(fn_vec);
	}
	else fclose(fp_vec);
	return 0;			/* normal exit */
}

void remapencoding(void) { 
	int k, count = 0;
	
	for (k = 0; k < 10; k++) {
		if (strcmp(encoding[k+161], "") != 0) count++;
		copydata(k+161, k);
	}
	for (k = 10; k < 32; k++) {
		if (strcmp(encoding[k+163], "") != 0) count++;
		copydata(k+163, k);	
	}
	copydata(195, 32);
	copydata(196, 127);
	copydata(128, 32);		/* ??? */
	if (count > 0) fprintf(errout, 
		"WARNING: Remapping collided with %d characters\n", count);
}

int readencoding(char *vector) {
	if (vector == NULL) {
		return -1;
	}
	if (readencodingsub(vector) != 0) {
		if (strcmp(vector, "standard") != 0) {
			fprintf(errout, "Using Adobe StandardEncoding instead\n");
			if(readencodingsub("standard") != 0) {
/*				exit(21); */
			}
		}
/*		else exit(19); */
	}
	if (remapflag) remapencoding();
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int styles[48], newstyle[48];

/* following takes into account that bit 3 (underline) is dropped */

unsigned int mapstyle(unsigned int n) {
	return ((n & 3) | ((n >> 3) << 2));
}

#define MAXINDEX 32

/* generate particular style name */

void showstyle(FILE *input, FILE *output, int index, int flag) {
	unsigned int na;	/* NA */
	unsigned long nb;	/* NA */
/*	int stylecount;	*/	/* NA */
	int c, k, l, n, np, nstrings, style;
	int styleindex=0;
	int indexes[MAXINDEX];
	unsigned int i;
	long oldpos;
	long reserved;
//	char *s;
	
	index = mapstyle(index);

	for (k = 0; k < MAXINDEX; k++) indexes[k] = 0;

	oldpos = ftell(input);
	if (fseek(input, FONDResStart + FONDStylOff, SEEK_SET) != 0) {  /* OK */
		fprintf(errout, "Seek Error in %s\n", "showstyle");
	}
	na = ureadtwo(input);			/* Font Class - page 29 of LaserWriter */
	if (NumOffsets != 1) return;	/* avoid lossage */
	nb = ureadfour(input);			/* relative offset to character-encoding table */
	reserved = ureadfour(input);	/* reserved  - page 28 LaserWriter */
	if (reserved && showhidden)
		printf("Reserved Word %08X\n", reserved);

/*	stylecount = 0; */
	for (k = 0; k < 48; k++) {
		c = getc(input);
		styles[k] = c; 
/*		if (verboseflag) {
			if (c > 0) putc(c + '0', stdout);
			else putc(' ', stdout);
		} */
		if (k == index) styleindex = c;	/* pick out style index */
/*		if (c != 0) {
			flag = 0;
			for (l = 0; l < k; l++) {
				if (styles[l] == c) {
					flag = 1; break;
				}
			}
			if (flag == 0) stylecount++;
		} */
	}
/*	if (verboseflag) putc('\n', stdout); */

/*  now for style naming table */
	
/*	This has been redone to store information in tables first */

	nstrings = ureadtwo(input);
//	is it OK to just have one string ?	
	if (traceflag && flag) 
		printf("%d style table string%s\n", nstrings, (nstrings==1) ? "" : "s"); 
	if ((nstrings == 1) && flag)
		printf("NOTE: Only one style table string\n");
	fflush(stdout);

	if (nstrings < 256) {
		n = 0;								/* index into BaseName */
		np = getc(input);
		for (l = 0; l < np; l++) {	/* Copy base name to output */
			c = getc(input);
			BaseName[n++] = (char) c;
		} 
		BaseName[n++] = (char) 0;
		nextsuffix = 2;				/* make life simpler ... */
		for (k = 1; k < nstrings; k++) {
			np = getc(input);
			if (np > 127) {
				fprintf(errout, "ERROR: String too long (%d)\n", np);
				break;
			}
			c = getc(input); (void) ungetc(c, input);	/* peek ahead */
			if (c < ' ') {				/* suffix index list */
				n = 0;
				SuffixString[nextsuffix][n++] = (char) np;
				for (l = 0; l < np; l++) {
					c = getc(input);
					if (k+1 == styleindex) 
						indexes[c] = 1; /* assume sequential ! (old) */
					SuffixString[nextsuffix][n++] = (char) c;
				}
				SuffixString[nextsuffix][n++] = (char) 0;
				nextsuffix++;
				if (nextsuffix >= MAXSUFFIX) {
					fprintf(errout, "ERROR: Too many strings\n");
					exit(19);
				}
			}
			else { /* full suffix */
				n = 0;
				SuffixString[nextsuffix][n++] = (char) np;
				for (l = 0; l < np; l++) {
					c = getc(input);
					SuffixString[nextsuffix][n++] = (char) c;
				}
				SuffixString[nextsuffix][n++] = (char) 0;
				nextsuffix++;
				if (nextsuffix >= MAXSUFFIX) {
					fprintf(errout, "ERROR: Too many strings\n");
					exit(19);
				}
			}
		}

/*		have now constructed full string table */
		if (traceflag && flag) {	/* show it for debugging purposes */
			putc('\n', stdout);
			printf("BaseNm: (1) %s (%d)\n", BaseName, strlen(BaseName));
			for (i = 2; i < nextsuffix; i++) {
				printf("Suffix: (%d) ", i);
				if (SuffixString[i][1] < ' ') {
					n = SuffixString[i][0];
					for (k = 0; k < n; k++) {
						printf("%d ", SuffixString[i][k+1]);
					}
					putc('\n', stdout);
				}
				else printf("%s\n", SuffixString[i]+1);
			}
		}

/*		now can output desired combination */
		style = styles[index];
		fprintf(output, "%s", BaseName);
		if (style > 1) {
			n = SuffixString[style][0];		/* how many components */
			for (k = 0; k < n; k++) {
				fprintf(output, "%s", 
					SuffixString[SuffixString[style][k+1]] + 1);
			}
		}
	}
	else fprintf(errout, "ERROR: Too many (%d) strings\n", nstrings);

	(void) fseek(input, oldpos, SEEK_SET);	/* OK */
}

/* page IV-39 Inside MacIntosh */

void decodestyle (int stylecode, FILE *output) {
	if (stylecode == 0) fprintf(output, "REGULAR ");
	if ((stylecode & 1) != 0) fprintf(output, "BOLD ");
	if ((stylecode & 2) != 0) fprintf(output, "ITALIC ");	
	if ((stylecode & 4) != 0) fprintf(output, "UNDERLINE ");	
	if ((stylecode & 8) != 0) fprintf(output, "OUTLINE ");		
	if ((stylecode & 16) != 0) fprintf(output, "SHADOW ");
	if ((stylecode & 32) != 0) fprintf(output, "CONDENSED ");	
	if ((stylecode & 64) != 0) fprintf(output, "EXTENDED ");		
}

int defaultwidth;		/* width of default character (Mac coordinates) */

/* show encoding and widths */

void showwidthssub(FILE *input, FILE *output, unsigned int stylecode) {
	int c, width;
/*	int d; */
	int k;
	unsigned int count=0, badcount=0;
	long current;

/*	stylecode= ureadtwo(input); */
	if (verboseflag) {
		printf("%% style: %d ", stylecode);
		decodestyle(stylecode, stdout); putc('\n', stdout); 
	}
	if (trymultiple == 0)
		fprintf(output, "Comment ResName %s\n", ResName); 

	count = 0; badcount = 0;
/*	for (k = FONDFirst; k <= FONDLast; k++) {
		if (strcmp(encoding[k], "") != 0) {
			if (showzerowidths || charwidths[k] != 0)	count++;
		}
	} */

	WidthStart = ftell(input); 			/* remember for later */
	
	for (k = FONDFirst; k <= FONDLast; k++) {
		c = sreadtwo(input);
/*		c = getc(input); d = getc(input); c = (c << 8) | d; */
		charwidths[k] = c;
		width = mapwidth(c); 
		if (strcmp(encoding[k], "") != 0) {
			if (showzerowidths || width)	count++;
		}
	}

	c = sreadtwo(input);		/* read beyond ? */
	defaultwidth = c;
	if (verboseflag) 
		printf("Default width is %d\n", mapwidth(defaultwidth));
	if (c == 0) fprintf(errout, "WARNING: Default width is zero\n");

	c = sreadtwo(input);
	if (c != 0) fprintf(errout, "WARNING: Reserved `width' is not zero\n");

/*	check whether encoding makes sense - any non-default widths ? */
/*	if (defaultwidth != charwidths[127]) { */	/* 1995/Oct/19 */
	if (defaultwidth != charwidths[127] && charwidths[127] != 0) {
		fprintf(errout,
/* "WARNING: char 127 (delete) is used --- i.e. does not have default width\n"); */
"NOTE: width of char 127 (delete) is %d - not default width (%d)\n",
				mapwidth(charwidths[127]), mapwidth(defaultwidth));
/*		is the following really a good idea ? */
		defaultwidth = charwidths[127];	/* assume delete is not a character */
		fprintf(errout, "NOTE: setting default width to %d\n",
				mapwidth(charwidths[127]));
	}
	for (k = FONDFirst; k <= FONDLast; k++) {
		if (strcmp(encoding[k], "") == 0) {
			if (charwidths[k] != defaultwidth && charwidths[k] != 0) {
				if (verboseflag) printf("%d ", k);
				badcount++;
			}
		}
	}
	if (verboseflag && badcount != 0) putc('\n', stdout);
	
	if (verboseflag && badcount != 0) 
/*		printf("%u chars not in encoding have `non-default' width\n", 
			badcount); */
	printf("Above character(s) not in encoding have `non-default' width\n");

	fprintf(output, "StartCharMetrics %d\n", count);

/*	fseek(input, WidthStart, SEEK_SET);	*/ /* step back over it ? */

/*	WidthStart = ftell(input); */		/* remember for later */

	for (k = FONDFirst; k <= FONDLast; k++) {
/*		c = getc(input); d = getc(input); */ /*		c = (c << 8) | d; */
/*		charwidths[k] = c; */
		c = charwidths[k];
		width = mapwidth(c);
		if (strcmp(encoding[k], "") != 0) {
			if (showzerowidths || width)
				fprintf(output, "C %d ; WX %d ; N %s ;\n", 
					k, width, encoding[k]);
		}
	}
	fprintf(output, "EndCharMetrics\n");
	current = ftell(input);
	if (current > FONDWidthEnd) FONDWidthEnd = current;
	if (traceflag)
		printf("End of width table at %ld\n", FONDWidthEnd - FONDResStart);
	
/*	putc('\n', output); */
}

void showwidths(FILE *input, FILE *output, int stylecount) {
	int k;
/*	int flag; */
	int ntables;
/*	long flag; */
	unsigned int stylecode;

	if (FONDWTabOff == 0) {
		fprintf(errout, "ERROR: apparently no glyph-width table(s)\n");
		return;
	}

	if (fseek(input, FONDResStart + FONDWTabOff, SEEK_SET) != 0) { /* OK */
		fprintf(errout, "Seek Error in %s\n", "showwidths");
	}

/*	if (verboseflag) putc('\n', stdout); */

	ntables = sreadtwo(input) + 1;
	if (verboseflag) putc('\n', stdout);
	if (verboseflag) printf("%d width table%s at %d\n",
							ntables, (ntables == 1) ? "" : "s", FONDWTabOff);
	if (ntables != stylecount) {
/*		printf("Stylecount %d <> ntables %d\n", stylecount, ntables); */
		printf("WARNING: There are %d styles, but %d width tables\n",
			stylecount, ntables);
	}

	if (ntables > 128) {
		fprintf(errout, "ERROR: Absurdly many (%d) Width Tables\n", 
			ntables);
		return;
	}

	if (trymultiple) {
/*		for (k = 0; k < stylecount; k++) { */
		for (k = 0; k < ntables; k++) {
			stylecode= ureadtwo(input);		/* moved here */
			if (verboseflag) {
				printf("FontName ");
/*				showstyle(input, stdout, k); */
				showstyle(input, stdout, stylecode, 0);
				putc(' ', stdout);
			}
			putc('\n', output); 
			fprintf(output, "FontName ");
/*			showstyle(input, output, k);	 */
			showstyle(input, output, stylecode, 0);
			putc('\n', output);	 

			showwidthssub(input, output, stylecode);
/* Why are there two extra words here ??? */
/* Now dealt with in showwidthssub */ /* 1992/July/21 */
/*			flag = sreadtwo(input); 
			if (flag && showhidden) 
				printf("Width Flag 1 %d\n", flag); 
			flag = sreadtwo(input); 
			if (flag && showhidden) 
				printf("Width Flag 2 %d\n", flag); */
		}
	}
	else {
		stylecode= ureadtwo(input);		/* moved here */
		showwidthssub(input, output, stylecode);
	}
}

// unsigned int nkern;

/* show kern pairs */

int showkernssub (FILE *input, FILE *output, unsigned int stylecode) {
	int c, d, e, kern;
	unsigned int k;
	unsigned int nkern;
	long current;

	if (trymultiple == 0) fprintf(output, "Comment ResName %s\n", ResName); 
/*	if (verboseflag) putc('\n', stdout); */
/*	putc('\n', output); */
/*	fprintf(output, "Comment %s\n", ResName); */

	if (verboseflag) {
		printf("%% style: %d ", stylecode);
		decodestyle(stylecode, stdout);
		putc('\n', stdout); 
	}

	KernStart = ftell(input);	/* remember for debug later */

	nkern = ureadtwo(input); 
	if (verboseflag) printf("%d kerning pairs\n", nkern);
/*	nkern = ureadtwo(input) - 1; */				/* 1992/May/2 */
//	if (nkern > 2000) {
	if (nkern > 20000) {
		fprintf(errout, "ERROR: Ridiculous number of kern pairs %d\n", nkern);
		fprintf(errout, "KernStart %ld\n", KernStart);
		return -1;
	}
	fprintf(output, "StartKernPairs %u\n", nkern);
	for (k = 0; k < nkern; k++) {
		c = getc(input);
		d = getc(input);
		e = sreadtwo(input);
		kern = mapwidth(e);
		if (kern > 512 || kern < -512) {
			fprintf(errout, "KPX %s %s %d (%d = %04X)\n", 
			encoding[c], encoding[d], kern, e, e);	
		} 
		fprintf(output, "KPX %s %s %d",
			encoding[c], encoding[d], kern);
		if (strcmp(encoding[c], "") == 0 || strcmp(encoding[d], "") == 0)
			fprintf(output, " %% %d %d %d bad kern pair ?\n", c, d, kern);
		else putc('\n', output);
	}
	fprintf(output, "EndKernPairs\n");
	putc('\n', output);
	current = ftell(input);
	if (current > FONDKernEnd) FONDKernEnd = current;
	if (traceflag)
		printf("End of kerning table at %ld\n", FONDKernEnd - FONDResStart);
	return 0;
}

void showkerns(FILE *input, FILE *output, int stylecount) {
	int k, ntables;
/*	int flag;  */
	int stylecode;
	
	if (FONDKernOff == 0) {
		FONDKernEnd = 0;
		if (verboseflag) printf("NOTE: No pair kerning tables\n");
		return;			/* no kerning pairs */
	}

	if (fseek(input, FONDResStart + FONDKernOff, SEEK_SET) != 0) {	/* OK */
		fprintf(errout, "Seek Error in %s\n", "showkerns");
	}

	ntables = sreadtwo(input) + 1;
	if (verboseflag) putc('\n', stdout);
	if (verboseflag) printf("%d kerning table%s at %d\n",
							ntables, (ntables == 1) ? "" : "s", FONDKernOff);
	if (ntables != stylecount) { 
/*		printf("Stylecount %d <> ntables %d\n", stylecount, ntables); */
		printf("WARNING: There are %d styles, but %d kern tables\n",
			stylecount, ntables);
	}
	if (ntables > 128) {
		fprintf(errout, "ERROR: Absurdly many (%d) Kern Tables\n", 
			ntables);
		return;
	}
	if (trymultiple) { 
/*		for (k = 0; k < stylecount; k++) { */
		for (k = 0; k < ntables; k++) {
			stylecode = ureadtwo(input);			/* ??? */
			if (stylecode && showhidden) 
				printf("Kern Style %02X\n", stylecode); /* Normally zero ? */
			if (verboseflag) {
				printf("FontName ");
				showstyle(input, stdout, stylecode, 0);
/*				putc('\n', stdout); */
				putc(' ', stdout);
			}
			putc('\n', output); 
			fprintf(output, "FontName ");
			showstyle(input, output, stylecode, 0);
			putc('\n', output);

			if (showkernssub(input, output, stylecode) < 0) break;
/*			flag = sreadtwo(input); */
/*			if (showhidden) printf("Kern Flag %d\n", flag); */
/*			getc(input); getc(input); */	/* ??? */
		}
	}
	else {
		stylecode =  ureadtwo(input);			/* ??? */
		if (stylecode && showhidden) 
			printf("Kern Style %02X\n", stylecode); /* Normally zero ... */
		(void) showkernssub(input, output, stylecode);
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

// bit 0 (1) Font name needs "coordinating"

// bit 1 (2) Macintosh vector reencoding scheme is required.
//			 Also, borrow glyphs frmo Symbol font for math glyphs

// bit 9 (512) Font requires reencoding other than Macintosh vector reencoding
//			 Implies the glyph-encoding table is present

void analyseclass(int class) {
//	if ((class & COORDINATE) != 0)			/*	usemacencoding = 1; */	/* NO ??? */

	if ((class & MACENCODING)) usemacencoding = 1;
	else usemacencoding = 0;			/* ??? */

	if ((class & NONMACENCODE)) nonmacencode = 1;
	else nonmacencode = 0;

	if (verboseflag == 0) return;

	printf("Font Class %04X ", class);
	if ((class & COORDINATE) != 0) printf("coordinate ");
	if ((class & MACENCODING) != 0) printf("mac-encode ");
	if ((class & PAINTTYPE) != 0) printf("paint-2 ");
	if ((class & NOOUTLINE) != 0) printf("no-outline ");
	if ((class & NOEMBOLDEN) != 0) printf("no-embold ");
	if ((class & BOLDENLARGE) != 0) printf("bold-enlarge ");
	if ((class & NOOBLIQUE) != 0) printf("no-oblique ");
	if ((class & NOCONDENSE) != 0) printf("no-condense ");
	if ((class & NOEXPANDE) != 0) printf("no-expend ");
	if ((class & NONMACENCODE) != 0) printf("non-mac-encode ");
	if ((class & NOINTERCHAR) != 0) printf("no-interchar ");	
	putc('\n', stdout);
//	following is some weird special case where glyphs are borrowed from Symbol
	if ((class & MACENCODING) && (class & NONMACENCODE))
		fprintf(errout, "WARNING: specified both Mac and non-Mac encoding\n");
	if (((class & MACENCODING) || (class & NONMACENCODE)) &&
		(class & COORDINATE) == 0)
   fprintf(errout, "WARNING: Mac or non-Mac encoding, but not coordinated\n");
}

/*	nine property words are (IV-38 Inside MacIntosh): */
/*	extra width for plain text == 0 */
/*	extra width for bold text */
/*	extra width for italic text */
/*	extra width for underline text */
/*	extra width for outline text */
/*	extra width for shadow text */
/*	extra width for condensed text */
/*	extra width for extended text */
/*	not used == 0 */

char *ffproperty[9] = {
	"plain", "bold", "italic", "underline", "outline", 
	"shadow", "condensed", "extended", ""
};

/* Following is from `Inside MacIntosh IV' page 37 4-92*/

/* Fond Flags: 0 (1) reserved - should be zero */
/* Fond Flags: 1 (2) set if there is an glyph-width table */
/* Fond Flags: 2-11 (4 ... 0X0800) reserved - should be zero */
/* Fond Flags: 12 (0X1000) ignore FractEnable for stylistic variations */
/* Fond Flags: 13 (0X2000) use integer extra width for stylistic variations */
/* Fond Flags: 14 (0X4000) set if family fractional-width table is NOT used */
/* Fond Flags: 15 (0X8000) set for fixed-width font */

long ResDataPos;

long wordadjust(FILE *input) {
	long current = ftell(input);
	if (current & 1) {
		fseek(input, current+1, SEEK_SET);
		current = ftell(input);
	}
	return current;
}

/* analyze FOND resource */ /* see page 27 in Apple LaserWriter Reference */

void readfonddata(FILE *input, FILE *output) {
	int c;
	unsigned int k, l;
	unsigned long nb;
	unsigned int na, nc, nd, chr, flag;
	unsigned int nx, ny, nz, np, nstrings;
/*	unsigned int ns; */
	char *s;
	int stylecount;
/*	long resdatapos; */
	unsigned int international;
	unsigned long reserved;
	unsigned int stylecode;
	long current;

	FONDVersion = 2;			/* for now, so mapwidth works correctly ! */ 
	FONDWidthEnd = 0;
	FONDKernEnd = 0;
	FONDStyleEnd = 0;

	if (nomacbinary) ResDataPos =  offset + ResDataOffset;
	else ResDataPos =  offset + ResDataOffset + MACBINARYHEAD;

	if (fseek(input, ResDataPos + FONDDataOffset, SEEK_SET) != 0) { /* OK */
		fprintf(errout, "Seek Error in %s\n", "readfonddata");
		exit(7);
	}			

	FONDDataLen = ureadfour(input);
	if (verboseflag) putc('\n', stdout);

	FONDResStart = ftell(input);

	if (verboseflag) 
		printf("FONDResStart %ld FONDDataLen %ld\n", FONDResStart, FONDDataLen);
	FONDFlags = ureadtwo(input);
	if (verboseflag) printf("FONDFlags %02X ", FONDFlags);
	if ((FONDFlags & 0X8000)) isfixedpitch = 1;
	else isfixedpitch = 0;

	if (verboseflag) {
		if ((FONDFlags & 0X0001) != 0) printf("Reserved ");			// Image-Heights
		if ((FONDFlags & 0X0002) != 0) printf("Glyph-Widths ");		// Image-Widths
		if ((FONDFlags & 0X1000) != 0) printf("Ignore-FractEnable ");
		if ((FONDFlags & 0X2000) != 0) printf("Integer-Extra ");
		if ((FONDFlags & 0X4000) != 0) printf("Fractional-Unused ");
		if ((FONDFlags & 0X8000) != 0) printf("Fixed-Width ");
		putc('\n', stdout);
	}

	FONDFamIDptr = ftell(input);	/* remember Font Family ID pointer */

	FONDFamID = ureadtwo(input);	/* Font Family ID */
	if (verboseflag) printf("FONDFamID %d\n", FONDFamID);
	FONDFirst = sreadtwo(input);
	FONDLast = sreadtwo(input);
	if (verboseflag) printf("FONDFirst %d FONDLast %d\n", FONDFirst, FONDLast);

	FONDAscentptr = ftell(input);

	FONDAscent = sreadtwo(input);
	FONDDescent = sreadtwo(input);
	if (verboseflag) printf("Ascent %d Descent %d ", 
		mapwidth(FONDAscent), mapwidth(FONDDescent)); 
	FONDLeading = sreadtwo(input);
	FONDWidMax = sreadtwo(input);
	if (verboseflag) printf("Leading %d --- MaxGlyphWidth %d\n",
		mapwidth(FONDLeading), mapwidth(FONDWidMax));

	FONDWTabOff = ureadfour(input);		// Offset Family Glyph Width Table
	FONDKernOff = ureadfour(input);		// Offset Family Kerning Table
	FONDStylOff = ureadfour(input);		// Offset Family Style Mapping Table
	if (verboseflag) {
//		printf("Width Table %ld Kerning Table %ld Style Mapping Table %ld\n",
//			FONDWTabOff, FONDKernOff, FONDStylOff);
		if (FONDWTabOff) printf("Width Table at %d, ", FONDWTabOff);
		if (FONDKernOff) printf("Kerning Table at %d, ", FONDKernOff);
		if (FONDStylOff) printf("Style Offset Table at %d.", FONDStylOff);
		putc('\n', stdout);
	}
	if ((FONDWTabOff & 1))
		fprintf(errout, "ERROR: Odd FONDWTabOff %ld\n", FONDWTabOff);
	if ((FONDKernOff & 1))
		fprintf(errout, "ERROR: Odd FONDKernOff %ld\n", FONDKernOff);
	if ((FONDStylOff & 1))
		fprintf(errout, "ERROR: Odd FONDStylOff %ld\n", FONDStylOff);

/*  Maybe deal with cyclical repetition also ? */

/*	if (verboseflag) printf("Style property info: "); */
	flag = 0;
	for (k = 0; k < 9; k++) {			/* Style property information */
		c = sreadtwo(input);
		if (c != 0) {
			if (verboseflag) 
printf("Style Property field word %d equals %d (extra width for %s text)\n", 
					k, mapwidth(c), ffproperty[k]); 
			flag = 1;
		}
	}
/*	if (verboseflag) putc('\n', stdout); */

	printf("FONDFirst %d FONDLast %d\n", FONDFirst, FONDLast);
	if (FONDFirst == 0 && FONDLast == 0) {
		printf("WARNING: Not a Scalable Font\n");
	}
	else if (FONDFirst > FONDLast) {
/*		fprintf(errout, "FONDFirst %d FONDLast %d\n", FONDFirst, FONDLast); */
		fprintf(errout, "ERROR: FONDFirst %d > FONDLAST %d\n", FONDFirst, FONDLast);
		exit(9); 
	}
	else if (FONDFirst > 32) {
		fprintf(errout, "NOTE: FONDFirst %d > 32\n", FONDFirst);
/*		exit(9); */
	}
	else if (FONDLast < 255) {
/*		fprintf(errout, "FONDLast > 255\n"); */
	}
	if (FONDFirst < 0) {
/*		fprintf(errout, "FONDFirst %d FONDLast %d\n", FONDFirst, FONDLast); */
		fprintf(errout, "ERROR: FONDFirst %d < 0\n", FONDFirst);
		exit(9);
	}
	else if (FONDLast > 255) {
		fprintf(errout, "ERROR: FONDLast %d > 255\n", FONDLast);
		exit(9);
	}

	international = ureadtwo(input); 		/* international field */
	if (international != 0) 
		printf("International word 1 %u\n", international);
	international = ureadtwo(input); 		/* international field */
	if (international != 0) 
		printf("International word 2 %u\n", international);

	FONDVersion = ureadtwo(input);			// an inconsistent field
//	0 created by Mac system software
//	1 original format as designed by font developer
//	2 may contain offset and bounding-box tables
//	3 does contain offset and bounding-box tables
	if (FONDVersion != 2) {
		if (FONDVersion < 2) 
			printf("WARNING: FONDVersion %d is not 2 (must be very old)\n", 
			FONDVersion);
		else printf("WARNING: FONDVersion %d is not 2 (must be very new)\n", 
			FONDVersion);
	}

	if (verboseflag) putc('\n',stdout);

	current = ftell(input);
	if (traceflag) printf("Font Associations table at %ld\n",
						  current - FONDResStart);		// should be 52

	NumAssociations = ureadtwo(input) + 1;		/*  Font Association Table */
	if (verboseflag) 
		printf("Number of FONT/NFNT Associations %d\n", NumAssociations);

	if (NumAssociations > 128) {
		fprintf(errout, "ERROR:  Absurdly large (%d) NumAssociations\n", 
			NumAssociations);
		return;
	}

	for (k = 0; k < NumAssociations; k++) {
		nx = ureadtwo(input);		// point
		ny = ureadtwo(input);		// style code
		nz = ureadtwo(input);		// FONT/NFNT res ID
		if (verboseflag) {
			printf("Point Size: %d  Style: %d ", nx, ny);
			decodestyle(ny, stdout);
			printf(" ResID of FONT/NFNT: %d\n", nz);
		}
	}

	if (verboseflag) putc('\n',stdout);
	fflush(stdout);

	NumOffsets = 1;

	if (FONDFirst == 0 && FONDLast == 0) {
		printf("No BBox data, since not a scalable font\n");
		goto avoidbbox;	/* not scalable */
	}

	if (FONDVersion < 2) {
		printf("No BBox data, since an ancient font\n");
		goto avoidbbox;	/* no BBox info - old version */
	}

	if (FONDWTabOff == 60) {
		printf("No BBox data, since no space for such data\n");
		goto avoidbbox;	/* kludge  - no BBox info */
	}

//	offsets to various optional tables --- actually BBox table only one so far

	OffsetsStart = ftell(input);
	if (traceflag) printf("Offsets Table at %ld\n", OffsetsStart - FONDResStart);
	NumOffsets = ureadtwo(input) + 1;
	if (NumOffsets != 1) printf("NumOffsets %d\n", NumOffsets);
/*  NumOffsets != 1 indicates not an outline font - old font form ? */

	FONDBBoxOffset = ureadfour(input);
	if (FONDBBoxOffset != 6) 
		printf("FONDBBoxOffset %ld\n", FONDBBoxOffset);
/*  this offset is from start of Offset table --- i.e. the NumOffsets word ... */
	if ((FONDBBoxOffset & 1)) 
		fprintf(errout, "ERROR: Odd FONDBBoxOffset %ld\n", FONDBBoxOffset);
//	if (fseek (input, FONDBBoxOffset - 6, SEEK_CUR) != 0) goto avoidbbox;
	if (fseek (input, FONDBBoxOffset + OffsetsStart, SEEK_SET) != 0) {
		fprintf(errout, "ERROR: Seek to BBox table at %d failed\n",
				FONDBBoxOffset + OffsetsStart);
		goto avoidbbox;
	}

	NumFontBBox = ureadtwo(input) + 1;
	if (verboseflag) printf("NumFontBBox %d at %ld\n",
							NumFontBBox, FONDBBoxOffset + OffsetsStart - FONDResStart);

	if (NumFontBBox > 16 && flag == 0) {
		fprintf(errout, "WARNING: NumFontBBox %d\n", NumFontBBox);
	}
	if (NumFontBBox < 16 && NumOffsets == 1) {	/* prevent lossage */
		FONDBBoxptr = ftell(input);
		if (verboseflag) putc('\n', output);	/* ??? */
		for (k = 0; k < NumFontBBox; k++) {
			stylecode = ureadtwo(input);				/* style word */
			xll = sreadtwo(input); yll = sreadtwo(input);
			xur = sreadtwo(input); yur = sreadtwo(input);
			if (verboseflag) {
				printf("FontBBox %d %d %d %d %% style: %d ", mapwidth(xll), 
					mapwidth(yll), mapwidth(xur), mapwidth(yur), stylecode); 
				decodestyle(stylecode, stdout); 
				putc('\n', stdout);
/*				printf("FontBBox %d %d %d %d\n", mapwidth(xll), 
					mapwidth(yll), mapwidth(xur), mapwidth(yur)); */
			}
			putc('\n', output);
			fprintf(output, "FontName ");		/* experiment ? */
			showstyle(input, output, stylecode, 1); 
			putc('\n', output);
/*			fprintf(output, "FontBBox %d %d %d %d %% style: %d ",  */
			fprintf(output, "FontBBox %d %d %d %d\n", mapwidth(xll), 
				mapwidth(yll), mapwidth(xur), mapwidth(yur));
/*					stylecode); */
/*			decodestyle(stylecode, output); */
/*			putc('\n', output); */
		}
	} 

//	now we are the end of the BBox Table, but optional tables may follow or not...

avoidbbox:		// get here if we skip Font BBox table reading

//	style mapping table 

	if (FONDStylOff == 0) return;	/* probably not scalable font */

	if (fseek(input, FONDResStart + FONDStylOff, SEEK_SET) != 0) { 
		fprintf(errout, "Seek Error in %s\n", "readfonddata");
	}

	current = ftell(input);
	if (verboseflag) putc('\n', stdout);
	
	if (traceflag) printf("Style Mapping Table at %ld\n", current - FONDResStart);

	na = ureadtwo(input);	/* Font Class - page 29 of LaserWriter */

	if (traceflag) 
		printf("FOND Class %08X at %ld in file\n", na, current - FONDResStart);

//	following also sets macencode and nonmacencode flags
	analyseclass (na);
	if (usemacencoding || basemacencoding) 
		for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], macencoding[k]);
//	else if (strcmp(vector, "") != 0) readencoding(vector);
	else if (vector != NULL) readencoding(vector);
	else for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], "");	

	if (NumOffsets != 1) return;	/* avoid lossage with non-standard fonts */

	nb = ureadfour(input);	/* relative offset from start of table to character-encoding table */

/*	In desparation we use numeric encoding */
	if (usemacencoding == 0 && basemacencoding == 0 && 
//		strcmp(vector, "") == 0)
		vector == NULL) {	
		for (k = 0; k < MAXCHRS; k++) sprintf(encoding[k], "n%d", k);
		if (nb == 0) printf(
"WARNING: Definitely need to specify actual outline font encoding vector\n");
/*		else if (nc < FONDLast - FONDFirst + 1)
			printf("WARNING: May need to specify encoding vector as	basis\n"); */
	}
/*  In this case, should really get encoding vector from outline font file. */

/*	style mapping table */
	reserved = ureadfour(input);		/* reserved  - page 28 LaserWriter */
	if (reserved && showhidden) 
		printf("Style Mapping Reserved Word %08X\n", reserved);

	if (verboseflag) printf("Style Mapping Table: ");

//	index to suffix string list for style k for k = 0, 1, ... 47
	stylecount = 0;
	for (k = 0; k < 48; k++) {
		c = getc(input);
		styles[k] = c;
		if (verboseflag) {
			if (traceflag || c > 0 || k < 4) putc(c + '0', stdout);
			else putc(' ', stdout);
		}
		newstyle[k] = 0;
		if (c != 0) {
			flag = 0;
			for (l = 0; l < k; l++) {
				if (styles[l] == c) {
					flag = 1; break;
				}
			}
			if (flag == 0) {
				newstyle[k]++;		/* style code not seen before */
				stylecount++;		/* count unique style codes */
			}
		}
	}
	if (verboseflag) putc('\n', stdout);

/*  now for style naming table */
	
	nstrings = ureadtwo(input);

	if (traceflag)
		printf("%d string%s in style table\n", nstrings, (nstrings==1) ? "" : "s");

//	now positioned at start of Base Font Name (Pascal String)

//	read over suffix string table next (not stored)
	if (nstrings < 256) {
		np = getc(input);			// Pascal string char count for Base Font Name
//		if (traceflag) printf("Base Font Name char count %d\n", np);
		for (l = 0; l < np; l++) c = getc(input);
		for (k = 1; k < nstrings; k++) {
			np = getc(input);		// Pascal string char count for suffix or suffix list
			if (np > 127) {
				fprintf(errout, "ERROR: String too long (%d)\n", np);
				break;
			}
			c = getc(input);
			(void) ungetc(c, input);	/* peek ahead */
			if (c < ' ') {				/* suffix index list */
				for (l = 0; l < np; l++) c = getc(input);
			}
			else {						/* full suffix */
				for (l = 0; l < np; l++) c = getc(input);
			}
		}
	}
	else fprintf(errout, "ERROR: Too many (%d) strings\n", nstrings);

	current = wordadjust(input);	// now after string table

/*  now go to reencoding table - if one exists */

	if (nb != 0) {
		if (current != (long) (FONDResStart + FONDStylOff + nb)) {
			printf("NOTE: encoding table at %ld not current position %ld\n",
				   FONDResStart + FONDStylOff + nb, current);
		}

		if (verboseflag) 
			printf("NB (offset to char encoding) = %lu, ", nb); 
		if ((nb & 1)) fprintf(errout, "ERROR: Odd NB offset %lu\n", nb);

		if (fseek(input, FONDResStart + FONDStylOff + nb, SEEK_SET) != 0) { 
			fprintf(errout, "Seek Error in %s\n", "readfonddata");
			exit(7);
		}

		nc = ureadtwo(input);	/* how many characters need to be reencoded */
		if (verboseflag) 
			printf("NC (chars need to reencode) = %d\n", nc); 
		if (usemacencoding == 0 && basemacencoding == 0 && 
//			strcmp(vector, "") == 0) 
			vector == NULL) {
			if (nc < (unsigned int) (FONDLast - FONDFirst + 1)) {
				printf(
	"WARNING: May need to specify actual outline font encoding vector as basis\n");
				printf(
    "         %d glyph name%s in encoding table, but may need up to %d names for AFM\n",
					   nc, (nc == 1) ? "" : "s", (FONDLast - FONDFirst + 1));
			}
		}

		if (nc <= MAXCHRS) {
/*			if (verboseflag) putc('\n', stdout); */
			for (k = 0; k < nc; k++) {
				chr = getc(input);
				nd = getc(input);
				s = encoding[chr];
				if (nd > 32) {
					fprintf(errout, "ERROR: Character Name too long (%d)\n", nd);
/*					exit(15); */
					break;
				}
				for (l = 0; l < nd; l++) *s++ = (char) getc(input);
				*s = '\0';
				if (traceflag)
					printf("C %d ; N %s ;\n", chr, encoding[chr]);
			}
		}
		else fprintf(errout, "ERROR: Too many (%d) characters to reencode\n", nc);

	}				// end of if nb != 0
	else if (usemacencoding == 0) {
		if (verboseflag) printf("WARNING: No detailed encoding given\n");
	}

	current = wordadjust(input);	// now at end of encoding table
	
	current = ftell(input);
	if (current > FONDStyleEnd) FONDStyleEnd = current;
	if (traceflag)
		printf("End of style mapping table %ld\n", FONDStyleEnd - FONDResStart);

/*	show style name table ? */

	putc('\n', output);
	fprintf(output, 
		"Comment common information for all styles of following font\n");
	if (isfixedpitch != 0) fprintf(output, "IsFixedPitch true\n");
	else fprintf(output, "IsFixedPitch false\n");
	if (FONDAscent != 0) 
		fprintf(output, "Ascender %d\n", mapwidth(FONDAscent));
	if (FONDDescent != 0) 
		fprintf(output, "Descender %d\n", mapwidth(FONDDescent));

/*	now deal with width tables */
	if (showwidthsflag) showwidths(input, output, stylecount);

/*	now deal with kern tables */
	if (showkernsflag) {
		showkerns(input, output, stylecount);
	}
	if (showwidthsflag && showkernsflag) {
//		check that the end of the last table is at end of FOND resource
		current = 0;
		if (FONDWidthEnd > current)  current = FONDWidthEnd;
		if (FONDStyleEnd > current)  current = FONDStyleEnd;
		if (FONDKernEnd > current)  current = FONDKernEnd;
		printf("End of last table in FOND resource at %ld\n", current- FONDResStart);
		if (current - FONDResStart != (long) FONDDataLen) {
			printf("WARNING: current position - FONDResStart %ld <> FONDDataLen %lu\n",
				   current - FONDResStart, FONDDataLen);
		}
	}
}

// nfntflag == 1 if NFNT resource --- nfntflag == 0 if FONT resource

void readnfntdata (FILE *input, long DataOffset, int nfntflag) {
	unsigned int NFNTtype;
	int NFNTfirst, NFNTlast;
	int NFNTwidthmax;							/* max bitmap width */
/*	(nkernmax, ndescent) defines lower left corner of character rectangle */
	int NFNTnkernmax, NFNTndescent;
/*	(rectwidth, rectheight) defines character rectangle size */
	int NFNTrectwidth, NFNTrectheight;
	int NFNTascent, NFNTdescent, NFNTleading;
	int NFNTwidthoffset, NFNTrowwords;
	int bitmapsize, widthoffset;
	int widthflag=0, heightflag=0;
	int location, width, height;
	int newoffset; 
/*	int oldlocation; */
	int k, m, l, c;
	long NFNTBitMapStart;
/*	long NFNTLocationStart; */
	int locationstart, locationend;
	int byteoffset, bitindex;
	int sidebearing, escapement;
/*	long resdatapos;*/
	long current;
	int tablesize;

	if (nomacbinary) ResDataPos =  offset + ResDataOffset;
	else ResDataPos =  offset + ResDataOffset + MACBINARYHEAD;

	if (fseek(input, ResDataPos + DataOffset, SEEK_SET) != 0) { /* OK */
		fprintf(errout, "Seek Error in %s\n", "readnfntdata");
		exit(7);
	}			

	NFNTDataLen = ureadfour(input);
	if (NFNTDataLen == 0) {
		fprintf(errout, "ERROR: NFNTDataLen == 0\n");
		return;
	}
	if (verboseflag) putc('\n', stdout);

	NFNTResStart = ftell(input);

	if (verboseflag) 
		printf("%sResStart %ld %sDataLen %ld\n", 
			   nfntflag ? "NFNT" : "FONT", NFNTResStart,
			   nfntflag ? "NFNT" : "FONT", NFNTDataLen);

	NFNTtype = ureadtwo(input);
	NFNTfirst = sreadtwo(input);
	NFNTlast = sreadtwo(input);
	NFNTwidthmax = sreadtwo(input);
/*	nkernmax, ndescent define lower left corner of character rectangle */
	NFNTnkernmax = ureadtwo(input);
	NFNTndescent = sreadtwo(input);
	if (NFNTndescent > 0) printf("WARNING: NFNTndescent %d > 0 !\n", NFNTndescent);
/*	rectwidth, rectheight define character rectangle size */
	NFNTrectwidth = sreadtwo(input);
	NFNTrectheight = sreadtwo(input);
	NFNTwidthoffset = sreadtwo(input);	/* offset to width table (from here!) */
	NFNTascent = sreadtwo(input);
	NFNTdescent = sreadtwo(input);
	NFNTleading = sreadtwo(input);
	NFNTrowwords = sreadtwo(input);		/* words per row of bitmap image */

	if (NFNTndescent + NFNTdescent != 0 && NFNTndescent + NFNTascent != 0) {
		printf("NOTE: Neg Descent %d <> -Descent %d  AND  Neg Descent %d <> - Ascent %d\n",
			  NFNTndescent, -NFNTdescent, NFNTndescent, NFNTascent);
	}
	if (NFNTrectheight != NFNTascent + NFNTdescent) {
		printf("NOTE: RectHeight %d <> to Ascent + Descent (%d + %d)\n",
			  NFNTrectheight, NFNTascent, NFNTdescent);
	}

	if (verboseflag == 0 && traceflag == 0) return;

/*	printf("NFNT Type %04X ", NFNTtype); */
	printf("%s Type %04X ", nfntflag ? "NFNT" : "FONT", NFNTtype);
//	if ((NFNTtype & 0XF000) == 0X9000) printf("Proportional ");
//	else if ((NFNTtype & 0XF000) == 0XB000) printf("Fixed-Width ");
//	else printf("Unknown ");
	if ((NFNTtype & 0X2000)) printf("Fixed-Width ");
	else printf("Proportional ");

	if ((NFNTtype & 1)) {			// won't normally happen
		heightflag = 1;
		printf("Height-table ");
	}
	if ((NFNTtype & 2)) {			// optional table - we don't make
		widthflag = 1;
		printf("Width-table ");
	}
	putc('\n', stdout);

	printf("NFNTFirst %d NFNTLast %d \n",  NFNTfirst, NFNTlast);
	printf("MaxWidth %d Negative MaxKern %d Negative Descent %d\n",
		NFNTwidthmax, NFNTnkernmax, NFNTndescent);
	printf("FRectWidth %d FRectHeight %d\n", 
		NFNTrectwidth, NFNTrectheight);
	printf("Ascent %d Descent %d Leading %d\n", 
		NFNTascent, NFNTdescent, NFNTleading);
	printf("Offset/Width Table %d (from start of resource) RowWords %d\n",
		NFNTwidthoffset * 2 + 8 * 2, NFNTrowwords);

/*  show bitmaps ? */
	bitmapsize = 2 * NFNTrowwords * NFNTrectheight;			// in bytes
	printf("Bitmap Image takes %d bytes\n", bitmapsize);
//	width/offset table after initial stuff, bitmap, and bitmap locator table
	widthoffset = 13 * 2 + bitmapsize + 2 * (NFNTlast - NFNTfirst + 3);
/*	Is NFNTWidthoffset in words from present position ? */
	if (NFNTwidthoffset * 2 + 8 * 2 != widthoffset)
		printf("NFNTwidthoffset * 2 + 8 * 2 %d <> widthoffset %d\n",
			NFNTwidthoffset * 2 + 8 * 2, widthoffset);
	NFNTBitMapStart = ftell(input);

/*	skip over character bitmaps for now */
	for (k = 0; k < bitmapsize; k++) (void) getc(input);

/*	NFNTLocationStart = ftell(input); */

	if (shownfnttables == 0 && showbitmaps == 0 && traceflag == 0) return; 

//	several tables of size (NFNTlast - NFNTfirst + 3) words
	tablesize = (NFNTlast - NFNTfirst + 3);

/*	read bitmap location table */
	for (k = NFNTfirst; k <= NFNTlast+2; k++) {
		location = ureadtwo(input);
		if (k >= 0) locations[k] = location;
	}

//	assuming the offset to width/offset table points here ?
	current = ftell(input);
//	normally expect to be at start of width/offset table at this point ...
	if (current != widthoffset +  NFNTResStart) {
		printf("current-NFNTResStart %ld, NFNTwidthoffset * 2 + 8 * 2 %ld, widthoffset %ld\n",
			   current-NFNTResStart, NFNTwidthoffset * 2 + 8 * 2, widthoffset);
		printf("Seek from %ld to %ld to width/offset table\n", current, NFNTResStart +  widthoffset);
		fseek(input, NFNTResStart +  widthoffset, SEEK_SET);
	}

	printf("\nOffset/Width table follows:\n");
/*	fseek(input, NFNTResStart + NFNTwidthoffset, SEEK_SET); */
	for (k = NFNTfirst; k <= NFNTlast + 2; k++) {
		newoffset = getc(input);
		width = getc(input);
		if (k >= 0) {
			offsets[k] = newoffset;
			widths[k] = width;
		}
	}

	if (shownfnttables) {
		int locationsk, locationskp1, missingflag;
		locationsk = locations[NFNTfirst];
		locationskp1 = locations[NFNTfirst];
		for (k = NFNTfirst; k <= NFNTlast + 2; k++) {
//			careful not to go off end of table
			if (k < NFNTlast+2) locationskp1 = locations[k+1];
			missingflag = (offsets[k] == 255 && widths[k] == 255);
			printf("k%4d bit-loc:%4d bit-width:%4d (%3d - %3d) offset:%4d width:%4d %s\n",
				k, locationsk, locationskp1 - locationsk,
				   locationskp1, locationsk, offsets[k], widths[k],
				   missingflag ? "MISSING GLYPH" : "");
			locationsk = locationskp1;
		}
	}
	fflush(stdout);
	k = NFNTlast+2;
	if (! (offsets[k] == 255 && widths[k] == 255)) {
		printf("WARNING: last word of offset/width table not 255 255 (%d %d)\n",
			   offsets[k], widths[k]);
	}

/*	show optional glyph-width table - not normally produced by AFMtoSCR */
	if (widthflag) {
		if (shownfnttables) printf("\nWidth Table:\n");
		for (k = NFNTfirst; k <= NFNTlast + 2; k++) {
			width = ureadtwo(input);
			if (shownfnttables)
				printf("k: %3d width: %5d (%d)\n", k, width, 
					(int) (((long) width * 100 + 127) / 256));
/* Above assumes 10pt size screen font */ /* need to know actual pt size */
/* Scale factor depend on the point size of the font:  1000 / ptsize */
		}
	}
	else if (verboseflag) printf("No optional glyph-width  table\n");

/*	show optional image-height table */
	if (heightflag) {
		if (shownfnttables) printf("\nHeight Table:\n");
		for (k = NFNTfirst; k <= NFNTlast + 2; k++) {
			height = ureadtwo(input);
			if (shownfnttables)
/* offset is from top of character rectangle downward */
				printf("k: %3d down: %2d height: %2d\n", k, 
					(height >> 8) & 255, height & 255);
		}
	}
	else if (verboseflag) printf("No optional image-height table\n");

	current = ftell(input);
	if (current - NFNTResStart != (long) NFNTDataLen) {
		printf("WARNING: current position - NFNTResStart %ld <> NFNTDataLen %lu\n",
			   current - NFNTResStart, NFNTDataLen);
	}

	if (showbitmaps == 0) return;

/*	show bit-mapped images in NFNT resource */
	for (k = NFNTfirst; k <= NFNTlast; k++) {
		locationstart = locations[k];	/* bit position of start */
		locationend = locations[k+1];	/* bit position of end + 1 */
		if (locationstart < locationend) {	/* only if character there */
/*			escapement = offsetwidths[k] & 255; */				/* ??? */
			escapement = widths[k];
/*			sidebearing = (offsetwidths[k] >> 8) & 255; */		/* ??? */
			sidebearing = offsets[k]; 
/*		printf("CHARACTER: %d - bitmap width: %d - side %d - escape %d\n",  */
		putc('\n', stdout);
		printf("CHARACTER: %d - bitmap width: %d - offset %d - advance width %d\n",
				k, locationend-locationstart, sidebearing, escapement);
/*		if (locationend - locationstart + NFNTnkernmax + sidebearing > escapement) */
/*	printf("WARNING: bitmapwidth + nkernmax + offset > advancewidth\n"); */
		
		for (m = 0; m < NFNTrectheight; m++) {
			byteoffset = locationstart / 8;
			fseek (input, NFNTBitMapStart + 2 * m * NFNTrowwords 
				+ byteoffset, SEEK_SET);				/* OK */
			bitindex = locationstart - byteoffset * 8;
			c = getc(input);
			for (l = 0; l < bitindex; l++) c = c << 1;
/*			for (l = 0; l < sidebearing; l++) {
				putc('.', stdout); 	putc(' ', stdout);
			} */
			for (l = locationstart; l < locationend; l++) {
				if ((c & 128) != 0) putc('@', stdout);  
/*				if ((c & 128) != 0) putc('*', stdout);  */
/*				if ((c & 128) != 0) putc(219, stdout); */
				else putc('.', stdout);
				putc(' ', stdout);
				c = c << 1;
				bitindex++;
				if (bitindex >= 8) {
					c = getc(input);
					bitindex = 0;
				}
			}
/*			for (l = locationend - locationstart; l < escapement; l++) {
				putc('.', stdout); 	putc(' ', stdout);
			} */
			putc('\n', stdout);
		}
		}
	}
/*	showbitmaps = 0; */	/* give only one while debugging */
}

/* copy the file, replacing Ascent, Descent, FontBBox & character widths */

void insertnew(FILE *output, FILE *input) {
	int c;
/*	unsigned long k, ks=0; */
	long k, ks=0;

	if (FONDFamIDptr > FONDResIDptr) {
		fprintf(errout, "FONDFamIDptr > FONDResIDptr\n");
		return;
	}
	else {
		for (k = ks; k < FONDFamIDptr; k++) putc(getc(input), output);
		ks = FONDFamIDptr;
		if (changeResID == 0) newResID = ureadtwo(input);
		else (void) ureadtwo(input);
		uwritetwo(newResID, output); ks += 2; 
	}

	if (FONDResIDptr > FONDAscentptr) {
		fprintf(errout, "FONDResIDptr > FONDAscentptr\n");
	}
	else {
		for (k = ks; k < FONDResIDptr; k++) putc(getc(input), output);
		ks = FONDResIDptr;
		if (changeResID == 0) newResID = ureadtwo(input);
		else (void) ureadtwo(input);
		uwritetwo(newResID, output); ks += 2; 
	}

	if (FONDAscentptr > FONDBBoxptr) {
		fprintf(errout, "FONDAscentptr  > FONDBBoxptr\n");
	}
	else {
		for (k = ks; k < FONDAscentptr; k++) putc(getc(input), output);
		ks = FONDAscentptr;
/*		uwritetwo(FONDAscent, output);	 */
		swritetwo(FONDAscent, output);	
		ks += 2; 
		(void) ureadtwo(input);
/*		uwritetwo(FONDDescent, output);   */
		swritetwo(FONDDescent, output);  
		ks += 2; 
		(void) ureadtwo(input);
	}

	if (FONDBBoxptr > WidthStart) {
		fprintf(errout, "FONDBBoxptr > WidthStart\n");
	}
	else {
		for (k = ks; k < FONDBBoxptr; k++) putc(getc(input), output);
		ks = FONDBBoxptr;
		swritetwo(xll, output); ks += 2; (void) ureadtwo(input);
		swritetwo(yll, output); ks += 2; (void) ureadtwo(input);
		swritetwo(xur, output); ks += 2; (void) ureadtwo(input);
		swritetwo(yur, output); ks += 2; (void) ureadtwo(input);
	}

	for (k = ks; k < WidthStart; k++) putc(getc(input), output);
	ks = WidthStart;
	for (k = FONDFirst; k <= FONDLast; k++) {
		(void) ureadtwo(input);			/* replace these */
/*		uwritetwo(charwidths[k], output); */
		swritetwo(charwidths[k], output);
	}
/*  doesn't do anything about kern pairs */
	while ((c = getc(input)) != EOF) putc(c, output);
}

/* read AFM file to extract Ascent, Descent, FontBBox, and character widths */

void readnewwidths(FILE *input) {
	int k, chr, wid, flag;
	char charname[64];
	
	while (fgets(line, MAXLINE, input) != NULL) {
		if (sscanf(line, "FontBBox %d %d %d %d",
			&xll, &yll, &xur, &yur) == 4) {
			xll = unmapwidth(xll);
			yll = unmapwidth(yll);
			xur = unmapwidth(xur);			
			yur = unmapwidth(yur);
		}
		else if (sscanf(line, "Ascender %d", &FONDAscent) == 1) {
			FONDAscent = unmapwidth(FONDAscent);
		}
		else if (sscanf(line, "Descender %d", &FONDDescent) == 1) {
			FONDDescent = -unmapwidth(FONDDescent);
		}
		else if (sscanf(line, "C %d ; WX %d ; N %s ;", 
			&chr, &wid, charname) == 3)	{
			flag = 0;
			for (k = 0; k < MAXCHRS; k++) {
				if (strcmp(encoding[k], charname) == 0) {
					charwidths[k] = unmapwidth(wid);
					flag = 1;
					break;
				}
			}
			if (flag == 0) fprintf(errout, "%s ? ", charname);
		}
	}
}

/* \tx  suppress AFM file output\n\ */

void showusage(char *name) {
	fprintf(errout, "%s [-{v}{z}{l}{h}{n}{b}] [-c=<encoding vector>]\n", name);
/*	fprintf(errout, "\t\t[-m=<afm-file>] [-f=<FOND ResID>]\n"); */
	fprintf(errout, "\t\t<Mac screen font file>\n");
	fprintf(errout, "\
\tv  verbose mode (sends lots of info to stdout)\n\
\tz  don't suppress zero width characters\n\
\tl  assume CM remap (0-9 => 161-170, 10-32 => 173-195, 127 => 196)\n\
\th  show hidden flags and reserved words\n\
\tn  show bitmap font tables in NFNT resource (lots of output!)\n\
\tb  show bitmaps of characters in NFNT resource (even more output!)\n\
\tc  vector specifying encoding used in corresponding outline font\n\
");
/*	putc('\n', errout); */
	fprintf(errout, "\
\t  (screen font may replace or modify this base encoding vector)\n");		
}

/* \te  use Mac encoding as base of font encoding\n\ */
/* \tm  make modified screen file based on AFM file (no AFM output)\n\ */
/* \t   (makes sense only when file contains single FOND resource)\n\ */
/* \tf  replace FOND ResID with new value specified\n\ */

/* \te  use Mac encoding as base for encoding vector\n\ */

/* NOTE: metrics modifications all apply only to LAST font seen in file */

/* \tt  trace mode\n\ */

/* remove file name - keep only path - inserts '\0' to terminate */
void removefilename(char *pathname) {
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
	scivilize(date);	 
	fprintf(outfile, "%s (%s %s)\n", 
		programversion, date, compiletime);
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(errout, "HASHED %ld\n", hash);	
/*	(void) getch();  */
	return hash;
}

char inafmfile[FILENAME_MAX];

int commandline(int argc, char *argv[], int firstarg) {
	char *s;
	int c;

	while (firstarg < argc && argv[firstarg][0] == '-') {
		s = argv[firstarg]+1;
		firstarg++;
		while ((c = *s++) != 0) {
			if (c == '?') {
				showusage(argv[0]);
				exit(1);
			}
			else if (c == 'v') verboseflag = 1;
			else if (c == 'h') showhidden = 1;			
/*			else if (c == 'e') usemacencoding = 1; */
			else if (c == 'e') basemacencoding = 1; 
			else if (c == 'E') errout = stderr;	/* 97/Sep/23 */
			else if (c == 't') traceflag = 1;
			else if (c == 'z') showzerowidths = 1;
			else if (c == 'l') remapflag = 1;
			else if (c == 'b') {
/*				shownfnttables = 1;  */
				showbitmaps = 1;
			}
			else if (c == 'n') shownfnttables = 1; 
/*			else if (c == 'x') {
				showwidthsflag = 0;
				showkernsflag = 0;
			} */
			else if (c == 'm') {
				changewidths = 1;
				if (*s++ == '=') {
					strcpy(inafmfile, s);
				}
				else {
					strcpy(inafmfile, argv[firstarg]);
					firstarg++;
				}
				showwidthsflag = 0;
				showkernsflag = 0;
				break;
			}
			else if (c == 'f') {
				changeResID = 1;
				if (*s++ == '=') {
					sscanf(s, "%d", &newResID);
				}
				else {
					sscanf(argv[firstarg], "%d", &newResID);
					firstarg++;
				}
				showwidthsflag = 0;
				showkernsflag = 0;
				break;
			}
			else if (c == 'c') {
				if (*s++ == '=') {
//					vector = s;
					vector = strdup(s);
				}
				else {
//					vector = argv[firstarg];
					vector = strdup(argv[firstarg]);
					firstarg++;
				}
				break;
			}
			else 
				fprintf(errout, "Don't understand command line flag %c\n", c);
		}
	}
	return firstarg;
}
	
/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

/* #define FNAMELEN 80 */

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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* analyses existing MacIntosh `screen font' */

/* if given second argument - reads that as AFM file and makes modified */

int main(int argc, char *argv[]) {
	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	char outafmfile[FILENAME_MAX];
	FILE *input, *output;
	int firstarg=1;
	char *s;
	int l;							/* 1993/Mar/7 */

/*	if (setvbuf(stdout, NULL, _IOLBF, 512))	
		printf("Unable to line buffer %s\n", "stdout"); */
/*	if (setvbuf(errout, NULL, _IOLBF, 512))	
		printf("Unable to line buffer %s\n", "errout"); */
/*	setvbuf(errout, NULL, _IONBF, 0); */		/* 97/Sep/13 */


	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("VECPATH")) != NULL)
//		vectorpath = s;
		vectorpath = strdup(s);
	else if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("VECPATH")) != NULL) 
//				vectorpath = s;
			vectorpath = strdup(s);		
	}

//	strncpy(programpath, argv[0], sizeof(programpath));
	programpath = strdup(argv[0]);
	removefilename(programpath);

/*  deal with command line arguments */
	if (argc < 2) {
		showusage(argv[0]);
		exit(1);
	}

	firstarg = commandline(argc, argv, 1);

	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASH %lu", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

/*	scivilize(compiledate);	 */
	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	for (l = firstarg; l < argc; l++) {
		strcpy(infile, argv[l]);			/* input Mac screen file */
		if((input = fopen(infile, "rb")) == NULL) {
			extension(infile, "scr");
			if ((input = fopen(infile, "rb")) == NULL) {
				perror(infile); 
				exit(3);
			}
		}

		if (showwidthsflag || showkernsflag) {
			if ((s=strrchr(infile, '\\')) != NULL) s++;
			else if ((s=strrchr(infile, ':')) != NULL) s++;
			else s = infile;
			strcpy(outafmfile, s);  	/* copy input file name minus path */

			forceexten(outafmfile, "afm");
			if((output = fopen(outafmfile, "w")) == NULL) {
				perror(outafmfile); exit(11);
			}
		}
		else output = NULL;

		if (readmacheader(input) != 0) {
			fclose(input);
			fclose(output);
			exit(9);
		}
		readresourceheader(input);
		readresourcemap(input, output);

		fclose(input);

		if (showwidthsflag || showkernsflag) {
			if (ferror(output) != 0) perror(outafmfile);
			else fclose(output);
		}

		if (changewidths) {
			extension(inafmfile, "afm");
			if((input = fopen(inafmfile, "rb")) == NULL) {
				perror(inafmfile); exit(11);
			}
			readnewwidths(input);
			fclose(input);

			if((input = fopen(infile, "rb")) == NULL) {
				perror(infile); exit(11);
			}

			if ((s=strrchr(infile, '\\')) != NULL) s++;
			else if ((s=strrchr(infile, ':')) != NULL) s++;
			else s = infile;
			strcpy(outfile, s);  	/* copy input file name minus path */

			forceexten(outfile, "new");
			if((output = fopen(outfile, "wb")) == NULL) {
				perror(outfile); exit(11);
			}
			insertnew(output, input);
			fclose(input);
			if (ferror(output) != 0) perror(outfile);
			else fclose(output);
		}
	}
	if (verboseflag) printf("Finished\n");
	fflush(stdout);
	return 0;
}

/* adjust file name, FOND name, and name in style table manually */

/* The old way:  FONT ResID = FOND ResID * 128 + point size */
/* and FONT resource with ResID = FOND ResID * 128 has resource name */

/* ResName starts with "I ", "B ", or "BI " for corresponding styles */
/* this prevents them from appearing in the font menu */ /* the OLD way */

/* could output each AFM `file' separately - but what file name to used ? */

/* adjust FONDResID and FONDFamID manually afterwards if needed */

/* NOTE: metrics modifications all apply only to LAST font seen in file */

/* allow specification of encoding vector ? */

/* is base encoding vector what is in font file, NOT Mac vector ? */

/* allow floats for widths and kern ? */

/* deal with non-integer widths and kerns (in Adobe coords) ? */

/* width table has two extra entries: width of default character & 0 */ 

/* possible problem with zero width characters ? */

/* FOND ID 17920 - 18431 Arabic */
/* FOND ID 18432 - 18943 Hebrew */
/* FOND ID 18944 - 19455 Greek */
/* FOND ID 19456 - 19967 Cyrillic */
