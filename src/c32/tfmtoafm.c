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

/* Program to make up AFM files from TFM files */

/* Needs encoding vector - since TFM file does not contain encoding */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#pragma warning(disable:4032)	// different type when promoted
#include <conio.h>
#pragma warning(default:4032)	// different type when promoted

/* #define FNAMELEN 80 */
#define MAXLINE 512
#define MAXCHRS 256
#define FONTNAME_MAX 80
#define CHARNAME_MAX 32
/* #define MAXPATHLEN 64 */
/* #define MAXBUFFER 8000 */

#define ITALICFUZZ 30		/* default italic fuzz */

/* #define NT */

#ifdef _WIN32
#define __far
#define _fmalloc malloc
#define _frealloc realloc
#define _ffree free
#endif

/* #define MAXLIG 512 */
/* #define MAXKERN 512 */

/* #define DEBUGGING */

/* Some inconsistencies in TFM file format: */

/* According to the book, nothing should be larger than 4096.0 ? */
/* But in cmman.tfm there occurs 4099.95, 4155.5 4907.41 */

double maxentry= 4096.0;

/* Also don't expect to see negative widths and such */
/* But in trip.tfm such things occur */

/* And characters aren't supposed to have zero width index */
/* but in cmsy and cmbsy char 54 and 55 do ... */

/* And the tables are supposed to be sorted, but AFM2TFM does not do this */

int verboseflag=0;
int traceflag=0;
int showencodeflag=0;
int diareaflag=0;
int detailflag=0;
int showcruft=0;
int showkernumbers=0;	/* debugging output of kerning info */
int showextra=0;		/* show Ascending and Extensible */
int writeextra=1;		/* write Ascending and Extensible */
int tableflag=1;
int showitalic=0;
int showratio=0;
int showactual=0;		/* do not convert coordinates - use scaled points */
int ascendanddescend=1;	/* write out capheight, ascender, descender */
int ansitexflag=1;		/* allow ANSI extended by TeX text 1993/Dec/18 */
int showansitex=0;		/* show ANSI extended stuff also 1993/Dec/18 */
int ansiextend=0;		/* extend ANSI in 16 - 32 for TeX for this font */
int descrlength=0;		/* compute rational description totals */

int rightboundary=-1;	/* right boundary character if any 97/Oct/1 */
int leftprogram=-1;		/* special left boundary character ligkern program */
int leftboundary=-1;	/* left boundary character,if any 97/Oct/1 */

/* double scale=1.000244141; *//* scale to fix EC font metrics (10pt) 1/4096 */

/* double scale=0.999997; */	/* scale to fix CM font metrics (10pt) */

double scale=1.0;

/* int tweak=-2; */			/* amount to adjust CM TFM numbers by */
double offset=-2;			/* amount to adjust CM TFM numbers by */

double charoffset=500.0;	/* offset to improve accuracy of aver st. dev. */

int denomlim=3600;			/* use by rationalize */
int numerlim=3600;			/* use by rationalize */

/* Following used by `EC compatible' fonts */

double cap_height, asc_height, acc_cap_height, desc_depth;
double max_height, max_depth, digit_width, cap_stem, baseline_skip;

int ecflag, mathsyflag, mathitflag, mathexflag;

double italicfuzz=ITALICFUZZ;

int standard=0;				/* non-zero if StandardEncoding */

int badencoding;

/* FILE *errout=stderr; */	/* where to send error output 96/Mar/3 */
FILE *errout=stdout;		/* where to send error output 97/Sep/23 */

#ifdef _WIN32
char *programversion =
/*  "TFMtoAFM (32) conversion utility version 2.0"; */ /* 98/Jan/26 */
/*  "TFMtoAFM (32) conversion utility version 2.1"; *//* 99/Apr/18 */
"TFMtoAFM (32) conversion utility version 2.1.1"; /* 00/Jun/30 */
#else
/* char *programversion = "TFMtoAFM conversion utility version 0.9"; */
/* char *programversion = "TFMtoAFM conversion utility version 1.0"; */
/* char *programversion = "TFMtoAFM conversion utility version 1.1"; */
/* char *programversion = "TFMtoAFM conversion utility version 1.2"; */
char *programversion =
/* "TFMtoAFM conversion utility version 1.2.1"; */ /* 94/June/17 */
/* "TFMtoAFM conversion utility version 1.3"; */ /* 95/Jan/2 */
/* "TFMtoAFM conversion utility version 1.3.1"; */ /* 96/Feb/25 */
/* "TFMtoAFM conversion utility version 1.3.2"; */ /* 96/Mar/3 */
/* "TFMtoAFM conversion utility version 1.3.3"; */ /* 96/Aug/3 */
/* "TFMtoAFM conversion utility version 1.3.4"; */ /* 96/Dec/26 */
/* "TFMtoAFM conversion utility version 1.4"; */ /* 97/Feb/7 */
/*  "TFMtoAFM conversion utility version 1.4.1"; */ /* 97/June/25 */
/*  "TFMtoAFM conversion utility version 1.4.2"; */ /* 97/July/24 */
/* "TFMtoAFM conversion utility version 1.5"; */ /* 97/Oct/1 */
"TFMtoAFM conversion utility version 1.5.1"; /* 97/Nov/17 */
#endif

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* #define COPYHASH 2244854 */
/* #define COPYHASH 2719644 */
/* #define COPYHASH 14008178 */
/* #define COPYHASH 3114191 */
/* #define COPYHASH 8997420 */
/* #define COPYHASH 14880649 */
/* #define COPYHASH 11982872 */
/* #define COPYHASH 12415373 */
/* #define COPYHASH 12847874 */
#define COPYHASH 5664704



static char *copyright = "\
Copyright (C) 1990-2000  Y&Y, Inc.  http://www.YandY.com\
";

/* Copyright (C) 1990--1997  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

int wantcpyrght=1;		/* want copyright message in there */

int vectorflag=0;		/* next arg is encoding vector */
int scaleargflag=0;		/* next arg is scale factor */
int offsetargflag=0;	/* next arg is tweak offset */
int extenflag=0;		/* next arg is extension to use */
int italicfuzzflag=0;	/* next arg is italic fuzz */

int commentflag=1;		/* non-zero => prefix TeX info with "Comment" */
int reencodeflag=0;		/* non-zero => read in encoding vector */

int showhidden=1;		/* show hidden information in TFM file */
int showchecksum=0;		/* show checksum in AFM file */
int warningflag=1;		/* give warnings in AFM file */

int totalcount;			/* number of characters */
int texthree=0;			/* non-zero if some feature of TeX 3.0 detected */

/* int limitatzero=1; */	/* limit height and depth at zero */

/* following for TeX CM fonts: */

int texcmflag = 0;		/* non-zero implies TeX CM flag => next three */

int offsetflag=0;		/* sub 2 from TFM long word numbers */ 
int scaleflag=0;		/* multiply by scale factor for EC Joerg Knappen */
int rationalize=0;		/* use rationalization to get "exact" tfm number */

int uppercaseflag=0;	/* output fontname is uppercase */
int usefilename=0;		/* use the file name for FontName */

char *vectorpath = "";	/*  path for vectors */

/* char programpath[MAXPATHLEN] = "c:\\dvipsone"; */

char programpath[FILENAME_MAX] = "c:\\yandy\\util";

char fontname[FONTNAME_MAX];

char line[MAXLINE];

unsigned long checksum; 

unsigned long checkdefault = 0x59265920;	/* default signature */

double designsize;		/* design size */

double italicangle;		/* slant angle of `vertical' stems */
double spacewidth;		/* normal spacing between words in text */
double spacestretch;	/* glue stretch between words */
double spaceshrink;		/* glue shrink between words */
double xheight;			/* size of one ex in the font - accents in line */
double emquad;			/* size of one em in the font */
double extraspace;		/* added to spacewidth at end of sentence */

double defrulethick;	/* thickness of \over bars - cmex */
double bigopspacing1;	/* min clearance above display op - cmex */
double bigopspacing2;	/* min clearance below display op - cmex */
double bigopspacing3;	/* min baselineskip above display op - cmex */
double bigopspacing4;	/* min baselineskip below display op - cmex */
double bigopspacing5;	/* padding above and below display limits - cmex */

double num1;			/* numer shift-up in display style */
double num2;			/* numer shift-up in non-display style non-atop */
double num3;			/* numer shift-up in non-display atop */
double denom1;			/* denom shift-down in display style */
double denom2;			/* denom shift-down in non-display styles */
double sup1;			/* superscript shift-up in uncramped display */
double sup2;			/* superscript shift-up in uncramped non-display */
double sup3;			/* superscript shift-up in cramped styles */
double sub1;			/* subscript shift-down if superscript is absent */
double sub2;			/* subscript shift-down if superscript is present */
double supdrop;			/* superscript baseline below top of large box */
double subdrop;			/* superscript baseline below bottom of large box */
double delim1;			/* size of atopwidthdelims delimites in display */
double delim2;			/* size of atopwidthdelims delimites in non-display */
double axisheight;		/* height of fraction line above baseline */

int extencount, ascendcount;

double fxll, fyll, fxur, fyur; 

int needspace;		/* stick space in as unencoded character */

int italicflag;		/* non-zero if font name suggests this is italic font */

int fixedpitchflag;	/* non-zero if font is fixed pitch */

/* int italicneeded; */ /* non-zero if italic correction table is needed */

int	lf, lh, bc, ec, nw, nh, nd,	ni, nl, nk, ne, np;

/*	lf	 length of file in words */ 
/*	lh	 length of header data in words */
/*	bc	 smallest character code */
/*	ec	 largest character code */
/*	nw	 number of words in the width table */
/*	nh	 number of words in the height table */
/*	nd	 number of words in the depth table */
/*	ni	 number of words in the italic correction table */
/*	nl	 number of words in the lig/kern table */ 
/*	nk	 number of words in the kern table */ 
/*	ne	 number of words in the extensible char table */ 
/*	np	 number of font parameter words */

/* char *ext="afm"; */

/* int = 64; nl = 0; nk = 0; ne = 0; np = 7; */

#define MAXSTRINGPOOL (MAXCHRS * CHARNAME_MAX / 2)

int stringindex=0;					/* index into following */

char stringpool[MAXSTRINGPOOL];		/* 1994/June/17 space for encoding */

/* char encoding[MAXCHRS][CHARNAME_MAX];	 */
char *encoding[MAXCHRS];					/* 1994/June/17 */

/* unsigned char buffer[MAXBUFFER]; */

unsigned char __far *buffer=NULL; 

#define MAXCODINGSCHEME (10 * 4)

#define MAXFONTID (5 * 4)

char codingscheme[MAXCODINGSCHEME];		/* Xerox PARC header data */
char fontid[MAXFONTID];			/* Xerox PARC header data */

unsigned long faceword=0;	/* Xerox PARC header data */ /* NA */
int facebyte=0;				/* Xerox PARC header data */

int charstart, widthstart, heightstart, depthstart, italicstart;
int ligkernstart, kernstart, extenstart, paramstart;

#define MAXVECTOR 260		/* max path name */

char *commandvector="";				/* specified on command line */

char codingvector[MAXVECTOR]="";	/* which coding scheme in use */
									/* could include path of vector file */

char checksumvector[MAXVECTOR]="";	/* derived from checksum */

/* following are for when we have to guess about the encoding vector */

/* the following have all been lower cased ... */

char *texturesvec="textures";		/* default encoding vector */
char *textypevec="textutyp";		/* default encoding vector */

char *encodingnames[][2] = {
{"TeX text", "textext"},				/* cmr*, cmb*, cmsl*, cmss* */
/*		or			"texital"		for			cmti* */
{"TeX text without f-ligatures", "textype"},	/* cmcsc10 & cmr5 */
{"TeX typewriter text", "typewrit"},		/* cm*tt* */
/*		or			"typeital"		for			cmitt* */
{"TeX extended ASCII", "texascii"},			/* cmtex* */
{"TeX math italic", "mathit"},				/* cmmi* */
{"TeX math symbols", "mathsy"},				/* cmsy* */
{"TeX math extension", "mathex"},			/* cmex10 */
{"ASCII caps and digits", "textext"},		/* cminch */
{"TeX text in Adobe setting", "textext"},	/* pctex style ? */
{"Adobe StandardEncoding", "standard"},
{"AdobeStandardencoding", "standard"},
{"StandardEncoding", "standard"},			/* 99/Apr/18 */
{"Adobe Symbol Encoding", "symbol"},
{"Adobe Dingbats Encoding", "dingbats"},
{"Microsoft Windows ANSI 3.0", "ansi"},
{"Microsoft Windows ANSI 3.1", "ansinew"},
{"WindowsANSIEncoding", "ansinew"},			/* 99/Apr/18 */
/* {"microsoft windows ansi", "ansi"}, */
/* {"microsoft windows new ansi", "ansinew"}, */
/* {"tex ansi windows 3.0", "texansi"}, */
{"TeX ANSI Windows 3.1", "texannew"},
{"Ventura Publisher Encoding", "ventura"},
{"TeX 256 character encoding", "tex256"},
{"CorkEncoding", "tex256"},					/* 99/Apr/18 */
{"T1Encoding", "tex256"},					/* 99/Apr/18 */
{"Extended TeX Font Encoding - Latin", "tex256"},
{"Extended TeX Font Encoding - Latin", "cork"},
{"TeX text companion symbols 1---TS1", "ts1"},
{"Macintosh", "mac"},
{"Cyrillic", "cyrillic"},
/* {"dvips new", "neonnew"},	*/					/* 92/Dec/22 */
{"TEX TEXT + ADOBESTANDARDENCODING", "neonnew"},	/* 92/Dec/22 */
/* {"ansi", "tex&ansi"}, */							/* 94/Jun/20 */
{"TeX typewriter and Windows ANSI", "texnansi"},	/* 94/Dec/31 */
{"TeXnANSIEncoding", "texnansi"},					/* 99/Apr/18 */
{"fontspecific", "numeric"},						/* 95/Apr/8 */
{"TeXBase1Encoding", "8r"},							/* 98/Feb/12 */
{"mh2scrEncoding", "mh2scr"},						/* 99/Apr/18 */
// {"MTEX", "mtex"},									/* 00/Jun/30 */
// {"MTSY", "mtsy"},									/* 00/Jun/30 */
// {"MTSYN", "mtsyn"},									/* 00/Jun/30 */
// {"MTMI", "mtmi"},									/* 00/Jun/30 */
// {"RMTMI", "rmtmi"},									/* 00/Jun/30 */
{"", ""}
 };	
  
/* AEFMNOPST only */

/* {"fontspecific", "numeric"}, */		/* ??? */

/* encoding vector for TeX roman text fonts */

/* static unsigned char *cmrencoding[] = { */

char *cmrencoding[] = { 
"Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon", 
"Phi", "Psi", "Omega", "ff", "fi", "fl", "ffi", "ffl", 
"dotlessi", "dotlessj", "grave", "acute", "caron", "breve", "macron", "ring", 
"cedilla", "germandbls", "ae", "oe", "oslash", "AE", "OE", "Oslash", 
"suppress", "exclam", "quotedblright", "numbersign", "dollar", "percent", "ampersand", "quoteright", 
"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", 
"zero", "one", "two", "three", "four", "five", "six", "seven", 
"eight", "nine", "colon", "semicolon", "exclamdown", "equal", "questiondown", "question", 
"at", "A", "B", "C", "D", "E", "F", "G", 
"H", "I", "J", "K", "L", "M", "N", "O", 
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "quotedblleft", "bracketright", "circumflex", "dotaccent", 
"quoteleft", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", 
"x", "y", "z", "endash", "emdash", "hungarumlaut", "tilde", "dieresis"};

/* accents are at 193 to 207 */
char *standardaccents[] =
{ "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
 "dieresis", "ring", "cedilla", "hungarumlaut", "ogonek", "caron",
 "dotlessi"};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

struct ratio { /* structure for rational numbers */
	long numer;
	long denom;
};

/* compute good rational approximation to floating point number */
/* assumes number given is positive */ /* number theory at work! */

struct ratio rational(double x, long nlimit, long dlimit) {
	long p0=0, q0=1, p1=1, q1=0, p2, q2;
	double s=x, ds;
	struct ratio res;

	if (showratio != 0) 
		printf("Entering rational %g %ld %ld\n", x, nlimit, dlimit);  
	
	if (x < 0.0) {		/* negative */
		res = rational(- x, nlimit, dlimit);
		res.numer = - res.numer;
		return res;
	}

	if (x == 0.0) {
			res.numer = 0; res.denom = 1;
			return res;		/* the answer is 0/1 */
	}
	for(;;) {
		p2 = p0 + (long) s * p1;
		q2 = q0 + (long) s * q1;
		if (showratio != 0) 
		  printf("%ld %ld %ld %ld %ld %ld %g\n", p0, q0, p1, q1, p2, q2, s);  
		if (p2 > nlimit || q2 > dlimit || p2 < 0 || q2 <= 0) {
			p2 = p1; q2 = q1; break;
		}
		if ((double) p2 / (double) q2 == x) break;
		ds = s - (double) ((long) s);
		if (ds == 0.0) break;
		p0 = p1; q0 = q1; p1 = p2; q1 = q2;	s = 1/ds;
/*		catch large s that will overflow when converted to long */
		if (s > 1000000000.0 || s < -1000000000.0) break; /* 1992/Dec/6 */
	}
	res.numer = p2; res.denom = q2;
	if (showratio != 0) 
		printf(" return with %ld/%ld = %lg\n", p2, q2, (double)p2/(double)q2);
	return res;		/* the answer is p2/q2 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *strdupx (char *name) {
	int n = strlen(name) + 1;
	char *s;

	if (stringindex + n >= MAXSTRINGPOOL) {
		fprintf(stderr, "ERROR: out of space for encoding vector\n");
		exit(1);
	}
	s = stringpool + stringindex;
	strcpy (s, name);
	stringindex = stringindex + n;
	return s;
}

/**********************************************************************/

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/**********************************************************************/

/* int getline(FILE *input, unsigned char *buff, int nmax) { */
int getline(FILE *input, char *buff, int nmax) {
	int c, k=0;
/*	unsigned char *s=buff; */
	char *s=buff;

	while ((c = getc(input)) != '\n' && c != EOF) {
		*s++ = (unsigned char) c;
		k++;
		if (k >= nmax) {
			*s = '\0';
			fprintf(stderr, "Line too long: %s\n", buff);
			exit (13);
		}
	}
	if (c != EOF) {
		*s++ = (unsigned char) c;
		k++;
	}
	*s = '\0';
/*	if (traceflag != 0) printf("%s", buff); */
	return k;
}

/* int getrealline(FILE *input, unsigned char *buff, int nmax) {*/
int getrealline(FILE *input, char *buff, int nmax) {
	int k;
	k = getline(input, buff, nmax);
	while ((*buff == '%' || *buff == '\n') && k > 0)
		k = getline(input, buff, nmax);		
	return k;
}

/****************************************************************************/

#define KERNBIT 128
#define STOPBIT 128

/* Following is the tfm to afm conversion stuff */

int readint(int k) {	 /* read 16-bit count from buffer */
	int a, b, k2= k << 1, res;
/*	a = buffer[k2]; b = buffer[k2+1]; */
	a = *(buffer + k2); b = *(buffer + k2 + 1);
/*	assert (a >= 0); assert (b >= 0); */
	res = (a << 8) | b;
	return res;
}

// read 32-bit number from buffer --- k is number of 32 bit word

long readlong(int k) {
	int a, b, c, d, k4 = k << 2;
	long res;
/*	a = buffer[k4]; b = buffer[k4+1]; */
/*	c = buffer[k4 + 2];  d = buffer[k4 + 3];  */
	a = *(buffer+k4); b = *(buffer+k4+1);
	c = *(buffer + k4 + 2);  d = *(buffer + k4 + 3);  
/*	assert (a >= 0); assert (b >= 0); assert(c >= 0); assert(d >= 0); */
	res = ((long) a) << 24 | ((long) b) << 16 | ((long) c << 8) | d;
	return res;
}

/* 1048576 = 2 ^ 20 */

double unmaplong(long n) {		/* map from tfm scaled integer */
	double res;
	struct ratio rats;
	long lres;

	if (n == 0) return 0.0;
	else if (n < 0) return - unmaplong( - n);
	else {
/*		if (tweakflag != 0)	n = n - tweak; */		/* ATTEMPT TO CORRECT */
/*		res = ((double) n) * 1000.0 / 1048576.0; */
		if (offsetflag) res = ((double) n + offset) / 1048576.0;	/* 2^20 */
		else res = ((double) n) / 1048576.0;		/* 2^20 */
		if (scaleflag) res = res * scale;	/* 97/Jun/20 */
		if (rationalize != 0) {
			rats = rational(res, (long) denomlim, (long) numerlim);
			res = (double) rats.numer / (double) rats.denom;
		}
		res = res * 1000.0;					/* Adobe units 1000 per em */   
/*		res = ((double) ((long) (res * 1000.0))) / 1000.0; */
/*		try and round to nearest thousand of an Adobe unit */
/*		res = ((double) ((long) (res * 1000.0 + 0.499999))) / 1000.0; */
		lres = (long) (res * 1000.0 + 0.4999999);
#ifdef IGNORED
		if ((lres % 100) == 01) lres = lres - 1; /* if last 2 digit off by 1 */
		if ((lres % 100) == 99) lres = lres + 1; /* if last 2 digit off by 1 */
#endif
		if ((lres % 1000) == 001) lres = lres - 1; /* last 3 digit off by 1 */
		if ((lres % 1000) == 002) lres = lres - 2; /* last 3 digit off by 2 */
		if ((lres % 1000) == 999) lres = lres + 1; /* last 3 digit off by 1 */
		if ((lres % 1000) == 998) lres = lres + 2; /* last 3 digit off by 2 */
		res = ((double) lres) / 1000.0;
		return res;
	}
}

double unmapread (int k) {
	if (showactual) return (double) readlong(k);
	return unmaplong(readlong(k));
}

void pause (void) {
	fprintf(errout, "Press any key to continue . . .\n");
	putc(7, errout);
	fflush(stdout);
	(void) _getch();
}

void showwords (int inx, int n) {
	int k;
	long w;
	double x;

	printf("\n");
	for(k = 0; k < n; k++) {
		w = readlong(inx + k);
		x = unmaplong(w);
		printf("%d %ld %lg\n", k, w, x);
	}
	pause();
}

double pi = 3.141592653;

void readparams (void) { /* read the (7, 6, 22, or 12 or 16) parameters */
/*	int n=1; */ /* not referenced */
	double tantheta, theta;
	
	ecflag = mathsyflag = mathitflag = mathexflag = 0;

	italicangle = 0.0; spacewidth = 0.0; spacestretch = 0.0;
	spaceshrink = 0.0; xheight = 0.0; 
	emquad = 0.0; extraspace = 0.0;
	if (np < 1) return;	/* 1 */
 	tantheta = unmapread(paramstart) / 1000.0;	/* param 1 */
	theta = atan(tantheta);
	italicangle = - (theta * 180.0) / pi; 
/*	italicangle = - (theta * 180.0) / pi;  */
	if (np < 2) return;	/* 2 */
	spacewidth = unmapread(paramstart + 1);		/* param 2 */
	if (np < 3) return;	/* 3 */
	spacestretch = unmapread(paramstart + 2);	/* param 3 */
	if (np < 4) return;	/* 4 */
	spaceshrink = unmapread(paramstart + 3);	/* param 4 */
	if (np < 5) return;	/* 5 */
	xheight = unmapread(paramstart + 4);		/* param 5 */
	if (np < 6) return;	/* 6 */
	emquad = unmapread(paramstart + 5);			/* param 6 */
	if (np == 6) mathitflag = 1;
	if (np < 7) return;	/* 7 */
	extraspace = unmapread(paramstart + 6);		/* param 7 */
	if (np < 8) return;	/* 8 */
	if (np >= 22) { /* for cmsy fonts */
		mathsyflag = 1;
		num1 = unmapread(paramstart + 7);
		num2 = unmapread(paramstart + 8);
		num3 = unmapread(paramstart + 9);
		denom1 = unmapread(paramstart + 10);
		denom2 = unmapread(paramstart + 11);		
		sup1 = unmapread(paramstart + 12);
		sup2 = unmapread(paramstart + 13);
		sup3 = unmapread(paramstart + 14);
		sub1 = unmapread(paramstart + 15);
		sub2 = unmapread(paramstart + 16);
		supdrop = unmapread(paramstart + 17);
		subdrop = unmapread(paramstart + 18);
		delim1 = unmapread(paramstart + 19);
		delim2 = unmapread(paramstart + 20);
		axisheight = unmapread(paramstart + 21);
	}
	else if (np >= 16) {	/* for EC */
		ecflag = 1;
		cap_height = unmapread(paramstart + 7);		/* param 8 */
		asc_height = unmapread(paramstart + 8);		/* param 9 */
		acc_cap_height = unmapread(paramstart + 9);	/* param 10 */
		desc_depth = unmapread(paramstart + 10);	/* param 11 */
		max_height = unmapread(paramstart + 11);	/* param 12 */
		max_depth = unmapread(paramstart + 12);		/* param 13 */
		digit_width = unmapread(paramstart + 13);	/* param 14 */
		cap_stem = unmapread(paramstart + 14);		/* param 15 */
		baseline_skip = unmapread(paramstart + 15);	/* param 16 */
	}
	else if (np >= 13) {	/* for cmex */
		mathexflag = 1;
		defrulethick = unmapread(paramstart + 7); /* over bar */
		bigopspacing1 = unmapread(paramstart + 8); /* above op */
		bigopspacing2 = unmapread(paramstart + 9); /* below op */
		bigopspacing3 = unmapread(paramstart + 10); /* skip above */
		bigopspacing4 = unmapread(paramstart + 11); /* skip below */
		bigopspacing5 = unmapread(paramstart + 12); /* padding */
	}
	else printf("WARNING: peculiar number of parameters (%d)\n", np);
}

/* convert to lower case letters */
/* void lowercase(unsigned char *s, unsigned char *t) {  */
void lowercase (char *s, char *t) { 
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'A' && c <= 'Z') *s++ = (unsigned char) (c + 'a' - 'A');
		else *s++ = (unsigned char) c;
	}
	*s++ = (unsigned char) c;
}

void uppercase (char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

void readheader (void) {		/* see whether Xerox PARC style header */
	int k, nlen;

	char *s;

	s = codingscheme;				// starts at byte 32
/*	nlen = buffer[24 + 8]; */		/* step over counts two header words */
	nlen = *(buffer + 24 + 8);		/* step over counts two header words */
	if (nlen > MAXCODINGSCHEME) nlen = MAXCODINGSCHEME;		// sanity check
	for (k = 0; k < nlen; k++)
/*		*s++ =  buffer[24 + 8 + 1 +k]; */
		*s++ =  *(buffer + 24 + 8 + 1 + k);
	*s = '\0';
	printf("CODINGSCHEME: %s (%d)\n", codingscheme, nlen);	// debugging only
	if (strcmp(codingscheme, "UNSPECIFIED") == 0) 
		strcpy(codingscheme, "");					/* 1993/Feb/9 */

	s = fontid;						// starts at byte 72
/*	nlen = buffer[24 + 8 + 40]; */	/* step over counts and header words */
	nlen = *(buffer + 24 + 8 + 40);	/* step over counts and header words */
	if (nlen > MAXFONTID) nlen = MAXFONTID;		// sanity check
	for (k = 0; k < nlen; k++)
/*		*s++ = buffer[24 + 8 + 40 + 1 + k]; */
		*s++ = *(buffer + 24 + 8 + 40 + 1 + k); 
	*s = '\0';
	printf("FONTID: %s (%d)\n", fontid, nlen);	// debugging only
	if (strcmp(fontid, "UNSPECIFIED") == 0) 
		strcpy(fontid, "");					/* 1993/Feb/9 */

	faceword = (unsigned long) readlong(6 + 2 + 10 + 5);	/* ??? */
/*	facebyte = buffer[24 + 8 + 40 + 20]; */ /* ??? */
/*  facebyte = faceword & 255; */	
}

int tfmfixedpitch(void) {	/* see whether characters are fixed pitch */
	int k, windex = -1, wold = -1;
	long charinfo;
/*	double charwidth; */

	for (k = bc; k <= ec; k++) {
		charinfo = readlong(charstart + k - bc);
		if (charinfo == 0) continue;
		windex = (int) (charinfo >> 24) & 255;
		if (wold < 0) wold = windex;
		else if (wold != windex) return 0;		/* not fixed pitch */
	}
/*	if (windex >= 0) charwidth = unmapread(widthstart + windex);
	else charwidth = 0.0; */					/* NEVER */
	return 1;									/* fixed pitch */
}

int spacepresent(void) { 
	int k;
	for (k = bc; k <= ec; k++) {
		if (strcmp(encoding[k], "space") == 0) return -1;
	}
	return 0;
}

int tfmcharcount(void) {	/* see how many actually valid characters */
	int k, n=0;
	int bcs=bc;				/* 1993/Dec/18 */
	long charinfo;
/*	bcs = bc; */
	if (ansiextend) {
		if (!showansitex) {			/* count only from above 32 */
			if (bc < 32) bcs = 32;
		}
	}
	for (k = bcs; k <= ec; k++) {
		charinfo = readlong(charstart + k - bc);
		if (charinfo != 0) n++;
	}
	return n;
}

/* need to count kerns first so can output correct count in AFM file */
/* ahead of actual kerning information */

int countkerns(FILE *output) { /* what a pain - have to do two passes ! */
	int windex, hindex, dindex, iindex, tag, remain;
	int liginx, kerninx, skipbyte;
/*	long extension;  */
	int top, mid, bot, rep;
	long charinfo, ligkern;
	int opbyte, remainder;		/* for > 255 steps TeX release 3.0 */
	int nkl=0, k, warntag=0, kex;
	int nextchar;				/* not accessed */
	double kernam;				/* not accessed */

/*	warntag=0; */
	if (traceflag != 0) printf("\nCounting kern pairs:\n");
	for (k = bc; k <= ec; k++) {
		charinfo = readlong(charstart + k - bc);
		if (charinfo == 0) continue;
		windex = (int) (charinfo >> 24) & 255;
		hindex = (int) (charinfo >> 20) & 15;
		dindex = (int) (charinfo >> 16) & 15;
		iindex = (int) (charinfo >> 10) & 63;
		tag = (int) charinfo >> 8 & 3;
		remain = (int) charinfo & 255;
/*		if (traceflag != 0)
			printf("chr %d w %d h %d d %d i %d t %d r %d\t(%lo)\n",
				k, windex, hindex, dindex, iindex, tag, remain, charinfo); */
/*		we have already seen the above */	/* 1993/June/24 */
		if (tag > 0) {
			if (tag > 1) {	/* ascending chain or extensible char */
				if (traceflag != 0) printf("tag: %d ", tag);
				if (tag == 2) {
					if (showextra != 0) {
/*						printf("Ascender %d, next %d\n", k, remain); */
						printf("Comment Ascending %d, %d\n", k, remain);
					}
					if (writeextra != 0)
						fprintf(output, "Comment Ascending %d, %d\n", k, remain);
					ascendcount++;
					warntag++;
					continue;
				}
				else if (tag == 3) {
/*						extension = readlong(extenstart + remain); 
						top = (extension >> 24) & 255;
						mid = (extension >> 16) & 255;
						bot = (extension >> 8) & 255;
						rep = extension & 255; */
						kex = (extenstart + remain) << 2;
/*						top = buffer[kex];  */
/*						mid = buffer[kex + 1]; */
/*						bot = buffer[kex + 2]; */
/*						rep = buffer[kex + 3]; */
						top = *(buffer + kex); 
						mid = *(buffer + kex + 1);
						bot = *(buffer + kex + 2);
						rep = *(buffer + kex + 3);
					if (showextra != 0) {
						printf("Extensible %d ", k);
						printf("top %d mid %d bot %d rep %d\n",
							top, mid, bot, rep);
					}
					if (writeextra != 0) {
						fprintf(output, "Comment Extensible %d ", k);
						fprintf(output, "top %d mid %d bot %d rep %d\n",
							top, mid, bot, rep);
					}
					extencount++;
					warntag++;
					continue;
				}
			}
/*			assert (tag == 1); */
/*			tag == 1 -- character has lig-kern program */
			liginx = ligkernstart + remain;
			ligkern = readlong(liginx);
			skipbyte = (int) ((ligkern >> 24) & 255);
			if (skipbyte > 128) {	/* first word in lig-kern program */
				texthree++;
				opbyte = (int) ((ligkern >> 8) & 255);
				remainder = (int) (ligkern & 255); 
				liginx = (opbyte << 8) + remainder;
/*				if (verboseflag != 0) printf("liginx %d ", liginx); */
				if (liginx >= nl) {
					fprintf(stderr, "ERROR: impossible ligkern index %d (> %d)\n", 
						liginx, nl);
					exit(7);
				}
				if (traceflag != 0) 
					printf("TeX 3.0: char %d skipbyte %d > 128 (liginx %d)\n", 
						k, skipbyte, liginx);
				liginx = liginx + ligkernstart; /* NEW */
				ligkern = readlong(liginx);	
			}			/* dealt with indirect pointers */

			for (;;) {
				skipbyte = (int) ((ligkern >> 24) & 255);
				opbyte = (int) ((ligkern >> 8) & 255); 
				remainder = (int) (ligkern & 255); 
				if ((opbyte & KERNBIT) != 0) {
					nextchar = (int) (ligkern >> 16) & 255;
/*					kerninx = (int) ligkern & 255;  OLD */
					if ((opbyte & 127) != 0) {
						texthree++;		/* TeX 3.0 */
						if (traceflag != 0)
						printf("TeX 3.0: char %d opbyte & 127 not zero (%d)\n",
								k, opbyte & 127);
					}
					kerninx = ((opbyte & 127) << 8) | remainder; /* new */
					kernam = unmapread(kernstart + kerninx);
					nkl++;
				}
/*				if ((ligkern >> 24) & STOPBIT) != 0) break; */
				if ((skipbyte & STOPBIT) != 0) break;	/* end of program */
				if ((skipbyte & 127) != 0) {	/* skip forward */
					texthree++;
					liginx = liginx + (skipbyte & 127);
					if (traceflag != 0) 
					printf("TeX 3.0: char %d skipbyte %d > 128 (liginx %d)\n", 
						k, skipbyte, liginx);
				}
				liginx++;
				ligkern = readlong(liginx);	
			}
		}
	}
	if (warntag > 0 && ascendcount > 0 && writeextra == 0)
		fprintf(output, 
			"Comment WARNING: ascendible characters (%d)\n", ascendcount);
	if (warntag > 0 && extencount > 0 && writeextra == 0)
		fprintf(output, 
			"Comment WARNING: extensible characters (%d)\n", extencount);
	return nkl;
}

int isititalic(char *name) {
	if (strncmp(name, "cmr", 3) == 0) return 0;
	if (strncmp(name, "cmssi", 5) == 0) return -1;
	if (strncmp(name, "cmssqi", 6) == 0) return -1;
	if (strncmp(name, "cms", 3) == 0) return 0;
	if (strncmp(name, "cmti", 4) == 0) return -1;
	if (strncmp(name, "cmbxti", 6) == 0) return -1;	
	if (strncmp(name, "cmitt", 5) == 0) return -1;	
	if (strncmp(name, "cmu", 3) == 0) return -1;	
	if (strncmp(name, "cmfi", 3) == 0) return -1;	
	return 0;
}

void setcmrencoding (void) {
	int k;
	if (traceflag != 0) 
		printf ("Setting up CM encoding\n");		/* DEBUGGING ONLY */
/*	for (k = 0; k < 128; k++) strcpy(encoding[k], cmrencoding[k]); */
	for (k = 0; k < 128; k++) encoding[k] = cmrencoding[k];
}

void extension (char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void forceexten (char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

void setupnumeric (void) {
	int k;
	char charcode[CHARNAME_MAX]; 
	for(k = 0; k < MAXCHRS; k++) {
/*		sprintf(encoding[k], "a%d", k); */
		sprintf(charcode, "a%d", k);
		encoding[k] = strdupx(charcode);
	}
}

/* int readencoding(unsigned char *name, int flag) { */
int readencoding(char *name, int flag) {
	char fn_in[FILENAME_MAX];
	char charcode[CHARNAME_MAX]; 
	FILE *input=NULL;
	int k, n;				/* not accessed */
	char *s, *t;

	stringindex=0;					/* reset string pool index */

	if (strcmp(name, "numeric") == 0) {	/* 1993/July/13 */
		setupnumeric();
		return 0;
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
			s = vectorpath;
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
				else {
					*t = ';';					// put back semicolon
					s = t+1;					/* continue after ; */
				}
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
		fprintf(errout, "ERROR: Can't open encoding vector (%s)\n", fn_in);
		perror(name);
		if (strcmp(name, "textext") != 0) setcmrencoding();	/*  */
			return -1;
	}

	if (flag > 0) /*  && flag == 0  or  && pass == 0  */
		printf("Using encoding vector file: `%s'\n", fn_in);

	n=0;
	stringindex=0;					/* reset string pool index */
	for (k = 0; k < MAXCHRS; k++)		 /* 93/Aug/31 */
/*		strcpy(encoding[k], ""); */
		encoding[k] = "";				/* 94/June/17 */

	while (getrealline(input, line, MAXLINE) > 0) {
		if (*line == '%' || *line == ';') continue;		/* ignore comment */
/*		if (sscanf(line, "%d %s", &k, &charcode) < 2) { */
		if (sscanf(line, "%d %s", &k, charcode) < 2) {		
			fprintf(errout, "Don't understand encoding line: %s", line);
		}
/*		else strcpy(encoding[k], charcode); */
		else encoding[k] = strdupx(charcode);		/* 94/June/17 */
/*		if (k == 0) printf("Encoding[0] is %s\n", charcode); */
		n++;
/*		printf("%d %s", n, line); */
	}
/*	if (n != 128 && standardflag == 0) {
		fprintf(errout, "Bad count in encoding vector file (%d)\n", n);
	} */
	if (ferror(input) != 0) {
		fprintf(errout, "Error in encoding vector file read\n");
	}
	else fclose(input);
	if (showencodeflag != 0) {
		for (k = 0; k < MAXCHRS; k++) {
			if (strcmp(encoding[k], "") != 0)
				printf("%d %s\n", k, encoding[k]);
		}
	}
/*	dotlessi, caron, breve should never occur since not in ANSI, but... */
/*	ring versus degree is problem: PSCRIPT says 176 is degree, ATM says ring */
#ifdef IGNORE
	if (ansiextend) {
		strcpy(encoding[16], standardaccents[13]);	/* dotlessi ? */
													/* dotlessj ? */
		strcpy(encoding[18], encoding[96]);			/* grave */
		strcpy(encoding[19], encoding[180]);		/* acute */
		strcpy(encoding[20], standardaccents[12]);	/* caron ? */
		strcpy(encoding[21], standardaccents[5]);	/* breve ? */
		strcpy(encoding[22], encoding[175]);		/* macron */
		strcpy(encoding[23], encoding[176]);		/* ring */ /* degree ? */
		strcpy(encoding[24], encoding[184]);		/* cedilla */
		strcpy(encoding[25], encoding[223]);		/* germandbls */
		strcpy(encoding[26], encoding[230]);		/* ae */
		strcpy(encoding[27], encoding[156]);		/* oe */
		strcpy(encoding[28], encoding[248]);		/* oslash */
		strcpy(encoding[29], encoding[198]);		/* AE */
		strcpy(encoding[30], encoding[140]);		/* OE */
		strcpy(encoding[31], encoding[216]);		/* Oslash */
	}
#endif
	if (ansiextend) {						/* 1994/June/17 */
		encoding[16] = standardaccents[13];	/* dotlessi ? */
											/* dotlessj ? */
		encoding[18] = encoding[96];		/* grave */
		encoding[19] = encoding[180];		/* acute */
		encoding[20] = standardaccents[12];	/* caron ? */
		encoding[21] = standardaccents[5];	/* breve ? */
		encoding[22] = encoding[175];		/* macron */
		encoding[23] = encoding[176];		/* ring */ /* degree ? */
		encoding[24] = encoding[184];		/* cedilla */
		encoding[25] = encoding[223];		/* germandbls */
		encoding[26] = encoding[230];		/* ae */
		encoding[27] = encoding[156];		/* oe */
		encoding[28] = encoding[248];		/* oslash */
		encoding[29] = encoding[198];		/* AE */
		encoding[30] = encoding[140];		/* OE */
		encoding[31] = encoding[216];		/* Oslash */
	}
	return 0;	/* indicate success ? */
}

/* flag is 0 for anything but kerns, since normally sorted and start at zero */
/* flag is 1 for kerns in following, since they need not be sorted */
/* check various tables (specified by wordstart and of given name */

int checkwords(int wordstart, int num, char *name, int flag)  {
	int k;
	double val, old, sorted=1, inrange=1;
	
	old = unmapread(wordstart);
	if (flag == 0) {
		if (old != 0.0) 
			fprintf(errout, "Zeroth entry in %s table not zero (%lg)\n", 
				name, old);
		old = unmapread(wordstart + 1);		/* step over zero entry */
	}
	for (k=1; k < num; k++) {
		val = unmapread(wordstart + k);
/*		if ((val < 0.0 && flag == 0)  || val > MAXENTRY)  */
		if (val < - maxentry  || val > maxentry) 
			inrange = 0;
/*			fprintf(errout, 
				"Value %lg at %d (of %d) in %s table out of range\n",
					val, k, num, name); */
		else if (val < old && flag == 0) 
			sorted = 0;
/*			fprintf(errout, "Value %lg at %d (of %d) in %s table <= %lg \n",
				val, k, num, name, old); */
		old = val;
	}
/*	if (inrange == 0) { */
	if (inrange == 0) {
		if (showactual == 0) {
			fprintf(errout,
					"WARNING: %s table entries out of range (> %lg)\n", 
				name, maxentry);
			return 0;
		}
	}
	if (sorted == 0 && flag == 0) {
		fprintf(errout, "WARNING: %s table is not sorted\n", name);
		return 0;
	}
	return -1;
}

void showtable (int wordstart, int n, char *name)  {
	int k;
	double val;
	
	fprintf(errout, "%s table starts at word %d, length %d\n", name, wordstart, n);
	for (k=0; k < n; k++) {
		if (k % 6 == 0 && k != 0) putc('\n', errout);
		val = unmapread(wordstart + k);
		fprintf(errout, "%d: %lg, ",	k, val); 
	}
	putc('\n', errout);
/*	pause(); */
}

/* Mac style code: 1 - bold, 2 - italic, 4 - underline, 8 - outline */
/* 16 - shadow, 32 - condensed, 64 - extended, 128 - reserved */

/* Encoding: 0 - none, 1 TeX text, 2 TeX type, 4 Lucida Math */

void showcode (unsigned int code) {
	if (code == 0) printf("None ");
	else if ((code & 3) == 1) printf("TeX text ");
	else if ((code & 3) == 2) printf("TeX type ");	
	else if ((code & 4) != 0) printf("Lucida Math ");	
	else printf("Other %d ", code);
}

void macfaceword (unsigned long face) {	/* analyze TeXtures style faceword */
	unsigned int qdcode, pscode;
/*	if (verboseflag) printf("Face %08lX\n", face); */
	if (face == 0) return;				/* 1996/Dec/26 */
	if (face == 0x80000000) return;		/* 1996/Dec/26 */
	putc('\n', stdout);
	if ((face & 1) != 0) printf("BOLD ");
	if ((face & 2) != 0) printf("ITALIC ");
	qdcode = (unsigned int) (face >> 24) & 255;
	pscode = (unsigned int) (face >> 16) & 255;
	if (qdcode != 0 || pscode != 0) {
		printf("QD code: "); showcode(qdcode); 
		printf("PS code: "); showcode(pscode);
	}
	putc('\n', stdout);
}

int	checkansiextend (void) {
	int flag = 0;
	char *s;
#ifdef IGNORED
	if (strcmp(codingvector, "ansinew") == 0 ||			/* 1993/Dec/18 */
		strcmp(codingvector, "ansi") == 0) {
		if (ansitexflag) flag = 1;
	}
#endif
	s = removepath(codingvector);
	if (strncmp(s, "ansinew", 7) == 0) {	 		/* 1996/Aug/3 */
		if (ansitexflag) flag = 1;
	}
	
	return flag;
}

/* Takes first six letters of encoding vector name and compresses */
/* into four bytes using base 40 coding */ /* 40^6 < s^32 */
/* Treats lower case and upper case the same, ignores non alphanumerics */
/* Except handles -, &, and _ */	/* This *decodes* from checksum */

/* writes back into first six chars of vector */

int decodefourty (unsigned long checksum, char *vector) {
	int c;
	int k;
/*	char vector[6+1]; */

/*	if (checksum == checkdefault) { */
	if (checksum == 0) {
		strcpy(vector, "unknown");
		return 1;
	}
	else if ((checksum >> 8) == (checkdefault >> 8)) {	/* last byte random */
		strcpy (vector,  "native");		/* if not specified ... */
		return 1;
	}
	else {
		for (k = 0; k < 6; k++) {
/*			c = (int) (checksum % 36); */
			c = (int) (checksum % 40);
/*			checksum = checksum / 36; */
			checksum = checksum / 40;
			if (c <= 'z' - 'a' ) c = c + 'a';
			else if (c < 36) c = (c + '0') - ('z' - 'a') - 1;
			else if (c == 36) c = '-';
			else if (c == 37) c = '&';
			else if (c == 38) c = '_';
			else {
/*				printf("c = %d ", c); */	/* can only be 39 */
				c = '.';				/* unknown */
			}
			vector[5-k] = (char) c;
		}
		vector[6] = '\0';
	}
/*	printf("Reconstructed vector %s\n", vector); */
/*	return strdup(vector); */
	return 0;
}

int log2 (long p) {
	int bits=0;
	while (p != 0) {
		p = (p >> 1);
		bits++;
	}
	return bits;
}

void showstarts (void) {
	putc('\n', stdout);
	printf("%s table: ", "Width");
	showtable(widthstart, nw, "Width");
	putc('\n', stdout);
	printf("%s table: ", "Height");
	showtable(heightstart, nh, "Height");
	putc('\n', stdout);
	printf("%s table: ", "Depth");
	showtable(depthstart, nd, "Depth");
	putc('\n', stdout);
	printf("%s table: ", "Italic");
	showtable(italicstart, ni, "Italic");
	putc('\n', stdout);
	printf("%s table: ", "Kern");
	showtable(kernstart, nk, "Kern");
	putc('\n', stdout);
}

void showligkern (long ligkern) {
	int skipbyte, nextchar, opbyte, remainder, liginx;
	skipbyte = (int) ((ligkern >> 24) & 255);
	nextchar = (int) ((ligkern >> 16) & 255);
	opbyte = (int) ((ligkern >> 8) & 255);
	remainder = (int) (ligkern & 255); 
	liginx = (opbyte << 8) + remainder;
	printf("skip %d next %d op %d rem %d ", skipbyte, nextchar, opbyte, remainder);
}	/* debugging code 97/Oct/3 */

/* find the character whose program starts at leftprogram */
/* this assumes we always point to the same first word for given char */

int findcharinfo(int leftprogram) {
	long charinfo, ligkern;
	int remain;
	int skipbyte, nextchar, opbyte, remainder, liginx;
	int k;

	for (k = bc; k <= ec; k++) {
		charinfo = readlong(charstart + k - bc);
		if (charinfo == 0) continue;
		remain = (int) charinfo & 255;
		if (remain == leftprogram) {
			if (traceflag)
				printf("char %d starts at %d\n", k, remain);
			return k;
		}
		ligkern = readlong(ligkernstart + remain);
/*		also follow pointer if this is indirection table */
		skipbyte = (int) ((ligkern >> 24) & 255);
		if (skipbyte > 128) {
			nextchar = (int) ((ligkern >> 16) & 255);
			opbyte = (int) ((ligkern >> 8) & 255);
			remainder = (int) (ligkern & 255); 
			liginx = (opbyte << 8) + remainder;
			if (liginx == leftprogram) {
				if (traceflag)
					printf("char %d starts at %d\n", k, liginx);
				return k;
			}
		}
	}
	return -1;
}

void tfmanalyze (FILE *output) { /* turn tfm into afm like output */
	int windex, hindex, dindex, iindex, tag, remain;
	double charheight, charwidth, chardepth, charitalic;
	double charwidthoff;
	double charxur;
	int liginx, nextchar, ligchar, kerninx;
	long charinfo, ligkern, wrd;
/*	double tantheta, theta; */
	double kernam;
	int skipbyte, opbyte, remainder;	/* for > 255 steps TeX release 3.0 */
	int k, n, nkl, nklnew;
	int nligs;					/* not accessed */
	int kold=-1, nextold=-1;
	int texturesflag = 0;
/*	int bcs=bc; */
	int bcs;
/*	int shownumeric; */
/*	char *vector; */
	char *s;
	double uctotal, lctotal, dgtotal;			/* 97/June/23 */
	double uc2total, lc2total, dg2total;		/* 97/June/24 */
/*	long numertotal, denomtotal; */				/* 97/July/24 */
	double numertotal, denomtotal;				/* 97/July/24 */

	if (verboseflag != 0 && showcruft != 0) {
		printf("Now decoding TFM information\n");
		fprintf(errout, "Press any key to continue . . .\n");
		fflush(stdout);
		(void) _getch();
	}

	lf = readint(0); lh = readint(1); bc = readint(2); ec = readint(3);
	nw = readint(4); nh = readint(5); nd = readint(6); ni = readint(7);
	nl = readint(8); nk = readint(9); ne = readint(10); np = readint(11);

/*	if (verboseflag != 0) {
		printf("lf %d, lh %d, bc %d, ec %d\n", lf, lh, bc, ec);
		printf("nw %d, nh %d, nd %d, ni %d, nl %d, nk %d, ne %d, np %d\n",
			nw, nh, nd, ni, nl, nk, ne, np);
	}  */

	if (verboseflag != 0 && showcruft != 0 || traceflag != 0) printf(
"lf %d lh %d bc %d ec %d nw %d nh %d nd %d ni %d nl %d nk %d ne %d np %d \n",
				lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np);

	if (lf != 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np
		|| ne > 256) {
		fprintf(stderr, 
			"Inconsistent start of TFM file (lengths don't add up)\n");
		exit(19);
	}
	charstart = 6 + lh;
	widthstart = charstart + (ec - bc + 1);
	heightstart = widthstart + nw;
	depthstart = heightstart + nh;
	italicstart = depthstart + nd;
	ligkernstart = italicstart + ni;
	kernstart = ligkernstart + nl;
	extenstart = kernstart + nk;
	paramstart = extenstart + ne;
/*	assert (paramstart + np == lf); */

/*	check very first instruction of ligkern array */
	ligkern = readlong(ligkernstart);
	skipbyte = (int) ((ligkern >> 24) & 255);
/*	printf("0 %d %8lo skip %d\n", 0, ligkern, skipbyte); */
	if (skipbyte == 255) {
		nextchar = (int) (ligkern >> 16) & 255;		/* 97/Oct/1 */
		rightboundary=nextchar;
		texthree++;	/* right boundary character */
		if (traceflag != 0)
			printf("TeX 3.0: right boundary character (%d)\n", rightboundary);
/*		if (verboseflag != 0) {
			printf("TeX 3.0: right boundary character %s (%d)\n",
				   encoding[rightboundary], rightboundary);
		} */ /* we don't have the encoding vector yet */
	}

/*	check very last instruction of ligkern array */
	ligkern = readlong(ligkernstart + nl - 1);
	skipbyte = (int) ((ligkern >> 24) & 255);
/*	printf("nl-1 %d %8lo skip %d\n", nl-1, ligkern, skipbyte); */
	if (skipbyte == 255) {
		opbyte = (int) ((ligkern >> 8) & 255); 
		remainder = (int) (ligkern & 255); 
		leftprogram = (opbyte << 8) + remainder;	/* 97/Oct/1 */
/*		printf("opbyte %d remainder %d\n", opbyte, remainder); */
		texthree++;	/* left boundary character ligkern program */
		if (traceflag) 
			printf("TeX 3.0: %s boundary character ligkern program at %d\n",
				   "left", leftprogram);
	}
	
	if ((wrd = readlong(widthstart)) != 0) 
		fprintf(errout, "width[0] = %ld not zero\n", wrd);
	if ((wrd = readlong(heightstart)) != 0)	
		fprintf(errout, "height[0] = %ld not zero\n", wrd);
	if ((wrd = readlong(depthstart)) != 0)	
		fprintf(errout, "depth[0] = %ld not zero\n", wrd);
	if ((wrd = readlong(italicstart)) != 0)	
		fprintf(errout, "italic[0] = %ld not zero\n", wrd);

	if(checkwords(widthstart, nw, "Width", 0) == 0) { /* widths neg ? */
		showtable(widthstart, nw, "Width"); /* pause(); */
	}
	if(checkwords(heightstart, nh, "Height", 0) == 0) {
		showtable(heightstart, nh, "Height"); /* pause(); */
	}
	if(checkwords(depthstart, nd, "Depth", 0) == 0) { /* depths neg ? */
		showtable(depthstart, nd, "Depth"); /* pause(); */
	}
	if(checkwords(italicstart, ni, "Italic", 0) == 0) {
		showtable(italicstart, ni, "Italic"); /* pause(); */
	}
/*	NOTE: kern table typically is NOT sorted so kill warning */ 
	if(checkwords(kernstart, nk, "Kern", 1) == 0) {
		showtable(kernstart, nk, "Kern"); /* pause(); */
	}

	if (traceflag != 0) showstarts();

//	checksum is four bytes staring at byte 24 (word 6)

	checksum = readlong(6);
	designsize = unmapread(6 + 1)/1000.0;

	lowercase(fontname, fontname);
	italicflag = isititalic(fontname);	/* try and guess if italic font */
/*	following moved here 1993/June/12 */
	fixedpitchflag =  tfmfixedpitch();	/* try and guess if fixed pitch */

/*	if (verboseflag) printf("lh %d (should be 16+2)\n", lh); */
	
	if (lh > 16+2)
		printf("WARNING: Unusually %s header %d %s 16+2 words\n", "long", lh, ">");
	if (lh < 16+2)
		printf("WARNING: Unusually %s header %d %s 16+2 words\n", "short", lh, "<");

/*	changed 99/Apr/18 to accomadate AFM2TFM generated TFMs */
/*	if (lh >= 16+2) */				/* is it Xerox PARC style header ? */
	if (lh >= 16+1) {				/* is it Xerox PARC style header ? */
		readheader();
		if (strncmp(codingscheme, "PostScript ", 11) == 0) {
			texturesflag = 1;
			printf("TeXtures PS FontName %s\n         QD fontname %s ", 
				codingscheme+11, fontid);
			macfaceword(faceword);
		}
		else if (strncmp(codingscheme, "PS ", 3) == 0) {
			texturesflag = 1;
			printf("TeXtures PS FontName %s\n         QD fontname %s ", 
				codingscheme+3, fontid);
			macfaceword(faceword);
		}
/*		Following may be problem if encoding scheme happens to equal FontID */
		else if (strcmp(codingscheme, fontid) == 0) {	/* 1993/June/18 */
			if (faceword != 0) {						/* 96/Dec/26 */
				texturesflag = 1;
				printf("TeXtures FontName %s (VF %d bytes) ", 
					   codingscheme, (lh - (16+2)) << 2);
				macfaceword(faceword);
			}
		}
		else if (verboseflag) {
			printf("CodingScheme `%s'\nFontID `%s' ", codingscheme, fontid);
			if ((faceword & 0XFFFFFF00) != 0)
				printf("FaceWord %08X\n", faceword);
			else printf("FaceByte %02X\n", faceword);
		}		
/*		else if (reencodeflag == 0)  */
		if (reencodeflag == 0) {
			if (texturesflag != 0) strcpy(line, "");  /* avoid misinterpret */
/*			else lowercase(line, codingscheme); */ /* not needed anymore */
			else strcpy(line, codingscheme);
			strcpy(codingvector, "");
/*			for (k = 0; k < encodingcount; k++)   */
			for (k = 0; k < 64; k++) { 
				if (strcmp(encodingnames[k][0], "") == 0) break;
				if (strcmp(encodingnames[k][1], "") == 0) break;
/*				if (strcmp(line, encodingnames[k][0]) == 0)  */
/*				printf("line %s encodingnames[%d][0]\n",
					   line, k, encodingnames[k][0]); */
				if (_strcmpi(line, encodingnames[k][0]) == 0) {
					strcpy(codingvector, encodingnames[k][1]);
					break;
				}
			}
			if (strcmp(codingvector, "") == 0) {
				if (texturesflag == 0) {
					printf("WARNING: don't recognize coding scheme %s\n",
						codingscheme);
					printf(
	"         please specify encoding vector on command line using -c=...\n");
				}
				if (texturesflag != 0) 	{
					if (fixedpitchflag != 0) strcpy(codingvector, textypevec);
					else strcpy(codingvector, texturesvec);
				}
				else {
#ifdef IGNORED
					if (fixedpitchflag != 0) {
						if (italicflag != 0) strcpy(codingvector, typeital);
						else strcpy(codingvector, typevec);
					}
					else if (italicflag != 0) strcpy(codingvector, italicvec);
						else strcpy(codingvector, defaultvec); /* default */
#endif
/* override the above nonsense! */
					strcpy(codingvector, "numeric");
				}
			}
		}
/*		lowercase(line, codingscheme); */

/*		if (standardflag != 0) strcpy(codingvector, "standard"); */
/*		if (traceflag != 0)  */
		if (verboseflag != 0)
			printf("Using `%s' encoding vector\n", codingvector); 
		ansiextend = checkansiextend();
		(void) readencoding(codingvector, 1);
		if ((strcmp(codingvector, "textext") == 0 ||
			strcmp(codingvector, "textype") == 0) 
				&& italicflag != 0) {
/*			strcpy(encoding[36], "sterling"); */	/* dollar => sterling */
			encoding[36] = "sterling";		/* dollar => sterling */
		}
	} /* end of if (lh >= 16 +2) */
	else {
/*		printf("WARNING: not Xerox style header (%d)\n", lh); */
		printf("WARNING: not Xerox style header (lh %d != 16+2)\n", lh);
/*		if (traceflag != 0)
			printf("Using `%s' encoding vector\n", codingvector);
		ansiextend = checkansiextend();
		(void) readencoding(codingvector, 1); */
	}

	if (decodefourty(checksum, checksumvector) == 0) {
		s = removepath(codingvector);			/* 96/Aug/3 */
		n = strlen(s);							/* 00/Jun/30 */
/*		printf("l(checksumvector) %d l(encodingvector) %d\n",
			   strlen(checksumvector), strlen(s)); */	/* debug */
/*		if (strncmp(checksumvector, s, 6) == 0) { */
		if (_strnicmp(checksumvector, s, n) == 0) {
			if (verboseflag)
				printf("CheckSum `%s..' matches encoding vector `%s'\n",
					checksumvector, s);
		}
		else {
			if (descrlength == 0) fprintf(errout,
	"WARNING: CheckSum `%s..' (%d) does not match encoding vector `%s' (%d)\n",
				checksumvector, strlen(checksumvector), s, strlen(s));
		}
	}

/*	for (k = 0; k < bc; k++) strcpy(encoding[k], "");
	for (k = ec + 1; k < MAXCHRS; k++) strcpy(encoding[k], ""); */

	if (traceflag != 0) printf("TFM file has %d parameters\n", np);
	if (np == 6) {
		printf("WARNING: Only 6 Params (OK for math italic)\n");
		printf("WARNING: ExtraSpace parameter missing\n");
	}
	else if (np < 7) 
		printf("WARNING: Fewer than 7 Params (%d)\n", np); 
	readparams();
	if (np == 13) printf("WARNING: 13 Params (OK for math extension)\n");
	else if (np == 16) {
		if (descrlength == 0)
			printf("WARNING: 16 Params (OK for EC style text fonts)\n");
	}
	else if (np == 22) printf("WARNING: 22 Params (OK for math symbol)\n");	
	else if (np > 7) 
		printf("WARNING: More than 7 Params (%d)\n", np); 

	if (traceflag != 0 || (verboseflag != 0 && showcruft != 0)) 
		printf("designsize = %lg\n", designsize);

/*	if (designsize < 5 || designsize > 17.5) */
	if (designsize < 5 || designsize > 17.5) {
		if (showactual == 0)
			printf("WARNING: Unusual design size (%lg)\n", designsize);
	}

	if (tableflag != 0 && showcruft != 0) {
		printf("\nHeader:"); showwords(6, lh);
		printf("\nWidths:"); showwords(widthstart, nw);
		printf("\nHeights:"); showwords(heightstart, nh);
		printf("\nDepths:"); showwords(depthstart, nd);
		printf("\nItalics:"); showwords(italicstart, ni);
		printf("\nKerns:"); showwords(kernstart, nk);
		printf("\nParams:"); showwords(paramstart, np);
	}
	
	if (verboseflag != 0 && showcruft != 0) printf("\n");
	if (verboseflag != 0 && leftprogram >= 0) {
		leftboundary = findcharinfo(leftprogram);
		if (leftboundary >= 0)
			printf("TeX 3.0: %s boundary char %s (%d)\n",
				   "left", encoding[leftboundary], leftboundary);
		else {
/*			in this case maybe we should dump it out in KernPairs ? */
			printf("TeX 3.0: unnamed left boundary char prog at %d\n",
				   leftprogram);
		}
	}
	if (verboseflag != 0 && rightboundary >= 0) {
		printf("TeX 3.0: %s boundary char %s (%d)\n",
			   "right", encoding[rightboundary], rightboundary);
	}
	fprintf(output, "StartFontMetrics 2.0\n");
	if (uppercaseflag != 0) uppercase(fontname, fontname);
	if (usefilename != 0 || strcmp(fontid, "") == 0) {
		fprintf(output, "Comment The following `FontName' is merely the file's name\n");
		fprintf(output, "FontName %s\n", fontname);
	}
	else {
		fprintf(output, "Comment The following `FontName' is merely the FontID\n");
		fprintf(output, "FontName %s\n", fontid);
	}

/*	fprintf(output,	"Comment FullName, FamilyName, Weight not available\n"); */
	fprintf(output,
	"Comment FullName, FamilyName, Weight, Version, Notice not available\n");
/*	fprintf(output, "Comment Version, Notice not available\n"); */
/*	fprintf(output, "Comment UnderlinePosition, UnderlineThickness not available\n"); */
	fprintf(output, "UnderlinePosition -100\n");	/* 1993/May/9 */
	fprintf(output, "UnderlineThickness 50\n");		/* 1993/May/9 */
/*	if (strcmp(codingvector, "standard") == 0) standard = 1; */
	s = removepath(codingvector);
/*	if (strcmp(codingvector, "standard") == 0) standard = 1; */
	if (strncmp(s, "standard", 8) == 0) standard = 1;
	else standard = 0;
	if (standard == 0) fprintf(output, "EncodingScheme FontSpecific\n");
	else fprintf(output, "EncodingScheme AdobeStandardEncoding\n");  

/*	FullName, FamilyName, Weight ? */
/*	tantheta = unmapread(paramstart) / 1000.0;
	theta = atan(tantheta);
	italicangle = - (theta * 180.0) / pi; */
	italicangle = ((double) ((long) (italicangle * 1000.0 + 0.5))) / 1000.0;
	fprintf(output, "ItalicAngle %lg\n", italicangle);
	/* IsFixedPitch, UnderlinePosition, UnderlineThickness */
	/* Version, Notice */
/*	fxll = 0.0; fyll = 0.0; fxur = 0.0; fyur = 0.0; */
	fxll = 0.0; 
	fyll = - unmapread(depthstart + nd - 1);
	fxur = unmapread(widthstart + nw - 1); /* + italic ? */
	fyur = unmapread(heightstart + nh - 1);
/*	fprintf(output, "FontBBox %lg %lg %lg %lg\n", 
		fxll, fyll, fxur, fyur); */
	fprintf(output, "FontBBox %d %d %d %d\n",
		(int) fxll, (int) fyll, (int) fxur, (int) fyur);
/*	fixedpitchflag =  tfmfixedpitch();	*/ /* try and guess if fixed pitch */
	if (fixedpitchflag != 0) {
		printf("WARNING: fixed pitch font\n");
		fprintf(output, "IsFixedPitch true\n");
	}
	else fprintf(output, "IsFixedPitch false\n");
/*	xheight = unmapread(paramstart + 4); */
	if (xheight != 0) fprintf(output, "XHeight %lg\n", xheight);

	if (ascendanddescend != 0) { /*  not meaningful for math fonts ... */
/*	generate XHeight	   => lowercase 'x' ? already done */
		if (xheight == 0) {		/* unless already done */
			charinfo = readlong(charstart + 'x' - bc);
			if (charinfo != 0) {
				hindex = (int) (charinfo >> 20) & 15;
				charheight = unmapread(heightstart + hindex);
				fprintf(output, "XHeight %lg\n", charheight);
			}
		}
/*	generate CapHeight  => uppercase 'H' */
		if (ecflag) charheight = cap_height;
		else {
			charinfo = readlong(charstart + 'H' - bc);
			if (charinfo != 0) { 
				hindex = (int) (charinfo >> 20) & 15;
				charheight = unmapread(heightstart + hindex);
			}
			else charheight = 0;
		}
		if (charheight != 0) fprintf(output, "CapHeight %lg\n", charheight);
/*	generate Ascender   => lowercase 'd' */
		if (ecflag) charheight = asc_height;
		else {
			charinfo = readlong(charstart + 'd' - bc);
			if (charinfo != 0) {
				hindex = (int) (charinfo >> 20) & 15;
				charheight = unmapread(heightstart + hindex);
			}
			else charheight = 0;
		}
		if (charheight != 0) fprintf(output, "Ascender %lg\n", charheight);
/*	generate Descender  => lowercase 'p' or 'q' */
		if (ecflag) chardepth = desc_depth;
		else {
			charinfo = readlong(charstart + 'q' - bc);
			if (charinfo != 0) {
				dindex = (int) (charinfo >> 16) & 15;
				chardepth = unmapread(depthstart + dindex);
			}
			else chardepth = 0;
		}
		if (chardepth != 0) fprintf(output, "Descender %lg\n", - chardepth);
	}

	if (offsetflag) fprintf(output, "Comment Offset %lg\n", offset); 
	if (scaleflag) fprintf(output, "Comment Scale %lg\n", scale);
	if (rationalize)
		fprintf(output, "Comment Rationalizing %d %d\n", denomlim, numerlim); 

	if (showhidden != 0) {
		fprintf(output, "Comment following is extra info from TFM file\n");
		if (strcmp(fontid, "") != 0) {
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "FontID %s\n", fontid);
		}
		if (designsize != 0.0) {
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "DesignSize %lg (pts)\n", designsize);
		}
		if (strcmp(codingscheme, "") != 0) {
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "CharacterCodingScheme %s\n", codingscheme);
		}
		if (facebyte != 0) {
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Facebyte %d\n", facebyte); 
		}
		if (leftboundary >= 0) {
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "LeftBoundary %s\n", encoding[leftboundary]);
		}

		if (rightboundary >= 0) {
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "RightBoundary %s\n", encoding[rightboundary]);
		}
/* the normal set of parameters */
/*		if (spacewidth != 0.0 || spacestretch != 0.0 || spaceshrink != 0) { */
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Space %lg %lg %lg\n",
				spacewidth, spacestretch, spaceshrink);
/*		}  */
/*		if (extraspace != 0.0) { */
		if (np >= 7) {		/* only math italic lacks this */
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "ExtraSpace %lg\n",	extraspace);
		}
/*		if (emquad != 0.0) { */
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Quad %lg\n", emquad);
/*		} */
		if (showchecksum != 0) {
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "CheckSum %08lX\n", checksum);
		}

		if (np >= 22) {			/* math symbol fonts */
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Num %lg %lg %lg\n", num1, num2, num3);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Denom %lg %lg\n", denom1, denom2);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Sup %lg %lg %lg\n", sup1, sup2, sup3);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Sub %lg %lg\n", sub1, sub2);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Supdrop %lg\n", supdrop);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Subdrop %lg\n", subdrop);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Delim %lg %lg\n", delim1, delim2);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Axisheight %lg\n", axisheight);
		}
		else if (np >= 16) {	/* EC fonts */
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "CapHeight %lg\n", cap_height);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Ascender %lg\n", asc_height);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "AccentedCapHeight %lg\n", acc_cap_height);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "Descender %lg\n", desc_depth);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "MaxHeight %lg\n", max_height);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "MaxDepth %lg\n", max_depth);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "FigureWidth %lg\n", digit_width);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "StemV %lg\n", cap_stem);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "BaseLineSkip %lg\n", baseline_skip);
		}
		else if (np >= 13) {			/* math extension fonts */
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "DefaultRuleThickness %lg\n", defrulethick);
			if (commentflag != 0) fprintf(output, "Comment ");
			fprintf(output, "BigOpSpacing %lg %lg %lg %lg %lg\n", 
				bigopspacing1, bigopspacing2, bigopspacing3,
					bigopspacing4, bigopspacing5); 
		}

		if (warningflag > 0) {
/*			if (np > 7 && np != 13 && np != 22)  */
			if (np > 7 && np != 13 && np != 16 && np != 22) 
				fprintf(output,				
"Comment WARNING: extra parameters (%d) not given in AFM file\n",
				np - 7);
			if (ne > 0) {
				fprintf(output, 
"Comment WARNING: extensible and ascendible character info in TFM file\n");
				fprintf(output,
"Comment WARNING: these appear at end and will need to be moved back here!\n");
			}
		}
		fprintf(output, 
"Comment BBox xur - char width + %lg => italic correction\n", italicfuzz);
	}	

/*	ec - bc + 1 */
	nligs = 0;								/* count ligatures seen */
	if (spacepresent() == 0 && spacewidth > 0.0) needspace = 1;
	else needspace = 0;
	totalcount = tfmcharcount() + needspace;
	fprintf(output, "StartCharMetrics %d\n", totalcount);
	if (traceflag != 0) printf("\nCreating CharMetrics:\n");
	bcs = bc; 
	if (ansiextend) {
		if (!showansitex) {
			if (bc < 32) bcs = 32;
		}
	}

	uctotal = lctotal = dgtotal = 0;
	uc2total = lc2total = dg2total = 0;
/*	numertotal = denomtotal = 0; */
	numertotal = denomtotal = 0.0;
/*	for (k = bc; k <= ec; k++) { */
	for (k = bcs; k <= ec; k++) {		/* step through characters */
		charinfo = readlong(charstart + k - bc);
		if (charinfo == 0) continue;
/*		if (verboseflag!= 0) (void) _getch(); */
		windex = (int) (charinfo >> 24) & 255;
		hindex = (int) (charinfo >> 20) & 15;
		dindex = (int) (charinfo >> 16) & 15;
		iindex = (int) (charinfo >> 10) & 63;
		tag = (int) charinfo >> 8 & 3;
		remain = (int) charinfo & 255;
		if (traceflag != 0) {	/* detailed debugging 97/Oct/1 */
			printf(
"chr %3d w %3d h %2d d %2d i %2d t %1d r %3d (%11lo %08lX) ",
				k, windex, hindex, dindex, iindex, tag, remain,
				   charinfo, charinfo);
			if (tag == 1) {
				liginx = ligkernstart + remain;
				ligkern = readlong(liginx);
				showligkern(ligkern);
				skipbyte = (int) ((ligkern >> 24) & 255);
				if (skipbyte > 128) {
					opbyte = (int) ((ligkern >> 8) & 255);
					remainder = (int) (ligkern & 255); 
					liginx = (opbyte << 8) + remainder;
					liginx = liginx + ligkernstart; /* NEW */
					ligkern = readlong(liginx);
					showligkern(ligkern);
				}
			}
			putc('\n', stdout);
		}
		charwidth = unmapread(widthstart + windex);
		if (charwidth == 0.0) {
			if (descrlength == 0)			
				printf("WARNING: character %s (%d) has zero width\n",
				   encoding[k], k);
		}
		charheight = unmapread(heightstart + hindex);
		chardepth = unmapread(depthstart + dindex);
		charitalic = unmapread(italicstart + iindex);
		fprintf(output, "C %d ; ", k);
		fprintf(output, "WX %lg ; ", charwidth);
/*		if (k == 0) printf("Encoding[0] is %s\n", encoding[k]); */
		if (strcmp(encoding[k], "") == 0) {
			fprintf(errout, "ERROR: char %d not in encoding\n", k);
			fprintf(output, "N a%d ; ", k);
			badencoding++;
		}
		else fprintf(output, "N %s ; ", encoding[k]);
		charxur = (charwidth + charitalic - italicfuzz);
		if (charxur < 0.0 && charwidth >= 0.0) charxur = 0.0;
		if (charxur == charwidth) charxur++; /* prevent ambiguity 1993/May/9 */
		if (strcmp(encoding[k], "space") == 0) charxur = 0; /* 1992/Oct/12 */
		fprintf(output, "B %lg %lg %lg %lg ; ", 0.0, - chardepth, 
/*			(charwidth + charitalic), charheight); */
			charxur, charheight);	/* 92/Mar/16 */
/* following uncommented for debugging only */
/*		if (showitalic != 0) fprintf(output, "; I %lg ;", charitalic); */
		if (tag == 1) {				/* try and deal with ligatures now */

			liginx = ligkernstart + remain;
			ligkern = readlong(liginx);
			skipbyte = (int) ((ligkern >> 24) & 255);
			if (skipbyte > 128) {	/* first word in lig-kern program */
/*				texthree++;  already done ? */
				opbyte = (int) ((ligkern >> 8) & 255);
				remainder = (int) (ligkern & 255); 
				liginx = (opbyte << 8) + remainder;
				if (liginx >= nl) {
					fprintf(stderr, "Impossible ligkern index %d (> %d)\n", 
						liginx, nl);
					exit(7);
				}
				if (traceflag != 0) 
					printf("TeX 3.0: char %d skipbyte %d > 128 (liginx %d)\n", 
						k, skipbyte, liginx);
				liginx = liginx + ligkernstart; /* NEW */
				ligkern = readlong(liginx);	
			}
			for (;;) {
				if (diareaflag != 0) 
					printf("liginx %d ligkern %ld\n", liginx, ligkern); 
				skipbyte = (int) ((ligkern >> 24) & 255);
				opbyte = (int) ((ligkern >> 8) & 255); 
				remainder = (int) (ligkern & 255); 
				if ((opbyte & KERNBIT) == 0) {
/*				if (((ligkern >> 8) & KERNBIT) == 0) */
					nextchar = (int) (ligkern >> 16) & 255;
					ligchar = (int) ligkern & 255;
					if (strcmp(encoding[nextchar], "") == 0) {
						fprintf(errout, 
							"ERROR: Next char (%d) in ligature not in encoding\n",
								nextchar);
						badencoding++;
					}
					if (strcmp(encoding[ligchar], "") == 0) {
						fprintf(errout, 
							"ERROR: Lig char (%d) in ligature not in encoding\n",
								ligchar);
						badencoding++;
					}
/*					fprintf(output, "L %s %s ; ", 
						encoding[nextchar], encoding[ligchar]); */
/* following expanded out 1994/June/20 */
					fputs("L ", output);
/* try and catch `hyphen' `hyphen' => `hyphen' */ /* 96/Feb/26 */
					if (nextchar == ligchar && k != nextchar &&
						strcmp(encoding[k], "hyphen") == 0 &&
							strcmp(encoding[nextchar], "hyphen") == 0) {
							fputs("sfthyphen sfthyphen", output);
					}
					else {
						if (strcmp(encoding[nextchar], "") != 0) 
							fputs(encoding[nextchar], output);
						else fprintf(output, "a%d", nextchar);
						putc (' ', output);
						if (strcmp(encoding[ligchar], "") != 0) 
							fputs(encoding[ligchar], output);
						else fprintf(output, "a%d", ligchar);
					}
					fputs(" ; ", output);
					nligs++;					/* count ligatures */
				}
/*				if (((ligkern >> 24) & STOPBIT) != 0) break; */
				if ((skipbyte & STOPBIT) != 0) break;	/* end of program */
				if ((skipbyte & 127) != 0) {	/* skip forward */
/*					texthree++; */
					liginx = liginx + (skipbyte & 127);
				}				
				liginx++;
				ligkern = readlong(liginx);	
			}
		}
		if (showitalic != 0 && charitalic != 0.0) 
			fprintf(output, "I %lg ;", charitalic);
		fprintf(output, "\n");
		charwidthoff= charwidth - charoffset;
		if (k >= 'A' && k <= 'Z') {
			uctotal += charwidthoff;
/*			uc2total += charwidth*charwidth; */
			uc2total += charwidthoff*charwidthoff;
		}
		if (k >= 'a' && k <= 'z') {
			lctotal += charwidthoff;
/*			lc2total += charwidth*charwidth; */
			lc2total += charwidthoff*charwidthoff;
		}
		if (k >= '0' && k <= '9') {
			dgtotal += charwidthoff;
			dg2total += charwidthoff*charwidthoff;
		}
		if (descrlength) {
			struct ratio q;
/*			q = rational(charwidth, 32767, 1131); */
/*			q = rational(charwidth, 65535, 1131); */
			q = rational(charwidth, 131071, 1131);
/*			numertotal += log2(q.numer); */
			if (q.numer != 0)
				numertotal += log((double) q.numer);
/*			denomtotal += log2(q.denom); */
			denomtotal += log((double) q.denom);
/*			printf("%d\t%lg\t%ld / %ld\t%lg . %lg\n",
				k, charwidth, q.numer, q.denom,
				log(q.numer), log(q.denom)); */
/*			printf("%lg %lg\n", numertotal, denomtotal); */
		}
	} /* end of for (k = bcs; k <= ec; k++) */

	if (verboseflag) {
		double aver, variance, stdev;
		int nchr = (26 + 26 + 10);
		aver = (uctotal + lctotal + dgtotal) / nchr;
		variance = (uc2total + lc2total + dg2total) / nchr - aver*aver;
		if (variance >= 0.0) stdev = sqrt(variance);
		else {
			if (variance < -0.000001)
				printf("Negative variance? %lg\n", variance);
			stdev = 0.0;
		}
		aver = aver + charoffset;
		printf("Average Character Width %lg (st. dev. %lg)\n", aver, stdev);
	}
/*		printf("%lg %lg\n", numertotal, denomtotal); */
	if (descrlength) {
		double numerav, denomav;
		numerav = numertotal / ((double) totalcount * log(2.0));
		denomav = denomtotal / ((double) totalcount * log(2.0));
		printf("Average rational decription (scale %lg) %lg (%lg / %lg) bits\n",
			   scale, numerav + denomav, numerav, denomav);
	}

/*	next come unencoded characters C -1 ... */
	if (spacepresent() == 0 && spacewidth > 0.0)
	   fprintf(output, "C -1 ; WX %lg ; N space ; B 0 0 0 0 ;\n", spacewidth);
	fprintf(output, "EndCharMetrics\n");

/*  now kerning information */	
/*	nkl = nl - nligs; */	/* entries in lig/kern table minus ligatures */
	nkl = countkerns(output);
	if (nkl > 0) {
		fprintf(output, "StartKernData\n");
		fprintf(output, "StartKernPairs %d\n", nkl); /* check count here ! */
		nklnew = 0;				/* to check the above count - debugging */
		if (traceflag != 0) printf("\nCreating KernPairs:\n");
		for (k = bc; k <= ec; k++) {
			charinfo = readlong(charstart + k - bc);
			if (charinfo == 0) continue;
/*			if (verboseflag != 0) (void) _getch(); */
			windex = (int) (charinfo >> 24) & 255;
			hindex = (int) (charinfo >> 20) & 15;
			dindex = (int) (charinfo >> 16) & 15;
			iindex = (int) (charinfo >> 10) & 63;
			tag = (int) charinfo >> 8 & 3;
			remain = (int) charinfo & 255;
/*			if (traceflag != 0)
				printf("chr %d w %d h %d d %d i %d t %d r %d\t(%lo)\n",
					k, windex, hindex, dindex, iindex, tag, remain, charinfo);*/
/*			we have already seen the above output ? */ /* 1993/June/24 */
/*			charwidth = unmapread(widthstart + windex);
			charheight = unmapread(heightstart + hindex);		
			chardepth = unmapread(depthstart + dindex);
			charitalic = unmapread(italicstart + iindex); */

			if (tag != 0) {
				if (tag != 1) {
/* 			fprintf(errout, "Don't know what to do with tag	%d\n", tag); */
					continue;
				}
				liginx = ligkernstart + remain;
				ligkern = readlong(liginx);

				skipbyte = (int) ((ligkern >> 24) & 255);
				if (skipbyte > 128) {	/* first word in lig-kern program */
					texthree++;				/* already done ? */
					opbyte = (int) ((ligkern >> 8) & 255);
					remainder = (int) (ligkern & 255); 
					liginx = (opbyte << 8) + remainder;
/*					if (verboseflag != 0) printf("liginx %d ", liginx); */
					if (liginx >= nl) {
						fprintf(stderr, "Impossible ligkern index %d (> %d)\n", 
							liginx, nl);
						exit(7);
					}
					if (traceflag != 0) 
					printf("TeX 3.0: char %d skipbyte %d > 128 (liginx %d)\n", 
						k, skipbyte, liginx);
					liginx = liginx + ligkernstart; /* NEW */
					ligkern = readlong(liginx);	
				} /* dealt with indirect pointers */

				if (strcmp(encoding[k], "") == 0) {
					fprintf(errout, 
						"ERROR: First char (%d) in kern is not in encoding\n",	k);
					badencoding++;
				}
				for (;;) {
					if (diareaflag != 0) 
						printf("liginx %d ligkern %ld\n", liginx, ligkern); 
					opbyte = (int) ((ligkern >> 8) & 255); 
					remainder = (int) (ligkern & 255); 
					if ((opbyte & KERNBIT) != 0) {
/*					if (((ligkern >> 8) & KERNBIT) != 0) { */
						nextchar = (int) (ligkern >> 16) & 255;
/*						kerninx = (int) ligkern & 255; old */
/*	allow for more than 255 kern pairs - TeX 3.0 - usually opbyte = 0 */
						kerninx = ((opbyte & 127) << 8) | remainder; 
						kernam = unmapread(kernstart + kerninx);
						if (diareaflag != 0) {
							printf("this %d next %d kern %d\n", 
								k, nextchar, kerninx);
							fprintf(errout, "Press any key to continue . . .\n");
							fflush(stdout);
							(void) _getch();
						}
						if (strcmp(encoding[nextchar], "") == 0) {
							fprintf(errout, 
								"ERROR: Next char (%d) in kern is not in encoding\n",
									nextchar);
							badencoding++;
						}
						if (showkernumbers != 0)
							fprintf(output, "KPX %s %s %lg %% %d %d\n", 
								encoding[k], encoding[nextchar],
									kernam, k, nextchar);
						else {
/*							fprintf(output, "KPX %s %s %lg\n", 
								encoding[k], encoding[nextchar], kernam); */
/* following expanded out 1994/June/20 */
							fputs("KPX ", output);
							if (strcmp(encoding[k], "") != 0) 
								fputs(encoding[k], output);
							else fprintf(output, "a%d", k);
							putc (' ', output);
							if (strcmp(encoding[nextchar], "") != 0) 
								fputs(encoding[nextchar], output);
							else fprintf(output, "a%d", nextchar);
							fprintf(output, " %lg\n", kernam);
						}
						if (k == kold && nextchar == nextold) {
							if (descrlength == 0)
								fprintf(errout,
								"WARNING: repeat kern pair KPX %s %s %lg\n", 
									encoding[k], encoding[nextchar], kernam);
						}
/* NOTE: this only notices obvious repetitions ... */
						kold = k;
						nextold=nextchar;
						nklnew++;
					}
					if (((ligkern >> 24) & STOPBIT) != 0) break;
					liginx++;
					ligkern = readlong(liginx);	
				}
			}
		}
		fprintf(output, "EndKernPairs\n");
		fprintf(output, "EndKernData\n");
/*		assert (nklnew == nkl); */
	}
	else if (fixedpitchflag == 0)		/* (OK for fixed pitch) */
/*			printf("WARNING: No kerning data (%d)\n", nkl); */
			printf("WARNING: No kerning data\n");

/* next come composites StartComposites <n> ... */
/* then EndComposites */
	fprintf(output, "EndFontMetrics\n");
	if (badencoding > 0) {
		fprintf(errout, 
			"WARNING: Encoding `%s' probably not correct - use `c' argument\n",
				codingvector);
		fprintf(errout,
		"         CheckSum suggests `%s..' encoding vector\n",
				checksumvector);
	}
	if (verboseflag != 0) {
		if (texthree != 0) printf("TFM file uses some TeX 3.0 features\n");
		printf("Processed %d characters ", totalcount - needspace);
		if (needspace != 0)	printf("(+ space)");
		putc('\n', stdout);
	}
}

/****************************************************************************/

/* how many non-empty character names are there 
int checkhowmany(void) { 
	int k, n=0;
	for (k=0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") != 0) n++;
	}
	return n;
} */

void readtfm (FILE *input) {
	int c;
/*	char *s = buffer; */
/*	unsigned char *s=buffer; */
	unsigned char __far *s;
	long length;

	fseek(input, 0, SEEK_END);
	length = ftell(input);					/* get file length */
	fseek(input, 0, SEEK_SET);				/* rewind to start */

#ifndef NT
	if (length > 65535) {
		fprintf(stderr,
		"ERROR: Supposed TFM file seems ridiculously large (%ld > 65535)\n",
				length);
		exit(1);
	}
#endif
	if (buffer != NULL) {
		fprintf(errout, "Allocation error\n");
		_ffree(buffer);
		buffer = NULL;		
	}
	buffer = (unsigned char __far *) _fmalloc((unsigned int) length);
	if (buffer == NULL) {
		fprintf(stderr,
			"ERROR: Unable to allocate memory for TFM file (%ld)\n", length);
		exit(1);
	}
	s = buffer;

	while ((c = getc(input)) != EOF) {
		*s++ = (unsigned char) c;
/*		if (s >= buffer + MAXBUFFER-1) { */	/* 1993/May/31 */
		if (s > buffer + length) {		/* 1993/Nov/7 */
/*			fprintf(errout, "TFM file too large (%d > %d)\n", 
				(s - buffer), length-1); */
			fprintf(stderr,
				"IMPOSSIBLE ERROR: TFM file overrun (> %d) char %d\n", 
					(s - buffer), c);
			exit(1);
		}
	}
}

/* \tt: trace mode\n\ */

void showusage (char *s) {
	fprintf (stderr, 
		"Correct usage is:\n%s [-{v}{k}{a}{r}{u}{i}{t}] [-c=<coding>]\n\
\t<tfm-file-1> <tfm-file-2>...\n\
", s);
	if (detailflag == 0) exit(1);
	fprintf (stderr, "\n\
\tv: verbose mode\n\
\tk: TeX CM font TFM file => a, r, u\n\
\t\ta: adjust coordinates (for scale error in some TFM files)\n\
\t\tr: rationalize coordinates (for errors in some TFM files)\n\
\t\tu: convert font name to upper case (for TeX CM TFM files)\n\
\ti: show italic corrections separately in AFM\n\
\tt: trace mode - show even more details!\n\
\tj: scale coordinates by multiplying by given amount\n\
\to: offset coordinates by given amount in scaled points\n\
\ts: show all quantities in scaled coordinates\n\
\tc: use specified encoding vector for decoding character codes\n\
\t   (default is to guess encoding from codingscheme field in file)\n");
/*	fprintf (stderr, "\n\
\t   NOTE: information in AFM file may be incomplete\n\
); */
	exit(1);
}

/* and to use `%s' encoding if that fails)\n\ */

/*  TeX text (`%s') encoding)\n\ */

/* \tZ: italic fuzz (default 30)\n */

/* \te: next arg is extension for output file (default '%s')\n\ */

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': if (verboseflag != 0) showcruft = 1; 
						else verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0; 
		case 'k': texcmflag = 1; return 0;
		case 'a': offsetflag = 1; return 0;
/*		case 'a': scaleflag = 1; return 0; */
		case 's': showactual = 1; return 0;
		case 'r': rationalize = 1; return 0;
		case 'u': uppercaseflag = 1; return 0;
		case 'i': showitalic = 1; return 0;
		case 'f': usefilename = 1; return 0;	/* 1993/Feb/9 */
		case 'z': showansitex = 1; return 0;	/* 1993/Dec/18 */
		case '1': ansitexflag = 0; return 0;	/* 1993/Dec/18 */
/*		case 'E': errout = stdout; return 0; */	/* 1996/Mar/3 */
		case 'E': errout = stderr; return 0;	/* 1997/Sep/23 */
		case 'd': descrlength = 1; return 0;	/* 1997/July/24 */
/*		the rest take arguments */
		case 'j': scaleargflag = 1; return 1;		/* 1997/Jun/20 */
  		case 'o': offsetargflag = 1; return 1;		/* 1997/Jun/20 */
		case 'c': vectorflag = 1; return 1;
		case 'Z': italicfuzzflag = 1; return 1;	/* 1994/Nov/1 */
/*		case 'e': extenflag = 1; return 1;   */
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
/*	return 0; */		/* ??? */
}

/* remove file name - keep only path - inserts '\0' to terminate */
void stripname (char *pathname) {
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

void stampit (FILE *outfile) {
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
	fflush(stdout);
	(void) _getch(); 
	return hash;
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

/*	if (argc < 2) showusage(argv[0]); */
	if (firstarg + 1 > argc) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command flags */
		if (firstarg + 1 > argc) break;
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (vectorflag != 0) {
/*					strncpy(codingvector, s, 40);  */
/*					strcpy(codingvector, s);  */
					commandvector = s;
					vectorflag = 0; reencodeflag = 1; 
				}
				else if (scaleargflag != 0) {
					if (sscanf(s, "%lg", &scale) < 1)
						fprintf(errout, "Don't understand scale %s\n", s);
					scaleflag = 1;
					scaleargflag = 0;
				}
				else if (offsetargflag != 0) {
					if (sscanf(s, "%lg", &offset) < 1)
						fprintf(errout, "Don't understand offset %s\n", s);
					offsetflag = 1;
					offsetargflag = 0;
				}
				else if (italicfuzzflag != 0) {
					if (sscanf(s, "%lg", &italicfuzz) < 1)
						fprintf(errout, "Don't understand italic fuzz %s\n", s);
					italicfuzzflag = 0;
				}
/*				else if (extenflag != 0) {
					ext = s; firstarg++;
					extenflag = 0; 
				} */
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

char *zstrdup (char *s) {
	char *new = _strdup(s);
	if (new != NULL) return new;
	printf("ERROR: Unable to allocate memory for %s\n", s);
	exit(1);
}

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindoini = "dviwindo.ini";		/* name of ini file we look for */

int dviwindoinisetup = 0;		/* turned on after setup */

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
	if ((windir = getenv("windir")) != NULL ||	/* 94/Jan/22 */
		(windir = getenv("WINDIR")) != NULL ||
		(windir = getenv("winbootdir")) != NULL ||
		(windir = getenv("SystemRoot")) != NULL ||
		(windir = getenv("SYSTEMROOT")) != NULL) { /* 95/Jun/23 */
		strcpy(fullfilename, windir);
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);
	}
	else _searchenv (ininame, "PATH", fullfilename);

	if (strcmp(fullfilename, "") == 0) {		/* ugh, try standard place */
		strcpy(fullfilename, "c:\\windows");	/* winnt ??? */
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
					return zstrdup(fullfilename);
				}
			}					
			fclose(input);
		}
/*	} */
	return "";							/* failed, for one reason or another */
}

int setupdviwindo (void) {
	if (! usedviwindo) return 0;		/* already tried and failed */
	if (dviwindo != NULL && *dviwindo != '\0') return 1;	/* already tried and succeeded */
	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");
	if (dviwindo != NULL && *dviwindo != '\0')	{
		dviwindoinisetup = 1;
		return 1;
	}
	else {
		usedviwindo = 0; /* failed, don't try this again */
		return 0;
	}
//	return (*dviwindo != '\0');
}

char *grabenvvar (char *varname, char *inifile, char *section, int useini) {
	FILE *input;
	char line[MAXLINE];
	char *s;
	int m, n;

	if (! useini || *inifile == '\0')
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
							if (traceflag)  {	/* DEBUGGING ONLY */
								printf("%s=%s\n", varname, line+n+1);
							}
							return zstrdup(line+n+1);
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
	if (usedviwindo && ! dviwindoinisetup) setupdviwindo();		/* 99/June/16 */
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
	if (! useatmini) return 0;		/* already tried and failed */
	if (*atmini != '\0') return 1;		/* already tried and succeeded */
/*	atmini = setupinifile ("atm.ini", "[Setup]");  */
	atmini = setupinifile (atmininame, atmsection);
	if (atmini != NULL && *atmini != '\0') {
		return 1;
	}
	else {
		useatmini = 0;	/* failed, don't try this again */
		return 0;
	}
//	return (*atmini != '\0');
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	FILE *fp_in, *fp_out;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	char *s;
/* 	unsigned int i; */
/*	int c; */
	int m, firstarg=1;
	
	strncpy(programpath, argv[0], sizeof(programpath));
	stripname(programpath);
/*	printf("Default program path is %s\n", programpath); */
/*	if programpath exists, use as default for vec */
/*	if (strcmp(programpath, "") != 0) vectorpath = programpath; */
	
	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("VECPATH")) != NULL) vectorpath = s;
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo();  /* need to do this before use */
		if ((s = grabenv("VECPATH")) != NULL) vectorpath = s;
	}

	strcpy(codingscheme, "");
	strcpy(fontid, ""); 

/*	setcmrencoding(); */				/* default encoding */
/*	strcpy(codingvector, "textext"); */	/* ? */

	setupnumeric();						/* 97/Feb/12 */
	strcpy(codingvector, "numeric"); /* ? */

/*	if (argc < 2) showusage(argv[0]); */
	
	firstarg = commandline(argc, argv, 1);

	if (texcmflag != 0) {
/*		tweakflag = 1; rationalize = 1; uppercaseflag = 1; */
		offsetflag = 1; rationalize = 1; uppercaseflag = 1;
	}

	if (firstarg + 1 > argc) showusage(argv[0]);

/*	scivilize(compiledate);	 */
	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);
	printf("\n");

	if (traceflag) printf("reencodeflag = %d, codingvector = %s\n",
						    reencodeflag, commandvector);

	if (reencodeflag != 0) { /* redundant ? done again later ? */
		if (verboseflag != 0) 
			printf("Using `%s' encoding vector\n", commandvector);
		(void) readencoding(commandvector, 0);
	}

	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASHED %lu", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	if (firstarg + 1 > argc) exit(1);

	for (m = firstarg; m < argc; m++) {
/*		may need to reinitialize things here for next file */
		fxll=0.0; fyll=0.0; fxur=1000.0; fyur=1000.0;
/*		nchrs=0; nkern=0;  */
/*		fchrs=0; lnext=0; knext=0; */

/*		default values */
		checksum=0;
		designsize = 10.0;
		italicangle = 0.0;
		spacewidth = 333.333;
		spacestretch = 166.667;
		spaceshrink = 111.111;
		xheight = 500.0;
		emquad = 1000.0;
		extraspace = 111.111;
		texthree=0;
		badencoding=0;
		rightboundary=-1;						/* 97/Oct/1 */
		leftprogram=-1;							/* 97/Oct/1 */
		strcpy(codingvector, commandvector);	/* 96/Aug/3 */
		if (traceflag) printf("codingvector %s %d %s\n", codingvector, m, argv[m]);

		extencount = 0;  ascendcount = 0;

		strcpy(fn_in, argv[m]);

		if (strstr(fn_in, "afm") != NULL ||
			strstr(fn_in, "AFM") != NULL) {		/* not afm file input */
/* the following really is for afmtotfm conversion - use other program */
			fprintf(stderr, "Use AFMTOTFM instead!\n");
			exit(17);
		}

		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, '/')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
		strcpy(fontname, s);  
/*		strcpy(fontname, fn_in); */
		if ((s = strchr(fontname, '.')) != NULL) *s = '\0';
		
		extension(fn_in, "tfm");
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {
			fprintf(stderr, "ERROR: Can't open input file ");
			perror(fn_in);
			exit(5);
		}
/*			if (verboseflag != 0)  */
			printf("Processing %s\n", fn_in);

/*		write output in current directory */
			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, '/')) != NULL) s++;
			else if ((s=strrchr(fn_in, ':')) != NULL) s++;
			else s = fn_in;
			strcpy(fn_out, s);  

/*			forceexten(fn_out, ext); */
			forceexten(fn_out, "afm");
			if ((fp_out = fopen(fn_out, "w")) == NULL) {
				fprintf(stderr, "ERROR: Can't open output file ");
				perror(fn_out);
				exit(5);
			}
			if (verboseflag != 0 && showcruft != 0) 
				printf("File %s is open\n", fn_out);

/*			setdefaultencoding(); */
/*			setcmrencoding(); */

			readtfm (fp_in);

			if (ferror(fp_in) != 0) {
				fprintf(stderr, "Error in input file ");
				perror(fn_in);
				exit(5);
			}
			else fclose(fp_in);

			tfmanalyze(fp_out);
				
			if (buffer != NULL) {		/* 97/Feb/12 */
				_ffree(buffer);
				buffer = NULL;
			}
			if (ferror(fp_out) != 0) {
				fprintf(stderr, "Error in output file ");
				perror(fn_out);
				exit(5);
			}
			else fclose(fp_out);
			printf("\n");
	} 

	if (argc - firstarg > 1) {
		if (verboseflag) printf("Processed %d TFM files\n", argc - firstarg);
	}

#ifdef _WIN32
	if ((m = _heapchk ()) != _HEAPOK) {		/* 94/Feb/18 */
		fprintf(stderr, "ERROR: heap corrupted (%d)\n", m);
/*		exit(1); */
		exit(1);
	}
#else
	if ((m = _fheapchk ()) != _HEAPOK) {		/* 1994/Feb/18 */
		fprintf(stderr, "WARNING: Far heap corrupted (%d)\n", m);
		exit(1);
	}
#endif

/*	if (buffer != NULL) {
		_ffree(buffer);
		buffer = NULL;
	} */
	return 0;
}

/* check IsFixedPitch both from tfm and from afm */

/* StartCharMetrics count of ec - bc + 1 is bogus - includes missing OK */

/* possibly try and do better with depth/height/italic to match actual */

/* not dealing with extra parameters for math fonts - OK */
/* not dealing with extensible character programs   - OK */
/* need command line choice of what encoding to use OK */
/* need to be able to disable snap to of scaled point quantities ? OK */

/* ignore all but first two words of header - */
/* use header info to pick up encoding scheme and fontid */
/* could construct fontname from header info and design size */
/* read in encoding vector from file */
/* could pick encoding vector based on header info */

/* use tweak flag in other direction also ? AFMTOTFM - NO */

/* zero width index means not a valid character <= says the book */
/* but: characters 54 and 55 in cmsy* and cmbsy* have zero width index */

/* Reconstruct FontBBox from individual; BBoxes ? */
/* Estimate 0 -maxdepth maxwidth maxheight */

/* Seems to miss out placing "EndCharMetrics" at times ? */
/* particularly for math fonts cmex10, cmmi*, cmsy* */

/* emquad and space info in comments should also be `rationalized' */
/* the same way information in character lines is */

/* If you change names of extra stuff in AFM file here - */
/* - better change it in AFMTOTFM also! */

/* for TeX want font name upper case ? */

/* Comment Ascender and Comment Extensible come out on stdout */
/* - gather these up in log file and integrate with rest of AFM */
/* could make separate pass for each - but is it worth it - cmex10 only */
/* so it spews out this stuff even in non-verbose mode for cmex10 */

/* (xur - width + italicfuzz) in BBox here used to encode italic correction */

/* Decode facebyte header[17] ? */
/* medium/regular bold/heavy light */
/* roman/regular italic/slanted/oblique */
/* regular condensed extended */
/* plus seven-bit-safe-flag */

/* allow for new format (TeX 3.0) TFM files ? */

/* Does NOT provide the following: */
/* FullName, FamilyName, Weight, Copyright, Notice, CreationDate ? */

/* IsFixedPitch if only ONE character has wrong width ? As in CM */

/* allocation/freeing once per file, not once per job */

/* use `-1' on command line to disallow/complain ANSI/TeXText extras */

/* use `-z' on command line to show the ANSI/TeXText extras */
/* normally these are suppressed to avoid encouraging user to use them */

/* WARNING: *MUST* COMPILE WITH OPTIMIZATIONS ! */

/* If the very first instruction of a character's |lig_kern| program has */
/* |skip_byte>128|, the program actually begins in location */
/* |256*op_byte+remainder|. This feature allows access to large |lig_kern| */
/* arrays, because the first instruction must otherwise */
/* appear in a location |<=255|. */

/* We now implement `right boundary character' ligkern entry 97/Oct/1 */
/* If the very first instruction of the |lig_kern| array has |skip_byte=255|, */
/* the |next_char| byte is the so-called right boundary character of this font; */
/* the value of |next_char| need not lie between |bc| and~|ec|.*/

/* We do not implement `special left boundary character program' */
/* If the very last instruction of the |lig_kern| array has |skip_byte=255|, */
/* there is a special ligature/kerning program for a left boundary character, */
/* beginning at location |256*op_byte+remainder|. */

/* The interpretation is that \TeX\ puts implicit boundary characters */
/* before and after each consecutive string of characters from the same font. */
/* These implicit characters do not appear in the output, but they can affect */
/* ligatures and kerning. */

/* EC font extra dimensions 
def font_cap_height expr x = fontdimen 8: x enddef;
def font_asc_height expr x = fontdimen 9: x enddef;
def font_acc_cap_height expr x = fontdimen 10: x enddef;
def font_desc_depth expr x = fontdimen 11: x enddef;
def font_max_height expr x = fontdimen 12: x enddef;
def font_max_depth  expr x = fontdimen 13: x enddef;
def font_digit_width expr x = fontdimen 14: x enddef;
def font_cap_stem expr x = fontdimen 15: x enddef;
def font_baselineskip  expr x = fontdimen 16: x enddef;
*/

/* tfmtoafm -v -j=0.9999965 d:\dc\cmr10 */
/* tfmtoafm -v -j=1.0000050 d:\dc\cmbx10 */
/* tfmtoafm -v -j=1.0000055 d:\dc\cmti10 */

/* tfmtoafm -v -j=1.000244141 d:\em\emr10 */

/* depth table may have negative entries */
/* kern table need not be sorted and zeroth entry need not be zero */

/* Assumes left boundary character ligkern program is that of named char */
