/* Copyright 1991,1992,1993,1994,1995,1996,1997,1998,1999 Y&Y, Inc.
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

#ifndef _WINDOWS
#define _WINDOWS
#endif

#define NOCOMM
#define NOSOUND
#define NODRIVERS

#define STRICT

#include "windows.h"
#include "windowsx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include "dviwindo.h"
#include "winextra.h"
#include "winhead.h"
#include "winbased.h"

#pragma hdrstop

/* stdlib.h not really needed - just for hdrstop */

#include <dos.h>
#include <memory.h>
#include <math.h> 			/* for sine and cosine */

#include "winspeci.h"

#pragma warning(disable:4100)	// unreferenced formal parameters

#define USEMEMCPY

#define DEBUGDIB

/* #define DEBUGTIFF */

/* #define DEBUGBMP */

/* #define DEBUGHYPER */

/* #define DEBUGMETAFILE */

/* #define DEBUGEPSPATH */	/* debugging only */

/* #define DEBUGREADDVIPS */

/* #define DEBUGCOLORSTACK */

/**********************************************************************
*
* DVI \special function implementation
*
* Copyright (C) 1991, 1992 Y&Y. All Rights Reserved. 
*
* DO NOT COPY OR DISTRIBUTE!
*
* This implements text and rule colors and figure insertion
*
**********************************************************************/

#define MAXIMAGEDIMEN 65536		/* maximum rows & columns permitted */
#define MAXIMAGESIZE 67108864	/* maximum BytesPerRow x rows */

/* #define MAXIMAGEDIMEN 4096 */
/* #define MAXIMAGESIZE 4194304 */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

#define PALVERSION  0x300	/* Windows 3.0 palette version */
#define MAXPALETTE	256		/* max. # supported palette entries */

int allowclip = 1;		/* read BBox from EPS file and allow clipping */

int tweakdash=1;		/* adjust dashes in TPIC */
int tweakdot=1;			/* adjust dots in TPIC */

COLORREF OldTextColor, SavedTextColor, SavedBkColor;
COLORREF FigureColor, BackColor, OldFigureColor, OldBackColor;

COLORREF RuleColor;						/* 94/July/5 used in winanal.c */

int captionindex=-1;				/* index into Caption Fonts table */

int captionttf=0;					/* is Caption font TrueType ? */

/* Use instead FakeFonts[] in dviwindo.c with offset to step over serifed */

int CaptionFontSize=8 * 20;

/* BOOL bBoxFigure=0; */	/* draw box around figure */

HGLOBAL hImage;				/* handle to memory for image bytes */

LPSTR lpImageBytes;		/* FAR pointer to start of Image Data */

char buttonlabel[MAXMARKS+1]="";	/* label of last button hit */

char line[MAXLINE];		/* buffer for reading lines from EPS file */

char moreline[MAXLINE];		/* buffer for reading more stuff from EPS file */

int lookup[256];		/* remapping table for color stretch */

long nspecialsav;		/* saved byte count of special */
long specstart;			/* saved start of \special for error message */

/*	Is this assuming standard dvi_num and dvi_den ??? */

double scaledpoint = (65536.0 * 72.27) / 72.0;

int prewidth=0, preheight=0, prebits=0, prelines=0;	/* 92/Jan/2 */

int rule_r, rule_g, rule_b;				/* rule color saved for TPIC */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** *** */

POINT FAR *lpTPIC;				/* FAR pointer to TPIC path table */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** *** */ 

/* Procedure specific to TIFF file format: */
/* Procedure specific to WMF file format: */	/* 95/Oct/1 */
#ifdef LONGNAMES
int readtifffile(HDC, HFILE, int, int, int, int, int, int);
int readwmffile(HDC, HFILE, int, int, int, int);
int readbmpfile(HDC, HFILE, int, int, int, int, int);
#else
int readtifffile(HDC, FILE *, int, int, int, int, int, int);
int readwmffile(HDC, FILE *, int, int, int, int);
int readbmpfile(HDC, FILE *, int, int, int, int, int);
#endif

/* procedure specific to MetaFile format: */

void showmetafile(HDC, char *, double, double, int);		/* 96/Apr/4 */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** *** */ 

/* Procedure specific to EPSI file format: */

#ifdef LONGNAMES
int readepsipreview(HDC, HFILE, int, int, int, int, int);
int renderimage(HDC, HFILE, int, int, int, int);
int getline(HFILE, char *);
#else
int readepsipreview(HDC, FILE *, int, int, int, int, int);
int renderimage(HDC, FILE *, int, int, int, int);
int getline(FILE *, char *);
#endif

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

static char *modname = "WINSPECI"; 

/* static char *modname = __FILE__; */

static void winerror(char *message) {
	HWND hFocus;
	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox(hFocus, message, modname, MB_ICONSTOP | MB_OK);	
}

/* using this while repainting can lead to looping and `BusyRepaint' */
/* try and use winbadimage or winbadspecial instead */

static int wincancel (char *message) {
	HWND hFocus;
	int flag;

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, message, modname, MB_ICONSTOP | MB_OKCANCEL);
	if (flag == IDCANCEL) {
		bShowFlag = 0;				/* turn off displaying */
		finish = -1;				/* set early termination */
		bUserAbort = 1;				/* stop printing */
		return -1;
	}
	return 0;
}

void winbadimage (char *message) {		/* 95/April/6 */
	if (bBadFileComplained++ == 0) {
		wincancel (message);
/*		bBadFileComplained++; */
	}
	else {							/* 96/Feb/4 */
		bShowFlag = 0;				/* turn off displaying */
		finish = -1;				/* set early termination */
		bUserAbort = 1;				/* stop printing */
	}
}

void winbadspecial (char *message) {		/* 95/Feb/4 */
	if (bBadSpecialComplained++ == 0) {
		wincancel (message);
/*		bBadSpecialComplained++; */
	}
	else {							/* 96/Feb/4 */
		bShowFlag = 0;				/* turn off displaying */
		finish = -1;				/* set early termination */
		bUserAbort = 1;				/* stop printing */
	}
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* Attempt to speed up EPSI preview reading */ /* modified 95/Dec/1 */

#define EPSISPEED 1

#ifdef EPSISPEED

#define EPSIBUFFERLEN 256

/* stuff for speeding up reading of EPSI preview - not worth it ? */

/* static char buffer[BUFFERLEN]; */	/* buffer for low level routines */

static char *lpspcBuffer=NULL;		/* buffer for low level routines */

static char *spcbufptr;		/* pointer to next byte to take out of buffer */

static int spcbuflen;			/* bytes still left in str buffer */

/* static int ungotten=-1; */		/* result of unwingetc - or -1 */

/* assumes only one file open at a time */ /* attempt to speed up readepsi */

/* versions of getc and ungetc using low-level C input/output */

/* static void unwingetc(int c, int input) { 
	if (ungotten >= 0) {
		wincancel("Repeated unwingetc"); errcount();
	}
	else filepos--;
	ungotten = c;
} */

#ifdef LONGNAMES
static int replenish(HFILE input, int bufferlen) {
	spcbuflen = _lread(input, lpspcBuffer, (unsigned int) bufferlen);
#else
static int replenish(FILE *input, int bufferlen) {
	spcbuflen = fread(lpspcBuffer, 1, (unsigned int) bufferlen, input); 
#endif
	if (spcbuflen < 0) {
		wincancel ("Read error in wingetc ");
		finish = -1;
	}
	if (spcbuflen <= 0) return EOF;	/* end of file or read error */
 	spcbufptr = lpspcBuffer;
	return spcbuflen;
}

#ifdef LONGNAMES
static int wingetc(HFILE input) {
#else
static int wingetc(FILE *input) {
#endif
/*	int c; */
/*	if (ungotten >= 0)  {
		c = ungotten; ungotten = -1; 		filepos++;  
		return c;
	}	else */
	if (spcbuflen == 0) {
/*		if (replenish(input, BUFFERLEN) < 0) return EOF; */
		if (replenish(input, EPSIBUFFERLEN) < 0) return EOF;
	}
	spcbuflen--;
	return (unsigned char) *spcbufptr++;
}

#ifdef LONGNAMES
static int wininit(HFILE input) {	/* ignores `input' */
#else
static int wininit(FILE *input) {	/* ignores `input' */
#endif
/*	filepos = 0;	*/		/* beginning of file */
/*	ungotten = -1; */
	if (lpspcBuffer != NULL) winerror("Buffer Error");
	else lpspcBuffer = (char *) LocalAlloc(LMEM_FIXED, EPSIBUFFERLEN);
	spcbuflen = 0;					/* nothing buffered yet */
	if (lpspcBuffer == NULL) {		/* 1996/May/12 */
/*		sprintf(str, "EPSI buffer alloc error (%d bytes)", EPSIBUFFERLEN); 
		winbadimage(str); */
		return -1;				/* indicate failure to caller */
	}
	spcbufptr = lpspcBuffer;		/* redundant ? */
	return 0;
}

static void winendit (void) {
	if (lpspcBuffer == NULL) winerror("Buffer Error");
	LocalFree ((HLOCAL) lpspcBuffer);
	lpspcBuffer = NULL;
}

#endif		/* EPSISPEED */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* Remove path name off fully qualified file name */
/* return pointer to file name - minus path */ /* the string is not copied */

char *removepath (char *pathname) {
	char *s;
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension (char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
/*    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) { 
			strcat(fname, "."); strcat(fname, ext);
	} */	/* rewritten 94/Apr/22 */
	if ((s = strrchr(fname, '.')) != NULL) {
		if ((t = strrchr(fname, '\\')) == NULL) t = strrchr(fname, '/');
		if (t == NULL || s > t) return;	/* already has extension */
	}
	strcat(fname, ".");
	strcat(fname, ext);
}

void forceexten (char *fname, char *ext) { /* change extension if present */
	char *s, *t;
/*    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext); *//* rewritten 94/Apr/22 */
	if ((s = strrchr(fname, '.')) != NULL) {
		if ((t = strrchr(fname, '\\')) == NULL) t = strrchr(fname, '/');
		if (t == NULL || s > t) strcpy(s+1, ext);	/* replace */
	}
	strcat(fname, ".");
	strcat(fname, ext);
}

/* remove file name - keep only path - inserts '\0' to terminate - not used */

void stripname (char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* file handle of \special file being read kept globally here */

/* should we reset this to BAD_ERROR when starting ? */

#ifdef LONGNAMES
HFILE special=HFILE_ERROR;
#else
FILE *special=NULL;
#endif

/* checks whether given filename has more than 8 characters 97/Nov/28 */
/* do similar hack for extension longer than 3 characters ? */

int islongname (char *name) {
	char *s, *t;
	if ((t = strrchr(name, '.')) == NULL) t = name + strlen(name);
	s = t;
	while (s > name && *s != '\\' && *s != '/' && *s != ':') s--;
	if (s > name) s++;
	if ((t - s) > 8) {
		strcpy(s+8, t);			/* truncate it */
/*		winerror(s);	*/		/* debugging */
		return 1;
	}
	else return 0;
}

								/* constructed name of special file */
								/* Use szHyperFile instead ??? */

/* Inserted EPS file is searched for as follows: */
/* If fully qualified, full filename is tried first */
/* If an EPSPath is given, directory part of the file name is stripped off */
/* - then each pathname in the search path is tried in turn */
/* If file is not found, it is next looked for in the dvi file's directory */
/* Finally, an attempt is made to find it in the current directory */

/* shouldn't it look in DVI file directory first ? if unqualified ? */

/* Drops constructed name in global var `filename' */
/* Returns BAD_FILE (NULL or HFILE_ERROR) if failed completely */

/* FINDEPSFILE:  Try to open the file to be included (or overlayed) */

#ifdef LONGNAMES
HFILE findepsfile(char *name, int warnflag, char *ext) {  
#else
FILE *findepsfile(char *name, int warnflag, char *ext) {  
#endif
/* FILE *findepsfile(char *name, int warnflag, char *ext, char *epsfilename) {*/
	char *s;
	char *epsname;				/* points to file name (minus path) */
	int foundfile = 0;
	char epsfilename[MAXFILENAME]; 	/* 95/Apr/26 move in caller or global ? */
#ifndef SUBDIRSEARCH
	char *searchpath;
#endif
/* #ifndef LONGNAMES */
	int nFile, nFileSpecial;	/* debugging tests on number of handles */
/* #endif */

#ifdef DEBUGEPSPATH
	if (bDebug > 1) {
		sprintf(debugstr, "FindEpsFile: %s %s", name, ext);
		OutputDebugString(debugstr);
	}
#endif

/*	if (special != NULL) */	/* 92/Feb/13 */
	if (special != BAD_FILE) {	/* 92/Feb/13 */
		sprintf(debugstr, "Attempt to reopen special (%d)", special);
		winerror(debugstr);
		rewind(special);
		return special;
	}

tryagain:								/* 97/Nov/28 */
	
	strcpy(epsfilename, name);			/*  file name as given */
/*	expand ..\foo.dvi notation and .\foo.dvi notation 1996/Jan/30 */
/*	Note that . here refers to directory of DVI file! 1996/Jan/30 */
	if (*name == '.' || *name == '\\' || *name == '/')
		expanddots(epsfilename);
	extension(epsfilename, ext);		/*  add extension */
	epsname = removepath(epsfilename);	/*	strip path from eps file name */

/*	printf("epsfilename  %s epsname = %s\n", epsfilename, epsname); */

/*  maybe consider only fully qualified if it contains `:' ??? */
/*	if fully qualified name, try that first */
	if (strchr(epsfilename, '\\') != NULL ||
		strchr(epsfilename, '/') != NULL ||
		strchr(epsfilename, ':') != NULL) {		/* fully qualified name */
		strcpy(FileName, epsfilename);
#ifdef DEBUGEPSPATH
		if (bDebug > 1) {
			sprintf(debugstr, "%s (fully qualified)", FileName);
			OutputDebugString(debugstr);
		}	/* 93/Dec/2 debugging */
#endif
/*		if ((special = fopen(filename, "rb")) != NULL) */
		special = fopen(FileName, "rb");
		if (special != BAD_FILE) foundfile = 1; 
	}
	else {	/* not fully qualified try anyway 99/July/3 */
		strcpy(FileName, epsfilename);
		special = fopen(FileName, "rb");
		if (special != BAD_FILE) foundfile = 1;
	}

/*	If not successful, try each path in szEPSPath in turn */
/*	The bug introduced 94/Aug/18 was fixed 4PM 94/Sep/3 ... */
	if (foundfile == 0 &&
/*			strcmp(szEPSPath, "") != 0) */ 		/* was epspath */
			szEPSPath != NULL) {
#ifdef SUBDIRSEARCH
/*		if ((special = findandopen(epsname, szEPSPath, "rb", 0)) != NULL) */
		special = findandopen(epsname, szEPSPath, FileName, "rb", 0);
		if (special != BAD_FILE) foundfile = 1;
#else
		searchpath = szEPSPath;
		for(;;) {
			if ((searchpath = nextpathname(filename, searchpath)) == NULL) {
				foundfile = 0; break;
			}
/*			printf("NEXTPATH %s %d", filename, strlen(filename)); */
			s = filename + strlen(filename) - 1;		/*93/Dec/21 */
			if (*s != '\\' && *s != '/') strcat(filename, "\\"); 
			strcat(filename, epsname);
/*			extension(epsfilename, ext); */
#ifdef DEBUGEPSPATH
			if (bDebug > 1) {
				sprintf(debugstr, "%s (on search path)", filename);
				OutputDebugString(debugstr);
			}	/* 93/Dec/2 debugging */
#endif
			special = fopen(filename, "rb");	/* "r" ? */
/*			if (special != NULL) */
			if (special != BAD_FILE) { 
				foundfile = 1; break;
			}
		}
#endif
	}

/*	if not found on search path */
	if (foundfile == 0 && strchr(epsfilename, ':') == NULL) {
		strcpy(FileName, DefPath);	 /*	try full name in dir of dvi file */
		if (*FileName != '\0') {
			s = FileName + strlen(FileName) - 1;		/*93/Dec/21 */
			if (*s != '\\' && *s != '/') strcat(FileName, "\\");
		}
		strcat(FileName, epsfilename);		/* 92/May/05 */
#ifdef DEBUGEPSPATH
		if (bDebug > 1) {
			sprintf(debugstr, "%s (in DVI file directory)", FileName);
			OutputDebugString(debugstr);
		}		/* 93/Dec/2 debugging */
#endif
		special = fopen(FileName, "rb");
/*		if (special != NULL) foundfile = 1; */
		if (special != BAD_FILE) foundfile = 1; 
	}

	if (foundfile == 0 && strcmp(epsfilename, epsname) != 0) {
		strcpy(FileName, DefPath);/* try file name in directory of dvi file */
		if (*FileName != '\0') {
			s = FileName + strlen(FileName) - 1;		/*93/Dec/21 */
			if (*s != '\\' && *s != '/') strcat(FileName, "\\");
		}
		strcat(FileName, epsname);			/* try file name (no path) */
#ifdef DEBUGEPSPATH
		if (bDebug > 1) {
			sprintf(debugstr, "%s (in DVI file directory)", FileName);
			OutputDebugString(debugstr);
		} 
#endif
		special = fopen(FileName, "rb");
/*		if (special != NULL) foundfile = 1; */
		if (special != BAD_FILE) foundfile = 1;
	}	

	if (foundfile == 0 && strchr(epsfilename, ':') == NULL) {
		strcpy(FileName, epsfilename);	/* try full name (current directory) */
#ifdef DEBUGEPSPATH
		if (bDebug > 1) {
			sprintf(debugstr, "%s (current directory)", FileName);
			OutputDebugString(debugstr);
		}		/* 93/Dec/2 debugging */
#endif
/*		if ((special = fopen(FileName, "rb")) != NULL) foundfile = 1;*/
		special = fopen(FileName, "rb");
		if (special != BAD_FILE) foundfile = 1;
	}

	if (foundfile == 0 && strcmp(epsfilename, epsname) != 0) {
		strcpy(FileName, epsname);	/* try in current directory */
#ifdef DEBUGEPSPATH
		if (bDebug > 1) {
			sprintf(debugstr, "%s (current directory)", FileName);
			OutputDebugString(debugstr);
		}		/* 93/Dec/2 debugging */
#endif
/*		if ((special = fopen(FileName, "rb")) != NULL) 	foundfile = 1;*/
		special = fopen(FileName, "rb");
		if (special != BAD_FILE) foundfile = 1;
	}
	
	if (foundfile == 0) {	/*	try with shortened name - 97/Nov/28 */
		if (islongname(name) != 0) goto tryagain;
	}

	if (foundfile == 0) {
/*		if (warnflag != 0) */
/*		if (warnflag != 0 && bComplainFiles != 0) */
		if (warnflag != 0 && bComplainFiles != 0 &&		/* 93/Mar/30 */
			(bShowPreview != 0 || bPrintFlag != 0 || bCopyFlag != 0)) { 
			sprintf(debugstr, "Can't find `%s'", epsname); 
/*	sprintf(str, "Can't find %s %d %d", epsname, warnflag, bComplainFiles); */
/* file to be inserted */	/*	perror(epsname); */
			winbadimage(debugstr);
/*			errcount(); */
		}
/*		return NULL; */			/* failed ! */
		return BAD_FILE;		/* 95/Dec/10 */
	}

/* #ifndef LONGNAMES */
/*	winerror(FileName); */
/*	debugging hack to check whether files are opened and not closed ! */
/*	if (bDebug && special != NULL) */ /* why is this suddenly not 6 & 7 */
/*	if (special != NULL) */ 			/* why is this suddenly not 6 & 7 */
	if (special != BAD_FILE) { 			/* why is this suddenly not 6 & 7 */
#ifdef LONGNAMES
		nFileSpecial = special;
#else
		nFileSpecial = _fileno(special);
#endif
#ifdef DEBUGEPSPATH
		if (bDebug > 1) {
			sprintf(debugstr, "File %d Open", nFileSpecial);
			OutputDebugString(debugstr);
		}
#endif
		if (nFileSpecial > nFileMax) {
			nFileMax = nFileSpecial;
			nFile = 6;
			if (bPrintFlag != 0) nFile++;
/*			if ((UINT) hTIFFLibrary >= MAXDOSERROR) nFile++; */
			if (hTIFFLibrary >= HINSTANCE_ERROR) nFile++;
/*			if (fileno(special) != nfile) {		 */
/*			if (fileno(special) > nfile + 3) {		 */
#ifdef IGNORED
			if (bDebug > 1 && nFileSpecial > nFile + 3) {		
				sprintf(debugstr, "File Handle %d", nFileSpecial);
				OutputDebugString(debugstr);
			}
#endif
		}
	}	 
/* #endif */												/* 95/Dec/1 */ 
	return special;
}

/* Above deals with names starting with ..\ and .\ only relative */
/* to DVI file directory.  Might also want to have it relative */
/* to each directory on the search path ??? */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

int scanspecial(int, char *, int);

#define TPIC

#ifdef TPIC

/* char *tpiccommand[] = {
	"pa", "fp", "ip", "da", "dt", "sp", "pn", "ar", "ia", 
		"sh", "wh", "bk", "tx", ""}; */

char *tpiccommands = "pa fp ip da dt sp pn ar ia sh wh bk tx cl";

/*	Is this assuming standard dvi_num and dvi_den ??? */
/*	* 0.001  => in */ /*	* 72.27 => points */ /*	* 65536 => scaled points */

long convscal (long z) {		/* convert to scaled point from 0.001 inch */
/*	return (long) ((double) z * 0.001 * 72.27 * 65536); */
/*	return (long) ((double) z * 4736.28672); */
	return z * 4736; 	/* avoid round-off error accumulation & floating */
}

long convscaldouble (double z) {	/* as above, but for double */
	if (z < 0.0) return - (long) (-z * 4736.0 + 0.5);
	return (long) (z * 4736.0 + 0.5);
}

/* #define CONVDEG(x) ((x) * 180 / 3.141592653) */

/* long xold=0, yold=0; */		/* last point placed in path */

int tpicxll, tpicyll, tpicxur, tpicyur;	/* bounding box of path */

/* double tpicgrey=0.5; */		/* selected gray level for fill */

int tpic_r=0, tpic_b=0, tpic_g=0;		/* fill color for TPIC specials */

int fillflag=0;					/* non-zero if shading requested */

#define MAXTPIC 1000			/* maximum path in TPIC special */

/* long FAR *xpath; */			/* FAR pointer to TPIC path table */

/* long FAR *ypath; */			/* FAR pointer to TPIC path table */

int tpicinx=0;					/* index into path */

int penwidth=10;				/* pen width from `pn' command */

#define MINMILLI 1.0			/* smallest measure in milli-inches */

void resetpath(void) {			/* do after stroking/filling path */
	if (lpTPIC != NULL) {
		ReleaseTPIC();			
		lpTPIC = NULL;
	}
/*	xold = 0; yold = 0; */				/* reset previous point */
	tpicxll = TWIPLIM;  tpicyll = TWIPLIM; 
	tpicxur = -TWIPLIM; tpicyur = -TWIPLIM;
	tpicinx = 0;					/* reset path pointer */
	fillflag = 0;					/* reset fill/shading if any */
	tpic_r = tpic_g = tpic_b = 0;	/* default fill color is black */
/*	greyspecified = 1; */			/* claim it is given 96/Mar/31 */
}

/* following used to convert bit pattern to grey shade */

char bits[16]="0112122312232334";	/* "0123456789ABCDEF" */

/* flag = 0 for `stroke', 1 for `fill' */ /* style is pen style */

/* should get here only for PS_SOLID style (or bElCheapo is set) */

void showpathsub (HDC hDC, int flag, int style) { 
	HPEN hTPICPen=NULL, hOldPen;
	HBRUSH hTPICBrush=NULL;
	HBRUSH hOldBrush=NULL;			/* 98/Mar/23 uninit ??? */
	int pen;

	if (tpicinx < 1) return;				/* nothing to show */
	if (lpTPIC == NULL) return;				/* rather serious error ! */

	if (flag == 0) {						/* fp draw polygonal line */
		if (style == PS_SOLID) pen = penwidth;
		else pen = 1;				/* limitation on dotted and dashed lines */
/*		if (pen == 0) pen = 1; */
		hTPICPen = CreatePen(style,	pen, RGB(rule_r, rule_g, rule_b));
	}
	else {									/* ip do filling only */
		hTPICPen = GetStockObject(NULL_PEN); 
/*		hTPICPen = CreatePen(PS_SOLID, 1, RGB(tpic_r, tpic_g, tpic_b)); */
	}
	hOldPen = SelectObject(hDC, hTPICPen);
	if (flag != 0 || fillflag != 0) {		/* fill polygonal region */
		hTPICBrush = CreateSolidBrush(RGB(tpic_r, tpic_g, tpic_b));
		hOldBrush = SelectObject(hDC, hTPICBrush);
	}

	if (flag == 0 && fillflag == 0) Polyline(hDC, lpTPIC, tpicinx);
	else Polygon(hDC, lpTPIC, tpicinx);

	(void) SelectObject(hDC, hOldPen);
	if (flag == 0) (void) DeleteObject (hTPICPen); 
/*	(void) DeleteObject (hTPICPen); */

/*	hOldBrush may be uninitialized */
	if (flag != 0 || fillflag != 0) {
		(void) SelectObject(hDC, hOldBrush);
		(void) DeleteObject (hTPICBrush);
	}
}

/* draw part of poylgonal line --- style PS_SOLID, PS_DASH, or PD_DOT */

void DrawLine(HDC hDC, int xold, int yold, int xnew, int ynew, 
		int style, int inter) {
	int dx, dy, delx, dely, dotx, doty, xo, yo, len, n, k;

/*	sprintf(str, "OLD %d %d NEW %d %d STYLE %d inter %d PEN %d",
		xold, yold, xnew, ynew, style, inter, penwidth);
	winerror(str); */

/*	if style is PS_SOLID, or if inter is zero, or if line length is zero */
	if (style == PS_SOLID || inter == 0 || (xnew == xold && ynew == yold)) {
		MoveToEx(hDC, xold, yold, NULL);
		LineTo(hDC, xnew, ynew);
		return;
	}
/*	assert(inter != 0); */	/* should not have zero interval here */
	if (inter == 0) {
		winerror("interval == 0");	/* debugging */
		return;						/* paranoia 94/Nov/19 */
	}

	dx = xnew - xold; dy = ynew - yold;

	if (dx == 0) len = abs(dy);
	else if (dy == 0) len = abs(dx);
	else len = (int) sqrt ((double) dx * dx + (double) dy * dy);
/*	assert(len != 0); */
	if (len == 0) {
		winerror("length == 0");	/* debugging */
		return;						/* paranoia 94/Nov/19 */
	}

	n = (len + inter/2) / inter; 
	if (n == 0) n = 1;
/*	assert(n != 0); */
	xo = xold; yo = yold;
	if (style == PS_DASH) {
		n = ((n - 1) / 2) * 2 + 1;		/* make an odd number */
		delx = dx / n; dely = dy / n;
/* the following shortens the dashes a bit a la TPIC specs */
		if (tweakdash != 0) {
			dotx = (int) ((double) penwidth * dx / len);
			doty = (int) ((double) penwidth * dy / len);
		}
		else { dotx = 0; doty = 0; }
		MoveToEx(hDC, xold, yold, NULL);
		for (k = 1; k <= n/2; k++) {
			LineTo(hDC, xo+delx-dotx/4, yo+dely-doty/4);
			xo = xo + delx + delx; yo = yo + dely + dely;
			MoveToEx(hDC, xo+dotx/4, yo+doty/4, NULL);
		}
		LineTo(hDC, xnew, ynew);
	}
	else if (style == PS_DOT) {	
		delx = dx / n; dely = dy / n;
/* the following elongates the dots a bit a la TPIC specs */
		if (tweakdot != 0) {
			dotx = (int) ((double) penwidth * dx / len);
			doty = (int) ((double) penwidth * dy / len);
		}
		else { dotx = 0; doty = 0; }
		MoveToEx(hDC, xold, yold, NULL); 
		for (k = 1; k <= n; k++) {
			LineTo(hDC, xo+dotx/4, yo+doty/4);
			xo = xo + delx; yo = yo + dely;
			MoveToEx(hDC, xo-dotx/4, yo-doty/4, NULL);
		}
		LineTo(hDC, xnew, ynew);
	}
	else winerror("Unrecognized style");
}

/* The above has no snapto device grid so at small sizes dots come and go...*/

/* Following used for dashed and dotted polylines */
/* Assume for now these will not be shaded/filled */

void showpathslow (HDC hDC, int flag, int style, int inter) { 
	HPEN hTPICPen=NULL, hOldPen;
	HBRUSH hTPICBrush=NULL;
	HBRUSH hOldBrush=NULL;					/* 98/Mar/23 ??? uninit */
	int xold, yold, xnew, ynew; 
	int k;

	if (tpicinx < 1) return;				/* nothing to show */
	if (lpTPIC == NULL) return;				/* rather serious error ! */

/*	if (penwidth < MinWidth)
		penwidth = (3 * penwidth + MinWidth)/4; */

	if (flag == 0) 						/* draw polygonal line */
		hTPICPen = CreatePen(PS_SOLID, penwidth, RGB(rule_r, rule_g, rule_b));
	else {
		hTPICPen = GetStockObject(NULL_PEN);	/* do filling only */
/*		hTPICPen = CreatePen(PS_SOLID, 1, RGB(tpic_r, tpic_g, tpic_b)); */
	}

	hOldPen = SelectObject(hDC, hTPICPen);
	if (flag != 0 || fillflag != 0) {		/* fill polygonal region */
		hTPICBrush = CreateSolidBrush(RGB(tpic_r, tpic_g, tpic_b));
		hOldBrush = SelectObject(hDC, hTPICBrush);
	}

/*	if (bDebug > 1) {
		sprintf(debugstr, "flag %d fillflag %d style %d tpicinx %d penwidth %d\n",
				flag, fillflag, style, tpicinx, penwidth);
		OutputDebugString(debugstr);
	} */
	xold = lpTPIC[0].x; yold = lpTPIC[0].y;
/*	MoveToEx(hDC, xold, yold, NULL); */
	for (k = 1; k < tpicinx; k++) {
		xnew = lpTPIC[k].x; ynew = lpTPIC[k].y;
/*		LineTo(hDC, xnew, ynew); */
		DrawLine(hDC, xold, yold, xnew, ynew, style, inter);
		xold = xnew; yold = ynew;
	} 

	MoveToEx(hDC, mapx(dvi_h), mapy(dvi_v), NULL);	/* current point */

	(void) SelectObject(hDC, hOldPen);
	if (flag == 0) (void) DeleteObject (hTPICPen); 
/*	(void) DeleteObject (hTPICPen); */

	if (flag != 0 || fillflag != 0) {
		(void) SelectObject(hDC, hOldBrush);
		(void) DeleteObject (hTPICBrush);
	}
}

/*	See whether TPIC path is visible */
/*	We won't worry here about Windows 95 bug in RectVisible */
/*	But maybe we should!  See below */

int pathvisible(HDC hDC) {
	RECT FigureRect;

/*	There ought to be a better way ... Just what is the smallest visible ? */
	if (bFixZeroWidth) {
/*		if ((tpicxur - tpicxll) < (tpicyur - tpicyll))
			tpicxur = tpicxll + (tpicyur - tpicyll);
		if ((tpicyur - tpicyll) < (tpicxur - tpicxll))
			tpicyur = tpicyll + (tpicxur - tpicxll); */
		if ((tpicxur - tpicxll) < MinWidth) tpicxur = tpicxll + MinWidth;
		if ((tpicyur - tpicyll) < MinWidth) tpicyur = tpicyll + MinWidth;
	}						/* fix 95/Sep/3 */

	FigureRect.left = tpicxll; FigureRect.right = tpicxur;
	FigureRect.bottom = tpicyll; FigureRect.top = tpicyur;

/*	Avoid RectVisible() in MetaFile - use InterSectRect instead */
	if ((bCopyFlag == 0 && RectVisible(hDC, &FigureRect) != 0) ||
		(bCopyFlag != 0 && InterSectRect(&CopyClipRect, &FigureRect) !=	0)) {
		return 1;			/* it is visible */
	}
	else return 0;			/* it is not visible */
}

void showpath (HDC hDC, int flag) {		/* flag = 0 for fp, 1 for ip */
	if (pathvisible(hDC) == 0) return;
	showpathsub(hDC, flag, PS_SOLID);
}

/* PS_SOLID, PS_DASH, PS_DOT, PS_DASHDOT, PS_DASHDOTDOT */

/* flag = 0 for da (dash), 1 for dt (dot) */

void dashdot (HDC hDC, int inter, int flag) { 
	int style;

	if (pathvisible(hDC) == 0) return;

	if (flag == 0) style = PS_DASH;
	else style = PS_DOT;

/*	El Cheapo implementation first ... */
	
	if (bElCheapo != 0 && bPrintFlag == 0 && bCopyFlag == 0) 
		showpathsub(hDC, 0, style);
	else showpathslow(hDC, 0, style, inter);
}

/* ************************* SPLINE STUFF ****************************** */

int flatten = 20;	/* flattenpath parameter in TWIPS */
					/* is device and magnifciation dependent */

int splinestyle;	/* style of spline */

int markingflag;	/* non-zero while on dash, zero in between */
int arclength;		/* arc length accumulated from previous flip */
int dashlength;		/* length of dash or dot-interval */

int xdash, ydash;	/* last MoveTo or LineTo in dashing spline */

void DashMoveTo(HDC hDC, int x, int y) {
	MoveToEx (hDC, x, y, NULL);
	markingflag = 1; 
	arclength = 0; 
	xdash = x; ydash = y;
}

void DashLineTo(HDC hDC, int x, int y) {
	int len, delx, dely, dx, dy, dlen;

	if (x == xdash && y == ydash) return;	/* zero length */

	delx = x - xdash; dely = y - ydash;
/*	if (delx == 0) len = abs(dely); */
/*	else if (dely == 0) len = abs(delx);	else */
	len = (int) sqrt ((double) delx * delx + (double) dely * dely);
/*	else if (abs (delx) > abs(dely)) len = abs(delx); */
/*	else len = abs(dely); */

/*	assert (len != 0); */
	if (len == 0) {
		winerror("length == 0");	/* debugging */
		return;						/* paranoia 94/Nov/14 */
	}

	if (arclength + len < dashlength) {		/* no flip in line */
		arclength += len;
		xdash = x; ydash = y;
		if (markingflag != 0) LineTo (hDC, x, y);
		else
			MoveToEx (hDC, x, y, NULL);			/* needed ? */
		return;
	}

/*	need at least one  flip */
	dlen = dashlength - arclength;					/* first partial */
	dx = (int) ((long) delx * dlen / len);	
	dy = (int) ((long) dely * dlen / len);
	xdash += dx; ydash += dy;
	if (markingflag != 0) LineTo (hDC, xdash, ydash);
	else
		MoveToEx (hDC, xdash, ydash, NULL);
	markingflag = 1 - markingflag;					/* flip state* */
	arclength = 0;									/* reset phase */

	dx = (int) ((long) delx * dashlength / len);	/* ready full dash */
	dy = (int) ((long) dely * dashlength / len);

	len -= dlen;									/* reduce length */

	while (len > dashlength) {						/* full dashes of need */
		xdash += dx; ydash += dy;
		if (markingflag != 0) LineTo (hDC, xdash, ydash);
		else
			MoveToEx (hDC, xdash, ydash, NULL);
		markingflag = 1 - markingflag;
		len -= dashlength;
	}
/*	arclength = 0; */

	dx = x - xdash; dy = y - ydash;
/*	if (dx == 0) dlen = abs(dy); */
/*	else if (dy == 0) dlen = abs(dx);	else */
	dlen = (int) sqrt ((double) dx * dx + (double) dy * dy);
/*	else if (abs (dx) > abs(dy)) dlen = abs(dx); */
/*	else dlen = abs(dy); */

	if (markingflag != 0) LineTo(hDC, x, y);
	else
		MoveToEx(hDC, x, y, NULL);		/* needed ? */
	xdash = x; ydash = y;
	arclength += dlen;		
/*	len -= dlen; */
}

void DotMoveTo(HDC hDC, int x, int y) {
	MoveToEx (hDC, x, y, NULL);
	LineTo (hDC, x, y); 		/* try and make a spot */
	arclength = 0; 
	xdash = x; ydash = y;
}

void DotLineTo(HDC hDC, int x, int y) {
	int len, delx, dely, dx, dy, dlen;

/* take care of initial spot */
/*	if (arclength == 0) {	
		MoveTo (hDC, x, y);
		LineTo (hDC, x, y); 
	} */

	if (x == xdash && y == ydash) return;	/* zero length */

	delx = x - xdash; dely = y - ydash;
/*	if (delx == 0) len = abs(dely); */
/*	else if (dely == 0) len = abs(delx);	else */
	len = (int) sqrt ((double) delx * delx + (double) dely * dely);
/*	else if (abs (delx) > abs(dely)) len = abs(delx); */
/*	else len = abs(dely); */

	if (len == 0) {
		winerror("length == 0");	/* debugging */
		return;						/* paranoia 94/Nov/14 */
	}

	if (arclength + len < dashlength) {		/* no dot on this line */
		arclength += len;
		xdash = x; ydash = y;
		MoveToEx (hDC, x, y, NULL);			/* needed ? */
/*		winerror("no dot"); */
		return;
	}

/*	need at least one dot */
	dlen = dashlength - arclength;					/* first partial */
	dx = (int) ((long) delx * dlen / len);	
	dy = (int) ((long) dely * dlen / len);
	xdash += dx; ydash += dy;
	MoveToEx (hDC, xdash, ydash, NULL);
	LineTo (hDC, xdash, ydash);						/* try and make a dot */
/*	sprintf(str, "%d %d", xdash, ydash); 
	winerror(str); */								/* debugging */
	arclength = 0;									/* reset phase */

	dx = (int) ((long) delx * dashlength / len);	/* ready full dash */
	dy = (int) ((long) dely * dashlength / len);

	len -= dlen;									/* reduce length */

	while (len > dashlength) {						/* full dots if need */
		xdash += dx; ydash += dy;
		MoveToEx (hDC, xdash, ydash, NULL);
		LineTo (hDC, xdash, ydash);				/* try and make a dot */
/*		sprintf(str, "%d %d", xdash, ydash); 
		winerror(str); */						/* debugging */
		len -= dashlength;
	}
/*	arclength = 0; */

	dx = x - xdash; dy = y - ydash;
/*	if (dx == 0) dlen = abs(dy); */
/*	else if (dy == 0) dlen = abs(dx);	else */
	dlen = (int) sqrt ((double) dx * dx + (double) dy * dy);
/*	else if (abs (dx) > abs(dy)) dlen = abs(dx); */
/*	else dlen = abs(dy); */

	MoveToEx(hDC, x, y, NULL);		/* needed ? */
	xdash = x; ydash = y;
	arclength += dlen;		
/*	len -= dlen; */
}

/* draw quadratic spline from (xa, ya) to (xc, yc), using (xb, yb) control */

void Quadratic(HDC hDC, int xa, int ya, int xb, int yb, int xc, int yc){
	int xn, yn, xm, ym, len;
	long cross;

	if ((xa == xb && ya == yb) ||
		(xb == xc && yb == yc)) {		/* only ONE line, not two */
		if (splinestyle == PS_SOLID) LineTo(hDC, xc, yc);
		else if (splinestyle == PS_DASH) DashLineTo(hDC, xc, yc);
		else DotLineTo(hDC, xc, yc);
		return;
	}

	cross = (long) (xb - xa) * (yc - ya) - (long) (xc - xa) * (yb - ya);
	if (cross < 0) cross = - cross;
	if (abs(xc - xa) > abs(yc - ya)) len = abs(xc - xa);
	else len = abs(yc - ya);
	if (cross < (long) len * flatten) {	/* terminating case */
/*		MoveToEx(hDC, xa, ya, NULL); */
		if (splinestyle == PS_SOLID) LineTo(hDC, xb, yb);
		else if (splinestyle == PS_DASH) DashLineTo(hDC, xb, yb);
		else DotLineTo(hDC, xb, yb);
		if (splinestyle == PS_SOLID) LineTo(hDC, xc, yc);
		else if (splinestyle == PS_DASH) DashLineTo(hDC, xc, yc);
		else DotLineTo(hDC, xc, yc);
		return;
	}
	else {								/* split and recurse */
/*		xm = (xa/2 + xc/2)/2 + xb/2; */
		xm = (int) (((long) xa + xb + xb + xc)/4);
/*		ym = (ya/2 + yc/2)/2 + yb/2; */
		ym = (int) (((long) ya + yb + yb + yc)/4);
/*		xn = xa/2 + xb/2; */
		xn = (int) (((long) xa + xb)/2);
/*		yn = ya/2 + yb/2; */
		yn = (int) (((long) ya + yb)/2);
		Quadratic (hDC, xa, ya, xn, yn, xm, ym);
/*		xn = xc/2 + xb/2; */
		xn = (int) (((long) xc + xb)/2);
/*		yn = yc/2 + yb/2; */
		yn = (int) (((long) yc + yb)/2);
		Quadratic (hDC, xm, ym, xn, yn, xc, yc);
	}
}

/* following used for showing splines */ 

/* so far only set up for SOLID splines */

/* interval = 0 => SOLID, interval > 0 => DASHED, interval < 0 => DOTTED */

/* flag = 0 for `stroke', 1 for `fill' */ /* style is pen style */

void showspline (HDC hDC, int flag, int style, int inter) { 
	HPEN hTPICPen=NULL, hOldPen;
	HBRUSH hTPICBrush=NULL;
	HBRUSH hOldBrush=NULL;	/* 98/Mar/23 ??? uninit */
	int xanc, yanc, xold, yold, xnew, ynew, xmid, ymid, xmud, ymud;
	int xstar, ystar;
	int closedflag=0;		/* non-zero if spline is closed */
	int k;

	if (pathvisible(hDC) == 0) return;

	if (tpicinx < 2) return;				/* nothing to show */
	if (lpTPIC == NULL) return;				/* rather serious error ! */

	markingflag = 1;			/* start with dash */ /* redundant */
	arclength = 0;				/* draw full dash first */ /* redundant */
	dashlength = inter;			/* remember dash interval */
	splinestyle = style;		/* remember style of spline */

/*	if (splinestyle != PS_SOLID) winerror("NOT SOLID"); */

	if (flag == 0)							/* draw spline */
		hTPICPen = CreatePen(PS_SOLID, penwidth, RGB(rule_r, rule_g, rule_b));
	else {
		hTPICPen = GetStockObject(NULL_PEN); /* do filling only */
/*		hTPICPen = CreatePen(PS_SOLID, 1, RGB(tpic_r, tpic_g, tpic_b)); */
	}

	hOldPen = SelectObject(hDC, hTPICPen);
	if (flag != 0 || fillflag != 0) {		/* fill polygonal region */
		hTPICBrush = CreateSolidBrush(RGB(tpic_r, tpic_g, tpic_b));
		hOldBrush = SelectObject(hDC, hTPICBrush);
	}

/*	Make dependent on magnification ? */
	flatten = 20;								/* 1 pt = 1/72 in */
/*	flatten = flatten - wantedzoom; */ /* NO, coords ARE screen coordinates */

/*	Use coarse resolution on screen - finer when printing */
/*	Should determine pixel size here ? */
	if (bPrintFlag != 0 || bCopyFlag != 0) flatten = flatten / 8;
	if (flatten <= 0) flatten = 1;

	xanc = lpTPIC[0].x; yanc = lpTPIC[0].y;
	xold = lpTPIC[1].x; yold = lpTPIC[1].y;
	xmid = xanc/2 + xold/2; ymid = yanc/2 + yold/2;

	xstar = xmid; ystar = ymid;
	if (xanc == lpTPIC[tpicinx-1].x &&
		yanc == lpTPIC[tpicinx-1].y) closedflag = 1;
	else closedflag = 0;
	
	if (bAllowClosed == 0) closedflag = 0;		/* only if permitted */

/*	markingflag = 1; */
/*	arclength = 0; */
	if (closedflag == 0) {
		if (splinestyle == PS_SOLID)
			MoveToEx(hDC, xanc, yanc, NULL);
		else if (splinestyle == PS_DASH)
			DashMoveTo(hDC, xanc, yanc);
		else DotMoveTo(hDC, xanc, yanc);
		if (splinestyle == PS_SOLID) LineTo(hDC, xmid, ymid);
		else if (splinestyle == PS_DASH) DashLineTo(hDC, xmid, ymid);
		else DotLineTo(hDC, xmid, ymid);
	}
	else {
		if (splinestyle == PS_SOLID)
			MoveToEx(hDC, xmid, ymid, NULL);
		else if (splinestyle == PS_DASH)
			DashMoveTo(hDC, xmid, ymid);
		else DotMoveTo(hDC, xmid, ymid);
	}

	for (k = 2; k < tpicinx; k++) {
		xnew = lpTPIC[k].x; ynew = lpTPIC[k].y;
		xmud = xold/2 + xnew/2; ymud = yold/2 + ynew/2;
		Quadratic(hDC, xmid, ymid, xold, yold, xmud, ymud);
		xanc = xold; yanc = yold;
		xmid = xmud; ymid = ymud;
		xold = xnew; yold = ynew;
	} 

	xnew = lpTPIC[tpicinx-1].x; ynew = lpTPIC[tpicinx-1].y;
	if (closedflag == 0) {
		if (splinestyle == PS_SOLID) LineTo(hDC, xnew, ynew);
		else if (splinestyle == PS_DASH) DashLineTo(hDC, xnew, ynew);
		else DotLineTo(hDC, xnew, ynew);
	}
	else Quadratic(hDC, xmid, ymid, xold, yold, xstar, ystar);

	MoveToEx(hDC, mapx(dvi_h), mapy(dvi_v), NULL);	/* current point */

	(void) SelectObject(hDC, hOldPen);
	if (flag == 0) (void) DeleteObject (hTPICPen); 
/*	(void) DeleteObject (hTPICPen); */

	if (flag != 0 || fillflag != 0) {
		(void) SelectObject(hDC, hOldBrush);
		(void) DeleteObject (hTPICBrush);
	}
}

/* interval = 0 => SOLID, interval > 0 => DASHED, interval < 0 => DOTTED */

void spline (HDC hDC, int inter) { 
	int style;

	if (inter == 0) style = PS_SOLID;
	else if (inter > 0) style = PS_DASH;
	else style = PS_DOT;
	
	if (inter < 0) inter = - inter;				/* absolute value */

/*  el cheapo implementation first ... */

	if (bElCheapo != 0 && bPrintFlag == 0 && bCopyFlag == 0) 
		showpathsub (hDC, 0, style);
/*	else if (style == PS_SOLID) showspline (hDC, 0, style, inter); */
	else showspline (hDC, 0, style, inter);
/*	else showpathsub (hDC, 0, style);	*/		/* until we fix it up */
}

void ellipse (HDC hDC, long x, long y, long xr, long yr, 
			double sa, double ea, int flag) { /* flag = 0 for ar, 1 for ia */
	int xll, yll, xur, yur, xs, ys, xe, ye;
	double da;
	RECT FigureRect;
	HPEN hTPICPen=NULL, hOldPen;
	HBRUSH hTPICBrush=NULL;
	HBRUSH hOldBrush=NULL;	/* 98/Mar/23 ??? uninit */
	
	if (flag == 0)								/* draw elliptical arc */
		hTPICPen = CreatePen(PS_SOLID, penwidth, RGB(rule_r, rule_g, rule_b));
	else {
		hTPICPen = GetStockObject(NULL_PEN);	 /* do filling only */
/*		hTPICPen = CreatePen(PS_SOLID, 0, RGB(tpic_r, tpic_g, tpic_b)); */
	}

	hOldPen = SelectObject(hDC, hTPICPen);

	if (flag != 0 || fillflag != 0) {		/* fill elliptical arc */
		hTPICBrush = CreateSolidBrush(RGB(tpic_r, tpic_g, tpic_b));
		hOldBrush = SelectObject(hDC, hTPICBrush);
	}

/*	Compute bounding box of ellipse */
	xll = mapx(dvi_h + x-xr);	yll = mapy(dvi_v + y-yr);
	xur = mapx(dvi_h + x+xr);	yur = mapy(dvi_v + y+yr);
/*	sprintf(str, "LL %d %d UR %d %d PEN %d", xll, yll, xur, yur, penwidth);
	winerror(str); */
/*	make sure tiny ellipse does not disappear ... */
/*	ellipse can't be smaller than this -- or maybe draw as point ??? */
/*	if (xur < xll + 2 && xur > xll - 2) xur = xll + 2; */
/*	if (yur > yll - 2 && yur < yll + 2) yur = yll - 2; */
	FigureRect.left = xll; FigureRect.right = xur;
	FigureRect.bottom = yll; FigureRect.top = yur;
/*  First check whether ellipse is visible */
/*	We won't worry here about Windows 95 bug */ /* why ??? */
/*	Avoid RectVisible() in MetaFile - use InterSectRect instead */
	if ((bCopyFlag == 0 && RectVisible(hDC, &FigureRect) != 0) ||
		(bCopyFlag != 0 && InterSectRect(&CopyClipRect, &FigureRect) !=	0)) {
/*	interchange start and end, because GUI counterclockwise, TPIC clockwise */
		xs = mapx(dvi_h + x + (long) (xr * cos(ea)));
		ys = mapy(dvi_v + y + (long) (yr * sin(ea)));
		xe = mapx(dvi_h + x + (long) (xr * cos(sa)));
		ye = mapy(dvi_v + y + (long) (yr * sin(sa)));
		da = ea - sa;
/*		if (fabs(da - 6.283185307) < 0.001) { xe = xs; ye = ys+1; } */
		if (flag != 0 || fillflag != 0) {
			if (fabs(da - 6.283185307) < 0.002) 
				Ellipse(hDC, xll, yll, xur, yur); 
			else Pie(hDC, xll, yll, xur, yur, xs, ys, xe, ye);
/*			else Chord(hDC, xll, yll, xur, yur, xs, ys, xe, ye); */
		}
		else Arc(hDC, xll, yll, xur, yur, xs, ys, xe, ye);
	}	/* end of clipping rectangle check */

	(void) SelectObject(hDC, hOldPen);
	if (flag == 0) (void) DeleteObject (hTPICPen); 
/*	(void) DeleteObject (hTPICPen); */

	if (flag != 0 || fillflag != 0) {
		(void) SelectObject(hDC, hOldBrush);
		(void) DeleteObject (hTPICBrush);
	}
}

int readtpic (HDC hDC, int input) {
	long x, y;
	double z=0.0;
	long xr=0, yr=0;
	double sa=0.0, ea=0.0;
	double r, g, b, grey=0.5;
	int bitsone, bitstot;
	int c, k;
	int xnew, ynew;
	char *s;
	int flag;
/*	long interval; */
	int interval;
/*	char temp[3]; */
/*	char bits[16]="0112122312232334"; */

	if (strlen(line) != 2) return 0;					/* quick exit */
	if (strstr(tpiccommands, line) == NULL) return 0;	/* quick exit */

	if (hTPIC == NULL) {		/* if path array storage does not yet exist */
		AllocTPIC(MAXTPIC);
		lpTPIC = NULL;
	}

	if (strcmp(line, "pn") == 0) {			/* pn n - set pen width */
/*		(void) scanspecial(input, line, MAXLINE); */
		(void) scanspecial(input, line, sizeof(line));
		(void) sscanf(line, "%lg", &z);		/* complain if < 1 ? */
		if (z > - MINMILLI && z < MINMILLI)	/* should be in miniinches */
			z = z * 1000.0;					/* argument was in inches ? */
/*		warn if pen-width is zero ? */
/*		tpicpenwidth = convscaldouble(z); */	/* remember pen width */
		penwidth = mapd(convscaldouble(z)); 	/* remember pen width */
/*		A hack to try and prevent thin dotted lines from dropping out */
		if (penwidth > 0 && penwidth < MinWidth)
			penwidth = (3 * penwidth + MinWidth) >> 2;	/* 95/Sep/3 */
		if (penwidth <= 0) penwidth = 1; 
/*		fprintf(output, "%ld pn ", convscaldouble(z)); */
	}
	else if (strcmp(line, "pa") == 0) {		/* pa x y - add point to path */
/*		(void) scanspecial(input, line, MAXLINE); */
		(void) scanspecial(input, line, sizeof(line));
/*		x = xold; y = yold; */						/* just in case */ 
		(void) sscanf (line, "%ld %ld", &x, &y); /* complain if < 2 ? */
		if (lpTPIC == NULL) {
			GrabTPIC();	
/*			xpath = lpTPIC; ypath = lpTPIC + MAXTPIC; */
			tpicinx = 0;
		}
/*		lpTPIC[tpicinx] = convscal(x - xold); */
/*		xpath[tpicinx] = convscal(x - xold); */
/*		xpath[tpicinx] = convscal(x); */
		xnew = mapx(dvi_h + convscal(x));
		lpTPIC[tpicinx].x = xnew;
/*		lpTPIC[MAXTPIC + tpicinx] = convscal(y - yold); */
/*		ypath[tpicinx] = convscal(y - yold); */
/*		ypath[tpicinx] = convscal(y); */
		ynew = mapy(dvi_v + convscal(y));
		lpTPIC[tpicinx].y = ynew;
		tpicinx++;
		if (tpicinx >= MAXTPIC) {	/* overflowed buffer */
			tpicinx--;
		}
/*	fprintf(output, "%ld %ld pa", convscal(x - xold), convscal(y - yold));*/
/*		xold = x; yold = y; */
/*		buil dup bounding box for path */
		if (xnew > tpicxur) tpicxur = xnew;
		if (xnew < tpicxll) tpicxll = xnew;
		if (ynew > tpicyur) tpicyur = ynew;
		if (ynew < tpicyll) tpicyll = ynew;
	}
	else if (strcmp(line, "da") == 0 ||
		     strcmp(line, "dt") == 0) {	   /* da/dt l - stroke dashed/dotted */
		if (strcmp(line, "dt") == 0) flag = 1;	else flag = 0;
/*		(void) scanspecial(input, line, MAXLINE); */	/* complain if < 1 ? */
		(void) scanspecial(input, line, sizeof(line));	/* complain if < 1 ? */
		z = 0;
		(void) sscanf (line, "%lg", &z);
		if (z > - MINMILLI && z < MINMILLI)		/* should be in inches */
			z = z * 1000.0;						/* argument was in inches */
/*		warn if dot/dash length is zero ? */
/*		interval =  convscaldouble(z); */
		interval =  mapd(convscaldouble(z));
/*		if (bDebug > 1) {
			sprintf(debugstr, "z %lg intderval %d scaled %ld\n",
					z, interval, convscaldouble(z));
			OutputDebugString(debugstr);
		} */ 										/* 95/Sep/3 */
		dashdot (hDC, interval, flag);
		resetpath();
	}				/* end of da/dt <interval> case */
	else if (strcmp(line, "sp") == 0) { /* sp [l] - stroke quadratic spline */
/*		(void) scanspecial(input, line, MAXLINE); */
		(void) scanspecial(input, line, sizeof(line));
		interval = 0;							/* solid spline case */
		if (sscanf (line, "%lg", &z) > 0) {
			if (z > - MINMILLI && z < MINMILLI) 
				z = z * 1000.0;					/* argument was in inches */
/*			warn if dot/dash length is zero ? */
/*			interval =  convscaldouble(z); */
			interval =  mapd(convscaldouble(z));
/*			sprintf(str, "z %lg convscal %ld interval %d", 
				z, convscaldouble(z), interval); 
			winerror (str); */					/* debugging */
		}
		spline (hDC, interval);
		resetpath();
	}			/* end of sp/sp <interval> case */
	else if (strcmp(line, "ar") == 0 ||
			strcmp(line, "ia") == 0) {			/* ar/ia x y xr yr sa ea */
		if (strcmp(line, "ia") == 0) flag = 1;	else flag = 0;
/* following added 94/June/20 */
		if (fillflag == 0) flag = 0;	/* if sh not used then ia => ar */
/*		(void) scanspecial(input, line, MAXLINE); */
		(void) scanspecial(input, line, sizeof(line));
		(void) sscanf(line, "%ld %ld %ld %ld %lg %lg",
			&x, &y, &xr, &yr, &sa, &ea);
/*	ah hoc attempt to compensate for apparent GUI radius error */
		if (xr > 0) xr += 3;
		if (yr > 0) yr += 3;	
/*	allow for dotted/dashed ellipses ? */
		ellipse(hDC, convscal(x), convscal(y), convscal(xr), convscal(yr), 
				sa, ea, flag);
/*		fillflag = 0;	*/				/* reset fill/shading if any */
/*		tpic_r = tpic_g = tpic_b = 0; */	/* default fill color is black */
		resetpath();
	}		/* end of ar/ia case */
	else if (strcmp(line, "sh") == 0 ||
				strcmp(line, "cl") == 0) { /* sh [s] */
/*		(void) scanspecial(input, line, MAXLINE); */
		(void) scanspecial(input, line, sizeof(line));
		grey = 0.5;							/* default grey value */
/*		greyspecified = 1; */				/* whether grey/color given */
		if (sscanf(line, "%lg %lg %lg", &r, &g, &b) < 3) {
			(void) sscanf (line, "%lg", &grey);
/*			if "sh" or "cl" given without args, use 0.5 ... */
/*			if (sscanf (line, "%lg", &grey) < 1)
				greyspecified = 0;			*/		/* 96/Mar/31 */
			r = g = b = grey;
		}
		if (r > 1.0 || g > 1.0 || b > 1.0) {
			tpic_r = (int) r;
			tpic_g = (int) g; 
			tpic_b = (int) b;
		}
		else {
			tpic_r = (int) ((1.0 - r) * 255.0 + 0.5);
			tpic_g = (int) ((1.0 - g) * 255.0 + 0.5);
			tpic_b = (int) ((1.0 - b) * 255.0 + 0.5);
		}
		fillflag++;
	}					/* end of sh/cl case */
	else if (strcmp(line, "fp") == 0 ||
			 strcmp(line, "ip") == 0) {						/* fp/ip */
		if (strcmp(line, "ip") == 0) flag = 1; else flag = 0;
/* following added 94/June/20 */
		if (fillflag == 0) flag = 0;	/* if sh not used then ip => fp */
		showpath(hDC, flag);
		resetpath();
	}					/* end of fp/ip case */
	else if (strcmp(line, "wh") == 0 ||
			 strcmp(line, "bk") == 0) {						/* wh/bk */
		if (strcmp(line, "bk") == 0) {
			tpic_r = tpic_g = tpic_b = 0;					/* black */
		}
		else {
			tpic_r = tpic_g = tpic_b = 255;					/* white */
		}
/*		greyspecified = 1; */								/* 96/Mar/31 */
		fillflag++;		
	}					/* end of bk/wh case */
	else if (strcmp(line, "tx") == 0) { /* tx */
/*		(void) scanspecial(input, line, MAXLINE); */
		(void) scanspecial(input, line, sizeof(line));
/* need to count bits in hexadecimal mask pattern here */
		bitsone=0; bitstot=0;
		s = line;
		while ((c = *s++) != 0) {
			if (c >= '0' && c <= '9') k = c - '0';
			else if (c >= 'A' && c <= 'Z') k = c + 10 - 'A';
			else if (c >= 'a' && c <= 'z') k = c + 10 - 'a';
			else k = 0;
			bitsone += bits[k] - '0';
			bitstot += 4;
		}
		if (bitstot == 0) bitstot = 1;			/* paranoia */
		grey = (double) bitsone / bitstot; 
		tpic_r = tpic_g = tpic_b = 	(int) ((1.0 - grey) * 255.0 + 0.5);
/*		greyspecified = 1; */					/* 96/Mar/31 */
		fillflag++;
	}							/* end of tx case */
/*	else return 0; */			/* impossible ! not a TPIC special */

/*	if (lpTPIC != NULL) {
		ReleaseTPIC();			
		lpTPIC = NULL;
	} */						/* somewhat inefficient */

	flushspecial(input);	/* clean out the rest */
	return -1;
}

#else

int readtpic (HDC hDC, int input) {	/* fail if not implemented */
	return 0;
}
#endif

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* stuff moved from winanal  - move back for efficiency ? */

/* read the whole special into a buffer */

// void readspecial(int input, char *buff, int nmax)
void readspecial (int input, char *buff, unsigned int nmax) {
	int c;
//	if (nspecial <= 0) return;
	if (nspecial == 0) return;
	if (nspecial >= nmax) {		/* protect ourselves from insanity 99/Feb/21 */
		if (bDebug > 1) {
			sprintf(debugstr, "special too long %d >= %d", nspecial, nmax);
			OutputDebugString(debugstr);
		}
		flushspecial(input);
		*buff = '\0';
		return;
	}
	c = an_wingetc(input); nspecial--;
	while (nspecial > 0 && c != EOF) {
		*buff++ = (char) c;
		c = an_wingetc(input);
		nspecial--;
	}
	if (c != EOF) *buff++ = (char) c;
	*buff = '\0';
}

void flushspecial (int input) {
	int c;
	if (nspecial <= 0) return;
	c = an_wingetc(input); nspecial--;
	while (nspecial > 0 && c != EOF) {
		c = an_wingetc(input); nspecial--;
	}
}

/* error message output of beginning of long item */

void showbeginning(char *s) {
	sprintf(debugstr, "Token too long: ");
	strncpy(debugstr + 16, s, 32);
	strcat(debugstr, " ...");
/*	wincancel(debugstr); */
	winbadspecial(debugstr);					/* 96/Feb/4 */
}

/* read next alphanumeric token from special - stop when not alphanumeric */
/* returns terminator */

/* here need to use ANSI character operations ? what about file names ? */
/* need to use AnsiToOem on filenames ? */ /* or use _lopen ? */

int getalphatoken(int input, char *token, int nmax) {
	int c, k = 0;
	char *s = token;

	*token = '\0';						/* in case pop out early 96/Aug/29 */
	if (nspecial <= 0) return 0;
	if (nmax <= 0) {					/* 95/Aug/30 */
		flushspecial(input);
		return 0;						/* error overflow */
	}
	c = an_wingetc(input); --nspecial;
	if (c == 0) {
		flushspecial(input);
		return 0;						/* first byte is null 96/Aug/29 */
	}

	while(c <= ' ' && nspecial > 0) {	/* over initial white space */
		c = an_wingetc(input); --nspecial;
	}
	if (nspecial <= 0) return 0;
/*	while(((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		   (c >= '0' && c <= '9'))) */
	while(IsCharAlphaNumeric((char) c)) {
		   *s++ = (char) c;
		   if(k++ >= nmax) { /* too long */
			   showbeginning(token);
/*			   errcount(); */
			   c = an_wingetc(input); --nspecial;
/*			   while(((c >= 'a' && c <= 'z')  || (c >= 'A' && c <= 'Z')  ||
				   (c >= '0' && c <= '9')) && nspecial > 0) {	*/
			   while (IsCharAlphaNumeric((char) c)) {
				   c = an_wingetc(input); --nspecial;
			   }
			   break;
		   }
		   if (nspecial <= 0) {
			   c = ' '; break;	/* pretend there is a space at the end */
		   }
		   c = an_wingetc(input); --nspecial;
	}
	*s++ = '\0';
/*	printf("token: %s (length %d)\n", line, s - line); */
	return c;		/* return terminator */
}

/*  read token from special *//* ignore leading white space */
/* - either up to white space - or - double quote delimited */ /* NEW */
/* returns terminating character */

int gettoken (int input, char *buff, int nmax) {
	int c, k=0, marker=' ';		// end of token marker
	char *s=buff;

	*s = '\0';						// in case pop out right away
	if (nspecial <= 0) return 0;		/* nothing more in \special */
	if (nmax <= 0) {					/* 95/Aug/30 */
		flushspecial(input);
		return 0;						/* error overflow */
	}
	c = an_wingetc(input);
	--nspecial;
	while(c <= ' ' && nspecial > 0) {	/* ignore initial white space */
		c = an_wingetc(input);
		--nspecial;
	}
	if (c <= ' ') return 0;				/* nothing more 1993/Sep/7 */
//	if (nspecial <= 0) return 0;		/* nothing more */

	if (c == '\"') {					/* deal with quoted string */
		marker = '\"';
		if (nspecial <= 0) return 0; 	/* nothing more */ /* 93/Sep/7 */
		c = an_wingetc(input);
		--nspecial;
	}
//	if (nspecial <= 0) return 0;		/* nothing more */

/*	while (c > ' ' && nspecial >= 0) {	 */
	while (c != marker && c >= ' ') {
/*		*s++ = (char) c; */
		if(k++ >= nmax) {
/*			fprintf(stderr, " Token in special too long (> %d)\n", nmax); */
			showbeginning(buff);
/*			errcount(); */
//			c = an_wingetc(input);
//			--nspecial;
//			while(c > ' ' && nspecial > 0) {	
//				c = an_wingetc(input);
//				--nspecial;
//			}
			flushspecial(input);
			break;
		}
		*s++ = (char) c;		/* moved down 95/Aug/30 */ 
		if (nspecial <= 0) { 	/* hit end of special ? */
			if (marker != ' ') {	/* complain if marker is not ` ' ...*/
				sprintf(debugstr, " Missing `%c' in special\n", marker);
				winerror(debugstr);
			}					/* above added 1993/Sep/7 */
/*			c = ' ';	*/		/* pretend finished with space */
			c = marker;			/* pretend hit desired marker */
			break;
		}
		c = an_wingetc(input);
		--nspecial;
	}
	*s = '\0';
	return c;		/* return terminator */
}

void analspecial(HDC, int, int);

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

int readimagefile (HDC, char *, int, int, int, int, int, int);

void DrawFigureBox(HDC hDC, int xll, int yll, int xur, int yur) {
	HPEN hOldPen=NULL;
	POINT Box[5];							/* 93/Jan/22 */

/*	if (bCopyFlag != 0) return; */				/* 95/April/15 OK */
	hOldPen = SelectObject(hDC, hFigurePen); /* ??? */

	Box[0].x = xll; 	Box[0].y = yll;
	Box[1].x = xur; 	Box[1].y = yll;	
	Box[2].x = xur; 	Box[2].y = yur;	
	Box[3].x = xll; 	Box[3].y = yur;	
	Box[4].x = xll; 	Box[4].y = yll;
	Polyline(hDC, Box, 5);				/* 93/Jan/22 */
	if ((UINT) hOldPen > 1) /* avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hDC, hOldPen);		
} 

int ShowNameFile(HDC hDC, int xll, int yll, int xur, int yur, 
			char *filename) {
	long atsize;
	int nHeight;
	HFONT hFont=NULL, hOldFont=NULL;
	SIZE TextSize;

	int xmd, ymd, dx, dy;
	COLORREF SavedTextColor, SavedBackColor;
	int k;
	int flag=0;				/* return non-zero if text was too large */

/*	can't use GetTextExtent in MetaFile DC, so why bother with file name */
/*	can't retrieve old text or background color in MetaFile DC */
	if (bCopyFlag != 0) return 0; 						/* 95/April/15 */

	if (*filename == '\0') return 0;
	atsize = (((long) CaptionFontSize) * 16384) / 5; 
	nHeight	= mapd(atsize);
	if (captionindex < 0) {	/* don't know yet what to use ? 93/Aug/26 */
/*		for (k = 0; k < 16; k++) */
/*	Reuse FakeFonts in dviwindo.c instead */
		for (k = 4; k < 16; k++) {
/*			if (strcmp(CaptionFonts[k], "") == 0) break; */
			if (strcmp(FakeFonts[k], "") == 0) break;
/*			hFont = createatmfont(CaptionFonts[k], nHeight, 0, 0, 0, 0); */
			hFont = createatmfont(FakeFonts[k], nHeight, 0, 0, 0, 0);
			if (hFont != NULL) {
				captionindex=k;
#ifdef DEBUGGING
				if (bDebug > 1) {
					sprintf(debugstr, "Using %s (%d) for caption font",
/*							CaptionFonts[k], captionttf ? " (TT)" : "", k);*/
							FakeFonts[k], k);
					OutputDebugString(debugstr);
				}
#endif
				break;
			}
		}
/*		if (captionindex < 0) {
			if (bDebug > 1)
				OutputDebugString("Can't find caption font!\n"); 
		} */ /* debugging */
	}
/*	we found a suitable caption font on a previous visit ... */	
/*	else hFont = createatmfont(CaptionFonts[captionindex], nHeight, 0, 0, 0, 0);*/
	else hFont = createatmfont(FakeFonts[captionindex], nHeight, 0, 0, 0, 0);

/*	if (bCopyFlag != 0) return 0; */					/* 95/April/15 */
/*	we hope ATM is not in some weird reencoded state at this point ... */
	if (hFont != NULL) 	hOldFont = SelectObject(hDC, hFont); 
	if (bCopyFlag) {
		dx = 0; dy = 0;
	}
	else {
/*	use GetTextExtentPoint32 in WIN32 ? */
/*	this is just for showing file name - no need to reencode */

		(void) GetTextExtentPoint32(hDC, filename,
								  (int) strlen(filename), &TextSize);

/*		(void) GetTextExtentPoint(hDC, filename,
								  (int) strlen(filename), &TextSize); */

		dx = TextSize.cx;
		dy = TextSize.cy;
	}
/*  we don't get here if bCopyFlag != 0 */
/*  otherwise return from SetTextColor & SetBkColor would be useless ... */
/*	SavedTextColor = SetTextColor(hDC, RGB(128, 0, 128));*/	/* frame color */
	SavedTextColor = SetTextColor(hDC, FrameColor); /* frame color 94/Sep/20 */
	SavedBackColor = SetBkColor(hDC, BkColor);		/* 92/June/25 */

/*	only show the darn name it fits in the box */
	if (dx < (xur - xll) && dy < (yur - yll)) {
		xmd = (xll + xur - dx) / 2;
		ymd = (yll + yur - dy) / 2;
		 MoveToEx(hDC, xmd, ymd, NULL); 
/*		this is just for showing file name - no need to reencode */
		(void) TextOut(hDC, 0, 0, filename, (int) strlen(filename));
	}
	else flag = 1;							/* indicate failure */
/*	if ((UINT) SavedTextColor > 1) */
		(void) SetTextColor(hDC, SavedTextColor);
/*	if ((UINT) SavedBackColor > 1) */
		(void) SetBkColor(hDC, SavedBackColor);	/* 92/June/25 */
	if (hFont != NULL) {
		if ((UINT) hOldFont > 1) /* avoid Windows 3.0 MetaFile problem */
			(void) SelectObject(hDC, hOldFont);
		if ((UINT) hFont > 1)
			(void) DeleteObject(hFont);
	}
	cp_valid = 0;  /* ??? */
	return flag;
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

long ImageWidth, ImageLength;				/* width and height of image */

long BitsPerSample, ExtraSamples;

long SamplesPerPixel, SamplesPerPixelX;

long compression, Orientation;

long PlanarConfiguration, Predictor;

long colormap;

int colormaptype, colormaplength;

int nColors;

long BitsPerPixel;		/* BitsPerSample * SamplePerPixel */
long BitsPerPixelX;		/* BitsPerSample * (SamplePerPixel-ExtraSamples) */

long MinSampleValue, MaxSampleValue;

int ResolutionUnit=2;				/* 93/Oct/20 */

/* long xresnum, xresden; */				/* 93/Oct/20 */
/* long yresnum, yresden; */				/* 93/Oct/20 */

unsigned long xresnum, xresden;				/* 95/Oct/10 */
unsigned long yresnum, yresden;				/* 95/Oct/10 */

int hptagflag=0;					/* non-zero if call from HPTAG */
/* may need something more elaborate hleft / hright and vhigh / vlow */

long PhotometricInterpretation;		/* 0 for reversed binary images */

int bCMYK=0;			/* non-zero if CMYK rather than RGB 2000 May 27 */

int mingrey, maxgrey, flip;

long StripOffset, StripByteCount;

long StripOffsetsPtr;
int StripOffsetsType;
int StripOffsetsLength;

long StripByteCountsPtr;
int StripByteCountsType;
int StripByteCountsLength;

long RowsPerStrip, StripsPerImage;

long InRowLength; 		/* number of source bytes from file */
long InRowLengthX; 		/* after removing ExtraSamples */
long OutRowLength;		/* number of processed bytes for output */

long BytesPerRow;		/* number of destination bytes in bitmap */

/* int checksum; */

/* x * (numer / denom) 1995/Oct/12 */

long MulRatio (long x, unsigned long numer, unsigned long denom) {
	if (denom == 0 || numer == 0) return 0;
	else return(MulDiv(x, numer, denom));		/* 97/Sep/25 */
}

/* long MulRatio(long x, unsigned long numer, unsigned long denom) {
	long result;
	if (denom == 0 || numer == 0) return 0;
	if ((numer % denom) == 0) {
		numer = numer / denom;
		denom = 1;
	}
	if ((denom % numer) == 0) {
		denom = denom / numer;
		numer = 1;
	}
	while (denom > 65536 || numer > 65536) {
		denom = denom >> 1;
		numer = numer >> 1;
	}
	result = x / denom * numer;
	return result;		
} */

int wmfflag=0;			/* set in readimagefile */
int bmpflag=0;			/* set in readimagefile */

/* actually insert the TIFF (or WMF? or BMP?) figure here */
/* negative nifd indicates figure missing - draw box and filename only */
/* if dheight == 0 => calculate dheight from dwidth based on aspect ratio */

/* added scaling (xscale, yscale) possibility 99/July/2 */

void showtiffhere (HDC hDC, char *filename, long dwidth, long dheight, 
				  double xscale, double yscale,	int nifd) {
	int xll=0, yll=0, xur=0, yur=0;
	RECT FigureRect;
#ifdef DEBUGTIFF
	char *stype;
#endif

#ifdef DEBUGTIFF
/*	can't use wmfflag and bmpflag yet - since not set */
	if (strstr(filename, ".wmf") != NULL) stype = "WMF";
	else if (strstr(filename, ".tif") != NULL) stype = "TIFF";
	else if (strstr(filename, ".bmp") != NULL) stype = "BMP";
	else if (strstr(filename, ".eps") != NULL) stype = "EPS";
	else stype="";
#endif

#ifdef DEBUGTIFF
	if (bDebug > 1) {
/*		sprintf(debugstr, "ShowTIFFHere: %s dwidth %ld dheight %ld", */
		sprintf(debugstr, "Show%sHere: %s dwidth %ld dheight %ld",
				stype, filename, dwidth, dheight);
		OutputDebugString(debugstr); 
	}  	/* debugging 95/April/13 */
#endif

	if (bShowFlag != 0 && bTextOutFlag != 0) {
/*		if (dheight == 0) */
		if (dheight == 0 || dwidth == 0) { /* do a pre-scan to extract fields */
/*			(void) readimagefile(hDC, filename, xll, yll, xur, yur, nifd, 0) */
			if (readimagefile(hDC, filename, xll, yll, xur, yur, nifd, 0)
				!= 0) return;						/* 94/Nov/19 */
/*	check again after prescan */
#ifdef DEBUGTIFF
			if (bDebug > 1) {		
				sprintf(debugstr, "dwidth %ld dheight %ld AFTER PRESCAN",
						dwidth, dheight);
				OutputDebugString(debugstr);
			}		/* debugging 95/April/13 */
#endif
			if (dwidth == 0 && dheight == 0) {		/* 95/April/13 ? */
/*				dwidth = (ImageWidth * xresden / xresnum) << 16; */
/*				dheight = (ImageLength * yresden / yresnum) << 16; */
				if (xresnum == 0 || yresnum == 0) {		/* 94/Nov/19 */
					if (bDebug > 1) {
						sprintf(debugstr,
								"xres %lu / %lu yres %lu / %lu unit %d",
								xresnum, xresden, yresnum, yresden, 
								ResolutionUnit);
						OutputDebugString(debugstr);
					}
/*					xresnum = yresnum = 72; */ /* 96/Apr/3 */
					xresnum = yresnum = nDefaultTIFFDPI;
/*					return; */					/* 1996/Apr/4 */
				}
/*				dwidth = ((long) ImageWidth << 16) / xresnum * xresden; */
				dwidth = MulRatio ((long) ImageWidth << 16, xresden, xresnum);
/*				dheight = ((long) ImageLength << 16) / yresnum * yresden; */
				dheight = MulRatio ((long) ImageLength << 16, yresden, yresnum);
				if (ResolutionUnit == 1) {			/* what to do ? */
					if (bDebug > 1)	OutputDebugString("ResolutionUnit == 1");
				}
				else if (ResolutionUnit == 2) {		/* 72.27 pt per in */
					dwidth = dwidth / 100 * 7227;
					dheight = dheight / 100 * 7227;
				}
				else if (ResolutionUnit == 3) {		/* 28.45 pt per cm */
					dwidth = dwidth / 100 * 2845;
					dheight = dheight / 100 * 2845;
				}
			} /* if dwidth == 0 && dheight == 0 *after* prescan */
/* changed order of multiplication and division to avoid overflow 94/Dec/16 */
			else if (dheight == 0) {
				if (ImageWidth != 0) 
/*					dheight = (ImageLength * dwidth) / ImageWidth; */
/*					dheight = (dwidth / ImageWidth) * ImageLength; */
					dheight = MulRatio(dwidth, ImageLength, ImageWidth);
			}
			else if (dwidth == 0) {
				if (ImageLength != 0)
/*					dwidth = (ImageWidth * dheight) / ImageLength; */
/*					dwidth = (dheight / ImageLength) * ImageWidth; */
					dwidth = MulRatio(dheight, ImageWidth, ImageLength); 
			}
/*			added 99/July/2 */
			if (xscale != 0.0) dwidth = (long) ((double) dwidth * xscale);
			if (yscale != 0.0) dheight = (long) ((double) dheight * yscale);
		} /* if dwidth == 0 || dheight == 0 when we come into readtiffimage */

#ifdef DEBUGTIFF
		if (bDebug > 1) {		
			sprintf(debugstr, "Show%sHere: dwidth %ld dheight %ld",
					stype, dwidth, dheight);
			OutputDebugString(debugstr);
		}					/* 94/Dec/16 */
#endif
		xll = mapx(dvi_h);
		xur = mapx(dvi_h + dwidth);
		if (hptagflag) {						/* 95/Oct/12 */
			yll = mapy(dvi_v + dheight);
			yur = mapy(dvi_v);
		}
		else {			/* current point is at bottom left corner */
			yll = mapy(dvi_v);
			yur = mapy(dvi_v - dheight);
		}
#ifdef DEBUGTIFF
		if (bDebug > 1) {
			sprintf(debugstr, "Show%sHere: xll %d yll %d xur %d yur %d",
					stype, xll, yll, xur, yur);
			OutputDebugString(debugstr); 	/* experiment */
		}  	/* debugging 95/April/13 */
#endif

		FigureRect.left = xll; FigureRect.right = xur;
		FigureRect.bottom = yll; FigureRect.top = yur;
/*		First check whether the TIFF image is even visible */
/*		We won't worry here about Windows 95 bug in RectVisible *//* why ??? */
/*		Avoid RectVisible() in MetaFile - use InterSectRect instead */
		if ((bCopyFlag == 0 && RectVisible(hDC, &FigureRect) != 0) ||
			(bCopyFlag != 0 && InterSectRect(&CopyClipRect, &FigureRect) !=	0)) {
/*			if (bBoxFigure != 0) DrawFigureBox(hDC, xll, yll, xur, yur);
			if (bShowFileName != 0) 
				ShowNameFile(hDC, xll, yll, xur, yur, filename); */
			if (bShowImage != 0) {
				if(nifd < 0 || bShowPreview == 0 ||		/* 93/April/9 */
				   readimagefile(hDC, filename, xll, yll, xur, yur, nifd, -1)
					   < 0) {
					 if (bShowFileName != 0) {
						 DrawFigureBox(hDC, xll, yll, xur, yur);
						 ShowNameFile(hDC, xll, yll, xur, yur, filename);
					 }
				}
			}
		}
#ifdef DEBUGTIFF
		else if (bDebug > 1) {
			sprintf(debugstr, "Not visible xll %d yll %d xur %d yur %d",
					xll, yll, xur, yur);
			OutputDebugString (str);
		}
#endif
	}
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* This attempts to do Textures style color calls */

/* Does not do anything about rule color, only text color */
/* Does not do anything about BackgroundColor only TextColor */
/* Does not do anything fancy about CMYK instead of RGB */
/* Does not do anything about page boundaries - should it ? */

/* 1 - .30 * C - .59 * M - .11 * Y - K => setgray on B/W devices */
/* .30 * R + .59 * G + .11 * B => setgray on B/W devices ? */

/* Arguments r, g, b here are from 0 - 255 */

void SetupRuleColor (HDC hDC, int r, int g, int b) { /* split out 95/Mar/8 */
	HPEN hOldPen;

/*	use color also for TPIC outline pen operations */
	rule_r = r; rule_g = g; rule_b = b;		/* save for TPIC \specials */
/*	use color also for TPIC fill operations 96/Mar/31 */
	tpic_r = r; tpic_g = g; tpic_b = b;		/* save for TPIC \specials */

/*	need RulePen whether filling rules or not */
	if (r == 0 && g == 0 && b == 0)	hRulePen = hBlackPen;
/*	else hRulePen = CreatePen(PS_SOLID, 10, RGB(r, g, b)); */
	else hRulePen = CreatePen(PS_SOLID, 0, RGB(r, g, b)); 
	hOldPen = SelectObject(hDC, hRulePen);
/*	if (hOldPen != hBlackPen) */
	if (hOldPen != hRulePenDefault) {
		if ((UINT) hOldPen > 1) (void) DeleteObject(hOldPen);
	}
/*	need RuleBrush only when filling rules */
/*	then it exists, but not selected into device context */
	if (bRuleFillFlag != 0) {
		if (hRuleBrush != NULL && hRuleBrush != hRuleBrushDefault) {
			if ((UINT) hRuleBrush > 1) (void) DeleteObject(hRuleBrush);
		}
		if (r == 0 && g == 0 && b == 0)	hRuleBrush = hBlackBrush;
		else hRuleBrush = CreateSolidBrush(RGB(r, g, b)); 
	}
/*	OldRuleColor = RuleColor; */
	RuleColor = RGB(r, g, b);			/* 94/July/6 */
}

/* clean up color stack and reset screen state */
/* called from winanal.c at do_eop */

void CheckColorStack(HDC hDC) {			/* 96/Nov/3 ??? */
	int r, g, b;
	if (colorindex == 0) return;		/* nothing left on stack */
	if (bCarryColor == 0) colorindex = 0;
	if (colorindex == 0)		{ 		/* 98/Feb/14 ??? */
		CurrentTextColor = ColorStack[colorindex];
		(void) SetTextColor(hDC, CurrentTextColor);
		r = GetRValue (CurrentTextColor);
		g = GetGValue (CurrentTextColor);
		b = GetBValue (CurrentTextColor);
		SetupRuleColor(hDC, r, g, b);
	} 
}

/* Ugh: Presumably GetTextColor won't work in Copying to clipboard ??? */

void doColorPush (HDC hDC) {
/*	if (bCopyFlag == 0)	ColorStack[colorindex] = GetTextColor (hDC);
	else ColorStack[colorindex] = CurrentTextColor; */
	ColorStack[colorindex] = CurrentTextColor;	/* 98/Feb/21 */
#ifdef DEBUGCOLORSTACK
	if (bDebug > 1) {
		int r, g, b;
		r = GetRValue (CurrentTextColor);
		g = GetGValue (CurrentTextColor);
		b = GetBValue (CurrentTextColor);
		sprintf(debugstr, "doColorPUSH colorindex %d RGB %d %d %d",
				colorindex, r, g, b);
		OutputDebugString(debugstr);
	}
#endif
	colorindex++;
	if (colorindex >= MAXCOLORSTACK) /* quietly overflow */
		colorindex = MAXCOLORSTACK-1;
}	/* hDC unreferenced */

void doColorPop (HDC hDC) {
	int r, g, b;
/*	colorindex--; */
/*	if (colorindex < 0)	colorindex = 0; */
/*	quietly ignore stack underflow */ /* happens on first page */
	if (colorindex <= 0) colorindex = 1; 
	CurrentTextColor = ColorStack[--colorindex];		/* pop */
/*			if ((UINT) CurrentTextColor > 1) */	/* verify its OK */
	(void) SetTextColor(hDC, CurrentTextColor);		/* set */
/*			OldColor = ColorStack[colorindex]; */	/* rule color ? */
	r = GetRValue (CurrentTextColor);
	g = GetGValue (CurrentTextColor);
	b = GetBValue (CurrentTextColor);
	SetupRuleColor(hDC, r, g, b);
#ifdef DEBUGCOLORSTACK
	if (bDebug > 1) {
		sprintf(debugstr, "doColorPOP  colorindex %d RGB %d %d %d",
				colorindex, r, g, b);
		OutputDebugString(debugstr);
	}
#endif
}

/* Do DVIPS / Textures style color calls */
/* \special{color cmyk <c> <m> <y> <k>} */
/* \special{color rgb <r> <g> <b>} */
/* \special{color gray <d>} */
/* \special{color push} and \special{color pop} */
/* also allow combinations such as: \special{color push rgb <r> <g> <b>} */
/* also called from winpslog.c in checkspecial in scanlogfile c == 0*/
/* to maintain current color and stack for end of page */

/* c == 0 during prescan */

void DoColor (HDC hDC, int input, int c) {	/* 95/Feb/26 */
	char *s;
	int r, g, b, n, setcolor=0;
	double fr, fg, fb;
	double fc, fy, fm, fk;

	s = line + strlen(line);
	if (c > 0) *s++ = (char) c;				/* stick in terminator */
	*s = '\0';								/* just in case */
	if (nspecial > 0)						/* read rest of line */
		readspecial (input, s, sizeof(line) - strlen(line));	
	if (c > 0) {							/* 98/Feb/15 */
		if (bColorFont != 0 || markfont >= 0) return;		/* 96/Nov/3 */
	}
	else {									/* prescan situation 98/Feb/15 */
		s = line;
		while (*s != ' ' && *s != '\0') s++;	/* step up to white space */
	}

/*	if (bDebug > 1) OutputDebugString(line); */	/* 99/Feb/21 test */

	for(;;) {								/* allow multiple commands */
		while (*s <= ' ' && *s != '\0') s++;	/* step over white space */
		if (*s == '\0') break;				/* processed all of them */
		if (strncmp(s, "pop", 3) == 0) {
			doColorPop(hDC);
			s += 3;
		}
		else if (strncmp(s, "popall", 6) == 0) {			/* 96/Nov/3 */
			colorindex = 0;
			CurrentTextColor = ColorStack[colorindex];
/*			if ((UINT) CurrentTextColor > 1) */	/* verify its OK */
			(void) SetTextColor(hDC, CurrentTextColor);
/*			OldColor = ColorStack[colorindex]; */	/* rule color ? */
			r = GetRValue (CurrentTextColor);
			g = GetGValue (CurrentTextColor);
			b = GetBValue (CurrentTextColor);
			SetupRuleColor(hDC, r, g, b);
			s += 6;
		}
		else if (strncmp(s, "push", 4) == 0) {
			doColorPush(hDC);
			s += 4;
		}
		else if (strncmp(s, "rgb", 3) == 0) {
			s += 3;
			if (sscanf(s, "%lg %lg %lg%n", &fr, &fg, &fb, &n) == 3) {
				setcolor=1;
				s += n;
			}
			else if (bComplainSpecial != 0)	{
				winbadspecial(line);				/* 96/Feb/4 */
				break;
			}
		}
		else if (strncmp(s, "cmyk", 4) == 0) {
			s += 4;
			if (sscanf(s, "%lg %lg %lg %lg%n", &fc, &fm, &fy, &fk, &n) == 4) {
				setcolor=1;
				fr = 1.0 - fc - fk;
				fg = 1.0 - fm - fk;
				fb = 1.0 - fy - fk;
				if (fr < 0.0) fr = 0.0;
				if (fg < 0.0) fg = 0.0;
				if (fb < 0.0) fb = 0.0;
				s += n;
			}
			else if (bComplainSpecial != 0)	{
				winbadspecial(line);				/* 96/Nov/3 */
				break;
			}
		}
		else if (strncmp(s, "gray", 4) == 0) {
			s += 4;
			if (sscanf(s, "%lg%n", &fk, &n) == 1) {
				setcolor=1;
				fr = fg = fb = fk;
				s += n;
			}
			else if (bComplainSpecial != 0)	{
				winbadspecial(line);				/* 96/Feb/4 */
				break;
			}
		}
		else if (_strnicmp(s, "black", 5) == 0) { /* 96/Feb/4 */
			s += 5;
			setcolor=1;
			fr = fg = fb = 0.0;
		}
		else if (_strnicmp(s, "white", 5) == 0) { /* 96/Feb/4 */
			s += 5;
			setcolor=1;
			fr = fg = fb = 1.0;
		}
		else if (bComplainSpecial != 0)	{	/* 95/April/30 */
			winbadspecial(line);				/* 96/Feb/4 */
			break;
		}

		if (setcolor) {	/* if "rgb" or "cmyk" or "gray" seen */
			r = (int) (fr * 255.9999);
			g = (int) (fg * 255.9999);
			b = (int) (fb * 255.9999);
			if (OldFore >= 0 || OldBack >= 0) {
				r = 255 - r; g = 255 - g; b = 255 - b;
			} 
			CurrentTextColor = RGB(r, g, b);
			if (c > 0) {
				OldTextColor = SetTextColor(hDC, CurrentTextColor);
				SetupRuleColor (hDC, r, g, b);				/* 95/Mar/8 */
			}
		}	/* if (setcolor) */
	} /* for ( ; ; ) */
}

/* Attempt at \special{background rgb 0 0 1} support 98 June 4 */
/* so far incomplete - effect is to just ignore background \special */

/* c == 0 during prescan (and this should not be called otherwise) */

void DoBackGround (HDC hDC, int input, int c) {	/* 95/Feb/26 */
	char *s;
	int r, g, b, n, setcolor=0;
	double fr, fg, fb;
	double fc, fy, fm, fk;

	s = line + strlen(line);
	if (c > 0) *s++ = (char) c;				/* stick in terminator */
	*s = '\0';								/* just in case */
	if (nspecial > 0)						/* read rest of line */
		readspecial (input, s, sizeof(line) - strlen(line));

/*	if (bDebug > 1) OutputDebugString(line); */	/* 99/Feb/21 test */

	if (c > 0) return;						/* only do in prescan ! */
	if (c > 0) {							/* 98/Feb/15 */
		if (bColorFont != 0 || markfont >= 0) return;		/* 96/Nov/3 */
	}
	s = line;
	while (*s != ' ' && *s != '\0') s++;	/* step up to white space */
	while (*s <= ' ' && *s != '\0') s++;	/* step over white space */
	if (*s == '\0') return;
	if (strncmp(s, "rgb", 3) == 0) {
		s += 3;
		if (sscanf(s, "%lg %lg %lg%n", &fr, &fg, &fb, &n) == 3) {
			setcolor=1;
			s += n;
		}
		else if (bComplainSpecial != 0)	{
			winbadspecial(line);				/* 96/Feb/4 */
			return;
		}
	}
	else if (strncmp(s, "cmyk", 4) == 0) {
		s += 4;
		if (sscanf(s, "%lg %lg %lg %lg%n", &fc, &fm, &fy, &fk, &n) == 4) {
			setcolor=1;
			fr = 1.0 - fc - fk;
			fg = 1.0 - fm - fk;
			fb = 1.0 - fy - fk;
			if (fr < 0.0) fr = 0.0;
			if (fg < 0.0) fg = 0.0;
			if (fb < 0.0) fb = 0.0;
			s += n;
		}
		else if (bComplainSpecial != 0)	{
			winbadspecial(line);				/* 96/Nov/3 */
			return;
		}
	}
	else if (strncmp(s, "gray", 4) == 0) {
		s += 4;
		if (sscanf(s, "%lg%n", &fk, &n) == 1) {
			setcolor=1;
			fr = fg = fb = fk;
			s += n;
		}
		else if (bComplainSpecial != 0)	{
			winbadspecial(line);				/* 96/Feb/4 */
			return;
		}
	}
	else if (_strnicmp(s, "black", 5) == 0) { /* 96/Feb/4 */
		s += 5;
		setcolor=1;
		fr = fg = fb = 0.0;
	}
	else if (_strnicmp(s, "white", 5) == 0) { /* 96/Feb/4 */
		s += 5;
		setcolor=1;
		fr = fg = fb = 1.0;
	}
	else if (bComplainSpecial != 0)	{	/* 95/April/30 */
		winbadspecial(line);				/* 96/Feb/4 */
		return;
	}

	if (setcolor) {	/* if "rgb" or "cmyk" or "gray" seen */
		r = (int) (fr * 255.9999);
		g = (int) (fg * 255.9999);
		b = (int) (fb * 255.9999);
		if (OldFore >= 0 || OldBack >= 0) {
			r = 255 - r; g = 255 - g; b = 255 - b;
		} 
		GrabBack();
/*		lpBack[dvipage] = RGB(r, g, b); */
		lpBack[pageno] = RGB(r, g, b);
/*		ReleaseBack(); */
		if (bDebug > 1) {
			sprintf(debugstr, "page %d back clr %X (rgb %d %d %d)",
				   pageno, lpBack[dvipage], r, g, b);
			OutputDebugString(debugstr);
		}
		ReleaseBack();
/*		actually, ignore while doing page ? *//* called from winpslog.c */
/*		if (c > 0) {
			OldBackColor = SetBkColor(hDC, CurrentBackColor);
		} */
	}
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* Look at \special string, extract string and strip leading white space */
/* Remove enclosing "..." if present */
/* White space or comma or end of special is delimiter */
/* Also returns pointer to buttoname in original string */

char *getbuttonname (char buttonname[], char *s) {
	int m;
	char *t;

/*	while (*s <= ' ') s++;	*/			/* search for non-space */
	while (*s <= ' ' && *s > 0) s++;	/* search for non-space */
	if (*s == '\"') {					/* deal with "..." case */
		s++; 
		t = s;
/*		while (*t != '\"' && t < s + MAXMARKS) t++; */
		while (*t != '\"' && *t > 0 && t < s + MAXMARKS) t++;
	}
	else {
		t = s;
/*		while (*t > ' ' && t < s + MAXMARKS) t++; */ /* 95/Dec/20 */
		while (*t > ' ' && *t != ',' && t < s + MAXMARKS) t++;
	}
/*	*t = '\0'; */
	if (t != s) {
		m = t - s;
		if (m > MAXMARKS) m = MAXMARKS;	/* 94/Dec/13 */
		strncpy(buttonname, s, m);
		buttonname[m] = '\0';
	}
	return s;
}		/* returns pointer into special string where button name starts */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* new to allow quoted file names with spaces 98/Jul/9 */

char *scaninsert(char *line, char *filename) {
	char *s = line;
	char *t = filename;

	*filename = '\0';

	while (*s <= ' ' && *s != '\0') s++;	/* step over white space */

	if (*s == '\0') return 0;

	if (*s == '\"') {			/* is it quoted file name ? */
		s++;					/* step over initial " */
		while (*s != '\"' && *s != '\0') *t++ = *s++;
		if (*s != '\0') s++;					/* step over final " */
	}
	else {						/* normal file name */
		while (*s > ' ' && *s != '\0') *t++ = *s++;
	}
	*t = '\0';
	if (*s <= ' ' && *s != '\0') s++;	/* step over white space after name */
/*	special hack to convert ## to # */
	if ((t = strstr(filename, "##")) != NULL) strcpy(t, t+1);
	if (bDebug > 1) {
		sprintf(debugstr, "line: %s filename: %s", line, filename);
		OutputDebugString(debugstr);
	}
	return s;
}

/* int rule_r, rule_g, rule_b; */

/* main entry point coming from winanal.c */

void dospecial(HDC hDC, int input, unsigned long n) {
	HPEN hOldPen;
	int nifd;
	char filename[MAXFILENAME];
	long dwidth, dheight;
	double xscale, yscale;
	long dx, dy;
	long xll, yll, xur, yur;
/*	long offadj; */
	int dxll, dyll, dxur, dyur;
/*	int tmp; */
	int c, m;
	int button;
	char *s, *samp, *t;
/*	char *t; */
	char buttonname[MAXMARKS+1]="";	/* label of button */
	int r=0, g=0, b=0, rback=255, gback=255, bback=255;
	double fr=0.0, fg=0.0, fb=0.0; 	/* experiment 94/Mar/3 */
	double frback=0.0, fgback=0.0, fbback=0.0; 	/* experiment 94/Mar/3 */
/*	RECT RuleRect; */

	nspecialsav = nspecial;			/* save length for error message output */
	specstart = an_wintell(input);	/* save start in case of error (?) */

	if (bIgnoreSpecial != 0) {
		flushspecial(input);
		return;
	}

	c = an_wingetc(input); --nspecial;
	while (c == ' ') {				/* flush leading spaces - 1999/Apr/23 */
		c = an_wingetc(input); --nspecial;
	}

	if (c == 0 && bFirstNull) {		/* 96/Aug/29 */
		flushspecial(input);
		return;
	}

/*  NO support for `literal graphics' kludge in dvips - start with " */
	if (c == '\"') {	 
		flushspecial(input);			/* basically for now */
/* 		copystring(input); */			/* PS passthrough ? */
		return;
	}
/*  NO support for `literal macros' kludge in dvips - start with ! */
	if (c == '!') {						/* 1997 Nov 26 */
		flushspecial(input);			/* basically for now */
/* 		copystring(input); */			/* should be done in prepass */
		return;
	}

	an_unwingetc(c, input);
	nspecial++;

	if ((c = getalphatoken(input, line, sizeof(line))) == 0) {
		if (bComplainSpecial != 0)	winbadspecial("Blank \\special");
		return;	
	}

	if (strncmp(line, "if", 2) == 0) {	/* new conditional option 99/July/2 */
		if (strcmp(line+2, "view") != 0) {	/* ignore all but ifview */
			flushspecial(input);	/* not for DVIWindo */
			return;
		}
		else if ((c = getalphatoken(input, line, sizeof(line))) == 0) { /* recurse */
			if (bComplainSpecial != 0)	winbadspecial("Blank \\special");
			return;					/* found nothing ! */
		}
	}	/* drop through here normally */

/*  see whether this is one of the new ones recognized here in DVIWindo */
	if ((strcmp(line, "textcolor") == 0 ||
		strcmp(line, "rulecolor") == 0 ||
		strcmp(line, "figurecolor") == 0 ||
		strcmp(line, "reversevideo") == 0 ||	
		strcmp(line, "button") == 0 ||
		strcmp(line, "mark") == 0 ||			
		strcmp(line, "viewrule") == 0 ||	/* 95/Mar/27 */
		strcmp(line, "viewtext") == 0 ||	/* 95/Mar/27 */
		strcmp(line, "insertimage") == 0 ||
		strcmp(line, "insertmf") == 0)		/* 94/Sep/27 */
			&& n < BUFLEN) {
/*		don't look at it if is extremely long */
/*		odd sort of test, since char line[MAXLINE], char str[BUFLEN] */
		s = line + strlen(line);
		*s++ = (char) c;	/* stick in terminator */
		readspecial(input, s, sizeof(line) - strlen(line));	
	}
	else {								/* not one of the DVIWindo specials */
		analspecial(hDC, input, c);
		flushspecial(input);			/* can't hurt ... */
		return;
	}
	
/*	Deal with the known DVIWindo \specials now */

	if (strncmp(line, "textcolor:", 10) == 0) {
		if (bColorFont == 0 && markfont < 0) {
			if (strstr(line, "revert") != NULL) {
				(void) SetTextColor(hDC, TextColor);	/* 92/May/07 */
				CurrentTextColor = TextColor;			/* 95/April/30 */
/*				(void) SetTextColor(hDC, OldTextColor); */
			}
/*			else if(sscanf(line, "textcolor: %d %d %d", &r, &g, &b) == 3) */
			else {
				if ((s = strchr(line, '.')) == NULL || *(s+1) < '0') {
					if (sscanf(line, "textcolor: %d %d %d", &r, &g, &b) == 3);
					else if (bComplainSpecial != 0)	{
/*						wincancel(line); */
						winbadspecial(line);				/* 96/Feb/4 */
					}
				}
				else {
					if (sscanf(line, "textcolor: %lg %lg %lg", &fr, &fg, &fb)
						== 3) {
						r = (int) (fr * 255.9999);
						g = (int) (fg * 255.9999);
						b = (int) (fb * 255.9999);
					}
					else if (bComplainSpecial != 0)	{
/*						wincancel(line); */
						winbadspecial(line);				/* 96/Feb/4 */
					}
				}
				if (OldFore >= 0 || OldBack >= 0) {
					r = 255 - r; g = 255 - g; b = 255 - b;
				} 
/*				text_r = r; text_g = g; text_b = b; */
				CurrentTextColor = RGB(r, g, b);
/*				OldTextColor = SetTextColor(hDC, RGB(r, g, b)); */
				OldTextColor = SetTextColor(hDC, CurrentTextColor);
			}
/* 			else if (bComplainSpecial != 0)	wincancel(line); */
/*				wincancel("Don't understand `textcolor' special"); */
		}
	}
	else if (strncmp(line, "rulecolor:", 10) == 0) { 
		if (strstr(line, "revert") != NULL) {	/* new 92/May/07 */
			hRulePen = hRulePenDefault;
			hOldPen = SelectObject(hDC, hRulePen);
			if (hOldPen != hRulePenDefault) {
				if ((UINT) hOldPen > 1)
					(void) DeleteObject(hOldPen);
			}
			if (bRuleFillFlag != 0) {
				if (hRuleBrush != hRuleBrushDefault) {
				if ((UINT) hRuleBrush > 1)
					(void) DeleteObject(hRuleBrush);					
				}
				hRuleBrush = hRuleBrushDefault;
			}
/*			RuleColor = RGB(0,0,0); */
			RuleColor = TextColor;
		} 
/*		else if (sscanf(line, "rulecolor: %d %d %d", &r, &g, &b) == 3) */
		else {
			if ((s = strchr(line, '.')) == NULL || *(s+1) < '0') {	
				if (sscanf(line, "rulecolor: %d %d %d", &r, &g, &b) == 3) ;
				else if (bComplainSpecial != 0)	{
/*					wincancel(line); */
					winbadspecial(line);				/* 96/Feb/4 */
				}
			}
			else {
				if (sscanf(line, "rulecolor: %lg %lg %lg", &fr, &fg, &fb)
					==3) {
					r = (int) (fr * 255.9999);
					g = (int) (fg * 255.9999);
					b = (int) (fb * 255.9999);
				}
				else if (bComplainSpecial != 0)	{
/*					wincancel(line); */
					winbadspecial(line);				/* 96/Feb/4 */
				}
			}
			SetupRuleColor(hDC, r, g, b);			/* spliced out 95/Mar/8 */
#ifdef IGNORED
			rule_r = r; rule_g = g; rule_b = b;		/* 92/May/09 */
/* need RulePen whether filling rules or not */
/*			if (bRuleFillFlag == 0) */
				if (r == 0 && g == 0 && b == 0)	hRulePen = hBlackPen;
/*				else hRulePen = CreatePen(PS_SOLID, 10, RGB(r, g, b)); */
				else hRulePen = CreatePen(PS_SOLID, 0, RGB(r, g, b)); 
				hOldPen = SelectObject(hDC, hRulePen);
/*				if (hOldPen != hBlackPen) */
				if (hOldPen != hRulePenDefault) {
					if ((UINT) hOldPen > 1)
						(void) DeleteObject(hOldPen);
				}
/*			} */
/* need RuleBrush only when filling rules */
			if (bRuleFillFlag != 0) {
/*			else */
				if (hRuleBrush != NULL && hRuleBrush != hRuleBrushDefault) {
					if ((UINT) hRuleBrush > 1) (void) DeleteObject(hRuleBrush);
				}
				if (r == 0 && g == 0 && b == 0)	hRuleBrush = hBlackBrush;
				else hRuleBrush = CreateSolidBrush(RGB(r, g, b)); 
			}
/*			OldRuleColor = RuleColor; */		/* 95/Mar/8 */
			RuleColor = RGB(r, g, b);			/* 94/July/6 */
#endif
		}	/*	bColorFont == 0 && markfont < 0 */
	}	 /* end of rulecolor */
	else if (strncmp(line, "figurecolor:", 12) == 0) {
		if (strstr(line, "revert") != NULL) {
/*			FigureColor = OldFigureColor;	*/
			FigureColor = TextColor;			/* 92/May/07 */
/*			BackColor = OldBackColor;	*/
			BackColor = BkColor;				/* 92/May/07 */
		}
		else {
			if ((s = strchr(line, '.')) == NULL || *(s+1) < '0') {
				if (sscanf(line, "figurecolor: %d %d %d %d %d %d",
					&r, &g, &b, &rback, &gback, &bback) >= 3) ;
				else if (bComplainSpecial != 0)	{
/*					wincancel(line); */
					winbadspecial(line);				/* 96/Feb/4 */
				}
			}
			else {
				if (sscanf(line, "figurecolor: %lg %lg %lg %lg %lg %lg",
					&fr, &fg, &fb, &frback, &fgback, &fbback) >= 3) {
					r = (int) (fr * 255.9999);
					g = (int) (fg * 255.9999);
					b = (int) (fb * 255.9999);
					rback = (int) (frback * 255.9999);
					gback = (int) (fgback * 255.9999);
					bback = (int) (fbback * 255.9999);
				}
				else if (bComplainSpecial != 0)	{
/*					wincancel(line); */
					winbadspecial(line);				/* 96/Feb/4 */
				}
			}
			OldFigureColor = FigureColor;
			FigureColor = RGB(r, g, b);
			OldBackColor = BackColor;
			BackColor = RGB(rback, gback, bback);
		}
	}
	else if (strncmp(line, "reversevideo:", 13) == 0) {
		if (strstr(line, "on") != NULL) bReverseVideo = 1;
		else if (strstr(line, "off") != NULL) bReverseVideo = 0;
		else if (strstr(line, "toggle") != NULL) 
			bReverseVideo = 1 - bReverseVideo;
		else if (bComplainSpecial != 0) {
/*			wincancel("Don't understand `reversevideo' special"); */
/*			wincancel(line); */
			winbadspecial(line);				/* 96/Feb/4 */
		}
		if (markfont < 0) colorandreverse();
	}
	else if (strncmp(line, "insertimage:", 12) == 0) {
		dwidth = dheight = 0;		/* NEW - use for isotropic scaling */
		xscale = yscale = 0.0;			/* 99/July/2 */
		nifd = 1;					/* n-th (sub-)image in TIFF file */
/*		if (sscanf(line, "insertimage: %s %ld %ld %d", 
			filename, &dwidth, &dheight, &nifd) */
		s = scaninsert(line+12, filename);
		if ((t = strstr(s, "scaled")) == NULL) {
			sscanf(s, "%ld %ld %d", &dwidth, &dheight, &nifd); /* normal */
		}
		else {	/* new case 99/July/2 */
			sscanf(t+7, "%lg %lg", &xscale, &yscale);
			if (xscale > 33.33) xscale = xscale / 1000.0;
			if (yscale > 33.33) yscale = yscale / 1000.0;
			if (xscale != 0.0 && yscale == 0.0) yscale = xscale;
			if (xscale == 0.0) xscale = 1.0;
			if (yscale == 0.0) yscale = 1.0;
		}
		if (*filename != '\0') {	/* need at least file name */
			hptagflag = 0;
			if (bShowTIFF)			/* allow suppression of 97/Jan/5 */
				showtiffhere(hDC, filename, dwidth, dheight, xscale, yscale, nifd);
		}
		else if (bComplainSpecial != 0) {
			winbadspecial(line);				/* 96/Feb/4 */
		}
	}
	else if (strncmp(line, "insertmf:", 9) == 0) {	/* 94/Sep/27 */
/*		dwidth = 0; dheight = 0; */
		xscale = yscale = 0.0;
		xll = yll = xur = yur = 0;				/* 96/Apr/4 */
/*		NEW FORM of insertmf: <filename> 0 0 <width> <height> 96/Apr/4 */
/*		if (sscanf(line, "insertmf: %s %ld %ld %ld %ld",
			filename, &xll, &yll, &xur, &yur) > 3 ) */
		s = scaninsert(line+9 , filename);
		if (sscanf(s, "%ld %ld %ld %ld", &xll, &yll, &xur, &yur) > 2 ) {
/*			showmetafile(hDC, filename,
						 (double) (xur - xll), (double) (yur - yll), 1); */
			if (bShowWMF)	/* allow suppression 97/Jan/7 */
				showmetafile(hDC, filename,
						 (double) (xur - xll), (double) (yll - yur), 1);
/*		vertical direction reversed ? 1996/July/16 */
#ifdef DEBUGMETAFILE
			if (bDebug > 1) OutputDebugString("After showmetafile");
#endif
		}
/*		OLD FORM of insertmf: <filename> <x-scale> <y-scale> */
/*		NEW FORM of insertmf: <filename> scaled <x-scale> <y-scale> */
/*		else if (sscanf(line, "insertmf: %s %lg %lg", 
		filename, &xscale, &yscale) > 0) */
		else if (*filename != '\0') {
			if ((t = strstr(s, "scaled")) != NULL) {
				sscanf(t+7, "%lg %lg", &xscale, &yscale);	/* new 99/July/2 */
			}
			else sscanf(s, "%lg %lg", &xscale, &yscale);
			if (xscale > 33.33) xscale = xscale / 1000.0;
			if (yscale > 33.33) yscale = yscale / 1000.0;
			if (xscale != 0.0 && yscale == 0.0) yscale = xscale;
			if (xscale == 0.0) xscale = 1.0;
			if (yscale == 0.0) yscale = 1.0;
/*			showmetafile(hDC, filename, xscale, yscale); */
			if (bShowWMF)	/* allow suppression 97/Jan/7 */
				showmetafile(hDC, filename, xscale, yscale, 0);
#ifdef DEBUGMETAFILE
			if (bDebug > 1) OutputDebugString("After showmetafile");
#endif
		}
		else if (bComplainSpecial != 0) {
/*			wincancel(line); */
			winbadspecial(line);				/* 96/Feb/4 */
		}
	}
	else if (strncmp(line, "mark:", 5) == 0) {	/* nothing to do */
/*		bHyperUsed = 1;*/	/* indicate this file uses hypertext 94/Oct/5 */
	}
	else if (strncmp(line, "button:", 7) == 0) {
/*		if (sscanf(line, "button: %d %ld %ld %ld %ld", 
			&button, &xll, &yll, &xur, &yur) == 5) */
		button = 0;
		if (bHyperText)			/* set, but only if HyperText implemented */
			bHyperUsed = 1;		/* indicate file uses hypertext 94/Oct/5 */

/*	optional third numeric arg is button number - no longer used ? */
		if (sscanf(line, "button: %ld %ld%n %d",
			&dx, &dy, &m, &button) >= 2) {
			xll = dvi_h; yll = dvi_v; xur = xll + dx; yur = yll - dy;
			dxll = mapx(xll); 	dyll = mapy(yll);
			dxur = mapx(xur); 	dyur = mapy(yur);

			if (bShowButtons != 0) DrawFigureBox(hDC, dxll, dyll, dxur, dyur);

/*	now get the button label */
/*	button name --- if any --- goes into buttonlabel[] - else empty string */
/*	command string --- if any --- goes into str[] - else empty string */		
			buttonname[0] = '\0';	/* reset, in case not filled 95/Aug/12 */
			s = line + m;
			s = getbuttonname (buttonname, s);
/* flush supposed button name if its a command instead 95/Aug/12 */
/* if there is no mark name and we go straight into actions ... */
/*			if (strchr(buttonname, ':') != NULL) */ /* fixed 95/Oct/3 */
			if (strncmp(s, "file:", 5) == 0 ||
				strncmp(s, "execute:", 8) == 0 || 
				strncmp(s, "launch:", 7) == 0 ||
				strncmp(s, "page:", 5) == 0) buttonname[0] = '\0'; 
/* allow for <mark> , launch: <...> , file: <...> 95/Aug/12 */
/* allow for page: <..> 96/May/18 */
/* comma is delimiter between hyper-text commands */
			else if ((samp = strchr(s, ',')) != NULL) {
				s = samp+1;
				while (*s <= ' ' && *s > 0) s++;
			}
/*	 if we grabbed launch / execute / file then keep on going */
			if (strncmp(s, "file:", 5) == 0 ||
				strncmp(s, "execute:", 8) == 0 ||
				strncmp(s, "launch:", 7) == 0 ||
				strncmp(s, "page:", 5) == 0) {
/*	maybe avoid the following to allow combined action 95/Aug/12 */
/*				(void) getbuttonname (buttonname, strchr(s, ':')+1); */
/*	 overwrite	`buttonname', but don't mess with s 95/April/30 */
			}
/* if <mark> comes after (but not before) launch: & or file: & ??? */
			else if (*buttonname == '\0') 
				(void) getbuttonname (buttonname, s); 
			if (bShowButtons != 0) {
#ifdef DEBUGBUTTON
				if (bDebug > 1) OutputDebugString(buttonname);
#endif
				CaptionFontSize = CaptionFontSize / 2;
				ShowNameFile(hDC, dxll, dyll, dxur, dyur, buttonname);
				CaptionFontSize = CaptionFontSize * 2;
			}

			if (bMarkFlag != 0 &&
				tagx >= dxll && tagx <= dxur &&
				tagy >= dyll && tagy <= dyur) {
/*	What to do if we have a hit: */
				buttonlabel[0] = '\0';					/* 95/Aug/12 */
				str[0] = '\0';							/* 95/Aug/12 */
				marknumber = button;
				if (strncmp(s, "file:", 5) == 0 ||
					strncmp(s, "execute:", 8) == 0 ||
					strncmp(s, "launch:", 7) == 0 ||
					strncmp(s, "page:", 5) == 0) {		/* 95/Feb/8 */
					strcpy(str, s);		
#ifdef DEBUGHYPER
					if (bDebug > 1) OutputDebugString(debugstr);
#endif
/*					buttonlabel[0] = '\0';	*//* indicate no button name */
				}	 /* hyper execute added 94/Dec/13 */
				else {
					strcpy(buttonlabel, buttonname);
/*					str[0] = '\0'; */		/* indicate ordinary button push */
				}
				if (*buttonname != '\0')
					strcpy(buttonlabel, buttonname);
				buttonpos = an_wintell(input);
/*				lastsearch = an_wintell(input); */
				buttondvipage = dvipage;	/* EXPERIMENT 95/Mar/10 */
/*				finish = -1; */		/* NO !!! Unless ShowFlag == 0 */
			}
		}
		else if (bComplainSpecial != 0) {
/*			wincancel("Don't understand `button' special"); */
/*			wincancel(line); */
			sprintf(debugstr, "Bad hyper-text \\special: %s", line);
			winbadimage(debugstr);
		}
	}
	else if (strncmp(line, "viewrule:", 9) == 0) { /* 95/Mar/27 */
		if (bPrintFlag == 0 && bCopyFlag == 0 && bViewExtras) {
			if (sscanf(line, "viewrule: %ld %ld%n",
					   &dx, &dy, &m) >= 2) {
/*			xll = dvi_h; yll = dvi_v; xur = xll + dx; yur = yll - dy; */
/*			dxll = mapx(xll); 	dyll = mapy(yll); */
/*			dxur = mapx(xur); 	dyur = mapy(yur); */
				DrawRule(dy, dx);
/*			DrawFigureBox(hDC, dxll, dyll, dxur, dyur); */ /* better rule ? */
/*			if (bDebug > 1) {
			sprintf(debugstr, "viewrule: %d %d %d %d\n", dxll, dyll, dxur, dyur);
			OutputDebugString(debugstr); } */
			}
		}
	}
	else if (strncmp(line, "viewtext:", 9) == 0) { /* 95/Mar/27 */
		if (bShowFlag != 0 && bTextOutFlag != 0) {
			if (bPrintFlag == 0 && bCopyFlag == 0 && bViewExtras) {
				s = line + 9;
				while (*s <= ' ' && *s > 0) s++;
				dvi_hh = mapx(dvi_h); dvi_vv = mapy(dvi_v);   /* ??? */
				if (cp_valid == 0)					/* moveto current pt */
					(void) MoveToEx(hDC, dvi_hh, dvi_vv, NULL);
				(void) ExtTextOut(hDC, 0, 0, 0, NULL, s, strlen(s), NULL);
				cp_valid = 0; 				/* ??? */
/*			if (bDebug > 1) {
				sprintf(debugstr, "viewtext: %s\n", s);
				OutputDebugString(debugstr);
				} */
			}
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* \special{tiff:/mount/mktpub_d1/users/temp_jobs/59625100nl/procroms.tif
   lib=control.glb xmag=1 ymag=1 hleft vhigh  bclip=0.000000 lclip=0.000000
   rclip=0.000000 tclip=0.000000 nostrip */
/* New stuff for HPTAG */ /* 95/Oct/12 */

/* int dohptag(HDC hDC, FILE *input) */
int dohptag(HDC hDC, int input) {
/*	double bclip=0, lclip=0, rclip=0, tclip=0; */
/*	double xmag=1, ymag=1; */
/*	int hleft=1, hright=0, vhigh=1, vlow=0; */
/*	int nostrip=1; */
	char *s, *t;
	char filename[MAXFILENAME];	

/*	(void) scanspecial(input, line, MAXLINE); */
	(void) scanspecial(input, line, sizeof(line));
	if ((s = strchr(line, ' ')) != NULL) *s = '\0';
	if ((t = strrchr(line, '/')) != NULL) strcpy(filename, t+1);
	else if (strlen(line) < sizeof(filename)) strcpy(filename, line);
	else return 0;							/* failure */
/*	now analyze the rest of the line starting at s+1 */
/*	showtiffhere(output, filename, dwidth, dheight, 0, 0, nifd); */
	hptagflag = 1;
	if (bShowTIFF)			/* allow suppression of 97/Jan/5 */
		showtiffhere(hDC, filename, 0, 0, 0, 0, 1);
	return 1;								/* success */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Stuff for reading simple uncompressed TIFF files (& PackBits compressed) */

unsigned int tiffversion;			/* TIFF version number */

unsigned int leastfirst=1;			/*  least significant first */

unsigned int ifdcount;				/* number of items image file directory */

unsigned long ifdposition;			/* position of image file directory */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following moved to end to try and avoid compiler error */

#ifdef LONGNAMES
static unsigned int ureadtwo (HFILE);
static unsigned long ureadfour(HFILE);
#else
static unsigned int ureadtwo (FILE *);
static unsigned long ureadfour(FILE *);
#endif

/* char *typename[6] = {"", "BYTE ", "ASCII", "SHORT", "LONG ", "RATIO"}; */

int typesize[6] = {0, 1, 1, 2, 4, 8};	/* units of length/count */

/* struct tagname {
	unsigned int tag; char *name;
}; */

/* header information from EPSF file */

long psoffset, pslength;
long metaoffset, metalength;
long tiffoffset, tifflength;

/* we are ignoring the possibility here that length > 1 and such ... */

#ifdef LONGNAMES
long indirectvalue(unsigned int type, long length, long offset, HFILE input) {
#else
long indirectvalue(unsigned int type, long length, long offset, FILE *input) {
#endif
	long present, val=0;

	present = ftell(input);			/* remember where we are */
	if (fseek(input, (long) (offset + tiffoffset), SEEK_SET) != 0) {
/*		wincancel("Error in seek to indirect value"); */
		winbadimage("Error in seek to indirect value"); 
		return -1;
	}
	if (type == TYPE_LONG) val = (long) ureadfour(input);
	else if (type == TYPE_SHORT) val = ureadtwo(input);
	else if (type == TYPE_BYTE) val = getc(input);
	else {
/*		wincancel("Invalid Indirect Value"); */
		winbadimage("Invalid Indirect Value"); 
	}
	fseek(input, present, SEEK_SET);	/* return to where we were */
	return val;
}	/* length unreferenced */

/* get value of a tag field in TIFF file */
/* if value fits in 4 bytes or less, use offset itself */

#ifdef LONGNAMES
long extractvalue (unsigned int type, unsigned long length, 
				long offset, HFILE input) {
#else
long extractvalue (unsigned int type, unsigned long length, 
				long offset, FILE *input) {
#endif
	if (length == 0) return 0;
	switch(type) {
		case TYPE_BYTE:
			if (length <= 4) return offset;
			else return indirectvalue(type, (long) length, (long) offset, input);
		case TYPE_SHORT:
			if (length <= 2) return offset;
			else return indirectvalue(type, (long) length, (long) offset, input);
		case TYPE_LONG:
			if (length == 1) return offset;
			else return indirectvalue(type, (long) length, (long) offset, input);
		default:
			return -1;
	}
}

#ifdef LONGNAMES
int skipthisimage (HFILE input, unsigned long ifdpos) {
#else
int skipthisimage (FILE *input, unsigned long ifdpos) {
#endif
	int k, j;
	if (fseek(input, (long) ifdpos + tiffoffset, SEEK_SET) != 0) {
		sprintf(debugstr, "Error in seek to %ld", (long) ifdpos + tiffoffset);
/*		wincancel(str); */
		winbadimage(debugstr);
		return -1;
	}
	ifdcount = ureadtwo(input);			/* How many tags in this IFD */
	for (k = 0; k < (int) ifdcount; k++) {	/* read to end of IFD */
		for (j = 0; j < 12; j++) (void) getc(input);
	}
	ifdposition = ureadfour(input);		/* get next IFD offset in file */
	if (ifdposition == 0) {		
		return -1; 			/*  no more IFDs !!! */
	}
	return 0;
}

/* read the tag fields in the TIFF file, ignore ones we don't care about */

#ifdef LONGNAMES
int readtifffields (HFILE input, unsigned long ifdpos) {
#else
int readtifffields (FILE *input, unsigned long ifdpos) {
#endif
	unsigned int k, tag, type;
	unsigned long length, offset;
	int c;
/*	char *s; */

	if (fseek(input, (long) ifdpos + tiffoffset, SEEK_SET) != 0) {
		sprintf(debugstr, "Error in seek to %ld", (long) ifdpos + tiffoffset);
/*		wincancel(str); */
		winbadimage(debugstr);
		return -1;
	}
	ifdcount = ureadtwo(input);			/* How many tags in this IFD */
	
	ImageWidth = ImageLength = -1;
	SamplesPerPixel = BitsPerSample = 1;
	ExtraSamples = 0;					/* default TIFF 6.0 99/May/10 */
	compression = 0; Orientation = 1;
	StripOffset = -1; StripByteCount = -1;
/*	RowsPerStrip = 0; */
	PhotometricInterpretation = 1;		/* default */
	PlanarConfiguration = 1; Predictor = 1;
	colormap = 0;			/* pointer to map in case of Palette Color Images */
	mingrey = -1; maxgrey = -1;
/*	xresnum = yresnum = 72; */
	xresnum = yresnum = nDefaultTIFFDPI;		/* 96/Apr/3 */	
	xresden = yresden = 1;				/* 93/Oct/20 */
	ResolutionUnit = 2;					/* default dpi */	/* 93/Oct/20 */

	for (k = 0; k < ifdcount; k++) {
		tag = ureadtwo(input);		/* tag - key */
		type = ureadtwo(input);		/* value type */
		if (tag == 0 && type == 0) {	/* invalid */
			c = getc(input); ungetc(c, input);
			sprintf(debugstr, "Tag: %u Type: %u (k %d c %d)\n",
					tag, type, k, c);
			winbadimage(debugstr); 
			break;
		}
		if (type > 5) {
			if (bDebug > 1) {
				c = getc(input); ungetc(c, input);
				sprintf(debugstr, "Tag: %u Type: %u (k %d c %d)\n",
						tag, type, k, c);
				OutputDebugString(debugstr);
			}	/* removed on screen warning 98/Sep/22 */
/*			break; */				/* ignore 98/Sep/22 */
		}
		length = ureadfour(input);	/* count - length */
		if (length == 1) {
			if (type == TYPE_LONG) offset = ureadfour(input);
			else if (type  == TYPE_SHORT) {
				offset = ureadtwo(input);	
				(void) ureadtwo(input);		/* should be zero */
			}
			else if (type == TYPE_BYTE) {
				offset = getc(input);
				(void) getc(input);(void) getc(input);(void) getc(input);
			}
			else offset = ureadfour(input);	/* for ratio e.g. */
		}
		else offset = ureadfour(input);	/* value */

		switch (tag) {
			case IMAGEWIDTH:
				ImageWidth = extractvalue(type, length, (long) offset, input);
				break;
			case IMAGELENGTH:
				ImageLength = extractvalue(type, length, (long) offset, input);
				break;
			case BITSPERSAMPLE:
				BitsPerSample =
					(int) extractvalue(type, length, (long) offset, input);
				break;
			case COMPRESSION:
				compression =
					(unsigned int) extractvalue(type, length, (long) offset, input);
				break;
			case SAMPLESPERPIXEL:
				SamplesPerPixel = extractvalue(type, length, (long) offset, input);
				break;
			case STRIPOFFSETS:
				StripOffset = extractvalue(type, length, (long) offset, input);
				StripOffsetsPtr = offset;
				StripOffsetsType = type;
				StripOffsetsLength = length;
				break;
			case STRIPBYTECOUNTS:
				StripByteCount = extractvalue(type, length, (long) offset, input);
				StripByteCountsPtr = offset;
				StripByteCountsType = type;
				StripByteCountsLength = length;
				break;
			case ROWSPERSTRIP:
				RowsPerStrip = extractvalue(type, length, (long) offset, input);
				break;
			case ORIENTATION:
				Orientation = extractvalue(type, length, (long) offset, input);
				break;
			case PLANARCONFIG:
				PlanarConfiguration = extractvalue(type, length, (long) offset, input);
				break;
			case PREDICTOR:
				Predictor = extractvalue(type, length, (long) offset, input);
				break;
			case EXTRASAMPLES:							/* 1999/May/10 */
				ExtraSamples =
					  (int) extractvalue(type, length, (long) offset, input);
				break;
			case MINSAMPLEVALUE:
				MinSampleValue = extractvalue(type, length, (long) offset, input);
				mingrey = (int) MinSampleValue;
				break;
			case MAXSAMPLEVALUE:
				MaxSampleValue = extractvalue(type, length, (long) offset, input);
				maxgrey = (int) MaxSampleValue;
				break;
			case XRESOLUTION:						/* 93/Oct/20 */
				xresnum = indirectvalue(TYPE_LONG, 1, offset, input);
				xresden = indirectvalue(TYPE_LONG, 1, offset+4, input);
				break;
			case YRESOLUTION:						/* 93/Oct/20 */
				yresnum = indirectvalue(TYPE_LONG, 1, offset, input);
				yresden = indirectvalue(TYPE_LONG, 1, offset+4, input);
				break;
			case RESOLUTIONUNIT:
				ResolutionUnit = (int) extractvalue(type, length, (long) offset, input);
				break;
			case PHOTOMETRICINTERPRETATION:
				PhotometricInterpretation = 
					extractvalue(type, length, (long) offset, input);
				break;
			case COLORMAP:
				colormap = offset;
				colormaptype = type;			/* remember type */
				colormaplength = (int) length;	/* remember length */
/*					extractvalue(type, length, (long) offset, input); */
				break;
			default:							/* ignore unknown tags */
				break;
		}
	}
	return 0;
}

/****************************************************************************
*                                                                           *
*		The EPSF header has the following structure:						*
*                                                                           *
* 0-3	the first four bytes are the letters EPSF with the meta-bit on -	*
*		that is, hex C5D0D3C6.												*
* 4-7	the next four are the byte position of start of the PS section		*
* 8-11	The next four are the length of the PostScript part of file;        *
* 12-15	The next four are the byte position of start of MetaFile version;	*
* 16-19 The next four are the length of the MetaFile version;				*
* 20-23	The next four are the byte position of start of TIFF version;		*
* 24-27	The next four are the length of the TIFF version;					*
* 28-29	The next two bytes are the header checksum or hex FFFF				*
*       that is, two bytes that are all ones (meta-control-? = 255)			*
*                                                                           *
*		IN EACH CASE THE LOW ORDER BYTE IS GIVEN FIRST						*
*                                                                           *
*		Either the MetaFile length or the TIFF length or both are zero		*
*                                                                           *
*		If there is no MetaFile or Tiff version, the PS code starts at 30	*
*                                                                           *
*	The file produced as plain PS by Designer, instead starts and ends      *
*       on control-D.  The first control-D is followed by %<space>,         *
*       which is non-standard, to say the least.                            *
*                                                                           *
****************************************************************************/

/* Try and see whether EPSF file and read header info if so: */
/* fills in tiffoffset and psoffset and metaoffset and lengths */
/* returns zero if not an EPSF file */ /* file position is end of EPSF head */

/* called from checkpcform and showtiffhere */

#ifdef LONGNAMES
int readepsfhead(HFILE special) {
#else
int readepsfhead(FILE *special) {
#endif
	int c, d, e, f;

	psoffset = 0;				/* redundant */
	metaoffset = 0;				/* redundant */
	tiffoffset = 0;				/* redundant */

	c = getc(special); d = getc(special);
	e = getc(special); f = getc(special);
	if (c == 'E' + 128 && d == 'P' + 128 &&
		e == 'S' + 128 && f == 'F' + 128) {
		leastfirst = 1;
		psoffset = (long) ureadfour(special);	/* read PS start offset */ 
		pslength = (long) ureadfour(special);	/* read PS length */
		metaoffset = (long) ureadfour(special);	/* read MF start offset */
		metalength = (long) ureadfour(special);	/* read MF length */
		tiffoffset = (long) ureadfour(special);	/* read TIFF start offset */
		tifflength = (long) ureadfour(special);	/* read TIFF length */
		(void) ureadtwo(special);			/* should be 255, 255 */
		return -1;
	}
	else return 0;							/* not an EPSF file */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

HPALETTE CreateBIPalette(LPBITMAPINFOHEADER);

/* macros to break C "far" pointers into their segment and offset components*/
/* from dos.h */

/*	#define FP_SEG(fp) (*((unsigned _far *)&(fp)+1)) */
/*	#define FP_OFF(fp) (*((unsigned _far *)&(fp))) */

/* if (FP_SEG(s) == FP_SEG(s + InRowLength)) copy using far pointer
   else copy using huge pointer */ /* try this ? */

/* combine r, g, b into palette index - divide row length by 3 */

/* NOTE: RGBQUAD and RGBTRIPLE have byte order: Blue, Green, Red */

/* void compresscolor(char *buffer, unsigned int InRowLength, int lookup[]) */
void compresscolor (unsigned char far *buffer, unsigned int InRowLength, int lookup[]) {
	int r, g, b;
	unsigned int k, OutRowLength;
/*	char *s=buffer, *t=buffer; */
	unsigned char far *s=buffer;
	unsigned char far *t=buffer;

	if (buffer == NULL) {
/*		winerror("NULL buffer in compresscolor"); */ /* debugging */
		return;
	}
	OutRowLength = InRowLength / 3;
	for (k = 0; k < OutRowLength; k++) {
		r = *s++ & 255; g = *s++ & 255; b = *s++ & 255;
		if (bStretchColor) {
			r = lookup[r]; g = lookup[g]; b = lookup[b];
		}

/*		Note: 224 = E0 (use top 3 bits) 192 = C0 (use top 2 bits) */
/*		The 8 bit palette index is built as |rrr|ggg|bb| */
		if (bBGRflag == 0)				/* reversed 92/Feb/21 */
			*t++ = (char) ((r & 224) | ((g & 224) >> 3) | ((b & 192) >> 6));
		else 
			*t++ = (char) ((b & 224) | ((g & 224) >> 3) | ((r & 192) >> 6));
	}	
/*	modified above 92/01/26 */
}

/* Will this work correctly in both Standard and Enhanced Mode ? YES */
/* In standard mode segments are high part of real address */
/* In enhanced mode segments are index into table instead */
/* But this code does no arithmetic on segment part of address */

/* copy bytes from NEAR buffer to HUGE image array */
/* attempt to be efficient about it */
/* split up copy operation if crosses segment boundary */

/*	IN FACT in WINVER >=0x03a, WE CAN USE _hread TO READ THE WHOLE THING */

/* should be easier - no need for breaking up into chunck then ? */

#ifndef USEMEMCPY
void copynearimage (unsigned char *to, unsigned char *from, unsigned long count) {
/*	strncpy(to, from, count); */
#ifdef USEMEMCPY
	memcpy(to, from, count);
#else
	while (count-- > 0) *to++ = *from++;
#endif
}
#endif

/* copy bytes from FAR buffer to HUGE image array */
/* attempt to be efficient about it */
/* split up copy operation if crosses segment boundary */

/*	IN FACT in WINVER >=0x03a, WE CAN USE _hread TO READ THE WHOLE THING */

/* should be easier - no need for breaking up into chunck then */

#ifndef USEMEMCPY
void copyfarimage(unsigned char *to, unsigned char *from, unsigned long count) {
/*	strncpy(to, from, count); */
#ifdef USEMEMCPY
	memcpy(to, from, count);
#else
	while (count-- > 0) *to++ = *from++;
#endif
}
#endif

/* This opens the file and makes sure it is closed before exiting */
/* should perhaps have helper function that does not fclose ... */

/* called from `showtiffhere' */ /* readflag if only scanning for w * h */

int readimagefile (HDC hDC, char *filename, 
		int xll, int yll, int xur, int yur, int nifd, int readflag) {
	char infilename[MAXFILENAME];
/*	char epsfilename[MAXFILENAME]; 	*/	/* 95/Apr/ 26 */
/*	FILE *input=NULL; */ /* changed to => special */
	long present;
	int c, d; 
	char *s;

	wmfflag=0;
	bmpflag=0;

	strcpy(infilename, filename);
/*	extension(infilename, "tif"); */	/* use `tif' if no extension given */
	
/*	if ((special = findepsfile(infilename, -1, "tif")) == NULL) {  */
	special = findepsfile(infilename, -1, "tif");
	if (special == BAD_FILE) {  
/*if ((special = findepsfile(infilename, -1, "tif", epsfilename)) == NULL) {*/
/*		sprintf(str, "Image file %s not found", t); 
		wincancel(str); */			/* already complained about */
		return -1;					/* failed */
	}

	tiffoffset = 0;				/* normally beginning of file */
	psoffset = 0;
	metaoffset = 0;

/*	First see whether this perhaps is an EPSF file with (TIFF) preview */
	c = getc(special);
	(void) ungetc(c, special);
	if (c > 128) {				/* see whether perhaps EPSF file */
		if (readepsfhead(special) != 0) {			/* is it EPSF file ? */
/*			If valid EPSF header with pointer to TIFF we get here */
			if (tiffoffset != 0 && tifflength != 0) {	/* 95/Oct/1 */
				if (fseek(special, tiffoffset, SEEK_SET) != 0)  {
					if (bDebug > 1) {
						sprintf(debugstr, "Error in seek to %ld in %s", 
								tiffoffset, filename);
						OutputDebugString(debugstr);	/* 94/Oct/14 */
					}
					fclose(special);
/*					special = NULL; */
					special = BAD_FILE;
					return -1;
				}
			}
/*			If valid EPSF header with pointer to WMF we get here */
			else if (metaoffset != 0 && metalength != 0) {	/* 95/Oct/1 */
				if (fseek(special, metaoffset, SEEK_SET) != 0)  {
					if (bDebug > 1) {
						sprintf(debugstr, "Error in seek to %ld in %s", 
								metaoffset, filename);
						OutputDebugString(debugstr);	/* 94/Oct/14 */
					}
					fclose(special);
/*					special = NULL; */
					special = BAD_FILE;
					return -1;
				}
			}
			else {										/* 95/Oct/1 */
				if (bDebug > 1) {
					sprintf(debugstr, "Zero TIFF or WMF offset or length in %s",
						filename);
					OutputDebugString(debugstr);	/* 94/Oct/14 */
				}
				fclose(special);
/*				special = NULL; */
				special = BAD_FILE;
				return -1;
			}
/*			if seek to TIFF or WMF preview *succeeds* we drop through here */
		}						/* end of readepsfhead (special) != 0 */
		else {
/*			sprintf(str, "`%s' not a valid EPSF or TIFF file", filename); */
			strcpy(debugstr, "Not a valid ");
			strcat(debugstr, "EPSF or TIFF file: ");
			strcat(debugstr, filename);
			winbadimage(debugstr);
			fclose(special);
/*			special = NULL; */
			special = BAD_FILE;
			return -1;
		}
	}							/* end of c > 128 (EPSF file) */

/*	Next, try and deal with PostScript ASCII stuff EPSI preview maybe ? */
/*	Or may meet TIFF / WMF preamble */ 
	c = getc(special);
/*	Ignore white space at the start of the file 98/Jul/29 */
	while (c <= ' ' && c > 1) c = getc(special);
	d = getc(special);
/*	if (c == '%' && d == '!') {	*/	/* ordinary PS file ? */
	if (c == '%') {					/* less strict 96/Sep/15 */
/*		see whether file has EPSI preview ... */ /* 92/Jan/2 */
		(void) getline(special, line);	/* skip over first line */	
		(void) getline(special, line);	/* grap next comment line */
/*		winerror("Checking whether EPSI"); */		/* debugging */
/*		search %% DSC comments at head of file for %%Preview */
		while (*line == '%' && *(line+1) == '%') {  /* was 94/May/19 */
			if (strncmp(line+2, "BeginPreview:", 13) == 0) {
/*				winerror("Yes it's EPSI"); */		/* debugging */
				if (sscanf(line+2+13, "%d %d %d %d", 
					&prewidth, &preheight, &prebits, &prelines) >= 3) {
/* OK, we have an EPSI preview here */ /* do something with it ! */
/*					sprintf(str, "w %d h %d b %d l %d", 
						prewidth, preheight, prebits, prelines);
					wincancel(str); */						/* debugging */
/*					winerror("Going to read EPSI"); */		/* debugging */
/* shouldn't really every get here if bShowPreview is zero - so flush ? */
					if (bShowPreview != 0 || bPrintFlag != 0 
							|| bCopyFlag != 0) {	/* 93/Mar/30 */
						(void) readepsipreview(hDC, special, 
							xll, yll, xur, yur, readflag);
						fclose(special);
/*						special = NULL; */
						special = BAD_FILE;
						return 0;				/* done with EPS preview */
					}
					else break;					/* 93/Mar/30 what ??? */
				}
			}
			if (getline(special, line) == 0) break;
/*			added following to skip blank line in Mathematica EPSI */
			while (*line == '\n' || *line == '\r')	/* 94/May/19 */
				if (getline(special, line) == 0) break;
		}
		fclose(special);
/*		special = NULL; */
		special = BAD_FILE;

/*		It was a PS file, but without a preview in TIFF or EPSI form */
/*		return -1; */
		if (!bForceTIFF) return -1;						/* just give up */

/*		Forced search for TIFF file with same name 95/Sep/7 */
		strcpy(infilename, filename);
/*		If extension was `eps' strip it and replace with `tif' and re-search */
		if ((s = strrchr(infilename, '.')) != NULL &&
			_stricmp(s+1, "eps") == 0) {
			*s = '\0';
/*			if ((special = findepsfile(infilename, -1, "tif")) == NULL) */
/*			if ((special = findepsfile(infilename, 0, "tif")) == NULL) */
			special = findepsfile(infilename, 0, "tif");
			if (special == BAD_FILE) return -1;				/* failed */
			c = getc(special); d = getc(special);
/*		drop through if we have opened what appears to be TIFF file */
		}
		else return -1;		/* it was a PS file, but without a preview */
	}

/*	else if (c == 'I' && d == 'I') leastfirst = 1;	*/	/* 1995/Sep/7 */
/*  Now check whether TIFF file (in Intel or Motorola format) */
	if (c == 'I' && d == 'I') leastfirst = 1;	/* PC style TIFF file */
	else if (c == 'M' && d == 'M') leastfirst = 0;	/* Mac style TIFF file */
	else if (c == 1 && d == 0) wmfflag = 1;		/* wmf, not placeable */
	else if (c == 'B' && d == 'M') bmpflag = 1;	/* BMP 98/May/28 */
	else {
		fclose(special);
/*		special = NULL; */
		special = BAD_FILE;
/*		sprintf(str, "`%s' not a valid EPSF or TIFF file", filename); */
/*		sprintf(str, "`%s' not a valid EPSF, TIFF, or WMF file", filename); */
		strcpy(debugstr, "Not a valid "); 
		strcat(debugstr, "EPSF, TIFF, BMP or WMF file: ");
		strcat(debugstr, filename);
		s = debugstr + strlen(debugstr);	/* add debugging info 95/Dec/1 */
		sprintf(s, " %c%c (%d %d) ", c, d, c, d);
/*		sprintf(debugstr, 
"TIFF file %s should start with `II' or `MM' not `%c%c'", filename, c, d); */
		winerror(debugstr);
		present = ftell(special);
/*		sprintf(debugstr, "at byte %ld with tiffoffset %ld",
				present, tiffoffset); */
		sprintf(debugstr, "at byte %ld, tiffoffset %ld, metaoffset %ld",
				present, tiffoffset, metaoffset);
		winbadimage(debugstr);
		return -1;		/* not a TIFF or WMF subfile !!! */
	}

/*	Now have decided that this is a TIFF file (or TIFF or WMF preview) */
	if (bShowPreview != 0 || bPrintFlag != 0 || bCopyFlag != 0) {
		if (wmfflag) readwmffile(hDC, special, xll, yll, xur, yur);
		else if (bmpflag) readbmpfile(hDC, special, xll, yll, xur, yur, readflag);
		else readtifffile(hDC, special, xll, yll, xur, yur, nifd, readflag);
	}
	fclose(special);	/* 92/Feb/13 */ /* moved here rather than scattered */
/*	special = NULL; */
	special = BAD_FILE;

	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for extracting hex-bytes in EPSI preview */
/* speed up using lookup table ? */

#ifdef LONGNAMES
int readhexbyte (HFILE input) {
#else
int readhexbyte (FILE *input) {
#endif
	int c, d;

#ifdef EPSISPEED
	while((c = wingetc(input)) < '0') if (c == EOF) return -1;
#else
	while((c = getc(input)) < '0') if (c == EOF) return -1; 
#endif
	if (c <= '9' && c >= '0') c = c - '0';
	else if (c <= 'F' && c >= 'A') c = c - 'A' + 10;
	else if (c <= 'f' && c >= 'a') c = c - 'a' + 10;
/*	else if (c == EOF) return -1; */
	else {	/* not a hexadecimal character */
/*		wincancel("Bad EPSI preview");	return -1; */
	} 
#ifdef EPSISPEED
	while((d = wingetc(input)) < '0') if (c == EOF) return -1;
#else
	while((d = getc(input)) < '0') if (c == EOF) return -1; 
#endif
	if (d <= '9' && d >= '0') d = d - '0';
	else if (d <= 'F' && d >= 'A') d = d - 'A' + 10;
	else if (d <= 'f' && d >= 'a') d = d - 'a' + 10;		
/*	else if (d == EOF) 	return -1; */
	else {	/* not a hexadecimal character */
/*		wincancel("Bad EPSI preview");	return -1; */
	} 
	return (c << 4) | d;
}

#ifdef LONGNAMES
int	readepsipreview (HDC hDC, HFILE input, int xll, int yll, int xur, int yur,
			int readflag) {
#else
int	readepsipreview (HDC hDC, FILE *input, int xll, int yll, int xur, int yur,
			int readflag) {
#endif
	int n;
	int i, j, flag; 
	unsigned char huge *s;				/* HUGE to access global image array */
	char far *t;				/* FAR to access global image array */
	unsigned char *u;					/* source for transfer to global image */
/*	HANDLE hBuffer=NULL; */		/* Flushed 95/July/27 */
	unsigned char *lpBuffer=NULL;		/* Buffer for a row, locally allocated */
	unsigned long imagesize;

/*	shouldn't get here if bShowPreview == 0 */

	if (special == BAD_FILE) return -1;		/* sanity check */

	ImageWidth = prewidth; ImageLength = preheight;
	SamplesPerPixel = 1; BitsPerSample = prebits,
	ExtraSamples = 0;						/* 99/May/10 */
	compression = 0; Orientation = 1; colormap = 0;
	if (BitsPerSample == 1)
		PhotometricInterpretation = 0;	/* NOT default - flipped for EPSI */
	else PhotometricInterpretation = 1;	/* default */	

	if (ExtraSamples > 0) SamplesPerPixelX = SamplesPerPixel-ExtraSamples;
	else SamplesPerPixelX = SamplesPerPixel;

	BitsPerPixel = BitsPerSample * SamplesPerPixel;
	if (ExtraSamples > 0) BitsPerPixelX = BitsPerSample * SamplesPerPixelX;
	else BitsPerPixelX = BitsPerPixel;

	if (RowsPerStrip > 0)				/* 1996/Sep/7 */
		StripsPerImage = (int) ((ImageLength + RowsPerStrip - 1) / RowsPerStrip);
	else StripsPerImage = 1;

	if (readflag == 0) {	/* if only scanning for width & height */
/*		fclose(input); */	/* done after call anyway */
		return -1;
	}

	mingrey = 0; maxgrey = (1 << (int) BitsPerSample) - 1;

/*  NOTE: (Bitmap) monochrome bitmap rows are int aligned (page 7-6) */
/*	NOTE: (DIB) color bitmap rows are long aligned (page 7-9, 7-11) */

	InRowLength = (ImageWidth * BitsPerPixel + 7) / 8;	/* row length in file */
	if (ExtraSamples > 0)InRowLengthX = (ImageWidth * BitsPerPixelX + 7) / 8;
	else InRowLengthX = InRowLength;

/*	OutRowLength = InRowLength;	*/
	OutRowLength = InRowLengthX;					/* possibly less */

/*	if (BitsPerPixel == 1)
		BytesPerRow = 4 * ((ImageWidth * BitsPerPixel + 31) / 32);
	else 
		BytesPerRow = 4 * ((ImageWidth * BitsPerPixel + 31) / 32); */

/*	BytesPerRow is *after* any removeal of ExtraSamples */
	BytesPerRow = 4 * ((ImageWidth * BitsPerPixelX + 31) / 32);	/* row length in BitMap data */

/*	if (ExtraSamples > 0) {
		BytesPerRow = 4 * ((ImageWidth * BitsPerSample * (SamplesPerPixel-ExtraSamples)
							+ 31) / 32);
	} */
/*	if (BitsPerPixel == 24 && bCompressColor != 0) {	  
		OutRowLength = OutRowLength / 3;	
		BytesPerRow = BytesPerRow / 3;
	} */

/*	sprintf(str, "inrow %ld outrow %ld BytesPerRow %ld", 
		InRowLength, OutRowLength, BytesPerRow);
	winerror(str); */						/* debugging */
	imagesize = (unsigned long) BytesPerRow * ImageLength;
	if (ImageWidth > MAXIMAGEDIMEN || ImageLength > MAXIMAGEDIMEN ||	/* too large - bad data */
		imagesize > MAXIMAGESIZE) {	/* arbitrary limits (to catch bad files) */
		sprintf(debugstr, "TIFF file too large\n(%ld x %ld (%ld) => %ld bytes)", 
				ImageWidth, ImageLength, BitsPerPixel, imagesize);
		winbadimage(debugstr);
/*		fclose(input); */
		return -1;
	}
/*	allocate global array to store image in */
/*	sprintf(str, "imagesize %lu", imagesize); 
	winerror(str); */ /* debug */
	if (imagesize == 0) {
/*		wincancel("Zero image size"); */
		sprintf(debugstr, "ERROR: Zero image size %ld x %ld (%s)",
				BytesPerRow, ImageLength, "readepsipreview");
		winbadimage(debugstr);
/*		fclose(input); */
		return -1;
	}
	hImage = GlobalAlloc(GMEM_MOVEABLE, imagesize);
	lpImageBytes = (LPSTR) GlobalLock(hImage);

/*	With fixed memory should be able to use LocalAlloc result directly! */
/*	if ((hBuffer = LocalAlloc(LMEM_FIXED, (WORD) InRowLength)) == NULL ||
		(lpBuffer = LocalLock(hBuffer)) == NULL) {
		sprintf(str, "Image memory alloc error (%ld bytes)", InRowLength);
		wincancel(str);
		return -1;
	} */

	lpBuffer = (unsigned char *) LocalAlloc(LMEM_FIXED, (UINT) InRowLength);
    if (lpBuffer == NULL) {
		sprintf(debugstr, "Image memory alloc error (%ld bytes)", InRowLength);
/*		wincancel(str); */
		winbadimage(debugstr);
		return -1;
	} 

/*	winerror("Allocated Memory"); */

#ifdef EPSISPEED
/*	wininit(input); */				/* set up buffering stuff */
	if (wininit(input) != 0) {		/* 1996/May/12 */
/*		failed to allocate EPSIBUFFERLEN bytes */
		winbadimage("EPSI buffer alloc error");
		return -1;
	}
#endif

	flip = 1; 						/*	DIB Bitmaps are reversed */
/*	monochrome (CreateBitMap) - is reversed from color (CreateDIBitmap) */
/*	if (BitsPerPixel == 1) flip = 1 - flip; */ /* use CreateDIBitmap now */
/*	else if (Orientation == 4) flip = 1 - flip; */	/* not for pmjean ... */
/*	need to flip when printing to PostScript printer driver */
/*	printer output is reversed from screen output */
/*	if (bPrintFlag != 0) {
		if (IsItPSCRPT(hDC) != 0) flip = 1 - flip; 	
	} */

	flag = 0;								/* flag gets set if EOF hit */
	for (i = 0; i < (int) ImageLength; i++) {	/* read image lines */
		if (flip == 0) s = (unsigned char huge *) lpImageBytes + (BytesPerRow * i);
		else s = (unsigned char huge *) lpImageBytes + (BytesPerRow * ((ImageLength-1) - i));
		t = (char far *) s;
/*		speed up later using buffered reads */
		if (imagesize < 65536L) {
			for (j = 0; j < (int) InRowLength; j++) {
				if ((n = readhexbyte(input)) < 0) {
					flag = -1;			/* hit EOF */
					break;
				}
/*				*u++ = (char) ((c << 4) | d); */	/* NOT INITIALIZED ! */
				else *t++ = (char) n;		/* speed up later */
			}
		}
		else {
			u = lpBuffer;
			for (j = 0; j < (int) InRowLength; j++) {
				if ((n = readhexbyte(input)) < 0) {
					flag = -1;			/* hit EOF */
					break;
				}
				else *u++ = (char) n;
/*				else *s++ = (char) n; */		/* speed up later */
			}
#ifdef USEMEMCPY
			memcpy(s, lpBuffer, InRowLength);
#else
			copynearimage(s, lpBuffer, InRowLength);			/* NEW */
#endif
		}
/*		Remove ExtraSamples in the above, if any ??? */
		if (bEnableTermination != 0 && checkuser() != 0) {  
#ifdef DEBUGTIFF
			if (bDebug > 1) OutputDebugString("checkuser abort\n");
#endif
			bShowFlag = 0;		/* turn off displaying */
			finish = -1;		/* redundant ??? */
			bUserAbort = 1;
			flag = 1;
			break;
		}
	}
	
/*	if (hBuffer != NULL) {
		(void) LocalUnlock(hBuffer);
		hBuffer = LocalFree (hBuffer);
	} */		/* 95/July/27 */
	if (lpBuffer != NULL)
		lpBuffer = (unsigned char *) LocalFree ((HLOCAL) lpBuffer);

	if (colormap == 0) {
/*		fclose(input);	 */
/*		input = NULL; */
	}
	
/*	winerror("Finished reading image"); */		/* debugging */

/*	if (flag != 0) {
		fclose(input); input = NULL;
		return -1;		NO !
	} */

	if (bUserAbort == 0) {	
#ifdef DEBUGTIFF
		if (bDebug > 1) {
			sprintf(debugstr, "ReadEPSIPreview: xll %d yll %d xur %d yur %d",
					xll, yll, xur, yur);
			OutputDebugString(debugstr);	
		}  /* debugging 95/April/13 */
#endif
		(void) renderimage(hDC, input, xll, yll, xur, yur);
	}
	else {
		if (bDebug > 1) OutputDebugString("renderimage abort\n");
	}

/*	if (colormap != 0 && input != NULL) */
	if (colormap != 0 && input != BAD_FILE) {
/*		fclose(input);	 */
/*		input = BAD_FILE; */
	}

	if (hImage != NULL) {
		if (GlobalUnlock(hImage) > 0) {
			sprintf(debugstr, "Lock count Not Zero %s", "Image");
			wincancel(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
		hImage = GlobalFree(hImage);
/*		hImage = NULL; */				/* for debugging */
	}	
#ifdef EPSISPEED
	winendit();			/* clear up buffering stuff activated 95/Dec/4 */
#endif

	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void flipRB (unsigned char *lpLineBuff, unsigned int n) {	// separated 2000 May 27
	unsigned char *u, *v;
	int unsigned i;
	int r, g, b;

	u = lpLineBuff; v = lpLineBuff;
	for (i = 0; i < n / 3; i++) {
		r = *u++; g = *u++; b = *u++;
		*v++ = (char) b; *v++ = (char) g; *v++ = (char) r;
	}
}

// Approximate transformation from CMYK to RGB
// Note heuristic adjustement to K component subtracted out ...

void CMYKtoRGB (unsigned char *lpLineBuff, unsigned int n) {	// 2000 May 27
	unsigned char *u, *v;
	int unsigned i;
	int c, y, m, k, r, g, b;

	u = lpLineBuff; v = lpLineBuff;
	for (i = 0; i < n / 4; i++) {
		c = *u++; m = *u++; y = *u++; k = *u++;
//		r = 255 - c - k;
		r = 255 - c - k/2;
//		g = 255 - m - k;
		g = 255 - m - k/2;
//		b = 255 - y - k;
		b = 255 - y - k/2;
		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;
		*v++ = (char) b; *v++ = (char) g; *v++ = (char) r; *v++ = (char) 0;
	}
}

int nFirstTime=0;

/* get here if asked to display TIFF image and it is found to be compressed */
/* first close file again ! */
/* nImage contains nfid (or nfid - 1 ?) */
/* lpszFileName contains TIFF file name */

/* Can use explicit Link-Time Import page 20-33 */

/* Line callback function for loading up TIFF image */
/* assumes global access to flip, InRowLength, lpImageBytes, BytesPerRow */
/* parameters passed are: */
/* pointer to line buffer, line number (zero based), parameter from caller */

/* int CALLBACK CopyLineFun(LPSTR lpLineBuff, int nLine, LONG lParam) */
int CALLBACK _export CopyLineFun (unsigned char *lpLineBuff, int nLine, LONG lParam) {
	unsigned char huge *s;				/* HUGE to access global image array */
//	unsigned char *u, *v;
//	int k, r, g, b;

/*	UNUSED (lParam); */

//	if (nFirstTime++ == 0) {
//		sprintf(debugstr, "BitsPerPixelX %d bCMYK %d bCompressColor %d InRowLengthX %d",
//				BitsPerPixelX, bCMYK, bCompressColor, InRowLengthX);
//		wincancel(debugstr);				// debugging only
//	}

	if (BitsPerPixelX == 32 && bCMYK) {
		CMYKtoRGB(lpLineBuff, (unsigned int) InRowLengthX);
	}

	if (BitsPerPixelX == 24 && bCompressColor) { 
		compresscolor(lpLineBuff, (unsigned int) InRowLengthX, lookup);
	}
/*	Following added 96/Sep/15 to support bCompressColor == 0 */
	if (BitsPerPixelX == 24 && bCompressColor == 0 && bBGRflag == 0) {
		flipRB(lpLineBuff, InRowLength);
	}

/*	copy the image line across */
	if (flip == 0) 	s = (unsigned char huge *) lpImageBytes + (BytesPerRow * nLine);
	else s = (unsigned char huge *) lpImageBytes + (BytesPerRow * ((ImageLength-1) - nLine));
/*	maybe do directly far => far instead if image is small ? */
#ifdef USEMEMCPY
	memcpy(s, lpLineBuff, OutRowLength);			/* 99/June/25 */
#else
	copyfarimage(s, lpLineBuff, OutRowLength);		/* 92/May/12 */
#endif
	return 0;				/* or error status */
}	/* lParam unreferenced */

/* above must be exported in EXPORTS section of .DEF file */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Call format modelled somewhat on DecodeTiffImage in Black Ice */
int DecompressTiff(HFILE, int, WORD, WORD, LONG, FARPROC);

/* This call CopyLineFun with data for each line */
/* int CALLBACK _export CopyLineFun(LPSTR lpLineBuff, int nLine, LONG lParam); */
/* Assumes global access to flip, InRowLength, lpImageBytes, BytesPerRow */

/* returns -1 if error, returns 0 if success */

int UseOurTIFF (int nImage) {			/* do our own 1996/Sep/2 */
	HFILE hFile;
	WORD wFirstStrip, wNumLines;
	LONG LineFunParam;
	int flag=0;

#ifdef DEBUGTIFF
	OutputDebugString("UseOurTIFF");
#endif

	hFile = _lopen(FileName, READ);
	if (hFile == HFILE_ERROR) {
#ifdef DEBUGTIFF
		if (bDebug > 1) {
			sprintf(debugstr, "Unable to open %s", FileName);
			OutputDebugString(debugstr);
		}
#endif
		return -1;					/* failed */
	}
/*	read TIFF TAGS for ImageWidth, ImageLength, BitsPerSample, SamplesPerPixel */
/*	let's assume this has been done already */
	wFirstStrip = 0;		/* start at first strip */
	wNumLines = 0;			/* decode all lines */
	LineFunParam = 0;		/* or whatever is to be passed */
#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr,
			"ImageWidth %ld ImageLength %ld BitsPerSample %ld SamplesPerPixel %ld",
			ImageWidth, ImageLength, BitsPerSample, SamplesPerPixel);
		OutputDebugString(debugstr);
	}
#endif
	flag = 0;
/*	call into wintiff.c */
	if (DecompressTiff(hFile, nImage, wFirstStrip, wNumLines, LineFunParam,
				   (FARPROC) CopyLineFun) == 0) {
#ifdef DEBUGTIFF
		if (bDebug > 1) OutputDebugString("DecompressTiff failed");
#endif
		flag = -1;				/* failure */
	}
	_lclose(hFile);
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Throw out ExtraSamples in SamplesPerPixel */
/* IMPORTANT NOTE: assume for the moment samples are one byte */

int RemoveExtraSamples (unsigned char *lpBuffer, int InRowLength) {
	unsigned char *s = lpBuffer;
	unsigned char *t = lpBuffer;
	int i, k, n = InRowLength / SamplesPerPixel;

	if (ExtraSamples == 0) return InRowLength;
/*	check that BitsPerSample == 8 ??? */	
	for (k = 0; k < n; k++) {
		for (i = 0; i < (SamplesPerPixel-ExtraSamples); i++) *t++ = *s++;
		s += ExtraSamples;
	}
	return (int) (t - lpBuffer);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long nImageBytes;

#ifdef LONGNAMES
int readtifffile (HDC hDC, HFILE input, 
		int xll, int yll, int xur, int yur, int nifd, int readflag) {
#else
int readtifffile (HDC hDC, FILE *input, 
		int xll, int yll, int xur, int yur, int nifd, int readflag) {
#endif
	int c, d; 
	int e; 
	int i, j, flag;
	long present; 
	int k, color, n, total;
	unsigned char huge *s;				/* HUGE to access global image array */
	unsigned char far *t;				/* FAR to access global image array */
	unsigned char *u; 					/* source for transfer to global image */
/*	HANDLE hBuffer=NULL; */		/* flushed 95/July/27 */
	unsigned char *lpBuffer=NULL;		/* image row buffer locally allocated */
/*	unsigned long imagesize; */
	unsigned int nread;
/*	HANDLE err; */
	int ndif;

/*	int lookup[256]; */			/* remapping table for color stretch */

	tiffversion = ureadtwo(input);
	if (tiffversion != TIFF_VERSION) {
/*		wincancel("Incorrect TIFF version code"); */
		winbadimage("Incorrect TIFF version code");
/*		fclose(input); */
		return -1;		/* bad version number for TIFF file */
	}
	
	ifdposition = ureadfour(input);		/* get first IFD offset in file */
	while (nifd-- > 1) {
		if (skipthisimage(input, ifdposition) < 0) {
/*			wincancel("Subimage not found"); */
			winbadimage("Subimage not found");
/*			fclose(input); */
			return -1;
		}
	}

	(void) readtifffields(input, ifdposition);	/* read tag fields in TIFF file */

	ifdposition = ureadfour(input);		/* get next IFD offset in file */
/*	if (ifdposition != 0) {	} */ /*  more than one IFD */

	if (readflag == 0) {			/* if only scanning for width & height */
/*		fclose(input); */
		return -1;
	}

/*	sprintf(str, 
		"Width %ld, Height %ld, BitsPerSample %ld, SamplesPerPixel %ld", 
		ImageWidth, ImageLength, BitsPerSample, SamplesPerPixel); 
	winerror(str); */						/* debugging */

/*	if (mingrey < 0 || maxgrey < 0 || mingrey == maxgrey) {
		mingrey = 0; maxgrey = (1 << (int) BitsPerSample) - 1;
	} */

	if (PhotometricInterpretation == 5) bCMYK = 1;		// 2000 May 27
	else bCMYK = 0;

	BitsPerPixel = BitsPerSample * SamplesPerPixel;

	if (ExtraSamples > 0) SamplesPerPixelX = SamplesPerPixel-ExtraSamples;
	else SamplesPerPixelX = SamplesPerPixel;

	if (ExtraSamples > 0) BitsPerPixelX = BitsPerSample * SamplesPerPixelX;
	else BitsPerPixelX = BitsPerPixel;

/*  NOTE: monochrome bitmap rows are int aligned (page 7-6) */
/*	NOTE: color bitmap rows are long aligned (page 7-9, 7-11) */

	InRowLength = (ImageWidth * BitsPerPixel + 7) / 8;	/* row length in file */
	if (ExtraSamples > 0)InRowLengthX = (ImageWidth * BitsPerPixelX + 7) / 8;
	else InRowLengthX = InRowLength;
	
/*	OutRowLength = InRowLength;	 */
	OutRowLength = InRowLengthX;					/* possibly less */

/*	BytesPerRow = InRowLength;	*/					/* unaligned so far ! */

	if (ExtraSamples > 0) BytesPerRow = InRowLengthX;
	else BytesPerRow = InRowLength;					/* in or out ??? */

#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr, "InRowLength %d OutRowLength %d BytesPerRow %d",
				InRowLength, OutRowLength, BytesPerRow);
		OutputDebugString(debugstr);
	}
#endif
/*	If we do compression of 24 bit color */
/*	if (BitsPerPixel == 24 && bCompressColor != 0 && forceice == 0) */
/*	if (BitsPerPixel == 24 && bCompressColor != 0) */
	if (BitsPerPixelX == 24 && bCompressColor) {
		OutRowLength = OutRowLength / 3;
		BytesPerRow = BytesPerRow / 3;
	}												/* moved here 92/Feb/22 */
/*	if (BitsPerPixel == 1) */					/* row length in BitMap data */
	if (BitsPerPixelX == 1)					/* row length in BitMap data */
/*		BytesPerRow = 2 * ((BytesPerRow + 1) / 2);	*//* word aligned */ 
		BytesPerRow = 4 * ((BytesPerRow + 3) / 4);	/* 92/April/11 */
	else
/*		BytesPerRow = 4 * ((ImageWidth * BitsPerPixel + 31) / 32); */
		BytesPerRow = 4 * ((BytesPerRow + 3) / 4);	/* dword aligned */

#ifdef DEBUGTIFF
	if (bDebug > 1) {	
		sprintf(debugstr, "ImageWidth %ld ImageLength %ld BitsPerPixel %ld\n",
			ImageWidth, ImageLength, BitsPerPixel);
		OutputDebugString(debugstr);
		sprintf(debugstr,
			"InRowLength %ld OutRowLength %ld BytesPerRow %ld StripByteCount %ld\n",
				InRowLength, OutRowLength, BytesPerRow, StripByteCount);
		OutputDebugString(debugstr);		
	}  		/* 94/Dec/16 */
#endif
/*	deal with GhostScript TIFF file format with gaps */		/* 94/Dec/16 */
	if (bGhostHackFlag) {				/*	made conditional 95/Nov/10 */
		if (InRowLength + 1 == StripByteCount) InRowLength++;
	}
/*	killed again 97/June/23 do avoid bugs in PackBits */

	if (compression > 1 && compression != PACK_BITS) { 
/*		winbadimage("Compressed TIFF files (other than PackBits) not supported"); */
/*		return -1; */	/* flushed 1996/Sep/2 */
	}
	
	nImageBytes = (unsigned long) BytesPerRow * ImageLength;
/*	check whether values reasonable */	/* if `too large' - bad data likely */
	if (ImageWidth > MAXIMAGEDIMEN || ImageLength > MAXIMAGEDIMEN ||	
		nImageBytes > MAXIMAGESIZE) {	/* arbitrary limits (to catch bad files) */
		sprintf(debugstr, "TIFF file too large\n(%ld x %ld (%ld) => %ld bytes)", 
				ImageWidth, ImageLength, BitsPerPixel, nImageBytes);
		winbadimage(debugstr);
		return -1;
	}
	if (ImageWidth < 0 || ImageLength < 0 ||		/* missing fields */			
		StripOffset < 0) {					/* missing fields */
		winbadimage("TIFF file missing required tags");
		return -1;
	}		

/*	{
		sprintf(debugstr, "StripOffset %d tiffoffset %d", StripOffset, tiffoffset);
		wincancel(debugstr);
	} */

	if (fseek(input, (long) StripOffset + tiffoffset, SEEK_SET) != 0)  {
		sprintf(debugstr, "Error in seek to %ld in %s", 
				StripOffset + tiffoffset, FileName);
		winbadimage(debugstr);
		return -1;
	}

#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr, "BitsPerSample %ld, BytesPerRow %ld", 
				BitsPerSample, BytesPerRow);
		OutputDebugString(debugstr);
	}
#endif

/*	allocate global array to store image in */
	nImageBytes = (unsigned long) BytesPerRow * ImageLength;	/* redundant */
#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr, "nImageBytes %ld BytesPerRow %d ImageLength %d",
				nImageBytes, BytesPerRow, ImageLength);
		OutputDebugString(debugstr);
	}
#endif
	if (nImageBytes == 0) {
/*		wincancel("Zero image size"); */
		sprintf(debugstr, "ERROR: Zero image size %ld x %ld (%s)",
				BytesPerRow, ImageLength, "readtifffile");
		winbadimage(debugstr);
/*		fclose(input); */
		return -1;
	}
	hImage = GlobalAlloc(GMEM_MOVEABLE, nImageBytes);
	lpImageBytes = (LPSTR) GlobalLock(hImage);

/*	With fixed memory should be able to use LocalAlloc result directly */

/*	if ((hBuffer = LocalAlloc(LMEM_FIXED, (WORD) InRowLength)) == NULL ||
		(lpBuffer = LocalLock(hBuffer)) == NULL) {
		sprintf(str, "Image memory alloc error (%ld bytes)", InRowLength);
		wincancel(str);
		return -1;
	} */

	if ((lpBuffer = (unsigned char *) LocalAlloc(LMEM_FIXED, (UINT) InRowLength))
		== NULL) {
		sprintf(debugstr, "Image memory alloc error (%ld bytes)", InRowLength);
/*		wincancel(str); */
		winbadimage(debugstr);
		return -1;
	} 

/*	scan the file and get the maximum color component - if requested */
	if (BitsPerPixelX == 24 && bStretchColor && bCompressColor) {
		if (maxgrey < 0) {
			present = ftell(input);			/* remember where we were */
			maxgrey = -1;
			for (i = 0; i < (int) ImageLength; i++) {	/* read image lines */
#ifdef LONGNAMES
				nread = _lread(input, lpBuffer, (unsigned int) InRowLength);
#else
				nread = fread(lpBuffer, 1, (unsigned int) InRowLength, input);
#endif
/*				if (nread != (unsigned int) InRowLength) {
					wincancel("Image file read failed");
				} */
				u = lpBuffer;
				for (j = 0; j < (int) InRowLength; j++) {
					if ((color = (*u++ & 255)) > maxgrey) maxgrey = color;
				}
			}
			fseek(input, present, SEEK_SET);	/* go back to where we were */
		}
/*		sprintf(str, "maxgrey %d", maxgrey); wincancel(str); 	*/
/*		build lookup table based on maximum color component seen */
/*		if (maxgrey == 0) maxgrey = 1; */			/* paranoia ??? */
		for (k = 0; k <= maxgrey; k++) 
			lookup[k] = ((unsigned int) k * 255 + maxgrey - 1) / maxgrey;
/*		for (k = maxgrey; k < 256; k++) lookup[k] = 255; *//* redundant ? */

	}

/*	in case MinSampleValue and MaxSampleValue given in tags */
	if (mingrey < 0 || maxgrey < 0 || mingrey == maxgrey) {
		mingrey = 0; maxgrey = (1 << (int) BitsPerSample) - 1;
	}

	flip = 1; 						/*	DIBitmaps are reversed */
/*	monochrome (CreateBitMap) - is reversed from color (CreateDIBitmap) */
/*	if (BitsPerPixel == 1) flip = 1 - flip; */ /* use CreateDIBitmap */
/*	else if (Orientation == 4) flip = 1 - flip; */	/* not for pmjean ... */
/*	need to flip when printing to PostScript printer driver */
/*	printer output is reversed from screen output */

/*  actually go and read the file now */

#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr, "compression %d BitsPerPixel %d bCompressColor %d", 
				compression, BitsPerPixel, bCompressColor);
		OutputDebugString(debugstr);
	}
#endif
/*	if (compression > 1 && compression != PACK_BITS) */ /* 92/April/2 */
/*	if ((compression > 1 && compression != PACK_BITS) || forceice != 0) */
	if ((compression > 1 && compression != PACK_BITS) ||
		(compression == PACK_BITS && BitsPerPixelX == 24 && bCompressColor)) { 
/*		added last clause 98/Aug/11 */

		if (UseOurTIFF(nifd) != 0) {	/* without TIFFLIB */
#ifdef DEBUGTIFF
			if (bDebug > 1) OutputDebugString("UseOurTIFF abort\n");
#endif
			bShowFlag = 0;				/* turn off displaying */
			finish = -1;				/* redundant ??? */
			bUserAbort = 1;
			flag = 1;
		}
/*		colormap = 0; */	/* NO ! */
	}			/* end of compression other than PackBits */
	else {		/* if no compression or PackBits do it all ourselves !!! */
		flag = 0;								/* flag gets set if EOF hit */
		for (i = 0; i < (int) ImageLength; i++) {	/* read image lines */
/*			if (imagesize < 65536L && compression != PACK_BITS) { 
			if (flip == 0) t = (char far *) lpImageBytes + BytesPerRow * i;
			else t = (char far *) lpImageBytes + BytesPerRow * ((ImageLength-1) - i);
			}
			else {
			if (flip == 0) s = (char huge *) lpImageBytes + (BytesPerRow * i);
			else s = (char huge *) lpImageBytes + (BytesPerRow * ((ImageLength-1) - i));
			} */
			if (flip == 0) s = (unsigned char huge *) lpImageBytes + (BytesPerRow * i);
			else s = (unsigned char huge *) lpImageBytes + (BytesPerRow * ((ImageLength-1) - i));
			t = (unsigned char far *) s;

/*			NOTE: also compare readpackbits in dvitiff.c */

			if (compression == PACK_BITS) {		/* compressed binary file */
/*				used to do the decompression directly into s rather than t */
				u = lpBuffer;
				total = 0;
				for (;;) {
					n = getc(input);
					if (n < 0) {	/* premature EOF */
						winbadimage("Premature EOF");
						flag = -1;
						break;	
					}
					else if (n < 128) {		/* use next (n+1) bytes as is */
						if (total + (n+1) <= InRowLength) /* safety valve */
							for (k = 0; k < n+1; k++) *u++ = (char) getc(input);
						else {					/* should never happen */
							ndif = (int) (InRowLength - total);
							for (k = 0; k < ndif; k++) *u++ = (char) getc(input);
							for (k = ndif; k < n+1; k++) (void) getc(input);
						}
						total += n+1;
					}
					else if (n > 128) {		/* repeat next byte (257 - n) times */
						c = getc(input);
						if (total + (257-n) <= InRowLength) {	/* safety valve */
/*							for (k = 0; k < (257 - n); k++) *u++ = (char) c; */
							(void) memset(u, c, (257 - n));	/* 1996/Jan/30 */
							u += (257 - n);
						}
						else {						/* should never happen */
							ndif = (int) (InRowLength - total);
							(void) memset(u, c, ndif);
							u += ndif;
						}
						total += (257 - n);
					}
/*					else if ( n == 128) ; */ /* 128 is a NOP */
					if (total == (int) InRowLength) break;	/* EOL */
					if (total > (int) InRowLength) {	/* too many bytes ? */
						sprintf(debugstr,
				"Too many bytes in compressed row %d (%d > %d) (%d)",
								   i, total, (int) InRowLength, n);
						winbadimage(debugstr); 
/*						winerror(buffer); */
						break;
					}
				}
/*				u = lpBuffer; */
#ifdef USEMEMCPY
				memcpy(s, lpBuffer, InRowLength);
#else
				copynearimage(s, lpBuffer, InRowLength);			/* NEW */
#endif
/*				for (j = 0; j < (int) InRowLength; j++) *s++ = *u++; */
			}						/* end of PACK_BITS compression */
			else {					/* uncompressed file */
#ifdef LONGNAMES
				nread = _lread(input, lpBuffer, (unsigned int) InRowLength);
#else
				nread = fread(lpBuffer, 1, (unsigned int) InRowLength, input);
#endif
				if (nread != (unsigned int) InRowLength) {
					flag = -1;
					break;
				} 
			
				if (ExtraSamples > 0) 	/* throw out ExtraSamples in SamplesPerPixel */
					(void) RemoveExtraSamples(lpBuffer, InRowLength);

				if (BitsPerPixelX == 24 && bCompressColor) 
				   compresscolor(lpBuffer, (unsigned int) InRowLength, lookup);
				u = lpBuffer;	/* 24 color stuff is slow, but not used */
/*				if (BitsPerPixel == 24 && bCompressColor == 0 && bBGRflag != 0) */
/*				default changed 96/Sep/7 now default is to flip */
/*				if (BitsPerPixel == 24 && bCompressColor == 0 && bBGRflag == 0) */
				if (BitsPerPixelX == 24 && bCompressColor == 0 && bBGRflag == 0) { 
					n = (int) InRowLength / 3;		/* interchange r & b */
					if (nImageBytes < 65536L) {		/* use far pointers */
						for (j = 0; j < n; j++)  {
							c = *u++; d = *u++; e = *u++;
							*t++ = (char) e; *t++ = (char) d; *t++ = (char) c;
						}
					}
					else {							/* use huge pointers */
						for (j = 0; j < n; j++)  {
							c = *u++; d = *u++; e = *u++;
							*s++ = (char) e; *s++ = (char) d; *s++ = (char) c;
						}
					}
				}	/* end of BGRflag == 0 case ... */
/*				else if (nImageBytes < 65536L) {
					for (j = 0; j < (int) OutRowLength; j++) *t++ = *u++; 
					}
				else  {
				for (j = 0; j < (int) OutRowLength; j++) *s++ = *u++; 
				} */
				else
#ifdef USEMEMCPY
					memcpy(s, u, OutRowLength);
#else
					copynearimage(s, u, OutRowLength);
#endif
			} /* end of uncompressed file case */

			if (flag != 0) break;	/* hit EOF */
			if (bEnableTermination  != 0 && checkuser() != 0) { 
				bShowFlag = 0;		/* turn off displaying */
				finish = -1;		/* redundant ??? */
				bUserAbort = 1;
				flag = 1;
				break;
			}
		} /* end of for loop over the rows */
	} /* end of code where we do the reading ourselves */
	
/*	now finished reading */

/*	if (hBuffer != NULL) {
		(void) LocalUnlock(hBuffer);
		hBuffer = LocalFree (hBuffer);
	} */	/* 95/July/27 */
	if (lpBuffer != NULL)
		lpBuffer = (unsigned char *) LocalFree ((HLOCAL) lpBuffer);

/*	if (bDebug > 1) OutputDebugString("Finished reading bitmapped image\n"); */

/*	if (colormap == 0 && input != NULL) */ 
	if (colormap == 0 && input != BAD_FILE) {
/*		fclose(input); */					/* done with input file */ 
/*		input = BAD_FILE; */				/* debugging marker */
	}
	
/*	if (colormap != 0 && input == NULL) */
	if (colormap != 0 && input == BAD_FILE) {
/*		input = fopen(filename, "rb");	*/ /* reopen the file ... */
		rewind(input);		/* ??? */
	}

	if (bUserAbort == 0) {	
#ifdef DEBUGTIFF
		if (bDebug > 1) {
			sprintf(debugstr, "ReadTIFFFile: xll %d yll %d xur %d yur %d",
				xll, yll, xur, yur);
			OutputDebugString(debugstr);
		}  			/* debugging 95/April/13 */
#endif
		(void) renderimage(hDC, input, xll, yll, xur, yur);
	} 
	else {
		if (bDebug > 1) OutputDebugString("Renderimage abort\n");
	}

/*	if (colormap != 0 && input != NULL) */
	if (colormap != 0 && input != BAD_FILE) {
/*		fclose(input);	 */
/*		input = BAD_FILE; */
	}

	if (hImage != NULL) {
		if (GlobalUnlock(hImage) > 0) {
			sprintf(debugstr, "Lock count Not Zero %s", "Image");
			wincancel(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
		hImage = GlobalFree(hImage);
/*		hImage = NULL; */				/* for debugging */
	}	
	return 0;
}

void makepalette (PBITMAPINFO pDIBInfo, int ncolors) {
	BYTE red, green, blue;
	int k;
	red = green = blue = 0;
/*	Generate 256 (= 8*8*4) RGB combinations to fill the palette entries. */
/*	Palette index is packed byte |rrr|ggg|bb| */
/*	We now expand the range of each component to fit 0 - 255 */
		for (k = 0; k < ncolors; k++){
/*			pDIBInfo->bmiColors[k].rgbRed =  red; */
/*			pDIBInfo->bmiColors[k].rgbRed =  red + 16; */
			pDIBInfo->bmiColors[k].rgbRed =
										   (BYTE) ((int) red * 255 / 224);
/*			pDIBInfo->bmiColors[k].rgbGreen =  green; */
/*			pDIBInfo->bmiColors[k].rgbGreen =  green + 16; */
			pDIBInfo->bmiColors[k].rgbGreen =
											(BYTE) ((int) green * 255 / 224);
/*			pDIBInfo->bmiColors[k].rgbBlue = blue; */
/*			pDIBInfo->bmiColors[k].rgbBlue = blue + 32; */
			pDIBInfo->bmiColors[k].rgbBlue =
											(BYTE) ((int) blue * 255 / 192);
			pDIBInfo->bmiColors[k].rgbReserved = (BYTE) 0;
/*				if (!(red += 32))
					if (!(green += 32))
						blue += 64; */
			if (!(blue += 64))
				if (!(green += 32))
					red += 32;
		}
}

long OffBits, ImageSize;		/* for BMP images */

#ifdef LONGNAMES
int renderimage (HDC hDC, HFILE input, int xll, int yll, int xur, int yur) {
#else
int renderimage (HDC hDC, FILE *input, int xll, int yll, int xur, int yur) {
#endif
	int flag=0, err, nscan;
	int ncolors, greyrange; 
	int k;
/*	BYTE red, green, blue */
	BYTE grey; 
	unsigned int Level;
	HDC hMemoryDC=NULL;
	HBITMAP hBitMap=NULL, hOldBitMap=NULL; 
	PBITMAPINFO pDIBInfo=NULL; 
	HPALETTE hPal=NULL, hOldPal=NULL, hOldMemPal=NULL; 
/*	BITMAP BitMap; */
	WORD bltmode;
	DWORD rop;
	int fig, bac;					/* 92/Apr/11 */
	HDC hWinDC;  					/* 92/Mar/29 */

#ifdef DEBUGDIB
	if (bDebug > 1) {
		sprintf(debugstr, "RenderImage BitsPerPixel %d", BitsPerPixel);
		OutputDebugString(debugstr);
	}
#endif

	if (BitsPerPixelX == 1) {				/* do monochrome case - easy */
		if (bCopyFlag == 0 && bPrintFlag == 0) {
			SavedTextColor = SetTextColor(hDC, FigureColor); 
			SavedBkColor = SetBkColor(hDC, BackColor);
		}

/*		Can't CreateCompatibleDC in MetaFile */
/*		if (bCopyFlag != 0) {
			hWinDC = GetDC(hwnd);
			hMemoryDC = CreateCompatibleDC(hWinDC); 
			(void) ReleaseDC(hwnd, hWinDC);
		} */
		if (bCopyFlag == 0 && bPrintFlag == 0)
			hMemoryDC = CreateCompatibleDC(hDC);    /* 92/Mar/26 */

/*		Use CreateDIBitmap instead  ---  92/April/11 */

		pDIBInfo = (PBITMAPINFO) LocalAlloc(LMEM_FIXED,
			sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
		if (pDIBInfo != NULL) {
			pDIBInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pDIBInfo->bmiHeader.biWidth = ImageWidth;
			pDIBInfo->bmiHeader.biHeight = ImageLength;	
			pDIBInfo->bmiHeader.biPlanes = 1;				/* required */

/*			pDIBInfo->bmiHeader.biBitCount = (WORD) BitsPerPixel; */ /* ??? */
			pDIBInfo->bmiHeader.biBitCount = (WORD) BitsPerPixelX;	/* 99/May/10 */
			pDIBInfo->bmiHeader.biCompression = BI_RGB;		/* 0L */
			pDIBInfo->bmiHeader.biSizeImage = 0L;	
/*			pDIBInfo->bmiHeader.biSizeImage = (long) BytesPerRow * ImageLength;	*/
			pDIBInfo->bmiHeader.biXPelsPerMeter = 0L;
			pDIBInfo->bmiHeader.biYPelsPerMeter = 0L;
			pDIBInfo->bmiHeader.biClrUsed = 0L; 
/*			pDIBInfo->bmiHeader.biClrUsed = (long) ncolors; */
			pDIBInfo->bmiHeader.biClrImportant = 0L; 
/*			pDIBInfo->bmiHeader.biClrImportant = (long) ncolors - 20; */

			ncolors = 2;
			if (PhotometricInterpretation == 0) {	/* reversal */
				fig = 1; bac = 0;
			}
			else {		/* `normal' case */
				fig = 0; bac = 1;
			}
/*			FigureColor */ 
			pDIBInfo->bmiColors[fig].rgbRed = GetRValue(FigureColor); 
			pDIBInfo->bmiColors[fig].rgbGreen = GetGValue(FigureColor);
			pDIBInfo->bmiColors[fig].rgbBlue = GetBValue(FigureColor); 

/*			BackgroundColor */ 
			pDIBInfo->bmiColors[bac].rgbRed = GetRValue(BackColor);
			pDIBInfo->bmiColors[bac].rgbGreen = GetGValue(BackColor);
			pDIBInfo->bmiColors[bac].rgbBlue = GetBValue(BackColor); 

#ifdef DEBUGDIB
			if (bDebug > 1) {
				sprintf(debugstr,
"CreateDIB %d %d x %d (Planes %d) BitCount %d (Compression %d)\
Size %d (%d %d) %d %d",
						pDIBInfo->bmiHeader.biSize,
						pDIBInfo->bmiHeader.biWidth,				
						pDIBInfo->bmiHeader.biHeight,
						pDIBInfo->bmiHeader.biPlanes,
						pDIBInfo->bmiHeader.biBitCount,
						pDIBInfo->bmiHeader.biCompression,
						pDIBInfo->bmiHeader.biSizeImage,
						pDIBInfo->bmiHeader.biXPelsPerMeter,
						pDIBInfo->bmiHeader.biYPelsPerMeter,
						pDIBInfo->bmiHeader.biClrUsed,
						pDIBInfo->bmiHeader.biClrImportant);
				OutputDebugString(debugstr);
			}
#endif

/*			Can't do CreateDIBitmap in MetaFile DC ... */
			if (bCopyFlag != 0) {	/* 92/Mar/29 */
				hWinDC = GetDC(hwnd);
				hBitMap = CreateDIBitmap(hWinDC,
					(LPBITMAPINFOHEADER)&(pDIBInfo->bmiHeader), CBM_INIT,
					lpImageBytes, (LPBITMAPINFO)pDIBInfo, DIB_RGB_COLORS);
				(void) ReleaseDC(hwnd, hWinDC);
			}
			else {
				hBitMap = CreateDIBitmap(hDC,
					(LPBITMAPINFOHEADER)&(pDIBInfo->bmiHeader), CBM_INIT,
					lpImageBytes, (LPBITMAPINFO)pDIBInfo, DIB_RGB_COLORS);
			}

			if (PhotometricInterpretation == 0) {
/*				bltmode = WHITEONBLACK; rop = NOTSRCCOPY; */
				bltmode = BLACKONWHITE; rop = SRCCOPY; 
			}
			else if (PhotometricInterpretation == 1) {
				bltmode = BLACKONWHITE; rop = SRCCOPY;
			}
			else {
				bltmode = BLACKONWHITE; rop = SRCCOPY;
			}

/*			if (bDebug > 1) OutputDebugString("StretchBlt or StretchDIBits\n");*/

			if (hBitMap != NULL) {
				if (hMemoryDC != NULL)
					hOldBitMap = SelectObject(hMemoryDC, hBitMap);
				(void) SetStretchBltMode(hDC, bltmode); 
/*				rop = BLACKNESS; */	/* debugging hack 95/April/13 */
/*				if (bDebug > 1) {
					sprintf(debugstr,
		"xll %d yur %d xur-xll %d yll-yur %d ImageWidth %d ImageLength %d rop %d\n",
					xll, yur, xur-xll, yll-yur,
					(int) ImageWidth, (int) ImageLength, rop);
					OutputDebugString(debugstr);
				} */ /* debugging 95/Apr/13 */
				if (bPrintFlag == 0 && bCopyFlag == 0) { /* on screen */
					err = StretchBlt (hDC, xll, yur, xur-xll, yll-yur, 
					hMemoryDC, 0, 0, (int) ImageWidth, (int) ImageLength, 
									  rop);
					if (err == 0) {
/*						if (bDebug)	 wincancel("Did not draw bitmap!"); */
						if (bDebug)	 winbadimage("Did not draw bitmap!");
					}
/* probably because it was just too big !!! */
				}
				else { /* in case we are Printing or MetaFiling */
					nscan = StretchDIBits(hDC, xll, yur, xur-xll, yll-yur,
						0, 0, (int) ImageWidth, (int) ImageLength, 
						lpImageBytes, (LPBITMAPINFO)pDIBInfo,
							DIB_RGB_COLORS,	rop);
				}
				if ((UINT) hOldBitMap > 1 && hMemoryDC != NULL)
					(void) SelectObject(hMemoryDC, hOldBitMap);
				if ((UINT) hBitMap > 1)
					(void) DeleteObject(hBitMap);
			} /* hBitMap != NULL */
/*			else wincancel ("Failed to create bitmap"); */
			else {
				sprintf(debugstr,
	"Failed to create bitmap for %d x %d x %d image\n%ld bytes\nFile: %s",
				ImageWidth, ImageLength, BitsPerPixel, nImageBytes, FileName);
/*				winbadimage ("Failed to create bitmap"); */
				winbadimage(debugstr);
			}
		} /* pbInfo != NULL */
/*		else wincancel ("Unable to allocate memory"); */
		else winbadimage ("Unable to allocate memory");
/*		Treat this memory allocation error somewhat less seriously ??? */

		if (hMemoryDC != NULL) (void) DeleteDC(hMemoryDC);
		if (bCopyFlag == 0 && bPrintFlag == 0) {
			(void) SetTextColor(hDC, SavedTextColor);
			(void) SetBkColor(hDC, SavedBkColor);
		}
	}	/* end of BitsPerPixel == 1 case */
	
	ncolors = (1 << BitsPerSample);
	if (BitsPerPixelX == 24) {
		if (bCompressColor) ncolors = 256;
		else ncolors = 0;							/* not for RGB color ? */
	}

/*	BitsPerSample == 2 is actually not supported, sigh... */
/*	We get here for everything except BitsPerSample == 1 */
	if (BitsPerSample == 8 || BitsPerSample == 4 || BitsPerSample == 2) { 
		pDIBInfo = (PBITMAPINFO) LocalAlloc(LMEM_FIXED,
			sizeof(BITMAPINFOHEADER) + ncolors * sizeof(RGBQUAD));
		if (pDIBInfo == NULL) {
			winbadimage("Local Allocation Failed");
			return flag;
		}
		pDIBInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pDIBInfo->bmiHeader.biWidth = ImageWidth;
		pDIBInfo->bmiHeader.biHeight = ImageLength;	
		pDIBInfo->bmiHeader.biPlanes = 1;				/* required */
/*		pDIBInfo->bmiHeader.biBitCount = (WORD) BitsPerPixel; */ /* ??? */
		pDIBInfo->bmiHeader.biBitCount = (WORD) BitsPerPixelX;	/* 99/May/10 */
/*		if (BitsPerPixel == 24 && bCompressColor != 0 && bmpflag == 0) */
		if (BitsPerPixelX == 24 && bCompressColor && bmpflag == 0)
			pDIBInfo->bmiHeader.biBitCount = (WORD) (BitsPerPixelX/3);
		pDIBInfo->bmiHeader.biCompression = BI_RGB;		/* 0L */
		pDIBInfo->bmiHeader.biSizeImage = 0L;
/*		pDIBInfo->bmiHeader.biSizeImage = (long) BytesPerRow * ImageLength;	*/
		pDIBInfo->bmiHeader.biXPelsPerMeter = 0L;
		pDIBInfo->bmiHeader.biYPelsPerMeter = 0L;
		pDIBInfo->bmiHeader.biClrUsed = 0L;
/*		pDIBInfo->bmiHeader.biClrUsed = (long) ncolors; */
		pDIBInfo->bmiHeader.biClrImportant = 0L; 
/*		pDIBInfo->bmiHeader.biClrImportant = (long) ncolors - 20; */
		
		if (bmpflag) {			/* if BMP image read color palette*/
			if (BitsPerPixelX == 24) makepalette (pDIBInfo, ncolors);
			else {
				int nlen;
				fseek(input, (long) colormap, SEEK_SET);
#ifdef LONGNAMES
				nlen = _lread(input, &(pDIBInfo->bmiColors[0]), sizeof(RGBQUAD) * nColors);
#else
				nlen = fread(&(pDIBInfo->bmiColors[0]), sizeof(RGBQUAD), nColor, input);
#endif
#ifdef DEBUGBMP
				if (bDebug > 1) {
					sprintf(debugstr, "Read color map of %d bytes from %d",
							nlen, colormap);
					OutputDebugString(debugstr);
				}
#endif
				if (compression > 0) pDIBInfo->bmiHeader.biSizeImage = ImageSize;
			}
/*			copy fields from BMP file read */
			pDIBInfo->bmiHeader.biCompression = compression;	/* saved */
/*			pDIBInfo->bmiHeader.biSizeImage = ImageSize; */		/* ? */
		}

/* #define BI_RGB        0L */
/* #define BI_RLE8       1L */
/* #define BI_RLE4       2L */
/* #define BI_BITFIELDS  3L */

/*		for (k=0; k < 16; k++) {
			int i;
			char *s=debugstr;
			for (i=0; i < 16; i++) {
				sprintf(s, "%d ", lpImageBytes[k*16+i]);
				s += strlen(s);
			}
			if(wincancel(debugstr)) break;
		} */

		if (bmpflag == 0) {		/* TIFF image */
			if (colormap > 0) {		/* if TIFF image was Palette Color Image */
/*				sprintf(str, "colormap at %ld", colormap); */
/*				wincancel(str); */
				if (colormaplength != 3 * ncolors) {	/* 93/Aug/3 */
					if (bDebug > 1) {
						sprintf(debugstr, "%d colormap entries for %d colors?\n",
							colormaplength, ncolors);
						OutputDebugString(debugstr);
					}
				}
/*				fseek(input, colormap, SEEK_SET); */	/* fixed 96/Sep/12 */
				fseek(input, (long) colormap + tiffoffset, SEEK_SET);
/*	Color map entries are 16 bits - pick out top 8 bits */
				for (k = 0; k < ncolors; k++) {
					Level = ureadtwo(input);
					pDIBInfo->bmiColors[k].rgbRed =  (BYTE) (Level >> 8);
//					pDIBInfo->bmiColors[k].rgbRed =  255;
				}
				for (k = 0; k < ncolors; k++) {
					Level = ureadtwo(input);
					pDIBInfo->bmiColors[k].rgbGreen =  (BYTE) (Level >> 8);
//					pDIBInfo->bmiColors[k].rgbGreen =  0;
				}
				for (k = 0; k < ncolors; k++) {
					Level = ureadtwo(input);
					pDIBInfo->bmiColors[k].rgbBlue =  (BYTE) (Level >> 8);
//					pDIBInfo->bmiColors[k].rgbBlue =  255;
				}
				for (k = 0; k < ncolors; k++) 
//					pDIBInfo->bmiColors[k].rgbReserved = (BYTE) 0;
					pDIBInfo->bmiColors[k].rgbReserved = 0;
/*				if (bDebug) {
					for (k = 0; k < ncolors; k++) {
						sprintf(debugstr, "RGB %d: %d %d %d\n", k,
								pDIBInfo->bmiColors[k].rgbRed,
								pDIBInfo->bmiColors[k].rgbGreen,
								pDIBInfo->bmiColors[k].rgbBlue);
						if (wincancel(debugstr)) break;
					}
				} */
			}		/* end of colormap case - TIFF with palette color */

/*			perhaps build this palette even when 24 bit color and no bCompressColor */
/*			else if (BitsPerPixel == 24 && bCompressColor != 0) */
/*			else if (BitsPerPixel == 24) */ 		/* 96/Sep/15 */
			else if (BitsPerPixelX == 24) {
				makepalette(pDIBInfo, ncolors);
			}	/* end of 24 bit color case (with CompressColor) */
			else {	/* not palette or RGB color - assume grey tone */
				greyrange = maxgrey - mingrey;
				if (greyrange == 0) {				/* trap error */
/*					wincancel("MaxGrey == MinGrey"); */
					winbadimage("MaxGrey == MinGrey");
					greyrange = 1;
				}
/*				Build palette for grey image */
				for (k = 0; k < ncolors; k++) {
					if (PhotometricInterpretation == 0) { /* reverse */
						if (bStretchGray == 0)
							grey = (BYTE) (((long) (ncolors - 1- k) * 255) / 
								(ncolors - 1)); 
						else {
							if (k < mingrey) grey = 255;
							else if (k > maxgrey) grey = 0;
							else grey = (BYTE) (((long) (maxgrey - k) * 255) /
								greyrange);		
						}
					}
					else {
						if (bStretchGray == 0)
							grey = (BYTE) (((long) k * 255) / (ncolors - 1)); 
						else {
							if (k < mingrey) grey = 0;
							else if (k > maxgrey) grey = 255;
							else grey = (BYTE) (((long) (k - mingrey) * 255) /
								greyrange);		
						}
					}
					pDIBInfo->bmiColors[k].rgbRed = grey;
					pDIBInfo->bmiColors[k].rgbGreen = grey;
					pDIBInfo->bmiColors[k].rgbBlue = grey;
					pDIBInfo->bmiColors[k].rgbReserved = (BYTE) 0;
				}
			}	/* end of grey tone - non RGB 24 bit, non palette colormap */
		} /* end of if bmpflag == 0 i.e. end of TIFF color case */

/*		Now Select and Realize the Palette we constructed */
		hPal = CreateBIPalette(&(pDIBInfo->bmiHeader)); /* ??? */
/*		hPal = CreateBIPalette(pDIBInfo); */		/* ??? */
		hOldPal = NULL;
		if (hPal == NULL) winbadimage("Bad Palette (CreateBIPalette)");
		else {
			hOldPal = SelectPalette(hDC, hPal, FALSE); 
			if (hOldPal == NULL) winbadimage("Can't Select Palette");
			else (void) RealizePalette(hDC);
		}

#ifdef DEBUGDIB
		if (bDebug > 1) {
			sprintf(debugstr,
"CreateDIB %d %d x %d (Planes %d) BitCount %d (Compression %d) Size %d (%d %d) %d %d",
					pDIBInfo->bmiHeader.biSize,
					pDIBInfo->bmiHeader.biWidth,				
					pDIBInfo->bmiHeader.biHeight,
					pDIBInfo->bmiHeader.biPlanes,
					pDIBInfo->bmiHeader.biBitCount,
					pDIBInfo->bmiHeader.biCompression,
					pDIBInfo->bmiHeader.biSizeImage,
					pDIBInfo->bmiHeader.biXPelsPerMeter,
					pDIBInfo->bmiHeader.biYPelsPerMeter,
					pDIBInfo->bmiHeader.biClrUsed,
					pDIBInfo->bmiHeader.biClrImportant);
			OutputDebugString(debugstr);
		}
#endif
/*		Can't do CreateDIBitmap in MetaFile DC ... */
		if (bCopyFlag != 0) {	/* 92/Mar/29 */
			hWinDC = GetDC(hwnd);
			hBitMap = CreateDIBitmap(hWinDC,
				(LPBITMAPINFOHEADER)&(pDIBInfo->bmiHeader), CBM_INIT,
					lpImageBytes, (LPBITMAPINFO)pDIBInfo, DIB_RGB_COLORS);
			(void) ReleaseDC(hwnd, hWinDC);
		}
		else {
			hBitMap = CreateDIBitmap(hDC,
				(LPBITMAPINFOHEADER)&(pDIBInfo->bmiHeader), CBM_INIT,
					lpImageBytes, (LPBITMAPINFO)pDIBInfo, DIB_RGB_COLORS);
		}
		if (hBitMap != NULL) {	/* if we actually have a bitmap */
/*			Can't CreateCompatibleDC in  MetaFile */
/*			if (bCopyFlag != 0) {	
				hWinDC = GetDC(hwnd);
				hMemoryDC = CreateCompatibleDC(hWinDC);
				(void) ReleaseDC(hwnd, hWinDC);
			} */
			if (bCopyFlag == 0 && bPrintFlag == 0)
				hMemoryDC = CreateCompatibleDC(hDC); /* 92/Mar/26 */

/*			if (hMemoryDC != NULL) */
			hOldMemPal = NULL;
			if (hPal == NULL) winbadimage("Bad Palette");
			else {
				if (hMemoryDC != NULL) {
					hOldMemPal = SelectPalette(hMemoryDC, hPal, FALSE); 
					if (hOldMemPal == NULL) winbadimage("Can't Select Palette");
					else (void) RealizePalette(hMemoryDC);
				}
			}

			if (hMemoryDC != NULL) 
				hOldBitMap = SelectObject(hMemoryDC, hBitMap);
			(void) SetStretchBltMode(hDC, COLORONCOLOR); /* ? */
/* any need to set some sort of clipping region here ? */
			if (bPrintFlag == 0 && bCopyFlag == 0) {
				err = StretchBlt (hDC, xll, yur, xur-xll, yll-yur, 
						hMemoryDC, 0, 0, (int) ImageWidth, (int) ImageLength, 
								SRCCOPY);
/*				if (bDebug && err == 0) */
/*						wincancel("Did not draw bitmap"); */
				if (err == 0) {
					if (bDebug) winbadimage("Did not draw bitmap");
/* probably because it was just too big !!! */
				}
			}
			else {	/* in case we are Printing or MetaFiling */
				(void) StretchDIBits(hDC, xll, yur, xur-xll, yll-yur,
							0, 0, (int) ImageWidth, (int) ImageLength, 
								lpImageBytes, (LPBITMAPINFO)pDIBInfo,
									DIB_RGB_COLORS,	SRCCOPY);	 
			}

			if ((UINT) hOldBitMap > 1 && hMemoryDC != NULL) 
				(void) SelectObject (hMemoryDC, hOldBitMap);

			if (hOldMemPal != NULL) {
				if ((UINT) hOldMemPal > 1 && hMemoryDC != NULL)
					(void) SelectPalette(hMemoryDC, hOldMemPal, FALSE);
/*						(void) DeleteObject(hPal); */
/*						hPal = NULL; */
			}
			if (hMemoryDC != NULL) (void) DeleteDC (hMemoryDC);
/*				} else wincancel("CreateCompatibleDC failed"); */
			if ((UINT) hBitMap > 1)
				(void) DeleteObject (hBitMap);
		}
/*		else wincancel("CreateDIBitmap failed"); */
		else winbadimage("CreateDIBitmap failed");

		if (hOldPal != NULL) {
			if ((UINT) hOldPal > 1)
				(void) SelectPalette(hDC, hOldPal, FALSE);
			if ((UINT) hPal > 1)
				(void) DeleteObject(hPal);
			hPal = NULL;
		} 
/*		(void) LocalFree ((HANDLE) pDIBInfo);
		pDIBInfo = NULL; */
		pDIBInfo = (PBITMAPINFO) LocalFree ((HLOCAL) pDIBInfo);
/*		else wincancel("Local Allocation Failed"); */
/*		else winbadimage("Local Allocation Failed"); */
	}	/* end of BitsPerSample == 8, or 4, or 2 case */
	return flag;		/* not changed ! */
}

/****************************************************************************
 *																			*
 *  FUNCTION   : DibNumColors(VOID FAR * pv)								*
 *																			*
 *  PURPOSE    : Determines the number of colors in the DIB by looking at   *
 *		 the BitCount field in the info block.								*
 *																			*
 *  RETURNS    : The number of colors in the DIB.							*
 *																			*
 ****************************************************************************/

WORD DibNumColors (VOID FAR *pv) {
    int 		bits;
    LPBITMAPINFOHEADER	lpbi;
    LPBITMAPCOREHEADER	lpbc;

    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);

/*	With the BITMAPINFO format headers, the size of the palette
 *	is in biClrUsed, whereas in the BITMAPCORE - style headers, it
 *	is dependent on the bits per pixel ( = 2 raised to the power of
 *	bits/pixel).
 */
    if (lpbi->biSize != sizeof(BITMAPCOREHEADER)){
        if (lpbi->biClrUsed != 0)
            return (WORD)lpbi->biClrUsed;
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;

    switch (bits){
		case 1:
			return 2;
		case 4:
			return 16;
		case 8:
			return 256;
		case 16:
			return 256;			/* ad hoc solution 99/May/10 for ExtraSamples=1 */
		case 24:
			return 0;			/* A 24 bitcount DIB has no color table */
		case 32:
			return 0;			/* ad hoc solution 99/May/10 for ExtraSamples=1 */
		default:
			sprintf(debugstr, "Bad number of bits %d\n", bits);
			winerror(debugstr);
			return 0;
	}
}

/****************************************************************************
 *																			*
 *  FUNCTION   : CreateBIPalette(LPBITMAPINFOHEADER lpbi)					*
 *																			*
 *  PURPOSE    : Given a Pointer to a BITMAPINFO struct will create a	    *
 *		 a GDI palette object from the color table.							*
 *																			*
 *  RETURNS    : A handle to the palette.									*
 *																			*
 ****************************************************************************/

HPALETTE CreateBIPalette (LPBITMAPINFOHEADER lpbi) {
    LOGPALETTE          *pPal;
    HPALETTE            hpal = NULL;
    WORD                nNumColors;
    BYTE                red;
    BYTE                green;
    BYTE                blue;
    int                 i;
    RGBQUAD        FAR *pRgb;

    if (! lpbi)	return NULL;

    if (lpbi->biSize != sizeof(BITMAPINFOHEADER)) return NULL;

/*	Get a pointer to the color table and the number of colors in it */
    pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)lpbi->biSize);
    nNumColors = DibNumColors(lpbi);

/*	sprintf(str, "nNumColors %d", nNumColors);
	wincancel(str); */

    if (nNumColors > 0) {	/* Allocate for the logical palette structure */
        pPal = (LOGPALETTE*) LocalAlloc (LPTR,
				sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
		if (! pPal)  return NULL;

		pPal->palVersion    = PALVERSION;
		pPal->palNumEntries = nNumColors;

	/*	Fill in the palette entries from the DIB color table and
		create a logical color palette. */
		for (i = 0; i < (int) nNumColors; i++) {
			red = pRgb[i].rgbRed;
			green = pRgb[i].rgbGreen;
			blue = pRgb[i].rgbBlue;
			pPal->palPalEntry[i].peRed = red; 
			pPal->palPalEntry[i].peGreen = green; 
			pPal->palPalEntry[i].peBlue  = blue; 
			pPal->palPalEntry[i].peFlags = (BYTE) 0;
/*			if (red != (BYTE) i || green != (BYTE) i || blue != (BYTE) i) {
				sprintf(str, "i %d r %d g %d b %d n %d", 
					i, red, green, blue, nNumColors);
				wincancel(str);	
				break;
			} */
		}
		hpal = CreatePalette(pPal);
/*		(void) LocalFree ((HANDLE)pPal); */
		if (hpal == NULL) winbadimage("CreatePalette failed");	// debugging only ?
		pPal = (LOGPALETTE*) LocalFree((HLOCAL) pPal); /* 95/July/27 ??? */
/*		pPal = NULL; */ /* ??? */
/*		wincancel("Created Palette"); */
	}	/* end of nNumColors != 0 */
	else if (lpbi->biBitCount == 24 ||
			 lpbi->biBitCount == 32) {
/*		A 24 bitcount DIB has no color table entries so, set to maximum	of 256 */
/*		A 36 bitcount DIB has no color table entries so, set to maximum	of 256 */
		nNumColors = MAXPALETTE;
        pPal = (LOGPALETTE*) LocalAlloc (LPTR,
				sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
        if (!pPal) return NULL;

		pPal->palNumEntries = nNumColors;
		pPal->palVersion    = PALVERSION;

		red = green = blue = 0;

/*		Generate 256 (= 8*8*4) RGB combinations to fill the palette entries. */
		for (i = 0; i < (int) pPal->palNumEntries; i++){
            pPal->palPalEntry[i].peRed = red;
            pPal->palPalEntry[i].peGreen = green;
            pPal->palPalEntry[i].peBlue = blue;
            pPal->palPalEntry[i].peFlags = (BYTE) 0;

			if (!(red += 32))
				if (!(green += 32))
					blue += 64;
		}
        hpal = CreatePalette(pPal);
		if (hpal == NULL) winbadimage("CreatePalette failed");	// debugging only ?
/*      (void) LocalFree ((HANDLE)pPal); */
        pPal = (LOGPALETTE*) LocalFree ((HANDLE) pPal); /* 95/July/27 ??? */
/*		pPal = NULL; */ /* ??? */
	}
//	else {
//		sprintf(debugstr, "nNumColors %d lpbi->biBitCount %d", nNumColors, lpbi->biBitCount);
//		winbadimage(debugstr);		// debugging only
//	}
	return hpal;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* NOTE: if offset is used directly for value */
/* then value is packed left justified */

/* not implemented: multiple strips (unless they happen to be contiguous) */

/* not implemented: GrayResponseCurve */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following dumps placeable METAFILE header - no good on in-core MetaFiles */

/* Following is in windows.h / wingdi.h - note packing code there ... */
/* typedef struct tagMETAHEADER {
	WORD mtType;
	WORD mtHeaderSize;
	WORD mtVersion;
	DWORD mtSize;
	WORD mtNoObjects;
	DWORD mtMaxRecord;
	WORD mtNoParameters;
} METAFILEHEADER, FAR *LPMETAFILEHEADER; */

/* typedef struct tagMETAHEADER UNALIGNED FAR *LPMETAHEADER; */
typedef struct tagMETAHEADER FAR *LPMETAHEADER;

/* #ifdef DEBUGMETAFILE */
/* void ShowMetaFileHeader (void *mf, char *name) */
void ShowMetaFileHeader (char far * mf, char *name) {
	LPMETAHEADER lpMF = (LPMETAHEADER) mf;
	
	if (bDebug < 2) return;
	sprintf(debugstr, "Type %u Head %u Version %0X Size %lu\n",
			lpMF->mtType, lpMF->mtHeaderSize, lpMF->mtVersion, lpMF->mtSize);
	OutputDebugString(debugstr);
	sprintf(debugstr,	"Objects %u MaxRecord %lu NoParams %u\n",
			lpMF->mtNoObjects, lpMF->mtMaxRecord, lpMF->mtNoParameters);
	OutputDebugString(debugstr);
}
/* #endif */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int xHmfLogExt=1024, yHmfLogExt=1024;		/* logical extent of WMF */
/* we can't set these, since the WMF preview is not a placeable WMF */
/* fortunately it is not needed if bTrustPlaceable == 0 */

/* reads file and creates in core MetaFile */
/* position at start of WMF in the file (1, 0, ...) */
/* remember, if this returns NULL, close file and restore DC */

/* This version uses STREAM input */	/* see other version in winsearc.c */

#ifdef LONGNAMES
static HMETAFILE MakeMetaFile (HFILE input, long metalength) {
#else
static HMETAFILE MakeMetaFile (FILE *input, long metalength) {
#endif
	HGLOBAL hMem=NULL;
	HMETAFILE hMF=NULL;
	unsigned char huge *lpMetaFile;				/* use huge pointer */
	unsigned char huge *u;						/* use huge pointer */
	unsigned char buffer[256];					/* small near buffer */
	int nlen;
	long metabytes;						/* how much has been read */

	hMem = GlobalAlloc(GMEM_MOVEABLE, metalength);
	if (hMem == NULL) {
		winerror ("Unable to allocate memory");
		return NULL;								/* 1995/Jan/19 */
	}
	lpMetaFile = (unsigned char huge *) GlobalLock (hMem);	/* make huge pointer ??? */
	if (lpMetaFile == NULL) {
		winerror ("Unable to allocate memory");	/* debugging only */
		return NULL;
	}
/*	read it all in */ /* use `huge' pointer to read more than 64k ? */
	u = lpMetaFile;
	metabytes = 0;
/*	*u++ = (char) 1; */
/*	*u++ = (char) 0; */
/*	metabytes = 2; */
/*	can't we read this directly in 64k chunks using _lread ? */
/*	since _lread treats pointer to buffer as huge ? */
/*	IN FACT in WINVER >=0x03a, WE CAN USE _hread TO READ THE WHOLE THING */
	for (;;) {
		nlen = sizeof(buffer);
		if (metabytes + nlen > metalength)
			nlen = (int) (metalength - metabytes);
#ifdef LONGNAMES
		nlen = _lread (input, buffer, nlen);
#else
		nlen = fread (buffer, 1, nlen, input);
#endif
		if (nlen == 0) break;			/* premature EOF */
#ifdef USEMEMCPY
		memcpy(u, buffer, nlen);
#else
		copynearimage(u, buffer, nlen);
#endif
		metabytes = metabytes + nlen;
		if (metabytes >= metalength) break;
		u = u + nlen;
	}
	if (metabytes != metalength) {
		if (bDebug > 1) {		/* debugging */
			sprintf(debugstr, "Read %ld bytes out of %ld\n",
					metabytes, metalength);
			OutputDebugString(debugstr);
		}
	}
#ifdef DEBUGMETAFILE
	if (bDebug > 1) ShowMetaFileHeader(lpMetaFile, "MakeMetaFile winspeci"); 
#endif
/*	should set up xHmfLogExt, yHmfLogExt ? */
/*	ReadHeader (lpMetaFile); */ /* can't, not a placeable metafile ! */
/*	need to use SetMetaFileBits to convert global memory object to MetaFile */

/*	hMF = SetMetaFileBitsEx(metalength, hMem); */ /* ??? */
	hMF = SetMetaFileBitsEx(metalength,
			(const unsigned char *) lpMetaFile); /* GlobalFree later ??? */
	if (hMF == NULL) {
		sprintf(debugstr, "SetMetaFileBitsEx failed %ld", metalength);
		wincancel(debugstr);	/* debugging */
	}
/*	Should we do hMem = GlobalFree (hMem) ??? */

/*	hMF = SetMetaFileBits(hMem); */
/*	if (hMF == NULL) wincancel("SetMetaFileBits failed"); */

	if (hMF == NULL) {
		GlobalUnlock(hMem);
		GlobalFree (hMem);
		hMem = NULL;
	}					/* 1995/Nov/5 */
	hMem = NULL;		/* no longer allowed to refer to it this way */
	return hMF;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following is in winsear.c meant for playing metafile in metafile DC */

int PlayInMetafile(HDC, HMETAFILE, int, int, int, int, int, int, int, int);

#ifdef LONGNAMES
int readwmffile (HDC hDC, HFILE input, int xll, int yll, int xur, int yur) {
#else
int readwmffile (HDC hDC, FILE *input, int xll, int yll, int xur, int yur) {
#endif
	HMETAFILE hMF;
	int mapmode = MM_ANISOTROPIC;			/* default assumption */
	int nSavedDC; 
/*	int nlen; */
	int ret;
/*	HGLOBAL hMem=NULL; */
/*	LPSTR lpMetaFile; */				/* maybe use huge pointer ??? */
/*	char huge *lpMetaFile; */			/* maybe use huge pointer ??? */
/*	char huge *u; */					/* maybe use huge pointer ??? */
/*	char *s; */							/* pointer into local buffer */
/*	char buffer[256]; */				/* small near buffer */
/*	long metabytes; */					/* how much has been read */
	int xcurr, ycurr;					/* origin in device units */
	POINT posarr[2];
	DWORD dret;
/*	int xview, yview; */			/* rectangle in logical units */
	int xextend, yextend;		/* rectangle in device units */

	if (bShowFlag == 0) return 0;	
/*	return -1; */						/* EPSF with WMF not implemented */

/*	Bounding box given in logical units (TWIPS) */
	if (bDebug > 1) {
		sprintf(debugstr, "PlayInMeta: xll %d yll %d xur %d yur %d\n", xll, yll, xur, yur);
		OutputDebugString(debugstr);
	}

	nSavedDC = SaveDC(hDC);				/* Don't want MF messing up DC ! */
	if (nSavedDC == 0) {
		winerror("Failed to Save DC");	/* debugging only */
		return -1;						/* some kind of error */
	}
/*  Cannot use Windows on the file, because it is not at start of file ... */

	fseek(input, -2, SEEK_CUR);			/* step back over 1, 0 */
	hMF = MakeMetaFile (input, metalength);

/*	fclose (input); */				/* ??? */
	if (hMF == NULL) {
/*		fclose (input); */			/* ??? */
		if (bCopyFlag == 0) RestoreDC(hDC, nSavedDC);
		else RestoreDC(hDC, -1);
		return -1;					/* failed */
	}

	SetMapMode (hDC, mapmode);		/* should not be needed ? */

	posarr[0].x=xll;
	posarr[0].y=yll;
	posarr[1].x=xur;
	posarr[1].y=yur;

	if (bCopyFlag == 0) {
		ret = LPtoDP(hDC, posarr, 2);  /* problem if CopyFlag ? avoid this ? */
		if (ret == 0) {
			if (bDebug > 1) OutputDebugString("LPtoDP failed\n");
		}
	}
/*	extract top left corner */
	xcurr = posarr[0].x;				/* device units - pixels */
	ycurr = posarr[1].y;				/* device units - pixels */
#ifdef DEBUGMETAFILE
	if (bDebug > 1) {
		sprintf(debugstr, "SetViewportOrg %d %d\n", xcurr, ycurr);
		OutputDebugString(debugstr);
	}
#endif
	dret = SetViewportOrgEx(hDC, xcurr, ycurr, NULL);
	xextend = posarr[1].x - posarr[0].x;
	yextend = posarr[0].y - posarr[1].y;

#ifdef DEBUGMETAFILE
	if (bDebug > 1) {
		sprintf(debugstr, "SetViewportExt %d %d\n", xextend, yextend);
		OutputDebugString(debugstr);
	}
#endif
	dret = SetViewportExtEx(hDC, xextend, yextend, NULL);  
	if (dret == 0) {
		if (bDebug > 1)	OutputDebugString("SetViewportExt failed\n");
	}
/*	if (bShowMetaFile) */		/* do we show it ? changed 97/Jan/7 */
/*		Reset `graphics state' to default	1996 Aug 5 */
		SetTextAlign(hDC, TA_NOUPDATECP);	/* default mode */
		SetBkMode(hDC, OPAQUE);				/* default mode */
		SetPolyFillMode(hDC, ALTERNATE);	/* default mode */
/*		SetBkColor, SetROP2, SetStretchBltMode ? */
/*		SetTextColor, SetTextCharacterExtra, SetTextJustification ? */
#ifdef DEBUGMETAFILE
		if (bDebug) OutputDebugString("PlayMetaFile\n");
#endif
/*		ret = PlayMetaFile(hDC, hMF); */
		if (bCopyFlag == 0) ret = PlayMetaFile(hDC, hMF);
		else {
			ret = PlayInMetafile(hDC, hMF,
/*		logical rectangle where MetaFile will be played on destination MF */
/*		left, top, right, bottom */
								 xll, yur, xur, yll,
/*		logical extents of destination MetaFile (as in SetWindowExtEx) */
								 xLogExt, yLogExt,
/*		logical extents of MetaFile being played */
								 xHmfLogExt, yHmfLogExt);
#ifdef DEBUGMETAFILE
			if (bDebug > 1) OutputDebugString("After PlayInMetafile\n");
#endif
		}
		if (ret == 0) wincancel("Bad MetaFile");	/* debugging */
/*	} */	/* bShowMetaFile changed 97/Jan/7 */
/*	if (hMF != NULL) DeleteMetaFile(hMF); */
	if (bCopyFlag == 0) RestoreDC(hDC, nSavedDC);
	else RestoreDC(hDC, -1);
	if (hMF != NULL) DeleteMetaFile(hMF);
	hMF = NULL;
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following is code from DVIPSONE, module dvispeci.c */

/**********************************************************************
*
*	Program for dealing with TeX's \special commands
*
*	Copyright (C) 90 Berthold K. P. Horn. All Rights Reserved
*
**********************************************************************/

int	psfigstyle=0;		/* non-zero if new style DVI2PS special */
int	pstagstyle=0;		/* non-zero if new style DVIPS special */
int	pssvbstyle=0;		/* non-zero if new style DVIPS special */
int pstpsstyle=0;		/* non-zero if new style DVITPS special */

long fliteral=0;			/* place in file where literal was */
long nliteral=0;			/* nspecial at start of literal */

/* int flushinit=1;	*/ /* flush initial colon in textures special */

/* disable other things ? */ /* check stack at specialend ? */

/* BOOL smartcopyflag=1; */	/* non-zero => look for %%EOF and such in EPSF */
BOOL verbatimflag=0;		/* non-zero => allow verbatim PostScript */
BOOL bFlushCR=1;			/* try and get rid of those pesky \r in input */
/* BOOL bAllowAndrew=1; */	/* try and allow Andrew Trevorrow style */

/*	figure inclusion parameters from \special */ /* NOTE: in scaled points */
long pswidth, psheight, psllx, pslly, psurx, psury;

static int clipflag;		/* non-zero if clipping requested */ /* not used yet */

/*	bounding box info read from EPS file */		/* NOTE: in scaled points */
long epswidth, epsheight, epsllx, epslly, epsurx, epsury;

/*  bounding box info read from EPS file */		/* NOTE: in points */
double depsllx, depslly, depsurx, depsury;

/* returns zero when reached EOF and line empty - char count otherwise */
/* get line from normal ASCII file */ /* used for getting lines in EPS file */

#ifdef LONGNAMES
int getline(HFILE input, char *buff) { 
#else
int getline(FILE *input, char *buff) { 
#endif
	int c, k=0;
	char *s=buff;

	c = getc(input);
/*	if (bFlushCR != 0) while (c == '\r') c = getc(input); */
	if (c == EOF) {
		*s = '\0';	return 0;
	}
	while ((c != '\n' && c != '\r' && c != EOF)) {
		if (c < ' ' && c != '\t') c = getc(input);
		else {
			*s++ = (char) c; k++;
			c = getc(input);
		}
	}
	if (c == '\r' && bFlushCR != 0) {
		c = getc(input); 
		if (c != '\n') (void) ungetc(c, input);
	}
	else if (c == EOF) {
		if (k > 0) c = '\n'; 
		else {
			*s = '\0'; return 0;
		}
	}
	*s++ = (char) c; k++;
	*s = '\0';
	return k;
}

/* copy special into line buffer for sscanf */
/* - either double quote delimited - or - up to end of special */

int scanspecial(int input, char *buff, int nmax) {
	int c, k=0;
	char *s=buff;

	*s = '\0';					/* in case we pop out early ... 92/Oct/23 */
	if (nspecial <= 0) return 0;		/* nothing there ? */
	c = an_wingetc(input); --nspecial;
	while (c <= ' ' && nspecial > 0) { /* skip over initial white space */
		c = an_wingetc(input); --nspecial;
	} 
/*	if (nspecial <= 0) return 0; */		/* BUG ! */
	if (c <= ' ' && nspecial <= 0) return 0;	/* nothing there 92/Oct/23 */

	an_unwingetc(c, input); nspecial++;	/* 93/Jan/23 */ /* step back to first */
	fliteral = an_wintell(input);		/* remember where this was ??? */
	nliteral = nspecial;
	c = an_wingetc(input); --nspecial;	/* 93/Jan/23 */

	if (c != '\"')  {	/* straight text */
		while (nspecial > 0) {
			if (k++ >= nmax) {
/*				fprintf(stderr, " Special too long for scan (> %d)\n", nmax);
				showbeginning(buff); */
/*				errcount(); */ /* ??? */
				flushspecial(input);
				return nmax;
			}
			*s++ = (char) c;
			c = an_wingetc(input); --nspecial;
		}
		*s++ = (char) c;	/* last one read */
	}
	else {							/* double quote delimited string */
		c = an_wingetc(input); --nspecial;
		while (c != '\"' && nspecial > 0) {
			if (k++ >= nmax) {
/*				fprintf(stderr, " Special too long for scan (> %d)\n", nmax);
				showbeginning(buff); */
/*				errcount(); */ /* ??? */
				flushspecial(input);
				return nmax;
			}
			*s++ = (char) c;
			c = an_wingetc(input); --nspecial;
		}
	}
	*s = '\0';
	return k;
}

/* read (short) double-quote-delimited string from special string */
/* possibly just use scanspecial or gettoken instead ? */
/* return value seems to be mostly ignores */

int getstring(int input, char *buff, int nmax) {
	int c, k = 0;
	char *s=buff;

	if (nspecial <= 0) return 0;
	c = an_wingetc(input); --nspecial;
	while(c != '\"' && nspecial > 0) {		/* scan up to double quote */
		c = an_wingetc(input); --nspecial;
	}
	if (nspecial <= 0) return 0;
	c = an_wingetc(input); --nspecial;
	while(c != '\"' && nspecial > 0) {
		*s++ = (char) c;
		if(k++ >= nmax) {
/*			fprintf(stderr, " String in special too long (> %d)\n", nmax); */
			showbeginning(buff);
/*			errcount(); */
			c = an_wingetc(input); --nspecial;
			while(c > '\"' && nspecial > 0) {
				c = an_wingetc(input); --nspecial;
			}
			break;
		}
		c = an_wingetc(input); --nspecial;
	}
	*s = '\0';
	return k;
}

/* skip forward to comma in special string */
void skiptocomma(int input) {	
	int c;
	if (nspecial <= 0) return;
	c = an_wingetc(input); --nspecial;
	while (c != ',' && nspecial > 0) {
		c = an_wingetc(input); --nspecial;
	}
}
	
/* skip over double-quote-delimited string in special string */
void flushstring(int input) { 
	int c;
	if (nspecial <= 0) return;
	c = an_wingetc(input); --nspecial;
	while (c != '\"' && nspecial > 0) {
		c = an_wingetc(input); --nspecial;
	}
	if (nspecial == 0) return;
	c = an_wingetc(input); --nspecial;
	while (c != '\"' && nspecial > 0 && c != EOF) {
		c = an_wingetc(input); --nspecial;
	}
}

/* copy string from special to output */
/* if first non-blank is indeed a " then copy up to the next " */
/* - otherwise copy to end of special */

void copystring(int input) { 
	int c;

	if (nspecial == 0) return;			/* nothing left to do ... */
	c = an_wingetc(input); --nspecial;
	while (c == ' ' && nspecial > 0) { /* search for non-blank */
		c = an_wingetc(input); --nspecial;
	}
	if (nspecial == 0) return;		/* all just blanks ? */
/*	putc('\n', output);	*/			/* just in case! */
	if (c == '\"') {		/* double quote delimited ? */
		c = an_wingetc(input); --nspecial;
		while (c != '\"' && nspecial > 0 && c != EOF) {
/*			putc(c, output); */
			c = an_wingetc(input); --nspecial;
		}
	}
	else {	
		while (nspecial > 0 && c != EOF) {
/*			putc(c, output); */
			c = an_wingetc(input); --nspecial; 
		}
/*		if (c != EOF) putc(c, output); */
	}
/*	putc('\n', output); */
} 

/* copy verbatim PostScript - but strip bracket enclosed crap first */
/* global | local | inline | asis | begin | end <user PS> ??? */
 
void stripbracket(int input) {
	int c;

	c = an_wingetc(input); nspecial--;
	if (c == '[')  {
		while (c != ']' && nspecial > 0) {
			c = an_wingetc(input);
			nspecial--;
		}
		if (nspecial == 0) return;
	}
	else {
		an_unwingetc(c, input); nspecial++;
	}
	copystring(input); 
} 

/* code to find deferred %%BoundingBox: (atend) if required */
/* problem here if EPSF file with included TIFF file ??? */
/* step to end of PostScript section, NOT end of file */

#define STEPSIZE 512		/* step back this far at one time */
#define NUMBERSTEPS 8		/* number of steps to try from end of file */

/* try and find bbox at end of file */
/* the following may inefficiently read stuff several times, but, so what */ 
/* needs to be careful about EPSF files, which may contain stuff after PS */

#ifdef LONGNAMES
int findbboxatend (HFILE special, char *fname) {
#else
int findbboxatend (FILE *special, char *fname) {
#endif
	int k, foundit = 0;

	if (pslength > 0) 
		fseek(special, psoffset + pslength - (long) STEPSIZE, SEEK_SET);
	else fseek(special, - (long) STEPSIZE, SEEK_END);
	if (getline(special, line) == 0) return -1;
	for (k = 0; k < NUMBERSTEPS; k++) {
		while(getline(special, line) != 0) {
			if (strncmp(line, "%%BoundingBox", 13) == 0) { /* : */
				foundit = 1; break;
			}
		}
		if (foundit != 0) break;
		fseek(special, - (long) (STEPSIZE * 2), SEEK_CUR);
	}
	if (foundit == 0) {
		sprintf(str, "Can't find %%%%BoundingBox at end in: %s", fname);
/*		if (verboseflag == 0) sprintf(str + 36, "in: %s", fname); */
		wincancel(str);
/*		errcount(); */
		return 0;
	}
	return -1;
}

/* extract bounding box from inserted eps file and offset */
/* returns zero if failed */

#ifdef LONGNAMES
int readbbox (HFILE input, char *fname) {
#else
int readbbox (FILE *input, char *fname) {
#endif
	char *s;
/*	int c; */
	int k;

	if (fseek(input, psoffset, SEEK_SET) != 0) { /* ??? */
		sprintf(str, "Error in seek to %ld", psoffset);
		wincancel(str);
		rewind(input); 
		/*	errcount(); ? */
		return 0;						/* failed */
	}	
	k = getline(input, line);	/* step over initial blank lines */
	while (*line < ' ' && k > 0) k = getline(input, line);	/* 95/Mar/28 */
/*	Let's be less fussy and also accept say %% here as well as %! */
/*	if (strncmp(line, "%!", 2) == 0) k = getline(input, line); */
	if (*line != '%') {
		if (bComplainSpecial != 0) {			/* 93/Jan/25 */
/*			sprintf(str, "File `%s' does not start with %%!  (%s)\n", */
			sprintf(debugstr, "File `%s' does not start with %%!\n%s",/* 96/Sep/12 */
					fname, line); 
/*			wincancel(str); */
			winbadimage(debugstr);
/*			errcount(); ? */
		}
	}
	else if (strncmp(line, "%!", 2) == 0) k = getline(input, line);
/*  scan until a line is found that does not start with %% */
/*  or with %X where X is not space, tab or newline (page 631 PS manual) */
	for (;;) {
/* 		if (strncmp(line, "%", 1) != 0) { k=0; break; } */ /* too whimpy */
/*		if (strncmp(line, "%%", 2) != 0) { k=0; break; } */
		if (*line != '%') {
			k=0;
			break;
		}			/* not a comment */
/*		c = *(line+1); */
/*		if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') */
		if (bAllowBadBox == 0) {					/* 1994/Aug/19 */
			if (*(line+1) <= ' ') {
				k=0;
				break;
			}	/* but DSC requires ! */
		}
		if (strncmp(line, "%%EndComments", 13) == 0) {
			k=0;
			break;
		}
		if (strncmp(line, "%%BoundingBox", 13) == 0) {
/*			if (strstr(line, "(at end)") != NULL) */   /* BoundingBox at end */
			if (strstr(line, "(atend)") != NULL ||
				strstr(line, "(at end)") != NULL)		/* 94/Aug/12 */
				k = findbboxatend(input, fname);
			break;
		}
		if ((k = getline(input, line)) == 0) break;
	}
	if (k == 0) {
/*		sprintf(str, "Can't find %%%%BoundingBox in: %s", fname);
		wincancel(str); */		/* redundant ? */
/*		errcount(); */
		rewind(input); 
		return 0;
	} 

/*	try and allow for lack of ':' after BoundingBox */
	s = strstr(line, "BoundingBox");
	if (s == NULL) {	/* can't happen */
		rewind(input); 
		return 0;
	}
	s += 12;	/* past space or colon after "Box" */
	if (sscanf(s, "%lg %lg %lg %lg", 
			&depsllx, &depslly, &depsurx, &depsury) < 4) {
		sprintf(str, "Don't understand BoundingBox: %s", line);
		wincancel(str);
/*		errcount(); */
		rewind(input); 
		return 0;
	}
	if (bDebug > 1) {
		sprintf(debugstr, "readbbox %lg %lg %lg %lg\n",
				depsllx, depslly, depsurx, depsury);
		OutputDebugString(debugstr);
	}  		/* debugging hack 95/April/13 */
	rewind(input);
	return 1;			/* apparently successful ! */
}

/* Actually use BoundingBox information */
/* if needshift > 0 then (xll, yll) is at TeX's current point */
/* if needshift = 0 then (0,0) is at Tex's current point */
/* if needshift < 0 then (xll, yur) is at TeX's current point */
/* - last one only used by DVIALW ? */

/* Check for EPSF files - with MetaFile or TIFF bitmap image */
/* Returns 0 if plain ASCII format  - or length of PS section if not */
/* or -1 if binary file and not EPSF */
/* Positioned at start of file if ASCII, after EPSF header if EPSF file */

#ifdef LONGNAMES
long checkpcform(HFILE input, char *fname) {
#else
long checkpcform(FILE *input, char *fname) {
#endif
/*	unsigned long n, m, i; */
	int c;

	tiffoffset = 0; 
	psoffset = 0; 
	metaoffset = 0; 

	c = getc(input); (void) ungetc(c, input);
	if (c < 128) { 				/* looks like plain ASCII */
		return 0;
	}
	else if (readepsfhead(input) != 0) return (long) pslength;
	else {
		sprintf(str, "File %s not PS (or EPSF) file\n", fname);
/*		errcount(); */
		wincancel(str);
		return -1;
	}
/* for weird file format, do we need to flush stuff at end also ? */
/* go only up to %%EOF (counting BeginDocument and EndDocument ? */
}

/****************************************************************************
*                                                                           *
*		The EPSF header has the following structure:						*
*                                                                           *
* 0-3	the first four bytes are the letters EPSF with the meta-bit on -	*
*		that is, hex C5D0D3C6.												*
* 4-7	the next four are the byte position of start of the PS section		*
* 8-11	The next four are the length of the PostScript part of file;        *
* 12-15	The next four are the byte position of start of MetaFile version;	*
* 16-19 The next four are the length of the MetaFile version;				*
* 20-23	The next four are the byte position of start of TIFF version;		*
* 24-27	The next four are the length of the TIFF version;					*
* 28-29	The next two bytes are the header checksum or hex FFFF				*
*       that is, two bytes that are all ones (meta-control-? = 255)			*
*                                                                           *
*		IN EACH CASE THE LOW ORDER BYTE IS GIVEN FIRST						*
*                                                                           *
*		Either the MetaFile length or the TIFF length or both are zero		*
*                                                                           *
*		If there is no MetaFile or Tiff version, the PS code starts at 30	*
*                                                                           *
****************************************************************************/

/* attempts to read bounding box - return zero if fail */
/* if fails, complain only if warnflag is non-zero */
/* doesn't complain about file not found */

/* int setupbbox(char *name, int warnflag) */
int setupbbox (char *name, int warnflag, int incflag) {	/* 93/Oct/31 */
/*	char epsfilename[MAXFILENAME]; */ 			/* 95/April/26 */

	special = findepsfile(name, warnflag, "eps");
/*	if (special == NULL)  */
	if (special == BAD_FILE) return 0;
	if ((pslength = checkpcform(special, name)) >= 0) {
		if (readbbox(special, name) == 0) {
			fclose(special);
/*			special = NULL; */
			special = BAD_FILE;
/* It's OK if "overlay" and BBox not found, but can't show box outline ? */
			if (incflag == 0) {				/* put in full page fake values */
				depsllx = 0.0; depslly = 0.0;
				depsurx = 612.0; depsury = 792.0;
				return -1;
			}
			else if (warnflag != 0) { 		/* 93/Oct/31 */
//				sprintf(debugstr, "%%%%BoundingBox not found in %s ", name);
				sprintf(debugstr, "%s not found in %s ", "%%BoundingBox", name);
				wincancel(debugstr);
				return 0;
			}
		}
	}  
	else if (warnflag != 0) {		/* redundant ??? */
/*		sprintf(debugstr, "%s not a valid PS or EPS file", name); */
		strcpy(debugstr, "Not a valid ");
		strcat(debugstr, "PS or EPS file: ");
		strcat(debugstr, FileName);
		wincancel(debugstr);
		fclose(special);
/*		special = NULL; */
		special = BAD_FILE;
		return 0;
	} 
	fclose(special);
/*	special = NULL; */
	special = BAD_FILE;
	return -1;						/* successful */
}

/* the following attempts to allow long lines other than in comments */
/* this may be more complex than really called for... */
/* result of paranoia about running into junk at end of file */
/* flush now ? has been butchered - if needed, refer to DVISPECI.C */

/*
void copyepsfilesub(FILE *special) {
	int c, nesting=0;

	if (smartcopyflag == 0) {			
		for(;;) {
			c = getc(special);
			if (c == EOF || c >= 128) return; 
			else if (c < ' ') {		
				if (c == '\n' || c == '\r' || c == '\t' ||
					c == '\b' || c == '\f')	; 
			}
		}
	} 				
	else {				
		for (;;) {
			c = getc(special);		
			if (c == EOF || c >= 128) return; 
			else if (c == '\r') { 
				c = getc(special);
				if (c != '\n') {
					(void) ungetc(c, special); 
				}
			}
			else if (c == '\n') { 
			}
			else if (c < ' ') {	  
				if ( 
				    c == '\t' || c == '\b' || c == '\f') ;
			}
			else if (c == '%') {	
				c = getc(special);
				if (c == EOF) return;
				if (c == '%') {
					if(getline(special, line) == 0) return;
					if (strncmp(line, "BeginDocument", 13) == 0) nesting++;
					else if (strncmp(line, "EndDocument", 10) == 0) nesting--;
					else if (nesting <= 0 && strncmp(line, "EOF", 3) == 0) 
						return;
				}
				else {
					while (c != '\n' && c != '\r' && c != EOF)
						c = getc(special);
					if (c == EOF) return;
					else if (c == '\r') { 
						c = getc(special);
						if (c != '\n') (void) ungetc(c, special);
					}
				}
			}
			else {
				if (c == EOF) return;
				while (c != '\n' && c != '\r' && c != EOF) {
					if (c < ' ') {	
						if ( 
							c == '\t' || c == '\b' || c == '\f') ;
					}
					c = getc(special);
				}
				if (c == EOF) return;
				else if (c == '\n') ; 
				else if (c == '\r') {
					c = getc(special);
					if (c != '\n') (void) ungetc(c, special);
				}
			}
		}
	}
} */

/* Use in EPSF file, when length of PostScript part is specified */
/* just copy across byte for byte to output */

/*
void copyepsfilesimple(FILE *special, long pslength) {
} */

/* Quick and dirty for now ... */ /* must improve later - see DVISPECI */
/* could avoid reopening file maybe */ /* but no hurry, going to printer */

/* In WIN32 there may be problem if called before any text put on page */

long copyfilethrough(HDC hDC, char *name) {
/*	FILE *special; */
/*	char epsfilename[MAXFILENAME]; */ 	/* 95/Apr/26 */
	long count=0;
/*	int nlen = BUFLEN; */
/*	int nlen = strlen(str); */
	int nlen = sizeof(str);
	int nbytes;

	special = findepsfile(name, 0, "eps");
	if (special == BAD_FILE) return 0; 

	strcpy (str, "%%BeginDocument: ");			/* 96/Sep/14 */
/*	strcat (str, name); */
	strcat (str, FileName);		/* the name discovered by findepsfile(...) */
	strcat (str, "\n");
	passthrough(hDC, str, strlen(str));

	pslength = checkpcform(special, FileName);
	if (pslength > 0) {		/*		it's an EPSF file with TIFF preview */
		fseek(special, psoffset, SEEK_SET);
/*		while (fgets(str, nlen, special) != NULL) */
		if (count + nlen > pslength) nlen = (int) (pslength - count);
		while ((nbytes = _lread(special, str, nlen)) > 0) {
/*		keep only non-comments and DSC comments --- not safe */
/*		binary stream, long lines - Mac EPS etc */
/*			if (*str != '%' || *(str+1) == '%') */ /* removed 96/Aug/22 */
/*			nbytes = strlen(str); */
			if (nbytes > 0) passthrough(hDC, str, nbytes);
			else break;								/* hit some bad stuff */
			count += nbytes;
			if (count >= pslength) break;
/*			when getting within one buffer length of end of PS section ... */
			if (count + nlen > pslength) nlen = (int) (pslength - count);
		}
	}
	else { /* its plain ASCII EPS file (we hope) */
/*		while (fgets(str, BUFLEN, special) != NULL) */
		while ((nbytes = _lread(special, str, nlen)) > 0) {
/*		keep only non-comments and DSC comments --- not safe */
/*		binary stream, long lines - Mac EPS etc */
/*			if (*str != '%' || *(str+1) == '%') */ /* removed 96/Aug/12 */
/*			nbytes = strlen(str); */
			if (nbytes > 0) passthrough(hDC, str, nbytes);
			else break;								/* hit some bad stuff */
			count += nbytes;
		}
	}
	fclose(special);
/*	special = NULL; */
	special = BAD_FILE;
	strcpy(str, "%%EndDocument\n");
	passthrough(hDC, str,strlen(str));		/* 96/Sep/14 */
	return count;
}

/***************************************************************************/

/* incflag >  0  if "included" */
/* incflag == 0 if "overlaid" */
/* incflag <  0  => "included" - but don't read bbox from EPS file again */

/* needshift > 0 if need to shift by (-xll, -yll) */  /* Textures ? */
/* needshift = 0 if no shift needed */				  /* DVI2PS ? */
/* needshift < 0 if need to shift by (-xll, -yur) */  /* DVIALW ? */

/* NOW needshift >  0 if TeX current point at bottom left corner */
/* NOW needshift == 0 if TeX current point at origin of PS coordinates */
/* NOW needshift <  0 if TeX current point at top left corner */

void copyepsfileaux(HDC hDC, char *fname, int incflag, int needshift) {
	long old_h, old_v;
	int devxll, devyll, devxur, devyur;
	POINT SetArr[2];
	int flag;
/*	int devx, devy; */
/*	DWORD CurPos; */
/*	POINT PrintingOffset; */
/*	POINT PhysPageSize; */

	if (bDebug > 1) {	
		sprintf(debugstr, "copyepsfileaux %s pswidth %ld psheight %ld\n",
				FileName, pswidth, psheight);
		OutputDebugString(debugstr);
	} 

/*  SHOW THE TIFF SUBFILE HERE ! */ 
/*  width is pswidth and height is psheight */ /* NOTE: in scaled points! */
	if (pswidth == 0 || psheight == 0) {
		if (bZeroWarned == 0) {
			sprintf(debugstr, "PSwidth (%ld) or PSheight (%ld) is zero",
					pswidth, psheight);
			wincancel(debugstr);
			bZeroWarned++;
		}
		return;						/* avoid getting into trouble */
	}
/*  let that code do things all over again - for now */
	if (incflag != 0) {
		if (needshift < 0) dvi_v += psheight;	/* current point TOP LEFT */
		else if (needshift == 0) {
			dvi_h += epsllx; dvi_v -= epsury; 
		}
	}
	else {							/* overlay instead of include */
		old_h = dvi_h; old_v = dvi_v;
		dvi_h = (long) (depsllx * scaledpoint);
		dvi_v = (long) ((PageHeight - depslly) * scaledpoint);
	}

/*	if (bPrintFlag != 0 && bPassThrough != 0 && IsItPSCRPT(hDC) != 0) {*/
	if (bPrintFlag && bPassThrough && IsItPSCRPT(hDC)) {
/*		dvi_hh = mapx(dvi_h); dvi_vv = mapy(dvi_v); */
		SetArr[0].x = mapx(dvi_h);
		SetArr[0].y = mapy(dvi_v);
		SetArr[1].x = mapx(dvi_h + pswidth);
		SetArr[1].y = mapy(dvi_v - psheight);
		LPtoDP(hDC, SetArr, 2);
		devxll = SetArr[0].x; devyll = SetArr[0].y;
		devxur = SetArr[1].x; devyur = SetArr[1].y;		
		
#ifdef IGNORED
/*		There is a problem here if we have not yet put anything on page */
/*		Then the code appears BEFORE %%Page: ... */
/*		Attempt to force it to emit page setup code 96/Sep/14 */
		if (MarkedPage++ == 0) {
			dvi_hh = mapx(dvi_h); dvi_vv = mapy(dvi_v);	/* ??? */
			MoveToEx (hDC, dvi_hh, dvi_vv, NULL); 
			LineTo (hDC, dvi_hh, dvi_vv);
		} 
#endif

/*		setup state for inserted figure */
		passthrough(hDC, "\n", 1);
/*		Note EPS_PREAMBLE assumes we are at start of new line */
/*		passthrough(hDC, "/dvistate save def mark /showpage{}def\n"); */
/*		UINT is 16 bit in WIN16 and 32 bit in WIN32 */
		flag = LoadString(hInst, (UINT) EPS_PREAMBLE1, str, sizeof(str));
		passthrough(hDC, str, flag);
/*		UINT is 16 bit in WIN16 and 32 bit in WIN32 */
		flag = LoadString(hInst, (UINT) EPS_PREAMBLE2, str, sizeof(str));
		passthrough(hDC, str, flag);
/*		test flag ?	error if zero */

/*		another way to do this (maybe): */
/*		sprintf(str, "%d neg %d %d sub translate\n", 
			PrintingOffset.x, PhysPageSize.y, PrintingOffset.y);
		passthrough(hDC, "iRes 72 div dup neg scale\n"); */
/*		We should now be back in default PostScript coordinate system ! */

/*		sprintf(str, "newpath %d %d moveto %d %d lineto %d %d lineto\n",
			devxll, devyll, devxur, devyll, devxur, devyur);
		passthrough(hDC, str); */		
/*		sprintf(str, "%d %d lineto %d %d lineto closepath stroke\n",
			devxll, devyur, devxll, devyll);
		passthrough(hDC, str); */

/*		set up inserted figure coordinate transformation */
		if (devxll != 0 || devyll != 0) {			/* 96/Sep/12 */
			sprintf(str, "%d %d translate\n", devxll, devyll);
			passthrough(hDC, str, strlen(str));
		}
		sprintf(str, 
		   "%d %d sub %lg %lg sub div %d %d sub %lg %lg sub div scale\n",
		  devxur, devxll, depsurx, depsllx, devyur, devyll, depsury, depslly);
		passthrough(hDC, str, strlen(str));
		if (depsllx != 0.0 || depslly != 0.0) {		/* 96/Sep/12 */
			sprintf(str, "%lg neg %lg neg translate\n", depsllx, depslly);
			passthrough(hDC, str, strlen(str));
		}

		copyfilethrough(hDC, fname);	/* copy EPS file through */

/*		restore state */
/*		passthrough(hDC, "\ncleartomark dvistate restore\n"); */
/*		UINT is 16 bit in WIN16 and 32 bit in WIN32 */
/*		Note EPS_POSTAMBLE assumes we are at start of new line */
		flag = LoadString(hInst, (UINT) EPS_POSTAMBLE, str, sizeof(str));
		passthrough(hDC, str, flag);
/*		test flag ?	error if zero */
	}	/* end of bPrintFlag && bPassThrough && IsItPSCRPT(hDC) case */

	else {/* NOT (bPrintFlag && bPassThrough && IsItPSCRPT(hDC)) */
/*		if (bShowTIFF) */			/* allow suppression of 97/Jan/5 */
		showtiffhere(hDC, fname, pswidth, psheight, 0, 0, 1); /* now try show it */
	}

	if (incflag != 0) {					
		if (needshift < 0) dvi_v -= psheight;	/* current point TOP LEFT */
		else if (needshift == 0) {
			dvi_h -= epsllx; dvi_v += epsury; 
		}
	}
	else {
/*		have old_h and old_v been initialized ??? 98/Mar/23 */
		dvi_h = old_h; dvi_v = old_v;
	}
} 

/* NOTE: showtiffhere assumes current point is BOTTOM LEFT */

/* somewhat wastefully just reads bounding box and closes file again */

/* used by DVIALW to set up BoxHeight and BoxWidth */

/* incflag >  0 =>	include - i.e. reposition according to needshift */
/* incflag == 0 =>	overlay - i.e. use coordinates as in file */
/* incflag <  0 =>	include - but don't read bbox from EPS file again */

/* if needshift >  0 then (xll, yll) is at TeX's current point */
/* if needshift == 0 then (0,0) is at Tex's current point */
/* if needshift <  0 then (xll, yur) is at TeX's current point */

/* NOW needshift >  0 if TeX current point at bottom left corner */
/* NOW needshift == 0 if TeX current point at origin of PS coordinates */
/* NOW needshift <  0 if TeX current point at top left corner */

void copyepsfile (HDC hDC, char *epsfilename, int incflag, int needshift) {
	long deltax, deltay;				/* 98/Nov/27 */
	
/*  try and find and open the file */
	special = findepsfile(epsfilename, 1, "eps");
	if (special == BAD_FILE) return;
/*	shouldn't this trigger an error - NO: already taken care of ... */
	pslength = checkpcform(special, FileName); /* moved here - always done */
	if (incflag > 0) {				/* include - rather than overlay */
		if (needshift != 0) {		/* need to read BBox in file first */
/*	for showtiffhere, need to get *original* width and height 98/Nov/27 */
/*	so read bounding box, but need to preserve modified bbox info */			
			double llxsvd = depsllx;
			double llysvd = depslly;
			double urxsvd = depsurx;
			double urysvd = depsury;
			long widthsvd = pswidth;		/* 99/Jan/3 */
			long heightsvd = psheight;		/* 99/Jan/3 */
			(void) readbbox(special, FileName);		/* NEW - useful ??? */
			pswidth = (long) ((depsurx - depsllx) * scaledpoint); 
			psheight = (long) ((depsury - depslly) * scaledpoint); 
			deltax = (long) ((llxsvd - depsllx) * scaledpoint);
			deltay = (long) ((llysvd - depslly) * scaledpoint);
/*	may be a problem when printing to PS device using pass through 98/Nov/27 */
			depsllx = llxsvd; 
			depslly = llysvd; 
 			depsurx = urxsvd; 
 			depsury = urysvd; 
			pswidth = widthsvd;				/* 99/Jan/3 */
			psheight = heightsvd;			/* 99/Jan/3 */
/*			rewind(special); */
		} 
	}
	fclose(special); 
	special = BAD_FILE;

	if (bDebug > 1) {
		sprintf(debugstr, "copyepsfile w %ld h %ld", pswidth, psheight);
		OutputDebugString(debugstr);
	}

/*	Maybe dont do this shift (for showtiffhere) if PS passthrough 97/Nov/27 */
	if (deltax != 0) dvi_h -= deltax;
	if (deltay != 0) dvi_v += deltay;

	copyepsfileaux(hDC, epsfilename, incflag, needshift);

/*	Maybe dont do this shift (for showtiffhere) if PS passthrough 97/Nov/27 */
	if (deltax != 0) dvi_h += deltax;
	if (deltay != 0) dvi_v -= deltay;
}

/* copy rest of special to get back to rest of DVI code (used by Textures) */

void colontoslash (char *name, char *buff) {
	int c; 
	char *s=name; 
	char *t=buff;
	
	if (*buff == ':') t++;		/* flush initial ':' */
	
	while ((c = *t++) != 0) {
		if (c == ':' && *t != '\\' && *t != '/') *s++ = '\\';
		else *s++ = (char) c;
	} 
	*s = '\0';
}

void startspecial (void) {
/*	fprintf(output, "\ndvispbegin sc "); */
} 

void endspecial (void) {
/*	fprintf(output, "\ndvispend "); */
/*	fprintf(output, "dvispend\n"); */
}

#define MAXCOMPLAIN 31				/* #define MAXCOMPLAIN 127 */

void complainspecial (int input) {
	int c, k=0;
	char *s;
/*	We assume this is longer than MAXCOMPLAIN + */
/*	28 "Don't understand \\special: " + 4 "..." + 1 */

	if (bComplainSpecial == 0) {			/* sanity check */
		flushspecial(input);				/* just flush it then */
		return;
	}

/*	sprintf(str, "Don't understand \\special: "); */
	strcpy(debugstr, "Don't understand \\special: ");
	s = debugstr + strlen(debugstr) - 1;
	(void) an_winseek(input, specstart);	/* start \special over again */
	nspecial = nspecialsav;		
	if (nspecial <= 0) return;	
	c = an_wingetc(input); nspecial--;
	while (nspecial > 0 && c != EOF) {
		k++;
		if (k < MAXCOMPLAIN) *s++ = (char) c;
		c = an_wingetc(input); nspecial--;
	}
	if (k >= MAXCOMPLAIN) strcat(debugstr, "...");
	else *s++ = (char) c;
	*s++ = '\0'; 
	winbadspecial(debugstr);						/* 1996/Aug/22 */
}	

/* first get bbox from file (or file with EPS extension) */
/* then read TIFF from file (or file with TIF extension) */

/* NOW needshift > 0 if TeX current point at bottom left corner */
/* NOW needshift < 0 if TeX current point at top left corner */

int doinsertion (HDC hDC, char *epsfilename, double scale, 
			int incflag, int needshift) {
	double rwi, rhe;
/*	FILE *special; */

/*  It's OK if "overlay" and BBox not found, but can't show box outline ? */

/*	if (setupbbox(epsfilename, 1) == 0) return 0; */		/* NEW */
	if (setupbbox(epsfilename, 1, incflag) == 0) return 0;	/* 93/Oct/31 */

	startspecial();
	rwi = depsurx - depsllx; rhe = depsury - depslly;
	epswidth = (long) (rwi * scale * scaledpoint); 
	epsheight = (long) (rhe * scale * scaledpoint); 
	pswidth = epswidth; psheight = epsheight;		/* ??? */
	
	copyepsfileaux(hDC, epsfilename, incflag, needshift);

	endspecial();	
	return -1;
}

/* Textures style include eps file */ /* added "scaled <double>" ? */
/* need to be able to read EPS file to get BBox */

void readtextures (HDC hDC, int input) { /* Texture style special ? */
	int c;
	double scale=1.0;
/*	char epsfilename[MAXLINE]=""; */	/* 256 bytes --- too much ? */
	char epsfilename[BUFSIZE]="";		/* 128 bytes *//* MAXFILENAME ??? */
	
/*	c = gettoken(input, line, MAXLINE); */
	c = gettoken(input, line, sizeof(line));
	colontoslash(epsfilename, line);	/* deal with Mac font names */
	startspecial(); 
	if (c > 0 && nspecial > 0) {	/* anything left in special ? */
/*		(void) gettoken(input, line, MAXLINE); */
		(void) gettoken(input, line, sizeof(line));
		if (strcmp(line, "scaled") == 0) {
/*			(void) gettoken(input, line, MAXLINE);	 */
			(void) gettoken(input, line, sizeof(line));	
			if(sscanf(line, "%lg", &scale) > 0) {
/*				printf(" SCALE %lg ", scale); */
				if (scale > 33.333) scale = scale/1000.0;
/*				if (scale != 1.0)
					fprintf(output, "%lg dup scale\n", scale); */
			}
		}
	}
/*	(void) doinsertion(hDC, epsfilename, scale, 0); */
	(void) doinsertion(hDC, epsfilename, scale, 1, 1);
/*	copyepsfile(hDC, epsfilename, 1, 1);	*/	/* include and shift */
	endspecial();
	flushspecial(input);					/* flush whatever is left */
}	

/* Textures style "postscript" - direct inclusion of PostScript code */

void copypostscript (int input, int raw) { /* Texture style special ? */
	copystring(input); 
}		/* raw unreferenced */

/* Textures style "postscriptfile" inclusion of PostScript file - no BBox */
/* can this really take a scale factor ? */ 
/* should this neuter stuff ? and use save-restore pair ? */

/* Texture style special ? */	/* really bogus here */
void readpostscript (HDC hDC, int input, int raw) { 
	double scale=1.0;
	char epsfilename[MAXFILENAME]="";
	
/*	(void) gettoken(input, epsfilename, MAXFILENAME); */
	(void) gettoken(input, epsfilename, sizeof(epsfilename));

/*	if (gettoken(input, line, MAXLINE) != 0 && */
	if (gettoken(input, line, sizeof(line)) != 0 &&
		strcmp(line, "scaled") == 0 &&
/*			gettoken(input, line, MAXLINE) != 0 && */
			gettoken(input, line, sizeof(line)) != 0 &&
				sscanf(line, "%lg", &scale) > 0) {
	}

	if (scale > 33.33) scale = scale/1000.0;
/*	copyepsfile(hDC, epsfilename, 0, 0); */		/* no incl and no shift */
	copyepsfile(hDC, epsfilename, 0, 1);		/* no incl and shift */
}		/* raw unreferenced */

void scalebbox (double rwi, double rhe, double xll, double yll,
				double xur, double yur) {
/*  convert from big points (bp) to scaled points (pt) */
	epswidth = (long) (rwi * scaledpoint); 
	epsheight = (long) (rhe * scaledpoint); 
	epsllx = (long) (xll * scaledpoint); 
	epslly = (long) (yll * scaledpoint); 
	epsurx = (long) (xur * scaledpoint);
	epsury = (long) (yur * scaledpoint);
	if (bDebug > 1) {
		sprintf(debugstr, "SCALEBBOX: w %ld h %ld xll %ld yll %ld xur %ld yur %ld",
				epswidth, epsheight, epsllx, epslly, epsurx, epsury);
		OutputDebugString(debugstr);
	}
}

/* Following used by (A), (B), (C) which all specify a BBox in \special */
/* Need to take care of case where this is different from BBox found in EPS */
/* Also provides for clipping request */

void copyepsfileclip (HDC hDC, char *name) {
/*	RECT ClipRect; */
	HDC hOldDC=NULL;
	int flag=0;
	int err;
	double rwi, rhe;
	long old_dvi_h, old_dvi_v;
	int xll, yll, xur, yur;	 /* corners of clipping region - logical coord */

	if (bDebug > 1) {
		sprintf(debugstr, "copyepsfileclip %s", name);
		OutputDebugString(debugstr);
	}
	old_dvi_h = dvi_h; 	old_dvi_v = dvi_v;
/*	try and read BBox from file and -  if there - use it to adjust figure */
/*	if(setupbbox(name, 0) == 0) {	*/	/* not found - assume same as given */
	if (setupbbox(name, 0, 1) == 0) {	/* 93/Oct/31 */
		epsllx = psllx; epslly = pslly;
		epsurx = psurx; epsury = psury;
		epswidth = pswidth; epsheight = psheight;	/* ? */
	}
	else {							/* did find BBox - use it */
/*	BBox now in depsllx, depslly, depsurx, depsury (in points) */
		rwi = depsurx - depsllx; rhe = depsury - depslly;
		scalebbox(rwi, rhe, depsllx, depslly, depsurx, depsury);
/*	This test too fussy - does not take into account round off ... 95/Apr/13 */
/*		if (epsllx != psllx || epslly != pslly || 
			epsurx != psurx || epsury != psury) */
		if (epsllx < psllx-1 || epsllx > psllx+1 ||
			epslly < pslly-1 || epslly > pslly+1 || 
			epsurx < psurx-1 ||	epsurx > psurx+1 ||
			epsury < psury-1 || epsury > psury+1) {
/*			if (bDebug > 1) { 
				sprintf(debugstr, 
						"SPECIAL: %ld %ld %ld %ld\nEPSFILE: %ld %ld %ld %ld\n",
						psllx, pslly, psurx, psury,
							epsllx, epslly, epsurx, epsury);
				OutputDebugString(debugstr); 
			} */ /* debugging 95/April/13 */
			flag = 1;		/* indicate need to undo this again */
		}
/*	BBox now in epsllx, epslly, epsurx, epsury (in scaled points) */
	}
/*	if (bDebug > 1) {
		sprintf(debugstr, "copyepsfileclip %s pswidth %ld psheight %ld flag %d\n",
				name, pswidth, psheight, flag);
		OutputDebugString(debugstr);
	} */ 	/* debugging 95/April/13 */
	if (clipflag != 0) {
		xll = mapx(dvi_h); yur = mapy(dvi_v);
		xur = mapx(dvi_h + pswidth); yll = mapy(dvi_v + psheight);
/*		sprintf(str, "CLIPPING: %d %d %d %d", xll, yll, xur, yur);
		wincancel(str); */
/*		(void) GetClipBox(hDC, &ClipRect);	
		sprintf(str, "REGION: %d %d %d %d", 
			ClipRect.left, ClipRect.bottom, ClipRect.right, ClipRect.top);
		wincancel(str); */
		hOldDC = hDC;
		hDC = GetDC(hwnd);
		if (hDC == NULL) {
			wincancel("Out of Device Contexts");
			hDC = hOldDC;
			hOldDC = NULL;
		}
		else if (bCopyFlag == 0)  /* ??? 92/Feb/18 */
			(void) SetMapMode(hDC, MM_TWIPS); 
		err = IntersectClipRect(hDC, xll, yur, xur, yll);
/*		sprintf(str, "err %d", err);	wincancel(str); */
		if(err == 0) wincancel("Error in Clip Rectangle");
	}

	if (flag != 0) {	/* when BBox of \special <> BBox of EPS file */
/*		if (bDebug  > 1) {	
sprintf(debugstr, "pswidth %ld (%ld) psheight %ld (%ld) rwi %lg rhe %lg\n",
				pswidth, psheight, psurx - psllx, psury - pslly, rwi, rhe);
			OutputDebugString(debugstr);
		} */ /* debugging 95/April/13 */
/* next test is possibly too fussy given round off problems ... 95/Apr/13 */
/*		if (pswidth != psurx - psllx || psheight != psury - pslly) */
		if (pswidth < (psurx-psllx)-1 || pswidth > (psurx-psllx)+1 ||
			psheight < (psury-pslly)-1 || psheight > (psury-pslly)+1) {
/*	scale width and height read from EPS file */
/*	not sure this is right, since rwi, rhe are in PS pts */
/*	rwi and rhe may not be iniitialized ??? 98/Mar/23 */
			if (psurx != psllx)		/* 93/Mar/27 */
/*				pswidth = (long) (rwi * pswidth / (psurx - psllx)); */
				pswidth = (long) (rwi * pswidth / (psurx - psllx)
								  * scaledpoint); /* fix 95/Apr/13 */
			if (psury != pslly)		/* 93/Mar/27 */
/*				psheight = (long) (rhe * psheight / (psury - pslly)); */
				psheight = (long) (rhe * psheight / (psury - pslly)
								  * scaledpoint); /* fix 95/Apr/13 */
/*			if (bDebug  > 1) {
				sprintf(debugstr, "pswidth %ld psheight %ld\n", pswidth, psheight);
				OutputDebugString(debugstr);
			} */ 	/* debugging 95/April/13 */
			if (psurx != psllx)		/* 93/Mar/27 */
/*				dvi_h += (long) (rwi * (epsllx - psllx) / (psurx - psllx)); */
				dvi_h += (long) (rwi * (epsllx - psllx) / (psurx - psllx)
								  * scaledpoint); /* fix 95/Apr/13 */
			if (psury != pslly)		/* 93/Mar/27 */
/*				dvi_v -= (long) (rhe * (epsury - psury) / (psury - pslly)); */
				dvi_v -= (long) (rhe * (epsury - psury) / (psury - pslly)
								  * scaledpoint); /* fix 95/Apr/13 */
		}
		else {	/* no scaling - use width and heigth read from EPS file */
			pswidth = epswidth; psheight = epsheight;
			dvi_h += (epsllx - psllx);		
			dvi_v -= (epsury - psury);
		}
	}
/*	if (bDebug > 1) {
		sprintf(debugstr, "copyepsfileclip %s pswidth %ld psheight %ld flag %d\n",
				name, pswidth, psheight, flag);
		OutputDebugString(debugstr);
	} */ 	/* debugging 95/April/13 */
/*	WORK (STILL) IN PROGRESS HERE */

	copyepsfile(hDC, name, -1, -1);	/* to avoid rereading BBox */

	dvi_h = old_dvi_h; 	dvi_v = old_dvi_v;
/*	if (flag != 0) {
		if (flag < 0) {
			dvi_h -= (epsllx - psllx);		
			dvi_v += (epsury - psury);
		}
		else {
			dvi_h -= (epsllx - psllx);		
			dvi_v += (epsury - psury);
		}
	} */

	if (clipflag != 0) {
		if (hOldDC != NULL) {
			(void) ReleaseDC(hwnd, hDC);
			hDC = hOldDC;
		}
	}
}

/*  Try Andrew Trevorrow's OzTeX and Psprint Vax VMS syntax */
/*  returns zero if this doesn't work */
/*  There may be a problem if /magnification != 1000 */
/*  - since OzTeX uses absolute 72 per inch scaling ??? */

/* can come here from several places when other things fail */
/* need to be able to read EPS file, to get bbox */

int readandrew (HDC hDC, int input) {
	double rwi, rhe;
	char epsfilename[MAXFILENAME]="";

	(void) an_winseek(input, specstart);	/* start \special over again */
	nspecial = nspecialsav;				/* and restore length */
/*	if (gettoken(input, epsfilename, MAXFILENAME) == 0) */
	if (gettoken(input, epsfilename, sizeof(epsfilename)) == 0) {
		flushspecial(input);
		return 0;
	}
/*	if ((special = findepsfile(epsfilename, 0, "eps")) == NULL) {
		flushspecial(input);
		return 0;
	}
	fclose (special);
	special = NULL;	*/
	startspecial();
	copystring(input);				/* copy rest of special verbatim */

/*	if (setupbbox(epsfilename, 1) == 0) return 0;		*/
	if (setupbbox(epsfilename, 1, 1) == 0) return 0;	/* 93/Oct/31 */

/*	if ((special = findepsfile(epsfilename, 1, "eps")) == NULL) return 0;
	pslength = checkpcform(special, epsfilename);
	if (readbbox(special, epsfilename) == 0) {
		sprintf(str, "BBox not found in %s ", epsfilename);
		wincancel(str);
		fclose(special);
		special = NULL;
		return 0;
	}  
	fclose(special); 
	special = NULL; */

	rwi = depsurx - depsllx; rhe = depsury - depslly;
	scalebbox(rwi, rhe, depsllx, depslly, depsurx, depsury); 
	pswidth = epswidth; psheight = epsheight;
	dvi_v += epsheight;			/* ??? */ 
/*	dvi_h += epsllx; dvi_v -= epsury; */
	copyepsfile(hDC, epsfilename, 1, 0); 	/* include & no shift */ /* NEW */
/*	dvi_h -= epsllx; dvi_v += epsury; */
	dvi_v -= epsheight;			/* ??? */ 

/*	(void) doinsertion(hDC, epsfilename, 1.0, 1, 0); */			/* OLD */
/*	fclose(special); */
/*	copyepsfileaux(hDC, epsfilename, 1, 0); */ /* OLD */
	endspecial();
/*	fclose (special);	*/			/* ??? NEW ??? */
	return -1;
}

/* Separator is ` ' (space) */ /* DVIALW style special ? */
void readdvialw (HDC hDC, int input) { 
	char epsfilename[MAXFILENAME]="";
	long flitpos=0;			/* place in file where literal was */
	long fendspec;				/* saved pointer to end of special */
	int includeflag=1;			/* zero => overlay, otherwise => include */
	int firsttime=1;			/* already read first token */
	int fileflag=0;				/* non-zero include or overlay */
	int bboxflag=0;				/* bounding box specified */

/*	if (traceflag != 0) printf("Processing DVIALW style special (?)\n"); */
	while (nspecial > 0) {		/* gather up information first */
/*		if (firsttime != 0) firsttime = 0; 
		else if (getalphatoken(input, line, MAXLINE) == 0) break; */ 
		if (firsttime == 0)
/*			if (getalphatoken(input, line, MAXLINE) == 0) break; */
			if (getalphatoken(input, line, sizeof(line)) == 0) break; 
		if (strcmp(line, "language") == 0) { 
/*			(void) getstring(input, line, MAXLINE); */
			(void) getstring(input, line, sizeof(line));
			if (strcmp(line, "PS") == 0 ||
				strcmp(line, "PostScript") == 0) {
				/* we like PS, so no need to do anything ! */
			}
			else {
				complainspecial(input);
				break; 
			}
		}
		else if (strcmp(line, "include") == 0) {
/*			(void) getstring(input, epsfilename, MAXFILENAME); */
			(void) getstring(input, epsfilename, sizeof(epsfilename));
			includeflag = 1; fileflag = 1;
		}
		else if (strcmp(line, "overlay") == 0) {
/*			(void) getstring(input, epsfilename, MAXFILENAME); */
			(void) getstring(input, epsfilename, sizeof(epsfilename));
			includeflag = 0;  fileflag = 1;
		}
		else if (strcmp(line, "literal") == 0) {
				 flitpos = an_wintell(input);	/* remember where this was */
				 nliteral = nspecial;
		}
/*		GRAPHICS and OPTIONS not yet defined - so flag as errors */		
		else if (strcmp(line, "boundingbox") == 0) {
/*	actually, this may involve TeX dimensions, see decodeunits? */
/*  - that seems truly bizarre, so ignore that possibility */
/*			(void) getstring(input, line, MAXLINE); */
			(void) getstring(input, line, sizeof(line));
			if (sscanf(line, "%lg %lg %lg %lg", 
				&depsllx, &depslly, &depsurx, &depsury) == 4)
/*	this should override bounding box in eps file if given */
				bboxflag = 1;
			else {
				sprintf(str, "Don't understand DVIALW bounding box: %s", 
					line);
				wincancel(str);
/*				errcount(); */
			}
		}
		else if (strcmp(line, "message") == 0) {
/*			(void) getstring(input, line, MAXLINE); */
			(void) getstring(input, line, sizeof(line));
/*			printf("%s\n", line); */	
			wincancel(line);			/* show message ! */
		}
		else if (bAllowAndrew == 0 || firsttime == 0 ||
				readandrew(hDC, input) == 0) { /*  try OzTeX ? */
/*			if (bComplainSpecial != 0) */
/*				sprintf(str, 
					"Unrecognized DVIALW special keyword: %s ", line);
				wincancel(str); */
			complainspecial(input);
/*			} */
			flushstring(input);		/*	errcount(); */
		}
		if (nspecial == 0) break;
		else skiptocomma(input);	/* look for next key value pair */
		if (firsttime != 0) firsttime = 0; 
	}
/*  not implemented: `graphics', `options', `position' */
/*	now actually do something */
/*		fprintf(output, "\ndvispbegin "); */

/*		include or overlay ? */
/*		if (includeflag != 0)  fprintf(output, "sc ");
		else  fprintf(output, "so ");	 */				
/*	} */
/*  sort of futile stuff to do here in DVIWindo ... */
	if (flitpos != 0) {			/* literal before included file if any */
		fendspec = an_wintell(input);	/* remember here (end of special) */
		(void) an_winseek(input, flitpos);	/* go back to where literal was */
		nspecial = nliteral;			/* reset nspecial */
/*		putc('\n', output); */
		copystring(input);
		(void) an_winseek(input, fendspec);	/* back to end of special */
		nspecial = 0;
	}
	if (fileflag != 0) {
/* should take into account given bounding box if bboxflag != 0 ? */
		if (*epsfilename == '\0')
			wincancel("Blank file name");
		if (includeflag != 0) {
			if (bboxflag == 0) { 
				(void) doinsertion(hDC, epsfilename, 1.0, 1, -1); 
/*				copyepsfile(hDC, epsfilename, 1, -1); */
			}
			else {	/* do have a bbox */
/*				fprintf(output, "%lg neg %lg neg translate\n", xll, yur); */
/*				copyepsfile(hDC, epsfilename, 1, 0); */
				copyepsfile(hDC, epsfilename, 1, -1);	/* ??? */	
			}
		}
		else { 	/* overlay instead of insertion */
/*			copyepsfile(hDC, epsfilename, 0, 0);	*/ /* ??? */
			(void) doinsertion(hDC, epsfilename, 1.0, 0, 0);
		}
	}
/*	if (fileflag != 0) */
	endspecial();
}

/* DVIALW also sets up PaperHeight = PageHeight PaperWidth = PageWidth OK */
/* DVIALW also sets up CurrentX and CurrentY OK */
/* DVIALW also sets up BoxHeight and BoxWidth OK if given */
/* DVIALW also sets up BoxHeight and BoxWidth need to read file */
/* DVIALW allows specification of both an included and an overlay file ? */
/* expects SB, SE, BPtoPX, FRAME, RESIZE and other stuff to be defined! */

/*	clip the epsf illustration if requested */
/* This is now redundant
void doclip(FILE *output) { 
	fprintf(output, " doclip ");
} */

/* Set things up for included figure (Trevor Darrell style) */
/*		these parameters are all in DVI units ! */
/*		this ignores BBox in file - uses bounding box in pstext special */

/* This is now redundant
void starttexfig(long pswidth, long psheight,
			long psllx, long pslly, long psurx, long psury) {
	if (psurx == psllx || psury == pslly) {
		sprintf(stderr, "Zero area BoundingBox %ld %ld %ld %ld\n",
			psllx, pslly, psurx, psury);
		wincancel(str);
	}
	fprintf(output, "\n%ld %ld %ld %ld %ld %ld startTexFig",
		pswidth, psheight, psllx, pslly, psurx, psury); 
} */

/* This saves up size and bounding box info for PSFIG style insertions */

void starttexfig(char *buff) {
	if (sscanf(buff, "%ld %ld %ld %ld %ld %ld", 
		&pswidth, &psheight, &psllx, &pslly, &psurx, &psury) < 6) { 
		sprintf(str, "Don't understand: %s", line);
		wincancel(str);
	}
	clipflag = 0;
}

/* This is now redundant 
void endtexfig(void) {
} */

void cantfind (HFILE input, char *s, char *line) { /* complain about missing field */
	sprintf(str, "Can't find `%s' (in `%s')\n", s, line);
	wincancel(str);
	flushspecial(input); 
/*	errcount(); */
}

/* output verbatim what is in buffer - except leading white space */
void verbout(char *s) { /* strip leading spaces */
	while (*s == ' ') s++;
/*	fprintf(output, "\n%s ", s); */	/* \n there for paranoia ? */
}

/* Separator is `='			dvi2ps style */
void readdvi2ps(HDC hDC, HFILE input) { 
	char epsfilename[MAXFILENAME]="";
/*	int includeflag=1;	*/		/* always include instead of overlay */
	int firsttime=1;			/* already read first token */
	int psepsfstyle=0;			/* non-zero =>  EPSF style */
	int clipflag=0;				/* for EPSF style */
	double rwi=0.0, rhe=0.0;	/* for EPSF style */
	double xll, yll, xur, yur;	/* temporary */
/*  parameters old DVI2PS style */
	double hsize=0.0, vsize=0.0, hoffset=0.0, voffset=0.0;
	double hscale=1.0, vscale=1.0, rotation=0.0;

/*	if (traceflag != 0) printf("Processing DVI2PS style special\n"); */

	while (nspecial > 0) {		/* gather up parameters */
		if (firsttime != 0) firsttime = 0;
/*		else if (getalphatoken(input, line, MAXLINE) == 0) break; */
		else if (getalphatoken(input, line, sizeof(line)) == 0) break;
/*		deal with new PSFIG DVI2PS style useage */
		if (strcmp(line, "pstext") == 0) {
/*			if (verbatimflag != 0) copystring(input); */
/*			if (scanspecial(input, line, MAXLINE) == MAXLINE) */
			if (scanspecial(input, line, sizeof(line)) == sizeof(line)) { 
				if (verbatimflag != 0) {		/* long pstext= verbatim */
					(void) an_winseek(input, fliteral);
					nspecial = nliteral;
					flushspecial(input);
/*					copystring(input);	*/		/* revscl - forscl PSTEXT */
				}
				else complainspecial(input);	/* TOO DAMN LONG ! */
			}
/*			check whether this is a startTexFig */
			else if (psfigstyle == 0 &&	/* avoid repeat  startTexFig */
				strstr(line, "startTexFig") != NULL) {
				starttexfig(line);
/*				verbout(line);	*/				/* copy verbatim output */
				psfigstyle=1;					/* note we are inside */
			}
			else if (psfigstyle != 0 &&		/* avoid repeat  endTexFig */
				strstr(line, "endTexFig") != NULL) {
/*				verbout(line); */
				psfigstyle=0;
			}
			else if (psfigstyle != 0 && /* otherwise makes no sense */
				strstr(line, "doclip") != NULL) {
/*				clip the epsf illustration if requested */
/*				verbout(line); */
			}
/*			maybe just copy the darn thing to the output ??? */
			else if (verbatimflag != 0) verbout(line); 
			else complainspecial(input);
			flushspecial(input);		/* ignore the rest */
		}	/* end of PSTEXT= code */
/*		kludge to allow insertion of header information - NO PROTECTION ! */
/*		ignore \special designed for passing info to pdfmark */
		else if (_strcmpi(line, "header") == 0 ||
				 _strcmpi(line, "headertext") == 0  ||
				 strcmp(line, "DSCheader") == 0 ||		/* 95/July/15 */
				 strcmp(line, "DSCtext") == 0 ||		/* 95/July/15 */
				 strcmp(line, "papersize") == 0		 	/* 98/June/28 */
/* flush the following two after switching to PDF: 96/July/4 */
/*				 _strcmpi(line, "keywords") == 0 || */	/* 96/May/10 */
/*				 strcmp(line, "BBox") == 0 */			/* 96/May/4 */
/*				 _strcmpi(line, "PDF") == 0 */			/* 96/July/4 */
				) {
				flushspecial(input);
		}
/*		kludge to change to PS default coordinates - NO PROTECTION ! */
		else if (strcmp(line, "verbatim") == 0 
/*				|| strcmp(line, "VERBATIM") == 0 */
				) {
/*				fprintf(output, "\nrevscl "); */
/*				if (preservefont != 0)  fprintf(output, "currentfont "); */
/*				putc('\n', output); */
				copystring(input);
/*				if (preservefont != 0) fprintf(output, "setfont "); */
/*				fprintf(output, "forscl ");	*/ /* switch to DVI coord */
		}
/*		deal with old PSFIG DVI2PS style useage */
		else if (strcmp(line, "psfile") == 0) {
/*			if (gettoken(input, epsfilename, MAXFILENAME) == 0) */
			if (gettoken(input, epsfilename, sizeof(epsfilename)) == 0)
				cantfind(input, line, epsfilename);
			else psepsfstyle=0;
		}
		else if (strcmp(line, "hsize") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &hsize) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=0;
		}
		else if (strcmp(line, "vsize") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &vsize) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=0;
		}
		else if (strcmp(line, "hoffset") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &hoffset) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=0;
		}
		else if (strcmp(line, "voffset") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &voffset) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=0;
		}
		else if (strcmp(line, "hscale") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &hscale) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=0;
		}
		else if (strcmp(line, "vscale") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
					sscanf(moreline, "%lg", &vscale) == 0) 
				cantfind(input, line, moreline);
			else psepsfstyle=0;
		}
		else if (strcmp(line, "rotation") == 0
				|| strcmp(line, "angle") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &rotation) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=0;
		}
/* Now for Rokicki EPSF.TEX style */
		else if (strcmp(line, "PSfile") == 0) { /* EPSF style - note uc/lc */
/*			if (gettoken(input, epsfilename, MAXFILENAME) == 0) */
			if (gettoken(input, epsfilename, sizeof(epsfilename)) == 0) 
				cantfind(input, line, moreline);
			else psepsfstyle=1;
		}
		else if (strcmp(line, "llx") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &depsllx) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=1;
		}
		else if (strcmp(line, "lly") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &depslly) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=1;
		}
		else if (strcmp(line, "urx") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &depsurx) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=1;
		}
		else if (strcmp(line, "ury") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &depsury) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=1;
		}
		else if (strcmp(line, "rwi") == 0) {
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &rwi) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=1;
		}
/*		else if (strcmp(line, "rhe") == 0) */		/* ??? */
		else if (strcmp(line, "rhe") == 0 ||
			     strcmp(line, "rhi") == 0) {		/* 1994/Mar/1 */
/*			if (gettoken(input, line, MAXLINE) == 0 || */
			if (gettoken(input, moreline, sizeof(moreline)) == 0 ||
				sscanf(moreline, "%lg", &rhe) == 0) 
					cantfind(input, line, moreline);
			else psepsfstyle=1;
		}				
		else if (strcmp(line, "clip") == 0) clipflag = 1;	/* 1994/Mar/1 */
		else complainspecial(input);			/* not one of the above */
	}
	if (bDebug > 1) {
		sprintf(debugstr,
		"PSfile %s BB %lg %lg %lg %lg rwi %lg rhe %lg clip %d", 
			epsfilename, depsllx, depslly, depsurx, depsury, rwi,
				rhe, clipflag);
		OutputDebugString(debugstr);
	}
/*	now actually go and do something - unless `pstext=' style */
/*	- that is, if `psfile=' and filename present not empty */
	if (*epsfilename == '\0') return;	/* NEW:  no file specified */
/*	NOTE possible conflict: two different uses of psfile= */
/*	one is old style DVI2PS, the other is by Rokicki's DVIPS */
	if (psfigstyle != 0)  {						/* old style DVI2PS */
/*		wincancel("DVI2PS old style Anthony Li"); */
/*		actually, now do nothing except copy the file ! */
/*		copyepsfile(hDC, epsfilename, 1, 0); *//* include & no shift */
/*		copyepsfile(hDC, epsfilename, 1, -1); *//* include & shift */
		copyepsfileclip(hDC, epsfilename); /* include &  shift */
	}
/*  NOW: Tom Rokicki's DVIPS style (EPSF) ? */
	else if (psepsfstyle != 0) {
		if (rwi != 0.0 && rhe == 0.0) {
			if (depsurx != depsllx)	
				rhe = rwi * (depsury - depslly) / (depsurx - depsllx);
		}
		else if (rwi == 0.0 && rhe != 0.0) {
			if (depsury != depslly) 
				rwi = rhe * (depsurx - depsllx) / (depsury - depslly);
		}
		rwi = rwi / 10.0; rhe = rhe / 10.0;		/* units are tenth pt */
		if (rwi == 0.0) rwi = depsurx - depsllx;
		if (rhe == 0.0) rhe = depsury - depslly;
		startspecial();
/*		need to set up sizes and what not ??? */ /* 72.27 / 72 */
		scalebbox(rwi, rhe, depsllx, depslly, depsurx, depsury);
		pswidth = epswidth; psheight = epsheight;
/*		need to set up sizes and what not ??? */
/*		dvi_v -= epsheight;	 */
/*		copyepsfile(hDC, epsfilename, 0, 0); */ /* changed back 99/Jan/3 */
/*		because we don't want bbox in EPS overriding scaling in \special */
		copyepsfile(hDC, epsfilename, 1, 1); 
/*		dvi_v += epsheight; */
		endspecial();
		flushspecial(input);
	}
/*	NOW old UNIX DVI2PS (psadobe) ? */
	else {
/*		wincancel("DVI2PS old style Mark Senn"); */
		startspecial();
/*		if (hoffset != 0.0 || voffset != 0.0) 
			fprintf(output, "%lg %lg translate ", hoffset, voffset); */
/* NOTE: in Rokicki DVIPS style, scale is given as percentage ... */
		if (hscale > 8.0) hscale = hscale/100.0;	/* Rokicki style ? */
		if (vscale > 8.0) vscale = vscale/100.0;	/* Rokicki style ? */
/* Using rather arbitrary threshold above  ... */
/*		if (hscale != 1.0 || vscale != 1.0)  
			fprintf(output, "%lg %lg scale ", hscale, vscale); */
/*		if (rotation != 0.0)
			fprintf(output, "%lg rotate ", rotation); */
/*		if (hsize != 0.0 && vsize != 0.0) 
			fprintf(output, "0 0 %lg %lg clipfig\n", hsize, vsize);  
		else if (hsize != 0.0 || vsize != 0.0) {
			wincancel("Specify both HSIZE and VSIZE or neither"); 
			errcount(); 
		} */
/*		in this case figure size NOT available from \special */ 
/*		so need to read %%BoundingBox  comment in file */

/*		if (setupbbox(epsfilename, 1) == 0) return; */	/* NEW */
		if (setupbbox(epsfilename, 1, 1) == 0) return;	/* 93/Oct/31 */

/*		if ((special = findepsfile(epsfilename, 1, "eps")) == NULL) return;
		pslength = checkpcform(special, epsfilename);
		if (readbbox(special, epsfilename) == 0) {
			sprintf(str, "%%%%BoundingBox not found in %s ", epsfilename);
			wincancel(str);
			fclose(special);
			special = NULL;
			return;
		}  
		fclose(special);
		special = NULL; */
/*		this may take some work */ /* also, rotation not implemented */
		xll = depsllx * hscale + hoffset;
		yll = depslly * vscale + voffset;
		xur = depsurx * hscale + hoffset; 
		yur = depsury * vscale + voffset; 
		rwi = xur - xll; rhe = yur - yll; 
		scalebbox(rwi, rhe, xll, yll, xur, yur);  
		pswidth = epswidth; psheight = epsheight;
		dvi_v += epsheight; 			/* ??? */
/*		dvi_h += epsllx; dvi_v -= epslly; */
		copyepsfile(hDC, epsfilename, 1, 0);  /* include & no shift */
/*		dvi_h -= epsllx; dvi_v += epslly; */
		dvi_v -= epsheight;			/* ??? */ 
		endspecial();		
	}
	flushspecial(input);		/* ignore the rest ? */
}

/* example: \special{picture screen0 scaled 500} */
/* To do this, one would need to extract a bit-map */

void readpicture(HFILE input) {
/*	fprintf(stderr, " Ignoring Textures special: %s", line); */
/*	flushspecial(input); */
	complainspecial(input);
}

/************************ support for CTM transformations ****************/

#ifdef AllowCTM

int CTMstackindex= 0;		/* CTM stack pointer */
							/* should reset at top of page */
							/* should check at bottom of page */
#define MAXCTMSTACK 32

XFORM CTM;

XFORM CTMstack[MAXCTMSTACK];

void identityCTM (XFORM *M) {				/* identity matrix */
	M->eM11 = M->eM22 = (float) 1.0;
	M->eM12 = M->eM21 = (float) 0.0;
	M->eDx = M->eDy = (float) 0.0;
}

void DebugShowCTM (HDC hDC, char *msg) {					// debugging output
	XFORM W;
	if (bDebug > 1) {
		sprintf(debugstr, "%s: [%g %g %g]  [%g %g %g]",
				msg, CTM.eM11, CTM.eM12, CTM.eDx, CTM.eM21, CTM.eM22, CTM.eDy);
		OutputDebugString(debugstr);
		GetWorldTransform(hDC, &W);
		sprintf(debugstr, "%s: [%g %g %g] [%g %g %g]",
				msg, W.eM11, W.eM12, W.eDx, W.eM21, W.eM22, W.eDy);
		OutputDebugString(debugstr);

	}
}

/* c = a * b */ /* c is the new CTM, b is the old CTM */
/* c may be the same matrix as b, since it uses a temp for computation */

int multiplyCTM (XFORM *pc, XFORM *pa, XFORM *pb) {
	XFORM t;
	double det;
	int flag=0;

	det = pa->eM11 * pa->eM22 - pa->eM12 * pa->eM21;
/*	avoid exact equality test ? */
/*	if (det == 0.0) wincancel("CTM singular"); */
	if (det < 0.0000001 && det > - 0.0000001) {
		flag = -1;
/*		wincancel("CTM singular"); */
		sprintf(debugstr, "CTM singular %lg %lg %lg %lg\n",
				pa->eM11, pa->eM12, pa->eM21, pa->eM22);
		wincancel(debugstr);
	}
/*	c11 = a11 * b11 + a12 * b21 */
	t.eM11 = pa->eM11 * pb->eM11 + pa->eM12 * pb->eM21;
/*	c12 = a11 * b12 + a12 * b22 */
	t.eM12 = pa->eM11 * pb->eM12 + pa->eM12 * pb->eM22; 
/*	c21 = a21 * b11 + a22 * b21 */
	t.eM21 = pa->eM21 * pb->eM11 + pa->eM22 * pb->eM21;
/*	c22 = a21 * b12 + a22 * b22 */
	t.eM22 = pa->eM21 * pb->eM12 + pa->eM22 * pb->eM22;
/*	c31 = a31 * b11 + a32 * b21 + b31 */
	t.eDx = pa->eDx * pb->eM11 + pa->eDy * pb->eM21 + pb->eDx;
/*	c32 = a31 * b12 + a32 * b22 + b32 */
	t.eDy = pa->eDx * pb->eM12 + pa->eDy * pb->eM22 + pb->eDy;
//	if translation happens to be almost zero - make it zero
	if (t.eDx < 0.001 && t.eDx > -0.001) t.eDx = 0.0;
	if (t.eDy < 0.001 && t.eDy > -0.001) t.eDy = 0.0;
	*pc = t;		/* use temp t so can write back into a or b */
	return flag;
}

int invertCTM(XFORM *pb, XFORM *pa) {		/* b = a^{-1} */
	XFORM t;
	double det;

	det = pa->eM11 * pa->eM22 - pa->eM12 * pa->eM21;
/*	avoid exact equality test ? */
/*	if (det == 0.0) fprintf(errout, " CTM singular\n"); */
	if (det < 0.0000001 && det > - 0.0000001) {
/*		wincancel("CTM singular"); */
		sprintf(debugstr, "CTM singular %lg %lg %lg %lg\n",
				pa->eM11, pa->eM12, pa->eM21, pa->eM22);
		wincancel(debugstr);
/*		pb->eM11 = pb->eM22 = (float) 1.0; */
/*		pb->eM12 = pb->eM21 = (float) 0.0; */
/*		pb->eDx = pb->eDy = (float) 0.0; */
		identityCTM(pb);
		return -1;
	}
	t.eM11 = (float) ( pa->eM22 / det);
	t.eM12 = (float) ( - pa->eM12 / det);
	t.eM21 = (float) ( - pa->eM21 / det);
	t.eM22 = (float) ( pa->eM11 / det);
	t.eDx = (float) ( (pa->eM21 * pa->eDy - pa->eM22 * pa->eDx) / det);
	t.eDy = (float) ( (pa->eM12 * pa->eDx - pa->eM11 * pa->eDy) / det);
	*pb = t;								/* so can write back in place */

	return 0;
}

/* Reset CTM at top of page */	/* Reset at bottom of page */
/* This would be a good place to implement global rotation of page !!! */

void resetCTM (HDC hDC) {					/* CALL AT START OF PAGE */
	CTMstackindex = 0;
	identityCTM(&CTM);						/* set to identity matrix */
/*	if (bUseAdvancedGraphics == 0) return; */	/* don't use it */
	if (bAdvancedGraphicsMode == 0) return; 	/* don't bother not set ! */
/*	This will fail in MetaFile context ? Need enhanced MetaFile ? */
	if (bCopyFlag) return;					/* don't bother for now ! */
	bAdvancedGraphicsMode = 0;
	if (bUseAdvancedGraphics == 0) return;	/* don't bother ... */
	if (SetGraphicsMode(hDC, GM_COMPATIBLE) == 0) {	/* start in GM_COMPATIBLE */
		SetWorldTransform(hDC, &CTM);		/* reset to identity */
		ModifyWorldTransform(hDC, &CTM, MWT_IDENTITY);	/* desparation */
		if (SetGraphicsMode(hDC, GM_COMPATIBLE) == 0) { /* fail ? */
			if (bDebug) {
				if (bDebug > 1)	OutputDebugString("GM_COMPATIBLE failed\n");
				else wincancel("GM_COMPATIBLE failed\n");
			}
			bUseAdvancedGraphics = 0;		/* switch it off permanent */
		}
	}
}

int AdvancedGraphicsOn(HDC hDC) {
	if (bUseAdvancedGraphics == 0) return -1;	/* don't use it */
	if (bAdvancedGraphicsMode != 0) return 0;	/* already in mode */
	if (bCopyFlag) return -1;	/*	This will fail in MetaFile context ? */
	if (SetGraphicsMode(hDC, GM_ADVANCED) == 0) { /* fail ? */
		if (bDebug) {
			if (bDebug > 1)	OutputDebugString("GM_ADVANCED failed\n");
			else wincancel("GM_ADVANCED failed\n");
		}
		bUseAdvancedGraphics = 0;				/* switch it off permanent */
		return -1;								/* so won't try again */
	}
/*	else bAdvancedGraphicsMode = 1; */			/* now we are in */
	bAdvancedGraphicsMode = 1;					/* now we are in */
	return 0;
}

int AdvancedGraphicsOff(HDC hDC) {
	if (bUseAdvancedGraphics == 0) return -1;	/* don't use it */
	if (bAdvancedGraphicsMode == 0) return 0;	/* mode already off */
	if (bCopyFlag) return -1; /* This will fail in MetaFile context ? */
	if (SetGraphicsMode(hDC, GM_COMPATIBLE) == 0) { /* fail ? */
		XFORM xf;					/* Use CTM ? Which should be identity */
		identityCTM(&xf);
		SetWorldTransform(hDC, &xf);
		ModifyWorldTransform(hDC, &xf, MWT_IDENTITY);	/* desparation */
		if (SetGraphicsMode(hDC, GM_COMPATIBLE) == 0) { /* fail ? */
			if (bDebug) {
				if (bDebug > 1)	OutputDebugString("GM_COMPATIBLE failed\n");
				else wincancel("GM_COMPATIBLE failed\n");
			}
			return -1;
		}
	}
/*	else bAdvancedGraphicsMode = 0; */
	bAdvancedGraphicsMode = 0;					/* now we are out */
	return 0;
}

int SetWorld (HDC hDC, XFORM CTM) {				/* absolute CTM setting */
	if (bUseAdvancedGraphics == 0) return -1;	/* 97/Mar/28 */
	if (SetWorldTransform(hDC, &CTM) == 0) {
		if (bDebug) {
			if (bDebug > 1)	OutputDebugString("SetWorldTransform failed\n");
			else wincancel("SetWorldTransform failed\n");
//			ShowLastError ???
		}
		return -1;							/* failed */
	}
	return 0;
}

int ModifyWorld (HDC hDC, XFORM M) {			/* relative CTM setting */
	if (bUseAdvancedGraphics == 0) return -1;	/* 97/Mar/28 */
	if (ModifyWorldTransform(hDC, &M, MWT_LEFTMULTIPLY) == 0) {
		if (bDebug) {
			if (bDebug > 1)	OutputDebugString("ModifyWorldTransform failed\n");
			else wincancel("ModifyWorldTransform failed\n");
//			ShowLastError ???
		}
		return -1;							/* failed */
	}
	return 0;
}

/* flag != 0 (pop*) means do not preserve current point */
/* This is not implemented here (would require changing dvi_hh dvi_vv?) */

int popCTM (HDC hDC, int flag) {
	XFORM NEW;
	double dx, dy;
	if (bUseAdvancedGraphics == 0) return 0;
	CTMstackindex--;
	if (CTMstackindex < 0) {
//		wincancel("CTM stack underflow");
		if (bDebug) wincancel("CTM stack underflow");	// cut verbosity
		CTMstackindex = 0;
		return -1;
	}
	NEW = CTMstack[CTMstackindex];
	if (flag) {			/* Try and preserve current point ? */
		dx = CTM.eDx + (CTM.eM11 - NEW.eM11) * dvi_hh +
			 (CTM.eM21 - NEW.eM21) * dvi_vv;
		dy = CTM.eDy + (CTM.eM12 - NEW.eM12) * dvi_hh +
			 (CTM.eM22 - NEW.eM22) * dvi_vv;
/*		CTM = CTMstack[CTMstackindex]; */
		CTM = NEW;
		CTM.eDx = (float) dx;
		CTM.eDy = (float) dy;
	}
	else CTM = NEW;			/* 98/Mar/23 */
	if (SetWorld(hDC, CTM) != 0) {
/*		error condition ? */
	}
//	there was an error here until 99/Aug/22
	if (CTMstackindex == 0) {
		if (CTM.eDx == 0.0 && CTM.eDy == 0.0 &&
			CTM.eM11 == 1.0 && CTM.eM22 == 1.0 &&
			CTM.eM12 == 0.0 && CTM.eM21 == 0.0) {
			XFORM xf;										/* not used */
			ModifyWorldTransform(hDC, &xf, MWT_IDENTITY);	/* needed ? */
			AdvancedGraphicsOff(hDC);						/* GM_COMPATIBLE */
		}
	}
	return 0;
}

int popallCTM (HDC hDC, int flag) {
	XFORM xf;										/* ignored */
	if (bUseAdvancedGraphics == 0) return 0;		/* 97/Mar/28 */
/*	if (CTMstackindex == 0) return 0; */			/* nothing to do ? */
/*	for (k = 0; k < CTMstackindex; k++) (void) popCTM(hDC, flag); */
	CTMstackindex = 0;
	CTM = CTMstack[CTMstackindex];
	if (SetWorld(hDC, CTM) != 0) {					/* should be identity */
	}
	ModifyWorldTransform(hDC, &xf, MWT_IDENTITY);	/* needed ? */
	AdvancedGraphicsOff(hDC);						/* GM_COMPATIBLE */
	return 0;
}	/* flag unreferenced */

int pushCTM (HDC hDC, int flag) {
	if (bUseAdvancedGraphics == 0) return 0; 		/* 97/Mar/28 */
	CTMstack[CTMstackindex] = CTM;
	CTMstackindex++;
	if (CTMstackindex >= MAXCTMSTACK) {
		wincancel("CTM stack overflow");
		CTMstackindex--;
		return -1;
	}
	return 0;
}	/* hDC, flag unreferenced */

int checkCTM (HDC hDC) {			/* CALL AT END OF PAGE */
	if (CTMstackindex == 0) return 0;
	winerror("CTM stack not empty at EOP");
	popallCTM(hDC, 0);			/* clear CTM stack */
	return -1;
}

/* flag != 0 (translate*) means reverse direction */
/* NOTE: This has different semantics from rotate, scale, and concat */
/* If would make no sense to make it relative to current point! */

int translateCTM (HDC hDC, double dx, double dy, int flag) {
	XFORM M;
	if (bUseAdvancedGraphics == 0) return 0;
	if (bAdvancedGraphicsMode == 0) {
		if (AdvancedGraphicsOn(hDC) != 0) return 0;
	}
	if (flag) {
		dx = -dx; dy = -dy;
	}
	M.eM11 = M.eM22 = (float) 1.0;
	M.eM12 = M.eM21 = (float) 0.0;
/*	translate to logical coordinates */
	M.eDx = (float) lmapd ((long) dx);
	M.eDy = (float) lmapd ((long) dy);	/* may need to flip sign here ? */
	multiplyCTM (&CTM, &M, &CTM);
	if (ModifyWorld(hDC, M) != 0) {
	}
	return 0;
}

/* flag != 0 (scale*) means scale by inverse of given factors */

int scaleCTM (HDC hDC, double sx, double sy, int flag) {
	XFORM M;
	if (bUseAdvancedGraphics == 0) return 0;
	if (bAdvancedGraphicsMode == 0) {
		if (AdvancedGraphicsOn(hDC) != 0) return 0;
	}
	if (flag) {
		if (sx == 0.0 || sy == 0.0) {
			wincancel("CTM zero scale");
			sx = sy = 1.0;
		}
		else {
			sx = 1.0 / sx;	 sy = 1.0 / sy;
		}
	}
	M.eM12 = M.eM21 = (float) 0.0;
	M.eM11 = (float) sx;	 M.eM22 = (float) sy;
	dvi_hh = mapx(dvi_h); dvi_vv = mapy(dvi_v);		/* ??? */
	cp_valid = 0;									/* ??? */
/*	tx = (1 - m11) * x - m21 * y */
	M.eDx = (float) ((1.0 - M.eM11) * dvi_hh - M.eM21 * dvi_vv);
/*	ty = (1 - m22) * y - m12 * x */
	M.eDy = (float) ((1.0 - M.eM22) * dvi_vv - M.eM12 * dvi_hh);
	multiplyCTM (&CTM, &M, &CTM);
	if (ModifyWorld(hDC, M) != 0) {
	}
	return 0;
}

/* flag != 0 (rotate*) means rotate in opposite direction */
/* NOTE: we are in a left-hand coordinate system so flip sign of angle ? */

int rotateCTM (HDC hDC, double theta, int flag) {
	double c, s;
	XFORM M;

	if (bUseAdvancedGraphics == 0) return 0;
	if (bAdvancedGraphicsMode == 0) {
		if (AdvancedGraphicsOn(hDC) != 0) return 0;
	}
	if (flag) theta = -theta;
	c = cos(theta / 180.0 * 3.141592653);
	s = sin(theta / 180.0 * 3.141592653);
/*	avoid rounding errors in special cases */
	if (theta == 0.0 || theta == 360.0 || theta == -360.0) {
		c = 1.0; s = 0.0;
	}
	else if (theta == 180.0 || theta == -180.0) {
		c = -1.0; s = 0.0;
	}
	else if (theta == 90.0 || theta == -270.0) {
		c = 0.0; s = 1.0;
	}
	else if (theta == -90.0 || theta == 270.0) {
		c = 0.0; s = -1.0;
	}
	M.eM11 = (float) c;		M.eM12 = (float) s;
	M.eM21 = - (float) s;	M.eM22 = (float) c;
	dvi_hh = mapx(dvi_h); dvi_vv = mapy(dvi_v);		/* ??? */
	cp_valid = 0;									/* ??? */
/*	tx = (1 - m11) * x - m21 * y */
	M.eDx = (float) ((1.0 - M.eM11) * dvi_hh - M.eM21 * dvi_vv);
/*	ty = (1 - m22) * y - m12 * x */
	M.eDy = (float) ((1.0 - M.eM22) * dvi_vv - M.eM12 * dvi_hh);
	multiplyCTM (&CTM, &M, &CTM);
	if (ModifyWorld(hDC, M) != 0) {
	}
	return 0;
}

/* We don't allow translation in concat --- for reason see translate */
/* NOTE: we are in a left-hand coordinate system so flip sign of m12 m21 ? */

int concatCTM (HDC hDC,
	double m11, double m12, double m21, double m22, double m31, double m32,
		  int flag) {
	XFORM M;
	if (bUseAdvancedGraphics == 0) return 0;
	if (bAdvancedGraphicsMode == 0) {
		if (AdvancedGraphicsOn(hDC) != 0) return 0;
	}
	M.eM11 = (float) m11;	M.eM12 = (float) m12;
	M.eM21 = (float) m21;	M.eM22 = (float) m22;
	M.eDx  = (float) m31;	M.eDy  = (float) m32;
/*	but we basically ignore m31 and m32 after all */
	if (flag) {
		invertCTM(&M, &M);
	}
	dvi_hh = mapx(dvi_h); dvi_vv = mapy(dvi_v);		/* ??? */
	cp_valid = 0;									/* ??? */
/*	tx = (1 - m11) * x - m21 * y */
	M.eDx = (float) ((1.0 - M.eM11) * dvi_hh - M.eM21 * dvi_vv);
/*	ty = (1 - m22) * y - m12 * x */
	M.eDy = (float) ((1.0 - M.eM22) * dvi_vv - M.eM12 * dvi_hh);
	multiplyCTM (&CTM, &M, &CTM);
	if (ModifyWorld(hDC, M) != 0) {
	}
	return 0;
}

/* #define NULLREGION          1 */
/* #define SIMPLEREGION        2 */
/* #define COMPLEXREGION       3 */
/* #define RGN_ERROR           0 */

/* Deal with \special{CTM: ...} push pop rotate scale translate concat */
/* Some of these have alternate forms indicated by trailing `*' */
/* In the case of rotate, scale, translate, concat these are inverses */

void doCTM (HDC hDC, HFILE input) {	/* \special{CTM: } 96/Oct/10 */
	char *s;
	int n, flag;
	double dx, dy, sx, sy, theta, m11, m12, m21, m22, m31, m32;

	scanspecial(input, line, MAXLINE);		/* read rest of line */
	s = line;
	for(;;) {								/* allow multiple commands */
		while (*s <= ' ' && *s != '\0') s++;	/* step over white space */
		if (*s == '\0') break;				/* processed all of them */
		n = 0;								/* no of characters to step over */
		flag = 0;							/* set if * follows command */

		if (strncmp(s, "pop", 3) == 0) {
			s += 3;
			if (*s == '*') {				/* pop* */
				flag++;				s++;
			}
/*			if (popCTM(hDC, flag) != 0) break; */
			(void) popCTM(hDC, flag);
		}
		else if (strncmp(s, "popall", 6) == 0) {
			s += 6;
			if (*s == '*') {				/* popall* */
				flag++;				s++;
			}
			popallCTM(hDC, flag);
		}
		else if (strncmp(s, "push", 4) == 0) {
			s += 4;
			if (*s == '*') {				/* push* */
				flag++;				s++;
			}
			if (pushCTM(hDC, flag) != 0) break;
		}
		else if (strncmp(s, "translate", 9) == 0) {
			s += 9;
			if (*s == '*') {				/* translate* */
				flag++;				s++;
			}
			if (sscanf(s, "%lg %lg%n", &dx, &dy, &n) < 2) {
				complainspecial(input);
				break;
			}
			translateCTM(hDC, dx, dy, flag);
		}
		else if (strncmp(s, "scale", 5) == 0) {
			s += 5;
			if (*s == '*') {				/* scale* */
				flag++;				s++;
			}
			if (sscanf(s, "%lg %lg%n", &sx, &sy, &n) < 2) {
				complainspecial(input);
				break;
			}
			if (scaleCTM(hDC, sx, sy, flag) != 0) break;
		}		
		else if (strncmp(s, "rotate", 6) == 0) {
			s += 6;
			if (*s == '*') {				/* rotate* */
				flag++;				s++;
			}
			if (sscanf(s, "%lg%n", &theta, &n) < 1) {
				complainspecial(input);
				break;
			}
			rotateCTM(hDC, theta, flag);
		}
		else if (strncmp(s, "concat", 6) == 0) {
			s += 6;
			if (*s == '*') {
				flag++;				s++;
			}
			m31 = m32 = 0.0;
			if (sscanf(s, "%lg %lg %lg %lg%n",
					   &m11, &m12, &m21, &m22, &n) < 4) { 
				complainspecial(input); /* must have at least 4 arguments */
				break;
			}
			else {				/*	or could have 6 arguments */
				sscanf(s, "%lg %lg %lg %lg %lg %lg%n",
					   &m11, &m12, &m21, &m22, &m31, &m32, &n);
			}					/* but last two are ignored in any case */
			concatCTM(hDC, m11, m12, m21, m22, m31, m32, flag);
		}
		else {
			complainspecial(input);
			break;
		}
		s += n;								/* step over arguments */
	}
	flushspecial(input);
}
#endif

/*******************************************************************/

/* support for clipping 98/Sep/10 */

/* logical coordinates upper left corner, lower right corner */
/* (flag & 1) != 0 for subtract instead of intersect clip region */
/* (flag & 2) != 0 for draw clip path boundary */

void clipbox(HDC hDC, int xll, int yll, int xur, int yur, int flag) {
	int ret;
	if (yur < yll) {
		ret = yll; yll = yur; yur = ret;
	}
	if (xur < xll) {
		ret = xll; xll = xur; xur = ret;
	}
	if (bDebug > 1) {
		RECT ClipRect;
		(void) GetClipBox(hDC, &ClipRect);
		sprintf(debugstr, "hDC %X %d %d %d %d",
				hDC, ClipRect.left, ClipRect.bottom, ClipRect.right,
				ClipRect.top); 
		OutputDebugString(debugstr);
		sprintf(debugstr, "hDC %X BBox %d %d %d %d flag %d",
				hDC, xll, yll, xur, yur, flag);
		OutputDebugString(debugstr);
	}

	if (flag & 2) {					/* draw clippath boundary ? */
		RECT RuleRect;
		POINT Rule[5];

		RuleRect.left = xll;	RuleRect.bottom = yll;
		RuleRect.right = xur;	RuleRect.top = yur;
/*		(void) MoveTo(hdc, RuleRect.left, RuleRect.bottom);
		(void) LineTo(hdc, RuleRect.right, RuleRect.bottom);
		(void) LineTo(hdc, RuleRect.right, RuleRect.top);
		(void) LineTo(hdc, RuleRect.left, RuleRect.top);
		(void) LineTo(hdc, RuleRect.left, RuleRect.bottom); */
/*		combine into one call for speed */
		Rule[0].x = RuleRect.left; 	Rule[0].y = RuleRect.bottom;
		Rule[1].x = RuleRect.right;	Rule[1].y = RuleRect.bottom;
		Rule[2].x = RuleRect.right;	Rule[2].y = RuleRect.top;
		Rule[3].x = RuleRect.left;	Rule[3].y = RuleRect.top;
		Rule[4].x = RuleRect.left; 	Rule[4].y = RuleRect.bottom;
		Polyline(hDC, Rule, 5);
	}

	if (flag & 1) ret = ExcludeClipRect(hDC,  xll, yll, xur, yur);
	else ret = IntersectClipRect(hDC,  xll, yll, xur, yur);

	if (bDebug > 1) {
		RECT ClipRect;
		(void) GetClipBox(hDC, &ClipRect);
		sprintf(debugstr, "hDC %X %d %d %d %d",
				hDC, ClipRect.left, ClipRect.bottom, ClipRect.right,
				ClipRect.top); 
		OutputDebugString(debugstr);
		sprintf(debugstr, "hDC %X BBox %d %d %d %d ret %d",
				hDC, xll, yll, xur, yur, ret);
		OutputDebugString(debugstr);
	}
}

int nSavedDC=-1;		/* set to -1 at beginning of page */
int nDCstackindex=0;	/* should set to zero at beginning of page */

void ResetDCStack(void) {			/* call at beginning of page */
	nDCstackindex = 0;
	nSavedDC = -1;
}

void doClipBoxPush(HDC hDC) {		/* clip push */
	int ret;
	if (! bAllowClip) return;
/*	save hDC on a stack, get new one */
	ret = SaveDC(hDC);
	if (ret == 0) winerror("SaveDC failed");
	if (nDCstackindex == 0) nSavedDC = ret;
	else nDCstackindex++;
}

void doClipBoxPop(HDC hDC) {		/* clip pop */
	int ret;
	HFONT hFont=NULL;
	if (! bAllowClip) return;
/*	hFont = GetCurrentObject(hDC, OBJ_FONT); */
	if ((UINT) hFontOld > 1) hFont = SelectObject(hDC, hFontOld);
/*	restore hDC off stack */
	ret = RestoreDC(hDC, -1);		/* restore most recently saved */
	if (! ret) winerror("RestoreDC failed");
	else nDCstackindex--;
	if ((UINT) hFont > 1) SelectObject(hDC, hFont);
/*	set current point correctly again ??? */
/*	dvi_hh = mapx(dvi_h); dvi_vv = mapy(dvi_v); */
	cp_valid = 0;			/* current point in window NOT valid */
}

void doClipBoxPopAll(HDC hDC) {			/* call at end of page */
	int ret;
	if (nDCstackindex == 0) return;
	ret = RestoreDC(hDC, nSavedDC);		/* restore first saved on page */
/*	or call repeatedly with -1 untill nDCstackindex counted down to zero */
	if (! ret) winerror("RestoreDC failed");
	else nDCstackindex=0;
}

void DoClipBox(HDC hDC, int input, int c) {	/* 98/Sep/8 */
	char *s;
	long dwidth, dheight;
/*	long cxll, cyll, cxur, cyur; */
	int n, flag, subtract, stroke;
	
	flag = subtract = stroke = 0;
	if (bViewExtras) stroke = 1;
	s = line + strlen(line);
	if (c > 0) *s++ = (char) c;				/* stick in terminator 99/Feb/21*/
	*s = '\0';								/* just in case */
	if (nspecial > 0)
		readspecial (input, s, sizeof(line) - strlen(line));	/* read rest of line */

/*	if (bDebug > 1) OutputDebugString(line); */	/* 99/Feb/21 test */

	s = line;
	while (*s != ' ' && *s != '\0') s++;

	for(;;) {								/* allow multiple commands */
		while (*s <= ' ' && *s != '\0') s++;	/* step over white space */
		if (*s == '\0') break;				/* processed all of them */
		if (strncmp(s, "pop", 3) == 0) {
			doClipBoxPop(hDC);
			s += 3;
			continue;
		}
		if (strncmp(s, "push", 4) == 0) {
			doClipBoxPush(hDC);
			s += 4;
			continue;
		}
		if (strncmp(s, "popall", 4) == 0) {
			doClipBoxPopAll(hDC);
			s += 6;
			continue;
		}
		if (strncmp(s, "stroke", 6) == 0) {
			stroke=1;
			s += 6;
			continue;
		}
		if (strncmp(s, "box", 3) == 0) {
			s += 3;
			if (*s == '*') {
				subtract=1;
				s++;
			}
			while (*s <= ' ' && *s != '\0') s++;	/* step over white space */
			dwidth = dheight = 0;
/*			cxll = cyll = cxur = cyur = 0; */
			flag = (stroke << 1) | subtract;
/*			if (sscanf(s, "%ld %ld%ld %ld%n",
			   &cxll, &cyll, &cxur, &cyur, &n) == 4) {
			s += n;
			clipbox(hDC, mapx(cxll), mapy(cyur), mapx(cxur), mapy(cyll),
				flag);
			} */
			if (sscanf(s, "%ld %ld%n", &dwidth, &dheight, &n) == 2) {
				s += n;
				clipbox(hDC, mapx(dvi_h), mapy(dvi_v),
					mapx(dvi_h + dwidth), mapy(dvi_v - dheight), flag);
			}
			else {
				complainspecial(input); /* must have at least 2 arguments */
				break;
			}
		}
		else {
			complainspecial(input);
			break;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Units of measure recognized by TeX */

char *unitnames[] = {
/*	WAS: "pt", "pc", "in", "bp", "cm", "mm", "dd", "cc", "sp", "" */
	"sp", "pt", "bp", 
		"dd", "mm", 
			"pc", "cc", 
				"cm", "in",
					"pcpt",	""							/* 93/Dec/16 */
};

/*	Is this assuming standard dvi_num and dvi_den ??? */

double unitscale[] = {
	(72.0 / 72.27) / 65536.0, (72.0 / 72.27), 1.0, 
		(1238.0  / 1157.0) * (72.0 / 72.27), (72.0 / 25.4), 
			12.0 * (72.0 / 72.27), 12.0 * (1238.0  / 1157.0) * (72.0 / 72.27),
				(72.0 / 2.54), 72.0, 
					12.0 * (72.0 / 72.27),					/* 93/Dec/16 */
						1.0,
/*	WAS: PT, PC, IN, BP, CM, MM, DD, CC,  SP, 1.0 */
};

int pcptflag=0;			/* decompose picas and points */

double decodeunits(char *units) {
	int k;
	int pcptflag;

/*	for (k = 0; k < 9; k++) */
	for (k = 0; k < 32; k++) {
		if (strcmp(unitnames[k], "") == 0) break;
		if (strcmp(unitnames[k], units) == 0) {
			if (k == 9) pcptflag=1;
			else pcptflag=0;
			return unitscale[k];
		}
	}
	if (bUnitsWarned++ < 4) {					/* 95/Aug/14 */
		sprintf(debugstr, "Don't understand units: %s", units);
		wincancel(debugstr);
	}
	return 1.0;
}

/* em:message xxx --- output message xxx right away on console */
/* em:linewidth w ---- set linewidth for subsequent lines 0.4pt default */
/* em:moveto --- remember current point as start for next line */
/* em:lineto --- draw line from previous moveto or lineto */

/* em:graph xxx ---- MSP or PCX image top-left corner at current point */
/* em:point n --- remember current coordinates for point n */
/* em:line a[h|v|p],b[h|v|p][,w] draw line thickness w from point a to b */

long emdvi_h=0, emdvi_v=0;				/* last point for lineto */

/*	Is this assuming standard dvi_num and dvi_den ??? */

void reademtex(HDC hDC, int input) {
	double linewidth, multiple;
	long emline = 26214; 					/* default 0.4pt * 65536 */
	char *s;
	char units[3];
	int n;
	int xold, yold, xnew, ynew; 
	HPEN hTPICPen=NULL, hOldPen;

/*	if (getalphatoken(input, line, MAXLINE) == 0) */
	if (getalphatoken(input, line, sizeof(line)) == 0) {
		complainspecial(input);
		return;
	}
	if (strcmp(line, "message") == 0) {
/*		putc(' ', stdout);	 */
/*		scanspecial(input, line, MAXLINE); */	/* throw away */
		scanspecial(input, line, sizeof(line)); /* throw away */
/*		fputs(line, stdout);	*/	
	}
/*	Is this assuming standard dvi_num and dvi_den ??? */
	else if (strcmp(line, "linewidth") == 0) {
/*		scanspecial(input, line, MAXLINE); */
		scanspecial(input, line, sizeof(line));
		s = line;
		if (sscanf(s, "%lg%n", &linewidth, &n) == 0) 
/*			fprintf(stderr, "linewidth not specified"); */ 
			emline = 26214;					/* used default */
		else {
			s = s + n;
			units[0] = *s++; units[1] = *s++; units[2] = '\0';
			multiple = decodeunits (units);		/* conversion to PS pt's */
/*			Is this assuming standard dvi_num and dvi_den ??? */
			linewidth = linewidth * multiple * (72.27 / 72.0) * 65536.0;
			emline = (long) (linewidth + 0.5);  /* convert to scaled pt's */
		}
		penwidth = mapd(emline);
		if (penwidth <= 0) penwidth = 1;
/*		fprintf(output, "\n%ld emw", emline); */
	}
	else if (strcmp(line, "moveto") == 0) {
		emdvi_h = dvi_h; emdvi_v = dvi_v;	/*	remember current position */
/*		fputs(" emm", output); */
	}
	else if (strcmp(line, "lineto") == 0) {
/*	draw line from (emdvi_h, emdvi_v) to (dvi_h, dvi_v) */
/*		fputs(" eml", output); */
		hTPICPen = CreatePen(PS_SOLID, penwidth, RGB(rule_r, rule_g, rule_b));
		hOldPen = SelectObject(hDC, hTPICPen);
		xnew = mapx(dvi_h);
		ynew = mapy(dvi_v);
		xold = mapx(emdvi_h);
		yold = mapy(emdvi_v);
		MoveToEx(hDC, xold, yold, NULL);
		LineTo(hDC, xnew, ynew);
		(void) SelectObject(hDC, hOldPen);
		(void) DeleteObject (hTPICPen); 
		emdvi_h = dvi_h; emdvi_v = dvi_v;	/*	remember current position */
	}
	else complainspecial(input);
}

/*		DVITPS specials for figure insertion --- 93/Mar/24	*/
/*		Not done as carefully as it should be, see DVI2PS and DVIPS code */

void dvitopsmiss (void) {
	wincancel("File name missing in DVITPS, DVITOPS, or PC-TeX special");
}

void readdvitps (HDC hDC, HFILE input) {
	char epsfilename[MAXFILENAME]="";
/*	long pslength; */
/*	int n; */
/*	char *s=line; */
	
/* get token following in `dvitps:' */

	if (getalphatoken(input, line, sizeof(line)) == 0) {
		complainspecial(input);
		return;
	}
	if (strcmp(line, "Include1") == 0) {
/*		get file name then */
		if (gettoken(input, epsfilename, sizeof(epsfilename)) == 0) {  
/*			sprintf(str, "File name missing in DVITPS special"); */
/*			wincancel(str);	*/
			dvitopsmiss();
			flushspecial(input);
			return; 
		}
/* complain if pstpsstyle not set ??? */
/* check on what follows after file name ? */
/*		if (scanspecial(input, line, MAXLINE) == MAXLINE) */
		if (scanspecial(input, line, sizeof(line)) == sizeof(line)) { 
			complainspecial(input);		/* TOO DAMN LONG ! */
		}
		copyepsfileclip(hDC, epsfilename);
/*		copyepsfileaux(hDC, epsfilename, 1, 1); */
	}
	else if (strcmp(line, "Include0") == 0) {
		if(gettoken(input, epsfilename, sizeof(epsfilename)) == 0) {  
/*			sprintf(str, "File name missing in DVITPS special"); */
/*			wincancel(str); */
			dvitopsmiss();
			flushspecial(input);
			return;
		}
/* Who knows whether the following ever makes sense ! */
		if (bPrintFlag != 0 && bPassThrough != 0 && IsItPSCRPT(hDC) != 0) {
			copyfilethrough(hDC, epsfilename);	 /* copy EPS file through */
		}
/*		copyepsfileclip(hDC, epsfilename);	*//* NOT REALLY */
/*		copyfilethrough(hDC, fname); */	/* copy EPS file through */
/*		copyepsfileaux(hDC, epsfilename, 0, 0); */
	}
	else if (strcmp(line, "Literal") == 0) { 
		if (pstpsstyle == 0 &&		/* avoid repeat  startTexFig */
			strstr(line, "startTexFig") != NULL) {
			starttexfig(line);
			pstpsstyle=1;					/* note we are inside */
		}
		else if (pstpsstyle != 0 &&		/* avoid repeat  endTexFig */
			strstr(line, "endTexFig") != NULL) {
			pstpsstyle=0;
		}
		else copystring(input);		/* just copy what follows ! */
	}
	else complainspecial(input);
	flushspecial(input);
}

/* DVITOPS specials for figure insertion	\special{dvitops: ...}	*/
/* Not implemented: begin, end, origin, rotate, transform ... regions color */
/*		added PC-TeX \special{eps: <filename> <x=2in> <y=3cm>} 94/Jun/17 */

/* void readdvitops(HDC hDC, HFILE input) */ /* 94/June/17 */
int readdvitops (HDC hDC, HFILE input, int pctexflag) {
	double width=0.0, height=0.0, widthd, heightd;
	double hscale, vscale, scale, multiple;
	double xshift, yshift;						/* not accessed */
	char epsfilename[MAXFILENAME]="";
	char units[3];
	int n;
	char *s, *sn;

	if (pctexflag == 0) {	/* if dvitops: check "import" next */
/*		if (getalphatoken(input, line, MAXLINE) == 0) */
		if (getalphatoken(input, line, sizeof(line)) == 0) {
			complainspecial(input);
			return -1;
		}
	}
/*	if (strcmp(line, "import") == 0) */
	if (pctexflag || strcmp(line, "import") == 0) {
/*		if(gettoken(input, epsfilename, MAXFILENAME) == 0) */
		if(gettoken(input, epsfilename, sizeof(epsfilename)) == 0) {  
/*			sprintf(str, "File name missing in DVITOPS special"); */
/* 			wincancel(str); */
			dvitopsmiss();
/*			errcount();	 */
			flushspecial(input); 
			return -1;
		}
/*		wincancel("DVITOPS James Clark"); */
/*		see if width and height specified in dvitops: import */
		if (scanspecial(input, line, sizeof(line)) == sizeof(line)) {
			complainspecial(input); /* TOO DAMN LONG ! */
//			return -1;
		}
/*		scale specified width and height to PS points */
		else {
			s = line;
/*			step over "x=" in PC-TeX style 94/June/17 */
			if (pctexflag && (sn = strstr(s, "x=")) != NULL) s = sn + 2;
			if (sscanf(s, "%lg%n", &width, &n) != 0) {
				s += n;
				units[0] = *s++; units[1] = *s++; units[2] = '\0';
				multiple = decodeunits(units);
				width = width * multiple;
			}
/*			else wincancel("Width not specified"); */
/*	step over "y=" in PC-TeX style 94/June/17 */
			if (pctexflag && (sn = strstr(s, "y=")) != NULL) s = sn + 2;
			if (sscanf(s, "%lg%n", &height, &n) != 0) {
				s += n;
				units[0] = *s++; units[1] = *s++; units[2] = '\0';
				multiple = decodeunits(units);
				height = height * multiple;
			}
/*			else wincancel("Height not specified"); */
/*			sprintf(str, "Height %lg Width %lg", height, width); 
			wincancel(str); */

			depsllx = depslly = depsurx = depsury = 0.0;

/*			try and read bbox (but NOT absolutely required) */
/*			(void) setupbbox(epsfilename, 0); */	/* failure is OK here */ 
			if (setupbbox(epsfilename, 0, 1) == 0) {
//				return -1;							// failure 
			}

			startspecial();
			widthd = depsurx - depsllx; heightd = depsury - depslly;
			if (widthd == 0.0) widthd = width;		/* in case no BBox */
			if (heightd ==0.0) heightd = height;	/* in case no BBox */
			if (width == 0.0) width = widthd;		/* in case not given */
			if (height == 0.0) height = heightd;	/* in case not given */
			if (widthd == 0.0 || heightd == 0.0) {
				sprintf(debugstr, "Zero area BoundingBox %lg %lg %lg %lg\n",
					depsllx, depslly, depsurx, depsury); 
				wincancel(debugstr);
/*				fclose(special); */
/*				special = NULL; */
				return -1;			// failure
			}
			hscale = width / widthd;
			vscale = height / heightd;
			xshift = 0.0; yshift = 0.0;
			if (hscale <= vscale) {		/* horizontal scale is limit */
/*				xshift = (height - heightd * hscale) / 2.0; */
				yshift = (height - heightd * hscale) / 2.0;
/*				fprintf(output, "0 %lg translate ", yshift); */
/*				fprintf(output, "%lg dup scale\n", hscale); */
				scale = hscale;
			}
			else {						/* vertical scale is limit */
/*				yshift = (width - widthd * vscale) / 2.0; */
				xshift = (width - widthd * vscale) / 2.0;
/*				fprintf(output, "%lg 0 translate ", xshift); */
/*				fprintf(output, "%lg dup scale\n", vscale); */
				scale = vscale;
			}
/*			fprintf(output, "%lg neg %lg neg translate ", xll, yll); */
/*			sprintf(str, "xshift %lg yshift %lg", xshift, yshift);
			wincancel(str); */
/*			scalebbox(width, height, depsllx, depslly, depsurx, depsury); */
/*			fclose(special); */
/*			special = NULL; */
/*			following is very suspicious !!! */
			epswidth = (long) (width * scaledpoint); 
			epsheight = (long) (height * scaledpoint); 
/*			pswidth = epswidth; psheight = epsheight; */ /* ??? */
			pswidth = (long) (widthd * scale * scaledpoint);
			psheight = (long) (heightd * scale * scaledpoint);
/*			dvi_v -= psheight; */
			if (xshift != 0)
				dvi_h += (long) (xshift * scaledpoint);	/* 93/Nov/7 */
			if (yshift != 0)
				dvi_v -= (long) (yshift * scaledpoint);	/* 93/Nov/7 */
			copyepsfileaux(hDC, epsfilename, 1, 1);
/*			dvi_v += psheight; */
			if (xshift != 0)
				dvi_h -= (long) (xshift * scaledpoint);	/* 93/Nov/7 */
			if (yshift != 0)
				dvi_v += (long) (yshift * scaledpoint);	/* 93/Nov/7 */
			endspecial();	
/*			fclose(special);	*/			/* ??? NEW ??? */
/*			special = NULL;		*/			/* debugging */
		}
	}
	else if (strcmp(line, "inline") == 0) {
/* NOTE: dvitops has DVI coordinates in effect for `inline' commands */
/* - so DONT use forscl and revscl here ! */
/*		putc('\n', output); */			/* just in case! */
/*		if (preservefont != 0) fprintf(output, "currentfont "); */ /*	NEW */
/*		putc('\n', output); */
		copystring(input);		/* just copy what follows ! */
/*		if (preservefont != 0)  fprintf(output, "setfont "); */
	}
	else if (strcmp(line, "landscape") == 0) {
/*		fprintf(output, "\nrevscl "); */
/*		fprintf(output, "Texlandscape "); */
/* 		fprintf(output, "forscl\n"); */
	}
	else complainspecial(input);
	flushspecial(input);
	return 0;				// OK
}

/* Special starts with `ps:' or `ps::' - lots of possibilities ... */
/* DVIPS style? - lots of DVIPS stuff not implemented */
void readdvips (HDC hDC, HFILE input) {
	int c;
#ifdef AllowCTM
	int n;
#endif
	double scale=1.0;
	char *s;
	int flag;
	char epsfilename[MAXFILENAME]="";

/*	if (traceflag != 0) wincancel("Processing DVIPS style special\n"); */

	if (bDebug > 1) OutputDebugString("Entering READDVIPS\n");

	c = an_wingetc(input); --nspecial;	/* peek at next character */

	if (c == ':') { /* a second colon --- deal with ps:: type command */
/*	global | local | inline | asis | begin | end <user PS> ??? */
/*		(void) gettoken(input, line, MAXLINE); */
/*		if (verbatimflag != 0) stripbracket(output, intput); */
		fliteral = an_wintell(input);		/* remember where this was */
		nliteral = nspecial;				/* and how many bytes left */
/*		if (scanspecial(input, line, MAXLINE) == MAXLINE) */
		if (scanspecial(input, line, sizeof(line)) == sizeof(line)) flag = 0;
		else flag = 1;
/*		if (bDebug > 1) {
			OutputDebugString(line);
			if (strncmp(line, "tx@Dict", 7) == 0) OutputDebugString("Match");
			else OutputDebugString("No Match");
		} */ /* PS Tricks debug output */
		if (flag == 0) {
			if (verbatimflag != 0) {		/* long ps:: type verbatim */
				(void) an_winseek(input, fliteral);	/* start over */
				nspecial = nliteral;
				stripbracket(input);		/* revscl - forscl ??? */
			}
			complainspecial(input);			/* TOO DAMN LONG ! */
		}
/*		check whether this is a startTexFig */
/*		else if (bDebug > 1) OutputDebugString("We get early\n"); */
		else if (pstagstyle == 0 &&		/* avoid repeat call to startTexFig */
				strstr(line, "startTexFig") != NULL) {
			if ((s = strstr(line, "[begin]")) != NULL) s = s + 7;
			else s = line;
			starttexfig(s);
/*			verbout(s); */
			pstagstyle=1;		/* note that we have `begin' for this */
		}
/*		check whether this is a endTexFig */
		else if (pstagstyle != 0 &&		/* avoid repeat call to endTexFig */
				strstr(line, "endTexFig") != NULL) {
			if ((s = strstr(line, "[end]")) != NULL) s = s + 5;
			else s = line;
/*			verbout(s);	 */
			pstagstyle=0;			/* note we have `end' for this */
		}
		else if (pstagstyle != 0 &&		/* makes no sense otherwise */ 
				strstr(line, "doclip") != NULL) {
			clipflag = 1;
/*			verbout(line); */
		}
#ifdef AllowCTM
/*	Try and catch PSTricks style rotate command */
/*		else if (bDebug > 1) OutputDebugString("We get here\n"); */
		else if (bUseCTMflag && strncmp(line, "tx@Dict", 7) == 0) {
/*	Looking for ps:: tx@Dict begin 45. Rot  end --- (or 45. neg) */
			if (bDebug > 1) {
				OutputDebugString("PSTricks rotate\n");
			}
			if (strstr(line, " Rot ") != NULL) {
				double theta;
				s = line+14;			/* past "tx@Dict begin " */
				if (sscanf (s, "%lg", &theta) == 1) {
					if (bDebug > 1) {
						sprintf(debugstr, "rotate CTM %lg\n", theta);
						OutputDebugString(debugstr);
					}
					if (strstr(line, " neg ") != NULL)
							rotateCTM(hDC, theta, 1);
					else rotateCTM(hDC, theta, 0);
				}
			} /* end of PSTricks code */
		}
#endif
/*		maybe just copy the darn thing to the output ??? */
		else if (verbatimflag != 0) {
/*			verbout(line); */			/* revscl - forscl ??? */
		}
/*		else complainspecial(input); */ /* no complaints ps:: ??? 96/Oct/2 */
		flushspecial(input);			/* flush the rest - if any */
	}
	else {	/* NOT ps::, so go deal with ps: plotfile or ps: overlay etc */
		an_unwingetc(c, input); nspecial++;
		fliteral = an_wintell(input);		/* remember where this was */
		nliteral = nspecial;				/* and how many bytes left */
/*		if (getalphatoken(input, line, MAXLINE) == 0) */
		if (bSciWord) flag = gettoken(input, line, sizeof(line));
		else flag = getalphatoken(input, line, sizeof(line));
/*		Note: we only got one token so far ... */
		if (flag == 0) {								/* 94/Apr/22 */
			wincancel("Premature end of special");
			return;
		}
#ifdef DEBUGREADDVIPS
		if (bDebug > 1) OutputDebugString(line);
#endif
		if (strcmp(line, "plotfile") == 0) {	/* deal with plotfile */
/*			while (getalphatoken(input, line, MAXLINE) > 0) */ 
/*	deal with filename | global | local | inline | asis ??? */
/*			(void) gettoken(input, epsfilename, MAXFILENAME);*//* file name */
			(void) gettoken(input, epsfilename, sizeof(epsfilename));
			if (pstagstyle != 0)  {		/*	only if between [begin] & [end] */
/*				actually, now do nothing except copy the file ! */
/*				copyepsfile(hDC, epsfilename, 1, 0); */ /* incl & no shift */
/*				copyepsfile(hDC, epsfilename, 1, -1); */ /* incl & shift */
				copyepsfileclip(hDC, epsfilename);  /* incl &  shift */
			}
		}
		else if (strcmp(line, "overlay") == 0) { /* deal with overlay */
/*  		while (getalphatoken(input, line, MAXLINE) > 0) */
/*	deal with filename | on | off ??? */
/*			(void) gettoken(input, epsfilename, MAXFILENAME);*//* file name */
			(void) gettoken(input, epsfilename, sizeof(epsfilename)); 
			if (pstagstyle != 0)  {		/*	only if between [begin] & [end] */
/*				actually, now do nothing except copy the file ! */
				copyepsfile(hDC, epsfilename, 0, 0);	/* over & no shift */
			}
		}
		else if (strcmp(line, "epsfile") == 0) {     /* ArborText ? */
/*			if(gettoken(input, epsfilename, MAXFILENAME) > 0) {*//*filename */
			if(gettoken(input, epsfilename, sizeof(epsfilename)) > 0) {
				startspecial();
/* ArborText scale may be per mille */
/*				if(gettoken(input, line, MAXLINE) > 0) */	/* scale ? */ 
				if(gettoken(input, line, sizeof(line)) > 0) {	/* scale ? */
					if(sscanf(line, "%lg", &scale) > 0) {
						if (scale > 33.33) scale = scale/1000.0;
/*						fprintf(output, "%lg dup scale \n", scale); */
					}
				}
/*				(void) doinsertion(hDC, epsfilename, scale, 0); */
				(void) doinsertion(hDC, epsfilename, scale, 1, 1);
/*				copyepsfile(hDC, epsfilename, 1, 1); *//* old */
				endspecial();
			}
			else complainspecial(input); /* flush complaints ??? 96/Oct/2 */
		}
		else if (strcmp(line, "include") == 0) {    /* SVB style ? */
/*			if(gettoken(input, epsfilename, MAXFILENAME) > 0) {*//*filename */
			if(gettoken(input, epsfilename, sizeof(epsfilename)) > 0) { 
				if (pssvbstyle != 0)		/*	only if after startTexFig */
/*					copyepsfile(hDC, epsfilename, 0, 0); *//* no inc & no shift */
/*					copyepsfile(hDC, epsfilename, 1, -1); *//* inc & shift */
					copyepsfileclip(hDC, epsfilename);	/* inc & shift */
			}
			else complainspecial(input);
		} 
		else if (strcmp(line, "psfiginit") == 0) {	/* SVB style init */
/*			pssvbstyle = 1; */
/*			ignore - NOP ? (DVI2PS-SVB) */
		}
/*	deal with DV2PS-SCB literal */
		else if (strcmp(line, "literal") == 0) {		/* DVI2PS-SVB */
/*			printf(" LITERAL: pssvbstyle %d", pssvbstyle); */
/*			if (verbatimflag != 0) copystring(input); */
/*   		if (scanspecial(input, line, MAXLINE) == MAXLINE) */
			if (scanspecial(input, line, sizeof(line)) == sizeof(line)) {
				if (verbatimflag != 0) {	/* long ps: literal verbatim */
					(void) an_winseek(input, fliteral);
					nspecial = nliteral;
/*					fprintf(output, "\nrevscl "); */
/*					if (preservefont != 0) fprintf(output, "currentfont "); */
/*					putc('\n', output); */
					flushspecial(input);
/*					copystring(input); */
/*					if (preservefont != 0) fprintf(output, "setfont "); */
/*					fprintf(output, "forscl ");	*/ /* switch to DVI coord */
				}
				complainspecial(input); /* TOO DAMN LONG ! */
			}
			else if (pssvbstyle == 0 && strstr(line, "startTexFig") != NULL) {
				starttexfig(line);
/*				verbout(line); */
				pssvbstyle = 1;
			}
			else if (pssvbstyle != 0 && strstr(line, "doclip") != NULL) {
				clipflag = 1;
/*				verbout(line); */
			}
			else if (pssvbstyle != 0 && strstr(line, "endTexFig") != NULL) {
/*				verbout(line); */
				pssvbstyle = 0;
			}

// NOTE: the following code is repeated further down...
#ifdef AllowCTM
/*	Translate heuristically from DVIPSONE PS style to CTM style 96/Nov/5 */
/*	Must start with gsave or currentpoint or currentfont */
/*	Need to scan in the whole \special to do this ... */
			else if (bUseCTMflag) {
/*			 (strcmp(line, "gsave") == 0 ||
									 strcmp(line, "grestore") == 0 ||	
									 strcmp(line, "setgray") == 0 ||	
									 strcmp(line, "currentfont") == 0 ||
									 strcmp(line, "currentpoint") == 0)) */
/* #ifdef DEBUGREADDVIPS */
				if (bDebug > 1)	OutputDebugString(line);
/* #endif */
				strcat(line, " ");
				s = line + strlen(line);
				n = sizeof(line) - strlen(line);
				if (scanspecial(input, s, n) == n) { /* scan rest of line */
					if (verbatimflag != 0) {	/* long ps: literal verbatim */
						(void) an_winseek(input, fliteral);
						nspecial = nliteral;
						flushspecial(input);
					}
/*					complainspecial(input); */ /* TOO DAMN LONG ! */
					*line = '\0';	/* too long, just ignore it */
				}
#ifdef DEBUGREADDVIPS
				if (bDebug > 1) OutputDebugString(line);
#endif

				if (strstr(line, "gsave") != NULL) {	/* 99/Apr/16 */
					if (bDebug > 1)	DebugShowCTM(hDC, "Before push CTM");
					pushCTM(hDC, 0);
					doColorPush(hDC);
					if (bDebug > 1)	DebugShowCTM(hDC, "After push CTM");
				}
				else if (strstr(line, "grestore") != NULL) {	/* 99/Apr/16 */
					if (bDebug > 1) DebugShowCTM(hDC, "Before pop CTM");
					doColorPop(hDC);
					popCTM(hDC, 0);
					if (bDebug > 1) DebugShowCTM(hDC, "After pop CTM");
				}

				if ((s = strstr(line, "setgray")) != NULL) {	/* 99/Apr/16 */
					double fk;
					int r, g, b;
					s--;
					while (s >= line && *s == ' ') s--;
					while (s >= line && *s > ' ') s--;
					s++;
					if (sscanf(s, "%lg", &fk) > 0) {
						if (bDebug > 1) OutputDebugString(s);
						r = g = b = (int) (fk * 255.999);
						if (OldFore >= 0 || OldBack >= 0) {
							r = 255 - r; g = 255 - g; b = 255 - b;
						} 
						CurrentTextColor = RGB(r, g, b);
						OldTextColor = SetTextColor(hDC, CurrentTextColor);
						SetupRuleColor (hDC, r, g, b);				/* 95/Mar/8 */
					}
				}

//				check for changes in rotation or scale
				if ((s = strstr(line,
						"currentpoint currentpoint translate")) != NULL &&
					strstr(line, "neg exch neg exch translate") != NULL) {
//					winerror(line);			// debugging only
/*					if (strstr(line, "gsave") != NULL) {
						if (bDebug > 1) OutputDebugString("push CTM\n");
						pushCTM(hDC, 0);
					} */ /* rotate start */ 	/* moved 99/Apr/16 */
					s += 36;	/* currentpoint currentpoint translate */
					if (strstr(s, "rotate") != NULL) {		/* rotate start */
						double theta;
						if (bDebug > 1)	DebugShowCTM(hDC, "Before rotate CTM" );
						if (sscanf(s, "%lg%n", &theta, &n) == 1) {
//							take possible PS "neg" command into account
							if (strstr(s+n, "neg") < strstr(s+n, "rotate"))
								theta = - theta;
							if (bDebug > 1) {
								if (bFlipRotate) OutputDebugString("Flipped ");
								sprintf(debugstr, "rotate CTM %lg\n", theta);
								OutputDebugString(debugstr);
							}
							if (bFlipRotate) rotateCTM(hDC, theta, 1);
							else rotateCTM(hDC, theta, 0);
						}
						else if (bDebug > 1) OutputDebugString(line);
						if (bDebug > 1)	DebugShowCTM(hDC, "After rotate CTM" );
					}
					else if (strstr(s, "scale") != NULL) {	/* scale start */
						if (bDebug > 1)	DebugShowCTM(hDC, "Before scale CTM" );
						if (strstr(s, "div") != NULL) {		/* scale end */
							double sx, sy;
							int flag = 0; // 1
							if (sscanf(s, "1 %lg div 1 %lg div", &sx, &sy) == 2){
								sx = 1.0 / sx;
								sy = 1.0 / sy;
//								sx = sy = 1.0;
								if (bDebug > 1) {
									sprintf(debugstr,
											"scale* CTM %lg %lg\n", sx, sy);
									OutputDebugString(debugstr);
								}
//								sprintf(debugstr, "scale %lg %lg", sx, sy);
//								winerror(debugstr);	// debugging only
								scaleCTM(hDC, sx, sy, flag);
							}
							else if (bDebug > 1) OutputDebugString(line);
						}
						else {								/* scale start */
							double sx, sy;
							int flag = 0;
							if (sscanf(s, "%lg %lg", &sx, &sy) == 2){
								if (bDebug > 1) {
									sprintf(debugstr,
											"scale CTM %lg %lg\n", sx, sy);
									OutputDebugString(debugstr);
								}
//								sprintf(debugstr, "scale %lg %lg", sx, sy);
//								winerror(debugstr);	// debugging only
								scaleCTM(hDC, sx, sy, flag);
							}
							else if (bDebug > 1) OutputDebugString(line);
						}
						if (bDebug > 1)	DebugShowCTM(hDC, "After scale CTM" );
					}
				} /* end of "currentpoint currentpoint translate" */
				else if (bUseCTMflag && strstr(line,
				 "currentpoint grestore moveto") != NULL) {	/* rotate end */
/*	this isn't quite right since it preserves the current point ? */
//					if (bDebug > 1) OutputDebugString("pop CTM\n");
//					(void) popCTM(hDC, 1);
//					already done earlier ? 99/Aug/21
				} /* end of "currentpoint grestore moveto" */
			} /* end of bUseCTMFlag etc. */
#endif	/* end of ifdef AllowCTM */

			else if (verbatimflag != 0) {
/*				fprintf(output, "\nrevscl ");  */ /* ? */
/*				verbout(line); */
/*				fprintf(output, "forscl\n");	*/ /* ? */
			}
/*			else complainspecial(input); */ /* no complaints ps: ? 96/Oct/2 */
		}	/* end of "literal" case ps: */

// NOTE: the following code is repeated higher down...
#ifdef AllowCTM
/*	Translate heuristically from DVIPSONE PS style to CTM style 96/Nov/5 */
/*	Must start with gsave or currentpoint or currentfont */
/*	Need to scan in the whole \special to do this ... */
		else if (bUseCTMflag) {
/* (strcmp(line, "gsave") == 0 ||
								 strcmp(line, "grestore") == 0 ||	
								 strcmp(line, "setgray") == 0 ||	
								 strcmp(line, "currentfont") == 0 ||
								 strcmp(line, "currentpoint") == 0))  */
/* #ifdef DEBUGREADDVIPS */
			if (bDebug > 1)	OutputDebugString(line);
/* #endif */
			strcat(line, " ");
			s = line + strlen(line);
			n = sizeof(line) - strlen(line);
			if (scanspecial(input, s, n) == n) { /* scan rest of line */
				if (verbatimflag != 0) {	/* long ps: literal verbatim */
					(void) an_winseek(input, fliteral);
					nspecial = nliteral;
					flushspecial(input);
				}
/*				complainspecial(input); */ /* TOO DAMN LONG ! */
				*line = '\0';	/* too long, just ignore it */
			}
#ifdef DEBUGREADDVIPS
			if (bDebug > 1) OutputDebugString(line);
#endif

			if (strstr(line, "gsave") != NULL) {	/* 99/Apr/16 */
				if (bDebug > 1)	DebugShowCTM(hDC, "Before push CTM");
				pushCTM(hDC, 0);
				doColorPush(hDC);
				if (bDebug > 1)	DebugShowCTM(hDC, "After push CTM");
			}
			else if (strstr(line, "grestore") != NULL) {	/* 99/Apr/16 */
				if (bDebug > 1) DebugShowCTM(hDC, "Before pop CTM");
				doColorPop(hDC);
				popCTM(hDC, 0);
				if (bDebug > 1) DebugShowCTM(hDC, "After pop CTM");
			}

			if ((s = strstr(line, "setgray")) != NULL) {	/* 99/Apr/16 */
				double fk;
				int r, g, b;
				s--;
				while (s >= line && *s == ' ') s--;
				while (s >= line && *s > ' ') s--;
				s++;
				if (sscanf(s, "%lg", &fk) > 0) {
					if (bDebug > 1) OutputDebugString(s);
					r = g = b = (int) (fk * 255.999);
					if (OldFore >= 0 || OldBack >= 0) {
						r = 255 - r; g = 255 - g; b = 255 - b;
					} 
					CurrentTextColor = RGB(r, g, b);
					OldTextColor = SetTextColor(hDC, CurrentTextColor);
					SetupRuleColor (hDC, r, g, b);				/* 95/Mar/8 */
				}
			}

			if ((s = strstr(line,
					"currentpoint currentpoint translate")) != NULL &&
				strstr(line, "neg exch neg exch translate") != NULL) {
//				winerror(line); // debugging only
/*				if (strstr(line, "gsave") != NULL) {
					if (bDebug > 1) OutputDebugString("push CTM\n");
					pushCTM(hDC, 0);
				} */ 	/* rotate start */ 	/* moved 99/Apr/16 */
				s += 36;	/* currentpoint currentpoint translate */
				if (strstr(s, "rotate") != NULL) {		/* rotate start */
					double theta;
					if (bDebug > 1)	DebugShowCTM(hDC, "Before rotate CTM" );
					if (sscanf(s, "%lg%n", &theta, &n) == 1) {
//							take possible PS "neg" command into account
						if (strstr(s+n, "neg") < strstr(s+n, "rotate"))
							theta = - theta;
						if (bDebug > 1) {
							if (bFlipRotate) OutputDebugString("Flipped ");
							sprintf(debugstr, "rotate CTM %lg\n", theta);
							OutputDebugString(debugstr);
						}
						if (bFlipRotate) rotateCTM(hDC, theta, 1);
						else rotateCTM(hDC, theta, 0);
					}
					else if (bDebug > 1) OutputDebugString(line);
					if (bDebug > 1)	DebugShowCTM(hDC, "After rotate CTM" );
				}
				else if (strstr(s, "scale") != NULL) {	/* scale start */
					if (bDebug > 1)	DebugShowCTM(hDC, "Before scale CTM" );
					if (strstr(s, "div") != NULL) {		/* scale end */
						double sx, sy;
						int flag = 0;	// 1
						if (sscanf(s, "1 %lg div 1 %lg div", &sx, &sy) == 2){
							sx = 1.0 / sx;
							sy = 1.0 / sy;
//							sx = sy = 1.0;
							if (bDebug > 1) {
								sprintf(debugstr,
										"scale* CTM %lg %lg\n", sx, sy);
								OutputDebugString(debugstr);
							}
//							sprintf(debugstr, "scale %lg %lg", sx, sy);
//							winerror(debugstr);	// debugging only
							scaleCTM(hDC, sx, sy, flag);
						}
						else if (bDebug > 1) OutputDebugString(line);
					}
					else {								/* scale start */
						double sx, sy;
						int flag = 0;
						if (sscanf(s, "%lg %lg", &sx, &sy) == 2){
							if (bDebug > 1) {
								sprintf(debugstr,
										"scale CTM %lg %lg\n", sx, sy);
								OutputDebugString(debugstr);
							}
//							sprintf(debugstr, "scale %lg %lg", sx, sy);
//							winerror(debugstr);	// debugging only
							scaleCTM(hDC, sx, sy, flag);
						}
						else if (bDebug > 1) OutputDebugString(line);
					}
					if (bDebug > 1)	DebugShowCTM(hDC, "After scale CTM" );
				}
			} /* end of "currentpoint currentpoint translate" */
			else if (bUseCTMflag && strstr(line,
			 "currentpoint grestore moveto") != NULL) {	/* rotate end */
/*				this isn't quite right since it preserves the current point ? */
//				if (bDebug > 1) OutputDebugString("pop CTM\n");
//				(void) popCTM(hDC, 1);
//					already done earlier ? 99/Aug/21
			} /* end of "currentpoint grestore moveto" */
		}  /* end of bUseCTMFlag etc. */
#endif	/* end of ifdef AllowCTM */

/*		could check for presence of : or / here */
/*		how does SciWord deal with scaling ? */
		else if (bSciWord != 0) {	/* just has the EPS file name here */
/*			if (gettoken(input, epsfilename, sizeof(epsfilename)) > 0) */
			if (*line != '\0' && strlen(line) < sizeof(epsfilename)) {
				strcpy(epsfilename, line);
#ifdef DEBUGEPSPATH
				if (bDebug > 1) OutputDebugString(epsfilename);
#endif
				startspecial(); 
/*				copyepsfile(hDC, epsfilename, 1, -1); */ /* inc & shift */
				copyepsfileclip(hDC, epsfilename); /* inc & shift */
				endspecial();	
			}
			else complainspecial(input);	/* 94/April/21 experiment */
		}
/*		token after ps: is not recognized, must be raw PostScript */
/*		else if (verbatimflag != 0) */
		else if (verbatimflag != 0) { 
/*			fprintf(output, "\nrevscl ");  */
/*			if (preservefont != 0) fprintf(output, "currentfont ");	*/ 
/*			putc('\n', output); */
			(void) an_winseek(input, fliteral);
			nspecial = nliteral;
			copystring(input);	
/*			if (preservefont != 0) fprintf(output, "setfont "); */
/*			fprintf(output, "forscl "); */
		}
/*		else complainspecial(input); */ /* flush complaints ??? 96/Oct/2 */
	}
	flushspecial(input);
}

/* Terminated by : => ArborText style special, DVITOPS or DVITPS or EM */
/* Separator is ':' *//* or HPTAG tiff: */

void readdvilaserps (HDC hDC, HFILE input) { 
	int n;
	char *s;
	if (strcmp(line, "ps") == 0) readdvips(hDC, input);
	else if (strcmp(line, "dvitops") == 0) readdvitops(hDC, input, 0); 
	else if (strcmp(line, "eps") == 0) readdvitops(hDC, input, 1); 
	else if (strcmp(line, "dvitps") == 0) readdvitps(hDC, input); 
	else if (strcmp(line, "em") == 0) reademtex(hDC, input); 
	else if (strcmp(line, "tiff") == 0) dohptag(hDC, input);
	else if (strcmp(line, "color") == 0)
		DoColor(hDC, input, ':');	/* Allow colon after color 95/June/21 */
	else if (strcmp(line, "clip") == 0)
		DoClipBox(hDC, input, ':');	/* Allow colon after clip 98/Sep/10 */
/*	we actually ignore background special in this pass */
	else if (strcmp(line, "background") == 0)
/*		DoBackGround(hDC, input, ':'); */ 	/* changed 99/Feb/21 */
		flushspecial(input); 
	else if (strcmp(line, "comment") == 0) flushspecial(input); /* 95/June/21*/
	else if (strcmp(line, "PDF") == 0) flushspecial(input);		/* 96/July/4 */
#ifdef AllowCTM
	else if (strcmp(line, "CTM") == 0) doCTM(hDC, input);	/* 95/Oct/10 */
#else
	else if (strcmp(line, "CTM") == 0) flushspecial(input);
#endif
	else if (strcmp(line, "message") == 0) flushspecial(input); /* 95/June/21*/
	else if (strcmp(line, "src") == 0) {			/* 98/Nov/5 */
		(void) scanspecial(input, line, sizeof(line)); /* read rest of line */
		if (bMarkSearch > 0 || bShowButtons != 0) {
/*			if (srctaghit == 0) */
			if (srctaghit == 0 || bMarkSearch == 1)	{	/* 98/Dec/20 */
				if (sscanf(line, "%d%n", &srclineno, &n) == 1) {
					s = line + n;
					while (*s == ' ') s++;
					strcpy(srcfile, s);
				}
			}
		}
		if (bMarkSearch == 1) {			/* inverse search 98/Nov/5 */
			if (srctaghit) finish = -1;		/* already hit cursor 98/Dec/20 */
		}
		else if (bMarkSearch == 2) {	/* forward src search 98/Dec/12 */
			if (srctaghit == 0) {
				if (_stricmp(srcfile, szSource) == 0 &&
					srclineno >= nSourceLine) {
/* remember where first saw the matching source tag ??? */
					srctaghit++;
					finish = -1;	/* can stop the search now */
				}
			}
		}
		if (bShowButtons) {		/* show SRC specials */
			int dx = 1500000, dy = 500000;
			int xll = dvi_h, yll = dvi_v;
			int xur = xll + dx, yur = yll - dy;
			int dxll = mapx(xll), dyll = mapy(yll);
			int dxur = mapx(xur), dyur = mapy(yur);
			DrawFigureBox(hDC, dxll, dyll, dxur, dyur);	/* ... */
			CaptionFontSize = CaptionFontSize / 2;
			sprintf(str, "%d", srclineno);
			ShowNameFile(hDC, dxll, dyll, dxur, dyur, str);
			CaptionFontSize = CaptionFontSize * 2;
		}
		flushspecial(input);
	}
/*	seems like a bad idea, since get here when delimited by : ? */
	else if (verbatimflag == 0) complainspecial(input);			/* 95/Oct/12 */
	else if (bAllowAndrew != 0) {
		if (readandrew(hDC, input) == 0) 	/*  try OzTeX ? go south ! */
			complainspecial(input);			/* don't understand special! */
	}
/*	else if (bComplainSpecial) complainspecial(input); */
	else complainspecial(input);			/* don't understand */
}

/* main entry point to this part of it - DVISPECI.C stuff */

/* read and analysize special command */ /* given input file and length */
/* we get here when DVIWindo doesn't recognize a special as one of its own */
/* already have set up alphanumeric token in `line' and terminator is c */

void analspecial(HDC hDC, HFILE input, int c) {

/*	was absolutely stupid verbatim mode */
/*	c = an_wingetc(input); --nspecial;
	if (c == '\"') {	 
 		copystring(input); return;
	}
	an_unwingetc(c, input); nspecial++; */

/*	sprintf(str, "token: %s, terminator: `%c'\n", line, c); 
	wincancel(str);	*/

/*	Try and recognize style from terminator of first token and first token */
	if (c == '=') readdvi2ps(hDC, input);				/* DVI2PS style */
	else if (c == ':') readdvilaserps(hDC, input);		/* DVIPS sty ? */
/*	If none of the above, then try Textures styles	(' ' terminated) */
	else if (strcmp(line, "illustration") == 0) readtextures(hDC, input);
	else if (strcmp(line, "postscript") == 0) copypostscript(input, 0);
	else if (strcmp(line, "rawpostscript") == 0) copypostscript(input, 1);
	else if (strcmp(line, "postscriptfile") == 0)
		readpostscript(hDC, input, 0);
	else if (strcmp(line, "rawpostscriptfile") == 0)
		readpostscript(hDC, input, 1);
	else if (strcmp(line, "picture") == 0) readpicture(input);
	else if (strcmp(line, "color") == 0) DoColor(hDC, input, c);/* 95/Feb/26 */
	else if (strcmp(line, "clip") == 0) DoClipBox(hDC, input, c);
/*	we actually ignore background special in this pass */
	else if (strcmp(line, "background") == 0)
/*		DoBackGround(hDC, input, ':'); */ /* changed 99/Feb/21 */
		flushspecial(input); 
	else if (strcmp(line, "landscape") == 0)	/* 99/Apr/5 */
		flushspecial(input); 
/*  have to do DVIPS / Texture style FIRST, or they get caught by following: */
/*	else if (c == ' ' && nspecial > 0) readdvialw(hDC, input); */
	else if (c == ' ' && nspecial > 0) {				/* 92/Dec/12 */
		if (bAllowTPIC == 0 || readtpic(hDC, input) == 0) {
			readdvialw(hDC, input); /* DVIALW */
		}
	}
	else if (bAllowTPIC == 0 || readtpic(hDC, input) == 0) { /* 92/Dec/12 */
		if (bAllowAndrew == 0 || readandrew(hDC, input) == 0) {	
												/*  try OzTeX ? go south ! */
			complainspecial(input);				/* don't understand special! */
		}
	}
}

/* following moved here because of compiler error ... */

/* ********************************************************************** */

#ifdef LONGNAMES
static unsigned int ureadtwo (HFILE input) {
	unsigned char buffer[2];
	unsigned int c, d;
	_lread(input, buffer, 2);
	c = buffer[0]; d= buffer[1];
	if (leastfirst != 0) return ((d << 8) | c);
	else return ((c << 8) | d);
}

static unsigned long ureadfour(HFILE input) {
	unsigned char buffer[4];
	unsigned long c, d, e, f;
	unsigned long result;				/* work around ? */
	_lread(input, buffer, 4);
	c = buffer[0]; d = buffer[1]; e = buffer[2]; f = buffer[3];
	if (leastfirst != 0) {
/*		return ((((((f << 8) | e) << 8) | d) << 8) | c); */
		result = f; result = result << 8;
		result = result + e; result = result << 8;
		result = result + d; result = result << 8;
		result = result + c;
		return result;
	}
	else return ((((((c << 8) | d) << 8) | e) << 8) | f);
}

#else
static unsigned int ureadtwo(FILE *input) {
	unsigned int c, d;
	c = (unsigned int) getc(input);
	d = (unsigned int) getc(input);	
	if (leastfirst != 0) return ((d << 8) | c);
	else return ((c << 8) | d);
}

static unsigned long ureadfour(FILE *input) {
	unsigned long c, d, e, f;
	c = (unsigned long) getc(input);
	d = (unsigned long) getc(input);	
	e = (unsigned long) getc(input);	
	f = (unsigned long) getc(input);		
	if (leastfirst != 0) return ((((((f << 8) | e) << 8) | d) << 8) | c);
	else return ((((((c << 8) | d) << 8) | e) << 8) | f);
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

BITMAPFILEHEADER bmfh;

BITMAPINFOHEADER bmih;

/* RGBQUAD colortable[256]; */

int dpifrom(int res) {
	double dres = (double) res * 25.4 / 1000.0;
	dres = (double) ((int) (dres * 10.0 + 0.5)) / 10.0;
	return (int) (dres + 0.5);
}

#ifdef LONGNAMES
int readbmpfields (HFILE input) { 
#else
int readbmpfields (FILE *input) {
#endif
	long nLen;
	double dw, dh;

#ifdef DEBUGBMP
	if (bDebug > 1) OutputDebugString("ReadBMPFields");
#endif
	ImageWidth = ImageLength = -1;
	SamplesPerPixel = BitsPerSample = 1;	/* for BMP only */
	ExtraSamples = 0;
	compression = 0; Orientation = 1;
	StripOffset = -1; StripByteCount = -1;
	PhotometricInterpretation = 1;			/* (uncertain) default */
	PlanarConfiguration = 1; Predictor = 1;
	colormap = 0;	/* pointer to map in case of Palette Color Images */
	mingrey = -1; maxgrey = -1;
	xresnum = yresnum = nDefaultTIFFDPI;		/* 96/Apr/3 */	
	xresden = yresden = 1;				/* 93/Oct/20 */
	ResolutionUnit = 2;					/* default dpi */	/* 93/Oct/20 */

	fseek(input, 0, SEEK_END);
	nLen = ftell(input);				/* Length of file */
	fseek(input, 0, SEEK_SET);			/* rewind */
#ifdef LONGNAMES
	_lread(input, &bmfh, sizeof(bmfh));		/*	read file header */
#else
	fread(&bmfh, sizeof(bmfh), 1, input);	/*	read file header */
#endif
	if (bmfh.bfType != (77 * 256 + 66)) {			/*  "BM" */
		sprintf(debugstr, "Not BMP file %X\n", bmfh.bfType);
		winbadimage(debugstr);
		return -1;
	}
#ifdef DEBUGBMP
	if (bDebug > 1) {
		sprintf(debugstr, "BMFH: Size %ld OffBits %ld", bmfh.bfSize, bmfh.bfOffBits);
		OutputDebugString(debugstr);
	}
#endif
/*	bmfh.bfSize file size in words ? NO in bytes */
/*	bmfh.bfOffBits offset from end of header ? NO from start */
	OffBits = bmfh.bfOffBits;
#ifdef LONGNAMES
	_lread(input, &bmih, sizeof(bmih));	/*	read file header */
#else
	fread(&bmih, sizeof(bmih), 1, input);	/*	read bitmap info header */
#endif
/*	if (bmih.biClrUsed > 0) nColor = bmih.biClrUsed;
	else if (bmih.biBitCount < 24) nColor = 1 << bmih.biBitCount; */
	if (bmih.biClrUsed > 0) nColors = bmih.biClrUsed;
	else if (bmih.biBitCount < 24) nColors = 1 << bmih.biBitCount;
	else nColors = 0;
	ImageWidth = bmih.biWidth;
	ImageLength = bmih.biHeight;
	if (bmih.biBitCount < 24) {			/* for BMP only */
		BitsPerSample = bmih.biBitCount;
		SamplesPerPixel = 1;
	}
	else {								/* for BMP only */
		BitsPerSample = 8;
		SamplesPerPixel = 3;
	}
#ifdef DEBUGBMP
	if (bDebug > 1) {
		sprintf(debugstr, "BMIH: %d x %d bits %d x %d",
				ImageWidth, ImageLength, SamplesPerPixel, BitsPerSample);
		OutputDebugString(debugstr);
		sprintf(debugstr,
				"BMIH: %ld Planes %d Compression %d SizeImage %ld ColorUsed %d",
				bmih.biSize, bmih.biPlanes,
				bmih.biCompression, bmih.biSizeImage, bmih.biClrUsed);
		OutputDebugString(debugstr);
	}
#endif
	ImageSize = bmih.biSizeImage;

/* "Size of header %lu\n", bmih.biSize */
/* "Number of image planes %u\n", bmih.biPlanes */
/* "Compression %lu\n", bmih.biCompression */
/* "Size of compressed image %lu\n", bmih.biSizeImage */
/* "Number of colors used %lu %d\n", bmih.biClrUsed */

/* "Horizontal pixel per meter %ld\n", bmih.biXPelsPerMeter */
	xresnum = dpifrom(bmih.biXPelsPerMeter);
	if (xresnum == 0) xresnum = nDefaultTIFFDPI;
	dw = (double) ImageWidth / xresnum;
/* "%d dpi (horizontal)\twidth %lg inch\n", xresnum, dw */

/* "Vertical pixel per meter %ld\n", bmih.biYPelsPerMeter */
	yresnum = dpifrom(bmih.biYPelsPerMeter);
	if (yresnum == 0) yresnum = nDefaultTIFFDPI;
	dh = (double) ImageLength / yresnum;
/* "%d dpi (vertical)\theight %lg inch\n", yresnum, dh */
#ifdef DEBUGBMP
	if (bDebug > 1) {
		sprintf(debugstr, "%d x %d dpi", xresnum, yresnum);
		OutputDebugString(debugstr);
	}
#endif

	compression = bmih.biCompression;	/* save it */
	colormap = 0;
	if (nColors > 0) {		/*	read color table */
		if (nColors > 256) {
			sprintf(debugstr, "ERROR: too many colors: %d\n", nColors);
			winbadimage(debugstr);
			nColors = 256;
			return -1;
		}
		colormap = ftell(input);
		tiffoffset = 0;
		colormaplength = sizeof(RGBQUAD) * nColors;
/*		fread(&colortable, sizeof(RGBQUAD), nColors, input); */
	}
	if (OffBits == 0)	/* fallback, if header omitted this information */
		OffBits= sizeof(bmfh) + sizeof(bmih) + sizeof(RGBQUAD) * nColors; 
	fseek(input, OffBits, SEEK_SET);
/*	ImageSize = bmih.biSizeImage; */
	if (ImageSize == 0)	/* fallback, if header omitted this information */
		ImageSize = nLen - OffBits;
#ifdef DEBUGBMP
	if (bDebug > 1) {
		sprintf(debugstr, "Image at %ld size %ld bytes", OffBits, ImageSize);
		OutputDebugString(debugstr);
	}
#endif
	return 0;
}

#ifdef LONGNAMES
int readbmpfile (HDC hDC, HFILE input, 
				 int xll, int yll, int xur, int yur, int readflag) {
#else
int readbmpfile (HDC hDC, FILE *input, 
					 int xll, int yll, int xur, int yur, int readflag) {
#endif
	long nlen;

	(void) readbmpfields(input);	/* read tag fields in BMP file */

	BitsPerPixel = BitsPerSample * SamplesPerPixel;		/* for BMP only */

	if (readflag == 0) return 0;	/* if only reading for info */

#ifdef DEBUGBMP
	if (bDebug > 1) {
		sprintf(debugstr, "Allocating %d bytes", ImageSize);
		OutputDebugString(debugstr);
	}
#endif
	hImage = GlobalAlloc(GMEM_MOVEABLE, ImageSize);
	lpImageBytes = (LPSTR) GlobalLock(hImage);
/*	fseek(input, 2, SEEK_CUR); */		/* BUT WHY ??? */
	fseek(input, OffBits, SEEK_SET); 	/* Needed ??? experiment */
/*	memset(lpImageBytes, 127, ImageSize); */	/* test */
	if (bUserAbort == 0) {
		long current;
		current = ftell(input);
#ifdef LONGNAMES
		nlen = _lread(input, lpImageBytes, (unsigned int) ImageSize);
#else
		nlen = fread(lpImageBytes, 1, ImageSize, input);
#endif

#ifdef DEBUGBMP
		if (bDebug > 1) {
			sprintf(debugstr, "Read %d bytes from %ld", nlen, current);
			OutputDebugString(debugstr);
		}
#endif
	}
/*	memset(lpImageBytes, 127, ImageSize); */	/* test */
#ifdef IGNORED
	{
		char *s = lpImageBytes;
		int i, j;
		for (i = 0; i < 256; i++) {
			for (j = 0; j < 256; j++) {
				*s++ = (char) i;		/* B ? */
				*s++ = (char) j;		/* G ? */
				if ((i % 16) == 0 || (j % 16) ==0) *s++ = 255; /* R ? */
				else *s++ = 0;
			}
		}
	}
#endif
	fseek(input, colormap, SEEK_SET);	/*	color map read in renderimage */ 
	if (bUserAbort == 0) {	
#ifdef DEBUGBMP
		if (bDebug > 1) {
			sprintf(debugstr, "ReadBMPFile: xll %d yll %d xur %d yur %d\n",
				xll, yll, xur, yur);
			OutputDebugString(debugstr);
		}  			/* debugging 95/April/13 */
#endif
		(void) renderimage(hDC, input, xll, yll, xur, yur);
	} 
	else {
		if (bDebug > 1) OutputDebugString("Renderimage abort\n");
	}

	if (hImage != NULL) {
		if (GlobalUnlock(hImage) > 0) {
			sprintf(debugstr, "Lock count Not Zero %s", "Image");
			wincancel(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
		hImage = GlobalFree(hImage);
/*		hImage = NULL; */				/* for debugging */
	}	
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* warning: this is the only module that uses floating point operations */

/* may need to link in win87em.lib ??? */

/* in many cases this needs to read EPS file just to get Bounding Box */

/* try and separate reading of EPS file and reading of BMP file */

/* Link to TIFFREAD library dynamically. Avoids startup problem if absent */

/* (3 * 8) color in compressed mode has not been tested */

/* CreateCompatibleDC won't work in MetaFiles */
/* CreateBitMap won't work in MetaFiles ? */
/* CreatePalette should be OK */
/* SelectObject returns 0 or 1 in MetaFile */
/* SelectPalette returns 0 or 1 in MetaFile */

/* could speed up TPIC drawing using Polyline() instead of LineTo */

/* Across page color continuity only works if pages are done */
/* sequentially 98/Feb/14 */
/* and if CarryColor=1 appears in [Window] section of dviwindo.ini */
/* This won't work if you zoom in, use PREVIOUS, or jump */

/* some weird problem when DEBUGTIFF is defined */
/* with LoadProfile32 crashing when task bar uncovers part of window */
