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

#include <errno.h>

/* #include <stdlib.h> */	/* for getenv / _environ */

#include <commdlg.h>		/* for GetOpenFileName */
#include <direct.h>			/* for _getcwd WIN32 ? */

#include "atm.h"
#include "atmpriv.h"

#pragma warning(disable:4100)	// unreferenced formal parameters

#define YANDYTEXDLL

// #define DEBUGATM

// #define DEBUGENCODING

// #define DEBUGHEAP 

// #define DEBUGMAPPING 

#define DEBUGMRU

// #define DEBUGOPENFILE 

#define DEBUGPATTERN

#define DEBUGSEARCH

#define DEBUGMETAFILE

#define USEMEMCPY

/**********************************************************************
*
* DVI search function
*
* Copyright (C) 1991, 1992 Y&Y. All Rights Reserved. 
*
* DO NOT COPY OR DISTRIBUTE!
*
* This is the part that finds the page with the desired text
*
**********************************************************************/

BOOL bMetaTopLeft=0;		/* current point top left of MetaFile image */
							/* now hard-wired to use bottom left ... */

BOOL bCustomEncoding=0;					/* using ENCODING=... */

/* if ENCODING=ansinew and bTeXANSI=1 actually reencode to ansinew 96/Aug/28 */
/* which will cause bitmapped partial fonts in printed output 96/Aug/28 */
/* otherwise ENCODING=ansinew will mean *no* reencoding right ? */

int nEncodingBytes=0;		/* have many bytes compressed table takes up */
int nWidthBytes=512;		/* bytes for bogus width table 256 WORD */
							/* 256 * sizeof(WORD) */

/* ------------------------------------------------------------------------ */

#ifdef ATMSOLIDTABLE
char encodefont[MAXFONTS];			/* non-zero if encoding already set up */
#else
HGLOBAL hATMTables[MAXFONTS];		/* non-NULL if encoding already set up */
#endif

WORD DefaultChar = (WORD) -1;		/* -1 default char for text font */
WORD BreakChar = 0;					/*  0 or 32 break char for text font */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

BOOL bATMShowRefresh=1;		/* non-zero if bATMShow has to be (re)set up */
							/* set from winfonts.c when in `Show Font' */

/* #define HOWOFTEN 512 */	/* how often to check for mouse clicks - 2^n */

BOOL bMarkFlag=0;		/* scan dvi page for marks */

int marknumber=-1;		/* number of mark or -1 */

long lastsearch=-1;		/* position in file that last search ended */

long buttonpos=0;		/* position of button last pressed */

int buttondvipage;		/* page of button last pressed 95/Mar/10 */

long oldbuttonpos=0;		/* position of button last pressed */

int oldbuttondvipage;		/* page of button last pressed 95/Mar/10 */

int lastdvipage=0;		/* dvipage corresponding to lastsearch */ /* DEBUG */

long startposition=0;	/* position in file that this search started at */

long pagestart;			/* position in file at last bop */ /* not used */

BOOL partwayflag = 0;	/* adjust screen position only part way */

BOOL wrapped;			/* non-zero if wrapped around once */

int pagenumber;			/* dvipage of page searching on */

int textlength;			/* length of text to search for */

int textindex=0;		/* pointer to next character to match */

/* #define MAXSEARCHTEXT 80 */	/* maximum length of search text */

/* #define MAXKEY 64 */		/* maximum length of key in dviwindo.ini */

char findtext[MAXSEARCHTEXT];		/* string searched for (no white space) */

int nkeybuffer=MAXKEYBUFFER;		/* keep global so it ratchets up only */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

#define HYPERFILE

/* Simple hypertext state push down stack */ /* push graphics state also ? */

static char *modname = "WINSEARC";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* put up error message in box - with byte count if relevant */

/* #define MAXMESSAGE 256 */

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

static void wincancel(char *message) {
	HWND hFocus;
	int flag;

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, message, modname, MB_ICONSTOP | MB_OKCANCEL);
	if (flag == IDCANCEL) {
		bShowFlag = 0;				/* turn off displaying */
		finish = -1;				/* set early termination */
		bUserAbort = 1;				/* stop printing */
	}
}

void WriteError (char *msg) {
/*	char buffer[128]; */
	char *s;

	strcpy (debugstr, msg);
	s = debugstr + strlen(debugstr) - 1;
	while (s > debugstr && *s <= ' ') *s-- = '\0';	/* strip \n at end */
//	WritePrivateProfileString("Window", "LastError", msg, "dviwindo.ini"); 
//	WritePrivateProfileString(achPr, "LastError", debugstr, achFile);
	WritePrivateProfileString(achDiag, "LastError", debugstr, achFile);
	WriteTime(achDiag, "LastErrorTime");		/*  show date and time */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define FINDDLGORD       1540		/* from dlg.h */

FINDREPLACE fr;						/* needs to be static ! */

/* Following added 95/Aug/14 to support bDismissFlag */

UINT APIENTRY SearchHook (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 

/* UINT CALLBACK _export SearchHook (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) */

{
	WORD id;
	
	switch (msg) {
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);			
			switch (id) {
				case IDOK:
/*					read out Dismiss FLag */
					bDismissFlag = (int) SendDlgItemMessage(hDlg, 
						ID_DISMISS, BM_GETCHECK, 0, 0L);
/*					read out Wrap Flag */
/*					bWrapFlag = (int) SendDlgItemMessage(hDlg,
						ID_WRAP, BM_GETCHECK, 0, 0L); */
				break;

				default: return (FALSE);
			}
			break;								/* end of WM_COMMAND case */

		case WM_INITDIALOG:						/* message: initialize	*/
/*	Set up Dismiss Flag check box */
			(void) SendDlgItemMessage(hDlg, ID_DISMISS, BM_SETCHECK, 
				(WPARAM) bDismissFlag, 0L);
/*	Set up Wrap Flag check box */
/*			(void) SendDlgItemMessage(hDlg, ID_WRAP, BM_SETCHECK, 
				(WPARAM) bWrapFlag, 0L); */
			return (TRUE); /* Indicates the focus is *not* set to a control */
/*			break; */

		default: return (FALSE);
	}
	return (FALSE);		/* let Dialog Box Proc process message */
/*	return (TRUE);	*/	/* processed message - don't process again */
}		/* lParam unreferenced */

void ShowCommDlgError(char *);	/* in winprint.c */

/* new CommDlg Search 94/Jan/7 */ /* no longer used 94/Feb/8 */
/* BUT: reinstated after bug fixes 95/Mar/1 */

void CommSearch (HINSTANCE hInst, HWND hWnd) { 
/*	HWND hDlg; */						/* 1994/Feb/9 */

	if (uFindReplaceMsg != 0) return;	/* don't if already put up */
	
/*	Moved to before FindText(...) 1996/Aug/14 */
	uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

/*	Note: fr needs to be static because of modeless dialog box ! */
	memset (&fr, 0, sizeof(FINDREPLACE));
	fr.lStructSize = sizeof(FINDREPLACE);
	fr.hwndOwner = hWnd;
	fr.hInstance = hInst;		/* needed only for FR_ENABLETEMPLATE */
/*	fr.Flags = FR_HIDEUPDOWN; */
/*	fr.Flags = FR_HIDEUPDOWN | FR_ENABLETEMPLATE; */
	fr.Flags = FR_HIDEUPDOWN | FR_ENABLETEMPLATE | FR_ENABLEHOOK; /* 96/Aug/14 */
/*	We misuse the `Match Whole Word' button for `Wrap' */
	if (bWrapFlag) fr.Flags = fr.Flags | FR_WHOLEWORD;
	if (bCaseSensitive) fr.Flags = fr.Flags | FR_MATCHCASE;
	fr.lpstrFindWhat = searchtext;
	fr.wFindWhatLen = sizeof(searchtext);
	fr.lpfnHook = (LPFRHOOKPROC) SearchHook;		/* 1996/Aug/15 */
	fr.lpTemplateName = MAKEINTRESOURCE(FINDDLGORD);
/*	hDlg = FindText(&fr); */			/* any need to remember hDlg ? */
	hFindBox = FindText(&fr);

/*	Shouldn't we register Window message *first* ? */
/*	uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING); */
	if (hFindBox == NULL) {
		ShowCommDlgError("FindText");
	}
	else {
		bShowSearchFlag = 1;
		bShowSearchExposed = 1;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* assumes only one file open at a time */

/* versions of getc and ungetc using low-level C input/output */

/* static void unwingetc(int c, HFILE input) { 
	if (ungotten >= 0) {
		winerror("Repeated unwingetc"); errcount();
	}
	else filepos--;
	ungotten = c;
} */

static int replenishbuf(HFILE input, int bufferlen) {
/*	presumably buflen = 0 at this point */
/*	buflen = read(input, buffer, (unsigned int) bufferlen); */
	buflen = _lread(input, buffer, (unsigned int) bufferlen);
	if (buflen < 0) {
		strcpy (debugstr, "Read error in wingetc ");
		if (errno == EBADF) strcat(debugstr, "invalid file handle");
		(void) wincancel(debugstr);
		finish = -1;
	}
	bufptr = buffer;
	if (buflen <= 0) return EOF;	/* end of file or read error */
/*	bufptr = buffer; */
	return buflen;
}

#ifdef IGNORED
static int wingetc(HFILE input) {
	int c;
	if (ungotten >= 0)  {
		c = ungotten; ungotten = -1; filepos++; return c;
	}
	else if (buflen-- > 0) {
		filepos++; 
		return (unsigned char) *bufptr++;
	}
	else {
		if (replenishbuf(input, BUFFERLEN) < 0) return EOF;
		buflen--;
		filepos++; 
		return (unsigned char) *bufptr++;
	}
}
#endif

static int wingetc(HFILE input) {
	int c;

	if (ungotten >= 0)  {				/* can't happen? no ungetc ? */
		c = ungotten; ungotten = -1; filepos++; return c;
	}
	if (buflen <= 0) {
		if (replenishbuf(input, BUFFERLEN) < 0) return EOF;
	}
	buflen--;
	filepos++; 
	return (unsigned char) *bufptr++;
}

static long wintell(HFILE input) {		/* where are we in the file */
	return filepos;
}

/* possibly check for whether new position is somewhere in buffer ? */

static long winseek(HFILE input, long place) {
	long foo;
 
	if (filepos == place) return place;				/* dangerous ? */
	if (place < 0) {
		sprintf(debugstr, "Negative seek %ld", place);
		(void) wincancel(debugstr);
		return 0;
	}
/*	foo = lseek(input, place, 0); */
	foo = _llseek(input, place, SEEK_SET);
	if (foo != place) {
/*		sprintf(str, "Seek error: to %ld - at %ld", place, foo); */
		sprintf(debugstr, "Seek error: to %ld ", place);
		if (errno == EBADF)  strcat(debugstr, "invalid file handle");
		else if (errno == EINVAL) strcat(debugstr, "invalid origin or position");
		(void) wincancel(debugstr);
	}
	filepos = place;
	ungotten = -1;
	buflen = 0;
	bufptr = buffer;	/* redundant ? */
	return foo;
}

static int wininit(HFILE input) {		
	filepos = 0;			/* beginning of file */
	ungotten = -1;
	buflen = 0;				/* nothing buffered yet */
	bufptr = buffer;		/* redundant ? */
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for reading signed and unsigned numbers of various lengths */

static unsigned int ureadone (HFILE input) {	
	return (unsigned int) wingetc(input);
} 

static unsigned int ureadtwo (HFILE input) {
	return (wingetc(input) << 8) | wingetc(input);
}

static unsigned long ureadthree (HFILE input) {
	int c, d, e;
	c = wingetc(input);	d = wingetc(input);	e = wingetc(input);
	return ((((unsigned long) c << 8) | d) << 8) | e;
}

/* static unsigned long ureadfour (HFILE input) {
	int c, d, e, f;
	c = wingetc(input);	d = wingetc(input);
	e = wingetc(input);	f = wingetc(input);
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
} */

/* static int sreadone (HFILE input) {
	int c;
	c = wingetc(input);
	if (c > 127) return (c - 256);
	else return c;
} */

/* static int sreadtwo (HFILE input) {
	return (wingetc(input) << 8) | wingetc(input);
} */

/* static long sreadthree (HFILE input) {
	int c, d, e;
	c = wingetc(input);	d = wingetc(input);	e = wingetc(input);
	if (c > 127) c = c - 256; 
	return ((((long) c << 8) | d) << 8) | e;
} */

static long sreadfour (HFILE input) {
	int c, d, e, f;
	c = wingetc(input);	d = wingetc(input);
	e = wingetc(input);	f = wingetc(input);
	return ((((((long) c << 8) | (long) d) << 8) | e) << 8) | f;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void seado_push(void) {
}

void seado_pop(void) {
}

/* here we need to use ANSI characterset operations on user supplied string */
/* returns non-zero if accumulated complete match */

int matchchar(int c) {				/* see whether matched the search text */
	if (c == '-') return 0;			/* try and ignore hyphens */
	if (bCaseSensitive == 0) {		/* make all lower case */
/*		if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a'; */ /* NO */
/*		AnsiLower is obsolete, use CharLower */

		c = (int) LOWORD (CharLower ((LPSTR) MAKELONG (c, 0)));

/*		c = (int) LOWORD (AnsiLower ((LPSTR) MAKELONG (c, 0))); */

	}
	if (findtext[textindex] == (char) c) {
		if (++textindex == textlength) return -1;
	}
/*	Try and allow for wild card character 1994/Oct/29 */
	else if (findtext[textindex] == '?') {
		if (++textindex == textlength) return -1;
	}
/*	Try and allow for ffi, ffl, ff, fi, fl ligatures 1994/Oct/29 */
	else if (bIgnoreLigatures && !IsCharAlphaNumeric((char) c) &&
		findtext[textindex] == 'f') {
		if (++textindex == textlength) return -1;
		if (findtext[textindex] == 'f') {
			if (++textindex == textlength) return -1;
			if (findtext[textindex] == 'i' || findtext[textindex] == 'l') {
				if (++textindex == textlength) return -1;			
			}
		}
		else if (findtext[textindex] == 'i' || findtext[textindex] == 'l') {
			if (++textindex == textlength) return -1;
		}
		else textindex = 0;		/* no match - reset - start over */
	}
	else textindex = 0;		/* no match - reset - start over */
	return 0;
}

static void normalchar (HFILE input, int c) {
	if (matchchar(c) != 0) {
/*		lastsearch = wintell(input) - 1; */	/* ??? */
		lastsearch = wintell(input);	/* ??? */
		lastdvipage = pagenumber;		/* debugging */
		finish = -1;		/* if found match, then go home ! */
	}
}

void seado_set1(HFILE input) { 
	int c;
	c = wingetc(input);
/*	in text search process character - in other searches, ignore it */
	if (bMarkSearch == 0) normalchar (input, c);
/*	textindex = 0; ? */ 
}

/* don't bother making this efficient, since it should never happen */

void seado_set2(HFILE input) { /* NOT REALLY NEEDED ! */
	(void) wingetc(input);
	seado_set1(input);
}

void seado_set3(HFILE input) { /* NOT REALLY NEEDED ! */
	(void) wingetc(input); (void) wingetc(input);
	seado_set1(input);
}

void seado_set4(HFILE input) { /* NOT REALLY NEEDED ! */
	(void) wingetc(input); (void) wingetc(input); (void) wingetc(input);
	seado_set1(input);
}

/* set character c and DO NOT increase h by width of character */

void seado_put1(HFILE input) {
	int c;
	c = wingetc(input);
/*	in text search process character - in other searches, ignore it */
	if (bMarkSearch == 0) normalchar (input, c);
/*	seado_set1(input);	*/	 /*	(void) wingetc(input); */
/*	textindex = 0; ? */ 
}

void seado_put2(HFILE input) { /* NOT NEEDED */
	(void) wingetc(input);
	seado_put1(input);
}

void seado_put3(HFILE input) { /* NOT REALLY NEEDED ! */
	(void) wingetc(input); (void) wingetc(input);
	seado_put1(input);
}

void seado_put4(HFILE input) { /* NOT REALLY NEEDED ! */
	(void) wingetc(input); (void) wingetc(input); (void) wingetc(input);
	seado_put1(input);
}

void seado_set_rule(HFILE input) {
	int k;
	for (k = 0; k < 8; k++) (void) wingetc(input);
/*	sreadfour(input); */
/*	sreadfour(input); */
	textindex = 0; 
}

void seado_put_rule(HFILE input) {
	int k;
	for (k = 0; k < 8; k++) (void) wingetc(input);
/*	sreadfour(input); */
/*	sreadfour(input); */
	textindex = 0; 
}

void seado_bop(HFILE input) {			/* beginning of page */
	int k;
	pagestart = wintell(input) - 1;		/* remember bop position */
/*	current = wintell(input) - 1; */
/*	pagenumber++;	*/					/* increment page number */
/*	for(k=0; k < 10; k++) counter[k] = an_sreadfour(input); */
	for(k=0; k < 40; k++) (void) wingetc(input); /*  10 counters */
	for(k=0; k < 4; k++) (void) wingetc(input);	 /*  previous */
}

void seado_eop(HFILE input) {		/* end of page */
	if (wrapped != 0) {				/* wrapped around already ? */
		if (wintell(input) > startposition) {
#ifdef DEBUGSEARCH
			if (bDebug > 1) {
				sprintf(debugstr, "wrapped %d present pos %ld start pos %ld\n",
						wrapped, wintell(input), startposition);
				OutputDebugString(debugstr);
			}
#endif
			lastsearch = -1;	/* indicate failure */ /* NEW */
			finish = -1;		/* went past point where we started */
			return;
		}
	}
	pagenumber++;					/* increment page number */
	if (textures != 0) 
		(void) sreadfour(input);		/* skip over length code */
}

void seado_right1(HFILE input) { /* rare */
	(void) wingetc(input);
}

void seado_right2(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);
} 

void seado_right3(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);  (void) wingetc(input);	
} 

void seado_right4(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	
	(void) wingetc(input);	(void) wingetc(input);
} 

void seado_w0(void) {
}

void seado_w1(HFILE input) { /* rare */
	(void) wingetc(input);
}

void seado_w2(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);
} 

void seado_w3(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_w4(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	
	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_x0(void) {
}

void seado_x1(HFILE input) { /* rare */
	(void) wingetc(input);
}

void seado_x2(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_x3(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	(void) wingetc(input);	
}

void seado_x4(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	
	(void) wingetc(input);	(void) wingetc(input);	
}

void seado_down1(HFILE input) { /* rare */
	(void) wingetc(input);	
}

void seado_down2(HFILE input) { /* rare */
	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_down3(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	(void) wingetc(input);	
}

void seado_down4(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	
	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_y0(void) {
}

void seado_y1(HFILE input) { /* rare */
	(void) wingetc(input);	
}

void seado_y2(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	
}

void seado_y3(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_y4(HFILE input) { /* not used */
	(void) wingetc(input);	(void) wingetc(input);	
	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_z0(void) {
}

void seado_z1(HFILE input) {  /* rare */
	(void) wingetc(input);	
}

void seado_z2(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_z3(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	(void) wingetc(input);	
} 

void seado_z4(HFILE input) {
	(void) wingetc(input);	(void) wingetc(input);	
	(void) wingetc(input);	(void) wingetc(input);	
} 

static void switchfont(int f) {	/*  switching to other font */
/*	textindex = 0; */
}

void seado_fnt1(HFILE input) { /* switch fonts */
	switchfont((int) wingetc(input));
}

void seado_fnt2(HFILE input) { /* switch fonts */
	switchfont((int) ureadtwo(input));
}

void seado_fnt3(HFILE input) { /* switch fonts */
	switchfont((int) ureadthree(input));
}

void seado_fnt4(HFILE input) { /* switch fonts */
	switchfont((int) sreadfour(input));
}

/*	The following may take more work ... no kidding! */

void seado_xxxi (HFILE input, unsigned int n) {
/*	int c; */
	unsigned int k;
	int m;
	char specialstring[MAXFILENAME];
	char *s, *t;

/*	ignore specials when searching for string or when special too long */
/*	if (bMarkSearch == 0 || n >= MAXFILENAME) */
	if (bMarkSearch == 0 || n >= sizeof(specialstring)) {
		for (k = 0; k < n; k++) (void) wingetc(input);
		textindex = 0;  
	}
	else {
		s = specialstring;					/* grab the rest of special */
		for (k = 0; k < n; k++) *s++ = (char) wingetc(input);
		*s = '\0';							/* 95/Aug/12 */
		if (bMarkSearch == 1 && strncmp(specialstring, "mark:", 5) == 0) {
			s = specialstring + 5;
/*	 now get the mark label */ /* just use getbuttonname ??? */
/*			while (*s <= ' ') s++; */	/* search for non-space */
			while (*s <= ' ' && s > 0) s++;	/* search for non-space */
			if (*s == '\"') {	/* scan up to matching " */
				s++; 
				t = s;
/*				while (*t != '\"' && t < s + MAXMARKS) t++; */
				while (*t != '\"' && *t > 0 && t < s + MAXMARKS) t++;
			}
			else {	/* scan up to white space or end of string */
				t = s;
/*				while (*t > ' ' && t < s + MAXMARKS) t++; */ /* 95/Dec/20 */
				while (*t > ' ' && *t != ',' && t < s + MAXMARKS) t++;
			}
/*			*t = '\0'; */
			if (t != s) {
				m = t - s;
/*				if (strncmp(s, buttonlabel, m) == 0) */ /* 95/Oct/3 fix */
				if ((strlen(buttonlabel) == (unsigned int) m) &&
					(strncmp(s, buttonlabel, m) == 0)) {
					lastsearch = wintell(input);	/* ??? */
					lastdvipage = pagenumber;		/* debugging */
					finish = -1;
				}
			}
		}
		else if (bMarkSearch == 2 && strncmp(specialstring, "src:", 4) == 0) {	/* 98/Dec/15 */
			s = specialstring+4;
			while (*s == ' ') s++;
			if (sscanf(s, "%d%n", &srclineno, &m) == 1) {
				s += m;
				while (*s == ' ') s++;
				if (strlen(s) < sizeof(srcfile)) strcpy(srcfile, s);
				if (szSource != NULL &&
					(_stricmp(srcfile, szSource) == 0) &&
					(srclineno >= nSourceLine)) {
					srctaghit++;
					lastsearch = wintell(input);	/* ??? */
					lastdvipage = pagenumber;		/* debugging */
					finish = -1;
/* what else needs to be done here ? */
				}
			}
			
		}
	}
}

void seado_xxx1 (HFILE input) { /* for /special */
	unsigned n;
	n = ureadone(input); 
/*	n = wingetc(input); */
	seado_xxxi(input, n);
}

void seado_xxx2 (HFILE input) { /* for /special */
	unsigned int n;
	n = ureadtwo(input);
	seado_xxxi(input, n);
}

void seado_xxxl (HFILE input, unsigned long n) {
/*	int c; */
	unsigned long k;

/*	if it needs a long form of \special then it is too long ... */
	for(k = 0; k < n; k++) (void) wingetc(input);
	textindex = 0;  
}

void seado_xxx3 (HFILE input) { 
	seado_xxxl(input, ureadthree(input));
}

void seado_xxx4 (HFILE input) { 
	seado_xxxl(input, (unsigned long) sreadfour(input));
}

/* need to do this even if skipping pages */
/* nothing much should actually happen here !!! */
/* ignore this all - should be done properly on prescan !!! */
/* simplify later by flushing stuff ! */

void seafnt_def(HFILE input, unsigned int k) {
/*	unsigned int f; */
	unsigned int na, nl, i;
/*	int newfont=1; */
/*	char *fp; */

	for (k = 0; k < 12; k++) (void) wingetc(input); /* don't analyze again */
/*	fc[f] = sreadfour(input); */
/*	fs[f] = sreadfour(input); */
/*	fd[f] = sreadfour(input); */
	na = ureadone(input);			/* always zero */ 
/*	na = wingetc(input); */
	nl = ureadone(input); 
/*	nl = wingetc(input); */
	for (i = 0; i < na+nl; i++) (void) wingetc(input);
}

void seado_fnt_def1(HFILE input) { /* define font */
	seafnt_def(input, ureadone(input)); 
/*	seafnt_def(input, wingetc(input)); */
}

void seado_fnt_def2(HFILE input) { /* define font */
	seafnt_def(input, (unsigned int) ureadtwo(input));
}

void seado_fnt_defsub(HFILE input, unsigned long k) {
	seafnt_def(input, (unsigned int) k);
}

void seado_fnt_def3(HFILE input) { /* define font */
	seado_fnt_defsub(input, (unsigned long) ureadthree(input));
}

void seado_fnt_def4(HFILE input) { /* define font */
	seado_fnt_defsub(input, (unsigned long) sreadfour(input));
}

/* need to do this even if skipping pages */

void seado_pre(HFILE input) { /* doesn't do output */
	unsigned int k, j; /* i */

/*	sprintf(str, "HIT PRE! buflen %d filepos %ld", buflen, filepos); */
/*	winerror(str);	*/ /* debugging */

	pagenumber = 1;		/* 0 ??? */
	textindex = 0;  

/*	if (wrapped++ != 0) finish = -1; */

/*	i = ureadone(input); */
	(void) ureadone(input); 
	for (j = 0; j < 12; j++) (void) wingetc(input); /* don't read again */
/*	dvi_num = sreadfour(input); */
/*	dvi_den = sreadfour(input); */
/*	mag = sreadfour(input); */

	k = ureadone(input); 
/*	k = wingetc(input); */
/*	s = comment; */
	for (j = 0; j < k; j++) (void) wingetc(input); 
/*	redundant:  done in dvipslog */
	if (textures != 0) (void) sreadfour(input);	/* skip over length code */
}

/* need to do this even if skipping pages */

void seado_post(HFILE input) { /* doesn't do output */
	int k;

	textindex = 0;  
	(void) sreadfour(input);	/* pointer to previous */

	pagestart = wintell(input) - 1; 
/*	current = wintell(input) - 1; */

	for (k = 0; k < 12; k++) (void) wingetc(input);	/* don't read again */
/*	dvi_num = sreadfour(input); */
/*	dvi_den = sreadfour(input); */
/*	mag = sreadfour(input); */
	for (k = 0; k < 8; k++) (void) wingetc(input);	/* don't read again */
/*	l = sreadfour(input); */
/*	u = sreadfour(input); */
	for (k = 0; k < 4; k++) (void) wingetc(input);	/* don't read again */
/*	s = ureadtwo(input);	 */
/*	t = ureadtwo(input);	*/
/*	if (bWrapFlag != 0 && wrapped++ == 0) */
/*	*always* wrap at end of file if its hyper text mark search 95/Dec/28 */
/*	or \special{src...} search 98/Dec/12 ? */
	if ((bWrapFlag != 0 || bMarkSearch != 0) && wrapped++ == 0) {
/*		current = dvistart; */
#ifdef DEBUGSEARCH
		if (bDebug > 1) OutputDebugString("Wrapping back to start\n");
#endif
		(void) winseek(input, dvistart);
		pagenumber = 1;		/* 0 ??? */
	}
	else {
		finish = -1;		/* gone around one too many times */
		lastsearch = -1;	/* ??? */	/* indicate failure */
	}
}

/* could terminate as soon as wrapped != 0 && wintell(input) > startpos */

void seado_post_post (HFILE input) { /* should never get here ! */
/*	unsigned long q; */
/*	unsigned int i; */
/*	q = sreadfour(input); */
	(void) sreadfour(input);
/*	i = ureadone(input); */
	(void) ureadone(input); 
	finish = -1;			/* STOP it before we hit EOF !!! */
/*	check ID_BYTE again ? */
/* followed by at least four 223's */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

long oldlastsearch;

/* searchdvifile used by both string search and search for hypertext mark */
/* flag = 0 for text string search */
/* flag = 1 for hypertext mark search */
/* flag = 2 for \special{src...} search */

/* NOTE:  removes white space and `-' from string supplied */

int searchdvifile (HFILE input, long position, char *s, BOOL flag) {
	int c, j, k;
/*	long oldlastsearch; */
/*	int flag=0; */
	int	olddvipage;			/* debugging */ /* not accessed */
	long bytecount = 0;

	bMarkSearch = flag;
	if (bDebug > 1) {
		sprintf(debugstr, "searchdvifile %ld %s bMarkSearch %d",
				position, s, flag);
		OutputDebugString(debugstr);
	}

/*	if (input == -1) */				/* sanity check 95/Aug/14 */
	if (input == HFILE_ERROR) {		/* sanity check 95/Aug/14 */
		winerror("Invalid File Handle");
		return -1;
	}

	(void) wininit(input);			/* buffer may have been screwed with */

/*	oldlastsearch = lastsearch; */
	olddvipage = dvipage;			/* remember for debugging ? */

#ifdef DEBUGSEARCH
	if (bDebug) {						/* DEBUGGING STUFF - FLUSH LATER */
		if (bFileValid == 0) winerror("bFileValid == 0");
		if (hPages == NULL) winerror("hPages == NULL");
		if (dvipage == -INFINITY) winerror("wantedpage == -INFINITY"); 
		if (dvipage <= 0 || dvipage > dvi_t) {
			sprintf(str, "wantedpage is %d [0--%d]", dvipage, dvi_t);
			winerror(str);
		}
	}
#endif
	
	if (pagecurrent(dvipage) == 0) { /* get file position for this page */ 
#ifdef DEBUGSEARCH
		if (bDebug) {					/* DEBUGGING STUFF - FLUSH LATER */
			sprintf(debugstr, "pagecurrent for %d yielded zero\n", dvipage);
			OutputDebugString(debugstr);
		}
#endif
		current = dvistart;  dvipage = 1;
	}
/*	sprintf(str, "SEARCHDVIFILE: dvipage %d lastsearch %ld current %ld", 
		dvipage, lastsearch, current);
	winerror(str); */
	if (current > lastsearch) {
		if (bDebug > 1) {
			sprintf(str, "current (%ld) > lastsearch (%ld)?", 
				current, lastsearch);
			winerror(str);	
		} 
/*		dvipage--; */

/*		lastsearch = -1; */		/* more aggressive 1993/March/29 ??? */
/*		lastdvipage = dvipage; */	/* debugging */

		if (pagecurrent(dvipage) == 0) { /* more aggressive ? */
			current = dvistart;	dvipage = 1;	
		}
		lastsearch = current;	/* more aggressive 1993/March/29 ??? */
	}
		
/*	strncpy(findtext, s, MAXSEARCHTEXT); */ /* remember text to search for */
	if (bMarkSearch == 0) {				/* text search case */
		strncpy(findtext, s, sizeof(findtext)); /* remember text search for */
/*		squeeze out spaces and hyphens */
		k = j = 0;
		while ((c = findtext[k++]) != '\0') {
			if (c > ' ' && c != '-') {
				if (bCaseSensitive == 0) {		/* make lower case */
/*					if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a'; */ /* NO */
/*	 AnsiLower is obsolete, use CharLower */

					c = (int) LOWORD (CharLower ((LPSTR) MAKELONG (c, 0)));

/*					c = (int) LOWORD (AnsiLower ((LPSTR) MAKELONG (c, 0))); */
				}
				findtext[j++] = (char) c;
			}
		}
		findtext[j] = '\0';
#ifdef DEBUGSEARCH
		if (bDebug > 1) OutputDebugString(findtext);
#endif

		textlength = (int) strlen(findtext);	/* how many characters */
		if (textlength > MAXSEARCHTEXT)		/* debugging 93/March/29 */
			winerror("Search text too long"); 
		textindex = 0;				/* how many have been matched */
	}	/* end of bMarkSearch == 0 */

/*	sprintf(str, "string %s length %d", findtext, textlength); */

	startposition = position;
	wrapped = 0;				/* not wrapped yet! */
	finish = 0;					/* not finished yet ! */ 

	pagenumber = dvipage;		/* incremented by eop */

#ifdef DEBUGSEARCH
	if (bDebug > 1) {
		sprintf(debugstr, "seek to %ld\n", position);
		OutputDebugString(debugstr);
	}
#endif
	(void) winseek(input, position);

	for(;;) {
		if ((bytecount++ & (HOWOFTEN-1)) == 0)	{ /* see if interrupted */
			if(bEnableTermination != 0 && checkuser() != 0) {
				finish = -1;			/* redundant */
				bUserAbort = 1;			/* set for termination */
/*				lastsearch = wintell(input) - 1; */
			}  
		}
		c = wingetc(input);
		if (c == EOF) {
			winerror("Unexpected EOF on input");
			finish = -1;			/* redundant */
			break; /* giveup(13); */ 
		}
		if (c < 128) {
			if (bMarkSearch == 0) normalchar(input, c);	/* text searach case */
		}
/* set character in current font and advance h by width of character */
		else if ((c - 171) >= 0 && (c -171) < 64) 
/*			fs = (c - 171);	*/				/*	switch to font (c - 171) */
			switchfont(c - 171);
		else switch(c) {
				case set1: seado_set1(input); break;
				case set2: seado_set2(input); break;  /* silly */
				case set3: seado_set3(input); break;  /* silly */
				case set4: seado_set4(input); break;  /* silly */
				case set_rule: seado_set_rule(input); break;
				case put1: seado_put1(input); break ;
				case put2: seado_put2(input); break;	/* silly */
				case put3: seado_put3(input); break;	/* silly */
				case put4: seado_put4(input); break;	/* silly */
				case put_rule: seado_put_rule(input); break;	
				case nop: break;				/* easy, do nothing ! */
				case bop: seado_bop(input); break;
				case eop: seado_eop(input); break;
				case push: seado_push(); break;
				case pop: seado_pop(); break;
				case right1: seado_right1(input); break;
				case right2: seado_right2(input); break;  
				case right3: seado_right3(input); break; 
				case right4: seado_right4(input); break; 
				case w0: seado_w0(); break;
				case w1: seado_w1(input); break;
				case w2: seado_w2(input); break; 
				case w3: seado_w3(input); break; 
				case w4: seado_w4(input); break;		/* not used ? */
				case x0: seado_x0(); break;
				case x1: seado_x1(input); break;
				case x2: seado_x2(input); break; 
				case x3: seado_x3(input); break; 
				case x4: seado_x4(input); break;		/* not used ? */
				case down1: seado_down1(input); break;
				case down2: seado_down2(input); break; 
				case down3: seado_down3(input); break; 
				case down4: seado_down4(input); break; 
/*				case y0: seado_y0(); break; */			/* conflict math.h */
				case y5: seado_y0(); break;
/*				case y1: seado_y1(input); break; */		/* conflict math.h */
				case y6: seado_y1(input); break;
				case y2: seado_y2(input); break; 
				case y3: seado_y3(input); break; 
				case y4: seado_y4(input); break;		/* not used ? */
				case z0: seado_z0(); break;
				case z1: seado_z1(input); break;
				case z2: seado_z2(input); break; 
				case z3: seado_z3(input); break; 
				case z4: seado_z4(input); break;		/* not used ? */
				case fnt1: seado_fnt1(input); break;
				case fnt2: seado_fnt2(input); break;	/* silly */
				case fnt3: seado_fnt3(input); break;	/* silly */
				case fnt4: seado_fnt4(input); break;	/* silly */
				case xxx1: seado_xxx1(input); break;
				case xxx2: seado_xxx2(input); break;	/* not used ? */
				case xxx3: seado_xxx3(input); break;	/* not used ? */
				case xxx4: seado_xxx4(input); break; 
				case fnt_def1: seado_fnt_def1(input); break;
				case fnt_def2: seado_fnt_def2(input); break; /* silly */
				case fnt_def3: seado_fnt_def3(input); break; /* silly */
				case fnt_def4: seado_fnt_def4(input); break; /* silly */
				case post: seado_post(input); break;
				case pre: seado_pre(input); break;
				case post_post: seado_post_post(input); break;
				default: {
/* we already complained about this in dvipslog ... */
					finish = -1;	/* ??? */
/* this should normally not happen: */
					sprintf(str, "Unknown DVI command (%d)", c);
					winerror(str);
					errcount();
					break;
				} 
/*				break; */
		}
#ifdef IGNORED
		if (pagenumber < olddvipage) { 
			if (bDebug > 1) {
				sprintf (debugstr, "pagenumber (%d) < olddvipage (%d)\n",
						 pagenumber, olddvipage);
				OutputDebugString (debugstr);
			}
			break;						/* ??? */
		}
#endif
		if (finish != 0) {
			if (bDebug > 1) {
				sprintf(debugstr, "finish %d", finish);
				OutputDebugString(debugstr);
			}
			break;			/* trip out */
		}
	}
/*	bMarkSearch = 0; */						/* 98/Dec/15 ? */
	if (bUserAbort != 0) return 0;			/* interrupted */
/*	if (lastsearch != oldlastsearch && lastsearch > 0) */
	if (lastsearch < 0) {				/* did not find anything */
/*		lastsearch = oldlastsearch;	*/	/* restore old value */
		return 0;
	}
	else {
		if (bDebug > 1) {
			sprintf(debugstr, "page %d", pagenumber);
			OutputDebugString(debugstr);
		}
		return pagenumber;				/* did find something */
	}
}

/* Returns non-zero if need to redraw screen after cursor movement */

int AdjustCursor (HWND hWnd, HDC hDC) {
	RECT ClientRect;			/* client area */
	POINT CurPoint; 
	POINT steparr[2];	
/*	int xll, yll, xur, yur; */
	int xlld, ylld, xurd, yurd;
	long xmd, ymd; /* int xmd, ymd; */
	long xdd, ydd;
/*	int xdn, ydn; */
	int redrawflag = 0;
	
	if (bSpreadFlag == 0) {
		xdd = mapx(dvi_spot_h);	ydd = mapy(dvi_spot_v);
	} 
	else {				/*  this is only non-trivial when bSpreadFlag != 0 */
		xoffsetold = xoffset; 
		if (bCountZero == 0) {	 /* arbitrarily use right page */
			xoffset = rightpageoffset(xoffsetold);
		}
		else { /*  this is only a pain when bCountZero != 0 */
			usepagetable(dvipage, 0);		/* redundant ? */
			if (rightcountzero != -INFINITY) {
				xoffset = rightpageoffset(xoffsetold);
			}
			else { /* this is only hard if its on the left page */
/*				sprintf(str, "ADJUSTCURSOR: dvipage %d, rightcountzero %ld", 
					dvipage, rightcountzero);
				winerror(str); */
				dvipage++;
				usepagetable(dvipage, 0);
/*				sprintf(str, "ADJUSTCURSOR: dvipage %d, leftcountzero %ld", 
					dvipage, leftcountzero);
				winerror(str); */
				if (leftcountzero == -INFINITY) winerror("Impossible!");
				xoffset = leftpageoffset(xoffsetold); 
			}
		}
		xdd = mapx(dvi_spot_h);	ydd = mapy(dvi_spot_v);
		xoffset = xoffsetold;
	}

	GetClientRect(hWnd, (LPRECT) &ClientRect);
/*	xll = ClientRect.left; yll = ClientRect.bottom; */
/*	xur = ClientRect.right; yur = ClientRect.top; */
	steparr[0].x = ClientRect.left; steparr[0].y = ClientRect.bottom;
	steparr[1].x = ClientRect.right; steparr[1].y = ClientRect.top;
	(void) DPtoLP(hDC, steparr, 2);
	xlld = steparr[0].x; ylld = steparr[0].y;
	xurd = steparr[1].x; yurd = steparr[1].y;
	
/*	xmd = (int) xdd; ymd = (int) ydd; */
	xmd = xdd; ymd =  ydd; 
	if (xdd < xlld || xdd > xurd) {
		xmd = (xlld + xurd) / 2; 
/*		if (partwayflag == 0) xoffset += ((long) xmd - xdd); */
		if (partwayflag == 0) xoffset += (xmd - xdd); 
		else {
/*			xoffset += ((long) xmd - xdd)/2; */
			xoffset += (xmd - xdd)/2;
/*			xmd = (xmd + (int) xdd)/2; */
			xmd = (xmd + xdd)/2;
		}
		redrawflag++;					/* not neccessarily - scroll ? */
	}
	if (ydd < ylld || ydd > yurd) {
		ymd = (ylld + yurd) / 2;	
/*		if (partwayflag == 0) yoffset += ((long) ymd - ydd); */
		if (partwayflag == 0) yoffset += (ymd - ydd);
		else {
/*			yoffset += ((long) ymd - ydd)/2; */
			yoffset += (ymd - ydd)/2;
/*			ymd = (ymd + (int) ydd)/2; */
			ymd = (ymd + ydd)/2;
		}
		redrawflag++;				/* not neccessarily - scroll ? */
	}		

/*	xdn = mapx(dvi_spot_h);		ydn = mapy(dvi_spot_v);
	sprintf(str, "xdn %ld ydn %ld xmd %d ymd %d", xdn, ydn, xmd, ymd);
	winerror(str); */

/*	steparr[0].x =  xmd; steparr[0].y =  ymd; */
	steparr[0].x = (int) xmd; steparr[0].y = (int) ymd;
	(void) LPtoDP(hDC, steparr, 1);
/*	can this LPtoDP fail?  Do we care? */
	CurPoint.x = steparr[0].x; CurPoint.y = steparr[0].y;
	ClientToScreen(hWnd, &CurPoint);
	if (bDebug > 1) {
		sprintf(debugstr, "New Cursor Position %d %d redraw",
				CurPoint.x, CurPoint.y, redrawflag);
		OutputDebugString(debugstr);
	}
	SetCursorPos(CurPoint.x, CurPoint.y);
	return redrawflag;
}

void savehyperstate(void) {
	int k;

	if (hyperindex == MAXHYPERPUSH - 1) {		/* move down if depth exceeded */
#ifdef HYPERFILE
		if (hyperfile[0] != NULL) {
			free (hyperfile[0]);
			hyperfile[0] = NULL;
		}
#endif
		for (k = 1; k < MAXHYPERPUSH; k++) {	/* overflowed stack */
			hyperpos[k-1] = hyperpos[k];
			hyperpage[k-1] = hyperpage[k];
#ifdef HYPERFILE
			hyperfile[k-1] = hyperfile[k];	 /* hMarks ? */
#endif
		}
#ifdef HYPERFILE
			hyperfile[hyperindex] = NULL;
#endif
		hyperindex--;
	}
#ifdef DEBUGHYPERTEXT
	if (bDebug > 1) {				/* DEBUGGING 1995/Mar/10 */
#ifdef HYPERFILE
		sprintf(debugstr, "Saving:  hyper index %d pos %ld page %d in %s\n",
/*			hyperindex, oldbuttonpos, oldbuttondvipage, FileName); */
			hyperindex, oldbuttonpos, oldbuttondvipage, szHyperFile);
#else
		sprintf(debugstr, "Saving: hyperpos[%d] %ld hyperpage[%d] %d\n",
			hyperindex, oldbuttonpos, hyperindex, oldbuttondvipage);
#endif
		OutputDebugString(debugstr);
	}
#endif
	if (hyperindex < MAXHYPERPUSH - 1) {
/*		hyperpos[hyperindex] = lastsearch; */
/*		hyperpos[hyperindex] = buttonpos; */			/* ??? */
		hyperpos[hyperindex] = oldbuttonpos;			/* 95/Aug/12 */
/*		if (bDebug && lastdvipage == 0) 
			winerror("Pushing zero page"); */	/* DEBUGGING ONLY */
		if (lastdvipage == 0) lastdvipage = dvipage;	/* ??? */
/*		hyperpage[hyperindex] = lastdvipage; */			/* ??? */
/*		hyperpage[hyperindex] = buttondvipage; */	/* experiment 95/Mar/10 */
		hyperpage[hyperindex] = oldbuttondvipage;	/* 95/Aug/12 */
#ifdef HYPERFILE
		if (hyperfile[hyperindex] != NULL) {
			free(hyperfile[hyperindex]);
			hyperfile[hyperindex] = NULL;
		}
		hyperfile[hyperindex] = zstrdup(szHyperFile);	/* not OpenName */
		*szHyperFile = '\0';
#endif
		hyperindex++;
	}
	bHyperLast = 1;	/* last action was hyper text jump 95/Aug/12 */
}

void restorehyperstate(void) {
	int hyperindexsvd;
	char *s;

	if (hyperindex == 0) return;	/* nothing to to */
	hyperindex--;

/*	lastsearch = hyperpos[hyperindex]; */
	current = hyperpos[hyperindex];
/*	lastdvipage = hyperpage[hyperindex]; */
	dvipage = hyperpage[hyperindex];
/*	if (bDebug && dvipage == 0) {	
		winerror("Restoring zero page"); */ /* DEBUGGING ONLY */
#ifdef DEBUGHYPERTEXT
	if (bDebug > 1) {				/* DEBUGGING 1995/Mar/10 */
#ifdef HYPERFILE
		sprintf(debugstr, "Restoring: hyper index %d pos %ld page %d in %s\n",
			hyperindex, current, dvipage, hyperfile[hyperindex]);
#else
		sprintf(debugstr, "Restoring: hyperpos[%d] %ld hyperpage[%d] %d\n",
			hyperindex, current, hyperindex, dvipage);
#endif
		OutputDebugString(debugstr);
	}
#endif
#ifdef HYPERFILE
/*	if (hyperfile[hyperindex] == NULL) DAMN ERROR ! */

	if (hyperfile[hyperindex] != NULL) {
/*	let's first check whether it is the same file we are already viewing ... */
		if (strchr(OpenName, '\\') == NULL &&
			strchr(OpenName, '/') == NULL &&
			strchr(OpenName, ':') == NULL) {
/*			strcpy(FileName, DefPath); */
			strcpy(szHyperFile, DefPath);
			s = szHyperFile + strlen(szHyperFile) - 1;
			if (*s != '\\' && *s != '/') strcat(szHyperFile, "\\");
			strcat(szHyperFile, OpenName);
		}
		else strcpy(szHyperFile, OpenName);
/*		if (strcmp(OpenName, hyperfile[hyperindex]) != 0) */
		if (_stricmp(szHyperFile, hyperfile[hyperindex]) != 0) {
			strcpy(OpenName, hyperfile[hyperindex]);
			hyperindexsvd = hyperindex;
			SwitchDVIFile(hwnd, dvipage, 0);	/* and now activate it !!! */			
/*	Need to somehow keep file open - avoid hFile = -1 ! */
			ReOpenFile(hwnd);				/* in dviwindo.c */
			hyperindex = hyperindexsvd;
		}
		free (hyperfile[hyperindex]);		/* free it, it has been used */
		hyperfile[hyperindex] = NULL;
		current = hyperpos[hyperindex];
		dvipage = hyperpage[hyperindex];
	}
#endif
}

void hyperundopush(void) {					/* 95/Dec/29 */
	if (hyperindex == 0) return;			/* nothing to do */
	if (bDebug > 1) OutputDebugString("Undoing hyper save\n");
	hyperindex--;
	free (hyperfile[hyperindex]);
	hyperfile[hyperindex] = NULL;
/*	current = hyperpos[hyperindex]; */
/*	dvipage = hyperpage[hyperindex]; */
}

void free_hyperfile (void) {
	int k;
/*	if (hyperindex == 0) return; */
/*	for (k = 0; k < hyperindex; k++) */
	for (k = 0; k < MAXHYPERPUSH; k++) {
		if (hyperfile[k] != NULL) {
			free(hyperfile[k]);
			hyperfile[k] = NULL;
		}
	}
	hyperindex = 0;
}

/* pagenumber is dvipage of page with text searched for on it */

/* lastsearch is byte in file where match was found */

/* searchandmark used by string search, search for hypertext mark */
/* and search for \special{src:...} */
/* flag / bMarkSearch is -1 for hyper button pop */
/* flag / bMarkSearch is  0 for ordinary text string search /*
/* flag / bMarkSearch is  1 for hyper button push target search */
/* flag / bMarkSearch is  2 for src \special 98/Dec/12 */

/* sets dvipage number of page with text searched for */
/* or, one page later if bSpreadFlag != 0 and bCountZero != 0, */
/* and page has even counter[0] */

/* returns 0 if no need to InvalidateRect (same page or failed) */
/* returns -1 otherwise */

/* Not safe in following to put up message boxes since repaint possible */

int searchandmark (HWND hWnd, BOOL flag) {
	HDC hDC;
	HWND hFocus;
	int olddvipage, redrawflag=0;

/*	if (bBusyFlag != 0) return 0; */		/* 1993/March/29 */

	bMarkSearch = flag;				/* remember globally */

/*	if (bMarkSearch > 0) savehyperstate(); */
	if (bMarkSearch == 1) savehyperstate(); /* PUSH --- for hyper button search */

	if (bMarkSearch == 0 && *searchtext == '\0') 
		return 0;					/* empty text string - nothing to do */

	bUserAbort = 0;					/* 1993/March/29 ??? */

	if (ReOpenFile(hWnd) < 0) return 0;		/* file failed to reopen */

	olddvipage = dvipage;			/* save up current page 94/Dec/11 ! */

	if (bMarkSearch == -1) {				/* hyper text pop ? */
		if (hyperindex == 0) {
			bMarkSearch = 0;
			return 0;	/* nothing to do */
		}
		restorehyperstate();			/* POP hypertext */
/*		if (bDebug > 1) {
			sprintf(debugstr, "POP to PAGE %d\n", dvipage);
			OutputDebugString(debugstr);	
		} */
		lastsearch = current;		/* ??? */
		lastdvipage = dvipage;		/* ??? */
/*		(void) winseek(input, current); */
		goto knownhit;
	}	/* end of bMarkSearch == -1 (hyper pop) */

/*	redrawflag = 0; */
/*	olddvipage = dvipage;	*/		/* moved higher up 94/Dec/11 */

/*	start were we left off last time --- if there was a last time */
	if (lastsearch < 0) {	/* either no previous search, or an error ... */
#ifdef DEBUGSEARCH
		if (bDebug > 1) OutputDebugString("lastsearch < 0\n"); 
#endif
		lastsearch = current;
		lastdvipage = dvipage; /* debugging */
	}
	else {					/* there was a previous search */
#ifdef DEBUGSEARCH
		if (dvipage != lastdvipage) {
			if (bDebug > 1) {
				sprintf(debugstr, "dvipage %d lastdvipage %d\n",
						dvipage, lastdvipage);
				OutputDebugString(debugstr);
			}
		} 
#endif
		dvipage = lastdvipage;				/* debugging */
/*		usepagetable(dvipage, 0); */
	}

/*  need to search file for text specified */

/*	BAD THINGS CAN HAPPEN HERE IF DVI FILE SCAN PUTS UP ERROR BOXES */
	bScanFlag = 1;			/* hack to prevent repaint 95/Dec/28 */
	bHourFlag = 1;
	hSaveCursor = SetCursor(hHourGlass); 
/*	sprintf(str, "SEARCHDVIFILE: dvipage %d lastsearch %ld current %ld",
		dvipage, lastsearch, current);
	winerror(str); */

	bEnableTermination = 0;		/* disable early termination */	/* debug */
	oldlastsearch = lastsearch; 
	if (bMarkSearch == 0)			/* text search */
		dvipage = searchdvifile(hFile, lastsearch, searchtext, bMarkSearch); 
/*	else */
	else if (bMarkSearch == 1)		/* hypertext button search */
		dvipage = searchdvifile(hFile, lastsearch, buttonlabel, bMarkSearch); 
	else if (bMarkSearch == 2) 		/* src special search 98/Dec/12 */
		dvipage = searchdvifile(hFile, lastsearch, szSource, bMarkSearch);
	(void) SetCursor(hSaveCursor);

	bHourFlag = 0;
	bScanFlag = 0;			/* undo hack to prevent repaint 95/Dec/28 */

/*	deal with case where mark is on same page as button and no redraw */
/*	in which case bHyperUsed is reset to zero, and may not get set again */
/*	if (bMarkSearch != 0 && dvipage == olddvipage) */
	if (bMarkSearch == 1 && dvipage == olddvipage)
		bHyperUsed = 1;							/* 94/Oct/10 */
	if (bMarkSearch == 2 && dvipage == olddvipage)
		bSourceUsed = 1;						/* 98/Dec/10 */

/*	check and reset bUserAbort ??? */
	if (bUserAbort != 0) {
		if (bDebug > 1)	OutputDebugString("Search Aborted\n");
		bUserAbort = 0;

		lastsearch = oldlastsearch;		/*  restore old value ??? */
		dvipage = olddvipage;

/*		is it safe to exit here? file still open? */
		if (bKeepFileOpen == 0) {
/*			if (hFile < 0) wincancel("File already closed (no hit)"); */
			if (hFile == HFILE_ERROR) {
				if (bDebug) {
					sprintf(debugstr, "%s (%s)\n", "File already closed", "no hit A");
					if (bDebug > 1) OutputDebugString(buffer);
					else (void) wincancel(debugstr);
				}
			}
			else (void) _lclose (hFile);	/* need to close input again */
/*			hFile = -1; */
			hFile = HFILE_ERROR; 
		}		/* end of if (bKeepFileOpen == 0) */
		bMarkSearch = 0; 
		return 0;
	}	/* end of bUserAbort */

/*	Now first deal with failure case */

	if (dvipage == 0) {					/*  did not find anything */
		lastsearch = oldlastsearch;		/*  restore old value */
		dvipage = olddvipage;

/*		if (bMarkSearch == 0) sprintf(str, "`%s' not found", searchtext);
		else sprintf(str, "`%s' not found", buttonlabel);

		if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
		(void) MessageBox(hFocus, str, "Search String", 
			MB_ICONEXCLAMATION | MB_OK); */

		if (bKeepFileOpen == 0) {
			if (hFile == HFILE_ERROR) {
				if (bDebug) {
					sprintf(debugstr, "%s (%s)\n", "File already closed", "no hit B");
					if (bDebug > 1) OutputDebugString(debugstr);
					else (void) wincancel(debugstr);
				}
			}
			else (void) _lclose (hFile);	/* need to close input again */
/*			hFile = -1; */
			hFile = HFILE_ERROR; 
		}			/* end of bKeepFileOpen == 0 */

/*		moved down here 95/Dec/20 to avoid MessageBox repaint */
/*		if (bMarkSearch == 0) sprintf(debugstr, "`%s' not found", searchtext); */
/*		else sprintf(debugstr, "`%s' not found", buttonlabel); */
		if (bMarkSearch == 0)
			sprintf(debugstr, "%s `%s' not found", "String", searchtext);
		else if (bMarkSearch == 1)
			sprintf(debugstr, "%s `%s' not found", "Anchor", buttonlabel);
		else if (bMarkSearch == 2)
			sprintf(debugstr, "Source line %d in `%s' not found",
					nSourceLine, szSource);
		else sprintf(debugstr, "Error");		/* should not happen */
		if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
		(void) MessageBox(hFocus, debugstr, "Search", 
			MB_ICONEXCLAMATION | MB_OK);

/*		if we failed hypertext search don't keep saved info on stack */
		if (bMarkSearch == 1) hyperundopush();	/*  --- 98/Dec/12 */

		bMarkSearch = 0; 		/* ??? */
		return 0;		/* failed */
	} /* end of dvipage == 0 (i.e. failed search) */

/*	drop through here if search succeeded */

knownhit:			/* or come here from hypertext POP */

/*	bMarkSearch = 0; */		/* ??? */

	if (bDebug > 1) OutputDebugString("HIT");
/*	now apparently have found a hit --- time to do something about it */

/*  need to rewind ? */
	if (lastsearch < 0) {

#ifdef DEBUGSEARCH
/*		if (bDebug) winerror("lastsearch < 0"); */
		if (bDebug > 1) OutputDebugString("lastsearch < 0\n");	/* 95/Dec/20 */
#endif
		lastsearch = current;
		lastdvipage = dvipage;
	}
/*	sprintf(str, "lastsearch %ld", lastsearch);	winerror(str); */

/*	if (dvipage != olddvipage) redrawflag++; */

	usepagetable(dvipage, 0);

/*	sprintf(str, "current %ld", current);	winerror(str); */
/*	showflag = 0; */

	bScanFlag = 1;			/* hack to prevent repaint 95/Dec/21 */
							/* do before GrabWidths and GetDC */
	hDC = GetDC(hWnd);				/* ??? */
/*	sprintf(str, "GetDC %d", hDC);	winerror(str); */
/*	if (bCopyFlag == 0) */			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS);		/* set unit to twips */
	bHourFlag = 1;
	hSaveCursor = SetCursor(hHourGlass); 

	bEnableTermination = 0;
	dvi_spot_h = oneinch;		/* if missed ! */
	dvi_spot_v = oneinch;		/* if missed ! */
	bMarkSpotFlag = 1;

	if (pagecurrent(dvipage) == 0)  {
		current = dvistart;
		dvipage = 1;
	}
		
	if (bDebug > 1) {
		sprintf(debugstr, "SCANDVIPAGE: dvipage %d lastsearch %ld current %ld bMarkSearch %d",
				dvipage, lastsearch, current, bMarkSearch);
		OutputDebugString(debugstr);
	}

	bEnableTermination = 0;		/* disable early termination */	/* debug */
/*	oldbCountZero = bCountZero; */

	GrabWidths();
/*	(void) scandvifile(hFile, hDC, 0);  */
/*	bCountZero = 0; */						/* need to fool it ! */
	(void) scandvipage(hFile, hDC, 0);		/* now scan only page of interest */
	ReleaseWidths(); 

/*	bCountZero = oldbCountZero;	 */
	bMarkSpotFlag = 0;

	if (bKeepFileOpen == 0) {
		if (hFile == HFILE_ERROR) {
			if (bDebug) {
				sprintf(debugstr, "%s (%s)\n", "File already closed", "hit");
				if (bDebug > 1) OutputDebugString(debugstr);
				else (void) wincancel(debugstr);
			}
		}
		else (void) _lclose (hFile);
/*		hFile = -1; */
		hFile = HFILE_ERROR; 
	}

	bHourFlag = 0;
	(void) SetCursor(hSaveCursor);
/*	(void) ReleaseDC(hWnd, hDC);	return 0;				*/ /* debug */
/*	sprintf(str, "h %ld v %ld", dvi_spot_h, dvi_spot_v); winerror(str); */
	
	if (dvi_spot_h == (long) oneinch  && dvi_spot_v == (long) oneinch) {
		if (bDebug) {
			sprintf(debugstr, "MISSED: dvipage %d lastsearch %ld current %ld\n", 
				dvipage, lastsearch, current);
/*			sprintf(str, "MISSED: dvipage %d lastsearch %ld current %ld\n\
lastdvipage %d startposition %ld pagestart %ld pagenumber %d wrapped %d\n\
buttonpos %ld", dvipage, lastsearch, current, lastdvipage, 
			startposition, pagestart, pagenumber, wrapped, buttonpos); */
			if (bDebug > 1) OutputDebugString(debugstr);
			else (void) wincancel(debugstr);
		}
	}
/*	else */
/*  now figure out whether need to move display data and where to put arrow */
		if (AdjustCursor(hWnd, hDC) > 0) redrawflag++;
		if (dvipage != olddvipage) redrawflag++;
/*		if ((bDebug > 1) && (dvipage == olddvipage))
			OutputDebugString("Same DVI Page\n");	*/
/*		end of fiddling with screen adjustments */
/*	} */

/*	sprintf(str, "ReleaseDC %d", hDC);
	winerror(str); */
	(void) ReleaseDC(hWnd, hDC);			/* ??? */
	bScanFlag = 0;			/* hack to prevent repaint 95/Dec/21 */
							/* do after ReleaseWidths and ReleaseDC */

	bMarkSearch = 0; 		/* ??? */

	if (redrawflag == 0) return 0;			/* no need to redraw */
	else {
		checkcontrols(hWnd, +1);
		SetupWindowText(hWnd);
		return -1;							/* indicate need to redraw */
	}
}

/* see whether user has hit a marked rectangular area */
/* need some work here to figure this out - similar to search  - or tagfont */

/* marknumber is -1 if nothing hit, 0 if hit labelled button, > 0 if number */

int checkmarkhit(HWND hWnd, int curx, int cury) {
	HDC hDC;
	POINT steparr[1];

/*	if (bBusyFlag != 0) return -1; */		/* 1993/March/29 */

	bUserAbort = 0;					/* 1993/March/29 ??? */

/*  need to reopen input file */
	if (ReOpenFile(hWnd) < 0) {
		return -1;		/* failed to reopen */
	}

	bScanFlag = 1;			/* hack to prevent repaint 95/Dec/21 */
							/* do before GrabWidths and GetDC */
	hDC = GetDC(hWnd);			
	if (bCopyFlag == 0)			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS);		/* set unit to twips */
	bHourFlag = 1;							/* 1995/Dec/21 */
	hSaveCursor = SetCursor(hHourGlass);	/* 1995/Dec/21 */
	bEnableTermination = 0;					/* Disable 1995/Dec/21 */
	bMarkFlag = 1;
	marknumber = -1;				/* nothing hit */

	GrabWidths();
/*	compute logical coordinates from window coordinates */
	steparr[0].x = curx;  steparr[0].y = cury;
	(void) DPtoLP(hDC, steparr, 1);
	tagx = steparr[0].x;  tagy = steparr[0].y; 
/*	sigh, may need to do both pages of two page spread */
	(void) scandvifile(hFile, hDC, 0);
	ReleaseWidths(); 

	bMarkFlag = 0;
	if (bKeepFileOpen == 0) {
		if (hFile == HFILE_ERROR) {
			sprintf(debugstr, "%s (%s)\n", "File already closed", "check mark hit");
			if (bDebug) {
				if (bDebug > 1) OutputDebugString(debugstr);
				else (void) wincancel(debugstr);
			}
		}
		else (void) _lclose (hFile);		/* 1992/Nov/6 */
/*		hFile = -1; */
		hFile = HFILE_ERROR;
	}
	bHourFlag = 0;							/* 1995/Dec/21 */
	(void) SetCursor(hSaveCursor);			/* 1995/Dec/21 */

	(void) ReleaseDC(hWnd, hDC);
	bScanFlag = 0;			/* allow repaint again */
							/* do after ReleaseWidths and ReleaseDC */
	return marknumber;						/* OK */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* BOOL bAddItem = 1; */		/* extra entry for adding an item */
/* BOOL bAddItem = 0; */		/* extra entry for adding an item */

BOOL bCallPreview = 1;			/* extra entry for calling preview */
BOOL bNewEnvVar = 1;			/* extra entry for setting up new var */

BOOL bMustExist = 0;	/* non-zero if file and path must exist 93/Dec/7 */

BOOL bAllowBlank = 0;	/* non-zero if CANCEL allowed in FileOpen 94/Jun/29 */

BOOL bCreateFile = 1;	/* create file if it does not exist */

/* BOOL bNoValidate = 1; */ /* Suppress validation TeX Menu GetOpenFileName */

char *texhelpfile="txhlp0.dvi";

char *texhelpdir="c:\\texhelp\\";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* HERE IS THE CODE FOR LINKING TO OTHER APPLICATIONS 1993/Dec/1 TeX menu */

/* this used to rely on checktexmenu to leave keys in str ... */

void filltexmenu (HWND hWnd) {
/*	HMENU hMenu; */
	HMENU hSubMenu;
	char *s, *sm;
	int k;
/*	int m; */
	char *keybuffer=NULL;				/* 97/June/5 */
/*	possible allocate memory for keybuffer ? */

	if (!bApplications) return;
	if (hTeXMenu == NULL) return;

	nApplications = 0;
#ifdef IGNORED
	hMenu = GetMenu(hWnd);
	m = GetMenuItemCount(hMenu);
	if (!bHelpAtEnd) hSubMenu = GetSubMenu(hMenu, m - 1);	/* last sub menu */
	else hSubMenu = GetSubMenu(hMenu, m - 2);	/* last sub menu */
#endif
	hSubMenu = hTeXMenu;
	if (bCallPreview) {
		strcpy(str, "Preview\t");
		strcat(str, szPreviewHotKey);
		AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
			IDM_APPLICATION + nApplications, str);
		nApplications++;
	}
	if (bCallPreview) {
		AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
		nApplications++;
	}
/*	start by reading in all of the keys in the [Applications] section */
	for (;;) {
		keybuffer = (char *) LocalAlloc(LMEM_FIXED, (UINT) nkeybuffer);
		if (keybuffer == NULL) {	/* unable to allocate memory */
			winerror ("Unable to allocate memory");
			return;
		}
		k = GetPrivateProfileString(achAp, NULL, "",
/*			keybuffer, sizeof(keybuffer), achFile); */
					keybuffer, nkeybuffer, achFile);
		if (k < nkeybuffer-2) break;		/* the keys fitted in */
		LocalFree ((HLOCAL) keybuffer);
		nkeybuffer = nkeybuffer * 2;		/* try again with larger buffer */
	}

	s = keybuffer;
	for (;;) {
		if ((sm = strchr(s, '|')) != NULL) *sm = '\t';	/* 93/Dec/25*/
		if (_strnicmp(s, "DummyEntry", 10) == 0) {	/* 97/July/12 */
		}
		else if (strncmp(s, "Separator", 9) == 0) {
			AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
			nApplications++;
		}
		else {
			AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
					   IDM_APPLICATION + nApplications, s);
			nApplications++;
		}
		s = s + strlen(s) + 1;
		if (*s == '\0') break;
	}
	if (keybuffer != NULL) LocalFree ((HLOCAL) keybuffer);
/*	bTeXMenuOK = 1; */		/* set once it has been filled */
	return;
}	/* hWnd unreferenced */

void cleartexmenu (HWND hWnd) {		/* clear the TeX menu */
/*	HMENU hMenu; */
	HMENU hSubMenu;
	int i, n;
/*	int m; */

	if (!bApplications) return;
	if (hTeXMenu == NULL) return;
#ifdef IGNORED
	hMenu = GetMenu(hWnd);
	m = GetMenuItemCount(hMenu);
	if (!bHelpAtEnd) hSubMenu = GetSubMenu(hMenu, m - 1);	/* last sub menu */
	else hSubMenu = GetSubMenu(hMenu, m - 2);	/* last sub menu */
#endif
	hSubMenu = hTeXMenu;
	n = GetMenuItemCount(hSubMenu);
	for (i = n-1; i >= 0; i--)	DeleteMenu(hSubMenu, i, MF_BYPOSITION);
}	/* hWnd unreferenced */

int checktexmenu (HWND hWnd) {	/* see whether menu agrees with ini file */
	int k, nmenu;
/*	int m; */
	int napp=0, flag=0;
/*	HMENU hMenu; */
	HMENU hSubMenu;
	char menustring[MAXKEY];
	char *keybuffer=NULL;				/* 97/June/5 */
/*	possible allocate memory for keybuffer ? */
	char *s, *sm;

	if (!bApplications) return 0;
	if (hTeXMenu == NULL) return 0;
/*	start by reading in all of the keys in the [Applications] section */
	for (;;) {
		keybuffer = (char *) LocalAlloc(LMEM_FIXED, (UINT) nkeybuffer);
		if (keybuffer == NULL) {
			winerror ("Unable to allocate memory");
			return -1;
		}
		k = GetPrivateProfileString(achAp, NULL, "",
/*				keybuffer, sizeof(keybuffer), achFile); */
						keybuffer, nkeybuffer, achFile);
		if (k < nkeybuffer-2) break;		/* the keys fitted in */
		LocalFree ((HLOCAL) keybuffer);
		nkeybuffer = nkeybuffer * 2;		/* try again with larger buffer */
	}
#ifdef IGNORED
	hMenu = GetMenu(hWnd);
	m = GetMenuItemCount(hMenu);
	if (!bHelpAtEnd) hSubMenu = GetSubMenu(hMenu, m - 1);	/* last sub menu */
	else hSubMenu = GetSubMenu(hMenu, m - 2);	/* last sub menu */
#endif
	hSubMenu = hTeXMenu;
	nmenu = GetMenuItemCount(hSubMenu);
	s = keybuffer;							/*	s = str; */
	napp = 0;
	if (bCallPreview) napp++;				/* step over first two */
	if (bCallPreview) napp++;				/* for separator */
	for (;;) {
		GetMenuString(hSubMenu, napp, menustring, sizeof(menustring),
			MF_BYPOSITION);
		if ((sm = strchr(menustring, '\t')) != NULL) *sm = '|';	/* 93/Dec/25*/
		if (strcmp(s, menustring) != 0) {	/* see if key matches */
			flag = 1; break;				/* no => mismatch */
		}
		s = s + strlen(s);
		napp++;
		if (*s == '\0' && napp == nmenu ) {	/* correct number of items ? */
			flag = 0; break;				/* yes, finish together */
		}
		if (*s == '\0' || napp == nmenu) {
			flag = 1; break;				/* no, one shorter than other */
		}
	}
	if (keybuffer != NULL) LocalFree ((HLOCAL) keybuffer);
	return flag;				/* returns non-zero if mismatch */
}	/* hWnd unreferenced */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Simpleminded bubblesort for env var names */
/* Should be efficient enough, since we typically have only 25-30 env vars */
/* Do we want to make this case insensitive? */

void sortkeys (char *keys[], int nenv) {
	int i, j, flag;
	char *s;
	for (i = 0; i < nenv-1; i++) {
		flag = 0;
		for (j = 0; j < nenv-i-1; j++) {
			if (strcmp(keys[j], keys[j+1]) > 0) {
				s = keys[j]; keys[j] = keys[j+1]; keys[j+1] = s; flag++;
			}
		}
		if (flag == 0) break;
	}
}

// Environment variables that should not be listed in the menu
// since these are handled elsewhere ...

int IgnoreEnv (char *s) {
	if (_stricmp(s, "DummyEntry") == 0) return 1;
	if (_stricmp(s, "DEBUGPAUSE") == 0) return 1;
	if (_stricmp(s, "Encoding") == 0) return 1;
	return 0;
}

/* New 1998/Aug/28 - support for [Environment] variable fiddling */

void fillenvmenu (HWND hWnd) {
	HMENU hSubMenu;
	char *s;
	int k, nenv;
	char *keybuffer=NULL;				/* 97/June/5 */
	char **keys=NULL;

	if (!bEnvironment) return;
	if (hEnvMenu == NULL) return;

	nEnvironment = 0;
	hSubMenu = hEnvMenu;
	if (bNewEnvVar) {
		strcpy(str, "[...NEW...]");
		AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
				   IDM_ENVIRONMENT + nEnvironment, str);
		nEnvironment++;
	}
	if (bCallPreview) {
		AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
		nEnvironment++;
	}

/*	start by reading in all of the keys in the [Environment] section */
	for (;;) {
		keybuffer = (char *) LocalAlloc(LMEM_FIXED, (UINT) nkeybuffer);
		if (keybuffer == NULL) {
			winerror ("Unable to allocate memory");
			return;
		}
		k = GetPrivateProfileString(achEnv, NULL, "",
									keybuffer, nkeybuffer, achFile);
		if (k < nkeybuffer-2) break;		/* the keys fitted in */
		LocalFree ((HLOCAL) keybuffer);
		nkeybuffer = nkeybuffer * 2;		/* try again with larger buffer */
	}

	s = keybuffer;
	nenv = 0;
	while (*s != '\0') {
/*		don't count "ENCODING"  "DEBUGPAUSE" "DummyEntry" */
		if (IgnoreEnv(s) == 0) nenv++;
		s = s + strlen(s) + 1;
	}
	if (nenv > 0) {
		keys = (char **) LocalAlloc(LMEM_FIXED, nenv * sizeof(char *));
		if (keys == NULL) {
			winerror ("Unable to allocate memory");
			return;
		}
		s = keybuffer;
		k = 0;
		while (*s != '\0') {
/*			don't include "ENCODING" */ /* don't include "DummyEntry" */
/*			don't count "ENCODING"  "DEBUGPAUSE" "DummyEntry" */
			if (IgnoreEnv(s) == 0) {
				keys[k++] = s;
				if (k >= nenv) break;				/* sanity check */			
			}
			s = s + strlen(s) + 1;
		}
		if (k < nenv) {
			if (bDebug > 1) {
				sprintf(debugstr, "k %d nenv %d", k, nenv);
				winerror(debugstr);
			}
			nenv = k;
		}
		if (bSortEnv) sortkeys(keys, nenv);
		for (k = 0; k < nenv; k++) {
			s = keys[k];
/*			if (_strnicmp(s, "DummyEntry", 10) == 0) continue; */
			AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
					   IDM_ENVIRONMENT + nEnvironment, s);
			nEnvironment++;
		}
	}
	if (keybuffer != NULL) LocalFree ((HLOCAL) keybuffer);
	return;
}	/* hWnd unreferenced */

#ifdef IGNORED
s = keybuffer;
while (*s != '\0') {
	AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
			   IDM_ENVIRONMENT + nEnvironment, s);
	nEnvironment++;
	s = s + strlen(s) + 1;
}
#endif

void clearenvmenu (HWND hWnd) {		/* clear the env menu */
	HMENU hSubMenu;
	int i, n;

	if (!bEnvironment) return;
	if (hEnvMenu == NULL) return;
	hSubMenu = hEnvMenu;
	n = GetMenuItemCount(hSubMenu);
	for (i = n-1; i >= 0; i--)	DeleteMenu(hSubMenu, i, MF_BYPOSITION);
}	/* hWnd unreferenced */

int checkenvmenu (HWND hWnd) {	/* see whether menu agrees with ini file */
	int k, nmenu;
	int napp=0, flag=0;
	HMENU hSubMenu;
	char menustring[MAXKEY];
	char *keybuffer=NULL;				/* 97/June/5 */
	char *s;

	if (!bEnvironment) return 0;
	if (hEnvMenu == NULL) return 0;

/*	start by reading in all of the keys in the [Environment] section */
	for (;;) {
		keybuffer = (char *) LocalAlloc(LMEM_FIXED, (UINT) nkeybuffer);
		if (keybuffer == NULL) {
			winerror ("Unable to allocate memory");
			return -1;
		}
		k = GetPrivateProfileString(achAp, NULL, "",
									keybuffer, nkeybuffer, achFile);
		if (k < nkeybuffer-2) break;		/* the keys fitted in */
		LocalFree ((HLOCAL) keybuffer);
		nkeybuffer = nkeybuffer * 2;		/* try again with larger buffer */
	}
	hSubMenu = hEnvMenu;
	nmenu = GetMenuItemCount(hSubMenu);
	s = keybuffer;							/*	s = str; */
	napp = 0;
	if (bNewEnvVar) napp++;				/* step over first two */
	if (bNewEnvVar) napp++;				/* for separator */

	for (;;) {
		GetMenuString(hSubMenu, napp, menustring, sizeof(menustring),
					  MF_BYPOSITION);
		if (strcmp(s, menustring) != 0) {	/* see if key matches */
			flag = 1; break;				/* no => mismatch */
		}
		s = s + strlen(s);
		napp++;
		if (*s == '\0' && napp == nmenu ) {	/* correct number of items ? */
			flag = 0; break;				/* yes, finish together */
		}
		if (*s == '\0' || napp == nmenu) {
			flag = 1; break;				/* no, one shorter than other */
		}
	}
	if (keybuffer != NULL) LocalFree ((HLOCAL) keybuffer);
	return flag;				/* returns non-zero if mismatch */
}	/* hWnd unreferenced */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*  hot key matches menu ? */
int translatehotkey (HWND hWnd, int nVKey, int control) {
	int nmenu, n;
/*	int m; */
	int napp=0;
/*	HMENU hMenu; */
	HMENU hSubMenu;
	char menustring[MAXKEY];
	char *sm;

	if (!bApplications) return 0;
	if (hTeXMenu == NULL) return 0;

/*	if (nVKey == VK_F1) {
		CallHelp(hWnd);
		return 0;
	} */									/* hard-wired to Help ? */
/*	if (nVKey == VK_F1 && control != 0) */
/*		return 1; */				/* C-F1 is hard wired to `Preview' */
/*  first check out preview */		/* just treat the same as all the rest ? */
/*	sm = szPreviewHotKey;
	if (control == 0 && *sm == 'F')
		n = atoi(sm+1);			
	else if (control != 0 && strncmp(sm, "C-F", 3) == 0)
		n = atoi(sm+3);			
	else n = 0;
	if (n == (nVKey - VK_F1 + 1)) return 1; */

#ifdef IGNORED
	hMenu = GetMenu(hWnd);
	m = GetMenuItemCount(hMenu);
	if (!bHelpAtEnd) hSubMenu = GetSubMenu(hMenu, m - 1);	/* last sub menu */
	else hSubMenu = GetSubMenu(hMenu, m - 2);	/* last sub menu */
#endif
	hSubMenu = hTeXMenu;
	nmenu = GetMenuItemCount(hSubMenu);
	napp = 0;
/*	if (bCallPreview) napp++;	*/			/* step over second */
	for (;;) {
		GetMenuString(hSubMenu, napp, menustring, sizeof(menustring),
			MF_BYPOSITION);
		if ((sm = strchr(menustring, '\t')) != NULL) {
			sm++;
			if (control == 0 && *sm == 'F')
				n = atoi(sm+1);		/* grab number */
			else if (control != 0 && strncmp(sm, "C-F", 3) == 0)
				n = atoi(sm+3);		/* grab number */
			else n = 0;
			if (n == (nVKey - VK_F1 + 1)) return napp; /* found it */
		}
		napp++;
		if (napp == nmenu ) break;
	}
	return -1;				/* returns negative if not found */
}	/* hWnd unreferenced */

/* NOTE: Also sets up value for this key in string str */

int findmenukey (HWND hWnd, char *name) {	/* find menu item if exist */
	int nmenu;
	int napp=0;
/*	int m; */
/*	int flag=0; */
/*	HMENU hMenu; */
	HMENU hSubMenu;
	char menustring[MAXKEY];
	char *s, *sm;
	int n;

	if (!bApplications) return 0;
	if (hTeXMenu == NULL) return 0;
	if ((s = strchr(name, '|')) != NULL) n = s - name;
	else n = strlen(name);

#ifdef IGNORED
	hMenu = GetMenu(hWnd);
	m = GetMenuItemCount(hMenu);
	if (!bHelpAtEnd) hSubMenu = GetSubMenu(hMenu, m - 1);	/* last sub menu */
	else hSubMenu = GetSubMenu(hMenu, m - 2);	/* last sub menu */
#endif
	hSubMenu = hTeXMenu;
	nmenu = GetMenuItemCount(hSubMenu);

	s = str;
	napp = 0;
	if (bCallPreview) napp++;				/* step over first two */
	if (bCallPreview) napp++;				/* for separator */
	for (;;) {
		GetMenuString(hSubMenu, napp, menustring, sizeof(menustring),
			MF_BYPOSITION);
		if ((sm = strchr(menustring, '\t')) != NULL) *sm = '|';
		if (strncmp(menustring, name, n) == 0 &&
			(menustring[n] == '\0' || menustring[n] == '|')) {
			n = GetPrivateProfileString(achAp, menustring, "",
				str, sizeof(str), achFile); 
			return n;
		}
		s = s + strlen(s);
		napp++;
		if (napp == nmenu) break;
	}
	return 0;				/* returns zero if failed */
}	/* hWnd unreferenced */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*  replace string at s of length ns with string at t length nt */
/*	remember to move/copy the terminating null of s also ... */
/*  returns 0 if it won't fit in buffer */ /* returns 1 if success */

/* void replacestring (char *s, int ns, char *t, int nt) */
int replacestring (char *s, int ns, char *t, int nt,
			char *buf, unsigned int nbuf) {
/*  check space available first */
	if (nt > ns) {			/* if replacement is larger than original */
		if (strlen (buf) + (nt - ns) >= nbuf) {
			sprintf(str, "Pattern too large:\n%s", buf);
			winerror(str);
			return 0;
		}
	}
	memmove (s + nt, s + ns, strlen (s + ns) + 1);
	memcpy (s, t, nt);
	return 1;
}
 
/* Replace string, but if it contains spaces, delimit with quotes "..." */
/* unless it is already delimited by "..." 2000 May 27 */

int replacestringspace (char *s, int ns, char *t, int nt,
			char *buf, unsigned int nbuf) {		/* 97/June/10 */
	if (strchr(t, ' ') == NULL)
		return replacestring(s, ns, t, nt, buf, nbuf);
	if (s > buf && *(s-1) == '\"' && *(s+ns) == '\"') {
		return replacestring(s, ns, t, nt, buf, nbuf);
	}
/*	somewhat inefficient, but should hardly ever happen */
	if (nt+2 > ns) {
		if (strlen (buf) + (nt+2 - ns) >= nbuf) {
			sprintf(str, "Pattern too large:\n%s", buf);
			winerror(str);
			return 0;
		}
	}
	replacestring(s, 0, "\"", 1, buf, nbuf);
	replacestring(s+1, ns, t, nt, buf, nbuf);
	replacestring(s+nt+1, 0, "\"", 1, buf, nbuf);
	return 1;
}

/* Used for szVector = grabenv("ENCODING"); and expandenv and TryWinExe */
/* and grabenv("YANDYPATH") in winprint.c for locating DVIPSONE and AFMtoTFM */
/* This assume str is available */  /* returns either str or DOS env var */
/* Will need to make copy using strdup for other than temporary use */
/* Returns NULL if not found ? */

char *grabenv (char *envvar) {			/* look in dviwindo.ini 1994/May/23 */
	if (bUseDVIWindo) {
		(void) GetPrivateProfileString(achEnv, envvar, "",
			str, sizeof(str), achFile);	
		if (*str != '\0') return str; /* drop through if not found */
	}
	return getenv(envvar);			/* else try and get from DOS env */
}

/* Deal with idiots who have white space in their environment variables! */
/* is this a safe thing to do now with file names that have spaces? */
/* conversely, should it be quoted if it does contain spaces? */

void stripspaces (char *str) {		/* 96/June/26 */
	char *s;
	int flag=0;
	s = str + strlen(str) - 1;		// work backwards from right end
	while (s >= str) {
		if (*s <= ' ') {
			if (flag++ == 0) {		/* show error first time in debug mode */
				if (bDebug) (void) wincancel(str);
			}
			if (*(s+1) == '\0')	strcpy(s, s+1);	// trailing spaces
			else if (s > str) {		// following on separator
				if (*(s-1) == ';' || *(s-1) == ',')
					strcpy(s, s+1);
			}

		}
		s--;
	}
}

/* Replace %<envvariable>% with value of environment variable */

int expandpercent (char *buf, unsigned int nbuf) {
	char environvar[BUFLEN];			/* needs to be this big? */
/*	char *se, *te, *t, *u; */
	char *se, *te;
	char *environvalue;
/*	int c, n, flag, first; */
	int n;

	if ((se = strchr(buf, '%')) == NULL) return 1;		/* nothing to do */
	if ((te = strchr(se+1, '%')) == NULL) {
		if (bDebug) (void) wincancel(buf);		/* show error 96/June/20 */
		return 0;						/* no match --- syntax error */
	}

	se = buf;
	while ((se = strchr(se, '%')) != NULL) {
		if ((te = strchr(se+1, '%')) == NULL) {
			if (bDebug) (void) wincancel(buf);		/* show error 96/June/20 */
			return 0;					/* no match --- syntax error */
		}
/*		if (*(te+1) > ' ') {		
			se = te + 1;
			continue;			
		} */	/* no, leave for expandpercent later if not white space */
		n = te - (se + 1);
		strncpy(environvar, se+1, n);
		*(environvar+n) = '\0';
/*		DOS generally believes only in upper case env vars */
		makeuppercase(environvar);					/* in winpslog.c */

/*		environvalue = getenv(environvar); */
		environvalue = grabenv(environvar);
		if (environvalue == NULL) {	/* environment variable not found */
			if (bDebug) (void) wincancel(buf);		/* show error 96/June/20 */
			strcpy(se, te+1);		/* just strip out the variable */
			continue;				/* could also call this an error? */
		}

/*		now replace the %environvar% with the value */ /* try and check */
/*		special case check on stuff like %TEXINPUTS%#*.tex 96/June/20 */
		if (*(te+1) == '#') stripspaces(environvalue);	/* 96/June/20 */
		if (replacestring(se, (te+1 - se), environvalue, strlen(environvalue),
			buf, nbuf) == 0) return 0;
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "replaced environment var: %s\n", buf);
			OutputDebugString(debugstr);
		}
#endif
	}
	return 1;
}

/* Replace * with file name currently open (minus extension) */
/* if no file open, replace * with ? (meaning prompt user) */
/* but don't expand if -* since that may be command line option DVIPSONE */

int expandasterisk (char *buf, unsigned int nbuf) {
	char *s, *t;			/* 97/Jan/14 */
	char *namestart;
/*	char *nameend; */
	int n;

	if (strchr(buf, '*') == NULL) return 1;	/* nothing to do */
	if (bFileValid == 0) {		/* if no file open, prompt for file name */
		s = buf;
/*		while ((s = strchr(s, '*')) != NULL) *s = '?'; */
		while ((s = strchr(s, '*')) != NULL) {
			if (s == buf || *(s-1) != '-')	/* avoid -* 97/Jan/14 */
				*s = '?';		/* replace * with ? */
			s++;				/* fix 97/Mar/7 */
		}
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "Replaced * with ?: %s\n", buf);
			OutputDebugString(debugstr);
		}
#endif
		return 1;				/* unable to replace * with file name */
	}
/*	ignore path, since DVI file may be in quite different place from source */
	namestart = removepath(OpenName);			/* in winspeci.c */
/*  if want fully qualified name, set namestart = OpenName here */
/*	ignore extension if any, since we don't want DVI extension */
/*	if ((s = strrchr(namestart, '.')) != NULL) nameend = s;
	else nameend = namestart + strlen(namestart);
	n = nameend - namestart; */
/*	if ((s = strrchr(namestart, '.')) != NULL) n = s - namestart; */
/*	made the following deal with things like ..\foo\bar 97/Jan/14 */
	if ((s = strrchr(namestart, '.')) != NULL &&
		((t = strrchr(namestart, '\\')) == NULL || t < s) &&
		((t = strrchr(namestart, '/')) == NULL || t < s))
		n = s - namestart;
	else n = strlen(namestart);
	s = buf;
/*	deal here with file name that has spaces in it ? */
	while ((s = strchr(s, '*')) != NULL) {
		if (s == buf || *(s-1) != '-') {	/* avoid -* 97/Jan/14 */
			if (strchr(s, ' ') != NULL) {	/* 97/Oct/23 */
				if (replacestringspace(s, 1, namestart, n, buf, nbuf) == 0)
					return 0;				/* won't fit in buffer give up */
			}
			else {
				if (replacestring(s, 1, namestart, n, buf, nbuf) == 0)
					return 0;				/* won't fit in buffer give up */
			}
		}
		s++;				/* fix 97/Mar/7 */
	}
/*	hack to get right file name in even though OpenFile Dialog not shown ??? */
/*	strcpy(SourceOpenName, ""); */
	strncpy(SourceOpenName, namestart, n);
	SourceOpenName[n] = '\0';
	strcat(SourceOpenName, SourceDefExt);	/* ??? */
/*	hack to get right file name in even though OpenFile Dialog not shown ??? */
	return 1;							/* succeeded */
}

/* replace @ with %TEXINPUTS%#* 1994/July/3 */
/* this makes menu entries look less intimidating */
/* what about format specific environment variables ??? */
/* how do we know what format is being used anyway ??? */

int expandatsign (char *buf, unsigned int nbuf) {
	char *s;
/*	char *namestart; */
/*	char *nameend; */
	int n;

	if (strchr(buf, '@') == NULL) return 1;	/* nothing to do */

	if (bQuoteAtSign) {					// 2000 May 27
		s = buf;
		while ((s = strchr(s, '@')) != NULL) {
//			don't do if already quoted
			if (s > buf && *(s-1) != '\"') {
				n = strlen(s);
				memmove(s+1, s, n+1);
				*s = '\"';
				while (*s > ' ') s++;
				n = strlen(s);
				memmove(s+1, s, n+1);
				*s = '\"';
			}
		}
	}

	s = buf;
	while ((s = strchr(s, '@')) != NULL) {
		if (replacestring(s, 1, "%TEXINPUTS%#*", 13, buf, nbuf)
			== 0) return 0;
	}
	return 1;							/* succeeded */
}

char *deslash (char *s) {
	char *name = s;
	if (name) {
		while (*s) {
			if (*s == '\\') *s = '/';
			s++;
		} /* endwhile */
	}
#ifdef DEBUGPATTERN
	if (bDebug > 1) {
		sprintf(debugstr, "Unixified name: %s\n", name);
		OutputDebugString(debugstr);
	}
#endif
	return name;
}

/* this was separated out 1994/Aug/18 */
/* semicolon separated list of directories in `environvalue' */
/* name of file in `FileName' */
/* resulting full path name written back into `testname' */
/* returns non-zero if it fails to find the file */

/* do we screw something up by replacing the _access ??? */

int getthepath (char *environvalue, char *filename, char *testname) {
	char *s;
#ifdef LONGNAMES
	HFILE hFile;		/* shows global hFile ? */
#endif
#ifndef SUBDIRSEARCH
	char *dirstar;
#ifndef LONGNAMES
	OFSTRUCT OfStruct;
#endif
#endif
	
	if (bCurrentFlag) {		/* try `current' (DVI) directory first ? */
		if (bFileValid || bUseSourcePath)
			strcpy(testname, DefPath);		/* use dir of DVI file */
		else {
/*			Will the following `DOS' code work in WIN32 ? */
			GetCurrentDirectory(sizeof(testname), testname);
		}

#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "current directory: %s\n", testname);
			OutputDebugString(debugstr);
		}
#endif
		s = (testname + strlen(testname) - 1);
		if (*s != '\\' && *s != '/') strcat(testname, "\\");	
		strcat(testname, filename);

#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "testname: %s\n", testname);
			OutputDebugString(debugstr); 
		}
#endif

/*	do we loose the sub-directory search capbility here ? */
/*	how does this compare with _access ??? */
/*	use LongOpenFile instead ???  No, has its own search...*/
#ifdef LONGNAMES			/* 95/Dec/1 */
/*		hFile = _lopen(testname, READ); */
		hFile = _lopen(testname, READ | OfExistCode);	/* 96/May/18 */
		if (hFile != HFILE_ERROR) {
			_lclose(hFile);
			hFile = HFILE_ERROR;
			return 0;		/* success */
		}
#else
#ifdef SUBDIRSEARCH
/*		check this can handle long file names ??? */
/*		ACCESSCODE = 4 means check for read permision */
		if (_access(testname, 4) == 0)
			return 0;	/* success */
#else
		if (OpenFile(testname, &OfStruct, OF_EXIST) != HFILE_ERROR) 
			return 0;		/* success */
#endif	/* not SUBDIRSEARCH */
#endif	/* not LONGNAMES */
	}

#ifdef SUBDIRSEARCH
	if (searchalongpath (filename, environvalue, testname, 0) != 0) 
		return -1;			/* it failed */
	else return 0;			/* success */
#else
	dirstar = environvalue;		/* step through and try paths in list */
	for (;;) {					/* try next directory in list */
		dirstar = nextpathname(testname, dirstar);
		s = (testname + strlen(testname) - 1);
		if (*s != '\\' && *s != '/') strcat(testname, "\\");	
		strcat(testname, filename);
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "next testname: %s\n", testname);
			OutputDebugString(debugstr); 
		}
#endif

#ifdef LONGNAMES
/*		hFile = _lopen(testname, READ); */
		hFile = _lopen(testname, READ | OfExistCode);	/* 96/May/18 */
		if (hFile != HFILE_ERROR) {
			_lclose(hFile);
			hFile = HFILE_ERROR;
			return 0;		/* success */
		}
#else
		if (OpenFile(testname, &OfStruct, OF_EXIST) != HFILE_ERROR) 
			return 0;		/* success */
#endif
		if (dirstar == NULL) /* end of the road ? - failed to find it */
			break;		/* failed */
	}						/* end of for(;;) env var search loop */
	return -1;		/* failed */
#endif

}

/* search for file matching specification along path given */

int expandhash (char *buf, unsigned int nbuf) {
	char filename[MAXFILENAME];
	char testname[MAXFILENAME];
	char *se, *te, *ue;
	char *environvalue;
	int c, n;
	char *s;						/* 96/Oct/12 */
	int flag, stripextension=0;		/* 96/Oct/12 */

#ifdef DEBUGPATTERN
	if (bDebug > 1) {
		sprintf(debugstr, "entering expandhash: %s\n", buf);
		OutputDebugString(debugstr);
	}
#endif
	if ((se = strchr(buf, '#')) == NULL) return 1;		/* nothing to do */
	se = buf;
	while ((se = strchr(se, '#')) != NULL) {
		if (*(se+1) == '?') {		/* does `file name' start with ? */
									/* strip out back to white space ? */
			se++;					/* step over it */
			continue;				/* and punt if filename unknown */
		}
		te = se;
//		while (*te > ' ' && te > buf)
		while ((c = *te) > ' ' && c != '\"' && te > buf) // 2000 May 21
			te--;	/* scan back to white space (or ") */
		if (te > buf) te++;
		environvalue = te;			/* semicolon delimited path */

		ue = se;
/*		change scan 97/Oct/23 will this create problems ? */
		while (*ue != '.' && *ue != '\0')
			ue++;	/* scan to file extension */
//		while (*ue > ' ')
		while ((c = *ue) > ' ' && c != '\"')
			ue++;	/* scan forward to white space (or ") */

		n = ue - (se+1);
		strncpy(filename, se+1, n);
		filename[n] = '\0';

#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "extracted filename: %s\n", filename);
			OutputDebugString(debugstr); 
		}
#endif

		*se = '\0';					/* delimit path list */
/*		first = bCurrentFlag; */	/* try current directory first ? */

/*		following split off 1994/Aug/18 */
/*		if (getthepath(environvalue, filename, testname) != 0) */
		flag = getthepath(environvalue, filename, testname);
/*		New: if no extension try with .tex extension, but strip off later */
		if (flag != 0) {					/* 1996/Oct/12 */
			if (strchr(filename, '.') == NULL) {
				stripextension = 1;
				strcat(filename, ".tex");	/* try .tex extension */
				flag = getthepath(environvalue, filename, testname);
			}
		}
		if (flag != 0) {
			*se = '#';				/* failed to find it - put hash back */
			*(se+1) = '?';			/* replace file name with ? */
			if ((ue = strchr(te, '.')) != NULL)	strcpy(se+2, ue);
			else if ((ue = strchr(te, ' ')) != NULL) strcpy(se+2, ue);
			else *(se+2) = '\0';
#ifdef DEBUGPATTERN
			if (bDebug > 1) {
				sprintf(debugstr, "failed: %s\n", buf);
				OutputDebugString(debugstr); 
			}
#endif
			return 1;		/* will prompt for file name later */
		}

/* succeeded - now replace the <path>#<file> with complete filename */
		*se = '#';
/* Now update SourceDefPath --- needed for next OpenFile Dialog */
/* ALSO needed if bUseSourcePath is non-zero 1993/Dec/10 */
		strcpy(SourceDefPath, testname);
		*removepath(SourceDefPath) = '\0';	/* terminate after separator */
		strcpy(SourceOpenName, removepath(testname));
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			OutputDebugString("Source Def Path: ");
			OutputDebugString(SourceDefPath);
		}
#endif
/*		moved down here so SourceDefPath is slash free */
/*		if (replacestring(te, (ue - te) + strlen(filename), */
		if (bUnixify) deslash (testname);				/* 1993/Dec/21 */
		if (stripextension) {							/* 1996/Oct/12 */
			if ((s = strrchr(testname, '.')) != NULL) *s = '\0';
		}
/*		quote the file name (to allow spaces in names) 97/Oct/23 */
		if (strchr(testname, ' ') != NULL) {			/* 97/Oct/23 */
			if (replacestringspace(te, (ue - te), testname, strlen(testname),
								   buf, nbuf) == 0) return 0;
		}
		else {
			if (replacestring(te, (ue - te), testname, strlen(testname),
							  buf, nbuf) == 0) return 0;
		}
		se = te;
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "replaced with filename: %s\n", buf);
			OutputDebugString(debugstr); 
		}				
#endif
	}
	return 1;
}

// Common Dialog box stuff for File Open Dialog 
// called to fill in ? in expanding TeX Menu item

#define FILEOPENORD      1536			/* in dlgs.h */

/* char *getuserfilename(HWND hWnd, char *filename) */
char *getuserfilename (HWND hWnd, char *filename, char *key) {
									/* do hairy stuff here later */
	OPENFILENAME ofn;
	char szDirName[MAXFILENAME];
	char szFile[MAXFILENAME];
	char szFileTitle[MAXFILENAME];	/* on output file name minus path */
/*	char szFilter[MAXFILENAME]; */
	char szFilter[256];				/* 1994/Feb/10 */
/*	char szExt[] = "tex"; */		/* default extension */
/*	char *szExt = "tex"; */			/* default extension */
	char szTitle[MAXKEY]="Select File";
	UINT i, cbString;
	char chReplace;			/* string separator for szFilter */
/*	HFILE hf; */
	int flag;
	char *s;

	if (bUsingCommDlg && bAllowCommDlg) {
		if (bDebug > 1) {   
			sprintf(debugstr, "SourceDefPath '%s' SourceDefSpec '%s' SourceDefExt '%s'\n",
				SourceDefPath, SourceDefSpec, SourceDefExt); 
			OutputDebugString(debugstr);
		} 

/*		removeback(SourceDefPath); */		/* 1993/Dec/9 */
		ChangeDirectory(SourceDefPath, 1);	/* change directory and drive */
/*		trailingback(SourceDefPath); */		/* 1993/Dec/9 */

		if (key != NULL) {						/* 1994/Feb/25 */
			strcpy(szTitle, key);
			if ((s = strchr(szTitle, '|')) != NULL) *s='\0';
		}

/*		if we validate file names ourselves we need to register message */
/*		if (bNoValidate) uFileOKMsg = RegisterWindowMessage(FILEOKSTRING); */

/*		May now be in wrong directory for application ! 1993/Dec/9 */

/*		GetSystemDirectory(szDirName, sizeof (szDirName)); */
		strcpy(szDirName, filename);		/* copy directory from file name */
		if ((s = strrchr(szDirName, '\\')) != NULL) ;
		else if ((s = strrchr(szDirName, '/')) != NULL) ;
		else if ((s = strrchr(szDirName, ':')) != NULL) s++;
		else s = szDirName + strlen(szDirName);
		*s = '\0';							/* overrridden later */
/*		szFile[0] = '\0'; */
		strcpy(szFile, removepath(filename));
/*		Now set up file type selection string */
		if ((cbString = LoadString(hInst, IDS_FILTERSTRING, szFilter,
								   sizeof(szFilter))) == 0) {
			strcpy(szFilter, "TeX files(*.tex)|*.tex|");
		}
		chReplace = szFilter[cbString - 1];	/* retrieve wild character */
		for (i = 0; szFilter[i] != '\0'; i++) 
			if (szFilter[i] == chReplace)	szFilter[i] = '\0';

		memset(&ofn, 0, sizeof(OPENFILENAME));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = szFilter;
/*		ofn.nFilterIndex = 1; */
		ofn.nFilterIndex = TeXFilterIndex;
		ofn.lpstrFile = szFile;			/* have enough space for long name */
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFileTitle = szFileTitle;
		ofn.nMaxFileTitle = sizeof(szFileTitle);

/*		ofn.lpstrTitle = "Select File"; */
		ofn.lpstrTitle = szTitle;			/* 1994/Feb/25 */
/*		ofn.lpstrInitialDir = szDirName; */
/*		ofn.lpstrInitialDir = SourceDefPath; */	/* 1993/Dec/9 */
/*		ofn.lpstrDefExt = szExt; */		/* default extension */
/*		ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; */
		ofn.Flags =  OFN_HIDEREADONLY;
/*		default is not to set this flags - but it doesn't help ? */
		if (bMustExist) ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		else if (bCreateFile) ofn.Flags |= OFN_CREATEPROMPT; /* 97/March/29 */

/*		if (bNoValidate) */	/* 1996/Aug/18 */
/*			ofn.Flags != OFN_NOVALIDATE; */
/*			ofn.lpfnHook = OpenFileHook; */
/*			ofn.Flags |= OFN_ENABLEHOOK; */
/*			need also OFN_EXPLORER if using new style dialog box */
/*			*and* a more complicated hook procedure ! */
/*		} */

/*		Use our own template with longer file list */
/*		except in new shell in Windows 95 where we have long file names */
		if (bHiresScreen && (bNewShell == 0)) 

/*		if (bHiresScreen)  */
		{
			ofn.hInstance = hInst;
			ofn.Flags = ofn.Flags | OFN_ENABLETEMPLATE;
			ofn.lpTemplateName = MAKEINTRESOURCE(FILEOPENORD);
		}
#ifdef LONGNAMES
		if (bLongNames) {	/* has no effect with Explorer style dialog ... */ 
			ofn.Flags = ofn.Flags | OFN_LONGNAMES; /* 95/Dec/5 */
/*			if (bDebug > 1) OutputDebugString("OFN_LONGNAMES\n"); */
		}
#endif

		flag = GetOpenFileName(&ofn);	/* WINVER == 0x300 problem ? */

		if (flag == 0) {				/* no file specified */
			if (bDebug > 1)	OutputDebugString("GetOpenFileName returned zero\n");
			if (bAllowBlank) return "";	/*  leave it blank - 1994/June/29 */
			else return NULL;	/* user cancelled - don't go on - 1993/Dec/7 */
		}
		else {
			if (bDebug > 1) {
				TeXFilterIndex = ofn.nFilterIndex;		/* remember 95/July/6 */
				sprintf (debugstr, "lpstrFile %s nFileOffset %d nFileExtension %d",
						 ofn.lpstrFile, ofn.nFileOffset, ofn.nFileExtension);
				OutputDebugString(debugstr);
			}
/*			New special case, a way to indicate don't want to specify file */
			if (lstrcmp(ofn.lpstrFile + ofn.nFileOffset, "!") == 0)
				return "";	/* 94/July/11 */
			if (lstrcmp(ofn.lpstrFile + ofn.nFileOffset, "nul") == 0)
				return "";	/* 95/July/6 */
/*			lopen(ofn.lpstrFile, OF_READ); */
/*			lstrcpy (SourceOpenName, ofn.lpstrFile); */	/* 1993/Dec/9 */
			lstrcpy (SourceOpenName, ofn.lpstrFile + ofn.nFileOffset);
/*			following added 1993/Dec/9 */
			if (ofn.nFileExtension > 0) {
/*				retrieve extension for future reference ".tex" */
				SourceDefExt[0] = '.';
				lstrcpy (SourceDefExt+1, ofn.lpstrFile + ofn.nFileExtension);
/*				retrieve default file specification for future reference "*.tex" */
				SourceDefSpec[0] = '*';
				SourceDefSpec[1] = '.';
				lstrcpy (SourceDefSpec+2, ofn.lpstrFile + ofn.nFileExtension);
			}
			else {									/* 98/Sep/14 */
				SourceDefExt[0] = '\0';
				SourceDefSpec[0] = '*';
				SourceDefSpec[1] = '\0';
			}
/*			retrieve path for future reference */
			lstrcpy (SourceDefPath, ofn.lpstrFile);
			*(SourceDefPath + ofn.nFileOffset) = '\0';
			if (bDebug > 1) {
				sprintf(debugstr,
						"lpstrFile '%s' nFileOffset %d nFileExtension %d",
						ofn.lpstrFile, ofn.nFileOffset, ofn.nFileExtension);
				OutputDebugString(debugstr);
				sprintf(debugstr,
						"OpenName '%s', DefExt '%s', DefSpec '%s', DefPath '%s'\n",
						SourceOpenName, SourceDefExt, SourceDefSpec, SourceDefPath);
				OutputDebugString(debugstr);
			}
		}
/*		return SourceOpenName; */
/*		is it safe to construct the name in str ? */
		strcpy(str, SourceDefPath);			/* 1993/Dec/9 */
		s =  SourceDefPath + strlen(SourceDefPath) - 1;
		if (*s != '\\' && *s != '/') strcat (str, "\\");
		strcat(str, SourceOpenName);
		if (!bMustExist && bCreateFile) {		/* 97/Mar/29 */
			HFILE hFile;
			hFile = _lopen(str, READ | OfExistCode);	/* does file exist? */
			if (hFile == HFILE_ERROR) {		/* if not, create an empty one */
/*				hFile = _lopen(str, WRITE); */
				hFile = _lcreat(str, 0);
				if (hFile == HFILE_ERROR) {
					DWORD err;
					err = GetLastError();

/*					err = errno; */
					sprintf(debugstr, "Unable to create file %s (%d)\n",
							str, errno);
					(void) ExplainError(debugstr, err, 1);
/*					winerror(debugstr); */
				}
			}
			_lclose(hFile);
		}
		if (bUnixify) deslash(str);				/* 94/Dec/31 ??? */
		return str;
	}		/* end of CommDlg method for file selection */

	else {						/* do it the old Windows 3.0 way */
		strcpy(SourceOpenName, filename);
		flag = DialogBoxParam(hInst, "FileSelect", hWnd, FileDlg, 1L);
						/* indicate  `Call Application' mode */
/*		if (flag == 0) return filename; */
		if (bUnixify) deslash(SourceOpenName);			/* 1994/Dec/31 ??? */
		if (flag <= 0) return NULL;
		else return SourceOpenName;
	}
}

void stripextension (char *name) {
	char *s, *t;
	if ((s = strrchr(name, '\\')) != NULL) s++;
	else if ((s = strrchr(name, '/')) != NULL) s++;
	else if ((s = strrchr(name, ':')) != NULL) s++;
	else s = name;
	if ((t = strrchr(s, '.')) != NULL) *t = '\0';
}

// copy string up to next white space
// or "..." 2000 May 21
// hence now somewhat of a misnomer ...

void copyspacedelimited (char *s, char *t) {
	int c;
//	char *so=s;		// debugging only
	char *te = t + strlen(t) - 1;
	if (*t == '\"') {				// form "..." or "...
		t++;		// 2000 May 21
		while ((c = *t++) != '\0' && c != '\"')	// 2000 May 21
			*s++ = (char) c;
	}
	else if (*te == '\"') {			// form ..."
		while ((c = *t++) != '\0' && c != '\"')	// 2000 May 21
			*s++ = (char) c;
	}
	else {							// form ... 
//		while ((c = *t++) > ' ')		/* ends on white space */
		while ((c = *t++) > ' ' && c != '\"')	// 2000 May 21
			*s++ = (char) c;
	}
	*s= '\0';						/* end string copied */
//	wincancel(so);		// debugging only
}

/* prompt user to supply arguments */ /* form: "Title|Default" */
/* what about quotes used to delimit file name with spaces in it ??? */

int expandquotes (HWND hWnd, char *buf, unsigned int nbuf) {
	char *se, *te;
/*	char *sq, *tq; */
	int n, m, flag;

#ifdef DEBUGPATTERN
	if (bDebug > 1) {
		sprintf(debugstr, "entering expandquotes: %s\n", buf);
		OutputDebugString(debugstr);
//		wincancel(debugstr);	// debugging only
	}
#endif

	if ((se = strchr(buf, '\"')) == NULL) return 1;	/* no ", nothing to do */
	if ((te = strchr(se+1, '\"')) == NULL) {	/* is there another " ? */
		if (bDebug) (void) wincancel(buf);		/* show error 96/June/20 */
		return 0;						/* mismatch --- syntax error */
	}
//	if ((sb = strchr(se, '|')) == NULL || sb >= te)
//		return 0;	// ignore quoted material without | 2000 May 21
	se = buf;
/*	keep going until no more "..." or "...|..." strings */
	while ((se = strchr(se, '\"')) != NULL)  {
		if ((te = strchr(se+1, '\"')) == NULL) {
			if (bDebug) (void) wincancel(buf);		/* show error 96/June/20 */
			return 0;					/* mismatch --- syntax error */
		}
//		if ((sb = strchr(se, '|')) == NULL || sb >= te) {
//			se = te + 1;	// step over it
//			continue;		// ignore quoted material without | 2000 May 21
//		}
		n = (te - (se+1));		/* may be zero if empty string */
		strncpy(str, se+1, n);
		str[n] = '\0';
		if (strchr(str, '|') != NULL) {		/* only if "...|..." 96/Oct/12 */
#ifdef DEBUGPATTERN
			if (bDebug > 1) {
				OutputDebugString(str);
//				wincancel(debugstr);	// debugging only
			}
#endif
			flag = DialogBoxParam(hInst, "FileSelect", hWnd, FileDlg, 2L);
								/* indicate  `Supply Parameter' mode */
/*			if (flag == 0) strcpy (sq, tq+1);*/	/* just flush the string*/
			if (flag <= 0) return 0;	/* failed - user cancelled */
/*			check whether result should be quoted ... 96/Jan/28 */
/*			look for characters that DOS may interpret on the command line */
/*			not sure this will work ... i.e., will the " be stripped again ? */
			if (strpbrk(str, " |<>=&^") != NULL) {
				memmove(str+1, str, strlen(str)+1);
				*str = '\"';
				strcat(str, "\"");
			}
/*			if (bDebug > 1) {
				OutputDebugString("User supplied: ");
				OutputDebugString(str);
			} */			/* debugging */
			m = strlen(str);
			if (replacestring(se, n+2, str, strlen(str), buf, nbuf) == 0)
				return 0;		/* error, did not fit in buffer */
			se = se + m;		/* avoid looking at "..." again 96/Jan/28 */
		}
		else se = te + 1;		/* step over "..." without change 96/Oct/12 */
/*		if (bDebug > 1) {
			OutputDebugString("Rest of string: ");
			OutputDebugString(se);
		} */		/* debugging */
	}
#ifdef DEBUGPATTERN
	if (bDebug > 1) {
		sprintf(debugstr, "leaving expandquotes: %s\n", buf);
		OutputDebugString(debugstr);
//		wincancel(debugstr);	// debugging only
	}
#endif
	return 1;					/* succeeded */
}

/* prompt user for file name */ /* either because ? or because shift key */

/* int expandquestion (HWND hWnd, char *buf, unsigned int nbuf, int shift) */
int expandquestion (HWND hWnd, char *buf, unsigned int nbuf,
		int shiftflag, char *key) {
	char filename[MAXFILENAME]; 
	char *s, *t, *sq, *tq, *uq;
	char *newfile;

#ifdef DEBUGPATTERN
	if (bDebug > 1) {
		sprintf(debugstr, "entering expandquestion: %s\n", buf);
		OutputDebugString(debugstr);
//		wincancel(debugstr);	// debugging only
	}
#endif

	if (strchr(buf, '?') == NULL) {			/* no ? on the command line */
		if (shiftflag) {					/* but shift key pressed */
/*	if shift key is pressed prompt for file name using what is constructed */
/*	assumes the file to be prompted for is the last thing on the line ! */
			s = buf + strlen(buf) - 1;		// 2000 May 21
			if (*s == '\"' && s > buf) {
				s = strrchr(s-1, '\"');		// allow for "..."
			}
			else s = strrchr(buf, ' ');		// last white space 
//			if ((s = strrchr(buf, ' ')) != NULL) /* last white space */
			if (s != NULL) {
				copyspacedelimited(filename, s+1);
/*				newfile = getuserfilename(hWnd, filename); */
				newfile = getuserfilename(hWnd, filename, key);
				if (newfile == NULL) return 0;	/* user cancelled */
				if (bStripPath) t = removepath(newfile);	/* 96/Oct/4 */
				else t = newfile;
/*				quote the file name (to allow spaces in names) 97/Oct/23 */
				if (strchr(newfile, ' ') != NULL) {	/* 97/Oct/23 */
					if (replacestringspace(s+1, strlen(s+1), t, strlen(t),
										   buf, nbuf) == 0) return 0;
				}
				else {
					if (replacestring(s+1, strlen(s+1), t, strlen(t),
									  buf, nbuf) == 0) return 0;
				}
				if (bUnixify) deslash (s+1);	/* 1994/Jan/27 */
			}
			else {	/* no ' ', only one item on the line - append file name */
				strcpy(filename, SourceOpenName);
/*				newfile = getuserfilename(hWnd, filename); */
				newfile = getuserfilename(hWnd, filename, key);
				if (newfile == NULL) return 0;	/* used cancelled */
				strcat (buf, " ");
/*				strcat (buf, newfile); */ /* need space checking ... */
				s = buf + strlen(buf);
/*				if (replacestring(buf + strlen(buf), 0, newfile, strlen(newfile), */
				if (bStripPath) t = removepath(newfile);	/* 96/Oct/4 */
				else t = newfile;
/*				if (replacestring(s, 0, newfile, strlen(newfile), */
				if (replacestring(s, 0, t, strlen(t),
					buf, nbuf) == 0) return 0;
				if (bUnixify) deslash (s);	/* 1994/Jan/27 */
			}
			return 1;
		}
		else return 1;	/* nothing to do no ? and no shift key */
/*		can't drop through here ... */
	} /* end of if (strchr(buf, '?') == NULL) */

/*	get here if there is at least one `?' on the line */
/*	advance to end of field and retreat to beginning wrt white space */
	sq = buf;
	while ((sq = strchr(sq, '?')) != NULL)  {
		uq = sq;
//		while (*uq > ' ') uq++;			/* advance to end of field */
		while (*uq > ' ' && *uq != '\"')
			uq++;	/* advance to end of field - 2000 May 21 */
		tq = sq;
//		while (*tq > ' ' && tq > buf) tq--;	/* retreat to beg of field */		
		while (*tq > ' ' && tq > buf && *tq != '\"')
			tq--;	/* retreat to beg of field - 2000 May 21 */
		tq++;
/*		use previous application field name ? */
		if (*SourceOpenName != '\0') {
			strcpy(filename, SourceOpenName);
			{
				sprintf(debugstr, "filename '%s' sq '%s' SourceDefExt '%s'",
						filename, sq, SourceDefExt);
//				wincancel(debugstr);		// debugging only
			}
			if (*(sq+1) == '.') {		/* but modify extension ? */
/*				if ((s = strrchr(filename, '.')) != NULL) strcpy(s, sq+1); */
/*				strncpy(SourceDefExt, sq+1, 4); */	/* new default extension ? */
				strcpy(SourceDefExt, sq+1); 	/* new default extension ? */
				s = SourceDefExt + strlen(SourceDefExt) - 1;
				if (*s == '\"') *s-- = '\0';	// 2000 May 21
//				while (*(--s) <= ' ' && s > SourceDefExt) ;
				while ((*s <= ' ' || *s == '\"') && s > SourceDefExt)
					*s-- = '\0';				// 2000 May 21
				*(s+1) = '\0';					/* 99/Jan/3 */
				if ((s = strrchr(filename, '.')) != NULL)
					strcpy(s, SourceDefExt);	// ???
			}
		}
		else {
			if (bFileValid != 0) {	/* if file open, use as starting point */
/*				Will the following `DOS' code work in WIN32 ? */
				GetCurrentDirectory(sizeof(filename), filename);
				if (*(filename + strlen(filename) -1) != '\\')
					strcat(filename, "\\");
				strcat(filename, removepath(OpenName));	
				stripextension(filename);		/* flush extension */
#ifdef DEBUGPATTERN
				if (bDebug > 1) {
					sprintf(debugstr, "constructed: %s\n", filename);
					OutputDebugString(debugstr);
//					wincancel(debugstr);	// debugging only
				}
#endif
			}
			else {
/*				Will the following `DOS' code work in WIN32 ? */
				GetCurrentDirectory(sizeof(filename), filename);
				if (*(filename + strlen(filename) - 1) != '\\')
					strcat(filename, "\\");
				strcat(filename, "*");					/* ? */
			}
/*			attach specified extension to guess */
			copyspacedelimited(filename+strlen(filename), sq+1);
		}
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "offering to getuserfilename: %s\n", filename);
			OutputDebugString(debugstr);
//			wincancel(debugstr);	// debugging only
		}
#endif
/*		newfile = getuserfilename(hWnd, filename); */
		newfile = getuserfilename(hWnd, filename, key);
		if (newfile == NULL) return 0;	/* user cancelled */
		if (bStripPath) t = removepath(newfile);	/* 96/Oct/4 */
		else t = newfile;
/*		if (replacestring(tq, uq - tq, newfile, strlen(newfile), */
		if (replacestring(tq, uq - tq, t, strlen(t),
						  buf, nbuf) == 0) return 0;
/*		s not initialized ??? use tq ??? instead */
/*		if (bUnixify) deslash (s); */			/* 1995/Jan/13 */
		if (bUnixify) deslash (tq);				/* 1998/Mar/26 ??? */
/*		replacestring(sq, tq - sq -1, newfile, strlen(newfile)); */
/*		if (*(sq-1) == '%') removeenviron(buf, sq); */
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "present state: %s\n", buf);
			OutputDebugString(debugstr);
//			wincancel(debugstr);	// debugging only
		}
#endif
		sq = tq;
	}
#ifdef DEBUGPATTERN
	if (bDebug > 1) {
		sprintf(debugstr, "leaving expandquestion: %s\n", buf);
		OutputDebugString(debugstr);
//		wincancel(debugstr);	// debugging only
	}
#endif
	return 1;							/* succeeded */
}

HWND hSwitchWindow;

/* Modified 99/Mar/17 to allow leading ... ellipsis if flag is non-zero */

BOOL CALLBACK _export EnumSwitchProc(HWND hwnd, LPARAM lParam) {
	char buffer[128];
	int flag = lParam;

	GetWindowText(hwnd, buffer, sizeof(buffer)); 
	if (flag == 0) {			/* compare start of title with given string */
		if (strncmp (buffer, str, strlen(str)) == 0) {
			hSwitchWindow = hwnd;
			return FALSE;		/* got it, stop enumerating */
		}
		else return TRUE;		/* not yet, continue enumerating */
	}
	else {						/* see whether title contains given string */
		if (strstr (buffer, str) != NULL) {	
			hSwitchWindow = hwnd;
			return FALSE;		/* got it, stop enumerating */
		}
		else return TRUE;		/* not yet, continue enumerating */
	}
}	/* lParam used for flag 0 => FooBar... format, while 1 => ...FooBar format */

/* WARNING: uses string str */

HWND GetSwitchWindow (char *szTitle, int flag) {
	strcpy (str, szTitle);
	hSwitchWindow = NULL;
	(void) EnumWindows((WNDENUMPROC) EnumSwitchProc, flag);
	return hSwitchWindow;
}

int switchtowindow (HWND hWnd, char *buf, int shift) {
	char *s, *t, *u, *v;
	HWND hWindow;

	if ((s = strchr(buf, '$')) == NULL) return 0;
	s++;
	if (strncmp(s, "Window", 6) == 0) {
		if ((t = strchr(s, '(')) == NULL) return 0;
		t++;
		if ((u = strchr(t, ')')) == NULL) return 0;
		*u = '\0';								/* terminate Window title */
		if ((v = strstr(t, "...")) == NULL) {	/* does not have ellipsis */
/*			try for *exact* match with Window Title */
			hWindow = FindWindow (NULL, t);
		}
		else {							/* does have ellipsis */
			if (v == t) {				/* starts with ellipsis */
/*				try to find given string in Window Title */
				hWindow = GetSwitchWindow (v+3, 1);	/* WARNING: uses string str */
			}
			else {						/* ends in ellipsis */
				*v = '\0';				/* strip off trailing ellipses */
/*				try to match start of Window Title against given string */
				hWindow = GetSwitchWindow (t, 0);	/* WARNING: uses string str */
				*v = '.';				/* put it back (for debug output) */
			}
		}
		if (hWindow != NULL) {
			if (SetForegroundWindow(hWindow) == 0) {
				(void) MessageBox(hWnd, "SetForegroundWindow failed",
						modname,  MB_ICONINFORMATION | MB_OK);
			}

/* Note: in WIN32 can't get SetFocus to Window in other thread */

/*			if (SetFocus (hWindow) == NULL) {
				if (bDebug > 1) OutputDebugString("SetFocus failed\n");
				shift = 1;
			}
			if (shift) {
				if (SetActiveWindow (hWindow) == NULL) {
					(void) MessageBox(hWnd, "SetActiveWindow failed",
						modname,  MB_ICONINFORMATION | MB_OK);
				}
			} */

			*u = ')';				/* put it back (for debug output) */
			return 1;
		}
		else {
			sprintf(str, "Can't find Window `%s' to activate", t); 
			(void) MessageBox(hWnd,	str, modname,
				MB_ICONEXCLAMATION | MB_OK); 
			*u = ')';				/* put it back (for debug output) */
			return 0;
		}
	}
	return 0;	/* ? */
}	/* shift unreferenced */

/* Tries key and replaces with number if it matches */
/* dlr points at leading dollar sign - part of what gets replaced */
/* returns -1 if it did not match */
/* returns 1 if it matched and did fit */
/* returns 0 if it won't fit */

int trynamed (char *dlr, char *key, long num, char *buf, int nbuf) {
	char number[16];
	int n = strlen(key);
	if (strncmp(dlr+1, key, n) != 0) return -1;
	sprintf(number, "%ld", num);
	return replacestring(dlr, n+1, number, strlen(number), buf, nbuf);
}

/* returns non-zero if it was `switchtowindow' --- so there is no WinExec */

int expanddollars (HWND hWnd, char *buf, int nbuf, int shift) {
/*	char number[16]; */
	int flag, n, nc;
	char *s;

	s = buf;
	while ((s = strchr(s, '$')) != NULL) {
/*		s++; */
		if (*(s+1) == '$') {	/* $$ gives a real $ (escape mechanism) */
			strcpy(s, s+1);		/* remove one $ */
			s++;				/* step over it */
			continue;
		}
		if (strncmp(s+1, "Window", 6) == 0) {
			switchtowindow(hWnd, buf, shift);
			return -1;						/* suppress WinExec */
		}
		if (strncmp(s+1, "Comment", 7) == 0) {	/* comment passed by TeX */
			*s = '\"';						/* put between " ? */
			if (replacestring(s+1, 7, dvi_comment, strlen(dvi_comment)+1, buf,
							  nbuf) == 0) 
				return 0;
			s += strlen(dvi_comment)+1;
			*s = '\"';						/* put between " ? */
			continue;
		}
		if (strncmp(s+1, "Font", 4) == 0 && *(s+1+4) != 's') {
			if (replacestring(s, 4+1, TestFont, strlen(TestFont),
							  buf, nbuf) == 0) 
				return 0;
			s += strlen(TestFont);
			continue;
		}
		if (strncmp(s+1, "Counter", 7) == 0) {
			if (sscanf(s+1+7,"[%d]%n", &nc, &n) == 1) {
				if (nc >= 0 && nc < TEXCOUNTERS) {
					sprintf(str, "%ld", counter[nc]);
					if (replacestring(s, 7+1+n, str, strlen(str), buf, nbuf)
						== 0) 
						return 0;
					continue;
				}
			}
		}
/*		Possible place for $File or $Name 96/Oct/4 */
		flag = trynamed(s, "Page", (long) dvipage, buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
/* following is redundant - use Counter[0] */
		flag = trynamed(s, "Logical", counter[0], buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
		flag = trynamed(s, "Last", (long) dvi_t, buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
/*		do we really want all of the following ??? */
		flag = trynamed(s, "Magnification", (long) dvi_mag, buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
		flag = trynamed(s, "Width", (long) dvi_u, buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
		flag = trynamed(s, "Height", (long) dvi_l, buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
		flag = trynamed(s, "Fonts", (long) dvi_fonts, buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
		flag = trynamed(s, "Stack", (long) dvi_s, buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
		flag = trynamed(s, "Byte", current, buf, nbuf);	/* bop ? */
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
		flag = trynamed(s, "Length", (long) dvi_bytes, buf, nbuf);
		if (flag == 0) return 0;			/* no space left */
		if (flag > 0) continue;				/* matched and replaced */
/*		else try for environment variable 97/June/5 */
/*		t = grabenv(s); */ /* no, just use %Variable% instead */
		if (bDebug) (void) wincancel(buf);			/* show error 96/June/20 */
/*		winerror("Don't understand call pattern"); */
/*		winerror(buf); */
/*		break;*/
/*		worry about getting stuck in loop if the $ is not replaced */
		if (*s == '\0') break;				/* sanity escape */
		if (*s == '$') s++;					/* allow nonsense $ */
	}
	return 0;			/* normal return - no window switch */
}

int isdvipsonecallflag;		/* Set if WinExe is used to call DVIPSONE 00/Apr/3 */
int istexcallflag;			/* Set if WinExe is used to call TeX 98/Jun/28 */

char *IsDVIPSONECall (char *buf) {
	char *s;
	if ((s = strstr(buf, "dvipsone.bat")) != NULL ||
		  (s = strstr(buf, "dvipsone.exe")) != NULL ||
		  (s = strstr(buf, "DVIPSONE.BAT")) != NULL ||
		  (s = strstr(buf, "DVIPSONE.EXE")) != NULL) {
		return s;	/* past end is s+12 */
	}
	return NULL;
}

char *IsTeXCall (char *buf) {
	char *s;
	if ((s = strstr(buf, "tex.bat")) != NULL ||
		  (s = strstr(buf, "tex.exe")) != NULL ||
		  (s = strstr(buf, "TEX.BAT")) != NULL ||
		  (s = strstr(buf, "TEX.EXE")) != NULL) {	/* includes yandytex.exe */
		if (strstr(buf, "bibtex") == NULL &&
			  strstr(buf, "BIBTEX") == NULL) {	/* but excludes bibtex.exe */
			return s;	/* past end is s+7 */
		}
	}
	return NULL;
}

/* interpret and fill pattern */
/* int parseapplication (HWND hWnd, char *buf, unsigned int nbuf, int shift) */
int parseapplication (HWND hWnd, char *buf, unsigned int nbuf,
	int shift, char *key) {
/*	int flag; */
	char *s;
#ifdef DEBUGPATTERN
	if (bDebug > 1) {
		sprintf(debugstr, "Entering parseapplication: %s\n", buf);
		OutputDebugString(debugstr); 
//		wincancel(debugstr);	// debugging only
	} 
#endif
	if (strchr(buf, '@') != NULL) {
		if (expandatsign (buf, nbuf) == 0) return 0;
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "After expandat: %s\n", buf);
			OutputDebugString(debugstr); 
//			wincancel(debugstr);	// debugging only
		}
#endif
	}
	if (strchr(buf, '*') != NULL) {
		if (expandasterisk (buf, nbuf) == 0) return 0;
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "After expandasterisk: %s\n", buf);
			OutputDebugString(debugstr); 
//			wincancel(debugstr);	// debugging only
		}
#endif
	}
	if (strchr(buf, '%') != NULL) {
		if (expandpercent (buf, nbuf) == 0) return 0;
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "After expandpercent: %s\n", buf);
			OutputDebugString(debugstr); 
//			wincancel(debugstr);	// debugging only
		}
#endif
	}
	if (strchr(buf, '#') != NULL) {
		if (expandhash(buf, nbuf) == 0) return 0; 
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "After expandhash: %s\n", buf);
			OutputDebugString(debugstr); 
//			wincancel(debugstr);	// debugging only
		}
#endif
	}
	if (strchr(buf, '\"') != NULL) {
		if (expandquotes(hWnd, buf, nbuf) == 0) return 0;
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "After expandquotes: %s\n", buf);
			OutputDebugString(debugstr); 
//			wincancel(debugstr);	// debugging only
		}
#endif
	}
	if (strchr(buf, '$') != NULL) {		/* 1994/Feb/20 */
/*		switchtowindow (hWnd, buf, shift); */
		if (expanddollars(hWnd, buf, nbuf, shift) != 0)
			return 0;			/* it was switchtowindow:  avoid WinExec */
	}
	if (strchr(buf, '?') != NULL || shift != 0) {
/*		if (expandquestion (hWnd, buf, nbuf, shift) == 0) return 0; */
		if (expandquestion(hWnd, buf, nbuf, shift, key) == 0) return 0;
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			sprintf(debugstr, "After expandquestion: %s\n", buf);
			OutputDebugString(debugstr); 
//			wincancel(debugstr);	// debugging only
		}
#endif
	}

	if ((s = IsDVIPSONECall(buf)) == NULL) isdvipsonecallflag = 0;
	else {
		isdvipsonecallflag = 1;
/*		pass along requested command line flags for DVIPSONE */
//		Direct to Distiller cannot happen when called from menu ? 99/Dec/30
//		if (bUsingDistiller && szDVIDistiller != NULL) {	// ???
//			replacestring (s+12, 0, " ", 1, buf, nbuf);
//			replacestring (s+13, 0, szDVIDistiller, strlen(szDVIDistiller), buf, nbuf);
//			if (bDebug > 1) OutputDebugString(buf);
//		}	else
		if (szDVIPSONE != NULL) {
			replacestring (s+12, 0, " ", 1, buf, nbuf);
			replacestring (s+13, 0, szDVIPSONE, strlen(szDVIPSONE), buf, nbuf);
			if (bDebug > 1) OutputDebugString(buf);
		}
	}

	if ((s = IsTeXCall(buf)) == NULL) istexcallflag = 0;
	else {
		istexcallflag = 1;
		if (strstr(buf, "initex") == NULL &&	/* 97/June/5 */
			strstr(buf, "INITEX") == NULL) {	/* don't screw iniTeX call */
/*			pass along requested command line flags for YANDYTEX */
			if (szYandYTeX != NULL) {
				replacestring (s+7, 0, " ", 1, buf, nbuf);
				replacestring (s+8, 0, szYandYTeX, strlen(szYandYTeX),
							   buf, nbuf);
				if (bDebug > 1) OutputDebugString(buf);
			}
		}
	}

/*	pass along requested command line flags for LaTeX --- 2000/April/3 ??? */
	if ((s = strstr(buf, "latex.bat")) != NULL ||
		  (s = strstr(buf, "LATEX.BAT")) != NULL) {
		if (szYandYTeX != NULL) {
			replacestring (s+9, 0, " +latex ", 8, buf, nbuf);
			replacestring (s+17, 0, szYandYTeX, strlen(szYandYTeX),
						   buf, nbuf);		// duplication ???
			if (bDebug > 1) OutputDebugString(buf);
		}
	}
	return 1;
}

/* Write new entry in [Applications] section of INI file - make it n-th item */
/* non-trivial if we want to avoid reading/writing the INI file */

// WritePrivateOrder(szMenuKey, szNewValue, id, 0);
// WritePrivateOrder("", "", id+1, 1);

BOOL WritePrivateOrder (char *szEntry, char *szString, int n, int delete) {
	int k;
	char *achTemp = "Temporary";
	char *keybuffer=NULL;				/* 97/June/5 */
	char szNewKey[MAXKEY];
	char *s;
	int napp;
	int flag=0;

/*	Replace tab with bar in key if user misused notation */
	if ((s = strchr(szEntry, '\t')) != NULL) *s = '|';
/*	Make a copy of the whole [Applications] section under new name */
/*	Clean out existing [Temporary] section if it exists (should not) */
	WritePrivateProfileString (achTemp, NULL, NULL, achFile);
	napp = 0;
	if (bCallPreview) napp++;				/* step over first two */

/*	start by reading in all of the keys in the [Applications] section */
	for (;;) {
		keybuffer = (char *) LocalAlloc(LMEM_FIXED, (UINT) nkeybuffer);
		if (keybuffer == NULL) {	/* unable to allocate memory */
			winerror ("Unable to allocate memory");
			return 0;
		}
		k = GetPrivateProfileString(achAp, NULL, "",
/*			keybuffer, sizeof(keybuffer), achFile); */
						keybuffer, nkeybuffer, achFile);
		if (k < nkeybuffer-2) break;		/* the keys fitted in */
		LocalFree ((HLOCAL) keybuffer);
		nkeybuffer = nkeybuffer * 2;		/* try again with larger buffer */
	}
	if (k == 0) {
		LocalFree ((HLOCAL) keybuffer);
		return 0;					/* failed */
	}
/*	now step through all the keys and read the corresponding values */
	s = keybuffer;
	for (;;) {				/* step through and transfer to Temporary */
		if (_strnicmp(s, "DummyEntry", 10) == 0) {	// 2000 Aug 15
		}
		else {
			GetPrivateProfileString (achAp, s, "", str, sizeof(str), achFile); 
			WritePrivateProfileString (achTemp, s, str, achFile);		
			napp++;
		}
		s = s + strlen(s) + 1;
		if (*s == '\0') break;
	}
/*	Now copy back items in [Temporary] to the [Applications] section */
/*	Clean out the [Applications] section to get ready */
	WritePrivateProfileString (achAp, NULL, NULL, achFile);
	napp = 0;
	if (bCallPreview) napp++;				/* step over first two */
/*	start by reading in all of the keys in the [Temporary] section */
	k = GetPrivateProfileString(achTemp, NULL, "",
/*		keybuffer, sizeof(keybuffer), achFile); */
		keybuffer, nkeybuffer, achFile);
/*	we are assuming it fits this time ... */
/*	if (k == 0) ; */	 /* failed */
/*	now step through all the keys and read the corresponding values */
	s = keybuffer;

	for (;;) {
		if (napp == n && delete == 0) {	/* insert new item here ? */
			WritePrivateProfileString (achAp, szEntry, szString, achFile);
			napp++;
#ifdef DEBUGGING
			if (bDebug > 1) {	 
				sprintf(debugstr, "%s=%s (%d %d)\n", szEntry, szString, n, napp);
				OutputDebugString(debugstr);
			}
#endif
			flag++;					/* note that we put it in */
		}
		GetPrivateProfileString(achTemp, s, "", str, sizeof(str), achFile); 
		if (strncmp(s, "Separator", 9) == 0) {	/* assign new numbers */
			sprintf(szNewKey, "Separator%d", napp);
		}
		else strcpy(szNewKey, s);
/*		don't copy it over if we have been asked to delete it */
		if (napp == n && delete != 0) {
//			sprintf(debugstr, "Deleting %s=%s (%d)", szNewKey, str, n);
//			winerror(debugstr);		// debugging only
			flag++;
		}
		else WritePrivateProfileString (achAp, szNewKey, str, achFile);
		napp++;
		s = s + strlen(s) + 1;
		if (*s == '\0') break;
	}

	if (flag == 0) {
/*		if (bDebug > 1) {
			sprintf(debugstr, "%s=%s (%d %d)\n", szEntry, szString, n, napp);
			OutputDebugString(debugstr); */
/*		following fixed 1995/Nov/17 */
		if (napp == n && delete == 0) {	/* insert new item here ? */
			WritePrivateProfileString (achAp, szEntry, szString, achFile);
			napp++;
#ifdef DEBUGGING
			if (bDebug > 1) {	 
				sprintf(debugstr, "%s=%s (%d %d)\n", szEntry, szString, n, napp);
				OutputDebugString(debugstr);
			}
#endif
			flag++;					/* note that we put it in */
		}
	}
/*	clean out [Temporary] section again */ /* omit for debugging */
	WritePrivateProfileString (achTemp, NULL, NULL, achFile); // omit for debugging only
/*	Clear Window's cache of the INI file - if it is currently cached */
/*	WritePrivateProfileString (NULL, NULL, NULL, lpszFileName); */
	if (keybuffer != NULL) LocalFree ((HLOCAL) keybuffer);
	return 1;
}

#ifdef IGNORED
/*	Clear Window's cache of the INI file - if it is currently cached */
/*	WritePrivateProfileString (NULL, NULL, NULL, lpszFileName); */
/*	memset (&ofin, 0, sizeof(OFSTRUCT));
	ofin.cBytes = sizeof(OFSTRUCT);
	inifile = OpenFile(lpszFileName, &ofin, OF_READ);
	if (inifile == NULL) {
		if (bDebug && bDebugKernel)	OutputDebugString ("Can't open INI\n");
		WritePrivateProfileString (lpszSection, lpszEntry, lpszString, lpszFileName);
		return 1;
	} */
#endif

char *appcallpatt = "Application Call Pattern|";

int addmenuitem (HWND hWnd) {			/* new version 94/Feb/24 */
	int n, flag;
	char szNewValue[BUFLEN];			/* needs to be this big? */

	*szMenuKey = '\0';					/* set up initial Key */
	*str = '\0';						/* set up initial Value */

//	flag = DialogBox(hInst, "EditItem", hWnd, (DLGPROC) EditItemDlg);
	flag = DialogBox(hInst, "EditItem", hWnd, EditItemDlg);
/*	Key returned in szMenuKey, Value in str */
	if (flag == 0) return 0;			/* user cancelled */
	if (flag == IAI_DELETE) return 0;	/* `Delete' makes no sense here */
	else if (flag == IAI_SEPARATOR) {	/* add a separator here */
		sprintf(szMenuKey, "Separator%d", nApplications);
		sprintf(str, "Line");
	}
/*	Treat IAI_REPLACE and IAI_ADD the same way */
	if (*szMenuKey == '\0' || *str == '\0') {
		winerror ("Need Key and Value");
		return 0;
	}
	if (strcmp(szMenuKey, "Preview") == 0 ||
		strcmp(szMenuKey, "Add Item") == 0) {
		winerror ("Illegal Key");
		return 0;
	}
	strcpy (szNewValue, str);			/* save value out of harms way */
/*	see if it already exists */		/* is it safe to use `buffer' here ? */
	n = findmenukey(hWnd, szMenuKey);			/* returns zero if not found */
	if (n > 0) {
		winerror ("Key Already Exists");
		return 0;
	}
	if (flag <= 0) return 0;					/* user cancelled */
	if (*szNewValue == '\0') return 0;	/* insanity check */
/*	always append to end of menu */
	(void) WritePrivateProfileString(achAp, szMenuKey, szNewValue, achFile);
	return 1;							/* success */
}

int editmenuitem (HWND hWnd, char *szKey, char *item, int id) {
	int n, flag;
	char szOldKey[MAXKEY];
	char szNewValue[BUFLEN];			/* needs to be this big? */

	strcpy(szOldKey, szKey);				/* remember old Key */
	strcpy(szMenuKey, szKey);				/* set up initial Key */
	strcpy(str, item);						/* set up initial Value */

//	flag = DialogBox(hInst, "EditItem", hWnd, (DLGPROC) EditItemDlg);
	flag = DialogBox(hInst, "EditItem", hWnd, EditItemDlg);
/*	Dialog Procedure:  Key returned in szMenuKey, Value in str */
	if (flag == 0) return 0;				/* user cancelled */

	if (flag == IAI_NEXT) {					/* delete NEXT */
/*		step through and delete Separator that comes next */
		WritePrivateOrder ("", "", id, 1);	// 2000 Aug 15
//		WritePrivateOrder("", "", id+1, 1);	
		return 1;
	}
	if (flag == IAI_DELETE) {				/* just delete item */
		if (strcmp(szOldKey, szMenuKey) != 0) {
			winerror("Key Has Changed");
			return 0;						/* don't mess with it then */
		}
/*		*str = '\0'; */
		(void) WritePrivateProfileString(achAp, szOldKey, NULL, achFile);
		return 1;
	}
	else if (flag == IAI_SEPARATOR) {		/* add a separator here */
		sprintf(szMenuKey, "Separator%d", id);
/*		sprintf(str, "Line"); */			/* then treat same as `Add' */
		strcpy(str, "Line");				/* then treat same as `Add' */
	}
/*	IAI_REPLACE and IAI_ADD cases: */
	else if (*szMenuKey == '\0' || *str == '\0') {
		winerror ("Need Key and Value");
		return 0;
	}

	if (strcmp(szMenuKey, "Preview") == 0 ||
		strcmp(szMenuKey, "Add Item") == 0) {
		winerror ("Illegal Key");
		return 0;
	}
	if (*szMenuKey == '\0') return 0;	/// sanity check
	if (flag <= 0) return 0;			/* user cancelled */

	strcpy(szNewValue, str);			/* save value out of harms way */
	if (flag == IAI_REPLACE) {			/* check simple replacement case */
		if (strcmp(szOldKey, szMenuKey) == 0) {	
			(void) WritePrivateProfileString(achAp, szMenuKey, szNewValue, achFile);
			return 1;		/* simple case --- user did not change the key */
		}
	}

/*	see if it already exists */		/* is it safe to use `buffer' here ? */
	n = findmenukey(hWnd, szMenuKey);			/* returns zero if not found */
	if (n > 0) {					/* sets up value of key in str .... */
		if (flag != IAI_REPLACE) {		/* 1994/Mar/3 */
			winerror ("Key Already Exists");
			return 0;
		}
	}
	if (flag == IAI_REPLACE) {	/* key changed - first get rid of old entry */
		(void) WritePrivateProfileString(achAp, szOldKey, NULL, achFile);
		id--;					// ???
		/* then treat just the same as `Add' case */
	}
/*	(void) WritePrivateProfileString(achAp, szMenuKey, szNewValue, achFile); */
/*	WritePrivateOrder(szMenuKey, str, id); */
	WritePrivateOrder(szMenuKey, szNewValue, id, 0);
	return 1;							/* success */
}

void trimoffname (char *filename) {	/* clip filename off, leaving path */
	char *s;
	if ((s = strrchr(filename, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(filename, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(filename, ':')) != NULL) *(s+1) = '\0';
}

/* Try and bring up appropriate DVI file on screen */
/* returns non-zero if it wants DVIWindo to open a file */
/* -1 for random DVI file, -2 for help file */

int callpreview (HWND hWnd) {		/* click on `Preview' in `TeX menu' */
	char namea[MAXDIRLEN], nameb[MAXDIRLEN];
	int helpflag=0;					/* get ready to show `txhlp0.dvi' */
	char *s;
	int n;
	char buffer[BUFLEN];	/* temp */ /* why not use global one ? */
#ifndef LONGNAMES
	OFSTRUCT OfStruct;		/* used only to check for texhelp DVI file */
#endif

	if (bTeXHelpFlag) {		/* If TeXHelp was the last thing called */
/*		make the directory up from texhelp= entry in dviwindo.ini */
/*		n = GetPrivateProfileString(achAp, "TeXHelp", "",
			str, sizeof(str), achFile); */
		n = findmenukey(hWnd, "TeXHelp");		/* find TeXHelp menu item */
		if (n == 0) strcpy(str, texhelpdir);	/* use default */
/* trim off the `texhelp.exe' part */
		trimoffname(str);
		strcat(str, "\\");
		strcat(str, texhelpfile);
#ifdef DEBUGTEXHELP
		if (bDebug > 1) {
			OutputDebugString(str);
//			wincancel(str);		// debugging only
		}
#endif
/*	check whether c:\texhelp\txhlp0.dvi exists - if not then punt */		
#ifdef LONGNAMES
		if (LongOpenFile(str, OF_EXIST) != HFILE_ERROR) {
#else
		if (OpenFile(str, &OfStruct, OF_EXIST) != HFILE_ERROR) {
#endif
			helpflag = 1;
			strcpy(buffer, str);	/* avoid corruption ? */
			if (bDebug > 1) OutputDebugString("FOUND\n");  
/*	maybe don't mess up SourceOpenName, SourceDefPath etc */
/*			strcpy(SourceOpenName, removepath(str)); */
/*			strcpy(SourceDefPath, str); */
/*			trimoffname(SourceDefPath); */
/*			strcpy(SourceDefExt, ".dvi"); */
		}	 /* success ! */
	}

/*	Maybe DON'T copy c:\texhelp\txhlp0.dvi over SourceOpenName */
	if (bFileValid) {	/*	check whether currently showing a file */
/*	check first whether happen to currently be showing the specified file ! */
/*	don't do this if showing TeXHelp dvi file */
		if (*SourceOpenName != '\0' && helpflag == 0) {
			strcpy(namea, removepath(OpenName));
			if ((s = strrchr(namea, '.')) != NULL) *s = '\0';
			strcpy(nameb, removepath(SourceOpenName));
			if ((s = strrchr(nameb, '.')) != NULL) *s = '\0';
			if (strcmp(namea, nameb) == 0) {
				if (bDebug > 1) {
					OutputDebugString(SourceOpenName);
					OutputDebugString(" already showing\n");
				}
				return 0;		/* if so, do nothing */
			}
		}
/* otherwise if showing wrong file, close the file and go on */
		closeup(hWnd, 1);
		if (bDebug > 1)	OutputDebugString("closeup\n"); 

/*		(void) SendMessage(hWnd, WM_COMMAND, IDM_CLOSE, 0L); 
		if (bDebug > 1)
			OutputDebugString("SendMessage IDM_CLOSE\n"); */
	}

/*	check whether SourceOpenName is non-empty */
/*	don't do this if showing TeXHelp dvi file */
	if (*SourceOpenName == '\0' && helpflag == 0) 
/*		if no name specified, just call Open Dialog */ /* use SendMessage ? */
		return -1;

/*	If calling TeXHelp, try and hide the old file name if any */
	if (helpflag && *OpenName != '\0') {
		strcpy(SourceOpenName, OpenName);
		strcpy(SourceDefPath, DefPath);
/*		strcpy(SourceDefExt, DefExt); */
/*		strcpy(SourceDefSpec, DefSpec); */
		bTeXHideFlag = 1;				/* indicate it was hidden */
		if (bDebug > 1) OutputDebugString("TeXHelp file name hidden\n");
	}

/*	If we are using WorkingDirectory then copy IniDefPath */
/*	If we are using SourceDirectory then copy SourceDefPath */
/*	If TEXDVI environment variable is set then use that */

/*	strcpy(DefPath, IniDefPath); */								/* ??? */
/*	if (bUseSourcePath) strcpy(DefPath, IniDefPath); */			/* ??? */
/*	else if (bUseWorkPath) _getcwd(DefPath, sizeof(DefPath)); */	/* ??? */
	if (bUseWorkPath && IniDefPath != NULL)
		strcpy(DefPath, IniDefPath);
//	if (IniDefPath != NULL) strcpy(DefPath, IniDefPath);
	if (bUseSourcePath) strcpy(DefPath, SourceDefPath);		/* 93/Dec/9 */
	if (bUseTeXDVI && szTeXDVI != NULL)
		strcpy(DefPath, szTeXDVI);				/* 94/Feb/18 */
/*	if a name has been specified, then just make up OpenName */
	strcpy(OpenName, DefPath);				/* ??? */
	s = OpenName + strlen(OpenName) - 1;
	if (*s != '\\' && *s != '/') strcat(OpenName, "\\");
	strcat(OpenName, removepath(SourceOpenName));
/*	first flush existing extension if any */
	if ((s = strrchr(OpenName, '.')) != NULL) { 
		*s = '\0';
	}
/*	then force default extension 98/June/30 */
	strcat(OpenName, DefExt);			/* or ".dvi" */

	if (helpflag) {
		if (bDebug > 1) OutputDebugString("Help Flag On ");
		strcpy(OpenName, removepath(buffer));
		strcpy(DefPath, buffer);		/* 93/Dec/19 */
		trimoffname(DefPath);
		strcpy(DefExt, ".dvi"); 
	}

	if (bDebug > 1) OutputDebugString(OpenName);

 	if (! helpflag) {
		ParseFileName();			/* 1996/Jan/10 ??? */
		strcpy(DefSpec, "*");		/* ??? */
		strcat(DefSpec, DefExt);	/* fix up DefSpec */
		return -1;
	}
	return -2;							// 2000/Jan/02
}

#ifdef YANDYTEXDLL

HINSTANCE hYandYTeX=NULL;

int RunYandYTeXDLL (HWND hWnd, char *str) {
	int (* lpYandYTeX) (HWND, char *, int(*)(char *, char *, char *))=NULL;	
/*	above must match ConsoleInput definition in texwin.h */
/*	int ConsoleInput(char *, char *, char *); */
	int ret, fault=0;

	if (bDebug > 1)	OutputDebugString("RunYandYTeXDLL");

	if (hYandYTeX == NULL) 
		hYandYTeX = LoadLibrary("YandYTeX");	/* connect to YandYTeX.DLL */
	if (hYandYTeX == NULL){
		WriteError("Can't link to YandYTeX.DLL");
		return -1;
	}

	lpYandYTeX = (int (*) (HWND, char *, int(*)(char *, char *, char *)))
				 GetProcAddress(hYandYTeX, "yandytex");
	if (lpYandYTeX == NULL)	{
		winerror("Can't find YandYTeX entry point");
		fault++;
	}
	else {
 		if (hConsoleWnd == NULL)		// create console window if needed 
			CreateConsole(hInst, hWnd);	// for YandYTeX.DLL
		if (hConsoleWnd == NULL) {
			ShowLastError();			// no console window created 
			fault++;
		}
		else {		
			(void) EnableWindow(hWnd, FALSE); 	/* disable main Window ??? */
/*			call YandYTeX with command line string */
			ret = (* lpYandYTeX) (hConsoleWnd, str, ConsoleInput);
			(void) EnableWindow(hWnd, TRUE); 	/* reenable main Window ??? */
		}
/*		can free library now since dviwindo takes care of modeless dialog */
	}
	FreeLibrary(hYandYTeX);
	hYandYTeX = NULL; 
	return fault;
}

#endif
	
// EXE_ERROR strings no longer exist...

/*	ideally want *.PIF around to control windowing and such... */
/*  WinExec: `For a non-Windows application, the PIF, if any, */
/*  determines the windows state' */

/* returns 0 if it fails, 1 if it succeeds, -1 if it wants IDM_OPEN */
/* -2 if it wants IDM_OPEN, but its just the Help file being opened */

// flag = callapplication(hWnd, id-IDM_APPLICATION, bShiftFlag, bControlFlag);

int callapplication (HWND hWnd, int id, int shift, int editflag) {
	HMENU hMenu, hSubMenu;
/*	HWND hFocus; */
	int m, n;
	int flag;
/*	int nCmdShow = SW_SHOW; */
	int nCmdShow;
/*	UINT err; */
	HINSTANCE err;					/* 95/Mar/31 */
	char key[MAXKEY];
	char buffer[BUFLEN];			/* temporary buffer */
	char *s, *sm;

	if (id < 0 || id >= nApplications) return 0;	/* fail */

	if (bCallPreview) {	/* "Preview" */
		if (id < 1)
			return callpreview(hWnd);	/* returns -1 if it wants FileOpen */
	}

/*	if (GetAsyncKeyState(VK_CONTROL) < 0) editflag = 1; else editflag = 0; */
	hMenu = GetMenu(hWnd);
	m = GetMenuItemCount(hMenu);
	if (!bHelpAtEnd) hSubMenu = GetSubMenu(hMenu, m - 1);	/* last sub menu */
	else hSubMenu = GetSubMenu(hMenu, m - 2);		/* last sub menu */

/* editnext:	*/								/* loop for edit next */

	if (id < 0 || id >= nApplications) return 0;	// sanity check
/*	get KEY for this application */
	n = GetMenuString(hSubMenu, id, key, sizeof(key), MF_BYPOSITION);
	if (n == 0) {
		if (bDebug > 1)	OutputDebugString("GetMenuString Failed\n");
		return 0;								/* fail */
	}
/*	if (strncmp(key, "Separator", 9) == 0) return 0; */	/* nonsense ? */
	if (*key == '\0') return 0;					// sanity check
/*  a tab in menu is represented by a bar in dviwindo.ini */
	if ((sm = strchr(key, '\t')) != NULL) *sm = '|';	/* 93/Dec/25 */
	if (bDebug > 1) {
		OutputDebugString("TeX Menu Call: ");
		OutputDebugString(key); OutputDebugString("=");
	}
/*	Get calling pattern for this application */
	n = GetPrivateProfileString(achAp, key, "", buffer, sizeof(buffer), achFile);
#ifdef DEBUGPATTERN
	if (bDebug > 1) {
		OutputDebugString(buffer);
//		wincancel(buffer);		// debugging only
	}
#endif
/*	if (n == 0) return 0; */					/* fail */
	if (editflag) {								/* user wants to edit item */
		editmenuitem(hWnd, key, buffer, id);	/* 1994/Feb/25 */
		return 1;								/* finished edit */
	}
	if (n == 0) return 0;						/* fail */
	if (parseapplication (hWnd, buffer, sizeof(buffer), shift, key) != 0) {

/*		if (bDebug > 1) OutputDebugString("Parsed Application\n"); */

/*		for DOS applications, need PIF file to control such things */
/*		here SW_SHOWMINIMIZED means as an icon ! */
/*		if (bRunMinimized != 0) nCmdShow = SW_SHOWMINIMIZED;
		else nCmdShow = SW_SHOW; */

		if (nCmdShowForce >= 0) nCmdShow = nCmdShowForce;
/*		else nCmdShow = SW_SHOWMAXIMIZED; */	/* 1994/Mar/7 */
		else nCmdShow = SW_NORMAL;				/* 1996/Dec/31 */

/* 0 Hide, 1 ShowNormal/Normal, 2 ShowMinimized, 3 ShowMaximized/Maximize */
/* 4 ShowNoactive, 5 Show, 6 Minimize, 7 ShowMinNoactive, 8 ShowNA, 9 Restore*/

/*		set up current directory properly 1993/Dec/9 */
		if (bUseWorkPath && IniDefPath != NULL)
			ChangeDirectory(IniDefPath, 1);	
//		if (IniDefPath != NULL) ChangeDirectory(IniDefPath, 1);	
		if (bUseSourcePath) ChangeDirectory(SourceDefPath, 1);
				/* above is redundant ? */
/*		here we *don't* want to use szTeXDVI */
/*		worry about unqualified file names in this case ??? */

/*		if (bDebug && bDebugKernel) */
/*		if (bDebug > 1) OutputDebugString (buffer); */
/*		redundant ? since command line shown in MaybeShowStr */

		if (strstr(buffer, "texhelp") != NULL)	bTeXHelpFlag = 1;
		else bTeXHelpFlag = 0;	/* maybe remember directory ? */

/*		Is it safe to use a string that is on the stack ??? 95/Jan/5 */
/*		Is it safe to use global str ??? 97/Mar/30 */
		strcpy(str, buffer);
		if (bDebug || bShowCalls) {
			err = (HINSTANCE) 0;		/* avoid problems if exit here */
			flag = MaybeShowStr(str, "Application");	/* 95/Jan/8 */
			if (flag == 0) return 1;	/* cancel => pretend success */
		}
/*		NOTE: WinExec returns task handle of spawned job */
/*		if ((err = WinExec(buffer, nCmdShow)) > 32) */
/*		err = WinExec(str, nCmdShow); */
/*		Do something more elaborate here ? Like TryWinExec ? Current Dir ? */
#ifdef DEBUGPATTERN
		if (bDebug > 1) {
			OutputDebugString(str);
//			wincancel(str);		// debugging only
		}
#endif

/*		new debugging info 98/Jun/28 */
//		WritePrivateProfileString(achPr,
//				istexcallflag ? "LastTeX" : "LastWinExe", str, achFile);
		WritePrivateProfileString(achDiag,
				istexcallflag ? "LastTeX" : "LastWinExe", str, achFile);


//		int RunYandYTeXDLL (HWND hWnd, char *str);
		if (istexcallflag && bUseDLLs) {
			if (RunYandYTeXDLL(hWnd, str) == 0) return 0; // success
							// else drop through if it fails					
		}

//		Following new 2000/Apr/3 ???  Stupid to run DVIPSONE this way!
//		Can't allow callback here since no printer driver setup --- bCallBackPass
//		int RunDVIPSONEDLL (HWND hWnd, char *str, int usecallbackflag);
		if (isdvipsonecallflag && bUseDLLs) {
			if (RunDVIPSONEDLL(hWnd, str, 0) == 0) return 0; // success
							// else drop through if it fails					
		}

		err = (HINSTANCE) WinExec(str, nCmdShow);
		if (err >= HINSTANCE_ERROR) {			/* 95/Jan/5 */
/*			do we want to keep track of the Task handle or the Window ? */
/*			(void) GetTaskWindow((HINSTANCE) err); */
#ifdef DEBUGPATTERN
			if (bDebug > 1) {
				OutputDebugString("After WinExec\n");
			}
#endif
			return 1;							/* success */
		}
		else {					/*  use error names in string table */
			strcpy(str, "TeX Menu: "); 
			s = str + strlen(str);
/*			flag = LoadString(hInst, (WORD) (EXE_ERROR + err), */
/*			UINT is 16 bit in WIN16 and 32 bit in WIN32 */
			flag = LoadString(hInst, (UINT) (EXE_ERROR + (UINT) err), 
/*				str+strlen(str), BUFLEN-strlen(str)); */
				s, sizeof(str) - strlen(str));
/*			if (flag == 0) sprintf(str+13, "Error %d ", err); */
			if (flag == 0) sprintf(s, "Error %d ", err); 
			strcat(str, "\n\n");
			strcat(str, " WinExec: ");
			strcat(str, buffer);		/* debugging info */
			strcat(str, "\n\n");
			winerror(str);
			return 0;				/* failed */
		}
	}
	else {
		if (bDebug > 1) {
			sprintf (debugstr, "Pattern: %s cancelled\n", buffer);
			OutputDebugString(debugstr);
		}
		return 0;					/* user cancelled */
	}
/*	return 1; */ /* unreachable ? */			/* success */
}

/* METAFILE METAFILE METAFILE METAFILE METAFILE METAFILE METAFILE METAFILE */

/* This should really be in winspeci.c - here to spread the load */

/* adjust size according to current zoom setting */ /* overflow check */

int adjustsize (int oldsize, int zoom) {
	long size=oldsize;

	if (zoom > 0)  
		while(zoom-- > 0) {
			if (bSmallStep == 0)
				size = (((size * 11 + 5) / 10) * 11 + 5) / 10;
			else size = (((size * 21 + 10) / 20) * 21 + 10) / 20;
		}
	else if (zoom < 0)
		while(zoom++ < 0) {
			if (bSmallStep == 0)
				size = (((size * 10 + 5) / 11) * 10 + 5) / 11;
			else size = (((size * 20 + 10) / 21) * 20 + 10) / 21;
		}
	return (int) size;
} 

/* unsigned long ureadfour(FILE *input) {	
	unsigned int a, b, c, d;
	a= getc(input); b= getc(input); c = getc(input); d = getc(input);
	return (((((((unsigned long) a << 8) | b) << 8) | c) << 8) | d);
} */

/* See whether Window MetaFile is visible - not used currently ? */
/* We won't worry here about Windows 95 bug in RectVisible */

/* bounding box given in logical units ? i.e. TWIPS */
/* dvi_hh, dvi_vv, dvi_hh + xview, dvi_vv + yview */

#ifdef IGNORED
int metavisible(HDC hDC, int xll, int yll, int xur, int yur) {
	RECT MetaRect;

	MetaRect.left = xll; MetaRect.right = xur;
	MetaRect.bottom = yll; MetaRect.top = yur;

/*	Avoid RectVisible() in MetaFile - use InterSectRect instead */
	if ((bCopyFlag == 0 && RectVisible(hDC, &MetaRect) != 0) ||
		(bCopyFlag != 0 && InterSectRect(&CopyClipRect, &MetaRect) !=	0)) {
		return 1;			/* it is visible */
	}
	else return 0;			/* it is not visible */
}
#endif

/* Show Viewport rectangle (logical coordinates) */
/* We already shifted the Viewport origin at this point ... */
/* So these are relative coordinates ... */

void ShowViewPort (HDC hDC, int xll, int yll, int xur, int yur) {
	POINT Box[5];
	HPEN hOldPen;
/*	int mapmode; */
/*	WORD mapmode; */
	
	if (bCopyFlag != 0) return;
	hOldPen = SelectObject(hDC, hFigurePen);
	Box[0].x = xll;		Box[0].y = yll;
	Box[1].x = xur;		Box[1].y = yll;	
	Box[2].x = xur;		Box[2].y = yur;	
	Box[3].x = xll;		Box[3].y = yur;	
	Box[4].x = xll;		Box[4].y = yll;
	Polyline(hDC, Box, 5);
	if ((UINT) hOldPen > 1)	/* avoid Windows MetaFile problem */
		(void) SelectObject(hDC, hOldPen);		
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef DEBUGMETAFILE
void ShowMetaFileHeader (char far *, char *);	/* in winspeci.c */
#endif

/* reads file and creates in-core MetaFile */
/* position at start of WMF in the file (1, 0, ...) */
/* remember, if this returns NULL, need to close file and restore DC */

/* This version uses LOW LEVEL input */	/* see other version in winspeci.c */

static HMETAFILE MakeMetaFile (HFILE input, long metalength) {
	HGLOBAL hMem=NULL;
	HMETAFILE hMF=NULL;
	char huge *lpMetaFile;				/* use huge pointer */
	char huge *u;						/* use huge pointer */
	char buffer[256];					/* small near buffer */
	int nlen;
	long metabytes;						/* how much has been read */
/*	int n; */

	hMem = GlobalAlloc(GMEM_MOVEABLE, metalength);
	if (hMem == NULL) {
		winerror ("Unable to allocate memory");
		return NULL;								/* 1995/Jan/19 */
	}
	lpMetaFile = (char huge *) GlobalLock (hMem);	/* make huge pointer ??? */
	if (lpMetaFile == NULL) {
		winerror ("Unable to allocate memory");		/* debugging only ? */
		return NULL;
	}
/*	read it all in */ /* use `huge' pointer to read more than 64k ? */
	u = lpMetaFile;
	metabytes = 0;
/*	*u++ = (char) 1; */
/*	*u++ = (char) 0; */
/*	metabytes = 2; */
	for (;;) {
		nlen = sizeof(buffer);
		if (metabytes + nlen > metalength)
			nlen = (int) (metalength - metabytes);
/*		nlen = fread (buffer, 1, nlen, input); */
		nlen = _lread (input, buffer, nlen);
		if (nlen == HFILE_ERROR) break;			/* premature EOF */
		if (nlen == 0) break;					/* premature EOF */
#ifdef USEMEMCPY
		memcpy(u, buffer, nlen);				/* 99/June/26 */
#else
		copynearimage(u, buffer, nlen);
#endif
		metabytes = metabytes + nlen;
		if (metabytes >= metalength) break;
		u = u + nlen;
	}
	if (metabytes != metalength) {
		if (bDebug > 1) {
			sprintf(buffer, "Read %ld bytes out of %ld\n", metabytes, metalength);
			OutputDebugString(buffer);
		}
	}
#ifdef DEBUGMETAFILE
	if (bDebug > 1)
		ShowMetaFileHeader((char far *)lpMetaFile, "MakeMetaFile winsearc");
#endif
/*	need to use SetMetaFileBits to convert global memory object to MetaFile */

/*	hMF = SetMetaFileBitsEx(metalength, hMem); */    /* 1995/Oct/31 */
	hMF = SetMetaFileBitsEx(metalength, (unsigned char *) lpMetaFile); /* GlobalFree later ??? */
	if (hMF == NULL) {
		sprintf(debugstr, "SetMetaFileBitsEx failed len %ld ", metalength);

		ExplainError(debugstr, -1, 1);

/*		(void) wincancel(debugstr); */
/*		ShowLastError(); */
	}
/*	Should we do hMem = GlobalFree (hMem) ??? */

/*	hMF = SetMetaFileBits(hMem); */
/*	if (hMF == NULL) (void) wincancel("SetMetaFileBits failed"); */

	if (hMF == NULL) {
		GlobalUnlock(hMem);
		GlobalFree (hMem);
		hMem = NULL;
	}					/* 1995/Nov/5 */
	hMem = NULL;		/* no longer allowed to refer to it this way */
	return hMF;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int SlowPlayMetaFile (HDC, HMETAFILE);	/* 1996/Aug/10 */

#ifdef DEBUGMETAFILE
void DumpMetaFile(HDC, HMETAFILE);		/* 1996/Aug/5 */
#endif

int bTrustPlaceable = 0;	/* trust MF extent specified in METAFILEPICTHEAD */

int bAdjustOrigin = 0;	/* need to adjust origin once inside MetaFile */

#ifdef IGNORED
RECT Savedbbox;			/* bounding box in metafile units - debugging */
						/* saved only for test in PlayInMetaFile */
#endif

int nSavedLeftRect, nSavedTopRect, nSavedRightRect, nSavedBottomRect;

int PlayInMetafile(HDC, HMETAFILE, int, int, int, int, int, int, int, int);

#ifdef DEBUGMAPPING
void ShowMapping(HDC);
#endif

/***************************************************************************/

typedef struct tagPLACEABLEMETAFILEHEADER {
    DWORD   key;
/*    HANDLE  hmf; */
    WORD  hmf;
    SMALL_RECT    bbox;
/*    RECT    bbox; */
    WORD    inch;
    DWORD   reserved;
    WORD    checksum;
} PLACEABLEMETAFILEHEADER;

/*	New code to support \special{insertmf: ...} 1994/October */
/*	called only winspeci.c */

/*	if sizeflag != 0 then xscale and yscale are actually width and height */
/*	to show at, not xscale and yscale 1996/April/4 experiment */

void showmetafile(HDC hDC, char *metaname, double xscale, double yscale,
				 int sizeflag) {
	int nSavedDC;
	HMETAFILE hMF;
/*	int dvi_hh, dvi_vv; */		/* origin in logical units */
	int xcurr, ycurr;			/* origin in device units */
	int ret;
	POINT posarr[2];
/*	DWORD dw; */
	DWORD dret;
	double width, height;		/* for new style insertmf: 1996/Apr/4 */
	int xview, yview;			/* rectangle in logical units */
	int xextend, yextend;		/* rectangle in device units */
/*	FILE *input; */
	HFILE input;
	int rawwmf=0;				/* raw metafile format */
	int placeable=0;			/* placeable metafile format */
	int clipboard=0;			/* metafile on clipboard file */
	int m;
/*	Placeable MetaFile header information */ /* first the old way */
#ifdef IGNORED
	DWORD key;			/* meta W meta M meta F control Z */
	WORD hmfzero;		/* should be zero */
	SMALL_RECT smallbbox;		/* 1995/Nov/5 */
	RECT bbox;			/* bounding box in metafile units */
	WORD perinch;		/* metafile units per inch */
/*	DWORD reserved; */	/* should be zero */
/*	WORD checksum; */	/* checksum XOR of first ten words */	
#endif
	PLACEABLEMETAFILEHEADER plc;	/* then the new way */
	RECT bbox;			/* bounding box in metafile units */
	WORD perinch=2540;	/* metafile units per inch */
/*	Clipboard header information */
	WORD FileIdentifier;
	WORD FormatCount;
	WORD FormatID;
	DWORD LenData;		// may be used before initialized ?
	DWORD OffData;
	char Name[79];		/* name of clipboard format */
/*	METAFILEPICT header */
/*	int mapmode = MM_ANISOTROPIC; */		/* default assumption */
	WORD kMapMode = MM_ANISOTROPIC;			/* 1995/Nov/5 */
/*	int xExt, yExt; */
/*	WORD xExt, yExt; */						/* 1995/Nov/5 */
	short xExt, yExt;						/* 1995/Nov/5 */
	WORD hMFPict;							/* read, but not used */
	int xHmfLogExt, yHmfLogExt;

/*	if (bShowFlag == 0 || bTextOutFlag == 0) return; */	/* 1994/Oct/4 */
	if (bShowFlag == 0) return;	/* 1994/Oct/4 */

	bbox.left=0; bbox.right=1;	/* to avoid uninit error ??? */
	bbox.top=1; bbox.bottom=0;	/* to avoid uninit error ??? */

#ifdef DEBUGMAPPING
	if (bDebug > 1) ShowMapping(hDC);
#endif

	nSavedDC = SaveDC(hDC);				/* Don't want MF messing up DC ! */
	if (nSavedDC == 0) {
		winerror("Failed to Save DC");	/* debugging only ? */
		return;							/* some kind of error */
	}
#ifdef DEBUGMETAFILE
	if (bDebug > 1) {					/* debugging 1996/Apr/4 */
		sprintf(debugstr, "%s xscl %lg yscl %lg flag %d\n",
				metaname, xscale, yscale, sizeflag);
		OutputDebugString(debugstr);				
	}
#endif
/*	strcpy(wmfname, filename);	*/		/* try and find file */
/*	extension(wmfname, "wmf"); */			/* try metafile extension */
	special = findepsfile (metaname, 0, "wmf"); /* 95/Apr/26 */
	if (special == BAD_FILE) {
		special = findepsfile (metaname, 0, "clp");		
	}
	if (special != BAD_FILE) {	/* close it again after finding it */
		fclose (special);		/* 95/Apr/26 */
		special = BAD_FILE;
	}

	input = _lopen(FileName, READ | OfShareCode); /* 95/Dec/11 */

/*	input = _lopen(FileName, READ | OfShareCode); */

/*	if (input == HFILE_ERROR) {
		forceexten(wmfname, "clp");		
		input = _lopen(wmfname, READ | OF_SHARE_DENY_WRITE);
	} */ /* try clipboard extension */
	if (input == HFILE_ERROR) {
		if (bCopyFlag == 0) RestoreDC(hDC, nSavedDC);
		else RestoreDC(hDC, -1);
/*		may not be good to show error message here ? */
		sprintf(debugstr, "MetaFile %s not found\n", metaname); 
/*		if (bDebug > 1) OutputDebugString(debugstr); */
		(void) wincancel(debugstr);					/* ??? */ 
		return;
	}

/*	first check what flavour of file it is */
/*	`ordinary' MetaFile starts with 1 (control-A) 0 (null) */
/*	`clip-board' file starts with P M-C */
/*	`placeable' MetaFile starts with M-W M-M M-F M-^Z */
/*	read first four bytes */
	_lread(input, &FileIdentifier, sizeof(WORD));
	_llseek(input, 0, SEEK_SET);			/* rewind */

	if (FileIdentifier == 1) {
		rawwmf=1;							/* probably raw MetaFile UGH! */
/*		if (bDebug > 1) OutputDebugString ("WMF file not placeable\n");*/
		if (bDebug) winerror("WMF file not placeable");
		else WriteError("WMF file not placeable");	/* 95/Oct/1 */
	}
	else if (FileIdentifier == 50000) {		/* clipboard file */
		_lread(input, &FileIdentifier, sizeof(FileIdentifier));
		_lread(input, &FormatCount, sizeof(FormatCount));
/*	check through the clipboard formats to see whether a MetaFile is there */
		for (m = 0; m < (int) FormatCount; m++) {
			_lread(input, &FormatID, sizeof(FormatID));
			_lread(input, &LenData, sizeof(LenData));
			_lread(input, &OffData, sizeof(OffData));
			_lread(input, Name, sizeof(Name));
			if (FormatID == CF_METAFILEPICT) {
				_llseek (input, OffData, SEEK_SET);
				_lread(input, &kMapMode, sizeof(kMapMode));	/* ??? */
				_lread(input, &xExt, sizeof(xExt));
				_lread(input, &yExt, sizeof(yExt));
				_lread(input, &hMFPict, sizeof(hMFPict)); 
#ifdef DEBUGMETAFILE
				if (bDebug > 1) {
					sprintf (debugstr, "Clipboard: mapmode %d xExt %d yExt %d\n",
							 kMapMode, xExt, yExt);
					OutputDebugString(debugstr);
				}  /* 1995/April/25 */
#endif
/*	not really needed (or correct?) - just for test in SlowPlayMetaFile */
				bbox.left = 0; bbox.right = xExt;
				bbox.top = yExt; bbox.bottom = 0;
/*	we are now in the file, right at the MetaFile itself */
				OffData = OffData + 8;	/* account for METAFILEPICT header */
				LenData = LenData - 8;	/* account for METAFILEPICT header */
				clipboard = 1;
				break;
			}
		}
		if (clipboard == 0) {
/*			if (bDebug > 1)	OutputDebugString("No MetaFile in ClipBoard\n"); */ 
			sprintf(debugstr, "No MetaFile in %s", metaname);
			(void) wincancel(debugstr);
		}
	}				/*  end of clipboard format */
	else {			/*	check whether `placeable' MetaFile */
/*		use PLACABLEMETAFILEHEADER struct ? */
		_lread(input, &plc, sizeof(plc));
/*		_lread(input, &key, sizeof(DWORD)); */
#ifdef DEBUGMETAFILE
		if (bDebug > 1) {
			sprintf(debugstr, "METAFILE key %0lX\n", plc.key);
			OutputDebugString(debugstr);
		}
#endif
/*	check whether a `placeable' MetaFile ... M-W M-M M-F M-^Z */
/*		if (key == 0x9AC6CDD7L) */
		if (plc.key == 0x9AC6CDD7L) {
/*	read four bytes */
/*			_lread(input, &hmfzero, sizeof(hmfzero)); */
/*			_lread(input, &smallbbox, sizeof(smallbbox)); */
/*			bbox.left = smallbbox.Left; 
			bbox.right = smallbbox.Right;
			bbox.top = smallbbox.Top;
			bbox.bottom = smallbbox.Bottom; */
			bbox.left = plc.bbox.Left;
			bbox.right = plc.bbox.Right;
			bbox.top = plc.bbox.Top;
			bbox.bottom = plc.bbox.Bottom;

/*			bbox = plc.bbox; */

/*	read four bytes */
/*			_lread(input, &perinch, sizeof(perinch)); */
		   perinch = plc.inch;
/*			_lread(input, &reserved, sizeof(DWORD)); */
/*			_lread(input, &checksum, sizeof(WORD)); */
#ifdef DEBUGMETAFILE
			if (bDebug > 1) {
				sprintf (debugstr, "Placeable %d %d %d %d (%u per inch)\n",
						 bbox.left, bbox.top, bbox.right, bbox.bottom, perinch);
				OutputDebugString(debugstr);
			}
#endif
/*			following used only if bTrustPlaceable == 0 */
#ifdef IGNORED
			Savedbbox = bbox;		/* save for test in PlayInMetaFile */
#endif
			placeable = 1;
			OffData = 22;			/*	skip over first 22 bytes */
/*			get the length of the file minus `placeable' MetaFile header */
			LenData = _llseek(input, 0, SEEK_END) - OffData;
			_llseek(input, OffData, SEEK_SET);
		}	/* end of placeable METAFILE format */
	}		/* end of not clipboard file format */

	if (placeable || clipboard) {
#ifdef IGNORED
/*		Maybe deal with larger MetaFiles later ... no longer an issue */
		if (LenData > 65534) {
			_lclose(input);
			if (bCopyFlag == 0) RestoreDC(hDC, nSavedDC);
			else RestoreDC(hDC, -1);
			sprintf(str, "MetaFile too large %lu > 65534\n", LenData);
			(void) wincancel(str);
			return;
		}
#endif
		hMF = MakeMetaFile (input, LenData);	/* Make METAFILE from file */
		_lclose (input);
		if (hMF == NULL) {
			if (bCopyFlag == 0) RestoreDC(hDC, nSavedDC);
			else RestoreDC(hDC, -1);
			return;
		}
/*		extracted code moved to MakeMetaFile () */
	}						/* end of placeable MetaFile, or clipboard */
	else if (rawwmf) {		/* must be raw MetaFile */ /* UGH ! */
		_lclose (input);
/*		hMF = GetMetaFile(wmfname);*/	/* let Windows do the stuff by itself*/
		hMF = GetMetaFile(FileName);	/* let Windows do the stuff by itself*/
	}
	else {				/* it is not any of the three we recognize ... */
		_lclose (input);
		if (bCopyFlag == 0) RestoreDC(hDC, nSavedDC);
		else RestoreDC(hDC, -1);
/*		if (bDebug > 1) OutputDebugString("Bad MF file format\n"); */
/*		(void) wincancel("Bad MF file format"); */
		{
			sprintf(debugstr, "Bad MetaFile format in %s", metaname);
			(void) wincancel(debugstr);
		}
		return;			/* none of the recognized formats */
	}

/*	Note: file is closed now, but hDC is still saved */

	if (hMF == NULL) {
		if (bCopyFlag == 0) RestoreDC(hDC, nSavedDC);
		else RestoreDC(hDC, -1);
		{
			sprintf(debugstr, "GetMetaFile failed on %s", metaname);
			(void) wincancel(debugstr);
		}
		return;							/* some kind of error */
	}

#ifdef DEBUGMETAFILE
	if (bDebug > 1) {
		sprintf(debugstr, "Setting MapMode %d\n", kMapMode);
		OutputDebugString(debugstr);
	}
#endif	
	SetMapMode (hDC, kMapMode);			/* should not be needed ? */

/*	Adjust Viewport Origin */
	posarr[0].x = dvi_hh;			/* logical units - TWIPS */
	posarr[0].y = dvi_vv;			/* logical units - TWIPS */
/*	if (bDebug > 1) {
		sprintf(debugstr, "dvi_hh %d dvi_vv %d\n", dvi_hh, dvi_vv);
		OutputDebugString(debugstr);
	} */

	if (bCopyFlag == 0) {
		ret = LPtoDP(hDC, posarr, 1);  /* problem if CopyFlag ? avoid this ? */
		if (ret == 0) {
			if (bDebug > 1) OutputDebugString("LPtoDP failed\n");
		}
		xcurr = posarr[0].x;				/* device units - pixels */
		ycurr = posarr[0].y;				/* device units - pixels */
/*		check return value for error ? */
/*		following is not used anymore since now use BottomLeft corner */
		if (bMetaTopLeft) {					/* 1994/Nov/29 */
#ifdef DEBUGMETAFILE
			if (bDebug > 1) {
				sprintf(debugstr, "SetViewPortOrg %d %d\n", xcurr, ycurr);
				OutputDebugString(debugstr);
			}
#endif
			dret = SetViewportOrgEx(hDC, xcurr, ycurr, NULL);
/*			if (dret == 0) winerror("SetViewportOrg failed"); */
/*			NO: returns old ViewportOrg, which typically is 0,0 ! */
		}	/* if bMetaFileTopLeft */
	}	/* end of if CopyFlag == 0 */

/*	METAFILEPICT, xExt and yExt are in units of corresponding mapping mode */
/*	except if mapping mode is MM_ISOTROPIC or MM_ANISOTROPIC */
/*	For MM_ISOTROPIC & MM_ANISOTROPIC, xExt and yExt optional size HIMETRIC */
/*	For MM_ANIISOTROPIC xExt and yExt can be zero when no size suggested */	
/*	For MM_ISOTROPIC xExt and yExt can be negative to specify aspect ratio */	

/*	now compute logical extent of MetaFile to be placed */
/*	bbox may not be initialized ??? Noticed 98/Mar/28 */
	if (placeable) {
		xview = bbox.right - bbox.left;
		yview = bbox.top - bbox.bottom;
	}
	else if (clipboard) {
		if (xExt == 0 && yExt == 0) {		/* no size info at all */
			xview = 5080; yview = 5080;		/* default to 2 inch x 2 inch */
		}
		else if (xExt < 0 && yExt < 0) {	/* aspect ratio only given */
			xview = 5080; yview = (int) (((long) xview * yExt) / xExt);
		}
		else {								/* actual size given in 0.01mm */
			xview = xExt; yview = yExt;
		}
/*		if (yExt < 0) yExt = - yExt; */		/* does it matter ??? */
											/* should it be yview ??? */
		yview = - yview;					/* 1995/April/29 */
		perinch = 2540;						/* units are HIMETRIC 0.01mm */
	}
	else {	/* neither clipboard file, nor placable - error! */
		xview = yview = 1;
		perinch = 2540;
	}
/*	xview and yview may not be initialized ??? noticed 98/Mr/26 */
	xHmfLogExt = xview;	/* remember logical extent of MetaFile to be played */
	yHmfLogExt = yview;	/* remember logical extent of MetaFile to be played */

/*	Now scale logical extent of MetaFile to be placed */
	if (placeable || clipboard) {
/*		convert to TWIPS first - there are 1440 TWIPS per inch */
/*		and the MetaFile units per inch are given in `perinch' */
/*		perinch may not be initialized ??? noticed 98/Mar/23 */
		if (perinch != 1440) xview  = (int) (((long) xview * 1440) / perinch);
		if (perinch != 1440) yview  = (int) (((long) yview * 1440) / perinch);
		if (sizeflag == 0) {	/*	scale if requested by user in \special */
			if (xscale != 1.0) xview = (int) ((double) xview * xscale);
			if (yscale != 1.0) yview = (int) ((double) yview * yscale);
		}
		else {	/* size given explicitly, rather than via scale 96/Apr/4 */
#ifdef DEBUGMETAFILE
			if (bDebug > 1) {
				sprintf(debugstr, "xview %d yview %d\n", xview, yview);
				OutputDebugString(debugstr);
			}
#endif
/*	Is this assuming standard dvi_num and dvi_den ??? */
			if (yscale == 0.0) {
				width = (xscale / 65536.0 * 20.0);
				if (xview != 0)
					yview = (int) (width * yview / xview);
				xview = (int) width;
			}
			else if (xscale == 0.0) {
				height = (yscale / 65536.0 * 20.0);
				if (yview != 0)
					xview = - (int) (height * xview / yview);
				yview = - (int) height;
			}
			else {
				xview = (int) (xscale / 65536.0 * 20.0);
				yview = (int) (yscale / 65536.0 * 20.0);
			}
#ifdef DEBUGMETAFILE
			if (bDebug > 1) {
				sprintf(debugstr, "xview %d yview %d\n", xview, yview);
				OutputDebugString(debugstr);
			}
#endif
		}
/*		scale if requested by user in \special */
/*		if (xscale != 1.0) xview = (int) ((double) xview * xscale); */
/*		if (yscale != 1.0) yview = (int) ((double) yview * yscale); */
/*		adjust for current zoom factor */ /* possible overflow problems ??? */
		xview = adjustsize(xview, wantedzoom);
		yview = adjustsize(yview, wantedzoom);
/*		need to scale viewport size appropriately */
		posarr[0].x = 0;
		posarr[0].y = 0;
		posarr[1].x = xview;
		posarr[1].y = yview;

		if (bCopyFlag == 0) {
			ret = LPtoDP(hDC, posarr, 2);	/* problem if CopyFlag ? */
			if (ret == 0) {
				if (bDebug > 1)	OutputDebugString("LPtoDP failed\n");
			}
/*			xview = posarr[1].x - posarr[0].x; */
/*			yview = posarr[1].y - posarr[0].y; */
/*			xview = adjustsize(xview, wantedzoom); */
/*			yview = adjustsize(yview, wantedzoom); */
			xextend = posarr[1].x - posarr[0].x;
			yextend = posarr[1].y - posarr[0].y;
			if (bMetaTopLeft == 0) {			/* 1994/Nov/29 */
#ifdef DEBUGMETAFILE
			if (bDebug > 1) {
				sprintf(debugstr, "SetViewPortOrg %d %d\n", xcurr, ycurr-yextend);
				OutputDebugString(debugstr);
			}
#endif
/*				xcurr, ycurr may not be initialized ??? noticed 98/Mar/26 */
				dret = SetViewportOrgEx(hDC, xcurr, ycurr-yextend, NULL);
/*				if (dret == 0) winerror("SetViewportOrg failed"); */
/*				NO: returns old ViewportOrg, which typically is 0,0 ! */
			}
/* optionally show ViewPort bounding box */
/* xcurr (left), ycurr-yextend (bottom), xcurr+xextend (right), ycurr (top) */
/* (those are device coordinates) */
/* note: origin already shifted at this point */
			if (bShowViewPorts)	{				/* debugging 94/Oct/5 */
				ShowViewPort(hDC, 0, 0,	xview, yview);	/* WORKS! */
#ifdef IGNORED
				if ((bShowViewPorts & 3) == 1) {
					if ((bShowViewPorts & 8) != 0) 
						ShowViewPort(hDC, 0, 0,	xview, yview);	/* WORKS! */
					else ShowViewPort(hDC, dvi_hh, dvi_vv,
						dvi_hh + xview, dvi_vv + yview);
				}
				else if ((bShowViewPorts & 3) == 2) {
					if ((bShowViewPorts & 8) != 0) 
						ShowViewPort(hDC, 0, 0,	xview, -yview);
					else ShowViewPort(hDC, dvi_hh, dvi_vv - yview,
						dvi_hh + xview, dvi_vv);
				}
				else if ((bShowViewPorts & 3) == 3) {
					if ((bShowViewPorts & 8) != 0) 
						ShowViewPort(hDC, 0, 0, xextend, yextend);
					else ShowViewPort(hDC, xcurr, ycurr-yextend,
						xcurr+xextend, ycurr);
				}
#endif
			}
/* CHECK WHETHER ANY OF THIS RECTANGLE LIES WITHIN THE CLIPPING REGION */
/* we need *logical* coordinates TWIPS */
/*			if (metavisible (hDC, dvi_hh, dvi_vv - yview,
					dvi_hh + xview, dvi_vv) == 0) {
					goto skipshow;
			} */ /* Test this earlier so can avoid reading the darn file ! */
#ifdef DEBUGMETAFILE
			if (bDebug > 1) {
				sprintf(debugstr, "SetViewPortExt %d %d\n", xextend, yextend);
				OutputDebugString(debugstr);
			}
#endif
			dret = SetViewportExtEx(hDC, xextend, yextend, NULL); 
			if (dret == 0) {
				if (bDebug > 1)	OutputDebugString("SetViewportExt failed\n");
			}
		}	/* end of if bCopyFlag == 0 */
		else {	/* bCopyFlag != 0 experiment 96/Aug/8 */
/*			OffsetWindowOrgEx(hDC, -dvi_hh, -dvi_vv, NULL); */
/*			ScaleViewportExtEx(hDC, 1, 10, 1, 10, NULL); */
		}
	}	/* end of if (placeable || clipboard) */

#ifdef DEBUGMETAFILE
/*	DumpMetaFile(hDC, hMF); */			/* 1996/Aug/5 */
#endif

/*	Reset `graphics state' to default	1996 Aug 5 */
	SetTextAlign(hDC, TA_NOUPDATECP);	/* default mode */
	SetBkMode(hDC, OPAQUE);				/* default mode */
	SetPolyFillMode(hDC, ALTERNATE);	/* default mode */
/*	SetTextColor(hDC, TextColor); */	/* GetSysColor(COLOR_WINDOWTEXT) */
/*	SetBkColor(hDC, BkColor); */		/* GetSysColor(COLOR_WINDOW) */
/*	SetROP2(hDC, R2_COPYPEN); */
/*	SetTextColor, SetBkColor, SetROP2 */
/*	SetStretchBltMode, SetTextCharacterExtra, SetTextJustification ? */
/*	SetWindowOrgEx(hDC, 0, 0, NULL); */			/* debugging only */
/*	SetWindowExtEx(hDC, 10000, 10000, NULL); */	/* debugging only */

/*	if (bShowMetaFile) */			/* do we show it ? changed 97/Jan/7 */ 
/*		ret = PlayMetaFile(hDC, hMF); */
		if (bCopyFlag == 0) ret = PlayMetaFile(hDC, hMF);
		else {
/*		Write out log file for first WMF copied to clipboard */
#ifdef DEBUGMETAFILE
			DumpMetaFile(hDC, hMF);				/* 1996/Aug/5 */
#endif
			ret = PlayInMetafile(hDC, hMF,
/*		logical rectangle where MetaFile will be played on destination MF */
/*		left, top, right, bottom */
								 dvi_hh, dvi_vv - yview, 
								 dvi_hh + xview, dvi_vv, 
/*		logical extents of destination MetaFile (as in SetWindowExtEx) */
								 xLogExt, yLogExt,
/*		logical extents of MetaFile being played */
								 xHmfLogExt, yHmfLogExt);
#ifdef DEBUGMETAFILE
			if (bDebug > 1) OutputDebugString("After PlayInMetafile\n");
#endif
		}
		if (ret == 0) (void) wincancel("Bad MetaFile");	/* debugging */
/*	} */	/* end of if bShowMetaFile changed 97/Jan/7 */

/*	if (bDebug > 1) OutputDebugString("Before DeleteMetaFile\n"); */
	DeleteMetaFile(hMF);
/*	if (bDebug > 1) OutputDebugString("After DeleteMetaFile\n"); */

/*	(void) SetTextAlign(hDC, TA_BASELINE  | TA_LEFT | TA_UPDATECP); */
/*	(void) SetMapMode (hDC, kMapMode); */

/*	RestoreDC with numeric arg does not work in MetaFile Context! */
/*	But try and avoid -1 arg in case MF has unmatched SaveDC/RestoreDC */
	if (bCopyFlag == 0) RestoreDC(hDC, nSavedDC);
	else RestoreDC(hDC, -1);		/* restore to state before MetaFile */

#ifdef DEBUGMETAFILE
	if (bDebug > 1) OutputDebugString("After RestoreDC\n");
#endif

/*	if (hMem != NULL) {
		(void) GlobalUnlock (hMem);
		(void) GlobalFree(hMem);
		hMem = NULL;
	} */	/* NO, DON'T DO THIS ! */
}

/* The METAFILE should set a mapping mode when it starts */
/* The METAFILE should call SetWindowOrg and SetWindowExt */
/* The METAFILE should NOT call SetViewportExt or SetViewportOrg */
/* The calling program should do this */

/* Code including the METAFILE should call SetViewportExt or SetViewportOrg */

/* *** *** *** *** *** BRAND NEW ATM CODE FOLLOWS *** *** *** *** *** *** */

#define WANTATMCODE TRUE

#ifdef WANTATMCODE

							/* to activate fonts ? or also read ATMREG.ATM */

char *atmdll;				/* ATM DLL atm32.dll name in enhanced mode */
HMODULE hATM=NULL;			/* ATM DLL if loaded (else NULL) */
							/* error code < 32 if failed to load */
#define ATM_NOERR (0)


/* There is a copy of this in dviwindo.c and winfonts.c */

/* The following is called ATMEncodingInfo, *LPATMEncodingInfo in atmpriv.h */

#define ATMDATA ATMEncodingInfo
#define LPATMDATA LPATMEncodingInfo

#ifdef IGNORED
/*	Data structure for `encoding' information in ATM */
/*	Note: could use same data block for given face/style/encoding */
/*        but different size 1996/June/3 */

typedef struct tagATMDATA {
/*	WORD	needInit; */		/* set to one to ask ATM to set up fields */
	short	needInit;			/* set to one to ask ATM to set up fields */
								/* enSetup - first time use or new font */
	WORD	defaultChar;		/* should be minus one (i.e. 255?) */
								/* enMinus - new default character */
	WORD	breakChar;			/* 0, or 20 hex to start at 32 ? or flag */
								/* enOffset - new break character */
	WORD	atmReserved1;		/* unique encoding handle assigned by ATM */
								/* enNumber */
/*	WORD	atmBlock[3*256]; */
	WORD	enGlyphNumber[256];	/* sequential numbers of glyphs */
								/* currCodepoint ? */
	WORD	enWindowsCode[256];	/* corresponding ANSI codes or -1 */
								/* normalCodepoint ? */
	WORD	enWhoKnows[256];	/* who knows what this is! */
								/* fontCodepoint ? */
	LPSTR	lpGlyphNames;		/* pointer to array of 256 LPSTR to glyphs */
/*	DWORD	offGlyphNames; */	/* offset to array of 256 DWORD ? */
								/* enEncoding - relative to start ??? */
	LPSTR	lpCharWidths;		/* pointer to array of 256 WORD widths */
/*	DWORD	offCharWidths; */	/* offset to array of 256 WORD 96/May/28 */
								/* enMetrics - relative to start ??? */
} ATMDATA;

typedef ATMDATA FAR *LPATMDATA;
#endif	/* end of ifdef IGNORED */

/* LPATMDATA hATMData=NULL; */		/* pointer to ATM Encoding structure */

/* Some of the functions in ATM32.DLL that we are interested in ... */

/* In WIN32 we just link through the library (works for Windows 95) */
/* #define MyATMIsUp ATMIsUp */
/* in atm.h */
#define MyATMGetVersion ATMGetVersion
#define MyATMGetBuildStr ATMGetBuildStr
#define MyATMSetFlags ATMSetFlags
#define MyATMFontSelected ATMFontSelected
#define MyATMGetFontBBox ATMGetFontBBox
#define MyATMGetOutline ATMGetOutline
#define MyATMFontAvailable ATMFontAvailable
#define MyATMSelectObject ATMSelectObject	/* not in Windows NT */
#define MyATMGetPostScriptName libATMGetPostScriptName
/* #define MyATMGetFontPaths libATMGetFontPaths */
/* #define MyATMGetMenuName libATMGetMenuName */
/* in atmpriv.h */
#define MyATMSelectEncoding libATMSelectEncoding
#define MyATMGetGlyphList libATMGetGlyphList
/* #define MyATMMarkDC libATMMarkDC */
/* #define MyATMGetFontInfo libATMGetFontInfo */

#ifdef IGNORED
/* In WIN16 we get the links via GetProcAddress after GetModuleHandle */
BOOL (WINAPI *MyATMIsUp) (VOID) = NULL;					/* not used */
WORD (WINAPI *MyATMGetVersion) (VOID) = NULL;
BOOL (WINAPI *MyATMFontSelected) (HDC) = NULL;
int  (WINAPI *MyATMGetFontBBox) (HDC, LPATMBBox) = NULL;
int  (WINAPI *MyATMGetOutline) (HDC, char, LPATMFixedMatrix,
                          FARPROC, FARPROC, FARPROC, FARPROC, LPSTR) = NULL;
int  (WINAPI *MyATMFontAvailable) (LPSTR, int, BYTE, BYTE, BYTE,
						BOOL FAR *) = NULL;
int  (WINAPI *MyATMSelectObject) (HDC, HFONT, WORD, HFONT FAR *) = NULL;
/* the following needed the API BD_VERSION number */
int  (WINAPI *MyATMGetPostScriptName) (int, LPSTR, LPSTR) = NULL;
/* int  (WINAPI *MyATMMarkDC) (int, HDC, LPWORD, LPWORD) = NULL; */
int  (WINAPI *MyATMSelectEncoding) (int, HDC, LPATMDATA) = NULL;
int  (WINAPI *MyATMGetGlyphList) (int, HDC, FARPROC, LPSTR) = NULL;
#endif	/* end of ifdef IGNORED */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* LPSTR   WINAPI lstrcpyn(LPSTR, LPCSTR, int);	*/ /* Windows 3.1 */

// #define lstrncpy strncpy 

#define lmemcpy memcpy 

/* LPSTR WINAPI lmemcpy(LPSTR to , LPCSTR from, int n) {
	LPSTR s=to;
	LPCSTR t=from;
	while (n-- > 0) *s++ = *t++;
	return to;
} */

/* set up global encoding vector with LPSTR pointer array for passing to ATM */
/* make a compact copy of global encoding vector */

/* This converts an encoding stored as an array of fixed length strings */
/* to the compact form used by ATM */

/* Compresses array of fixed length strings (lpVector) */
/* into array of LPSTR pointers to strings for ATM WIN16 */
/* or into array of DWORD offsets from start of EncodingInfo ATM WIN32 */
/* It is assumed here that stuck right on end of EncodingInfo structure */
/* so all offsets are sizeof(LPATMDATA) + offset from start of this */

// void CompressEncoding (LPSTR lpEncoding, LPSTR lpVector) {
void CompressEncoding (LPSTR lpEncoding, LPSTR *lpVector) {
	LPSTR szGlyph;					/* where to insert next glyph name */
	int k;
	LPDWORD GlyphOffset;			/* 96/July/18 */
/*	Offset of glyph name from start of EncodingInfo */
	DWORD Offset;					/* 96/July/16 */

/*	char FAR * FAR *GlyphPtr; */	/* if not WIN32 */

	GlyphOffset = (LPDWORD) lpEncoding;	/* table of glyph name offsets */
/*	Offset is from start of EncodingInfo - assumed contiguous */
	Offset = sizeof(ATMDATA);			/* step over EncodingInfo */
	Offset += 256 * sizeof(WORD);		/* step over bogus width info */ 
	Offset += MAXCHRS * sizeof(DWORD);	/* step over GlyphOffset[MAXCHRS] */

/*	GlyphPtr = (char FAR * FAR *) lpEncoding; */	/* if not WIN32 */

/*	szGlyph = lpEncoding + MAXCHRS * 4; */ /* step over the array of pointers */
	szGlyph = lpEncoding + MAXCHRS * sizeof(DWORD);	/* step over array pntrs */
	for (k = 0; k < MAXCHRS; k++) {

/*		Offset = szGlyph - lpEncoding + 256 * 4; */				/* ??? */
/*		lmemcpy (lpEncoding + k * sizeof(DWORD), (LPSTR) &Offset,
				 sizeof(DWORD)); */						/* 96/July/17 */
		GlyphOffset[k] = Offset;						/* 96/July/18 */

/*		GlyphPtr[k] = szGlyph; */	/* if not WIN32 */

//		lstrcpy (szGlyph, lpVector + k * MAXCHARNAME); 
		if (lpVector[k] != NULL) lstrcpy (szGlyph, lpVector[k]);
		else *szGlyph = '\0';

		Offset += lstrlen(szGlyph) + 1;			/* 96/July/17 */ 

		szGlyph += lstrlen(szGlyph) + 1;		/* allow terminating null */
	}
	*szGlyph = '\0';					/* extra null at the very end */
/* some ifdef IGNORED stuff was here */
}

/* This compresses the encoding into the form used by ATM */
/* shouldn't really bother if bUseNewEncodeT1 is on ??? */
/* Returns globally allocated data area */
/* This may get called in WriteAFM.../WriteTFM... to get at unencoded */

// HGLOBAL CompressEncodingSub (LPSTR lpVector) 
HGLOBAL CompressEncodingSub (LPSTR *lpVector) {
	int k, n;
	int total=0;
	HGLOBAL hEncodingTemp;				/* to distinguish from global */
	LPSTR lpEncoding;					/* to distinguish from global */
	
/*	compute how much space we need */
	for (k = 0; k < MAXCHRS; k++) {
//		n = lstrlen (lpVector + k * MAXCHARNAME);
		if (lpVector[k] != NULL) {
			n = lstrlen(lpVector[k]);
			total += (n+1);
		}
		else total += 1;		// need one byte for empty string
	}
	total++;					/* for extra null at end 96/July/28 */
/*	total += MAXCHRS * 4; */	/* plus, need four bytes per far address */
	total += MAXCHRS * sizeof(DWORD); /* plus, four bytes per far address */
	hEncodingTemp = GlobalAlloc(GMEM_MOVEABLE, total); 
	if (hEncodingTemp == NULL) {
		winerror("Unable to allocate memory");
		return NULL;							/* 1995/Jan/19 */
/*		PostQuitMessage(0); */					/* ??? */
	}
/*	hEncodingTemp = GlobalAlloc (GMEM_FIXED, total); */ /* experiment 95/Jan/5 */
	lpEncoding = GlobalLock(hEncodingTemp);
	if (lpEncoding == NULL) {
		winerror("Unable to allocate memory");
		return NULL;
/*		PostQuitMessage(0); */					/* ??? */
	}
	CompressEncoding(lpEncoding, lpVector);
	GlobalUnlock(hEncodingTemp);
	nEncodingBytes = total;						/* remember how much needed */
	return hEncodingTemp;
}

#ifdef MAKEUNICODE
// int makeUIDtable(LPSTR lpEncoding);
int makeUIDtable(LPSTR *lpEncoding);
#endif

void FreeVector(HGLOBAL hVector) {
	LPSTR *lpVector;
	int k;
	if (hVector == NULL) return;
	lpVector = GlobalLock (hVector);	
	if (lpVector == NULL) return;
	for (k = 0; k < MAXCHRS; k++) {
		if (lpVector[k] != NULL) {
			free(lpVector[k]);
			lpVector[k] = NULL;
		}
	}
	GlobalUnlock(hVector);
}

/*  HGLOBAL SetupEncoding (char *szVector) { */
HGLOBAL SetupEncoding (char *szVector, int atmflag) {	/* 97/Apr/6 */
	HGLOBAL hVector;		/* temporary global for character name array */
//	LPSTR lpVector;
	LPSTR *lpVector;
	HGLOBAL hEncodingTemp;
/*	unsigned long total=0; */
	
	if (szVector == NULL) return NULL;	/* sanity check */

	if (bDebug > 1) {
		sprintf(debugstr, "ENCODING=%s\n", szVector);
		OutputDebugString(debugstr);
	}
	hVector = getencoding(szVector);	/* in winfonts.c */
	if (hVector == NULL) {
		if (bDebug > 1) OutputDebugString("hVector is NULL\n");
		return NULL;
	}

#ifdef MAKEUNICODE
/*	Make mapping table for new encoding method */
/*	if (bUseNewEncode) */
	if (bUseNewEncodeT1 || bUseNewEncodeTT) {
		lpVector = GlobalLock (hVector);
		if (lpVector == NULL) {
			if (bDebug > 1) OutputDebugString("lpVector is NULL\n");
			return NULL;
		}
		makeUIDtable(lpVector);			/* set up UNICODE remapping */
		GlobalUnlock(hVector);
		if (bUseNewEncodeT1) {
			FreeVector(hVector);
			hVector = GlobalFree(hVector);		/* get rid of it again */
			return NULL;	/* don't bother with compressed form for ATM */
		}
	}
#endif

	if (atmflag == 0 || bUseNewEncodeT1) {				/* 97/Apr/6 */
		FreeVector(hVector);
		hVector = GlobalFree(hVector);		/* get rid of it again */
		return NULL;
	}

/*	This is for ATM reencoding only --- don't do if bUseNewEncodeT1 */
	lpVector = GlobalLock(hVector);
	if (lpVector == NULL) {
		if (bDebug > 1) OutputDebugString("lpVector is NULL\n");
		return NULL;
	}
	hEncodingTemp = CompressEncodingSub(lpVector);
	GlobalUnlock(hVector);
	FreeVector(hVector);
	hVector = GlobalFree(hVector);		/* get rid of it again */
	return hEncodingTemp;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

WORD ATMVersion;	/* high byte is version, low byte is sub version */

/* At the point this is called, bDebug can only be set if env var WINDEBUG */

/* Try and connect through ATM library (Windows 95) */
int LoadATMAux32 (void) {			/* 1996/July/16 */
	if (bATMLoaded) return 0;		/* avoid calling ATMProperlyLoaded twice */
	bATMLoaded = ATMProperlyLoaded();
	if (bDebug) {
		sprintf(debugstr, "ATMProperlyLoaded %d\n", bATMLoaded);
		if (bDebug > 1) OutputDebugString(debugstr);
		else winerror(debugstr);
	}
	if (bATMLoaded) return 0;						/* success */
/*	Hopefully the following doesn't happen repeatedly ??? */
/*	WriteError("ATM not properly loaded\n"); */		/* 96/Oct/12 */
	if (! bWinNT)
		WriteError("Not linked to ATM\n");
	return -1;										/* failed */
}

#ifdef IGNORED
/* This loads up Proc Address for ATM and also allocated some memory */
int LoadATMAux (void) {		/* get called only if bUseATM is non-zero */
	
/*	if (bATMLoaded != 0) return 0; */	/* already loaded */

	if (hATM != NULL) return 0;		/* already found DLL (but failed other) */

	if (bWinNT) atmdll="atm.dll";	/* in Windows NT */
	else atmdll="atm32.dll";		/* in Windows 3.1 */

/*	hEncoding = NULL; */			/* reset the global value ??? */

/*	if (OpenFile (atmdll, &OfStruct, OF_EXIST) == -1) {
		MessageBox ((HWND) NULL, "Can't find ATM DLL", "ATM", MB_OK);
		return -1;
	} */
/*	Try and get module handle of ATM DLL if already loaded */
	hATM = GetModuleHandle (atmdll);			/* HMODULE */
/*	if (hATM < HINSTANCE_ERROR) { */			/* wrong ! for GetLibrary() */
	if (hATM == NULL) {							/* 95/Mar/31 */
		sprintf(debugstr, "Can't get %s\n", atmdll);
		if (bDebug > 1) {

/*			ShowLastError(); */
/*			ExplainError(debugstr, GetLastError(), 1); */
			ExplainError(debugstr, -1, 1);

		}
		else WriteError(debugstr);

		return -1;
	}

#ifdef DEBUGATM
	if (bDebug > 1) OutputDebugString("Trying to GetProc\n");
#endif

	if (GetATMProcAddresses(hATM) != 0) return -1;	/* failed 97/Feb/4 */
	bATMLoaded = 1;
	return 0;						/* success */
}
#endif /* endof #ifdef IGNORED */

int LoadATMAuxNew (void);		/* place for LoadATMAuxNew */

/* read encoding vector */ /*  and set up data structure for ATM */
/* and set up UID table if needed */
/* called from dviwindo.c */ /* new 1997/Jan */
/* returns -1 if it fails */

int ReadEncoding(int redoflag) {		/* split out 97/Jan/17 */
	char *s, *szVector=NULL;					/* temp pointer */

	if (redoflag && (hEncoding != NULL)) {
//		FreeVector(hEncoding);			// no, different structure
		hEncoding = GlobalFree(hEncoding);	/* 97/Apr/5 */
	}

/*	if (hEncoding != NULL) return 0; */	/* already set up */

	s = grabenv("ANSIBITS");				/* changed 97/Apr/7 */
	if (s != NULL) sscanf(s, "%d", &bTeXANSI);	/* scan in ANSIBITS=... */
	else bTeXANSI = 0;					/* default 97/Apr/6 */

	szVector = grabenv("ENCODING");	/* will need to make copy to use safely */
	if (szVector == NULL) {
		winerror("ENCODING env var undefined\nPerhaps dviwindo.ini missing?");
		szVector = "texnansi";
	}
	if (szVector != NULL) {				/* 1995/April/10 */
		if (strcmp(szVector, "null") == 0) szVector = NULL;
	}									/* may simplify job of installer... */

	if (szVector != NULL) {
		makelowercase(szVector);		/* lower case encoding vec 97/Apr/5 */
/*		if (strcmp(szVector, "ansinew") != 0) */ /* avoid wasting time? */
/*		setting ENCODING=ansinew and TEXANSI=1 provides way to output */
/*		partial bitmap fonts instead of complete Type 1 fonts .. PSCRIPT */
		if (szReencodingVector != NULL)	free(szReencodingVector);	/* 97/Apr/5 */
/*		szReencodingVector = _strdup(szVector); */		/* keep a copy */
		szReencodingVector = zstrdup(szVector);			/* 95/July/15 */
/*		Trim off .vec if any ... */
		if ((s = strrchr(szReencodingVector, '.')) != NULL) *s = '\0';
/*		extract just name of encoding - minus path if any */
/*		szReencodingName = removepath(szReencodingVector); */
		if (szReencodingName != NULL) free(szReencodingName);
		szReencodingName = zstrdup(removepath(szReencodingVector));
		nCheckSum = codefourty(szReencodingName);	/* in winanal.c */
/*		if (bDebug > 1) OutputDebugString(szReencodingName); */
/*		don't set up reencoding if ENCODING=ansinew (unless TEXANSI=1) */
/*		hence ENCODING=ansinew normally has same effect as no ENCODING */
/*		if want partial bitmapped fonts in printing set TEXANSI=1 also */
/*		if (strcmp(szReencodingName, "ansinew") != 0 ||	bTeXANSI == 1) */
		if ((_stricmp(szReencodingName, "ansinew") != 0 &&
			 _stricmp(szReencodingName, "ansi") != 0) ||
			bTeXANSI == 1) {	/* generic, non-ANSI case */
			hEncoding = SetupEncoding(szReencodingVector, 1); 
#ifdef USEUNICODE
/*			if (bUseNewEncode == 0) */
			if (!bUseNewEncodeTT && !bUseNewEncodeT1) {
				if (hEncoding != NULL) bANSITeXFlag = 0; /* prevent conflict */
				else {
					sprintf(debugstr, "ENCODING %s not found", szReencodingVector);
					if (bDebug) winerror(debugstr);
					else WriteError(debugstr);
				}
			} 
			else bANSITeXFlag = 0;		/* prevent conflict 97/Jan/26 */
#else
			if (hEncoding != NULL) bANSITeXFlag = 0;	/* prevent conflict */
			else {
				sprintf(debugstr, "ENCODING %s not found", szReencodingVector);
				if (bDebug) winerror(debugstr);
				else WriteError(debugstr);
			}
#endif
		}
		else {
#ifdef MAKEUNICODE
/*			get here when szReencodingVector is "ansinew" (and TEXANSI=0) */
			if (bUseNewEncodeT1 || bUseNewEncodeTT) {
/*			if we use new method for reencoding, need to set up table anyway */
				hEncoding = SetupEncoding (szReencodingVector, 0);
			}
			else
#endif
			if (bDebug > 1) OutputDebugString("NOT reencoding\n");
		}
	}

/*	if (hEncoding == NULL) winerror("hEncoding == NULL!"); */
	if (hEncoding == NULL) return -1;	/* failed to set up */
#ifdef DEBUGATM
	if (bDebug > 1) OutputDebugString("hEncoding != NULL");
#endif
	return 0;
}

/* initialize ATM data */

void ATMDataInit (LPATMDATA lpATMData) {	/* separated out 96/July/16 */
#ifdef DEBUGATM
	if (bDebug > 1) OutputDebugString("ATMDataInit\n");
#endif
#ifdef DEBUGHEAP
	CheckHeap("ATMDataInit", 0);
	sprintf(debugstr, "%08X", lpATMData);
	wininfo(debugstr);	// debugging only
#endif

	lpATMData->needInit = 1;		/* tell ATM it is the first time */
/*	lpATMData->defaultChar = 0xFFFF; */	/* always -1 */
	lpATMData->defaultChar = DefaultChar;	/* always -1 */
/*	lpATMData->breakChar = 0; */		/* 0 or 20h ? */
	lpATMData->breakChar = BreakChar;		/* 0 or 20h ? */
	lpATMData->atmReserved1 = 0;	/* unique number to be assigned by ATM */
/*	memset ((LPSTR) lpATMData+8, 0, 3*512); */	/* debugging test only */

	lpATMData->offCharWidths = sizeof(ATMDATA);
	lpATMData->offGlyphNames = sizeof(ATMDATA) + nWidthBytes;

#ifdef DEBUGHEAP
	CheckHeap("ATMDataInit", 0);
#endif
//	#ifndef WIN32
/*	lpATMData->lpCharWidths = (LPSTR) (long) -1;  */
//  #endif
}

/* int SetupATMSelectEncoding(redoflag) */	/* split off 97/Apr/5 */
int SetupATMSelectEncoding(int redoflag) {	/* split off 97/Apr/5 */
	LPATMDATA lpATMData;
	LPSTR lpEncoding;				/* 96/July/21 */

/*	(void) ReadEncoding(); */		/* now mess with encoding vector ... */
/*	redundant - already done */ /* we assume encoding has been loaded */

/*	this has now been moved earlier into dviwindo.c before LoadATM */
/*	so it can also be used with bUseNewEncode in Windows NT */

/*	The above should set up nEncodingBytes 96/July/20 */

/*	Allocate ShowFont ATM data only once ? What if size changes ? */
/*	if (hEncoding != NULL) */
/*	But, we need this even if ENCODING is NULL ! WriteAFM... WriteTFM... */
/*	Do the ATM Data for those in temporary space ? */
	if (redoflag && (hATMShow != NULL))
		hATMShow = GlobalFree(hATMShow);	/* 97/Apr/5 */

	bCustomEncoding=0;
/*	need contiguous space also for width table and compressed encoding vector */
//	nWidthBytes = 256 * sizeof(WORD); // redundant ?
	hATMShow = GlobalAlloc(GMEM_MOVEABLE,
							sizeof (ATMDATA) + nWidthBytes +
							nEncodingBytes);

/*	hATMShow = GlobalAlloc(GMEM_MOVEABLE, sizeof (ATMDATA));  */

	if (hATMShow ==  NULL) {
		winerror ("Unable to allocate memory");
		return -1;						/* 1995/Jan/19 */
/*		PostQuitMessage(0); */			/* ??? */
	}
	lpATMData = GlobalLock(hATMShow);
	ATMDataInit(lpATMData);

/*	zero out bogus width data */	/* 96/July/21 */
	memset ((LPSTR) lpATMData + sizeof(ATMDATA), 0, nWidthBytes);
	lpATMData->offCharWidths = sizeof(ATMDATA);
/*	stick in compressed encoding vector */
	if (hEncoding != NULL) {
		lpEncoding = GlobalLock(hEncoding);
		lmemcpy ((LPSTR) lpATMData + sizeof(ATMDATA) + nWidthBytes, lpEncoding,
				 nEncodingBytes);
		GlobalUnlock(hEncoding);
	}
	else {
		if (bDebug > 1) OutputDebugString("hEncoding == NULL\n");
/*		return -1; */					/* experiment 97/Dec/21 */
	}
	lpATMData->offGlyphNames = sizeof(ATMDATA) + nWidthBytes;

/*	lpATMData->lpCharWidths = (LPSTR) (long) -1; */

	GlobalUnlock(hATMShow);
/*	} */
	if (hEncoding == NULL) return -1;	/* failed - experiment 97/Dec/21 */
	return 0;					/* success */
}

int LoadATM (void) {				/* returns zero if it fails */
	WORD ret;
/*	LPATMDATA lpATMData; */			/* pointer into ATM data */
/*	char *s, *szVector; */			/* temp pointer (to str sometimes) */
/*	LPSTR lpEncoding; */
	WORD ATMFlags;

	if (bATMLoaded != 0) return 0;		/* already loaded */

	if (bDebug > 1) OutputDebugString("Trying to load ATM\n");

/*	nCheckANSI = codefourty("ansinew"); */	/* in winanal.c */

/*	hEncoding = NULL; */		/* reset the global value ??? */

/*	Remove ATM information from previous tries 96/Oct/12 */
	(void) WritePrivateProfileString(achDiag, "ATMVersion", NULL, achFile);
	(void) WritePrivateProfileString(achDiag, "ATMBuildStr", NULL, achFile);
	(void) WritePrivateProfileString(achDiag, "ATMFlags", NULL, achFile);

	if (LoadATMAux32() != 0) {			/* connect through atm.lib */
/*		Have we set bWinNT yet ? Yes */
		if (bDebug > 1) {
			sprintf(debugstr, "LoadATMAux: dWin95 %d, dWinNT %d\n",
			   bWin95, bWinNT);
			OutputDebugString(debugstr);
		}
/*		if (bWinNT) {
			if (LoadATMAuxNew() != 0) return 0;	
		} */ /* debugging hack */
		return 0;		/* exit because of failure */
	}

/*	if (LoadATMAux() != 0) return 0; */

#ifdef DEBUGATM
	if (bDebug > 1) OutputDebugString("ATMIsLoaded\n");
#endif

	if (! bATMLoaded) {			/* Not really useful test */
		winerror("ATM not running"); 
		return -1;
	}

#ifdef IGNORED
	ret = MyATMIsUp();
	if (ret == 0) {				/* How can this happen ? */
		winerror("ATM not running"); 
		return -1;
	}
#endif

#ifdef DEBUGATM
	if (bDebug > 1) OutputDebugString("ATMIsUp\n");
#endif

/*	ATMVersion = MyATMGetVersion(); */
	ret = MyATMGetVersion();
/*	ATMVersion = (LOBYTE(ret) << 8) | HIBYTE(ret); */
/*	ATMVersion = (LOBYTE(ret) << 8) | HIBYTE(ret); */
	ATMVersion = (WORD) ((LOBYTE(ret) << 8) | HIBYTE(ret));	/* 99/Mar/17 */
/*	should be at least ATM version 2.6, preferrably 3.0 */
/*	if (((LOBYTE(ATMVersion) << 8) | HIBYTE(ATMVersion)) >= 0x0205) */
/*	if (((LOBYTE(ATMVersion) << 8) | HIBYTE(ATMVersion)) > 0x0205) */
/*	if (ATMVersion > 0x0205) bATMLoaded = 1; */
	if (ATMVersion < 0x0206) {
/*		if (bDebug > 1) OutputDebugString("Old ATM Version\n"); */
		winerror("Need ATM version 2.6 or later");
		bATMLoaded = 0;
		return -1;
	}
/*		ATMVersionInfo ver; */
/*		(void) ATMGetVersionEx(&ver) */
/*		ver.wdDrvMajorVersion, ver.wdDrvMinorVersion */
/*		version of ATM.DLL (Win95) ATMDRVR.DLL (WinNT) */
/*		ver.wdLibMajorVersion, ver.wdLibMinorVersion */
/*		zero (Win95) version ATMLIB.DLL (WinNT) */
/*		ver.szBuildStrings[128] */
/*		one or two build strings (Driver and Library) */

/*	default for AutoActivate is determined by ATM version */
/*  this is before reading user preferences ? 1996/June/4 */
/*	bAutoActivate = (LOBYTE(ret) >= 4); */
/*	if (bWin95) bAutoActivate = (HIBYTE(ATMVersion) >= 4); */
/*	else bAutoActivate = 0; */			/* not in Windows NT ? */
/*	right now we only come here in Windows 95/98 anyway ? */
	bAutoActivate = (HIBYTE(ATMVersion) >= 4);	/* 1997/Sep/14 ??? */

/*  is this before reading user preferences ??? 1996/June/4 */
/*	bATM4 = (LOBYTE(ret) >= 4); */
	bATM4 = (HIBYTE(ATMVersion) >= 4);	/* do also in Windows NT ? */

/*	New debugging output in dviwindo.ini 1996/June/4 */
/*	wsprintf(str, "%2d.%02d", LOBYTE(ret), HIBYTE(ret)); */
	wsprintf(str, "%2d.%02d", HIBYTE(ATMVersion), LOBYTE(ATMVersion));	
	(void) WritePrivateProfileString(achDiag, "ATMVersion", str, achFile);
	if (bDebug > 1) {
		sprintf(debugstr, "ATMVersion %s\n", str);
		OutputDebugString(debugstr);
	}

	*str = '\0';
	MyATMGetBuildStr(str);
	(void) WritePrivateProfileString(achDiag, "ATMBuildStr", str, achFile);
	if (bDebug > 1) {		
		sprintf(debugstr, "ATMBuildStr %s\n", str);
		OutputDebugString(debugstr);
	}
/*	should we set ATMAUTOACTIVATE if bAutoActivate != 0 ??? */
/*	if we do this, does it have to be reset at the end ??? */
/*	read out ATM flag settings - mask == 0 means don't change flags ? */
/*	ATMFlags = MyATMSetFlags (0, (WORD) -1); */
/*	(void) MyATMSetFlags (ATMFlags, (WORD) -1); */
/*	used only to read out flags presently */
	ATMFlags = MyATMSetFlags (0, (WORD) 0);
	sprintf(str, "%0X", ATMFlags); 
	(void) WritePrivateProfileString(achDiag, "ATMFlags", str, achFile);
	if (bDebug > 1) {
		sprintf(debugstr, "ATMFlags %0X: Activate %d, Smooth %d, DevFonts %d, GDIFonts %d, Download %d, Subst %d\n",
			ATMFlags,
			(ATMFlags & ATMAUTOACTIVATE) != 0,
			(ATMFlags & ATMANTIALIAS) != 0,
			(ATMFlags & ATMUSEDEVFONTS) != 0,
			(ATMFlags & ATMGDIFONTS) != 0,
			(ATMFlags & ATMDOWNLOAD) != 0,
			(ATMFlags & ATMSUBSTITUTE) != 0);
		OutputDebugString(debugstr);
	}

/*	Don't set up the ATM SelectEncoding table in Windows NT */

/*	if (bWinNT) return 0; */
	if (!bWinNT) {
		return SetupATMSelectEncoding(0);		/* 97/Apr/5 */
	}
	return 0;
}

void UnloadATM (void) {			/* 1996/July/16 */
	int ret;
	if (bATMLoaded) {
		ret = ATMFinish();
		if (bDebug > 1) {
			sprintf(debugstr, "ATMFinish %d\n", ret);
			OutputDebugString(debugstr);
		}
		bATMLoaded = 0;
		hATM = NULL;		/* 97/Jan/30 */
	}
}

/* do the GlobalUnlock / GlobalLock once per page ? */

BOOL allowreencode (HDC hDC) {	/* check for allowing reencoding - Show Font */
	TEXTMETRIC TextMetric;				/* used by font sample module */
	char FaceName[MAXFULLNAME]="";	/* LF_FULLFACESIZE just in case 95/July/7 */
	int ret;
	if (hDC == NULL) return FALSE;		/* ??? */
	if (GetTextMetrics(hDC, &TextMetric) == 0) return FALSE;
	(void) GetTextFace(hDC, sizeof(FaceName), FaceName);	/* 98/Sep/25 */
/*	that last argument should be ttfflag */
	ret = isansi (TextMetric, FaceName, 1);	/* in winanal.c */
	if (bDebug > 1) {
		sprintf(debugstr, "%d Face: %s fontttf: %d textfont: %d",
				-1, FaceName, 1, ret);
		OutputDebugString(debugstr);
	}
	return ret; 
}

/* Show FaceName and Style of font currently selected into hDC */

/* void ShowhDCFontInfo(HDC hDC, int verboseflag) */
void ShowhDCFontInfo (HDC hDC) {
#ifdef DEBUGENCODING
	char FaceName[MAXFULLNAME]="";	/* LF_FULLFACESIZE just in case 95/July/7 */
	int boldflag, italicflag, regularflag;
	TEXTMETRIC TextMetric;
#endif 

	if (bDebug < 2) return;		/* only go in here if debugging mode */
	if (bCopyFlag) return;		/* Don't try this in MetaFile hDC */

#ifndef DEBUGENCODING
//	if (verboseflag == 0) return;
#endif

#ifdef DEBUGENCODING
/*	if (bDebug > 1) OutputDebugString("Before\n"); */
/*	(void) GetTextFace(hDC, MAXFACENAME, FaceName);	 */
	(void) GetTextFace(hDC, sizeof(FaceName), FaceName);	
	if (*FaceName == '\0') {
	    if (bDebug > 1) OutputDebugString("Empty Face Name?\n");
	    return;					/* failed probably MetaFile hDC ... */
	}
/*	if (bDebug > 1) OutputDebugString("After\n"); */
	(void) GetTextMetrics(hDC, &TextMetric);
	boldflag = (TextMetric.tmWeight > FW_NORMAL);
	italicflag = TextMetric.tmItalic;
	if (boldflag || italicflag) regularflag = 0;
	else regularflag = 1;
	wsprintf(debugstr, "Face: %s,%s%s%s\n",
		(LPSTR) FaceName,
			regularflag ? (LPSTR) " REGULAR" : (LPSTR) "",
			 boldflag ? (LPSTR) " BOLD" : (LPSTR) "",
			  italicflag ? (LPSTR) " ITALIC" : (LPSTR) "");
	if (bDebug > 1) OutputDebugString(debugstr);
#endif
}

int fnlast=-1;		/* remember last font encoded for debugging output */
					/* just for debugging output in UnencodeFont */

/* fn is internal DVIWindo font number */
/* if fn == -1 this is temporary - call from Show Font */

/* metricflag is 1 for metric font (width info) */
/* metricflag is -1 to force reencoding even if not ANSI */

/* can call from DVI file display, ShowFont, and WriteAFM... */

/* Use ATM to reencode a font - don't do this if bUseNewEncode set */
/* don't try this for TT fonts */

/* void ReEncodeFont (HDC hDC, int fn, int firsttime) */
int ReencodeFont (HDC hDC, int fn, int metricflag) {
	LPATMDATA lpATMData;			/* pointer into ATM data */
/*	LPATMDATA lpATMBase; */			/* pointer into ATM data */
	LPSTR lpEncoding;				/* pointer into Encoding data */
	int ret;
	BOOL bNewTableFlag=0;			/* if new table - need initialization */
#ifdef ATMSOLIDTABLE
	ATMDATA __huge *u;				/* 95/Nov/12 */
#endif
	int fnbase;						/* base font for this name and style */

	if (bCopyFlag) return -1;		/* 95/Mar/17 */

	if (bWinNT) return -1;			/* 97/Mar/2 redundant ? */

#ifdef USEUNICODE
	if (bUseNewEncodeT1) return -1;	/* sanity check 97/Jan/16 */
#endif

/*	if (hEncoding == NULL) return -1; */ /* redundant ? */

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

/*	Check first whether ATM is still in reencoded state ! */
	if (bATMReencoded)	UnencodeFont (hDC, fn, 1); /* shouldn't happen! */

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

/*	Do check only if its a font called for by DVI file */
	if (fn >= 0) {
#ifdef IGNORED
		sprintf(str, "ttf %d tex %d ansi %d",
				fontttf[fn], texfont[fn], ansifont[fn]);
		wininfo(str);					/* DEBUGGING ONLY */
#endif
		if (fontttf[fn]) return -1;		/* can't do TTF fonts */
		if (!ansifont[fn]) return -1;	/* only can do text fonts */
/*		if (texfont[fn]) return -1; */	/* don't do TeX remapped fonts */
										/* removed 97/Sep/11 dangerous ??? */
#ifdef DEBUGENCODING
		if (bDebug > 1) {
			char *sfname;
			if (subfontname[fn] != NULL) sfname = subfontname[fn];
			else sfname = "UNKNOWN";
			wsprintf(str, "Reencoding %d (%04X) Face: %s %s\n",
				fn, (int) hDC, sfname, /* (LPSTR) subfontname[fn], */
	fontbold[fn] ? (LPSTR) "Bold" : (LPSTR) "",
	fontitalic[fn] ? (LPSTR) "Italic" : (LPSTR) "",
	metricflag ? (LPSTR) "  (metric)" : (LPSTR) "");
			OutputDebugString (str);
		}
#endif
	}	/* end of fn >= 0 case (i.e. font in DVI file) */
	else {	/*	Now for reencoding needed in ShowFont, WriteAFM etc */
		if (metricflag != -1) {			/* so can force in WriteAFMFile */
			if (allowreencode (hDC) == 0) {
#ifdef DEBUGENCODING
				if (bDebug > 1) {
					OutputDebugString ("Only reencode plain text font ");
/*					ShowhDCFontInfo(hDC, 0); */
					ShowhDCFontInfo(hDC);	/* not verbose */
				}
#endif
				return -1;	/* not ANSI */
			}
		}
#ifdef DEBUGENCODING
		if (bDebug > 1) {
/*			wsprintf(str, "Reencoding (%04x)\n", (int) hDC); */
			wsprintf(str, "Reencoding (%04X) ", (int) hDC);
			OutputDebugString (str);
		}
#endif
	} /* end of reencoding needed in ShowFont, WriteAFM etc */

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

/*	if (bDebug > 1) ShowhDCFontInfo(hDC, 0); */	/* not verbose */
	if (bDebug > 1) ShowhDCFontInfo(hDC);	/* not verbose */

	if (hDC == NULL) {
		if (bDebug > 1) OutputDebugString("Reencode NULL hDC\n");
		return -1;
	}

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

/*	first check whether font selecting in current hDC is an ATM font ? */
/*	if (bDebug > 1) OutputDebugString("BeFoRe\n"); */
	if (!MyATMFontSelected (hDC)) {
		if (bDebug > 1) {
			OutputDebugString("Reencode: current not ATM font ");
			if (fn >= 0) {
				char *fname="UNKNOWN";
				if (fontname[fn] != NULL) fname = fontname[fn];
/*				OutputDebugString(fontname[fn]); */
				OutputDebugString(fname);
			}
			ShowhDCFontInfo(hDC);		/* verbose */
		}
		return -1;  /* 95/March/17 is it safe to pop out here ??? */
	}
/*	if (bDebug > 1) OutputDebugString("AfTeR\n"); */

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

/*	should never get here if hEncoding == NULL ... */
	if (hEncoding == NULL) {
		winerror("hEncoding NULL");
		return -1;
	}					/* debugging */

	lpEncoding = GlobalLock (hEncoding);
	if (fn >= 0) {
#ifdef ATMSOLIDTABLE
		if (hATMTable == NULL) {				/* debugging */
			winerror("hATMTable NULL");
			return -1;
		}
		lpATMData = GlobalLock (hATMTable);
/*		following is fixed 95/Nov/12 */ /* need to use huge pointers */

		lpATMData = lpATMData + fn;			/* OK only for fn < 43 */

/*		u = lpATMData; */
/*		u = u + fn; */
/*		lpATMData = u; */

#else  /* if not ATMSOLIDTABLE */
/*		Does this chew up a lot of segment table entries in WIN16 ? */
/*		Use lowest numbered font slot of this name and style */ 
/*		Typically fnbase == fn */ /* with luck fnbase < fn */
		if (bUseBaseFont) fnbase = basefont[fn];		/* 1996/July/26 */
		else fnbase = fn;			/* short-circuit --- debugging test */
/*		This way different sizes share same ATM data block */

		if (hATMTables[fnbase] == NULL) {		/* not set up yet ? */
			hATMTables[fnbase] = GlobalAlloc(GMEM_MOVEABLE,
					sizeof(ATMDATA) + nWidthBytes + nEncodingBytes);

//		if (hATMTables[fnbase] == NULL) 
//			hATMTables[fnbase] = GlobalAlloc(GMEM_MOVEABLE, sizeof(ATMDATA));

			if (bDebug > 1) { 
				char *sfname;
				if (subfontname[fn] != NULL) sfname = subfontname[fn];
				else sfname = "UNKOWN";
#ifdef DEBUGATM
				sprintf(debugstr, "ATM Data for %d (%d) Face: %s,%s%s%s\n",
						fn, fnbase, sfname, /* subfontname[fn], */
	(!fontbold[fn] && !fontitalic[fn]) ? "Regular" : "",
	fontbold[fn] ? "Bold" : "", fontitalic[fn] ? "Italic" : "");
				OutputDebugString(debugstr);
#endif
			}
/*			if (hATMTables[fn] == NULL) */
			if (hATMTables[fnbase] == NULL) {
				winerror ("Unable to allocate memory");
				PostQuitMessage(0);			/* pretty serious ! */
/*				return -1; */
			}
			bNewTableFlag = 1;				/* indicate need to fill in */
		}
/*		lpATMData = GlobalLock (hATMTables[fn]); */
		lpATMData = GlobalLock (hATMTables[fnbase]); 
#endif  /* if not ATMSOLIDTABLE */
	}
	else {			/* i.e. if fn < 0 */
		if (hATMShow == NULL) {				/* debugging */
			winerror ("hATMShow NULL");
			return -1;
		}
		lpATMData = GlobalLock (hATMShow);
		fnbase = fn;			/* 98/Mar/26 ??? uninit */
	}

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
	sprintf(debugstr, "fn %d bNewTableFlag %d", fn, bNewTableFlag);
	wininfo(debugstr);	// debugging only
#endif
	
	if (fn < 0) {						/* ShowFont, WriteAFM case etc */
		if (bATMShowRefresh) {			/* see whether font or style changed */
			ATMDataInit(lpATMData);
			bATMShowRefresh=0;			/* note that we *did* refresh it */
		}
/*		don't make up new encoding if FaceName and Style have not changed */
	} /* end of fn < 0 case */
#ifdef ATMSOLIDTABLE
	else if (encodefont[fn] == 0) {		/* first time ask ATM to set up */
#else
	else if (bNewTableFlag) {
#endif
		LPSTR lpEncoding;
		lpEncoding = GlobalLock (hEncoding);	 /* redundant */
		ATMDataInit(lpATMData);

#ifdef DEBUGHEAP
		CheckHeap("before memset", 0);
#endif
/*		reset width data ??? */
		memset ((LPSTR) lpATMData + sizeof(ATMDATA), 0, 256 * sizeof(WORD)); 

#ifdef DEBUGHEAP
		CheckHeap("before memcpy", 0);
#endif

#ifdef IGNORED
		sprintf(debugstr, "nwidthBytes %d nEncodingBytes %d",
				nWidthBytes, nEncodingBytes);
		wininfo(debugstr);
#endif

/*		copy compressed encoding vector */
		lmemcpy((LPSTR) lpATMData + sizeof(ATMDATA) + nWidthBytes,
				lpEncoding, nEncodingBytes);

#ifdef DEBUGHEAP
		CheckHeap("after memcpy", 0);
#endif

/*		lpATMData->lpGlyphNames = lpEncoding; */
/*		lpATMData->lpCharWidths = (LPSTR) (long) -1; */

		GlobalUnlock(hEncoding);			 /* redundant ? */
	}
	else { /* fn >= 0 && encodefont[fn] != 0 */	/* do a sanity check first */
		if (lpATMData->needInit != 0 ||
/*			lpATMData->defaultChar != 0xFFFF || */
			lpATMData->defaultChar != DefaultChar ||
/*			lpATMData->breakChar != 0 || */
			lpATMData->breakChar != BreakChar ||
			lpATMData->atmReserved1 == 0) {
/* debugging output when reencoding produced unexpected results ... */
			if (bDebug > 1) { /* && fontname[fn] != NULL) */
				char *fname="UNKNOWN";
				if (fontname[fn] != NULL) fname = fontname[fn];
				wsprintf (debugstr,
"Before. Font %d (%s) Init %d (0), Default %d (255), Break %d (0), Reserved %d (> 0)\n",
					fn, (fn < 0) ? (LPSTR) "" : fname, /* fontname[fn], */
					  lpATMData->needInit, lpATMData->defaultChar,
						  lpATMData->breakChar, lpATMData->atmReserved1);
				OutputDebugString(debugstr);
			} /* debugging output 95/Nov/12 */
/*			return -1; */	/* not safe, need to GlobalUnlock ???? */
/*			following is feeble attempt to fix ??? 95/Nov/12 */ 
			ATMDataInit(lpATMData);
		}
	} 

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

	if (lpEncoding == NULL) {				/* debugging */
		winerror("lpEncoding NULL");
		return -1;
	}

/*	if encoding selected is not the one in ENCODING=... */
/*	need to make temporary copy and put that encoding in there */
	if (bCustomEncoding) {
		HGLOBAL hATMCopy;
		LPATMDATA lpATMCopy;
		int total=0;
		LPSTR lpEncoding;
		lpEncoding = GlobalLock(hEncoding);	/* redundant */
		total = sizeof(ATMDATA);	/* for copy of lpATMData */
		total += nWidthBytes;		/* for bogus width data */
		total += nEncodingBytes;	/* for copy of encoding vector */
		hATMCopy = GlobalAlloc(GMEM_MOVEABLE, total);
		lpATMCopy = GlobalLock(hATMCopy);
		if (lpATMCopy == NULL) {
			winerror("Unable to allocate memory");
			PostQuitMessage(0);			/* pretty serious ! */
		}
/*		copy ATM info structure */
		lmemcpy((LPSTR) lpATMCopy, (LPSTR) lpATMData, sizeof(ATMDATA));
/*		reset width data ??? */
		memset ((LPSTR) lpATMCopy + sizeof(ATMDATA), 0, 256 * sizeof(WORD)); 
/*		copy compressed encoding vector */
		lmemcpy((LPSTR) lpATMCopy + sizeof(ATMDATA) + nWidthBytes, lpEncoding, nEncodingBytes);
/*		ret = MyATMSelectEncoding (BD_VERSION, hDC, lpATMData); */
		ret = MyATMSelectEncoding (BD_VERSION, hDC, lpATMCopy); 
/*		copy back modified ATM info structure ??? */
/*		lmemcpy((LPSTR) lpATMData, (LPSTR) lpATMCopy, sizeof(ATMDATA)); */
/*		GlobalUnlock (lpVector); */
		GlobalUnlock(hEncoding);		/* redundant */
		GlobalUnlock(hATMCopy);
		GlobalFree(hATMCopy);
	}

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

/*	EncodingInfo is followed by bogus width data and compressed encoding */
	lpATMData->offCharWidths = sizeof(ATMDATA);
	lpATMData->offGlyphNames = sizeof(ATMDATA) + nWidthBytes;
	ret = MyATMSelectEncoding (BD_VERSION, hDC, lpATMData); 

/*	lpATMData->lpGlyphNames = lpEncoding; */
/*	lpATMData->lpCharWidths = (LPSTR) (long) -1; */
/*	ret = MyATMSelectEncoding (BD_VERSION, hDC, lpATMData); */

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

	if (ret != ATM_NOERR) {
		if (bDebug > 1) {
			wsprintf(debugstr, "ATM Error %d (%s)\n", ret, (LPSTR) "Reencode");
			OutputDebugString(debugstr);
			wsprintf(debugstr, "VER %d hDC %08X Data %08X Encoding %08X\n",
					 BD_VERSION, hDC, &lpATMData, &lpEncoding);
			OutputDebugString(debugstr);	 
		}
	}
/*	reinstated debugging check 96/May/28 */
	if (lpATMData->needInit != 0 ||
/*		lpATMData->defaultChar != 0xFFFF || */
		lpATMData->defaultChar != DefaultChar ||
/*		lpATMData->breakChar != 0 || */
		lpATMData->breakChar != BreakChar ||
		lpATMData->atmReserved1 == 0) {
		if (bDebug > 1) {
			char *fname="UNKNOWN";
			if (fontname[fn] != NULL) fname = fontname[fn];
			wsprintf (debugstr,
"After. Font %d (%s) Init %d (0), Default %d (255), Break %d (0), Reserved %d (> 0)\n",
				fn, (fn < 0) ? (LPSTR) "" : fname, /* (LPSTR) fontname[fn], */
				  lpATMData->needInit, lpATMData->defaultChar,
					  lpATMData->breakChar, lpATMData->atmReserved1);
			OutputDebugString(debugstr);
		} 
	}	/* debugging check 1995/Nov/12 */

#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif

/* fnbase may be uninitialized ??? may be -1 ???*/

	if (fn >= 0) {
#ifdef ATMSOLIDTABLE
		GlobalUnlock(hATMTable);
		encodefont[fn] = 1;			/* note that this one has been set up */
#else
/*		GlobalUnlock (hATMTables[fn]); */
		GlobalUnlock(hATMTables[fnbase]);
/*		the fact that hATMTables[fn] != NULL means it has been set up */
#endif
	}
	else {
		 GlobalUnlock(hATMShow);
/*		 GlobalFree (hATMShow);	*/	/* is it safe to do this ? NOT */
	}
	GlobalUnlock(hEncoding);
	bATMReencoded = 1;				/* note that we are in reencoded state */
	fnlast = fn;	/* remember last font reencoded - for debug output only */
#ifdef DEBUGHEAP
	CheckHeap("ReencodeFont", 0);
#endif
	return 0;
}
	
/* void UnencodeFont (HDC hDC, int fn) */
int UnencodeFont (HDC hDC, int fn, int cleanflag) {
	int ret;
/*	int nVersion=3; */

	if (bCopyFlag) return -1;		/* 1995/Mar/17 */

	if (bWinNT) return -1;			/* 1997/Mar/2 redundant ? */

#ifdef USEUNICODE
	if (bUseNewEncodeT1) return -1;	/* sanity check 1997/Jan/16 */
#endif

	if (hDC == NULL) {
		if (bDebug > 1) OutputDebugString("Unencode NULL hDC\n");
		return -1;
	}

/*	first check whether font selecting in current hDC is an ATM font ? */
	if (!MyATMFontSelected (hDC)) {
		if (bDebug > 1) {
			OutputDebugString("Unencode: current not ATM font ");
			if (fn >= 0) {
				char *fname="UNKNOWN";
				if (fontname[fn] != NULL) fname = fontname[fn];
/*				OutputDebugString(fontname[fn]); */
				OutputDebugString(fname);
			}
			ShowhDCFontInfo(hDC);
		}
		return -1;			 /* 95/March/17 is it safe to pop out here ??? */
	}

	if (bDebug > 1) {
/*		if (fn >= 0) */
		if (fnlast >= 0) {
#ifdef DEBUGENCODING
			char *sfname;
			if (subfontname[fnlast] != NULL) sfname = subfontname[fnlast];
			else sfname="UNKNOWN";
			wsprintf(debugstr, "Unencoding %d (%04X) Face: %s %s\n", 
				fnlast, (int) hDC, sfname, /*  subfontname[fnlast], */
	(cleanflag) ? (LPSTR) "  (clean)" : (LPSTR) "");
			OutputDebugString (debugstr);
#endif
			fnlast = -1;
		}
		else {
#ifdef DEBUGENCODING
			wsprintf(str, "Unencoding (%04X)\n", (int) hDC); 
/*			wsprintf(str, "Unencoding (%04x)\t", (int) hDC); */
			OutputDebugString (str);
#endif
		}
	}
/*	if (bDebug > 1) ShowhDCFontInfo(hDC, 0); */  /* shows NEW font ? ... */
	if (bDebug > 1) ShowhDCFontInfo(hDC);  /* shows NEW font ? ... */
	ret = MyATMSelectEncoding (BD_VERSION, hDC, NULL);	/* undo encoding */
	if (ret != ATM_NOERR) {
		if (bDebug > 1) {
			wsprintf(debugstr, "ATM Error %d (%s)\n", ret, (LPSTR) "Unencode");
			OutputDebugString(debugstr);
		}
	}
	bATMReencoded = 0;
	return 0;
}

#endif // end of ifdef WANTATMCODE ???

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Used to wrap file names that may potentially contain spaces ... */
/* NOTE: "..." in ini file values are stripped by GetPrivateProfileString */

/* use quotestring in winprint.c instead */

#ifdef IGNORED
char *wrapinquotes (char *str) {					/* 1997/July/26 */
	strcat(str, "\"");
	memmove(str+1, str, strlen(str)+1);
	*str = '\"';
	return str;
}
#endif

/* Keep MAXDOCUMENTS to 10 or less so we can use one digit in File<n>=... */

/* [Documents] */
/* File1=c:\dvitest\foo.dvi */
/* File2=c:\users\default\bar.dvi */

/* char *achDocs="Documents"; */
/* char *achFile="dviwindo.ini"; */

/* int ndocuments=0; */

int RemoveDocument (char *path, char *file) {				/* 1995/Sep/17 */
	int k, n, klast=0;
	int kexist;
	char *s;
	char keyname[]="File1";
	char buffer[MAXFILENAME];
	char name[MAXFILENAME];

	if (strchr(file, ':') == NULL &&
		strchr(file, '\\') == NULL &&
		strchr(file, '/') == NULL) {
		strcpy(name, path);
		s = name + strlen(name) - 1;
		if (*s != '\\' && *s != '/') strcat(name, "\\");
		strcat(name, file);
	}
	else strcpy(name, file);

#ifdef DEBUGMRU
	if (bDebug > 1) {
		sprintf(buffer, "RemoveDocuments path %s file %s name %s\n", path, file, name);
		OutputDebugString(buffer);
	}
#endif

	kexist = -1;
/*	First check whether on list, and mark place */
	for (k = 0; k < nMaxDocuments; k++) {
		keyname[4] = (char) ('1' + k);
		n = GetPrivateProfileString(achDocs, keyname, "",
				buffer, sizeof(buffer), achFile); 
		if (n == 0) continue;			/* must have hit end (or gap ?) */
/*		if (_stricmp(buffer, name) == 0) return -1; */	/* match found */
		if (_stricmp(buffer, name) == 0) {
			if (kexist < 0)	kexist = k;	/* remember where (first occurence) */
/*			break; */					/* no need to continue to set klast */
		}
		klast = k;						/* last number actually used */
	}
	if (kexist < 0) return -1;			/* failed to find it, nothing to do */
	
#ifdef DEBUGMRU
	if (bDebug > 1) {
		sprintf(debugstr, "RemoveDocuments kexist %d klast %d\n", kexist, klast);
		OutputDebugString(debugstr);
	}
#endif
/*	if (klast == nMaxDocuments-1) klast--; */
/*	shift `left' all names after the one to be removed list */
/*	for (k = klast; k >= 0; k--) */
/*	for (k = kexist; k <= klast; k++) */
	for (k = kexist; k < klast; k++) {		/* an experiment 96/Sep/29 */
		keyname[4] = (char) ('2' + k);
		n = GetPrivateProfileString(achDocs, keyname, "",
				buffer, sizeof(buffer), achFile); 
		if (n == 0) continue;			/* nothing there */
/*		if (n == 0) break; */
		keyname[4] = (char) ('1' + k);
		WritePrivateProfileString(achDocs, keyname, buffer, achFile);
	}
	keyname[4] = (char) ('1' + klast);	/* fix 96/Sep/29 */
	WritePrivateProfileString(achDocs, keyname, NULL, achFile);	
	return 0;							/* success, removed one */
}

int AddDocument (char *path, char *file) {				/* 1995/Sep/17 */
	int k, n, klast=0;
	char *s;
	char keyname[]="File1";
	char buffer[MAXFILENAME];
	char name[MAXFILENAME];

	if (strchr(file, ':') == NULL &&
		strchr(file, '\\') == NULL &&
		strchr(file, '/') == NULL) {
		strcpy(name, path);
		s = name + strlen(name) - 1;
		if (*s != '\\' && *s != '/') strcat(name, "\\");
		strcat(name, file);
	}
	else strcpy(name, file);

	CharLower (name);							/* 96/Sep/29 ??? */

/*	AnsiLower (name) */

#ifdef DEBUGMRU
	if (bDebug > 1) {
		sprintf(buffer, "AddDocument path %s file %s name %s\n", path, file, name);
		OutputDebugString(buffer);
	}
#endif

/*	first check whether already on list */ /* won't happen if bKeepSorted */
	for (k = 0; k < nMaxDocuments; k++) {
		keyname[4] = (char) ('1' + k);
		n = GetPrivateProfileString(achDocs, keyname, "",
				buffer, sizeof(buffer), achFile); 
		if (n == 0) continue;			/* or break ? */
		if (_stricmp(buffer, name) == 0) return -1;		/* match found */
		klast = k;						/* last number actually used */
	}

#ifdef DEBUGMRU
	if (bDebug > 1) {
		sprintf(buffer, "AddDocument klast %d\n", klast);
		OutputDebugString(buffer);
	}
#endif

	if (klast == nMaxDocuments-1) klast--;		/* prevent overflow */
/*	shift `right' all names on list - drop last one */
/*	for (k = nMaxDocuments-2; k >= 0; k--) */
	for (k = klast; k >= 0; k--) {
		keyname[4] = (char) ('1' + k);
		n = GetPrivateProfileString(achDocs, keyname, "",
				buffer, sizeof(buffer), achFile); 
		if (n == 0) continue; 
/*		if (n == 0) break; */
		keyname[4] = (char) ('2' + k);
/*		wrapinquotes(buffer); */		/* 1997/July/24 */
		quotestring(buffer);			/* 1998/Jan/10 */
		WritePrivateProfileString(achDocs,  keyname, buffer, achFile);
	}
/*	Now insert new one at the top */
	keyname[4] = '1';
/*	wrapinquotes(name); */		/* 1997/July/24 */
	quotestring(name);			/* 1998/Jan/10 */
	WritePrivateProfileString(achDocs, keyname, name, achFile);
	return 0;
}

void filldocmenu (HWND hWnd) {
	HMENU hMenu, hSubMenu;
	int k, n;
	char keyname[]="File1";
	char buffer[MAXFILENAME];	

	if (! bRememberDocs) return;

	hMenu = GetMenu(hWnd);
	hSubMenu = GetSubMenu(hMenu, 0);	/* first sub menu */
/*	add a separator at the end */
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
	for (k = 0; k < nMaxDocuments; k++) {
		keyname[4] = (char) ('1' + k);
		buffer[0] = '&';
		buffer[1] = (char) ('1' + k);
		buffer[2] = ' ';
		n = GetPrivateProfileString(achDocs, keyname, "",
				buffer+3, sizeof(buffer)-3, achFile);
/*		if (bDebug > 1) {
			char buf[128];
			sprintf(buf, "%d\t%d\t%s\n", k, n, buffer);
			OutputDebugString(buf);
		} */
/*		if (n == 0) break; */
		if (n == 0) continue;
		AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
			IDM_DOCUMENTS + k, buffer);
	}
/*	add Separator, About menu item and Exit menu item at bottom */
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
	if (!bHelpAtEnd) AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
			   IDM_ABOUT, "&About DVIWindo®...");
	AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
			   IDM_EXIT, "E&xit\tEsc");
}

void cleardocmenu (HWND hWnd) {		/* clear the Doc menu */
	HMENU hMenu, hSubMenu;
	int i, n;
	int id;
	int count=0;

	if (! bRememberDocs) return;

	hMenu = GetMenu(hWnd);
	hSubMenu = GetSubMenu(hMenu, 0);	/* first sub menu "File" */
	n = GetMenuItemCount(hSubMenu);
/*	remove everything back to (second?) separator from end */
	for (i = n-1; i > 0; i--)	{
		id = GetMenuItemID(hSubMenu, i);
		if (id == -1) break;		/* sanity check ! */
		if (id == IDM_BOTTOM) break;	/* stop if hit Pa&ge Top */
		if (id == IDM_TOP) break;		/* stop if hit Page &Bottom */
		DeleteMenu(hSubMenu, i, MF_BYPOSITION);
/*		is it a separator ? stop if second one seen - safety feature */
/*		this assumes we have two separators in dviwindo.rc before "Exit" */
		if (id == 0) if (count++ > 0) break;
/*		GetMenuString(hSubMenu, i, buffer, sizeof(buffer), MF_BYPOSITION); */
/*		Check for &About DVIWindo (R) */
	}
}

int checkdocmenu (HWND hWnd) {	/* see whether menu agrees with ini file */
	int k, nmenu;
	int nk, nm;
/*	int napp=0; */
	HMENU hMenu, hSubMenu;
	char menustring[MAXFILENAME];
	char keyname[]="File1";
	char buffer[MAXFILENAME];

	if (! bRememberDocs) return 0;
/*	k = GetPrivateProfileString(achAp, NULL, "", str, sizeof(str), achFile);*/

	hMenu = GetMenu(hWnd);
	hSubMenu = GetSubMenu(hMenu, 0);	/* first sub menu */
	nmenu = GetMenuItemCount(hSubMenu);

	for (k = 0; k < nMaxDocuments; k++) {
		keyname[4] = (char) ('1' + k);
		nk = GetPrivateProfileString(achDocs, keyname, "",
				buffer, sizeof(buffer), achFile);
		nm = GetMenuString(hSubMenu, k, menustring, sizeof(menustring),
					  MF_BYPOSITION);
		if ((nk == 0 && nm != 0) ||
			(nk != 0 && nm == 0)) return 1;		/* mismatch */
		if (strcmp(buffer, menustring+3) != 0)	/* step over "&1 " */
			return 1;							/* mismatch */
		if (nk == 0 || nm == 0) return 0;		/* matches */
	}
	return 0; /* ? */
}

/*	calldocument called from dviwindo.c if document selected from MRU list */
/*	Control key => remove from MRU list, do not open document ? */
/*	Shift key => do not re-insert at head of list (not sorted on time) ? */
/*	default changed 1996/June/5 to keep MRU list sorted on time */

int calldocument (int fileID, int bShiftFlag, int bControlFlag) {
	char keyname[]="File1";
	char buffer[MAXFILENAME];
	int nflag;
/*	int klast=0; */
#ifdef IGNORED
	int n, k;
#endif

	keyname[4] = (char) ('1' + fileID); 
	nflag = GetPrivateProfileString(achDocs, keyname, "",
			buffer, sizeof(buffer), achFile);
	if (nflag == 0) return 0;				/* not found, impossible ? */
	strcpy(OpenName, buffer);
	ParseFileName();
	strcpy(DefSpec, "*");		/* ??? */
	strcat(DefSpec, DefExt);	/* fix up DefSpec */

/*	Most of this may be redundant since RemoveDocument/AddDocument */
/*  is called in dviwindo.c at goto idmopen anyway 96/June/5 ... */

	return -1;
}	/* bControlFlag, bShiftFLag unreferenced */

/* COnstruct help menu as left item on main menu */

void fillhelpmenu (HWND hWnd) {
	HMENU hMenu; 
	HMENU hSubMenu;
	int m;

	if (!bHelpAtEnd) return;
	if (hHelpMenu == NULL) return;
	hMenu = GetMenu(hWnd);
	m = GetMenuItemCount(hMenu);
#ifdef IGNORED
	hSubMenu = GetSubMenu(hMenu, m-1);	/* last sub menu */
#endif
	hSubMenu = hHelpMenu;
	AppendMenu(hSubMenu, MF_ENABLED | MF_STRING, IDM_HELP, "&Help\tF1");
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
/*	Stick System Info in `Help' Menu */
	AppendMenu(hSubMenu, MF_ENABLED | MF_STRING, IDM_SYSTEMINFO, "&System Info");
	AppendMenu(hSubMenu, MF_ENABLED | MF_STRING, IDM_SYSFLAGS, "System &Flags");
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
/*	AppendMenu(hSubMenu, MF_ENABLED | MF_STRING, IDM_MAGNIFICATION, "Magnification"); */
/*	AppendMenu(hSubMenu, MF_ENABLED | MF_STRING, IDM_SHOWCOUNTER, "Counter[0]-[9]"); */
/*	AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL); */
	AppendMenu(hSubMenu, MF_ENABLED | MF_STRING,
			   IDM_ABOUT, "&About DVIWindo®...");
/*	hSubMenu = GetSubMenu(hMenu, m-3); */	/* Font menu (assuming TeX menu) */
/*	Remove System Info from `Font' menu */
/*	DeleteMenu(hSubMenu, IDM_SYSTEMINFO, MF_BYCOMMAND); */
/*	DeleteMenu(hSubMenu, IDM_SHOWCOUNTER, MF_BYCOMMAND); */
/*	DeleteMenu(hSubMenu, IDM_SYSFLAGS, MF_BYCOMMAND); */
/*	hSubMenu = GetSubMenu(hMenu, 0); */	/* File menu */
/*	DeleteMenu(hSubMenu, IDM_ABOUT, MF_BYCOMMAND); */	/* redundant */
}

void CallHelp (HWND hWnd) {					/* 1995/Sep/18 */
	char buffer[128];
/*	HMODULE hModule; */						/* 1995/Dec/10 */
	char *s;
	int flag;
	int nCmdShow;
	HINSTANCE err;							/* 95/Mar/31 */
	
/*	Can't use szExeWindo in dviwindo.c for this since that is only the path... */
/*	hModule = GetModuleHandle("dviwindo.exe");
	if (hModule == NULL) {
		if (bDebug)	winerror("Cannot get Module Handle");
		strcpy(buffer, "dviwindo.exe");	
	}
	else GetModuleFileName(hModule, buffer, sizeof(buffer)); */
/*	Or just use hInst instead of hModule ... */
	GetModuleFileName(hInst, buffer, sizeof(buffer));	/* 95/Dec/10 */
	strcat(buffer, " ");
	strcat(buffer, szExeWindo);	
	s = szExeWindo + strlen(szExeWindo) - 1;
	if (*s != '\\' && *s != '/') strcat(buffer, "\\");
	strcat(buffer, "dvi_help.dvi");
	if (bDebug || bShowCalls) {
		err = (HINSTANCE) 0;		/* avoid problems if exit here */
		flag = MaybeShowStr(buffer, "Application");		/* 95/Jan/8 */
		if (flag == 0) return;		/* cancel => pretend success */
	}
	if (nCmdShowForce >= 0) nCmdShow = nCmdShowForce;
//	else nCmdShow = SW_SHOWMAXIMIZED;		/* 1994/Mar/7 */
	else nCmdShow = SW_NORMAL;				/* 2000/Jan/5 */

/*	new debugging info 98/Jun/28 */
//	WritePrivateProfileString(achPr, "LastWinExe", buffer, achFile);
	WritePrivateProfileString(achDiag, "LastWinExe", buffer, achFile);

//	wininfo(buffer);		// debugging only
	err = (HINSTANCE) WinExec(buffer, nCmdShow);
}	/* hWnd unreferenced */

// Using WinExe creates a problem here use CreateProcess instead ???

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef LONGNAMES
/*	Simulate OpenFile - but allow long file names 95/Dec/7 */
/*	If unqualified name, look in current directory first */
/*	Search (i) current dir (ii) GetWindowsDirectory() dir, */
/*	(iii) GetSystemDirectory() dir (iv) GetModuleFileName() dir, */
/*	(v) directories on PATH, (vi) directories mapped in network (?) */

/*  If OF_EXIST flag used, _lclose file again (but strip flag before _lopen) */

HFILE LongOpenFileSub (char *name, int OpenMode) {
	char filename[MAXFILENAME];
	HFILE hFile;
	char *s;
	char *sname;
	
/*	First, just try name as given */
/*	If name is unqualified this means try in current directory */	
/*	If OpenMode == 0 should we force OF_SHARE_DENY_NONE ? */
	hFile = _lopen(name, OpenMode);
	if (hFile != HFILE_ERROR) return hFile;		/* OK, found it */

	sname = name;

/*	if (OpenMode & OF_SEARCH) {
		if ((s = strrchr(sname, '\\')) != NULL) sname = s;
		if ((s = strrchr(sname, '/')) != NULL) sname = s;
		if ((s = strrchr(sname, ':')) != NULL) sname = s;
	} */

/*	if name is fully qualified than that is it ... go no further */
	if (strchr(sname, '\\') != NULL ||
		strchr(sname, '/') != NULL ||
		strchr(sname, ':') != NULL) return hFile;

/*	Or use szWinDir in dviwindo.c ... */
	if (GetWindowsDirectory(filename, sizeof(filename)) > 0) {
		s = filename + strlen(filename) - 1;
		if (*s != '\\') strcat(s, "\\");
		strcat(s, sname);
		hFile = _lopen(filename, OpenMode);
		if (hFile != HFILE_ERROR) return hFile;		/* OK, found it */
	}

	if (GetSystemDirectory(filename, sizeof(filename)) > 0) {
		s = filename + strlen(filename) - 1;
		if (*s != '\\') strcat(s, "\\");
		strcat(s, sname);
		hFile = _lopen(filename, OpenMode);
		if (hFile != HFILE_ERROR) return hFile;		/* OK, found it */
	}

/*	Or use szExeWindo in dviwindo.c ... */
/*	hModule = GetModuleHandle("dviwindo.exe"); ? use hModule instead hInst ? */
	if (GetModuleFileName(hInst, filename, sizeof(filename)) > 0) {
/*		strip off name of executable */
		if ((s = strrchr(filename, '\\')) != NULL) {
			strcpy(s+1, sname);
			hFile = _lopen(filename, OpenMode);
			if (hFile != HFILE_ERROR) return hFile;		/* OK, found it */
		}
	}

/*	search along DOS PATH */
	_searchenv(sname, "PATH", filename);
	if (*filename == '\0') return HFILE_ERROR;
	hFile = _lopen(filename, OpenMode);
	return hFile;	
}

HFILE LongOpenFile(char *name, int OpenMode) {
	HFILE hFile;
/*	sprintf(debugstr, "Trying to open `%s' using flags %d\n");
	winerror(debugstr); */
	if (OpenMode & OF_EXIST) {
		OpenMode = OpenMode & ~OF_EXIST;			/* remove this flag bit */
		OpenMode = OpenMode | OF_SHARE_DENY_NONE;	/* 1996/May/18 experim */
		hFile = LongOpenFileSub(name, OpenMode);
		if (hFile != HFILE_ERROR) _lclose(hFile);
#ifdef DEBUGOPENFILE
		if (bDebug > 1) {
			sprintf(debgustr, "Does `%s' exist? hFile %d\n", name, hFile);
			OutputDebugString(debugstr);
		}
#endif
		return hFile;	/* won't be valid handle, but not HFILE_ERROR */
	}
	else return LongOpenFileSub(name, OpenMode);
}

#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Expand ..\ and .\ notation in file names, allow for nested occurence */
/* First prepends current directory, then removes .\ and foo\..\ patterns */
/* Also handles case where ..\ and .\ have already been appended to path */
/* Also handles \foo notation --- meaining top level in that drive */

int expanddots (char *filename) {
	char *t, *u;
	char *s=filename;
	int n;
	char DefPath[MAXFILENAME]="";			/* use global version ? */

	if (*s == '.') {						/* get current directory first */

		GetCurrentDirectory(sizeof(DefPath), DefPath);

/*		printf("Current Directory is %s\n", DefPath); */
		t = DefPath + strlen(DefPath) - 1;
		if (*t != '\\' && *t != '/') strcat(DefPath, "\\");
		n = strlen(DefPath);
		t = s + n;						/* where string will get moved to */
		memmove (s+n, s, strlen(s)+1);	/* make space for current directory */
		strncpy (s, DefPath, n);		/* splice it in */
	}
	else if (*s == '\\' || *s == '/') {	

		GetCurrentDirectory(sizeof(DefPath), DefPath);

		if ((t = strchr(DefPath, ':')) != NULL) {
			n = t - DefPath + 1;
			memmove (s+n, s, strlen(s)+1);
			strncpy (s, DefPath, n);	/* splice in "c:" or whatever */
			return 0;
		}		
	}
	else {								/* the .\ or ..\ is inside already */
		t = s;
		for (;;) {
			while (*t != '\0' && *t != '\\' && *t != '/') t++;
			if (*t == '\0') return -1;	/* end of string, nothing to do */
			t++;
			if (*t == '.') break;		/* hit \. or /. */
		}
	}
	while (*t == '.') {					/* loop until all . and .. removed */
		if (*(t+1) != '.')	{			/* single dot case */
			if (*(t+1) != '\\' && *(t+1) != '/') {
/*				strcpy(s, t); */		/* restore (partial) bad string */
				strcpy(filename, t);	/* restore (partial) bad string */
				return -1;				/* garbage format */
			}
			strcpy(t, t+2);			/* flush ".\" */
		}
		else {							/* double dot */
			if (*(t+2) != '\\' && *(t+2) != '/') {
/*				strcpy(s, t); */		/* restore (partial) bad string */
				strcpy(filename, t);	/* restore (partial) bad string */
				return -1;				/* garbage format */
			}
			u = t-2;					/* assuming ..\ form ... */
			if (u < s) u = s;			/* sanity check avoid error */
/* search back for separator or colon or start of string ... */
			while (u > s && *u != '\\' && *u != '/' && *u != ':') u--;
			if (*u == ':') u++;			/* leave in separator after : */
			strcpy(u+1, t+3);			/* flush "..\" */
			t = u+1;
		}
/*		printf("After removal of dots we have %s\n", s); */
	}
	if (bUnixify) deslash(filename); 
	return 0;
}

/* allow for differences in captialization and use of / versus \ */
/* returns zero if the same file name */

int comparenames (char *a, char *b) {
	int ca, cb;
	ca = *a++;
	cb = *b++;
	while (ca != '\0' && cb != '\0') {
		if ((ca != cb) &&
			((ca != '\\' || cb != '/')) &&
			((ca != '/' || cb != '\\')) &&
			((ca < 'a' || ca > 'z' || cb != ca - 32)) &&
			((ca < 'A' || ca > 'Z' || cb != ca + 32))) break;
		ca = *a++;
		cb = *b++;
	}
	if (ca > cb) return	+1;			/* a > b */
	else if (ca < cb) return -1;	/* a < b */	
	return 0;						/* a == b */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* New code to play MetaFile nested in MetaFile 96/Aug/10 */
/* `Nested Scalable Metafiles' PSS ID: Q107171 1995-01-05 */

int PlayInMetafile(HDC hdcMeta, HMETAFILE hmf,
/*	logical rectangle where metafile will be played on destination MF */
	   int nLeftRect, int nTopRect, int nRightRect, int nBottomRect,
/*	logical extent of the destination MF that we are playing into */
	   int xLogExt, int yLogExt,
/*	logical extent of metafile being played - only used if bTrustPlaceable */
	   int xHmfLogExt, int yHmfLogExt) {
	int ret=0;
	int xOffset, yOffset;
	int xDestExt, yDestExt; 

#ifdef DEBUGMAPPING
	if (bDebug > 1) {
		sprintf(debugstr,	"Placed %d %d %d %d\n", 
				nLeftRect, nTopRect, nRightRect, nBottomRect);
		OutputDebugString(debugstr);
		sprintf(debugstr, "Dest Ext %d %d - Play Ext %d %d\n",
				xLogExt, yLogExt, xHmfLogExt, yHmfLogExt);
		OutputDebugString(debugstr);
	}
#endif
/*	SaveDC(hdcMeta); */			/* redundant ? already in SavedDC */
	if (bTrustPlaceable == 0) {
/*		Save for use in SowPlayMetaFile for OffsetWindowOrg */
		nSavedRightRect = nRightRect;
		nSavedLeftRect = nLeftRect;
		nSavedTopRect = nTopRect;
		nSavedBottomRect = nBottomRect;
	}
	xDestExt = nRightRect-nLeftRect;
	yDestExt = nBottomRect-nTopRect;
	if (bTrustPlaceable) {
		xOffset = MulDiv(nLeftRect, xHmfLogExt, xDestExt);
		yOffset = MulDiv(nTopRect, yHmfLogExt, yDestExt); 
#ifdef DEBUGMAPPING
		if (bDebug > 1) {
			sprintf(debugstr, "OffsetWindowOrg %d %d\n", -xOffset, yOffset);
			OutputDebugString(debugstr);
		}
#endif
/*	Following seems to have no effect (because SetWindoOrigin used in MF) */
/*    ret = OffsetWindowOrgEx(hdcMeta, -xOffset, -yOffset, NULL); */
/*	if (bTrustPlaceable) */
		(void) OffsetWindowOrgEx(hdcMeta, -xOffset, yOffset, NULL); 
/*		(void) OffsetWindowOrgEx(hdcMeta, xOffset, yOffset, NULL); */
	}
	else bAdjustOrigin = 1;	/* make note to adjust when MT_SETWINDOWEXT */
#ifdef DEBUGMAPPING
	if (bDebug > 1) {
		sprintf(debugstr, "ScaleViewPort %d %d %d %d\n",
				xDestExt, xLogExt, yDestExt, yLogExt);
		OutputDebugString(debugstr);
	}
#endif
    ret = ScaleViewportExtEx(hdcMeta, xDestExt, xLogExt, yDestExt, yLogExt, NULL);
/*	if (ret == 0) {
		if (bDebug > 1) OutputDebugString("ScaleViewport failed\n");
	} */

/*	ret = PlayMetaFile(hdcMeta, hmf); */
    ret = SlowPlayMetaFile(hdcMeta, hmf);
/*	RestoreDC(hdcMeta, -1); */			/* redundant ? already in SavedDC */
	return ret;
}

#ifdef DEBUGMAPPING
void ShowMapping(HDC hDC) {
	SIZE Size;				/* 1995/Oct/31 */
	POINT Point;			/* 1996/Aug/9 */
	if (bCopyFlag != 0) return;		/* bogus data in that case */
	if (bDebug > 1) {
		GetViewportExtEx(hDC, &Size);
/*		sprintf(debugstr, "ViewPortExt %ld %ld\n", Size.cx, Size.cy); */
		sprintf(debugstr, "ViewPortExt %d %d\n", Size.cx, Size.cy);
		OutputDebugString(debugstr);
		GetViewportOrgEx(hDC, &Point);
		sprintf(debugstr, "ViewPortOrg %d %d\n", Point.x, Point.y);
		OutputDebugString(debugstr);

		GetWindowExtEx(hDC, &Size);
/*		sprintf(debugstr, "WindowExt %ld %ld\n", Size.cx, Size.cy); */
		sprintf(debugstr, "WindowExt %d %d\n", Size.cx, Size.cy);
		OutputDebugString(debugstr);
		GetWindowOrgEx(hDC, &Point);
		sprintf(debugstr, "WindowOrg %d %d\n", Point.x, Point.y);
		OutputDebugString(debugstr);

	}
}
#endif

#ifdef DEBUGMETAFILE
void ShowMetaFileRecord(METARECORD FAR *, HFILE);
int nMetaRecord;				/* running count of METAFILERECORDS */
#endif

#ifdef DEBUGMETAOBJECT
char bObject[256];		/* flags on object allocated */
#endif

/* stuff to play back METAFILE and changed some operations */
/* used both by SlowPlayMetaFile and DumpMetaFile */
/* used only when playing MetaFile into MetaFile DC */
/* or when debugging MetaFile by writing its records to a log */

/* typedef int (CALLBACK* MFENUMPROC)(HDC, HANDLETABLE FAR*, METARECORD FAR*, int, LPARAM); */

int CALLBACK _export EnumMetaFileProc (HDC hDC, HANDLETABLE FAR *lpht,
					  METARECORD FAR *lpmr, int cObj, LPARAM lParam) {
/*	int n, op; */
	DWORD n;
	WORD op;
	HFILE output=HFILE_ERROR;
	int xExt, yExt;
#ifdef IGNORED
	int xll, yll, xur, yur;
#endif

/*	First, a sanity check on packing in wingdi.h */
/*	if (sizeof(METARECORD) != 8) winerror("METARECORD wrong size"); */
	output = (HFILE) lParam; 
/*	if lParam == HFILE_ERROR, we play the file */
	if (output == HFILE_ERROR) {
		n = lpmr->rdSize;				/* size of MF record */
		op = (WORD) lpmr->rdFunction;	/* operation of MF record */

/*	and also dump out records of special significance, if in debug mode */
#ifdef DEBUGMETAFILE
		if (op == META_SETWINDOWORG ||
			op == META_OFFSETWINDOWORG ||
			op == META_SETWINDOWEXT ||
			op == META_SCALEWINDOWEXT ||
			op == META_SETVIEWPORTORG ||
			op == META_SETVIEWPORTEXT ||
			op == META_OFFSETVIEWPORTORG ||
			op == META_SCALEVIEWPORTEXT ||
			op == META_SAVEDC ||
			op == META_RESTOREDC ||
			op == META_SETMAPMODE ||
			op == META_SETRELABS ||
/*			op == META_SETBKMODE || */
			op == META_SETTEXTALIGN) {
				if (bDebug > 1)
					ShowMetaFileRecord(lpmr, output);
		}
		nMetaRecord++;
#ifdef DEBUGMETAOBJECT
		if (op == META_SELECTOBJECT) {
			int k = lpmr->rdParm[0];
			bObject[k] = 1;
		}
		if (op == META_DELETEOBJECT) {
			int k = lpmr->rdParm[0];
			if (bObject[k] == 0) {
				if (bDebug > 1) {
					sprintf(debugstr,
							"Deleting deleted object %d record %d\n",
							k, nMetaRecord);
					OutputDebugString(debugstr);
				}
			}
			bObject[k] = 0;
		}
#endif
#endif
/*		Turn absolute Window Origin Setting into Relative 1996/Aug/4 */
/*		Problems if these commands occur more than once ... */
		if (op == META_SETWINDOWORG) {
/*			RestoreDC(hDC, -1);	*/
/*			SaveDC(hDC); */
			lpmr->rdFunction = META_OFFSETWINDOWORG; 
			op = (WORD) lpmr->rdFunction;
		}
		else if (op == META_SETWINDOWEXT) {
/*			Problems if these commands occur more than once ... */
/*			Compare with what we were told earlier in placeable WMF header */			
/*			Note: often they are *not* the same ... */
			xExt = (short) lpmr->rdParm[1];
			yExt = (short) lpmr->rdParm[0];
#ifdef IGNORED
/*			debugging output if mismatch in WindowExt */
			xll = Savedbbox.left;
			yur = Savedbbox.top;
			xur = Savedbbox.right;
			yll = Savedbbox.bottom;
			if (xExt != (xur - xll) || yExt != (yll - yur)) {
				if (bDebug > 1) {
					sprintf(debugstr,
			"ERROR: xExt %d yExt %d xur - xll %d yll - yur %d\n",
							xExt, yExt, xur - xll, yll - yur);
					OutputDebugString(debugstr);
				}
			}
#endif
/*		Make adjustment in origin only now, where we know actual extent ! */
/*			if (bTrustPlaceable == 0 && bAdjustOrigin) */
			if (bTrustPlaceable == 0) {
				int xDestExt, yDestExt, xOffset, yOffset;
				if (bAdjustOrigin == 0) {	/* need to restore state first */
					RestoreDC(hDC, -1);
					SaveDC(hDC);
				}
				else bAdjustOrigin = 0;		/* unless it is the first time */
				xDestExt = nSavedRightRect-nSavedLeftRect;
				yDestExt = nSavedBottomRect-nSavedTopRect;
				xOffset = MulDiv(nSavedLeftRect, xExt, xDestExt);
				yOffset = MulDiv(nSavedTopRect, yExt, yDestExt); 
#ifdef DEBUGMAPPING
				if (bDebug > 1) {
					sprintf(debugstr, "OffsetWindowOrg %d %d\n", -xOffset, -yOffset);
					OutputDebugString(debugstr);
				}
#endif
				(void) OffsetWindowOrgEx(hDC, -xOffset, -yOffset, NULL);
				bAdjustOrigin = 0;			/* has been adjusted now */
			}
		}
/*		Problems if SetWindowOrg (hDC, -1, -1); ??? */
/*		Problems if SetWindowExt does not match play ext ??? */
/*		Avoid absolute Window Origin Setting ??? */
/*		Avoid setting of map mode ??? */
		if (op == META_SETWINDOWORG) ;		/* taken care of above */
/*		else if (op == META_SETMAPMODE) ; */	/* better be ANISOTROPIC ! */
/*		else if (op == META_SETRELABS) ; */		/* what is this ? */
/*		else if (op == META_CREATEREGION) ; */		/* ??? */
/*		else if (op == META_CREATEPALETTE) ; */		/* ??? */
/*		else if (op == META_SELECTPALETTE) ; */		/* ??? */
/*		else if (op == META_REALIZEPALETTE) ; */	/* ??? */
		else PlayMetaFileRecord(hDC, lpht, lpmr, cObj);
/*		return TRUE; */
	}
#ifdef DEBUGMETAFILE
	else if (bDebug > 1)	/* if lParam != HFILE_ERROR, we write a log */
		ShowMetaFileRecord(lpmr, output);
#endif
	return TRUE;		/* to continue enumerating MF */
/*	return FALSE; */	/* to stop */
}

int SlowPlayMetaFile (HDC hDC, HMETAFILE hMF) {
	int ret;
    SaveDC(hDC);					/* needed for repeated SetWindowExt */
#ifdef DEBUGMETAFILE
	if (bDebug > 1) OutputDebugString("Slow Start\n");
	nMetaRecord = 0;
#ifdef DEBUGMETAOBJECT
	if (bDebug > 1) {
		int k; 
		for (k=0; k < 256; k++) bObject[k] = 0;
	}
#endif
#endif
	ret = EnumMetaFile (hDC, hMF, (MFENUMPROC) EnumMetaFileProc,
				 (LPARAM) HFILE_ERROR);
#ifdef DEBUGMETAFILE
	if (bDebug > 1) {
/*		int flag = 0; */
/*		non-zero means it enumerated all records */
		sprintf(debugstr, "EnumMetaFile returns %d\n", ret);
		OutputDebugString(debugstr);
#ifdef DEBUGMETAOBJECT
		{
			int k;
/*		Show Objects not deleted at end */
			for (k=0; k < 256; k++)
				if (bObject[k]) {
					sprintf(debugstr, "%d ", k);
					OutputDebugString(debugstr);
					flag++;
				}
			if (flag) OutputDebugString(" (undeleted objects)\n");
		}
#endif
		sprintf(debugstr, "Slow End %d METAFILERECORDS\n", nMetaRecord);
		OutputDebugString(debugstr);
	}
/*	if (bDebug > 1) OutputDebugString("Slow End\n"); */
#endif
    RestoreDC(hDC, -1);
	return 1;
}

/************************************************************************/

/* stuff to dump out metafile records to disk log file */

#ifdef DEBUGMETAFILE

char *showmetastring (char *s, METARECORD FAR *lpmr, int ks, int ke) {
int k;
WORD nchrs;
	s = s + strlen(s);
	*s++ = 96;					/* quoteleft */
	for (k = ks; k < ke; k++) {
		nchrs = (WORD) lpmr->rdParm[k];
		if ((nchrs & 255) == 0) break;
		*s++ = (char) (nchrs & 255);
/*		else *s++ = '@'; */
		nchrs = (WORD) (nchrs >> 8);
		if ((nchrs & 255) == 0) break;
		*s++ = (char) (nchrs & 255);
/*		else *s++ = '@'; */
	}
	*s++ = 39;					/* quoteright */
	*s++ = ' ';
	*s = '\0';
	return s;
}

char *GDIfromMETA (int);

void ShowMetaFileRecord(METARECORD FAR *lpmr, HFILE output) {
/*	int k, n, op, nargs, flag; */
	DWORD n;
	WORD op;
	int nargs;
	int k, flag;
/*	WORD nchrs; */
	char *s;
	n = lpmr->rdSize;
	op = (WORD) lpmr->rdFunction;
/*	nargs = n - (sizeof(DWORD) + sizeof(UINT)) / sizeof(UINT); */
/*	nargs = n - (sizeof(METARECORD) / sizeof(UINT)); */
/*	nargs = (int) n - (sizeof(METARECORD) / sizeof(WORD)); */
	nargs = (int) n - (sizeof(METARECORD) / sizeof(WORD)) + 1;
	s = GDIfromMETA (op);
	sprintf(str, "%04X %s (%d) ", op, s, nargs);
	s = str;
	if (op == 0x0a32) {				/* treat ExtTextOut special */
		s = s + strlen(s);
		s = showmetastring(s, lpmr, 4, nargs);
		for (k = 3; k >= 0; k--) {
			s = s + strlen(s);
			sprintf(s, "%d ", (short) lpmr->rdParm[k]);
		}
	}
	else if (op == 0x0521) {		/* treat TextOut special */
		k = nargs-1;
		s = s + strlen(s);
		sprintf(s, "%d ", (short) lpmr->rdParm[k]);
		k = nargs-2;
		s = s + strlen(s);
		sprintf(s, "%d ", (short) lpmr->rdParm[k]);
		s = showmetastring(s, lpmr, 1, nargs-2);
		k = 0;
		sprintf(s, "%d ", (short) lpmr->rdParm[k]);
	}
	else if (op == 0x02FB) {		/* treat CreateFontIndirect special */
		s = s + strlen(s);
		k = nargs - 1;
		sprintf(s, "%d ", (short) lpmr->rdParm[k]);
		s = showmetastring(s, lpmr, 9, nargs-1);
		for (k = 8; k >= 0; k--) {
			s = s + strlen(s);
			sprintf(s, "%d ", (short) lpmr->rdParm[k]);
		}
	}
/*	Note, args are in *reverse* order */
	else if (nargs < 20) {
/*		for (k = 0; k , nargs; k++) */
		for (k = nargs-1; k >= 0; k--) {
			s = s + strlen(s);
			sprintf(s, "%d ", (short) lpmr->rdParm[k]);
		}
	}
	strcat(s, "\n");
	if (output == HFILE_ERROR) {
		if (bDebug > 1) OutputDebugString(str);
	}
	else flag = _lwrite(output, str, strlen(str));
/*	flag == HFILE_ERROR implies trouble */
}

int metadebugdone=0;								/* 1996/Aug/5 */

void DumpMetaFile(HDC hDC, HMETAFILE hMF) {		/* 1996/Aug/5 */
	char filename[MAXFILENAME];
	HFILE output=HFILE_ERROR;
	
	if (metadebugdone++ > 0) return;				/* do only once */

/*	if we set up and open output file, the log will be written there */
/*	otherwise it appears in OutputDebugString(...) */
	strcpy(filename, "e:\\metafile.log");
	output = _lcreat(filename, 0);
	if (output == HFILE_ERROR) {
		strcpy(filename, "c:\\users\\default\\metafile.log");
		output = _lcreat(filename, 0);
		if (output == HFILE_ERROR) {
			winerror ("Can't open LOG");
			return;
		}
	}
/*	sprintf(str, "LOG %s started", filename);	winerror(str); */
	
/*	EnumMetaFile (hDC, hMF, (MFENUMPROC) EnumMetaFileProc, (LPARAM) (long) NULL); */
	EnumMetaFile (hDC, hMF, (MFENUMPROC) EnumMetaFileProc, (LPARAM) output);
	if (output != HFILE_ERROR) _lclose(output);
/*	sprintf(str, "LOG %s completed", filename); winerror(str); */
}
#endif

/* Following only for decoding Windows MetaFiles */ 

#ifdef DEBUGMETAFILE
/* Metafile Functions */
char *GDIfromMETA (int code) {
	switch(code) {
	case 0x001E:	return "SAVEDC";
	case 0x0035:	return "REALIZEPALETTE";
	case 0x0037:	return "SETPALENTRIES";
	case 0x00f7:	return "CREATEPALETTE";
	case 0x0102:	return "SETBKMODE";
	case 0x0103:	return "SETMAPMODE";
	case 0x0104:	return "SETROP2";
	case 0x0105:	return "SETRELABS";
	case 0x0106:	return "SETPOLYFILLMODE";
	case 0x0107:	return "SETSTRETCHBLTMODE";
	case 0x0108:	return "SETTEXTCHAREXTRA";
	case 0x0127:	return "RESTOREDC";
	case 0x012A:	return "INVERTREGION";
	case 0x012B:	return "PAINTREGION";
	case 0x012C:	return "SELECTCLIPREGION";
	case 0x012D:	return "SELECTOBJECT";
	case 0x012E:	return "SETTEXTALIGN";
	case 0x0139:	return "RESIZEPALETTE";
	case 0x0142:	return "DIBCREATEPATTERNBRUSH";
	case 0x01F9:	return "CREATEPATTERNBRUSH";
	case 0x01f0:	return "DELETEOBJECT";
	case 0x0201:	return "SETBKCOLOR";
	case 0x0209:	return "SETTEXTCOLOR";
	case 0x020A:	return "SETTEXTJUSTIFICATION";
	case 0x020B:	return "SETWINDOWORG";
	case 0x020C:	return "SETWINDOWEXT";
	case 0x020D:	return "SETVIEWPORTORG";
	case 0x020E:	return "SETVIEWPORTEXT";
	case 0x020F:	return "OFFSETWINDOWORG";
	case 0x0211:	return "OFFSETVIEWPORTORG";
	case 0x0213:	return "LINETO";
	case 0x0214:	return "MOVETO";
	case 0x0220:	return "OFFSETCLIPRGN";
	case 0x0228:	return "FILLREGION";
	case 0x0231:	return "SETMAPPERFLAGS";
	case 0x0234:	return "SELECTPALETTE";
	case 0x02FA:	return "CREATEPENINDIRECT";
	case 0x02FB:	return "CREATEFONTINDIRECT";
	case 0x02FC:	return "CREATEBRUSHINDIRECT";
	case 0x0324:	return "POLYGON";
	case 0x0325:	return "POLYLINE";
	case 0x0410:	return "SCALEWINDOWEXT";
	case 0x0412:	return "SCALEVIEWPORTEXT";
	case 0x0415:	return "EXCLUDECLIPRECT";
	case 0x0416:	return "INTERSECTCLIPRECT";
	case 0x0418:	return "ELLIPSE";
	case 0x0419:	return "FLOODFILL";
	case 0x041B:	return "RECTANGLE";
	case 0x041F:	return "SETPIXEL";
	case 0x0429:	return "FRAMEREGION";
	case 0x0436:	return "ANIMATEPALETTE";
	case 0x0521:	return "TEXTOUT";
	case 0x0538:	return "POLYPOLYGON";
	case 0x0548:	return "EXTFLOODFILL";
	case 0x061C:	return "ROUNDRECT";
	case 0x061D:	return "PATBLT";
	case 0x0626:	return "ESCAPE";
	case 0x06FF:	return "CREATEREGION";
	case 0x0817:	return "ARC";
	case 0x081A:	return "PIE";
	case 0x0830:	return "CHORD";
	case 0x0922:	return "BITBLT";
	case 0x0940:	return "DIBBITBLT";
	case 0x0B23:	return "STRETCHBLT";
	case 0x0a32:	return "EXTTEXTOUT";
	case 0x0b41:	return "DIBSTRETCHBLT";
	case 0x0d33:	return "SETDIBTODEV";
	case 0x0f43:	return "STRETCHDIB";
	default: return "UNKNOWN";
	}
}
#endif

/*******************************************************************************/

/* Note that old style dialog box is used unless OFN_EXPLORER flag set ... */

#ifdef USEUNICODE

#ifdef MAKEUNICODE
WCHAR encodeUID[256];		/* will set up when loading */
#else

/* hard wired table for texnansi for now, will be set up later */
/* when loading by ReadEncoding in winsearc.c */

WCHAR encodeUID[256]= {
0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2044, 0x02D9, 0x02DD, 0x02DB,
0xFB02, 0x2044, 0xFFFF, 0xFB00, 0xFB01, 0xFB02, 0xFB03, 0xFB04,
0x0131, 0xFFFF, 0x0060, 0x00B4, 0x02C7, 0x02D8, 0x00AF, 0x02DA,
0x00B8, 0x00DF, 0x00E6, 0x0153, 0x00F8, 0x00C6, 0x0152, 0x00D8,
0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x2019,
0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x02C6, 0x005F,
0x2018, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x02DC, 0x00A8,
0x0141, 0x0027, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x017D, 0x005E, 0x2212,
0x0142, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x017E, 0x007E, 0x0178,
0x0020, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x002D, 0x00AE, 0x00AF,
0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF };

#endif /* end of if not MAKEUNICODE */

/* Following is the remapping on 128 - 159 in NT for almost all code pages */
/* 0xFFFF marks 8 spots that are not remapped */

/*
0x20AC, 0xFFFF, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFF, 0x017D, 0xFFFF,
0xFFFF, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFF, 0x017E, 0x0178,
*/

/* 128 Euro 0x20AC */ /* 142 Zcaron 0x017D */ /* 158 zcaron 0x017E */

/* Following is the remapping for CP 1251 Cyrillic */

/*
0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021
0xFFFE, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F
0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014
0xFFFE, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F
*/

/* TextOutA(NA) == TextOutW(NW) where NW = NA    for  32 <= NA <= 127 */
/* TextOutA(NA) == TextOutW(NW) where NW = f(NA) for 128 <= NA <= 159 */
/* TextOutA(NA) == TextOutW(NW) where NW = NA    for 160 <= NA <= 255 */
/* where f(NA) is given by this table */

/* following is the code for setting up encodeUID[] */
/* unfortunately it means loading this huge table of glyph names */
/* currently 922 lines */

#ifdef MAKEUNICODE

// typedef struct {
//	char *glyphname;
//	unsigned short UID;
// } UNICODE_MAP;

/* #define OLDUNICODE */	/* use F001 and F002 for fi and fl */

/* #define CORPORATEUSE */	/* extra characters that Adobe uses */

/* SMALLCAPS LARGEACCENTS BLOCKCHARS INFERIOR OLDSTYLE CORPORATEUSE */

#define SUPERIOR

#define LARGEACCENTS

#define MATHEXTENSION		/* we want the extensible glyph pieces */

#define HP					/* glyphs in HP fonts - but not in ATM ! */

/* For glyph names in Type 1 fonts, I would go with Adobe
 * http://partners.adobe.com/asn/developer/typeforum/unicodegn.html
 * since T1 fonts will only work properly in Unicode enabled
 * systems if ATM can match the glyph name.
 * http://partners.adobe.com/asn/developer/typeforum/glyphlist.txt */

/* 922 entries of which 174 are Adobe special hacks (i.e. not UNICODE) */

/* we do not include the Zapf Dingbats glyph names in Adobe coprorate extension */

/* NOTE: this *MUST* be alphabetically ordered for bsearch */

/* static const UNICODE_MAP unicodeMap[] = */
const UNICODE_MAP unicodeMap[] =
{
/* NOTE:  * indicates that this line differs from the unicodeMap from Sairus */
	{ "A",                    0x0041 },
	{ "AE",                   0x00C6 },
	{ "AEacute",			  0x01FC },	/* new 97/Mar */
#ifdef SMALLCAPS
	{ "AEsmall",              0xF7E6 }, /* 0xF8E6 */	/* Adobe special hack */
#endif
	{ "Aacute",               0x00C1 },
#ifdef SMALLCAPS
	{ "Aacutesmall",          0xF7E1 }, /* 0xF8E1 */	/* Adobe special hack */
#endif
	{ "Abreve",               0x0102 },
	{ "Acircumflex",          0x00C2 },
#ifdef SMALLCAPS
	{ "Acircumflexsmall",     0xF7E2 }, /* 0xF8E2 */	/* Adobe special hack */
#endif
#ifdef LARGEACCENTS
	{ "Acute",                0xF6C9 }, /* 0xF7B0 */	/* Adobe special hack */
#endif
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Acutesmall",           0xF7B4 }, /* 0xF8B4 */	/* Adobe special hack */
#endif
	{ "Adieresis",            0x00C4 },
#ifdef SMALLCAPS
	{ "Adieresissmall",       0xF7E4 }, /* 0xF8E4 */	/* Adobe special hack */
#endif
	{ "Agrave",               0x00C0 },
#ifdef SMALLCAPS
	{ "Agravesmall",          0xF7E0 }, /* 0xF8E0 */	/* Adobe special hack */
#endif
	{ "Alpha",                0x0391 },
	{ "Alphatonos",           0x0386 },
	{ "Amacron",              0x0100 },
	{ "Aogonek",              0x0104 },
	{ "Aring",                0x00C5 },
	{ "Aringacute",			  0x01FA },	/* new 97/Mar */ /* Aacutering */
#ifdef SMALLCAPS
	{ "Aringsmall",           0xF7E5 }, /* 0xF8E5 */	/* Adobe special hack */
	{ "Asmall",               0xF761 }, /* 0xF861 */	/* Adobe special hack */
#endif
	{ "Atilde",               0x00C3 },
#ifdef SMALLCAPS
	{ "Atildesmall",          0xF7E3 }, /* 0xF8E3 */	/* Adobe special hack */
#endif
	{ "B",                    0x0042 },
	{ "Beta",                 0x0392 },
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Brevesmall",           0xF6F4 }, /* 0xF7F4 */	/* Adobe special hack */
#endif
#ifdef SMALLCAPS
	{ "Bsmall",               0xF762 }, /* 0xF862 */	/* Adobe special hack */
#endif
	{ "C",                    0x0043 },
	{ "Cacute",               0x0106 },
#ifdef LARGEACCENTS
	{ "Caron",                0xF6CA }, /* 0xF7B1 */	/* Adobe special hack */
#endif
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Caronsmall",           0xF6F5 }, /* 0xF7F5 */	/* Adobe special hack */
#endif
	{ "Ccaron",               0x010C },
	{ "Ccedilla",             0x00C7 },
#ifdef SMALLCAPS
	{ "Ccedillasmall",        0xF7E7 }, /* 0xF8E7 */	/* Adobe special hack */
#endif
	{ "Ccircumflex",          0x0108 },
	{ "Cdotaccent",           0x010A },
#ifdef SMALLCAPS
	{ "Cedillasmall",         0xF7B8 }, /* 0xF8B8 */	/* Adobe special hack */
#endif
#ifdef HP
	{ "Cfraktur",             0x212D }, /* Fraktur Uppercase C NOT Adobe*/
#endif
	{ "Chi",                  0x03A7 },
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Circumflexsmall",      0xF6F6 }, /* 0xF7F6 */	/* Adobe special hack */
#endif
#ifdef SMALLCAPS
	{ "Csmall",               0xF763 }, /* 0xF863 */	/* Adobe special hack */
#endif
	{ "D",                    0x0044 },
	{ "Dcaron",               0x010E },
	{ "Dcroat",               0x0110 },
	{ "Delta",	              0x2206 },		/* Macintosh indicator */ /* increment */
	{ "Delta_",               0x0394 },		/* Greek Delta duplicate * */
#ifdef LARGEACCENTS
	{ "Dieresis",             0xF6CB }, /* 0xF7B2 */	/* Adobe special hack */
	{ "DieresisAcute",        0xF6CC }, /* 0xF7B3 */	/* Adobe special hack */
	{ "DieresisGrave",        0xF6CD }, /* 0xF7B4 */	/* Adobe special hack */
#endif
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Dieresissmall",        0xF7A8 }, /* 0xF8A8 */	/* Adobe special hack */
	{ "Dotaccentsmall",       0xF6F7 }, /* 0xF7F7 */	/* Adobe special hack */
#endif
#ifdef SMALLCAPS
	{ "Dsmall",               0xF764 }, /* 0xF864 */	/* Adobe special hack */
#endif
	{ "E",                    0x0045 },
	{ "Eacute",               0x00C9 },
#ifdef SMALLCAPS
	{ "Eacutesmall",          0xF7E9 }, /* 0xF8E9 */	/* Adobe special hack */
#endif
	{ "Ebreve",				  0x0114 }, /* new 97/Mar */
	{ "Ecaron",               0x011A },
	{ "Ecircumflex",          0x00CA },
#ifdef SMALLCAPS
	{ "Ecircumflexsmall",     0xF7EA }, /* 0xF8EA */	/* Adobe special hack */
#endif
	{ "Edieresis",            0x00CB },
#ifdef SMALLCAPS
	{ "Edieresissmall",       0xF7EB }, /* 0xF8EB */	/* Adobe special hack */
#endif
	{ "Edotaccent",           0x0116 },
	{ "Egrave",               0x00C8 },
#ifdef SMALLCAPS
	{ "Egravesmall",          0xF7E8 }, /* 0xF8E8 */	/* Adobe special hack */
#endif
	{ "Emacron",              0x0112 },
	{ "Eng",                  0x014A },
	{ "Eogonek",              0x0118 },
	{ "Epsilon",              0x0395 },
	{ "Epsilontonos",         0x0388 },
#ifdef SMALLCAPS
	{ "Esmall",               0xF765 }, /* 0xF865 */	/* Adobe special hack */
#endif
	{ "Eta",                  0x0397 },
	{ "Etatonos",             0x0389 },
	{ "Eth",                  0x00D0 },
#ifdef SMALLCAPS
	{ "Ethsmall",             0xF7F0 }, /* 0xF8F0 */	/* Adobe special hack */
#endif
	{ "Euro",                 0x20AC },	 /* new Euro glyph 97/Nov/24 */
	{ "Euro_",                0x20A0 },	 /* old Euro glyphs WRONG 98/Oct/26 */
	{ "F",                    0x0046 },
#ifdef SMALLCAPS
	{ "Fsmall",               0xF766 }, /* 0xF866 */	/* Adobe special hack */
#endif
	{ "G",                    0x0047 },
	{ "Gamma",                0x0393 },
	{ "Gbreve",               0x011E },
	{ "Gcaron",               0x01E6 },
	{ "Gcircumflex",          0x011C },
	{ "Gcommaaccent",         0x0122 },
	{ "Gdotaccent",           0x0120 },
#ifdef LARGEACCENTS
	{ "Grave",                0xF6CE }, /* 0xF7B5 */	/* Adobe special hack */
#endif
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Gravesmall",           0xF760 }, /* 0xF860 */	/* Adobe special hack */
#endif
#ifdef SMALLCAPS
	{ "Gsmall",               0xF767 }, /* 0xF867 */	/* Adobe special hack */
#endif
	{ "H",                    0x0048 },
	{ "H18533",				  0x25CF },	/* 97/Mar medium filled bullet */
	{ "H18543",				  0x25AA },	/* 97/Mar filled square bullet */
	{ "H18551",				  0x25AB },	/* 97/Mar open square bullet */
	{ "H22073",				  0x25A1 },	/* 97/Mar open square */
	{ "Hbar",                 0x0126 },
	{ "Hcircumflex",          0x0124 },
#ifdef SMALLCAPS
	{ "Hsmall",               0xF768 }, /* 0xF868 */	/* Adobe special hack */
#endif
#ifdef LARGEACCENTS
	{ "Hungarumlaut",         0xF6CF }, /* 0xF7B6 */	/* Adobe special hack */
#endif
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Hungarumlautsmall",    0xF6F8 }, /* 0xF7F8 */	/* Adobe special hack */
#endif
	{ "I",                    0x0049 },
	{ "IJ",                   0x0132 },
	{ "Iacute",               0x00CD },
#ifdef SMALLCAPS
	{ "Iacutesmall",          0xF7ED }, /* 0xF8ED */	/* Adobe special hack */
#endif
	{ "Ibreve",				  0x012C }, /* new 97/Mar */
	{ "Icircumflex",          0x00CE },
#ifdef SMALLCAPS
	{ "Icircumflexsmall",     0xF7EE }, /* 0xF8EE */	/* Adobe special hack */
#endif
	{ "Idieresis",            0x00CF },
#ifdef SMALLCAPS
	{ "Idieresissmall",       0xF7EF }, /* 0xF8EF */	/* Adobe special hack */
#endif
	{ "Idotaccent",           0x0130 },		/* Turkish indicator */
	{ "Ifraktur",             0x2111 },		/* Fraktur upper case I */
	{ "Igrave",               0x00CC },
#ifdef SMALLCAPS
	{ "Igravesmall",          0xF7EC }, /* 0xF8EC */	/* Adobe special hack */
#endif
	{ "Imacron",              0x012A },
	{ "Iogonek",              0x012E },
	{ "Iota",                 0x0399 },
	{ "Iotadieresis",         0x03AA },
	{ "Iotatonos",            0x038A },
#ifdef SMALLCAPS
	{ "Ismall",               0xF769 }, /* 0xF869 */	/* Adobe special hack */
#endif
	{ "Itilde",               0x0128 },
	{ "J",                    0x004A },
	{ "Jcircumflex",          0x0134 },
#ifdef SMALLCAPS
	{ "Jsmall",               0xF76A }, /* 0xF86A */	/* Adobe special hack */
#endif
	{ "K",                    0x004B },
	{ "Kappa",                0x039A },
	{ "Kcommaaccent",         0x0136 },
#ifdef SMALLCAPS
	{ "Ksmall",               0xF76B }, /* 0xF86B */	/* Adobe special hack */
#endif
	{ "L",                    0x004C },
/* #ifdef CORPORATEUSE */
	{ "LL",                   0xF6BF }, /* 0xF7A6 */	/* Adobe special hack */
/* #endif */
	{ "Lacute",               0x0139 },
	{ "Lambda",               0x039B },
#ifdef HP
	{ "Laplace",              0x2112 },	/* Laplace Transform NOT Adobe */
#endif
	{ "Lcaron",               0x013D },
	{ "Lcommaaccent",         0x013B },
	{ "Ldot",                 0x013F },
	{ "Lslash",               0x0141 },
#ifdef SMALLCAPS
	{ "Lslashsmall",          0xF6F9 }, /* 0xF7F9 */	/* Adobe special hack */
	{ "Lsmall",               0xF76C }, /* 0xF86C */	/* Adobe special hack */
#endif
	{ "M",                    0x004D },
#ifdef LARGEACCENTS
	{ "Macron",               0xF6D0 }, /* 0xF7B7 */	/* Adobe special hack */
#endif
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Macronsmall",          0xF7AF }, /* 0xF8AF */	/* Adobe special hack */
#endif
#ifdef SMALLCAPS
	{ "Msmall",               0xF76D }, /* 0xF86D */	/* Adobe special hack */
#endif
	{ "Mu",                   0x039C },
	{ "N",                    0x004E },
	{ "Nacute",               0x0143 },
#ifdef HP
	{ "Napierian",            0x212F },	/* italic e NOT Adobe */
#endif
	{ "Ncaron",               0x0147 },
	{ "Ncommaaccent",         0x0145 },
#ifdef SMALLCAPS
	{ "Nsmall",               0xF76E }, /* 0xF86E */	/* Adobe special hack */
#endif
	{ "Ntilde",               0x00D1 },
#ifdef SMALLCAPS
	{ "Ntildesmall",          0xF7F1 }, /* 0xF8F1 */	/* Adobe special hack */
#endif
	{ "Nu",                   0x039D },
	{ "O",                    0x004F },
	{ "OE",                   0x0152 },
#ifdef SMALLCAPS
	{ "OEsmall",              0xF6FA }, /* 0xF7FA */	/* Adobe special hack */
#endif
	{ "Oacute",               0x00D3 },
#ifdef SMALLCAPS
	{ "Oacutesmall",          0xF7F3 }, /* 0xF8F3 */	/* Adobe special hack */
#endif
	{ "Obreve",				  0x014E },	/* new 97/Mar */
	{ "Ocircumflex",          0x00D4 },
#ifdef SMALLCAPS
	{ "Ocircumflexsmall",     0xF7F4 }, /* 0xF8F4 */	/* Adobe special hack */
#endif
	{ "Odieresis",            0x00D6 },
#ifdef SMALLCAPS
	{ "Odieresissmall",       0xF7F6 }, /* 0xF8F6 */	/* Adobe special hack */
#endif
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Ogoneksmall",          0xF6FB }, /* 0xF7FB */	/* Adobe special hack */
#endif
	{ "Ograve",               0x00D2 },
#ifdef SMALLCAPS
	{ "Ogravesmall",          0xF7F2 }, /* 0xF8F2 */	/* Adobe special hack */
#endif
	{ "Ohorn",                0x01A0 },
	{ "Ohungarumlaut",        0x0150 },
	{ "Omacron",              0x014C },
	{ "Omega",                0x2126 },		/* Ohm sign */
	{ "Omega_",               0x03A9 },		/* duplicate */
/*	above two interchanged from Sairus Patel's table */ /* undone 98/Oct/28 */
	{ "Omegatonos",           0x038F },
	{ "Omicron",              0x039F },
	{ "Omicrontonos",         0x038C },
	{ "Oslash",               0x00D8 },
	{ "Oslashacute",		  0x01FE },	/* new 97/Mar */
#ifdef SMALLCAPS
	{ "Oslashsmall",          0xF7F8 }, /* 0xF8F8 */	/* Adobe special hack */
	{ "Osmall",               0xF76F }, /* 0xF86F */	/* Adobe special hack */
#endif
	{ "Otilde",               0x00D5 },
#ifdef SMALLCAPS
	{ "Otildesmall",          0xF7F5 }, /* 0xF8F5 */	/* Adobe special hack */
#endif
	{ "P",                    0x0050 },
	{ "Phi",                  0x03A6 },
	{ "Pi",                   0x03A0 },
#ifdef HP
	{ "Planckover2pi",        0x210F },		/* Planck's / 2 pi NOT Adobe */
#endif
	{ "Psi",                  0x03A8 },
#ifdef SMALLCAPS
	{ "Psmall",               0xF770 }, /* 0xF870 */	/* Adobe special hack */
#endif
	{ "Q",                    0x0051 },
#ifdef SMALLCAPS
	{ "Qsmall",               0xF771 }, /* 0xF871 */	/* Adobe special hack */
#endif
	{ "R",                    0x0052 },
	{ "Racute",               0x0154 },
	{ "Rcaron",               0x0158 },
	{ "Rcommaaccent",         0x0156 },
	{ "Rfraktur",             0x211C },	/* Fraktur Uppercase R */
	{ "Rho",                  0x03A1 },
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Ringsmall",            0xF6FC }, /* 0xF7FC */	/* Adobe special hack */
#endif
#ifdef SMALLCAPS
	{ "Rsmall",               0xF772 }, /* 0xF872 */	/* Adobe special hack */
#endif
	{ "S",                    0x0053 },
#ifdef BLOCKCHARS
	{ "SF010000",			0x250C },
	{ "SF020000",			0x2514 },
	{ "SF030000",			0x2510 },
	{ "SF040000",			0x2518 },
	{ "SF050000",			0x253C },
	{ "SF060000",			0x252C },
	{ "SF070000",			0x2534 },
	{ "SF080000",			0x251C },
	{ "SF090000",			0x2524 },
	{ "SF100000",			0x2500 },
	{ "SF110000",			0x2502 },
	{ "SF190000",			0x2561 },
	{ "SF200000",			0x2562 },
	{ "SF210000",			0x2556 },
	{ "SF220000",			0x2555 },
	{ "SF230000",			0x2563 },
	{ "SF240000",			0x2551 },
	{ "SF250000",			0x2557 },
	{ "SF260000",			0x255D },
	{ "SF270000",			0x255C },
	{ "SF280000",			0x255B },
	{ "SF360000",			0x255E },
	{ "SF370000",			0x255F },
	{ "SF380000",			0x255A },
	{ "SF390000",			0x2554 },
	{ "SF400000",			0x2569 },
	{ "SF410000",			0x2566 },
	{ "SF420000",			0x2560 },
	{ "SF430000",			0x2550 },
	{ "SF440000",			0x256C },
	{ "SF450000",			0x2567 },
	{ "SF460000",			0x2568 },
	{ "SF470000",			0x2564 },
	{ "SF480000",			0x2565 },
	{ "SF490000",			0x2559 },
	{ "SF500000",			0x2558 },
	{ "SF510000",			0x2552 },
	{ "SF520000",			0x2553 },
	{ "SF530000",			0x256B },
	{ "SF540000",			0x256A },
#endif
	{ "Sacute",               0x015A },
	{ "Scaron",               0x0160 },
#ifdef SMALLCAPS
	{ "Scaronsmall",          0xF6FD }, /* 0xF7FD */	/* Adobe special hack */
#endif
	{ "Scedilla",             0x015E },	/* 98/Oct/22 */
	{ "Scedilla_",            0xF6C1 },	/* change 97/Nov/24 duplicate */
/*	{ "Scedilla_",            0x1E9E }, */ 	/* Adobe special hack removed */
	{ "Scircumflex",          0x015C },
	{ "Scommaaccent",         0x0218 },	/* 98/Oct/22 0x015E */
	{ "Sigma",                0x03A3 },
#ifdef SMALLCAPS
	{ "Ssmall",               0xF773 }, /* 0xF873 */	/* Adobe special hack */
#endif
	{ "T",                    0x0054 },
	{ "Tau",                  0x03A4 },
	{ "Tbar",                 0x0166 },
	{ "Tcaron",               0x0164 },
	{ "Tcommaaccent",         0x0162 },
	{ "Tcommaaccent_",        0x021A },		/* 98/Oct/22 duplicate */
	{ "Theta",                0x0398 },
	{ "Thorn",                0x00DE },
#ifdef SMALLCAPS
	{ "Thornsmall",           0xF7FE }, /* 0xF8FE */	/* Adobe special hack */
#endif
#if defined(SMALLCAPS) && defined(LARGEACCENTS)
	{ "Tildesmall",           0xF6FE }, /* 0xF7FE */	/* Adobe special hack */
#endif
#ifdef SMALLCAPS
	{ "Tsmall",               0xF774 }, /* 0xF874 */	/* Adobe special hack */
#endif
	{ "U",                    0x0055 },
	{ "Uacute",               0x00DA },
#ifdef SMALLCAPS
	{ "Uacutesmall",          0xF7FA }, /* 0xF8FA */	/* Adobe special hack */
#endif
	{ "Ubreve",               0x016C },
	{ "Ucircumflex",          0x00DB },
#ifdef SMALLCAPS
	{ "Ucircumflexsmall",     0xF7FB }, /* 0xF8FB */	/* Adobe special hack */
#endif
	{ "Udieresis",            0x00DC },
#ifdef SMALLCAPS
	{ "Udieresissmall",       0xF7FC }, /* 0xF8FC */	/* Adobe special hack */
#endif
	{ "Ugrave",               0x00D9 },
#ifdef SMALLCAPS
	{ "Ugravesmall",          0xF7F9 }, /* 0xF8F9 */	/* Adobe special hack */
#endif
	{ "Uhorn",                0x01AF },
	{ "Uhungarumlaut",        0x0170 },
	{ "Umacron",              0x016A },
	{ "Uogonek",              0x0172 },
	{ "Upsilon",              0x03A5 },
	{ "Upsilon1",             0x03D2 },
	{ "Upsilondieresis",      0x03AB },
	{ "Upsilontonos",         0x038E },
	{ "Uring",                0x016E },
#ifdef SMALLCAPS
	{ "Usmall",               0xF775 }, /* 0xF875 */	/* Adobe special hack */
#endif
	{ "Utilde",               0x0168 },
	{ "V",                    0x0056 },
#ifdef SMALLCAPS
	{ "Vsmall",               0xF776 }, /* 0xF876 */	/* Adobe special hack */
#endif
	{ "W",                    0x0057 },
	{ "Wacute",				  0x1E82 },	/* new 97/Mar */
	{ "Wcircumflex",		  0x0174 },	/* new 97/Mar */
	{ "Wdieresis",			  0x1E84 },	/* new 97/Mar */
	{ "Wgrave",				  0x1E80 },	/* new 97/Mar */
#ifdef SMALLCAPS
	{ "Wsmall",               0xF777 }, /* 0xF877 */	/* Adobe special hack */
#endif
	{ "X",                    0x0058 },
	{ "Xi",                   0x039E },
#ifdef SMALLCAPS
	{ "Xsmall",               0xF778 }, /* 0xF878 */	/* Adobe special hack */
#endif
	{ "Y",                    0x0059 },
	{ "Yacute",               0x00DD },
#ifdef SMALLCAPS
	{ "Yacutesmall",          0xF7FD }, /* 0xF8FD */	/* Adobe special hack */
#endif
	{ "Ycircumflex",		  0x0176 },	/* new 97/Mar */
	{ "Ydieresis",            0x0178 },
#ifdef SMALLCAPS
	{ "Ydieresissmall",       0xF7FF }, /* 0xF8FF */	/* Adobe special hack */
#endif
	{ "Ygrave",				  0x1EF2 },	/* new 97/Mar */
#ifdef SMALLCAPS
	{ "Ysmall",               0xF779 }, /* 0xF879 */	/* Adobe special hack */
#endif
	{ "Z",                    0x005A },
	{ "Zacute",               0x0179 },
	{ "Zcaron",               0x017D },
#ifdef SMALLCAPS
	{ "Zcaronsmall",          0xF6FF }, /* 0xF7FF */	/* Adobe special hack */
#endif
	{ "Zdotaccent",           0x017B },
	{ "Zeta",                 0x0396 },
#ifdef HP
	{ "Zfraktur",             0x2128 },	/* Fraktur Uppercase Z NOT Adobe */
#endif
#ifdef SMALLCAPS
	{ "Zsmall",               0xF77A }, /* 0xF87A */	/* Adobe special hack */
#endif
	{ "a",                    0x0061 },
	{ "aacute",               0x00E1 },
	{ "abreve",               0x0103 },
	{ "acircumflex",          0x00E2 },
	{ "acute",                0x00B4 },
	{ "acutecomb",            0x0301 },
	{ "adieresis",            0x00E4 },
	{ "ae",                   0x00E6 },
	{ "aeacute",			  0x01FD },		/* new 97/Mar */
	{ "afii00208",            0x2015 },		/* quotedash */
	{ "afii10017",            0x0410 },		/* Acyril */
	{ "afii10018",            0x0411 },		/* Be */
	{ "afii10019",            0x0412 },		/* Ve */
	{ "afii10020",            0x0413 },		/* Ge */
	{ "afii10021",            0x0414 },		/* De */
	{ "afii10022",            0x0415 },		/* Ie */
	{ "afii10023",            0x0401 },		/* Io */
	{ "afii10024",            0x0416 },		/* Zhe */
	{ "afii10025",            0x0417 },		/* Ze */
	{ "afii10026",            0x0418 },		/* Ii */
	{ "afii10027",            0x0419 },		/* Iibreve */
	{ "afii10028",            0x041A },		/* Ka */
	{ "afii10029",            0x041B },		/* El */
	{ "afii10030",            0x041C },		/* Em */
	{ "afii10031",            0x041D },		/* En */
	{ "afii10032",            0x041E },		/* Ocyril */
	{ "afii10033",            0x041F },		/* Pecyril */
	{ "afii10034",            0x0420 },		/* Er */
	{ "afii10035",            0x0421 },		/* Es */
	{ "afii10036",            0x0422 },		/* Te */
	{ "afii10037",            0x0423 },		/* Ucyril */
	{ "afii10038",            0x0424 },		/* Ef */
	{ "afii10039",            0x0425 },		/* Kha */
	{ "afii10040",            0x0426 },		/* Tse */
	{ "afii10041",            0x0427 },		/* Che */
	{ "afii10042",            0x0428 },		/* Sha */
	{ "afii10043",            0x0429 },		/* Shcha */
	{ "afii10044",            0x042A },		/* Hard */
	{ "afii10045",            0x042B },		/* Yeri */
	{ "afii10046",            0x042C },		/* Soft */
	{ "afii10047",            0x042D },		/* Ecyrilrev */
	{ "afii10048",            0x042E },		/* Iu */
	{ "afii10049",            0x042F },		/* Ia */
	{ "afii10050",            0x0490 },		/* Geupturn */
	{ "afii10051",            0x0402 },		/* Dje */
	{ "afii10052",            0x0403 },		/* Gje */
	{ "afii10053",            0x0404 },		/* Ecyril */
	{ "afii10054",            0x0405 },		/* Dze */
	{ "afii10055",            0x0406 },		/* Icyril */
	{ "afii10056",            0x0407 },		/* Yi */
	{ "afii10057",            0x0408 },		/* Je */
	{ "afii10058",            0x0409 },		/* Lje */
	{ "afii10059",            0x040A },		/* Nje */
	{ "afii10060",            0x040B },		/* Tshe */
	{ "afii10061",            0x040C },		/* Kje */
	{ "afii10062",            0x040E },		/* Ucyrilbreve */
/* #ifdef CORPORATEUSE */
	{ "afii10063",            0xF6C4 }, /* 0xF7AB */	/* Serbian Ghe */
	{ "afii10064",            0xF6C5 }, /* 0xF7AC */	/* Serbian Be */
/* #endif */
	{ "afii10065",            0x0430 },		/* acyril */
	{ "afii10066",            0x0431 },		/* be */
	{ "afii10067",            0x0432 },		/* ve */
	{ "afii10068",            0x0433 },		/* ge */
	{ "afii10069",            0x0434 },		/* de */
	{ "afii10070",            0x0435 },		/* ie */
	{ "afii10071",            0x0451 },		/* io */	/* Russian indicator */
	{ "afii10072",            0x0436 },		/* zhe */
	{ "afii10073",            0x0437 },		/* ze */
	{ "afii10074",            0x0438 },		/* ii */
	{ "afii10075",            0x0439 },		/* iibreve */
	{ "afii10076",            0x043A },		/* ka */
	{ "afii10077",            0x043B },		/* el */
	{ "afii10078",            0x043C },		/* em */
	{ "afii10079",            0x043D },		/* en */
	{ "afii10080",            0x043E },		/* ocyril */
	{ "afii10081",            0x043F },		/* pecyril */
	{ "afii10082",            0x0440 },		/* er */
	{ "afii10083",            0x0441 },		/* es */
	{ "afii10084",            0x0442 },		/* te */
	{ "afii10085",            0x0443 },		/* ucyril */
	{ "afii10086",            0x0444 },		/* ef */
	{ "afii10087",            0x0445 },		/* kha */
	{ "afii10088",            0x0446 },		/* tse */
	{ "afii10089",            0x0447 },		/* che */
	{ "afii10090",            0x0448 },		/* sha */
	{ "afii10091",            0x0449 },		/* shcha */
	{ "afii10092",            0x044A },		/* hard */
	{ "afii10093",            0x044B },		/* yeri */
	{ "afii10094",            0x044C },		/* soft */
	{ "afii10095",            0x044D },		/* ecyrilrev */
	{ "afii10096",            0x044E },		/* iu */
	{ "afii10097",            0x044F },		/* ia */
	{ "afii10098",            0x0491 },		/* geupturn */
	{ "afii10099",            0x0452 },		/* dje */
	{ "afii10100",            0x0453 },		/* gje */
	{ "afii10101",            0x0454 },		/* ecyril */
	{ "afii10102",            0x0455 },		/* dze */
	{ "afii10103",            0x0456 },		/* icyril */
	{ "afii10104",            0x0457 },		/* yi */
	{ "afii10105",            0x0458 },		/* je */
	{ "afii10106",            0x0459 },		/* lje */
	{ "afii10107",            0x045A },		/* nje */
	{ "afii10108",            0x045B },		/* tshe */
	{ "afii10109",            0x045C },		/* kje */
	{ "afii10110",            0x045E },		/* ucyrilbreve */
	{ "afii10145",            0x040F },		/* Dzhe */
	{ "afii10146",            0x0462 },		/* Yat */
	{ "afii10147",            0x0472 },
	{ "afii10148",            0x0474 },
/* #ifdef CORPORATEUSE */
	{ "afii10192",            0xF6C6 }, /* 0xF7AD */	/* Serbian De */
/* #endif */
	{ "afii10193",            0x045F },		/* dzhe */
	{ "afii10194",            0x0463 },		/* yat */
	{ "afii10195",            0x0473 },
	{ "afii10196",            0x0475 },
/* #ifdef CORPORATEUSE */
	{ "afii10831",            0xF6C7 }, /* 0xF7AE */	/* Serbian Pe */
	{ "afii10832",            0xF6C8 }, /* 0xF7AF */	/* Serbian Te */
/* #endif */
	{ "afii10846",            0x04D9 },
	{ "afii299",			  0x200E },		/* leftotright */
	{ "afii300",			  0x200F },		/* righttoleft */
	{ "afii301",			  0x200D },		/* zerojoin */
	{ "afii57381",			  0x066A },		/* new 97/Mar */
	{ "afii57388",            0x060C },
	{ "afii57392",            0x0660 },
	{ "afii57393",            0x0661 },
	{ "afii57394",            0x0662 },
	{ "afii57395",            0x0663 },
	{ "afii57396",            0x0664 },
	{ "afii57397",            0x0665 },
	{ "afii57398",            0x0666 },
	{ "afii57399",            0x0667 },
	{ "afii57400",            0x0668 },
	{ "afii57401",            0x0669 },
	{ "afii57403",            0x061B },
	{ "afii57407",            0x061F },
	{ "afii57409",            0x0621 },
	{ "afii57410",            0x0622 },		/*  Arabic indicator */
	{ "afii57411",            0x0623 },
	{ "afii57412",            0x0624 },
	{ "afii57413",            0x0625 },
	{ "afii57414",            0x0626 },
	{ "afii57415",            0x0627 },
	{ "afii57416",            0x0628 },
	{ "afii57417",            0x0629 },
	{ "afii57418",            0x062A },
	{ "afii57419",            0x062B },
	{ "afii57420",            0x062C },
	{ "afii57421",            0x062D },
	{ "afii57422",            0x062E },
	{ "afii57423",            0x062F },
	{ "afii57424",            0x0630 },
	{ "afii57425",            0x0631 },
	{ "afii57426",            0x0632 },
	{ "afii57427",            0x0633 },
	{ "afii57428",            0x0634 },
	{ "afii57429",            0x0635 },
	{ "afii57430",            0x0636 },
	{ "afii57431",            0x0637 },
	{ "afii57432",            0x0638 },
	{ "afii57433",            0x0639 },
	{ "afii57434",            0x063A },
	{ "afii57440",            0x0640 },
	{ "afii57441",            0x0641 },
	{ "afii57442",            0x0642 },
	{ "afii57443",            0x0643 },
	{ "afii57444",            0x0644 },
	{ "afii57445",            0x0645 },
	{ "afii57446",            0x0646 },
	{ "afii57448",            0x0648 },
	{ "afii57449",            0x0649 },
	{ "afii57450",            0x064A },
	{ "afii57451",            0x064B },
	{ "afii57452",            0x064C },
	{ "afii57453",            0x064D },
	{ "afii57454",            0x064E },
	{ "afii57455",            0x064F },
	{ "afii57456",            0x0650 },
	{ "afii57457",            0x0651 },
	{ "afii57458",            0x0652 },
	{ "afii57470",            0x0647 },
	{ "afii57505",			  0x06A4 },		/* new 97/Mar */
	{ "afii57506",            0x067E },
	{ "afii57507",            0x0686 },
	{ "afii57508",            0x0698 },
	{ "afii57509",            0x06AF },
	{ "afii57511",			  0x0679 },		/* new 97/Mar */
	{ "afii57512",			  0x0688 },		/* new 97/Mar */
	{ "afii57513",			  0x0691 },		/* new 97/Mar */
	{ "afii57514",			  0x06BA },		/* new 97/Mar */
	{ "afii57519",			  0x06D2 },		/* new 97/Mar */
	{ "afii57534",			  0x06D5 },		/* new 97/Mar */
#ifdef IGNORED
	{ "afii57596",            0x200E },		/* => AFII299  lefttoright ? */
	{ "afii57597",            0x200F },		/* => AFII230  rightoleft ? */
	{ "afii57598",            0x200D },		/* => AFII231  zerojoin ? */
#endif
	{ "afii57636",            0x20AA },		/* newsheqel */
	{ "afii57645",            0x05BE },		/* maqaf */
	{ "afii57658",            0x05C3 },		/* sofpasuq */
	{ "afii57664",            0x05D0 },		/* alef */	/* Hebrew indicator */
	{ "afii57665",            0x05D1 },		/* bet */
	{ "afii57666",            0x05D2 },		/* gimel */
	{ "afii57667",            0x05D3 },		/* dalet */
	{ "afii57668",            0x05D4 },		/* he */
	{ "afii57669",            0x05D5 },		/* vav */
	{ "afii57670",            0x05D6 },		/* zayin */
	{ "afii57671",            0x05D7 },		/* het */
	{ "afii57672",            0x05D8 },		/* tet */
	{ "afii57673",            0x05D9 },		/* yod */
	{ "afii57674",            0x05DA },		/* kaffinal */
	{ "afii57675",            0x05DB },		/* kaf */
	{ "afii57676",            0x05DC },		/* lamed */
	{ "afii57677",            0x05DD },		/* memfinal */
	{ "afii57678",            0x05DE },		/* mem */
	{ "afii57679",            0x05DF },		/* nunfinal */
	{ "afii57680",            0x05E0 },		/* nun */
	{ "afii57681",            0x05E1 },		/* samekh */
	{ "afii57682",            0x05E2 },		/* ayin */
	{ "afii57683",            0x05E3 },		/* pefinal */
	{ "afii57684",            0x05E4 },		/* pe */
	{ "afii57685",            0x05E5 },		/* tsadifinal */
	{ "afii57686",            0x05E6 },		/* tsadi */
	{ "afii57687",            0x05E7 },		/* qof */
	{ "afii57688",            0x05E8 },		/* resh */
	{ "afii57689",            0x05E9 },		/* shin */
	{ "afii57690",            0x05EA },		/* tav */
	{ "afii57694",			  0xFB2A },		/* alphabetic presentation form */
	{ "afii57695",			  0xFB2B },		/* alphabetic presentation form */
	{ "afii57700",			  0xFB4B },		/* alphabetic presentation form */
	{ "afii57705",			  0xFB1F },		/* alphabetic presentation form */
	{ "afii57716",            0x05F0 },		/* vavdbl */
	{ "afii57717",            0x05F1 },		/* vavyod */
	{ "afii57718",            0x05F2 },		/* yoddbl */
	{ "afii57723",			  0xFB35 },		/* alphabetic presentation form */
	{ "afii57793",            0x05B4 },		/* hiriq */
	{ "afii57794",            0x05B5 },		/* tsere */
	{ "afii57795",            0x05B6 },		/* segol */
	{ "afii57796",            0x05BB },		/* qubuts */
	{ "afii57797",            0x05B8 },		/* qamats */
	{ "afii57798",            0x05B7 },		/* patah */
	{ "afii57799",            0x05B0 },		/* sheva */
	{ "afii57800",            0x05B2 },		/* hatafpatah */
	{ "afii57801",            0x05B1 },		/* hatafsegol */
	{ "afii57802",            0x05B3 },		/* hatafqamats */
	{ "afii57803",            0x05C2 },		/* sindot */
	{ "afii57804",            0x05C1 },		/* shindot */
	{ "afii57806",            0x05B9 },		/* holam */
	{ "afii57807",            0x05BC },		/* dagesh */
	{ "afii57839",            0x05BD },		/* meteg */
	{ "afii57841",            0x05BF },		/* rafe */
	{ "afii57842",            0x05C0 },		/* paseq */
	{ "afii57929",            0x02BC },		/* modifier letter apostrophe */
											/* not quoteright - which is 2019 */
	{ "afii61248",            0x2105 },		/* careof c/o */
	{ "afii61289",            0x2113 },		/* lscript litre liter */
	{ "afii61352",            0x2116 },		/* numero */
	{ "afii61573",			  0x202C },		/* pop directional new 97/Mar */
	{ "afii61574",			  0x202D },		/* left-to-right override 97/Mar */
	{ "afii61575",			  0x202E },		/* right-to-left override 97/Mar */
	{ "afii61664",            0x200C },		/* zerowidth no joiner ? */
	{ "afii63167",			  0x066D },		/* Arabic 5 pointed star 97/Mar */
	{ "afii64937",            0x02BD },		/* apostropherev ? commareverse */
	{ "agrave",               0x00E0 },
	{ "aleph",                0x2135 },
	{ "alpha",                0x03B1 },
	{ "alphatonos",           0x03AC },
	{ "amacron",              0x0101 },
	{ "ampersand",            0x0026 },
#ifdef SMALLCAPS
	{ "ampersandsmall",       0xF726 }, /* 0xF826 */	/* Adobe special hack */
#endif
	{ "angle",                0x2220 },
	{ "angleleft",            0x2329 },
	{ "angleright",           0x232A },
	{ "anoteleia",			  0x0387 },	/* new 97/Mar */
	{ "aogonek",              0x0105 },
	{ "approxequal",          0x2248 },	/* almostequal */
	{ "aring",                0x00E5 },
	{ "aringacute",			  0x01FB },	/* new 97/Mar */ /* aacutering */
#ifdef HP
	{ "arrow-135",			  0x2199 },
	{ "arrow-45",			  0x2198 },
	{ "arrow+135",			  0x2196 },
	{ "arrow+45",			  0x2197 },
#endif
	{ "arrowboth",            0x2194 },
	{ "arrowdblboth",         0x21D4 },
	{ "arrowdbldown",         0x21D3 },
	{ "arrowdblleft",         0x21D0 },
	{ "arrowdblright",        0x21D2 },
	{ "arrowdblup",           0x21D1 },
#ifdef HP
	{ "arrowdblupdown",       0x21D5 },	/* Up/Down Arrow Double Stroke */
#endif
	{ "arrowdown",            0x2193 },
#ifdef MATHEXTENSION
	{ "arrowhorizex",         0xF8E7 }, /* 0xF7C0 */	/* Adobe special hack */
#endif
	{ "arrowleft",            0x2190 },
#ifdef HP
	{ "arrowleftoverright",	  0x21C6 },
#endif
	{ "arrowright",           0x2192 },
#ifdef HP
	{ "arrowrightoverleft",	  0x21C4 },
#endif
	{ "arrowup",              0x2191 },
	{ "arrowupdn",			  0x2195 },	/* new 97/Mar */
	{ "arrowupdnbse",		  0x21A8 },	/* new 97/Mar */
#ifdef MATHEXTENSION
	{ "arrowvertex",          0xF8E6 }, /* 0xF7C1 */	/* Adobe special hack */
#endif
	{ "asciicircum",          0x005E },
	{ "asciitilde",           0x007E },
	{ "asterisk",             0x002A },
	{ "asteriskmath",         0x2217 },
#ifdef SUPERIOR
	{ "asuperior",            0xF6E9 }, /* 0xF7E9 */	/* Adobe special hack */
#endif
	{ "at",                   0x0040 },
	{ "atilde",               0x00E3 },
	{ "b",                    0x0062 },
	{ "backslash",            0x005C },
	{ "bar",                  0x007C },
	{ "beta",                 0x03B2 },
#ifdef HP
	{ "beth",                 0x2136 },	/* NOT Adobe */
#endif
	{ "block",				  0x2588 },	/* solid full high & wide */
#ifdef MATHEXTENSION
	{ "braceex",              0xF8F4 }, /* 0xF7C2 */	/* Adobe special hack */
#endif
	{ "braceleft",            0x007B },
#ifdef MATHEXTENSION
	{ "braceleftbt",          0xF8F3 }, /* 0xF7C3 */	/* Adobe special hack */
	{ "braceleftmid",         0xF8F2 }, /* 0xF7C4 */	/* Adobe special hack */
	{ "bracelefttp",          0xF8F1 }, /* 0xF7C5 */	/* Adobe special hack */
#endif
	{ "braceright",           0x007D },
#ifdef MATHEXTENSION
	{ "bracerightbt",         0xF8FE }, /* 0xF7C6 */	/* Adobe special hack */
	{ "bracerightmid",        0xF8FD }, /* 0xF7C7 */	/* Adobe special hack */
	{ "bracerighttp",         0xF8FC }, /* 0xF7C8 */	/* Adobe special hack */
#endif
	{ "bracketleft",          0x005B },
#ifdef MATHEXTENSION
	{ "bracketleftbt",        0xF8F0 }, /* 0xF7C9 */	/* Adobe special hack */
	{ "bracketleftex",        0xF8EF }, /* 0xF7CA */	/* Adobe special hack */
#ifdef HP
	{ "bracketleftdbl",       0x301A },	/* NOT Adobe */
#endif
	{ "bracketlefttp",        0xF8EE }, /* 0xF7CB */	/* Adobe special hack */
#endif
	{ "bracketright",         0x005D },
#ifdef MATHEXTENSION
	{ "bracketrightbt",       0xF8FB }, /* 0xF7CC */	/* Adobe special hack */
	{ "bracketrightex",       0xF8FA }, /* 0xF7CD */	/* Adobe special hack */
#ifdef HP
	{ "bracketrightdbl",      0x301B },	/* NOT Adobe */
#endif
	{ "bracketrighttp",       0xF8F9 }, /* 0xF7CE */	/* Adobe special hack */
#endif
	{ "breve",                0x02D8 },
#ifdef IGNORED
	{ "brevecomb",            0x0306 }, 	/* ??? */
#endif
	{ "brokenbar",            0x00A6 },
#ifdef SUPERIOR
	{ "bsuperior",            0xF6EA }, /* 0xF7EA */	/* Adobe special hack */
#endif
	{ "bullet",               0x2022 },	/* small filled round bullet */
	{ "c",                    0x0063 },
	{ "cacute",               0x0107 },
	{ "caron",                0x02C7 },
	{ "carriagereturn",       0x21B5 },	/* CR */
	{ "ccaron",               0x010D },
	{ "ccedilla",             0x00E7 },
	{ "ccircumflex",          0x0109 },
	{ "cdotaccent",           0x010B },
	{ "cedilla",              0x00B8 },
	{ "cent",                 0x00A2 },
#ifdef INFERIOR
	{ "centinferior",         0xF6DF }, /* 0xF7DF */	/* Adobe special hack */
#endif
#ifdef OLDSTYLE
	{ "centoldstyle",         0xF7A2 }, /* 0xF8A2 */	/* Adobe special hack */
#endif
#ifdef SUPERIOR
	{ "centsuperior",         0xF6E0 }, /* 0xF7E0 */	/* Adobe special hack */
#endif
	{ "chi",                  0x03C7 },
	{ "circle",               0x25CB },		/* SM750000 medium open round */
	{ "circlemultiply",       0x2297 },
	{ "circleplus",           0x2295 },
	{ "circumflex",           0x02C6 },
#ifdef IGNORED
	{ "circumflexcomb",       0x0302 },		/* ??? */
#endif
	{ "club",                 0x2663 },	/* solid */
	{ "colon",                0x003A },
	{ "colonmonetary",        0x20A1 },
	{ "comma",                0x002C },
/* #ifdef CORPORATEUSE */
	{ "commaaccent",          0xF6C3 }, /* 0xF7AA */	/* Adobe special hack */
/* #endif */
/* #ifdef INFERIOR */
	{ "commainferior",        0xF6E1 }, /* 0xF7E1 */	/* Adobe special hack */
/* #endif */
/* #ifdef SUPERIOR */
	{ "commasuperior",        0xF6E2 }, /* 0xF7E2 */	/* Adobe special hack */
/* #endif */
	{ "congruent",            0x2245 },
	{ "copyright",            0x00A9 },
#ifdef CORPORATEUSE
	{ "copyrightsans",        0xF8E9 }, /* 0xF7CF */	/* Adobe special hack */
	{ "copyrightserif",       0xF6D9 }, /* 0xF7D0 */	/* Adobe special hack */
#endif
	{ "currency",             0x00A4 },
#ifdef CORPORATEUSE
	{ "cyrBreve",             0xF6D1 }, /* 0xF7B8 */	/* Adobe special hack */
	{ "cyrFlex",              0xF6D2 }, /* 0xF7B9 */	/* Adobe special hack */
#endif
/* #ifdef CORPORATEUSE */
	{ "cyrbreve",             0xF6D4 }, /* 0xF7BB */	/* Adobe special hack */
	{ "cyrflex",              0xF6D5 }, /* 0xF7BC */	/* Adobe special hack */
/* #endif */
	{ "d",                    0x0064 },
	{ "dagger",               0x2020 },
	{ "daggerdbl",            0x2021 },
#ifdef LARGEACCENTS
	{ "dblGrave",             0xF6D3 }, /* 0xF7BA */	/* Adobe special hack */
#endif
/* #ifdef CORPORATEUSE */
	{ "dblgrave",             0xF6D6 }, /* 0xF7BD */	/* Adobe special hack */
/* #endif */
	{ "dcaron",               0x010F },
	{ "dcroat",               0x0111 },
	{ "degree",               0x00B0 },
	{ "delta",                0x03B4 },
	{ "diamond",              0x2666 },	/* solid */
	{ "dieresis",             0x00A8 },
#ifdef IGNORED
	{ "dieresiscomb",         0x0308 }, 	/* ??? */
#endif
/* #ifdef CORPORATEUSE */
	{ "dieresisgrave",        0xF6D8 }, /* 0xF7BF */	/* Adobe special hack */
/* #endif */
	{ "dieresistonos",        0x0385 },
	{ "divide",               0x00F7 },
	{ "dkshade",			  0x2593 },	/* dark shading */
	{ "dnblock",			  0x2584 },	/* bottom half solid */
	{ "dollar",               0x0024 },
#ifdef INFERIOR
	{ "dollarinferior",       0xF6E3 }, /* 0xF7E3 */	/* Adobe special hack */
#endif
#ifdef OLDSTYLE
	{ "dollaroldstyle",       0xF724 }, /* 0xF824 */	/* Adobe special hack */
#endif
#ifdef SUPERIOR
	{ "dollarsuperior",       0xF6E4 }, /* 0xF7E4 */	/* Adobe special hack */
#endif
	{ "dong",                 0x20AB },
	{ "dotaccent",            0x02D9 },
#ifdef IGNORED
	{ "dotaccentcomb",        0x0307 },	/* ??? */
#endif
	{ "dotbelowcomb",         0x0323 },	/* dotsubnosp ? */
	{ "dotlessi",             0x0131 },
	{ "dotlessj",             0xF6BE },			/* 98/Oct/22 was 0xFB0F */
	{ "dotmath",              0x22C5 },
#ifdef SUPERIOR
	{ "dsuperior",            0xF6EB }, /* 0xF7EB */	/* Adobe special hack */
#endif
	{ "e",                    0x0065 },
	{ "eacute",               0x00E9 },
	{ "ebreve",				  0x0115 },	/* new 97/Mar */
	{ "ecaron",               0x011B },
	{ "ecircumflex",          0x00EA },		/* Windows ANSI indicator */
	{ "edieresis",            0x00EB },
	{ "edotaccent",           0x0117 },
	{ "egrave",               0x00E8 },
	{ "eight",                0x0038 },
	{ "eightinferior",        0x2088 },
#ifdef OLDSTYLE
	{ "eightoldstyle",        0xF738 }, /* 0xF838 */	/* Adobe special hack */
#endif
	{ "eightsuperior",        0x2078 },
	{ "element",              0x2208 },
	{ "ellipsis",             0x2026 },
	{ "emacron",              0x0113 },
	{ "emdash",               0x2014 },
	{ "emptyset",             0x2205 },
#ifdef HP
	{ "emquad",               0x2001 },
	{ "emspace",              0x2003 },	
#endif
	{ "endash",               0x2013 },
	{ "eng",                  0x014B },
#ifdef HP
	{ "enquad",               0x2000 },
	{ "enspace",              0x2002 },
#endif
	{ "eogonek",              0x0119 },
	{ "epsilon",              0x03B5 },
	{ "epsilontonos",         0x03AD },
	{ "equal",                0x003D },
	{ "equivalence",          0x2261 },
	{ "estimated",			  0x212E },	/* new 97/Mar */
#ifdef SUPERIOR
	{ "esuperior",            0xF6EC }, /* 0xF7EC */	/* Adobe special hack */
#endif
	{ "eta",                  0x03B7 },
	{ "etatonos",             0x03AE },
	{ "eth",                  0x00F0 },
	{ "exclam",               0x0021 },
	{ "exclamdbl",            0x203C },	/* double exclamation mark */
	{ "exclamdown",           0x00A1 },
#ifdef SMALLCAPS
	{ "exclamdownsmall",      0xF7A1 }, /* 0xF8A1 */	/* Adobe special hack */
	{ "exclamsmall",          0xF721 }, /* 0xF821 */	/* Adobe special hack */
#endif
	{ "existential",          0x2203 },
	{ "f",                    0x0066 },
	{ "female",				  0x2640 },	/* new 97/Mar */
	{ "ff",                   0xFB00 },		/* f ligatures */
	{ "ffi",                  0xFB03 },		/* f ligatures */
	{ "ffl",                  0xFB04 },		/* f ligatures */
	{ "fi",                   0xFB01 },		/* f ligatures */
	{ "figuredash",           0x2012 },
	{ "filledbox",			  0x25A0 },	/* medium solid square box */
	{ "filledrect",			  0x25AC },	/* thick horizontal mark */
	{ "five",                 0x0035 },
	{ "fiveeighths",          0x215D },
	{ "fiveinferior",         0x2085 },
#ifdef OLDSTYLE
	{ "fiveoldstyle",         0xF735 }, /* 0xF835 */	/* Adobe special hack */
#endif
	{ "fivesuperior",         0x2075 },
	{ "fl",                   0xFB02 }, /* 0xFB02 */		/* f ligatures */
	{ "florin",               0x0192 },
	{ "four",                 0x0034 },
	{ "fourinferior",         0x2084 },
#ifdef OLDSTYLE
	{ "fouroldstyle",         0xF734 }, /* 0xF834 */	/* Adobe special hack */
#endif
	{ "foursuperior",         0x2074 },
	{ "fraction",             0x2044 },	/* fraction slash */
	{ "fraction_",            0x2215 }, /* division slash change duplicate */
	{ "franc",                0x20A3 },
	{ "g",                    0x0067 },
	{ "gamma",                0x03B3 },
	{ "gbreve",               0x011F },
	{ "gcaron",               0x01E7 },
	{ "gcircumflex",          0x011D },
	{ "gcommaaccent",         0x0123 },
	{ "gdotaccent",           0x0121 },
	{ "germandbls",           0x00DF },
#ifdef HP
	{ "gimmel",               0x2137 },
#endif
	{ "gradient",             0x2207 },
	{ "grave",                0x0060 },
	{ "gravecomb",            0x0300 },
	{ "greater",              0x003E },
	{ "greaterequal",         0x2265 },
	{ "guillemotleft",        0x00AB },
	{ "guillemotright",       0x00BB },
	{ "guilsinglleft",        0x2039 },
	{ "guilsinglright",       0x203A },
	{ "h",                    0x0068 },
	{ "hbar",                 0x0127 },
	{ "hcircumflex",          0x0125 },
	{ "heart",                0x2665 },	/* solid */
	{ "hookabovecomb",        0x0309 },
	{ "house",                0x2302 },	/* SM790000 97/Apr/5 */
	{ "hungarumlaut",         0x02DD },
	{ "hyphen",               0x002D },	/* hyphen-minus */
	{ "hyphen_",              0x00AD },	/* sfthyphen duplicate */
/* #ifdef INFERIOR*/
/* #endif */
/* #ifdef SUPERIOR */
	{ "hyphensuperior",       0xF6E6 }, /* 0xF7E6 */	/* Adobe special hack */
/* #endif */
	{ "i",                    0x0069 },
	{ "iacute",               0x00ED },
	{ "ibreve",				  0x012D },	/* new 97/Mar */
	{ "icircumflex",          0x00EE },
	{ "idieresis",            0x00EF },
	{ "igrave",               0x00EC },
	{ "ij",                   0x0133 },
	{ "imacron",              0x012B },
	{ "infinity",             0x221E },
	{ "integral",             0x222B },
	{ "integralbt",           0x2321 },
#ifdef MATHEXTENSION
	{ "integralex",           0xF8F5 }, /* 0xF7D1 */	/* Adobe special hack */
#endif
	{ "integraltp",           0x2320 },
	{ "intersection",         0x2229 },
	{ "invbullet",			  0x25D8 },	/* large solid square with white dot */
	{ "invcircle",			  0x25D9 },	/* large solid square with white circle */
	{ "invsmileface",		  0x263B },	/* solid smiling face */
	{ "iogonek",              0x012F },
	{ "iota",                 0x03B9 },
	{ "iotadieresis",         0x03CA },
	{ "iotadieresistonos",    0x0390 },
	{ "iotatonos",            0x03AF },
#ifdef SUPERIOR
	{ "isuperior",            0xF6ED }, /* 0xF7ED */	/* Adobe special hack */
#endif
	{ "itilde",               0x0129 },
	{ "j",                    0x006A },
	{ "jcircumflex",          0x0135 },
	{ "k",                    0x006B },
	{ "kappa",                0x03BA },
	{ "kcommaaccent",         0x0137 },
	{ "kgreenlandic",         0x0138 },	/* kra */
	{ "l",                    0x006C },
	{ "lacute",               0x013A },
	{ "lambda",               0x03BB },
	{ "lcaron",               0x013E },
	{ "lcommaaccent",         0x013C },
	{ "ldot",                 0x0140 },
	{ "less",                 0x003C },
	{ "lessequal",            0x2264 },
	{ "lfblock",			  0x258C },	/* left half solid rectangle */
	{ "lira",                 0x20A4 },
/* #ifdef CORPORATEUSE */
	{ "ll",                   0xF6C0 }, /* 0xF7A7 */	/* Adobe special hack */
/* #endif */
	{ "logicaland",           0x2227 },
	{ "logicalnot",           0x00AC },
	{ "logicalor",            0x2228 },
	{ "longs",				  0x017F },	/* new 97/Mar */
	{ "lozenge",              0x25CA },	/* diamond */
	{ "lslash",               0x0142 },
#ifdef SUPERIOR
	{ "lsuperior",            0xF6EE }, /* 0xF7EE */	/* Adobe special hack */
#endif
	{ "ltshade",			  0x2591 },	/* light shading */
	{ "m",                    0x006D },
	{ "macron",               0x00AF },
	{ "macron_",			  0x02C9 },	/* new 97/Mar duplicate */
#ifdef IGNORED
	{ "macroncomb",           0x0304 },	/* ??? */
#endif
	{ "male",				  0x2642 },	/* new 97/Mar */
	{ "minus",                0x2212 },
	{ "minute",               0x2032 },	/* prime, feet */
#ifdef SUPERIOR
	{ "msuperior",            0xF6EF }, /* 0xF7EF */	/* Adobe special hack */
#endif
	{ "mu",                   0x00B5 },	/* micro sign */
	{ "mu_",                  0x03BC },	/* Greek mu duplicate */
	{ "multiply",             0x00D7 },
	{ "musicalnote",		  0x266A },	
	{ "musicalnotedbl",		  0x266B },	/* pair of musical notes */
	{ "n",                    0x006E },
	{ "nacute",               0x0144 },
	{ "napostrophe",          0x0149 },	/* Afrikaans */
	{ "ncaron",               0x0148 },	/* Eastern European indicator */
	{ "ncommaaccent",         0x0146 },
	{ "nine",                 0x0039 },
	{ "nineinferior",         0x2089 },
#ifdef OLDSTYLE
	{ "nineoldstyle",         0xF739 }, /* 0xF839 */	/* Adobe special hack */
#endif
	{ "ninesuperior",         0x2079 },
	{ "notelement",           0x2209 },
	{ "notequal",             0x2260 },
	{ "notsubset",            0x2284 },
	{ "nsuperior",            0x207F },
	{ "ntilde",               0x00F1 },
	{ "nu",                   0x03BD },
	{ "numbersign",           0x0023 },
	{ "o",                    0x006F },
	{ "oacute",               0x00F3 },
	{ "obreve",				  0x014F },	/* new 97/Mar */
	{ "ocircumflex",          0x00F4 },
	{ "odieresis",            0x00F6 },
	{ "oe",                   0x0153 },
	{ "ogonek",               0x02DB },
	{ "ograve",               0x00F2 },
	{ "ohorn",                0x01A1 },
	{ "ohungarumlaut",        0x0151 },
	{ "omacron",              0x014D },
	{ "omega",                0x03C9 },
	{ "omega1",               0x03D6 },		/* ? */
	{ "omegatonos",           0x03CE },
	{ "omicron",              0x03BF },
	{ "omicrontonos",         0x03CC },
	{ "one",                  0x0031 },
	{ "onedotenleader",       0x2024 },
	{ "oneeighth",            0x215B },
/* #ifdef CORPORATEUSE */
	{ "onefitted",            0xF6DC }, /* 0xF7DC */	/* Adobe special hack */
/* #endif */
	{ "onehalf",              0x00BD },
	{ "oneinferior",          0x2081 },
#ifdef OLDSTYLE
	{ "oneoldstyle",          0xF731 }, /* 0xF831 */	/* Adobe special hack */
#endif
	{ "onequarter",           0x00BC },
	{ "onesuperior",          0x00B9 },
	{ "onethird",             0x2153 },
	{ "openbullet",			  0x25E6 },	/* small open round bullet */
#ifdef HP
	{ "opencircle",           0x20DD },	/* large open circle NOT Adobe */
#endif
	{ "ordfeminine",          0x00AA },
	{ "ordmasculine",         0x00BA },
	{ "orthogonal",			  0x221F },	/* new 97/Mar */ /* rightangle */
	{ "oslash",               0x00F8 },
	{ "oslashacute",		  0x01FF },	/* new 97/Mar */
#ifdef SUPERIOR
	{ "osuperior",            0xF6F0 }, /* 0xF7F0 */	/* Adobe special hack */
#endif
	{ "otilde",               0x00F5 },
#ifdef IGNORED
	{ "overscore",			  0x00AF },		/* not in ATM */
	{ "overscore_",			  0x203E },		/* not in ATM */
#endif
	{ "p",                    0x0070 },
	{ "paragraph",            0x00B6 },
	{ "parenleft",            0x0028 },
#ifdef MATHEXTENSION
	{ "parenleftbt",          0xF8ED }, /* 0xF7D2 */	/* Adobe special hack */
	{ "parenleftex",          0xF8EC }, /* 0xF7D3 */	/* Adobe special hack */
#endif
	{ "parenleftinferior",    0x208D },
	{ "parenleftsuperior",    0x207D },
#ifdef MATHEXTENSION
	{ "parenlefttp",          0xF8EB }, /* 0xF7D4 */	/* Adobe special hack */
#endif
	{ "parenright",           0x0029 },
#ifdef MATHEXTENSION
	{ "parenrightbt",         0xF8F8 }, /* 0xF7D5 */	/* Adobe special hack */
	{ "parenrightex",         0xF8F7 }, /* 0xF7D6 */	/* Adobe special hack */
#endif
	{ "parenrightinferior",   0x208E },
	{ "parenrightsuperior",   0x207E },
#ifdef MATHEXTENSION
	{ "parenrighttp",         0xF8F6 }, /* 0xF7D7 */	/* Adobe special hack */
#endif
	{ "partialdiff",          0x2202 },
	{ "percent",              0x0025 },
	{ "period",               0x002E },
	{ "periodcentered",       0x00B7 },	/* middle dot */
	{ "periodcentered_",	  0x2219 },	/* new 97/Mar bullet math duplicate */
/* #ifdef INFERIOR */
	{ "periodinferior",       0xF6E7 }, /* 0xF7E7 */	/* Adobe special hack */
/* #endif */
/* #ifdef SUPERIOR */
	{ "periodsuperior",       0xF6E8 }, /* 0xF7E8 */	/* Adobe special hack */
/* #endif */
	{ "perpendicular",        0x22A5 },
#ifdef HP
	{ "pertenthousand",       0x2031 },		/* not in ATM */
#endif
	{ "perthousand",          0x2030 },		/* per mill sign */
	{ "peseta",				  0x20A7 },		/* Pts, pesetas */
	{ "phi",                  0x03C6 },
	{ "phi1",                 0x03D5 },		/* Greek phi symbol */
	{ "pi",                   0x03C0 },
	{ "plus",                 0x002B },
	{ "plusminus",            0x00B1 },
	{ "prescription",         0x211E },		/* Rx */
	{ "product",              0x220F },
	{ "propersubset",         0x2282 },
	{ "propersuperset",       0x2283 },
	{ "proportional",         0x221D },
	{ "psi",                  0x03C8 },
	{ "q",                    0x0071 },
	{ "question",             0x003F },
	{ "questiondown",         0x00BF },
#ifdef SMALLCAPS
	{ "questiondownsmall",    0xF7BF }, /* 0xF8BF */	/* Adobe special hack */
	{ "questionsmall",        0xF73F }, /* 0xF83F */	/* Adobe special hack */
#endif
	{ "quotedbl",             0x0022 },
	{ "quotedblbase",         0x201E },	/* double baseline quote */
	{ "quotedblleft",         0x201C },	/* double open quote */
	{ "quotedblright",        0x201D },	/* double close quote */
#ifdef HP
	{ "quotedblreversed",	  0x201F },	/* not in ATM ... */
#endif
	{ "quoteleft",            0x2018 },	/* single open quote */
	{ "quotereversed",		  0x201B },	/* new 97/Mar */
	{ "quoteright",           0x2019 },	/* single close quote */
										/* punctuation apostrophe - not 02BC */
	{ "quotesinglbase",       0x201A },
	{ "quotesingle",          0x0027 },
	{ "r",                    0x0072 },
	{ "racute",               0x0155 },
	{ "radical",              0x221A },
#ifdef MATHEXTENSION
	{ "radicalex",			  0xF8E5 },	/* ? Apple corporate use subarea 0x203E */
#endif
	{ "rcaron",               0x0159 },
	{ "rcommaaccent",         0x0157 },
	{ "reflexsubset",         0x2286 },
	{ "reflexsuperset",       0x2287 },
	{ "registered",           0x00AE },
#ifdef CORPORATEUSE
	{ "registersans",         0xF8E8 }, /* 0xF7D8 */	/* Adobe special hack */
	{ "registerserif",        0xF6DA }, /* 0xF7D9 */	/* Adobe special hack */
#endif
	{ "revlogicalnot",		  0x2310 },	/* new 97/Mar */
	{ "rho",                  0x03C1 },
	{ "ring",                 0x02DA },
#ifdef SUPERIOR
	{ "rsuperior",            0xF6F1 }, /* 0xF7F1 */	/* Adobe special hack */
#endif
	{ "rtblock",			  0x2590 },	/* right half solid rectangle */
/* #ifdef CORPORATEUSE */
	{ "rupiah",               0xF6DD }, /* 0xF7DD */	/* Adobe special hack */
/* #endif */
	{ "s",                    0x0073 },
	{ "sacute",               0x015B },
	{ "scaron",               0x0161 },
	{ "scedilla",             0x015F },	/* 98/Oct/22 */
	{ "scedilla_",            0xF6C2 },	/* 97/Nov/24 98/Oct/26 duplicate */
/*	{ "scedilla_",            0x1E9F }, */	/* Adobe special hack removed */
	{ "scircumflex",          0x015D },
	{ "scommaaccent",         0x0219 },	/* 98/Oct/22 0x015F */
	{ "second",               0x2033 },	/* double prime, inches */
	{ "section",              0x00A7 },
	{ "semicolon",            0x003B },
#ifdef HP
	{ "servicemark",          0x2120 },	/* Service Mark NOT Adobe */
#endif
	{ "seven",                0x0037 },
	{ "seveneighths",         0x215E },
	{ "seveninferior",        0x2087 },
#ifdef OLDSTYLE
	{ "sevenoldstyle",        0xF737 }, /* 0xF837 */	/* Adobe special hack */
#endif
	{ "sevensuperior",        0x2077 },
	{ "shade",				  0x2592 },	/* medium shading */ /* OEM indicator */
	{ "sigma",                0x03C3 },
	{ "sigma1",               0x03C2 },		/* sigmafinal */
	{ "similar",              0x223C },
	{ "six",                  0x0036 },
	{ "sixinferior",          0x2086 },
#ifdef OLDSTYLE
	{ "sixoldstyle",          0xF736 }, /* 0xF836 */	/* Adobe special hack */
#endif
	{ "sixsuperior",          0x2076 },
	{ "slash",                0x002F },
	{ "smileface",			  0x263A },	/* open smiling face */
	{ "space",                0x0020 },	/* space */
	{ "space_",               0x00A0 },	/* nbspace * duplicate */
	{ "spade",                0x2660 },	/* solid */
#ifdef SUPERIOR
	{ "ssuperior",            0xF6F2 }, /* 0xF7F2 */	/* Adobe special hack */
#endif
	{ "sterling",             0x00A3 },
	{ "suchthat",             0x220B },
	{ "summation",            0x2211 },
	{ "sun",				  0x263C },	/* compass, eight pointed sun */
	{ "t",                    0x0074 },
	{ "tau",                  0x03C4 },
	{ "tbar",                 0x0167 },
	{ "tcaron",               0x0165 },
	{ "tcommaaccent",         0x0163 },
	{ "tcommaaccent_",        0x021B },		/* 98/Oct/22 duplicate */
	{ "therefore",            0x2234 },
	{ "theta",                0x03B8 },
	{ "theta1",               0x03D1 },		/* Greek theta symbol */
#ifdef HP
	{ "thinspace",            0x2009 },		/* NOT Adobe */
#endif
	{ "thorn",                0x00FE },
	{ "three",                0x0033 },
	{ "threeeighths",         0x215C },
	{ "threeinferior",        0x2083 },
#ifdef OLDSTYLE
	{ "threeoldstyle",        0xF733 }, /* 0xF833 */	/* Adobe special hack */
#endif
	{ "threequarters",        0x00BE },
/* #ifdef CORPORATEUSE */
	{ "threequartersemdash",  0xF6DE }, /* 0xF7DE */	/* Adobe special hack */
/* #endif */
	{ "threesuperior",        0x00B3 },
	{ "tilde",                0x02DC },
	{ "tildecomb",            0x0303 },
	{ "tonos",                0x0384 },
	{ "trademark",            0x2122 },
#ifdef CORPORATEUSE
	{ "trademarksans",        0xF8EA }, /* 0xF7DA */	/* Adobe special hack */
	{ "trademarkserif",       0xF6DB }, /* 0xF7DB */	/* Adobe special hack */
#endif
#ifdef HP
	{ "triagbullet",		  0x2023 },	/* triangle bullet points right */
#endif
	{ "triagdn",			  0x25BC },	/* down solid arrowhead */
#ifdef HP
	{ "triagdnopen",		  0x25BF },	/* down open triangle */
#endif
	{ "triaglf",			  0x25C4 },	/* left solid arrowhead */
#ifdef HP
	{ "triaglfopen",		  0x25C3 },	/* left open triangle */
#endif
	{ "triagrt",			  0x25BA },	/* right solid arrowhead */
#ifdef HP
	{ "triagrtopen",		  0x25B9 },	/* right open triangle */
#endif
	{ "triagup",			  0x25B2 },	/* up solid arrowhead */
#ifdef HP
	{ "triagupopen",		  0x25B5 },	/* up open triangle */
#endif
#ifdef SUPERIOR
	{ "tsuperior",            0xF6F3 }, /* 0xF7F3 */	/* Adobe special hack */
#endif
	{ "two",                  0x0032 },
	{ "twodotenleader",       0x2025 },
	{ "twoinferior",          0x2082 },
#ifdef OLDSTYLE
	{ "twooldstyle",          0xF732 }, /* 0xF832 */	/* Adobe special hack */
#endif
	{ "twosuperior",          0x00B2 },
	{ "twothirds",            0x2154 },
	{ "u",                    0x0075 },
	{ "uacute",               0x00FA },
	{ "ubreve",               0x016D },
	{ "ucircumflex",          0x00FB },
	{ "udieresis",            0x00FC },
	{ "ugrave",               0x00F9 },
	{ "uhorn",                0x01B0 },
	{ "uhungarumlaut",        0x0171 },
	{ "umacron",              0x016B },
	{ "underscore",           0x005F },
	{ "underscoredbl",        0x2017 },		/* double underline */
	{ "union",                0x222A },
	{ "universal",            0x2200 },
	{ "uogonek",              0x0173 },		/* Baltic indicator */
	{ "upblock",              0x2580 },		/* SF600000 top half solid */
	{ "upsilon",              0x03C5 },
	{ "upsilondieresis",      0x03CB },		/* Greek indicator */
	{ "upsilondieresistonos", 0x03B0 },
	{ "upsilontonos",         0x03CD },
	{ "uring",                0x016F },
	{ "utilde",               0x0169 },
	{ "v",                    0x0076 },
	{ "w",                    0x0077 },
	{ "wacute",				  0x1E83 },	/* new 97/Mar */
	{ "wcircumflex",		  0x0175 },	/* new 97/Mar */
	{ "wdieresis",			  0x1E85 },	/* new 97/Mar */
	{ "weierstrass",          0x2118 },
	{ "wgrave",				  0x1E81 },	/* new 97/Mar */
	{ "x",                    0x0078 },
	{ "xi",                   0x03BE },
	{ "y",                    0x0079 },
	{ "yacute",               0x00FD },
	{ "ycircumflex",		  0x0177 },	/* new 97/Mar */
	{ "ydieresis",            0x00FF },
	{ "yen",                  0x00A5 },
	{ "ygrave",				  0x1EF3 },	/* new 97/Mar */
	{ "z",                    0x007A },
	{ "zacute",               0x017A },
	{ "zcaron",               0x017E },
	{ "zdotaccent",           0x017C },
	{ "zero",                 0x0030 },
	{ "zeroinferior",         0x2080 },
#ifdef OLDSTYLE
	{ "zerooldstyle",         0xF730 }, /* 0xF830 */	/* Adobe special hack */
#endif
	{ "zerosuperior",         0x2070 },
	{ "zeta",                 0x03B6 },
};

/*	{ "longst",               0xFB05 }, */		/* s ligatures */

/*	{ "st",                   0xFB06 }, */		/* s ligatures */

/* Some alternate glyph names.  Note that: */
/* `dot' `hungar' `dblacute' `commaaccent' `hacek' `quoteright' `diaeresis' */
/* already taken care of systematically below */

/* NOTE: Greek "tonos" is "acute" */
/* NOTE: Greek "dialytika" is "dieresis" */
/* NOTE: Greek "perispomene" is "tilde" */

/* NOTE: this must be alphabetically ordered for bsearch */

/* static const UNICODE_MAP unicodeMapX[] = */
const UNICODE_MAP unicodeMapX[] =
{
	{ "Alphaacute",         0x0386 },	/* Alphatonos */
	{ "Digamma", 			0x03DC },	/* not in ATM */
	{ "Dslash", 			0x0110 },	/* Dcroat */
	{ "Epsilonacute",       0x0388 },	/* Epsilontonos */
	{ "Etaacute",           0x0389 },	/* Etatonos */
	{ "Germandbls",			0x00DF },	/* germandbls */
	{ "Iotaacute",          0x038A },	/* Iotatonos */
	{ "Koppa",              0x03DE },	/* not in ATM */
	{ "Nabla",              0x2207 },	/* gradient */
	{ "Ohm",				0x2126 },	/* Omega */
	{ "Omegaacute",         0x038F },	/* Omegatonos */
	{ "Pts",				0x20A7 },	/* peseta */
	{ "SS",					0x00DF },	/* germandbls */
	{ "Upsilonacute",	    0x038E },	/* Upsilontonos */
	{ "afii00941",          0x20A4 },	/* lira */ /* afii08941 ? */
	{ "almostequal",		0x2248 },	/* approxequal */
	{ "alphaacute",         0x03AC },	/* alphatonos */
	{ "bom",				0xFEFF },	/* byte order mark */
	{ "bulletmath",			0x2219 },	/* periodcentered_ */
	{ "careof",             0x2105 },	/* afii61248 */
	{ "cwm",				0xFEFF },	/* zero-width no-break space */
	{ "dash",				0x00AD },	/* hyphen */
	{ "dieresisacute",      0x0385 },	/* dieresistonos */
	{ "dmacron",			0x0111 },	/* dcroat */
/*	{ "dslash" ,			0x0111 }, */	/* dcroat */
	{ "epsilonacute",       0x03AD },	/* epsilontonos */
	{ "etaacute",           0x03AE },	/* etatonos */
	{ "euro",               0x20AC },	/* Euro */
	{ "fscript",            0x0192 },	/* florin */
	{ "questiongreek",		0x037E },	/* semicolon, not in ATM */
	{ "iotadieresisacute",  0x0390 },	/* iotadieresistonos */
	{ "iotaacute",          0x03AF },	/* iotatonos */
	{ "iotasub",    		0x037A },	/* iota under accent */
	{ "iotasubscript",		0x037A },	/* iota under accent */
	{ "increment",			0x2206 },	/* Delta */
	{ "kra",                0x0138 },	/* kgreenlandic */
	{ "liter",              0x2113 },	/* afii61289 */
	{ "litre",              0x2113 },	/* afii61289 */
	{ "lscript",            0x2113 },	/* afii61289 */
	{ "micro",				0x00B5 },	/* mu */
/*	{ "middot",		        0x00B7 }, */	/* periodcentered */
	{ "middot",		        0x2219 },	/* periodcentered */
	{ "nabla",              0x2207 },	/* gradient */
	{ "nbspace",			0x00A0 },	/* space */
	{ "numero",             0x2116 },	/* afii61352 */
	{ "ohm",				0x2126 },	/* Omega */
	{ "omegaacute",         0x03CE },	/* omegatonos */
	{ "omicronacute",       0x03CC },	/* omicrontonos */
	{ "overscore",			0x00AF },	/* macron */
	{ "overscore_",			0x203E },	/* overscore */
	{ "perzero",			0x2030 },	/* perthousand */
	{ "pi1",                0x03D6 },	/* Adobe's omega1 ? */
	{ "quotedash",			0x2015 },	/* afii00208 */
	{ "rightangle",			0x221F },	/* orthogonal */
	{ "sfthyphen",			0x00AD },	/* hyphen */
	{ "sigmafinal",         0x03C2 },	/* sigma1 */
	{ "slong",              0x017F },	/* longs */
	{ "upsilondieresisacute", 0x03B0 },	/* upsilondieresistonos */
	{ "upsilonacute",       0x03CD },	/* upsilontonos */
	{ "visiblespace",		0x0020 },	/* space */
};

/* Zapf Dingbats uses F8D7-F8DF, F8E0-F8E4 */
/* Symbol font uses   F8E5-F8EF, F8F0-F8FE */

#define notdefUID 0xFFFF

/* The font driver uses a set of heuristics.  For each charset, the font */
/* driver checks to see if the font contains a specific glyphname. If the */
/* font contains the glyphname, the font driver reports that the font */
/* supports the charset. */

/* { "ncaron",           0x0148 }, */	/* 1250 Latin 2 / CE */
/* { "afii10071",        0x0451 }, */	/* 1251 io */ /* Slavic / Cyrillic*/
/* { "ecircumflex",      0x00EA }, */	/* 1252 Latin 1 ANSI WE */
/* { "upsilondieresis",  0x03CB }, */	/* 1253 Greek indicator */
/* { "Idotaccent",       0x0130 }, */	/* 1254 Turkish indicator */
/* { "afii57664",        0x05D0 }, */	/* 1255 alef */ /* Hebrew indicator */ 
/* { "afii57410",        0x0622 }, */	/* 1256 Arabic indicator */
/* { "uogonek",          0x0173 }, */	/* 1257 Baltic Rim indicator */
/* { "dong",             0x20AB }, */   /* 1258 Vietnamese */
/* { "Delta_",           0x2206 }, */	/* Macintosh indicator */
/* { "shade",			 0x2592 }, */	/* OEM / Linedraw indicator */

/* first arg is the search argument second argument the table entry */
int compareglyphs (const void *ptr1, const void *ptr2) {
	char *name = (char *) ptr1;
	UNICODE_MAP *ptr = (UNICODE_MAP *) ptr2;
	return strcmp(name, ptr->glyphname);
}

int sizeofUID(void) {		/* so can get this in winfonts.c */
	return sizeof(unicodeMap) / sizeof(unicodeMap[1]);
}

/* Finds UID given glyph name based on ATM glyph table */
/* Also takes account of alternate names in encoding vector */
/* But cannot deal with alternate names in the font itself! */

int findUID (char *name) {		/* this version uses bsearch */
	UNICODE_MAP *ptr;
//	char buffer[MAXCHARNAME+10];
	char buffer[MAXFILENAME];
	char *s;
	int n;
	int nlen = sizeof(unicodeMap) / sizeof(unicodeMap[1]);
	int nlenX = sizeof(unicodeMapX) / sizeof(unicodeMapX[1]);
	if (*name == '\0') return notdefUID;		/* empty string */
	ptr = bsearch(name, unicodeMap, nlen, sizeof(unicodeMap[1]), compareglyphs);
	if (ptr != NULL) {
		n = ptr->UID;
/*		deal with stupid auto TT => T1 conversion using old UNICODE values */
		if (bOldUnicode && n >= 0xFB00 && n <= 0xFB10) {
			n = (n - 0xFB00) + 0xF000;
		}
		return n;
	}
#ifdef DEBUGENCODING
	if (bDebug > 1) {
		sprintf(debugstr, "UNKNOWN %s\n", name);
		OutputDebugString(debugstr);
	}
#endif
/*	Try table of alternate names next */
	ptr = bsearch(name, unicodeMapX, nlenX, sizeof(unicodeMapX[1]), compareglyphs);
	if (ptr != NULL) return ptr->UID;
#ifdef DEBUGENCODING
	if (bDebug > 1) {
		sprintf(debugstr, "UNKNOWN %s\n", name);
		OutputDebugString(debugstr);
	}
#endif
	if ((s = strstr(name, "dot")) != NULL &&
		strcmp(s, "dot") == 0) {
		strcpy (buffer, name);
		strcat (buffer, "accent");
		ptr = bsearch(buffer, unicodeMap, nlen, sizeof(unicodeMap[1]), compareglyphs);		
	}
	if (ptr != NULL) return ptr->UID;
	if ((s = strstr(name, "hungar")) != NULL &&
		strcmp(s, "hungar") == 0) {
		strcpy (buffer, name);
		strcat (buffer, "umlaut");
		ptr = bsearch(buffer, unicodeMap, nlen, sizeof(unicodeMap[1]), compareglyphs);		
	}
	if (ptr != NULL) return ptr->UID;
	if ((s = strstr(name, "dblacute")) != NULL &&
		strcmp(s, "dblacute") == 0) {
		strcpy (buffer, name);
		strcpy (buffer + (s - name), "hungarumlaut");
		ptr = bsearch(buffer, unicodeMap, nlen, sizeof(unicodeMap[1]), compareglyphs);		
	}
	if (ptr != NULL) return ptr->UID;
	if ((s = strstr(name, "hacek")) != NULL &&
		strcmp(s, "hacek") == 0) {
		strcpy (buffer, name);
		strcpy (buffer + (s - name), "caron");
		ptr = bsearch(buffer, unicodeMap, nlen, sizeof(unicodeMap[1]), compareglyphs);		
	}
	if (ptr != NULL) return ptr->UID;
	if ((s = strstr(name, "quoteright")) != NULL &&
		strcmp(s, "quoteright") == 0) {
		strcpy (buffer, name);
		strcpy (buffer + (s - name), "caron");
		ptr = bsearch(buffer, unicodeMap, nlen, sizeof(unicodeMap[1]), compareglyphs);		
	}
	if (ptr != NULL) return ptr->UID;
	if ((s = strstr(name, "cedilla")) != NULL &&
		strcmp(s, "cedilla") == 0) {
		strcpy (buffer, name);
		strcpy (buffer + (s - name), "commaaccent");
		ptr = bsearch(buffer, unicodeMap, nlen, sizeof(unicodeMap[1]), compareglyphs);		
	}
	if (ptr != NULL) return ptr->UID;
	if ((s = strstr(name, "diaeresis")) != NULL) {
		strcpy (buffer, name);		/* try and turn into "dieresis" */
		strcpy (buffer + (s - name)+2, buffer  + (s - name)+3);
		ptr = bsearch(buffer, unicodeMap, nlen, sizeof(unicodeMap[1]), compareglyphs);		
	}
	if (ptr != NULL) return ptr->UID;

	return notdefUID;
}

// int makeUIDtable(LPSTR lpEncoding)
int makeUIDtable (LPSTR *lpVector) {
	int k, count=0;
	int UID;
	LPSTR s;

	if (bDebug > 1) OutputDebugString("makeUIDTable\n");
//	s = lpVector;
	for (k = 0; k < MAXCHRS; k++) {
/*		s = lpVector + k * MAXCHARNAME; */
/*		encodeUID[k] = (WCHAR) findUID(s); */
/*		if (encodeUID[k] == notdefUID) */
		s = lpVector[k];
		if (s == NULL) {
			encodeUID[k] = notdefUID;
			continue;
		}
		if (strncmp(s, "uni", 3) == 0) {		/* official ATM 4.0 */
			if (sscanf(s+3, "%4X", &UID) == 1)
				encodeUID[k] = (WCHAR) UID;
			else encodeUID[k] = notdefUID;
		}
		else if (strncmp(s, "U+", 2) == 0) {	/* UNICODE UID */
			if (sscanf(s+2, "%4X", &UID) == 1)
				encodeUID[k] = (WCHAR) UID;
			else encodeUID[k] = notdefUID;
		}
/*		also UID<CODE>, UV<CODE> ? */
		else encodeUID[k] = (WCHAR) findUID(s);
		if (encodeUID[k] != notdefUID) count++;
//		s += MAXCHARNAME;
	}
	return count;				/* how many we found */
}
#endif /* end of if MAKEUNICODE */
#endif /* end of if USEUNICODE */

/*****************************************************************************/

BOOL CALLBACK _export CommandDlg (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	WORD id;
	int n;
	char title[64];

	switch (message) { 

		case WM_COMMAND:
/*			id = GET_WM_COMMAND_ID(wParam, lParam); */
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

			case IDOK:
				n = (int) GetDlgItemText(hDlg, IPC_EDIT, str, sizeof(str));
				EndDialog(hDlg, n);	/* can be zero */
				return (TRUE);

			case IDM_REVERT:
				n = (int) GetDlgItemText(hDlg, IPC_OLDTEXT, str, sizeof(str));
				SetDlgItemText(hDlg, IPC_EDIT, str);
				(void) SendDlgItemMessage(hDlg,		/* dialog handle */
								  IPC_EDIT,			/* where to send message */
								  EM_SETSEL,		/* select characters	*/
								  0, -1
/*								  0, MAKELONG(0, 0x7fff) */
				 );						/* select entire contents	*/
				(void) SetFocus(GetDlgItem(hDlg, IPC_EDIT));
				return (TRUE);

			case IDCANCEL:
				EndDialog(hDlg, -1);
				return (TRUE);

			default:
				break; /* ? */
			}
			break;

/* end of WM_COMMAND case */

		case WM_INITDIALOG:						/* message: initialize	*/
/*			(void) SendDlgItemMessage(hDlg, IPC_EDIT, EM_LIMITTEXT,	5, 0L); */
			strcpy(title, str);
			strcat(title, " ");
			strcat(title, "Command Line");
			SetWindowText(hDlg, title);
			strcpy(title, str);
			GetPrivateProfileString(achPr, title, "",
									str, sizeof(str), achFile);
			SetDlgItemText(hDlg, IPC_OLDTEXT, str);
			SetDlgItemText(hDlg, IPC_EDIT, str);
/*			(void) SendDlgItemMessage(hDlg,
				IPC_OLDTEXT,			
				EM_SETREADONLY,
				TRUE, 0L); */
			(void) SendDlgItemMessage(hDlg,		/* dialog handle */
				IPC_EDIT,						/* where to send message */
				EM_SETSEL,						/* select characters	*/
				0, -1
/*				0, MAKELONG(0, 0x7fff) */
			);									/* select entire contents	*/
			(void) SetFocus(GetDlgItem(hDlg, IPC_EDIT));
			return (FALSE); /* Indicates we set the focus to a control */

			default:
				return (FALSE); /* ? */

	}
	return FALSE;					/* message not processed */
}	/* lParam unreferenced */

int	editcommandflags (HWND hWnd, WORD id) {
	int flag;
	char *application;

	switch(id) {
		case IDM_TEXFLAGS:
			application="TeX";
			break;
		case IDM_DVIFLAGS:
			application="DVIPSONE";
			break;
		case IDM_DVIDISFLAGS:
			application="DVIPSONE/Distiller";
			break;
		case IDM_TFMFLAGS:
			application="AFMtoTFM";
			break;
		default:
			application="UNKOWN";	/* keep compiler happy */
			break;
	}
	strcpy(str, application);
/*	GetPrivateProfileString(achPr, application, "", str, sizeof(str), achFile);*/
//	flag = DialogBox(hInst, "CommandLine", hWnd, (DLGPROC) CommandDlg);
	flag = DialogBox(hInst, "CommandLine", hWnd, CommandDlg);
	if (flag < 0) return -1;	/* failed or user cancelled */
/*	flag can be zero - indicating empty string */
	if (*str != '\0')
		(void) WritePrivateProfileString(achPr, application, str, achFile);
	else
		(void) WritePrivateProfileString(achPr, application, NULL, achFile);
	return flag;
}

BOOL CALLBACK _export EnvironmentDlg (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	WORD id;
	int n;
	char title[64];

	switch (message) { 

		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);

			switch (id) {

				case IDOK:
					n = (int) GetDlgItemText(hDlg, IPC_EDIT, str, sizeof(str));
					EndDialog(hDlg, n);	/* can be zero */
					return (TRUE);

				case IDM_REVERT:
					n = (int) GetDlgItemText(hDlg, IPC_OLDTEXT, str, sizeof(str));
					SetDlgItemText(hDlg, IPC_EDIT, str);
					(void) SendDlgItemMessage(hDlg,		/* dialog handle */
							IPC_EDIT,			/* where to send message */
							EM_SETSEL,		/* select characters	*/
							0, -1
/*							0, MAKELONG(0, 0x7fff) */
					 );						/* select entire contents	*/
					(void) SetFocus(GetDlgItem(hDlg, IPC_EDIT));
					return(TRUE);
					
				case IDCANCEL:
					EndDialog(hDlg, -1);
					return (TRUE);

				default:
					break; /* ? */
			}
			break;

/* end of WM_COMMAND case */

		case WM_INITDIALOG:						/* message: initialize	*/
/*			(void) SendDlgItemMessage(hDlg, IPC_EDIT, EM_LIMITTEXT,	5, 0L); */
			strcpy(title, "Environment Variable");
			strcat(title, " ");
			strcat(title, str);
			SetWindowText(hDlg, title);
			strcpy(title, str);
			GetPrivateProfileString(achEnv, title, "",
									str, sizeof(str), achFile);
			SetDlgItemText(hDlg, IPC_OLDTEXT, str);
			SetDlgItemText(hDlg, IPC_EDIT, str);
/*			(void) SendDlgItemMessage(hDlg,	
									  IPC_OLDTEXT,
									  EM_SETREADONLY,
									  TRUE, 0L); */
			(void) SendDlgItemMessage(hDlg,		/* dialog handle */
									  IPC_EDIT,			/* where to send message */
									  EM_SETSEL,		/* select characters	*/
									  0, -1
/*									  0, MAKELONG(0, 0x7fff) */
									 );									/* select entire contents	*/
			(void) SetFocus(GetDlgItem(hDlg, IPC_EDIT));	
			return (FALSE); /* Indicates we set the focus to a control */

		default:
			return (FALSE); /* ? */

	}
	return FALSE;					/* message not processed */
}	/* lParam unreferenced */

BOOL CALLBACK _export NewVariableDlg (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	WORD id;
	int n, m;
	char *s;
	char title[64];

	switch (message) { 

		case WM_COMMAND:
/*			id = GET_WM_COMMAND_ID(wParam, lParam); */
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

				case IDOK:
					n = (int) GetDlgItemText(hDlg, IPC_VARIABLE, str, sizeof(str));
					s = str+n;
					*s++ = '=';
					m = (int) GetDlgItemText(hDlg, IPC_VALUE, s, sizeof(str)-n-1);
					EndDialog(hDlg, n+m);	/* can be zero */
					return (TRUE);

				case IDCANCEL:
					EndDialog(hDlg, -1);
					return (TRUE);

				default:
					break; /* ? */
			}
			break;

/* end of WM_COMMAND case */

		case WM_INITDIALOG:						/* message: initialize	*/
/*			(void) SendDlgItemMessage(hDlg, IPC_EDIT, EM_LIMITTEXT,	5, 0L); */
			strcpy(title, "New ");
			strcat(title, "Environment Variable");
			SetWindowText(hDlg, title);
			(void) SetFocus(GetDlgItem(hDlg, IPC_VARIABLE));	
			return (FALSE); /* Indicates we set the focus to a control */

		default:
			return (FALSE); /* ? */

	}
	return FALSE;					/* message not processed */
}	/* lParam unreferenced */

int addenvironment (void) {				/* called from editenvironment */
	char *s;
	int flag;
	
//	flag = DialogBox(hInst, "NewVariable", hwnd, (DLGPROC) NewVariableDlg);
	flag = DialogBox(hInst, "NewVariable", hwnd, NewVariableDlg);
	if (flag < 0) return -1;	/* failed or user cancelled */
	if (*str != '\0') {			/* assume str of form key=value */
		if ((s = strchr(str, '=')) != NULL) {
			*s++ = '\0';
			(void) WritePrivateProfileString(achEnv, str, s, achFile);
		}
	}
	return flag;
}

int	editenvironment (int id, int shift, int editflag) {	/* 98/Aug/28 */
/*	char buffer[BUFLEN]; */			/* temporary buffer */
	char key[MAXKEY];
	int n;
	int flag;

	if (id == 0) return addenvironment();
	n = GetMenuString(hEnvMenu, id, key, sizeof(key), MF_BYPOSITION);
/*	n = GetPrivateProfileString(achEnv, str, "", buffer, sizeof(buffer), achFile); */
	strcpy(str, key);
//	flag = DialogBox(hInst, "Environment", hwnd, (DLGPROC) EnvironmentDlg);
	flag = DialogBox(hInst, "Environment", hwnd, EnvironmentDlg);
	if (flag < 0) return -1;	/* failed or user cancelled */
/*	flag can be zero - indicating empty string */ /* remove var in that case */
	(void) WritePrivateProfileString(achEnv, key,
						 (*str != '\0') ? str : NULL, achFile);
/*	if (*str != '\0')
		(void) WritePrivateProfileString(achEnv, key, str, achFile);
	else (void) WritePrivateProfileString(achEnv, key, NULL, achFile); */
	return flag;
}

int CheckEncodingMenu (HWND hWnd, WORD id) {	/* 97/Apr/3 */
	char *encode;
	HMENU hMenu;

	switch(id) {
		case IDM_TEXNANSI:
			encode="texnansi";
			break;
		case IDM_TEX256:
			encode="tex256";
			break;
		case IDM_ANSINEW:
			encode="ansinew";
			break;
		case IDM_STANDARD:
			encode="standard";
			break;
		case IDM_TEXTEXT:
			encode="textext";
			break;
		case IDM_CUSTOMENCODING:
/*			encode="custom"; */
			if (szCustom != NULL) encode=szCustom;
			else encode="unknown";
/*			if CUSTOM env variable set use that 1997/Dec/22 */
			break;
		default:
			encode="unknown";	/* keep compiler happy */
			break;
	}
/*	check if it changed - if not, do nothing */
	if (szReencodingName != NULL &&
		strcmp(encode, szReencodingName) == 0) return 0;

	(void) WritePrivateProfileString(achEnv, "ENCODING", encode, achFile);

/*	need to change checked menu item */
	hMenu = GetMenu(hWnd);
	(void) CheckMenuItem (hMenu, IDM_TEXNANSI, MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_TEX256, MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_ANSINEW, MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_STANDARD, MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_TEXTEXT, MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_CUSTOMENCODING, MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, id, MF_CHECKED);
	return 1;
}

/*****************************************************************************/

#ifdef IGNORED
char *texnansi[256] = {
"", "", "", "",
"fraction", "dotaccent", "hungarumlaut", "ogonek", "fl", "fraction", "",
"ff", "fi", "fl", "ffi", "ffl", "dotlessi", "dotlessj", "grave", "acute",
"caron", "breve", "macron", "ring", "cedilla", "germandbls", "ae", "oe",
"oslash", "AE", "OE", "Oslash",
"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
"ampersand", "quoteright", "parenleft", "parenright", "asterisk", "plus",
"comma", "hyphen", "period", "slash", "zero", "one", "two", "three", "four",
"five", "six", "seven", "eight", "nine", "colon", "semicolon", "less",
"equal", "greater", "question", 
"at", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", 
"bracketleft", "backslash", "bracketright", "circumflex", "underscore", 
"quoteleft", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", 
"braceleft", "bar", "braceright", "tilde", "dieresis", 
"Lslash", "quotesingle", "quotesinglbase", "florin", "quotedblbase",
"ellipsis", "dagger", "daggerdbl", "circumflex", "perthousand", "Scaron",
"guilsinglleft", "OE", "Zcaron", "asciicircum", "minus", "lslash", 
"quoteleft", "quoteright", "quotedblleft", "quotedblright", "bullet",
"endash", "emdash", "tilde", "trademark", "scaron", "guilsinglright", "oe", 
"zcaron", "asciitilde", "Ydieresis", 
"space", "exclamdown", "cent", "sterling", "currency", "yen", "brokenbar",
"section", "dieresis", "copyright", "ordfeminine", "guillemotleft",
"logicalnot", "hyphen", "registered", "macron", "degree", "plusminus",
"twosuperior", "threesuperior", "acute", "mu", "paragraph", "periodcentered",
"cedilla", "onesuperior", "ordmasculine", "guillemotright", "onequarter",
"onehalf", "threequarters", "questiondown", "Agrave", "Aacute",
"Acircumflex", "Atilde", "Adieresis", "Aring", "AE", "Ccedilla", "Egrave",
"Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex",
"Idieresis", "Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde",
"Odieresis", "multiply", "Oslash", "Ugrave", "Uacute", "Ucircumflex",
"Udieresis", "Yacute", "Thorn", "germandbls", "agrave", "aacute",
"acircumflex", "atilde", "adieresis", "aring", "ae", "ccedilla", "egrave",
"eacute", "ecircumflex", "edieresis", "igrave", "iacute", "icircumflex",
"idieresis", "eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde",
"odieresis", "divide", "oslash", "ugrave", "uacute", "ucircumflex",
"udieresis", "yacute", "thorn", "ydieresis" };
#endif

/*****************************************************************************/

#ifdef IGNORED
/*	Try and link to ATM in Windows 3.1 --- returns -1 on failure */
int GetATMProcAddresses32(HMODULE hATMNT) {
/*	Ask for addresses of functions used */
/*	NewATMIsUp = GetProcAddress  (hATMNT, "ATMIsUp@0"); */
	NewATMIsUp = (BOOL (WINAPI *) (VOID))
				GetProcShrouded (hATMNT, "Jt", "VqA1"); 
/*	NewATMGetVersion = GetProcAddress (hATMNT, "ATMGetVersion@0"); */
	NewATMGetVersion = (WORD (WINAPI *) (VOID))
			   GetProcShrouded (hATMNT, "Hfu", "WfstjpoA1"); 
/*	NewATMGetBuildStr = GetProcAddress (hATMNT, "ATMGetBuildStr@4"); */
	NewATMGetBuildStr = (WORD (WINAPI *) (LPSTR))
			   GetProcShrouded (hATMNT, "Hfu", "CvjmeTusA5"); 
/*	NewATMSetFlags = GetProcAddress (hATMNT, "ATMSetFlags@8"); */
	NewATMSetFlags = (WORD (WINAPI *) (WORD, WORD))
			   GetProcShrouded (hATMNT, "Tfu", "GmbhtA9"); 
/*	NewATMFontSelected = GetProcAddress  (hATMNT, "ATMFontSelected@4"); */
	NewATMFontSelected = (BOOL (WINAPI *) (HDC))
				GetProcShrouded (hATMNT, "Gpou", "TfmfdufeA5"); 
/*	NewATMMarkDC = GetProcAddress  (hATMNT, "ATMMarkDC"); */	/* 96/June/4 */
/*	NewATMMarkDC = (int (WINAPI *) (int, HDC, LPWORD, LPWORD))
				GetProcShrouded (hATMNT, "Nbsl", "ED"); */
/*	NewATMSelectEncoding = GetProcAddress  (hATMNT, "ATMSelectEncoding@12"); */
	NewATMSelectEncoding = (int (WINAPI *) (int, HDC, LPATMDATA))
				GetProcShrouded (hATMNT, "Tfmfdu", "FodpejohA23"); 
/*	NewATMGetFontBBox = GetProcAddress (hATMNT, "ATMGetFontBBox@8"); */
	NewATMGetFontBBox = (int (WINAPI *) (HDC, LPATMBBox))
				GetProcShrouded (hATMNT, "HfuGpou", "CCpyA9");
/*	NewATMGetGlyphList = GetProcAddress (hATMNT, "ATMGetGlyphList@16"); */
	NewATMGetGlyphList = (int (WINAPI *) (int, HDC, FARPROC, LPSTR))
				GetProcShrouded (hATMNT, "HfuHmzqi", "MjtuA27");
/*	NewATMGetPostScriptName = GetProcAddress(hATMNT, "ATMGetPostScriptName@12"); */
	NewATMGetPostScriptName = (int (WINAPI *) (int, LPSTR, LPSTR))
				GetProcShrouded(hATMNT, "HfuQptu", "TdsjquObnfA23");
/*	NewATMGetOutline = GetProcAddress(hATMNT, "ATMGetOutline@32"); */
	NewATMGetOutline = (int (WINAPI *) (HDC, char, LPATMFixedMatrix,
				  FARPROC, FARPROC, FARPROC, FARPROC, LPSTR))
					  GetProcShrouded(hATMNT, "Hfu", "PvumjofA43");
/*	NewATMFontAvailable = GetProcAddress(hATMNT, "ATMFontAvailable@24"); */
	NewATMFontAvailable = (int (WINAPI *) (LPSTR, int,
					 BYTE, BYTE, BYTE, BOOL FAR *))
				GetProcShrouded(hATMNT, "Gpou", "BwbjmbcmfA35"); 
/*	NewATMGetFontInfo = GetProcAddress(hATMNT, "ATMGetFontInfo@24"); */
	NewATMGetFontInfo = (int  (WINAPI *) (WORD, HDC, WORD, LPSTR,
/*								 LPSTR, LPWORD) */
								 LPATMInstanceInfo, LPWORD))
				GetProcShrouded(hATMNT, "HfuGpou", "JogpA35");
	if (NewATMIsUp == NULL ||
			NewATMGetVersion == NULL ||
			NewATMGetBuildStr == NULL ||
			NewATMSetFlags == NULL ||
				NewATMFontSelected == NULL ||
/*					NewATMMarkDC == NULL || */
						NewATMSelectEncoding == NULL ||
							NewATMGetFontBBox == NULL ||
								NewATMGetGlyphList == NULL ||
									NewATMGetPostScriptName == NULL ||
										NewATMGetOutline == NULL ||
									NewATMGetFontInfo == NULL ||
											NewATMFontAvailable == NULL
										) {
/*		if (bDebug > 1) OutputDebugString("Can't find Proc Addresses\n"); */
		if (bDebug) winerror("Can't find Proc Addresses");
		else WriteError("Can't find Proc Addresses");
		if (bDebug > 1) {
			sprintf(debugstr, "%d %d %d %d %d %d %d %d %d %d %d %d \n",
					(NewATMIsUp != NULL),
					(NewATMGetVersion != NULL),
					(NewATMGetBuildStr != NULL),
					(NewATMSetFlags != NULL),
					(NewATMFontSelected != NULL),
					(NewATMSelectEncoding != NULL),
					(NewATMGetFontBBox != NULL),
					(NewATMGetGlyphList != NULL),
					(NewATMGetPostScriptName != NULL),
					(NewATMGetOutline != NULL),
					(NewATMGetFontInfo != NULL), 
					(NewATMFontAvailable != NULL));
					OutputDebugString(debugstr);
		}
/*		FreeLibrary ((HINSTANCE) hATMNT); */
/*		hATMNT = (HANDLE) 31; */
		return -1;
	}
/*	NewATMSelectObject = GetProcAddress (hATMNT, "ATMSelectObject@16"); */
	NewATMSelectObject = (int (WINAPI *) (HDC, HFONT, WORD, HFONT FAR *))
						GetProcShrouded (hATMNT, "Tfmfdu", "PckfduA27"); 
	if (NewATMSelectObject == NULL) {
/*		if (bDebug > 1) OutputDebugString("Can't find Proc Addresses\n");*/
		if (bDebug) winerror("Can't find Proc Addresses");
l		else WriteError("Can't find Proc Addresses");
		return -1;
	}
	return 0;	/* success */
}
#endif

/* New code to copy text to clipboard 1997/July/12 */

HGLOBAL CopyStringToHGlobal(LPTSTR psz) {
	HGLOBAL    hMem;
	LPTSTR     pszDst;

	hMem=GlobalAlloc(GHND, (DWORD)(lstrlen(psz)+1));
	if (hMem != NULL) {
		pszDst=GlobalLock(hMem);
		lstrcpy(pszDst, psz);
		GlobalUnlock(hMem);
	}
	return hMem;
}

int PutStringOnClipboard(HWND hWnd, LPSTR psz) {
	HGLOBAL    hMem;

	hMem=CopyStringToHGlobal(psz);

	if (hMem != NULL) {
		if (OpenClipboard(hWnd)) {
			(void) EmptyClipboard();			/* 97/Jul/24 */
			(void) SetClipboardData(CF_TEXT, hMem);
			(void) CloseClipboard();
			return 0;			/* success */
		}
		else {
			GlobalFree(hMem);   /* We must clean up. */
			return -1;			/* failed */
		}
	}
	else return -1;				/* failed */
}

