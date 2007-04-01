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

/****************************************************************************
*																			*
* Read outline font file in HEX format										*
*																			*
* Program to draw character outlines on screen								*
* Program to extract character-level hinting information					*
* Program to interactively edit character-level hinting information			*
* Program to write character-level hinting information						*
*                                                                         	*
* Command line usage:	----see showusage---								*
*																			*
* Can handle multiple files on command line, as well as wild cards.			*
*                                                                         	*
****************************************************************************/

#include <graph.h>	/* graphics functions (16 bit only !) */
#include <stdio.h>	/* puts */
#include <conio.h>	/* getch */
#include <string.h> /* strchr */
#include <stdlib.h> /* atexit */
#include <search.h>	/* qsort */ /* also in stdlib ... */
#include <math.h>   /* sqrt */
#include <assert.h> /* assert */
#include <time.h>
#include <signal.h>

#define MAXCOMPLEX 32		/* 4 limit on complexity for centering */
#define MAXCHARNAME 32		/* maximum character name length */

/* #define VERSION "1.3" */					/* version of this program */
/* #define VERSION "1.3.1" */				/* version of this program */
#define VERSION "1.3.2"						/* version of this program */
/* #define MODDATE "1992 December 28" */	/* date of last modification */
/* #define MODDATE "1996 May 18" */			/* date of last modification */
#define MODDATE "199& May 18"				/* date of last modification */

/* #define READ_CODE "r" */		/* 1996/May/18 */
/* #define WRITE_CODE "w" */	/* 1996/May/18 */
/* #define READ_WRITE "a" */	/* 1996/May/18 */

#define READ_CODE "rb"		/* 1996/May/18 */
#define WRITE_CODE "wb"		/* 1996/May/18 */
#define READ_WRITE "ab"		/* 1996/May/18 */

/* #define MAXVGAX 640	*/				/* VGA pixel columns */
/* #define MAXVGAY 480	*/				/* VGA pixel rows */

#define RGB(r, g, b) (0x3F3F3FL & ((long)(b) << 16 | (g) << 8 | (r)))

#define CONTROLBREAK 1		/* set up handling of cntrl-break */
/* #define FANCYSTUFF 1 */	/* allow improving outlines before hinting */
/* #define EXTRASTUFF 1 */	/* */
/* #define FUDGEFLAG 1 */	/* attempt to correct for bugs in BBox */
/* #define TRACE 1 */		/* non-zero give details on tiny lineto curveto */
/* #define DEBUG 1 */		/* non-zero means careful, debugging mode */

/* ************************************************************************* */

#define CHR_ESCAPE 27

#define CHR_UPARROW			(256 + 72)
#define CHR_LEFTARROW		(256 + 75)
#define CHR_RIGHTARROW		(256 + 77)
#define CHR_DOWNARROW		(256 + 80)

#define CHR_C_LEFTARROW		(256 + 115)
#define CHR_C_RIGHTARROW	(256 + 116)
#define CHR_C_UPARROW		(256 + 141)
#define CHR_C_DOWNARROW		(256 + 145)

#define CHR_M_UPARROW		(256 + 152)
#define CHR_M_LEFTARROW		(256 + 155)
#define CHR_M_RIGHTARROW	(256 + 157)
#define CHR_M_DOWNARROW		(256 + 160)

#define CHR_M_U		(256 + 22)
#define CHR_M_H		(256 + 35)
#define CHR_M_J		(256 + 36)
#define CHR_M_N		(256 + 49)

#define CHR_F1	(256 + 59)
#define CHR_F2	(256 + 60)
#define CHR_F3	(256 + 61)
#define CHR_F4	(256 + 62)
#define CHR_F5	(256 + 63)
#define CHR_F6	(256 + 64)
#define CHR_F7	(256 + 65)
#define CHR_F8	(256 + 66)
#define CHR_F9	(256 + 67)
#define CHR_F10	(256 + 68)

#define CHR_F11 (256 + 133)		/* not in NT */
#define CHR_F12 (256 + 134)		/* not in NT */

#define CHR_C_F1	(256 + 94)
#define CHR_C_F2	(256 + 95)
#define CHR_C_F3	(256 + 96)
#define CHR_C_F4	(256 + 97)
#define CHR_C_F5	(256 + 98)
#define CHR_C_F6	(256 + 99)
#define CHR_C_F7	(256 + 100)
#define CHR_C_F8	(256 + 101)
#define CHR_C_F9	(256 + 102)
#define CHR_C_F10	(256 + 103)

#define CHR_C_F11	(256 + 137)	/* not in NT */
#define CHR_C_F12	(256 + 138)	/* not in NT */

#define CHR_M_F1	(256 + 104)
#define CHR_M_F2	(256 + 105)
#define CHR_M_F3	(256 + 106)
#define CHR_M_F4	(256 + 107)
#define CHR_M_F5	(256 + 108)
#define CHR_M_F6	(256 + 109)
#define CHR_M_F7	(256 + 110)
#define CHR_M_F8	(256 + 111)
#define CHR_M_F9	(256 + 112)
#define CHR_M_F10	(256 + 112)

#define CHR_M_F11	(256 + 139)	/* not in NT */
#define CHR_M_F12	(256 + 140)	/* not in NT */

#define CHR_PGUP	(256 + 73)
#define CHR_PGDN	(256 + 81)
#define CHR_HOME	(256 + 71)
#define CHR_END		(256 + 79)

#define CHR_INSERT	 (256 + 82)
#define CHR_DELETE	 (256 + 83)
#define CHR_C_INSERT (256 + 146)	/* not in NT */
#define CHR_C_DELETE (256 + 147)	/* not in NT */
#define CHR_M_INSERT (256 + 162)	/* not in NT */
#define CHR_M_DELETE (256 + 163)	/* not in NT */

#define CHR_C_PLUS  (256 + 144)		/* not in NT */
#define CHR_M_PLUS  (256 + 78)		/* not in NT */
#define CHR_C_MINUS (256 + 142)		/* not in NT */
#define CHR_M_0		(256 + 129)		/* not in NT */

/* ************************************************************************* */

volatile int abortflag=0; 

#define STARTSUBR 255					/* code used for start of path */

char *dosfonts="c:\\dosfonts\\*.fon";	/* fonts used in VGA mode */

long background = RGB( 8, 0, 18);
/* long background = RGB(18, 0, 28); */	/* brighter on NT */
/* long background = RGB(16, 0, 24); */	/* brighter on NT */
/* long background = RGB(12, 0, 22); */	/* brighter on NT */
/* long background = RGB(10, 0, 20); */	/* brighter on NT */
long brightred  = RGB(52, 0, 0);

/* short videomode=_MAXRESMODE; */ /* int videomode=_MAXCOLORMODE; */
short videomode = _VRES16COLOR;	/* works on NT 4.0 */ /* 640 x 480, 16 color */

/* int scolumns, srows; */
short scolumns, srows;
int numgrey, numbits;

unsigned char fillarr[8];	/* space for fill pattern */

/* structures for representing knots, strokes and stems */

struct knot {				/* structure for a knot */
/*	int x; int y; char code; unsigned char subr; */
	short x; short y; char code; unsigned char subr;
};

struct stroke {				/* structure for a stroke */
	int pos; int start; int end; int flag;
};

struct stem {				/* structure for a stem */
	int sstroke; int estroke; int value; int flag;
};

struct ratio { /* structure for rational numbers */
	long numer;	long denom;
};

struct range { /* structure for coordinate range */
	int start; int end;
};

#define MAXSTROKES 200			/* maximum number of strokes allowed */
#define MAXSTEMS 1000			/* maximum number of potential stems */
#define MAXRANGES 16			/* maximum subpath ranges */

int minseg=1;					/* 5 ? minimum stroke length to record */

int minsep=1;					/* 7 ? minimum separation between stems */

int minstroke;					/* minimum stroke for side of stem */
int minstem;					/* 20 minimum stem width allowed */
int maxstem;					/* 195 maximum stem width allowed */

/* above will need to be tuned to bold, sans serif etc */

/* thresholds and parameters */

int minnormstroke = 32;			/* 16 minimum stroke for cmr10 */	
int minnormstem = 15;			/* minimum stem width for cmr10 */
int maxnormstem = 150; 			/* 125 maximum stem for cmr10 */

/* int stem3eps = 9;	*/		/* six times allowed error vstem3 & hstem3 */ 
short stem3eps = 6;				/* six times allowed error vstem3 & hstem3 */

int nupstrokes; 			/* number of up strokes */
int ndownstrokes;			/* number of down strokes */
int numvstems;				/* number of potential vstems */

#ifdef IGNORED
unsigned int nupstrokes;			/* number of up strokes */
unsigned int ndownstrokes;			/* number of down strokes */
unsigned int numvstems;				/* number of potential vstems */
#endif

static struct stroke upstrokes[MAXSTROKES];
static struct stroke downstrokes[MAXSTROKES];
static struct stem vstems[MAXSTEMS];

int nleftstrokes;				/* number of left strokes */
int nrightstrokes;				/* number of right strokes */
int numhstems;					/* number of potential hstems */

int complainnleftstrokes;				/* complained about left strokes */
int complainnrightstrokes;				/* complained about right strokes */
int complainnupstrokes;					/* complained about up strokes */
int complainndownstrokes;				/* complained about down strokes */

static struct stroke leftstrokes[MAXSTROKES];
static struct stroke rightstrokes[MAXSTROKES];
static struct stem hstems[MAXSTEMS];

static struct range xranges[MAXRANGES];
static struct range yranges[MAXRANGES];

int nxrange, nyrange;	/* number of distinct coord ranges */

int correctscale=0;		/* non-zero => undo BSR rescaling */
int stem3flag=1;		/* try for vstem3 and hstem3 if possible */
int avoidhair=0;		/* suppress ghost stems and centering */
int keepscale=1;		/* keep input scale - do not adjust to .001 */
int ghostadjust=0;		/* set bottom ghost stems to 21, top to 20 */
int readallhints=1;		/* read all hints, including substitutions */
						/* set to zero when recording (-m) */
int enforcestart=1;		/* use specified subpath starting point */

double oscale=0.001;	/* output scale desired */

int sequenceflag=1;		/* non-zero => show characters in ordered sequence */
						/* default is to step through in sequence */
int marksflag=1;		/* non-zero => show knots on curves and lines */
int arrowflag=1;		/* non-zero => draw arrows on lineto's */
int shwhntflag=1;		/* non-zero => just show potential stem tangents */
int hintflag=0;			/* non-zero => read input hint file */
int guessflag=1;		/* attempt to suggest hints automatically */
int showhintline=0;		/* show hint output line on screen DEBUGGING */
int ignoresizes=1;		/* write stems even if too narrow or too wide */
int overlay=0;			/* suppress clear screen if non-zero */
int straighten=0;		/* show curveto's as lineto's for winding check */
int labelnode=0;		/* put numbers next to nodes */
int cumulative=1;		/* don't restart node numbers at subpath */
int showcoords=0;		/* show coordinate mode */
int currentnode=0;		/* where we are right now */
int dontredraw=0;		/* do suppress redrawing while stepping nodes */

#ifdef FANCYSTUFF
int levelflag=0;		/* non-zero => level almost horizontal lineto's */
int combineflag=0;		/* non-zero => combine almost colinear lineto's */
int flattenflag=0;		/* non-zero => flatten almost linear curveto's */
#endif

int scanmode = 0;		/* non-zero => scan through outlines without wait */
int automode = 0;		/* non-zero => as above but also don't draw them */
int batchmode = 0;		/* non-zero => do multiple files - no graphics */

/* batchmode => automode => scanmode */

int ghostoff = 20;		/* 20 or 21 - Adobe mandated ghost stem width */
int minarrow = 16;		/* 32 shortest lineto on which to show arrow */ 

int flexmax = 10;		/* 20 maximum bow allowed in flex */
int flexmin = 80;		/* minimum allowed length of bow in flex */

int hintnumshow=1;		/* show stem coordinates on screen */
int stemnumshow=1;		/* show stem width - if found */
int strokeflag=1;		/* collect stroke information for hinting */
int recordflag=0;		/* write stem hinting output file */
int fuzzflag=0;			/* non-zero => allow error in hint stem position */

/* int stepscale=1; */		/* scale for stepping in response to arrow keys */
short stepscale=1;		/* scale for stepping in response to arrow keys */

int flexflag = 1;		/* insert flex strokes as potential stem edges */
int showwidth = 1;		/* insert 0 and width as potential stem edges */
int hvscale = 0; 		/* use only horizontal for screen scale if positive */
						/* use only vertical for screen scale if negative */

int showslanted=1;		/* show vstrokes even for italic and slanted */
int extremaflag=1;		/* insert extrema lines as potential stem edges */

int smallfont=1;		/* use small font for numbers */
int largefont=0;		/* use large font for font name in top left */

int charflag=0;			/* non-zero means next arg starting character */
int extenflag=0;		/* non-zero means next arg extension for output */
int hintpathflag=0;		/* non-zero means next arg is path for hint file */
int zoomflag=0;			/* non-zero means next arg is zoom factor */
int hintcolorflag=0;	/* non-zero means next arg is hint color number */
int videoflag=0;		/* non-zero means next arg is video mode */

int node;				/* current node */

double zoomfactor=1.0;	/* extra initial scale factor to apply */

int suppresskip=1;		/* non-zero => suppress forward skip if recording */

char *ext = "stm";		/* default extension for output */

int italicflag=0;		/* no vstems for italic and slanted fonts */
int mathitalic=0;		/* math italic fonts - special case */
int mathsymbol=0;		/* math symbol fonts - special case */
int mathextended=0;		/* math extended font - special case */
int boldflag=0;			/* bold font */
int sansflag=0;			/* sans serif font */
int typeflag=0;			/* typewriter font */

/* sizes, color, line styles for graphics shown on screen come next */

/* int tickeps=3; */			/* size of tick mark */
short tickeps=3;			/* size of tick mark */
/* int tickcol=15; */			/* color index to use for tickmarks */
short tickcol=15;			/* color index to use for tickmarks */
/* int ticksty=0XFFFF;	 */	/* tickmark style */
unsigned short ticksty=0XFFFF;		/* tickmark style */

/* int arroweps=4; */			/* size of arrow mark */
short arroweps=4;			/* size of arrow mark */
/* int arrowcol=15; */		/* color index to use for arrowmarks */
short arrowcol=15;		/* color index to use for arrowmarks */
/* int arrowsty=0XFFFF;	 */ /* arrowmark style */
unsigned short arrowsty=0XFFFF;	/* arrowmark style */

/* int squareeps=2; */		/* size of square mark */
short squareeps=2;		/* size of square mark */
/* int squarecol=9; */		/* color index to use for squares */
short squarecol=9;		/* color index to use for squares */
/* int squaresty=0XFFFF;	 */ /* square style */
unsigned short squaresty=0XFFFF;	/* square style */

/* int diamondeps=3; */		/* size of diamond mark */
short diamondeps=3;		/* size of diamond mark */
/* int diamondcol=11; */		/* color index to use for diamonds */
short diamondcol=11;		/* color index to use for diamonds */
/* int diamondsty=0XFFFF;	 */ /* diamond style */
unsigned short diamondsty=0XFFFF;	/* diamond style */

/* int circleeps=4; */		/* size of circle mark */
short circleeps=4;		/* size of circle mark */
/* int circlecol=12; */		/* color index to use for circles */
short circlecol=12;		/* color index to use for circles */
/* int circlesty=0XFFFF;	 */ /* circle style */
unsigned short circlesty=0XFFFF;	/* circle style */

/* int diagoneps=8; */		/* size of diagonal cross mark */
short diagoneps=8;		/* size of diagonal cross mark */
/* int diagoncol=13; */		/* color index to use for diagonals */
short diagoncol=13;		/* color index to use for diagonals */
/* int diagonsty=0XFFFF;	 */ /* diagonal style */
unsigned short diagonsty=0XFFFF;	/* diagonal style */

/* int stemrightcol = 4; 	 */
short stemrightcol = 4; 	
/* int stemleftcol = 2; 	 */
short stemleftcol = 2; 	
/* int cursorrightcol = 12; */
short cursorrightcol = 12;
/* int cursorleftcol = 10; */
short cursorleftcol = 10;

/* int lstemsty=0X1010; */
unsigned short lstemsty=0X1010;
/* int rstemsty=0X0303; */
unsigned short rstemsty=0X0303;

/* int nodecol=14; */			/* color for present node label */
short nodecol=14;			/* color for present node label */

/* make setabble --- 1992/Dec/1 */
/* int hintcol = 8; */		/* 7 ? color for unselected stem numbers */
short hintcol = 8;		/* 7 ? color for unselected stem numbers */

/* int selectrightcol = 12;  */
short selectrightcol = 12; 
/* int selectleftcol = 10; */
short selectleftcol = 10;
/* int stemwidthcol=7; */		/* color for stem width numbers */
short stemwidthcol=7;		/* color for stem width numbers */

/* int textcol = 7; */		/* color for text in top left corner */
short textcol = 7;		/* color for text in top left corner */
/* int outlinecol = 14; */	/* color for outline itself */
short outlinecol = 14;	/* color for outline itself */

/* int basecol=13; */		/* color index to use for baseline */
short basecol=13;		/* color index to use for baseline */
/* int basesty=0xF0F0;	 */ /* baseline style */
unsigned short basesty=0xF0F0;	/* baseline style */

/* int bodycol=6; */		/* color used for fill of body NOT USED */
short bodycol=6;		/* color used for fill of body NOT USED */

FILE *fp_in, *fp_out, *fp_hnt; 

#define INFINITY 8191		/*  4095 number larger than valid coordinate */

int cursorx=-INFINITY;	/* position of selection cursor */
int upflag=1;			/* on up stroke if coincident */

int cursory=-INFINITY;	/* position of selection cursor */
int leftflag=1;			/* on left stroke if coincident */

/* int interx, intery; */		/*	 interior point (on screen) NOT USED */

/* int xold, yold, xstart, ystart, kstart; */
short xold, yold, xstart, ystart;
int kstart;

int reverseflag=0;		/* non-zero means reverse path */

int complaindouble=0;	/* complain for double knots, not just triples */
int complaintriple=0;	/* complain for triple knots */

/* #define FNAMELEN 80			/* maximum file name length */ */
#define FONTNAME_MAX 64
#define MAXLINE 512			/* 128 maximum line length assumed in input */

#define MAXCHRS 256			/* 128 Number of characters defined in fonts */

#define CURSORINC 10		/* motion of screen origin */

/* #define MAXKNOTS 175 */
/* Longest subpath in ComputerModern fonts only has 148 knots */
/* #define MAXKNOTS 350 */		/* maximum number of knots in subpath */
#define MAXKNOTS 400		/* maximum number of knots in subpath */

int verboseflag = 0; 		/* forced to zero when standard output used */
int traceflag = 0;			/* lots of detailed tracing output */
int detailflag = 0;
int quietflag = 0;			/* ignore stroke direction error */
int showstateflag = 0;		/* 1996/June/16 */

int showadjust = 0;			/* show adjustments made to stems */

int fontchrs = MAXCHRS;

/* The following may not really need to be long */

long numerscale;				/* numerator of scale factor */
long denomscale;				/* denominator of scale factor */

double rawnumer, rawdenom;		/* raw numer and denom in font matrix */

static char line[MAXLINE];		/* general scratch area for  strings */

static struct knot knots[MAXKNOTS];		/* place for storing knots */

#define MAXWIDTH 1600 			/* large for cmbx<n> */
#define MAXHEIGHT 3800			/* large for cmex10 */

long fileposition[MAXCHRS];		/* position of start of character */

static char fontname[FONTNAME_MAX];	/* place to save fontname */

int ptsize;						/* point size as determined from fontname */

int knot;						/* index into arrays for next knot */
/* int xold, yold; */			/* previous knot coordinates remembered */
int maxknts, maxkchrs;			/* maximum of above, character it occured */

int doubleknots;				/* count where two knots in same place */
int tripleknots;				/* count where three knots in same place */

/* int xll, yll, xur, yur; */	/* given bounding box - from FontBBox input */
short xll, yll, xur, yur;  		/* given bounding box - from FontBBox input */
/* int sxll, syll, sxur, syur; */	/* scaled versions - for FontBBox output */
short sxll, syll, sxur, syur;		/* scaled versions - for FontBBox output */

int xmsn, ymsn, xmsx, ymsx; 	/* extremes of x and y (for one subpath) */
int xmin, ymin, xmax, ymax;		/* extremes of x and y (for one char) */

int checkflag=0;				/* set by coincident knots in checkknots */
int iwidth;						/* width of current character */

long fstart=-1;					/* pointer to after header in font file */
long flast=-1;
long hstart=-1;					/* pointer to after header in hint file */
long hlast=-1;

int suppressflag=0;				/* draw strokes in background color */

/* char *path;	*/				/* path for input hint file to read from */

char hintpath[FILENAME_MAX]="";		/* path for input hint file to read from */

int chrs;						/* character code working on currently */	

char *task;  					/* current activity - for error message */

int selectnumber = -1;			/* which set of hints or paths to be used */
								/* 0 is base, 1 is replacement 1 etc */
								/* -1 means show all paths / hints */
int alwaysknots = -1;			/* always show knots even if path not */

int selectpathflag = 0;			/* non-zero => select paths, not hint sets */

int nsubr;						/* current subr position for this character */

int npath;						/* current path number for this character */

int fsubr=0;			/* which hint switching set are we in ? */
						/* zero before first hint switch */

int subroffset=0;		/* non-zero when inserted extra hint switching */

int oldchrs=0;			/* char number for last hint line analyzed */

/* some simple macros */

#define ABS(x) (((x) < 0) ? (-x) : (x))
#define MAX(x, y) (((x) < (y)) ? (y) : (x))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int getsafe(void) {		/* gobble character - even if two bytes */
	int k;
	if ((k = _getch()) != 0) return k;
	else return 256 + _getch();
}

void giveup(int code) { 	/* graceful exit with meaningful error message */
	fprintf(stderr, " while %s", task);
	if (chrs >= 0) fprintf(stderr, " for character %d ", chrs);
	else fprintf(stderr, " ");
	if (batchmode == 0) getsafe();		/* ? */
	_setvideomode(_TEXTC80);			/* reset video mode to 80 x 25 */
	exit(code);
}

#ifdef IGNORE
void extension(char *fname, char *ext) { /* supply extension if not present */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
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

int ngetc(FILE *fp_in) { /* safely get character - check for user interrupt */
	int c;

	if ((c = getc(fp_in)) == EOF) {
		fprintf(stderr, "\nUnexpected EOF");  giveup(4);
	}
	if (c <= 32) { /* cut down on checking of key types */
/*		if (kbhit() && getsafe() == '\035' ) {
			fprintf(stderr, "\nUser interrupt");  giveup(5); 
		} */
	}
	return c;
}

int ogetc(FILE *fp_in) { /* get character - ignore cntrl char and space */
	int c;

	while ((c = getc(fp_in)) <= ' ' && c != EOF)	{
/*		if (kbhit() && getsafe() == '\035' ) { 		
			fprintf(stderr, "\nUser interrupt"); giveup(5); 
		} */
	}

	if (c == EOF) {
		fprintf(stderr, "\nUnexpected EOF"); giveup(4);
	}
	return c;
}

/* int gobblenumber(FILE *fp_in) { */ /* read "coded" decimal number */
short gobblenumber(FILE *fp_in) { /* read "coded" decimal number */
/*	int num=0; */
	short num=0;
	int sgn=1, c;

	c = ogetc(fp_in);
	if (c == 'A') c = ogetc(fp_in); /* ignore initial 'A' (blank) */
	if (c == 'B') { 				/* 'B' for minus */
		sgn = -1; c = ogetc(fp_in);
	}
	while (c >= '0' && c <= '9') { /* like atoi */
		num = (short) (10 * num + (c - '0'));
        c = ogetc(fp_in);
	}
	ungetc(c, fp_in);				/* unread terminating char */
	return (sgn < 0) ? -num : num;
}

/* int scalenum (int num) { */ /* scale and round integer - positive or negative */
short scalenum (short num) { /* scale and round integer - positive or negative */
	long res;
#ifdef DEBUG
	if (denomscale == 0) {
		fprintf(stderr, "\nDenomscale = 0"); giveup(7);
	}
#endif
		
	if (num < 0) return ( - scalenum( - num));
	else {
		res = (numerscale * num * 2 + denomscale) / (denomscale * 2);
/*		return (int) res; */
		return (short) res;
	}
/* possible problem if result does not fit into int */
}

void checkbbox (void) { 	/* check if need to interchange yll and xur */
							/* this will not catch all cases of interchange */
/*	int tmp; */
	short tmp;
	
	if (xll > xur || yur < yll) {
		if (verboseflag != 0)
			printf( "\nFontBBox flaw: reversing xur and yll ");
		tmp = yll; yll = xur; xur = tmp;
	}
}

/* compute good rational approximation to floating point number */
/* assumes number given is positive */

struct ratio rational(double x, long nlimit, long dlimit) {
	long p0=0, q0=1, p1=1, q1=0, p2, q2;
	double s=x, ds;
	struct ratio res;
/*	printf("Entering rational %lg %ld %ld\n", x, nlimit, dlimit);  */
	
	if (x == 0.0) {
			res.numer = 0; res.denom = 1;
			return res;		/* the answer is 0/1 */
	}
	for(;;) {
		p2 = p0 + (long) s * p1;
		q2 = q0 + (long) s * q1;
/*		printf("%ld %ld %ld %ld %ld %ld %lg\n", p0, q0, p1, q1, p2, q2, s);  */
		if (p2 > nlimit || q2 > dlimit || p2 < 0 || q2 <= 0) {
			p2 = p1; q2 = q1; break;
		}
		if ((double) p2 / (double) q2 == x) break;
		ds = s - (double) ((long) s);
		if (ds == 0.0) break;
	
		p0 = p1; q0 = q1; p1 = p2; q1 = q2;	s = 1/ds;
	}
	res.numer = p2; res.denom = q2;
	return res;		/* the answer is p2/q2 */
}

/*
long gcd(long a, long b) {
	if (b > a) return gcd(b, a);
	else if (b == 0) return a;
	else return gcd (b, a % b);
} */

/* for scale, expect:  5pt => 9/25,  7pt => 9/35,  8pt => 9/40, */
/*					  10pt => 9/50, 12pt => 3/20, 17pt => 5/48  */
/* unless we are compensting for BSR scaling */

/* scale factor ignored if ignorescale is non-zero */

void setscale(double a, double b) { 	/* set up scale factor */
	double as, bs;
	struct ratio rscale;

/* 	a = a * 1000; */
	as =  a / oscale; /* to get standard 0.001 scale in FontMatrix */
	bs = b;
/*	if (correctscale != 0) { as = as * 803; bs = bs * 800; } */
	if (correctscale != 0) { as = as * 72.27; bs = bs * 72.0; }
/*	gs = gcd(as, bs); */
	rscale = rational(as/bs, 2000000l, 2000000l);
/*	numerscale = as / gs; denomscale = bs / gs; */
	numerscale = rscale.numer; denomscale = rscale.denom;

	if (verboseflag != 0)
		printf( "Using Scale Factor %ld/%ld \n", numerscale, denomscale);
	
	sxll = scalenum(xll);
	syll = scalenum(yll);
	sxur = scalenum(xur);
	syur = scalenum(yur);
#ifdef FUDGEFLAG
	sxur = sxur + 5;			/* attempt to correct for errors */
	syur = syur + 7;			/* attempt to correct for errors */
#endif	

/* NOTE: this is claimed to be a problem, but it is apparently not */

	if (verboseflag != 0) {
		if (sxll <= - 2000) {
			fprintf(stdout, "Scaled xll = %d < -2000 \n", sxll);
		}
		if (syll <= - 2000) {
			fprintf(stdout, "Scaled yll = %d < -2000 \n", syll);
		}
		if (sxur >=  2000) {
			fprintf(stdout, "Scaled xll = %d > 2000 \n", sxll);
		}
		if (syur >=  2000) {
			fprintf(stdout, "Scaled yll = %d > 2000 \n", syll); 
		}
	}
/* changes scales AND FontMatrix when the following happens => cmex10 ? */
/*	if (whimpflag != 0 && 
		(sxll <= -2000 || syll < -2000 || sxur > 2000 || syur > 2000)) {
		oscale = oscale * 1.5;
		setscale(a, b);
	} */
}

void getline(FILE *fp_in, char *line) {	/* read a line up to newline */
	if (fgets(line, MAXLINE, fp_in) == NULL) {
		fprintf(stderr, "\nUnexpected EOF"); giveup(4);
	}
/*	return strlen(line); */
}

void lowercase(char *s) { /* convert to lower case letters */
	int c;
	while ((c = *s) != '\0') {
		if (c >= 'A' && c <= 'Z') *s = (char) (c + 'a' - 'A');
		s++;
	}
}

void analyzename(char *fontname) { /* get information on CM font from name */

	int ptsized;
	char *s;

	italicflag = 0; /* suppress vstems for italic and slanted fonts */
	mathitalic = 0;
	mathsymbol = 0;
	boldflag = 0; 	/* allow wider stems in bold font */
	sansflag = 0;
	typeflag = 0;
	ptsize = 10;	/* try to get ptsize from font name */

	lowercase(fontname);
	if (strstr(fontname, "cm") != NULL) {
		if (strstr(fontname, "sl") != NULL) italicflag = 1;
		if (strstr(fontname, "i") != NULL) italicflag = 1;
		if (strstr(fontname, "inch") != NULL) italicflag = 0; /* cminch */
		if (strstr(fontname, "mmi") != NULL) {mathitalic = 1; italicflag = 1;}
		if (strstr(fontname, "sy") != NULL) {mathsymbol = 1; italicflag = 1;}
		if (strstr(fontname, "ex") != NULL) mathextended = 1;
		if((s = strpbrk(fontname, "0123456789")) != NULL) 
			sscanf(s, "%d", &ptsize);
		if (strstr(fontname, "b") != NULL) boldflag = 1;
		if (strstr(fontname, "inch") != NULL) boldflag = 1; /* cminch */
		if (strstr(fontname, "ss") != NULL) sansflag = 1;
		if ((strstr(fontname, "tt") != NULL &&
			strstr(fontname, "vtt") == NULL) ||
				strstr(fontname, "tex") != NULL ||
					strstr(fontname, "tcsc") != NULL) typeflag = 1;
	}
	else {
		if (strstr(fontname, "bold") != NULL) boldflag = 1;
		if (strstr(fontname, "italic") != NULL) italicflag = 1;
	}
	
	switch (ptsize) {
		case 5: ptsized= 6; break;
		case 7: ptsized= 8; break;			
		case 10: ptsized = 10; break; 
		case 12: ptsized = 12; break;
		case 17: ptsized = 15; break;
		default: ptsized = 10; break;
	}

/* adjust various threshold to suit this font */
	
	minstroke = (minnormstroke * 10) / ptsized;
	minstem = (minnormstem * 10) / ptsized;	
	maxstem = (maxnormstem * 10) / ptsized; 

	if (boldflag != 0) {
		minstroke = (int) ((double) minstroke * 1.5);
		minstem = (int) ((double) minstem * 1.5);
		maxstem = (int) ((double) maxstem * 1.5);
	}
	else if (sansflag != 0 || typeflag != 0) {
		minstroke = (int) ((double) minstroke * 1.25);
		minstem = (int) ((double) minstem * 1.25);
		maxstem = (int) ((double) maxstem * 1.25);
	}
}

/* void removecr(char *s) {
	while(*s != '\n' && *s != '\0') s++;
	if (*s == '\n') *s = '\0';
} */

void doheader(FILE *fp_in) { /* deal with header */
	char *ptr, *cpt;
	int n;
	struct ratio rscale;
	
#ifdef DEBUG
/*	if (verboseflag != 0) printf( "Working on header\n"); */
#endif

	task = "looking for copyright or font name";
	
	getline(fp_in, line);	/* search for Copyright line */
	while (*line == '%' ||
		(strstr(line, "Copyright") == NULL &&
		 strstr(line, "dict") == NULL)) { /* "/cm" */
		getline(fp_in, line);
	}

	task = "looking for font name";

/*	getline(fp_in, line); */
	while (*line == '%' ||
		strstr(line, "dict") == NULL) { /* look for font name */
		getline(fp_in, line);
	}
	ptr = strchr(line, '/');
	n = strchr(ptr, ' ') - (ptr + 1);
	if (n > 32) { /* longest font name ? */
		fprintf(stderr, "\nCan't find fontname in %s", ptr); giveup(7);
	}
	strncpy(fontname, (ptr + 1), n);	/* copy the font name */
	fontname[n] = '\0';

	analyzename(fontname);

	task = "looking for FontMatrix and FontBBox";

	rawdenom = 0.0; rawnumer = 0.0;
	xll = 0; yll = 0; xur = 0; yur = 0;

	getline(fp_in, line);
	while ((ptr = strstr(line, "/CharDefs get")) == NULL) {
		if ((ptr = strstr(line, "/FontMatrix")) != NULL) {
			if (((cpt = strchr(line, '%')) == NULL) || cpt > ptr) {
				if(sscanf (ptr, "/FontMatrix [%lg %lg", 
					&rawnumer, &rawdenom) != 2) {
					fprintf(stderr, "Don't understand FontMatrix: %s", line);
					giveup(4);
				}
				if (rawdenom == 0.0) {
/*					removecr(line); */
/*					fprintf(stderr, "Non-standard FontMatrix: %s ", line); */
/*					correctscale = 0; */ /* needed even for `rustic' format */
					rscale = rational(rawnumer, 2000000l, 2000000l); 
					rawnumer = (double) rscale.numer;
					rawdenom = (double) rscale.denom;
/*					fprintf(stderr, "%lg/%lg \n", rawnumer, rawdenom); */
				}
/*				printf("FontMatrix line: %s\n", line); */
			}
		}
		if ((ptr = strstr(line, "/FontBBox")) != NULL) {
			if (((cpt = strchr(line, '%')) == NULL) || cpt > ptr) {
				if (sscanf(ptr, "/FontBBox [%d %d %d %d]",
					&xll, &yll, &xur, &yur) != 4) {
					if (sscanf(ptr, "/FontBBox {%d %d %d %d}",
						&xll, &yll, &xur, &yur) != 4) {
						fprintf(stderr, "Don't understand FontBBox: %s", 
							line);
						giveup(3);
					}
				}
/*				xur = xur + 20; */
/*				yur = yur + 50; */
			}
		}
		getline(fp_in, line); 
	}

	if (rawnumer == 0.0 || rawdenom == 0.0) {
		fprintf(stderr, "Did not find FontMatrix"); giveup(7);
	}

	if (xll == xur && yll == yur) {
		fprintf(stderr, "Did not find FontBBox");
/*		giveup(7); */	/* use some random default instead */
		xll = -50; yll = -250; xur = 1000; yur = 1000;
	}

	checkbbox();		/* check whether yll and xur reversed */
	if (keepscale == 0)
		setscale(rawnumer, rawdenom);		/* compute scale factor */
	else
		setscale(oscale, 1.0);				/* pretend it was .001 */

	if (verboseflag != 0) 
		printf( "FontBBox [%d %d %d %d]\n",	sxll, syll, sxur, syur);

#ifdef DEBUG
/*	if (verboseflag != 0) printf( "Finished with header\n");  */
#endif
}

/* scan to character level hints - copying to output if desired */
/* this will already skip over hint replacement lines also */

void scantochar(FILE *fp_hnt, FILE *fp_out) { 
	int c;
	c = getc(fp_hnt);
	if (c == 'C') {
		ungetc(c, fp_hnt); 
		return;
	}
	if (recordflag != 0) putc(c, fp_out);
	for(;;) {
		while (c != '\n' && c != EOF) {
			c = getc(fp_hnt);
			if (recordflag != 0) putc(c, fp_out);
		}
		if (c == EOF) {
			fprintf(stderr, "Unexpected EOF in hint file\n");
			break;
		}
		c = getc(fp_hnt);
		if (c == 'C') {
			ungetc(c, fp_hnt); 
			break;
		}
		if (recordflag != 0) putc(c, fp_out);
	}
}

/* int xoffset, yoffset; */
short xoffset, yoffset;
/* int scrscale; */
short scrscale;

/* int xorgset, yorgset; */
short xorgset, yorgset;
/* int orgscale; */
short orgscale;

#define SCALESHIFT 4

/* scale used for display on screen is (scrscale / 2^SCALESHIFT) */

/* int xmap(int x) { 
	return (xoffset + (int) (((long) x * scrscale) >> SCALESHIFT)); 
} */

short xmap(int x) { 
	return (xoffset + (short) (((long) x * scrscale) >> SCALESHIFT)); 
} 

/* int ymap(int y) {
	return (yoffset - (int) (((long) y * scrscale) >> SCALESHIFT)); 
} */

short ymap(int y) {
	return (yoffset - (short) (((long) y * scrscale) >> SCALESHIFT)); 
} 

void setscreenscale(void) {
	int dx, dy;
	double xscale, yscale, scale;

	dx = sxur - sxll; dy = syur - syll;
	assert(dx != 0); assert(dy != 0);
	xscale = (double) (scolumns - 30) / (double) dx;
	yscale = (double) (srows - 30) / (double) dy;
	if (hvscale > 0) scale = xscale;
	else if (hvscale < 0) scale = yscale;
	else {
		if (xscale > yscale) scale = yscale;
		else scale = xscale;
	}
	if (zoomfactor != 1.0) scale = scale * zoomfactor; /* 1992/Sep/12 */
/*	scrscale = (int) (scale * (double) (1 << SCALESHIFT)); */
	scrscale = (short) (scale * (double) (1 << SCALESHIFT));
	printf("dx %d dy %d  xscl %lg yscl %lg  scl %lg scrscl %d/%d\n",
		dx, dy, xscale, yscale, scale, scrscale, (1 << SCALESHIFT)); 
/*	xoffset = 0; yoffset = 0; */
	xoffset = scolumns/2 - xmap((sxll + sxur)/2);
	yoffset = srows/2 - ymap((syll + syur)/2);
	orgscale = scrscale;		/* remember it for reset later by M-N-- */
	xorgset = xoffset;	yorgset = yoffset;
}

/* show potential stem edges on screen */

void hstemright(int y, int hghlit, int flag) {
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;

	cinx = _getcolor();
	lsty = _getlinestyle();
	if (cursory == y) _setcolor(cursorrightcol);
	else _setcolor(stemrightcol);
	if (suppressflag != 0) _setcolor(0);
	_setlinestyle(rstemsty);
	_moveto (20, ymap(y));
	_lineto (scolumns, ymap(y)); /* -20 */
	if (hintnumshow != 0) {
		if ((hghlit == 0 && flag == 0) || 
			(hghlit != 0 && flag != 0)) {
			if (flag != 0) 	_setcolor(selectrightcol);  
			else _setcolor(hintcol);
			if (suppressflag != 0) _setcolor(0);
			_moveto (0, (short) (ymap(y) - 5));	/*  half of letter height */
			sprintf(line, "%d", y);
			_outgtext(line);
			}
	}
	_setcolor(cinx); _setlinestyle(lsty);
}

void hstemleft(int y, int hghlit, int flag) {
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;

	cinx = _getcolor();
	lsty = _getlinestyle();
	if (cursory == y)  _setcolor(cursorleftcol);
	else _setcolor(stemleftcol);
	if (suppressflag != 0) _setcolor(0);
	_setlinestyle(lstemsty);
	_moveto (20, ymap(y));
	_lineto (scolumns, ymap(y)); /* -20 */
	if (hintnumshow != 0) {
		if ((hghlit == 0 && flag == 0) ||
			(hghlit != 0 && flag != 0)) {
		if (flag != 0) _setcolor(selectleftcol); 
		else _setcolor(hintcol);
		if (suppressflag != 0) _setcolor(0);
		_moveto (0, (short) (ymap(y) - 5)); /* to account for half of letter height */
		sprintf(line, "%d", y);
		_outgtext(line);
		}
	}
	_setcolor(cinx); _setlinestyle(lsty);
}

void hstemwidth(int yl, int yr, int flag) {
	int ya, dy;
	dy = yr - yl;	ya = (yl + yr)/2; 
	if (stemnumshow != 0) {
		if (flag == 0) _setcolor(0); 		/* background */
		else _setcolor(stemwidthcol);  
		_moveto ((short) (scolumns - 20), (short) (ymap(ya) - 5));
		sprintf(line, "%d", dy);
		_outgtext(line);
	}
}

void vstemup(int x, int hghlit, int flag) { /* show vertical stem edge going up */
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;

	cinx = _getcolor();
	lsty = _getlinestyle();
	if (cursorx == x)  _setcolor(cursorleftcol);
	else _setcolor(stemleftcol);
	_setlinestyle(lstemsty);
	if (suppressflag != 0) _setcolor(0);
	_moveto (xmap(x), (short) (srows - 20)); 
	_lineto (xmap(x), 0);			/* 20 */
	if (hintnumshow != 0) {
		if ((hghlit == 0 && flag == 0) ||
			(hghlit != 0 && flag != 0)) {
			if (flag != 0) _setcolor(selectleftcol); 
			else _setcolor(hintcol);
			if (suppressflag != 0) _setcolor(0);
			_moveto ((short) (xmap(x) - 5), srows); /*  for half of letter height */
			sprintf(line, "%d", x);
			_setgtextvector(0,1); _outgtext(line); _setgtextvector(1,0);
		}
	}
	_setcolor(cinx); _setlinestyle(lsty);
}

void vstemdown(int x, int hghlit, int flag) { /* show vertical stem edge going down */
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;

	cinx = _getcolor();
	lsty = _getlinestyle();
	if (cursorx == x)  _setcolor(cursorrightcol);
	else _setcolor(stemrightcol);
	if (suppressflag != 0) _setcolor(0);
	_setlinestyle(rstemsty);
	_moveto (xmap(x), (short) (srows - 20));
	_lineto (xmap(x), 0);		/* 20 */
	if (hintnumshow != 0) {
		if ((hghlit == 0 && flag == 0) ||
			(hghlit != 0 && flag != 0)) {
			if (flag != 0) _setcolor(selectrightcol); 
			else _setcolor(hintcol);
			if (suppressflag != 0) _setcolor(0);
			_moveto ((short) (xmap(x) - 5), srows); /* for half of letter height */
			sprintf(line, "%d", x);
			_setgtextvector(0,1); _outgtext(line); _setgtextvector(1,0); 
		}
	}
	_setcolor(cinx); _setlinestyle(lsty);
}

void vstemwidth(int xu, int xd, int flag) {
	int xa, dx;
	dx = xd - xu;	xa = (xu + xd)/2; 
	if (stemnumshow != 0) {
		if (flag == 0) _setcolor(0); 		/* background */
		else _setcolor(stemwidthcol);  
		_moveto ((short) (xmap(xa) - 5), 20);
		sprintf(line, "%d", dx);
		_setgtextvector(0,1); _outgtext(line); _setgtextvector(1,0); 
	}
}

/* insert a new coordinate range into table */

void insertxrange(int xs, int xe) {
	int k, l;

/*	printf("XS %d XE %d ", xs, xe); */

	for (k=0; k < nxrange; k++) {
		if (xranges[k].end < xs) continue;
		if (xranges[k].start > xe) {	/* no overlap - insert new */
			for (l = nxrange - 1; l >= k; l--) xranges[l+1] = xranges[l];
			xranges[k].start = xs; 	xranges[k].end = xe;
			if(nxrange++ > MAXRANGES) {
				fprintf(stderr, "Too many x ranges\n");
				giveup(12);
			}
			return;
		}
		xs = MIN(xranges[k].start, xs);
		xe = MAX(xranges[k].end, xe);
		xranges[k].start = xs;
		xranges[k].end = xe;
		while (k+1 < nxrange && xranges[k+1].start <= xe) {
			xs = MIN(xranges[k+1].start, xs);
			xe = MAX(xranges[k+1].end, xe);
			for (l = k+1; l < nxrange; l++) xranges[l-1] = xranges[l];
			xranges[k].start = xs;
			xranges[k].end = xe;
			nxrange--;
			assert(nxrange > 0);
		}
		return;		
	}
	xranges[nxrange].start = xs; 	xranges[nxrange].end = xe;
	if(nxrange++ > MAXRANGES) {
		fprintf(stderr, "Too many x ranges\n");
		giveup(12);
	}
	return;
}

void insertyrange(int ys, int ye) {
	int k, l;

/*	printf("YS %d YE %d ", ys, ye); */
	
	for (k=0; k < nyrange; k++) {
		if (yranges[k].end < ys) continue;
		if (yranges[k].start > ye) {	/* no overlap - insert new */
			for (l = nyrange - 1; l >= k; l--) yranges[l+1] = yranges[l];
			yranges[k].start = ys; 	yranges[k].end = ye;
			if(nyrange++ > MAXRANGES) {
				fprintf(stderr, "Too many y ranges\n");
				giveup(12);
			}
			return;
		}
		ys = MIN(yranges[k].start, ys);
		ye = MAX(yranges[k].end, ye);
		yranges[k].start = ys;
		yranges[k].end = ye;
		while (k+1 < nyrange && yranges[k+1].start <= ye) {
			ys = MIN(yranges[k+1].start, ys);
			ye = MAX(yranges[k+1].end, ye);
			for (l = k+1; l < nyrange; l++) yranges[l-1] = yranges[l];
			yranges[k].start = ys;
			yranges[k].end = ye;
			nyrange--;
			assert(nxrange > 0);
		}
		return;		
	}
	yranges[nyrange].start = ys; 	yranges[nyrange].end = ye;
	if(nyrange++ > MAXRANGES) {
		fprintf(stderr, "Too many y ranges\n");
		giveup(12);
	}
	return;
}

void endchar(void) {
/*	int cinx; */
	short cinx;

	cinx = _getcolor();
	_setcolor(bodycol);
	_setfillmask(fillarr);
/*	if (marksflag == 0) _floodfill(interx, intery, cinx); */
	_setcolor(cinx);
}

/* Now returns start position of subpath if specified 1992/Sep/06 */

int checkarr(int n) { 		/* update x and y - min and max */
	int k, x, y;
	int flag = 0, korigin = 0;
	
/*	get min and max for subpath */
	xmsn = INFINITY; ymsn = INFINITY; xmsx = -INFINITY; ymsx = -INFINITY; 
	
	for (k = 0; k < n; k++) {
		if (knots[k].subr == STARTSUBR) korigin = k;	/* 1992/Sep/6 */
		x = knots[k].x; 	y = knots[k].y;
		if (x < xmsn) xmsn = x;		if (x > xmsx) xmsx = x;
		if (y < ymsn) ymsn = y;		if (y > ymsx) ymsx = y;
/*		if (x > 2000 || x < -2000 || y > 2000 || y < -2000)	{
			flag = -1;
		} */
	}
	if (xmsx > 2000 || xmsn < -2000 || ymsx > 2000 || ymsn < -2000)	{
/*		printf("Coord (%d, %d) in char %d\n", x, y, chrs); */
		flag = -1;
/*		printf("Coord (%d, %d) in char %d\n", x, y, chrs); */
	} 
/* update min and max for whole character */
	if (xmsn < xmin) xmin = xmsn;	if (xmsx > xmax) xmax = xmsx;
	if (ymsn < ymin) ymin = ymsn;	if (ymsx > ymax) ymax = ymsx;
	if (flag != 0 && verboseflag != 0) {
/*		printf("Coordinates may be out of range in char %d\n", chrs); */
/*		apparently, this is not a problem after all ... */
	}
	insertxrange(xmsn, xmsx);
	insertyrange(ymsn, ymsx);
/*	if (korigin != 0) printf("Path starts at k = %d\n", korigin);  */
	return korigin;
}

void showarr(int n, int m) {		/* debugging output */
	int k;

	if (n < 0) n = 0;
	if (m > knot) m = knot;
	putc('\n', stderr);
	for (k = n; k <= m; k++)
		fprintf(stderr, "%d:(%d,%d) %c ", k, 
			knots[k].x, knots[k].y, knots[k].code);
}

void resetstrokes() {			/* flush old stroke information */
	nupstrokes = 0; ndownstrokes = 0;
	nleftstrokes = 0; nrightstrokes = 0;
	complainnupstrokes = 0; complainndownstrokes = 0;	/* 1994/June/2 */
	complainnleftstrokes = 0; complainnrightstrokes = 0;
	cursorx = -INFINITY; cursory = -INFINITY;
}

/* Following is mostly silly - suppressed by showslanted != 0 */

/* no vstems for italic and slanted fonts ? */
/* but math italic numerals are not italic - cmmi<n> 40 -- 63 */
/* and symbol font calligraphic symbols are italic - cmsy<n> 64 -- 90 ? */

int italiccharsub (void) {
	if (italicflag == 0) return 0;	/* not a potentially italic font */
	if (mathitalic != 0 && (chrs >= 40 && chrs <= 63)) return 0; /* nums */
	if (mathitalic != 0 && (chrs >= 91 && chrs <= 93)) return 0; /* music */
	if (mathsymbol != 0 && (chrs < 64 || chrs > 90)) return 0; /* not cal */
	return -1;			/* this character is italic */
}

int italicchar(void) { /* is character REALLY italic, slanted or calligra ? */
	if (showslanted != 0) return 0;
	else return italiccharsub();
}

int calligraphicsub(void) {
	if (mathsymbol != 0 && (chrs >= 64 && chrs <= 90)) return -1; 
	else return 0;
}

int calligraphic(void) {
	if (showslanted != 0) return 0;
	else return calligraphicsub();
}	

void eliminate (struct stroke strokes[], int j, int nstrokes) {
	int k;
/*	printf("Eliminating %d\n", j); */
	for (k = j; k < nstrokes - 1; k++)	strokes[k] = strokes[k+1];
/*	printf("Eliminated %d\n", j); */
}

void combine(struct stroke strokes[], int i, int j, int nstrokes) {
	if (strokes[i].end == strokes[j].start) {
		strokes[i].end = strokes[j].end;
		eliminate(strokes, j, nstrokes);
	}
	else if (strokes[i].start == strokes[j].end) {
		strokes[i].start = strokes[j].start;
		eliminate(strokes, j, nstrokes);
	}
	else fprintf(stderr, "Can't combine strokes %d and %d\n", i, j);
}

int connected(struct stroke strokes[], int i, int j) {
	if (strokes[i].end == strokes[j].start ||
		strokes[j].end == strokes[i].start) return 1;
	else return 0;
}

int merge(struct stroke strokes[], int nstrokes) {
	int i, j, x, flag;
	i = 0;
	while (i < nstrokes -1) {
/*		printf("i = %d ", i); getsafe(); */
		x = strokes[i].pos;
		j = i + 1;
		flag = 0;
		while (j < nstrokes && x == strokes[j].pos) { /* fixed ! */
/*			printf("j = %d ", j); getsafe(); */
			if (connected(strokes, i, j)) {
/*				printf("Combining %d and %d\n", i, j); */
				combine(strokes, i, j, nstrokes);
				nstrokes--;
				flag++;
			}
			j++;
		}
		if (flag == 0) i = j;
	}
	if (nstrokes < 0) fprintf(stderr, "Negative strokes in merge ");
	return nstrokes;
}

/* int compare(struct stroke *elem1, struct stroke *elem2) {
	int pos1, pos2;
	pos1 = elem1->pos;
	pos2 = elem2->pos;
	if (pos1 > pos2) return +1;
	else if (pos1 < pos2) return -1;
	else return 0;
} */

int compare(const void *ptelem1, const void *ptelem2) {
	const struct stroke *elem1, *elem2;
	int pos1, pos2;
	elem1 = (const struct stroke *) ptelem1;
	elem2 = (const struct stroke *) ptelem2;
	pos1 = elem1->pos; 
	pos2 = elem2->pos; 
	if (pos1 > pos2) return +1;
	else if (pos1 < pos2) return -1;
	else return 0;
}


/* void __cdecl qsort(void *, size_t, size_t, int (__cdecl *)
	(const void *, const void *)); */
/* size_t unsigned int */

/* sort and merge the strokes arrays */

void sortupstrokes(void) { /* sort and merge up strokes */
	if (traceflag != 0) printf("SORTING UP %d ", nupstrokes); 
	qsort(upstrokes, nupstrokes, sizeof(upstrokes[0]), compare);
	nupstrokes = merge(upstrokes, nupstrokes);
	if (traceflag != 0) {printf("=> %d ", nupstrokes); getsafe();}
}

void sortdownstrokes(void) {  /* sort and merge down strokes */
	if (traceflag != 0) printf("SORTING DOWN %d ", ndownstrokes); 
	qsort(downstrokes, ndownstrokes, sizeof(downstrokes[0]), compare);
	ndownstrokes = merge(downstrokes, ndownstrokes);
	if (traceflag != 0) {printf("=> %d ", ndownstrokes); getsafe();}
}

void sortleftstrokes(void) { /* sort and merge left strokes */
	if (traceflag != 0) 
		printf("SORTING LEFT %d ", nleftstrokes); 
	qsort(leftstrokes, nleftstrokes, sizeof(leftstrokes[0]), compare);
	nleftstrokes = merge(leftstrokes, nleftstrokes);
/*	if (nleftstrokes < 0) fprintf(stderr, "Negative leftstrokes "); */
	if (traceflag != 0) {printf("=> %d ", nleftstrokes); getsafe();}
}

void sortrightstrokes(void) {  /* sort and merge right strokes */
	if (traceflag != 0) 
		printf("SORTING RIGHT %d ", nrightstrokes); 
	qsort(rightstrokes, nrightstrokes, sizeof(rightstrokes[0]), compare);
	nrightstrokes = merge(rightstrokes, nrightstrokes);
/*	if (nrightstrokes < 0) fprintf(stderr, "Negative rightstrokes "); */
	if (traceflag != 0) {printf("=> %d ", nrightstrokes); getsafe();}
}

/* record strokes in arrays of stroke structures */

void recordupstroke(int x, int ys, int ye) {
	if (nupstrokes >= MAXSTROKES) {
		if (complainnupstrokes++ == 0)
			fprintf(stderr, "Too many up strokes\n");
		return;
	}
	if (ye != ys && ye < ys + minseg) return; 
	upstrokes[nupstrokes].pos = x;
	upstrokes[nupstrokes].start = ys;
	upstrokes[nupstrokes].end = ye;
	upstrokes[nupstrokes].flag = 0;
	nupstrokes++;
}

void recorddownstroke(int x, int ys, int ye) {
	if (ndownstrokes >= MAXSTROKES) {
		if (complainndownstrokes++ == 0)
			fprintf(stderr, "Too many down strokes\n");
		return;
	}
	if (ye != ys && ys < ye + minseg) return; 
	downstrokes[ndownstrokes].pos = x;
	downstrokes[ndownstrokes].start = ys;
	downstrokes[ndownstrokes].end = ye;
	downstrokes[ndownstrokes].flag = 0;
	ndownstrokes++;
}

void recordleftstroke(int y, int xs, int xe) {
	if (nleftstrokes >= MAXSTROKES) {
		if (complainnleftstrokes++ == 0)
			fprintf(stderr, "Too many left strokes\n");
		return;
	}
	if (xe != xs && xs < xe + minseg) return; 
	leftstrokes[nleftstrokes].pos = y;
	leftstrokes[nleftstrokes].start = xs;
	leftstrokes[nleftstrokes].end = xe;
	leftstrokes[nleftstrokes].flag = 0;
	nleftstrokes++;
}

void recordrightstroke(int y, int xs, int xe) {
	if (nrightstrokes >= MAXSTROKES) {
		if (complainnrightstrokes++ == 0)
			fprintf(stderr, "Too many right strokes\n");
		return;
	}
	if (xe != xs && xe < xs + minseg) return; 
	rightstrokes[nrightstrokes].pos = y;
	rightstrokes[nrightstrokes].start = xs;
	rightstrokes[nrightstrokes].end = xe;
	rightstrokes[nrightstrokes].flag = 0;
	nrightstrokes++;
}

/* needed by automatic ghost insertion at top of sub-character */

void insertleftstroke(int y, int xs, int xe, int k) {
	int l;
	if (nleftstrokes >= MAXSTROKES) {
		if (complainnleftstrokes++ == 0)
			fprintf(stderr, "Too many left strokes\n");
		return;
	}
	for (l=nleftstrokes-1; l >= k; l--) leftstrokes[l+1] = leftstrokes[l];
	nleftstrokes++;
	leftstrokes[k].pos = y;
	leftstrokes[k].start = xs;
	leftstrokes[k].end = xe;
	leftstrokes[k].flag = 0;
}

/* needed by automatic ghost insertion at bottom of sub-character */

void insertrightstroke(int y, int xs, int xe, int k) {
	int l;
	if (nrightstrokes >= MAXSTROKES) {
		if (complainnrightstrokes++ == 0)
			fprintf(stderr, "Too many right strokes\n");
		return;
	}
	for (l=nrightstrokes-1; l >= k; l--) rightstrokes[l+1] = rightstrokes[l];
	nrightstrokes++;
	rightstrokes[k].pos = y;
	rightstrokes[k].start = xs;
	rightstrokes[k].end = xe;	
	rightstrokes[k].flag = 0;
}

void recordleftright(int ystart, int xstart, int xlast) {
	int dx = xlast - xstart;
	if (dx > 0 && reverseflag != 0)
		recordrightstroke(ystart, xstart, xlast);
	else if (dx < 0 && reverseflag == 0)
		recordrightstroke(ystart, xlast, xstart);
	else if (dx < 0 && reverseflag != 0)
		recordleftstroke(ystart, xstart, xlast);
	else if (dx > 0 && reverseflag == 0)
		recordleftstroke(ystart, xlast, xstart);
}

void recordupdown(int xstart, int ystart, int ylast) {
	int dy = ylast - ystart; 
	if (dy > 0 && reverseflag != 0)
		recordupstroke(xstart, ystart, ylast);
	else if (dy < 0 && reverseflag == 0)
		recordupstroke(xstart, ylast, ystart);
	else if (dy < 0 && reverseflag != 0)
		recorddownstroke(xstart, ystart, ylast);
	else if (dy > 0 && reverseflag == 0)
		recorddownstroke(xstart, ylast, ystart);
}

/* combine with strokeflex to remove redundancy ? */

void strokeknots(int n) { /* enter subpath stroke information in arrays */
	int i, xnew, ynew, xold, yold, dx, dy;

	if (traceflag != 0) printf("STROKE KNOTS %d ", n);
	xold = knots[0].x; yold = knots[0].y;
	for (i = 1; i < n; i++) {
		xnew = knots[i].x; ynew = knots[i].y;
		dx = xnew - xold; dy = ynew - yold;
		if (dx == 0 && italicchar() == 0) { /* don't if vstem suppressed ? */
			recordupdown(xnew, yold, ynew);
		}
		else if (dy == 0 && calligraphic() == 0) {	/* don't calligraphic */
			recordleftright(ynew, xold, xnew);
		}  
		xold = xnew; yold = ynew;
	}
} 

/* look for FlexProc like features - somewhat crude but very effective */
/* assumes subpath ends are not in middle of Flex ! */
/* assumes no extraneous line segments in middle of Flex ! */

/* void strokeflex(int n) { 
	int i, xnew, ynew, xmid, ymid, xold, yold, dx, dy;
	int dmx, dmy;

	if (n < 7) return;
	if (traceflag != 0) printf("FLEX KNOTS %d ", n);
	for (i = 0; i < n-6; i++) {
		if (knots[i+3].code != 'C') continue;
		if (knots[i+6].code != 'C') continue;
		xold = knots[i].x; yold = knots[i].y;
		xmid = knots[i+3].x; ymid = knots[i+3].y;
		xnew = knots[i+6].x; ynew = knots[i+6].y;
		dx = xnew - xold; dy = ynew - yold;
		dmx = xmid - xold; dmy = ymid - yold;
		if (dx == 0 && (ABS(dmx) < flexmax) && (ABS(dy) > flexmin)) {
			recordupdown(xnew, yold, ynew);
		}
		else if (dy == 0 && (ABS(dmy) < flexmax) && (ABS(dx) > flexmin)) {
			recordleftright(ynew, xold, xnew);
		}
	}
} */

/* A horizontal flex satisfies the following: */
/*	o	end points must have same height */
/*	o	intermediate points are either ALL above or ALL below chord */
/*	o	all intermdiate points are within flexmin of chord */
/*	o	must start and end on knot (not control point) */
/*	o	chord is at least flexmax long */
/* A vertical flex satisfies corresponding... */

void strokeflex(int n) {						/* new version */
	int i, j, jlast, upflag, rightflag;
	int xold, yold, xnew, ynew;
	int xstart, ystart, xlast, ylast, xnext, ynext, dx, dy;

	if (n < 3) return;									/* don't bother */
	if (traceflag != 0) printf("FLEX KNOTS %d ", n);

	jlast = 0;
	for (i = 0; i < n; i++) {
		if (jlast != i) {
			xold = knots[i-1].x; yold = knots[i-1].y;
			xnew = knots[i].x; ynew = knots[i].y;
			dx = xnew - xold; dy = ynew - yold;
/*			if (dx == 0 && dy != 0) recordupdown(xnew, yold, ynew); */
			if (dy == 0 && dx != 0) recordleftright(ynew, xold, xnew);	 
		}
		if (knots[i].code == ' ') { /* scan up to next knot */
			continue;
		}
		xstart = knots[i].x; ystart = knots[i].y;
		xlast = xstart; ylast = ystart; jlast = i;		/* just in case */
		upflag = 0;										/* deflection */
		for (j = i + 1; j < n; j++) {
			xnext = knots[j].x; ynext = knots[j].y;
			if (ynext > ystart) {
				if (upflag < 0) break;					/* crossed over */
				if (ynext > ystart + flexmax) break;	/* too far above */
				upflag = 1;								/* upward bow */
			}
			else if (ynext < ystart) {
				if (upflag > 0) break;					/* crossed over */
				if (ynext < ystart - flexmax) break;	/* too far below */
				upflag = -1;							/* downward bow */
			}
			if (knots[j].code != ' ') {
				xlast = xnext; ylast = ynext; jlast = j; /* previous knot */
				if (ylast == ystart) break;				 /* ? */
			}
		}
		dx = xlast - xstart; 
		if (ylast == ystart &&	/* j > i + 1 && knots[j-1].code != ' ' && */
			jlast > i &&
			((upflag == 0 && ABS(dx) > 0) || (ABS(dx) > flexmin))) {
/*			 printf("Y: %d, XS %d, XE %d ", ystart, xstart, xlast); */
			 recordleftright(ystart, xstart, xlast);
			i = jlast-1;  /* i = j-1; */				/* step over */
		}
	}

	jlast = 0;
	for (i = 0; i < n; i++) {
		if (jlast != i) {
			xold = knots[i-1].x; yold = knots[i-1].y;
			xnew = knots[i].x; ynew = knots[i].y;
			dx = xnew - xold; dy = ynew - yold;
			if (dx == 0 && dy != 0) recordupdown(xnew, yold, ynew); 
/*			if (dy == 0 && dx != 0) recordleftright(ynew, xold, xnew); */
		}
		if (knots[i].code == ' ') {				/* scan up to next knot */
			continue;
		}
		xstart = knots[i].x; ystart = knots[i].y;
		xlast = xstart;	ylast = ystart; jlast = i;		/* just in case */

		rightflag = 0;									/* deflection */
		for (j = i + 1; j < n; j++) {
			xnext = knots[j].x; ynext = knots[j].y;
			if (xnext > xstart) {
				if (rightflag < 0) break;				/* crossed over */
				if (xnext > xstart + flexmax) break;	/* too far right */
				rightflag = 1;							/* rightward bow */
			}
			else if (xnext < xstart) {
				if (rightflag > 0) break;				/* crossed over */
				if (xnext < xstart - flexmax) break;	/* too far left */
				rightflag = -1;							/* leftward bow */
			}
			if (knots[j].code != ' ') {
				xlast = xnext; ylast = ynext; jlast = j; /* previous knot */
				if (xlast == xstart) break;				 /* ? */
			}
		}
		dy = ylast - ystart;  
		if (xlast == xstart &&	/* j > i + 1 && knots[j-1].code != ' ' && */
			jlast > i &&
			((rightflag == 0 && ABS(dy) > 0) || (ABS(dy) > flexmin))) {
/*			printf("X: %d, YS %d, YE %d ", xstart, ystart, ylast);  */
			recordupdown(xstart, ystart, ylast);
			i = jlast-1;  /* i = j-1; */				/* step over */
		}
	}
}

/* enter extrema stroke information - OLD VERSION
void strokeextrema(int n) { 
	int i, xnew, ynew, xcurr, ycurr, xold, yold;

	if (n == 0) {
		fprintf(stderr, "No outline ");
		if (batchmode != 0) fprintf(stderr, "for character %d\n", chrs);
		return;
	}
	if ((knots[n-1].x != knots[0].x) ||
		(knots[n-1].y != knots[0].y)) {
		fprintf(stderr, "Last knot not equal to first knot ");
		if (batchmode != 0) fprintf(stderr, "character %d\n", chrs);		
		else putc('\n', stderr);
		knots[n].x = knots[0].x;	knots[n].y = knots[0].y;		
		knots[n].code = 'E';
		n++; knot++;
	}

	if (traceflag != 0) printf("STROKE EXTREMA %d ", n);
	xold = knots[n-2].x; yold = knots[n-2].y;
	xcurr = knots[0].x; ycurr = knots[0].y;	
	for (i = 1; i < n; i++) {
		xnew = knots[i].x; ynew = knots[i].y;
		if (xcurr > xold && xcurr > xnew) {
			if ((ynew > yold && reverseflag == 0) ||
				(ynew < yold && reverseflag != 0))
				recorddownstroke(xcurr, -INFINITY, -INFINITY);
			else if ((ynew > yold && reverseflag != 0) ||
				     (ynew < yold && reverseflag == 0))
				     recordupstroke(xcurr, -INFINITY, -INFINITY);	
		} 
		else if (xcurr < xold && xcurr < xnew) {
			if ((ynew > yold && reverseflag != 0) ||
				(ynew < yold && reverseflag == 0))
				recordupstroke(xcurr, -INFINITY, -INFINITY);
			else if ((ynew > yold && reverseflag == 0) ||
				     (ynew < yold && reverseflag != 0))
					 recorddownstroke(xcurr, -INFINITY, -INFINITY);	
		}
		if (ycurr > yold && ycurr > ynew) {
			if ((xnew > xold && reverseflag == 0) ||
				(xnew < xold && reverseflag != 0))
				recordleftstroke(ycurr, -INFINITY, -INFINITY);
			else if ((xnew > xold && reverseflag != 0) ||
				     (xnew < xold && reverseflag == 0))
					 recordrightstroke(ycurr, -INFINITY, -INFINITY);	
		}
		else if (ycurr < yold && ycurr < ynew) {
			if ((xnew > xold && reverseflag != 0) ||
				(xnew < xold && reverseflag == 0))
				recordrightstroke(ycurr, -INFINITY, -INFINITY);
			else if ((xnew > xold && reverseflag == 0) ||
				     (xnew < xold && reverseflag != 0))
					 recordleftstroke(ycurr, -INFINITY, -INFINITY);	
		}
		xold = xcurr; yold = ycurr; xcurr = xnew; ycurr = ynew;
	}
}  */

/*		was: knots[n-1].x = knots[0].x;	knots[n-1].y = knots[0].y; */

/* The following version only inserts extrema that occur at knots */
/* - well-behaved outlines should not have extrema in between */
/* - to this right would have to compute tangets on curvetos */

/* NEW - modified version */
void strokeextrema(int n) {						
	int i, xnew, ynew, xcurr, ycurr, xold, yold;

	if (n == 0) {
		fprintf(stderr, "No outline ");
		if (batchmode != 0) fprintf(stderr, "for character %d\n", chrs);
		return;
	}
	if ((knots[n-1].x != knots[0].x) ||
		(knots[n-1].y != knots[0].y)) {
		fprintf(stderr, "Last knot not equal to first knot ");
		if (batchmode != 0) fprintf(stderr, "character %d\n", chrs);
		else putc('\n', stderr);
		knots[n].x = knots[0].x;	knots[n].y = knots[0].y;		
		knots[n].code = 'E';
		n++; knot++;
	}

	if (traceflag != 0) printf("STROKE EXTREMA %d ", n);
	xold = knots[n-2].x; yold = knots[n-2].y;
	xcurr = knots[0].x; ycurr = knots[0].y;	
	for (i = 1; i < n; i++) {
		xnew = knots[i].x; ynew = knots[i].y;
		if (knots[i-1].code != ' ') {				
		if (xcurr > xold && xcurr > xnew) {
			if ((ynew > yold && reverseflag == 0) ||
				(ynew < yold && reverseflag != 0))
				recorddownstroke(xcurr, -INFINITY, -INFINITY);
			else if ((ynew > yold && reverseflag != 0) ||
				     (ynew < yold && reverseflag == 0))
				     recordupstroke(xcurr, -INFINITY, -INFINITY);	
		} 
		else if (xcurr < xold && xcurr < xnew) {
			if ((ynew > yold && reverseflag != 0) ||
				(ynew < yold && reverseflag == 0))
				recordupstroke(xcurr, -INFINITY, -INFINITY);
			else if ((ynew > yold && reverseflag == 0) ||
				     (ynew < yold && reverseflag != 0))
					 recorddownstroke(xcurr, -INFINITY, -INFINITY);	
		}
		if (ycurr > yold && ycurr > ynew) {
			if ((xnew > xold && reverseflag == 0) ||
				(xnew < xold && reverseflag != 0))
				recordleftstroke(ycurr, -INFINITY, -INFINITY);
			else if ((xnew > xold && reverseflag != 0) ||
				     (xnew < xold && reverseflag == 0))
					 recordrightstroke(ycurr, -INFINITY, -INFINITY);	
		}
		else if (ycurr < yold && ycurr < ynew) {
			if ((xnew > xold && reverseflag != 0) ||
				(xnew < xold && reverseflag == 0))
				recordrightstroke(ycurr, -INFINITY, -INFINITY);
			else if ((xnew > xold && reverseflag == 0) ||
				     (xnew < xold && reverseflag != 0))
					 recordleftstroke(ycurr, -INFINITY, -INFINITY);	
		}
		} 
		xold = xcurr; yold = ycurr; xcurr = xnew; ycurr = ynew;
	}
} 

/* was:	knots[n-1].x = knots[0].x;	knots[n-1].y = knots[0].y;	*/

/* Above may have occasional problems get extrema - */
/* - when short and slightly slanted or extremum is on curveto ... */

/* To do the following need to take into account winding - */
/* - need to determine sense of path first */

void strokebounding(int k) {
	int i, xold, yold, xnew, ynew;
	long area=0, darea;

	if (k < 2) return;
	xold = knots[0].x; yold = knots[0].y;
	for (i = 1; i < k; i++) {
		if (knots[i].code != ' ') {	/* only consider actual knots */
			xnew = knots[i].x; ynew = knots[i].y;
			darea = (long) xold * (long) ynew - (long) xnew * (long) yold;
			area = area + darea;
			xold = xnew; yold = ynew;
		}
	}
	if (area > 0) {
/*		if (rightstrokes[nrightstrokes-1].pos != ymsx) */
			recordrightstroke(ymsx, -INFINITY, -INFINITY);	
/*		if (leftstrokes[0].pos != ymsn) */
			recordleftstroke(ymsn, -INFINITY, -INFINITY);
/*		if (downstrokes[ndownstrokes-1].pos != xmsx) */
			recorddownstroke(xmsx, -INFINITY, -INFINITY);
/*		if (upstrokes[0].pos != xmsn) */
			recordupstroke(xmsn, -INFINITY, -INFINITY);		
	}
	else if (area < 0) {
/*		if (rightstrokes[0].pos != ymsn) */
			recordrightstroke(ymsn, -INFINITY, -INFINITY);	
/*		if (leftstrokes[nleftstrokes-1].pos != ymsn) */
			recordleftstroke(ymsx, -INFINITY, -INFINITY);	
/*		if (downstrokes[0].pos != xmsx) */
			recorddownstroke(xmsn, -INFINITY, -INFINITY);	
/*		if (upstrokes[nupstrokes-1].pos != xmsn) */
			recordupstroke(xmsx, -INFINITY, -INFINITY);	
	} 
}

void showgrerror(short err) {
	switch (err) {
		case 0:
			fprintf(stderr, "GR no error"); break;
		case _GRERROR:
			fprintf(stderr, "GR error"); break;
		case _GRMODENOTSUPPORTED:
			fprintf(stderr, "GR mode not supported"); break;
		case _GRNOTINPROPERMODE:
			fprintf(stderr, "GR not in proper mode"); break;
		case _GRFONTFILENOTFOUND:
			fprintf(stderr, "GR font file not found"); break;
		case _GRINVALIDFONTFILE:
			fprintf(stderr, "GR INVALIDFONTFILE"); break;
		case _GRCORRUPTEDFONTFILE:
			fprintf(stderr, "GR corrupted font file"); break;
		case _GRINSUFFICIENTMEMORY:
			fprintf(stderr, "GR insufficient memory"); break;
		case _GRINVALIDIMAGEBUFFER:
			fprintf(stderr, "GR invalid image buffer"); break;
		default:
			fprintf(stderr, "GR unknown error %d", err); break;
	}
	fprintf(stderr, "\n");
}

void sethintfont(void) {
	char *font;

/*	if (_setfont("t'helv'h10w5b") < 0)  */
	if (smallfont != 0) font = "t'helv'h10w5b";
	else font = "t'helv'h12w6b";
	if (_setfont(font) < 0) {
		if (_setfont("t'helv'h10w5b") < 0) {
			fprintf(stderr, "Failed to set font\n");
			showgrerror(_grstatus());
		}
	}
}

void hintarrsub(int n, int flag) { 		/* show verticals and horizontals */
	int k, x, y;
	int xold=-INFINITY, yold=-INFINITY;
	
	if (automode != 0) return;
	
	if (hintnumshow != 0) {
/*		if (_setfont("t'helv'h10w5b") < 0)  */
/*			fprintf(stderr, "Failed to set font\n"); */
		sethintfont();
	}
	for (k = 0; k < n; k++) {
		x = knots[k].x; y = knots[k].y;
		if (x == xold) {
			if ((y > yold && reverseflag != 0) ||
				(y < yold && reverseflag == 0)) vstemup(x, 0, flag);
			else vstemdown(x, 0, flag);
		}
		if (y == yold) {
			if ((x < xold && reverseflag != 0) ||
				(x > xold && reverseflag == 0)) hstemleft(y, 0, flag);
			else hstemright(y, 0, flag);
		}
		xold = x; yold = y;
	}
}

void hintarr(int n) {	hintarrsub(n, 0); 	hintarrsub(n, 1); }

void showstrokessub(int flag) { 		/* show verticals and horizontals */
	int k;
	
	if (automode != 0) return;

	if (hintnumshow != 0) {
/*		if (_setfont("t'helv'h10w5b") < 0)  */
/*			fprintf(stderr, "Failed to set font\n"); */
		sethintfont();
	}
	if (italicchar() == 0) { 
		if (traceflag != 0) {
			printf("SHOWUPSTROKES %d ", nupstrokes); getsafe();
		}
		for (k = 0; k < nupstrokes; k++) {
			vstemup(upstrokes[k].pos, upstrokes[k].flag, flag);
		}
		if (traceflag != 0) {
			printf("SHOWDOWNSTROKES %d ", ndownstrokes); getsafe();
		}
		for (k = 0; k < ndownstrokes; k++) {
			vstemdown(downstrokes[k].pos, downstrokes[k].flag, flag);
		}
		if (traceflag != 0) {
			printf("SHOWVSTEMS %d ", numvstems); getsafe();
		}
/*		printf("D %d U %d ", ndownstrokes, nupstrokes); */
	} 
	if (calligraphic() == 0) { 
		if (traceflag != 0) {
			printf("SHOWLEFTSTROKES %d ", nleftstrokes); getsafe();
		}
		for (k = 0; k < nleftstrokes; k++) {
			hstemleft(leftstrokes[k].pos, leftstrokes[k].flag, flag);
		}
		if (traceflag != 0) {
			printf("SHOWRIGHTSTROKES %d ", nrightstrokes); getsafe();
		}
		for (k = 0; k < nrightstrokes; k++) {
			hstemright(rightstrokes[k].pos, rightstrokes[k].flag, flag);
		}
		if (traceflag != 0) {
			printf("SHOWHSTEMS %d ", numhstems); getsafe();
		}
/*		printf("L %d R %d ", nleftstrokes, nrightstrokes); */
	}	  
}

/* show stem widths on screen   --- OLD VERSION
void showstemwidths(int flag) {
	int ku, kd, kl, kr, xu, xd, yl, yr;

	if (automode != 0) return;
	if (stemnumshow == 0) return;
	if (suppressflag != 0) flag = 0;

	kl = 0; kr = 0; yl = -INFINITY; yr = -INFINITY;
	while (kl < nleftstrokes && kr < nrightstrokes) {
		while (leftstrokes[kl].flag == 0 && kl < nleftstrokes) kl++;
		if (kl != nleftstrokes) yl = leftstrokes[kl].pos;
		else yl = -INFINITY;
		while (rightstrokes[kr].flag == 0 && kr < nrightstrokes) kr++;
		if (kr != nrightstrokes) yr = rightstrokes[kr].pos;
		else yr = -INFINITY;
		if (kl == nleftstrokes || kr == nrightstrokes) break;
		if (yr > yl) {
			if (ignoresizes != 0 ||
				(yr > yl + minstem && yr < yl + maxstem))
					hstemwidth(yl, yr, flag);
		}
		kl++; kr++;
	}

	ku = 0; kd = 0; xu = -INFINITY; xd = -INFINITY;
	while (ku < nupstrokes && kd < ndownstrokes) {
		while (upstrokes[ku].flag == 0 && ku < nupstrokes) ku++;
		if (ku != nupstrokes) xu = upstrokes[ku].pos;
		else xu = -INFINITY;
		while (downstrokes[kd].flag == 0 && kd < ndownstrokes) kd++;
		if (kd != ndownstrokes) xd = downstrokes[kd].pos;
		else xd = -INFINITY;
		if (ku == nupstrokes || kd == ndownstrokes) break;
		if (xd > xu) {
			if (ignoresizes != 0 ||
				(xd > xu + minstem && xd < xu + maxstem))
					vstemwidth(xu, xd, flag);
		}
		ku++; kd++;
	}
} */

/* Following is messy because it has to deal with incomplete strokes */

void showstemwidths(int flag) {
	int ku, kd, kl, kr, xu, xd, yl, yr;
	int kln, krn, yln, yrn;
	int kun, kdn, xun, xdn;

	if (automode != 0) return;
	if (stemnumshow == 0) return;
	if (suppressflag != 0) flag = 0;

	kl = 0; kr = 0; /* yl = -INFINITY; yr = -INFINITY;  */
/* find first marked left stroke - if any */
	while (leftstrokes[kl].flag == 0 && kl < nleftstrokes) kl++;		
	if (kl != nleftstrokes) yl = leftstrokes[kl].pos;
	else yl = INFINITY;
/* find first marked right stroke that is higher */
	for(;;) {
		while (rightstrokes[kr].flag == 0 && kr < nrightstrokes) kr++;
		if (kr != nrightstrokes) yr = rightstrokes[kr].pos;
		else yr = INFINITY;
		if (yr >= yl) break;
		kr++;
	}
/* now pick up all stems */
	for(;;) {
/* have we run out of matching strokes ? */
		if (kl == nleftstrokes) break;
		if (kr == nrightstrokes) break;
		kln = kl + 1;
/* move up left stroke until next left one is above present right stroke */
		for (;;) {
			while (leftstrokes[kln].flag == 0 && kln < nleftstrokes) kln++;
			if (kln != nleftstrokes) yln = leftstrokes[kln].pos;
			else yln = INFINITY;
			if (yln >= yr) break;
			kl = kln; yl = yln;
			kln++;
		}
/* show stem between yl and yr */
		if (ignoresizes != 0 ||
			(yr > yl + minstem && yr < yl + maxstem))
				hstemwidth(yl, yr, flag);
		krn = kr + 1;
/* find the next right stroke above new left stroke */
		for(;;) {
			while (rightstrokes[krn].flag == 0 && krn < nrightstrokes) krn++;
			if (krn != nrightstrokes) yrn = rightstrokes[krn].pos;
			else yrn = INFINITY;
			if (yrn >= yln) break;
			krn++;
		}
		kl = kln; kr = krn; yl = yln; yr = yrn;
	}

/* now do vertical stems - modelled on above */

	ku = 0; kd = 0; /* xu = -INFINITY; xd = -INFINITY;  */
/* find first marked up stroke - if any */
	while (upstrokes[ku].flag == 0 && ku < nupstrokes) ku++;		
	if (ku != nupstrokes) xu = upstrokes[ku].pos;
	else xu = INFINITY;
/* find first marked down stroke that is higher */
	for(;;) {
		while (downstrokes[kd].flag == 0 && kd < ndownstrokes) kd++;
		if (kd != ndownstrokes) xd = downstrokes[kd].pos;
		else xd = INFINITY;
		if (xd >= xu) break;
		kd++;
	}
/* now pick up all stems */
	for(;;) {
/* have we run out of matching strokes ? */
		if (ku == nupstrokes) break;
		if (kd == ndownstrokes) break;
		kun = ku + 1;
/* move up stroke right until next up is right of present down stroke */
		for (;;) {
			while (upstrokes[kun].flag == 0 && kun < nupstrokes) kun++;
			if (kun != nupstrokes) xun = upstrokes[kun].pos;
			else xun = INFINITY;
			if (xun >= xd) break;
			ku = kun; xu = xun;
			kun++;
		}
/* show stem between xu and xd */
		if (ignoresizes != 0 ||
			(xd > xu + minstem && xd < xu + maxstem))
				vstemwidth(xu, xd, flag);
		kdn = kd + 1;
/* find the next down stroke to right of new up stroke */
		for(;;) {
			while (downstrokes[kdn].flag == 0 && kdn < ndownstrokes) kdn++;
			if (kdn != ndownstrokes) xdn = downstrokes[kdn].pos;
			else xdn = INFINITY;
			if (xdn >= xun) break;
			kdn++;
		}
		ku = kun; kd = kdn; xu = xun; xd = xdn;
	}
}

void showstrokes(void) { 		/* show verticals and horizontals */
	if (automode != 0) return;
	showstrokessub(0); showstrokessub(1); showstemwidths(1);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following are responses to interactive commands */

void leftarrowsub(void) {
	int kup, kdown, xup, xdown, fup=0, fdown=0;

	cursory = -INFINITY;
	if (cursorx == -INFINITY) cursorx = INFINITY; /* ? */
	xup = cursorx;			/* to keep compiler quite */
	for (kup = nupstrokes-1; kup >= 0; kup--) {
		xup = upstrokes[kup].pos;
		if (xup < cursorx) {fup = 1; break;}
	}
	xdown = cursorx; 	/* to keep compiler quite */
	for (kdown = ndownstrokes-1; kdown >= 0; kdown--) {
		xdown = downstrokes[kdown].pos;
		if (xdown < cursorx) {fdown = 1; break;}
	}

	if (fup != 0 && fdown == 0) cursorx = xup;
	else if (fup == 0 && fdown != 0) cursorx = xdown;
	else if (fup != 0 && fdown != 0) {
/*		assert(xup != xdown); */
		if (xup == xdown) {
/*			fprintf(stderr, "*c* "); */
/*			upflag = 1 - upflag; */
		}
		cursorx = MAX(xup, xdown);
	} /* otherwise cant move further left */
/*	printf("xu %d xd %d x %d", xup, xdown, cursorx); */
}

void leftarrow(void) {
	leftarrowsub();
	showstrokes(); /*	redisplay number */
}

void rightarrowsub(void) {
	int kup, kdown, xup, xdown, fup=0, fdown=0;

	cursory = -INFINITY;
	xup = cursorx; 	/* to keep compiler quite */
	for (kup = 0; kup < nupstrokes; kup++) {
		xup = upstrokes[kup].pos;
		if (xup > cursorx) {fup = 1; break;}
	}
	xdown = cursorx; 	/* to keep compiler quite */
	for (kdown = 0; kdown < ndownstrokes; kdown++) {
		xdown = downstrokes[kdown].pos; 
		if (xdown > cursorx) {fdown = 1; break;}
	}

	if (fup != 0 && fdown == 0) cursorx = xup;
	else if (fup == 0 && fdown != 0) cursorx = xdown;
	else if (fup != 0 && fdown != 0) {
/*		assert(xup != xdown); */
		if (xup == xdown) {
/*			fprintf(stderr, "*c* "); */
/*			upflag = 1 - upflag; */
		}
		cursorx = MIN(xup, xdown);
	} /* otherwise cant move further right */
/*	printf("xu %d xd %d x %d ", xup, xdown, cursorx); */

}
			
void rightarrow(void) {
	rightarrowsub();
	showstrokes(); /*	redisplay number */
}

void downarrowsub(void) {
	int kleft, kright, yleft, yright, fleft=0, fright=0;

	cursorx = -INFINITY;
	if (cursory == -INFINITY) cursory = INFINITY; /* ? */
	yleft = cursory; 	/* to keep compiler quite */
	for (kleft = nleftstrokes-1; kleft >= 0; kleft--) {
		yleft = leftstrokes[kleft].pos;
		if (yleft < cursory) {fleft = 1; break;}
	}
	yright = cursory; 	/* to keep compiler quite */
	for (kright = nrightstrokes-1; kright >= 0; kright--) {
		yright = rightstrokes[kright].pos;
		if (yright < cursory) {fright = 1; break;}
	}

	if (fleft != 0 && fright == 0) cursory = yleft;
	else if (fleft == 0 && fright != 0) cursory = yright;
	else if (fleft != 0 && fright != 0) {
/*		assert(yleft != yright); */
		if (yleft == yright) {
/*			fprintf(stderr, "*c* "); */
/*			leftflag =  1 - leftflag;  */
		}
		cursory = MAX(yleft, yright);
	} /* otherwise cant move further up */
/*	printf("yl %d yr %d y %d", yleft, yright, cursory); */
}

void downarrow(void) {
	downarrowsub();
	showstrokes(); /*	redisplay number */
}
			
void uparrowsub(void) {
	int kleft, kright, yleft, yright, fleft=0, fright=0;

	cursorx = -INFINITY;
	yleft = cursory; 	/* to keep compiler quite */
	for (kleft = 0; kleft < nleftstrokes; kleft++) {
		yleft = leftstrokes[kleft].pos;
		if (yleft > cursory) {fleft = 1; break;}
	}
	yright = cursory; 	/* to keep compiler quite */
	for (kright = 0; kright < nrightstrokes; kright++) {
		yright = rightstrokes[kright].pos; 
		if (yright > cursory) {fright = 1; break;}
	}

	if (fleft != 0 && fright == 0) cursory = yleft;
	else if (fleft == 0 && fright != 0) cursory = yright;
	else if (fleft != 0 && fright != 0) {
/*		assert(yleft != yright); */
		if (yleft == yright) {
/*			fprintf(stderr, "*c* "); */
/*			leftflag =  1 - leftflag;  */
		}
		cursory = MIN(yleft, yright);
	} /* otherwise cant move further right */
/*	printf("yu %d yd %d y %d ", yleft, yright, cursory); */
}

void uparrow(void) {
	uparrowsub();
	showstrokes();					/*	redisplay number */
}
			
int insertsub(void) {
	int ku, kd, kl, kr, flag = 0;

	if (cursorx != -INFINITY && cursorx != INFINITY) { 
		for (ku = 0; ku < nupstrokes; ku++) {
			if (upstrokes[ku].pos == cursorx) break;
		}
		for (kd = 0; kd < ndownstrokes; kd++) {
			if (downstrokes[kd].pos == cursorx) break;
		}				
		if (ku < nupstrokes && kd < ndownstrokes) {
			if (upflag != 0) kd = ndownstrokes;
			else ku = nupstrokes;
/*			upflag = 1 - upflag;		*/
		}
		if (ku < nupstrokes) {
			upstrokes[ku].flag = 1; flag = 1; 
		}
		else if (kd < ndownstrokes) {
			downstrokes[kd].flag = 1; flag = -1; 
		}
	}
	else if (cursory != -INFINITY && cursory != INFINITY) { 
		for (kl = 0; kl < nleftstrokes; kl++) {
			if (leftstrokes[kl].pos == cursory) break;
		}
		for (kr = 0; kr < nrightstrokes; kr++) {
			if (rightstrokes[kr].pos == cursory) break;
		}				
		if (kl < nleftstrokes && kr < nrightstrokes) {
			if (leftflag != 0) kr = nrightstrokes;
			else kl = nleftstrokes;
/*			leftflag = 1 - leftflag; */
		}
		if (kl < nleftstrokes) {
			leftstrokes[kl].flag = 1; flag = 1; 
		}
		else if (kr < nrightstrokes) {
			rightstrokes[kr].flag = 1; flag = -1; 
		}
	}		
	return flag;
}

void insert(void) {
	if (recordflag != 0) 	/* don't bother if not recording */
		insertsub();
	showstrokes();
}

int deletesub(void) {
	int k, flag = 0;
	if (cursorx != -INFINITY && cursorx != INFINITY) { 
		for (k = 0; k < nupstrokes; k++) {
			if (upstrokes[k].pos == cursorx &&
				upstrokes[k].flag != 0) {	
					upstrokes[k].flag = 0; flag = +1;
			}
		}
		for (k = 0; k < ndownstrokes; k++) {
			if (downstrokes[k].pos == cursorx &&
				downstrokes[k].flag != 0) {
					downstrokes[k].flag = 0; flag = -1;
			}
		}				
		if (flag != 0) upflag = 1 - (flag + 1)/2;		/* ??? */
	}
	else if(cursory != -INFINITY && cursory != INFINITY) {
		for (k = 0; k < nleftstrokes; k++) {
			if (leftstrokes[k].pos == cursory &&
				leftstrokes[k].flag != 0) {
					leftstrokes[k].flag = 0; flag = +1;
			}
		}
		for (k = 0; k < nrightstrokes; k++) {
			if (rightstrokes[k].pos == cursory &&
				rightstrokes[k].flag != 0) {
					rightstrokes[k].flag = 0; flag = -1;
			}
		}				
		if (flag != 0) leftflag = 1 - (1 + flag)/2;		/* ??? */
	}
	return flag;
}

void delete(void) {
	if (recordflag != 0) 	/* don't bother if not recording */
		deletesub();
	showstrokes();
}

void ghostinsert(void) { 
	int flag;
	if (recordflag != 0) {		/* don't bother if not recording */
	flag = insertsub();			/* turn on stroke at cursor */
	if (flag == 0) return;		/* no cursor ! */
	if (cursorx != -INFINITY && cursorx != INFINITY) {	/* vertical stroke */
		if (flag > 0) { 		/* an up stroke */
			cursorx = cursorx + ghostoff; /*  + 1 */
			recorddownstroke(cursorx, -INFINITY, -INFINITY);
			sortdownstrokes();
			insertsub();
		}
		else if (flag < 0) {	/* a down stroke */
			cursorx = cursorx - ghostoff;
			recordupstroke(cursorx, -INFINITY, -INFINITY);
			sortupstrokes();
			insertsub();
		}
	}
	else if (cursory != -INFINITY && cursory != INFINITY) {	/* horiz stroke */
		if (flag > 0) { 			/* a left stroke */
			cursory = cursory + ghostoff + 1;
			recordrightstroke(cursory, -INFINITY, -INFINITY);
			sortrightstrokes();
			insertsub();
		}
		else if (flag < 0) {		/* a right stroke */
			cursory = cursory - ghostoff;
			recordleftstroke(cursory, -INFINITY, -INFINITY);
			sortleftstrokes();
			insertsub();
		}
	}
	}
	showstrokes(); 
} 

void stemdelete(void) {
	int flag;
	if (recordflag != 0) { 	/* don't bother if not recording */
	flag = deletesub();		/* turn off stroke at cursor */
	if (flag == 0) return;	/* no cursor ! */
	if (cursorx != -INFINITY && cursorx != INFINITY) {  /* vertical stroke */
		if (flag > 0) { 	/* an up stroke */
			rightarrowsub();
			if (deletesub() == 0) {rightarrowsub(); deletesub();}
		}
		else if (flag < 0) { /* a down stroke */
			leftarrowsub();
			if (deletesub() == 0) {leftarrowsub(); deletesub();}
		}
	}
	else if (cursory != -INFINITY && cursory != INFINITY) { /* horiz stroke */
		if (flag > 0) {			/* a left stroke */
			uparrowsub();
			if (deletesub() == 0) {uparrowsub(); deletesub();}
		}
		else if (flag < 0) {	/* a right stroke */
			downarrowsub();
			if (deletesub() == 0) {downarrowsub(); deletesub();}
		}
	}
	}
	showstrokes(); 
} 

int metaleft(void) {
	int k, flag = 0;
	if (cursorx != -INFINITY && cursorx != INFINITY) { 
		suppressflag = 1; showstrokes();
		for (k = 0; k < nupstrokes; k++) {
			if (upstrokes[k].pos == cursorx) {
				cursorx = cursorx - stepscale;	/* showstrokes(); */
				upstrokes[k].pos = cursorx; flag = 1; break;
			}
		}
		for (k = 0; k < ndownstrokes; k++) {
			if (downstrokes[k].pos == cursorx) {
				cursorx = cursorx - stepscale; /* showstrokes();  */
				downstrokes[k].pos = cursorx; flag = -1; break;
			}
		}				
/*		suppressflag = 0; showstrokes(); */
	}
	suppressflag = 0; showstrokes();
	return flag;
}

int metaright(void) {
	int k, flag = 0;
	if (cursorx != -INFINITY && cursorx != INFINITY) { 
		suppressflag = 1; showstrokes();	 /* was before if ... */
		for (k = 0; k < nupstrokes; k++) {
			if (upstrokes[k].pos == cursorx) {
				cursorx = cursorx + stepscale; /* showstrokes(); */
				upstrokes[k].pos = cursorx; flag = 1; break;
			}
		}
		for (k = 0; k < ndownstrokes; k++) {
			if (downstrokes[k].pos == cursorx) {
				cursorx = cursorx + stepscale; /* showstrokes(); */
				downstrokes[k].pos = cursorx; flag = -1; break;
			}
		}				
/*		suppressflag = 0; showstrokes(); */
	}
	suppressflag = 0; showstrokes();
	return flag;
}

int metadown(void) {
	int k, flag = 0;
	if(cursory != -INFINITY && cursory != INFINITY) {
		suppressflag = 1; showstrokes();
		for (k = 0; k < nleftstrokes; k++) {
			if (leftstrokes[k].pos == cursory) {
				cursory = cursory - stepscale; /* showstrokes(); */
				leftstrokes[k].pos = cursory; flag = 1; break;
			}
		}
		for (k = 0; k < nrightstrokes; k++) {
			if (rightstrokes[k].pos == cursory) {
				cursory = cursory - stepscale; /* showstrokes(); */
				rightstrokes[k].pos = cursory; flag = -1; break;
			}
		}				
/*		suppressflag = 0; showstrokes(); */
	}		
	suppressflag = 0; showstrokes();
	return flag;
}

int metaup(void) {
	int k, flag = 0;
	if(cursory != -INFINITY && cursory != INFINITY) {
		suppressflag = 1; showstrokes();
		for (k = 0; k < nleftstrokes; k++) {
			if (leftstrokes[k].pos == cursory) {
				cursory = cursory + stepscale; /* showstrokes(); */
				leftstrokes[k].pos = cursory; flag = 1; break;
			}
		}
		for (k = 0; k < nrightstrokes; k++) {
			if (rightstrokes[k].pos == cursory) {
				cursory = cursory + stepscale; /* showstrokes(); */
				rightstrokes[k].pos = cursory; flag = -1; break;
			}
		}				
/*		suppressflag = 0; showstrokes();  */
	}		
	suppressflag = 0; showstrokes(); 
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* New stuff for tree search approach to optimization */

int vstemoverlap(int i, int j) {	
/* see if stems i and j overlap  (or are too close together) */
	int upstrokei, downstrokei, upstrokej, downstrokej;
	int posupi, posdowni, posupj, posdownj;
	
	upstrokei = vstems[i].sstroke;
	downstrokei = vstems[i].estroke;
	upstrokej = vstems[j].sstroke;
	downstrokej = vstems[j].estroke;

	posupi = upstrokes[upstrokei].pos;
	posdowni = downstrokes[downstrokei].pos;
	posupj = upstrokes[upstrokej].pos;
	posdownj = downstrokes[downstrokej].pos;

/*	printf("UI %d DI %d UJ %d DJ %d ", posupi, posdowni, posupj, posdownj); */

	if (posupi > posdownj + minsep || posupj > posdowni + minsep) return 0;
	else return -1;
}
	
int vcompatible(int k) { 		/* see if can include k-th stem */
	int i, flag = 1;
	
	for (i = 0; i < k; i++) {
		if (vstems[i].flag != 0 && vstemoverlap(i, k) != 0) { 
/* overlaps previous selected stem - so can't include it */
			flag = 0; break;
		}
	}
	return flag;
}

int vpickbest(int k) {			 /* search tree of possible sets of stems */
	int valuewith, valuewithout;
	if (k == numvstems) return 0;	/* hit leaf of search tree */
	if (vcompatible(k) != 0) {		/* can we add this stem ? */
		vstems[k].flag = 1; 
		valuewith = vpickbest(k+1) + vstems[k].value;
		vstems[k].flag = 0;
		valuewithout = vpickbest(k+1);
		if (valuewith > valuewithout) {
			vstems[k].flag = 1;
			vpickbest(k+1);			/* just to reset the flags properly */
			return valuewith;
		}
		else return valuewithout;
	}
	else {							/* can not add this stem */
		vstems[k].flag = 0;
		return vpickbest(k+1);
	}
}

/* weighting function for different stem widths */
/* favours stem widths that are not near the end of the range */
/* piecewise linear:  0 (minstem) - 100 (30) - 100 (70) - 0 (maxstem) */

int weight (int d) {
	if (d >= 30 && d <= 70) return 100;
	else if (d < 30) return ((d - minstem) * 100) / (30 - minstem);
	else if (d > 70) return ((maxstem - d) * 100) / (maxstem - 70);
}

int vstrokeoverlap(int i, int j) { 
/* determine overlap of strokes - negative is they don't */
	int upbottoms, uptops, downbottoms, downtops;	
	
	upbottoms = upstrokes[i].start;
	uptops = upstrokes[i].end; 
	downbottoms = downstrokes[j].end;
	downtops = downstrokes[j].start;

/*	printf("Upbottom = %d, uptops = %d, downbottoms = %d, downtops = %d\n",
		upbottoms, uptops, downbottoms, downtops); */

	if (upbottoms > downtops || uptops < downbottoms) return -1;
	else return (MIN(uptops, downtops) - MAX(upbottoms, downbottoms));
}

int vquality(int i, int j) {	/* compute quality index of potential stem */
	int width, overlap, uplength, downlength;
	int total;
	
	width = downstrokes[j].pos - upstrokes[i].pos;	/* width */
	assert (width > 0);
	overlap = vstrokeoverlap(i, j);			/* overlap of strokes */
	assert (overlap >= 0);
	uplength = upstrokes[i].end - upstrokes[i].start;
	assert (uplength >= 0);
	downlength = downstrokes[j].start - downstrokes[j].end;		
	assert (downlength >= 0);
	
	total = overlap;
	total = total + (uplength + downlength) / 10;
	total = total + weight(width);
	return total;
}

void vrecordstem(int k, int i, int j) { /* record potential stem */
	int val;
	if (k >= MAXSTEMS)  {
		fprintf(stderr, "Too many potential stems\n");
		return;
	}
	val = vquality(i, j);
	vstems[k].sstroke = i;
	vstems[k].estroke = j;
	vstems[k].value = val; 
	vstems[k].flag = 0;						
	if (traceflag != 0) 
		printf("Stem %d - up stroke %d - downstroke %d - value %d\n",
			k, i, j, val);

}

int vmakematches(void) { /* set up potential stems */
	int i, j, k = 0;
	
	for (i = 0; i < nupstrokes; i++) {
		for (j = 0; j < ndownstrokes; j++) {
			if ((upstrokes[i].end - upstrokes[i].start > minstroke ||
				downstrokes[j].start - downstrokes[j].end > minstroke) &&
				upstrokes[i].pos + minstem < downstrokes[j].pos &&
				downstrokes[j].pos < upstrokes[i].pos + maxstem &&
/*				vstrokeoverlap(i, j) >= 0) { */
				vstrokeoverlap(i, j) > 0) { 
				vrecordstem(k, i, j);
				k++;
				if (k >= MAXSTEMS)  {
					fprintf(stderr, "Too many potential stems\n");
					break;
				}
			}
		}
		if (k >= MAXSTEMS) break;
	}
	numvstems = k;
	return numvstems;
}

void vshowsolution(void) {
	int k, upi, downj;
	
	for (k = 0; k < numvstems; k++) {
		if (vstems[k].flag != 0) {
			printf("Stem %d included with value %d\n", k, vstems[k].value);
			upi = vstems[k].sstroke;
			downj = vstems[k].estroke;
			printf("\tUp stroke %d and down stroke %d\n", upi, downj);
			printf("\tUp stroke at x = %d, from y = %d to y = %d\n",
				upstrokes[upi].pos, upstrokes[upi].start, 
					upstrokes[upi].end);
			printf("\tDown stroke at x = %d, from y = %d to y = %d\n",
				downstrokes[downj].pos, downstrokes[downj].end, 
					downstrokes[downj].start);
		}
	}
}

/* now for horizontal version */

int hstemoverlap(int i, int j) {	
/* see if stems i and j overlap  - or are too close together */
	int leftstrokei, rightstrokei, leftstrokej, rightstrokej;
	int poslefti, posrighti, posleftj, posrightj;
	
	leftstrokei = hstems[i].sstroke;
	rightstrokei = hstems[i].estroke;
	leftstrokej = hstems[j].sstroke;
	rightstrokej = hstems[j].estroke;

	poslefti = leftstrokes[leftstrokei].pos;
	posrighti = rightstrokes[rightstrokei].pos;
	posleftj = leftstrokes[leftstrokej].pos;
	posrightj = rightstrokes[rightstrokej].pos;

/*	printf("LI %d RI %d LJ %d RJ %d ", 	poslefti, posrighti, posleftj, posrightj); */

	if (poslefti > posrightj + minsep || posleftj > posrighti + minsep) 
		return 0;
	else return -1;
}
	
int hcompatible(int k) { 		/* see if can include k-th stem */
	int i, flag = 1;
	
	for (i = 0; i < k; i++) {
		if (hstems[i].flag != 0 && hstemoverlap(i, k) != 0) { 
/* overlaps previous selected stem - so can't include it */
			flag = 0; break;
		}
	}
	return flag;
}

int hpickbest(int k) {			 /* search tree of possible sets of stems */
	int valuewith, valuewithout;
	if (k == numhstems) return 0;	/* hit leaf of search tree */
	if (hcompatible(k) != 0) {		/* can we add this stem ? */
		hstems[k].flag = 1; 
		valuewith = hpickbest(k+1) + hstems[k].value;
		hstems[k].flag = 0;
		valuewithout = hpickbest(k+1);
		if (valuewith > valuewithout) {
			hstems[k].flag = 1;
			hpickbest(k+1);			/* just to reset the flags properly */
			return valuewith;
		}
		else return valuewithout;
	}
	else {							/* can not add this stem */
		hstems[k].flag = 0;
		return hpickbest(k+1);
	}
}

int hstrokeoverlap(int i, int j) { 
/* determine overlap of strokes - negative is they don't */
	int leftrights, leftlefts, rightrights, rightlefts;	
	
	leftrights = leftstrokes[i].start;
	leftlefts = leftstrokes[i].end; 
	rightrights = rightstrokes[j].end;
	rightlefts = rightstrokes[j].start;

/*	printf("leftbottom = %d, leftlefts = %d, rightrights = %d, rightlefts = %d\n",
		leftrights, leftlefts, rightrights, rightlefts); */

/*	printf ("YL %d LL %d LR %d YR %d RL %d RR %d \n", 
				leftsrokes[i].pos,	leftlefts, leftrights, 
					rightstrokes[j].pos, rightlefts, rightrights); */
	if (leftrights < rightlefts || leftlefts > rightrights) return -1;
	else return (MIN(leftrights, rightrights) - MAX(leftlefts, rightlefts));
}

int hquality(int i, int j) {	/* compute quality index of potential stem */
	int height, overlap, leftlength, rightlength;
	int total;
	
	height = rightstrokes[j].pos - leftstrokes[i].pos; /* height */
	assert (height > 0);
	overlap = hstrokeoverlap(i, j);			/* overlap of strokes */
	assert (overlap >= 0);
	leftlength = leftstrokes[i].start - leftstrokes[i].end;
	assert (leftlength >= 0);
	rightlength = rightstrokes[j].end - rightstrokes[j].start;
	assert (rightlength >= 0);
	
	total = overlap;
	total = total + (leftlength + rightlength) / 10;
	total = total + weight(height);
	return total;
}

void hrecordstem(int k, int i, int j) { /* record potential stem */
	int val;
	if (k >= MAXSTEMS)  {
		fprintf(stderr, "Too many potential stems\n");
		return;
	}
	val = hquality(i, j);
	hstems[k].sstroke = i;
	hstems[k].estroke = j;
	hstems[k].value = val;
	hstems[k].flag = 0;					
	if (traceflag != 0) 
		printf("Stem %d - left stroke %d - right stroke %d - value %d\n",
			k, i, j, val);

}

int hmakematches(void) { /* set up potential stems */
	int i, j, k = 0;
	
	for (i = 0; i < nleftstrokes; i++) {
		for (j = 0; j < nrightstrokes; j++) {
			if ((leftstrokes[i].start - leftstrokes[i].end > minstroke ||
				rightstrokes[j].end - rightstrokes[j].start > minstroke) &&
				leftstrokes[i].pos + minstem < rightstrokes[j].pos &&
				rightstrokes[j].pos < leftstrokes[i].pos + maxstem &&
/*				hstrokeoverlap(i, j) >= 0) { */
				hstrokeoverlap(i, j) > 0) { 
				hrecordstem(k, i, j);
				k++;
				if (k >= MAXSTEMS)  {
					fprintf(stderr, "Too many potential stems\n");
					break;
				}
			}
		}
		if (k >= MAXSTEMS) break;
	}
	numhstems = k;
	return numhstems;
}

void hshowsolution(void) {
	int k, lefti, rightj;
	
	for (k = 0; k < numhstems; k++) {
		if (hstems[k].flag != 0) {
			printf("Stem %d included with value %d\n", k, hstems[k].value);
			lefti = hstems[k].sstroke;
			rightj = hstems[k].estroke;
			printf("\tleft stroke %d and right stroke %d\n", lefti, rightj);
			printf("\tleft stroke at x = %d, from y = %d to y = %d\n",
				leftstrokes[lefti].pos, leftstrokes[lefti].start, 
					leftstrokes[lefti].end);
			printf("\tright stroke at x = %d, from y = %d to y = %d\n",
				rightstrokes[rightj].pos, rightstrokes[rightj].end, 
					rightstrokes[rightj].start);
		}
	}
}

void flagstrokes() { /* flag those strokes that are in flagged stems */
	int k, sl, sr, su, sd;
	for (k = 0; k < numhstems; k++) {
		if (hstems[k].flag != 0) {
			sl = hstems[k].sstroke;
			sr = hstems[k].estroke;
			leftstrokes[sl].flag = 1;
			rightstrokes[sr].flag = 1;
		}
	}
	for (k = 0; k < numvstems; k++) {
		if (vstems[k].flag != 0) {
			su = vstems[k].sstroke;
			sd = vstems[k].estroke;
			upstrokes[su].flag = 1;
			downstrokes[sd].flag = 1;
		}
	}
}

/* heuristics for ghost stems at top of sub-character */

int checktop(int leftlow, int lefthigh, int rightlow, int righthigh) { 
	int yl, yr, ytop;
	int kl = lefthigh-1, kr = righthigh-1; 

	if (righthigh == rightlow) return 0;			/* no right strokes ! */
	if (rightstrokes[kr].flag != 0) return 0;		/*  already flagged */
	ytop = rightstrokes[kr].pos;
	while(rightstrokes[kr].pos == ytop && kr >= rightlow) {
		if (rightstrokes[kr].flag != 0) return 0;	/* already flagged */
		kr--; 
	}
	if (kl < leftlow) yl = -INFINITY;	else yl = leftstrokes[kl].pos;
	while(rightstrokes[kr].flag == 0 && kr >= rightlow) kr--;	/* NEW */
	if (kr < rightlow) yr = -INFINITY;	else yr = rightstrokes[kr].pos;
	if (yr >= yl) {			/* next lower (marked) stroke is also right */
		if (yr >= ytop - minstem * 2) return 0;	/* too little space tweaked */
		assert(rightstrokes[righthigh-1].flag == 0);
		rightstrokes[righthigh-1].flag = -1;  
		insertleftstroke(ytop - ghostoff, -INFINITY, -INFINITY, lefthigh);
		assert(leftstrokes[lefthigh].flag == 0);
		leftstrokes[lefthigh].flag = -1;	/* and flag it */
		return 1;
	}
	else if (yl <= ytop - maxstem/2 || /* too wide for stem - tweaked */
		leftstrokes[kl].start == leftstrokes[kl].end || /* extremum only */
		leftstrokes[kl].flag != 0	/* opposite stroke already in use ! */
		) { 
		assert(rightstrokes[righthigh-1].flag == 0);
		rightstrokes[righthigh-1].flag = -1; 
		insertleftstroke(ytop - ghostoff, -INFINITY, -INFINITY, lefthigh);
		assert(leftstrokes[lefthigh].flag == 0);
		leftstrokes[lefthigh].flag = -1;	/* and flag it */
		return 1;
	}
	else {							/* mark top stem */
		assert(rightstrokes[righthigh-1].flag == 0);
		if(rightstrokes[righthigh-1].pos <= leftstrokes[kl].pos) {
			if (batchmode != 0) fprintf(stderr, "CHAR: %d ", chrs);
			if (!quietflag)
			fprintf(stderr, "TOP STROKES: RIGHT %d <= LEFT %d ",
				rightstrokes[righthigh-1].pos, leftstrokes[kl].pos);
			if (batchmode != 0) putc('\n', stderr);
			return 0;
		}
		assert(rightstrokes[righthigh-1].pos > leftstrokes[kl].pos); /* ? */
		assert(kl >= leftlow && kl < lefthigh);
		assert(leftstrokes[kl].flag == 0);
		rightstrokes[righthigh-1].flag = -1; 
		leftstrokes[kl].flag = -1;
		return 0;							/* nothing new inserted */
	}
}

/* heuristic for ghost stems at bottom of sub-character */

int checkbottom(int leftlow, int lefthigh, int rightlow, int righthigh) { 
	int yl, yr, ybottom;
	int kl = leftlow, kr = rightlow; 

	if (lefthigh == leftlow) return 0;			/* no left strokes ! */
	if (leftstrokes[kl].flag != 0) return 0;	/* already flagged */
	ybottom = leftstrokes[kl].pos;
	while (leftstrokes[kl].pos == ybottom && kl < lefthigh) {
		if (leftstrokes[kl].flag != 0) return 0; /* already flagged */
		kl++; 
	}
	if (kr >= righthigh) yr = INFINITY; else yr = rightstrokes[kr].pos;
	while (leftstrokes[kl].flag == 0 && kl < lefthigh) kl++; /* NEW */
	if (kl >= lefthigh) yl = INFINITY;	else yl = leftstrokes[kl].pos;
	if (yl <= yr) {			/* next higher (marked) stroke is also left */
		if (yl <= ybottom + minstem * 2) return 0;	/* too little space */
		assert(leftstrokes[leftlow].flag == 0);
		leftstrokes[leftlow].flag = -1;
		insertrightstroke(ybottom + ghostoff + 1, -INFINITY, -INFINITY, 
			rightlow);
		assert(rightstrokes[rightlow].flag == 0);
		rightstrokes[rightlow].flag = -1; /* ? */
		return 1;
	}
	else if (yr >= ybottom + maxstem/2 || /* too wide for stem - tweaked */
		rightstrokes[kr].start == rightstrokes[kr].end || /* extremum only */
		rightstrokes[kr].flag != 0		/* opposite stroke already in use ! */
		) {
		assert(leftstrokes[leftlow].flag == 0);
		leftstrokes[leftlow].flag = -1;
		insertrightstroke(ybottom + ghostoff + 1, -INFINITY, -INFINITY, 
			rightlow);
		assert(rightstrokes[rightlow].flag == 0);
		rightstrokes[rightlow].flag = -1; /* ? */
		return 1;
	}
	else {						/* mark bottom stem */
		assert(leftstrokes[leftlow].flag == 0);
		if(leftstrokes[leftlow].pos >= rightstrokes[kr].pos) {
			if (batchmode != 0) fprintf(stderr, "CHAR %d ", chrs);
			if (!quietflag)
			fprintf(stderr, "BOTTOM STROKES: LEFT %d <= RIGHT %d ",
				leftstrokes[leftlow].pos, rightstrokes[kr].pos);
			if (batchmode != 0) putc('\n', stderr);
			return 0;
		}
		assert(leftstrokes[leftlow].pos < rightstrokes[kr].pos); /* ? */
		assert(kr < righthigh && kr >= rightlow);
		assert(rightstrokes[kr].flag == 0);
		leftstrokes[leftlow].flag = -1;
		rightstrokes[kr].flag = -1;
		return 0;				/* nothing new inserted */
	}
}

/* see whether horizontal centering called for */ 
int checkcentering(int uplow, int uphigh, int downlow, int downhigh) {
	int k, width;

/* don't bother if character looks at all complex ... */
	if (uphigh - uplow > MAXCOMPLEX || downhigh - downlow > MAXCOMPLEX ||
		nleftstrokes > MAXCOMPLEX || nrightstrokes > MAXCOMPLEX) return 0;

/* must have at least one stroke (excluding origin and char width stroke) */
/* 	if (uphigh < uplow + 2 || downhigh < downlow + 2) return 0; */
	if (uphigh <= uplow || downhigh <= downlow) return 0;

/* don't try this if any of the up or down strokes are flagged */
	for (k = uplow; k < uphigh; k++) 
		if (upstrokes[k].flag != 0) return 0;
	for (k = downlow; k < downhigh; k++) 
		if (downstrokes[k].flag != 0) return 0;

	width = downstrokes[downhigh-1].pos - upstrokes[uplow].pos;
/*	if (width > maxstem * 4) return 0;	*/ /* heuristic max width - NO ? */
/*	if (upstrokes[uplow].pos == 0) {
		printf("Trying to use x=0 stroke in char %d\n", chrs);
		return 0; 
	} */
/*	if (downstrokes[downhigh-1].pos == iwidth) {
		printf("Trying to use x=%d stroke in char %d\n", iwidth, chrs);
		return 0; 
	} */
	assert(downstrokes[downhigh-1].flag == 0);
	downstrokes[downhigh-1].flag = -1;
	assert(upstrokes[uplow].flag == 0);
	upstrokes[uplow].flag = -1;
	return -1;
}

/* following is for coincidence of horizontal strokes */
/* do we want this for vertical strokes also ? */

/* remove stems on coincident strokes 
void checkcoincident(void) { 
	int kl=0, kr=0, kld, krd, yl, yr;

	for(;;) {
		yl = leftstrokes[kl].pos;
		yr = rightstrokes[kr].pos;
		if (yl == yr) {
			if (leftstrokes[kl].flag != 0) {
				assert(rightstrokes[kr].flag == 0);
				leftstrokes[kl].flag = 0;
				krd = kr;
				while(rightstrokes[krd].flag == 0 && krd < nrightstrokes) 
					krd++;
				assert(krd < nrightstrokes);
				rightstrokes[krd].flag = 0;
			}
			if (rightstrokes[kr].flag != 0) {
				assert(leftstrokes[kl].flag == 0);
				rightstrokes[kr].flag = 0;
				kld = kl;
				while(leftstrokes[kld].flag == 0 && kld >= 0) kld--;
				assert(kld >= 0);
				leftstrokes[kld].flag = 0;
			}
			kl++; kr++;
		}
		else if (yl < yr) kl++;
		else if (yl > yr) kr++;
		if (kl == nleftstrokes || kr == nrightstrokes) break;
	}
} */

void guessvhints(void) {
	int val;
	if (traceflag != 0) {printf("VMAKEMATCHES "); getsafe();}
	vmakematches();
	if (traceflag != 0) {printf("VPICKBEST "); getsafe();}
	if (numvstems > 10) return;		/* trial - avoid blowout */
	val = vpickbest(0); 
	if (traceflag != 0) 	printf(" %d ", val);
	if (traceflag != 0) 	vshowsolution();
}

void guesshhints(void) {
	int val;
	if (traceflag != 0) {printf("HMAKEMATCHES "); getsafe();}
	hmakematches();
	if (traceflag != 0) {printf("HPICKBEST "); getsafe();}
	if (numhstems > 10) return;		/* trial - avoid blowout */
	val = hpickbest(0);
	if (traceflag != 0) 	printf(" %d ", val);
	if (traceflag != 0) 	hshowsolution();
}

void checkghosts(void) {
	int k, l, action, ystart, yend;
	int leftstart, leftend, rightstart, rightend;

/*	step through yranges of different sub-characters */
	for (k=0; k < nyrange; k++) {
		ystart = yranges[k].start; yend = yranges[k].end;
/*		for each yrange, find appropriate part of stroke arrays */
		leftstart = nleftstrokes-1;
		for (l=0; l < nleftstrokes; l++) {
			if (leftstrokes[l].pos >= ystart) {
				leftstart = l; break;
			}
		}
		leftend = 0;
		for (l=nleftstrokes-1; l >=0; l--) {
			if (leftstrokes[l].pos <= yend) {
				leftend = l; break;
			}
		}		
		rightstart = nrightstrokes-1;
		for (l=0; l < nrightstrokes; l++) {
			if (rightstrokes[l].pos >= ystart) {
				rightstart = l; break;
			}
		}
		rightend = 0;
		for (l=nrightstrokes-1; l >=0; l--) {
			if (rightstrokes[l].pos <= yend) {
				rightend = l; break;
			}
		}		
/*	check for possible ghost at top */
/*		checktop(0, nleftstrokes, 0, nrightstrokes); OLD */
 		action = checktop(leftstart, leftend+1, rightstart, rightend+1);  
		if (action != 0) leftend = leftend + 1;		/* new */
/*	check for possible ghost at bottom */
/*		checkbottom(0, nleftstrokes, 0, nrightstrokes); OLD */
		action = checkbottom(leftstart, leftend+1, rightstart, rightend+1);  
		if (action != 0) rightend = rightend + 1;	/* new */
	}
	return;
}

void checkcenter(void) {
	int k, l, xstart, xend;
	int upstart, upend, downstart, downend;	
	
/*	see if centering makes sense */
/*  leave out up stroke on left and downstroke on right */
	for (k=0; k < nxrange; k++) {
		xstart = xranges[k].start; xend = xranges[k].end;
/*		for each xrange, find appropriate part of stroke arrays */
		upstart = nupstrokes-1;
		for (l=0; l < nupstrokes; l++) {
			if (upstrokes[l].pos >= xstart) {
				upstart = l; break;
			}
		}
		upend = 0;
		for (l=nupstrokes-1; l >=0; l--) {
			if (upstrokes[l].pos <= xend) {
				upend = l; break;
			}
		}		
		downstart = ndownstrokes-1;
		for (l=0; l < ndownstrokes; l++) {
			if (downstrokes[l].pos >= xstart) {
				downstart = l; break;
			}
		}
		downend = 0;
		for (l=ndownstrokes-1; l >=0; l--) {
			if (downstrokes[l].pos <= xend) {
				downend = l; break;
			}
		}		
		checkcentering(upstart, upend+1, downstart, downend+1); 
	}
}

void guesshints(void) {
	if (italicchar() == 0) guessvhints(); 
	else numvstems = 0; 
	if (calligraphic() == 0) guesshhints();
	else numhstems = 0;
	flagstrokes();		/* transfer flags from stems to strokes */
/*	printf("XR %d ", nxrange);
	for (k=0; k < nxrange; k++) 
		printf("(%d %d) ", xranges[k].start, xranges[k].end); */
/*	printf("YR %d ", nyrange);
	for (k=0; k < nyrange; k++) 
		printf("(%d %d) ", yranges[k].start, yranges[k].end); */
	if (avoidhair != 0) return;
	checkghosts();
	checkcenter();
/*	checkcoincident();	*/ /* flush stems with edges on coincident strokes */
}

int leftaction(void) {	/* see if any leftstrokes */
	int k;
	for (k = 0; k < nleftstrokes; k++) 
		if (leftstrokes[k].flag != 0) return -1;
	return 0;
}

int rightaction(void) {	/* see if any rightstrokes */
	int k;
	for (k = 0; k < nrightstrokes; k++) 
		if (rightstrokes[k].flag != 0) return -1; 
	return 0;
}

int upaction(void) {	/* see if any upstrokes */
	int k;
	for (k = 0; k < nupstrokes; k++) 
		if (upstrokes[k].flag != 0) return -1;
	return 0;
}

int downaction(void) {	/* see if any downstrokes */
	int k;
	for (k = 0; k < ndownstrokes; k++) 
		if (downstrokes[k].flag != 0) return -1;
	return 0;
}

int vstem3test(FILE *fp_out) {		/* see if fits vstem3 pattern */
	int k, kup=0, kdown=0;
	int m, a, b, c, dx;
	int xup[3], xdown[3];

	if (stem3flag == 0) return 0;
	for (k = 0; k < nupstrokes; k++) {
		if (upstrokes[k].flag != 0) {
			if (kup >= 3) return 0;
			xup[kup] = upstrokes[k].pos;	kup++;
		}
	}
	if (kup != 3) return 0;
	for (k = 0; k < ndownstrokes; k++) {
		if (downstrokes[k].flag != 0) {
			if (kdown >= 3) return 0;
			xdown[kdown] = downstrokes[k].pos;	kdown++;
		}
	}
	if (kdown != 3) return 0;
	m = (xup[0] + xup[1] + xup[2] + xdown[0] + xdown[1] + xdown[2] + 3);
	a = (xdown[1] - xup[1]) * 3;
	b = (xup[2] - xdown[0]) * 3;
	c = (xdown[2] - xup[0]) * 3;
	if (traceflag != 0) {
		printf("m %d a %d b %d c %d ", m, a, b, c); getsafe(); 
	}
	dx = xdown[1] * 6 - (m + a);	if (ABS(dx) > stem3eps) return 0;
	dx = xup[1] * 6 - (m - a);		if (ABS(dx) > stem3eps) return 0;
	dx = xup[2] * 6 - (m + b);		if (ABS(dx) > stem3eps) return 0;
	dx = xdown[0] * 6 - (m - b);	if (ABS(dx) > stem3eps) return 0;
	dx = xdown[2] * 6 - (m + c);	if (ABS(dx) > stem3eps) return 0;	
	dx = xup[0] * 6 - (m - c);		if (ABS(dx) > stem3eps) return 0;	
	fprintf(fp_out, "M %d %d %d %d %d %d ",
		xup[0], xdown[0], xup[1], xdown[1], xup[2], xdown[2]);
	return -1;
}

int hstem3test(FILE *fp_out) {		/* see if fits hstem3 pattern */
	int k, kleft=0, kright=0;
	int m, a, b, c, dy;
	int yleft[3], yright[3];

	if (stem3flag == 0) return 0;
	for (k = 0; k < nleftstrokes; k++) {
		if (leftstrokes[k].flag != 0) {
			if (kleft >= 3) return 0;
			yleft[kleft] = leftstrokes[k].pos;	kleft++;
		}
	}
	if (kleft != 3) return 0;
	for (k = 0; k < nrightstrokes; k++) {
		if (rightstrokes[k].flag != 0) {
			if (kright >= 3) return 0;
			yright[kright] = rightstrokes[k].pos;	kright++;
		}
	}
	if (kright != 3) return 0;
/*	don't consider hstem3 if ghost stem on either end ??? */
	dy = yright[0] - yleft[0];
	if (dy == 20 || dy ==21) return 0;	
	dy = yright[2] - yleft[2];
	if (dy == 20 || dy ==21) return 0;	
/*  compute rounded middle of the three */
	m = (yleft[0] + yleft[1] + yleft[2] + yright[0] + yright[1] + yright[2] + 3);
	a = (yright[1] - yleft[1]) * 3;
	b = (yleft[2] - yright[0]) * 3;
	c = (yright[2] - yleft[0]) * 3;
	if (traceflag != 0) {
		printf("m %d a %d b %d c %d ", m, a, b, c); getsafe(); 
	}
	dy = yright[1] * 6 - (m + a);	if (ABS(dy) > stem3eps) return 0;
	dy = yleft[1] * 6 - (m - a);	if (ABS(dy) > stem3eps) return 0;
	dy = yleft[2] * 6 - (m + b);	if (ABS(dy) > stem3eps) return 0;
	dy = yright[0] * 6 - (m - b);	if (ABS(dy) > stem3eps) return 0;
	dy = yright[2] * 6 - (m + c);	if (ABS(dy) > stem3eps) return 0;	
	dy = yleft[0] * 6 - (m - c);	if (ABS(dy) > stem3eps) return 0;	
	fprintf(fp_out, "E %d %d %d %d %d %d ",
		yleft[0], yright[0], yleft[1], yright[1], yleft[2],	yright[2]); 
	return -1;
}

/****************************************************************************/

void seekerror (char *msg, char *str) {
	fprintf(stderr, "SEEK ERROR IN %s ON %s ", msg, str);
}

/****************************************************************************/

/* long charstart=-1; */ /* make global, so keeps start of character ??? */

int writestrokes(FILE *fp_out) { /* write edited hints to file */
	int ku, kd, kl, kr, xu, xd, yl, yr;
	int hact, vact, conflag = -1;
	int kln, krn, yln, yrn;
	int kun, kdn, xun, xdn;
	long charstart=-1; 

	if (recordflag == 0) return -1; 	/* don't bother if not recording */
	hact = 0;
	if (leftaction() != 0 || rightaction() != 0) hact = -1; 
/*	if (leftaction() != 0 && rightaction() != 0) hact = -1; */
	vact = 0;
	if (upaction() != 0 || downaction() != 0) vact = -1; 
/*	if (upaction() != 0 && downaction() != 0) vact = -1; */
	if (hact == 0 && vact == 0) return -1;

	if (ferror(fp_out) != 0) {
		perror("Stem Output File");
	}
/*	remember start of character in output hint file ??? */
/*	if (fsubr == 0) */ /* experiment 1996/June/16 ??? */
		charstart = ftell(fp_out);	/* remember this place, in case of error */
/*	if (fsubr > 0) */
	if (fsubr + subroffset > 0)
/*		 fprintf(fp_out, "S %d ; ", fsubr); */		/* 1994/May/21 */
		 fprintf(fp_out, "S %d ; ", fsubr + subroffset);	/* 1994/June/5 */

	fprintf(fp_out, "C %d ; ", chrs);
	if (hact != 0) {
		if (hstem3test(fp_out) == 0) {
			fprintf(fp_out, "H ");

/*			kl = 0; kr = 0; yl = -INFINITY; yr = -INFINITY;
			while (kl < nleftstrokes && kr < nrightstrokes) {
				while (leftstrokes[kl].flag == 0 && kl < nleftstrokes) kl++;
				if (kl != nleftstrokes) yl = leftstrokes[kl].pos;
				else yl = -INFINITY;
				while (rightstrokes[kr].flag == 0 && kr < nrightstrokes) kr++;
				if (kr != nrightstrokes) yr = rightstrokes[kr].pos;
				else yr = -INFINITY;
				if (kl == nleftstrokes && kr == nrightstrokes) break; 
				if (kl == nleftstrokes || kr == nrightstrokes) {
					conflag = 0; break;
				}
				if (yr > yl) {
					if (ignoresizes != 0 ||
						(yr > yl + minstem && yr < yl + maxstem))
							fprintf(fp_out, "%d %d ", yl, yr);
					else conflag = 0;
				}
				else conflag = 0; 
				kl++; kr++;
			} */

	kl = 0; kr = 0; /* yl = -INFINITY; yr = -INFINITY;  */
/* find first marked left stroke - if any */
	while (leftstrokes[kl].flag == 0 && kl < nleftstrokes) kl++;		
	if (kl != nleftstrokes) yl = leftstrokes[kl].pos;
	else yl = INFINITY;
/* find first marked right stroke that is higher */
	for(;;) {
		while (rightstrokes[kr].flag == 0 && kr < nrightstrokes) kr++;
		if (kr != nrightstrokes) yr = rightstrokes[kr].pos;
		else yr = INFINITY;
		if (yr >= yl) break;
		else conflag = 0;	/* first right stroke lower than first left */
		kr++;
	}
/* now pick up all stems */
	for(;;) {
/* have we run out of matching strokes ? */
		if (kl == nleftstrokes && kr == nrightstrokes) break;
		if (kl == nleftstrokes || kr == nrightstrokes) {
			conflag = 0; break;
		}
		kln = kl + 1;
/* move up left stroke until next left one is above present right stroke */
		for (;;) {
			while (leftstrokes[kln].flag == 0 && kln < nleftstrokes) kln++;
			if (kln != nleftstrokes) yln = leftstrokes[kln].pos;
			else yln = INFINITY;
			if (yln >= yr) break;
			else conflag = 0; /* next left stroke is below next right */
			kl = kln; yl = yln;
			kln++;
		}
/* write stem between yl and yr */
		if (ignoresizes != 0 ||
			(yr > yl + minstem && yr < yl + maxstem))
				fprintf(fp_out, "%d %d ", yl, yr);
		else conflag = 0;
		krn = kr + 1;
/* find the next right stroke above new left stroke */
		for(;;) {
			while (rightstrokes[krn].flag == 0 && krn < nrightstrokes) krn++;
			if (krn != nrightstrokes) yrn = rightstrokes[krn].pos;
			else yrn = INFINITY;
			if (yrn >= yln) break;
			conflag = 0; /* next right stroek not above next left */
			krn++;
		}
		kl = kln; kr = krn; yl = yln; yr = yrn;
	}

		}
		fprintf(fp_out, "; ");
	}
	if (vact != 0) {
		if (vstem3test(fp_out) == 0) {
			fprintf(fp_out, "V ");

/*			ku = 0; kd = 0; xu = -INFINITY; xd = -INFINITY;
			while (ku < nupstrokes && kd < ndownstrokes) {
				while (upstrokes[ku].flag == 0 && ku < nupstrokes) ku++;
				if (ku != nupstrokes) xu = upstrokes[ku].pos;
				else xu = -INFINITY;
				while (downstrokes[kd].flag == 0 && kd < ndownstrokes) kd++;
				if (kd != ndownstrokes) xd = downstrokes[kd].pos;
				else xd = -INFINITY;
				if (ku == nupstrokes && kd == ndownstrokes) break; 
				if (ku == nupstrokes || kd == ndownstrokes) {
					conflag = 0; break; 
				}
				if (xd > xu) {
					if (ignoresizes != 0 ||
						(xd > xu + minstem && xd < xu + maxstem))
							fprintf(fp_out, "%d %d ", xu, xd);
					else conflag = 0;
				}
				else conflag = 0;
				ku++; kd++;
			} */

	ku = 0; kd = 0; /* xu = -INFINITY; xd = -INFINITY;  */
/* find first marked up stroke - if any */
	while (upstrokes[ku].flag == 0 && ku < nupstrokes) ku++;		
	if (ku != nupstrokes) xu = upstrokes[ku].pos;
	else xu = INFINITY;
/* find first marked down stroke that is higher */
	for(;;) {
		while (downstrokes[kd].flag == 0 && kd < ndownstrokes) kd++;
		if (kd != ndownstrokes) xd = downstrokes[kd].pos;
		else xd = INFINITY;
		if (xd >= xu) break;
		else conflag = 0;
		kd++;
	}
/* now pick up all stems */
	for(;;) {
/* have we run out of matching strokes ? */
		if (ku == nupstrokes && kd == ndownstrokes) break;
		if (ku == nupstrokes || kd == ndownstrokes) {
			conflag = 0; break;
		}
		kun = ku + 1;
/* move up stroke right until next up is right of present down stroke */
		for (;;) {
			while (upstrokes[kun].flag == 0 && kun < nupstrokes) kun++;
			if (kun != nupstrokes) xun = upstrokes[kun].pos;
			else xun = INFINITY;
			if (xun >= xd) break;
			else conflag = 0;
			ku = kun; xu = xun;
			kun++;
		}
/* show stem between xu and xd */
		if (ignoresizes != 0 ||
			(xd > xu + minstem && xd < xu + maxstem))
				fprintf(fp_out, "%d %d ", xu, xd);
		else conflag = 0;

		kdn = kd + 1;
/* find the next down stroke to right of new up stroke */
		for(;;) {
			while (downstrokes[kdn].flag == 0 && kdn < ndownstrokes) kdn++;
			if (kdn != ndownstrokes) xdn = downstrokes[kdn].pos;
			else xdn = INFINITY;
			if (xdn >= xun) break;
			else conflag = 0;
			kdn++;
		}
		ku = kun; kd = kdn; xu = xun; xd = xdn;
	}


		}
		fprintf(fp_out, "; ");
	}
	putc('\n', fp_out);
	if (conflag == 0) {	/* inconsistent stem data - complain and back up */
		putc('\a', stderr); /* scanmode = 0; automode = 0; */
		if (batchmode != 0) {
			fprintf(stderr, "Inconsistent stems in char %d\n", chrs);
			fprintf(stderr, "Type F-3 and PgDn or ");
			fprintf(stderr, "type C-F-1 and PgDn to proceed\n");
		}
		if (charstart < 0) {
			seekerror("writestrokes", "fp_out"); 
			fputs("% POSSIBLE PROBLEM HERE DUE TO HINT OVERLAP\n", fp_out);
		}
		else fseek(fp_out, charstart, SEEK_SET);
	}
	return conflag;
}

/* Approx sqrt of sum of squares  for range x > 0, y > 0, x > y */
/* r approx a * x + b * x, a = 2/(1+sqrt(4-2*sqrt(2))), b = (sqrt(2)-1)*a */
/* return (int) sqrt ((double) x * (double) x + (double) y * (double) y); */

/*
int radius(int x, int y) {
	int t;
	if (x < 0) x = -x;
	if (y < 0) y = -y;
	if (x < y) {t = x; x = y; y = t;}
	return (int) (((long) x * 960l + (long) y * 398l) / 1000l);
} */

/* int radius(int x, int y) {
	double fx = (double) x, fy = (double) y;
	return (int) sqrt( fx * fx + fy * fy );
} */

short radius(short x, short y) {
	double fx = (double) x, fy = (double) y;
	return (short) sqrt( fx * fx + fy * fy );
}

#define POLY(a, b, c, d, ft) ((((a) * (ft) + (b)) * (ft) + (c)) * (ft) + (d))

/* int round(double x) {
	if (x < 0) return (- (round (- x)));
	else return (int) (x + 0.5);
} */

short round(double x) {
	if (x < 0) return (- (round (- x)));
	else return (short) (x + 0.5);
}

/* old version */

/* void drawbezier(int x0, int y0, int x1, int y1, 
				int x2, int y2, int x3, int y3) { */
void drawbezier(short x0, short y0, short x1, short y1, 
				short x2, short y2, short x3, short y3) {
/*	 int xmin, xmax, ymin, ymax; */
  	 short xmin, xmax, ymin, ymax;
	 double fx0, fy0, fx1, fy1, fx2, fy2, fx3, fy3;
	 double ax, bx, cx, ay, by, cy;
	 double ft, fx, fy;
	 int k, n;
	 
	 xmin = x0, xmax = x0; ymin = y0; ymax = y0;
	 if (x1 < xmin) xmin = x1; 	 if (y1 < ymin) ymin = y1;
	 if (x2 < xmin) xmin = x2; 	 if (y2 < ymin) ymin = y2;
	 if (x3 < xmin) xmin = x3; 	 if (y3 < ymin) ymin = y3;
	 if (x1 > xmax) xmax = x1; 	 if (y1 > ymax) ymax = y1;
	 if (x2 > xmax) xmax = x2; 	 if (y2 > ymax) ymax = y2;
	 if (x3 > xmax) xmax = x3; 	 if (y3 > ymax) ymax = y3;
	 n = (xmax - xmin +1) + (ymax - ymin + 1);
	 n = (n - 1) / 4 + 1;
	 if (n < 2) n = 2;

	 fx0 = (double) x0; fy0 = (double) y0;
	 fx1 = (double) x1; fy1 = (double) y1;
	 fx2 = (double) x2; fy2 = (double) y2;
	 fx3 = (double) x3; fy3 = (double) y3;
	 cx = (fx1 - fx0) * 3.0; 
	 cy = (fy1 - fy0) * 3.0;
	 bx = ((fx2 - fx1) - (fx1 - fx0)) * 3.0; 
	 by = ((fy2 - fy1) - (fy1 - fy0)) * 3.0; 
	 ax = (fx3 - fx0) + 3.0 * (fx1 - fx2);
	 ay = (fy3 - fy0) + 3.0 * (fy1 - fy2);
	 
	 _moveto(x0, y0);
	 for (k = 1; k < n; k++) {
		 ft = (double) k / (double) n;
		 fx = POLY(ax, bx, cx, fx0, ft);
		 fy = POLY(ay, by, cy, fy0, ft);
		 _lineto (round(fx), round(fy));
	 }
	 _lineto(x3, y3);
} 

/* int strepsilon = 1.0;

int straightcurve(double x0, double y0, double x1, double y1, 
				double x2, double y2, double x3, double y3, double epsilon) {
	double dx, dy, len;
	double cros, rho;
	
	dx = x3 - x0; dy = y3 - y0; 
	if (ABS(dx) <= 1.0 && ABS(dy) <= 1.0) return -1;
	cros = y0 * x3 - x0 * y3;
	len = sqrt(dx * dx + dy * dy);
	assert (len != 0.0);
	rho = dy * x1 - dx * y1 + cros;
	if (ABS(rho) >= len * epsilon) return 0;
	rho = dy * x2 - dx * y2 + cros;
	if (ABS(rho) >= len * epsilon) return 0;
	return -1;
}

void drawbeziersub(double x0, double y0, double x1, double y1, 
				double x2, double y2, double x3, double y3) {
	 double xl1, yl1, xh, yh, xr2, yr2, xl2, yl2, xr1, yr1, xm, ym;

	 if (straightcurve (x0, y0, x1, y1, x2, y2, x3, y3, strepsilon) != 0) {
		 _moveto((int) x0, (int) y0);
		 _lineto((int) x3, (int) y3);
	 }
	 else {
		 xl1 = (x0 + x1)/2.0;  	yl1 = (y0 + y1)/2.0;
		 xh =  (x1 + x2)/2.0;  	yh =  (y1 + y2)/2.0;
		 xr2 = (x2 + x3)/2.0;  	yr2 = (y2 + y3)/2.0;
		 xl2 = (xl1 + xh)/2.0; 	yl2 = (yl1 + yh)/2.0;
		 xr1 = (xh + xr2)/2.0; 	yr1 = (yh + yr2)/2.0;
		 xm = (xl2 + xr1)/2.0; 	ym = (yl2 + yr1)/2.0;
		 drawbeziersub(x0, y0, xl1, yl1, xl2, yl2, xm, ym);
		 drawbeziersub(xm, ym, xr1, yr1, xr2, yr2, x3, y3);
	 }
}

void drawbezier(int x0, int y0, int x1, int y1, 
				int x2, int y2, int x3, int y3) {
	drawbeziersub((double) x0, (double) y0, (double) x1, (double) y1,
				(double) x2, (double) y2, (double) x3, (double) y3);
}

*/

/* void drawcircle(int xo, int yo, int rad) { */
void drawcircle(short xo, short yo, short rad) {
/*	int binc = (rad * 55)/100; */
	short binc;

	binc = (short) ((rad * 55) / 100);

	drawbezier(xo, yo - rad, xo + binc, yo - rad,
		xo + rad, yo - binc, xo + rad, yo);
	drawbezier(xo + rad, yo, xo + rad, yo +binc,
		xo + binc, yo + rad, xo, yo + rad);
	drawbezier(xo, yo + rad, xo - binc, yo + rad,
		xo - rad, yo + binc, xo - rad, yo);
	drawbezier(xo - rad, yo, xo - rad, yo - binc, 
		xo - binc, yo - rad, xo, yo - rad);
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* void square(int x, int y, int color) { */
void square(short x, short y, unsigned short color) { 
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;
	struct _xycoord cxy;

	cinx = _getcolor();
	lsty = _getlinestyle();
	cxy = _getcurrentposition();
/*	_setcolor(squarecol); */
	_setcolor(color);
	_setlinestyle(squaresty);
	_moveto (x - squareeps, y - squareeps);	
	_lineto (x - squareeps, y + squareeps);
	_lineto (x + squareeps, y + squareeps);	
	_lineto (x + squareeps, y - squareeps);	
	_lineto (x - squareeps, y - squareeps);	
	_setcolor(cinx);
	_setlinestyle(lsty);
	_moveto(cxy.xcoord, cxy.ycoord);
}

/* void diamond(int x, int y, int color) { */
void diamond(short x, short y, unsigned short color) {
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;
	struct _xycoord cxy;

	cinx = _getcolor();
	lsty = _getlinestyle();
	cxy = _getcurrentposition();
/*	_setcolor(diamondcol); */
	_setcolor(color);
	_setlinestyle(diamondsty);
	_moveto (x + diamondeps, y);	
	_lineto (x, y + diamondeps);	_lineto (x - diamondeps, y);
	_lineto (x, y - diamondeps);	_lineto (x + diamondeps, y);
	_setcolor(cinx);
	_setlinestyle(lsty);
	_moveto(cxy.xcoord, cxy.ycoord);
}

/* void tick(int x, int y) { */
void tick(short x, short y) {
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;
	struct _xycoord cxy;

	cinx = _getcolor();
	lsty = _getlinestyle();
	cxy = _getcurrentposition();
	_setcolor(tickcol);
	_setlinestyle(ticksty);
	_moveto (x - tickeps, y);
	_lineto (x + tickeps, y);
	_moveto (x, y - tickeps);
	_lineto (x, y + tickeps);
	_setcolor(cinx);
	_setlinestyle(lsty);
	_moveto(cxy.xcoord, cxy.ycoord);
}

/* void circle(int x, int y, int color) { */
void circle(short x, short y, unsigned short color) {
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;
	struct _xycoord cxy;

	cinx = _getcolor();
	lsty = _getlinestyle();
	cxy = _getcurrentposition();
/*	_setcolor(circlecol); */
	_setcolor(color);
	_setlinestyle(circlesty);
	_ellipse(_GBORDER,
		x - circleeps, y - circleeps, x + circleeps, y + circleeps);
	_setcolor(cinx);
	_setlinestyle(lsty);
	_moveto(cxy.xcoord, cxy.ycoord);
}

/* void diagonal(int x, int y) { */
void diagonal(short x, short y) {
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;
	struct _xycoord cxy;

	cinx = _getcolor();
	lsty = _getlinestyle();
	cxy = _getcurrentposition();
	_setcolor(diagoncol);
	_setlinestyle(diagonsty);
	_moveto(x - diagoneps, y - diagoneps);
	_lineto(x + diagoneps, y + diagoneps);
	_moveto(x - diagoneps, y + diagoneps);
	_lineto(x + diagoneps, y - diagoneps);
	_setcolor(cinx);
	_setlinestyle(lsty);
	_moveto(cxy.xcoord, cxy.ycoord);
}

/* void arrow(int xo, int yo, int xm, int ym) { */
void arrow(short xo, short yo, short xm, short ym) {
/*	int xc, yc, xd, yd, xf, yf; */
	short xc, yc, xd, yd, xf, yf;
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;
/*	int len; */
	short len;
	struct _xycoord cxy;

	cinx = _getcolor();
	lsty = _getlinestyle();
	cxy = _getcurrentposition();
	_setcolor(arrowcol);
	_setlinestyle(arrowsty);
	xc = (short) ((xo + xm) / 2);
	yc = (short) ((yo + ym) / 2);
	xd = xm - xo; yd = ym - yo;
/*	len = (int) sqrt((double) xd * (double) xd + (double) yd * (double) yd);*/
	assert (xd != 0 || yd != 0);
	len = radius(xd, yd);
	assert (len != 0);
	xd = (xd * arroweps) / len;
	yd = (yd * arroweps) / len;
	xf = xd - yd; yf = xd + yd;
	_moveto(xc, yc);
	_lineto(xc - xf, yc - yf);
	xf = xd + yd; yf = - xd + yd;
	_moveto(xc, yc);
	_lineto(xc - xf, yc - yf);
	_setcolor(cinx);
	_setlinestyle(lsty);
	_moveto(cxy.xcoord, cxy.ycoord);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

int nodeold=0;
short xnodeold=0, ynodeold=0;

/* void showposition(int node, int x, int y) { */
void showposition(int node, short x, short y) {
/*	int cinx, lsty; */
	short cinx;
/*	unsigned short lsty; */
	struct _xycoord cxy;

	cxy = _getcurrentposition();
	cinx = _getcolor();
	sprintf(line, "NODE%4d  X%6d  Y%6d", nodeold, xnodeold, ynodeold);
	_setcolor(0);
	_moveto((short) (scolumns - 150), (short) (srows - 20));
	_outgtext(line);
/*	_getch();		*/		/* debugging */
	sprintf(line, "NODE%4d  X%6d  Y%6d", node, x, y); 
	_setcolor(arrowcol);
	_moveto((short) (scolumns - 150), (short) (srows - 20));
	_outgtext(line);
	_setcolor(cinx);
	_moveto(cxy.xcoord, cxy.ycoord);	/* reset position */
	nodeold = node;
	xnodeold = x;
	ynodeold = y;
}	

/* void shownode(int x, int y) { */
void shownode(short x, short y) {
/*	int cinx; */
	short cinx;
	struct _xycoord cxy;

	if (node != 0) {
		cxy = _getcurrentposition();
		cinx = _getcolor();
		if (showcoords == 0) _setcolor(arrowcol);
		else if (currentnode == node) _setcolor(nodecol); /* hot */
		else _setcolor(hintcol); 
		sprintf(line, " %d", node);
		_outgtext(line);
		_setcolor(cinx);
		_moveto(cxy.xcoord, cxy.ycoord);	/* reset position */
	}
	if (showcoords != 0 && currentnode == node) 
		showposition(node, x, y);
}	

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* void moveto(int x, int y) { */
void moveto(int k) { 
/*	int x, y, xm, ym; */
	short x, y, xm, ym;

	x = knots[k].x;
	y = knots[k].y;	

	xm = xmap(x); ym = ymap(y);
	if (dontredraw == 0) {
/*  only show circular mark if subpath selected etc NEW */
	if (selectnumber < 0 || 
		(selectpathflag != 0 && selectnumber == npath) ||
		(selectpathflag == 0 && 
			((knots[k].subr == 0 && selectnumber == nsubr) ||
			 (knots[k].subr > 0 && selectnumber == (int) knots[k].subr)))) {
/*			(selectpathflag == 0 && selectnumber == nsubr)) { */
			if (marksflag != 0) {
/* show circle of different color for font switching */
				if (knots[k].subr > 0) circle(xm, ym, diagoncol);	/* NEW */
				else circle(xm, ym, circlecol);
			}
	}
	}
	_moveto (xm, ym);
	xold = x; yold = y;
	xstart = x; ystart = y;
	kstart = k;
	if (labelnode != 0) shownode(x, y);
}

void lineto(int k) {
/*	int x, y, xm, ym, xo, yo; */
	short x, y, xm, ym, xo, yo;
	
	x = knots[k].x; y = knots[k].y;
	xm = xmap(x); ym = ymap(y);
	xo = xmap(xold); yo = ymap(yold);
	if (dontredraw == 0) {
/*  only show circular mark if subpath selected etc NEW */
	if (alwaysknots != 0 || selectnumber < 0 || 
		(selectpathflag != 0 && selectnumber == npath) ||
			(selectpathflag == 0 && selectnumber == nsubr)) {
			if (marksflag != 0) {
				diamond(xm, ym, diamondcol);
				if (knots[k].subr > 0) circle(xm, ym, diagoncol);	/* NEW */
			}
	}
/*	if (arrowflag != 0 && 
		(xm > xo + minarrow || xm < xo - minarrow || 
			ym > yo + minarrow || ym < yo - minarrow))
			arrow(xo, yo, xm, ym); */
/*	if (selectnumber < 0 || selectpath == npath) */
	if (selectnumber < 0 || 
		(selectpathflag != 0 && selectnumber == npath) ||
			(selectpathflag == 0 && selectnumber == nsubr)) {
			if (arrowflag != 0 && 
				(xm > xo + minarrow || xm < xo - minarrow || 
					ym > yo + minarrow || ym < yo - minarrow))
						arrow(xo, yo, xm, ym);
			_lineto (xm, ym);
	}
	else _moveto (xm, ym);
	}
	else _moveto (xm, ym);		/* in node stepping mode */

/*	if (interx == 0 && intery == 0) {
		interx = xmap((x + xold)/2);  
		intery = ymap((y + yold)/2);  
		if (x < xold) intery++; else intery--;
		if (y < yold) interx--; else interx++;  
	} */

	xold = x; yold = y;
	if (labelnode != 0) shownode(x, y);
}

void curveto(int k1, int k2, int k3) {
/*	int x1, y1, x2, y2, x3, y3; */
	short x1, y1, x2, y2, x3, y3;
/*	int xm0, ym0, xm1, ym1, xm2, ym2, xm3, ym3; */
	short xm0, ym0, xm1, ym1, xm2, ym2, xm3, ym3;
	
	if (straighten != 0) {
		lineto(k3);		/* 1993/Jan/7 */
		return;
	}

	x1 = knots[k1].x; y1 = knots[k1].y;
	x2 = knots[k2].x; y2 = knots[k2].y;
	x3 = knots[k3].x; y3 = knots[k3].y;

	xm0 = xmap(xold); ym0 = ymap(yold);
	xm1 = xmap(x1); ym1 = ymap(y1);
	xm2 = xmap(x2); ym2 = ymap(y2);
	xm3 = xmap(x3); ym3 = ymap(y3);

	if (dontredraw == 0) {
	if (alwaysknots != 0 || selectnumber < 0 || 
		(selectpathflag != 0 && selectnumber == npath) ||
			(selectpathflag == 0 && selectnumber == nsubr)) {
			if (marksflag != 0)  {
				square(xm1, ym1, squarecol); 
				square(xm2, ym2, squarecol); 
				diamond(xm3, ym3, diamondcol); /*	square(xm3, ym3);  */
				if (knots[k3].subr > 0) circle(xm3, ym3, diagoncol); /* NEW */
			}
	}
/*	if (selectnumber < 0 || selectnumber == nsubr)  */
	if (selectnumber < 0 || 
		(selectpathflag != 0 && selectnumber == npath) ||
			(selectpathflag == 0 && selectnumber == nsubr)) 	
				drawbezier(xm0, ym0, xm1, ym1, xm2, ym2, xm3, ym3);
	else _moveto (xm3, ym3);
	}
	else _moveto (xm3, ym3);	/* in node stepping mode */

	xold = x3, yold = y3;
	if (labelnode != 0) shownode(x3, y3);
}

void closepath(void) {
	if (xstart != xold || ystart != yold) {
/*		linetoaux(xstart, ystart); */
		lineto(kstart);
/*		_lineto(xmap(xstart), ymap(ystart)); */
/*		xold = xstart, yold = ystart; */
	}
/* do the fill here ? */
}
	
void hsbw(int width) { /* draw baseline and tick marks */
/*	int cinx, lsty; */
	short cinx;
	unsigned short lsty;

	if (automode != 0) return;

	cinx = _getcolor();
	lsty = _getlinestyle();
	_setcolor(basecol);
	_setlinestyle(basesty);
	_moveto (xmap(-100), ymap(0)); 
	_lineto (xmap(1000), ymap(0));
	tick(xmap(0), ymap(0));
	tick(xmap(width), ymap(0));
/*	interx = 0; intery = 0; */
	_setcolor(cinx);
	_setlinestyle(lsty);

}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void checkknots(int n) {  /* check for bad knot sequences */
	int k, xa, ya, xb, yb, xc, yc;

/*	if (automode != 0) return; */ 			/* don't bother ! */
	
	if (n < 2) return;					/* don't bother ! */
	
	xa = knots[0].x; ya = knots[0].y;
	xb = knots[1].x; yb = knots[1].y;

	for (k = 2; k < n; k++) {
		xc = knots[k].x; yc = knots[k].y;
		if(xa == xb && ya == yb) {
			if(xb == xc && yb == yc) {
#ifdef TRACE
				printf("Equal knots %d in char %d:\n", k, chrs);
				showarr(k-3, k+1); 
#endif
				if (complaindouble == 0 && complaintriple != 0 ) {
					putc('\a', stderr); scanmode = 0; /* automode =	0;  */
					if (batchmode != 0) {
						fprintf(stderr, "Triple knot in char %d -", chrs);
						fprintf(stderr, "type PgDn to continue\n");
					}
					else {
						checkflag++;
/*						automode = 0; */
/*						newredochar(fp_in, fp_hnt); */
					}
				}
				tripleknots++;
				drawcircle (xmap(xa), ymap(ya), 10);
			}
			if (complaindouble != 0) {
				putc('\a', stderr);	scanmode = 0; /* automode = 0; */
				if (batchmode != 0) {
					fprintf(stderr,"Double knot in char %d -", chrs);
					fprintf(stderr, "type PgDn to continue\n");
				}
				else {
					checkflag++;
/*					automode = 0; */
/*					newredochar(fp_in, fp_hnt); */
				}
			}
			doubleknots++;
			diagonal(xmap(xa), ymap(ya));
		}
		xa = xb; xb = xc; ya = yb; yb = yc;
	}
	return;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef FANCYSTUFF

/* Following is the code for "improving" outlines */
/* There are some parameters/thresholds in this ... */

int levepsilon = 6; 		/* allowed departure from level line */
int lenepsilon = 6; 		/* allowed departure from straight line */

double tansmall=0.2;		/* angle considered small if tangent less */

void shiftdown(int k, int dk) { /* shift knots down by dk */
	int i;

	for (i = k; i < knot; i++) 	knots[i-dk] = knots[i];
	knot = knot - dk;
}

void shiftup(int k, int dk) { /* shift knots up by dk */
	int i;

	knot = knot + dk;
	assert (k > 2);
	assert (knot < MAXKNOTS);
	for (i = knot-1; i > k; i--) knots[i] = knots[i - dk];
}

/* don't mess with near vertical and near horizontal lines */

int nottang(int xs, int ys, int xm, int ym, int xf, int yf) {
	int dx, dy;
	dx = xs - xm; dy = ys - ym;
	if (ABS(dx) <= 1 || ABS(dy) <= 1) return 0;
	dx = xf - xm; dy = yf - ym;
	if (ABS(dx) <= 1 || ABS(dy) <= 1) return 0;
	return -1;
}

/* used to avoid combining short, but sharply turning curve segments */

int smallangle(int x0, int y0, int x1, int y1, 
				int  x2, int y2, int x3, int y3) {
	int dx1, dy1, dx3, dy3;
	long cros, dot;
	double tantheta;
	
	dx1 = x1 - x0; dy1 = y1 - y0; dx3 = x3 - x2, dy3 = y3 - y2;
	dot = (long) dx1 * (long) dx3 + (long) dy1 * (long) dy3;
	if(dot < 0) return 0;	/* more than ninety degree turn ! */
	cros = (long) dx1 * (long) dy3 - (long) dx3 * (long) dy1;
	if (dot <= 0l) return 0;			/* more than ninety degrees */
	tantheta = (double) cros / (double) dot;
	if (ABS(tantheta) < tansmall) return -1;
	else return 0; 
}

int nearline(int xs, int ys, int xm, int ym, int xf, int yf, int epsilon) {
	int dx, dy, len;
	long cros, rho;

	dx = xf - xs; dy = yf - ys; 
	cros = (long) ys * (long) xf - (long) xs * (long) yf;
	len = radius(dx, dy);
	rho = (long) dy * (long) xm - (long) dx * (long) ym + cros;
	if (ABS(rho) < (long) len * (long) epsilon) return -1;
	else return 0;
}
		
int flatcurve(int x0, int y0, int x1, int y1, 
				int  x2, int y2, int x3, int y3) {
	if ((nearline(x0, y0, x1, y1, x3, y3, lenepsilon)  != 0) &&
		(nearline(x0, y0, x2, y2, x3, y3, lenepsilon)  != 0) &&
		(smallangle(x0, y0, x1, y1, x2, y2, x3, y3) != 0)) return -1;
	else return 0;
}

void levelarr(void) {
	int k, xold, xnew, dx, yold, ynew, dy, yaver;
	
	if (knot < 1) return;
	for (k = 1; k < knot; k++) {
		if (knots[k].code == 'E') {
			xold = knots[k-1].x; yold = knots[k-1].y;
			xnew = knots[k].x; ynew = knots[k].y;
			dx = xnew - xold; dy = ynew - yold;
			if (ABS(dy) < levepsilon && ABS(dx) > 300) {
				yaver = (yold + ynew) / 2;
				knots[k-1].y = yaver;
				knots[k].y = yaver;
			}
		}
	}
}

/* maybe adjust end points of new lineto also ? */

void combinearr(void) {
	int k, xa, ya, xb, yb, xc, yc;
	if (knot < 2) return;
	for (k = 2; k < knot; k++) {
		if (knots[k].code == 'E' && knots[k-1].code == 'E') {
			xa=knots[k-2].x; ya=knots[k-2].y;
			xb=knots[k-1].x; yb=knots[k-1].y;
			xc=knots[k].x; yc=knots[k].y;
			if (nottang(xa, ya, xb, yb, xc, yc) != 0 &&
				nearline(xa, ya, xb, yb, xc, yc, lenepsilon) != 0) {
				shiftdown(k, 1);
				k--;
			}
		}
	}
}

void flattenarr(void) {
	int k, dy, dx, xa, ya, xb, yb, xc, yc, xd, yd;
	
	if (knot < 3) return;
	
	for (k = 3; k < knot; k++) {
		if (knots[k].code == 'C') {
			dy = knots[k].y - knots[k - 3].y; 
			dx = knots[k].x - knots[k - 3].x;
			if (ABS(dy) > 2 || ABS(dx) > 200) {
				xa=knots[k-3].x; ya=knots[k-3].y;
				xb=knots[k-2].x; yb=knots[k-2].y;
				xc=knots[k-1].x; yc=knots[k-1].y;
				xd=knots[k].x; yd=knots[k].y;
				if (flatcurve(xa, ya, xb, yb,  xc, yc, xd, yd) != 0) {
					knots[k].code = 'E';
					shiftdown(k, 2);
					assert (knots[k-2].code == 'E');
				}
			}
		}
	}
}

#endif

/* The above was the code for "improving" outlines to improve hinting */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

/* actually do reordering of subpath in arrays */

void circulate(int start, int end, int korigin) {			
	int j;
/*	circulate drawing commands so the desired one is first/last */
/*	printf("Reordering subpath in char %d\n", chrs);  */

/*	sanity checks - remove later DEBUGGING ONLY */
/*	if (knots[end-1].x != knots[start].x ||	knots[end-1].y != knots[start].y)
			fprintf(stderr, "Start %d %d End %d %d\n",
		   knots[start].x, knots[start].y, knots[end-1].x, knots[end-1].y); */
/*	if (knots[start].code == 'M') knots[start].code = 'L';
	else fprintf (stderr, "First knot not moveto\n"); */
	if (knots[korigin].subr == STARTSUBR) knots[korigin].subr = 0;
	else fprintf (stderr, "Start not consistent\n");	

/*	for (j = start + 1; j < korigin; j++) */
	for (j = start + 1; j < korigin+1; j++) 
		knots[j - (start + 1) + end] = knots[j];
	for (j = start; j < end; j++) 
/*		knots[j] = knots[j - start + (korigin - 1)];  */
		knots[j] = knots[j - start + korigin]; 
	knots[start].code = 'M';
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void endsubpath (void) { /* now check subpath, reverse it, and create code */
	char com;			/* current command code */
	int k;				/* current knot */
	int lstk;		 	/* knot at which previous command found */
	char lstcom;		/* code of previous command */
	int n;				/* temporary */
	int korigin;			/* start position of subpath, if specified */

/* at this point knot = number of knots in subpath */

	if (knot > maxknts) {	maxknts = knot; maxkchrs = chrs; }	
	k = knot;

	korigin = checkarr(k);		/* update xmin, ymin, xmax, ymax */

	if (korigin != 0 && enforcestart != 0) circulate(0, knot, korigin);

	checkknots(k);	 	/* check on bad knot sequences */

/*	if (checkflag != 0) automode = 0; */

#ifdef FANCYSTUFF
	if (flattenflag != 0) flattenarr(); /* F8 - C-F8 */
	if (combineflag != 0) combinearr();	/* F7 - C-F7 */
	if (levelflag != 0) levelarr();		/* F6 - C-F6 */
#endif

	if (strokeflag != 0) {
		if (flexflag != 0) strokeflex(k);
		else  strokeknots(k); 
		if (extremaflag != 0) {
			strokeextrema(k);
			strokebounding(k);
		}
	}
	else if (shwhntflag != 0) hintarr(k);	
	
	if (automode != 0) {
		knot = 0; return;
	}

	if (reverseflag == 0) {
		if (cumulative == 0) node = 0;
		if (knot > 0) {
			moveto(0);		/*  moveto first point of path */
			node++;
		}
		if ((n = knots[0].subr) > 0) nsubr = n;
		for (k = 1; k < knot; k++) {
			com = knots[k].code;		/* grab code of this knot */
			if (com != ' ') { 
				if (com == 'E') lineto(k);
				else if (com == 'C') curveto(k-2, k-1, k);
				else { 
					fprintf(stderr, "Error in subpath * cod = %c", com);
					showarr(k-3, k);	giveup(7);
				}
				if ((n = knots[k].subr) > 0) nsubr = n;
				node++;
			}
		}
	}
	else {
		k--;						/* go back to last knot in subpath */
		if (k >= 0)	moveto(k);		/* moveto first point of path */
		lstk = k; lstcom = knots[k].code; /* remember command here */
		k--;
		while (k >= 0) {				/* step backward through subpath */
			com = knots[k].code;		/* grab code of this knot */
			if (com != ' ') { 
				if ((n = knots[k].subr) > 0) nsubr = n;
				if (lstcom == 'E') lineto(k);
				else if (lstcom == 'C') curveto(k+2, k+1, k);
				else {
					fprintf(stderr, "Error in subpath * cod = %c", lstcom);
					showarr(k, lstk);	giveup(7);
				}
				lstk = k; lstcom = com; /* knots[k].code */
			}
			k--;
		}
	}
	if (knot > 0) closepath();
	knot = 0;	 /* ready for next subpath */
	npath++;	 /* up path count */
}

void startsubpath(void) {
	if (knot != 1) {
		fprintf(stderr, "Moveto not at start of subpath %d", knot);
		giveup(5);
	}
/*	pxur = pxll = knots[knot-1].x; pyur = pyll = knots[knot-1].y; */
	knots[knot-1].code = 'M';  /* moveto - first knot */
}

/* void startstring (int sbxchr, int width) { */ /* left sidebearing and width */
void startstring (short sbxchr, short width) { /* left sidebearing and width */
/*	long numerwidth, denomwidth; */
/*	long g; */
	double fwidth;

	fwidth = (double) width * (double) numerscale / (double) denomscale; 
#ifdef TRACE
	printf ("C %d : WX %lg ; OWX %lg\t%c\n", 
		chrs, fwidth, owidth, (fwidth == owidth) ? '-' : '+');
#endif

	iwidth = (int) fwidth;
	if (showwidth != 0) {
		recorddownstroke(iwidth, -INFINITY, -INFINITY); /* new */
		recordupstroke(0, -INFINITY, -INFINITY);		/* new */
	}

/*	numencode(sbxchr); 	lnumencode(numerwidth); lnumencode(denomwidth); 
	code_div();  code_hsbw(); */

	if (automode == 0) hsbw(iwidth);
/*	xold = sbxchr, yold = 0; */
	xold = sbxchr;
	yold = 0;
}

void endstring(void) { /* end of character outline */
/*	sort strokes here, and merge them */
	sortupstrokes();
	sortdownstrokes();
	sortleftstrokes();
	sortrightstrokes(); 
/*  don't display - it will be done later anyway */
/*	if (automode == 0 && strokeflag != 0) showstrokes(); */
}

int dealwithcomment(FILE *fp_in) {
	char comname[MAXCHARNAME];			/* new */
	int c;
	
/* new stuff to scan in character name comment */
	c = getc(fp_in);
	while (c != EOF && c != '%' && c != 'd') c = getc(fp_in);
	if (c == '%') {
		if (fscanf(fp_in, "%s", comname) == 0) comname[0] = '\0';
		c = getc(fp_in);
		while (c != EOF && c != '\n' && c != '\r') c = getc(fp_in);
		if (c != EOF) ungetc(c, fp_in);
	}
	else if (c != EOF) ungetc(c, fp_in);
	if (c == EOF) return -1;
	else return 0;
}

int searchoutlines(FILE *fp_in, int chr) { /* search character outlines */
	int c, fchr=-1, wrap=0, nxtchr=fontchrs;
	
	task = "searching character outlines";

	for(;;) {

		dealwithcomment(fp_in);

		if (fscanf(fp_in, " dup %d", &fchr) != 1) { /* read character code */
/*			fprintf(stderr, "Reached the end\n"); */
/*			fprintf(stderr, "E %d ", chr); */
			if (fstart < 0) seekerror("searchoutlines", "fp_in");
			else fseek(fp_in, fstart, SEEK_SET);  
			if (sequenceflag == 0) 	return -1;			/* reached the end */
			if (wrap != 0) return -nxtchr;				/* not found */
			wrap = 1;

			dealwithcomment(fp_in);
			fscanf(fp_in, " dup %d", &fchr);
		}

		if (sequenceflag == 0 && fchr < 0) return -1;
		
		if (fchr < 0 || fchr > fontchrs) {
			fprintf(stderr, "Character code %d out of range\n",	fchr);
/*			giveup(13); */
			return -1;			/* ??? */
		}
/*		nchrs++; */

		if (fchr == chr || chr == -1) return fchr; /* OK */

		if (fchr > chr && fchr < nxtchr) nxtchr = fchr;	/* remember nearest */

/*		if (fchr == fontchrs - 1) return -1; */ /* reached the end */

		while ((c = ngetc(fp_in)) != '<' && c != EOF) ;	/* start hex field */
		while ((c = ngetc(fp_in)) != '>' && c != EOF) ;	/* end hex field */
		while ((c = ngetc(fp_in)) <= ' ' && c != EOF) ; /* white space */
		ungetc(c, fp_in);
		fscanf(fp_in, "put");				/* scan over "put" */
	}
}

int flagupstroke(int x) {
	int k, flag=0;
/*	printf("U %d", nupstrokes); */
	for(k = 0; k < nupstrokes; k++) {
		if (upstrokes[k].pos == x) {
			upstrokes[k].flag = 1; flag = 1; break;
		}
	}
	return flag;
}

int flagdownstroke(int x) {
	int k, flag=0;
/*	printf("D %d", ndownstrokes); */
	for(k = 0; k < ndownstrokes; k++) {
		if (downstrokes[k].pos == x) {
			downstrokes[k].flag = 1; flag = 1; break;
		}
	}
	return flag;
}

int flagleftstroke(int y) {
	int k, flag=0;
/*	printf("L %d ", nleftstrokes); */
	for(k = 0; k < nleftstrokes; k++) {
		if (leftstrokes[k].pos == y) {
			leftstrokes[k].flag = 1; flag = 1; break;
		}
	}
	return flag;
}

int flagrightstroke(int y) {
	int k, flag=0;
/*	printf("R %d", nrightstrokes); */
	for(k = 0; k < nrightstrokes; k++) {
		if (rightstrokes[k].pos == y) {
			rightstrokes[k].flag = 1; flag = 1; break;
		}
	}
	return flag;
}

void clearstrokes(void) {
	int k;
	for(k = 0; k < nupstrokes; k++) upstrokes[k].flag = 0;
	for(k = 0; k < ndownstrokes; k++) downstrokes[k].flag = 0;
	for(k = 0; k < nleftstrokes; k++) leftstrokes[k].flag = 0;
	for(k = 0; k < nrightstrokes; k++) rightstrokes[k].flag = 0;
}

#ifdef EXTRASTUFF
void setstrokes(void) {
	int k;
	int xold=-INFINITY, yold=-INFINITY;
	for(k = 0; k < nupstrokes; k++) {
		if (upstrokes[k].pos != xold) {
			upstrokes[k].flag = 1;
			xold = upstrokes[k].pos;
		}
	}
	for(k = 0; k < ndownstrokes; k++) {
		if (downstrokes[k].pos != xold) {
			downstrokes[k].flag = 1;
			xold = downstrokes[k].pos;
		}
	}
	for(k = 0; k < nleftstrokes; k++) {
		if(leftstrokes[k].pos != yold) {
			leftstrokes[k].flag = 1;
			yold = leftstrokes[k].pos;
		}
	}
	for(k = 0; k < nrightstrokes; k++) {
		if(rightstrokes[k].pos != yold) {
			rightstrokes[k].flag = 1;
			yold = rightstrokes[k].pos;
		}
	}
}
#endif

int enterleft(int ys) {
	int k, marked = 0;
	if (flagleftstroke(ys) != 0) marked++;
	else if (fuzzflag > 0) { /* snapto */
		for (k=1; k <= fuzzflag; k++) {
			if (flagleftstroke(ys-k) != 0) { marked=-k; break; }
			if (flagleftstroke(ys+k) != 0) { marked=k; break; }
		}
		if (marked != 0) {
			if (showadjust != 0) printf("RIGHT %d => %d ", ys, ys+marked);
			if (automode == 0) {
/*				if (showadjust != 0) printf("RIGHT %d => %d ", ys, ys+marked); */
				scanmode = 0; 
			}
			if (verboseflag != 0) putc('\a', stdout); 
			return marked;
		}
	}
	if (marked == 0) {
		recordleftstroke(ys, -INFINITY, -INFINITY);
		sortleftstrokes();			/* or later */
		flagleftstroke(ys);
	}
	return 0;
}

int enterright(int ye) {
	int k, marked = 0;
	if (flagrightstroke(ye) != 0) marked++;
	else if (fuzzflag > 0) {  /* snapto */
		for (k=1; k <= fuzzflag; k++) {
			if (flagrightstroke(ye-k) != 0) { marked=-k; break; }
			if (flagrightstroke(ye+k) != 0) { marked=k; break; }
		}
		if (marked != 0) {
			if (showadjust != 0) printf("LEFT %d => %d ", ye, ye+marked);
			if (automode == 0) {
/*				if (showadjust != 0) printf("LEFT %d => %d ", ye, ye+marked); */
				scanmode = 0; 
			}
			if (verboseflag != 0) putc('\a', stdout); 
			return marked;
		}
	}
	if (marked == 0) {
		recordrightstroke(ye, -INFINITY, -INFINITY);
		sortrightstrokes();			/* or later */
		flagrightstroke(ye);
	}
	return 0;
}

int enterup(int xs) {
	int k, marked = 0;
	if (flagupstroke(xs) != 0) marked++;
	else if (fuzzflag > 0) {  /* snapto */
		for (k=1; k <= fuzzflag; k++) {
			if (flagupstroke(xs-k) != 0) { marked=-k; break; }
			if (flagupstroke(xs+k) != 0) { marked=k; break; }
		}
		if (marked != 0) {
			if (showadjust != 0) printf("DOWN %d => %d ", xs, xs+marked);
			if (automode == 0) {
/*				if (showadjust != 0) printf("DOWN %d => %d ", xs, xs+marked); */
				scanmode = 0; 
			}
			if(verboseflag != 0) putc('\a', stdout); 
			return marked;
		}
	}
	if (marked == 0) {
		recordupstroke(xs, -INFINITY, -INFINITY);
		sortupstrokes(); /* or later */
		flagupstroke(xs);
	}
	return 0;
}

int enterdown(int xe) {
	int k, marked = 0;
	if (flagdownstroke(xe) != 0) marked++;
	else if (fuzzflag > 0) {  /* snapto */
		for (k=1; k <= fuzzflag; k++) {
			if (flagdownstroke(xe-k) != 0) { marked=-k; break; }
			if (flagdownstroke(xe+k) != 0) { marked=k; break; }
		}
		if (marked != 0) {
			if (showadjust != 0) printf("UP %d => %d ", xe, xe+marked);
			if (automode == 0) {
/*				if (showadjust != 0) printf("UP %d => %d ", xe, xe+marked); */
				scanmode = 0; 
			}
			if (verboseflag != 0) putc('\a', stdout); 
			return marked;
		}
	}
	if (marked == 0) {
		recorddownstroke(xe, -INFINITY, -INFINITY);
		sortdownstrokes(); /* or later */
		flagdownstroke(xe);
	}
	return 0;
}

void analyzehintline(char *s, int nstart) {
	int k, ghostflag, n = nstart;
	int xs, xe, ys, ye, z;
	char c;
	int xoff=0, yoff=0;						/* 1991/April/6 */

	s = line + n; 
	while(sscanf(s, " %c %n", &c, &n) == 1) {
		s = s + n;
		if (c == 'H' || c == 'E') { /* E stands for hstem3 */
			while ((sscanf(s, "%d %d %n", &ys, &ye, &n) == 2)) {
				ys += yoff; ye += yoff;	/* 1992/April/6 */
				if (ye > ys + 2048 || ys > ye + 2048) {
					fprintf(stderr, "Hints out of range: %s", s);
					s = s + n;
					continue;	/* return; */
				}
				s = s + n;
/* first of all, try and base bottom/top decision on width in file */
				if (ye == ys + 20) ghostflag = + 1; /* top ghost stem */
				else if (ye == ys + 21) ghostflag = -1; /* bottom ghost */
				else ghostflag = 0;
/* next, if asked for, check whether one side or other is already aligned */
/* if so, adjust stem width to be 20 for top and 21 for bottom */
				if (ghostadjust != 0 && ghostflag != 0) {
					if (flagrightstroke(ye) != 0) { /* top ghost */
						if (flagleftstroke(ys) == 0) { /* only if not hit */
							ys = ye - 20; ghostflag =0;
						}
					}
					else if (flagleftstroke(ys) != 0) { /* bottom ghost */
						if (flagrightstroke(ye) == 0) { /* only if not hit */
							ye = ys + 21; ghostflag =0;
						}
					}
				}
/* if (ghostflag > 0) TOP snap ye first adjust ys accordingly - then enter */
/* if (ghostflag < 0) BOT snap ys first adjust ye accordingly - then enter */
				if (fuzzflag > 0 && ghostflag > 0) {
					k = enterright(ye);
					if (showadjust != 0 && k != 0) 
						printf("[YS %d => %d] ", ys, ys+k);
					ys = ys + k;
					enterleft(ys);
				}
				else if (fuzzflag > 0 && ghostflag < 0) {
					k = enterleft(ys);
					if (showadjust != 0 && k != 0) 
						printf("[YE %d => %d] ", ye, ye+k);
					ye = ye + k;
					enterright(ye);
				}
				else {		/* either fuzzflag == 0 or ghostflag == 0 */
/*					if (showadjust != 0) printf("[YS %d YE %d] ", ys, ye); */
					enterleft(ys); enterright(ye); 
				}
			}
		}
		else if (c == 'V' || c == 'M') { /* M stands for vstem3 */
			while ((sscanf(s, "%d %d %n", &xs, &xe, &n) == 2)) {
				xs += xoff; xe += xoff;	/* 1992/April/6 */				
				if (xe > xs + 2048 || xs > xe + 2048) {
					fprintf(stderr, "Hints out of range: %s", s);
					s = s + n;
					continue; /* return; */
				}
				s = s + n;
/*	there are no vertical ghost stems */
				enterup(xs); enterdown(xe);
			}
		}
		else if (c == 'O' || c == 'o') { /* O stands for offset */
			if (sscanf(s, "%d %d %d", &xoff, &yoff, &n) != 2) {
				fprintf(stderr, "Don't understand offset %s", s);
					return;
			}
			s = s + n;
		}
		else {
			while ((sscanf(s, "%d %n", &z, &n) == 1)) s = s + n;
		}
		sscanf(s, ";%n", &n);
		s = s + n;
	}
}

/* don't know yet what to do when ghostflag is on ... */
/* would be nice if one knew which side of stem was just due to ghost */
/* this is where 20 for top and 21 for bottom convention comes in ... */

/* automatically ignores font-level hints ? and hint replacement subrs ? */

/* come here if readallhints == 0 */

int readoldhints(FILE *fp_hnt, int chrs) {
	int fchrs=-1;
	int marked=0;
	int n;
	char *s;

	if (fgets(line, MAXLINE, fp_hnt) == NULL) return chrs;		/* EOF */
/*	ignore comment lines and blank lines and lines that start with space */
/*	while (*line == '%' || *line == '\n')  */
	while (*line == '%' || *line == '\n' || *line == '\t' || *line == ' ') 
		if (fgets(line, MAXLINE, fp_hnt) == NULL) return chrs; 

/*	Is it a hint replacement line ? NEW 1994/May/21 */
	fsubr = 0;							/* default if not hint replacement */
	if (*line == 'S') {
		if (sscanf (line, "S %d ;%n", &fsubr, &n) == 0) {
			fprintf(stderr, "Don't understand hint replacement: %s", line);
		}
		else {	/* now shift down to expose normal hint code */
			s = line+n;
			while (*s != 'C' && *s != '\0') s++;
			strcpy(line, s);
			if (sscanf(line, "C %d ;%n", &fchrs, &n) != 0) {
				if (fchrs == oldchrs) {	/* is it the char we are working on? */
					chrs = fchrs;		/* then reset char number we want */
				}
			}			
		}		
	}
	if (*line == 'C') {
		if (sscanf(line, "C %d ;%n", &fchrs, &n) == 0) {
			fprintf(stderr, "Don't understand hint line: %s", line);
		}
		while (fchrs < chrs) { 	/* scan up to line for char we want */
			if (fgets(line, MAXLINE, fp_hnt) == NULL) return chrs; 
			sscanf(line, "C %d ;%n", &fchrs, &n);
		}
	}
	if (fchrs > chrs) {			/* gone too far ? attempt to undo */
		if (hstart < 0) seekerror("readoldhints", "fp_hnt");
		else fseek(fp_hnt, hstart, SEEK_SET);
		return chrs;
	}
	oldchrs = fchrs;			/* remember we just worked on this one */
	analyzehintline(line, n);	
	return chrs;
} 

/* read ahead to see if hint replacement follows */

int isnextreplace(FILE *fp_hnt) {
	long present=-1;
	int xsubr;

	present = ftell(fp_hnt);
	if (fgets(line, MAXLINE, fp_hnt) == NULL) return 0;
	while (*line == '%' || *line == '\n' || *line == '\t' || *line == ' ') 
		if (fgets(line, MAXLINE, fp_hnt) == NULL) return 0;
	if (present < 0) seekerror("isnextreplace", "fp_hnt");
	else fseek (fp_hnt, present, SEEK_SET);
	if (*line == 'S') {
		sscanf(line, "S %d ", &xsubr);
		return xsubr;
	}
	return 0;					/* next is not hint replacement */
}

/* ALL hints if selectnumber < 0 --- only hints for select subr otherwise */

/* New version - reads hint replacement hints if asked to */
/* come here if readallhints != 0 */

int readnewhints(FILE *fp_hnt, int chrs) {
	int fchrs=-1, n, marked=0, nsubr=0;

	fseek(fp_hnt, 0, SEEK_SET);		/* rewind(fp_hnt); */

	fsubr=0;
	oldchrs=-1;

	for (;;) {
		if (fgets(line, MAXLINE, fp_hnt) == NULL) return chrs;		/* EOF */
/*		ignore comments, blank lines and font level hints */
/*		if (*line == '%'|| *line == '\n' || *line == '/') continue; */
		if (*line == '%'|| *line == '\n' || *line == '/' || *line == ' ')
			 continue;
		if (*line == 'C') {
			nsubr = 0;
			if (sscanf(line, "C %d ;%n", &fchrs, &n) == 0) {
				fprintf(stderr, "Don't understand hint line: %s", line);
				continue;
			}
		}
		else if (*line == 'S') {
			if (sscanf(line, "S %d ; C %d ;%n", &nsubr, &fchrs, &n) == 0) {
				fprintf(stderr, "Don't understand hint line: %s", line);
				continue;
			}
		}
		else {
			if (verboseflag)	/* taken out 95/June/12 */
			fprintf(stderr, "Don't understand hint line: %s", line); 
			continue;
		}

		if (fchrs != chrs) continue;	/* not for this character */
		else if (selectnumber < 0 || selectpathflag != 0 ||
			(selectpathflag == 0 && selectnumber == nsubr))
			analyzehintline(line, n);	/* found a relevant line */
	}
}

int readhints(FILE *fp_hnt, int chrs) {
	if (readallhints != 0) return readnewhints(fp_hnt, chrs);
	else return readoldhints(fp_hnt, chrs);	
}

/* checkfor dummy curveto that calls for hint replacment */
void checksubr(void) {
	int nsubr;
	nsubr = knots[knot-1].x;
	if (knots[knot-1].y == nsubr &&
		knots[knot-2].x == nsubr &&
		knots[knot-2].y == nsubr &&
		knots[knot-3].x == nsubr &&
		knots[knot-3].y == nsubr) {
		knots[knot-1].code = ' ';
		knot = knot - 3;					/* ignore if fake curveto ! */
		if (nsubr < 0) nsubr = STARTSUBR;	/* special code for START */
		if (knot > 0)						/* tag code on previous */
			knots[knot-1].subr = (unsigned char) nsubr;	
	}
}

void readoutline(FILE *fp_in) { /* process character outline */
/*	int width, xnext; */
	int xnext;
	short width;
/*	int num; */
	short num;
	int c;
	
	task = "working on character outline";
/*	if (verboseflag != 0) putchar('\n'); */

		c = ngetc(fp_in);
		while (c != '<') { 		/* Scan for hex field */
			c = ngetc(fp_in);
		}

		checkflag = 0;
		nxrange = 0; nyrange = 0;		/* subpath coord ranges */
		knot = 0;		/* index into array of numbers accumulated */
		xnext = 1; 		/* first coordinate will be an x */
		nsubr = 0;		/* initial subr number for this character */
		npath = 0;		/* initial path number for this character */
		task = "reading width";

		width = gobblenumber(fp_in); 	/* get width - no scaling here */
		startstring (0, width);	/* start string */

		node = 0;

		task = "processing outline";
		c = ogetc(fp_in);
		while (c != '>') {
			if (knot >= MAXKNOTS) {			/* 1994/July/9 */
				fprintf(stderr, "Too many knots %d", knot);
				while (c != '>') c = ogetc(fp_in);
				break;
			}
			if ((c >= '0' && c <= '9') || c == 'B') {
				ungetc(c, fp_in);
				num = scalenum(gobblenumber(fp_in));
				if (xnext != 0) {			/* next number is x */
					knots[knot].x = num; 
					xnext = 0;
				}
				else {						/* next number is y */
					knots[knot].y = num; 
					xnext = 1; 
					knots[knot].code = ' ';	
					knots[knot].subr = 0;	/* invalid subr code */
					knot++; 
					if (knot >= MAXKNOTS) {
						fprintf(stderr, "\nToo many knots %d", knot);
						giveup(7);
					}
				}
			}
			else {
				if (c != 'A' && knot == 0 ) { /* flush this test - space */
					if (traceflag != 0)
						fprintf(stderr, 
						   "\nSubpath does not start with coordinates %c", c);
				}
				switch(c) {
					case 'D': endsubpath(); break; /* GO OFF AND SHOW IT */
					case 'F': startsubpath(); break; /* GO START A SUBPATH */
					case 'E': knots[knot-1].code = 'E'; break; /* lineto  */
					case 'C': knots[knot-1].code = 'C'; checksubr();
							break; /* curveto  */
					case 'A': break;			/* i.e. blank */
					default: {
						fprintf(stderr, 
							"\nError: char %c not an A (blank)", c);
						giveup(3);
					} break;
				}
			}
			c = ogetc(fp_in);
		}

		while ((c = ngetc(fp_in)) <= ' ' && c != EOF) ; /* white space */
		ungetc(c, fp_in);
		fscanf(fp_in, "put");				/* scan over "put" */

		endstring();		/* finish outline code string */
		if (checkflag != 0) automode = 0;
}

void showadapter(int adapter) {
	switch (adapter) {
		case _MDPA:
			printf("MDPA		Monochrome Display Adapter\n"); break;
		case _CGA:
			printf("CGA		Color Graphics Adapter\n"); break;
		case _EGA:
			printf("EGA		Enhanced Graphics Adapter\n"); break;
		case _VGA:
			printf("VGA		Video Graphics Array\n"); break;
		case _MCGA:
			printf("MCGA		MultiColor Graphics Array\n"); break;
		case _HGC:
			printf("HGC		Hercules Graphics Card\n"); break;
		case _OCGA:
			printf("OCGA		Olivetti Color Graphics\n"); break;
		case _OEGA:
			printf("OEGA		Olivetti Enhanced Graphics\n"); break;
		case _OVGA:
			printf("OVGA		Olivetti Video Graphics\n"); break;
		case _SVGA:
			printf("SVGA		Super VGA with VESA BIOS support\n"); break;
		default:
			printf("Unknown adapter type\n"); break;
	}
}

void showmode (int mode) {
	switch(mode) {
		case _TEXTBW40:
			printf("TEXTBW40	40-column text, 16 grey\n"); break;
		case _TEXTC40:
			printf("TEXTC40		40-column text, 16/8 color\n"); break;
		case _TEXTBW80:
			printf("TEXTBW80	80-column text, 16 grey\n"); break;
		case _TEXTC80:
			printf("TEXTC80		80-column text, 16/8 color\n"); break;
		case _MRES4COLOR:
			printf("MRES4COLOR	320 x 200, 4 color\n"); break;
		case _MRESNOCOLOR:
			printf("MRESNOCOLOR	320 x 200, 4 grey\n"); break;
		case _HRESBW:
			printf("HRESBW		640 x 200, BW\n"); break;
		case _TEXTMONO:
			printf("TEXTMONO	80-column text, BW\n"); break;
		case _HERCMONO:
			printf("HERCMONO	720 x 348, BW for HGC\n"); break;
		case _MRES16COLOR:
			printf("MRES16COLOR	320 x 200, 16 color\n"); break;
		case _HRES16COLOR:
			printf("HRES16COLOR	640 x 200, 16 color\n"); break;
		case _ERESNOCOLOR:
			printf("ERESNOCOLOR	640 x 350, BW\n"); break;
		case _ERESCOLOR:
			printf("ERESCOLOR	640 x 350, 4 or 16 color\n"); break;
		case _VRES2COLOR:
			printf("VRES2COLOR	640 x 480, BW\n"); break;
		case _VRES16COLOR:
			printf("VRES16COLOR	640 x 480, 16 color\n"); break;
		case _MRES256COLOR:
			printf("MRES256COLOR	320 x 200, 256 color\n"); break;
		case _ORESCOLOR:
			printf("ORESCOLOR	640 x 400, 1 of 16 colors (Olivetti only)\n"); break;
/* the following 8 modes require VESA SuperVGA BIOS extensions */
		case _ORES256COLOR:
			printf("ORES256COLOR	640 x 400, 256 colors (Olivetti only)\n"); break;
		case _VRES256COLOR:
			printf("VRES256COLOR	640 x 480, 256 color\n"); break;

/* WARNING: DO NOT attempt to set the following modes without ensuring that
   your monitor can safely handle that resolution.  Otherwise, you may risk
   damaging your display monitor!  Consult your owner's manual for details.
   Note: _MAXRESMODE and _MAXCOLORMODE never select SRES, XRES, or ZRES modes
   */
/* requires NEC MultiSync 3D or equivalent, or better */
		case _SRES16COLOR:
			printf("SRES16COLOR		800 x 600, 16 color\n"); break;
		case _SRES256COLOR:
			printf("SRES256COLOR	800 x 600, 256 color\n"); break;
/* requires NEC MultiSync 4D or equivalent, or better */
		case _XRES16COLOR:
			printf("XRES16COLOR		1024 x 768, 16 color\n"); break;
		case _XRES256COLOR:
			printf("XRES256COLOR	1024 x 768, 256 color\n"); break;
/* requires NEC MultiSync 5D or equivalent, or better */
		case _ZRES16COLOR:
			printf("ZRES16COLOR		1280 x 1024, 16 color\n"); break;
		case _ZRES256COLOR:
			printf("ZRES256COLOR	1280 x 1024, 256 color\n"); break;
		default: printf("Unknown mode code\n"); break;
	}
}

void showmonitor (int monitor) {
	switch(monitor) {
		case _MONO:
			printf("MONO		Monochrome\n"); break;
		case _COLOR:
			printf("COLOR		Color (or Enhanced emulating color)\n"); break;
		case _ENHCOLOR:
			printf("ENHCOLOR	Enhanced Color\n"); break;
		case _ANALOGMONO:
			printf("ANALOGMONR	Analog Monochrome only\n"); break;
		case _ANALOGCOLOR:
			printf("ANALOGCOLOR	Analog Color only\n"); break;
		case _ANALOG:
			printf("ANALOG		Analog Monochrome and Color modes\n"); break;
		default:
			printf("Unknown monitor code\n"); break;
	}
}

void showstate(struct _videoconfig vc) { 
	printf( "Adapter code %x\t", vc.adapter);
	showadapter(vc.adapter);
	printf( "Mode code    %x\t", vc.mode);
	showmode(vc.mode);
	printf( "Monitor code %x\t", vc.monitor);
	showmonitor(vc.monitor);
	printf( "\n");
	printf( "Bits per pixel      %4d\n", vc.bitsperpixel);
	printf( "Memory in kilobytes %4d\n", vc.memory);
	printf( "No. color indeces   %4d\n", vc.numcolors);
	printf( "Text columns        %4d\n", vc.numtextcols);
	printf( "Text rows           %4d\n", vc.numtextrows);
	printf( "Video pages         %4d\n", vc.numvideopages);
	printf( "Pixels in row       %4d\n", vc.numxpixels);
	printf( "Pixels in column    %4d\n", vc.numypixels);
}

int docharacter(FILE *fp_in) {
	int fchr;
	char *font;

	if (sequenceflag != 0) {
		fchr = searchoutlines(fp_in, chrs);
		while (fchr < 0) {
			chrs = - fchr;	/* next one in sequence */
/*			chrs++; */
			if (chrs >= fontchrs) break;
			fchr = searchoutlines(fp_in, chrs);
		}
	}
	else {
		fchr = searchoutlines(fp_in, -1);
		if (fchr < 0) return -1;
		else chrs = fchr;			/* ? */
	}

	if (batchmode == 0  && dontredraw == 0) { 
		if (overlay == 0) _clearscreen(_GVIEWPORT);
		_moveto(0,0);
/*		if (fsubr > 0) */
		if (fsubr + subroffset > 0)
/*			sprintf(line, "\n%s: %d S %d", fontname, chrs, fsubr); */
/*			sprintf(line, "\n%s: %d S %d", fontname, chrs, fsubr + subroffset); */	
			sprintf(line, "%s: %d S %d", fontname, chrs, fsubr + subroffset);
/*		else sprintf(line, "\n%s: %d", fontname, chrs); */
		else sprintf(line, "%s: %d", fontname, chrs);
		if (largefont != 0) font = "t'helv'h28w16b";
		else font = "t'helv'h14w7b";
		if (_setfont(font) < 0) {		/* 1992/Dec/1 */
			if (_setfont("t'helv'h28w16b") < 0) {
				fprintf(stderr, "Failed to set font\n");
				showgrerror(_grstatus());
			}
		}
		_setcolor(textcol);
		_outgtext(line);
		sethintfont();						/* reset font size 1993/Jan/10 */
	} 
	if (automode == 0)	_setcolor(outlinecol);
	
	if (fchr < 0) return -1;

	xmin = INFINITY; ymin = INFINITY; xmax = -INFINITY; ymax = -INFINITY; 
	if (batchmode != 0 && verboseflag != 0) {
		printf("."); 
		if (chrs == fontchrs/2-1 || chrs == fontchrs-1 ) putchar('\n'); 
	}
	readoutline(fp_in);
	return 0;
}

void initializevga(void) {
	struct _videoconfig vc;
	long bkold, bknew;
	int k, n;
	short err;
	short cinx;
	short xnw, ynw, xse, yse;

	if (batchmode != 0) return;
	
/*	_setvideomode(_TEXTC80); */
/*	printf("Initialized TEXTC80\n"); */
/* 	getsafe(); */

	task = "initializing VGA";
/*	printf("Initializing VGA to MAXRESMODE\n"); */
/*	videomode = _MAXRESMODE; */		/* old method */

/*	videomode = _VRES2COLOR; */		/* 640 x 480, BW */
/*	videomode = _VRES16COLOR; */	/* WORKS NT */ /* 640 x 480, 16 color */
/*	videomode = _VRES256COLOR; */	/* 640 x 480, 256 color */
/*	videomode = _SRES16COLOR; */	/* 800 x 600, 16 color */
/*	videomode = _SRES256COLOR; */	/* 800 x 600, 256 color */
/*	videomode = _XRES16COLOR; */	/* 1024 x 768, 16 color */
/*	videomode = _XRES256COLOR; */	/* 1024 x 768, 256 color */
/*	videomode = _ZRES16COLOR; */	/* 1280 x 1024, 16 color */
/*	videomode = _ZRES256COLOR; */	/* 1280 x 1024, 256 color */
	printf("Initializing VGA to: ");
	showmode (videomode);
/*	getsafe(); */

	if (_setvideomode(videomode) == 0) {
		_setvideomode(_TEXTC80);		/* try and reset here ? */
		fprintf(stderr, "\nCouldn't set desired video mode (%d)", videomode);
		showgrerror(_grstatus());
		giveup(7);
	}
/*	printf("Initialized Video\n"); */
/*	getsafe(); */
/*	giveup(1); */
	
	_getvideoconfig( &vc );
	scolumns = vc.numxpixels;
	srows = vc.numypixels;
	numgrey = vc.numcolors; 
	numbits = vc.bitsperpixel;
	if (verboseflag != 0) {
		printf("%d columns %d rows %d grey %d bits \n", 
			scolumns, srows, numgrey, numbits); 
	}
	
/*	show video state on screen */
	_getvideoconfig( &vc );
	showstate(vc);
	err = _grstatus();
	if (err != 0) showgrerror(err);
	printf("\n");

	for (k = 0; k < 8; k++) {
		if ((k % 2) == 0) fillarr[k] = 170;
		else fillarr[k] = 85;
	}	

	xnw = 0; ynw = 0;
	xse = scolumns; 		/* depends on mode */
	yse = srows;			/* depends on mode */

	_setviewport(xnw, ynw, xse, yse);

/*	getsafe(); */
	bknew = background; /* 	bknew = RGB(20, 0, 20); */
/*	dark blue/magenta background */

	bkold = _getbkcolor();
	_setbkcolor(bknew);

 	cinx = _getcolor(); 
	_setfillmask(fillarr);

	_remappalette(4, brightred);	/* make red brighter */

	task = "registering fonts";
/*	if ((n =_registerfonts("c:\\c600\\source\\samples\\*.fon")) < 0) {
		fprintf(stderr, "Failed to register fonts %d\n", n);
	} */
	if ((n =_registerfonts(dosfonts)) < 0) {
		fprintf(stderr, "Failed to register fonts (%d) in %s\n", n, dosfonts);
		showgrerror(_grstatus());
/*		giveup(1); */
	}	/* 1992/Oct/1 */

}

/* void showusage (char *s) { */
void showusage (char s[]) {
	fprintf (stderr, "\
Correct usage is:\n\n\
%s [-{v}{m}{a}{x}{n}{t}{o}{b}{i}{r}{s}{z}{q}{w}{g}]\n\
\t[-c <char>] [-h <path>] [-e <ext>] <file-1> <file-2>...\n", s);
if (detailflag == 0) exit(1);
	fprintf (stderr, "\
\tv: 'verbose' mode (first screen only)\n\
\tm: make output hint file (enable editing - implies `x')\n\
\tx: do not superimpose substituted hints\n\
\ty: allow selection of subpaths, not hint replacement sections\n\
\tz: allow skipping forward in file while recording\n\
\to: do characters in order in file (rather than in ascending order)\n\
\ta: batch mode, no screen output (implies `m')\n\
\tu: do not show knots in unselected subpaths\n\
\tt: complain about coincident knots\n\
\tb: do not insert extrema lines as potential stem edges\n\
\ti: do not use vertical stems for italic and slanted characters\n\
\tq: allow snapto of strokes in hint file to strokes in outline\n\
\tw: show adjustments made to stems by snapto rules\n\
\tg: adjust ghost stems (bottom => 21 and top => 20)\n\
\tc: next argument is starting character number (disables hint output)\n\
\th: next argument is path for input hint file\n");
	fprintf(stderr, "\
\te: next argument is extension for output hint file (default '%s')\n", ext);
	fprintf(stderr, "\
\tV: next arg is videomode (default %d)\n", videomode);
	fprintf(stderr, "\
\t   If multiple files are specified, they are treated in batch mode\n");
	exit(1);
}

/* \td: don't guess hints\n\ */
/* \tx: use only x for computing screen scale\n\ */
/* \ty: use only y for computing screen scale\n\ */
/* \tv: verbose mode\n\ */
/* \tt: tracing mode (DEBUGGING) \n\ */
/* \tr: reverse paths (only needed for old Projective Solutions outlines)\n\ */
/* \ts: undo BSR scaling (only needed for BSR release 0.8 fonts)\n\ */


int decodeflag (int c) {
/*	printf ("FLAG: %c%n", c); */
	switch(c) { 
		case '?': detailflag = 1; return 0;
		case 'd': guessflag = 0; return 0;		/* no hint guessing */
/*		case 'x': readallhints = 1; recordflag = 0; return 0; */
		case 'x': readallhints = 1; return 0; 
		case 'y': selectpathflag = 1; return 0;
		case 'u': alwaysknots = 0; return 0;		
/*		case 'x': hvscale = 1; return 0; */
/*		case 'y': hvscale = -1; return 0; */
		case 'm': recordflag = 1; readallhints = 0; return 0;
		case 'i': showslanted = 0; return 0;
		case 'b': extremaflag = 0; return 0; 
		case 'r': reverseflag = 1; return 0;
		case 's': correctscale = 1; return 0;
		case 'g': ghostadjust = 1; return 0;
		case 'w': showadjust = 1; return 0;
		case 'o': sequenceflag = 0; return 0;
		case 'a': batchmode = 1; return 0; 
		case 'q': fuzzflag++; return 0;
		case 'Q': quietflag++; return 0;	/* 1994/July/10 */
		case 'l': largefont=1; return 0;	/* use large font */
		case 'f': smallfont=0; return 0;	/* do not use small font */
		case 'v': verboseflag = 1; return 0; 
		case 'z': suppresskip = 0; return 0;
		case 'Z': showstateflag = 1; return 0;
/*		case 't': traceflag = 1; return 0; */
		case 't': if (complaintriple != 0) complaindouble = 1;
				  else complaintriple = 1; return 0; 
/* following need argument */
		case 'e': extenflag = 1; break;
		case 'h': hintpathflag = 1; break;
		case 'S': zoomflag = 1; break;
		case 'C': hintcolorflag = 1; break;
		case 'c': charflag = 1; sequenceflag = 1; recordflag = 0; break;
		case 'V': videoflag = 1; break;
		default: {
				fprintf(stderr, "\nInvalid command line flag '%c'", c);
				giveup(7);
		}
	}
	return -1;				/* need argument */
}

int dochar(FILE *fp_in, FILE *fp_hnt) {
	int code;
	if (sequenceflag == 0) { 
		flast = ftell(fp_in); 	/* ? */
		if (hintflag != 0) 	hlast = ftell(fp_hnt);	/* ? */
	} 
/*	if (readallhints == 0) {				
		if (hintflag != 0) 	hlast = ftell(fp_hnt);	
	} */
	resetstrokes();
	code = docharacter(fp_in);
	if (code < 0) return code;
	if (dontredraw == 0) {
		if (hintflag != 0) chrs = readhints(fp_hnt, chrs); 
		else if (guessflag != 0) guesshints(); 
		showstrokes();
	}
	return 0;
}

int redochar(FILE *fp_in, FILE *fp_hnt) {
	if (fstart < 0) seekerror("redochar", "fp_in");
	else fseek(fp_in, fstart, SEEK_SET);	/* go back to start of this char */
	if (hintflag != 0) {
		if (hstart < 0) seekerror("redochar", "fp_hnt");
		else fseek(fp_hnt, hstart, SEEK_SET);
	}
/*	if (hintflag != 0) {
		if (readallhints == 0) fseek(fp_hnt, hlast, SEEK_SET); 
		else fseek(fp_hnt, hstart, SEEK_SET); 
	} */
	return dochar(fp_in, fp_hnt);
}

int newredochar(FILE *fp_in, FILE *fp_hnt) {
	long fptr;
	if (readallhints == 0) {	/* step back to start of character in output */
		if ((fptr = fileposition[chrs]) >= 0) {
			fseek(fp_out, fptr, SEEK_SET);
		}
	}
	if (sequenceflag == 0) {
		if (flast < 0) seekerror("newredochar", "fp_in");
		else fseek(fp_in, flast, SEEK_SET);
		if (hintflag != 0) 	{
			if (hstart < 0) seekerror("newredochar", "fp_hnt");
			else fseek(fp_hnt, hlast, SEEK_SET);
		}
		return dochar(fp_in, fp_hnt);
	}
	else return redochar(fp_in, fp_hnt);
}

/* void cleanup(void) { _setvideomode(_DEFAULTMODE); } */

#ifdef CONTROLBREAK
void cleanup(void) {
	if (recordflag != 0 && fp_out != NULL) {
		fprintf(fp_out, "\n%% INCOMPLETE STEM FILE\n");
		fclose(fp_out);		/* close output */
	}
	if (batchmode == 0) {
/*		_setvideomode(_DEFAULTMODE); */
		_setvideomode(_TEXTC80);			/* reset video mode to 80 x 25 */
		_unregisterfonts();
	}
}
#endif

/* #undef SIG_IGN */
/* #define SIG_IGN (void (__cdecl *)(int))1L */

/* void (__cdecl * __cdecl signal(int, void (__cdecl *)(int)))(int); */

#ifdef CONTROLBREAK
/* not clear whether the C style I/O calls (in cleanup) are safe ... */
/* maybe just set flags and then do this at higher level ? */
/* void ctrlbreak(void) { */
void __cdecl ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
/*	cleanup(); */
	abortflag++;
	if (abortflag > 4) exit(3);
/*	if (traceflag != 0) fprintf(stderr, "\nUser Interrupt\n");  */
/*	exit(3);  */
	(void) signal(SIGINT, ctrlbreak); 
}
#endif

#ifdef CONTROLBREAK
/* void cleanabort(void) { */
void __cdecl cleanabort(int err) {
	signal(SIGABRT, SIG_IGN);	/* disallow abort */
	abortflag++;
	if (abortflag > 4) exit(3);
/*	getsafe(); */				/* hang so user can read screen */
/*	cleanup(); */
/*	exit(3);  */
	(void) signal(SIGINT, cleanabort); 
}
#endif

int finishchar(void) {
	if (readallhints == 0) {
		if (fsubr == 0) fileposition[chrs] = ftell(fp_out);	
/*		record only position of start of character, not current replacement */
	}
	else fileposition[chrs] = ftell(fp_out);	
	if (writestrokes(fp_out) != 0) {
		if (readallhints == 0) {	/* increment character code if done */
			fsubr = isnextreplace(fp_hnt);	/* is next a hint replacement */
			if (fsubr == 0) {
				chrs++;				/* no, so step character code */
				subroffset = 0;		/* 94/June/4 ??? */
			}
/*			else selectnumber = fsubr; */		/* ??? */
			selectnumber = fsubr;				/* show only relevant part */
/*			printf("S %d ", selectnumber); */	/* debugging */
		}
		else {
			chrs++;
			subroffset = 0;		/* 94/June/4 ??? */
		}
/*		if (chrs >= fontchrs) {
			chrs = 0;
			if (sequenceflag != 0) break;
		} */
		return(dochar(fp_in, fp_hnt)); /* test ? here */
	}
	else { /* failed to write correct stem codes */
		scanmode = 0;  /*		batchmode=0; automode=0; */
		if (automode != 0 && batchmode == 0) {
			newredochar(fp_in, fp_hnt); /* ??? */
			automode = 0;
		}
		else if (batchmode == 0) showstrokes();
		return -1;
	}
}					

void centeroffset(int newscale, int oldscale) {
/*	xoffset = scolumns/2 - (int) (((long) (scolumns/2 - xoffset) * newscale) / oldscale);*/
	xoffset = scolumns/2 - (short) (((long) (scolumns/2 - xoffset) * newscale) / oldscale);
/*	yoffset = srows/2 - (int) (((long) (srows/2 - yoffset) * newscale) / oldscale);*/
	yoffset = srows/2 - (short) (((long) (srows/2 - yoffset) * newscale) / oldscale);
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

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
/*	date[20] = '\n'; */
/*	date[21] = '\0'; */
	date[20] = '\0'; 
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

/* strip down file name to path only */
void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
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
	while ((c = *sarg++) != '\0') {				/* until end of string */
		if (decodeflag(c) != 0) {				/* flag requires argument ? */
/*			if ((s = strchr(sarg, '=')) == NULL) { */
			if (*sarg != '=' && *sarg != ':') {	/* arg in same string ? */
				if (next != NULL) {
					firstarg++; s = next;	/* when `=' or `:' is NOT used */
				}
				else {
					fprintf(stderr, "Don't understand: %s\n", command);
					return firstarg;
				}
			}
/*			else s++;	*/		/* when `=' IS used */
			else s = sarg+1;	/* when `=' or `:' IS used */

/* now analyze the various flags that could have gotten set */
			if (hintpathflag != 0) {
				strcpy(hintpath, s); hintpathflag = 0;
			}
			else if (extenflag != 0) {
				ext = s; extenflag = 0;
			}
			else if (charflag != 0) {
				if (sscanf(s, "%d", &chrs) < 1) {
					fprintf(stderr, "\nExpected character number, not %s", 
						s);
					giveup(3);
				}
				charflag = 0;
			}
			else if (zoomflag != 0) {
				if (sscanf(s, "%lg", &zoomfactor) < 1) {
					fprintf(stderr, "\nExpected character number, not %s", 
						s);
					giveup(3);
				}
				zoomflag = 0;
			}
			else if (hintcolorflag != 0) {	/* 7 for white 1992/Dec/1 */
				if (sscanf(s, "%d", &hintcol) < 1) {
					fprintf(stderr, "\nExpected character number, not %s", 
						s);
					giveup(3);
				}
				hintcolorflag = 0;
			}
			else if (videoflag) {
				if (sscanf(s, "%d", &videomode) < 1) {
					fprintf(stderr, "\nExpected video mode, not %s", s);
				}
				videoflag = 0;
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

void underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = removepath(filename);
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

void removeunder(char *filename) { /* remove Adobe style underscores */
	char *s;
	s = filename + strlen(filename) - 1;
	while (*s == '_') s--;
	*(s + 1) = '\0';
}

int breakflag=0;

/* moved out of main to help compiler ... 94/June/5 */

int doloop(FILE *fp_out, FILE *fp_in, FILE *fp_hnt, int c) {
/*	int newscale, oldscale; */
	short newscale, oldscale;
	long fptr;
	int code=0;
	
	while (c != 256) {	
		if (abortflag != 0) c= CHR_ESCAPE;		/* 1994/May/21 */
		if (c == CHR_ESCAPE) {			/* until escape character seen */
			if (recordflag != 0) {
				putc(7, stdout);
				printf("\nWARNING: Are you sure you want to quit? ");
				printf("\nWARNING: The output hint file is incomplete. ");
				c = getsafe();			/* new */
				if (c == 'Y' || c == 'y' || c == CHR_ESCAPE || c < ' ') {
					breakflag++;
					break;
				}
				c = _getch();
/*				if (c == 0) c = 256; */
				continue;
			}
			breakflag++;
			break;
		}
/*		Fake arrow keys enable control and alt in Windows NT */
		if (c == 104 || c == 110 || c == 106 || c == 117) c = -c;
		if (c ==   8 || c ==  14 || c ==  10 || c ==  21) c = -c;
/*		if (c == 0) { */	/* special function key hit or scan mode */
		if (c <= 0) {		/* special function key hit or scan mode */
			if (c == 0) {
				if (scanmode == 0) {	/* not in scan mode */
					while ((c = _getch()) == 0) ; /* get second part of char */
					if (traceflag) printf("%d ", c);
					c += 256;		/* to distinguish from ordinary char */
				}
			}
			if (c < 0) c = -c;
			if (c == CHR_LEFTARROW || c == 'h') leftarrow();
			else if (c == CHR_RIGHTARROW || c == 'j') rightarrow();
			else if (c == CHR_DOWNARROW || c == 'n') {
				if (showcoords == 0) downarrow();
				else {
					currentnode--;
					dontredraw = -1;
					newredochar(fp_in, fp_hnt);
					dontredraw = 0;
				}
			}
/*			else if (c == CHR_UPARROW) { */
			else if (c == CHR_UPARROW || c == 'u') {
				if (showcoords == 0) uparrow();
				else {
					currentnode++;
					dontredraw = -1;
					newredochar(fp_in, fp_hnt);
					dontredraw = 0;
				}		
			}
			else if (c == CHR_F8) {				/* F-8 */
				if (minarrow > 1) minarrow = 1;
				else minarrow = 16;
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_F9) {				/* F-9 */
				if (recordflag == 0) {
					showcoords = (1 - showcoords);
					/*		labelnode = showcoords; */
					currentnode = 0;
					newredochar(fp_in, fp_hnt);
				}
			}
			else if (c == CHR_F10) {				/* F-10 */
				labelnode = (1 - labelnode);
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_C_F10) {				/* C-F-10 */
				cumulative = (1 - cumulative);
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_F11 || c == CHR_F7) {
										/* F-11 show curveto's as lineto */
				straighten = (1 - straighten);
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_F12 || c == CHR_F8) {
										/* F-12 increase step size */
				stepscale = (short) (stepscale * 2);
				if (stepscale < 0) stepscale = 1; /* ? */
			}
			else if (c == CHR_C_F12 || c == CHR_C_F8) {
										/* C-F-12 decrease step size */
/*				stepscale = (short) (stepscale / 2); */
				stepscale = (short) ((stepscale + 1) / 2);
				if (stepscale == 0) stepscale = 1;
			}
			else if (c == CHR_M_F12 || c == CHR_M_F8) {
										/* M-F-12 reset step size */
				stepscale = 1; /* ? */
			}
			else if (c == CHR_C_LEFTARROW || c == 8) {  /* control h */
				/*	xoffset = xoffset - CURSORINC * stepscale; */
				/*	xoffset = xoffset - 4 * scrscale * stepscale; */
				xoffset = xoffset - 4 * orgscale * stepscale; 
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_C_RIGHTARROW || c == 10) { /* control j */
				/*	xoffset = xoffset + CURSORINC * stepscale; */
				/*	xoffset = xoffset + 4 * scrscale * stepscale; */
				xoffset = xoffset + 4 * orgscale * stepscale; 
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_C_UPARROW || c == 21) {  /* control u */
				/*	yoffset = yoffset - CURSORINC * stepscale; */
				/*	yoffset = yoffset - 4 * scrscale * stepscale; */
				if (showcoords == 0) {
					yoffset = yoffset - 4 * orgscale * stepscale; 
					newredochar(fp_in, fp_hnt);
				}
				else currentnode++;
			}
			else if (c == CHR_C_DOWNARROW || c == 14) { /* control n */
				/*	yoffset = yoffset + CURSORINC * stepscale; */
				/*	yoffset = yoffset + 4 * scrscale * stepscale; */
				if (showcoords == 0) {
					yoffset = yoffset + 4 * orgscale * stepscale;
					newredochar(fp_in, fp_hnt);
				}
				else currentnode--;
			}					
			else if (c == CHR_C_PLUS || c == CHR_C_F5) {  /* control + */
				/*	scrscale = (int) ((double) scrscale * 1.2); */
				oldscale = scrscale;
				newscale = (short) ((scrscale * 12 + 5) / 10);
				if (newscale == scrscale) newscale++;
				if (newscale > 1024) scrscale = 1024;
				else scrscale = newscale;	
				centeroffset(scrscale, oldscale);
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_M_PLUS || c == CHR_M_F5) {	/* meta + */
				oldscale = scrscale;
				newscale = (short) (scrscale * 2);
				if (newscale == scrscale) newscale++;
				if (newscale > 1024) scrscale = 1024;
				else scrscale = newscale;	
				centeroffset(scrscale, oldscale);
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_C_MINUS || c == CHR_C_F6) { /* control - */
				/*		scrscale = (int) ((double) scrscale / 1.2); */
				/*		scrscale = scrscale - stepscale; */
				oldscale = scrscale;
				newscale = (short) ((scrscale * 10 + 6) / 12);
				if (newscale == scrscale) newscale--;
				if (newscale <= 0) scrscale = 1;
				else scrscale = newscale;
				centeroffset(scrscale, oldscale);
				newredochar(fp_in, fp_hnt);
			}
			else if (c == (256 + 74)) { /* ??? */
				/*		oldscale = scrscale; */
				scrscale = orgscale;
				/*		centeroffset(scrscale, oldscale); */
				xoffset = xorgset;
				yoffset = yorgset;						
				newredochar(fp_in, fp_hnt);
			}
			else if (c == CHR_PGUP) {				/* Pg Up */
				scanmode = 1; automode = 1; c = 0;
			}
			else if (c == CHR_PGDN) {  					/* Pg Dn */
				scanmode = 1; c = 0;
			}
			else if (c == CHR_HOME) { 				/* Home */
				if (readallhints == 0) {
					/*				fsubr = 0; */
					selectnumber = 0;			/* back to base hints */ 
				}
				else selectnumber = -1;			/* old way */
				currentnode = 0;
				if (sequenceflag != 0) {
					if (readallhints == 0) {
						/* if in middle of hint replacement ? */
						/* otherwise step back to previous character */
						if (fsubr == 0) chrs--;
						/* step back to start of this character */
						else fsubr = 0;
						subroffset = 0;		/* 94/June/4 */
					}
					else chrs--;	
					if (chrs < 0) chrs = fontchrs - 1;
/*					fsubr = -1;			/* ??? */
					fsubr = 0;			/* ??? */
					subroffset = 0;		/* 94/June/4 */
					oldchrs = -1;
					if ((fptr = fileposition[chrs]) >= 0) {
						fseek(fp_out, fptr, SEEK_SET);
					}
/*					else printf("file %ld for char %d ", fptr, chrs); */
					newredochar(fp_in, fp_hnt);
				}	/* end of sequenceflag != 0 case */
/*				else  {
					fseek(fp_in, flast, SEEK_SET);
					if (hintflag != 0) fseek(fp_hnt, hlast, SEEK_SET);
					dochar(fp_in, fp_hnt);
				} */
			} /* end of `Home' case */
			else if (c == 0 || c == CHR_END) { 			/* End */
				selectnumber = -1;
				currentnode = 0;
				code = finishchar();				/* may set selectnumber */
				if (chrs >= fontchrs) {
					chrs = 0;
					if (sequenceflag != 0) break;	/* finished */
				} 
/*				subroffset = 0;	*/	/* 94/June/4 ??? */
			}
/*			else if (c == CHR_F9) {		F-9 
				chrs = _getch();
				code = dochar(fp_in, fp_hnt);
			} */
			else {		/*  rest may require rewrite of stem widths */
				showstemwidths(0);		/* remove old stem widths */
				if (c == CHR_INSERT || c == CHR_F1) insert(); 
				else if (c == CHR_DELETE || c == CHR_F2) delete(); 
				else if (c == CHR_C_INSERT || c == CHR_C_F1) ghostinsert(); 
				else if (c == CHR_C_DELETE || c == CHR_C_F2) stemdelete();
				else if (c == CHR_M_LEFTARROW || c == CHR_M_H)	/* Alt-h */
					metaleft();	
				else if (c == CHR_M_RIGHTARROW || c == CHR_M_J)	/* Alt-j */
					metaright(); 
				else if (c == CHR_M_DOWNARROW || c == CHR_M_N)	/* Alt-n */
					metadown();
				else if (c == CHR_M_UPARROW || c == CHR_M_U)	/* Alt-u */
					metaup(); 
#ifdef EXTRASTUFF
				else if (c == CHR_F1) {			/* F-1 set all strokes */
					setstrokes();	showstrokes();
				}
				else if (c == CHR_C_F1) {		/* C-F-1 clear all strokes */
					clearstrokes(); showstrokes();
				}
				else if (c == CHR_F2) {			/* F-2 show vstem in slanted */
					showslanted = 1; newredochar(fp_in, fp_hnt);
				}
				else if (c == CHR_C_F2) {		/* C-F-2 do not show vstem in */
					showslanted = 0; newredochar(fp_in, fp_hnt);
				}
#endif
				else if (c == CHR_F3) { 		/* F-3 GUESSS HINTS ! */
					clearstrokes(); guesshints(); showstrokes();
				}
				else if (c == CHR_F4 && hintflag != 0) {	/* F-4 */
					clearstrokes();				/* REREAD HINTS for this one */
					if (hstart < 0) seekerror("F-4", "fp_hnt");
					else fseek(fp_hnt, hstart, SEEK_SET);
					readhints(fp_hnt, chrs);
					showstrokes();
				}
#ifdef FANCYSTUFF
				else if (c == CHR_F5) {		/* F-5 insert lines at extrema */
					extremaflag = 1; newredochar(fp_in, fp_hnt);
				}
				else if (c == CHR_C_F5) {	/* C-F-5 remove lines at extrema */
					extremaflag = 0; newredochar(fp_in, fp_hnt);
				}
				else if (c == CHR_F6) {		/* F-6 level almost horizontal */
					levelflag = 1;	newredochar(fp_in, fp_hnt); 
				}
				else if (c == CHR_C_F6) {	/* C-F-6 do not level almost ... */
					levelflag = 0;	newredochar(fp_in, fp_hnt); 
				}
				else if (c == CHR_F7) {		/* F-7 combine almost coliner */
					combineflag = 1; newredochar(fp_in, fp_hnt); 
				}
				else if (c == CHR_C_F7) {	/* C-F-7 do not combine almost .. */
					combineflag = 0; newredochar(fp_in, fp_hnt); 
				}
				else if (c == CHR_F8) { 				/* F-8 */
					flattenflag = 1; newredochar(fp_in, fp_hnt);
				}
				else if (c == CHR_C_F8) { 				/* C-F-8 */
					flattenflag = 0; newredochar(fp_in, fp_hnt);
				}
#endif
				else if (c == CHR_F9) { 				/* F-9 */
					overlay = -1;
				}
				else if (c == CHR_C_F9) { 				/* C-F-9 */
					overlay = 0;
				}						/* If none of the above: */
				else if (c == CHR_M_0) {				/* alt-0 */
					selectnumber = 0;
					newredochar(fp_in, fp_hnt);
				}
/*				else if (c >= 120 && c <= 128) { */	/* alt-digit */
				else if (c >= (256+120) && c <= (256+128)) {	/* alt-digit */
/*					selectnumber = c - 119; */
					selectnumber = c - (256+119);
					newredochar(fp_in, fp_hnt)
;
				}
				else if (c == CHR_M_INSERT || c == CHR_M_F1) { 	/* alt-insert */
					cursorx = -INFINITY; cursory = -INFINITY;	/* 94/Nov/10 */
					writestrokes(fp_out);	
					showstemwidths(1);
					subroffset++;
				} 
				else if (c == CHR_M_DELETE || c == CHR_M_F2) {	 	/* alt-delete */
					showstemwidths(1);
					subroffset--;
					if (subroffset < 0) subroffset = 0;
				} 
				else printf ("You typed the character 0-%d\n", c);
			}
		}	/* end of if (c == 0) special character type */
		/*		} */
		else if (c == ' ') {			/* space */
			if (readallhints == 0) {	/* 1994/May/21 */
				if (selectnumber < 0) selectnumber = 0;
				else selectnumber = -1; 					
				fsubr = 0;
				subroffset = 0;		/* 94/June/5 */
			}
			if (readallhints != 0) 	selectnumber = -1;
			newredochar(fp_in, fp_hnt);
		}
		else if (c == -1) dochar(fp_in, fp_hnt); /* prime the pump */
		else if (c >= 0) {			/* not a special function character */
			if (recordflag == 0) {	/* just jump if not recording */
				selectnumber = -1;
				currentnode = 0;
				chrs = c;
				code = newredochar(fp_in, fp_hnt);
			}
			else if (suppresskip == 0) { /* if recording - skip forward */
/*				if (chrs > c) {
					if (automode != 0) c = chrs + 1;
					else c = chrs;
				} */ 		/* an experiment 96/July/10 */
				if (chrs > c) {
/*					do nothing, since already past that character */
/*					printf("chrs (%d) >= c (%d) ", chrs, c); */
					c = 0;			/* 96/July/7 ??? */
					scanmode = 0;	/* 96/July/7 ??? */
					automode = 0;	/* 96/July/7 ??? */
					finishchar();	/* 96/July/7 ??? */
				}
				else if (chrs == c) {
/*					printf("chrs (%d) == c (%d) ", chrs, c); */
/*					stop scan mode ? when chars == c */
					c = 0;
					scanmode = 0; /* automode = 0; */
				}
				else {
/*					printf("chrs %d < c (%d) ", chrs, c); */
/*					go into scan mode --- up to char c */
					scanmode = 1;
					automode = 1;
/*					stop auto mode one character before chrs == c */
					if (chrs == c - 1) automode = 0;
					finishchar(); 
				}
			}
		} 
/*		if (scanmode != 0) {
			code = dochar(fp_in, fp_hnt);
			chrs++;	
			subroffset = 0;	
			if (chrs >= fontchrs) {
				chrs = 0;
				if (sequenceflag != 0) break;
				} 
		} */
		if (batchmode != 0) c = 0;
		if (scanmode == 0) {
			c = _getch();
			if (traceflag) printf("%d ", c);
/*			if (c == 0) c = 256; */
		}
		if (sequenceflag == 0 && code < 0) break;
	}	/* end of while (c != 256) */
	
	return c;
}

int main(int argc, char *argv[]) {       /* main program */
/* command line usage is: showchar [-c <char>] <file-1> <file-2> ... */
	static char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	static char fn_bak[FILENAME_MAX], fn_hnt[FILENAME_MAX];
	char *s;
	int c, k, m, firstarg=1;
/*	int newscale, oldscale; */
/*	long fptr; */
/*	int code; */
/*	int breakflag=0; */
/*	unsigned int i; */
	time_t ltime;		/* for time and date */
	struct _videoconfig vc;	/* for video configuration */

	abortflag=0;

	task = "interpreting command line";

	chrs = 0; 

	if (argc == 1) showusage(argv[0]); 

#ifdef CONTROLBREAK
	(void) signal(SIGINT, ctrlbreak);
	(void) signal(SIGABRT, cleanabort); 
#endif

	firstarg = commandline(argc, argv, 1); /* check for command flags */

	if (showstateflag) {
		_getvideoconfig( &vc );
		showstate(vc);
		getsafe();
	}

	/* more than one file specified */
	if (argc > firstarg + 1 || batchmode != 0) {
		batchmode = 1; automode = 1; scanmode = 1; showhintline = 0;
		recordflag = 1;
		chrs = 0;				/* ignore starting character if any */
	}
	/* batchmode => automode => scanmode */

	if (firstarg >= argc) showusage(argv[0]); /* left out something ? */

	printf( "CM outline font hinting and display program version %s\n",	
		VERSION);

	if (argc == firstarg + 1) { /* only one file specified */
		if (strchr(argv[firstarg], '*') != NULL) { /* unexpanded wild card */
			fprintf (stderr, "\nFile not found: %s", argv[firstarg]);
			giveup(2);
		}
	}		

	if (batchmode == 0) initializevga();

	for (m = firstarg; m < argc; m++) { /* do each file in command line */

		task = "initializing files";

		for (k = 0; k < MAXCHRS; k++) fileposition[k] = -1;		/* NEW */

		strcpy(fn_in, argv[m]);			/* copy in file name */
		extension(fn_in, "hex");		/* default extension */

		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
/*		strcpy(fn_out, s); */  	/* copy input file name minus path */
/*		forceexten(fn_out, ext); */	/* add output extension */
	
		task = "opening file";

/* open input early to get correct file name for hint and output files */

/*		if ((fp_in = fopen(fn_in, "r")) == NULL) { */
		if ((fp_in = fopen(fn_in, READ_CODE)) == NULL) {
			underscore(fn_in);
/*			if ((fp_in = fopen(fn_in, "r")) == NULL) { */
			if ((fp_in = fopen(fn_in, READ_CODE)) == NULL) {
				putc('\n', stderr); perror(fn_in); giveup(2);
			}
		}

		strcpy(fn_out, s);  	/* copy input file name minus path */
		forceexten(fn_out, ext);	/* add output extension */

		if (hintflag == 0) {	/* 1992/Aug/29 */
			strcpy(hintpath, argv[m]);
			stripname(hintpath);
			hintflag++;
			printf("WARNING: Using HEX file path for HNT file path\n");
		}

		if (hintflag != 0) { /* was input hint file path specified ? */ 
			strcpy(fn_hnt, hintpath);
			if (strcmp(fn_hnt, "") != 0) strcat(fn_hnt, "\\");	/* 92/Sep/29 */
			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, ':')) != NULL) s++;
			else s = fn_in;
			strcat(fn_hnt, s);  	/* catenate input file name minus path */
			forceexten(fn_hnt, "hnt"); 	/* add extension */
		}

/*		task = "opening file";

		if ((fp_in = fopen(fn_in, READ_CODE)) == NULL) {
			underscore(fn_in);
			if ((fp_in = fopen(fn_in, READ_CODE)) == NULL) {
				putc('\n', stderr); perror(fn_in); giveup(2);
			}
		} */

		if (recordflag != 0) {
			if (chrs > 0) {			/* starting character specified */
/*				if ((fp_out = fopen(fn_out, "a")) == NULL) { */
				if ((fp_out = fopen(fn_out, READ_WRITE)) == NULL) {
					putc('\n', stderr); perror(fn_out); giveup(2);
				}
			}
			else {
/*				if ((fp_out = fopen(fn_out, "r")) != NULL) { */
				if ((fp_out = fopen(fn_out, READ_CODE)) != NULL) {
					fclose(fp_out);
					fprintf(stderr, 
					"\nWARNING: Output stem file %s already exists\n", fn_out);
/*					putc('\a', stderr); */
					strcpy(fn_bak, fn_out);
					forceexten(fn_bak, "bak");
					fprintf(stderr, "\nRename as backup file %s? [Y]\n", 
						fn_bak);
/*					may lead to problem if this is the input file also ! */
					c = getsafe();
					if (c != '\n' && c != '\r' && 
						c != ' ' && c != '\t' &&
						c != 'y' && c != 'Y') {
						fprintf(stderr, "Terminating ");
						giveup(17);
					}
					if (strcmp(fn_out, fn_hnt) == 0) {
						strcpy(fn_hnt, fn_bak);	/* attempt to fix problem */
					}
					rename(fn_out, fn_bak);
					putchar('\n');
				}
/*				if ((fp_out = fopen(fn_out, "w")) == NULL) { */
				if ((fp_out = fopen(fn_out, WRITE_CODE)) == NULL) {
					putc('\n', stderr); perror(fn_out); giveup(2);
				}
			}
		}

		if (hintflag != 0) {
/*			if ((fp_hnt = fopen(fn_hnt, "r")) == NULL) { */
			if ((fp_hnt = fopen(fn_hnt, READ_CODE)) == NULL) {
				if (recordflag == 0) {	/* 1992/Sep/25 */
					forceexten(fn_hnt, "stm");
					fprintf(stderr, "\nTrying STM file instead of HNT\n");
					putc('\n', stderr); 
				}
/*				if ((fp_hnt = fopen(fn_hnt, "r")) == NULL) { */
				if ((fp_hnt = fopen(fn_hnt, READ_CODE)) == NULL) {
					putc('\n', stderr); perror(fn_hnt); 
/*					putc('\a', stderr);  */
					hintflag = 0;
					getsafe();
/*					giveup(2); */
				}
			}
		}

		if (batchmode != 0 )
			printf("Processing font file %s\n", fn_in); 
/*			printf("Fontname = %s\n", fontname); */

/*		printf("CHARACTER %d\n", chrs); */
		maxknts=0;	doubleknots= 0;	tripleknots=0;
		doheader(fp_in);  		/* deal with the header of font file */
		fstart = ftell(fp_in);  /* remember place in file */
		if (recordflag != 0) {
			time(&ltime); /* get seconds since 1970 */
			s = ctime(&ltime); 
			lcivilize(s);					/* ??? */
			fprintf(fp_out, "%% %s %s \n", fn_in, s);
		}
		if (hintflag != 0) {
			scantochar(fp_hnt, fp_out);
			hstart = ftell(fp_hnt);
		}
/*		printf("CHARACTER %d\n", chrs); */
		
		if (batchmode == 0) { 
			setscreenscale();  
			if (verboseflag != 0) 
			 printf("boldflag=%d, italicflag=%d, sansflag=%d, typeflag=%d\n", 
					boldflag, italicflag, sansflag, typeflag); 
/*			getsafe(); */	/* wait for user to see screen before going	on */
		}
		c = -1;							/* first time through */
/*		if (scanmode != 0) c = ' ';		*/
		if (scanmode == 0) getsafe();	/* let user see page */
/*		code = 0; */
		if (sequenceflag == 0) {
			flast = ftell(fp_in);		/* ? */
			if (hintflag != 0) hlast = ftell(fp_hnt);	/* ? */
		}
/*		if (readallhints == 0) {	
			if (hintflag != 0) hlast = ftell(fp_hnt);	
		} */

/*		printf("CHARACTER %d\n", chrs); */

		c = doloop(fp_out, fp_in, fp_hnt, c);

		if (breakflag != 0) {
			if (recordflag != 0 && fp_out != NULL) 
				fprintf(fp_out, "\n%% INCOMPLETE STEM FILE\n");
		}

		if (batchmode != 0 && verboseflag != 0) {
/*			printf("Reached end of font (or escaped)  %s\n", fontname); */
/*			printf("m = %d argc = %d\n", m, argc); */
		}

		task = "closing files";

		assert (fp_in != NULL);
		fclose(fp_in);
		if (recordflag != 0) {
			assert (fp_out != NULL);
			if (ferror(fp_out) != 0) {
				perror(fn_out);
			}
			fclose (fp_out);
		}
		if (hintflag != 0) {
			assert (fp_hnt != NULL);
			fclose (fp_hnt); 
		}
	
		if (batchmode != 0 && verboseflag != 0) putchar('\n');
/*			printf("Closed file %s\n", fn_in); */

/*		resetstrokes(); */
		chrs = 0; 

	}	/* end of for loop stepping through files on command line */

	if (batchmode == 0) {
/*		_setvideomode(_DEFAULTMODE); */
		_setvideomode(_TEXTC80);			/* reset video mode to 80 x 25 */
		_unregisterfonts();
	}

	if (argc - firstarg > 1) {
		printf("Processed %d font files\n", argc - firstarg);
	}
		
	if (breakflag != 0) return breakflag;
	else return 0;
}

/* Use _getch() codes as follows: */
/* <- 0-75, -> 0-77, ^ 0-72, v 0-80, Home 0-71, End 0-79, */
/* PgUp 0-73, PgDn 0-81, */
/* Ins 0-82, Del 0-83 */ /* C-Ins 0-146  C-Del 0-147 */
/* F-1 0-59 ... F-10 0-68 */ /* F-11 0-133 F-12 0-134 */
/* C-F-1 0-94 ... C-F-10 0-103 */

/* no vstems for italic and slanted fonts ? */
/* but watch for math italic numerals - cmmi<n> 40 -- 63 */
/* also no vstems for calligraphic symbols in math symbol font 64 -- 90 ? */
/* actually, the calligraphic symbols shouldn't have ANY stems ? */

/* INTERACTIVE MODE FUNCTION KEYS: */

/* left arrow	move cursor to vertical stroke next left */
/* right arrow	move cursor to vertical stroke next right */
/* down arrow	move cursor to horizontal stroke next down */
/* up arrow		move cursor to horizontal stroke next up */
/* Ins			flag this stroke */
/* Del			remove flag from this stroke */
/* End			write character level hint and go to next character */
/* Home			return to previous character */
/* Pg Dn			leave interactive mode, but still display characters */ 
/* Pd Up			leave interactive mode, do not display characters */
/* escape		emergency exit, stop analysis at this point, close files */
/* Control Ins		generate ghost stem, and flag both strokes */
/* Control Del		remove both strokes of a stem if possible */
/* Alt left arrow	displace vertical stroke to left by one */
/* Alt right arrow	displace vertical stroke to right by one */
/* Alt up arrow		displace horizontal stroke up by one */
/* Alt down arrow	displace horizontal stroke down by one */
/* N-+			zoom in one step (magnify) */
/* N--			zoom out one step (demagnify) */
/* M-N-- M-N-+		reset zoom and offsets to original values */
/* Control left arrow	move screen image further left */
/* Control right arrow	move screen image further right */
/* Control down arrow	move screen image lower down */
/* Control up arrow		move screen image higher up */

/* F-1			set flags on all stroke positions */
/* C-F-1		clear flags on all stroke positions */

/* F-2 			show vstrokes even if italic or slanted font OBSOLETE */
/* C-F-2		show vstrokes only if not italic/slanted (default) OBSOLETE*/
/* F-3 			show strokes flagged by guessing algorithm */
/* F-4			show strokes flagged in hinting file - if supplied */
/* F-5			insert strokes for bbox */
/* C-F-5		do not insert strokes for bbox (default) */
/* F-6			level almost horizontal lineto's */
/* C-F-6		do not level almost horizontal lineto's (default) */
/* F-7 			combine almost colinear lineto's */
/* C-F-7		do not combine almost colinear lineto's (default) */
/* F-8			linearize almost linear curveto's */
/* C-F-8		do not linearize almost linear curveto's (default) */

/* F-12			double increments used for control-arrow & alt-arrow */
/* C-F-12		halve increments used for control-arrow & alt-arrow */
/* M-F-12		reset increments used for control-arrow & alt-arrow to 1 */

/* ADD FILL OF BODY ? To do this: */
/* do stroking in two parts, marks first, then lines - for full char */
/* collect points that are inside from each closed path */
/* start floodfill at end of character from each such point */
/* paint the outline only AFTER painting all the knots */
/* too hairy - interferese with too many other things */

/* due to historical accident (reversed form of old Projective Solutions) */
/* upstroke and downstroke are systematically interchanged ... */

/* make stroke at level with opposing strokes ineligible for stem ? OK */

/* can't redraw without destroying stroke information OK dealt with */

/* maybe redraw old stem width in background color to delete it ?  OK !*/

/* possible recognize vstem3 and hstem3 automatically ? OK ! */

/* ghost stem strokes should introduce new strokes. OK */

/* when a ghoststem s = e = 0 is turned off, remove it from array ? NO */

/* show wide stem widths also ? */

/* add ALL extrema as possibilities (of zero length) ? OK, I think */

/* Remember: for  BSR fonts need to use -r and -s flags ! */

/* Remember: for old Projective Solution outlines need to use -r flag */

/* allow flexibility in number of characters defined in font. OK */

/* present kinky extrema also as possible strokes. OK ?*/

/* insert ghost stems where there are no strokes at extremes already ? */

/* if no vertical strokes at all, make one very wide vertical stem ? */

/* if top or bottom is not part of some stroke make a ghost stroke ? */

/* rescaling not usually needed anymore - taken care off in CONVERT */

/* Allows limiting backing up in output file when `Home' is used */

/* Can skip forward while editing existing file - type character to skip to */

/* in ghostinsert may want to check whether one leg already flagged ? */

/* in stemdelete may want to check whether both legs actually flagged ? */

/* possibly allow for comment line in HEX file ? */

/* should ghost stems participate in hstem3 decision ? */

/* Typical use: showchar -vmq -h c:\bsr c:\bsr\cmr10.hex */

/* either remove some stuff or use medium model (COMPILEM) */

/* problem with snapto (fuzzflag) of ghost stems */

/* NOTE: has location of *.fon files wired in !!! */

/* 0 black, 1 blue, 2 green, 3 cyan, 4 red, 5 magenta, 6 brown, 7 white */ 
/* 8 gray, 9 lblue, 10 lgreen, 11 lcyan, 12 lred, 13 lmagenta, 14 yellow */

/* should really prescan to find out where characters */
/* should really get rid of some code to make space */

/* preserve hstem3 vstem3 decisions in input file ? */

/* deal with hint replacement subrs */

/* alt-Insert adds a hint replacement line */

/*
showchar.obj:  '__getlinestyle' : unresolved external
showchar.obj:  '__setfillmask' : unresolved external
showchar.obj:  '__setvideomode' : unresolved external
showchar.obj:  '__outgtext' : unresolved external
showchar.obj:  '__getvideoconfig' : unresolved external
showchar.obj:  '__getbkcolor' : unresolved external
showchar.obj:  '__lineto' : unresolved external
showchar.obj:  '__setfont' : unresolved external
showchar.obj:  '__moveto' : unresolved external
showchar.obj:  '__setlinestyle' : unresolved external
showchar.obj:  '__getcolor' : unresolved external
showchar.obj:  '__getcurrentposition' : unresolved external
showchar.obj:  '__setbkcolor' : unresolved external
showchar.obj:  '__ellipse' : unresolved external
showchar.obj:  '__clearscreen' : unresolved external
showchar.obj:  '__remappalette' : unresolved external
showchar.obj:  '__unregisterfonts' : unresolved external
showchar.obj:  '__setgtextvector' : unresolved external
showchar.obj:  '__setviewport' : unresolved external
showchar.obj:  '__registerfonts' : unresolved external
showchar.obj:  '__setcolor' : unresolved external
*/

/* rem c:\windev\bin\link /noi /noe %1.obj
c:\windev\lib\setargv.obj,,,graphics.lib; */

/* rem c:\windev\bin\link /noi /noe %1.obj
c:\windev\lib\setargv.obj,,,graphics.lib; */

/* LINK with graphics.lib use compileg.bat */

/* _MAXRESMODE -3 _MAXCOLORMODE -2 DEFAULTMODE -1 */
/* _VRES2COLOR 17 _VRES16COLOR 18 */
/* _ORES256COLOR 256 _VRES256COLOR 257 */
/* _SRES16COLOR 258 _SRES256COLOR 259 */
/* _XRES16COLOR 260 _XRES256COLOR 261 */
/* _ZRES16COLOR 262 _ZRES256COLOR 263 */
