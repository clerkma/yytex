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

#include "atm.h"
#include "atmpriv.h"

/* stdlib.h not really needed - just for hdrstop */

#include <commdlg.h>				/* for PrintDlg */
#define PRINTDLGORD      1538		/* from dlgs.h */

#pragma warning(disable:4100)	// unreferenced formal parameters

#define DVIPSONEDLL

/* #define DEBUGCOPY */

#define DEBUGPRINTING

/* #define DEBUGDVIPSONE */

// extern BOOL bUseDVIPSONEDLL;		/* DVIPSONE.DLL 99/June/13 */
// extern BOOL bCallBackPass;		/* DVIPSONE.DLL 99/June/13 */
BOOL bCallBack;		/* local value of bCallBackPass, may be modified if print to file */

BOOL bUsingDistiller=0;			// 99/Dec/30

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Copyright (C) 1991, 1992 Y&Y. All rights reserved. */
/* DO NOT COPY OR DISTRIBUTE! */

/****************************************************************************

	PROGRAM: dviprint.c

	PURPOSE: Take some infrequently used code out of dviwindo.c

	Mostly for printing  and for DVI Info display

****************************************************************************/

/* MAXPRINTERNAME 48 */	/* 32 "Apple LaserWriter II NT" say */
						/* same as CCHDEVICENAME in print.h which is 32 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following may not work in WIN32 ??? */

/* 95/Mar/12 Launching Other Windows-Based Applications */
/* Technical Articles: Kernel and Drivers for Windows (16-bit) */

/* local structure for enum windows call (from exec.c) */

typedef struct _ENUMINFO {
    HINSTANCE hInstance;        /* supplied on input */
    HWND hWnd;                  /* filled in by enum func */
} ENUMINFO, FAR *LPENUMINFO;

/* Define a structure used to hold info about an app we */
/* start with ExecApp so that this info can be used again */
/* later to test if the app is still running (from appexe.c) */

// typedef struct _EXECAPPINFO {
//     HINSTANCE hInstance;            /* The instance value */
//     HWND hWnd;                      /* The main window handle */
//     HTASK hTask;                    /* The task handle */
// } EXECAPPINFO, FAR *LPEXECAPPINFO;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* BOOL DVIPSONEexists=0; */	/* non-zero if PIF file (or EXE file) found */
BOOL bDVIPSONEexists=0;			/* non-zero if EXE found (on path, */
								/* or c:\dvipsone or d:\dvipsone) */

// BOOL bApplicationFlag=0;			/* non-zero if call for application file */

BOOL bPrintSuccess=0;				/* False if NEWFRAME Escape failed */

int oldaborterr = 0;		/* avoid repeat OutputDebugString in AbortProc */

int xprintoffset = -288;		/* was -630 */
int yprintoffset = 158;			/* was 430 */

int xposition=10, yposition;	/* where to place text on screen */

static char *modname = "WINPRINT";

int aCopies;					/* temporary */

/* magic halftone string is now string resource in RC */

/* char *magichalftone="currentscreen pop\n\
{1 add 180 mul cos 1 0.08 add mul exch 2 add 180 mul cos 1 0.08 sub\n\
mul add 2 div} bind setscreen % (c) Y&Y 1989\n"; */

/*  Keep it here can keep it around if desired */
/*	Just remember to reload it if user selects different printer ? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Most of this depends only on COMMDLG */

// extern PRINTDLG pd;				/* place for COMMDLG structure 95/Dec/14 */

/* remember to set pd.hDevMode and pd.hDevNames to NULL before */
/* remember to free pd.hDevMode and pd.hDevNames after */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*-------------------------------------------------------------*\
 |                        Static Data.                         |
\*-------------------------------------------------------------*/

/* review need for this in WIN32 and allocations */
/* copied from DEVNAME structure - do we need to do that ? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* put up error message in box */

static void winerror(char *mss) {
	HWND hFocus;
	if((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox(hFocus, mss, modname, MB_ICONSTOP | MB_OK);
}

static int wincancel(char *buff) {				/* 1993/July/15 */
	HWND hFocus;
	int flag;

	if((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, buff, modname, MB_ICONSTOP | MB_OKCANCEL);
	if (flag == IDCANCEL) return -1;
	else return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Mostly printer selection: */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*-------------------------------------------------------------*\
 |                    Function Prototypes.                     |
\*-------------------------------------------------------------*/

/* in Windows NT PASCAL => WINAPI */

/* Process WM_INITDIALOG for dialog box - gets all printers available */
/* plus highlight the one we had before, or system default */
BOOL NEAR PASCAL PrSetupInit (HWND);
/* Parse string in list box => printer, driver and port */
int  NEAR PASCAL PrsetupParseLB (HWND, PSTR, PSTR, PSTR);
/* Call driver setup function for detailed setup of specific driver */
int  NEAR PASCAL PrSetupCallDriver (HWND);
/* Get default printer */
/* Returns handle to memory block containing DEVMODE structure */
/* HANDLE NEAR PASCAL PrSetupGetDefPrinter (PSTR, PSTR, PSTR, int); */
/* HLOCAL NEAR PASCAL PrSetupGetDefPrinter (PSTR, PSTR, PSTR, int); */
HGLOBAL NEAR PASCAL PrSetupGetDefPrinter (PSTR, PSTR, PSTR, int);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*	check for PSCRIPT.DRV or use GETTECHNOLOGY Escape => `PostScript' */

/*	To identify the printer as a PostScript printer, use this code: 95/Dec/20 */

int IsItPSCRPT (HDC printDC) {
	int gPrCode;				/* Set according to platform. */

	if (bDebug > 1) OutputDebugString("IsItPSCRPT?");

//	WIN32 test, checks POSTSCRIPT_PASSTHROUGH escape supported */
//	strangely in NT at least this is the only reliable method ...
	gPrCode = POSTSCRIPT_PASSTHROUGH;
	if (Escape(printDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &gPrCode, NULL)) {
		if (bDebug > 1) OutputDebugString("POSTSCRIPT_PASSTHROUGH supported");
		return TRUE;
	}

#ifdef IGNORED
//	not clear the following is reasonable in WIN32 
/*	Thorough WIN16 test, checks TECHNOLOGY = PostScript and PASSTHROUGH */
	gPrCode = GETTECHNOLOGY;
	if (Escape(printDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &gPrCode, NULL)) {
		gPrCode = PASSTHROUGH;		/* PASSTHROUGH / DEVICEDATA 19 */ 
		if (ExtEscape(printDC, GETTECHNOLOGY, 0, NULL, sizeof(str), (LPSTR) str) > 0) {
			if (bDebug > 1) {
				sprintf(debugstr, "GETTECHNOLOGY %s", str);
				OutputDebugString(debugstr);
			}
			if (_strnicmp(str, "PostScript", 10) == 0) 
				return TRUE;		/* The printer is PostScript. */
		}
	}
//	if it doesn't support GETTECHNOLOGY try EPSPRINTING escape
	else {
		gPrCode = EPSPRINTING;
		if (Escape(printDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &gPrCode, NULL)) {
			if (bDebug > 1) OutputDebugString("EPSPRINTING");
			return TRUE;		/* support EPSPRINTING */
		}
	}
#endif

   return FALSE;
}

HANDLE hPrinter=NULL;				// used by Open- / Close- Printer 

int gPrCode=POSTSCRIPT_PASSTHROUGH;		// Set according to printer support. 

// passthrough used for magic screen function and EPS figure insertion
// and now by dvipsone.dll ...

// int passthrough (HDC hDC, const char *ps, int nbyte) 
int passthrough (HDC hDC, char *ps, unsigned int nbyte) {
	char buffer[BUFLEN+sizeof(short)];	/* safe to use place on stack ??? */
	int ret;
	short snbyte = (short) nbyte;

#ifdef IGNORED
	if (bDebug > 1) {
		sprintf(debugstr, "Passthrough %d bytes %s\n", nbyte,
				(nbyte < 80) ? ps : "");
		OutputDebugString(debugstr);
	}
#endif
	if (nbyte == 0) {		// used to indicate end of PS output
//		int k;
//		return 0;			// prevent PASSTHROUGH failure 
//		passthrough(hDC, "%%EOF\n", 6);
//		passthrough(hDC, "\004", 1);
//		for (k=0; k < 64; k++) passthrough(hDC, blanks, strlen(blanks));
		gPrCode = PASSTHROUGH;	// ???
//		return 0;
//		very last passthrough needs to use PASSTHROUGH not POSTSCRIPT_PASSTHROUGH ???
//		ps = "\n";			// experiment only
		ps = " ";			// experiment only
		ps = "\n% Final PASSTHROUGH\n";	// debugging
		nbyte = strlen(ps);
//		somehow this fails after using POSTSCRIPT_PASSTHROUGH to Distiller
//		but seems required to force closing of PS output stream...
//		doesn't matter what the string is in this case
//		this does not fail when printing to HP driver		
	}

	if (bOpenClosePrinter) {
		DWORD nwritten;							// bytes actually written
		if (! WritePrinter(hPrinter, ps, nbyte, &nwritten)) {
			ShowLastError();
			return -1;
		}
		if (nwritten != nbyte) {
//			some sort of error I suppose...
		}
		return 0;
	}

/*	copy to local buffer so can add length word at start ... */
	if (nbyte > BUFLEN) {
		sprintf(debugstr, "PassThrough string too long %d > %d", nbyte, BUFLEN);
		ret = wincancel(debugstr);
		if (ret) bUserAbort = 1;		// ???
		return ret;
	}

#ifdef IGNORED
//	do we need this nonsense also in Windows 98 ???
//	if (WinVerNum == 0x0400) 			// Windows 95 
	if (WinVerNum <= 0x0410) {			// Windows 95/98
//		chop it up for Windows 95/98 ???
		while (nbyte > 0) {
			int nlen;						// 99/Nov/28
			short snlen;

			if (nbyte < 256) nlen = nbyte;
			else nlen = 255;
			snlen = (short) nlen;
		
//		memcpy(buffer+sizeof(short), ps, nbyte);		/* 95/Dec/14 */
			memcpy(buffer+sizeof(short), ps, nlen);

//		*((short *) buffer) = snbyte;	/* store length in buffer 95/Dec/14 */
			*((short *) buffer) = snlen;	/* store length in buffer 95/Dec/14 */
//  This is *one* Escape that *does* work in WIN32... 


			if (gPrCode == POSTSCRIPT_PASSTHROUGH) {
//			ret = ExtEscape(hDC, gPrCode, (short) (nbyte+sizeof(short)), 
//							(LPSTR) &buffer, 0, NULL);
				ret = ExtEscape(hDC, gPrCode, (short) (nlen+sizeof(short)), 
								(LPSTR) &buffer, 0, NULL);
				if (ret < 0) {
//				sprintf(debugstr, "%s Escape failed (ret %d nbyte %d `%s')",
//				"PASSTHROUGH", ret, nbyte, ps);
					sprintf(debugstr, "%s Escape failed (ret %d nle %d `%s')",
							"POSTSCRIPT_PASSTHROUGH", ret, nlen, ps);
					ret = wincancel(debugstr);
					if (ret) bUserAbort = 1;		// ???
					ShowLastError();
//				return ret;
					gPrCode = PASSTHROUGH;		// fall back
				}
			}

			if (gPrCode == PASSTHROUGH) {
//			ret = Escape(hDC, gPrCode, (short) (nbyte+sizeof(short)), 
//						 (LPSTR) &buffer, NULL);
				ret = Escape(hDC, gPrCode, (short) (nlen+sizeof(short)), 
							 (LPSTR) &buffer, NULL);
				if (ret < 0) {
//				sprintf(debugstr, "%s Escape failed (ret %d nbyte %d `%s')",
//				"PASSTHROUGH", ret, nbyte, ps);
					sprintf(debugstr, "%s Escape failed (ret %d nlen %d `%s')",
							"PASSTHROUGH", ret, nlen, ps);
					ret = wincancel(debugstr);
					if (ret) bUserAbort = 1;		// ???
					ShowLastError();
					return ret;
				}
			}
			nbyte = nbyte - nlen;
			ps += nlen;
		}
	}
	else {
#endif
	memcpy(buffer+sizeof(short), ps, nbyte);		/* 95/Dec/14 */

	*((short *) buffer) = snbyte;	/* store length in buffer 95/Dec/14 */
//  This is *one* Escape that *does* work in WIN32... 

	if (gPrCode == POSTSCRIPT_PASSTHROUGH) {
		ret = ExtEscape(hDC, gPrCode, (short) (nbyte+sizeof(short)), 
						(LPSTR) &buffer, 0, NULL);
		if (ret < 0) {
			sprintf(debugstr, "%s Escape failed (ret %d nbyte %d `%s')",
					"POSTSCRIPT_PASSTHROUGH", ret, nbyte, ps);
			ret = wincancel(debugstr);
			if (ret) bUserAbort = 1;		// ???
			ShowLastError();
//			return ret;
			gPrCode = PASSTHROUGH;		// fall back
		}
	}
	else if (gPrCode == PASSTHROUGH) {
		ret = Escape(hDC, gPrCode, (short) (nbyte+sizeof(short)), 
					 (LPSTR) &buffer, NULL);
		if (ret < 0) {
			sprintf(debugstr, "%s Escape failed (ret %d nbyte %d `%s')",
					"PASSTHROUGH", ret, nbyte, ps);
			ret = wincancel(debugstr);
			if (ret) bUserAbort = 1;		// ???
			ShowLastError();
			return ret;
		}
	}
//	}

	return 0;
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  *** */

/* pass paper size spec to DVIPSONE - unless its "letter" 95/Jun/22 */
/* papertype is in wingdi.h form DMPAPER_LETTER == 1 up etc.*/

char *insertpapersize (char *s, int papertype) {	/* 95/Jun/22 */
	if (bDebug > 1) {
		sprintf(debugstr, "papertype %d papersize %s",
			papertype, papersize[papertype]);
		OutputDebugString(debugstr);
//		wincancel(debugstr);			// debugging only
	}
	
	if (papertype == DMPAPER_LETTER) return s;	/* don't bother in this case */
/*	if (papertype > 1 && papertype < 14) */
/*	if (papertype > 1 && papertype <= MAXPAPERSIZE) */
//	if (papertype > 1 && papertype < maxpapersize) 
	if (papertype < maxpapersize)  {
		if (papertype == (DMPAPER_LETTER + IDM_CUSTOMSIZE - IDM_LETTER))	// 0
//			strcpy (s, "-l=");
//			strcat(s, szCustomPageSize);
//			strcat (s, " ");
			sprintf(s, "-l=%d*%d ", PageWidth, PageHeight);
		else {
			strcpy (s, "-l=");
			strcat(s, papersize[papertype]);
			strcat (s, " ");
		}
	}
	return s + strlen(s);
}

/* ********************************* NEWPRINTDLG / WIN32 */

/* Set up Driver, Printer/Device, Port, for default printer/device */

int setuseDVIPSONEsub(LPSTR);

int setuseDVIPSONE(void);

#define stc13       0x044c		/* from dlgs.h */
#define stc14       0x044d		/* from dlgs.h */
#define cmb4        0x0473		/* from dlgs.h */

void FillDevModeFields (void) {
	LPDEVMODE lpDevMode;
	if (pd.hDevMode != NULL) {
		lpDevMode = (LPDEVMODE) GlobalLock(pd.hDevMode); 
		if (bLandScape) lpDevMode->dmOrientation = DMORIENT_LANDSCAPE;
		else lpDevMode->dmOrientation = DMORIENT_PORTRAIT;
		lpDevMode->dmFields |= DM_ORIENTATION;
		if (papertype != 0) {					/* 96/July/7 */
			lpDevMode->dmPaperSize = (short) papertype;
			lpDevMode->dmFields |= DM_PAPERSIZE;
		}
		if (bUseDevModeCopies) {
			lpDevMode->dmCopies = (short) nCopies;
			lpDevMode->dmFields |= DM_COPIES;
			if (bCollateFlag) lpDevMode->dmCollate = DMCOLLATE_TRUE;
			else lpDevMode->dmCollate = DMCOLLATE_FALSE;
			lpDevMode->dmFields |= DM_COLLATE;
		}
		if (nDuplex != DMDUP_SIMPLEX) {
			lpDevMode->dmDuplex = (short) nDuplex;
			lpDevMode->dmFields |= DM_DUPLEX;
		}										/* ??? 96/Nov/17 */
		GlobalUnlock(pd.hDevMode);
	}
}

void ReadDevModeFields (void) {
	LPDEVMODE lpDevMode;
/*  should check lpDevMode.dmFields |= DM_ORIENTATION DM_PAPERSIZE ? */
	if (pd.hDevMode != NULL) {
		lpDevMode = (LPDEVMODE) GlobalLock(pd.hDevMode); 
		if (lpDevMode->dmFields & DM_ORIENTATION) {
			if (lpDevMode->dmOrientation == DMORIENT_LANDSCAPE)	bLandScape = 1;
			if (lpDevMode->dmOrientation == DMORIENT_PORTRAIT) bLandScape = 0;
		}
		if (papertype != 0) {	// 2000 May 27 preserve custom size
			if (lpDevMode->dmFields & DM_PAPERSIZE) {
				if (lpDevMode->dmPaperSize > 0 &&
					  lpDevMode->dmPaperSize < maxpapersize)
					papertype = lpDevMode->dmPaperSize;
			}
		}
		if (lpDevMode->dmFields & DM_DUPLEX) {
			nDuplex = lpDevMode->dmDuplex;
		}											/* 96/Nov/17 */
/*		possible values DMDUP_SIMPLEX, _VERTICAL and _HORIZONTAL */
		if (bUseDevModeCopies) {
			if (lpDevMode->dmFields & DM_COPIES) {
				if (lpDevMode->dmCopies > 0) nCopies = lpDevMode->dmCopies;
			}
			if (lpDevMode->dmFields & DM_COLLATE) {
				if (lpDevMode->dmCollate == DMCOLLATE_TRUE) bCollateFlag = 1;
				if (lpDevMode->dmCollate == DMCOLLATE_FALSE) bCollateFlag = 0;
			}
		}
/* 	lpDevMode.dmCopies lpDevMode.dmCollate */
		GlobalUnlock(pd.hDevMode);
	}
}

/**************************************************************************/

/*		Use CommDlgExtendedError() to get detailed error info */

/* If the most recent call to a common dialog box function succeeded,
   the return value is undefined. */
/* If the common dialog box function returned FALSE because the user closed
   or canceled the dialog box, the return value is zero. */
/* Otherwise, the return value is a nonzero error code. */

/* int ShowCommDlgError(char *s) */
DWORD ShowCommDlgError(char *s) {
	DWORD err;
	err = CommDlgExtendedError();
/*	err = 0 means user cancelled or closed the common dialog */
	if (err != 0) {
		sprintf(debugstr, "Common Dialog Error %lX in %s\nRefer to cderr.h\n",
			err, s);
/*		OutputDebugString(debugstr); */
		winerror(debugstr);
/*		for codes CDERR_..., see cderr.h */
	}
	return err;
}

/* *********************************** NEWPRINTDLG / WIN32 */

/* use PD_RETURNDEFAULT to retrieve info on default printer */
/* is this ever called later and thus wipes out info in structures already ? */
/* BOOL PASCAL GetDefPrinter (void) */
// BOOL PASCAL GetDefPrinterNew (void) {
BOOL PASCAL GetDefPrinter (void) {
/*	LPDEVNAMES dvnm; */
/*	DWORD err; */
	if (pd.hDevMode != NULL) GlobalFree(pd.hDevMode);		/* PRINTDLG */
	pd.hDevMode = NULL;							/* needs to be NULL here */
	if (pd.hDevNames != NULL) GlobalFree(pd.hDevNames); 	/* PRINTDLG */
	pd.hDevNames = NULL;						/* needs to be NULL here */
/*	memset (&pd, 0, sizeof(PRINTDLG)); */
	pd.lStructSize = sizeof(PRINTDLG);
/*	pd.hwndOwner = NULL; */
	pd.hwndOwner = hwnd;						/* ??? */
	pd.hDC = NULL;
/*  if DevMode & DevNames = NULL, PrintDlg inits use current default printer */
/*	returns DEVMODE and DEVNAMES for default printer if the above are NULL */
/*	NOTE: does not interact with user */
/*	Hence need not set up our own template here *//* set up more fields ??? */
	pd.Flags = PD_RETURNDEFAULT;
	if (PrintDlg(&pd)) {
/*		maybe don't bother to copy these strings, just use directly later ? */
/*		dvnm = (LPDEVNAMES) GlobalLock(pd.hDevNames);
		strcpy (achDriver, (LPSTR) (dvnm + dvnm->wDriverOffset));
		strcpy (achDevice, (LPSTR) (dvnm + dvnm->wDeviceOffset));
		strcpy (achPort, (LPSTR) (dvnm + dvnm->wOutputOffset));
	    GlobalUnlock(pd.hDevNames); */
		ReadDevModeFields();		/* ??? */
		return TRUE;				/* success */
	}
	else {
//		ShowCommDlgError("GetDefPrinterNew");
		ShowCommDlgError("GetDefPrinter");
		return FALSE;					/* failure */
	}
}

/****************************************************************************/

#ifdef IGNORED

BOOL PASCAL GetDefPrinter (void) {
#ifdef NEWPRINTDLG							// WIN32
	return GetDefPrinterNew ();				/* NEW */
#else
	return GetDefPrinterOld ();				/* OLD */
//	GetDefPrinterOld no longer exists...
#endif
}

#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

//	Acrobat Distiller,winspool,"G:\.....\*.pdf"
//	so check "Driver Name"   --- refers to global pd

int IsItDistiller (void) {
	LPDEVNAMES lpDevNames;
	LPSTR lpszDriver, lpszDevice, lpszPort;
	int flag=0;

	if (pd.hDevNames == NULL) return flag;
	
	lpDevNames = (LPDEVNAMES) GlobalLock(pd.hDevNames);
	lpszDriver = (LPSTR) lpDevNames + lpDevNames->wDriverOffset;
	lpszDevice = (LPSTR) lpDevNames + lpDevNames->wDeviceOffset;
	lpszPort = (LPSTR) lpDevNames + lpDevNames->wOutputOffset;
	if (strstr(lpszDevice, "Distiller") != NULL) flag = 1;
	if (bDebug > 1) {
		sprintf(debugstr, "%s => Distiller = %d", lpszDevice, flag);
		OutputDebugString(debugstr);
	}
	GlobalUnlock(pd.hDevNames);
	
	return flag;
}

/* HDC MakePrintDC(void) */
// HDC MakePrintDCNew(void) {
HDC MakePrintDC(void) {
	LPDEVNAMES lpDevNames;
	LPDEVMODE lpDevMode;
	LPSTR lpszDriver, lpszDevice, lpszPort;
	
/*	This conflicts with bKeepPrinterDC ??? */
	if (hPrintDC != NULL) {				/* sanity check */
#ifdef DEBUGPRINTING
		if (bDebug > 1) OutputDebugString("Delete Print DC\n");
#endif
		(void) DeleteDC(hPrintDC);		/* get rid of old one if any */
		hPrintDC = NULL;
	}
/*	If nothing set up get default printer information ??? */
/*	if (pd.hDevNames == NULL) (void) GetDefPrinter(); */
/*	Rather call PrintDlg below ... */
	if (pd.hDevNames != NULL) {			/* stuff already set up ? */
		lpDevNames = (LPDEVNAMES) GlobalLock(pd.hDevNames);
		lpszDriver = (LPSTR) lpDevNames + lpDevNames->wDriverOffset;
		lpszDevice = (LPSTR) lpDevNames + lpDevNames->wDeviceOffset;
		lpszPort = (LPSTR) lpDevNames + lpDevNames->wOutputOffset;
		if (pd.hDevMode != NULL) {
/*			use device mode data already set up */
			lpDevMode = (LPDEVMODE) GlobalLock(pd.hDevMode); 
/*			hPrintDC = CreateDC (achDriver, achDevice, achPort, lpDevMode);*/
			hPrintDC = CreateDC (lpszDriver, lpszDevice, lpszPort, lpDevMode); 
/*	in Windows 95 lpszDriver is ignored and should be NULL ??? */
/*	in Windows 95 lpszPort is ignored and should be NULL ??? */
			GlobalUnlock(pd.hDevMode);
		}
		else {
/*			if lpInitData = NULL, use default initialization */
/*			hPrintDC = CreateDC (achDriver, achDevice, achPort, NULL); */
			hPrintDC = CreateDC (lpszDriver, lpszDevice, lpszPort, NULL); 
/*			returns NULL if it fails */
		}
		GlobalUnlock(pd.hDevNames);
		return hPrintDC;
	}
/*	Does this mean user will get bothered again with PrintDlg box ??? */
/*	Just use current default printer instead ??? */
	else {									/* Do we ever get here ? */
/*		Should we set up our own template here ??? */
		pd.Flags = PD_RETURNDC;		/* what other flags to set ??? */
		if (PrintDlg (&pd)) {
			ReadDevModeFields(); /*	should we pick out other flags ? */ 
			return pd.hDC;
		}
		return NULL;
	}
}

/****************************************************************************/

#ifdef IGNORED

HDC MakePrintDC(void) {
#ifdef DEBUGPRINTING
	if (bDebug > 1) OutputDebugString("MAKEPRINTDC\n");
#endif
#ifdef NEWPRINTDLG						// WIN32
	return MakePrintDCNew();			/* NEW */
#else
	return MakePrintDCOld();			/* OLD */
//	MakePrintDCOld no longer exists...
#endif
}

#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Show printing error using LoadString - common routine 95/Dec/14 */

/* write back into mss (typically this is str) */

// may need to flush this! PRN_ERROR error strings no longer defined 

void printerror (int err, char *mss, int nlen, char *task, int show) {
	int flag;

	flag = LoadString(hInst, (UINT) (PRN_ERROR + err), mss, nlen);
	if (flag == 0) sprintf(mss, "Printing Error %d", err);
	strcat(mss, " --- ");							/* 96/Jan/6 */
	strcat(mss, task);
/*	if (show) winerror(str); */
	if (show) winerror(mss);
	else if (bDebug > 1) OutputDebugString(mss);
	ShowLastError();		// in WIN NT only ?
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int PrintPage(HWND hWnd, HDC hDC) {
	int flag, err;
	int paintflag;								/* 1992/Dec/20 */
/*	int oldrelative, newrelative=1; */
	
	if (hDC == NULL) return -1;					/* redundant */

#ifdef DEBUGPRINTING
	if (bDebug > 1) OutputDebugString("StartPage\n");
#endif
	err = StartPage(hDC);
	if (err < 0) {
		printerror(err, str, sizeof(str), "STARTPAGE", 1);
		return -1;
	}

	if (IsItPSCRPT(hDC) != 0) {
/*		Attempt to force emission of %%Page: etc in WIN32 96/Sep/12 */
/*		MoveToEx (hDC, dvi_hh, dvi_vv, NULL); */
		MoveToEx (hDC, 0, 0, NULL);
/*		LineTo (hDC, dvi_hh, dvi_vv); */
		LineTo (hDC, 0, 0);
/*		install decent half-tone screen */ /* need to do each page */
		if (bUseMagicScreen != 0) {
			passthrough(hDC, "\n", 1);
/*			UINT is 16 bit in WIN16 and 32 bit in WIN32 */
			flag = LoadString(hInst, (UINT) EPS_MAGIC, str, sizeof(str));
/*			passthrough(hPrintDC, str); */
			passthrough(hDC, str, flag);
		}
/*	Note: in WIN32 above appears between %%EndSetup and %%Page: ... */
/*	Note: in WIN32 above appears between %%PageTrailer and %%Page: ... */
/*	Note: in WIN16 above appears within page wrapped in %%BeginDocument ... */
	}

#ifdef IGNORED		/*	can only happen if DEBUGGING is defined ? */
	if (bPrintFrame != 0) DoCropMarks(hDC);
#endif

/*	install decent half-tone screen */ /* need to do each page */
/*	if (bUseMagicScreen != 0 && IsItPSCRPT(hPrintDC) != 0) {
		flag = LoadString(hInst, (UINT) EPS_MAGIC, str, sizeof(str));
		passthrough(hPrintDC, str, flag);
	} */

	paintflag = paintpage(hWnd, hDC);

#ifdef COMMENT				/*	was an experiment not sure this is right ... */
	if (bUserAbort != 0) {	/* maybe only if not first NEWFRAME ? */

#ifdef DEBUGPRINTING
		if (bDebug > 1) OutputDebugString("AbortDoc\n");
#endif
		(void) AbortDoc(hDC);

/*		if (bDebug > 1)	OutputDebugString("Escape ABORTDOC (bUserAbort)\n"); */
		bUserAbort = -1;	/* flipped ! 1992/Nov/3 */
		return -1;		/* indicate failure */
	}
		
/*	if (paintflag < 0) return -1;	*/	/* failed */
	if (paintflag < 0) {		/* maybe only if not first NEWFRAME ? */

#ifdef DEBUGPRINTING
		if (bDebug > 1) OutputDebugString("AbortDoc\n");
#endif
		(void) AbortDoc(hDC);

/*		if (bDebug > 1)	OutputDebugString("Escape ABORTDOC (flag)\n"); */
		bUserAbort = -1;		/* flipped ! 1992/Nov/3 */
		return -1;				/* indicate failure */
	}
#endif	/* end of experiment ifdef COMMENT ? */

/*	if (paintflag < 0) bUserAbort = 1; */

#ifdef DEBUGPRINTING
	if (bDebug > 1) OutputDebugString("EndPage\n");
#endif
	err = EndPage(hDC);

	if (err < 0) {
		bPrintSuccess = FALSE;
		if (err == 0 ||				/*  0 */	/* when fax cancel 94/Jun/21 */
			err == SP_ERROR ||		/* -1 */	/* 1993/Aug/27 ??? */
			err == SP_APPABORT ||	/* -2 */
			err == SP_USERABORT 	/* -3 */
				) flag = 0;			/* OutputDebugString only */
		else flag = 1;				/* use winerror */
		printerror(err, str, sizeof(str), "NEWFRAME", flag);
		return -1;
	}
	if (paintflag < 0) return -1;			/* 1992/Nov/3 */
	return 0;								/* no problem */
}

/*	Gray PRINT menu item if no printer DC, otherwise enable */
/*	Does this really make sense ? */ /* changed 92/Feb/18 */

void checkprintmenu(BOOL flag) {
	flag = 1;		/* don't really want to disable `Print' 92/Feb/18 */
	(void) EnableMenuItem(GetMenu(hwnd), IDM_PRINTVIEW, 
		((flag != 0) && (bFileValid != 0 || bFontSample != 0)) ?
			MF_ENABLED : MF_GRAYED);
	(void) EnableMenuItem(GetMenu(hwnd), IDM_PRINTALL, 
		((flag != 0) && (bFileValid != 0)) ?		/*  || bFontSample != 0 */
			MF_ENABLED : MF_GRAYED);
}

/* Print a Blank Page (for odd/even duplex printing) 1993 August 5 */

int BlankPage(HWND hWnd, HDC hDC) {
	int err, flag;

#ifdef DEBUGPRINTING
	if (bDebug > 1) OutputDebugString("StartPage\n");
#endif
	err = StartPage(hDC);
	if (err < 0) {
		printerror(err, str, sizeof(str), "STARTPAGE", 1);
		return -1;
	}

#ifdef DEBUGPRINTING
	if (bDebug > 1) OutputDebugString("EndPage\n");
#endif
	err = EndPage(hDC);

	if (err < 0) {
		bPrintSuccess = FALSE;
		if (err == SP_APPABORT ||
			err == SP_USERABORT ||
			err == SP_ERROR				/* 1993/Aug/27 ??? */
				) flag = 0;				/* OutputDebuString only */
		else flag = 1;					/* use winerror */
		printerror(err, str, sizeof(str), "NEWFRAME", flag);
		return -1;
	}
	return 0;
}

/* for some reason this is not defined in WIN32 wingdi.h */
typedef struct tagSCREENPARAMS {
  int   angle;
  int   frequency;
} SCREENPARAMS;

// split out 97/July/20
// returns -1 if fails

int GDIPrint (HWND hWnd, int fullflag, int bOldSpreadFlag) {
	int k, kcollate, err;
	BOOL bFirstPage;	/* non-zero when nothing printed yet */
	BOOL bSkipFlag;		/* non-zero when skipping this page */

/*	following is attempt to deal with request for collated printing */
	if (bCollateFlag != 0 && nCopies > 1) kcollate = nCopies;
	else kcollate = 1;

	for (k = 0; k < kcollate; k++) {		/* 1995/Dec/15 */	
/*		Following two page spread printing mode flushed */
/*		1994/Aug/17 - inconsistent with DVIPSONE */
		if (bOldSpreadFlag != 0) {		/* reset bSpreadFlag for now */
			bSpreadFlag = 0;
		}
		if (fullflag != 0) {			/* do full document ... */
/*			for (dvipage = frompage; dvipage <= topage; dvipage++) */
			if (topage < frompage) {	/* safety check 92/Sep/27 */
				err = frompage; frompage = topage; topage = err;
				bReversePages = 1;		/* ??? */
			}
			if (pageincrement < 0) {	/* safety catches 92/Sep/27 */
				pageincrement = - pageincrement;
				bReversePages = 1;		/* ??? */
			}
			if (pageincrement == 0) pageincrement = 1;
			if (bOddEven != 0) {			/* 1993/Aug/28 */
/*				if ((bReversePages != 0 && bOddOnly != 0) ||
					(bReversePages == 0 && bEvenOnly != 0)) 
					dvipage = topage;
				else dvipage = frompage; */	/* fix bug 1994/Jan/12 */
				if (bOddOnly != 0 || bEvenOnly != 0) {
					if ((bReversePages != 0 && bOddOnly != 0) ||
						(bReversePages == 0 && bEvenOnly != 0)) 
						dvipage = topage;
					else dvipage = frompage;
				}
				else if (bReversePages != 0) dvipage = topage;
				else dvipage = frompage;	/* normal case */
			}
			else {				/* old way of doing this */
				if (bReversePages != 0) dvipage = topage;
				else dvipage = frompage;
			}
/* New attempt at odd/even duplex printing --- 1993 Aug 5 */
/* Watch out for match with what call to DVIPSONE does !!! */
/* Check pageincrement instead of bAlternatePages ? */
/* Insert blank pages ? */  /* What if bCountZero on ? */
/* This is a kludge - should really look at counter[0] of page as we go */
/*			if (bAlternatePages != 0 && bOddEven != 0) {
				if (bReversePages == 0) {
					if (dvipage % 2 == 0) {
						if (BlankPage(hWnd, hPrintDC) < 0) goto printexit;
						dvipage++;
				}
				}
				else {
					if (dvipage % 2 != 0) {
						if (BlankPage(hWnd, hPrintDC) < 0) goto printexit;
							dvipage--;
					}
				}
			} */
			if (bOddEven != 0 && (bOddOnly != 0 || bEvenOnly != 0)) {
/* use the following when using new scheme AND not printing all pages */
/* BRAND NEW ON 1993/Aug/28 */
				bSkipFlag = 0; bFirstPage=1; 
/*		first figure out which end to start from */
				if ((bReversePages != 0 && bOddOnly != 0) ||
					(bReversePages == 0 && bEvenOnly != 0)) {
					dvipage = dvi_t;
				}
				else dvipage = 1;			/* not zero based ! */

/* the range is `frompage' to `topage' --- inclusive */
				for (;;) {
					usepagetable(dvipage, 0);
					if (dvipage < frompage || dvipage > topage) bSkipFlag = 1;
					else bSkipFlag = 0;
					if (bSkipFlag != 0)
						bFirstPage = 1; /* reset for next group */
					if (bOddOnly != 0) {	/* if printing only odd pages */
						if ((counter[0] & 1) == 0) { /* seen even page */
							if (bFirstPage != 0)   /* need matching blank */
								if (BlankPage(hWnd, hPrintDC) < 0)
//									goto printexit;
									return -1;
							bSkipFlag++;
						}
						bFirstPage = 0;			
					}
					if (bEvenOnly != 0) {	/* if printing only even pages */
						if ((counter[0] & 1) == 1) {	/* seen odd page */
							if (bFirstPage != 0)  /* need matching blank */
								if (BlankPage(hWnd, hPrintDC) < 0)
//									goto printexit;
									return -1;
							bSkipFlag++;
						}
						bFirstPage = 0;
					}
					if (bSkipFlag == 0) {	/* if page not being skipped */
/* change caption on dialog box */
						strcpy(str, "Sending ");  /* change page number */
/*						addpagestring(str + strlen(str), bSpreadFlag); */
						addpagestring(str + strlen(str));
//						strcat(str, " of");
						if (hDlgPrint != NULL)
							SetDlgItemText(hDlgPrint, IDC_SOURCE, str); 
#ifdef DEBUGPRINTING
						if (bDebug > 1) OutputDebugString(str);
#endif
//						if (PrintPage(hWnd, hPrintDC) < 0) goto printexit;
						if (PrintPage(hWnd, hPrintDC) < 0) return -1;
/* skip out if there is an error ... */
					}
/* step backward if bReversePages != 0  and bOddOnly != 0 */
/* step backward if bReversePages == 0  and bEvenOnly != 0 */					
					if ((bReversePages != 0 && bOddOnly != 0) ||
						(bReversePages == 0 && bEvenOnly != 0)) {
						dvipage--;
/*						if (dvipage < frompage) break; */
						if (dvipage <= 0) break;
					}
					else {				/* otherwise step forward */
						dvipage++;
/*						if (dvipage > topage) break; */
						if (dvipage > dvi_t) break;
					}
				}	/* end of loop through all DVI pages */
			} /* end of odd/even printing the new way */
			else {

/* use the following when printing all pages or when using old scheme */
				for(;;) {						/* 1992/Sep/25 */
					usepagetable(dvipage, 0);
					strcpy(str, "Sending ");  /* change page number */
/*					addpagestring(str + strlen(str), bSpreadFlag); */
					addpagestring(str + strlen(str));
					strcat(str, " of");
					if (hDlgPrint != NULL)
						SetDlgItemText(hDlgPrint, IDC_SOURCE, str); 
#ifdef DEBUGPRINTING
					if (bDebug > 1) {
						sprintf(debugstr, "Printing page %d\n", dvipage);
						OutputDebugString(debugstr);
					}	/* 95/Sep/10 */
#endif
//					if (PrintPage(hWnd, hPrintDC) < 0) goto printexit;
					if (PrintPage(hWnd, hPrintDC) < 0) return -1;
/* skip out if there is an error ... */
					if (bReversePages != 0) {	/* 1992/Sep/25 */
						dvipage -= pageincrement;
						if (dvipage < frompage) break;
					}
					else {
						dvipage += pageincrement;
						if (dvipage > topage) break;
					}
				}
			}	/* end of doing all pages or doing old way case */
		}	/* end of full document case */
		else {						/* single page of document */
//			if (PrintPage(hWnd, hPrintDC) < 0) goto printexit; 
			if (PrintPage(hWnd, hPrintDC) < 0) return -1;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////


void WritePrivateProfileInt (char *achSect, char *szKey, int n, char *achFile) {
	char buffer[32];
	sprintf(buffer, "%d", n);
	WritePrivateProfileString(achSect, szKey, buffer, achFile);
}

// following is for debugging purposes only

void ShowPrinterSupport (HDC hPrintDC, int verboseflag) {
	int iEsc, ret;
	char *s=debugstr;

	iEsc = QUERYESCSUPPORT;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "QUERYESCSUPPORT", ret, achFile);
	sprintf(s, "%s %d\n", "QUERYESCSUPPORT", ret);

	s += strlen(s);

	iEsc = GETTECHNOLOGY;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "GETTECHNOLOGY", ret, achFile);
	sprintf(s, "%s %d\n", "GETTECHNOLOGY", ret);
	s += strlen(s);

	iEsc = EPSPRINTING;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "EPSPRINTING", ret, achFile);
	sprintf(s, "%s %d\n", "EPSPRINTING", ret);
	s += strlen(s);

	iEsc = ENCAPSULATED_POSTSCRIPT;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "ENCAPSULATED_POSTSCRIPT", ret, achFile);
	sprintf(s, "%s %d\n", "ENCAPSULATED_POSTSCRIPT", ret);
	s += strlen(s);

	iEsc = PASSTHROUGH;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "PASSTHROUGH", ret, achFile);
	sprintf(s, "%s %d\n", "PASSTHROUGH", ret);
	s += strlen(s);

	iEsc = POSTSCRIPT_PASSTHROUGH;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "POSTSCRIPT_PASSTHROUGH", ret, achFile);
	sprintf(s, "%s %d\n", "POSTSCRIPT_PASSTHROUGH", ret);
	s += strlen(s);

	iEsc = POSTSCRIPT_DATA;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "POSTSCRIPT_DATA", ret, achFile);
	sprintf(s, "%s %d\n", "POSTSCRIPT_DATA", ret);
	s += strlen(s);

	iEsc = POSTSCRIPT_IGNORE;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "POSTSCRIPT_IGNORE", ret, achFile);
	sprintf(s, "%s %d\n", "POSTSCRIPT_IGNORE", ret);
	s += strlen(s);

	iEsc = POSTSCRIPT_IDENTIFY;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "POSTSCRIPT_IDENTIFY", ret, achFile);
	sprintf(s, "%s %d\n", "POSTSCRIPT_IDENTIFY", ret);
	s += strlen(s);

	iEsc = POSTSCRIPT_INJECTION;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "POSTSCRIPT_INJECTION", ret, achFile);
	sprintf(s, "%s %d\n", "POSTSCRIPT_INJECTION", ret);
	s += strlen(s);

	iEsc = OPENCHANNEL;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "OPENCHANNEL", ret, achFile);
	sprintf(s, "%s %d\n", "OPENCHANNEL", ret);
	s += strlen(s);

	iEsc = DOWNLOADHEADER;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "DOWNLOADHEADER", ret, achFile);
	sprintf(s, "%s %d\n", "DOWNLOADHEADER", ret);
	s += strlen(s);

	iEsc = DOWNLOADFACE;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "DOWNLOADFACE", ret, achFile);
	sprintf(s, "%s %d\n", "DOWNLOADFACE", ret);
	s += strlen(s);

	iEsc = CLOSECHANNEL;
	ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
	WritePrivateProfileInt(achEsc, "CLOSECHANNEL", ret, achFile);
	sprintf(s, "%s %d\n", "CLOSECHANNEL", ret);
	s += strlen(s);

	if (verboseflag) winerror(debugstr);
	if (bDebug > 1) OutputDebugString(debugstr);

}

// #define POSTSCRIPT_IDENTIFY          4117   /* new escape for NT5 pscript driver */
// #define POSTSCRIPT_INJECTION         4118   /* new escape for NT5 pscript driver */


////////////////////////////////////////////////////////////////////////////

//	Undo coordinate transformation in PSCRIPT header
//	PSCRPT driver coordinates are device pixels from top left corner down
//	While PS default is 72 per inch from bottom left corner

void nukeheader (HDC hPrintDC) {
	int xres, yres, xpxl, ypxl;
	xpxl = GetDeviceCaps(hPrintDC, PHYSICALWIDTH);
	ypxl = GetDeviceCaps(hPrintDC, PHYSICALHEIGHT);
	sprintf(str, "0 %d translate\n", ypxl);
	passthrough(hPrintDC, str, strlen(str));
	xres = GetDeviceCaps(hPrintDC, LOGPIXELSX);
	yres = GetDeviceCaps(hPrintDC, LOGPIXELSY);
	sprintf(str, "%d 72 div %d 72 div neg scale\n", xres, yres);
	passthrough(hPrintDC, str, strlen(str));
}

//	Trailer has junk like:
//	Pscript_WinNT_Compat dup /suspend get exec
//	Pscript_WinNT_Incr dup /resume get exec
//	LH	% i.e. showpage
//	an alternative is to use 10 {currentfile 128 string readline} repeat
//	read to %%PageTrailer or %%EOF

void nuketrailer (HDC hPrintDC) {
	strcpy(str, "/exec{pop}def\n");
	passthrough(hPrintDC, str, strlen(str));
	strcpy(str, "/restore{pop}def\n");
	passthrough(hPrintDC, str, strlen(str));
	strcpy(str, "/showpage{}def\n");
	passthrough(hPrintDC, str, strlen(str));
	strcpy(str, "/LH{}def\n");
	passthrough(hPrintDC, str, strlen(str));
	strcpy(str, "/colspRefresh{}def\n");
	passthrough(hPrintDC, str, strlen(str));
}

///////////////////////////////////////////////////////////////////

#define MAXJOBNAME 31				// but why ?

int ConstructCommandLine(HWND);

// get here to do actual printing --- called from DoPrintAll and
// from DVIWindo's PRINTVIEW
// fullflag == 0 print current view
// fullflag != 0 print full document

int DoPrintStuff (HWND hWnd, int fullflag) {
	POINT PrintingOffset;
	POINT SetArr[1];
	RECT PageRect;				/* not accessed */
	int bOldSpreadFlag;
	int olddvipage;
	long oldxoffset, oldyoffset;
	int err, ret, flag;
	int bUseStartEndDoc=1;		/* default use StartDoc / EndDoc */
	SCREENPARAMS ScreenParams;
	DOCINFO DocInfo;			/* Title etc. for StartDoc */
	DWORD nJobID;
//	char *s;
//	PRINTER_DEFAULTS pDefault;

	if (bFileValid == 0 && bFontSample == 0) return 0;	/* nothing to do */

	if (bBusyFlag == 0) bBusyFlag++;		// get busy

	if (bFileValid) {
		GrabWidths();			/* 99/Jan/12 ??? */
		touchallfonts(hWnd);	/* we want this on main window hDC ? */
		ReleaseWidths();		/* 99/Jan/12 ??? */
	}

	if (hPrintDC == NULL) {		/* no saved printed DC --- have to make one */
		bHourFlag = -1; 
		hSaveCursor = SetCursor(hHourGlass);
		if (pd.hDevNames == NULL) (void) GetDefPrinter();		/* NEW */
		hPrintDC = MakePrintDC();	/* NEW or OLD */
		bHourFlag = 0;
		(void) SetCursor(hArrow);	

/*		checkprintmenu(hPrintDC); */	/* new:  check or uncheck Print */
		if (hPrintDC != NULL) checkprintmenu(TRUE);
		else checkprintmenu(FALSE);
		if (hPrintDC == NULL) {
/*			winerror("No Printer DC"); */ /* e.g. if null port selected */
			if (pd.hDevNames == NULL)
				winerror ("Unable to load printer driver"); 
			bBusyFlag = 0;
			return -1;
		}
	}

/*	if (bDebug > 1) OutputDebugString("Driver loaded\n"); */ /* 95/Sep/10 */

/*  get ready in case we skip to printexit */
	hDlgPrint = NULL; 

	bOldSpreadFlag = bSpreadFlag;			/* remember old values */
	olddvipage = dvipage;
	oldxoffset = xoffset; oldyoffset = yoffset;

/*	now supposedly have an acceptable printer DC ... */
	(void) SetMapMode(hPrintDC, MM_TWIPS);  

/*	cxInch = GetDeviceCaps (hPrintDC, LOGPIXELSX);
	cyInch = GetDeviceCaps (hPrintDC, LOGPIXELSY); */

//	Does the following make sense ?

	PrintingOffset.x = GetDeviceCaps (hPrintDC, PHYSICALOFFSETX);
	PrintingOffset.y = GetDeviceCaps (hPrintDC, PHYSICALOFFSETY);
#ifdef DEBUGPRINTING
	if (bDebug > 1) {
		sprintf(debugstr, "Offset %d %d\n", PrintingOffset.x, PrintingOffset.y);
		OutputDebugString(debugstr);
	}
#endif

/*	sprintf(str, "x %d y %d", PrintingOffset.x, PrintingOffset.y); 
	winerror(str); */
/*  printing offsets are in device coordinates *//* convert to logical coor */
	SetArr[0].x = PrintingOffset.x; SetArr[0].y = PrintingOffset.y; 
	(void) DPtoLP(hPrintDC, SetArr, 1);
	xprintoffset = -SetArr[0].x;
	yprintoffset = -SetArr[0].y;
/*	sprintf(str, "x %d y %d", xprintoffset, yprintoffset); 
	winerror(str); */

// fiddle with halftone screen function

	if (ScreenFrequency != 0 || ScreenAngle != 32767) {
/*		Get Old Values First */ /* probably won't work in WIN32 */

/*		GETSETSCREENPARAMS Escape may very well not work in WIN32 ??? */
		err = ExtEscape(hPrintDC, GETSETSCREENPARAMS, sizeof(SCREENPARAMS), 
			NULL, sizeof(ScreenParams), (LPSTR) &ScreenParams);

/*		if (err < 0) */
		if (err <= 0) {							/* 1995/Dec/21 */
			if (bDebug) {
				if (bDebug > 1)
					OutputDebugString("GETSETSCREENPARAMS Escape failed\n");
				else winerror("GETSETSCREENPARAMS Escape failed\n");
			}
		}
/*		Change those that were specified */
		if (ScreenAngle != 0) 
			ScreenParams.angle = ScreenAngle;
		if (ScreenFrequency != 32767) 
			ScreenParams.frequency = ScreenFrequency;
/*		Now set the new values */ /* probably won't work in WIN32 */

		err = ExtEscape(hPrintDC, GETSETSCREENPARAMS, sizeof(SCREENPARAMS),
			(LPSTR) &ScreenParams, 0, (LPSTR) &ScreenParams);

/*		err = Escape(hPrintDC, GETSETSCREENPARAMS, sizeof(SCREENPARAMS),
			(LPSTR) &ScreenParams, NULL); */

/*		if (err < 0) */
		if (err <= 0) {
			if (bDebug) {
				if (bDebug > 1)
					OutputDebugString("GETSETSCREENPARAMS Escape failed\n");
				else winerror("GETSETSCREENPARAMS Escape failed\n");
			}
		}
	}

/*	oldxoffset = xoffset; oldyoffset = yoffset; */
	bPrintFlag = 1;				/* prevent WM_PAINT etc */
	xoffset += xprintoffset;
	yoffset += yprintoffset;

#ifdef IGNORED
/*	should AbortProc be set up AFTER PrintDlg ??? */
	bUserAbort = FALSE;
	oldaborterr = 0;

#ifdef DEBUGPRINTING
	if (bDebug > 1) OutputDebugString("SetAbortProc\n");
#endif
	err = SetAbortProc (hPrintDC, (ABORTPROC) AbortProc);

/*	if (err < 0) {    */	/* SetAbortProc > 0 if OK */
	if (err <= 0) {   
		winerror("SETABORTPROC failed");
		bPrintFlag = 0;				/* ??? */
		goto printloss;				/* inappropriate ? NEW */
	}
#endif

/*	if (bDebug > 1) OutputDebugString("SetAbortProc done\n"); */ /* 95/Sep/10 */

/*	bHourFlag = -1; */
/*	hSaveCursor = SetCursor(hHourGlass); */
/*	bPrintFlag = 1; */

/*	should AbortProc be set up AFTER PrintDlg ??? */
/*	moved here to avoid problem with Fax Printer Driver ... */
/*  contrary to SDK Guide, but as in Yao & Norton */
/*	But possible problem with `Print to File' dialog put up *after* this */
	bUserAbort = FALSE;
	bPrintSuccess = TRUE;
/*	Petzold disables main Window first ??? */
	(void) EnableWindow(hWnd, FALSE); 	/* disable main Window ??? */

//	Ask for file name if needed
//	In case of DVIPSONE, should we just send the file to standard place ???

//	moved up here to be before PrintDlgProc dialog box creation 96/Aug/22 
//	to prevent losing focus on print dialog box 

//	We don't want to ask for file name if print dialog will do it later anyway
//	that is, if DVIPSONE.DLL us used in CallBackPass mode 99/July/21
//	or if we are not using DVIPSONE

//	in case of Printing to File, don't bother with callback
//	(except for debugging purposes with code sent to driver in callback

	bCallBack = bCallBackPass;					// default selected by user
	if (bUseDVIPSONE && bPrintToFile) {
		bCallBack = 0;
		if (bForcePassBack) bCallBack = 1;
	}

//	Get output file name from user if needed

//	if (bPrintToFile) {					/* is needed / OK in Windows 3.1 ??? */
//	if (bPrintToFile && (bUseDVIPSONEDLL == 0 || bCallBackPass == 0)) {
//	if (bPrintToFile && (bUseDVIPSONE == 0 || bUseDVIPSONEDLL == 0 || bCallBackPass == 0)) {
//	if (bPrintToFile && bUseDVIPSONE && (bUseDVIPSONEDLL == 0 || bCallBackPass == 0)) {
//	special case now always get File Name if using DVIPSONE - avoid STARTDOC/ENDDOC
	if (bPrintToFile && bUseDVIPSONE &&
//		(bUseDVIPSONEDLL == 0 || bCallBack == 0)) {
		(bUseDLLs == 0 || bCallBack == 0)) {
/*		Maybe use `Save As' Common Dialog instead ? */
		if (bDontAskFile == 0) {
//			bApplicationFlag = 0;			/* ? */
#ifdef DEBUGPRINTING
			if (bDebug > 1)	OutputDebugString("DialogBoxParam FileSelect");
#endif
			flag = DialogBoxParam(hInst, "FileSelect", hWnd, FileDlg, 0L);
									/* indicate print output file mode */
			if (flag == 0) {
/*				winerror ("Printing Cancelled"); */	/* exit ? return ? */
				goto printloss;						/* correct place ? */
			}								/* else File Name in szHyperFile */
/*	Use file name that was put in `szHyperFile' by FileDlg */
/*	Dialog puts name temporarily in szHyperFile - need to make copy now */
			if (szPrintFile != NULL) free(szPrintFile);
			szPrintFile = zstrdup(szHyperFile);
/*			DocInfo.lpszOutput = szPrintFile; *//* output file Print To File */
//			we now have the output file name
		}
	}
/*	else DocInfo.lpszOutput = NULL; */

#ifdef DEBUGPRINTING
	if (bDebug > 1)	OutputDebugString("CreateDialog DVIPrintDlg");
#endif
//	hDlgPrint = CreateDialog (hInst, "DVIPrintDlg", hWnd, (DLGPROC) PrintDlgProc);
	hDlgPrint = CreateDialog (hInst, "DVIPrintDlg", hWnd, PrintDlgProc);
 	if (hDlgPrint == NULL) {
		winerror("Null hDlgPrint");
/*		if (bDebug) winerror("Null hDlgPrint"); */
	}
	else {
		(void) ShowWindow(hDlgPrint, SW_NORMAL);	/* ??? */
		UpdateWindow(hDlgPrint);					/* ??? */
/*		SetFocus(hDlgPrint);	*/			/* experiment ? */
	}
/*	if (bDebug > 1) {		
		sprintf(debugstr, "hDlgPrint %X\n", hDlgPrint);
		OutputDebugString(debugstr);
	} */									/* debugging 92/Dec/20 */
/*	should following be BEFORE CreateDialog ? */
/*	(void) EnableWindow(hWnd, FALSE); */	/* disable main Window */
										/* IMPORTANT */
/*	SetFocus(hDlgPrint); */			/* experiment ? */

/*	if (bDebug > 1) OutputDebugString("Disable Main\n"); */ /* 95/Sep/10 */

/*	Page Rectangle in device coordinates */ /* just before STARTDOC */
	PageRect.left =  0;
	PageRect.top = 0;
	PageRect.right = (PageWidth * 300) / 72;
	PageRect.bottom = (PageHeight * 300) / 72;
/*	Escape(hPrintDC, SET_BOUNDS, sizeof(RECT), (LPSTR) &PageRect, NULL); */

//	job name --- limit jobname to 31 (60) characters ? - why ?
//	strcpy(str, "DVIWindo ");
	*str = '\0';						// 99/Aug/15
	if (bFontSample) {
		if (strlen(TestFont) < MAXJOBNAME) strcat(str, TestFont); 
		else {
			strncpy(str, TestFont, MAXJOBNAME);
			*(str+MAXJOBNAME) = '\0';
		}
	}
	else if (bFileValid) {
		if (strlen(OpenName) < MAXJOBNAME) strcat(str, OpenName); 
		else {
			strncpy(str, OpenName, MAXJOBNAME);
			*(str+MAXJOBNAME) = '\0';
		}
	}

	bUsingDistiller=0;

//	see whether it supports POSTSCRIPT_PASSTHROUGH
//	if (bUseDVIPSONE && bUseDVIPSONEDLL && bCallBackPass) {
	if (bUseDVIPSONE && bUseDLLs) {
		int iEsc;

		if (bDebug > 1 || bDebugMenu) 
			ShowPrinterSupport(hPrintDC, 0);		// debugging aid [Diagnostics]

		iEsc = POSTSCRIPT_PASSTHROUGH;
		ret = Escape(hPrintDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &iEsc, NULL);
		if (ret == 0) {
			if (bPrintToFile) {						// 2000 June 16
				if (wincancel("SORRY: Apparently not a PostScript device"))
					goto printloss;
				
			}
			else {
				winerror("SORRY: Apparently not a PostScript device");
				goto printloss;
			}
		}

		if (bNewPassThrough) gPrCode = POSTSCRIPT_PASSTHROUGH;
		else gPrCode = PASSTHROUGH;
	
		bUsingDistiller = IsItDistiller();
//		Following special case may not really needed...
//		Distiller is OK with the PSCRIPT header and trailer
//		but not with headers injected in the middle of the job 99/Nov/30
		if (bUsingDistiller) {					// special case Distiller
			if (bOldPassThrough) gPrCode = PASSTHROUGH;
			else gPrCode = POSTSCRIPT_PASSTHROUGH;
//			if (WinVerNum == 0x0400)	// only need in Windows ??? 95 99/Nov/30
			if (WinVerNum <= 0x0410)	// need in Windows 95/98 ? 99/Dec/5
				gPrCode = PASSTHROUGH;
		}

		if (bOpenCloseChannel || bOpenClosePrinter) bUseStartEndDoc = 0;

#ifdef IGNORED
//		Try it, even if it claims not to be implemented
		iEsc = 1;
		ret = ExtEscape(hPrintDC, EPSPRINTING, sizeof(int), (LPSTR) &iEsc, 0, NULL);
		if (bDebug > 1)	OutputDebugString("EPSPRINTING");
		if (ret > 0) {
//			gPrCode = POSTSCRIPT_PASSTHROUGH;
		}
		else {
			winerror("EPSPRINTING escape failed");
		}
#endif

	}
	WritePrivateProfileString(achDiag, "PassThrough_Used",
			 (gPrCode == PASSTHROUGH) ? "PASSTHROUGH" : "POSTSCRIPT_PASSTHROUGH",
									 achFile);

//	moved down here so it appears *after* the print dialog 99/Aug/1

/*	should AbortProc be set up AFTER PrintDlg ??? */
	bUserAbort = FALSE;
	oldaborterr = 0;

#ifdef DEBUGPRINTING
	if (bDebug > 1) OutputDebugString("SetAbortProc\n");
#endif
	err = SetAbortProc (hPrintDC, (ABORTPROC) AbortProc);

/*	if (err < 0) {    */	/* SetAbortProc > 0 if OK */
	if (err <= 0) {   
		winerror("SETABORTPROC failed");
		bPrintFlag = 0;				/* ??? */
		goto printloss;				/* inappropriate ? NEW */
	}


/*  STARTDOC is where user is asked for file name if printing to FILE: */
/*  STARTDOC is where user is asked for fax number if printing to fax */

/*	If fax cancelled here, we don't get an error indication ? */
/*	But everything sent to that device context disappears ... */
/*	Or we get an `impossible' error code of zero ! */

/*	if (bDebug > 1) OutputDebugString("StartDoc call\n"); */ /* 95/Sep/10 */

/*	Is str a safe place for anything while calling StartDoc ? static ? */ 
/*	lpszDocName should be no longer than 32 bytes including null */
/*	lpszOutput is file name if output redirected, else NULL send to device */
/*	May need to setup lpszOutput here ??? */

	DocInfo.cbSize = sizeof(DOCINFO);	/* size of structure ? */
	DocInfo.lpszDocName = str;			/* Name of document file */
//	DocInfo.lpszDocName = NULL;			// debugging

//	put in file name if available (else FILE: will appear)
	if (bPrintToFile) DocInfo.lpszOutput = szPrintFile;
	else DocInfo.lpszOutput = NULL;
//	If DocInfo.lpszOutput == NULL it will put up a dialog to ask for	filename
//	sprintf(debugstr, "WINVER %8X szPrintFile %s", WinVerNum, szPrintFile);
//	winerror(debugstr);	// debugging

//	if we are asking DVIPSONE to print direct, we need to change the
//	file name here so Windows 95 doesn't open the bloody file!

//	check whether this nonsense is also needed in Windows 98 ???

//	if (WinVerNum == 0x0400) {	// only need in Windows ??? 95 99/Nov/28
	if (WinVerNum <= 0x0410) {	// need in Windows 95 / 98 ? 99/Dec/4
		if (bPrintToFile && bUseDVIPSONE && // bUseDLLs &&
		  (bCallBack == 0)) {
			DocInfo.lpszOutput = "bogosity.pst";	// 99/Nov/28 
		}
	}

// #if (WINVER >= 0x0400)		default in 32 bit compiler
	DocInfo.lpszDatatype = NULL;
	DocInfo.fwType = 0;			// Windows 95 only; ignored on Windows NT 
// #endif

//	can't use PASSTHROUGH yet

	if (bOpenClosePrinter) {
//		DOC_INFO_2 Info;			// Windows 95/98
		DOC_INFO_1 Info;			// Win NT / W2K and Windows 95/98
		LPDEVNAMES lpDevNames;
		LPSTR lpszDriver, lpszDevice, lpszPort;

		if (pd.hDevNames == NULL) goto printloss;

		lpDevNames = (LPDEVNAMES) GlobalLock(pd.hDevNames);
		lpszDriver = (LPSTR) lpDevNames + lpDevNames->wDriverOffset;
		lpszDevice = (LPSTR) lpDevNames + lpDevNames->wDeviceOffset;
		lpszPort = (LPSTR) lpDevNames + lpDevNames->wOutputOffset;
	
//		wininfo(str);
//		wininfo(lpszDriver);
//		wininfo(lpszDevice);
//		wininfo(lpszPort);
		memset(&Info, 0, sizeof(Info));

		Info.pDocName = str;				// name of document
		if (bPrintToFile)
			Info.pOutputFile = szPrintFile;	// name output file or port ???
		else 
			Info.pOutputFile = lpszPort;	// name output file or port ???
//		Info.pOutputFile = "foobar";
		Info.pDatatype = NULL;				// type of data
//		Info.dwMode = DI_CHANNEL_WRITE;	
//		Info.dwMode = 0;					// Win 95/98 only ?
//		Info.JobId = 0;						// Win 95/98 only ?

//		First arg should be "friendly" printer name
//		last argument pointer to PRINTER_DEFAULTS structure
//		if NULL can use SetJob later to change later
//		pDefault.pDataType = "";
//		pDefault.pDevMode = lpDevMode;
//		pDefault.DesiredAccess = PRINTER_ACCESS_USE;
		if (! OpenPrinter(lpszDevice, &hPrinter, NULL)) {
			sprintf(debugstr, "OpenPrinter failed on %s", lpszDevice);
			ret = wincancel(debugstr);
			ShowLastError();
			GlobalUnlock(pd.hDevNames);
			goto printloss;
//			bUseStartEndDoc = 1;
		}

//		Second argument indicated DOC_INFO_1 or DOC_INFO_2
//		if (! StartDocPrinter(hPrinter, 2, (LPBYTE) &Info)) 
		nJobID = StartDocPrinter(hPrinter, 1, (LPBYTE) &Info);
//		return value is 31 bit "job number"
//		WritePrivateProfileInt(achDiag, "PrintJob", nJobID, achFile); // debugging
		if (nJobID == 0) {
			ShowLastError();
			ClosePrinter(hPrinter);
			hPrinter = NULL;
			GlobalUnlock(pd.hDevNames);
			goto printloss;
		}
		GlobalUnlock(pd.hDevNames);
	}
	else if (bOpenCloseChannel) {
#ifdef DEBUGPRINTING
		if (bDebug > 1) OutputDebugString("OPENCHANNEL\n");
#endif
		ret = Escape(hPrintDC, OPENCHANNEL, sizeof(WORD), NULL, NULL);
		if (ret <= 0) {
			winerror("OPENCHANNEL failed");
			bUseStartEndDoc = 1;
		}
	}

	if (bUseStartEndDoc) {
#ifdef DEBUGPRINTING
		if (bDebug > 1) OutputDebugString("StartDoc\n");
#endif

		err = StartDoc(hPrintDC, &DocInfo);	/* Windows 3.1:  StartDoc */

/*		If user cancels we get SP_ERROR here --- don't announce ? Fix 95/Sep/7 */
/*		#define SP_ERROR		     (-1) */

/*		if (err < 0) */					/* StartDoc > 0 if OK */
		if (err <= 0) {
			if (err == SP_APPABORT ||
				err == SP_USERABORT ||
				err == SP_ERROR) flag = 0;	/* OutputDebugString only */
			else flag = 1;					/* winerror */
			printerror(err, str, sizeof(str), "STARTDOC", flag);
			goto printloss;
		}
//		if (DocInfo.lpszOutput != NULL)
//			winerror(DocInfo.lpszOutput);	// DEBUGGING ONLY
//		else winerror("DocInfo.lpszOutput NULL");
	}

//	Try and undo PSCRIPT driver header mangling of coordinate system
	if (bUseDVIPSONE && bUseDLLs && bCallBack) {
		if (gPrCode == PASSTHROUGH &&  bOpenClosePrinter == 0)
			nukeheader(hPrintDC);
	}

/*	set copy count - do this before or after STARTDOC ? */
/*	NOTE: need to do this even if 1 to reset PSCRIPT.DRV - it remembers ! */

	nCopies = 1;			/* reset to default ? */

	if (bBusyFlag == 0) bBusyFlag++;	/* getting serious now */

/*	THIS IS WHERE WE ACTUALLY FINALLY GET TO DO SOME PRINTING ! */

/*	following is attempt to deal with request for collated printing */
//	if (bCollateFlag != 0 && nCopies > 1) kcollate = nCopies;
//	else kcollate = 1;

/*	FIRST DEAL WITH FONT SAMPLE CASE */

	if (bFontSample != 0) {				/* font sample or widths */
#ifdef DEBUGPRINTING
		if (bDebug > 1) OutputDebugString("StartPage\n");
#endif
		err = StartPage(hPrintDC);
		if (err < 0) {
			printerror(err, str, sizeof(str), "STARTPAGE", 1);
			return -1;					/* failed */
		}

#ifdef IGNORED
		if (bPrintFrame != 0) DoCropMarks(hPrintDC);
#endif
		if (bShowWidths != 0) ShowCharWidths(hWnd, hPrintDC);
		else ShowFontSample(hWnd, hPrintDC, -1);
#ifdef DEBUGPRINTING
		if (bDebug > 1) OutputDebugString("EndPage\n");
#endif
		err = EndPage(hPrintDC);

/*		NOTE: EndPage >= 0 if OK  versus Escape > 0 if OK */

/*		lets just ignore errors here for now - who really cares after all ? */
/*		this is when we print a font sample ... */
	}	/* end of bFontSample != 0 case */

/*	DEAL WITH PRINTING THE DVI FILE THAT IS OPEN */

	if (bFileValid != 0 && bFontSample == 0) {		/* print currently open file */
#ifdef DVIPSONEDLL
//		if (bUseDVIPSONEDLL && bCallBackPass) 
//		if (bUseDVIPSONE && bUseDVIPSONEDLL && bCallBackPass) {
		if (bUseDVIPSONE && bUseDLLs) {
			err = ConstructCommandLine(hWnd);
			if (err == 0) {
#ifdef IGNORED
				sprintf(debugstr, "RunDVIPSONEDLL command: `%s' callback %d (%s)",
						str, bCallBack, "DoPrintStuff");
				if (wincancel(debugstr))		// debugging only
					goto printloss;
#endif
//				winerror("DOPRINTSTUFF");		// debugging only
				if (bDebug || bShowCalls) {		// 2000 June 5
//					err = (HINSTANCE) 0;		/* avoid problems if exit here */
					flag = MaybeShowStr(str, "Application");	/* 95/Jan/8 */
//					if (flag == 0) return 1;	/* cancel => pretend success */
				}
//				(void) RunDVIPSONEDLL(hWnd, str, 1);
				if (RunDVIPSONEDLL(hWnd, str, bCallBack) == 0) {
//					successful completion
				}
				else {
// Now drop through to call dvipsone.exe ??? WinExe ??? no callback PS ???
					int nCmdShow;
					HINSTANCE err;
					if (nCmdShowForce >= 0) nCmdShow = nCmdShowForce;
					else nCmdShow = SW_SHOWMAXIMIZED;
					if (bRunMinimized != 0) nCmdShow = SW_SHOWMINIMIZED;
					err = TryWinExec(str, nCmdShow, "DVIPSONE"); // 2000 June 2
					if (err >= HINSTANCE_ERROR) {
						(void) GetTaskWindow(err);	/* success ! */
					}
					else {
						/* failed to call dvipsone.exe */
					}
				}
			}
		}
		else 
#endif
			(void) GDIPrint(hWnd, fullflag, bOldSpreadFlag);
	}	/* end of if bFileValid && bFontSample == 0 */

/* either drop through with success from above, or via goto printexit */
/* either drop through with success from above, or with failure ... */

//	printexit:				/* new position for this ??? */
	if (bPrintSuccess) {			/* 1992/Nov/3 */

//		try and undo stupid windows trailer effect of extra page...
		if (bUseDVIPSONE && bUseDLLs && bCallBack) {
			if ((gPrCode == PASSTHROUGH) && bOpenClosePrinter == 0)
				nuketrailer(hPrintDC);
		}

		if (bOpenClosePrinter) {
			if (! EndDocPrinter(hPrinter)) ShowLastError();
			if (! ClosePrinter(hPrinter)) ShowLastError();
			hPrinter = NULL;
		}
		else if (bOpenCloseChannel) {
#ifdef DEBUGPRINTING
			if (bDebug > 1) OutputDebugString("CLOSECHANNEL\n");
#endif
			ret = Escape(hPrintDC, CLOSECHANNEL, sizeof(WORD), NULL, NULL);
			if (ret <= 0) {
				winerror("CLOSECHANNEL failed");
				bUseStartEndDoc = 1;
			}
		}

		if (bUseStartEndDoc) {
#ifdef DEBUGPRINTING
			if (bDebug > 1) OutputDebugString("EndDoc\n");
#endif
			err = EndDoc(hPrintDC);

			if (err < 0) { 
				if (err == SP_APPABORT ||
					err == SP_USERABORT 
/*					err == SP_ERROR */				/* 1993/Aug/27 ??? */
				   ) flag = 0;
				else flag = 1;
				printerror(err, str, sizeof(str), "ENDDOC", flag);
			}
		}
	}

/* drop through from above or get here via goto from early error problem */

printloss:
/*	bPrintFlag = 0; */
/*	following already done in WM_COMMAND of DialogBox WindowProc if ABORTED */
	if (!bUserAbort) {
		(void) EnableWindow(hWnd, TRUE); 	/* reenable in case disabled */
/*		above must come before modeless dialog box is destroyed (4-113) */
		if (hDlgPrint != NULL) {
			(void) DestroyWindow(hDlgPrint);
/*			if (bDebug > 1) 
				OutputDebugString("Destroy hDlgPrint (printexit)\n"); */
			hDlgPrint = NULL;			/* debugging */
		}
	}
	if (bKeepPrinterDC == 0) {
#ifdef DEBUGPRINTING
		if (bDebug > 1) OutputDebugString("Delete Print DC\n");
#endif
		(void) DeleteDC(hPrintDC);		/* NO: reuse ??? */
		hPrintDC = NULL;
	}

/*	Now reset all of the things we changed for printing */
/*	bBusyFlag = 0; */
/*	bHourFlag = 0; */
/*	SetCursor(hArrow);	*/
	xoffset = oldxoffset;
	yoffset = oldyoffset;
	dvipage = olddvipage;
	bSpreadFlag = bOldSpreadFlag;
	if (bOldSpreadFlag != 0) usepagetable(dvipage, 0);
	bPrintFlag = 0; 
	bBusyFlag = 0;
	bUserAbort = 0;
	return 0;						/* success ? */
}	// end of DoPrintStuff --- at last!

/* Get Here on `Print Page' Menu Selection */ /* print just current page */

// int DoPrinting(HWND hWnd) {
//	return DoPrintStuff(hWnd, 0);
// }

int bOldSpreadFlag, bOldCountZero, bOldColorFont;
int olddvipage, oldwantedzoom, oldmarkfont;
long oldxoffset, oldyoffset;
	
void resetpagestate(HWND hWnd) {
	bOldSpreadFlag = bSpreadFlag;
	bOldCountZero = bCountZero;
	olddvipage = dvipage;
	oldwantedzoom = wantedzoom; 
	oldxoffset = xoffset; oldyoffset = yoffset;
/*	bOldColorFont = bColorFont; bColorFont = 0; */ /* ??? */
	oldmarkfont = markfont; markfont = -1;
	xoffset = 0; yoffset = 0; wantedzoom = 0; 
	resetmapping(hWnd, 0);			/* redundant ? */
	setscale(wantedzoom); 			/* redundant ? */
	newmapandfonts(hWnd); 			/* REALLY ??? 1991 Dec 27 */
}

void restorepagestate(HWND hWnd) {
	bSpreadFlag = bOldSpreadFlag;
	bCountZero = bOldCountZero;
	dvipage = olddvipage; usepagetable(dvipage, 0); 
	wantedzoom = oldwantedzoom; 
	xoffset = oldxoffset; yoffset = oldyoffset;	
/*	bColorFont = bOldColorFont; */	/* ??? */
	markfont = oldmarkfont;
	setscale(wantedzoom);
	newmapandfonts(hWnd); 			/* REALLY ??? 1991 Dec 27 */
}

/*	WinExec gives us task handle, what we want is handle of Window */
/*	EnumTaskWindows gives us the associated Windows handles */

/* BOOL CALLBACK _export EnumTaskWndProc(HWND hwnd, LPARAM lparam) {
	if (enumwindowscount++ == 0) hTaskWindow = hwnd;
	return TRUE;		
} */

/* void GetTaskWindow (HTASK hTask) {
	int k;
	for (k = 0; k < 32000; k++) {
		enumwindowscount = 0;
		(void) EnumTaskWindows(hTask, EnumTaskWndProc, (LPARAM) NULL);
		if (enumwindowscount != 0) break;
	}
	sprintf(str, "Task %X has %d windows. First window is %X (A)",
		hTask, enumwindowscount, hTaskWindow);
	winerror(str);
} */

BOOL CALLBACK _export EnumWndProc (HWND hWnd, LPARAM lParam) {
	HINSTANCE hInstance;
	LPENUMINFO lpInfo;

	lpInfo = (LPENUMINFO) lParam;

	hInstance = (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE);

/*	hInstance = (HINSTANCE) GetWindowWord(hWnd, GWW_HINSTANCE); */

	if (hInstance == lpInfo->hInstance) {
		lpInfo->hWnd = hWnd;
		return FALSE;					/* found it, so stop enumerating */
	}
	return TRUE;						/* no match, continue enumerating */
}

ENUMINFO EnumInfo;				/* to communicate with enum win proc */

HWND GetTaskWindow (HINSTANCE hInstance) {
/*	ENUMINFO EnumInfo; */		/* to communicate with enum win proc */

/*	Don't bother if hInstance is < MAXDOSERROR ??? */

	TaskInfo.hInstance = hInstance;			/* ??? */
	TaskInfo.hWnd = NULL;
	TaskInfo.hTask = NULL;

	EnumInfo.hInstance = TaskInfo.hInstance; 
	EnumInfo.hWnd = NULL;
	
	EnumWindows((WNDENUMPROC) EnumWndProc, (LPARAM) (LPENUMINFO) &EnumInfo);

	if (EnumInfo.hWnd == NULL) TaskInfo.hTask = NULL;	/* failed to find */
	else {
		TaskInfo.hWnd = EnumInfo.hWnd;
		TaskInfo.hTask = GetWindowTask(TaskInfo.hWnd);
	}

	return TaskInfo.hWnd;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Show command line for calling DOS or Windows program */

int MaybeShowStr (char *str, char *title) {		/* 95/Jan/8 */
	HWND hFocus;
	
	if (bShowCalls != 0) {
		if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
/*		(void) MessageBox(hFocus, str, title, MB_ICONASTERISK | MB_OK); */
		if (MessageBox(hFocus, str, title, MB_ICONASTERISK | MB_OKCANCEL)
			== IDCANCEL) return 0;				/* user cancelled */
	}
/*	else */
	if (bDebug > 1) OutputDebugString(str);
	return 1;									/* OK, continue */
}

void stripoutpath (char *str) {				/* 95/July/15 */
	char *s;
	int c;

	s = str;
	if (*str == '\"') {				/* deal with quoted material 97/Dec/1 */
		s = strchr(str+1, '\"');		/* find matching quote */
		if (s != NULL) s++;
		else s = str + strlen(str);	/* safety valve */
	}
	else {
		while (*s > ' ') s++;				/* find the end of executable */
	}
	while (s > str) {						/* reverse search for separator */
		c = *(s-1);
		if (c == '\\' || c == '/' || c == ':') {
			if (*str == '\"') strcpy(str+1, s);	/* keep quotes */
			else strcpy(str, s);			/* splice out the path */
			break;
		}
		s--;
	}
}

/*	Will add separator if path does not already end in one */
/*	Tries to deal with quoted filenames ... */

void spliceinpath(char *filename, char *path) { /* 95/July/15 */
	char *s; 
	int c, n, m;
	int flag=0;								/* if need separator */
	int quoted=0;							/* if "..." */

	if (*filename == '\"') quoted++;
	n = strlen(path);
	c = *(path + n - 1);
/*	if (c != '\\' && c != '/' && c != ':') flag = 1; */
	if (c != '\\' && c != '/') 	flag = 1;
	if (quoted) s = filename+1;				/* step over " */
	else s = filename;
	m = strlen(s);
	memmove (s + n + flag, s, m + 1);
	memcpy (s, path, n);
	if (flag) *(s + n) = '\\';
}

/* Separated out so it can be reused 1995/Jan/2 */
/* TryWinExec --- used for DVIPSONE and AFMtoTFM */

/* New order of doing things: */

/* (A) Tries calling executable in DVIWindo directory */
/* (B) Tries calling executable as is */
/* (C) If that fails, tries in c:\yandy\util */ /* %YANDYPATH%\util */
/* (D) If that fails, tries in PrePath Directory */
/* (E) If that fails, tries in VecPath Directory */
/* (F) If that fails, tries in parent of VecPath Directory if dir is `vec' */
/* (G) If that fails, tries in hard-wired DVIPSONE directory */

/* Now, as far as (A) & (B) are concerned: */
/* WinExec --- if filename does not contain a path --- searches: */
/* (i)   current dir, */
/* (ii)  Windows dir, */
/* (iii) Windows System dir, */
/* (iv)  dir of current task executable, */
/* (v)   dirs in PATH, */
/* (vi)  network */

/* But this *fails* if: */
/* (i) application is in dir of executable, after (ii) ChangeDirectory! */
/* In this case WinExec returns success, but then WINDOALP throws up a box: */
/* `Cannot find file' `Check to ensure the path and filename are correct,' */
/* with title `AFMTOTFM.EXE' and yellow exclamation icon! */

/* Note: for non-Windows app, nCmdShow is ignored, PIF file controls */

/* Note give up control when calling WinExec, may repaint, hence mess up str */

/* int TryWinExec(char *str, int nCmdShow, char *szProgram) */
HINSTANCE TryWinExec (char *str, int nCmdShow, char *szProgram) {
	char cmd[BUFLEN];	/* place for safe copy of str ! 95/Jun/24 */
	HINSTANCE err=0;
	int flag;
	int c, n;
	UINT OldFlag;
	char *s, *t;

	strcpy(cmd, str);	/* prevent repainting from screwing up command ! */
	OldFlag = SetErrorMode(SEM_NOOPENFILEERRORBOX);

/*	If fully qualified path, try as is and then stop trying ! 96/July/7 */
/*	First remove everything after path and name of program being called */
	if (*cmd == '\"') {		/* deal with quoted string "..." 97/Dec/1 */
		s = strchr(cmd+1, '\"');	/* find matching quote */
		if (s != NULL) s++;		/* point to space after quoted string */
	}
	else s = strchr(cmd, ' ');	/* space after program name */
	if (s == NULL) s = cmd + strlen(cmd);	/* safety valve! */
	c = *s;					/* save old character */
	*s = '\0';				/* truncate for the moment after program name */
	t = removepath(cmd);	/* pointer to filename minus path if given */
/*	*s = ' '; */			/* restore space */
	*s = (char) c;			/* restore space */

	if (t > cmd) {			/* if fully qualified try (only) as is */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow); 
		goto analerr;		/* don't try anything else if full path given */
	}
/*	Make special case of DVIPSONE and AFMtoTFM (97 Dec 1) */
	if (_stricmp(szProgram, "DVIPSONE") == 0 ||
		_stricmp(szProgram, "AFMtoTFM") == 0) {
		s = grabenv("YANDYPATH");
		if (s == NULL) s = "c:\\yandy";		// default 99/Aug/3
		if (s != NULL) {
			stripoutpath(cmd);		/* should not be needed if we get here */
			spliceinpath(cmd, s);
			if (_stricmp(szProgram, "DVIPSONE") == 0)
				spliceinpath(cmd+strlen(s)+1, "dvipsone");
			else if (_stricmp(szProgram, "AFMtoTFM") == 0)
				spliceinpath(cmd+strlen(s)+1, "util");				
			if (bDebug || bShowCalls) {
				flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
				if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
			}
			err = (HINSTANCE) WinExec(cmd, nCmdShow); 
		}
	}
/*	Try directory of DVIWINDO.EXE (szExeWindo see dviwindo.c) */
/*	Theoretically this should be redundant since Windows should look here */
/*	But need to do this because of bug in Windows described above */
	if (err < HINSTANCE_ERROR) {				  /* 95/July/12 */
		stripoutpath(cmd);
		spliceinpath(cmd, szExeWindo);		/* Try in DVIWindo dir */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow); 
	}
/*	Try as is, no path - let Windows do the search */
	if (err < HINSTANCE_ERROR) {
		stripoutpath(cmd);
/*		no need for spliceinpath(cmd, ""); */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow);
	}
	if (err < HINSTANCE_ERROR &&
/*			strcmp(szBasePath, "") != 0) */	
			szBasePath != NULL) {
		stripoutpath(cmd);
		spliceinpath(cmd, "util");		/* Next try in utilities */
		spliceinpath(cmd, szBasePath);	/* c:\\yandy\\util\\... */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow);
	}
	if (err < HINSTANCE_ERROR &&
/*			strcmp(szBasePath, "") != 0) */	/* added 1997/Mar/9 */
			szBasePath != NULL) {
		stripoutpath(cmd);
		spliceinpath(cmd, "dvipsone");	/* Next try in dvipsone */
		spliceinpath(cmd, szBasePath);	/* c:\\yandy\\dvipsone\\... */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow);
	}
	if (err < HINSTANCE_ERROR &&
/*			strcmp(szPrePath, "") != 0) */		/* added 1995/May/8 */
			szPrePath != NULL) {
		stripoutpath(cmd);
		spliceinpath(cmd, szPrePath); /*	Next try in PREPATH dir */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow);
	}

	if (err < HINSTANCE_ERROR &&
/*			strcmp(szVecPath, "") != 0) */
			szVecPath != NULL) {
		stripoutpath(cmd);
		spliceinpath(cmd, szVecPath);	/*	Next try in VECPATH dir */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow);
	}

	if (err < HINSTANCE_ERROR &&
/*			strcmp(szVecPath, "") != 0) */		/* 1996/Jan/28 */
			szVecPath != NULL) {
		s = szVecPath + strlen(szVecPath);	/* see if ends in \vec */
		if (s-4 > szVecPath && _strnicmp(s-4, "\\vec", 4) == 0) {
			stripoutpath(cmd);
			spliceinpath(cmd, szVecPath);	/*	Parent dir of VECPATH */
			n = strlen(szVecPath);
			strcpy(cmd+n-4, cmd+n);			/* splice out the \vec */
			if (bDebug || bShowCalls) {
				flag = MaybeShowStr (cmd, szProgram);	/* 96/Jan/28 */
				if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
			}
			err = (HINSTANCE) WinExec(cmd, nCmdShow);
		}
	}

	if (err < HINSTANCE_ERROR) {						/* 1994/Feb/25 */
/*  Failed find on path, splice in the default path for dvipsone 94/Jan/2 */
		stripoutpath(cmd);
//		spliceinpath(cmd, "c:\\dvipsone\\"); /* try in default DVIPSONE dir */
		spliceinpath(cmd, "c:\\yandy\\dvipsone\\"); /* try in default DVIPSONE dir */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow);
	}

/*	if (err < HINSTANCE_ERROR) */
	if (err < HINSTANCE_ERROR && nDriveD == DRIVE_FIXED) {	/* 95/Sep/12 */
		*cmd = 'd';					/* also try d:  but only if fixed drive */
/*	Next try in non-standard hard-wired DVIPSONE dir */
		if (bDebug || bShowCalls) {
			flag = MaybeShowStr (cmd, szProgram);	/* 95/Jan/8 */
			if (flag == 0) return (HINSTANCE) 0;	/* user cancelled */
		}
		err = (HINSTANCE) WinExec(cmd, nCmdShow);
	}

analerr:

	(void) SetErrorMode(OldFlag);   /* restore previous mode */

	if (err >= HINSTANCE_ERROR) {
		if (bDebug > 1) OutputDebugString("WinExec succeeded\n");
		return err;					/* success ! */
	}

//	EXE_ERROR strings no longer exist...

/*	Everything we tried failed! So create error message */
/*	if (bDebug) MaybeShowStr (str, szProgram);	*/	/* 95/Jan/8 */
/*  failed to find it anywhere, use error names in string table */
	strcpy(str, szProgram);
	strcat(str, " ");								/* 95/Jan/15 */
/*	n = strlen(str); */
	s = str + strlen(str);
/*	flag = LoadString(hInst, (UINT) (EXE_ERROR + err), */
/*	UINT is 16 bit in WIN16 and 32 bit in WIN32 */
	flag = LoadString(hInst, (UINT) (EXE_ERROR + (UINT) err), 
		s, sizeof(str) - strlen(str)); 
/*	if (flag == 0) sprintf(str+n, "Error %d ", err); */
	if (flag == 0) sprintf(s, "Error %d ", err);
	strcat(str, " --- WinExec");
	winerror(str);
	return err;
}

/* This checks whether DVIPSONE.EXE can be found */
/* reuses buffer space passed down for string manipulations */
/* reuses OFSTRUCT passed down for file existence tests */

/* int CheckDVIPSONE (char *buffer, OFSTRUCT OfStruct) */
int CheckDVIPSONE (char *buffer) { 
#ifndef LONGNAMES
	OFSTRUCT OfStruct;					/* for existence test only */
#endif
	HFILE perr=HFILE_ERROR;				/* 95/July/15 */
	char *s;
	int n;

/*	Ideally want DVIPSONE.PIF around to control windowing and such... */
/*  WinExec: `For a non-Windows application, the PIF, if any, */
/*  determines the windows state' */

/*  Above rewritten 94/Jan/2 since dvipsone.pif alone doesn't work */
/*  This also tries to find DVIPSONE in the standard place */
/*	Hence avoiding the need to place DVIPSONE directory on the path */

/*	Does this match the actual TryWinExec path searching ? 1995/July/15 */
/*	Does OpenFile follow same rules as WinExec in search ? YES */
/*	(i) Current Dir, (ii) Windows Dir, (iii) Windows System Dir */
/*	(iv) directory of current executable, (v) PATH Dirs, (vi) network */
/*	Following tries (A) OpenFile with unqualified name */
/*	(B) PREPATH, (C) VECPATH, (D) parent of VECPATH */
/*	(E) c:\yandy\dvipsone, (F) c:\dvipsone, (G) d:\dvipsone */
/*	maybe explicitly add in test for szExeWindo also ? */

	bDVIPSONEexists = 1;
/*	if user specified command for it, try that 95/Sep/12 */
/*	if (strcmp(szDVIPSONEcom, "") != 0) */
	if (szDVIPSONEcom != NULL) {
		strcpy(buffer, szDVIPSONEcom);
		s = buffer;
		while (*s > ' ') s++;
		*s = '\0';				/* snip off at first white space 96/Jan/28 */
/*	Check on existence of dvipsone.exe - presumably can use OpenFile here */
#ifdef LONGNAMES
		perr = LongOpenFile(buffer, OF_EXIST);
#else
		perr = OpenFile(buffer, &OfStruct, OF_EXIST);
#endif
		if (perr == HFILE_ERROR) {
			bDVIPSONEexists=0;
			bUseDVIPSONE=0;
		}			
		return bDVIPSONEexists;					/* don't do any other test */
	}

	strcpy(buffer, "dvipsone.exe");
	if (perr == HFILE_ERROR) {					/* try standard Windows search */
		stripoutpath(buffer);
#ifdef DEBUGDVIPSONE
		if (bDebug > 1) {
			sprintf(debugstr, "DVIPrint: %s\n", buffer);
			OutputDebugString(debugstr);
		}   /* debugging only */
#endif
#ifdef LONGNAMES
		perr = LongOpenFile(buffer, OF_EXIST);	
#else
		perr = OpenFile(buffer, &OfStruct, OF_EXIST);	
#endif
	}
	if (perr == HFILE_ERROR &&
/*			strcmp(PrePath, "") != 0) */		/* try in PREPATH directory */
			szPrePath != NULL) {				/* try in PREPATH directory */
		stripoutpath(buffer);
		spliceinpath(buffer, szPrePath);
#ifdef DEBUGDVIPSONE
		if (bDebug > 1) {	
			sprintf(debugstr, "DVIPrint: %s\n", buffer);
			OutputDebugString(debugstr);
		}   /* debugging only */
#endif
#ifdef LONGNAMES
		perr = LongOpenFile(buffer, OF_EXIST);
#else
		perr = OpenFile(buffer, &OfStruct, OF_EXIST);
#endif
	}

/*	Try in new directory structure if YANDYPATH is defined */
	if (perr == HFILE_ERROR &&
/*			strcmp (szBasePath, "") != 0) */					/* 96/Aug/29 */
			szBasePath != NULL) {
		stripoutpath(buffer);
		spliceinpath(buffer, "dvipsone");
		spliceinpath(buffer, szBasePath);	/* c:\\yandy\\dvipsone\\... */
#ifdef DEBUGDVIPSONE
		if (bDebug > 1) {	
			sprintf(debugstr, "DVIPrint: %s\n", buffer);
			OutputDebugString(debugstr);
		}  /* debugging only */
#endif
#ifdef LONGNAMES
		perr = LongOpenFile(buffer, OF_EXIST);
#else
		perr = OpenFile(buffer, &OfStruct, OF_EXIST);
#endif
	}

	if (perr == HFILE_ERROR &&
/*			strcmp(szVecPath, "") != 0) */	/* try in VECPATH directory */
			szVecPath != NULL) {	
		stripoutpath(buffer);
		spliceinpath(buffer, szVecPath);
#ifdef DEBUGDVIPSONE
		if (bDebug > 1) {	
			sprintf(debugstr, "DVIPrint: %s\n", buffer);
			OutputDebugString(debugstr);
		}  /* debugging only */
#endif
#ifdef LONGNAMES
		perr = LongOpenFile(buffer, OF_EXIST);
#else
		perr = OpenFile(buffer, &OfStruct, OF_EXIST);
#endif
	}
	if (perr == HFILE_ERROR &&
/*			strcmp(szVecPath, "") != 0) */	/* try in parent of VECPATH directory */
			szVecPath != NULL) {
		s = szVecPath + strlen(szVecPath);	/* see if ends in \vec */
		if (s-4 > szVecPath && _strnicmp(s-4, "\\vec", 4) == 0) {
			stripoutpath(buffer);
			spliceinpath(buffer, szVecPath);
			n = strlen(szVecPath);
			strcpy(buffer+n-4, buffer+n);		/* splice out the \vec */
#ifdef DEBUGDVIPSONE
			if (bDebug > 1) {	
				sprintf(debugstr, "DVIPrint: %s\n", buffer);
				OutputDebugString(debugstr);
			}  /* debugging only */
#endif
#ifdef LONGNAMES
			perr = LongOpenFile(buffer, OF_EXIST);
#else
			perr = OpenFile(buffer, &OfStruct, OF_EXIST);
#endif
		}
	}
	if (perr == HFILE_ERROR) {					/* 1994/Feb/25 */
		stripoutpath(buffer);
//		spliceinpath(buffer, "c:\\dvipsone\\");
		spliceinpath(buffer, "c:\\yandy\\dvipsone\\");
#ifdef DEBUGDVIPSONE
		if (bDebug > 1) {	
			sprintf(debugstr, "DVIPrint: %s\n", buffer);
			OutputDebugString(debugstr);
		}  /* debugging only */
#endif
#ifdef LONGNAMES
		perr = LongOpenFile(buffer, OF_EXIST);
#else
		perr = OpenFile(buffer, &OfStruct, OF_EXIST);
#endif
	}
/*	if (perr == HFILE_ERROR) */
	if (perr == HFILE_ERROR && nDriveD == DRIVE_FIXED) { /* 95/Sep/12 */
		*buffer = 'd';				/* also try d: but only if fixed drive */
#ifdef DEBUGDVIPSONE
		if (bDebug > 1) {	
			sprintf(debugstr, "DVIPrint: %s\n", buffer);
			OutputDebugString(debugstr);
		}   /* debugging only */
#endif
#ifdef LONGNAMES
		perr = LongOpenFile(buffer, OF_EXIST);
#else
		perr = OpenFile(buffer, &OfStruct, OF_EXIST);
#endif
	}

	if (perr == HFILE_ERROR) { 
		bDVIPSONEexists=0;		/* ugh! could not find it */
		bUseDVIPSONE=0;			/* 1994/June/20 safety experiment */
		if (bDebug) {
			if (bDebug > 1) OutputDebugString("DVIPSONE not found\n");
			else winerror("DVIPSONE not found\n");
		}
	}
	return bDVIPSONEexists;
}

/* ******************************* NEWPRINTDLG / WIN32 */

UINT APIENTRY PrintDlgHook (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	WORD id, cmd;
	int count, n;
//	char szPrinter[64];	/* #define CCHDEVICENAME 32? how long can it be ? */
	char szPrinter[FILENAME_MAX];	/* 99/July/30 */
	char *s;
/*	LPDEVMODE lpDevMode; */

	switch (msg) {
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);			
			cmd = (WORD) GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id) {
				case IDOK:
/*					read out data from dialog box for fields we put in ... */
					bReversePages = (int) SendDlgItemMessage(hDlg, 
						IPR_REVERSE, BM_GETCHECK, 0, 0L);
					bOddOnly = (int) SendDlgItemMessage(hDlg, 
						IPR_ODDONLY, BM_GETCHECK, 0, 0L);
					bEvenOnly = (int) SendDlgItemMessage(hDlg, 
						IPR_EVENONLY, BM_GETCHECK, 0, 0L);
					bUseDVIPSONE = (int) SendDlgItemMessage(hDlg, 
						IPR_USEPSONE, BM_GETCHECK, 0, 0L);
					break;

				case IPR_ODDONLY:
					(void) SendDlgItemMessage(hDlg, IPR_EVENONLY, BM_SETCHECK, 
							(WPARAM) 0, 0L);
					break;

				case IPR_EVENONLY:
					(void) SendDlgItemMessage(hDlg, IPR_ODDONLY, BM_SETCHECK, 
							(WPARAM) 0, 0L);
					break;

				case cmb4:
//	look at wParam do decide whether this is good time
//	cmd 3 gain focus, 4 kill focus, 9 is lose selection, 1 is gain selection 
//	when starting up we get a 3, when selecting a new one we get a 1 
					if (cmd != 3 && cmd != 1) break;
//	Figure out whether to suggest use of DVIPSONE or not 
					if (bDVIPSONEexists == 0) break;
					count = GetDlgItemText(hDlg, cmb4, szPrinter, sizeof(szPrinter));
					if (count <= 0) break;
// could read out stc14 field (contains PORT) instead ? 
					count = GetProfileString("devices", szPrinter, "", str, sizeof(str));
					if (count <= 0) break;
// szPrinter contains things like "HP LaserJet 4M Plus" 
					if (bDebug > 1) {
						sprintf(debugstr, "szPrinter `%s' devices `%s'", 
								szPrinter, str);
						OutputDebugString(debugstr);
					}
// str contains things like   "winspool,LPT1:"  
					if ((s = strchr(str, ',')) != NULL) *s++ = '\0';
					else s = str;
// explicit choice in dviwindo.ini overrides all
					n = setuseDVIPSONEsub(s);
					if (n >= 0) bUseDVIPSONE = n;
					else {
// new code to check on PS capabilities of selected printer 99/July/31
						int gPrCode=POSTSCRIPT_PASSTHROUGH;
						HDC hIC = CreateIC(
										   str,		// driver
										   szPrinter,	// device/model
										   s,			// port, IGNORED
										   NULL);		// DEVMODE optional
						if (hIC != NULL) {
							if (Escape(
									   hIC,
									   QUERYESCSUPPORT,
									   sizeof(int),
									   (LPSTR) &gPrCode, NULL))
								bUseDVIPSONE = 1;
							else bUseDVIPSONE = 0;
							DeleteDC(hIC);
						}
					}
					(void) SendDlgItemMessage(hDlg, IPR_USEPSONE,
								  BM_SETCHECK, (WPARAM) bUseDVIPSONE, 0L); 
					break;

				default: return (FALSE);
			}
		break;									/* end of WM_COMMAND case */
			
		case WM_INITDIALOG:						/* message: initialize	*/
			(void) SendDlgItemMessage(hDlg, IPR_REVERSE, BM_SETCHECK, 
				(WPARAM) bReversePages, 0L);
			(void) SendDlgItemMessage(hDlg, IPR_ODDONLY, BM_SETCHECK, 
				(WPARAM) bOddOnly, 0L);
			(void) SendDlgItemMessage(hDlg, IPR_EVENONLY, BM_SETCHECK, 
				(WPARAM) bEvenOnly, 0L);
/* Show user what ports DVIPSONE is suggested on */
			if (bDVIPSONEexists) {
/*				if (strcmp(szDVIPSONEport, "") != 0) */
				if (szDVIPSONEport != NULL) {
					strcpy(str, "Use DVIPSONE on: ");
					strcat(str, szDVIPSONEport);
					SetDlgItemText(hDlg, stc13, str);
					EnableWindow(GetDlgItem(hDlg, stc13), TRUE);
				}
/* The above `comment' field disappears when we do something in combo box .. */
/* Read out `Where' field and see what Port it is set to */
/*				count = GetDlgItemText(hDlg, stc14, str, sizeof(str));
				if (count > 0) {
					n = setuseDVIPSONE(str);
					if (n >= 0) bUseDVIPSONE = n;
				} */	/* rather keep previous setting ? */
/* Check Use DVIPSONE box if selected printer allows this ? */
				(void) SendDlgItemMessage(hDlg, IPR_USEPSONE, BM_SETCHECK, 
										  (WPARAM) bUseDVIPSONE, 0L); 
			}
			else {
				bUseDVIPSONE = 0;
/*	Turn off DVIPSONE selection if DVIPSONE does not exist */
				(void) SendDlgItemMessage(hDlg, IPR_USEPSONE, BM_SETCHECK, 0, 0L);
/*	Disable Use DVIPSONE if DVIPSONE does not exist */
				EnableWindow(GetDlgItem(hDlg, IPR_USEPSONE), FALSE);
			}
/*	Make is explicit that this INITDIALOG returns FALSE 96/Aug/14 */
/*			return (FALSE); */ /* Indicates we set the focus to a control */
/*	Isn't this wrong? Shouldn't this return TRUE - fixed 96/Aug/14 */
			return (TRUE);	/* Indicates the focus is *not* set to a control */
		break;

		default: return (FALSE);

	}
	return (FALSE); 	/* let Dialog Box Proc process message */
/*	return (TRUE);	*/	/* processed message - don't process again */
}

/* returns 0 if failed (user cancelled or error occurs) */
/* returns 1 on success */

int NewPrintDlg (HINSTANCE hInst) {		// hard wired in WIN32
/*	DWORD err; */
/*	LPDEVMODE lpDevMode; */

	if (pd.hDevNames == NULL) (void) GetDefPrinter();	/* NEW */
	FillDevModeFields();					/* Orientation & Page Size */
/*	set up fields in pd */
/*	memset (&pd, 0, sizeof(PRINTDLG)); */
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = hwnd;						/* ??? */
	pd.hDC = NULL;
	pd.nMinPage = 1;
	pd.nMaxPage = (WORD) dvi_t;
/*	nFromPage, nToPage, nMinPage, nMaxPage are all WORD ... */
/*	crude attempt to avoid disaster in count[0] mode 96/Aug/18 */
/*	if (frompage > 0) pd.nFromPage = frompage; */
	if (frompage > 0 && frompage <= dvi_t)
		pd.nFromPage = (WORD) frompage;	/* sanity check added 97/Apr/7 */
	else pd.nFromPage = 1;
/*	if (topage > 0 && topage >= frompage) pd.nToPage = topage; */
	if (topage > 0 && topage <= dvi_t && topage >= frompage)
		pd.nToPage = (WORD) topage;		/* sanity check added 97/Apr/7 */
	else pd.nToPage = (WORD) dvi_t;
/*	Maybe do the above only if bCountZero == 0 ??? */
/*	Or set nMaxPage to max counter[0] ??? */
#ifdef DEBUGPRINTING
	if (bDebug > 1) {
		sprintf(debugstr, "min %d max %d from %d to %d\n",
				pd.nMinPage, pd.nMaxPage, pd.nFromPage, pd.nToPage);
		OutputDebugString(debugstr);
	}
#endif
/*	Note: if the above are set to -1 the fields are blank in dialog */
	pd.Flags = 0;							/* Clean slate for flags */
	if (bUseDevModeCopies) pd.Flags |= PD_USEDEVMODECOPIES;
/*	if we set PD_USEDEVMODECOPIES: */
/*		Copies = DEVMODE dmCopies 	Collate = DEVMODE dmCollate */
/*		otherwise we use nCopies and PD_COLLATE */
	if (bUseDevModeCopies == 0) {
		pd.nCopies = (WORD) nCopies;			/* if hDevMode = NULL */
		if (bCollateFlag) pd.Flags |= PD_COLLATE;
	}
	else pd.nCopies = 1;
	pd.hInstance = hInst;					/* if we use our own templates */
/*	lCustData lpfnPrintHook lpfnSetupHook */
/*	lpPrintTemplateName lpSetupTemplateName */
	pd.lpPrintTemplateName = MAKEINTRESOURCE(PRINTDLGORD); 
	pd.Flags |= PD_ENABLEPRINTTEMPLATE; 	/* if we use own template */
	pd.lpfnPrintHook = (LPOFNHOOKPROC) PrintDlgHook;	/* 95/Dec/26 */
	pd.Flags |= PD_ENABLEPRINTHOOK;			/* if we use our own hook proc */
	pd.Flags |= PD_NOSELECTION; 			/* Disables Selection button */
/*	pd.Flags |= PD_USEDEVMODECOPIES; */		/* Driver copies and collates */
/*  if we do not do this we need to be ready to make copies / collate */
	if (frompage != 1 || topage != dvi_t) pd.Flags |= PD_PAGENUMS;
/*	else pd.Flags |= PD_ALLPAGES; */		/* that is, zero ... */
	if (bPrintToFile) pd.Flags |= PD_PRINTTOFILE;
#ifdef DEBUGPRINTING
	if (bDebug > 1) {
		sprintf(debugstr, "BEFORE PrintDlg %s\n", bPrintToFile ? "to file" : "");
		OutputDebugString(debugstr);
	}
#endif
/*	PD_COLLATE PD_NOWARNING PD_PRINTSETUP */
/*	PD_DISABLEPRINTTOFILE PD_HIDEPRINTTOFILE */
/*  PD_ENABLEPRINTHOOK PD_ENABLEPRINTTEMPLATE */
/*  PD_ENABLESETUPHOOK PD_ENABLESETUPTEMPLATE */
/*	PD_NOPAGENUMS PD_NOSELECTION */
/*	PD_PAGENUMS PD_SELECTION PD_ALLPAGES */
/*	PD_RETURNDC PD_RETURNIC PD_RETURNDEFAULT */
/*	PD_SHOWHELP PD_USEDEVMODECOPIES */
/*	Now for the big event !!! */
	if (PrintDlg(&pd)) {
/*		extract fields from pd */
		if (pd.hDevMode == NULL) {
			if (bDebug > 1) OutputDebugString("Pre Windows 3.0 Driver\n");
		}
		if (pd.Flags & PD_PAGENUMS) {
			frompage = pd.nFromPage;
/*			if (frompage > dvi_t) frompage = dvi_t; */
			topage = pd.nToPage;
/*			if (topage > dvi_t) topage = dvi_t; */
		}
		else if (pd.Flags & PD_SELECTION) {
			frompage = 1; topage = dvi_t;		/* can't happen */
		}
		else {									/* PD_ALLPAGES = 0 */
			frompage = 1; topage = dvi_t;
		}
		if (pd.Flags & PD_PRINTTOFILE) {
/*			DEVNAMES  + wOuputOffset points to the string FILE: */
			bPrintToFile = 1;
		}
		else bPrintToFile = 0;
#ifdef DEBUGPRINTING
		if (bDebug > 1) {
			sprintf(debugstr, "AFTER PrintDlg %s\n",
					bPrintToFile ? "to file" : "");
			OutputDebugString(debugstr);
		}
#endif
/*		if we set PD_USEDEVMODECOPIES: */ 
/*		Copies = DEVMODE dmCopies 	Collate = DEVMODE dmCollate */
/*		otherwise we use nCopies and PD_COLLATE */
		if (bUseDevModeCopies == 0) {
			if (pd.Flags & PD_COLLATE) {
				bCollateFlag = 1;					/* overwritten below */
			}
			else bCollateFlag = 0;
			nCopies = pd.nCopies;					/* overwritten below */
		}
		ReadDevModeFields();
		checkpaper(hwnd, papertype);			/* paper type check mark */
		checkduplex(hwnd, nDuplex);			/* duplex check mark */
		checkpreferences(hwnd);				/* landscape check mark */
/*		should also repaint if landscape / portrait or papertype changed ... */
/* if we set PD_USEDEVMODECOPIES: */
/*		Copies = DEVMODE dmCopies 	Collate = DEVMODE dmCollate */
/*		otherwise we can just use nCopies and PD_COLLATE */
/*		if (pd.hDevMode != NULL) {
			lpDevMode = (LPDEVMODE) GlobalLock(pd.hDevMode); 
			if (lpDevMode->dmCollate != 0) 
				bCollateFlag = lpDevMode->dmCollate;
			if (lpDevMode->dmCopies > 1)
				nCopies = lpDevMode->dmCopies;
			GlobalUnlock(pd.hDevMode);
		} */
		return TRUE;							/* success */
	}
	else {
		ShowCommDlgError("NewPrintDlg");
		return FALSE;					/* failure */
	}
}

/*************************************************************************/

/* quote a string (presumably because it has white space) 98/Jan/10 */

char *quotestring (char *str) {
	char *s=str;
	int d, c = '\"';

	if (*s == '\"') return str;		/* already quoted ? */
	while (c != '\0') {
		d = *s;
		*s++ = (char) c;
		c = d;
	}
	*s++='\"';
	*s = '\0';
/*	winerror(str); */	/* debugging */
	return str;
}

/* Following is to pick up port for passing to DVIPSONE */
/* puts port (or sharename) in argument */

void copyportname(char *buffer) {				/* 1996/Jan/7 */
	LPDEVNAMES lpDevNames;
	int n;

	if (pd.hDevNames != NULL) {		/* SHOULD be non NULL now! */
		lpDevNames = (LPDEVNAMES) GlobalLock(pd.hDevNames);
/*		record last printer selected for potential debugging later */
		sprintf(buffer, "%s,%s,%s",
			(LPSTR) lpDevNames + lpDevNames->wDeviceOffset,
			(LPSTR) lpDevNames + lpDevNames->wDriverOffset,
			(LPSTR) lpDevNames + lpDevNames->wOutputOffset); 
//		WritePrivateProfileString(achPr, "LastPrinter", buffer, achFile);
		WritePrivateProfileString(achDiag, "LastPrinter", buffer, achFile);
		if (bPrintToFile) strcpy(buffer, "FILE:");
/*		Pick up `Output Port' to ship to from DEVNAMES structure */
		else strcpy(buffer, (LPSTR) lpDevNames + lpDevNames->wOutputOffset);
/*		See whether user has remapped this port to something else */
		n = GetPrivateProfileString(achPr, buffer, "",
									str, sizeof(str), achFile);
		if (n > 0) strcpy(buffer, str);
/*		else check is one of those bogus Ne00: Ne01: ports ? Use UNC instead */
		else if (bUseSharedName && (sscanf(buffer, "Ne%d:", &n) == 1)) {
/*			Then use `shared name' instead - found in `Device' field */
			strcpy(buffer,  (LPSTR) lpDevNames + lpDevNames->wDeviceOffset);
/*			Don't try and do this if it is not a `shared name' \\foo\bar UNC */
			if (strncmp(buffer, "\\\\", 2) != 0)
				strcpy(buffer, (LPSTR) lpDevNames + lpDevNames->wOutputOffset);
/*			driver should be `winspool' */
/*			in (LPSTR) (lpDevNames + lpDevNames->wDriverOffset) */
		}
		GlobalUnlock(pd.hDevNames);
	}
	else strcpy(buffer, "FILE:");	/* should NOT happen */
/*	port name with space already taken care of later ? */
/*	if (strchr(buffer, ' ') != NULL) quotestring(buffer); */
#ifdef DEBUGPRINTING
	if (bDebug > 1) OutputDebugString(buffer);
#endif
}

#ifdef IGNORED

/* Deal with shared name UNC in old version (not using new print dialog) */

void copyportnameold(char *buffer, char *achPort, char *achDriver, char *achDevice) {
	int n;
/*	record last printer selected for potential debugging later */
	sprintf(buffer, "%s,%s,%s", achDevice, achDriver, achPort);
//	WritePrivateProfileString(achPr, "LastPrinter", buffer, achFile);
	WritePrivateProfileString(achDiag, "LastPrinter", buffer, achFile);
	strcpy (buffer, achPort);
/*	See whether user has remapped this port to something else */
	n = GetPrivateProfileString(achPr, buffer, "",
								str, sizeof(str), achFile);
	if (n > 0) strcpy(buffer, str);
/*	else check is one of those bogus Ne00: Ne01: ports ? Use UNC instead */
	else if (bUseSharedName && (sscanf(buffer, "Ne%d:", &n) == 1)) {
		strcpy(buffer, achDevice);
		if (strncmp(buffer, "\\\\", 2) != 0)
			strcpy(buffer, achPort);
	}
}

#endif

////////////////////////////////////////////////////////////////////////////

HINSTANCE hDVIPSONE=NULL;

// This gets called with passthrough PS from dvipsone.dll

// int PScallback (const char *szPS) {
// int PScallback (char *szPS) {
int PScallback (char *szPS, int nlen) {
//	int nlen = strlen(szPS);
	int npage;
	char buffer[64];
	
	if (bUserAbort) return -1;				// user wants to abort this printing
//	Pick up DSC %%Page: comments and use to update dialog
	if (strncmp(szPS, "%%Page:", 7) == 0) {
		if (sscanf(szPS+7, "%d", &npage) == 1) {
			sprintf(buffer, "Sending page %d", npage);
			if (hDlgPrint != NULL)
				SetDlgItemText(hDlgPrint, IDC_SOURCE, buffer); 
		}
	}
	if (passthrough(hPrintDC, szPS, nlen) != 0) 
		return -1;								// PASSTHROUGH Escape failed
	return 0;
}

//	call dvipsone.dll
//	args: window handle, command line string, callbackflag

int RunDVIPSONEDLL (HWND hWnd, char *str, int usecallbackflag) {
//	following must match what is in DVIPSONE.C
//	int dvipsone (HWND hConsole, char *line, int (* PScall) (const char *)) 
//	last argument is a call-backup that can send PS strings back
//	returns zero if OK, returns -1 if it fails
	int (* lpDVIPSONE) (HWND, char *, int (*) (char *, int))=NULL;
	int ret, fault=0;

#ifdef DEBUGPRINTING
	if (bDebug > 1)	OutputDebugString("RunDVIPSONEDLL");
#endif

	if (hDVIPSONE == NULL) 
		hDVIPSONE = LoadLibrary("DVIPSONE");	/* connect to DVIPSONE.DLL */
	if (hDVIPSONE == NULL){
		WriteError("Can't link to DVIPSONE.DLL");
		return -1;			// error return
	}
	
	lpDVIPSONE = (int (*) (HWND, char *, int (*) (char *, int)))
					 GetProcAddress(hDVIPSONE, "dvipsone");
	if (lpDVIPSONE == NULL)	{
		winerror("Can't find DVIPSONE.DLL entry point");
		fault++;
	}
	else {
		if (hConsoleWnd == NULL)		// create console window if needed 
			CreateConsole(hInst, hWnd);	// for DVIPSONE.DLL
		if (hConsoleWnd == NULL) {
			ShowLastError();			// no console window created
			fault++;
		}
		else {
			(void) EnableWindow(hWnd, FALSE); 	/* disable main Window ??? */
/*			call DVIPSONE with command line string */
			ret = (* lpDVIPSONE) (hConsoleWnd, str, usecallbackflag ? PScallback : NULL);
			(void) EnableWindow(hWnd, TRUE); 	/* reenable main Window ??? */
		}
/*		can free library now since dviwindo takes care of modeless dialog */
	}
//	can free library since dviwindo takes care of modeless dialog 
//	or can leave it around for cleanup at the end
	FreeLibrary(hDVIPSONE); 
	hDVIPSONE = NULL;
//	return ret;			// ???
	return fault;
}

/***************************************************************************/

void ForceExtension (char *fname, char *str) {	// change extension if present 
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, ".");
		strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}


// constructs command line for DVIPSONE.EXE or DVIPSONE.DLL in str
// called from DoPrintStuff and from DoPrintAll
// which means it may get called twice, since DoPrintAll calls DoPrintStuff

// We assume bUsingDistiller has been set above in DoPrintStuff

int ConstructCommandLine (HWND hWnd) {
	char buffer[MAXFILENAME]; 	// temp port name to strip colon etc.
	char *s, *szDVI;
	int flag, firstpage;
	int quoteflag=0;			// may be redundant 
	
/*	copy port name into command string for DVIPSONE */
/* #ifdef USEPRINTDLG
	if (pd.hDevNames == NULL) (void) GetDefPrinter();
	if (pd.hDevNames != NULL) {
		lpDevNames = (LPDEVNAMES) GlobalLock(pd.hDevNames);
		if (bPrintToFile) strcpy(buffer, "FILE:");
		else strcpy(buffer,
					 (LPSTR) lpDevNames + lpDevNames->wOutputOffset);
		GlobalUnlock(pd.hDevNames);
	}
	else strcpy(buffer, "FILE:");
#else
	if (*achPort == '\0') (void) GetDefPrinter();
	strcpy(buffer, achPort);
#endif */

	if (pd.hDevNames == NULL) (void) GetDefPrinter();	/* NEW */
	copyportname(buffer);

	if (strchr(DefPath, ' ') != NULL ||	strchr(OpenName, ' ') != NULL)
		quoteflag = 1;		//	alternative: always quote filenames

/*	assumes ports all upper case ... */
	if (bPrintToFile) {				// deal with printing to file
		if (szPrintFile != NULL) strcpy(buffer, szPrintFile);
		else strcpy(buffer, "FILE:");	
		if (bDontAskFile) {			//	construct from input file name and path
			*str = '\0';		// ???
			if (quoteflag) strcat(str, "\"");
			strcat(str, DefPath);
/*			DefPath should end with path separator ... test just in case */
			s = str + strlen(str) - 1;
			if (*s != '\\' && *s != '/') strcat(str, "\\");
			strcat(str, OpenName);
			ForceExtension(str, "ps");
			if (quoteflag) 	strcat(str, "\""); /* 97/Oct/23 */
			if (szPrintFile != NULL) free (szPrintFile);
			szPrintFile = zstrdup(str);
//			sprintf(debugstr, "Construct szPrintFile %s", szPrintFile);
//			winerror(debugstr);	// debugging
//			alternative: *buffer = '\0';  // see below
		}
	}
/*	else strcpy(buffer, achPort); */	/* assumes ports all upper case */
	if (strncmp(buffer, "CAS", 3) == 0 ||
		strncmp(buffer, "FAX", 3) == 0 ||		/* 1993/July/15 */
		strncmp(buffer, "FX-WORKS", 8) == 0 ||	/* 1993/Nov/8 */
		strncmp(buffer, "BFDRV", 5) == 0 ||		/* 1994/Sep/3 */
		strncmp(buffer, "WinFax", 5) == 0) {	/* 1994/Nov/17 */
/*		winerror("PostScript output to FaX board?"); */
/*		return -1; */
		if (wincancel("PostScript output to FaX board?") != 0) return -1;
	}
/* need to get rid of colon after port name for DVIPSONE */
	if (strncmp(buffer, "COM", 3) == 0 ||
		strncmp(buffer, "LPT", 3) == 0) *(buffer+4) = '\0';
	if (strncmp(buffer, "AUX", 3) == 0 ||
		strncmp(buffer, "PRN", 3) == 0 ||
		strncmp(buffer, "EPT", 3) == 0 ||
		strncmp(buffer, "NUL", 3) == 0) *(buffer+3) = '\0';
/*			winerror(buffer);	*/				/* debugging */
/*		need to deal with NONE */
	if (strcmp(buffer, "NONE:") == 0) strcpy(buffer, "NUL"); /* ha ha */
/*		need to deal with FILE */	
	if (strcmp(buffer, "FILE:") == 0) {		// print to FILE:
//		if (bUseDVIPSONEDLL == 0 || bCallBackPass == 0) 
//		always print directly to file from DVIPSONE ...
		if (bUseDLLs) {
			strcpy(buffer, "PASSBACK");	// 1999/July/27
//			Windows Print Dialog will get file name later in dialog ???
		}
		else {
			if (bDontAskFile == 0) {
#ifdef DEBUGPRINTING
				if (bDebug > 1) OutputDebugString("DialogBoxParam FileSelect");	
#endif
				flag = DialogBoxParam(hInst, "FileSelect", hWnd,
							  FileDlg, 0L);	/* indicate `Print to File' mode */
				if (flag == 0) return -1;		/* user cancelled */
/*				Use file name that was put in `szHyperFile' by FileDlg */
/*				Dialog puts name temporarily in szHyperFile - need to make copy now */
				if (szPrintFile != NULL) free(szPrintFile);	/* 95/Dec/25 */
				szPrintFile = zstrdup(szHyperFile);			/* 95/Dec/25 */
				strcpy(buffer, szPrintFile); 	/* replace FILE: for DVIPSONE */
			}
//			else *buffer = '\0';
		}
	}

//	what if port name contains a space? use "..." form 97/Jan/5 
//	if (strchr(buffer, ' ') != NULL) 
	if (strchr(buffer, ' ') != NULL && *buffer != '\"') quotestring(buffer);

	if (bDebug > 1) OutputDebugString(buffer);

/*		Now construct command line for DVIPSONE */
/*		sprintf(str, "DVIPSONE -v -B=%d -E=%d -c=%d -d=%s ", 
			frompage, topage, nCopies, buffer); */
/*		Possibly allow for linkage to dvipsone.bat instead of dvipsone.exe ? */
/*		sprintf(str, "DVIPSONE -v -d=%s ", buffer); */
/*		if (strcmp(szDVIPSONEcom, "") != 0) */
	if (szDVIPSONEcom != NULL) 
		strcpy(str, szDVIPSONEcom);		/* call user supplied prog */
	else strcpy(str, "DVIPSONE");		/* or the default */
	strcat(str, " ");					/* fix 1995/June/28 */
	strcat(str, "-v ");
	s = str + strlen(str);				/* fix ? 98/Jun/30 */

//	omit output port specification if will be forced by user command line
	if (bUsingDistiller && szDVIDistiller != NULL) szDVI = szDVIDistiller;
	else szDVI = szDVIPSONE;
//	if (szDVIPSONE == NULL || strstr(szDVIPSONE, "-d") == NULL) {
	if (szDVI == NULL || strstr(szDVI, "-d") == NULL) {
//		if buffer empty, omit output specification
		if (*buffer != '\0') {
			strcat(str, "-d=");
			strcat(str, buffer);	// port or file name in buffer
			strcat(str, " ");
		}
	}
	s += strlen(s);
	if (frompage != 1 || topage != dvi_t) {/* Omit -B= -E= if full range */
		sprintf(s, "-B=%d -E=%d ", frompage, topage);
		s = s + strlen(s);
	}
	if (nCopies != 1) {						/* Omit -c= if nCopies==1 */
		if (bCollateFlag) sprintf(s, "-C=%d ", nCopies);
		else sprintf(s, "-c=%d ", nCopies);
		s += strlen(s);
	}
	if (nDuplex != DMDUP_SIMPLEX) {			/* 96/Nov/17 */
		if (bPassDuplex) {
			strcat(s, "-*r ");				/* pass duplex request */
			if (nDuplex == DMDUP_HORIZONTAL)
				strcat(s, "-*w ");			/* pass tumble 97/Jan/5 */
		}
	}
	if (bLandScape) {
		if (bPassOrient) {				/* 95/June/24 */
			strcat(s, "-*O ");			/* rotate 90 for landscape ? */
		}
		else {
			strcat(s, "-o=90 ");		/* the old way - not correct */
/* need to do more to account for rotation about center of page ... */
		}
		s += strlen(s);
	}
/*		Try and pass paper size to DVIPSONE ? */
	if (bPassSize) 	/*	Try and pass paper size to DVIPSONE ? */
		s = insertpapersize(s, papertype);	/* 95/June/22 */

/*	if (bAllowVerbatim) strcat(str, "-j "); */		/* 94/Mar/5 */
/*	if (*szDVIPSONE != '\0') strcat(str, szDVIPSONE); */
/*	Following may work the way SciWord wants, but ... */
	if (bOddEven == 0) {		/* doing it the old way */
		if (bReversePages != 0) {	/* 92/Sep/27 */
			firstpage = topage;
			strcat(str, "-r ");		/* 92/Sep/25 */
		}
		else firstpage = frompage;
		if (pageincrement == 2) {	/* 92/Sep/27 */
			if (firstpage % 2 == 0) strcat(str, "-h ");	/* even pages */
			else strcat(str, "-g ");					/* odd pages */
		}
	}
	else {		/* do it the way DVIPSONE wants -- 1993/Aug/28 */
		if (bReversePages != 0) strcat(str, "-r ");
		if (bOddOnly != 0) strcat(str, "-g ");
		if (bEvenOnly != 0) strcat(str, "-h ");
	}
/*		Put user specified command line stuff last so it can override above */
/*		if (strcmp(szDVIPSONE, "") != 0) */

//		sprintf(debugstr, "`%s'", str);			/* DEBUGGING */
//		winerror(debugstr);


//	if (szDVIPSONE != NULL) {
	if (bUsingDistiller && szDVIDistiller != NULL) {
		strcat(str, szDVIDistiller);
		strcat(str, " ");					/* 99/Dec/30 */
//		may also want to force use of PS level II - such as image compression
	}
	else if (szDVIPSONE != NULL) {
		strcat(str, szDVIPSONE);
		strcat(str, " ");					/* 95/Jun/22 */
	}

//	quoteflag = 0;
//	if (strchr(DefPath, ' ') != NULL ||	strchr(OpenName, ' ') != NULL) quoteflag = 1;
	if (quoteflag) strcat(str, "\""); /* 97/Oct/23 */
	strcat(str, DefPath);
/*	DefPath should end with path separator ... test just in case */
	s = str + strlen(str) - 1;
	if (*s != '\\' && *s != '/') strcat(str, "\\");
	strcat(str, OpenName);
	if (quoteflag) 	strcat(str, "\""); /* 97/Oct/23 */

//	WritePrivateProfileString(achPr, "LastDVIPSONE", str, achFile);
	WritePrivateProfileString(achDiag, "LastDVIPSONE", str, achFile);

	return 0;
}

/****************************************************************************/

/* Get Here on `Print' Menu Selection */ /* print whole document */

int DoPrintAll(HWND hWnd) {
	int ret, flag;
	int nCmdShow;
	HINSTANCE err;					/* 95/Mar/31 */
	WORD prange;
	char buffer[MAXFILENAME]; 		/* temp port name to strip colon */
	BOOL OldUseCharSpacing;
	DWORD OldTextColor, OldBkColor;

/*	frompage = 1; topage = dvi_t; */	/* 1993/Aug/29 */

	if (frompage < 1) frompage = 1;		/* 1993/Aug/29 */
	if (topage > dvi_t) topage = dvi_t;	/* 1993/Aug/29 */

	if (bCountZero != 0) {			/* Try and get count[0] pages (?) */
		olddvipage = dvipage;
		usepagetable(frompage, 0);
		frompage = (int) counter[0];
		usepagetable(topage, 0);
		topage = (int) counter[0];
		dvipage = olddvipage;
		usepagetable(dvipage, 0);
	}

/*	This passes down temporary space for CheckDVIPSONE to use ... */
/*	if (bUseDVIPSONE) */
/*	CheckDVIPSONE(buffer, OfStruct); */		
	bDVIPSONEexists = CheckDVIPSONE(buffer);	/* set up bDVIPSONEExists */

/*	if printer information not set up, try and get default printer anyway */

	if (pd.hDevNames == NULL) (void) GetDefPrinter();		/* NEW */

/*	If we are not called from command line - need to put up Print Dialog box */
	if (bPrintOnly == 0) {
/*      Issue   : Check if incorrect cast of 32-bit value        */

		prange = (WORD) NewPrintDlg(hInst);					/* 1995/Dec/15 */

/*		frompage = LOWORD(prange); topage = HIWORD(prange); */ /* global ? */
/*		frompage = LOBYTE(prange); topage = HIBYTE(prange); */ /* global ? */
		if (prange == 0 || frompage > topage) {
			bBusyFlag = 0;
			return -1;						/* user cancelled */
		}
	}
	else {							/* command line print only call */
		if (beginpage <= endpage) { /* command line spec of page range */
			if (beginpage > 0 && beginpage <= dvi_t) frompage = beginpage;
			else {					/* invalid parameters ? */
				bBusyFlag = 0;
				return -1;
			}
			if (endpage <= dvi_t && endpage > 0) topage = endpage;
			else {					/* invalid parameters ? */
				bBusyFlag = 0;
				return -1;
			}
		}
		else {						/* invalid parameters ? */
			bBusyFlag = 0;
			return -1;
		}
//		Now set bUseDVIPSONE according to port ??? done above in RangeDlg 
//		Should have required information from GetDefPrinter() 
//		n = setuseDVIPSONE();			/* 97/May/3 */
//		if (n >= 0) bUseDVIPSONE = n;	/* override old value if needed */
	}

/*	do the right thing when bCountZero != 0 ??? */
/*	if (bCountZero != 0) */
	if (bCountZero != 0 && bSinglePage == 0) {		/* 96/Dec/8 */
		olddvipage = dvipage;
		dvipage--;
		usepagetable(frompage, +1);
		frompage = dvipage;
		dvipage--;
		usepagetable(topage, +1);
		topage = dvipage;
/*		sprintf(str, "From: %d To: %d", frompage, topage);
		winerror(str);	*/			/* debugging */
		dvipage = olddvipage;
		usepagetable(dvipage, 0);
	}

/*	final sanity check to avoid disasters ! */
	if (frompage < 1) frompage = 1;
	if (topage > dvi_t) topage = dvi_t;

/*	Can only get here if bDVIPSONEexists is non-zero ??? */
/*	if (bUseDVIPSONE != 0) */	/* let DVIPSONE do all the hard work ! */
	if (bUseDVIPSONE && bDVIPSONEexists) {			// 1994/June/20 
		ret = ConstructCommandLine(hWnd);			// 1998/July/20
		if (ret < 0) return -1;						// user cancelled

		if (nCmdShowForce >= 0) nCmdShow = nCmdShowForce;	/* 94/Mar/7 */
		else nCmdShow = SW_SHOWMAXIMIZED;
/*		This is now dead - since we removed the check box ... */
		if (bRunMinimized != 0) nCmdShow = SW_SHOWMINIMIZED;

/*		call DVIPSONE.DLL --- drop through to the old way if it fails */

/*		check dvipsone.h for definition of DVIPSONE entry point ! */

//		NOTE: this version sends output directly from dvipsone.dll
//		to device or file rather than using callback
		
#ifdef DVIPSONEDLL
//		if (bUseDVIPSONE && bUseDVIPSONEDLL && (bCallBackPass == 0)) 
		if (bUseDVIPSONE && bUseDLLs && (bCallBackPass == 0)) {
			int bCallBack = 0;
//			call dvipsone.dll with NO callback (direct to port or file)
#ifdef IGNORED
			sprintf(debugstr, "RunDVIPSONEDLL command `%s' bCallBack %d (%s)",
					str, bCallBack, "DoPrintAll");
			if (wincancel(debugstr))		// debugging only
				return -1;					
#endif
//			winerror("DOPRINTALL");		// debugging only
			if (bDebug || bShowCalls) {		// 2000 June 5
				err = (HINSTANCE) 0;		/* avoid problems if exit here */
				flag = MaybeShowStr(str, "Application");	/* 95/Jan/8 */
//					if (flag == 0) return 1;	/* cancel => pretend success */
			}
			if (RunDVIPSONEDLL(hWnd, str, bCallBack) == 0)
				return 0;		// finished, success
								// else drop through
		}
/*	in any of the above failure cases we drop through to the old way */
#endif

//		if (bUseDVIPSONE && (bUseDVIPSONEDLL == 0)) 
		if (bUseDVIPSONE && (bUseDLLs == 0)) {
			err = TryWinExec(str, nCmdShow, "DVIPSONE"); /* split off 1995/Jan/2 */

			if (err >= HINSTANCE_ERROR) {
				(void) GetTaskWindow(err);
				return 0;					/* success ! */
			}
			return -1;			/* failed to call DVIPSONE.EXE */
		}				/* never drop through here - end of use DVIPSONE */
	}		

/*	only get here if DVIPSONE.EXE is NOT used --- which may mean DVIPSONE.DLL used */

	if (bBusyFlag == 0) bBusyFlag++;

/*	winerror("DoPrintAll"); */
	resetpagestate(hWnd);

/*	OldStringLimit = StringLimit; */	/* avoid StringLimit if any */
/*	StringLimit = MAXCHARBUF; */
	OldUseCharSpacing = bUseCharSpacing;
	bUseCharSpacing = FALSE;			/* force use of width tables */
	OldTextColor = TextColor;
	TextColor = RGB(0, 0, 0);
	OldBkColor = BkColor;
	BkColor = RGB(255, 255, 255);
/*	OldRulePenDefault = hRulePenDefault; hRulePenDefault = hBlackPen; */
/*	OldRuleBrushDefault = hRuleBrushDefault; hRuleBrushDefault = hBlackBrush; */

	flag = DoPrintStuff(hWnd, -1);

/*	StringLimit = OldStringLimit; */
	bUseCharSpacing = OldUseCharSpacing;
	TextColor = OldTextColor;
	BkColor = OldBkColor;
/*	hRulePenDefault = OldRulePenDefault;
	hRuleBrushDefault = OldRuleBrushDefault; */
	restorepagestate(hWnd);

	return flag;
}	/* end of DoPrintAll */

/* Function to process print manager abort messages - such as out of disk */
/* and also give opportunity for print job or hDlgPrint to cancel the job */
/* print job sets bUserAbort = TRUE if it wants to quit */
/* IsDialogMessage processes message if it is for hDlgPrint */
/* in that case it is not passed to TranslateMessage or DispatchMessage */
/* code == SP_OUTOFDISK => currently out of disk, wait for more if desired */
/* out of disk condition is here ignored ... assumed to clear later */
/* return value non-zero if printing should continue, zero otherwise */

/* Spooler Error Codes */
/* SP_NOTREPORTED		 0x4000 */
/* SP_ERROR		         (-1) */
/* SP_APPABORT		     (-2) */
/* SP_USERABORT		     (-3) */
/* SP_OUTOFDISK		     (-4) */
/* SP_OUTOFMEMORY		 (-5) */

BOOL CALLBACK _export AbortProc(HDC hDC, int nCode) {
	MSG msg;
	char buffer[128];

/*	if (bDebug > 1) {
		sprintf(debugstr, "AbortProc code %d\n", nCode);
		OutputDebugString(debugstr);
	} */
	if (nCode != 0 && nCode != oldaborterr) {	/* check this ? */
		if (bDebug > 1) {
			printerror(nCode, buffer, sizeof(buffer), "ABORTPROC", 0);
		}
	}
	oldaborterr = nCode;			/* avoid complaining about same thing */

	while (!bUserAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!hDlgPrint ||
			!IsWindow(hDlgPrint)) {		/* shouldn't ever happen ! */
			if (bDebug > 1) 
				OutputDebugString("hDlgPrint invalid (B)\n"); 
/*			return (!bUserAbort); */
		}
/*		need IsDialogMessage(...) to send message to modeless dialog box */
		if (!hDlgPrint || 
			!IsWindow(hDlgPrint) ||
			!IsDialogMessage(hDlgPrint, &msg)) { 
/*	pass the message through if it is not for the abort dialog box,*/
/*	or if the abort dialog box is not yet set up, or already destroyed */
			(void) TranslateMessage(&msg);
			(void) DispatchMessage(&msg);
		}
	}
/*	return non-zero if job is NOT to be aborted */
	return (!bUserAbort);
}

/* ******************************* NEWPRINTDLG / WIN32 */

// called from PrintDlgProc to set up text in dialog box

void copydeviceandport(HWND hDlg, char *str) {
	LPDEVNAMES dvnm;

/*	record last printer selected for potential debugging later */
	if (pd.hDevNames != NULL) {
		dvnm = (LPDEVNAMES) GlobalLock(pd.hDevNames);
		wsprintf(str, "to `%s'", (LPSTR) dvnm + dvnm->wDeviceOffset);
		SetDlgItemText(hDlg, IDC_DEVICE, str);
		wsprintf(str, "on `%s'", (LPSTR) dvnm + dvnm->wOutputOffset); 
		SetDlgItemText(hDlg, IDC_DESTINATION, str);
		GlobalUnlock(pd.hDevNames);
	}
}

/***************************************************************************/

/* dialog box with file name and abort button for user to interrupt print */
/* If user clicks button, this procedure here sets bUserAbort to non-zero */ 
/* Change in bUserAbort is picked up by AbortProc above - terminates print */
/* Communicated to the main printing program by a failure in NEWFRAME */

/* wparam and lparam not referenced */ /* Petzold calls this PrintDlgProc */

/* int CALLBACK PrintDlg(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) */
/* int CALLBACK _export PrintDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) */
int CALLBACK _export PrintDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) { 

	switch(msg) {

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
/* don't even bother to analyze wParam and lParam, can only press CANCEL! */
			if (bDebug > 1)	OutputDebugString("CANCEL\n");
/* following is a test to see whether this is a way to go ... */

/*			if (hDlg != hDlgPrint) {
				if (bDebug > 1)	OutputDebugString("hDlg NOT hDlgPrint\n");
			} */
/*			We just *know* that it is cancel / abort ... */
/*			following added from Petzold 1992/Nov/3 */
			EnableWindow (GetParent (hDlg), TRUE); 
/*			following added from Petzold 1992/Nov/3 */
			DestroyWindow(hDlg);	 		/* ??? */
			hDlgPrint = NULL;				/* ??? */
			bUserAbort = TRUE;				/* Set abort flag for AbortProc */
			return TRUE;

		case WM_INITDIALOG:
/*			(void) SetFocus(GetDlgItem(hDlg, IDCANCEL)); */	/* ??? */
/*			if (bFileValid != 0) */
			if (bFontSample == 0) { 
				strcpy(str, "Sending "); 
/*				addpagestring(str + strlen(str), bSpreadFlag); */
				addpagestring(str + strlen(str));
//				strcat(str, " of");
				SetDlgItemText(hDlg, IDC_SOURCE, str);
//				SetDlgItemText(hDlg, IDC_FILENAME, OpenName); /* FileName */
				wsprintf(str, "of `%s'", OpenName);
				SetDlgItemText(hDlg, IDC_FILENAME, str); /* FileName */
			}
			else  {
				SetDlgItemText(hDlg, IDC_SOURCE, "Sending font");
				SetDlgItemText(hDlg, IDC_FILENAME, TestFont);  /* FontName */
			}
/* #ifdef USEPRINTDLG
			if (pd.hDevNames != NULL) {
				dvnm = (LPDEVNAMES) GlobalLock(pd.hDevNames);
				wsprintf(str, "to %s on %s",
					(LPSTR) dvnm + dvnm->wDeviceOffset,
					(LPSTR) dvnm + dvnm->wOutputOffset); 
				GlobalUnlock(pd.hDevNames);
			}
			else strcpy(str, "to nowhere...");
#else
			sprintf(str, "to %s on %s", achDevice, achPort);
#endif */

			copydeviceandport(hDlg, str);

//			SetDlgItemText(hDlg, IDC_DESTINATION, str);

/*	following added from Petzold 1992/Nov/3 */ /* prevent closing this */
			EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_CLOSE, MF_GRAYED);
/*			SetFocus(hDlg); */ /* ??? */
			return (TRUE); 		/* focus is *not* set to a control */
/*			(void) SetFocus(GetDlgItem(hDlg, IDCANCEL)); */
/*			return (FALSE); */	/* if we had set focus ourselves */

			default:
				return(FALSE);
	}
/*	return (FALSE); */
}

/* ******************************* NEWPRINTDLG / WIN32 */

/*		last field if LPDEVMODE pointer to DEVMODE structure */

/* Get Here on `Printer Setup...' Menu Selection */
/* Or from `Setup...' in `Print...' Menu --- in old version only */

/* int DoPrintSetup(HWND hWnd) */
// int DoPrintSetupNew(HWND hWnd) {
int DoPrintSetup (HWND hWnd) {
/*	DWORD err; */
	int oldLandScape, oldpapertype;	/* saved state */
	int newpapertype;				/*temporary var */
/*	HDC hDC; */
/*	HMENU hMenu; */

	oldLandScape = bLandScape;
	oldpapertype = papertype; /* save old state */
	if (pd.hDevNames == NULL) (void) GetDefPrinter();	/* NEW */
	FillDevModeFields();					/* Orientation & Page Size */
/*	memset (&pd, 0, sizeof(PRINTDLG)); */
	pd.lStructSize = sizeof(PRINTDLG);
/*	pd.hwndOwner = NULL; */
	pd.hwndOwner = hwnd;
	pd.hDC = NULL;
	pd.Flags = PD_PRINTSETUP;	/* show Print Setup dialog box */
/*	initialize any more fields ??? */
	if (PrintDlg(&pd)) {
		ReadDevModeFields();			/* read out fields from DEVMODE */
		if (oldLandScape != bLandScape) {
/*			checkpreferences (hwnd);*/	/* landscape check mark change */
/*			InvalidateRect(hwnd, NULL, TRUE); */
/* or, just send IDM_LANDSCAPE message to main Window ? */
			bLandScape = oldLandScape;
			SendMessage (hWnd, WM_COMMAND, IDM_LANDSCAPE, 0L);
		}
		if (oldpapertype != papertype) {
/*			checkpaper (hwnd, papertype); *//* paper type check mark change */
/*			InvalidateRect(hwnd, NULL, TRUE); */
			newpapertype = papertype;
			papertype = oldpapertype;
			SendMessage (hWnd, WM_COMMAND,
						 IDM_LETTER + newpapertype - DMPAPER_LETTER, 0L);
		}
		return TRUE;
	}
	else {
//		ShowCommDlgError("DoPrintSetupNew");
		ShowCommDlgError("DoPrintSetup");
		return FALSE;					/* failure */
	}
}

/****************************************************************************/

#ifdef IGNORED

/* Called from DVIWINDO.C from IDM_PRINTSETUP (old), or init if PrintOnly */

int DoPrintSetup(HWND hWnd) {
#ifdef NEWPRINTDLG						// WIN32
	return DoPrintSetupNew(hWnd);
#else
	return DoPrintSetupOld(hWnd);
//	DoPrintSetupOld no longer exists...
#endif
}

#endif

/****************************************************************************

	FUNCTION: DVIMetric(HWND, unsigned, WORD, LONG)

	PURPOSE: Modeless dialog box to display DVI file information

****************************************************************************/

/* hInfoBox */

/*BOOL CALLBACK DVIMetric(HWND hDlg, unsigned message, WORD wParam, LONG lParam) */
BOOL CALLBACK _export DVIMetric(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HMENU hMenu;
	RECT WinRect;				/* window rectangle - MOVE or RESIZE */	
	WORD id, cmd;

/*	if (bDebug > 1) {
		sprintf(debugstr, "msg %x wParam %lx lParam %lx\n", message, wParam, lParam);
		OutputDebugString(debugstr);
	} */ /* debugging tracing */
	switch (message) {

		case WM_INITDIALOG:
			(void) ShowWindow(hDlg, SW_HIDE);	/* hide while messing ? */
			SetWindowText(hDlg, (LPSTR) dvi_comment); 			
			sprintf(str, "%lu", dvi_num);
			SetDlgItemText(hDlg, IDMB_NUM, (LPSTR) str);
			sprintf(str, "%lu", dvi_den);
			SetDlgItemText(hDlg, IDMB_DEN, (LPSTR) str);
/*			sprintf(str, "%lu.%03lu", dvi_mag/1000, dvi_mag % 1000); */
			sprintf(str, "%.3f", (double) dvi_mag / 1000.0);
			SetDlgItemText(hDlg, IDMB_MAG, (LPSTR) str); 
/*			Is this assuming standard dvi_num and dvi_den ??? */
/*			sprintf(str, "%lu pts", (dvi_l + 32767) / 65536); */
			sprintf(str, "%.2f pts",
					(double) dvi_l * ((double) dvi_num / dvi_den)
					* (72.27 / 254000.0));			/* 99 /Apr/18 */
			SetDlgItemText(hDlg, IDMB_HPLUSD, (LPSTR) str);
/*			Is this assuming standard dvi_num and dvi_den ??? */
/*			sprintf(str, "%lu pts", (dvi_u + 32767) / 65536); */
			sprintf(str, "%.2f pts",
					(double) dvi_u * ((double) dvi_num / dvi_den)
					* (72.27 / 254000.0));
			SetDlgItemText(hDlg, IDMB_WIDTH, (LPSTR) str);
			sprintf(str, "%lu", dvi_bytes);
			SetDlgItemText(hDlg, IDMB_BYTES, (LPSTR) str);  
			strcpy(str, DefPath);
/*			strcat(str, "\\"); */
			strcat(str, OpenName);
			SetDlgItemText(hDlg, IDMB_FILE, (LPSTR) str);  			
/*			SetDlgItemInt(hDlg, IDMB_STACK, (WORD) dvi_s, FALSE);
			SetDlgItemInt(hDlg, IDMB_PAGES, (WORD) dvi_t, FALSE);
			SetDlgItemInt(hDlg, IDMB_FONTS, (WORD) dvi_fonts, FALSE);
			SetDlgItemInt(hDlg, IDMB_VERSION, (WORD) dvi_version, FALSE); */
			SetDlgItemInt(hDlg, IDMB_STACK, (UINT) dvi_s, FALSE);
			SetDlgItemInt(hDlg, IDMB_PAGES, (UINT) dvi_t, FALSE);
			SetDlgItemInt(hDlg, IDMB_FONTS, (UINT) dvi_fonts, FALSE);
			SetDlgItemInt(hDlg, IDMB_VERSION, (UINT) dvi_version, FALSE); 

/* move to last saved position of this box */
			if (InfoxLeft != 0 && InfoyTop != 0) {	/* 1994/Mar/21 */
				SetWindowPos(hDlg, NULL, InfoxLeft, InfoyTop, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
			}
			if (bShowInfoExposed != 0)
				(void) ShowWindow(hDlg, SW_SHOW);	/* show window again ? */
			return (TRUE);	/* Indicates focus is *not* set to a control */

		case WM_COMMAND:	/* 98/Sep/1 */
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			if (bDebug > 1) {
				sprintf(debugstr, "WM_COMMAND id %d cmd %d", id, cmd);
				OutputDebugString(debugstr);
			}
			if (id == IDCANCEL) goto close;  /* drop through into WM_CLOSE */
			else break;					/* else ignore */

		case WM_CLOSE:
close:
/*			bShowInfoFlag = 0; */
/*			bShowInfoExposed = 0; */
			(void) DestroyWindow(hDlg); 
			return(TRUE); 
/*			break; */
		
		case WM_DESTROY:
			bShowInfoFlag = 0;
			bShowInfoExposed = 0;
			hMenu = GetMenu(hwnd);
			(void) CheckMenuItem (hMenu, IDM_SHOWINFO, MF_UNCHECKED);
			return(TRUE); 
/*			break; */

/*		case WM_QUIT:		do some cleanup ? */
			
/*		case WM_PAINT:		refill entries ? */

		case WM_MOVE:							/* 1994/Mar/21 */
			GetWindowRect(hDlg, &WinRect);
			InfoxLeft = WinRect.left;
			InfoyTop = WinRect.top;
			break;

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details */
/*		case WM_COMMAND:		{
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam); 
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			if (bDebug && bDebugKernel) {
				sprintf(debugstr, "id %u cmd %u\n", id, cmd);
				OutputDebugString(debugstr);
			}
			switch (id) {
				case IDM_ESCAPE:
					PostMessage (hDlg, WM_DESTROY, 0, 0L);
					return (TRUE);
				default:
					break;
			}
		} */				/* experiment 1994/Jan/11 */

		default:
			break;	/* ??? */

	}
	return (FALSE);
}

/****************************************************************************

	FUNCTION: ShowDVIMetric(HWND)

	PURPOSE: Create dialog box to show information about DVI file

****************************************************************************/

/* return handle to dialog box ? */

void ShowDVIMetric(HWND hWnd) {		/* ShowInfo */
/*	DLGPROC lpProcDVIMetric; */		/* NEED STATIC ! */

/*	hInfoBox = CreateDialog(hInst, "DVIMetricBox", hWnd,lpProcDVIMetric); */
/*	hInfoBox = CreateDialog(hInst, "DVIMetricBox", hWnd, (DLGPROC) DVIMetric); */
	hInfoBox = CreateDialog(hInst, "DVIMetricBox", hWnd, DVIMetric); 
	SetWindowText(hInfoBox, (LPSTR) dvi_comment);  /* redundant ? */
	SetFocus(hWnd);				/* get focus back to main window ? 1999/Jan/9 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* provide string corresponding to DOS file open error code */
/* at the moment used in DoOpenFile in dviwindo.c only */
/* FLUSHED 1999/Jan/3 */

#ifdef IGNORED
void doserror (int err, char *mss, int nlen, char *filename) {
	int flag=0;

	err = GetLastError();
	if (err > 0) flag = LoadString(hInst, (UINT) (IDS_ERROR + err), mss, nlen);
	if (flag == 0) sprintf(mss, "File Open Error %0X", err);

/*	if (err > 0) flag = LoadString(hInst, (UINT) (IDS_ERROR + err), mss, nlen); */
/* 	if (flag == 0) sprintf(mss, "DOS file open error %d", err); */

	strcat(mss, " ");				/* 1996/July/23 */
	strcat(mss, filename);
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define hexmagic (newmagic+8)	/* start of encrypted owner etc */

/****************************************************************************

	FUNCTION: About(HWND, unsigned, WORD, LONG)

	PURPOSE:  Processes messages for "About" dialog box

	MESSAGES:

		WM_INITDIALOG - initialize dialog box
		WM_COMMAND	- Input received

****************************************************************************/

/* Decrypt owner name, from sour => buff */

/* The decrypted owner string may contain \xyz to indicate char <xyz> */
/* Or it may contain ISO Latin 1 accented characters */

void showowner(char *buff, char *sour, int nlen) {
	unsigned int cryptma = REEXEC;
	unsigned char e;
	int i;
	char *s = sour, *t = buff;

	if (serialnumber == 0) {		/* not customized ... */
		*buff = '\0';
		return;
	}
	for (i = 0; i < 4; i++)				/* prime the pump - discard key */
		e = decryptbyte((unsigned char) *s++, &cryptma);
	for (i = 4; i < nlen; i++) {
		e = decryptbyte((unsigned char) *s++, &cryptma);
		*t++ = (char) e;				/* unsigned char => char ? */
		if (e == 0) break;				/* null byte termination */
	}
	*t = '\0';					/* terminate it */

/*	This eventually won't be used anymore \xyz */
	t = buff;					/* postprocess for accented characters */
	i = 0;
	while ((e = *t) != '\0') {
		if (e == '\\') {			/* look for \xyz string */
/*			winerror(t); */
			i = 0; s = t; t++;
			while ((e = *t) >= '0' && e <= '9') {
				i = (i * 10) + (e - '0'); 
				t++;
			}
			if (i != 0) *s++ = (char) i;
			strcpy(s, t);
			t = s;
			if (e == 0) break;
			continue;				/* 1992/Jan/3 */
		}
		t++;
	}
}

/****************************************************************************/

/*BOOL CALLBACK About(HWND hDlg, unsigned message, WORD wParam, LONG lParam) */
BOOL CALLBACK _export About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	char *s;
	WORD id;

#ifdef DEBUGGING
	if (bDebug > 1) {
		sprintf(debugstr, "message %0X\n", message);
		OutputDebugString(debugstr);
	}				
#endif
	switch (message) {
		case WM_INITDIALOG:
			sprintf(str, "%s (32) Previewer %s", TitleText, version);
/*			sprintf(str, "%s Previewer %s", TitleText, version); */
/* 				(LPSTR) TitleText, (LPSTR) version); */
			SetDlgItemText(hDlg, IDM_VERSION, str);		
			strcpy (str, copyright);
/*			Truncate copyright string before phone number, if any */ 
			if ((s = strchr(str, '.')) != NULL) *s = '\0';
			SetDlgItemText(hDlg, IDM_COPYRIGHT, str);		/* 95/Mar/28 */
/*			sprintf(str, "SN: %d ", (int) (serialnumber/REEXEC)); */
			showowner(str, hexmagic, sizeof(str));
/*			winerror(str); */
/*	split encrypted message into two parts: user + SN & date + time */
			if (bDebug > 1) OutputDebugString(str); 	/* debug 94/Dec/21 */
#ifdef IGNORED
			{
/*				char buffer[512]; */
				char *s=str;
/*				char *t=buffer; */
				char *t=debugstr; 
				while (*s != 0 && *s != '@') {
					sprintf(t, "%d ", (int) (unsigned char) *s);
					s++;
					t += strlen(t);
/*					if (strlen(buffer) > 500) break; */
					if (strlen(debugstr) > DEBUGLEN) break;
				}
				OutputDebugString(debugstr);
			}  /* debugging only */
#endif
			if ((s = (strchr(str, '@'))) != NULL) *s++ = ' ';
			else s = str;				/* empty string fix 95/Nov/3 */
			if ((s = (strchr(s, '@'))) != NULL) *s++ = '\0';
			else s = str;				/* empty string fix 95/Nov/3 */
			SetDlgItemText(hDlg, IDM_USER, str);
			if (s == str) strcpy(str, "ERROR: Uncustomized!");
			SetDlgItemText(hDlg, IDM_DATE, s);
			return (TRUE);	/* Indicates focus is *not* set to a control */

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details */
		case WM_COMMAND:
/*			id = GET_WM_COMMAND_ID(wParam, lParam); */
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
#ifdef DEBUGGING
			if (bDebug > 1) {
				sprintf(debugstr, "WM_COMMAND %0X\n", id);
				OutputDebugString(debugstr);
			}				
#endif
			if (id == IDOK	|| id == IDCANCEL) {
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			return (TRUE);

/*		New: dismiss when clicked anywhere 1996/Aug/14 */
		case WM_LBUTTONDOWN:
			EndDialog(hDlg, TRUE);
			return (TRUE);

/* end of WM_COMMAND case */

		default:
			return(FALSE); /* ? */
	}
/*	return (FALSE); */
}

void ShowAppAndSys(HWND hWnd) {
	char *s, *ss, *str;
	int nlen=1024;

	str = (char *) LocalAlloc(LMEM_FIXED, nlen);	/* allocate str */
	if (str == NULL) return;

	sprintf(str, "%s (32) Previewer %s", TitleText, version);
/*	sprintf(str, "%s Previewer %s", TitleText, version); */

	strcat(str, "\n");
	strcat(str, "\r");
	strcat(str, copyright);
	strcat(str, "\n");
	strcat(str, "\r");
	s =  str + strlen(str);
	showowner(s, hexmagic, nlen - strlen(str));
	s += strlen(s);
	ss = s;
	if ((s = (strchr(ss, '@'))) != NULL) *s++ = ' ';
	else s = ss;
	if ((s = (strchr(ss, '@'))) != NULL) *s++ = ' ';
	else s = ss;
	strcat(s, "\n");
	strcat(s, "\r");
	s += strlen(s);
	ShowSystemInfoAux(s);
	strcat(s, "\n");
	strcat(s, "\r");
	strcat(s, "\n");
	strcat(s, "\r");
	s += strlen(s);
	ShowSysFlagsAux (s);
	strcat(s, "\n");
	strcat(s, "\r");
	(void) PutStringOnClipboard(hWnd, str);
	LocalFree ((HLOCAL) str);	/* free str again */
}

/****************************************************************************

	FUNCTION: SearchDlg(HWND, unsigned, WORD, LONG)

	PURPOSE: Let user select a search string.

****************************************************************************/

/* Mostly now use Common Dialog for search instead --- if CommFind non zero */

/*BOOL CALLBACK SearchDlg(HWND hDlg, unsigned message, WORD wParam, LONG lParam) */
BOOL CALLBACK _export SearchDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int count;
	WORD id;

	switch (message) { 

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
/*			id = GET_WM_COMMAND_ID(wParam, lParam); */
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

			case IDOK:
				count = GetDlgItemText(hDlg, IPS_EDIT, 
/*									searchtext, MAXSEARCHTEXT); */
									searchtext, sizeof(searchtext));
				if(count == 0) {	/* shouldn't happen ! */
					(void) MessageBox(hDlg, "Invalid Search Text.",
						"Search String", MB_ICONSTOP | MB_OK); 
					return (TRUE);  /* what the hell, terminate */
				}
/*				sprintf(str, "%d", count);	winerror(str); */
				bCaseSensitive = (int) SendDlgItemMessage(hDlg, IPS_CASE, 
					BM_GETCHECK, 0, 0L);
				bWrapFlag = (int) SendDlgItemMessage(hDlg, IPS_WRAP,
					BM_GETCHECK, 0, 0L);
				EndDialog(hDlg, count);
				return (TRUE);

			case IDCANCEL:
				EndDialog(hDlg, 0);
				return (TRUE);

/*			case IPS_CASE:
				if (bCaseSensitive == 0) bCaseSensitive = 1;
				else bCaseSensitive = 0;
				return(TRUE); */

			default:
				return (FALSE); /* ? */

			}
/*		break; */ /* ??? */
		
/* end of WM_COMMAND case */		

		case WM_INITDIALOG:						/* message: initialize	*/
			(void) SendDlgItemMessage(hDlg, IPS_EDIT, EM_LIMITTEXT, 
/*												MAXSEARCHTEXT,	0L); */
												sizeof(searchtext), 0L);
			SetDlgItemText(hDlg, IPS_EDIT, (LPSTR) searchtext);
/*	EM_SETSEL in WIN32 wParam starting position, lParam ending position */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
			(void) SendDlgItemMessage(hDlg,		/* dialog handle	*/
				IPS_EDIT,						/* where to send message */
				EM_SETSEL,						/* select characters	*/
				0, -1
/*				0, MAKELONG(0, 0x7fff) */

			);
/*			(void) SetFocus(GetDlgItem(hDlg, IPS_EDIT)); */ /* below */
			(void) SendDlgItemMessage(hDlg, IPS_CASE, BM_SETCHECK, 
				(WPARAM) bCaseSensitive, 0L);
			(void) SendDlgItemMessage(hDlg, IPS_WRAP, BM_SETCHECK, 
				(WPARAM) bWrapFlag, 0L);
			(void) SetFocus(GetDlgItem(hDlg, IPS_EDIT));
			return (FALSE); /* Indicates we set the focus to a control */

			default:
				return(FALSE); /* ? */
	}
/*	return FALSE; */					/* message not processed */
}

/****************************************************************************

	FUNCTION: PageDlg(HWND, unsigned, WORD, LONG)

	PURPOSE: Let user select a page.

****************************************************************************/

/*BOOL CALLBACK PageDlg(HWND hDlg, unsigned message, WORD wParam, LONG lParam) */
BOOL CALLBACK _export PageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int newpage;
	int translated;
	WORD id;

	switch (message) { 

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
/*			id = GET_WM_COMMAND_ID(wParam, lParam); */
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

			case IDOK:
				newpage = (int) GetDlgItemInt(hDlg, IPC_EDIT, &translated, TRUE);
				if(translated == 0) {	/* shouldn't happen ! */
					(void) MessageBox(hDlg, "Invalid Page Number.",
						"Select Page", MB_ICONSTOP | MB_OK);
					return (TRUE);
				}
				EndDialog(hDlg, newpage);
				return (TRUE);

			case IDCANCEL:
				EndDialog(hDlg, -INFINITY);	/* invalid page number */
				return (TRUE);

			default:
				break; /* ? */
			}
			break;

/* end of WM_COMMAND case */

		case WM_INITDIALOG:						/* message: initialize	*/
			if (bCountZero != 0) newpage = (int) counter[0]; /* ??? */
			else newpage = dvipage;
			if (newpage == -INFINITY) newpage = 1;
			(void) SendDlgItemMessage(hDlg, IPC_EDIT, EM_LIMITTEXT,	5, 0L);
/*			SetDlgItemInt(hDlg, IPC_EDIT, (WORD) newpage, TRUE); */ /* FALSE */
			SetDlgItemInt(hDlg, IPC_EDIT, (UINT) newpage, TRUE);  /* FALSE */
/*	EM_SETSEL in WIN32 wParam starting position, lParam ending position */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details */
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
}

/* decide whether this port is the one to use DVIPSONE on 1994/June/3 */
/* szDVIPSONEport may contain semicolon delimited list of ports */
/*  1 means yes, this is a port we want to use DVIPSONE on */
/*  0 means no,  this is a port we do *not* want to use DVIPSONE on */
/* -1 means use the old way of doing this */

int setuseDVIPSONEsub(LPSTR szPort) { 
	char *s, *t;
//	char buffer[128];						/* 97/Sep/25 */
	char buffer[FILENAME_MAX];				/* 99/Jul/31 */
	int n;

	if (bDVIPSONEexists == 0) return 0;		/* should not happen */

/*	if (strcmp(szDVIPSONEport, "") == 0) */	/* were any ports specified ? */
	if (szDVIPSONEport == NULL) {			/* were any ports specified ? */
		return -1;							/* NO: use old bUseDVIPSONE */
	}
	n = strlen(szPort);
	
//	if (n > 127) n = 127;					/* 97/Sep/25 */
	if (n > sizeof(buffer)) n = sizeof(buffer);		/* 99/Jul/31 */

	s = szDVIPSONEport;					/* now known not to be NULL */
	for (;;) {
/*		t = strchr(s, ';'); */	/* extend to ; AND , 94/Nov/10 */
		t = strpbrk(s, ";,");
		if (t == NULL) t = s + strlen(s);
/*		if (bDebug > 1) {
			sprintf(debugstr, "szPort %s DVIPSONEport %s n %d nd %d\n",
				szPort, s, n, t - s);
			OutputDebugString(debugstr);
		}	*/		/* WARNING: remove this again */
		if (t - s == n) {
/*			if (strncmp(szPort, s, n) == 0) return 1; */
			strncpy(buffer, s, n);		/* copy to enable strcmp use */
			*(buffer+n) = '\0';
//			if (strcmp(szPort, buffer) == 0)	/* case sensitive ? */
			if (_stricmp(szPort, buffer) == 0)	/* case sensitive ? */
				return 1;	/* found in list */
		}
		if (*t == '\0') return 0;				/* not found in list */
		else s = t+1;
	}
}

int setuseDVIPSONE (void) {
	int n=0;
	LPDEVNAMES dvnm; 

	if (pd.hDevNames != NULL) {
		dvnm = (LPDEVNAMES) GlobalLock(pd.hDevNames);			
		n = setuseDVIPSONEsub((LPSTR) dvnm + dvnm->wOutputOffset);
//		if (n >= 0) bUseDVIPSONE = n;	/* override old value if needed */
		GlobalUnlock(pd.hDevNames);
	}

	return n;
}

/****************************************************************************

	FUNCTION: FileDlg(HWND, unsigned, WORD, LONG)

	PURPOSE: Let user select a file to print to using DVIPSONE.

****************************************************************************/

BOOL bApplicationFlag=0;			/* non-zero if call for application file */
									/* set in INITDIALOG from lParam*/
/* 0 => get print file name - result in szHyperFile */
/* 1 => get file name - result in SourceOpenName */
/* 2 => get parameter  - result in str */
/* 3 => get working directory - result in str */
/* 4 => get input line for TeX - question in str - result in str */

/* Should we use GetSaveFileName Instead ??? or the "tree" folder dialog ? */
/* for case where we ask for file destination */

/* BOOL CALLBACK FileDlg(HWND hDlg, unsigned message, WORD wParam, LONG lParam) */
BOOL CALLBACK _export FileDlg (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int count=0;
	WORD id;
	char *s;

	switch (message) { 

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

			case IDOK:
				if (bApplicationFlag == 0) { /* get print file name case */
					count = GetDlgItemText(hDlg, IPS_EDIT, 
/*								szPrintFile, sizeof(szPrintFile)); */
							    szHyperFile, sizeof(szHyperFile)); 
// szHyperFile is temporary place - make copy from there after use FileDlg 
				}
				else if (bApplicationFlag == 1)		/* get filename case */
					count = GetDlgItemText(hDlg, IPS_EDIT, 
								SourceOpenName, sizeof(SourceOpenName));
				else if (bApplicationFlag == 2)	/* get parameter case */
					count = GetDlgItemText(hDlg, IPS_EDIT, str, sizeof(str));
				else if (bApplicationFlag == 3)	/* working directory case */
					count = GetDlgItemText(hDlg, IPS_EDIT, str, sizeof(str));
				else if (bApplicationFlag == 4)	/* TeX input line case */
					count = GetDlgItemText(hDlg, IPS_EDIT, str, sizeof(str));

				if (count == 0) {			/* shouldn't normally happen ! */
					if (bApplicationFlag == 2) {
						EndDialog(hDlg, -1);	/* special case mark */
						return (TRUE);
					}
					if (bApplicationFlag == 4) {
						EndDialog(hDlg, -1);	/* special case mark */
						return (TRUE);
					}
					else {
						(void) MessageBox(hDlg, "Invalid File Name",
							"File Selection", MB_ICONSTOP | MB_OK); 
						return (TRUE);			/* what the hell, terminate */
					}
				}
				EndDialog(hDlg, count);
				return (TRUE);

			case IDCANCEL:
				EndDialog(hDlg, 0);
				return (TRUE);

			default:
				return (FALSE); /* ? */
			}
/*		break; */		/* end of WM_COMMAND case */		

		case WM_INITDIALOG:						/* message: initialize	*/
			bApplicationFlag = (BOOL) lParam;	/* remember mode of use */
			(void) SendDlgItemMessage(hDlg, IPS_EDIT, EM_LIMITTEXT, 
					(bApplicationFlag > 1) ? sizeof(str) : MAXFILENAME, 0L);
			if (bApplicationFlag == 0)  {			/* print to file case */
/*				SetDlgItemText(hDlg, IPS_EDIT, szHyperFile); */
				SetDlgItemText(hDlg, IPS_EDIT, szPrintFile);
				SetWindowText(hDlg, "Print to File (DVIWindo)");
				SetDlgItemText(hDlg, IPS_FILE, "Output File Name"); 
			}
			else if (bApplicationFlag == 1) {		/* supply file name case */
				SetDlgItemText(hDlg, IPS_EDIT, SourceOpenName);
				SetWindowText(hDlg, "Select Input File");
				SetDlgItemText(hDlg, IPS_FILE, "File Name");
			}
			else if (bApplicationFlag == 2) {		/* supply parameter */
				if ((s = strchr(str, '|')) != NULL) {
					SetDlgItemText(hDlg, IPS_EDIT, (s+1));
					*s = '\0';
				}
				else SetDlgItemText(hDlg, IPS_EDIT, "");
				SetWindowText(hDlg, str);
				SetDlgItemText(hDlg, IPS_FILE, "Argument"); 
			}
			else if (bApplicationFlag == 3) {		/* Working Directory */
				SetDlgItemText(hDlg, IPS_EDIT, str);
				SetWindowText(hDlg, "Select Working Directory");
				SetDlgItemText(hDlg, IPS_FILE, "Working Directory"); 
			}
			else if (bApplicationFlag == 4) {		/* Question from TeX  */
				SetDlgItemText(hDlg, IPS_EDIT, "");
				SetWindowText(hDlg, "Question from TeX");
				SetDlgItemText(hDlg, IPS_FILE, str); // question here
			}
/*	EM_SETSEL in WIN32 wParam starting position, lParam ending position */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
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
	}	/* end of switch on message */
/*	return FALSE; */					/* message not processed */
}

// get strings from console window back to line starting with !
// font used should be fixed width

#ifdef IGNORED
char *getcontextstr (HWND hConsoleWnd) {
	int nNum, nCount, nLen, nStart = 0, nTotal = 0;
	HWND hWndLB; 
	char *contextstr, *s, *t;

	if (hConsoleWnd == NULL) return NULL;

	hWndLB = GetDlgItem(hConsoleWnd, ICN_LISTBOX);
	nCount = ListBox_GetCount(hWndLB);
	for (nNum = nCount-1; nNum >= 0; nNum--) {
		nStart = nNum;
		nLen = ListBox_GetTextLen(hWndLB, nNum);
		nTotal += nLen + 2;			// extra for \r\n
		ListBox_GetText(hWndLB, nNum, str);
//		if (*str == '!' || *str == '?') 
		if (*str == '!')
			break;					// start at this line
		if (nCount - nStart > 12) break;
	}

	contextstr = malloc(nTotal + 1);
	s = contextstr;

	for (nNum = nStart; nNum < nCount; nNum++) {
		ListBox_GetText(hWndLB, nNum, str);
		t = str + strlen(str) - 1;		// stripping needed ?
		while (t >= str && *t <= ' ') *t-- = '\0';
		strcpy(s, str);
		s += strlen(str);
		strcpy(s, "\r\n");
		s += 2;
	}
	return contextstr;	// 	free(contextstr);
}
#endif

// get strings from console window back to line starting with !
// font used should be fixed width

void showcontext (HWND hQuestion, HWND hConsoleWnd) {
	int nNum, nCount, nStart = 0;
	HWND hWndLB; 

	if (hConsoleWnd == NULL) return;
	SendMessage(hQuestion, LB_RESETCONTENT, 0L, 0L);
	hWndLB = GetDlgItem(hConsoleWnd, ICN_LISTBOX);
	nCount = ListBox_GetCount(hWndLB);
	for (nNum = nCount-1; nNum >= 0; nNum--) {
		nStart = nNum;
		ListBox_GetText(hWndLB, nNum, str);
//		if (*str == '!' || *str == '?') 
		if (*str == '!') break;					// start at this line
		if (strncmp(str, "This is TeX", 11) == 0) break;
		if (strncmp(str, "DONE", 4) == 0) {
			nStart++;
			break;
		}
		if (nCount - nStart > 12) break;
	}

	for (nNum = nStart; nNum < nCount; nNum++) {
		*str = '\0';
		ListBox_GetText(hWndLB, nNum, str);
		SendMessage(hQuestion, LB_ADDSTRING, 0L, (LPARAM) str);
	}
}

void showhelp (HWND hHelp, char *str) {
	char *s= str, *t;
	SendMessage(hHelp, LB_RESETCONTENT, 0L, 0L);
	while ((t = strchr(s, '\r')) != NULL) {
		*t = '\0';
		SendMessage(hHelp, LB_ADDSTRING, 0L, (LPARAM) s);
		s = t+1;
//		while(*s != '\0' && *s < ' ') s++;
//		while(*s == '\r' || *s == '\n') s++;
		if (*s == '\n') s++;
	}
}

// handle input of user line for TeX 99/Nov/10
// make sure this is killed by ESC key ???
// returns 0 if CANCEL is pressed - otherwise length of string + 1

BOOL CALLBACK _export TeXDlg (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int count=0;
	WORD id;
	HICON hIcon;
	RECT WinRect;
	char *s;

	switch (message) { 

		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

				case IDOK:
					count = GetDlgItemText(hDlg, IPS_EDIT, str, sizeof(str)); 
					EndDialog(hDlg, count+1);	// string length + 1
					return (TRUE);

				case IDCANCEL:
					EndDialog(hDlg, 0);
					return (TRUE);

				default:
					return (FALSE); /* ? */
			}
/*		break; */		/* end of WM_COMMAND case */		

//	question and help strings are in str, one after the other

		case WM_INITDIALOG:						/* message: initialize	*/
			bApplicationFlag = (BOOL) lParam;	/* remember mode of use */

			hIcon = LoadIcon(hInst, "DviIcon");
			if (hIcon != NULL) {
//		following sets the B/W icon
//		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) hIcon);
//		following sets the grey/color icon
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
			}
//	Make a horizontal scroll bar appear in the list box.
			ListBox_SetHorizontalExtent(GetDlgItem(hDlg, IPS_QUESTION),
										150 * LOWORD(GetDialogBaseUnits()));
//	Make a horizontal scroll bar appear in the list box.
			ListBox_SetHorizontalExtent(GetDlgItem(hDlg, IPS_HELP),
										150 * LOWORD(GetDialogBaseUnits()));
			if (hConsoleFont != NULL) {
//				SendMessage(hDlg, WM_SETFONT, (LPARAM) hConsoleFont, 0);
				SendMessage(GetDlgItem(hDlg, IPS_EDIT), WM_SETFONT, (LPARAM) hConsoleFont, 0);
				SendMessage(GetDlgItem(hDlg, IPS_PROMPT), WM_SETFONT, (LPARAM) hConsoleFont, 0);
				SendMessage(GetDlgItem(hDlg, IPS_HELP), WM_SETFONT, (LPARAM) hConsoleFont, 0);
				SendMessage(GetDlgItem(hDlg, IPS_QUESTION), WM_SETFONT, (LPARAM) hConsoleFont, 0);
			}
			(void) SendDlgItemMessage(hDlg, IPS_EDIT, EM_LIMITTEXT, MAXFILENAME, 0L);
			SetDlgItemText(hDlg, IPS_PROMPT, str);
//			SetDlgItemText(hDlg, IPS_HELP, str + strlen(str) + 1);
			s = str + strlen(str) + 1;		// force last line to show
			if (*(s + strlen(s)-1) >= ' ') strcat(s, "\r\n");
//			showhelp(GetDlgItem(hDlg, IPS_HELP), str + strlen(str) + 1);
			showhelp(GetDlgItem(hDlg, IPS_HELP), s);
			SetWindowText(hDlg, "Query from TeX");
//			if (hConsoleWnd != NULL && *str == '?') 
			if (hConsoleWnd != NULL) {
//				contextstr = getcontextstr(hConsoleWnd);
//				SetDlgItemText(hDlg, IPS_QUESTION, contextstr);
//				free(contextstr);
				showcontext(GetDlgItem(hDlg, IPS_QUESTION), hConsoleWnd);
			}
			*str = '\0';						/* in case we cancel out */
//			SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
//						 SWP_NOMOVE | SWP_NOSIZE);
			(void) SendDlgItemMessage(hDlg,			/* dialog handle	*/
									  IPS_EDIT,		/* where to send message */
									  EM_SETSEL,	/* select characters	*/
									  0, -1);									/* select entire contents */
			if (QxLeft != 0 && QyTop != 0 && QcxWidth != 0 && QcyHeight != 0) {
//				int horizBorder=4;
//				int topBorder=GetSystemMetrics(SM_CYCAPTION) + 26 + 4;
//				int scrollHeight=GetSystemMetrics(SM_CYHSCROLL) + 8;

// The LISTBOX window can now be positioned and sized
				SetWindowPos(hDlg, NULL, QxLeft, QyTop, QcxWidth, QcyHeight,
							 SWP_NOZORDER);
// The list box can now be sized
//				MoveWindow( hWndQ, horizBorder, topBorder,
//							QcxWidth - (2*horizBorder) - 8,
//							QcyHeight - topBorder - scrollHeight, TRUE );
//				MoveWindow( hWndH, horizBorder, topBorder,
//							QcxWidth - (2*horizBorder) - 8,
//							QcyHeight - topBorder - scrollHeight, TRUE );
			}
			(void) SetFocus(GetDlgItem(hDlg, IPS_EDIT));
			return (FALSE);			/* Indicates we set the focus to a control */

		case WM_MOVE:
			if (IsIconic(hDlg) == 0 && IsZoomed(hDlg) == 0) {
				GetWindowRect(hDlg, &WinRect);
				QxLeft = WinRect.left;
				QyTop = WinRect.top;
//		do width and height also to ensure recording in ini file
				QcxWidth = WinRect.right - WinRect.left;
				QcyHeight = WinRect.bottom - WinRect.top;
			}
			break;

		case WM_SIZE:
			if (IsIconic(hDlg) == 0 && IsZoomed(hDlg) == 0) {
				POINT pt;
				int horizBorder, topBorder, Height, Width;
				int qHeight, qWidth, hHeight, eWidth, oWidth, oHeight;
				RECT dlgRect, qRect, oRect;

				HWND hDlgQ = GetDlgItem(hDlg, IPS_QUESTION);
				HWND hDlgH = GetDlgItem(hDlg, IPS_HELP);
				HWND hDlgP = GetDlgItem(hDlg, IPS_PROMPT);
				HWND hDlgE = GetDlgItem(hDlg, IPS_EDIT);
				HWND hDlgO = GetDlgItem(hDlg, IDOK);
				HWND hDlgC = GetDlgItem(hDlg, IDCANCEL);

				GetClientRect(hDlg, &dlgRect);
				Width = dlgRect.right - dlgRect.left;
				Height = dlgRect.bottom - dlgRect.top;

				GetWindowRect(hDlgQ, &qRect);
				pt.x = qRect.left;
				pt.y = qRect.top;
				ScreenToClient(hDlg, &pt);
				horizBorder = pt.x;
				topBorder = pt.y;

				GetWindowRect(hDlgO, &oRect);
				oWidth = oRect.right - oRect.left;
				oHeight = oRect.bottom - oRect.top;

				qWidth = Width - 2 * horizBorder;
//				qHeight = ((Height - 5 * topBorder) - 2 * (14 * baseunitY) / 8) * 60 / 100;
				qHeight = ((Height - 5 * topBorder) - 2 * oHeight) * 50 / 100;
				if (qHeight < 0) qHeight = 0;
//				hHeight = ((Height - 5 * topBorder) - 2 * (14 * baseunitY) / 8) - qHeight;
				hHeight = ((Height - 5 * topBorder) - 2 * oHeight) - qHeight;
				if (hHeight < 0) hHeight = 0;
				eWidth = qWidth - horizBorder - oWidth;
				if (eWidth < 0) eWidth = 0;
				MoveWindow (hDlgQ, horizBorder, topBorder,
							qWidth, qHeight, TRUE);
				MoveWindow (hDlgP, horizBorder, qHeight + topBorder * 2 ,
							eWidth, oHeight, TRUE);
				MoveWindow (hDlgC, Width - oWidth - horizBorder,
							qHeight + topBorder * 2, oWidth, oHeight, TRUE);
				MoveWindow (hDlgE, horizBorder, qHeight + topBorder * 3 + oHeight,
							eWidth, oHeight, TRUE);
				MoveWindow (hDlgO, Width - oWidth - horizBorder,
							qHeight + topBorder * 3 + oHeight, oWidth, oHeight, TRUE);
				MoveWindow (hDlgH, horizBorder, Height - topBorder - hHeight,
							qWidth, hHeight, TRUE);
				GetWindowRect(hDlg, &WinRect);
				QxLeft = WinRect.left;
				QyTop = WinRect.top;
				QcxWidth = WinRect.right - WinRect.left;
				QcyHeight = WinRect.bottom - WinRect.top;
			}
			break;

		default:
			return(FALSE); /* ? */
	}	/* end of switch on message */
	return FALSE; 					/* message not processed */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int CopyPage(HWND hWnd, HDC hDC) {
	if (bPrintFrame != 0) DoCropMarks(hDC);
	if (paintpage(hWnd, hDC) < 0) return -1;
	else return 0;
} */

RECT CopyClipRect;							/* will be used in winanal */

int xLogExt=0, yLogExt=0;					/* will be used in winsearc.c */

int DoCopy(HWND hWnd, RECT CopyRect) {
	HDC hMetaDC=NULL;
	HANDLE hMetaFile;
	LPMETAFILEPICT hMetaFilePict;
	HGLOBAL hData;
	HDC hDC;
	int xs, ys, xe, ye;
	int flag = 0;
	long lxs, lys, lxe, lye;
/*	BOOL bOldShowImage; */
/*	RECT ClipRect; */
/*	int OldStringLimit; */
	BOOL OldUseCharSpacing;
/*	DWORD OldTextColor, OldBkColor; */
/*	HPEN OldRulePenDefault;
	HBRUSH OldRuleBrushDefault; */
/*	WORD ATMIn, ATMOut; */

	if (bFileValid == 0 && bFontSample == 0) return 0;	/* nothing to do */
#ifdef DEBUGCOPY
	if (bDebug > 1) OutputDebugString("Start DoCopy\n");
#endif
	xs = CopyRect.left;
	ys = CopyRect.top;
	xe = CopyRect.right;
	ye = CopyRect.bottom;
/*	sprintf(str, "Device: xs %d ys %d xe %d ye %d", xs, ys, xe, ye);
	winerror(str); */
	if ((xe > xs + 10) && (ye > ys + 10)) flag++;
/*	Get clipping rectangle in logical coordinates (TWIPS) */
	hDC = GetDC(hWnd);
	(void) SetMapMode(hDC, MM_TWIPS);
	CopyRect = DPtoLPRect(hDC, CopyRect);
	(void) ReleaseDC(hWnd, hDC);
	xs = CopyRect.left;
	ys = CopyRect.top;
	xe = CopyRect.right;
	ye = CopyRect.bottom;
/*	sprintf(str, "Logical: xs %d ys %d xe %d ye %d", xs, ys, xe, ye);
	winerror(str); */
/*	Get clipping rectangle in DVI coordinates */
	lxs = lunmap(xs - xoffset);
	lys = lunmap(yoffset - ys);
	lxe = lunmap(xe - xoffset);
	lye = lunmap(yoffset - ye);
/*	sprintf(str, "DVI: xs %ld ys %ld xe %ld ye %ld", lxs, lys, lxe, lye);
	winerror(str); */

	hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT));
	if (hData == NULL) {
		winerror ("Unable to allocate memory"); /* debug */
		return 0;
	}
	if ((hMetaFilePict = GlobalLock(hData)) == NULL) {
		winerror("Can't Lock Memory");
/*		(void) GlobalFree(hData); */
		hData = GlobalFree(hData);
		return 0;
	}

	if (OpenClipboard(hWnd) == 0) {
		(void) GlobalUnlock(hData);
/*		(void) GlobalFree(hData); */
		hData = GlobalFree(hData);
		winerror("Another application has the Clipboard");
		return 0;
	}
	if ((hMetaDC = CreateMetaFile(NULL)) == NULL) {
		(void) GlobalUnlock(hData);
/*		(void) GlobalFree(hData); */
		hData = GlobalFree(hData);
		(void) CloseClipboard();
		winerror ("Unable to Create MetaFile");
		return 0;
	}
	bCopyFlag = 1;							/* to keep out WM_PAINT */
/*	output here should NOT set mapping mode, so importing application can */
/*	should contain call to SetWindowExt for MetaFile to work */
/*	output produced here is in TWIPS and covers about 8.5 * 11 inches */
/*	following should be here, but ... */
/*	SetWindowExt(hMetaDC, 20 * PageWidth, - 20 * PageHeight); */
/*  should NOT contain calls to SetViewPortExt for MetaFile to work */
/*	SetViewportExt(hMetaDC, 72 * 8, 72 * 10); */
	
#ifdef IGNORED
/*	Mark DC as one requiring ATM intervention 1996/June/4 */
	if (bATMLoaded) {		/* 96/June/4 */
		flag = MyATMMarkDC(BD_VERSION, hMetaDC, NULL, NULL);
/*		flag = MyATMMarkDC(hMetaDC, BD_VERSION, NULL, NULL); */
/*		ATMIn=(WORD) hDC; ATMOut=(WORD) hDC; */
/*		flag = MyATMMarkDC(BD_VERSION, hMetaDC, &ATMIn, &ATMOut); */
		if (flag != ATM_NOERR) {
			if (bDebug > 1) {
				wsprintf(debugstr, "ATM Error %d (%s)\n",
						 flag, (LPSTR) "MarkDC");
				OutputDebugString(debugstr);
			}
		} 
	}
#endif

/*	Now paint everything */
	if (bFontSample != 0) {
/*		SetWindowExt(hMetaDC, 20 * PageWidth, - 20 * PageHeight); */
		resetpagestate(hWnd);
/*	Get clipping rectangle in NEW logical coordinates */
		xs = mapx(lxs); ys = mapy(lys);
		xe = mapx(lxe); ye = mapy(lye);		
		if (flag == 0) {			/* if want to copy whole page */
			xs = 0; ys = 0;
			xe = 20 * PageWidth; ye = - 20 * PageHeight;
		}
		xoffset -= xs; yoffset -= ys;
/*		Remember destination metafile Window Extent for nested MetaFile */ 
		xLogExt = xe - xs;
		yLogExt = ye - ys;
		(void) SetWindowExtEx(hMetaDC, xe - xs, ye - ys, NULL); 
		flag = IntersectClipRect(hMetaDC, 0, 0, xe - xs, ye - ys);
#ifdef IGNORED
		if (bPrintFrame != 0) DoCropMarks(hMetaDC);
#endif
/*		Should we also SetWindowOrgEx ? */
		(void) SetWindowOrgEx(hMetaDC, 0, 0, NULL);			/* 1996/Aug/10 */

		if (bShowWidths != 0) ShowCharWidths(hWnd, hMetaDC);
		else ShowFontSample(hWnd, hMetaDC, -1);

		restorepagestate(hWnd); 
	}	/* end of bFontSample */
	else if (bFileValid != 0) {
/*		we get here if we are showing a file in the window ... */
/*		(void) CopyPage(hWnd, hMetaDC); */
/*		if (bPrintFrame != 0) DoCropMarks(hMetaDC); */

/*		OldStringLimit = StringLimit; */
/*		StringLimit = MAXCHARBUF;	 */
		resetpagestate(hWnd);
/*	suppress image stuff - since it is not debugged */
/*	make this conditional on bDebug ??? */ /* HACK FIX LATER */
/*		bOldShowImage = bShowImage; */
/*		if (bDebug) bShowImage = 0; */

		if (bFileValid) {
			GrabWidths();			/* 99/Jan/12 ??? */
			touchallfonts(hWnd);	/* we want this on main window hDC ? */
			ReleaseWidths();		/* 99/Jan/12 ??? */
		}

/*	Get clipping rectangle in NEW logical coordinates */
		xs = mapx(lxs); ys = mapy(lys);
		xe = mapx(lxe); ye = mapy(lye);		
		if (flag == 0) {			/* if want to copy whole page */
			xs = 0; ys = 0;
			xe = 20 * PageWidth; ye = - 20 * PageHeight;
		}
/*		sprintf(str, "Logical: xs %d ys %d xe %d ye %d", xs, ys, xe, ye);
		winerror(str); */
/*		flag = IntersectClipRect(hMetaDC, xs, ys, xe, ye); */
		xoffset -= xs; yoffset -= ys;

/*		remember for clipping in winanal */
		CopyClipRect.left = 0; CopyClipRect.top = 0;
		CopyClipRect.right = xe - xs; CopyClipRect.bottom = ye - ys;

/*		Remember destination metafile Window Extent for nested MetaFile */ 
		xLogExt = xe - xs;
		yLogExt = ye - ys;
		SetWindowExtEx(hMetaDC, xe - xs, ye - ys, NULL); 
/*		Should we also SetWindowOrgEx ? */
		(void) SetWindowOrgEx(hMetaDC, 0, 0, NULL);	/* 1996/Aug/10 */

		flag = IntersectClipRect(hMetaDC, 0, 0, xe - xs, ye - ys);
/*		sprintf(str, "IntersectClipRect returns %d", flag);
		winerror(str); */
/*		sprintf(str, "xs %d ys %d xe %d ye %d", xs, ys, xe, ye);
		winerror(str); */

		OldUseCharSpacing = bUseCharSpacing;
		bUseCharSpacing = FALSE;
/*		OldTextColor = TextColor;		TextColor = RGB(0, 0, 0); */
/*		OldBkColor = BkColor;		BkColor = RGB(255, 255, 255); */
/*		OldRulePenDefault = hRulePenDefault;	
		hRulePenDefault = hBlackPen; */
/*		OldRuleBrushDefault = hRuleBrushDefault;
		hRuleBrushDefault = hBlackBrush; */
		
		flag = paintpage(hWnd, hMetaDC);	/* negative if failed */

/*	restore previous image showing state */
/*		bShowImage = bOldShowImage; */
/*		StringLimit = OldStringLimit;	*/
		bUseCharSpacing = OldUseCharSpacing;
/*		TextColor = OldTextColor; */
/*		BkColor = OldBkColor; */
/*		hRulePenDefault = OldRulePenDefault; */
/*		hRuleBrushDefault = OldRuleBrushDefault; */
		restorepagestate(hWnd);
	}
#ifdef IGNORED
	else DoCropMarks(hMetaDC); 
#endif

	bCopyFlag = 0;					/* we are done - save for WM_PAINT */
	if ((hMetaFile = CloseMetaFile(hMetaDC)) == NULL) {
		(void) GlobalUnlock(hData);
/*		(void) GlobalFree(hData); */
		hData = GlobalFree(hData);
		(void) CloseClipboard();
		winerror("Error in closing MetaFile");
		return 0;
	}
/*	hMetaFilePict->mm = MM_TWIPS;	*/	/* mapping mode */ /* ??? */
	if (bISOTROPIC) hMetaFilePict->mm = MM_ISOTROPIC;	/* mapping mode */ 
	else hMetaFilePict->mm = MM_ANISOTROPIC;	/* 1994/Oct/5 alternate */
/*  in MM_ISOTROPIC and MM_ANISTORPIC these are in MM_HIMETRIC units */
/*	if (bFontSample != 0) 
		hMetaFilePict->xExt = (int) ((long) PageWidth * 2540 / 72) ; 
	else hMetaFilePict->xExt = (int) ((long) (xe - xs) * 254 / 144) ; */
/*	hMetaFilePict->xExt = (int) ((long) (xe - xs) * 254 / 144) ; */
	hMetaFilePict->xExt = (long) (xe - xs) * 254 / 144 ; 
/*	if (bFontSample != 0) 
		hMetaFilePict->yExt = (int) ((long) PageHeight * 2540 / 72); 
	else hMetaFilePict->yExt = (int) ((long) (ys - ye) * 254 / 144); */
/*	hMetaFilePict->yExt = (int) ((long) (ys - ye) * 254 / 144); */
	hMetaFilePict->yExt = (long) (ys - ye) * 254 / 144;
	hMetaFilePict->hMF = hMetaFile;

	if (bDebug > 1) {
		sprintf(debugstr, "mm %ld xExt %ld yExt %ld", hMetaFilePict->mm,
				hMetaFilePict->xExt, hMetaFilePict->yExt);
		OutputDebugString(debugstr);
	}

/*	mapping mode here doesn't seem to have any effect ? */
/*	PageMaker doesn't like it if xExt=0 & yExt = 0 or xExt < 0 & yExt < 0 */

	(void) GlobalUnlock(hData);/* don't keep locked when giving to ClipBoard */

	(void) EmptyClipboard();
	(void) SetClipboardData(CF_METAFILEPICT, hData);
/*	may need to also use SetClipboardData(CF_PALETTE, ...); */
	(void) CloseClipboard();

	hMetaFile = NULL;	/* now belongs to the clipboard */
	hData = NULL;		/* now belongs to clipboard */
#ifdef DEBUGCOPY
	if (bDebug > 1) OutputDebugString("End DoCopy\n");
#endif
	return 0;
}

/*	DeleteMetaFile(hMetaFile); */  /* probably not ??? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showprintspecs(HWND hWnd) {
	LPDEVNAMES dvnm;

/*	if (pd.hDevNames != NULL) {
		dvnm = (LPDEVNAMES) GlobalLock(pd.hDevNames);
		wsprintf(str, "Driver: %s\nPrinter: %s\nPort: %s",
			 (LPSTR) dvnm + dvnm->wDriverOffset,
			 (LPSTR) dvnm + dvnm->wDeviceOffset,
			 (LPSTR) dvnm + dvnm->wOutputOffset);
		GlobalUnlock(pd.hDevNames);
	}
	else return;
#else
	sprintf(str, "Driver: %s\nPrinter: %s\nPort: %s",
		achDriver, achDevice, achPort);		
#endif */

	if (pd.hDevNames != NULL) {
		dvnm = (LPDEVNAMES) GlobalLock(pd.hDevNames);
		wsprintf(str, "Driver: %s\nPrinter: %s\nPort: %s",
				 (LPSTR) dvnm + dvnm->wDriverOffset,
				 (LPSTR) dvnm + dvnm->wDeviceOffset,
				 (LPSTR) dvnm + dvnm->wOutputOffset);
		GlobalUnlock(pd.hDevNames);
	}
	else return;

	(void) MessageBox(hWnd, str, "DVIWindo print specs",
					  MB_ICONINFORMATION | MB_OK);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define MAXKEY 64 */

/* following used by `Add Item' in `TeX Menu' */

char szMenuKey[MAXKEY];		/* User specified key */

BOOL CALLBACK _export EditItemDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int count;
	WORD id;

	switch (message) { 

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			switch (id) {

/*			case IDOK: */						/* was */
/* we return id, so we can tell what user selected when terminating */
			case IAI_REPLACE:
			case IAI_ADD:
				count = GetDlgItemText(hDlg, IAI_KEY, 
									szMenuKey, sizeof(szMenuKey));
				if(count == 0) {
					(void) MessageBox(hDlg, "Invalid Key.",
						"Edit Item", MB_ICONSTOP | MB_OK); 
					return (TRUE);  
				}					 /* what the hell, terminate */
				count = GetDlgItemText(hDlg, IAI_VALUE, 
									str, sizeof(str));
				if(count == 0) {
					(void) MessageBox(hDlg, "Invalid Pattern.",
						"Edit Item", MB_ICONSTOP | MB_OK); 
					return (TRUE); 
				}					  /* what the hell, terminate */
/*				EndDialog(hDlg, count); */
				EndDialog(hDlg, id);	/* so we know how user ended it */
				return (TRUE);

			case IAI_SEPARATOR:				
			case IAI_DELETE:
			case IAI_NEXT:
				EndDialog(hDlg, id);	/* indicate we want the next item */
				return (TRUE);

			case IDCANCEL:
				EndDialog(hDlg, 0);			/* indicate cancellation */
				return (TRUE);

			default:
				return (FALSE); /* ? */

			}
/*		break; */ /* ??? */
		
/* end of WM_COMMAND case */		

		case WM_INITDIALOG:						/* message: initialize	*/
			(void) SendDlgItemMessage(hDlg, IAI_KEY, EM_LIMITTEXT, 
												sizeof(szMenuKey), 0L);
			(void) SendDlgItemMessage(hDlg, IAI_VALUE, EM_LIMITTEXT, 
												sizeof(str), 0L);
/*			if (strcmp(szMenuKey, "") != 0) */
			if (*szMenuKey != '\0')
				SetDlgItemText(hDlg, IAI_KEY, (LPSTR) szMenuKey);
/*			if (strcmp(str, "") != 0) */
			if (*str != '\0')
				SetDlgItemText(hDlg, IAI_VALUE, (LPSTR) str);
/*	EM_SETSEL in WIN32 wParam starting position, lParam ending position */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
			(void) SendDlgItemMessage(hDlg,		/* dialog handle	*/
				IAI_KEY,						/* where to send message */
				EM_SETSEL,						/* select characters	*/
				0, -1
/*				0, MAKELONG(0, 0x7fff) */
			);									/* select entire contents */
			(void) SetFocus(GetDlgItem(hDlg, IAI_KEY));
			return (FALSE); /* Indicates we set the focus to a control */

			default:
				return(FALSE); /* ? */
	}
/*	return FALSE; */					/* message not processed */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* DrawFrame shows nested set of rectangles for calibrating printer offset */

#ifdef IGNORED
/*	can only happen if DEBUGGING is defined ? */
void DrawFrame (HDC hDC, int xll, int yll, int xur, int yur) {
	POINT Frame[5];				/* 1992/Jan/22 */

	xll += xprintoffset; yll += yprintoffset;
	xur += xprintoffset; yur += yprintoffset;

	Frame[0].x = xll; 	Frame[0].y = yll;
	Frame[1].x = xll; 	Frame[1].y = yur;
	Frame[2].x = xur; 	Frame[2].y = yur;
	Frame[3].x = xur; 	Frame[3].y = yll;
	Frame[4].x = xll; 	Frame[4].y = yll;
	Polyline(hDC, Frame, 5);			/* 1992/Jan/22 */
}

/* hack to help get offset of page in paper correct ... */

/*	can only happen if DEBUGGING is defined ? */
void DoCropMarks (HDC hDC) {
	DrawFrame(hDC, 4 * 1440, -4 * 1440, 9 * 720, -7 * 1440);	
	DrawFrame(hDC, 2 * 1440, -2 * 1440, 13 * 720, -9 * 1440);	
	DrawFrame(hDC, 1 * 1440, -1 * 1440, 15 * 720, -10 * 1440);
	DrawFrame(hDC, 720, -720, 8 * 1440, -21 * 720);
	DrawFrame(hDC, 360, -360, 33 * 360, -43 * 360);
	DrawFrame(hDC, 180, -180, 67 * 180, -87 * 180);
	DrawFrame(hDC, 0, 0, 17 * 720, -11 * 1440);
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* need to preserve DEVMODE data structure - use in CreateDC call */

/* should really use DEVMODE it creates for opening */

/* should NOT modify global printer properties - just this instance */

/* lots of error message strings - maybe curtail after debugging */

/* discrepancy in spacing between screen and printer at high magnification */
/* result of different rounding - different size pixels */

/* also set dmPaperSize based on user selection ? */

/* also set dmDuplex based on user selection ? */

/* move the fonts used display to winfonts.c ? ? ? */

/* should disable PRINT menu item if port == nulldevice */

/* Consider retaining loaded Driver ? Don't FreeLibrary ? */

/* Should showing font sample on printer uses black/white instead of screen */

/* Should copying font sample to clipboad use black/white instead of screen */

/* Need to set up black pen and brush also for printing and copying ... */

/* Seems to be some problem with reuse of DEVMODE structure when... 
 ...different printer driver selected */

/* For some reason, QuickLink II writes junk in YResolution & TTOption */
/* Which should be ignored, since dmFields bits not set, */
/* Yet causes CreateDC to overwrite random parts of memory it seems */

/* Seems to cause crash when different DEVMODE sizes encountered */

/* Maybe totally flush old DEVMODE structure when new driver selected */

/* Not clear why get SP_ERROR error code when user cancels printing ? */

/* Maybe set page range to just a pair of pages if SpreadFlag is on ? */

/* Use DeviceModEx or ExtDeviceModEx ??? WIN32 */

/*    PortTool v2.2     11/2/1995    15:40         */
/*      Found   : ExtDeviceMode         */
/*      Issue   : Replaced by portable ExtDeviceModeEx         */

/* mode selections for the device mode function */

/* #define DM_UPDATE	    1 */
/* #define DM_COPY			2 */
/* #define DM_PROMPT	    4 */
/* #define DM_MODIFY	    8 */

/* #define DM_IN_BUFFER	    DM_MODIFY */
/* #define DM_IN_PROMPT	    DM_PROMPT */
/* #define DM_OUT_BUFFER	DM_COPY */
/* #define DM_OUT_DEFAULT	DM_UPDATE */

/*	NOTE Escape can be used for QUERYESCSUPPORT and PASSTHROUGH */
/*	Other escapes have been replaced with new API calls like StartDoc */
/*	or should be handled using ExtEscape */


