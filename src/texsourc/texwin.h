/**********************************************************************

  Common header information for YANDYTEX

   Copyright 1992,1993,1994,1995,1996,1997,1998,1999 Y&Y, Inc.
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

**********************************************************************/

/////////////////////////////////////////////////////////

#ifdef _WINDOWS

// MYLIBAPI is defined as __declspec(dllexport) in the 
// implementation file (local.c).  Thus when this
// header file is included by local.c, functions will 
// be exported instead of imported.

#ifndef MYLIBAPI
#define MYLIBAPI __declspec(dllimport)
#endif

//////////////////////////////////////////////////////////

// MYLIBAPI int yandytex(HWND, char *);

MYLIBAPI int yandytex(HWND, char *, int (*) (char *, char *, char *));

// last argument is a call-backup that can send PS strings back

#define ICN_LISTBOX		525
#define ICN_COPY		526
#define ICN_RESET		527
#define ICN_ADDTEXT		528
#define ICN_SETTITLE	529
#define ICN_DONE		530
#define ICN_CLEAR		531

// MYLIBAPI int reencodeflag;

#endif

#ifdef _WINDOWS
// #define showline(str,flag) ShowLine(str,flag)
// #define showchar(chr) ShowChar(chr)
int ConsoleInput(char *, char *, char *);
#else
#define showline(str,flag) fputs(str,stdout)
#define showchar(chr) putc(chr, stdout)
#endif

// #define ABORTCHECK if (abortflag) return
#define ABORTCHECK 
// #define ABORTCHECKZERO if (abortflag) return 0
#define ABORTCHECKZERO 

//////////////////////////////////////////////////////////

extern char logline[];
