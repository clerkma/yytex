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

/* #include <stdlib.h> */	/* for _searchenv */

/* dviwindo.h not really needed - just for hdrstop */

#include <dos.h>

#pragma warning(disable:4100)	// unreferenced formal parameters

#define DEBUGREGISTRY

#define DEBUGINI

/* #define DEBUGFONTSEARCH */

/* #define DEBUGCOLORSTACK */

/* #define DEBUGATM */

/* #define DEBUGSPECIAL */

#define ALLOWCOLOR

#define TTF_DONE 1			/* 98/Nov/20 */
#define PFM_DONE 2			/* 98/Nov/20 */

/**********************************************************************
*
* DVI analyzer to extract font usage and font names and so on
*
* Copyright (C) 1991, 1992, 1999 Y&Y. All Rights Reserved. 
*
* DO NOT COPY OR DISTRIBUTE!
*
* part of DVIWINDO
*
**********************************************************************/

COLORREF FAR **lpColor;			/* FAR pointer to color table for current file */

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** *** */ 

/* Define a structure used to hold info about an app we */
/* start with ExecApp so that this info can be used again */
/* later to test if the app is still running */

// typedef struct _EXECAPPINFO {
//    HINSTANCE hInstance;            /* The instance value */
//    HWND hWnd;                      /* The main window handle */
//    HTASK hTask;                    /* The task handle */
// } EXECAPPINFO, FAR *LPEXECAPPINFO;

// EXECAPPINFO TaskInfo;				/* place for Task info */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define DEBUGFLAG */

BOOL bTryEnum=0;			/* This may be too darn slow 95/July/8 */

#define FONTMAP

#ifdef FONTMAP

typedef struct map_element_struct {		/* list element key . value pair */
	char *key;
	char *value;
	struct map_element_struct *next;
} map_element_type;

typedef map_element_type **map_type; 

map_type fontmap = NULL;	/* static - keep around once set 94/Nov/20 */

#endif

char str[BUFLEN];		/* 256 (or more) temporary for random strings */

// char debugstr[BUFLEN];	/* 256 (or more) temporary for debug strings */
char debugstr[DEBUGLEN];	/* 256 (or more) temporary for debug strings */

int	pageno;			/* current pagenumber - normally zero based */

/* int pagenumber; */	/* count of pages actually seen NOT USED ? */

int textures;		/* non-zero if textures file */

int finish;			/* non-zero when seen last eop - or after error */

long texlength;		/* length of next page in textures files */

long dvistart;		/* where DVI file bytes starts */

/* long nspecial;	*/	/* byte count of special */

int maxpagenumber;	/* number of pages in file */
long postposition;	/* position of post in file */

int errlevel;		/* number of errors encountered */

int reverseflag = -1;	/* non-zero => try and read end of file instead */
						/* for speed prescan - but can't build page table ? */

/* int pagetableflag = -1; */	/* construct page table first */

int skiptoend = 0;		/* non-zero => means skip to end of file first bop */

int dvi_f;	/* current font - changed by fnt and fnt_num - reset by bop */
int fnt;	/* fnt =  finx[dvi_f] */

/* c0 ... c9  - \count0 ... \count9 in TeX */

long counter[TEXCOUNTERS];		/* TeX's counters - after bop */

long ps_previous;	/* pointer to previous bop in this file */
long ps_current;	/* pointer to bop of current page */
long ps_next; 		/* pointer to next bop in this file */ /* NA */

int fnext;					/* next font slot to use (total number at end) */

/* Implicitly assuming MAXFONTS < 256 ? */

// int finx[MAXFONTNUMBERS];	/* indeces into next few */
short finx[MAXFONTNUMBERS];		/* indeces into next few */

/* maybe get global memory for the following ? 72 * 13 = 936 bytes */

char *fontname[MAXFONTS];		/* TeX TFM font name */

char *subfontname[MAXFONTS];	/* corresponding Windows Face Name */

/* int fontsubflag[MAXFONTS]; */	/* non-negative if substitute font used */
									/* number indicates base font */ /* NA */

HFONT windowfont[MAXFONTS];		/* handles to scaled fonts */

int fontdescent[MAXFONTS];		/* drop below baseline */
int fontascent[MAXFONTS];		/* rise above baseline */

char fontbold[MAXFONTS];			/* info from ATM.INI or WIN.INI */
char fontitalic[MAXFONTS];			/* info from ATM.INI or WIN.INI */
char fontttf[MAXFONTS];				/* info from ATM.INI or WIN.INI */
char texfont[MAXFONTS];				/* non-zero if font is TeX font */
char ansifont[MAXFONTS];			/* non-zero for text font */

char fontvalid[MAXFONTS];			/* non-zero if handle valid */
char metricsvalid[MAXFONTS];		/* non-zero if metrics valid */
char fontexists[MAXFONTS];			/* non-zero if font found by ATM */
char fontwarned[MAXFONTS];			/* non-zero => absence noted */

unsigned long fc[MAXFONTS];		/* checksum  TFM file (useless ?) */

unsigned long fs[MAXFONTS];		/* at size */

WORD basefont[MAXFONTS];		/* font of same name and style */

/* unsigned long fd[MAXFONTS];	*/	/* design size */ /* NOT ACCESSED */

/* font k is to be used at mag * s / (1000 * d) times its normal size */

/* int fonthit[MAXFONTS]; */	/* which fonts have been seen */ /* used ? */

/* char fontchar[MAXFONTS][MAXCHRS]; */	/* which characters seen */

/* char *currentfont; */						/* pointer to current font */

/* int maxstack; */			/* max stack depth allowed on printer */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char buffer[BUFFERLEN];		/* 512 buffer for low level input routines */

char *bufptr;				/* pointer to next byte to take out of buffer */

int buflen;					/* bytes still left in buffer */

int ungotten;				/* result of unwingetc - or -1 */

long filepos;				/* position of byte in input file */

char *modname = "WINPSLOG";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* put up error message in box - with byte count if relevant */

static void winerror(char *mss) {
	HWND hFocus;
	char errmess[MAXMESSAGE];

	if (strlen(mss) > (MAXMESSAGE - 32)) mss = "Error Message too long!";
	if (filepos > 0) {
		sprintf(errmess, "%s at byte %ld", mss, filepos-1);
/*			(LPSTR) mss, filepos-1); */
	} 
	else strcpy(errmess, mss);

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox (hFocus, errmess, modname, MB_ICONSTOP | MB_OK);
}

static int wincancel(char *buff) {				/* 1993/July/15 */
	HWND hFocus;
	int flag;

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, buff, modname, MB_ICONSTOP | MB_OKCANCEL);
	if (flag == IDCANCEL) return -1;
	else return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void errcount(void) {	/* called each time `survivable' error encountered */
	finish = -1;		/*  stop as soon as feasable ! */
	if (++errlevel > MAXERRORS) {
		winerror("Too many errors!");
/*		cleanup();	giveup(1); */
	}
}

/* `graceful' exit with meaningful error message */
/*	Need to clean up and free up resources ? This needs more work ? 
void giveup(int code) {
	winerror("FATAL ERROR");
	PostQuitMessage(code);
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* assumes only one file open at a time */

/* versions of getc and ungetc using low-level C input/output */

static void unwingetc (int c, HFILE input) { /* ignores `input' */
	if (ungotten >= 0) {
		winerror("Repeated unwingetc");
		errcount();
	}
	else filepos--;
	ungotten = c;
}

static int replenish (HFILE input, int bufferlen) {
/*	buflen = read(input, buffer, (unsigned int) bufferlen); */
	buflen = _lread(input, buffer, (unsigned int) bufferlen);
	if (buflen < 0) {
/*		winerror("Read error in wingetc"); */
		strcpy (debugstr, "Read error in wingetc ");
		if (errno == EBADF) strcat(debugstr, "invalid file handle");
		(void) wincancel(debugstr);
		finish = -1;
	}
	if (buflen <= 0) return EOF;	/* end of file or read error */
 	bufptr = buffer;
	return buflen;
}

static int wingetc (HFILE input) {
	int c;
	if (ungotten >= 0)  {
		c = ungotten; ungotten = -1; filepos++; return c;
	}
	else if (buflen-- > 0) {
		filepos++; 
		return (unsigned char) *bufptr++;
	}
	else {
		if (replenish(input, BUFFERLEN) < 0) return EOF;
		buflen--;
		filepos++; 
		return (unsigned char) *bufptr++;
	}
}

static long wintell (HFILE input) {		/* where are we in the file */
	return filepos;
}

/* possibly check for whether new position is somewhere in buffer ? */
/* returns -1 if it fails 96/Aug/22 */

static long winseek (HFILE input, long place) {
	long foo;

/*	Don't do anything if current position same as requested position ? */
	if (filepos == place) return place;
	if (place < 0) {
		sprintf(str, "Negative seek %ld", place);
		winerror(str);
		filepos = _llseek(input, 0, SEEK_SET);			/* rewind */
/*		return 0; */
		return -1;
	}
/*	foo = lseek(input, place, 0);	 */
	foo = _llseek(input, place, SEEK_SET);
	if (foo != place) {
/*		sprintf(str, "Seek error: to %ld - at %ld", place, foo); */
		sprintf(str, "Seek error: to %ld ", place);
		if (errno == EBADF)  strcat(debugstr, "invalid file handle");
		else if (errno == EINVAL) strcat(debugstr, "invalid origin or position");
		winerror(debugstr);
	}
	filepos = place;
	ungotten = -1;
	buflen = 0;
	bufptr = buffer;	/* redundant ? */
/*	return foo; */
	if (foo == place) return foo;
	else return -1;		/* failed */
}

static int wininit (HFILE input) {	/* ignores `input' */
	filepos = 0;			/* beginning of file */
	ungotten = -1;
	buflen = 0;				/* nothing buffered yet */
	bufptr = buffer;		/* redundant ? */
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for reading signed and unsigned numbers of various lengths */
/* high order bytes first ... */

static unsigned int ureadone (HFILE input) {	
	int c = wingetc(input);
	if (c == EOF) return 255;
/*	return (unsigned int) wingetc(input); */
	return (unsigned int) c;
} 

static unsigned int ureadtwo (HFILE input) {
	int c=wingetc(input);
	int d=wingetc(input);
	if (c == EOF || d == EOF) return 65535;
/*	return (wingetc(input) << 8) | wingetc(input); */
/*	return (unsigned int) (c << 8 | d); */
	return (unsigned int) ((c << 8) | d);		/* 99/Jan/12 */
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

/*** code for working way into back end of file looking for post ***/

/* This does some things to work around possible crap at end of file */
/* The way to loose is get garbage at end that comes from other DVI file ! */

/* post_post q[4] i[1] followed by 4-7 223's */
/* where q[4] is pointer to post and i[1] is DVI version number (2) */
/* post p[4] num[4] den[4] mag[4] l[4] u[4] s[2] t[2] */
/* where p[4] is pointer to last eop */

/* moves to post position and sets postposition */

int gotopost(HFILE input) { /* search for post at end of file */
	int c, d, e, f;
	int k, i, j, count;
	unsigned long n, filelen;
/*	we must have BUFSIZE <= BUFFERLEN */

	if (postposition > 0) {	/* if we already know where to go ... */
		(void) winseek(input, postposition);
		return -1;
	}
	filelen = _llseek(input, 0, SEEK_END);
/*  step one buffer size back from end of file */
	if (filelen > BUFSIZE) n = BUFSIZE;
	else n = filelen;	/*	if DVI file is shorter than BUFSIZE ! */
	filepos = _llseek(input, - (long) n, SEEK_END);
	if (filepos < 0) {
		winerror("Seek Error");			/* should not happen */
		filepos = _llseek(input, 0L, SEEK_SET); /* rewind if error */
	}

	for (j=0; j < NUMSTEPS; j++) {			/* let's not go on forever ! */
/*		for (k = 0; k < BUFSIZE; k++) buffer[k] =  wingetc(input); */
		count = _lread(input, buffer, BUFSIZE);		/* get a buffer full */
		if (count != BUFSIZE) {
			winerror("Did not get full buffer");
/*			k = count - 1; */
		}
/*		else k = BUFSIZE - 1; */
		k = count - 1;
		while (k > 10) {					/* may be up to 7 MAGIC */
			count=0;						/* count MAGIC codes seen */
/*			for (i = k; i >= 5; i--) */		/* need at least seq of four */
			for (i = k; i > MINMAGIC; i--) {	/* need at least seq of four */
				if ((unsigned char) buffer[i] == MAGIC) {
					count++;
/*					if (count == 4) break; */
					if (count == MINMAGIC) break;
				}
				else count = 0;
			}
			k = i;
/*			if (count == 4) */	   /* found sequence of four */
			if (count == MINMAGIC) {	   /* found sequence of four */
/*				for (i = k; i >= 5; i--) */	/* but there can be more */
				for (i = k; i > MINMAGIC; i--)	/* but there can be more */
					if ((unsigned char) buffer[i] != MAGIC) break;
				k = i;						/* first non MAGIC - ID_BYTE ? */
				if ((unsigned char) buffer[k] != MAGIC) {  
											/* did see end of MAGIC */
					if ((unsigned char) buffer[k] != ID_BYTE) {
						winerror("File is wrong DVI version");  
						if ((unsigned char) buffer[k-5] != (int) post_post) 
							winerror("Can't find post_post");
					}
/*					let's skip this stringent test 96/Aug/22 ??? */
/*					if ((unsigned char) buffer[k-5] != (int) post_post) 
						winerror("Can't find post_post"); */
					k = k - 5;			/* try step back to post_post */
					c = (unsigned char) buffer[k+1];	
					d = (unsigned char) buffer[k+2];
					e = (unsigned char) buffer[k+3];	
					f = (unsigned char) buffer[k+4];
					n = ((((((unsigned long) c << 8) |
						(unsigned long) d) << 8) | e) << 8) | f;
/*					adjust for dvistart if textures ? looses ! */
					filepos = -1;	/* to prevent filepos == place */
/*					now try and go to post */
					if (n > filelen) continue;			/* avoid bad seek */
					if (winseek(input, (long) n) >= 0) {
						c = wingetc(input);				/* test it */
						unwingetc(c, input);			/* put it back */
						if (c == (int) post) {
							postposition = n;			/* remember it */
							return -1;	/* in good shape */
						}
						else {
							sprintf(str, "Expected POST (%d) not %d",
									(int) post, c);
							winerror(str);
							(void) winseek(input, dvistart);
							skiptoend = 0;			/* !!! */
							return 0;				/* failed */
						}
					}	/* found post_post */
				} /* not MAGIC */
			} /* count up to 4 */
		} /* while k > 10 */
		filepos = _llseek(input, - (long) ((BUFSIZE - 10) * (j+2)), SEEK_END);
		if (filepos < 0) {
/*			lseek(input, - (long) ((BUFSIZE - 10) * (j+2)), 2)) < 0) */
/*			in case got back past start of file maybe ? */
			break;
		}
	} /* maximum of NUMSTEPS backward steps from end of file */
	winerror("Can't find proper ending of DVI file"); 
	(void) winseek(input, dvistart);	/* go back to start of good stuff */
	skiptoend = 0;						/* !!! */
/*	pretty serious ? */ /* not setting postposition */
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* remove *trailing* underscores */ /* for Adobe fonts */
/* convert embedded underscores to spaces */ /* for TrueType fonts */
/* will not convert embedded underscores if begins or ends on underscore */

void removeunderscores (char *name) {
	int nlen, c;
	char *s;

	if (name == NULL) return;				/* sanity check */
	if (strchr(name, '_') == NULL) return;	/* nothing to do */
	nlen = (int) strlen(name);
	if (nlen == 8) {							/* right length ? */
		s = name + nlen - 1;
		if (*s-- == '_') {					/* end on '_' ? */
			nlen--;
			while(nlen-- > 0) if (*s-- != '_') break;
			if (nlen >= 0) *(s + 2) = '\0';	/* cut off the tail */
			return;
		}
	}
	s = name; 
	if (*s == '_') return;	/* Adobe Arial - don't mess with it 92/June/8 */
	if (bConvertUnderScores) {
		while ((c = *s++) != '\0')				/* convert embedded underscores */
			if (c == '_') *(s-1) = ' ';
	}
}

/* use this for OEM character set (file names for example) */
/* hmmm --- does it matter 96/Sep/29 ??? */

void makeuppercase (char *s) {			/* turn string into upper case */
/* NOT FOR FILE NAMES */ /* need DOS code page ? */
	if (s == NULL) return;

	CharUpper(s); 

/*	AnsiUpper(s); */
}

void makelowercase(char *s) {			/* turn string into lower case */
/* NOT FOR FILE NAMES */ /* need DOS code page ? */
	if (s == NULL) return;
	CharLower(s); 

/*	AnsiLower(s); */

}

/* make canonical font name - upper case and no trailing underscores */
/* No longer force uppercase 97/Oct/21 */ /* called from mapfontnames */

void standardizenames(void) {
	int k;
	for (k = 0; k < fnext; k++) {
		if (fontname[k] == NULL) continue;
		removeunderscores(fontname[k]);
		if (bUpperCase)	makeuppercase(fontname[k]);	/*	AnsiUpper(fontname[k]); ? */
	}
}

/*	Find other entries for same font but different sizes */
/*	This is to save on reencoding data blocks for ATM */
/*	Can also use this to copy font properties ! */
/*	Made case insensitive 97/Oct/21 */

void findbasenames (void) {					/* 96/July/23 */
	int k, i;
	basefont[0] = 0;
	for (k = 1; k < fnext; k++) {
		if (fontname[k] == NULL) continue;

		basefont[k] = (WORD) k;				/* default */

/*		basefont[k] = (unsigned char) k; */

		if (bUseBaseFont == 0) continue;	/* short-circuit */
		for (i = 0; i < k; i++) {
			if (fontname[i] == NULL) continue;
/* compare font file name - could compare face name + style instead */
/*			if (strcmp(fontname[i], fontname[k]) == 0) */
			if (_stricmp(fontname[i], fontname[k]) == 0) {	/* 97/Oct/21 ? */
/*				if (bDebug > 1) {
					sprintf(debugstr, "%d (%s) == %d (%s)\n",
							k, fontname[k], i, fontname[i]);
					OutputDebugString(debugstr);
				} */

				basefont[k] = (WORD) i;

/*				basefont[k] = (unsigned char) i; */

				break;
			}
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following rather special case, not totally reliable ... */
/* Note: may come in here with Windows Face Name - OR - font file name */
/* For CM fonts: Windows Face Name = font file name = PostScript FontName */

/*  mainly used to determine whether remapping may be required for 0 - 31 */
/*  also used to decide whether to try upper case name first */

/*  cm-text-fonts: cm
	(r|bx|tt|sltt|vtt|tex|ss|ssi|ssdc|ssbx|ssqi|dunh|bxsl|b|ti|bxti|csc|tcsc)
	([0-9]+) */

/* switched to _strnicmp to avoid need for local copy */
/* speeded this up a bit based on first letter of name */
/* BUT WAS BUGGY! dropped through in some cases - fixed 97/Sep/11 */

/* This is mostly used to guess whether a font may remap 0 - 31 upward */

BOOL istexfont(char *name) { 	/* is this a TeX CM or LaTeX font ? */
	int flag=0;
/*	char name[MAXFACENAME]; */
/*	char name[MAXFULLNAME]; */	/* LF_FULLFACENAME just in case 95/July/7 */
/*	if (strncmp(s, "LucidBriMat", 11) == 0) return TRUE; */
/*	if (strncmp(s, "LucidNewMat", 11) == 0) return TRUE; */
/*	if (strlen(s) > 12) return 0; */
/*	if (strncmp(s, "LucidMat", 8) == 0) return TRUE; */
/*	if (strlen(s) > 8) return FALSE; */
/*	strcpy(name, s); */
/*	makeuppercase(name); */ 	/*	AnsiUpper(name); ? */	/* just in case */
	if (name == NULL) return 0;		/* sanity check */
	if (bIgnoreRemapped) {		/* only worry about fixed width remapped */
		if (_strnicmp(name, "CM", 2) == 0) {
			if (strstr(name, "TT") != NULL ||
				strstr(name, "tt") != NULL) flag = 1;
		}
		return flag;							/* new 97/Sep/11 */
	}
	if (*name == 'L' || *name == 'l') {
		if (
		_strnicmp(name, "LCM", 3) == 0 ||		/* SliTeX */
		_strnicmp(name, "LASY", 4) == 0 ||		/* LaTeX Symbol */
		_strnicmp(name, "LCIRCLE", 7) == 0 || 	/* LaTeX circle */
		_strnicmp(name, "LINE", 4) == 0 ||    	/* LaTeX line */
		_strnicmp(name, "LOGO", 4) == 0 ||		/* M E T A F O N T */
		_strnicmp(name, "LM", 2) == 0 ||		/* Lucida Math Fonts (old) */
		_strnicmp(name, "LUM", 3) == 0 ||		/* Lucida Math Fonts (new) */
		_strnicmp(name, "LBM", 3) == 0 ||		/* LucidaBright Math Fonts */
		_strnicmp(name, "LNM", 3) == 0 ||		/* LucidaNew Math Fonts */
		_strnicmp(name, "LMATH", 5) == 0		/* TT LucidaBrightMath */
			) flag=1;
	}
	else if (*name == 'M' || *name == 'm') {
		if (
		_strnicmp(name, "MANFNT", 6) == 0 ||
		_strnicmp(name, "MSAM", 4) == 0 ||		/* AMS symbol fonts */
		_strnicmp(name, "MSBM", 4) == 0	||		/* AMS symbol fonts */
		_strnicmp(name, "MT", 2) == 0			/* MathTime fonts */
									/* conflict with MonoType ? */
			) flag=1;
	}
	else if (*name == 'I' || *name == 'i') {
		if (
		_strnicmp(name, "ILCM", 4) == 0 ||		/* SliTeX */
		_strnicmp(name, "ICM", 3) == 0			/* SliTeX */
			) flag=1;
	}
	else if (_strnicmp(name, "CM", 2) == 0 ||	/* Computer Modern */
			 _strnicmp(name, "EU", 2) == 0 ||	/* AMS Euler fonts */
			 _strnicmp(name, "WNCY", 4) == 0	/* AMS Cyrillic fonts */
			) flag=1;
/*	else return FALSE; */
#ifdef IGNORED
	sprintf(str, "%s\ttexfont %d", name, flag);
	wininfo(str);
#endif
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* restore at start of pageno what was current at end of pageno-1 */
/* called from winanal.c */

void RestoreColorStack(int pageno) {
	COLORREF FAR *lpColorStack;
	int k;
	if (bCarryColor == 0 || bColorUsed == 0) return;
	if (pageno >= MaxColor) return;	/* sanity check */
	pageno--;			/* restore what was saved at eop of *previous* */
	if (pageno < 1) {	/* nothing saved to restore at start of first page */
		colorindex = 0;
		return;
	}
	GrabColor();
	if (lpColor[pageno] == NULL) {
		sprintf(debugstr, "Bad Color Restore pageno-1 %d", pageno);
		(void) wincancel(debugstr);
	}
	else {
		lpColorStack = lpColor[pageno];
		colorindex = lpColorStack[0];
#ifdef DEBUGCOLORSTACK
		if (bDebug > 1) {
			sprintf(debugstr, "RestoreColorStack page-1 %d colorindex %d",
					pageno, colorindex);
			OutputDebugString(debugstr);
		}
#endif
		if (colorindex > 0 && colorindex < MAXCOLORSTACK) {
			for (k = 0; k < colorindex; k++)
				ColorStack[k] = lpColorStack[k+1];
		}
		else {
			if (bDebug > 1) {
				sprintf(debugstr, "colorindex %d", colorindex);
				OutputDebugString(debugstr);
			}
			
		}
	}
	ReleaseColor();
}

/* set background color of page from saved \special{background ...} info */

void RestoreBack(int dvipage) {
	if (bCarryColor == 0 || bBackUsed == 0) return;
	if (dvipage < 0 || dvipage >= MaxBack) return;	/* sanity check */
	GrabBack();
	if (bDebug > 1) {
		sprintf(debugstr, "dvipage %d back clr %X", dvipage, lpBack[dvipage]);
		OutputDebugString(debugstr);
	}
/*	don't bother if the background color hasn't changed */
	if (clrBackground != lpBack[dvipage]) {
		clrBackground = lpBack[dvipage];
/*		use the default brush if its just the standard window color */
/*		if (clrBackground != COLOR_WINDOW) */
/*		if (clrBackground != (COLOR_WINDOW+1)) *//* 98/Sep/8 ??? */
		if (clrBackground != OrgBkColor)		/* 98/Sep/8 ??? */
/*			make special colored brush for background */
			hbrBackground = CreateSolidBrush(clrBackground);
		else hbrBackground = hbrDefaultBackground;
	}
	ReleaseBack();
}

/* need to delete this background brush again at end of page ? */

void DeleteBackBrush(HDC hDC) {
/*	don't try and delete brush with standard window color */
/*	if (clrBackground != COLOR_WINDOW) */
/*	if (clrBackground != (COLOR_WINDOW+1)) */ 	/* 98/Sep/8 ??? */
	if (clrBackground != OrgBkColor) {	/* 98/Sep/8 ??? */
/*		don't try and delete default background brush */
		if (hbrBackground != hbrDefaultBackground) {
			(void) SelectObject(hDC, hbrDefaultBackground);
			DeleteObject(hbrBackground);
			hbrBackground = hbrDefaultBackground;
		}
	}
}

/* Save at end of pageno for start of pageno+1 */
/* called from winpslog.c */

void SaveColorStack(int pageno, int colorindex) {
	COLORREF FAR *lpColorStack;
	int k;
/*	if (pageno > MaxColor) error */
/*	if (bCarryColor == 0 || bColorUsed == 0) return; */
	if (bCarryColor == 0) return;					/* 98/Sep/6 */
/*	if (pageno < 0 || pageno >= MaxColor) return; */
	GrabColor();
	if (lpColor[pageno] != NULL) {
		sprintf(debugstr, "Bad Color Save pageno %d (%d) %08x",
				pageno, MaxColor, lpColor[pageno]);
		winerror(debugstr);
/*		free(lpColor[pageno]); */
	}
#ifdef DEBUGCOLORSTACK
	if (bDebug > 1) {
		sprintf(debugstr, "SaveColorStack pageno %d colorindex %d",
				pageno, colorindex);
		OutputDebugString(debugstr);
	}
#endif
	lpColorStack = (COLORREF FAR *) malloc ((colorindex+1) * sizeof(COLORREF));
	lpColor[pageno] = lpColorStack;
	lpColorStack[0] = colorindex;
	for (k = 0; k < colorindex; k++)
		lpColorStack[k+1] = ColorStack[k];
	ReleaseColor();
}

/* format of allocated area is count followed by stack dump */

/* These saved color stacks can be flushed again if at end of prescan */
/* we finds that bColorUsed == 0 */ /* FreeColor() in dviwindo.c */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int stinx;				/* stack index */ /* just for verification */

/* following activated 96/Sep/2 to deal with \special before bop */
/* flush rest of special to get back to rest of DVI code */

void log_flushspecial(HFILE input) {
	int c;
	if (nspecial <= 0) return;
	c = wingetc(input); nspecial--;
	while (nspecial > 0 && c != EOF) {
		c = wingetc(input); nspecial--;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Now for the scan of the DVI file for font log generation */

/* void reset_stack(void) {
	stinx = 0;
} */ 	/* done at BOP */

void check_stack(void) {	/* done at EOP */
	if (stinx != 0) {
		winerror("Stack not empty at EOP");
		errcount();
	}
}

LPLONG PageStart;	/* Pointer to Page Start Table */
LPLONG PageCount;	/* Pointer to Count[0] Table */

void logdo_bop(HFILE input) { /* beginning of page */
	int k;
	long current;

	current = wintell(input) - 1;
	if (ps_current < 0)  	/* remember FIRST bop */
		ps_current = current;

	if (skiptoend != 0) { /* see whether we should now go to end of file */
		if (gotopost(input) == 0) {
			errcount();		/*  stop as soon as feasable ! */
/*			failed to find POST */
		}
		return;
	}

/*	pagenumber++; */		/* not if reversing ??? */
	
/*	reset_stack(); */		/* stinx = 0; */
	stinx = 0;
	dvi_f = -1;				/* undefined font */
	fnt = finx[0];			/* just in case - not clear could be -1 ! or 255 */

	for(k=0; k < 10; k++) counter[k] = sreadfour(input);

/*	PageStart[pageno] = current; */ 		/* build later ? BuildPages */
/*	PageCount[pageno] = counter[0]; */ 		/* build later ? BuildPages */
	if (bCarryColor) {
		PageStart[pageno] = current; 
		PageCount[pageno] = counter[0]; 
#ifdef DEBUGCOLORSTACK
		if (bDebug > 1) {
			sprintf(debugstr, "pageno %d PageStart %d PageCount %d\n",
					pageno, PageStart[pageno], PageCount[pageno]);
			OutputDebugString(debugstr);
		}
#endif
/*		pageno++;  */
	}
	pageno++;	/* destored here 99/Apr/05 */
	if (pageno > 0) {		/* inherit page color of previous page 99/Apr/05 */
		GrabBack();			/* ? */
		lpBack[pageno] = lpBack[pageno-1];
		ReleaseBack();		/* ? */
	}

	if (textures != 0) (void) sreadfour(input);
	else ps_previous = sreadfour(input); 
	if (bCarryColor) {			/*  && colorindex > 0 ??? */
/*		if (colorindex > 0) */ /* aoivd underflow on first page ? */
		doColorPop(hdc);		/* 98/Feb/15 to match ColorPush in eop */
	}
}

void logdo_eop(HFILE input) { /* end of page */
	int c;
	check_stack();									/* check DVI stack */
	if (bCarryColor) {
		doColorPush(hdc);							/* 98/Feb/15 */
		SaveColorStack(pageno, colorindex);		/* 98/Feb/19 */
	}
	if (textures != 0) 
		texlength = sreadfour(input);	/* TeXtures length code */
	c = wingetc(input); unwingetc(c, input);		/* peek ahead */
/*	here we expect to see bop, nop or fnt_def's ONLY */
	if (c >= 0 && c <= 127) {
		sprintf(str, "Invalid DVI code %d between EOP and BOP", c);
		winerror(str); 
		errcount();
	}
}

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

#ifdef ALLOWCOLOR

int maxstinx;			/* max stack index seen */

void logdo_push(void) {
	if (++stinx > maxstinx) maxstinx = stinx;
} 

void logdo_pop(void) {
	if (--stinx < 0) {
		winerror("The stack will underflow"); 
		errcount(); 
	}
} 

void logdo_set1(HFILE input) {
	unsigned int c;
	c = ureadone(input);
/*	if (dvi_f < 0) invalidset((int) c); */
} 

void logdo_set2(HFILE input) {
	unsigned int c;
	c = ureadtwo(input);
/*	if (dvi_f < 0) invalidset((int) c); */
}

void logdo_setsub(unsigned long c) {
/*	if (dvi_f < 0) invalidset((int) c); */
}

void logdo_set3(HFILE input) {
	logdo_setsub(ureadthree(input)); 
}

void logdo_set4(HFILE input) {
	logdo_setsub(sreadfour(input)); 
}

void logdo_put1(HFILE input) {
	unsigned int c;
	c = ureadone(input);
/*	if (dvi_f < 0) invalidset((int) c); */
}

void logdo_put2(HFILE input) {
	unsigned int c;
	c = ureadtwo(input);
/*	if (dvi_f < 0) invalidset((int) c); */
}

void logdo_putsub(unsigned long c) {
/*	if (dvi_f < 0) invalidset((int) c); */
}

void logdo_put3(HFILE input) {
	logdo_putsub(ureadthree(input));
}

void logdo_put4(HFILE input) {
	logdo_putsub(sreadfour(input));
}

void logdo_set_rule(HFILE input) {
	int k;
	for (k=0; k < 8; k++)  wingetc(input); /* a and b */
}

void logdo_put_rule(HFILE input) {
	int k;
	for (k=0; k < 8; k++)  wingetc(input); /* a and b */
}

void logdo_right1(HFILE input) { 
	wingetc(input);
}

void logdo_right2(HFILE input) {
	wingetc(input); wingetc(input);
}

void logdo_right3(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input);
}

void logdo_right4(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input); wingetc(input);
}

void logdo_w0(void) {
}

void logdo_w1(HFILE input) { 
	wingetc(input); 
}

void logdo_w2(HFILE input) {
	wingetc(input); wingetc(input);
}

void logdo_w3(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input);
}

void logdo_w4(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input); wingetc(input);
} 

void logdo_x0(void) {
}

void logdo_x1(HFILE input) { 
	wingetc(input);
}

void logdo_x2(HFILE input) {
	wingetc(input); wingetc(input);
}

void logdo_x3(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input);
}

void logdo_x4(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input); wingetc(input);
}

void logdo_down1(HFILE input) { 
	wingetc(input);
}

void logdo_down2(HFILE input) { 
	wingetc(input); wingetc(input);
}

void logdo_down3(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input);
}

void logdo_down4(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input); wingetc(input);
}

void logdo_y0(void) {
}

void logdo_y1(HFILE input) { 
	wingetc(input);
}

void logdo_y2(HFILE input) {
	wingetc(input); wingetc(input);
}

void logdo_y3(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input);
}

void logdo_y4(HFILE input) { 
	wingetc(input); wingetc(input); wingetc(input); wingetc(input);
}

void logdo_z0(void) {
}

void logdo_z1(HFILE input) {  
	wingetc(input);
}

void logdo_z2(HFILE input) {
	wingetc(input); wingetc(input);
} 

void logdo_z3(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input);
}

void logdo_z4(HFILE input) {
	wingetc(input); wingetc(input); wingetc(input); wingetc(input);
}

/********************************************************************/

void logdo_fnt1(HFILE input) { 
	int fs;
	fs = ureadone(input);
/*	logswitchfont(fs, input); */
}

void logdo_fnt2(HFILE input) { 
	unsigned int fs;
	fs = ureadtwo(input);
/*	if (fs > 255) fs = 255; */
/*	logswitchfont((int) fs, input); */
}

void logdo_fntsub(HFILE input, unsigned long fs) { 
/*	if (fs > 255) fs = 255;*/
/*	logswitchfont((int) fs, input); */
}

void logdo_fnt3(HFILE input) { 
	logdo_fntsub(input, ureadthree(input));
}

void logdo_fnt4(HFILE input) { 
	long fs;
	fs = sreadfour(input);
	if (fs < 0) fs = 0;
	logdo_fntsub(input, (unsigned long) fs);
}
#endif	/* ifdef ALLOWCOLOR */

/**************************************************************************/

#ifdef ALLOWCOLOR

/* \special{papersize=5.04in,3.75in} */

int decodepapersize (char *papersize, int *pagewidth, int *pageheight) {
	double multiple;
	double width, height;
	char units[3];
	char *s=papersize;
	int n;

	if (sscanf(papersize, "%d*%d", pagewidth, pageheight) == 2)
		return 0;			// 2000 May 27
/*	printf("papersize=%s\n", papersize); */	/* DEBUGGING ONLY */
	if(sscanf(s, "%lg%n", &width, &n) > 0) {
		s = s + n;
		units[0] = *s++;
		units[1] = *s++;
		units[2] = '\0';
		multiple = decodeunits(units);
		width = width * multiple;
	}
	else {
		sprintf(debugstr, "Don't understand papersize %s\n", papersize);
		(void) wincancel(debugstr);
		return -1;
	}
	while (*s != ',' && *s != '*' && *s != '\0') s++;
	if (*s != '\0') s++;
	if(sscanf(s, "%lg%n", &height, &n) > 0) {
		s = s + n;
		units[0] = *s++; units[1] = *s++; units[2] = '\0';
		multiple = decodeunits(units);
		height = height * multiple;
	}
	else {
		sprintf(debugstr, "Don't understand papersize %s\n", papersize);
		(void) wincancel(debugstr);
		return -1;
	}
	*pagewidth = (int) (width+0.5);		/* in bp */
	*pageheight = (int) (height+0.5);	/* in bp */
/*	sprintf(debugstr, "pagewidth %lg pageheight %lg\n", *pagewidth, *pageheight); */
	return 0;
}

void getpapersize (void) {	 //  from papersize=....
	char *s;
	s = line;
	while (*s != '=' && *s != ' ' && *s != '\0') s++;
	if (*s == '\0') return;
	s++;
	while (*s <= ' ' && *s != '\0') s++;
	if (*s == '\0') return;
	decodepapersize(s, &PageWidth, &PageHeight);
}

/* Presently only looking for \special{PDF: BBox * * * *} */
/* and for \special{color ... } */
/* and for \special{background  ... } */
/* can't do this since we don't actually scan the DVI file ! */
/* now we do if bCarryColor is set ! */

void checkspecial (HFILE input) {					/* 96/Sep/29 */
	char *s=line;
	int c;
/*	int flag=0; */
/*	int n; */
#ifdef DEBUGSPECIAL
	if (bDebug > 1) {
		OutputDebugString("CHECKSPECIAL\n");	/* debugging only */
	}
#endif
/*	read in the whole special */
	if (nspecial <= 0) return;				/* ignore if empty */
	if (nspecial >= sizeof(line)-1) return;	/* ignore if too long */
	while (nspecial > 0) {
		c = wingetc(input); nspecial--;
		*s++ = (char) c;
	}
	*s = '\0';
#ifdef DEBUGSPECIAL
	if (bDebug > 1) OutputDebugString(line);
#endif
	if (strncmp(line, "color", 5) == 0 && (*(line+5) == ' ' || *(line+5) == ':')) {
		DoColor(hdc, input, 0);
/*		build up color stack here */ /* bottom of stack has the color before */
		bColorUsed = 1;
	}
	else if (strncmp(line, "background", 10) == 0 &&
		(*(line+10) == ' ' || *(line+10) == ':')) {
		DoBackGround(hdc, input, 0);	/* remember background for this page */
		bBackUsed = 1;					/*  98/Sep/6 ??? */
	}
	else if (strncmp(line, "papersize", 9) == 0) getpapersize();
	else if (strncmp(line, "PDF:", 4) == 0) {
		if ((s = strstr(line, "BBox")) != NULL) {
			if (sscanf(s+5, "%d %d %d %d", &BBxll, &BByll, &BBxur, &BByur) == 4);
			else {
				BBxll = BByll = BBxur = BByur = 0;
			}
		}
	}
	return;
}
#endif	/* ifdef ALLOWCOLOR */

void logdo_com(HFILE input) {	
#ifdef ALLOWCOLOR
	checkspecial(input); 					/* not useful */
#endif
	log_flushspecial(input);
}

void logdo_xxxi(HFILE input, unsigned int n) {
	nspecial=(long) n;
	logdo_com(input);
}

void logdo_xxx1(HFILE input) { 
	logdo_xxxi(input, ureadone(input));
}

void logdo_xxx2(HFILE input) { 
	logdo_xxxi(input, ureadtwo(input));
}

void logdo_xxxl(HFILE input, unsigned long n) {
	nspecial=(long) n;
	logdo_com(input);
}

void logdo_xxx3(HFILE input) { 
	logdo_xxxl(input, ureadthree(input));
}

void logdo_xxx4(HFILE input) { 
	logdo_xxxl(input, sreadfour(input));
}

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

char *zstrdup (char *s) {		/* 1995/July/7 */
	char *new = _strdup(s);
/*	int n = strlen(s); */				/* debugging */
/*	if (n > maxstrlen) maxstrlen = n; *//* debugging */
	if (new == NULL) {
/*		winerror("Unable to Allocate Memory"); */
		sprintf(debugstr, "Unable to Allocate Memory (for %s)", s);
		winerror(debugstr);
		PostQuitMessage(0);				/* pretty serious ! */
/*		PostMessage(hwnd, WM_CLOSE, 0, 0L); */		/* say bye-bye */
	}
	return new;
}

void checkfontname (int k) {
	if (fontname[k] != NULL) {
		if (bDebug > 1) {
			sprintf(debugstr, "fontname %d (%s) not free", k, fontname[k]);
			OutputDebugString(debugstr);
		}
		free(fontname[k]);
		fontname[k] = NULL;
	}
}

void checksubfontname (int k) {
	if (subfontname[k] != NULL) {
		if (bDebug > 1) {
			sprintf(debugstr, "subfontname %d (%s) not free", k, subfontname[k]);
			OutputDebugString(debugstr);
		}
		free(subfontname[k]);
		subfontname[k] = NULL;
	}
}

/* need to do this even if skipping pages */

void logfnt_def (HFILE input, unsigned int k) { /* fnt_def */
	unsigned int f, na, nl, i;
	int newfont=1;		/* if this is a new one (not defined before) */
//	char fp[MAXTEXNAME];					/* 1995/July/7 */
	char fp[FNAMELEN];

	if (k == (unsigned int) -1) {		/* bad DVI file */
		winerror("logfnt_def -1");
		errcount();
	}
/*	if (bDebug > 1) {
		sprintf(debugstr, "fnt_def %u\n", k);
		OutputDebugString(debugstr);
	} */
	if (k >= MAXFONTNUMBERS) {
		sprintf(debugstr, "Internal font number %u > %u", k, MAXFONTNUMBERS-1);
		winerror(debugstr);
		k = MAXFONTNUMBERS-1;
		errcount();				/* die as soon as possible */
	}
	if (finx[k] != BLANKFONT) {		/* seen this font before !!! */
		sprintf(debugstr, "Font %u being redefined", k);
		winerror(debugstr);
		errcount();
		newfont = 0;
		f = (unsigned int) finx[k];
	}
	else {				/* definition of font not seen before */
		f = (unsigned int) fnext; 
		finx[k] = (short) f;					/* 96/July/25 */
		if (fnext++ >= MAXFONTS) {
			sprintf(str, "More than %d fonts in use", MAXFONTS-1);
			fnext--;
			winerror(str);
			errcount();
		}
	}
/*	(void) sreadfour(input); */
	fc[f] = (unsigned long) sreadfour(input);	/* checksum 95/Jan/12 */
	fs[f] = (unsigned long) sreadfour(input);	/* at size */
/*	fd[f] = (unsigned long) sreadfour(input); *//* design size */ /* NA */
	(void) sreadfour(input);					/* ignore design size */
	na = ureadone(input); 		/* length of `area' or directory */
/*	na = wingetc(input); */
	nl = ureadone(input);		/* length of `font file name' */
/*	nl = wingetc(input); */
#ifdef IGNORED
	if (bDebug > 1) {
		sprintf(debugstr, "f %d fc %0X fs %0X na %u nl %u na+nl %u",
				f, fc[f], fs[f], na, nl, na+nl);
		OutputDebugString(debugstr);
	}
#endif
/*	sanity check on size of na and nl ? */
	if (newfont == 0) {			/* just skip over if already defined */
		for (i = 0; i < na + nl; i++) (void) wingetc(input);
		return;
	}
/*	fp = fontname[f]; */	/* pointer to font name - now assemble first */
	for (i = 0; i < na + nl; i++) {
		if (i < sizeof(fp)) fp[i] = (char) wingetc(input);
		else (void) wingetc(input);			/* discard */
	}
	if (na + nl == 0) {
		fp[na + nl] = '\0';
		sprintf (debugstr, "Name of font %d zero length", k);
		winerror (debugstr);
		errcount();
		strcpy(fp, "NullFont");	/* :-) */
		nl = strlen(fp);
	}
//	if (na + nl >= MAXTEXNAME) 
	if (na + nl >= sizeof(fp)) {
//		fp[MAXTEXNAME-1] = '\0';
		*(fp + sizeof(fp) - 1) = '\0';
		sprintf (debugstr, "Font name %s... longer than %d",
				 fp, sizeof(fp)-1);
		winerror (debugstr);
		errcount();
		na = 0;
//		nl = MAXTEXNAME-1;
		nl = sizeof(fp)-1;
	}
	else fp[na + nl] = '\0';
	if (fontname[f] != NULL) checkfontname(f);
	fontname[f] = zstrdup(fp);				/* 1995/July/5 */
		
/*  following superceeded by code in winanal.c */
/*	but used when getting metrics ? */
	texfont[f] = (char) istexfont(fontname[f]);	/* may eliminate ? */
	
	if (subfontname[f] != NULL) checksubfontname(f);
	subfontname[f] = NULL;						/* 1999/Jan/4 */

/*	fontsubflag[f] = -1; */ /* NA */ 	/* all this goes to extract now */
/*	if (substitute != 0) 
		fontsubflag[f] = fontremap(fontname[f]);
	if (uppercaseflag != 0) makeuppercase(font, fontname[f]); */
}

void logdo_fnt_def1(HFILE input) { /* define font */
/*	unsigned int k; */
/*	k = ureadone(input); */
	logfnt_def(input, ureadone(input)); 
/*	logfnt_def(input, wingetc(input)); */
}

void logdo_fnt_def2(HFILE input) { /* define font */
	unsigned int k;

	k = ureadtwo(input);
/*	if (bDebug && bDebugKernel) {
		sprintf(debugstr, "fnt_def2 %u\n", k);
		OutputDebugString(debugstr);
	} */
/*	if (k > 255) k = 255; */
	if (k >= MAXFONTNUMBERS) k = MAXFONTNUMBERS-1;	/* 93/Dec/11 */
	logfnt_def(input, k);
}

void logdo_fnt_defsub(HFILE input, unsigned long k) {
/*	if (k > 255) k = 255; */
	if (k >= MAXFONTNUMBERS) k = MAXFONTNUMBERS-1;	/* 93/Dec/11 */
	logfnt_def(input, (unsigned int) k);
}

void logdo_fnt_def3(HFILE input) { /* define font */
	logdo_fnt_defsub(input, ureadthree(input));
}

void logdo_fnt_def4(HFILE input) { /* define font */
	long k;
	k = sreadfour(input);
	if (k < 0) k = 0;
	logdo_fnt_defsub(input, (unsigned long) k);
}

/* need to do this even if skipping pages */

void logdo_pre(HFILE input) {		/* pre */
	unsigned int i, k, j;
	int c;
	char *s;
	
	i = ureadone(input); 
/*	i = wingetc(input); */
	if (i < 1 || i > 3) {
/*		sprintf(str, "FilePos %ld BufLen %d", filepos, buflen);	
		winerror(str); */ /* DEBUG */
		filepos = 0; /* to prevent file position message */
		winerror("ERROR: Not a valid DVI file"); 
		finish = -1;	/* giveup(13); */
		return;			/* ? */
	}
	else if (i != ID_BYTE) {
		winerror("File is wrong DVI version");  /* but maybe OK anyway ? */
		/* errcount(); */ 
	}
	dvi_version = (int) i;

	dvi_num = (unsigned long) sreadfour(input);
	dvi_den = (unsigned long) sreadfour(input);
	dvi_mag = (unsigned long) sreadfour(input);

	k = ureadone(input); 
/*	k = wingetc(input); */
	s = dvi_comment; 

	c = wingetc(input);			/* try and discard space */
	if (c == ' ') k--;
	else unwingetc(c, input);

	for (j=0; j < k; j++) { 
		c = wingetc(input); 
		if (j < MAXCOMMENT)	*s++ = (char) c;  
/*		if (j < MAXCOMMENT)	dvi_comment[j] = (char) c; */
	}
	*s++ = '\0';
	
	if (textures != 0)				/* texlength = ureadfour(input); */
		(void) sreadfour(input); 		/* skip TeXtures length code */
}

/* need to do this even if skipping pages */

void logdo_post(HFILE input) {
	if (postposition == 0)
		postposition = wintell(input) - 1;
	if (textures != 0) (void) sreadfour(input);
	else ps_previous = sreadfour(input);  
/*	sreadfour(input); */
	dvi_num = (unsigned long) sreadfour(input);
	dvi_den = (unsigned long) sreadfour(input);
	dvi_mag = (unsigned long) sreadfour(input);
/*	compare the above with what was in preamble ? */
	dvi_l = (unsigned long) sreadfour(input);	/* max page height + depth */
	dvi_u = (unsigned long) sreadfour(input);	/* max page width */
	dvi_s = (int) ureadtwo(input);				/* max stack depth */
	dvi_t = (int) ureadtwo(input);				/* number bops */
//	sprintf(debugstr, "dvi_t %d", dvi_t);
//	winerror(debugstr);
//	Problem if more than 65535 pages since then dvi_t wraps around
/*	here l and u could be used for bbox info ? */
/*	except: don't include headers and footers and other problems */	
/*	if (reverseflag == 0) */ /* end of file - NORMAL `finish' */
	if (skiptoend == 0 && textures == 0) finish = -1; 
	else skiptoend = 0;
#ifdef DEBUGCOLORSTACK
	if (bDebug > 1) {
		OutputDebugString("POST"); 			/* debugging only */
		sprintf(debugstr, "colorindex at end %d", colorindex);
		OutputDebugString(debugstr);		/* 98/Feb/28 */
	}
#endif
}

/* could do this even in forward mode to check on number of pages ? */

void logdo_post_post(HFILE input) { /* only in reverse ? */
	unsigned long q; 
	unsigned int i; 
	q = (unsigned long) sreadfour(input);	/* backward pointer to post */
	i = ureadone(input);					/* DVI version number (2) */
/*	check ID_BYTE again ? */
/*  followed by at least four 223's */
	finish = -1;		/* end of DVI file - REVERSE use of `finish' */
}

#ifdef ALLOWCOLOR
int getpagecount (HFILE input) {	/* get dvi_t etc. so can alloc pages */
	int c;
	long present;
	present = wintell(input);
	if (gotopost(input) == 0) {
		errcount();				/*  stop as soon as feasable ! */
		return -1; 				/* failed to find post */
	}
	c = wingetc(input);				/* this better be post ! */
	if (c != post) {
		sprintf(debugstr, "NOT POST %d (at byte %ld)", c, postposition);
		winerror(debugstr);
		errcount();		/*  stop as soon as feasable ! */
		return -1; 					/* error return */
	}
	logdo_post(input);
	if (bDebug > 1) {
		sprintf(debugstr, "dvi_s %d dvi_t %d", dvi_s, dvi_t);
		OutputDebugString(debugstr);
	}
/*	May want to do a sanity check here on max stack depth and number of pages */
	(void) winseek (input, present);
	return 0;						/* normal return */
}									/* new 98/Feb/15 */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* This version scans for Textures length code followed by pre & DVI code */
/* could do something more fancy to get quickly to resource fork */
/* should be fairly safe, unless initial length code is > 256 */
/* Search for 3 or more zeros in a row, followed by dont-care (length) */
/* - followed by pre and ID_BYTE */

int readovertext(HFILE input) {
	int c, n;

	c = wingetc(input);
	for(;;) {
		if (c == 0) {
			n = 1;
			while ((c = wingetc(input)) == 0) n++;
			if (c == EOF) return 0;
			if (n >= 3) {
				if((c = wingetc(input)) == (int) pre) {
/*					unwingetc(c, input);
					dvistart = wintell(input);
					c = wingetc(input); */
					dvistart = wintell(input) - 1;
					if ((c = wingetc(input)) == ID_BYTE) {
/*						(void) winseek(input, dvistart); */
						return -1;
					}
				}
			}
		}
		else if ((c = wingetc(input)) == EOF) return 0;
	}	
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Code to read a line from ASCII file using Windows _lread */
/* Could be made more efficient by reading in larger chuncks ? */
/* Does not allow \r alone as line terminator */			/* 1995/Dec/1 */
/* Converts \r \n sequence into just \n (like fopen "r" mode does) */

#ifdef LONGNAMES

#define LFBUFFERLEN 256

BOOL bWrite=0;							/* open in "w*" mode */
BOOL bBinary=0;							/* open in "*b" mode */

int nbuflen=0;							/* how many bytes left */

/* char lfbuffer[256];	*/				/* low level buffer */

char *lpBuffer=NULL;					/* locally allocated buffer */

char *sbuffer;							/* pointer into above buffer */

/* This allocates a local buffer for reading - not always used ... */

int lfclose (HFILE hfile) {
	if (lpBuffer != NULL) {
		LocalFree ((HLOCAL) lpBuffer);
		lpBuffer = NULL;
		nbuflen = 0;					/* redundant */
		sbuffer = NULL;					/* redundant */
	}
	if (hfile == HFILE_ERROR) {
		sprintf(debugstr, "%s (%s)\n", "File already closed", "lfclose");
		if (bDebug) {
			if (bDebug > 1) OutputDebugString(debugstr);
			else (void) wincancel(debugstr);
		}
		return -1;
	}
	else return _lclose(hfile);
}

HFILE lfopen(const char *name, const char *omode) {
	HFILE hfile;

	if (*(omode+1) == 'b') bBinary=1;	else bBinary = 0;
	if (*omode == 'w') bWrite=1;	else bWrite = 0;
	if (bWrite) {
		hfile = _lcreat(name, 0);
	}
	else {
		if (lpBuffer != NULL) {
			winerror("Open Error");		/* opening when already one open */
/*			return NULL; */
			return HFILE_ERROR;			/* 95/Dec/10 */
		}
/*		hfile = _lopen(name, READ); */
		hfile = _lopen(name, READ | OfShareCode); /* 96/May/18 ??? */
		if (hfile != HFILE_ERROR) {
			lpBuffer = (char *) LocalAlloc(LMEM_FIXED, LFBUFFERLEN);
			nbuflen = 0;
			sbuffer = lpBuffer;			/* redundant */
		}
	}
	return hfile;
}

int lfreplenish (HFILE input) {
	if (input == HFILE_ERROR) {
		(void) wincancel("File not open");		/* debugging */
		return 0;
	}
	nbuflen = _lread(input, lpBuffer, LFBUFFERLEN);
	sbuffer = lpBuffer;
	return nbuflen;								/* zero on end of file */
}

char *lfgets(char *buffer, int nlen, HFILE input) {
	char *s=buffer;

	if (nlen-- <= 0) return NULL;

	if (nbuflen == 0) {							/* buffer empty yet ? */
		if (lfreplenish(input) == 0) return NULL;	/* EOF right away */
	}
	*s = *sbuffer++;						/* first byte */
	nbuflen--;

	while (nlen-- > 0) {
		if (*s == '\n') {					/* continue up to newline */
			if ((bBinary == 0) && (s > buffer) && (*(s-1) == '\r')) {
				*(s-1) = '\n';				/* combined \r \n into just \n */
			}
			else s++;
			break;
		}
		s++;
		if (nbuflen == 0) {
			if (lfreplenish(input) == 0) break;	/* EOF */
		}
		*s = *sbuffer++;
		nbuflen--;
	}
	*s = '\0';
/*	if (bDebug > 1) {
		if ((nLines++ % 10) == 0) OutputDebugString(buffer);
	} */
	return buffer;
}

/* presently expand \n into \r\n only at end of string ... */
/* check for buried \n also ? */
/* be careful not to write back into character string given ... */

int lfputs(const char *s, HFILE file) {
	int n = strlen(s);
	char *newline = "\r\n";

	if (bBinary == 0 && *(s+n-1) == '\n') {
		if (_lwrite(file, s, n-1) == HFILE_ERROR) return EOF;
		if (_lwrite(file, newline, 2) == HFILE_ERROR) return EOF;
	}
	else if (_lwrite(file, s, n) == HFILE_ERROR) return EOF;
	return 0;
}

/* type 0 -> from begin, type 1 -> from current, type 2 -> from end */

int lfseek (HFILE file, long filepos, int type) {
	nbuflen = 0;				/* buffered data now irrelevant */
	sbuffer = lpBuffer;			/* redundant */
	if (_llseek(file, filepos, type) == HFILE_ERROR) return -1;
	else return 0;
}

void lfrewind (HFILE file) {
	nbuflen = 0;					/* buffered data now irrelevant */
	sbuffer = lpBuffer;				/* redundant */
	_llseek(file, 0, 0);			/* with respect to start of file */
}

/* FOLLOWING ONLY IF READING DIRECT, NOT BUFFERED ... */

long lftell (HFILE file) {
	return _llseek(file, 0, 1);		/* with respect to current point */
}

int lfgetc(HFILE file) {
	unsigned char buffer[1];
	if (_lread(file, buffer, 1) > 0) return buffer[0];
	else return EOF;
}

int lfungetc(int c, HFILE file) {
	_llseek(file, -1, 1);
	return c;
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Called *only* in dviwindo.c InitInstance */

 void initialfonts (void) {				/* called from InitInstance */
	int k;
	for (k = 0; k < MAXFONTS; k++) {
		fontname[k] = NULL;				/* 1998/Jan/4 */
		subfontname[k] = NULL;			/* 1998/Jan/4 */
		windowfont[k] = NULL;			/* handle to GDI font */
		fontwarned[k] = 0;				/* NO ? */
		fontbold[k] = fontitalic[k] = fontttf[k] = 0;
		ansifont[k] = 0;				/* 1999/Jan/78 ? */
	}
}

/* Following called when fnext reset to zero */
/* (and from dviwindo.c at very end during cleanup) */

void FreeFontNames (void) {				/* 1995/July/7 */
	int k;

/*	maybe only need to go up to k == fnext ? */
	for (k = 0; k < MAXFONTS; k++) {
		if (fontname[k] != NULL) {
			free(fontname[k]);
			fontname[k] = NULL;			/* 1998/Jan/4 */
		}
		if (subfontname[k] != NULL) {
			free(subfontname[k]);
			subfontname[k] = NULL;		/* 1998/Jan/4 */
		}
	}
}

/*  To avoid deleting currently selected font: */
/*  make sure you select something safe like hFontOld before calling this */

/* void clearfonts(void) {
	int k;

	for (k = 0; k < MAXFONTS; k++) {
		fontvalid[k] = 0;
		fontexists[k] = 0;
		if (windowfont[k] != NULL) {
			(void) DeleteObject(windowfont[k]);
			windowfont[k] = NULL;
		}
	}
} */

/* void clearmetrics(void) {
	int k;
	for (k = 0; k < MAXFONTS; k++) metricsvalid[k] = 0;
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Code for reading ATM.INI (Type 1 fonts) and WIN.INI (TrueType) */
/* to determine mapping from font file name to Windows Face name */
/* result used to setup subfontname[] based on fontname[] */
/* NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */

int aliasflag=0;		/* first try without, then try with 1994/Mar/18 */

/* See if any font in DVI file has same name as font file name */
/* If so, replace with (presumed) MicroSoft Windows Face name */
/* (Used to return number of fonts for which it has found the FaceName) */
/* Now returns number of fonts not yet resolved 96/May/28 */
/* Made case insensitive 97/Oct/21 */

int InsertSub (char *filename, char *MSWname, 
			int boldflag, int italicflag, int ttfflag) {
	int k, count = 0;
	char *fname;
	int fnbase;

//	if (strncmp(MSWname, "Ocean", 5) == 0 ||
//		  strncmp(filename, "Ocean", 5) == 0) {
//		sprintf(debugstr, "FileName `%s' MSWname `%s'", filename, MSWname);
//		if (wincancel(debugstr)) return 0; // debugging only
//	}

	if (*filename == '\0' || *MSWname == '\0') {
//		sprintf(debugstr, "File name `%s' MSW name `%s'", filename, MSWname);
//		wincancel(debugstr);		// debugging only
		return -1;	// 2000 July 2
	}

/*	In font menu listed as Wingdings, in Registry listed as WingDings */
/*	if (ttfflag && strcmp(MSWname, "WingDings") == 0)
		MSWname="Wingdings"; */	/* special case kludge - reinstated 96/Nov/30 */
	for (k = 0; k < fnext; k++) {
		if (fontname[k] == NULL) continue;			/* sanity check */
		if (subfontname[k] != NULL) continue;		/* already dealt with */
/*		not clear the following shortcut saves much ... cost of alias() ? */
/*		is this is another instance of some base font ? */ /* 96/July/23 */
		if (bUseBaseFont) fnbase = basefont[k];
		else fnbase = k;
		if (fnbase != k) {
/*			if basefont has been dealt with already, just copy information */
			if (subfontname[fnbase] != NULL) {
/*				if (subfontname[k] != NULL) checkfontname(k); */ /* BUG */
				if (subfontname[k] != NULL) checksubfontname(k);
				subfontname[k] = zstrdup(subfontname[fnbase]);
				fontbold[k] = fontbold[fnbase];
				fontitalic[k] = fontitalic[fnbase];
				fontttf[k] = fontttf[fnbase];
				ansifont[k] = ansifont[fnbase];	/* 1999/Jan/8 ? not set yet */
			}
			continue;		/* in any case, no need to look at this more */
		}	/* new 1996/July/23 */
/*		if (strcmp(fontname[k], filename) == 0) */
		if (aliasflag) {
			fname = alias(fontname[k]);				/* 1994/Mar/18 */
			if (fname == NULL) continue;			/* ignore if no alias */
		}
		else fname = fontname[k];					/* TeX TFM file name */
//		if (strncmp(fname, "Ocean", 5) == 0) {
//			sprintf(debugstr, "fname[%d] `%s' FileName `%s'", k, fname, filename);
//			if (wincancel(debugstr)) return 0;	// debugging only
//		}
/*		if (strcmp(fname, filename) == 0 || */
/*		Account for possible TFM name truncation in DOS 95/Aug/26 */
/*		Provide for case mismatch 97/Oct/1 */
/*		if ((bAllowTruncate && strncmp(fname, filename, 8) == 0) || */
		if ((bAllowTruncate && _strnicmp(fname, filename, 8) == 0) ||
/*			(!bAllowTruncate && strcmp(fname, filename) == 0)) */
			(! bAllowTruncate && _stricmp(fname, filename) == 0)) {
			if (subfontname[k] != NULL) checksubfontname(k);	/* BUG */
			subfontname[k] = zstrdup(MSWname);
			fontbold[k] = (char) boldflag;
			fontitalic[k] = (char) italicflag;
			fontttf[k] = (char) ttfflag;
/*			count++; */
/*			break; */ /* NO! TeX TFM file name may occur more than once */
		}
		else count++;					/* count those not yet resolved */
	}
	return count;
}

/* special case hack for TrueType fonts to see whether text or `snowflake' */

/* actually not needed, because DEFAULT_CHARSET can be used in CreateFont */

/* BOOL decorative(char *name) */
int decorative(char *name) {
	int flag=0;
	if (_stricmp(name, "WINGDING") == 0 ||
		_stricmp(name, "LARROWS") == 0 ||
		_stricmp(name, "LICONS") == 0 ||
		_stricmp(name, "LSTARS") == 0 ||
		_strnicmp(name, "LMATH", 5) == 0) flag = -1;
/*	sprintf(str, "%s %d", name, flag);	winerror(str); */
	return flag;
}

/* from winfonts.c */	/* 1995/July/8 */

#define LF_FULLFACESIZE 64

/* parse  key - value  pair from ATM.INI */	/* ttfflag == 0 */
/* Symbol,BOLDITALIC	c:\psfonts\sybi____.pfm,c:\psfonts\sybi____.pfb */

/* ATM.INI has FaceName and Style */

/* parse  key - value  pair from WIN.INI */	/* ttfflag == 1 */
/* Times New Roman Bold Italic (TrueType)	TIMESBI.FOT */

/* WIN.INI [TTFonts] has FullName ? */
/* WIN.INI [Fonts] has mangled FontName ? */

int parsekeyvalue(char *key, char *value, int ttfflag) { 
//	char filename[MAXTEXNAME];		/* name used in TeX => assumed file	name */
	char filename[FNAMELEN];		/* name used in TeX => assumed file	name */
/*	char MSWname[MAXFACENAME]; */	/* MS Windows Face Name - for rendering */
	char MSWname[MAXFULLNAME];		/* LF_FULLFACESIZE just to be safe */

	int boldflag=0, italicflag=0, parsedflag=0;
	int flag;
	unsigned int namelen, mswlen;
	char *s, *equ, *com, *per, *sta, *bak, *sec;
	
/*	sec = strchr(value, '\n'); */					/* redundant */
/*	if (sec == NULL) */
	sec = value + strlen(value);

/*	first pick off filename from value --- after ',' in case of ATM.INI */
	sta = value;									/* beginning of field */
/*	if (ttfflag == 0) if ((com = strchr(value, ',')) != NULL) sta = com+1; */
	
/*	if ((per = strchr(sta, '.')) != NULL) */
	if ((per = strrchr(sta, '.')) != NULL) {		/* file extension */
		if ((bak = strchr(sta, ':')) != NULL &&	bak < per) 
			sta = bak+1;
/*		while ((bak = strchr(sta+1, '\\')) != NULL && bak < per) */
/*		should this be  strrchr(sta, ...) for case c:\myfont.fot ? */
		if ((bak = strrchr(sta+1, '\\')) != NULL && bak < per) 
			sta = bak+1;
/*		namelen = (unsigned int) (per - sta - 1); */
		namelen = (unsigned int) (per - sta);
//		if (namelen >= MAXTEXNAME) namelen = MAXTEXNAME-1;
		if (namelen >= sizeof(filename)) namelen = sizeof(filename) - 1;
/*		strncpy(filename, sta + 1,  namelen); */
		strncpy(filename, sta, namelen);
		filename[namelen] = '\0';
		
/* Heuristic for dealing with stupid file name modifications of FOT files */
/* This only deals with case where last character is modified, */
/* and then only if underscores precede it */	/* 1995/July/4 */
/* This will miss the case of a font with original 7 character name */
		if (ttfflag && strlen(filename) == 8) {
			s = filename + 7;
/* flush even hexadecimal (?) number at end if preceded by underscores */
/* the call to removeunderscores then gets rid of underscores */
			if (*(s-1) == '_') {
				if (*s >= '0' && *s <= '9') *s = '_';
				else if (*s >= 'A' && *s <= 'F') *s = '_';
				else if (*s >= 'a' && *s <= 'f') *s = '_';
			}
		}
/*		standardize filename - all uppercase and no trailing underscores */
		removeunderscores(filename);
/*		if (bUpperCase) */ 				/* normalize file names */
			makeuppercase (filename);	/*	AnsiUpper(filename); ? No */

/*		equ = strchr(key, '=');	*/							/* redundant */
/*		if (equ == NULL) */
		equ = key + strlen(key);	/* in case no = */

/*		pick up Windows Face name and style (bold & italic) flags */
		if (ttfflag == 0) {							/* ATM.INI version */
			if ((com = strchr(key, ',')) != NULL) {	
				if (com < equ) {
					if (strncmp(com+1, "BOLDITALIC", 10) == 0) {
						boldflag = 1;
						italicflag = 1;
					}
					else if (strncmp(com+1, "ITALIC", 6) == 0) 
						italicflag = 1;
					else if (strncmp(com+1, "BOLD", 4) == 0) 
						boldflag = 1;
					equ = com;
				}
			}
		}
		else {										/* WIN.INI version */
/*			if ((com = strstr(key, " (TrueType)")) != NULL) */
/*			should we try and get FaceName from FullName ? */
/*			if (bTTUsable && bTryEnum) {		
				if ((s = strrchr(key, '(')) != NULL) {
					while (s > key && *(s-1) == ' ') s--;
					strncpy(szFullName, key, (s - key));
					*(szFullName + (s - key)) = '\0';
					FaceFromFull(hDC, szFullName);
					strcpy(key, szFaceName);	
					equ = key + strlen(key);	
					boldflag = bBoldFlag;
					italicflag = bItalicFlag;
				}
			} */
			if ((com = strchr(key, '(')) != NULL) {
				while (com > key && *(com-1) == ' ') com--;
				equ = com;
				com = strchr(key, ' ');				/* first space */ 
				if (com != NULL) {
				if ((s = strstr(com, " Medium")) != NULL ||
					(s = strstr(com, " Regular")) != NULL) {
					italicflag = 0;
					if (s < equ) equ = s;
				}
				if ((s = strstr(com, " Italic")) != NULL ||
					(s = strstr(com, " Oblique")) != NULL) {
					italicflag = 1;
					if (s < equ) equ = s;
				}
				if ((s = strstr(com, " Bold")) != NULL ||
					(s = strstr(com, " Demibold")) != NULL) {
					boldflag = 1;
					if (s < equ) equ = s;
				}
				}
			} /* end of strchr for ( is non NULL */
			while (equ > key && *(equ-1) == ' ') equ--;
/*			non-zero for TT, negative if decorative */
/*			following won't work right cause name not terminated ! */
			if (decorative(key) != 0) ttfflag = -1;
/*			else ttfflag = 1; */ /* redundant */
		} /* end of WIN.INI part for TT fonts */

		mswlen = (unsigned int) (equ - key);
/*		if (mswlen >= MAXFACENAME) mswlen = MAXFACENAME -1; */
		if (mswlen >= sizeof(MSWname)) mswlen = sizeof(MSWname) - 1;
		strncpy(MSWname, key, mswlen);
		MSWname[mswlen] = '\0';
#ifdef DEBUGFONTSEARCH
		if (bDebug > 1) {
			sprintf(debugstr, "File: %s Face: %s,%s%s%s", 
					filename, MSWname,
					(!boldflag && !italicflag) ? "REGULAR" : "",
					boldflag ? "BOLD" : "",
					italicflag ? "ITALIC" : "");
			OutputDebugString(debugstr); 
		}
#endif
		flag = InsertSub(filename, MSWname, boldflag, italicflag, ttfflag); 
		parsedflag = -1;
	}
	return parsedflag;
}

/* In Window 3, read ATM.INI, WIN.INI directly for speed - */
/* In Windows NT should instead use only GetPrivateProfileString */
/* NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */
/* Following is the version for Windows 3 */
/* parse a line from the *.INI file */

/* int parseatmline(char *str) */
int parseiniline(char *str, int ttfflag) { 
	char *equ, *ret, *val, *com;

	if ((equ = strchr(str, '=')) == NULL) return 0;		/* must have an '=' */
	*equ = '\0';										/* 1995/Feb/3 */
	val = equ+1;										/* 1995/Feb/3 */
	if ((ret = strchr(val, '\n')) != NULL) *ret = '\0';	/* 1995/Feb/3 */
	if (ttfflag == 0) {	/* in ATM.INI, pick PFB part not PFM, if given */
		if ((com = strchr(val, ',')) != NULL) val=com+1;	/* 1995/Feb/3 */
	}
	return parsekeyvalue (str, val, ttfflag); 
/*	return parsekeyvalue (hDC, str, val, ttfflag); */
} 

/* Watch out: font names may contain spaces */

/* Read the ATM.INI looking for mapping filename => MS Windows font name */
/* This assumes names in fontname[k] have been standardized (uc and no _) */
/* This code sets fontbold[ff] and fontitalic[ff] flags */

/* NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */

/* above removed */

/* Read WIN.INI looking for mapping filename => MS Windows font name */
/* This assumes names in fontname[k] have been standardized (uc and no _) */
/* This code sets fontbold[ff] and fontitalic[ff] flags */
/* This part takes care of TrueType fonts if any */
/* This now allows [TTFonts] section instead of [Fonts] section */
/* This would be a user defined section protected from Windows 95 boot */

/* int parsewinini(FILE *input, char *ininame, int ttfflag) */
/* int parseinifile(FILE *input, char *ininame, int ttfflag) */
/* returns -1 if it fails ? */

#ifdef LONGNAMES
int parseinifile(HFILE input, char *ininame, int ttfflag,
			char *szSection) 
#else
int parseinifile(FILE *input, char *ininame, int ttfflag,
			char *szSection)  
#endif
{
	char *s;									/* 1995/August/1 */

	if (bDebug > 1) {
/*		sprintf(buffer, "Parsing [%s] in %s %s\n", */
		sprintf(debugstr, "Parsing %s in %s %s\n",
			   szSection, ininame, ttfflag ? "(TT)" : "");
		OutputDebugString(debugstr);
	}

/*	if (fgets(str, BUFLEN, input) == NULL) */		/* prime the pump */
	if (fgets(str, sizeof(str), input) == NULL) {	/* prime the pump */
		sprintf(str, "Premature EOF in %s", ininame);
		winerror(str);		/* unlikely */
		return -1;			/* EOF */
	}

/*	makeuppercase(str); */
/*	while(strncmp(str, "[FONTS]", 7) != 0) */ /* scan up to [Fonts] */
	while(_strnicmp(str, szSection, strlen(szSection)) != 0) {
		/* scan up to [Fonts] (or [TTFonts]) */
/*		if (fgets(str, BUFLEN, input) == NULL) */
		if (fgets(str, sizeof(str), input) == NULL) {
			if (bDebug) {
				sprintf(debugstr, "No %s section in %s\n", szSection, ininame);
				if (bDebug > 1) OutputDebugString(debugstr);
				else winerror(str);		/* ignore ? */
			}
			return -1;			/* EOF */
		}
/*		makeuppercase(str);	 */
	}
/*	Now have found [Fonts] (or [TTFonts]) */
/*	if (fgets(str, BUFLEN, input) == NULL) */		/* prime the pump */
	if (fgets(str, sizeof(str), input) == NULL) {	/* prime the pump */
		sprintf(str, "Premature EOF in %s", ininame);
		winerror(str);		/* unlikely */
		return -1;			/* EOF */
	}
/*	Scan until we hit the next section or EOF */
	while(strchr(str, '[') == NULL && strchr(str, ']') == NULL) {
/*		ignore blank lines and comments */
/*		if (*str < ' ' || *str == ';') */
		if (*str > ' ' && *str != ';') { 
			if (ttfflag == 0 || strstr(str, "TrueType") != NULL) {
/*	remove all " (quotebl) in line 95/Aug/1 */
/*	firstly, value string may be quoted using "..." */
/*	secondly, we want to be able to parse saved registry files ... */
				while ((s = strchr(str, '\"')) != NULL) strcpy(s, s+1);
				if (parseiniline(str, ttfflag) == 0) winerror(str); 
			}
		}
/*		if (fgets(str, BUFLEN, input) == NULL) return 0; */ 		/* EOF */
		if (fgets(str, sizeof(str), input) == NULL) return 0; 		/* EOF */
	}
/*	Now have hit end of [Fonts] (or [TTFonts]) section */
	return 0;		/* end of [Fonts] section - don't need the rest */
} 

/* Read ATM file and use to fill in subfontname[fn], fontbold & fontitalic */
/* returns -1 if it can't find the file */

/* int readatminisub(void) */
int ReadIniSub (char *ininame, int ttfflag) {  
	char inifilename[MAXFILENAME];
#ifdef LONGNAMES
	HFILE input;
#else
	FILE *input;
#endif
	int flag;
	char *s;

	strcpy(inifilename, szWinDir);				
	s = szWinDir + strlen(szWinDir) - 1;			/* 1993/Dec/21 */
	if (*s != '\\' && *s != '/') strcat(inifilename, "\\");
	strcat(inifilename, ininame);
#ifdef DEBUGINI
	if (bDebug > 1) OutputDebugString(inifilename);
#endif
	input = fopen(inifilename, "r");

	if (input == BAD_FILE) {
		_searchenv(ininame, "PATH", inifilename);		/* 1995/Feb/1 */

		input = fopen(inifilename, "r");

		if (input == BAD_FILE) {
			if (ttfflag) {
				sprintf(str, "Can't find %s", ininame);  
				winerror(str); 		/* not an error anymore ? (cause of TT) */
			}
			return -1;
		}
	}
/*	flag = parseatmini(input); */
/*	flag = parseinifile(input, "atm.ini", ttfflag); */
/*	flag = parseinifile(input, ininame, ttfflag); */		/* 1995/April/11 */
/*	flag = parseinifile(input, ininame, ttfflag, "[Fonts]"); */
/*	Note: [TTFonts] via GetProfile from win.ini has FullNames */
	if (ttfflag && bTTFontSection) {
		flag = parseinifile(input, ininame, ttfflag, "[TTFonts]");
#ifdef LONGNAMES
		_llseek(input ,0, 0);
#else
		rewind(input);
/*		fseek(input, 0, SEEK_SET); */
#endif
	}
/*	Note: [Fonts] via GetProfile from registry has mangled FontNames */
	flag = parseinifile(input, ininame, ttfflag, "[Fonts]");
	fclose(input);
/*	if (flag < 0) {
		sprintf(str, "Parse error in %s", ininame);
		winerror(str);
	} */
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following is the version for Windows 3 */

/* Parse a line from the WIN.INI file - format: */
/* Times New Roman Bold Italic (TrueType)=TIMESBI.FOT */
/* Times New Roman Bold Italic (TrueType)=N:\WINDOWS\SYSTEM\TIMESBI.FOT */
/* CMMI10 (TrueType)=CMMI10__.FOT */
/* Should `Bold' and `Italic' be part of font name or stripped off ? */

/* flushed following to use common code 95/Feb/3 */

/* read win.ini file and to fill in subfontname[fn], fontbold & fontitalic */

/* Following reworked for Windows NT to avoid direct read of ATM.INI */
/* New version that avoids directly accessing ATM.INI */
/* Slower, uses GetPrivateProfile */ /* expects [Fonts] section */
/* NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */

/* read INI file and use to fill in subfontname[fn] */
/* returns zero if if fails ? */

/* Of course this gets either (i) Full Names from [TTFonts] in win.ini */
/* or (ii) Mangled FontNames from [Fonts] i.e. registry ... ugh */

int GetCommonIni (char *achFile, char *achPr, int ttfflag) { 
	char *keys=NULL;
	char *s;
	int keylen, pairlen;
	int buffersize = 1024;
/*	HLOCAL hmem; */		/* flushed 1995/July/27 */

	for(;;) {
/*		hmem = LocalAlloc(LHND, (WORD) buffersize); */

/*		UINT is 16 bit in WIN16 and 32 bit in WIN32 */
		keys = (char *) LocalAlloc(LPTR, (UINT) buffersize);
		if (keys == NULL) {
			winerror ("Unable to allocate memory");
/*			return -1; */					/* 95/Jan/17 */
			PostQuitMessage(0); 			/* 95/July/27 ??? */
		}
/*		keys = (char *) LocalLock (hmem);*/
/*		if (keys == NULL) {
			winerror ("Unable to allocate memory");
			PostQuitMessage(0); 
		} */
		keylen = GetPrivateProfileString(achPr, NULL, "", 
			keys, buffersize, achFile);
		if (keylen < buffersize - 2) break;	/* do keys fit in buffer ? */
		buffersize += 1024;					/* NO: try larger buffer ! */
/*		if (LocalUnlock(hmem) != 0) {
			winerror("Lock count not zero");
			return -1;
		} */
/*		hmem = LocalFree(hmem); */
		keys = (char *) LocalFree(keys);
	}	/* end of for(;;) trying to allocate large enough key buffer */

	if (bDebug > 1) {
		sprintf(debugstr, "GetPrivateProfile %s [%s] TTF %d keylen %d\n",
				achFile, achPr, ttfflag, keylen);
		OutputDebugString(debugstr);
	}					/* debugging 1995/March/15 */

	if (keylen == 0) {
/*		if (ttfflag && strcmp(achPr, "TTFonts") == 0) */
		if (ttfflag && _stricmp(achPr, "TTFonts") == 0) {
/*		ignore if we are looking for [TTFonts] - it may not exist */
		}
		else {
			if (bDebug) {
				sprintf(debugstr, "Failed to find %s keys in %s\n", achPr, achFile);
				if (bDebug > 1) OutputDebugString(debugstr);
				else winerror(debugstr);
			}
		}
	}
	else {
		s = keys;
		while (s - keys < keylen) {
			pairlen = GetPrivateProfileString(achPr, s, "", 
/*				str, BUFLEN, achFile); */
				str, sizeof(str), achFile);
/*			winerror(str); */
			if (pairlen == 0) {
				winerror(str);
			}
			if (parsekeyvalue(s, str, ttfflag) == 0) { 
				winerror(str);	/* blank line ? */
			}
			s += lstrlen(s) + 1;				/* go to next key */
		}
	}
/*	if (LocalUnlock(hmem) != 0)	winerror("Lock count not zero"); */
/*	hmem = LocalFree(hmem); */					/* Free memory again */ 
	keys = (char *) LocalFree(keys);			/* 1995/July/27 */
	return keylen;
}

/* #endif */

/* If bUseGetProfileATM is set (not default) use GetProfileString on ATM.INI */
/* If bUseGetProfileTT is set (default) use GetProfileString on WIN.INI */
/* Otherwise we actually open and read the INI file */
/* GetProfileString is much slower in Windows 3.1, but needed in Windows NT */
/* because there is no [Fonts] section in WIN.INI and */
/* GetProfile fakes this based in registry entries */

int ReadIniFile(char *ininame, int ttfflag) { 
	int flag;
/*	ATM.INI ? */
	if (bUseGetProfileATM && !ttfflag) {
		flag = GetCommonIni (ininame, "Fonts", ttfflag);	/* atm.ini */
		return flag;
	}
/*	WIN.INI ? */
	else if (bUseGetProfileTT && ttfflag) {
/*		flag = GetCommonIni (ininame, "Fonts", ttfflag); */
		if (bTTFontSection)
			flag = GetCommonIni (ininame, "TTFonts", ttfflag);
		flag = GetCommonIni (ininame, "Fonts", ttfflag);
		return flag;
	}
/*	don't use GetProfile ... */
	else return ReadIniSub (ininame, ttfflag); 
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int realfontnumber (int k) {	/* find original TeX font number */
	int ff;
	for (ff = 0; ff < MAXFONTNUMBERS; ff++) {
		if (finx[ff] ==	(short) k) break;		/* 96/July/25 */
	}
	return ff;
}

// Base13 explicitly listed in case problem with substitution

char *Base13[][3]={
	{"com", "Courier", ""},
	{"cob", "", "B"},
	{"coo", "", "I"},	
	{"cobo", "", "BI"},	
	{"hv", "Helvetica", ""},
	{"hvb", "", "B"},
	{"hvo", "", "I"},	
	{"hvbo", "", "BI"},	
	{"ti", "Times", ""},
	{"tib", "", "B"},
	{"tii", "", "I"},	
	{"tibi", "", "BI"},	
	{"sy", "Symbol", ""},
	{"zd", "ZapfDingbats", ""},
	{"", "", ""}
};

// The following deals with conflicts due to Windows substituting
// Mostly only needed for com => Courier as result of Courier FON font

int InsertBase13 (void) {
	int i, k, count=0;
	char *fname;
	char *sfname=NULL;
	
	for (i = 0; i < fnext; i++) {
		if (fontname[i] == NULL) continue;		/* sanity check */
		if (subfontname[i] != NULL)  continue;
		if (fontname[i] != NULL) fname = fontname[i];
		else fname = "UNKNOWN";
		for (k = 0; k < 16; k++) {
			if (strcmp(Base13[k][0], "") == 0) break;
			if (strcmp(Base13[k][1], "") != 0) sfname = Base13[k][1];
			if (_stricmp(Base13[k][0], fname) == 0) {
				subfontname[i] = zstrdup(sfname);
				if (strchr(Base13[k][2], 'B') != NULL) fontbold[i] = 1;
				else fontbold[i] = 0;
				if (strchr(Base13[k][2], 'I') != NULL) fontitalic[i] = 1;
				else fontitalic[i] = 0;
				fontttf[i] = 0;
				count++;
				if (bDebug > 1) {
					sprintf(debugstr, "fname %s fsname %s B %d I %d",
						fontname[i], subfontname[i], fontbold[i], fontitalic[i]);
					OutputDebugString(debugstr);
//					wincancel(debugstr);	// debugging only
				}
			}
		}
	}
	return count;
}

/* Check whether all fonts found have been mapped scan */
/* verboseflag added for debugging purposes 1995/August/1 */

int missingfonts (int verboseflag) {
	int k, ff, missed=0;
	char *fname;

	for (k = 0; k < fnext; k++) {
		if (fontname[k] == NULL) continue;		/* sanity check */
		if (fontname[k] != NULL) fname = fontname[k];
		else fname = "UNKNOWN";
		if (subfontname[k] == NULL) {
			if (verboseflag && bDebug > 1) {
				ff = realfontnumber(k);
				wsprintf(debugstr, "%d %s remains unknown\n",
						 ff, fname);
				OutputDebugString(debugstr);				
			}
			missed++;
		}
/*		could skip out early, since all that matters is whether *any* missed */
	}
#ifdef DEBUGFONTSEARCH
	if (missed > 0) {
		if (bDebug > 1) {
			sprintf(debugstr, "%d out of %d still missing\n", missed, fnext);
			OutputDebugString(debugstr);
		}
	}
#endif
	return missed;
}

/* Used for dviwindo.fnt files */
/* shouldn't this be done same way as WIN.INI & ATM.INI ??? */

/* look for lines form: NewCenturySchoolBook,BOLD,ITALIC=ncbi */
/* look for lines form: NewCenturySchoolBook,BOLD,ITALIC=c:\psfonts\ncbi.pfm */
/* and ignore line [FontMapping] and comment lines starting with `;' */

/* revised 1994/Feb/17 to deal with Face Names that include `space' */

/* returns 0 if nothing more to do 98/May/28 */

#ifdef LONGNAMES
int scansubfile(HFILE subfile) 
#else
int scansubfile(FILE *subfile) 
#endif
{
	int c;
	int flag = 1;				/* 98/Mar/26 noninit ??? */
	int boldflag, italicflag;
	char *s, *t, *u;

/*	while (fgets(str, BUFLEN, subfile) != NULL) */
	while (fgets(str, sizeof(str), subfile) != NULL) {
/*  ignore blank lines, comment lines and [Font] section headers */
		if (*str == ';' || *str == '%' || 
			*str == '[' || *str == '\n') continue;
		s = str;
/*		hmm, why not use s = strchr(str, '=') ?? */
/*		while ((c = *(++s)) != '=' && c > ' ') ; */	/* search for `=' */
		while ((c = *(++s)) != '=' && c >= ' ') ; 	/* search for `=' */
/*		if (*s <= ' ') continue; */			/* only one item on line ? */
		if (*s < ' ') continue; 			/* only one item on line ? */
		*s++ = '\0';						/* terminate MS face name */
											/* --- and step over it */
		boldflag = 0; italicflag = 0;		/* bold and/or italic ? */
		if ((u = strchr(str, ',')) != NULL) {
			*u++ = '\0';					/* isolate font name */
			if (strncmp(u, "BOLDITALIC", 10) == 0) {
				boldflag = 1; italicflag = 1;
			}
			else if (strncmp(u, "BOLD", 4) == 0) {
				boldflag = 1;
			}
			else if (strncmp(u, "ITALIC", 6) == 0) {
				italicflag = 1;
			}
		}
/* no comma, look for last non white-space */
/*		else {	
			u = str + strlen(str);
			while (*(--u) < ' ') ;
			*(u+1) = '\0';
		} */
/*	strip off path from file name (which is pointed to by s), if any */
		if ((t = strrchr(s, '\\')) != NULL) t++;
		else if ((t = strrchr(s, '/')) != NULL) t++;
		else if ((t = strrchr(s, ':')) != NULL) t++;
		else t = s;
/* strip off file extension if any */
		if ((u = strrchr(t, '.')) != NULL) *u = '\0';
/*		u = str + strlen(str); */
		u = t + strlen(t);
/*		while (*(--u) <= ' ') ;	*/	/* stip off trailing white space */
		while (*(--u) <= ' ' && u > t) ; /* stip off trailing white space */
		*(u+1) = '\0';
/*		if (bUpperCase) */
			makeuppercase(t);	/* uppercase file name for comparison */
		if (bDebug > 1) {
			char buff[64];
			sprintf(buff, "%s\t%s,%s%s%s\n", t, str,
				   (!boldflag && !italicflag) ? "REGULAR" : "",
				   boldflag ? "BOLD" : "",
				   italicflag ? "ITALIC" : "");
			OutputDebugString(buff);
		} 
		flag = InsertSub(t, str, boldflag, italicflag, 0);
		if (flag == 0) return 0;		/* early exit 96/May/28 */
	}
/*	fclose(subfile); */		/* moved out 1994/Sep/2 */
	return flag;			/* there are more font names to find */
}

/* change extension if present 
void forceexten(char *fname, char *str) { 
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);	
} */

/* use font substitution file if any fonts were missing */
/* this accesses the version that came with the DVI file */
/* <file>.DVI  ==>  <file>.FNT */
/* Then tries for dviwindo.fnt in the DVI file's directory ??? */
/* returns -1 if it failed to find either of these files */

int readdviini (void) {
	char filename[MAXFILENAME];
#ifdef LONGNAMES
	HFILE subfile;
#else
	FILE *subfile;
#endif

/*	We are assuming here that the file is in the current directory ! */
	strcpy(filename, DefPath);		/* figure out directory of current file */
	strcat(filename, OpenName);
	forceexten(filename, "fnt");		/* NOT "sub" */

	subfile = fopen(filename, "r");

	if (subfile != BAD_FILE) {
		if (bDebug > 1) {
			sprintf(debugstr, "Scanning %s\n", filename);
			OutputDebugString(debugstr);
		}
		(void) scansubfile(subfile);
		fclose(subfile);				/* 1994/Sep/2 */
		return 0;
	}
/*	added 1992/July/21 */
	strcpy(filename, DefPath);		/* figure out directory of current file */
	strcat(filename, "dviwindo.fnt");	/* NOT ".sub" */

	subfile = fopen(filename, "r");

	if (subfile != BAD_FILE) {
		if (bDebug > 1) {
			sprintf(debugstr, "Scanning %s\n", filename);
			OutputDebugString(debugstr);
		}
		(void) scansubfile(subfile);
		fclose(subfile);				/* 1994/Sep/2 */
		return 0;
	}
	return -1;							/* did not find either file */
}

/* use font substitution file if any fonts were missing */
/* this accesses the generic version in DVIWindo directory */
/* returns -1 if it fails to find the file */

int readsubini(void) {
	char filename[MAXFILENAME];
#ifdef LONGNAMES
	HFILE subfile;
#else
	FILE *subfile;
#endif

	strcpy(filename, szExeWindo);			/* get path of executable */
	strcat(filename, "dviwindo.fnt");	/* NOT ".sub" */
/*	if (bDebug) winerror(filename); */

	subfile = fopen(filename, "r");

	if (subfile == BAD_FILE) return -1;	/* failed to find the file */
	if (bDebug > 1) {				/* 1995/Dec/1 */
		sprintf(debugstr, "Scanning %s\n", filename);
		OutputDebugString(debugstr);
	}
	(void) scansubfile(subfile);
	fclose(subfile);				/* 1994/Sep/2 */
	return 0;
}
	
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*	Try and cope with mismatch in last character of name of TT font file */
/*	We already deal elsewhere with case of name < 7 characters _ */
/*	Which is easy because then we have an _ before that last digit */
/*	This here allows match if szFileName has length 8 and matches fname */
/*  in all but final character, and that is a digit */

int MatchVersion (char *fname, char *szFileName) {	/* 1995/July/30 */
	int c;
	if (strlen(szFileName) != 8) return 0;
	if (strlen(fname) < 7) return 0;
/*	if (strncmp(fname, szFileName, 7) != 0) return 0; */ /* ??? */
	if (_strnicmp(fname, szFileName, 7) != 0) return 0;
	c = *(szFileName+7);
	if (c >= '0' && c <= '9') return 1;
/*	if (c >= 'A' && c <= 'F') return 1; */
/*	if (c >= 'a' && c <= 'f') return 1; */
	return 0;
}

/* ntmFlags field flags */

/* Windows 3.1 New Text DWORD ntmField flags */

#define NTM_REGULAR	0x00000040L
#define NTM_BOLD	0x00000020L
#define NTM_ITALIC	0x00000001L

int ApplyTTMapping (void) {
	LPSTR lpFileCurrent;
	int k, m;
	char *fname;
	int count = 0;
	char szFileName[MAXTFMNAME+2];
	int nStyle;
	int nDone;
	
	if (bDebug > 1) OutputDebugString("Apply TT Mapping\n");
	lpFileNames = GrabFileNames();
	if (lpFileNames == NULL) return -1;
/*	Step trough all TrueType Faces, Styles known to Windows */
	for (m = 0; m < nFileIndex; m++) {
		lpFileCurrent = lpFileNames + m * (LF_FACESIZE + 2 + MAXTFMNAME + 2);
		lstrcpy(szFaceName, lpFileCurrent);
		nDone = *(lpFileCurrent + LF_FACESIZE);				/* 98/Nov/20 */
		nStyle = *(lpFileCurrent + LF_FACESIZE + 1);
		lstrcpy(szFileName, lpFileCurrent + LF_FACESIZE + 2);
/*		Step through all TFM font file names used in job */
		for (k = 0; k < fnext; k++) {
			if (fontname[k] == NULL) continue;
			if (subfontname[k] != NULL) continue; /* already dealt with */
			if (aliasflag) {
				fname = alias(fontname[k]);	
				if (fname == NULL) continue;		 /* ignore if no alias */
			}
			else fname = fontname[k];				/* TeX TFM file name */
/* should this be case insensitive ? or did we already `normalize' ? */
/*			if (strcmp(fname, szFileName) == 0 || */ /* ??? */
			if (_stricmp(fname, szFileName) == 0 || 
				(bAllowVersion && MatchVersion (fname, szFileName))) {
/* check strlen(szFaceName) < MAXFONTS ? */
/*				if (subfontname[k] != NULL) checkfontname(k); */ /* BUG */
				if (subfontname[k] != NULL) checksubfontname(k);
				subfontname[k] = zstrdup(szFaceName);
				if (nStyle & NTM_BOLD) fontbold[k] = 1;
				else fontbold[k] = 0;		/* 1999/Jan/8 */
				if (nStyle & NTM_ITALIC) fontitalic[k] = 1;
				else fontitalic[k] = 0;		/* 1999/Jan/8 */
				if (nStyle & NTM_REGULAR)
					fontbold[k] = fontitalic[k] = 0;
#if DEBUGFONTSEARCH
				if (bDebug > 1) {
					sprintf(debugstr, "%s (%d) => %s,%s%s%s (%d)\n",
							fname, k, szFaceName,
							(!fontbold[k] && !fontitalic[k]) ? "REGULAR" : "",
							fontbold[k] ? "BOLD" : "",
							fontitalic[k] ? "ITALIC" : "",
							m);
					OutputDebugString(debugstr);
				}
#endif
/* The following assumes no Type 1 fonts will be listed here ? */
				if (nDone == TTF_DONE) fontttf[k] = 1;	/* ??? 98/Nov/20 */
				else fontttf[k] = 0;					/* ??? 98/Nov/20 */
				count++;
/*				break; */ /* NO: TeX TFM file name may occur more than once */
			}
		}
	}
	ReleaseFileNames();
	if (bDebug > 1) {
		sprintf(debugstr, "Matched %d font file name%s",
				count, (count == 1) ? "" : "s");
		OutputDebugString(debugstr);
	}
	return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// EXE_ERROR strings no longer exist...

/* In Windows 95, make the following WinExec call to set up TT font data*/
/* Insert Windows directory explicitly in /E filename ? */
/* or switch to Windows directory first ? */

/* regedit /E c:\windows.000\ttfonts.reg
   HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Fonts */

/* "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts"; */

/* default "ttfonts.reg" */

int WriteRegFile (char *szRegistry) {
	char *s;
	int flag;
/*	int err; */
	HINSTANCE err;
	
	if (bRegistrySetup != 0) return 0;		/* already set up */
	if (bUseRegistryFile == 0) return -1;	/* tried and failed before */
/*    if (bWin95 == 0) */		/* only to any of this in Windows 95 */
    if (bNewShell == 0) {		/* changed 96/Oct/2 */
		bUseRegistryFile = 0;	/* stop this nonsense */
		return -1;				/* only makes sense with New Shell */
	}

	if (szRegistry == NULL ||
		*szRegistry == '\0' ||
		strcmp(szRegistry, "nul") == 0 ||
		strcmp(szRegistry, "win.ini") == 0) {
		bUseRegistryFile = 0;				/* to avoid calling again */
		return -1;
	}

/*	if we get here, szRegistry is defined */
#ifdef DEBUGREGISTRY
	if (bDebug > 1) OutputDebugString("Calling RegEdit\n");
#endif
/*	Need to flush the cache for ttfonts.reg */
/*	If we have just accessed it, and if RegEdit will overwrite it */
	WritePrivateProfileString(NULL, NULL, NULL, szRegistry);

	if (szRegistryFonts == NULL) {	// modified 2000 June 2
		ConstructFontKey(str);
		szRegistryFonts = zstrdup(str);
	}

/*	if (bWinNT) strcpy(str, "regedt32 /E ");
	else strcpy(str, "regedit /E "); */  /* In Windows NT ??? 96/Oct/2 */
	strcpy(str, "regedit /E ");
    strcat(str, szWinDir);    /* from GetWindowsDirectory - in dviwindo.c */
	s = szWinDir + strlen(szWinDir) - 1;		/* 1993/Dec/21 */
	if (*s != '\\' && *s != '/') strcat(str, "\\");
    strcat(str, szRegistry);	/* or use "ttfonts.reg" */
	strcat(str, " ");
    strcat(str, szRegistryFonts);		/* should be set up winfonts.c */
    if (bDebug || bShowCalls) {
		flag = MaybeShowStr(str, "RegEdit");
		if (flag == 0) return -1;		/* cancelled by user */
	}
/*	new debugging output 98/Jun2/28 */
//	WritePrivateProfileString(achPr, "LastRegEdit", str, achFile);//
	WritePrivateProfileString(achDiag, "LastRegEdit", str, achFile);
    err = (HINSTANCE) WinExec(str, SW_SHOW);

/*	if (err >= MAXDOSERROR) */	/* try and keep busy for a little while ! */
	if (err >= HINSTANCE_ERROR) {
/*		(void) GetTaskWindow ((HINSTANCE) err);
		if (bDebug > 1) {
			sprintf(debugstr, "hWnd %0X hTask %0X hInstance %0X err %0X\n",
					TaskInfo.hWnd, TaskInfo.hTask, TaskInfo.hInstance, err);
			OutputDebugString(debugstr);
			winerror(debugstr);
		} */
		bRegistrySetup = 1;			/* no need to do this again */
	}								/* 1995/Aug/18 */
	else { 
/*		char buffer[BUFLEN]; */		/* does it need to be this long? */
		strcpy(debugstr, str);
		strcat(debugstr, "\n");
/*		n = strlen(buffer); */
		s = str + strlen(str);
		flag = LoadString(hInst,
/*						  (UINT) (EXE_ERROR + err), */
						  (EXE_ERROR + (UINT) err),
/*						  debugstr+n, sizeof(debugstr)-n); */
						  s, sizeof(debugstr) - strlen(debugstr));
/*		if (flag == 0) sprintf(buffer+n, "Error %d ", err); */
		if (flag == 0) sprintf(s, "Error %d ", err); 
		strcat(debugstr, " WinExec");
		winerror(debugstr);
		bUseRegistryFile = 0;				/* to avoid calling again */
		return -1;							/* indicate failure */
	}
	return 0;								/* success */
}

/* Should this all be WIN32 only ??? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* New code for reading ATMREG.ATM (Windows 95 ATM 4.0) 96/May/28 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef LONGNAMES
unsigned int xreadtwo (HFILE input) 
#else
unsigned int xreadtwo (FILE *input) 
#endif
{
	unsigned int c, d, n;
	c = wingetc(input);
	d = wingetc(input);
	n = (d << 8) | c; 
	return n;
}

#ifdef LONGNAMES
unsigned long xreadfour(HFILE input) 
#else
unsigned long xreadfour(FILE *input) 
#endif
{
	unsigned int a, b, c, d;
	unsigned long n;
	a = wingetc(input);
	b = wingetc(input);
	c = wingetc(input);
	d = wingetc(input);
	n = (d << 8) | c;
	n = (n << 16) | (b << 8) | a;
	return n;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

BOOL bATM41=FALSE;			// needs to be set based on ATMREG.ATM header

#ifdef LONGNAMES
int ReadString (HFILE input, char *name, int nlen)
#else
int ReadString (FILE *input, char *name, int nlen)
#endif
{
	int c;
	int n=0;
	char *s=name;

	*s = '\0';				/* in case we pop out early */
/*	c = wingetc(input); */	/* always read first byte ... */
/*	*s++ = (char) c; */
/*	n++; */					/* changed 1996/June/4 */
	for (;;) {				/* read string up to zero byte */
		c = wingetc(input);
//		if (bWinNT5)
		if (bATM41)					// 2000 July 3 ATM 4.1 ???
			(void) wingetc(input);	// discard second byte of UNICODE
		if (c == EOF) {		/* EOF */
			*s++ = '\0';
			return -1;
		}
		*s++ = (char) c;
		if (c == 0) break;
		n++;
		if (n >= nlen) {	/* too long */
			if (bDebug > 1) {
				strncpy(debugstr, name, nlen);
				s = debugstr + nlen;
				*s++ = '\n';
				*s++ = '\0';
				OutputDebugString(debugstr);
			}
			*name = '\0';
		    return -1;
		}
	}
	return 0;
}

/* Should this all be WIN32 only ??? */

/* #define LF_FACENAME 32 */
/* #define LF_FULLFACENAME 64 */

/* #define PFMName 16 */		/* >= 8+1+3+1 */
/* #define PFBName 16 */		/* >= 8+1+3+1 */

/* testflag == 0 look through all fonts listed */
/* testflag != 0 check only whether specific font exists */

#ifdef LONGNAMES
/* int ScanAllFonts(HFILE input, unsigned long endfontlist) */
int ScanAllFonts (HFILE input, unsigned long endfontlist, int testflag) 
#else
/* int ScanAllFonts(FILE *input, unsigned long endfontlist) */
int ScanAllFonts (FILE *input, unsigned long endfontlist, int testflag) 
#endif
{
	int c, k, count=0;
	unsigned int stroffset, nlen;
	unsigned long next;
	int boldflag, italicflag;		/* style bits */
	int ttfflag;
/*	following just used for statistics - could remove to save time */
	int psflag, mmmflag, mmiflag, genflag;		/* font type bits */
	int ttfcount=0;					/* number of TT fonts */
	int pscount=0;					/* number of T1 fonts */
	int mmmcount=0;					/* number of MM Master fonts */
	int mmicount=0;					/* number of MM Instances fonts */
	int gencount=0;					/* number of generic MM fonts */
	int total;						/* total number of fonts */
	int nMMM, nPFB, nPFM;			/* index into dir path table */
	unsigned int flag[16];			/* 16 bytes of flags */
	char FaceName[LF_FACESIZE+1];		/* Windows Face Name */
	char StyleName[LF_FACESIZE+1];		/* Style Name for TT font - not used */
	char FullName[LF_FULLFACESIZE+1];	/* Full Name - not used */
	char FontName[LF_FULLFACESIZE+1];	/* Font Name - not used */
	char MMMName[LF_FACESIZE+1];		/* PFM file or TTF file or MMM file */
	char PFBName[LF_FACESIZE+1];		/* PFB file or PSS file - not used */
	char PFMName[LF_FACESIZE+1];		/* PFM file of MMM font - not used */
	char *s;

/*	if (findfontstart(input) < 0) return -1; */

/*	 positioned at start of font list at this point */

	for (;;) {
		c = getc(input);				/* check for end of file 99/Mar/1 */
		if (c == EOF) break;
		ungetc(c, input);
		stroffset = xreadtwo(input);	/* offset to first string == 44 */
		nlen = xreadtwo(input);			/* length of this record in bytes */
		next = xreadfour(input);		/* pointer to next record */
		for (k = 0; k < (28 - 8); k++) (void) wingetc(input);
		for (k = 0; k < 16; k++) flag[k] = wingetc(input);
		boldflag = flag[1];
		if (boldflag == 0 || boldflag > 2) {
			if (boldflag > 2) boldflag = 1;	/* pretend it is OK */
/*			break; */	 /* impossible */	/* `fixed' 97/Sep/14 */
		}
		else boldflag = boldflag - 1;
		italicflag = flag[2];
		if (italicflag > 1) {
/*			break; */	/* impossible */	/* `fixed' 97/Sep/14 */
		}
		ttfflag = psflag = mmmflag = mmiflag = genflag = 0;
/*		ttfflag = flag[5]; */
		if (flag[4] == 0) ttfflag = 1;
		else if (flag[4] == 1) psflag = 1;
		else if (flag[4] == 2) mmmflag = 1;
		else if (flag[4] == 4) mmiflag = 1;
		if (flag[6] == 10) {
			genflag = 1;
			mmmflag = 0;
		}
		nMMM = flag[8] | (flag[9] << 8);	/* index into path name table */
		nPFB = flag[10] | (flag[11] << 8);	/* index into path name table */
		nPFM = flag[12] | (flag[13] << 8);	/* index into path name table */
/*		mmflag = flag[12]; */
		if (ttfflag) ttfcount++;
		else if (genflag) gencount++;
		else if (mmiflag) mmicount++;
		else if (mmmflag) mmmcount++;
		else pscount++;

/*		These used to all continue when they hit trouble */
/*		Windows Face Name */
	    if (ReadString(input, FaceName, sizeof(FaceName)) < 0) goto donext;
/*		Style Name (will be empty string for PS SM or MM font) */
	    if (ReadString(input, StyleName, sizeof(StyleName)) < 0) goto donext;
/*		Full Name  (will be empty string for PS SM or MM font) */
	    if (ReadString(input, FullName, sizeof(FullName)) < 0) goto donext;
/*		Font Name  (may be empty if font file not yet read by ATM) */
		if (ReadString(input, FontName, sizeof(FontName)) < 0) goto donext;
/*		Name of MMM file or PFM file or TTF file */ 
	    if (ReadString(input, MMMName, sizeof(MMMName)) < 0) goto donext;
/*		Name of PFB file or PSS file */ 
	    if (ReadString(input, PFBName, sizeof(PFBName)) < 0) goto donext;
/*		Name of PFM file in case of MMM font */ 
	    if (ReadString(input, PFMName, sizeof(PFMName)) < 0) goto donext;
/*		Flush extension from file name --- MMMName is file name */
		if ((s = strchr(MMMName, '.')) != NULL) *s = '\0';
/*		Remove underscores from file name */
/*		removeunderscores(MMMName); */
		if (! testflag) removeunderscores(MMMName);
/*		Make all uppercase ? It's a file name so its safe at least */
		if (bUpperCase)	makeuppercase (MMMName);
#ifdef DEBUGATM
		if (bDebug > 1) {
//			sprintf(debugstr, "`%s' `%s' `%s%s' %s (%d)\n", MMMName, FaceName,
//					boldflag ? "BOLD" : "",
//					italicflag ? "ITALIC" : "",
//					ttfflag ? "(TT)" : "", pscount);
//			OutputDebugString(debugstr);
			sprintf(debugstr, "Face: `%s' Style: `%s' Full: `%s' Font: `%s' MMM: `%s' PFB: `%s' PFM: `%s'",
					FaceName, StyleName, FullName, FontName, MMMName, PFBName, PFMName);
			OutputDebugString(debugstr);
		}
#endif
//		ATM may not have updated ATMREG.ATM yet --- in this case MMMName may be blank ...
		if (! testflag) {
			if (*MMMName != '\0') {	// based on FileName 2000 July 3
				count = InsertSub(MMMName, FaceName,
								  boldflag, italicflag, ttfflag);
				if (count == 0) {
					if (bDebug < 2)		/* if not in debug mode ... */
						break;			/* ... early exit if all found 98/May/28 */
				}
			}
//			allow for use of FontName instead of FileName in TeX TFM naming !
//			maybe only for MM font instances ? and maybe only with ATMREG.ATM ?
			if (bAllowFontNames && *FontName != '\0') {	// based on FontName 2000 July 4
				count = InsertSub(FontName, FaceName,
								  boldflag, italicflag, ttfflag);
				if (count == 0) {
					if (bDebug < 2)		/* if not in debug mode ... */
						break;			/* ... early exit if all found 98/May/28 */
				}
			}
		}
		else {	/* new use of this function 97/Jan/21 */
/*			if (strcmp(FaceName, TestFont) == 0 && */ /* ??? */
			if (_stricmp(FaceName, TestFont) == 0 &&
				boldflag == bCheckBoldFlag &&
				italicflag == bCheckItalicFlag) {
				if (bUseFontName && *FontName != '\0')
					strcpy(str, FontName);	// use FontName (new)
				else strcpy(str, MMMName);	// use FileName (normal)
				return 0;			/* success */
			}
		}
donext:
/*		if (findfontstart(input) < 0) break; */
		if (next >= endfontlist) break;
		if (winseek(input, next) < 0) break;
	}
//	wincancel("FOO!");	// debugging only
	total = ttfcount + pscount + mmmcount + mmicount + gencount;
#ifdef DEBUGATM
	if (bDebug > 1) {
		sprintf(debugstr,
		"ATMREG: %d TTF + %d T1 + %d MM + %d GEN + %d PSS = %d total fonts\n",
				ttfcount, pscount, mmmcount, gencount, mmicount, total);
		OutputDebugString(debugstr);
//		wincancel(debugstr);		// debugging only
	}
#endif
	*str = '\0';		/* wipe clean again */
	return total;
}

// Read ATM Build String from ATMREG.ATM

#ifdef LONGNAMES
void ReadATMBuild (HFILE input)
#else
void ReadATMBuild (FILE *input)
#endif
{
	char *s=str;
	int c;
	for (;;) {
		c = wingetc(input);
		if (bATM41) (void) wingetc(input);
		if (c <= 0 || c == '<') break;
		*s++ = (char) c;
	}
	*s = '\0';
	WritePrivateProfileString(achDiag, "ATMBuildString", str, achFile);
}

/*	Get start and end of font list, position at start, return end */
/*	Also decide whether wide character strings used (=> ATM41 flag) */

#ifdef LONGNAMES
unsigned long ReadPointers (HFILE input) 
#else
unsigned long ReadPointers (FILE *input) 
#endif
{
	unsigned long startfontlist, endfontlist;
	unsigned long startdirlist, enddirlist, endsetlist;
	int nDirs, nFonts, nSets;

	(void) winseek(input, 6);			/* start of global counts */
	nDirs = xreadtwo(input);			/* 6 number of directory paths */
	nFonts = xreadtwo(input);			/* 8 number of font entries */
	nSets = xreadtwo(input);			/* 10 number of font sets (?) */
//	(void) winseek(input, 12);			/* start of pointers into file */
	enddirlist = xreadfour(input);		/* 12 enddirlist */
	(void) xreadfour(input);			/* 16 mystery ??? */
	startfontlist = xreadfour(input);	/* 20 startfontlist */
	startdirlist = xreadfour(input);	/* 24 startdirlist */
	endfontlist = xreadfour(input);		/* 28 endfontlist = startsetlist */
	endsetlist = xreadfour(input);		/* 32 endsetlist */
/*	if (bDebug > 1) {
		sprintf(debugstr, "ATMREG font list from %lu to %lu\n",
				startfontlist, endfontlist);
		OutputDebugString(debugstr);
	} */
//	See whether strings in ATMREG,ATM are in UNICODE format
	(void) winseek(input, endsetlist);
	(void) wingetc(input);
	if (wingetc(input) == 0) bATM41 = 1;
	else bATM41 = 0;
	(void) winseek(input, endsetlist);
	ReadATMBuild(input);
	(void) winseek(input, startfontlist);	/* position start of font list */
	return endfontlist;	
}

#ifdef LONGNAMES
int ScanATMReg (HFILE input, int testflag) 
#else
int ScanATMReg (FILE *input, int testflag) 
#endif
{
	int n;
	unsigned long endfontlist;

	(void) wininit(input);		/*	initialize buffering stuff */
	endfontlist = ReadPointers(input);	/* positions at start of list */
/*	if (skipdirectories(input) < 1) return -1; */
/*	ScanAllFonts(input, endfontlist); */
	n = ScanAllFonts(input, endfontlist, testflag);	/* changed 97/Jan/21 */
/*	return 0; */
	return n;			/* pass back return value ??? 97/Jan/21 */
/*	return is number of fonts found if testflag == 0 */
/*	return is zero if testflag != 0 and file name *was* found */
}

/* scan atmreg.atm in the windows directory for font info */
/* returns -1 if it fails for one reason or another */

/* Two uses: if testflag == 0, fill DVI font demands 97/Jan/21 */
/*           if testflag != 0, find corresponding file name, put in str */
/*			 (Face name in TestFont, style in bCheckBold, bCheckItalic */

/* is it safe to use str here ? */

/* We don't store this away in an internal table */
/* Read again for next DVI file */

/* int ReadATMReg(void) */	/* changed 97/Jan/21 */
int ReadATMReg (int testflag) {	/* Try and read atmreg.atm */
#ifdef LONGNAMES
	HFILE input;
#else
	FILE *input;
#endif
	int n=0;
	char *s;

/*	Check if ATM is setup to auto activate ... let's flush this test ? */
/*	(void) GetPrivateProfileString("Settings", "AutoActivate", "", 
								   str, sizeof(str), "atm.ini"); */
/*	so it will just fail later, but at least we have Windows Face Name... */
/*	if (_strcmpi(str, "On") != 0) return -1; */
/*	Try and find location of font data base */
/*	NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */
	if (bWinNT)	*str = '\0';	/* there is no atm.ini in NT 97/Jan/25 */
	else GetPrivateProfileString("Settings", "ACPBase", "", 
								   str, sizeof(str), "atm.ini");
/*	get from Registry in Windows NT 4.0 ? */
	if (*str == '\0') {	/* if not specified, try default */
		strcpy(str, szWinDir);
		s = szWinDir + strlen(szWinDir) - 1;
		if (*s != '\\' && *s != '/') strcat(str, "\\");
		strcat(str, "atmreg.atm");
	}
	input = fopen(str, "rb");			/* open in binary mode for reading */
	if (input == BAD_FILE) return -1;	/* probably because not found */
#ifdef DEBUGFONTSEARCH
	if (bDebug > 1) {
		sprintf(debugstr, "ReadATMReg %s\n", str);
		OutputDebugString(debugstr);
	}
#endif
/*	n = ScanATMReg(input); */			/* read and apply to DVI fonts */
	n = ScanATMReg(input, testflag);	/* read and apply to DVI fonts */
	fclose(input);
	return n;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

map_type map_create (char *);	/* arg is TeXFonts - list of directories */

/* Find mapping from TeX fontname (i.e. font file name) to Windows Face Name */
/* First do everything with aliasflag off, then repeat with aliasflag on */
/* The exact sequence is: */

/* (1) if ATMREG.ATM exists use it --- NEW 96/May/26 */
/* (2) if ATM is loaded and bTrueTypeOnly is off etc:	use ATM.INI */
/*     ReadIniFile("atm.ini"); */
/* (3) if TTUsable and bType1Only is off etc:	use WIN.INI */
/*     SetupTTMapping(); and ApplyTTMapping(); */
/*     or ReadIniFile("win.ini"); */	/* move higher up ? */
/*     if fails WriteRegFile() */
/* (4a) try reading <filename>.FNT in <filename>.DVI directory */
/*	   readdviini(); */
/* (4b) if not found, try reading DVIWINDO.FNT in <filename>.DVI directory */
/*	   readdviini(); */
/* (5) try reading DVIWINDO.FNT in DVIWINDO directory */
/*	   readsubini(); */
/*	   If above fails to find all fonts map_create(szTeXFonts); */
/*	   and try again with aliases */

/*	NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */

/* This is called from dviwindo.c */

int mapfontnames (HDC hDC) {
	int flag;

	standardizenames();			/* No Underscores (and Upper Case) */

	findbasenames();			/* 1996/July/23 */

#ifdef DEBUGFONTSEARCH
	if (bDebug > 1) {
		sprintf(debugstr, "TTOnly %d T1Only %d TTUsable %d UseATM %d ATMLoaded %d\n",
			bTrueTypeOnly, bTypeOneOnly, bTTUsable, bUseATMINI, bATMLoaded);
		OutputDebugString(debugstr);
	}			/* debugging 1995/Mar/15 */
#endif	

	aliasflag = 0;
	for (;;) {					/* once without, once with aliasing */
								/* possibly also repeat after call RegEdit */
/*		See whether we can use ATMREG.ATM to figure out fonts 96/May/28 */
/*		Maybe we should do this not only when bAutoActivate is on ? */
		if (bAutoActivate) {						/* ATM 4.0 96/May/28 */
			if (ReadATMReg(0) < 0) bAutoActivate = 0;	/* remember failed */
			else if (missingfonts(0) == 0) return 0;
		}

/*		if (bUseATMINI != 0) */
/*		if bUseATMINI < 0 check bATMLoaded (or ATM running at least) ??? */
/*		if (bUseATMINI != 0 && bTrueTypeOnly == 0) {	*/	/* 1993/Aug/17 */
/*		NOTE: there is no ATM.INI in Windows NT ! 97/Jan/21 */
		if ((bUseATMINI > 1 || (bUseATMINI > 0 && bATMLoaded))
/*				&& bTrueTypeOnly == 0) */
				&& !bTrueTypeOnly && !bWinNT) {	/* 97/Feb/5 */
#ifdef DEBUGFONTSEARCH
			if (bDebug > 1) OutputDebugString("Reading ATM.INI\n");
#endif
			flag = ReadIniFile("atm.ini", 0);  /* ATM.INI first to map fonts */
/*			ReadIniFile(hDC, "atm.ini", 0); */ /* ATM.INI first to map fonts */
			if (missingfonts(0) == 0) return 0;
		}

//		if (bUseBase13) {
//			InsertBase13();		// 2000 May 24
//			if (missingfonts(0) == 0) return 0;
//		}

/*		check whether TrueType is enabled ??? */
/*		if (bUseTTFINI != 0) */			/* TrueType info next if needed */
		if ((bUseTTFINI > 1 || (bUseTTFINI > 0 && bTTUsable))
				&& bTypeOneOnly == 0) {
/*			moved up here to come before potentially bad registry info */
/*			if (bDebug > 1) OutputDebugString("Reading WIN.INI\n");
			flag = ReadIniFile("win.ini", 1);
			if (missingfonts(0) == 0) return 0; */

			if (bUseTTMapping) {
/*				if (bDebug > 1) OutputDebugString("GetProfile WIN.INI\n"); */
#ifdef DEBUGFONTSEARCH
				if (bDebug > 1) OutputDebugString("TT Mapping Tables\n");
#endif
				if (hFileNames == NULL) SetupTTMapping(hDC);
				ApplyTTMapping();
			}
			if (missingfonts(0) == 0) return 0; 
#ifdef DEBUGFONTSEARCH
			if (bDebug > 1) OutputDebugString("Reading WIN.INI\n");
#endif
			flag = ReadIniFile("win.ini", 1);
			if (missingfonts(0) == 0) return 0;

/*	Do this if TT fonts used *and* search fails.  Unless user szRegistry=... */
/*	Then it has already been done in SetupTTMapping */
/*			else if (bWin95 && bUseRegistryFile && !bRegistrySetup) */
			else if (bNewShell && bUseRegistryFile && !bRegistrySetup) {
#ifdef DEBUGREGISTRY
				if (bDebug > 1) OutputDebugString("Calling RegEdit\n");
#endif
				WriteRegFile(szRegistry);
/*	Now need to flush the bad list of font file names again */
				if (hFileNames != NULL) FreeFileNames();
				continue;		/* and then try again ! */
			}
		}

		if (bUseBase13) {
			InsertBase13();		// 2000 May 24
			if (missingfonts(0) == 0) return 0;
		}

		if (bUseUSRINI != 0) {
			if (bDebug > 1) OutputDebugString("Reading <filename>.FNT\n");
/*			(void) readdviini(); */	/* Use DVI file specific mapping info */
			if (readdviini() == 0) {	/* Use DVI file specific mapping info */
				if (missingfonts(0) == 0) return 0;
			}
		}
		if (bUseTEXINI != 0) {
			if (bDebug > 1) OutputDebugString("Reading DVIWINDO.FNT\n");
/*			(void) readsubini(); */	/* Use General Purpose substitution */
			if (readsubini() == 0) {	/* Use General Purpose substitution */
				if (missingfonts(0) == 0) return 0;
			}
		}
/*	Now have tried ATM.INI, WIN.INI, <fname>.FNT, DVIWINDO.FNT ... */
/*	If there is a texfonts.map file, and we haven't tried aliasing yet, */
/*	then try the whole thing again with aliasing... */

		if (bDebug > 1) OutputDebugString("Loading texfonts.map\n");

		if (bTeXFontMap && (aliasflag < 1)) {
			aliasflag++;
/*			if (fontmap == NULL) fontmap = map_create ("TEXFONTS"); */
			if (fontmap == NULL) {
				if (szTeXFonts != NULL) 
					fontmap = map_create (szTeXFonts);
			}
			if (fontmap == NULL) {
				if (bDebug > 1) OutputDebugString("No Aliases\n");
				break;			/* 1995/July/30 */
			}
			if (bDebug > 1) OutputDebugString("Switching to Aliases\n");
		}
		else break;
/*		if (bDebug > 1) OutputDebugString("Switching to Aliases\n"); */
	}
/*	now have tried everything without aliasing and with aliasing */
/*	if (missingfonts(0) == 0) return 0; */
	if (bDebug > 1) missingfonts(1);	/* show list of missing fonts */
	return -1;							/* incomplete substitution */
}

void ShowFontNames (void) {			/* debugging code */
	int k;
	char *fname, *sfname;
	if (bDebug && bDebugKernel) {
		for (k = 0; k < fnext; k++) {
			if (fontname[k] != NULL) fname = fontname[k];
			else fname = "UNKNOWN";
			if (subfontname[k] != NULL) sfname = subfontname[k];
			else sfname = "UNKNOWN";
/* is it safe to print if fontname[k] or subfontname[k] is NULL */
			sprintf(debugstr, "%s %s B %d I %d T %d\n",
/*				fontname[k], subfontname[k], */
				fname, sfname, fontbold[k], fontitalic[k], fontttf[k]);
				OutputDebugString(debugstr);
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for building page table */

/* build page table in textures files */
/* also reads stuff after `pre' and after `post' */
/* so has all DVI parameters and all font definitions after this */

void buildtextures (HFILE input) {
	int c;
/*	int	pageno; */
	long current;
	
	pageno = 1;				/* not zero based */
	current = dvistart;
	(void) winseek(input, current-4);
	texlength = sreadfour(input);		/* TeXtures length code */
	if ((c = wingetc(input)) != (int) pre) winerror("Expecting PRE");
	else logdo_pre(input);
/* skip forward through file from page to page */
	for (;;) {
		current = current + texlength + 4;
		(void) winseek(input, current-4);
		texlength = sreadfour(input);		/* TeXtures length code */
		if ((c = wingetc(input)) != (int) bop) break;
		pageno++;
	}
	maxpagenumber = pageno-1;
	if (c == (int) post) logdo_post(input);		/* pick up position of post */
	else winerror("Not BOP or POST");
	dvi_bytes = (unsigned long) filepos;  /* ??? */
/*	if (bDebug && dvi_t != maxpagenumber) {
		sprintf(str, "maxpagenumber %d dvi_t %d", maxpagenumber, dvi_t); 
		winerror(str);
	} */
	dvi_t = maxpagenumber;		/* ??? */		/* get number of pages */

	AllocPages(dvi_t);		/*  allocate page table (textures mode) */
	GrabPages();			/*  lock page table */
	PageStart = lpPages;
	PageCount = PageStart + dvi_t; /* MAXTEXTUREPAGES */
	pageno = 1;				/* not zero based */
	current = dvistart;
	(void) winseek(input, current-4);
	texlength = sreadfour(input);		/* TeXtures length code */
/*	if ((c = wingetc(input)) != pre) winerror("Expecting PRE"); */
/*	skip forward through file from page to page */
	for(;;) {
		current = current + texlength + 4;
		(void) winseek(input, current-4);
		texlength = sreadfour(input);		/* TeXtures length code */
		if ((c = wingetc(input)) != (int) bop) break;
		PageStart[pageno-1] = current;
		PageCount[pageno-1] = sreadfour(input);	/* counter[0] */
		pageno++;
	}
/*	maxpagenumber = pageno; */
	ReleasePages();							/* unlock page table (textures) */
/*	if (c == post) logdo_post(input);	*/	/* read info on fonts */
/*	unwingetc(c, input); */
/*	postposition = wintell(input); *//* remember position of post */
}

/* build page table in normal DVI file */

void BuildPages (HFILE input) {
	int pageno;
	
/*	if (textures != 0) winerror("Textures");	*/ /* debug */
#ifdef IGNORED
	if (skiptoend != 0) {
		winerror("skip to end"); /* debugging only */
	}
#endif
/*	maxpageno = dvi_t; */
	AllocPages(dvi_t);		/*  allocate page table for dvi_t pages ... */
	GrabPages();			/*  lock page table */
	PageStart = lpPages;				/* first half of memory */
	PageCount = PageStart + dvi_t;		/* second half of memory */

	if (bDebug > 1) OutputDebugString("BuildPages");
	pageno = dvi_t-1;				/* last page - zero based */
	ps_next = -1;
	ps_current = postposition;			/* from pointer after `post' */
	(void) winseek(input, ps_current);
	(void) replenish(input, 64);
	if (wingetc(input) == (int) post) {
		logdo_post(input);
	}
	else winerror("Failed Post");		/* debugging only ? */
/*	now skip backward through file from page to page */
	for(;;) {
		ps_next = ps_current;
		ps_current = ps_previous;
		(void) winseek(input, ps_current);
		(void) replenish(input, 64);
		if (wingetc(input) != (int) bop) {
			winerror("Expected BOP");		/* debugging */
			break;
		}
		PageStart[pageno] = ps_current;
		logdo_bop(input);				/* read counters and previous */
		PageCount[pageno] = counter[0];
#ifdef DEBUGCOLORSTACK
		if (bDebug > 1) {
			sprintf(debugstr, "pageno %d PageStart %d PageCount %d\n",
					pageno, PageStart[pageno], PageCount[pageno]);
			OutputDebugString(debugstr);
		}									/* 97/May/10 */
#endif
		if (ps_previous < 0 || --pageno < 0) break;
	}
/*	if (pageno != 0) */
	if (bDebug > 1 && pageno != 0) {
		sprintf(str, "Stopped at page %d (dvi_t %d)", pageno, dvi_t);
		OutputDebugStr(str);
	} 
	ReleasePages();				/* unlock page table */
/*	(void) winseek(input, ps_current); */	/* ??? */
	(void) winseek(input, dvistart);	/* ??? */
/*	pagenumber = 1; */				/* ??? */
/*	bop_valid = 1; */
/*	sprintf(str, "prev %ld curr %ld next %ld", ps_previous, ps_current, ps_next);
	winerror(str); */
}

/*	Main entry point to this part of the program */
/*	Pick up font info ? Build page table ? */
/*	Remember to deallocate pagetable when file is closed !!! */

/*	Revision 98/Feb/15 bCarryColor - scan DVI file forward to build table */
/*	This means must first find dvi_t from post using getpagecount() */
/*	DON'T want to quit early here, or page table won't get built ! */

long scanlogfile (HFILE input, HDC hDC) {	/* prescan of DVI file */
	int c, k;
	int hitend = 0;

	if (input == -1) {
		winerror("Invalid File Handle");
		return -1;
	}

	hdc = hDC;					/* make globally available 98/Feb/15 */

	(void) wininit(input);		/* initialize buffering stuff */
	errlevel = 0;				/* no errors yet ! */
	textures=0;					/* start off assuming normal DVI file */
	dvistart = 0;				/* In case its a normal TeX file */
	postposition = -1;			/* fill in later if found */

	ps_previous = -1;			/* for textures ... */
	ps_current = -1;			/* flag first bop not seen yet */

	dvi_f = -1;					/* redundant */
	FreeFontNames();			/* free strings font name tables 95/July/7 */
	fnext = 0;					/* reset count of fonts seen */
	for (k = 0; k < MAXFONTNUMBERS; k++)
		finx[k] = (short) BLANKFONT;	/* 93/Dec/11 */
/*	for (k = 0; k < MAXFONTS; k++) fonthit[k] = 0; */
	for (k = 0; k < MAXFONTS; k++) {
		fontwarned[k] = ansifont[k] = 0; 	/* 99/Jan/7 */
		fontbold[k] = fontitalic[k] = 0; 	/* 99/Jan/7 */
	}

	finish = 0;
	stinx = 0;				/* redundant, hopefully */
/*	maxstinx = 0; */
/*	markindex = 0; */		/* index for next mark seen */
	bHyperUsed = 0;			/* assume no hypertext until evidence 94/Oct/5 */
	bSourceUsed = 0;		/* assume no src until evidence 98/Dec/12 */
	bColorUsed = 0;			/* assume no color \special until ... 98/Feb/15 */
	bBackUsed = 0;			/* assume no background color \special */
	
	c = wingetc(input); unwingetc(c, input);
	if (c != (int) pre) {		/* not standard DVI file - figure it out */
		if(readovertext(input) == 0) {
/*			sprintf(str, "FilePos %ld BufLen %d", filepos, buflen);
			winerror(str); */ /* DEBUG */
			filepos = 0;		/* to prevent file position message */
/*			winerror("ERROR: Not a valid DVI or Textures file"); */
			(void) LoadString(hInst, DVI_ERR, debugstr, sizeof(debugstr));
			winerror(debugstr);
			errcount();
			filepos = -1;			/* to stop `at byte' message */
			return -1;				/* giveup(31); */
		}
		else {
			textures = 1;	/* OK, i.e. it is a "Textures DVI file - "; */
			buildtextures(input);		/* build page table */
/*			(void) winseek(input, dvistart); */
			(void) winseek(input, postposition);
		}
	}

#ifdef ALLOWCOLOR
	if (bCarryColor) {			/* 98/Feb/15 */
		if (getpagecount(input)) {	/* get dvi_t */
			errcount();
			return -1;			/* failed to get page count */
		}
		AllocPages(dvi_t);		/* allocate page table */
		GrabPages();			/* lock page table */
		AllocColor(dvi_t);		/* even if it may not be used */
		AllocBack(dvi_t);		/* even if it may not be used */
/*		GrabColor(); */

		pageno = 0;				/* zero based ? */
		PageStart = lpPages;			/* file position */
		PageCount = PageStart + dvi_t;	/* count[0] */
		finish = 0;
	}
#endif	/* ALLOWCOLOR */

/*	if (bDebug > 1) OutputDebugString("After Page, Color, Back Alloc"); */

/* REMEMBER TO RELEASE THESE AGAIN LATER !!! */

/*  SKIP to end doesn't work with textures files - but we already know */
/*  also for textures files we already built page table */
	if (textures != 0) {
		skiptoend = 0;
/*		(void) winseek(input, postposition); */
	}
	else skiptoend = reverseflag;
#ifdef ALLOWCOLOR
	if (bCarryColor) skiptoend = 0;
#endif

/*	normally we'll be at beginning of goodies in DVI file now, then */
/*	after first bop we skip to post at end to pick up font info */
/*	in textures files we will already be at post */
/*	with bCarryColor we actually scan the file sequentially */	

	for(;;) {
		c = wingetc(input);
/*		if (bDebug > 1) {
			sprintf(debugstr, "%d ", c);
			OutputDebugString(debugstr);
		} */
		if (c == EOF) {
			winerror("Unexpected EOF on input");
			errcount();
			return -1;	/*	giveup(13); */
		}
		if (c < 128) {
/*			if (dvi_f < 0) invalidset((int) c); */
		}
		else if (c >= 171 && c <= 234) { /*	switch to font (c - 171) */
/*			logswitchfont(c - 171, input); */
		}
		else {
			switch(c) {
				case post: logdo_post(input); hitend = 1; break;
				case pre: logdo_pre(input); break;
				case post_post: logdo_post_post(input); hitend = 1; break;
				case nop: break;			/* do nothing */
				case bop: logdo_bop(input); break;
				case eop: logdo_eop(input); break;
				case fnt_def1: logdo_fnt_def1(input); break;
				case fnt_def2: logdo_fnt_def2(input); break;
				case fnt_def3: logdo_fnt_def3(input); break;
				case fnt_def4: logdo_fnt_def4(input); break;
#ifdef ALLOWCOLOR							/* 98/Feb/15 */
/*	None of the following need any action in prescan of the file ! */
				case set1: logdo_set1(input); break;
				case set2: logdo_set2(input); break;
				case set3: logdo_set3(input); break;
				case set4: logdo_set4(input); break;
				case set_rule: logdo_set_rule(input); break;
				case put1: logdo_put1(input); break ;
				case put2: logdo_put2(input); break;
				case put3: logdo_put3(input); break;
				case put4: logdo_put4(input); break;
				case put_rule: logdo_put_rule(input); break;	
/*				case push: logdo_push(); break; */
				case push: break; 
/*				case pop: logdo_pop(); break; */
				case pop: break;
				case right1: logdo_right1(input); break;
				case right2: logdo_right2(input); break;  
				case right3: logdo_right3(input); break; 
				case right4: logdo_right4(input); break; 
/*				case w0: logdo_w0(); break; */
				case w0: break; 
				case w1: logdo_w1(input); break;
				case w2: logdo_w2(input); break; 
				case w3: logdo_w3(input); break; 
				case w4: logdo_w4(input); break;	
/*				case x0: logdo_x0(); break; */
				case x0: break; 
				case x1: logdo_x1(input); break;
				case x2: logdo_x2(input); break; 
				case x3: logdo_x3(input); break; 
				case x4: logdo_x4(input); break;	
				case down1: logdo_down1(input); break;
 				case down2: logdo_down2(input); break; 
				case down3: logdo_down3(input); break; 
				case down4: logdo_down4(input); break; 
/*				case y5: logdo_y0(); break; */
				case y5: break; 
				case y6: logdo_y1(input); break;
				case y2: logdo_y2(input); break; 
				case y3: logdo_y3(input); break; 
				case y4: logdo_y4(input); break;
/*				case z0: logdo_z0(); break; */
				case z0: break; 
				case z1: logdo_z1(input); break;
				case z2: logdo_z2(input); break; 
				case z3: logdo_z3(input); break; 
				case z4: logdo_z4(input); break;	
				case fnt1: logdo_fnt1(input); break;
				case fnt2: logdo_fnt2(input); break;
				case fnt3: logdo_fnt3(input); break;
				case fnt4: logdo_fnt4(input); break;
#endif
/*	following activated 96/Sep/2 */ /* because \special can occur before bop */
				case xxx1: logdo_xxx1(input); break;
				case xxx2: logdo_xxx2(input); break;
				case xxx3: logdo_xxx3(input); break;
				case xxx4: logdo_xxx4(input); break;
	
				default: {
					sprintf(debugstr, "Unknown DVI command (%d)", c);
/*					if (bDebug > 1) OutputDebugString(debugstr); */
					winerror(debugstr);
					errcount();
/*					winerror("DEFAULT"); */		/* debugging */
					return -1;
				} 
			} /* end of switch(c) */
		} /* end of else */
		if (bCarryColor) {
			if (hitend) break;
			if (finish) {
				errcount();
				break;		/* 98/Aug/26 get out of trouble fast */
			}
		}
		else {
			if (finish) break;
		} 
	} /* end of for{;;} */

	if (bDebug > 1) {
		sprintf(debugstr, "finish %d hitend %d", finish, hitend);
		OutputDebugString(debugstr);
	}
	if (bCarryColor) {
		if (hitend == 0) {
			sprintf(debugstr, "Did not hit %s in prescan", "end");
			winerror(debugstr);
		}
	}
	else {
		if (finish == 0) {
			sprintf(debugstr, "Did not hit %s in prescan", "finish");
			winerror(debugstr);
		}
	} 

	if (bDebug > 1) OutputDebugString("End of Prescan");

/*	winerror("END"); */		/* debugging */

	dvi_fonts = fnext;
/*	if (dvi_fonts == 0) winerror("Zero Fonts"); */

	if (textures == 0) {
		dvi_bytes = (unsigned long) filepos;
/*		BuildPages(input); */
	}

/*	define ONEINCH 4736287		*/ 	/* 72.72 * 65536 `scaled' points */
	oneinch = (long) ((double) dvi_den / dvi_num * 254000.0);	/* 99/Apr/18 */

	if (bCarryColor) {
/*		ReleaseColor(); */
		ReleasePages();							/* unlock page table */
		if (bDebug > 1) {
			sprintf(debugstr, "pageno %d", pageno);
			OutputDebugString(debugstr);
		}
		if (bColorUsed == 0) FreeColor();		/* 98/Sep/6 */
	}
	else if (textures == 0) BuildPages(input); 

#ifdef IGNORED
	if (bColorUsed) {
		if (bDebug > 1)
			OutputDebugString("Color Specials Used");  /* debugging only */
	}
	
	if (bBackUsed) {
		if (bDebug > 1)
			OutputDebugString("Background Specials Used");  /* debugging only */
	}
#endif

	if (errlevel != 0) return -1; 		/* error encountered */
	else return (long) dvi_bytes;
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showfilespecs (HWND hWnd) {
	sprintf(str, "DefPath: %s;\nDefSpec: %s;  DefExt: %s;\nOpenName: %s;",
		DefPath, DefSpec, DefExt, OpenName);
	MessageBox(hWnd, str, "DVIWindo file specs", MB_ICONINFORMATION | MB_OK);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Use `texfonts.map' in directory path TEXFONTS for aliasing */
/* Only have to read this in once - not once per file ... */

#ifdef IGNORED

typedef struct map_element_struct {		/* list element key . value pair */
	char *key;
	char *value;
	struct map_element_struct *next;
} map_element_type;

typedef map_element_type **map_type; 

#endif

#ifdef FONTMAP

/* **************************** auxiliary functions *********************** */

static void complain_mem (unsigned int size) {	/* out of memory */
	sprintf(str, "Unable to honor request for %u bytes.\n", size);
	winerror (str); 
/*	bShowFlag = 0; */				/* turn off displaying */
/*	finish = -1; */					/* set early termination */
/*	bUserAbort = 1; */				/* stop printing */
	PostMessage(hwnd, WM_CLOSE, 0, 0L);		/* say bye-bye */
/*	PostQuitMessage(0); */			/* ??? */
}

static void *xmalloc (unsigned int size) {
/*	void *new_mem = (void *) malloc (size); */
	void *new_mem = (void *) LocalAlloc (LMEM_FIXED, size);
	if (new_mem == NULL) complain_mem(size);
	return new_mem; 
}

/* static void *xrealloc (void *old_ptr, unsigned int size) {
	void *new_mem;
	if (old_ptr == NULL)
		new_mem = xmalloc (size);
	else {
		new_mem = (void *) LocalReAlloc ((HLOCAL) old_ptr, size, 0);
		if (new_mem == NULL) complain_mem(size);
	}
	return new_mem;
} */

/* static void xfree (void *ptr) {
	LocalFree ((HLOCAL) ptr); 
} */

static char *xstrdup (char *s) {
	char *new_string = (char *) xmalloc (strlen (s) + 1);
	return strcpy (new_string, s);
}

/* static char *concat3 (char *s1, char *s2, char *s3) {
	char *answer
		= (char *) xmalloc (strlen (s1) + strlen (s2) + strlen (s3) + 1);
	strcpy (answer, s1);
	strcat (answer, s2);
	strcat (answer, s3);
	return answer;
} */	/* used by extend_filename only */

static void *xcalloc (unsigned int nelem, unsigned int elsize) {
/*	void *new_mem = (void *) calloc (nelem, elsize); */
	void *new_mem =
		(void *) LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, nelem * elsize);
	if (new_mem == NULL) complain_mem (nelem * elsize);
	return new_mem;
}

/*	Here we work only with suffix-free names - so this is silly */

/* static char *find_suffix (char *name) {
	char *dot_pos; 
	char *slash_pos; 
  
	dot_pos = strrchr (name, '.');
	if (dot_pos == NULL) return NULL;	
	if ((slash_pos = strrchr (name, '/')) != NULL) ;
	else if ((slash_pos = strrchr (name, '\\')) != NULL) ;  
	else if ((slash_pos = strrchr (name, ':')) != NULL) ;
	else slash_pos = name;
	if (dot_pos < slash_pos) return NULL;
	return dot_pos + 1;
} */

/* static char *extend_filename (char *name, char *default_suffix) {
  char *suffix = find_suffix (name);
  char *new_s;
  if (suffix != NULL) return name;	
  new_s = concat3 (name, ".", default_suffix);
  return new_s;						
} */

/* static char *remove_suffix (char *s) {
	char *ret;
	char *suffix = find_suffix (s);
  
	if (suffix == NULL) return NULL;
	suffix--;
	ret = (char *) xmalloc (suffix - s + 1);
	strncpy (ret, s, suffix - s);
	ret[suffix - s] = 0;
	return ret; 
} */

#ifdef LONGNAMES
static char *read_line (HFILE f) {
	return lfgets(line, sizeof(line), f);
}
#else
static char *read_line (FILE *f) {
	return fgets(line, sizeof(line), f);
}
#endif

/***************************************************************************/

/* Fontname mapping.  We use a straightforward hash table. */

#define MAP_SIZE 199

/* The hash function.  We go for simplicity here. */

static unsigned int map_hash (char *key) {
	unsigned int n = 0;
	char *s = key;
/*	There are very few font names which are anagrams of each other
	so no point in weighting the characters. */
	while (*s != 0) n += *s++;
	n %= MAP_SIZE;
#ifdef DEBUGFLAG
/*	if (traceflag) printf("hash %u for %s\n", n, key); */
#endif
	return n;
}

/* Look up `key' in MAP.  Return the corresponding `value' or NULL. */
/* Made case insensitive 97/Oct/21 */

static char *map_lookup_str (map_type map, char *key) {
	unsigned int n = map_hash (key);
	map_element_type *p;
  
	if (map == NULL) return NULL;				/* 1995/July/30 */
	for (p = map[n]; p != NULL; p = p->next) {
#ifdef DEBUGFLAG
/*		if (traceflag) printf("Trying %s against %s\n", key, p->key); */
#endif
/*		if (strcmp (key, p->key) == 0) return p->value; */
		if (_stricmp (key, p->key) == 0) return p->value; /* 97/Oct/21 */
#ifdef DEBUGFLAG
/*		if (traceflag) printf("p->next %p\n", p->next); */
#endif
	}
#ifdef DEBUGFLAG
/*	if (traceflag) printf(" failed to find %s\n", key); */
#endif
	return NULL;					/* failed to find it */
}

/* map_type fontmap = NULL; */	/* static - keep around once set 94/Nov/20 */

void map_free (map_type map) {	/* Free key and value strings, and structure */
	map_element_type *p;
	map_element_type *q;
	unsigned int n;
  
	if (fontmap == NULL) return;				/* 1995/July/30 */

	for (n = 0; n < MAP_SIZE; n++) {
		for (p = map[n]; p != NULL; p = q) {
			if (p->key) LocalFree(p->key);		/* free key */
			if (p->value) LocalFree(p->value);	/* free value */
			q = p->next;
			if (p) LocalFree(p);		/* free structure itself */
		}
	}
}

void free_fontmap (void) {	/* call when exiting to make BoundChecker happy */
	if (fontmap == NULL) return;
	map_free (fontmap);
	LocalFree(fontmap);
	fontmap = NULL;
}

#ifdef DEBUGFLAG
static void map_show (map_type map) {			/* debugging tool */
	map_element_type *p;
	unsigned int n;
  
	if (fontmap == NULL) return;				/* 1995/July/30 */

	for (n = 0; n < MAP_SIZE; n++) {
		for (p = map[n]; p != NULL; p = p->next) {
//			printf("n %u key %s next %p\n", n, p->key, p->next);
		}
	}
}
#endif

/*	Look up KEY in MAP; if it's not found, remove any suffix from KEY and
	try again.  Then paste key back into answer ... */

/*	OK, the original KEY didn't work.  Let's check for the KEY without
    an extension -- perhaps they gave foobar.tfm, but the mapping only
    defines `foobar'. */

/*	Append the same suffix we took off, if necessary. */
/*	if (ret) ret = extend_filename (ret, suffix); */

char *map_lookup (map_type map, char *key) {
	char *ret;

	if (map == NULL) return NULL;			/* 1995/July/30 */

	ret = map_lookup_str (map, key);
/*	char *suffix; */
  
/*	lets assume we never have to deal with names that have extensions! */
/*	suffix = find_suffix (key); 
	if (!ret) {
		if (suffix) {
			char *base_key = remove_suffix (key);
			ret = map_lookup_str (map, base_key);
			free (base_key);
		}
	}
	if (ret && suffix) ret = extend_filename (ret, suffix); */

	return ret;
}

/* If KEY is not already in MAP, insert it and VALUE. */
/* This was a total mess! Fixed 1994/March/18 */
/* Made case insensitive 97/Oct/21 */

static void map_insert (map_type map, char *key, char *value) {
/*	unsigned int n = map_hash (key); */
	unsigned int n;
/*	map_element_type **ptr = &map[n]; */
	map_element_type **ptr; 

	if (map == NULL) return;				/* 1995/July/30 */

	n = map_hash (key);
	ptr = &map[n]; 

/*	while (*ptr != NULL && !(strcmp(key, (*ptr)->key) == 0)) */
	while (*ptr != NULL && !(_stricmp(key, (*ptr)->key) == 0))  /* 97/Oct/21 */
		ptr = &((*ptr)->next); 

	if (*ptr == NULL) {
		*ptr = (map_element_type *) xmalloc (sizeof(map_element_type));
		(*ptr)->key = xstrdup (key);
		(*ptr)->value = xstrdup (value);
		(*ptr)->next = NULL;
	}
}

/* Open and read the mapping file FILENAME, putting its entries into
   MAP. Comments begin with % and continue to the end of the line.  Each
   line of the file defines an entry: the first word is the real
   filename (e.g., `ptmr'), the second word is the alias (e.g.,
   `Times-Roman'), and any subsequent words are ignored.  .tfm is added
   if either the filename or the alias have no extension.  This is the
   same order as in Dvips' psfonts.map; unfortunately, we can't have TeX
   read that same file, since most of the real filenames start with an
   `r', because of the virtual fonts Dvips uses.  
   And what a load of bull! And who cares about DVIPS and VF files !*/

static void map_file_parse (map_type map, char *map_filename) {
	char *l;
	unsigned int map_lineno = 0;
	unsigned int entries = 0;
#ifdef LONGNAMES
	HFILE f;
#else
	FILE *f;
#endif

	if (map == NULL) return;		/* 1995/July/30 */

	f = fopen (map_filename, "r");

	if (f == BAD_FILE) {
/*		perror(map_filename); */	/* shouldn't ever happen */
		return;						/* better get out of here then ! */
	}

	while ((l = read_line (f)) != NULL) {
		char *filename;
		char *comment_loc;

		comment_loc = strrchr (l, '%');
		if (comment_loc == NULL) comment_loc = strrchr (l, ';');
      
/*		Ignore anything after a % or ; */
		if (comment_loc)  *comment_loc = 0;
/*		if (comment_loc != NULL)  *comment_loc = 0; */ /* cleaner ? */
      
		map_lineno++;
      
/*		If we don't have any filename, that's ok, the line is blank. */
/*		filename = strtok (l, " \t"); */
		filename = strtok (l, " \t\n");
/*		if (filename) */
		if (filename != NULL) {
			char *alias;
/*			standardize filename - uppercase and no trailing underscores */
			removeunderscores(filename);	
/*			if (bUpperCase) */		/* normalize file names */
				makeuppercase(filename);	/* standardize name */
/*			alias = strtok (NULL, " \t"); */
			alias = strtok (NULL, " \t\n");          
/*			But if we have a filename and no alias, something's wrong. */
			if (alias == NULL || *alias == '\0') { /* ignore errors */
/*				fprintf (stderr,
					" font name `%s', but no alias (line %u in `%s').\n",
						filename, map_lineno, map_filename); */
			}
			else {       /* We've got everything.  Insert the new entry. */
/*				standardize alias - uppercase and no trailing underscores */
				removeunderscores(alias);	
				if (bUpperCase) 
					makeuppercase(alias);	/* standardize name */
				map_insert (map, alias, filename);		/* alias is the key */
				entries++;
			}
		}
	}
	fclose (f);
/*	if (bDebug > 1)	{
		sprintf(debugstr, "%u entries", entries);
		winerror(debugstr);
	} */
}

/* Look for the file `texfonts.map' in each of the directories in
   TEXFONTS.  Entries in earlier files override later files. */

/* make our own searchenv */ /* writes back into last argument */
/* used for "TEXFONTS" only - which may differ from TexFonts */

/*void oursearchenv (char *mapname, char *envvar, char *pathname) */
void oursearchenv (char *mapname, char *searchpath, char *pathname) { 
/*	char *searchpath; */	/* 97/May/10 */
	int foundfile=0;
#ifndef SUBDIRSEARCH
/*	FILE *input; */			/* 96/May/26 */
#ifdef LONGNAMES
	HFILE input;
#else
	FILE *input;
#endif
	char *s;
#endif

	*pathname = '\0';					/* just in case 97/May/10 */
	if (searchpath == NULL) return;		/* fail 97/May/10 */

	if (bDebug > 1) OutputDebugString(searchpath);	/* 97/May/10 */
#ifdef SUBDIRSEARCH
	if (searchalongpath (mapname, searchpath, pathname, 0) != 0)
		*pathname = '\0';
	else foundfile = 1;						/* 1994/Aug/18 */
#else
	for (;;) {
		if ((searchpath = nextpathname(pathname, searchpath)) == NULL) {
/*			foundfile = 0; */
			break;
		}
		s = pathname + strlen(pathname) - 1;
		if (*s != '\\' && *s != '/') strcat(pathname, "\\"); 
		strcat(pathname, mapname);
		input = fopen(pathname, "r");

/*		if (input != NULL) */
		if (input != BAD_FILE) {		/* 96/May/28 */
			foundfile = 1;
			fclose (input);
			break;
		}
	}
#endif	/* SUBDIRSEARCH */
}

/* used to use environment variable TEXFONTS, now uses list of dirs */

/*map_type map_create (char *envvar) */
map_type map_create (char *searchpath) {		/* 97/May/10 */
/*	char filename[128]; */
	char filename[MAXFILENAME]="";			/* 97/May/10 */
	map_type map;
      
	if (searchpath == NULL) return NULL;	/* sanity check */
	if (*searchpath == '\0') return NULL;	/* sanity check */

	if (bTeXFontMap == 0) return NULL;		/* 95/July/30 already failed */

/*	if (bDebug > 1) winerror("map_create");	*/

/*	use our own searchenv */ /* writes back into last argument */
/*	oursearchenv("texfonts.map", envvar, filename); */
	oursearchenv("texfonts.map", searchpath, filename);

	if (*filename == '\0') {
		bTeXFontMap = 0;				/* 95/July/30 don't try again */
		return NULL;
	}

	if (bDebug > 1) OutputDebugString(filename);	/* 97/May/10 */
	map = (map_type) xcalloc(MAP_SIZE, sizeof (map_element_type *));
	map_file_parse(map, filename);
#ifdef DEBUGFLAG
	if (traceflag) map_show(map);
#endif
	return map;
}

/* ************************************************************************* */

/*	if we didn't find the font, maybe its alias to be found in texfonts.map */

char *alias (char *name) {  
/*	static map_type fontmap = NULL; */		/* static - keep around once set */
	char *mapped_name;
      
	if (name == NULL) return NULL;			/* sanity check */
/*	if (bDebug > 1) winerror("alias"); */

/*	Fault in the mapping if necessary. */
/*	Note: uses szTexFonts string now already set up in dviwindo.c */
/*	if (fontmap == NULL) fontmap = map_create ("TEXFONTS"); */ /* wrong */
	if (fontmap == NULL) {
		if (szTeXFonts != NULL)
			fontmap = map_create (szTeXFonts);	/* 97/Sep/6 */
	}      
	if (fontmap == NULL) return NULL;		/* 1995/July/30 */
	
/*	Now look for our filename in the mapping. */
	mapped_name = map_lookup (fontmap, name);
	return mapped_name;						/* possibly NULL */
}

/* ************************************************************** */

#endif	/* FONTMAP */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifndef SUBDIRSEARCH

/* Extract next pathname from a searchpath - and WRITE into pathname */
/* return NULL if there are no more pathnames, */
/* otherwise returns pointer to NEXT pathname in searchpath */
/* searchpath = pathname1;pathname2; ... ;pathnamen */
/* use for eps search path */
/* allows space as separator as well as `;' WHY? long file name problem */

char *nextpathname(char *pathname, char *searchpath) {
	int n;
	char *s;

	if (*searchpath == '\0') return NULL;	/* nothing left */
	else if (((s = strchr(searchpath, ';')) != NULL) ||
		     ((s = strchr(searchpath, ' ')) != NULL)) {	/* WHY ' ' ??? */
		n = (s - searchpath);
		if (n >= MAXPATHLEN) {
/*			sprintf(str, "Path too long %s\n", searchpath); 
			winerror(str); */
			winerror("Path too long");
			return NULL;
		}
		strncpy(pathname, searchpath, (unsigned int) n);
		*(pathname + n) = '\0';				/* terminate it */
		return(s + 1);						/* next pathname or NULL */
	}
	else {
		n = (int) strlen(searchpath);
		if (n >= MAXPATHLEN) {
/*			sprintf(str, "Path too long %s\n", searchpath); 
			winerror(str); */
			winerror("Path too long");
			return NULL;
		}
		strcpy(pathname, searchpath);
		return(searchpath + n);
	}
}

#endif	/* if not SUBDIRSERACH */


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef SUBDIRSEARCH

/* Code to implement (possibly recursive) sub-directory search 94/Aug/18 */

#define ACCESSCODE 4

#define SEPARATOR "\\"

/* This is the (possibly recursive) sub-directory searching code */

/* We come in here with the directory name in pathname */
/* Which is also where we plan to return the result when we are done */
/* We search in this directory first, */
/* and we search one level subdirectories (if it ended in \), */
/* or recursively if the flag is set (when it ends in \\). */
/* File name searched for is in filename */
/* Returns zero if successful --- return non-zero if not found */

/* Uses _access to check for existence of directories ??? */

/*  struct _finddata_t c_file; */
/*  _finddata_t structure *can* be reused, unlike _find_t 95/Jan/31 */
/*  so save on stack space, by having one copy, not one per expand_subdir */
/*  possibly allocate this in the caller of findsubpath ? not static ? */

/* unsigned attrib = _A_NORMAL | _A_SUBDIR | _A_RDONLY; */

int findsubpath (char *filename, char *pathname, int recurse) {
	char *s;
	int code;
	int ret;

	static struct _finddata_t c_file; 	/* structure can be reused (?) */
	long hFind;

/*	struct _find_t c_file; */

#ifdef LONGNAMES
	HFILE hfile;
#endif

	s = pathname + strlen(pathname);	/* remember the end of the dirpath */
/*	if (traceflag) printf("Entering findsubpath: %s %s %d\n",
		filename, pathname, recurse); */

/*	First try directly in this directory (may want this conditional) */
	strcat (pathname, SEPARATOR);		/* \ or / */
	strcat (pathname, filename);

/*	Try for match in this directory first - precedence over files in subdirs */
/*	if (_access(pathname, ACCESSCODE) == 0) */ 		/* check for read access */
/*	check this can handle long file names ??? */
#ifdef LONGNAMES
/*	hfile = _lopen(pathname, READ); */
	hfile = _lopen(pathname, READ | OfExistCode); /* 96/May/18 */
	if (hfile != HFILE_ERROR) {
		_lclose(hfile);
		hfile = HFILE_ERROR;
		code = 0;						/* success */
	}
	else code = -1;
#else
	code = _access(pathname, ACCESSCODE);
#endif
	if (code == 0) {
/*		if (traceflag) printf("SUCCESS: %s\n", pathname); */
		return 0;						/* success */
	}
/*	if (traceflag) {
		code = _access(pathname, ACCESSCODE);
		printf("File %s Access %d\n", pathname, code);
	} */

	*s = '\0';						/* strip off the filename again */
/*	no luck, so try and find subdirectories */
	strcat (pathname, SEPARATOR);	/* \ or / */
	strcat (pathname, "*.*");
/*	if (_dos_findfirst (pathname, _A_NORMAL | _A_SUBDIR, &c_file) != 0) */
/*	if (_dos_findfirst (pathname, _A_NORMAL | _A_SUBDIR | _A_RDONLY,
		&c_file) != 0) */

	hFind = _findfirst (pathname, &c_file);
	if (hFind > 0) 	ret = 0;		/* found something */
	else ret = -1;					/* did not find path ? */

/*	ret = _dos_findfirst (pathname,	attrib, &c_file); */

	if (ret != 0) {					/* nothing found ? */
		return -1;					/* failure */
	}
	*s = '\0';						/* strip off the \*.* again */
/*	Step through sub-directories */
	for (;;) {
/*		Ignore all but sub-directories here - also ignore . and .. */
		if ((c_file.attrib & _A_SUBDIR) == 0 ||	*c_file.name == '.') {
/*			if (_dos_findnext (&c_file) != 0) break; */
			ret = _findnext (hFind, &c_file);	/* success == TRUE ??? */

/*			ret = _dos_findnext (&c_file); */		/* success == 0 */
/*			need to flip polarity of ret ? apparently not ... */

			if (ret != 0) break;	/* give up, found nothing more */
			continue;				/* did find something else, go on */
		}
/*		extend pathname with subdir name */
		strcat(pathname, SEPARATOR);
		strcat(pathname, c_file.name);
/*		if (traceflag) printf("Checking subdir: %s\n", pathname); */
/*		OK, now try for match in this directory */
		if (recurse) {							/* recursive version */
			if (findsubpath(filename, pathname, recurse) == 0) {
				_findclose(hFind);
				hFind = 0;
				return 0;						/* succeeded */
			}
		}
		else {									/* not recursive */
			strcat (pathname, SEPARATOR);
			strcat (pathname, filename);
/*			if (traceflag) printf("Checking file: %s\n", pathname); */
/*			if (_access(pathname, ACCESSCODE) == 0) */ /* check read access */
/*			check this can handle long file names ??? */
#ifdef LONGNAMES
/*			hfile = _lopen(pathname, READ); */
			hfile = _lopen(pathname, READ | OfExistCode); /* 96/May/18 */
			if (hfile != HFILE_ERROR) {
				_lclose(hfile);
				hfile = HFILE_ERROR;
				code = 0;						/* success */
			}
			else code = -1;
#else
			code = _access(pathname, ACCESSCODE);	
#endif
			if (code == 0) { 
/*				if (traceflag) printf("SUCCESS: %s\n", pathname); */
				_findclose(hFind);
				hFind = 0;
				return 0;						/* success */
			}
/*			if (traceflag) {
				code = _access(pathname, ACCESSCODE);
				printf("File %s Access %d\n", pathname, code);
			} */
		}

/*		No match in this directory, so continue */
		*s = '\0';
/*		if (traceflag) printf("Ready for dos_findnext: %s %s %d\n",
			filename, pathname, recurse); */
/*		if (_dos_findnext (&c_file) != 0) break; */

		ret = _findnext (hFind, &c_file);		/* success == TRUE ??? */

/*		ret = _dos_findnext (&c_file); */

/*		need to flip polarity of ret ? apparently not ... */

		if (ret != 0) break;					/* found no more */
	}	/* end of for{;;} loop */
	if (hFind > 0) {
		_findclose (hFind);
		hFind = 0;
	}
	return -1;									/* failed */
}

/* Our searchalongpath is (somewhat) analogous to DOS _searchenv */
/* The name of the desired file is given in `filename' */
/* The list of paths is given in `pathlist' */
/* searchalongpath returns the full pathname of first match in `pathname' */
/* (make sure there is enough space there!) */
/* If the file is not found, then pathname contains "" */
/* and it also returns non-zero if it fails. */
/* It first searches in the current directory (control by flag?) */
/* If a path in `pathlist' ends in \, then its sub-directories are searched, */
/* (after the specified directory) */
/* If a path in `pathlist' ends in \\, then this works recursively */
/* (which may be slow and cause stack overflows ...) */

int searchalongpath (char *filename, char *pathlist, char *pathname, int current) {
/*	struct find_t c_file; *			/* need to preserve across calls to DOS */
	char *s, *t, *u;
	int c, n;
	int recurse;
/*	int code; */
#ifdef LONGNAMES
	HFILE hfile;
	int code;
#endif
	
	if (current) {	/*	Try for exact match in current directory first ? */
		strcpy(pathname, filename);
/*		if (traceflag) printf("Trying: %s\n", pathname); */
/*		check this can handle long file names ??? */
#ifdef LONGNAMES
/*		hfile = _lopen(pathname, READ); */
		hfile = _lopen(pathname, READ | OfExistCode); /* 96/May/18 */
		if (hfile != HFILE_ERROR) {
			_lclose(hfile);
			hfile = HFILE_ERROR;
			code = 0;						/* success */
		}
		else code = -1;
#else
		code = _access(pathname, ACCESSCODE);
#endif
		if (code == 0) {	/* check for read access */
/*			if (traceflag) printf("SUCCESS: %s\n", pathname); */
			return 0;							/* success */
		}
/*		if (traceflag) {
			code = _access(pathname, ACCESSCODE);
			printf("File %s Access %d\n",
				pathname, _access(pathname, ACCESSCODE));
		} */
	}

	*pathname = '\0';						/* just in case 97/May/10 */
	if (pathlist == NULL) return -1;		/* failed 97/May/10 */

/*	Now step through paths in pathlist */
	s = pathlist;
	for (;;) {
		if (*s == '\0') break;				/* sanity check */
		if ((t = strchr(s, ';')) == NULL)
			t = s + strlen(s);				/* if last path */
		n = t - s;
		strncpy(pathname, s, n);
		u = pathname + n;
		*u-- = '\0';						/* null terminate */
		c = *u;								/* check whether ends on \ */
		if (c == '\\' || c == '/') {		/* yes it does */
			*u-- = '\0';					/* remove it */
			c = *u;							/* check whether ends on \\ */
			if (c == '\\' || c == '/') {	/* yes it does */
				*u-- = '\0';				/* remove it */
				recurse = 1;				/* recursive subdir search */
			}
			else recurse = 0;
/*			if (traceflag) printf("Trying subdir: %s\n", pathname); */
			if (findsubpath (filename, pathname, recurse) == 0)
				return 0;	/* success */
		}
		else {									/* its just a directory */
			strcat (pathname, SEPARATOR);		/* \ or / */
			strcat (pathname, filename);
/*			if (traceflag) printf("Trying: %s\n", pathname); */
/*			check this can handle long file names ??? */
#ifdef LONGNAMES
/*			hfile = _lopen(pathname, READ); */
			hfile = _lopen(pathname, READ | OfExistCode); /* 96/May/18 */
			if (hfile != HFILE_ERROR) {
				_lclose(hfile);
				hfile = HFILE_ERROR;
				code = 0;						/* success */
			}
			else code = -1;
#else
			code = _access (pathname, ACCESSCODE);
#endif
			if (code == 0) {
/*				if (traceflag) printf("SUCCESS: %s\n", pathname); */
				return 0;						/* success */
			}
/*			if (traceflag) {		
				code = _access(pathname, ACCESSCODE);
				printf("File %s Access %d\n",
					pathname, _access(pathname, ACCESSCODE));
			} */
		}

		s = t;						/* move on to next item in list */
		if (*s == ';') s++;			/* step over separator */
		else break;					/* we ran off the end */
	}
	*pathname = '\0';				/* failed to find it */
	return -1;
}

/* search for file in path list and open it if found 1994/Aug/18 */
/* just make sure you close the darn file again later ! */
/* write full path name back into `pathname' unless NULL */

/* FILE *findandopen(char *filename, char *pathlist, char *mode, int current) */

/* HFILE findandopen(char *filename, char *pathlist, char *pathname, char
 *mode, int current) */

/* used from winspeci.c in SUBDIRSEARCH in findepsfile */

#ifdef LONGNAMES
HFILE findandopen (char *filename, char *pathlist, char *pathname, char *mode, int current) 
#else
FILE *findandopen (char *filename, char *pathlist, char *pathname, char *mode, int current) 
#endif
{
#ifdef LONGNAMES
	HFILE file;
#else
	FILE *file;
#endif
	char dummyname[MAXFILENAME]; 

#ifdef DEBUGSPECIAL
	if (bDebug > 1) {
		sprintf(debugstr, "FindAndOpen %s %s %s %s %d", filename, pathlist,
				pathname, mode, current);
		OutputDebugString(debugstr);
	}
#endif
	if (pathname == NULL) pathname = dummyname;	/* no _alloca in Windows ... */

	if (searchalongpath(filename, pathlist, pathname, current) == 0) {
		file = fopen(pathname, mode);
		return file;
	}
/*	else return NULL; */
	else return BAD_FILE;
}

#endif /* SUBDIRSEARCH */

/***********************************************************************/

/* Use registry to find Application to open given FileName 98/Dec/2 */

/* HKEY_CLASSES_ROOT\.gif = "giffile"*/
/* HKEY_CLASSES_ROOT\giffile\shell\open\command = foo.exe "%1" */
/* or use DDE */
/* HKEY_CLASSES_ROOT\giffile\shell\open\ddeexec\Application */
/* HKEY_CLASSES_ROOT\giffile\shell\open\ddeexec\Topic */

/* returns non-zero if it succeeds (length of application string) */
/* application written into given argument - if found */

int FindApplication (char *szFileName, char *szApplication, int nLen) {
	HKEY hKey=NULL;
	HKEY hKeyOpen=NULL;
/*	char szValue[FILENAME_MAX];	*/
	char szKey[FILENAME_MAX];	
	DWORD Type;				/* Probably will be set to REG_SZ */
	DWORD cdData;
	char *s, *sext;
	int err, ret;

	*szApplication = '\0';			/* in case pop out early */
	
	if (_strnicmp(szFileName, "http://", 7) == 0) 
		sext = ".html";				/* let browser handle this 2000/Jan/11 */
	else if ((sext = strrchr(szFileName, '.')) == NULL)
		return 0;					/* need extension to key off */

	err = RegOpenKeyEx(HKEY_CLASSES_ROOT, sext, 0L, KEY_QUERY_VALUE, &hKey);
	if (err	!= ERROR_SUCCESS) {
		ExplainError(sext, err, 0);	/* "RegOpenKey" */
		return 0;					/* "impossible" */
	}

/*	get the default value of this key */
/*	cdData = sizeof(szValue); */
	cdData = sizeof(szKey);
/*	err = RegQueryValueEx(hKey, NULL, 0L, &Type, szValue, &cdData); */
	err = RegQueryValueEx(hKey, NULL, 0L, &Type, (unsigned char *) szKey, &cdData);
	if (err == ERROR_SUCCESS) {
#ifdef DEBUGREGISTRY
/*		if (bDebug > 1) OutputDebugString(szValue); */
		if (bDebug > 1) OutputDebugString(szKey);
#endif
/*		strcpy(szKey, szValue); */
		strcat(szKey, "\\shell");
		strcat(szKey, "\\open");	
		strcat(szKey, "\\command");
		err = RegOpenKeyEx (HKEY_CLASSES_ROOT, szKey, 0L, KEY_QUERY_VALUE, &hKeyOpen);
		if (err	== ERROR_SUCCESS) {
/*			get the default value of this key */
			cdData = nLen;
			err = RegQueryValueEx(hKeyOpen, NULL, 0L, &Type, (unsigned char *) szApplication, &cdData);
			if ((s = strchr(szApplication, '%')) != NULL) { /* 98/Dec/7 */
/*				c = *(s+1); */
/*				if (c < '0' || c > '9') */
					strcpy(szKey, szApplication);
					ret = ExpandEnvironmentStrings(szKey, szApplication, nLen);
/*					ret number of bytes in result, 0 if failed */
					if (ret == 0) ExplainError(szKey, GetLastError(), 0);
/*				} */

			}
			if (err == ERROR_SUCCESS) {
#ifdef DEBUGREGISTRY
				if (bDebug > 1) OutputDebugString(szApplication);
#endif
			}
			else {	/* err != ERROR_SUCCESS */
/*				ERROR_FILE_NOT_FOUND 2 --- TrueType font, not Type 1 */
/*				ERROR_MORE_DATA 234 --- too little space for value */
				ExplainError("RegQueryValueEx", err, 0);
				cdData = 0;
			}
		}
		else {	/* err != ERROR_SUCCESS */
			ExplainError(szKey, err, 0);
			cdData = 0;
		}
	}
	else { /* err != ERROR_SUCCESS */
		ExplainError("RegQueryValueEx", err, 0); 
		cdData = 0;
	}

	if (hKeyOpen != NULL) {
		err = RegCloseKey(hKeyOpen);
		if (err != ERROR_SUCCESS) ExplainError("RegCloseKey", err, 0);
	}
	if (hKey != NULL) {	
		err = RegCloseKey(hKey);
		if (err != ERROR_SUCCESS) ExplainError("RegCloseKey", err, 0);
	}
	return cdData;
}

//////////////////////////////////////////////////////////////////////////////

/* Console Dialog box used by AFMtoTFM.DLL passed by dviwindo 99/June/25 */

/* The dialog box procedure should return TRUE to direct the system to set
the keyboard focus to the control given by hwndFocus. Otherwise, it should
return FALSE to prevent the system from setting the default keyboard focus. */

/* The control to receive the default keyboard focus is always the first
control in the dialog box that is visible, not disabled, and that has the
WS_TABSTOP style. When the dialog box procedure returns TRUE, the system
checks the control to ensure that the procedure has not disabled it. If it
has been disabled, the system sets the keyboard focus to the next control
that is visible, not disabled, and has the WS_TABSTOP. */

/* An application can return FALSE only if it has set the keyboard focus
to one of the controls of the dialog box. */

///////////////////////////////////////////////////////////////////////////////////

// Function to copy the contents of a list box to the clipboard.

void CopyControlToClipboard (HWND hwnd) {
	int nNum, nLen;
	char *szClipData = NULL;
	char *szLine = NULL;
	char *s;
	HGLOBAL hClipData;
	LPTSTR lpClipData;
	BOOL fOk;
	int nTotal=0, nMax=0;
	int nCount = ListBox_GetCount(hwnd);

	if (nCount == 0) return;				// empty list box
	
//	first figure out how much space we need for all this
	for (nNum = 0; nNum < nCount; nNum++) {
		nLen = ListBox_GetTextLen(hwnd, nNum);
		nTotal += nLen + 2;					// extra for \r\n
		if (nLen > nMax) nMax = nLen;
	}

	if (nMax == 0 || nTotal == 0) return;	// sanity check

	szClipData = (char *) malloc(nTotal+1);
	if (szClipData == NULL) return;			// failed

	*szClipData = '\0';
	szLine = (char *) malloc(nMax + 1);
	if (szLine == NULL) return;				// failed
	
	s = szClipData;
	for (nNum = 0; nNum < nCount; nNum++) {
		ListBox_GetText(hwnd, nNum, szLine);
		strcpy(s, szLine);
		s += strlen(szLine);
		strcpy(s, "\r\n");
		s += 2;
	}
	free(szLine);

	OpenClipboard(NULL); 
	EmptyClipboard();

//	Clipboard accepts only data that is in a block allocated 
//	with GlobalAlloc using the GMEM_MOVEABLE and GMEM_DDESHARE flags.
	hClipData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(szClipData) + 1);
	lpClipData = (LPTSTR) GlobalLock(hClipData);

	strcpy(lpClipData, szClipData);
	free(szClipData);
	szClipData = NULL;

	fOk = (SetClipboardData(CF_TEXT, hClipData) == hClipData);
	CloseClipboard();

	if (!fOk) {
		GlobalFree(hClipData);
		MessageBox(GetFocus(),
				   "Error putting text on the clipboard",
				   NULL, MB_OK | MB_ICONINFORMATION);
	}
}

//////////////////////////////////////////////////////////////////////////////

#ifdef IGNORED
void ShowDimensions(HWND hwnd, HWND hWndLB) {
	char *s = debugstr;
	RECT dlgRect, lbRect;
	
	GetClientRect(hwnd, &dlgRect);
	sprintf(s, "hDlg %08X\tClientRect %d %d %d %d\n",
			hwnd, dlgRect.left, dlgRect.bottom, dlgRect.right, dlgRect.top);
	s += strlen(s),
	GetWindowRect(hWndLB, &lbRect);
	sprintf(s, "hWndLB %08X\tLBRect %d %d %d %d\n",
			hWndLB, lbRect.left, lbRect.bottom, lbRect.right, lbRect.top);
	winerror(debugstr);
}
#endif

///////////////////////////////////////////////////////////////////////////////////

// This maintains the same borders around the listbox as the window is resized
// So the position and size of the LISTBOX better be set up right at the start

void Dlg_OnSize (HWND hwnd, UINT state, int cx, int cy) {
	RECT dlgRect, lbRect;
	POINT pt;
	int horizBorder, topBorder;
	HWND hWndLB = GetDlgItem(hwnd, ICN_LISTBOX);
	
	GetClientRect(hwnd, &dlgRect);
	GetWindowRect(hWndLB, &lbRect);
	pt.x = lbRect.left;
	pt.y = lbRect.top;
	ScreenToClient(hwnd, &pt);
	horizBorder = pt.x;
	topBorder = pt.y;
	if (IsIconic(hwnd) == 0 && IsZoomed(hwnd) == 0) {
		RECT WinRect;
		GetWindowRect(hwnd, &WinRect);
		CxLeft = WinRect.left;
		CyTop = WinRect.top;
		CcxWidth = WinRect.right - WinRect.left;
		CcyHeight = WinRect.bottom - WinRect.top;
	}

	if ( cy <= topBorder )	return;

	MoveWindow( hWndLB, horizBorder, topBorder,
				dlgRect.right - dlgRect.left - (2*horizBorder),
				dlgRect.bottom - topBorder, 
				TRUE );

	return;
}

//////////////////////////////////////////////////////////////////////////////

// This records where the window has moved to for the ini file

void Dlg_OnMove (HWND hwnd, int cx, int cy) {
	RECT WinRect;

	if (IsIconic(hwnd) == 0 && IsZoomed(hwnd) == 0) {
		GetWindowRect(hwnd, &WinRect);
		CxLeft = WinRect.left;
		CyTop = WinRect.top;
//		do width and height also to ensure recording in ini file
		CcxWidth = WinRect.right - WinRect.left;
		CcyHeight = WinRect.bottom - WinRect.top;
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////////

BOOL Dlg_OnClose (HWND hwnd) {
//	time to save coordinate values in ini file ?
	winerror("WM_CLOSE");
//	need to return FALSE in this case, since we didn't handle it
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////

BOOL Dlg_OnDestroy (HWND hwnd) {
	hConsoleWnd = NULL;				// redundant ? 
	winerror("WM_DESTROY");
//	need to return FALSE in this case, since we didn't handle it
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////

BOOL Dlg_OnInitDialog (HWND hwnd, HWND hwndFocus, LPARAM lParam) {
	int ptsize;		// what units are these ? 
	HICON hIcon;
	HWND hWndLB = GetDlgItem(hwnd, ICN_LISTBOX);

	if (hWndLB == NULL) winerror("hWndLB NULL");	// should not happen

	if (bHiresScreen) ptsize=13;
	else ptsize=11;
//	Associate an icon with the dialog box 
//	chSETDLGICONS(hwnd, IDI_VMMAP, IDI_VMMAP);
//	hIcon = LoadIcon(GetWindowInstance(hwnd), IDI_APPLICATION); // or "DVIIVON");
//	hIcon = LoadIcon(hInst, IDI_APPLICATION); // or "DVIIVON");
	hIcon = LoadIcon(hInst, "DviIcon");
	if (hIcon != NULL) {
//		following sets the B/W icon
//		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) hIcon);
//		following sets the grey/color icon
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
	}

//	SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
//						 SWP_NOMOVE | SWP_NOSIZE);
//	Make a horizontal scroll bar appear in the list box.
	ListBox_SetHorizontalExtent(hWndLB,
							150 * LOWORD(GetDialogBaseUnits()));

	if (CxLeft != 0 && CyTop != 0 && CcxWidth != 0 && CcyHeight != 0) {
		int horizBorder=4;
//		based on LISTBOX     ICN_LISTBOX, 2, 26, 244, 355, ...
		int topBorder=GetSystemMetrics(SM_CYCAPTION) + 26 + 4;
		int scrollHeight=GetSystemMetrics(SM_CYHSCROLL) + 8;

//		The LISTBOX window can now be positioned and sized
		SetWindowPos(hwnd, NULL, CxLeft, CyTop, CcxWidth, CcyHeight,
				 SWP_NOZORDER);
//		The list box can now be sized
		MoveWindow( hWndLB, horizBorder, topBorder,
				CcxWidth - (2*horizBorder) - 8,
				CcyHeight - topBorder - scrollHeight, TRUE );
	}

//	Switch to fixed width font in list box
	hConsoleFont = CreateFont(-ptsize, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
							  0, 0, 0, FF_MODERN | VARIABLE_PITCH, "Lucida Console");
	if (hConsoleFont == NULL) {
		hConsoleFont = CreateFont(-ptsize, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
								  0, 0, 0, FF_MODERN | VARIABLE_PITCH, "Courier");
	}
	if (hConsoleFont != NULL) 
		SendMessage(hWndLB, WM_SETFONT, (LPARAM) hConsoleFont, 0);
	return(TRUE);
}

/////////////////////////////////////////////////////////////

// This buffers up to LINEBUFFERLEN characters from strings on same line

#define LINEBUFFERLEN 80				// max length of buffered line

int nBuffer=0;							// bytes in buffer so far

char szBuffer[LINEBUFFERLEN]="";		// to buffer up one line of console stuff

void Dlg_OnCommand (HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	HWND hWndLB = GetDlgItem(hwnd, ICN_LISTBOX);

	switch (id) {
		case IDOK:							// clicking OK button
		case IDCANCEL:						// clicking [X] in top right corner
			if (hConsoleFont != NULL) DeleteObject(hConsoleFont);
			hConsoleFont = NULL;
			hConsoleWnd = NULL;
			EndDialog(hwnd, id);
			break;

		case ICN_COPY:						// click COPY button
			CopyControlToClipboard(hWndLB);
			break;

		case ICN_CLEAR:						// click CLEAR button
			SendMessage(hWndLB, LB_RESETCONTENT, 0L, 0L);
			*szBuffer = '\0';
			nBuffer = 0;
			break;
	}
}

// handle our special dialog window functions

BOOL HandleRest (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int nLen, flag;
	char *szLine, *szNewLine;
	HWND hWndLB = GetDlgItem(hwnd, ICN_LISTBOX);

	if (hWndLB == NULL) return (TRUE);		// sanity check

	switch(uMsg) {
		case ICN_SETTITLE:
			SetWindowText(hwnd, (LPSTR) wParam);
			return (TRUE);					// maybe drop through ?

		case ICN_RESET:
			SendMessage(hWndLB, LB_RESETCONTENT, 0L, 0L);
			*szBuffer = '\0';
			nBuffer = 0;
			return (TRUE);

//		Modified to handle multiple lines in single string 2000 June 18
		case ICN_ADDTEXT:
			szLine = (LPSTR) wParam;
#ifdef DEBUGGING
			{
				char *s=szLine;
				wincancel(szLine);
				while (*s != '\0') {
					sprintf(debugstr, "%d ", *s++);
					if (wincancel(debugstr)) break;
				}
			}
#endif
			szNewLine = strchr(szLine, '\n');		// any newlines ?
			if (szNewLine != NULL) *szNewLine = '\0';	// terminate string
			for (;;) {					// break out if no more newlines
				nLen = strlen(szLine);
//				if new stuff won't fit in buffer, dump out buffer first
				if (nBuffer > 0 && nBuffer + nLen + 1 >= LINEBUFFERLEN) {
					SendMessage(hWndLB, LB_ADDSTRING, 0L, (LPARAM) szBuffer);
					*szBuffer = '\0';
					nBuffer = 0;
				}
//				if new stuff too long to fit in buffer, dump out new stuff
//				s = szLine + nLen - 1;
				if (nLen + 1 >= LINEBUFFERLEN) {	// dump long line
//					if (*s == '\n') *s = '\0';
					SendMessage(hWndLB, LB_ADDSTRING, 0L, (LPARAM) szLine);
				}
//			otherwise concatinate it with what is already there
				else {
					strcat(szBuffer, szLine);
					nBuffer += nLen;
//					s = szBuffer + nBuffer - 1;
//					if new stuff ends in \n, then dump out the whole lot now
//					if (*s == '\n') {
//						*s = '\0';
//					if new stuff ends in \n, then dump out the whole lot now
					if (szNewLine != NULL) {
						SendMessage(hWndLB, LB_ADDSTRING, 0L, (LPARAM) szBuffer);
						*szBuffer = '\0';
						nBuffer = 0;
					}
				}
				if (szNewLine == NULL) break;
				*szNewLine = '\n';					// restore newline
				szLine = szNewLine + 1;				// just past last \n
				szNewLine = strchr(szLine, '\n');		// any newlines ?
				if (szNewLine != NULL) *szNewLine = '\0';	// terminate string
			}
			return (TRUE);

		case ICN_DONE:
			flag = (int) wParam;
			if (nBuffer > 0) {			// clear out the buffer
				SendMessage(hWndLB, LB_ADDSTRING, 0L, (LPARAM) szBuffer);
				*szBuffer = '\0';
				nBuffer = 0;
			}
			SendMessage(hWndLB, LB_ADDSTRING, 0L, (LPARAM) "DONE");
			SendMessage(hWndLB, LB_ADDSTRING, 0L, (LPARAM) "");
			if (flag == 0) {				// if no error 
				if (bConsoleOpen == 0)		// and if not keep open
					SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
			}
			return (TRUE);

		 default:
			return (FALSE);
	}
}

/////////////////////////////////////////////////////////////

// #define HANDLE_MSG(hwnd, message, fn)    \
//         case (message): return HANDLE_##message((hwnd), (wParam), (lParam), (fn))

#define chHANDLE_DLGMSG(hwnd, message, fn)  \
		 case (message): return (SetDlgMsgResult(hwnd, uMsg, \
		 HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

// NOTE: this always returns TRUE (meaning it handled the message) --- except for
// WM_INITDIALOG WM_COMPAREITEM WM_VKEYTOITEM  WM_CHARTOITEM  WM_QUERYDRAGICON
// where it returns whatever the message handler returns

// Typically, the dialog box procedure should return TRUE if it processed the
// message, and FALSE if it did not. If the dialog box procedure returns FALSE,
// the dialog manager performs the default dialog operation in response to the message.

/////////////////////////////////////////////////////////////

BOOL CALLBACK _export ConsoleText (HWND hwnd, UINT uMsg,
						WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
		chHANDLE_DLGMSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
		chHANDLE_DLGMSG(hwnd, WM_COMMAND, Dlg_OnCommand);
		chHANDLE_DLGMSG(hwnd, WM_SIZE, Dlg_OnSize);
		chHANDLE_DLGMSG(hwnd, WM_MOVE, Dlg_OnMove);
//		HANDLE_DLGMSG(hwnd, WM_CLOSE, Dlg_OnClose);
//		HANDLE_DLGMSG(hwnd, WM_DESTROY, Dlg_OnDestroy);
		default:
			HandleRest(hwnd, uMsg, wParam, lParam);
	}
	return(FALSE);
}

HWND CreateConsole (HINSTANCE hInst, HWND hWnd) {
	if (hConsoleWnd != NULL) return hConsoleWnd;		// 2000 June 2
//	hConsoleWnd = CreateDialog(hInst, "CONSOLETEXT", NULL, (DLGPROC) ConsoleText);
	hConsoleWnd = CreateDialog(hInst, "ConsoleText", hWnd, ConsoleText);
	return hConsoleWnd;
}

//////////////////////////////////////////////////////////////////////////

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* no longer need to be able to interpret everything - since read only */
/* beginning and end (after `pre' and after `post') */

/* could perhaps be speeded up slightly - using table of chars to skip */

/* check that fonts that have no characters used */
/* (because of page limits) are NOT loaded in dviextra.c */

/* check that nothing but nop and font defs happen between eop and bop ? */

/* doesn't complain if pre not encountered ? IT DOES */

/* search for start of DVI in Textures file a bit crude ? */

/* try and avoid actually giving up if at all possible */

/* catch DVI commands before PRE - OK */

/* catch DVI commands after POST */

/* is TeX comment ever used later ? */

/* may need to initialize everything since multiple calls likely */

/* interacting with font substitution files ? */
/* need to be able to specify on command line ? */

/* do this more simply just by reading END of file */
/* or read but remember all page starts ? */

/* check stack depth against MAXSTACK ? */

/* show current page number, TeX counter[0] in ShowInfo box */

/* implement fast forward mode that only looks for bop ? not safe */

/* may be a problem with this eagerness to make all font names uppercase ... */


	
