/* Copyright 1990, 1991, 1999, 2000 Y&Y, Inc.
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

/****************************************************************************
* Read outline font file, analyze font information -						*
* and recode result in Adobe Type 1 Font format
*                                                                         	*
* Command line usage:	----see showusage---								*
* Can handle multiple files on command line, as well as wild cards.			*
* If output has same name as input, uses temp file and renames at end.     	*
* User can interrupt processing by typing control-C or control-break		*
****************************************************************************/

/* The following are for function prototypes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <io.h>						/* for mktemp ? */

#pragma warning(disable:4032)	// different type when promoted
#include <conio.h>
#pragma warning(default:4032)	// different type when promoted

#include <time.h>					/* for CreationDate */
#include <math.h>					/* only for sqrt */

#include <assert.h>
#include <signal.h>
#include <malloc.h>

#ifdef _WIN32
#define __far
#define _fmalloc malloc
#define _ffree free
#define  _fheapchk heapchk
#endif

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *programversion = "version 2.4";	/* program version */

char *copyright = "Copyright (C) 1990--2000, Y&Y, Inc.\
All rights reserved";				/* program copyright */

#define AUTHOR "Y&Y "				/* author's initials */

/* #define FNAMELEN 80 */
#define FONTNAME_MAX 80		/* maximum font name length */
#define NUMBERSTR_MAX 40
#define CHARNAME_MAX 80
#define MAXLINE 128			/* maximum line length assumed in PS input */
#define MAXINLINE 300		/* maximum line length assumed in hint input */
#define MAXCHRS 256			/* Number of characters defined in fonts */
#define TEXCHRS 256			/* Number of characters in TeX fonts */
#define MAXCHARNAME 32		/* (22) maximum characters in character name */
#define MAXFONTNAME 32		/* maximum font name length */

#define ZEROS 512			/* need 512 zeros at end of font definition */
#define ZEROSPERLINE 64		/* pack 64 zeros per line */

#define MAXSUBRS 512		/* maximum number of subrs allowed */

#define MAXSCLNUM 2000000 		/* maximum to use in scale num and den */ 

/* #define MAXSCLNUM 32767 */	/* maximum to use in scale num and den */ 

FILE *errout=stdout;

/* stuff for FontInfo directory - obtained from AFM file if possible */

static char fullname[MAXLINE];				/* Full name of font */
static char familyname[FONTNAME_MAX];		/* "Computer Modern" */
static char version[MAXCHARNAME];			/* "000.009" */
static char notice1[MAXLINE];				/* enclosed notice */
static char notice2[MAXLINE];				/* enclosed notice */
static char weight[MAXCHARNAME];			/* Weight of font */

double italicangle=0.0;
int isfixedpitch;				
int underlineposition;
int underlinethickness;

static char fontname[MAXFONTNAME];			/* font presently working on */
int ptsize;									/* point size from fontname */

/* char line[MAXLINE]; */					/* input line 'buffer' */
/* char line[MAXLINE+MAXLINE]; */			/* input line 'buffer' 94/June/2 */
char line[MAXINLINE];						/* input line 'buffer' 95/Dec/10 */

unsigned char chardone[MAXCHRS];	/* character outline has been processed */

unsigned char subrchar[MAXSUBRS];	/* which char a given subr belongs to */

unsigned char subrcode[MAXSUBRS];	/* which subr call within a character */

unsigned char subrused[MAXSUBRS];	/* flags subr has been called */

int texfont=0;				/* non-zero => this is a TeX font */

int wantuniqueid=1;			/* put uniqueID in font if possible */
int numsubrs=0;				/* number of Subrs */ 
int wantothersubrs=0;		/* set iff OtherSubrs needed */

int fatal = 0;				/* fatal errors flag */
int ignorefatal = 0;		/* ignore fatal errors flag */

int statisticsflag = 0;			/* output statistical information at end */

int padinfo = 1;			/* offset FontInfo lines by one space */

/* #define DEBUG 1 */		/* careful, debugging mode */
/* #define DEBUGFLUSH 1 */	/* flush output buffers */
/* #define WANTBACKUP 1 */	/* backup of output file if exists */
/* #define REORDER 1 */		/* start path at lineto, if possible */

#define SNAPTOWIDTH 1		/* want widths snapped to expected quanta */
#define USEAFMENCODE 1		/* use Encoding in AFM file if specified */

#define FLUSHCR 1			/* don't output C-M before C-J */
							/* (C-M taken care of by text I/O functions) */
#define CONTROLBREAK 1		/* allow user interrupt */

int autocloseflag=1;		/* use closepath to close if lineto is last */
int reorderflag=1;			/* start path at lineto, minimize length */

int subrindex;				/* highest numbered subr so far */
int hintsubrs;				/* no of hint replace subr calls in hint file */
int nsubrs;					/* no of hint replace subr calls within char */

int fontchrs = MAXCHRS;		/* size of encoding vector needed */

int reverseflag=0;			/* reverse subpaths */
int spikeflag=1;			/* check for and correct spikes */
int insideonly=0;			/* fix only inside spikes */

int userdictflag=0;			/* put RD, ND, NP in userdict instead Private */
							/* most recent Adobe fonts don't do this */

int usehexbbox=0;			/* use BBox from .hex file (not from .afm) */
int useafmbbox=1;			/* use BBox from .afm file (not outlines) */

int showcharbbox=0;			/* show character BBox in AFM format */

int suppresscontrol=0;		/* do not include control characters encoding */
							/* default of above changed 1993/June/20 */
int checkhexwidths=0;		/* check if hex file widths are reasonable */
int unencodeflag=0;			/* insert unencoded characters control pos */
int uppercaseflag=0;		/* make font name upper case */
int numcodchrs=0;			/* use numbers for CharStrings names, not names */
int remapflag=0;			/* remap elements of Encoding vector */
int use128flag=0;			/* use 128 for duplicate remap of 32 */
int remapfirst=1;			/* non-zero remap 32 => 195 & 127 => 196 first */
int notdefflag=1;			/* include /.notdef CharString */
int spaceflag=0;			/* include /space CharString */
int dcaddflag=0;			/* turns on nbspaceflag, cwmflag, sfthyphenflag */
int nbspaceflag=0;			/* include /nbspace CharString */
int cwmflag=0;				/* include /cwmspace CharString */
int sfthyphenflag=0;		/* include /sfthyphen CharString */
int psadobeflag=1;			/* non-zero means use PS-AdobeFont-1.0 */
							/* for MS Windows PS driver compatability */
int suppressvstem3=0;		/* use 3 vstems instead of one vstem3 */
int suppresshstem3=0;		/* use 3 hstems instead of one hstem3 */
double spacewidth;			/* non-negative => space width from AFM file */

int fakezone=1;				/* fake one alignment zone if BlueValues missing */
int noafmflag=0;			/* if no AFM file found - use HEX info only */

int switchtrick=1;			/* if using new way to switch hints */
int subroffset;				/* 4 <= how many subrs used up before first hint */

int ndgapflag=0;			/* insert extra space before ND */
int originflag=0;			/* == 0 => my own fonts */
							/* == 1 => BSR (CM) fonts */
							/* == 2 => PS (AT&T) fonts */
							/* == 3 => B & H fonts */
							/* == 4 => AMS Euler fonts */
							/* == 5 => MathTime fonts */
							/* == 6 => Wolfram fonts */
							/* == 7 => MathTime Plus fonts */
							/* otherwise, no copyright line */
/* also affects time capsule storage ... */
							/* == 0 => Y&Y capsule */
							/* == 1 => BSR CM capsule */
							/* == 2 => PS nontex capsule */
							/* == 3 => B & H capsule */
							/* == 4 => AMS Euler capsule */
							/* == 5 => MathTime capsule */
							/* == 6 => Wolfram capsule */
							/* > 4 => random starting bytes */

int atmflag=1;				/* make ATM compatible - default now !!! */
int wantcomposites=1;		/* non-zero means build composite characters */
int adjuststem3=1;			/* adjust non-matching hstem3, vstem3 */
/* int usestandard=1; */	/* use ASE codes in SEAC calls for composites */
int relaxseac=0;			/* allows SEAC calls chars not in ASE 96/June/30 */

int sfthyphenhit=0;			/* if asked and was able to create sfthyphen */

int widthscaleflag=0;		/* scale widths also if non-zero scale 97/Aug/27 */

/* following are mutually exlusive */

int romanflag;				/* non-zero for roman font */
int	boldflag;				/* non-zero for bold font */
int	italicflag;				/* non-zero for italic font */
int	mathitalic;				/* non-zero for math italic */
int	mathsymbol;				/* non-zero for math symbol */
int	mathextend;				/* non-zero for math extended */

int forceboldseen;			/* non-zero if ForceBold in hint file */
int needcountercontrol;		/* non-zero if StmRndUp in hint file */

int	typeflag;				/* non-zero typewriter font */

/* int sansflag; */			/* non-zero for sans-serif font */ /* NA */

char *ext = "pfa";
/*	int extenflag=0;	*/	/* non-zero => next arg is extension for output */

char *opath = "";		/* output path "" => current directory */
/*	int opathflag=0; */		/* non-zero => next arg path for output files */

char *wpath = "";		/* width file path */
int wpathflag=0;		/* non-zero => next arg is path for width files */

char *hpath = "";		/* hinting file path */
int hpathflag=0;		/* non-zero => next arg is path for hint files */

char hexpath[FILENAME_MAX];		/* path to HEX file */	/* 1992/Aug/31 */

int hintsflag=0;		/* non-zero => character hints available */
int widthflag=0;		/* non-zero => correct widths have been read */
int correctscale=0;		/* non-zero => undo BSR rescaling */
int correctfontscale;	/* copy of above for current font */
int dontrescale=0;		/* non-zero =>  use input coordinates as is */ 
int largescale=1;		/* power of two for large fonts */

/*	int download=0; */		/* non-zero => add downloading header to file */

int columns=78;			/* 128 ? maximum line length for output */
							/* 78 ? max line length in unencoded form */

/* following old stuff no longer supported - define OLDSTUFF */
int relatflag=1;		/* use implicit Type 1 BuildChar commands */
							/* if not, use absolute PS commands */
int charencode=1;		/* non-zero means encode charstring */
							/* if not encoded file, need to encode in PS */
int charencrypt=1;		/* non-zero means encrypt charstrings  */
							/* if not encrypted file, need to encrypt in PS */
/* above old stuff no longer supported - define OLDSTUFF */

int charbinflag=1;		/* non-zero means write charstring in binary */
							/* zero means write charstring in hexcode */

int randomchar=-1;		/* positive start charstring with random bytes */
							/* zero => start with zero bytes */
							/* negative => bury time capsule here */
int extrachar=4; 		/* random bytes start of charstring - default 4 */
							/* MUST be 4 for version 21.0 of PS interpretor */
							/* otherwise can be 0 or 1 to save space */

int eexecbinflag=0;		/* non-zero means write eexec in binary format */
							/* zero means write in hex format */
/* setting above non-zero does NOT quite produce Adobe downloader format... */
int eexecencrypt=1;		/* non-zero means encrypt eexec */
#define EXTRAEEXEC 4 		/* random bytes start of eexec - MUST BE 4 */
/* NOTE: EXTRAEEXEC must always be 4, although RANDOMEEXEC may be 0 */
/* NOTE: EXTRAEEXEC must always be 4, although EXTRACHAR may be 0 or 1 */

int executeonly=1;		/* make silly things executeonly */
int conform=0;			/* include RD code even if not used */
int noaccess=1;			/* make it 'hard' to spy on innards */

double widtheps=1.0;	/* tolerance on HEX versus AFM width mismatch */

/* #define MAXKNOTS 350 */	/* maximum number of knots in subpath */
/* #define MAXKNOTS 512 */	/* maximum number of knots in subpath */
#define MAXKNOTS 1024		/* maximum number of knots in subpath */

/* Longest subpath in AT&T files has 337 knots */
/* #define MAXKNOTS 175	*/
/* Longest subpath in 40 plain TeX Computer Modern fonts has 148 knots */

/* Following changed 1992/August/29 - no longer needed since NUMLIM changed */
/* #define ONEBYTELIM 1 */	/* means use one bytes for numerator */
							/* otherwise use two byte code for numerator */

/* following reduced 1992/March/17 to cope with LaserMaster 800/4 problem */

/* #define NUMLIM 2000000 */	/* maximum numerator in hsbw div arguments */
#define NUMLIM 32767		/* maximum numerator in hsbw div arguments */

#ifdef ONEBYTELIM
#define DENLIM 107 			/* one byte encoding limit on denominator */
#else
#define DENLIM 1131			/* two byte encoding limit on denominator */
#endif

/* #define CHARSTRINGLEN 500 */ /* normal max space needed for TeX */
/* #define CHARSTRINGLEN 1000 */ /* normal max space needed for AT&T */
/* #define CHARSTRINGLEN 2000 */ /* max space needed for charstring */
#define CHARSTRINGLEN 4096	    /* max space needed for charstring */
							    /* need lots for open coding */

/* In most TeX fonts longest charpath character 15 (ffl) with < 391 bytes */
/* In TeX symbol fonts, longest charpath character 60 (Real) with 313 bytes */

#define CRYPT_MUL 52845u	/* pseudo-random number generator multiplier */
#define CRYPT_ADD 22719u	/* pseudo-random number generator addend */
#define REEXEC 55665 		/* seed constant for eexec encryption */
#define RCHARSTRING 4330 	/* seed constant for charstring encryption */

#define UNIQUEIDSTART 5000761	/* for CM fonts */
/* Adobe UniqueID coordinator:  Terry O'Donnell 415-962-3836 */

#define MINLINETO 1			/* 3 shortest rlineto permitted */
#define MINCURVETO 3		/* 5 shortest rcurveto permitted */

/*
#define ABS(x) (((x) < 0) ? (-x) : (x))
#define MAX(x, y) (((x) < (y)) ? (y) : (x))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
*/

/* This code makes certain assumptions about BSR CM version 0.8 fonts: 		*/
/*   Both Font Matrix and Font Bounding Box come before character outlines  */
/*	 The characters are ordered on character code							*/
/*   There are 128 character outlines in the font							*/

/* We scale on input to convert to standard 0.001 scale in FontMatrix */
/*			(unless dontrescale is non-zero) */
/* So need to check that BBox does not go outside -2000 to +2000 */
/* Although this apparently is NOT really a problem! */

int onepass = 0;				/* non-zero => single pass operation */
								/* dont gather sbx & bbox on first pass */
int verboseflag = 0;			/* lots of interesting output */	
int traceflag = 0;				/* more verbage */
int detailflag = 0;

/* the following may not really need to be long rather than int... */
long numerscale;				/* numerator of scale factor */
long denomscale;				/* denominator of scale factor */
long numerold;					/* numerator of scale factor - not reset */
long denomold;					/* denominator of scale factor - not reset */

double rawnumer, rawdenom;		/* raw numer and denom in font matrix */

int forceothersubrs=1;			/* force in OtherSubrs for image setter */
								/* font downloading software bug work around */
int forcesubrs=1;				/* add dummy /Subrs entry  */
int forceboiler=1;				/* put in standard Subrs, even if no hints */
int avoiddiv=0;					/* round widths to integers for NewGen */
int avoidlargescale=0;			/* rescale when BBox too large */
double adobescale=0.001;		/* default output scale */

int clm;						/* current output column */

static double widths[MAXCHRS];	/* character widths read from AFM file */
								/* straight, no scaling here */

static int sbx[MAXCHRS];		/* side bearings - obtained first pass */

struct knot {				/* structure for a knot */
	int x; int y; char code;
};

static struct knot knots[MAXKNOTS * 2];

int closed;						/* non-zero if closepath seen */

int xold, yold;					/* previous knot remembered */
int knot;						/* index into arrays for next knot */
int maxknts, maxkchrs;			/* maximum of above,  character it occured */
int maxebytes, maxechrs;		/* maximum bytes in charstring, character */
int avoided;					/* number of lineto's avoided */
int reordered;					/* subpaths rearranged to make lineto first */
int shortline;					/* number of short rlineto's */
int shortcurve;					/* number of short rcurveto's */
int tripleknots;				/* places where three knots in same place */
int flexprocs;					/* places where FlexProc may be useful */
int snapped;					/* number of character widths corrected */
int spikes;						/* number of spikes corrected */

int charcount;					/* number of characters processed */
int charseen;					/* count on first pass */
int ncomposed;					/* number of composite characters */
int ncdict;						/* size of CharString dict */

int xll, yll, xur, yur;			/* given bounding box - from FontBBox input */
int sxll, syll, sxur, syur;		/* scaled versions - for FontBBox output */
int xmin, ymin, xmax, ymax;		/* actual extremes of x and y for font */
int cxmin, cymin, cxmax, cymax;	/* actual extremes of x and y for character */

int mcount, lcount, ccount, hcount; /* counts of various commands seen */

int frezflag;					/* non-zero means don't use BBox from file */
int redoflag;					/* non-zero means another pass required */
int chrs;						/* character code working on currently */	
int unchrs;						/* character code for next unencoded one  */

#ifdef DEBUG
long minlsq;			/* shortest rlineto and character */
int minlchrs;
long mincsq;			/* shortest rcurveto and character */
int mincchrs;
#endif

static FILE *fp_in, *fp_out, *fp_hnt, *fp_afm;

static char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
static char fn_hnt[FILENAME_MAX], fn_afm[FILENAME_MAX];
#ifdef WANTBACKUP
static char fn_bak[FILENAME_MAX], 
#endif

int hintedflag;		/* zero until hints emitted for this CharString */

/* int fontlevel=0; */		/* number of font-level hint lines */ /* NA */

char *task;  					/* current activity - for error message */
long uniqueid=0;				/* unique ID constructed from file name */
								/* or found in AFM file */

unsigned short int cryptchar; 	/* current seed for charstring encryption */
unsigned short int cryptee; 	/* current seed for eexec encryption */

/* unsigned char charstring[CHARSTRINGLEN]; */	/*  assemble charstring */
/* unsigned char _far *charstring; */	/*  assemble charstring */
unsigned char __far *charstring;

/* unsigned char *charptr;	*/	/* pointer into above */
/* unsigned char _far *charptr; */ 	/* pointer into above */
unsigned char __far *charptr;

/* unsigned char crypstring[CHARSTRINGLEN]; */	/*  encrypted version */
/* unsigned char _far *crypstring; */ 	/*  encrypted version */
unsigned char __far *crypstring; 	/*  encrypted version */

/* unsigned char *crypptr; */ 	/* pointer into above */
/* unsigned char _far *crypptr; */ 	/* pointer into above */
unsigned char __far *crypptr; 	/* pointer into above */

/* The magic encoding vector used to allow across-job font caching (C) */
/* Now we use this as default OR read it from AFM file if specified */

static char encoding[MAXCHRS][MAXCHARNAME]; /* for encoding vector */

/*
static unsigned char *defaultencoding[] = { 
 "Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon",
 "Phi", "Psi", "Omega", "alpha", "beta", "gamma", "delta", "epsilon",
 "zeta", "eta", "theta", "iota", "kappa", "lambda", "mu", "nu",
 "xi", "pi", "rho", "sigma", "tau", "upsilon", "phi", "chi",
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
 "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "dieresis" };
 */

#define USESTANDARD

#ifdef USESTANDARD
/* needed only for finding accents and base in composites ??? */
/* static unsigned char *standardencoding[] = {  */
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
#else
/* we use the following because there is no space for StandardEncoding */
/* accents are at 192 to 207 */
#define NUMACCENTS 16
static char *standardaccents[] =
{ "", "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
 "dieresis", "", "ring", "cedilla", "", "hungarumlaut", "ogonek", "caron"};
#endif

/* asciitilde -> 126, asciicircum -> 94, hyphen -> 45 */

/* The seventy-five ComputerModern font names */

/* unsigned */	/* flush this, since UniqueID should be in AFM file ? */
char *cmfonts[] = {
	"b10", "bsy10", "bx5", "bx6", "bx7", "bx8", "bx9", "bx10", "bx12",
	"bxsl10", "bxti10", "csc10", "dunh10", "ex10", "ff10", "fi10",
	"fib8", "inch", "itt10", "mi5", "mi6", "mi7", "mi8", "mi9",
	"mi10", "mi12", "mib10", "r5", "r6", "r7", "r8", "r9", "r10",
	"r12", "r17", "sl8", "sl9", "sl10", "sl12", "sltt10", "ss8",
	"ss9", "ss10", "ss12", "ss17", "ssbx10", "ssdc10", "ssi8",
	"ssi9", "ssi10", "ssi12", "ssi17", "ssq8", "ssqi8", "sy5", "sy6",
	"sy7", "sy8", "sy9", "sy10", "tcsc10", "tex8", "tex9", "tex10",
	"ti7", "ti8", "ti9", "ti10", "ti12", "tt8", "tt9", "tt10",
	"tt12", "u10", "vtt10"
};

/* for BSR CM fonts: */

static char bsrcopyrighttxt[] = "\
% Copyright (C) 1988 Blue Sky Research. All rights reserved.\n\n"; 

/* use for my own fonts */
/* static char yanycopyrighttxt[] = "\
% Copyright (C) 1990, 1991 Y&Y. All rights reserved.\n\n"; */
/* static char yanycopyrighttxt[] = "\
% Copyright (C) 1994 Y&Y, Inc. All Rights Reserved.\n\n"; */
/* static char yanycopyrighttxt[] = "\
% Copyright (C) 1996 Y&Y, Inc. All Rights Reserved.\n\n"; */
static char yandycopyrighttxt[] = "\
% Copyright (C) 2000 Y&Y, Inc.  All Rights Reserved.\n\n";

/* use for AMS fonts */
static char amscopyrighttxt[] = "\
% Copyright (C) 1991, 1992 Y&Y. All rights reserved.\n\
% Copyright (C) 1991, 1992 Blue Sky Research. All rights reserved.\n\n";

/* use for PS AT&T fonts */
static char attcopyrighttxt[] = "\
% Copyright (c) 1990, 1992 American Telephone and Telegraph Company. \
All rights reserved.\n\n";

/* use for Lucida fonts */
static char bahcopyrighttxt[] = "\
% Lucida is a registered trademark of Bigelow & Holmes, Inc.\n\n";

/* use for MathTime fonts */
static char texcopyrighttxt[] = "\
% Copyright (c) 1992, 1993 The TeXplorators Corporation\n\
% Hinting Copyright (c) 1992, 1993 Y&Y, Inc.\n\n";

/* use for MathTime Plus fonts */
static char pluscopyrighttxt[] = "\
% Copyright (c) 1996 Michael Spivak.\n\
% Hinting Copyright (c) 1996 Y&Y, Inc.\n\n";

/* use for Wolfram fonts */
static char wolframcopyrighttxt[] = "\
% Mathematica font designed by Andre Kuzniarek, with Gregg Snyder and Stephen Wolfram. (c) 1994 Wolfram Research.\n\n";
/* % Copyright (c) 1993, 1994 Wolfram\n\\n"; */

/* Other Subrs needed for hint replacement only */

/* static char othersubrs[] = "\
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
]ND\n";	*/ /* noaccess def */

/* hint replacement OtherSubr */

char *othersubr3 = "\
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
";

/* counter control OtherSubr */

char *othersubr13 = "\
{2 {cvi {{pop 0 lt {exit} if} loop} repeat} repeat}\n\
";

#ifdef OLDSTUFF
static char encrypttxt[] = "\
/encrypt {4330 0 1 3 index length 1 sub {\n\
1 index -8 bitshift 3 index 2 index get\n\
xor 2 copy 5 index 3 1 roll put exch pop\n\
add dup 15 bitshift 65535 and exch 20077 mul\n\
22719 add add 65535 and} for pop} bind def\n";

static char encodetxt[] = "\
systemdict /currentpacking known {currentpacking true setpacking} if\n\
40 dict begin\n\
/emitt {outstr outk 3 -1 roll put /outk outk 1 add def} bind def\n\
/hstem {1 emitt} bind def\n\
/vstem {3 emitt} bind def\n\
/vmoveto {4 emitt} bind def\n\
/rlineto {5 emitt} bind def\n\
/hlineto {6 emitt} bind def\n\
/vlineto {7 emitt} bind def\n\
/rrcurveto {8 emitt} bind def\n\
/closepath {9 emitt} bind def\n\
/callsubr {10 emitt} bind def\n\
/return {11 emitt} bind def\n\
/escape {12 emitt} bind def\n\
/hsbw {13 emitt} bind def\n\
/endchar {14 emitt} bind def\n\
/rmoveto {21 emitt} bind def\n\
/hmoveto {22 emitt} bind def\n\
/strokewidth {25 emitt} bind def\n\
/baseline {26 emitt} bind def\n\
/capheight {27 emitt} bind def\n\
/bover {28 emitt} bind def\n\
/xheight {29 emitt} bind def\n\
/vhcurveto {30 emitt} bind def\n\
/hvcurveto {31 emitt} bind def\n\
/dotsection {12 emitt 0 emitt} bind def\n\
/vstem3 {12 emitt 1 emitt} bind def\n\
/hstem3 {12 emitt 2 emitt} bind def\n\
/seac {12 emitt 6 emitt} bind def\n\
/sbw {12 emitt 7 emitt} bind def\n\
/div {12 emitt 12 emitt} bind def\n\
/callothersubr {12 emitt 16 emitt} bind def\n\
/pop {12 emitt 17 emitt} bind def\n\
/setcurrentpoint {12 emitt 33 emitt} bind def\n\
/code {token not {stop} if emitt} bind def\n\
/largen {255 emitt dup -24 bitshift 255 and emitt\n\
dup -16 bitshift 255 and emitt dup -8 bitshift 255 and emitt \n\
255 and emitt} bind def\n\
/encodenum {dup 107 le {dup -107 ge {139 add emitt}\n\
{dup -1131 ge {neg 108 sub dup -8 bitshift 251 add emitt 255 and emitt}\n\
{largen} ifelse} ifelse}{dup 1131 le\n\
{108 sub dup -8 bitshift 247 add emitt 255 and emitt}\n\
{largen} ifelse} ifelse} bind def\n\
currentdict end /encodedict exch def\n\
/encode{encodedict begin /outstr 420 string def /outk 0 def\n\
{token {dup type /integertype eq {encodenum} {exec} ifelse}\n\
{outstr 0 outk getinterval exit} ifelse} loop end} bind def\n\
systemdict /currentpacking known {setpacking} if\n";

static char absencodetxt[] =  "\
systemdict /currentpacking known {currentpacking true setpacking} if\n\
20 dict begin\n\
/hsbw {13 emitt} bind def\n\
/div {12 emitt 12 emitt} bind def\n\
/hstem {1 emitt} bind def\n\
/vstem {3 emitt} bind def\n\
/vstem3 {12 emitt 1 emitt} bind def\n\
/hstem3 {12 emitt 2 emitt} bind def\n\
/moveto {21 emitt} bind def\n\
/lineto {5 emitt} bind def\n\
/curveto {8 emitt} bind def\n\
/closepath {9 emitt} bind def\n\
/endchar {14 emitt} bind def\n\
/emitt {outstr outk 3 -1 roll put /outk outk 1 add def} bind def\n\
/largen {255 emitt dup -24 bitshift 255 and emitt\n\
dup -16 bitshift 255 and emitt dup -8 bitshift 255 and emitt \n\
255 and emitt} bind def\n\
/encodenum {dup 107 le {dup -107 ge {139 add emitt}\n\
{dup -1131 ge {neg 108 sub dup -8 bitshift 251 add emitt 255 and emitt}\n\
{largen} ifelse} ifelse}{dup 1131 le\n\
{108 sub dup -8 bitshift 247 add emitt 255 and emitt}\n\
{largen} ifelse} ifelse} bind def\n\
currentdict end /absencodedict exch def\n\
/absencode {absencodedict begin\n\
/outstr 600 string def	/outk 0 def /yold 0 def %  /xold 0 def\n\
token pop dup /xold exch def encodenum token pop encodenum\n\
token pop dup type /integertype eq {encodenum token pop exec} if\n\
exec {token {dup type /integertype eq \n\
{dup xold sub exch /xold exch def encodenum token pop\n\
dup yold sub exch /yold exch def encodenum}\n\
{exec} ifelse} {outstr 0 outk getinterval exit}\n\
ifelse} loop end} \nbind def\n\
systemdict /currentpacking known {setpacking} if\n";
#endif

#ifdef REMAPPING
/* stuff for "Lucida" style remapping */
static char remapping[] = "\
Encoding\n\
0 1 9 {2 copy 2 copy get exch 161 add exch put pop} for\n\
10 1 32 {2 copy 2 copy get exch 163 add exch put pop} for\n\
dup dup 32 get 128 exch put\n\
dup dup 127 get 196 exch put\n\
pop\n";
#endif

/* time capsules */
/* max useful length in bytes is MAXCHRS * lenIV = 128 * 4 = 512 */
/* if there is less here, text wraps around ... */

/* there are twenty five spaces for insertion of data and time from ctime */

/* use for BSR/Projective Solution TeX fonts */
static char *cmcapsule = "\
Y&Y  TeX CM\n\
01234567890123456789    \n\
Computer Modern fonts were designed by Donald E. Knuth at Stanford,\n\
based in part on (American) Monotype Corporation Modern 8A.\n\
Font outlines generated by Doug Henderson at Blue Sky Research,\n\
using the ScanLab tool created by Ian Morrison of Projective Solutions.\n\
Character level hinting by Blenda Horn of Y&Y.\n\
Adobe Type 1 encoding designed by Berthold K.P. Horn.\n\
Barry Smith of Blue Sky Research coordinated the effort.\n\
Long live TeX!\n\
\n";

/* use for my own  (CM ?) fonts */
/* static char *mycapsule = "\
Y&Y  Type 1\n\
01234567890123456789    \n\
Copyright (C) 1990, 1991 Y&Y.  All rights reserved\n\
Do not copy or distribute without written permission.\n\
Computer Modern fonts were designed by Donald E. Knuth at Stanford,\n\
based in part on (American) Monotype Corporation Modern 8A.\n\
\n";  */

/*  Berthold K.P. Horn */

/* use for BSR non-TeX fonts (such as the AT&T fonts) */
static char *nontexcapsule = "\
Y&Y  Type 1\n\
01234567890123456789    \n\
Font produced using tools from Y&Y\n\
Character level hinting by Blenda Horn of Y&Y.\n\
Adobe Type 1 encoding designed by Berthold K.P. Horn.\n\
\n";

/* use for BSR non-TeX fonts (such as the AT&T fonts) */
static char *attcapsule = "\
Y&Y  Type 1\n\
01234567890123456789    \n\
Font outlines generated by Ian Morrison at Projective Solutions.\n\
Character level hinting by Blenda Horn of Y&Y.\n\
Adobe Type 1 encoding designed by Berthold K.P. Horn.\n\
\n";

/* Barry Smith of Blue Sky Research coordinated the effort.\n\ */

/* use for B & H fonts */
static char *bahcapsule = "\
Y&Y  Type 1\n\
01234567890123456789    \n\
Lucida is a registered trademark of Bigelow & Holmes Inc.\n\
Copyright (c) 1991, 1992 Bigelow & Holmes Inc. Pat. Des. 289,421.\n\
Copyright (c) 1991, 1992 Y&Y. All Rights Reserved.\n\
Lucida fonts were designed by Charles Bigelow and Kris Holmes,\n\
Font outlines generated using URW's Ikarus M,\n\
Character level hinting by Blenda Horn of Y&Y.\n\
Hint replacement by Berthold K.P. Horn.\n\
\n";

/* use for Euler fonts */
static char *eulercapsule = "\
Y&Y  Type 1\n\
01234567890123456789    \n\
Euler fonts were designed by Hermann Zapf\n\
Converted to METAFONT by the Euler Project at Stanford\n\
Under the direction of Charles Bigelow's Digital Typography Project\n\
David Siegel, Carol Twombly, Scott Kim, Dan Mills, Lynn Ruggles et al\n\
Font outlines generated by Y&Y directly from METAFONT source,\n\
Character level hinting by Blenda Horn of Y&Y.\n\
Adobe Type 1 encoding by Berthold K.P. Horn.\n\
\n";

static char *texcapsule = "\
Y&Y  Type 1\n\
01234567890123456789    \n\
Georgia Tobin designed the original MathTime font outlines using METAFONT.\n\
Mike Spivak of TeXplorators converted to Type 1 format using Fontographer.\n\
Blenda Horn of Y&Y cleaned up the outlines and did the basic hinting.\n\
Berthold K.P. Horn of Y&Y did the hint replacement and outlines adjustments.\n\
Y&Y did the conversion to Macintosh, IBM PC, as well as Unix/NeXT format.\n\
Y&Y generated the various metric files, including Textures metrics.\n\
\n";

static char *pluscapsule = "\
Y&Y  Type 1\n\
01234567890123456789    \n\
Mike Spivak designed the MathTime bold fonts using Fontographer.\n\
Berthold K.P. Horn of Y&Y cleaned up the outlines.\n\
Blenda Horn of Y&Y did the basic hinting.\n\
Y&Y did the conversion to Macintosh, IBM PC, as well as Unix/NeXT format.\n\
Y&Y generated the various metric files, including Textures metrics.\n\
\n";

/* Converted to METAFONT by the Euler Project at Stanford\n\ */

char *capsuleptr; 		/* pointer into the above */
char *capsulestart;		/* pointer for wrap around */

struct ratio { /* structure for rational numbers */
	long numer;
	long denom;
};

void giveup(int code) {	/* graceful exit with meaningful error message */
	fprintf(errout, " while %s", task);
	if (chrs >= 0) fprintf(errout, " for character %d ", chrs);
	else fprintf(errout, " ");
/* 	fclose(fp_out);	remove(fn_out); */
	fputc('\n', stderr);			/* 1995/Dec/14 */
	printf("EXITING\n");
	fflush(stdout);
	exit(code);
}

/* not used right now
void setredo(void) {
	printf(" while %s", task);
	if (chrs >= 0) printf(" for character %d ", chrs);
	else printf(" ");
	redoflag++; frezflag++;
}  */

void outputerr(void) {		/* not tested for consistently yet */
	putc('\n', errout);
	perror("Output error ");
	giveup(3);
}

/* compute good rational approximation to floating point number */
/* assumes number given is positive */

struct ratio rational(double x, long nlimit, long dlimit) {
	long p0=0, q0=1, p1=1, q1=0, p2, q2;
	double s=x, ds;
	struct ratio res;
/*	printf("Entering rational %lg %ld %ld\n", x, nlimit, dlimit);  */
	
	if (x == 0.0) {		/* zero */
		res.numer = 0; res.denom = 1;
		return res;		/* the answer is 0/1 */
	}
	if (x < 0.0) {		/* negative */
		res = rational(-x, nlimit, dlimit);
		res.numer = - res.numer; 
		return res;
	}
	for(;;) {
		p2 = p0 + (long) s * p1;
		q2 = q0 + (long) s * q1;
/*		printf("%ld %ld %ld %ld %ld %ld %lg\n", p0, q0, p1, q1, p2, q2, s); */
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
	assert(q2 != 0);
	res.numer = p2; res.denom = q2;
	return res;		/* the answer is p2/q2 */
}

/* the encryption and decryption stuff comes next */

void setbyte(int num) {				/* put byte in charstring */
	*charptr++ = (unsigned char) num; 
}

unsigned char getbyte(void) {		/* get byte from charstring */
	return *charptr++; 
}

void setcrypt(int num) { /* put byte in encrypted string */
	*crypptr++ = (unsigned char) num; 
}

/* get byte from encrypted string 
unsigned char getcrypt(void) { 
	return *crypptr++; } */

/* insert unencoded (short) decimal number */
/* unsigned int unsprint(unsigned char _far *s, int num) {	 */
unsigned int unsprint(unsigned char __far *s, int num) {	
/*	unsigned char str[FILENAME_MAX]; */
	char str[NUMBERSTR_MAX];
/*	unsigned char *t=str; */
	char *t=str;
	sprintf(str, "%d ", num);
	while (*t++ != '\0') *s++ = *t++;
	return strlen(str);
}

/* insert unencoded (long) decimal number */
/* unsigned int unsprintl(unsigned char _far *s, long num) {  */
unsigned int unsprintl(unsigned char __far *s, long num) { 
/*	unsigned char str[FILENAME_MAX]; */
	char str[NUMBERSTR_MAX];
/*	unsigned char *t=str; */
	char *t=str;
	sprintf(str, "%ld ", num);
	while (*t++ != '\0') *s++ = *t++;
	return strlen(str);
}

/* insert unencoded message */
/* unsigned int unsprints(unsigned char _far *s, char *mss) {  */
unsigned int unsprints(unsigned char __far *s, char *mss) { 
	char str[NUMBERSTR_MAX];
	char *t=str;
	sprintf(str, "%s ", mss);
	while (*t++ != '\0') *s++ = *t++;
	return strlen(str);
}

/* Adobe type 1 font CharString number encoding */

void numencode(int num) {
	if (charencode != 0) {
		if (num >= -107 && num <= 107) 	setbyte(num + 139); /* single byte */
		else if (num >= 108 && num <= 1131) {  /* two bytes */
			setbyte(((num-108) >> 8) + 247); setbyte((num-108) & 0377);
		}
		else if (num >= -1131 && num <= -108) { /* two bytes */
			setbyte(((-num-108) >> 8) + 251); setbyte((-num-108) & 0377);
		}
		else if (num > 1131) { /* five bytes */
			setbyte(255); setbyte(0); setbyte(0);
			setbyte(num >> 8); setbyte(num & 0377);
		}
		else if (num < -1131) { /* five bytes */
			setbyte(255); setbyte(255); setbyte(255);
			setbyte(num >> 8); setbyte(num & 0377);
		}
	}
/*	else charptr += sprintf(charptr, "%d ", num); */
	else charptr += unsprint(charptr, num);
}

void lnumencode(long num) { /* version used for long numbers */
	if (charencode != 0) {
		if (num < 32767 && num > - 32767) numencode((int) num);
		else {	/* does this do the right thing when num is negative ? */
			setbyte(255);	/* indicate 5 byte coding used */
			setbyte((int) ((num >> 24) & 0377) );
			setbyte((int) ((num >> 16) & 0377) );
			setbyte((int) ((num >> 8) & 0377) );
			setbyte((int) (num & 0377) );
/*			printf(" %ld ", num); debugging */
		}
	}
/*	else charptr += sprintf(charptr, "%ld ", num); */
	else charptr += unsprintl(charptr, num);
}

/* Following is for test purposes only:
int numdecode(void) {
	unsigned int num, nxt;

  num = getbyte();
  if (num >= 32 && num <= 246) return num - 139;
  else if (num >= 247 && num <= 250) {
	nxt = getbyte();
	return ((num - 247) << 8) + nxt + 108;
  }
  else if (num >= 251 && num <= 254) {
	nxt = getbyte();
	return (-(num - 251) << 8) - nxt - 108;
  }
  else {
	nxt = getbyte(); nxt = getbyte();
	num = getbyte(); nxt = getbyte();
	return (num << 8) | nxt;
  }
} */
	 
unsigned char encryptbyte (unsigned char plain, unsigned short *crypter) {
	unsigned char cipher;
/*	cipher = (plain ^ (unsigned char) (*crypter >> 8)); */
	cipher = (unsigned char) ((plain ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return cipher;
}

/* Following is for test purposes only:

unsigned char decryptbyte (unsigned char cipher, unsigned short *crypter) {
	unsigned char plain;
	plain = (cipher ^  (unsigned char) (*crypter >> 8));
	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD;
	return plain;
} */

/* void makespace(unsigned char _far *s, int extra, int len) { */
void makespace(unsigned char __far *s, int extra, int len) {
	int k;
	for (k = len + extra - 1; k >= extra; k--) {
		s[k] = s[k - extra];
	}
}

void encryptcharstring(int bytes) { /* encrypt charstring */
	int i, c;
/*	char *s; */
	
	cryptchar = RCHARSTRING;
	if (extrachar > 0) 	
/*		memmove(charstring + extrachar, charstring, (unsigned int) bytes); */
		makespace(charstring, extrachar, bytes);	/* new version */
	charptr = charstring;
	assert(extrachar <= 4); 
	if (randomchar > 0)
		for (i = 0; i < extrachar; i++) setbyte(rand());
	else if (randomchar == 0)
		for (i = 0; i < extrachar; i++) setbyte(0); 
	else { /* randomchar < 0 */
/*		printf("Chrs %d Ptr %d -  ", chrs, capsuleptr - capsulestart);  */
		for (i = 0; i < extrachar; i++) {
			if ((c = *capsuleptr++) == '\0') {
				capsuleptr = capsulestart; /* wrap around */
				c = *capsuleptr++;
			}
			setbyte(c); 
		}
	}

	charptr = charstring;
	crypptr = crypstring;
	for (i = 0; i < bytes + extrachar; i++)
		if(charencrypt != 0)
			setcrypt(encryptbyte(getbyte(), &cryptchar));
		else
			setcrypt(getbyte()); /* just copy if not encrypting charstring */
	setcrypt(0); /* for good measure */
}

/* Following is for test purposes only:
void decryptcharstring(int bytes) {
	int i;
	cryptchar = RCHARSTRING;
	charptr = charstring;
	crypptr = crypstring;
	for (i = 0; i < bytes + extrachar; i++)
		setbyte(decryptbyte(getcrypt(), &cryptchar));
	memmove(charstring, charstring + extrachar, bytes);
	charptr = charptr - extrachar;
	setbyte(0);
	charptr--;
} */

/* Following is for test purposes only:
void writestringnumbers(FILE *output, unsigned char *s, int n) {
	int nc;
	int i;
	fprintf(output, " *** ");
	for (i = 0; i < n; i++) fprintf(output, "%d ", s[i]);
	nc = fprintf(output, "*** ");
	if (nc <= 0) outputerr();
} */

/* Following is for test purposes only:

void readhexstring(unsigned char *s, char *h) {
	unsigned char c, d;

	while (((c = *h++) != 0) && ((d = *h++) != 0)) {
		if (c >= '0' && c <= '9') c = c - '0';
		else c = c - '7';
		if (d >= '0' && d <= '9') d = d - '0';
		else d = d - '7';
		*s++ = (c << 4) | d;
	}
} */

/***************************************************************************/

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

void makename(char *fname, char *str) {	/* make unique temporary filename */
    char *s;
    s = strrchr(fname,'\\');			/* search back to directory part */
    if (s == NULL) {
		s = strrchr(fname,':');			/* search back to drive part */
		if (s == NULL) strcpy(fname+2, "XXXXXX");
		else strcpy(s+2, "XXXXXX");		/* stick in template six characters */
	}
    (void) _mktemp(fname);		/* replace template by unique string */
	forceexten(fname, str);		/* force appropriate extension */
}

/***************************************************************************/

int ngetc(FILE *input) { /* safely get character - check for user interrupt */
 	int c;
/*	int d; */

	if ((c = getc(input)) == EOF) {
		fprintf(errout, "Unexpected EOF");
		giveup(4);
	}
/*	if (c <= 32) { 
		if (kbhit()) {
			if ((d = _getch()) == '\035') {
				fprintf(errout, "User interrupt");
				giveup(5); 
			}
			else ungetch(d);
		}
	} */
	return c;
}

int ogetc(FILE *input) { /* get character - ignore cntrl character & space */
	int c;
/*	int d; */

	while ((c = getc(input)) <= ' ' && c != EOF) {
/*		if (kbhit()) {
			if ((d = _getch()) == '\035' ) {
			fprintf(errout, "User interrupt");
			giveup(5); 
			}
			else ungetch(d);
		} */
	}
	if (c == EOF) {
		fprintf(errout, "Unexpected EOF");
		giveup(4);
	}
	return c;
}

void writecr(FILE *output) { 	/* write new-line */
	int nc;
	
	if (redoflag == 0) {
#ifndef FLUSHCR
		putc('\r', output); 	/* stick in CR if I/O routines don't */
#endif
		nc = putc('\n', output);
		if (nc == EOF) outputerr();
	}
	clm = 0; 				/* reset column count */
}

/* put a character - count columns 
void oputc(int c, FILE *output) {	
	if (redoflag == 0) {
		if (c >= ' ') { 
			putc(c, output); clm++;
		}
		else if (c == '\n') writecr(output);
		else if (c != '\r') { 
				putc(c, output);
				clm++;
		}
	}
} */

/* Use the following for string output limited in line length */

void nputc(int c, FILE *output)	{ /* put  character - limit line */
	if (redoflag == 0) {
		if (c >= ' ') {
			if (clm >= columns) writecr(output);	/* 1993/Jan/16 */
			putc(c, output);
			clm++;	
/*			if (clm >= columns)	writecr(output); */	/* 1993/Jan/16 */
		}
		else if (c == '\n') writecr(output);
		else if (c != '\r') { /* ignore CR */
			putc(c, output);
			clm++;	
/*			if (clm >= columns)	writecr(output); */	/* 1993/Jan/16 */
		}
	}
}

int lastnumber;		/* remember last number read */

int gobblenumber(FILE *input) { /* read "coded" decimal number */
	int num=0, sgn=1, c;

	c = ogetc(input);
	if (c == 'A') c = ogetc(input); /* ignore initial 'A' (blank) */
	if (c == 'B') { 				/* 'B' for minus */
		sgn = -1; c = ogetc(input);
	}
	while (c >= '0' && c <= '9') { /* like atoi */
		num = 10 * num + (c - '0');
		c = ogetc(input);
	}
	ungetc(c, input);				/* unread terminating char */
	lastnumber = num;				/* 1992/Aug/28 */
	return (sgn < 0) ? -num : num;
}

int scalenum (int num) {	/* scale & round integer - positive or negative */
	long res;

#ifdef DEBUG
	if (denomscale == 0) {
		fprintf(errout, "Denomscale = 0");
		giveup(7);
	}
#endif
		
	if (num < 0) return ( - scalenum( - num));
	else {
		res = (numerscale * num * 2 + denomscale) / (denomscale * 2);
		return (int) res;
	}
/* shouldn't ever have overflow problems here (?) */
}

/* Debugging use only
void showstringhex(unsigned char *s, int n) {
	int i;
	putchar('\n');
	for (i = 0; i < n; i++) printf("%02X", s[i]);
} */

/* rewrite below using above ? */

void writehex(FILE *output, int num) {	/* write two hex digits given char */
	int a, b;
	if (redoflag == 0) {

#ifdef DEBUG
		if (num < 0 || num > 255) {		/* should never happen */
			fprintf(errout, "Hex num %d out of range", num);
			giveup(9);
		}
#endif

		a = (num >> 4);  b = (num & 017);
		if (a < 10) nputc(a + '0', output);
		else nputc(a + 'A' - 10, output);
		if (b < 10)	nputc(b + '0', output);
		else nputc(b + 'A' - 10, output);
	}
}

/* void writestringhex(FILE *output, unsigned char *s, int n) { */
/* void writestringhex(FILE *output, unsigned char _far *s, int n) { */
void writestringhex(FILE *output, unsigned char __far *s, int n) {
	int i;
	if (redoflag == 0)
		for (i = 0; i < n; i++) writehex(output, *s++);
}

void checkbbox (void) {	/* check if need to interchange yll and xur */
						/* NOTE: will not catch ALL cases of interchange */
	int tmp;
	
	if (xll > xur || yur < yll) {
		if (verboseflag != 0)
			printf( "FontBBox flaw: reversing xur and yll %d %d %d %d\n",
				  xll, yll, xur, yur);
		tmp = yll; yll = xur; xur = tmp;
	}
}

/*
long gcd(long a, long b) { 		
	if (b > a) return gcd(b, a);
	else if (b == 0) return a;
	else return gcd (b, a % b);
} */

/* for scale, expect:  5pt => 9/25,  7pt => 9/35,  8pt => 9/40, */
/*					  10pt => 9/50, 12pt => 3/20, 17pt => 5/48  */
/* (provided we are not correcting for bp versus pt problem) */

void setscale(double a, double b) { 	/* set up scale factor */
	double as, bs;
	struct ratio rscale;

/* 	a = a * 1000; */
	as =  a / adobescale; 				/* to get 0.001 scale in FontMatrix */
	bs = b;
/*	if (correctfontscale != 0) { as = as * 803; bs = bs * 800; } */
	if (correctfontscale != 0) { as = as * 72.27; bs = bs * 72.0; }
/*	gs = gcd(as, bs); */
	rscale = rational(as/bs, MAXSCLNUM, MAXSCLNUM);
/*	numerscale = as / gs; denomscale = bs / gs; */
	numerscale = rscale.numer; denomscale = rscale.denom;
	numerold = numerscale; denomold = denomscale;	/* original scale saved */
/*	if (dontrescale != 0) { */
	if (dontrescale != 0 && largescale == 1) {
		numerscale = 1;	denomscale = 1;
	}

	if (verboseflag != 0) 
		printf( "Using Scale Factor %ld/%ld \n", numerscale, denomscale);
	
	sxll = scalenum(xll);
	syll = scalenum(yll);
	sxur = scalenum(xur);
	syur = scalenum(yur);

/* changes scales AND FontMatrix when the following happens => cmex10 ? */
/* NOTE: this is claimed to be a problem, but it is apparently not */

	if (verboseflag != 0) {
		if (sxll <= - 2000) {
			fprintf(errout, "Scaled xll = %d < -2000 \n", sxll);
		}
		if (syll <= - 2000) {
			fprintf(errout, "Scaled yll = %d < -2000 \n", syll);
		}
		if (sxur >=  2000) {
			fprintf(errout, "Scaled xur = %d > 2000 \n", sxur);
		}
		if (syur >=  2000) {
			fprintf(errout, "Scaled yur = %d > 2000 \n", syur);
		}
	}
	if (avoidlargescale != 0 &&
		(sxll <= -2000 || syll < -2000 || sxur > 2000 || syur > 2000)) {
		largescale = largescale * 2;
		a = a * 0.5;
		setscale(a, b);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void complainlongline (char *line, int n) {
	fprintf(errout, "ERROR: input line longer than %d characters:\n", n-1);
	fputs(line, errout);
	fputc('\n', errout);
}

#ifdef IGNORED
void getline(FILE *input, int n, char *buff) {/* read a line up to newline */
	if (fgets(buff, n, input) == NULL) {
		fprintf(errout, "Unexpected EOF");
		giveup(4);
	}
	if ((int) strlen(buff) >= (n-1)) complainlongline(buff, n);
}
#endif

/* read a line up to newline or return */	/* now handles \n and \r */

/* char *getline(FILE *input, int n, char *buff) { */
char *getline(char *buff, int n, FILE *input) {
	int c;
	int k=0;
	char *s=buff;
	
	c = getc(input);
	while (c != EOF && c != '\n' && c != '\r' && k < n-2) {
		*s++ = (char) c;
		c = getc(input);		
		k++;
	}
	if (c == EOF) {
		if (k == 0) return NULL;	/* nothing more in file */
		c = '\n';					/* last line, unterminated */
	} 
	if (k >= n-2) {
		complainlongline(buff, n);
	}
	*s++ = (char) c;
	if (c == '\r') {
		c = getc(input);
		if (c == '\n') 	*s++ = (char) c;
		else ungetc(c, input);
	}
	*s++ = '\0';
	return buff;
}

void getrealline(FILE *input, char *buff, int n) { /* ignore blanks & comments */
	if (getline(buff, n, input) == NULL) {
		fprintf(stderr, "Unexpected EOF");
		giveup(4);
	}
/*	while (*buff == '%' || *buff == '\n') */
	while (*buff == '%' || *buff == '\n' || *buff == '\r')
		getline(buff, n, input);
}

/* int getsafeline(FILE *input, char *buff) { */ /* don't crash on EOF */
int getsafeline(FILE *input, char *buff, int n) { /* don't crash on EOF */
/*	if (fgets(buff, MAXLINE, input) == NULL) return EOF; */
/*	if (fgets(buff, n, input) == NULL) return EOF; */
	if (getline(buff, n, input) == NULL) return EOF;
	if ((int) strlen(buff) >= (n-1)) complainlongline(buff, n);
/*	while (*buff == '%' || *buff == '\n')  */
	while (*buff == '%' || *buff == '\n' || *buff == '\r') {
/*		if (fgets(buff, MAXLINE, input) == NULL) return EOF; */
/*		if (fgets(buff, n, input) == NULL) return EOF; */
		if (getline(buff, n, input) == NULL) return EOF;
		if ((int) strlen(buff) >= (n-1)) complainlongline(buff, n);
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void scantochars (FILE *input) {
	int c;
	char *s;
/*	char line[MAXLINE]; */
	
	task = "reading AFM file up to character metrics";
	if (traceflag) printf("%s\n", task);

	c = getc(input); /* see first whether condensed WID file */
	if (c == 'C') {
		ungetc(c, input);	return;
	}
	ungetc(c, input);
/*	getrealline(input, line); */	/* treat as full AFM file */
	getrealline(input, line, sizeof(line));	/* treat as full AFM file */
	while (strstr(line, "StartCharMetrics") == NULL) {
		if (strstr(line, "UniqueID") != NULL) { 
/*			sscanf(line, "Comment UniqueID: %ld", &uniqueid); */
		}
/*		what is happening here ? why not in readtochars ? */
		else if ((s = strstr(line, "FontName")) != NULL) {	/*  top level */
			if(sscanf(s, "FontName %s", fontname) < 1) {
				fprintf(errout, "Don't understand FontName: %s", line);
			}
			if (traceflag) printf("FontName: %s\n", fontname);
		}
/*		getrealline(input, line); */
		getrealline(input, line, sizeof(line));
	}
}

void removecr(char *str) {
	char *s, *t;
	int c;

/*	s = str;
	while(*s != '\n' && *s != '\0') s++;
	if (*s == '\n') *s = '\0'; */

	s = str;
	while (*s == ' ') s++;
	t = str;
/*	while ((c = *s) != '\n' && c != '\0') *t++ = *s++; */
	while ((c = *s) != '\n' && c != '\r' && c != '\0') *t++ = *s++;
	if (c == '\n' || c == '\r') *t = '\0'; 
} 

void readtochars (FILE *input) {	/* extract FontInfo items from AFM file */
	int c;
	char *s;
/*	char line[MAXLINE]; */
	
	task = "reading AFM file up to character metrics";
	if (traceflag) printf("%s\n", task);

	c = getc(input); /* see first whether condensed WID file */
	if (c == 'C') {
		ungetc(c, input);	return;
	}
	ungetc(c, input);

/*	first reset everything */  	/* treat as full AFM file */
	strcpy(version, "");
/*	strcpy(notice, ""); */
	strcpy(notice1, "");
	strcpy(notice2, "");
/*	strcpy(copyright, ""); */
	strcpy(fullname, "");
	strcpy(familyname, "");
	strcpy(weight, "");
/*	underlineposition = +1; */
	underlineposition = 0;
/*	underlinethickness = -1; */
	underlinethickness = 0;	
	isfixedpitch = 0;
	italicangle = 0.0;
	uniqueid = 0;
	strcpy(fontname, "");

/*	getrealline(input, line); */
	getrealline(input, line, sizeof(line));
/*	printf("FIRST LINE: %s\n", line); */
	while (strstr(line, "StartCharMetrics") == NULL) {
/*		if ((s = strstr(line, "Notice")) != NULL) { */
		if (strncmp(line, "Notice", 6) == 0) {
/*			strcpy(notice, s + 7); removecr(notice); */
			if (strcmp(notice1, "") == 0) {
				strcpy(notice1, line + 7);
				removecr(notice1);
			}
			else if	(strcmp(notice2, "") == 0) {
				strcpy(notice2, line + 7); removecr(notice2);
			}
		}
		else if ((s = strstr(line, "FullName")) != NULL) {
			strcpy(fullname, s + 9); removecr(fullname);
		}
		else if ((s = strstr(line, "Version")) != NULL) {
			strcpy(version, s + 8); removecr(version);
		}
		else if ((s = strstr(line, "FamilyName")) != NULL) {
			strcpy(familyname, s + 11); removecr(familyname);
		}
		else if ((s = strstr(line, "Weight")) != NULL) {
			strcpy(weight, s + 7); removecr(weight);
		}			
		else if ((s = strstr(line, "ItalicAngle")) != NULL) {
			if(sscanf(s, "ItalicAngle %lg", &italicangle) < 1) {
				fprintf(errout, "Don't understand ItalicAngle: %s", line);
			}
		}
		else if ((s = strstr(line, "IsFixedPitch")) != NULL) {
			if (strstr(s, "true") != NULL) isfixedpitch = -1;
			else if (strstr(s, "false") != NULL) isfixedpitch = 0;
			else fprintf(errout, "Don't understand IsFixedPitch: %s", line);
		}
		else if ((s = strstr(line, "UnderlinePosition")) != NULL) {
			if(sscanf(s, "UnderlinePosition %d", &underlineposition) < 1) {
				fprintf(errout, "Don't understand UnderlinePosition: %s", 
					line);
			}
		}
		else if ((s = strstr(line, "UnderlineThickness")) != NULL) {
			if(sscanf(s, "UnderlineThickness %d", &underlinethickness) < 1) {
				fprintf(errout, "Don't understand UnderlineThickness: %s", 
					line);
			}			
		}
		else if ((s = strstr(line, "FontName")) != NULL) {	/*  top level */
			if(sscanf(s, "FontName %s", fontname) < 1) {
				fprintf(errout, "Don't understand FontName: %s", line);
			}			
			if (traceflag) printf("FontName: %s\n", fontname);
		}
		else if ((s = strstr(line, "FontBBox")) != NULL) { 	/*  top level */
			if (sscanf(s, "FontBBox %d %d %d %d", 
				&xll, &yll, &xur, &yur) < 4) 
					fprintf(errout, "Don't understand FontBBox: %s", line);
/*	not prepared for floating point numbers here ... */
			if (verboseflag) printf("Using %s", line);
		}
		else if ((s = strstr(line, "UniqueID")) != NULL) {	/* top level */
			if(sscanf(s, "UniqueID %ld", &uniqueid) < 1) {
				fprintf(errout, "Don't understand uniqueID: %s", line);
			}
		}
		else if ((s = strstr(line, "Space ")) != NULL) { /* Comment Space */
			if(sscanf(s, "Space %lg", &spacewidth) < 1) {
				fprintf(errout, "Don't understand Space: %s", line);
			}
/*			printf("%s", line); */
/*			printf("spacewidth %lg\n", spacewidth); */
		}
/*		getrealline(input, line); */
		getrealline(input, line, sizeof(line));
	}
	if ((strstr(weight, "Bold") != NULL) ||
		(strstr(weight, "Demi") != NULL) ||		/* ??? */
		(strstr(weight, "Heavy") != NULL)) boldflag = 1;
	else boldflag = 0;
/*	printf("WEIGHT %s => BOLD FLAG %d ", weight, boldflag); */
	if (italicangle == 0.0) italicflag = 0;
	else italicflag = 1;
}

/*
void scantoEOF (FILE *input) {
		
	getrealline(input, line, sizeof(line));
	while (strstr(line, "EndFontMetrics") == NULL) {
		getrealline(input, line, sizeof(line));
	}
}  */

int lookup(char *charname, int guess) { /* get char code from char name */
	int k;

	if (guess >= 0) {
		if (strcmp(encoding[guess], charname) == 0) return guess;
	}
	for (k=0; k < fontchrs; k++) {				/* MAXCHRS ? */
		if (strcmp(encoding[k], charname) == 0) return k;
	}
	return -1;	/* not found */
}

/* FORMAT:  C 0 ; WX 625.333 ; */
/* FORMAT:	C 0 ; WX 625 ; N 0 ; B 34 0 580 677 ; */
/* FORMAT:	C 0 ; WX 625.33 ; N 0 ; B 34 0 580 677 ; */
/* FORMAT:	C 0 ; WX 625.33 ; N Gamma ; B 34 0 580 677 ; */

void dowidthlines(FILE *input) {
/* 	char line[MAXLINE]; */
	int chr;
 	double fwidth;
	static char ename[CHARNAME_MAX];
	char charhit[256];

	for (chr=0; chr < 256; chr++) charhit[chr] = 0;	/* 1992/Sep/27 */

	if (texfont) {
		if (remapflag == 0) unchrs = 160; /* was: != ? if at all ? */
		else unchrs = 128;				  /* if at all ? */
	}
	else unchrs = 0;		/* if not texfont, start at zero */
	
	task = "reading char widths and codes from AFM file";
	if (traceflag) printf("%s\n", task);
/*	getrealline(input, line); */
	getrealline(input, line, sizeof(line));

/*	printf("Reading AFM file now\n"); */

/*	spacewidth = -1000.0; */

/*	for (k = 0; k < fontchrs; k++) { */
	while(strstr(line, "EndCharMetrics") == NULL) {
		if (traceflag) printf("%s", line);
		if(strstr(line, "EndCharMetrics") != NULL) break;
		if (sscanf(line," C %d ; WX %lg ; N %s", &chr, &fwidth, ename) < 3) {
			if (sscanf(line, " C %d ; WX %lg", &chr, &fwidth) < 2) {
				fprintf(errout, "Parse error line from AFM file: %s",
					line);
				giveup(1);
			}
/*			what if sscanf returns 2 but not 3 ??? BUG ? */
		}
#ifdef USEAFMENCODE
		else {
			if (strcmp(ename, "space") == 0) {
				spacewidth = fwidth; /* NEW */
/*				printf("spacewidth set to %lg\n", spacewidth); */
			}
			if (chr < 0 && unencodeflag != 0) {	/* unencoded characters */
/*				fprintf(errout, "Character code %d out of range\n", chr);  */
				chr = unchrs; unchrs++;
				if (unchrs == 32) unchrs = 128;
				else if (unchrs == 160) {
					fprintf(errout, "ERROR: Overflowed unencoded character space");
					fatal++;
					unchrs--;
				}
			}
			if (chr > fontchrs) {
				fprintf(errout,	"ERROR: Character code %d out of range (AFM)\n", chr);
				fatal++;
			}			
			else if (strlen(ename) >= MAXCHARNAME) {
				fprintf(errout, "ERROR: Character name to long: `%s'", ename); 
				fatal++;
			} 
			else if (chr >= 0) {
				if (strcmp(encoding[chr], "") != 0 &&
					strcmp(encoding[chr], ename) != 0) {
					fprintf(errout, 
					 "ERROR: Attempt to overwrite encoding `%s' with `%s'\n",
							encoding[chr], ename);
					fatal++;
				}
				else strcpy(encoding[chr], ename);
/*				printf("Char %d: `%s'  ", chr, ename); */
			}
		}
#endif
/* following doesn't seem to work since largescale not yet changed from 1 */
/*		if (largescale != 1) fwidth = fwidth / largescale; *//* 1992/Aug/28 */

		if (chr >= 0 && chr < 256) {
			if (charhit[chr] != 0)	{			/* 1992/Sep/27 */
				fprintf(errout, "\nERROR: Duplicate entry for char %d\n", 
					chr);
				fatal++;
			}
			charhit[chr]++;
		}

		if (chr >= 0 && chr < fontchrs) widths[chr] = fwidth;
/*		if (getsafeline(input, line) == EOF) break; */
		if (getsafeline(input, line, sizeof(line)) == EOF) break;
/*		getrealline(input, line, sizeof(line)); */
	}
}

int countcomposites(FILE *input) {
	int k=0, i, present;
	char charname[MAXCHARNAME];

	task = "looking for composites";
	if (traceflag) printf("%s\n", task);

/*	getrealline(input, line); */
	getrealline(input, line, sizeof(line));
	while (strstr(line, "StartComposites") == NULL) {
		if (strstr(line, "EndFontMetrics") != NULL) break;
/*		getrealline(input, line); */
		getrealline(input, line, sizeof(line));
	}
/*	getrealline(input, line, sizeof(line)); */
	while (strstr(line, "EndComposites") == NULL) {
		if (strstr(line, "EndFontMetrics") != NULL) break;
		if (strstr(line, "CC ") != NULL) {
/*			if (sscanf(line, "CC %s", &charname) < 1) { */
			if (sscanf(line, "CC %s", charname) < 1) {
				fprintf(errout, "ERROR: Don't understand composite: %s\n", line);
				fatal++;
			}
			present = 0;
			for (i = 0; i < fontchrs; i++) {
				if (strcmp(encoding[i], charname) == 0) {
					present = 1; break;
				}
			}
			if (present == 0) k++;	/* count only if not in encoding */
		}
/*		getrealline(input, line); */
		getrealline(input, line, sizeof(line));
	}
	return k;
}

void readwidths(FILE *input) {
	spacewidth = -1000.0;			/* initialize */
/*	printf("spacewidth reset\n"); */
	scantochars(input);
	dowidthlines(input);
	ncomposed = 0;
}

void readafmfile(FILE *input) {
	spacewidth = -1000.0;			/* initialize */
/*	printf("spacewidth reset\n"); */
	readtochars(input);
	dowidthlines(input);
	if (wantcomposites != 0) ncomposed = countcomposites(input);
	else ncomposed = 0;
/*	scantoEOF(input); */
}

void lowercase(char *s, char *t) { /* convert to lower case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'A' && c <= 'Z') *s++ = (char) (c + 'a' - 'A');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

void uppercase(char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

long makeuniqueid(char *name) { /* make unique ID */ /* unsigned ??? */
	long unique=0;
	char namel[MAXFONTNAME]; /* lower case version */
	int i, n;

	lowercase(namel, name);		/* in case it is upper case */
	if(strncmp(namel, "cm", 2) != 0) {
		printf("Not cm font: %s (no unique ID)\n", namel);
		return uniqueid;		/* hopefully there was one in AFM file */
/*		texfont = 0;  ? */
	}
	else {
		n = sizeof(cmfonts)/sizeof(cmfonts[0]);
		for (i = 0; i < n; i++)	{
			if(strcmp(namel + 2, cmfonts[i]) == 0) {
				unique = UNIQUEIDSTART + i; break;
			}
		}
		if (unique == 0)
			printf("Unknown cm font: %s\n", namel);
		if (verboseflag != 0 && unique > 0)
			printf("UniqueID is %ld\n", unique);
	}
	return unique;
}

/* encrypt and write n chars */
/* void eexecout(FILE *output, unsigned char *s, int n) { */
/* void eexecout(FILE *output, unsigned char _far *s, int n) { */
void eexecout(FILE *output, unsigned char __far *s, int n) {
	int i;
	unsigned char c;

	if (redoflag == 0) {
		for (i = 0; i < n; i++) {
			c = *s++;
			if (eexecencrypt != 0) c = encryptbyte(c, &cryptee); /* encrypt */
			if (eexecbinflag != 0) nputc(c, output); /* write binary bytes */
			else writehex(output, c); 	/* write hex format */
		}
	}
}		

void nputcout(FILE *output, char *s) {		/* write string using nputc */
	unsigned char c;

	while ((c = (unsigned char) *s++) != '\0')	
		nputc(c, output);			/* write character */
}		

/* write unencoded string */
/* void plainout(FILE *output, unsigned char *s) {  */
/* void plainout(FILE *output, unsigned char _far *s) {  */
void plainout(FILE *output, unsigned char __far *s) { 
	unsigned int c;
/* 	unsigned char _far *t; */
	unsigned char __far *t;
	int n;

	if (redoflag == 0) {
		while ((c = *s++) != 0) {
			n = 0; t = s;
			while (*t++ != ' ') n++;
/*			if (c == ' ' && (clm + (strchr(s, ' ') - s) >= columns)) */
			if (c == ' ' && (clm + n >= columns))
				writecr(output);
			putc(c, output); 	/* test for overflow ? */
			clm++;
		}
	}
}
		
/* write raw binary out */
/* void binout(FILE *output, unsigned char *s, int n) {  */
/* void binout(FILE *output, unsigned char _far *s, int n) {  */
void binout(FILE *output, unsigned char __far *s, int n) { 
	int i;

	if (redoflag == 0) {
		if (charencode != 0) 
			for (i = 0; i < n; i++) putc(*s++, output);
		else plainout(output, s);
	}
}	

/*  possibly eexec encrypt - and then print strings  */
void eeprintf(FILE *output, char *s) { 
	int nc;

	if (redoflag == 0) {
		if (eexecencrypt != 0) 	/* non-zero means encrypt eexec */
/*			eexecout(output, s, (int) strlen(s)); */
			eexecout(output, (unsigned char *) s, (int) strlen(s));
		else {
			nc = fprintf(output, "%s", s);	
			if (nc <= 0) outputerr(); /* see if device full */
		}
	}
}

void initialeexec(FILE *output) { /* prime the encryption pump */
/*	int i; */
	char head[EXTRAEEXEC + 1];

	cryptee = REEXEC; 	/* initialize seed */
	clm = 0;			/* initialize column */
	if (redoflag == 0) {
/*		for (i = 0; i < EXTRAEEXEC; i++) head[i] = (char) rand(); */
/*		for (i = 0; i < EXTRAEEXEC; i++) head[i] = 0;  */
		strcpy(head, AUTHOR);		/* bury author' signature ! */
		if (eexecencrypt != 0)		/* non-zero means encrypt eexec */
/*			eexecout(output, head, EXTRAEEXEC); */ /* encrypt random header */
			eexecout(output, (unsigned char *) head, EXTRAEEXEC);
		else {
/*			putc('\n', output);  */
		}
	}
}

void spewout(char *s) {
/*	charptr += sprintf(charptr, "%s ", s); */
	charptr += unsprints(charptr, s);
}

/* Adobe Font 1 CharString BuildChar Encoding: */

void code_hstem(void) {
	if (charencode != 0) setbyte(1); else spewout("hstem");}
void code_vstem(void) {
	if (charencode != 0) setbyte(3); else spewout("vstem");}
void code_vmoveto(void) {
	if (charencode != 0) setbyte(4); else spewout("vmoveto");}
void code_rlineto(void) {
	if (charencode != 0) setbyte(5); else spewout("rlineto");}
void code_hlineto(void) {
	if (charencode != 0) setbyte(6); else spewout("hlineto");}
void code_vlineto(void) {
	if (charencode != 0) setbyte(7); else spewout("vlineto");}
void code_rrcurveto(void) {
	if (charencode != 0) setbyte(8); else spewout("rrcurveto");}
void code_closepath(void) {
	if (charencode != 0) setbyte(9); else spewout("closepath");}
void code_callsubr(void) {
	if (charencode != 0) setbyte(10); else spewout("callsubr");}
void code_return(void) {
	if (charencode != 0) setbyte(11); else spewout("return");}
/* void code_escape(void) {
	if (charencode != 0) setbyte(12); else spewout("escape");} */
void code_hsbw(void) {
	if (charencode != 0) setbyte(13); else spewout("hsbw");}
void code_endchar(void) {
	if (charencode != 0) setbyte(14); else spewout("endchar");}
void code_rmoveto(void) {
	if (charencode != 0) setbyte(21); else spewout("rmoveto");}
void code_hmoveto(void) {
	if (charencode != 0) setbyte(22); else spewout("hmoveto");}
void code_vhcurveto(void) {
	if (charencode != 0) setbyte(30); else spewout("vhcurveto");}
void code_hvcurveto(void) {
	if (charencode != 0) setbyte(31); else spewout("hvcurveto");}
void code_dotsection(void) {
	if (charencode != 0) {setbyte(12); setbyte(0);}
	else spewout("dotsection");} 
void code_vstem3(void) {
	if (charencode != 0) {setbyte(12); setbyte(1);}
	else spewout("vstem3");}
void code_hstem3(void) {
	if (charencode != 0) {setbyte(12); setbyte(2);}
	else spewout("hstem3");}
 void code_seac(void) {
	if (charencode != 0) {setbyte(12); setbyte(6);}
	else spewout("seac");} 
/* void code_sbw(void) {
	if (charencode != 0) {setbyte(12); setbyte(7);}
	else spewout("sbw");} */
void code_div(void) {
	if (charencode != 0) {setbyte(12); setbyte(12);}
	else spewout("div");}
void code_callothersubr(void) {
	if (charencode != 0) {setbyte(12);  setbyte(16);}
	else spewout("callothersubr");} 
 void code_pop(void) {
	if (charencode != 0) {setbyte(12); setbyte(17);}
	else spewout("pop");} 
void code_setcurrentpoint(void) {
	if (charencode != 0) {setbyte(12); setbyte(33);}
	else spewout("setcurrentpoint");}

/* absolute calls only used in absolute output mode */
	
void code_moveto(void) { spewout("moveto"); }
void code_lineto(void) { spewout("lineto"); }
void code_curveto(void) { spewout("curveto"); }

/* counts font hints in order to calculate required size of Private dict */
/* assumes that each item to be placed in Private is represented by */
/* a line that starts with a '/' */
/* if you don't want an item to count, don't put first in a line ! */

int countfonthints (FILE *hinting) { /* count number of font level hints */
/*	char line[MAXLINE]; */
/*	int m; */
	int k=0, l=0, c;

	forceboldseen = 0;
	needcountercontrol = 0;
/*	stop at char-level hints or subr info */
/*	while ((c = getc(hinting)) != 'C' && c != 'S') { */
	while ((c = getc(hinting)) != 'C' && c != 'S' && c != EOF) {
		if (c == '/') k++;			/* count hint line if starts with `/'*/
/*		unless it's something special - see below */
		ungetc(c, hinting);
		if (getline(line, sizeof(line), hinting) == NULL) {
			printf("EOF in reading hints\n");
			break;		/* no more hints */
		}
/*		printf("Line of hint file counted: %s", line); */ /* debugging */
/*		if (*line != '%' && *line != '\n') { */
		if (*line == '%' || *line == '\n' || *line == '\r')
			continue;	/* ignore comments */
/*		if (strstr(line, "BlueValues") != NULL) k--;  */
		if (strncmp(line, "/BlueValues", 11) == 0) k--;  /*	don't count BlueValues */
		else if (strncmp(line, "/MinFeature", 11) == 0 ||
				 strncmp(line, "/password", 9) == 0 ||
				 strncmp(line, "/UniqueID", 9) == 0) k--;
/*				ignore stuff that we will put in later directly 97/July/13 */
		else if (strstr(line, "ForceBold") != NULL) {
			forceboldseen++; 	/* make note of presence of ForceBold */
/*			k--; */		    /* (since already counted elsewhere) */
		}
		else if (strstr(line, "RndStemUp") != NULL ||
			strstr(line, "ExpansionFactor") != NULL ||
			strstr(line, "LanguageGroup") != NULL) needcountercontrol++;
	}
	ungetc(c, hinting);
	
/*	topsubr = 0; */			/* highest number of subr encountered */
/*  new - count hint replacement subrs */
/*	assumed to come between font level hints and character level hints */
/*	while ((c = getc(hinting)) != 'C') { */
	while ((c = getc(hinting)) != EOF) { /* NEW count all - to end of file */
		if (c == 'S') l++;				/* count hint replacement subr line */
		ungetc(c, hinting);
		getline(line, sizeof(line), hinting);
/*		if (sscanf(line, "S %d", &m) < 1) {
			fprintf(errout, "Don't understand subr line: %s", line);
		} 
		else {
			if (m < 4) 
				fprintf(errout, "Not allowed to define Subr %d < 4", m); 
			if (m > topsubr) topsubr = m; 
		} */
	}

	ungetc(c, hinting);
	hintsubrs = l;			/* number of hint replacement subrs */
/*	printf("There are %d hint replacement subrs called for\n", k); */

/*	printf("There are %d lines of font level hints\n", k); */
	rewind(hinting);
/*	fontlevel=k; */			/* number of font-level hint lines */
	return k;
}

/* /StemSnapH [ 74 107 117 132 158 ] def
   /StemSnapV [ 85 130 172 199 ] def	becomes: */

/* /StemSnapH [ 74 107 117 132 158 ] 
  systemdict /internaldict known
  { 1183615869 systemdict /internaldict get exec
  /StemSnapLength 2 copy known {get 5 lt } { pop pop true } ifelse }
  { true } ifelse {pop [ 107 132]} if def
/StemSnapV [ 85 130 172 199 ] 
  systemdict /internaldict known
  { 1183615869 systemdict /internaldict get exec
  /StemSnapLength 2 copy known {get 4 lt } { pop pop true } ifelse }
  { true } ifelse {pop [ 107 132]} if def
*/

/* /StdHW [27] def */
/* /StdVW [108] def */

int stdhw, stdvw;

void dostemsnap (FILE *output) {			/* added 94/April/13 */
	int stemsnapshorten;	/* if more than two StemSnap values */
	int preferflag=0;		/* have not yet seen preferred values */
	int snap1, snap2, snap3, snap4, snap5, snap6;
	int snap7, snap8, snap9, snap10, snap11, snap12;
	int nsnap=0;				/* how many snap values */
	char *s=NULL;

	stemsnapshorten=0;		/* normally don't take that branch */
	if ((s = strchr(line, '[')) != NULL) {
		nsnap = sscanf(s+1, "%d %d %d %d %d %d %d %d %d %d %d %d",
			&snap1, &snap2, &snap3, &snap4, &snap5, &snap6, 
				&snap7, &snap8, &snap9, &snap10, &snap11, &snap12);
		if (nsnap > 2) {					/* more than two entries ? */
			if ((s = strstr(s, "def")) != NULL) { /* snip off "def" */
/*				*s++ = '\n'; *s = '\0'; */
				*s++ = '\n'; *s++ = '\0';
				stemsnapshorten=1;	/* need to conditionally shorten */
			}
			else fprintf(errout, line);		/* missing def ? */
		}
	}
	else fprintf(errout, line);				/* missing '[' ? */
/*	output line in encrypted form as always */
	eeprintf(output, line);
/*	add extra stuff if there are more than 2 entries in the array */
	if (stemsnapshorten) {						/* add extra code if needed */
		preferflag=0;
/* s should point past the \0 at this point */
		if ((s = strchr(s, '%')) != NULL) {		/* look for preferred pair */
			if (sscanf(s+1, "%d %d", &snap1, &snap2) == 2) 
				preferflag=1;				/* found two preferred numbers */
		}
		if (!preferflag) {						/* if no preferred pair */
			fprintf(errout, "MISSING PREFERRED: %s", line);
			if (strstr(line, "SnapH") != NULL && stdhw > 0) snap1 = stdhw;
			if (strstr(line, "SnapV") != NULL && stdvw > 0) snap1 = stdvw;
		}
		eeprintf(output, "  systemdict /internaldict known\n");
		eeprintf(output, "  { 1183615869 systemdict /internaldict get exec\n");
		sprintf(line, "  /StemSnapLength 2 copy known { get %d lt } { pop pop true } ifelse }\n", nsnap);
		eeprintf(output, line);
		if (preferflag)
			sprintf(line, "  { true } ifelse {pop [%d %d]} if def\n",
				snap1, snap2);	/* snap1, snap2 initialized always ? */
		else sprintf(line, "  { true } ifelse {pop [%d]} if def\n", snap1);
		eeprintf(output, line);
	}
}

/* font level hint line must start with '/' and others may not */

void copyfonthints (FILE *output, FILE *hinting) { /* copy font level hints */
/*	char line[MAXLINE]; */
	int c, blueflag= 0;
	char *s;

	stdhw = -1; stdvw = -1;
	while ((c = getc(hinting)) != 'C' && c != 'S') { 
		if (c < 0) {						/* 1996/June/8 */
			fprintf(errout, "ERROR: Unexpected EOF\n");
			printf("WARNING: No Character Level Hints?\n");
			fflush(stdout);
			break;
		}
		ungetc(c, hinting);	
/*		if (c == '%' || c == '\n') {
			if (fgets(line, sizeof(line), hinting) == NULL) {
				fprintf(errout, "Unexpected EOF\n");
				exit(1);
			}
			continue;
		} */ /* maybe not: will miss %% comments ... */
		getline(line, sizeof(line), hinting);	/* revert to this 96/June/8 */
/*		if (getsafeline(hinting, line, sizeof(line)) == EOF) break; */
/* 		printf("Line of hint file copied: %s", line); */
/*		if (*line != '%' && *line != '\n') */	/* 1993/April/26 */
		if (*line == '%') {	/* comment line */
			if (*(line+1) == '%') /* ignore comment unless starts with `%%' */
				eeprintf(output, line+1); /* copy comment minus first `%' */
		}
		else if (*line == '\n') continue; /* copy to output unless blank line */
		else if (*line == '\r') continue; /* copy to output unless blank line */
		else if (strncmp(line, "/MinFeature", 11) == 0 ||
				 strncmp(line, "/password", 9) == 0 ||
				 strncmp(line, "/UniqueID", 9) == 0) {
/*				ignore stuff that we will put in later directly 97/July/13 */
			continue;
		}
		else {
			if (strncmp(line, "/StemSnap", 9) == 0) {
/*				sanitize /StemSnap line if needed */
				dostemsnap(output);
			}
			else eeprintf(output, line);
/*			check whether /StdHW, /StdVW, or BlueValues seen */
			if ((s = strstr(line, "/StdHW")) != NULL) {
				if ((s = strchr(s, '[')) != NULL) 
					sscanf (s+1, "%d", &stdhw);
			}
			else if ((s = strstr(line, "/StdVW")) != NULL) {
				if ((s = strchr(s, '[')) != NULL) 
					sscanf (s+1, "%d", &stdvw);
			}
			else if (strstr(line, "BlueValues") != NULL) blueflag = 1;
		}
	}					/* end of looping over font level hint lines */

	ungetc(c, hinting);		/* found a line that starts with C or S */
	if (blueflag == 0) {
		if (fakezone) eeprintf(output, "/BlueValues [-20 0] def\n");
		else eeprintf(output, "/BlueValues [] def\n");
/*		if (verboseflag != 0) */
		printf("WARNING: BlueValues missing in hint file\n");
	}
}

int iround (int k, int largescale) {	/* integer divide with rounding */
	if (k < 0) return (- iround (-k, largescale));
	else return ((k + largescale / 2) / largescale);
}

/* NOTE: only check for overlap of adjacent hints - complete if ordered */

int overlapping (int a, int b, int c, int d) {
	int temp;
	if (a > b) {temp = a; a = b; b = temp; }
	if (c > d) {temp = c; c = d; d = temp; }    
	if (c > b || a > d) return 0;
	else return -1;	
}

/* Assume for now we don't need stack deeper than 22 ... */
/* ... otherwise we need to save them on a stack and use OtherSubr 12 also */

char *countercontrol (char *buff, int sbxchr, int sbychr) {	/* 1994/Apr/19 */
	char *s;
	int k, n;
	int hgroups=0, vgroups=0;
	int stack=0;
	int xl, xr, yl, yu;
	int xold, yold;
	int temp, dummy;

	s = buff;
/*	fprintf(errout, "COUNTER: %s", buff); */
	while ((s = strchr(s, 'Y')) != NULL) {	/* how many horizontal groups */
		hgroups++; s++;						/* hstem groups come first */
	}
	s = buff;
	while ((s = strchr(s, 'X')) != NULL) {	/* how many vertical groups */
		vgroups++; s++;						/* vstem groups come after */
	}	
	if (hgroups == 0 && vgroups == 0) return buff;	/* sanity check */
	s = buff;
	if (hgroups > 0) {
		s = buff;
		s = strchr(s, 'Y');	s++;
	}
	numencode (hgroups); stack++;
	for (k = 0; k < hgroups; k++) {
		yold=sbychr;				/* usually zero */
		for (;;) {
			if (sscanf (s, "%d %d%n", &yl, &yu, &n) < 2) {
/*				fprintf(errout, "Break out on `%s'\n", s); */
				break;
			}
			s += n;
			if (sscanf (s, "%d %d", &dummy, &dummy) < 2) {
/*				fprintf(errout, "Reversing on `%s'\n", s); */
				temp = yl; yl = yu; yu = temp;
			}
			numencode (yl - yold); stack++;
			numencode (yu - yl); stack++;
			yold = yu;
		}
	}

	if (vgroups > 0) {
		s = buff;
		s = strchr(s, 'X'); s++;
	}
	numencode (vgroups); stack++;	
	for (k = 0; k < vgroups; k++) {
		xold=sbxchr;			/* usually non zero */
		for (;;) {
			if (sscanf (s, "%d %d%n", &xl, &xr, &n) < 2) {
/*				fprintf(errout, "Break out on `%s'\n", s); */
				break;
			}
			s += n;
			if (sscanf (s, "%d %d", &dummy, &dummy) < 2) {
/*				fprintf(errout, "Reversing on `%s'\n", s); */
				temp = xl; xl = xr; xr = temp;
			}
			numencode (xl - xold); stack++;
			numencode (xr - xl); stack++;
			xold = xr;
		}
	}
	if (stack > 22) {
		fprintf(errout, "ERROR: Type 1 stack exceeded\n");
		printf("EXITING\n");
		fflush(stdout);
		exit(1);
	}
	numencode (stack); stack++;
	numencode (13); stack++;			/* call OtherSubr 13 */
	code_callothersubr();
/*	printf ("LEFT OVER: %s ", s); */	/* debugging */
	return s;							/* now ready for other hints */
}

#define INFINITY 4095		/* number larger than valid coordinate */

/* code: 'H' => hstem pairs follow - 'V' => vstem pairs follow */
/* code: 'E' => hstem3 params follow - 'M' => vstem3 params follow */

/* Insert character level hints  & Insert hint replacement subr ! */
/* For character chr */
/* NOTE: for char level, ksubr < 0 */
/* NOTE: for hint replacement, ksubr > 0 */

void copycharhints (FILE *hinting, int chr, int ksubr) {
	char *s;
	long fstart;
	int k, m, c, n, zsold=-INFINITY, zfold=-INFINITY, hflag=0, sbxchr, sbychr;
	int v3flag=0, h3flag=0, xs, xf, ys, yf, zs, zf;
	int xoff=0, yoff=0; 						/* 1991/Apr/6 */
	int dif;									/* 1992/Aug/7 */

	task = "copying character level hints";
	if (traceflag) printf("%s\n", task);
	sbychr=0;					/* assumed here */
/*	if (relatflag == 0) sbxchr = 0;	else sbxchr = sbx[chr]; */
	fstart = ftell(hinting);		/* remember where we were in file */
/*	if (getsafeline(hinting, line) == EOF) return; */
	if (getsafeline(hinting, line, sizeof(line)) == EOF) return;

	if (ksubr < 0) {				/* for character level hints */
		while (*line != 'C') {		/* scan up to next character level hint */
/*			if (getsafeline(hinting, line) == EOF) return; */
			if (getsafeline(hinting, line, sizeof(line)) == EOF) return;
		}
		if(sscanf(line, "C %d ;%n", &k, &n) != 1) {
			printf("Don't understand char hint: %s\n", line);
			return;
		}
	}
	else {							/* for subrs */
		while (*line != 'S') {		/* scan up to next hint replace subr */
/*			if (getsafeline(hinting, line) == EOF) return; */
			if (getsafeline(hinting, line, sizeof(line)) == EOF) return;
		}
		if(sscanf(line, "S %d ; C %d %n", &m, &k, &n) != 2) { 
			printf("Don't understand subr hint: %s\n", line);
			return;
		}
		if (k > 255) {
			fprintf(errout, "ERROR: Invalid char code: %s", line);
			fatal++;
		}
		if (m > 255) {
			fprintf(errout, "ERROR: Invalid subr code: %s", line);
			fatal++;
		}
		chr = k;
		subrchar[subrindex] = (unsigned char) k;  /* subr for char k */
		subrcode[subrindex] = (unsigned char) m;  /* subr code m for char k */
		subrused[subrindex] = '\0';				  /* subr not called yet */
		if (traceflag != 0) 
			printf("Char %d code %d will call subr %d\n", 
				chr, m, subrindex + subroffset);
		subrindex++;
	}

	while ((ksubr < 0 && k < chr)) {	/*  (ksubr > 0 && m < ksubr) */
/*		some sequence problem ? */ /* or hint for non-exiting character */
/*		most likely repeated hint, actually... */
		printf("Hint out of order %d: %s \n", k, line);
/*		getrealline(hinting, line); *//*  attempt to fix order problem */ 
		getrealline(hinting, line, sizeof(line)); /*  attempt to fix order problem */

		if (ksubr < 0) {
			if(sscanf(line, "C %d ;%n", &k, &n) != 1) {
				printf("Don't understand char hint: %s\n", line);
				return;
			}
		}
	}
	s = line + n;
	if (relatflag == 0) sbxchr = 0;
	else sbxchr = sbx[chr];				/* side bearing x of this character */

	if ((ksubr < 0 && k > chr)) { /* (ksubr > 0 && m > ksubr)) */
		fseek(hinting, fstart, SEEK_SET);	/* no hint for this character */
	}
	else {
		while ((c = *s++) > '\0') {
			if (c != ';' && c > ' ') {
/* avoid problems with hstem3 and vstem3 when rescaled ? */	/* 1992/Aug/28 */
/* NO, OK to depend on built in fixup later */
/*				if (largescale != 1) { */
				if (suppressvstem3)
					if (c == 'M' || c == 'm') c = 'V';
				if (suppresshstem3)
					if (c == 'E' || c == 'e') c = 'H';
				if (c == 'H' || c == 'h') {
					if (h3flag != 0) {
						printf("Can't mix hstem and hstem3: %s\n", line);
						return;
					}
					hflag = 1; zsold = -INFINITY; zfold = -INFINITY;
				}
				else if (c == 'V' || c == 'v') {
					if (v3flag != 0) {
						printf("Can't mix vstem and vstem3: %s\n", line);
						return;
					}
					hflag = 0; zsold = -INFINITY; zfold = -INFINITY;
				}
				else if (c == 'M' || c == 'm') {
					v3flag = 1;
/*					printf("What is there before is: %s\n", s); */
					if (sscanf(s, "%d %d %d %d %d %d %n",
						&xs, &xf, &ys, &yf, &zs, &zf, &n) != 6) {
						printf("Need six parameters: %s\n", line);
						return;
					}
					if (largescale != 1) {	/* 1992/Aug/28 */
/*						xs = xs / largescale; xf = xf / largescale; */ 
						xs = iround (xs, largescale);
						xf = iround (xf, largescale);
/*						ys = ys / largescale; yf = yf / largescale; */
						ys = iround (ys, largescale);
						yf = iround (yf, largescale);
/*						zs = zs / largescale; zf = zf / largescale; */
						zs = iround (zs, largescale);
						zf = iround (zf, largescale);
					}
					if ((xf - xs) != (zf - zs)) {
						fprintf(errout, "ERROR: VSTEM3 mismatch in char %d\n", chr);
						fatal++;				/* ??? */
						if (adjuststem3 != 0) {
							dif = (zf - zs) - (xf - xs);
							if (dif == 1) zf--;
							else if (dif == -1) xf--;
							else if (dif == 2) {	zf--; xf++;		}
							else if (dif == -2) {	xf--; zf++;		}
							else {
								xf += dif/2;	zf -= (dif - dif/2);
							}							
						}
					}
					numencode (xs - sbxchr + xoff); numencode(xf -xs);
					numencode (ys - sbxchr + xoff); numencode(yf -ys);
					numencode (zs - sbxchr + xoff); numencode(zf -zs);
					code_vstem3();
					s = s + n;
/*				printf("What is left is: %s\n", s); */
				}
				else if (c == 'E' || c == 'e') {
					h3flag = 1;
/*				printf("What is there before is: %s\n", s); */
					if (sscanf(s, "%d %d %d %d %d %d %n",
						&xs, &xf, &ys, &yf, &zs, &zf, &n) != 6) {
						printf("Need six parameters: %s\n", line);
						return;
					}
					if (largescale != 1) {	/* 1992/Aug/28 */
/*						xs = xs / largescale; xf = xf / largescale; */ 
						xs = iround (xs, largescale);
						xf = iround (xf, largescale);
/*						ys = ys / largescale; yf = yf / largescale; */
						ys = iround (ys, largescale);
						yf = iround (yf, largescale);
/*						zs = zs / largescale; zf = zf / largescale; */
						zs = iround (zs, largescale);
						zf = iround (zf, largescale);
					}
					if ((xf - xs) != (zf - zs)) {
						fprintf(errout, "ERROR: HSTEM3 mismatch in char %d\n", chr);
						fatal++;				/* ??? */
						if (adjuststem3 != 0) {
							dif = (zf - zs) - (xf - xs);
							if (dif == 1) zf--;
							else if (dif == -1) xf--;
							else if (dif == 2) {	zf--; xf++;		}
							else if (dif == -2) {	xf--; zf++;		}
							else {
								xf += dif/2;	zf -= (dif - dif/2);
							}							
						}
					}
					numencode (xs - sbychr + yoff); numencode(xf - xs);
					numencode (ys - sbychr + yoff); numencode(yf - ys);
					numencode (zs - sbychr + yoff); numencode(zf - zs);
					code_hstem3();
					s = s + n;
/*				printf("What is left is: %s\n", s); */
				}
				else if (c == 'O' || c == 'o') {
/* 1992/April/6 offset in hint needs to come first to be useful */
/* is this actually used anywhere ??? */
					if (sscanf(s, "%d %d %n", &xoff, &yoff, &n) != 2) {
						fprintf(errout, "ERROR: Need two parameters: %s\n", line);
						fatal++;				/* ??? */
						return;
					}
					if (largescale != 1) {	/* 1992/Aug/28 */
/*						xoff = xoff / largescale; yoff = yoff / largescale; */
						xoff = iround(xoff, largescale);
						yoff = iround(yoff, largescale);						
					}
					s = s + n;		/* step over it ??? */
				}
				else if (c == 'X' || c == 'Y') {	/* new 1994/Apr/19 */
					s = countercontrol (s-1, sbxchr, sbychr);
				}
				else {
					s--;
					if (sscanf(s, "%d %d %n", &zs, &zf, &n) != 2) {
						printf("Need even number parameters: %s\n", line);
						return;
					}
					if (largescale != 1) {	/* 1992/Aug/28 */
/*						zs = zs / largescale; zf = zf / largescale; */
						zs = iround (zs, largescale);
						zf = iround (zf, largescale);						
					}
					if (zf <= zs) {
						printf("Reversed stem in char %d: %d %d\n", 
							chr, zs, zf);
/*						return; */
					}
/*					else if (overlapping(zsold, zfold, zs, zf) != 0) { */
					if (overlapping(zsold, zfold, zs, zf) != 0) {
/*						printf("Overlapping stem in char %d: %d %d %d\n", */
						printf("Stems not ordered in char %d: %d %d %d %d\n",
							chr, zsold, zfold, zs, zf);
/*						(void) _getch(); */
						return;
					}
					else {
						s = s + n;
						zsold = zs; zfold = zf;
/*					now write zs, (zf - ze), hstem or vstem */
						if (hflag != 0) {
							numencode(zs - sbychr + yoff);
							numencode(zf - zs);
							code_hstem();	
						}
						else {
							numencode(zs - sbxchr + xoff);
							numencode(zf - zs);
							code_vstem();
						}
					}
				}
			}
		}
		hintedflag++;
	}	
}

/* at least for TeX fonts try and determine properties from file name */
/* most of these flags are presently not used */
/* (used to be used for stuff now extracted from AFM file) */
/* but boldflag is used for ForceBold in Private dictionary */

void analyzename(char *name) {
	romanflag = 1; 					/* assumed default */
/*	following three used only to turn off romanflag */
	mathitalic = 0;
	mathsymbol = 0;
	mathextend = 0;
	typeflag = 0;
/*	sansflag = 0; */		/* never used */
	
	if (strncmp(name, "CM", 2) == 0) lowercase(name, name);
	if (strstr(name, "cm") != NULL) {
		boldflag = 0;		/* this has been set in analysis of AFM ? */
		italicflag = 0;		/* this has been set in analysis of AFM ? */
		if (strstr(name, "sl") != NULL ||
		   (strstr(name, "i") != NULL && 
			   strstr(name, "inch") == NULL))   /* cminch not italic */
				   italicflag = 1;
		if (strstr(name, "mmi") != NULL) mathitalic = 1; 
		if (strstr(name, "sy") != NULL) mathsymbol = 1;
		if (strstr(name, "ex") != NULL) mathextend = 1;
		if (strstr(name, "b") != NULL ||
			strstr(name, "inch") != NULL) {
			boldflag = 1; /* cminch bold */
/*	printf("NAME %s SETS BOLDFLAG %d ", name, boldflag); */
		}
/*		if (strstr(name, "ss") != NULL) sansflag = 1; */
		if ((strstr(name, "tt") != NULL && 
			 strstr(name, "vtt") == NULL) ||
			strstr(name, "tex") != NULL ||
			strstr(name, "tcsc") != NULL) typeflag = 1;
/*		if((s = strpbrk(name, "0123456789")) != NULL) 
			sscanf(s, "%d", &ptsize);  already set */
		if (italicflag != 0 || mathitalic != 0 || mathsymbol != 0 ||
			mathextend != 0 || typeflag !=0) romanflag = 0;
/* ued to include: boldflag != 0  */
/* but actually sans serif and bold extended is also roman  */
/*		if (ptsize == 5 && romanflag != 0) {  cmr5 is a kind of hybrid...
			typeflag = 1; romanflag = 0;} */
	}
}

void generateencoding(FILE *output) {
	int i, nencode;

	nencode = 128;
/*	if (notdefflag != 0) nencode++;  */		 /* silly */
/*	if (spaceflag != 0)  {
		if (remapflag) nencode = 161; 
		else nencode = 129;
	} */
	if (remapflag) nencode = 197;	/* after all ... */
	if (atmflag) nencode = 256;    /* after all that ! */

	if (nencode < fontchrs) nencode = fontchrs;  /* ! */

/*	for (k = fontchrs - 1; k >= 0; k--) 
		if (strcmp(encoding[k], "") != 0) break;
	nencode = k; */

	if (atmflag != 0) {		/* long-winded ATM compatible version */
		fprintf(output,	"/Encoding %d array\n", MAXCHRS);
/*		fprintf(output, "0 1 127 {1 index exch /.notdef put} for\n"); */
/*		if (nencode > fontchrs)
			fprintf(output, "%d 1 %d {1 index exch /.notdef put} for\n",
				fontchrs, (nencode - 1));  */
		fprintf(output, "0 1 %d {1 index exch /.notdef put} for\n",
			(nencode - 1)); /* ? */
		if (remapflag) {		/* put AHEAD of the normal encoding ! */
			for (i = 0; i <= 9; i++) 
				if (strcmp(encoding[i], "") != 0)
				  fprintf(output, "dup %d /%s put\n", (i + 161), encoding[i]);
			for (i = 10; i <= 31; i++)
				if (strcmp(encoding[i], "") != 0)
				  fprintf(output, "dup %d /%s put\n", (i + 163), encoding[i]);
			if (remapfirst != 0) { /* if 32 => 195 and 127 => 196 first */
				if (strcmp(encoding[32], "") != 0)
					fprintf(output, "dup %d /%s put\n", 195, encoding[32]);
				if (strcmp(encoding[127], "") != 0)
					fprintf(output, "dup %d /%s put\n", 196, encoding[127]); 
			}
		}
		for (i = 0; i < fontchrs; i++) {
			if (strcmp(encoding[i], "") != 0) {
				if (suppresscontrol == 0 || (i >= 32 && (i < 128 || i > 159)))
					fprintf(output, "dup %d /%s put\n", i, encoding[i]);
			}
		}
		if (remapflag) {
/*			for (i = 0; i <= 9; i++) 
				if (strcmp(encoding[i], "") != 0)
				  fprintf(output, "dup %d /%s put\n", (i + 161), encoding[i]);
			for (i = 10; i <= 32; i++)
				if (strcmp(encoding[i], "") != 0)
				  fprintf(output, "dup %d /%s put\n", (i + 163), encoding[i]); */
			if (remapfirst == 0) {
				if (strcmp(encoding[32], "") != 0)
					fprintf(output, "dup %d /%s put\n", 195, encoding[32]);
				if (strcmp(encoding[127], "") != 0)
					fprintf(output, "dup %d /%s put\n", 196, encoding[127]); 
			}
/* This puts the encoding for 128 AFTER main encoding */
			if (use128flag) {
				if (strcmp(encoding[32], "") != 0)
					fprintf(output, "dup %d /%s put\n", 128, encoding[32]);
			}
		} 
		else if (texfont != 0 && notdefflag != 0) {
/* 			fprintf(output, "dup 128 /.notdef put\n"); */ /* silly */
		}
		if (texfont != 0 && spaceflag != 0)
			fprintf(output, "dup 160 /space put\n"); /* already done ? */
		if (noaccess != 0)	fprintf(output, "readonly ");  
		fprintf(output, "def\n"); 
	}
	else {
#ifdef REMAPPING
		fprintf(output, "/Encoding "); 

/* simple form: list all character names */
		for (i=0; i < fontchrs; i++) { 
			if ((i % 8) == 0) putc('\n', output);
			fprintf(output, "/%s", encoding[i]);
		}
		if (remapflag) {
			fprintf(output, "69 {/.notdef} repeat\n");
			fprintf(output, "%d array astore def\n", nencode); /* 197 */
			fprintf(output, "%s", remapping);
		}
		else {
			if (notdefflag != 0) fprintf(output, "/.notdef\n");
			fprintf(output, "%d array astore def\n", nencode); /* 128 or 129 */
		}
#else
		fprintf(errout, "Non ATM encoding no longer supported\n");
#endif
	}
}

void writefontinfo(FILE *output) {		/* write FontInfo directory */
	int nidict = 2;							/* ItalicAngle, isFixedPitch */

	if (strcmp(fullname, "") != 0) nidict++;
	if (strcmp(familyname, "") != 0) nidict++;	
	if (strcmp(version, "") != 0) nidict++;
/*	if (strcmp(notice, "") != 0) nidict++; */
	if (strcmp(notice1, "") != 0 || strcmp(notice2, "") != 0) nidict++;
/*	if (strcmp(copyright, "") != 0) nidict++;	 */
	if (strcmp(weight, "") != 0) nidict++;
/*	if (underlineposition < 0) nidict++; */
	if (underlineposition != 0) nidict++;
/*	if (underlinethickness > 0) nidict++; */
	if (underlinethickness != 0) nidict++;
	fprintf(output, "/FontInfo %d dict dup begin\n", nidict);
	if (strcmp(version, "") != 0) {
		if (padinfo) fprintf(output, " ");
		fprintf(output, "/version (%s) readonly def\n", version);
	}
/*	if (strcmp(notice, "") != 0) 
		fprintf(output, "/Notice (%s) readonly def\n", notice); */
	if (strcmp(notice1, "") != 0) {
		if (padinfo) fprintf(output, " ");
		fprintf(output, "/Notice (%s) readonly def\n", notice1);
	}
	if (strcmp(notice2, "") != 0) {
		if (padinfo) fprintf(output, " ");
		fprintf(output, "/Notice (%s) readonly def\n", notice2);
	}
/*	if (strcmp(copyright, "") != 0) 
		fprintf(output, "/Copyright (%s) readonly def\n", copyright); */
	if (strcmp(fullname, "") != 0) {
		if (padinfo) fprintf(output, " ");
		fprintf(output, "/FullName (%s) readonly def\n", fullname);
	}
	if (strcmp(familyname, "") != 0) {
		if (padinfo) fprintf(output, " ");
		fprintf(output, "/FamilyName (%s) readonly def\n", familyname);
	}
	if (strcmp(weight, "") != 0) {
		if (padinfo) fprintf(output, " ");
		fprintf(output, "/Weight (%s) readonly def\n", weight);
	}
	if (padinfo) fprintf(output, " ");
	fprintf(output, "/ItalicAngle %lg def\n", italicangle);
	if (padinfo) fprintf(output, " ");
	if (isfixedpitch != 0) 	fprintf(output, "/isFixedPitch true def\n");
	else fprintf(output, "/isFixedPitch false def\n"); 		
/*	if (underlineposition < 0) */
	if (underlineposition != 0) {
		if (padinfo) fprintf(output, " ");
		fprintf(output, "/UnderlinePosition %d def\n", underlineposition);
	}
/*	if (underlinethickness > 0)  */
	if (underlinethickness != 0) {
		if (padinfo) fprintf(output, " ");
		fprintf(output, "/UnderlineThickness %d def\n", underlinethickness);
	}
	fprintf(output, "end readonly def\n");
}

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

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

void showmissing(void) {
	int k;
	for(k = 0; k < MAXCHRS; k++) {
		if (chardone[k] == 0 && strcmp(encoding[k], "") != 0) {
			if (traceflag != 0 || strstr(encoding[k], "space") == NULL)
				fprintf(errout, "No outline for: %s\n", encoding[k]);
		}
	}
}

void endstring(FILE *, char *, int);

void startsubr(void) {
	charptr = charstring; /* ??? */
}

/* insert the four basic subrs  */

void boilerplate(FILE *output) {

/* dup 0 15 RD 3 0 callothersubr pop pop setcurrentpoint return noaccess put*/
	startsubr(); numencode(3); numencode(0); code_callothersubr();
	code_pop(); code_pop(); code_setcurrentpoint(); 
	code_return();	endstring(output, "", 0);

/* dup 1 9 RD 0 1 callothersubr return noaccess put */
	startsubr(); numencode(0); numencode(1); code_callothersubr(); 
	code_return();	endstring(output, "", 1);

/* dup 2 9 RD 0 2 callothersubr return noaccess put */
	startsubr(); numencode(0); numencode(2); code_callothersubr();
	code_return();	endstring(output, "", 2);

/* dup 3 5 RD return noaccess put */
	startsubr();	code_return(); 	endstring(output, "", 3);

/* dup 4 15 RD 1 3 callothersubr pop callsubr return noaccess put */
	if (switchtrick != 0) {
		startsubr(); numencode(1); numencode(3); code_callothersubr(); 
		code_pop(); code_callsubr(); code_return();	endstring(output, "", 4);
	}
}

/* insert specified hint replacements */

void insertsubrs(FILE *output, FILE *hinting) {
	int k;

/*	fprintf(errout, "Insert Subrs\n"); */

/* QUESTION: has sbx[chr] been set up at this point ? YES ! */

	subrindex = 0;					/* reset subr counter */
	for (k = 0; k < hintsubrs; k++) { 
		startsubr();	
		copycharhints(hinting, 0, k + subroffset);	/* subr */
		code_return(); 	
		endstring(output, "", k + subroffset);
/*		fprintf(errout, "k %d subrchar %d subrcode %d\n",
			k, subrchar[k], subrcode[k]); */
	}
/*  actually expect subrindex = k above */
	if (subrindex != hintsubrs) {
		fprintf(errout, "ERROR: Miscounted subrs %d, not %d\n", subrindex, hintsubrs);
		fatal++;			
	}
	rewind(hinting);
}

void insertorigin(FILE *output) {
	if (originflag == 0)  fprintf(output, "%s", yandycopyrighttxt);
	else if (originflag == 1) fprintf(output, "%s", bsrcopyrighttxt);
	else if (originflag == 2)  fprintf(output, "%s", attcopyrighttxt);
	else if (originflag == 3)  fprintf(output, "%s", bahcopyrighttxt);
	else if (originflag == 4)  fprintf(output, "%s", amscopyrighttxt);
	else if (originflag == 5)  fprintf(output, "%s", texcopyrighttxt);
	else if (originflag == 6)  fprintf(output, "%s", wolframcopyrighttxt);
	else if (originflag == 7)  fprintf(output, "%s", pluscopyrighttxt);
}	/*  else leave out the copyright message */

/* write new header */

void writeheader1(FILE *output, FILE *hinting, char *fontname) {
	char namel[MAXFONTNAME];
	char namec[MAXFONTNAME];
	char *s;
	int nfdict, npdict, k;
	time_t ltime;				/* for time and date */

	lowercase(namel, fontname); 
	if (uppercaseflag > 0) uppercase(namec, fontname);
/*	else if (uppercaseflag < 0) lowercase(namec, fontname);  */
	else strcpy(namec, fontname);
	if (verboseflag != 0) printf("FontName used: %s\n", namec);	
	analyzename(namel);

	(void) time(&ltime); /* get seconds since 1970 */
/*  Here the 1.1 refers to version 1.1 of Type 1 format */
	if (psadobeflag == 0)
		fprintf(output, "%s: %s %s", "%!FontType1-1.1", namec, version); 
	else
	   fprintf(output, "%s: %s %s", "%!PS-AdobeFont-1.1", namec, version);
	if (avoiddiv != 0) fprintf(output, " NG"); /* neutered font */
	putc('\n', output);
/*	possibly truncate the creation date a bit ? */
	s = ctime(&ltime);
	lcivilize(s);									/* ??? */
/*	if (verboseflag != 0) printf("Time: %s\n", s); */
	fprintf(output, "%s: %s\n", "%%CreationDate", s);
	capsuleptr = capsulestart + 12;
	for (k=0; k < 21; k++) *capsuleptr++ = *s++;  
/*	for (k=0; k < 25; k++) *capsuleptr++ = *s++;   */
/* copy in date and time */
	capsuleptr = capsulestart;
/*	fprintf(output, "%s: %d %d\n", "%%VMusage", 23000, 28000); *//* unknown */
	insertorigin(output); 
	nfdict = 10;								/* size of font	dictionary */
	if (wantuniqueid != 0) nfdict++; 
	if (charencode == 0) nfdict = nfdict + 2;
	fprintf(output, "%d dict begin\n", nfdict);
	writefontinfo(output);
	fprintf(output, "/FontName /%s def\n", namec); 
	fprintf(output, "/PaintType 0 def\n");
	fprintf(output, "/FontType 1 def\n");
	if (dontrescale == 0) {
		fprintf(output, "/FontMatrix [%lg 0 0 %lg 0 0] readonly def\n",
			adobescale * largescale, adobescale * largescale);
	}
	else fprintf(output, "/FontMatrix [%lg 0 0 %lg 0 0] readonly def\n",
			rawnumer/rawdenom * largescale, rawnumer/rawdenom * largescale);

	task = "generating encoding";
	if (traceflag) printf("%s\n", task);

	generateencoding(output);

/*	fprintf(output, "/FontBBox {%d %d %d %d} readonly def\n", 
		sxll, syll, sxur, syur); */
	fprintf(output, "/FontBBox{%d %d %d %d}readonly def\n", 
		sxll, syll, sxur, syur);
	if (wantuniqueid != 0 && uniqueid > 0)
		fprintf(output, "/UniqueID %ld def\n", uniqueid);
	fprintf(output, "currentdict end\n");

	if (eexecencrypt != 0)	fprintf(output, "currentfile eexec\n");

/* now for the eexec encrypted part */

	initialeexec(output); /* prime the encryption pump */

/*	if (userdictflag != 0) npdict = 3;		
	else npdict = 5;	*/		/* min size of private dict */
	npdict = 3;			/* space for MinFeature, password, BlueValues  */
	forceboldseen = 0;
	if (hintsflag != 0) npdict = npdict + countfonthints(hinting); /* hints */
/*	fprintf(errout, "Saw %d subrs\n", hintsubrs); */

	if (hintsubrs > 0 || forceothersubrs) 	/* need /OtherSubrs ? */
		wantothersubrs = 1;	
	else wantothersubrs = 0; 

/*  added forceboiler 1992/Oct/8 */
	if (hintsubrs > 0 || forceboiler != 0) {
		numsubrs = hintsubrs + subroffset;	/* allow for basic subrs 0 - 3 */
	}
	else {			/* no hint replacement - so don't need this stuff */
		numsubrs = 0;	subroffset = 0; 
	}
	if (forceboldseen == 0) npdict++;			/* need space for ForceBold */
	if (charencode == 0) npdict += 2;			/* encode & NE */
	if (charencrypt == 0) npdict += 3;			/* encrypt & NC */
	if (relatflag == 0) npdict +=3;				/* absencode & NA */
	if (userdictflag == 0) {			/* if not placed in userdict */
		npdict++;						/* ND or | */
		if (conform != 0 || (charbinflag != 0 && charencode != 0))
			npdict++;					/* RD or -| */
/*		if (wantothersubrs != 0 || numsubrs != 0) */
/*		NOTE: numsubrs > 0	if	wantothersubrs != 0 */
		if (numsubrs > 0) npdict++;					/* NP or | */
	}
	if (wantothersubrs != 0) npdict++;				/* OtherSubrs */
	if (numsubrs > 0 || forcesubrs != 0) npdict++;	/* Subrs */
 	if (extrachar != 4) npdict++;					/* lenIV */
/*	if (texfont != 0) {  */
/*		if (strchr(namel, 'b') != NULL || 
			strstr(namel, "inch") != NULL)) */ /* do this always now ! */
/*			npdict++;	*/ /* ForceBold counted here - not anymore ! */ 
/*	}  */
	if (wantuniqueid != 0) npdict++;				/* UniqueID */
	sprintf(line, "dup/Private %d dict dup begin\n", npdict);
	eeprintf(output, line);
/*  if userdictflag != 0 can stick RD, ND, NP before /Private */
	if (conform != 0 || (charbinflag != 0 && charencode != 0)) { /* RD */
		if (userdictflag != 0) eeprintf(output, "userdict");
		eeprintf(output, "/RD{string currentfile exch readstring pop}");
		if (executeonly != 0) eeprintf(output, "executeonly ");
		else eeprintf(output, "bind ");
		if (userdictflag != 0) eeprintf(output, "put\n");
		else eeprintf(output, "def\n");
	}

}

/* formal parameter fontname not referenced */

void writeheader2(FILE *output, FILE *hinting, char *fontname) {
	int k;
	
	if (charencrypt == 0 || charencode == 0 || relatflag == 0) {
		fprintf(errout, "FATAL ERROR: old stuff no longer supported\n");
		fatal++;
	}
/* following taken out in order to save space */
#ifdef OLDSTUFF
	if(charencrypt == 0) 					/*  && relatflag != 0 */
		/* include encryption procedure in PS code */
		fprintf(output, "%s", encrypttxt); /* eexecencrypt == 0 */
	if (charencode == 0)					/*  && relatflag != 0 */
		/* include encoding procedure in PS code */
		fprintf(output, "%s", encodetxt);
	if (relatflag == 0)
		/* include absolute encoding procedure in PS code */
		fprintf(output, "%s", absencodetxt);
#endif
	if (relatflag == 0) 
/*		fprintf(output, "/NA{absencode encrypt def}bind def\n"); */
		fprintf(output, "/NA{absencode encrypt ND}bind def\n"); 
	if (charencode == 0)
/*		fprintf(output, "/NE{encode encrypt def}bind def\n"); */
		fprintf(output, "/NE{encode encrypt ND}bind def\n");
	if (charencrypt == 0)
/*		fprintf(output, "/NC{encrypt def}bind def\n"); */
		fprintf(output, "/NC{encrypt ND}bind def\n"); 
	if (numcodchrs != 0) {	/* use numbers not codes for character id */
		if (userdictflag != 0) eeprintf(output, "userdict");
		eeprintf (output, "/ND{0 3 -1 roll get exch def}");
		eeprintf (output, "dup 0 6 index /Encoding get put bind "); 
		if (userdictflag != 0) eeprintf(output, "put\n");
		else eeprintf(output, "def\n");
	} 
	else {	/* ND */
		if (userdictflag != 0) eeprintf(output, "userdict");
		eeprintf(output, "/ND{");
		if (noaccess != 0) eeprintf(output, "noaccess ");
		eeprintf(output, "def}");
		if (executeonly != 0) eeprintf(output, "executeonly ");
		else eeprintf(output, "bind ");
		if (userdictflag != 0) eeprintf(output, "put\n");
		else eeprintf(output, "def\n");
	}
	if (numsubrs > 0) { /* NP */ /* may need for other reason ? */
		if (userdictflag != 0) eeprintf(output, "userdict");
		eeprintf(output, "/NP{");
		if (noaccess != 0) eeprintf(output, "noaccess ");	
		eeprintf(output, "put}");
		if (executeonly != 0) eeprintf(output, "executeonly ");
		else eeprintf(output, "bind ");
		if (userdictflag != 0) eeprintf(output, "put\n");
		else eeprintf(output, "def\n");
	}

/*	eeprintf(output, "\n"); */ /* kludge - all should have \n at end */
	if (hintsflag == 0)	{
		if (fakezone) eeprintf(output, "/BlueValues [-20 0] def\n");
		else eeprintf(output, "/BlueValues [] def\n");
	}
	else copyfonthints(output, hinting);	/* copy in font level hints */
	if (extrachar != 4) {
		sprintf(line, "/lenIV %d def\n", extrachar);
		eeprintf(output, line);
	}
	eeprintf(output, "/MinFeature{16 16}def\n"); /* why braces ? */
	eeprintf(output, "/password 5839 def\n");
/*	we should avoid putting this in more than once ... 97/July/13 */	

/*	ForceBold should be given in HNT file */

	if (forceboldseen == 0) {		/* if ForceBold NOT in hint file */
		if (boldflag != 0) eeprintf(output, "/ForceBold true def\n");
		else eeprintf(output, "/ForceBold false def\n");
/*		printf("FORCEBOLD TIME boldflag = %d ", boldflag); */
	}

	if (wantuniqueid != 0 && uniqueid > 0) {
		sprintf(line, "/UniqueID %ld def\n", uniqueid);
		eeprintf(output, line);
	}
/* this is where OtherSubrs go if we need them */
	if (wantothersubrs != 0) {
/*		eeprintf(output, othersubrs); */
		eeprintf(output, "/OtherSubrs[\n");	/* opening gambit */
		eeprintf(output, "{}{}{}\n");		/* 0 - 2 FlexProc OtherSubrs */
		eeprintf(output, othersubr3);		/* hint replacement OtherSubr */
		if (needcountercontrol) {
			eeprintf(output, "{}{}{}{}{}{}{}{}\n");	/* 8 blank OtherSubrs */
			eeprintf(output, "{}");			/* Counter Control OtherSubrs */
			eeprintf(output, othersubr13);	/* Counter Control OtherSubrs */
		}
		eeprintf(output, "]ND\n");			/* noaccess def */
	}
/* this is where Subrs go if we need them */
	if (numsubrs > 0 || forcesubrs != 0) {
		sprintf(line, "/Subrs %d array\n", numsubrs);
		eeprintf(output, line);
/* now insert the subrs (numsubrs  of them) */
/* dup <n> <len> RD <binary bytes> NP */
		if (numsubrs > 0) boilerplate(output);		/* standard Subrs */
		if (hintsubrs > 0) insertsubrs(output, hinting);
		eeprintf(output, "ND\n");		/* new line because of ATM bug */
	}
/*	eeprintf(output, "2 index /CharStrings 128 dict dup begin\n"); */
/*	ncdict = fonttchrs; 	*/			/* MAXCHRS fontchrs ? */
/*	make sure count unencoded characters ! */
	ncdict = 0;		/* number of characters need slots for */
/*	we are now assuming that the AFM file has encoding correct */
	for (k = 0; k < fontchrs; k++)  
		if (strcmp(encoding[k], "") != 0) ncdict++; 
	if (ncdict != charseen) {
/*		if(traceflag != 0) */
		fprintf(errout, "ERROR: ncdict %d <> charseen %d\n", 
			ncdict, charseen); 
		showmissing();			/* show characters missing */
		fatal++;				/* ??? */
/*		ncdict = charseen; */   /* try and fix it ? */
	}
	ncdict = ncdict + ncomposed;	/* number of composite characters */
	if (notdefflag != 0) ncdict++;	/* leave space for /.notdef */
	if ((spaceflag != 0) && (lookup("space", 32) < 0)) {
		ncdict++;									/* for /space */
		if (verboseflag != 0) printf("Had to make space for `%s'\n", "space"); 
	}
	if ((nbspaceflag != 0) && (lookup("nbspace", 160) < 0)) {
		ncdict++;									/* for /nbspace */
		if (verboseflag != 0) printf("Had to make space for `%s'\n", "nbspace"); 
	}
	if ((cwmflag != 0) && (lookup("cwm", 160) < 0)) {
		ncdict++;									/* for /cwm */
		if (verboseflag != 0) printf("Had to make space for `%s'\n", "cwm"); 
	}
	if ((sfthyphenflag != 0) && (lookup("sfthyphen", 173) < 0)) {
		ncdict++;
		if (verboseflag != 0) printf("Had to make space for `%s'\n", "sfthyphen"); 
	}
	sprintf(line, "2 index /CharStrings %d dict dup begin\n", ncdict);
	eeprintf(output, line);

/*	if (verboseflag != 0) printf( "Finished with header\n"); */
}

void appendzeros(FILE *output) { /* rewrite to include in binary section */
	int k, m, n=ZEROS;
	clm = 0;
	putc('\n', output);
	while (n > 0) {
		if (n > ZEROSPERLINE) m = ZEROSPERLINE;
		else m = n;
		for (k=0; k < m; k++) putc('0', output);
		putc('\n', output);
		n = n - m;
	}
/*	fprintf (output, "cleartomark\n"); */
	clm = 0;
}

void doheader(FILE *input, FILE *output, FILE *hinting) { /* deal with head */
/* 	char line[MAXLINE+2]; */
	char *ptr, *cpt;
	char *s, *t;
	struct ratio rscale;
	int c, n;
	
#ifdef DEBUG
/*	if (verboseflag != 0) printf( "Working on header\n"); */
#endif

	task = "looking for copyright or font name";
	if (traceflag) printf("%s\n", task);

/*	getrealline(input, line); */		/* search for Copyright line */ 
	getrealline(input, line, sizeof(line));			/* search for Copyright line */
	while (strstr(line, "Copyright") == NULL &&
			strstr(line, "dict") == NULL) { /* "/cm" */
/*		getrealline(input, line); */
		getrealline(input, line, sizeof(line));
	}

	task = "looking for font name";
	if (traceflag) printf("%s\n", task);

/*	getrealline(input, line, sizeof(line)); */
	while (strstr(line, "dict") == NULL) { /* look for font name */
/*		getrealline(input, line); */
		getrealline(input, line, sizeof(line));
	}
	ptr = strchr(line, '/');
	n = strchr(ptr, ' ') - (ptr + 1);
	if (n > MAXFONTNAME) {
		fprintf(errout, "Font name too long in: %s", ptr);
		giveup(7);
	}
	if (strcmp(fontname, "") == 0) {
/*		copy the font name and  terminate it properly */
		strncpy(fontname, (ptr + 1), (unsigned int) n); 
		fontname[n] = '\0';
	}
	ptsize = 0; /* if design size is not part of fontname */
	if ((ptr = strpbrk(ptr + 1, "0123456789")) != NULL)
		sscanf(ptr, "%d ", &ptsize);
/*	printf ("Point size appears to be %d\n", ptsize); */
	if (wantuniqueid != 0 && frezflag == 0 && uniqueid == 0 && texfont != 0)
		uniqueid = makeuniqueid(fontname);

	task = "looking for FontMatrix and FontBBox";
	if (traceflag) printf("%s\n", task);

	rawnumer=0.0; rawdenom=0.0;	
	if (usehexbbox != 0) {	/* ignore FontBBox from AFM file */
		xll = 0; yll = 0; xur = 0; yur = 0;
	}
/*	getrealline(input, line); */
	getrealline(input, line, sizeof(line));
	while ((ptr = strstr(line, "/CharDefs get")) == NULL) {
/*		printf("XXXXX %s", line); */
/*		if (*line == '%' || *line == '\n') {
			getrealline(input, line, sizeof(line));
			continue;
		} */
		if ((ptr = strstr(line, "/FontMatrix")) != NULL) {
/*			printf("ZZZZZ %s", line); */
			if (((cpt = strchr(line, '%')) == NULL) || cpt > ptr) {
/*				printf("*** %s", line); */
				if(sscanf (ptr, "/FontMatrix [%lg %lg",
					&rawnumer, &rawdenom) != 2) {
					fprintf(errout, "Can't understand FontMatrix: %s", line);
					giveup(4);
				}
				if (rawdenom == 0.0) {
/*					removecr(line); */
/*					if (redoflag == 0)
						printf("Non-standard FontMatrix: %s ", line); */
/*					correctfontscale = 0; */ /* needed even for `rustic' format */
					rscale = rational(rawnumer, MAXSCLNUM, MAXSCLNUM);
					rawnumer = (double) rscale.numer;
					rawdenom = (double) rscale.denom;
/*					if (redoflag == 0)
						printf("%lg/%lg \n", rawnumer, rawdenom); */
				}
/*				printf("FontMatrix line: %s\n", line); */
			}
		}
		if ((ptr = strstr(line, "/FontBBox")) != NULL) {
/*			printf("frezflag %d usehexbbox %d line %s",
				   frezflag, usehexbbox, line); */
			if (frezflag == 0) {
			if (((cpt = strchr(line, '%')) == NULL) || cpt > ptr) {
				if (usehexbbox != 0) {
					if (sscanf(ptr, "/FontBBox [%d %d %d %d] ",
						&xll, &yll, &xur, &yur) != 4) {
						if (sscanf(ptr, "/FontBBox {%d %d %d %d} ",
							&xll, &yll, &xur, &yur) != 4) {
							fprintf(errout, "Can't understand FontBBox: %s", 
								line);
							giveup(3);
						}
					}
					if (verboseflag) printf("Using %s\n", line);
				}
			}
			}
		}
/* should need to do following only first pass ? */
/* In hex file: /version (00.99) readonly def */ /* need to strip parens */
		if ((s = strstr(line, "/version")) != NULL) {
			s = s + 8;
			c = (int) *s++;
/*			printf("BEFORE %s ", version); */
			while (c != '(' && c != '\n' && c != '\r' && c != '\0')
				c = (int) *s++;
			if (c != '(') 
				fprintf(errout, "Don't understand version: %s", line);
			else {
				t = &version[0]; 
				c = (int) *s++;
/*				while (c != ')' && c != '\n' && c != '\0') { */
				while (c != ')' && c != '\n' && c != '\r' && c != '\0') {
					*t++ = (char) c; c = (int) *s++;	
				}
				if (c != ')') 
					fprintf(errout, "Don't understand version: %s", line);
				*t++ = '\0';
/*				printf("AFTER %s ", version); */
			}
/*			strcpy(version, ptr + 8); removecr(version); */
/*			sscanf(ptr, "/version (%s)", &version); */ 
		}
/*		getrealline(input, line); */
		getrealline(input, line, sizeof(line));
	}
	if (rawnumer == 0.0 || rawdenom == 0.0) {
		fprintf(errout, "ERROR: did not find FontMatrix\n");
		rawnumer = adobescale; rawdenom = 1.0;			/* 93/Oct/25 */
/*		giveup(7); */
	}
	if (frezflag == 0 && xll == xur && yll == yur) {
		fprintf(errout, "ERROR: did not find FontBBox %d %d %d %d\n",
			   xll, yll, xur, yur);
		xll = 0; yll = -250; xur = 1000; yur = 1000; 	/* 93/Oct/25 */
/*		giveup(7); */
	}

	if (frezflag == 0) {
		checkbbox();		/* check whether yll and xur reversed */
		setscale(rawnumer, rawdenom);	/* compute scale factor */
	}

	if (verboseflag != 0 && xll != xur && yll != yur)
		printf( "FontBBox {%d %d %d %d}\n",	sxll, syll, sxur, syur);

	task = "writing new header";
	if (traceflag) printf("%s\n", task);

	if (redoflag == 0) {
		writeheader1(output, hinting, fontname);
		writeheader2(output, hinting, fontname);
	}
}

void dotrailer(FILE* output) {
	if (redoflag == 0) {
		if (ncdict != charcount) {
			putc('\n', errout);
			fprintf(errout, 
				"FATAL ERROR MISMATCH: CharString dict %d <> char count %d\n",
					ncdict, charcount);
/*			fprintf(errout, "\a");	*/ /* ring bell - disabled */
			fatal++;
			fflush(stdout);
			(void) _getch(); /* ??? */
		}
		task = "writing trailer information";
		if (traceflag) printf("%s\n", task);
		
		if (noaccess != 0)
/*		eeprintf(output, "end end readonly put noaccess put\n"); */
			eeprintf(output, "end\nend\nreadonly put\nnoaccess put\n");
		else eeprintf(output, "end end put put\n");
		eeprintf(output, "dup/FontName get exch definefont pop\n");
		if (eexecencrypt != 0) {
			eeprintf(output, "mark currentfile closefile\n"); /* ? */
			appendzeros(output);
/*		putc('\n', output); */
			fprintf(output, "cleartomark\n");
		}
		putc('\n', output);
	}
}

/* watch out: compress needs cleartomark directly after line of zeros */

/* Want to use `-1 s %' to explicitely mark start of path */

/* construct CharBBox and hence sbx */

void checkarr(int start, int end) {		/* update x and y - min and max */
	int k, x, y, sbxchr;
/*	int korigin=0; */
	int flag = 0;
	
	sbxchr = sbx[chrs];
	for (k = start; k < end; k++) {
		if (knots[k].code == 'S') {
/* following added to note where `-1 s %' occured 1992/Sep/5 */
/*			if (knots[k].x == -1 && knots[k].y == -1) korigin = k; */
			continue;		/* ignore hint subr calls */
		}
		x = knots[k].x; 	y = knots[k].y;
		if (x < sbxchr) sbxchr = x;
/* on a per character basis */
		if (x < cxmin) cxmin = x;		if (x > cxmax) cxmax = x;
		if (y < cymin) cymin = y;		if (y > cymax) cymax = y;
/* on a per font basis */
		if (x < xmin) xmin = x;		if (x > xmax) xmax = x;
		if (y < ymin) ymin = y;		if (y > ymax) ymax = y;
/*		if (x > 2000 || x < -2000 || y > 2000 || y < -2000)	{
			flag = 1;
		}  */
	}
	if (cxmax > 2000 || cxmin < -2000 || cymax > 2000 || cymin < -2000)	{
		flag = 1;
	}  
	sbx[chrs] = sbxchr;				/* sbx for this character */
	if (flag != 0 && verboseflag != 0) {
/*		printf("Coordinates may be out of range in char %d", chrs); */
/* 		apparently, this is not a problem after all ... */
	}
/*	return korigin; */			/* non-zero means => reorder path ? */
}

int checkorg(int start, int end) {		/* find origin 1993/May/6 */
	int k, korigin=0;
	
	for (k = start; k < end; k++) {
		if (knots[k].code == 'S') {
/*	following added to note where `-1 s %' occured 1992/Sep/5 */
			if (knots[k].x == -1 && knots[k].y == -1) {
				if (korigin != 0) {
					fprintf(errout, 
						"ERROR: More than one start in char %d\n", chrs);
					fatal++;
				}
				korigin = k;
			}
		}
	}
	return korigin;			/* non-zero means => reorder path ? */
}

void showarr(FILE *output, int n, int m)  {		/* debugging output */
	int k;

	if (n < 0) n = 0;
	if (m > knot) m = knot; 
/*	fprintf(output, ""); */
	for (k = n; k <= m; k++) {
		fprintf(output, "%d:(%d,%d) %c ", k,
			knots[k].x, knots[k].y, knots[k].code);
		if (knots[k].code != ' ') putc('\n', output);
	}
	putc('\n', output);
}

void checkknots(int start, int end) {  /* check for bad knot sequences */
	int k, xa, ya, xb, yb, xc, yc;
	xa = knots[start].x; ya = knots[start].y;	
	xb = knots[start+1].x; yb = knots[start+1].y;

	for (k = start+2; k < end; k++) {
		if (knots[k].code == 'S') continue;		/* ignore hint subr calls */
		xc = knots[k].x; yc = knots[k].y;
		if(xa == xb && ya == yb && xb == xc && yb == yc) tripleknots++;
		xa = xb; xb = xc; ya = yb; yb = yc;
	}
}

/* move all knots down by dk starting with knot start */
/* may need to preserve code of knot (start - dk) ... */
/* shift knots down by dk 
int shiftdown(int start, int end, int dk) { 
	int i;

	assert (start < end); 
	assert(knots[end-1].code != ' '); 
	assert(start >= dk);
	for (i = start; i < end; i++) knots[i - dk] = knots[i];
	end = end - dk; 
	return end;
} */

/* move all knots up by dk starting with knot (k - dk)  */
/* there will be two copies of knots (start - dk) through knot (start - 1) */
int shiftup(int start, int end, int dk) { 
	int i;

	assert (start < end);
/*	showarr(errout, start-3, start+3); */
	assert(knots[end-1].code != ' '); 
	end = end + dk;
	assert(start >= dk);
 	assert(start + dk < MAXKNOTS); 
	for (i = end-1; i > start; i--) 
		knots[i] = knots[i - dk];
	return end;
}

int round(double x) {
	if (x < 0) return (- (round (- x)));
	else return (int) (x + 0.5);
}

/* This does not correct spikes near hint switching code etc */

int checkspikes(int start, int end) {  /* check for spikes and correct */
	int k, xa, ya, xb, yb, xc, yc, xba, yba, xcb, ycb;
	long dot, crs;
	double rba, rcb, sint, cost, rad;

	while (knots[start].code == 'S') start++;
	xa = knots[start].x; ya = knots[start].y;
	while (knots[start+1].code == 'S') start++;	
	xb = knots[start+1].x; yb = knots[start+1].y;
	xba = xb - xa; yba = yb -ya;

	for (k = start+2; k < end; k++) {

		if (knots[k].code == 'S') continue;	
/*		if (knots[k-1].code == 'S') continue; *//* ignore hint subr calls */
/*		if (knots[k+1].code == 'S') continue; *//* ignore hint subr calls */
		xc = knots[k].x; yc = knots[k].y;
		xcb = xc - xb; ycb = yc - yb;
		if (knots[k-1].code == 'S' || knots[k+1].code == 'S') {
			xa = xb; xb = xc; ya = yb; yb = yc;
			xba = xcb; yba = ycb;
			continue; 
		}
		if (knots[k-1].code != ' ') { /* only test at break knots */
			dot = (long) xba * (long) xcb + (long) yba * (long) ycb;
/* first check if turn through more than 90 degrees */
			if (dot < 0) {
				crs = (long) xba * (long) ycb - (long) yba * (long) xcb;
				rba = sqrt((double) xba * (double) xba + 
					(double) yba * (double) yba);
				rcb = sqrt((double) xcb * (double) xcb + 
					(double) ycb * (double) ycb);
				assert(rba > 0.0); assert(rcb > 0.0); /* else dot = 0 */
/*				if (rba == 0 || rcb == 0) {
 					fprintf(errout, "\nchar %d: rba = %lg, rcb = %lg ",
						chrs, rba, rcb);
					continue;
				} */
				sint = (double) crs / (rba * rcb);
/*				if (fabs(sint) < 0.423) { */
/* second check whether |theta| > (180 - 30) */
/* can also restrict splitting to places where turns it to the right */
/* that is - negative sint - in that case split only inside spikes */
				if (fabs(sint) < 0.5 && (insideonly == 0 || sint < 0.0)) {
/*				if (fabs(sint) < 0.5) { */
					cost = (double) dot / (rba * rcb);
/*	limit how small cost is allowed to get */
					if (cost < -0.99) cost = -0.99;
/*					assert(cost != -1.0); */
/*					if (cost == -1.0) {
						xa = xb; xb = xc; ya = yb; yb = yc;
						xba = xcb; yba = ycb;
						continue; 
					} */
/* compute how for from tip the lines are separated by 1 unit */
/* r = 1 / (2 * cos (theta/2)) = (1/2) sqrt (2 / (1 + cos(theta))) */
					rad = 0.5 * sqrt(2.0 / (1 + cost));
/*					if (verboseflag != 0)
						printf("\nchar %d: cost = %lg sint = %lg rad = %lg", 
							chrs, cost, sint, rad); */
/* limit rad to be less than rcb and rba */
					if (rad >= rcb) rad = rcb;
					if (rad >= rba) rad = rba;
/* printf("(%c %c %c) ", knots[k-2].code, knots[k-1].code, knots[k].code); */
/*					showarr(stdout, 0, end); */
					end = shiftup(k - 1, end, 1);
/*					showarr(stdout, 0, end); */
/* printf("k = %d end = %d ", k, end); */
					xc = xb + round((double) xcb * rad / rcb);
					yc = yb + round((double) ycb * rad / rcb);
					xb = xb - round((double) xba * rad / rba);
					yb = yb - round((double) yba * rad / rba);
					if (xc == xb && yc == yb) {
						fprintf(errout, 
							"WARNING: Problem split in char %d at %d %d\n", 
								chrs, xb, yb);
						if (abs(xcb + xba) > abs(ycb + yba)) {
							if (xcb + xba > 0) xb--;
							else xb++;
						}
						else {
							if (ycb + yba > 0) yb--;
							else yb++;
						}
					}
/*					if (verboseflag != 0)
						printf ("(%d %d) (%d %d)\n", xb, yb, xc, yc); */
					xcb = xc - xb; ycb = yc - yb;
					knots[k].code = 'E';	/* new lineto */
					knots[k].x = xc;
					knots[k].y = yc; 
					knots[k-1].x = xb; 
					knots[k-1].y = yb;
					spikes++;
				}
			}
		}
		xa = xb; xb = xc; ya = yb; yb = yc;
		xba = xcb; yba = ycb;
	}
	return end;
}

void checkflex(int start, int end) { /* check Flex Procedure opportunities */
	int k;

	for (k = start+3; k < end-3; k++) {
		if (knots[k].code == 'S') continue;		/* ignore subr - redundant */
		if (knots[k+3].code == 'S') continue;	/* ignore subr - redundant */
		if (knots[k].code == 'C' && knots[k+3].code == 'C') {
			if (knots[k-1].y == knots[k].y &&
				knots[k].y == knots[k+1].y &&
				knots[k-3].y == knots[k+3].y &&
				abs(knots[k].y - knots[k-3].y) <= 20) {
				flexprocs++;
			}
			else if (knots[k-1].x == knots[k].x &&
				knots[k].x == knots[k+1].x &&
				knots[k-3].x == knots[k+3].x &&
				abs(knots[k].x - knots[k-3].x) <= 20) {
				flexprocs++;
			}
		}
	}
}

void closepath(void) {
	hcount++;
	code_closepath(); 	/* NOTE: should NOT update xold, yold */
}

void moveto (int x, int y) {
	mcount++;
	if (relatflag != 0) {
		if (y == yold) { 
			numencode(x-xold); code_hmoveto(); 
		}
		else if (x == xold) { 
			numencode(y-yold); code_vmoveto(); 
		}
		else {
			numencode(x-xold); numencode(y-yold); code_rmoveto(); 
		}
	}
	else {
		numencode(x); numencode(y); code_moveto();
	}
	xold=x; yold=y;
}

void dolineto(int x, int y) {
	int dx, dy;

#ifdef DEBUG
	long lsq;
#endif		

	dx = x - xold; dy = y - yold;
	if (abs(dx) < MINLINETO && abs(dy) < MINLINETO) {
		shortline++; 	/* flush this rlineto */
	}
	else {
		if (relatflag != 0) {
			if (dy == 0) { 
				numencode(dx); code_hlineto(); 
			}
			else if (dx == 0) { 
				numencode(dy); code_vlineto(); 
			}
			else { 
				numencode(dx); numencode(dy); code_rlineto(); 
			}
		}
		else {
			numencode(x); numencode(y); code_lineto();
		}
		xold=x; yold=y; 

#ifdef DEBUG
		lsq = (long) dx * (long) dx + (long) dy * (long) dy;
		if ((lsq < minlsq)) { minlsq = lsq; minlchrs = chrs;}
#endif

	}
}

void lineto (int x, int y, int k) {
	lcount++;
	if (autocloseflag != 0) {
		if ((k == 0 && reverseflag != 0) || 
			(k == knot - 1 && reverseflag == 0)) avoided++; /* knot - 2 ? */
		else dolineto(x, y);
	}
	else dolineto(x, y);
}
		
int hintreplace (int k) {		/* call for hint replacing subr */
	int m;
/*	at this point chrs is the character code and */
/*	k is the subr code within that character */
	
/*	ignore explicit specification of path start */
	if (k < 0) return 0;
	
	if (k == 0) {				/* special case for ``dotsection'' code */
/*		first check whether	happening at beginning or end of charpath */
		if (k+1 == knot) {
			closepath();		/* close path first */
			return -1;			/* indicate this happened */
		}
		else {
			code_dotsection();		/* unusual to come here ... */
			return 0;
		}
	}
	for (m = 0; m < hintsubrs; m++) {	/* find subr called for */
		if (subrchar[m] == (unsigned char) chrs && 
			subrcode[m] == (unsigned char) k) break;
	}
	
	if (m == hintsubrs) {
		fprintf(errout, 
			"ERROR: Can't find subr code %d for char %d\n", k, chrs);
		fatal++;		/* ??? */
		return 0;
	}
	else if (traceflag != 0) 
		printf("Char %d code %d now calls subr %d\n", chrs, k, m + subroffset);

	if (subrused[m] != 0) {
		fprintf(errout, "Subr %d being reused by char %d\n", k, chrs);
/* this is benign and may be for efficiency reasons */		
	}
	else subrused[m] = 1;			/* indicate it has been used */
	numencode(m + subroffset); 
	if (hintedflag++ > 0) {	/* only if we are replacing old hints here */
		if (switchtrick != 0) numencode(4);		/* short method */
		else {
			numencode(1); numencode(3); 
			code_callothersubr(); code_pop(); 
		}
	}
	code_callsubr();
	return 0;
}

void curveto (int xa, int ya, int xb, int yb, int xc, int yc) {
	int dx, dy;

#ifdef DEBUG
	long csq;
#endif

	ccount++;
	dx = xc - xold, dy = yc - yold;
	if (abs(dx) < MINCURVETO && abs(dy) < MINCURVETO) {
		dolineto(xc, yc); /* convert to rlineto */
		shortcurve++;
	}
	else {
		if (relatflag != 0) {
			if ((xa == xold) && (yc == yb)) {
				numencode(ya - yold); numencode(xb - xa);
				numencode(yb - ya); numencode(xc - xb);
				code_vhcurveto();
			}
			else if ((ya == yold) && (xc == xb)) {
				numencode(xa - xold); numencode(xb - xa);
				numencode(yb - ya); numencode(yc - yb);
				code_hvcurveto();
			}
			else {
				numencode(xa - xold); numencode(ya - yold);
				numencode(xb - xa);	numencode(yb - ya);
				numencode(xc - xb); numencode(yc - yb);
				code_rrcurveto();
			}
		}
		else {
			numencode(xa); numencode(ya); numencode(xb);
			numencode(yb); numencode(xc); numencode(yc);
			code_curveto();
		}
		xold = xc; yold = yc; 

#ifdef DEBUG
		csq = (long) dx * (long) dx + (long) dy * (long) dy;
		if ((csq < mincsq)) { mincsq = csq; mincchrs = chrs; }
#endif

	}
}
	
int codecount(int z) { /* how many bytes to code this coordinate */
	if (z == 0) return 0;
	z = abs(z);
	if (z <= 107) return 1;
	else if (z <= 1131) return 2;
	else return 5;
}

/* actually do reordering of subpath in arrays */
/* suppress when hint replacement subr calls present */
/* also used when start of path explicitely specified using -1 s % */

void circulate(int start, int end, int jo) {			
	int j;
/*	circulate drawing commands so the desired one is first/last */
/*	printf("Reordering subpath in char %d\n", chrs); */
	for (j = start + 1; j < jo; j++) 
		knots[j - (start + 1) + end] = knots[j];
	for (j = start; j < end; j++) 
		knots[j] = knots[j - start + (jo - 1)];
	knots[start].code = 'M';
	reordered++;					/* count how many reordered */
}

int checkends (int start, int end) {
	if (knots[end].x != knots[start].x ||
		knots[end].y != knots[start].y) {
		fprintf(errout, "Mismatch of ends in character %d\n", chrs);
		fprintf(errout, "%d (%d %d) not equal %d (%d %d)\n",
			end, knots[end].x, knots[end].y,
				start, knots[start].x, knots[start].y);
		fflush(errout);
		(void) _getch();
		return -1;
	}
	else return 0;
}

/* try to rearrange subpath to end/start on lineto */
/* suppress if using hint replacement subr calls */
void recycle(int start, int end) {
	int j, jbest, xe, ye, dx, dy, n, nbest;

	if (end < start + 2) return;	/* cannot do it in this case */
	checkends(start, end-1);
/*	assert(knots[end-1].x == knots[start].x); */
/*	assert(knots[end-1].y == knots[start].y); */
	jbest=0; nbest=0;
	for (j = start + 1; j < end; j++) { /* find "best" lineto */
		if (knots[j].code == 'S')	/* debugging only ! */
			fprintf(errout, "Recycle called on path with hint replacement\n");
		if (knots[j].code == 'E') {
/*			jbest = j; break; 		  */
			xe = knots[j].x; ye = knots[j].y;
			dx = xe - knots[j-1].x; dy = ye - knots[j-1].y;
			n = codecount(dx) + codecount(dy);
			n = n + codecount(xe-xold) + codecount(ye-yold); 
			if (n > nbest) { jbest = j; nbest = n; }  
		} 
	}
	if (reverseflag != 0) { /* case when path will be reversed */
		if (jbest != 0 && jbest > start + 1) {
			circulate(start, end, jbest);
			assert (knots[start+1].code == 'E');
		}
	}
	else {		/* case when path will not be reversed */
		if (jbest != 0 && jbest < end - 1) { 
			circulate(start, end, jbest+1); /* jbest - 1 ? */
			assert (knots[end-1].code == 'E');
		}
	}
}

/* this is where most of the action actually takes place ! */
/* including optimal reorganization of path and code generation */
/* should check for absence of path first ... NEW */

void endsubpath (void) { /* now check subpath, reverse it, and create code */
/*	int lstk; */	 	/* knot at which previous command found */ /* NA */
	char lstcom;		/* code of previous command */
	char com;			/* current command code */
	int k;				/* current knot */
	int kstart;			/* starting k - hack for reversal of `S' & `M' */
	int dotflag=0;		/* non-zero if path ended on `dotsection' */
	int korigin=0;		/* non-zero if start of path specified */

/* at this point knot = number of knots in subpath */
/* note that coordinates in knots[knot] should equal knots[0] ? */

	if (knot > maxknts) { maxknts = knot; maxkchrs = chrs; }	

/*	korigin = checkarr(0, knot); */ 		/* update xmin, ymin, xmax, ymax */
	checkarr(0, knot); 		/* update xmin, ymin, xmax, ymax */

	closed++;
	if (redoflag != 0) 	{knot = 0; return;}	/* don't bother on first pass */

	if (knot == 0) 	{ return; }	/* don't bother, nothing in path - NEW */

	checkknots(0, knot);	/* check on bad knot sequences */
	checkflex(0, knot);		/* check on possible Flex Proc uses */
	if (spikeflag != 0) 	/* check and correct spikes */
		knot = checkspikes(0, knot);	
	
	korigin = checkorg(0, knot);		/* 1993/May/6 - AFTER checkspikes */

	if (korigin > 0) {					/* 1992/Sep/6 */
		checkends(0, knot-1);
/*		if (knots[knot-1].x != knots[0].x ||
			knots[knot-1].y != knots[0].y) {
				fprintf(errout, "Ends don't match\n"); 
		}
		showarr(stdout, 0, knot-1); */
		knots[korigin].x = knots[korigin-1].x;
		knots[korigin].y = knots[korigin-1].y;
		knots[korigin].code = 'E';				/* ??? */
		circulate(0, knot, korigin+1);	/* change starting knot */
		knot--;							/* flush repeated element at end */
/*		showarr(stdout, 0, knot-1); */
	}

	if (reorderflag != 0 && autocloseflag != 0 && 
		nsubrs == 0 && korigin == 0) {
		recycle(0, knot);		/* try to end subpath on lineto */
	}

	if (reverseflag != 0) {
		k = knot - 1; 			/* go back to last knot in subpath */
		moveto(knots[k].x, knots[k].y);		/*  moveto first point of path */
/*		lstk = k; */
		lstcom = knots[k].code; /* remember command here */
		k--;
		while (k >= 0) {				/* step backward through subpath */
			com = knots[k].code;		/* grab code of this knot */
			if (com != ' ') {
				if (lstcom == 'E') lineto(knots[k].x, knots[k].y, k);
				else if (lstcom == 'S') hintreplace(knots[k].x);
				else if (lstcom == 'C') 
					curveto(knots[k+2].x, knots[k+2].y, knots[k+1].x,
						knots[k+1].y, knots[k].x, knots[k].y);
				else { 
					fprintf(errout, "Error subpath - code = %c", lstcom);
					showarr(errout, k, k+2);
					giveup(7);
				}
/*				lstk = k; */
				lstcom = com; /* knots[k].code */
			}
			k--;
		}
		closepath();
	}
	else {
/* if hint replacement happens right after moveto for new path ... */
/* ... then interchange order of hint replacement and moveto */
		kstart = 1;			/* following allows switching before new path */
/* hack to reverse order of moveto & hint replacement subr call */
/* (but don't do if `hint replacement' is actually `dotsection') */
		if (knots[1].code == 'S' && knots[1].x != 0) {
			hintreplace(knots[1].x);
			moveto(knots[0].x, knots[0].y);	 /* moveto first point  */
			kstart++;
		}
		else moveto(knots[0].x, knots[0].y); /* moveto first point of path */

		for (k = kstart; k < knot; k++) { 	 /* step  through subpath */
			com = knots[k].code;			 /* grab code of this knot */
			if (com != ' ') {
				if (com == 'E') lineto(knots[k].x, knots[k].y, k);
				else if (com == 'S') dotflag = hintreplace(knots[k].x); 
				else if (com == 'C') 
					curveto(knots[k-2].x, knots[k-2].y, knots[k-1].x,
						knots[k-1].y, knots[k].x, knots[k].y);
				else {
					fprintf(errout, "Error subpath - code = %c", com);
					showarr(errout, k-2, k);
					giveup(7);
				}
			}
		}
/* if `dotsection' came at end, we already closed the path in hintreplace */
		if (dotflag != 0) code_dotsection();
		else closepath();
	}
	knot = 0;	 /* ready for next subpath */
}

void startsubpath(void) {
	if (knot != 1) {
		fprintf(errout, "ERROR: Moveto not at start of subpath %d in char %d", 
			knot, chrs);
		fatal++;
		knot = 1;
/*		giveup(5); */
	}
	knots[knot-1].code = 'M';  /* moveto - first knot */
	closed = 0;
}

/* Improve poor character widths by taking advantage
   of the fact that Knuth liked 1/36pt units (for cmr10),
   and submultiples for other fonts */

double snapto (double fwidth, double quantum, double inter) {
	double stp, rstp;

	stp = fwidth / quantum;
	rstp = floor(stp + 0.5);				/* round */
/*	printf("st %lg rs %lg ", stp, rstp); */
	if (fabs(stp - rstp) * quantum < inter) {
/*		printf(" %lg", rstp * quantum); */
		snapped++;							/* count them */
		return (rstp * quantum);
	}
	else return -1.0; 	/* returns negative if not near quantum step */
}

double snapto360 (double fwidth) {
	double quantum, snapwidth, inter;
	
	if (ptsize == 0) return fwidth; /* can't do this if fontsize unknown */
	if (ptsize == 17) quantum = (250.0 / 18.0) / 17.28;
	else  quantum = (250.0 / 18.0) / (double) ptsize;
/* 	assuming +/- half of last digit error possible */
	inter = 0.5 * ((double) numerscale) / (double) denomscale;
	snapwidth = snapto (fwidth, quantum, inter);
	if (snapwidth >= 0.0) {
		if (verboseflag != 0) printf("0");
		return snapwidth;
	}
	snapwidth = snapto (fwidth, quantum, inter);
	if (snapwidth >= 0.0) {
		if (verboseflag != 0) printf("1");
		return snapwidth;
	}
	snapwidth = snapto (fwidth, quantum, inter);
	if (snapwidth >= 0.0) {
		if (verboseflag != 0) printf("2");
		return snapwidth;
	}
	snapwidth = snapto (fwidth, quantum, inter);
	if (snapwidth >= 0.0) {
		if (verboseflag != 0) printf("3");
		return snapwidth;
	}
	if (verboseflag != 0)	printf("-");
/*	printf(" %lg", fwidth); */
	return fwidth; 	/* give up, this is hopeless */
}

/* void startstring (int sbxchr, int width) { */	 /* left sidebearing and width */
void startstring (int sbxchr, int width, char *charname) {
	long numerwidth, denomwidth;
	double fwidth;
	struct ratio rwidth;

	rwidth.numer = 1;			/* in case not initialized later */
	rwidth.denom = 1;			/* in case not initialized later */
/*  first, compute width from what was in HEX file */
/*  adjust for non-standard font matrix */
	fwidth = (double) width * (double) numerscale / (double) denomscale;
/*	if (verboseflag) printf("%s\t%lg\n", charname, fwidth); */

/*	now try and improve upon it if called for */

#ifdef SNAPTOWIDTH
	if (correctfontscale != 0) 	fwidth = snapto360(fwidth);
#endif

	if (widthflag == 0) {		/* have to use widths in outline font ... */
/*	   fwidth = (double) width * (double) numerscale / (double) denomscale; */

#ifdef SNAPTOWIDTH 
/*		if (correctfontscale != 0) 	fwidth = snapto360(fwidth); */
#endif

		widths[chrs] = fwidth;		/* for fun and later use by .notdef */
	}
	else {	/* widthflag != 0  =>  use widths in AFM file */
		if (dontrescale != 0) { 	/* new twist - convert to hex units */
			fwidth = (double) width * (double) numerold / (double) denomold;
		}
		if (largescale != 1) fwidth = fwidth * largescale;	 /*	NO ? */
		if (fabs(fwidth - widths[chrs]) > widtheps) {
			fprintf(errout, 
				"ERROR: AFM width %lg <> HEX width %lg for char %d -", 
					widths[chrs], fwidth, chrs);
			if (checkhexwidths != 0) {
				fprintf(errout, " using HEX file width %lg\n", fwidth);
/*				fwidth = widths[chrs];	*/	/* use width from AFM file ? */
/*	NOTE:	presently uses width from HEX file in this case */
/*  This happens for the Euler fonts for some reason ... */
			fatal++;			/* 1994/July/18 ??? */
			}
			else {
				fwidth = widths[chrs];			/* use width from AFM file */
				fprintf(errout, " using AFM file width %lg\n", fwidth);
			}
		}
		else {						/* hex widths agrees with AFM width */
			fwidth = widths[chrs];			/* use width from AFM file */
/*			fwidth = snaptowidth(fwidth); */	 /* should not need */
		}
	}
	numencode(sbxchr); 				/* side bearing x */
	if (largescale != 1) fwidth = fwidth / largescale;	/* 1992/Aug/28 */
	if (largescale != 1) fwidth = fwidth / largescale;	/* 1992/Aug/28 */
/*  deal with non-standard font matrix problem ? */
/*	fwidth = (double) fwidth * (double) denomscale / (double) numerscale; */
	if (widthscaleflag == 0)
		fwidth = (double) fwidth * (double) denomold / (double) numerold; 
/*	if (verboseflag) printf("%s\t%lg\n",  charname, fwidth); */
	if (avoiddiv != 0) numencode(round(fwidth)); 
	else {
		rwidth = rational(fwidth, (long) NUMLIM, (long) DENLIM);
		numerwidth = rwidth.numer; denomwidth = rwidth.denom;
/*		printf(" %ld/%ld=%lg ", numerwidth, denomwidth, fwidth);  */
		lnumencode(numerwidth);
		if (denomwidth != 1) {
			lnumencode(denomwidth);
			code_div(); /* compute width */
		}
	}
	code_hsbw();
	if (traceflag)
		printf("char %s sbxchr %d width %d fwidth %lg (%ld / %ld)\n",
		   charname, sbxchr, width, fwidth, rwidth.numer, rwidth.denom);
	xold = sbxchr, yold = 0;
}

/* end of character outline (or subr) */
void endstring(FILE *output, char *charname, int ksubr) {
	int bytes, ebytes;
	char stuff[MAXLINE];	/* should be enough ? */

	if (ksubr < 0) 	charcount++;
/*	code_endchar(); */
	bytes = charptr - charstring; /* determine number of bytes used */
	ebytes = bytes + extrachar;
	if (ebytes > maxebytes) { maxebytes = ebytes; maxechrs = chrs; }
	if (ebytes >= CHARSTRINGLEN) {		/* check buffer overflow */
		fprintf(errout, "Overflowed charstring buffer (%d > %d)",
			ebytes, CHARSTRINGLEN); giveup(7);
	}
	encryptcharstring(bytes); /* encrypt - if charencrypt != 0 */
							  /* otherwise just copy across */

/* check this out - watch for buried zeros */

	if (charbinflag != 0 && charencode != 0) {
		if (numcodchrs != 0)
			sprintf(stuff, "%d %d RD ", chrs, ebytes); /* ? */
		else {
			if (ksubr < 0) {
				if (strcmp(charname, "") == 0) {
					fprintf(errout, 
						"ERROR: Missing encoding for char %d\n", chrs);
					fatal++;
					sprintf(stuff, "/a%d %d RD ", chrs, ebytes);  
				}
				else sprintf(stuff, "/%s %d RD ", charname, ebytes);  
			}
			else sprintf(stuff, "dup %d %d RD ", ksubr, ebytes);
		}
		eeprintf(output, stuff);
	}
	else {
		if (numcodchrs != 0) sprintf(stuff, "%d", chrs);
		else if (ksubr < 0) sprintf(stuff, "/%s", charname);
		else sprintf(stuff, "dup %d", ksubr);
		nputcout(output, stuff);
	}

	if (charencode == 0) nputcout(output, " (");

	if (eexecencrypt != 0) {		/* non-zero means use eexec encryption */
		eexecout(output, crypstring, ebytes);
		if (ndgapflag != 0) eeprintf(output, " ");
		if (ksubr < 0) eeprintf(output, "ND\n"); /* ? */
		else eeprintf(output, "NP\n"); /* ? */
	}
	else if (charbinflag != 0) {		/* non-zero means use binary */
		binout(output, crypstring, ebytes);
		if (ndgapflag != 0) fprintf(output, " ");
		if (ksubr < 0) fprintf(output, "ND\n"); /* new ? */
		else fprintf(output, "NP\n"); /* new ? */
	}
	else {						/* else hex code */
		nputc('<', output);
		writestringhex(output, crypstring, ebytes);
		nputc('>', output);
/*	nputc(' ', output); */
		if ((clm + 2) >= columns) writecr(output); /* ? */
		if (charencrypt == 0) nputcout(output, "NC");
		else {
			if (ndgapflag != 0) nputcout(output, " ");
			nputcout(output, "ND");
		}
		if (clm != 0) nputc('\n', output);
	}
	if ((clm + 1) >= columns) writecr(output); /* needed ? */
	if (charencode == 0) {
		nputc(')', output);
		nputc(' ', output); 					/* needed ? */
		if ((clm + 1) >= columns) writecr(output);
		if (relatflag == 0) nputcout(output, "NA");
		else nputcout(output, "NE");
		if (clm != 0) nputc('\n', output);
	}
}

void gencomposites (FILE *output, FILE *input) {
	char composite[MAXCHARNAME], base[MAXCHARNAME], accent[MAXCHARNAME];
	int k, asb, bsb, adx, ady, bchar, achar, btruechar, atruechar;
	int kc;
	int width;

/* bchar & achar are positions in StandardEncoding - use for `seac' */
/* btruechar & atruechar are positions in currently used encoding */
/* these are used later for side bearing and width calculations */

/*	rewind(input); */
/*	getrealline(input, line); */
	getrealline(input, line, sizeof(line));
	while (strstr(line, "StartComposites") == NULL) {
		if (strstr(line, "EndFontMetrics") != NULL) return; /* break */
/*		if (getsafeline(input, line) == EOF) return; */
		if (getsafeline(input, line, sizeof(line)) == EOF) return;
	}
/*	getrealline(input, line);  */
	getrealline(input, line, sizeof(line)); 
	while (strstr(line, "EndComposites") == NULL) {
		if (strstr(line, "EndFontMetrics") != NULL) return; /* break */
/*		assuming TWO components in composite, and first one is NOT displaced */
		if (sscanf(line, "CC %s 2 ; PCC %s 0 0 ; PCC %s %d %d ;",
			composite, base, accent, &adx, &ady) < 5) {
			fprintf(errout, "ERROR: Can't understand composite: %s", line);
			fatal++;
		}
		else {
/*			first check whether composite is already in encoding */
			if ((kc = lookup (composite, -1)) >= 0) {
				fprintf(errout, 
					"ERROR: composite char %s already in encoding at %d\n",
						composite, kc);
				fatal++;
			}
/*			we assume that base and accent are in same position as StandardEncoding? */
			btruechar = -1;
			for(k = 0; k < MAXCHRS; k++) {		
/*				if (strcmp(standardencoding[k], base) == 0) {  */
				if (strcmp(encoding[k], base) == 0) {   
					btruechar = k; break;
				}
			}
#ifdef USESTANDARD
			bchar = -1;
			for (k = 0; k < MAXCHRS; k++) {
				if (strcmp(standardencoding[k], base) == 0) {  
					bchar = k; break;
				}
			}
#else
/* Most base character should be in same position as in StandardEncoding */
			bchar = btruechar;
/* For normal base characters we can compute SE code number from ASCII */
			if (strlen(base) == 1) {
				if (base[0] >= 'A' && base[0] <= 'Z') bchar = base[0];
				else if (base[0] >= 'a' && base[0] <= 'z') bchar = base[0];
			}
			else if (strcmp("dotlessi", base) == 0) bchar = 245;
			else if (strcmp("AE", base) == 0) bchar = 225 ;
			else if (strcmp("ae", base) == 0) bchar = 241;
			else if (strcmp("OE", base) == 0) bchar = 234;
			else if (strcmp("oe", base) == 0) bchar = 250;
#endif
/*			if (usestandard == 0) bchar = btruechar; */	/* 96/June/30 */
			if (bchar < 0 && relaxseac != 0) bchar = btruechar + 256;
			if (bchar < 0 || btruechar < 0) {
				fprintf(errout, "ERROR: Invalid base `%s' (ASE %d font %d)\n", 
					base, bchar, btruechar);
				fatal++;
				if (bchar < 0) bchar = btruechar;	/* in desperation */
			}
			atruechar = -1;
			for(k=0; k < MAXCHRS; k++) {	/* fontchrs ??? */
				if (strcmp(encoding[k], accent) == 0) {   
/*				if (strcmp(standardencoding[k], base) == 0) { */ /* ? */
					atruechar = k; break;
				}
			}
			achar = -1;
#ifdef USESTANDARD
			for(k=0; k < MAXCHRS; k++) { 
				if (strcmp(standardencoding[k], accent) == 0) {
					achar = k; break;
				}
			}
#else
/*			for some accent characters we can compute SE code number from ASCII */
			if (strlen(accent) == 1) {
				if (accent[0] >= 'A' && accent[0] <= 'Z') achar = accent[0];
				else if (accent[0] >= 'a' && accent[0] <= 'z') achar = accent[0];
			}
			else if (strcmp("dotlessi", accent) == 0) achar = 245;
			for(k=0; k < NUMACCENTS; k++) { 
				if (strcmp(standardaccents[k], accent) == 0) {
					achar = k + 192; break;
				}
			} 
#endif
/*			if (usestandard == 0) achar = atruechar; */	/* 96/June/30 */
			if (achar < 0 && relaxseac != 0) achar = atruechar + 256;
/*			following may not make sense, since `suppress' is not StandardEncoding */
			if (achar < 0) {
				if (strcmp(accent, "suppress") == 0) achar = 32;
				else if (strcmp(accent, "hyphen") == 0) achar = 45;
				else if (strcmp(accent, "asciicircum") == 0) achar = 94;
				else if (strcmp(accent, "asciitilde") == 0) achar = 126;
			}
			if (achar < 0 || atruechar < 0) {
				fprintf(errout, "ERROR: Invalid accent `%s' (ASE %d font %d)\n", 
					accent, achar, atruechar);
				fatal++;
				if (achar < 0) achar = atruechar;			/* in desperation */
			}
			if (achar >= 0 && bchar >= 0) {
/*				following assumes characters in same position as in StandardEncoding? */
/*				asb = sbx[achar];  */
				asb = sbx[atruechar];
/*				bsb = sbx[bchar];  */
				bsb = sbx[btruechar]; 
			
				charptr = charstring; 		/* reset buffer pointers here */
/*				chrs = bchar;	*/			/* so computes correct width */
				chrs = btruechar;			/* so computes correct width */
				width = round(widths[btruechar]);
/*				startstring(bsb, round(widths[bchar]));	 */
/*				startstring(bsb, round(widths[btruechar]));	*/ /* copy base */
				startstring(bsb, width, composite);
				numencode(asb);				/* copy sidebearing from accent */
/*				numencode(adx);			*/		/* what the book says */
				numencode(adx + asb - bsb);		/* the real story ? */
				numencode(ady);
				numencode(bchar); numencode(achar);	/* char pos in StandEnc */
				code_seac();
				endstring(output, composite, -1);
				if (traceflag) printf(
   "achar %d atruechr %d bchar %d btruechar %d wx %d asb %d bsb %d adx %d ady %d\n",
			achar, atruechar, bchar, btruechar, width, asb, bsb, adx, ady);
			}
		}
/*		getrealline(input, line); */
		getrealline(input, line, sizeof(line));
	}
}

void showencoding(void) {
	int k;
	fprintf(errout, "ENCODING VECTOR:\n");
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") != 0)
			fprintf(errout, "%d %s \t", k, encoding[k]);
	}
	fflush(errout);
	(void) _getch();
}

/****************************************************************************/

/* Make Charstring for a `glyph' with no visible marks */

void blankstare(FILE *output, char *charname, double fwidth) {
	long numerwidth, denomwidth;
	struct ratio rwidth;

	numencode(0); 				/* side bearing x */
/*  deal with non-standard font matrix problem ? */
/*	fwidth = (double) fwidth * (double) denomscale / (double) numerscale;  */
	fwidth = (double) fwidth * (double) denomold / (double) numerold; 
	if (avoiddiv != 0) numencode(round(fwidth)); 
	else {
		rwidth = rational(fwidth, (long) NUMLIM, (long) DENLIM); 
		numerwidth = rwidth.numer; denomwidth = rwidth.denom;
		lnumencode(numerwidth);
		if (denomwidth != 1) {
			lnumencode(denomwidth);
			code_div(); /* compute width */
		}
	}
	code_hsbw();
	code_endchar();
	endstring(output, charname, -1);
}

void doblank(FILE *output, char *charname) { /* make .notdef or space */
	int k;
	double fwidth;

	charptr = charstring; 		/* reset buffer pointers here */

/*	first try and find character code from encoding so can get width */
	if (strcmp(charname, ".notdef") == 0) {		/* flag = 0 - for notdef */
		if (remapflag) chrs = 129;	else chrs = 128;
	}
	else if (strcmp(charname, "space") == 0) {		/* for space */
		if (remapflag) chrs = 160; else chrs = 128;
	}
	else if (strcmp(charname, "nbspace") == 0) {	/* for nbspace */
		if (remapflag) chrs = 160; else chrs = 128;
	}
	else if (strcmp(charname, "cwm") == 0) {		/* for cwm */
		if (remapflag) chrs = 160; else chrs = 128;
	}
	else if((k = lookup(charname, chrs)) < 0) {
		fprintf(errout, "ERROR: Can't find `%s' in encoding\n", charname);
		fatal++;
		showencoding();
		return;
	}
	else chrs = k;

	fwidth = 300.0;				/* default size of /.notdef space */
/*	if (strcmp(charname, ".notdef") == 0 || strcmp(charname, "space") == 0) {*/
	if (strcmp(charname, ".notdef") == 0 ||
		strcmp(charname, "space") == 0 ||
		strcmp(charname, "nbspace") == 0 ||
		strcmp(charname, "cwm") == 0) {
/*		printf("spacewidth %lg\n", spacewidth); */
/*		might want to reconsider all this if spacewidth = 0 ? */
		if (spacewidth >= 0.0) fwidth = spacewidth;
/*		rest of this is a bit silly - space width SHOULD be in AFM */
		else if ((k = lookup("space", 32)) >= 0) fwidth = widths[k];
		else if (widths[160] != 0.0 && strcmp(encoding[160], "") != 0)
			fwidth = widths[160];	/* width of space from AFM ? */
		else if (widths[128] != 0.0 && strcmp(encoding[128], "") != 0)
			fwidth = widths[128];	/* width of space from AFM ? */
		else if (typeflag != 0 && strcmp(encoding[32], "") != 0)
			fwidth = widths[32];   /* width of visible space for typewriter */
		else if ((romanflag != 0 || italicflag != 0) 
			&& strcmp(encoding[123], "") != 0)
				fwidth = widths[123]; 	/* width of en-dash TeX encoding */
		else if (widths[177] != 0.0 && strcmp(encoding[177], "") != 0) 
			fwidth = widths[177];	/* width of en-dash in StandardEncoding */
		if (strcmp(charname, "cwm") == 0) fwidth = 0.0;	/* compound word mark */
/*		if (fwidth < 200.0) fwidth = 200.0;			
		else if (fwidth > 1000.0) fwidth = 1000.0;  */
/*		if (verboseflag != 0 && strcmp(charname, "space") == 0) */
		if (verboseflag != 0 && strcmp(charname, ".notdef") != 0) 
			printf("\nUsing %lg for `%s' width ", fwidth, charname);
	}
	else fwidth = widths[lookup(charname, chrs)];
	blankstare(output, charname, fwidth);
}

/* create CharStrings for those items in encoding not yet taken care of */

void domissing(FILE *output) {
	int k;
	for(k = 0; k < MAXCHRS; k++) {
		if (chardone[k] == 0 && strcmp(encoding[k], "") != 0) {
			if (traceflag != 0 || strstr(encoding[k], "space") == NULL)
				fprintf(errout, " Making `%s' ", encoding[k]);
			doblank(output, encoding[k]);
		}
	}
}

/****************************************************************************/

/* check whether this is a bogus curveto - hint replacement Subr call */
/* if so, replace with single nsubr, nsubr, `S' code */

/* should hint changing be allowed between `h' and `m' ??? */

void checksubr(void) {
	int nsubr;
	nsubr = knots[knot-1].x;
	if (knots[knot-1].y == nsubr &&
		knots[knot-2].x == nsubr &&
		knots[knot-2].y == nsubr &&
		knots[knot-3].x == nsubr &&
		knots[knot-3].y == nsubr) {
			if (largescale != 1) {	/* undo scaling ! */ /* 1992/Aug/28 */
				nsubr = lastnumber;	/* need unscaled version */
				knots[knot-1].x = nsubr;
				knots[knot-1].y = nsubr;
				knots[knot-2].x = nsubr;
				knots[knot-2].y = nsubr;
				knots[knot-3].x = nsubr;
				knots[knot-3].y = nsubr;
			}
			knots[knot-1].code = ' ';	/* get rid of 'C' here */
			knot = knot - 2;
			knots[knot-1].code = 'S';
			nsubrs++;
	}
}

/* process character outlines */

void dooutlines(FILE *input, FILE *output, FILE *hinting) {
	int c, width, num, xnext, nchrs;
	char comname[MAXCHARNAME];			/* new */
	long currentinput=0, currenthint=0;	/* 96 Aug 6 */
	int hyphenflag=0;					/* 96 Aug 6 */
	char *s;							/* 96 Aug 6 */
	
	if (sfthyphenflag) sfthyphenhit=0;

	if (texfont != 0) {		/* where to put unencoded characters ? */
		if (remapflag) unchrs = 160;	/* if at all ? */
		else unchrs = 128;					/* if at all ? */
	}
	else unchrs = 0;

	task = "working on character outlines";
	if (traceflag) printf("%s\n", task);
/*	if (verboseflag != 0) putchar('\n'); */

/*	if (notdefflag != 0) doblank(output, 0); */
	nchrs = 0;
/*	for (chrs = 0; chrs < fontchrs; chrs++) { */ /* do fontchrs characters */
	for(;;) {
		if (sfthyphenflag) {
			currentinput = ftell(input);
			currenthint = ftell(hinting);
		}
		task = "searching for character string";
		if (traceflag) printf("%s\n", task);
		if (traceflag) fflush(stdout);

/*		hintedflag = 0;	 */		/* NEW */

/*		new stuff to scan in character name comment */
		comname[0] = '\0';
		c = getc(input);
		while (c != EOF && c != '%' && c != 'd') c = getc(input);
		if (c == EOF) break;		/* reached the end */
		else if (c == '%') {
			if (fscanf(input, "%s", comname) == 0) comname[0] = '\0';
			c = getc(input);
			while (c != EOF && c != '\n' && c != 'd') c = getc(input);
			ungetc(c, input);
		}
		else ungetc(c, input);
		if (c == EOF) break;		/* reached the end */

		if (fscanf(input, " dup %d", &chrs) != 1) { /* read character code */
			if (redoflag == 0) { /* nchrs < fontchrs && */
				if (verboseflag != 0)
					printf("\nRead %d CharDefs ", nchrs); /* \n ? */
			}
			break;						/* reached the end */
		}

		if (noafmflag) {				/* 97/Oct/23 */
			if (chrs >= 0) {
				strcpy(encoding[chrs], comname);
			}
		}

		if (redoflag != 0 && chrs >= 0) { /* && redoflag == 0 ??? */
			if (strcmp(comname, "") != 0 &&
				strcmp(encoding[chrs], comname) != 0) {
				fprintf(errout, 
					"ERROR: Name mismatch (%d) - encoding: `%s' outline: `%s'\n",
						chrs, encoding[chrs], comname);
				fatal++;
				fflush(errout);
				_getch();
			}
		}

		if (sfthyphenflag) {
			if (strcmp(comname, "sfthyphen") == 0) 
				fprintf(errout,	"ERROR: sfthyphen already exists!\n");

			if (strcmp(comname, "hyphen") == 0) {
				if (hyphenflag < 0) {
/*					strcpy(comname, "sfthyphen"); */
					sfthyphenhit++;			/* make note that we did it */ 
				}
				hyphenflag++;				/* reset to zero if -1 */
			}
		}

		if (chrs < 0) {				/* unencoded characters in control ? */
			fprintf(errout, "Character code %d negative\n",	chrs);
			chrs = unchrs; unchrs++; 
			if (unchrs == 32) unchrs = 128;
			else if (unchrs == 160) {
				fprintf(errout, "ERROR: Overflowed unencoded character space");
				fatal++;
				unchrs--;
			}
		}
		else if (chrs > fontchrs) {
			fprintf(errout, "Character code %d out of range (HEX)\n", chrs);
			giveup(13);
		}
		nchrs++;

		if (traceflag) printf("Starting to scan again\n");
		if (traceflag) fflush(stdout);

		c = ngetc(input);			/* NEW - based on warning in lint */
		while (c != '<') c = ngetc(input); /* scan for hex field */

		if (verboseflag != 0 && (showcharbbox == 0 || redoflag == 0)) {
			if (chrs % 64 == 0 && chrs != 0) putchar('\n');
#ifdef SNAPTOWIDTH 
			if (redoflag != 0 || widthflag != 0) printf(".");
#else
			printf(".");
#endif

/*			printf("%d\n", chrs);  */
		}

		knot = 0;		/* index into array of numbers accumulated */
		xnext = 1; 		/* first coordinate will be an x */
		nsubrs = 0;		/* no hint replacement subrs seen yet */
		hintedflag = 0;	/* no hints seen so far (hence no need for replace) */

		task = "reading width from CharDef";
		if (traceflag) printf("%s %d\n", task, chrs);

		width = gobblenumber(input); 	/* get width - no scaling here */

/*		an experiment 97/Aug/27 */
/*		assumes widths in font are scaled down - not final AFM widths */
/*		if (numerscale != 1 || denomscale != 1)	width = scalenum(width); */

		charptr = charstring; 		/* reset buffer pointers here */

/* 		if two pass are used, can get real sbx */		
		if (redoflag == 0) {		/* not first pass */
			if (onepass == 0) {		/* second pass in two pass operation */
/*				startstring(sbx[chrs], width); */
				startstring(sbx[chrs], width, comname); 
			}
			else {	/* first pass in one pass operation - use sbx = 0 */
/*				startstring(0, width); */
				startstring(0, width, comname);
			}
			if (hintsflag != 0) copycharhints(hinting, chrs, -1);
		}
		else {						/* first pass of two pass operation */
			sbx[chrs] = 2000;		/* special marker ? */
		}
/*		character bounding box */
		cxmin = 10000; cymin = 10000; cxmax = -10000; cymax = -10000;

		task = "processing character outline";
		if (traceflag) printf("%s %d\n", task, chrs);

		c = ogetc(input);
		while (c != '>') {
			if ((c >= '0' && c <= '9') || c == 'B') {
				ungetc(c, input);
				num = scalenum(gobblenumber(input));
				if (xnext != 0) {
					knots[knot].x = num;
					xnext = 0;
				}
				else {
					knots[knot].y = num;
					xnext = 1;
					knots[knot].code = ' ';	
					knot++;
					if (knot > MAXKNOTS) {
						fprintf(errout, "Too many knots %d", knot);
						giveup(7);
					}
				}
			}
			else {
				if (c != 'A' && knot == 0 ) { /* flush this test - space */
					if (traceflag != 0)
						fprintf(errout,
							"Subpath does not start with coordinates %c", c);
/*					giveup(7); */
				}
/* most of the action is actually in endsubpath ! */
				switch(c) {
					case 'D': endsubpath(); break; /* GO OFF AND REVERSE IT */
					case 'F': startsubpath(); break; /* GO START A SUBPATH */
					case 'E': knots[knot-1].code = 'E'; break; /* lineto */
					case 'C': knots[knot-1].code = 'C';  
							checksubr(); break; /* curveto */
					case 'A': break;			/* i.e. blank */
					default: {
						fprintf(errout,
							"Error: char %c not an A (blank)", c);
						giveup(3);
					} break;
				}
			}
			c = ogetc(input);
		}
 /*		catch open paths (as in space character) */
		if (closed == 0) { 
			if (redoflag == 0) {
				fprintf(errout, "ERROR: Subpath in char %d not closed\n", chrs);
				fatal++;
			}
			if(knot == 0) {
				sbx[chrs]= 0;	/* checkarr(0, 1); */
			}
			else checkarr(0, knot);  /* update xmin, ymin, xmax, ymax */
		}
		if (sbx[chrs] == 2000) sbx[chrs] = 0;	/* no outlines ? */
		while ((c = ngetc(input)) <= ' ') ; /* scan over white space if any */
		ungetc(c, input);
		fscanf(input, "put");				/* scan over "put" */
		charseen++;
		if (sfthyphenflag) {		/* don't count both hyphen & sfthyphen */
			if (hyphenflag) charseen--;
		}
		chardone[chrs]++;					/* mark character as done */
		if (traceflag) printf("Done with %d\n", chrs);

		if (redoflag == 0) {
			code_endchar();
			s = encoding[chrs];
			if (sfthyphenflag) {
				if (hyphenflag) s = "sfthyphen";
			}
/*			endstring(output, encoding[chrs], -1); */ 	/* finish outline  */
			endstring(output, s, -1); 	/* finish outline  */
		}
		else if (showcharbbox != 0)
			printf("C %d ; WX %lg ; N %s ; B %d %d %d %d ;\n",
				chrs, widths[chrs], encoding[chrs], 
					cxmin, cymin, cxmax, cymax);
		if (sfthyphenflag) {
			if (hyphenflag > 0) {	/* reset and do a second time */
				fseek(input, currentinput, SEEK_SET);
				fseek(hinting, currenthint, SEEK_SET);
				hyphenflag = -1;		/* so its reset to 0 above */
			}
		}
		else hyphenflag = 0;
		if (traceflag) fflush(stdout);
	} /* end of for (;;) */

	if (traceflag) printf("popped out of charstring reading loop");

/* have now processed all character outlines in the HEX source file */
	if (sfthyphenflag) {
		if (sfthyphenhit == 0) fprintf(errout, "ERROR: did not find hyphen\n");
	}

/* maybe do this AFTER generating composites ??? */ /* 1993/April/26 */
/*	if (redoflag == 0) {
		task = "inserting /.notdef CharString";
		if (notdefflag != 0) doblank(output, ".notdef");
	} */
	if (redoflag == 0) {
		task = "inserting /space CharString";
		if (traceflag) printf("%s\n", task);
		if (spaceflag != 0 && lookup("space", 32) < 0)
			doblank(output, "space");
		if (nbspaceflag != 0 && lookup("nbspace", 160) < 0)
			doblank(output, "nbspace");
		if (cwmflag != 0 && lookup("cwm", 160) < 0)
			doblank(output, "cwm");
/*		if (sfthyphenflag != 0 && lookup("sfthyphen", 173) < 0)
			doblank(output, "sfthyphen"); */
		domissing(output);		/* do characters without outlines */
	}
}

/* still available flags: g, n, o */

int decodeflag (int c) {
/*	printf ("FLAG: %c%n", c); */
	switch(c) { /* NOTE: e => c and c => x. o is 'normal' type 1 font */
		case '?': detailflag = 1; return 0;
		case 'v': verboseflag = (1 - verboseflag); return 0;
		case 'V': suppressvstem3 = (1 - suppressvstem3); return 0;
		case 'H': suppresshstem3 = (1 - suppresshstem3); return 0;
		case 't': traceflag = (1 - traceflag); return 0;
		case 'g': statisticsflag = (1 - statisticsflag); return 0;
		case 'j': suppresscontrol = (1 - suppresscontrol); return 0;
		case 'u': uppercaseflag = (1 - uppercaseflag); return 0;
		case 'b': spaceflag = (1 - spaceflag); return 0;
		case 'D': dcaddflag = (1 - dcaddflag); return 0;
		case 'r': reverseflag = 1; return 0;
		case 'l': remapflag = 1; return 0;
		case 'f': spikeflag = 0; return 0;
		case 'F': ignorefatal = 1; return 0;	/* 97/June/10 */
		case 'q': originflag++; return 0;		/* whose copyright to use */
		case 'k': texfont = 1; return 0;
		case 'm': forcesubrs = 1; avoiddiv = 1; return 0; /* flush */
		case 'z': extrachar = 0; return 0;
		case 'n': checkhexwidths = 1; return 0;	/* use HEX with, not AFM */
		case 'o': showcharbbox = 1; return 0;
		case 's': correctscale = 1; return 0;
		case 'p': avoidlargescale = 1; return 0;
		case 'd': dontrescale = 1; return 0; 
		case 'U': wantuniqueid = 0; return 0;			 /* 94/Jul/18 */
/*		case 'E': switchtrick = 0; return 0;  */
		case 'E': errout = stderr; return 0;			/* 97/Sep/25 */
/*		case 'C': forceboiler = 0; return 0; */	/* 92/Oct/8 */
		case 'C': forceboiler = 0; return 0;	/* 92/Oct/8 */
		case 'T': forcesubrs = 0; return 0;		/* 92/Oct/8 */
		case 'O': forceothersubrs = 0; return 0;	/* 93/Sep/17 */
		case 'I': insideonly = 1; return 0;			/* 94/Feb/24 */
		case 'S': relaxseac = 1; return 0;		/* 96/Jun/30 */
/*		case 'g': atmflag=0; return 0; */
/*		case 'n': wantcomposites = 1; return 0;  */
/*		case 't': onepass = 1; return 0; */
/*		case 'n': numcodchrs = 1; return 0; */
/*		case 'd': download = 0; return 0; */
/*	following have control flowing into one another: */ /* defunct ? */
#ifdef OLDSTUFF
		case 'a': relatflag = 0;
		case 'i': charencode = 0; 
		case 'c': charencrypt = 0;
		case 'y': charbinflag = 0;
		case 'x': eexecencrypt = 0; return 0;
#endif
/* following take arguments */
		case 'h': hpathflag = 1; break;
		case 'w': wpathflag = 1; break;
/*		case 'e': extenflag = 1; break; */
/*		case 'p': opathflag = 1; break; */
		default: {
			if (c >= '0' && c <= '9') {
				originflag = c - '0';
				return 0;
			}
			else {
				fprintf(errout, "Invalid command line flag '%c'", c);
				giveup(7);
			}
		}
	}
	return -1;		/* need argument */
}

/* 't' => trust given FontBBox and sbx (no two pass operation) */
/* 'n' => use numeric codes in CharStrings instead of names */
/* 'm' => avoid use of div operator before hsbw */
/* 'm' => use non-standard FontMatrix when FontBBox is large OLD */
/* 's' => undo old BSR scaling (only need for BSR release 0.8 files) */
/* 'z' => save four bytes per character (assumes PS interpretor > 23.0) */
/* 'd' => do not insert downloading header */
/* 'f' => do not remove dangerous spikes (to avoid `epaulets') */

/* \tx: no eexec encryption ==> removes outer layer of encryption\n\ */
/* \ty: no eexec encryption and use hex format for CharStrings\n\ */
/* \tc: no charstring encryption ==> removes inner layer of encryption\n\ */
/* \ti: no encoding of charstrings ==> show implicit BuildChar commands\n\ */
/* \ta: no use of implicit BuildChar commands ==> show absolute PS commands\n\ */

/* \tg: make output ATM compatible (also implies n)\n\ */
/* \tn: make composite characters based on AFM description\n\ */

/* Correct usage is:\n\ */

void showusage(char *s) {
	fprintf (errout, "\
%s [-{v}{k}{u}{b}{j}{l}{m}{g}{r}{d}{n}{q}{0=9}]\n", s);
	fprintf (errout, "\
\t[-w <wpath>] [-h <hpath>]  <file-1> <file-2> ...\n");
	if (detailflag == 0) exit(1);
	fprintf (errout, "\
\tv: verbose mode\n\
\tk: treat as TeX font (also implies u, b) may also want l\n\
\t\tu: convert font name to upper case (toggle)\n\
\t\tb: insert code for /space (toggle)\n\
\tj: suppress use of control char positions in encoding (toggle)\n\
\tl: remap encoding vector elements (from 0 - 31 to above 160)\n\
\tm: neuter font so it will work on NewGen (avoid div before hsbw)\n\
\tg: show statistics at end\n\
\tr: reverse paths (only need for older PS CM outlines)\n\
\td: do not rescale input coordinates (use as is)\n\
\tp: allow rescaling if coordinates large than 2000\n\
\tn: ignore widths in HEX file (suppress warnings)\n\
\tS: allow SEAC calls to chars not in ASE\n\
\to: only show character BBox (for debugging AFM files)\n\
\tq: 0 => Y&Y, 1 => BSR, 2 => AT&T, 3 => B&H, 4 => AMS, 5 => MT, 6 => Wolfram\n\
\tV: convert vstem3 into 3 vstem hints\n\
\tH: convert hstem3 into 3 hstem hints\n\
\tw: next argument is path for AFM (or WID) character width files\n\
\th: next argument is path for HNT character hinting files\n");
/*	fprintf(errout, "\
\te: next argument is extension to use on output file (default '%s')", 
	ext); */
	exit(1);
}

/* \tC: use char codes in font instead of ASE codes for SEAC calls\n\ */

/* \t   (-q => BSR CM,  -qq => PS AT&T, -qqq B & H, -qqqq AMS fonts)\n\ */

/* \tp: next argument is path for output files (default current directory)\n\ */

/* [-p <path>] [-e <ext>] */

void showstatistics(void) {
	printf( "Processed %d moveto, %d lineto, %d curveto, %d closepath\n",
				mcount, lcount, ccount, hcount);
	printf( "Longest subpath in character %d has %d knots\n",
		maxkchrs, maxknts);
	printf( "Longest charpath is for character %d and has %d bytes\n",
		maxechrs, maxebytes);
	if (hintsubrs > 0)
		printf ( "Seen %d hint replacement subrs\n", hintsubrs); 
	if (snapped > 0)
		printf ( "Adjusted %d character widths\n", snapped);
	if (flexprocs > 0)
		printf ( "Flex Proc possibilities %d places\n", flexprocs);
	if (tripleknots > 0)
		printf ( "Triple knots in %d places\n", tripleknots);
	if (spikes > 0)
		printf( "Corrected %d spike overshoots\n", spikes);
	if (reordered > 0)
		printf ( "Reordered %d subpaths\n", reordered);
	if (avoided > 0)
		printf ( "Removed %d redundant rlineto's\n", avoided);
	if (shortcurve > 0)
		printf ( "Converted %d tiny rcurveto's to rlineto's\n",	shortcurve);
	if (shortline > 0)
		printf ( "Removed %d tiny rlineto's\n", shortline);

#ifdef DEBUG
	printf ( "Shortest rlineto %6.3f in char %d, ",
		sqrt((double) minlsq), minlchrs); /* remaining */
	printf ( "rcurveto %6.3f in char %d\n",
		sqrt((double) mincsq), mincchrs);
	putchar('\n');
#endif
}

#ifdef CONTROLBREAK
void cleanup(void) {
	if (fp_out != NULL) {
		fclose(fp_out);		/* close output */
		(void) remove(fn_out);		/* remove bogus file */
	}
}
#endif

#ifdef CONTROLBREAK
/* not clear whether the C style I/O calls (in cleanup) are safe ... */
/* maybe just set flags and then do this at higher level ? */
/* void ctrlbreak(void) { */
void ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
	cleanup();
	if (traceflag != 0) fprintf(errout, "\nUser Interrupt\n"); 
	exit(3); 
/*	(void) signal(SIGINT, ctrlbreak); */
}
#endif

void checksubruse(void) {
	int k;
	for (k = 0; k < hintsubrs; k++) { 
		if (subrused[k] == 0) {
			fprintf(errout, 
				"ERROR: Subr code %u for char %u was never called\n", 
					subrcode[k], subrchar[k]);
/*			fatal++; */	/* ??? */
		}
	}
}

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *extractfilename(char *pathname) {
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

	s = extractfilename(filename);
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

int doonefile(char *argv[], int m) {
	int k, forcepass;
	int renameflag=0; 
	char *s;

	subroffset = 4;	if (switchtrick != 0) subroffset++;

	if (verboseflag != 0) putc('\n', stdout);	/* ??? */
	task = "initializing encoding vector";
	if (traceflag) printf("%s\n", task);
/*	for (k = 0; k < fontchrs; k++)   
		strcpy(encoding[k], defaultencoding[k]); */

/*	if (wantcomposites != 0) {
		for (k = 0; k < 16; k++)
			strcpy(encoding[k + 192], standardaccents[k]);
		strcpy(encoding[245], "dotlessi");
	} */

	for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], ""); /* ??? */

	uniqueid = 0;

	strcpy(fn_in, argv[m]);			/* copy in next input file name */
		
/*	new stuff - see if file is there - deal with underscore */
	extension(fn_in, "hex");
/*	if ((fp_in = fopen(fn_in, "r")) == NULL) { */
	if ((fp_in = fopen(fn_in, "rb")) == NULL) {
		underscore(fn_in);
/*		if ((fp_in = fopen(fn_in, "r")) == NULL) { */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {
			perror(fn_in); return -1;
		}
		else fclose(fp_in);
	}
	else fclose(fp_in);

	strcpy(hexpath, argv[m]);
	stripname(hexpath);
	if (strcmp(wpath, "") == 0) {	/* 1992/Aug/29 */
		wpath = hexpath;
		printf("NOTE: Using HEX file path for AFM file path\n");
	}
	if (strcmp(hpath, "") == 0) {	/* 1992/Aug/29 */
		hpath = hexpath;
		printf("NOTE: Using HEX file path for HNT file path\n");
	}
	
	task = "opening width file";
	if (traceflag) printf("%s\n", task);
	
	largescale = 1;						/* default */

	widthflag = 0;
/*	if (strcmp(wpath, "") != 0) {  */
	task = "reading character width info";
	if (traceflag) printf("%s\n", task);
	strcpy(fn_afm, wpath);
	if (strcmp(fn_afm, "") != 0) strcat(fn_afm, "\\");	/* 92/Sep/29 */
	if ((s = strrchr(fn_in, '\\')) == NULL) {
		if ((s = strrchr(fn_in, ':')) == NULL) s = fn_in;
		else s++;
	} else s++;
	strcat(fn_afm, s);
	forceexten(fn_afm, "afm");		/* try AFM first */
	noafmflag = 0;					/* 1997/Oct/23 */
/*	if ((fp_afm = fopen(fn_afm, "r")) == NULL) { */
	if ((fp_afm = fopen(fn_afm, "rb")) == NULL) {
		forceexten(fn_afm, "wid");	/* try WID if no AFM file */
/*		if ((fp_afm = fopen(fn_afm, "r")) == NULL) { */
		if ((fp_afm = fopen(fn_afm, "rb")) == NULL) {
			fprintf(errout, 
					"WARNING: Can't open WID or AFM file: %s!\n", fn_afm);
			fprintf(errout, 
					"Will try and get all information from HEX file\n");
/*			we now allow this - get info frmo hex file 97/Oct/23 */
/*			fprintf(errout, 
			   "Need encoding - skipping file: %s!\n", fn_in); */
/*	in this case maybe copy standard encoding vector in ? */
/*	or just get it from the hex file ??? */
		   noafmflag = 1;
/*		   usehexbbox = !usehexbbox; */
		   usehexbbox = 1;		/* 98/Jul/22 */
		   for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], ""); /* ? */
/*		   fatal++; */
/*		   (void) _getch(); */
/*		   return -1; */
		}					
		else {						/* found a .WID file */
			if (verboseflag != 0)
				printf ("Reading WID file: %s\n", fn_afm);
			readwidths(fp_afm);	fclose(fp_afm);	widthflag = 1;
		}
	}
	else {							/* found an .AFM file */
		if (verboseflag != 0) printf ("Reading AFM file: %s\n", fn_afm);
		for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], ""); /* ? */
		readafmfile(fp_afm);
		fclose(fp_afm);
		widthflag = 1;
	}
/*	}
	else { 
		fprintf(errout, "No AFM or WID file specified!\n");
		fprintf(errout, 
			"Need encoding - skipping file: %s!\n", fn_in);
		(void) _getch();
		return -1;	
	} */
/*	this means we will miss the encoding - which is now required ! */

	task = "creating output font file name";
	if (traceflag) printf("%s\n", task);

	if (strcmp(opath, "") != 0) {
		strcpy(fn_out, opath); strcat(fn_out, "\\");
	}
	else strcpy(fn_out, "");

/*  following already done above */
/*	strcpy(fn_in, argv[m]); */
/*	extension(fn_in, "hex"); */

	if ((s=strrchr(fn_in, '\\')) != NULL) s++;
	else if ((s=strrchr(fn_in, ':')) != NULL) s++;
	else s = fn_in;
	strcat(fn_out, s);  	/* copy input file name minus path */

	forceexten(fn_out, ext);	/* change extension */
	if (traceflag) printf("Output file name: %s\n", fn_out);
	if (strcmp(fn_in, fn_out) == 0) { /* check in file = out file */
		makename(fn_out, ext);  /* make unique temporary name */
		renameflag = 1;			/* need to rename at end */
	}

#ifdef WANTBACKUP
/*	if ((output = fopen(fn_out, "r")) != NULL) {  */
	if ((output = fopen(fn_out, "rb")) != NULL) { 
		fclose(output);
		strcpy(fn_bak, fn_out);
		forceexten(fn_bak, "bak");
		remove(fn_bak); 		/* in case backup already exists */
		rename(fn_out, fn_bak); /* ignore error in renaming */
	}
#endif
	printf( "Processing font file: %s\n", fn_in);
	
/*	if (onepass == 0) forcepass = 1; else forcepass = 0; */
	forcepass = (onepass == 0) ? 1 : 0;
/*	forcepass = twopass;  */
	frezflag = 0;
	charcount = 0;					/* number of characters	processed */
	charseen = 0;
	for (k = 0; k < MAXCHRS; k++) chardone[k] = 0;
	correctfontscale = correctscale;		/* copy in correction flag */
	xmin = 10000; ymin = 10000; xmax = -10000; ymax = -10000;

#ifdef DEBUG
	minlsq = 10000L; mincsq = 10000L;
#endif
	if (texfont != 0) capsulestart = cmcapsule;
	else capsulestart = nontexcapsule;
/*	if (originflag == 0) capsulestart = mycapsule; */
	if (originflag == 0) capsulestart = nontexcapsule;
	else if (originflag == 1) capsulestart = cmcapsule;
	else if (originflag == 2) capsulestart = attcapsule;
	else if (originflag == 3) capsulestart = bahcapsule;
	else if (originflag == 4) capsulestart = eulercapsule;
	else if (originflag == 4) capsulestart = texcapsule;	
	else if (originflag == 5) capsulestart = "";	/* use random bytes */
	else if (originflag == 6) capsulestart = nontexcapsule;	
	else if (originflag == 7) capsulestart = pluscapsule;	

	capsuleptr = capsulestart; 			/* initialize capsule pointer */

	/* This shouldn't get into an infinite loop (we hope) */

	do { /* while (redoflag != 0) */
		redoflag = 0;
		if (forcepass != 0) { /* check whether forced to do two passes */
			forcepass = 0; redoflag++;
		}
		if (redoflag == 0) {
			task = "opening hint file";
			if (traceflag) printf("%s\n", task);
			hintsflag = 0;
/*			if(strcmp(hpath, "") != 0) { */
			task = "reading character hinting info";
			if (traceflag) printf("%s\n", task);
			strcpy(fn_hnt, hpath);
			if (strcmp(fn_hnt, "") != 0) strcat(fn_hnt,	"\\"); /* ? */
/*			if ((s = strrchr(argv[m], '\\')) == NULL) {
				if ((s = strrchr(argv[m], ':')) == NULL) s = argv[m];
				else s++;
			} else s++;
			strcat(fn_hnt, s); */
			if ((s = strrchr(fn_in, '\\')) == NULL) {	/* 92/Nov/21 */
				if ((s = strrchr(fn_in, ':')) == NULL) s = fn_in;
				else s++;
			} else s++;
			strcat(fn_hnt, s);		
			forceexten(fn_hnt, "hnt");	
/*			if ((fp_hnt = fopen(fn_hnt, "r")) == NULL) { */
			if ((fp_hnt = fopen(fn_hnt, "rb")) == NULL) {
				forceexten(fn_hnt, "stm");
/*				if ((fp_hnt = fopen(fn_hnt, "r")) == NULL) { */
				if ((fp_hnt = fopen(fn_hnt, "rb")) == NULL) {
					fprintf(errout, 
							"WARNING: can't open HNT or STM file: %s\n",
							fn_hnt);
/*					fatal++; */ /* ??? */
				}
				else {
					fprintf(errout, "Using STM file: %s\n", fn_hnt);
					hintsflag = 1;
				}
			}
			else hintsflag = 1;
/*			}
		else fprintf(errout, "WARNING: No hinting file specified\n");	*/
		}

		task = "opening input font file";
		if (traceflag) printf("%s\n", task);
/*		renameflag = 0; */

/*		following already done above ??? */
/*		strcpy(fn_in, argv[m]);  */
/*		extension(fn_in, "hex"); */
/*		if ((fp_in = fopen(fn_in, "r")) == NULL) { */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {
			putc('\n', errout); perror(fn_in); giveup(2);
		}

		task = "opening output font file";
		if (traceflag) printf("%s\n", task);
		if (redoflag == 0) {
/*			if ((fp_out = fopen(fn_out, "w")) == NULL) { */
			if ((fp_out = fopen(fn_out, "wb")) == NULL) {
				putc('\n', errout); perror(fn_out);	giveup(2);
			}
		}

#ifdef DEBUGFLUSH
		setbuf(fp_out, NULL); 
#endif

		if (traceflag != 0) printf("Opened files\n"); 

		if (verboseflag != 0) {
			printf("In: %s, Out: %s", fn_in, fn_out);
			if (hintsflag != 0 && redoflag == 0)
				printf(", Hint: %s", fn_hnt);
			putchar('\n');
		}

/*		reset statistics and extreme values */
		avoided = 0; reordered = 0; snapped = 0; spikes = 0;
		shortline = 0; shortcurve = 0; tripleknots = 0; flexprocs = 0;
		maxknts=0; maxebytes = 0;
		mcount=0; lcount=0; ccount=0; hcount=0;  /* count of commands */

		task = "doing header";
		if (traceflag) printf("%s\n", task);

		strcpy(fontname, "");				/* 98/May/15 */
		if (traceflag) printf("%s\n", task);		
		doheader(fp_in, fp_out, fp_hnt);  /* go deal with the header */

		task = "doing outlines";
		if (traceflag) printf("%s\n", task);		

		dooutlines(fp_in, fp_out, fp_hnt);	/*  deal with body of file */

		task = "closing files";
		if (traceflag) printf("%s\n", task);		

/*		if (verboseflag) printf("HINT FILE NUMBER: %d\n", fileno(fp_hnt)); */
		if (hintsflag != 0 && redoflag == 0) fclose(fp_hnt); /*	|| */
		fclose(fp_in);

		task = "making composite characters";
		if (traceflag) printf("%s\n", task);

		if (redoflag == 0 && widthflag != 0 && wantcomposites != 0) {
/*			fp_afm = fopen(fn_afm, "r"); */
			fp_afm = fopen(fn_afm, "rb");
			gencomposites(fp_out, fp_afm);
			fclose(fp_afm);
		}
/*		Maybe do this AFTER generating composites ??? */ /* 1993/April/26 */
		if (redoflag == 0) {
			task = "inserting /.notdef CharString";
			if (traceflag) printf("%s\n", task);
			if (notdefflag != 0) doblank(fp_out, ".notdef");
		}
		dotrailer(fp_out);			/* go write trailer of file */

		if (redoflag == 0) {
		if (ferror(fp_out) != 0) {
				putc('\n', errout); 
				fprintf(errout, "Output error ");
				perror(fn_out); giveup(2);
			}
			fclose(fp_out);
			if (showcharbbox != 0) (void) remove(fn_out);
		}

#ifdef DEBUG
		if (verboseflag != 0) printf("Closed files\n"); 
#endif

		if (verboseflag != 0) putchar('\n');
		if (redoflag != 0) {
			if (usehexbbox == 0 && useafmbbox == 0) {
				sxll = xmin; syll = ymin; sxur = xmax; syur = ymax;
			}
			if (verboseflag != 0) printf("Second pass follows:\n");
			frezflag++;
		}
	} while (redoflag != 0);

	if (renameflag != 0) {      /* do only if no error encountered? */

#ifdef DEBUG
		printf( "Renaming `%s' to `%s'\n", fn_out, fn_in);
#endif

		(void) remove(fn_in);          /* then delete input file and */
		(void) rename(fn_out, fn_in);  /* rename output to input name */
	}

	if (traceflag != 0) fprintf(errout, "Now check subr use\n");
	checksubruse();
		
	task = "showing statistics";
	if (traceflag) printf("%s\n", task);

	if (verboseflag != 0) 
		printf( "TrueBBox [%d %d %d %d] (ignoring composites)\n",
			xmin, ymin, xmax, ymax);

	if (verboseflag != 0) {
		printf( "Processed %d characters ", charcount-ncomposed);
		if (spaceflag != 0) printf( "(including space) ");
		if (notdefflag != 0) printf( "(including .notdef) ");
		putc('\n', stdout);
		if (ncomposed > 0)
			printf ( "Created %d composite characters\n", ncomposed);
	}
	if (statisticsflag != 0) showstatistics();
	return 0;
}

/* Flags and Arguments start with `-' */
/* Also allow use of `/' for convenience */
/* Normal use of `=' for command line arguments */
/* Also allow use of `:' for convenience */
/* Archaic: use space to separate - only for backward compatability */

int decodearg(char *command, char *next, int firstarg) {
	char *s;
	char *sarg=command;
	int c;
	
	if (*sarg == '-' || *sarg == '/') sarg++;	/* step over `-' or `/' */
	if (*sarg == '\0') return firstarg+1;
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
			if (wpathflag != 0) {
				wpath = s; wpathflag = 0;
			}
			else if (hpathflag != 0) {
				hpath = s; hpathflag = 0;
			}
			break;	/* default - no flag set */
		}
	}
	return firstarg;
}

/*	check command line flags and command line arguments */

int commandline(int argc, char *argv[], int firstarg) {
/*	int c; */
	
	if (argc < 2) showusage(argv[0]);
	while (firstarg < argc && argv[firstarg][0] == '-') {
		firstarg = decodearg(argv[firstarg], argv[firstarg+1], firstarg+1);
	}
	return firstarg;
}

int main(int argc, char *argv[]) {       /* main program */
/* Command line usage: */
/* fontone [-[v][s][p][f][d][{x|c|r|a}]] */
/* [-e <ext>] [-w <wpath>] [-h <hpath>]	<file-1> <file-2> ... */
	char *s;
/*	unsigned int i; */
	int m;
/*	int c, k, forcepass, renameflag = 0; */
	int firstarg=1;

#ifdef CONTROLBREAK
	(void) signal(SIGINT, ctrlbreak);
#endif

/*	if (setvbuf(stdout, NULL, _IOLBF, 512))	
		printf("Unable to line buffer %s\n", "stdout"); */
/*	if (setvbuf(errout, NULL, _IOLBF, 512))	
		printf("Unable to line buffer %s\n", "errout"); */
/*	setvbuf(errout, NULL, _IONBF, 0); */		/* 97/Sep/13 */

	task = "interpreting command line";
	if (traceflag) printf("%s\n", task);

	chrs = -1; /* prevent character number output in case of error */

	if ((s = getenv("widpath")) != NULL) wpath = s;
	if ((s = getenv("afmpath")) != NULL) wpath = s;
	if ((s = getenv("hntpath")) != NULL) hpath = s;

/*	printf("%d arguments \n", argc); */
/* 	for (i=0; i < argc; i++) printf("%s ", argv[i]); */

	if (argc < 2) showusage(argv[0]);  /* check for no argument case */
		
	printf( "FONTONE outline font conversion program %s\n",	programversion);

	firstarg = commandline(argc, argv, 1); /* check for command flags */

	if (dcaddflag) {	/* use -D to add nbspace and cwm 1996 Aug 6 */
		nbspaceflag = 1; cwmflag = 1; sfthyphenflag = 1;
/*		should check later whether sfthyphen already occurs in encoding! */
/*		if so, reset sfthyphenflag = 0 */
/*		should check later whether hyphen actually occurs in encoding! */
/*		if not, reset sfthyphenflag = 0 */
/*		use this only on text font */
	}

	if (atmflag != 0) {
		numcodchrs = 0; notdefflag = 1; ndgapflag = 1; conform = 1; 
		onepass=0; wantcomposites = 1; /* ? */
	}

	if (texfont != 0) {
		spaceflag = 1 - spaceflag; 
		uppercaseflag = 1 - uppercaseflag;  
/*		suppresscontrol = 1 - suppresscontrol;  */
		suppresscontrol = 0; 
		/* remapflag = 1;  */ /* wantcomposites = 0; */
		fontchrs = TEXCHRS; 
	}

	if (originflag == 0) {
/*		spaceflag = 0; uppercaseflag = -1; remapflag = 0; */
	}
	
/*	if (originflag > 5) randomchar = 1; *//* random starting bytes 92/Apr/6 */
/*	if (originflag > 6) randomchar = 1;	*//* random starting bytes 92/Apr/6 */
	if (originflag > 7) randomchar = 1;	/* random starting bytes 92/Apr/6 */

	if (eexecencrypt == 0) noaccess = 0; 
			/* charbinflag=0; */
	if (charencrypt == 0) {randomchar = 0;	extrachar = 0;}
	if (charencode == 0) {charbinflag = 1;	columns = 78;}

/*	if (verboseflag != 0) {
		if (eexecencrypt == 0) 	printf( "No eexec encrypting");
		else printf( "\nEncrypting eexec");
		if (charbinflag != 0) printf (" - binary charstring");
		else printf (" - hexadeximal charstring");
		if (charencrypt == 0) printf (" - no charstring encrypting");
		else printf (" - encrypting charstring");
		if (charencode == 0) printf (" - no encoding");
		else printf (" - encoding");
		putchar('\n');
	} */

#ifdef DEBUGFLUSH
 	setbuf(stdout, NULL);  
#endif 

	if (firstarg >= argc) showusage(argv[0]); /* left out extension ? */

/* 	charstring = (unsigned char _far *) _fmalloc(CHARSTRINGLEN); */
	charstring = (unsigned char __far *) _fmalloc(CHARSTRINGLEN);
/* 	crypstring = (unsigned char _far *) _fmalloc(CHARSTRINGLEN); */
	crypstring = (unsigned char __far *) _fmalloc(CHARSTRINGLEN);
	if ((charstring == NULL) || (crypstring == NULL)) {
		fprintf(errout, "Unable to allocate memory\n");
		printf("EXITING\n");
		fflush(stdout);
		exit(3);
	}

/*	subroffset = 4;	if (switchtrick != 0) subroffset++; */

	for (m = firstarg; m < argc; m++) { /* do each file in command line */
		fatal = 0;
		(void) doonefile(argv, m);
		if (fatal != 0 && ignorefatal == 0) break;
	}

#ifndef _WIN32
	if ((m = _fheapchk ()) != _HEAPOK) {		/* 1994/Feb/18 */
		fprintf(errout, "WARNING: Far heap corrupted (%d)\n", m);
	}
#endif
	_ffree(charstring);
	_ffree(crypstring);

	if (fatal != 0) {
		printf("FATAL ERROR: EXITING\n");
		fflush(stdout);
		exit(1);		/* 1993/Sep/14 */
	}
/*	if (verboseflag != 0)  */
		if (argc - firstarg > 1) 
/*			printf("Processed %d font files", argc - firstarg); */
			printf("Processed %d font files\n", argc - firstarg);
	return 0;
}

#ifdef IGNORED

/*
 * Adobe Type 1 and Type 2 CharString commands; the Type 2 commands are
 * suffixed by "_2". Many of these are not used for computing the
 * weight vector but are listed here for completeness.
 */
#define HSTEM          1
#define VSTEM          3
#define VMOVETO        4
#define RLINETO        5
#define HLINETO        6
#define VLINETO        7
#define RRCURVETO      8
#define CLOSEPATH      9
#define CALLSUBR      10
#define RETURN        11
#define ESCAPE        12
#define HSBW          13
#define ENDCHAR       14

#define BLEND_2       16
#define HSTEMHM_2     18
#define HINTMASK_2    19
#define CNTRMASK_2    20

#define RMOVETO       21
#define HMOVETO       22

#define VSTEMHM_2     23
#define RCURVELINE_2  24
#define RLINECURVE_2  25
#define VVCURVETO_2   26
#define HHCURVETO_2   27
#define CALLGSUBR_2   29

#define VHCURVETO     30
#define HVCURVETO     31

/*
 * Adobe Type 1 and Type 2 CharString Escape commands; the Type 2
 * commands are suffixed by "_2"
 */
#define DOTSECTION       0
#define VSTEM3           1
#define HSTEM3           2

#define AND_2            3
#define OR_2             4
#define NOT_2            5

#define SEAC             6
#define SBW              7

#define STORE_2          8
#define ABS_2            9
#define ADD_2           10
#define SUB_2           11


#define DIV             12

#define LOAD_2          13
#define NEG_2           14
#define EQ_2            15

#define CALLOTHERSUBR   16
#define POP             17

#define DROP_2          18
#define PUT_2           20
#define GET_2           21
#define IFELSE_2        22
#define RANDOM_2        23
#define MUL_2           24
#define DIVX_2          25
#define SQRT_2          26
#define DUP_2           27
#define EXCH_2          28
#define INDEX_2         29
#define ROLL_2          30

#define SETCURRENTPOINT 33

#define HFLEX_2         34
#define FLEX_2          35
#define HFLEX1_2        36
#define FLEX1_2         37

#endif

/* how serious is that limit of +/- 2000 in coordinates ? NOT VERY */
/* ATM supports +/- 8191 --- Type 1 coprocessor chip supports +/- 4095 */

/* how neccessary are: noaccess, executeonly, readonly ? NOT VERY */

/* use 2X for writehex to speed things up ? */
/* use signal interrupt instead of keyboard hit for user interrupt ? */
/* test value returned by fprintf and putc to catch output errors ? */
/* for binary output, use "wb" in open and set FLUSHCR to 0 ? */
/* count bytes output in eexec encoding so know many zeros REALLY needed ? */
/* check on C-M, C-J sequence -- what does PostScript really need ? */
/* strangle ALL output if redoflag is not zero ? OK */
/* break up main, writeheader, dooutlines - too large for optimization ? */
/* spell out encoding vector line by line to satisfy Adobe ATM spec ? OK */
/* share encoding vector ? rethink naming of characters ? */
/* stick in a CharString for /.notdef, and maybe space ? OK  */
/* systemdict /currentpacking known {currentpacking true setpacking} if ? */
/* systemdict /currentpacking known {setpacking} if ? */
/* add VMUsage line ? OK */
/* to get the widths REALLY right, need to read AFM files OK */
/*  ungetch() type characters if they are not interrupt characters? OK */

/* NOTE: need two pass operation to get sbx for each character OK */
/* NOTE: hint information is relative to sbx - which is now non-zero OK */

/* need to supply /BlueValues if not given in hint file OK */
/* stick in /ForceBold unless already seen in hint file OK */

/* can use environment variables afmpath and hntpath for afm and hint info */

/* NOTE: this computes FontBBox on assumption that knots are well placed */
/* That is, NO knots are outside the bounding box ! *//* Knots at extrema */

/* do short lineto and curveto differently ? */

/* add ATM compatability master switch OK */

/* eliminate dependence on order of characters and on presence of all */

/* NOTE: not used anymore... */
/*  abs encoded output does not work correctly because of xold/yold */
/* 	problems at start up because hstem, vstem set xold, yold by mistake */

/* use other italic angle for slanted fonts OK */

/* provide decrypted output WITHOUT change to hexadecimal form internal OK */ 

/* look for successive anti-parallel segments as well ? */

/* Note: /.notdef has been given half character width - so can use as space */
/* - might want to make something reasonable like size of endash ? */
/* - might want to make equal to size of characters in fixed width font ?  */

/* AFM compatability may mean eexec encryption if wish to avoid binary ? OK */

/* Constraints also from downloader compatability requirement */

/* someday put in hooks for Subrs ? */

/* make note on first pass which characters will be seen and - */
/* - only list those parts of encoding vector on output ? as for logo10 ? */

/* Bury data and time in capsule also ! OK */

/* Note: for old BSR fonts, need to use -r and -s switch ! */

/* Note: for old Projective Solution outlines, need to use -r switch only */

/* make fontchrs be controllable from command line ? */

/* allow  suppresscontrol of 0 -- 31 and 128 -- 159 in encoding vector ? */

/* need to count CharStrings on first pass ? OK */

/* in TeX don't need to insert /.notdef in first 128 characters ? */

/* computation of private directory may be off by one - FIXED */

/* computation of CharStrings directory may be off by one - FIXED */

/* Presently have to stick "unencoded" characters in somewhere in */
/* input file, as if they were encoded - use control character positions */

/* This means the AFM file also has to have them in those positions ... */

/* Limit: 64 unencoded characters placed in 0 - 31 and 128 - 159 */

/* maybe sometime rewrite to treat characters by name, not numeric code */

/* add /Copyright to /FontInfo directory ? */

/* read raw outlines directly - avoid the hex intermediate ? someday ... */

/* add ability to specify output path OK */

/* remember m flag sets avoiddiv flag (and sets forcesubrs)  */

/* forcesubrs adds /Subrs 0 array ND */

/* avoiddiv = 1 round widths to integers - use for NewGen */

/* forcesubrs = 1 for now to increase robustness to lousy PS interpreters */

/* remap: 0-9 => 161-171, 10-31 => 173-194, 32 => 195, 127 => 196, 32 => 128 */

/* when remapped use 160 for space, otherwise use 128 */

/* seems to overallocate CharStrings dictionary space ? FIXED */
/* attempt to fix by not counting those in encoding vector already ? FIXED */
/* possible problem with counting or not counting of space CharString OK */

/* doesn't really work anymore without AFM file specified - need encoding */
/* requires same char count in AFM file and characters in HEX file */
/* use character count in HEX file if AFM file char count disagrees */
/* the old abbrevaited WID files don't really make it anymore... */

/* rescaling not usually needed anymore - taken care off in CONVERT */

/* command line flag to force Bold or force Light ? */

/* using AFM widths, unless HEX widths and AFM widths disagree */

/* maybe mark characters for which outlines have been seen OK */

/* close to being too large for small memory model */
/* maybe flush some tracing printf */
/* maybe compile with some optimizations turned on */
/* to make space, cut out hairy encoding vector hacks OK */

/* NOTE: hint replacement subrs called for in hex file must agree */
/* with replacement hint subrs provided in hint file */

/* possibly allow for comment line in HEX file ? */

/* allow for version to be obtained from HEX file instead of AFM file ? */

/* if hint replacement happens right after moveto for new path => */
/* ... interchange order of hint replacement and moveto */

/* `2 index /CharStrings <n> dict' VERSUS `dup /CharStrings <n> dict' ? */

/* problem with FontBBox if composites are present */
/* use FontBBox from AFM file then ? */

/* nothing means Y&Y fonts, -q means BSR fonts, -qq means AT&T fonts, */
/* -qqq means B&H Lucida fonts -qqqq means Euler fonts */

/* typical: fontone -vkln -w c:\afm\tex -h c:\cmbkph c:\cmbkph\cmr10.hex */
/* typical: fontone -vklnm -w c:\afm\tex -h c:\cmbkph c:\cmbkph\cmr10.hex NG*/

/* OLD:     fontone -vklqr -w c:\afm\tex -h c:\cm c:\bsr\cmr10.hex */

/* typical: fontone -vklq  -w c:\afm\tex -h c:\bsr c:\bsr\cmr10.hex */
/* typical: fontone -vklqb -w c:\afm\tex -h c:\bsr c:\bsr\cmsy10.hex */
/* typical: fontone -vklqb -w c:\afm\tex -h c:\bsr c:\bsr\cmmi10.hex */
/* typical: fontone -vklqbd -w c:\afm\tex -h c:\bsr c:\bsr\cmex10.hex */

/* typical: fontone -vjqq -w c:\at&t -h c:\at&t c:\at&t\bookregu.hex */

/* need to have all characters encoded for this program */
/* do neccessary reencoding afterwards... */

/* Maybe optionally produce ^M in text section not ^M & ^J */
/* For bug in Corel Draw! */

/* Use 'p' command line fly to allow rescaling for fonts with large chars */

/* this may produce a blank line before 512 zeros if hex ends line ... */

/* always have .notdef CharString last ? */ /* use -| |- and | ??? */
