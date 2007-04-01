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

/* program to make up TFM files from AFM files - needs encoding vector */

/* Revised 1999 June 13 to run in DLL form */

#ifdef _WINDOWS
#define NOCOMM
#define NOSOUND
#define NODRIVERS
#define STRICT
#include <windows.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>				/* for tangent */
#include <malloc.h>
#include <direct.h>				/* for _getcwd() */
#include <signal.h>
#include <assert.h>

#pragma warning(disable:4032)	// different type when promoted
#include <conio.h>
#pragma warning(default:4032)	// different type when promoted


#ifdef _WINDOWS
// We must define MYLIBAPI as __declspec(dllexport) before including
// afmtotfm.h, then afmtotfm.h will see that we have already
// defined MYLIBAPI and will not (re)define it as __declspec(dllimport)
#define MYLIBAPI __declspec(dllexport)
#include "afmtotfm.h"
#endif

#ifdef _WINDOWS
#pragma message("DLL Version")
#else
#pragma message("EXE Version")
// #pragma warning(disable:4127)	// conditional expression is constant
#endif

FILE *errout=stdout;	/* where to send error output 97/Sep/23 */

// #ifdef _WINDOWS
// #define showline(str,flag) ShowLine(str,flag);
// #else
// #define showline(str,flag) {if(flag){fflush(stdout);fputs(str,errout);fflush(errout);}else{fputs(str,stdout);}}
// #endif

#define showline(str,flag) ShowLine(str,flag)

/* #include <search.h> */	/* for qsort ? */

#define CONTROLBREAK

/* #define DEBUGGING */
/* #define USECONTRACTION */
/* #define SHOWBASES */

#define FONTNAME_MAX 100
#define CHARNAME_MAX 100
#define MAXLINE 512
#define MAXCHRS 256

/* #define MAXCHARNAME 25 */
/* #define MAXCHARNAME 26 */ 	/* an experiment */
#define MAXCHARNAME 32

/* #define MAXDISTINCT 256 */	/* limit before TeX 3.0 ? */
/* #define MAXDISTINCT 512 */	/* maximum number of distinct kern values */
#define MAXDISTINCT 1024		/* maximum number of distinct kern values */

#define MAXEXTEN 256			/* max extensible table */

#define ITALICFUZZ 30.0				/* default italic fuzz */

/* #define MAXLIG 256 */
/* #define MAXLIG 2048 */		/* don't know if this is safe ? 93/Nov/23 */
#define MAXLIG 10000			/* go for broke ? 93/Nov/26 */

/* #define MAXKERN 512 */
/* #define MAXKERN 900 */
/* #define MAXKERN 1024 */
/* #define MAXKERN 1200 */	
/* #define MAXKERN 2048 */ 		/* an experiment */
/* #define MAXKERN 10000U */	/* using far memory */

#ifdef _WIN32
#define MAXKERN 16384U
#else
/* A solid limit for MAXKERN is 65536 / sizeof(kernpair) or approx 10922 ? */
#define MAXKERN 10900U			/* using far memory */
#endif

#ifdef _WIN32
#define __far
#define _fmalloc malloc
#define _frealloc realloc
#define _ffree free
#endif

#ifdef _WIN32
#define INIMEMORY 8000U				/* do this in stages ? realloc ? */
#define INCMEMORY 8000U				/* do this in stages ? realloc ? */
#else
#define INIMEMORY 4000U				/* do this in stages ? realloc ? */
#define INCMEMORY 4000U				/* do this in stages ? realloc ? */
#endif

#define SHOWTEXBASE  	/* show ligature table debugging information */

/* Following hard-wired by TeX TFM file format */

#define MAXHEIGHTS 16
#define MAXDEPTHS 16
#define MAXITALICS 64

#define XEROXCODEN 10	/* number of words set aside for coding scheme */
#define XEROXFONTN 5	/* number of words set aside for font ID */
						/* may be expanded, but then FaceByte is shifted */

#define NOWIDTH -8000000			/* if no width assigned yet */

#define KERNBIT 128		/* if kern step instead of lig step */
#define STOPBIT 128		/* if last step in lig_kern program */

int fontchrs = MAXCHRS;		/* 256.  Set to 128 if backwardflag non-zero */

FILE *input, *output;

char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];

#ifdef _WINDOWS
char *appname="AFMtoTFM.DLL conversion utility";
#else
char *appname="AFMtoTFM (32) conversion utility";
#endif

#ifdef _WIN32
char *programversion =
/* "AFMtoTFM (32) conversion utility version 3.0"; */ /* 97/Dec/4 */  
/* "AFMtoTFM (32) conversion utility version 3.1"; */ /* 98/Jan/26 */
/* "AFMtoTFM (32) conversion utility version 3.1.2"; */ /* 98/Jun/30 */
/* "AFMtoTFM (32) conversion utility version 3.1.3"; */ /* 98/Aug/15 */
/* "AFMtoTFM (32) conversion utility version 3.1.4"; */ /* 98/Aug/26 */
/* "AFMtoTFM (32) conversion utility version 3.1.5"; */ /* 98/Sep/20 */
/* "AFMtoTFM (32) conversion utility version 3.1.6"; */ /* 1999/Feb/19 change in em_quad */
/* "AFMtoTFM (32) conversion utility version 3.1.7"; */ /* 1999/June/11 change in showline */
/* "version 3.2"; */	/* 1999/June/11 DLL work start */
/* "version 3.2.1"; */	/* 1999/Sep/24 LetraSet modification */
/* "version 3.2.2";	*/	/* em_quad fix when emdash missing 2000/May/18 */
"version 3.2.3";		/* 00/Jun/30 */
#else
char *programversion = 
/* "AFMtoTFM conversion utility version 1.7"; */
/* "AFMtoTFM conversion utility version 1.8"; */
/* "AFMtoTFM conversion utility version 2.0"; */
/* "AFMtoTFM conversion utility version 2.1"; */
/* "AFMtoTFM conversion utility version 2.2"; */	/* 95/Jan/8 */ 
/* "AFMtoTFM conversion utility version 2.3"; */	/* 95/Apr/22 */
/* "AFMtoTFM conversion utility version 2.4"; */	/* 95/Jun/8 */ 
/* "AFMtoTFM conversion utility version 2.5"; */	/* 95/Aug/14 */
/* "AFMtoTFM conversion utility version 2.6"; */	/* 95/Oct/28 */
/* "AFMtoTFM conversion utility version 2.6.1"; */	/* 96/Oct/12 */
/* "AFMtoTFM conversion utility version 2.7"; */	/* 97/Feb/5 */
/* "AFMtoTFM conversion utility version 2.7.1"; */	/* 97/June/5 */
/* "AFMtoTFM conversion utility version 2.8"; */	/* 97/Oct/1 */
/* "AFMtoTFM conversion utility version 2.8.1"; */	/* 97/Oct/19 */ 
/* "AFMtoTFM conversion utility version 2.8.2"; */		/* 97/Oct/28 */
/* "AFMtoTFM conversion utility version 2.9"; */		/* 97/Dec/21 */
"AFMtoTFM conversion utility version 3.0";			/* 98/June/15 */
#endif

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\
Copyright (C) 1990--2000  Y&Y, Inc.  http://www.YandY.com\
";

char *phone="(978) 371-3286";

/* #define COPYHASH 14251529 */
/* #define COPYHASH 2719644 */
/* #define COPYHASH 8124949 */
/* #define COPYHASH 14008178 */
/* #define COPYHASH 3114191 */
/* #define COPYHASH 8997420 */
/* #define COPYHASH 14880649 */
/* #define COPYHASH 95688 */
/* #define COPYHASH 5978917 */
/* #define COPYHASH 10060254 */
/* #define COPYHASH 3104287 */
/* #define COPYHASH 6557415 */
#define COPYHASH 5190533

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

int wantcpyrght=1;		/* want copyright message in there */

/* int usecontraction=0; */	/* use Mac contracted font file names for FontID */
int usecontraction=1;	/* use Mac contracted font file names for FontID */
						/* default changed 98/01/26 since 5+3+3... unique */

int maxcharkern=MAXCHRS;	/* max kern pairs per character */

int verboseflag=0;		/* show progress */
int traceflag=0;		/* only used if DEBUGGING or SHOWTEXBASE defined */
int showkernflag=0;		/* debugging only */
int detailflag=0;
int debugflag=0;		/* DEBUGGING ONLY */
int showital=0;			/* if non-zero, show italic corrections */
int optimize=1;			/* if non-zero check for repeat kern programs */
int showoptimize=0;		/* show ligkern program sharing */
int showovershoot=0;	/* show overshoot correction */
int showreplicate=0;	/* show -FG replication of kern pairs */
int replicateleftflag=0;	/* add composite character kern pairs 97/Oct/19 */
int replicaterightflag=0;	/* add composite character kern pairs 97/Oct/19 */
int sortonname=0;		/* sort on name rather than character code 97/Oct/30 */
int letrasetflag=0;		/* rename f-ligatures based on bogus LetraSet names */

int abortflag=0;		/* 99/June/15 */
int errlevel=0;			/* 99/June/15 */

int underscoreflag=0;	/* zero => don't retain underscores in output name */
int tryunderscore=1;	/* try underscore name of file on input */
int dontexten=0;		/* dont add 'x' to file name if remapped */
int suppressligsini=0;	/* suppress ligatures if non-zero (from command) */
int suppressligs=0;		/* suppress ligatures if non-zero (per file) */
int allowligs=0;		/* allow ligatures even for fixed pitch font */
int allowdotlessi=1;	/* allow ligatures based on dotlessi */
int allowdottedi=1;		/* allow ligatures based on dotted i */
int backwardflag=0;		/* backward compatible - limit to 255 kern pairs */
int longtable=0;		/* indirect address - on if (lnext + knext) > 255 */
int ringbell=1;			/* ring bell on warning */
int ignorezeros=1;		/* ignore zero width kerns */
/* int preferfullname=0; */	/* use FullName instead of FontName */
/* int forcexerox=1; */	/* force xerox style header lengths */
int forcexerox=0;		/* force xerox style header lengths - changed 98/Aug/25*/
int hideyandy=1;		/* add Y&Y, Inc. to header */
int forcelong=0;		/* force use of long table (debugging) */
int fullwidth=0;		/* add italic correction to widths */
int lowerdef = 1;		/* lower case is not upper case ! */
int avoiddups = 0;		/* second hyphen => sfthyphen etc. */
int boundaryadd = 0;	/* add boundary character kern pairs */

int texligflag=0;		/* -a texligatures1/2 11 TeX f-ligs & pseudo-ligs */
int pseudoligs=0;		/* -f texligatures2 just 6 TeX standard pseudo ligs */
int extendligs=0;		/* -d extraligatures 3 extra `ligatures << >> ,, */
int accentligs=0;		/* -j accentligatures 58 accent`ligatures' */
int greekparens=0;		/* use ((...)) instead of <<...>> */

int ansitexflag=1;		/* allow, if Windows ANSI, add some TeX text chars */
int ansiextend=0;		/* extend ANSI in 16 - 32 for TeX *for this font* */

int texturesflag=0;		/* insert `codingscheme' & `fontID' TeXtures wants */

int suppressitalic=0;	/* For Textures, suppress Italic bit */
int suppressbold=0;		/* For Textures, suppress Bold bit */

int alignmentflag=1;	/* implement alignment zone adjustments 97/Dec/21 */

int boldflag = 0;		/* for face byte in case of TeXtures */
int italicflag = 0;		/* for face byte in case of TeXtures */
/* int QDEncode=0; */	/* screen encoding for TeXtures metric file */
/* int PSEncode=0; */	/* printer encoding for TeXtures metric file */

int noteabsence=0;		/* warn if character in AFM, not in encoding */
int presortflag=1;		/* sort kern pairs before passing to main routine */
						/* this now is the default ... */

int killkern=0;			/* non-zero if too many kern pairs seen */

int limitatzero=1;		/* limit height and depth at zero */

int vectorflag=0;		/* next arg is reencoding vector */
int spaceflag=0;		/* next arg is space width */
int extraflag=0;		/* next arg is extra space width */
int pathflag=0;			/* next arg is path for vectors */
int outputpathflag=0;	/* next arg is path for output TFM file 95/Jan/2 */
int checksumflag=0;		/* next arg is checksum */
int italicfuzzflag=0;	/* next arg is new italicfuzz */
int maxkernflag=0;		/* next arg is max kern pairs per character */
int minkernflag=0;		/* next arh is min kernsize */
int boundarygapflag=0;	/* next arg is boundary char kern size */

int mathitflag=0;		/* non-zero => 1 fewer parameters (6 total) */
int mathsyflag=0;		/* non-zero => 15 extra parameters (22 total) */
int mathexflag=0;		/* non-zero => 6? extra parameters (13 total) */

int ecflag=0;			/* non-zero => 9 extra parameters (16 total) */
						/* can be set from command line or via comments */

int showwidthflag=0;	/* show sum of lower case widths 2000 May 14th */

int rightboundary=-1;	/* right boundary character, if any 97/Oct/1 */
int leftboundary=-1;	/* left boundary character, if any 97/Oct/1 */
int leftprogram=-1;		/* where special left boundary program lives */

int reencodeforg=0;		/* need to reencode vector on way in orginal copy */
int reencodeflag=0;		/* need to reencode vector on way in */
int vetonontext=1;		/* do not reencode non-text fonts 98/Apr/5 98/Jun/30 */
int encodingset=0;		/* if encoding set up still */
int textfontflag=0;		/* if apparently text font 98/Apr/5 */
						/* based on Encoding in AFM file */
int pass=0;				/* incremented in each pass through file */

int spacefixed;			/* if spacewidth from command line or Comment Space */ 
int extrafixed;			/* if spacewidth from command line or Comment Extra */
int spaceseen;			/* have already encountered "N space" */

int paulflag=0;			/* -P use Paul A's scheme for stretch and shrink */
						/* 1/2 stretch 1/6 shrink */
int alanflag=0;			/* -J use Alan J's scheme for stretch and shrink */
						/* 1/3 stretch 1/3 shrink */
int sebasflag=0;		/* -R use Sebastian Rahtz's for stretch and shrink */
						/* 0.6 stretch 0.24 shrink */
int averageflag=0;		/* -N average of fixed and scaled scheme */
						/* 0.6 stretch 0.3 shrink averaged with 160 and 110 */

int stripbolditalic = 0;	/* hard wired now */

int missingboxes;		/* non-zero if some bboxes missing */

int kernsave=0;			/* steps that can be omitted */

int duplicates;			/* how many repeat encoding chars found */

int kernligsize;		/* current max kern and lig allowed 94/July/18 */
						/* shouldn't get larger than MAXKERN */

int keepingacc;			/* how many composite kerns conflicted */
int keepingbou;			/* how many boundary kerns conflicted */

double italicfuzz=ITALICFUZZ;

volatile int bAbort=0;	/* 92/Nov/24 */

double minkernsize=0.0;	/* ignore kerning pairs smaller than this */

double xscale=1.0;		/* aspect ratio --- first entry in FontMatrix */
						/* used for condensed and expanded fonts */

double ascender, descender;	/* from AFM file -  */ /* not used */
double xheight, capheight;	/* from AFM file - in case no BBox */

double cap_height, asc_height, acc_cap_height, desc_depth;
double max_height, max_depth, digit_width, cap_stem, baseline_skip;

double alignment=25.0;	/* max width of typical alignment zone 97/Dec/21 */

int chr;				/* current character working on (for debug out) */

/* char *defaultencoding="textext"; */

char *defaultencoding="none"; 

char *vectorpath = "";	/* directory for encoding vectors */

char *outputpath = "";	/* directory for TFM file output */

/* char programpath[MAXPATHLEN] = "c:\\dvipsone"; */

char programpath[FILENAME_MAX] = "c:\\yandy\\util";

char fontname[FONTNAME_MAX];
char fullname[FONTNAME_MAX];
char familyname[FONTNAME_MAX];
char macintoshname[FONTNAME_MAX];

char line[MAXLINE];

char logline[MAXLINE];					/* 99/June/11 */

int wantsignature = -1;					/* +1 checksum Y&Y_ */
										/* -1 checksum vector base fourty */

unsigned long checksum = 0;					/* checksum if specified */

unsigned long checkdefault = 0x59265920;	/* default signature "Y&Y " */

int checksumset = 0;						/* non-zero if checksum set */

double designsize;						/* design size */

/* the normal 7 parameters in font  - last missing in cmmi */

double italicangle=0.0;
double spacewidth=0.0;
double spacestretch=0.0;
double spaceshrink=0.0;
/* double xheight; */
double em_quad=0.0;
double extraspace=0.0;

int isfixedpitch=0;

double boundaryspace=0.0;		/* kern read from command line - 100 typical */
								/* -g=100 */

char *leftboundarychar=NULL;	/* From comment in AFM file 97/Oct/1 */
char *rightboundarychar=NULL;	/* From comment in AFM file 97/Oct/1 */
char *boundarychar="space";		/* disable from command line -H 98/Aug/15 */

/* 15 extra mathsy parameters  - from 8 to 22 */

double num1, num2, num3, denom1, denom2;
double sup1, sup2, sup3, sub1, sub2, supdrop, subdrop;
double delim1, delim2, axisheight;

/* 6 extra mathex parameters - from 8 to 13 */

double defaultrule;
double bigopspacing1, bigopspacing2, bigopspacing3, bigopspacing4, bigopspacing5;

double fxll, fyll, fxur, fyur; 	/* from FontBBox -- never used ! */

int nchrs;				/* number of characters - StartCharMetrics */
int nkern;				/* how many kern pairs total */
int nkernzero;			/* how many kern pairs in AFM file */
int fchrs;				/* number of characters in font also in encoding ? */
int lnext, knext, ktotal;

int bufferinx;			/* `pointer' into buffer (byte count) */
int ligkerninx;			/* `pointer' in lig/kern program (word count) */

int maxligkern;			/* debugging - max of above */

int italicneeded;		/* non-zero if italic correction table is needed */

int	lf, lh, bc, ec, nw, nh, nd,	ni, nl, nk, ne, np;

/* Indeces that tell where lig kern programs start and end */

int ligbegin[MAXCHRS], ligend[MAXCHRS];
int kernbegin[MAXCHRS], kernend[MAXCHRS];
/* int ligkerndup[MAXCHRS]; */	/* indicate duplicated 92/Dec/11 */

#define NOEQUIV 255				/* mark for no equivalent kern program */

unsigned char kerneqv[MAXCHRS];	/* ptr earlier char with same ligkern prog */
								/* determined in findrepeats 97/Oct/1 */

unsigned char charrem[MAXCHRS];	/* ptr to character program saved 97/Oct/1 */
								/* used when kerneqv not NOEQUIV */

/* char encoding[MAXCHRS][MAXCHARNAME];	*/	/* name from AFM file */

char *encoding[MAXCHRS];	/* name from AFM file or vector 94/July/18 */

/* double width[MAXCHRS]; */					/* width from AFM file */
long width[MAXCHRS];							/* width from AFM file */

/* double xll[MAXCHRS], yll[MAXCHRS], xur[MAXCHRS], yur[MAXCHRS]; */
/* float xll[MAXCHRS], yll[MAXCHRS], xur[MAXCHRS], yur[MAXCHRS]; */
long xll[MAXCHRS], yll[MAXCHRS], xur[MAXCHRS], yur[MAXCHRS]; 

/* double height[MAXCHRS], depth[MAXCHRS], italic[MAXCHRS]; */
/* float height[MAXCHRS], depth[MAXCHRS], italic[MAXCHRS];  */
long height[MAXCHRS], depth[MAXCHRS], italic[MAXCHRS];

/* double widthtable[MAXCHRS]; */
/* double widthtable[MAXCHRS+1]; */	/* includes initial zero width */
long widthtable[MAXCHRS+1];	/* includes initial zero width */

/* double heighttable[MAXCHRS], depthtable[MAXCHRS], italictable[MAXCHRS]; */
/*double heighttable[MAXCHRS+1], depthtable[MAXCHRS+1], italictable[MAXCHRS+1];*/
long heighttable[MAXCHRS+1], depthtable[MAXCHRS+1], italictable[MAXCHRS+1];

/* double kerntable[MAXCHRS]; */		/* was MAXKERN - big space eater */
long kerntable[MAXDISTINCT];			/* *distinct* kern sizes only */

/* to change the above to float from double, need to change `comparedouble' */
/* probably shouldn't change widths to float - loose too much accuracy */

unsigned long extentable[MAXEXTEN]; /* extensible char table */

int extenpointer[MAXCHRS];	/* pointers into above or -1 */

int ascendpointer[MAXCHRS];	/* pointers to next char in chain or -1 */

int counttable[MAXCHRS];	/* used in reduction of tables */

/* is there a (char) problem here with characters above 127 ??? */

/* int ligsucc[MAXLIG]; */
/* int ligature[MAXLIG]; */
/* int kernsucc[MAXKERN]; */	/* MAXKERN ? NO */
/* float kern[MAXKERN]; */		/* was double - big space eater */

/* char ligsucc[MAXLIG]; */
char __far *ligsucc=NULL;		/* put in far space */
/* char ligature[MAXLIG]; */
char __far *ligature=NULL;		/* put in far space */
/* char kernsucc[MAXKERN];	*/	/* MAXKERN ? NO */ 
char __far *kernsucc=NULL;		/* put in far space */
/* long kern[MAXKERN];	*/		
long __far *kern;				/* put in far space ? */

/* int charstart; */ /* NOT USED */

/* int widthstart, heightstart, depthstart, italicstart; */	/* NOT USED */

/* int ligkernstart, kernstart, extenstart, paramstart;	*/	/* NOT USED */

#define MAXENCODING (XEROXCODEN*4)		/* 40 bytes in Xerox PARC header */

char encodingvecname[MAXENCODING]="";	/* first line encoding vector file */

char encodingscheme[MAXENCODING]="";	/* from EncodingScheme (AFM) */

char charcodingscheme[MAXENCODING]="";	/* from CharacterCodingScheme (AFM) */

#define MAXFONTID 60					/* actual limit is 20 */

char fontid[MAXFONTID];					/* 20 for Xerox PARC header data */

char codingvector[MAXENCODING];			/* 20 which coding scheme in use */

char *vectorfile="";					/* full filename with path 95/Feb/19 */

int buffersize;							/* presently allocated */

/* unsigned char buffer[MAXBUFFER]; */		/* place for constructing TFM */

/* unsigned char __far *buffer; */			/* place for constructing TFM */

unsigned char __far *buffer=NULL;			/* place for constructing TFM */ 

unsigned char __far *kernbuffer=NULL;		/* place for kern pairs - sort */

int nforward=0;		/* how many forward pointers will be needed */
					/* i.e. how many have ligkern progs */
int nextslot=0;		/* next slot in forwarding array */

/* struct kernpair {		
	int a; int b; float kern;
}; */

/* struct kernpair {
	int a; int b; long kern;
}; */ /* inefficient 8 byte version */

struct kernpair {		/* structure for a kern pair - now 6 bytes */
	unsigned char a; unsigned char b; long kern;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following is only used for Mac TFM's using Mac Encoding Vector */

int usemacencoding = 0;		/* When using Mac encoding */
int borrowsymbol = 0;		/* Apple LW borrows math chars from symbol */
							/* Note that Textures printing fails to do this */

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

#ifdef IGNORED
/* deal with bogus glyph names in LetraSet fonts */

char *letranames[][2] = {
	{"asciicircum",	"fi"},
	{"asciitilde",	"fl"},
	{"logicalnot",	"ff"},
	{"plusminus",	"ffi"},
	{"mu",	"ffl"},
	{"fi",	"asciicircum"},
	{"fl",	"asciitilde"},
	{"ff",	"logicalnot"},
	{"ffi",	"plusminus"},
	{"ffl",	"mu"},
	{"", ""}
};
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *encodingnames[][2] = {				/* `standard' encoding names */
{"TeX text", "textext"},					/* cmr*, cmb*, cmsl*, cmss* */
{"TeX text italic", "texital"},					/* for	cmti* */
{"TeX text without f-ligatures", "textype"},	/* cmcsc10 & cmr5 */
{"TeX typewriter text", "typewrit"},		/* cm*tt* */
{"TeX typewriter text italic", "typeital"},		/* for  cmitt* */
{"TeX extended ASCII", "texascii"},			/* cmtex* */
{"TeX math italic", "mathit"},				/* cmmi* */
{"TeX math symbols", "mathsy"},				/* cmsy* */
{"TeX math extension", "mathex"},			/* cmex10 */
{"ASCII caps and digits", "textext"},		/* cminch ??? */
{"AdobeStandardEncoding", "standard"},
{"Adobe StandardEncoding", "standard"},
{"Adobe Symbol Encoding", "symbol"},
{"Adobe Dingbats Encoding", "dingbats"},
{"MicroSoft Windows ANSI 3.0", "ansi"},
{"MicroSoft Windows ANSI 3.1", "ansinew"},
{"Ventura Publisher Encoding", "ventura"},
{"TeX 256 character Encoding", "tex256"},
{"Extended TeX Font Encoding - Latin", "tex256"},
{"Extended TeX Font Encoding - Latin", "cork"},
{"TeX text companion symbols 1---TS1", "ts1"},
/* {"Default TeX Encoding", "default"}, */
/* {"MacIntosh", "mac"}, */
{"Macintosh", "mac"},
/* {"AT&T Garamond Encoding", "atandt"}, */
/* {"ASCII caps and digits", "textext"}, */		 /* cminch */
/* {"TEX TEXT IN ADOBE SETTING", "textext"}	*/   /* pctex style ? */
{"TEX TEXT + ADOBESTANDARDENCODING", "neonnew"}, /* 92/Dec/22 */
{"TeX typewriter and Windows ANSI", "texnansi"}, /* 94/Dec/31 */
{"TEXBASE1ENCODING", "8r"},							/* 98/Feb/12 */
{"", ""}
 };	
  
/**********************************************************************/

int allowaliases=0;			/* 94/August/9 use 'A' on command line */

char *charaliases[][2] = {
{"sfthyphen", "hyphen"},
{"nbspace", "space"},
{"brokenbar", "bar"},
{"space", "visiblespace"},			/* 98/Jul/12 for stupid T1 encoding */
/* {"scedilla", "scommaaccent"}, */	/* ? */
/* {"tcedilla", "tcommaaccent"}, */	/* ? */
/* {"Scedilla", "Scommaaccent"}, */	/* ? */
/* {"Tcedilla", "Tcommaaccent"}, */	/* ? */
/* {"middot", "periodcentered"}, */
/* {"overscore", "macron"}, */
/* {"Ohm", "Omega"}, */
/* {"increment", "Delta"}, */
/* {"micro", "mu"}, */
{"", ""}
};

/**********************************************************************/

/* ALL LIGATURES WITH THE SAME STARTING CHARACTER MUST APPEAR TOGETHER */

/* The first eleven `ligatures' are standard in Computer Modern fonts */

/* The  following should be suppressed in fixed-width fonts */

char *texligatures1[][3]={		/* these are the only REAL ligatures */
{"f", "i", "fi"},
{"f", "l", "fl"},
{"f", "f", "ff"},
{"f", "j", "fj"},				/* added 95/Oct/15 */
{"f", "b", "fb"},				/* added 95/Oct/17 */
{"f", "k", "fk"},				/* added 95/Oct/17 */
{"ff", "i", "ffi"},
{"ff", "l", "ffl"},
{"ff", "j", "ffj"},				/* added 95/Oct/15 */
{"c", "t", "ct"},				/* added 95/June/8 */
{"s", "t", "st"},				/* added 95/June/8 */
{"i", "j", "ij"},				/* added 95/Oct/15 ??? */
{"I", "J", "IJ"},				/* added 97/Oct/15 ??? */
{"", "", ""},					/* terminator */
};

/* The following may be OK in fixed width fonts */

char *texligatures2[][3]={		/* these are standard TeX pseudo ligatures */
{"exclam", "quoteleft", "exclamdown"},	/* Spanish opening exclamation mark */
{"question", "quoteleft", "questiondown"},	/* Spanish opening question mark */
{"hyphen", "hyphen", "endash"},
{"hyphen", "sfthyphen", "sfthyphen"},		/* 96/Feb/24 */
{"endash", "hyphen", "emdash"},
{"quoteleft", "quoteleft", "quotedblleft"}, /* English opening quotes, German closing quotes */
{"quoteright", "quoteright", "quotedblright"},	/* English and Polish closing quotes */
{"quotereversed", "quotereversed", "quotedblreversed"},	/* 97/May/10 */
{"quotesinglbase", "quotesinglbase", "quotedblbase"},	/* 97/Oct/30 */
{"", "", ""},								/* terminator */
};

/* The next three/five/nine are useful extras adopted from Sebastian Rahtz: */

char *extraligatures[][3]={
{"less", "less", "guillemotleft"},			/* French opening quotes */
{"greater", "greater", "guillemotright"},	/* French closing quotes */
/* {"exclamdown", "exclamdown", "guillemotleft"}, */
/* {"questiondown", "questiondown", "guillemotright"}, */
{"comma", "comma", "quotedblbase"},		/* German and Polish opening quotes */
/* {"suppress", "l", "lslash"}, */
/* {"suppress", "L", "Lslash"}, */
/* {"space", "l", "lslash"}, */			/* ugh ??? */
/* {"space", "L", "Lslash"}, */			/* ugh ??? */
{"percent", "perthousand", "perthousand"},	/* 98/Jun/30 for T1 encoding */
{"", "", ""},						/* terminator */
};

char *alterligatures[][3]={			/* for Greek fonts ? */
	{"parenleft", "parenleft", "guillemotleft"},			/* French opening quotes */
	{"parenright", "parenright", "guillemotright"},	/* French closing quotes */
/* {"exclamdown", "exclamdown", "guillemotleft"}, */
/* {"questiondown", "questiondown", "guillemotright"}, */
	{"comma", "comma", "quotedblbase"},		/* German and Polish opening quotes */
/* {"suppress", "l", "lslash"}, */
/* {"suppress", "L", "Lslash"}, */
/* {"space", "l", "lslash"}, */			/* ugh ??? */
/* {"space", "L", "Lslash"}, */			/* ugh ??? */
	{"percent", "perthousand", "perthousand"},	/* 98/Jun/30 for T1 encoding */
	{"", "", ""},						/* terminator */
};

char *letraligatures1[][3]={		/* using bogus names in LetraSet fonts */
	{"f", "i", "asciicircum"},
	{"f", "l", "asciitilde"},
	{"f", "f", "logicalnot"},
	{"logicalnot", "i", "plusminus"},
	{"logicalnot", "l", "mu"},
	{"", "", ""},					/* terminator */
};


/* The next 58 are convenient ligatures for accented characters */
/* (These do nothing if encoding is StandardEncoding or TeX text) */
/* The composite character name itself is generated automatically, */
/* and "" for the base means copy the previous base. */

/* possible weird alternatives `quotedbl A' => Adieresis ? UGH, NO */
/* possible weird alternatives `asciicircum A' => Acircumflex ? NO */
/* possible weird alternatives `asciitilde A' => Atilde ? NO */

/* changed `dotlessi' => `i'	93/March/13 */
/* maybe put in *both* `i' and `dotlessi' version ??? OK 95/April/22 */
/* maybe allow `degree' `A' => `Aring' ? */ /* But TeX uses \aa and \AA */

char *accentligatures[][3]={
{"acute", "A", ""},				/* {"acute", "A", "Aacute"}, */
{"", "E", ""},					/* {"acute", "E", "Eacute"}, */
{"", "I", ""},					/* {"acute", "I", "Iacute"}, */
{"", "O", ""},					/* {"acute", "O", "Oacute"}, */
{"", "U", ""},					/* {"acute", "U", "Uacute"}, */
{"", "Y", ""},					/* {"acute", "Y", "Yacute"}, */
{"", "a", ""},					/* {"acute", "a", "aacute"}, */
{"", "i", ""},					/* {"acute", "i", "iacute"}, */
{"", "dotlessi", ""}, 		/* {"acute", "dotlessi", "iacute"}, 95/Apr/22 */
{"", "e", ""},					/* {"acute", "e", "eacute"}, */
{"", "o", ""},					/* {"acute", "o", "oacute"}, */
{"", "u", ""},					/* {"acute", "u", "uacute"}, */
{"", "y", ""},					/* {"acute", "y", "yacute"}, */
{"caron", "S", ""},				/* {"caron", "S", "Scaron"}, */
{"", "Z", ""},					/* {"caron", "Z", "Zcaron"}, */
{"", "s", ""},					/* {"caron", "s", "scaron"}, */
{"", "z", ""},					/* {"caron", "z", "zcaron"}, */
{"cedilla", "C", ""},			/* {"cedilla", "C", "Ccedilla"}, */
{"", "c", ""},					/* {"cedilla", "c", "ccedilla"}, */
{"circumflex", "A", ""},		/* {"circumflex", "A", "Acircumflex"}, */
{"", "E", ""},					/* {"circumflex", "E", "Ecircumflex"}, */
{"", "I", ""},					/* {"circumflex", "I", "Icircumflex"}, */
{"", "O", ""},					/* {"circumflex", "O", "Ocircumflex"}, */
{"", "U", ""},					/* {"circumflex", "U", "Ucircumflex"}, */
{"", "a", ""},					/* {"circumflex", "a", "acircumflex"}, */
{"", "i", ""},					/* {"circumflex", "i", "icircumflex"},*/
{"", "dotlessi", ""},	/* {"circumflex", "dotlessi", "icircumflex"}, 95/Apr/22 */
{"", "e", ""},					/* {"circumflex", "e", "ecircumflex"}, */
{"", "o", ""},					/* {"circumflex", "o", "ocircumflex"}, */
{"", "u", ""},					/* {"circumflex", "u", "ucircumflex"}, */
{"dieresis", "A", ""},			/* {"dieresis", "A", "Adieresis"}, */
{"", "E", ""},					/* {"dieresis", "E", "Edieresis"}, */
{"", "I", ""},					/* {"dieresis", "I", "Idieresis"}, */
{"", "O", ""},					/* {"dieresis", "O", "Odieresis"}, */
{"", "U", ""},					/* {"dieresis", "U", "Udieresis"}, */
{"", "Y", ""},					/* {"dieresis", "Y", "Ydieresis"}, */
{"", "a", ""},					/* {"dieresis", "a", "adieresis"}, */
{"", "dotlessi", ""},		/* {"dieresis", "dotlessi", "idieresis"}, 95/Apr/22 */
{"", "i", ""},					/* {"dieresis", "i", "idieresis"}, */
{"", "e", ""},					/* {"dieresis", "e", "edieresis"}, */
{"", "o", ""},					/* {"dieresis", "o", "odieresis"}, */
{"", "u", ""},					/* {"dieresis", "u", "udieresis"}, */
{"", "y", ""},					/* {"dieresis", "y", "ydieresis"}, */
{"grave", "A", ""},				/* {"grave", "A", "Agrave"}, */
{"", "E", ""},					/* {"grave", "E", "Egrave"}, */
{"", "I", ""},					/* {"grave", "I", "Igrave"}, */
{"", "O", ""},					/* {"grave", "O", "Ograve"}, */
{"", "U", ""},					/* {"grave", "U", "Ugrave"}, */
{"", "a", ""},					/* {"grave", "a", "agrave"}, */
{"", "i", ""},					/* {"grave", "i", "igrave"}, */
{"", "dotlessi", ""},		/* {"grave", "dotlessi", "igrave"}, 95/Apr/22 */
{"", "e", ""},					/* {"grave", "e", "egrave"}, */
{"", "o", ""},					/* {"grave", "o", "ograve"}, */
{"", "u", ""},					/* {"grave", "u", "ugrave"}, */
{"ring", "A", ""},				/* {"ring", "A", "Aring"}, */
{"", "a", ""},					/* {"ring", "a", "aring"}, */
{"tilde", "A", ""},				/* {"tilde", "A", "Atilde"}, */
{"", "N", ""},					/* {"tilde", "N", "Ntilde"}, */
{"", "O", ""},					/* {"tilde", "O", "Otilde"}, */
{"", "a", ""},					/* {"tilde", "a", "atilde"}, */
{"", "n", ""},					/* {"tilde", "n", "ntilde"}, */
{"", "o", ""},					/* {"tilde", "o", "otilde"}, */
{"", "", ""},					/* terminator */
};

/* {"degree", "A", ""},	*/			/* {"degree", "A", "Aring"}, UGH */
/* {"", "a", ""}, */				/* {"degree", "a", "aring"}, UGH */

/**********************************************************************/

/* Codes used for expanding kern pairs in AFM file */
/* Adding kern pairs for accented characters based on base characters */

/* code "*" start of list for new base letter */
/* code ""   equivalent on left and right, both upper and lower case */
/* code "L"  only equivalent on the left  (i.e. use as charb in kern) */
/* code "R"  only equivalent on the right (i.e. use as chara in kern) */
/* code " l"  lower case only */
/* code " u" upper case only */
/* If first letter is a space, use rest as replacement */
/* otherwise construct replacement by concatenating to base char */

char *kernrep[][2] = {
	{"A", "*"},
	{ "acute", ""},
	{ "breve", ""},
	{ "circumflex", ""},
	{ "dieresis", ""},
	{ "grave", ""},
	{ "ogonek", ""},
	{ "ring", ""},
	{ "tilde", ""},
	{ "E", "Lu"},			/* AE equiv A on left only */
	{ "e", "Ll"},			/* ae equiv a on left only */

	{"C", "*"},
	{ "acute", ""},
	{ "caron", ""},
	{ "cedilla", ""},

	{"D", "*"},
	{ "caron", " u"},		/* Dcaron equiv D upper case only */
	{ "caron", "Ll"},		/* dcaron equiv d left only */
	{ "croat", ""},
	{ " Eth", "Ru"},		/* right and upper case only */

	{"E", "*"},
	{ "acute", ""},
	{ "caron", ""},
	{ "circumflex", ""},
	{ "dieresis", ""},
	{ "grave", ""},
	{ "ogonek", ""},
	{ " AE", "Ru"},			/* AE right only */
	{ " ae", "Rl"},			/* ae right only */
	{ " OE", "Ru"},			/* OE right only */
	{ " oe", "Rl"},			/* oe right only */

	{"G", "*"},				/* ? */
	{ "breve", ""},

	{"I", "*"},
	{ "acute", ""},
	{ "circumflex", ""},
	{ "dieresis", ""},
	{ "dotaccent", " u"},	/* upper case only */
	{ "grave", ""},
	{ "J", "Lu"},			/* IJ left only */
	{ "j", "Ll"},			/* ij left only */

	{"J", "*"},
	{ " IJ", "Ru"},			/* IJ right only */
	{ " ij", "Rl"},			/* ij right only */

	{"L", "*"},				/* ? */
	{ "acute", ""},

	{"N", "*"},				/* ? */
	{ "acute", ""},
	{ "caron", ""},
	{ "tilde", ""},
	{ " Eng", "L"},			/* Eng on left only */

	{"O", "*"},
	{ "acute", ""},
	{ "circumflex", ""},
	{ "dieresis", ""},
	{ "grave", ""},
	{ "hungarumlaut", ""},
	{ "tilde", ""},
	{ "slash", ""},			/* Oslash */
	{ "E", "Lu"},			/* OE left only */
	{ "e", "Ll"},			/* oe left only */

	{"R", "*"},				/* ? */
	{ "acute", ""},
	{ "caron", ""},
	
	{"S", "*"},
	{ "acute", ""},
	{ "caron", ""},
	{ "cedilla", ""},
	{ "commaaccent", ""},

	{"T", "*"},
	{ "caron", "L"},		/* lower case only */
	{ "cedilla", ""},
	{ "commaaccent", ""},

	{"U", "*"},
	{ "acute", ""},
	{ "circumflex", ""},
	{ "dieresis", ""},
	{ "grave", ""},
	{ "hungarumlaut", ""},
	{ "ring", ""},

	{"Y", "*"},
	{ "acute", ""},
	{ "dieresis", ""},	

	{"Z", "*"},
	{ "acute", ""},
	{ "caron", ""},
	{ "dotaccent", ""},

	{"", "*"}
};

/**********************************************************************/

/* Following stuff here for Mac Style FontName contraction */

char MacName[64];				/* 5 + 3 + 3 ... Mac Style Font Name */

char BaseName[32];				/* base name derived from FontName */

/* #define MAXSUFFIX 8 */
#define MAXSUFFIX 12

char SuffixString[MAXSUFFIX][24];	/* suffix strings */

unsigned int nextsuffix;			/* count of suffix strings */

/* #endif */

/**********************************************************************/

void ShowLine (char *, int);

#ifndef _WINDOWS
void ShowLine (char *line, int errflag) {	/* 99/June/11 */
 	if (line == NULL || *line == '\0') return;
 	if (errflag) {
 		fflush(stdout);
 		fputs(line, errout);
 		fflush(errout);
 	}
 	else fputs(line, stdout);
 }
#endif

/* DLL version of this comes later ... */

/**********************************************************************/

void perrormod (char *s) {
//#ifdef _WINDOWS
	sprintf(logline, "%s: %s\n", s, strerror(errno));
	showline(logline, 1);
// #else
//	perror (s);
// #endif
}

void pause (void) {
#ifndef _WINDOWS
	fflush(stdout);			/* ??? */
	fflush(stderr);			/* ??? */
	(void) _getch();		/* ??? */
#endif
}

/**********************************************************************/

char *zstrdup (char *s) {
	char *new = _strdup(s);
	if (new != NULL) return new;
	sprintf(logline, "ERROR: Unable to allocate memory for %s\n", s);
	showline(logline, 1);
#ifdef _WINDOWS
	abortflag++;
	return NULL;
#else
	showline("Press any key to continue . . .\n", 0);
	pause();
	exit(1);
#endif
}

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */
/* in DLL version just use GetPrivateProfileString instead ... 99/June/15 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindoini = "dviwindo.ini";		/* name of ini file we look for */

#ifndef _WINDOWS

// when run as console application we read INI file directly

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
	if (dviwindo != NULL && *dviwindo != '\0') {
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
							if (traceflag) {  /* DEBUGGING ONLY */
								sprintf(logline, "%s=%s", varname, line+n+1);
								showline(logline, 0);
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

#endif // end of ifn _WINDOWS

char *grabenv (char *varname) {	/* get from [Environment] in dviwindo.ini */
#ifdef _WINDOWS
	(void) GetPrivateProfileString("Environment", varname, "", line, sizeof(line), dviwindoini);
	if (traceflag) {
		sprintf(logline, "%s=%s\n", varname, line);
		showline(logline, 0);
	}
	if (*line != '\0') return zstrdup(line);
	else return getenv(varname);
#else
	if (usedviwindo && ! dviwindoinisetup) setupdviwindo();		/* 99/June/16 */
	return grabenvvar (varname, dviwindo, "[Environment]", usedviwindo);
#endif
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

/**********************************************************************/

int checkpause (int flag) {						/* 95/Oct/28 */
/*	if (grabenv ("DEBUGPAUSE") != NULL) { */		/* 97/Nov/28 */
	if (grabenv ("DEBUGPAUSE") != NULL || flag) {
		showline("\n", 0);
		showline("Press any key to continue . . .\n", 0);
		pause();
		return -1;
	}
	return 0;
}

void checkenter (int argc, char *argv[]) {			/* 95/Oct/28 */
	int m;
	char current[FILENAME_MAX];
	if (grabenv ("DEBUGPAUSE") != NULL) {
		(void) _getcwd(current, sizeof(current));
		sprintf(logline, "Current directory: `%s'\n", current);
		showline(logline, 0);
		for (m = 0; m < argc; m++) {
//			sprintf(logline, "%2d:\t`%s'\n", m, argv[m]); 
			sprintf(logline, "%2d:  `%s'\n", m, argv[m]); 
			showline(logline, 0);
		}
		checkpause(0);
	}
}

void checkexit (int n) {							/* 95/Oct/28 */
	checkpause(1);
#ifdef _WINDOWS
	abortflag++;
#else
	exit(n);
#endif
}

/**********************************************************************/


/* Add ability to continue line using \ before \n ? */

int getline (FILE *infile, char *buff, int nmax) {
	int c, k=0;
	char *s=buff;

	while ((c = getc(infile)) != '\n' && c != EOF) {
		*s++ = (char) c;
		k++;
		if (k >= nmax) {
			*s = '\0';
			sprintf(logline, "Line too long: %s (> %d)\n", buff, MAXLINE);
			showline(logline, 1);
/*			exit (3); */
			checkexit (3);
			return 0;
		}
	}
	if (c != EOF) {
		*s++ = (char) c;
		k++;
	}
	*s = '\0';
	return k;
}

int getrealline (FILE *infile, char *buff, int nmax) {
	int k;
	k = getline(infile, buff, nmax);
	while ((*buff == '%' || *buff == '\n' || *buff == ';') && k > 0) {
		k = getline(infile, buff, nmax);
	}
	return k;
}

/* 2^20 = 1048576 */

long mapdouble (double x) {		/* map to tfm scaled integer */
	if (x == 0.0) return 0L;
	else if (x < 0.0) return -mapdouble(-x);
	else return (long) ((x * 1048576.0 + 499.999) / 1000.0);
}

double unmap (long x) {			/* map back to Adobe double */
	double dx;
	long th;
	if (x == 0) return 0.0;
	if (x < 0) return -unmap(-x);
	dx = (double) x / 1048.576;
/*	th = (long) (dx * 1000.0 + 500.0); */
/*	th = (long) (dx * 500.0 + 250.0); */
	th = (long) (dx * 500.0 + 0.5);			/* 98/Aug/15 */
/*	return ((double) x / 1048.576); */
/*	return (th / 1000.0); */
	return (th / 500.0); 
}

/*	modified to allow for repeated occurences */ /* 92/Nov/26 */
/*	should this also check whether font has character ? */
/*	that is, whether width[k] != NOWIDTH ? */

int clookup (char *character, int last) {		/* 92/Nov/26 */
	int k, l;
	char *alias;
	if (strlen(character) == 1) {	/* speedup 94/July/18 */
		k = *character;
		if (k > last && k < fontchrs)
			if (strcmp(character, encoding[k]) == 0) return k;
	}
/*	sfthyphen in ligature table => second occurence of hyphen in encoding */
	if (strcmp(character, "sfthyphen") == 0) { 	/* special case */
/*	disable this hack while reading AFM file ? */
		for (k = last + 1; k < fontchrs; k++) {
			if (strcmp("hyphen", encoding[k]) == 0) break;
		}
		for (k = k + 1; k < fontchrs; k++) {
			if (strcmp("hyphen", encoding[k]) == 0) return k;
		}
	}
/*	main lookup loop */
	for (k = last + 1; k < fontchrs; k++) {
		if (strcmp(character, encoding[k]) == 0) return k;
	}
/*	kludge to deal with T1 encoding when AFM has space kerns 98/Jul/12 */
	if (strcmp(character, "space") == 0) {
		for (k = last + 1; k < fontchrs; k++) {
			if (strcmp("visiblespace", encoding[k]) == 0) {
				return k;
			}
		}		
	}
/*	New 94/April/10 */
	if (allowaliases && last < 0) {
		for (l = 0; l < 16; l++) {
			if (strcmp(charaliases[l][0], "") == 0) break;
			if (strcmp(charaliases[l][0], character) == 0) {
				alias = charaliases[l][1];
				for (k = 0; k < fontchrs; k++) {
					if (strcmp(alias, encoding[k]) == 0) return k;
				}
				break;
			}
		}
	}
	if (noteabsence && last < 0) {
		sprintf(logline, "%s? ", character);
		showline(logline, 0);
	}
	return -1;
}

/* do not copy lig/kern stuff if flag == 0 */

void copymetrics (int i, int k, int flag) {		/* spliced out 93/Dec/18 */
	width[i] = width[k]; italic[i] = italic[k];
	height[i] = height[k]; depth[i] = depth[k];
	xll[i] = xll[k]; yll[i] = yll[k]; 
	xur[i] = xur[k]; yur[i] = yur[k]; 
	if (ligbegin[i] != -1 || kernbegin[i] != -1) {
		sprintf(logline, "ERROR: Attempt to overwrite kern/lig %d %d\n", i, k);
		showline(logline, 1);
/*		continue; */
		return;
	}
	if (flag) {
		ligbegin[i] = ligbegin[k]; ligend[i] = ligend[k];
		kernbegin[i] = kernbegin[k]; kernend[i] = kernend[k];
	}
/*	otherwise ligbegin, ligend, kernbegin, kernend left at -1 */
}

void replicate(int k) {
/*	see whether same character code appears in another position and copy */
/*	this is for multiple encoding of same character in encoding vector */
	int i;
	if (k < 0 || k > MAXCHRS) {		/* shouldn't happen 93/Jan/5 */
		sprintf(logline, "ERROR: Bad replication %d\n", k);
		showline(logline, 1);
		return;
	}
/*	for(i=k+1; k < fontchrs; k++) { */	/* changed 92/Dec/11 */
/*	for(i=0; k < fontchrs; k++) { */
	for(i=0; i < fontchrs; i++) {		/* changed 93/Jan/5 */
		if (i == k) continue;
		if (strcmp(encoding[i], encoding[k]) == 0) {
			if (width[i] != NOWIDTH) {
				if (width[i] != width[k]) {
					sprintf(logline, 
					"ERROR: Attempt to overwrite width %d (%lg) with %d (%lg)\n", 
							i, unmap(width[i]), k, unmap(width[k]));
					showline(logline, 1);
						continue;
				}
				else { /* attempt to check that it is OK */
#ifdef DEBUGGING
					char *s;
					sprintf(logline, "ERROR: Attempt to overwrite width %d with %d MISMATCH:\n", i, k); 
					s = line + strlen(line);
					if (italic[i] != italic[k] ||
						height[i] != height[k] || depth[i] != depth[k]) {
						sprintf(s, "italic %lg %lg - height %lg %lg - depth %lg %lg\n",
								unmap(italic[i]), unmap(italic[k]),
								unmap(height[i]), unmap(height[k]),
								unmap(depth[i]), unmap(depth[k]));
						s += strlen(s);
					}
					if (xll[i] != xll[k] ||	xur[i] != xur[k] ||
						yll[i] != yll[k] ||	yur[i] != yur[k]) {
						sprintf(s, "bbox %ld %ld %ld %ld - %ld %ld %ld %ld\n",
								xll[i], yll[i], xur[i], yur[i],  xll[k], yll[k], xur[k], yur[k]);
						s += strlen(s);
					}
					if (ligbegin[i] != ligbegin[k] ||
						ligend[i] != ligend[k] ||
						kernbegin[i] != kernbegin[k] ||
						kernend[i] != kernend[k]) {
						sprintf(s, "lig %d - %d %d - %d - kern %d - %d %d - %d\n",
								ligbegin[i], ligend[i], ligbegin[k], ligend[k], 
								kernbegin[i], kernend[i], kernbegin[k], kernend[k]);
						s += strlen(s);
					}
					showline(logline, 0);
#endif
					continue; 	 /* ??? 93/Jan/6 */
				}
			}
/*			copymetrics (i, k, 0); */	/* should this be zero ? */
/*		this may be a problem if the character carries a lig/kern prog ??? */
			copymetrics (i, k, 1);		/* ??? */
		}
	}
}

void stripreturn(char *name) {
	int c;
	char *s=name;

	while ((c = *s) != '\0' && c != '\n' && c != '\r') s++;
	if (c == '\n' || c == '\r') *s = '\0';
	s--;			/* also get rid of trailing white space */
	while ((c = *s) <= ' ') *s-- = '\0';
}

void complain(void) {
#ifdef _WINDOWS
	if (ringbell) MessageBeep(0xFFFFFFFF);
	abortflag++;
#else
	if (ringbell) putc('\a', errout);	/* ??? */
	pause();
#endif
}

int alluppercase(char *s) {
	int c;
	while ((c = *s++) > 0) 
		if ((c < 'A' || c > 'Z') && (c < '0' || c > '9')) return 0;
	return -1;
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
	"extrablack", "ultrablack", "black", "ultra", "heavy", "extrabold",
	"bold",
	"demi", "semi", "demibold", "semibold",
	"medium", "plain", "regular", "standard", "roman", "normal", "book",
	"light", "thin", "extralight", "ultralight", ""
};

int weightvalues[] = {
	800, 800, 800, 800, 800, 800,
	800,
	600, 600, 600, 600,
	400, 400, 400, 400, 400, 400, 400,
	200, 200, 200, 200, 0
};

int analyzeweight(char *s, char *line) {
	char str[128];
	int k;
	int weight=400;				/* default */

	sscanf(s, "%s", str);
/*	lowercase(str, str); */		/* for stupid DTF fonts 92/Oct/22 */
	for (k = 0; k < 64; k++) {
		if (_strcmpi(weightnames[k], "") == 0) break;
		if (_strcmpi(weightnames[k], str) == 0) break;
	}
	if (strcmp(weightnames[k], "") != 0) weight = weightvalues[k];
	else {
		showline("Unrecognized Weight ", 1);
		showline(line, 0);
	}
	if (texturesflag && verboseflag) {
		sprintf(logline, "Weight `%s' => %d\n", str, weight);
		showline(logline, 0);
	}
	return weight;
}

/* read up to StartCharMetrics */
/* gathering information as it goes */
/* returns non-zero if some serious problem */

int tostartchar (FILE *infile, int passes) { /* read up to StartCharMetrics */
	char *s, *so;
	int n, last, next;
	unsigned int exten, top, mid, bot, rep;
	
	if (passes == 0) xscale = 1.0;
/*	don't reset if spacewidth set from command line */
	if (spacefixed > 0)	spacefixed = 0;
	if (extrafixed > 0)	extrafixed = 0;
	if (passes == 0) {
		italicflag = 0;
		boldflag = 0;
		if (boundarychar != NULL) {	/* copy default boundary char */
			leftboundarychar = zstrdup(boundarychar); /* 98/Aug/15 */
			rightboundarychar = zstrdup(boundarychar); /* 98/Aug/15 */
			if (leftboundarychar == NULL || rightboundarychar == NULL) return -1;
		}
		else leftboundarychar = rightboundarychar = NULL;
/*		checksumset = 0; */
	}
	while (getrealline(infile, line, MAXLINE) > 0) {
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;	// end of this section
/*		if (passes == 0)  */
		if (passes != 0) continue;
/*		following are useful items normally found in  AFM file */
		if (strncmp(line, "ItalicAngle", 11) == 0) {
			s = line + 11;
			while (*s > ' ') s++;
			s++;
/*			sscanf(line, "ItalicAngle %lg", &italicangle); */
			if (sscanf(s, "%lg", &italicangle) < 1) showline(line, 1);
			if (italicangle != 0.0) italicflag = 1;
			else italicflag = 0;
			if (traceflag) showline(line, 0);
			if (italicangle > 0.0) {
				sprintf(logline,
					"WARNING: ItalicAngle positive %lg (%s)\n",
					   italicangle, fn_in);
				showline(logline, 0);
			}
		}
		else if (strncmp(line, "Weight", 6) == 0) {
			s = line + 6;
			while (*s > ' ') s++;
			s++;
/*			if (analyzeweight(line + 7, line) > 400) boldflag = 1; */
			if (analyzeweight(s, line) > 400) boldflag = 1;
			if (traceflag) showline(line, 0);
		}
		else if (strncmp(line, "XHeight", 7) == 0) {
			s = line + 7;
			while (*s > ' ') s++;
			s++;
/*			sscanf(line, "XHeight %lg", &xheight); */
			if (sscanf(s, "%lg", &xheight) < 1) showline(line, 1);
			if (traceflag) showline(line, 0);
		}
		else if (strncmp(line, "CapHeight", 9) == 0) {
			s = line + 9;
			while (*s > ' ') s++;
			s++;
/*			sscanf(line, "CapHeight %lg", &capheight); */
			if (sscanf(s, "%lg", &capheight) < 1) showline(line, 1);
			if (traceflag) showline(line, 0);
		}
		else if (strncmp(line, "Ascender", 7) == 0) {
			s = line + 7;
			while (*s > ' ') s++;
			s++;
/*			sscanf(line, "Ascender %lg", &ascender); */
			if (sscanf(s, "%lg", &ascender) < 1) showline(line, 1);
			if (traceflag) showline(line, 0);
		}
		else if (strncmp(line, "Descender", 7) == 0) {
			s = line + 7;
			while (*s > ' ') s++;
			s++;
/*			sscanf(line, "Descender %lg", &descender); */ 	/* negative quantity */
			if (sscanf(s, "%lg", &descender) < 1) showline(line, 1);
			if (traceflag) showline(line, 0);
		}
/*		else if (strncmp(line, "IsFixedPitch", 12) == 0 ||
			     strncmp(line, "isFixedPitch", 12) == 0) { */
		else if (_strnicmp(line, "IsFixedPitch", 12) == 0) {
			if (traceflag) showline(line, 0);
			if (strstr(line, "true") != NULL) isfixedpitch = 1;
			else if (strstr(line, "false") != NULL) isfixedpitch = 0;
			else {
				showline("WARNING: Don't understand pitch: ", 0);
				showline(line, 0);
			}
		}
		else if (strncmp(line, "FontName", 8) == 0) {
			s = line + 8;
			while (*s > ' ') s++;
			s++;
/*			strncpy(fontname, line + 9, FONTNAME_MAX); */	/* 92/01/26 */
			strncpy(fontname, s, FONTNAME_MAX);		/* 97/Feb/04 */
			stripreturn(fontname);	/* allow for TrueType font names ? */
			if (verboseflag) showline(line, 0);
			else if (traceflag) showline(line, 0);
		}
		else if (strncmp(line, "FullName", 8) == 0) {
			s = line + 8;
			while (*s > ' ') s++;
			s++;
/*			strncpy(fullname, line + 9, FONTNAME_MAX); */
			strncpy(fullname, s, FONTNAME_MAX);
			stripreturn(fullname);
			if (traceflag) showline(line, 0);
		}
		else if (strncmp(line, "FamilyName", 10) == 0) {
			s = line + 10;
			while (*s > ' ') s++;
			s++;
/*			strncpy(familyname, line + 11, FONTNAME_MAX); */
			strncpy(familyname, s, FONTNAME_MAX);
			stripreturn(familyname);
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "MacIntoshName")) != NULL ||
//				 (s = strstr(line, "MacintoshName")) != NULL) {
		else if (_strnicmp(line, "Comment MacintoshName", 21) == 0) {
			s = line + 21;
			while (*s > ' ') s++;
			s++;
/*			strncpy(macintoshname, s + 14, FONTNAME_MAX); */
			strncpy(macintoshname, s, FONTNAME_MAX);
			stripreturn(macintoshname);
			if (traceflag) showline(line, 0);
		}
		else if (strncmp(line, "FontBBox", 8) == 0) {
			s = line;
			while (*s > ' ') s++;
			s++;
/*			sscanf(line, "FontBBox %lg %lg %lg %lg", */
			if (sscanf(s, "%lg %lg %lg %lg", 
				&fxll, &fyll, &fxur, &fyur) < 4) showline(line, 1);
			if (traceflag) showline(line, 0);
		}
		else if (strncmp(line, "EncodingScheme", 14) == 0) {
			s = line;
			while (*s > ' ') s++;
			s++;
/*			sscanf(line, "EncodingScheme %s", encodingscheme); */
/*			strncpy(encodingscheme, line + 15, MAXENCODING); */
			strncpy(encodingscheme, s, MAXENCODING);
			stripreturn(encodingscheme);	/* allow for long names ? */
			if (traceflag) showline(line, 0);
		}
/* following are special things from TFM files made by AFMtoTFM */
//		else if ((s = strstr(line, "Condensed")) != NULL ||
//				(s = strstr(line, "Expanded")) != NULL) {
		else if ((_strnicmp(line, "Comment Condensed", 17) == 0) ||
				 (_strnicmp(line, "Comment Expanded", 16) == 0)) {
			s = line + 16;
			while (*s > ' ') s++;
			s++;
/*			sscanf(s, "Condensed %lg", &xscale); */
			if (sscanf(s, "%lg", &xscale) < 1)  showline(line, 1);
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "CheckSum ")) != NULL) {
		else if (_strnicmp(line, "Comment CheckSum", 16) == 0) {
			s = line + 16;
			while (*s > ' ') s++;
			s++;
/*			sscanf(s, "CheckSum  %08lX", &checksum);  */
/*			sscanf(s, "CheckSum  %lx", &checksum); */
			if (sscanf(s, "%lx", &checksum) < 1) showline(line, 1);
			if (traceflag) showline(line, 0);
			checksumset = 1;
		}
//		else if ((s = strstr(line, "Quad ")) != NULL) {
		else if (_strnicmp(line, "Comment Quad", 12) == 0) {
			s = line + 12;
			while (*s > ' ') s++;
			s++;
/*			sscanf(s, "Quad %lg", &em_quad); */
			if (sscanf(s, "%lg", &em_quad) < 1) showline(line, 1);
			em_quad = em_quad * xscale;		/* do later ? */
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "ExtraSpace ")) != NULL &&
		else if (_strnicmp(line, "Comment ExtraSpace", 18) == 0 &&
			! extraflag) {
			s = line + 18;
			while (*s > ' ') s++;
			s++;
/*			sscanf(s, "ExtraSpace %lg", &extraspace); */
			if (sscanf(s, "%lg", &extraspace) < 1)  showline(line, 1);
			extraspace = extraspace * xscale;	/* do later ? */
			if (extraspace != 0.0) {		/* 92/Aug/18 */
				mathitflag = 0;			/* can't be math italic then! */
			}
			if (traceflag) showline(line, 0); 
			extrafixed = 1;		/* avoid overriding this later */
		}
//		else if ((s = strstr(line, "Space ")) != NULL &&
		else if (_strnicmp(line, "Comment Space", 13) == 0 &&
			! spacefixed) {
			s = line + 13;
			while (*s > ' ') s++;
			s++;
/*			sscanf(s, "Space %lg %lg %lg", */
//			spacewidth = spacestretch = spaceshrink = 0;
			if (sscanf(s, "%lg %lg %lg",
				   &spacewidth, &spacestretch, &spaceshrink) < 1)  showline(line, 1);
			spacewidth = spacewidth * xscale;
			spacestretch = spacestretch * xscale;
			spaceshrink = spaceshrink * xscale;
			if (traceflag) showline(line, 0);
			spacefixed = 1;		/* avoid overriding this later */
		}
//		else if ((s = strstr(line, "FontID ")) != NULL) {
		else if (_strnicmp(line, "Comment FontID", 14) == 0) {
			s = line + 14;
/*	avoid using BitStream Comment bitsFontID line ! */
			if (strstr(line, "bits") == NULL) {
				while (*s > ' ') s++;
				s++;
/*				strncpy(fontid, s + 7, MAXFONTID); */
				strncpy(fontid, s, MAXFONTID);
				stripreturn(fontid);
				if (traceflag) showline(line, 0);
			}
		}
//		else if ((s = strstr(line, "CharacterCodingScheme ")) != NULL) {
		else if (_strnicmp(line, "Comment CharacterCodingScheme", 29) == 0) {
			s = line + 29;
			while (*s > ' ') s++;
			s++;
/*			strncpy(charcodingscheme, s + 22, MAXENCODING); */
			strncpy(charcodingscheme, s, MAXENCODING);
			stripreturn(charcodingscheme);
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "DesignSize ")) != NULL) {
		else if (_strnicmp(line, "Comment DesignSize", 18) == 0) {
			s = line + 18;
			while (*s > ' ') s++;
			s++;
/*			sscanf(s, "DesignSize %lg", &designsize); */
			if (sscanf(s, "%lg", &designsize) < 1) showline(line, 1);
			if (traceflag) showline(line, 0);
		}
/* following is new for boundary characters */
//		else if ((s = strstr(line, "LeftBoundary ")) != NULL) {
		else if (_strnicmp(line, "Comment LeftBoundary", 20) == 0) {
			char str[128];
			s = line + 20;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%s", &str) < 1) showline(line, 1);
			leftboundarychar = zstrdup(str);
			if (leftboundarychar == NULL) return -1;
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "RightBoundary ")) != NULL) {
		else if (_strnicmp(line, "Comment RightBoundary", 21) == 0) {
			char str[128];
			s = line + 21;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%s", &str) < 1) showline(line, 1);
			rightboundarychar = zstrdup(str);
			if (rightboundarychar == NULL) return -1;
			if (traceflag) showline(line, 0);
		}
/* following are extra parameters for EC */ /* Comment xxxxxx */
		else if (_strnicmp(line, "Comment CapHeight", 17) == 0) {
			s = line + 17;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &cap_height) < 1) showline(line, 1);
/*			ecflag++; */
		}
		else if (_strnicmp(line, "Comment Ascender", 16) == 0) {
			s = line + 16;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &asc_height) < 1) showline(line, 1);
/*			ecflag++; */
		}
		else if (_strnicmp(line, "Comment AccentedCapHeight", 25) == 0) {
			s = line + 25;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &acc_cap_height) < 1) showline(line, 1);
			ecflag++;
		}
		else if (_strnicmp(line, "Comment Descender", 17) == 0) {
			s = line + 17;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &desc_depth) < 1)  showline(line, 1); 	/* positive */
/*			ecflag++; */
		}
		else if (_strnicmp(line, "Comment MaxHeight", 17) == 0) {
			s = line + 17;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &max_height) < 1) showline(line, 1);
			ecflag++;
		}
		else if (_strnicmp(line+8, "Comment MaxDepth", 16) == 0) {
			s = line + 16;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &max_depth) < 1)  showline(line, 1);	/* positive */
			ecflag++;
		}
		else if (_strnicmp(line, "Comment FigureWidth", 19) == 0) {
			s = line + 19;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &digit_width) < 1) showline(line, 1);
			ecflag++;
		}
/* capital letter vertical stem width */
		else if (_strnicmp(line, "Comment StemV", 13) == 0) {
			s = line + 13;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &cap_stem) < 1) showline(line, 1);
			ecflag++;
		}
		else if (_strnicmp(line, "Comment BaseLineSkip", 20) == 0) {
			s = line + 20;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &baseline_skip) < 1) showline(line, 1);
			ecflag++;
		}
/* following are extra parameters for mathsy */
//		else if ((s = strstr(line, "Num ")) != NULL) {
		else if (_strnicmp(line, "Comment Num", 11) == 0) {
			s = line + 11;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg %lg %lg", &num1, &num2, &num3) < 3) showline(line, 1);
			mathsyflag++;
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "Denom ")) != NULL) {
		else if (_strnicmp(line, "Comment Denom", 13) == 0) {
			s = line + 13;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg %lg", &denom1, &denom2) < 2) showline(line, 1);
			mathsyflag++;
			if (traceflag) showline(line, 0);
		}			
//		else if ((s = strstr(line, "Sup ")) != NULL) {
		else if (_strnicmp(line, "Comment Sup", 11) == 0) {
			s = line + 11;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg %lg %lg", &sup1, &sup2, &sup3) < 3) showline(line, 1);
			mathsyflag++;
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "Sub ")) != NULL) {
		else if (_strnicmp(line, "Comment Sub", 11) == 0) {
			s = line + 11;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg %lg", &sub1, &sub2) < 2) showline(line, 1);
			mathsyflag++;
			if (traceflag) showline(line, 0);
		}						
//		else if ((s = strstr(line, "Supdrop ")) != NULL) {
		else if (_strnicmp(line, "Comment Supdrop", 15) == 0) {
			s = line + 15;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &supdrop) < 1) showline(line, 1);
			mathsyflag++;
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "Subdrop ")) != NULL) {
		else if (_strnicmp(line, "Comment Subdrop", 15) == 0) {
			s = line + 15;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &subdrop) < 1) showline(line, 1);
			mathsyflag++;
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "Delim ")) != NULL) {
		else if (_strnicmp(line, "Comment Delim", 13) == 0) {
			s = line + 13;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg %lg", &delim1, &delim2) < 2) showline(line, 1);
			mathsyflag++;
			if (traceflag) showline(line, 0);
		}
//		else if ((s = strstr(line, "Axisheight ")) != NULL) {
		else if (_strnicmp(line, "Comment Axisheight", 18) == 0) {
			s = line + 18;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &axisheight) < 1) showline(line, 1);
			mathsyflag++;
			if (traceflag) showline(line, 0);
		}
/*	following are for mathex extra paramaters */
//		else if ((s = strstr(line, "DefaultRuleThickness ")) != NULL) {
		else if (_strnicmp(line, "Comment DefaultRuleThickness", 28) == 0) {
			s = line + 28;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg", &defaultrule) < 1) showline(line, 1);
			mathexflag++;
			if (traceflag) showline(line, 0); 
		}			
//		else if ((s = strstr(line, "BigOpSpacing ")) != NULL) {
		else if (_strnicmp(line, "Comment BigOpSpacing", 20) == 0) {
			s = line + 20;
			while (*s > ' ') s++;
			s++;
			if (sscanf(s, "%lg %lg %lg %lg %lg", 
				   &bigopspacing1, &bigopspacing2, &bigopspacing3,
				   &bigopspacing4, &bigopspacing5) < 5) showline(line, 1);
			mathexflag++;
			if (traceflag) showline(line, 0); 
		}
		
/* following provide extra information for cmex10 */
//		else if ((s = strstr(line, "Ascending ")) != NULL) {
		else if (_strnicmp(line, "Comment Ascending", 17) == 0) {
			s = line + 17;
			so = s;
//			s = s + 9;
			if (sscanf(s, "%d%n,", &last, &n) < 1) showline(line, 1);
			s = s + n + 1;
			if (last < 0 || last >= fontchrs) {
				sprintf(logline, "ERROR: Asc char code %d invalid\n", last);
				showline(logline, 1);
				last = 0;
				complain();
			}
			while (sscanf(s, "%d%n,", &next, &n) > 0) {
				if (next < 0 || next >= fontchrs) {
					sprintf(logline, "ERROR: Asc char code %d invalid\n", next);
					showline(logline, 1);
					next = 0;
					complain();
				}
				if (next == 0) {	/* debugging */
					showline("LINE: ", 1);
					showline(line, 0);
					sprintf(logline, "so: %s, s: %s ", so, s);
					showline(logline, 0);
					sprintf(logline, "LAST %d NEXT %d ", last, next);
					showline(logline, 0);
				}
				ascendpointer[last] = next;
				last = next;
				s = s + n + 1;
			}
			mathexflag++;
			if (traceflag) showline(line, 0);
		}			
//		else if ((s = strstr(line, "Extensible ")) != NULL) {
		else if (_strnicmp(line, "Comment Extensible", 18) == 0) {
			s = line + 18;
/*		sprintf(logline, "EXTENSIBLE %d\n",
				sscanf(s, 
					"Extensible %d top %d mid %d bot %d rep %d", 
						&exten, &top, &mid, &bot, &rep)); 
			showline(logline, 0);
			pause(); */
			if (sscanf(s, 
/*				"Extensible %d top %d mid %d bot %d rep %d", */
//					   "Extensible %u top %u mid %u bot %u rep %u",
					   "%u top %u mid %u bot %u rep %u",
					   &exten, &top, &mid, &bot, &rep) == 5) {
				if (exten >= (unsigned int) fontchrs) {
					sprintf (logline, 
								 "ERROR: Extensible code %u invalid\n", exten);
					showline(logline, 1);
					exten = 0;
					complain();
				}
/*			assert (exten >= 0 && exten < fontchrs); */
				extenpointer[exten] = ne;
				assert (ne >= 0 && ne < fontchrs);
				extentable[ne] = 
								(((unsigned long) top) << 24) | 
								(((unsigned long) mid) << 16) | 
								(bot << 8) | rep;
				ne++;
				if (ne > MAXEXTEN) {
					sprintf(logline, 
							"ERROR: Too many extensible chars (> %d)\n", 
							MAXEXTEN);
					showline(logline, 1);
					complain();
					ne--;
				}
				mathexflag++;
				if (traceflag) showline(line, 0);
			}	/* if sscanf == 5 */
			else showline(line, 1);
		} /* end of Extensible case */
		if (abortflag > 0) return -1;
	}  // end of while loop reading lines of AFM file
	return 0;			/* successful */
}

void removesemi (char *name) {	/* get rid of trailing semi colon - if any */
	char *s;
	s = name + strlen(name) - 1;
	if (*s == ';') *s = '\0';
}

/* int aspect(int x) {
	if (x == 0) return 0;
	else if (x < 0) return - (int) ((double) (- x) * xscale + 0.5);
	else return (int) ((double) x * xscale + 0.5);
} */

int kernzeros, kernignored, kerntruncated;

/* read kerning pairs */	

int readkerns(FILE *infile, struct kernpair __far *rawkerns) {
/*	int chara, charb; */
	char chara[MAXCHARNAME], charb[MAXCHARNAME];
	int nchara, ncharb;
	double kerndist; 
	int nkern=0;

	kernzeros = 0; kernignored = 0; kerntruncated = 0;
	
	while (getrealline(infile, line, MAXLINE) > 0) {
/*		if (strncmp(line, "EndKernPairs", 12) == 0) break; */
		if (strncmp(line, "EndKern", 7) == 0) break;		
		if (strstr(line, "KPX") == NULL) continue;
		if (sscanf(line, "KPX %s %s %lg", 
			chara, charb, &kerndist) < 3) {
			showline("Don't understand: ", 1);
			showline(line, 0);
			continue;
		}
		if (xscale != 1.0) kerndist = kerndist * xscale;
		if (ignorezeros && kerndist == 0.0) {
			kernzeros++; continue;   /* ignore zero kern */
		}
		if (kerndist < minkernsize && kerndist > - minkernsize) {
			kernignored++; continue;   /* ignore small kern */
		}	
		if (killkern) {
			kerntruncated++; continue;	/* or break; ? */
		}
/*		sprintf(logline, "KPX %s %s %lg\n", chara, charb, kerndist); */
/*		modified to use `visiblespace' if `space' not in encoding 98/Jun/30 */
		nchara = clookup(chara, -1); 
/*		width[nchara] != NOWIDTH ??? */
//		if (nchara < 0)
		if (nchara < 0 || width[nchara] == NOWIDTH)	// 2000/May/18
		{
			if (strcmp(chara, "space") == 0) {
				nchara = clookup("visiblespace", -1); 
/*				sprintf(logline, "%s => %d %s", chara, nchara, line); */
//				if (nchara < 0)
				if (nchara < 0 || width[nchara] == NOWIDTH) // 2000/May/18
					continue;
			}
			else continue;		/* not in encoding */
		}
		ncharb = clookup(charb, -1);
//		if (ncharb < 0)
		if (ncharb < 0 || width[ncharb] == NOWIDTH)	// 2000/May/18
		{
			if (strcmp(charb, "space") == 0) {
				ncharb = clookup("visiblespace", -1);
/*				sprintf(logline, "%s => %d %s", charb, ncharb, line); */
//				if (ncharb < 0)
				if (ncharb < 0 || width[ncharb] == NOWIDTH)	// 2000/May/18
					continue;
			}
			else continue;		/* not in encoding */
		}
		
/*		rawkerns[nkern].a = nchara; */
/*		rawkerns[nkern].b = ncharb; */
		rawkerns[nkern].a = (unsigned char) nchara;
		rawkerns[nkern].b = (unsigned char) ncharb;
/*		rawkerns[nkern].kern = (float) kerndist; */
		rawkerns[nkern].kern = mapdouble(kerndist);
		nkern++;
		if (nkern > kernligsize-1) { 
			sprintf(logline, "Too many kern pairs (> %d)\n", kernligsize-1);
			showline(logline, 1);
			killkern++;
		}
		if (abortflag > 0) return 0;
	} // end of while loop reading kerns
	return nkern;
}

/* Check whether this kern already in raw kern table */
/* returns index into rawkerns if a pair already exists */

int kernindex (int nchara, int ncharb) {
	int k;
	struct kernpair __far *rawkerns = (struct kernpair __far *) kernbuffer;
	for (k = 0 ; k < nkern; k++) {
		if (rawkerns[k].a == nchara && rawkerns[k].b == ncharb) return k;
	}
	return -1;
}

/* Stuff for replicating kern pairs for accented characters 97/Oct/19 */

int matchreplicate (char *name) {
	int k;
/*	if (traceflag) sprintf(logline, "Find match for %s\n", name); */
	for (k = 0; k < 128; k++) {
		if (*(kernrep[k][0]) == '\0') break;	/* end of list */
		if (*(kernrep[k][1]) != '*') continue;	/* not start of char list */
		if (_stricmp(kernrep[k][0], name) == 0) return k;	/* match */
	}
	return -1;
}

/* Replicate left character base => accented */
/* This adds little to size of TFM file if optimize turned on */

int replicateleft(int nkern, struct kernpair __far *rawkerns) {
	int nchara, ncharb;
	int k, kbase, m, caseflag, nk;
	int nkerninx=nkern;
	long kpx;
/*	int exitflag=0; */
	char *s;

	if (showreplicate) {
		sprintf(logline, "REPLICATELEFTKERN %d\n", nkern);
		showline(logline, 0);
	}
/*	for (k = 0; k < nkern; k++) { */
	for (k = 0; k < nkernzero; k++) {
		nchara = rawkerns[k].a;
		ncharb = rawkerns[k].b;
		kpx = rawkerns[k].kern;
		if (showreplicate) {
			sprintf(logline, "MASTER: KPX, %s %s %lg\n",
							  encoding[nchara], encoding[ncharb], unmap(kpx));
			showline(logline, 0);
		}
/*		deal with pairs where this is the first character */
		s = encoding[nchara];
		if (*s >= 'A' && *s <= 'Z') caseflag = 1;
		else caseflag = 0;
		kbase = matchreplicate(s);
		if (kbase >= 0) {
			for (m = kbase+1; m < 128; m++) {
				if (*(kernrep[m][1]) == '*') break;		/* next base letter */
				if (*(kernrep[m][0]) == '\0') break;	/* end of table */
				if (*(kernrep[m][1]) == 'L') continue;	/* not on left */
/*				sprintf(logline, "%s\t%c %c\n", kernrep[m][1],
					   *(kernrep[m][1]), *(kernrep[m][1]+1)); */
				if (*(kernrep[m][1]) != '\0') {
					if ((*(kernrep[m][1]+1) == 'l' && caseflag) ||
						(*(kernrep[m][1]+1) == 'u' && ! caseflag))
						continue;	/* wrong case */
				}
				if (*(kernrep[m][0]) != ' ') {
					*line = *(kernrep[kbase][0]);	/* base letter */
					strcpy(line+1, kernrep[m][0]);
				}
				else strcpy(line, kernrep[m][0]+1);	/* don't use base */
				if (! caseflag && (*line >= 'A' && *line <= 'Z'))
					*line = (char) (*line + 'a' - 'A');
				else if (caseflag != 0 && (*line >= 'a' && *line <= 'z'))
					*line = (char) (*line + 'A' - 'a');
				nchara = clookup(line, -1);
				if (nchara >= 0 && width[nchara] == NOWIDTH) {
					if (traceflag) {
						showline(line, 1);
						showline(" not in font\n", 1);
					}
					nchara = -1;	/* pretend not in encoding */
				}
				if (nchara >= 0) {
					nk = kernindex(nchara, ncharb);
					if (nk >= 0) {
						if (traceflag) {
							sprintf(logline, "Keeping: KPX %s %s %lg\n",
								   encoding[nchara], encoding[ncharb],
								   unmap(rawkerns[nk].kern));
							showline(logline, 0);
						}
						keepingacc++;
						continue;	/* don't override KPX from AFM ? */
					}
/* new special casing for accented chars based on "i" and "I" 98/Jun/30 */
					if (_stricmp(kernrep[kbase][0], "i") == 0) {
						if (kpx < 0) {
							if (traceflag) {
								sprintf(logline, "Not adding: %s %s %lg\n",
									 encoding[nchara], encoding[ncharb],
									   unmap(kpx));
								showline(logline, 0);
							}
							continue;
						}
						else {
							if (traceflag) {
								sprintf(logline, "Modifying KPX %s %s %lg\n",
									   encoding[nchara], encoding[ncharb],
									   unmap(kpx));
								showline(logline, 0);
							}
							kpx += mapdouble(30);	/* arbitrary additional kerning */
						}
					}
					if (showreplicate) {
						sprintf(logline, "Adding: %s %s %lg\n",
								encoding[nchara], encoding[ncharb], unmap(kpx));
						showline(logline, 0);
					}

					if (nkerninx >= kernligsize) {
						sprintf(logline, "ERROR: Too many added kern pairs %d\n", 
								nkerninx);
						showline(logline, 1);
						return nkerninx;			/* 98/Apr/4 */
					}
					rawkerns[nkerninx].a = (unsigned char) nchara;	/* new */
					rawkerns[nkerninx].b = (unsigned char) ncharb;
					rawkerns[nkerninx].kern = kpx;
					nkerninx++;
					kpx = rawkerns[k].kern;	/* restore in case changed */
				}
/*				if (exitflag) break; */
			}
		}
/*		nchara = rawkerns[k].a; */			/* restore ? */
	}
	if (verboseflag) {
		if (nkerninx-nkern > 0) {
			sprintf(logline, "Added %d kern pairs (%s) (%d + %d = %d)\n",
							nkerninx-nkern, "left", nkern, nkerninx-nkern,
							nkerninx);
			showline(logline, 0);
		}
	}
	return nkerninx;
}
	
/* Replicate right character base => accented */
/* This can add a lot to size of TFM file */

int replicateright(int nkern, struct kernpair __far *rawkerns) {
	int nchara, ncharb;
	int k, kbase, m, caseflag, nk;
	int nkerninx=nkern;
	long kpx;
/*	int exitflag=0; */
	char *s;

	if (showreplicate) {
		sprintf(logline, "REPLICATERIGHTKERN %d\n", nkern);
		showline(logline, 0);
	}
/*	for (k = 0; k < nkern; k++) { */
	for (k = 0; k < nkernzero; k++) {
		nchara = rawkerns[k].a;
		ncharb = rawkerns[k].b;
		kpx = rawkerns[k].kern;
		if (showreplicate) {
			sprintf(logline, "MASTER: KPX, %s %s %lg\n",
							  encoding[nchara], encoding[ncharb], unmap(kpx));
			showline(logline, 0);
		}
/*		deal with pairs where this is the second character */
		s = encoding[ncharb];
		if (*s >= 'A' && *s <= 'Z') caseflag = 1;
		else caseflag = 0;
		kbase = matchreplicate(s);
		if (kbase >= 0) {
			for (m = kbase+1; m < 128; m++) {
				if (*(kernrep[m][1]) == '*') break;		/* next base letter */
				if (*(kernrep[m][0]) == '\0') break;	/* end of table */
				if (*(kernrep[m][1]) == 'R') continue;	/* not on right */
/*				sprintf(logline, "%s\t%c %c\n", kernrep[m][1],
					   *(kernrep[m][1]), *(kernrep[m][1]+1)); */
				if (*(kernrep[m][1]) != '\0') {
					if ((*(kernrep[m][1]+1) == 'l' && caseflag) ||
						(*(kernrep[m][1]+1) == 'u' && ! caseflag))
						continue;	/* wrong case */
				}
				if (*(kernrep[m][0]) != ' ') {
					*line = *(kernrep[kbase][0]);	/* base letter */
					strcpy(line+1, kernrep[m][0]);
				}
				else strcpy(line, kernrep[m][0]+1);	/* don't use base */
				if (! caseflag && (*line >= 'A' && *line <= 'Z'))
					*line = (char) (*line + 'a' - 'A');
				else if (caseflag != 0 && (*line >= 'a' && *line <= 'z'))
					*line = (char) (*line + 'A' - 'a');
				ncharb = clookup(line, -1);
				if (ncharb >= 0 && width[ncharb] == NOWIDTH) {
					if (traceflag) {
						showline(line, 1);
						showline(" not in font\n", 1);
					}
					ncharb = -1;	/* pretend not in encoding */
				}
				if (ncharb >= 0) {
					nk = kernindex(nchara, ncharb);
					if (nk >= 0) {
						if (traceflag) {
							sprintf(logline, "Keeping: KPX %s %s %lg\n",
								   encoding[nchara], encoding[ncharb],
								   unmap(rawkerns[nk].kern));
							showline(logline, 0);
						}
						keepingacc++;
						continue;	/* don't override KPX from AFM ? */
					}
/* new special casing for accented chars based on "i" and "I" 98/Jun/30 */
					if (_stricmp(kernrep[kbase][0], "i") == 0) {
						if (kpx < 0) {
							if (traceflag) {
								sprintf(logline, "Not adding: %s %s %lg\n",
									 encoding[nchara], encoding[ncharb],
									   unmap(kpx));
								showline(logline, 0);
							}
							continue;
						}
						else {
							if (traceflag) {
								sprintf(logline, "Modifying KPX %s %s %lg\n",
									   encoding[nchara], encoding[ncharb],
									   unmap(kpx));
								showline(logline, 0);
							}
							kpx += mapdouble(30);	/* arbitrary additional kerning */
						}
					}
					if (showreplicate) {
						sprintf(logline, "Adding: %s %s %lg\n",
								encoding[nchara], encoding[ncharb], unmap(kpx));
						showline(logline, 0);
					}

					if (nkerninx >= kernligsize) {
						sprintf(logline, "ERROR: Too many added kern pairs %d\n", 
								nkerninx);
						showline(logline, 1);
						return nkerninx;			/* 98/Apr/4 */
					}
					rawkerns[nkerninx].a = (unsigned char) nchara;
					rawkerns[nkerninx].b = (unsigned char) ncharb;	/* new */
					rawkerns[nkerninx].kern = kpx;
					nkerninx++;
					kpx = rawkerns[k].kern;	/* restore in case changed */
				}
/*				if (exitflag) break; */
			}
		}
/*		ncharb = rawkerns[k].b; */			/* restore ? */
	}
	if (verboseflag) {
		if (nkerninx-nkern > 0) {
			sprintf(logline, "Added %d kern pairs (%s) (%d + %d = %d)\n",
							nkerninx-nkern, "right", nkern, nkerninx-nkern,
							nkerninx);
			showline(logline, 0);
		}
	}
	return nkerninx;
}

/* our own heap sort routine that deals with far addresses */
/* not a stable sort - does not preserve order of equals ... */
/* we could really use a stable sort in case of stupid AFMs */

int swapcount;			/* debugging */

void swap(char __far *v, int nsize, int i , int j) {
	int c;
	char __far *vi;
	char __far *vj;
	int k;

	if (i == j) return;
	swapcount++;
	vi = v + i * nsize;
	vj = v + j * nsize;
	for (k = 0; k < nsize; k++) {
		c = *vi; *vi = *vj; *vj = (char) c;
		vi++; vj++;
	}
}

void fqsortsub(void __far *v, int nsize, int left, int right, 
		int (*compare) (const void __far *, int, int, int)) {	/* 92/Nov/24 */
	int i, last;

	if (left >= right) return;
	swap(v, nsize, left, (left + right) /2 );
	last = left;
	for (i = left+1; i <= right; i++) {
/*		if ((*compare)((void __far *) ((char __far *) v + i * nsize), 
			(void __far *) ((char __far *) v + left * nsize)) < 0)
			swap(v, nsize, ++last, i); */
		if ((*compare)(v, nsize, i, left) < 0)
			swap(v, nsize, ++last, i);
	}
	swap(v, nsize, left, last);
	fqsortsub(v, nsize, left, last-1, compare);
	fqsortsub(v, nsize, last+1, right, compare);		
}

void fqsort(void __far *v, int nitems, int nsize,
		int (*compare) (const void __far *, int, int, int)) {
	swapcount=0;
	fqsortsub(v, nsize, 0, nitems-1, compare);
	if (traceflag) {			/* debugging */
		sprintf(logline, "Did %d swaps in presort of kern pairs\n", swapcount);
		showline(logline, 0);
	}
}

int comparekernpair(const void __far *arr, int nsize, int i, int j) {
	const struct kernpair __far *kern1;
	const struct kernpair __far *kern2;

	kern1 = (const struct kernpair __far *) ((char __far *) arr + i * nsize);
	kern2 = (const struct kernpair __far *) ((char __far *) arr + j * nsize);

	if (sortonname) {	/* sort on character name */		/* 97/Oct/30 */
		int ret;
		ret = strcmp(encoding[kern1->a], encoding[kern2->a]);
		if (ret != 0) return ret;
		else return strcmp(encoding[kern1->b], encoding[kern2->b]);
	}
	else {				/* sort on character code */
		if (kern1->a < kern2->a) return -1; 
		else if (kern1->a > kern2->a) return 1;
		if (kern1->b < kern2->b) return -1; 
		else if (kern1->b > kern2->b) return 1;
/*		sprintf(logline, "ERROR: Repeated kern pair %s %s (%d %d) %lg %lg\n", 
			encoding[kern1->a], encoding[kern1->b], 
			kern1->a, kern1->b, unmap(kern1->kern), unmap(kern2->kern)); */
		return 0;	/* same kern pair */
	}
}

#ifdef DEBUGKERNS
void dumpkernpairs (struct kernpair __far *arr, int n) {	/* debugging */
	int k;
	sprintf(logline, "StartKernPairs %d\n", n);
	showline(logline, 0);
	for (k = 0; k < n; k++) {
		sprintf(logline, "KPX %s %s %lg\n",
			   encoding[arr[k].a], encoding[arr[k].b],
			   unmap(arr[k].kern));
		showline(logline, 0);
	}
	showline("EndKernPairs\n", 0);
}
#endif

/* remove duplications from kern table */

int removeduplicates(struct kernpair __far *arr, int n) {
	int k, m=0;
	int repeated=0;
	while (m < n-1) {
		if ((arr[m].a != arr[m+1].a) || (arr[m].b != arr[m+1].b)) {
			m++;
			continue;
		}
		sprintf(logline, "WARNING: Repeated kern pair %s (%d) %s (%d) %lg %lg\n", 
				encoding[arr[m].a], arr[m].a, encoding[arr[m].b], arr[m].b,
				unmap(arr[m].kern), unmap(arr[m+1].kern)); 
		showline(logline, 0);
		repeated++;
/*		which one should we keep ? first or last ? */
/*		for (k=m; k < n-1; k++) arr[k] = arr[k+1]; */	/* keep first */
		for (k=m+1; k < n-1; k++) arr[k] = arr[k+1];	/* keep last */
		n--;
		if (abortflag > 0) return 0;
	}
	if (repeated > 0) {
		if (verboseflag) {
			sprintf(logline,"%d repeated kern pair%s\n",
				   repeated, (repeated == 1) ? "" : "s");
			showline(logline, 0);
		}
	}
	return n;
}

/* compare kern programs for k < m */
/* returns zero if no match, returns number of matching steps if match */

int compareprogs (struct kernpair __far *arr, int k, int m, int n) {
	int i=k, j=m;
	if (debugflag)	{
		sprintf(logline, "Comparing progs for %s (%d) and %s (%d)\n",
		   encoding[arr[k].a], arr[k].a, 
		   encoding[arr[m].a], arr[m].a);
		showline(logline, 0);
	}
	while (i < n && j < n) {
		if (arr[i].a != arr[k].a) {			/* stepped into next program */
			if (arr[j].a != arr[m].a) return (j - m);	/* check other */
			else return 0;					/* mismatch in length */
		}
		if (arr[j].a != arr[m].a) {			/* stepped into next program */
			if (arr[i].a != arr[k].a) return (j - m);	/* impossible */
			else return 0;					/* mismatch in length */
		}
		if (arr[i].b != arr[j].b) return 0;			/* mismatch in charb */
		if (arr[i].kern != arr[j].kern) return 0;	/* mismatch in amount */
		i++, j++;
	}
	if (traceflag) showline("Dropped through\n", 0);	/* debugging only */
	if (i == n && j < n) {						/* impossible */
		if (arr[j].a != arr[m].a) return (j - m);
		else return 0;					/* mismatch in length */
	}
	else if (j == n && i < n) {
		if (arr[i].a != arr[k].a) return (j - m);
		else return 0;					/* mismatch in length */
	}
	showline("IMPOSSIBLE\n", 1);
	return 0;							/* should never get here */
}

/*	Help remove redundant stuff from rawkerns */

/*	search for earlier kern program equal to the one starting at m */
/*	we only those for characters that don't also have lig programs */

/*	compare against all, find the one with lowest char code */
/*	not very efficient as a result */
/*	could be done better by searching backward and building */
/*	chain of equivalences ... */

int searchrepeats(struct kernpair __far *arr, int m, int n) {
	int k=0, nlen;
	int aold=-1;
	int klow=-1;
	int chrlow;
	int nlenlow=0;
	if (debugflag) {
		sprintf(logline, "Searching for repeats for %s (%d) at %d\n",
						  encoding[arr[m].a], arr[m].a, m);
		showline(logline, 0);
	}
/*	don't bother if character has a ligature program ? */
/*	if ((ligbegin[arr[m].a] != ligend[arr[m].a]) return 0; */
	klow = m;
	chrlow = arr[m].a;
	while (k < n) {
		if (arr[k].a != aold) {		/* start of new program ? */
			if (k == m) {			/* don't compare with self */
				aold = arr[k].a;
				k++;
				continue;
			}
			nlen = compareprogs(arr, k, m, n);
			if (nlen > 0) {
				if (arr[k].a < chrlow) {
					klow = k;
					chrlow = arr[k].a;
					nlenlow = nlen;
				}
			}
			aold = arr[k].a;
		}
		k++;
	}
	if (chrlow == arr[m].a) return 0;
/*	if (traceflag || replicateleftflag || replicaterightflag) */
	if (traceflag || showoptimize) {
		sprintf(logline, "Kern program for %s (%d) same as for %s (%d)\n",
		   encoding[arr[m].a], arr[m].a,
		   encoding[arr[klow].a], arr[klow].a);
		showline(logline, 0);
	}
/*	don't bother if equivalent character has a ligature program ? */
/*	if ((ligbegin[arr[klow].a] != ligend[arr[klow].a]) return 0; */
	kerneqv[arr[m].a] = arr[klow].a;	/* chrlow indicate equivalence */
	if (ligbegin[arr[m].a] != ligend[arr[m].a]) {
		if (ligbegin[arr[klow].a] != ligend[arr[klow].a]) {
/*			kerneqv[arr[m].a] = -1;	*/	/* saves checking later ? */
			return 0;
		}
	}
	return nlenlow;
}

/* find equal kern programs */

int findrepeats(struct kernpair __far *arr, int n) {
	int m=0, len, total=0;
	int aold=-1;
	while (m < n) {
		if (arr[m].a != aold) {	/* start of new program ? */
			if (aold >= 0) {
				len = searchrepeats(arr, m, n);
/*				should not count those with ligature programs ... */			
				total += len;
			}
			aold = arr[m].a;
		}
		m++;
	}
	if (total > 0){
		if (verboseflag) {
			sprintf(logline, "Saving %d ligkern steps by sharing (%d - %d = %d)\n",
				   total, n, total, n-total);
			showline(logline, 0);
		}
	}
	return total;
}

void sortkernpairs(struct kernpair __far *rawkerns, int n) {
	if (n == 0) return;			/* 98/Apr/5 */
	if (verboseflag) {
		sprintf(logline, "Sorting %d kern pairs\n", n);
		showline(logline, 0);
	}
	fqsort(rawkerns, n, sizeof(struct kernpair), comparekernpair);
}

/* ligaturing on the left char in pair happens before kerning */
/* ligaturing on the right char in pair happens after kerning */
/* hence  space quotedblleft/right  not normally invoked */
/* instead  space quoteleft/right  do the job */
/* this is also why we need space comma (space quotedblbase not invoked) */
/* this also means you can't have different kerns for space ` and space `` ! */

char *boundarykern[][2] = {
	{"space", "quoteleft"},			/* used also for space `` */
	{"space", "quoteright"},		/* used also for space '' */
	{"space", "quotesinglbase"},
	{"space", "quotereversed"},		/* rarely in encoding */
	{"space", "comma"},				/* used for space ,, */
	{"space", "quotedblleft"},		/* not normally invoked, instead space ` */
	{"space", "quotedblright"}, 	/* not normally invoked, instead space ' */
	{"space", "quotedblbase"},		/* not normally invoked, instead space , */
	{"space", "quotedblreversed"},	/* rarely in encoding */
	{"quotedblright", "space"},
	{"quotedblleft", "space"},
	{"quoteright", "space"},		/* used only for ' space, not '' space */
	{"quoteleft", "space"},			/* used only for ` space, not `` space */
	{"", ""}
};

/* add kerns between space (boundary char) and quotes */
/* avoid repeated kerns which are a problem with nonstable sort */

int addboundarykerns (void) {
	int count = 0;
	int k, nk, nchara, ncharb;
	struct kernpair __far *rawkerns = (struct kernpair __far *) kernbuffer;

	if (! textfontflag) {
		showline("WARNING: Not adding boundary kerns to non-text font\n", 0);
		return nkern; 				/* 98/Apr/5 ??? */
	}
	if (verboseflag) showline("Adding boundary kerns\n", 0);
	for (k = 0; k < 32; k++) {
		if (*(boundarykern[k][0]) == '\0') {
/*			boundaryspace = boundaryspace / 2;
			if (flag++) break;
			continue; */
			break;				/* end of list */
		}
		nchara = clookup(boundarykern[k][0], -1);
		if (nchara >= 0 && width[nchara] == NOWIDTH) {
			if (traceflag) {
				sprintf(logline, "%s not in font\n", boundarykern[k][0]);
				showline(logline, 0);
			}
			nchara = -1;	/* pretend not in encoding */
		}
		if (nchara < 0) continue;
		ncharb = clookup(boundarykern[k][1], -1);
		if (ncharb >= 0 && width[ncharb] == NOWIDTH) {
			if (traceflag) {
				sprintf(logline, "%s not in font\n", boundarykern[k][1]);
				showline(logline, 0);
			}
			ncharb = -1;	/* pretend not in encoding */
		}
		if (ncharb < 0) continue;
		nk = kernindex(nchara, ncharb);
		if (nk >= 0) {		/* don't overwrite */
			if (traceflag) {
				sprintf(logline, "Keeping: KPX %s %s %lg\n",
					   encoding[nchara], encoding[ncharb],
					   unmap(rawkerns[nk].kern));
				showline(logline, 0);
			}
/*			rawkerns[nk].kern = mapdouble(boundaryspace); */	/* overwrite */
			keepingbou++;
		}
		else {										/* add a new kern pair */
			rawkerns[nkern].a = (unsigned char) nchara;
			rawkerns[nkern].b = (unsigned char) ncharb;
			rawkerns[nkern].kern = mapdouble(boundaryspace);
			nkern++;
			if (nkern > kernligsize-1) { 
				sprintf(logline, "ERROR: Too many kern pairs (> %d)\n", kernligsize-1);
				showline(logline, 1);
				break;
			}
			count++;
		}
		if (traceflag) {
			sprintf(logline, "Adding: KPX %s %s %lg\n",
				   encoding[nchara], encoding[ncharb], boundaryspace);
			showline(logline, 0);
		}
	}
	if (verboseflag) {
		sprintf(logline, "Added %d boundary kerns\n", count);	/* debugging */
		showline(logline, 0);
	}
	return nkern;
}

int checkborrow(int);

/* c:\c\afmtotfm.c(3228) : fatal error C1001: internal compiler error */
/* 	(compiler file 'msc2.cpp', line 1011) */

void freeencoding (void) {
	int k;
/*	if (! encodingset) return; */		/* already freed ? */
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") == 0) continue;
		if (debugflag) {
			sprintf(logline, "free %d %s ", k, encoding[k]);
			showline(logline, 0);
		}
		free(encoding[k]);
		encoding[k] = ""; 
	}
	encodingset = 0;
}

void spaceguess (char *s) {
	if (traceflag) showline(line, 0);
	if (sscanf(s, " WX %lg", &spacewidth) < 1) {
		showline("Don't understand: ", 1);
		showline(line, 1);
	}
	if (xscale != 1.0) spacewidth = spacewidth * xscale;
/*	if (verboseflag) {
		sprintf(logline, "\nSpace (from space char) %lg\n", spacewidth);
		showline(logline, 0);
	} */
//	if (verboseflag) {
//		sprintf(logline, "p %d a %d s %d a %d\n", paulflag, alanflag, sebasflag, averageflag);
//		showline(logline, 0);
//	}
	if (paulflag) {
		spacestretch = spacewidth/2;		/* 0.5    */
		spaceshrink  = spacewidth/6;		/* 0.1666 */
/*		extraspace = 500 - spacewidth; */	/* em/2 ??? */
	}
	else if (alanflag) {
		spacestretch = spacewidth/3;		/* 0.3333 */
		spaceshrink  = spacewidth/3;		/* 0.3333 */
/*		extraspace = 500 - spacewidth; */	/* em/2 ??? */
	}
	else if (sebasflag) {
		spacestretch = spacewidth * 3 / 5;	/* 0.6  */
		spaceshrink  = spacewidth * 6 / 25;	/* 0.24 */
/*		extraspace = 500 - spacewidth; */	/* em/2 ??? */
	}
	else if (averageflag) {			/* 95/August/3 */
		spacestretch = (spacewidth * 6 / 10 + 160) / 2;
		spaceshrink  = (spacewidth * 3 / 10 + 110) / 2;
		if (spacewidth < 500)
			extraspace = ((500 - spacewidth) + 110) / 2;
		else extraspace = 110;		/* lower limit */
	}
/*  prevent spacewidth - spaceshrink from becoming too small */
	if (spacewidth - spaceshrink < 140) {	/* 95/Oct/28 */
		spacestretch = spacestretch +
					   (140 - (spacewidth - spaceshrink));
		spaceshrink = spacewidth - 140;
		if (spaceshrink < 0) spaceshrink = 0;
	}
/*  prevent spacewidth + spacestretch from becoming too large */
	if (spacewidth + spacestretch > 500) {
		spacestretch = 500 - spacewidth;
		if (spacestretch < 0) spacestretch = 0;
	}
//	if (verboseflag) {
//		sprintf(logline, "Space %lg %lg %lg\n",
//				spacewidth, spacestretch, spaceshrink);
//		showline(logline, 0);
//	}
	spaceseen++;
}

#ifdef IGNORED
int letrarename(char *s, int passes) {							// 99/Sep/24
	int i;
	for (i = 0; i < 16; i++) {
		if (strcmp(letranames[i][0], "") == 0) return 0;
		if (strcmp(s, letranames[i][0]) == 0) {
			strcpy(s, letranames[i][1]);
			if (verboseflag && passes == 0)
				printf("Replacing %s with %s\n",
					   letranames[i][0], letranames[i][1]);
			return -1;
		}
	}
	return 0;
}
#endif

/* returns non zero if some serious problem */

int readafmfile(FILE *infile, int passes) {
	int k, n, mat;
/*	char chara[MAXCHARNAME], charb[MAXCHARNAME]; */
	char ligsuccess[MAXCHARNAME], ligcombine[MAXCHARNAME];
	int nchara, ncharb, nlast;
	int ochrs;
	int charkerns=0;		/* to keep compiler happy */
/*	double kerndist, italiccorrect; */
	double italiccorrect;
	long kerndist;
	double twidth, txll, tyll, txur, tyur;
	char tencoding[CHARNAME_MAX];		/* for safety sake */
	char *s;
	int last;
	struct kernpair __far *rawkerns;
	int kfirst;
	long newwidth;					/* check on conflict */
	int bad, flag;
	int skipchar;
	int keqv;						/* 97/Oct/1 */

	rawkerns = (struct kernpair __far *) kernbuffer;

	if (traceflag) {
		sprintf(logline, "Reading AFM file pass %d\n", passes);
		showline(logline, 0);
	}

	lnext = 0; knext = 0; ktotal = 0;
	bc = fontchrs; ec = 0; missingboxes = 0;
	fchrs = 0;  duplicates=0;
	spaceseen = 0;

	if (passes == 0) {
/*		reset a bunch of flags the first time around */
		mathitflag=0;		/* non-zero => 1 fewer parameters */
		mathsyflag=0;		/* non-zero => 15 extra parameters */
		mathexflag=0;		/* non-zero => 6? extra parameters */
		textfontflag=0;		/* reset 98/Jun/30 */
		suppressligs = suppressligsini;		/* reset in case changed */

		fxll=0.0, fyll=0.0, fxur=1000.0, fyur=1000.0;
		nchrs=0, nkern=0; ne = 0;
		if (! reencodeflag)		/* free since will read from AFM */
			freeencoding();			/* just in case left over */
		if (traceflag) {
			sprintf(logline, "Resetting tables for %d chars\n", fontchrs);
			showline(logline, 0);
		}
		for (k = 0; k < fontchrs; k++) {
			width[k] = NOWIDTH;				/* mark unused position */
			ligbegin[k] = -1; ligend[k] = -1;
			kernbegin[k] = -1; kernend[k] = -1;
/*			ligkerndup[k] = 0;	*/			/* 92/Dec/11 */
			kerneqv[k] = NOEQUIV;			/* reset 97/Oct/1 */
		}
	}

	if (passes == 0)  {
		italicangle = 0.0;
		isfixedpitch = 0;
	}

#ifdef DEBUGGING
	if (traceflag) showline("Starting to read afm file\n", 0); 
#endif
	/* StartFontMetrics */

	if (tostartchar(infile, passes) != 0)		/* get to StartCharMetrics */
		return -1;								/* some serious error */

	if (passes == 0) {
		if (_stricmp(encodingscheme, "AdobeStandardEncoding") ==  0 ||
			_stricmp(encodingscheme, "StandardEncoding") ==  0 ||
			_stricmp(encodingscheme, "MicroSoft Windows ANSI 3.1") == 0 ||
			_stricmp(encodingscheme, "MS Windows ANSI") == 0)
			textfontflag = 1;
		else {									/* 98/Apr/5 */
			if (vetonontext) {
				if (reencodeflag) {
					showline("WARNING: turning off reencoding for this non-text font\n", 0);
					sprintf(logline, "         EncodingScheme %s\n", encodingscheme);
					showline(logline, 0);
				}
				reencodeflag=0;	/* temporary for this AFM */
				freeencoding();
				textfontflag = 0;
			}
			else {
				if (reencodeflag) {
					showline("WARNING: treating as text font\n", 0);
					sprintf(logline, "         EncodingScheme %s\n", encodingscheme);
					showline(logline, 0);
				}
				textfontflag = 1;	/* ??? */
			}
		}
		if (xscale != 1.0) {
			sprintf(logline, "NOTE: Imposing horizontal scaling by %lg\n", xscale);
			showline(logline, 0);
		}
	}

	ochrs = 0;
	if (sscanf(line, "StartCharMetrics %d", &ochrs) < 1) {
/*		no longer a problem since we don't rely on it */
/*		sprintf(logline, "WARNING: Missing number: %s", line); */
	}

	if (passes == 0) nchrs = 0;	/* can't trust the above anyway 92/Oct/8 */

	flag = 0;

/*	Now do all characters in CharMetrics Section */
	while (getrealline(infile, line, MAXLINE) > 0) {
		if (strncmp(line, "Comment", 7) == 0) continue;	/* 92/Oct/8 */
/*		if (strstr(line, "EndCharMetrics") != NULL) break; */
/*		if (strncmp(line, "EndCharMetrics", 14) == 0) break;  */
		if (strncmp(line, "EndChar", 7) == 0) break; 
		if (sscanf(line, "C %d ;%n", &k, &n) < 1) {
			showline("Don't understand: ", 1);
			showline(line, 1);
			continue;
		}
		if (passes == 0) nchrs++;
		s = line + n;
		last = -1;									/* 92/Nov/26 */
		if (passes == 0) {
/*			if (strstr(line, "N space") != NULL) */ /* fixed 94/Oct/13 */
			if (strstr(line, "N space ") != NULL) { /* pick up space width */
				if (! spacefixed) {			/* 92/March/17 */
					if (! spaceseen) {
/*						sprintf(logline, "At byte %ld\n", ftell(infile)); */
						spaceguess(s);				/* separated 95/Oct/28 */
					}
				}	/* end of spacefixed == 0 */
			} /* end of N space found */
		} /* end of passes == 0 */
 /*		only care about encoded chars unless font is reencoded */
		if ((k >= 0 && k < fontchrs) || reencodeflag) {
			mat = sscanf(s, " WX %lg ; N %s ; B %lg %lg %lg %lg ;%n",
				&twidth, tencoding, &txll, &tyll, &txur, &tyur, &n);
			if (mat < 6) {
				if (mat < 2 || mat > 2) {
					sprintf(logline, "Don't understand: %s\n", s);
					showline(logline, 1);
					continue;
				}
				else {
					if (passes == 0) {
						if (strcmp(tencoding, ";") == 0) {
							showline("WARNING: Missing Character Name: ", 0);
							showline(line, 1);
							sprintf(tencoding, "a%d", k);
						}
						else {
							showline("WARNING: Missing BoundingBox: ", 0);
							showline(line, 1);
						}
/*						sprintf(logline, "tencoding %s\n", tencoding); */
					}
					missingboxes++;
					txll = 0.0; txur = twidth; 
/*					tyll = descender; tyur = ascender; */
					tyll = 0.0; 
					if (k >= 'a' && k <= 'z') tyur = xheight;
					else tyur = capheight;
				}
			}
/*			Maybe only do this for A-Z a-z ? */    
			if (alignmentflag) {	/* overshoot correction 97/Dec/21 */
				double tyllold=tyll, tyurold=tyur;
				if (tyll < 0.0) {
					if (tyll > 0.0 - alignment) tyll = 0.0;
					if (tyll < descender) {
						if (tyll > descender - alignment) tyll = descender;
					}
				}
				if (tyur > xheight) {
					if (tyur < xheight + alignment) tyur = xheight;
					else if (tyur > capheight) {
						if (tyur < capheight + alignment) tyur = capheight;
						else if (tyur > ascender) {
							if (tyur < ascender + alignment) tyur = ascender;
						}
					}
				}
				if (traceflag && showovershoot) {
					sprintf(logline, "%s\t(%lg %lg) --> (%lg %lg)\n",
					   tencoding, tyllold, tyurold, tyll, tyur);
					showline(logline, 0);
				}
			}
/* Absorb italic correction into total widths */
/* Preserves character widths, but keep superscript and subscript aligned */
			if (fullwidth) {
				if (txur + italicfuzz > twidth)	twidth = txur + italicfuzz; 
			}
			if (xscale != 1.0) {
				twidth = twidth * xscale;
				txll = txll * xscale;
				txur = txur * xscale;
			}
			removesemi(tencoding);
			s = s + n;
			if (sscanf (s, " I %lg", &italiccorrect) == 1)
/*				txur = twidth + italiccorrect;		/* ??? */
				txur = twidth + italiccorrect - italicfuzz;	/* 93/Nov/9 */
			
			kfirst = -1;	/* 93/Jan/5 */

			for (;;) {								/* 92/Nov/26 */
				if (last == MAXCHRS) break;
				if (reencodeflag) {
/*					if ((k = clookup(tencoding, k)) < 0) */
/*					if ((k = clookup(tencoding, last)) < 0) { */
					k = clookup(tencoding, last);
/* also check  width[k] != NOWIDTH) ? */
					if (k < 0) {
						k = MAXCHRS;	/* indicate failure */
						break;			/* pop out of this search loop */
					}
/*					avoid aliases for hyphen here ... 97/May/29 */
/*					but allow aliases for space here ... 98/Jul/12 */
					if (strcmp(encoding[k], tencoding) != 0 &&
						strcmp(tencoding, "space") != 0) {
						k = MAXCHRS;	/* indicate failure */
						break;			/* pop out of this search loop */
					}
				}
				if (kfirst < 0) kfirst = k;		/* remember the first one */
				if (k > ec) ec = k;
				if (k < bc) bc = k;
				assert(k >= 0 && k < fontchrs); 
				if (! reencodeflag)	{				/* 94/July/18 */
					if (strcmp(encoding[k], "") != 0) { /* 95/Feb/20 */
/*						if (debugflag) sprintf(logline, "free %d %s ", k, encoding[k]);*/
						free(encoding[k]);
						encoding[k] = "";	/* ? */
					}
					encoding[k] = zstrdup(tencoding);
					if (encoding[k] == NULL) return -1;	/* OUT OF MEMORY */
				}
				if (twidth < 0.0 && passes == 0) { 
					sprintf(logline, "\nWidth %lg in char %d?\n",
						twidth, k); 
					showline(logline, 0);
				}
/*				assert(twidth >= 0.0); */
				if (passes == 0 && width[k] != NOWIDTH) { /* 92/Sep/24 */
					newwidth = mapdouble(twidth);
					bad = 0;
/* It is bad if the widths don't match ! */
					if (newwidth != width[k]) bad = 1;
/* Serious problems if ligkern program on character with repeat encoding */
					if ((k >= 'A' && k <= 'Z') ||
						(k >= 'a' && k <= 'z')) bad = 1;
					if (bad) {
						sprintf(logline, "\n%s: Duplicate entry for char %d (%s) ",
							"ERROR", k, tencoding);
						showline(logline, 1);
					}
					else {
//						sprintf(logline, "%s (%d) # ", tencoding, k);
						sprintf(logline, "%s (%d) ", tencoding, k);
						showline(logline, 0);
						flag++;
/*						if ((flag % 6) == 0) showline("\n", 0); */
					}
					if (newwidth != width[k]) {
						sprintf(logline,
								"\nERROR: character width mismatch %ld <> %ld ",
								twidth, unmap(width[k]));
						showline(logline, 1);
					}
					duplicates++;
				}				
/*				width[k] = twidth;  */
				width[k] = mapdouble(twidth); 
/*				xll[k] = (float) txll;			yll[k] = (float) tyll; */
/*				xur[k] = (float) txur;			yur[k] = (float) tyur; */
				xll[k] = mapdouble(txll);		yll[k] = mapdouble(tyll);
				xur[k] = mapdouble(txur);		yur[k] = mapdouble(tyur);
				last = k;
				if (! reencodeflag) break;	/* no repetition */
			}							/* 92/Nov/26 */

/* if reencodeflag == 0 then kfirst will equal k above at this point */
			k = kfirst;					/* 93/Jan/5 */
/*			if (verboseflag) */		/* moved here 93/Jan/5 */
			if (traceflag) {		/* moved here 93/Jan/5 */
				char *s;
				if (k < 0) s = "-";
				else if (k >= fontchrs) s = ":";
				else s= "*";
				showline(s, 0);
			}
			if (k < 0 || k == MAXCHRS) continue;	/* ??? */
		
			if (traceflag)	{
				sprintf(logline, "%d ", k);		/* debugging */
				showline(logline, 0);
			}
			fchrs++;					/* count if in encoding */

			if (passes > 0) {			/* only do after first pass */
/*				s = s + n; */
				if (ligbegin[k] >= 0 && ligbegin[k] != lnext) {
					sprintf(logline, 
							"ERROR: Split lig program for char %s (%d)\n",
							encoding[k], k);	/* unlikely ! */
					showline(logline, 1);
					complain ();
				}
/*				assert(ligbegin[k] < 0 || ligbegin[k] == lnext); */
				ligbegin[k] = lnext;
/*				ignore ligatures in AFM file if suppressligs is non-zero */
				if (! suppressligs) {
				while (sscanf(s, " L %s %s ;%n", 
					ligsuccess, ligcombine, &n) == 2) {
					removesemi(ligcombine);
/*					sprintf(logline, "Noted ligature %s for %d ", s, k); */
#ifdef DEBUGGING
					if (traceflag) {
						sprintf(logline, "Noted ligature %s for %d ", s, k);
						showline(logline, 0);
					}
#endif
					s = s + n;
					nchara = clookup(ligsuccess, -1);
/*					sprintf(logline, "LIGSUCCESS %s %d ", ligsuccess, nchara); */
					if (nchara < 0)	continue;  /* second char not found */
/* Should we complain here or just quietly ignore it ? */
					if (width[nchara] == NOWIDTH) {
						sprintf(logline, "LIG %s NCHARA %s (%d) NO WIDTH\n", 
								tencoding, ligsuccess, nchara);
						showline(logline, 0);
						continue;
					}
					ncharb = clookup(ligcombine, -1);
/*					sprintf(logline, "LIGCOMBINE %s %d ", ligcombine, ncharb); */
					if (ncharb < 0) continue;  /* ligature not found */
/* Should we complain here or just quietly ignore it ? */
					if (width[ncharb] == NOWIDTH) {
						sprintf(logline, "LIG %s NCHARB %s (%d) NO WIDTH\n", 
								tencoding, ligcombine, ncharb);
						showline(logline, 0);
						continue;
					}
					ligsucc[lnext] = (char) nchara;		/* far space */
					ligature[lnext] = (char) ncharb;	/* far space */
#ifdef DEBUGGING
					if (traceflag) {
						sprintf(logline, "LIG: %s %s %s\n", 
							tencoding, ligsuccess, ligcombine);
						showline(logline, 0);
					}
#endif
/*					if (verboseflag) sprintf(logline, "Remainder %s\n", s); */
//					printf("%s + %s => %s\n", tencoding, ligsuccess, ligcombine); // debugging
					lnext++;
					if (lnext >= MAXLIG) { /* 255 - MAXLIG ? NO */
						sprintf(logline,
							"ERROR: Too many ligatures (> %d)\n", MAXLIG);
						showline(logline, 1);
						complain();
/*						lnext = 254; break; */
/*						exit(7); */		/* very unlikely ! */
						checkexit(7);	/* very unlikely ! */
						return -1;
					}
					if (abortflag > 0) return -1;
				}	/* end of while scanf ... */
				}	/* end of if suppreslig == 0 */
				ligend[k] = lnext;
/* possible problem with mismatch ? */
				replicate(k);	/* see if another character position */
								/* is this now redundant ??? */
			}	/* end of if passes > 0 */
		}	/* end of if k >= 0 ... */
		if (abortflag > 0) return -1;
	} /* end of while (getrealline(infile, line, MAXLINE) > 0)  */

	if (verboseflag) showline("\n", 0);
	if (flag > 0) {
		flag = flag / 2;
		showline("\n", 0);
		sprintf(logline,
				"WARNING: above %d char%s appear more than once\n",
				flag, (flag == 1) ? "" : "s");
		showline(logline, 0);
	}

/*  '*' in encoding, '-' unencoded, '.' not in encoding, ':' above 127 */
/*	if (verboseflag) showline("\n", 0); */
	if (bc > ec) {
		if (passes > 0) {
			showline("ERROR: No chars in AFM are in specified encoding!\n", 1);
			complain();
			return -1;
		}
		bc = ec;
	}

/*	if (ansiextend && bc > 18) bc = 18; */ 		/* 93/Dec/19 ??? */
	if (ansiextend && bc > 16) bc = 16; 		/* 93/Dec/29 ??? */

/*	if (ochrs != 0 && nchrs != ochrs) */	/* 92/Oct/8 */
	if (pass == 0 && ochrs != 0 && nchrs != ochrs) {	/* 92/Oct/8 */
		sprintf(logline, "WARNING: Saw %d characters, expected %d (%s)\n", 
			nchrs, ochrs, fn_in);
		showline(logline, 0);
		if (duplicates > 0) {
			sprintf(logline, "(There were %d repeat encodings)\n",
				duplicates/2);
			showline(logline, 0);
		}
	}

/*  now for kerning information ... */ /* sort first ? */
/*	if (verboseflag) sprintf(logline, "Ended on: %s", line); */
/*  forget the following, in case the fool forgot StartKernData ! */
/*	while (getrealline(infile, line, MAXLINE) > 0) {
		if (strstr(line, "StartKernData") != NULL) break;
	} */							/* ditched 93/July/10 */

/*	if (verboseflag) sprintf(logline, "Found: %s", line); */
	while (getrealline(infile, line, MAXLINE) > 0) {
		if (strstr(line, "StartKernPairs") != NULL) break;
	}

/*	if (verboseflag) sprintf(logline, "Found: %s", line); */
	nkernzero = 0;						/* may not be any kern info */
	if (strstr(line, "StartKernPairs") != NULL) {
		if (sscanf(line, "StartKernPairs %d", &nkernzero) < 1) {
			showline("Don't understand: ", 1);
			showline(line, 1);
		}
	}

/*	if (nkernzero >= (255 - lnext) && passes > 0) {
		sprintf(logline, 
		   "There may be too many kern pairs for old TFM format (%d >= %d)\n", 
				nkernzero, 255-lnext);
	} */
	
	if (traceflag) {
		sprintf(logline, "textfontflag %d\n", textfontflag);
		showline(logline, 0);
	}
	if (passes > 0) {
		nkern = readkerns(infile, rawkerns);	/* 92/Oct/12 */
		keepingacc=0;
		keepingbou=0;
/*		dumpkernpairs(rawkerns, nkern); */
		nkernzero = nkern;						/* override 97/Oct/1 */
/*		add boundary char ligatures with 'space' character and quotes */
		if (boundaryadd && textfontflag && ! isfixedpitch) {
			if (rightboundary < 0 && leftboundary < 0) {
				leftboundarychar = zstrdup("space");
				rightboundarychar = zstrdup("space");
				if (leftboundarychar == NULL || rightboundarychar == NULL) return -1;
				leftboundary = clookup(leftboundarychar, -1);
				rightboundary = clookup(rightboundarychar, -1);
			}
			if (rightboundary >= 0 || leftboundary >= 0) 
				addboundarykerns()	;	/* right place to do this ??? */
/* ??? 98/Jun/30 */
			if (leftboundary < 0) {
				if (leftboundarychar != NULL) {
					free(leftboundarychar);	
					leftboundarychar = NULL;
				}
			}
			if (rightboundary < 0) {
				if (rightboundarychar != NULL) {
					free(rightboundarychar);
					rightboundarychar = NULL;
				}
			}
		}
		nkernzero = nkern;		/* remember how many real kern pairs */
/*		Avoid producing accented versions of duplicate characters */
		if (replicateleftflag || replicaterightflag) {		/* 98/Apr/5 */
			sortkernpairs(rawkerns, nkern);		/* 92/Oct/12 */
			nkern = removeduplicates(rawkerns, nkern);	/* 97/Oct/1 */
		}
		nkernzero = nkern;						/* override 97/Oct/1 */
/*		Now replicated kerns for accented characters based on base character */
		if (replicateleftflag && textfontflag) 
			nkern = replicateleft(nkern, rawkerns);	/* 97/Oct/19 */
		if (replicaterightflag && textfontflag) 
			nkern = replicateright(nkern, rawkerns);	/* 97/Oct/19 */

		if (keepingacc) {
			sprintf(logline, 
	   "WARNING: AFM file already contains %d kern pair%s for %s characters\n",
				   keepingacc, (keepingacc == 1) ? "" : "s",
				   "accented");
			showline(logline, 0);
		}
		if (keepingbou) {
			sprintf(logline,
		"WARNING: AFM file already contains %d kern pair%s for %s characters\n",
				   keepingbou, (keepingbou == 1) ? "" : "s",
				   "boundary");
			showline(logline, 0);
		}

		nkernzero = nkern;						/* override 97/Oct/1 */
		if (presortflag) {
			sortkernpairs(rawkerns, nkern);		/* 92/Oct/12 */
#ifdef DEBUGKERNS
			dumpkernpairs(rawkerns, nkern);
#endif
			nkern = removeduplicates(rawkerns, nkern);	/* 97/Oct/1 */
#ifdef DEBUGKERNS
			dumpkernpairs(rawkerns, nkern);
#endif
			if (optimize)
				kernsave = findrepeats(rawkerns, nkern);	/* 97/Oct/1 */
			else kernsave=0;
		}
/*		nkernzero = nkern; */						/* ??? */
	}
	killkern = 0;		/* reset in case */

/*	nkern here is the wrong number to base decision on ... */
/*	if (nkern >= (255 - lnext) && passes > 0) { */
	if (nkern-kernsave >= (255 - lnext) && passes > 0) {
		if (traceflag) {
			sprintf(logline,
	"WARNING: Too many kern pairs for old TFM format (%d >= 255 - %d ligatures)\n", 
/*				nkern, 255-lnext); */
/*				nkern, lnext); */
				nkern-kernsave, lnext);
			showline(logline, 0);
		}
	}

/*	now process raw kerning data */	/* assuming grouped by first character */
	nlast = -1;						/* previous character code */
	skipchar=0;
	charkerns=0;
	for (k = 0; k < nkern; k++) {
		if (passes == 0) continue;	/* skip this stuff first time around */
		nchara = rawkerns[k].a;
		ncharb = rawkerns[k].b;
		kerndist = rawkerns[k].kern;
		if (nchara != nlast) {			/* start on new first letter */
			skipchar = 0;
/*			NOTE: kern pairs MUST be grouped by first letter */
/*			assert(kernbegin[nchara] < 0 || kernbegin[nchara] == knext); */
			if (kernbegin[nchara] >= 0 && kernbegin[nchara] != knext) {
				sprintf(logline, 
					"ERROR: Split kern program for char %s (%d)\n",
						encoding[nchara], nchara); 
				showline(logline, 1);
				showline("ERROR: Kern pairs must be sorted on first char\n", 1);
				complain();
			}
			kernbegin[nchara] = knext;
			kernend[nchara] = knext;	/* in case it doesn't get filled in */
/* 			if (nlast >= 0) kernend[nlast] = knext; */
			nlast = nchara;
			charkerns = 0;
/*			Is kern program the same as one earlier ? */
			if ((keqv = kerneqv[nchara]) != NOEQUIV) {
/*				only do this is if neither character has lig program */
				if (ligend[nchara] == ligbegin[nchara]){
					if (ligend[keqv] == ligbegin[keqv]) {
						skipchar=1;
/*						continue; */
					}
/* if kern programs the same, but has a ligature program also 97/Oct/15 */
/*					else if (verboseflag) */
					else if (traceflag) {
						sprintf(logline, "SPECIAL LIG CASE %s (%d) %s (%d)\n",
								encoding[nchara], nchara,
								encoding[keqv], keqv); /* debugging */
						showline(logline, 0);
					}
				}
			}
		} 

		if (skipchar) continue;						/* 97/Oct/1 */

		charkerns++;
		if (charkerns < maxcharkern && ! killkern) { 
			kernsucc[knext] = (char) ncharb;		/* far space */
/*			kern[knext] = (float) kerndist; */ /* float */ 
			kern[knext] = kerndist;
			knext++;
			kernend[nchara] = knext;
			if (showkernflag) {
				sprintf(logline, "knext %d KPX %s %s %lg \n",
					   knext, encoding[nchara], encoding[ncharb],
					   unmap(kerndist));
				showline(logline, 0);
			}
/*			if (knext >= MAXKERN)  */ /* MAXKERN */
			if (knext >= kernligsize) { 
				sprintf(logline, 
					"WARNING: Truncating all kern pairs after char %d\n", 
						nchara);
				showline(logline, 0);
/*				sprintf(logline, 
					"(ERROR: More than %d kern pairs)\n", MAXKERN); */
				sprintf(logline, 
					"(ERROR: More than %d kern pairs)\n", kernligsize);
				showline(logline, 1);
				complain();
				kernend[nchara] = knext; /* needed ? */
				killkern++;			/* should not happen ! */
			}
			if (backwardflag && knext >= 255 - lnext) { /* MAXKERN */
				sprintf(logline, 
					"WARNING: Truncating all kern pairs after char %d\n", 
						nchara);
				showline(logline, 0);
				showline(
				"(old style TFM can contain only 256 ligs & kerns)\n", 1);
				complain();
				kernend[nchara] = knext; /* needed ? */
				killkern++;			/* should not happen */
			}

		}
		else if (charkerns == maxcharkern && ! killkern) {
			sprintf(logline, "Truncating kern pairs for char %d\n", nchara);
			showline(logline, 0);
			kernend[nchara] = knext;		/* needed ? */
			kerntruncated++;
		}
		else kerntruncated++;
	}

/*	have now read all kerning information - print out statistics */
	if (verboseflag && passes > 0 && nkern > 0) {
		if (kernsave > 0) {
			sprintf(logline, "Used %d + %d = %d kerns - ",
								  knext, kernsave, knext + kernsave);
			showline(logline, 0);
		}
		else {
			sprintf(logline, "Used %d kerns - ", knext);
			showline(logline, 0);
		}
		if (kernzeros > 0) {
			sprintf(logline, "ignored %d zero size kerns - ", kernzeros);
			showline(logline, 0);
		}
		if (kernignored > 0) {
			sprintf(logline, "ignored %d tiny kerns - ", kernignored);
			showline(logline, 0);
		}
		if (kerntruncated > 0) {
			sprintf(logline, "truncated %d extra kerns - ", kerntruncated);
			showline(logline, 0);
		}
/*		sprintf(logline, "(seen %d kerns)\n", nkern); */
/*		sprintf(logline, "(out of %d kerns)\n", nkernzero); */
/*		sprintf(logline, "(out of %d)\n", nkernzero); */
		sprintf(logline, "(out of %d)\n",
				nkernzero+kernzeros+kernignored+kerntruncated);
		showline(logline, 0);
	}
	nkern = knext;
/*	if (nlast >= 0) kernend[nlast] = knext; */

/*	we don't really need to read the rest of the AFM file */

#ifdef IGNORED
	while (getrealline(infile, line, MAXLINE) > 0) {
		if (strstr(line, "EndKernData") != NULL) break;
	}
/*	Now read up to composite characters */
	while (getrealline(infile, line, MAXLINE) > 0) {
		if (strstr(line, "StartComposites") != NULL) break;
	}
/*  now read over composite characters */
	while (getrealline(infile, line, MAXLINE) > 0) {
		if (strstr(line, "EndComposites") != NULL) break;
	}	
/*	read to end of file */
	while (getrealline(infile, line, MAXLINE) > 0) {
		if (strstr(line, "EndFontMetrics") != NULL) break;
	}		
#endif

	if (traceflag) {
		sprintf(logline, "XSCALE is %lg\n", xscale);	/* DEBUGGING ONLY */
		showline(logline, 0);
	}
	if (missingboxes) {
		showline(
		"WARNING: Missing BoundingBoxes => inaccurate height & depth\n", 0);
	}
	return 0;		 /* succesfully read */
}

void listmissing (void) {
	int k, flag=0, ncol=0;

	for (k = 0; k < fontchrs; k++) {
		if (width[k] == NOWIDTH) {
			if (strcmp(encoding[k], "") != 0) flag++;
		}
	}
	if (flag == 0) return;		/* none missing */

	showline("\n", 0);
	flag = 0; 
	for (k = 0; k < fontchrs; k++) {
		if (width[k] == NOWIDTH) {
			if (strcmp(encoding[k], "") != 0) {
				sprintf(logline, "%s ? ", encoding[k]);
				ncol += strlen(logline);
				if (ncol >= 78) {
					showline("\n", 0);
					ncol = strlen(logline);
				}
				showline(logline, 0);
				flag++; 
/*				if ((flag % 6) == 0) showline("\n", 0); */
			}
		}
	}
	showline("\n", 0);
	sprintf(logline,
	"WARNING: above %d char%s in encoding vector, but not in AFM file\n", 
		flag, (flag == 1) ? "" : "s");
	showline(logline, 0);
	showline("(or possibly repeated entries in encoding vector)\n", 0);
}

/* Try and add TeX standard `ligatures' */

int addligatures(char *ligatures[][3], int accented, int nligtype) {	
	int count=0;					/* debugging */
	int k, m, nchara, ncharb;
	int kbase=-1;						/* 95/Jan/22 */
	int kold=-1;					/* index of entry with base name */
	char *ligbase="", *ligsuccess="", *ligcombine="";
	char *oldbase="";					/* not accessed - debug only */
	char composite[MAXCHARNAME];		/* for constructing composite */
	
/*	if (suppressligs && ! allowligs) return; */	/* 93/May/29 */
/*	no, if user explicitly asks for it, then do it ! */

/*	for(m = tstart; m < tend; m++) {  */
	for(m = 0; m < 256; m++) { 
//		printf("Considering %s + %s => %s\n",
//			   ligatures[m][0], ligatures[m][1], ligatures[m][2]);
		if (strcmp(ligatures[m][0], "") == 0) {		/* keep old ligbase ? */
			if (strcmp(ligatures[m][1], "") == 0) break;	/* termination */
			if (kbase < 0) continue;	/* base does not exist ! 95/Jan/22 */
			if (kold < 0) {
/*				not an error, just means base was not found ? */
/*				sprintf(logline, "Bad ligature table %d\n", m); */
			}
			k = kold;
		}
		else {
			ligbase = ligatures[m][0];				/* base character */
/*			if (strcmp(ligbase, "") == 0) break; */	/* termination */
			k = clookup(ligbase, -1); // also check width != NOWIDTH ?
			kbase = k;				/* rem whether base exist ! 95/Jan/22 */
/*			kold = k;  */ /* ??? */
		}
		if (k < 0 || width[k] == NOWIDTH) {		/* ? */
#ifdef SHOWTEXBASE
			if (traceflag) {
				sprintf(logline, "LIGBASE %s not in encoding\n", ligbase);
				showline(logline, 0);
			}
#endif
			kbase = -1;				/* if not already ! 95/Jan/22 */
//			printf("LIGBASE %s not in encoding\n", ligbase); // debugging
			continue;				/* ligbase not in encoding */
		}
/*		is this a new base character ?  */
		if (k != kold) {
			if (kold != -1) {		/* if so finish last one */
#ifdef SHOWTEXBASE
				if (traceflag) {
					sprintf(logline, "LIGBASE %s ends at %d\n", oldbase, lnext);
					showline(logline, 0);
				}
#endif
				ligend[kold] = lnext;
/* possible problem with mismatch ? */ /* DEBUGGING TAKE OUT TEST */
				replicate(kold);	/* see if another character position */
			}
/*			if (ligbegin[k] != ligend[k] && ligend[k] != lnext) {  */
			if (ligbegin[k] != ligend[k]) { 
#ifdef SHOWTEXBASE
				if (traceflag) {
					sprintf(logline, "LIGBASE %s, LIGBEGIN %d, LIGEND %d LNEXT %d\n", 
						ligbase, ligbegin[k], ligend[k], lnext);
					showline(logline, 0);
				}
#endif
				continue;					/* avoid split ligature	prog */
			}
#ifdef SHOWTEXBASE
			if (traceflag) {
				sprintf(logline, "LIGBASE %s begins at %d\n", ligbase, lnext);
				showline(logline, 0);
			}
#endif
			assert(ligbegin[k] == ligend[k]);
			ligbegin[k] = lnext;		/* start program for char k */
			ligend[k] = lnext;			/* dummy end program for now */
			kold = k;		/* remember character code working on now */
			oldbase = ligbase;
		}
		ligsuccess = ligatures[m][1];
		nchara = clookup(ligsuccess, -1);
		if ((! allowdotlessi && strcmp(ligsuccess, "dotlessi") == 0) ||
//			(allowdottedi && strcmp(ligsuccess, "i") == 0)) 
			(! allowdottedi && strcmp(ligsuccess, "i") == 0)) { // fix 99/Oct/1
			if (traceflag) {
				sprintf(logline, "Ignoring %s %s\n", ligbase, ligsuccess);
				showline(logline, 0);
			}
			continue;
		}
		if (nchara < 0 || width[nchara] == NOWIDTH) {
#ifdef SHOWTEXBASE
			if (traceflag) {
				sprintf(logline, "LIGBASE %s SUCCESS %s not found\n", 
					ligbase, ligsuccess);
				showline(logline, 0);
			}
#endif
			continue;			/* second char not found */
		}
		ligcombine = ligatures[m][2];
		if (accented) {		/* special for accented characters */
			strcpy(composite, ligsuccess);
/* following needed if pseudo ligatures based on `dotlessi' instead of `i' */
			if (strcmp(composite, "dotlessi") == 0)
				strcpy(composite, "i");
/* following needed if pseudo ligatures based on `degree' instead of `ring' */
/*			if (strcmp(ligbase, "degree") == 0) strcpy(ligbase, "ring"); */
			strcat(composite, ligbase);
			ligcombine = composite;
		}
		ncharb = clookup(ligcombine, -1);
		if (ncharb < 0 || width[ncharb] == NOWIDTH) {
#ifdef SHOWTEXBASE
			if (traceflag) {
				sprintf(logline, "LIGBASE %s LIGAT %s not found\n", 
					ligbase, ligcombine);
				showline(logline, 0);
			}
#endif
			continue;			/* ligature not found */
		}
#ifdef SHOWTEXBASE
		if (traceflag) {
			sprintf(logline, "LIGBASE %s SUCCESS %s LIGATURE %s LNEXT %d\n",
				ligbase, ligsuccess, ligcombine, lnext);
			showline(logline, 0);

		}
#endif
		ligsucc[lnext] = (char) nchara;		/* far space */
		ligature[lnext] = (char) ncharb;	/* far space */
//		printf("Using       %s + %s => %s\n",
//			   ligbase, ligsuccess, ligcombine); // debugging
		lnext++;
		if (lnext >= MAXLIG) { /* 255 ? MAXLIG ? NO */
			sprintf(logline,
				"ERROR: Too many ligatures (> %d)\n", MAXLIG);
			showline(logline, 1);
			complain();
/*			exit(7); */		/* very unlikely ! */
			checkexit(7);	/* very unlikely ! */
			return -1;
		}
		count++;					/* debugging */
	} 
/*	is this the last ligature for this base character ? */
	if (kold != -1) {
#ifdef SHOWTEXBASE
		if (traceflag) {
			sprintf(logline, "LIGBASE %s ends at %d\n", oldbase, lnext);
			showline(logline, 0);
		}
#endif
		ligend[kold] = lnext;
/* possible problem with mismatch ? */ /* DEBUGGING TAKE OUT TEST */
		replicate(kold); 	/* see if another character position */
	}
/* following is debugging stuff */
	if (traceflag) {
		char *ligtype;
		switch (nligtype) {
			case 1: ligtype = "f"; break;
			case 2: ligtype = "dash and quote"; break;
			case 3: ligtype = "guillemet"; break;
			case 4: ligtype = "accent"; break;
			case 5: ligtype = "accent"; break;
			default: ligtype = "unknown"; break;
		}
		if (accented) {
			sprintf(logline, "%d accent pseudo ligatures added\n", count);
			showline(logline, 0);
		}
		else {
			sprintf(logline, "%d %s ligatures added\n", count, ligtype);
			showline(logline, 0);
		}
	}
	return 0;
}

int expandbuffer (void) {		/* expand TFM file output buffer */
	unsigned char __far *newbuffer;
	unsigned int newsize;
	
	newsize = buffersize + INCMEMORY;
	newbuffer = (unsigned char __far *) _frealloc (buffer, newsize);
	if (newbuffer == NULL) {
		sprintf(logline, 
			"ERROR: Unable to allocate memory for %s\n", "TFM file");
		showline(logline, 1);
		checkexit(1);
		return -1;
	}
	else {
		buffer = newbuffer;
		buffersize = newsize;
		if (traceflag) {				/* debugging */
			sprintf(logline, "Expanded TFM file buffer to %u\n", newsize);
			showline(logline, 0);
		}
		return newsize;
	}
}

/* *** *** *** goodies for writing into the output TFM buffer *** *** *** */

int writeint(int word) {	 /* write 16-bit count into buffer */
	int ret;
	if (bufferinx + 1 >= buffersize) {
		ret = expandbuffer();
		if (ret < 0) return -1;
	}
	buffer[bufferinx++] = (unsigned char) ((word >> 8) & 255);
	buffer[bufferinx++] = (unsigned char) (word & 255);
/*	if (bufferinx >= MAXBUFFER) overflow(); */
	return 0;
}

int writelong(long word) {		/* write 32-bit number into buffer */
	int ret;
	if (bufferinx + 3 >= buffersize) {
		ret = expandbuffer();
		if (ret < 0) return -1;
	}
	buffer[bufferinx++] = (unsigned char) ((word >> 24) & 255);
	buffer[bufferinx++] = (unsigned char) ((word >> 16) & 255);
	buffer[bufferinx++] = (unsigned char) ((word >> 8) & 255);	
	buffer[bufferinx++] = (unsigned char) (word & 255);	
/*	if (bufferinx >= MAXBUFFER) overflow(); */
	return 0;
}

/* writefour(skipbyte, nextchar, opbyte, remainder); */

int writefour(int a, int b, int c, int d) {
	int ret;
	if (bufferinx + 3 >= buffersize) {
		ret = expandbuffer();
		if (ret < 0) return -1;
	}
	buffer[bufferinx++] = (unsigned char) a;
	buffer[bufferinx++] = (unsigned char) b;
	buffer[bufferinx++] = (unsigned char) c;	
	buffer[bufferinx++] = (unsigned char) d;	
/*	if (bufferinx >= MAXBUFFER) overflow(); */
	return 0;
}

/* special lig_kern program step that points to start of actual program */

int writepointer(int place, int ligkerin) {
	int ret;
	if (place + 3 >= buffersize) {
		ret = expandbuffer(); 	/* should not ! */
		if (ret < 0) return -1;
	}
	buffer[place++] = 128 + 64;	/* > 128 */			/* skip byte */
	buffer[place++] = 0;							/* next byte */
	buffer[place++] = (char) ((ligkerin >> 8) & 255);	/* op byte */
	buffer[place++] = (char) (ligkerin & 255);		/* remainder */
	if (ligkerin > maxligkern) maxligkern = ligkerin; /* debugging */
/*	if (verboseflag) sprintf(logline, "ligkerin %d ", ligkerin); */
	return 0;
}

void markend(void) {	/* mark end of lig/kern program */
	if (bufferinx > 3)
		buffer[bufferinx - 4] =	
		   (unsigned char) (buffer[bufferinx - 4] | STOPBIT);	 /* 92/Nov/24 */
}

/* write pascal style string */ /* starts with byte count - pad with zeros */
/* null delimited string, how many bytes, how many bytes total (not count) */

int writepascal(char *s, int n, int m) {
	int k;
	int ret;

	if (bufferinx + m + 1 >= buffersize) {
		ret = expandbuffer(); /* 98/Aug/26 */
		if (ret < 0) return -1;
	}
	if (n > m) n = m;						/* sanity check */ 
	buffer[bufferinx++] = (unsigned char) n;	/* byte count */
	for (k=0; k < n; k++) buffer[bufferinx++] = (unsigned char) *s++;
	for (k=n; k < m; k++) buffer[bufferinx++] = '\0';
	return 0;
}

int longalign(void) {		/* buffer must be multiple of four long */
	int ret;

	if (bufferinx + 3 >= buffersize) {
		ret = expandbuffer(); 	/* should not ! */
		if (ret < 0) return -1;
	}
	if ((bufferinx % 4) != 0) {
		sprintf(logline, "Header not word aligned %d\n", bufferinx);
		showline(logline, 0);
	}
	while ((bufferinx % 4) != 0) bufferinx++;
	return 0;
}

int writetfm(FILE *outfile) {
	int k;
/*	unsigned char *bufptr = buffer; */
/*	unsigned char __far *bufptr = buffer; */
	unsigned char __far *bufptr = buffer;
#ifdef DEBUGGING
	int a, b, c, d;
	unsigned int e, f;
	long g;
	double h;
#endif

#ifdef DEBUGGING
	for (k = 0; k < lf; k++) {
		a = *bufptr++; b = *bufptr++; c = *bufptr++; d = *bufptr++;
		if (traceflag) { 
			pause();
			e = (a << 8) | b ; f = (c << 8) | d;
			g = ((long) e) << 16 | (long) f;
			h = 1000.0 * ((double) g) / 1048576.0;
			sprintf(logline, "%d\t%d\t%d\t%d => \t%u\t%u => \t%ld \t(%lg)\n",
				a, b, c, d, e, f, g, h);
			showline(logline, 0);
		}
		putc(a, outfile);
		putc(b, outfile);
		putc(c, outfile);
		putc(d, outfile);
	}
#else
/*	fwrite(bufptr, 4, lf, outfile); */ /* bufptr is _far ... */
	for (k = 0; k < lf * 4; k++) putc(*bufptr++, outfile);
#endif
	return 0;
}

/* *** *** *** goodies for writing into the output TFM buffer *** *** *** */

/* make up packed char_info_word */ /* k only needed for debug output */

unsigned long packcharinfo(int width, int height, int depth, int italic, 
	int tag, int remainder, int k) {
	unsigned long res;

	assert (width >= 0);  assert (height >= 0);
	assert (depth >= 0);  assert (italic >= 0); 
	assert (width < 256); assert (height < 16);
	assert (depth < 16);  assert (italic < 64); 
	if (remainder < 0 || remainder > 255) {
		sprintf(logline, "ERROR: Remainder %d out of range in char %d\n", 
			remainder, k);
		showline(logline, 1);
/*		sprintf(logline, "Width %d Height %d Depth %d Italic %d Tag %d\n",
			width, height, depth, italic, tag);	*/	/* debugging */
/*		sprintf(logline, "ligkerninx %d nextslot %d\n", ligkerninx, nextslot);*/
		complain();
		remainder = remainder & 255;		/* just to be safe */
	}
	res = ((long) width << 24) | ((long) height << 20) | 
			((long) depth << 16) | ((long) italic << 10) | 
			  (tag << 8) | remainder;
#ifdef DEBUGGING
	if (traceflag) {
		sprintf(logline, "%d %d %d %d %d %d => %lo\n",
				width, height, depth, italic, tag, remainder, res);
		showline(logline, 0);
		pause();
	} 
#endif
	return res;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef DEBUGGING
/* void showtable(double table[], int n) { */
void showtable(long table[], int n) {
	int k;
	if (n == 0) return;
	showline("\n", 0);
#ifdef DEBUGGING
	pause();
#endif
	for (k = 0; k < n; k++) {
		if ((k % 6) == 0) showline("\n", 0);
/*		sprintf(logline, "%d w: %lg ", k, table[k]); */
		sprintf(logline, "%d w: %lg ", k, unmap(table[k]));
		showline(logline, 0);
	}
}

void showwidths(void) {
	showtable(widthtable, nw);
}

void showheights(void) {
	showtable(heighttable, nh);
}

void showdepths(void) {
	showtable(depthtable, nd);
}

void showitalics(void) {
	showtable(italictable, ni);
}

void showkerns(void) {
	showtable(kerntable, nk);
}
#endif

/* int comparedouble(const void *dbla, const void *dblb) {
	const double *a, *b;

	a = (const double *) dbla;
	b = (const double *) dblb;

	if (*a < *b) return -1; 
	else if (*a > *b) return 1;
	else return 0;
} */

int comparelong(const void *dbla, const void *dblb) {
	const long *a, *b;

	a = (const long *) dbla;
	b = (const long *) dblb;

	if (*a < *b) return -1; 
	else if (*a > *b) return 1;
	else return 0;
}

/* for width and kern can expect exact match - since tables big enough */

/* int mapwidth(double charwidth) {	*/		/* get width index from width */
int mapwidth(long charwidth) {				/* get width index from width */
	int k;
/*	double *ptr; */
	long *ptr;

/* 	assert(charwidth >= 0.0 && charwidth < 4096.0); */
/*	assert(charwidth >= -4096.0 && charwidth < 4096.0); */
/*	if (charwidth == 0.0) return 0; */

/*	ptr = bsearch(&charwidth, widthtable, (unsigned int) nw, sizeof(double), 
		comparedouble); */
	ptr = bsearch(&charwidth, widthtable + 1, (unsigned int) (nw - 1), 
/*				sizeof(double), comparedouble);  */
				sizeof(long), comparelong);

	if (ptr == NULL) {
		sprintf(logline, 
			"ERROR: Can't find %s %lg in table\n", "width", charwidth);
		showline(logline, 1);
#ifdef DEBUGGING
		if (verboseflag) showwidths();	/* may want this ? */
#endif
/*		checkexit(7); */
		ptr = widthtable;
	}
	k = (int) (ptr - widthtable);
#ifdef DEBUGGING
/*	if (charwidth == 0.0 && verboseflag) {	 */
	if (charwidth == 0 && verboseflag) {	
		sprintf(logline, "Char %d width %d widthindex %d\n",
				chr, unmap(charwidth), k);
		showline(logline, 0);
	}
#endif
/*	return (int) (ptr - widthtable);  */
	return k;
}

/* for width and kern can expect exact match - since tables big enough */

/* int mapkern(double charkern) {	*/		/* get kern index from kern */
int mapkern(long charkern) {				/* get kern index from kern */
/*	double *ptr;  */
	long *ptr; 

	if (nk == 0) {
		showline("No kerning table, but kern?\n", 1); 
/*		exit(7); */
		checkexit(7);
		return 0;		/* ??? */
	}
	ptr = bsearch(&charkern, kerntable, (unsigned int) nk, 
/*				sizeof(double), comparedouble); */
				sizeof(long), comparelong);
	if (ptr == NULL) {
/*		sprintf(logline, "Can't find kern %lg in table", kern); */
		sprintf(logline, "Can't find %s %lg in table\n",
				"kern", unmap(charkern)); 
		showline(logline, 1);
#ifdef DEBUGGING
		showkerns();
#endif
/*		exit(7); */
		checkexit(7);
		ptr = kerntable;
	}
	return (int) (ptr - kerntable); 
}

/* can't use bsearch here - since may not be exact match: */

/* int mapheight(double charheight) { */	/* get height index from height */
int mapheight(long charheight) {			/* get height index from height */
	int k;
/*	int kmin=-1;
	double mindist=10000.0; */

/*	if (charheight == 0.0) return 0; */
	if (charheight == 0) return 0;
	for (k = 1; k < nh; k++) {
		if (charheight <= heighttable[k]) return k;	/* pick next highest */
/*		if (fabs(charheight-heighttable[k]) < mindist) {
			mindist = fabs(charheight-heighttable[k]); kmin = k; 
			} */
	}
/*	sprintf(logline, "Can't find height %lg in table\n", charheight); */
	sprintf(logline, "Can't find %s %lg in table\n",
			"height", unmap(charheight));
	showline(logline, 1);
	return nh-1;		/* nonsense ? */

/*	if (kmin < 0) {
		sprintf(logline, "Negative index in mapheight ?\n");
		checkexit(3);
	}
	else return kmin; */
}

/* can't use bsearch here - since may not be exact match: */

/* int mapdepth(double chardepth) {	*/	/* get depth index from depth */
int mapdepth(long chardepth) {		/* get depth index from depth */
	int k;
/*	int kmin=-1;
	double mindist=10000.0; */

	if (chardepth == 0) return 0;
	for (k = 1; k < nd; k++) {
		if (chardepth <= depthtable[k]) return k;	/* next highest */
/*		if (fabs(chardepth - depthtable[k]) < mindist) {
			mindist = fabs(chardepth - depthtable[k]); kmin = k;
		} */
	}
/*	sprintf(logline, "Can't find depth %lg in table\n", chardepth); */
	sprintf(logline, "Can't find %s %lg in table\n",
			"depth", unmap(chardepth));
	showline(logline, 1);
	return nd-1;		/* nonsense ? */

/*	if (kmin < 0) {
		sprintf(logline, "Negative index in mapdepth ?\n");
		checkexit(3);
	}
	else return kmin; */
}

/* can't use bsearch here - since may not be exact match: */

/* int mapitalic(double charitalic) { */		/* get italic index from italic */
int mapitalic(long charitalic) {		/* get italic index from italic */
	int k;
/*	int kmin=0;
	double mindist=10000.0; */

	if (! italicneeded) return 0;
/*	if (charitalic == 0.0) return 0; */
	if (charitalic == 0) return 0;
	for (k = 1; k < ni; k++) {
		if (charitalic <= italictable[k]) return k; /* next highest */
/*		if (fabs(charitalic-italictable[k]) < mindist) {
			mindist = fabs(charitalic-italictable[k]); kmin = k;
		} */
	}
/*	sprintf(logline, "Can't find italic %lg in table\n", charitalic);*/
	sprintf(logline, "Can't find %s %lg in table\n",
			"italic", unmap(charitalic));
	showline(logline, 1);
	return ni-1;				/* nonsense ? */

/*	if (kmin < 0) {
		sprintf(logline, "Negative index in mapitalic ?\n");
		checkexit(3);
	}
	else return kmin; */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int updateinx(int k) {				/* update lig/kern program index */
	int ligadd, kernadd;
	ligadd = (ligend[k] - ligbegin[k]);
	kernadd = (kernend[k] - kernbegin[k]);
	if (ligadd == 0 && kernadd == 0) return 0; 	/* 98/Jun/30 */
	ligkerninx += ligadd;	/* space for ligs */
	ligkerninx += kernadd;	/* space for kerns */
	if (traceflag) {
		sprintf(logline, "Space for %d ligs and %d kerns char %s (%d)\n",
			   ligadd, kernadd, encoding[k], k);
		showline(logline, 0);
	}
	return ligadd+kernadd;
}

/* construct one character information word and fill in ligs and kerns */

unsigned long constructchar(int k) {
	int chartag, charremainder;
	int keqv, kprog;
	int kwidth, kheight, kdepth, kitalic;
	int reuseflag;
	unsigned long res;
	int oldslot, oldligkern;

	if (traceflag) {
		sprintf(logline, "ConstructChar %s (%d):\n", encoding[k], k);
		showline(logline, 0);
	}
	kprog = k;			/* normal case */
	reuseflag = 0;

	oldslot = nextslot;			/* remember old one for debugging out */
	oldligkern = ligkerninx;	/* remember old one for debugging out */

/*	Is kern program the same as one earlier ? */
	if ((keqv = kerneqv[k]) != NOEQUIV) {
/*		only do this is if neither character has lig program */
		if (ligend[k] == ligbegin[k]) {
			if (ligend[keqv] == ligbegin[keqv]) {
				kprog = keqv;
				reuseflag = 1;			/* OK, reuse existing one */
			}
/*			else if (verboseflag) */
			else if (traceflag) {
				sprintf(logline, "SPECIAL LIG CASE %s (%d) %s (%d)\n",
						encoding[k], k,
						encoding[keqv], keqv); /* debugging */
				showline(logline, 0);
			}
		}
	}
	
	if (ligend[kprog] == ligbegin[kprog] &&
		kernend[kprog] == kernbegin[kprog]) {
		chartag = 0; charremainder = 0;	/* no ligature or kern prog needed */
		if (traceflag) {
			sprintf(logline, "No ligkern prog for char %s (%d)\n",
				   encoding[kprog], kprog);
			showline(logline, 0);
		}
	}
	else {					/* does have a lig or kern program */
#ifdef DEBUGGING
		if (traceflag) {
			sprintf(logline, "ls %d le %d ks %d ke %d\n", 
			ligbegin[kprog], ligend[kprog], kernbegin[kprog], kernend[kprog]);
			showline(logline, 0);
			pause();
		}
#endif
		chartag = 1;		/* indicate lig/kern program */
		if (reuseflag)		/* if we reuse a program, just link to same */
			charremainder = charrem[kprog];	/* hope already set ! */
		else if (longtable) {
			charremainder = nextslot;	/* point to forwarding table */
			nextslot++;
			if (rightboundary >= 0 && textfontflag)		/* 98/Apr/5 */
				charremainder++;
/*			may want to check that charremainder doesn't overflow 255 */			
/*			sprintf(logline, "k %d ligkerninx %d ", k, ligkerninx); */
/*			update indirect pointers here ? writepointer ? do later */
		}
		else charremainder = ligkerninx;	/* direct to ligkern program */
		
/*		this should not happen ... */
		if (charremainder < 0 || charremainder > 255) {
			sprintf(logline, 
					"ERROR: charremainder %d out of range (char %d)\n", 
					charremainder, k);
			showline(logline, 1);
			complain();
			charremainder = 255; 
		}
		charrem[k] = (unsigned char) charremainder;	/* record for reuse */
		if (! reuseflag)
			(void) updateinx(k);	/* space for lig/kern program */
/*		don't do if using program that already appears earlier 97/Oct/1 */
	}
/*	following is redundant ? */
/*	if (width[k] < 0.0 || width[k] >= 4096.0) { */
/*	if (width[k] >= 4096.0) { */
	if (unmap(width[k]) >= 4096.0) {
/*		sprintf(logline, "Width %lg in char %d?\n", width[k], k); */
		sprintf(logline, "Width %lg in char %d?\n", unmap(width[k]), k);
		showline(logline, 0);
	}
	kwidth = mapwidth(width[k]); 
	kheight = mapheight(height[k]);
	kdepth = mapdepth(depth[k]); 
	kitalic = mapitalic(italic[k]);
	if (extenpointer[k] >= 0) {
		if (chartag) {
/*			an extensible character cannot have ligs or kerns */
			sprintf(logline, "Extensible char %d has ligs or kerns!\n", k);
			showline(logline, 1);
		}
		chartag = 3;	/* Extensible character */
		charremainder = extenpointer[k];
	}
	if (ascendpointer[k] >= 0) {
		if (chartag) {
/*			an ascending character cannot have ligs or kerns */
			sprintf(logline, "Ascending char %d has ligs or kerns!\n", k);
			showline(logline, 1);
		}
		chartag = 2;	/* Ascending character */
		charremainder = ascendpointer[k];
		if (charremainder == 0) {
			sprintf(logline, "char %d charremainder %d\n", k, charremainder);
			showline(logline, 0);
#ifdef DEBUGGING
			pause();
#endif
		}
	}
/*	pack it all into one TFM file `word' */
	res = packcharinfo(kwidth, kheight, kdepth, kitalic,
					   chartag, charremainder, k);
/* #ifdef DEBUGGING */
	if (traceflag) { 
		sprintf(logline,
"K %3d w %2d h %2d d %2d i %2d t %1d r %3d (%10lo) nextslot %2d ligkerninx %3d\n",
				k, kwidth, kheight, kdepth, kitalic, chartag, charremainder, res,
				oldslot, oldligkern);
/*		(void) _getch(); */
		showline(logline, 0);
	}
/* #endif */
	return res;
}

/* how many characters need lig/kern program ? */

int howmany(void) {
	int k, keqv, count=0;

	for (k = bc; k <= ec; k++) {
/*		ignore the ones that have equivalents */
		if ((keqv = kerneqv[k]) != NOEQUIV) {
/*			but only if neither has a ligature program */
			if (ligend[k] == ligbegin[k]) {
				if (ligend[keqv] == ligbegin[keqv]) {
					continue;
				}
/*				else if (verboseflag) */
				else if (traceflag) {
					sprintf(logline, "SPECIAL LIG CASE %s (%d) %s (%d)\n",
							encoding[k], k,
							encoding[keqv], keqv); /* debugging */
					showline(logline, 0);
				}
			}
		}
		if ((ligend[k] != ligbegin[k]) || (kernend[k] != kernbegin[k])) count++;
	}
	return count;
}

int writepreamb(void) {
	bufferinx = 0;
/*	nw = ec - bc + 1; */
	if (suppressligs && 
		! texligflag && ! pseudoligs &&			/* 95/Jan/22 */
			! extendligs && ! accentligs &&
				lnext != 0) {
		sprintf(logline, "No ligatures %d?\n", lnext);
		showline(logline, 0);
	}

/*	nl = lnext + knext; */	/* ligatures + kerns */

	if (nl > 255) {
		if (backwardflag) {
			sprintf(logline, 
				"ERROR: Too many ligs (%d) & kerns (%d) ( > 255)\n", 
					lnext, knext);
			showline(logline, 1);
			complain();
		}
		else {
/*			if (verboseflag) { */
			if (traceflag) {						/* 97/Oct/26 */
				sprintf(logline,
"WARNING: Extending lig (%d) & kern (%d) table (nl %d + %d) (requires TeX 3.0)\n",
					lnext, knext, nl, nforward);
				showline(logline, 0);
			}
			if (longtable == 0) {
				if (verboseflag) 
					showline("Switching to indirect table format\n", 0);
				longtable = 1;		/* can't fit in old style table */
			}
/*			nl = nl + fontchrs; */		/* add space for indirect addresses */
		}
	}

/*	if (nforward > 0) {
		if (verboseflag) sprintf(logline, "Forwarding table size %d\n", nforward);
	} */

/*	if (longtable == 0) nforward = 0;		
	else nforward = howmany(); */

/*	if (longtable != 0) nl = nl + fontchrs;	*/	/* 92/April/4 */
	if (longtable) {
		if (traceflag) {
			sprintf(logline, "nl %d = nl %d + nforward %d\n",
				   nl+nforward, nl, nforward); /* ??? */
			showline(logline, 0);
		}
		nl = nl + nforward;		/* 92/April/4 */
	}

/*	unlikely to overflow - since 2^15 words is 2^17 byte (131072) */
	if (lf >= 32768) {
		sprintf(logline, "ERROR: %s too large %d words\n", "TFM file", lf);
		showline(logline, 1);
	}
	if (nl >= 32768) {
		sprintf(logline, "ERROR: %s too large %d words\n", "lig/kern programs", nl);
		showline(logline, 1);
	}
	if (nk >= 32768) {
		sprintf(logline, "ERROR: %s too large %d words\n", "kern table", nk);
		showline(logline, 1);
	}

	lf = 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np;
	writeint(lf);	/* length of file in words */ /* later */
	writeint(lh);	/* length of header data in words */
	writeint(bc);	/* smallest character code */
	writeint(ec);	/* largest character code */
	writeint(nw);	/* number of words in the width table */
	writeint(nh);	/* number of words in the height table */
	writeint(nd);	/* number of words in the depth table */
	writeint(ni);	/* number of words in the italic correction table */
	writeint(nl);	/* number of words in the lig/kern table */ /* later */
	writeint(nk);	/* number of words in the kern table */ /* later */
	writeint(ne);	/* number of words in the extensible char table */ 
	writeint(np);	/* number of font parameter words */
	if (abortflag) return -1;
	if (traceflag) { 
/*	if (verboseflag) { */
		sprintf(logline, "lf %d lh %d bc %d ec %d\n", lf, lh, bc, ec);
		showline(logline, 0);
		sprintf(logline, "nw %d nh %d nd %d ni %d nl %d nk %d ne %d np %d\n",
			nw, nh, nd, ni, nl, nk, ne, np);
		showline(logline, 0);
	}
	return 0;
}

double pi = 3.141592653;

int writemapped(double x) {
	return writelong(mapdouble(x));
}

/* void writeparams(int n) */		/* write the fontdimen parameters */
int writeparams() {	/* write the fontdimen parameters */
	double theta;
	int ret;
	
	theta = - (italicangle * pi) / 180.0;			/* 93/June/22 */
/*	if (verboseflag)  */
	if (verboseflag && italicangle != 0.0) {
		sprintf(logline, "theta = %lg, tan(theta) = %lg\n", theta, tan(theta));
		showline(logline, 0);
	}
	ret = writemapped(1000.0 * tan (theta));	/* slant - param 1 */
 	if (ret < 0) return ret;
	if (verboseflag) {					/* 95/Oct/28 */
		if (! isfixedpitch) {
			sprintf(logline, "space %lg stretch %lg shrink %lg extra %lg\n",
				   spacewidth, spacestretch, spaceshrink, extraspace);
			showline(logline, 0);
		}
		else {
			sprintf(logline, "space %lg stretch %lg shrink %lg extra %lg\n",
				   spacewidth, 0.0, 0.0, spacewidth);
			showline(logline, 0);
		}
	}
	ret = writemapped(spacewidth);			/* space - param 2 */
	if (ret < 0) return ret;
	if (! isfixedpitch) ret = writemapped(spacestretch);	/* param 3 */
	else ret = writemapped(0.0);
	if (ret < 0) return ret;
	if (! isfixedpitch) ret = writemapped(spaceshrink);	/* param 4 */
	else ret = writemapped(0.0);
	if (ret < 0) return ret;
	ret = writemapped(xheight);				/* xheight - param 5 */
	if (ret < 0) return ret;
	if (! isfixedpitch) ret = writemapped(em_quad); /* em quad size - param 6 */
	else ret = writemapped(spacewidth * 2);
	if (ret < 0) return ret;
	if (mathitflag) return 0;		/* that is it for math italic */

	if (! isfixedpitch) ret = writemapped(extraspace);	/* param 7 */
	else ret = writemapped(spacewidth);		/* typewrite extraspace = space */
	if (ret < 0) return ret;

	if (traceflag) {
		sprintf (logline, "ec %d mathex %d mathsy %d mathit %d\n",
				ecflag, mathexflag, mathsyflag, mathitflag);
		showline(logline, 0);
	}
	if (ecflag) {
		if (traceflag) showline("Writing extra `EC' parameters\n", 1);
		writemapped(cap_height);				/* param 8 */
		writemapped(asc_height);				/* param 9 */
		writemapped(acc_cap_height);			/* param 10 */
		writemapped(desc_depth);				/* param 11 */
		writemapped(max_height);				/* param 12 */
		writemapped(max_depth);					/* param 13 */
		writemapped(digit_width);				/* param 14 */
		writemapped(cap_stem);					/* param 15 */
		writemapped(baseline_skip);				/* param 16 */
		if (abortflag) return -1;
	}
	else if (mathexflag) {				/* extra stuff for math exten */
		writemapped(defaultrule);
		writemapped(bigopspacing1);
		writemapped(bigopspacing2);
		writemapped(bigopspacing3);
		writemapped(bigopspacing4);
		writemapped(bigopspacing5);
		if (abortflag) return -1;
	}
	else if (mathsyflag) {			/* extra stuff for math symbol */
		writemapped(num1);
		writemapped(num2);
		writemapped(num3);
		writemapped(denom1);
		writemapped(denom2);
		writemapped(sup1);
		writemapped(sup2);
		writemapped(sup3);
		writemapped(sub1);
		writemapped(sub2);
		writemapped(supdrop);
		writemapped(subdrop);
		writemapped(delim1);
		writemapped(delim2);
		writemapped(axisheight);
		if (abortflag) return -1;
	}
	return 0;
}

int writeligkern(void);
int compresswidthtable(void);
int compressheighttable(void);
int compressdepthtable(void);
int compressitalictable(void);

int offbyone (long xur, long width) { /* if (xur[k] == width[k] + 1.0) */
	double eps;
	eps = unmap(width) + 1.0 - unmap(xur);
	if (eps > -0.5 && eps < 0.5) return 1;
	else return 0;
}

/*  c:\c\afmtotfm.c(3228) : fatal error C1001: internal compiler error */
/*		(compiler file 'msc2.cpp', line 1011) */

/* assuming height, depth, and italic all non-negative */
/* allowing for negative depths though ... */

void buildtables(void) { /* collect enrties into dimensional tables */
	int k;
	double val; 
/*	double itacor; */ 						/* 92/May/15 */
/*	double dep, hei; */
/*	double minheight=10000.0, maxheight= -10000.0; */
/*	double mindepth =10000.0, maxdepth = -10000.0; */
/* 	double minitalic=10000.0, maxitalic= -10000.0; */
	long itacor;	 						/* 92/May/15 */
	long dep, hei;
	long minheight=10000000, maxheight= -10000000;
	long mindepth =10000000, maxdepth = -10000000;
	long minitalic=10000000, maxitalic= -10000000; 

/*	zeroth entry in each case should be zero */
/*	widthtable[0] = 0.0; heighttable[0] = 0.0;  */
/*	depthtable[0] = 0.0; italictable[0] = 0.0; */
	widthtable[0] = 0; heighttable[0] = 0; 
	depthtable[0] = 0; italictable[0] = 0; 
	nw = 1; nh = 1; nd = 1; ni = 1;
	assert (bc >= 0 && ec < fontchrs);
	for (k = bc; k <= ec; k++) {
/*		if (strcmp(encoding[k], "") != 0) { */
		if (width[k] != NOWIDTH) { /* ignore unused positions */
/* WAS: ignore zero width (already in table) */ 
/* BUT: zero width index => not a valid character - SO KEEP IT */
/*			if (width[k] != 0.0)  */
				widthtable[nw++] = width[k];
#ifdef DEBUGGING
/*			if (width[k] == 0.0) { */
			if (width[k] == 0) {
				if (verboseflag) {
					sprintf(logline, "Char %d width %lg index %d\n",
						k, unmap(width[k]), (nw-1));
					showline(logline, 0);
				}
			}
#endif
			hei = yur[k];
/*			if (limitatzero != 0 && hei < 0.0) hei = 0.0; */
			if (limitatzero) {
				if(hei < 0) hei = 0;
			}
/*			if (hei != 0.0) */ /* ignore zero height (already in table) */
			if (hei != 0) /* ignore zero height (already in table) */
				heighttable[nh++] = hei;
			if (hei > maxheight) maxheight = hei;
			if (hei < minheight) minheight = hei;
			height[k] = hei;
			
			dep = - yll[k];
/*			if (limitatzero != 0 && dep < 0.0) dep = 0.0; */
			if (limitatzero) {
				if (dep < 0) dep = 0;
			}
/*			if (dep != 0.0) */ /* ignore zero depth (already in table) */
			if (dep) /* ignore zero depth (already in table) */
				depthtable[nd++] = dep;
			if (dep > maxdepth) maxdepth = dep;
			if (dep < mindepth) mindepth = dep;
			depth[k] = dep;

/*			italic correction is (xur - width) */ /* NOT ANYMORE */
/*			italic correction is (xur - width + italicfuzz) */ /* 92/Mar/16 */
/*			if (xur[k] == 0.0)	itacor = 0.0; */	/* BBox 0 0 0 0 case */
			if (xur[k] == 0) itacor = 0;			/* BBox 0 0 0 0 case */
/*			else if (italicangle == 0.0) itacorr = 0.0; *//* non-italic */
/*			else if (xur[k] >= width[k] - WIDTHFUZZ &&
				     xur[k] <= width[k] + WIDTHFUZZ)	*/
/*			else if (xur[k] == (float) width[k]) */	/* can this test fail ? */
			else if (xur[k] == width[k])	/* can this test fail ? */
				itacor = 0;					/* special case hack */
/*			else itacor = (xur[k] - width[k]);  */
/*			else itacor = (xur[k] - width[k] + italicfuzz);	*/	/* 92/Mar/16 */
			else itacor = (xur[k] - width[k] + mapdouble(italicfuzz));
/*			if (xur[k] == width[k] + 1.0) */
			if (offbyone(xur[k], width[k]))
/*				itacor = italicfuzz;	*/					/* 93/May/9 */
				itacor = mapdouble(italicfuzz);
/*			take care of special case where italic correction == italicfuzz */
			if (isfixedpitch && ! italicflag) 
				itacor = 0;	/* 92/Jan/5 */
/*			if (itacor < 0.0) itacor = 0.0; */ /* shouldn't be negative */
			if (itacor < 0) itacor = 0; /* shouldn't be negative */
/*			if (itacor != 0.0) *//* ignore zero italic (already in table) */
			if (itacor != 0)	/* ignore zero italic (already in table) */
				italictable[ni++] = itacor; 
			if (itacor > maxitalic) maxitalic = itacor;
			if (itacor < minitalic) minitalic = itacor;		
			italic[k] = itacor;
			if (showital && itacor != 0) {	/* DEBUGGING ONLY */
				sprintf(logline, "%d XUR %lg WX %lg I %lg\n", 
					k, unmap(xur[k]), unmap(width[k]), unmap(italic[k]));
				showline(logline, 0);
			}
		}
	}
/* 	assert(minitalic >= 0.0); */
/*	assert(minheight >= 0.0); */
/*	assert(mindepth >= 0.0); */
	assert(minitalic >= 0);
	if (limitatzero) assert(minheight >= 0);
	if (limitatzero) assert(mindepth >= 0);				/* ??? */
#ifdef DEBUGGING
	if (traceflag) {
		sprintf(logline, "minheight %lg maxheight %lg mindepth %lg maxdepth %lg\n",
				unmap(minheight), unmap(maxheight), unmap(mindepth), unmap(maxdepth));
		showline(logline, 0);
		sprintf(logline, "minitalic %lg maxitalic %lg\n",
			unmap(minitalic), unmap(maxitalic));
		showline(logline, 0);
	} 
#endif
/*	if (limitatzero) {
		if (minheight < 0.0) minheight = 0.0;
		if (maxdepth > 0.0) maxdepth = 0.0;		
		if (minitalic < 0.0) minitalic = 0.0;	
	} */
	compresswidthtable();					/* this MUST always work */
/*	assert(widthtable[0] == 0.0); */
	assert(widthtable[0] == 0);
	if (compressheighttable() < 0) {
		nh = 16; 
		for (k = 1; k < nh; k++) {
/*			val = (minheight + (maxheight - minheight) * 
				(double) k / ((double) nh - 1.0)); */
			val = ((double) minheight + (maxheight - minheight) * 
				(double) k / ((double) nh - 1.0));
/*			heighttable[k] = ((double) ((long) (val + 0.5))); */
			heighttable[k] = (long) (val + 0.5); 
/*			heighttable[k] = (float) ((long) (val + 0.5)); */
		}
	}
/*	assert(heighttable[0] == 0.0); */
	assert(heighttable[0] == 0);
	if (compressdepthtable() < 0) {
		nd = 16;
		for (k = 1; k < nd; k++) {
/*			val = (maxdepth + (mindepth - maxdepth) * 
				(double) k / ((double) nd - 1.0)); */
			val = ((double) maxdepth + (mindepth - maxdepth) * 
				(double) k / ((double) nd - 1.0));
/*			depthtable[k] = ((double) ((long) (val + 0.5))); */
			depthtable[k] = (long) (val + 0.5); 
/*			depthtable[k] = (float) ((long) (val + 0.5)); */
		}
	}
/*	assert(depthtable[0] == 0.0); */
	assert(depthtable[0] == 0);

/*	if (minitalic == 0.0 && maxitalic == 0.0) { */
	if (minitalic == 0 && maxitalic == 0) {
		if (verboseflag) {
			sprintf(logline, "Don't need italics (min: %lg max: %lg)\n", 
				unmap(minitalic), unmap(maxitalic));
			showline(logline, 0);
		}
		italicneeded = 0; ni = 1; /* NOT 0 ! */
	} 
	else {
		italicneeded = 1; 
/*		if (verboseflag) 
			sprintf(logline, "\nNeed italics (min: %lg max: %lg) ", 
				minitalic, maxitalic); */
		if (compressitalictable() < 0) {	/* unlikely we get here */
			ni = 64;
			for (k = 1; k < ni; k++) {
/*				val = (minitalic + (maxitalic - minitalic) * 
					(double) k / ((double) ni - 1.0)); */
				val = ((double) minitalic + (maxitalic - minitalic) * 
					(double) k / ((double) ni - 1.0));
/*				italictable[k] = ((double) ((long) (val + 0.5))); */
				italictable[k] = (long) (val + 0.5);
/*				italictable[k] = (float) ((long) (val + 0.5)); */
			}	
		}
	}
/*	assert(italictable[0] == 0.0); */
	assert(italictable[0] == 0);
}

/*	copy distinct kern pair sizes down into kerntable */
/*  somewhat of a kludge to allow keeping kerntable smaller */

int copykerntable(int nker) { /* copy and compress kern table */
	int i, n=0, k, flag;
/*	double kerndist;  */
	long kerndist; 
/*	float kerndist; */
	
/*	for (k=0; k < nker; k++) kerntable[k] = (double) kern[k]; */ /* old */
	n = 0;
	for (k=0; k < nker; k++) {
		flag = 0; 
/*		kerndist = (double) kern[k]; */
		kerndist = kern[k];
		for (i = 0; i < n; i++) {
			if (kerntable[i] == kerndist) {
				flag = 1;
				break;			/* already in table */
			}
		}
		if (! flag) {
			if (n < MAXDISTINCT) kerntable[n] = kerndist; /* add to table */
			n++;
		}
	}
	if (n >= MAXDISTINCT) {
		sprintf(logline,
			"\nERROR: Too many (%d > %d) distinct kern sizes -",
				n, MAXDISTINCT);
		showline(logline, 1);
		return MAXDISTINCT;
	}
	else if (n > 255) {
		if (traceflag) {
			sprintf(logline,
	"WARNING: More than 255 distinct kern distances (%d) (requires TeX 3.0)",
				n);
			showline(logline, 0);
		}
	}
	return n;	/* return number of distinct entries */
}

void buildkerntable(void) {
/*	int i, k; */

	if (nkern == 0) {
		nk = 0; return;
	}
/*	nk = nkern; */
	nk = copykerntable(nkern);
	if (traceflag) {
		showline("\n", 0);
		sprintf(logline, "%s %d %s %s ", "nk", nk, "sorting", "kern sizes");
		showline(logline, 0);
	}

	qsort(kerntable, (unsigned int) nk, 
/*			sizeof(double), comparedouble); */
			sizeof(long), comparelong); 

#ifdef DEBUGGING
	if (traceflag) showkerns();
#endif

/*	following not useful, since we already removed redundancies earlier */
/*	k = 0;
	for (i = 1; i < nkern; i++) {
		if (kerntable[i] != kerntable[k]) {
			k++; kerntable[k] = kerntable[i];
		}
	}
	nk = k + 1; */

/*	if (nk > MAXDISTINCT)
		sprintf(logline, "Too many distinct kerns (%d > %d)\n",
			nk, MAXDISTINCT); */		/* redundant */
#ifdef DEBUGGING
	if (traceflag) showkerns();
#endif
	if (traceflag) {
		sprintf(logline, "nk %d ", nk);
		showline(logline, 0);
	}
	if (traceflag) showline("\n", 0);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Break up name at hyphen `-'  -and-  where upper case follows lower case */
/* Treat digits as uppercase ? 98/Jun/19 NO */ /* do not modifiy arg */

int splitname(char *name) {
	char *s=name, *t=name;
	unsigned int k;
	int c, n, lowerflag;

	if (*name == '\0') 	showline("FontName NULL\n", 1);

	if (strncmp(name, "ITC-", 4) == 0)
		name = name + 4;		/* ignore ITC- ??? */

	for (k = 0; k < MAXSUFFIX; k++) strcpy(SuffixString[k], "");
	nextsuffix = 0;

	lowerflag = 0;
	while ((c = *t) != '\0') {
/*		if (c == '-') break; */			/* 98/Aug/17 */
		if (c == '-' || c == ' ' || c == '_') break;
		if (lowerdef) {	/*	following changed 93/March/15 */
			if (lowerflag && c >= 'A' && c <= 'Z') break;  
/*			if (lowerflag && c >= '0' && c <= '9') break; */ /* 98/June/19 */
/*			if (c < 'A' || c > 'Z') lowerflag = 1; */
		}
		else {
			if (lowerflag && (c < 'a' || c > 'z')) break; 
/*			if (c >= 'a' && c <= 'z') lowerflag = 1; */
		}
		if (c >= 'a' && c <= 'z') lowerflag = 1;
		t++;
	}
	n = t - s;
	if (n == 0) showline("Empty Base Name\n", 1);
	strncpy(BaseName, name, n);
	*(BaseName + n) = '\0';
/*	if (c == '-') { */
	if (c == '-' || c == ' ' || c == '_') {
		strcpy(SuffixString[nextsuffix++], "-");
		t++;
	}
	for(;;) {
		s = t;
/*		if (strcmp(s, "") == 0) break; */	/* nothing left */
		if (*s == '\0') break;	/* nothing left */
		lowerflag = 0;
		while ((c = *t) != '\0') {
/*			if (c == '-') break; */			/* 98/Aug/17 */
			if (c == '-' || c == ' ' || c == '_') break;
			if (lowerdef) {
				if (lowerflag && c >= 'A' && c <= 'Z') break; 
/*				if (lowerflag && c >= '0' && c <= '9') break; */ /* 98/Jun/19 */
/*				if (c < 'A' || c > 'Z') lowerflag = 1; */
			}
			else {
				if (lowerflag && (c < 'a' || c > 'z')) break;
/*				if (c >= 'a' && c <= 'z') lowerflag = 1; */
			}
			if (c >= 'a' && c <= 'z') lowerflag = 1;
			t++;
		}
		n = t - s;
		if (n == 0) {
			showline("Empty Suffix\n", 1);
			break;
		}
		if (nextsuffix >= MAXSUFFIX) {
			sprintf(logline, "ERROR: Too many suffixes (> %d)\n", MAXSUFFIX);
			showline(logline, 1);
			break;
		}
		strncpy(SuffixString[nextsuffix], s, n);
		SuffixString[nextsuffix][n] = '\0';
		nextsuffix++;
/*		if (c == '-')  { */
		if (c == '-' || c == ' ' || c == '_') {
			strcpy(SuffixString[nextsuffix++], "-");
			t++;
			continue;
		}
		if (c == '\0') break;
		if (abortflag > 0) return -1;
	}
	if (traceflag) {
		showline("\n", 0);
/* 		sprintf(logline, "FontName is: %s\n", FontName); */
		sprintf(logline, "BaseName: `%s' ", BaseName);
		showline(logline, 0);		
/*		if (nextsuffix > 0) putc('\n', stdout); */
		for (k = 0; k < nextsuffix; k++) {
			sprintf(logline, "Suffix %d: `%s' ", k, SuffixString[k]);
			showline(logline, 0);		
		}
		showline("\n", 0);
	}
	return 0;
} 

char *modifiers[]={
"Demi", "Demibold", "Semibold", "Bold", "Heavy", "Black",
"Light", "Roman", "Regular", "Normal", "Medium",
"Italic", "Oblique", "Slanted", "Kursiv", "Upright", ""
};

int stylemodifier (char *suffix) {
	int k;
	for (k = 0; k < 16; k++) {
		if (strcmp(suffix, modifiers[k]) == 0) return 1;
	}
	return 0;
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
	while ((c = MacName[k]) > ' ') {	/* 5 letters in first component */
/*		if (c < 'A' || c > 'Z') lowflag = 1; */
		if (c >= 'a' && c <= 'z') lowflag = 1;
		if (lowflag && count > 4) break;
		count++;
		k++;
	}
/*	if (verboseflag)  
		sprintf(logline, "MacName %s k %d low %d\n", MacName, k, lowflag); */
	MacName[k] = '\0'; 
/*	lowcount = 0; */

	for (m = 0; m < nextsuffix; m++) {
		if (strcmp(SuffixString[m], "-") == 0) continue;
		if (strcmp(SuffixString[m], "MM") == 0) continue; /* 98/Aug/17 */
/*		if (stylemodifier(SuffixString[m])) {
			if (stripbolditalic) continue;
			else SuffixString[m][1] = '\0';
		} */ /* ??? */
		if (*SuffixString[m] == '-') strcat(MacName, SuffixString[m]+1);
		else strcat(MacName, SuffixString[m]);
/*		lowcount = 0; */
		count = 0; 
		lowflag = 0;
		while ((c = MacName[k]) > ' ') {	/* 3 letters in component */
/*			if (c < 'A' || c > 'Z') lowflag = 1; */
			if (c >= 'a' && c <= 'z') lowflag = 1;
			if (lowflag && count > 2) break;
			count++;
			k++;
		}		
		MacName[k] = '\0';
	}
/*	if (verboseflag) 
		sprintf(logline, "Outline font file name should be: %s\n", MacName); */
	if (k > 31) {				/* on the Mac font name limit ? */
		sprintf(logline, "Outline font name %s is too long (> 31)\n", MacName);
		showline(logline, 0);
	}
	if (traceflag) {
		sprintf(logline, "MacName:  `%s' (%d) ", MacName, strlen(MacName));
		showline(logline, 0);
	}
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  */

int nfontid;	/* words of fontid information - default XEROXFONTN */

/* Standard Xerox header has 16 words - after checksum and design size. */
/* 10 of these are CharCoding, 5 are FontID and 1 word is the `Face Byte' */
/* A TFM file word is 4 bytes */ /* Textures uses FaceByte for style info */

int countxerox(void) {	/* figure out how many words needed in header */
/*	int nfontid; */

	if (strcmp(charcodingscheme, "") == 0 && strcmp(fontid, "") == 0)
		return 2;							/* no info to hide */
/*	header length must be multiple of four bytes */
/*	ceiling ((strlen(fontid) + 1) / 4) */
	nfontid = (((int) strlen(fontid) + 1) + 3) / 4;	
	if (nfontid > XEROXFONTN) {
		if (forcexerox || texturesflag) nfontid = XEROXFONTN; /* 92/April/4 */
		else if (verboseflag) {
			sprintf(logline, "Extending FontID field to %d (from %d)\n",
				   nfontid * 4 - 1, XEROXFONTN * 4 - 1);
			showline(logline, 0);
		}
	}
	else if (nfontid < XEROXFONTN) nfontid = XEROXFONTN; /* pad xerox PARC header */
	return (2 + XEROXCODEN + nfontid + 1);
}

/* modified to use FontName if FontID is not present */
/* modified to use coding vector name if codingscheme not present */

int writexerox(void) {
	int n, m, ret;
	int style;
	long facelong=0;
	char *s;
	int QDEncode=0;			/* screen encoding for TeXtures metric file */
	int PSEncode=0;			/* printer encoding for TeXtures metric file */

	if (strcmp(charcodingscheme, "") == 0 && strcmp(fontid, "") == 0)
		return 0;				/* no info to hide */
	s = charcodingscheme;

	n = (int) strlen(s);
	m = 4*XEROXCODEN-1;
	if (n > 4*XEROXCODEN-1) {
		if (verboseflag)
		sprintf(logline, "NOTE: Encoding scheme (%s) too long (%d > %d)\n", 
			s, n, 4*XEROXCODEN-1);
		showline(logline, 0);
		n = m = 4*XEROXCODEN-1;
	}

	ret = writepascal(s, n, m);
	if (ret < 0) return ret;
	if (hideyandy) {
/*		char *s = "Y&Y Inc"; */
		char *s = "Y&Y ";						/* 98/Aug/25 */
		int ny = strlen(s);
		int k;
/*		if (n+ny < 4*XEROXCODEN-1) { */
		if (n+ny <= m) {			/* 98/Aug/25 */
			k = bufferinx - ny;
			while (*s != '\0')	buffer[k++] = *s++;
		}
	}

	s = fontid;
	n = (int) strlen(s);
/*	m = 4*XEROXFONTN-1; */
	m = nfontid * 4 -1;
	if (n > 4*XEROXFONTN-1) {
		if (forcexerox || texturesflag) {
			if (verboseflag)
			sprintf(logline, "NOTE: FontID (%s) too long (%d > %d)\n", 
				s, n, 4*XEROXFONTN-1);
			showline(logline, 0);
			n = m = 4*XEROXFONTN-1; 
		} 
	}

	ret = writepascal(s, n, m);
	if (ret < 0) return ret;

	ret = longalign();	/*	Header length must be multiple of four ! */
	if (ret < 0) return ret;

	if (texturesflag) {
		style = 0;
		if (boldflag && ! suppressbold) style = style | 1;
		if (italicflag && ! suppressitalic) style = style | 2; 

		QDEncode = 0; PSEncode = 0;			/* default - non-text fonts */
/*		Use -T on command line for math fonts in Textures ? redundant ? */
		if (
/*			a total hack, just to get going on LucidaNewMath fonts */
			strstr(fontname, "Math") != NULL ||
			strstr(fontname, "math") != NULL) {
/*			style = style & ~2; */		/* ignore italic flag (lbmi, lbms) */
										/* but not now with lbmr ? */
										/* modified 95/Mar/17 */
/*			QDEncode = 128;  */	/* ??? */
		}
		else {
/*			For Textures so-called `TeX text' encoding */
			if (strcmp(codingvector, "textures") == 0 ||
				strcmp(codingvector, "texital") == 0 ||		/* 92/June/12 */
				strcmp(codingvector, "textext") == 0) {
				QDEncode = 1; PSEncode = 1;
			}
/*			For fixed-width/typewriter */
			else if (strcmp(codingvector, "textutyp") == 0 ||
				strcmp(codingvector, "typewrit") == 0 ||	/* 92/June/12 */
				strcmp(codingvector, "typeital") == 0 ||
				strcmp(codingvector, "textype") == 0) {
				QDEncode = 2; PSEncode = 2;
			}
/*			New Textures 1.6 MacintoshStandard */
			else if (strcmp(codingvector, "texmac") == 0) { /* 93/July/19 */
				QDEncode = 3; PSEncode = 3;
			}
/*			For remapping 0 - 31 => 161 - 195 */
			else if (strcmp(codingvector, "lucida") == 0) { /* 93/July/19 */
				QDEncode = 4; PSEncode = 4;
			}
/*			For `standard encoding' or `none' */
			else if (strcmp(codingvector, "standard") == 0)	{
				QDEncode = 0; PSEncode = 0;
			}
		}
/*		for math fonts we stay with QDEncode = 0 and PSEncode = 0 */

/*		EdMetrics/Textures hides style info in `face byte' */
/*		facebyte = style;
		facebyte = facebyte | (QDEncode << 24);
		facebyte = facebyte | (PSEncode << 16); */
		facelong = style;
		facelong = facelong | ((long) QDEncode << 24);
		facelong = facelong | ((long) PSEncode << 16);
		if (verboseflag) {
			sprintf(logline,
 "QDEncode %d PSEncode %d Style %d (BoldFlag %d ItalicFlag %d) Face %0lX\n", 
				QDEncode, PSEncode, style, boldflag, italicflag, facelong);
			showline(logline, 0);
		}
	}			/* end of if (texturesflag) */
	else {				/* compute face byte some other way */
/*		facelong = 128 << 24; */
	}
/*	writelong((long) facebyte);	*/	/* ??? */
	ret = writelong (facelong);		/* ??? */
	if (ret < 0) return ret;
	return 0;
}

/* Try and create hash code based on font file name */
/* stop before .afm extension, and convert upper to lower case */

unsigned long hashstring(char *s) {			/* 93/May/14 */
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		if (c == '.') break;
		if (c >= 'A' && c <= 'Z') c = c + ('a' - 'A');
		hash = (hash * 53 + c) & 16777215;
	}
	return hash;
}

/* Takes first six letters of encoding vector name and compresses */
/* into four bytes using base 40 coding */ /* 40^6 < s^32 */
/* Treats lower case and upper case the same, ignores non alphanumerics */
/* Except handles -, &, and _ */

unsigned long codefourty(char *codingvector) {
	unsigned long result=0;
	int c, k;
	char *s=codingvector;

	if (traceflag) {
		sprintf(logline, "Given coding vector %s\n", codingvector);
		showline(logline, 0);
	}
	if (strcmp(codingvector, "") == 0) {
		codingvector = "native";		/* if not specified ... */
		return checkdefault;			/* use default signature */
	}
	for (k = 0; k < 6; k++) {
		for (;;) {
			c = *s++;
			if (c >= 'A' && c <= 'Z') c = c - 'A';
			else if (c >= 'a' && c <= 'z') c = c - 'a';
			else if (c >= '0' && c <= '9') c = (c - '0') + ('Z' - 'A') + 1;
			else if (c == '-') c = 36;
			else if (c == '&') c = 37;
			else if (c == '_') c = 38;
			else c = 39;					/* none of the above */
/*			else continue; */				/* not alphanumeric character */
/*			result = result * 36 + c; */
			result = result * 40 + c;
			break;
		}
	}
	if (traceflag) {
		sprintf(logline, "Computed CheckSum %08lX\n", result);
		showline(logline, 0);
	}
	return result;
}

/* Fill in missing parameters */

void setupparams (void) {
	int nchar;
/*	if designsize not given ? */	/* defaults to 10.0 */
/*	if italicangle not given ? */	/* defaults to 0.0 */
	if (capheight == 0.0) {		/*	if capheight not given ? */
		capheight = unmap(yur['H']);	/*		capheight = 750.0; */
	}
	if (xheight == 0.0) {		/*	if xheight not given ? */
		xheight = unmap(yur['x']);		/*		xheight = 500.0; */
	}
	if (ascender == 0.0) {		/*	if ascender not given ? */
		ascender = unmap(yur['f']);
	}
	if (descender == 0.0) {		/*	if descender not given ? */
		descender = unmap(yll['g']);
	}
	if (em_quad == 0.0) {		/*	if em_quad not given ? */
/*		em_quad = unmap(width['m']) * xscale; */	/* BUG! lowercase */
		nchar = clookup("emdash", -1);	/* use width of emdash instead? */
//		printf("nchar %d emdash\n", nchar);
		if (nchar < 0 || width[nchar] == NOWIDTH) {
			nchar = clookup("M", -1);
			if (nchar < 0 || width[nchar] == NOWIDTH) {
				nchar = clookup("m", -1);
				if (nchar < 0 || width[nchar] == NOWIDTH) {
					nchar = clookup("H", -1);
				}
			}
		}
//		NOTE: Adobe uses as default with of 'H' ?
		em_quad = unmap(width[nchar]) * xscale;
//		NOTE: may prefer to make default quad be 1000 units ?
//		em_quad = 1000;

//		else em_quad = unmap(width['M']) * xscale;	/* 1999 Feb 19 */
		if (traceflag) {
			sprintf(logline, "em_quad %lg\n", em_quad);
			showline(logline, 0);
		}
	}
}

/* Fill in EC parameters - either -K on command line or AFM comments */

void setupec (void) {		/* 97/Feb/5 */
	int k;
	if (cap_height == 0.0) cap_height = capheight;
	if (asc_height == 0.0) asc_height = ascender;
	if (acc_cap_height == 0.0) {
		for (k = 0; k < MAXCHRS; k++) {
			if (strcmp(encoding[k], "") == 0) continue;
			if (strstr(encoding[k], "dieresis") != NULL ||
				strstr(encoding[k], "grave") != NULL ||
				strstr(encoding[k], "acute") != NULL ||
				strstr(encoding[k], "circumflex") != NULL ||
				strstr(encoding[k], "tilde") != NULL) {
				if (unmap(yur[k]) > acc_cap_height)
					acc_cap_height = unmap(yur[k]);
			}
		}
	}
	if (desc_depth == 0.0) desc_depth = -descender;
	if (max_height == 0.0) max_height = ascender;	/* or yur ? */
	if (max_depth == 0.0) max_depth = -descender;	/* or yll ? */
	if (digit_width == 0.0) {
		for (k = '0'; k <= '9'; k++) {
			if (strcmp(encoding[k], "") == 0) continue;
			if (unmap(width[k]) > digit_width)
				digit_width = unmap(width[k]);
		}
	}
	if (cap_stem == 0.0) cap_stem = 100;			/* fix ? */
	if (baseline_skip == 0) baseline_skip = 1200;
}

int needligkern(int k) {			/* does character need ligkern prog ? */
	if (ligend[k] != ligbegin[k]) return 1;
	if (kernend[k] != kernbegin[k]) return 1;
	return 0;
}

/* Mac style code: 1 - bold, 2 - italic, 4 - underline, 8 - outline */
/* 16 - shadow, 32 - condensed, 64 - extended, 128 - reserved */

/* Encoding: 0 - none, 1 TeX text, 2 TeX type, 4 Lucida Math */

/* for text fonts use QDEncode = 0 and PSEncode = 0 */
/* for math fonts use QDEncode = 128 and PSEncode = 0 */
/* possibly last byte of QuickDraw font menu name also matters */
/* ^] for text fonts and ^X for math fonts ? */

int constructtfm(char *name) {
	int k, n, ret;
	unsigned long hash;
	unsigned long charinfo;
	
	if (traceflag) showline("ConstructTFM:\n", 0);
/*  first try and make up sensible FONTID field */
/*  if fontid not given, try and fill in FontName at least */
/*	sprintf(logline, "FONTID: %s\n", fontid); */	/* debugging */
	if (strcmp(fontid, "") == 0) {
		if (usecontraction) {
			ret = splitname(fontname);
			if (ret < 0) return ret;
			constructname();
			strncpy(fontid, MacName, MAXFONTID);
			if (verboseflag) {
				sprintf(logline, "FontID: %s\n", fontid);
				showline(logline, 0);
			}
		}
		else {
			strncpy(fontid, fontname, MAXFONTID);
		}
	}
	if (strcmp(fontid, "") == 0) {			/* what, no FontName ? */
		strncpy(fontid, fullname, MAXFONTID);
	}
/*	sprintf(logline, "FONTID: %s\n", fontid); */	/* debugging */

/*  next try and make up sensible encoding scheme field */
	if (strcmp(charcodingscheme, "") == 0)  {
/*		for (k = 0; k < encodingcount; k++) { */
		for (k = 0; k < 64; k++) {
			if (strcmp(encodingnames[k][0], "") == 0) break;
			if (strcmp(encodingnames[k][1], "") == 0) break;
/*			if (strncmp(codingvector, encodingnames[k][1], 8) == 0) { */
			if (_strnicmp(codingvector, encodingnames[k][1], 8) == 0) {
				strcpy(charcodingscheme, encodingnames[k][0]); break;
			}
		}
	}
/*	if can't recognize encodingscheme, try and fill in encodingvector */
	if (strcmp(charcodingscheme, "") == 0) {
		strncpy(charcodingscheme, codingvector, MAXENCODING);
	}
/* if no encoding vector, use encoding scheme in  AFM file */
	if (strcmp(charcodingscheme, "") == 0) 
		strncpy(charcodingscheme, encodingscheme, MAXENCODING);
	if (strcmp(encodingvecname, "") != 0)	/* override everything if exist */
		strncpy(charcodingscheme, encodingvecname, MAXENCODING);
	
/*	sprintf(logline, "FONTID: %s\n", fontid); */	/* debugging */
	if (texturesflag) {	/* the way barry likes it */
/*		strcpy(charcodingscheme, "PostScript "); */
		strcpy(charcodingscheme, "PS ");
		strcat(charcodingscheme, fontname);
/*		if (strlen(charcodingscheme+3) > 28)  */
		if (strlen(charcodingscheme) >= XEROXCODEN*4) {		/* 40 */
			sprintf(logline,
				"WARNING: PS FontName `%s' too long for TeXtures\n",
					fontname);
			showline(logline, 0);
		}
/*		Also need QuickDraw name */ 
		if (strcmp(macintoshname, "") != 0)
			strcpy(fontid, macintoshname);
		else if (strncmp(familyname, fontname, strlen(familyname)) == 0 &&
			alluppercase(fontname) == 0) {
			strcpy(fontid, familyname);	
			showline("\n", 0);
			sprintf(logline,
				"WARNING: Using FamilyName `%s' as QuickDraw name\n",
				fontid);
			showline(logline, 0);
		}
		else {
			strcpy(fontid, fontname);	
			if (alluppercase(fontid) != 0) lowercase(fontid, fontid);
			sprintf(logline, "WARNING: Using FontName `%s' as QuickDraw name\n",
				fontid);
			showline(logline, 0);
		}
		if (strlen(fontid) >= XEROXFONTN*4)	{ /* 20 */
			sprintf(logline,
				"WARNING: MacintoshName `%s' too long for TeXtures\n",
					fontid);
			showline(logline, 0);
		}
	}	/* end of if (texturesflag) */
/*	sprintf(logline, "FONTID: %s\n", fontid); */	/* debugging */

/*	figure out lh np nl etc. for header */
/*	lh = 2;  */
	lh = countxerox();
/*	sprintf(logline, "LH %d\n", lh); */
/*	ne = 0;  */
	np = 7;
	if (mathitflag) np = 6;			/* 7 - 1 */
	else if (mathexflag) np = 13;		/* 7 + 6 */
	else if (ecflag) np = 16;			/* 7 + 9 new */
	else if (mathsyflag) np = 22;		/* 7 + 15 */
	if (traceflag) showline("\n", 0);
/*	if (verboseflag) sprintf(logline, "Writing preamble\n");  */

	nl = lnext + knext;			/* ligatures + kerns */
	if (traceflag) {
		sprintf(logline, "nl %d = lnext %d + knext %d\n", nl, lnext, knext); /* ??? */
		showline(logline, 0);
	}

	leftboundary = rightboundary = -1;	/* 97/Oct/28 ??? */
	if (leftboundarychar != NULL) {		/* 97/Oct/1 */
		leftboundary = clookup(leftboundarychar, -1);
/*		check also that width[leftboundary] != NOWIDTH ? */
/*		should not complain for symbol/math/pi font */
		if (leftboundary < 0) {
			if (textfontflag) {
				sprintf(logline, "WARNING: %s boundary char %s not in encoding\n",
					"left", leftboundarychar);
				showline(logline, 0);
				leftboundary = clookup("space", -1); /* try and use space */
				if (leftboundary < 0)	/* try and use visiblespace */
					leftboundary = clookup("visiblespace", -1);
/*				not helpful if kern table doesn't use this name ? */
			}
		}
		if (! needligkern(leftboundary)) {			/* 98/Sep/20 */
			if (traceflag) {
				sprintf(logline, "No ligkern program for %s (%d)\n",
								  leftboundarychar, leftboundary);
				showline(logline, 0);
			}
			leftboundary = -1;
		}
		if (traceflag) {
			sprintf(logline, "leftboundarychar  %s %d\n",
				   leftboundarychar, leftboundary);
			showline(logline, 0);
		}
	}
	if (rightboundarychar != NULL) {		/* 97/Oct/1 */
		rightboundary = clookup(rightboundarychar, -1);
/*		check also that width[leftboundary] != NOWIDTH ? */
		if (rightboundary < 0) {
			if (textfontflag) {
				sprintf(logline, "WARNING: %s boundary char %s not in encoding\n",
					"right", rightboundarychar);
				showline(logline, 0);
				rightboundary = clookup("space", -1); /* try and use space */
				if (rightboundary < 0)	/* try and use visiblespace */
					rightboundary = clookup("visiblespace", -1); 
/*				not helpful if kern table doesn't use this name ? */
			}
		}
		if (! needligkern(rightboundary)) {			/* 98/Sep/20 */
			if (traceflag) {
				sprintf(logline, "No ligkern program for %s (%d)\n",
					   rightboundarychar, rightboundary);
				showline(logline, 0);
			}
			rightboundary = -1;
		}
		if (traceflag) {
			sprintf(logline, "rightboundarychar %s %d\n",
				   rightboundarychar, rightboundary);
			showline(logline, 0);
		}
	}
/*	Here we should check whether this is needed - any boundary char kerns ? */
	if (leftboundary >= 0 && textfontflag) {
		if (traceflag) {
			sprintf(logline, "%s Boundary char %s (%d) nl++\n", "Left ", 
				   leftboundarychar, leftboundary);
			showline(logline, 0);
		}
		nl++;	/* for very last word, left boundary program */
	}
/*	Here we should check whether this is needed - any boundary char kerns ? */
	if (rightboundary >= 0 && textfontflag) {
		if (traceflag) {
			sprintf(logline, "%s Boundary char %s (%d) nl++\n", "Right",
				   rightboundarychar, rightboundary);
			showline(logline, 0);
		}
		nl++;		/* for very first word, right boundary char */
	}

	if (traceflag) {
		sprintf(logline, "np %d nl %d\n", np, nl);
		showline(logline, 0);
	}
	if (nl > 255 && ! backwardflag) {
		if (verboseflag) 
			showline("Switching to indirect table format\n", 0);	/* 98/Jun/30 */
		longtable = 1;
	}

	if (! longtable) nforward = 0;	/* 92/April/4 */
	else {
		nforward = howmany();	/* how many need forward pointers */
								/* i.e. how many have ligkern progs */
/*		if (verboseflag) sprintf(logline, "Forwarding table size %d\n", nforward); */
		if (traceflag) {
			sprintf(logline, "Forwarding table size %d\n", nforward);
			showline(logline, 0);
		}
	}
	
	ret = writepreamb();
	if (ret < 0) return ret;

/*	if (verboseflag) sprintf(logline, "Writing header\n");  */
	if (! checksumset && checksum == 0) {
		checksum = 0L;				/* header[0] = 32 bit check-sum */
		if (wantsignature < 0)	{
			checksum = codefourty(codingvector);
/*			(void) decodefourty(checksum); */	/* debugging */
		}
		else if (wantsignature > 0)	{
			hash = hashstring(name);		/* make last digit random */
			checksum = (checkdefault & ~255) | (hash & 255);
		}
	}
	if (traceflag) {
		sprintf(logline, "CheckSum  %08lX\n", checksum);
		showline(logline, 0);
	}
	ret = writelong(checksum);
	if (ret < 0) return -1;

/*	checksum = 0; checksumset = 0; */	/* reset for next AFM file */
	ret = writemapped(designsize * 1000.0);		/* header[1] = design size */
	if (ret < 0) return -1;

	ret = writexerox();				/* try and write xerox style header */
	if (ret < 0) return -1;
/*	if (verboseflag) sprintf(logline, "Writing Char Info\n");  */

	ligkerninx = 0;
	if (traceflag) showline("Resetting ligkerninx=0\n", 0);
/*	Here we should check whether this is needed - any boundary char kerns ? */
	if (rightboundary >= 0 && textfontflag) {		/* 98/Apr/5 */
		if (traceflag) showline("First word for rightboundary\n", 0);
		ligkerninx++;	/* very first word 97/Oct/1 */
	}
	
/*	if (longtable) ligkerninx = ligkerninx + fontchrs; */	/* ? */
	if (longtable) {
		if (traceflag) {
			sprintf(logline, "%d words for forwarding table\n", nforward);
			showline(logline, 0);
		}
		ligkerninx = ligkerninx + nforward; 
		nextslot = 0;
	}

/*	Now build charinfo table */

	if (traceflag) {
		sprintf(logline, "Building CharInfo for %d through %d\n", bc, ec);
		showline(logline, 0);
	}
	for (k = bc; k <= ec; k++) {
/*		if (strcmp(encoding[k], "") != 0) */
		chr = k;				/* make accessible to debugging output */
		if (width[k] != NOWIDTH) {		/* real character information ? */
			charinfo = constructchar(k); /* may increment nextslot */
			ret = writelong(charinfo);		/* ? */
			if (ret < 0) return -1;
		}
		else {				/* not a valid character - not in font ? */
			if ((n = updateinx(k)) > 0)	{ /* just in case any ligs or kerns ? */
				sprintf(logline, "ERROR: %d ligs or kerns for %s (%d) not in font\n",
					   n, encoding[k], k);
				showline(logline, 0);
			}
			ret = writelong(0L);	/* blank for missing character slot */
			if (ret < 0) return -1;
		}
	}
/*	Here we should check whether this is needed - any boundary char kerns ? */
	if (leftboundary >= 0 && textfontflag) {				/* 98/Apr/5 */
		if (traceflag) showline("Last word for leftboundary\n", 0);
		ligkerninx++;	/* very last word 97/Oct/1 */
	}

	if (longtable && nextslot != nforward) {
		sprintf(logline, "ERROR: Mismatch nextslot %d <> nforward %d\n", 
			nextslot, nforward);
		showline(logline, 1);
	}
/*	if ((! longtable && ligkerninx != nl) ||
		(longtable && (ligkerninx + fontchrs) != nl)) { */
	if (ligkerninx != nl) {
		sprintf(logline, 
			"FATAL ERROR: Inconsistent ligkerninx: (%d) versus nl (%d)\n", ligkerninx, nl);
		showline(logline, 1);
		sprintf(logline, "textfontflag %d fontchars %d\n",
				textfontflag, fontchrs);
		showline(logline, 1);
		sprintf(logline, "LNEXT: %d KNEXT: %d\n", lnext, knext);
		showline(logline, 1);
		complain();		
#ifdef DEBUGGING
		if (! traceflag) 
#endif
			checkexit(7);
		return -1;
	}

	if (bufferinx != 4 * (6 + lh + (ec - bc + 1))) {
		sprintf(logline, 
	"FATAL ERROR: Inconsistent bufferinx: (%d) versus 4 * lf (%d)\n", 
				bufferinx, 4 * (6 + lh + (ec - bc + 1)));
		showline(logline, 1);
		complain();
/*		checkexit(7); */
	}
	for(k = 0; k < nw; k++) writelong(widthtable[k]); 
	for(k = 0; k < nh; k++) writelong(heighttable[k]); 
	for(k = 0; k < nd; k++) writelong(depthtable[k]); 
	for(k = 0; k < ni; k++) writelong(italictable[k]); 
	if (abortflag) return -1;	

	if (bufferinx != 4 * (6 + lh + (ec - bc + 1) + nw + nh + nd + ni)) {
		sprintf(logline, 
				"FATAL ERROR: Inconsistent bufferinx: (%d) versus 4 * lf (%d)\n", 
				bufferinx, 4 * (6 + lh + (ec - bc + 1) + nw + nh + nd + ni));
		showline(logline, 1);
		complain();
/*		checkexit(7); */		
	}

	ret = writeligkern();			/* lig_kern commands */
	if (ret < 0) return -1;

	if (bufferinx != 4 * (6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl)) {
		sprintf(logline, 
				"FATAL ERROR: Inconsistent bufferinx: (%d) versus 4 * lf (%d)\n", 
				bufferinx, 4 * (6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl));
		showline(logline, 1);
		complain();
/*		checkexit(7); */
	}

/*	if (verboseflag) sprintf(logline, "Kern table at %d\n", bufferinx/4); */
	for(k = 0; k < nk; k++) {
/*		if (verboseflag) sprintf(logline, "k %d, kern %lg ", k, kerntable[k]); */
/*		ret = writemapped(kerntable[k]);  */
		ret = writelong(kerntable[k]); 
		if (ret < 0) return -1;
	}
#ifdef DEBUGGING
	if (traceflag) showline("Writing Extensible Table\n", 0);
#endif
	for(k = 0; k < ne; k++) writelong(extentable[k]);  /* ? */
	if (abortflag) return -1;
#ifdef DEBUGGING
	if (traceflag) showline("Writing Font Parameters\n", 0);
#endif
	setupparams();				/* 97/Feb/5 */
	if (ecflag) setupec();		/* 97/Feb/5 */
	ret = writeparams();		/* write the font parameters */
	if (ret < 0) return -1;
	if (bufferinx != 4 * lf) { 
		sprintf(logline, 
			"FATAL ERROR: Inconsistent bufferinx: (%d) versus 4 * lf (%d)\n", 
				bufferinx, 4 * lf);
		showline(logline, 1);
		complain();
#ifdef DEBUGGING
		if (! traceflag) 
#endif
			checkexit(7);
		return -1;
	}
	return 0;
}

/****************************************************************************/

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

/****************************************************************************/

void removeexten(char *fname) {  /* remove extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) != NULL) {
		if ((t = strrchr(fname, '\\')) == NULL || s > t) *s = '\0';
	}
} 

/****************************************************************************/

void showencoding (void) {				/* debugging output */
	int k;
	for (k = 0; k < 256; k++) {
		if (strcmp(encoding[k], "") == 0) continue;
		sprintf(logline, "%d\t%s\n", k, encoding[k]);
		showline(logline, 0);
	}
}

/* modified 92/Jan/5 to read first line of encoding vector for Encoding: */

int readencodingsub(char *name) {
	char fn_in[FILENAME_MAX];
/*	char charcode[MAXCHARNAME]; */
	char charcode[CHARNAME_MAX];		/* for safety sake */
	FILE *input;
	char *s, *t;
	int n, k;

/* First try current directory */
/* First try name as given possibly with path */
	strcpy(fn_in, name);
	extension(fn_in, "vec");
	if (traceflag) {
		sprintf(logline, "Trying: %s\n", fn_in);
		showline(logline, 0);
	}
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
				if (strcmp(fn_in, "")) {
					s = fn_in + strlen(fn_in) - 1;
					if (*s != '\\' && *s != '/') strcat(fn_in, "\\");
				}
				strcat(fn_in, name);
				extension(fn_in, "vec");
				if (traceflag) {
					sprintf(logline, "Trying: %s\n", fn_in);
					showline(logline, 0);
				}
				input = fopen(fn_in, "r");
				if (input != NULL) break;		/* success */
				if (t == NULL) break;			/* failed */
				else s = t+1;					/* continue after ; */
				if (abortflag > 0) return -1;
			}
		}
	}

/*	if (input == NULL) */	/*	then try in directory of procedure itself */
	if (input == NULL && *programpath !='\0') {	/*	then try in directory of procedure itself */
		strcpy(fn_in, programpath);
		strcat(fn_in, "\\");
		strcat(fn_in, name);
		extension(fn_in, "vec");
		if (traceflag) {
			sprintf(logline, "Trying: %s\n", fn_in);
			showline(logline, 0);
		}
		input = fopen(fn_in, "r");
	}
	if (input == NULL) {	/*	OK, time to punt ! */
		sprintf(logline, "ERROR: Can't open encoding vector: %s\n", name);
		showline(logline, 1);
		perrormod(name);
		return -1;
	}
	if (verboseflag) {
		sprintf(logline, "Loading encoding vector file: %s\n", fn_in);
		showline(logline, 0);
	}


	n=0;
	strcpy(encodingvecname, "");
	(void) getline(input, line, MAXLINE);			/* get first line */
	if ((s = strstr(line, "Encoding:")) != NULL) {
		s += strlen("Encoding:");
		while (*s++ <= ' ') ; s--;					/* step over white space */
		strncpy(encodingvecname, s, MAXENCODING);
		stripreturn(encodingvecname);
		if (verboseflag) {
			sprintf(logline, "EncodingVectorName: %s\n", encodingvecname);
			showline(logline, 0);
		}
/*		(void) getrealline(input, line, MAXLINE); */ /* 92/May/22 */
	}
/*	if (*line == '%' || *line == ';' || *line == '\n') 
		(void) getrealline(input, line, MAXLINE);	*/
	for(;;) {										/* fixed 93/Sep/16 */
		if (*line == '%' || *line == ';' || *line == '\n') {
			if (getrealline(input, line, MAXLINE) <= 0) {
				break;
			}
			continue;
		}
		if (sscanf(line, "%d %s", &k, charcode) < 2) {
			sprintf(logline, "Don't understand encoding line: %s", line);
			showline(logline, 1);
		} 
		else if (k >= 0 && k < fontchrs) {
			if (strcmp(encoding[k], "") != 0) {
				sprintf(logline,
					"WARNING: duplicate for %d: `%s' and `%s'\n",
						k, encoding[k], charcode);
				showline(logline, 0);
			}
			if (strcmp(encoding[k], "") != 0) {	/* 95/Feb/20 */
/*				if (debugflag) sprintf(logline, "free %d %s ", k, encoding[k]); */
				free(encoding[k]);
				encoding[k] = "";		/* ? */
			}
			encoding[k] = zstrdup(charcode);	/* 94/July/18 */
			if (encoding[k] == NULL) return -1;	/* OUT OF MEMORY */
		}
		else {
			sprintf(logline, "ERROR: Character code %d out of range\n", k);
			showline(logline, 1);
		}
		n++;
/*		sprintf(logline, "%d %s --- %d %s", k, charcode, n, line); */ /* debugging */
		if (getrealline(input, line, MAXLINE) <= 0) break; 
		if (abortflag > 0) return -1;
	}
	if (n != 128) {			/* not in future TeX 3.0 ? */
		sprintf(logline, "Encoding vector has %d elements\n", n);
		showline(logline, 0);
	}
	if (ferror(input)) {
		showline("Error in encoding vector\n", 1);
		perrormod(fn_in);
	}
	else fclose(input);
	if (debugflag)	showencoding();	/* debugging code added 93/Sep/16 */
	return 0;			/* normal exit */
}

int checkdupsub (char *, char *);

int checkdups (void);

int readencoding (char *vector) {
	int ret;

	if (encodingset) return 0; 
	if (_stricmp(vector, "mac") == 0 ||
		_stricmp(vector, "texmac") == 0) usemacencoding = 1; /* 93/Aug/3 */
	else usemacencoding = 0;
	if (readencodingsub(vector) != 0) {
		if (_stricmp(vector, "textext") != 0) {
			showline("Using TeX text (OT1) encoding instead\n", 1);
			if (readencodingsub("textext") != 0) {
/*				exit(2); */
				checkexit(2);
				return -1;
			}
		}
/*		else exit(9); */
		else {
			checkexit(9);
			return -1;
		}
	}
	if (avoiddups) {
		ret = checkdups();
		if (ret < 0) return ret;
	}
	encodingset = 1;			/* indicate we have the encoding loaded */
	return 0;
}

/****************************************************************************/

#ifdef DEBUGGING
int checkhowmany(void) { /* how many non-empty character names are there */
	int k, n=0;
	for (k=0; k < fontchrs; k++) {
/*		if (strcmp(encoding[k], "") != 0) n++; */
		if (width[k] != NOWIDTH) n++;
	}
	return n;
}
#endif

/* \tb: limit total kerning pairs and ligatures to 255 (for old TeX)\n\ */
/* \tb: backward compatiblity with TeX before 3.0 (limit to 256 kerns)\n\ */

void showusage (char *program) {
	char *s=logline;
	
	abortflag++;
	sprintf (s, "\
%s [-{v}{a}{d}{j}{n}{x}{F}{G}{t}{M}] [-c=<coding>]\n\
    [-s=<width>:<stretch>:<shrink>] [-e=<extra-width>] [-C=<checksum>]\n\
    [-g=<gap>] <afm-file-1> <afm-file-2>...\n", program);
	showline(s, 0);
/*	if (! detailflag) exit(1); */
	if (! detailflag) {
		checkexit(1);
		return;
	}
	sprintf (s, "\n\
    v:    verbose mode\n\
    a:    add f-ligatures + 8 pseudo ligatures (e.g. `---' => `emdash')\n\
    f:    (add only 8 standard TeX pseudo ligatures (i.e. no f-ligatures))\n\
    d:    add 3 extra pseudo ligatures (<<  >> and ,,)\n\
    j:    (add 58 accent ligatures (e.g. `dieresis A' => `Adieresis'))\n\
    n:    suppress ligatures in AFM file (use for fixed width fonts)\n\
    x:    do not add `x' to output file name when font is reencoded\n\
    F:    add kerns for accented chars based on left char in kern pair\n\
    G:    add kerns for accented chars based on right char in kern pair\n\
    t:    TeXtures mode: FontName for CodingScheme, MacIntoshName for FontID\n\
    T:    turn off veto on reencoding of what appear to be non-text fonts\n\
    s:    word space width (override width of `space' character in AFM)\n\
          (optionally specify stretch and shrink -s=width:stretch:shrink)\n\
    e:    extra space width after punctuation (override default)\n\
    C:    insert specified checksum in TFM file\n\
    g:    kern amount of boundary char kern pairs added for quotes\n\
    c:    use specified encoding vector (default `%s')\n\
          Note: if `none' specified, use encoding in AFM file\n\
", defaultencoding);
//	showline(logline, 0);
	showline(s, 1);
#ifndef _WINDOWS
	exit(1); 
#endif
/*	checkexit(1); */ /* presumably only called direct from command line */
}


/* \t0: do not flush zero size kerning pairs\n\ */

/* \tM: use Macintosh short font file name for FontID (instead of FontName)\n\ */

/*	\tl: minimum pairwise kerning recorded in TFM file\n\ */
/*	\tm: maximum number of kern pairs allowed per character\n\ */
/*	\t   (use `l' & `m' to limit no. of kern pairs in TFM file for old TeX)\n\ */

/* \tu: do not remove trailing `_'s from file names\n\ */

/* abcdefghijklmnopq.stuvwxyz used */

/* ABCDEFGHIJKLMNOPQRST.V.XYZ used */

/* \tZ: italic fuzz (default is 30)\n */

/* \t   (use `l' & `m' if there are too many kern pairs for TFM file)\n\ */

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': if (verboseflag) {
					  if (traceflag) debugflag = 1;
					  else traceflag = 1;
				  }
				  else verboseflag = 1; 
			return 0; 
/*		case 'v': verboseflag = 1; return 0;   */
/*		case 'x': addexe = 0; return 0;  */
		case 'x': dontexten = 1; return 0; 
		case 'n': suppressligsini = 1; boundarychar = NULL; return 0; 
		case 'L': allowligs = 1; return 0; 
		case 'a': texligflag = 1; return 0;	
		case 'f': pseudoligs = 1; return 0;		/* 95/Jan/22 */
		case 'd': extendligs = 1; return 0; 
		case 'j': accentligs = 1; return 0;
		case 'i': showital = 1; return 0;
		case 'I': suppressitalic = 1; return 0;
		case 'u': underscoreflag = 1; return 0; 
		case 'B': suppressbold = 1; return 0;
		case 'z': limitatzero = 0; return 0;
		case 't': texturesflag = 1; return 0;
/*		case 'M': usecontraction = 1; return 0; */ /* 92/Dec/23 */
		case 'M': usecontraction = 1 - usecontraction; return 0; /* 98/Jan/26 */
		case '0': ignorezeros = 0; return 0;	/* 92/Dec/23 */
		case 'S': borrowsymbol = 1; return 0;	/* 93/Aug/3 */
		case 'P': paulflag = 1 - paulflag; return 0;	/* 94/Sep/8 */
		case 'J': alanflag = 1 - alanflag; return 0;	/* 94/Sep/8 */
		case 'N': averageflag = 1 - averageflag; return 0;	/* 95/Aug/3 */
		case 'R': sebasflag = 1 - sebasflag; return 0;	/* 95/Aug/14 */
		case 'r': showwidthflag = 1 - showwidthflag; return 0;
		case 'E': errout = stderr; return 0;		/* 96/Mar/3 */
		case 'K': ecflag = !ecflag; return 0;		/* 97/Feb/5 */
		case 'D': avoiddups = !avoiddups; return 0;	/* 97/May/29 */
		case 'o': optimize = !optimize; return 0;	/* 97/Oct/1 */
		case 'V': showoptimize = !showoptimize; return 0;
		case 'F': replicateleftflag = 1;	/* adds little to file size */
				  presortflag = 1; optimize = 1; return 0;	/* 97/Oct/19 */
		case 'G': replicaterightflag = 1;	/* can add a lot to file size */
				  presortflag = 1; optimize = 1; return 0;	/* 97/Oct/19 */
		case 'A': allowaliases = 1; return 0;	/* 94/Aug/9 */
		case 'q': allowdotlessi = 1- allowdotlessi; return 0;
		case 'Q': allowdottedi = 1- allowdottedi; return 0; /* 95/Apr/22 */
		case 'y': sortonname = !sortonname; return 0;	/* 97/Oct/30 */
		case 'h': alignmentflag = !alignmentflag;	/* 97/Dec/21 */
		case 'T': vetonontext = !vetonontext; return 0; /* veto 98/Apr/5 */
		case 'H': boundarychar = NULL; return 0; /* 98/Aug/15 */
		case 'X': forcexerox = ! forcexerox; return 0;	/* 98/Aug/24 */
		case 'Y': hideyandy = ! hideyandy; return 0;	/* 98/Aug/24 */
		case 'k': greekparens = ! greekparens; return 0;	/* 99/Apr/10 */
		case 'W': letrasetflag = ! letrasetflag ; return 0;	/* 99/Sep/24 */
/* following may be obsolete/debug only */
/*		case 'b': backwardflag = 1; fontchrs = 128; return 0; */
		case '1': ansitexflag = 0; return 0;	/* 93/Dec/17 */
		case '3': forcelong = 1; return 0;
		case 'w': fullwidth = 1; return 0;
/* rest take arguments */
		case 's': spaceflag = 1; return 1; 
		case 'e': extraflag = 1; return 1; 
		case 'm': maxkernflag = 1; return 1; 
		case 'l': minkernflag = 1; return 1; 
		case 'c': vectorflag = 1; return 1; 
		case 'C': checksumflag = 1; return 1;		/* 93/May/14 */
		case 'Z': italicfuzzflag = 1; return 1;	/* 94/Nov/1 */
		case 'O': outputpathflag = 1; return 1;	/* 95/Jan/2 */
		case 'p': pathflag = 1; return 1; 
		case 'g': boundarygapflag = 1;	presortflag = 1;
				  return 1;			/* add boundary char kerns */
		default: {
			sprintf(logline, "Invalid command line flag '%c'", c);
			showline(logline, 1);
			checkexit(7);
			return -1;
		}
	}
//	return -1;		/* drop through via break - need argument */
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

void stampit (char *buffer) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);
	sprintf(buffer, "%s %s (%s %s)\n",
			appname, programversion, date, compiletime);
}

char *removepath(char *);

int commandline (int argc, char *argv[], int firstarg) {	/* command line flags */
	int c, flag;
	unsigned int i;
	char *s;

	if (argc < 2) {
//		showusage(argv[0]);
		showline("ERROR: too few arguments\n", 1);
		return -1;
	}

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for (i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			flag = decodeflag(c);
			if (flag < 0) {
//				showline("BAD ARGUMENT FLAG\n", 1); // debugging only
				return -1;	// failure
			}
			if (flag > 0) {			/* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++;
					s = argv[firstarg];
				}
				else s++;
				if (vectorflag) {
					vectorfile = s;						/* 95/Feb/19 */
					strncpy(codingvector, removepath(s), MAXENCODING);
					vectorflag = 0;
					reencodeforg = 1; reencodeflag = 1; 
				}
				else if (checksumflag) {
					if (sscanf(s, "%lx", &checksum) < 1) {
						sprintf(logline, "Don't understand checksum %s\n", s);
						showline(logline, 1);
					}
					checksumflag = 0; checksumset = 1;
				}
				else if (italicfuzzflag) {
					if (sscanf(s, "%lg", &italicfuzz) < 1) {
						sprintf(logline, "Don't understand italic fuzz %s\n", s);
						showline(logline, 1);
					}
					italicfuzzflag = 0;
				}
				else if (spaceflag) {
					if (sscanf(s, "%lg,%lg,%lg", 
						&spacewidth, &spacestretch, &spaceshrink) < 1) {
						sprintf(logline, "Don't understand space %s\n", s);
						showline(logline, 1);
					}
/* alternate form to get over batch use of comma as separator 94/Sep/8 */
					else if (sscanf(s, "%lg:%lg:%lg", 
						&spacewidth, &spacestretch, &spaceshrink) < 1) {
						sprintf(logline, "Don't understand space %s\n", s);
						showline(logline, 1);
					}
					else spacefixed = -1;	/* force use of this space */
					spaceflag = 0; 
				}
				else if (extraflag) {
					if (sscanf(s, "%lg", &extraspace) < 1) {
						sprintf(logline, "Don't understand extra space %s\n", s);
						showline(logline, 1);
					}
					else extrafixed = -1;	/* force use of this space */
					extraflag = 0; 
				}
				else if (boundarygapflag) {
					if (sscanf(s, "%lg", &boundaryspace) < 1) {
						sprintf(logline, "Don't understand boundary kern %s\n", s);
						showline(logline, 1);
					}
					else {
						if (traceflag) {
							sprintf(logline, "boundary space %lg\n", boundaryspace); 
							showline(logline, 1);
						}
						boundaryadd = 1;
					}
					boundarygapflag = 0; 
				}
				else if (pathflag) {
					vectorpath = s;
					pathflag = 0;
				}
				else if (outputpathflag) {
					outputpath = s;
					outputpathflag = 0;
				}
				else if (maxkernflag) {
					if (sscanf(s, "%d", &maxcharkern) < 1) {
						sprintf(logline, "Don't understand maxcharkern %s\n", 
							s);
						showline(logline, 1);
					}
					maxkernflag = 0; 
				}
				else if (minkernflag) {
					if (sscanf(s, "%lg", &minkernsize) < 1) {
						sprintf(logline, "Don't understand minkernsize %s\n", 
							s);
						showline(logline, 1);
					}
					minkernflag = 0; 
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* strips extension and underscores before extension */

void removeunder(char *name) {
/*	char *s=name; */
	char *s;

	s = removepath(name);						 /* 95/July/18 fix */
/*	if ((s = strchr(s, '.')) == NULL) return; */ /* to be safe ! */
	if ((s = strchr(s, '.')) == NULL) s = s + strlen(s);
	s--;
/*	while (*s == '_') s--; */
	while (s > name && *s == '_') s--;
	*(s + 1) = '\0';	
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

int underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = removepath(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
#ifdef DEBUGGING
	if (traceflag) {
		sprintf(logline, "n = %d m = %d ", n, m);
		showline(logline, 0);
	}
#endif
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
#ifdef DEBUGGING
	if (traceflag) {
		sprintf(logline, "Trying: %s\n", s);
		showline(logline, 0);
	}
#endif
	return 0;
}

/* remove file name - keep only path - inserts '\0' to terminate */

void stripname(char *pathname) {
	char *s;
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	sprintf(logline, "HASHED %ld\n", hash);	
	showline(logline, 1);
#ifdef DEBUGGING
	pause();
#endif
	return hash;
}

#ifdef CONTROLBREAK
void cleanup(void) {
	if (output != NULL) {
		fclose(output);				/* close output */
		(void) remove(fn_out);		/* and remove bad file */
	}
/*	fcloseall();  */
}
#endif

#ifdef CONTROLBREAK
void ctrlbreak (int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
/*	cleanup(); */
	if (bAbort++ >= 3) exit(3);				/* emergency exit */
/*	abort(); */
	(void) signal(SIGINT, ctrlbreak);
}
#endif

int abortjob (void) {
	cleanup();
	checkexit(3);
	return -1;
}

/* returns non-zero if name extended with `x' otherwise 0 if not possible */

int addanex(char *filename) {		/* add `x' at end of name if possible */
	int n;
	char *sname;				/* 95/Jan/2 */
	char *s;

/*	if (traceflag) sprintf(logline, "Attempting to extend %s with `x'\n", name); */
	sname = removepath(filename);
	n = (int) strlen(sname);				/* length of name */
/*	n = (int) strlen(name); */				/* length of name */
	if (n > 8) return 0;					/* name too long! bug */
	s = sname + n - 1;						/* last character */
/*	s = name + n - 1; */					/* last character */
	if (n == 8) {							/* full 8 characters */
		if (*s == '_') {					/* extended with underscores */
			while (*s-- == '_') ;			/* step back over underscores */
			s++;							/* step forward again */
/*			if (*(s+1) != 'x') *(s+2) = 'x';  */
			if (*s != 'x' && *s != 'X') {	/* 93/July/1 */
				*(s+1) = 'x';
				return 1;					/* was extended with x */
			}
		}
		else return 0;						/* no underscores in full name */
	}
/*	else if (*s == 'x') return; */
	else if (*s == 'x' || *s == 'X') return 0;	/* 93/July/1 */
	else {
		s++; *s++ = 'x'; *s = '\0';
		return 1;							/* was extended with x */
	}
	return 0;
}

void doansitex (void) {	/* add TeX text chars to Windows ANSI 93/Dec/18 */
/*	if (bc != 18)
		sprintf(logline, "WARNING: ANSITeX bc %d != 18\n", bc); */
	if (bc != 16) {		/* revised 93/Dec/29 */
		sprintf(logline, "WARNING: ANSITeX bc %d != 16\n", bc);
		showline(logline, 0);
	}
/*  make sure to use 0 for flag since grave & acute may have lig progs */
	copymetrics(16, 'i', 0);	/* fake dotlessi */	
/*	17 dotlessj */
	copymetrics(18, 96, 0);		/* grave */		/* only important ! */
	copymetrics(19, 180, 0);	/* acute */		/* only important ! */
/*	20 caron */					/* not in ANSI */
/*  21 breve */					/* not in ANSI */
	copymetrics(22, 175, 0);	/* macron */
	copymetrics(23, 176, 0);	/* ring */	/* faulty: 176 degree in PSCRIPT */
	copymetrics(24, 184, 0);	/* cedilla */
	copymetrics(25, 223, 0);	/* germandbls */
	copymetrics(26, 230, 0);	/* ae */
	copymetrics(27, 156, 0);	/* oe */
	copymetrics(28, 248, 0);	/* oslash */
	copymetrics(29, 198, 0);	/* AE */
	copymetrics(30, 140, 0);	/* OE */
	copymetrics(31, 216, 0);	/* Oslash */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* free kern and lig tables */

void freekernlig(void) {
	if (traceflag) showline("Freeing kern and lig tables\n", 0);
	if (kernbuffer != NULL) _ffree(kernbuffer);
	if (kern != NULL) _ffree(kern);
	if (kernsucc != NULL) _ffree(kernsucc);
	if (ligsucc != NULL) _ffree(ligsucc);
	if (ligature != NULL) _ffree(ligature);
}

/* allocate kern and lig tables -- returns 0 if not enough space */
/* this makes ligature tables same size as kern tables - not abs required */

int allocatekernlig (void) {			/* 94/July/18 */
	unsigned int bufsize; 
	
/*	bufsize = MAXKERN * sizeof(struct kernpair); */
	bufsize = kernligsize * sizeof(struct kernpair);
	if (traceflag) {
		sprintf(logline, "Allocating %u bytes for kernbuffer\n", bufsize);
		showline(logline, 0);
	}
	kernbuffer = (unsigned char __far *) _fmalloc(bufsize);
/*	kern = (long __far *) _fmalloc(MAXKERN * sizeof(long)); */
	kern = (long __far *) _fmalloc(kernligsize * sizeof(long));
/*	kernsucc = (char __far *) _fmalloc(MAXKERN); */
	kernsucc = (char __far *) _fmalloc(kernligsize);
	if (kernbuffer == NULL || kern == NULL || kernsucc == NULL) {
		sprintf(logline, "NOTE: Unable to allocate memory for %s %d\n", "kern table",
			kernligsize);
		showline(logline, 0);
/*		checkexit(1); */
		freekernlig();			/* 97/Oct/1 */
		return 0;
	}
/*	ligsucc = (char __far *) _fmalloc(MAXLIG); */
	ligsucc = (char __far *) _fmalloc(kernligsize);
/*	ligature = (char __far *) _fmalloc(MAXLIG); */
	ligature = (char __far *) _fmalloc(kernligsize);
	if (ligature == NULL || ligsucc == NULL) {
		sprintf(logline, "NOTE: Unable to allocate memory for %s %d\n", "ligature table",
			kernligsize);
		showline(logline, 0);
/*		checkexit(1); */
		freekernlig();			/* 97/Oct/1 */
		return 0;
	}
	return 1;
}

/* New - see whether characters borrowed from symbol font (Mac only) */

int checkborrow(int pass) {			/* pass not used */
	int k, m, borrow = 0;

	if (! usemacencoding || ! borrowsymbol) return 0;
/*  If text font, may borrow up to 20 math characters from Symbol font */
	if (verboseflag) showline("\n", 0);
	for (m = 0; m < 256; m++) {			/* step through `math' chars */
		if ((k = mathchar[m]) == 0) break;
		if (width[k] == NOWIDTH) {		/* is width still unknown */
/*			width[k] = symwidth[m]; */	/* use width from Symbol font */
			width[k] = mapdouble(symwidth[m]);	/* use width from Symbol font */
			if (k < bc) bc = k;
			if (k > ec) ec = k;
/*			charhit[k]++; */
			borrow++;
			if (verboseflag) {
				sprintf(logline, "%s~ ", encoding[k]);
				showline(logline, 0);
			}
		}
	}
	if (verboseflag) {
		if (borrow > 0) {
			if (verboseflag) showline("\n", 0);
			sprintf(logline, "WARNING: borrowing %d widths from Symbol\n",
				borrow);
			showline(logline, 0);
		}
	}
	return borrow;
}

double showalphasum (void) {
	int i;
	long sumwidths = 0;
	for (i = 'a'; i <= 'z'; i++)
		sumwidths += width[i];
//	printf("Sum of widths %lg average %lg\n",
//		   unmap(sumwidths), unmap(sumwidths)/26.0);
	printf("Average width %lg\n", unmap(sumwidths)/26.0);
	return sumwidths;
}

/****************************************************************/

/* main workhorse function */

int processfile(char *filename) {
	int k, ret;
	char *s, *t;

/*	may need to reinitialize things here for next file */
	fxll=0.0; fyll=0.0; fxur=1000.0; fyur=1000.0;
	nchrs=0; nkern=0; lnext=0; knext=0; killkern=0;
	ansiextend=0;
/*	fchrs=0;   */
	strcpy(charcodingscheme, "");	 
	strcpy(fontid, ""); 
	strcpy(fullname, "");
	strcpy(familyname, ""); 
	strcpy(macintoshname, "");
	reencodeflag = reencodeforg; /* reset to command line version each file */

	if (reencodeforg && ! encodingset) {			/* 95/Feb/19 */
		if (strcmp(vectorfile, "") == 0) ret = readencoding(codingvector);
		else ret = readencoding(vectorfile);
		if (ret < 0) return ret;
	}

/*	default values */
	designsize = 10.0;		/* default */
	italicangle = 0.0;		/* default */
/*	em_quad = 1000.0; */
	em_quad = 0.0;
/*	capheight = 750.0; */
	capheight = 0;			/* so can fill in at end */
/*	xheight = 500.0; */
	xheight = 0;			/* so can fill in at end */
/*	ascender = 800.0; */
	ascender = 0;
/*	descender = -250.0; */
	descender = 0;
/*	following may be overridden from command line */
	if (!  spacefixed) {
		spacewidth = 300.0;		/* 333.333; */
		spacestretch = 160.0;	/* 166.667; */
		spaceshrink = 110;		/* 111.111; */
	}
	if (! extrafixed) extraspace = 110;		/* 111.111; */
	rightboundary=-1;			/* 97/Oct/1 */
	leftboundary=-1;			/* 97/Oct/1 */
	leftprogram=-1;				/* 97/Oct/1 */

	for(k=0; k < fontchrs; k++) extenpointer[k] = -1;
	for(k=0; k < fontchrs; k++) ascendpointer[k] = -1;

/*	strcpy(fn_in, argv[m]); */
	strcpy(fn_in, filename);

	if (strstr(fn_in, "tfm") != NULL ||
		strstr(fn_in, "TFM") != NULL) { /* tfm file input */
		showline("Use TFMTOAFM instead!\n", 1);
/*			exit(7); */
		checkexit(7);
		return -1;
	}
/*		else {		*/	/* afm input file */
	extension(fn_in, "afm");
/*		 sprintf(logline, "Processing afm file %s\n", fn_in); */
	if ((input = fopen(fn_in, "r")) == NULL) {
		if (tryunderscore) (void) underscore(fn_in);
		if ((input = fopen(fn_in, "r")) == NULL) {
			sprintf(logline, "ERROR: Can't open input file: %s ", filename);
			showline(logline, 1);
			perrormod(fn_in);
/*			exit(5); */
			checkexit(5);
			return -1;
		}
	}
	if (verboseflag) {
		sprintf(logline, "PROCESSING AFM FILE: %s\n", fn_in);
		showline(logline, 0);
	}
#ifdef DEBUGGING
	if (traceflag) {
		sprintf(logline, "File %s is open\n", fn_in);
		showline(logline, 0);
	}
#endif

/*	longtable = 0; */
	longtable = forcelong;
	if (traceflag) {
		sprintf(logline, "Resetting longtable to %d\n", longtable);	/* ??? */
		showline(logline, 0);
	}

/*	reset the `EC ' parameters */
	cap_height = asc_height = acc_cap_height = desc_depth = 0;
	max_height = max_depth = digit_width = cap_stem = baseline_skip = 0;

	if (readafmfile(input, 0) != 0) {	/* pass 1 */
		fclose(input);
/*		continue;	*/		/* some serious error 93/Dec/19 */
		return -1;			/* give up for serious error */
	}

	if (showwidthflag) showalphasum();
	if (usemacencoding && borrowsymbol) checkborrow(0);
	if (verboseflag) listmissing();
	if (isfixedpitch && ! suppressligs) {	/* 93/Jan/5 */
/*	sprintf(logline, "WARNING: fixed pitch font, use `n' command line flag\n"); */
		if (! allowligs) {						/* 93/May/29 */
			sprintf(logline,
		"WARNING: suppressing ligatures in fixed pitch font (%s)\n",
					fn_in);
			showline(logline, 0);
			suppressligs = 1;							/* 93/May/29 */
/*	perhaps turn off texligflag and turn on pseudoligs if it was on ??? */
			if (texligflag) pseudoligs = 1;			/* 97/June/5 */
			texligflag = 0;							/* 97/June/5 */
		}
	}
	ansiextend = 0;							/* new stuff 93/Dec/18 */
	if (strcmp(codingvector, "ansinew") == 0 ||
		strcmp(codingvector, "ansi") == 0) {
		if (ansitexflag) {
/*					if (bc < 32) */
/* its bc != ec because of case where there is nothing in font */
			if (bc != ec) {
/* it is 30 in order to deal with TrueType crap */
				if (bc < 30) {
					sprintf(logline,
							"Can't add TeX text to ANSI if bc = %d < 32\n",
							bc);
					showline(logline, 0);
				}
				else ansiextend = 1;
			}
		}
	}
/*			if (ansiextend && bc > 18) bc = 18; */			/* 93/Dec/19 */
/*			sprintf(logline, "ANSIEXTEND == %d bc == %d\n", ansiextend, bc); */
/* Moved down here after above 93/May/29 - so change in suppressligs works */
	rewind(input);				/* get set for second pass */

	if (readafmfile(input, 1) != 0) {	/* pass 2 */
		fclose(input);
/*		continue; */	/* some serious error 93/Dec/19 */
		return -1;			/* give up for serious error */
	}

	if (usemacencoding && borrowsymbol) checkborrow(1);

/*			if (ansiextend && bc > 18) bc = 18; */			/* 93/Dec/19 */
/*			sprintf(logline, "ANSIEXTEND == %d bc == %d\n", ansiextend, bc); */

	if (texligflag) {
		if (suppressligs && ! allowligs) {
			showline(
	"WARNING: Attempt to add Tex pseudo ligs to fixed width font ignored\n", 0);
		}
		else {
			if (letrasetflag)
				ret = addligatures(letraligatures1, 0, 1);
			else ret = addligatures(texligatures1, 0, 1);	/* *real* f ligatures */
			if (ret < 0) return -1;
			ret = addligatures(texligatures2, 0, 2);	/* en-, emdash, quotes */
			if (ret < 0) return -1;
		}
	}
/*	provide mechanism to *only* add pseudo ligs without f-ligatures */
	else if (pseudoligs) {					/* 95/Jan/22 */
		ret = addligatures(texligatures2, 0, 2);		/* en-, emdash, quotes */
		if (ret < 0) return -1;
	}
/*	allow the following even for fixed width fonts */
	if (extendligs) {				/* guillemots, quotedblbase */
		if (greekparens) ret = addligatures(alterligatures, 0, 3);
		else ret = addligatures(extraligatures, 0, 3);
		if (ret < 0) return -1;
	}
	if (accentligs) {				/* 58 accented chars */
		ret = addligatures(accentligatures, 1, 4);
		if (ret < 0) return -1;
	}
	if (ansiextend) doansitex();		/* 93/Dec/18 ??? */
#ifdef DEBUGGING
	if (traceflag) {
		sprintf(logline, "%d (encoded) charnames\n", checkhowmany());
		showline(logline, 0);
	}
#endif

	if (ferror(input)) {
		showline("Error in input file ", 1);
		perrormod(fn_in);
/*				exit(5); */
		checkexit(5);
		return -1;
	}
	else fclose(input);

	if (verboseflag) {
		sprintf(logline, "Total of %d characters in font\n", nchrs); /* nkern */
		showline(logline, 0);
	}
	if (fchrs < 10 && reencodeflag)	{
		sprintf(logline,
	"WARNING: Only %d encoded character%s, possible mismatch with encoding?\n",
				fchrs, (fchrs == 1) ? "" : "s");
		showline(logline, 0);
	}
	if (verboseflag) {
		sprintf(logline, "%d encoded character%s, %d ligature%s, %d kern%s\n",
			   fchrs, (fchrs == 1) ? "" : "s",
				lnext, (lnext == 1) ? "" : "s",
				knext, (knext == 1) ? "" : "s");
		showline(logline, 0);
	}
#ifdef DEBUGGING
	if (traceflag) {
		sprintf(logline, "File %s is closed\n", fn_in);
		showline(logline, 0);
	}
#endif
	if (bAbort > 0) {
		abortjob();				/* 92/Nov/24 */
		return -1;
	}

	buildtables();
	buildkerntable();

	if (ne > 0) {
/*		if (verboseflag) { */
		if (traceflag) {
			showline("\n", 0);
			sprintf(logline, "ne is %d\n", ne);
			showline(logline, 0);
		}
	}
#ifdef DEBUGGING
	if (traceflag) {
		showwidths(); showheights(); showdepths();	showitalics(); 
		showkerns();
	}
#endif		
	if (isfixedpitch && boundaryadd && textfontflag) {
		showline(
			"ERROR: using boundary character kerns in fixed width\n", 1);
	}
	if (isfixedpitch && nkern > 0) {
		showline("ERROR: kerning in fixed width font\n", 1);
	}
	if (bAbort > 0) {
		abortjob();				/* 92/Nov/24 */
		return -1;
	}
	ret = constructtfm(removepath(fn_in));
	if (ret < 0) return -1;

/*		write output in current directory */
/*			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, '/')) != NULL) s++;	
			else if ((s=strrchr(fn_in, ':')) != NULL) s++;
			else s = fn_in; */
	s = removepath(fn_in);	
	if (strcmp(outputpath, "") == 0) strcpy(fn_out, s);
	else {					/* 95/Jan/2 */
		strcpy(fn_out, outputpath);
/*				strcat(fn_out, "\\"); */
		t = fn_out + strlen(fn_out) - 1;		/* 96/Oct/12 */
/*		outputpath may end in separator ... 96/Oct/12 */
		if (*t != '\\') strcat(fn_out, "\\");
/*		outputpath may end in double separator ... 96/Oct/12 */
		else if (t > fn_out && *(t-1) == '\\') *t = '\0';
		strcat(fn_out, s);
	}

	if (! underscoreflag) removeunder(fn_out);
/* decision whether to add an `x' at the end of the file name */
/*			if (addexe && reencodeflag && strlen(fn_out) < 8) { 
				n = strlen(fn_out);
				if (*(fn_out + n - 1) != 'x') strcat(fn_out, "x");
			} */
/*	don't extend with `x' if: */
/* (i) asked not to, (ii) no reencoding, or (iii) name too long */
/*			s = removepath(fn_out); */
	if (! reencodeflag ||
		strcmp(codingvector, "") == 0 ||			/* 93/July/10 */
		strcmp(codingvector, "none") == 0 ||		/* 93/July/10 */
		strcmp(codingvector, "standard") == 0 ||	/* 95/Jan/2 */
/*				strlen(fn_out) >= 8) ; */					/* 95/Jan/2 */
		dontexten ) ;
	else {
		if (addanex(fn_out) != 0) {
			sprintf(logline,
					"WARNING: extending name with `x': %s\n", fn_out);
			showline(logline, 0);
		}
									/* extend with `x' to indicate remapped */
	}

	forceexten(fn_out, "tfm"); /* char *ext="tfm";	*/

	sprintf(logline, "Processing file %s => %s\n", fn_in, fn_out);
	showline(logline, 0);

	if ((output = fopen(fn_out, "wb")) == NULL) {
		sprintf(logline, "ERROR: Can't open output file: %s ", fn_out);
		showline(logline, 1);
		perrormod(fn_out);
/*				exit(5); */
		checkexit(5);
		return -1;
	}
#ifdef DEBUGGING
	if (traceflag) {
		sprintf(logline, "File %s is open\n", fn_out);
		showline(logline, 0);
	}
#endif
	if (bAbort > 0) {
		abortjob();				/* 92/Nov/24 */
		return -1;
	}

	ret = writetfm(output); 
	if (ret < 0) return -1;

	if (ferror(output)) {
		showline("ERROR: in output file ", 1);
		perrormod(fn_out);
/*				exit(5); */
		checkexit(5);
		return -1;
	}
	else fclose(output);
	showline("\n", 0);

	if (! reencodeflag)			/* free if read from AFM file */
		freeencoding();				/* free again 94/July/18 */
	if (leftboundarychar != NULL) {	/* 98/Jun/30 */
		free(leftboundarychar);
		leftboundarychar = NULL;
	}
	leftboundary = -1;
	if (rightboundarychar != NULL) {	/* 98/Jun/30 */
		free(rightboundarychar);
		rightboundarychar = NULL;
	}
	rightboundary = -1;
		
	if (bAbort > 0) {
		abortjob();				/* 92/Nov/24 */
		return -1;
	}
	return 0;
}

void winshow(char *);

void winerror(char *);

/* entry point for console application version */

int main (int argc, char *argv[]) {
	int k, m, ret, firstarg=1;
	char *s;

#ifdef CONTROLBREAK
	(void) signal(SIGINT, ctrlbreak);
#endif

	verboseflag=0;							/* reset initial state */

#ifdef _WINDOWS
	strcpy(programpath, GetCommandLine());			/* 99/June/25 dviwindo.exe */
#else
	strncpy(programpath, argv[0], sizeof(programpath));
#endif
	stripname(programpath);

/*	sprintf(logline, "Default program path is %s\n", programpath); */
/*	if programpath exists, use as default for vec */
/*	if (strcmp(programpath, "") != 0) vectorpath = programpath; */

	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 94/June/14 */

	if ((s = getenv("VECPATH")) != NULL) vectorpath = s;	/* real DOS env var */

#ifndef _WINDOWS
	if (usedviwindo) setupdviwindo(); 	 /* need to do this before use */
#endif

	if (usedviwindo) {
		if ((s = grabenv("VECPATH")) != NULL) vectorpath = s;
	}

	checkenter(argc, argv);							/* 95/Oct/28 */

/*	task = "analyzing command line"; */

	firstarg = commandline(argc, argv, 1);

	if (firstarg < 0 || firstarg > argc - 1) {
		showusage(argv[0]);
		return -1;				// failure not enough args or bad args
	}

	stampit(logline);					/* ??? */
	showline(logline, 0);

	if (wantcpyrght) {
		sprintf(logline, "%s\n", copyright); 
		showline(logline, 0);
	}

	for (k = 0; k < MAXCHRS; k++) encoding[k] = "";		/* initialize */

	if(strcmp(codingvector, "") == 0) {			/* new - use default */
		strcpy(codingvector, defaultencoding);
		reencodeforg = 1;
/*		reencodeflag = 1; */
	}
	if(strcmp(codingvector, "none") == 0) {		/* check whether to use raw */
		strcpy(codingvector, "");
		reencodeforg = 0;
/*		reencodeflag = 0; */
	} 

	if (checkcopyright(copyright) != 0) {
/*		sprintf(logline, "HASH %lu", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

/* TFM file buffer allocated small at first, but allowed to grow later */

/*	buffer = (unsigned char __far *) _fmalloc(MAXBUFFER); */
	buffer = (unsigned char __far *) _fmalloc(INIMEMORY); 
	if (buffer == NULL) {
		sprintf(logline, "ERROR: Unable to allocate memory for %s\n", "TFM buffer");
		showline(logline, 1);
/*		exit(1); */
		checkexit(1);
		return -1;				// failure memory allocation
	}
	buffersize = INIMEMORY;						/* 94/July/18 */

/*  kern & lig tables allocated here and made smaller if not enough space */

	kernbuffer = NULL;
	kernsucc = ligsucc = ligature = NULL;
	kern = NULL;
	kernligsize = MAXKERN;		/* ==  MAXLIG */
	while (allocatekernlig () == 0 && kernligsize > 512) {
		kernligsize = kernligsize / 2;
		sprintf(logline, "NOTE: reducing size of kern & ligature tables to %d\n",
				kernligsize);
		showline(logline, 0);
		freekernlig();
		if (abortflag > 0) return -1;
	}
	if (kernligsize <= 512) {
		sprintf(logline, "ERROR: Unable to allocate memory for %s\n", "kern & ligature tables");
		showline(logline, 1);
/*		exit(1); */
		checkexit(1);
		return -1;
	}

	if (backwardflag) showline("WARNING: Backward compatability mode\n", 0);

	for (m = firstarg; m < argc; m++) {
		ret = processfile(argv[m]);
		if (ret < 0) return -1;
#ifndef _WINDOWS
		fflush(stdout);						/* ??? 98/Jun/30 */
#endif
	}	/* end of for loop over all input files */

	if (argc - firstarg > 1) {
		sprintf(logline, "Processed %d AFM files\n", argc - firstarg);
		showline(logline, 0);
	}

	if ((m = _heapchk ()) != _HEAPOK) {		/* 94/Feb/18 */
		sprintf(logline, "ERROR: heap corrupted (%d)\n", m);
		showline(logline, 1);
/*		exit(1); */
		checkexit(1);
		return -1;
	}

	if (reencodeforg) freeencoding();		/* free again 95/Feb/25 */
	freekernlig();
	if (leftboundarychar != NULL) free(leftboundarychar);
	if (rightboundarychar != NULL) free(rightboundarychar);
/*	if (kernbuffer != NULL) _ffree(kernbuffer);
	if (kern != NULL) _ffree(kern);
	if (kernsucc != NULL) _ffree(kernsucc);
	if (ligsucc != NULL) _ffree(ligsucc);
	if (ligature != NULL) _ffree(ligature); */
	if (buffer != NULL) _ffree(buffer);

	checkpause(0);
	if (abortflag) showline("Fatal Error in AFMtoTFM conversion\n", 1);
	return 0;				// OK
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Attempt to overcome compiler bug */
/*	c:\c\afmtotfm.c(2830) : fatal error C1001: internal compiler error */
/*		(compiler file 'msc2.cpp', line 1011) */

int compresswidthtable (void) {
	int k, i;
/*	double ta, tb; */

/*	assert(widthtable[0] == 0.0); */
	assert(widthtable[0] == 0);

	if (traceflag) {
		sprintf(logline, "nw %d ", nw);
		showline(logline, 0);
	}
	if (traceflag) {
		sprintf(logline, "- sorting %s - ", "widths");
		showline(logline, 0);
	}
/*	qsort(widthtable, (unsigned int) nw, sizeof(double), comparedouble);  */
	qsort(widthtable + 1, (unsigned int) (nw - 1), 	
/*			sizeof(double), comparedouble);  */
			sizeof(long), comparelong); 
#ifdef DEBUGGING
	if (traceflag) showwidths();
#endif

/*	k=1; */
	k = 2;
/*	for (i = 1; i < nw; i++) */	/* eliminate redundant entries, after zero */
	for (i = 2; i < nw; i++)	/* eliminate redundant entries, after one */
		if (widthtable[i] != widthtable[k-1]) widthtable[k++] = widthtable[i];
	nw = k;
/*	if (verboseflag) sprintf(logline, "nw %d ", nw); */
	if (traceflag) {
		sprintf(logline, "nw %d ", nw);
		showline(logline, 0);
	}
/*	if (widthtable[0] != 0.0) { 
		ta = widthtable[0];		widthtable[0] = 0.0; 
		for (i = 1; i < nw; i++) {
			tb = widthtable[i];		widthtable[i] = ta;
			if (tb == 0.0) break;
			ta = tb;
		}
	} */
	return 0;
}

int compressheighttable(void) { /* try to any way */
	int k, i, kmin, mincount;
	double mincost, cost;
	long separ;

/*	assert(heighttable[0] == 0.0); */
	assert(heighttable[0] == 0);
/*	for (k=bc; k <= ec; k++) heighttable[k-bc+1] = height[k]; */
/*	nh = ec - bc + 2;	*/
	if (traceflag) {
		showline("\n", 0);
		sprintf(logline, "%s %d %s %s ", "nh", nh, "sorting", "heights");
		showline(logline, 0);
	}

/*	qsort(heighttable, (unsigned int) nh, sizeof(double), comparedouble);  */
	qsort(heighttable + 1, (unsigned int) (nh - 1), 
/*			sizeof(double), comparedouble); */
			sizeof(long), comparelong); 
#ifdef DEBUGGING
	if (traceflag) showheights();
#endif

	k=1; counttable[k-1]=1;
	for (i = 1; i < nh; i++) {	/* remove redundant entries */
		if (heighttable[i] != heighttable[k-1]) {
			heighttable[k++] = heighttable[i];	counttable[k-1]=1;
		}
		else counttable[k-1]++;
	}

	nh = k;
/*	if (verboseflag) sprintf(logline, "nh %d ", nh); */
	if (nh <= MAXHEIGHTS) return nh;			/* success */
	if (traceflag) {
		sprintf(logline, "nh %d - %s", nh, "coalescing height table");
		showline(logline, 0);
	}

/*	try and scrunch down */
	for (;;) {
		mincost = 1000000000.0; kmin = -1;
		for (k = 2; k < nh; k++) {
			separ = heighttable[k] - heighttable[k-1];
/*			if (counttable[k] < counttable[k-1]) mincount  = counttable[k];
			else */
			mincount = counttable[k-1];		/* the smaller one goes	up */
/*			cost = separ * separ * (double) mincount; */
			cost = unmap(separ) * unmap(separ) * (double) mincount;
			if (cost < mincost) { mincost = cost; kmin = k; }
		}
		assert(kmin > 0 && kmin < nh);
/* 		if (counttable[kmin] > counttable[kmin-1])  */
		heighttable[kmin-1] = heighttable[kmin]; /* smaller goes up */
		counttable[kmin-1] = counttable[kmin] + counttable[kmin-1];
		for (k = kmin + 1; k < nh; k++) {
			heighttable[k-1] = heighttable[k];
			counttable[k-1] = counttable[k];
		}
			
		nh--;
		if(nh <= MAXHEIGHTS) return nh;
	}

/*		sprintf(logline, "Too many (%d) distinct heights ", nh);
		return -1; */
}

int compressdepthtable(void) { /* try to any way */
	int k, i, kmin, mincount;
	double mincost, cost;
	long separ;

/*	assert(depthtable[0] == 0.0); */
	assert(depthtable[0] == 0);
/*	for (k=bc; k <= ec; k++) depthtable[k-bc+1] = depth[k]; */
/*	nd = ec - bc + 2;	*/
	if (traceflag) {
		showline("\n", 0);
		sprintf(logline, "%s %d %s %s ", "nd", nd, "sorting", "depths");
		showline(logline, 0);
	}

/*	qsort(depthtable, (unsigned int) nd, sizeof(double), comparedouble);  */
	qsort(depthtable + 1, (unsigned int) (nd - 1), 
/*			sizeof(double), comparedouble); */
			sizeof(long), comparelong); 
#ifdef DEBUGGING
	if (traceflag) showdepths();
#endif

	k=1; counttable[k-1]=1;
	for (i = 1; i < nd; i++) {
		if (depthtable[i] != depthtable[k-1]) {
			depthtable[k++] = depthtable[i];  counttable[k-1]=1;
		}
		else counttable[k-1]++;
	}

	nd = k;
/*	if (verboseflag) sprintf(logline, "nd %d ", nd); */
	if (nd <= MAXDEPTHS) return nd;					/* success */
	if (traceflag) {
		sprintf(logline, "nd %d - %s", nd, "coalescing depth table");
		showline(logline, 0);
	}


/*	try and scrunch down */
	for (;;) {
		mincost = 1000000000.0; kmin = -1;
		for (k = 2; k < nd; k++) {
			separ = depthtable[k] - depthtable[k-1];
/*			if (counttable[k] < counttable[k-1]) mincount  = counttable[k];
			else */
			mincount = counttable[k-1]; 		/* the smaller one goes	up */
/*			cost = separ * separ * (double) mincount; */
			cost = unmap(separ) * unmap(separ) * (double) mincount;
			if (cost < mincost) { mincost = cost; kmin = k; }
		}
		assert(kmin > 0 && kmin < nd);
/*		if (counttable[kmin] > counttable[kmin-1])  */
		depthtable[kmin-1] = depthtable[kmin]; /* smalled goes up */
		counttable[kmin-1] = counttable[kmin] + counttable[kmin-1];
		for (k = kmin + 1; k < nd; k++) {
			depthtable[k-1] = depthtable[k];
			counttable[k-1] = counttable[k];
		}
			
		nd--;
		if(nd <= MAXDEPTHS) return nd;
	}

/*		sprintf(logline, "Too many (%d) distinct depths ", nd);
		return -1; */
}

int compressitalictable(void) { /* try to any way */
	int kmin, k, i, mincount;
	double mincost, cost;
	long separ;

/*	assert(italictable[0] == 0.0); */
	assert(italictable[0] == 0);
/*	for (k=bc; k <= ec; k++) italictable[k-bc+1] = italic[k]; */
/*	ni = ec - bc + 2; */
	if (traceflag) {
		showline("\n", 0);
		sprintf(logline, "%s %d %s %s ", "ni", ni, "sorting", "italic corrections");
		showline(logline, 0);
	}

/*	qsort(italictable, (unsigned int) ni, sizeof(double), comparedouble);  */
	qsort(italictable + 1, (unsigned int) (ni - 1), 
/*			sizeof(double),	comparedouble);  */
			sizeof(long), comparelong); 
#ifdef DEBUGGING
	if (traceflag) showitalics();
#endif

	k=1; counttable[k-1]=1;
	for (i = 1; i < ni; i++) {
		if (italictable[i] != italictable[k-1])  {
			italictable[k++] = italictable[i]; counttable[k-1]=1;
		}
		else counttable[k-1]++;
	}

	ni = k;
/*	if (verboseflag) sprintf(logline, "ni %d ", ni); */
	if (ni <= MAXITALICS) return ni;		/* success */
	if (traceflag) {
		sprintf(logline, "ni %d - %s", ni, "coalescing italic table");
		showline(logline, 0);
	}

/*	try and scrunch down */
	for (;;) {
		mincost = 1000000000.0; kmin = -1;
		for (k = 2; k < ni; k++) {
			separ = italictable[k] - italictable[k-1];
/*			if (counttable[k] < counttable[k-1]) mincount  = counttable[k];
			else */
			mincount = counttable[k-1];		/* the smaller one goes	up */
/*			cost = separ * separ * (double) mincount; */
			cost = unmap(separ) * unmap(separ) * (double) mincount;
			if (cost < mincost) { mincost = cost; kmin = k; }
		}
/* 		if (counttable[kmin] > counttable[kmin-1])  */
		assert(kmin > 0 && kmin < ni);
		italictable[kmin-1] = italictable[kmin]; /* smaller goes up */
		counttable[kmin-1] = counttable[kmin] + counttable[kmin-1];
		for (k = kmin + 1; k < ni; k++) {
			italictable[k-1] = italictable[k];
			counttable[k-1] = counttable[k];
		}
			
		ni--;
		if(ni <= MAXITALICS) return ni;
	}

/*	if (ni > MAXITALICS) {
		sprintf(logline, "Too many (%d) distinct italics", ni);
		return -1;
	}
	else return ni;	*/
}

/* An experiment 97/May/29 */

int checkdupsub (char *charname, char *alternate) {
	int k, count;
	int klast=0;
	count = 0;
	if (traceflag) {
		sprintf(logline, "CHECKDUPSUB %s %s\n", charname, alternate);
		showline(logline, 0);
	}
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], charname) == 0) {
			klast=k;
			count++;
		}
	}
	if (count > 1) {
		if (strcmp(encoding[k], "") != 0) {
			if (strcmp(encoding[klast], "") == 0) {
				showline("Error in checkdupsub\n", 1);
			}
			free(encoding[klast]);
			encoding[klast] = "";	/* ? */
		}
		encoding[klast] = zstrdup(alternate);
		if (encoding[klast] == NULL) return -1;
		if (verboseflag) {
			sprintf(logline, "char code %d %s => %s\n", klast, charname, alternate);
			showline(logline, 0);
		}
	}
	return 0;
}

int checkdups (void) {
	int ret;
	ret = checkdupsub("hyphen", "sfthyphen");
	if (ret < 0) return ret;
	ret = checkdupsub("space", "nbspace");
	if (ret < 0) return ret;
	ret = checkdupsub("bar", "brokenbar");
	if (ret < 0) return ret;
	return 0;
}


int writeligkern (void) {
	int k, i, skipbyte, nextchar, opbyte, remainder;
	int n=0;			/* not accessed */
	int markit=0, ligkernstar;
	int keqv, ret;

	if (traceflag) showline("WriteLigKern:\n", 0);
/*	dprintf(logline, "Writing ligkern left %d right %d\n", leftboundary, rightboundary); */

/*	if (! longtable) nforward = 0;
	else nforward = howmany(); */

	ligkernstar = bufferinx; 	/* remember start of lig_kern table */

	ligkerninx = 0;
	if (rightboundary >= 0 && textfontflag) {	/* 98/Apr/5 */
		if (traceflag) showline("First word for rightboundary\n", 1);
/*		if (traceflag) sprintf(logline, "Writing very %s word %s boundary\n",
							  "first", "right"); */
/*		The very first instruction in ligkern array skip-byte=255 */
		ret = writefour(255, rightboundary, 0, 0);
		if (ret < 0) return -1;
		ligkerninx++;			/* space for boundary char step */
		if (traceflag) {
			sprintf(logline, "boundary %d ligkerninx %d ligkernstar %d bufferinx %d\n",
				   rightboundary, ligkerninx, ligkernstar, bufferinx);
			showline(logline, 0);
		}
	}
/*	else sprintf(logline, "RIGHTBOUNDARY %d\n", rightboundary); */

	ligkernstar = bufferinx; /* remember `true' start 97/Oct/1 ??? */
							 /* may be offset by boundary char */

	if (longtable) {		
		if (traceflag) {
			sprintf(logline, "%d words for forwarding table\n", nforward);
			showline(logline, 0);
		}
/*		for (k = 0; k < fontchrs; k++) writelong(0L); *//* clean out table */
/*		leave space for indirect pointers */
/*		ligkerninx = ligkerninx + fontchrs; */				/* ??? */
		for (k = 0; k < nforward; k++) writelong(0L);   /* clean out table */
		if (abortflag) return -1;
/*		leave space for indirect pointers */ /* filled in later */
		ligkerninx = ligkerninx + nforward;			/* 92/April/4 */
		nextslot = 0;
	}
	maxligkern = ligkerninx;	 /* debugging */

	for (k = bc; k <= ec; k++) {	/* step through characters */
/*		moved back here 98/Sep/20 - is this right ??? */
		if (leftboundary >= 0 && k == leftboundary && textfontflag) { /* 98/Apr/5 */
			leftprogram = ligkerninx;	/* remember where special program */
			if (traceflag) {
				sprintf(logline, "Left program for %s (%d) at ligkerninx %d\n",
					   leftboundarychar, leftboundary, leftprogram);
				showline(logline, 0);
			}
		}
/*		No ligature program, and kern program is the same as earlier */
/*		if ((ligend[k] == ligbegin[k]) && kerneqv[k] != NOEQUIV) { */
		if ((keqv = kerneqv[k]) != NOEQUIV) {					/* 97/Oct/1 */
			if (ligend[k] == ligbegin[k]) {
				if (ligend[keqv] == ligbegin[keqv]) {
					if (traceflag) {
						sprintf(logline, "Char %s (%d) -> %s (%d)\n",
							   encoding[k], k, encoding[kerneqv[k]],
							   kerneqv[k]);
						showline(logline, 0);
					}
/*			ret = writepointer(ligkernstar + nextslot * 4, charrem[kerneqv[k]]); */
/*			nextslot++; */
					if (traceflag) {
						sprintf(logline, "k %3d reuse %3d\n", k, keqv);
						showline(logline, 0);
					}
					continue;	/* nothing to do - using existing program */
				}
/*				else if (verboseflag) */
				else if (traceflag) {
					sprintf(logline, "SPECIAL LIG CASE %s (%d) %s (%d)\n",
						   encoding[k], k,
						   encoding[keqv], keqv); /* debugging */
					showline(logline, 0);
				}
			}
		}
		if (longtable) {	/* if we have ligs or kerns, need pointer */
			if (ligend[k] != ligbegin[k] || kernend[k] != kernbegin[k])  {
/*				write pointer even if not used ? NO */
/*				ret = writepointer(ligkernstar + k * 4, ligkerninx); *//* ??? */
				if (traceflag) {
					sprintf(logline, "k %3d nextslot %3d ligkerninx %3d\n",
						   k, nextslot, ligkerninx);
					showline(logline, 0);
				}
/*				write the forwarding pointer into forwarding table */
				ret = writepointer(ligkernstar + nextslot * 4, ligkerninx); 
				if (ret < 0) return -1;
				nextslot++;
/*				sprintf(logline, "k %d ligkerninx %d ", k, ligkerninx); */
/*				ret = writepointer(ligkernstar + k * 4, bufferinx - ligkernstar); */
			}
		}

#ifdef IGNORED
		if (leftboundary >= 0 && k == leftboundary && textfontflag) { /* 98/Apr/5 */
			leftprogram = ligkerninx;	/* remember where special program */
			if (traceflag) {
				sprintf(logline, "Left program for %s (%d) at ligkerninx %d\n",
				   leftboundarychar, leftboundary, leftprogram);
				showline(logline, 0);
			}
		}
#endif

/*		first write all of the ligatures for this character */
		for (i = ligbegin[k]; i < ligend[k]; i++) {
			skipbyte = 0;
			nextchar = ligsucc[i];				/* far space */
			opbyte = 0;
			remainder = ligature[i];
/*			writelong(((long) ligsuck) << 16 | ligature[i]); */
			ret = writefour(skipbyte, nextchar, opbyte, remainder);
			if (ret < 0) return -1;
			markit=1;
			n++;
		}

/*		then write all of the kerns for this character */
		for (i = kernbegin[k]; i < kernend[k]; i++) {
			skipbyte = 0;
			nextchar = kernsucc[i];					/* far space */
			opbyte = KERNBIT;
/*			remainder = mapkern((double) kern[i]); */ /* double */
			remainder = mapkern(kern[i]); 
			if (remainder > 255) {
				opbyte = opbyte | (remainder >> 8);
				remainder = remainder & 255;
			}
/*			writelong(((long) kernsuck) << 16 | ((long) KERNBIT) << 8 | kerninx); */
			ret = writefour(skipbyte, nextchar, opbyte, remainder);
			if (ret < 0) return -1;
			markit=1;
		}
		if (markit) markend();	/* need to mark the end for this char ? */
		(void) updateinx(k);	/* space for lig/kern program ??? */
	}

	if (leftboundary >= 0 && textfontflag) {
		if (leftprogram <= 0) {
			sprintf(logline,
				"ERROR: Missing left boundary char ligkern program for %s (%d)\n",
					leftboundarychar, leftboundary);
			opbyte = remainder = 0;
			showline(logline, 1);
		}
		if (traceflag) {
			sprintf(logline, "Leftprogram at %d\n", leftprogram);
			showline(logline, 0);
		}
		remainder = leftprogram & 255;
		opbyte = leftprogram >> 8;
		ret = writefour(255, 0, opbyte, remainder);
		if (ret < 0) return -1;
		ligkerninx++;
		if (traceflag) {
			showline("Last word for leftboundary\n", 0);
		}
/*		if (traceflag) sprintf(logline, "Writing very %s word\n", "last"); */
	}
/*	if (maxligkern > nl) 
		sprintf(logline, "Max lig/kern (%d) > nl (%d)\n", maxligkern, nl); */
	if (ligkerninx > nl) {
		sprintf(logline, "ligkerninx (%d) > nl (%d)\n", ligkerninx, nl);
		showline(logline, 1);
		return -1;		/* ??? */
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////

// Here follows the new stuff for the DLL version

#ifdef _WINDOWS

#define WHITESPACE " \t\n\r"

HINSTANCE hInstanceDLL=NULL;		/* remember for this DLL */

/* This is the callback function for the EDITTEXT Control in CONSOLETEXT */

#define GET_WM_COMMAND_CMD(wParam, lParam)	(HIWORD(wParam))
#define GET_WM_COMMAND_ID(wParam, lParam)	(LOWORD(wParam))
#define GET_WM_COMMAND_HWND(wParam, lParam)	((HWND)lParam)

HWND hConsoleWnd=NULL;		/* Console Text Window Handle passed from DVIWindo */

void ShowLine (char *line, int errflag) {	/* windows version 99/June/11 */
	int ret;

	if (IsWindow(hConsoleWnd) == 0) {		// in case the other end died
		hConsoleWnd = NULL;
		abortflag++;						// kill job in this case ???
	}		// how expensive is this test ???

	if (hConsoleWnd != NULL) {
		SendMessage(hConsoleWnd, ICN_ADDTEXT, (WPARAM) line, 0L);
	}

	if (errflag) {
		errlevel++;
		ret =  MessageBox(NULL, line, "AFMtoTFM", MB_ICONSTOP | MB_OKCANCEL | MB_TASKMODAL);
		if (ret == IDCANCEL) abortflag++;
	}
}

void winshow(char *line) {
	(void) MessageBox(NULL, line, "AFMtoTFM",
					  MB_ICONINFORMATION | MB_OK | MB_TASKMODAL);
}

void winerror(char *line) {
	int ret;
	ret = MessageBox(NULL, line, "AFMtoTFM",
					 MB_ICONSTOP | MB_OKCANCEL | MB_TASKMODAL);
	if (ret == IDCANCEL) abortflag++;
}

// argument info constructed from command line 

int argc;

char **argv=NULL;

// Need to be careful here because of quoted args with spaces in them
// e.g. -d="G:\Program Files\Adobe\Acrobat\*.pdf"
// needs to be counted as one argument only,
// and needs to have " stripped out

int makecommandargs (char *line) {
	int argc=0;
	char *s, *t;

	if (line == NULL) return -1;		/* sanity check */

//	first figure out how many arguments are on command line
	s = line;
	while (*s != '\0') {
		while (*s <= 32 && *s > 0) s++;			// step over white space
		if (*s == '\0') break;					// hit the end
		t = s;
		while (*t > 32 && *t != '\"') t++;		// step over argument
		if (*t == '\"') {						// if "... delimited
			t++;
			while (*t > 0 && *t != '\"') t++;	// search for ..."
			if (*t == '\0') break;				// hit the end
			t++;
		}
//		argv[argc] = s;
		argc++;
		if (*t == '\0') break;
//		*t = '\0';
		s = t+1;							// look for next argument
	}

	if (argc == 0) return -1;			/* nothing to do */

	argv = (char **) malloc(argc * sizeof(char *));
	if (argv == NULL) {
		sprintf(logline, "ERROR: Unable to allocate memory for %s\n", "arguments");
		winerror(logline);
		return -1;
	}

//	Now extract argc arguments and put in argv array
	argc = 0;
	s = line;
	while (*s != '\0') {
		while (*s <= ' ' && *s > '\0') s++;		// step over white space
		if (*s == '\0') break;					// hit the end
		t = s;
		while (*t > ' ' && *t != '\"') t++;		// step over argument
		if (*t == '\"') {						// if "... delimited
//			t++;
			strcpy(t, t+1);						// flush "... 2000 May 27
			while (*t > 0 && *t != '\"') t++;	// search for ..."
			if (*t == '\0') break;				// hti the end
//			t++;
			strcpy(t, t+1);						// flush ..." 2000 May 27
		}
		argv[argc] = s;							// next argument
		argc++;
		if (*t == '\0') break;
		*t = '\0';
		s = t+1;								// look for next argument
	}

#ifdef DEBUGGING
	s = logline;
	*s = '\0';
	for (k = 0; k < argc; k++) {
		sprintf(s, "%d\t%s\n", k, argv[k]);
		s += strlen(s);
	}
	winshow(logline);
#endif
	return argc;
}

/* This is the new entry point of DLL called from DVIWindo */

int AFMtoTFM (HWND hConsole, char *line) {
	int firstarg=0;
	int flag;

	argc = makecommandargs(line);			// sets up global *argv[]

	if (argc < 0) return -1;				// sanity check

	firstarg = commandline(argc, argv, firstarg);	// analyze args

	if (firstarg < 0 || firstarg >= argc) {
		sprintf(logline, "ERROR: Too few arguments in %s\n", line);
		winerror(logline);
		return -1;
	}
	
	hConsoleWnd = hConsole;				// remember console window handle
	
	if (hConsoleWnd != NULL)
		SendMessage(hConsoleWnd, ICN_SETTITLE, (WPARAM) "AFM to TFM", 0L);
//	if (hConsoleWnd != NULL)
//		SendMessage(hConsoleWnd, ICN_RESET, 0, 0L); // if want to clear window

	(void) main(argc, argv);			// now run AFMtoTFM proper 

	if (errlevel > 0 || abortflag > 0) {
		sprintf(logline, "Errors in Processing (err %d abort %d)\n", errlevel, abortflag);
		winerror(logline);
	}

	if (hConsoleWnd != NULL) {
		if (errlevel > 0 || abortflag > 0) flag = 1;
		else flag = 0;
		SendMessage(hConsoleWnd, ICN_DONE, flag, 0);	// flush out buffer if needed
	}
	hConsoleWnd = NULL;

	if (argv != NULL) free(argv);
	if (abortflag > 0) return -1;
	else return 0;
}

BOOL WINAPI DllMain (HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad) {

	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			// The DLL is being mapped into the process's address space
			// place to allocate memory ???
			// return FALSE if this fails
			hInstanceDLL = hInstDll;		/* remember it */
			break;

		case DLL_THREAD_ATTACH:
			// A thread is being created
			break;

		case DLL_THREAD_DETACH:
			// A thread is exiting cleanly
			break;

		case DLL_PROCESS_DETACH:
			// The DLL is being unmapped from the process's address space
			// place to free any memory allocated
			// but make sure it in fact *was* allocated
			hInstanceDLL = NULL;		/* forget it */
			break;
	}
	return(TRUE);	// used only for DLL_PROCESS_ATTACH
}
#endif	// end of new stuff for DLL version

////////////////////////////////////////////////////////////////////////////

/* check IsFixedPitch both from tfm and from afm ? */

/* StartCharMetrics count of ec - bc + 1 is bogus - includes missing OK */

/* possibly try and do better with depth/height/italic to match actual OK */

/* may need to reinitialize things here for next file OK */

/* stick in string header with something useful in it ? */
/* stick characterencoding and fontid in header - xerox style ? OK  */
/* stick signature into TFM file OK */

/* read in encoding vector from file OK */

/* use tweak flag in other direction also ? */

/* according to the book: zero width index means not a valid character ? */
/* BUT: characters 54 and 55 in cmsy* and cmbsy* have zero width index ? */
/* So: need a zero width entry at position other than zero in table ! */

/* maybe improve reaction to excess of heights and depths ? OK */ 

/* problems when ligatures point off to unencoded characters ? */

/* there is a need for MAXLIG to equal MAXKERN ... ??? */

/* need a way to have the same character name be used for more */
/* than one character code OK now, I think */

/* avoid error message output on one of the two passes ? OK */

/* NOTE: kern pairs MUST be grouped on first letter */

/* truncation of kerns before selection of characters actually encoded ... */

/* typical use: afmtotfm -varu -c atandt c:\at&t\bookregu.afm */

/* typical use: afmtotfm -v -c atandt c:\at&t\bookregu.afm */

/* this is very tight - reduce redundancy to gain some space ? move to __far */

/* possibly suppress ligatures in fixed width fonts - typewriter fonts ? */

/* provide some way of restricting to 128 chars ? */

/* (xur - width) in BBox here used to encode italic correction */ /* NOT */
/* (xur - width + ITALICFUZZ) in BBox here used to encode italic correction */
/* hence BBox info should NOT be actual BBox */

/* take ligatures out of fixed width fonts ? even if in AFM ? */

/* Make up facebyte header[17] ? */
/* medium/regular bold/heavy light */
/* roman/regular italic/slanted/oblique */
/* regular condensed extended */
/* plus seven-bit-safe-flag */

/* will have to be clever to overcome 255 lig + kern limitation ! */

/* problem if ligature code found in encoding vector, but not in AFM file */

/* assumes widths are non-negative ? */

/* backwardflag should also suppress characters with k > 128 */

/* if lnext + knext > 255, use an array of fontchrs forwarding pointers */
/* the pointer for the program for character k is in position k */

/* could save some space by only having pointer for characters from
   bc to ec instead of all fontchrs */

/* Generate `automatic' ligatures for composite characters ? */
/* controlled by command line flag ? */

/* may want to compile with other than S small model in future */

/* there is NO coalescing of kern distances */
/* it is just assumed there are less than 256 distinct ones */
/* hmm, is it allowed in TeX 3.0 to have more ? */

/* this had problems with more than 255 kerns before 1991 May 21 */

/*  '*' in encoding, '-' unencoded, '.' not in encoding, ':' above 127 */

/* typical use afmtotfm -va -c atanttx c:\at&t\*.afm */

/* typical use afmtotfm -va c:\afm\adobe\po*.afm */

/* typical use afmtotfm -va -c=textext c:\afm\adobe\por*.afm */

/* typical use afmtotfm -van -c=typewrit c:\afm\adobe\com*.afm */

/* had to move TFM buffer out of local memory to make space */

/* Use 'w' flag to suppress subscript left shift */

/* Paul Anagostopolous recommendations for space parameters: */

/* spacestretch = spacewidth/2,  spaceshrink = spacewidth/6 */
/* except both zero for mono spaced */
/* extra space = em/2 - spacewidth, where em/2 = enspace */

/* For fixed width fonts use 0 for space shrink and stretch */
/* For fixed width fonts make quad equal to 2 * spacewidth */
/* For fixed width fonts make extra space equal to space */

/* NOTE: FONTINST instead sets stretch and shrink to 1/3 of space width */

/* NOTE: non italic fonts MAY have italic correction (cmex10 e.g.) */

/* use mac 5 + 3 + 3 contraction to get reasonable size FONTID ? */

/* if need more space, maybe put encoding out in hyper space ? 6.7k */ 
/* also may cut those tables of doubles to tables of float */
/* but then need comparefloat instead of comparedouble */

/* not clear that multiple encoding works exactly right */
/* see `attempt to overwrite ... ' */

/* add abilitiy to propagate kern pairs to composites ? */

/* need to compile with some optimizations or it won't fit in 64k ... */

/* allocation/freeing once per job, not once per file */

/* could allocate kern tables based in KernPairs line in AFM ... */

/* can have more than 256 distinct kern sizes in TeX 3.0 YES */

/* use `-1' on command line to disallow ANSI/TeXText extras */

/* NOTE: there may be a bug if a character with ligkern program occurs */
/* more than once in encoding - get confused lig/kern program */
/* if this is a problem, change the copymetrics(.., .., 1) to a 0 */

/* If the very first instruction of a character's |lig_kern| program has */
/* |skip_byte>128|, the program actually begins in location */
/* |256*op_byte+remainder|. This feature allows access to large |lig_kern| */
/* arrays, because the first instruction must otherwise */
/* appear in a location |<=255|. */

/* We now implement `right boundary character' ligkern entry 97/Oct/1 */
/* If the very first instruction of the |lig_kern| array has |skip_byte=255|, */
/* the |next_char| byte is the so-called right boundary character of this font; */
/* the value of |next_char| need not lie between |bc| and~|ec|.*/

/* We also implement `special left boundary character program' 97/Oct/1 */
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

/* we now check whether two characters have the same kern program */
/* if they do and neither has a lig program we make them point to */
/* the same ligkern code */ /* turn off optimization using -o */
/* we could also do this if A has the same kern program as B, but B */
/* has some ligatures, then we could point to the start of B's kern */
/* program instead of the start of B's overall ligkern program */
/* But it hardly seems worth worrying about this case */
/* example f => ff, but both have lig programs */

/* kern table need not be sorted */
/* zero entry of kern table need not be zero */
/* depth table entries can be negative (when yll > 0) */

/***************************************************************************/

/*	In DLL version, we create a modeless dialog box that can hang around */
/*	even after AFMtoTFM has returned */

/*	But this means we can't FreeLibrary in the caller upon return */
/*	For otherwise the dialog procedure for this box will be gone */
/*	We also have to watch out that we don't create more and more instances */
/*	so here we kill the previews dialog when a new one is about to be set up */

/*	One problem is that global variables in AFMtoTFM now remain set since we */
/*	don't free the library ... hmm */
/* Also make sure all is freed to avoid memory leaks */ 

/*	Alternatives: kill dialog box before return, then free library */
/*	But then user can't interact with it.

/*	Alternative: make dialog box be owned and run by DVIWindo */
/*	let it have the dialog procedure */
/*	then can use FreeLibrary without screwing up the callback function */

/***************************************************************************/
