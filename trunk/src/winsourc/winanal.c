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

#include "ddeml.h"

#include <errno.h>

#include <malloc.h>

#include "atm.h"
#include "atmpriv.h"

#pragma warning(disable:4100)	// unreferenced formal parameters

// #define DEBUGWIDTH 

// #define DEBUGRULE 

// #define DEBUGDDE

// #define DEBUGHEAP

// #define DEBUGFACES

#define DEBUGFONTS

/* stdlib.h not really needed - just for hdrstop */
/* dviwindo.h not really needed - just for hdrstop */
/**********************************************************************
*
* DVI analyzer for Adobe Type 1 (ATM compatible) fonts
*
* Copyright (C) 1991, 1992, Y&Y. All Rights Reserved. 
* DO NOT COPY OR DISTRIBUTE!
*
* This is the part that actually draws characters and rules on the screen
*
**********************************************************************/

/* NOTE: ATMSelectObject does not work in Windows NT */
#define MyATMSelectObject ATMSelectObject
#define MyATMFontSelected ATMFontSelected

BOOL bDdeClientBusy=0;			/* flag to prevent reentrancy in DDE client */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* int text_r=0, text_g=0, text_b=0; */
/* int rule_r=0, rule_g=0, rule_b=0; */

/* BOOL bItalicFlag; */	/* communicates with FontSelect ? *//* not accessed */
/* BOOL bBoldFlag;	*/	/* communicates with FontSelect ? *//* not accessed */

BOOL bReverseVideo=0;			/* reverse video for a while */

BOOL bMarkSpotFlag=0;			/* indicated we wish to mark the spot */

long dvi_spot_h, dvi_spot_v;	/* x marks the spot in search */

int maxdrift=0;					/* maximum drift allowed - one pixel */

unsigned int maxrulewidth=20 * 4;/* threshold for using pen when UseRect = 2 */
								 /* in TWIPS we assume ? */

long atsize;				/* at size of currently selected font */
long wordspace;				/* 0.2 * quad, where quad = atsize */
long backspace;				/* 0.9 * quad, where quad = atsize */
long verspace;				/* 0.8 * quad, where quad = atsize */

int descent;				/* descent (in twips) of selected font */
int ascent;					/* ascent (in twips) of selected font */ /* NA */

BOOL extentflag, getcharflag, getabcflag;	/* where do widths come from */
							/*  GetWidth: GETEXTENTTABLE, GetWidths, GetABC */

BOOL bSkipFlag;				/* on when page to be skipped reading forward */
BOOL bShowFlag;				/* zero if not showing text on screen */

/* BOOL bLeftPage = 0; */	/* working on left page of spread */ /* NA */

HDC hdc=NULL;				/* device-context variable */

BOOL bCorrectFlag = 0;		/* correct for apparent error in ATM sizing */
							/* To avoid need for this, go into ATM.INI */
							/* in [Settings] add ScreenAdjust=Off */

BOOL bSignFlag = 1;			/* use character height instead of cell height */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* current state of DVI interpretation */

long dvi_h, dvi_v, dvi_w, dvi_x, dvi_y, dvi_z; 

int dvi_hh, dvi_vv;					/* in logical Window coords - TWIPS */

/* need stack for (h, v, w, x, y, z) and push and pop */

int stackinx;						/* index into stack */

/* use global memory for this maybe ? 256 * 4 * 6 = 6144 */

long stack[MAXDVISTACK * 6];		/* push h, v, w, x, y, z each time */

int charbufinx;						/* pointer into charbuf */

/* unsigned char charbuf[MAXCHARBUF+1]; */ /* buffer accumulated characters */ 

/* char charbuf[MAXCHARBUF+1]; */  /* buffer accumulated characters */

unsigned char charbuf[MAXCHARBUF+1];  /* buffer accumulated characters */

int charwid[MAXCHARBUF+1];		/* character widths - logical units */

#ifdef USEUNICODE
WCHAR charbufW[MAXCHARBUF+1];	/* UNICODE version of charbuf */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

WORD basefont[MAXFONTS];		/* font of same name and style */

/* #if MAXFONTS > 255 */
/* #error MAXFONTS > 255 and using unsigned char */
/* unsigned char basefont[MAXFONTS]; */

/* int *fontcharptr; */	/* pointer into data for current font */ /* used ? */

/* font k is to be used at (mag * fs) / (1000 * fd) times its normal size */

/* watch out, fnt_def1 and xxx1 can appear between eop and bop */

/* NOTE: S = s w, T = s x, W = w<n>, X = x<n>, Y = y<n>, Z = z<n> */
/* bp = bop, ep = eop, pr = put_rule, sr = set_rule */

/* int firstpage=0; */		/* non-zero when nothing has been output yet */

/* int skiptoend=0; */		/* non-zero => still need to skip to last page */

int cp_valid=0; 		/* non-zero when last sent out "set" (NOT "put") */ 
						/* zero when need to set current point in window */

int fnt_exists=0;		/* non-zero current if font was found by ATM */
						/* if zero, don't use TextOut ? */

int fnt_texatm=0;		/* non-zero means TeX CM or LaTeX font */
						/* needs remapping to deal with bug in ATM */

int fnt_ansi=0;			/* font is ANSI encoded *and* needs remap */

int page_seen=0;		/* triggered when desired page has been hit */


/* static int histogram[MAXCHRS]; */		/* counts of DVI commands seen */

/* long h; */		/* horizontal position */
/* long v; */		/* vertical position */

/* long w; */		/* horizontal spacing */	/* not referenced ??? */
/* long x; */		/* horizontal spacing */
/* long y; */		/* vertical spacing */
/* long z; */		/* vertical spacing */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static char *modname = "WINANAL";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* remapping for TeX text accents and special chars (in Windows ANSI)*/
/* remap bad positions to bullet to warn user about encoding problem */
/* NOTE: STILL need to \input ansiacce because need active ` and ' */

unsigned char ansitex[32] = { 
149, 149, 149, 149, 149, 149, 149, 149,		/* upright uppercase Greek */
149, 149, 149, 149, 149, 149, 149, 149,		/* Psi, Omega, ff fi fl ffi ffl */
'i', 'j',  96, 180, 149, 149, 175, 176,		/* dotlessi, dotlessj, accents */
184, 223, 230, 156, 248, 198, 140, 216		/* ss ae oe oslash AE OE Oslash */
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static void winerror(char *);

//	supposedly, _heapwalk is only supported in Windows NT
//	supposedly, HeapValidate is only supported in Windows NT
//	the GlobalAlloc however may crash the system and catch ...

void CheckHeap (char *s, int verboseflag) {
	int flag=0;
	char *buffer = malloc(1);
//	int status = _heapset('Z');
	int status = _heapchk();
	strcpy(debugstr, s);
	strcat(debugstr, " ");
	switch(status) {
		case _HEAPBADBEGIN:
			strcat(debugstr, "HEAP BAD START");
			flag++;
			break;
		case _HEAPBADNODE:
			strcat(debugstr, "HEAP BAD NODE");
			flag++;
			break;
		case _HEAPBADPTR:
			strcat(debugstr, "HEAP BAD POINTER");
			flag++;
			break;
		case _HEAPEMPTY:
			strcat(debugstr, "OK: HEAP EMPTY");
			break;
		case _HEAPOK:
			strcat(debugstr, "OK: HEAP FINE");
			break;
	}
	if (flag || verboseflag) wininfo(debugstr);
	if (errno == ENOSYS) winerror("errno == ENOSYS");
//	contrary to MS info:	 ENONT == 2
	free(buffer);
	if (HeapValidate(GetProcessHeap(), 0, NULL) == 0) {
		winerror("HEAP INVALID");
	}
#ifdef IGNORED
	{
		HGLOBAL hInfo;
		LPATMInstanceInfo lpInfo;
		int BufSize=572;
		hInfo = GlobalAlloc (GMEM_MOVEABLE, BufSize);
		lpInfo = GlobalLock (hInfo);
		GlobalUnlock(hInfo);
		GlobalFree(hInfo);
	}
#endif
}

/* put up error message in box - with byte count if relevant */

static void winerror(char *buff) {
	HWND hFocus;
	char errmess[MAXMESSAGE];

	if (strlen(buff) > (MAXMESSAGE - 32)) buff = "Error Message too long!";
	if (filepos > 0) {
		sprintf(errmess, "%s at byte %ld", buff, filepos-1);
/*				(LPSTR) buff, filepos-1); */
	} 
	else strcpy(errmess, buff);

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox(hFocus, errmess, modname, MB_ICONSTOP | MB_OK);
}

static int wincancel(char *buff) {				/* 1993/March/15 */
	HWND hFocus;
	int flag;
	char errmess[MAXMESSAGE];

	if (strlen(buff) > (MAXMESSAGE - 32)) buff = "Error Message too long!";
	if (filepos > 0) {
		sprintf(errmess, "%s at byte %ld", buff, filepos-1);
/*				(LPSTR) buff, filepos-1); */
	} 
	else strcpy(errmess, buff);

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, errmess, modname, MB_ICONSTOP | MB_OKCANCEL);
	if (flag == IDCANCEL) {
		bShowFlag = 0;				/* turn off displaying */
		finish = -1;				/* set early termination */
		bUserAbort = 1;				/* stop printing */
		return -1;					/* 95/Aug/12 */
	}
	else return 0;					/* 95/Aug/12 */
}

/* new in 98/Nov/19 */ /* warnflag means show on screen */
/* use either err, or if err < 0 use GetLastError() */

int ExplainError(char *szMsg, int err, int warnflag) {
	LPVOID lpMsgBuf=NULL;
	LPVOID lpFullMsg=NULL;
	int nLen, flag=0;
	
	if (warnflag == 0 && bDebug <= 1) return 0;

	if (bDebug > 1) OutputDebugString( szMsg );

	if (err == -1) err = GetLastError();
	FormatMessage( 
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		  FORMAT_MESSAGE_FROM_SYSTEM | 
		  FORMAT_MESSAGE_IGNORE_INSERTS,
		  NULL,
		  err,						/* GetLastError(), */
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		  (LPTSTR) &lpMsgBuf,
		  0,
		  NULL);
// Process any inserts in lpMsgBuf.
// ...
// Display the string.
//	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	if (lpMsgBuf != NULL) {
		if (bDebug > 1) OutputDebugString( lpMsgBuf );
	}
	if (warnflag) {
		if (lpMsgBuf != NULL) nLen = strlen(szMsg) + strlen(lpMsgBuf) + 1;
		else nLen = strlen(szMsg) + 1;
		lpFullMsg = LocalAlloc(LMEM_FIXED, nLen);
		if (lpFullMsg == NULL) {
			winerror ( szMsg );
			if (lpMsgBuf != NULL) flag = wincancel( lpMsgBuf );
		}
		else {
			strcpy(lpFullMsg, szMsg);
			if (lpMsgBuf != NULL) strcat(lpFullMsg, lpMsgBuf);
			flag = wincancel( lpFullMsg );
			LocalFree( lpFullMsg );
		}
	}

// Free the buffer again
	if (lpMsgBuf != NULL) LocalFree( lpMsgBuf );
	return flag;
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef DEBUGWIDTH
void DebugWidthCode (HDC);
#endif

typedef struct {
	BYTE chFirst;
	BYTE chLast;
} CHAR_RANGE_STRUCT;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

HGLOBAL GetATMFontInfo(HDC, WORD);			/* in winfonts.c */

#ifdef USEUNICODE
WCHAR encodeUID[256];		/* table of mappings char code => UID */
#endif

/* In WIN16 the following holds: */
/* It appears that the GETEXTENTTABLE escape works for Type 1 fonts */
/* and the GetCharWidth call works for TrueType fonts */
/* GETEXTENTTABLE always yields char widths for 1000 point font */
/* GetCharWidth yields char widths for the font size requested */

/* In WIN32 it appears that GetTextExtentPoint32 and relatives work */
/* And it appears that GetCharWidth does not work in 95? */
/* And it appears that GetCharWidth does work in NT? */

/* presently always called with first = 0 and last = 255 */
/* this assumption is now built in ... */

/* int GetWidth(HDC hDC, int first, int last, LPINT widths) */
int GetWidth(HDC hDC, int first, int last, LPSHORT widths) {
	int k;
	int ret;
	char chars[2];
	DWORD err=0;
/*	int NewFlag=1, OldFlag=0; */
	SIZE TextSize;
/*	int flag; */
	CHAR_RANGE_STRUCT CharRange;
/*	Following take a lot of stack space ... share the space ? */
	HGLOBAL hInfo=NULL;				/* 1996/July/28 */
	LPATMInstanceInfo lpInfo=NULL;	/* 1996/July/28 */
	LPSTR lpATMWidths;				/* 1996/July/28 */
/*	for WIN32, the GETEXTENTTABLE Escape uses int (32 bit) widths ... */
/*	short shortwidths[MAXCHRS]; */ 	/* temporary for widths GETEXTENTTABLE */
	int charwidths[MAXCHRS]; 		/* temporary for widths GetCharWidth */
	int n;

/*	int charwidths[MAXCHRS]; */

#ifdef DEBUGGING
	int bad;
#endif
	int ttffontflag=0;

/*	if (first != 0 || last != MAXCHRS-1) winerror("BAD"); */

#ifdef DEBUGWIDTH
	if (bDebug > 1) OutputDebugString("GetWidth\n");
#endif
	if (widths == NULL) {
		winerror("widths NULL");
		return 0;
	}

#ifdef IGNORED
	if (first < 0 || last < first || last > 255) {
		sprintf(debugstr, "first %d last %d", first, last);
		winerror(debugstr);
		return 0;				/* failed */
	}
#endif

#ifdef DEBUGHEAP
	CheckHeap("GetWidth", 0);
#endif

#ifdef DEBUGWIDTH
	if (bDebug > 1) DebugWidthCode (hDC);
#endif

	if (bATMLoaded && MyATMFontSelected(hDC)) ttffontflag = 0;
	else ttffontflag = 1;						/* 97/Jan/25 */

#ifdef USEUNICODE
/*	if (bUseNewEncode) goto UseGetExtent; */	/* 97/Jan/17 */
/*	if use new encoding method and TTF font or in Windows NT */
	if ((bUseNewEncodeTT && ttffontflag) ||
		(bUseNewEncodeT1 && !ttffontflag)) goto UseGetExtent;	/* 97/Jan/25 */
#endif

#ifdef DEBUGHEAP
	CheckHeap("GetWidth", 0);
#endif

/*	goto UseGetExtent; */			/* seems to be the way to go ??? */
/*	we don't know whether this works in print DC context ??? */
/*	actually, is this ever called in printer DC context ??? */
/*	for this, we don't actually need special METRICSIZE font ... */
	if (bUseATMFontInfo) {
/*		if (MyATMFontSelected(hDC)) */
		if (bATMLoaded && MyATMFontSelected(hDC)) {
/*			hInfo = GetATMFontInfo(hDC, ATM_GETWIDTHS, 0); */
			hInfo = GetATMFontInfo(hDC, ATM_GETWIDTHS);
			if (hInfo != NULL) {
#ifdef DEBUGWIDTH
				if (bDebug > 1) OutputDebugString("Using ATMFontInfo\n");
#endif
				lpInfo = GlobalLock(hInfo);
				n = lpInfo->num_chars;
				if (n != MAXCHRS) {
					if (bDebug > 1) {
						sprintf(debugstr, "num_chars %d < %d\n", n, MAXCHRS);
						OutputDebugString(debugstr);
					}
				}
				lpATMWidths = (LPSTR) lpInfo + lpInfo->widthsOffset;
/*				memcpy((LPSTR) widths, lpATMWidths, MAXCHRS * sizeof(short)); */
				memcpy((LPSTR) widths, lpATMWidths, n * sizeof(short));
/*	heuristic: if widths is zero character missing - at least at start */
				for (k = 0; k < n; k++) {
					if (widths[k] == 0)	widths[k] = lpInfo->missing_width;
					else break;
				}
/*	fill in the blanks at the end also */
				if (n < MAXCHRS) {
/*					memset((LPSTR) widths + n * sizeof(short), 0,
						   (MAXCHRS - n) * sizeof(short)); */
					for (k = n; k < MAXCHRS; k++)
						widths[k] = lpInfo->missing_width;
				}
				GlobalUnlock(hInfo);
				GlobalFree(hInfo);
				return 1;						/* 1996/Aug/14 fix */
			} /* if hInfo != NULL */
		} /* if bATMLoaded and current font is ATM font */
	} /* if bATMUseFontInfo */
/*	drop through if the above did not work for some reason */
/* #endif */

	if (bUseGetExtent) goto UseGetExtent;	/* force it -- if asked for */

	extentflag = getcharflag = getabcflag = 0;

	CharRange.chFirst = (BYTE) first;
	CharRange.chLast = (BYTE) last;

/*	debugging code can be inserted here */

/*	memset(charwidths, 0, MAXCHRS * sizeof(int)); */	/* not needed ? */
/*	memset(shortwidths, 0, MAXCHRS * sizeof(short)); */

	if (bPrintFlag == 0) {					/* first for normal screen DC */
/*		Appears to be how to get accurate width information on screen WIN16 */

/*		NOTE: GETEXTENTTABLE Escape does not work in WIN32 */
		extentflag = ExtEscape(hDC, GETEXTENTTABLE, sizeof(CHAR_RANGE_STRUCT), 
/*			(LPSTR) &CharRange, sizeof(shortwidths), (LPSTR) &shortwidths); */
			(LPSTR) &CharRange, sizeof(charwidths), (LPSTR) &charwidths);

/*		extentflag = Escape(hDC, GETEXTENTTABLE, sizeof(CHAR_RANGE_STRUCT), 
			(LPSTR) &CharRange, (LPSTR) &charwidths); */

/*		extentflag < 0 is error, extentflag = 0 is not implemented */
/*		if (extentflag == 0) */
		if (extentflag <= 0) {		/* if that fails we try GetCharWidth */
/*			NOTE: GetCharWidth does not work in Windows 95! */

			getcharflag = GetCharWidth32(hDC, first, last, charwidths);
/*			getcharflag = GetCharWidth(hDC, first, last, charwidths); */

/*	if (getcharflag == 0) wincancel("Problems Getting Character Widths"); */
		}
	}
	else {		/* do we ever use GetWidth with bPrintFlag != 0 ??? */
/*		This appears to be the way to get information for printer WIN16 */
/*		NOTE: GetCharWidth does not work in Windows 95 */

		getcharflag = GetCharWidth32(hDC, first, last, charwidths);

/*		getcharflag = GetCharWidth(hDC, first, last, charwidths); */

		if (getcharflag == 0) {		/* if that fails we try the Escape */
/*			NOTE: GETEXTENTTABLE Escape does not work in WIN32 */

			extentflag = ExtEscape(hDC, GETEXTENTTABLE, sizeof(CHAR_RANGE_STRUCT),
/*				(LPSTR) &CharRange, sizeof(shortwidths), (LPSTR) &shortwidths); */
				(LPSTR) &CharRange, sizeof(charwidths), (LPSTR) &charwidths);

/*			extentflag = Escape(hDC, GETEXTENTTABLE, sizeof(CHAR_RANGE_STRUCT),
				(LPSTR) &CharRange, (LPSTR) &charwidths); */

/*	if (extentflag <= 0) wincancel("Problems Getting Character Widths"); */
		}
	}

/*	If above fails, try following. Works TrueType fonts --- 1996/Aug/5 */
	if (extentflag == 0 && getcharflag == 0) {
		ABC abc[MAXCHRS];	/* 256 * (sizeof(int)+sizeof(int)+sizeof(int)) */
		getabcflag = GetCharABCWidths (hDC, 0, MAXCHRS-1, abc);
		if (getabcflag) {
#ifdef DEBUGWIDTH
		if (bDebug > 1) OutputDebugString("Using GetABCCharWidths result\n");
#endif
			for (k = 0; k < MAXCHRS; k++) 
/*				widths[k] = (short) abc[k].abcA + abc[k].abcB + abc[k].abcC; */
				widths[k] = (short) (abc[k].abcA + abc[k].abcB + abc[k].abcC); 
			return 1;				/* success */
		}
	}

/*	Now copy the width information if extentflag != 0 or getcharflag != 0 */
	if (extentflag > 0) {
#ifdef DEBUGWIDTH
		if (bDebug > 1) OutputDebugString("Using GETEXTENTTABLE result\n");
#endif
		for (k = first; k <= last; k++)
			widths[k] = (short int) charwidths[k];
		return 1;
	}
	else if (getcharflag != 0) {
#ifdef DEBUGWIDTH
		if (bDebug > 1)	OutputDebugString("Using GetCharWidth result\n");
#endif
		for (k = first; k <= last; k++)
			widths[k] = (short int) charwidths[k];
		return 1;
	}
/*	GetCharWidth32, ExtEscape GETTEXTENTTABLE *and* GetABCCHarWidths failed: */
	else {
/*		wincancel("Problems Getting Character Widths"); */
		if (bDebug) wincancel("Problems Getting Character Widths");
	}

/*	drop through if the above don't work */

UseGetExtent:

/*	NOTE: GetTextExtentPoint gives widths 3% too narrow for Type 1 fonts */
/*	unless ScreenAdjust=Off in [Settings] of ATM.INI */
/*	Doing it the slow way ... character by character ... */

#ifdef DEBUGWIDTH
	if (bDebug > 1) {
		sprintf(debugstr, "UseGetExtent %d %d", first, last);
		OutputDebugString(debugstr);		/* debugging */
	}
#endif

	for (k = first; k <= last; k++) {		/* first = 0 last = 255 */
		*chars = (char) k;					/* 95/Nov/5 */
		TextSize.cx=0; TextSize.cy=0;			/* 97/July/13 */

#ifdef USEUNICODE
/* Will trigger only for text fonts in NT or for TT in 95 with UseNewEncode */
/*		if (bUseNewEncode && bFontEncoded) */
		if (bFontEncoded) {
			WCHAR charsW[1];
			charsW[0] = encodeUID[k];
			ret = GetTextExtentPoint32W(hDC, charsW, 1, &TextSize);
		}
		else {
			ret = GetTextExtentPoint32A(hDC, chars, 1, &TextSize);
			if (bWinNT) {	/* ATM NT returns 4095 for undefined ??? */
				if (TextSize.cx == 4095) TextSize.cx = -1;
			}
		}
#else
		ret = GetTextExtentPoint32(hDC, chars, 1, &TextSize);
#endif

/*		ret = GetTextExtentPoint(hDC, chars, 1, &TextSize); */

/*		succeeds => TRUE, fails => FALSE */	/* never fails ? */
/*		Get extended error information, call DWORD GetLastError() in WIN32 */
		if (!ret) {
/*			if (bDebug > 1) OutputDebugString("Bad Char"); */

			err = GetLastError();

			sprintf(debugstr,
					"GetWidth for char %d (err %x):\tSize (x %d y %d)\n",
					k, err, TextSize.cx, TextSize.cy);

			if (ExplainError(debugstr, err, 1)) return 0;

/*			if (wincancel(debugstr)) return 0; */

/*			The above can happen if the font has a bad character 97/July/13 */
/*			or if PFM is inconsistent with PFB 97/Dec/5 */
		}
/*		widths[k] = TextSize.cx; */
		widths[k] = (short) TextSize.cx;
/*		widths[k] = (short) (TextSize.cx * 103 / 100); */ /* MOBY HACK!!! */
	}	/* end of for loop */
#ifdef DEBUGWIDTH
	if (bDebug > 1) OutputDebugString("Leaving GetWidth");
#endif
	return 1;
/*	return ((extentflag > 0) | (getcharflag > 0)); */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*
void setupremaptable(void) {	
	int k;
 	for (k = 0; k < 10; k++) remaptable[k] = (char) (k + 161);
	for (k = 10; k < 33; k++) remaptable[k] = (char) (k + 163);
	for (k = 33; k < 127; k++) remaptable[k] = (char) k;
	remaptable[127] = 196;
	for (k = 128; k < 256; k++) remaptable[k] = (char) k;
} */

int getmaxdrift(HDC hDC) {
	POINT steparr[1];
	if (bCopyFlag != 0) return 0;			/* 92/Feb/18 */
	(void) SetMapMode(hDC, MM_TWIPS);		/* set unit to twips */
	steparr[0].x = 1;
	steparr[0].y = 0;
	(void) DPtoLP(hDC, steparr, 1);
#ifdef IGNORED
	sprintf(debugstr, "maxdrift %d", steparr[0].x); 
	OuputDebugString(debugstr);					/* debugging */
#endif
	return(steparr[0].x);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following all used to be static - but now called by winspeci ... */

/* assumes only one file open at a time */

/* versions of getc and ungetc using low-level C input/output */

/* static */ 
void an_unwingetc(int c, HFILE input) { /* ignores `input' */
	if (ungotten >= 0) {
		wincancel("Repeated unwingetc"); errcount();
	}
	else filepos--;
	ungotten = c;
}

/* static */ 
int an_replenishbuf(HFILE input, int bufferlen) {
/*	buflen = read(input, buffer, (unsigned int) bufferlen); */
	buflen = _lread(input, buffer, (unsigned int) bufferlen);
	if (buflen < 0) {
/*		winerror("Read error in an_wingetc"); */
		strcpy (debugstr, "Read error in wingetc ");
		if (errno == EBADF) strcat(debugstr, "invalid file handle");
		wincancel(debugstr);
		finish = -1;
	}
	if (buflen <= 0) return EOF;	/* end of file or read error */
	bufptr = buffer;
	return buflen;
}

/* static */ 
int an_wingetc(HFILE input) {
	int c;
	if (ungotten >= 0)  {
		c = ungotten; ungotten = -1; filepos++; return c;
	}
	else if (buflen-- > 0) {
		filepos++;
		return (unsigned char) *bufptr++;
	}
	else {
		if (an_replenishbuf(input, BUFFERLEN) < 0) return EOF;
		buflen--;
		filepos++; 
		return (unsigned char) *bufptr++;
	}
}

/* static */ 
long an_wintell(HFILE input) {		/* where are we in the file */
	return filepos;
}

/* possibly check for whether new position is somewhere in buffer ? */

/* static */ 
long an_winseek(HFILE input, long place) {
	long foo;
 
	if (filepos == place) return place;
	if (place < 0) {
		sprintf(str, "Negative seek %ld", place);
		wincancel(str);
		return 0;
	}
/*	foo = lseek(input, place, 0);	 */
	foo = _llseek(input, place, SEEK_SET);

	if (foo != place) {
/*		sprintf(str, "Seek error: to %ld - at %ld", place, foo); */
		sprintf(str, "Seek error: to %ld ", place);
		if (errno == EBADF)  strcat(str, "invalid file handle");
		if (errno == EINVAL) strcat(str, "invalid origin or position");
		wincancel(str);
	}
	filepos = place;
	ungotten = -1;
	buflen = 0;
	bufptr = buffer;	/* redundant ? */
	return foo;
}

/* static */ 
int an_wininit(HFILE input) {	/* ignores `input' */
	filepos = 0;			/* beginning of file */
	ungotten = -1;
	buflen = 0;				/* nothing buffered yet */
	bufptr = buffer;		/* redundant ? */
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for reading signed and unsigned numbers of various lengths */

static unsigned int ureadone (HFILE input) {	
	return (unsigned int) an_wingetc(input);
}

static unsigned int ureadtwo (HFILE input) {
	return (an_wingetc(input) << 8) | an_wingetc(input);
}

static unsigned long ureadthree (HFILE input) {
	int c, d, e;
	c = an_wingetc(input);	d = an_wingetc(input);	e = an_wingetc(input);
	return ((((unsigned long) c << 8) | d) << 8) | e;
}

/* static unsigned long ureadfour (HFILE input) {
	int c, d, e, f;
	c = an_wingetc(input);	d = an_wingetc(input);
	e = an_wingetc(input);	f = an_wingetc(input);
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
} */

static int sreadone (HFILE input) {
	int c;
	c = an_wingetc(input);
	if (c > 127) return (c - 256);
	else return c;
}

#ifdef IGNORED
#pragma optimize ("lge", off) 
static int sreadtwo (HFILE input) {
	short int result;
	result = ((short int) an_wingetc(input) << 8) | (short int) an_wingetc(input);
	return result;
}
#pragma optimize ("lge", on) 
#endif

/* compiler optimization bug worked around 98/Feb/8 */

static int sreadtwo (HFILE input) {
	int c, d;
	c = an_wingetc(input);	d = an_wingetc(input);
	if (c > 127) c = c - 256;
/*	return c << 8 | d; */
	return (c << 8) | d;		/* 99/Jan/12 */
}

static long sreadthree (HFILE input) {
	int c, d, e;
	c = an_wingetc(input);	d = an_wingetc(input);	e = an_wingetc(input);
	if (c > 127) c = c - 256; 
	return ((((long) c << 8) | d) << 8) | e;
}

static long sreadfour (HFILE input) {
	int c, d, e, f;
	c = an_wingetc(input);	d = an_wingetc(input);
	e = an_wingetc(input);	f = an_wingetc(input);
	return ((((((long) c << 8) | (long) d) << 8) | e) << 8) | f;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* x and y offsets controlled by scroll bars ??? */

/* scaling controlled by magnify and demagnify ??? */

/* we changed scale to larger value 97/Sep/17 */

/* watch out for assumptions about default dvi_num and dvi_den ??? */

long magnum = 8000;		/* if MM_TWIPS is use in SetMapMode */
/* long magden = 26329; */	/* DVI uses `scaled' points */
long magden = 25696;	/* DVI uses `scaled' points - not always */

/* long magnum = 100; */		/* if MM_TWIPS is use in SetMapMode */
/* long magden = 329; */		/* DVI uses `scaled' points - not always */

/* Mapping is from DVI units (`scaled' points) => twips */

/* The correct scale is (num/den) * (72/2540000) * (mag/1000) * 20 ! */
/* Typical values are num = 25400000, den = 473628672, mag = 1000 or 1200 */

void setscale (int wantedzoom) { 
	int k=wantedzoom;
	if (dvi_num == 0 || dvi_den == 0) {	/* shouldn't happen! */
		wincancel("No DVI file open"); 
		return;
	}
/* SCALE IS: (dvi_num/dvi_den) * (dvi_mag/1000) * (72/254000) * 20 */
/* TeX:		dvi_num = 25,400,000 and dvi_den = 473,628,672 */
/* TeXInfo:	dvi_num =    254,000 and dvi_den =      57,816 */

	if (dvi_den > 1000000) {	/* normal TeX case */
/*		Change 97 Sep 17 to make magnum and magden larger - by * 80 */
/*		magnum = (dvi_num / 254000); */
		magnum = dvi_num / 3175; 
/*		magden = (dvi_den / (20 * 72 * 1024); */
		magden = dvi_den / 18432;
/*		if (dvi_mag != 1000) magnum = (magnum * dvi_mag + 500 ) / 1000; */
		if (dvi_mag != 1000) magnum = MulDiv(magnum, dvi_mag, 1000);
	}
	else {						/* TeXInfo use */
		magnum = MulDiv(dvi_num, 1000, 3175);
		magden = MulDiv(dvi_den, 1000, 18432);
		if (dvi_mag != 1000) magnum = MulDiv(magnum, dvi_mag, 1000);
	}

/*	magnum = (long) (dvi_num / 254000);  */
/*	magden = (long) (dvi_den / (20 * 72)) / 1024; */
/*	magnum = (magnum * (long) dvi_mag) / 1000;  */

	if (bDebug > 1) {
		OutputDebugString("");					// blank line separator
		sprintf(debugstr, "dvi_num %ld dvi_den %ld magnum %ld magden %ld",
				dvi_num, dvi_den, magnum, magden);
		OutputDebugString(debugstr);
	}
/*	1993/March/22 */
	if (! bPrintFlag && ! bCopyFlag) {
/*		if (magbase != 1000) magnum = magnum * (long) magbase / 1000; */
		if (magbase != 1000) magnum = MulDiv(magnum, (long) magbase, 1000);
	}
	if (k > 0) {
/*		while (k-- > 0) magden = (magden * 5) / 6; 20% increase */
		while (k-- > 0) {
			if (bSmallStep == 0) {					/* 94/Apr/19 */
				magden = (magden * 10 + 5) / 11; 
				magnum = (magnum * 11 + 5) / 10;
			}
			else {
				magden = (magden * 20 + 10) / 21; 
				magnum = (magnum * 21 + 10) / 20;
			}
		}
		if (magden == 0) magden = 1;
	} 
	else {
/*		while (k++ < 0) magnum = (magnum * 5) / 6; 20% decrease */
		while (k++ < 0) {
			if (bSmallStep == 0) {					/* 94/Apr/19 */
				magnum = (magnum * 10 + 5) / 11; 
				magden = (magden * 11 + 5) / 10;
			}
			else {
				magnum = (magnum * 20 + 10) / 21; 
				magden = (magden * 21 + 10) / 20;
			}
		}
		if (magnum == 0) magnum = 1;
	}
#ifdef DEBUGSCALE
	if (bDebug > 1) {
		sprintf(debugstr, "zoom %d magnum %ld magden %ld\n",
				wantedzoom, magnum, magden);
		OutputDebugString(debugstr);
	}
#endif
}

/* #define TWIPLIM 32000 */	/* 16383 try and prevent overflow problems */

/* map from dvi units to twips */	/* use MulDiv for these calculations ? */

int mapx (long x) { /* or use Windows to remap ? (but long => int) */
	long twipx;

	if (x >= 0) twipx = MulDiv(x, magnum, magden * 1024) + xoffset;
	else twipx = - MulDiv(-x, magnum, magden * 1024) + xoffset;

/*	if (x >= 0) twipx = (((x / magden) * magnum + 500) >> 10) + xoffset; */
/*	else twipx = - ((( -x / magden) * magnum + 500) >> 10) + xoffset; */

	if (twipx > TWIPLIM) twipx = TWIPLIM;
	else if (twipx < -TWIPLIM) twipx = -TWIPLIM;
	return (int) twipx;
}

long lmapx (long x) { /* or use Windows to remap ? (but long => int) */
	long twipx;

	if (x >= 0) twipx = MulDiv(x, magnum, magden * 1024) + xoffset;
	else twipx = - MulDiv(-x, magnum, magden * 1024) + xoffset;

/*	if (x >= 0) twipx = (((x / magden) * magnum + 500) >> 10) + xoffset; */
/*	else twipx = - ((( -x / magden) * magnum + 500) >> 10) + xoffset; */

	return twipx;
}

/* map from dvi units to twips */

int mapy (long y) { /* or use Windows to remap ? (but long => int) */
	long twipy;

	if (y >= 0) twipy = - MulDiv(y, magnum, magden * 1024) + yoffset;
	else twipy = MulDiv(-y, magnum, magden * 1024) + yoffset;

/*	if (y >= 0) twipy = - (((y / magden) * magnum + 500) >> 10) + yoffset; */
/*	else twipy =  (((- y / magden) * magnum + 500) >> 10) + yoffset; */

	if (twipy > TWIPLIM) twipy = TWIPLIM;
	else if (twipy < -TWIPLIM) twipy = -TWIPLIM;
	return (int) twipy;
}

long lmapy (long y) { /* or use Windows to remap ? (but long => int) */
	long twipy;

	if (y >= 0) twipy = - MulDiv(y, magnum, magden * 1024) + yoffset;
	else twipy = MulDiv(-y, magnum, magden * 1024) + yoffset;

/*	if (y >= 0) twipy = - (((y / magden) * magnum + 500) >> 10) + yoffset; */
/*	else twipy =  (((- y / magden) * magnum + 500) >> 10) + yoffset; */

	return twipy;
}

/* map from dvi units to twips - no offsets */ /* watch for overflow */

int mapd (long d) { /* or use Windows to remap ? (but long => int) */
	long twipd;

	if (d >= 0) twipd = MulDiv(d, magnum, magden * 1024);
	else twipd = - MulDiv(-d, magnum, magden * 1024);

/*	if (d >= 0) twipd = (((d / magden) * magnum + 500) >> 10); */
/*	else twipd = - (((- d / magden) * magnum + 500) >> 10); */

	if (twipd > TWIPLIM) twipd = TWIPLIM;
	else if (twipd < -TWIPLIM) twipd = -TWIPLIM;
	return (int) twipd;
}

long lmapd (long d) {
	long twipd;

	if (d >= 0) twipd = MulDiv(d, magnum, magden * 1024);
	else twipd = - MulDiv(-d, magnum, magden * 1024);

/*	if (d >= 0) twipd = (((d / magden) * magnum + 500) >> 10); */
/*	else twipd = - (((- d / magden) * magnum + 500) >> 10); */

	return twipd;
}

/* map twips to dvi units */

long unmap (int d) {		/* approximate, of course */
	long ldvi;

	if (d >= 0) ldvi = MulDiv((long) d, magden *1024, magnum); 
	else ldvi =  -MulDiv((long) -d, magden * 1024, magnum); 

/*	if (d >= 0) ldvi = ((((long) d) << 10) / magnum * magden); */
/*	else ldvi = -((((long) -d) << 10) / magnum * magden); */

	return ldvi;
}

/*  Used in winprint.c DoCopy to get clipping rectangle in DVI coordinates */
/*	and here in winanal.c in redomapping */

long lunmap (long d) {		/* approximate, of course */
	long ldvi;

#ifdef DEBUGWIDTH
	if (bDebug > 1) {
		sprintf(debugstr, "lunmap %ld magden %ld magnum %ld -> %ld\n",
/*			d, magden * 1024, magnum, MulDiv(d, magden * 1024, magnum)); */
			d, magden, magnum, MulDiv(d, magden, magnum)*1024); 
		OutputDebugString(debugstr);
	}
#endif
/*	if (d >= 0) ldvi = MulDiv(d, magden * 1024, magnum); */
	if (d >= 0) ldvi = MulDiv(d, magden, magnum) * 1024; 
/*	else ldvi = - MulDiv(-d, magden * 1024, magnum); */
	else ldvi = - MulDiv(-d, magden, magnum) * 1024; 

/*	if (d >= 0) ldvi = ((d << 10) / magnum * magden); */
/*	else ldvi = -(((-d) << 10) / magnum * magden); */

	return ldvi;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Given (scale factor * 1024), compute corresponding zoom increment */

int inczoom (long newscale, long oldscale) {
	int k = 0;
	while (oldscale < newscale) {
		if (bSmallStep == 0) {					/* 94/Apr/19 */
			oldscale = (oldscale * 11 + 5) / 10;
			newscale = (newscale * 10 + 5) / 11;
		}
		else {
			oldscale = (oldscale * 21 + 10) / 20;
			newscale = (newscale * 20 + 10) / 21;
		}
		k++;
	}
	return (k-1);
}

/* convert Rect from pixel to logical coordinates */

RECT DPtoLPRect (HDC hDC, RECT Rect) {
	POINT corners[2];
/*	if (bCopyFlag != 0) return Rect; */	/* shouldn't get here then */
	corners[0].x = Rect.left; corners[0].y = Rect.top;
	corners[1].x = Rect.right; corners[1].y = Rect.bottom;
	(void) DPtoLP(hDC, corners, 2);		/* shouldn't fail */
	Rect.left = corners[0].x; Rect.top =  corners[0].y;
	Rect.right = corners[1].x; Rect.bottom = corners[1].y;	
	return Rect;
}

/* Ideally MAXZOOM and MINZOOM should be multiples of 4 */
/* so that Shft-Magnify will increment up to limit exactly */
/* so that Shft-Unmagnify will then hit the *same* magnifications */

int redomapping (int newzoom, RECT ClipRect, RECT ZoomRect) {
	long xclip, yclip;		/* center of rectangle (NEW) in twips */
	long xzoom, yzoom;		/* center of rectangle (OLD) in twips */
	long xdvi, ydvi;		/* old point that maps to center */
	long xclipnew, yclipnew; /* where this maps to after change */
	long xerror, yerror;	/* initial error in mapping */	

/*	forced (long) on following 1993/Sep/28 */
/*	xclip = (ClipRect.left + ClipRect.right) / 2; */
	xclip = ((long) ClipRect.left + ClipRect.right) / 2;
/*	yclip = (ClipRect.bottom + ClipRect.top) / 2; */
	yclip = ((long) ClipRect.bottom + ClipRect.top) / 2;
/*	xzoom = (ZoomRect.left + ZoomRect.right) / 2; */
	xzoom = ((long) ZoomRect.left + ZoomRect.right) / 2;
/*	yzoom = (ZoomRect.bottom + ZoomRect.top) / 2; */
	yzoom = ((long) ZoomRect.bottom + ZoomRect.top) / 2;
/*	find point that maps to center */
/*	xdvi = unmap((int) ((long) xclip - xoffset));	
	ydvi = - unmap((int) ((long) yclip - yoffset)); */
#ifdef IGNORED
/*	kludge to try and deal with offset shift problem */
	if (xoffset < 8192 || xoffset > -8192
		|| yoffset < 8192 || yoffset > -8192) {
		xdvi = unmap((int) ((xzoom - xoffset + 15) / 32)) * 32; 
		ydvi = -unmap((int) ((yzoom - yoffset + 15) / 32)) * 32; 
	}
	else { 
		xdvi = unmap((int) ((xzoom - xoffset + 511) / 1024)) * 1024;
		ydvi = -unmap((int) ((yzoom - yoffset + 511) / 1024)) * 1024;
	} 
#endif
	xdvi = lunmap(xzoom - xoffset);			/* 97/Sep/18 */
	ydvi = -lunmap(yzoom - yoffset);
	xerror = xzoom - mapx(xdvi);
	yerror = yzoom - mapy(ydvi);
/*	if (xerror != 0 || yerror != 0) */
		if (bDebug > 1) {
			sprintf(debugstr, "redomapping %ld %ld <=> %ld %ld\n",
				xzoom, yzoom, mapx(xdvi), mapy(ydvi));
			OutputDebugString(debugstr);
		}
/*	} */
/*	sprintf(str, "ex %ld ey %ld", xerror, yerror);
	wincancel(str); */
	if (xerror != 0 || yerror != 0) {
#ifdef IGNORED
		xdvi += unmap((int) xerror);
		ydvi -= unmap((int) yerror);
#endif
		xdvi += lunmap(xerror);
		ydvi -= lunmap(yerror);
/*		xerror = xzoom - mapx(xdvi);
		yerror = yzoom - mapy(ydvi); */
/*		sprintf(str, "ex %ld ey %ld", xerror, yerror);
		wincancel(str); */
	}
/*	newzoom = wantedzoom + flag; */
	if (newzoom - wantedzoom == 4 || newzoom - wantedzoom == -4) {
		if (newzoom > -3 && newzoom < 3) newzoom = 0;
	}
	if (newzoom > MAXZOOM) newzoom = MAXZOOM;
	else if (newzoom < MINZOOM) newzoom = MINZOOM;
	if (newzoom == wantedzoom) return 0;	/* 0 - no change */
	wantedzoom = newzoom;
	setscale(wantedzoom);

	xclipnew = lmapx(xdvi);
	yclipnew = lmapy(ydvi);

/*	if (xdvi >= 0) 
		xclipnew = (((xdvi / magden) * magnum + 500) >> 10) + xoffset; 
	else
		xclipnew =-(((-xdvi / magden) * magnum + 500) >> 10) + xoffset; 
	if (ydvi >= 0) 
		yclipnew =-(((ydvi / magden) * magnum + 500) >> 10) + yoffset;
	else
		yclipnew=(((-ydvi / magden) * magnum + 500) >> 10) + yoffset; */

	if (bDebug > 1) {
		sprintf(debugstr, "clip %ld %ld newclip %ld %ld\n",
				xclip, yclip, xclipnew, yclipnew);
		OutputDebugString(debugstr);
	}
	xoffset += (xclip - xclipnew);
	yoffset += (yclip - yclipnew);
	return -1;					/* -1 scaling and offset have changed */
}

/* both ClientRect and ZoomRect are in pixels */

int setnewzoom (HDC hDC, RECT ClientRect, RECT ZoomRect) {
	int dx, dy, wx, wy, newzoom;
	long mscalex, mscaley, mscale;

/*	first get new wantedzoom */
/*	sprintf(str, "client %d %d %d %d",
		ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom);
	wincancel(str);
	sprintf(str, "zoom %d %d %d %d",
		ZoomRect.left, ZoomRect.top, ZoomRect.right, ZoomRect.bottom);
	wincancel(str); */
	dx = ClientRect.right - ClientRect.left;
	dy = ClientRect.bottom - ClientRect.top;
	wx = ZoomRect.right - ZoomRect.left;
	wy = ZoomRect.bottom - ZoomRect.top;
	mscalex = (((long) dx) * 1024) / wx;
	mscaley = (((long) dy) * 1024) / wy;
	if (mscalex > mscaley) mscale = mscaley;
	else mscale = mscalex;
	newzoom = wantedzoom + inczoom(mscale, 1024L); 
	if (newzoom > MAXZOOM) newzoom = MAXZOOM;
	else if (newzoom < MINZOOM) newzoom = MINZOOM;
	if (newzoom  == wantedzoom) return 0;		/* no change */
/*	sprintf(str, "mscalex %ld mscaley %ld inczoom %d", 
		mscalex, mscaley, inczoom(mscale, 1024));
	wincancel(str); */
/*  then compute translation */
	ClientRect = DPtoLPRect(hDC, ClientRect); 
	ZoomRect = DPtoLPRect(hDC, ZoomRect); 
	return redomapping(newzoom, ClientRect, ZoomRect);
/*	return -1; */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* void showscaling(HWND hWnd) {
	HDC hDC;
	hDC = GetDC(hWnd);
	yposition = 10;
	sprintf(str, "xoffset: %ld;  yoffset: %ld;  zoom: %d ; ", 
		xoffset, yoffset, wantedzoom);
	(void) TextOut(hDC, xposition, yposition, str, (int) strlen(str));
	yposition += 20;
	sprintf(str, "magnum: %ld;  magden: %ld;  ", magnum, magden);
	(void) TextOut(hDC, xposition, yposition, str, (int) strlen(str));
	yposition += 20;
	sprintf(str, "dvi_num: %ld;  dvi_den: %ld;  dvi_mag: %ld; ",
		dvi_num, dvi_den, dvi_mag);
	(void) TextOut(hDC, xposition, yposition, str, (int) strlen(str));
	yposition += 20;
	(void) ReleaseDC(hwnd, hDC);
} */

void showmagnification (HWND hWnd) {
	double dscale;
	dscale = (double) magnum / magden;
	dscale = dscale * 25696.0 / 8000.0;
	sprintf(str, "Magnification\t%lg\nZoom Index\t%d\n(%ld / %ld) * (%ld / %ld)",
			dscale, wantedzoom, magnum, magden, 25696, 8000);
	MessageBox(hWnd, str, "DVIWindo", MB_ICONINFORMATION | MB_OK);	
}

void showscaling (HWND hWnd) {
	char *s=str;

	sprintf(s, "xoffset: %ld;  yoffset: %ld;  zoom: %d;\n",
		xoffset, yoffset, wantedzoom);
	s = s + strlen(s);
	sprintf(s, "mag_num: %ld;  mag_den: %ld;\n", magnum, magden);
	s = s + strlen(s);
/*	sprintf(s, "dvi_num: %ld;  dvi_den: %ld;\ndvi_mag: %lu.%03lu;\n", */
	sprintf(s, "dvi_num: %ld;  dvi_den: %ld;\ndvi_mag: %3f;\n",
		dvi_num, dvi_den, (double) dvi_mag / 1000.0);
	MessageBox(hWnd, str, "DVIWindo scaling", MB_ICONINFORMATION | MB_OK);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Windows 3.1 stuff */

#define OUT_TT_PRECIS 4
#define OUT_TT_ONLY_PRECIS 7
/* #define CLIP_TT_ALWAYS 0x20 */	/* defined in windows.h */

#define TMPF_TRUETYPE 0x04

/* Stuff for finding font given detailed description, including Face */
/* (both Type 1 and TrueType) */
/* TTFFlag != 0 implies want TTF font */
/* TTFflag < 0 implies want symbol/decorative font (is this still true?) */
/* bBoldflag < 0 => we don't know the weight */

HFONT createatmfont (char *name, int nHeight, int nWidth, 
			int bBoldFlag, int bItalicFlag, int TTFflag) {
	int FontWeight;
	BYTE FontOutPrec=0, FontClipPrec=0;
	BYTE FontItalic;
	BYTE FontCharSet;
	BYTE FontPitchAndFamily;
	LOGFONT NewFont;
	HFONT hFont;
/*	HFONT hOldFont=NULL; */
/*	int charset, family; */
/*	HDC hDC; */

#ifdef DEBUGFONTS
	if (bDebug > 1) {
		sprintf(debugstr, "%s H %d W %d B %d I %d TT %d\n",
				name, nHeight, nWidth, bBoldFlag, bItalicFlag, TTFflag);
		OutputDebugString(debugstr);
	}
#endif

/*	if (bTrueTypeOnly != 0 && TTFflag == 0) return NULL; */ /* 1993/Aug/17 */
	if (bTrueTypeOnly != 0) TTFflag = 1;					/* 1993/Aug/17 */
	if (bTypeOneOnly != 0) TTFflag = 0;						/* 1995/Jun/30 */

/*	FontCharSet = ANSI_CHARSET; */					/* 0 */
	FontCharSet = DEFAULT_CHARSET;					/* 1 */ /* CHECK W 3.0 */
	FontPitchAndFamily=VARIABLE_PITCH | FF_DONTCARE;	
	
/*	if (bDebug > 1) {	
		sprintf(debugstr, "%s bold %d italic %d ttf %d\n", 
			name, bBoldFlag, bItalicFlag, TTFflag);
		OutputDebugString(debugstr);
	} */			/* debug 1992/Dec/24 */

	if (TTFflag) {					/* try and force TTF */
		FontOutPrec = OUT_TT_ONLY_PRECIS;
		FontClipPrec = CLIP_TT_ALWAYS;
	}

/*	if (TTFflag) {
		FontOutPrec = OUT_TT_ONLY_PRECIS;
		FontClipPrec = CLIP_TT_ALWAYS;
	} */

/*	does FW_DONTCARE really work? does it matter */
	if (bBoldFlag > 0) FontWeight = FW_BOLD;
	else if (bBoldFlag < 0) FontWeight = FW_DONTCARE;	/* unknown */
	else FontWeight = FW_NORMAL;
	if (bItalicFlag != 0) FontItalic = TRUE;
	else FontItalic = FALSE;
/*	Use full name and no modifiers --- attempt to fix problem 1992/Sep/17 */
/*	if (TTFflag != 0) {
		FontItalic = FALSE;
		FontWeight = FW_DONTCARE;
	} */
/*	symbol/decorative TT font */	/* try omitting in future ? */
/*	if (TTFflag < 0) {	
		FontCharSet = SYMBOL_CHARSET; 
		FontPitchAndFamily=VARIABLE_PITCH | FF_DECORATIVE;
	} */

/*	following NOT needed if ScreenAdjust=Off set in [Settings] in ATM.INI */
/*	#define MAGICFACT 10324 */	/* 1.0324 scale factor on ATM font size */
/*	if (bCorrectFlag != 0) */
	if (bCorrectFlag	 != 0 && TTFflag == 0)		/* not used anymore */
		nHeight = (int) ((((long) nHeight) * MAGICFACT) / 10000);
/*	A Hack!!! REMOVE!!! DEBUGGING ONLY */
	if (nMagicFact != 10000 && nHeight != metricsize) {
/*		A Hack!!! REMOVE DEBUGGING ONLY */	/* 97/Sep/14 */
/*		nHeight = (int) ((((long) nHeight) * 10000) / 10324); */
/*		nHeight = (int) ((((long) nHeight) * 10000) / 10800); */
		nHeight = (int) ((((long) nHeight) * nMagicFact) / 10000);
	}
/*	nHeight = (int) ((((long) nHeight) * 500) / 1000); */
/*	negative height means => character height (H), not cell height (H+D) */
	if (bSignFlag != 0) nHeight = - nHeight; 			/* ALWAYS ! */

/*	New, switched to using CreateFontIndirect - for no good reason */
	NewFont.lfHeight = nHeight;				/* height		     */
	NewFont.lfWidth = nWidth;				/* width			 */
	NewFont.lfEscapement = 0;				/* escapement	     */
	NewFont.lfOrientation =	0;				/* orientation	     */
	NewFont.lfWeight = FontWeight;			/* weight		     */
	NewFont.lfItalic = FontItalic;			/* italic		     */
	NewFont.lfUnderline = FALSE;			/* underline		 */
	NewFont.lfStrikeOut = FALSE;			/* strikeout		 */
	NewFont.lfCharSet = FontCharSet;		/* charset           */
	NewFont.lfOutPrecision = FontOutPrec;	/* out precision     */
	NewFont.lfClipPrecision = FontClipPrec;	/* clip precision    */
	NewFont.lfQuality = 0;					/* quality           */
	NewFont.lfPitchAndFamily = FontPitchAndFamily; /* pitch & family */
	strncpy(NewFont.lfFaceName, name, LF_FACESIZE);	/* typeface  */
	hFont = CreateFontIndirect(&NewFont);

	return hFont;

/*				ANSI_CHARSET, */					/* charset		 */
/*				DEFAULT_CHARSET, */					/* charset		 */
/*				SYMBOL_CHARSET, */					/* charset		 */
/*				OUT_DEFAULT_PRECIS,	*/				/* out precision	*/
/*				CLIP_DEFAULT_PRECIS,	*/			/* clip precision  */
/*				PROOF_QUALITY,	*/					/* quality		 */
/*				DEFAULT_QUALITY, */					/* quality		 */
/*				VARIABLE_PITCH | FF_ROMAN, */		/* pitch and family */
/*				VARIABLE_PITCH | FF_DONTCARE, */	/* pitch and family */
/*				VARIABLE_PITCH | FF_DECORATIVE, */	/* pitch and family */

}

/* do CharSet and PitchAndFamily in the above matter ? */
/* are they ignored ? what about ANSI PFM files ? */ 

/* bold < 0 => we don't know the weight */

/* Try and open a font - check Face returned - return NULL if bad */
/* also try and check whether italic and bold are set as requested */
/* although ATM seems to simulate these if not present ... */

/* char FaceName[MAXFACENAME]; */	/* make accessible for error message ? */
/* char FaceName[64];	*/	/* LF_FULLFACESIZE just in case */
char FaceName[64+2];		/* LF_FULLFACESIZE just in case */

HFONT openfontsub (HDC hDC, char *name, 
		int nHeight, int nWidth, int bold, int italic, int ttf) {
	HFONT hFont, hFontSaved;
	WORD options;
	int ret, flag, n;
	char *s;
/*	char FaceName[MAXFACENAME]; */	/* 1993/March/31 */

	if (name == NULL) return NULL;		/* sanity check */
	*FaceName = '\0';

/*	if (hdc == NULL) */
	if (hDC == NULL) {
		if (wincancel("hDC is NULL (openfontsub)"))
			finish = -1;
		return NULL;
	}
	if (hdc != NULL) hDC = hdc;	/* experiment ??? 99/Jan/12 ??? */

#ifdef IGNORED
	if (bDebug > 1) {
		sprintf(debugstr, "Trying %s,%s%s%s\n", name, 
					(!bold && !italic) ? "REGULAR" : "",
					bold ? "BOLD" : "",
					italic ? "ITALIC" : "",
					ttf? " (TT)" : "");
		OutputDebugString(debugstr);
	}
#endif
/*	one place for hack */
	hFont = createatmfont(name, nHeight, nWidth, bold, italic, ttf);
	if (hFont == NULL) {		/* Very Unlikely ! */
		sprintf(str, "Can't create Font: %s", name);
		wincancel(str);			/* debugging */
		return NULL;
	}
	else {							/* do a quick sanity check */
/*		actually select into device context so we can do some sanity checks */
		if (!ttf && bATMPrefer && bATMLoaded) {		/* 95/Mar/17 */
/*		if (!ttf && bATMPrefer && bATMLoaded && bCopyFlag == 0) */ /* ? */
			if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
			else options = ATM_SELECT;
/*			ret = MyATMSelectObject (hdc, hFont, options, &hFontSaved); */
			ret = MyATMSelectObject (hDC, hFont, options, &hFontSaved);
/*			if (bDebug && ret != ATM_SELECTED &&
			    bComplainMissing && !bIgnoreSelect) */
/*			if (ret != ATM_SELECTED) */
/* If it is not ATM font, we get -202 error, but font still selected OK */
/*			if (bDebug > 1) {
				sprintf(debugstr, "ATM select %s %s%s returns %d\n",
						name, bold ? "BOLD" : "", italic ? "ITALIC" : "", ret);
				OutputDebugString(debugstr);
			} */ /* debugging code 96/May/18 */
/*			if (ret != ATM_SELECTED && ret != ATM_SELECTMISSING) */
			if (ret != ATM_SELECTED && ret != ATM_FOREIGNFONTSELECTED) {
			    if (bDebug && bComplainMissing && !bIgnoreSelect) { 
					sprintf(debugstr, "ATM Select Error %d (%s) %s,%s%s%s %s\n",
							 ret, "Font", name,
							 (bold == 0 && italic == 0) ? "REGULAR" : "",
							 (bold>0) ? "BOLD" : "",
							 italic>0 ? "ITALIC" : "",
							 ttf ? " (TT)" : "");
					if (bDebug > 1) OutputDebugString(debugstr);
					else winerror (debugstr);
/*				Most commonly the error is ATM_SELECTMISSING (-202) */
			    }
			}
		}
/*		else hFontSaved = SelectObject (hdc, hFont); */
		else hFontSaved = SelectObject (hDC, hFont);

/*		Check on Face Name to verify font */
/*		(void) GetTextFace(hdc, MAXFACENAME, FaceName);	 */
/*		n = GetTextFace(hdc, sizeof(FaceName), FaceName); */
		n = GetTextFace(hDC, sizeof(FaceName), FaceName);
/*		provide for mismatch in Windows Face Name ? 1994/Oct/29 */
/*		if (strcmp(name, FaceName) != 0) */	/* when font not `installed' */

		if (bIgnoreFontCase) flag = _stricmp(name, FaceName);
		else flag = strcmp(name, FaceName);

/*		e.g. Lucida New Math Alternate Italic */
		if (flag != 0) {	/* check on possible FaceName truncation */
/*			n = strlen(FaceName); */
			if (n >= 31) {
				if (bDebug > 1) {
					sprintf(debugstr, "%s (%d)\n", FaceName, strlen(FaceName));
					OutputDebugString (debugstr);
				}	/* debugging 97/Feb/10 */
				if (bIgnoreFontCase) flag = _strnicmp(name, FaceName, n);
				else flag = strncmp(name, FaceName, n);
			}
		}

		if (flag != 0) {	/* name does not match */
/*			if (bDebug && bDebugKernel) */
/*			if (bDebug > 1) */
/* we already know we don't have a face name if bold < 0, so don't bother */
			if (bDebug > 1 && bold >= 0) { 
				sprintf(debugstr, 
/*					"Face: %s <> Font: %s B: %d I: %d TTF: %d\n", 
						FaceName, name, bold, italic, ttf); */
					"Offer: `%s' Request: %s", FaceName, name);
				s = debugstr + strlen(debugstr);
				if (bold >= 0) {
					sprintf(s, ",%s%s%s",
						(!bold && !italic) ? "REGULAR" : "",
						bold ? "BOLD" : "",
						italic ? "ITALIC" : "");
				}
				if (ttf) strcat (debugstr, " (TT)");
				strcat(debugstr, "\n");				/* 1995/July/30 */
				OutputDebugString(debugstr);
			}
			if ((UINT) hFontSaved > 1)	/* avoid Win 3.0 MetaFile problem */
/*				(void) SelectObject(hdc, hFontSaved); */
				(void) SelectObject(hDC, hFontSaved);
			if ((UINT) hFont > 1) {
				(void) DeleteObject(hFont);		/* get rid of it again */
				hFont = (HFONT) -1;
			}
			return NULL;						/* happens if not found */
		}
/*	Get here if Face Name matches the desired Face Name */
/*	Now read metrics, and check if italic/bold match ok ??? */
		else {	
/*			if (nHeight != METRICSIZE) */		/* don't want to disturb ??? */
			if (nHeight != metricsize) {		/* don't want to disturb ??? */
/*				if (GetTextMetrics(hdc, &TextMetric) == 0) */
				if (GetTextMetrics(hDC, &TextMetric) == 0) {
					wincancel("GetTextMetrics failed"); 
					if ((UINT) hFontSaved > 1)	/* Windows 3.0 MetaFile */
/*						(void) SelectObject(hdc, hFontSaved); */
						(void) SelectObject(hDC, hFontSaved);
					if ((UINT) hFont > 1) {
						(void) DeleteObject(hFont);
						hFont = (HFONT) -1;
					}
					return NULL;
				}
/*		there must be a better way of checking for synthesized ! */
/*		following does nothing useful, since bold & italic synthesized */
/*		none of the following ever kicks in ! so flush it ! */
			}
			if ((UINT) hFontSaved > 1)	/* avoid Windows 3.0 MetaFile probl */
/*				(void) SelectObject(hdc, hFontSaved); */	/* switch back */
				(void) SelectObject(hDC, hFontSaved);	/* switch back */
			return hFont;
		}
	}
}

/* Standard substitutions 98/Dec/15 */
/* Windows Face Name => Windows Face Name (last string "TT" if sub is TTF) */

char *Substitutions[][3]={
	{"Arial", "Helvetica", ""},
	{"Times New Roman", "Times", ""},
	{"Courier New", "Courier", ""},
	{"Helvetica", "Arial", "TT"},
	{"Times", "Times New Roman", "TT"},
	{"Courier", "Courier New", "TT"},
	{"", "", ""}
};

/* Tries to find font using both given name and all upper case version */
/* if subfontname is non-null, it uses that, as is, assuming ATM was ok */

HFONT openfont (HDC hDC, int fn, int nHeight, int nWidth) {
/*	char name[MAXFACENAME]; */
	char name[MAXFULLNAME];			/* LF_FULLFACENAME just in case */
	HFONT hFont;
	char *s;
	int k, ttfflag;
//	int boldflag, italicflag;
	char *fname="UNKNOWN";
	char *sfname="UNKNOWN";

	if (fontname[fn] != NULL) fname = fontname[fn];
	if (subfontname[fn] != NULL) sfname = subfontname[fn];

	if (hdc != NULL) hDC = hdc;	/* experiment ??? 99/Jan/12 ??? */

/*	if (strcmp(subfontname[fn], "") != 0)  */
	if (subfontname[fn] != NULL) {
#ifdef DEBUGFONTS
		if (bDebug > 1) {
			sprintf(debugstr, "TFM: %s (%d) => Face: %s,%s%s%s%s\n",
/*					fontname[fn], fn, subfontname[fn], */
					fname, fn, sfname,
					(!fontbold[fn] && !fontitalic[fn]) ? "REGULAR" : "",
					fontbold[fn] ? "BOLD" : "",
					fontitalic[fn] ? "ITALIC" : "",
					fontttf[fn]? " (TT)" : "");
			OutputDebugString(debugstr);
//			wincancel(debugstr);	// debugging only
		}
#endif
		hFont = openfontsub(hDC, sfname, /* subfontname[fn], */
					nHeight, nWidth, fontbold[fn], fontitalic[fn], fontttf[fn]);
/*		following should be the normal situation if ATM.INI read succesfully */
		if (hFont != NULL)	return hFont;

//		we failed to open the font...
		if (bDebug > 1) {
			sprintf(debugstr,
					"name %s subname %s bold %d italic %d ttf %d", 
/*					fontname[fn], subfontname[fn], */
					fname, sfname,
					fontbold[fn], fontitalic[fn], fontttf[fn]);
			OutputDebugString(debugstr);
//			wincancel(debugstr);	// debugging only
		}
/*		failed, now try standard substitutions 98/Dec/15 */
		for (k = 0; k < 16; k++) {
			if (strcmp(Substitutions[k][0], "") == 0) break;
			if (strcmp(Substitutions[k][1], "") == 0) break;
/*			make comparison case insensitive ? */
/*			if (strcmp(subfontname[fn], Substitutions[k][0]) == 0) */
			if (strcmp(sfname, Substitutions[k][0]) == 0) {
				strcpy(name, Substitutions[k][1]);
				if (strcmp(Substitutions[k][2], "") == 0) ttfflag = 0;
				else ttfflag = 1;
				hFont = openfontsub(hDC, name, 
						nHeight, nWidth, fontbold[fn], fontitalic[fn], ttfflag);
				if (hFont != NULL) return hFont;
/*				break; */ /* look for more ? */
			}
		}

/*		try and deal with case of TT font name involving "Italic" 1992/Dec/24 */
		if (fontttf[fn] != 0) {
/*	this seems like a bunch of heuristic kludges of sorts ...97/Feb/13 */
/*	try and convert to FaceName if it has spaces - but why should it ??? */
/*			strcpy(name, subfontname[fn]); */
			strcpy(name, sfname);
			while ((s = strchr(name, ' ')) != NULL) {
				strcpy(s, s+1);
				hFont = openfontsub(hDC, name, 
						nHeight, nWidth, fontbold[fn], fontitalic[fn], fontttf[fn]);
				if (hFont != NULL) return hFont;				
			}
/*	try and unmangle FontName (replace ' ' with '-') */
/*			strcpy(name, subfontname[fn]); */
			strcpy(name, sfname);
			if ((s = strchr(name, ' ')) != NULL) {
				*s = '-';	
				hFont = openfontsub(hDC, name, 
						nHeight, nWidth, fontbold[fn], fontitalic[fn], fontttf[fn]);
				if (hFont != NULL) return hFont;				
			}
/*	but why on earth should FullNames (with spaces) work here ??? */
/*			strcpy(name, subfontname[fn]); */
			strcpy(name, sfname);
			if (!fontbold[fn] && !fontitalic[fn]) strcat(name, " Regular");
			if (fontbold[fn]) strcat(name, " Bold");
			if (fontitalic[fn]) strcat(name, " Italic");
			hFont = openfontsub(hDC, name, nHeight, nWidth, 0, 0, fontttf[fn]);
			if (hFont != NULL) return hFont;
/* OK try this heuristic one more time for Demibold*/
			if (fontbold[fn]) {
/*				strcpy(name, subfontname[fn]); */
				strcpy(name, sfname);
				if (fontbold[fn]) strcat(name, " Demibold");
				if (fontitalic[fn]) strcat(name, " Italic");
				hFont = openfontsub(hDC, name, nHeight, nWidth, 0, 0, fontttf[fn]);
				if (hFont != NULL) return hFont;
			}
/* OK try this heuristic one more time for Oblique */
			if (fontitalic[fn]) {
/*				strcpy(name, subfontname[fn]); */
				strcpy(name, sfname);
				if (fontbold[fn]) strcat(name, " Bold");
				if (fontitalic[fn]) strcat(name, " Oblique");
				hFont = openfontsub(hDC, name, nHeight, nWidth, 0, 0, fontttf[fn]);
				if (hFont != NULL) return hFont;
			}
/* OK try this heuristic one more time for Demibold Oblique */
			if (fontbold[fn] && fontitalic[fn]) {
/*				strcpy(name, subfontname[fn]); */
				strcpy(name, sfname);
				if (fontbold[fn]) strcat(name, " Demibold");
				if (fontitalic[fn]) strcat(name, " Oblique");
				hFont = openfontsub(hDC, name, nHeight, nWidth, 0, 0, fontttf[fn]);
				if (hFont != NULL) return hFont;
			}
/* do substitutions only down here ? */
		} /* end of if fontttf[fn] != 0 */
		else return NULL;	/* what ? in ATM.INI and yet not found ? */
	} /* end of if subfontname[fn] != NULL */

/*	shouldn't ever get here if read ATM.INI earlier and font exists */
	if (fontname[fn] == NULL) return NULL;		/* sanity check */

#ifdef IGNORED
//	New code to deal with Windows lossage of standard fonts 2000 May 22
//	Should not be needed if subfontname[fn] was filled in earlier !!!
	for (k = 0; k < 16; k++) {
		if (strcmp(Base13[k][0], "") == 0) break;
		if (strcmp(Base13[k][1], "") != 0) strcpy(name, Base13[k][1]);
		if (_stricmp(Base13[k][0], fname) == 0) {
			if (strchr(Base13[k][2], 'B') != NULL) boldflag = 1;
			else boldflag = 0;
			if (strchr(Base13[k][2], 'I') != NULL) italicflag = 1;
			else italicflag = 0;
//			sprintf(debugstr, "%s => %s,%s%s",
//					fname, name, boldflag ? "BOLD" : "", italicflag ? "ITALIC" : "");
//			wincancel(debugstr);			//	debugging only
			hFont = openfontsub(hDC, name, nHeight, nWidth,
								boldflag, italicflag, 0);
			if (hFont != NULL) return hFont;
			break;
		}
	}
#endif
	
/*	strcpy(name, fontname[fn]); */
	strcpy(name, fname);
/*	if (istexfont(name) != 0) */		/* try upper case first if TeX font */
	if (bIgnoreFontCase == 0 && istexfont(name)) { /* 1994/Oct/29 */
		makeuppercase(name);		/*	AnsiUpper(name); ? */
		hFont = openfontsub(hDC, name, nHeight, nWidth, -1, 0, 0);
		if (hFont != NULL) return hFont;
/*		strcpy(name, fontname[fn]); */
		strcpy(name, fname);
		hFont = openfontsub(hDC, name, nHeight, nWidth, -1, 0, 0);
/*		if (hFont != NULL) return hFont;
		else return NULL; */
	}
	else {						/* try name as given first if not TeX font */
		hFont = openfontsub(hDC, name, nHeight, nWidth, -1, 0, 0);
		if (hFont != NULL) return hFont;
		makeuppercase(name);	/*	AnsiUpper(name); ? */
		hFont = openfontsub(hDC, name, nHeight, nWidth, -1, 0, 0);
		if (hFont != NULL) return hFont;
		else return NULL;  
	}
	return hFont;
} /* end of openfont */

void adjustatsize(int fn) {
	atsize = (long) fs[fn];
	wordspace = atsize / 5;			/* 0.2 quad */
	backspace = (atsize * 9) / 10;	/* 0.9 quad */
	verspace = (atsize * 4) / 5;	/* 0.8 quad */
	descent = fontdescent[fn];
	ascent = fontascent[fn];		/* not accessed ??? */
}

/* Look at character width table to guess whether font is remapped */
/* That is, whether 0 - 31 appears again higher up in 161 - 170 173 - 195 */

/* More reliable than looking at font file name ... */
/* Only do this test if first = 0 and last = 255 */
/* Assumes TextMetric structure set up */

/* Remaining problem: constant width fonts covering 0 - 255 */
/* Can't tell whether fixed width fonts remapped or not ??? */
/* New trick for fixed width fonts in AFMtoPFM: ... */
/* ... MaxCharWidth = AveCharWidth + 1 if remapped ! */

/* Also a problem: some old BSR CM font sets have FirstChar=32 */
/* Probably because of the Micrografx Designer bug work-around */

/* Tests on CharSet and Family are not very strict */
/* could insist that CharSet == SYMBOL_CHARSET (or DEFAULT_CHARSET) */
/* could insist that Family == FF_DECORATIVE (or FF_DONTCARE) */

/* BOOL isremapped(LPINT widths, TEXTMETRIC TextMetric, int flag) {*/
BOOL isremapped(LPSHORT widths, TEXTMETRIC TextMetric, int flag) {
	int k;

/*	Following has been rearranged  1993/July/6  so now ... */
/*	... more strict testing can now be turned off by Remap=3 */
	if (bATMBugFlag != 3) {		/* fairly strict test */
/*		Font can't be remapped if ANSI encoded - i.e. must be `decorative' */
/*		This one is the critical one for PSCRIPT.DRV, I think */
		if ((TextMetric.tmPitchAndFamily & 0xF0) != FF_DECORATIVE) 
			return FALSE;
/*		Font can't be remapped unless covers full range from 0 - 255 hmm... */
/*		At least that is what AFMtoPFM enforces ... */
		if (TextMetric.tmFirstChar != 0 || TextMetric.tmLastChar < 255) 
			return FALSE;
	}
	else {		/* much less stringent test if Remap=3 */
/*		Font can't be remapped unless covers at least 161 - 170 and 173 - 196 */
		if (TextMetric.tmFirstChar != 0 || TextMetric.tmLastChar != 196) 
			return FALSE;			/* problem for old CM fonts ? */
	}
/*	IBM OEM CharSet can't be remapped */ /*  Less critical, but makes sense */
/*  ... and needed to avoid bugs with some fixed width fonts (DOS code page) */
/*	if ((TextMetric.tmCharSet & 0xF0) == OEM_CHARSET) */
	if (TextMetric.tmCharSet == OEM_CHARSET)		/* 1993/Dec/18 */
		return FALSE;								/* 1993/Feb/14 ??? */
/*	Test below doesn't work with fixed widths fonts - so use old test */
/*  This path will NOT be triggered by new PFM's for fixed width fonts: */
/*  using the trick: ... MaxCharWidth = AveCharWidth + 1 if remapped ! */
	if (TextMetric.tmAveCharWidth == TextMetric.tmMaxCharWidth) 
		return flag;		
/*	above we simply return the flag passed in ... (from istexfont(...) */
/*	sprintf(debugstr, "Ave %d Max %d First %d Last %d\n", 
		TextMetric.tmAveCharWidth, TextMetric.tmMaxCharWidth,
			TextMetric.tmFirstChar,	TextMetric.tmLastChar); 
	if (bDebug > 1) OutputDebugString(debugstr); */
/*	Character widths need to match for remapping to be conjectured */
	for (k = 0; k < 10; k++) if (widths[k] != widths[k+161]) return FALSE;
/*	for (k = 10; k < 31; k++) if (widths[k] != widths[k+163]) return FALSE; */
	for (k = 10; k < 32; k++) if (widths[k] != widths[k+163]) return FALSE;
/*	extra tests considered too risky: */
/*	if (widths[32] != widths[195]) return FALSE; */			/* ??? */
/*	if (widths[32] != widths[128]) return FALSE; */			/* ??? */
/*	if (widths[127] != widths[196]) return FALSE; */		/* ??? */	
	return TRUE;	/* Seems to be remapped - passed all the tests */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Zapf Chancery Medium:	CharSet == ANSI		Family == Roman */
/* Zapf Dingbats:			CharSet == ANSI		Family == Decorative */
/* Symbol:					CharSet == Symbol	Family == Decorative */

/* For T1 fonts, critical is that Family is *not* decorative */

/* BOOL isansi (TEXTMETRIC TextMetric) {
	if ((TextMetric.tmCharSet == ANSI_CHARSET ||
		 TextMetric.tmCharSet == DEFAULT_CHARSET) &&
			(TextMetric.tmPitchAndFamily & 0xF0) != FF_DECORATIVE) return 1;
	else return 0;
} */

/* check whether this is a text font - that is, it should be reencoded */
/* non-zero return means reencode this text font */

/* BOOL isansi (TEXTMETRIC TextMetric) */
BOOL isansi (TEXTMETRIC TextMetric, char *FaceName, int ttfflag) {
	if (FaceName == NULL) return 0;			/* sanity check */
#ifdef IGNORED
	if (bDebug > 1) {
		sprintf(debugstr, "Face: %s  CharSet: %0X  Family: %0X  TeXFont: %d\n",
				FaceName,
				TextMetric.tmCharSet,
				TextMetric.tmPitchAndFamily, istexfont(FaceName));
		OutputDebugString(debugstr);
	}
#endif
/*  we apply this test to both T1 and TT fonts ? */ /* bDecorative ? */
	if ((TextMetric.tmPitchAndFamily & 0xF0) == FF_DECORATIVE) return 0;
/*	Maybe do this only in WINNT for auto converted TT fonts ? 97/Feb/16 */
	if ((TextMetric.tmPitchAndFamily & 0xF0) == FF_DONTCARE) return 0;
	if (TextMetric.tmCharSet == SYMBOL_CHARSET) return 0;
	if (TextMetric.tmCharSet == OEM_CHARSET) return 0;
	if (TextMetric.tmCharSet == DEFAULT_CHARSET) return 1;	/* text ? */
/*	if (TextMetric.tmCharSet == ANSI_CHARSET) return 1; */	/* text ? */
	if (TextMetric.tmCharSet != ANSI_CHARSET) return 0;
/*	use this Patch only if font is TrueType !!! */
	if (ttfflag && bPatchBadTTF && istexfont(FaceName)) return 0;
	return 1;
}

/* CharSet ANSI, DEFAULT, SYMBOL, SHIFTJIS, HANGEUL, CHINESEBIG5, OEM */
/* Pitch DEFAULT, FIXED, VARIABLE (low 4 bits) */
/* Family DONTCARE, ROMAN, SWISS, MODERN, SCRIPT, DECORATIVE (high 4 bits) */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long checkdefault = 0x59265920;	/* default signature */

/* Takes first six letters of encoding vector name and compresses */
/* into four bytes using base 40 coding */ /* 40^6 < 2^32 */
/* Treats lower case and upper case the same, ignores non alphanumerics */
/* Except handles -, &, and _ */

unsigned long codefourty(char *codingvector) {
	unsigned long result=0;
	int c, k;
	char *s=codingvector;

/*	printf("Given coding vector %s\n", codingvector); */
/*	if (strcmp(codingvector, "") == 0) */
	if (*codingvector == '\0') {
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
/*	printf("Computed CheckSum %08lx\n", result); */
	return result;
}

int decodefourty(unsigned long checksum, char *codingvector) {
	int c;
	int k;
/*	char codingvector[6+1]; */

/*	if (checksum == checkdefault) */
	if (checksum == 0) {
		strcpy(codingvector, "unknown");
		return 1;
	}
	else if ((checksum >> 8) == (checkdefault >> 8)) {	/* last byte random */
		strcpy (codingvector,  "native");		/* if not specified ... */
		return 1;								/* no info available */
	}
	else {
		for (k = 0; k < 6; k++) {
			c = (int) (checksum % 40);
			checksum = checksum / 40;
			if (c <= 'z' - 'a' ) c = c + 'a';
			else if (c < 36) c = (c + '0') - ('z' - 'a') - 1;
			else if (c == 36) c = '-';
			else if (c == 37) c = '&';
			else if (c == 38) c = '_';
			else c = '.';				/* unknown */
			codingvector[5-k] = (char) c;
		}
		codingvector[6] = '\0';
	}
/*	printf("Reconstructed vector %s\n", codingvector); */
	return 0;					/* encoding info returned in codingvector */
}

/*	Is encoding checked too early - before Windows font NULL ? */
/*	This checks whether encoding makes sense for font type */
/*	Issues warning if not */

int checksumencoding(int k) {
	char checksumvector[8];
	char *vector;
	
	if (fontwarned[k]) return 0;				/* already warned */
/*	if (fc[k] == nCheckSum) return 0; */		/* 1995/April/15 */
/*	This will have to change as we can reencode TT fonts ! 97/Jan/18 */
#ifdef USEUNICODE
	if (bUseNewEncodeTT && fontttf[k]) {		/* if TT reencoded */
		if (fc[k] == nCheckSum) return 0;		/* correct encoding for text */
	}
	else if (bUseNewEncodeT1 && !fontttf[k]) {	/* if T1 reencoded */
		if (fc[k] == nCheckSum) return 0;		/* correct encoding for text */
	}
	else if (fontttf[k])						/* for TrueType ANSI NEW */
		if (fc[k] == nCheckANSI) return 0;		/* correct encoding for text */
	else										/* for Type 1 ENCODING */
		if (fc[k] == nCheckSum) return 0;		/* correct encoding for text */
#else
	if (fontttf[k])								/* for TrueType ANSI NEW */
		if (fc[k] == nCheckANSI) return 0;		/* correct encoding for text */
	else										/* for Type 1 ENCODING */
		if (fc[k] == nCheckSum) return 0;		/* correct encoding for text */
#endif
/*	get here if we do not pass the test above */
#ifdef UNICODE
	if (fontttf[k]) {							/* 97/May/25 */
/*		if (bUseNewEncodeTT) */
		if (bUseNewEncodeTT && (szReencodingName != NULL))			
			vector = szReencodingName;
		else vector = "ansinew";
	}
#else
	if (fontttf[k]) vector="ansinew";			/* ? 1995/April/15 */
#endif
/*	else vector = szReencodingName; */
	else {
		if (szReencodingName != NULL) vector = szReencodingName;
		else vector = "ansinew";				/* should not happen */
	}
	
/*	if (strcmp(subfontname[k], "") == 0) vector = "unknown"; */ /* 95/Aug/2 */
	if (subfontname[k] == NULL) vector = "unknown";	/* 99/Jan/4 */
	if (decodefourty(fc[k], checksumvector) != 0) return 0;
/*	if (_strnicmp(checksumvector, szReencodingVector, 6) != 0) */
/*	if (_strnicmp(checksumvector, szReencodingName, 6) == 0) return 0; */
	if (_strnicmp(checksumvector, vector, 6) == 0) return 0;
	{
		char *fname="UNKNOWN";
		char *sfname="UNKNOWN";
		if (fontname[k] != NULL) fname = fontname[k];
		if (subfontname[k] != NULL) sfname = subfontname[k];
/*		mark as warned *first* to avoid reentrancy problems 95/May/5 */
		fontwarned[k] = 1;		 /* do this later to avoid repeat warning */
		sprintf(debugstr,
/*		"Encoding mismatch Font: %s, Face Name: %s\n`%s..' != `%s'", */
			"Encoding mismatch.\n Font: %s, Face Name: %s\n`%s..' versus `%s'", 
/*				fontname[k], subfontname[k], */
				fname, sfname,
/*				checksumvector, szReencodingVector); */
/*				checksumvector, szReencodingName); */
				checksumvector, vector);
/*	Don't complain about mismatch if no Face Name known for this font */
/*		if (strcmp(subfontname[k], "") == 0)  */
		if (subfontname[k] == NULL) { 	/* just send debug output maybe */
			if (bDebug > 1) OutputDebugString(debugstr);
		}
		else wincancel(debugstr);	/* otherwise tell user about it */
/*		fontwarned[k] = 1; */		/* do this later to avoid repeat warning */
	}
	return 1;								/* failed */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* separated out 95/June/11 */	/* slight rewrite 96/Mar/31 */

HFONT GetFakeFont (HDC hDC, int size) {
	HFONT hFont=NULL;
	int k;

	if (bUseFakeFont == 0) return hFont; 

	if (fakeindex >= 0) {		/* fake font already identifed ? */
/*		we found a suitable fake font on a previous visit ... */
/*		hFont=openfontsub(FakeFonts[fakeindex], METRICSIZE, 0, 0, 0, 0); */
/*		hFont=openfontsub(FakeFonts[fakeindex], size, 0, 0, 0, 0); */
		hFont=openfontsub(hDC, FakeFonts[fakeindex], size, 0, 0, 0, 0);
		return hFont;
	}
/*	if (fakeindex < 0) */		/* fake font not yet identifed ? */
	for (k = 0; k < 16; k++) {
		if (strcmp(FakeFonts[k], "") == 0) break;
/*		hFont = openfontsub(FakeFonts[k], METRICSIZE, 0, 0, 0, 0); */
		hFont = openfontsub(hDC, FakeFonts[k], size, 0, 0, 0, 0); 
		if (hFont != NULL) {
			fakeindex = k;		/* remember this one worked ! */
/*			if (bATMLoaded) fakettf = !MyATMFontSelected(hdc);
			else fakettf = 1; */ /* not selected into hdc yet ... */
			if (bDebug > 1) {
				sprintf(debugstr, "Using %s (%d) for fake font\n",
						FakeFonts[k], k);
				OutputDebugString(debugstr);
			}
			break;
		}
	}
/*	if (fakeindex < 0) {
		bUseFakeFont = 0;
		if (bDebug > 1)
			OutputDebugString("Can't find fake font!\n"); 
	} */ /* debugging */
	return hFont;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define lmemcpy memcpy 

/* LPSTR WINAPI lmemcpy(LPSTR , LPCSTR, int n);	*/

int copymetrics (int fnew, int fold) {

	if (hWidths == NULL) return -1;
	if (lpWidths == NULL) {
		if (bDebug > 1) {
			sprintf(debugstr, "Missing Memory for Widths %s", "copymetrics");
			OutputDebugString(debugstr);	/* impossible */
		}
		GrabWidths(); 							/* should not be needed */
		if (lpWidths == NULL) {
			finish = -1; 							/* ? */
			return -1; 
		}
	}								/* debugging only */
#ifdef IGNORED
	if (bDebug > 1) {
		sprintf(debugstr, "Copying info for font %d from font %d\n", fnew, fold);
		OutputDebugString(debugstr);
	}
#endif
	lmemcpy((LPSTR) (lpWidths + fnew * MAXCHRS),
			(LPSTR) (lpWidths + fold * MAXCHRS),
			sizeof(WORD) * MAXCHRS);
	ansifont[fnew] = ansifont[fold];
	texfont[fnew] = texfont[fold];
	metricsvalid[fnew] = metricsvalid[fold];
/*	fontbold[] and fontitalic[] and fontttf[] presumably are set up */
/*	is this supposed to set up anything else ? */
/*	how about fontdescent[] fontascent[] fontexists[] fontwarned[] */
	return 0;
}

/* Maybe use GetATMInstanceInfo instead of selecting font at METRICSIZE ? */

/* set up `metric font' (at size METRICSIZE = 1000) */

int setupmetrics(HDC hDC, int fn) {
	int flag;
/*	int k; */
	int texfontflag=0; 
	HFONT hMetricFont, hFontSaved;
	int ttf;
	WORD options;
	int ret;
	int fnbase;							/* 1996/July/28 */
/*	char FaceName[MAXFULLNAME]=""; */		/* LF_FULLFACESIZE 98/Sep/25 */

	if (hWidths == NULL) return 0;
	if (lpWidths == NULL) {						/* debugging */
		if (bDebug > 1) {
			sprintf(debugstr, "Missing Memory for Widths %s", "setupmetrics");
			OutputDebugString(debugstr);	/* impossible */
		}
		GrabWidths(); 							/* should not be needed */
		if (lpWidths == NULL) {
			finish = -1; 							/* ? */
			return 0; 
		}
	}
	if (fn < 0 || fn > MaxFonts-1) {					/* debugging */
		sprintf(debugstr, "Invalid font number %d > %d (%d)",
				fn, MaxFonts-1, dvi_f);	
		if (wincancel(debugstr)) {
			finish = -1;
			return 0;
		}
		fn = 0;
	}

#ifdef DEBUGHEAP
	CheckHeap("GetWidth", 0);
#endif

/*	Copy metrics from base font - potential cycle saver ? 1996/July/28 */
	if (bUseBaseFont) fnbase = basefont[fn];
	else fnbase = fn;
#ifdef IGNORED
	if (bDebug > 1) {
		sprintf(debugstr, "fn %d fnbase %d valid[fn] %d valid[fnbase] %d\n",
				fn, fnbase, metricsvalid[fn], metricsvalid[fnbase]);
		OutputDebugString(debugstr);
	}
#endif
	if (fnbase != fn) {				/* see if we already have metrics */
		if (metricsvalid[fnbase]) { 
			copymetrics(fn, fnbase);
			return metricsvalid[fn];
		}
	}

#ifdef DEBUGENCODING
/*		is it safe top print if fontname[k] or subfontname[k] is NULL ? */
	if (bDebug > 1) {					/* 1994/Dec/25 */
		char *fname="UNKNOWN";
		char *sfname="UNKNOWN";
		if (fontname[k] != NULL) fname = fontname[k];
		if (subfontname[k] != NULL) sfname = subfontname[k];
/*		Is this assuming standard dvi_num and dvi_den ??? */
		sprintf(debugstr, "Metric %d\t%s\t%s\t%.3f\t%s%s\n",
			fn, fname, sfname,
/*				(int) ((fs[fn] + 32000) / 65536), */
				(double) fs[fn] * ((double) num / den) * (72.27 / 254000.0),
					fontbold[fn] ? (LPSTR) "BOLD" : (LPSTR) "",
						fontitalic[fn] ? (LPSTR) "ITALIC" : (LPSTR) "");
		OutputDebugString(debugstr);
	}
#endif
	metricsvalid[fn] = 0;
	ttf = fontttf[fn];					/* 1994/Dec/22 ??? */
/*	adjustatsize(fn); */ /* not needed for metric font ??? */
/*	following is initial guess needed later if font is fixed width */
	if (fontname[fn] != NULL) 
		texfontflag = (char) istexfont(fontname[fn]);  /* not used */

/*	make up logical font at METRICSIZE size - to get PFM width table */
/*	hMetricFont = openfont(fn, METRICSIZE, 0); */
	hMetricFont = openfont(hDC, fn, metricsize, 0); 
	if (hMetricFont == NULL) {	/* shouldn't happen if normal font found */
		hMetricFont = GetFakeFont(hDC, metricsize);
	}
	if (hMetricFont == NULL) {	/* shouldn't happen if normal font found? */
		sprintf(str, "Metric font %s (%d) not found", fontname[fn], fn);
		wincancel(str);
	}
	else {	/* hMetricFont != NULL */
/*		how is mapping in hDC set up at this point ? MM_TEXT or MM_TWIPS ? */
		(void) SetMapMode(hdc, MM_TEXT);		/* EXPERIMENT 92/Apr/25 */
		if (bATMReencoded) UnencodeFont (hdc, fn, 1); /* shouldn't happen ! */
		if (!ttf && bATMPrefer && bATMLoaded) {		/* 95/Mar/17 */
/*		if (!ttf && bATMPrefer && bATMLoaded && bCopyFlag == 0) */ /* ? */
			if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
			else options = ATM_SELECT;
			ret = MyATMSelectObject (hdc, hMetricFont, options, &hFontSaved);
/*			if (bDebug && ret != ATM_SELECTED &&
			    bComplainMissing && !bIgnoreSelect) */
/*			if (ret != ATM_SELECTED) */
/* If it is not ATM font, we get -202 error, but font still selected OK */
/*			if (ret != ATM_SELECTED && ret != ATM_SELECTMISSING) */
			if (ret != ATM_SELECTED && ret != ATM_FOREIGNFONTSELECTED) {
			    if (bDebug && bComplainMissing && !bIgnoreSelect) { 
					char *fname="UNKNOWN";
					if (fontname[fn] != NULL) fname = fontname[fn];
					sprintf(debugstr, "ATM Select Error %d (%s) %s,%s%s%s %s\n",
							 ret, "Metric", fname, /* fontname[fn], */
							 (!fontbold[fn] && !fontitalic[fn]) ? "REGULAR" : "",
							 fontbold[fn] ? "BOLD" : "",
							 fontitalic[fn] ? "ITALIC" : "",
							 ttf ? " (TT)" : "");
					if (bDebug > 1) OutputDebugString(debugstr);
					else winerror (debugstr);
/*				Most commonly the error is ATM_SELECTMISSING (-202) */
				}
			}
		} /*	if (!ttf && bATMPrefer && bATMLoaded) */ /* 95/Mar/17 */
		else hFontSaved = SelectObject(hdc, hMetricFont);/* go metric font */

/*		now grab the metric information */ 
		if (GetTextMetrics(hdc, &TextMetric) == 0)
			winerror("Can't get TextMetrics");

/*		(void) GetTextFace(hDC, sizeof(FaceName), FaceName); */	/* 98/Sep/25 */
		if (fontname[fn] != NULL)
			ansifont[fn] = (char) isansi(TextMetric, fontname[fn], ttf); /* 98/Sep/25 */
#ifdef DEBUGFACES
		if (bDebug > 1) {
			sprintf(debugstr, "%d Face: %s fontttf: %d textfont: %d",
					fn, fontname[fn], fontttf[fn], ansifont[fn]); 
			OutputDebugString(debugstr);
		}
#endif
/*		bFontEncoded = isansi(TextMetric); */	/* fix 97/Jan/26 */
#ifdef DEBUGHEAP
		CheckHeap("GetWidth", 0);
#endif
#ifdef USEUNICODE
		if ((bUseNewEncodeTT && fontttf[fn]) ||
			(bUseNewEncodeT1 && !fontttf[fn])) {
			bFontEncoded = ansifont[fn];
			if (ansifont[fn] && bCheckEncoding) checksumencoding(fn);
		}
/*		else */	/* fix 97/Jan/26 */
/*		hEncoding will be NULL in Windows NT */
		if (bATMLoaded && hEncoding != NULL && ansifont[fn]) {
			ReencodeFont(hdc, fn, 1);				/* 94/Dec/25 */
/*			wininfo("Reencoding"); */
			if (bCheckEncoding) checksumencoding(fn);	/* 95/Jan/12 */
		}
#else
		if (bATMLoaded && hEncoding != NULL && ansifont[fn]) {
			ReencodeFont(hdc, fn, 1);				/* 94/Dec/25 */
/*			wininfo("Reencoding"); */
			if (bCheckEncoding) checksumencoding(fn);	/* 95/Jan/12 */
		}
#endif
#ifdef DEBUGHEAP
		CheckHeap("GetWidth", 0);
#endif
/*			lpCharTemp = lpWidths + fn * 256; */			/* MAXCHRS ? */
			lpCharTemp = lpWidths + fn * MAXCHRS;
/*			flag = GetCharWidth(hdc, 0, 255, lpCharTemp); */
			flag = GetWidth(hdc, 0, 255, lpCharTemp);
			if (flag == 0) {
				sprintf(str, "Can't get char widths for %s (%d)",
						fontname[fn], fn);
				wincancel(str);
/*				metricsvalid[fn]=0; */
			}
			else metricsvalid[fn]=1;
/*			this overrides earlier setting based on font name ? */
/*			but comes *after* metrics determined ? */
/*			we pass in texfontflag for fixed width fonts */
			texfont[fn] = (char) isremapped(lpCharTemp, TextMetric, texfontflag);
/*			ansifont[fn] = (char) isansi(TextMetric); */
/*		} */
		if (bATMReencoded) UnencodeFont (hdc, fn, 0);	/* moved here ... */
		(void) SetMapMode(hdc, MM_TWIPS);		/* EXPERIMENT 92/Apr/25 */
		if ((UINT) hFontSaved > 1)	/* avoid Windows 3.0 MetaFile problem */
			(void) SelectObject(hdc, hFontSaved);	/* deselect metric font */
		if ((UINT) hMetricFont > 1) {
			(void) DeleteObject(hMetricFont);		/* free up space again */
			hMetricFont = (HFONT) -1;
		}
	}		 /* end of hMetricFont is not NULL */
/*	if (bATMReencoded) UnencodeFont (hdc, fn, 0); */ /* reset encoding */
/*	(void) MoveToEx(hdc, devx, devy, NULL);	*/
/*	cp_valid = 0; */				/* ha ha */ /* why needed ? */
/*	fnbase = basefont[fn]; */
	if (fnbase != fn) {				/* see if we already have metrics */
		if (metricsvalid[fnbase] == 0) {
			copymetrics(fnbase, fn);
			return metricsvalid[fn];
		}
	}
	return metricsvalid[fn];		/* indicate success or failure */
}

/* set up font itself (as opposed to metrics version) */

int setupfont (HDC hDC, int fn) {				/* returns non-zero if success */
	int nHeight;
/*	int k; */
	HFONT hFontWindow;

#ifdef DEBUGENCODING
	if (bDebug > 1) {					/* 1994/Dec/25 */
		char *fname="UNKNOWN";
		char *sfname="UNKNOWN";
		if (fontname[k] != NULL) fname = fontname[k];
		if (subfontname[k] != NULL) sfname = subfontname[k];
/*		Is this assuming standard dvi_num and dvi_den ??? */
		sprintf(debugstr, "Font   %d\t%s\t%s\t%.3f\t%s%s\n",
/*			fn, (LPSTR) fontname[fn], (LPSTR) subfontname[fn], */
			fn, fname, sfname,
/*				(int) ((fs[fn] + 32000) / 65536), */
				 (double) fs[fn] * ((double) num / den) * (72.27 / 254000.0),
					fontbold[fn] ? (LPSTR) "BOLD" : (LPSTR) "",
						fontitalic[fn] ? (LPSTR) "ITALIC" : (LPSTR) "");
		OutputDebugString(debugstr);
	}
#endif
	if (fn < 0 || fn > MaxFonts-1) {
		sprintf(debugstr, "Invalid Font number %d > %d (%d)",
				fn, MaxFonts-1, dvi_f);
		if (wincancel(debugstr)) {
			return 0;
		}
		fn = 0;
	}
/*	fontexists[fn] = 0; */
	adjustatsize(fn);
	nHeight = mapd((long) atsize); 	/* calculate `at size' in twips */

/*	now make up logical font at correct size */
	hFontWindow = openfont(hDC, fn, nHeight, 0); 
/*  then check whether handle received from Windows is ok */
	if (hFontWindow == NULL) {
		if (bComplainMissing != 0  && fontwarned[fn] == 0) { /* ??? */
/*			mark as warned *first* to avoid reentrancy problems 95/May/5 */
			fontwarned[fn] = 1;
			sprintf(debugstr, "Can't find Font:\n%s (%d)\n",
					fontname[fn], fn);
/*			if (strcmp(subfontname[fn], "") == 0) */
			if (subfontname[fn] == NULL) {
				strcat (debugstr, "No Face Name\n");
			}
			else {
				strcat(debugstr, "Face Name: ");
				strcat(debugstr, subfontname[fn]);
/*	added style information 95/July/19 */
				if (fontbold[fn]) strcat(debugstr, " BOLD");
				if (fontitalic[fn]) strcat(debugstr, " ITALIC");
				if (fontbold[fn] == 0 && fontitalic[fn] == 0)
					strcat(debugstr, " REGULAR");
				strcat(debugstr, "\n");
			}
			strcat (debugstr, "(Windows offers: `");
			strcat (debugstr, FaceName);
			strcat (debugstr, "')\n");
			if (wincancel(debugstr)) {
/*	do something more drastic here ? to avoid many messages ? */
				finish = -1;
			}
/*			fontwarned[fn] = 1; */
		} /* end if (bComplainMissing != 0  && fontwarned[fn] == 0) */
/*		Need to turn on fontwarned even if bComplainMissing is off 95/May/5 */
		else fontwarned[fn] = 1;	

		if (bUseFakeFont != 0) {
			hFontWindow = GetFakeFont(hDC, nHeight);
			if (hFontWindow == NULL && fontwarned[fn] == 0) {
				wincancel("Can't even find font substitute!");
/*				fontwarned[fn] = 1; */ /* ??? */
			}
			else {
/*				strcpy(fontname[fn], fakefontname);	 */
/*				fontwarned[fn] = 1; */ /* ??? */
			}
		}	/* end of bUseFakeFont */
	} /* end of if (hFontWindow == NULL) */

	if (hFontWindow == NULL)  {
		fontvalid[fn] = 0; 
		fontexists[fn] = 0; 
/*		fontwarned[fn] = 1;		*//* to avoid complaining again later */
		windowfont[fn] = NULL;
	}
	else {				/* OK we got a font (real or fake) */
/*		not needed - left over in TextMetric data structure
		hFontSaved = SelectObject(hdc, hFontWindow);
		if (GetTextMetrics(hdc, &TextMetric) == 0) {
			wincancel("Can't get TextMetrics");
			(void) SelectObject(hdc, hFontSaved);
			return NULL;
		} */

		fontvalid[fn] = 1; 
		fontexists[fn] = 1; 
/*		fontwarned[fn] = 0; */
		fontdescent[fn] = TextMetric.tmDescent; /* in twips - of course */
		fontascent[fn] = TextMetric.tmAscent;	/* in twips - of course */
		windowfont[fn] = hFontWindow;
/*		ansifont[fn] = (char) isansi(TextMetric); */		/* 94/Dec/25 */
		if (fontname[fn] != NULL)				/* sanity check */
			ansifont[fn] = (char)
				isansi(TextMetric, fontname[fn], fontttf[fn]);	/* 98/Sep/25 */
		else ansifont[fn] = 0;
#ifdef DEBUGFACES
		if (bDebug > 1) {
			sprintf(debugstr, "%d Face: %s fontttf: %d textfont: %d",
					fn, fontname[fn], fontttf[fn], ansifont[fn]);
			OutputDebugString(debugstr);
		}
#endif
	}
/*  maybe also want to check whether the size is what was requested etc ? */
/*	fontexists[fn] = (char) fnt_exists; */
	return fontexists[fn];
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void do_push(void) {
	long *stackptr; 
	if (bSkipFlag == 0) {
		if (++stackinx >= MAXDVISTACK) { /* see whether stack getting full */
			winerror("Exceeded DVI stack depth"); 
			finish = -1; /* bSkipFlag = -1 ? */
			errcount();
			stackinx--;
		}
		else { /* speed up LATER ??? */
/* push (h, v, w, x, y, z) on stack */
/* read current point ? */
/* possibly need to retrieve dvi_h and dvi_v from `screen' ? */
/*			current = MoveToEx(hdc, dvi_hh, dvi_vv, NULL); */
/*			dev_hh = LOWORD(current); dev_vv = HIWORD(current); */
			stackptr =  stack + stackinx * STACKINC;
			*stackptr++ = dvi_h;
			*stackptr++ = dvi_v;	
			*stackptr++ = dvi_w;
			*stackptr++ = dvi_x;
			*stackptr++ = dvi_y;
			*stackptr++ = dvi_z; 
			*stackptr++ = (long) dvi_hh; /* NEW */
			*stackptr++ = (long) dvi_vv; /* NEW */
		}
/*		cp_valid = 0; */  /* NO, that hasn't changed */ 
/*		charbufinx = 0; */ /* ??? */
	}
}

void do_pop(void) {
/*	int c; */
	long *stackptr;
	if (bSkipFlag == 0)  { /* speed up LATER ??? */
/* pop (h, v, w, x, y, z) off stack */
		stackptr = stack + stackinx * STACKINC + (STACKINC - 1);
		dvi_vv = (int) *stackptr--;	/* NEW */
		dvi_hh = (int) *stackptr--;	/* NEW */
		dvi_z = *stackptr--;
		dvi_y = *stackptr--;
		dvi_x = *stackptr--;
		dvi_w = *stackptr--;
		dvi_v = *stackptr--;
		dvi_h = *stackptr--;
/*		(void) MoveToEx(hdc, dvi_hh, dvi_vv, NULL); */ /* TRIAL */ 
		cp_valid = 0;
		if(--stackinx < 0) { /* should never happen */
			winerror("DVI Stack underflowed"); 
			finish = -1; /* bSkipFlag = -1; */
			stackinx = 0;
		}
/*	possibly need to use dvi_h and dvi_v for `screen' ? */
/*		(void) MoveToEx(hdc, mapx(dvi_h), mapy(dvi_v), NULL); */  /* set current */
		cp_valid = 0;		/* indicate need to reset current point */
/*		charbufinx = 0; */
	}
}

/* int checkflag = -1; */
/* this sets up logical character widths in charwid[k] */

/* Widths in the table are based on 1000 em size */

/* If bUseCharSpacing is non-zero we set up the vector charwid[] of widths */
/* Adobe recommends against this - hurts printer driver efficiency ? */

/* long AccurateExtent(unsigned char *s, int n) */
long AccurateExtent(char *str, int n) {				/* 1992/Oct/3 */
	int k;
	int width;
	long sumx = 0;
	long result;		/* debugging temp */
	char *s=str;

/*  here we could get shafted by char being signed ? */
/*	if (bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0) */
	if (bUseCharSpacing == 0) {		/* Favour Position OFF */
		for (k = 0; k < n; k++) sumx += lpCharWidths[*s++ & 255];
#ifdef IGNORED
		if (bDebug > 1) {
			char *t=debugstr;
			int c;
			s = str;
			sprintf(t, "n %d sumx %ld: ", n, sumx);
			t += strlen(t);
			for (k= 0; k < n; k++) {
				c = (*s & 255);
				sprintf(t, "%d %c %d ", k, *s, lpCharWidths[c]);
				t += strlen(t);
				s++;
			}
			strcat(t, "\n");
			OutputDebugString(debugstr);
		}
#endif
	}
	else {							/* Favour Position ON */
		for (k = 0; k < n; k++) {
			width = lpCharWidths[*s++ & 255];		/* ??? */
			sumx += width;
/*			charwid[k] = mapd((width * atsize / 1000;   */ /* overflow */

			charwid[k] = mapd(MulDiv(width, atsize, 1000));

/*			charwid[k] = mapd((width * (atsize >> 3) + 62) / 125); */

/*			above should be safe from overflow - for a single character */
		}
	}
/*	result = ((sumx * atsize) / 1000); */	/* overflow problems */

	result = MulDiv(sumx, atsize, 1000);

/*	result = ((sumx * (atsize / 125) + 3) >> 3); */

/*	made it safe from overflow even for long strings */
#ifdef IGNORED
	if (result < 0) { 
		if (bDebug > 1) {
			sprintf(debugstr, "sumx %ld atsize %ld result %ld\n",
				sumx, atsize, result);
			OutputDebugString(debugstr);		/* debugging only 1993/Oct/2 */
		}
	}
#endif
	return result; 
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* remember: units for drift and maxdrift are TWIPS */
/* if maxdrift = 0  then this just does dvi_hh = mapx(dvi_h) */

void horizontaldrift(void) {
	int xx, drift;

	xx = mapx(dvi_h);
	if (maxdrift == 0) {	/* added for speed */
		if (dvi_hh != xx) {
			dvi_hh = xx;
			cp_valid = 0;
		}
		return;
	}
	drift = dvi_hh - xx;
	if (drift > maxdrift) {
		dvi_hh = xx + maxdrift;
		cp_valid = 0;
	}
	else if (drift < - maxdrift) {
		dvi_hh = xx - maxdrift;
		cp_valid = 0;
	}
} 

void horizontal(long b) {
	int delx;

	if (b == 0) return;					/* NOP */
	dvi_h += b;
	if ((b > 0 && b < wordspace) ||
		(b < 0 && b > -backspace))	{
		delx = mapd(b);
		if (delx != TWIPLIM && delx != - TWIPLIM) {
			dvi_hh += delx;
		}
		else {
			dvi_hh = mapx(dvi_h);
			cp_valid = 0;
		}
		if (bMaxDriftFlag > 0) horizontaldrift();
	}
	else dvi_hh = mapx(dvi_h);
/*	dvi_hh = mapx(dvi_h); */	/* TRIAL BALLOON */
	cp_valid = 0;
} 

void verticaldrift(void) {
	int yy, drift;

	yy = mapx(dvi_v);
	if (maxdrift == 0) {	/* added for speed */
		if (dvi_vv != yy) {
			dvi_vv = yy;
			cp_valid = 0;
		}
		return;
	}
	drift = dvi_vv - yy;
	if (drift > maxdrift) {
		dvi_vv = yy + maxdrift;
		cp_valid = 0;
	}
	else if (drift < - maxdrift) {
		dvi_vv = yy - maxdrift;
		cp_valid = 0;
	}
}

void vertical(long a) {
	int dely;

	if (a == 0) return;
	dvi_v += a; 
	if ((a > 0 && a < verspace) ||
		(a < 0 && a > -verspace)) {
		dely = mapd(a);
		if (dely != TWIPLIM && dely != - TWIPLIM) {
			dvi_vv -= dely;
		}
		else {
			dvi_vv = mapy(dvi_v);
			cp_valid = 0;
		}
 		if (bMaxDriftFlag > 0) verticaldrift();
	}
	else dvi_vv = mapy(dvi_v);
	cp_valid = 0;
} 
		
/* the following are simpler versions valid for maxdrift == 0 */
		
/*
void horizontaldrift(void) {
	int x;
	x = mapx(dvi_h);
	if (dvi_hh != x) {
		dvi_hh = x;
		cp_valid = 0;
	}
} */

void horizontalsimple(long b) {
	int xx;
	if (b == 0) return;
	dvi_h += b;
	xx = mapx(dvi_h);
	if (dvi_hh != xx) {
		dvi_hh = xx;
		cp_valid = 0;
	}
} 

/*
void verticaldrift(void) {
	int y;
	y = mapx(dvi_v);
	if (dvi_vv != y) {
		dvi_vv = y;
		cp_valid = 0;
	}
} */

void verticalsimple(long a) {
	int yy;
	if (a == 0) return;
	dvi_v += a;
	yy = mapy(dvi_v);
	if (dvi_vv != yy) {
		dvi_vv = yy;
		cp_valid = 0;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void GreekText(HDC hDC, RECT Rect) {
/*	DWORD CurPos; */
	HBRUSH hBrushTemp=NULL; 
	HPEN hPenTemp=NULL;
	int curx, cury;
	int a; /* b; */

/*	do NOT use CurrentPosition - to avoid MetaFile problems */
/*	use instead mapped values of dvi_h and dvi_v */
	curx = mapx(dvi_h);
	cury = mapy(dvi_v); 
	a = Rect.right - Rect.left; 
/*	b = Rect.top - Rect.bottom; */

	if (bGreyPlus == 0)	{
/*		Avoid FillRect in MetaFile */				/* 94/Sep/28 */
/*		if (bUseRect != 0 && bCopyFlag == 0) */
		if (bUseRect == 1 && bCopyFlag == 0)		/* 96/Aug/18 */
			(void) FillRect(hDC, &Rect, hGreyBrush);
		else  {
			hBrushTemp = SelectObject(hDC, hGreyBrush);
/*			hPenTemp = SelectObject(hDC, GetStockObject(NULL_PEN)); */
			hPenTemp = SelectObject(hDC, hGreyPen);
			(void) Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);
			if ((UINT) hBrushTemp > 1)
				(void) SelectObject(hDC, hBrushTemp); 
			if ((UINT) hPenTemp > 1)
				(void) SelectObject(hDC, hPenTemp); 
		}
/*	now advance the width of the rule ... */
		MoveToEx(hDC, curx + a, cury, NULL); 
	}
	else {
/*		Avoid FillRect in MetaFile */			/* 1994/Sep/28 */
/*		if (bUseRect != 0 && bCopyFlag == 0) */
		if (bUseRect == 1 && bCopyFlag == 0)		/* 96/Aug/18 */
			 (void) FillRect(hDC, &Rect, hLightGreyBrush);	/* 1994/Sep/28 */
		else {
			hBrushTemp = SelectObject(hDC, hLightGreyBrush);
/*			hPenTemp = SelectObject(hDC, GetStockObject(NULL_PEN)); */
			hPenTemp = SelectObject(hDC, hLightGreyPen); 
			(void) Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);
			if ((UINT) hBrushTemp > 1)
				(void) SelectObject(hDC, hBrushTemp); 
			if ((UINT) hPenTemp > 1)
				(void) SelectObject(hDC, hPenTemp); 
		}
/* now advance the width of the text string ... */
		MoveToEx(hDC, curx, cury, NULL); 
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Type independent min and max macros */
#define MIN(A,B) ( (A < B) ? A : B )
#define MAX(A,B) ( (A > B) ? A : B )

/* Brute force */ /* Built in IntersectRect doesn't seem to work */

BOOL InterSectRect(const RECT FAR *ClipRect, const RECT FAR *TextRect) {
	int cl, ct, cr, cb, tl, tt, tr, tb;

	cl = ClipRect->left;	ct = ClipRect->top;
	cr = ClipRect->right;	cb = ClipRect->bottom;
	tl = TextRect->left;	tt = TextRect->top;
	tr = TextRect->right;	tb = TextRect->bottom;  
	if (MAX(tl, tr) < MIN(cl, cr)) return 0;
	if (MIN(tl, tr) > MAX(cl, cr)) return 0;
	if (MAX(tt, tb) < MIN(ct, cb)) return 0;
	if (MIN(tt, tb) > MAX(ct, cb)) return 0;	
	return -1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*	THIS IS THE SANE WAY TO DO THIS - other method now defunct */
/*	Come here either with character code 0 - 127 OR from set1 OR put1 */
/*	Modified to deal with put1 as well as set1 ... 1994/Aug/10 */

/*	Too many branches - too complicated ... */

/* void normalcharold (HFILE input, int c) */
void normalchar (HFILE input, int c) {
	int k;
	int n;
	int refx, refy;
	int dx, dy;				/* WORD ? */
	int devx, devy;			/* WORD ? */
/*	int deltax; */
	int dmax, dmiy, dmay;
	long ldx;
	RECT TextRect;	 					/* rectangle occupied by text */
	unsigned char *s;
/*	char *s; */
/*	int *lpDx=NULL; */					/* NULL or pointer to widhts */
	POINT CurPoint;
	SIZE TextSize;
/*	DWORD CurPos; */
/*	DWORD escapement; */
	int fn;								/* 95/Mar/5 */
	int bPutFlag;						/* 95/Aug/15 */

	if (bSkipFlag == 0) charbufinx = 0;	/* reset pointer character buffer */

	if (c == (int) put1) bPutFlag=1;
	else bPutFlag=0;

/*	accumulate a whole string of characters up to next non character */
/*	may want to limit this in future ... set StringLimit accordingly */
/*	while (c < 128 || c == (int) set1) */
/*	Take advantage of the fact that set1 == 128 95/Aug/15 */
	while (c <= (int) set1) {							/* 95/Aug/15 */
		if (c == (int) set1) c = an_wingetc(input);		/* new ! */
		if (bSkipFlag == 0)	charbuf[charbufinx++] = (unsigned char) c;
		c = an_wingetc(input);				  /* look at next byte */
		if (charbufinx >= StringLimit) break; /* don't overfill */
/*		if (bMarkSpotFlag > 0) break; */		/* SLOWER, but BETTER */
		if (bMarkSpotFlag > 0 && bShowFlag == 0) break;	
		if (bTagFlag > 0 && bShowFlag == 0) break;	/* 1992/June/11 */
/*		if (bPutFlag) break; */
	} 
/* 	an_unwingetc(c, input); */	/* put back the terminator */
	if (bPutFlag == 0) 	{		/* normal char (0 -- 127) or set1 */
		an_unwingetc(c, input);	/* put back the terminator */
	}
	else {						/* case of put1 */
		c = an_wingetc(input);	/* new ! we did not grab it above */
		if (bSkipFlag == 0) charbuf[charbufinx++] = (unsigned char) c;
	}

	if (bSkipFlag != 0) return;		/* 1992/April/11 */

/*		HACK, until we figure out what is really going on !!! */
/*		refx = mapx(dvi_h); refy = mapy(dvi_v); */
/*		(void) MoveToEx(hdc, refx, refy, NULL); */

	cp_valid = 0;			/* now always !!! */ /* 92/March/25 */

	if (cp_valid == 0) {	/* need to set current point on window ? */
		refx = mapx(dvi_h); refy = mapy(dvi_v);
/*		WARNING: don't really depend on CurPos --- problems in metafiles ... */
		MoveToEx(hdc, refx, refy, &CurPoint); /* move & read current point */
		devx = CurPoint.x;
		devy = CurPoint.y; 
/*		CurPos = MoveTo(hdc, refx, refy); *//* move & read current point */ 
/*		devx = (int) LOWORD(CurPos); */
/*		devy = (int) HIWORD(CurPos); */

/*		avoid use of value returned by MoveTo  --- MetaFile problem */
/*		following flushed since bDVIFakeFlag permanently zero now */
#ifdef IGNORED
		if (bDVIFakeFlag != 0 && bCopyFlag == 0) { 
/*			devx = (int) LOWORD(CurPos); devy = (int) HIWORD(CurPos); */
/*			undo move if difference is small */
			deltax = refx - devx;
			if (refy == devy) {
				if (deltax < DVITYPEINC && deltax > -DVITYPEINC)
					(void) MoveToEx(hdc, devx, devy, NULL);  /* undo move */
			}
		}
#endif
	}

/*  the above ensures that no repositioning if previous was also a `set' */
/*  not anymore ! now we always reposition ... */

	if (fnt_exists == 0) {						/* 1992/April/11 */
		charbufinx = 0; return;	
	}

/*	draw ONLY if font found */

/*	if (bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0) 
		lpDx = NULL;	else lpDx = charwid; */

	if (fnt_texatm != 0) {					/* if remapped 0 -- 31 font */
		s = charbuf;
		for (k = 0; k < charbufinx; k++) {
			n = (*s & 255);					/* 95/Jun/23 */
/*			*s = remaptable[*s & 255]; */
			if (n <= 32 || n == 127) {		/* does this speed up ? */
/*				*s = remaptable[n]; */			/* 95/Jun/23 */
/*				*s = (char) remaptable[n]; */		/* 96/July/25 ? */
				*s = (unsigned char) remaptable[n];		/* 96/July/25 ? */
			}
			s++;
		}
	}
	else if (fnt_ansi != 0) {	/* if ansi, remap low end */
		s = charbuf;
		for (k = 0; k < charbufinx; k++) {
			n = (*s & 255);							/* 95/Jun/23 */
/*			if (*s < 32 && *s >= 16) *s = ansitex[(*s & 255) - 16]; */
/*			if ((*s & 255) < 32) */
			if (n < 32) {
/*				*s = ansitex[(*s & 255)]; */
				*s = ansitex[n];
				if (bNeedANSIacce == 0) {		/* no warning yet remembered */
					fn = finx[dvi_f];			/* use of finx[] */
					if (fnt_exists) {			/* filter 95/Mar/31 */
						if (fontwarned[fn] == 0) {	/* filter 95/Mar/5 */
/*							bNeedANSIfont = dvi_f; */
							nNeedANSIfont = fn;	/* remember font at the time */
							bNeedANSIacce = 1;
							nNeedANSIchar = n;	/* 95/Jun/23 */
						}
					}
				}
			}
			s++;	
		}
	}

	if (bPreserveSpacing == 0) {			/* NO LONGER USE THIS BRANCH */
/*		winerror("IMPOSSIBLE"); */
		if (bCopyFlag == 0 && 
/*			(bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0)) */
			(bUseCharSpacing == 0)) {		/* Favour Position OFF */
/*			possibly use GetTextExtentPoint32 in WIN32 ? */

#ifdef USEUNICODE
/*			if (bUseNewEncode && bFontEncoded) */
			if (bFontEncoded) {
				int k;
				for (k=0; k < charbufinx; k++)
					charbufW[k] = encodeUID[(unsigned char) charbuf[k]];
				(void) GetTextExtentPoint32W(hdc, charbufW, charbufinx,
											 &TextSize); 
			}
			else (void) GetTextExtentPoint32A(hdc, (char *) charbuf, charbufinx,
											  &TextSize);
#else
			(void) GetTextExtentPoint32(hdc, charbuf, charbufinx, &TextSize);
#endif

/*			(void) GetTextExtentPoint(hdc, charbuf, charbufinx, &TextSize); */

			dx = TextSize.cx;
			dy = TextSize.cy;
/* pre Windows 0x030a version */
/*			escapement = GetTextExtent(hdc, charbuf, charbufinx); */
/*			dx = (int) LOWORD(escapement); */
/*			dy = (int) HIWORD(escapement); */
		}		/* end of bUseCharSpacing == 0 */
		else {
			ldx = AccurateExtent((char *) charbuf, charbufinx);
			dx = mapd(ldx);
			dy = ascent + descent;	/* ??? */
		}
	}				/* end of Preserve Spacing == 0 case */
	else {			/* DEFAULT BRANCH */
		ldx = AccurateExtent((char *) charbuf, charbufinx);  /* 91/Dec/27 */
	}

/*	Now actually think about showing the text (or checking tag) ! */
	if ((bShowFlag != 0 && bTextOutFlag != 0) ||
/*		(bShowFlag == 0 && (bTagFlag != 0 || bMarkSpotFlag != 0))) */
		(bShowFlag == 0 && 
			(bTagFlag != 0 || bMarkSpotFlag != 0 || bMarkFlag != 0))) {  /*??*/

		if (bDrawVisible == 0) {		/* WE DON'T USE THIS ANYMORE */
/*			winerror("IMPOSSIBLE");*/
/*			if (bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0) {*/
#ifdef USEUNICODE
			if (bUseCharSpacing == 0) {		/* Favour Position OFF */
/*				if (bUseNewEncode && bFontEncoded) */
				if (bFontEncoded) {
					int k;
/*					redundant ? following already done ? */
					for (k=0; k < charbufinx; k++)
						charbufW[k] = encodeUID[(unsigned char) charbuf[k]]; 
					if (ExtTextOutW(hdc, 0, 0, 0, NULL, 
						charbufW, charbufinx, NULL) == 0) cp_valid = 0;
				}
				else if (ExtTextOutA(hdc, 0, 0, 0, NULL, 
						(char *) charbuf, charbufinx, NULL) == 0) cp_valid = 0;
			}
			else {	/* end of bCharSpacing == 0 */	/* Favour Position ON */
/*				if (bUseNewEncode && bFontEncoded) */
				if (bFontEncoded) {
					int k;
/* redundant ? following already done ? */
					for (k=0; k < charbufinx; k++)
						charbufW[k] = encodeUID[(unsigned char) charbuf[k]];
					if (ExtTextOutW(hdc, 0, 0, 0, NULL, 
						charbufW, charbufinx, charwid) == 0) cp_valid = 0;
				}
				else if (ExtTextOutA(hdc, 0, 0, 0, NULL, 
						(char *) charbuf, charbufinx, charwid) == 0) cp_valid = 0;
			}
#else
			if (bUseCharSpacing == 0) {		/* Favour Position OFF */
				if (ExtTextOut(hdc, 0, 0, 0, NULL,
					charbuf, charbufinx, NULL) == 0) cp_valid = 0;			
			}
			else {							/* Favour Position ON */
				if (ExtTextOut(hdc, 0, 0, 0, NULL,
					charbuf, charbufinx, charwid) == 0) cp_valid = 0;
			}	/* end of bCharSpacing != 0 */
#endif
		}		/* end of DrawVisible == 0 case */
		else {	/* THIS IS THE DEFAULT BRANCH NOW */
/*			avoid getting current position - MetaFile problems */
/*			CurPos = GetCurrentPosition(hdc); */
/*			devx = (int) LOWORD(CurPos); devy = (int) HIWORD(CurPos); */
/*			use instead mapped values of dvi_h and dvi_v */
			devx = mapx(dvi_h); devy = mapy(dvi_v);

			if (bCopyFlag == 0) { 
/*				possibly use GetTextExtentPoint32 in WIN32 ? */

#ifdef USEUNICODE
/*				if (bUseNewEncode && bFontEncoded) */
				if (bFontEncoded) {
					int k;
					for (k=0; k < charbufinx; k++)
						charbufW[k] = encodeUID[(unsigned char) charbuf[k]];
					(void) GetTextExtentPoint32W(hdc, charbufW, charbufinx,
						&TextSize); 
				}
				else (void) GetTextExtentPoint32A(hdc, (char *) charbuf, charbufinx,
					&TextSize); 
#else
				(void) GetTextExtentPoint32(hdc, charbuf, charbufinx, &TextSize);
#endif

/*				(void) GetTextExtentPoint(hdc, charbuf, charbufinx, &TextSize); */

				dx = TextSize.cx;
				dy = TextSize.cy;
/* pre Windows 0x030a version */
/*				escapement = GetTextExtent(hdc, charbuf, charbufinx); */
/*				dx = (int) LOWORD(escapement); */
/*				dy = (int) HIWORD(escapement); */
			}
			else {			/* bCopyFlag != 0 branch */
				ldx = AccurateExtent((char *) charbuf, charbufinx);
				dx = mapd(ldx);
				dy = ascent + descent;	/* ??? */
			}
/*			if (dx < 0 || devx + dx < devx) {
				if (bDebug > 1) {
					sprintf(debugstr, "devx %d dx %d devx + dx %d\n",
						devx, dx, devx+ dx);
					OutputDebugString (debugstr);
				}
			} */				/* debugging 1993/Oct /2 */
			dmax = devx + dx; 
/* This assumes we do not have a string with negative total advance width ! */
/*			if (dmax < devx || devx >= TWIPLIM) */
			if (dmax < devx) { 		/* try and catch wrap around ? */
/*				sprintf(str, "WRAP %d + %d = %d", devx, dx, dmax);
				wincancel(str); */	/* debugging */
				dmax = TWIPLIM;	
			}
			dmiy = devy - descent;
			dmay = devy + dy - descent;  
			if (dmiy > devy) {		/* try and catch wrap around ? */
/*				sprintf(str, "WRAP %d + %d = %d", devy, dy, dmay);
				wincancel(str); */	/* debugging */
				dmiy = -TWIPLIM;	
			} 
			if (dmay < devy) {		/* try and catch wrap around ? */
/*				sprintf(str, "WRAP %d + %d = %d", devy, dy, dmay);
				wincancel(str); */	/* debugging */
				dmay = TWIPLIM;	
			} 
/*	Deal Windows 95 bug with very narrow boxes in RectVisible */
/*	--- somewhat kludgy fix ... 1995/Aug/26 */
/*			if (bWin95) */
			if (bFixZeroWidth) { 
/*  don't do this if we are just trying figure a hit of text rectangle */
				if (bShowFlag) {		/* that is bTagFlag == 0 */
/*					if ((dmax - devx) < (dmay - dmiy))
						dmax = devx + (dmay - dmiy); */
					if ((dmax - devx) < MinWidth) dmax = devx + MinWidth;
				}
			} 
/* 			TextRect.left = devx; TextRect.right = devx + dx; */
			TextRect.left = devx;
			TextRect.right = dmax;
/*			TextRect.top = devy + dy - descent; */
			TextRect.top = dmay;	
/* try and correct for descender (reference is baseline here) */
/*			TextRect.bottom = devy - descent; */
			TextRect.bottom = dmiy;

			if (bTagFlag != 0) {				/* bShowflag == 0 */
				if (bMarkSearch == 1) {			/* 98/Dec/26 */
					devx = devx - (dmax -devx);		/* double width leftward */
					dmay = dmay + (dmay - dmiy);	/* double height upward */
				}
/*				if (TextRect.left <= tagx && TextRect.right >= tagx &&
					TextRect.bottom <= tagy && TextRect.top	>= tagy) */
				if (devx <= tagx && dmax >= tagx &&
					dmiy <= tagy && dmay >= tagy) { 	/* 98/Dec/26 */
						taggedfont = dvi_f; 
/*						taggedchar = *charbuf;	*//* the very character hit */
						taggedchar = (unsigned char) *charbuf;	/* the character hit */
/* following addded 1994/Mar/7 */
						if (bCopyFlag == 0) { 
/* possibly use GetTextExtentPoint32 in WIN32 ? */

#ifdef USEUNICODE
/*							if (bUseNewEncode && bFontEncoded) */
							if (bFontEncoded) {
								int k;
								for (k=0; k < charbufinx; k++)
									charbufW[k] = encodeUID[(unsigned char) charbuf[k]];
								(void) GetTextExtentPoint32W(hdc, charbufW,
									charbufinx, &TextSize);
							}
							else (void) GetTextExtentPoint32A(hdc, (char *) charbuf,
								charbufinx, &TextSize); 
#else
							(void) GetTextExtentPoint32(hdc, charbuf, charbufinx, &TextSize);
#endif

/*							(void) GetTextExtentPoint(hdc, charbuf, charbufinx, &TextSize); */

							dx = TextSize.cx;
							dy = TextSize.cy;
/*							escapement = GetTextExtent(hdc, charbuf, charbufinx); */
/*							dx = (int) LOWORD(escapement); */
/*							dy = (int) HIWORD(escapement); */
						}
						else {
							ldx = AccurateExtent((char *) charbuf, charbufinx);
							dx = mapd(ldx);
							dy = ascent + descent;
						}
						taggedwidth = dx;
/*						ltaggedwidth = AccurateExtent(charbuf, charbufinx); */
						ltaggedwidth = lpCharWidths[taggedchar];
						srctaghit = 1;	/* hit the text clicked on 98/Nov/5 */
/* in inverse search, make sure we have already seen a SRC special */
						if (bMarkSearch == 1) {		/* 98/Dec/20 */
							if (srclineno > 0 && *srcfile != '\0') {
								finish = -1;
							}
						}
						else finish = -1;
/*				sprintf(str, 
					"left %d right %d bottom %d top %d", 
						TextRect.left, TextRect.right, 
							TextRect.bottom, TextRect.top);
				wincancel(str); */
				}
			} /* end of if bTagFlag != 0 */

			if (bShowFlag != 0) {	/* i.e. bTagFlag == 0 */
/*	Avoid RectVisible() in MetaFile - use InterSectRect instead */
				if ((bCopyFlag == 0 && RectVisible(hdc, &TextRect) != 0) ||
					(bCopyFlag != 0 && InterSectRect(&CopyClipRect, 
						&TextRect) != 0)) {
					if (bGreyText != 0 || bGreyPlus != 0) 
						GreekText(hdc, TextRect);
					if (bGreyText == 0 || bGreyPlus != 0) {
/*						if (bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0) */
#ifdef USEUNICODE
						if (bUseCharSpacing == 0) { /* Favour Position OFF */
/*						if (bUseNewEncode && bFontEncoded) */
							if (bFontEncoded) {
								int k;
/*								redundant ? following already done ? */
								for (k=0; k < charbufinx; k++)
									charbufW[k] = encodeUID[(unsigned char) charbuf[k]];
								if (ExtTextOutW(hdc, 0, 0, 0, NULL, 
												charbufW, charbufinx, NULL) == 0) cp_valid = 0;
							}
							else if (ExtTextOutA(hdc, 0, 0, 0, NULL, 
								(char *) charbuf, charbufinx, NULL) == 0) cp_valid = 0;
						}
						else {								/* Favour Position ON */
/*							if (bUseNewEncode && bFontEncoded) */
							if (bFontEncoded) {
								int k;
/*						redundant ? following already done ? */
								for (k=0; k < charbufinx; k++)
									charbufW[k] = encodeUID[(unsigned char) charbuf[k]];
								if (ExtTextOutW(hdc, 0, 0, 0, NULL, 
												charbufW, charbufinx, charwid) == 0) cp_valid = 0;
							}
							else if (ExtTextOutA(hdc, 0, 0, 0, NULL, 
								(char *) charbuf, charbufinx, charwid) == 0) cp_valid = 0;
						}
#else		
						if (bUseCharSpacing == 0) {
							if (ExtTextOut(hdc, 0, 0, 0, NULL,
										   charbuf, charbufinx, NULL) == 0) cp_valid = 0;
						}	
						else {
							if (ExtTextOut(hdc, 0, 0, 0, NULL,
										   charbuf, charbufinx, charwid) == 0) cp_valid = 0;
						}
#endif
					} /* bGreyText == 0 || bGreyPlus != 0 */
				} /* if visible */
/*				else if (bCopyFlag != 0 && InterSectRect(&InterRect, 
						&CopyClipRect, &TextRect) == 0) {
sprintf(str, "Clip %d %d %d %d Text %d %d %d %d Inter %d %d %d %d A %d D %d",
CopyClipRect.left, CopyClipRect.top, CopyClipRect.right, CopyClipRect.bottom,
TextRect.left, TextRect.top, TextRect.right, TextRect.bottom,
InterRect.left, InterRect.top, InterRect.right, InterRect.bottom,
ascent, descent);
					wincancel(str); 
				} */ /* debugging */
			}	/* if bShowFlag != 0 */
		}	/* bDrawVisible != 0 */

/*		Do not advance TeX DVI position if it was a put1 command */
		if (bPutFlag == 0) {								/* 95/Aug/15 */
/* dx and ldx may be uninit ??? 98/Mar/23 */
			if (bPreserveSpacing == 0)		/* no longer take this branch */
				dvi_h += unmap(dx);			/* crude  uninit ??? */
			else dvi_h += ldx;				/* more accurate uninit ??? */
/*			sprintf(str, "dx %d unmap(dx) %ld ldx %ld", dx, unmap(dx), ldx);
			wincancel(str);	*/
		}
	}
	charbufinx = 0;		/* reset buffer index */
	if (bPutFlag == 0)		/* 95/Aug/15 */
		cp_valid++;			/* indicate current point on window is valid */
}

/* int tracecount = 0; */

/* could be more efficient here if we ever see several in a row OK */
/* model on "normalchar" if needed OK */
		
void do_set1(HFILE input) { /* new version */
	if (bSkipFlag == 0) {
		normalchar(input, (int) set1);		/* new ! */
/*		cp_valid++;	 */					/* taken care of in normalchar */
	}
	else (void) an_wingetc(input);	/* throw it away */
/* set character c and increase h by width of character */
/* used (normally only) for characters in range 128 to 255 */
}	

void charcodeerr(void) {
	wincancel("Character code > 255"); errcount();
}

/* don't bother making this efficient, since it should never happen */

void do_set2(HFILE input) { /* NOT REALLY NEEDED ! */
	int c;
	c = an_wingetc(input);
	if (bSkipFlag == 0) {
		if (c != 0) {
			charcodeerr();
/*			bSkipFlag = -1 */
/*			finish = -1; */
		}
		do_set1(input);
	}
	else (void) an_wingetc(input);	/* throw it away */
}

void do_set3(HFILE input) { /* NOT REALLY NEEDED ! */
	int c, d;
	c = an_wingetc(input); d = an_wingetc(input);
	if (bSkipFlag == 0) {
		if (c != 0 || d != 0) {
			charcodeerr();
/*			bSkipFlag = -1 */
/*			finish = -1; */
		}
		do_set1(input);
	}
	else (void) an_wingetc(input);	/* throw it away */
}

void do_set4(HFILE input) { /* NOT REALLY NEEDED ! */
	int c, d, e;
	c = an_wingetc(input); d = an_wingetc(input); e = an_wingetc(input);
	if (bSkipFlag == 0) {
		if (c != 0 || d != 0 || e != 0) {
			charcodeerr();
/*			bSkipFlag = -1 */
/*			finish = -1; */
		}
		do_set1(input);
	}
	else (void) an_wingetc(input);	/* throw it away */
}

/* set character c and DO NOT increase h by width of character */

void do_put1(HFILE input) { /* new version */
	if (bSkipFlag == 0) {
		normalchar(input, (int) put1);		/* new ! */
/*		cp_valid++;	 */					/* taken care of in normalchar */
	}
	else (void) an_wingetc(input);	/* throw it away */
/* set character c and increase h by width of character */
/* used (normally only) for characters in range 128 to 255 */
}	

void do_put2(HFILE input) { /* NOT NEEDED */
	int c;
	c = an_wingetc(input);
	if (bSkipFlag == 0) {
		if (c != 0) {
			charcodeerr();
/*			bSkipFlag = -1 */
/*			finish = -1; */
		}
		do_put1(input);
	}
	else (void) an_wingetc(input);	/* throw it away */
}

void do_put3(HFILE input) { /* NOT REALLY NEEDED ! */
	int c, d;
	c = an_wingetc(input); d = an_wingetc(input);
	if (bSkipFlag == 0) {
		if (c != 0 || d != 0) {
			charcodeerr();
/*			bSkipFlag = -1 */
/*			finish = -1; */
		}
		do_put1(input);
	}
	else (void) an_wingetc(input);	/* throw it away */
}

void do_put4(HFILE input) { /* NOT REALLY NEEDED ! */
	int c, d, e;
	c = an_wingetc(input); d = an_wingetc(input); e = an_wingetc(input);
	if (bSkipFlag == 0) {
		if (c != 0 || d != 0 || e != 0) {
			charcodeerr();
/*			bSkipFlag = -1 */
/*			finish = -1; */
		}
		do_put1(input);
	}
	else (void) an_wingetc(input);	/* throw it away */
}

/*	Attempt to get Acrobat to draw equal thickness lines equal thickness */
/*	should really only do this is width >> height or height >> width */
/*	should only do this if the smaller of width and height are small */

/* Windows 3.1 GUI can only draw strokes with rounded ends */
/* so we have to be careful to avoid using this if rule is too wide */
/* may want to shorten each end by 1/4 of line thickness ? */
/* or can draw background color rectangles over the ends to block out ... */

int NewDrawRule (HDC hdc, RECT RuleRect) {		/* 1994/July/5 */
	unsigned int width, height, rulewidth;
	unsigned int epsilon;						/* 1995/Feb/26 */
	int xs, ys, xe, ye;
	HPEN hTempPen;
	HPEN hSavedPen;
	
	width = (unsigned int) ((long) RuleRect.right - RuleRect.left);
	height = (unsigned int) ((long) RuleRect.top - RuleRect.bottom);

	if (width == height) return 0;		/* fast exit for square blot */

	if (width > height) {	/* draw horizontal stroke */
/*		if (width < height * 4) return 0; */	/* not elongated enough */
		if ((width >> 2) < height) return 0;	/* not elongated enough */
		rulewidth = height;
		if (rulewidth > maxrulewidth) return 0;	/* rule is too fat */
/*		xs = RuleRect.left; xe = RuleRect.right; */
		epsilon = rulewidth / 3;
		xs = RuleRect.left + epsilon;				/* 95/Feb/26 */
		xe = RuleRect.right - epsilon;
/*		ys = RuleRect.bottom / 2 + RuleRect.top / 2; ye = ys; */
		ys = (int) (((long) RuleRect.bottom + RuleRect.top) / 2);
		ye = ys;
	}
	else {					/* draw vertical stroke */
/*		if (height < width * 4) return 0; */	/* not elongated enough */
		if ((height >> 2) < width) return 0;	/* not elongated enough */
		rulewidth = width;
		if (rulewidth > maxrulewidth) return 0;	/* rule is too fat */
/*		xs = RuleRect.left / 2 + RuleRect.right / 2; xe = xs; */
		xs = (int) (((long) RuleRect.left + RuleRect.right) / 2);
		xe = xs;
/*		ys = RuleRect.bottom; ye = RuleRect.top; */
		epsilon = rulewidth / 3;
		ys = RuleRect.bottom + epsilon;				/* 95/Feb/26 */
		ye = RuleRect.top - epsilon;
	}
	hTempPen = CreatePen (PS_SOLID, rulewidth, RuleColor);
	if (hTempPen != NULL) {
		hSavedPen = SelectObject (hdc, hTempPen);
		if (hSavedPen != NULL) { 
			MoveToEx (hdc, xs, ys, NULL);
			LineTo (hdc, xe, ye);
		}
		else winerror ("Unable to Select Rule Pen");		/* debug */
/*		moved back here 98/Mar/23 */
		if ((UINT) hSavedPen > 1) 
			(void) SelectObject (hdc, hSavedPen);
		DeleteObject (hTempPen);
	}
	else winerror ("Unable to Create Rule Pen");		/* debug */
/*	if ((UINT) hSavedPen > 1) 
		(void) SelectObject (hdc, hSavedPen);
	DeleteObject (hTempPen); */					/* modified 98/Mar/26 */
	return 1;					/* we *did* draw the rule */
}

/* Do a visibility test in here ? */ 

/* What a royal pain in the ass! */

void DrawRule(long a, long b) {	/* height a width b at current dvi_h dvi_v */
	int xll, yll, xur, yur; 
	int ull, vll, uur, vur; 
	int xlld, ylld, xurd, yurd;
	int delx, dely;
	int flag, failed;
	int nstep;					/* 1998/Jan/12 */
/*	POINT steparr[1]; */
	POINT steparr[2];			/* 1998/Jan/12 */
	RECT RuleRect;
	RECT TestRect;				/* 1995/Sep/3 */
	POINT Rule[5];				/* 1992/Jan/22 */
	HBRUSH hBrushTemp=NULL;
	HPEN hPenTemp=NULL;

	if (bShowFlag == 0 || bTextOutFlag == 0) return;	/* 1994/July/6 */
/*	if (bShowFlag != 0 && bTextOutFlag != 0) */
/*	if (a <= 0 || b <= 0)  return; */					/* 1994/July/6 */
/*	if (a > 0 && b > 0) */
/*		if (traceflag != 0) {
			sprintf(str, 
				"dvi_h %ld dvi_v %ld b %ld a %ld", dvi_h, dvi_v, b, a);
			wincancel(str);
		} */

	if (bOffsetRule) nstep = 2;
	else nstep = 1;
#ifdef DEBUGRULE
	if (bDebug > 1) {
		sprintf(debugstr, "RULE a %ld b %ld", a, b);
		OutputDebugString(debugstr);
	}
#endif

/*	FIRST, we TRY and do some clever alignment with grid here */
/*	but not if sending output to MetaFile */
	failed = 0;					/* this will get turned on if we fail */
	if (bSnapToFlag != 0 && bCopyFlag == 0) {
		xll = mapx(dvi_h);	 yll = mapy(dvi_v);
/*		following two uncommented as experiment 95/Sep/3 */
/*		if (xll == TWIPLIM || xll == -TWIPLIM) failed++; */
/*		if (yll == TWIPLIM || yll == -TWIPLIM) failed++; */
		steparr[0].x = xll; steparr[0].y = yll;
		flag =  LPtoDP(hdc, steparr, 1);		/* perform snapto */
		if (flag == 0) wincancel("LPtoDP failed"); /* debug, impossible */
		ull = steparr[0].x; 
		vll = steparr[0].y;						/* pixels */
		if (bOffsetRule) {
			steparr[1].x = ull+1;
			steparr[1].y = vll-1;
		}
		flag = DPtoLP(hdc, steparr, nstep);
		if (flag == 0) wincancel("DPtoLP failed"); /* debug, impossible */
		if (bOffsetRule == 0) {
			xlld = steparr[0].x; ylld = steparr[0].y;
		}
		else {
			xlld = (steparr[0].x + steparr[1].x)/2;
			ylld = (steparr[0].y + steparr[1].y)/2;
		}
#ifdef DEBUGRULE
		if (bDebug > 1) {
			sprintf(debugstr, "RULE xll %d yll %d xlld %d ylld %d",
					xll, yll, xlld, ylld);
			OutputDebugString(debugstr);
		}
#endif
/*		if (xlld > xll+maxdrift || xlld < xll-maxdrift) failed++; */
/*		if (ylld > yll+maxdrift || ylld < yll-maxdrift) failed++; */

/*	check first for possible overflow ? */				
		xur = mapx(dvi_h + b);	yur = mapy(dvi_v - a); 
/*	don't use careful method if there is an overflow */
/*		if (xur != TWIPLIM && xur != -TWIPLIM) */
		if (xll != TWIPLIM && xll != -TWIPLIM &&
			xur != TWIPLIM && xur != -TWIPLIM) {	/* fix 95/Sep/3 */
			delx = mapd(b);
			if (delx != TWIPLIM && delx != - TWIPLIM)
				xur = xlld + delx;
		}
/*		if (yur != TWIPLIM && yur != -TWIPLIM) */
		if (yll != TWIPLIM && yll != -TWIPLIM &&
			yur != TWIPLIM && yur != -TWIPLIM) {	/* fix 95/Sep/3 */
			dely = mapd(a);
			if (dely != TWIPLIM && dely != -TWIPLIM)
				yur = ylld + dely;
		}
/*		xur = xlld + mapd(b); yur = ylld + mapd(a); */ /* OLD */
/*		following two uncommented as experiment 95/Sep/3 */
/*		if (xur > TWIPLIM || xur < -TWIPLIM) failed++; */
/*		if (yur > TWIPLIM || yur < -TWIPLIM) failed++; */
		steparr[0].x = xur; steparr[0].y = yur;
		flag =  LPtoDP(hdc, steparr, 1);		/* perform snapto */
		if (flag == 0) wincancel("LPtoDP failed"); /* debug, impossible */
		uur = steparr[0].x; vur = steparr[0].y;			/* pixels */
#ifdef DEBUGRULE
		if (bDebug > 1) {
			sprintf(debugstr, "RULE ull %d vll %d uur %d vur %d",
					ull, vll, uur, vur);
			OutputDebugString(debugstr);
		}
#endif
		if (bAvoidZeroWidth > 0) {			/* min size on device pix */
			if (uur == ull) uur++;			/* min of 1 pixel */
			if (vur == vll) vur--;			/* min of 1 pixel */
/*			work around for 0.4pt x 0.4pt `rules' used by Bezier */
			if (uur == ull+1 && vur == vll-1 && bUseRect == 2) {
				uur++;
				vur--; 		/* need only increase one of them ? */				
			}
/*			fixed 98/Feb/24 - brought inside outer if */
			if (bAvoidZeroWidth > 1 && bUseRect == 2) {
/*				either one of these will do ? */
				if (uur == ull+1) uur++;		/* min of 2 pixel */ 
				if (vur == vll-1) vur--; 	 	/* min of 2 pixel */
			}
		}
		else {
			uur++; vur--;	/* bAvoidZeroWidth == 0, always increase ... */
		}
		steparr[0].x = uur;
		steparr[0].y = vur;
		if (bOffsetRule) {
			steparr[1].x = uur+1;
			steparr[1].y = vur-1;
		}
		flag =  DPtoLP(hdc, steparr, nstep);
		if (flag == 0) wincancel("DPtoLP failed"); /* debug, impossible */
		if (bOffsetRule == 0) {
			xurd = steparr[0].x;
			yurd = steparr[0].y;
		}
		else {
			xurd = (steparr[0].x + steparr[1].x) / 2;
			yurd = (steparr[0].y + steparr[1].y) / 2;
		}
#ifdef DEBUGRULE
		if (bDebug > 1) {
			sprintf(debugstr, "RULE xur %d yur %d xurd %d yurd %d",
					xur, yur, xurd, yurd);
			OutputDebugString(debugstr);
		}
#endif
/*		if (xurd > xur+maxdrift || xurd < xur-maxdrift) failed++; */
/*		if (yurd > yur+maxdrift || yurd < yur-maxdrift) failed++; */

		RuleRect.left = xlld;	RuleRect.bottom = ylld;
		RuleRect.right = xurd;	RuleRect.top = yurd;
		TestRect = RuleRect;			/* for visibility test */
	}	/* end of bSnapToFlag true and bCopyFlag false */

/*	Following uses no snapto ! just raw */
	if (bSnapToFlag == 0 || bCopyFlag != 0 || failed != 0) {
		xll = mapx(dvi_h);		yll = mapy(dvi_v);
/*		xur = mapx(dvi_h + b);	yur = mapy(dvi_v - a); */ /* no ? */
/*	check first for possible overflow ? */				
		xur = mapx(dvi_h + b);	yur = mapy(dvi_v - a); 
/* don't use careful method if there is an overflow anyway */
/*		if (xur != TWIPLIM && xur != -TWIPLIM) */
		if (xll != TWIPLIM && xll != -TWIPLIM &&
			xur != TWIPLIM && xur != -TWIPLIM) {	/* fix 95/Sep/3 */
			delx = mapd(b);
			if (delx != TWIPLIM && delx != - TWIPLIM) 
				xur = xll + delx;
		}
/*		if (yur != TWIPLIM && yur != -TWIPLIM) */
		if (yll != TWIPLIM && yll != -TWIPLIM &&
			yur != TWIPLIM && yur != -TWIPLIM) {	/* fix 95/Sep/3 */
			dely = mapd(a);
			if (dely != TWIPLIM && dely != -TWIPLIM) 
				yur = yll + dely;
		}
/*		if (traceflag != 0) {
			sprintf(str, 
				"xll %d yll %d xur %d yur %d", xll, yll, xur, yur);
			wincancel(str);
		} */
		RuleRect.left = xll;	RuleRect.bottom = yll;
		RuleRect.right = xur;	RuleRect.top = yur;
/*	Deal with  RectVisible problem 95/Sep/3 */
		if (bFixZeroWidth) {
/*			if ((xur - xll) < (yur - yll)) xur = xll + (yur - yll); */
			if ((xur - xll) < MinWidth) xur = xll + MinWidth;
/*			if ((yur - yll) < (xur - xll)) yur = yll + (xur - xll); */
			if ((yur - yll) < MinWidth) yur = yll + MinWidth;
		}									/* kludge 95/Sep/3 */
		TestRect.left = xll;	TestRect.bottom = yll;
		TestRect.right = xur;	TestRect.top = yur;
	}

/*	following will not happen in snapto mode */
	if (RuleRect.right == RuleRect.left) RuleRect.right++;
	if (RuleRect.bottom == RuleRect.top) RuleRect.top++;
/*	following is impossible */
/*	if (RuleRect.left > RuleRect.right) wincancel("LEFT > RIGHT");
	if (RuleRect.bottom > RuleRect.top) wincancel("BOTTOM > TOP"); */

/*	In any case, at this point the rule coordinates are in RuleRect */
/*	So this is a good time to do a RectVisible(hdc, RuleRect) test */
/*  or InterSectRect(&CopyClipRect, &RuleRect) if copying to clipboard */
/*	For on screen we don't care much for time savings, but... */
/*	For copy to ClipBoard we do care since some importers do not clip */
/*	Thin vertical rules may get lost due to Windows 95 bug ??? */
	if (bClipRules) {
/*	First check whether bumping into hyper space - is this safe ??? 95/Sep/3 */
/*		if ((xll == -TWIPLIM && xur == -TWIPLIM) ||
			(xll == TWIPLIM && xur == TWIPLIM) ||
			(yll == -TWIPLIM && yur == -TWIPLIM) ||
			(yll == TWIPLIM && yur == TWIPLIM)) return; */
/*	Then check whether visible */
/*		if ((bCopyFlag == 0 && RectVisible(hdc, &RuleRect) != 0) ||
			(bCopyFlag != 0 && InterSectRect(&CopyClipRect, &RuleRect) != 0)){
		}
		else return; */
		if ((bCopyFlag == 0 && RectVisible(hdc, &TestRect) != 0) ||
			(bCopyFlag != 0 && InterSectRect(&CopyClipRect, &TestRect) != 0)){
		}
		else {
#ifdef DEBUGRULE
			OutputDebugString("RULE: not visible!");
#endif
			return;
		}
	}  /* 1994/Aug/26 */

	if (bRuleFillFlag != 0) {		/* do we want the rules filled (default) */
/*		NOTE: NewDrawRule returns false if it doesn't want to do it ... */
		if (bUseRect == 2 && NewDrawRule(hdc, RuleRect));	/* 1994/July/5 */
/*		else if (bUseRect == 1) */
/*		Avoid FillRect in MetaFile */			/* 1994/Sep/28 */
		else if (bUseRect == 1 && bCopyFlag == 0) {			/* 1994/Sep/28 */
			(void) FillRect(hdc, &RuleRect, hRuleBrush);
		}
		else {				/* bUseRect = 0 */
			hBrushTemp = SelectObject(hdc, hRuleBrush);
/*			hPenTemp = SelectObject(hdc, GetStockObject(NULL_PEN)); */
			hPenTemp = SelectObject(hdc, hRulePen);
/*			{
				int temp;
				temp = RuleRect.top;
				RuleRect.top = RuleRect.bottom;
				RuleRect.bottom = temp;
			} */
#ifdef DEBUGRULE
			if (bDebug > 1) {
				sprintf(debugstr, "RECTANGLE %d %d %d %d",
					RuleRect.left, RuleRect.top, RuleRect.right, RuleRect.bottom);
				OutputDebugString(debugstr);
			}
#endif
/*			(void) Rectangle(hdc, RuleRect.left, RuleRect.top, RuleRect.right, RuleRect.bottom); */
			if (Rectangle(hdc, RuleRect.left, RuleRect.top, RuleRect.right, 
						  RuleRect.bottom) == 0) { 
#ifdef DEBUGRULE
				if (bDebug > 1) {
					OutputDebugString("RULE: Rectangle failed!");
				}
#endif
			}
			if ((UINT) hBrushTemp > 1)
				(void) SelectObject(hdc, hBrushTemp); 
			if ((UINT) hPenTemp > 1)
				(void) SelectObject(hdc, hPenTemp); 
		}			/* end of bUseRect false */
	}				/* end of bRuleFillFlag true */
	else {			/* if rule is *not* to be filled */
/*		if (bUseRect == 1) 	FrameRect(hdc, &RuleRect, hRuleBrush); 
		else */ /* FrameRect doesn't seem to work ??? */

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
		Polyline(hdc, Rule, 5);			/* 1993/Jan/22 */
	}	/* end of bRuleFillFlag false */
}

void do_set_rule(HFILE input) {
	long a, b;
	
	a = sreadfour(input);					/* height */
	b = sreadfour(input);					/* width */
	if (bSkipFlag == 0) {
/*	set black rectangle of height a and width b, then h <- h + b */
/*	nothing is shown unless a > 0 and b > 0 */
/*	need to do clever things to align properly with underlying grid */
		if (a > 0 && b > 0) DrawRule(a, b);			/* 95/Mar/27 */
/*		current position NOT used OR modified by `Rect' */
/*		horizontal(b); */
		horizontalsimple(b);	/* just avoid maxdrift bullshit for rules */
/*		dvi_h += b; */
/*		cp_valid = 0; */
	}
}

void do_put_rule(HFILE input) {
	long a, b;
	
	a = sreadfour(input);
	b = sreadfour(input);
	if (bSkipFlag == 0) {
/*	set black rectangle of height a and width b, then DO NOT h <- h + b */
/*	nothing is shown unless a > 0 and b > 0 */
		if (a > 0 && b > 0) DrawRule(a, b);			/* 95/Mar/27 */
/*	do NOT change current point or dvi_h */
/*		cp_valid = 0; */ /* current point in winwow status unchanged */
	}
}

/* do even if skipping page ... ? */

void do_bop(HFILE input) {		/* beginning of page */
	int k;
/*	long pagestart; */

/*	pagestart = an_wintell(input) - 1; */	/* remember bop position */
/*	pagenumber++; */				/* increment page number */
	stackinx = 0;					/* reset DVI stack counter */
	if (bCarryColor == 0) colorindex = 0;	/* reset color index */
	if (bAllowClip == 0) nDCstackindex = 0;	/* reset DC stack index */
#ifdef AllowCTM
	resetCTM(hdc);		/* or call resetCTM() in winspeci.c 96/Nov/3 */
#endif

/*	dvi_h = ONEINCH; dvi_v = ONEINCH; */ /* 1" across, 1" down */
/*  This adjustment is because magnification is wrt TeX origin, not 0,0 */

	dvi_h = MulDiv(oneinch, 1000, dvi_mag);	/* 1 inch across */
	dvi_v = MulDiv(oneinch, 1000, dvi_mag);	/* 1 inch across */

/*	dvi_h = oneinch / dvi_mag * 1000; */
/*	dvi_v = oneinch / dvi_mag * 1000; */

	dvi_hh = mapx(dvi_h); dvi_vv = mapy(dvi_v); 
/*	(void) MoveToEx(hdc, dvi_hh, dvi_vv, NULL); */
/*	cp_valid = 1; */
	cp_valid = 0;			/* current point in window NOT valid */

	dvi_w = 0; dvi_x = 0; dvi_y = 0; dvi_z = 0; /* needed ??? */
	dvi_f = -1;					/* undefined font */

	for(k=0; k < 10; k++) counter[k] = sreadfour(input); /* TeX counters */

/*	if (textures != 0) (void) sreadfour(input); */	
/*	else  previous = sreadfour(input); */		/* pointer to previous page */
	(void) sreadfour(input);			/* not needed if we have page table */

	bSkipFlag = 0;					/* resetting it ... */
/*	if (bSpreadFlag != 0 && bCountZero != 0) {
		if (bLeftPage != 0 && (counter[0] & 1) != 0) bSkipFlag = 1;
		if (bLeftPage == 0 && (counter[0] & 1) == 0) bSkipFlag = 1;		
		if (dvipage < 0) bSkipFlag = 1;
	} */
/*	actually, bSkipFlag should never end up being non-zero now */
/*	current = pagestart; */	/* OK ? */
	page_seen++;
/*	this is the place to set everything up for displaying the desired page */
/*	if (colorindex != 0 && ! bColorFont) */
	if (bCarryColor && bColorUsed && ! bColorFont) {
/*		RestoreColorStack(pageno); */
		RestoreColorStack(dvipage);
		doColorPop(hdc); /* 98/Feb/14 */
/*		RestoreBack(dvipage, 1); */	/* 98/Jun/2 too late ! */
	}
	penwidth = 1;				/* reset TPIC line width */
}

void do_eop(HFILE input) { /* end of page */
	int c;

#ifdef AllowCTM
	resetCTM(hdc);			/* or call resetCTM() in winspeci.c 97/Feb/27 */
#endif
	cp_valid = 0; 
	if (bSkipFlag == 0) {	/* finish what started at bop */
#ifdef AllowCTM
		if (CTMstackindex > 0) checkCTM(hdc);		/* 1996/Nov/3 */
#endif
		if (colorindex > 0) CheckColorStack(hdc);	/* 1996/Nov/3 */
		if (bCarryColor && bColorUsed && ! bColorFont) {
/*			doColorPush(hdc); */ /* NO - do in winpslog.c */
/*			UGH! This only makes sense if we page sequentially */
		}
		if (bAllowClip && nDCstackindex > 0) doClipBoxPopAll(hdc);
	}
/*	bSkipFlag = 0; */ /* ??? */
	if (textures != 0) 
		(void) sreadfour(input);	/* skip over length code  - use it ? */
/*	may also want to check whether length is something reasonable ? */

	c = an_wingetc(input); an_unwingetc(c, input);		/* peek ahead */
/*	here we expect to see bop, nop or fnt_def's ONLY */
	if (c >= 0 && c <= 127) { /* this should normally not happen: */
		wincancel("Invalid code between EOP and BOP"); errcount(); 
/*		bSkipFlag = -1; */
/*		finish = -1; */
	}
/*  this is the time to finish the page */
/*	if (c != post) {	}	else {	} */
/*	should be able to stop reading file here if bSkipFlag == 0 ? */
/*	if (bSkipFlag == 0) finish = 1; */	/* we did the page desired */
	finish = 1;			/* no matter what !!! */
}

void do_right1(HFILE input) { /* rare */
	int b;
	b = sreadone(input);
	if (bSkipFlag == 0) {
/*		dvi_h += b; */
/*		cp_valid = 0; */
		horizontal((long) b); 
/*		h = h + b; */
	}
}

void do_right2(HFILE input) {
	int b;
	b = sreadtwo(input);
	if (bSkipFlag == 0) {
/*		dvi_h += b; */
/*		cp_valid = 0; */
		horizontal((long) b); 
/*		h = h + b; */
	}
} 

void do_rightsub(long b) {
	if (bSkipFlag == 0) {
/*		dvi_h += b; */
/*		cp_valid = 0; */
		horizontal(b); 
/*		h = h + b; */
	}
}

void do_right3(HFILE input) {
	do_rightsub(sreadthree(input));
} 

void do_right4(HFILE input) {
	do_rightsub(sreadfour(input));
} 

void do_w0(void) {
	if (bSkipFlag == 0) {
/*		dvi_h += dvi_w; */
/*		cp_valid = 0; */
		horizontal(dvi_w);
/*		h = h + w; */
	}
}

void do_w1(HFILE input) { /* rare */
	dvi_w = sreadone(input);
	if (bSkipFlag == 0) {
/*		dvi_h += dvi_w; */
/*		cp_valid = 0; */
		horizontal(dvi_w);
/*		h = h + w; */
	}
}

void do_w2(HFILE input) {
	dvi_w = sreadtwo(input);
	if (bSkipFlag == 0) {
/*		dvi_h += dvi_w; */
/*		cp_valid = 0; */
		horizontal(dvi_w);
/*		h = h + w; */
	}
} 

void do_wsub(long w) {
	dvi_w = w;
	if (bSkipFlag == 0) {
/*		dvi_h += w; */
/*		cp_valid = 0; */
		horizontal(dvi_w);
/*		h = h + w; */
	}
}

void do_w3(HFILE input) {
	do_wsub(sreadthree(input));
} 

void do_w4(HFILE input) {
	do_wsub(sreadfour(input));
} 

void do_x0(void) {
	if (bSkipFlag == 0) {
/*		dvi_h += dvi_x; */
/*		cp_valid = 0; */
		horizontal(dvi_x);
/*		h = h + x; */
	}
}

void do_x1(HFILE input) { /* rare */
	dvi_x = sreadone(input);
	if (bSkipFlag == 0) {
/*		dvi_h += dvi_x; */
/*		cp_valid = 0; */
		horizontal(dvi_x);
/*		h = h + x; */
	}
}

void do_x2(HFILE input) {
	dvi_x = sreadtwo(input);
	if (bSkipFlag == 0) {
/*		dvi_h += dvi_x; */
/*		cp_valid = 0; */
		horizontal(dvi_x);
/*		h = h + x; */
	}
} 

void do_xsub(long x) {
	dvi_x = x;
	if (bSkipFlag == 0) {
/*		dvi_h += dvi_x; */
/*		cp_valid = 0; */
		horizontal(dvi_x);
/*		h = h + x; */
	}
}

void do_x3(HFILE input) {
	do_xsub(sreadthree(input));
}

void do_x4(HFILE input) {
	do_xsub(sreadfour(input));
}

void do_down1(HFILE input) { /* rare */
	int a;
	a = sreadone(input);
	if (bSkipFlag == 0) {
/*		dvi_v += a; */
/*		cp_valid = 0; */
		vertical((long) a);
/*		v = v + a; */
	}
}

void do_down2(HFILE input) { /* rare */
	int a;
	a = sreadtwo(input);
	if (bSkipFlag == 0) {
/*		dvi_v += a; */
/*		cp_valid = 0; */
		vertical((long) a);
/*		v = v + a; */
	}
} 

void do_downsub(long a) {
	if (bSkipFlag == 0) {
/*		dvi_v += a; */
/*		cp_valid = 0; */
		vertical(a);
/*		v = v + a; */
	}
}

void do_down3(HFILE input) {
	do_downsub(sreadthree(input));
}

void do_down4(HFILE input) {
	do_downsub(sreadfour(input));
} 

void do_y0(void) {
	if (bSkipFlag == 0) {
/*		dvi_v += dvi_y; */
/*		cp_valid = 0; */
		vertical(dvi_y); 
/*		v = v + y; */
	}
}

void do_y1(HFILE input) { /* rare */
	dvi_y = sreadone(input);
	if (bSkipFlag == 0) {
/*		dvi_v += dvi_y; */
/*		cp_valid = 0; */
		vertical(dvi_y);
/*		v = v + y; */
	}
}

void do_y2(HFILE input) {
	dvi_y = sreadtwo(input);
	if (bSkipFlag == 0) {
/*		dvi_v += dvi_y; */
/*		cp_valid = 0; */
		vertical(dvi_y);
/*		v = v + y; */
	}
} 

void do_ysub(long y) {
	dvi_y = y;
	if (bSkipFlag == 0) {
/*		dvi_v += y; */
/*		cp_valid = 0; */
		vertical(dvi_y);
/*		v = v + y; */
	}
}

void do_y3(HFILE input) {
	do_ysub(sreadthree(input));
} 

void do_y4(HFILE input) { /* not used */
	do_ysub(sreadfour(input));
} 

void do_z0(void) {
	if (bSkipFlag == 0) {
/*		dvi_v += dvi_z; */
/*		cp_valid = 0; */
		vertical(dvi_z);
/*		v = v + z; */
	}
}

void do_z1(HFILE input) {  /* rare */
	dvi_z = sreadone(input);
	if (bSkipFlag == 0) {
/*		dvi_v += dvi_z; */
/*		cp_valid = 0; */
		vertical(dvi_z);
/*		v = v + z; */
	}
}

void do_z2(HFILE input) {
	dvi_z = sreadtwo(input);
	if (bSkipFlag == 0) {
/*		dvi_v += dvi_z; */
/*		cp_valid = 0; */
		vertical(dvi_z);
/*		v = v + z; */
	}
} 

void do_zsub(long z) {
	dvi_z = z;
	if (bSkipFlag == 0) {
/*		dvi_v += z; */
/*		cp_valid = 0; */
		vertical(dvi_z);
/*		v = v + z; */
	}
}

void do_z3(HFILE input) {
	do_zsub(sreadthree(input));
} 

void do_z4(HFILE input) {
	do_zsub(sreadfour(input));
} 

/**********************************************************************/

/* #define MAXPALINX 32 */

#define MAXPALINX 64				/* 1994/Match/9 */

int npalinx=MAXPALINX;

int palettemap[MAXPALINX];

/*	this need be done only once when starting up */ /* initinstance */

/* rewritten 1994/Mar/7 - partly to allow nPaletteOffset */

int buildpalettemap(HDC hDC) {
	DWORD palinx;
	DWORD colori;
	DWORD colorpal[MAXPALINX];
	int k, m, palindex=0;
	int kmod, flag;
	int r, g, b;
/*	int ro, go, bo; */
	
/*	avoid bright colors, such as white, yellow and light grey */
	for (k = 0; k < 256; k++) { 
/*		palinx = PALETTEINDEX(k); */
		kmod = (k + nPaletteOffset) & 255;
		palinx = PALETTEINDEX(kmod);	/* 1994/Mar/9 */
		colori = GetNearestColor(hDC, palinx);
		r = GetRValue(colori);
		g = GetGValue(colori);
		b = GetBValue(colori);
/*		if (k == 0) { ro = r; go = g; bo = b; } */
/*		else if (ro == r && go == g && bo == b) break; */
		if ((r + r + g + g + g + b) > nMaxBright * 6) continue;	/* too bright */
		flag = 0;
		for (m = 0; m < palindex; m++) {	/* see if we already have it */
			if (colorpal[m] == colori) {
				flag = 1;
				break;
			}
		}
		if (flag) continue;					/* this color already used */
		colorpal[palindex] = colori;		/* save it for comparison */
/*		if ((r + r + g + g + g + b) < 1100) {*/
/*			palettemap[i++] = k; */
		palettemap[palindex] = kmod;
		palindex++;
		if (palindex >= MAXPALINX) break;		/* found enough */
/*		}*/
/*		sprintf(str, "k %d i %d r %d g %d b %d", k, i, r, g, b); */
/*		wincancel(str); */			/* debugging */
	}
/*	sprintf(str, "k %d i %d", k, i); wincancel(str); */
	npalinx = palindex;
/* sigh, at this point we don't know */
/*	if (bDebug && bDebugKernel) */
	if (bDebug > 1) {
		sprintf(debugstr, "Color Fonts: %d palette colors\n", npalinx);
		OutputDebugString(debugstr);
	}
/*	if (i < MAXPALINX) {
		for (k = i; k < MAXPALINX; k++) palettemap[k] = palettemap[k-i];
	}
	npalinx = MAXPALINX; */
	return npalinx;
}

/* following version appears to make assumptions about the palette ... */

/* DWORD ChangeColor(HDC hDC, int ff) {
	DWORD oldcolor;
	int pindex;
	pindex = ff % 16; pindex = 15 - pindex;
	if (pindex >= 10) pindex++; 
	if (pindex >= 15) pindex++;	
	oldcolor = SetTextColor(hDC, PALETTEINDEX(pindex));
	return oldcolor;
} */

/* The returned old color is no good in MetaFile DC - so don't use it */

DWORD ChangeColor(HDC hDC, int ff) {
	DWORD oldcolor;
	int pindex, k;

/*	buildpalettemap(hDC); */		/* now do once,  ahead of time */
	if (bColorStable) pindex = ff % npalinx;
	else pindex = finx[ff] % npalinx;	/* use `short' number */
/*	pindex = (npalinx - 1) - pindex; */
	k = palettemap[pindex];
	oldcolor = SetTextColor(hDC, PALETTEINDEX(k));
	return oldcolor;				/* don't use if bCopyFlag != 0 */
} 

long OldFore=-1;		/* colors of text before ... */
long OldBack=-1;		/* colors of background before ... */

int nfont;											/* made global 94/Dec/25 */

/* switch to new Windows Font - and also switch metrics to new font */

int switchmetric(HDC hDC, int fn) {
	int ttf;
	WORD options;
	int ret;
/*	HFONT hFontBarf; */

	if (hWidths == NULL) return -1;
	if (lpWidths == NULL) {						/* debugging */
		if (bDebug > 1) {
			sprintf(debugstr, "Missing Memory for Widths %s", "switchmetric");
			OutputDebugString(debugstr);	/* impossible */
		}
		GrabWidths();							/* should not be needed */
		if (lpWidths == NULL) {
			finish = -1; 							/* ? */
			return -1;
		}
	}
	if (fn < 0 || fn > MaxFonts-1) {					/* debugging */
		sprintf(debugstr, "Invalid font number %d > %d (%d)",
				fn, MaxFonts-1, dvi_f);
		if (wincancel(debugstr)) {
			finish = -1;
			return -1;
		}
		fn = 0;
	}

	ttf = fontttf[fn];
/*	bDVITypeFlag always zero these days */
/*	if (bPreserveSpacing != 0 || bDVITypeFlag != 0) */
	if (bPreserveSpacing) {						/* always do this now */
/*		first set up metric information needed in this case */
/*		CAN happen if user switches from bPreserveShape to bPreserveSpacing */
		if (metricsvalid[fn] == 0) {	/* metrics set up earlier? */
/*			wincancel("Metrics of known font unknown!"); */
			(void) setupmetrics(hDC, fn);	/* need to set up metrics */
		}
/*		lpCharWidths = lpWidths + fn * 256; */		/* MAXCHRS ? */
		lpCharWidths = lpWidths + fn * MAXCHRS;		/* MAXCHRS ? */
		adjustatsize(fn);
/*		descent = fontdescent[fn]; */
	}
	if (bATMReencoded) UnencodeFont (hdc, nfont, 1);			/*  ??? */
/*	if (!ttf && bATMPrefer && bATMLoaded) */	/* 95/Mar/17 */
	if (!ttf && bATMPrefer && bATMLoaded && !bCopyFlag) {
/*		This is where we get an error if we are working in MetaFile DC ... */
/*		This is where we get an error if we are working in Fax Board DC ... */
		if (bATMExactWidth) options = ATM_SELECT | ATM_USEEXACTWIDTH;
		else options = ATM_SELECT;
/*		if (bDebug > 1) OutputDebugString("BEFORE\n"); */
/*		discarding old font in this device context ? NULL */
		ret = MyATMSelectObject (hdc, windowfont[fn], options, NULL);  
/*		ret = MyATMSelectObject (hdc, windowfont[fn], options, &hFontBarf); */
/*		if (bDebug && (ret != ATM_SELECTED && ret != ATM_SELECTALT) &&
		    bComplainMissing && !bIgnoreSelect) */
/*		If it is not ATM font, we get -202 error, but font still selected OK */
/*		if (ret != ATM_SELECTED && ret != ATM_SELECTALT) */
		if (ret != ATM_SELECTED && ret != ATM_OLDFONTSELECTED) { 
/*		if (ret != ATM_SELECTED && ret != ATM_SELECTMISSING) {*/ /* ??? */
		    if (bDebug && bComplainMissing && !bIgnoreSelect) { 
				char *fname="UNKNOWN";
				if (fontname[fn] != NULL) fname = fontname[fn];
				sprintf(debugstr, "ATM Select Error %d (%s) %s\n",
/*						 ret, "Window", fontname[fn]); */
						ret, "Window", fname);
				if (bDebug > 1) OutputDebugString(debugstr);
				else winerror (str);
/*				most commonly the error is ATM_SELECTMISSING (-202) */
			}
		}
/*		if (bDebug > 1) OutputDebugString("AFTER\n"); */
	}
	else (void) SelectObject (hdc, windowfont[fn]);
#ifdef USEUNICODE
/*	if (bUseNewEncode) bFontEncoded = ansifont[fn];*/
	if ((bUseNewEncodeTT && fontttf[fn])||
		(bUseNewEncodeT1 && !fontttf[fn])) bFontEncoded = ansifont[fn];
/*	else */	/* fix 97/Jan/26 */
/*	hEncoding will be NULL in Windows NT */
	if (bATMLoaded && hEncoding != NULL && ansifont[fn])	/* ??? */
		ReencodeFont(hdc, fn, 0);							/* 1994/Dec/25 */
#else
	if (bATMLoaded && hEncoding != NULL && ansifont[fn])	/* ??? */
		ReencodeFont(hdc, fn, 0);							/* 1994/Dec/25 */
#endif
	nfont = fn;								/* remember for next switch */
	return fn;
}

/* OldFore & OldBack obtained from SetTextColor hence no good in MetaFile*/

void colorandreverse (void) { /*	deal with color and reverse video stuff */
/*	if (bCopyFlag != 0) return; */				/* 1995/April/30 ? */
	if (OldBack >= 0) {
		(void) SetBkColor(hdc, (unsigned long) OldBack); /* restore back */
		(void) SetBkMode(hdc, TRANSPARENT);
		OldBack = -1;
	}
	if (OldFore >= 0) {
		(void) SetTextColor(hdc, (unsigned long) OldFore);	/* previous ??? */
		OldFore = -1;
	}
/*	if (bReverseVideo != 0) {	
		sprintf(str, "markfont %d dvi-f %d", markfont, dvi_f);
		wincancel(str);
	} */ /* debugging stuff */
	if (dvi_f == markfont || (markfont < 0 && bReverseVideo != 0)) {
/*		OldFore = (long) SetTextColor(hdc, RGB(255,255,255)); *//* white */
/*		OldBack = (long) SetBkColor(hdc, RGB(0,0,0));		 *//* on black */
		OldFore = (long) SetTextColor(hdc, BkColor);	/* 1992/May/06 */
		OldBack = (long) SetBkColor(hdc, TextColor);	/* 1992/May/06 */
		(void) SetBkMode(hdc, OPAQUE);
	}
	else if (bColorFont != 0) (void) ChangeColor(hdc, dvi_f);
}

static void switchfont(HDC hDC, int f) {	/*  switching to another font */
	int fn; 

	dvi_f = f;						/* set font state */
	fn = finx[dvi_f];

	if (bSkipFlag != 0) return;		/* are we `screen setting' ? */
	if (fn < 0 || fn > MaxFonts-1) {
		sprintf(debugstr, "Invalid font number %d > %d (%d)",
				fn, MaxFonts-1, dvi_f);
		if (wincancel(debugstr)) {
			finish = -1;
			return;						/* 1997/Oct/27 */
		}
		fn = 0;
	}

	if (bReverseVideo == 0)		/* don't interfere with reverse video */
		colorandreverse();		/* deal with color and reverse video stuff */

	fnt_exists = fontexists[fn];	/* was font found by ATM ? */
	if (fontvalid[fn] != 0) {		/* is font info valid (seen before) ? */
		if (fnt_exists != 0) switchmetric(hDC, fn);
/*		switch to font at correct size */
	}
/*	else if (fontwarned[fn] != 0) {  
		wincancel("No font switch possible!");
	} */
	else {								/* font does not exists */
		(void) setupfont(hDC, fn);			/* try and make up windows font */
		fnt_exists = fontexists[fn];	/* was font found by ATM ? */
/*		metricsvalid[fn] = 0; */		/* make sure to get new	metrics */
		if (fnt_exists != 0) switchmetric(hDC, fn);
	}

	fnt_texatm = 0;						/* default: use font as is ! */
	if (bATMBugFlag != 0) fnt_texatm = texfont[fn];		/* if remapped font */
	if (fontttf[fn] != 0 && bTTRemap != 0) fnt_texatm = 1;	/* if TT font */
/*	if (bATMBugFlag > 1) fnt_texatm = 1; */	/* always remap in this case */
	if (bATMBugFlag == 2) fnt_texatm = 1;	/* always remap in this case */
#ifdef IGNORED
	if (bDebug > 1) {  
		sprintf(debugstr, "F%d fnt_texatm %d fontttf %d texfont %d\n",
			fn, fnt_texatm, fontttf[fn], texfont[fn]);
		OutputDebugString(debugstr);
	}	/* debug only 1993/Aug/18 overload on output ... */
#endif
/*	winerror(str);	*/								/* debugging */
/*	if (fnt < 0) fnt = 0;	*/	/* for safety sake ??? */
/*	cp_valid = 0; */			/* why ? */
/*	only set fnt_ansi if bANSITeXFlag is set ... */
	if (bANSITeXFlag) fnt_ansi = ansifont[fn];
	else fnt_ansi = 0;			/* does font need low remap ? */
}

void cancelmetrics (void) {		/* reset metricsvalid[] array 98/Sep/20 */
	int fn;
	for (fn = 0; fn < fnext; fn++) {
		metricsvalid[fn] = fontwarned[fn] = 0;
	}
}

/*	make sure all metric information is available when printing (to fax) */
/*	make sure all metric information is available when copying to clipboard */
/*	maybe suppress missing font complaints here ? */
/*	slight rewrite 1999/Jan/8 */

void touchallfontsub(HDC hDC) {
	int fn;
	BOOL bOldComplain;

	bOldComplain = bComplainMissing;
	bComplainMissing = 0;				/* suppress for the moment */

	hdc = hDC;		/* ??? 99/Jan/12 remove ?*/

/*	if (bDebug > 1) OutputDebugString("Touch all Fonts\n"); */
/*	careful with this logic - had a bug 99/Jan/12 */
	for (fn = 0; fn < fnext; fn++) {
		if (fontvalid[fn] == 0)		 		/* font info valid (seen before) ? */
			(void) setupfont(hDC, fn);		/* try and make up windows font */
		fnt_exists = fontexists[fn];	/* was font found by ATM ? */
		if (fnt_exists) {
			if (metricsvalid[fn] == 0)		/* metrics set up earlier? */
				(void) setupmetrics(hDC, fn);	/* need to set up metrics */
/*				switchmetric(hDC, fn); */
		}
	}
/*	if (bDebug > 1) OutputDebugString("Touch all Fonts\n"); */

	hdc = NULL; 						/* to avoid inadvertent reuse ??? */
	bComplainMissing = bOldComplain;
}

/* Should really hav grabbed widths before and release them after */

void touchallfonts(HWND hWnd) {
	HDC hDC;
	int flagwidth=0;

/*	if (bDebug > 1) OutputDebugString("Touch all Fonts\n"); */
	hDC = GetDC(hWnd);
	if (hDC == NULL) {
		winerror("hDC NULL");
		return;
	}

	if (hWidths == NULL) return;
	if (lpWidths == NULL) {					/* 99/Jan/12 */
		if (bDebug > 1) OutputDebugString("GrabWidths in TouchAllFonts");
		GrabWidths();
		if (lpWidths == NULL) {
			finish = -1; 							/* ? */
			return; 
		}
		flagwidth++;						/* make note we grabbed */
	}
	(void) SetMapMode(hDC, MM_TWIPS);
	touchallfontsub(hDC);
	if (flagwidth) ReleaseWidths();			/* 99/Jan/12 */
	(void) ReleaseDC(hWnd, hDC);
/*	if (bDebug > 1) OutputDebugString("Touch all Fonts\n"); */
}

void do_fnt1(HDC hDC, HFILE input) { /* switch fonts */
	unsigned int fnew;
	fnew = ureadone(input); 
/*	fnew = an_wingetc(input); */
/*	if (bSkipFlag == 0) */
		switchfont(hDC, (int) fnew);
}

void fontcodeerr(long fnum) {
/*	sprintf(str, "Font code %ld > 255", fnum); */
	sprintf(str, "Font code %ld > %d", fnum, MAXFONTNUMBERS-1);
	wincancel(str); 
	errcount();
}

void do_fnt2(HDC hDC, HFILE input) { /* switch fonts */
	unsigned int fnew;
	fnew = ureadtwo(input);
/*	if (bSkipFlag == 0) */
/*	if (fnew >= 256) */
	if (fnew >= MAXFONTNUMBERS) {	/* 93/Dec/11 */
		fontcodeerr(fnew);
/*		fnew = 255; */
		fnew = MAXFONTNUMBERS-1;
	}
	switchfont(hDC, (int) fnew);
}

void do_fntsub(HDC hDC, unsigned long fnew) {
/*	if (fnew >= 256) */
	if (fnew >= MAXFONTNUMBERS) {
		fontcodeerr(fnew);
/*		fnew = 255; */
		fnew = MAXFONTNUMBERS-1;
	}
	switchfont(hDC, (int) fnew);
}

void do_fnt3(HDC hDC, HFILE input) { /* switch fonts */
	do_fntsub(hDC, ureadthree(input));
}

void do_fnt4(HDC hDC, HFILE input) { /* switch fonts */
	long fnew;
	fnew = sreadfour(input);
	if (fnew < 0) {
/*		wincancel("Negative font code"); errcount(); */
		fontcodeerr(fnew);
		fnew = 0;
	}
	do_fntsub(hDC, (unsigned long) fnew);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* the following may take more work ... */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long nspecial;		/* length of remaining part of special */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void do_xxxi(HFILE input, unsigned int n) {
/*	unsigned int k; */

/*	if (n < BUFLEN)  dospecial(hdc, input, n); */
	nspecial = n;
	dospecial(hdc, input, (unsigned long) n);
/*	else for(k = 0; k < n; k++) (void) an_wingetc(input); */
/*	cp_valid = 0; */
}

void do_xxx1(HFILE input) { /* for /special */
	unsigned n;
	n = ureadone(input); 
/*	n = an_wingetc(input); */
	do_xxxi(input, n);
}

void do_xxx2(HFILE input) { /* for /special */
	unsigned int n;
	n = ureadtwo(input);
	do_xxxi(input, n);
}

void do_xxxl(HFILE input, unsigned long n) {
/*	unsigned long k; */

/*	if (n < BUFLEN)	 dospecial(hdc, input, (unsigned int) n); */
	nspecial = n;
	dospecial(hdc, input, n);
/*	else for(k = 0; k < n; k++) (void) an_wingetc(input); */
/*	cp_valid = 0; */
}

void do_xxx3(HFILE input) { 
	unsigned long n;
	n = ureadthree(input);
	do_xxxl(input, n);
}

void do_xxx4(HFILE input) { 
	long n;
	n = sreadfour(input);
	do_xxxl(input, (unsigned long) n);
}

#ifdef IGNORED
void checkfontname(int f) {
	if (fontname[f] != NULL) {
		if (bDebug > 1) {
			sprintf(debugstr, "fontname %s (%d) not free",	fontname[f], f);
			OutputDebugString(debugstr);
		}
		free(fontname[f]);
		fontname[f] = NULL;					/* 1999/Jan/4 */
	}
}
#endif /* in winpslog.c */

/* need to do this even if skipping pages */
/* nothing much should actually happen here !!! */
/* ignore this all - should be done properly on prescan !!! */
/* simplify later by flushing stuff ! */

void fnt_def(HFILE input, unsigned int k) {
	unsigned int f, na, nl, i;
	int newfont=1;
//	char fp[MAXTEXNAME];			/* 1995/July/7 */
	char fp[FNAMELEN];			/* 1995/July/7 */

	if (finx[k] != BLANKFONT) { /* has this font been defined already ? */
		newfont = 0;
		f = (unsigned int) finx[k];	/* ok, find its code number */
	}
	else {				/* should NOT ever happen !!! */
		sprintf(str, "Font %u being redefined", k);
/*		wincancel("Font being redefined"); */
		wincancel(str);
		f = (unsigned int) fnext;	/* and would screw things up if it */
		if ((unsigned int) finx[k] != f) {
			sprintf(str, "Font %d not defined in prescan (%d)", k, fnext);
			winerror(str); 
			errcount();
			finish = -1;	/* bSkipFlag = -1 ? */
		}
		fnext++; 
		if (fnext > MaxFonts-1) { 
/*			winerror("Too many fonts in use"); */
			sprintf(str, "More than %d fonts in use", MaxFonts-1);
			winerror(str);
			fnext--;
			errcount();
			finish = -1;
		} 
	} 

	for (k = 0; k < 12; k++) (void) (void) an_wingetc(input); /* don't again */
/*	fc[f] = sreadfour(input); 
	fs[f] = sreadfour(input); 
	fd[f] = sreadfour(input); */
	na = ureadone(input);			/* always zero */
/*	na = an_wingetc(input); */
	nl = ureadone(input); 
/*	nl = an_wingetc(input); */
	if (newfont == 0) {		/* just skip over if already defined */
		for (i = 0; i < na+nl; i++) (void) an_wingetc(input);
	}
	else {					/* this should NOT ever happen !!! */
							/* and if it did it would screw things up */
		for (i = 0; i < na + nl; i++) {
//			if (i < MAXTEXNAME) fp[i] = (char) an_wingetc(input);
			if (i < sizeof(fp)) fp[i] = (char) an_wingetc(input);
			else (void) an_wingetc(input);
		}
//		if (na + nl >= MAXTEXNAME) 
		if (na + nl >= sizeof(fp)) {
			wincancel("Font name too long");	errcount();
//			fp[MAXTEXNAME-1] = '\0';
			*(fp + sizeof(fp) - 1) = '\0';
		}
		else fp[na + nl] = '\0';
		checkfontname(f);				/* free name if left over */
		fontname[f] = zstrdup(fp);		/* 1995/July/15 */
	}
/* dont define fonts here - otherwise pages won't be separable ! */
}

void do_fnt_def1(HFILE input) { /* define font */
/*	unsigned int k; */
/*	k = ureadone(input); */
	fnt_def(input, ureadone(input)); 
/*	fnt_def(input, an_wingetc(input)); */
}

void do_fnt_def2(HFILE input) { /* define font */
	unsigned int k;

	k = ureadtwo(input);
/*	if (k > 255) */
	if (k >= MAXFONTNUMBERS) {
		fontcodeerr(k);
/*		k = 255; */
		k = MAXFONTNUMBERS-1;
	}
	fnt_def(input, (unsigned int) k);
}

void do_fnt_defsub(HFILE input, unsigned long k) {
/*	if (k > 255) */
	if (k >= MAXFONTNUMBERS) {
		fontcodeerr(k);
/*		wincancel("Font code > 255"); errcount(); */
/*		k = 255; */
		k = MAXFONTNUMBERS-1;
	}
	fnt_def(input, (unsigned int) k);
}

void do_fnt_def3(HFILE input) { /* define font */
/*	unsigned long k;
	k = ureadthree(input); */
	do_fnt_defsub(input, (unsigned long) ureadthree(input));
}

void do_fnt_def4(HFILE input) { /* define font */
	long k;

	k = sreadfour(input);
	if (k < 0) {
/*		wincancel("Font code < 0"); */
		fontcodeerr(k);
		k = 0;
	}
	do_fnt_defsub(input, (unsigned long) k);
}

/* need to do this even if skipping pages */

void do_pre(HFILE input) { /* doesn't do output */
	unsigned int k, j; /* i */
/*	int c; */

/*	i = ureadone(input); */
	(void) ureadone(input);  
/*	if (i != ID_BYTE) {
		fprintf(stderr, "File is DVI version %d - program designed for %d\n",
			i, ID_BYTE);
		errcount(); 
	} */

	for (j = 0; j < 12; j++) (void) an_wingetc(input); /* don't read again */
/*	dvi_num = sreadfour(input); 
	dvi_den = sreadfour(input);
	mag = sreadfour(input); */

	k = ureadone(input); 
/*	k = an_wingetc(input); */
/*	s = comment; */
	for (j = 0; j < k; j++) (void) an_wingetc(input); 
/*	redundant:  done in dvipslog */
	if (textures != 0) (void) sreadfour(input);	/* skip over length code */
	cp_valid = 0; 
}

/* need to do this even if skipping pages */

void do_post(HFILE input) { /* doesn't do output */
	int k;

/*	if (textures != 0) sreadfour(input); */	/* not valid for textures files */
/*	else  previous = sreadfour(input); */		/* pointer to previous page */
	(void) sreadfour(input);

	current = an_wintell(input) - 1;
/*	next = -1; */
/*	bop_valid = 0; */

	finish = -1;	/* STOP IT ALREADY ! (before we get to post_post) */

	for (k = 0; k < 12; k++) (void) an_wingetc(input);	/* don't read again */
/*	dvi_num = sreadfour(input);
	dvi_den = sreadfour(input);
	mag = sreadfour(input); */
	for (k = 0; k < 8; k++) (void) an_wingetc(input); 	/* don't read again */
/*	l = sreadfour(input);
	u = sreadfour(input); */
	for (k = 0; k < 4; k++) (void) an_wingetc(input); 	/* don't read again */
/*	s = ureadtwo(input);	
	t = ureadtwo(input);	*/
	cp_valid = 0; 
}

void do_post_post(HFILE input) { /* should never get here ! */
/*	unsigned long q; */
/*	unsigned int i; */
/*	q = sreadfour(input); */
	(void) sreadfour(input);
/*	i = ureadone(input); */
	(void) ureadone(input); 
	finish = -1;			/* STOP it before we hit EOF !!! */
/*	check ID_BYTE again ? */
/*	followed by at least four 223's */
	cp_valid = 0; 
}

/* quick way to get to page in textures using block sizes 
int skiptopage(HFILE input, int wantedpage) { 
	long position;
	ungotten = -1;
	position = dvistart - 4;
	while (++pagenumber < wantedpage) {
		filepos = _llseek(input, position, SEEK_SET);
		buflen = _lread(input, buffer, 4);	
		if (buflen < 4) return EOF;	
		bufptr = buffer;
		position += sreadfour(input);
	}
	return 0;
} */

int dvipageold;
long currentold;
long xoffsetanc;

void savestate (void) {
	xoffsetanc = xoffset;	dvipageold = dvipage;	currentold = current;
}

void restorestate (void) {
	xoffset = xoffsetanc;	dvipage = dvipageold;  	current = currentold; 
}

/*	Is this assuming standard dvi_num and dvi_den ??? */

long leftpageoffset (long x) {
	long xo;
	if (bLandScape)
		xo = x - lmapd(((long) PageHeight + GutterOffset) * 65536 / 2);
	else xo = x - lmapd(((long) PageWidth + GutterOffset) * 65536 / 2);
	return xo;
}

/*	Is this assuming standard dvi_num and dvi_den ??? */

long rightpageoffset (long x) {
	long xo;
	if (bLandScape)
		xo = x + lmapd(((long) PageHeight + GutterOffset) * 65536 / 2);
	else xo = x + lmapd(((long) PageWidth + GutterOffset) * 65536 / 2);
	return xo;
}

/* main entry point to this part of the program */
/* returns -1 if failed for some reason, 0 if OK */

int scandvifile (HFILE input, HDC hDC, int bShowFlagset) { 
	int flag=0;							/* 98/Mar/26 uninit ??? */

	if (hDC == NULL) {							/* debugging only */
		wincancel("Null hDC (scandvifile)");
		return -1;
	}
	if (input == -1) {							/* debugging only */
		wincancel("Invalid File Handle");
		return -1;
	}
	maxdrift = getmaxdrift(hDC);	/* one pixel in terms of TWIPS */
/*	bUserAbort = 0; */
	if (bSpreadFlag == 0) 
		flag = scandvipage(input, hDC, bShowFlagset);	/* easy ! */
	else {
		savestate();	/* xoffsetanc = xoffset etc */
		xoffset = leftpageoffset(xoffsetanc);			/* ? */
		dvipage = leftdvipage;
/*		bLeftPage = 1; */
		current = leftcurrent;
		if (current > 0) 
			flag = scandvipage(input, hDC, bShowFlagset);  /* do left page */
		xoffset = rightpageoffset(xoffsetanc);			/* ? */
		dvipage++;	/* ? */
		dvipage = rightdvipage;
/*		bLeftPage = 0; */
		current = rightcurrent;
/*		if (current > 0 && bUserAbort == 0) */		/*  bMarkSpotFlag >= 0 */
/*	1993/Nov/2 solve a problem: remove bUserAbort */
/*	also: not running scandvipage in this situation doesn't save much time */
#ifdef DEBUGPAGETABLE
		if (bDebug > 1) {
			sprintf(debugstr, "SCANDVI leftdvi %d rightdvi %d leftcurrent %d rightcurrent %d",
					leftdvipage, rightdvipage, leftcurrent, rightcurrent);
			OutputDebugString(debugstr);
		}
#endif
		if (current > 0)							/*  bMarkSpotFlag >= 0 */
			flag = scandvipage(input, hDC, bShowFlagset);  /* do right page */
		else restorestate();						/* wrong ??? */
/*		next = current; current = previous;	previous = before; before = -1; */
		xoffset = xoffsetanc;
/*		leave it set to right hand side page */
	}
	if (bATMReencoded) UnencodeFont (hdc, nfont, 0);			/* 1994/Dec/25 */
	return flag;
}
	
/* #define HOWOFTEN 128 */	/* how often to check for mouse clicks - 2^n */

int scandvipage(HFILE input, HDC hDC, int bShowFlagset) {
	int c;
	long bytecount = 0;
	HPEN hOldPen = (HPEN) -1;

	if (hDC == NULL) {
		wincancel("Null hDC (scandvipage)"); /* debugging only */
		return -1;							 /* 95/Aug/14 */
	}
	if (input == -1) {						/* BAD_FILE ? */
		wincancel("Invalid File Handle");
		return -1;							 /* 95/Aug/14 */
	}

	hdc = hDC;			/* make accessible to other functions ??? */

	OldFore = -1; OldBack = -1;						/* 1992/May/07 */
/*	reset state of things controlled by \special */
/*	bHyperUsed = 0; */				/* DVI page contains hypertext linkage */
									/* kill this for now ... 94/Oct/13 */
	bReverseVideo = 0;
/*	text_r = text_g = text_b = 0; */
/*	rule_r = rule_g = rule_b = 0; */

/*	This is the new way of doing things */
/*	If bColorUsed, then the following will soon be overridden */
/*	if (colorindex == 0 || bCarryColor == 0) */
		(void) SetTextColor(hdc, TextColor); 	/* 1992/May/06 */
		CurrentTextColor = TextColor;			/* 1995/April/30 */
		(void) SetBkColor(hdc, BkColor); 
		CurrentBackColor = BkColor;			/* 1995/April/30 */

		if (hRuleBrush != NULL && hRuleBrush != hRuleBrushDefault) {
			if ((UINT) hRuleBrush > 1) {
				(void) DeleteObject(hRuleBrush);
				hRuleBrush = (HBRUSH) -1;
			}
		}
		hRuleBrush = hRuleBrushDefault;

		hOldPen = SelectObject(hdc, hRulePenDefault);
		if (hOldPen != hRulePenDefault) {
			if ((UINT) hOldPen > 1) {
				(void) DeleteObject(hOldPen);
				hOldPen = (HPEN) -1;
			}
		}
		hRulePen = hRulePenDefault;
		FigureColor = TextColor;				/* 1992/May/06 */
		BackColor = BkColor;					/* 1992/May/06 */
		RuleColor = TextColor;					/* 1993/July/06 */
/*	 */

	srclineno = 0;				/* reset line number */
	*srcfile = '\0';			/* reset source file */
	srctaghit = 0;				/* reset tag 98/Nov/5 */

	nfont = -1;					/* initial font 94/Dec/25 */

/*	setupremaptable();	*/		/* setup remapping table */
	page_seen = 0;				/* desired page not seen yet */

/*	hdc = hDC; */					/* make accessible to other functions */
	(void) SetBkMode(hdc, TRANSPARENT); 	/* background remains untouched */
	(void) SetTextAlign(hdc, TA_BASELINE  | TA_LEFT | TA_UPDATECP); /* ??? */

/*	initialize buffer I/O - redundant ? */
/*	if(an_wininit(input) != 0) return -1; */		/* lossage */
	(void) an_wininit(input);
	if (current < 0) {
		sprintf(debugstr, "Negative file position %ld", current);
		wincancel(debugstr);						/* ??? 95/Dec/21 */
		return -1;								/* lossage */
	}
	(void) an_winseek(input, current);

	finish = 0;					/* not finished yet ! */ 

/*  should see only pre, bop, nop, fnt_def, xxx, or post here */
	c = an_wingetc(input); an_unwingetc(c, input);	/* verify - good place ? */
	if (c == (int) post) {
		wincancel("End of File");		/* ??? */
		finish = -1;
/*		bop_valid = 0; */				/* ??? */
		return 0;
	}
/*	well, could be nop, fnt_def or xxx in theory ??? */
	if (c != (int) pre && c != (int) bop && c != (int) nop) {
		sprintf(debugstr, "Expected BOP, not %d", c);
		wincancel(debugstr);
	}

	stackinx = 0; /* maxstackinx = 0;	*/	/* redundant, hopefully */

/*	if (wanthistogram != 0) for(k = 0; k < 256; k++) histogram[k] = 0; */

/* 	bShowFlag = -1; */
	bShowFlag = bShowFlagset;
	for(;;) {
		if ((bytecount++ & (HOWOFTEN-1)) == 0)	/* see if interrupted */
			if (bEnableTermination  != 0 && checkuser() != 0) { 
				bShowFlag = 0;				/* turn off displaying */
				finish = -1;				/* redundant */
				bUserAbort = 1;					/* set for termination */
/*				break; */ /* ??? */ 
			}

		c = an_wingetc(input);
		if (c == EOF) {
			wincancel("Unexpected EOF on input");
			finish = -1;		/* redundant */
			break; /* giveup(13); */ 
		}
/*		if (wanthistogram != 0) histogram[c]++; */
		if (c < 128) normalchar(input, c);
/* set character in current font and advance h by width of character */
		else if ((c - 171) >= 0 && (c -171) < 64) 
/*			fs = (c - 171);	*/				/*	switch to font (c - 171) */
			switchfont(hDC, c - 171);
		else {
			switch(c) {
				case set1: do_set1(input); break;
				case set2: do_set2(input); break;  /* silly */
				case set3: do_set3(input); break;  /* silly */
				case set4: do_set4(input); break;  /* silly */
				case set_rule: do_set_rule(input); break;
				case put1: do_put1(input); break ;
				case put2: do_put2(input); break;	/* silly */
				case put3: do_put3(input); break;	/* silly */
				case put4: do_put4(input); break;	/* silly */
				case put_rule: do_put_rule(input); break;	
				case nop: break;				/* easy, do nothing ! */
				case bop: do_bop(input); break;
				case eop: do_eop(input); break;
				case push: do_push(); break;
				case pop: do_pop(); break;
				case right1: do_right1(input); break;
				case right2: do_right2(input); break;  
				case right3: do_right3(input); break; 
				case right4: do_right4(input); break; 
				case w0: do_w0(); break;
				case w1: do_w1(input); break;
				case w2: do_w2(input); break; 
				case w3: do_w3(input); break; 
				case w4: do_w4(input); break;		/* not used ? */
				case x0: do_x0(); break;
				case x1: do_x1(input); break;
				case x2: do_x2(input); break; 
				case x3: do_x3(input); break; 
				case x4: do_x4(input); break;		/* not used ? */
				case down1: do_down1(input); break;
 				case down2: do_down2(input); break; 
				case down3: do_down3(input); break; 
				case down4: do_down4(input); break; 
/*				case y0: do_y0(); break; */		/* conflict math.h */
				case y5: do_y0(); break;
/*				case y1: do_y1(input); break; *//* conflict math.h */
				case y6: do_y1(input); break;
				case y2: do_y2(input); break; 
				case y3: do_y3(input); break; 
				case y4: do_y4(input); break;		/* not used ? */
				case z0: do_z0(); break;
				case z1: do_z1(input); break;
				case z2: do_z2(input); break; 
				case z3: do_z3(input); break; 
				case z4: do_z4(input); break;		/* not used ? */
				case fnt1: do_fnt1(hDC, input); break;
				case fnt2: do_fnt2(hDC, input); break;	/* silly */
				case fnt3: do_fnt3(hDC, input); break;	/* silly */
				case fnt4: do_fnt4(hDC, input); break;	/* silly */
				case xxx1: do_xxx1(input); break;
				case xxx2: do_xxx2(input); break;	/* not used ? */
				case xxx3: do_xxx3(input); break;	/* not used ? */
				case xxx4: do_xxx4(input); break; 
				case fnt_def1: do_fnt_def1(input); break;
				case fnt_def2: do_fnt_def2(input); break; /* silly */
				case fnt_def3: do_fnt_def3(input); break; /* silly */
				case fnt_def4: do_fnt_def4(input); break; /* silly */
				case post: do_post(input); break;
				case pre: do_pre(input); break;
				case post_post: do_post_post(input); break;
	
				default: {
/* we already complained about this in dvipslog ... */
					finish = -1;	/* ??? */
/* this should normally not happen: */
					sprintf(debugstr, "Unknown DVI command (%d)", c);
					wincancel(debugstr);
					errcount();
					break;
				} 
/*				break; */
			}
		}
 		if (bMarkSpotFlag > 0 && bShowFlag == 0) {
/*		if (bMarkSpotFlag > 0) {	 */
			if (filepos >= lastsearch) {	/* mark this spot and get out */
				dvi_spot_h = dvi_h, dvi_spot_v = dvi_v;
				bMarkSpotFlag = 0;		/* -1 */
				finish = -1;
				if (bDebug > 1) {
					sprintf(debugstr, "SPOT %ld %ld", dvi_spot_h, dvi_spot_v);
					OutputDebugString(debugstr);
				}
			}
		}
		if (finish != 0) break;
	}

	bReverseVideo = 0;
	OldFore = -1; OldBack = -1;

/*	if (hRuleBrush != NULL && hRuleBrush != hBlackBrush) {*//* some cleanup */
	if (hRuleBrush != NULL && hRuleBrush != hRuleBrushDefault) {
		if ((UINT) hRuleBrush > 1) {
			DeleteObject(hRuleBrush);
			hRuleBrush = (HBRUSH) -1;
		}
	}
/*	hRuleBrush = hBlackBrush; */
	hRuleBrush = hRuleBrushDefault;

	if ((UINT) hOldPen > 1)			/* avoid Windows 3.0 MetaFile problem */
/*	if ((UINT) hRulePenDefault > 1) *//* avoid Windows MetaFile problem ? */
/*		hOldPen = SelectObject(hdc, hBlackPen); */
		hOldPen = SelectObject(hdc, hRulePenDefault);
/*	if (bDebug > 1) {
		sprintf(debugstr, "hOldPen %0X hRulePenDefault %0X\n",
				hOldPen, hRulePenDefault);
		OutputDebugString(debugstr);
	} */
/*	Following gives an invalid HGDIOBJ in WIN16 when SlowPlayMetaFile */
/*	i.e. with playing MetaFile into MetaFile context */
/*	--- could avoid by testing bCopyFlag == 0 */
/*  But what causes it? mismatch SaveDC/RestoreDC ? WIN16 bug ? */
	if (hOldPen != hRulePenDefault) {
		if ((UINT) hOldPen > 1) {
			DeleteObject(hOldPen);
			hOldPen = (HPEN) -1;
		}
	}
/*	hRulePen = hBlackPen; */
	hRulePen = hRulePenDefault; 

	if (bATMReencoded) UnencodeFont (hdc, nfont, 0);		/* 1994/Dec/25 */
	if ((UINT) hFontOld > 1)	/* avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hDC, hFontOld);		/* deselect any font used */
	if (hdc == NULL)
		wincancel("Null hdc (end scandvipage)"); /* debugging only */
	hdc = NULL;							/* to avoid inadvertent reuse ? */
/*	if (bATMReencoded) UnencodeFont (hdc, nfont, 0); */		/* 1994/Dec/25 */
	return page_seen;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef DEBUGWIDTH

/* In WIN32 Escape GETTEXTEXTENTTABLE fails (both NT and 95) */
/* In WIN95 GetCharWidth(...) fails (but works in NT) */
/* In WIN95 ATM font returns wrong widths unless ScreenAdjust=Off */

/* In WIN16 all three methods work !!! */

/* In WIN32 Windows NT: Escape GETEXTENTTABLE fails (and leaves junk) */
/* Both GetCharWidth(...) & GetTextExtentPoint(...) do work */
/* In Windows 95 it seems GETEXTENTTABLE fails and GetCharWidth(...) fails */
/* While GetTextExtentPoint works for TrueType */

int debugdone=0;								/* 1996/Jan/1 */
   
void DebugWidthCode (HDC hDC) {		/* can only be called if bDebug > 1 */
	HFILE output;
	int k, flag;
	char filename[MAXFILENAME];
	char chars[2];
#ifdef USEUNICODE
	WCHAR charsW[2];
#endif
	CHAR_RANGE_STRUCT CharRange;
	DWORD err=0;
	SIZE TextSize;
	char *s;
/*	static short shortwidths[MAXCHRS]; */	/* temporary home for widths */
	static int charwidths[MAXCHRS];			/* temporary home for widths */

	if (debugdone++ > 0) return;			/* do this only once */
/*	winerror("LOG started"); */
/*	output = _lopen("metrics.log", WRITE); */
/*	output = _lcreat("e:\\metrics.log", 0); */
	strcpy(filename, "e:\\metrics.log");
	output = _lcreat(filename, 0);
	if (output == HFILE_ERROR) {
/*		output = _lcreat("c:\\users\\default\\metrics.log", 0); */
		strcpy(filename, "c:\\users\\default\\metrics.log");
		output = _lcreat(filename, 0);
		if (output == HFILE_ERROR) {
/*		output = _lcreat("c:\\users\\default\\metrics.log", 0); */
			strcpy(filename, "d:\\temp\\metrics.log");
			output = _lcreat(filename, 0);
			if (output == HFILE_ERROR) {
				winerror ("DebugWIdthCode: Can't open LOG");
				return;
			}
		}
	}
/*	sprintf(str, "LOG %s started", filename);	winerror(str); */
/*	winerror("LOG opened"); */

/*	for (k = 0; k < MAXCHRS; k++) shortwidths[k] = 0; */
/*	winerror("shortwidths cleared"); */
	for (k = 0; k < MAXCHRS; k++) charwidths[k] = 0; 
/*	winerror("charwidths cleared"); */
	sprintf(str, "bATMReencoded %d bFontEncoded %d\n", bATMReencoded, bFontEncoded);
	flag = _lwrite(output, str, strlen(str));
	sprintf(str, "bUseCharSpacing %d bCheckReencode %d\n", bUseCharSpacing, bCheckReencode);
	flag = _lwrite(output, str, strlen(str));

	CharRange.chFirst = (BYTE) 0;
	CharRange.chLast = (BYTE) (MAXCHRS-1);

	flag = ExtEscape(hDC, GETEXTENTTABLE, sizeof(CHAR_RANGE_STRUCT),
/*			  (LPSTR) &CharRange, sizeof(shortwidths), (LPSTR) &shortwidths);*/
			  (LPSTR) &CharRange, sizeof(charwidths), (LPSTR) &charwidths);

/*	flag = Escape(hDC, GETEXTENTTABLE, sizeof(CHAR_RANGE_STRUCT),
				  (LPSTR) &CharRange, (LPSTR) &charwidths); */

	if (flag <= 0) {
		sprintf(str, "GETEXTENTTABLE Escape failed %d (expected in NT)\n", flag);
		winerror(str);
		flag = _lwrite(output, str, strlen(str));
		if (flag == HFILE_ERROR) return;
	}
/*	if (flag > 0) */
	else {
		sprintf(str, "Comment GETEXTENTTABLE\n");
		flag = _lwrite(output, str, strlen(str));
		for (k = 0; k < MAXCHRS; k++) {
/*			if (shortwidths[k] == 0) continue; */
			if (charwidths[k] == 0) continue;
/*			sprintf(str, "C %d ; WX %d ; \n", k, shortwidths[k]); */
			sprintf(str, "C %d ; WX %d ; \n", k, charwidths[k]);
			flag = _lwrite(output, str, strlen(str));
			if (flag == HFILE_ERROR) return;
		}
	}
	for (k = 0; k < MAXCHRS; k++) charwidths[k] = 0; 

	flag = GetCharWidth32(hDC, 0, MAXCHRS-1, charwidths);

/*	flag = GetCharWidth(hDC, 0, MAXCHRS-1, charwidths); */

	if (flag == 0) {
		sprintf(str, "GetCharWidth failed %d\n", flag);
		winerror(str);
		flag = _lwrite(output, str, strlen(str));
		if (flag == HFILE_ERROR) return;

		err = GetLastError();
		s = str + strlen(str);
		sprintf(s, "GetLastError %lu (%0X)\n", err, err);
		ExplainError(str, err, 1);

/*		winerror(str); */

		flag = _lwrite(output, str, strlen(str));
		if (flag == HFILE_ERROR) return;
	}
/*	if (flag != 0) */
	else {
		sprintf(str, "Comment GetCharWidth\n");
		flag = _lwrite(output, str, strlen(str));
		for (k = 0; k < MAXCHRS; k++) {
			if (charwidths[k] == 0) continue;
			sprintf(str, "C %d ; WX %d ; \n", k, charwidths[k]);
			flag = _lwrite(output, str, strlen(str));
			if (flag == HFILE_ERROR) return;
		}
	} 

/*	Following works for TrueType fonts */
	{
		ABC abc[MAXCHRS];	/* 256 * (sizeof(int)+sizeof(int)+sizeof(int)) */
		flag = GetCharABCWidths (hDC, 0, MAXCHRS-1, abc);
		if (flag == 0) {
			sprintf(str, "GetCharABCWidths failed %d\n", flag);
			winerror(str);
			flag = _lwrite(output, str, strlen(str));
			if (flag == HFILE_ERROR) return;		
		}
		else {
			sprintf(str, "Comment GetCharABCWidths\n");
			flag = _lwrite(output, str, strlen(str));
			for (k = 0; k < MAXCHRS; k++) {
				sprintf(str, "C %d ; WX %d ; A %d B %u C %d\n", k,
					abc[k].abcA + abc[k].abcB + abc[k].abcC,
					abc[k].abcA, abc[k].abcB, abc[k].abcC);
			flag = _lwrite(output, str, strlen(str));
		}
	}
	}

	for (k = 0; k < MAXCHRS; k++) charwidths[k] = 0; 
/*	now for the slow way ... */
	sprintf(str, "Comment GetTextExtentPoint\n");
	flag = _lwrite(output, str, strlen(str));
	for (k = 0; k < MAXCHRS; k++) {
		*chars = (char) k;					/* 1995/Nov/5 */

#ifdef USEUNICODE
/*		if (bUseNewEncode && bFontEncoded) */
		if (bFontEncoded) {
			*charsW = encodeUID[k];
			flag = GetTextExtentPoint32W(hDC, charsW, 1, &TextSize);
		}
		else flag = GetTextExtentPoint32A(hDC, chars, 1, &TextSize);
#else
		flag = GetTextExtentPoint32(hDC, chars, 1, &TextSize);
#endif

/*		flag = GetTextExtentPoint(hDC, chars, 1, &TextSize); */

		if (flag == 0) continue;
		else {
/*			if (TextSize.cx == 0) continue; */
			sprintf(str, "C %d ; WX %d ; \n", k, TextSize.cx);
			flag = _lwrite(output, str, strlen(str));
			if (flag == HFILE_ERROR) return;
		}
	}
	_lclose (output);
/*	winerror("LOG completed"); */
	sprintf(str, "LOG %s completed", filename);	winerror(str); 
}
#endif

/*************************************************************************/

/* replace %d with line number and %s with file name and %; with ; */
/* if no working directory in use, prepend file name with DVI dir */
/* %l for log file name ??? 98/Nov/11 */

int setupeditstring(char *str, int nLine, char *sFile, char *szEditPattern) {
	char *s=szEditPattern, *t=str;
	char *u;
	int c;

	while ((c = *s++) >= ' ') {
		if (c == '%') {					/* check for escape character */
			c = *s++;					/* what comes after % ? */
			if (c == 'd') {
				sprintf(t, "%d", nLine);	/* line number */
				t += strlen(t);
			}
			else if (c == 's' || c == 'l' || c == 'a') {
/*				*t++ = '\"'; */			/* in case name has spaces */
				if (bUseSourcePath) {	/* when not using working directory */
					strcpy(t, DefPath);
					t += strlen(t);
				}						/* need separator here ? */
				strcpy(t, sFile);
				if (c == 'l') {			/* log file rather than source ? */
					if ((u = strrchr(t, '.')) != NULL)
						strcpy(u, ".log");
				}
				else if (c == 'a') {	/* aux file rather than source ? */
					if ((u = strrchr(t, '.')) != NULL)
						strcpy(u, ".aux");
				}
				t += strlen(t);
/*				*t++ = '\"'; */			/* in case name has spaces */
			}
			else if (c == '\0') break;	/* % followed by null */
			else *t++ = (char) c;		/* just quote next character */
		}
		else *t++ = (char) c;			/* normal character */
	}
	*t++ = '\0';						/* terminate */
	return (t - str);
}

/* DDE client support in DVIWindo */

HINSTANCE StartEditor (int nLine, char *sFile, char *szTeXEdit) {
	int flag;
	HINSTANCE err;
	int nCmdShow = SW_NORMAL;
	
	if (szTeXEdit == NULL) return 0;		/* sanity check */
	if (*szTeXEdit == '\0') return 0;		/* sanity check */
	setupeditstring(str, nLine, sFile, szTeXEdit);
#ifdef DEBUGDDE
	if (bDebug > 1) OutputDebugString(str);
#endif
	if (bDebug || bShowCalls) {
		flag = MaybeShowStr (str, "Editor");
		if (flag == 0) return 0;	/* user cancelled */
	}
//	WritePrivateProfileString(achPr, "LastEditor", str, achFile);	
	WritePrivateProfileString(achDiag, "LastEditor", str, achFile);	
	err = (HINSTANCE) WinExec(str, nCmdShow);
	if (err < HINSTANCE_ERROR) {
		sprintf(debugstr, "Unable to launch %s", str);
		winerror(debugstr);
	}
	return err;
}

void DdeFailureReport (DWORD idInst, char *szCall) {
	int errcode = DdeGetLastError(idInst);
	sprintf(debugstr, "%s failed %04X", szCall, errcode);
#ifdef DEBUGDDE
	if (bDebug > 1) ExplainError(debugstr, errcode, 0);
#else
	if (bDebug > 1) OutputDebugString(debugstr);
#endif
}

/* time in milli-seconds to wait for DDE client transaction */
/* note that the last one is treated as asynchronous (good?) */

#define DDE_TIME 2000

/*	New code DDE */ /* Sample DDE server name: Epsilon, topic name: Open */
/*	strings limited to 255 bytes */

/*	EditorDDE=Application;Topic;+% %s;... */
/*	semicolons have been replaced by null - two nulls at the very end */

int CallEditor (int nLine, char *sFile, char *szEditorDDE, char *szTeXEdit) {
	char *szApplication, *szTopic, *szEditPattern;
	HSZ hszClientAppName=NULL, hszClientTopic=NULL;	/* Service and Topic */
	HCONV hConv;
	DWORD idClientInst, dwResult, dwTimeout;
	HDDEDATA hddret;
	int errcode;
	int flag = 0;				/* normal return value */
	char *s;

	if (szEditorDDE == NULL) return -1;
	if (*szEditorDDE == '\0') return -1;	

	if (bDdeClientBusy > 0) return -1;

	s = szEditorDDE;
	while (*s > 0 && *s <= ' ') s++;
	szApplication = s;
	if (*szApplication == '\0') return -1;	/* sanity check */
	s += strlen(s) + 1;
	while (*s > 0 && *s <= ' ') s++;
	szTopic = s;
	if (*szTopic == '\0') return -1;		/* sanity check */
	s += strlen(s) + 1;
	while (*s > 0 && *s <= ' ') s++;
	szEditPattern = s;
	if (*szEditPattern == '\0') return -1;	/* sanity check */

#ifdef DEBUGDDE
	if (bDebug > 1) {
		sprintf(debugstr, "|%s| |%s| |%s|",
				szApplication, szTopic, szEditPattern);
		OutputDebugString(debugstr);
	}
#endif

	idClientInst = 0;
	errcode = DdeInitialize(&idClientInst,
				 NULL,						/* pfnCallback */
				 APPCLASS_STANDARD |
				 APPCMD_CLIENTONLY |
				 CBF_FAIL_ALLSVRXACTIONS |
				 CBF_SKIP_ALLNOTIFICATIONS, /* afCmd filters */
				 0L);						/* ulRes must be zero */
	if (errcode != DMLERR_NO_ERROR || idClientInst == 0) {	/* failed ? */
#ifdef DEBUGDDE
/*		DMLERR_DLL_USAGE, DMLERR_INVALIDPARAMETER, DMLERR_SYS_ERROR */
		if (bDebug > 1) {
			sprintf(debugstr, "DdeInitialize failed %04X", errcode);
			OutputDebugString(debugstr);
		}
#endif
		return -1;	/* safe to just quit here - noting to release */
	}

	if (bDdeClientBusy++ > 0) return -1;
					/* set only after all return -1 instances */

/*	CP_WINANSI or CP_WINUNICODE depending on flavour of DdeInitialize */
/*	hence dependent on whether UNICODE is defined or not */
	hszClientAppName = DdeCreateStringHandle(idClientInst, szApplication, CP_WINNEUTRAL);
	hszClientTopic = DdeCreateStringHandle(idClientInst, szTopic, CP_WINNEUTRAL);
	if (hszClientAppName == NULL || hszClientTopic == NULL) {
#ifdef DEBUGDDE
		DdeFailureReport(idClientInst, "DdeCreateStringHandle");
#endif
		flag = -1;
/*		return -1; */
	}
	else {	/* now lets try and connect */
		hConv = DdeConnect(idClientInst, hszClientAppName, hszClientTopic, NULL);
		if (hConv == NULL) {
#ifdef DEBUGDDE
			DdeFailureReport(idClientInst, "DdeConnect");
#endif
/*			sprintf(debugstr, "Failed to connect to %s (%s)",
				szApplication, szTopic); */
/*			winerror(debugstr); */
			flag = -1;		/* failed */ /* Use WinExe instead on TeXEdit */
/*			if (*szTeXEdit != '\0') */
			if (szTeXEdit != NULL)
				StartEditor(nLine, sFile, szTeXEdit);
		}	
		else if (hConv != NULL) {
/*			Note: there may be more than one call pattern to fill */
			while (*szEditPattern != '\0') {
				setupeditstring(str, nLine, sFile, szEditPattern);
#ifdef DEBUGDDE
				if (bDebug > 1) OutputDebugString(str);
#endif
				szEditPattern += strlen(szEditPattern)+1; /* next */
				if (*(szEditPattern) == '\0')	/* last string ? */
					dwTimeout = TIMEOUT_ASYNC;	/* asynchronous */
				else dwTimeout = DDE_TIME;		/* in milliseconds */
				hddret = DdeClientTransaction((unsigned char *) str,	/* lpbData */
								 strlen(str) + 1,	/* cbDataLen */
								 hConv,			/* hConv */
								 NULL,			/* hszItem */
								 CF_TEXT,		/* wFmt */
								 XTYP_EXECUTE,	/* wType 0 ? */
								 dwTimeout,		/* dwTimeout ??? */
								 &dwResult);
				if (hddret == 0) {				/* execution failed */
#ifdef DEBUGDDE
					DdeFailureReport(idClientInst, "DdeClientTransaction");
#endif
					break; 		/* ignore remaining patterns ? */
				}
/*				szEditPattern += strlen(szEditPattern)+1; */
			}	/* end of loop over strings */
			if (DdeDisconnect(hConv) == 0) {
#ifdef DEBUGDDE
				DdeFailureReport(idClientInst, "DdeDisconnect");
#endif
			}
		}
	} /* end of string handles obtained successfully */

	if (hszClientTopic != NULL) {
		if (DdeFreeStringHandle(idClientInst, hszClientTopic) == 0) { 
#ifdef DEBUGDDE
			DdeFailureReport(idClientInst, "DdeFreeStringHandle");
#endif
		}
		hszClientTopic = NULL;
	}
	if (hszClientAppName != NULL) {
		if (DdeFreeStringHandle(idClientInst, hszClientAppName) == 0) {
#ifdef DEBUGDDE
			DdeFailureReport(idClientInst, "DdeFreeStringHandle");
#endif
		}
		hszClientAppName = NULL;
	}
	if (DdeUninitialize(idClientInst) == 0) {
/*		failed to free DDEML library resources */
#ifdef DEBUGDDE
		DdeFailureReport(idClientInst, "DdeUninitialize");
#endif

	}
	bDdeClientBusy = 0;					/* reset so can use again */
/*	should we set the Focus here to this window ? */
	return flag;		
}

/*************************************************************************/

/* DDE server support in DVIWindo */

HSZ hszServerAppName=NULL, hszServerTopic=NULL;	/* Service and Topic */

int bDdeServerBusy=0;			/* avoid reentrancy */

HCONV hConversation=0;			/* current conversation (info only for now) */

/* [<cmd>;<cmd>;<cmd>] where <cmd> is Open(...) Source(...) Line(...) */

char DDEcmdstr[FILENAME_MAX+FILENAME_MAX];	/* static for PostMessage */

/* Split up command string and PostMessage to DVIWindo */
/* Make sure not to use pointers in calls */

int PostCommand (char *cmd) {
	char *s, *t;
	DWORD nLine, nPage;
	
	if (bDebug > 1) OutputDebugString(cmd);
	if (cmd == NULL) return -1;
	if ((s = strchr(cmd, '(')) == NULL) return -1;
	*s++ = '\0';		/* terminate command */
	t = s + strlen(s);
	if (*(t-1) == ')') *(t-1) = '\0';
	if (_stricmp(cmd, "Open") == 0) {
		if (bDebug > 1) {
			sprintf(debugstr, "PostMessage %s", s);
			OutputDebugString(debugstr);
		}
		PostMessage(hwnd, WM_COMMAND, IDM_OPEN, (LPARAM) s);
	}
	else if (_stricmp(cmd, "Source") == 0) {
		PostMessage(hwnd, WM_COMMAND, IDM_SOURCEFILE, (LPARAM) s);
	}
	else if (_stricmp(cmd, "Line") == 0) {
		if (sscanf(s, "%ld", &nLine) == 1) {
			PostMessage(hwnd, WM_COMMAND, IDM_SOURCELINE, (LPARAM) nLine);
		}
	}
	else if (_stricmp(cmd, "Page") == 0) {
		if (sscanf(s, "%ld", &nPage) == 1) {
			PostMessage(hwnd, WM_COMMAND, IDM_SELECTPAGE, (LPARAM) nPage);
		}
	}
/*	should we set the Focus here to this window ? */
	return 0;
}

int ProcessCommandString(char *cmds) {
	int flag = 0;
	char *s;
	if (cmds == NULL) return -1;
	if (strlen(cmds) < 2) return -1;
	if (*cmds == '[') cmds++;	/* ignore leading '[' */
	s = cmds + strlen(cmds);
	if (*(s-1) == ']') *(s-1) = '\0';	/* flush trailing ']' */
	while ((s = strchr(cmds, ';')) != NULL) {
		*s = '\0';
		PostCommand(cmds);
		cmds = s+1;
		while (*s == ' ') s++;
	}
	PostCommand(cmds);					/* last one */
	if (bDebug > 1) OutputDebugString("End ProcessCommandString");
	return flag;
}

#ifdef DEBUGDDE
char *ClassName (UINT nClass) {
	if (nClass == XCLASS_BOOL) return( "BOOL");
	else if (nClass == XCLASS_DATA) return( "DATA");
	else if (nClass == XCLASS_FLAGS) return( "FLAGS");
	else if (nClass == XCLASS_NOTIFICATION) return( "NOTIFICATION");
	else return( "UNKNOWN");
}

char *TypeName (UINT uType) {
	switch(uType) {
		case(XTYP_ERROR): return "ERROR";
		case XTYP_ADVDATA: return "ADVDATA";
		case XTYP_ADVREQ: return "ADVREQ";
		case XTYP_ADVSTART:	return "ADVSTART";
		case XTYP_ADVSTOP: return "ADVSTOP";
		case XTYP_EXECUTE: return "EXECUTE";
		case XTYP_CONNECT: return "CONNECT";
		case XTYP_CONNECT_CONFIRM: return "CONNECT_CONFIRM";
		case XTYP_XACT_COMPLETE: return "XACT_COMPLETE";
		case XTYP_POKE: return "POKE";
		case XTYP_REGISTER:	return "REGISTER";
		case XTYP_REQUEST: return "REQUEST";
		case XTYP_DISCONNECT: return "DISCONNECT";
		case XTYP_UNREGISTER: return "UNREGISTER";
		case XTYP_WILDCONNECT: return "WILDCONNECT";
		default: return "UNKNOWN";
	}
}
#endif

/* HDDEDATA CALLBACK DdeCallback( */
HDDEDATA CALLBACK _export DdeCallback(
							  UINT uType,     // transaction type
							  UINT uFmt,      // clipboard data format
							  HCONV hConv,    // handle to the conversation
							  HSZ hsz1,       // handle to a string
							  HSZ hsz2,       // handle to a string
							  HDDEDATA hData, // handle to a global memory object
							  DWORD dwData1,  // transaction-specific data
							  DWORD dwData2   // transaction-specific data
							 ) {
	DWORD nClass;
	int nLen;

	if (bDdeServerBusy++ > 0) return 0;	/* shouldn't happen ? */
/*		or return CBR_BLOCK and then DdeEnableCallback later */
	nClass = (uType & XCLASS_MASK);		/* Class of DDE call */
#ifdef DEBUGDDE
	if (bDebug > 1) {
		char localstr[FILENAME_MAX];
		sprintf(localstr, "%s %s: uType %04X uFmt %X hConv %X",
				ClassName(nClass), TypeName(uType), uFmt, hConv);
		OutputDebugString(localstr);
		nLen = DdeQueryString(idServerInst, hsz1, localstr, sizeof(localstr), CP_WINNEUTRAL);
		if (nLen > 1) OutputDebugString(localstr);
		nLen = DdeQueryString(idServerInst, hsz2, localstr, sizeof(localstr), CP_WINNEUTRAL);
		if (nLen > 1) OutputDebugString(localstr);
		if (hData != 0 || dwData1 != 0 || dwData2 != 0) {
			sprintf(localstr, "hData %X dwData1 %lu dwData2 %lu",
				hData, dwData1, dwData2);
			OutputDebugString(localstr);
		}
	}
#endif
	if (nClass == XCLASS_NOTIFICATION) {
/* XTYP_REGISTER, XTYP_UNREGISTER, XTYP_CONNECT_CONFIRM, XTYP_DISCONNECT */
/* hconv handle to new conversation in CONNECT_CONFIRM & DISCONNECT */
		if (uType == XTYP_CONNECT_CONFIRM)
			hConversation = hConv;			/* remember it (info only)*/
		else if (uType == XTYP_DISCONNECT)
			hConversation = 0;				/* forget it */
		bDdeServerBusy = 0;
		return 0;						/* return value ignored */
	}
	else if (nClass == XCLASS_FLAGS) {
/* 	XTYP_EXECUTE, XTYP_POKE => DDE_FACK, DDE_FBUSY, DDE_FNOTPROCESSED */
/*  hsz1 is Topic --- hsz2 is Application */
		if (uType == XTYP_EXECUTE) {
			if (DdeCmpStringHandles(hsz1, hszServerTopic) == 0) {
/* hData is handle to the command string */
				nLen = DdeGetData(hData, (unsigned char *) DDEcmdstr, sizeof(DDEcmdstr), 0);
				if (nLen > 1) OutputDebugString(DDEcmdstr);
				if (nLen > 1) ProcessCommandString(DDEcmdstr);
/*				DdeFreeDataHandle(hData); */ /* NO */
				bDdeServerBusy = 0;
				return (HDDEDATA) DDE_FACK;
			}
		}
		bDdeServerBusy = 0;
		return DDE_FNOTPROCESSED;	/* 0 */
	}
	else if (nClass == XCLASS_BOOL) {
/*	XTYP_CONNECT */	/* return value TRUE or FALSE */
/*  hsz1 is Topic --- hsz2 is Application */
		if (uType == XTYP_CONNECT) {
			if (DdeCmpStringHandles(hsz1, hszServerTopic) == 0) {
				bDdeServerBusy = 0;
				return (HDDEDATA) TRUE;		/* OK */
			}
		}
		bDdeServerBusy = 0;
		return FALSE;				/* 0 */
	}
	else return 0;
};

/* hsz1 topic */ /* hsz2 service/application */

/* Service: DVIWindo, Topic: SRCSpecial */

int DDEServerStart (void) {
	int errcode;
/*	char *szApplication = "DVIWindo"; */
/*	char *szTopic = "SRCSpecial"; */

/*	bDebug = 2; */		/* TEMPORARY DEBUGGING ???? */

	idServerInst = 0;
	errcode = DdeInitialize(&idServerInst,
							DdeCallback,
							APPCLASS_STANDARD |
/*							CBF_FAIL_ADVISES | */
/*							CBF_FAIL_POKES | */
/*							CBF_FAIL_REQUESTS */
							CBF_SKIP_REGISTRATIONS 
/*							CBF_SKIP_CONNECT_CONFIRMS */
/*							CBF_SKIP_DISCONNECT */
/*							CBF_SKIP_ALLNOTIFICATIONS */ /* debugging ONLY ???? */
							, /* afCmd filters */
							0L);						/* ulRes must be zero */
	if (errcode != DMLERR_NO_ERROR || idServerInst == 0) {	/* failed ? */
#ifdef DEBUGDDE
/*		DMLERR_DLL_USAGE, DMLERR_INVALIDPARAMETER, DMLERR_SYS_ERROR */
		if (bDebug > 1) {
			sprintf(debugstr, "DdeInitialize failed %04X", errcode);
			OutputDebugString(debugstr);
		}
#endif
		idServerInst = 0;
		return -1;	/* safe to just quit here - noting to release */
	}

/*	CP_WINANSI or CP_WINUNICODE depending on flavour of DdeInitialize */
/*	hence dependent on whether UNICODE is defined or not */
	hszServerAppName = DdeCreateStringHandle(idServerInst, szApplication, CP_WINNEUTRAL);
	hszServerTopic = DdeCreateStringHandle(idServerInst, szTopic, CP_WINNEUTRAL);
	if (hszServerAppName == NULL || hszServerTopic == NULL) {
#ifdef DEBUGDDE
		DdeFailureReport(idServerInst, "DdeCreateStringHandle");
#endif
/*		return ? need to DdeUninitialize in that case */
	}

	errcode = (int) DdeNameService(
							idServerInst,  // instance identifier
							hszServerAppName,    // handle to service name string
							0L,   // reserved
							DNS_REGISTER     // service name flags
/*							DNS_FILTEROFF */ /* DEBUGGING TEST ???? */
						   );
	if (errcode == 0) {	/* failed */
		DdeFailureReport(idServerInst, "DdeNameService");
	}
	if (bDebug > 1) OutputDebugString("DDEServerStart");
	bDdeServerBusy = 0;
	return 1;
}

int DDEServerStop (void) {
	int errcode;
	
	if (idServerInst == 0) return 0;

	errcode = (int) DdeNameService(
							 idServerInst,  // instance identifier
							 0L,      // handle to service name string
							 0L,      // reserved
							 DNS_UNREGISTER    // service name flags
							);

	if (errcode == 0) {	/* failed */
		DdeFailureReport(idServerInst, "DdeNameService");
	}

	if (hszServerTopic != NULL) {
		if (DdeFreeStringHandle(idServerInst, hszServerTopic) == 0) { 
#ifdef DEBUGDDE
			DdeFailureReport(idServerInst, "DdeFreeStringHandle");
#endif
		}
		hszServerTopic = NULL;
	}
	if (hszServerAppName != NULL) {
		if (DdeFreeStringHandle(idServerInst, hszServerAppName) == 0) {
#ifdef DEBUGDDE
			DdeFailureReport(idServerInst, "DdeFreeStringHandle");
#endif
		}
		hszServerAppName = NULL;
	}
	if (DdeUninitialize(idServerInst) == 0) {
/*		failed to free DDEML library resources */
#ifdef DEBUGDDE
		DdeFailureReport(idServerInst, "DdeUninitialize");
#endif
	}
	idServerInst = 0;
	if (bDebug > 1) OutputDebugString("DDEServerStop");
	return 1;
}

/*************************************************************************/

/* WARNING: very expensive to do systems calls one character at a time ! */

#ifdef IGNORED

/* come here either with character code 0 - 127 OR from set1 */
/* this version does it one character at a time using DVITYPE algorithm */
/* NOT USED ANYMORE ??? */  /* HAS NOT BEEN KEPT UP TO DATE */

void normalcharnew (HFILE input, int c) {
/*	int refx, refy, devx, devy; */
	int dx, dy;
	long ldx;
/*	DWORD current; */
	DWORD escapement;
	RECT TextRect; 					/* rectangle occupied by text */
/*	RECT InterRect; */

/*	don't need character buffer for this version */

	if (c == (int) set1) c = an_wingetc(input);		/* get next character */
	if (bSkipFlag == 0) {
		if (fnt_exists != 0) {	 /* draw ONLY if font found by ATM */
			if (cp_valid == 0)	 /* if current point not valid */
				(void) MoveToEx(hdc, dvi_hh, dvi_vv, NULL);	/* move to current pt */
/*	need more shifting below ? to deal with large atsizes CMINCH ? */
/*  compute AccurateExtent */
/*			ldx = (((long) lpCharWidths[c]) * atsize + 62) / 1000; */

			ldx = MulDiv((long) lpCharWidths[c], atsize, 1000);

/*			ldx = (((long) lpCharWidths[c]) * (atsize >> 3) + 62) / 125; */

/* above should be safe from overflow since its for single character */
/* should we check for encoding problems as in normalcharold ? */
			if (fnt_texatm != 0) {
				if (c <= 32 || c == 127)		/* 95/June/23 */
					c = remaptable[c];			/* remapped 0 -- 31 font */
			}
			else if (fnt_ansi) {
/*				if (c < 32 && c >= 16) c = ansitex[c + 16]; */  /* ??? */
				if (c < 32)	c = ansitex[c];		/* 95/June/11 */
			}
			str[0] = (char) c;
			if (bCopyFlag == 0) {
/* possibly use GetTextExtentPoint32 in WIN32 ? */

				(void) GetTextExtentPoint32(hdc, str, 1, &TextSize);

/*				(void) GetTextExtentPoint(hdc, str, 1, &TextSize); */

				dx = TextSize.cx;
				dy = TextSize.cy;
/* pre Windows 0x030a version */
/*				escapement = GetTextExtent(hdc, str, 1); */ /* get rectangle */ 
/*				dx = (int) LOWORD(escapement); */
/*				dy = (int) HIWORD(escapement); */
			}
			else {
				ldx = AccurateExtent(str, 1);
				dx = mapd(ldx);
/*				dy = ascent;	*/		/* FIX THIS CROCK */
				dy = ascent + descent;	/* ??? */
			}			
/*			if (dx < 0 || dy < 0) {
				sprintf(str, "(C) dx %d dy %d", dx, dy);
				wincancel(str);
			}	*/
/*			if (tracecount++ < 32) {
				sprintf(str + 1, " c %d dx %d dy %d", c, dx, dy);
				wincancel(str);
			} */

/*	or use a second table of font metrics ? */
			if ((bShowFlag != 0 && bTextOutFlag != 0) ||
				(bShowFlag == 0 && bTagFlag != 0)) { /* new */
				if (bDrawVisible == 0) { /* we don't use this anymore */
/*					if (TextOut(hdc, 0, 0, str, 1) == 0) cp_valid = 0; */
/* not fixed up since not used anymore */
					if (ExtTextOut(hdc, 0, 0, 0, NULL, 
						str, 1, NULL) == 0) cp_valid = 0;
				}
/*				else if (bCopyFlag != 0) {
					if (ExtTextOut(hdc, 0, 0, 0, NULL,
						str, 1, NULL) == 0) cp_valid = 0;
				} */
				else {
					TextRect.left = dvi_hh; TextRect.right = dvi_hh + dx;
/*					TextRect.bottom = dvi_vv; */
					TextRect.top = dvi_vv + dy - descent; /* ??? */
/* try and correct for descender (reference is baseline here) */
/* this maybe uses the wrong units (twips instead of device units) ? */
/*					TextRect.bottom = TextRect.bottom - descent; */
					TextRect.bottom = dvi_vv - descent; 
/*	Deal Windows 95 bug with very narrow boxes in RectVisible */
/*	--- somewhat kludgy fix ... 1995/Aug/26 */
/*					if (bWin95) */
					if (bFixZeroWidth) { 
						if (bShowflag) { /* if bTagFlag == 0 */
/*							if (dx < dy) TextRect.right = TextRect.left + dy;*/
							if (dx < MinWidth)
								TextRect.right = TextRect.left + MinWidth;
						}
 					} 
/* we don't use this anymore ... so don't bother to maintain this code */
					if (bTagFlag != 0) { /* bShowflag == 0 */
						if (TextRect.left <= tagx && TextRect.right	>= tagx &&
							TextRect.bottom <= tagy && TextRect.top >= tagy) {
								srctaghit = 1;
								taggedfont = dvi_f;
								finish = -1;
						}
					} 
					if (bShowFlag != 0) { /* bTagFlag == 0 */
/*	Avoid RectVisible() in MetaFile - use InterSectRect instead */
						if ((bCopyFlag == 0 && RectVisible(hdc, &TextRect) != 0) ||
							(bCopyFlag != 0 && InterSectRect(&CopyClipRect, 
								&TextRect) != 0)) {
							if (bGreyText != 0 || bGreyPlus != 0) 
								GreekText(hdc, TextRect);
							if (bGreyText == 0 || bGreyPlus != 0) {
/* not fixed up since not used anymore */
								if (ExtTextOut(hdc, 0, 0, 0, NULL,
									str, 1, NULL) == 0) 
										cp_valid = 0;
							}
						}
					}
					else cp_valid = 0; 
				}
			}
			dvi_h += ldx;	/* accurate ... */
			dvi_hh += dx;
			horizontaldrift();
		}			
		cp_valid++;		/* indicate current point on window is valid */
	}
}
#endif

/* presently bDVITypeFlag is ALWAYS 0 - hence normalcharold is always used */

#ifdef IGNORED
static void normalchar (HFILE input, int c) {
	if (bDVITypeFlag > 0) normalcharnew(input, c);	/* DVITYPE algorithm */
	else normalcharold(input, c);					/* assorted	methods */
}
#endif

/* following special case treatement killed 95/Aug/15 */
/* more robust to treat just like set1 and suppress advance of dvi_h */

#ifdef IGNORED
void do_put1(HFILE input) {
	int refx, refy;
/*	unsigned char buff[1]; */
	char buff[1];			 /* 1992/Oct/3 */
/*	POINT TextPoint; */
	DWORD escapement;
	int dx, dy;
	long ldx;
	RECT TextRect; 					/* rectangle occupied by text */
/*	RECT InterRect;	 */				/* for intersection */
/*	int *lpDx=NULL; */
	int c;

	buff[0] = (unsigned char) ureadone(input); 
/*	buff[0] = (unsigned char) an_wingetc(input); */

/*	if (bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0) 
		lpDx = NULL; 	else lpDx = charwid; */

	if (bSkipFlag == 0) {
		refx = mapx(dvi_h); refy = mapy(dvi_v);
		if (cp_valid == 0) { /* set current point */
/*	want to check bDVITypeFlag on this ? */
/*	bDVITypeFlag always zero these days */
#ifdef IGNORED
			if (bDVITypeFlag > 0)
				(void) MoveToEx(hdc, dvi_hh, dvi_vv, NULL); 
			else (void) MoveToEx(hdc, refx, refy, NULL);
#endif
			(void) MoveToEx(hdc, refx, refy, NULL); 
		}
		if (fnt_exists != 0) {			/* draw only if font found by ATM */
			if (fnt_texatm != 0) {
				c = (buff[0] & 255);
				if (c <= 32 || c == 127)			/* 95/June/23 */
					buff[0] = remaptable[c];
			}
/* should we check for encoding problems as in normalcharold ? */
/* oh, how nice - we don't really need to compute the escapement here ! */
			if (bShowFlag != 0 && bTextOutFlag != 0) {
/* should check whether visible - to save time ! clipping needed ?*/
/* and so as to avoid adding crud to MetaFile ? */
				if (bCopyFlag == 0 && 
/*					(bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0)) */
					(bUseCharSpacing == 0)) { 	/* Favour Position OFF */
/* possibly use GetTextExtentPoint32 in WIN32 ??? */

/* didn't fix this with bUseNewEncode since no longer used */
					(void) GetTextExtentPoint32(hdc, buff, 1, &TextSize);

/*					(void) GetTextExtentPoint(hdc, buff, 1, &TextSize); */

					dx = TextSize.cx;
					dy = TextSize.cy;
/*					escapement = GetTextExtent(hdc, buff, 1); *//*  rectangle */
/*					dx = (int) LOWORD(escapement); */
/*					dy = (int) HIWORD(escapement); */
				}
				else {				/* Favour Position ON */
					ldx = AccurateExtent(str, 1);
					dx = mapd(ldx);
/*					dy = ascent; */			/* FIX THIS CROCK */
					dy = ascent + descent;	/* ??? */
				}			
				TextRect.left = dvi_hh; TextRect.right = dvi_hh + dx;
				TextRect.bottom = dvi_vv - descent;
				TextRect.top = dvi_vv + dy - descent;
/*	Deal Windows 95 bug with very narrow boxes in RectVisible */
/*	--- somewhat kludgy fix ... 1995/Aug/26 */
/*				if (bWin95) */
				if (bFixZeroWidth) { 
					if (bShowflag) { /* if bTagFlag == 0 */
/*						if (dx < dy) TextRect.right = TextRect.left + dy;*/
						if (dx < MinWidth)
							TextRect.right = TextRect.left + MinWidth;
					}
				} 
				if (bDrawVisible == 0) {	/* we don't use this anymore */
/*					if (bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0) */
					if (bUseCharSpacing == 0) { 	/* Favour Position OFF */
/* not fixed up since not used anymore */
						if (ExtTextOut(hdc, 0, 0, 0, NULL,
							buff, 1, NULL) == 0) cp_valid = 0;
					}
					else {							/* Favour Position ON */
/* not fixed up since not used anymore */
						if (ExtTextOut(hdc, 0, 0, 0, NULL,
							buff, 1, charwid) == 0) cp_valid = 0;
					}
				}
				else { 
/*	Avoid RectVisible() in MetaFile - use InterSectRect instead */
					if ((bCopyFlag == 0 && RectVisible(hdc, &TextRect) != 0) ||
						(bCopyFlag != 0 && InterSectRect(&CopyClipRect, 
							&TextRect) != 0)) {
/*						if (bUseCharSpacing == 0 || bPrintFlag != 0 || bCopyFlag != 0) */
						if (bUseCharSpacing == 0) { 
/* not fixed up since not used anymore */
							if (ExtTextOut(hdc, 0, 0, 0, NULL,
								buff, 1, NULL) == 0) cp_valid = 0;
						}
						else {
/* not fixed up since not used anymore */
							if (ExtTextOut(hdc, 0, 0, 0, NULL,
								buff, 1, charwid) == 0) cp_valid = 0;
						}
					}
				}
			}
			cp_valid = 0;	/* current point on window NOT valid */
		}
	}
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* deal with CMINCH */ /* deal with MANFNT */

/* add in PostScript 2.0 structuring convention bullshit */

/* can use either box or line to draw rules - don't need both */

/* combine /font9 def and /f9 def ? what ? */

/* reduce size of scanfile to allow optimize */

/* deal with other page formats ? A4 ? portrait ? */

/* precompute the scale factor used on each BOP - don't recompute it */

/* quick way to get fonts: go to end of file - NOT NEEDED */

/* maybe control how much goes on one output line ? */
/* presently somewhat kludgy in allowing MAXSHOWONLINE items */
/* rather count character columns and check before output if it will fit */

/* avoid bind def in some cases to allow redefinition via special ? */

/* may need to align lower left of rule to underlying grid... */
/* do tests with rules spaced along page to see effect */

/* shorten code for set_rule and put_rule and bp and ep */
/* set up scale constant to use at top of page */
/* improve mf  & set font */

/* should bop include "dvidict begin" and eop include "end" ? */
/* but watch out, fnt_def1 and xxx1 can appear between eop and bop */

/* check on fonthit even when font switch on page that is not printed ? */

/* check on redundant operations that dvipslog already does anyway */

/* check that nothing but nop and font defs happen between eop and bop ? */

/* Implement %%PageTable: ? */

/* avoid shortening octal codes for old interpretors maybe */

/* try and avoid actually giving up if at all possible */

/* when print even pages only, print a blank at first if last page is odd */
/* when print odd pages only, print a blank at first if first is even */

/* NEW FOR WINDOWS */

/* Use SetTextAlign() TA_UPDATECP to update current position ? OK */

/* check SetTextCharacterExtra() ? */ /* check SetTextJustification() ? */

/* Use GetTextExtent() ? OK */

/* use SetMapMode(MM_ISOTROPIC) ??? */

/* use DWORD MoveTo(hDC, mapx(dvi_h), mapy(dvi_v));  OK */

/* how to read out current position ??? */

/* don't need mapx(dvi_h), mapy(dvi_v) in TextOut ??? OK */

/* use page table later to get directly to desired page */

/* use previous and next to move around file easily ??? */

/* Scroll window before redrawing it to reduce `flicker' */

/* reset current point ONLY when about to draw character  OK */

/* put a border to indicate edge of page ??? */

/* need to REOPEN font and call fron dviwindo when WM_PAINT message */

/* flag to say we have read file and previous and next are valid */

/* release all font handles when user CLOSE file is closed again ? */

/* check on NULL returns from calls like CreateFont */

/* draw appropriate size border around the page ? */

/* need to clear screen before redrawing in magnify/demagnify case */

/* use FatalAppExit() to exit ? */

/* use RectVisible to determine whether something is even worth drawing */
/* or use InterSectRect in case of hDC being for MetaFile */

/* problem with first page coming up ? */

/* any REAL advantage to using TWIPS instead of device coordinates ? */

/* may need to draw one character at a time to use DVITYPE algorithm */

/* suppress `put' as well as `set' if outside rectangle to be repainted */

/* suppress rules if outside rectangle to be repainted ? */

/* need to take into account DEPTH of characters also */

/* remove relative MoveTo's once DVITYPE method works ? */

/* should we switch a fontttf[fn] flag if ATM Select Error received ? */

