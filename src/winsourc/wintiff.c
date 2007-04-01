/* Copyright 1995,1996,1997,1998,1999 Y&Y, Inc.
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

#include "winspeci.h"

/* #include "tiff.h" */				/* for BlackIce DLL */

#pragma warning(disable:4100)	// unreferenced formal parameters

/* #define DEBUGLZW */				/* to get in extra tracing code */

/* #define DEBUGTIFF */				/* to get in extra tracing code */

/* #define DEBUGHUFFMAN */		/* to get in extra tracing code */

#define USEMEMCPY

// #ifdef ALLOWSCALE
// #endif

/****************************************************************************/

/* Make PS code from TIFF image file 1, 2, 4, 8 bit pixels only */
/* Support palette color images => expands to RGB */	/* FLUSH !!! */
/* Support 24 bit color images converts RBG to gray if colorimage not avail */
/* Supports: no compression, TIFF_CCITT, LZW, and PACK_BITS */
/* Does not support BitsPerSample other than power of 2 */
/* Does not support compression schemes 3 and 4 (CCITT) */

/****************************************************************************/

int bytes;							/* total bytes per row output used ? */

int CurrentStrip;					/* for debugging purposes */

long BufferLength;					/* number of bytes of intermediate data */
									/* length of lpWinBuffer allocated space */

#define MAXIMAGEDIMEN 65536		/* maximum rows & columns permitted */
#define MAXIMAGESIZE 67108864	/* maximum bytesperrow x rows */

/* #define MAXIMAGEDIMEN 4096 */
/* #define MAXIMAGESIZE 4194304 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int readpackbits (LPSTR, HFILE, int); */

/* int huffmanrow (LPSTR, HFILE, int width); */

/* int DecodeLZW (HFILE input, LPSTR, LONG, FARPROC); */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static char *modname = "WINTIFF"; 

/* static char *modname = __FILE__; */

static void winerror(char *message) {
	HWND hFocus;
	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox(hFocus, message, modname, MB_ICONSTOP | MB_OK);	
}

void winbadimage (char *);	 /* in winspeci.c */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define TIFFBUFLEN 256

static char *lpWinBuffer=NULL;		/* buffer for low level routines */

static char *tifbufptr;		/* pointer to next byte to take out of buffer */

static int tifbuflen;			/* bytes still left in str buffer */

// static int ungotten=-1;		/* result of unwingetc - or -1 */

static long tiffilepos;		/* position of byte in input file */

#ifdef LONGNAMES
static int replenishbuf(HFILE input, int bufferlen) {
	tifbuflen = _lread(input, lpWinBuffer, (unsigned int) bufferlen);
#else
static int replenishbuf(FILE *input, int bufferlen) {
	tifbuflen = fread(lpWinBuffer, 1, (unsigned int) bufferlen, input); 
#endif
	if (tifbuflen < 0) {
		winerror ("Read error in wingetc ");
		finish = -1;
	}
	if (tifbuflen <= 0) return EOF;	/* end of file or read error */
 	tifbufptr = lpWinBuffer;
	return tifbuflen;
}

/* versions of getc and ungetc using low-level C input/output */
/* speed up reading compared to using getc / ungetc */ /* not used */

#ifdef IGNORED
static void unwingetc(int c, HFILE input) { /* ignores `input' */
	if (ungotten >= 0) {
		winerror("Repeated unwingetc"); errcount();
	}
	else tiffilepos--;
	ungotten = c;
}
#endif

#ifdef LONGNAMES
static int wingetc(HFILE input) {
#else
static int wingetc(FILE *input) {
#endif
//	int c;
//	if (ungotten >= 0)  {
//		c = ungotten; ungotten = -1; tiffilepos++; return c;
//	}
//	else
	if (tifbuflen-- > 0) {
		tiffilepos++;
		return (unsigned char) *tifbufptr++;
	}
	else {
		if (replenishbuf(input, TIFFBUFLEN) < 0) return EOF;
		tifbuflen--;
		tiffilepos++; 
		return (unsigned char) *tifbufptr++;
	}
}

#ifdef LONGNAMES
static long wintell(HFILE input) {		/* where are we in the file */
#else
static long wintell(FILE *input) {		/* where are we in the file */
#endif
	return tiffilepos;
}

/* possibly check for whether new position is somewhere in buffer ? */

#ifdef LONGNAMES
static long winseek(HFILE input, long place) {
#else
static long winseek(FILE *input, long place) {
#endif
	long foo;
 
	if (tiffilepos == place) return place;
	if (place < 0) {
		sprintf(debugstr, "Negative seek %ld", place);
		winerror(debugstr);
		return 0;
	}
	foo = _llseek(input, place, SEEK_SET);
	if (foo != place) {
		sprintf(debugstr, "Seek error: to %ld ", place);
/*		if (errno == EBADF)  strcat(buffer, "invalid file handle"); */
/*		if (errno == EINVAL) strcat(buffer, "invalid origin or position"); */
		winerror(debugstr);
	}
	tiffilepos = place;
//	ungotten = -1;
	tifbuflen = 0;
	tifbufptr = lpWinBuffer;	/* redundant ? */
	return foo;
}

static void winendit(void) {
#ifdef DEBUGTIFF
	if(bDebug > 1) OutputDebugString("winendit\n");
#endif
	if (lpWinBuffer == NULL) winerror("Buffer Error");
	else {
		LocalFree ((HLOCAL) lpWinBuffer);
		lpWinBuffer = NULL;
	}
}

#ifdef LONGNAMES
static int wininit(HFILE input) {	/* ignores `input' */
#else
static int wininit(FILE *input) {	/* ignores `input' */
#endif
#ifdef DEBUGTIFF
	if(bDebug > 1) OutputDebugString("wininit\n");
#endif
	if (lpWinBuffer != NULL) {
		winerror("Buffer Error");
		winendit();
	}
	lpWinBuffer = (char *) LocalAlloc(LMEM_FIXED, TIFFBUFLEN);
	tifbuflen = 0;					/* nothing buffered yet */
	if (lpWinBuffer == NULL) {		/* 1996/May/12 */
		return -1;				/* indicate allocation failure to caller */
	}
	tifbufptr = lpWinBuffer;		/* redundant ? */
	return 0;
}

/****************************************************************************/

/* Here is the code for LZW (compression scheme number 5) */

#define MAXTABLE 4096					/* to deal with up to 12 bit codes */

#define MAXCHR 256						/* 2^8 possible bytes */

#define CLEAR 256						/* clear string table code */

#define EOD 257							/* end of data code */

#define FIRST 258						/* first code available - new string */

HGLOBAL hStringByte=NULL;
HGLOBAL hStringFirst=NULL;
HGLOBAL hStringLength=NULL;
HGLOBAL hStringPrevious=NULL;

char *StringByte=NULL;			/* contains last byte of string */

char *StringFirst=NULL;			/* contains first byte of string */
										/* to speed up processing ... */

#ifdef USESHORTINT
short int *StringPrevious=NULL;		/* points to previous char of string */
short int *StringLength=NULL;		/* length of string */
										/* to speed up processing ... */
#else
int *StringPrevious=NULL;			/* points to previous char of string */
int *StringLength=NULL;				/* length of string */
										/* to speed up processing ... */
#endif

int TableIndex=FIRST;					/* index into above String Tables */
										/* next available string entry */

int CodeLength=9;						/* current code length INPUT */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following is never called ? should call from dviwindo.c when exiting ? */

void DeAllocStringsIn (void) {				/* remember to do this at end */
/*	if (Memory != NULL) _ffree(Memory); Memory = NULL; */
/*	if (StringByte != NULL) */
	if (hStringByte != NULL) {
/*		_ffree(StringByte); StringByte = NULL; */
		GlobalUnlock(hStringByte);
		hStringByte = GlobalFree(hStringByte);
	}
/*	if (StringFirst != NULL) */
	if (hStringFirst != NULL) {
/*		_ffree(StringFirst); StringFirst = NULL; */
		GlobalUnlock(hStringFirst);
		hStringFirst = GlobalFree(hStringFirst);
	}
/*	if (StringPrevious != NULL) */
	if (hStringPrevious != NULL) {
/*		_ffree(StringPrevious); StringPrevious = NULL; */
		GlobalUnlock(hStringPrevious);
		hStringPrevious = GlobalFree(hStringPrevious);
	}
/*	if (StringLength != NULL) */
	if (hStringLength != NULL) {
/*		_ffree(StringLength); StringLength = NULL; */
		GlobalUnlock(hStringLength);
		hStringLength = GlobalFree(hStringLength);
	}
}

/* worry here about allocation in NT ? sizeof(int) */

int AllocStringsIn (void) {				
/*	if (Memory == NULL) {		
		Memory = (char *) _fmalloc(INIMEMORY);
		MemorySize = INIMEMORY;		
	} */ /* memory for strings in string table */
/*	if (StringByte == NULL) *//* allocate string table indeces */
	if (hStringByte == NULL) {	/* allocate string table indeces */
/*		StringByte = (char *) _fmalloc(MAXTABLE * sizeof(char)); */
		hStringByte =  GlobalAlloc(GMEM_MOVEABLE, MAXTABLE * sizeof(char));
		StringByte = (char *) GlobalLock(hStringByte);
	}
/*	if (StringFirst == NULL) */	/* allocate string table indeces */ 
	if (hStringFirst == NULL) {	/* allocate string table indeces */
/*		StringFirst = (char *) _fmalloc(MAXTABLE * sizeof(char)); */
		hStringFirst = GlobalAlloc(GMEM_MOVEABLE, MAXTABLE * sizeof(char));
		StringFirst = (char *) GlobalLock(hStringFirst);
	}
/*	should this be short int in NT ? */
#ifdef USESHORTINT
/*	if (StringPrevious == NULL) */	/* allocate string table lengths */ 
	if (hStringPrevious == NULL) {	/* allocate string table lengths */
/*		StringPrevious = (short int *)
						 _fmalloc(MAXTABLE * sizeof(short int)); */
		hStringPrevious = GlobalAlloc (GMEM_MOVEABLE, MAXTABLE * sizeof(short int));
		StringPrevious = (short int *) GlobalLock(hStringPrevious);
	}
/*	if (StringLength == NULL) */	/* allocate string table lengths */ 
	if (hStringLength == NULL) {	/* allocate string table lengths */
/*		StringLength = (short int *)
					   _fmalloc(MAXTABLE * sizeof(short int)); */
		hStringLength = GlobalAlloc(GMEM_MOVEABLE, MAXTABLE * sizeof(short int));
		StringLength = (short int *) GlobalLock(hStringLength);
	}
#else
/*	if (StringPrevious == NULL) */	/* allocate string table lengths */
	if (hStringPrevious == NULL) {	/* allocate string table lengths */
/*		StringPrevious = (int *) _fmalloc(MAXTABLE * sizeof(int)); */
		hStringPrevious = GlobalAlloc(GMEM_MOVEABLE, MAXTABLE * sizeof(int));
		StringPrevious = (int *) GlobalLock(hStringPrevious);
	}
/*	if (StringLength == NULL) */	/* allocate string table lengths */ 
	if (hStringLength == NULL) {	/* allocate string table lengths */
/*		StringLength = (int *) _fmalloc(MAXTABLE * sizeof(int)); */
		hStringLength = GlobalAlloc(GMEM_MOVEABLE, MAXTABLE * sizeof(int));
		StringLength = (int *) GlobalLock(hStringLength);
	}
#endif
/*	if (Memory == NULL || StringTable == NULL ||  StringLength == NULL) */
	if (StringByte == NULL || StringFirst == NULL ||
		StringPrevious == NULL || StringLength == NULL) {
		winerror("Unable to allocate memory");
		PostQuitMessage(0);
/*		checkexit(1); */
/*		return (-1); */
	}
	return 0;
}

/* work around for new MS C compiler bug */

#pragma optimize ("lge", off) 

void InitializeStringTable (void) {		/* set up string table initially */
	int k;

	AllocStringsIn();					/* grab memory for tables if needed */
//	What if it failed ???
	for (k = 0; k < MAXCHR; k++) {	/* 256 single byte strings */
		StringByte[k] = (char) k;
		StringFirst[k] = (char) k;
		StringPrevious[k] = -1;		/* indicate beginning of string */
		StringLength[k] = 1;
	}
/*	FreeIndex = MAXCHR; */
	TableIndex = FIRST;
	CodeLength = 9;					/* initial code length */
}

#pragma optimize ("lge", on) 

void ResetStringTable (int quietflag) {		/* clear string table */
/*	int k; */

#ifdef DEBUGTIFF
	if (!quietflag) {
		if (bDebug > 1) {
			if (TableIndex <= 258) OutputDebugString("CLEAR\n"); 
			else {
				sprintf(debugstr, "CLEAR TableIndex %d CodeLength %d\n",
						TableIndex, CodeLength);
				OutputDebugString(debugstr);
			}
		}
	} 
#endif
/*  following not really needed */
/*	for (k = FIRST; k < TableIndex; k++) { 
		StringTable[k] = 0;
		StringLength[k] = 0;
	} */
/*	FreeIndex = MAXCHR; */
	TableIndex = FIRST;
	CodeLength = 9;
}

#ifdef USESHORTINT
void AddNewEntry (short int OldCode, short int Code) {
#else
void AddNewEntry (int OldCode, int Code) {
#endif
/*	char *s; */
/*	char *t; */
/*	int k; */

#ifdef DEBUGLZW
	if (bDebug)	{
		sprintf(debugstr, "Add string %d + %d TableIndex %d\n",
				OldCode, Code, TableIndex);
		OutputDebugString(debugstr);
	}
#endif

/*	This is where we enter new one in table */
/*	StringTable[TableIndex] = FreeIndex; */
/*	StringLength[TableIndex] = len; */
	StringByte[TableIndex] = StringFirst[Code];
	StringFirst[TableIndex] = StringFirst[OldCode];
	StringPrevious[TableIndex] = OldCode;
#ifdef USESHORTINT
	StringLength[TableIndex] =
				(unsigned short int) (StringLength[OldCode] + 1); 
#else
	StringLength[TableIndex] = StringLength[OldCode] + 1;
#endif
	TableIndex++;
/*	s = Memory + FreeIndex; */
/*	t = Memory + StringTable[OldCode]; */
/*	for (k = 0; k < len-1; k++) *s++ = *t++; */
/*	t = Memory + StringTable[Code];	 */
/*	*s = *t; */					/* last byte comes from next code string */
/*	FreeIndex += len; */

	if (TableIndex == 511 || TableIndex == 1023 || TableIndex == 2047) {
		CodeLength++;
#ifdef DEBUGTIFF
		if (bDebug >1) {
			sprintf(debugstr, "CODELENGTH++ TableIndex %d CodeLength %d\n", 
					TableIndex, CodeLength);
			OutputDebugString(debugstr);
		}
#endif
	} 
	if (TableIndex > MAXTABLE) {
		winerror("ERROR: Table overflow");
		PostQuitMessage(0);
/*		checkexit(1); */
	}
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int IsInTable (int Code) {			/* is code already in table ? */
	return (Code >= 0 && Code < TableIndex);
}

HGLOBAL hStripData;

unsigned char *StripData;	/* address of Strip Buffer */

unsigned int StripDataLen;		/* length of Strip Buffer */

long InStripLen;				/* length of Strip in file 96/Nov/17 */

/* copy string from table by code number */

unsigned char *WriteString (unsigned char *buffer, int Code) { 
/*	int k; */
	int n, len=0;
	unsigned char *s;
	unsigned char *t;

/*	s = buffer; */
/*	t = Memory + StringTable[Code]; */
//	if (Code < TableIndex)
	len = StringLength[Code]; 
	t = buffer + len;					/* byte after last one to copy */
	if (t > StripData + StripDataLen) {
/*		special case kludge if terminates right after code length switch */
		if (((unsigned int) (buffer - StripData) == StripDataLen) &&
// kludge to try and deal with lack of code length increase just before EOI
			  ((Code == (EOD*2)) || (Code == (EOD*4))))  // 2000/Feb/4
		{
			return buffer;
		}	/* 98/Sep/22 */
/*		winerror("ERROR: ran off end of Strip Buffer"); */
		sprintf(debugstr,
	"ERROR: Strip Buffer len %d code %d EOD %d CodeLength %d TableIndex %d",
			len, Code, EOD, CodeLength, TableIndex);
		winerror(debugstr);
		return buffer;
	}
	s = t - 1;							/* last byte to copy */
/*	for (k = 0; k < len; k++) *s++ = *t++; */
	n = Code;
/*	for (k = 0; k < len; k++) */
/*	for (k = len; k > 0; k--) */
	while (n >= 0) {
/*		if (k != StringLength[n])
			printf("k %d <> len %d ", k, StringLength[n]); */ /* check */
		*s-- = StringByte[n];						/* copy the byte */
/*		if (StringPrevious[n] < 0 && k != 1)
			printf("k %d n %d Code %d ", k, n, Code); */
		n = StringPrevious[n];
/*		if (n < 0) break; */						/* termination */
	}
	if ((s + 1) != buffer) {						/* sanity check */
		int err;
		if ((s+1) > buffer) err = (s+1) - buffer;
		else err = buffer - (s+1);
		sprintf(debugstr, "Off by %d (len %d Code %d)\n", err, len, Code);
		winerror(debugstr);
	}
/*	s = buffer;									
	while (s < t) {
		if (*s++ != 0) putc('1', stdout);
		else putc('0', stdout);
		if (counter++ % 1024 == 0) putc('\n', stdout);
	}
	return buffer; */ 								/* TESTING HACK! */
	return t;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long OldByte=0;		/* cadaver being eaten */
int OldBitsLeft=0;				/* how many bits left in OldByte */

#ifdef USESHORTINT
short int GetNextCode (HFILE input) {	/* get next LZW code number from input */
#else
int GetNextCode (HFILE input) {	/* get next LZW code number from input */
#endif
	int bits;
	int c;
	unsigned long k;

	bits = OldBitsLeft;					/*  how many bits do we have */
	k = OldByte;						/*  start with old bits */
	
	while (bits < CodeLength) {			/* need more bits ? */
		if (InStripLen-- <= 0) {		/* treat as EOD 96/Nov/17 */
/*		if (traceflag) printf("No EOD at end of strip %d ", CurrentStrip); */
/*		winerror("No EOD at end of strip"); */
			return EOD;
		}
		if ((c = wingetc(input)) < 0) { 
			winerror("Unexpected EOF (GetNextCode)");
/*			checkexit(1); */
/*			PostQuitMessage(0); */ 		/* pretty serious ? */
			finish = -1;
			return -1;					/* EOF serious error ... */
		} 
		k = (k << 8) | c;
		bits += 8;
	}
	OldByte = k;
	OldBitsLeft = bits - CodeLength;		/* extra bits not used */
/*	OldByte = OldByte & ((1 << OldBitsLeft) - 1); *//* redundant */
	k = k >> OldBitsLeft;					/* shift out extra bits */
	k = k & ((1 << CodeLength) - 1);		/* mask out high order */
#ifdef DEBUGLZW
	if (bDebug > 1) {
		sprintf(debugstr, "CODE %d ", k);
		OutputDebugString(debugstr);
	}
#endif

#ifdef USESHORTINT
	return (short int) k;
#else
	return (int) k;
#endif
}

#ifdef LONGNAMES
void LZWdecompress (unsigned char *StripData, HFILE input) {
#else
void LZWdecompress (unsigned char *StripData, FILE *input) {
#endif
#ifdef USESHORTINT
	short int Code, OldCode;
#else
	int Code, OldCode;
#endif
	unsigned int nlen;
	unsigned char *s = StripData;

	OldCode = 0;						/* to keep compiler happy */
/*	InitializeStringTable(); */			/* assume already done once */
/*	ResetStringTable(1); */				/* not needed - first code CLEAR */
	while ((Code = GetNextCode(input)) != EOD) {
		if (Code == -1) {				// error check only
			winerror("Premature end of LZW");
			return;
		}
// #ifdef DEBUGLZW
//		if (bDebug > 1) {
//			sprintf(debugstr, "%d ", Code);
//			OutputDebugString(debugstr);
//		}
// #endif */
		if (Code == CLEAR) {
			ResetStringTable(0);
			Code = GetNextCode(input);
//			if (traceflag) showline("\n", 0);
			if (Code == -1) {			// error check only
				winerror("Premature end of LZW (after CLEAR)");
				return;
			}
			if (Code == EOD) break;
			s = WriteString(s, Code);
			OldCode = Code;
		}								/* end of CLEAR case */
		else {
//			if (IsInTable(Code)) {
			if (Code >= 0 && Code < TableIndex) {
/*				AddTableEntry(StringTable(OldCode)
					+ FirstChar(StringFromCode(Code);)); */
				AddNewEntry(OldCode, Code);
				s = WriteString(s, Code);
				OldCode = Code;
			}							/* end of Code in Table case */
			else {						/* Code is *not* in table */
/*				OutString = StringFromCode (OldCode) +
					+ FirstChar(StringFromCode(Code);)); */
				if (Code > TableIndex) {
// kludge to try and deal with lack of code length increase just before EOI
					Code = Code / 2;		// Try and deal with bad ending
					OldBitsLeft++;
//					CodeLength--;
					if (bDebug) {
						sprintf(debugstr,	"Code (%d) > TableIndex (%d)",
								Code, TableIndex);
						winerror(debugstr);
					}
//					break;			/* ugh! */
				}
/*				strcpy(Omega, StringFromCode(OldCode));
				ConcatOneChar(Omega, StringFromCode(OldCode));
				WriteString(Omega, output);
				AddTableEntry(Omega); */
				AddNewEntry(OldCode, OldCode); 
				s = WriteString(s, Code);
				OldCode = Code;
			} /* end of Code *not* in Table case */
		} /* end of *not* CLEAR case */
	} /* end of not EOD loop */

/*	if (bExpandColor) nlen = StripDataLen / 3;
	else nlen = StripDataLen; */ /* ??? */
	if (CurrentStrip < StripsPerImage-1) {
		nlen = StripDataLen;
		if ((unsigned int) (s - StripData) != nlen) {
#ifdef DEBUGTIFF
			if (bDebug > 1) {  /*	NOTE: mismatch can occur on last strip - is OK */
				sprintf(debugstr,
					"Strip data mismatch %u < %u bytes (Strip %d of %ld)",
						(s - StripData), nlen, CurrentStrip, StripsPerImage);
				OutputDebugString(debugstr); /* winerror ? */
			}
#endif
		}
	}

/*	reset table for next one */
	ResetStringTable(0);
#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr, "RESET TABLE at byte %ld in file\n",
/*				ftell(input)); */
				wintell(input));
		OutputDebugString(debugstr);
	}
#endif
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

long indirectvalue(unsigned int, long, long, HFILE);	/* in winspeci.c */

/* int typesize[6] = {0, 1, 1, 2, 4, 8}; */	/* units of length/count */


/* Set up StripOffset & StripByteCount for strip number k */
/* and go to start of data for strip number k */
/* First one was already set up - don't disturb in case not indirect */
/* In practice, strips are usually contiguous */

#ifdef LONGNAMES
long SetupStrip (HFILE input, int k) {
#else
long SetupStrip (FILE *input, int k) {
#endif
//	long present;
/*	we already have the correct values for k == 0 ? and position ? */
	if (k > 0) {
//		present = wintell(input);	/* redundant */
		StripOffset = indirectvalue(StripOffsetsType, 1,
			StripOffsetsPtr + k * typesize[StripOffsetsType], input);
		StripByteCount = indirectvalue(StripByteCountsType, 1,
			StripByteCountsPtr + k * typesize[StripByteCountsType], input);
//		winseek(input, present);	/* redundant */
	}
#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr, "SETUPSTRIP Strip %d Offset %ld ByteCount %ld\n",
				k, StripOffset, StripByteCount);
		OutputDebugString(debugstr);
	}
#endif
	if (winseek (input, (long) StripOffset + tiffoffset) == 0) {
		winerror("Error in seek to StripOffset");	/* ??? */
		finish = -1;
		return -1;
	}
	if (k == 0 && StripByteCount < 0)
		StripByteCount = 0XFFFFFF;		// kludge if missing 2000/Apr/13
	return StripByteCount;				/* 96/Nov/17 */
}

/* Copy a row from space used by LZW to near space used for output */

void CopyRow (unsigned char *lpBuffer, unsigned char *StripData,
		long InRowLength) {
	int n;
	unsigned char *s=lpBuffer;
	unsigned char *t=StripData;
#ifndef USEMEMCPY
	int k;
#endif

	n = (int) InRowLength;		/* assuming row not longer than 32k */
/*	if (InRowLength > BufferLength) */ /* DEBUGGING 95/Nov/10 */
	if (InRowLength > BufferLength+1) {  /* DEBUGGING 95/Nov/10 */
		sprintf(debugstr,	"ERROR: InRowLength %ld > BufferLength %ld",
				InRowLength, BufferLength);
		winerror(debugstr);
/*		return;	*/
		n = (int) BufferLength;			 /* prevent heap corruption */
	}
#ifdef USEMEMCPY
	(void) memcpy(s, t, n);
/*	s += n; t += n; */
#else
	for (k = 0; k < n; k++) *s++ = *t++; 
#endif
}

/* A whole strip is treated as one unit for LZW encoding ... */
/* So need to do once per strip and need memory for strip output */

#ifdef LONGNAMES
int DecodeLZW (HFILE input,
#else
int DecodeLZW (FILE *input,
#endif
			 unsigned char *lpBuffer, long LineFunParam, FARPROC CopyLineFun) { 
	int k, i, j, n, flag = 0;
	int row = 0;
	long nlen;
	int ret;
/*	unsigned char *StripData; */

	nlen = (long) BufferLength * RowsPerStrip;
#ifdef IGNORED
/*	is this 65535U limit still valid in NT ? _fmalloc limitation ? */
	if (nlen > 65535U) {
		sprintf(debugstr, "Cannot handle Strip Buffer of %lu bytes\n", nlen);
		winerror(debugstr);
/*		nlen = 30000U;	*/				/* TESTING HACK! */
/*		checkexit(1); */
		PostQuitMessage(0); 		/* pretty serious ! */
		finish = -1;
		return -1;					/* serious error ... */
	}
#endif
	StripDataLen = (unsigned int) nlen;
#ifdef DEBUGTIFF
	if (bDebug > 1){						/* debugging */
		sprintf(debugstr, " Allocating %u bytes for Strip Buffer\n", StripDataLen); 
		OutputDebugString(debugstr);
	}
#endif
/*	StripData = (unsigned char *) _fmalloc(StripDataLen); */
	hStripData = GlobalAlloc(GMEM_MOVEABLE, StripDataLen);
	StripData = (unsigned char *) GlobalLock(hStripData);
/*		_fmalloc((unsigned int) (InRowLength * RowsPerStrip)); */
/*		_fmalloc((unsigned int) (BufferLength * RowsPerStrip)); */
	if (StripData == NULL) {
		winerror("Unable to allocate memory");	/* 1995/July/15 */
/*		checkexit(1); */
		PostQuitMessage(0);
	}
	InitializeStringTable(); 

	row = 0;
	for (k = 0; k < StripsPerImage; k++) {
		CurrentStrip = k;					/* global for debug output */
/*		SetupStrip(input, k); */
		InStripLen = SetupStrip(input, k);	/* save GetNextCode 96/Nov/17 */
/*		{
			sprintf(debugstr, "StripOffset %d StripByteCount %d InStripLen %d present %d", 
					StripOffset, StripByteCount, InStripLen, wintell(input));
			winerror(debugstr);
		} */
		OldByte = 0;
		OldBitsLeft = 0;
		ResetStringTable(1);				/* redundant ? */
		LZWdecompress(StripData, input);
/*		{
			int k;
			char *s=debugstr;
			for (k = 0; k < 256; k++) {
				sprintf(s, "%d ", StripData[k]);
				s += strlen(s);
			}
			winerror(debugstr);
		} */
		n = RowsPerStrip;
		if (row + n > ImageLength) n = (int) (ImageLength - row);
		for (i = 0; i < n; i++) {
			CopyRow(lpBuffer, StripData + i * InRowLength, InRowLength);
			if (ExtraSamples > 0)				/* 99/May/10 */
				(void) RemoveExtraSamples(lpBuffer, InRowLength);	/* ? */
			if (Predictor == 2) {		/* only applies to LZW images */
/*				We will assume here that BitsPerSample == 8 ... */
/*				if (SamplesPerPixel == 1) */			/* gray level */
				if (SamplesPerPixelX == 1) {			/* gray level */
/*					for (j = 1; j < InRowLength; j++) */
					for (j = 1; j < InRowLengthX; j++)
/*						lpBuffer[j] += lpBuffer[j-1]; */
						lpBuffer[j] = (unsigned char) (lpBuffer[j] + lpBuffer[j-1]);
				}
				else {	/* RGB (3) or CMYK (4) */
/*					for (j = SamplesPerPixel; j < InRowLength; j++) */
					for (j = SamplesPerPixelX; j < InRowLengthX; j++)
/*						lpBuffer[j] += lpBuffer[j-SamplesPerPixel]; */
						lpBuffer[j] =
/*							 (unsigned char) (lpBuffer[j] + lpBuffer[j-SamplesPerPixel]); */
							 (unsigned char) (lpBuffer[j] + lpBuffer[j-SamplesPerPixelX]);
				}
			}		
/*			if (ProcessRow (output, lpBuffer, InRowLength, BufferLength,
				OutRowLength) != 0) break; */ /* ??? */
/*			if (bCompressColor)
				compresscolorfar(StripData + i * BufferLength, InRowLength);
			if (writearowfar (output, StripData + i * BufferLength,
				OutRowLength) != 0) {
				flag = 1;
				break;
			}
			if (flag) break; */
/*			{
				int k;
				char *s=debugstr;
				for (k = 0; k < 16; k++) {
					sprintf(s, "%d ", lpBuffer[k]);
					s += strlen(s);
				}
				winerror(debugstr);
			} */
/*			ret = CopyLineFun(lpBuffer, row+i, LineFunParam); */ /* callback */
			ret = CopyLineFun(lpBuffer, row, LineFunParam);  /* callback */
			if (ret != 0) break;			/* if something went wrong ... */
			row++;
/*			check on abort flag here somewhere also ? */
		}	/* end of stepping over rows in this strip */
/*		row += n; */
	}	/* end of stepping over all strips in image */
	ResetStringTable(1);				/* redundant ? */
#ifdef DEBUGTIFF
	if (bDebug > 1) {					/* debugging */
		sprintf(debugstr, " Freeing %u bytes Strip Buffer\n", StripDataLen); 
		OutputDebugString(debugstr);
	}
#endif
/*	_ffree(StripData); */
	GlobalUnlock(hStripData);
/*	StripData = NULL; */				/* debugging 95/Nov/10 */
	hStripData = GlobalFree(hStripData);
	return flag;
}

/* InitializeStringTable AllocString - may need to DeAllocString eventually */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Implement 1d CCITT Huffman code decompression (minus EOL) TIFF CCITT */
/* ==> implies BitsPerSample == 1 */
/* white runs = 0, black runs = 1 */
/* here normal PhotometricInterpretation == 0 */
/* invert black/white if PhotometricInterpretation == 1 */

int nbyte, bitsleft;				/* input buffering in splitting bits */

#ifdef LONGNAMES
int getbit (HFILE input) {
#else
int getbit (FILE *input) {
#endif
	if (bitsleft-- <= 0) {
/*		if ((nbyte = getc(input)) == EOF) */
		if ((nbyte = wingetc(input)) == EOF) {
			winerror("Unexpected EOF");
/*			checkexit(1); */			/* Wait for it to create bad code */
		}
		bitsleft = 7;
	}
	nbyte = nbyte << 1;
#ifdef DEBUGHUFFMAN
	if (bDebug > 1) {
		sprintf(debugstr, "%d", (nbyte & 256) ? 1 : 0);
		OutputDebugString(debugstr);
	}
#endif
	if (nbyte & 256) return 1;
	else return 0;
}

/* #pragma optimize ("lge", off) */

/* Compiler screws up the following code if optimizations turned on */
/* Actually commonmake may be OK with compiler optimizations ON */

#ifdef LONGNAMES
int commonmake (HFILE input) { /* common black/white make up codes (7 zeros) */
#else
int commonmake (FILE *input) { /* common black/white make up codes (7 zeros) */
#endif
#ifdef DEBUGTIFF
	if (bDebug > 1) OutputDebugString ("commonmake entry "); 
#endif
	if (getbit(input)) {	/* 00000001 */
		if (getbit(input)) {	/* 000000011 */
			if (getbit(input)) {	/* 0000000111 */
				if (getbit(input)) {	/* 00000001111 */
					if (getbit(input)) {	/* 000000011111 */
						return 2560;
					}
					else { /* 000000011110 */
						return 2496;
					}
				}
				else { /* 00000001110 */
					if (getbit(input)) {	/* 000000011101 */
						return 2432;
					}
					else { /* 000000011100 */
						return 2368;
					}
				}
			}
			else { /* 0000000110 */
				if (getbit(input)) {	/* 00000001101 */
					return 1920;
				}
				else { /* 00000001100 */
					return 1856;
				}
			}
		}
		else { /* 000000010 */
			if (getbit(input)) {	/* 0000000101 */
				if (getbit(input)) {	/* 00000001011 */
					if (getbit(input)) {	/* 000000010111 */
						return 2304;
					}
					else { /* 000000010110 */
						return 2240;
					}
				}
				else { /* 00000001010 */
					if (getbit(input)) {	/* 000000010101 */
						return 2176;
					}
					else { /* 000000010100 */
						return 2112;
					}
				}
			}
			else { /* 0000000100 */
				if (getbit(input)) {	/* 00000001001 */
					if (getbit(input)) {	/* 000000010011 */
						return 2048;
					}
					else { /* 000000010010 */
						return 1984;
					}
				}
				else { /* 00000001000 */
					return 1792;
				}
			}
		}
	}
	else { /* 00000000 */
/*	Actually, EOL code is not supposed to be used in TIFF compression 2 */
		if (!getbit(input)) { /* 000000000 */
			if (!getbit(input)) { /* 0000000000 */
				if (!getbit(input)) { /* 00000000000 */
					if (getbit(input)) { /* 000000000001 */
						winerror("EOL");
						return 0;				/* ??? */
					}
				}
			}
		}
	}
	winerror("Impossible make-up run");
	return -1;	/* error */
}

/* Compiler screws up the following code if optimizations turned on */

#ifdef LONGNAMES
int whiterun (HFILE input) {
#else
int whiterun (FILE *input) {
#endif
#ifdef DEBUGHUFFMAN
	if (bDebug > 1) OutputDebugString ("whiterun entry "); 
#endif
	if (getbit(input)) {	/* 1 */
		if (getbit(input)) {	/* 11 */
			if (getbit(input)) {	/* 111 */
				if (getbit(input)) {	/* 1111 */
					return 7;
				}
				else {			/* 1110 */
					return 6;
				}
			}
			else {			/* 110 */
				if (getbit(input)) {	/* 1101 */
					if (getbit(input)) { /* 11011 */
						return 64;		/* make up */
					}
					else {			/* 11010 */
						if (getbit(input)) { /* 110101 */
							return 15;
						}
						else {			/* 110100 */
							return 14;
						}
					}
				}
				else {			/* 1100 */
					return 5;
				}
			}
		}
		else {			/* 10 */
			if (getbit(input)) {	/* 101 */
				if (getbit(input)) {	/* 1011 */
					return 4;
				}
				else {			/* 1010 */
					if (getbit(input)) { /* 10101 */
						if (getbit(input)) { /* 101011 */
							return 17;
						}
						else {			/* 101010 */
							return 16;
						}
					}
					else {			/* 10100 */
						return 9;
					}
				}
			}
			else {			/* 100 */
				if (getbit(input)) {	/* 1001 */
					if (getbit(input)) { /* 10011 */
						return 8;
					}
					else {			/* 10010 */
						return 128;	/* make up */
					}
				}
				else {			/* 1000 */
					return 3;
				}
			}
		}
	}
	else {			/* 0 */
		if (getbit(input)) {	/* 01 */
			if (getbit(input)) {	/* 011 */
				if (getbit(input)) {	/* 0111 */
					return 2;
				}
				else {			/* 0110 */
					if (getbit(input)) { /* 01101 */
						if (getbit(input)) { /* 011011 */
							if (getbit(input)) { /* 0110111 */
								return 256;	/* make up */
							}
							else {			/* 0110110 */
								if (getbit(input)) {	/* 01101101 */
									if (getbit(input)) {	/* 011011011 */
										return 1408;	/* make up */
									}
									else { /*  011011010 */
										return 1344;	/* make up */
									}
								}
								else {			/* 01101100 */
									if (getbit(input)) {	/* 011011001 */
										return 1280;	/* make up */
									}
									else { /* 011011000 */
										return 1216;	/* make up */
									}
								}
							}
						}
						else { /* 011010 */
							if (getbit(input)) { /* 0110101 */
								if (getbit(input)) { /* 01101011 */
									if (getbit(input)) { /* 011010111 */
										return 1152;	/* make up */
									}
									else {			/* 011010110 */
										return 1088;	/* make up */
									}
								}
								else { /* 01101010 */
									if (getbit(input)) { /* 011010101 */
										return 1024;	/* make up */
									}
									else { /* 011010100 */
										return 960; /* make up */
									}
								}
							}
							else { /* 0110100 */
								if (getbit(input)) { /* 01101001 */
									if (getbit(input)) { /* 011010011 */
										return 896;	/* make up */
									}
									else { /* 011010010 */
										return 832;	/* make up */
									}
								}
								else { /* 01101000 */
									return 576;	/* make up */
								}
							}
						}
					}
					else { /* 01100 */
						if (getbit(input)) { /* 011001 */
							if (getbit(input)) { /* 0110011 */
								if (getbit(input)) { /* 01100111 */
									return 640;	/* make up */
								}
								else { /* 01100110 */
									if (getbit(input)) { /* 011001101 */
										return 768;	/* make up */
									}
									else { /* 011001100 */
										return 704;	/* make up */
									}
								}
							}
							else { /* 0110010 */
								if (getbit(input)) { /* 01100101 */
									return 512;	/* make up */
								}
								else { /* 01100100 */
									return 448;	/* make up */
								}
							}
						}
						else { /* 011000 */
							return 1664;	/* make up */
						}
					}
				}
			}
			else {			/* 010 */
				if (getbit(input)) {	/* 0101 */
					if (getbit(input)) { /* 01011 */
						if (getbit(input)) { /* 010111 */
							return 192;		/* make up */
						}
						else { /* 010110 */
							if (getbit(input)) { /* 0101101 */
								if (getbit(input)) { /* 01011011 */
									return 58;
								}
								else { /* 01011010 */
									return 57;
								}
							}
							else { /* 0101100 */
								if (getbit(input)) { /* 01011001 */
									return 56;
								}
								else { /* 01011000 */
									return 55;
								}
							}
						}
					}
					else {			/* 01010 */
						if (getbit(input)) { /* 010101 */
							if (getbit(input)) { /* 0101011 */
								return 25;
							}
							else {			/* 0101010 */
								if (getbit(input)) { /* 01010101 */
									return 52;
								}
								else { /* 01010100 */
									return 51;
								}
							}
						}
						else {			/* 010100 */
							if (getbit(input)) { /* 0101001 */
								if (getbit(input)) { /* 01010011 */
									return 50;
								}
								else { /* 01010010 */
									return 49;
								}
							}
							else {			/* 0101000 */
								return 24;
							}
						}
					}
				}
				else {			/* 0100 */
					if (getbit(input)) {	/* 01001 */
						if (getbit(input)) { /* 010011 */
							if (getbit(input)) { /* 0100111 */
								return 18;
							}
							else {			/* 0100110 */
								if (getbit(input)) {	/* 01001101 */
									if (getbit(input)) {	/* 010011011 */
										return 1728;	/* make up */
									}
									else { /* 010011010 */
										return 1600;	/* make up */
									}
								}
								else { /* 01001100 */
									if (getbit(input)) {	/* 010011001 */
										return 1536;	/* make up */
									}
									else { /* 010011000 */
										return 1472;	/* make up */
									}
								}
							}
						}
						else {			/* 010010 */
							if (getbit(input)) { /* 0100101 */
								if (getbit(input)) { /* 01001011 */
									return 60;
								}
								else { /* 01001010 */
									return 59;
								}
							}
							else {			/* 0100100 */
								return 27;
							}
						}
					}
					else {			/* 01000 */
						return 11;
					}
				}
			}
		}
		else {			/* 00 */
			if (getbit(input)) {	/* 001 */
				if (getbit(input)) {	/* 0011 */
					if (getbit(input)) {	/* 00111 */
						return 10;
					}
					else {			/* 00110 */
						if (getbit(input)) { /* 001101 */
							if (getbit(input)) { /* 0011011 */
								if (getbit(input)) {	/* 0110111 */
									return 384; /* make up */
								}
								else { /* 0110110 */
									return 320; /* make up */
								}
							}
							else { /* 0011010 */
								if (getbit(input)) { /* 00110101 */
									return 0;
								}
								else { /* 00110100 */
									return 63;
								}
							}
						}
						else {			/* 001100 */
							if (getbit(input)) { /* 0011001 */
								if (getbit(input)) { /* 00110011 */
									return 62;
								}
								else { /* 00110010 */
									return 61;
								}
							}
							else {			/* 0011000 */
								return 28;
							}
						}
					}
				}
				else {			/* 0010 */
					if (getbit(input)) { /* 00101 */
						if (getbit(input)) { /* 001011 */
							if (getbit(input)) { /* 0010111 */
								return 21;
							}
							else {			/* 0010110 */
								if (getbit(input)) { /* 00101101 */
									return 44;
								}
								else { /* 00101100 */
									return 43;
								}
							}
						}
						else {			/* 001010 */
							if (getbit(input)) { /* 0010101 */
								if (getbit(input)) { /* 00101011 */
									return 42;
								}
								else { /* 00101010 */
									return 41;
								}
							}
							else { /* 0010100 */
								if (getbit(input)) { /* 00101001 */
									return 40;
								}
								else { /* 00101000 */
									return 39;
								}
							}
						}
					}
					else {			/* 00100 */
						if (getbit(input)) { /* 001001 */
							if (getbit(input)) { /* 0010011 */
								return 26;
							}
							else {			/* 0010010 */
								if (getbit(input)) { /* 00100101 */
									return 54;
								}
								else { /* 00100100 */
									return 53;
								}
							}
						}
						else {			/* 001000 */
							return 12;
						}
					}
				}
			}
			else {		/* 000 */
				if (getbit(input)) {	/* 0001 */
					if (getbit(input)) {	/* 00011 */
						if (getbit(input)) { /* 000111 */
							return 1;
						}
						else {			/* 000110 */
							if (getbit(input)) { /* 0001101 */
								if (getbit(input)) { /* 00011011 */
									return 32;
								}
								else { /* 00011010 */
									return 31;
								}
							}
							else {		/* 0001100 */
								return 19;
							}
						}
					}
					else {		/* 00010 */
						if (getbit(input)) { /* 000101 */
							if (getbit(input)) { /* 0001011 */
								if (getbit(input)) { /* 00010111 */
									return 38;
								}
								else { /* 00010110 */
									return 37;
								}
							}
							else { /* 0001010 */
								if (getbit(input)) { /* 00010101 */
									return 36;
								}
								else { /* 00010100 */
									return 35;
								}
							}
						}
						else {			/* 000100 */
							if (getbit(input)) { /* 0001001 */
								if (getbit(input)) { /* 00010011 */
									return 34;
								}
								else { /* 00010010 */
									return 33;
								}
							}
							else {			/* 0001000 */
								return 20;
							}
						}
					}
				}
				else {		/* 0000 */
					if (getbit(input)) { /* 00001 */
						if (getbit(input)) { /* 000011 */
							return 13;
						}
						else {			/* 000010 */
							if (getbit(input)) { /* 0000101 */
								if (getbit(input)) { /* 00001011 */
									return 48;
								}
								else { /* 00001010 */
									return 47;
								}
							}
							else {			/* 0000100 */
								return 23;
							}
						}
					}
					else {		/* 00000 */
						if (getbit(input)) { /* 000001 */
							if (getbit(input)) { /* 0000011 */
								return 22;
							}
							else {			/* 0000010 */
								if (getbit(input)) { /* 00000101 */
									return 46;
								}
								else { /* 00000100 */
									return 45;
								}
							}
						}
						else {			/* 000000 */
							if (getbit(input)) { /* 0000001 */
								if (getbit(input)) { /* 00000011 */
									return 30;
								}
								else { /* 00000010 */
									return 29;
								}
							}
							else { /* 0000000 */
								return commonmake(input);	/* seven zeros */
							}
						}
					}
				}
			}
		}
	}
/*	winerror("Impossible white run"); */ /* unreachable */
/*	return -1 */	/* error */
}

/* Compiler screws up the following code if optimizations turned on */

#ifdef LONGNAMES
int blackzero (HFILE input) {		/* black run code starts with four zeros */
#else
int blackzero (FILE *input) {		/* black run code starts with four zeros */
#endif
#ifdef DEBUGHUFFMAN
	if (bDebug > 1) OutputDebugString (" blackzero entry "); 
#endif
	if (getbit(input)) { /* 00001 */
		if (getbit(input)) { /* 000011 */
			if (getbit(input)) { /* 0000111 */
				return 12;
			}
			else {			/* 0000110 */
				if (getbit(input)) {	/* 00001101 */
					if (getbit(input)) { /* 000011011 */
						if (getbit(input)) { /* 0000110111 */
							return 0;
						}
						else {			/* 0000110110 */
							if (getbit(input)) { /* 00001101101 */
								if (getbit(input)) { /* 000011011011 */
									return 43;
								}
								else {			/* 000011011010 */
									return 42;
								}
							}
							else {			/* 00001101100 */
								return 21;
							}
						}
					}
					else {			/* 000011010 */
						if (getbit(input)) { /* 0000110101 */
							if (getbit(input)) { /* 00001101011 */
								if (getbit(input)) { /* 000011010111 */
									return 39;
								}
								else {			/* 000011010110 */
									return 38;
								}
							}
							else {			/* 00001101010 */
								if (getbit(input)) { /* 000011010101 */
									return 37;
								}
								else {			/* 000011010100 */
									return 36;
								}
							}
						}
						else {			/* 0000110100 */
							if (getbit(input)) { /* 00001101001 */
								if (getbit(input)) { /* 000011010011 */
									return 35;
								}
								else { /* 000011010010 */
									return 34;
								}
							}
							else {			/* 00001101000 */
								return 20;
							}
						}
					}
				}
				else {			/* 00001100 */
					if (getbit(input)) {	/* 000011001 */
						if (getbit(input)) { /* 0000110011 */
							if (getbit(input)) { /* 00001100111 */
								return 19;
							}
							else {			/* 00001100110 */
								if (getbit(input)) { /* 000011001101 */
									return 29;
								}
								else {			/* 000011001100 */
									return 28;
								}
							}
						}
						else {			/* 0000110010 */
							if (getbit(input)) { /* 00001100101 */
								if (getbit(input)) { /* 000011001011 */
									return 27;
								}
								else {			/* 000011001010 */
									return 26;
								}
							}
							else {			/* 00001100100 */
								if (getbit(input)) { /* 000011001001 */
									return 192;	/* make up */
								}
								else {			/* 000011001000 */
									return 128;	/* make up */
								}
							}
						}
					}
					else {			/* 000011000 */
						return 15;
					}
				}
			}
		}
		else {			/* 000010 */
			if (getbit(input)) { /* 0000101 */
				return 11;
			}
			else {			/* 0000100 */
				return 10;
			}
		}
	}
	else {		/* 00000 */
		if (getbit(input)) { /* 000001 */
			if (getbit(input)) { /* 0000011 */
				if (getbit(input)) {	/* 00000111 */
					return 14;
				}
				else {			/* 00000110 */
					if (getbit(input)) { /* 000001101 */
						if (getbit(input)) { /* 0000011011 */
							if (getbit(input)) { /* 00000110111 */
								return 22;
							}
							else {			/* 00000110110 */
								if (getbit(input)) { /* 000001101101 */
									return 41;
								}
								else {			/* 000001101100 */
									return 40;
								}
							}
						}
						else {			/* 0000011010 */
							if (getbit(input)) {  /* 00000110101 */
								if (getbit(input)) { /* 000001101011 */
									return 33;
								}
								else {			/* 000001101010 */
									return 32;
								}
							}
							else {			/* 00000110100 */
								if (getbit(input)) { /* 000001101001 */
									return 31;
								}
								else {			/* 000001101000 */
									return 30;
								}
							}
						}
					}
					else {			/* 000001100 */
						if (getbit(input)) { /* 0000011001 */
							if (getbit(input)) { /* 00000110011 */
								if (getbit(input)) { /* 000001100111 */
									return 63;
								}
								else {			/* 000001100110 */
									return 62;
								}
							}
							else {			/* 00000110010 */
								if (getbit(input)) { /* 000001100101 */
									return 49;
								}
								else {			/* 000001100100 */
									return 48;
								}
							}
						}
						else {			/* 0000011000 */
							return 17;
						}
					}
				}
			}
			else {			/* 0000010 */
				if (getbit(input)) {	/* 00000101 */
					if (getbit(input)) { /* 000001011 */
						if (getbit(input)) { /* 0000010111 */
							return 16;
						}
						else {			/* 0000010110 */
							if (getbit(input)) { /* 00000101101 */
								if (getbit(input)) { /* 000001011011 */
									return 256;		/* make up */
								}
								else {			/* 000001011010 */
									return 61;
								}
							}
							else {			/* 00000101100 */
								if (getbit(input)) { /* 000001011001 */
									return 58;
								}
								else {			/* 000001011000 */
									return 57;
								}
							}
						}
					}
					else {			/* 000001010 */
						if (getbit(input)) { /* 0000010101 */
							if (getbit(input)) { /* 00000101011 */
								if (getbit(input)) { /* 000001010111 */
									return 47;
								}
								else {			/* 000001010110 */
									return 46;
								}
							}
							else {			/* 00000101010 */
								if (getbit(input)) { /* 000001010101 */
									return 45;
								}
								else {			/* 000001010100 */
									return 44;
								}
							}
						}
						else {			/* 0000010100 */
							if (getbit(input)) { /* 00000101001 */
								if (getbit(input)) { /* 000001010011 */
									return 51;
								}
								else {			/* 000001010010 */
									return 50;
								}
							}
							else {			/* 00000101000 */
								return 23;
							}
						}
					}
				}
				else {			/* 00000100 */
					return 13;
				}
			}
		}
		else {			/* 000000 */
			if (getbit(input)) { /* 0000001 */
				if (getbit(input)) { /* 00000011 */
					if (getbit(input)) { /* 000000111 */
						if (getbit(input)) { /* 0000001111 */
							return 64;	/* make up */
						}
						else {			/* 0000001110 */
							if (getbit(input)) { /* 00000011101 */
								if (getbit(input)) { /* 000000111011 */
									if (getbit(input)) { /* 0000001110111 */
										return 1216;
									}
									else {			/* 0000001110110 */
										return 1152;
									}
								}
								else {			/* 000000111010 */
									if (getbit(input)) { /* 0000001110101 */
										return 1088;
									}
									else {			/* 0000001110100 */
										return 1024;
									}

								}
							}
							else {			/* 00000011100 */
								if (getbit(input)) { /* 000000111001 */
									if (getbit(input)) { /* 0000001110011 */
										return 960;
									}
									else {			/* 0000001110010 */
										return 896;
									}
								}
								else {			/* 000000111000 */
									return 54;
								}
							}
						}
					}
					else {			/* 000000110 */
						if (getbit(input)) { /* 0000001101 */
							if (getbit(input)) { /* 00000011011 */
								if (getbit(input)) { /* 000000110111 */
									return 53;
								}
								else {			/* 000000110110 */
									if (getbit(input)) { /* 0000001101101 */
										return 576;	/*make up */
									}
									else {			/* 0000001101100 */
										return 512;	/* make up */
									}
								}
							}
							else {			/* 00000011010 */
								if (getbit(input)) { /* 000000110101 */
									return 448;	/* make up */
								}
								else {			/* 000000110100 */
									return 384;	/* make up */
								}
							}
						}
						else {			/* 0000001100 */
							if (getbit(input)) { /* 00000011001 */
								if (getbit(input)) { /* 000000110011 */
									return 320;	/* make up */
								}
								else {			/* 000000110010 */
									if (getbit(input)) { /* 0000001100101 */
										return 1728;
									}
									else {			/* 0000001100100 */
										return 1664;
									}
								}
							}
							else {			/* 00000011000 */
								return 25;
							}
						}
					}
				}
				else { /* 00000010 */
					if (getbit(input)) { /* 000000101 */
						if (getbit(input)) { /* 0000001011 */
							if (getbit(input)) { /* 00000010111 */
								return 24;
							}
							else {			/* 00000010110 */
								if (getbit(input)) { /* 000000101101 */
									if (getbit(input)) { /* 0000001011011 */
										return 1600;
									}
									else {			/* 0000001011010 */
										return 1536;
									}
								}
								else {			/* 000000101100 */
									return 60;
								}
							}
						}
						else {			/* 0000001010 */
							if (getbit(input)) { /* 00000010101 */
								if (getbit(input)) { /* 000000101011 */
									return 59;
								}
								else {			/* 000000101010 */
									if (getbit(input)) { /* 0000001010101 */
										return 1472;
									}
									else {			/* 0000001010100 */
										return 1408;
									}
								}
							}
							else {			/* 00000010100 */
								if (getbit(input)) { /* 000000101001 */
									if (getbit(input)) { /* 0000001010011 */
										return 1344;
									}
									else {			/* 0000001010010 */
										return 1280;
									}
								}
								else {			/* 000000101000 */
									return 56;
								}
							}
						}
					}
					else {			/* 000000100 */
						if (getbit(input)) { /* 0000001001 */
							if (getbit(input)) { /* 00000010011 */
								if (getbit(input)) { /* 000000100111 */
									return 55;
								}
								else {			/* 000000100110 */
									if (getbit(input)) { /* 0000001001101 */
										return 832;
									}
									else {			/* 0000001001100 */
										return 768;
									}
								}
							}
							else {			/* 00000010010 */
								if (getbit(input)) { /* 000000100101 */
									if (getbit(input)) { /* 0000001001011 */
										return 704;
									}
									else {			/* 0000001001010 */
										return 640;
									}
								}
								else {			/* 000000100100 */
									return 52;
								}
							}
						}
						else {			/* 0000001000 */
							return 18;
						}
					}
				}
			}
			else { /* 0000000 */
				return commonmake(input);	/* seven zeros */
			}
		}
	}
/*	winerror("Impossible black run (starting with four zeros)"); */
/*	return -1; */	/* error */
}

/* blackrun may actually be OK with compiler optimizations turned on */

#ifdef LONGNAMES
int blackrun (HFILE input) {
#else
int blackrun (FILE *input) {
#endif
#ifdef DEBUGHUFFMAN
	if (bDebug) OutputDebugString ("blackrun entry "); 
#endif
	if (getbit(input)) {	/* 1 */
		if (getbit(input)) {	/* 11 */
			return 2;
		}
		else {			/* 10 */
			return 3;
		}
	}
	else {			/* 0 */
		if (getbit(input)) {	/* 01 */
			if (getbit(input)) {	/* 011 */
				return 4;
			}
			else {			/* 010 */
				return 1;
			}
		}
		else {			/* 00 */
			if (getbit(input)) {	/* 001 */
				if (getbit(input)) {	/* 0011 */
					return 5;
				}
				else {			/* 0010 */
					return 6;
				}
			}
			else {		/* 000 */
				if (getbit(input)) {	/* 0001 */
					if (getbit(input)) {	/* 00011 */
						return 7;
					}
					else {		/* 00010 */
						if (getbit(input)) { /* 000101 */
							return 8;
						}
						else {			/* 000100 */
							return 9;
						}
					}
				}
				else {		/* 0000 */
					return blackzero(input);	/* four zeros */
				}
			}
		}
	}
/*	winerror("Impossible black run"); */ /* unreachable code */
/*	return -1;	*/ /* error */
}

/* #pragma optimize ("lge", on)	 */	/* or just reset ? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int index, bitinx;	/* index into row of bytes and bits within them */

int total;			/* maybe long ? */

/* Following could be speeded up some ... */

int writewhite (unsigned char *lpBuffer, int run, int width) {
	if (run == 0) return 0;			/* nothing to do */
	if (run < 0) return -1;			/* hit invalid run */
	total += run;
#ifdef DEBUGHUFFMAN
	if (bDebug > 1) {
		sprintf(debugstr, "run %d total %d\n", run, total);
		OutputDebugString(debugstr);
	}
#endif
	if (total > width) return -1;	/* error */
/*	just advance pointers */
	while (run > 0) {
		if (bitinx == 0) {
			while (run >= 8) {	
				index++;
#ifdef DEBUGHUFFMAN
				if (bDebug > 1) {
					sprintf(debugstr, "index %d run %d ", index, run);
					OutputDebugString(debugstr);
				}
#endif
/*				*(lpBuffer+index) = 0; */	/* already zeroed out */
				run -= 8;
			}
		}
		if (run > 0) {
			if (bitinx-- <=  0) {
				index++; bitinx = 7;
			}
#ifdef DEBUGHUFFMAN
			if (bDebug > 1) {
				sprintf(debugstr, "index %d bitinx %d ", index, bitinx); 
				OutputDebugString(debugstr);
			}
#endif
/*			*(lpBuffer + index) &= ~(1 << bitinx); */
			run--;
		}
	}
	if (total == width) return 1;	/* EOL */
	else return 0;
}	/* lpBuffer unreferenced */

int writeblack (unsigned char *lpBuffer, int run, int width) {
	if (run == 0) return 0;			/* nothing to do */
	if (run < 0) return -1;			/* hit invalid run */
	total += run;
#ifdef DEBUGHUFFMAN
	if (bDebug > 1) {
		sprintf(debugstr, "run %d total %d\n", run, total);
		OutputDebugString(debugstr);
	}
#endif
	if (total > width) return -1;	/* error */
	while (run > 0) {
		if (bitinx == 0) {
			while (run >= 8) {	
				index++;
#ifdef DEBUGHUFFMAN
				if (bDebug > 1) {
					sprintf(debugstr, "index %d run %d ", index, run);
					OutputDebugString(debugstr);
				}
#endif
				*(lpBuffer+index) = 255;	/* write a byte at a time */
				run -= 8;
			}
		}
		if (run > 0) {
			if (bitinx-- <=  0) {
				index++; bitinx = 7;
			}
#ifdef DEBUGHUFFMAN
			if (bDebug > 1) {
				sprintf(debugstr, "index %d bitinx %d ", index, bitinx); 
				OutputDebugString(debugstr);
			}
#endif
			*(lpBuffer + index) |= (1 << bitinx);	/* write a bit at a time */
			run--;
		}
	}
	if (total == width) return 1;	/* EOL */
	else return 0;
}

/* make width long ? */

#ifdef LONGNAMES
int huffmanrow (unsigned char *lpBuffer, HFILE input, int width) {	
#else
int huffmanrow (unsigned char *lpBuffer, FILE *input, int width) {	
#endif
	int bytes;
	int run;
/*	int k; */
/*	int total = 0; */							/* long total ? */

/*	if (lpBuffer == NULL) {
		winerror(" Bad buffer pointer\n");
		return -1;
	} */
	total = 0;
	index = -1; bitinx = 0;			/* output buffering */
	nbyte = 0; bitsleft = 0;		/* input buffering */

	bytes = (width + 7) / 8;		/* preset with zeros */
#ifdef DEBUGHUFFMAN
	if (bDebug > 1) {
		sprintf (debugstr, "Cleaning out %d bytes\n", bytes);
		OutputDebugString(debugstr);
	}
#endif
	if (bytes > BufferLength) {
#ifdef DEBUGHUFFMAN
		if (bDebug > 1) OutputDebugString("OVERRUN ! ! !\n");
#endif
	}
#ifdef USEMEMCPY
	(void) memset (lpBuffer, 0, bytes);
#else
	for (k = 0; k < bytes; k++) lpBuffer[k] = 0;
#endif

	for (;;) {
#ifdef DEBUGHUFFMAN
		if (bDebug > 1) OutputDebugString("Looking for white run\n"); 
#endif
		while ((run = whiterun (input)) >= 64) {
#ifdef DEBUGHUFFMAN
			if (bDebug > 1) {
				sprintf(debugstr, " W %d ", run);
				OutputDebugString(debugstr);
			}
#endif
			if (writewhite(lpBuffer, run, width) < 0) break;
		}
		if (total >= width) break;
#ifdef DEBUGHUFFMAN
		if (bDebug > 1) {
			sprintf(debugstr, " W %d\n", run);
			OutputDebugString(debugstr);
		}
#endif
		if (writewhite(lpBuffer, run, width) != 0) break;	 /* terminal run */

#ifdef DEBUGHUFFMAN
		if (bDebug > 1) OutputDebugString("Looking for black run\n"); 
#endif
		while ((run = blackrun (input)) >= 64) {
#ifdef DEBUGHUFFMAN
			if (bDebug > 1) {
				sprintf(debugstr, " B %d ", run);
				OutputDebugString(debugstr);
			}
#endif
			if (writeblack(lpBuffer, run, width) < 0) break;
		}
		if (total >= width) break;
#ifdef DEBUGHUFFMAN
		if (bDebug > 1) {
			sprintf(debugstr, " B %d\n", run);
			OutputDebugString(debugstr);
		}
#endif
		if (writeblack(lpBuffer, run, width) != 0) break;	 /* terminal run */
	}
	if (total != width) {
		sprintf(debugstr, "Sum of runs %d not equal width %d", total, width);
		winerror(debugstr);
		return -1;
	}
#ifdef DEBUGHUFFMAN
	else if (bDebug > 1) {
		sprintf(debugstr, "Sum of runs equal to width %d\n", width);
		OutputDebugString(debugstr);
	}
#endif
	return 0;
}

/**************************************************************************/

/* Read a row using PACK_BITS compression scheme */
/* returns non-zero if problems encountered */

/* int readpackbits (unsigned char *lpBuffer, FILE *input, int RowLength) */
int readpackbits (unsigned char *lpBuffer, HFILE input, int RowLength, int i) {
/*	unsigned char *u=lpBuffer; */
	unsigned char *u;
	int c, k, n, total, flag=0;
	int ndif;
	
	u = lpBuffer;
	total = 0;
	for (;;) {
		n = wingetc(input);
		if (n < 0) {	/* premature EOF */
			winerror("Premature EOF");
			flag = -1;
			break;	
		}
		else if (n < 128) {			/* use next (n+1) bytes as is */
			if (total + (n+1) <= RowLength) 	/* safety valve */
				for (k=0; k < n+1; k++) *u++ = (char) wingetc(input);
			else {					/* should never happen */
				ndif = RowLength - total;
				for (k = 0; k < ndif; k++) *u++ = (char) wingetc(input);
				for (k = ndif; k < n+1; k++) (void) wingetc(input);
			}
			total += n+1;
		}
		else if (n > 128) {			/* repeat next byte (257 - n) times */
			c = wingetc(input);
			if (total + (257-n) <= RowLength) {	/* safety valve */
/*				for (k=0; k < (257 - n); k++) *u++ = (char) c; */
				(void) memset(u, c, (257 - n));
				u += (257 - n);
			}
			else {						/* should never happen */
				ndif = (RowLength - total);
				(void) memset(u, c, ndif);
				u += ndif;
			}
			total += (257 - n);
		}
/*		else if ( n == 128) ; */ /* 128 is a NOP */
		if (total == RowLength) break;	/* EOL enough bytes yet ? */
		if (total > RowLength) {		/* too many bytes ? */
			sprintf(debugstr,
				"Too many bytes in compressed row %d (%d > %d) (%d)",
					i, total, RowLength, n);
			winerror(debugstr);
			flag = -1; 
			break;
		}
	}
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*	returns 0 on success */
/*	returns -1 if some sort of error */

int ReadTIFF (HFILE input, int nImage, unsigned char *lpBuffer,
				  LONG LineFunParam, FARPROC CopyLineFun) {
/*	long dwidth, long dheight, int nifd, int readflag) */
	int i, flag, ret;
	unsigned int nread;
	unsigned long ImageSize;
/*	unsigned char *lpBuffer=NULL; */

/*	Let's assume TIFF file has been verified, tags read */
/*	positioned at start of TIFF part of file ? */

#ifdef IGNORED
	TIFFVersion = ureadtwo(input);
	if (TIFFVersion != TIFF_VERSION) {
		winerror("Incorrect TIFF version code");
		return -1;		/* bad version number for TIFF file */
	}
	
/*	Skip to desired image (if not first in file) */

	IFDPosition = ureadfour(input);		/* get first IFD offset in file */
	while (nifd-- > 1) {
		if (skipthisimage(input, IFDPosition) < 0) {
			sprintf(debugstr, "ERROR: Subimage %d not found", nifd); 
			winerror(debugstr);
			return -1;
		}
	}

/*	Now at desired image */
	(void) readfields(input, IFDPosition);	/* read tag fields in TIFF file */

	IFDPosition = ureadfour(input);		/* get next IFD offset in file */

	if (readflag != 0) return -1;	/*  only scanning for width & height */

	bColorImage = 0;			/* comes on if image is RGB or Palette color */
	bExpandColor = 0;			/* comes on if Palette color image */
	bCompressColor = 0;			/* if image is colored and not `colorimage' */
	if (ColorMapPtr) {			/* is it a palette color image ? */
		(void) ReadColorMap(input, ColorMapPtr, BitsPerSample);
		bExpandColor = 1;
		bColorImage = 1;
	}
#endif

#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr,
		"ImageWidth %ld ImageLength %ld BitsPerSample %d SamplesPerPixel %d\n", 
				ImageWidth, ImageLength, BitsPerSample, SamplesPerPixel);
		OutputDebugString(debugstr);
	}
#endif

#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr, "Compression %u PhotometricInterpretation %d\n",
				compression, PhotometricInterpretation);
		OutputDebugString(debugstr);
	}
#endif

	if (Predictor != 1) {
#ifdef DEBUGTIFF
		if (bDebug > 1) {
			sprintf(debugstr, "Predictor %d\n", Predictor);
			OutputDebugString(debugstr);
		}
#endif
	}

	BitsPerPixel = BitsPerSample * SamplesPerPixel;
	if (ExtraSamples > 0) BitsPerPixelX = BitsPerSample * (SamplesPerPixel-ExtraSamples);
	else BitsPerPixelX = BitsPerPixel;
	
	InRowLength = (ImageWidth * BitsPerPixel + 7) / 8;	/* row length file */
	if (ExtraSamples > 0)InRowLengthX = (ImageWidth * BitsPerPixelX + 7) / 8;
	else InRowLengthX = InRowLength;

	if (RowsPerStrip > 0)
		StripsPerImage = (int) ((ImageLength + RowsPerStrip - 1) / RowsPerStrip);
	else StripsPerImage = 1;

	BufferLength = InRowLength;

/*	if (BitsPerPixel == 24 && bCompressColor) */
	if (BitsPerPixelX == 24 && bCompressColor) {
/*		bCompressColor = 1; */
		OutRowLength = BufferLength / 3;
	}
	else OutRowLength = BufferLength;
		
/*	OutRowLength not used here but in winspeci.c */

#ifdef DEBUGTIFF
	if (bDebug > 1) {
		sprintf(debugstr,
	"InRowLength %ld BufferLength %ld OutRowLength %ld RowsPerStrip %ld\n",
			InRowLength, BufferLength, OutRowLength, RowsPerStrip);
		OutputDebugString(debugstr);
	}
#endif

/*	deal with GhostScript TIFF file format with gaps */		/* 94/Dec/16 */

/*	shouldn't this only kick in if we are not using (LZW) compression ? */
	if (bGhostHackFlag) {				/*	made conditional 95/Nov/10 */
		if (InRowLength + 1 == StripByteCount) 	InRowLength++;
	}

/*	should also increase BufferLength allocation ? 95/Nov/10 */

#ifdef IGNORED
	if (computeheader () != 0) return -1;	/* noticed format not supported? */
	writepsheader(output, dwidth, dheight);
#endif

/*	following should already be taken care of ... */
	if (compression > TIFF_CCITT && compression != LZW_COMPRESSION &&
		compression != PACK_BITS) { 
		sprintf(debugstr,	"ERROR: Unknown compression scheme (%d) in %s",
				compression, FileName);
/*		winerror(buffer); */
		winbadimage(debugstr);
		return -1; 
	}
	
/*	ImageSize = (unsigned long) InRowLength * ImageLength; */
	ImageSize = (unsigned long) InRowLengthX * ImageLength;
/*	check whether values reasonable */	
	if (ImageSize == 0) {
/*		winerror("ERROR: Zero image size"); */
		sprintf(debugstr, "ERROR: Zero image size %ld x %ld (%s)",
				InRowLengthX, ImageLength, "readTIFF");
		winbadimage(debugstr);
		return -1;

	}
	if (ImageWidth > MAXIMAGEDIMEN || ImageLength > MAXIMAGEDIMEN ||	/* bad data ? */
		ImageSize > MAXIMAGESIZE) {	/* arbitrary limits (to catch bad files) */
		sprintf(debugstr,
			"ERROR: TIFF file too large\n(%ld x %ld (%d) => %ld bytes)", 
				ImageWidth, ImageLength, BitsPerPixel, ImageSize);
/*		winerror(buffer); */
		winbadimage(debugstr);
		return -1;
	}

	if (ImageWidth < 0 || ImageLength < 0 ||		/* missing fields */
				StripOffset < 0) {					/* missing fields */
/*		winerror("ERROR: TIFF file missing required tags"); */
		winbadimage("ERROR: TIFF file missing required tags");
		return -1;
	}		

	if (winseek(input, (long) StripOffset + tiffoffset) == 0)  {
		winerror("Error in seek to StripOffset");
		return -1;
	}

/*  Actually go and read the file now ! */

/*	following should already be taken care of in `computeheader' */

	if (compression > TIFF_CCITT && compression != LZW_COMPRESSION &&
		compression != PACK_BITS) {  
		sprintf(debugstr, "ERROR: Unknown compression scheme (%d) in %s",
				compression, FileName);
/*		winerror(buffer); */
		winbadimage(debugstr);
		return -1;
	}

#ifdef DEBUGTIFF
	if (bDebug > 1) {
		if (compression == PACK_BITS)
			OutputDebugString("Using PACK_BITS\n");
		else if (compression == TIFF_CCITT)
			OutputDebugString("Using TIFF_CCITT\n");
		else if (compression == LZW_COMPRESSION)
			OutputDebugString("Using LZW\n");
	} 
#endif

	flag = 0;								/* flag gets set if EOF hit */

	if (compression != 0) {					/* set up buffering for wingetc */
		if (wininit (input) < 0) return -1;
	}

	if (compression == LZW_COMPRESSION) {
/*		LZW needs to be done by strips */
		DecodeLZW(input, (unsigned char *) lpBuffer, LineFunParam, CopyLineFun);
	}
	else {		/* else not LZW compression */
/*		non LZW can be done by row */
		for (i = 0; i < (int) ImageLength; i++) {	/* read image lines */
			if (compression == PACK_BITS) {		/* compressed binary file */
				if (readpackbits(lpBuffer, input, (int) InRowLength, i) != 0) {
					flag = -1;
					break;
				}
/*				remove ExtraSamples if any ? */				
			}
			else if (compression == TIFF_CCITT) {	/* CCITT 1D compression */
				if (huffmanrow((unsigned char *) lpBuffer, input, (int) ImageWidth)
					!= 0) {
					flag = -1;
					break;
				}
/*				remove ExtraSamples if any ? */
			}
			else {								/* uncompressed file */
#ifdef LONGNAMES
				nread = _lread(input, lpBuffer, (unsigned int) InRowLength);
#else
				nread = fread(lpBuffer, 1, (unsigned int) InRowLength, input);
#endif
/*				if nread < 0 then error, if nread == 0 then EOF */
				if (nread != (unsigned int) InRowLength) {	/* read a row */
					flag = -1;
					break;
				} 
				if (ExtraSamples > 0) 	/* throw out ExtraSamples in SamplesPerPixel */
					(void) RemoveExtraSamples(lpBuffer, InRowLength); /* 99/May/10 ? */
			}
/*			we don't expand or compress color at this level */
/*			if (ProcessRow (output, lpBuffer, InRowLength, BufferLength,
				OutRowLength) != 0) break; */
			ret = CopyLineFun(lpBuffer, i, LineFunParam);
			if (ret != 0) break;			/* if something went wrong ... */
/*			check on abort flag here somewhere also ? */
		}	/* end of this row */
	} /* end of not LZW compression */
	
	if (compression != 0) winendit ();				/* flush buffering */

/* now finished reading */	/* maybe use memory for lpBuffer ? */

#ifdef IGNORED
	if (lpBuffer != NULL) {
/*		free(lpBuffer); */ /* lpBuffer = NULL; */
		lpBuffer = (char *) LocalFree((HLOCAL) lpBuffer);
	}
	if (hPalette != NULL) {
		GlobalUnlock(hPalette);
		hPalette = GlobalFree(hPalette);
	}
	writepstrailer(output);
#endif
	return flag;
} /* end of ReadTIFF(...) --- presently nImage unreferenced ??? 98/Mar/23 */

/**************************************************************************/

/* Linkage to winspeci.c */

/* Assumes global access to flip, inrowlength, lpImageBytes, bytesperrow */

/* Call format modelled somewhat on DecodeTiffImage in Black Ice */
/* int DecompressTiff(HFILE, int, WORD, WORD, LONG, FARPROC); */

/* This call CopyLineFun with data for each line *//* parameters passed are: */
/* pointer to line buffer, line number (zero based), parameter from caller */
/* int CALLBACK _export CopyLineFun(LPSTR lpLineBuff, int nLine, LONG lParam); */

/* returns 0 on failure - 1 on success */

int DecompressTiff (HFILE input, int nImage, WORD wFirstStrip,
				   WORD wNumLines, LONG LineFunParam, FARPROC CopyLineFun) {
	HGLOBAL hLine;
	unsigned char *lpLine=NULL;
	int ret;

/*	Create line buffer */	/* need to use LocalAlloc in WIN16 version ? */
	hLine = GlobalAlloc(GMEM_MOVEABLE, InRowLength);
	if (hLine == NULL) {
		winerror("Unable to allocate memory");
		PostQuitMessage(0);
		return(0);
	}
	lpLine = (unsigned char *) GlobalLock(hLine);

	BufferLength = InRowLength;
	ret = ReadTIFF(input, nImage, lpLine, LineFunParam, CopyLineFun);

	GlobalUnlock(hLine);
	hLine = GlobalFree(hLine);
	if (ret < 0) return 0;		/* failure */
	else return 1;				/* success */
}	/* wNumLines wFirstStrip unreferenced */

/**************************************************************************/

/* much of wintiff.c has moved to winspeci.c leaving mostly decompression here */

/**************************************************************************/
