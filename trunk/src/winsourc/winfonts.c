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

#include <math.h>				/* needed for sqrt in WriteAFM... */
#include <direct.h>				/* for _mkdir */
#include <malloc.h>

#include "atm.h"
#include "atmpriv.h"

/* #include "afmtotfm.h" */

#define OUT_SCREEN_OUTLINE_PRECIS      9
/* Allows OUtlineTextMetrics (et al) for currently selected printer DC T1 */

// #ifdef USEUNICODE
// typedef struct {
// 	char *glyphname;
// 	unsigned short UID;
// } UNICODE_MAP;
// #endif

#pragma warning(disable:4100)	// unreferenced formal parameters

#define AFMTOTFMDLL

#define DEBUGFONTSEARCH

#define DEBUGTABLES

// #define DEBUGHEAP

// #define DEBUGFONTCHECK 

// #define DEBUGFONTSELECT 

#define DEBUGREGISTRY

#define DEBUGAFMNAME

#define DEBUGWRITEAFM

#define DEBUGCHARMETRICS

/* #define DEBUGCOPY */

/* #define DEBUGATM */

/* #include <shellapi.h> */		/* for Registration Data Base HKEY_CLASSES_ROOT */

/* Copyright (C) 1991, 1992 Y&Y. All rights reserved. */
/* DO NOT COPY OR DISTRIBUTE! */

/****************************************************************************

	PROGRAM: winfonts.c

	PURPOSE: Take some infrequently used code out of dviwindo.c

	Mostly for font display

****************************************************************************/

#define ITALICFUZZ 30

typedef struct{
	short etmSize;			/* size of structure */
	short etmPointSize;		/* nominal point size */
	short etmOrientation;	/* 0 => either, 1 => portrait, 2 => landscape */
	short etmMasterHeight;	/* font size for which extent table is exact */
	short etmMinScale;		/* min valid size */
	short etmMaxScale;		/* max valid size */
	short etmMasterUnits;	/* integer numberof units per em */
	short etmCapHeight;		/* height of upper case, typically of `H' */
	short etmXHeight;		/* height of lower case, typically of `x' */
	short etmLowerCaseAscent;	/* ascent of lower case, typically of `d' */
	short etmLowerCaseDescent;	/* descent of lower case, typically of `p' */
	short etmSlant;			/* clockwise angle in tenth of degree */
	short etmSuperScript;	/* offset of superscript from base (negative) */
	short etmSubScript;		/* offset of subscript from base (positive) */
	short etmSuperScriptSize;
	short etmSubScriptSize;
	short etmUnderlineOffset;
	short etmUnderlineWidth;
	short etmDoubleUpperUnderlineOffset;
	short etmDoubleLowerUnderlineOffset;
	short etmDoubleUpperUnderlineWidth;	
	short etmDoubleLowerUnderlineWidth;	
	short etmStrikeOutOffset;
	short etmStrikeOutWidth;
	WORD etmKernPairs;	/* number of character kerning pairs */
	WORD etmKernTracks;	/* number of kerning tracks */
} EXTTEXTMETRIC;

/* typedef struct {
	union {
		BYTE each [2]; 
		WORD both;
	} kpPair;
	short kpKernAmount;
} KERNPAIR; */			/* used by GETPAIRKERNTABLE ExtEscape() */

typedef struct kernpair { 
/*	unsigned int kppair; int kpamount; */	/* this is wrong in WIN32 */
	WORD kpPair; short kpKernAmount;		/* ??? */
} KERNPAIR;				/* used by GETPAIRKERNTABLE ExtEscape() */

/* NOTE: above *DIFFERENT* from KERNINGPAIR struct used by GetKerningPairs */

/* typedef struct tagKERNINGPAIR
{
    WORD wFirst;
    WORD wSecond;
    int  iKernAmount;
} KERNINGPAIR, FAR* LPKERNINGPAIR; */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define LF_FULLFACESIZE     64 */

/* Structure passed to FONTENUMPROC */
/* NOTE: NEWTEXTMETRIC is the same as TEXTMETRIC plus 4 new fields */

/* typedef struct tagNEWTEXTMETRIC
{
    int     tmHeight;
    int     tmAscent;
    int     tmDescent;
    int     tmInternalLeading;
    int     tmExternalLeading;
    int     tmAveCharWidth;
    int     tmMaxCharWidth;
    int     tmWeight;
    BYTE    tmItalic;
    BYTE    tmUnderlined;
    BYTE    tmStruckOut;
    BYTE    tmFirstChar;	
    BYTE    tmLastChar;		
    BYTE    tmDefaultChar;	
    BYTE    tmBreakChar;
    BYTE    tmPitchAndFamily;
    BYTE    tmCharSet;
    int     tmOverhang;
    int     tmDigitizedAspectX;
    int     tmDigitizedAspectY;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRIC;
typedef NEWTEXTMETRIC*       PNEWTEXTMETRIC;
typedef NEWTEXTMETRIC NEAR* NPNEWTEXTMETRIC;
typedef NEWTEXTMETRIC FAR*  LPNEWTEXTMETRIC; */

/* NEWTEXTMETRIC structure is same as TEXTMETRIC except for last four items */
/* These last four are only filled in for a TrueType font */

/* ntmFlags field flags */

/* #define NTM_REGULAR	0x00000040L */
/* #define NTM_BOLD		0x00000020L */
/* #define NTM_ITALIC	0x00000001L */

/* Structure passed to FONTENUMPROC */
/* typedef struct tagENUMLOGFONT
{
    LOGFONT elfLogFont;
    char    elfFullName[LF_FULLFACESIZE];
    char    elfStyle[LF_FACESIZE];
} ENUMLOGFONT, FAR* LPENUMLOGFONT; */

/* last element of LOGFONT is `Face Name' */
/* two members added to LOGFONT structure only for TrueType fonts */

/* EnumFonts font type values */ /* from wingdi.h */

/* In Windows 95 ATM fonts show up as DEVICE_FONTTYPE, *NOT* in NT */
/* Font Type of Type 1 fonts is 0 (zero!) in Windows NT (for now) UGH */

/* #define RASTER_FONTTYPE     0x0001 */
/* #define DEVICE_FONTTYPE     0X0002 */
/* #define TRUETYPE_FONTTYPE   0x0004 */

#define TTF_DONE 1			/* 98/Nov/20 */
#define PFM_DONE 2			/* 98/Nov/20 */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  */

/* Define a structure used to hold info about an app we */
/* start with ExecApp so that this info can be used again */
/* later to test if the app is still running */

// typedef struct _EXECAPPINFO {
//     HINSTANCE hInstance;            /* The instance value */
//     HWND hWnd;                      /* The main window handle */
//     HTASK hTask;                    /* The task handle */
// } EXECAPPINFO, FAR *LPEXECAPPINFO;


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Added 1995/August/2 for registry files - for Windows 95 and NT 4.0 */
/* For Windows NT need to replace "Windows" with "Windows NT" */
/* But in Windows NT regedt32.exe writes a different form of file anyway */
/* --- unless we have the New Shell */

/* char *szRegistryFonts=
 "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts"; */

// char szRegistryFonts[128]="";	/* constructed dynamically now */
char *szRegistryFonts=NULL;			/* constructed dynamically now */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* The following are all in WINSEARC.C 1994/Dec/22 */

/* #define MyATMIsUp ATMIsUp */
/* in atm.h */
#define MyATMGetVersion ATMGetVersion
#define MyATMFontSelected ATMFontSelected
#define MyATMGetFontBBox ATMGetFontBBox
#define MyATMGetOutline ATMGetOutline
#define MyATMFontAvailable ATMFontAvailable
#define MyATMSelectObject ATMSelectObject
#define MyATMGetPostScriptName libATMGetPostScriptName
#define MyATMGetFontPaths libATMGetFontPaths
/* #define MyATMGetMenuName libATMGetMenuName */
/* in atmpriv.h */
/* #define MyATMSelectEncoding libATMSelectEncoding */
#define MyATMGetGlyphList libATMGetGlyphList
/* #define MyATMMarkDC libATMMarkDC */
#define MyATMGetFontInfo libATMGetFontInfo

#ifdef IGNORED
extern WORD (WINAPI *MyATMGetVersion) (VOID);
extern BOOL (WINAPI *MyATMFontSelected) (HDC);
extern int  (WINAPI *MyATMGetFontBBox) (HDC, LPATMBBox);
extern int  (WINAPI *MyATMGetOutline) (HDC, char, LPATMFixedMatrix,
                          FARPROC, FARPROC, FARPROC, FARPROC, LPSTR);
extern int  (WINAPI *MyATMFontAvailable) (LPSTR, int, BYTE, BYTE, BYTE,
						BOOL FAR *);
extern int  (WINAPI *MyATMSelectObject) (HDC, HFONT, WORD, HFONT FAR *);
/* the following needed the API BD_VERSION number */
extern int  (WINAPI *MyATMGetPostScriptName) (int, LPSTR, LPSTR);
extern int  (WINAPI *MyATMGetGlyphList) (int, HDC, FARPROC, LPSTR);
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

TEXTMETRIC TextMetric;			/* used by font sample module */

/* int charwidths[MAXCHRS]; */		/* character widths for font */
short int charwidths[MAXCHRS];		/* character widths for font */
									/* used by test function CHARWIDTHS */

/* int extwidths[MAXCHRS]; */	/* character widths for font - test only */

BOOL bTexFontFlag;				/* non-zero if likely to be TeX font */
BOOL bCheckBoldFlag = 0;		/* checked font desired is bold */
BOOL bCheckItalicFlag = 0;		/* checked font desired is italic */
BOOL bCheckSymbolFlag = 0;		/* font is symbol/decorative/math */
BOOL bCheckReencode = 0;		/* reencode font being shown */
BOOL bSyntheticFlag = 0;		/* synthesized font */

BOOL bShowHidden=0;				/* info in AFM file on char without name */

/* BOOL redosizes = 0; */

int testcharset = 0;				/* CharSet of selected font */
int testpitchandfamily = 0;			/* PitchAndFamily of selected font */
int testwantttf = 0;				/* test font is TTF if non-zero */
									/* negative for decorative/symbol */

/* BYTE CharSet[MAXOUTLINEFONTS]; */
BYTE CharSet[MAXOUTLINEFACES];				/* 1995/July/12 */

/* BYTE PitchAndFamily[MAXOUTLINEFONTS]; */
BYTE PitchAndFamily[MAXOUTLINEFACES]; 		/* 1995/July/12 */

/* BYTE IsTTF[MAXOUTLINEFACES]; */	/* bit wasteful - use (TT) instead */

int nFaceIndex = 0;			/* How many Faces */
int nTrueIndex = 0;			/* How many TT Faces */
int nFullIndex = 0;			/* index into Full Table - number loaded */
int nFileIndex = 0;			/* index into File Table - number loaded */

/* int SizeIndex = 0; */	/* how many sizes */

int CurrentFace = 0;		/* index of current font in FontList table */

/* int PreviousFont = 0; */	/* index of current font in FontList table */

/* int CurrentSize = 0; */	/* index of current size in SizeList table */

// LPSTR lpFaceNames=NULL;		/* FAR pointer to start of Font Name Table */
LPSTR *lpFaceNames=NULL;		/* pointer to Font Name Table */

// LPSTR lpCurrentName=NULL;	/* pointer into Font Name Table */
// LPSTR lpPreviousName=NULL;	/* pointer into Font Name Table */

LPSTR lpFullNames=NULL;		/* table of FaceName+Style => FullName */

LPSTR lpFileNames=NULL;		/* table of FaceName+Style => FileName */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

BOOL bTagFlag=0;					/* on while searching for tag */
int tagx, tagy;						/* spot marked for font info */
int taggedfont=-1;					/* font selected when tagged */
int taggedchar=0;					/* character tagged */
int taggedwidth=0;					/* character width */
// int ltaggedwidth;				/* the width of the tagged char*/
long ltaggedwidth;					/* the width of the tagged char*/

int StyleBits;

/* four flags orred to indicate which styles are available in this Face */
/* 1 => regular, 2 => italic, 4=> bold, 8 => bold-italic */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

BOOL bDoUnencodedGlyphs=1;			/* a test 1995/July/20 */
BOOL bDoReencodeKern=1;				/* another test 1995/July/20 */

BOOL bListUnencoded;/* non-zero => list unencoded rather than encoded glyph */
					/* global used in Glyph Enumeration callback routine */

int nglyph;			/* number of glyphs in enumeration so far */
int nencoded;		/* number of encoded glyphs */
int nunencode;		/* number of unencoded glyphs */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Adobe Standard Encoding just for AFM output of text font */

char *standardencoding[] = {					/* 1998/Aug/26 */
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

int standardcode(char *name) {				/* 1998/Aug/26 */
	int k;
	if (*name == '\0') return -1;			/* sanity check */
	for (k = 64; k < 128; k++) {
		if (strcmp(standardencoding[k], name) == 0) return k;
	}
	for (k = 32; k < 64; k++) {
		if (strcmp(standardencoding[k], name) == 0) return k;
	}
	for (k = 160; k < 256; k++) {
		if (strcmp(standardencoding[k], name) == 0) return k;
	}
	return -1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static char *modname =  "WINFONTS";

/*
static void winerror(char *mss) {
	HWND hFocus;
	char errmess[MAXMESSAGE];

	if (strlen(mss) > (MAXMESSAGE - 32)) mss = "Error Message too long!";
	strcpy(errmess, mss);

	if((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox (hFocus, errmess, modname, MB_ICONSTOP | MB_OK);
} */

static void winerror (char *message) {
	HWND hFocus;
	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox(hFocus, message, modname, MB_ICONSTOP | MB_OK);	
}

/* static void wininfo(char *message) */
static void wininfoTFM (char *message, char *TFMname) {
	HWND hFocus;
	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox(hFocus, message, TFMname, MB_ICONINFORMATION | MB_OK);
}

static int wincancel (char *buff) {				/* 1993/March/15 */
	HWND hFocus;
	int flag;

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, buff, modname, MB_ICONSTOP | MB_OKCANCEL);
	if (flag == IDCANCEL) return -1;
	else return 0;
}

static int wincancelTFM (char *buff, char *TFMname) {				/* 1993/March/15 */
	HWND hFocus;
	int flag;

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, buff, TFMname, MB_ICONSTOP | MB_OKCANCEL);
	if (flag == IDCANCEL) return -1;
	else return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* share common code to save space and ensure consistency */

char *SansPath (char *fname) { /* returns pointer to filename minus path */
	char *s;
	if ((s = strrchr(fname, '\\')) != NULL ||
		(s = strrchr(fname, '/')) != NULL ||
		(s = strrchr(fname, ':')) != NULL) return (s+1);
	else return fname;
}

void FlushExtension (char *fname) { /* remove extension, newline if present */
	char *s;
/*	if ((s = strchr(fname, '.')) != NULL) *s = '\0'; */
	if ((s = strrchr(fname, '.')) != NULL) *s = '\0';
	else if ((s = strchr(fname, '\n')) != NULL) *s = '\0';	
}

char *StripQuotes (char *name) {	/* remove double quote around string */
	char *s;
	if ((s = strchr(name+1, '\"')) != NULL) *s = '\0'; /* terminate final " */
	if (*name == '\"') return (name+1);		/* step over initial " */
	else return name;
}

char *StripTrueType (char *name) {	/* remove (TrueType) return NULL if not */
	char *s;

	if ((s = strstr(name, "(TrueType)")) == NULL) return NULL;
	while (s > name && *(s-1) <= ' ') s--;
	*s = '\0';
	return name;
}

void StripTTVersion (char *name) {	/* remove version number if possible */
	int c;
	if (*(name+6) != '_') return;		/* not safe without _ before */
	if (strlen(name) != 8) return;		/* not full 8 character name */
	c = *(name+7);
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
		(c >= 'a' && c <= 'f')) *(name+7) = '_';
}

void StripUnderScores (char *name) {	/* remove underscores from name */
	char *s;
	if (*(name+7) != '_') return;		/* not end in _ */
	if (strlen(name) != 8) return;		/* not full 8 character name */
	s = name + 7;
	while (s > name && *(s-1) == '_') s--;
	*s = '\0';
}

void UpperCase (char *name) {

	CharUpper(name);					/* 96/Sep/29 */

/*	AnsiUpper(name); */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// LPSTR GrabFaceNames (void) {
LPSTR *GrabFaceNames (void) {
//	LPSTR lpFaceNames=NULL;
	LPSTR *lpFaceNames=NULL;

	if (hFaceNames == NULL) winerror("No Fonts to grab");
	else {
//		lpFaceNames = (LPSTR) GlobalLock(hFaceNames);
		lpFaceNames = (LPSTR *) GlobalLock(hFaceNames);
		if (lpFaceNames == NULL) {
			winerror("Unable to lock Fonts"); 
/*			winerror("Unable to lock Face Names"); */
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
	return lpFaceNames;
}

void ReleaseFaceNames (void) {
	int n;
	if (hFaceNames == NULL) {
		winerror("No Fonts to release");
		return;
	}
	if ((n = GlobalUnlock(hFaceNames)) > 0) {
/*		if (bDebug) {
			sprintf(str, "%d\t", n);
			strcat(str, "Lock count Not Zero");
			strcat(str, " - ReleaseFaceNames");
			if (wincancel(str))	PostQuitMessage(0);
		} */
	}
	else {
		lpFaceNames = NULL;
//		lpCurrentName = NULL;
	}
}

/* 256 * (32 + 8) = 10,240 byte - quite a chunck 1995/July/7 */
/* 512 * (32 + 8) = 20,480 byte - quite a chunck 1995/July/7 */

void AllocFaceNames (void) {		/* grab space for FontNames - if needed */
	unsigned long n;
	if (hFaceNames != NULL) return;			/* already allocated */
/*	entries are character strings of length MAXFACENAME */
//	n = (unsigned long) MAXOUTLINEFACES * MAXFACENAME;
//	entries are pointers to character strings 
	n = (unsigned long) MAXOUTLINEFACES * sizeof(LPSTR);
/*	hFaceNames = GlobalAlloc(GMEM_MOVEABLE, */
	hFaceNames = GlobalAlloc(GHND, /* GMEM_MOVEABLE | GMEM_ZEROINIT */
/*				(unsigned long) MAXOUTLINEFONTS * MAXFACENAME); */
				n);
	if (hFaceNames == NULL) {
		winerror("Unable to allocate memory"); /* debug */
		bFontSample = 0;			/* sigh ... */
		bCharacterSample = 0;
		PostQuitMessage(0);			/* pretty serious ! */
	}
	nFaceIndex = 0;
	nTrueIndex = 0;
}

void FreeFaces (void) {
	int k;
	if (hFaceNames == NULL) return;
	lpFaceNames = GrabFaceNames();
	for (k = 0; k < MAXOUTLINEFACES; k++) {
		if (lpFaceNames[k] != NULL) {
			free(lpFaceNames[k]);
			lpFaceNames[k] = NULL;
		}		
	}		
	ReleaseFaceNames();
}

/* called only from dviwindo.c */

void FreeFaceNames (void) {
	if (hFaceNames == NULL) {
		if (bDebug > 1) winerror("No Fonts to free");
		return;
	}
	FreeFaces();				// 99/Nov/8
	hFaceNames = GlobalFree(hFaceNames);
	hFaceNames = NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* typedef struct {
	BYTE chFirst;
	BYTE chLast;
} CHAR_RANGE_STRUCT; */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *FormatStyle (char *buffer, int boldflag, int italicflag) {
	sprintf(buffer, "%s%s%s,", 
			(boldflag == 0 && italicflag == 0) ? "REGULAR" : "",
			boldflag ? "BOLD" : "",
			italicflag ? "ITALIC" : "");
	return buffer + strlen(buffer);
}

char *FormatNameStyle (char *buffer, char *fname,
					   int boldflag, int italicflag) {	/* 1995/Aug/20 */
	char *s; 
	strcpy(buffer, fname);
	strcat(buffer, ",");
	s = buffer + strlen(buffer);
	return FormatStyle (s, boldflag, italicflag);
	/* returns pointer to null at end */
}

char *FormatNewNameStyle (char *buffer, int m, char *szFaceName,
						  int nStyle, char *szFileName) {
	char *s;
	sprintf(buffer, "%3d %s,%s%s%s=%s\n", m, szFaceName,
			(nStyle & NTM_REGULAR) ? "REGULAR" : "",
			(nStyle & NTM_BOLD) ? "BOLD" : "",
			(nStyle & NTM_ITALIC) ? "ITALIC" : "",
			szFileName);
	s = buffer + strlen(buffer);
	return s;
}

#ifdef IGNORED
/*	sprintf(buffer, "%s,%s%s%s ", fname,
			(boldflag == 0 && italicflag == 0) ? "REGULAR" : "",
			boldflag ? "BOLD" : "",
			italicflag ? "ITALIC" : ""); 
	s = buffer + strlen(buffer); */
#endif

/* somewhat expensive in space to keep this as global ? */

/* char szTTFName[LF_FACESIZE + MAXMSEXTRA]; */	/* Face Name */
char szFaceName[LF_FACESIZE];				/* Face Name */
char szFullName[LF_FULLFACESIZE];			/* Full Name */
BOOL bBoldFlag, bItalicFlag;				/* Style Bits */
int nFullName;								/* size of longest match */

/* For TT, the `Face Name' is what is listed in font menus */
/* For TT, the `Full Name' is what appears in WIN.INI */
/* Somehow need to get from `Face Name' to `Full Name' ??? */

/*	See whether can find TrueType font with this name *//* 1992/Sep/17 */
/*	returns length of string of name set up in szFullName */
/*	returns zero if not found or fail some other way */
/*	returns file name in str */

int FullFromFace (HDC, char *, int, int);	/* defined at end */

char *ConstructFontKey0 (char *s) {		// split off 2000 June 2
	strcpy(s, "SOFTWARE\\Microsoft\\Windows");
	if (bWinNT) strcat(str, " NT");
	strcat(s, "\\CurrentVersion");
	strcat(s, "\\Fonts");
	return s;
}

char *ConstructFontKey (char *s) {		// split off 2000 June 2
	strcpy(s, "HKEY_LOCAL_MACHINE\\");
//	strcat(s, "SOFTWARE\\Microsoft\\Windows");
//	if (bWinNT) strcat(s, " NT");
//	strcat(s, "\\CurrentVersion");
//	strcat(s, "\\Fonts");
	ConstructFontKey0(s + strlen(s));
	return s;
}

/*	See whether can find TrueType font with this name *//* 1992/Sep/17 */
/*	returns length of string of name set up in szFullName */

/*	bCheckBoldFlag and bCheckItalicFlag global pass information */

/* int TryForTrue (char *testfont, int boldflag, int italicflag) */
int TryForTrue (HDC hDC, char *testfont, int boldflag, int italicflag) {
	int lenstr;
	char szTrueType[LF_FULLFACESIZE + 12];	/* space for " (TrueType)" */
	int m, nStyle=0, nStyleWant=0;
	LPSTR lpFileCurrent;					/* 95/Sep/20 */

/*	following is the *real* way to do this for TT fonts */

#ifdef DEBUGFONTSELECT
	if (bDebug > 1) OutputDebugString("TryForTrue\n");
#endif

/*	if (bWin95 && bUseRegEnumValue) */				/* 1995/Sep/20 */
	if (bUseTTMapping) {				/* default 1995/Sep/20 */
/*		check whether table has been set up and if not, set it up ? */
		if (hFileNames == NULL) SetupTTMapping(hDC);
		lpFileNames = GrabFileNames();
		if (lpFileNames == NULL) return 0;			/* super ugh ! */
/*		need to set up szFullName also ? */
/*		this returns length of string in szFullName - filename in str */
		lenstr = FullFromFace(hDC, testfont, boldflag, italicflag);
		if (lenstr == 0) return 0;					/* impossible */
		lenstr = 0; 
		*str = '\0';
		if (!boldflag && !italicflag) nStyleWant |= NTM_REGULAR;
		else {
			if (boldflag) nStyleWant |= NTM_BOLD;
			if (italicflag) nStyleWant |= NTM_ITALIC;
		}
/*		if (strcmp(testfont, "Marlett") == 0) nStyleWant = 0;*/ /* 96/Oct/12 */
		for (m = 0; m < nFileIndex; m++) {
			lpFileCurrent = lpFileNames + m * (LF_FACESIZE + 2 + MAXTFMNAME + 2);
/*			strcpy(szFaceName, lpFileCurrent); */
/*			if (_stricmp(szFaceName, testfont) != 0) continue; */
			if (strcmp(testfont, lpFileCurrent) != 0) continue;
			nStyle = *(lpFileCurrent + LF_FACESIZE + 1);
#ifdef DEBUGFONTSELECT
			if (bDebug > 1) {
				sprintf(debugstr, "testfont %s nStyle %x nStyleWant %x\n",
						testfont, nStyle, nStyleWant);
				OutputDebugString(debugstr);
			}
#endif
			if (nStyle != nStyleWant && nStyle != 0) continue;
/*			strcpy(szFileName, lpFileCurrent + LF_FACESIZE + 2); */
			strcpy(str, lpFileCurrent + LF_FACESIZE + 2);
/*			if (bDebug > 1) OutputDebugString(str); */
			lenstr = strlen(str);
			break;
		}
		ReleaseFileNames();
		if (bDebug > 1) {
			sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 1);
			OutputDebugString(debugstr);
		}
		return lenstr;
	}

/*	Get here if not Windows 95 or not using table made from registry data */
	if (szRegistry == NULL) return 0;	/* sanity - should not happen */

	if (bTTUsable) {
/*		result - if any - of following is setup in szFullName */
		lenstr = FullFromFace(hDC, testfont, boldflag, italicflag);
/*		if (bDebug > 1) {
			sprintf(debugstr, "FullFromFace\t%s\t%s\tlength %d\n",
					testfont, szFullName, lenstr);
			OutputDebugString(debugstr);
		} */
		if (lenstr > 0) {		// if we do have a FullName
/*			if (bDebug > 1) OutputDebugString(szFullName); */

/*	try in [TTFonts] of win.ini first (as set up by SETUPTTF) */
			strcat(szFullName, " (TrueType)");
			lenstr = GetProfileString("TTFonts", szFullName,
									  "", str, sizeof(str));
			if (lenstr > 0) {
				if (bDebug > 1) {
					sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 2);
					OutputDebugString(debugstr);
				}
				return lenstr;
			}
/*	try in [Fonts] of win.ini (as set up by SETUPTTF) */
//			if (bTTFontSection)
			lenstr = GetProfileString("Fonts", szFullName, "",
										  str, sizeof(str));
			if (lenstr > 0) {
				if (bDebug > 1) {
					sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 3);
					OutputDebugString(debugstr);
				}
				return lenstr;
			}

			if (bUseRegistryFile) {	/* use ttfonts.reg --- 1995/Aug/18 */
				if (szRegistryFonts == NULL) {	// modified 2000 June 2
					ConstructFontKey(str);
					szRegistryFonts = zstrdup(str);
				}
/* First try "<fontname> (TrueType)"="<filename>.ttf" style */
				strcpy (szTrueType, "\"");
				strcat (szTrueType, szFullName);
				strcat (szTrueType, " (TrueType)");			
				strcat (szTrueType, "\"");
				lenstr = GetPrivateProfileString(szRegistryFonts, szTrueType,
					"",	str, sizeof(str), szRegistry);
				if (lenstr > 0) {
					if (bDebug > 1) {
						sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 4);
						OutputDebugString(debugstr);
					}
					return lenstr;
				}
/*	Try also without the quotedbl, just in case user modified the file */
				if (lenstr == 0) {
					strcpy (szTrueType, szFullName);
					strcat (szTrueType, " (TrueType)");			
					lenstr = GetPrivateProfileString(szRegistryFonts,
						szTrueType, "",	str, sizeof(str), szRegistry);
				}
/* The quotedbl's in the `value' are removed by GetPrivateProfileString */
				if (lenstr > 0) {
					if (bDebug > 1) {
						sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 5);
						OutputDebugString(debugstr);
					}
					return lenstr;
				}
			}
/*	moved higher up to come before SetupTT */
/*			else {			
				strcat (szFullName, " (TrueType)");
				lenstr = GetProfileString("TTFonts", szFullName,
										  "", str, sizeof(str));
				if (lenstr == 0 && bTTFontSection)
					lenstr = GetProfileString("Fonts", szFullName, "",
											  str, sizeof(str));
				if (lenstr > 0) return lenstr;
			} */
		}	// end of we have a FUllName ...
/*		if (lenstr == 0) {
			if (bDebug > 1) {
				FormatNameStyle (debugstr, testfont, boldflag, italicflag);
				strcat(debugstr, " apparently not TT\n");
				OutputDebugString(debugstr);
			}
		} */
		if (bDebug > 1) {
			sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 6);
			OutputDebugString(debugstr);
		}
		return lenstr;
	}

/*	we get to this old heuristic approach *ONLY* if bTTUsable is false */
/*  `Demibold' instead of `Bold' */
/*  `Oblique' instead of `Italic' */
/*  `Regular' */ /* Medium ? */
/*	following is a kludge to try all combinations */ /* flush eventually */

/*	if (bDebug > 1) OutputDebugString("Heuristic Font Name Hack\n"); */
	if (boldflag == 0 && italicflag == 0) {		/* `regular' font */
		strcpy(szFullName, testfont);
		strcat(szFullName, " (TrueType)");
		lenstr = GetProfileString("Fonts", szFullName, "", str, sizeof(str)); 
		if (*str != '\0') {
			if (bDebug > 1) {
				sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 6);
				OutputDebugString(debugstr);
			}
			return lenstr;
		}
		strcpy(szFullName, testfont);
		strcat(szFullName, " Regular");
		strcat(szFullName, " (TrueType)");
		lenstr = GetProfileString("Fonts", szFullName, "", str, sizeof(str)); 
		if (*str != '\0') {
			if (bDebug > 1) {
				sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 7);
				OutputDebugString(debugstr);
			}
			return lenstr;
		}
		strcpy(szFullName, testfont);
		strcat(szFullName, " Medium");				/* 1995/Feb/4 */
		strcat(szFullName, " (TrueType)");
		lenstr = GetProfileString("Fonts", szFullName, "", str, sizeof(str)); 
		if (*str != '\0') {
			if (bDebug > 1) {
				sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 8);
				OutputDebugString(debugstr);
			}
			return lenstr;
		}
/*		kludge for Lucida Calligraphy Italic and  Lucida Handwriting Italic */
		strcpy(szFullName, testfont);
		strcat(szFullName, " Italic");
		strcat(szFullName, " (TrueType)");
		lenstr = GetProfileString("Fonts", szFullName, "", str, sizeof(str)); 
		if (bDebug > 1) {
			sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 9);
			OutputDebugString(debugstr);
		}
/*		return lenstr; */
		return -lenstr;				/* indicate its fake */
	}
	else {							/* either bold or italic or both */
		strcpy(szFullName, testfont);
		if (bCheckBoldFlag != 0) strcat(szFullName, " Bold");
		if (bCheckItalicFlag != 0) strcat(szFullName, " Italic");	
		strcat(szFullName, " (TrueType)");
		lenstr = GetProfileString("Fonts", szFullName, "", str, sizeof(str)); 
		if (*str != '\0') {
			if (bDebug > 1) {
				sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 10);
				OutputDebugString(debugstr);
			}
			return lenstr;
		}
		strcpy(szFullName, testfont);
		if (bCheckBoldFlag != 0) strcat(szFullName, " Demibold");
		if (bCheckItalicFlag != 0) strcat(szFullName, " Italic");	
		strcat(szFullName, " (TrueType)");
		lenstr = GetProfileString("Fonts", szFullName, "", str, sizeof(str)); 
		if (*str != '\0') {
			if (bDebug > 1) {
				sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 11);
				OutputDebugString(debugstr);
			}
			return lenstr;
		}
		strcpy(szFullName, testfont);
		if (bCheckBoldFlag != 0) strcat(szFullName, " Bold");
		if (bCheckItalicFlag != 0) strcat(szFullName, " Oblique");	
		strcat(szFullName, " (TrueType)");
		lenstr = GetProfileString("Fonts", szFullName, "", str, sizeof(str)); 
		if (*str != '\0') {
			if (bDebug > 1) {
				sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 12);
				OutputDebugString(debugstr);
			}
			return lenstr;
		}
		strcpy(szFullName, testfont);
		if (bCheckBoldFlag != 0) strcat(szFullName, " Demibold");
		if (bCheckItalicFlag != 0) strcat(szFullName, " Oblique");	
		strcat(szFullName, " (TrueType)");
		lenstr = GetProfileString("Fonts", szFullName, "", str, sizeof(str)); 
		if (bDebug > 1) {
			sprintf(debugstr, "TryForTrue: `%s' (%d)", str, 13);
			OutputDebugString(debugstr);
		}
		return lenstr;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* see whether font really exists in specified form or synthesized */
/* uses Windows 3.0 functions */ 

/* check only TrueType if ttfflag == 1 */
/* check only Type 1 if ttfflag == 0 */
/* check both if ttfflag < 0 */

/* returns -1 if font name is an alias */
/* returns 0 if not synthetic */
/* returns 1 if synthetic */

/* int IsSynthetic(char *name, int bBoldFlag, int bItalicFlag) */
/* int IsSynthetic(char *name, int bBoldFlag, int bItalicFlag, int ttfflag) {
*/

/* ttfflag > 0  => try TT font only */
/* ttfflag == 0 => try T1 font only */
/* ttfflag < 0  => try T1 font and then TT */ /* ever used ? */

BOOL IsSynthetic (HDC hDC, char *name, int bBoldFlag, int bItalicFlag,
				 int ttfflag) {
	int lenstr;
	char fontspec[LF_FACESIZE + MAXMSEXTRA];

	int bFromOutline=0;				/* 96/July/21 */

/*	BOOL bFromOutline=0; */

/*	if (bDebug > 1) {
		sprintf(debugstr, "ISSYNTHETIC: %s Bold %d Italic %d TTF %d\n",
			name, bBoldFlag, bItalicFlag, ttfflag);
		OutputDebugString(debugstr);
	} */  /* debugging 95/Mar/15 */

/*	First try for Type 1 font installed in ATM */
	if (ttfflag == 0 || ttfflag < 0) {
		if (bATMLoaded) {
			if (MyATMFontAvailable (name, bBoldFlag ? 700 : 400,
/*				(BYTE) bItalicFlag, 0, 0, &bFromOutline)) */
				(BYTE) (bItalicFlag != 0), 0, 0, &bFromOutline)) {
#ifdef DEBUGATM
				if (bDebug > 1) {
					char *s;
					s = FormatNameStyle (debugstr, name, bBoldFlag, bItalicFlag);
					sprintf(s, "From Outline: %d\n", bFromOutline);
					OutputDebugString(debugstr);
				}
#endif
				if (bFromOutline == 0) return 1;	/* synthesized */
/*				else return 0; */					/* from outline */
			}
			else {		/* end of MyATMFontAvailable */
/*	if (ttfflag == 0) goto tryttnow; *//* If ttfflag < 0, need try TT also ? */
				return -1;	  /* ATM cannot render this font */
/*	This usually means its an alias Helv, Modern, Roman, Tms Rmn */
			}
			return 0;				/* its presumably OK */
		}							/* end of ATM Loaded */
		else {						/* Type 1 font, but ATM not loaded */
			strcpy(fontspec, name);
			if (bBoldFlag != 0 || bItalicFlag != 0) {
				strcat(fontspec, ",");
				if (bBoldFlag != 0) strcat(fontspec, "BOLD");
				if (bItalicFlag != 0) strcat(fontspec, "ITALIC");
			}
/*			winerror(fontspec); */
/*			NOTE: fontname NOT case dependent */ /* problem because of comma */
			if (bWinNT) lenstr = 0;			/* no atm.ini in NT 97/Jan/25 */
			else lenstr = GetPrivateProfileString("Fonts", fontspec, "", 
				str, sizeof(str), "atm.ini");
/*			winerror(str); */
/*			if (lenstr > 0 && *str != '\0') return 0; */
			if (lenstr > 0) return 0;			/* found */
/*			NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */
			if (bWinNT && bNewShell) return 0;	/* until ATM exist ???? */
												/* 97/Jan/21 */
/*			if (bBoldFlag == 0 && bItalicFlag == 0) return 0; */
		}
		return 1;					/* not found, so must be synthetic */
	}	/* end of dealing with Type 1 font */

/*	if (bBoldFlag == 0 && bItalicFlag == 0) return 0; */

/*	Not ATM font, now try for TrueType font */
	if (ttfflag == 1 || ttfflag < 0) {
/*		lenstr = TryForTrue(name, bBoldFlag, bItalicFlag); */ /* 1992/Sep/17 */
		lenstr = TryForTrue(hDC, name, bBoldFlag, bItalicFlag); 
/*		if (lenstr < 0) return 1; */	/* it is synthetic */
		if (lenstr <= 0) return 1;		/* it is synthetic */

/*		strcpy(fontspec, name);
		if (bBoldFlag != 0 || bItalicFlag != 0) {
			if (bBoldFlag != 0) strcat(fontspec, " Bold");
			if (bItalicFlag != 0) strcat(fontspec, " Italic");
			}
			strcat(fontspec, " (TrueType)");
			lenstr = GetProfileString("Fonts", fontspec, "", sizeof(str)); */
/*		if (bDebug > 1) OutputDebugString(str); */

/*		if (lenstr > 0 && *str != '\0') return 0; */
		if (lenstr > 0) return 0;
	}	/* end of dealing with TrueType font */

	return 1;					/* Apparently Synthesized */
} 

/*	NOTE: fontname NOT case dependent ... */ /* problem because of comma */

/*	if ATM font, can do this better see ATMFontAvailable below */

/*	extern BOOL FAR PASCAL ATMFontAvailable (
                                LPSTR    lpFacename,
                                int      nWeight,
                                BYTE     cItalic,
                                BYTE     cUnderline,
                                BYTE     cStrikeOut,
                                BOOL FAR *lpFromOutline); */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* adjust size according to zoom */

unsigned int setsize (unsigned int fontsize, int zoom) {
	unsigned long size =  fontsize;
	int k = zoom;
	if (k > 0) {
		while(k-- > 0) {
			if (bSmallStep == 0)					/* 94/Apr/19 */
				size = (((size * 11 + 5) / 10) * 11 + 5) / 10;
			else size = (((size * 21 + 10) / 20) * 21 + 10) / 20;
		}
	}
	else if (k < 0) {
		while(k++ < 0) {
			if (bSmallStep == 0)					/* 94/Apr/19 */
				size = (((size * 10 + 5) / 11) * 10 + 5) / 11;
			else size = (((size * 20 + 10) / 21) * 20 + 10) / 21;
		}
	}
//	sprintf(debugstr, "fontsize %d setsize %d zoom %d TestSize %d",
//			fontsize, size, zoom, TestSize);
//	wincancel(debugstr);		// debugging only
	return (unsigned int) size;
} 

void showcharset (char *buff, int charset) {
	char temp[12];

	strcat (buff, " CharSet: ");
	switch (charset) {
		case ANSI_CHARSET:
			strcat(buff, "ANSI");
			break;
		case DEFAULT_CHARSET:			/* 1993/June/3 */
			strcat(buff, "Default");
			break;
		case SYMBOL_CHARSET:
			strcat(buff, "Symbol");
			break;
		case SHIFTJIS_CHARSET:
			strcat(buff, "Kanji");	/* shiftjis */
			break;
		case HANGEUL_CHARSET:		/* 1993/June/3 */
			strcat(buff, "Hanguel");
			break;
		case CHINESEBIG5_CHARSET:	/* 1993/June/3 */
			strcat(buff, "Chinese");
			break;
		case OEM_CHARSET:
			strcat(buff, "OEM"); 
			break;
		default:
/*			strcat(buff, "Unknown"); */
			sprintf(temp, "%d", charset);
			strcat(buff, temp);
/*			winerror("Unknown CharSet"); */ /*  CharSet */
			break;
	}
}

void showfamily (char *buff, int family) {
	char temp[12];

	strcat(buff, ", Family: ");
	switch((family & 0xF0)) {
		case FF_DONTCARE:  /* Don't care or don't know. */
			strcat(buff, "DontCare"); /* (DontKnow)"); */
			break;
		case FF_ROMAN:  /* Variable stroke width, serif. */
			strcat(buff, "Roman"); /*  (variable width, serif)"); */
			break;
		case FF_SWISS:  /* Variable stroke width, sans-serifed. */
			strcat(buff, "Swiss"); /*  (variable width, sans-serif)"); */
			break;
		case FF_MODERN:  /* Constant stroke width, serifed or not. */
			strcat(buff, "Modern"); /*  (constant width)"); */
			break;
		case FF_SCRIPT:  /* Cursive, etc. */
			strcat(buff, "Script"); /*  (cursive)"); */
			break;
		case FF_DECORATIVE:  /* Old English, etc. */
			strcat(buff, "Decorative"); /*  (symbol)"); */
			break;
		default: 
/*			strcat(buff, "Unknown"); */
			sprintf(temp, "%d", family);
			strcat(buff, temp);
/*			winerror("Unknown Family");	*/ /* Family */
			break;
	}
}

/* Used for setting up window title bar in ShowFont */

void ShowFontInfo (char *buff, int nChar) { /* HDC hDC, int size */
	unsigned int size;
	int intsize, fracsize;
	char *s;

/*	if (bShowWidths != 0) size = METRICSIZE * 20; */
	if (bShowWidths) size = metricsize * 20;
	else size = setsize(TestSize, wantedzoom); 
/*	size = (size + 1) / 2; */	/* convert from TWIPS to 1/10 pt */
	size = size * 5;		/* convert from TWIPS to 1/100 pt ? */
/*	intsize = (int) (size / 10); */
	intsize = (int) (size / 100);
/*	fracsize = (int) (size - ((unsigned) intsize) * 10); */
	fracsize = (int) (size - ((unsigned) intsize) * 100);
	if (fracsize == 0) 	sprintf(buff, "%s (%d pt) ", TestFont, intsize);
/*	else sprintf(buff, "%s (%d.%d pt) ", TestFont, intsize, fracsize); */
	else sprintf(buff, "%s (%d.%02d pt) ", TestFont, intsize, fracsize);
	if (bTexFontFlag != 0) strcat(buff, "(map) ");
	if (bCheckBoldFlag > 0) strcat(buff, "Bold");
	else if (bCheckBoldFlag < 0) strcat(buff, "Light");
	if (bCheckItalicFlag != 0) strcat(buff, "Italic");
	strcat(buff, " ");
	if (bSyntheticFlag) strcat(buff, "*SYNTHESIZED* ");	/* 95/Mar/12 */
	else {
		showcharset(buff, testcharset);					/* 1993/June/3 */
		showfamily(buff, testpitchandfamily);			/* 1993/June/3 */
	}
//	maybe add whether it is TrueType or not (if known) ?
	if (bCharacterSample && nChar >= 0) {
		s = buff + strlen(buff);
		sprintf(s, " (%d)", nChar);
	}
/*	if (bSyntheticFlag != 0) strcat(buff, "  SYN"); */
/*	if (bDebug > 1)
		if (bSyntheticFlag != 0) OutputDebugString(buff); */
/*	(void) TextOut(hDC, 40, -40, buff, strlen(buff)); */
/*	(void) SetMapMode(hDC, oldmapmode); */
}

/* Draw character `bounding box' and baseline */

void DrawMarks (HDC hDC, long dvi_h, long dvi_v, long width, 
		long avascent, long avdescent) {
// long avinternal, long avexternal
/*	HPEN hOldPen; */
	int xll, yll, xur, yur, ymd;
//	int yin, yex;			/* not accessed ? */
	POINT Box[7];			/* 1993/Jan/21 */
	
/*	hOldPen = SelectObject(hDC, hBorderPen); */

	xll = mapx(dvi_h);
	xur = mapx(dvi_h + width);
	yll = mapy(dvi_v + avdescent);
	yur = mapy(dvi_v - avascent);
	ymd = mapy(dvi_v);
//	yin = mapy(dvi_v - avascent + avinternal);
//	yex = mapy(dvi_v - avascent - avexternal);
		
	Box[0].x = xll; 	Box[0].y = ymd;
	Box[1].x = xll; 	Box[1].y = yur;
	Box[2].x = xur; 	Box[2].y = yur;
	Box[3].x = xur; 	Box[3].y = yll;
	Box[4].x = xll; 	Box[4].y = yll;
	Box[5].x = xll;		Box[5].y = ymd;
	Box[6].x = xur;		Box[6].y = ymd;	
	Polyline(hDC, Box, 7);			/* MUCH FASTER!	1993/Jan/21 */
}

/*	if ((UINT) hOldPen > 1)	(void) SelectObject(hDC, hOldPen); */

/* check visibility to avoid wasting time */ /* don't use if bCopyFlag != 0 */

int CharBoxVisible (HDC hDC, long dvi_h, long dvi_v, long width, 
		long avascent, long avdescent) {
	RECT TextRect;
	int xll, yll, xur, yur;
	BOOL visible;

	if (bCopyFlag != 0) return 1; 

	xll = mapx(dvi_h);	xur = mapx(dvi_h + width);
/*	if (xur == xll) xur += 20; */
	yll = mapy(dvi_v + avdescent); yur = mapy(dvi_v - avascent);
/*	if (bDebug > 1) {
		sprintf(debugstr, "%d %d %d %d\n", xll, yll, xur, yur);
		OutputDebugString(debugstr);
	} */
/*	Deal Windows 95 bug with very narrow boxes in RectVisible */
/*	--- somewhat kludgy fix ... 1995/Aug/26 */
/*	if (bWin95) */
	if (bFixZeroWidth) {		/* limit minimum width of rectangle */
/*		if ((xur - xll) < (yur - yll)) 	xur = xll + (yur - yll); */
		if ((xur - xll) < MinWidth)	xur = xll + MinWidth; /* 95/Sep/3 */
	} 
	TextRect.left = xll;	TextRect.right = xur;
	TextRect.bottom = yll;	TextRect.top = yur;
/*	return RectVisible(hDC, &TextRect); */
/*	we assume this doesn't get called when CopyFlag is on ... */
	visible = RectVisible(hDC, &TextRect);
/*	if ((bCopyFlag == 0 && RectVisible(hdc, &TextRect) != 0) ||
		(bCopyFlag != 0 && InterSectRect(&CopyClipRect, &TextRect) != 0))
		visible = 1;
	else visible = 0; */ /* alternative - if we decide allowing CopyFlag */
	return visible;
}

/* set for screen font - but also used for printer */
/* set for screen font - but kept around for MetaFileDC also */

/* font metric information (in DVI units) for actual font (not metric) */

long charw, charh, chara, chard, chari, chare;

/* compute metrics for actual font from those for metric font */

long compsize (long x, int matsize) {	/* adjust relative to METRICSIZE */
/*	return ((x * (long) matsize) / METRICSIZE); */
/*	return ((x * (long) matsize) / metricsize); */
	return ((x * (long) matsize + metricsize/2) / metricsize); 
}

/* should use real default char instead - just in case ? */

#ifdef USEUNICODE
#define notdefUID 0xFFFF
WCHAR encodeUID[];		/* table of mappings char code => UID */
#endif

/* draw rectangular array of characters */
/* flag 1 bit => draw boxes flag 2 bit => draw character */

void ScanLayOut (HDC hDC, int matsize, int flag) {
	int i, j, k, kk;
	char buff[1];
	long dvi_h, dvi_v, chrwdt;
	

	for (i = 0; i < 16; i++) {
		k = i << 4;						/* i * 16 new */
		for (j = 0; j < 16; j++) {
/*					k = i * 16 + j; */
/*					if (bTexFontFlag != 0 && bATMBugFlag != 0) */
/*					if (bTexFontFlag != 0) {	*/		/* 1993/Jan/24 */
			if (bTexFontFlag && (k <=32 || k == 127)){/* 1995/Jun/23 */
				kk = remaptable[k]; 
/*				kk = (unsigned char) remaptable[k]; */
			}
			else kk = k;
/*					buff[0] = (unsigned char) kk; */
			buff[0] = (char) kk;
			dvi_h = oneinch + charw * j + (charw * j) / 5; 
			dvi_v = oneinch + chara + charh * i + (charh * i) / 5; 
/*					chrwdt = (long) charwidths[k]; */
			chrwdt = (long) charwidths[kk];		 /* new remapped */
			chrwdt = (chrwdt * (long) matsize + 499) / 1000;	// metricsize ?
			chrwdt = unmap((int) chrwdt);	/* DVI units for metric */
/*					if (bDebug && (k % 16) == 0) {
						sprintf(str, "%d %d %ld %ld", kk, charwidths[kk], 
							compsize(chrwdt, matsize), chrwdt);
						winerror(str);
					}	*/
/*					if (bCopyFlag != 0 || CharBoxVisible(hDC, dvi_h, dvi_v, */
			if (CharBoxVisible(hDC, dvi_h, dvi_v,  
							   chrwdt, chara, chard)) {
				if (bShowBoxes && (flag & 1))		/* second pass */
					DrawMarks(hDC, dvi_h, dvi_v, 
							  chrwdt, chara, chard); // chari, chare);
				if (flag & 2) {			/* first pass */
#ifdef USEUNICODE
/* Will trigger only for text fonts in NT or for TT in 95 with UseNewEncode */
/*						if (bUseNewEncode && bFontEncoded) */
					if (bFontEncoded) {
						WCHAR buffW[1];
						buffW[0] = encodeUID[kk];
						(void) TextOutW(hDC, mapx(dvi_h), mapy(dvi_v), 
										buffW, 1);
					}
					else (void) TextOutA(hDC, mapx(dvi_h), mapy(dvi_v), 
										 buff, 1);
#else
					(void) TextOut(hDC, mapx(dvi_h), mapy(dvi_v), buff, 1);
#endif
				}
			}	
/*			else {	
				if (bDebug > 1) {
					sprintf(debugstr, "i %d j %d width %ld\n",
							i, j, chrwdt);
					OutputDebugString(debugstr);
				}
			} */
			k++;		/* new */
		} /* show a row of table */
	} /* show 16 rows of table */
}

int DrawOutlineW(HWND, HDC, UINT);			/* defined later */

/* show sample of font  ---  size is  TestSize in TWIPS */

/* NOTE: this is called inside WM_PAINT */

/* We DO the remapping in this case for TeX fonts */

// if charcode >= 0 it is the character code in the current encoding
// if charcode < 0 then show the whole array of characters

void ShowFontSample (HWND hWnd, HDC hDC, int nChar) {
	HFONT hFont, hMetricFont, hFontOld=NULL;
/*	unsigned buff[1]; */
/*	char buff[1]; */
/*	int i, j, k, kk; */
	int flag;
	int charwidth, charheight, charascent, chardescent;
	int charinternal, charexternal;
	long atsize;
	unsigned long old_dvi_num, old_dvi_den, old_dvi_mag;
	int old_wantedzoom;				/* remembered view scale */
	int matsize;
	HPEN hOldPen=NULL;
	WORD options;

/*  reset scale and map - do we always want to do this ? */
	old_dvi_num = dvi_num;
	old_dvi_den = dvi_den;
	old_dvi_mag = dvi_mag;
	old_wantedzoom = wantedzoom;
	dvi_num = 25400000;
	dvi_den = 473628672;
	dvi_mag = 1000;
	setscale(wantedzoom);						// ?

/*	bTexFontFlag = istexfont(TestFont);			*//* overridden later ??? */
	bTexFontFlag = 0;								/* 1993/Jan/24 */
	if (istexfont(TestFont) && bATMBugFlag) bTexFontFlag = 1;
	if (testwantttf && bTTRemap) bTexFontFlag = 1; /* 1993/Jan/24 */
/*	if (bATMBugFlag > 1) bTexFontFlag = 1; */		/* 1993/March/15 */
	if (bATMBugFlag == 2) bTexFontFlag = 1;			/* 1993/July/6 */

/*	if (bCopyFlag == 0)
		(void) SetMapMode(hDC, MM_TWIPS); */		/* set unit to twips */
/*	Is this assuming standard dvi_num and dvi_den ??? */
/*	atsize = (TestSize * 65536) / 20; */
	atsize = (((long) TestSize) * 16384) / 5;		/* mapped to DVI */
	if (nChar < 0) matsize = mapd(atsize);			/* mapped to TWIPS */
//	else matsize = metricsize * 20;					/* 1997/Jan/17 */
//	else matsize = mapd(atsize);					/* experiment */
//	else matsize = mapd(atsize * 16);				/* experiment */
//	else matsize = mapd(((long) metricsize * 16384) / 5);				/* experiment */
//	else matsize = ((long) metricsize * 16384) / 5;				/* experiment */
//	else matsize = metricsize * 16;				/* experiment */
	else matsize = metricsize;					/* experiment */
//	if (TestSize != 10) matsize = matsize * TestSize / 10;	// ???
//	sprintf(debugstr, "TestSize %d matsize %ld", TestSize, matsize);
//	wincancel(debugstr);	// debugging only
/*	if (matsize != (int) TestSize) {
		sprintf(str, "TestSize %d matsize %d", TestSize, matsize);
		winerror(str);
	} */ /*  shouldn't matsize = TestSize now ??? */
	hFont = createatmfont(TestFont, matsize, 0, 
				bCheckBoldFlag, bCheckItalicFlag, testwantttf);
	if (hFont == NULL) {
		sprintf(debugstr, "Can't find font %s", TestFont);
		winerror(debugstr);
	}
	else {
/*		hMetricFont = createatmfont(TestFont, METRICSIZE, 0, */
		hMetricFont = createatmfont(TestFont, metricsize, 0, 
							bCheckBoldFlag, bCheckItalicFlag, testwantttf);
		if (hMetricFont == NULL) winerror("Can't create metric font");
		else {
/*			SetupWindowText(hWnd); */ /* don't know about remap yet */
			(void) SetBkMode(hDC, TRANSPARENT); 	/* background untouched */
			(void) SetTextAlign(hDC, TA_BASELINE  | TA_LEFT);	/* ??? */
/* possibly use black pen on printer instead ??? */
			hOldPen = SelectObject(hDC, hBorderPen); 
/*			hFontOld = SelectObject(hDC, hFont); */
			if (bATMReencoded) UnencodeFont(hDC, -1, 1);
#ifdef DEBUGCOPY
			if (bDebug > 1) OutputDebugString("ATMSelectObject\n");
#endif
/*			if (!testwantttf && bATMPrefer && bATMLoaded) */
			if (! testwantttf && bATMPrefer && bATMLoaded && ! bCopyFlag) {
/*		This is where we get an error if we are working in MetaFile DC ... */
				if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
				else options = ATM_SELECT;
				(void) MyATMSelectObject (hDC, hMetricFont, options, &hFontOld);
			}
			else hFontOld = SelectObject(hDC, hMetricFont);	/* select metric */

#ifdef USEUNICODE
/*			if (bUseNewEncode) bFontEncoded = bCheckReencode; */
/*			new method only in Windows NT or for TT in 95 with UseNewEncode */
/*			in Windows 95 ATM will be loaded and hence hEncoding != NULL */
			if ((testwantttf && bUseNewEncodeTT) ||
				(! testwantttf && bUseNewEncodeT1)) {
				bFontEncoded = bCheckReencode;
			}
			else bFontEncoded = 0;
/*			else */	/* 97/Jan/25 */
/*			hEncoding will be NULL in Windows NT */
			if (bCheckReencode && ! testwantttf && hEncoding != NULL)
				ReencodeFont(hDC, -1, 1); 				/* 94/Dec/25 */
#else
			if (bCheckReencode && ! testwantttf && hEncoding != NULL)
				ReencodeFont(hDC, -1, 1); 				/* 94/Dec/25 */
#endif
/*			will need to compensate for (matsize / 1000) scale factor */
/*			if (bCopyFlag == 0) */ /* Try and deal with printer crock */
/*			if (bCopyFlag == 0 && bPrintFlag == 0) */
			if (! bCopyFlag && ! bPrintFlag && nChar < 0) { 
/*				if((flag = GetCharWidth(hDC, 0, 255, charwidths)) == 0) */
				flag = GetTextMetrics(hDC, &TextMetric);
				if (flag == 0) winerror("GetTextMetrics failed");	/* debug */
/*				flag = GetWidth(hDC, 0, 255, charwidths); */
				flag = GetWidth(hDC, 0, 255, charwidths);
				if (flag == 0) winerror("Can't get character widths"); 
/*				new way to attempt to determine whether remapped or not */
/*				this comes *after* getting widths ? */
				bTexFontFlag = 0;			/* default is to use unremapped */
				if (isremapped(charwidths, TextMetric, bTexFontFlag) != 0
					&& bATMBugFlag)	/* 1993/Jan/24 */
					bTexFontFlag = 1; 
/*				else bTeXFontFlag = 0; */
				if (testwantttf && bTTRemap) bTexFontFlag = 1; 
/*				if (bATMBugFlag > 1) bTexFontFlag = 1; *//* 1993/March/15 */
				if (bATMBugFlag == 2) bTexFontFlag = 1;	/* 1993/March/15 */
/*				if (bTexFontFlag != 0) {
					sprintf(debugstr, "Ave %d Max %d First %d Last %d\n", 
						TextMetric.tmAveCharWidth, TextMetric.tmMaxCharWidth,
							TextMetric.tmFirstChar,	TextMetric.tmLastChar);
					if (bDebug > 1) OutputDebugString(debugstr);	
				} */
				charheight = TextMetric.tmHeight + TextMetric.tmExternalLeading;
				charwidth = TextMetric.tmMaxCharWidth;  
				charascent = TextMetric.tmAscent;
				chardescent = TextMetric.tmDescent;
				charinternal = TextMetric.tmInternalLeading;
				charexternal = TextMetric.tmExternalLeading;
				if (bDebug > 1) {
					sprintf(debugstr, "Height %d ExtLead %d Max %d Asc %d Des %d",
							TextMetric.tmHeight, TextMetric.tmExternalLeading,
							charwidth, charascent, chardescent);
					OutputDebugString(debugstr);
				}
/*				since we use MaxWidth for cell spacing horizontally */
/*				let us tweak it in extreme cases */
				if (charwidth > 1000) charwidth = (charwidth + 1000) / 2;
				if (charwidth < 500) charwidth = (charwidth + 500) / 2;
/*				adjust for difference in size (matsize / METRICSIZE) */
/*				compute for actual font based on metric font */
				charwidth = (int) compsize((long) charwidth, matsize); 
				charheight = (int) compsize((long) charheight, matsize);
				charascent = (int) compsize((long) charascent, matsize);
				chardescent = (int) compsize((long) chardescent, matsize);
				charinternal = (int) compsize((long) charinternal, matsize);
				charexternal = (int) compsize((long) charexternal, matsize);
/*				turn into DVI units */	/* use saved values if printing (?) */
				if (! bPrintFlag) {	
					charw = unmap(charwidth);
					charh = unmap(charheight);
					chara = unmap(charascent);
					chard = unmap(chardescent);
					chari = unmap(charinternal);
					chare = unmap(charexternal);
				}
/*				if (! bPrintFlag) { 
					schara = chara; scharh = charh;
					scharw = charw; schard = chard; 
					schari = chari; schare = chare;
				} */
			}
			SetupWindowText(hWnd);  /* now know about remapping or not */
			if (bATMReencoded) UnencodeFont(hDC, -1, 1);
			if (! bCopyFlag) (void) SetMapMode(hDC, MM_TWIPS); /* here ? */
#ifdef DEBUGCOPY
			if (bDebug > 1) OutputDebugString("ATMSelectObject\n");
#endif
/*			if (! testwantttf && bATMPrefer && bATMLoaded) */
			if (! testwantttf && bATMPrefer && bATMLoaded && ! bCopyFlag) {
/*				This is where we get an error if we are working in MetaFile DC ... */
				if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
				else options = ATM_SELECT;
				(void) MyATMSelectObject (hDC, hFont, options, NULL);
			}
			else (void) SelectObject(hDC, hFont); /* switch to selected size */

#ifdef USEUNICODE
/*			if (bUseNewEncode) bFontEncoded = bCheckReencode; */
/*			new method only in Windows NT or for TT in 95 with UseNewEncode */
/*			in Windows 95 ATM will be loaded and hence hEncoding != NULL */
			if ((testwantttf && bUseNewEncodeTT) ||
				(! testwantttf && bUseNewEncodeT1)) bFontEncoded = bCheckReencode;
			else bFontEncoded = 0;
/*			else */ /* 97/Jan/25 */
/*			hEncoding will be NULL in Windows NT */
			if (bCheckReencode && ! testwantttf && hEncoding != NULL)
				ReencodeFont(hDC, -1, 0); 				/* 94/Dec/25 */
#else
			if (bCheckReencode && ! testwantttf && hEncoding != NULL)
				ReencodeFont(hDC, -1, 0); 				/* 94/Dec/25 */
#endif

/*			nChar = -1 for normal drawing character layout */
			if (nChar < 0) {
				if (bShowBoxes)	ScanLayOut(hDC, matsize, 1);	/* bounding boxes */
				ScanLayOut(hDC, matsize, 2);					/* characters */
/*				ScanLayOut(hDC, matsize, 3);	*/ /* do both at once */
			}
/*			nChar flag is character code for drawing character */
			else {
				int UID;							// fixed 2000 May 27
				if (bFontEncoded) UID = encodeUID[nChar];
				else UID = nChar;
				DrawOutlineW(hWnd, hDC, UID);
			}

			if ((UINT) hOldPen > 1)			/* moved here 92/April/16 */
				(void) SelectObject(hDC, hOldPen); 
		} /* end of was able to establish metric font */

		if (bATMReencoded) UnencodeFont(hDC, -1, 0);	/* moved up here ? */
		if ((UINT) hFontOld > 1)	/* avoid Windows 3.0 MetaFile problem */
			(void) SelectObject(hDC, hFontOld); /* deselect hFont */
		if ((UINT) hFont > 1)
			(void) DeleteObject(hFont);			/* free up space */
		if ((UINT) hMetricFont > 1)
			(void) DeleteObject(hMetricFont);	/* free up space */
	}	/* end of font exists case */

	dvi_num = old_dvi_num;
	dvi_den = old_dvi_den;
	dvi_mag = old_dvi_mag;
	wantedzoom = old_wantedzoom;
	setscale(wantedzoom);						/* restore zoom */
/*	if (bATMReencoded) UnencodeFont(hDC, -1, 0); */ /* too late here */
}

/* bug in ATM when character code is zero ... */

unsigned int UseSize = 10 * 20;	/* size on screen for char width display TWIPS */

/* use SetTextAlign(hDC, TA_RIGHT) for numbers ? */

/* the spacing here should be calculated using GetSystemMetrics */
/* instead of being wired in ... */

/* We DON'T do the remapping in this case */

void ShowCharWidths (HWND hWnd, HDC hDC) {
	HFONT hFont, hFontOld=NULL;
	HFONT hMetricFont, hUseFont;
/*	HFONT hFixedFont; */
	unsigned int size;
	int i, j, k, flag;
	int charwidth, charheight, charascent, chardescent;
	long dvi_h, dvi_v;
/*	long chrwdt; */
	WORD options;
	char txt[16];						/* avoid using str - 95/Jun/24 */

/*	(void) SetMapMode(hDC, MM_TWIPS); */  	/* set unit to twips */ /* NU */
	size = setsize(UseSize, wantedzoom);
/*	matsize = mapd(size); */							/* mapped to TWIPS */
/*	hUseFont = createatmfont("Courier", (int) size, 0, 0, 0); */ /* display */
	hUseFont = createatmfont("Helvetica", (int) size, 0, 0, 0, 0); 
	if (hUseFont == NULL) winerror("Can't find Helvetica"); /* debug */
/*	hMetricFont = createatmfont("Helvetica", METRICSIZE, 0, 0, 0, 0); */
	hMetricFont = createatmfont("Helvetica", metricsize, 0, 0, 0, 0); 
	if (hMetricFont == NULL) winerror("Can't create metric font");	/* debug */
/*	hFont = createatmfont(TestFont, METRICSIZE, 0, */
	hFont = createatmfont(TestFont, metricsize, 0, 
				bCheckBoldFlag, bCheckItalicFlag, testwantttf);
	if (hFont == NULL) winerror("Can't create test font");
	else {
		if (bATMReencoded) UnencodeFont(hDC, -1, 1);
		if (! testwantttf && bATMPrefer && bATMLoaded) {
			if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
			else options = ATM_SELECT;
			(void) MyATMSelectObject (hDC, hFont, options, &hFontOld);
		}
		else hFontOld = SelectObject(hDC, hFont);	/* select test font */

#ifdef USEUNICODE
/*		if (bUseNewEncode) bFontEncoded = bCheckReencode; */
/*		new method only in Windows NT or for TT in 95 with UseNewEncode */
/*		in Windows 95 ATM will be loaded and hence hEncoding != NULL */
		if ((testwantttf && bUseNewEncodeTT) ||
			(! testwantttf && bUseNewEncodeT1)) bFontEncoded = bCheckReencode;
		else bFontEncoded = 0;
/*		else */ /* 97/Jan/25 */
/*		hEncoding will be NULL in Windows NT */
		if (bCheckReencode && ! testwantttf && hEncoding != NULL)
			ReencodeFont(hDC, -1, 0);				/* 94/Dec/25 */		
#else
		if (bCheckReencode && ! testwantttf && hEncoding != NULL)
			ReencodeFont(hDC, -1, 0);				/* 94/Dec/25 */
#endif
/*		if (bCopyFlag == 0) */	/* Try and deal with printer crock */
		if (bCopyFlag == 0 && bPrintFlag == 0) {
/*			flag = GetCharWidth(hDC, 0, 255, charwidths); */
			flag = GetWidth(hDC, 0, 255, charwidths);
			(void) SetMapMode(hDC, MM_TWIPS); /* set unit to twips */ /* NU */
		}
		else flag = 1;

		if (flag == 0) winerror("Can't get character widths");
		else {
/*			(void) SelectObject(hDC, hUseFont); *//* switch to display font */
			if (bATMReencoded) UnencodeFont(hDC, -1, 1);
			if (! testwantttf && bATMPrefer && bATMLoaded) {
				if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
				else options = ATM_SELECT;
				(void) MyATMSelectObject (hDC, hMetricFont, options, NULL);
			}
			else (void) SelectObject(hDC, hMetricFont);	/* get metrics */
				
#ifdef USEUNICODE
/*			if (bUseNewEncode) bFontEncoded = bCheckReencode; */
/*			new method only in Windows NT or for TT in 95 with UseNewEncode */
/*			in Windows 95 ATM will be loaded and hence hEncoding != NULL */
			if ((testwantttf && bUseNewEncodeTT) ||
				(! testwantttf && bUseNewEncodeT1)) bFontEncoded = bCheckReencode;
			else bFontEncoded = 0;
/*			else */ /* 97/Jan/25 */
/*			hEncoding will be NULL in Windows NT */
			if (bCheckReencode && ! testwantttf && hEncoding != NULL)
				ReencodeFont(hDC, -1, 1);			/* 94/Dec/25 */
#else
			if (bCheckReencode && ! testwantttf && hEncoding != NULL)
				ReencodeFont(hDC, -1, 1);			/* 94/Dec/25 */
#endif
			if (bCopyFlag == 0) {
				flag = GetTextMetrics(hDC, &TextMetric);
				if (flag == 0) winerror("GetTextMetrics failed"); /* debug */
				charheight = TextMetric.tmHeight + TextMetric.tmExternalLeading;
/*				charwidth = TextMetric.tmMaxCharWidth; */
				charwidth = TextMetric.tmAveCharWidth; 
				charascent = TextMetric.tmAscent;
				chardescent = TextMetric.tmDescent;
/*				charinternal = TextMetric.tmInternalLeading; */
/*				charexternal = TextMetric.tmExternalLeading; */
/*				adjust for difference in size (atsize / METRICSIZE) */
				charwidth = (int) compsize((long) charwidth, size); 
				charheight = (int) compsize((long) charheight, size);
				charascent = (int) compsize((long) charascent, size);
				chardescent = (int) compsize((long) chardescent, size);	
/*				charinternal = (int) compsize((long) charinternal, size); */
/*				charexternal = (int) compsize((long) charexternal, size); */
/*				turn into DVI units */	/* use saved values when printing */
				if (! bPrintFlag) {
					charw = unmap(charwidth);
					charh = unmap(charheight);
					chara = unmap(charascent);
					chard = unmap(chardescent);
/*					chari = unmap(charinternal); */
/*					chare = unmap(charexternal); */
				}
/*				if (bPrintFlag == 0) {
			schara = chara; scharh = charh; scharw = charw; schard = chard;
				} */
			}
/*			if (! testwantttf && bATMPrefer && bATMLoaded) 
				(void) MyATMForceCreate();	*/			/* 94/Dec/22 NOT */
			(void) SelectObject(hDC, hUseFont); /* switch to display font */
/*			if (! testwantttf && bATMPrefer && bATMLoaded)
				 (void) MyATMClearAllForce(); */		/* 95/Jan/15 NOT */
			bTexFontFlag = 0;				/* always get REAL widths ? */
			SetupWindowText(hWnd);			/* calls ShowFontInfo */
			(void) SetTextAlign(hDC, TA_RIGHT);	/* right justify numbers */
			for (i = 0; i < 16; i++) {
				for (j = 0; j < 16; j++) {
					k = i * 16 + j;
/*					k = remaptable[i * 16 + j]; */
					dvi_h = oneinch + charw * (j + 1) * 6; /* 5 */
/*					dvi_v = ONEINCH + chara + charh * (i * 15 / 10); */
					dvi_v = oneinch + chara + charh * i + (charh * i) / 2;
/*					sprintf(str, "%5d ", charwidths[k]); */
					sprintf(txt, "%5d ", charwidths[k]); 
/*					if (bCopyFlag != 0 || CharBoxVisible(hDC, dvi_h, dvi_v, */
					if (CharBoxVisible(hDC, dvi_h, dvi_v,  
/*						1L, chara, chard)) */
						(chara + chard) * 5, chara, chard)) { /* 1993/June/5 */
						(void) TextOut(hDC, mapx(dvi_h), mapy(dvi_v),
/*							str, (int) strlen(str)); */
							txt, (int) strlen(txt));
					}
				} /* end of do one row */
			} /* end of do 16 rows */
			if (bATMReencoded) UnencodeFont(hDC, -1, 0); /* moved here ? */
			if ((UINT) hFontOld > 1) /* avoid Windows 3.0 MetaFile problem */
				(void) SelectObject(hDC, hFontOld); /* deselect hFont */
			if ((UINT) hFont > 1)
				(void) DeleteObject(hFont);			/* test font */
		} /* end of can get character widths */
		if ((UINT) hUseFont > 1)
			(void) DeleteObject(hUseFont);		/* free up space */
		if ((UINT) hMetricFont > 1)
			(void) DeleteObject(hMetricFont);	/* free up space */
	} /* end of hFont was created case */
/*	if (bATMReencoded) UnencodeFont(hDC, -1, 0); */ /* too late here */
}		

#define DEBUGVECTOR

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* _lopen defaults to OF_SHARE_COMPAT 0x0000 */

/* returns NULL if can't allocate memory --- or if user cancels */
/* or if can't find encoding vector */
/* This assumes that vectorname does *not* include `.vec' extension */
/* --- but it *may* include path */ /* tries various places for file */

/* Looks for encoding vector in: */
/* Fully qualified path, if given - then look *only* there */
/* In VECPATH directory */
/* In PREPATH directory */
/* In DVIWindo directory */
/* In c:\yandy\fonts\encoding */
/* In c:\dvipsone */

HFILE getencodingfile (char *vectorname) {
#ifdef LONGNAMES
	HFILE vecfile;
#else
	FILE *vecfile;
#endif
	char filename[FILENAME_MAX];	/* use for file name */
	char *s, *t;

	vecfile = BAD_FILE;

/*	first deal with case where path is specified */ /* 95/Feb/8 */
/*	potential problem if path is not fully qualified \dvisourc\foo.vec ... */
	if (strpbrk(vectorname, "\\/:") != NULL) {
		strcpy(filename, vectorname);
/*		if ((s = strrchr(filename, '.')) != NULL) *s = '\0'; */
/*		s = filename + strlen(filename) - 1; */
		strcat(filename, ".vec");
#ifdef DEBUGVECTOR
		if (bDebug > 1) OutputDebugString(filename);
#endif
		vecfile = fopen(filename, "r");
	}
	else {					/* normal case, path was not specified */
/*		if (szVecPath == NULL) winerror("NULL VecPath"); */
/*		if (strcmp(szVecPath, "") != 0) */	/* if VECPATH defined in environment */
		if (szVecPath != NULL) {	
			t = szVecPath;	/* deal with list of paths 1997/Nov/13 */
			while (t != NULL) {
				strcpy(filename, t); 
				if ((s = strchr(filename, ';')) != NULL) *s = '\0';
				s = filename + strlen(filename) - 1;
				if (*s != '\\' && *s != '/') strcat(filename, "\\");
				strcat(filename, vectorname);
				strcat(filename, ".vec");
#ifdef DEBUGVECTOR
				if (bDebug > 1) OutputDebugString(filename);
#endif
				vecfile = fopen(filename, "r");
				if (vecfile != BAD_FILE) break;
				if ((t = strchr(t, ';')) != NULL) t++;
/*				t = strchr(t, ';')+1; */ /* fixed 97/Dec/21 */
			}
		}
		if (vecfile == BAD_FILE) {
/*			if (strcmp(PrePath, "") != 0) */	/* if PREPATH defined 95/May/8 */
			if (szPrePath != NULL) {	
				t = szPrePath;	/* deal with list of paths 1997/Noc/13 */
				while (t != NULL) {
					strcpy(filename, t);
					if ((s = strchr(filename, ';')) != NULL) *s = '\0';
					s = filename + strlen(filename) - 1;
					if (*s != '\\' && *s != '/') strcat(filename, "\\"); 
					strcat(filename, vectorname);
					strcat(filename, ".vec");
#ifdef DEBUGVECTOR
					if (bDebug > 1) OutputDebugString(filename);
#endif
					vecfile = fopen(filename, "r");
					if (vecfile != BAD_FILE) break;
					if ((t = strchr(t, ';')) != NULL) t++;
/*					t = strchr(t, ';')+1; */
				}
			}
		}
		if (vecfile == BAD_FILE) { /* see if in DVIWindo directory */
			strcpy(filename, szExeWindo);
/*			following added 1996/Jan/28 --- paranoia */
/*			s = filename + strlen(filename) - 1; */
/*			if (*s != '\\' && *s != '/') strcat(filename, "\\"); */
			strcat(filename, vectorname); 
			strcat(filename, ".vec");
#ifdef DEBUGVECTOR
			if (bDebug > 1) OutputDebugString(filename);
#endif
			vecfile = fopen(filename, "r");
		}

		if (vecfile == BAD_FILE) { /* in default vector directory - 96/Aug/29 */
			strcpy(filename, "c:\\yandy\\fonts\\encoding\\");
			strcat(filename, vectorname); 
			strcat(filename, ".vec");
#ifdef DEBUGVECTOR
			if (bDebug > 1) OutputDebugString(filename);
#endif
			vecfile = fopen(filename, "r");
		}

		if (vecfile == BAD_FILE) {	/* in default DVIPSONE directory - 95/Mar/1 */
//			strcpy(filename, "c:\\dvipsone\\");
			strcpy(filename, "c:\\yandy\\dvipsone\\");	// not much use...
			strcat(filename, vectorname); 
			strcat(filename, ".vec");
#ifdef DEBUGVECTOR
			if (bDebug > 1) OutputDebugString(filename);
#endif
			vecfile = fopen(filename, "r");
		}
	}
/*	Also try `vec' sub-directory of DVIPSONE directory ??? */
	if (vecfile == BAD_FILE) {
		sprintf(str, "Can't find %s encoding vector", vectorname);
		if (wincancel (str) != 0) {		/* 1993/April/10 */
			return BAD_FILE;				/* user cancelled ... */
		}

//		(void) WritePrivateProfileString(achPr, "Vector", "", achFile);
		(void) WritePrivateProfileString(achPr, "Vector", NULL, achFile);
		return BAD_FILE;			/* too dangerous to continue 95/Mar/5 */
	}

/*	We now have an encoding vector file open */

/*	record vector file name in case of question about what gives */
	(void) WritePrivateProfileString(achDiag, "Vector", filename, achFile);
	return vecfile;
}

// LPSTR getencodingsub (LPSTR lpEncoding, char *vectorname) {
LPSTR *getencodingsub (LPSTR *lpVector, char *vectorname) {
#ifdef LONGNAMES
	HFILE vecfile;
#else
	FILE *vecfile;
#endif
	char charname[FILENAME_MAX];	/* try and be safe */
	int k;
//	LPSTR lpGlyph;

/*	bDebug = 2; */			/* DANGER !!! DEBUGGING ONLY */

	vecfile = BAD_FILE;
	if (lpVector == NULL) return NULL;		/* sanity check */
	if (vectorname == NULL) return NULL;		/* sanity check */
#ifdef DEBUGVECTOR
	if (bDebug > 1) OutputDebugString(vectorname); 
#endif
	for (k = 0; k < MAXCHRS; k++) {		/* free any strings left over */
		if (lpVector[k] != NULL) {
			free(lpVector[k]);
			lpVector[k] = NULL;
		}
	}

	if (strcmp(vectorname, "numeric") == 0) {	
//		lpGlyph=lpVector;
		for (k = 0; k < MAXCHRS; k++) {
//			wsprintf(lpGlyph, "a%d", k);
			wsprintf(charname, "a%d", k);
			lpVector[k] = zstrdup(charname);
//			lpGlyph += MAXCHARNAME;
		}
		return lpVector;
	}							/* special case 95/Jan/2 */

	vecfile = getencodingfile(vectorname);

	if (vecfile == BAD_FILE) {
		for (k = 0; k < MAXCHRS; k++) {
//			wsprintf(lpGlyph, "a%d", k);
			wsprintf(charname, "a%d", k);
			lpVector[k] = zstrdup(charname);
//			lpGlyph += MAXCHARNAME;
		}
		return lpVector;
	}

//	for (k = 0; k < MAXCHRS; k++) {	/* construct blank encoding */
//		lpVector[k] = NULL;
//	}

	while (fgets(str, sizeof(str), vecfile) != NULL) {
		if (*str == '%' || *str == ';' || *str == '\n') continue;
		if (sscanf(str, "%d %s", &k, charname) == 2) {
			if (k >= 0 && k < MAXCHRS) {	/* for safety sake ! */
//				if (strlen(charname) < MAXCHARNAME) {
//					(void) strcpy(lpVector + k * MAXCHARNAME, charname);
//					if (lpVector[k] != NULL) free(lpVector[k]);
				if (lpVector[k] != NULL) free(lpVector[k]);
				lpVector[k] = zstrdup(charname);
			}
		}
//		for (k = 0; k < MAXCHRS; k++) {	
//			if (lpVector[k] == NULL)
//				lpVector[k] = zstrdup("");
//		}
	}
	fclose(vecfile);
/*	sprintf(str, "Found %d encodings", n); */
/*	winerror(str); */							/* debug */
/*	} */
/* 	(void) GlobalUnlock (hMemEncoding); */
/*	return hMemEncoding; */
	return lpVector;
}

/* called from winsearc.c in SetupEncoding */
/* called from winfonts.c in WriteAFMFile */
/* returns allocated array of glyph names MAXCHRS * MAXCHARNAME */
/* which should to be freed again later */

// switched to array of LPSTR pointers to strings 99/Nov/8
// which should to be freed again later

HGLOBAL getencoding (char *vectorname) {
	HGLOBAL hMemEncoding;
//	LPSTR lpVector;
	LPSTR *lpVector;

//	hMemEncoding =
//		GlobalAlloc(GHND, /* GMEM_MOVEABLE | GMEM_ZEROINIT */
//					(unsigned long) MAXCHRS * MAXCHARNAME);
	hMemEncoding =
		GlobalAlloc(GHND, (unsigned long) MAXCHRS * sizeof(LPSTR)); // 99/Nov/8
	if (hMemEncoding == NULL) {
		winerror ("Unable to allocate memory");
		return NULL;					/* 1995/Jan/19 */
/*		PostQuitMessage(0); */			/* ??? */
	}
	lpVector = GlobalLock(hMemEncoding); 
	if (lpVector == NULL) {
		winerror("Unable to allocate memory");
		return NULL;
	}
	if (getencodingsub(lpVector, vectorname) == NULL) {
/*		modification to try for ansinew in this case 97/Dec/21 */
/*		this means Windows ANSI will be set up if vector not found */		
		if (getencodingsub(lpVector, "ansinew") == NULL) {
/*			file not found & user cancelled */
			(void) GlobalUnlock(hMemEncoding);
/*			(void) GlobalFree (hMemEncoding); */
			hMemEncoding = GlobalFree(hMemEncoding); 
			return NULL;
		}
	}
	(void) GlobalUnlock (hMemEncoding);
	return hMemEncoding;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int (_FAR_ _cdecl *)	(const void _FAR_ *, const void _FAR_ *)); */

/* compare used to qsort KERNPAIRs from GETKERNINGPAIR Escape */

int _cdecl compare(const void *kavoid, const void *kbvoid) {
/*	unsigned int botha, bothb; */
	unsigned int aone, atwo, bone, btwo;
/*	const struct kernpair *ka, *kb; */			/* 1992/Oct/4 */
	const KERNPAIR *ka, *kb;					/* 1992/Oct/4 */
/*	ka = (const struct kernpair *) kavoid; */	/* 1992/Oct/4 */
	ka = (const KERNPAIR *) kavoid;				/* 1992/Oct/4 */
/*	kb = (const struct kernpair *) kbvoid; */	/* 1992/Oct/4 */
	kb = (const KERNPAIR *) kbvoid;				/* 1992/Oct/4 */
/*	botha = ka->kpPair; bothb = kb->kpPair; */
/*	aone = LOBYTE(botha); atwo = HIBYTE(botha); */
	aone = LOBYTE(ka->kpPair); atwo = HIBYTE(ka->kpPair);
/*	bone = LOBYTE(bothb); btwo = HIBYTE(bothb); */
	bone = LOBYTE(kb->kpPair); btwo = HIBYTE(kb->kpPair);
	if (aone < bone) return -1;
	else if (aone > bone) return +1;
	else if (atwo < btwo) return -1;
	else if (atwo > btwo) return +1;	
	else return 0;
}

/* comparez used to qsort KERNINGPAIRs from GetKerningPairs(...) */

int _cdecl comparez(const void *kavoid, const void *kbvoid) {
	const KERNINGPAIR *ka, *kb;
	WORD aone, atwo, bone, btwo;
	ka = (const KERNINGPAIR *) kavoid;
	kb = (const KERNINGPAIR *) kbvoid;
	aone = ka->wFirst;	atwo = ka->wSecond;
	bone = kb->wFirst;	btwo = kb->wSecond;
	if (aone < bone) return -1;
	else if (aone > bone) return +1;
	else if (atwo < btwo) return -1;
	else if (atwo > btwo) return +1;	
	else return 0;
}

/* comparex used to qsort ATMKernPairs from ATMGetInstanceInfo(...) */

int _cdecl comparex(const void *kavoid, const void *kbvoid) {
	const ATMKernPair *ka, *kb;
	WORD aone, atwo, bone, btwo;
	ka = (const ATMKernPair *) kavoid;
	kb = (const ATMKernPair *) kbvoid;
	aone = ka->char_1;	atwo = ka->char_2;
	bone = kb->char_1;	btwo = kb->char_2;
	if (aone < bone) return -1;
	else if (aone > bone) return +1;
	else if (atwo < btwo) return -1;
	else if (atwo > btwo) return +1;	
	else return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

ATMFixed ATMx, ATMy;								/* current point */

ATMFixed ATMxll, ATMyll, ATMxur, ATMyur;			/* bounding box */

int firsttime=0;

void UpDateBBox (LPATMFixedPoint lpFixPnt) {
	if (firsttime) {
		ATMxll = ATMxur = lpFixPnt->x;
		ATMyll = ATMyur = lpFixPnt->y;
		firsttime = 0;
	}
	else {
		if (lpFixPnt->x > ATMxur) ATMxur = lpFixPnt->x;
		if (lpFixPnt->x < ATMxll) ATMxll = lpFixPnt->x;
		if (lpFixPnt->y > ATMyur) ATMyur = lpFixPnt->y;
		if (lpFixPnt->y < ATMyll) ATMyll = lpFixPnt->y;
	}
}

int InBBox (LPATMFixedPoint lpFixPnt) {
	if (firsttime) return 0;				/* sanity check */
	if (lpFixPnt->x < ATMxll || lpFixPnt->x > ATMxur) return 0;
	if (lpFixPnt->y < ATMyll || lpFixPnt->y > ATMyur) return 0;
	return 1;								/* point is in current BBox */
}

#ifdef IGNORED
ATMFixed ATMmul (ATMFixed a, ATMFixed b) {		/* Multiply two ATM Fixed */
	long ah, al, bh, bl;
	ATMFixed result;
	int sign=+1;
	if (a <= 0) {
		a = -a; sign = - sign;
	}
	if (b <= 0) {
		b = -b; sign = - sign;
	}
	ah = a >> 16; al = a - (ah << 16);
	bh = b >> 16; bh = b - (bh << 16);
	result = (ah * bh) << 16 + (ah * bl + al * bh) + (al * bl) >> 16;
	if (sign > 0) return result;
	else return - result;
}
#endif

ATMFixed ATMRound (double x) {
	int sign = +1;
	ATMFixed result;

	if (x < 0) {
		x = -x; sign = -1;
	}
	result = (ATMFixed) (x * 65536.0 + 0.5);
	if (sign > 0) return result;
	return - result;
}

/* The following tries to find extrema in curveto's and use those */
/* Note that the *ends* have already been entered into current BBox */
/* Watch out for the 16.16 representation of coordinates */
/* Screw it --- lets just use floating point ... */ /* 95/June/28 */

int DoCurveto (ATMFixed x0, ATMFixed y0, ATMFixed x1, ATMFixed y1, 
	ATMFixed x2, ATMFixed y2, ATMFixed x3, ATMFixed y3) { 
	double ax, bx, cx, ay, by, cy;
	double delta, t, xs, ys, xr, yr;
	ATMFixedPoint ATMExt;
/*	ATMFixedPoint ATMPnt1, ATMPnt2; */
	int flag=0;

/*	(x0, y0) and (x3, y3) have already been considered in current BBox */
/*	quick exit if the other two control points are inside BBox also */
/*	ATMPnt1.x = x1;	ATMPnt1.y = y1;
	ATMPnt2.x = x2;	ATMPnt2.y = y2;
	if (InBBox(&ATMPnt1) && InBBox(&ATMPnt2)) return 0; */

	xs = (double) x0 / 65536.0;
	cx = (double) (3 * (x1 - x0)) / 65536.0;
	bx = (double) (3 * ((x2 - x1) - (x1 - x0))) / 65536.0;
	ax = (double) ((x3 - x2) - 2.0 * (x2 - x1) + (x1 - x0)) / 65536.0;

	ys = (double) y0 / 65536.0;
	cy = (double) (3 * (y1 - y0)) / 65536.0;
	by = (double) (3 * ((y2 - y1) - (y1 - y0))) / 65536.0;
	ay = (double) ((y3 - y2) - 2 * (y2 - y1) + (y1 - y0)) / 65536.0;

/*	Try for extrema in x first */
	delta = (2 * bx * 2 * bx - 4 * 3 * ax * cx);
	if (ax != 0 && delta >= 0) {
		t = (- 2 * bx + sqrt (delta)) / (2 * 3 * ax);
		if (t > 0 && t < 1) {
			xr = (((ax * t + bx) * t + cx)) * t + xs;
			yr = (((ay * t + by) * t + cy)) * t + ys;
			ATMExt.x = ATMRound (xr);
			ATMExt.y = ATMRound (yr);
			UpDateBBox (&ATMExt);
			flag++;
		}
		t = (- 2 * bx - sqrt (delta)) / (2 * 3 * ax );
		if (t > 0 && t < 1) {
			xr = (((ax * t + bx) * t + cx)) * t + xs;
			yr = (((ay * t + by) * t + cy)) * t + ys;
			ATMExt.x = ATMRound (xr);
			ATMExt.y = ATMRound (yr);
			UpDateBBox (&ATMExt);
			flag++;
		}
	}

/*	Try for extrema in y next */
	delta = (2 * by * 2 * by - 4 * 3 * ay * cy);
	if (ay != 0.0 && delta >= 0) {
		t = (- 2 * by + sqrt (delta)) / (2 * 3 * ay);
		if (t > 0 && t < 1) {
			xr = (((ax * t + bx) * t + cx)) * t + xs;
			yr = (((ay * t + by) * t + cy)) * t + ys;
			ATMExt.x = ATMRound (xr);
			ATMExt.y = ATMRound (yr);
			UpDateBBox (&ATMExt);
			flag++;
		}
		t = (- 2 * by - sqrt (delta)) / (2 * 3 * ay);
		if (t > 0 && t < 1) {
			xr = (((ax * t + bx) * t + cx)) * t + xs;
			yr = (((ay * t + by) * t + cy)) * t + ys;
			ATMExt.x = ATMRound (xr);
			ATMExt.y = ATMRound (yr);
			UpDateBBox (&ATMExt);
			flag++;
		}
	}
	return flag;
}

/* In WIN32: change CALLBACK to ATMCALLBACK ? drop _export ? */

/*                  MyClosePath			*/

BOOL ATMCALLBACK MyClosePath (DWORD dwUserData) 
/* BOOL CALLBACK _export MyClosePath (LPSTR lpData)  */
{
	return (TRUE);
}	/* dwUserData unreferenced */

/*                  MyMoveTo			*/

BOOL ATMCALLBACK MyMoveTo (LPATMFixedPoint lpFixPnt, DWORD dwUserData) 
/* BOOL CALLBACK _export MyMoveTo (LPATMFixedPoint lpFixPnt, LPSTR lpData) */
{
	UpDateBBox (lpFixPnt);
	ATMx = lpFixPnt->x;	ATMy = lpFixPnt->y;		/* current point */
	return (TRUE);
}	/* dwUserData unreferenced */

/*                  MyLineTo			*/

BOOL ATMCALLBACK MyLineTo (LPATMFixedPoint lpFixPnt, DWORD dwUserData) 
/* BOOL CALLBACK _export MyLineTo (LPATMFixedPoint lpFixPnt, LPSTR lpData) */
{
	UpDateBBox (lpFixPnt);
	ATMx = lpFixPnt->x;	ATMy = lpFixPnt->y;		/* current point */
	return (TRUE);
}	/* dwUserData unreferenced */

/*                  MyCurveTo			*/

BOOL ATMCALLBACK MyCurveTo (LPATMFixedPoint lpFixPnt1,
	LPATMFixedPoint lpFixPnt2, LPATMFixedPoint lpFixPnt3, DWORD dwUserData) 
/* BOOL CALLBACK _export MyCurveTo (LPATMFixedPoint lpFixPnt1,
	LPATMFixedPoint lpFixPnt2, LPATMFixedPoint lpFixPnt3, LPSTR lpData) */
{
/*	if (bUseControl) {
		UpDateBBox (lpFixPnt1);
		UpDateBBox (lpFixPnt2);
	} */
	UpDateBBox (lpFixPnt3);
	if (bTraceCurveto) {				/* trace all of curveto ? */
/*	don't bother if the other two control points are inside BBox also */
		if (!InBBox(lpFixPnt1) || !InBBox(lpFixPnt2)) {
			DoCurveto(ATMx, ATMy, lpFixPnt1->x, lpFixPnt1->y,
					  lpFixPnt2->x, lpFixPnt2->y, lpFixPnt3->x, lpFixPnt3->y);
		}
	}
	ATMx = lpFixPnt3->x;	ATMy = lpFixPnt3->y;	/* current point */
	return (TRUE);
}	/* dwUserData unreferenced */

/* int GetATMCharBBox (HDC hDC, char gCurChar, int FontSize) */
int GetATMCharBBox (HDC hDC, WORD gCurChar, int FontSize) {
	ATMFixedMatrix gMatrix;
/*	char szSecretMessage [] = "sdffkjhsdkfjh"; */ /* a joke, of course ! */
	char *szSecretMessage = "sdffkjhsdkfjh";	 /* a joke, of course ! */
	int res;

	if (bATMLoaded == 0) return -1;		/* error ! --- sanity text */
	firsttime = 1;
	gMatrix.a = ATMINTTOFIXED(FontSize);
	gMatrix.b = gMatrix.c = 0;
	gMatrix.d = ATMINTTOFIXED(FontSize);
	gMatrix.tx = gMatrix.ty = 0;
/*	Now want MyATMGetOutlineA or MyATMGetOutlineW */

	res = MyATMGetOutline (hDC, (char) gCurChar, &gMatrix,	/* for now */
				MyMoveTo, MyLineTo,	MyCurveTo, MyClosePath,
							(DWORD) (LPSTR) szSecretMessage); /* ??? */

/*	res = MyATMGetOutline (hDC, (char) gCurChar, &gMatrix,
				(FARPROC) MyMoveTo, (FARPROC) MyLineTo,
						(FARPROC) MyCurveTo, (FARPROC) MyClosePath,
							(LPSTR) szSecretMessage); */

/*	in case there is nothing in the outline - e.g. `space' char */
	if (firsttime) ATMxll = ATMyll = ATMxur = ATMyur = 0;
#ifdef DEBUGATM
	if (res != ATM_NOERR) {
		if (bDebug > 1) {
			sprintf(debugstr, "ATMGetOutline returns %d for char %d\n",
					res, gCurChar);
			OutputDebugString(debugstr);
		}
	}
#endif
	return res;				/* ATM_NOERR == 0 if success */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* from winsearc.c */

/* following separated out 94/Dec/31 */
/* checks whether TextMetrics info agrees with font */
/* winansi is 0 if bCheckReencode non-zero and Encoding not ansinew */

int checkansiencode (int winansi, int charset, int family) { 
	int failed=0;
/*	char buffer[128]; */

/*	First deal with case were PFM metrics say its ANSI encoded */
/*	but we try to use a different encoding vector ... */
	if (winansi != 0 && szEncodingVector != NULL &&
		(strcmp(szEncodingVector, "ansinew") != 0 &&
		 strcmp(szEncodingVector, "ansi") != 0)) {
		sprintf(debugstr, "Font claims to be ANSI encoded\n(");
		showcharset(debugstr, charset);		/* 1993/Jun/3 */
		showfamily(debugstr, family);		/* 1993/Jun/3 */
		strcat(debugstr, " )");
		if (wincancel(debugstr) != 0) 
			failed = 1;
/*			winerror("Font claims to be ANSI encoded"); */
/*			winerror("May want to use `ansi' or `ansinew'"); */
	}
/*	Next we deal with case were PFM metrics say it is *not* ANSI encoded */
/*	but we try to use Windows ANSI encoding vector ... */
	if (winansi == 0 && szEncodingVector != NULL &&
		(strcmp(szEncodingVector, "ansinew") == 0 ||
		 strcmp(szEncodingVector, "ansi") == 0)) {
		sprintf(debugstr,"Font claims NOT to be ANSI encoded\n(");
		showcharset(debugstr, charset);		/* 1993/Jun/3 */
		showfamily(debugstr, family);		/* 1993/Jun/3 */
		strcat(debugstr, " )");
		if (wincancel(debugstr) != 0) 
			failed = 1;
/*			winerror("Font claims NOT to be ANSI encoded"); */
/*			winerror("May want to NOT use `ansi' or `ansinew'"); */
	}
	return failed;
}

/* Try and guess whether fixed pitch font */ /* separated out 94/Dec/31 */

/* BOOL IsItFixed (ABC abcwidths[], int charwidths[], int bABCFlag) */
BOOL IsItFixed (ABC abcwidths[], short int charwidths[], int bABCFlag) {
	int k;
	int charwidth;
	BOOL bFixedPitch = 1;

	if (bABCFlag != 0) {
		charwidth =	abcwidths[65].abcA
			+ (int) abcwidths[65].abcB + abcwidths[65].abcC;
		for (k = 33; k < 126; k++)
/*			if (charwidth != abcwidths[i].abcA
			+ (int) abcwidths[i].abcB + abcwidths[i].abcC) */
			if (charwidth != abcwidths[k].abcA
			+ (int) abcwidths[k].abcB + abcwidths[k].abcC) {
				bFixedPitch = 0;
				break;
			}
	}
	else {
		charwidth = charwidths[65];
		for (k = 33; k < 126; k++)
/*			if (charwidth != charwidths[i]) */
			if (charwidth != charwidths[k]) {
				bFixedPitch = 0;
				break;
		}
	}
	return bFixedPitch;
}

/* LF_FACESIZE 32		needed for Face Name */
/* ATM_PSNAMESIZE 64	needed for PostScript FontName */

/* Use in WriteAFM */	/* Won't work in W2K */
/* Could use FullName in W2K - replace space with hyphen */

int GetPSFontName (char *TestFont, BOOL bCheckBoldFlag,
				BOOL bCheckItalicFlag, char *PSName) {
	ATMFontSpec FontSpec;
/*	char szFaceStyle[32+2]; */
	int ret;

	strcpy(FontSpec.faceName, TestFont);
/*	strcpy(szFaceStyle, TestFont); */

	strcpy(PSName, TestFont);	/* in case of failure */
/*	This assumes ATM_BOLD (2) ATM_ITALIC (1) */

	FontSpec.styles = (WORD) (bCheckItalicFlag | (bCheckBoldFlag << 1));

/*	szFaceStyle[32] = (char) (bCheckItalicFlag | (bCheckBoldFlag << 1)); 
	szFaceStyle[33] = 0;	*/

	if (bATMLoaded == 0) return 0;		/* error! - sanity check */

	ret = MyATMGetPostScriptName(BD_VERSION, &FontSpec, PSName);
/*	ret = MyATMGetPostScriptName(BD_VERSION, szFaceStyle, PSName); */

	if (ret != ATM_NOERR) {
		if (bDebug > 1) {
			wsprintf(debugstr, "GetPostScriptName returns %d\n", ret);
			OutputDebugString(debugstr);
		}
/*		strcpy(PSName, TestFont); */
	} 
	return ret;
}

int bAFMTextFont;	/* set when writing AFM file */

/* Separated out so can be reused */

#ifdef LONGNAMES
// int DoATMCharMetrics(HFILE afmfile, HDC hDC, LPSTR lpEncoding, int codeflag) 
int DoATMCharMetrics (HFILE afmfile, HDC hDC, LPSTR *lpEncoding, int codeflag) 
#else
// int DoATMCharMetrics(FILE *afmfile, HDC hDC, LPSTR lpEncoding, int codeflag)
int DoATMCharMetrics (FILE *afmfile, HDC hDC, LPSTR *lpEncoding, int codeflag) 
#endif
{
	int i, width, ncode, flag;
	int xll, yll, xur, yur;
	int nglyphs=0;
	LPSTR lpCharName;
	char buffer[128];							/* for AFM file line */

	flag = GetWidth(hDC, 0, 255, charwidths);	/* redundant ? */
	if (bATMLoaded == 0) return 0;				/* error! - sanity check */
	for (i = 0; i < MAXCHRS; i++) {
//		lpCharName = lpEncoding + i * MAXCHARNAME;
//		if (*lpCharName == '\0') continue;
		if (lpEncoding[i] == NULL) continue;
		if (*lpEncoding[i] == '\0') continue;
		lpCharName = lpEncoding[i];
		width = charwidths[i];
/*		if (GetATMCharBBox(hDC, (char) i, 1000) != 0) */
		if (GetATMCharBBox(hDC, (WORD) i, 1000) != 0) {
			continue;					/* ignore failure of ATMGetOutline */
		}
		xll = (int) (ATMxll >> 16);
		yll = (int) (ATMyll >> 16);
		xur = (int) (ATMxur >> 16);
		yur = (int) (ATMyur >> 16);
		if (codeflag) ncode = i;		/* for encoded characters */
		else ncode = -1;				/* for unencoded characters */
		if (bAFMTextFont) ncode = standardcode(lpCharName);	/* use ASE 98/Aug/26 */
		strcpy(str, lpCharName);
/*		fprintf(afmfile,
			"C %d ; WX %d ; N %s ; B %d %d %d %d ;\n",
				ncode, width, str, xll, yll, xur, yur); */
		sprintf(buffer,
			"C %d ; WX %d ; N %s ; B %d %d %d %d ;\n",
				ncode, width, str, xll, yll, xur, yur);
		fputs(buffer, afmfile);
		nglyphs++;
	}
	return nglyphs;
}

/* GuessEncoding refers to the following globally defined vars */
/* testcharset, testpitchandfamily, testwantttf */
/* bCheckReencode, bCheckBoldFlag, bCheckItalicFlag */
/* and bUseNewEncode */

/* Rewritten 95/Feb/5 */	/* DAMN! */

/* Symbol		 charset = symbol family = decorative Type 1 NICE */
/* Zapf Chancery charset = ANSI	family = roman --- Type 1 text */
/* Zapf Dingbats charset = ANSI family = decorative --- Type 1 */
/* Braggadocio   charset = ANSI family = decorative --- TrueType text */
/* Algerian      charset = ANSI family = decorative --- TrueType text */
/* converted LMS charset = ANSI family = decorative !!! UGH */

char *GuessEncoding (void) {					/* 1994/Dec/31 */
	int symbolflag, decorativeflag, dontcareflag;
/*	if ((testcharset == ANSI_CHARSET || testcharset == DEFAULT_CHARSET) && */
/*		(testpitchandfamily & 0xF0) != FF_DECORATIVE) */
/*		if (testwantttf || (bCheckReencode == 0)) return "ansinew"; */
/*		else if (szReencodingVector == NULL) return "ansinew"; */ /* impossible */
/*		else return szReencodingVector; */			/* szReencodingName ? */
/*	}*/
/*	symbol or decorative font */
/*	else return "numeric"; */
/*	if ((testcharset != ANSI_CHARSET && testcharset != DEFAULT_CHARSET))
		symbolflag = 1;
	else symbolflag = 0; */
#ifdef DEBUGWRITEAFM
	if (bDebug > 1) {
		sprintf(debugstr,
			"CharSet %X PitchFamily %X TTF %d Reencode %d Encoding %s\n",
				testcharset, testpitchandfamily, testwantttf, bCheckReencode,
			   (szReencodingVector == NULL) ? "" : szReencodingVector);
		OutputDebugString(debugstr);
	}
#endif
	if (testcharset == ANSI_CHARSET) symbolflag = 0;
	else if (testcharset == DEFAULT_CHARSET) symbolflag = 0;
	else symbolflag = 1;

	if ((testpitchandfamily & 0xF0) == FF_DECORATIVE) decorativeflag = 1;
	else decorativeflag = 0;
	if ((testpitchandfamily & 0xF0) == FF_DONTCARE) dontcareflag = 1;
	else dontcareflag = 0;
/*	if the user explicitly asks for reencoding, lets not argue ! */
	if (bCheckReencode == 0 && szReencodingVector != NULL) { /* 98/Feb/28 */
/*		for all fonts look at CharSet if not ANSI */
		if (symbolflag) return "numeric";
/*		for Type 1 fonts look at the 'decorative' bit */
		if (!testwantttf && decorativeflag) return "numeric";
/*		Revised 97/Feb/5 to deal with auto converted math fonts */
		if (bDecorative) {	/* treat decorative TT font as non-text */
			if (testwantttf && decorativeflag) return "numeric";
		}
/*		Revised 97/Feb/16 to deal with auto converted fixed width non text fonts */
		if (bDontCare) {	/* treat decorative TT font as non-text */
			if (testwantttf && dontcareflag) return "numeric";
		}
	}
/*	plain vanilla text fonts (i.e. Windows ANSI encoded font) */
/*  if TrueType font, or no reencoding specified by env var ENCODING */
/*	if (testwantttf ||
			bCheckReencode == 0 ||
				szReencodingVector == NULL) return "ansinew"; */
	if (bCheckReencode == 0) return "ansinew";		/* no reencoding */
	if (szReencodingVector == NULL) return "ansinew";	/* can't do it */
#ifdef USEUNICODE
/*	can reencode TTF fonts if UseNewEncode != 0 */
	if (testwantttf && !bUseNewEncodeTT) return "ansinew";
	if (!testwantttf && !bUseNewEncodeT1 && (hEncoding == NULL))
		return "ansinew";							/* 97/Jan/25 */
#else
	if (testwantttf) return "ansinew";
#endif
/*	For Type 1 fonts, if ENCODING= is set and ATM loaded return encoding */
/*	For TrueType, if bUseNewEncode is set then reencode it ! */
	return szReencodingVector;		/* not NULL */
}

/* write back into argument - the first in list of dirs */

int SetupTeXFirst(char *szTeXFirst, char *szTeXFonts) {	/* separated 97/Apr/20 */
	char *s;
	int nlen;
	*szTeXFirst = '\0';
/*	if (strcmp(szTeXFonts, "") == 0) */
	if (szTeXFonts == NULL) {
		sprintf(debugstr, "Please set up %s  env var first", "TEXFONTS");
		winerror(debugstr);
		return -1;
	}
	if ((s = strchr(szTeXFonts, ';')) != NULL)
		nlen = s - szTeXFonts;
	else nlen = strlen(szTeXFonts);
	strncpy(szTeXFirst, szTeXFonts, nlen);
	*(szTeXFirst+nlen) = '\0';				/* Terminate it */
/*	Avoid double separator at end 96/Oct/12 */
	s = szTeXFirst + strlen(szTeXFirst) - 1;
	if (*s == '\\') {					/* Does it end in terminator ? */
		if (s > szTeXFirst && *(s-1) == '\\') *s = '\0';
	}
	return 0;
}

#ifdef AFMTOTFMDLL

HINSTANCE hAFMtoTFM=NULL;

int RunAFMtoTFMDLL (HWND hWnd, char *str) {
	int (* lpAFMtoTFM) (HWND, char *)=NULL;	/* compare afmtotfm.h */
	int ret, fault=0;

	if (bDebug > 1)	OutputDebugString("RunAFMtoTFMDLL");

	if (hAFMtoTFM == NULL) 
		hAFMtoTFM = LoadLibrary("AFMtoTFM");	/* connect to AFMtoTFM.DLL */
	if (hAFMtoTFM == NULL){
		WriteError("Can't link to AFMtoTFM.DLL");
		return -1;			// error return
	}
	lpAFMtoTFM = (int (*) (HWND, char *))
				 GetProcAddress(hAFMtoTFM, "AFMtoTFM");
	if (lpAFMtoTFM == NULL)	{
		winerror("Can't find AFMtoTFM entry point");
		fault++;
	}
	else {
		if (hConsoleWnd == NULL)			// create console window if needed 
			CreateConsole(hInst, hWnd);		// for AFMtoTFM.DLL
		if (hConsoleWnd == NULL) {
			ShowLastError();				// no console window created
			fault++;
		}
		else {
			(void) EnableWindow(hWnd, FALSE); 	/* disable main Window ??? */
/*			call AFMtoTFM with command line string */
			ret = (* lpAFMtoTFM) (hConsoleWnd, str);
			(void) EnableWindow(hWnd, TRUE); 	/* reenable main Window ??? */
		}
/*		can free library now since dviwindo takes care of modeless dialog */
	}
	FreeLibrary(hAFMtoTFM); 
	hAFMtoTFM = NULL;
	return fault;
}

void ShowLastError (void) {
	int err, nlen;
	err = GetLastError();
	nlen = FormatMessage(
				  FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL,					  
				  err, 
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // or 0
				  debugstr,
				  sizeof(debugstr), 
				  NULL);
	if (nlen > 0) {
		winerror(debugstr);
//		sprintf(debugstr, "err %d nlen %d", err, nlen);
//		winerror(debugstr);		
	}
	else {
		sprintf(debugstr, "FormatMessage failed on error %d", err, err);
		winerror(debugstr);
	}
	if (bDebug > 1) OutputDebugString(debugstr);
	
/*	sprintf(debugstr, "hInst %08X hWnd %08X", hInst, hWnd);
	winerror(debugstr); */
}
#endif

/* fname is Windows Face Name, presently not used here */
/* waitflag set if batchflag is set (i.e. when using WriteTFMs... */

int CallAFMtoTFM (HWND hWnd, char *fname, char *szAFMFileName,
			char *szEncodingVector, int waitflag) {
	int n, nCmdShow;
	HINSTANCE err;					/* 95/Mar/31 */
	char szTeXFirst[MAXFILENAME];		/* first path on TEXFONTS */
/*	char *s; */
/*	int nlen; */
	HWND hFocus;					/* 1995/Nov/1 */

/*	SW_HIDE ? SW_MINIMIZE ? SW_SHOWMINIMIZED ? SW_SHOW ? */
/*	recommend us of PIF file for AFMtoTFM ? */	
/*	nCmdShow = SW_SHOWMINIMIZED; */	/* 1997/Nov/30 change */
	if (nCmdShowForce >= 0) nCmdShow = nCmdShowForce;	/* 94/Mar/7 */
	else nCmdShow = SW_SHOWMAXIMIZED;
/*	This is now dead - since we removed the check box ... */
	if (bRunMinimized != 0) nCmdShow = SW_SHOWMINIMIZED;

	if (szEncodingVector == NULL) {
		sprintf(debugstr, "Please set up %s  env var first", "ENCODING");
		winerror(debugstr);
		return -1;
	}

	if (SetupTeXFirst(szTeXFirst, szTeXFonts) < 0) return -1;
/*	if (szTeXFonts == NULL) {
		sprintf(debugstr, "Please set up %s  env var first", "TEXFONTS");
		winerror(debugstr);
		return -1;
	}
	if ((s = strchr(szTeXFonts, ';')) != NULL)
		nlen = s - szTeXFonts;
	else nlen = strlen(szTeXFonts);
	strncpy(szTeXFirst, szTeXFonts, nlen);
	*(szTeXFirst+nlen) = '\0';
	s = szTeXFirst + strlen(szTeXFirst) - 1;
	if (*s == '\\') {
		if (s > szTeXFirst && *(s-1) == '\\') *s = '\0';
	} */

	for (n = 0; n < 4; n++) {
		if (TaskInfo.hWnd == NULL) break;
		if (IsTaskActive() == 0) break;
		if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
/*		(void) MessageBox(GetFocus(), */
		(void) MessageBox(hFocus,
			"Please wait for AFMtoTFM to finish",
				modname, MB_ICONASTERISK | MB_OK);
	}

/*	if (strcmp (szAFMtoTFM, "") != 0) */
/*	if (strcmp (szAFMtoTFMcom, "") != 0) */
	if (szAFMtoTFMcom != NULL)
		strcpy(str, szAFMtoTFMcom);			/* call user supplied prog */
	else strcpy(str, "AFMTOTFM");			/* revised 1995/June/26 */
	strcat (str, " ");
	strcat (str, "-v ");
	if (strcmp(szEncodingVector, "numeric") == 0) { /* symbol/decorative */
/*		wsprintf(str, "afmtotfm -v -O=%s %s",
			(LPSTR) szTeXFirst, (LPSTR) szAFMFileName); */
	}
	else {		/* plain text font using Windows ANSI as default */
/*		strcat(str, "-adjx -c="); */
		strcat(str, "-adx ");		/* drop the `j' 98/Aug/17 */
		strcat(str, "-c=");
		strcat(str, szEncodingVector);
		strcat(str, " ");
/*		wsprintf(str, "afmtotfm -vadjx -c=%s -O=%s %s",
		(LPSTR) szEncodingVector, (LPSTR) szTeXFirst, (LPSTR) szAFMFileName); */
	}
	strcat(str, "-O=");
	strcat(str, szTeXFirst);
	strcat(str, " ");
/*	if (strcmp(szAFMtoTFM, "") != 0) */				/* 1995/June/26 */
	if (szAFMtoTFM != NULL) {	
		strcat(str, szAFMtoTFM);
		strcat(str, " ");
		if (bDebug > 1) OutputDebugString(str);
	}
	strcat (str, szAFMFileName);					/* 1995/June/26 */

	WritePrivateProfileString(achDiag, "LastAFMtoTFM", str, achFile);

/*	call AFMtoTFM.DLL --- drop through to the old way if it fails */

/*	check afmtotfm.h for definition of AFMtoTFM entry point ! */

#ifdef AFMTOTFMDLL
//	if (bUseAFMtoTFMDLL) {
	if (bUseDLLs) {
		if (RunAFMtoTFMDLL(hWnd, str) == 0) return 0;		// success
								// else drop through to the old way...
	}
/*	in any of the above failure cases we drop through to the old way */
#endif

	err = TryWinExec(str, nCmdShow, "AFMtoTFM");	/* in winprint.c */
/*	optionally stop here and wait for it to finish ? */
	if (err >= HINSTANCE_ERROR) {
/*		if (waitflag) */	/* experiment 97/Nov/30 */
			(void) GetTaskWindow (err);
/*			optionally delete the AFM file again ? */
/*		} */
		return 0;
	}
/*	optionally delete the AFM file again ? */
	return -1;
}	/* fname, waitflag unreferenced */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* For T1 fonts, ATM.INI lists:  Face Name, Style=PFM file, PFB file */
/* NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */

/* Can use ATMGetFontPaths for this ... */
/* Could also use ATMREG.ATM, but we only need this for enumerated fonts */
/* Now this also works for inactive fonts, by the way */
/* In W2K, should we check registry ? */
/* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Type 1 Installer\Type 1 Fonts */
/* <FullName> (hyphen -> space) T <PFM file> <PFB file> */

/* bCheckBoldFlag and bCheckItalicFlag global pass information */

/* called only from FileFromFace */

int TryForATM (char *fname, char *TestFont) { /* 1995/April/15 */
	int flag=0;
	char *s;
	int ret;
	ATMFontSpec FontSpec;			/* FaceName and Style */
	ATMFontPathInfo FontPathInfo;	/* pfb, pfm, mmm, pss file names 4*260 */

	if (bDebug > 1) OutputDebugString("TryForATM");
/*	The new way, using ATMGetFontPaths - try this first - if ATM loaded */
	if (bATMLoaded) {		/* 97/Jan/15 get here in Windows 95/98 only */
		strcpy(FontSpec.faceName, TestFont);
		FontSpec.styles = 0;
		if (bCheckItalicFlag) FontSpec.styles |= ATM_ITALIC;
		if (bCheckBoldFlag) FontSpec.styles |= ATM_BOLD;
/*		put in empty strings in case it fails 96/Oct/12 ??? */
/*		FontPathInfo.pfbPath[0] = '\0'; etc. */
		ret = MyATMGetFontPaths(BD_VERSION, &FontSpec, &FontPathInfo);
		if (ret == ATM_NOERR) {
#ifdef DEBUGFONTSELECT
		if (bDebug > 1) {
			OutputDebugString(FontPathInfo.pfbPath);
			OutputDebugString(FontPathInfo.pfmPath);
			OutputDebugString(FontPathInfo.mmmPath);
			OutputDebugString(FontPathInfo.pssPath);
		}
#endif
/*		strcpy(fname, FontPathInfo.pfbPath); */
		strcpy(fname, FontPathInfo.pfmPath);
		if (*fname != '\0') return 1;	/* OK use PFM file name */
#ifdef IGNORED									/* removed 97/Dec/2 */
		strcpy(fname, FontPathInfo.mmmPath);
		if (*fname != '\0')	return 1;	/* OK use PFB file name */
#endif
		}
	}
/*	should the above fail for some reason, we just drop through */

#ifdef DEBUGAFMNAME
	if (bDebug > 1) {
		sprintf(debugstr, "After ATMGetFontPaths: %s\n", TestFont);
		OutputDebugString(debugstr);
	}
#endif

/*	Next try ATMREG.ATM 97/Jan/21 */
	if (bTryATMRegFlag) {
		ret = ReadATMReg(1);
/*		if (ret == 0) */
		if (ret == 0 && *str != '\0') {
			strcpy(fname, str);
			return 1;				/* success ! */
		}
		if (ret < 0) bTryATMRegFlag = 0;	/* don't try this again */
	}

#ifdef DEBUGAFMNAME
	if (bDebug > 1) {
		sprintf(debugstr, "After ReadATMReg: %s\n", TestFont);
		OutputDebugString(debugstr);
	}
#endif

/*	Next try and see whether this is an ATM font */
	strcpy(fname, TestFont);
/*	Following added to cope with common Alias --- 1993/April/10 */
	if (strcmp(fname, "Times Roman") == 0) strcpy(fname, "Times");
	if (bCheckBoldFlag != 0 || bCheckItalicFlag != 0) strcat(fname, ",");
	if (bCheckBoldFlag != 0) strcat(fname, "BOLD");
	if (bCheckItalicFlag != 0) strcat(fname, "ITALIC");
#ifdef DEBUGAFMNAME
	if (bDebug > 1) OutputDebugString(fname);
#endif
	if (bWinNT)	*str = '\0';	/* there is no atm.ini in NT 97/Jan/25 */

	else GetPrivateProfileString("fonts", fname, "",
								 str, sizeof(str), "atm.ini");
/*	NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */
/*	The following takes care of cases where base font is missing 95/Jan/2 */
	if (*str == '\0' && bCheckItalicFlag == 0 && bCheckBoldFlag == 0) {
		strcat(fname, ",");
		strcat(fname, "ITALIC");	
		if (bWinNT)	*str = '\0'; /* there is no atm.ini in NT 97/Jan/25 */
		else GetPrivateProfileString("fonts", fname, "",
									 str, sizeof(str), "atm.ini");
/*		NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */
		if (*str == '\0') {				/* Desperation 95/Jan/2 */
			s = strchr(fname, ',');
/*			*s = '\0'; */
/*			strcat(fname, ","); */
			if (s != NULL) strcpy(s+1, "BOLD");	
			(void) GetPrivateProfileString("fonts", fname, "",
										   str, sizeof(str), "atm.ini");
/*	NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */
		}
	}

	if (*str != '\0') {				/* something found ? */
/*		if (!bATMLoaded) *atmfontflag = 1; */	/* only if ATM can't be asked*/
		flag = 1;
		strcpy(fname, str);
		if ((s = strchr(fname, ',')) != NULL) *s = '\0';
	}
#ifdef DEBUGAFMNAME
	if (bDebug > 1) {
		sprintf(debugstr, "TryForATM: %s %s FLAG: %d\n", TestFont, fname, flag);
		OutputDebugString(debugstr);
//		wincancel(debugstr);
	}
#endif
	if (bWinNT5) {
//	Try in registry? \Type 1 Installer\Type 1 Fonts
	}
	return flag;
}

/* For TT, the `Face Name' is what is listed in font menus */
/* For TT, the `Full Name' is what appears in WIN.INI */
/* Somehow need to get from `Face Name' to `Full Name' ??? Enumerate Fonts */
/* Called only from FileFromFace */

/* int TryForTTF (char *fname, char *TestFont) */ /* 1995/Apr/15 */
int TryForTTF (HDC hDC, char *fname, char *TestFont) { /* 1995/July/8 */
	int flag=0;
	int lenstr;

	if (bDebug > 1) OutputDebugString("TryForATM");
	*str = '\0';
/*	See whether maybe a True Type font */
/*	lenstr = TryForTrue(TestFont, bCheckBoldFlag, bCheckItalicFlag); */
	lenstr = TryForTrue(hDC, TestFont, bCheckBoldFlag, bCheckItalicFlag);

	if (*str != '\0') {					/* yes, it is TrueType */
/*		*truetypeflag = 1; */
		flag = 1;
//		strcpy(fname, str); 
		strcpy(fname, removepath(str));		// 2000 June 2
/*		This is where before we set up hMetric and did ABCMetrics */
	}
/*	if (bDebug > 1) {
		sprintf(debugstr, "TryForTTF: %s %s FLAG: %d\n", TestFont, fname, flag);
		OutputDebugString(debugstr);
	} */
	return flag;
}

/* Writes back into first argument ... */	/* 1995/July/19 */

void BadStyleMessage (char *szBuffer, char *szFont, char *szInfo) {
	char *s;
	sprintf(szBuffer, "Can't find file name for face `%s'\n", szFont);
	s = szBuffer + strlen(szBuffer);
/*	show style */	/* 1995/July/19 */
	FormatStyle (s, bBoldFlag, bItalicFlag);
/*	sprintf(s, "%s%s%s",
			(bBoldFlag == 0 && bItalicFlag == 0) ? "REGULAR" : "",
			bBoldFlag ? "BOLD" : "",
			bItalicFlag ? "ITALIC" : ""); */
/*	if (bBoldFlag == 0 && bItalicFlag == 0) strcat(buffer, "REGULAR");
	if (bBoldFlag) strcat (buffer, "BOLD");
	if (bItalicFlag) strcat (buffer, "ITALIC"); */
	strcat (szBuffer, " style does not exist ");
	if (bDebug > 1) {
		if (testwantttf) strcat(szBuffer, "TT ");
		else strcat(szBuffer, "T1 ");
		strcat (szBuffer, szInfo);
	}
}

/* Complains if uncertain about TFM file name - may overwrite another */

int AvoidTrouble(char *TestFont, int bBoldFlag, int bItalicFlag) {
/*	char buffer[BUFLEN]; */			/* does it need to be this big ? */
	char *s;
	
	FormatNameStyle (debugstr, TestFont, bBoldFlag, bItalicFlag);
	strcat(debugstr, "\n");
	s = debugstr + strlen(debugstr);
	LoadString(hInst, BAD_STYLE, s, sizeof(debugstr) - strlen(debugstr));
	if (wincancel(debugstr))	return 1; /* user decided to cancel */
	else return 0;
}

/*	Separated out 95/Feb/6 */ /* may write back into truetypeflag */
/*	Check incoming value of truetypeflag to see atm.ini or win.ini first */
/*	Rewritten 95/April/15 to select order of looking at atm.ini and win.ini */
/*	Returns -1 if fails for some reason 1995/July/19 */

/*	For Type 1 fonts, can use ATMGetFontPaths for this ... */

/* int FileFromFace (char *fname, char *TestFont, */
int FileFromFace (HDC hDC, char *fname, char *TestFont,
			int *atmfontflag, int *truetypeflag) {
	char afmfilename[MAXFILENAME];	/* use for both vector in and afm out .. */
	char *s;
	int flag = 0;			/* this gets set once we found a name */
/*	int lenstr; */

#ifdef DEBUGAFMNAME
	if (bDebug > 1) {
		sprintf(debugstr, "FileFromFace %s ATM %d TTF %d\n",
				TestFont, *atmfontflag, *truetypeflag);
		OutputDebugString(debugstr);
//		wincancel(debugstr);	// debugging only
	}
#endif
	*fname = '\0';					/* 96/Oct/12 */
	if (*truetypeflag == 0) {		/* Try ATM.INI before WIN.INI */
		if (TryForATM(fname, TestFont) != 0) {
			if (! bATMLoaded) *atmfontflag = 1; 	/* only if ATM can't be asked */
			*truetypeflag = 0;
			flag = 1;
		}
/*		else if (TryForTTF(fname, TestFont) != 0) */
		else if (TryForTTF(hDC, fname, TestFont) != 0) {
			*truetypeflag = 1;
			*atmfontflag = 0;
			flag = 1;
		}
	}
	else {							/* Try WIN.INI before ATM.INI */
/*		if (TryForTTF(fname, TestFont) != 0) */
		if (TryForTTF(hDC, fname, TestFont) != 0) {
			*truetypeflag = 1;
			*atmfontflag = 0;
			flag = 1;
		}
		else if (TryForATM(fname, TestFont) != 0) {
			if (! bATMLoaded) *atmfontflag = 1; 	/* only if ATM can't be asked */
			*truetypeflag = 0;
			flag = 1;
		}
	}

#ifdef DEBUGAFMNAME
	if (bDebug > 1) {
		sprintf(debugstr, "FileFromFace %s ATM %d TTF %d\n",
				TestFont, *atmfontflag, *truetypeflag);
		OutputDebugString(debugstr);
	}
#endif

	if (flag == 0) {	/* font is neither ATM nor TrueType ... */
/*	now look in dviwindo.fnt --- for private mapping */
/*	NOTE: this does NOT look in <filename>.fnt of current DVI file */
/*	NOTE: this does NOT look in dviwindo.fnt of current DVI file */	
		strcpy(fname, TestFont);		/* same as for ATM.INI ... */
		if (bCheckBoldFlag != 0 || bCheckItalicFlag != 0)
			strcat(fname, ",");
		if (bCheckBoldFlag != 0) strcat(fname, "BOLD");
		if (bCheckItalicFlag != 0) strcat(fname, "ITALIC");	
		strcpy(afmfilename, szExeWindo);	/* assuming has separator */
		strcat(afmfilename, "dviwindo.fnt");
		(void) GetPrivateProfileString("Fonts", fname, 
			"", str, sizeof(str), afmfilename);
		if (*str == '\0') { /* backward compatability */
			(void) GetPrivateProfileString("FontMapping", fname, 
				"", str, sizeof(str), afmfilename);
		}
		if (*str == '\0') {
			strcpy(str, TestFont);				/* Desperation 95/July/19 */
			strcpy(fname, TestFont);			/* Desperation */
			if ((s = strchr(fname, '(')) != NULL) {
				*(s-1) = '\0'; /* (TT) */
			}
/* UGH!  Not found there either - try and construct file name from font name */
/*			BadStyleMessage(debugstr, TestFont, "A");
			if (wincancel(debugstr) != 0) return -1;	*//* FAIL 95/July/19 */
			if (AvoidTrouble(TestFont, bCheckBoldFlag, bCheckItalicFlag)) {
				return -1;
			}
/* heuristic - squezze out spaces */ /* problem with overlap of string ? */
/* problem with hyphens that became spaces and should stay spaces ? */
			while ((s = strchr(fname, ' ')) != NULL) strcpy(s, s+1);
/* limit filename to 8 characters - to avoid open error */
			if (strlen(fname) > 8) fname[8] = '\0';
		}	/* end of not in dviwindo.fnt file case */
	}	/* end of not ATM and not TrueType case */

	if (*fname == '\0') return -1;		/* failed */
/*	make up file name for AFM file */ /* remove path if any */
	if ((s = strrchr(fname, '\\')) != NULL) strcpy(fname, s+1);
	if ((s = strrchr(fname, '/')) != NULL) strcpy(fname, s+1);
	if ((s = strrchr(fname, ':')) != NULL) strcpy(fname, s+1);
/*	remove extension of any */
	if ((s = strrchr(fname, '.')) != NULL) *s = '\0';
	return 0;									/* success */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// int GetGlyphList(HDC, LPSTR, int);

int GetGlyphList(HDC, LPSTR *, int);

/* segregated out 95/Apr/4 --- writes back into afmfilename */
/* expanded for case BasePath is defined 96/Aug/30 */

#ifdef LONGNAMES
HFILE makeafmfilename(char *afmfilename, char *fname) 
#else
FILE *makeafmfilename(char *afmfilename, char *fname) 
#endif
{
#ifdef LONGNAMES
	HFILE afmfile;
#else
	FILE *afmfile;
#endif
	char *s;
	
	afmfile = BAD_FILE;			/* 95/Dec/1 */

/*	If BasePath defined then construct c:\\yandy\\fonts\\afm 96/Aug/29 */ 
/*	if (strcmp(szBasePath, "") != 0) */
	if (szBasePath != NULL) {
		strcpy(afmfilename, szBasePath);
		s = afmfilename + strlen(afmfilename) - 1;
		if (*s == '\\' || *s == '/') *s = '0';
		strcat (afmfilename, "\\fonts");
		strcat (afmfilename, "\\afm");
		strcat (afmfilename, "\\");
		strcat (afmfilename, fname);	/* assumed extensionless */
		strcat (afmfilename, ".AFM");
		afmfile = fopen(afmfilename, "w");

		if (afmfile == BAD_FILE) {	/* try and open */
			if (bDebug > 1) OutputDebugString(afmfilename);
/*			try and create directory then ... assumes //fonts exists ... */
			s = strrchr(afmfilename, '\\');
			if (s != NULL) {
				*s = '\0';
/* is the following OK in WIN32 use CreateDirectory instead ? */
/* can this handle long file names in WIN16 ? */
				if (_mkdir(afmfilename)) {
					if (bDebug > 1) {
						sprintf(debugstr, "_mkdir failed on %s", afmfilename);
						OutputDebugString(debugstr);
					}
				}
				*s = '\\';
			}
			afmfile = fopen(afmfilename, "w");
		}
	}

/*	If BasePath *not* defined, or if the above failed ... */
	if (afmfile == BAD_FILE) {
/*		get path of executable */	/* assumes it has path separator at end */
		strcpy (afmfilename, szExeWindo); 
/*		go use AFM sub-directory */	/* if it exists 95/Feb/11 */
		strcat (afmfilename, "afm\\");
/*		concatenate desired font file name */
		strcat (afmfilename, fname);
/*		add new extension */
		strcat (afmfilename, ".AFM");	/* 1992/Sep/17 */

		afmfile = fopen(afmfilename, "w");
		if (afmfile == BAD_FILE) {	/* try and open */
			if (bDebug > 1) OutputDebugString(afmfilename);
/*			try and create directory then ... */
			s = strrchr(afmfilename, '\\');
			if (s != NULL) {
				*s = '\0';
/* is the following OK in WIN32 use CreateDirectory instead ? */
/* can this handle long file names in WIN16 ? */
				if (_mkdir(afmfilename)) {
					if (bDebug > 1) {
						sprintf(debugstr, "_mkdir failed on %s", afmfilename);
						OutputDebugString(debugstr);
					}
				}
				*s = '\\';
			}		
			afmfile = fopen(afmfilename, "w");

			if (afmfile == BAD_FILE) {	/* try and open */
				if (bDebug > 1) OutputDebugString(afmfilename);
				s = strstr(afmfilename, "afm");		/* c:\\dviwindo\\afm...*/
				if (s != NULL) strcpy(s, s+4);		/* strip out again */
				afmfile = fopen(afmfilename, "w");  /* directly dviwindo dir */
				if (afmfile == BAD_FILE) {
					sprintf(debugstr, "Unable to create %s", afmfilename);
					winerror (debugstr);
/*					failed = 1; */
/*					goto cleanup; */						/* 94/Dec/31 */
/*					return NULL; */
					return BAD_FILE;						/* 95/Dec/10 */
				}
			}
		}
	}
	return afmfile;
}

void ShowhDCFontInfo(HDC, int);			/* in winsearc.c */

/*  Return allocated data structure and use in WriteATMFile ? */
/*	Currently not use for a whole lot --- use more later ? */

/* HGLOBAL GetATMFontInfo(HDC hDC, WORD flags, int verboseflag) */
HGLOBAL GetATMFontInfo(HDC hDC, WORD flags) {
	WORD BufSize=0;
	char FaceName[LF_FACESIZE];
	HGLOBAL hInfo;
	LPATMInstanceInfo lpInfo;
/*	WORD flags = 0; */
	int ret;

#ifdef DEBUGHEAP
	CheckHeap("GetATMFontInfo", 0);
#endif

	if (!MyATMFontSelected(hDC)) return NULL;		/* check if ATM font */
/*	debugging change only 97/Feb/5 reinstate the above */
/*	if (bATMLoaded && !MyATMFontSelected(hDC)) return NULL; */

#ifdef DEBUGHEAP
	CheckHeap("GetATMFontInfo", 0);
#endif

	if (bDebug > 1) ShowhDCFontInfo (hDC, 1);		/* in winsearc.c */
/*	strcpy (FaceName, TestFont); */
/*	flags = ATM_GETWIDTHS | ATM_GETKERNPAIRS; */
	ret = -1;
	if (bATMLoaded)	ret = MyATMGetFontInfo(BD_VERSION, hDC, flags, FaceName, NULL, &BufSize);
	if (ret != ATM_NOERR) {
		if (bDebug > 1) {
			sprintf(debugstr, "GetATMFontInfo returns %d %d\n", ret, BufSize);
			OutputDebugString(debugstr);
		}
		return NULL;			/* failed */
	}
#ifdef DEBUGHEAP
	CheckHeap("GetATMFontInfo", 0);
#endif
	hInfo = GlobalAlloc(GMEM_MOVEABLE, BufSize);
	lpInfo = GlobalLock(hInfo);
	if (lpInfo == NULL) {
		winerror("Unable to allocate memory");
		return NULL;			/* failed to allocate */
	}
	ret = -1;
	if (bATMLoaded)
		ret = MyATMGetFontInfo(BD_VERSION, hDC, flags, FaceName, lpInfo, &BufSize);
	if (ret != ATM_NOERR) {
		if (bDebug > 1) {
			sprintf(debugstr, "GetATMFontInfo returns %d %d\n", ret, BufSize);
			OutputDebugString(debugstr);
		}
		GlobalUnlock(hInfo);
		GlobalFree(hInfo);
		return NULL;
	}
#ifdef DEBUGATMINFO
	if (bDebug > 1) {					/* now dump the information */
/*		For ATMInstanceInfo.flags field see atmpriv.h */
/*		1 fixed-width, 2 serif, 4 PI, 8 Script, 20 ASE, 40 Italic, 4000 bold */
		sprintf(debugstr, "FaceName %s Flags %0X BufSize %d\n",
				FaceName, lpInfo->flags, BufSize);
		OutputDebugString(debugstr);
		sprintf(debugstr, "%d chars %d kerns\n",
				lpInfo->num_chars, lpInfo->num_kern_pairs);
		OutputDebugString(debugstr);
		sprintf(debugstr, "FontBBox %d %d %d %d\n",
lpInfo->font_bb_left, lpInfo->font_bb_bottom, lpInfo->font_bb_right, lpInfo->font_bb_top);
		OutputDebugString(debugstr);		
/* ATMInstanceInfo.encoding is 0 for text fonts and -1 (65535) for PI fonts */
		sprintf(debugstr, "Missing %d Default %d Break %d Encoding %d\n",
lpInfo->missing_width, lpInfo->default_char,  lpInfo->break_char, lpInfo->encoding);
		OutputDebugString(debugstr);
		sprintf(debugstr,
			"stem_v %d stem_h %d cap_height %d x_height %d figure_height %d\n",
lpInfo->stem_v, lpInfo->stem_h, lpInfo->cap_height, lpInfo->x_height, lpInfo->figure_height);
		OutputDebugString(debugstr);
		sprintf(debugstr, "Ascender %d Descender %d Leading %d\n",
				lpInfo->ascent, lpInfo->descent, lpInfo->leading);
		OutputDebugString(debugstr);		
	sprintf(debugstr, "max_width %d avg_width %d italic_angle %d superior %d\n",
lpInfo->max_width, lpInfo->avg_width, lpInfo->italic_angle, lpInfo->superior_baseline);
		OutputDebugString(debugstr);
		sprintf(debugstr,
				"Under_pos %d Under_thick %d Strike_off %d Strike_thick %d\n",
		lpInfo->underline_position, lpInfo->underline_thickness,
		lpInfo->StrikeOutOffset, lpInfo->StrikeOutThickness);
		OutputDebugString(debugstr);		
		sprintf(debugstr, "widths_off %d kernpair_off %d\n",
				lpInfo->widthsOffset, lpInfo->kern_pairsOffset);
		OutputDebugString(debugstr);	
#ifdef IGNORED
		if (bDebug > 1) {
			LPWORD lpFontWidths;
/*	dump char widths */
/*	missing characters have width 0 (char code 0 - 31) or Missing_width */
			OutputDebugString("Char Widths\n");
			lpFontWidths = (LPWORD) ((LPSTR) lpInfo + lpInfo->widthsOffset);
			for (k = 0; k < lpInfo->num_chars; k++) {
				if (lpFontWidths[k] == 0) continue;		/* heuristic only */
				sprintf(debugstr, "%d %d\n", k, lpFontWidths[k]);
				OutputDebugString(debugstr);
			}
		}
#endif
#ifdef IGNORED
		OutputDebugString("Kern Pairs\n");
		lpKernPairs = (LPATMKernPair) ((LPSTR) lpInfo + lpInfo->kern_pairsOffset);
/*	dump kern pairs */
		for (k = 0; k < lpInfo->num_kern_pairs; k++) {
			sprintf(debugstr, "%d %d %d\n", lpKernPairs[k].char_1,
					lpKernPairs[k].char_2, lpKernPairs[k].distance);
			OutputDebugString(debugstr);
		}
#endif
	}
#endif	/* ifdef DEBUGATM */
	GlobalUnlock(hInfo);
/*	GlobalFree(hInfo); */
	return hInfo;
}

/**************************************************************************/

/* returns -1 if glyphname is *not* in encoding, otherwise char code */

// int isinencoding(LPSTR lpEncoding, char *glyphname)
int isinencoding (LPSTR *lpEncoding, char *glyphname) {
//	LPSTR = lpEncoding;
	int i; 

	for (i = 0; i < MAXCHRS; i++) {
		if (lpEncoding[i] == NULL) continue;
//		if (strcmp(s, glyphname) == 0) return i;
		if (strcmp(lpEncoding[i], glyphname) == 0) return i;
//		s += MAXCHARNAME;
	}
	return -1;			/* not in currently selected encoding */
}

#ifdef USEUNICODE

/* Do this only in NT ? Do this only for TT fonts ? Or do for either one */
/* 	if (truetypeflag || bWinNT) ? */
/* if buffer == NULL just return the count */

/* used first to get total glyph count with buffer == NULL */
/* NOTE: this will not take into account repeated entries in encoding ... */

#ifdef LONGNAMES
// int AddNamedGlyphs (HFILE afmfile, char *buffer, LPSTR lpEncoding, HDC hTestDC)  
int AddNamedGlyphs (HFILE afmfile, char *buffer, LPSTR *lpEncoding, HDC hTestDC) 
#else
// int AddNamedGlyphs (FILE *afmfile, char *buffer, LPSTR lpEncoding, HDC hTestDC) 
int AddNamedGlyphs (FILE *afmfile, char *buffer, LPSTR *lpEncoding, HDC hTestDC) 
#endif
{
	WORD UID;						/* Unicode */
	MAT2 Mat;						/* Transformation matrix */
	GLYPHMETRICS GMDefault;			/* Glyph Metrics for default char */
	DWORD nLenDefault;				/* space needed for default char */
	GLYPHMETRICS GM;				/* Glyph Metrics structure */
	DWORD nLen;
	int xll, yll, xur, yur;
	int ntable, i;
	int ncount = 0;
	int charwidth;
	int ncode;
	char *glyphname;

/*	Set up identity transformation matrix */ /* which is ignored GGO_NATIVE */
	Mat.eM11.value=1;	Mat.eM12.value=0;
	Mat.eM21.value=0;	Mat.eM22.value=1;
	Mat.eM11.fract=0;	Mat.eM12.fract=0;
	Mat.eM21.fract=0;	Mat.eM22.fract=0;
/*	Get shape and size of default character for comparison */
//	nLenDefault = GetGlyphOutlineW(hTestDC,
//		notdefUID, GGO_NATIVE, &GMDefault, 0, NULL, &Mat); // 2000 June 7
	nLenDefault = GetGlyphOutlineW(hTestDC,
		   notdefUID, GGO_METRICS, &GMDefault, 0, NULL, &Mat);
	if (bDebug > 1) {
		sprintf(debugstr, "default ret %d Inc %d %d Org %d %d Black %d %d",
				nLenDefault, GMDefault.gmCellIncX, GMDefault.gmCellIncY,
				GMDefault.gmptGlyphOrigin.x, GMDefault.gmptGlyphOrigin.y,
				GMDefault.gmBlackBoxX, GMDefault.gmBlackBoxY);
		OutputDebugString(debugstr);
	}

/*	ntable = sizeof(unicodeMap) / sizeof(unicodeMap[1]); *//* Adobe names */
	ntable = sizeofUID();			/* in winsearc.c */
	for (i = 0; i < ntable; i++) {
		UID = unicodeMap[i].UID;		/* get Unicode number */
		glyphname = unicodeMap[i].glyphname;
		if (buffer != NULL) {
			if (isinencoding(lpEncoding, glyphname) >= 0)
				continue; 					/* already done, ignore */
		}
/*		get glyph metrics - returns size of buffer needed for outline */
//		nLen = GetGlyphOutlineW(hTestDC,
//				UID, GGO_NATIVE, &GM, 0, NULL, &Mat); // 2000 June 7
		nLen = GetGlyphOutlineW(hTestDC,
					UID, GGO_METRICS, &GM, 0, NULL, &Mat);
/*		ignore if there was some kind of error */
		if (nLen <= 0) continue;			// GDI_ERROR
/*		see whether appears to be the default character */
		if (nLen == nLenDefault &&
			GM.gmCellIncX == GMDefault.gmCellIncX &&
			GM.gmBlackBoxX == GMDefault.gmBlackBoxX &&
			GM.gmBlackBoxY == GMDefault.gmBlackBoxY &&
			GM.gmptGlyphOrigin.x == GMDefault.gmptGlyphOrigin.x &&
			GM.gmptGlyphOrigin.y == GMDefault.gmptGlyphOrigin.y) {
			continue;
		}
		if (buffer != NULL) {	/* second pass actually want output */
			charwidth = GM.gmCellIncX;
			xll = GM.gmptGlyphOrigin.x;
			yll = GM.gmptGlyphOrigin.y - GM.gmBlackBoxY;
			xur = GM.gmptGlyphOrigin.x + GM.gmBlackBoxX;
			yur = GM.gmptGlyphOrigin.y;
#ifdef DEBUGCHARMETRICS
			if (bDebug > 1) {
				sprintf(debugstr,
						"%d UID %04X %s nLen %d width %d xll %d yll %d xur %d yur %d", 
						i, UID, glyphname, nLen, charwidth, xll, yll, xur, yur);
				OutputDebugString(debugstr);
			}
/*			if (bDebug > 1) {
				sprintf(debugstr,
						"%d UID %04X %s nLen %d width %d xll %d yll %d xur %d yur %d", 
						i, UID, glyphname, nLen, charwidth, xll, yll, xur, yur);
				OutputDebugString(debugstr);
			} */
#endif
			ncode = -1;
			if (bAFMTextFont) ncode = standardcode(glyphname); /* expect -1 */
			sprintf(buffer, "C %d ; WX %d ; N ", ncode, charwidth);
			fputs(buffer, afmfile);
			sprintf(buffer, "%s ;", glyphname);
			fputs(buffer, afmfile);
			sprintf(buffer, " B %d %d %d %d ;\n", 
					xll, yll, xur, yur);
			fputs(buffer, afmfile);
		}
		ncount++;
	}
	return ncount+1;	/* add one for default char which was not counted */
}
#endif

/**************************************************************************/

void FreeVector(HGLOBAL);

typedef short int far   *LPSHORT;

/* The core of it - a monster function by now both Type 1 and TrueType */
/* can be called directly from WriteAFMFileSafe (called from dviwindo.c) */
/* used in WriteTFM... batchflag == 0 and WriteAllTFMs... batchflag != 0 */
/* returns non-zero if some serious error in creating font or what */
/* returns -1 if failed or cancelled 1995/July/19 */

/*	For Type 1 fonts, could use ATMGetFontInfo to get much of this */
/*	including widths and kern pairs... */

int WriteAFMFile (HWND hWnd, HDC hDC, int bMakeTFM, int batchflag) {
	HFONT hFont=0, hFontOld=0;
	HDC hTestDC=NULL;
	int i, flag, ncount, ncheck;
/*	int k, n; */
	int truetypeflag=0;	/* may get set by FileFromFace ? */
	int atmfontflag=0;	/* may get set MyATMFontSelected (or FileFromFace) */
	int nkerns=0, kern;
	int kcount;
	unsigned int chara, charb;
	int firstchar, lastchar;
	int charset, family, winansi; 
	int slant, slantdecimal, slantdegree, slantsign;
	int charweight;
	int xheight, capheight, charascender, chardescender;	/* from .etm */
	int underoffset, underwidth;							/* from .etm */
	int charascent, chardescent, maxwidth, charheight;		/* from .tm */
	int xll, yll, xur, yur;									/* for BBox */
	char *s;
	int width;
	int failed = 0;
	int err;
	HGLOBAL hMemTemp=NULL;			/* used only locally here 95/Dec/25 */
	KERNPAIR *lpKernPairs=NULL;		/* attempt at doing this right */
	LPSTR *lpEncoding=NULL;			/* pointer to array of pointers to char names */
	WORD lpOutData = sizeof(EXTTEXTMETRIC);
	EXTTEXTMETRIC ExtTextMetrics;	/* maybe allocate from memory ? */
	int exttextflag=0;				/* did we get extra data ? 96/Jan/8 */
	int charwidth;
/*  following for TrueType fonts */
	int outsize;
/*	HLOCAL hMetric=NULL; */			/* flushed 1995/July/27 */
	OUTLINETEXTMETRIC *lpMetric=NULL;	/* for TrueType only ? */
	BOOL bABCFlag=0;				/* non-zero if ABC metrics available */
	BOOL bFixedPitch;
	ABC abcwidths[256];
/*  above for TrueType fonts */
	char afmfilename[MAXFILENAME]=""; /* use for vector in and afm out .. */
	char fname[MAXFILENAME]="";
	char szFontName[MAXFILENAME]="";		/* 98/Aug/25 */
//	char charaname[MAXCHARNAME], charbname[MAXCHARNAME];
	char charaname[MAXFILENAME], charbname[MAXFILENAME];
	int ret;
	ATMBBox FontBBox;
/*	char szFaceStyle[32+2]; */		/* for ATM backdoor FaceName + style */
	HGLOBAL hEncodingSaved;			/* saved hEncoding */
	int nEncodingBytesSaved;		/* saved nEncodingBytes */ /* 96/July/21 */
/*	LPINT lpKernAmount; */
/*	LPSHORT lpKernAmount; */
	WORD options;
/*  Following is for getting TT Char BBox */
	GLYPHMETRICS GM;				/* Glyph Metrics structure */
	DWORD nLen=0;					/* space needed for data */	
#ifdef USEUNICODE
	GLYPHMETRICS GMDefault;			/* Glyph Metrics for default char */
	DWORD nLenDefault=0;			/* space needed for default char */
#endif
	MAT2 Mat;						/* Transformation matrix */
	char buffer[128];				/* for AFM file line */
#ifdef LONGNAMES
	HFILE afmfile;
#else
	FILE *afmfile; 
#endif
	HCURSOR hSaveCursor=NULL;		/* shadow global version noninit ??? */
	HGLOBAL hInfo=NULL;				/* 1996/July/28 */
	LPATMInstanceInfo lpInfo=NULL;	/* 1996/July/28 */
	int defaultchar;				/* 1997/Jan/15 */
	int bANSIFlag;					/* 1997/Apr/20 */

	afmfile = BAD_FILE;

	if (bTTUsable) {				/* for TT Glyph Metrics 95/July/8 */
		Mat.eM11.value=1;	Mat.eM12.value=0;
		Mat.eM21.value=0;	Mat.eM22.value=1;
		Mat.eM11.fract=0;	Mat.eM12.fract=0;
		Mat.eM21.fract=0;	Mat.eM22.fract=0;
	}

/*	Try and guess encoding `ansinew' or `numeric' for TT fonts */
/*	User specified ENCODING, `ansinew' or FontSpecific for T1 fonts */
	if (szEncodingVector != NULL) free (szEncodingVector);	/* safe ? */
	szEncodingVector = zstrdup(GuessEncoding());			/* 94/Dec/31 */
#ifdef DEBUGWRITEAFM
	if (bDebug > 1) OutputDebugString(szEncodingVector);
#endif
/*	flush the following --- just confuses the user ... */
/*	we'll just trust the Guess Encoding() to get the encoding right ... */
/*	if (!bATMLoaded) */
/*	We assume that for ATM and TT we can figure out encoding ... 95/July/8 */
/*	Don't know for sure yet (but testwantttf?) whether its T1 or TT font ... */
/*	if (!bATMLoaded && !bTTUsable) */
#ifdef USEUNICODE
	if ((!testwantttf && !bATMLoaded && !bUseNewEncodeT1) ||
		(testwantttf && !bTTUsable)) {			/* 97/Jan/21 */
#else
	if ((!testwantttf && !bATMLoaded) ||
		(testwantttf && !bTTUsable)) {
#endif
//		flag = DialogBox(hInst, "EncodingSelect", hWnd, (DLGPROC) EncodingDlg);
		flag = DialogBox(hInst, "EncodingSelect", hWnd, EncodingDlg);
		if (flag == 0) return -1;				/* exit: user cancelled */
	}
/*	file name in `encoding' */

	if (hPrintDC != NULL) {	/* use printer DC if printer has been selected */
		if (bDebug > 1) OutputDebugString("Using PrintDC\n");	/* 95/July/8 */
		hTestDC = hPrintDC;
	}
	else hTestDC = hDC;							/* else use screen DC */

/*	(void) SetMapMode(hTestDC, MM_TWIPS); *//* set unit to twips */ /* ??? */
/*	if (bDebug > 1) {
		sprintf(debugstr, "%s BOLD: %d ITALIC: %d TTF: %d\n",
				TestFont, bCheckBoldFlag,  bCheckItalicFlag, testwantttf);
		OutputDebugString(debugstr);
	} */ 				/* 1995/April/15 */
/*	hFont = createatmfont(TestFont, METRICSIZE, 0, */
	hFont = createatmfont(TestFont, metricsize, 0, 
				bCheckBoldFlag, bCheckItalicFlag, testwantttf);
	if (hFont == NULL) {						/* rewritten 94/Dec/31 */
		failed = 1;
		winerror("Can't create test font");		/* very unlikely */
		return failed;
	}

/*	winerror(szEncodingVector); */					/* debugging */
	hMemTemp = getencoding(szEncodingVector); 
	if (hMemTemp == NULL) {
/*		winerror ("Unable to allocate memory"); */ /* or user cancelled */
/*		or unable to find encoding vector */
		return -1;
	}
	if (batchflag == 0) {				/* in batch mode already done */
		bHourFlag = 1;
		hSaveCursor = SetCursor(hHourGlass);			/* 1996/July/25 */
	}
	if (bATMReencoded) UnencodeFont(hTestDC, -1, 1);
	if (!testwantttf && bATMPrefer && bATMLoaded) {
		if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
		else options = ATM_SELECT;
		(void) MyATMSelectObject (hTestDC, hFont, options, &hFontOld);
	}
	else hFontOld = SelectObject(hTestDC, hFont);	/* select test font */

	if (hFontOld == NULL) {
		failed = 1;
/*		winerror("Can't select font"); *//* unlikely ... */
		if (wincancel("Can't create test font")) goto cleanup;
	}

	if (bUseATMFontInfo) {
		if (MyATMFontSelected(hTestDC)) {
/*			we don't presently use width from this call, but ... */
/*			hInfo = GetATMFontInfo(hTestDC, ATM_GETWIDTHS | ATM_GETKERNPAIRS, 1); */
			hInfo = GetATMFontInfo(hTestDC, ATM_GETWIDTHS | ATM_GETKERNPAIRS);
			if (hInfo != NULL) lpInfo = GlobalLock(hInfo);
		}
	}

#ifdef USEUNICODE
/*	if (bUseNewEncode) bFontEncoded = bCheckReencode; */
/*	new method only in Windows NT or for TT in 95 with UseNewEncode */
/*	in Windows 95 ATM will be loaded and hence hEncoding != NULL */
	if ((testwantttf && bUseNewEncodeTT) ||
		(!testwantttf && bUseNewEncodeT1)) bFontEncoded = bCheckReencode;
	else bFontEncoded = 0;
/*	else */
/*	hEncoding will be NULL in Windows NT */
	if (bCheckReencode && !testwantttf && hEncoding != NULL)
		ReencodeFont(hTestDC, -1, 0); 				/* 94/Dec/25 */
#else
	if (bCheckReencode && !testwantttf && hEncoding != NULL)
		ReencodeFont(hTestDC, -1, 0); 				/* 94/Dec/25 */
#endif

/*	flag = GetCharWidth(hTestDC, 0, 255, charwidths); */
	flag = GetWidth(hTestDC, 0, 255, charwidths);
/*	Use widths in hInfo ? but those are for unreencoded font ? */
	if (flag == 0) {
		winerror("Can't get character widths");
		failed = 1;
		goto cleanup;							/* 94/Dec/31 */
	}
/*	else */
	if (GetTextMetrics(hTestDC, &TextMetric) == 0) {
		failed = 1;
		winerror("GetTextMetrics failed"); 		/* unlikely ... */
/*		if (wincancel("GetTextMetrics failed")) */
		goto cleanup;
	}
	charset = TextMetric.tmCharSet;
/*	family = TextMetric.tmPitchAndFamily >> 4; */
	family = TextMetric.tmPitchAndFamily;	/* 1993/June/3 */
/*	debugging TextMetric fields */
#ifdef DEBUGWRITEAFM
	if (bDebug > 1) {
		sprintf(debugstr, "TestFont %s charset %X pitchandfamily %X (%s)\n",
				TestFont, charset, family, "TextMetric");
		OutputDebugString(debugstr);
	}
#endif
/*	if (charset == ANSI_CHARSET && */		/* 1993/June/3 */
/*	if ((charset == ANSI_CHARSET || charset == DEFAULT_CHARSET) &&
		(family & 0xF0) != FF_DECORATIVE) winansi = 1;
	else winansi = 0; */
	winansi = 1;			/* assume for a start its plain text font */
	if (strcmp(szEncodingVector, "ansinew") == 0 ||
		strcmp(szEncodingVector, "ansi") == 0) bANSIFlag = 1;
	else bANSIFlag = 0;
	if (charset != ANSI_CHARSET && charset != DEFAULT_CHARSET) winansi = 0;
							/* not plain text if charset not ANSI or DEFAULT */
	if (!testwantttf && (family & 0xF0) == FF_DECORATIVE) winansi = 0;
							/* not plain text font if T1 and DECORATIVE */
/*	Revised 97/Feb/5 to deal with auto converted math fonts */
	if (bDecorative) {	/* treat decorative TT font as non-text */
		if (testwantttf && (family & 0xF0) == FF_DECORATIVE)  winansi =0; 
	}
/*	Revised 97/Feb/16 to deal with auto converted fixed width non text fonts */
	if (bDontCare) {	/* treat decorative TT font as non-text */
		if (testwantttf && (family & 0xF0) == FF_DONTCARE) winansi = 0;
	}
	if (bDebug > 1) {
		sprintf(debugstr,
				"winansi %d vec %s charset %X family %X ttf %d",
				winansi, szEncodingVector, charset, family, testwantttf);
		OutputDebugString(debugstr);
	}
/*	if (winansi && testwantttf == 0 && bCheckReencode)
		winansi = 0; */		/* not ANSI if T1 and we reencode it 94/Dec/31 */
#ifdef USEUNICODE
	if ((bUseNewEncodeTT && testwantttf) ||
		(bUseNewEncodeT1 && !testwantttf)) {
/*		if (bCheckReencode) winansi = 0; */
		if (bCheckReencode && bANSIFlag == 0)
				winansi = 0;	/* not ANSI if we reencode it 97/Jan/17 */
	}
/*	else if (testwantttf == 0) */	/* T1 can be reencoded */
	else if (testwantttf == 0 && hEncoding != NULL) { /* T1 can be reencoded */
/*		if (bCheckReencode) winansi = 0; */
		if (bCheckReencode && bANSIFlag == 0) 
			winansi = 0;	/* not ANSI if T1 and we reencode it 94/Dec/31 */
	}
#else
	if (testwantttf == 0) {	/* only T1 can be reencoded */
/*		if (bCheckReencode) winansi = 0; */
		if (bCheckReencode && bANSIFlag == 0)
			winansi = 0;	/* not ANSI if T1 and we reencode it 94/Dec/31 */
	}
#endif

	if (checkansiencode(winansi, charset, family)) {
		failed = 1;
		goto cleanup; 					/* 95/July/20 ? */
	}
	memset (&ExtTextMetrics, 0, sizeof(EXTTEXTMETRIC));		/* clear out */

	flag = ExtEscape(hTestDC, GETEXTENDEDTEXTMETRICS, sizeof(WORD), 
		(LPSTR) &lpOutData, sizeof(EXTTEXTMETRIC), (LPSTR) &ExtTextMetrics);
/*	not sure about 3 rd & 4th argument ... */
/*	flag = Escape(hTestDC, GETEXTENDEDTEXTMETRICS, sizeof(WORD), 
		(LPSTR) &lpOutData, (LPSTR) &ExtTextMetrics); */

	if (flag <= 0) {
		if (bDebug) winerror("GETEXTENDEDTEXTMETRICS Escape failed");
/*		failed = 1; */
/*		goto cleanup; */					/* 95/July/20 */
	}
	else exttextflag = 1;		/* did snag ExtendedTextMetrics information */

	if (failed) goto cleanup;				/* 94/Dec/31 */

/*	Construct a useable AFM file name */

/*	Use MyATMFontSelected to decide whether this is an ATM font */
	if (bATMLoaded) atmfontflag = MyATMFontSelected(hTestDC);

	truetypeflag = testwantttf;	/* *initial* only for FileFromFace 95/April/15 */
/*	if (truetypeflag) atmfontflag = 1;	else atmfontflag = 0; */
/*	FileFromFace(fname, TestFont, &atmfontflag, &truetypeflag); */
	err = FileFromFace(hDC, fname, TestFont, &atmfontflag, &truetypeflag);
#ifdef DEBUGAFMNAME
	if (bDebug > 1) OutputDebugString(fname);
#endif
	if (err) {
		failed = 1;						/* 95/July/19 */
		goto cleanup;
	}

/*	For TrueType fonts get ABC metric info while we are at it */
/*	Better be true if we get here since Windows 3.0 does not have TT fonts */
/*	Windows NT and W2K also supports this for Type 1 fonts */
	if (truetypeflag || bWinNT) {
/*		get size of local space needed for metrics */
/*		outsize = GetOutlineTextMetricsW(hTestDC, 0, NULL); */
		outsize = GetOutlineTextMetrics(hTestDC, 0, NULL);
/*		but don't trust tmFirstChar, tmLastChar, tmDefaultChar */
		if (outsize > 0) {
/*			hMetric = LocalAlloc (LHND, outsize); 
			if (hMetric == NULL) {
				winerror("Unable to allocate memory");
				outsize = 0;
				failed = 1;
				goto cleanup;
			}
			else {
				lpMetric = LocalLock (hMetric);
				if (lpMetric == NULL) goto cleanup;	
				outsize = GetOutlineTextMetrics(hTestDC, outsize, lpMetric);
			} */	/* 1995/July/27 */
			lpMetric = (OUTLINETEXTMETRIC *) LocalAlloc(LPTR, outsize); 
			if (lpMetric == NULL) {
				winerror("Unable to allocate memory");
/*	Take more drastic action ? */
				outsize = 0;
				failed = 1;
				goto cleanup;
			}
/*			GetOutlineTextMetrics works with ATM 4.0 in Windows 95 also */
/*			but we get here only with TT fonts presently ... */
			else {
/*				outsize = GetOutlineTextMetricsW(hTestDC, outsize,	lpMetric); */
				outsize = GetOutlineTextMetrics(hTestDC, outsize, lpMetric);
/*				but don't trust tmFirstChar, tmLastChar, tmDefaultChar */
				defaultchar = lpMetric->otmTextMetrics.tmDefaultChar;
#ifdef DEBUGWRITEAFM
				if (bDebug > 1) {
					sprintf(debugstr, "Default Char %d\n", defaultchar);
					OutputDebugString(debugstr);
				}
#endif
			}
		}
		if (outsize == 0) {
			failed =1 ;
			winerror("Bad TT Metrics"); 
/*			if (wincancel("Bad TT Metrics")) */
			goto cleanup;
		}

		if (bDebug > 1) {
			OutputDebugString("About to use GetABCCharWidth\n");
		}
/*		GetCharABCWidths works in ATM 4.0 in Windows 95 and W2K (not just TT) */
/*		But we only use it here with TT ... ? */
#ifdef USEUNICODE
/*		if(bUseNewEncode && bCheckReencode) */ /* 97/Jan/25 */
		if (bCheckReencode &&
			((bUseNewEncodeTT && testwantttf) ||
			 (bUseNewEncodeT1 && !testwantttf))) {
			int k, n;
			ABC abc[1];
			bABCFlag = 0;
			for (k = 0; k < 256; k++) {
				n = encodeUID[k];
/*	GetCharABCWidthsW not supported in Windows 95 */				
				if (bWinNT) {
					int ret;
					ret = GetCharABCWidthsW (hTestDC, n, n, abc);
					if (ret) {
						abcwidths[k] = abc[0];
						bABCFlag++;
					}
					else {	/* never seems to happen ... */
					abcwidths[k].abcA=abcwidths[k].abcB=abcwidths[k].abcC=0;
#ifdef DEBUGWRITEAFM
					if (bDebug > 1) {
						sprintf(debugstr, "ABC failed on char %d (%04X)\n", k, n);
						OutputDebugString(debugstr); 
					} 	/*	never seems to fail, no matter what ... */
#endif
					}
				}
#ifdef IGNORED
/* following doesn't work anyway because GetCharWidth always returns zero ! */
				else {	/* above fails in Windows 95 */
					int ret;
					int wid[1];
/*					ret = GetCharWidthW (hTestDC, n, n, wid); */ /* always zero */
					ret = GetCharWidthW (hTestDC, n, n, wid);
					if (ret) {
						if (wid[1] != charwidths[k]) {
#ifdef DEBUGWRITEAFM
							if (bDebug > 1) {
								sprintf(debugstr, "charwidths[%d] %d wid %d\n",
										k, charwidths[k], wid[1]);
								OutputDebugString(debugstr);
							}
#endif
						}
						else charwidths[k] = wid[1];		/* redundant ? */
/*						bABCFlag++;	*/	/* yes, but... */
					}
#ifdef DEBUGWRITEAFM
					else if (bDebug > 1) {
						sprintf(debugstr, "GetCharWidthW %d (%04X) failed\n",
								k, n);
						OutputDebugString(debugstr);
					}
#endif
				}	/* end of attempt at Windows 95 case */
#endif
			}
/*			bABCFlag = 1; */
		}
		else {
			bABCFlag = GetCharABCWidthsA (hTestDC, 0, 255, abcwidths);
			if (bABCFlag == 0) {
				failed = 1;
				winerror ("Bad ABC Metrics"); 
				goto cleanup;
			}
		}
#else		
		bABCFlag = GetCharABCWidths (hTestDC, 0, 255, abcwidths);
		if (bABCFlag == 0) {
			failed = 1;
			winerror ("Bad ABC Metrics"); 
			goto cleanup;
		}
#endif
#ifdef DEBUGATM
		if (bDebug > 1) OutputDebugString("Got ABC metrics\n");
#endif
	}	/* end of TrueType fonts get ABC metric info */

/*	Following call writes back into afmfilename, and opens file for writing */
	afmfile = makeafmfilename(afmfilename, fname);	/* create AFM file */
#ifdef DEBUGAFMNAME
	if (bDebug > 1) OutputDebugString(afmfilename);
#endif
	if (afmfile == BAD_FILE) {			/* 95/Apr/4 */
		failed = 1;
		goto cleanup;
	}

	lpEncoding = GlobalLock(hMemTemp);
/*	if (lpEncoding == NULL) winerror("lpEncoding NULL"); *//* debugging only */
	if (lpEncoding == NULL) goto cleanup;	/* sanity check */

	if (truetypeflag &&	strcmp(szEncodingVector, "ansinew") == 0) {
/*	Flush bogus `dotlessi' and `caron' in ansinew for fixed ATM */
/*	do we still need to do this ? */
//		(void) strcpy(str, lpEncoding + 141 * MAXCHARNAME);
		strcpy(str, lpEncoding[141]);
		if (strcmp(str, "caron") == 0) {
//			*(lpEncoding + 141 * MAXCHARNAME) = '\0';
			if (lpEncoding[141] != NULL) free(lpEncoding[141]);
//			lpEncoding[141] = zstrdup("");
			lpEncoding[141] = NULL;
		}
//		(void) strcpy(str, lpEncoding + 157 * MAXCHARNAME);
		strcpy(str, lpEncoding[157]);
		if (strcmp(str, "dotlessi") == 0) {
//			*(lpEncoding + 157 * MAXCHARNAME) = '\0';
			if (lpEncoding[157] != NULL) free(lpEncoding[157]);
//			lpEncoding[157] = zstrdup("");
			lpEncoding[157] = NULL;
		}
/*	and deal with repeat encoding of `space' and `hyphen' ??? */
//		(void) strcpy(str, lpEncoding + 160 * MAXCHARNAME);
		strcpy(str, lpEncoding[160]);
		if (strcmp(str, "space") == 0) {
//			strcpy(lpEncoding + 160 * MAXCHARNAME, "nbspace");		/* ??? */
			if (lpEncoding[160] != NULL) free(lpEncoding[160]);
			lpEncoding[160] = zstrdup("nbspace");		/* ??? */
		}
//		(void) strcpy(str, lpEncoding + 173 * MAXCHARNAME);
		strcpy(str, lpEncoding[173]);
		if (strcmp(str, "hyphen") == 0) {
//			strcpy(lpEncoding + 173 * MAXCHARNAME, "sfthyphen");	/* ??? */
			if (lpEncoding[173] != NULL) free(lpEncoding[173]);
			lpEncoding[173] = zstrdup("sfthyphen");	/* ??? */
		}
	} /* end of if (truetypeflag &&	strcmp(szEncodingVector, "ansinew") */

/*	don't trust the following if reencoded */ /* not used anymore */
	firstchar = TextMetric.tmFirstChar;  
	lastchar = TextMetric.tmLastChar;  			
/*	sprintf(str, "first %d last %d", firstchar, lastchar); */
/*	winerror(str); */						/* debugging */

/*	if (bDebug > 1) OutputDebugString("StartFontMetrics 2.0\n"); */
	fputs("StartFontMetrics 2.0\n", afmfile);
/*	try and construct a resonable font name ... */
	strcpy(str, TestFont);
/*	deal with common alias ... */
	if (strcmp(str, "Times Roman") == 0) strcpy(str, "Times"); /* ??? */
/*	Try and make FontName from FaceName for Type 1 font (first guess) */
	if (truetypeflag == 0) {			/* for ATM font ? */
		if (bCheckBoldFlag != 0 || bCheckItalicFlag != 0)
			strcat(str, "-");
		if (bCheckBoldFlag != 0) strcat(str, "Bold");
		if (bCheckItalicFlag != 0) strcat(str, "Italic");
	}

/*	if (atmfontflag) */					/* if we can actually ask ATM ... */
	if (atmfontflag && bATMLoaded) {	/* if we can actually ask ATM ... */
		strcpy(str, TestFont);			/* in case it fails ... */
		GetPSFontName(TestFont, bCheckBoldFlag, bCheckItalicFlag, str);
	}

/*	leave it in str for FontName output further down */
/*	for TrueType font should use what was constructed */
	if (truetypeflag) strcpy(str, szFullName);	/* 1992/Sep/17 */
	strcpy(fname, str);	/* save it for message box 1992/Sep/19 */

	if (*fname == '\0') {		/* sanity check 1995/July/15 */
		strcpy(str, TestFont);				/* Desperation 95/July/19 */
		strcpy(fname, TestFont);			/* Utter Desperation 95/Aug/20 */
		if ((s = strchr(fname, '(')) != NULL) {
			*(s-1) = '\0'; /* (TT) */
		}
/*		if (wincancel("No font name\nSelected style does not exist")) */
		if (AvoidTrouble(TestFont, bCheckBoldFlag, bCheckItalicFlag)) {
			failed = 1;
			goto cleanup;
		}
	}
	
	if ((s = strchr(str, '(')) != NULL)	{
		*(s-1) = '\0'; /* flush (TT) */ // experiment
		if (bWinNT5) {		// experiment 2000 June 7
			truetypeflag = 1;
			atmfontflag = 0;
		}
	}
	else {
		if (bWinNT5) {		// experiment 2000 June 7
			truetypeflag = 0;
			atmfontflag = 1;
		}
	}

	sprintf(buffer, "Comment Partial AFM file for font: %s\n",	str);
	fputs(buffer, afmfile);
	sprintf(buffer, "Comment Based on %s DC\n",
		(hPrintDC == hTestDC) ? "Printer" : "Screen");
	fputs(buffer, afmfile);
	if (truetypeflag)
		fputs("Comment Apparently a TrueType font\n", afmfile);
/*	for ATM fonts we can get real encoding - what if not reencoded ? */
	if (!atmfontflag || hEncoding != NULL) {
		sprintf(buffer, "Comment Using `%s' as encoding vector\n", 
			szEncodingVector);
		fputs(buffer, afmfile);
	}
	if (! truetypeflag && ! atmfontflag)
		fputs("Comment following is merely the Face Name\n", afmfile);
/*	sprintf(buffer, "FontName %s\n", str); */
	if (! truetypeflag && lpMetric != NULL)
		sprintf(buffer, "Comment MSMenuName %s\n", str); 
	else {
		sprintf(buffer, "FontName %s\n", str);			/* ? 98/Aug/26 */
/*		remember FontName for info box showing ? */	
		strcpy(szFontName, str);							/* ? 98/Aug/26 */
	}
	fputs(buffer, afmfile);

/*	Cannot add two pointers ??? (int) cast work-around 1995/August/10 */
	if (lpMetric != NULL) {			/* for TrueType fonts ... */
		sprintf(buffer, "FamilyName %s\n",
				((char *) lpMetric) + (int) lpMetric->otmpFamilyName);  
		fputs(buffer, afmfile);
		sprintf(buffer, "FullName %s\n",
				((char *) lpMetric) + (int) lpMetric->otmpFullName); 
//		if (bWinNT5) {			// experiment 2000 June 7
//			if ((s = strstr(buffer+9, "1.0;ADBE;")) != NULL)
//				strcpy(s, s+9);
//			may still have style modified attached ....
//			this is actually more like the Face Name plus Style?
//			could use GetPSFontName - except it doesn't work in W2K!
//		}
		fputs(buffer, afmfile);
/*		sprintf(buffer, "Comment MSMenuName %s\n",
				((char *) lpMetric) + (int) lpMetric->otmpFaceName); */
		if (truetypeflag) sprintf(buffer, "Comment MSMenuName %s\n",
				((char *) lpMetric) + (int) lpMetric->otmpFaceName); 
		else {
			sprintf(buffer, "FontName %s\n",
				 ((char *) lpMetric) + (int) lpMetric->otmpFaceName); /* ? 98/Aug/26 */
			strcpy(szFontName,  ((char *) lpMetric) + (int) lpMetric->otmpFaceName); 
/*			remember FontName for info box showing ? */	
		}
		fputs(buffer, afmfile);
		sprintf(buffer, "Comment StyleName %s\n",
				((char *) lpMetric) + (int) lpMetric->otmpStyleName);  
		fputs(buffer, afmfile);
	}
/*	The above seems to interchange PS FontName and Windows Face name for T1 */
/*	fprintf(afmfile, "Weight "); */
	fputs("Weight ", afmfile);
	charweight = TextMetric.tmWeight;
	if (charweight == 400) fputs("Medium\n", afmfile);
	else if (charweight > 500) fputs("Bold\n", afmfile);
	else if (charweight > 400) fputs("Demi\n", afmfile);
	else if (charweight < 400) fputs("Light\n", afmfile);

	if (hInfo != NULL) {				/* for ATM - redundant */
		slant = lpInfo->italic_angle;
	}

	if (lpMetric != NULL) slant = lpMetric->otmItalicAngle;
	else if (exttextflag) slant = ExtTextMetrics.etmSlant;
	else if (TextMetric.tmItalic) slant = 100;
	else slant = 0;

/*	etmSlant for TT fonts may have wrong sign, but otmItalicAngle seems OK */
	slantdecimal = slant;
/*	avoid floating arithmetic and fprinting if possible */
	if (slantdecimal < 0) {
		slantdecimal = - slantdecimal; slantsign = -1;
	}
	else slantsign = 1;
	slantdegree = slantdecimal / 10;
	slantdecimal = slantdecimal - slantdegree * 10;
	slantdegree = slantdegree * slantsign;
	if (slantdecimal == 0) 
		sprintf(buffer, "ItalicAngle %d\n", slantdegree);
	else sprintf(buffer, "ItalicAngle %d.%d\n", 
		slantdegree, slantdecimal);
	fputs(buffer, afmfile);

/*	Try and guess whether fixed pitch or not */
/*	or use testpitchandfamily FF_MODERN ??? */
	bFixedPitch = IsItFixed(abcwidths, charwidths, bABCFlag);

/*	if (bFixedPitch != 0) fprintf(afmfile, "IsFixedPitch true\n");*/
	sprintf(buffer, "IsFixedPitch %s\n", bFixedPitch ? "true" : "false");
	fputs(buffer, afmfile);

/* Underline position and thickness */
	if (hInfo != NULL) {				/* for ATM - redundant */
		underoffset = lpInfo->underline_position;
		underwidth = lpInfo->underline_thickness;
	}

/*	The following may not be accurate for Type 1 fonts ... */
	if (exttextflag) {
		underoffset = ExtTextMetrics.etmUnderlineOffset;
		underwidth = ExtTextMetrics.etmUnderlineWidth;
	}
	else {										/* 96/Jan/8 */
		underoffset = 75;
		underwidth = 50;
	}
/*	Includes various kludges for dealing with buggy metrics of TT fonts */
/*	TT fonts return these metrics based on 2048 em it seems ... */
	if (truetypeflag) {
		if (underoffset < 0) underoffset = - underoffset;
		if (underoffset > 150) 	underoffset =
			(int) (((long) underoffset * 1000 + 1000) >> 11);
		if (underwidth > 100) underwidth =
			(int) (((long) underwidth * 1000 + 1000) >> 11);
	}
/*	here `underlineposition' is TOP of underline stroke */ /* 1993/June/3 */
	underoffset = underoffset + underwidth / 2;
	if (lpMetric != NULL) {	/* for TT font */
		underoffset = -lpMetric->otmsUnderscorePosition;
		underwidth = lpMetric->otmsUnderscoreSize;
#ifdef IGNORED
		if (metricsize != 1000)	{
			underoffset =
			(int) (((long) underoffset * 1000 + 499) / metricsize);
			underwidth =
			(int) (((long) underwidth * 1000 + 499) / metricsize);
		}  /* lets assume we use 1000 */
#endif
	}
	sprintf(buffer, "UnderlinePosition %d\n", -underoffset);
	fputs(buffer, afmfile);
	sprintf(buffer, "UnderlineThickness %d\n",	underwidth);
	fputs(buffer, afmfile);
/*	Version, Notice, FontBBox ??? */
	fputs("EncodingScheme ", afmfile);
/*	if (winansi != 0) fputs("MicroSoft Windows ANSI 3.1\n", afmfile);
	else fputs("FontSpecific\n", afmfile); */
/*	should make this StandardEncoding for text font ... */
	bAFMTextFont = 0;
	if (strcmp(szEncodingVector, "numeric") == 0) 
		fputs("FontSpecific\n", afmfile);		/* symbol/decorative */
	else {
		fputs("StandardEncoding\n", afmfile);	/* text font - 98/Aug/26 */
/*		now also remap character codes to suite StandardEncoding */
		bAFMTextFont = 1;		/* force character code to ASE */
	}

/*	Now for XHeight, CapHeight, Ascender and Descender */
	if (hInfo != NULL) {				/* for ATM - redundant */
		charascender = lpInfo->ascent;
		chardescender = lpInfo->descent;
		capheight = lpInfo->cap_height;
		xheight = lpInfo->x_height;
	}

	if (exttextflag) {
		charascender = ExtTextMetrics.etmLowerCaseAscent;/* Ascender */
		chardescender = ExtTextMetrics.etmLowerCaseDescent;/* Descender */
		capheight = ExtTextMetrics.etmCapHeight;	/* CapHeight */
/*	Get CapHeight in another way for TT fonts ? */ /* always <= Ascender ! */
/*	if (hMetric != NULL) capheight = lpMetric->otmsCapEmHeight; */ /*500*/
		xheight = ExtTextMetrics.etmXHeight;		/* XHeight */
	}
	else {											/* 96/Jan/8 */
		charascender = TextMetric.tmAscent;
		chardescender = TextMetric.tmDescent;
		capheight = 700;
		xheight = 500;
	}

/*	Get XHeight in another way for TT fonts ? */	/* always <= 512 ! */
/*	if (hMetric != NULL) xheight = lpMetric->otmsXHeight; */ /* 250 */
/*	if (chardescender < 0) chardescender = - chardescender; */
/*	Includes various kludges for dealing with buggy metrics of TT fonts */
/*	TT fonts return these metrics based on 2048 em it seems ... */
	if (truetypeflag) {  
/*		capheight = capheight/2; */		/* damn TT */
		if (capheight > 1000) capheight =
			(int) (((long) capheight * 1000 + 1000) >> 11);
/*	New XHeight, but GetTextExtent always just gives font height (yur - yll) */
/*		charascender = charascender/2; */	/* damn TT*/
		if (charascender > 1000) charascender =
			(int) (((long) charascender * 1000 + 1000) >> 11);
/*	More kludges for broken TT fonts like WingDings */
		if (chardescender < 0) chardescender = - chardescender;
/*		chardescender = chardescender/2; */	/* damn TT */
		if (chardescender > 333) chardescender =
			(int) (((long) chardescender * 1000 + 1000) >> 11);
	}

/*	xheight in various metrics is worthless for TT font, so get it from `x' */
/*	but only for text font and only if TT is enabled */
/*	Might as well get better estimate of capheight while we are at it ? */
	if (bTTUsable && bABCFlag && winansi) {			/* 1995/July/8 */
/*		GetGlyphOutline works in ATM 4.0 in Windows 95 and W2K (not just TT) */
//		nLen = GetGlyphOutline(hTestDC,
//				'x', GGO_NATIVE, &GM, 0, NULL, &Mat);  // 2000 June 7
		nLen = GetGlyphOutline(hTestDC,
					   'x', GGO_METRICS, &GM, 0, NULL, &Mat);
		if ((long) nLen > 0) xheight = GM.gmptGlyphOrigin.y;
//		nLen = GetGlyphOutline(hTestDC,
//				   'X', GGO_NATIVE, &GM, 0, NULL, &Mat);  // 2000 June 7
		nLen = GetGlyphOutline(hTestDC,
						   'X', GGO_METRICS, &GM, 0, NULL, &Mat);
		if ((long) nLen > 0) capheight = GM.gmptGlyphOrigin.y;
	}	/* end of TrueType methods for capheight, xheight and so on */

	sprintf(buffer, "CapHeight %d\n", capheight);
	fputs(buffer, afmfile);
	sprintf(buffer, "XHeight %d\n", xheight);
	fputs(buffer, afmfile);

/*	May override the above now using lpMetric info ... */
	if (lpMetric != NULL) {
		charascender = lpMetric->otmAscent;
		chardescender = -lpMetric->otmDescent;
/*		if (metricsize != 1000)	{
			charascender =
			(int) (((long) charascender * 1000 + 499) / metricsize);
			chardescender =
			(int) (((long) chardescender * 1000 + 499) / metricsize);
		} */ /* lets assume we use 1000 */
	}
	sprintf(buffer, "Ascender %d\n", charascender);
	fputs(buffer, afmfile);
	sprintf(buffer, "Descender %d\n", -chardescender);
	fputs(buffer, afmfile);
	
/*	Get information for FontBBox */
	charascent = TextMetric.tmAscent;
	chardescent = TextMetric.tmDescent;
	maxwidth = TextMetric.tmMaxCharWidth;
	charheight = TextMetric.tmHeight;
/*	Normalize TEXTMETRIC info to font of size 1000 */
/*	if (metricsize != 1000) {
		charascent =
			(int) (((long) charascent * 1000 + 499) / metricsize);
		chardescent = 
			(int) (((long) chardescent * 1000 + 499) / metricsize);
		maxwidth =
			(int) (((long) maxwidth * 1000 + 499) / metricsize);
		charheight =
			(int) (((long) charheight * 1000 + 499) / metricsize);
	} */ /* lets assume we use 1000 */

	xll = 0; yll = -chardescent; xur = maxwidth, yur = charascent;
	if (lpMetric != NULL) { /* For TT font */
		xll = lpMetric->otmrcFontBox.left;
		yll = lpMetric->otmrcFontBox.bottom;
		xur = lpMetric->otmrcFontBox.right;
		yur = lpMetric->otmrcFontBox.top;
		/* we assume we use 1000 */
	}

	if (hInfo != NULL) {				/* for ATM - redundant */
		xll = lpInfo->font_bb_left;
		yll = lpInfo->font_bb_bottom;
		xur = lpInfo->font_bb_right;
		yur = lpInfo->font_bb_top;
	}

	if (atmfontflag && bATMLoaded) {					/* 94/Dec/31 */
		FontBBox.ll.x = 0; FontBBox.ll.y = 0;
		FontBBox.ur.x = 0; FontBBox.ur.y = 0; 
		ret = MyATMGetFontBBox(hTestDC, &FontBBox);
		if (ret == ATM_NOERR) {
			xll = FontBBox.ll.x;
			yll = FontBBox.ll.y;
			xur = FontBBox.ur.x;
			yur = FontBBox.ur.y;
		}
	}
	sprintf(buffer, "FontBBox %d %d %d %d\n", xll, yll, xur, yur);
	fputs(buffer, afmfile);
	sprintf(buffer, "Comment Height %d (FontBBox yur - yul)\n",
		charheight);
	fputs(buffer, afmfile);

	if (hInfo != NULL) {			/* 1996/July/28 - for ATM */
		sprintf(buffer, "Comment Leading %d (Height - 1000)\n", lpInfo->leading);
		fputs(buffer, afmfile);
		sprintf(buffer, "Comment StemV %d\n", lpInfo->stem_v);
		fputs(buffer, afmfile);	
		sprintf(buffer, "Comment StemH %d\n", lpInfo->stem_h);
		fputs(buffer, afmfile);	
/* Default? Break? Encoding? Figure_Height? Max_width? Avg_Width? */
/* Superior Baseline? Strike offset? Strike thickness? */
	}

/*	Now for individual character metrics */
/*		if (bABCFlag != 0) fprintf(afmfile, 
			"Comment Character BBox%s mere approximations\n", " yll and yur");
		else if (!bATMLoaded) fprintf(afmfile,  
			"Comment Character BBox%s mere approximations\n", ""); */
	if (bDebug > 1) {
		sprintf(debugstr, "truetypeflag %d atmfontflag %d bABCFlag %d bATMLoaded %d",
				truetypeflag, atmfontflag, bABCFlag, bATMLoaded);
		OutputDebugString(debugstr);
	}
	if ((truetypeflag && ! bABCFlag) ||
		(atmfontflag && ! bATMLoaded && ! bABCFlag))	/* 97/Jan/21 */
/*	if ((truetypeflag && !bABCFlag) ||
		(atmfontflag && !bATMLoaded)) */
		fputs("Comment Character BBox mere approximations\n", afmfile);
	ncount = 0;						/* count valid encodings */
/*	for (i = firstchar; i <= lastchar; i++) 
		if (strlen(lpEncoding + i * MAXCHARNAME) > 0) ncount++; */
/*	rewritten 97/Jan/19 --- assumes empty strings in unused positions */
/*	this is an overestimate since we may later come across blanks ... */
	for (i = 0; i < 256; i++) {
		if (lpEncoding[i] == NULL) continue;
		if (*lpEncoding[i] == '\0') continue;
		ncount++;
//		if (*(lpEncoding + i * MAXCHARNAME) != '\0') ncount++;
	}
/*	ncount = lastchar - firstchar + 1); */

/*	if (bATMLoaded) */
	if (atmfontflag && bATMLoaded) {
/*		this computes the total number of glyphs in the font and fills encoding */
		(void) GetGlyphList(hTestDC, lpEncoding, 0);
		ncount = nencoded + nunencode;
	}
/*	need to do this another way in NT and for TT fonts ... */
#ifdef USEUNICODE
	if (bWinNT && !(atmfontflag && bATMLoaded)) {
		if (strcmp(szEncodingVector, "numeric") != 0) /* ??? */
			ncount = AddNamedGlyphs(afmfile, NULL, lpEncoding, hTestDC);
	}
#endif
	if (ncount == 0) {
		failed = 1;					/* 1995/July/19 */
/*		winerror("No Glyphs in Font?"); */
		if (wincancel("No Glyphs in Font?")) {
			goto cleanup;
		}
	}

/*  NOTE: the following does *not* count repeat encoded glyphs */
	sprintf(buffer, "StartCharMetrics %d\n", ncount); 
	fputs(buffer, afmfile);
	ncheck=ncount;
	
/*	now dump out CharMetrics */
/*	new, if we have links into ATM then lets do this right ! */
/*	if (atmfontflag) */							/* potential trap */
	if (atmfontflag && bATMLoaded) {			/* 95/June/28 */
#ifdef DEBUGATM
		if (bDebug > 1) OutputDebugString("Calling DoATMCharMetrics\n");
#endif
		/* first do encoded */
		ncheck = DoATMCharMetrics(afmfile, hTestDC, lpEncoding, 1); 
/*		if (bATMReencoded) UnencodeFont(hTestDC, -1, 0); */
		(void) GetGlyphList(hDC, lpEncoding, 1);
/*		reencode temporarily to get at unencoded glyphs */
		if (bDoUnencodedGlyphs) {					/* then do unencoded */
			hEncodingSaved = hEncoding;				/* save global value */
			nEncodingBytesSaved = nEncodingBytes;	/* save size of data */
			bCustomEncoding = 1;					/* warn about vector */
			hEncoding = CompressEncodingSub(lpEncoding); /* compress for ATM */
/*			hEncoding will be NULL in Windows NT */
			if (hEncoding != NULL) {
				bATMShowRefresh=1;				/* force new encoding setup */
/*				ReencodeFont(hDC, -1, 0); */
				ReencodeFont(hDC, -1, -1);		/* force even if not ANSI */
			}
/* ATM has problem accented characters using breve, dotaccent, hungarumlaut */
/* doesn't seem to help to put these in ASE positions ... */
#ifdef DEBUGATM
			if (bDebug > 1) OutputDebugString("Calling DoATMCharMetrics\n");
#endif
			ncheck += DoATMCharMetrics(afmfile, hTestDC, lpEncoding, 0); 
			if (bATMReencoded) UnencodeFont(hTestDC, -1, 0);
//			FreeVector(hEncoding);			// no, different structure
			hEncoding = GlobalFree(hEncoding);	/* get rid of temporary again */
/* note that at this point hEncoding == NULL ... */
			(void) GetGlyphList(hDC, lpEncoding, 0);
			hEncoding = hEncodingSaved;			/* restore global ENCODING */
			nEncodingBytes = nEncodingBytesSaved;	/* restore size of data */
			bCustomEncoding = 0;					/* back to normal vector */
/*			hEncoding will be NULL in Windows NT */
			if (hEncoding != NULL) {
				bATMShowRefresh=1;			/* force new encoding setup */
				ReencodeFont(hDC, -1, 0); 
			}
		} /* end of doing unencoded glyphs */
		if (ncheck == 0) {
			failed = 1; 	/* maybe just this font ??? */
			winerror("Problem Finding Char BBoxes"); 
/*			if(wincancel("Problem Finding Char BBoxes")) */
			goto cleanup;
		}
	}								/* end of if atmfontflag */
/*	The following is fall back position does not produce good char BBoxes */
/*	Get here for TT fonts or when not linked into ATM (e.g. in NT) */
	else {							/* otherwise do it the old way ... */

#ifdef USEUNICODE
/*	get info on default character so can filter out non chars below */
/*	defaultchar may be uninitialized ??? 98/Mar/23 */
/*		if (bUseNewEncode && bCheckReencode) */
		if (bCheckReencode &&
			((bUseNewEncodeTT && testwantttf) ||
			 (bUseNewEncodeT1 && !testwantttf))) {
			if (bWinNT) {
//				nLenDefault = GetGlyphOutlineW(hTestDC,
//					   notdefUID, GGO_NATIVE, &GMDefault, 0, NULL, &Mat);  // 2000 June 7
				nLenDefault = GetGlyphOutlineW(hTestDC,
					   notdefUID, GGO_METRICS, &GMDefault, 0, NULL, &Mat);
			}
			else {/* above will fail in Windows 95 */ /* or use 127 */
//				nLenDefault = GetGlyphOutlineA(hTestDC,
//				   defaultchar, GGO_NATIVE, &GMDefault, 0, NULL, &Mat); // 2000 June 7
			nLenDefault = GetGlyphOutlineA(hTestDC,
				   defaultchar, GGO_METRICS, &GMDefault, 0, NULL, &Mat);
			}
		}
#endif
		
/*	Step through characters one by one */
/*		for (i = firstchar; i <= lastchar; i++) */ /* NO! 1997/Jan/19 */
		for (i = 0; i < 256; i++) {
/*			check whether name empty ? */
			if (lpEncoding[i] == NULL) continue;
			if (*lpEncoding[i] == '\0') continue;
/*	Try and get better BBox info using GetTextExtent ??? actually worse ... */
/*	For Type 1 fonts get something 3.5 % less than char width ... */
			width = charwidths[i];			/* 1993/June/5 */
			strcpy(str, lpEncoding[i]);
			if (bABCFlag != 0) {	/* for TT fonts */
				xll = abcwidths[i].abcA;
				xur = xll + (int) abcwidths[i].abcB;
				charwidth = xur + abcwidths[i].abcC;
				if (abcwidths[i].abcA == 0 &&
					abcwidths[i].abcB == 0 &&
					abcwidths[i].abcC == 0) continue;	/* never */
/*				how to avoid missing character ??? */
/*				if (metricsize != 1000) charwidth =
					(int) (((long) charwidth * 1000 + 499) / metricsize); */
/*				lets assume we use 1000 */						
			} /* if (bABCFlag != 0) */
			else if (bATMLoaded) {				/* new 94/Dec/31 */
				charwidth = charwidths[i];
/*				(void) GetATMCharBBox (hTestDC, (char) i, 1000); */
				(void) GetATMCharBBox (hTestDC, (WORD) i, 1000);
				xll = (int) (ATMxll >> 16);
				yll = (int) (ATMyll >> 16);
				xur = (int) (ATMxur >> 16);
				yur = (int) (ATMyur >> 16);
			}
			else {					/* for other fonts */
				charwidth = charwidths[i];
				xll = ITALICFUZZ; xur = width - ITALICFUZZ;
				if (xur < 0) xur = 0;
				if (xur < xll) 	xll = 0;
/*	adjust xur for italic angle based on yur ? --- get italic correction */
/*  based on tan (theta) approx theta, and angle approx theta / 56 */
				if (slant != 0) 
				xur += (int) ((long) yur * slant / 560) - ITALICFUZZ;
			}
		
/*	Only attempt to trace TrueType glyph contours if TTUsable and TT font */
			if (bTTUsable && bABCFlag) {	/* 1995/July/8 */
/*				GetTTCharBBox(hTestDC, i); */
/*	Load up the GM glyph metrics structure */
/*				nLen = GetGlyphOutline(hTestDC, i, GGO_METRICS, &GM, */
/*		GetGlyphOutline works in ATM 4.0 in Windows 95 (not just TT) */
#ifdef USEUNICODE
/*				if (bUseNewEncode && bCheckReencode) */
				if (bCheckReencode &&
					((bUseNewEncodeTT && testwantttf) ||
					 (bUseNewEncodeT1 && !testwantttf))) {
					if (bWinNT) {
//						nLen = GetGlyphOutlineW(hTestDC,
//								encodeUID[i], GGO_NATIVE, &GM, 0, NULL, &Mat); // 2000 June 7
						nLen = GetGlyphOutlineW(hTestDC,
								encodeUID[i], GGO_METRICS, &GM, 0, NULL, &Mat);
					}
					else { /* above will fail in Windows 95 */
						if ((i >= 'A' && i <= 'Z') || (i >= 'a' && i <= 'z')) {
//							nLen = GetGlyphOutlineA(hTestDC,
//								i, GGO_NATIVE, &GM, 0, NULL, &Mat);	 //	2000 June 7
							nLen = GetGlyphOutlineA(hTestDC,
								i, GGO_METRICS, &GM, 0, NULL, &Mat);
						}
						else {
//							nLen = GetGlyphOutlineA(hTestDC,
//								defaultchar, GGO_NATIVE, &GMDefault, 0, NULL, &Mat); // 2000 June 7
							nLen = GetGlyphOutlineA(hTestDC,
								defaultchar, GGO_METRICS, &GMDefault, 0, NULL, &Mat);
						}
					}
/* see whether appears to be the default character */
					if (nLen == nLenDefault &&
						GM.gmCellIncX == GMDefault.gmCellIncX &&
						GM.gmBlackBoxX == GMDefault.gmBlackBoxX &&
						GM.gmBlackBoxY == GMDefault.gmBlackBoxY &&
						GM.gmptGlyphOrigin.x == GMDefault.gmptGlyphOrigin.x &&
						GM.gmptGlyphOrigin.y == GMDefault.gmptGlyphOrigin.y) {

						s = lpEncoding[i];
/* heuristic to avoid omitting common default characters */
						if (strcmp(s, "bullet") != 0 &&
							strcmp(s, "space") != 0 &&
							strcmp(s, "period") != 0) {
#ifdef DEBUGCHARMETRICS
						if (bDebug > 1) {
							sprintf(debugstr, "Skipping %d (%04X) %s\n",
									i, encodeUID[i], s);
							OutputDebugString(debugstr);
						}
#endif
						continue;		/* skip this one */
						}
					}
				}
				else {
//					nLen = GetGlyphOutlineA(hTestDC,
//							i, GGO_NATIVE, &GM, 0, NULL, &Mat);	 // 2000 June 7
					nLen = GetGlyphOutlineA(hTestDC,
								i, GGO_METRICS, &GM, 0, NULL, &Mat);
				}
#else
//				nLen = GetGlyphOutline(hTestDC,
//							   i, GGO_NATIVE, &GM, 0, NULL, &Mat);	 // 2000 June 7
				nLen = GetGlyphOutline(hTestDC,
								   i, GGO_METRICS, &GM, 0, NULL, &Mat);
#endif
				if (nLen < 0) continue;	/* avoid bad characters ??? never */
//				if (bDebug > 1) {
//					sprintf(debugstr, "%d nLen %lu\n", i, nLen);
//					OutputDebugString(debugstr);
//				} 
//				DAMN: in W2K for Type 1 fonts, always returns font bounding box!!!
#ifdef DEBUGCHARMETRICS
				if (bDebug > 1) {
					sprintf(debugstr, "%3d %s ret %d Inc %d %d Org %d %d Black %d %d\n",
						i, str, nLen,
						GM.gmCellIncX, GM.gmCellIncY,
						GM.gmptGlyphOrigin.x, GM.gmptGlyphOrigin.y,
						GM.gmBlackBoxX, GM.gmBlackBoxY);
					OutputDebugString(debugstr);
				}
#endif
				charwidth = GM.gmCellIncX;
				xll = GM.gmptGlyphOrigin.x;
				yll = GM.gmptGlyphOrigin.y - GM.gmBlackBoxY;
				xur = GM.gmptGlyphOrigin.x + GM.gmBlackBoxX;
				yur = GM.gmptGlyphOrigin.y;
/*				 */
				if (nLen == 0) {			/* space, nbspace */
					xll = yll = xur =yur = 0;
/*					nLen = 1; */
				} 
			} /* end of bTTUsable && bABCFlag && bWinNT */

/*			bShowHidden for debugging only - maybe eliminate ? */
			if (*str != '\0' || bShowHidden != 0) {
				int ncode = i;
/*				if (bAFMTextFont && *str != '\0') */
				if (bAFMTextFont)
					ncode = standardcode(str);		/* 98/Aug/26 */
				sprintf(buffer, "C %d ; WX %d ; N ", ncode, charwidth);
				fputs(buffer, afmfile);
			}
			if (*str != '\0') {
				sprintf(buffer, "%s ;", str);
				fputs(buffer, afmfile);
			}
			else if (bShowHidden != 0) {
				sprintf(buffer, "a%d ;", i);
				fputs(buffer, afmfile);
			}

/* Unless we have char BBox from ATM or TT, need to *guess* some more */
/* For TT need bABCFlag */
/* For T1 need bABCFlag or ATM loaded */
/*			if (truetypeflag || !bATMLoaded) */
			if ((truetypeflag && !bABCFlag) ||
				(atmfontflag && !bATMLoaded && !bABCFlag))  /* 97/Jan/21 */
/*			if ((truetypeflag && !bABCFlag) ||
				(atmfontflag && !bATMLoaded)) */
			{
				yll = -chardescender; yur = charascender;
				if (i >= 'A' && i <= 'Z') {		/* upper case */
					yll = 0; yur = capheight;
					if (i == 'Q') yll = -(capheight/5);
					if (i == 'J') yll = -(capheight/5);		/* ? */
				}
				else if (i >= '0' && i <= '9') { /* digit */
					yll = 0; yur = capheight;
				}
				else if (i >= 'a' && i <= 'z') { /* lower case */
					yll = 0; yur = xheight;
					if (strchr("bdfhklt", i) != NULL) yur = charascender;
					if (strchr("fgjpqy", i) != NULL) yll =-chardescender;
				}
				if (strcmp(str, "space") == 0) {	/* 1993/June/5 */
					xll = 0; yll = 0; xur = 0; yur = 0;
				}
			}

			if (*str != '\0' || bShowHidden != 0) {
				sprintf(buffer, " B %d %d %d %d ;\n", 
/*					0, chardescender, charwidths[i], charascender); */
						xll, yll, xur, yur);
				fputs(buffer, afmfile);
			}
		} /* for i = firstchar to i = lastchar */
#ifdef USEUNICODE
		if (bWinNT && !(atmfontflag && bATMLoaded)) {
									/* try UNICODE points in Adobe table */
			if (strcmp(szEncodingVector, "numeric") != 0) /* ??? */
				(void) AddNamedGlyphs(afmfile, buffer, lpEncoding, hTestDC);
		}
#endif
	} /* end of doing it the old way ... */

/*	if (bDebug > 1)	OutputDebugString("EndCharMetrics\n"); */
	fputs("EndCharMetrics\n", afmfile);

/*	Loose all kern pairs in reencoded font, so need to unencode it now */
	if (atmfontflag && bATMReencoded) {
		if (bDoReencodeKern) {
			if (bATMReencoded) UnencodeFont(hTestDC, -1, 0);	/* 95/Jan/2 */
/*			need to load up Windows ANSI encoding now again ... */
/*			hEncodingTemp = getencodingsub(lpEncoding, "ansinew"); */
/*			ATM danger here because we reuse hMemEncoding ??? */
			lpEncoding = getencodingsub (lpEncoding, "ansinew"); 
			if (lpEncoding == NULL) {						/* 95/Mar/1 */
				failed = 1;
				winerror("ERROR: Cannot find ansinew.vec");
				goto cleanup;
			}
		}
	}

/*	nkerns = ExtTextMetrics.etmKernPairs; */  	/* debugging only */
/*	goto forceescape; */  			/* debugging only */

/*	Following is mostly done as a test, it's not actually needed */
/*	But now this has no effect since we use GetATMInstanceInfo ... */

	if (bATM4) {			/* force use of GetKerningPairs in ATM 4.0 */
		exttextflag = 0;	/* pretent that ExtendedTextMetrics failed */
		if (bDebug > 1) {
			sprintf(debugstr, "ExtTextMetrics.etmKernPairs %d\n",
				ExtTextMetrics.etmKernPairs);
			OutputDebugString(debugstr);
		}
	}

	nkerns = 0;

	if (hInfo != NULL) {				/* for ATM */
		LPATMKernPair lpKernPair;
		nkerns = lpInfo->num_kern_pairs;
		lpKernPair = (LPATMKernPair) ((LPSTR) lpInfo + lpInfo->kern_pairsOffset);
		qsort(lpKernPair, (unsigned int) nkerns, sizeof(ATMKernPair), comparex);
		if (!bKeepZero) {
			kcount = 0;						/* count non-zero kerns */
			for (i = 0; i < nkerns; i++) {
				if (lpKernPair[i].distance != 0) kcount++;
			}
		}
		else kcount = nkerns;

		if (bDebug > 1) OutputDebugString("Using ATMFontInfo\n");

		fputs("StartKernData\n", afmfile);
		sprintf(buffer, "StartKernPairs %d\n", kcount);
		fputs(buffer, afmfile);
		for (i = 0; i < nkerns; i++) {
			chara = lpKernPair[i].char_1;
//			(void) strcpy(charaname, (lpEncoding + chara * MAXCHARNAME));
			strcpy(charaname, lpEncoding[chara]);
			charb = lpKernPair[i].char_2;
//			(void) strcpy(charbname, (lpEncoding + charb * MAXCHARNAME));
			strcpy(charbname, lpEncoding[charb]);
			kern = lpKernPair[i].distance;
			if (kern == 0 && !bKeepZero) continue;
/*			check whether names are empty */
			if (*charaname != '\0' && *charbname != '\0') 
					sprintf(buffer, "KPX %s %s %d\n",
							charaname, charbname, kern);
			else /* if (bShowHidden != 0) */
				sprintf(buffer, "KPX a%d a%d %d\n", 
					chara, charb, kern);
			fputs(buffer, afmfile);
		}
		fputs("EndKernPairs\n", afmfile);
		fputs("EndKernData\n", afmfile);
		goto endmetrics;					/* skip over the old way */
	}

/*	can't trust the following if exttextflag == 0 */
/*	get Kern Pairs some other way 96/Jan/8 */
	if (exttextflag) nkerns = ExtTextMetrics.etmKernPairs;
	else nkerns = 0;

/*  forceescape: */	 	/* debugging only */

/*	if (bDebug > 1) {
		wsprintf(debugstr, "ExtTextMetrics.etmKernPairs %d\n", nkerns);
		OutputDebugString(debugstr);
	} */  					/* debugging only */

	if (nkerns > 0) {	/* if ExtendedTextMetrics says kern pairs exist */
/*		hKern = LocalAlloc(LHND, nkerns * 4);
		if (hKern == NULL) {
			winerror("Unable to allocate memory");
			goto cleanup;
		}
		lpKernPairs = LocalLock(hKern);
		if (lpKernPairs == NULL) {
			winerror("Unable to allocate memory");
			goto cleanup;
		} */

/*		lpKernPairs = (char *) LocalAlloc(LPTR, nkerns * 4); */
		lpKernPairs = (KERNPAIR *) LocalAlloc(LPTR, nkerns * sizeof(KERNPAIR));
		if (lpKernPairs == NULL) {
			winerror("Unable to allocate memory");
/*			take more drastic action ??? */
			failed = 1;
			goto cleanup;
		}
/*		if (atmfontflag && bATMReencoded)
			UnencodeFont(hTestDC, -1, 0); */		 /* 95/Jan/2 */

		kcount = ExtEscape(hTestDC, GETPAIRKERNTABLE, 0, NULL,
						   nkerns * sizeof(KERNPAIR), (LPSTR) lpKernPairs); 
/*		kcount = Escape(hTestDC, GETPAIRKERNTABLE, 0, NULL, (LPSTR) lpKernPairs); */

		if (kcount < 0) {
			winerror("GETPAIRKERNTABLE Escape failed");	/* 95/Dec/21 */
			kcount = 0;
		}
		if (kcount != nkerns) {
			if (bDebug) {
				wsprintf(debugstr, "nkerns %d kcount %d\n", nkerns, kcount);
				if (bDebug > 1) OutputDebugString(debugstr);
				else winerror(str);
			}
			nkerns = kcount;
		}
		if (!bKeepZero) {
			kcount = 0;						/* count non-zero kerns */
			for (i = 0; i < nkerns; i++) {
/*				kern = (short) *(lpKernPairs + i * 4 + 2); */
/*				lpKernAmount = (LPINT) (lpKernPairs + i * 4 + 2); */
/*				lpKernAmount = (LPSHORT) (lpKernPairs + i * 4 + 2); */
/*				lpKernAmount = (LPSHORT) (lpKernPairs + i * sizeof(KERNPAIR) + 2); */
/*				kern = *lpKernAmount; */
/*				if (kern != 0) kcount++; */
				if (lpKernPairs[i].kpKernAmount != 0) kcount++;
			}
		}
		else kcount = nkerns;	/* list them all */

#ifdef DEBUGATM
		if (bDebug > 1) OutputDebugString("Using GETPAIRKERNTABLE\n");
#endif

		fputs("StartKernData\n", afmfile);
		sprintf(buffer, "StartKernPairs %d\n", kcount);
		fputs(buffer, afmfile);
/*		qsort(lpKernPairs, (unsigned int) nkerns, 4, compare); */
		qsort(lpKernPairs, (unsigned int) nkerns, sizeof(KERNPAIR), compare);

		for (i = 0; i < nkerns; i++) {
/*			chara = (unsigned char) *(lpKernPairs + i * 4); */
/*			chara = (unsigned char) *(lpKernPairs + i * sizeof(KERNPAIR)); */
			chara = LOBYTE(lpKernPairs[i].kpPair);
//			(void) strcpy(charaname, (lpEncoding + chara * MAXCHARNAME));
			strcpy(charaname, lpEncoding[chara]);
/*			charb = (unsigned char) *(lpKernPairs + i * 4 + 1); */
/*			charb = (unsigned char) *(lpKernPairs + i * sizeof(KERNPAIR) + 1); */
			charb = HIBYTE(lpKernPairs[i].kpPair);
//			(void) strcpy(charbname, (lpEncoding + charb * MAXCHARNAME));
			strcpy(charbname, lpEncoding[charb]);
/*			kern = (short) *(lpKernPairs + i * 4 + 2); */
/*			lpKernAmount = (LPINT) (lpKernPairs + i * 4 + 2); */
/*			lpKernAmount = (LPSHORT) (lpKernPairs + i * 4 + 2); */
/*			lpKernAmount = (LPSHORT) (lpKernPairs + i * sizeof(KERNPAIR) + 2); */
/*			kern = *lpKernAmount; */
			kern = lpKernPairs[i].kpKernAmount;
/* check whether non-zero kern */ /* ignore zero kerns ... */
			if (kern == 0 && !bKeepZero) continue;		/* 94/Dec/31 */
/* take care of special case hack old ATM treats 157 as dotlessi */
			if (*charbname == '\0' && charb == 157)
				strcpy(charbname, "dotlessi"); /* 95/Jan/2 */
/* check whether names are empty ? */
			if (*charaname != '\0'  && *charbname != '\0') 
					sprintf(buffer, "KPX %s %s %d\n", 
							charaname, charbname, kern);
			else /* if (bShowHidden != 0) */
				sprintf(buffer, "KPX a%d a%d %d\n", 
					chara, charb, kern);
			fputs(buffer, afmfile);
		} /* for i = 0; i < nkerns, i++ */
		fputs("EndKernPairs\n", afmfile);
		fputs("EndKernData\n", afmfile);
/*		(void) LocalUnlock(hKern); */
/*		hKern = LocalFree(hKern); */
/*		lpKernPairs = (char *) LocalFree(lpKernPairs); */
		lpKernPairs = (KERNPAIR *) LocalFree(lpKernPairs);
	} /* if nkerns > 0 from ExtTextMetrics */
/*	1996/Jan/8 ExtendedTextMetrics failed - try using GetKerningPairs */
	else if (exttextflag == 0) {						/* 1996/Jan/8 */
		DWORD i, kcount, nkerns;
/*		int i, kcount, nkerns; */

/*		LPKERNINGPAIR lpKernPairs=NULL; */	/* Need GlobalAlloc then */
		KERNINGPAIR *lpKernPairs=NULL;

		nkerns = GetKerningPairs (hDC, 0, NULL);
		if (nkerns == 0) {
/*			winerror("GetKerningPairs failed"); */
			if (bDebug > 1) OutputDebugString("No kern pairs?\n");
		}
		if (nkerns > 0) {
			if (bDebug > 1) {
				sprintf(debugstr, "GetKerningPairs %d\n", nkerns);
				OutputDebugString(debugstr);
			}
			lpKernPairs = (KERNINGPAIR *)
/*						 LocalAlloc (LPTR, nkerns * sizeof(KERNPAIR)); */
						 LocalAlloc (LPTR, nkerns * sizeof(KERNINGPAIR));
			if (lpKernPairs == NULL) {
				winerror("Unable to allocate memory");
/*				take more drastic action ??? */
				failed = 1;
				goto cleanup;
			}
			nkerns = GetKerningPairs (hDC, nkerns, lpKernPairs);
			if (!bKeepZero) {
				kcount = 0;						/* count non-zero kerns */
				for (i = 0; i < nkerns; i++) {
					if (lpKernPairs[i].iKernAmount != 0) kcount++;
				}
			}
			else kcount = nkerns;

#ifdef DEBUGATM
			if (bDebug > 1) OutputDebugString("Using GetKerningPairs\n");
#endif

			fputs("StartKernData\n", afmfile);
/*			sprintf(buffer, "StartKernPairs %d\n", ncount); */
			sprintf(buffer, "StartKernPairs %d\n", kcount); /* fix 96/Jul/28 */
			fputs(buffer, afmfile);
/*			qsort(lpKernPairs, (unsigned int) nkerns, 4, compare); */
/*			if we want to sort this, need different compare function */
/*			since the kern pair structure here is different */
			qsort(lpKernPairs, (unsigned int) nkerns, sizeof(KERNINGPAIR), comparez);
			for (i = 0; i < nkerns; i++) {
				chara = (unsigned char) lpKernPairs[i].wFirst; 
//				(void) strcpy(charaname, (lpEncoding + chara * MAXCHARNAME));
				strcpy(charaname, lpEncoding[chara]);
				charb = (unsigned char) lpKernPairs[i].wSecond;
//				(void) strcpy(charbname, (lpEncoding + charb * MAXCHARNAME));
				strcpy(charbname, lpEncoding[charb]);
				kern = lpKernPairs[i].iKernAmount;
/* check whether non-zero kern */ /* ignore zero kerns ... */
				if (kern == 0 && !bKeepZero) continue;		/* 94/Dec/31 */
/* take care of special case hack old ATM treats 157 as dotlessi */
				if (*charbname == '\0' && charb == 157)
					strcpy(charbname, "dotlessi"); /* 95/Jan/2 */
/* check whether names are empty ? */
				if (*charaname != '\0'  && *charbname != '\0') 
					sprintf(buffer, "KPX %s %s %d\n", 
							charaname, charbname, kern);
				else /* if (bShowHidden != 0) */
					sprintf(buffer, "KPX a%d a%d %d\n", chara, charb, kern);
				fputs(buffer, afmfile);
			} /* for i = 0; i < nkerns, i++ */
			fputs("EndKernPairs\n", afmfile);
			fputs("EndKernData\n", afmfile);
			if (lpKernPairs != NULL)  {
				LocalFree(lpKernPairs);
				lpKernPairs = NULL;
			}
		}
	}

endmetrics:										/* 1996/July/28 */
/*	if (bDebug > 1)	OutputDebugString("EndFontMetrics\n"); */
	fputs("EndFontMetrics\n", afmfile);
/*		if (lpMetric != NULL) {
			fprintf(afmfile, "EMSquare %d\n", lpMetric->otmEMSquare);
			fprintf(afmfile, "ItalicAngle %d\n",lpMetric->otmItalicAngle); 
			fprintf(afmfile, "LineGap %d\n", lpMetric->otmLineGap);
			fprintf(afmfile, "XHeight %d\n", lpMetric->otmsXHeight);
			fprintf(afmfile, "CapHeight %d\n", lpMetric->otmsCapEmHeight);
		} */ /* capheight  &  xheight worthless */
/*	or just drop through to after cleanup: */
	fclose(afmfile);
	afmfile = BAD_FILE;
/*		(void) GlobalUnlock (hMemTemp); */ /* redundant */
/*	 */ /* if can create afm file */
/*	 */	/* if (failed == 0) 1993/April/10 */
/*	 */	/* if can get char widths */

cleanup:
	if (afmfile != BAD_FILE) {		/* close output file */
		fclose (afmfile);
		afmfile = BAD_FILE;
	}
	if (hInfo != NULL) {			/* Deallocate ATM font instance info */
		GlobalUnlock(hInfo);
		lpInfo = NULL;
		GlobalFree(hInfo); 
		hInfo = NULL;
	}

	if (bATMReencoded) UnencodeFont(hTestDC, -1, 0);	/* move up here ? */
	if ((UINT) hFontOld > 1)	/* avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hTestDC, hFontOld); /* deselect hFont */
	if ((UINT) hFont > 1)
		(void) DeleteObject(hFont);			/* delete test font */
/*	 */ /* was if hFont != NULL */
	if (hMemTemp != NULL) {
		(void) GlobalUnlock(hMemTemp);
//		FreeVector(hMemTemp);				// ???
		hMemTemp = GlobalFree(hMemTemp);
	}
	if (*fname == '\0') failed = 1;		/* sanity check 95/July/15 */
/*	if (failed == 0) */		/* 1993/April/10 */
	if (batchflag == 0) {				/* in batch mode already done */
		bHourFlag = 0;
		SetCursor(hSaveCursor);					/* 1996/July/25 */
	}

/*	Maybe stop here anyway, even when batchflag is set for safety ? */
	if (failed == 0 && batchflag == 0) {				/* 1994/Feb/8 */
/*		sprintf(str, "Font Name: %s\nFile Name: %s", fname, afmfilename); */
/*		sprintf(str, "Font Name:  %s\nAFM File:  %s\nEncoding:  %s", 
				fname, afmfilename, szEncodingVector); */
		if (*szFontName != '\0') strcpy(str, "Face Nm:\t"); /* ??? 98/Aug/26 */
		else strcpy(str, "Font:\t"); 
		strcat(str, fname);
		strcat(str, "\n");
		if (*szFontName != '\0') { /*	add PS FontName here ? 98/Aug/25 */
			strcat(str, "Font Nm:\t");
			strcat(str, szFontName);
			strcat(str, "\n");
		}
		strcat(str, "AFM File:\t");
		strcat(str, afmfilename);
		strcat(str, "\n");
		if (bWriteAFM == 2) {
			char szTeXFirst[MAXFILENAME];		/* first path on TEXFONTS */
			if (SetupTeXFirst(szTeXFirst, szTeXFonts) < 0) ;	/* can't fail ? */
			strcat(str, "TFM File:\t");
			strcat(str, szTeXFirst);
			strcat(str, "\\");
			strcat(str, removepath(afmfilename));
			if ((s = strrchr(str, '.')) != NULL) strcpy(s+1, "tfm");
			strcat(str, "\n");
		}
		strcat(str, "Encoding: ");
		strcat(str, szEncodingVector);
		{
			char szTFMname[MAXFILENAME];		/* first path on TEXFONTS */
			strcpy(szTFMname, removepath(afmfilename));
			if ((s = strrchr(szTFMname, '.')) != NULL) *s = '\0';
/*			For WriteAFM just show information */		/* 95/July/19 */
/*			For WriteTFM also give cancel option */		/* 95/July/19 */
//			if (bWriteAFM < 2) wininfo(str);			/* 1992/Sep/19 */
			if (bWriteAFM < 2) wininfoTFM(str, szTFMname);	// 2000 July 2
/*			else failed = wincancel(str); */			/* 1995/July/18 */
//			else failed = wincancelinfo(str);			/* 1995/July/18 */
			else failed = wincancelTFM(str, szTFMname);	// 2000 July 2
		}
	}
/*	if (hMetric != NULL) {
		(void) LocalUnlock(hMetric);
		hMetric = LocalFree(hMetric);
	} */
	if (lpMetric != NULL)
		lpMetric = (OUTLINETEXTMETRIC *) LocalFree(lpMetric);

/*	if (trouble) {
		strcpy(debugstr, "Sure you want to write TFM file for ");
		s = debugstr + strlen(debugstr);
		FormatNameStyle (s, TestFont, bCheckBoldFlag, bCheckItalicFlag);
		strcat(debugstr, "?");
		if (wincancel(debugstr))	failed = 1;
	} */
	if (bDebug > 1) {
		if (failed) OutputDebugString("Will not call AFMtoTFM\n");
	}
/*	if (bATMReencoded) UnencodeFont(hTestDC, -1, 0); *//* too late */
	if (failed) return failed;
/*	if (ncheck == 0) return 0; */		/* don't call AFMtoTFM then */
	if (bMakeTFM) {
		int ret;
		ret = CallAFMtoTFM(hWnd, fname, afmfilename, szEncodingVector, batchflag);
/*		winerror("Back in  WriteAFMFile"); */			/* DEBUGGING */
		return ret;
	}
	return 0;
}		

/* returns 1 if file exists, 0 if not, -1 if bad style */
/* segregated out 1995/July/20 - so can attempt to reuse */
/* complaints suppressed if verboseflag == 0 */

int TFMFileExists (HDC hDC, char *TestFont, int verboseflag) {
	char fname[MAXFILENAME]="";		/* 96/Oct/26 */
	char szTeXFirst[MAXFILENAME];		/* first path on TEXFONTS */
	char *s;
	int atmfontflag=0, truetypeflag=0;
/*	int nlen; */
	int err;
	int bFileExists=0;
#ifdef LONGNAMES
	HFILE hfile;
#else
	OFSTRUCT OfStruct;				/* for file exist test */
#endif

	if (SetupTeXFirst(szTeXFirst, szTeXFonts) < 0) return -1;
/*	Pick the first directory out of list of TEXFONTS dirs */
	truetypeflag = testwantttf;	/* initial for FileFromFace 95/Apr/15 */
/*	if (truetypeflag) atmfontflag = 0;	else atmfontflag = 1; */
/*	premature ? we haven't selected font into DC yet ? */
	if (bATMLoaded) atmfontflag = MyATMFontSelected(hDC);  /* hTestDC */
/*	FileFromFace(fname, TestFont, &atmfontflag, &truetypeflag); */
	err = FileFromFace(hDC, fname, TestFont, &atmfontflag, &truetypeflag);
#ifdef DEBUGAFMNAME
	if (bDebug > 1) OutputDebugString(fname);
#endif
	if (err < 0) {							/* should not happen ... */
		winerror("FileFromFace failed");	/* experiment */
		return -1;
	}
	strcpy(str, fname);		/* drop for Dialog Box to pick up */
	if (*str == '\0') {
		strcpy(fname, TestFont);				/* Desperation 96/Oct/26 */
		if (verboseflag) {
/*			strcpy(fname, TestFont); */			/* Desperation */
			BadStyleMessage(debugstr, TestFont, "C");
			winerror(debugstr);
		}
		return -1; 
	}
	StripUnderScores(str);
/*	Here szTeXFirst actually becomes full path and name of TFM file */
	s = szTeXFirst + strlen(szTeXFirst) - 1;
	if (*s != '\\') strcat(szTeXFirst, "\\");
	strcat(szTeXFirst, str);
	strcat(szTeXFirst, ".tfm");
#ifdef LONGNAMES
/*	hfile = _lopen(szTeXFirst, READ); */
/*	hfile = _lopen(szTeXFirst, READ | OF_SHARE_DENY_NONE); */ /* 96/May/18 ? */
	hfile = _lopen(szTeXFirst, READ | OfExistCode);	/* 96/May/18 ? */
	if (hfile == HFILE_ERROR) bFileExists = 0;
	else {
		_lclose(hfile);
		hfile = HFILE_ERROR;
		bFileExists = 1;
	}
#else
	if (OpenFile(szTeXFirst, &OfStruct, OF_EXIST) == HFILE_ERROR)
		bFileExists = 0;
	else bFileExists = 1;
#endif
	if (bFileExists) {
		if (verboseflag) {
			sprintf(debugstr, "WARNING: `%s' exists.\n\nOverwrite it?",
					szTeXFirst);
			if (wincancel(debugstr)) return -1;
		}
	} 
	return bFileExists;
}

/* May want to segregate out so can check if file exists ? 95/Apr/4 */
/* Duplicates some code in WriteAFMFile and WriteTFMFiles ... inefficient */
/* Like checking whether TFM file exists ... */
/* May use quite a bit more stack space ... */ /* 95/Apr/4 */
/* Possible bug in determining atmfontflag ... */

int WriteAFMFileSafe (HWND hWnd, HDC hDC, int bMakeTFM, int batchflag) {
	int bFileExists;
	int ret;

#ifdef DEBUGAFMNAME
	if (bDebug > 1) OutputDebugString("WriteAFMFileSafe\n");
#endif
/*	check first whether this face and style exist 95/July/21 */
	bSyntheticFlag = IsSynthetic (hDC, TestFont,
		  bCheckBoldFlag, bCheckItalicFlag, (testwantttf != 0));
/*  Maybe can't trust `From Outline' in WIN32 ATM 4.0 ? */
/*	In case of font with incomplete set of styles ? */
/*	And with CommFont can't select bad style anyway ? */
/*	if (bSyntheticFlag) {
		if (bDebug > 1) {
			sprintf(debugstr, "STR: %s\tbSyntheticFlag %d\n", str, bSyntheticFlag);
			OutputDebugString(debugstr);
		} 
	} */
/*	if (bSyntheticFlag) {
		if (bDebug > 1) {
			sprintf(debugstr, "%s bold %d italic %d tt %d SYNTHETIC\n",
				TestFont, bCheckBoldFlag, bCheckItalicFlag, testwantttf);
			OutputDebugString(debugstr);
		}
	} */
	if (bSyntheticFlag) {
		bBoldFlag = bCheckBoldFlag;
		bItalicFlag = bCheckItalicFlag;
		strcpy(str, TestFont);					/* Desperation */
/*		strcpy(fname, TestFont); */				/* 96/Oct/12 ? */
		BadStyleMessage(debugstr, TestFont, "D");
		if (wincancel(debugstr) != 0) return -1;		/* FAIL early 95/July/21 */
	}

/*	check whether TFM file exists */
	if (bMakeTFM) {	
/*		if (strcmp(szTeXFonts, "") == 0) */
		if (szTeXFonts == NULL) {
			sprintf(debugstr, "Please set up %s  env var first", "TEXFONTS");
			winerror(debugstr);
			return -1;
		}

/*		old stuff removed here */

		bFileExists = TFMFileExists(hDC, TestFont, 1);	/* new segregated */
		if (bFileExists < 0) return -1;				/* bad stuff happened */
	}
/*	Then actually go and make up the AFM file */
	ret = WriteAFMFile(hWnd, hDC, bMakeTFM, batchflag);
/*	winerror("Back in WriteAFMFileSafe"); */	/* DEBUGGING */
	return ret; 
} 

/* Peculiarities: */
/* ATM does not treat 160 as `space' */
/* ATM treats 173 as hyphen, just like 45, not treated as repeat */
/* Something treats 157 as dotlessi in kern pairs ... */
/* StartCharMetrics count produced by above does not count repeat encoding */

/****************************************************************************

	FUNCTION: EncodingDlg(HWND, unsigned, WORD, LONG)

	PURPOSE: Let user select an encoding file.

****************************************************************************/

/* BOOL CALLBACK EncodingDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) */
BOOL CALLBACK _export EncodingDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int count;
	WORD id;

	switch (message) { 
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details */
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

			case IDOK:
/*				count = GetDlgItemText(hDlg, IPS_EDIT, szEncodingVector, 8+1); */
				count = GetDlgItemText(hDlg, IPS_EDIT, str, sizeof(str));
				if (szEncodingVector != NULL) free(szEncodingVector);
/*				szEncodingVector = zstrdup(str); */			/* 94/Dec/31 */
				szEncodingVector = zstrdup(str);			/* 94/Dec/31 */
				if (count == 0) {					/* shouldn't happen ! */
					(void) MessageBox(hDlg, "Invalid Encoding Vector Name.",
						"Encoding Select", MB_ICONSTOP | MB_OK); 
					return (TRUE);  /* what the hell, terminate */
				}
/*				sprintf(str, "%d", count);	winerror(str); */
				EndDialog(hDlg, count);
				return (TRUE);

			case IDCANCEL:
				EndDialog(hDlg, 0);
				return (TRUE);

			default:
				return (FALSE); /* ? */
			}
/*		break; */ /* ??? */
		
/* end of WM_COMMAND case */		

		case WM_INITDIALOG:						/* message: initialize	*/
			SetDlgItemText(hDlg, IPS_COMMENT,
		        testwantttf ? (LPSTR) "TrueType font" : (LPSTR) "Type 1 font");
/*			if (szEncodingVector != NULL) free (szEncodingVector);
			szEncodingVector = zstrdup(GuessEncoding ()); *//* 94/Dec/31 */
			(void) SendDlgItemMessage(hDlg,
									  IPS_EDIT,
									  EM_LIMITTEXT,
									  8,
									  0L);
			SetDlgItemText(hDlg, IPS_EDIT, (LPSTR) szEncodingVector);
/*	EM_SETSEL in WIN32 wParam starting position, lParam ending position */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details */
			(void) SendDlgItemMessage(hDlg,		/* dialog handle	*/
				IPS_EDIT,						/* where to send message */
				EM_SETSEL,						/* select characters	*/
				0, -1
/*				0, MAKELONG(0, 0x7fff) */
			);									/* select entire contents */
			(void) SetFocus(GetDlgItem(hDlg, IPS_EDIT));
			return (FALSE); /* Indicates we set the focus to a control */

			default:
				return(FALSE); /* ? */
	}
/*	return FALSE; */					/* message not processed */
}	/* lParam unreferenced */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* void GetFaces(HWND); */
void GetFacesSub(HDC);
void SortFaces(void);
void EliminateDuplicates(void);

/* Note that parallel to the globally allocated far array of names are */
/* near array of bytes CharSet[], and PitchAndFamily[], but not IsTTF[] */
/* Remember to call ReleaseFaceNames() after SetupFaceNames() ! */
/* Call with flag = 1 if happy to use old font list if one exists */

// LPSTR SetupFaceNamesSub (HDC hDC, int flag) {
LPSTR *SetupFaceNamesSub (HDC hDC, int flag) {
	if (hFaceNames == NULL) {
		AllocFaceNames();
		flag = 0;					/* need to do GetFaces(hWnd) */
	}
#ifdef DEBUGFONTSELECT
	if (bDebug > 1) OutputDebugString("Enter SetupFaceNamesSub\n");
#endif
	lpFaceNames = GrabFaceNames();
	if (flag == 0) {
		GetFacesSub(hDC);			/* good place ??? */
		SortFaces();				/* 95/Feb/11 ? use qsort ? */
		EliminateDuplicates();		/* 95/Feb/11 ? needed ? */
	}
#ifdef DEBUGFONTSELECT
	if (bDebug > 1) OutputDebugString("Exit SetupFaceNamesSub\n");
#endif
	return lpFaceNames;
}

/* Call with flag = 1 if happy to use old font list if one exists */
/* Need to pair this with ReleaseFaceNames() ? */

// LPSTR SetupFaceNames (HWND hWnd, int flag) {
LPSTR *SetupFaceNames (HWND hWnd, int flag) {
	HDC hDC = GetDC(hWnd);
	if (hDC == NULL) {					/* 1995/July/20 */
		winerror("NULL hDC");
		return NULL;
	}
	SetupFaceNamesSub(hDC, flag);
	(void) ReleaseDC(hWnd, hDC);
#ifdef DEBUGFONTSELECT
	if (bDebug > 1) OutputDebugString("Exit SetupFaceNames\n");
#endif
	return lpFaceNames;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* How can we prevent listing fonts whose names start with @ ??? */

/* This should catch selection changes and adjust encoding flag ??? */

BOOL bOldCheckReencode;		/* old encoding state 96/Jul/21 INIT_DIALOG */

BOOL bEncodeCheckDisable;	/* on if check box disabled */

/* typedef UINT (CALLBACK *LPOFNHOOKPROC) (HWND, UINT, WPARAM, LPARAM); */

UINT APIENTRY ChooseFontHook (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
/* UINT CALLBACK _export ChooseFontHook (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) */
{
	WORD id, cmd;
	HWND hwnd;
	LOGFONT lf;
	int rasterflag, decorativeflag, dontcareflag, symbolflag, textflag;
	int testwantttf, testcharset, testpitchandfamily;
	int EncodeFlag;

	switch (msg) {
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			cmd = (WORD) GET_WM_COMMAND_CMD(wParam, lParam),
			hwnd = (HWND) GET_WM_COMMAND_HWND(wParam, lParam);

			switch (id) {
				case IDOK:
/*					time to read out our own added flags */
					EncodeFlag = (int) SendDlgItemMessage(hDlg, 
						ID_ENCODED, BM_GETCHECK, 0, 0L);
/*					if `Reencoded' check box change need to redo */
/*					if (bCheckReencode != EncodeFlag) bATMShowRefresh = 1; */
					if (bOldCheckReencode != EncodeFlag) bATMShowRefresh = 1;
					bCheckReencode = EncodeFlag;
					if (! bShowWidths)
						bShowBoxes = (int) SendDlgItemMessage(hDlg, 
							ID_SHOWBOXES, BM_GETCHECK, 0, 0L);
					if (bWriteAFM) 
						bUseFontName = (int) SendDlgItemMessage(hDlg, 
							ID_USEFONTNAME, BM_GETCHECK, 0, 0L);
				break;

				case cmb1:
/* combo box 1 is the one with the list of Windows Face names */
#ifdef DEBUGFONTCHECK
					if (bDebug > 1) {
						sprintf(debugstr, "%s id %04X cmd %04X wnd %04X\n",
								"CMB1",id, cmd, hwnd);
						OutputDebugString(debugstr);
					}
#endif
/* Don't bother if the check box has been disabled */
					if (bEncodeCheckDisable) break;
/* we can't use WM_CHOOSEFONT_GETLOGFONT right here, because we get old font */
/* so instead we post a message to the hidden APPLY button ... */
/* cmd 3 gain focus, 4 kill focus, 9 is lose selection, 1 is gain selection */
/* when starting up we get a 3, when selecting a new one we get a 1 */
/* we could do this work for all codes, although that would be wasteful */
					if (cmd != 3 && cmd != 1) break;
					PostMessage(hDlg, WM_COMMAND, psh3, cmd);
				break;

/* This has also been tested in WIN16 version running in Windows 95 ... */
				case psh3:
					SendMessage(hDlg, WM_CHOOSEFONT_GETLOGFONT,
									0, (LPARAM) (LONG) &lf);
#ifdef DEBUGFONTCHECK
					if (bDebug > 1) {
						sprintf(debugstr, "%s id %04X cmd %04X wnd %04X\n",
								"PSH3", id, cmd, hwnd);
						OutputDebugString(debugstr);
						sprintf(debugstr, "%s CharSet %04X PaF %04X Prec %04X\n",
								lf.lfFaceName, lf.lfCharSet,
								lf.lfPitchAndFamily, lf.lfOutPrecision);
						OutputDebugString(debugstr);
					}
#endif
/*	Don't bother if the check box has been disabled */
					if (bEncodeCheckDisable) break;
/*					if (hwnd == 3 || hwnd == 1) */

/*	Try and figure out whether this font should be reencoded or not ! */
/*	TT gives 3 here, T1 gives 2 (at least in Windows 95) */
/*	TT and T1 give 3 here in Windows NT fron WIN16 */
/*	Raster fonts like Fixedsys give 1 */
					if (lf.lfOutPrecision == 1) rasterflag = 1;
					else rasterflag = 0;
					if (lf.lfOutPrecision == OUT_STROKE_PRECIS)
						testwantttf = 1;	/* truetypeflag = 1; */
					else testwantttf = 0;	/* truetypeflag = 0; */

					testcharset = lf.lfCharSet;
					testpitchandfamily = lf.lfPitchAndFamily;

					if (testcharset == ANSI_CHARSET) symbolflag = 0;
					else if (testcharset == DEFAULT_CHARSET) symbolflag = 0;
					else symbolflag = 1;	/* Symbol or OEM or whatever */

/*	catch non text fonts if possible (except fixed width */
					if ((testpitchandfamily & 0xF0) == FF_DECORATIVE)
						decorativeflag = 1;
					else decorativeflag = 0;
/*  an experiment to deal with fixed width fonts auto converted in NT */
/*	CMTT*, CMTEX*, CMITT10, CMSLTT10 when converted have FF_DONTCARE */
					if ((testpitchandfamily & 0xF0) == FF_DONTCARE)
						dontcareflag = 1;
					else dontcareflag = 0;

					textflag = 1;
					if (symbolflag) textflag = 0;
/*	if Type 1 and decorative it sure is not a text font */
					if (!testwantttf && decorativeflag) textflag = 0;
/*	revised 97/Feb/5 to deal with auto converted math fonts */
/*	this means some decorative TT text fonts will not be reencoded ... */
					if (bDecorative) {	/* treat decorative TT font non-text */
						if (testwantttf && decorativeflag) textflag = 0; 
					}
/*	catch auto converted fixed width non text fonts CMTT10 */
					if (bDontCare) {
						if (testwantttf && dontcareflag) textflag = 0;
					}
#ifdef USEUNICODE
/*	can't reencode type 1 fonts without hEncoding or bUseNewEncodeT1 */
					if (!testwantttf && !bUseNewEncodeT1 &&
						(hEncoding == NULL)) textflag = 0;
/*	can't reencode truetype fonts without bUseNewEncodeTT */
					if (testwantttf && !bUseNewEncodeTT) textflag = 0;
#else
					if (!testwantttf && (hEncoding == NULL)) textflag = 0;
					if (testwantttf) textflag = 0;
#endif
					if (rasterflag) textflag = 0;	/* override ! */
/*	deal with NT crock that Courier and Symbol are listed as RASTER fonts */
					if (strcmp(lf.lfFaceName, "Courier") == 0)	textflag = 1;
/*  an experiment to deal with fixed width fonts auto converted in NT */
/*	CMTT*, CMTEX*, CMITT10, CMSLTT10 when converted have FF_DONTCARE */
					if (testwantttf) {
						if ((testpitchandfamily & 0xF0) == FF_DONTCARE)
								textflag = 0;
					}
#ifdef DEBUGFONTCHECK
					if (bDebug > 1) {
						sprintf(debugstr,
								"%s TT %d Ras %d Sym %d Dec %d Text %d\n",
								lf.lfFaceName, testwantttf,
								rasterflag, symbolflag,
								decorativeflag, textflag);
						OutputDebugString(debugstr);
					}
#endif
					(void) SendDlgItemMessage(hDlg, ID_ENCODED,
							BM_SETCHECK, (WPARAM) textflag, 0L);
/*					} */
				break;

				default: return (FALSE);
			}
		break;									/* end of WM_COMMAND case */
			
		case WM_INITDIALOG:						/* message: initialize	*/
			bOldCheckReencode = bCheckReencode;		/* 96/Jul/21 save old */
			if (hEncoding == NULL) {
				if (bDebug > 1) OutputDebugString ("hEncoding == NULL\n");
			}
			if (hEncoding != NULL) bCheckReencode = 1;		/* 95/Jan/4 */
#ifdef USEUNICODE
			if (bUseNewEncodeTT) bCheckReencode = 1;		/* 97/Jan/17 */
			if (bUseNewEncodeT1) bCheckReencode = 1;		/* 97/Jan/17 */
#endif
/*			set out our own added checkboxes */
			(void) SendDlgItemMessage(hDlg, ID_ENCODED, BM_SETCHECK, 
				(WPARAM) bCheckReencode, 0L);
			SetDlgItemText(hDlg, ID_ENCONAME,
				   (bCheckReencode && (szReencodingName != NULL)) ?
						   (LPSTR) szReencodingName : (LPSTR) "");
/*			we can reencode if bUseNewEncode *or* ATM all set up for it */
			bEncodeCheckDisable=0;
#ifdef USEUNICODE
/*			if ((!bATMLoaded || (hEncoding == NULL)) && bUseNewEncodeT1 == 0)*/
			if (!bUseNewEncodeTT && !bUseNewEncodeT1 &&  
				(!bATMLoaded || (hEncoding == NULL))) bEncodeCheckDisable=1;
#else
			if (!bATMLoaded || (hEncoding == NULL)) bEncodeCheckDisable=1;
#endif
			if (bEncodeCheckDisable) {
				(void) SendDlgItemMessage(hDlg, ID_ENCODED, BM_SETCHECK, 
					0, 0L);
				(void) EnableWindow(GetDlgItem(hDlg, ID_ENCODED), FALSE);
			}
/*			can customize sample text string */
/*			SetDlgItemText(hDlg, stc5, "ABCDabcd"); */
/*			Hide a bunch of controls */
/*			ShowWindow(GetDlgItem(hDlg, stc6), SW_HIDE); */ /* ??? */
			ShowWindow(GetDlgItem(hDlg, stc7), SW_HIDE);	/* Script: */
			ShowWindow(GetDlgItem(hDlg, cmb5), SW_HIDE);	/* combo box */
			if (! bShowWidths && ! bWriteAFM) {
				(void) SendDlgItemMessage(hDlg, ID_SHOWBOXES, BM_SETCHECK, 
										  (WPARAM) bShowBoxes, 0L);
			}
			else {
				(void) ShowWindow(GetDlgItem(hDlg, ID_SHOWBOXES), SW_HIDE);
			}
			if (! bWriteAFM) {
				(void) ShowWindow(GetDlgItem(hDlg, ID_USEFONTNAME), SW_HIDE);
				(void) ShowWindow(GetDlgItem(hDlg, ID_USEFILENAME), SW_HIDE);
				(void) ShowWindow(GetDlgItem(hDlg, ID_NAMEGROUP), SW_HIDE);
			}
			else {
				(void) SendDlgItemMessage(hDlg, ID_USEFONTNAME, BM_SETCHECK, 
										  (WPARAM) bUseFontName, 0L);
				(void) SendDlgItemMessage(hDlg, ID_USEFILENAME, BM_SETCHECK, 
										  (WPARAM) ! bUseFontName, 0L);
			}

			return (TRUE); /* Indicates the focus is *not* set to a control */
/*		break; */

		default: return(FALSE);

	}
	return (FALSE); 	/* let Dialog Box Proc process message */
/*	return (TRUE);	*/	/* processed message - don't process again */
}

/* WIN16: if we use our template we get HWND == 0 warning in DebugWin ? */
/* every time we change font, style, or size selection. Why ? */
/* This is not dependent on whether we use our own hook function */

/* does this set up testcharset and testpitchandfamily ??? */
/* as GetFontSelectionOld does ? yes, but see CF_NOSCRITPSEL below */

/* Following uses common dialog box in Windows 3.1 / Windows 95 */

int GetFontSelectionNew (HWND hWnd, int bShowWidthsOld, int skipflag) {	/* 95/Nov/25 */
	LOGFONT lf;
	CHOOSEFONT cf;
	HDC hDC=NULL;
	BOOL bUsePrintDC = 0;			/* hard wired not to use PrinterDC ... */
	int flag=0;
/*	char szStyle[LF_FACESIZE]="Regular"; */		/* if used ... */

/*	if (bDebug > 1) OutputDebugString("GetFontSelectionNEW\n"); */

/*	initialize LOGFONT lf structure for initial selection */
/*	set CD_INITTOLOGFONTSTRUCT flag in that case */

/*	if (bUsePrintDC) {
		hDC = GetDC(hWnd);
		cf.hDC = hDC;
		cf.Flags |= CF_PRINTERFONTS;
	} */ 					/* only if CF_PRINTERFONTS flag */
	memset(&lf, 0, sizeof(LOGFONT));
/*	lf.lfHeight = - (int) TestSize / 10; */	/* logical units TWIPS ??? */
	lf.lfHeight = - (int) TestSize / 12; 	/* logical units TWIPS ? 99/Jan/3*/
	lf.lfWidth = 0;
	if (bCheckBoldFlag) lf.lfWeight = 700;
	else lf.lfWeight = 400;
	if (bCheckItalicFlag) lf.lfItalic = 1;
/*	lf.lfCharSet = DEFAULT_CHARSET; */		/* 1 ? */
/*	lf.lfPitchAndFamily = 0; */
	strcpy(lf.lfFaceName, TestFont);	

	memset(&cf, 0, sizeof(CHOOSEFONT));
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hWnd;
	if (bUsePrintDC) {
		hDC = GetDC(hWnd);
		cf.hDC = hDC;
		cf.Flags |= CF_PRINTERFONTS;
	}  					/* but *only* if CF_PRINTERFONTS flag */
/*	if (bCheckBoldFlag) cf.nFontType += BOLD_FONTTYPE; */
	if (bCheckBoldFlag) cf.nFontType |= BOLD_FONTTYPE;
/*	if (bCheckItalicFlag) cf.nFontType += ITALIC_FONTTYPE; */
	if (bCheckItalicFlag) cf.nFontType |= ITALIC_FONTTYPE;
	if (!bCheckItalicFlag && !bCheckBoldFlag) cf.nFontType |= REGULAR_FONTTYPE;
/*	Actually seems like nFontType has those bits added in for callback */
/*	Suspect they do not matter at all in this call ... */	
	cf.lpLogFont = &lf;
	cf.Flags |= CF_INITTOLOGFONTSTRUCT;	/* use LOGFONT struct to init */
/*	cf.lpszStyle = szStyle; */			/* if style specified */
/*	cf.Flags |= CF_USESTYLE; */			/* if we want style */
	cf.Flags |= CF_SCREENFONTS;
/*	cf.Flags |= CF_PRINTERFONTS; */		/* exp 97/Jan/18 ??? */
	cf.Flags |= CF_NOSIMULATIONS;		/* show only existing styles */
/*	In Windows NT ATM beta, Symbol and Courier are RASTER_FONTTYPE FIX ??? */
/*	cf.Flags |= CF_SCALABLEONLY; */		/* ??? */
	if (!bWinNT)						/* REMOVE AGAIN 97/Jan/18 ??? */
		cf.Flags |= CF_SCALABLEONLY;	/* ??? */
/*	with above commented out, get Fixedsys, MS Sans Serif, MS Serif, System */
	cf.Flags |= CF_FORCEFONTEXIST;		/* can't select non-existent style */
	cf.Flags |= CF_NOVECTORFONTS;		/* don't allow Modern */
// #if (WINVER >= 0x0400)				default in 32 bit compiler
	if ((bNoScriptSel & 1) != 0) cf.Flags |= CF_NOSCRIPTSEL;
/*	This flag disables the Script Combo box, */	/* changed 96/Oct/26 */
/*	BUT also forces charset to be set to DEFAULT_CHARSET (i.e. 1) */
/*	Which we do *not* want, since we and testcharset for GuessEncoding */
/*	NOTE: this make cause GPFs under bizarre circumstances, perhaps */
/*	having to do with stc7 and cmb5 in dviwindo.dlg for FORMATDLGORD31 */
/*	being NOT WS_VISIBLE */
// #endif
/*	cf.rgbColors = RGB(0, 255, 255); */ /* only for CF_EFFECTS */
	cf.nFontType = SCREEN_FONTTYPE;		/* overwrite bits set above ??? */
/*	cf.iPointSize = (TestSize+1) / 2; */	/* no effect (output only) */
/*	cy = GetDeviceCaps(hDC, LOGPIXELSY);	Height = -(iPointSize*cy)/72; */

	cf.hInstance = hInst;
/*	cf.lpfnHook = ChooseFontHook; */	/* _export - different attributes */
	cf.lpfnHook = (LPOFNHOOKPROC) ChooseFontHook;	/* 95/Dec/26 */
	cf.Flags |= CF_ENABLEHOOK;
/*	We get a Invalid HWND: 0000 error if we use our own template */
/*	Provide for suppression of our own template for ChooseFont 96/Oct/28 */
	if ((bNoScriptSel & 2) == 0) {
		cf.lpTemplateName = MAKEINTRESOURCE(FORMATDLGORD31);
		cf.Flags |= CF_ENABLETEMPLATE;
	}

	if (skipflag == 0) 
		flag = ChooseFont(&cf);				/* actually go do it ! */
	else flag = 1;

	if (hDC != NULL) (void) ReleaseDC(hWnd, hDC);
	if (flag == 0) {
/*		winerror("No Font selected"); */
		if (bDebug > 1) OutputDebugString("No Font selected\n");
		return 0;
	}
/*	if size changed in font selection dialog  iPointSize is in 1/10 pt */
/*	don't use since this may be cell height, not character height */
/*	if (TestSize != cf.iPointSize * 2) {
		if (bDebug > 1) {
			sprintf(debugstr, "TestSize %d cf.iPointSize * 2 %d",
					TestSize, cf.iPointSize * 2);
			OutputDebugString(debugstr);
		}
	}
	TestSize = cf.iPointSize * 2; */			/* 1/10 pt to TWIPS */
	TestSize = - lf.lfHeight * 12;				/* 99/Jan/3 */

/*	cy = GetDeviceCaps (hDC, LOGPIXELSY);
	Height = -(cf.iPointSize * cy) / 720; */
	if (bATMLoaded) {				/* 97/Jan/21 don't bother otherwise */
/*	New code for setting bATMShowRefresh 96/July/21 */
/*	bATMShowRefresh = 0; */
/*	if (bCheckBoldFlag != (cf.nFontType & BOLD_FONTTYPE)) bATMShowRefresh = 1; */
	if (bCheckBoldFlag && !(cf.nFontType & BOLD_FONTTYPE)) bATMShowRefresh = 1;
	if (!bCheckBoldFlag && (cf.nFontType & BOLD_FONTTYPE)) bATMShowRefresh = 1;
/*	if (bCheckItalicFlag != (cf.nFontType & ITALIC_FONTTYPE)) bATMShowRefresh = 1; */
	if (bCheckItalicFlag && !(cf.nFontType & ITALIC_FONTTYPE)) bATMShowRefresh = 1;
	if (!bCheckItalicFlag && (cf.nFontType & ITALIC_FONTTYPE)) bATMShowRefresh = 1;
	if (strcmp(TestFont, lf.lfFaceName) != 0) bATMShowRefresh = 1;
#ifdef DEBUGATM
	if (bDebug > 1) {
		wsprintf(debugstr, "bATMShowRefresh %d\n", bATMShowRefresh);
		OutputDebugString(debugstr);
	}
#endif
/*	bATMShowRefresh = 1; */					/* DEBUGGING TEST ONLY */
	}

/*	bCheckBoldFlag = cf.nFontType & BOLD_FONTTYPE; */
	if (cf.nFontType & BOLD_FONTTYPE) bCheckBoldFlag = 1;
	else bCheckBoldFlag = 0;
/*	bCheckItalicFlag = cf.nFontType & ITALIC_FONTTYPE; */
	if (cf.nFontType & ITALIC_FONTTYPE) bCheckItalicFlag = 1;
	else bCheckItalicFlag = 0;
	if (cf.nFontType & REGULAR_FONTTYPE) bCheckBoldFlag = bCheckItalicFlag = 0;
	strcpy(TestFont, lf.lfFaceName);
	testcharset = lf.lfCharSet;		/* always 1 if CF_NOSCRIPTSEL set */
	testpitchandfamily = lf.lfPitchAndFamily;
#ifdef DEBUGWRITEAFM
	if (bDebug > 1) {
		sprintf(debugstr, "TestFont %s charset %X pitchandfamily %X (%s)\n",
				TestFont, testcharset, testpitchandfamily, "LOGFONT");
		OutputDebugString(debugstr);
	}
#endif
/*	TrueType => TRUETYPE_FONTTYPE */
/*	Type 1 => DEVICE_FONTTYPE in Windows 95, Type 1 => 0 in Windows NT */
	if (cf.nFontType & TRUETYPE_FONTTYPE) testwantttf = 1;
	else testwantttf = 0;
/*	Could also check tmDigitizedAspectX == 1001 in TEXTMETRIC for T1 in NT */
#ifdef DEBUGFONTSELECT
	{			/*	show result	*/
/*		sprintf(str, "Size %d Style %s (%s%s)", cf.iPointSize, szStyle, */
		sprintf(debugstr,
"Face %s,%s%s%s %s Size %d.%d (CharSet %X PitchAndFamily %X Type %X)\n",
			TestFont,
			(!bCheckBoldFlag && !bCheckItalicFlag) ? "REGULAR" : "",
			bCheckBoldFlag ? "BOLD" : "",
			bCheckItalicFlag ? "ITALIC" : "",
			testwantttf ? "(TT)" : "(T1)",
			cf.iPointSize / 10, cf.iPointSize % 10,
			lf.lfCharSet, lf.lfPitchAndFamily,
/*			testcharset, testpitchandfamily, */
			cf.nFontType);
		if (bDebug > 1) OutputDebugString(debugstr);
		else if (bDebug) wininfo(debugstr);
	}
#endif
	bSyntheticFlag = 0;			/* this can't return synthetic fonts */
/*	copied from below ??? */
	if (bResetScale != 0) resetmapping(hWnd, 1);	/* new */
	if (bShowWidthsOld != 0) SetFontState(-1);	/* show widths */
	else SetFontState(+1);						/* show char table */
	checkfontmenu();
	checkcontrols(hWnd, 0);			/* new */
/*	return 0; */ /* for now */
	return flag;
/*	result is in LOGFONT lf */
/*	copied to TestFont, TestSize, bCheckBoldFlag, bCheckItalicFlag */
}

/* CF_SCREENFONTS */
/* CF_PRINTERFONTS => need hDC */
/* CF_BOTH => CF_SCREENFONTS | CF_PRINTERFONTS */
/* CF_SCALABLEONLY */ /* YES ? */
/* CF_ANSIONLY => no symbol fonts */
/* CF_TTONLY => TrueType only */
/* CF_FIXEDPITCHONLY => monospace only */
/* CF_NOVECTORFONTS == CF_NOEMFONTS */
/* CF_NOSIMULATIONS */ /* YES ? */
/* CF_EFFECTS => lfStrikeOut, lfUnderline, rgbColors */ /* NO */
/* CF_ENABLEHOOK => lpfnHook enable */
/* CF_ENABLETEMPLATE => hInstance, use lpTemplateName */
/* CF_ENABLETEMPLATEHANDLE => hInstance, ignore lpTemplateName */
/* CF_FORCEFONTEXIST => error if non-existent font selected *//* YES ? */
/* CF_INITTOLOGFONTSTRUCT => use lpLogFont to initialize */
/* CF_LIMITSIZE => limit size selection nSizeMin, nSizeMax */
/* CF_NOFACESEL *//* CF_NOSIZESEL *//* CF_NOSTYLESEL */
/* CF_APPLY */ /* CF_SHOWHELP */
/* CF_USESTYLE => lpszStyle specifies initial style */
/* CF_WYSIWYG => only fonts available both screen & printer */
/*			     need CF_BOTH & CF_SCALABLEONLY in this case */
/* lpfnHook <= CF_ENABLEHOOK */
/* lpTemplateName <= CF_ENABLETEMPLATE */
/* hInstance <= CF_ENABLETEMPLATE or CF_ENABLETEMPLATEHANDLE */
/* lpszStyle => buffer for style (LF_FACESIZE long) */
/* nFontType => BOLD_FONTTYPE ITALIC_FONTTYPE  REGULAR_FONTTYPE */
/*			    PRINTER_FONTTYPE SCREEN_FONTTYPE SIMULATED_FONTTYPE */
/* nSizeMin nSizeMax */
/* if(WINVER >= 0x0400) */
/* CF_SELECTSCRIPT */
/* CF_NOSCRIPTSEL */
/* CF_NOVERTFONTS */

int GetFontSelectionOld (HWND hWnd, int bShowWidthsOld, int skipflag) {
	int flag=0;
	char *s;
	HDC hDC;						/* new 95/July/8 */
//	LPSTR lpCurrentName;

/*	if (bDebug > 1) OutputDebugString("GetFontSelectionOLD\n"); */

	if (lpFaceNames == NULL) return (FALSE);	/* ??? */

/*	if (hFaceNames == NULL) AllocFaceNames();
	lpFaceNames = GrabFaceNames(); 
	GetFaces(hWnd);
	SortFaces();
	EliminateDuplicates(); */
	SetupFaceNames(hWnd, 0);			/* use common code */

/*	if (DialogBox(hInst, "SelectFont", hWnd, lpFontDlg)) */
/*	if (DialogBox(hInst, "SelectFont", hWnd, FontDlg)) */
/*	PreviousFont = CurrentFace; */	/* save up last font selected 94/Dec/31 */
	if (CurrentFace >= nFaceIndex) CurrentFace = 0;	/* sanity 95/Feb/11 */

//	if (DialogBox(hInst, "SelectFont", hWnd, (DLGPROC) FontDlg)) 
	if (DialogBox(hInst, "SelectFont", hWnd, FontDlg)) {
//		lpCurrentName = lpFaceNames + CurrentFace * MAXFACENAME;
//		lpCurrentName = lpFaceNames[CurrentFace];
//		if (strlen(lpCurrentName) >= MAXFACENAME)
//			winerror("Font Name too long");
//		else
		strcpy(TestFont, lpFaceNames[CurrentFace]);  /* NEW */
/*		to get rid of any possible added debugging info from list box: */
		if ((s = strchr(TestFont, '(')) != NULL) {
			testwantttf = 1;
			*(s-1) = '\0';
/*			would like TrueType font - use flag in createatmfont */
		}
		else testwantttf = 0;

		hDC = GetDC(hWnd);						/* 95/July/8 */
		if (hDC == NULL) winerror("NULL hDC");
/*		testwantttf = IsTTF[CurrentFace]; */ /* or use this ? */
/*		if (bDebug > 1) OutputDebugString("GetFontSelection\n"); */
		bSyntheticFlag = 
/*		IsSynthetic(TestFont, bCheckBoldFlag, bCheckItalicFlag);*/
/*		IsSynthetic(TestFont, bCheckBoldFlag, bCheckItalicFlag, testwantttf);*/
		IsSynthetic (hDC, TestFont, bCheckBoldFlag, bCheckItalicFlag,
					 (testwantttf != 0));
		(void) ReleaseDC(hWnd, hDC);					/* 95/July/8 */

/*	    if (bCheckBoldFlag == 0 && bCheckItalicFlag == 0)
			 bSyntheticFlag = 0; */ /* treat base case as if it exists ??? */
/*		an experiment 95/March/9 */
		testcharset = CharSet[CurrentFace];
		testpitchandfamily = PitchAndFamily[CurrentFace];
		if (testwantttf != 0) {
/*			sprintf(str, "charset %d pitchandfamily %d",
				testcharset, testpitchandfamily);
			winerror(str); */
/*				testpitchandfamily == VARIABLE_PITCH | FF_DONTCARE) */
			if (testcharset == SYMBOL_CHARSET && 
				testpitchandfamily == (VARIABLE_PITCH | FF_DONTCARE)) /* ? */
					testwantttf = -1;	/* used anywhere ? */
		}
/*			sprintf(str, "Weight %d Index %d", 
				Weight[CurrentFace], CurrentFace); 
				winerror(str); */
/*			if (Weight[CurrentFace] == 0 ||
					Weight[CurrentFace] == 400) bBoldFlag = 0;
				else if (Weight[CurrentFace] > 400) bBoldFlag = +1;
				else if (Weight[CurrentFace] < 400) bBoldFlag = -1;
				italicflag = Italic[CurrentFace]; */
		if (bResetScale != 0) resetmapping(hWnd, 1);	/* new */
/*			} */
		if (bShowWidthsOld != 0) SetFontState(-1);	/* show widths */
		else SetFontState(+1);						/* show char table */
		checkfontmenu();
		checkcontrols(hWnd, 0);			/* new */
/*		if (bShowWidths == 0) ExposeScrollBars(hWnd);
		else HideScrollBars(hWnd);		*/ /* ??? */
		flag =1;					/* user did not cancel selection */
	}								/* new place */
/*	winerror("ReleaseFaceNames - GetFontSelection"); */	/* DEBUGGING */
	ReleaseFaceNames();	 
	return flag;
}

/* Called from DVIWindo --- returns zero if user cancels font selection */

int GetFontSelection (HWND hWnd, int bShowWidthsOld, int skipflag) {
	int flag;
/*	char *s; */
/*	HDC hDC; */						/* new 95/July/8 */

	if (bCommFont) flag = GetFontSelectionNew(hWnd, bShowWidthsOld, skipflag);
	else flag = GetFontSelectionOld(hWnd, bShowWidthsOld, skipflag);
/*	flag = GetFontSelectionOld(hWnd, bShowWidthsOld); */
	return flag;
}

/* void FixedSizes(void); */

/* The following is no longer used: */

/****************************************************************************

	FUNCTION: GetSizes(hWnd, CurrentFace)

	PURPOSE: Get size of current font

****************************************************************************/

/****************************************************************************
Module name: Spin.H
Programmer : Jeffrey M. Richter.
*****************************************************************************/

/* Spin Button doesn't send WM_CTLCOLOR message to parent window. */

/* remember to unregister when leaving: WM_DESTROY in main window function */
/*    if (bSpinOK != 0) UnregisterClass(szControlName, hInst); */

/* static BOOL NEAR PASCAL RegisterControlClass (HINSTANCE hInst); */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/****************************************************************************

	FUNCTION: FontDlg(HWND, unsigned, WORD, LONG)

	PURPOSE: Let user select a Font.

****************************************************************************/

/*BOOL CALLBACK FontDlg(HWND hDlg, unsigned message, WORD wParam, LONG lParam) */
BOOL CALLBACK _export FontDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int i;
	int index, inx;
/*	char buf[LF_FACESIZE]; */
	HWND hSpin=NULL;
	int xll, yll, xur, yur;
	RECT SpinRect;
	POINT ULCorner, LRCorner;
	int CreateParams[3];		/* 3 parameters to pass to spinner */
	int translated;				/* flag for valid integer conversion */
	int low = 1, high = 999;	/* 144 */
	int spinvalue;
	WORD id, cmd;
	HWND wnd;
/*	int flag; */					/* 94/Dec/31 */
	int BoldFlag, ItalicFlag, EncodeFlag;	/* 95/Mar/12 */

	if (lpFaceNames == NULL) return (FALSE);	/* serious bug */

	switch (message) {
		case WM_INITDIALOG:

/*			if (bSpinOK != 0 && bShowWidths == 0) */
			if (bShowWidths == 0) {		/* reorganized 93/Sep/12 */
				spinvalue = (int) ((TestSize + 9)/20); 
				if (bSpinOK != 0) {
/* position spinner relative to ID_SPINVALUE control */
				GetWindowRect(GetDlgItem(hDlg, ID_SPINVALUE), &SpinRect);
				ULCorner.x = SpinRect.left; ULCorner.y = SpinRect.top;
				LRCorner.x = SpinRect.right; LRCorner.y = SpinRect.bottom;
				ScreenToClient(hDlg, &ULCorner); 
				ScreenToClient(hDlg, &LRCorner);
/* rectangle of edit control ID_SPINVALUE: */
				SpinRect.left = ULCorner.x; SpinRect.top =  ULCorner.y;
				SpinRect.right = LRCorner.x; SpinRect.bottom = LRCorner.y;
/* rectangle of spin button ID_SPIN: */
				SpinRect.left = SpinRect.right; 
				SpinRect.right = SpinRect.left + 
					(SpinRect.bottom - SpinRect.top)/2;
				xll = SpinRect.left; yll = SpinRect.bottom;
				xur = SpinRect.right; yur = SpinRect.top;
/* pass parameters in to spin control as it is being created */
/*				spinvalue = (int) ((TestSize + 9)/20); */
				CreateParams[0] = low;
				CreateParams[1] = spinvalue;
				CreateParams[2] = high;
/* This may not work if this is the second instance of DVIWindo, and first */
/* instance had to rename class, or make it task specific rather than global */
				hSpin = CreateWindow(szControlName, 
/*					"Spin Window",	*/						/* window name */
					"DVI Spin Window",						/* window name */
/*					WS_CHILD | WS_VISIBLE | WS_BORDER,*/	/* window style */
					WS_CHILD | WS_BORDER,					/* window style */
					xll,			/* x */
					yur,			/* y */
					xur-xll,		/* width */
					yll-yur,		/* height */
					hDlg,			/* parent window */
					(HMENU) ID_SPIN,		/* child window identifier */
					hInst,			/* Module instance */
					(LPSTR) CreateParams	/* params passed WM_CREATE */
/*					NULL			*//* params passed WM_CREATE */
					);
				if (hSpin == NULL) {
					winerror("No custom control");
					bSpinOK = 0;	/* avoid future complaints 1993/Sep/12 */
/* Could call RegisterControlClass maybe at this point - */
/* yes, but, not unregistered at end ? */
/*					bSpinOK = RegisterControlClass(hInst); */
				}
				else (void) ShowWindow(hSpin, SW_SHOW);  
								/* or use WM_VISIBLE above in CreateWindow */
/*				following not needed anymore, since CreateParams used */
/*				(void) SendMessage(hSpin, SPNM_SETRANGE, 0, MAKELONG(low, high)); */
/*				(void) SendMessage(hSpin, SPNM_SETCRNTVALUE, spinvalue, 0); */
				}	/* end of bSpinOK != 0 */
				/* following moved outside 93/Sep/12 */
				/* limit text that can be entered in edit control */ 
				(void) SendDlgItemMessage(hDlg,
										  ID_SPINVALUE,
										  EM_LIMITTEXT, 
										  3,
										  0L);
				/* set initial value of edit box */
/*				SetDlgItemInt(hDlg, ID_SPINVALUE, (WORD) spinvalue, TRUE); */
				SetDlgItemInt(hDlg, ID_SPINVALUE, (UINT) spinvalue, TRUE);
			}	/* end of bShowWidths != 0 */

			for (i = 0; i < nFaceIndex; i++) {  	/* displays available fonts */
/*				SendDlgItemMessage(hDlg, ID_TYPEFACE, LB_ADDSTRING,
					NULL, (LONG) (LPSTR) FontList[i]); */
//				lpCurrentName = lpFaceNames + i * MAXFACENAME;
//				lpCurrentName = lpFaceNames[i];
				if (lpFaceNames[i] == NULL) continue;	// sanity check
				(void) SendDlgItemMessage(hDlg,
										  ID_TYPEFACE,
										  LB_ADDSTRING,
										  0,	/* NULL, */
										  (LONG) (LPSTR) lpFaceNames[i]);
			}
/* now mark the one corresponding to the selected test font */
/* use LB_FINDSTRING (Windows 3.0) to do this */
/* following does weird thing: wParam = -1 ? => 0XFFFF */
/*			inx = (int) SendDlgItemMessage(hDlg, ID_TYPEFACE,
					(WORD) LB_FINDSTRING, 0xFFFF, (LONG) (LPSTR) TestFont); */
			strcpy(str, TestFont);
			if (testwantttf != 0) strcat(str, " (TT)"); /* 92/01/26 */
/*			UINT is 16 bit in WIN16 and 32 bit in WIN32 */
			inx = (int) SendDlgItemMessage(hDlg, ID_TYPEFACE,
					(UINT) LB_FINDSTRING, 0xFFFF, (LONG) (LPSTR) str); 
			if (inx != LB_ERR) { 
/*			WPARAM is 16 bit in WIN16 and 32 bit in WIN32 */
				(void) SendDlgItemMessage(hDlg, ID_TYPEFACE, LB_SETCURSEL, 
					(WPARAM) inx, 0L);
				(void) SetFocus(GetDlgItem(hDlg, ID_TYPEFACE)); /* redundant */
			}

/* setup bold and italic check marks */
			if (bCheckItalicFlag) {							/* 95/Mar/12 */
				if (bCheckBoldFlag)
					(void) SendDlgItemMessage(hDlg,
							ID_BOLDITALIC, BM_SETCHECK,  (WPARAM) 1, 0L);
				else (void) SendDlgItemMessage(hDlg, ID_ITALIC, BM_SETCHECK, 
						(WPARAM) 1, 0L);
			}
			else {
				if (bCheckBoldFlag)
					(void) SendDlgItemMessage(hDlg,
							ID_BOLD, BM_SETCHECK, (WPARAM) 1, 0L);
				else (void) SendDlgItemMessage(hDlg, ID_REGULAR, BM_SETCHECK, 
						(WPARAM) 1, 0L);
			}
/*			(void) SendDlgItemMessage(hDlg, ID_BOLD, BM_SETCHECK, 
				(WORD) bCheckBoldFlag, 0L);
			(void) SendDlgItemMessage(hDlg, ID_ITALIC, BM_SETCHECK, 
				(WORD) bCheckItalicFlag, 0L); */

			if (hEncoding != NULL) bCheckReencode = 1;		/* 95/Jan/4 */
			(void) SendDlgItemMessage(hDlg, ID_ENCODED, BM_SETCHECK, 
				(WPARAM) bCheckReencode, 0L);
/*			Send encoding vector name to dialog box 1995/July/19 */
			SetDlgItemText(hDlg, ID_ENCONAME,
/*				bCheckReencode ?  (LPSTR) szEncodingVector : (LPSTR) ""); */
				   (bCheckReencode && (szReencodingName != NULL)) ?
						   (LPSTR) szReencodingName : (LPSTR) "");
#ifdef USEUNICODE
/*			if ((!bATMLoaded || (hEncoding == NULL)) && bUseNewEncodeT1 == 0) */
			if (!bUseNewEncodeTT && !bUseNewEncodeT1 &&  
				(!bATMLoaded || (hEncoding == NULL))) {
				(void) SendDlgItemMessage(hDlg, ID_ENCODED, BM_SETCHECK, 
					0, 0L);
				(void) EnableWindow(GetDlgItem(hDlg, ID_ENCODED), FALSE);
			}
#else
			if (!bATMLoaded || (hEncoding == NULL)) {
				(void) SendDlgItemMessage(hDlg, ID_ENCODED, BM_SETCHECK, 
					0, 0L);
				(void) EnableWindow(GetDlgItem(hDlg, ID_ENCODED), FALSE);
			}
#endif
			if (bShowWidths == 0 && bWriteAFM == 0) {
				(void) SendDlgItemMessage(hDlg, ID_SHOWBOXES, BM_SETCHECK, 
					(WPARAM) bShowBoxes, 0L);
			}
			else {
				(void) ShowWindow(GetDlgItem(hDlg, ID_SHOWBOXES), SW_HIDE);
				(void) ShowWindow(GetDlgItem(hDlg, ID_SPINTEXT), SW_HIDE);
				(void) ShowWindow(GetDlgItem(hDlg, ID_SPINVALUE), SW_HIDE);
			}
			
			(void) SetFocus(GetDlgItem(hDlg, ID_TYPEFACE)); /* no effect ? */
			return (FALSE);  /* indicate we set focus to control */

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
/*			id = GET_WM_COMMAND_ID(wParam, lParam); */
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam); 
/*			wnd = (HWND) GET_WM_COMMAND_HWND(wParam, lParam); */
			wnd = GET_WM_COMMAND_HWND(wParam, lParam);

			switch (id) {
				case IDOK:
			okay:
					index = (int) SendDlgItemMessage(hDlg, ID_TYPEFACE,
						 LB_GETCURSEL, 0, 0L); 
					if (index == LB_ERR) {
						(void) MessageBox(hDlg, "No font selected",
							"Select Font", MB_OK | MB_ICONEXCLAMATION);
						break;
					}
/*		(void) SendDlgItemMessage(hDlg, ID_TYPEFACE, LB_SETCURSEL,	0, 0L); */
/*		index not directly meaningful, since items sorted */
/*			WPARAM is 16 bit in WIN16 and 32 bit in WIN32 */
					(void) SendDlgItemMessage(hDlg, ID_TYPEFACE,
						(UINT) LB_GETTEXT, (WPARAM) index, (LONG) (LPSTR) str);
					index = -1;			/* new ??? */
					for (i = 0; i < nFaceIndex; i++) {
//						lpCurrentName = lpFaceNames + i * MAXFACENAME;
//						lpCurrentName = lpFaceNames[i];
						if (lpFaceNames[i] == NULL) continue;	// sanity check
						if (strcmp(str, lpFaceNames[i]) == 0) index = i;
					}
					if (index < 0) {
						(void) MessageBox(hDlg, str,
							"Select Font", MB_OK | MB_ICONEXCLAMATION);
						index = 0;
						break;
					}
/*					bATMShowRefresh = 0; */ 			/* default 94/Dec/31 */
/*	If we have selected a different Face need to redo encoding */
					if (index != CurrentFace) bATMShowRefresh = 1;
					CurrentFace = index;

/*	new stuff to retrieve spinvalue from edit box */
/*					if (bSpinOK != 0 && bShowWidths == 0) */
					if (bShowWidths == 0) {	/* need not Spin 93/Sep/12 */
						spinvalue = (int) GetDlgItemInt(hDlg, ID_SPINVALUE, 
							&translated, TRUE);
						if(translated == 0) {	/* shouldn't happen ! */
							(void) MessageBox(hDlg, "Invalid Font Size.",
								NULL, MB_OK | MB_ICONSTOP);
							return (TRUE);
						}
						/*	Prevent overflow */
						if (spinvalue < 1) spinvalue = 1;
						else if (spinvalue > 999) spinvalue = 999;
/* hmm, does the following need bSpinOK ? 93/Sep/12 */
						if (bSpinOK != 0) {
							if ((unsigned int) spinvalue != (TestSize + 9) / 20) {
								TestSize = (unsigned int) spinvalue * 20;
								(void) SendDlgItemMessage(hDlg, ID_SPIN, 
									SPNM_SETCRNTVALUE, (WPARAM) spinvalue, 0L); 
							}
						}
					}

/* now retrieve state of bold and italic check boxes */
					if ((int) SendDlgItemMessage(hDlg,
						ID_REGULAR, BM_GETCHECK, 0, 0L)) {
						ItalicFlag=0; BoldFlag=0;
					}
					else if ((int) SendDlgItemMessage(hDlg,
						ID_ITALIC, BM_GETCHECK, 0, 0L)) {
						ItalicFlag=1; BoldFlag=0;
					}					
					else if ((int) SendDlgItemMessage(hDlg,
						ID_BOLD, BM_GETCHECK, 0, 0L)) {
						ItalicFlag=0; BoldFlag=1;
					}					
					else if ((int) SendDlgItemMessage(hDlg,
						ID_BOLDITALIC, BM_GETCHECK, 0, 0L)) {
						ItalicFlag=1; BoldFlag=1;
					}					
/*	ItalicFlag and BoldFlag may not be initialized ??? 98/Mar/23 */
/*	If we have changed regular/italic setting need to redo encoding */
					if (ItalicFlag != bCheckItalicFlag) bATMShowRefresh=1;
					bCheckItalicFlag = ItalicFlag;
/*	If we have changed regular/bold setting need to redo encoding */
					if (BoldFlag != bCheckBoldFlag) bATMShowRefresh=1;
					bCheckBoldFlag = BoldFlag;

					EncodeFlag = (int) SendDlgItemMessage(hDlg, 
						ID_ENCODED, BM_GETCHECK, 0, 0L);
/*	If we have changed `Reencode' check box need to redo encoding */
					if (bCheckReencode != EncodeFlag) bATMShowRefresh = 1;
					bCheckReencode = EncodeFlag;
					if (bShowWidths == 0)
						bShowBoxes = (int) SendDlgItemMessage(hDlg, 
							ID_SHOWBOXES, BM_GETCHECK, 0, 0L);
					EndDialog(hDlg, 1);
				break;

				case IDCANCEL:
					EndDialog(hDlg, 0);
					break;

				case ID_SPIN:
					if (bSpinOK == 0) break;
/*					switch (HIWORD(lParam)) */
					switch(cmd) {
					case SPNN_VALUECHANGE:
/*		User has changed the current value of the Spin Button. */
/*		Request the current value from the Spin Button. */
/*						spinvalue = (int) SendMessage(LOWORD(lParam), */
						spinvalue = (int) SendMessage(wnd, SPNM_GETCRNTVALUE, 
							  0, 0L);
 /*		Update static window to reflect current value in the Spin Button. */
						  if ((unsigned int) spinvalue != (TestSize + 9)/20) {
							  TestSize = (unsigned int) spinvalue * 20;
/*							  SetDlgItemInt(hDlg, ID_SPINVALUE, 
								  (WORD) spinvalue, FALSE); */
							  SetDlgItemInt(hDlg, ID_SPINVALUE, 
								  (UINT) spinvalue, FALSE);
/*	EM_SETSEL in WIN32 wParam starting position, lParam ending position */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
							  (void) SendDlgItemMessage(hDlg,
									  ID_SPINVALUE,		
									  EM_SETSEL,			
									  0, -1
/*									  0, MAKELONG(0, 0x7fff) */
							  );
							  (void) SetFocus(GetDlgItem(hDlg, ID_SPINVALUE));
						  }
						  break;

						  default:	  break;	/* ? */
					}
					break;
			
					case ID_SPINVALUE:			/* NEW STUFF */
/*					switch (HIWORD(lParam)) */	
						switch (cmd) {			/* when edit box changes */
						case EN_CHANGE:
							spinvalue = (int) GetDlgItemInt(hDlg, 
									ID_SPINVALUE, &translated, TRUE);
/*  avoid following since blank edit control yields translated = 0 */
/*					if(translated == 0) {
						(void) MessageBox(hDlg, "Invalid Font Size.",
							NULL, MB_OK | MB_ICONSTOP);
						return (TRUE);
					} */
/*				Prevent overflow */
								if (spinvalue < 1) spinvalue = 1;
								else if (spinvalue > 999) spinvalue = 999;
								if ((unsigned int) spinvalue 
										!= (TestSize + 9)/20) {
									TestSize = (unsigned int) spinvalue * 20; 
									if (hSpin != NULL)	/* NEW */
										(void) SendDlgItemMessage(hDlg, 
											ID_SPIN, SPNM_SETCRNTVALUE, 
												(WPARAM) spinvalue, 0L);
								}
								break;
							
								default:
									break;		/* ? */
						}
						break;

						case ID_TYPEFACE:
/*							switch (HIWORD(lParam)) */
							switch (cmd) {
							case LBN_SELCHANGE:
								index = (int) SendDlgItemMessage(hDlg, 
									ID_TYPEFACE, LB_GETCURSEL, 0, 0L);
								if (index == LB_ERR) break;
								break;

							case LBN_DBLCLK:
								goto okay; /* ouch ! */
							
							default:
								break; /* ? */

							}
							break;
			
			default:
				break; /*  ? */
			}
			break;

/* end of WM_COMMAND case */

			default:
				break; /*  ? */

	}
	return (FALSE);
}

/****************************************************************************

	FUNCTION: EnumFunc(LPLOGFONT, LPTEXTMETRIC, short, LPSTR)

	PURPOSE: Initializes window data and registers window class
	Argument passed in (LOWORD(lpData): 
				0 enumerating faces, 1 enumerating sizes

****************************************************************************/

/* NOTE: defined as 0x004 instead of 0x0004 in wingdi.h for some reason */

/* In Windows 3.1 NEWTEXTMETRIC is returned instead of TEXTMETRIC (for TT) */
/* NEWTEXTMETRIC has some extra fields */
/* DWORD ntmFlags	NTM_REGULAR  NTM_BOLD  NTM_ITALIC */
/* UINT ntmSizeEM	em square in `notional units' */
/* UINT ntmCellHeight height of the font in `notional units' */
/* UINT ntmAvgWidth average width in `notional units' */

/* int (CALLBACK* OLDFONTENUMPROC)(const LOGFONT FAR*, const TEXTMETRIC FAR*, int, LPARAM); */

/* int CALLBACK EnumFunc(const LPLOGFONT lpLogFont, const LPTEXTMETRIC lpTextMetric, */

/* int CALLBACK _export EnumFunc(const LOGFONT FAR *lpLogFont, 
	const TEXTMETRIC FAR *lpTextMetric, int nFontType, LPARAM lpData) */

/* This is only used by GetFaces() to gather all Face names in table */
/* Or rather SetupFaceNames() */
/* Similar to FontEnumProcEx but only case 0 is needed */
/* Could merge EnumFuncEx with FontEnumProcEx as case 5 say ... */

/* work in progress 97/Feb/11 */
int CALLBACK _export EnumFuncEx (const ENUMLOGFONTEX FAR *lpEnumLogFontEx,
	const NEWTEXTMETRICEX FAR *lpNewTextMetricEx,
								int nFontType, LPARAM lpData) {
	LPLOGFONT lpLogFont = (LPLOGFONT) lpEnumLogFontEx;		/* 95/July/8 */
	LPNEWTEXTMETRIC lpNewTextMetric = (LPNEWTEXTMETRIC) lpNewTextMetricEx; 
	WORD choice;
/*	int ttfflag = 0; */

/*	if (bDebug > 1) OutputDebugString("Enter EnumFuncEx\n"); */

	choice = LOWORD((LONG) lpData);				/* was: 0 => faces */

	if (lpFaceNames == NULL) {
		if (bDebug > 1) OutputDebugString("lpFaceNames == NULL\n");
		return FALSE;		/* serious bug ! */
	}

/*	The following filters fonts and rejects for various reasons */

	switch (choice) {
		case 0:					/* collect all Face Names for table */
#ifdef IGNORED
			if (bDebug > 1) {
				int boldflag=0, italicflag=0;
				if (lpLogFont->lfWeight > 400) boldflag = 1;
				if (lpLogFont->lfItalic) italicflag = 1;
				wsprintf(debugstr,
						 "%3d %2X Face: %s,%s%s%s %2X %2X Full: %s, %s, %s\n",
						 nFaceIndex, nFontType,
						 lpLogFont->lfFaceName,
						 (!boldflag && !italicflag) ? "REGULAR" : "",
						 boldflag ? "BOLD" : "",
						 italicflag ? "ITALIC" : "",
						 lpLogFont->lfCharSet,
						 lpLogFont->lfPitchAndFamily,
						 lpEnumLogFontEx->elfFullName,
						 lpEnumLogFontEx->elfStyle,
						 lpEnumLogFontEx->elfScript);	/* 97/Feb/11 */
				OutputDebugString(debugstr);
			}
#endif
/*			if (nFaceIndex >= MAXOUTLINEFONTS) return (0); */ /* STOP ! */
			if (nFaceIndex >= MAXOUTLINEFACES) {
				sprintf(debugstr, "Too Many Face Names (> %d)\n", MAXOUTLINEFACES);
				if (bDebug > 1) OutputDebugString(debugstr);
				WriteError(debugstr);
				return (0); /* STOP ENUMERATING ! */
			}
/*		for Windows 3.1 ??? */
/*		check (lpNewTextMetric->lfPitchandFamily & 4) Windows 3.1 ??? */
/*		lpPitchandFamily:	device, truetype, vector, variable pitch bits */
/*	Font should be either DEVICE_FONTTYPE (T1) or TRUETYPE_FONTTYPE (TT) */
/*  --- and font should *not* be RASTER_FONTTYPE */
/*	WARNING: in Windows NT 4, ATM fonts are NOT DEVICE_FONTTYPE ! 97/Jan/21 */
			if (!bWinNT) {				/* 97/Jan/15 and 97/Feb/2 */
				if (((nFontType & TRUETYPE_FONTTYPE) == 0) &&
					((nFontType & DEVICE_FONTTYPE) == 0)) return (1);
			}

			if (bWinNT) {	/* 97/Jan/15 and 97/Feb/2 */
/*	In Windows NT ATM beta, Symbol and Courier are RASTER_FONTTYPE 97/Jan/21 */
/*				if ((strcmp(lpLogFont->lfFaceName, "Courier") != 0) &&
					(strcmp(lpLogFont->lfFaceName, "Symbol") != 0)) {
					if ((nFontType & RASTER_FONTTYPE) != 0) return(1); 
				} */ /* replaced with below 97/Mar/2 */
				if (lpNewTextMetric->tmDigitizedAspectX == 1001) {
/*					It is a Type 1 font in Windows NT 97/Mar/2 */
#ifdef DEBUGTABLES
					if (bDebug > 1) {
						sprintf(debugstr, "%s is Type 1 (1001)\n",
								lpLogFont->lfFaceName);
						OutputDebugString(debugstr);
					}
#endif
				}
/*	we don't want raster fonts */
				else if ((nFontType & RASTER_FONTTYPE) != 0) return(1);
/*	we want only TrueType and Type 1 */
				else if (((nFontType & TRUETYPE_FONTTYPE) == 0) &&
						 ((nFontType & DEVICE_FONTTYPE) == 0)) return (1);
			}
			else if ((nFontType & RASTER_FONTTYPE) != 0) return(1); 

/* check (nFontType & RASTER_FONTTYPE) should be FALSE */
/*				if ((nFontType & RASTER_FONTTYPE) != 0) return (1); */
/* check (nFontType & DEVICE_FONTTYPE) - should be TRUE */
/*				if ((nFontType & DEVICE_FONTTYPE) == 0) return (1); */
/* check whether it really is an ATM font ... */ /* not for TrueType */
/*				if (lpLogFont->lfQuality != PROOF_QUALITY) return (1); */
/*			} */
	
//			lpCurrentName = lpFaceNames + nFaceIndex * MAXFACENAME; 
//			lpCurrentName = lpFaceNames[nFaceIndex]; 
/* FaceName is BYTE, e.g. unsigned char ... */
//			(void) strcpy(lpCurrentName, lpLogFont->lfFaceName);    /* NEW */
			strcpy(str, lpLogFont->lfFaceName);
/*			sprintf(str, " (%d)", nFontType); */
/*			(void) lstrcat(lpCurrentName, str); */ /* debugging only */
			if ((nFontType & TRUETYPE_FONTTYPE) != 0) {
/*				nTrueIndex++; */
//				(void) lstrcat(lpCurrentName, " (TT)"); /* TrueType */
				lstrcat(str, " (TT)"); /* TrueType */
/*  If TrueType, and only Type 1 requested, then ignore this one 95/Jun/30 */
				if (bTypeOneOnly != 0) return(1);
/*				IsTTF[nFaceIndex] = 1; */		/* 95/Feb/5 */
/*				ttfflag = 1; */
			}
/*	If not TrueType, and only TrueType requested, then ignore this one */
			else if (bTrueTypeOnly != 0) return(1);

/*	Windows NT same face may occur many times with different Script */
/*	Western, Greek, Turkish, Baltic, Central European, Cyrillic */
/*	CharSet 0, A1, A2, BA, EE, CC or Mac: 4D Symbol: 02 OEM/DOS: FF */
/*	Avoid duplication in Face Name table *//* keep first one only */
			if (nFaceIndex > 0)	{
//				lpPreviousName = lpFaceNames[nFaceIndex-1]; 
//				if(strcmp(lpCurrentName - MAXFACENAME, lpCurrentName) == 0)
				if (strcmp(lpFaceNames[nFaceIndex-1], str) == 0)
					return(TRUE);	/* ignore this one, it is a duplicate */
			}

//			OK, it's a keeper and not a duplicate
			
			if (lpFaceNames[nFaceIndex] != NULL) free(lpFaceNames[nFaceIndex]);
			lpFaceNames[nFaceIndex] = zstrdup(str);

			if ((nFontType & TRUETYPE_FONTTYPE) != 0) nTrueIndex++;

			CharSet[nFaceIndex] = lpLogFont->lfCharSet;
			PitchAndFamily[nFaceIndex] = lpLogFont->lfPitchAndFamily;
/*			Italic[nFaceIndex] = lpLogFont->lfItalic; */
/*			Weight[nFaceIndex] = lpLogFont->lfWeight; */

#ifdef DEBUGFONTSELECT
			if (bDebug > 1) {
				int boldflag=0, italicflag=0;
				if (lpLogFont->lfWeight > 400) boldflag = 1;
				if (lpLogFont->lfItalic) italicflag = 1;
				wsprintf(debugstr,
						 "%3d %2X Face: %s,%s%s%s %2X %2X Full: %s, %s, %s\n",
						 nFaceIndex, nFontType,
						 lpLogFont->lfFaceName,
						 (!boldflag && !italicflag) ? "REGULAR" : "",
						 boldflag ? "BOLD" : "",
						 italicflag ? "ITALIC" : "",
						 lpLogFont->lfCharSet,
						 lpLogFont->lfPitchAndFamily,
						 lpEnumLogFontEx->elfFullName,
						 lpEnumLogFontEx->elfStyle,
						 lpEnumLogFontEx->elfScript);	/* 97/Feb/11 */
				OutputDebugString(debugstr);
			}
#endif
			nFaceIndex++;		/* OK, it IS a new one */
/*			else IsTTF[nFaceIndex] = 0; */			/* 95/Feb/5 */

/*			if (strcmp(lpLogFont->lfFaceName, "Symbol") == 0) {
				showlogfont(lpLogFont, ttfflag);		
			} */
			return (TRUE);			/* continue enumerating */

		default:
			break;	/* ? */
	}
	return (-1);	/* didn't understand ??? */
}

int CALLBACK _export EnumFunc (const ENUMLOGFONT FAR *lpEnumLogFont, 
	const NEWTEXTMETRIC FAR *lpNewTextMetric, int nFontType, LPARAM lpData) { 
	LPLOGFONT lpLogFont = (LPLOGFONT) lpEnumLogFont;		/* 95/July/8 */
	WORD choice;
/*	int ttfflag = 0; */

/*      Issue   : Check if LOWORD target is 16- or 32-bit         */
	choice = LOWORD((LONG) lpData);	/* was: 0 => faces */

/* The following filters fonts and rejects for various reasons */

	if (lpFaceNames == NULL) return FALSE;		/* serious bug ! */

/*	switch (LOWORD(lpData)) */
	switch (choice) {
		case 0:
#ifdef DEBUGFONTSELECT
			if (bDebug > 1) {
				wsprintf(debugstr, "%3d %2X %s\n", nFaceIndex, nFontType,
						 lpLogFont->lfFaceName);
				OutputDebugString(debugstr);
			}
#endif
/*			if (nFaceIndex >= MAXOUTLINEFONTS) return (0); */ /* STOP ! */
			if (nFaceIndex >= MAXOUTLINEFACES) {
				sprintf(debugstr, "Too Many Face Names (> %d)\n", MAXOUTLINEFACES);
				if (bDebug > 1) OutputDebugString(debugstr);
				WriteError(debugstr);
				return (0); /* STOP ! */
			}
/*		for Windows 3.1 ??? */
/*		check (lpTextMetric->lfPitchandFamily & 4) Windows 3.1 ??? */
/*		lpPitchandFamily:	device, truetype, vector, variable pitch bits */
/*	Font should be either DEVICE_FONTTYPE (T1) or TRUETYPE_FONTTYPE (TT) */
/*  --- and font should *not* be RASTER_FONTTYPE */
/*	WARNING: in Windows NT, ATM fonts are NOT DEVICE_FONTTYPE ! 97/Jan/21 */
			if (!bWinNT) {				/* 97/Jan/15 and 97/Feb/2 */
				if (((nFontType & TRUETYPE_FONTTYPE) == 0) &&
					((nFontType & DEVICE_FONTTYPE) == 0)) return (1);
			}

			if (bWinNT) {	/* 97/Jan/15 and 97/Feb/2 */
/*	In Windows NT ATM beta, Symbol and Courier are RASTER_FONTTYPE 97/Jan/21 */
/*				if ((strcmp(lpLogFont->lfFaceName, "Courier") != 0) &&
					(strcmp(lpLogFont->lfFaceName, "Symbol") != 0)) {
					if ((nFontType & RASTER_FONTTYPE) != 0) return(1); 
				} */ /* replaced with below 97/Mar/2 */
				if (lpNewTextMetric->tmDigitizedAspectX == 1001) {
/*					It is a Type 1 font in Windows NT 97/Mar/2 */
#ifdef DEBUGTABLES
					if (bDebug > 1) {
						sprintf(debugstr, "%s is Type 1 (1001)\n",
								lpLogFont->lfFaceName);
						OutputDebugString(debugstr);
					}
#endif
				}
/*	we don't want raster fonts */
				else if ((nFontType & RASTER_FONTTYPE) != 0) return(1); 
/*	we want only TrueType and Type 1 */
				else if (((nFontType & TRUETYPE_FONTTYPE) == 0) &&
						 ((nFontType & DEVICE_FONTTYPE) == 0)) return (1);
			}
			else if ((nFontType & RASTER_FONTTYPE) != 0) return(1); 

/* check (nFontType & RASTER_FONTTYPE) should be FALSE */
/*				if ((nFontType & RASTER_FONTTYPE) != 0) return (1); */
/* check (nFontType & DEVICE_FONTTYPE) - should be TRUE */
/*				if ((nFontType & DEVICE_FONTTYPE) == 0) return (1); */
/* check whether it really is an ATM font ... */ /* not for TrueType */
/*				if (lpLogFont->lfQuality != PROOF_QUALITY) return (1); */
/*			} */

//			lpCurrentName = lpFaceNames + nFaceIndex * MAXFACENAME; 
//			lpCurrentName = lpFaceNames[nFaceIndex]; 
/* FaceName is BYTE, e.g. unsigned char ... */
//			(void) strcpy(lpFaceNames[nFaceIndex], lpLogFont->lfFaceName); 
			strcpy(str, lpLogFont->lfFaceName); 
/*			sprintf(str, " (%d)", nFontType); */
/*			(void) lstrcat(lpCurrentName, str); */ /* debugging only */
			if ((nFontType & TRUETYPE_FONTTYPE) != 0) {
				nTrueIndex++;
//				(void) lstrcat(lpCurrentName, " (TT)"); /* TrueType */
				lstrcat(str, " (TT)"); /* TrueType /*
/*  If TrueType, and only Type 1 requested, then ignore this one 95/Jun/30 */
				if (bTypeOneOnly != 0) return(1);
/*				IsTTF[nFaceIndex] = 1; */		/* 95/Feb/5 */
/*				ttfflag = 1; */
			}
/*	If not TrueType, and only TrueType requested, then ignore this one */
			else if (bTrueTypeOnly != 0) return(1);

/*	it's a keeper */
			
			if (lpFaceNames[nFaceIndex] != NULL) free(lpFaceNames[nFaceIndex]);
			lpFaceNames[nFaceIndex] = zstrdup(str); 

			CharSet[nFaceIndex] = lpLogFont->lfCharSet;
			PitchAndFamily[nFaceIndex] = lpLogFont->lfPitchAndFamily;
/*			Italic[nFaceIndex] = lpLogFont->lfItalic; */
/*			Weight[nFaceIndex] = lpLogFont->lfWeight; */
			nFaceIndex++;
/*			if (strcmp(lpLogFont->lfFaceName, "Symbol") == 0) {
				showlogfont(lpLogFont, ttfflag);		
			} */
			return (TRUE);			/* continue enumerating */

		default:
			break;	/* ? */
	}
	return (-1);	/* didn't understand ??? */
}

/* may want to somehow keep track of lpLogFonts data ? */

/****************************************************************************

	FUNCTION: GetFaces(HWND)

	PURPOSE: Get available fonts

****************************************************************************/

void GetFacesSub(HDC hDC) {
	LOGFONT lf;

	if (nFaceIndex != 0) return;/* maybe don't do this if already exists ? */
	nFaceIndex = 0;				/* reset indices into arrays */
	nTrueIndex = 0;				/* count TrueType faces */
#ifdef DEBUGFONTSELECT
	if (bDebug > 1) {
		sprintf(debugstr, "GetFacesSub: dWin95 %d, dWinNT %d\n",
			   bWin95, bWinNT);
		OutputDebugString(debugstr);
	}
#endif
/*	(void) SetMapMode(hDC, MM_TWIPS); */ /* 92/Feb/16 */ /* ??? */

	memset(&lf, 0, sizeof(LOGFONT));
/*	Work in progress 97/Feb/11 */
	lf.lfCharSet = DEFAULT_CHARSET;	/* enumerate *all* character set fonts */
	strcpy(lf.lfFaceName, "");	/* enumerate one font per typeface name */
	lf.lfPitchAndFamily = 0;	/* MONO_FONT for Hebrew and Arabic */
	if (bNewShell)				/* experiment ??? 97/Mar/26 */
		(void) EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC) EnumFuncEx,
							  (LPARAM) 0, (DWORD) 0L); 
	else (void) EnumFontFamilies(hDC, (LPSTR) NULL, (FONTENUMPROC) EnumFunc,
						(LPARAM) NULL);	 /* try this ??? 97/Mar/26 */
/*		(void) EnumFontFamilies(hDC, (LPSTR) NULL, (FONTENUMPROC) EnumFunc,
						(LPARAM) NULL); */

#ifdef DEBUGFONTSELECT
	if (bDebug > 1) {
		sprintf(debugstr, "Exit GetFacesSub %d Faces (%d TT + %d non-TT)",
				nFaceIndex, nTrueIndex, nFaceIndex-nTrueIndex);
		OutputDebugString(debugstr);
	}
#endif
}

/* May want to switch from EnumFonts to EnumFontFamilies sometime */
/* Get lfFaceName, lfFullName, lfStyle */ /* For Windows 3.1 */
/* FullName is longer than FaceName because it contains style modifiers */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* spew out TextMetric information */

void spewmetrics (char *buf, TEXTMETRIC TextMetric, char *text) {
	char *s = buf;
	strcpy(s, text);
	s = s + strlen(s);
	sprintf(s, 
	"Height: %d;  Ascent: %d;  Descent: %d;\nIntLead: %d;  ExtLead: %d;\n", 
		TextMetric.tmHeight, TextMetric.tmAscent, TextMetric.tmDescent,
			TextMetric.tmInternalLeading, TextMetric.tmExternalLeading);
	s = s + strlen(s);
	sprintf(s, " AveWidth: %d;  MaxWidth: %d;  Weight: %d;  Italic: %d;\n",
			TextMetric.tmAveCharWidth, TextMetric.tmMaxCharWidth,
				TextMetric.tmWeight, TextMetric.tmItalic);
	s = s + strlen(s);
	sprintf(s, "First: %d;  Last: %d;  Default: %d;  Break: %d;\n",
		TextMetric.tmFirstChar, TextMetric.tmLastChar, 
			TextMetric.tmDefaultChar, TextMetric.tmBreakChar);
	s = s + strlen(s);
 	sprintf(s, "CharSet: %d;  Family: %d;\n",
		(int) TextMetric.tmCharSet, (TextMetric.tmPitchAndFamily >> 4));
} 

/* crude way to list out metric information */

void showmetrics(HWND hWnd) {
	HFONT hFontOld, hPrintFont, hMetricFont;
/*	HLOCAL hBuffer; */			/* 1995/July/27 */
	char *lpBuffer=NULL;
	char *s;
	HDC hDC;
	int flag;
	WORD options;

	if (*TestFont == '\0') return;	
/*	hBuffer = LocalAlloc(LMEM_FIXED, 1000);
	if (hBuffer == NULL) {
		winerror("Unable to allocate memory");
		return;
	}
	if ((lpBuffer = LocalLock(hBuffer)) == NULL) {
		hBuffer = LocalFree(hBuffer);
		return;
	} */
	lpBuffer = (char *) LocalAlloc(LMEM_FIXED, 1000);
	if (lpBuffer == NULL) {
		winerror("Unable to allocate memory");
		return;
	}

	s = lpBuffer;
	hDC = GetDC(hWnd);

/*	hMetricFont = createatmfont(TestFont, METRICSIZE, 0, */
	hMetricFont = createatmfont(TestFont, metricsize, 0, 
		bCheckBoldFlag, bCheckItalicFlag, testwantttf); 
	if (hMetricFont == NULL) winerror("Can't create metric font");
	else {
		if (bATMReencoded) UnencodeFont(hDC, -1, 1);
		if (!testwantttf && bATMPrefer && bATMLoaded) {
			if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
			else options = ATM_SELECT;
			(void) MyATMSelectObject (hDC, hMetricFont, options, &hFontOld);
		}
		else hFontOld = SelectObject(hDC, hMetricFont);	

		if (hFontOld == NULL) winerror("SelectObject failed");
#ifdef USEUNICODE
/*		if (bUseNewEncode) bFontEncoded = bCheckReencode; */
		if ((bUseNewEncodeTT && testwantttf) ||
			(bUseNewEncodeT1 && !testwantttf)) bFontEncoded = bCheckReencode;
		else bFontEncoded = 0;
/*		else */
/*		hEncoding will be NULL in Windows NT */
		if (bCheckReencode && !testwantttf && hEncoding != NULL)
			ReencodeFont(hDC, -1, 1); 				/* 94/Dec/25 */
#else
		if (bCheckReencode && !testwantttf && hEncoding != NULL)
			ReencodeFont(hDC, -1, 1); 				/* 94/Dec/25 */
#endif
		flag = GetTextMetrics(hDC, &TextMetric);
		if (flag == 0) winerror("GetTextMetrics failed"); 
/*		yposition = 10; */
		if ((UINT) hFontOld > 1)	/* avoid Windows 3.0 MetaFile problem */
			(void) SelectObject(hDC, hFontOld);	
		spewmetrics(s, TextMetric, "SCREEN:\n");
		if (hPrintDC != NULL) {			
			s = s + strlen(s);
			if ((UINT) hMetricFont > 1)
				(void) DeleteObject(hMetricFont);	
/*			hMetricFont = createatmfont(TestFont, METRICSIZE, 0, */
			hMetricFont = createatmfont(TestFont, metricsize, 0, 
/*				bCheckBoldFlag, bCheckItalicFlag, 0); */
				bCheckBoldFlag, bCheckItalicFlag, testwantttf);
			if (bATMReencoded) UnencodeFont(hDC, -1, 1);
			if (!testwantttf && bATMPrefer && bATMLoaded) {
				if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
				else options = ATM_SELECT;
				(void) MyATMSelectObject (hPrintDC, hMetricFont, options, &hPrintFont);
			}
			else hPrintFont = SelectObject(hPrintDC, hMetricFont);

			if (hPrintFont == NULL) winerror("SelectObject failed");
#ifdef USEUNICODE
/*			if (bUseNewEncode) bFontEncoded = bCheckReencode; */
			if ((bUseNewEncodeTT && testwantttf) ||
				(bUseNewEncodeT1 && !testwantttf)) bFontEncoded = bCheckReencode;
			else bFontEncoded = 0;
/*			else */
/*			hEncoding will be NULL in Windows NT */
			if (bCheckReencode && !testwantttf && hEncoding != NULL)
				ReencodeFont(hDC, -1, 1); 				/* 94/Dec/25 */
#else
			if (bCheckReencode && !testwantttf && hEncoding != NULL)
				ReencodeFont(hDC, -1, 1); 				/* 94/Dec/25 */
#endif
			flag = GetTextMetrics(hPrintDC, &TextMetric);
			if (flag == 0) winerror("GetTextMetrics failed"); 
			spewmetrics(s, TextMetric, "PRINTER:\n");
			if (bATMReencoded) UnencodeFont(hDC, -1, 0); /* moved up here */
			if ((UINT) hPrintFont > 1) /* avoid Windows 3.0 MetaFile problem */
				(void) SelectObject(hPrintDC, hPrintFont);	
		} /* hPrintDC != NULL */
		if ((UINT) hMetricFont > 1)
			(void) DeleteObject(hMetricFont);	
		(void) MessageBox(hWnd, lpBuffer, "DVIWindo metrics", 
			MB_ICONINFORMATION | MB_OK);
	} /* hMetricFont was able to create */
	(void) ReleaseDC(hWnd, hDC);
/*	(void) LocalUnlock(hBuffer); */
/*	(void) LocalFree(hBuffer); */
/*	hBuffer = LocalFree(hBuffer); */
	lpBuffer = (char *) LocalFree (lpBuffer);
/*	if (bATMReencoded) UnencodeFont(hDC, -1, 0); */
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/****************************************************************************
Module name: Spin.C
Programmer : Jeffrey M. Richter.
*****************************************************************************/

typedef enum { TD_NONE, TD_UP, TD_DOWN } TRIANGLEDOWN;

/* TD_NONE not used */

/* LONG CALLBACK SpinWndFn (HWND hWnd, 
							WORD wMsg, WORD wParam, LONG lParam); */

/* moved to dviwindo.c */

/* static BOOL NEAR PASCAL RegisterControlClass (HINSTANCE hInstance) {
	WNDCLASS wc;
	wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = SpinWndFn;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = CBWNDEXTRA;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1); 
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szControlName;
	return(RegisterClass(&wc));
} */

/* NOTE: in Windows NT the packing of wParam and lParam will change */

/* Use a different method ? WIN32 ? */

/* static */ /* ??? */
LONG NEAR PASCAL NotifyParent (HWND hWnd, WORD wNotifyCode) {
	LONG lResult;

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
/*      Issue   : Check if LOWORD target is 16- or 32-bit         */

	lResult = SendMessage(GetParent(hWnd), WM_COMMAND,
			(UINT) MAKELONG(LOWORD(GetWindowLong(hWnd, GWL_ID)), wNotifyCode),
						  (LPARAM) hWnd);

/*	lResult = SendMessage(GetParent(hWnd), WM_COMMAND,
			GetWindowWord(hWnd, GWW_ID),		
				MAKELONG(hWnd, wNotifyCode)); */
	
	return(lResult);
}

/* check how things are packed in wParam and lParam in WIN32 SPNM_SCROLLVALUE*/

/* Window Function for Spinner */
/* note absences of use of global or static variables */

/* typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM); */

/* LONG CALLBACK SpinWndFn (HWND hWnd, WORD wMsg, WORD wParam, LONG lParam) */
/* LONG CALLBACK _export SpinWndFn (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) */
LRESULT CALLBACK _export SpinWndFn (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
	LONG lResult = 0;
	HDC hDC;
	POINT pt;
	RECT rc;
	PAINTSTRUCT ps;
	int nCrntVal, nNewVal, x, y;
	TRIANGLEDOWN TriangleDown, OldTriangleDown;
	DWORD dwTime, dwRange;
	BOOL fWrap;
	LPCREATESTRUCT CreateStruct;
	LPINT CreateParams; 
/*	MSG peekmsg; */
	int shiftflag;				/* negative if shift key is down */
	int low=0, curr=0, high=0;	
	POINT Box[4];								/* 1995/Jan/15 */
	int fwKeys;					/* shift, control key bits */
	int xPos, yPos;
	POINTS pts;

	switch (wMsg) {
	  case WM_GETDLGCODE:
		 lResult = DLGC_STATIC;
		 break;

	  case WM_CREATE:			/* lParam == &CreateStruct */
		 CreateStruct = (LPCREATESTRUCT) lParam;
		 CreateParams = (LPINT) CreateStruct->lpCreateParams;  
		 low = CreateParams[0]; 
		 curr = CreateParams[1]; 
		 high = CreateParams[2];
		 (void) SendMessage(hWnd, SPNM_SETRANGE, 0, MAKELONG(low, high));
		 (void) SendMessage(hWnd, SPNM_SETCRNTVALUE, (WPARAM) curr, 0L);
		 break;

	  case WM_PAINT:
	/* Calling BeginPaint sends a WM_ERASEBKGND message.  Because that */
	/* message is not trapped, DefWindowProc uses the system color */
	/* COLOR_BTNFACE because it was specified in the hbrBackground */
	/* member of the WNDCLASS structure when this class was registered. */
		  hDC = BeginPaint(hWnd, &ps);

		  GetClientRect(hWnd, &rc);
		  x = rc.right / 2;
		  y = rc.bottom / 2;

		  /* Draw middle separator bar */
		MoveToEx(hDC, 0, y, NULL); 
		(void) LineTo(hDC, rc.right, y);

/* could speed up using Polyline(), but hardly worth it */

/* Whenever a DC is retrieved, it is created with a WHITE_BRUSH */
/* by default, we must change this to a BLACK_BRUSH so that we */
/* can fill the triangles. */
		 (void) SelectObject(hDC, GetStockObject(BLACK_BRUSH));

/* Some weirdness with FloodFill on Diamond Viper ... */

/* Draw top triangle & fill it in */

		 Box[0].x = x;				Box[0].y = 2;
		 Box[1].x = rc.right - 2;	Box[1].y = y - 2;
		 Box[2].x = 2;				Box[2].y = y - 2;
		 Box[3].x = x;				Box[3].y = 2;		 
		 if (bAvoidFlood) Polyline (hDC, Box, 4);	/* 1995/Jan/12 */
		 else Polygon (hDC, Box, 4);				/* 1995/Jan/12 */

/* Draw bottom triangle & fill it in */

		 Box[0].x = 2;				Box[0].y = y + 2;
		 Box[1].x = rc.right - 2;	Box[1].y = y + 2;
		 Box[2].x = x;				Box[2].y = rc.bottom - 2;
		 Box[3].x = 2;				Box[3].y = y + 2;		 
		 if (bAvoidFlood) Polyline (hDC, Box, 4);	/* 1995/Jan/12 */
		 else Polygon (hDC, Box, 4);				/* 1995/Jan/12 */

		 EndPaint(hWnd, &ps);
		 break;

/* fwKeys = wParam;       key flags                    
xPos = LOWORD(lParam);    horizontal position of cursor 
yPos = HIWORD(lParam);    vertical position of cursor  
*/ /* WIN16 */  

/* fwKeys = wParam;       key flags 
xPos = LOWORD(lParam);    horizontal position of cursor 
yPos = HIWORD(lParam);    vertical position of cursor 
*/ /* WIN32 */

	  case WM_LBUTTONDOWN:
		fwKeys = wParam;        	 /* key flags */
		pts = MAKEPOINTS(lParam);
		xPos = pts.x;
		yPos = pts.y;

/*		xPos = LOWORD(lParam); */   	 /* horizontal position of cursor */
/*		yPos = HIWORD(lParam); */   	 /* vertical position of cursor */

	/* Get coordinates for the Spin Button's window. */
		 GetClientRect(hWnd, &rc);
/*		 if ((int) HIWORD(lParam) < rc.bottom / 2) */ /* Up arrow */
		 if (yPos < rc.bottom / 2) { /* Up arrow */
			TriangleDown = TD_UP;
	/* Change coordinates so rectangle includes only the top-half */
	/* of the window. */
			rc.bottom /= 2;
		 } else {
			TriangleDown = TD_DOWN;

	/* Change coordinates so rectangle includes only the bottom-half */
	/* of the window. */
			rc.top = rc.bottom / 2;
		 }

	/* Save which triangle the mouse was clicked over. */
		 (void) SetWindowLong(hWnd, GWL_TRIANGLEDOWN, (LONG) TriangleDown);
/*		 (void) SetWindowWord(hWnd, GWW_TRIANGLEDOWN, (WORD) TriangleDown); */

	/* Invert The top or bottom half of the window where the */
	/* mouse was clicked. */
		 hDC = GetDC(hWnd);
		 InvertRect(hDC, &rc);
		 (void) ReleaseDC(hWnd, hDC);
		 (void) SetCapture(hWnd);
		 (void) SendMessage(hWnd, SPNM_SCROLLVALUE, 0, 0L); /* do once */

/*		 Subtract TIME_DELAY so that action is performed at least once. */
/*		 dwTime = GetTickCount() - TIME_DELAY; */
		 dwTime = GetTickCount() + EXTRA_DELAY;		/* first time */

		 do {
		/* If TIME_DELAY hasn't passed yet, test loop condition. */
			if (dwTime + TIME_DELAY > GetTickCount()) continue;

		/* Time delay has passed, scroll value in Spin Button. */
			(void) SendMessage(hWnd, SPNM_SCROLLVALUE, 0, 0L);

		/* Get last time when scroll occurred. */
			dwTime = GetTickCount();

		/* Check if left mouse button is still down. */
/*		 } while (GetAsyncKeyState(VK_LBUTTON) & 0x8000); */
		 } while (GetAsyncKeyState(VK_LBUTTON) < 0); 

		 ReleaseCapture();

	/* Invalidate the entire window.  This will force Windows to send */
	/* a WM_PAINT message restoring the window to its original colors. */
		 InvalidateRect(hWnd, NULL, TRUE);
		 break;

	  case SPNM_SCROLLVALUE:
	/* Get shift key => negative if shift is down */
/*		 shiftflag = GetKeyState(VK_SHIFT); */
		 shiftflag = GetAsyncKeyState(VK_SHIFT); 

	/* Get the location of the mouse. */
		 GetCursorPos(&pt);

	/* Convert the point from screen coordinates to client coordinates. */
		 ScreenToClient(hWnd, &pt);

	/* If the point is NOT in Spin's client area, nothing to do. */
		 GetClientRect(hWnd, &rc);
		 if (!PtInRect(&rc, pt)) break;

	/* Get the Spin Button's current value and range, */
		 nNewVal = (int) SendMessage(hWnd, SPNM_GETCRNTVALUE, 0, 0L);
		 nCrntVal = nNewVal;
		 dwRange = (DWORD) SendMessage(hWnd, SPNM_GETRANGE, 0, 0L);
/*      Issue   : Check if HIWORD target is 16- or 32-bit         */
		 high = (int) HIWORD(dwRange);
/*      Issue   : Check if LOWORD target is 16- or 32-bit         */
		 low = (int) LOWORD(dwRange);

   /* Get Spin Button's styles and test if the "wrap" flag is set. */
		 fWrap = (BOOL) (GetWindowLong(hWnd, GWL_STYLE) & SPNS_WRAP);

	/* Determine whether the up- or down- triangle was selected. */
		 OldTriangleDown = GetWindowLong(hWnd, (int) GWL_TRIANGLEDOWN);
/*		 OldTriangleDown = GetWindowWord(hWnd, (int) GWW_TRIANGLEDOWN); */

	/* Determine whether the mouse is now over the up- or down-	triangle. */
		 TriangleDown = (pt.y < rc.bottom / 2) ? TD_UP : TD_DOWN;

	/* If the user has switched triangles, invert the entire rectangle. */
	/* This restores the half that was inverted in the WM_LBUTTONDOWN */
	/* message and inverts the new half. */
		 if (OldTriangleDown != TriangleDown) {
			hDC = GetDC(hWnd);
			InvertRect(hDC, &rc);
			(void) ReleaseDC(hWnd, hDC);
		 }

		 if (TriangleDown == TD_UP) {
	/* If value is not at top of range, increment it. */
			if (high > nCrntVal) {
				if (shiftflag < 0) {
					nNewVal += 4;
					if (nNewVal > high) nNewVal = high;
				}
				else nNewVal++;
			}
			else {	/* don't increment it anymore */
	  /* If value at top of range and the "wrap" flag is set, */
	  /* set the value to the bottom of the range. */
				if (fWrap) nNewVal = low;
			}

		 } else {
	/* If value is not at bottom of range, decrement it. */
			if (low < nCrntVal) {
				if (shiftflag < 0) {
					nNewVal -= 4;
					if (nNewVal < low) nNewVal = low;
				}
				else nNewVal--;
			}
			else { /* don't decrement it any more */
	  /* If value at bottom of range and the "wrap" flag is set, */
	  /* set the value to the top of the range. */
				if (fWrap) nNewVal = high;
			}
		 }

	/* If the value has been changed, set the new value. */
		 if (nNewVal != nCrntVal)
			(void) SendMessage(hWnd, SPNM_SETCRNTVALUE, (WPARAM) nNewVal, 0L);

	/* Set the new triangle that is down for the next call to here. */
		 (void) SetWindowLong(hWnd, GWL_TRIANGLEDOWN, (LONG) TriangleDown);
/* 		 (void) SetWindowWord(hWnd, GWW_TRIANGLEDOWN, (WORD) TriangleDown); */
		 break;

	  case SPNM_SETRANGE:
		 (void) SetWindowLong(hWnd, GWL_RANGE, lParam);	/* OK in win32 ? */
		 break;

	  case SPNM_GETRANGE:
		 lResult = GetWindowLong(hWnd, GWL_RANGE);
		 break;

	  case SPNM_SETCRNTVALUE:
/*		 if (wParam < 1) wParam = 1;
		 else (wParam > 999) wParam = 999; */
		 (void) SetWindowLong(hWnd, GWL_CRNTVALUE, (LONG) wParam);
/*	 	 (void) SetWindowWord(hWnd, GWW_CRNTVALUE, (WORD) wParam); */
		 (void) NotifyParent(hWnd, SPNN_VALUECHANGE);	/* recursion ? */
		 break;

	  case SPNM_GETCRNTVALUE:
		 lResult = GetWindowLong(hWnd, GWL_CRNTVALUE);
/*		 lResult = (LONG) GetWindowWord(hWnd, GWW_CRNTVALUE); */
		 break;

	  default:
		 lResult = DefWindowProc(hWnd, wMsg, wParam, lParam);
		 break;
	}
	return(lResult);
}

/* Setting it up: */

/* SendDlgItemMessage(hDlg, ID_SPIN, SPNM_SETRANGE, 0, MAKELONG(0, 999)); */
/* SendDlgItemMessage(hDlg, ID_SPIN, SPNM_SETCRNTVALUE, 10, 0L); */
/* SendDlgItemMessage(hDlg, ID_SPIN, SPNM_GETCRNTVALUE, 0, 0L); */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff needed for Fonts Used display */

int markfont=-1;

/* int oldmarkfont=-1; */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* used in FontUsed dialog box procedure */ /* change if layout changes */

int extractfontnumber(char *item) {	/* font number follows fifth tab */
	int k, ff;
	char *s = item;
/*	if ((s = strchr(s, '\t')) != NULL) {
		if ((s = strchr(s+1, '\t')) != NULL) {
			if (sscanf(s+1, "(%d)", &ff) > 0) return ff;
		}
	} */
	for (k = 0; k < 5; k++) {
		if ((s = strchr(s, '\t')) == NULL) {
/*			if (bDebug > 1) OutputDebugString(item); */
			return -1;
		}
		s++;
	}
	if (sscanf(s, "(%d)", &ff) > 0) return ff;
/*	if (bDebug > 1) {
		OutputDebugString(item);
		OutputDebugString(s);
	} */
	return -1;
}

/* map from scaled points to 1/100th pt used in winfonts.c and dviwindo.c */
/* revised 97/Sep/16 */

unsigned long mappoints(unsigned long fsk) {
	unsigned long x, y, z;
/*	d * (dvi_num / 254000) * ((20 * 72 * 1024) / dvi_den) * 5 */
/*	dvi_num = 25400000, dvi_den = 473628672 */
	x = MulDiv(fsk * 5, dvi_num, dvi_den);
/*	y = x + (x * 3 + 399) / 800; */			/* (7227 / 7200); */
	y = MulDiv(x, 7227, 7200);
	z = MulDiv(y, 20 * 72, 254000); 
/*	{
		sprintf(debugstr, "fsk %lu x %lu y %lu z %lu\n", fsk, x, y, z);
		if (bDebug > 1) OutputDebugString(debugstr);
	} */
	return z;
}

#ifdef IGNORED
unsigned int mappoints(unsigned long fsk) {
	unsigned int size;
/*				revised to get two decimal points 97/Apr/10 */
	setscale(0); 
/*	size = (unsigned int) mapd((long) fs[k]); */
	size = (unsigned int) (mapd((long) fs[k] * 5));
	size = size + (size * 3 + 399) / 800; /* (7227 / 7200); */
	setscale(wantedzoom);
/*				following similar to code in ShowFontInfo ... */
/*				size = (size + 1) / 2; */	/* from TWIPS to 1/10 pt */
/*				size = size * 5; */			/* from TWIPS to 1/100 pt */
	return size;
}
#endif

unsigned int maxlistheight = 0;  /* saved initial height of IDC listbox */
unsigned int maxlistwidth = 0;   /* saved width of IDC listbox pixels */

/****************************************************************************/

/* DLGPROC lpFontsUsed=NULL; */	/* 1992/Nov/1 */

/* Problem: this may be called when some font information is not known yet */
/* such as ansifont[k] ... */

/****************************************************************************

	FUNCTION: FontsUsed(HWND, unsigned, WORD, LONG)

	PURPOSE: Modeless dialog box to display DVI font information

****************************************************************************/

/*BOOL CALLBACK FontsUsed(HWND hDlg, unsigned message, WORD wParam, LONG lParam) */
BOOL CALLBACK _export FontsUsed (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HMENU hMenu;
	TEXTMETRIC TextMetric;			/* use top-level version ? */
	LPMEASUREITEMSTRUCT MeasureItem;
	LPDRAWITEMSTRUCT DrawItem;
	HDC hDC;
	HWND hDlgList;
	DWORD oldFore;
/*	long oldBack; */
	char *s, *t, *sencode;
	int xText, yText;
	int index;
	int k, ff, validflag, taggedflag;
	int selectflag, selectchange;
/*	int focusflag, focuschange; */
	int highlightchange, drawflag;
	unsigned int size; 
	int intsize, fracsize;
/*	int intsized, fracsized; */
	int count, loctaggedfont;
	int cx, cy;
	RECT ListBoxRect;
	POINT ULCorner, LRCorner;
	unsigned itemheight, totalheight;
	WORD id, cmd;
	RECT WinRect;				/* window rectangle - MOVE or RESIZE */	

/*	if (bDebug > 1) {
		if (message != WM_DRAWITEM && message != WM_SETCURSOR &&
		   message != WM_NCHITTEST && message != WM_NCMOUSEMOVE) {
			sprintf(debugstr, "msg %X (%d) wParam %X (%d) lParam %X (%d)",
				message, message, wParam, wParam, lParam, lParam);
			OutputDebugString(debugstr);
		}
	} */
	switch (message) {
		case WM_INITDIALOG:
			(void) ShowWindow(hDlg, SW_HIDE);	/* hide while messing */
/*	First delete any strings from before - if any */
			(void) SendDlgItemMessage(hDlg, IDC_USEDLIST, 
										LB_RESETCONTENT, 0, 0L);
			if (hWidths == NULL) return(TRUE);	/* sanity check */
/* Then attempt adjust box size to number of items present */
			hDC = GetDC(hDlg);
			(void) GetTextMetrics (hDC, &TextMetric); /* Sys font metrics */
			(void) ReleaseDC(hDlg, hDC);
			itemheight = (unsigned int) (TextMetric.tmHeight * 11) / 10;
//			itemheight = (unsigned int) (TextMetric.tmHeight * 12) / 10; // ???
//			totalheight = itemheight * (fnext + 1);	/* number of fonts + 1 */
//			totalheight = itemheight * (fnext + 10); /* ??? */
			totalheight = itemheight * (fnext + 2);	/* heuristic adjustment ? */
			hDlgList = GetDlgItem(hDlg, IDC_USEDLIST);
			GetWindowRect(hDlgList, &ListBoxRect);
			ULCorner.x = ListBoxRect.left; ULCorner.y = ListBoxRect.top;
			LRCorner.x = ListBoxRect.right; LRCorner.y = ListBoxRect.bottom;
			ScreenToClient(hDlgList, &ULCorner); 
			ScreenToClient(hDlgList, &LRCorner);
			cx = LRCorner.x - ULCorner.x;
			cy = LRCorner.y - ULCorner.y; 
			if (maxlistheight == 0) maxlistheight = cy;	/* first time */
			if (totalheight < maxlistheight) cy = totalheight;
			else cy = maxlistheight;
			maxlistwidth = cx;
/* set size of list box */
			SetWindowPos(hDlgList, NULL, 0, 0, cx, cy, 
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
									
/* Then adjust window size to fit ListBox size */
			GetWindowRect(hDlgList, &ListBoxRect);
			ULCorner.x = ListBoxRect.left; ULCorner.y = ListBoxRect.top;
			LRCorner.x = ListBoxRect.right; LRCorner.y = ListBoxRect.bottom;
			ScreenToClient(hDlg, &ULCorner); 
			ScreenToClient(hDlg, &LRCorner);
			cx = LRCorner.x - ULCorner.x;
			cy = LRCorner.y - ULCorner.y;
			cx += 2 * GetSystemMetrics(SM_CXDLGFRAME);
			cy += GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYDLGFRAME);
/* set size of dialog box */
			SetWindowPos(hDlg, NULL, 0, 0, cx, cy, 
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
/* move to last saved position of this box */
			if (UsedxLeft != 0 && UsedyTop != 0) {	/* 1994/Mar/21 */
				SetWindowPos(hDlg, NULL, UsedxLeft, UsedyTop, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
			}
/* works, since item size IS known - since WM_ITEMSIZE called first! */

			if (bFileValid) {
				GrabWidths();				/* 99/Jan/12 */
				touchallfonts(hwnd);		/* 99/Jan/8 ??? */
				ReleaseWidths();			/* 99/Jan/12 */
			}

/*			oldmarkfont = markfont; */
			markfont = -1;

/*			insert dummy entry */ /* make sure it sorts first ! */
/*			sprintf(str, "%d\tFonts\t(-1)", fnext); *//* should sort first */
/*			sprintf(str, " %d\tFonts\t(-1)", fnext); */ /* 1992/June/8 */
			sprintf(str, " %d\tFonts\t\t\t\t(-1)", fnext); /* 1999/Jan/9 */
/*			sprintf(str, "Name\tSize\t(-1)"); */
			(void) SendDlgItemMessage(hDlg,
									  IDC_USEDLIST,
									  LB_ADDSTRING, 
									  0,	/* NULL, */
									  (LONG) (LPSTR) str);

/*	add fonts to list box */ /* do in order of TeX's font numbers (why?) */
			for (k = fnext-1; k >= 0; k--) { /* step through fonts in list */
/*	or use realfontnumber in winpslog.c ? */
				for (ff = 0; ff < MAXFONTNUMBERS; ff++) {
					if (finx[ff] ==	(short) k) break; /* 96/July/25 */
				}
/*				error if drop through with ff == MAXFONTNUMBERS ... */
				size = mappoints(fs[k]);
/*				intsize = (int) (size / 10); */
				intsize = (int) (size / 100);
/*				fracsize = (int) (size - ((unsigned int) intsize) * 10); */
				fracsize = (int) (size - ((unsigned int) intsize) * 100);
				{
					char *fname="UNKNOWN";
					char *sfname="UNKNOWN";
					if (fontname[k] != NULL) fname = fontname[k];
					if (subfontname[k] != NULL) sfname = subfontname[k];
					sprintf(str, "%s\t at ", fname);
					s = str + strlen(str);
				if (fracsize == 0)
/* 					sprintf(str, "%s\tat %3d pt\t(%d)", */
/*						fontname[k], intsize); */
					sprintf(s, "%3d", intsize);
				else if ((fracsize % 10) == 0)
/*					sprintf(str, "%s\tat %3d.%d pt\t(%d)",  */
/*						fontname[k], intsize, fracsize / 10, ff); */
					sprintf(s, "%3d.%d", 
							intsize, fracsize / 10);
/*				else sprintf(str, "%s\tat %3d.%02d pt\t(%d)",  */
/*						fontname[k], intsize, fracsize, ff); */
				else sprintf(s, "%3d.%02d", 
							intsize, fracsize);
				strcat(str, " pt\t");
				strcat(str, sfname);
/*				strcat(str, ","); */
				if (fontbold[k] || fontitalic[k]) strcat(str, ",");
				if (fontbold[k]) strcat(str, "BOLD");
				if (fontitalic[k]) strcat(str, "ITALIC");
/*				if (!fontbold[k] && !fontitalic[k]) strcat(str, "REGULAR"); */
				strcat(str, "\t");
				if (fontttf[k]) strcat(str, "TT");
				else strcat(str, " ");		/* ??? */
				strcat(str, "\t");
				if (ansifont[k]) {	/* imported from setupcharinfo dviwindo.c */
#ifdef USEUNICODE
					if ((szReencodingName != NULL) &&				
						(fontttf[k] && bUseNewEncodeTT) ||
						(!fontttf[k] && bUseNewEncodeT1))
						sencode = szReencodingName;
					else if (fontttf[k]) sencode = "ansinew";
#else
					if (fontttf[k]) sencode = "ansinew";
#endif
					else if ((szReencodingName != NULL) &&
							 bATMLoaded && hEncoding != NULL)
						sencode = szReencodingName;
					else sencode = "ansinew";
				}
				else sencode = " ";
				strcat(str, sencode); 
/*				s = str + strlen(str);
				sprintf(s, "%d", ansifont[k]); */
				strcat(str, "\t");
				s = str + strlen(str);
				sprintf(s, "(%d)", ff);
				(void) SendDlgItemMessage(hDlg,
										  IDC_USEDLIST,
										  LB_ADDSTRING, 
										  0, /* NULL, */
										  (LONG) (LPSTR) str);
				}
			}

			if (bShowUsedExposed != 0)
				(void) ShowWindow(hDlg, SW_SHOW);	/* show window again ? */
/*	how can we get the focus back form this modeless dialog box ? */
			return (TRUE); 		/* Indicates focus *not* set here to control */

/*	following happens before WM_INITDIALOG ! */ /* wParam contains item no */

		case WM_MEASUREITEM:
			MeasureItem = (LPMEASUREITEMSTRUCT) lParam;
			hDC = GetDC(hDlg);
			(void) GetTextMetrics (hDC, &TextMetric); /* Sys font metrics */
			(void) ReleaseDC(hDlg, hDC);
			MeasureItem->itemHeight = 
				(unsigned int) (TextMetric.tmHeight * 11) / 10; /* pixels */
//			    (unsigned int) (TextMetric.tmHeight * 12) / 10; /* ??? */
/* Also set MeasureItem->itemWidth ??? */
			MeasureItem->itemWidth = 
				(unsigned int) maxlistwidth;	/* pixels 1999/Jan/9 */
			break;

		case WM_DRAWITEM:
			DrawItem = (LPDRAWITEMSTRUCT) lParam;
			(void) GetTextMetrics (DrawItem->hDC, &TextMetric);
/* owner draw state */
			selectflag = DrawItem->itemState & ODS_SELECTED; 
/*			focusflag = DrawItem->itemState & ODS_FOCUS; */
/* owner draw action */
			selectchange = DrawItem->itemAction & ODA_SELECT;
/*			focuschange = DrawItem->itemAction & ODA_FOCUS; */
			drawflag = DrawItem->itemAction & ODA_DRAWENTIRE;

			if (selectchange != 0) {
				if (selectflag != 0) highlightchange=+1;	/* gaining */
				else highlightchange = -1;	/* loosing selection */
			}
			else highlightchange = 0;					/* no change */

			if (highlightchange < 0) markfont = -1; /* turn this one off */

/*			couldn't get DrawItem->itemData to work ... */
/*			(void) strcpy(str, (LPSTR) DrawItem->itemData); */

		    index = (int) DrawItem->itemID;
			validflag = (int) SendDlgItemMessage(hDlg, IDC_USEDLIST,
							(UINT) LB_GETTEXT, (WPARAM) index, 
								(LONG) (LPSTR) str);
			if (validflag == LB_ERR) break;			/* not valid item */

			ff = extractfontnumber(str);
/*			if (highlightchange > 0)  markfont = ff; */
			if (selectflag != 0)  markfont = ff;

			if (drawflag != 0 || highlightchange != 0) { 
/*				if (highlightchange < 0) */
				if (highlightchange < 0 && ff >= 0)		/* losing selection */
					InvertRect(DrawItem->hDC, &DrawItem->rcItem); 

				xText = DrawItem->rcItem.left + 3;
				yText = DrawItem->rcItem.top + 3;

/*				if (highlightchange > 0) */			/* turn this one on */
				if (selectflag != 0) {					/* turn this one on */
					oldFore = SetTextColor(DrawItem->hDC, RGB(0,0,0));  
/*					oldFore = SetTextColor(DrawItem->hDC, TextColor);*//* ? */
				}
				else if (bColorFont != 0) {
					if(ff >= 0) oldFore = ChangeColor(DrawItem->hDC, ff);
				}

/*				replace dummy entry */
/*				if (ff < 0) sprintf(str, "Name\tSize\tID"); */	/* 99/Jan/7 */
				if (ff < 0)	sprintf(str,
//						"TFM Name\tSize\tFace & Style\tTT\tEncode\tID");  
						"TFM File Name\tSize\tFace Name & Style\tTT\tEncode\tID");  
/*				if (ff < 0) {
					if (bDebug > 1 && taggedchar >= 0)
						sprintf(debugstr, "Name\tSize\tID (%d)", taggedchar);
					else sprintf(debugstr, "Name\tSize\tID");
					OutputDebugString(debugstr);
				} */ /* NO: can't mess with str ! */

				s = t = str;						/* TFM Name */
				if ((t = strchr(s, '\t')) != NULL) *t++ = '\0';
				(void) TextOut(DrawItem->hDC, xText, yText, 
					s, (int) strlen(s)); 
/*				xText += TextMetric.tmAveCharWidth * 11; */	/* narrow */
//				xText += TextMetric.tmAveCharWidth * 12;	/* 1997/Jan/7 */
				xText += TextMetric.tmAveCharWidth * 36;	// 2000 July 4
			
				if (t != NULL) s = t;				/* at size */
				if ((t = strchr(s, '\t')) != NULL) *t++ = '\0';	
				(void) TextOut(DrawItem->hDC, xText, yText, 
					s, (int) strlen(s)); 
/*				xText += TextMetric.tmAveCharWidth * 10; */ /* too narrow */
				xText += TextMetric.tmAveCharWidth * 12;   /* 1997/Jan/7 */

				if (t != NULL) s = t;				/* Face & Style */
				if ((t = strchr(s, '\t')) != NULL) *t++ = '\0';	
				(void) TextOut(DrawItem->hDC, xText, yText, 
							   s, (int) strlen(s)); 				
/*				xText += TextMetric.tmAveCharWidth * 24; */
/*				xText += TextMetric.tmAveCharWidth * 28; */
//				xText += TextMetric.tmAveCharWidth * 30;
				xText += TextMetric.tmAveCharWidth * 42;

				if (t != NULL) s = t;				/* TT or not */
				if ((t = strchr(s, '\t')) != NULL) *t++ = '\0';	
				(void) TextOut(DrawItem->hDC, xText, yText, 
							   s, (int) strlen(s)); 
				xText += TextMetric.tmAveCharWidth * 4;

				if (t != NULL) s = t;				/* encoding */
				if ((t = strchr(s, '\t')) != NULL) *t++ = '\0';	
				(void) TextOut(DrawItem->hDC, xText, yText, 
							   s, (int) strlen(s));
				xText += TextMetric.tmAveCharWidth * 10;

				if (t != NULL) s = t;				/* ID */
				if ((t = strchr(s, '\t')) != NULL) *t++ = '\0';	
				if (ff >= 0) sprintf(s, "%d", ff);	/* change ... */
				(void) TextOut(DrawItem->hDC, xText, yText, 
							   s, (int) strlen(s));
				xText += TextMetric.tmAveCharWidth * 4;

/*	oldFore may not be initialized ??? */
				if (selectflag > 0) {			/* back to old color */
					(void) SetTextColor(DrawItem->hDC, oldFore);
				} 
				else if (bColorFont != 0) {
					if (ff >= 0) (void) SetTextColor(DrawItem->hDC, oldFore);
				}

/*				if (selectflag != 0) */
				if (selectflag != 0 && ff >= 0)		/* trial */
/*				if (highlightchange > 0) */			/* this one highlighted */
					InvertRect(DrawItem->hDC, &DrawItem->rcItem);

				if (highlightchange != 0) InvalidateRect(hwnd, NULL, TRUE); 
			}
			break;

/*		case WM_DELETEITEM:
			break; */

		case WM_CHANGESEL: 			/* work in progress here ! */
			loctaggedfont = (int) wParam;
			/* figure out how many items */
			count = (int) SendDlgItemMessage(hDlg, IDC_USEDLIST, 
										LB_GETCOUNT, 0, 0L);
			/* walk through them and retrieve font number */
			taggedflag = 0;
			for (k = count-1; k >= 0; k--) {
				validflag = (int) SendDlgItemMessage(hDlg, IDC_USEDLIST,
							(UINT) LB_GETTEXT, (WPARAM) k, 
								(LONG) (LPSTR) str);
				if (validflag == LB_ERR) continue;	/* not a valid item */
				ff = extractfontnumber(str);
			/* if a match then change selection */
				if (ff == loctaggedfont) {
					(void) SendDlgItemMessage(hDlg, IDC_USEDLIST, 
						LB_SETCURSEL, (WPARAM) k, 0L);
					taggedflag = 1;
					break;
				}
			}
			if (taggedflag == 0) {
/*	if none match then set selection to none */
/*	following does weird thing: wParam = -1 ? => 0XFFFF */
				(void) SendDlgItemMessage(hDlg, IDC_USEDLIST, 
					LB_SETCURSEL, 0xFFFF, 0L);
			}
			break;

		case WM_MOVE:					/* 1994/Mar/21 */
			GetWindowRect(hDlg, &WinRect);
			UsedxLeft = WinRect.left;
			UsedyTop = WinRect.top;
			break;

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
/*			if (bDebug > 1) {
				sprintf(debugstr, "id %X cmd %X wParam %X lParam %X",
						id, cmd, wParam, lParam);
				OutputDebugString(debugstr);
			} */

			switch (id) {
/* requires LBS_NOTIFY style to get following messages */
/* Presently IDC_USERLIST == LB_SETITEMDATA --- some problem if changed 1999/Jan/9 */
/*				case IDC_USEDLIST: */	
				case LB_SETITEMDATA:
					if (cmd == LBN_SELCHANGE || cmd == LBN_DBLCLK) {
						index = (int) SendDlgItemMessage(hDlg, 
							IDC_USEDLIST, LB_GETCURSEL, 0, 0L);
						if (index != LB_ERR) {
							(void) SendDlgItemMessage(hDlg, IDC_USEDLIST,
								(UINT) LB_GETTEXT, (WPARAM) index, 
								(LONG) (LPSTR) str);
/*							if (bDebug > 1) OutputDebugString(str); */
/*							fracsize = 0;
							(void) sscanf(str, "%s at %d.%d", 
								   str, &intsized, &fracsized);	*/
						}
						else {
							if (bDebug > 1) OutputDebugString("LB_ERR");
							markfont = -1;
						}
					}
					if (cmd == LBN_SELCHANGE) { /* selection changed */

					}
					else if (cmd == LBN_DBLCLK) {	/* double clicked */
/*						if (bDebug > 1)	OutputDebugString("Double Click"); */
/* parse: TFM name \t at xxxpt \t FACE,Style \t \t \t */						
						if (index == 0) return(TRUE);	/* dummy index */
						s = str;
						for (k = 0; k < 2; k++) {
							if ((s = strchr(s, '\t')) == NULL) return(TRUE);
							s++;
						}
						if ((t = strchr(s, '\t')) == NULL) return(TRUE);
						else *t = '\0';
						if (strstr(t+1, "TT") != NULL) testwantttf = 1;
						else testwantttf = 0;		/* needed? ignored? */
/*						if (bDebug > 1) OutputDebugString(s); */
						if (strstr(s, "UNKNOWN") != NULL) return(TRUE);
						if (strstr(s, "BOLD") != NULL) bCheckBoldFlag = 1;
						else bCheckBoldFlag = 0;
						if (strstr(s, "ITALIC") != NULL) bCheckItalicFlag = 1;
						else bCheckItalicFlag = 0;						
						if ((t = strchr(s, ',')) != NULL) *t = '\0';
						strcpy(TestFont, s);
/*						SendMessage(hwnd, WM_COMMAND, IDM_SELECTFONT, 0L); */
						SendMessage(hwnd, WM_COMMAND, IDM_SELECTFONT, 1L);
						SetFocus(hwnd);		/* give focus back to main */
					}
					return(TRUE);
					
				case IDCANCEL:
					goto close;
					break;
					
				default:
					return(FALSE); /* ? */
			}

/* end of the WM_COMMAND case */			

		case WM_CLOSE:
close:
			(void) DestroyWindow(hDlg); 
			return(TRUE); 
/*			break; */
		
		case WM_DESTROY:
			bShowUsedFlag = 0;
			bShowUsedExposed = 0;
			hMenu = GetMenu(hwnd);
			(void) CheckMenuItem (hMenu, IDM_FONTSUSED, MF_UNCHECKED);
			if (markfont >= 0) {
				InvalidateRect(hwnd, NULL, TRUE); 
				markfont = -1;
			}
			return(TRUE); 
/*			break; */

/*		case WM_GETDLGCODE:		
			return  DLGC_BUTTON | DLGC_DEFPUSHBUTTON | DLGC_HASSETSEL |
					DLGC_RADIOBUTTON | DLGC_STATIC | DLGC_UNDEFPUSHBUTTON |
					DLGC_WANTARROWS | DLGC_WANTTAB; */ /* experiment */

		default:
				break;	/* ??? */
	}
	return (FALSE);
}

/****************************************************************************

	FUNCTION: ShowFontsUsed(HWND)

	PURPOSE: Create dialog box to show information about DVI fonts

****************************************************************************/

/* return handle to dialog box ? */

void ShowFontsUsed (HWND hWnd) {
/*	DLGPROC lpFontsUsed; */ /* NEED STATIC ! */

/*	hFntBox = CreateDialog(hInst, "FontsUsed", hWnd, lpFontsUsed); */
/*	hFntBox = CreateDialog(hInst, "FontsUsed", hWnd, (DLGPROC) FontsUsed); */
	hFntBox = CreateDialog(hInst, "FontsUsed", hWnd, FontsUsed); 
	SetFocus(hWnd);				/* get focus back to main window ? 1999/Jan/9 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int TagFont(HWND hWnd, int curx, int cury) */
int TagFont(HWND hWnd, int curx, int cury, int control) {
	HDC hDC;
/*	HWND hFocus; */
	POINT steparr[1];
	HCURSOR hSaveCursor;		/* shadow global version ? 98/Mar/23 */

/*  need to reopen input file */
	if (ReOpenFile(hWnd) < 0) return -1;		/* failed to reopen */

	if (bDebug > 1) OutputDebugString("TagFont");
	
	bScanFlag = 1;				/* prevent repaint of screen 95/Dec/21 */
								/* do before GetDC and GrabWidths */
	hDC = GetDC(hWnd);			
	if (bCopyFlag == 0)			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS);		/* set unit to twips */
	bHourFlag = 1;							/* 1995/Dec/21 ? */
	hSaveCursor = SetCursor(hHourGlass); 	/* 1995/Dec/21 ? */
	bEnableTermination = 0;					/* 1995/Dec/21 ? */
/*	bHourFlag = 1; */
/*	hSaveCursor = SetCursor(hHourGlass); */
/*	bEnableTermination = 0; */
	bTagFlag = 1;
	taggedfont = -1;
/*	if (pagecurrent(dvipage) == 0)  {
		current = dvistart;	 dvipage = 1;
	} */

/*	GrabWidths(); */
/*	compute logical coordinates from window coordinates */
	steparr[0].x = curx;  steparr[0].y = cury;
	(void) DPtoLP(hDC, steparr, 1);
	tagx = steparr[0].x;  tagy = steparr[0].y; 
/*	sprintf(str, "lx %d ly %d", tagx, tagy);
	winerror(str); */
	GrabWidths();
/*	sigh, may need to do both pages of two page spread */
/*	bScanFlag = 1; */				/* prevent repaint of screen 95/Dec/21 */
	(void) scandvifile(hFile, hDC, 0);	
/*	bScanFlag = 0; */				/* allow repaint of screen 95/Dec/21 */
/*	(void) scandvipage(hFile, hDC, 0);	*/	/* for now */
	ReleaseWidths(); 

	bTagFlag = 0;

	if (bKeepFileOpen == 0) {
/*		if (hFile < 0) winerror("File already closed"); */
/*		if (hFile < 0) wincancel("File already closed"); */
		if (hFile == HFILE_ERROR) {
/*			wincancel("File already closed (no hit)"); */
			sprintf(debugstr, "%s (%s)\n", "File already closed", "tag font");
			if (bDebug) {
				if (bDebug > 1) OutputDebugString(debugstr);
				else wincancel(debugstr);
			}
		}
		else (void) _lclose (hFile);			/* 1992/Nov/6 */
/*		hFile = -1; */
		hFile = HFILE_ERROR;
	}

	bHourFlag = 0;						/* 1995/Dec/21 ? */
	(void) SetCursor(hSaveCursor);		/* 1995/Dec/21 ? */
/*	bHourFlag = 0; */
/*	(void) SetCursor(hSaveCursor); */
	(void) ReleaseDC(hWnd, hDC);
	bScanFlag = 0;				/* allow repaint of screen 95/Dec/21 */
								/* do after ReleaseDC and ReleaseWidths */

/*	if (bShowUsedFlag != 0)	 */
/*	if (bShowUsedFlag != 0 && control == 0) */		/* 1994/Mar/8 */
	if (bShowUsedFlag != 0 && control == 0 && taggedfont >= 0)	/* 1998/Nov/8 */
		(void) SendMessage(hFntBox, WM_CHANGESEL, (WPARAM) taggedfont, 0L);
	if (taggedfont < 0) return -1;		/* failed */
/*	sprintf(str, "Tagged font is %d", taggedfont);
	winerror(str); */
/*	now send message to dialog box ! */
	return taggedfont; /* OK */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* if dummy entry selected in FontsUsed, maybe don't highlight it ? */
/* and maybe set selection to -1 i.e. no selection ? */

/* Helv => MS Sans Serif */ /* Tms Tmn => MS Serif */ /* Windows 3.1 */

/* GetOutlineTextMetrics => OUTLINETEXTMETRICS */ /* Windows 3.1 */

/* GetRasterizerCaps => whether TrueType is installed */

/* GetCharABCWidths => left side-bear, character width, right side-bear */

/* EnumFontFamilies */  /* GetGlyphOutline */ /* GetTextExtentEx */

/* In ATM at least: */
/* Ascent = FontBBox.yur */
/* Descent = - FontBBox.yll */
/* Height = Ascent + Descent */
/* InternalLeading = max (0, Height - 1000) */

/* Try and deal with printer crock - avoid GetWidth */

/* switch from TEXTMETRIC to NEWTEXTMETRIC structures at least for EnumFonts*/

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define WANTATMCODE TRUE

#ifdef WANTATMCODE

/* char *standard[] = {
"grave", 	"acute", 	"circumflex", 	"tilde", 	"macron", 	
"breve", 	"dotaccent", 	"dieresis", ""	,		
"ring", 	"cedilla", 	"",			
"hungarumlaut", 	"ogonek", 	"caron", 	
}; */

/* New stuff for producing unencoded character output in AFM file */

/* In GlyphEnum, lpChar is the glyph name; */
/* CharCode & CharNum are codes in current encoding, or -1 if not encoded; */
/* CharNum == CharCode, unless character has repeat encoding, */
/* in which case CharNum gives earlier appearance of same glyph. */
/* CharFlag is apparently always -1; */
/* lpEncode is the argument passed in to ATMGetGlyphList. */

/* Encoded glyphs are enumerated in numeric order in current encoding. */
/* Unencoded glyphs are enumerated in order they appear in PFB font file (?) */

/* space		32	32	-1 */
/* circumflex	146	94	-1 */
/* .notdef		-1	-1	-1 */

/* change CALLBACK to ATMCALLBACK ? remove _export ? */

/*	BOOL CALLBACK _export GlyphEnum (LPSTR lpChar, int CharCode,
	int CharNum, int CharFlag, LPSTR lpEncoding); */

BOOL ATMCALLBACK GetGlyphProc (LPSTR glyphName, short currCodepoint,
						   short normalCodepoint, short fontCodepoint, DWORD userData)
/* BOOL CALLBACK _export GetGlyphProc (LPSTR glyphName, int currCodepoint,
							int normalCodepoint, int fontCodepoint, LPSTR lpEncoding) */
/*	7 WORD args */
{
//	LPSTR lpEncoding = (LPSTR) userData;
	LPSTR *lpEncoding = (LPSTR *) userData;
	
	if (strcmp(glyphName, ".notdef") == 0) return TRUE; /* ignore .notdef */

	if (currCodepoint < 0) nunencode++;
	else if (currCodepoint == normalCodepoint) nencoded++;
/*	else nrepeated++; */						/* repeated encodings */

	if (!bListUnencoded) {						/* list all encoded glyphs */
		if (currCodepoint >= 0 && currCodepoint < MAXCHRS) {
//			strcpy(lpEncoding + currCodepoint * MAXCHARNAME, glyphName);
			if (lpEncoding[currCodepoint] != NULL) free(lpEncoding[currCodepoint]);
			lpEncoding[currCodepoint] = zstrdup(glyphName);
			nglyph++;
		}
	}
	else {								/* list all unencoded glyphs */
		if (currCodepoint < 0) {		/* is it presently unencoded ? */
//			strcpy(lpEncoding + nglyph * MAXCHARNAME, glyphName);
			if (lpEncoding[nglyph] != NULL) free(lpEncoding[nglyph]);
			lpEncoding[nglyph] = zstrdup(glyphName);
			nglyph++;
/*			if (bSkipASE && nglyph == 193) nglyph = 208; */
			if (nglyph >= MAXCHRS) return FALSE;	/* no more space */
/*			currently limited to dealing with MAXCHRS unencoded glyphs */
		}
	}
	return TRUE;				/* when we want more */
}	/* fontCodepoint unreferenced */

/*	Get glyph list for Type 1 font when linked to ATM */

/*	atmpriv.h claims: currCodepoint		char code in current encoding */
/*	atmpriv.h claims: normalCodepoint	char code in normal encoding */
/*	atmpriv.h claims: normalCodepoint	-1 if ASE, otherwise currCodePoint */

// int GetGlyphList(HDC hDC, LPSTR lpEncoding, int unencodedflag) {
int GetGlyphList(HDC hDC, LPSTR *lpEncoding, int unencodedflag) {
	int ret;
/*	int nVersion=3; */
	int k;
/*	int m; */
//	LPSTR lpGlyph;

#ifdef DEBUGATM
	if (bDebug > 1) OutputDebugString("GetGlyphList\n");
#endif

//	lpGlyph=lpEncoding;
	bListUnencoded = unencodedflag;				/* set global for call back */
	for (k=0; k < MAXCHRS; k++) {				/* reset the encoding */
/*		*(lpEncoding + k * MAXCHARNAME) = '\0'; */
//		*lpGlyph = '\0';
		if (lpEncoding[k] != NULL) free(lpEncoding[k]);
		lpEncoding[k] = NULL;
//		lpGlyph += MAXCHARNAME;
	}
	nglyph = 0;
	nencoded = 0;
	nunencode = 0;	
	if (bATMLoaded == 0) return 0;				/* error ! --- sanity check! */
/*	ret = MyATMGetGlyphList (nVersion, hDC, (FARPROC) GlyphEnum, lpEncoding); */
	ret = MyATMGetGlyphList (BD_VERSION, hDC, GetGlyphProc, (DWORD) lpEncoding);
/*	ret = MyATMGetGlyphList (BD_VERSION, hDC, (FARPROC) GetGlyphProc, lpEncoding); */

//	for (k = 0; k < MAXCHRS; k++) {
//		if (lpEncoding[k] == NULL) lpEncoding[k] = zstrdup("");
//	}

/*	if (bSkipASE && unencodedflag) {
		lpGlyph = lpEncoding + 193 * MAXCHARNAME;
		for (k = 193; k < 208; k++) {
			if (strcmp(standard[k-193], "") != 0) {
				for (m = 0; m < k; m++) {
					if (strcmp(lpEncoding + m * MAXCHARNAME,
						standard[k-193]) == 0) {
						*(lpEncoding + m * MAXCHARNAME) = '\0'; 
					}
				}
			} 
			strcpy(lpGlyph, standard[k-193]);
			lpGlyph += MAXCHARNAME;
		}
		if (nglyph >= 208) nglyph -= 15;
	} */		/* try and force accents into StandardEncoding Position ? */
#ifdef DEBUGATM
	if (bDebug > 1) {
		wsprintf(debugstr, "GetGlyphList: Encoded %d Unencoded %d Total %d Nglyph %d\n",
			nencoded, nunencode, nencoded + nunencode, nglyph);
		OutputDebugString(debugstr);
	}
#endif
#ifdef IGNORED
	if (bDebug && unencodedflag) {
		for (k = 0; k < MAXCHRS; k++) {
			if (lpEncoding[k] == NULL) continue;
			if (*lpEncoding[k] == '\0') continue;
			wsprintf(debugstr, "%d\t%s\n", k, lpEncoding[k]);
			OutputDebugString(debugstr);
		}
	} 					/* show the accumulated glyph list */
#endif
	return ret;
}

#endif

/* ATMFORCEEXACTWIDTH has no effect on printer DC */
/* ATMFORCEEXACTWIDTH not clear if we want/need this for metrics */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int bTypeOneFlag=1;
int bTrueTypeFlag=0;		/* !bTypeOneFlag */
int bTextFontFlag=1;
int bSymbolFlag=0;			/* ! bTextFontFlag */
/* int bDontStopFlag=0; */	/* not implemented/checked out */
int bNewOnlyFlag=1;	

/* New code to make TFM files for all text fonts */

BOOL CALLBACK _export TFMMakeDlg(HWND hDlg, UINT message,
								 WPARAM wParam, LPARAM lParam) {
	WORD id;
	int bMakeTFM = bWriteAFM-1;
	char *s;

	switch (message) { 

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

			case IDOK:
				bTypeOneFlag = (int) SendDlgItemMessage(hDlg, 
					ID_TYPE1, BM_GETCHECK, 0, 0L);
				bTrueTypeFlag = (int) SendDlgItemMessage(hDlg, 
					ID_TRUETYPE, BM_GETCHECK, 0, 0L);
				bTextFontFlag = (int) SendDlgItemMessage(hDlg, 
					ID_TEXTFONT, BM_GETCHECK, 0, 0L);
				bSymbolFlag = (int) SendDlgItemMessage(hDlg, 
					ID_SYMBOL, BM_GETCHECK, 0, 0L);
/*				bDontStopFlag = (int) SendDlgItemMessage(hDlg, 
					ID_DONTSTOP, BM_GETCHECK, 0, 0L); */
				bNewOnlyFlag = (int) SendDlgItemMessage(hDlg, 
					ID_NEWONLY, BM_GETCHECK, 0, 0L);
				EndDialog(hDlg, 1);
				return (TRUE);

			case IDCANCEL:
				EndDialog(hDlg, 0);
				return (TRUE);

			default:
				return (FALSE); /* ? */
			}
		
/*		break; */

/* end of WM_COMMAND case */		

		case WM_INITDIALOG:						/* message: initialize	*/
			if (bMakeTFM == 0) {
				char buff[64];
				if (GetWindowText(hDlg, buff, sizeof(buff)) > 0) { 
					if ((s = strstr(buff, "TFM")) != NULL) {
						*s = 'A';
						SetWindowText(hDlg, buff);
					}
				}	/* 1995/July/25 */
			}
			(void) SendDlgItemMessage(hDlg, ID_TYPE1, BM_SETCHECK, 
				(WPARAM) bTypeOneFlag, 0L);
			(void) SendDlgItemMessage(hDlg, ID_TRUETYPE, BM_SETCHECK, 
				(WPARAM) bTrueTypeFlag, 0L);
			(void) SendDlgItemMessage(hDlg, ID_TEXTFONT, BM_SETCHECK, 
				(WPARAM) bTextFontFlag, 0L);
			(void) SendDlgItemMessage(hDlg, ID_SYMBOL, BM_SETCHECK, 
				(WPARAM) bSymbolFlag, 0L);
/*			(void) SendDlgItemMessage(hDlg, ID_DONTSTOP, BM_SETCHECK, 
				(WPARAM) bDontStopFlag, 0L); */
			(void) SendDlgItemMessage(hDlg, ID_NEWONLY, BM_SETCHECK, 
				(WPARAM) bNewOnlyFlag, 0L);
			if (bMakeTFM == 0) 
				(void) ShowWindow(GetDlgItem(hDlg, ID_NEWONLY), SW_HIDE);
			return (TRUE); /* Indicates the focus is *not* set to a control */

			default:
				return(FALSE); /* ? */
	}
/*	return FALSE; */					/* message not processed */
}

/* New code to make TFM files for all text fonts */

BOOL CALLBACK _export TFMAskDlg(HWND hDlg, UINT message,
								WPARAM wParam, LPARAM lParam) {
	WORD id;
	int bMakeTFM = bWriteAFM-1;
	char *s;

	switch (message) { 

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

			case IDYES:
				EndDialog(hDlg, IDYES);
				return (TRUE);

			case IDNO:
				EndDialog(hDlg, IDNO);
				return (TRUE);

			case IDCANCEL:
				EndDialog(hDlg, IDCANCEL);
				return (TRUE);

			default:
				return (FALSE); /* ? */
			}
		
/*		break; */

/* end of WM_COMMAND case */		

		case WM_INITDIALOG:						/* message: initialize	*/
			if (bMakeTFM == 0) {
				char buff[64];
				if (GetWindowText(hDlg, buff, sizeof(buff)) > 0) { 
					if ((s = strstr(buff, "TFM")) != NULL) {
						*s = 'A';
						SetWindowText(hDlg, buff);
					}
				}	/* 1995/July/25 */
			}
			SetDlgItemText(hDlg, ID_TYPEFACE, (LPSTR) TestFont);
			SetDlgItemText(hDlg, ID_FILENAME, (LPSTR) str); /* file name */
			if (bCheckSymbolFlag)
				SetDlgItemText(hDlg, ID_ENCODING, (LPSTR) "numeric");
/*			else if (bTypeOneFlag && szReencodingName != NULL) */
/*			else if (bTypeOneFlag && *szReencodingName != '\0') */ /* 98/Jul/10 */
			else if (bTypeOneFlag && (szReencodingName != NULL)) /* 98/Dec/25 */
				SetDlgItemText(hDlg, ID_ENCODING, (LPSTR) szReencodingName);
			else SetDlgItemText(hDlg, ID_ENCODING, (LPSTR) "ansinew");

/*			setup bold and italic check marks */
			if (bCheckItalicFlag) {							/* 95/Mar/12 */
				if (bCheckBoldFlag) (void) SendDlgItemMessage(hDlg,
					ID_BOLDITALIC, BM_SETCHECK,  (WPARAM) 1, 0L);
				else (void) SendDlgItemMessage(hDlg, ID_ITALIC, BM_SETCHECK, 
						(WPARAM) 1, 0L);
			}
			else {
				if (bCheckBoldFlag) (void) SendDlgItemMessage(hDlg,
					ID_BOLD, BM_SETCHECK,  (WPARAM) 1, 0L);
				else (void) SendDlgItemMessage(hDlg, ID_REGULAR, BM_SETCHECK, 
						(WPARAM) 1, 0L);
			}

			(void) SendDlgItemMessage(hDlg, ID_TYPE1, BM_SETCHECK, 
				(WPARAM) bTypeOneFlag, 0L);
			(void) SendDlgItemMessage(hDlg, ID_TRUETYPE, BM_SETCHECK, 
				(WPARAM) bTrueTypeFlag, 0L);
			(void) SendDlgItemMessage(hDlg, ID_TEXTFONT, BM_SETCHECK, 
				(WPARAM) bTextFontFlag, 0L);
			(void) SendDlgItemMessage(hDlg, ID_SYMBOL, BM_SETCHECK, 
				(WPARAM) bSymbolFlag, 0L);
			return (TRUE); /* Indicates the focus is *not* set to a control */

			default:
				return(FALSE); /* ? */
	}
/*	return FALSE; */					/* message not processed */
}	/* lParam unreferenced */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* use this also in ShowFont etc ??? */

/* rewritten to use qsort 98/Dec/4 */
/* do we really need to preserve CharSet and PitchandFamily in sorting ? */

typedef struct faceinfo { 
	char FaceName[MAXFACENAME];
	BYTE CharSet;
	BYTE PitchAndFamily;
} FACEINFO, FAR *LPFACEINFO;

int _cdecl comparef(const void *favoid, const void *fbvoid) {
	const FACEINFO *fa, *fb;
	fa = (const FACEINFO *) favoid;
	fb = (const FACEINFO *) fbvoid;
	return strcmp((LPSTR) fa, (LPSTR) fb);
}

void SortFaces (void) {		/* qsort version */
	LPFACEINFO lpFaceInfo;
//	LPSTR lpFontName;
	int i;
	
/*	if (bDebug > 1) OutputDebugString("Sorting Faces\n"); */
	if (nFaceIndex == 0) {
		if (bDebug > 1) OutputDebugString("nFaceIndex == 0\n");
		return;		/* sanity check */
	}
/*	allocate FaceInfo and copy into FaceInfo */	
	lpFaceInfo = (FACEINFO *) LocalAlloc (LMEM_FIXED, nFaceIndex * sizeof(FACEINFO));
	if (lpFaceInfo == NULL) return;	/* no memory, don't sort */
//	lpFontName = lpFaceNames;
	for (i = 0; i < nFaceIndex; i++) {
//		lpFontName = lpFaceNames[i];
		if (lpFaceNames[i] != NULL) 
			strcpy(lpFaceInfo[i].FaceName, lpFaceNames[i]); 
		else {
//			sprintf(debugstr, "lpFaceNames[%d] = NULL?", i);
//			winerror(debugstr);			// debugging only
			strcpy(lpFaceInfo[i].FaceName, "");	// sanity check
		}
		lpFaceInfo[i].CharSet = CharSet[i];
		lpFaceInfo[i].PitchAndFamily = PitchAndFamily[i];
//		lpFontName += MAXFACENAME;
	} 
/*	if (bDebug > 1) OutputDebugString("Before qsort"); */
	qsort(lpFaceInfo, (unsigned int) nFaceIndex, sizeof(FACEINFO), comparef);

/*	if (bDebug > 1) OutputDebugString("After qsort"); */
/*	copy back from FaceInfo and free FaceInfo */
//	lpFontName = lpFaceNames;
	for (i = 0; i < nFaceIndex; i++) {
//		lpFontName = lpFaceNames[i];
//		strcpy(lpFontName, lpFaceInfo[i].FaceName); 
		if (lpFaceNames[i] != NULL) free(lpFaceNames[i]);
		lpFaceNames[i] = zstrdup(lpFaceInfo[i].FaceName); 
		CharSet[i] = lpFaceInfo[i].CharSet;
		PitchAndFamily[i] = lpFaceInfo[i].PitchAndFamily;
//		lpFontName += MAXFACENAME;
	} 
	LocalFree(lpFaceInfo);
}

/*	use this also in ShowFont etc ??? */ /* is this really needed ? */
/*	in case of multiple occurence all but the last string is freed */

void EliminateDuplicates(void) {
	int i=0, j=0;

	if (lpFaceNames == NULL) return;	/* sanity check */
	if (nFaceIndex == 0) return;		/* sanity check */

	for (;;) {
		if (j < i) {		// copy down
//			strcpy(lpFontNamej, lpFontNamei);
			if (lpFaceNames[j] != NULL) free(lpFaceNames[j]);
			lpFaceNames[j] = lpFaceNames[i];
			lpFaceNames[i] = NULL;
			CharSet[j] = CharSet[i];
			PitchAndFamily[j] = PitchAndFamily[i];
//			IsTTF[j] = IsTTF[i];
		}
		if (i == nFaceIndex-1) {
			j++;
			break;		// last entry
		}
		if (  lpFaceNames[j] == NULL ||		// can't happen
			  lpFaceNames[i+1] == NULL ||	// can't happen
			  strcmp(lpFaceNames[j], lpFaceNames[i+1]) != 0) {
			j++;			// they are *not* the same - go on
		}
		else {
			if (bDebug > 1) {
				sprintf(debugstr, "Duplicate Face Name (%d) %s", j, lpFaceNames[j]);
				OutputDebugString(debugstr);
			}
//			sprintf(debugstr, "Duplicate Face Name (%d) %s", j, lpFaceNames[j]);
//			winerror(debugstr);
		}
		i++;
//		if (i == nFaceIndex) break;
/*		if (bDebug > 1) {
			sprintf(debugstr, "j %d i %d n %d\n", j, i, nFaceIndex);
			OutputDebugString(debugstr);
		} */
	}

//	sprintf(debugstr, "nFaceIndex %d reduced to %d", nFaceIndex, j);
//	winerror(debugstr);
	if (j != nFaceIndex) {	//	reset to current high value
		if (bDebug > 1) {
			sprintf(debugstr, "nFaceIndex %d => %d", nFaceIndex, j);
			OutputDebugString(debugstr);
		}
		nFaceIndex = j;
	}
	WritePrivateProfileInt(achDiag, "Faces", nFaceIndex, achFile);
}

char *mathprefix[] = {
	"CMMI", "CMSY", "CMBSY", "CMEX", /* "CMMIB", */
	   "MTEX", "MTSY", "MTMI", "RMTMI", "LBM",
		   "MSAM", "MSBM", "EUF", "EUR", "EUS",
				"LASY", /* "LASYB", */
					""	/* termination */
};

int IsItMathFont(char *Face, char *File) {
	int k;
	
	if (strstr(Face, "Math") != NULL)
		return 1;						/* LucidaNewMath, Adobe	MathPi */
	if (strstr(File, "MATH") != NULL)
		return 1;						/* LMATH1, LMATH2, LMATH3 */
	for (k=0; k < 16; k++) {
		if (strcmp(mathprefix[k], "") == 0) break;
		if (strncmp(mathprefix[k], Face, strlen(mathprefix[k])) == 0)
			return 1;
	}
	return 0;
}

/* separated out 1995/July/20 */ /* maybe use common code to test for TFM ? */
/* returns zero when font is Synthetic or Alias */
/* can also return IDNO or IDCANCEL passed from user */
/* return -1 on SUCCESS ? */

int DoOneStyle (HWND hWnd, HDC hDC, char *szTeXFirst, int tfmflag) {
	int atmfontflag=0, truetypeflag=0;
	char fname[MAXFILENAME];
	int bFileExists=0;
	int flag=0;
	int err, ret;
/*	int c; */
	char *s;
#ifdef LONGNAMES
	HFILE hfile;
#else
	OFSTRUCT OfStruct;				/* for file exist test */
#endif

/*	check whether synthetic */ /* skip if so */
/*	bSyntheticFlag = IsSynthetic(TestFont, */
/*	if (bDebug > 1) OutputDebugString(" DoOneStyle\n"); */
	bSyntheticFlag = IsSynthetic(hDC, TestFont,	bCheckBoldFlag,
								 bCheckItalicFlag, (testwantttf != 0));  
	if (bDebug > 1) 
		if (bSyntheticFlag) OutputDebugString(" SYNTHETIC\n");

/*	if (bSyntheticFlag < 0) continue; */	/* alias */ 
	if (bSyntheticFlag < 0) return 0;		/* alias */
/*	if (bSyntheticFlag > 0) continue; */	/* synthetic */
	if (bSyntheticFlag > 0) return 0;		/* synthetic */
/* compute file name */	/* set up initial truetypeflag properly */
    truetypeflag = testwantttf;				/* 1995/April/15 ? */
/*	FileFromFace(fname, TestFont, &atmfontflag, &truetypeflag); */
	err = FileFromFace(hDC, fname, TestFont, &atmfontflag, &truetypeflag);
/*	if (err) continue; */			/* 1995/July/19 should not happen */
	if (err) return flag;			/* 1995/July/19 should not happen */
	strcpy(str, fname);				/* drop for Dialog Box to pick up */
	if (*str == '\0') {
		strcpy(fname, TestFont);				/* Desperation */
		BadStyleMessage(debugstr, TestFont, "F");
		winerror(debugstr);
/*		continue; */				/* should not happen */
		return flag;				/* should not happen */
	}
	if (truetypeflag) {				/* 1995/July/25 */
		StripTTVersion(str);		/* 1995/Aug/24 */
	}
	StripUnderScores(str);			/* 1995/Aug/24 */
/*	FileExists = 0; */
	if (tfmflag) {			/* 1995/Juy/19 maybe drop ... */
/*		Here szTeXFirst actually becomes full path and name of TFM file */
		s = szTeXFirst + strlen(szTeXFirst) - 1;
		if (*s != '\\') strcat(szTeXFirst, "\\");
		strcat(szTeXFirst, str);
		strcat(szTeXFirst, ".tfm");
/*		check whether TFM file already exists */
#ifdef LONGNAMES
/*		hfile = _lopen(szTeXFirst, READ); */
/*		hfile = _lopen(szTeXFirst, READ | OF_SHARE_DENY_NONE); */ /* 96/May/18 ? */
		hfile = _lopen(szTeXFirst, READ | OfExistCode); /* 96/May/18 ? */
		if (hfile == HFILE_ERROR) bFileExists = 0;
		else {
			_lclose(hfile);
			hfile = HFILE_ERROR;
			bFileExists = 1;
		}
#else
		if (OpenFile(szTeXFirst, &OfStruct, OF_EXIST) == HFILE_ERROR) 
			bFileExists = 0;
		else bFileExists = 1;
#endif
/*		if (bDebug > 1) {	
			sprintf(debugstr, "File %s Exist %d\n", szTeXFirst, bFileExists);
			OutputDebugString(debugstr);
		} */
/*		*(szTeXFirst+nlen) = '\0'; */			/* Truncate it again */
		if (bNewOnlyFlag && bFileExists) {
/*			*(szTeXFirst+nlen) = '\0';*/		/* Truncate it again */
/*			continue;	*/		/* ignore this one */
			return flag;		/* ignore this one */
		}
	} /* end of if tfmflag */
/*	present to user - unless in DONT STOP mode */
	if (bHourFlag) {
		bHourFlag = 0;
		(void) SetCursor(hSaveCursor);
	}
/*	It is probably not really wise to *not* interact with user here ... */
/*	For one thing they don't know the TFM file name then! */
/*	File Name is left in str for the dialog box to use ? */
/*		if (bDontStopFlag) flag = IDOK; else */
//	flag = DialogBox(hInst, "TFMAsk", hWnd, (DLGPROC) TFMAskDlg);
	flag = DialogBox(hInst, "TFMAsk", hWnd, TFMAskDlg);
	if (!bHourFlag) {
		bHourFlag = 1;
		hSaveCursor = SetCursor(hHourGlass);
	}
	if (flag == IDOK) flag = 0;			/* 95/July/20 */
/*	if (flag == IDNO) break; */			/* don't do this face */
	if (flag == IDNO) return flag;		/* don't do this face */
/*	if (flag == IDCANCEL) break; */		/* stop everything */
	if (flag == IDCANCEL) return flag;	/* stop everything */
/*	check on bDontStopFlag here ? */
	if (bSymbolFlag && IsItMathFont(TestFont, fname)) {
/*		"WARNING: Cannot make TFM file for math font" */
		FormatNameStyle (debugstr, TestFont,bCheckBoldFlag, bCheckItalicFlag);
		strcat(debugstr, "\n");
		strcat(debugstr, "Sure you want to make metric file for math font?");
		if (wincancel(debugstr)) {
			flag = IDNO;
/*			break; */
			return flag;
		}
	}
/*	check on bDontStopFlag STOP here ? */
/*	if (bFileExists) */
	if (bFileExists && tfmflag) {
/*		if (wincancel("WARNING: TFM file exists.  Overwrite?")) */
		sprintf(str, "WARNING: `%s' exists.\n\nOverwrite it?",
				szTeXFirst);
		if (wincancel(str)) {
			flag = IDNO;
/*			break; */
			return flag;
		}
	}
/*	Now set things up for calling AFMtoTFM */
/*	following applies only to Type 1 plain text fonts */
	if (hEncoding != NULL) bCheckReencode = 1;
/*	AFMtoTFM refers to TestFont, bCheckBoldFlag, bCheckItalicFlag */
/*		hDC = GetDC(hWnd); */			/* moved out 95/July/8 ??? */
/*	flag that want to call AFMtoTFM, and flag that its in batch mode */
/*		ret = WriteAFMFile(hWnd, hDC, 1, 1); */
	ret = WriteAFMFile(hWnd, hDC, tfmflag, 1);
	if (ret != 0) {		/* error in creating AFM file */
		if (wincancel("Do you want to continue?")) {
			flag = IDCANCEL;	/* terminate ? */
			return flag;
		}
	}
/*		else ntfms++; */
/*		*(szTeXFirst+nlen) = '\0'; */		/* Truncate it again */
/*		(void) ReleaseDC(hWnd, hDC); */ /* moved out 95/July/8 */
/*		Wait for AFMtoTFM to finish ? */
		return -1;			/* SUCCESS ??? */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int CALLBACK _export FontEnumProc(const ENUMLOGFONT FAR *,
									  const NEWTEXTMETRIC FAR*, int, LPARAM);

int CALLBACK _export FontEnumProcEx(const ENUMLOGFONTEX FAR *,
									  const NEWTEXTMETRICEX FAR*, int, LPARAM);

int WriteAllTFM (HWND hWnd, int tfmflag) {
	int flag=0;
	int ntfms=0;			/* how many TFMs actually written */
	int nfonts=0;			/* how many fonts considered (before check TFM) */
/*	int err=0; */
/*	int ret=0; */
/*	int ttfflag; */
	int k, nlen;
	char *s;
	int symbolflag, decorativeflag, dontcareflag;
/*	int atmfontflag, truetypeflag; */
/*	int bFileExists; */
	HDC hDC=NULL;
/*	char fname[MAXFILENAME]; */
	char szTeXFirst[MAXFILENAME];		/* first path on szTeXFonts */
	LOGFONT lf;
	
/*	if (bTestFlag) {				
		bDoUnencodedGlyphs=0;
		bDoReencodeKern=0;
	}
	else {
		bDoUnencodedGlyphs=1;
		bDoReencodeKern=1;
	} */							/* quick text hack 95/July/20 */

	if (SetupTeXFirst(szTeXFirst, szTeXFonts) < 0) return -1;
/*	Pick the first directory out of list of TEXFONTS dirs */
	nlen = strlen(szTeXFirst);	/* remember length ? */

/*	gather up bTypeOneFlag, bTrueTypeFlag, bDontStopFlag (global) */
//	flag = DialogBox(hInst, "TFMMake", hWnd, (DLGPROC) TFMMakeDlg);
	flag = DialogBox(hInst, "TFMMake", hWnd, TFMMakeDlg);

	if (flag == 0) return -1;	/* oops, user changed his/her mind */

	if (!bHourFlag) {			/* this make take a while so change cursor */
		bHourFlag = 1;
		hSaveCursor = SetCursor(hHourGlass);
	}
/*	Remember to reset the cursor again afterwards ! */

/*	if (hFaceNames == NULL) AllocFaceNames();
	lpFaceNames = GrabFaceNames(); 
	GetFaces(hWnd);
	SortFaces();
	EliminateDuplicates(); */
	SetupFaceNames(hWnd, 0);		/* use common code */

/*	Remember to ReleaseFaceNames() at end */
/*	if (bHourFlag) {
		bHourFlag = 0;
		(void) SetCursor(hSaveCursor);
	} */
/*	if (!bHourFlag) {
		bHourFlag = 1;
		hSaveCursor = SetCursor(hHourGlass);
	} */

/*	hDC = GetDC(hWnd); */			/* moved out here 1995/July/8 ??? */
									/* make sure to Release	DC 95/July/8 */

	if (hFaceNames == NULL) {
		winerror("NULL hFaceNames"); /* DEBUGGING ONLY */
		return -1;
	}
	if (lpFaceNames == NULL) {
		winerror("NULL lpFaceNames"); /* DEBUGGING ONLY */
		return -1;
	}
	for (k = 0; k < nFaceIndex; k++) {	/* step through all faces */
//		lpCurrentName = lpFaceNames + k * MAXFACENAME;
//		lpCurrentName = lpFaceNames[k];
		if (lpFaceNames[k] != NULL) strcpy(TestFont, lpFaceNames[k]);
		else strcpy(TestFont, "");		// sanity check
		if ((s = strchr(TestFont, '(')) != NULL) {	/* Is it TT font ? */
			testwantttf = 1;
			*(s-1) = '\0';		/* strip the (TT) */
		}
		else {
			testwantttf = 0;							/* no, its Type 1 */
			if (hEncoding != NULL) bATMShowRefresh = 1;	/* 95/July/20 */
		}
/*		testwantttf = IsTTF[k]; */
		if (bTypeOneFlag && testwantttf) continue;		/* ignore */
		if (bTrueTypeFlag && !testwantttf) continue;	/* ignore */
		testcharset = CharSet[k];
		testpitchandfamily = PitchAndFamily[k];
		if ((testcharset != ANSI_CHARSET && testcharset != DEFAULT_CHARSET))
			symbolflag = 1;
		else symbolflag = 0;
		if ((testpitchandfamily & 0xF0) == FF_DECORATIVE) decorativeflag = 1;
		else decorativeflag = 0;
		if ((testpitchandfamily & 0xF0) == FF_DONTCARE) dontcareflag = 1;
		else dontcareflag = 0;
/*		CharSet not ANSI or DEFAULT => symbol font */
		if (symbolflag) bCheckSymbolFlag = 1;
		else bCheckSymbolFlag = 0;
/*		For Type 1 fonts, Family == DECORATIVE => symbol / math font */
		if (!testwantttf && decorativeflag) bCheckSymbolFlag = 1;
		if (bDecorative) {	/* treat decorative TT font as non-text */
			if (testwantttf && decorativeflag) bCheckSymbolFlag = 1; 
		}
		if (bDontCare) {	/* treat dontcare TT font as non-text */
			if (testwantttf && dontcareflag) bCheckSymbolFlag = 1; 
		}
		if (bTextFontFlag && bCheckSymbolFlag) continue;
		if (bSymbolFlag && !bCheckSymbolFlag) continue;
/*	Work in progress 97/Feb/11 */
		hDC = GetDC(hWnd);				/* move in here ? */
		lf.lfCharSet = DEFAULT_CHARSET;
		strcpy(lf.lfFaceName, TestFont);
		lf.lfPitchAndFamily = 0;
		StyleBits = 0;
		(void) EnumFontFamiliesEx(hDC, &lf,	(FONTENUMPROC) FontEnumProcEx,
								  (LPARAM) 4, (DWORD) 0L);
		(void) ReleaseDC(hWnd, hDC);	/* move in here ? */

/*		StyleBits = 0;
		hDC = GetDC(hWnd);
		(void) EnumFontFamilies(hDC, (LPSTR) TestFont,
					(FONTENUMPROC) FontEnumProc, (LPARAM) 4);
		(void) ReleaseDC(hWnd, hDC); */

		if (bDebug > 1) {
/*			sprintf(str, "StyleBits %0X\n", StyleBits); */
			sprintf(debugstr, "%s\tStyles: %s%s%s%s\n",
					TestFont,
					(StyleBits & 1) ? "R " : "",
					(StyleBits & 4) ? "B " : "",
					(StyleBits & 2) ? "I " : "",
					(StyleBits & 8) ? "BI " : "");					
			OutputDebugString(debugstr);

		}
/*		Try all four styles regular, bold, italic, bold italic */
/*		Skip the ones not found by EnumFontFamilies for this Face */
		for (bCheckBoldFlag = 0; bCheckBoldFlag < 2; bCheckBoldFlag++) {
			for (bCheckItalicFlag = 0; bCheckItalicFlag < 2; bCheckItalicFlag++) {	 

				if (testwantttf == 0) {			/* 95/July/20 */
					if (hEncoding != NULL) bATMShowRefresh = 1;
				}
				if ((StyleBits &
					 (1 << (bCheckItalicFlag + bCheckBoldFlag * 2))) == 0)
					continue;					/* style does not exist */
				if (bDebug > 1) {
					sprintf (debugstr, "%s%s%s%s%s%s",
							 TestFont,
							 (!bCheckBoldFlag && !bCheckItalicFlag) ?
							 " REGULAR" : "", 
							 bCheckBoldFlag ? " BOLD" : "",
							 bCheckItalicFlag ? " ITALIC" : "",
							 bCheckSymbolFlag ? " SYMBOL" : "",
							 testwantttf ? " TT" : " Type 1");
					OutputDebugString(debugstr);
				}
				hDC = GetDC(hWnd);				/* move in here ? */
				if (hDC == NULL) {
					winerror("NULL hDC");
					flag = IDCANCEL;
					break;
				}
				if (hFaceNames == NULL) {
					winerror ("hFaceNames NULL");
					flag = IDCANCEL;
					break;
				}
/*				Attempt to split out to ease debugging */
				flag = DoOneStyle (hWnd, hDC, szTeXFirst, tfmflag);
				(void) ReleaseDC(hWnd, hDC);	/* move in here ? */
				if (bDebug > 1) {
					if (flag < 0) OutputDebugString(" --- OK\n");
					else OutputDebugString(" --- FAILED/SKIP\n");
				}
				if (flag != 0) nfonts++;			/* correct count ??? */
				if (flag < 0) ntfms++;				/* Succeeded in writing */
				*(szTeXFirst+nlen) = '\0'; 			/* Truncate it again */
				if (flag == IDNO) break;			/* don't do this face */
				if (flag == IDCANCEL) break;		/* stop everything */
			} /* end of loop over bCheckItalicFlag */
			if (flag == IDNO) break;			/* don't do this face */
			if (flag == IDCANCEL) break;		/* stop everything */
		} /* end of loop over bCheckBodFlag */
		if (flag == IDCANCEL) break;			/* stop everything */
	} /* end of for (k = 0; k < nFaceIndex; k++) */

/*	(void) ReleaseDC(hWnd, hDC); */		 /* moved out here 95/July/8 */

	if (bHourFlag) {
		bHourFlag = 0;
		(void) SetCursor(hSaveCursor);
	}

	bCheckBoldFlag = bCheckItalicFlag = 0;
/*	winerror("WriteALLTFM - ReleaseFaceNames");	*/ /* DEBUGGING */
	ReleaseFaceNames();
/*	if (flag != IDCANCEL) */
		sprintf(str, "Created %d %s file%s", ntfms,
			(bWriteAFM < 2) ? "AFM" : "TFM",
				(ntfms == 1) ? "" : "s");
		if (ntfms == 0) {
			if (nfonts == 0) strcat (str, "\n(No fonts of this type)");
			else if (bNewOnlyFlag != 0)
				strcat (str, "\n(TFMs already exist)");
		}
		if (flag == IDCANCEL) strcat(str, "\n(Incomplete Pass)");
		wininfo(str);

	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* compare null terminated strings from far to far - limit length */

/* int lstrncmp(const LPSTR s, const LPSTR t, int n) */

#define lstrncmp strncmp

#ifdef IGNORED
int lstrncmp(const char FAR s[], const char FAR t[], int n) {
	int c, d;
	if (n == 0) return 0;
	for (;;) {
		c = *s++; d = *t++;
		if (c == '\0' || d == '\0') break;
/*		else if (c == d) continue; */
		else if (c < d) return -1;
		else if (c > d) return 1;
		n--;
		if (n == 0) return 0;
	}
	if (c == d) return 0;
	else if (c < d) return -1;
	else if (c > d) return 1; 
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Attempt to go from Windows FaceName & Style to FullName for TT font */
/* So we can then look up FullName in WIN.INI to get file name of font ... */

/* In Windows 3.1 we have EnumFontFamilies */

/* Remember to also stick FontEnumProc in dviwindo.def */

/* This is specific for TrueType fonts except for c == 4 */

/* Here lParam set to indicate what the task we want it to perform: */
/* 0: Find font matches szFaceName & Style, return szFullName - FullFromFace */
/* 1: Find font matches szFullName, return szFaceName & Style - NOT USED */
/* 2: Find font matches szFullName initial, return szFaceName - NOT USED */
/* 3: Stuff szFaceName, Style and szFull into table - for SetupTTMapping */
/* 4: set bits in style word so can tell which styles exist for face */
/* uses szFaceName, szFullName, bBoldFlag & bItalicFlag */

/* case 0 from GetFaces and SetupTTMapping, case 3 from SetupTTMapping */
/* case 4 from WriteAllTFM */

/* all but 4 are only used with TrueType fonts */

/* work in progress 97/Feb/11 */
int CALLBACK _export FontEnumProcEx (const ENUMLOGFONTEX FAR *lpEnumLogFontEx,
	const NEWTEXTMETRICEX FAR *lpNewTextMetricEx,
								int nFontType, LPARAM lpData) {
	LPLOGFONT lpLogFont = (LPLOGFONT) lpEnumLogFontEx;		/* 95/July/8 */
	LPNEWTEXTMETRIC lpNewTextMetric = (LPNEWTEXTMETRIC) lpNewTextMetricEx;
	int n;
	int italicflag, boldflag;
	LPSTR lpFullCurrent;
	int choice = (int) lpData;

/*	if (bDebug > 1) {
		OutputDebugString(lpLogFont->lfFaceName);
		OutputDebugString("\t");
		OutputDebugString(lpEnumLogFont->elfFullName);
		OutputDebugString("\t");
		sprintf (debugstr, "FontType %d lpData %d Style %x\n",
				 nFontType, c, lpNewTextMetric->ntmFlags);
		OutputDebugString(debugstr);
	} */

/*	Ignore all but TrueType fonts at this point (except for usage number 4) */
/*	Do we want to change this in NT with ATM ? and W2K  ? */ 
/*	if (choice < 4) */
/*	if ((bWinNT5 == 0) && (choice < 4)) */
	if ((bWinNT == 0) && (choice < 4)) {		/* 98/Nov/23 */
		if ((nFontType & TRUETYPE_FONTTYPE) == 0) return(TRUE);
	}

	switch (choice) {
		case 0:						/* find FullName from FaceName */
/*			if (strcmp(lpEnumLogFont->lfFaceName, szFaceName) == 0) */
			if (strcmp(lpLogFont->lfFaceName, szFaceName) == 0) { 
/* now check for match in style information */
/* use lpEnumLogFont.lfItalic and lpEnumLogFont.lpWeight ? */
/* use lpNewTextMetrics.tmItalic and lpNewTextMetrics.tmWeight ? */
				if (lpNewTextMetric->ntmFlags & NTM_BOLD) {
					if (!bBoldFlag) return (TRUE);
				}
				else if (bBoldFlag) return (TRUE);
				if (lpNewTextMetric->ntmFlags & NTM_ITALIC) {
					if (!bItalicFlag) return (TRUE);
				}
				else if (bItalicFlag) return (TRUE);
				if (lpNewTextMetric->ntmFlags & NTM_REGULAR) {
					if (bItalicFlag || bBoldFlag) return (TRUE);
				}
				else if (!bItalicFlag && !bBoldFlag) return (TRUE);
/* OK, Found match with Face Name and Style, return FullName in szFullName */
				strcpy(szFullName, (char *) lpEnumLogFontEx->elfFullName);
#ifdef DEBUGFONTSELECT
				if (bDebug > 1) {
					sprintf(debugstr, "Face: %s,%s%s%s Full: %s\n",
							szFaceName,
							(!bBoldFlag && !bItalicFlag) ? "REGULAR" : "",
							bBoldFlag ? "BOLD" : "",
							bItalicFlag ? "ITALIC" : "",
							szFullName);
					OutputDebugString(debugstr);
				}
#endif
				return (FALSE);			/* zero to stop enumerating */
			} 
			break;

		case 1:	/* FullName => FaceName + Style NOT USED ANYMORE */
			if (strcmp(szFullName, (char *) lpEnumLogFontEx->elfFullName) == 0) {
/*				strcpy(szFaceName, lpEnumLogFont->lfFaceName); */
				strcpy(szFaceName, lpLogFont->lfFaceName);
				if (lpNewTextMetric->ntmFlags & NTM_BOLD) bBoldFlag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_ITALIC) bItalicFlag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_REGULAR) {
					bBoldFlag = 0;
					bItalicFlag = 0;
				}
				nFullName = 1;				/* indicate successful match */
				return (FALSE);				/* zero to stop enumerating */
			}
			break;

		case 2:	/* FullName (initial part) => FaceName + Style NOT USED */
			n = strlen((char *) lpEnumLogFontEx->elfFullName);
			if (n < nFullName) return (TRUE);
			if (strcmp(szFullName, (char *) lpEnumLogFontEx->elfFullName) == 0) {
				strcpy(szFaceName, lpLogFont->lfFaceName);
				nFullName = n;
				return (FALSE);			/* stop, found exact match */
			}
			if (lstrncmp(szFullName, (char *) lpEnumLogFontEx->elfFullName, n) == 0) {
/*				strcpy(szFaceName, lpEnumLogFont->lfFaceName); */
				strcpy(szFaceName, lpLogFont->lfFaceName);
				nFullName = n;
				return (TRUE);			/* incomplete match, continue */
			}
			break;

		case 3:		/* get all Full Names - build FaceName FullName table */
			if (nFullIndex >= MAXOUTLINEFONTS) {
				sprintf(debugstr, "Too Many Full Names (> %d)\n", MAXOUTLINEFONTS);
				if (bDebug > 1) OutputDebugString(debugstr);
				WriteError(debugstr);
				return (FALSE);			/* full up - stop! */
			}
			lpFullCurrent = lpFullNames + nFullIndex *
							(LF_FACESIZE+2 + LF_FULLFACESIZE);
/*			strcpy(szFaceName, lpFullCurrent); */ /* debug */
			strcpy(lpFullCurrent, lpLogFont->lfFaceName);		/* Face Name */
			*(lpFullCurrent + LF_FACESIZE) = '\0';
			*(lpFullCurrent + LF_FACESIZE+1) = (char) lpNewTextMetric->ntmFlags;
			strcpy(szFullName, (char *) lpEnumLogFontEx->elfFullName); 	/* debugging only */
/*			if (strlen(szFullName) > 31) {
				if (bDebug > 1) {
					sprintf(debugstr, "%s length %d\n", szFullName, strlen(szFullName));
					OutputDebugString(debugstr);
				}			
			} */ 			/* 1995/July/24 TEMPORARY */
			strcpy((char *) lpFullCurrent + LF_FACESIZE+2,
				   (char *) lpEnumLogFontEx->elfFullName);

			if (strcmp(lpFullCurrent + LF_FACESIZE+2 - (LF_FACESIZE+2 + LF_FULLFACESIZE),
						(char *) lpEnumLogFontEx->elfFullName) == 0) {
/*	do nothing, duplication */
			}
			else {
#ifdef DEBUGFONTSELECT
				if (bDebug > 1) {
					boldflag = italicflag = 0;
					if (lpNewTextMetric->ntmFlags & NTM_BOLD) boldflag = 1;
					if (lpNewTextMetric->ntmFlags & NTM_ITALIC) italicflag = 1;
					if (lpNewTextMetric->ntmFlags & NTM_REGULAR) {
						boldflag = 0;
						italicflag = 0;
					}
					sprintf(debugstr, "%3d Face: %s,%s%s%s Full: %s\n",
							nFullIndex,
							szFaceName,
							(!boldflag && !italicflag) ? "REGULAR" : "",
							boldflag ? "BOLD" : "",
							italicflag ? "ITALIC" : "",
							szFullName);
					OutputDebugString(debugstr);
				}
#endif
				nFullIndex++;
			}
			return (TRUE);			/* continue enumerating */
/*			break; */

		case 4:	/* set bits in style word for given Face */
/*			for TT fonts, tmItalic can be 255 instead of 1 */
			if ((nFontType & TRUETYPE_FONTTYPE) == 0) {
				italicflag = (lpNewTextMetric->tmItalic != 0);
				boldflag = (lpNewTextMetric->tmWeight > 400);
			}
			else {
/* also use new method here 95/Aug/20 */
				boldflag = 0;
				italicflag = 0;
				if (lpNewTextMetric->ntmFlags & NTM_BOLD) boldflag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_ITALIC) italicflag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_REGULAR) {
					boldflag = 0;
					italicflag = 0;
				}
			}
/*			if (bDebug > 1) {
				sprintf(debugstr, "italic %d bold %d style %d\n",
						italicflag, boldflag,
							(1 << (italicflag + boldflag * 2)));
				OutputDebugString(debugstr);
			} */
			StyleBits |= (1 << (italicflag + boldflag * 2));
			return (TRUE);			/* continue enumerating */
			break;

		default:
			return (FALSE);			/* zero to stop enumerating */
			break;
	}
	return (TRUE);				/* non zero to continue enumerating */
}

int CALLBACK _export FontEnumProc (const ENUMLOGFONT FAR *lpEnumLogFont,
									  const NEWTEXTMETRIC FAR*lpNewTextMetric,
									  int nFontType, LPARAM lpData) {
	LPLOGFONT lpLogFont = (LPLOGFONT) lpEnumLogFont;
	int n;
	int italicflag, boldflag;
	LPSTR lpFullCurrent;
	int c = (int) lpData;

/*	if (bDebug > 1) {
		OutputDebugString(lpLogFont->lfFaceName);
		OutputDebugString("\t");
		OutputDebugString(lpEnumLogFont->elfFullName);
		OutputDebugString("\t");
		sprintf (debugstr, "FontType %d lpData %d Style %x\n",
				 nFontType, c, lpNewTextMetric->ntmFlags);
		OutputDebugString(debugstr);
	} */

/*	Ignore all but TrueType fonts at this point (except for usage number 4) */
	if (c < 4) {
		if ((nFontType & TRUETYPE_FONTTYPE) == 0) return(TRUE);
	}
	switch (c) {
		case 0:
/*			if (strcmp(lpEnumLogFont->lfFaceName, szFaceName) == 0) */
			if (strcmp(lpLogFont->lfFaceName, szFaceName) == 0) { 
/* now check for match in style information */
/* use lpEnumLogFont.lfItalic and lpEnumLogFont.lpWeight ? */
/* use lpNewTextMetrics.tmItalic and lpNewTextMetrics.tmWeight ? */
				if (lpNewTextMetric->ntmFlags & NTM_BOLD) {
					if (!bBoldFlag) return (TRUE);
				}
				else if (bBoldFlag) return (TRUE);
				if (lpNewTextMetric->ntmFlags & NTM_ITALIC) {
					if (!bItalicFlag) return (TRUE);
				}
				else if (bItalicFlag) return (TRUE);
				if (lpNewTextMetric->ntmFlags & NTM_REGULAR) {
					if (bItalicFlag || bBoldFlag) return (TRUE);
				}
				else if (!bItalicFlag && !bBoldFlag) return (TRUE);
/* OK, Found match with Face Name and Style, return FullName */
				strcpy(szFullName, (char *) lpEnumLogFont->elfFullName);
				return (FALSE);			/* zero to stop enumerating */
			} 
			break;
		case 1:
			if (strcmp(szFullName, (char *) lpEnumLogFont->elfFullName) == 0) {
/*				strcpy(szFaceName, lpEnumLogFont->lfFaceName); */
				strcpy(szFaceName, lpLogFont->lfFaceName);
				if (lpNewTextMetric->ntmFlags & NTM_BOLD) bBoldFlag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_ITALIC) bItalicFlag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_REGULAR) {
					bBoldFlag = 0;
					bItalicFlag = 0;
				}
				nFullName = 1;				/* indicate successful match */
				return (FALSE);				/* zero to stop enumerating */
			}
			break;

		case 2:
			n = strlen((char *) lpEnumLogFont->elfFullName);
			if (n < nFullName) return (TRUE);
			if (strcmp(szFullName, (char *) lpEnumLogFont->elfFullName) == 0) {
				strcpy(szFaceName, lpLogFont->lfFaceName);
				nFullName = n;
				return (FALSE);			/* stop, found exact match */
			}
			if (lstrncmp(szFullName, (char *) lpEnumLogFont->elfFullName, n) == 0) {
/*				strcpy(szFaceName, lpEnumLogFont->lfFaceName); */
				strcpy(szFaceName, lpLogFont->lfFaceName);
				nFullName = n;
				return (TRUE);			/* incomplete match, continue */
			}
			break;

		case 3:
			if (nFullIndex >= MAXOUTLINEFONTS) {
				sprintf(debugstr, "Too Many Full Names (> %d)\n", MAXOUTLINEFONTS);
				if (bDebug > 1) OutputDebugString(debugstr);
				WriteError(debugstr);
				return (FALSE);	/* full up - stop! */
			}
			lpFullCurrent = lpFullNames + nFullIndex *
							(LF_FACESIZE+2 + LF_FULLFACESIZE);
/*			strcpy(szFaceName, lpFullCurrent); */ /* debug */
			strcpy(lpFullCurrent, lpLogFont->lfFaceName);
			*(lpFullCurrent + LF_FACESIZE) = '\0';
			*(lpFullCurrent + LF_FACESIZE+1) = (char) lpNewTextMetric->ntmFlags;
			strcpy(szFullName, (char *) lpEnumLogFont->elfFullName); 	/* debug */
/*			if (strlen(szFullName) > 31) {
				if (bDebug > 1) {
					sprintf(debugstr, "%s length %d\n", szFullName, strlen(szFullName));
					OutputDebugString(debugstr);
				}			
			} */			/* 1995/July/24 TEMPORARY */
			strcpy((char *) lpFullCurrent + LF_FACESIZE+2,
				   (char *) lpEnumLogFont->elfFullName);

#ifdef DEBUGTABLES
			if (bDebug > 1) {
				boldflag = italicflag = 0;
				if (lpNewTextMetric->ntmFlags & NTM_BOLD) boldflag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_ITALIC) italicflag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_REGULAR) {
					boldflag = 0;
					italicflag = 0;
				}
				sprintf(debugstr, "`%s' FACE `%s,%s%s%s'\n",
						szFullName, szFaceName,
						boldflag ? "BOLD" : "",
						italicflag ? "ITALIC" : "",
						(!boldflag && !italicflag) ? "REGULAR" : "");
				OutputDebugString(debugstr);
			}
#endif
			nFullIndex++;
			return (TRUE);			/* continue enumerating */
			break;

		case 4:
/*			for TT fonts, tmItalic can be 255 instead of 1 */
			if ((nFontType & TRUETYPE_FONTTYPE) == 0) {
				italicflag = (lpNewTextMetric->tmItalic != 0);
				boldflag = (lpNewTextMetric->tmWeight > 400);
			}
			else {
/* also use new method here 95/Aug/20 */
				boldflag = 0;
				italicflag = 0;
				if (lpNewTextMetric->ntmFlags & NTM_BOLD) boldflag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_ITALIC) italicflag = 1;
				if (lpNewTextMetric->ntmFlags & NTM_REGULAR) {
					boldflag = 0;
					italicflag = 0;
				}
			}
/*			if (bDebug > 1) {
				sprintf(debugstr, "italic %d bold %d style %d\n",
						italicflag, boldflag,
							(1 << (italicflag + boldflag * 2)));
				OutputDebugString(debugstr);
			} */
			StyleBits |= (1 << (italicflag + boldflag * 2));
			return (TRUE);			/* continue enumerating */
			break;
		default:
			return (FALSE);			/* zero to stop enumerating */
			break;
	}
	return (TRUE);				/* non zero to continue enumerating */
}

/* FontType DEVICE_FONTTYPE, RASTER_FONTTYPE, TRUETYPE_FONTTYPE */

/* if lpszFamilies == NULL, selects one font from each family */
/* if lpszFamiles != NULL, enumerates fonts of that family */

/* lpData points to application specific data passed by EnumFontFamilies */
/* 0 map from FaceName + Style to FullName */
/* 1 map from FullName to FaceName + Style */

/*  Only for TrueType fonts (?) --- so we only get here if bWin31 != 0 */

int FullFromFace (HDC hDC, char *name, int boldflag, int italicflag) {
	LOGFONT lf;

/*	if (bDebug > 1) OutputDebugString("FullFromFace\n"); */

	strcpy(szFaceName, name);		/* set up FaceName */
	bBoldFlag = boldflag;			/* set up style */
	bItalicFlag = italicflag;		/* set up style */
	*szFullName = '\0';				/* so can check whether it worked */
/*	special case damn Marlett (system hidden) font */	/* 1996/Oct/26 ? */
	if (_strcmpi(name, "marlett") == 0 && boldflag == 0 && italicflag == 0) {	
		strcpy(szFullName, "Marlett");
		return strlen(szFullName);
	}								
/*	if (bDebug > 1) {
		sprintf(debugstr, "Face: %s bold %d italic %d\n",
				szFaceName, bBoldFlag, bItalicFlag);
		OutputDebugString(debugstr);
	} */
/*	(void) EnumFontFamilies(hDC, (LPSTR) NULL, (NEWFONTENUMPROC) FontEnumProc,
				 (LPARAM) 0); */

/*	work under construction 97/Feb/11 */
	lf.lfCharSet = DEFAULT_CHARSET;
	strcpy(lf.lfFaceName, szFaceName);
	lf.lfPitchAndFamily = 0;
	(void) EnumFontFamiliesEx(hDC, &lf,	(FONTENUMPROC) FontEnumProcEx,
							  (LPARAM) 0, (DWORD) 0L);

/*	(void) EnumFontFamilies(hDC, (LPSTR) szFaceName,
							(FONTENUMPROC) FontEnumProc, (LPARAM) 0); */

#ifdef DEBUGFONTSELECT
	if (bDebug > 1) {
		sprintf(debugstr, "FullName `%s' in FullFromFace\n", szFullName);
		OutputDebugString(debugstr);
	}
#endif
	if (*szFullName != '\0')
		return strlen(szFullName);			/* it worked ! */
	else if (_strcmpi(name, "marlett") == 0 &&
			 boldflag == 0 && italicflag == 0)	{	
/*			 special case this damn font */	/* 1996/Oct/26 ? */
		strcpy(szFullName, "Marlett");
		return strlen(szFullName);
	}	/* 96/Oct/26 */
	else return 0;							/* it failed ! */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Code for building table relating TFM File Name to Face Name and Style */

LPSTR GrabFullNames (void) {
	LPSTR lpFullNames=NULL;

	if (hFullNames == NULL) winerror("No Fonts to grab");
	else {
		lpFullNames = (LPSTR) GlobalLock(hFullNames);
		if (lpFullNames == NULL) {
			winerror("Unable to lock Fonts"); 
/*			winerror("Unable to lock Full Names"); */
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
	return lpFullNames;
}

void ReleaseFullNames (void) {
	int n;
	if (hFullNames == NULL) winerror("No Fonts to release");
	else {
		if ((n = GlobalUnlock(hFullNames)) > 0) {
/*			if (bDebug) {
				sprintf(str, "%d\t", n);
				strcat(str, "Lock count Not Zero");
				strcat(str, " - ReleaseFullNames");
				if (wincancel(str))	PostQuitMessage(0);
			} */
		}
		else lpFullNames = NULL;
	}
}

/* 512 * (32 + 2 + 64) = 50,176 byte - quite a chunck 1995/July/7 ! */

void AllocFullNames (void) {		/* grab space for FullNames - if needed */
	unsigned long n;
	if (hFullNames == NULL) {
/*	entries are: FaceName, Style, FullName */
		n = (unsigned long) MAXOUTLINEFONTS * (LF_FACESIZE + 2 + LF_FULLFACESIZE); 
		if (bDebug > 1) {
			sprintf(debugstr, "FullNames %d bytes", n);
			OutputDebugString(debugstr);
		}
		hFullNames = GlobalAlloc(GHND, /* GMEM_MOVEABLE | GMEM_ZEROINIT */
			n);
		if (hFullNames == NULL) {
			winerror("Unable to allocate memory"); /* debug */
			PostQuitMessage(0);			/* pretty serious ! */
		}
		nFullIndex = 0;
	} 
}

void FreeFullNames (void) {
	if (hFullNames == NULL) {
		if (bDebug) winerror("No Fonts to free");
	}
	else hFullNames = GlobalFree(hFullNames);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

LPSTR GrabFileNames (void) {
	LPSTR lpFileNames=NULL;

	if (hFileNames == NULL) winerror("No Fonts to grab");
	else {
		lpFileNames = (LPSTR) GlobalLock(hFileNames);
		if (lpFileNames == NULL) {
			winerror("Unable to lock Fonts"); 
/*			winerror("Unable to lock File Names"); */
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
	return lpFileNames;
}

void ReleaseFileNames (void) {
	int n;
	if (hFileNames == NULL) winerror("No Fonts to release");
	else {
		if ((n = GlobalUnlock(hFileNames)) > 0) {
/*			if (bDebug) {
				sprintf(str, "%d\t", n);
				strcat(str, "Lock count Not Zero");
				strcat(str, " - ReleaseFileNames");
				if (wincancel(str))	PostQuitMessage(0);
			} */
		}
		else lpFileNames = NULL;
	}
}

/* 512 * (32 + 2 + 8 + 2) = 22,528 byte - quite a chunck 1995/July/7 ! */
/* But can be much less if fewer than 512 TrueType FullNames found */

void AllocFileNames (int n) {	/* grab space for FileNames - if needed */
	DWORD nLen;
	if (n == 0) {
		if (bDebug > 1) OutputDebugString("No Font Files\n");
		WriteError("No Font Files\n");
		n = 1;
	}
	if (hFileNames != NULL)
		FreeFileNames();  /* need to deallocate since may be different size */
						  /* alternatively use GlobalReAlloc (,,) */
	nLen = (DWORD) n * (LF_FACESIZE + 2 + MAXTFMNAME + 2);
#ifndef IGNORED
	if (nLen >= 65536L) {
		winerror("Too many TT fonts installed\n");			/* Over 963 ! */
		return;				/* sanity check, danger overflow 64 k segment */
	}
#endif
/*	if (n > 1024) return; */	/* sanity check, danger overflow 64 k segment */
	if (hFileNames == NULL) {   /* entries are: FaceName, Style, FileName */
		hFileNames = GlobalAlloc(GHND, /* GMEM_MOVEABLE | GMEM_ZEROINIT */
								 nLen);
/*			(unsigned long) MAXOUTLINEFONTS * */
/*			(unsigned long) n * (LF_FACESIZE + 2 + MAXTFMNAME + 2)); */
		if (hFileNames == NULL) {
			winerror("Unable to allocate memory"); /* debug */
			PostQuitMessage(0);			/* pretty serious ! */
		}
		nFileIndex = 0; 
	} 
}

void FreeFileNames(void) {
	if (hFileNames == NULL) {
		if (bDebug) winerror("No Fonts to free");
	}
	else hFileNames = GlobalFree(hFileNames);
	hFileNames = NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*	Try and add entry for FullName <-> FileName pair to table */
/*	Looks up FaceName & Style corresponding to FullName in temp table */
/*	And then tries to enter it into the final table */

void FillFileSub (char *szFullName, char *szFileName) {
	LPSTR lpFullCurrent;
	LPSTR lpFileCurrent;
/*	char szFaceName[LF_FACESIZE];	*/			/* Face Name */
	int m;
	int nStyle = 0;
	int nFileIndex = -1;

/*	we assume here that szFileName is all uppercase, has no trailing _ */
/*	and is less than MAXTFMNAME in length */

	*szFaceName = '\0';
	for (m = 0; m < nFullIndex; m++) {		/*	Look for matching Full Name */
		lpFullCurrent =  lpFullNames + m * (LF_FACESIZE + 2 + LF_FULLFACESIZE);
		if (strcmp(szFullName, lpFullCurrent + (LF_FACESIZE + 2)) == 0) {
			strcpy(szFaceName, lpFullCurrent);			 /* grab Face Name */
			nStyle = *(lpFullCurrent + LF_FACESIZE + 1); /* grab Style */
			nFileIndex = m;								 /* remember where */
			break;
		}
	}
/*	if something was found, fill in entry in output table */
	if (nFileIndex >= 0 && nFileIndex < nFullIndex) {
		lpFileCurrent = lpFileNames + nFileIndex *
						(LF_FACESIZE + 2 + MAXTFMNAME + 2);
/*		if (bDebug > 1) {
			sprintf(debugstr, "Adding %d `%s' `%s' (%s)\n",
					nFileIndex, szFullName, szFaceName, szFileName);
			OutputDebugString(debugstr);
		} */
/*	Next check whether duplicate entry for this FullName (in WIN.INI) */
/*		if (strlen(szFaceName) > LF_FACESIZE ||
			strlen(szFileName) > MAXTFMNAME) {
			if (bDebug > 1)
				OutputDebugString("Face or File too long\n");
			continue;
		} */
		if (*lpFileCurrent == '\0') {
			strcpy(lpFileCurrent, szFaceName);
			*(lpFileCurrent + LF_FACESIZE) = '\0';
			*(lpFileCurrent + LF_FACESIZE + 1) = (char) nStyle;
			strcpy(lpFileCurrent + LF_FACESIZE + 2, szFileName);
		}
		else {
			if (bDebug > 1) {
				sprintf(debugstr,	"TT Font `%s' `%s' (%s) listed twice\n",
					szFullName, szFaceName, szFileName);
/*				strcat(debugstr, " in WIN.INI\n"); */
				OutputDebugString(debugstr);
/*				else winerror(debugstr); */
			}
		}
	}
/*	complain if it was not found (yet appears in WIN.INI) */
	else {	/* end if (nFileIndex >= 0 && nFileIndex < nFullIndex) */
		if (bDebug > 1) {
			sprintf(debugstr,	"TT Font `%s' (%s) not installed, yet listed\n",
					szFullName, szFileName);
/*			strcat(debugstr, " in WIN.INI\n"); */
			OutputDebugString(debugstr);
/*			else winerror(debugstr); */
		}
	}
}

/* Warning: following writes back into arguments as it works */

void FillFileHelp (char *full, char *file) {
	char *szFullName;
	char *szFileName;

	if (StripTrueType(full) == NULL) return;	/* not TrueType */
	szFullName = StripQuotes(full);		/* do *after* terminating font name */

	szFileName = SansPath(file);
	FlushExtension(szFileName);
/*	if (strlen(szFileName) > 8) */		/* MAXTFMNAME ? long names ??? */
	if (strlen(szFileName) >= MAXTFMNAME) {	/* allow long names 95/Dec/26 */
		if (bDebug > 1) {
			sprintf(debugstr, "File name too long (%d >= %d): %s\n",
					strlen(szFileName), MAXTFMNAME, szFileName);
			OutputDebugString(debugstr);
		}
		return;			/* DOS file name ? */
	}
	StripTTVersion(szFileName);
	StripUnderScores(szFileName);
	UpperCase (szFileName);			
/* #ifdef DEBUGTABLES
	if (bDebug > 1) {
		sprintf(debugstr, "Adding `%s' (%s)\n", szFullName, szFileName);
		OutputDebugString(debugstr);
	} 
#endif */
	FillFileSub(szFullName, szFileName);
}

/* Try and Parse lines in WIN.INI */ /* Isolate FullName and TFM File Name */

#ifdef LONGNAMES
void FillFileAux (HFILE input) 
#else
void FillFileAux (FILE *input) 
#endif
{
	char *seq;
	
	while (fgets(str, sizeof(str), input) != NULL) {
		if (*str == '[') break;				/* hit next section of WIN.INI */
		if (*str == ';') continue;			/* comment */
		if (*str <= ' ') continue;			/* white space */

		if ((seq = strchr(str, '=')) == NULL) continue;
		*seq = '\0';		/* prevent any cross talk between full and file */
		FillFileHelp(str, seq+1);
	}
}

/* Fills file table by explicitly reading WIN.INI - no good in Windows NT */
/* Looks for specified section [Fonts] or [TTFonts] */
/* Returns non-zero if successful */

/* int FillFileTableRead (char *ininame, char *szSection) */
int FillFileTableRead (char *ininame, char *szSection) {
#ifdef LONGNAMES
	HFILE input;
#else
	FILE *input;
#endif
	char winini[MAXFILENAME];
	int flag=0;
	char *s;
	
	if (ininame == NULL) return 0;		/* sanity check */
/*	combine to make table face name, style <=> file name */ 
/*	MAXOUTLINEFONTS * (LF_FACESIZE + 2 + MAXTFMNAME + 2);
/*	we don't care about file *path* - so MAXTFMNAME equals 8 (32) */
/*	keep this table */

	if (bDebug > 1) {
		sprintf(debugstr, "Read File on %s\n", ininame);
		OutputDebugString(debugstr);
	}
/*	find and open win.ini, scan up to [fonts] */
	if (GetWindowsDirectory (winini, sizeof(winini)) == 0) return 0;
	s = winini + strlen(winini) - 1;
	if (*s != '\\') strcat(winini, "\\");
/*	strcat(winini, "win.ini"); */
	strcat(winini, ininame);

	input = fopen(winini, "r");

	if (input == BAD_FILE) { 
		if (bDebug) {
			sprintf(debugstr, "Can't find %s\n", winini);	/* debugging */
			if (bDebug > 1) OutputDebugString(debugstr);
			else winerror(debugstr);
		}
		return 0;								/* can't find WIN.INI */
	}
/*	winerror("Opened WIN.INI"); */
	while (fgets(str, sizeof(str), input) != NULL) {
		if (*str != '[') continue;
/*		if (_strnicmp(str, "[Fonts]", 7) == 0) */ /* 95/July/30 */
		if (_strnicmp(str, szSection, strlen(szSection)) == 0) {
			flag = 1;
			break;
		}
	}
/*	winerror("Found [Fonts]"); */
	if (flag) FillFileAux (input);
	else {									/* failed to find [Fonts] */
		if (bDebug) {
			sprintf(debugstr, "Did not find %s\n", szSection);
			if (bDebug > 1) OutputDebugString(debugstr);
			else winerror(debugstr);
		}
/*		winerror("Did not find [Fonts]"); */	/* debugging */
	}
	fclose (input);
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*  We use GetProfileString instead of reading WIN.INI if bUseGetProfileTT */
/*	Which is required in Windows NT to access registry data base for fonts */
/*	Try stepping through in order of FullName table - check speed */
/*	Is it better to step through order of GetProfileString ? */
/*  Problem is bug in T1INSTALL - inserts PS FontName instead of FullName */

/* Generalized to deal with ASCII registry files where inifile != win.ini */
/* wininiflag == 1 for win.ini, wininiflag == 0 for ttfonts.reg */

int FillFileTableProfile (char *inifile, int wininiflag) {
	LPSTR lpFullCurrent, lpFileCurrent;
	int m, n, err, ttfflag;
	int flag=0;							/* 98/Mar/26 noninit */
	int nStyle, nDone;
	char *s;
	char *szFileName;
	char szTrueType[LF_FULLFACESIZE + 12]; /* space for " (TrueType)" */
	int nBadFont=-1;

	if (inifile == NULL) return 0;		/* sanity check */

	if (bDebug > 1) {
		OutputDebugString("FillFileTableProfile\n");
		sprintf(debugstr, "GetProfile on %s ([TTFonts] then [Fonts]) FullIndex %d FileIndex %d\n",
				inifile, nFullIndex, nFileIndex);
		OutputDebugString(debugstr);
	}
	if (*inifile == '\0') {
		winerror("Null ini file?");			/* debugging */
		return 0;			/* bug ! */
	}
	if (nFullIndex == 0) {
		if (bDebug > 0) OutputDebugString("nFullIndex == 0\n");
		return 0;			/* sanity check */
	}
//	if (bDebug > 1) {
//		sprintf(debugstr, "FullIndex %d FileIndex %d", nFullIndex, nFileIndex);
//		OutputDebugString(debugstr);
//	}

	if (nFileIndex == nFullIndex) {		/* nothing to do ... 98/Nov/20 ? */
		return 1;
	}

	for (m = 0; m < nFullIndex; m++) {
		lpFullCurrent =  lpFullNames + m * (LF_FACESIZE + 2 + LF_FULLFACESIZE);
		nDone = *(lpFullCurrent + LF_FACESIZE);		/* grab Done bit */
		if (nDone != 0) continue;					/* 98/Nov/20 */
		nStyle = *(lpFullCurrent + LF_FACESIZE + 1); /* grab Style */
		strcpy(szFaceName, lpFullCurrent);			 /* grab Face Name */
		strcpy(szFullName, lpFullCurrent + (LF_FACESIZE + 2));
		n = strlen(szFullName);

//		the following should now be redundant since we trap .ttc font files
		if (bWinNT) {
//			@SimHei, @SimSun, @NSimSun	- rotated CJK fonts
			if (*szFullName == '@') strcpy(szFullName, szFullName+1);
//			deal with SimSun and NSimSun CJK fonts in W2K 99/Oct/9
			if (strncmp(szFullName, "SimSun", 6) == 0 ||
				  strncmp(szFullName, "NSimSun", 7) == 0)
				strcpy(szFullName, "SimSun & NSimSun");
			else if (strncmp(szFullName, "MingLiU", 7) == 0 ||
					 strncmp(szFullName, "PMingLiU", 8) == 0)
				strcpy(szFullName, "MingLiU & PMingLiU");
			else if (strncmp(szFullName, "Dotum", 5) == 0 ||
					 strncmp(szFullName, "Gulim", 5) == 0)
				strcpy(szFullName, "Gulim & GulimChe & Dotum & DotumChe");
			else if (strncmp(szFullName, "Batang", 6) == 0 ||
					 strncmp(szFullName, "Gungsuh", 7) == 0)
				strcpy(szFullName, "Batang & BatangChe & Gungsuh & GungsuhChe");
			else if (strncmp(szFullName, "MS Mincho", 9) == 0 ||
					 strncmp(szFullName, "MS PMincho", 10) == 0)
				strcpy(szFullName, "MS Mincho & MS PMincho");
			else if (strncmp(szFullName, "MS Gothic", 9) == 0 ||
					 strncmp(szFullName, "MS PGothic", 10) == 0 ||
					 strncmp(szFullName, "MS UI Gothic", 10) == 0)
				strcpy(szFullName, "MS Gothic & MS PGothic & MS UI Gothic");
		}

/*		First, attempt to patch bug with 32 byte truncation --- 1995/Aug/10 */
/*		Not needed in Windows NT ? */ /* Not needed with elfFullName ? */
		if (! bWinNT) {	
/*			Lucida Bright Smallcaps Demibold | */
/*			Lucida Sans Typewriter Bold Obli | que */
/*			Lucida New Math Alternate Italic | */
/*			Lucida New Math Alternate Demibo | ld */
/*			Lucida New Math Alternate Demibo | ld Italic */
			if (n >= 30) {
				if (bDebug > 1) {
					sprintf(debugstr, "%s (%d)\n", szFullName, n);
					OutputDebugString(debugstr);
				}  /* before - debugging */
			}
			if (n == 32) {
				if ((s = strstr(szFullName, "Itali")) != NULL) {
					if (s > szFullName + n - 9) strcpy(s, "Italic"); /* ? */
				}
				if ((s = strstr(szFullName, "Obl")) != NULL) {
					if (s > szFullName + n - 7) strcpy(s, "Oblique");
				}
				if ((s = strstr(szFullName, "Demi")) != NULL) {
					if (s > szFullName + n - 8) strcpy(s, "Demibold");
					if ((nStyle & NTM_ITALIC) && (nStyle & NTM_BOLD))
						strcat(s, " Italic");	/* added 95/Dec/5 */
				}
/*			if (bDebug > 1) OutputDebugString(szFullName); */
			}
		}	/* end of if !bWinNT dealing with 32 byte truncation */

/*	Are we using WIN.INI ? or using TTFONTS.REG ? */
		if (wininiflag == 0) {				/* bUseRegistryFile 95/Aug/18 */
/*			bracket Full Font Name `name' in quotedbl */
			strcpy (szTrueType, "\"");
			strcat (szTrueType, szFullName);
			strcat (szTrueType, " (TrueType)");			
			strcat (szTrueType, "\"");
			flag = GetPrivateProfileString(szRegistryFonts, szTrueType,
										   "", str, sizeof(str), inifile);
/*	Try also without the quotedbl, just in case user modified the file */
/*			if (flag == 0 || *str == '\0') */
			if (flag == 0) {
				strcpy (szTrueType, szFullName);
				strcat (szTrueType, " (TrueType)");			
				flag = GetPrivateProfileString(szRegistryFonts,
					   szTrueType, "",  str, sizeof(str), inifile);
			}
 /* The quotedbl's in the `value' are removed by GetPrivateProfileString */
/*			if (flag == 0 || *str == '\0') */
		}
		else {				/* wininiflag != 0 */
			strcpy (szTrueType, szFullName);
			strcat (szTrueType, " (TrueType)");
/*	look in [TTFonts] first 97/Feb/13 */ /* This based on actual Full Name */
			if (bTTFontSection) {
				flag = GetProfileString("TTFonts", szTrueType,
									"", str, sizeof(str));
			}
/*			if (flag == 0 || *str == '\0' && bTTFontSection) */
/*			if (flag == 0 && bTTFontSection) */
/*	look in [Fonts] next - not so good - since referred to registry */
/*	and registry has mangled FontName for converted fonts */
			if (flag == 0) {
				flag = GetProfileString("Fonts", szTrueType, "",
										str, sizeof(str));
			}
		}

/*		deal with Marlett bug in Windows NT for WIN16 version */
		if (flag == 0) {						/* 97/Feb/2 */
			if (strncmp(szTrueType, "Marlett", 7) == 0) {
				strcpy(str, "MARLETT.TTF");
				nStyle = NTM_REGULAR;			/* ??? */
				flag = strlen(str);
			}
		}
/*		MicroSoft Sans Serif ? .FON */
		if (flag == 0) {
			if (bDebug > 1) {
				sprintf(debugstr, "%s not found in %s\n", szTrueType, inifile);
				OutputDebugString(debugstr);
			}
		}


		if (flag == 0) {
/*			Now we have an error --- the font was not found */
			sprintf(str, "Face Name: %s,%s%s%s Full Name: %s",
					szFaceName,
					(nStyle & NTM_REGULAR) ? "REGULAR" : "",
					(nStyle & NTM_BOLD) ? "BOLD" : "",
					(nStyle & NTM_ITALIC) ? "ITALIC" : "",
					szFullName);
			if (bDebug > 1) OutputDebugString(str);
			n = strlen(szFullName);
/*			we ignore errors with FullNames of length 32 due to bug in Windows */
/*			where apparently elfFullName is truncated !!! */
/*			if (n == 32) continue; */					/* 1995/Aug/1 */
			if (n == 32 && bWinNT == 0) continue;		/* 1999/Oct/9 ??? */
//			if (n == 32 || bWinNT == 0) continue;		/* 1998/Nov/20 ??? */
/*			we ignore fonts with no real styles - deal with Marlett bug */
			if ((nStyle & (NTM_REGULAR | NTM_BOLD | NTM_ITALIC)) == 0) 
				continue;
/*			we ignore fonts with names that start witj @ */
			if (*szFullName == '@' && *szFaceName == '@') {
//				@SimHei, @SimSun, @NSimSun	- rotated CJK fonts
				continue;
			}
/*			ignore error until we have tried to make registry file ? */
			if (bNewShell && bUseRegistryFile && !bRegistrySetup) continue;
/*			ignore if user doesn't want warning */
//			if (bIgnoreBadInstall) continue;
/*			ignore if have been warned once */
//			if (bT1InstallWarn) continue;
			if (bIgnoreBadInstall < 0 && bT1InstallWarn == 0) 	{	// 2000 June 2
/*				Use T1INSTALL_ERR string instead to save space in data segment */
				s = str + strlen(str);
				if (bWin95) err = W95INSTALL_ERR;		/* Windows 95 */
				else if (bWinNT) err = T1INSTALL_ERR;	/* Windows NT */
				else err = TTSETUP_ERR;					/* Windows 3.1 ? */
				LoadString(hInst, err, s, sizeof(str) - strlen(str));
				if (wincancel(str) != 0) bT1InstallWarn = 1;
/*				bUseRegistryFile = 1; */	/* 96/Nov/30 ??? */
				if (bT1WarnOnce) bT1InstallWarn = 1;
			}
/*			write diagnostics to dviwindo.ini file 2000 June 2 */
			convertnewlines(str);
			s = str + strlen(str) + 1;		// just a little space
			if (nBadFont < 0) {			// Clean out old ones
				int k;
				for (k = 0; k < 10; k++) {
					sprintf(s, "BadFont%d", k);
					(void) WritePrivateProfileString(achDiag, s, NULL, achFile);
				}
				nBadFont = 0;
			}
			if (nBadFont < 10) {
				sprintf(s, "BadFont%d", nBadFont++);
				(void) WritePrivateProfileString(achDiag, s, str, achFile);
			}
			continue;					/* don't try and use this */
		}

/*		Now have font file name that we can try and use */
/*		First, ignore .FON and .TTC font files */
		szFileName = SansPath(str);
		if ((s = strchr(szFileName, '.')) != NULL) {
			if (_stricmp(s, ".FON") == 0) continue;		// 98/Nov/20 
			if (_stricmp(s, ".TTC") == 0) continue;		// 99/Oct/29 ???
		}

		ttfflag = 1;								/* .TTF and .FOT */
		if ((s = strchr(szFileName, '.')) != NULL) {	/* 98/Nov/20 */
			if (_stricmp(s, ".PFM") != 0) ttfflag = 0;
			if (_stricmp(s, ".PFB") != 0) ttfflag = 0;
		}
		FlushExtension(szFileName);

/*		Don't want to use FillFileHelp here, since we know nFileIndex */
/*		if (strlen(szFileName) > 8) */
		if (strlen(szFileName) >= MAXTFMNAME) {	/* allow long names 95/Dec/26 */
			if (bDebug > 1) {
				sprintf(debugstr, "File name too long (%d > %d): %s\n",
						strlen(szFileName), MAXTFMNAME, szFileName);
				OutputDebugString(debugstr);
			}
			continue;		/* DOS file name ? */
		}
		StripTTVersion(szFileName);	/* version numbers if after underscores */
		StripUnderScores (szFileName);	/* flush trailing underscores */
		UpperCase(szFileName);		/* fix ???  1995/Dec/26 */

		lpFileCurrent = lpFileNames + nFileIndex *
						(LF_FACESIZE + 2 + MAXTFMNAME + 2);
/*		if (strlen(szFaceName) > LF_FACESIZE ||
			strlen(szFileName) > MAXTFMNAME) {
			if (bDebug > 1)
				OutputDebugString("Face or File too long\n");
			continue;
		} */
		strcpy(lpFileCurrent, szFaceName);
		*(lpFileCurrent + LF_FACESIZE) = '\0';
		*(lpFileCurrent + LF_FACESIZE + 1) = (char) nStyle;
		if (ttfflag) *(lpFileCurrent + LF_FACESIZE) = TTF_DONE;
		else *(lpFileCurrent + LF_FACESIZE) = PFM_DONE;
		strcpy(lpFileCurrent + LF_FACESIZE + 2, szFileName);
		if (ttfflag) *(lpFullCurrent + LF_FACESIZE) = TTF_DONE;
		else *(lpFullCurrent + LF_FACESIZE) = PFM_DONE;

		if (bDebug > 1) {
			FormatNewNameStyle (debugstr, m, szFaceName, nStyle, szFileName);
/*			sprintf(debugstr, "%d\t%s,%s%s%s=%s\n", m, szFaceName,
					(nStyle & NTM_REGULAR) ? "REGULAR" : "",
					(nStyle & NTM_BOLD) ? "BOLD" : "",
					(nStyle & NTM_ITALIC) ? "ITALIC" : "",
					szFileName); */
			OutputDebugString(debugstr);
		}
		nFileIndex++;		// make it so !
	}
	if (bDebug > 1) {
		sprintf(debugstr, "Exit %d FileNames (for %d FullNames) %s",
				nFileIndex, nFullIndex,
				(nFileIndex == nFullIndex) ? "SUCCESS" : "INCOMPLETE");
		OutputDebugString(debugstr);
	}
	return (nFileIndex == nFullIndex);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef IGNORED
/* Find entry in lpFileNames that matches entry in lpFullNames */
/* i.e. has the same FaceName + Style */

int FileEntryFromFullEntry (int m) {
	LPSTR lpFullCurrent;
	LPSTR lpFileCurrent;
	int k;
	
	lpFullCurrent = lpFullNames + m * (LF_FACESIZE+2 + LF_FULLFACESIZE);
	for (k = 0; k < nFullIndex; k++) {
		lpFileCurrent = lpFileNames + k * (LF_FACESIZE+2 + MAXTFMNAME+2);
		if (bDebug > 1) {
			sprintf(debugstr, "%s %d => %s %d",
					lpFullCurrent, m, lpFileCurrent, k);
			OutputDebugString(debugstr);
		}
		if (*lpFileCurrent == '\0') continue;
/*		Compare Face Names */
		if (strcmp(lpFileCurrent, lpFullCurrent) == 0) {
/*			Compare Styles */
			if (*(lpFileCurrent + LF_FACESIZE + 1) ==
				*(lpFullCurrent + LF_FACESIZE + 1)) {
				if (bDebug > 1) {
					sprintf(debugstr, "%s %d => %s %d",
							lpFullCurrent, m, lpFileCurrent, k);
					OutputDebugString(debugstr);
				}
				return k;
			}   	 
		}
	}
	return -1;
}
#endif

int nFileIndex;

/* "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Type 1 Installer\\Type 1 Fonts"; */

/* Get Registry information for T1 fonts in NT5 => .PFM file names */

int GetFontInstallerInfo (LPSTR lpFullNames) {	/* 98/Nov/20 */
	HKEY hKey;
	char szValue[FILENAME_MAX + FILENAME_MAX + 2];	/* PFB and PFM */
	DWORD Type;				/* Probably will be set to REG_MULTI_SZ */
	DWORD cdData;
	char *s;
	char *szPFM, *szPFB;
	LPSTR lpFullCurrent;
	LPSTR lpFileCurrent;
	int nDone;
	int err;
	int m;

	if (nFullIndex == 0) return 0;
	
	if (bDebug > 1) {
		sprintf(debugstr, "GetFontInstallerInfo (T1) nFullIndex %d nFileIndex %d",
				nFullIndex, nFileIndex);
		OutputDebugString(debugstr);
	}
	
	strcpy(str, "SOFTWARE\\Microsoft\\Windows");
	if (bWinNT) strcat(str, " NT");
	strcat(str, "\\CurrentVersion");
	strcat(str, "\\Type 1 Installer");
	strcat(str, "\\Type 1 Fonts");

	err = RegOpenKeyEx (HKEY_LOCAL_MACHINE, str, 0L, KEY_QUERY_VALUE, &hKey);
	if (err	!= ERROR_SUCCESS) {
		ExplainError("RegOpenKey", err, 0);
		return -1;
	}										/* Open Key once only */

	for (m = 0; m < nFullIndex; m++) {
		lpFullCurrent = lpFullNames + m * (LF_FACESIZE+2 + LF_FULLFACESIZE);
/*		lpFullCurrent --- Windows Face Name */
/*		lpFullCurrent + LF_FACESIZE+1 --- style byte */
/*		lpFullCurrent + LF_FACESIZE+2 --- FullName */
		lpFileCurrent = lpFileNames + nFileIndex * (LF_FACESIZE+2 + MAXTFMNAME+2);
/*		lpFileCurrent --- Windows Face Name */
/*		lpFileCurrent + LF_FACESIZE+1 --- style byte */		
/*		lpFileCurrent + LF_FACESIZE+2 --- File Name */
		nDone = *(lpFullCurrent + LF_FACESIZE);
		if (nDone != 0) continue;
		strcpy(str, lpFullCurrent + LF_FACESIZE+2);	/* Full Name */
		s = str;
		while ((s = strchr(s, '-')) != NULL) *s = ' ';	
		*szValue = '\0';
		cdData = sizeof(szValue);
		err = RegQueryValueEx(hKey, str, 0L, &Type,
							  (unsigned char *) szValue, &cdData);
		if (err != ERROR_SUCCESS) {
/*			ERROR_FILE_NOT_FOUND 2 --- TrueType font, not Type 1 */
/*			ERROR_MORE_DATA 234 --- too little space for value */
/*			sprintf(debugstr, "RegQueryValueEx %d", err); */
/*			if (ExplainError(debugstr, err, 0) != 0) return -1; */
/*			sprintf(debugstr, "FAILED: %d %s %d %d", m, str, err, cdData); */
/*			OutputDebugString(debugstr); */
			continue;
		}
/*		buffer has T null PFM file name null PFB file name null null */	
		if (*szValue == '\0') continue;		/* sanity check */
		szPFM = szValue + 2;				/* step over "T" */
		szPFB = szPFM + strlen(szPFM) + 1;
#ifdef DEBUGFONTSELECT
		if (bDebug > 1) {
			sprintf(debugstr, "%3d FullName: %s %s PFM: %s PFB: %s",
				m, str, szValue, szPFM, szPFB);
			OutputDebugString(debugstr);
		}
#endif
		szPFM = SansPath(szValue + 2);
		if ((s = strrchr(szPFM, '.')) != NULL) *s = '\0'; 
		StripUnderScores (szPFM);			/* flush trailing underscores */
		UpperCase(szPFM);					/* ? */
/*		if (strlen(szPFM) >= MAXTFMNAME)	
			strncpy(lpFileCurrent + LF_FACESIZE+2, szPFM, MAXTFMNAME);
		else strcpy(lpFileCurrent + LF_FACESIZE+2, szPFM); */
		strncpy(lpFileCurrent + LF_FACESIZE+2, szPFM, MAXTFMNAME);
		*(lpFullCurrent + LF_FACESIZE) = PFM_DONE;	/* mark as T1 DONE */
/*		strncpy(lpFileCurrent, lpFullCurrent, LF_FACESIZE); 
		*(lpFileCurrent+LF_FACESIZE) = *(lpFullCurrent+LF_FACESIZE); 
		*(lpFileCurrent+LF_FACESIZE+1) = *(lpFullCurrent+LF_FACESIZE+1); */
		memcpy(lpFileCurrent, lpFullCurrent, LF_FACESIZE+2);
		nFileIndex++;
	}

	err = RegCloseKey(hKey);			/* do only once */
	if (err != ERROR_SUCCESS) {
		ExplainError("RegCloseKey", err, 0);
	}
	if (bDebug > 1) {
		sprintf(debugstr, "Resolved %d Font File Names (need %d more)",
				nFileIndex, nFullIndex-nFileIndex);
		OutputDebugString(debugstr);
	}
	return nFileIndex;
}

/* "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"; */

/* Get Registry information for TT fonts => .TTF file names */

int GetFontRegistryInfo (LPSTR lpFullNames) {	/* 98/Nov/20 */
	HKEY hKey;
	char szValue[FILENAME_MAX + 2];	/* TTF */
	DWORD Type;				/* Probably will be set to REG_SZ */
	DWORD cdData;
	char *s;
	char *szTTF;
	LPSTR lpFullCurrent;
	LPSTR lpFileCurrent;
	int err;
	int nDone;
	int m;

	if (nFullIndex == 0) return 0;

	if (bDebug > 1) {
		sprintf(debugstr, "GetFontRegistryInfo (TT) nFullIndex %d nFileIndex %d",
				nFullIndex, nFileIndex);
		OutputDebugString(debugstr);
	}

	ConstructFontKey0(str);

	err = RegOpenKeyEx (HKEY_LOCAL_MACHINE, str, 0L, KEY_QUERY_VALUE, &hKey);
	if (err	!= ERROR_SUCCESS) {
		ExplainError("RegOpenKey", err, 0);
		return -1;
	}										/* Open Key once only */

	for (m = 0; m < nFullIndex; m++) {
		lpFullCurrent = lpFullNames + m * (LF_FACESIZE+2 + LF_FULLFACESIZE);
/*		lpFullCurrent --- Windows Face Name */
/*		lpFullCurrent + LF_FACESIZE+1 --- style byte */
/*		lpFullCurrent + LF_FACESIZE+2 --- FullName */
		lpFileCurrent = lpFileNames + nFileIndex * (LF_FACESIZE+2 + MAXTFMNAME+2);
/*		lpFileCurrent --- Windows Face Name */
/*		lpFileCurrent + LF_FACESIZE+1 --- style byte */		
/*		lpFileCurrent + LF_FACESIZE+2 --- File Name */
		nDone = *(lpFullCurrent + LF_FACESIZE);
		if (nDone != 0) continue;
		strcpy(str, lpFullCurrent + LF_FACESIZE+2);	/* Full Name */
		strcat(str, " (TrueType)");
		*szValue = '\0';
		cdData = sizeof(szValue);
		err = RegQueryValueEx(hKey, str, 0L, &Type,
							  (unsigned char *) szValue, &cdData);
		if (err != ERROR_SUCCESS) {
/*			ERROR_FILE_NOT_FOUND 2 --- not TT font --
  			or not in c:\winnt\fonts !!! --- or TTC font */
/*			ERROR_MORE_DATA 234 --- too little space for value */
			sprintf(debugstr, "RegQueryValueEx %d", err);
//			if (ExplainError(debugstr, err, 0) != 0) return -1; 
			if (bDebug > 1) {
				sprintf(debugstr, "FAILED: %d %s err %d", m, str, err); 
				OutputDebugString(debugstr);
			}
			if (ExplainError(debugstr, err, 0) != 0) return -1; 
/*			kludge to deal with Marlett font special case */
			if (strncmp(str, "Marlett ", 8) == 0) {
				strcpy(szValue, "Marlett.ttf");
				err = ERROR_SUCCESS;
			}
			if (err != ERROR_SUCCESS) {
/*				kludge for Microsoft Sans Serif Regular (TrueType) */
				strcpy(str, lpFullCurrent + LF_FACESIZE+2);	/* Full Name */			
				strcat(str, " Regular");
				strcat(str, " (TrueType)");
				*szValue = '\0';
				cdData = sizeof(szValue);
				err = RegQueryValueEx(hKey, str, 0L, &Type,
									  (unsigned char *) szValue, &cdData);
			}
			if (err != ERROR_SUCCESS) continue;
		}
#ifdef DEBUGFONTSELECT
		if (bDebug > 1) {
			sprintf(debugstr, "%3d FullName: %s TTF %s",
					m, str, szValue);
			OutputDebugString(debugstr);
		}
#endif
		szTTF = SansPath(szValue);
		if ((s = strchr(szTTF, '.')) != NULL) *s = '\0'; 
		StripUnderScores (szTTF);			/* flush trailing underscores */
		UpperCase(szTTF);							/* ? */
/*		if (strlen(szTTF) >= MAXTFMNAME)	
			strncpy(lpFileCurrent + LF_FACESIZE+2, szTTF, MAXTFMNAME);
		else strcpy(lpFileCurrent + LF_FACESIZE+2, szTTF); */
		strncpy(lpFileCurrent + LF_FACESIZE+2, szTTF, MAXTFMNAME);
		*(lpFullCurrent + LF_FACESIZE) = TTF_DONE;	/* mark as TT DONE */
/*		strncpy(lpFileCurrent, lpFullCurrent, LF_FACESIZE+2); 
		*(lpFileCurrent+LF_FACESIZE) = *(lpFullCurrent+LF_FACESIZE);
		*(lpFileCurrent+LF_FACESIZE+1) = *(lpFullCurrent+LF_FACESIZE+1); */
		memcpy(lpFileCurrent, lpFullCurrent, LF_FACESIZE+2);
		nFileIndex++;
	}

	err = RegCloseKey(hKey);			/* do only once */
	if (err != ERROR_SUCCESS) {
		ExplainError("RegCloseKey", err, 0);
	}
	if (bDebug > 1) {
		sprintf(debugstr, "Resolved %d Font File Names (need %d more)",
				nFileIndex, nFullIndex-nFileIndex);
		OutputDebugString(debugstr);
	}
	return nFileIndex;
}

void ShowMissingFullNames (LPSTR lpFullNames) {	/* 98/Nov/20 */
	LPSTR lpFullCurrent;
	int m;
	int nDone;

	for (m = 0; m < nFullIndex; m++) {
		lpFullCurrent = lpFullNames + m * (LF_FACESIZE+2 + LF_FULLFACESIZE);
		nDone = *(lpFullCurrent + LF_FACESIZE);
		if (nDone != 0) continue;
		strcpy(str, lpFullCurrent + LF_FACESIZE+2);	/* Full Name */
#ifdef DEBUGFONTSELECT
		if (bDebug > 1) {
			sprintf(debugstr, "MISSING: %3d FullName: %s", m, str);
			OutputDebugString(debugstr);
		}
#endif
	}
}

int TryRegistry(void);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Do this the first time a TrueType font is called for ? */
/* How do we know?  If it is not Type 1 ? That is, can't find in ATM.INI */
/* The final table relates Face Name + Style to File Name */
/* Remember the final table, but not the intermediate ones */
/* Since the intermediate table with FullNames is rather large */

/* This is only for TrueType fonts --- so we only get here if bWin31 != 0 */
/* Well in W2K it is also for Type 1 fonts ... */

/* Called from TryForTrue in winfonts.c and mapfontnames in winpslog.c */

/* int SetupTTMapping (HWND hWnd) */
int SetupTTMapping (HDC hDC) {
/*	HGLOBAL hFullNames; */
/*	HDC hDC; */
	int m;
	char *s;
//	LPSTR lpFaceCurrent;
//	LPSTR lpFullCurrent; 
//	LPSTR lpFileCurrent; 
	LOGFONT lf;

	if (hFileNames != NULL) return 0;		/* already done ! */

	if (bDebug > 1) OutputDebugString("SetupTTMapping\n");
/*	call RegEdit - do *always* so its safer to read later ? 95/Aug/14 */
	if (bAlwaysWriteReg) {
/*		if (bWin95 && bUseRegistryFile && !bRegistrySetup) */
/*		we can assume szRegistry is not NULL */
		if (bNewShell && bUseRegistryFile && !bRegistrySetup) {
#ifdef DEBUGREGISTRY
			if (bDebug > 1) OutputDebugString("Calling RegEdit\n");
#endif
			if (szRegistry != NULL)	WriteRegFile(szRegistry);
		}  /* this does it brute force 95/Aug/18 */
	}

/*	Read all installed Windows Face Names --- use EnumFontFamilies */	
/*	=> table of Face Names */  /* this includes TT *and* T1 faces */
/*	Already have code to do this --- adds (TT) to FaceName though if TT */ 
/*	MAXOUTLINEFACES * MAXFACENAME *//* keep this table for ShowFont ? */
/*	hDC = GetDC(hWnd); */
	if (hFaceNames == NULL) {
		SetupFaceNamesSub(hDC, 1);
		ReleaseFaceNames();				/* fixed 1995/Nov/3 */
	}
#ifdef DEBUGFONTSELECT
	if (bDebug > 1) {
		sprintf(debugstr, "%d Face Names\n", nFaceIndex);
		OutputDebugString(debugstr);
//		wincancel(debugstr);			// debugging only
	}
#endif
/*	winerror("GrabFaceNames - SetupTTMapping"); */	/* DEBUGGING */

/*	Now have table of all Windows Face Names */ /********************/

/*	We may here Lock FaceName memory a second time ... but, OK */
	lpFaceNames = GrabFaceNames();

/*	For each Face Name enumerate all Styles and get Full Name */
/*	=> table of Face Name, Style <=> Full Name */ /* transient table */
/*	MAXOUTLINEFONTS * (LF_FACESIZE + 2 + LF_FULLFACESIZE) */
	if (hFullNames == NULL) (void) AllocFullNames();
	if (lpFullNames == NULL) lpFullNames = GrabFullNames();

#ifdef DEBUGFONTSELECT
	if (bDebug > 1) {
		sprintf(debugstr, "Get FullNames for %d FaceNames", nFaceIndex);
		OutputDebugString(debugstr);
	}
#endif

	nFullIndex = 0;
/*	what if nFaceIndex == 0 ? */
	for (m = 0; m < nFaceIndex; m++) {
//		lpFaceCurrent = lpFaceNames + m * MAXFACENAME;
//		lpFaceCurrent = lpFaceNames[m];
		if (lpFaceNames[m] != NULL) strcpy(szFaceName, lpFaceNames[m]);
		else strcpy(szFaceName, "");
/*		this assumes we don't want to do this for Type 1 ? except NT5 */
/*		if ((s = strchr(szFaceName, '(')) == NULL) */
		if ((s = strchr(szFaceName, '(')) == NULL) {
/*			if (bWinNT5 == 0) */	/* 98/Nov/12 */
			if (bWinNT == 0)		/* 98/23 */
				continue;			/* not TT */
		}
		else {
			while (s > szFaceName && *(s-1) == ' ') s--;
			*s = '\0';		/* trim white space at end of FaceName */
		}

		memset(&lf, 0, sizeof(LOGFONT));
/*		work under construction 97/Feb/11 */
		lf.lfCharSet = DEFAULT_CHARSET;
		strcpy(lf.lfFaceName, szFaceName);
		lf.lfPitchAndFamily = 0;
		if (bNewShell)				/* experiment ??? 97/Mar/26 */
			EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC) FontEnumProcEx,
						   (LPARAM) 3, (DWORD) 0L);
		else EnumFontFamilies(hDC, (LPSTR) szFaceName,
				(FONTENUMPROC) FontEnumProc, (LPARAM) 3); /* 97/Mar/26 */

/*		EnumFontFamilies(hDC, (LPSTR) szFaceName,
				(FONTENUMPROC) FontEnumProc, (LPARAM) 3); */
	}
/*	winerror("ReleaseFaceNames - SetupTTMapping"); */	 /* DEBUGGING */
/*	we may here UnLock FaceName memory only a first time ... but OK */
	ReleaseFaceNames();				/* now don't need Face Name table anymore */
/*	(void) ReleaseDC(hWnd, hDC); */	/* and don't need hDC anymore */

#ifdef DEBUGFONTSEARCH
	if (bDebug > 1) {
		sprintf(debugstr,
/* "%d Type 1 Face Names\n%d TrueType Face Names\n%d TrueType Full Names\n", */
"%d TT Face Names + %d non TT Face Names => %d Full Names\n",
			nTrueIndex, nFaceIndex-nTrueIndex, nFullIndex);
		OutputDebugString(debugstr);
	}
#endif

/*	Now have table of all Full Names */	/***************************/

/*	Read win.ini [fonts] section (using GetProfileString perhaps) */
/*	Which is table of Full Name <=> File Name */
/*	MAXOUTLINEFONTS * (LF_FACESIZE + 2 for style + MAXTFMNAME + 2) */
/*	We don't care about file *path* - so MAXTFMNAME equals 8 (32) */
	(void) AllocFileNames(nFullIndex);
	lpFileNames = GrabFileNames();

/*	What if nFullIndex == 0 ? */ /* is it necessary to zero this ? */
#ifdef IGNORED
	for (m = 0; m < nFullIndex; m++) {	/* reset the table */
		lpFileCurrent =  lpFileNames + m * (LF_FACESIZE + 2 + MAXTFMNAME + 2);
		*lpFileCurrent = '\0';						/* blank face name */
		*(lpFileCurrent + LF_FACESIZE) = 0;			/* done byte */
		*(lpFileCurrent + LF_FACESIZE + 1) = 0;		/* style */
		*(lpFileCurrent + LF_FACESIZE + 2) = '\0';	/* blank file name */
	}
#endif

/*	Don't actually build the full name <=> file name table */
/*	Instead build the final table direct ? */

	nFileIndex = 0;

/*	if (bWinNT5) */	/* get Type 1 installer registry info */
	if (bWinNT) {		/* get Type 1 installer registry info 98/Nov/23 */
		int flag;
		flag = GetFontInstallerInfo(lpFullNames);		
		flag = GetFontRegistryInfo(lpFullNames);
		if (bDebug > 1) {
			ShowMissingFullNames(lpFullNames);		/* debugging only */
		}
/*		need we do any more ? */ /* particularly if flag == 0) ? */
	}

	if (nFileIndex < nFullIndex) {						/* 98/Nov/20 */
		if (bDebug > 1) OutputDebugString("Font Table incomplete");
/*	Do  only if there are still FUllNames for which FileName not known */
/*	Registry has mangled FontNames for Type 1 fonts - not Full Names ... */
		if (bNewShell && bUseRegEnumValue) {			/* 96/Oct/2 ??? */
			TryRegistry(); /* Use KRNL386.EXE export RegEnumValue 1995/Aug/24 */
		}
		else if (bUseGetProfileTT) {
/*	if (bUseGetProfileTT) */
/*  use szRegistry = "ttfonts.reg" */
/*	Use win.ini if szRegistry is empty string, or "nul" or "win.ini" */
/*	We assume here that szRegistry is not the empty string... */
			if (bUseRegistryFile == 0)
/*				strcmp(szRegistry, "") == 0 ||
				strcmp(szRegistry, "nul") == 0 ||
				strcmp(szRegistry, "win.ini") == 0) */
				FillFileTableProfile("win.ini", 1);		/* use win.ini */
			else FillFileTableProfile(szRegistry, 0);	/* the new way ! */
/*			if (*szRegistry != '\0')
				FillFileTableProfile(szRegistry, 0);
			else FillFileTableProfile("win.ini", 1); */
		}
		else { /* if we don't use GetProfile - we actually read the file */
			if (bUseRegistryFile == 0) {
/*				*szRegistry == '\0' ||
				strcmp(szRegistry, "nul") == 0 ||
				strcmp(szRegistry, "win.ini") == 0) */
/*				FillFileTableRead("win.ini", "[Fonts]"); */
				if (bTTFontSection)	FillFileTableRead("win.ini", "[TTFonts]");
				FillFileTableRead("win.ini", "[Fonts]");
			}
			else FillFileTableRead(szRegistry, szRegistryFonts); /* the new way */ 
/*			if (*szRegistry != '\0')
				FillFileTableRead(szRegistry, szRegistryFonts);
			else {
				FillFileTableRead("win.ini", "[Fonts]");
				if (bTTFontSection)	FillFileTableRead("win.ini", "[TTFonts]");
			} */
		}
	}

	ReleaseFileNames();				/* release it now, but don't free it */
	ReleaseFullNames();				/* release this and then free it */
	FreeFullNames();				/* don't need this temp table anymore */
	nFileIndex = nFullIndex;		/* remember how many (???) */
	if (bDebug > 1) OutputDebugString("Exit SetupTTMapping\n");
	return 0;
}

/* remember to FreeFileNames() if at termination hFileNames != NULL */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* some stuff from shellapi.h -- or #include <shellapi.h> */
/* Instead getting from shell32.dll, get it from krnl386.exe in Windows 95 */
/* Which also has RegEnumValue */

#define ERROR_SUCCESS           0L

#define REG_NONE		( 0 ) 	    /* no value type from winnt.h */
#define REG_SZ			( 1 ) 	    /* string type from winnt.h */

/* #define HKEY_CLASSES_ROOT	1 */ /* from c:\msvc\shellapi.h */

/* Reserved Key Handles. c:\\msvcnt\include\winreg.h */

/* #define HKEY_CLASSES_ROOT           (( HKEY ) 0x80000000 ) */
/* #define HKEY_CURRENT_USER           (( HKEY ) 0x80000001 ) */
/* #define HKEY_LOCAL_MACHINE          (( HKEY ) 0x80000002 ) */
/* #define HKEY_USERS                  (( HKEY ) 0x80000003 ) */

// EXE_ERROR strings no longer exist...

/* Brand new Windows 95 debute fix for Windows 95 new bug ! */
/* returns 0 if it failed for one reason or another */

int TryRegistry(void) {
	char szName[128], szValue[128];	/* Name string and Value string */
	DWORD nNameLen, nValueLen;		/* Length of Name string & Value string */
	DWORD nType;					/* Type Code for Value */
	HKEY hkResult;
	char szFonts[100];			/* long enough ? */
	static DWORD dwIndex;		/* why static ? */
/*	char *szShell="shell32.dll"; */	/* LoadLibrary error 21 */
/*	char *szShell="shell.dll"; */
	char *s; 

/*	UINT OldFlag; */
/*	int flag; */
/* 	char *szKRNL386="krnl386.exe"; */
/*	OFSTRUCT OfStruct; */

/*	if (bWin95 == 0) return 0; */	/* do only in Windows 95 ! not NT or 3.1 */
	if (bNewShell == 0) return 0;	/* changed 96/Oct/2 ??? */

#ifdef IGNORED
	if (hKRNL386 == NULL) {
/*		Following is merely a sanity test - MAY FLUSH EVENTUALLY */
/*		Presumably OpenFile is safe here 95/Dec/6 */
/*		Else check in GetSystemDirectory() dir using _lopen */
#ifdef LONGNAMES
		if (LongOpenFile(szKRNL386, OF_EXIST) == HFILE_ERROR) 
#else
		if (OpenFile(szKRNL386, &OfStruct, OF_EXIST) == HFILE_ERROR) 
#endif
		{
			sprintf(str, "%s not found", szKRNL386);
/*			if (bDebug) winerror(str); */
			winerror(str);			/* should not happen ! */
			return 0;				/* not very likely !!! */
		}
		if (bDebug > 1) {
			OutputDebugString("Trying Registry");
		}
/* should really not need LoadLibrary, the Module should already be loaded */
		OldFlag = SetErrorMode(SEM_NOOPENFILEERRORBOX); /* 95/Mar/31 */
/*		hKRNL386 = LoadLibrary(szKRNL386); */
		hKRNL386 = GetModuleHandle(szKRNL386);
		(void) SetErrorMode(OldFlag);  /* restore previous mode */
/*		if (hKRNL386 < HINSTANCE_ERROR) */
		if (hKRNL386 == NULL) {
			strcpy(str, szKRNL386);
			strcat(str, " ");
			flag = LoadString(hInst, (UINT) hKRNL386 + EXE_ERROR,	
							  str+9, sizeof(str)-9);
			if (flag == 0) {
				sprintf(str+9, "LoadLibrary error %d", (int) hKRNL386); 
				winerror(str);
				return 0;
			}
		}	/* end of hKRNL386 error */
	}	/* end of if hKRNL386 is NULL */

		
	if (bDebug > 1) OutputDebugString("TryRegistry\n");
#ifdef DEBUGREGISTRY
	if (bDebug > 1) {
		sprintf(debugstr, "%s handle is %0X\n", szKRNL386, hKRNL386);
		OutputDebugString(debugstr);
	}
#endif	/* end of if DEBUGREGISTRY */

/*	Now get the procedure addresses in KRNL386.EXE */
	lpRegOpenKey = (LPFNREGOPENKEY) GetProcAddress (hKRNL386, "RegOpenKey"); 
	lpRegCloseKey = (LPFNREGCLOSEKEY) GetProcAddress (hKRNL386, "RegCloseKey");
/*	For some reason can't get RegQueryValue in NT fron WIN16 prog */
	lpRegQueryValue = (LPFNREGQUERYVALUE) GetProcAddress (hKRNL386,
		"RegQueryValue");
	lpRegEnumKey = (LPFNREGENUMKEY) GetProcAddress (hKRNL386, "RegEnumKey");
/*	an experiment 1995/July/30 revived 1995/Aug/24 ! */
	lpRegEnumValue = (LPFNREGENUMVALUE) GetProcAddress (hKRNL386, "RegEnumValue");

	if (lpRegOpenKey == NULL || lpRegCloseKey == NULL ||
/*		lpRegQueryValue == NULL || */
		lpRegEnumKey == NULL || lpRegEnumValue == NULL) {
/*		sprintf(debugstr, "Unable to find functions in %s", szKRNL386); */
		sprintf(debugstr,
			"OpenKey %d CloseKey %d QueryValue %d EnumKey %d EnumValue %d",
				(lpRegOpenKey != NULL), (lpRegCloseKey != NULL),
				(lpRegQueryValue != NULL), (lpRegEnumKey != NULL),
				(lpRegEnumValue != NULL));
		winerror (debugstr);
/*		FreeLibrary((HINSTANCE) hKRNL386); */ /* don't free hShell */
/*		Don't need to free, since we used GetModuleHandle, not LoadLibrary */
/*		FreeLibrary (hKRNL386);		hKRNL386 = NULL; */
		return 0;
	}
#endif

/*	szFonts = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts"; */
/*	szFonts = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"; */
	ConstructFontKey0(szFonts);

	if (RegOpenKey(HKEY_LOCAL_MACHINE, szFonts, &hkResult) != ERROR_SUCCESS) 
	{
#ifdef DEBUGREGISTRY
		if (bDebug > 1) {				/* fixed evening of 95/Aug/24 */
			OutputDebugString("Failed on ");
			OutputDebugString(szFonts);
		}
#endif

		RegCloseKey(hkResult);
#ifdef LOADKRNL386
		FreeLibrary(hKRNL386);
		hKRNL386=NULL;
#endif

		return 0;					/* failure presumably */
	}

#ifdef DEBUGREGISTRY
	if (bDebug > 1) OutputDebugString(szFonts);
#endif

	for (dwIndex = 0; dwIndex < 1024; dwIndex++) {
		*szName = '\0';							/* clear it */
		nNameLen = sizeof(szName);				/* set max size */
		*szValue = '\0';						/* clear it */
		nValueLen = sizeof(szValue);			/* set max size */

		if (RegEnumValue(hkResult, dwIndex, szName, &nNameLen, NULL,
			  &nType, (BYTE FAR *) szValue, &nValueLen)	!= ERROR_SUCCESS) 
/*		if ((*lpRegEnumValue)(hkResult, dwIndex, szName, &nNameLen, NULL,
			  &nType, (BYTE FAR *) szValue, &nValueLen)	!= ERROR_SUCCESS) */
		{
/* return should be ERROR_SUCCESS 0L  or  ERROR_NO_MORE_ITEMS  259L */
#ifdef DEBUGREGISTRY
			if (bDebug > 1) {
				sprintf(debugstr, "Exit on dwIndex %lu (RegEnumValue)\n", dwIndex);
				OutputDebugString(debugstr);
			}
#endif
			break; 
		}
/*		ignore if value is not null terminated string type */
		if (nType != REG_SZ) continue;	/*	check that nType == REG_SZ ? */
/*		ignore all but TrueType fonts */ /* flushed 97/Jan/21 */
/*		if ((s = strstr(szName, "(TrueType)")) == NULL) continue; */
/*		if (*szBuff == '.') continue; */
#ifdef DEBUGREGISTRY
		if (bDebug > 1) {
/* switched around so neater output on screen */
/*			sprintf(debugstr, "%ld\t%s\t%s\n", dwIndex, szValue, szName); */
			sprintf(debugstr, "%4ld %s %s\n", dwIndex, szValue, szName);
			OutputDebugString(debugstr);
		}
#endif
		if ((s = strstr(szName, "(TrueType)")) == NULL) continue;
/*		now use the pair name = value to fill in the table */
		FillFileHelp (szName, szValue);
	}	/* end of for dwIndex loop */
/*	FillFileHelp ("Marlett (TrueType)", "marlett"); */
	FillFileSub("Marlett", "MARLETT");		/* deal with special case */

	RegCloseKey(hkResult);
#ifdef LOADKRNL386
	FreeLibrary(hKRNL386);
	hKRNL386=NULL;
#endif

#ifdef DEBUGREGISTRY
	if (bDebug > 1) OutputDebugString("Closed Fonts\n");
#endif
	return 1;					/* success presumably */
}

/* don't free hKRNL386 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
								
/* glyph drawing exercise 1999/Jan/17 */

#ifdef IGNORED
DWORD GetGlyphOutline(
					  HDC hdc,             // handle to device context
					  UINT uChar,          // character to query
					  UINT uFormat,        // format of data to return
					  LPGLYPHMETRICS lpgm, // pointer to structure for metrics
					  DWORD cbBuffer,      // size of buffer for data
					  LPVOID lpvBuffer,    // pointer to buffer for data
					  CONST MAT2 *lpmat2   // pointer to transformation matrix structure
					 );
#endif

#ifdef IGNORED
typedef struct _GLYPHMETRICS { // glmt 
	UINT  gmBlackBoxX; 
	UINT  gmBlackBoxY; 
	POINT gmptGlyphOrigin; 
	short gmCellIncX; 
	short gmCellIncY; 
} GLYPHMETRICS;
#endif
	
/* currently set up to use wide character code */
/* need narrow character for nontext fonts */
/* text fonts need to be reencoded */
/* need to erase background - draw using WM_PAINT */
/* show control points (color) */
/* show baseline, advance width, font ascender/descender lines */
/* needs to be set up to scale properly based on window size and metrics */
/* do we need to ever deal with quadratic splines ? */
/* need to figure out how to tell where end of data occurs */

void DrawKnot (HDC hDC, int x, int y, int flag){
	HPEN hOldPen=NULL;
	int eps=20;
/*	if (bDebug > 1) {
		sprintf(debugstr, "knot %d %d", x, y);
		OutputDebugString(debugstr);
	} */
	hOldPen = SelectObject (hDC, flag ? hLinerPen : hBorderPen); 
	MoveToEx(hDC, x+eps, y+eps, NULL);
	LineTo(hDC, x-eps, y+eps);
	LineTo(hDC, x-eps, y-eps);
	LineTo(hDC, x+eps, y-eps);
	LineTo(hDC, x+eps, y+eps);	
	MoveToEx(hDC, x, y, NULL);
	if ((UINT) hOldPen > 1)	(void) SelectObject(hDC, hOldPen);
}

void DrawControl (HDC hDC, int xa, int ya, int xb, int yb){
	HPEN hOldPen=NULL;
/*	if (bDebug > 1) {
		sprintf(debugstr, "control %d %d  %d %d", xa, ya, xb, yb);
		OutputDebugString(debugstr);
	} */
	hOldPen = SelectObject (hDC, hLinerPen); 
	MoveToEx(hDC, xa, ya, NULL);
	LineTo(hDC, xb, yb);
	if (bDrawKnots) DrawKnot(hDC, xb, yb, 1);
	MoveToEx(hDC, xa, ya, NULL);
	if ((UINT) hOldPen > 1)	(void) SelectObject(hDC, hOldPen);
}

/* convert from int + WORD to double */

/* double fixtofloat(int value, WORD fract) {
	return (double) (((long) value << 16) | fract) / 65536.0;
} */

double fixedtodouble (FIXED z) {
	return (double) (((long) z.value << 16) | z.fract) / 65536.0;
}

// int debugcounter=0;

int yshift;			// character ascent - so top left is at (1", 1")

// The factor of 19 = 16 * (1 + 1/5), but where the 4 comes from ???

int mapoutlinex (double xs) {
	int xd;
//	xd = (int) mapx((long) xs);
//	dvi_h = oneinch + (long) xs * charw / 1000 * 16;
//	dvi_h = oneinch + (long) (xs * charh);
//	dvi_h = oneinch + (long) (xs * 1000 * 16);
//	dvi_h = oneinch + (long) (xs * 1000 * 16 * 4);
	dvi_h = oneinch + (long) (xs * 1000 * 19 * 4);
//	dvi_h = oneinch + (long) (xs * 1000);
	xd = (int) mapx(dvi_h);
//	if (debugcounter++ < 10) {
//		sprintf(debugstr, "xs %lg xd %d", xs, xd);
//		wincancel(debugstr);
//	}
	return xd;
}

int mapoutliney (double ys) {
	int yd;
//	yd = (int) mapy((long) ys);
//	dvi_v = oneinch + (long) ys * charw / 1000 * 16;
//	dvi_v = oneinch + (long) (ys * charh);
//	dvi_v = oneinch + (long) ((1200 - ys) * 1000 * 16);
//	dvi_v = oneinch + (long) ((1200 - ys) * 1000 * 16 * 4);
// 	dvi_v = oneinch + (long) ((yshift - ys) * 1000 * 16 * 4);
	dvi_v = oneinch + (long) ((yshift - ys) * 1000 * 19 * 4);
//	dvi_v = oneinch + (long) ((1200 - ys) * 1000);
	yd = (int) mapy(dvi_v);
//	if (debugcounter++ < 10) {
//		sprintf(debugstr, "ys %lg yd %d", ys, yd);
//		wincancel(debugstr);
//	}
	return yd;
}

/* This gets called from ShowFontSample in  WM_PAINT */
/* UID is Unicode for reencoded fonts, else just char code */

int DrawOutlineW (HWND hWnd, HDC hDC, UINT UID) {
	GLYPHMETRICS GM;
	MAT2 Mat;
	int nLen, flag;
	int xll, yll, xur, yur, charwidth;
	char *lpPoints;
	HPEN hOldPen;

	Mat.eM11.value=1;	Mat.eM12.value=0;
	Mat.eM21.value=0;	Mat.eM22.value=1;
	Mat.eM11.fract=0;	Mat.eM12.fract=0;
	Mat.eM21.fract=0;	Mat.eM22.fract=0;

/*	get size of buffer needed for outline data (and metrics) */
	if (bFontEncoded) nLen = GetGlyphOutlineW(hDC, UID,
			GGO_BEZIER | GGO_NATIVE, &GM, 0, NULL, &Mat);
	else nLen = GetGlyphOutlineA(hDC, UID,
			GGO_BEZIER | GGO_NATIVE, &GM, 0, NULL, &Mat);
	if (nLen <= 0) {
		if (bDebug > 1) {
			sprintf(debugstr, "nLen %d", nLen);
			OutputDebugString(debugstr);
		}
		return -1;
	}
/*	advance width and character bounding box */
	charwidth = GM.gmCellIncX;
	xll = GM.gmptGlyphOrigin.x;
	yll = GM.gmptGlyphOrigin.y - GM.gmBlackBoxY;
	xur = GM.gmptGlyphOrigin.x + GM.gmBlackBoxX;
	yur = GM.gmptGlyphOrigin.y;
	if (bDebug > 1) {
		sprintf(debugstr, "UID %d width %d nLen %d xll %d yll %d xur %d yur %d\n",
				UID, charwidth, nLen, xll, yll, xur, yur);
		OutputDebugString(debugstr);
	}

	if (! bDrawOutline) return 0;		/* don't go on if disabled */

/*	Draw character `bounding box' and baseline 2000/June/24 */
	{
		int xkz, ykz, xkw, yka, ykd;
		(void) GetTextMetrics(hDC, &TextMetric);	// use of global OK ?
		yshift = (int) ((double) TextMetric.tmAscent / 9.45);
//		sprintf(debugstr, "GM.gmCellIncX; %d TextMetric.tmAveCharWidth %d",
//					GM.gmCellIncX, TextMetric.tmAveCharWidth);
//		wincancel(debugstr);		// debugging only
		xkz = mapoutlinex(0);
		ykz = mapoutliney(0);
		MoveToEx(hDC, xkz, ykz, NULL);
		xkw = mapoutlinex(charwidth);
		LineTo(hDC, xkw, ykz);
//		We do not know why 9.45 is the right scale factor ???
		yka = mapoutliney((double) TextMetric.tmAscent / 9.45);
		ykd = mapoutliney(- (double) TextMetric.tmDescent / 9.45);
		LineTo(hDC, xkw, yka);
		LineTo(hDC, xkz, yka);
		LineTo(hDC, xkz, ykd);
		LineTo(hDC, xkw, ykd);
		LineTo(hDC, xkw, ykz);
	}						// experiment

	lpPoints = (char *) LocalAlloc(LPTR, nLen);
	if (lpPoints == NULL) {
		if (bDebug > 1) {
			sprintf(debugstr, "memory allocation error %d", nLen);
			OutputDebugString(debugstr);
		}
		return -1;
	}
/*	get actual outline data TTPOLYGONHEADER + n * TTPOLYCURVE */
	if (bFontEncoded) flag = GetGlyphOutlineW(hDC, UID,
					  GGO_BEZIER | GGO_NATIVE, &GM, nLen, lpPoints, &Mat);
	else flag = GetGlyphOutlineA(hDC, UID,
					GGO_BEZIER | GGO_NATIVE, &GM, nLen, lpPoints, &Mat);
	hOldPen = SelectObject(hDC, hBlackPen);
	{
		TTPOLYGONHEADER *lpPolyHead;
		DWORD cb;				/* bytes in this contour */
		DWORD dwType;			/* should be TT_POLYGON_TYPE */
		double xs, ys, xn, yn;
		TTPOLYCURVE *lpPolyCurve;
		WORD wType;				/* TT_PRIM_LINE, QSPLINE or CSPLINE */
		WORD cpfx;				/* how many POINTFX structures */
		POINTFX *apfx;
		POINT bpoints[3];
		int k, kk, xks, yks, xkn=0, ykn=0;
		char *s;
//		double xoffset=200.0;
//		double yoffset=-1600.0;
//		double scale=5.0;

		lpPolyHead = (TTPOLYGONHEADER *) lpPoints;	/* first one */
		for (;;) {				/* loop over contours */
			if ((char *)lpPolyHead >= (((char *) lpPoints) + nLen)) break;
			cb = lpPolyHead->cb;		/* bytes in this contour */
			dwType = lpPolyHead->dwType;	/* sould be TT_POLYGON_TYPE */
			if (bDebug > 1) {
				sprintf(debugstr, "%08d cb %lu dwType %lu",	lpPolyHead, cb, dwType);
				OutputDebugString(debugstr);
			}
			if (cb == 0) break;				/* should not happen */
			if (cb > 10000) break;			/* sanity check */
			if (dwType != TT_POLYGON_TYPE) break; /* should not happen */
			xs = xn = fixedtodouble(lpPolyHead->pfxStart.x);
			ys = yn = fixedtodouble(lpPolyHead->pfxStart.y);
			if (bDebug > 1) {
				sprintf(debugstr, "%lg %lg moveto", xs, ys);
				OutputDebugString(debugstr);				
			}
			if (bDrawOutline) {
//				xks = xkn = (int) ((xs + xoffset) * scale);
//				yks = ykn = (int) ((ys + yoffset) * scale);
				xks = xkn = mapoutlinex(xs);
				yks = ykn = mapoutliney(ys);
				MoveToEx(hDC, xks, yks, NULL);
				if (bDrawKnots) DrawKnot(hDC, xkn, ykn, 0);
			}

			lpPolyCurve = (TTPOLYCURVE *) (((char *) lpPolyHead) +
										   sizeof(TTPOLYGONHEADER));
			for (;;) {		/* loop over polycurves in this contour */
				if ((char *)lpPolyCurve >= (((char *) lpPolyHead) + cb)) break;
				wType = lpPolyCurve->wType;	/* should be 1, 2 or 3 */
				cpfx = lpPolyCurve->cpfx;	/* number of points */
				apfx = lpPolyCurve->apfx;	/* array of POINTFX */
				if (bDebug > 1) {
					sprintf(debugstr, "%08d wType %u cpfx %u",
							lpPolyCurve, wType, cpfx);
					OutputDebugString(debugstr);
				}
				if (cpfx == 0) break;				/* should not happen */
				if (cpfx > 1000) break;				/* sanity check */
				if (wType < 1 || wType > 3) break;
				switch (wType) {
					case TT_PRIM_LINE:				/* PolyLineTo */
						for (k = 0; k < cpfx; k++) {
							xn = fixedtodouble(apfx[k].x); 
							yn = fixedtodouble(apfx[k].y); 
							if (bDebug > 1) {
								sprintf(debugstr, "%lg %lg lineto", xn, yn);
								OutputDebugString(debugstr);				
							}
							if (bDrawOutline) {
//								xkn = (int) ((xn + xoffset) * scale);
//								ykn = (int) ((yn + yoffset) * scale);
								xkn = mapoutlinex(xn);
								ykn = mapoutliney(yn);
								LineTo(hDC, xkn, ykn);
								if (bDrawKnots) DrawKnot(hDC, xkn, ykn, 0);
							}
						}
						break;

					case TT_PRIM_CSPLINE:			/* PolyBezierTo */
						s = debugstr;
						kk = 0;
						for (k = 0; k < cpfx; k++) {
							xn = fixedtodouble(apfx[k].x); 
							yn = fixedtodouble(apfx[k].y); 
							sprintf(s, "%lg %lg ", xn, yn);
//							bpoints[kk].x = (int) ((xn + xoffset) * scale);
//							bpoints[kk].y = (int) ((yn + yoffset) * scale);
							bpoints[kk].x = mapoutlinex(xn);
							bpoints[kk].y = mapoutliney(yn);
							kk++;
							s += strlen(s);
							if (((k + 1) % 3) == 0) {
								if (bDebug > 1) {
									strcpy (s, "curveto");
									OutputDebugString(debugstr);
								}
								if (bDrawOutline) {
									if (bDrawControls)
										DrawControl(hDC, xkn, ykn,
											bpoints[0].x, bpoints[0].y);
									PolyBezierTo(hDC, bpoints, 3);
									if (bDrawControls)
										DrawControl(hDC, bpoints[2].x, bpoints[2].y,
											bpoints[1].x, bpoints[1].y);
									if (bDrawKnots)
										DrawKnot(hDC, bpoints[2].x, bpoints[2].y, 0);
									xkn = bpoints[2].x;
									ykn = bpoints[2].y;
								}
								kk = 0;
							}
						}
						break;

					case TT_PRIM_QSPLINE:
						if (bDebug > 1) OutputDebugString("UNEXPECTED QUADRATIC SPLINE");
						for (k = 0; k < cpfx; k++) ;
						break;
					default:
						if (bDebug > 1) OutputDebugString("UNEXPECTED POLYCURVE TYPE");
						break;
				}
				lpPolyCurve = (TTPOLYCURVE *)
						  (((char *) lpPolyCurve) + sizeof(TTPOLYCURVE)
						   + (cpfx-1) * sizeof(POINTFX));
/*				if ((char *)lpPolyCurve >= (((char *) lpPolyHead) + cb)) break; */
			}	/* loop over POLYCURVEs */
			if (xn != xs || yn != ys) {
				if (bDebug > 1) {
					sprintf(debugstr, "%lg %lg lineto", xs, ys);
					OutputDebugString(debugstr);				
				}
				if (bDrawOutline) {
//					xks = (int) ((xs + xoffset) * scale);
//					yks = (int) ((ys + yoffset) * scale);
					xks = mapoutlinex(xs);
					yks = mapoutliney(ys);
					LineTo(hDC, xks, yks);
				}
				if (bDebug > 1) OutputDebugString("closepath");
			}
/*			lpPolyHead = (TTPOLYGONHEADER *) ((char *) lpPolyCurve); */
			lpPolyHead = (TTPOLYGONHEADER *) (((char *) lpPolyHead) + cb);
/*			if ((char *)lpPolyHead >= (((char *) lpPoints) + nLen)) break; */
		} /* loop over contours */
		{
			int k, nMiss = (((char *) lpPoints) + nLen) - (char *) lpPolyHead;
			char *s, *t;
			if (nMiss > 0) {
				if (bDebug > 1) {
					sprintf(debugstr, "%d extra bytes", nMiss);
					OutputDebugString(debugstr);
					s = (char *) lpPolyHead;
					t = debugstr;
					for (k = 0; k < nMiss; k++) {
						sprintf(t, "%u ", (unsigned char) *s++);
						t = t + strlen(t);
					}
					OutputDebugString(debugstr);
				}
			}
		}
	}
	LocalFree(lpPoints);
	if ((UINT) hOldPen > 1)	(void) SelectObject(hDC, hOldPen);
	return 0;
}

/* What character hit in font display 99/Jan/10 returns character code or -1 */

int ShowWhatChar (HWND hWnd, int curx, int cury) {
	POINT steparr[1];
	int tagx, tagy;
	long dvi_h, dvi_v;
	int i, j, k;
	unsigned long old_dvi_num, old_dvi_den, old_dvi_mag;
	int old_wantedzoom;				/* remembered view scale */
	HWND hFocus;
	HDC hDC;

	if (charw == 0 || charh == 0) return -1;	/* sanity 99/June/20 */

	old_dvi_num = dvi_num;
	old_dvi_den = dvi_den;
	old_dvi_mag = dvi_mag;
	old_wantedzoom = wantedzoom;
	dvi_num = 25400000;
	dvi_den = 473628672;
	dvi_mag = 1000;
//	setscale(wantedzoom);

	hDC = GetDC(hWnd);			
	(void) SetMapMode(hDC, MM_TWIPS);
	steparr[0].x = curx;
	steparr[0].y = cury;
	(void) DPtoLP(hDC, steparr, 1);
	tagx = steparr[0].x;  tagy = steparr[0].y; 
/*	(void) ReleaseDC(hWnd, hDC); */
	dvi_h = unmap(tagx - xoffset);
	dvi_v = unmap(yoffset - tagy);
	j = (dvi_h - oneinch) / (charw * 6 / 5);
	i = (dvi_v - oneinch) / (charh * 6 / 5); 
/*	i = (dvi_v - ONEINCH) / (charh * 6 / 5); */
//	if (bDebug > 1) {	// debugging only
//		sprintf(debugstr, "curx %d cury %d tagx %d tagy %d dvi_h %d dvi_v %d i %d j %d",
//				curx, cury,tagx, tagy, dvi_h, dvi_v, i, j);	
//		OutputDebugString(debugstr);
//		wincancel(debugstr);	// debugging only
//	}
	if (i >= 0 && i < 16 && j >= 0 && j < 16) k = (i << 4) | j;
	else k = -1;
	if (k >= 0) {
/*	sprintf(str, "curx %d cury %d tagx %d tagy %d dvi_h %ld dvi_v %ld i %d j %d k %d charw %d charh %d chara %d",
			curx, cury, tagx, tagy, dvi_h, dvi_v, i, j, k, charw, charh, chara); */
		sprintf(str, "Character Code: %c\n\n%03d\tdecimal\n%03X\thexadecimal\n%03o\toctal",
				(k > 32) ? k : 32, k, k, k);
		if ((hFocus = GetFocus()) == NULL)
			hFocus = hWnd;
		(void) MessageBox(hFocus, str,
						  "Character Info", 
						  MB_ICONINFORMATION | MB_OK);
		bCharacterSample = 1;					// switch to showing outline
//		nChar = -1;								// initial value ?
		nChar = k;								// initial value ?
	}
/*	nChar = k; */
	dvi_num = old_dvi_num;
	dvi_den = old_dvi_den;
	dvi_mag = old_dvi_mag;
	wantedzoom = old_wantedzoom;
//	setscale(wantedzoom);						/* restore zoom */
/*	ShowFontSample (hWnd, hDC, k); */
	(void) ReleaseDC(hWnd, hDC);
	return k;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* BEYOND HERE EVERYTHING IS IGNNORED / EXPERIMENTAL */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* from winreg.h */

#ifdef IGNORED
WINADVAPI
LONG
APIENTRY
RegEnumValueA (
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );
WINADVAPI
LONG
APIENTRY
RegEnumValueW (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );
#endif

/* in Windows 95, make the following WinExec call to set up TT font data*/
/* switch to Windows directory first? */
/* insert windows directory in /E filename ? */

/* regedit /E c:\windows.000\ttfonts.reg
   HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Fonts */

/* char *szRegistryFonts=
 "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts"; */

/* Problem in Windows 95 after trying registry it still tries to dump */
/* registry to file using regedit.exe - it should not ... */

/* #define TT_POLYGON_TYPE   24 */

/* #define TT_PRIM_LINE       1 */
/* #define TT_PRIM_QSPLINE    2 */
/* #define TT_PRIM_CSPLINE    3 */

/* outline consists of one or more TTPOLYGONHEADERS followed by one or */
/* more TTPOLYCURVEs.  Each of these contains one or more POINTFXs */

/*   9-10-1999  12:00:00a   16,257,484  batang.ttc */
/*   9-10-1999  12:00:00a   13,517,576  gulim.ttc */
/*   9-10-1999  12:00:00a    8,637,684  mingliu.ttc */
/*   9-10-1999  12:00:00a   10,500,352  simsun.ttc */
/*   9-10-1999  12:00:00a    8,272,028  msgothic.ttc */
/*   9-10-1999  12:00:00a    9,136,456  msmincho.ttc */
