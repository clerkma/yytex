/* Copyright 1991,1992 Y&Y, Inc.
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

/* macros for `parsing' of WM_COMMAND parameters wParam & lParam */


// #define GET_WM_COMMAND_CMD(wParam, lParam)	(HIWORD(wParam))
// #define GET_WM_COMMAND_ID(wParam, lParam)	(LOWORD(wParam))
// #define GET_WM_COMMAND_HWND(wParam, lParam)	((HWND)lParam)

// above are in windowsx.h message crackers already 

/* Also need inverse of the above - see e.g. NotifyParent in winfonts.c */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Stuff needed for font selection code  */

/* #define MAXOUTLINEFONTS	512 */ /* max TT fonts in hFullName table */
#define MAXOUTLINEFONTS	1024 /* max TT fonts in hFullName table */

/* #define MAXOUTLINEFACES 256 */ /* max faces - i.e. up to 1024 faces/styles */
#define MAXOUTLINEFACES 512	/* max faces - i.e. up to 2048 faces x styles */

/* #define MAXSEARCHTEXT 64 */	/* maximum length of search text */
#define MAXSEARCHTEXT 80	/* maximum length of search text for CommDlg */

#define MAXCHRS	256			/* max number of characters in font */

#define INFINITY 32767		/* invalid dvipage number */

/* BUFLEN must be greater than MAXPATHLEN */
#define BUFLEN 512			/* length of scratch string char str[BUFLEN]; */

// #define DEBUGLEN 256		/* string for OutputDebugString */
#define DEBUGLEN 512		/* string for OutputDebugString */

#define MAXCOMMENT 256		/* maximum length of comment in DVI file */

#define TEXCOUNTERS 10		/* number of counter[]s in TeX */

#define MAXCHARBUF 64		/* 128 max character string in winanal.c */

#define LONGNAMES 1			/* use _lopen, _lread, _lwrite, _llseek, _lclose */
							/* means HFILE instead of FILE * */

/* Need at least one of NEWPRINTDLG & OLDPRINTDLG */
/* Cannot use OLDPRINTDLG in WIN32 ... */
/* And apparently cannot use NEWPRINTDLG in WIN16 ... */

#define NEWPRINTDLG 1		/* use CommDlg for Printing Dialog Boxes */

#define USEUNICODE
#define MAKEUNICODE

#define AllowCTM

#define MAXFILENAME FILENAME_MAX 	/* 260 stdio.h */
#define MAXPATHLEN _MAX_PATH		/* 260 semi-colon sep path stdlib.h? */
#define MAXDIRLEN _MAX_DIR			/* 256 max length of path comp stdlib.h? */

#define MAXSPECLEN 13				/* "*.DVI" -- used by file open dialog ? */
#define MAXEXTLEN 5					/* ".DVI" --- used by file open dialog ? */

#define MAXFAKENAME 33		/* max name of substitute font */

#define ONEINCH 4736287 	/* 72.72 * 65536 `scaled' points */

#define CRYPT_MUL 52845u	/* pseudo-random number generator multiplier */
#define CRYPT_ADD 22719u	/* pseudo-random number generator addend */

#define REEXEC 55665			/* seed constant for eexec encryption */
/* #define RCHARSTRING 4330 */ 	/* seed constant for charstring encryption */

#define MAXZOOM 18				/* 12 safe ??? */

/* #define MAXZOOM 16 */		/* 12 safe ??? */

#define MINZOOM (-12)			/* (-12) safe ??? */

#define WM_IAMHERE		(WM_USER + 100)
#define WM_GOTOPAGE		(WM_USER + 101)
#define WM_MARKHIT		(WM_USER + 102)
#define WM_GOODBYE		(WM_USER + 103)

#define WM_CHANGESEL	(WM_USER + 200)

/* #define MAXCHARNAME (32+1) */	/* make computation easier 94/Dec/25 */
// #define MAXCHARNAME 32			// not used anymore

#define ATMTABLESIZE (512 * 3 + 8)	/* needed for saved info from ATM */

/* #define MAXDOSERROR	32 */		/* max number of error return GetLibrary */

/* _lopen() flags - not define in winbase.h in WIN32 */

#define READ	    0
#define WRITE       1
#define READ_WRITE  2
