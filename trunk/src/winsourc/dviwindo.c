/* Copyright 1990,1991,1992,1993,1994,1995,1996,1997,1998,1999 Y&Y, Inc.
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

#include "zmouse.h"		// MOUSEWHEEL support in Windows 95 and NT 3.51

/* winhead.h not really needed ? */

#include <direct.h>
#include <time.h>			/* for demo models mostly */
#include <shellapi.h>		/* needed for drag and drop stuff */
#include <errno.h>			/* for EBADF */

#pragma warning(disable:4100)	// unreferenced formal parameters

/* #define DEBUGINIT */		/* DANGEROUS: ALWAYS REMOVE BEFORE RELEASE !!! */

/* #define DEBUGMAIN */		/* for testing purposes only */

/* #define DEBUGENVIRONMENT */

/* #define DEBUGCHANGE */

/* #define DEBUGEXTRACT */

/* #define DEBUGHYPER */

/* #define DEBUGCOMMAND */

/* #define DEBUGPAGETABLE */

/* #define DEBUGOPEN */

/* good end place for precompiled headers ? */

#define WHEEL_DELTA                     120
#define GET_KEYSTATE_WPARAM(wParam)  ((short)LOWORD(wParam))
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define USEPALETTE */	/* 1996/March/24 experiment */

#define ALLOWDEMO		/* demo model - limited lifetime - DANGER ! */

/****************************************************************************

	PROGRAM: dviwindo.c

	PURPOSE: Show DVI files on screen!

	FUNCTIONS:

		WinMain() - calls initialization function, processes message loop
		InitApplication() - initializes window data and registers window
		InitInstance() - saves instance handle and creates main window
		MainWndProc() - processes messages
		About() - processes messages for "About" dialog box
		PageDlg() - let user select a page.
		FontDlg() - let user select a font.
		OpenDlg() - let user select a file, and open it.
		UpdateListBox() - Update the list box of OpenDlg
		ChangeDefExt() - Change the default extension
		SeparateFile() - Separate filename and pathname
		AddExt() - Add default extension

****************************************************************************/

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* File Drag and Drop definitions */
/* remove when switching to Win 3.1 */

/****** Drag-and-drop support ***********************************************/

/* from windows.h */

/* #define WM_QUERYDRAGICON   0x0037 */
/* #define WM_DROPFILES	    0x0233 */

/* from shellapi.h */

/* UINT WINAPI DragQueryFile(HDROP, UINT, LPSTR, UINT); */
/* BOOL WINAPI DragQueryPoint(HDROP, POINT FAR*); */
/* void WINAPI DragFinish(HDROP); */
/* void WINAPI DragAcceptFiles(HWND, BOOL); */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* specific defines needed in this file only */

/* #define VERSION "1.3.0" */		/* 1995/Nov/17 */
/* #define VERSION "2.0.0" */		/* 1996/Sep/10 */
/* #define VERSION "2.0.1" */		/* 1996/Oct/12 */
/* #define VERSION "2.0.2" */		/* 1997/Jan/5 */
/* #define VERSION "2.0.3" */		/* 1997/Jan/23 */ /* UseNewEncode */
/* #define VERSION "2.0.4" */		/* 1997/Feb/23 */ /* UNICODE fixups */
/* #define VERSION "2.0.5" */		/* 1997/Mar/9 */ /* command flags */
/* #define VERSION "2.0.6" */		/* 1997/Apr/23 */ /* encoding menu */
/* #define VERSION "2.0.7" */		/* 1997/May/25 */ /* demo support */
/* #define VERSION "2.0.8" */		/* 1997/Jul/25 */ /* maximized - Aug 3 */
/* #define VERSION "2.0.9" */		/* 1997/Sep/11 */ /* fix `tex font' bug */ 
/* #define VERSION "2.0.10" */		/* 1997/Dec/1 */ /* afmtotfm dvipsone call */
/* #define VERSION "2.0.11" */		/* 1998/Jan/12 */ /* Bezier rule fix */
/* #define VERSION "2.0.12" */		/* 1998/Feb/14 */ /* sreadtwo fix */
/* #define VERSION "2.0.13"	*/		/* 1998/Apr/4 */
/* #define VERSION "2.0.14" */		/* 1998/May/28 */
/* #define VERSION "2.1" */			/* 1998/Jun/10 */	/* RELEASE */
/* #define VERSION "2.1.1" */		/* 1998/Sep/12 */ /* clipping */
/* #define VERSION "2.1.2" */		/* 1998/Nov/4 */ /* synchronicity */
/* #define VERSION "2.1.3" */		/* 1998/Nov/4 */ /* NT 5 T1 fonts */
/* #define VERSION "2.1.4" */		/* 1998/Dec/18 */ /* src editor search */
/* #define VERSION "2.1.5" */		/* 1998/Dec/18 */ /* fix PSfile insertion */
/* #define VERSION "2.1.6" */		/* 1999/Apr/5 */ /* landscape special ignore */
/* #define VERSION "2.1.7" */		/* 1999/May/10 */ /* ExtraSamples in TIFF */
/* #define VERSION "2.1.8" */		/* 1999/May/31 */ /* MultiInstance support */
/* #define VERSION "2.1.9" */		/* 1999/May/31 */ /* AFMtoTFM DLL */
/* #define VERSION "2.1.10" */		/* 1999/Nov/16 */ /* YANDYTEX DLL */
/* #define VERSION "2.2.0" */		/* 1999/Dec/26 */
/* #define VERSION "2.2.1" */		/* 2000/Feb/12 */
/* #define VERSION "2.2.2" */		/* 2000/Apr/8 */
/* #define VERSION "2.2.3" */		/* 2000/May/22 */
#define VERSION "2.2.4"				/* 2000/June/17 */

// #define MAXGRAPHICSTACK 16		/* max depth of graphics stack */

/* #define MAXDIRLEN		80 */		/* used by file open dialog */
/* #define MAXSPECLEN		13 */		/* used by file open dialog */
/* #define MAXEXTLEN		5 */

#define SCROLLTHRESH	2		/* used by click and drag */
/* #define SCROLLTHRESH	4 */	/* used by click and drag */

#define SCROLLSTEP		40

#define TICKTOCK		1000	/* milliseconds in timer interval */

/* #define MAXTITLE		20 */	/* (Inactive DVIPSONE) */
#define MAXTITLE		32		/* (Inactive DVIPSONE) */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
int nMagicFact=10000;	/* adjust nHeight of fonts 97/Sep/14 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Define a structure used to hold info about an app we */
/* start with ExecApp so that this info can be used again */
/* later to test if the app is still running */

// typedef struct _EXECAPPINFO {
//     HINSTANCE hInstance;            /* The instance value */
//     HWND hWnd;                      /* The main window handle */
//     HTASK hTask;                    /* The task handle */
// } EXECAPPINFO, FAR *LPEXECAPPINFO;

// EXECAPPINFO TaskInfo;				/* place for Task info */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* In the new way, DVISETUP modifies the EXE file directly */
/* newmagic is the old hexmagic string with 8 preliminary bytes added */
/* four are the author signature, four are the serial number * REEXEC */

long serialnumber=0;
char newmagic[97]=				/* coordinate with winsetup.c */
"bkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkph";
#define hexmagic (newmagic+8)	/* start of encrypted owner etc */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* NOTE: following is correct, but differs from what SDK says */

DWORD CombinedVersion;		/* set up when starting DOS + Windows version */
WORD WindowVersion;			/* set up when starting - Windows version */
WORD DOSVersion=0;			/* set up when starting - DOS version */
WORD WinVerNum;				/* rearranged bytes of Windows Version 95/Nov/3 */

long xoffset = 0;			/* current x offset in mapping to screen */
long yoffset = 0;			/* current y offset in mapping to screen */

int wantedzoom = 0;			/* current zoom value */

long fontxoffset = 0;		/* font show x offset in mapping to screen */
long fontyoffset = 0;		/* font show y offset in mapping to screen */

int fontzoom = 0;			/* font show zoom value */ /* 96/Aug/26 */

int magbase = 1000;			/* user selected base magnification default 1000 */

long pagexoffset = 0;		/* page default x offset in mapping */
long pageyoffset = 0;		/* page default y offset in mapping */

int pagewantedzoom = 0;		/* page default zoom value */

long spreadxoffset = 0;		/* spread default x offset in mapping */
long spreadyoffset = 0;		/* spread default y offset in mapping */

int spreadwantedzoom = 0;	/* spread default zoom value 96/May/12 */

/* ******************************************************************** */

long pagexoffsetsvd = 0;		/* page default x offset in mapping */
long pageyoffsetsvd = 0;		/* page default y offset in mapping */

int pagewantedzoomsvd = 0;		/* page default zoom value */

long spreadxoffsetsvd = 0;		/* spread default x offset in mapping */
long spreadyoffsetsvd = 0;		/* spread default y offset in mapping */

int spreadwantedzoomsvd = 0;	/* spread default zoom value 96/May/12 */

/* ******************************************************************** */

long xoffsetold = 0;		/* saved state while showing DVI fonts */
long yoffsetold = 0;

int wantedzoomold = 0;		/* saved value of wantedzoom while in Font Show */

int metricsize=METRICSIZE; /* 1000 - size used for getting metrics */

int graphicindex = 0;				/* stack pointer */

long xoffsetsaved[MAXGRAPHICSTACK], yoffsetsaved[MAXGRAPHICSTACK];

int wantedzoomsaved[MAXGRAPHICSTACK];		/* saved value */

/* need following from winhead.h */

unsigned int TestSize = 20 * 20;	/* size to use in tests (TWIPS) */

char TestFont[MAXFACENAME] = "";	/* font name used in tests in winfonts.c */

WORD remaptable[128];					/* cut down 1995/June/23 */

/* unsigned char remaptable[128]; */

COLORREF TextColor; 	/* RGB(0, 0, 0); default Window Text Color */
COLORREF BkColor;		/* RGB(255, 255, 255); default Background Color */
COLORREF OrgBkColor;

int RBorderPen=255, GBorderPen=0, BBorderPen=255;	/* magenta */
/* int RLinerPen=0, GLinerPen=255, BLinerPen=255; */
int RLinerPen=0, GLinerPen=196, BLinerPen=255;		/* cyan */
int RFigurePen=128, GFigurePen=0, BFigurePen=128;	/* dark magenta */

DWORD FrameColor = RGB (128, 0, 128);		/* default EPS frame color */

char szPreviewHotKey[10]="C-F1";			/* default preview hotkey */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following are set from [Window] section of dviwindo.ini */

char *szDVIPSONEport=NULL;	/* port with PS DVIPSONE printer(s) achPr */

char *szDVIPSONEcom=NULL;	/* first item if contains . 95/June/23 achPr */

char *szDVIPSONE=NULL;		/* passed to DVIPSONE 94/June/3 achPr */

char *szDVIDistiller=NULL;	/* passed to DVIPSONE/Distiller 99/Dec/30 achPr */

char *szAFMtoTFMcom=NULL;	/* first item if contains . 95/June/23 achPr */

char *szAFMtoTFM=NULL;		/* passed to AFMtoTFM 95/June/26 achPr */

char *szYandYTeX=NULL;		/* passed to Y&YTeX 94/July/10 achPr*/

char *szYandYTeXcom=NULL;	/* passed to Y&YTeX 94/July/10 achPr */

char *szRegistry=NULL;		/* default ttfonts.reg 95/Aug/15 achPr */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following are set from [Environment] section of dviwindo.ini */

char *szEditorDDE=NULL;		/* info to call Editor by DDE 98/Nov/5 achEnv */

char *szTeXEdit=NULL;		/* info to call Editor by WinExe 98/Nov/5 achEnv */

char *szCustom=NULL;		/* custom encoding name 97/Dec/22 achEnv */

char *szCustomPageSize=NULL;		/* custom page size string 2000 May 27 achEnv */

/* ******************* following are from environment variables if defined */

char *szBasePath=NULL;		/* non blank if new directory structure 96/Aug/29 */
							/* does not include terminating separator ? */

char *szEPSPath=NULL;		/* default path from PSPATH */

char *szVecPath=NULL;		/* default path from VECPATH */
							/* no trailing terminator */

char *szPrePath=NULL;		/* default path from PREPATH */
							/* no trailing terminator */

char *szTeXFonts=NULL;		/* TEXFONTS env variable */
							/* or encoding specific env var */

char *szTeXDVI=NULL;		/* Y&YTeX DVI output redirection */

/********************************************************************************/

char *logtext=NULL;			/* console text buffer 99/Jun/25 */

/********************************************************************************/

char *szWinDir=NULL;		/* windows directory 1996/July/23 */

char *szExeWindo=NULL;		/* path of executable file */
							/* *includes* terminating separator */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef AllowCTM
BOOL bUseAdvancedGraphics=1;
BOOL bAdvancedGraphicsMode=0;
BOOL bUseCTMflag=1;					/* new 1996/Nov/5 */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long oneinch=ONEINCH;			/* 99/Apr/18 */

unsigned long dvi_num = 25400000;		/* default numerator of scale */
unsigned long dvi_den = 473628672;		/* default denominator of scale */
unsigned long dvi_mag = 1000;			/* default magnification */

unsigned long dvi_l = 0;			/* max page height + depth */
unsigned long dvi_u = 0;			/* max width */
unsigned long dvi_bytes = 0;		/* size of file in bytes */

char dvi_comment[MAXCOMMENT]="No DVI file open";	/* for TeX comment */

int dvi_s = 0;				/* max stack depth */
int dvi_t = 0;				/* number of pages (bop) in DVI file */
int dvi_version = 0;		/* DVI version from header */
int dvi_fonts = 0;			/* number of fonts seen */

long current=-1;		/* pointer to bop of current page */

/* int maxmarks=0; */  /* maximum number of marks fit in memory allocated */

int nApplications=0;	/* number of applications found in [Applications] */
int nEnvironment=0;		/* number of vars found in [Environment] */

int nDefaultTIFFDPI=72;		/* if resolution not specified in TIFF */

DWORD FileSizeLow;		/* 32 low order bits of 64 bit FileSize */
DWORD FileSizeHigh;		/* 32 high order bits of 64 bit FileSize */
FILETIME FilewTime;		/* 64 bit .dwLowDateTime, .dwHighDateTime */
						/* 100 nano sec interval since 1601 Jan 1 */
/* long FileSize; */
/* long FilemTime; */

int nFileMax=0;			/* maximum file number findepsfile */

BOOL bUseTimer=1;		/* non-zero to use timer to track changes in DVI */
UINT bTimerOn=0;		/* non-zero when timer is active */
BOOL bSleeping=0;		/* non-zero in iconic state - restart timer later */

UINT nAlarmTicks=0;		/* how many clock ticks since change noted */
/* UINT nRefreshDelay=1; */	/* how many clock ticks to wait before refresh */
int nRefreshDelay=0;	/* no wait - just refresh 95/Jan/2 new default */

BOOL bAwakened=0;		/* non-zero when Timer noted file change */

int OfShareCode = OF_SHARE_DENY_WRITE;	/* default for Open File */

int OfExistCode = OF_SHARE_DENY_NONE;	/* default for File Exist */

BOOL bSpinOK = 0;			/* non-zero if custom spinner child registered */
BOOL bAvoidFlood=1;			/* new default 94/Jan/10 */

/* BOOL bPreventConflict=0; */	/* don't WM_PAINT if DVI file in use */
BOOL bPreventConflict=1;	/* new default 95/Jan/10 */

BOOL bFloppyDisk=0;		/* if DVI file appears to be on floppy disk */

BOOL bHiresScreen=0;	/* if screen better than VGA */

BOOL bAllowDrag=1;		/* non-zero implies allow drag and drop */
BOOL bUsingDrag=0;		/* non-zero if initialized drag-and-drop */

BOOL bAllowCommDlg=1;	/* non-zero common dialog box - hard wired 95/Sep/3 */
BOOL bUsingCommDlg=0;	/* non-zero if COMMDLG.DLL functions available */

BOOL bDDEServer=1;		/* Set up to act as DDE Server 98/Dec/12 */
DWORD idServerInst=0;	/* Server instance if DDE Server set up */

BOOL bAddToMRU=1;		/* when opening file 99/Jan/3 */
						/* set to 0 for INSERT, ... */

/* Avoid use of registry if possible at least in NT --- 1995/Feb/15 */
/* BOOL bUseRegistryFile=1; */	/* OK to try and call RegEdit in Windows 95 */
BOOL bUseRegistryFile=0;	/* OK to try and call RegEdit in Windows 95 */
BOOL bRegistrySetup=0;	/* If RegEdit has already been called */
BOOL bAlwaysWriteReg=0;	/* If set, always dump ttfonts.reg ahead of time */
/* BOOL bUseRegEnumValue=1; */ /* Use KRNL386 imports to registry Windows 95 */
BOOL bUseRegEnumValue=0;/* Use KRNL386 imports to registry Windows 95 */

/* BOOL bNewSearch=0; */		/* non-zero use CommDlg Find Dialog */
BOOL bNewSearch=1;		/* non-zero use CommDlg Find Dialog 95/Mar/1 */

BOOL bCompressColor=1;	/* compress 24 bit RGB color to palette color */
BOOL bStretchGray=1;	/* expand (mingrey - maxgrey) range to full range */
BOOL bStretchColor=1;	/* expand color range to use dynamic range properly */
BOOL bBGRflag=0;		/* interchange B and R if non-zero */
BOOL bTTRemap=0;		/* remap TrueType font control character range */
BOOL bPatchBadTTF=0;	/* deal with `text' TTF fonts that aren't */
BOOL bPassThrough=1;	/* allow EPS file pass through if PS printer */
BOOL bShowPreview=1;	/* non-zero => Show TIFF and EPSI previews */
BOOL bAllowBadBox=1;	/* allow single % comment before %%BoundingBox */
BOOL bForceTIFF=0;		/* force search for .tif file for .eps file */
BOOL bQuickLink=1;		/* non-zero => work around QuickLink II Fax bug */
BOOL bEscapeEnable=1;	/* non-zero => Escape exits program */
BOOL bTrueTypeOnly=0;	/* non-zero => use only TrueType fonts */
BOOL bTypeOneOnly=0;	/* non-zero => use only Adobe Type 1 fonts */
BOOL bHelpAtEnd=0;		/* non-zero => move `Help' and `About' to end */
BOOL bRememberDocs=1;	/* non-zero => remember opened documents 96/Sep/16 */
BOOL bKeepSorted=1; 	/* non-zero => keep opened documents sorted 96/Jun/5 */
BOOL bSortEnv=1;		/* keep environment sorted 98/Sep/1 */

/* BOOL bUpperCase=1; *//* convert font name to upper case 97/Oct/20 */
BOOL bUpperCase=0;		/* change default to 0 when found to be safe */
BOOL bHyperText=1;		/* implement hypertext linkage */
BOOL bHyperUsed=0;		/* if DVI page contains hypertext linkage */
BOOL bHyperLast=0;		/* last mouse action was hyper-tex, not zoom */
BOOL bSourceUsed=0;		/* if DVI page contains src linkage */
BOOL bShowButtons=0;	/* show hypertext buttons if on */
BOOL bShowViewPorts=0;	/* show viewports of Windows MetaFiles */
BOOL bShowWMF=1;		/* allow showing of Windows MetaFiles */
BOOL bShowTIFF=1;		/* allow showing of TIFF insertimage: */
BOOL bShowFileName=1;	/* show name of file for EPS figure w/o preview */
BOOL bShowImage=1;		/* show ESPF or TIFF image */ /* 97/Nov/7 */
BOOL bTrueInch=0;		/* use true inches in ruler measurement */
BOOL bVerbatimFlag=0;	/* non-zero allow verbatim PostScript 96/Oct/2 */
BOOL bAllowAndrew=0;	/* try and allow Andrew Trevorrow style */
BOOL bSinglePage=0;		/* `Print Current Page' instead of `Print' called */
						/* only used to avoid page number overwrite ... */
BOOL bInfoToClip=1;		/* copy system info to clipboard */
BOOL bFlipRotate=0;		/* flip sign of rotation from PS code */
#ifdef USEUNICODE
BOOL bUseNewEncodeTT=0;	/* Use UNICODE method for TT encoding 97/Jan/16 */
						/* should veto bTTRemap in this case ? */
						/* can use NewEncodeTT=1 in Windows 95 */
BOOL bUseNewEncodeT1=0;	/* Use UNICODE method for T1 encoding 97/Jan/16 */
						/* Both set non-zero in Windows NT */
BOOL bOldUnicode=0;		/* Use old UNICODE assignments */
#endif
BOOL bDecorative=1;		/* Family == FF_DECORATIVE => NOT text font */
						/* needed for auto converted T1 fonts in NT */
						/* but problem because some TT text set this */
BOOL bDontCare=1;		/* Family == FF_DONTCARE => not text font */
						/* important for auto converted non text */
BOOL bDrawOutline=1;	/* show character outline in ShowFonts */
BOOL bDrawKnots=1;		/* draw knots on outline in ShowFonts */
BOOL bDrawControls=1;	/* show control points of curveto's */
BOOL bShowCommand=0;	/* show command line when entering 99/Apr/11 */

// BOOL bUseAFMtoTFMDLL=0;	/* AFMtoTFM.DLL 99/June/13 */
// BOOL bUseYandYTeXDLL=0;	/* YANDYTEX.DLL 99/June/13 */
// BOOL bUseDVIPSONEDLL=0;	/* DVIPSONE.DLL 99/June/13 */

BOOL bUseDLLs=1;		/* use AFMtoTFM.DLL and DVIPSONE.DLL and YandYTeX.DLL */
BOOL bConsoleOpen=1;	/* keep Console Window Open */
BOOL bCallBackPass=1;	/* callback from dvipsone.dll and passthrough */
BOOL bDontAskFile=1;	/* don't ask for file name when "Print to File" */
BOOL bOldPassThrough=0;	/* force old PASSTHROUGH even for Distiller */
BOOL bNewPassThrough=0;	/* POSTSCRIPT_PASSTHROUGH instead of PASSTHROUGH */
BOOL bForcePassBack=0;	/* force passback of print to file output */
BOOL bOpenClosePrinter=0;	/* use OpenPrinter/ClosePrinter and StartDoc/EndDocPrinter */ 
BOOL bOpenCloseChannel=0;	/* use OPENCHANNEL/CLOSECHANNEL not STARTDOC/ENDDOC */ 

BOOL bUseFontName=0;	// use FontName instead of FileName in WriteAFM/TFM 2000 July 4
						// write TFMs using FontNames, not file names 2000 July 4 

#ifdef USEPALETTE
BOOL bUsePalette=0;		/* 1996/March/24 */
BOOL bUpdateColors=1;	/* UpdateColors rather than redraw client area */
HPALETTE hPal=NULL;		/* 96/March/24 */
int nUpdateCount = 0;	/* how many times UpdateColors has been called */
#endif

BOOL bCallSelf=0;		/* -1 command flag to call other instance 98/Dec/15 */
BOOL bSearchCall=0;		/* -s command flag search call from editor 98/Dec/15 */
BOOL bPageCall=0;		/* -p command line flag 99/Jan/10 */
//BOOL bNotFirst=0;		/* non-zero if another DVIWindo already running */
BOOL bFirstInst=0;		/* zero if another DVIWindo already running */
						/* set from DVIWindo_Mutex in WinMain */
BOOL bDebugKernel=0;	/* non-zero => running in Windows debug kernel */
BOOL bPrivate=0;		/* non-zero => my own copy --- not used ??? */
BOOL bDebug=0;			/* non-zero => debugging mode request - command line */
						/* 1 => allow only MessageBox / winerror output */
						/* 2 => enable also OutputDebugString output */

BOOL bShowCalls=0;		/* non-zero => show WinExec calls */
						/* controlled by int in Preferences */
BOOL bPauseCalls=0;		/* non-zero => pause WinExe calls */
						/* controlled and controls by DEBUGPAUSE env var */
						/* env var read by DVIPSONE and YANDYTEX =>
						 * causes them to show command line if set */
BOOL bDebugMenu=0;		/* keep extension of "special" menu for debugging */

BOOL bWin95=FALSE;		/* non-zero => Windows 95 (or 98, but not NT) */
BOOL bWin98=FALSE;		/* non-zero => Windows 98 (not NT) --- not used */
BOOL bWinNT=FALSE;		/* non-zero => Windows NT (or greater, not not 95/98) */
BOOL bWinNT5=FALSE;		/* non-zero => Windows NT >= 5 ? W2K */
BOOL bNewShell=FALSE;	/* non-zero => new shell (Win 95 user interface) */
BOOL bATM4=FALSE;		/* non-zero => ATM 4.0 - used only GetKerningPairs */

BOOL bCommFont=TRUE;	/* use new dialogs for fonts etc */
BOOL bLongNames=FALSE;	/* show long names in OpenFile dialog 95/Dec/1 */
						/* LONGNAMES preprocessor flag stops use OpenFile */
						/* use _lopen _lread _lclose instead 95/Dec/1 */
BOOL bGhostHackFlag=FALSE;		/* for now */
UINT nDriveD=0;			/* drive D: DRIVE_FIXED DRIVE_REMOVABLE DRIVE_REMOTE */
BOOL bReadFile=0;		/* non-zero => read requested file when launched */
						/* DVI file name given on command line */
BOOL bReadHelp=0;		/* non-zero if asked to read help file 2000/Jan/5 */
BOOL bPrintOnly=0;		/* print specified file - no screen display */
						/* positive means no printer was specified */
						/* use default printer in that case */
						/* also used by Print Current Page */

BOOL bMultiInstance=0;	/* don't call existing instance with file on command line */
BOOL bApplications=0;	/* [Applications] section found in DVIWINDO.INI */
BOOL bEnvironment=0;	/* [Environment] section found in DVIWINDO.INI */

BOOL bCurrentFlag=1;	/* search in current directory first */
						/* used in getthepath / expandhash in winsearc.c */
BOOL bUnixify=0;		/* convert \ to / when expanding # in TeX Menu */
						/* not needed with Y&Y TeX normally ... */
BOOL bUseDVIWindo=1;	/* use [Environment] section of `dviwindo.ini' */
						/* grabenv otherwise uses getenv DOS env vars */

BOOL bElCheapo=0;		/* non-zero => fast/cheap TPIC dashes/splines */

BOOL bAllowTPIC=1;		/* non-zero => allow TPIC \specials */

BOOL bAllowClosed=1;	/* allow closure of splined curves */

BOOL bGrayFlag = 1;		/* gray PREVIOUS & NEXT if appropriate */

BOOL bTeXHelpFlag=0;		/* last call from tex menu was to texhelp */
BOOL bTeXHideFlag=0;		/* original DVI file saved while TeXHelp */

BOOL bUseWorkPath = 0;		/* use DVIWindo `working directory' */
BOOL bUseSourcePath = 1;	/* use directory of file TeX worked on */
							/* these two ought to be mutually exclusive */
BOOL bUseTeXDVI=0;			/* set if TEXDVI environment variable set */
							/* all DVI output goes to this directory if set */

BOOL bIgnoreBadANSI=0;			/* do not check ANSI encoding violations */

BOOL bNeedANSIacce=0;			/* set if char < 32 in ANSI font */
BOOL bNeedANSIwarn=0;			/* set if already warned about this */
int  nNeedANSIfont=0;			/* what font last violated ANSI encoding */
int  nNeedANSIchar=0;			/* what char violated ANSI encoding ... */

BOOL bIgnoreBadInstall=0;		/* do not warn about T1INSTALL problem */
BOOL bT1InstallWarn=0;			/* warned about T1INSTALL yet ? */
/* BOOL bT1WarnOnce=1; */		/* warn just for first font */
BOOL bT1WarnOnce=0;				/* warn just for first font 96/Nov/30 */

BOOL bUnitsWarned=0;			/* if warned yet about bad units in \special */
BOOL bZeroWarned=0;				/* if warned about zero width or height */

BOOL bKeepZero=0;				/* keep zero size kern pairs in WriteAFM */

int nCmdShowForce=-1;			/* negative, or CmdShow for WinExec 94/Mar/7 */

BOOL bQuoteAtSign=0;			// replace @.tex with "@.tex" automatically 

/* 0 Hide, 1 ShowNormal/Normal, 2 ShowMinimized, 3 ShowMaximized/Maximize */
/* 4 ShowNoactive, 5 Show, 6 Minimize, 7 ShowMinNoactive, 8 ShowNA, 9 Restore*/

BOOL bSmallStep=0;				/* non-zero for small zoom steps */
BOOL bSciWord=0;				/* Scientific Word Figure Inclusion */

BOOL bISOTROPIC=1;				/* default MM_ISOTROPIC in MetaFile output */
			/* Windows Write cannot handle MM_ISOTROPIC */
			/* on the other hand MM_ANISOTROPIC allows unequal scaling */
			/* which screws up text since it is only isotropically scalable */

BOOL bBadFileComplained=0;			/* have complained aready ? */
BOOL bBadSpecialComplained=0;		/* have complained aready ? */

// can't have OLDPRINTDLG defined in WIN32...

// #if defined(NEWPRINTDLG) && defined(OLDPRINTDLG)
// BOOL bCommPrint=FALSE;		/* use new print dialogs 96/Jan/7 */
// #endif

BOOL bDismissFlag=1;		/* Dismiss Find dialog box 96/Aug/14 */

BOOL bShowFullName=1;		/* show full file name in title bar */

BOOL bConvertUnderScores=0;	// Convert Underscors to Spaces flipped 2000 July 4 

BOOL bAllowFontNames=1;		// allow use of FontNames as TFM names 2000 July 4 

/* BOOL bSynchronicity=0; */		/* 1998/Nov/5 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define HYPERFILE

/* Simple hypertext state push down stack */ /* push graphics state also ? */

long hyperpos[MAXHYPERPUSH];	/* saved position in file */

int hyperpage[MAXHYPERPUSH];	/* saved page number in file */

#ifdef HYPERFILE
char *hyperfile[MAXHYPERPUSH];	/* saved file name or NULL -> hMarks */
#endif

int hyperindex=0;				/* index into the above */

COLORREF CurrentTextColor, CurrentBackColor;		/* 1995/April/30 */

COLORREF ColorStack[MAXCOLORSTACK];

int colorindex=0;				/* points at next slot to push into */

int bColorUsed=0;				/* This DVI file has \special{color ...} */

int bBackUsed=0;			/* This DVI file has \special{background ...} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

HINSTANCE hInst;						/* Instance handle */
HINSTANCE hPrevInst;					/* Previous Instance handle - not used */

HWND hwnd;								/* handle to Main Window - kept global */
HWND hInfoBox=NULL;						/* handle to DVIMetric dialog box */
HWND hFntBox=NULL;						/* handle to Fonts Used dialog box */
HWND hFindBox=NULL;						/* handle to Find Dialog box */
HWND hDlgPrint=NULL;					/* handle to abort spooling box */
HWND hConsoleWnd=NULL;					/* handle to console text window 99/Jun/25 */
HWND hClientWindow;						/* handle to Client Window */

HACCEL hAccTable;						/* handle to accelerator table */

HDC hPrintDC=NULL;						/* printer DC - keep around ? */
HDC hOldPrintDC=NULL;					/* saved printer DC */

HMENU hTeXMenu=NULL;					/* made global 1994/Nov/20 */
HMENU hEnvMenu=NULL;					/* made global 1998/Aug/28 */
HMENU hHelpMenu=NULL;					/* made global 1995/Sep/17 */

/* HDRVR hTIFFLibrary=NULL; */			/* handle of library if loaded */
/* HANDLE hTIFFLibrary=NULL; */			/* handle of library if loaded */
HINSTANCE hTIFFLibrary=NULL;			/* handle of library if loaded */
							/* or error code < 32 if tried and failed */

HBRUSH hOldBrush;						/* not accessed */

HBRUSH hbrBackground;					/* not accessed */
HBRUSH hbrDefaultBackground;			/* not accessed */

COLORREF clrBackground;

HBRUSH hGreyBrush;
HBRUSH hLightGreyBrush;
HBRUSH hBlackBrush;
HBRUSH hRuleBrush=NULL;
HBRUSH hRuleBrushDefault=NULL;

HPEN hBorderPen;
HPEN hLinerPen;
HPEN hFigurePen;
HPEN hErasePen;

HPEN hGreyPen;
HPEN hLightGreyPen;
HPEN hBlackPen;
HPEN hRulePen=NULL;
HPEN hRulePenDefault=NULL;

HRGN hRgn=NULL;		  /* place for update region when dragging image around */
HRGN hUpdateRgn=NULL; /* place for incremental update region */

HGLOBAL hFaceNames=NULL;			/* handle to memory for font names */ 
HGLOBAL hFullNames=NULL;			/* handle to memory for Full Names (TT) */ 
HGLOBAL hFileNames=NULL;			/* handle to memory for TFM file names */ 

HGLOBAL hWidths=NULL;				/* handle to memory for char widths */
HGLOBAL hPages=NULL;				/* handle to memory for page table */
HGLOBAL hBack=NULL;					/* handle to memory for page table */
HGLOBAL hColor=NULL;				/* handle to memory for color table */
HGLOBAL hTPIC=NULL;					/* handle to memory for TPIC path */

/* flushed hMarks 95/Aug/21 since presently not used for hyper-text */
/* HGLOBAL hMarks=NULL; */			/* handle to memory for hyptertext */

HGLOBAL hEncoding=NULL;	/* Global Encoding if ENCODING defined 94/Dec/25 */
						/* compressed ATM form - not used if bUseNewEncodeT1 */

#ifdef ATMSOLIDTABLE		
// If we use a single large (huge) table for these encoding data ...
HGLOBAL hATMTable=NULL;			// global ATM Data for Reencoding 94/Dec/25
char encodefont[MAXFONTS];		// non-zero if encoding already set up 
#else							
// If we use separate allocations for each encoding data set
HGLOBAL hATMTables[MAXFONTS];	// non-NULL if encoding already set up 
#endif


HGLOBAL hATMShow=NULL;	/* Global ATM Data for Show Fonts 94/Dec/25 */
						/* Contains 1552 bytes for ATM + encoding + width */

/* BOOL bUseControl=0; *//* use control points in ATM char BBox computation */

BOOL bTraceCurveto=1;	/* find extrema in curveto in WriteAFM... */

BOOL bCheckEncoding=1;	/* check TFM checksum encoding info in DVI 95/Jan/12 */
						/* also now controls BadANSI 97/Feb/18 */

BOOL bUseGetExtent=0;	/* GetExtent for char widths 95/Nov/5 */

/* BOOL bViewExtras=1; */	/* View preview info only 95/Mar/28 */
BOOL bViewExtras=0;		/* View preview info only 98/Sep/28 */

BOOL bFirstNull=1;		/* flush \special start with null 96/Aug/29 */

char srcfile[FILENAME_MAX];	/* reset to "" read from src special 98/Nov/12 */
int srclineno;				/* reset to zero read from src special 98/Nov/12 */
int srctaghit;				/* set when found src tag in search */

BOOL bMarkSearch=0;			/* +1 if searching for mark */
							/* +2 if searching for src */

/* BOOL bInverseSearch=0; */	/* non-zero in inverse search (calleditor) mode */

unsigned long nCheckSum=0;	/* expected TFM checksum */		/* 95/Feb/3 */
unsigned long nCheckANSI=0;	/* expected TFM checksum ansinew */	/* 95/Feb/3 */

BOOL bUseGetProfileATM=0;	/* use GetPrivateProfile on atm.ini 95/Feb/3 */
BOOL bUseGetProfileTT=1;	/* use GetPrivateProfile on win.ini 95/July/22 */
							/* in NT this means getting registry info */

BOOL bUseTTMapping=1;	/* Using new mapping table for TT - no more guessing */
						/* However the registry has mangled FontNames */
						/* while we need FullNames for this to work ... */

BOOL bUseBaseFont=1;	/* share metric info for same face and style */

BOOL bIgnoreRemapped=0; /* do not make special case of remapped fonts */

BOOL bUseATMFontInfo=1;		/* use ATMGetFontInfo */

BOOL bAllowVersion=1;	/* allow mismatch in last char of TT file name */

// BOOL bAllowTruncate=1;	/* allow for TFM file name truncation */
BOOL bAllowTruncate=0;	/* allow for TFM file name truncation 2000 July 4 */

int BBxll=0, BByll=0, BBxur=0, BByur=0; 	/* crop \special 96/May/4 */

/* In WIN32 make following LPSHORT instead ??? */

/* LPINT lpWidths; */		/* FAR pointer to char widths table all fonts */
/* LPINT lpCharWidths; */	/* FAR pointer to width table for current font */
/* LPINT lpCharTemp; */		/* FAR pointer to width table for new font */

typedef short int far   *LPSHORT;

LPSHORT lpWidths;		/* FAR pointer to char widths table all fonts */
LPSHORT lpCharWidths;	/* FAR pointer to width table for current font */
LPSHORT lpCharTemp;		/* FAR pointer to width table for new font */

LPLONG lpPages;			/* FAR pointer to page table for current file */
COLORREF FAR *lpBack;	/* FAR pointer to background color table */
COLORREF FAR **lpColor;		/* FAR pointer to color table for current file */
POINT FAR *lpTPIC;		/* FAR pointer to TPIC path table */

/* LPSTR lpApplications; */	/* FAR pointer to string from [Applications] */

/* char *phone="(978) 371-3286"; */	/* not used */

char *TitleText="DVIWindo";				/* normal Window title */

char *version = VERSION;

/* Copyright © 1991--1999 Y&Y, Inc. All rights reserved. (978) 371-3286 */
char *copyright="\
Copyright © 1991--2000 Y&Y, Inc. All rights reserved. http://www.YandY.com\
"; 
#define COPYHASH 6597019

char *szControlName = "DVI Spin";		 /* renamed 93/Sep/8 */
char *szControlNameAlt = "AI Bug!";		/*  alternate name (NOT longer) */

char *DviWindoClass="DviWindo32";		/* normal Window class */

char *DviWindoMenu= "DviWindoMenu";		/* normal Window menu */

char *szApplication="DVIWindo";			/* used also winanal.c */
char *szTopic="SRCSpecial";				/* used also winanal.c */

char searchtext[MAXSEARCHTEXT]="";		/* place for search text */

/********************************************************************************/

// char IniDefPath[MAXDIRLEN] = "";		/* working directory -- ends in \\ */

char *IniDefPath=NULL;					/* working directory if specified */

/* Following used by File Open Dialog */

char OpenName[MAXDIRLEN] = "";			/* copied from IDC_EDIT box */

char DefPath[MAXDIRLEN] = "";			/* default path from DVIPATH */
										/* updated from latest file opened */

char DefSpec[MAXDIRLEN] = "*.dvi";		/* default file specification */
										/* appears in edit dialog box */
										/* seems to be used for file name ? */

char DefExt[MAXDIRLEN] = ".dvi";		/* default extension, includes . */
										/* updated from latest file opened */

char SourceOpenName[MAXDIRLEN]="";		/* for calling app - space ? */

char SourceDefPath[MAXDIRLEN]="";		/* default path for application */

char SourceDefSpec[MAXDIRLEN]="*.tex";	/* default file specification */

char SourceDefExt[MAXDIRLEN]=".tex";	/* default extension */

char FileName[MAXFILENAME]="";		/* used by _lopen */
									/* ReOpenFile, WM_TIMER DoOpenFile */
									/* constructed name of special file */

char szHyperFile[MAXFILENAME]="";	/* temporary use Hyper Text file jump */
									/* temporary use Print to File */ 

/* make following two global to allow for split command strings */

char *szSource=NULL;			/* source file name to search for */
int nSourceLine=0;				/* line number to search for */
int nPageNum=0;					/* page number from command line */

char *ssrc=NULL;				/* source file in search call */
char *sdvi=NULL;				/* dvi file in search call (and read or print) */

char *szPrintFile=NULL;			/* allocate and free to save space */

BOOL bUseFakeFont=1;			/* use fake font (Times) for missing fonts */

/* following are fake fonts to use if a font is not found */

int fakeindex = -1;		/* index into following table of fake font names */

int fakettf=0;			/* is fake font TrueType or Type 1 1995/August/2 */

/* char *FakeFonts[] = {
	"Times Roman", "Times New Roman PS", "Times New Roman", "Times",
		"Arial MT", "Gill Sans", "Arial", "Helvetica", "Helv",
			"Courier", "Courier New", ""
}; */

/*	List the Type 1 fonts first if possible */
/*	List seriffed before sans serif */
/*  List variable width before fixed width */

/*	This is used in winspeci.c for CaptionFonts also, starting at 4 */

/*  Changed `Times Roman' to `Times' (actual Windows Face Name) 96/Apr/30 */

char *FakeFonts[] = {
/*	"Times Roman", "Times New Roman PS", */
	"Times", "Times New Roman PS",
		"Times New Roman", "Tms Rmn",
			"Helvetica", "Gill Sans", "Arial MT", 
				"Arial", "Helv",
					"Courier", "Courier New", ""
};

char *textext[32] = {							/* 1995/Jun/23 */
"Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon", 
"Phi", "Psi", "Omega", "ff", "fi", "fl", "ffi", "ffl",
"dotlessi", "dotlessj", "grave", "acute", "caron", "breve", "macron", "ring", 
"cedilla", "germandbls", "ae", "oe", "oslash", "AE", "OE", "Oslash"
};

/* following set from GetClipBox in HSCROLL & VSCROLL - PIXELS */

int width = 1000;
int height = 700;

/* WORD width = 1000; */
/* WORD height = 700; */

UINT uFindReplaceMsg = 0;	/* registered window message for FindText */

/* UINT uFileOKMsg = 0; */	/* registered window message for GetOpenFileName */

BOOL bWrapFlag = 1;			/* permit wrap around at end of file in search */
BOOL bCaseSensitive = 0;	/* non-zero for case sensitive search */

BOOL bStripPath = 0;		/* strip path from ? menu call 96/Oct/4 */

BOOL bNoScriptSel = 0;		/* CD_NOSCRIPTSEL winfonts.c 96/Oct/28 */

BOOL bAutoActivate = 1;		/* use ATMREG.ATM if possible 96/June/29 default */
							/* turned off if ATM loaded and version < 4.0 */
							/* turn off in Windows NT ? */

BOOL bUseBase13 = 1;		/* Use hard wired info on Base 13 fonts */

BOOL bTryATMRegFlag=1;		/* default for now 98/Nov/12 */

/* default increased from 1 => 2 in WIN32 until ATM works ... 95/Nov/5 */

int bUseATMINI = 2;		/* use information from ATM.INI for Type 1 */
int bUseTTFINI = 2;		/* use information from WIN.INI for TrueTypes */

/* int bUseATMINI = 1; */
/* int bUseTTFINI = 1; */

int bUseUSRINI = 1;		/* use substitution file <filename>.fnt */
int bUseTEXINI = 1;		/* use generic substitution dviwindo.fnt */

BOOL bATMBugFlag=1;			/* non-zero => need to correct for ATM bug */
							/* remap so old ATM can handle double encoding */
							/* Remap=2 needed for TrueType CM fonts */
BOOL bANSITeXFlag=1;		/* remap low part of ANSI encoded fonts */

/* BOOL bTeXANSI=0; */		/* value of TEXANSI env var - default 0 */
BOOL bTeXANSI=0;			/* value of ANSIBITS env var - default 0 */
/* if ENCODING=ansinew and bTeXANSI=1 actually reencode to ansinew 96/Aug/28 */
/* which will cause bitmapped partial fonts in printed output 96/Aug/28 */
/* otherwise ENCODING=ansinew will mean *no* reencoding right ? */

BOOL bTTUsable=0;			/* Is TrueType Enabled (and fonts installed) */

BOOL bTTFontSection=1;		/* allow for [TTFonts] as well as [Fonts] */
							/* Actually that is now preferred method */

BOOL bUseATM = 1;			/* try and connect up to ATM */
BOOL bATMPrefer = 1;		/* try and Force Select ATM font */
							/* off in Windows NT since no ATMSelectObject */
BOOL bATMExactWidth = 1;	/* try and Force ExactWidth for ATM font */
BOOL bATMLoaded = 0;		/* if loaded properly and links setup */
							/* via GetProcAddress in WIN16 or library WIN32 */
BOOL bATMReencoded = 0;		/* non-zero when ATM in reencoded state */
/* HANDLE hATM=NULL; */		/* ATM DLL if loaded */
/* HMODULE hATM=NULL; */	/* ATM DLL if loaded 95/Mar/31 */
#ifdef USEUNICODE
BOOL bFontEncoded = 0;		/* non-zero if current font is reencoded */
							/* using the new method */
#endif

BOOL bTeXFontMap = 1;		/* allow use of `texfonts.map' for aliasing */

HCURSOR hArrow;						/* handle to normal cursor */
HCURSOR hHourGlass;					/* handle to hourglass cursor	*/
HCURSOR hSaveCursor;				/* current saved cursor handle/paintpage */
HCURSOR hOldCursor;					/* current saved cursor handle/ruler	*/

HCURSOR hHandCursor=NULL;			/* handle to hand cursor */
HCURSOR hZoomCursor=NULL;			/* handle to magnifier cursor */
HCURSOR hCopyCursor=NULL;			/* handle to magnifier cursor */
HCURSOR hBlankCursor=NULL;			/* handle to ruler cursor */

long nFileLength;					/* length of file (up to post) */

BOOL bPreferKnown = 0;				/* preferences read when we came in */
BOOL bSavePrefer = 1;				/* save preferences when exiting */
BOOL bFlipSave = 0;					/* flip saving if shift exit */
BOOL bFactory = 0;					/* bSafePref reset by Factory Default */

BOOL bShowBorder = 1;				/* draw page border */
BOOL bShowLiner = 1;				/* draw text outline */
BOOL bResetScale = 0;				/* reset scale each file open */
BOOL bResetPage = 1;				/* reset scale each new page */
									/* flipped the above 1993/Feb/7 */
BOOL bUseNewPageOffset = 1;			/* use new page yoffset */
BOOL bComplainMissing = 1;			/* complain about missing fonts */
BOOL bComplainSpecial = 1;			/* complain about bad \special's */
BOOL bComplainFiles = 1;			/* complain about missing EPS files */
BOOL bIgnoreSpecial = 0;			/* ignore all \special's */
BOOL bShowBoxes = 1;				/* show character boxes */
BOOL bGreyText = 0;					/* `greek' text */
BOOL bGreyPlus = 0;					/* `greek' text plus text */
BOOL bCarryColor = 1;				/* carry color across page boundary */
									/* assumed to be on - affects PageCount[] also */
BOOL bAllowClip = 1;				/* allow clipping push and pop */
#ifdef IGNORED
BOOL bPrintFrame = 0;				/* print frames (for alignment) */
#endif

BOOL bUseDVIPSONE = 0;				/* Use DVIPSONE for printing */
BOOL bRunMinimized = 0;				/* Run DVIPSONE minimized */
BOOL bPrintToFile = 0;				/* Have DVIPSONE print to file */
BOOL bIgnoreLigatures=1;			/* in search, step over ligatures */
BOOL bIgnoreFontCase=1;				/* ignore case of Windows Face name */
									/* new default value 95/Jan/12 */

BOOL bCollateFlag = 0;				/* Collate multiple copy output */
BOOL bUseDevModeCopies = 1;			/* Only copies/collate if driver support */
BOOL bUseSharedName = 1;			/* Use `shared name' for Ne00: etc */

HWND hWidth=NULL, hHeight=NULL;		/* windows for ruler width and height */

BOOL bAltHomeEnd=1;					/* alternate use of Home and End keys */
									/* for top and bottom of page 98/Jun/30 */

BOOL bLandScape = 0;				/* 1 => landscape mode 0 => portrait */

int units = 1;						/* units to measure with */
									/* see table in winspeci.c */

/* May be able to keep file open, since (20 - 5) = 15 handles per job */
/* and 240 total available */
/* but this stops TeX from making new DVI file */
/* maybe close file when losing focus ? */

BOOL bKeepFileOpen = 0;				/* keep file open - 1 untested */

/* Maybe not keep around to avoid tying up a DC ? */
/* But it does slow things down when it has to be recreated ... */

BOOL bKeepPrinterDC = 0;			/* keep printer DC open - */
BOOL bKeepDEVMODE = 0;				/* keep DEVMODE when opening new printer */

/* BOOL bKeepPrinterDC = 1; */
/* BOOL bKeepDEVMODE = 1; */

BOOL bFileValid = 0;				/* non-zero if connected to file */
BOOL bFontSample = 0;				/* non-zero if want to show font sample */
BOOL bCharacterSample = 0;			/* non-zero if want character sample */
									/* (implies bFontSample is also on) */
// UINT nChar=-1;					/* which character to show */
int nChar=-1;						/* which character to show */
BOOL bShowWidths = 0;				/* non-zero if want character widths */
BOOL bWriteAFM = 0;					/* 1 if want AFM file */
									/* 2 if want AFM *and* TFM file */

BOOL bCountZero=0;			/* non-zero => page numbers are TeX count[0] */
BOOL bSpreadFlag=0;			/* non-zero => show spread */

BOOL bColorFont = 0;		/* non-zero => use different colors */
BOOL bWorkingExists = 0;	/* does working directory exist ? 95/July/7 */

/* Next few are really just compile time constants at this point ! */

BOOL bTextOutFlag = 1;				/* show text using TextOut */
BOOL bSnapToFlag = 1;				/* non-zero means snapto rules */
/* BOOL bDVITypeFlag = 0; */		/* non-zero means modify spacing OLD */
									/* negative => don't make small moves */
									/* bDVITypeFlag always 0 */
/* BOOL bDVIFakeFlag = 0; */		/* pseudo DVITYPE  (above subsumes) */
BOOL bDrawVisible = 1;				/* non-zero means refresh only visible */
									/* otherwise spew out all */

/* BOOL bUseCharSpacing=1; */		/* Default changed 94/Feb/18 */
/* BOOL bUseCharSpacing=0; */		/* Default changed back 95/Feb/19 */
BOOL bUseCharSpacing=1;				/* non-zero use accurate spacing */
									/* `Favor Position' IDM_STRINGLIMIT */
BOOL bForceCharSpacing=0;			/* non-zero to force in COPY and PRINT */

BOOL bClipRules=1;					/* clip rules as well 95/Aug/26 */

BOOL bFixZeroWidth=1;				/* fix zero widths RectVisible problem */

int MinWidth = 2 * 20;	/* safe minimum RectVisible width in TWIPS 95/Sep/3 */

BOOL bShowInfoFlag = 0;				/* show DVI file info in window */
									/* (but only if connected to file) */
BOOL bShowInfoExposed = 0;			/* showinfo window is not hidden */
									/* (hidden if not connected to file) */

BOOL bShowUsedFlag = 0;				/* show DVI file fonts in window */
									/* (but only if connected to file) */
BOOL bShowUsedExposed = 0;			/* usedfonts window is not hidden */
									/* (hidden if not connected to file) */

BOOL bShowSearchFlag = 0;			/* show modeless Search Dialog Box */

BOOL bShowSearchExposed = 0;		/* (hidden if not connected to file) */

int nMapMode=MM_ANISOTROPIC;		/* an experiment 96/Aug/8 remove? */

BOOL bShiftFlag = 0;				/* non-zero when shift key depressed */

BOOL bControlFlag = 0;				/* non-zero when control key depressed */

BOOL bEnableTermination = 0;		/* permit early termination */

BOOL bWasIconic = 0;				/* prevent double repaint when restore */

BOOL bMaximized = 0;				/* Window is maximized *//* 97/Jul/25 */
BOOL bMaximInit = 0;				/* read from dviwindo.ini */

BOOL bMaxDriftFlag = 1;				/* non-zero implement max drift rule */

BOOL bBusyFlag = 0;					/* non-zero while reading page */
BOOL bHourFlag = 0;					/* non-zero while hourglass displayed */
BOOL bPrintFlag = 0;				/* non-zero when printing */
BOOL bCopyFlag = 0;					/* non-zero when copying to ClipBoard */
BOOL bScanFlag = 0;					/* non-zero while scandvi... 95/Dec/21 */
BOOL bRectFlag = 0;					/* non-zero while setting rectangle */
BOOL bSearchFlag = 0;				/* non-zero while searching */

BOOL bUserAbort=0;					/* if WM_PAINT was interrupted */
									/* used by winprint and winsearc */
									/* and by winspeci.c ... */

BOOL bDisableSize = 0;				/* on to avoid writing rect size */

RECT UpdateRect;					/* rectangle needs update (PRINT/COPY) */

int dvipage = -INFINITY;			/* bop count from beginning */
									/* physical page count --- starts at 1 */

int gopage=0;						/* accumulated page number from keyboard */

BOOL bPageNumber=0;					/* non-zero if user has keyed something */

int nCopies=1;						/* number of copies to print */

int nDuplex=DMDUP_SIMPLEX;			/* Duplex flag from DevMode 96/Nov/17 */

int beginpage=-1;					/* set up by print only call */
int endpage=-1;						/* set up by print only call */

int bReversePages=0;				/* non-zero to reverse page order */

BOOL bOddEven=1;				/* use new odd/even page scheme 93/Aug/28 */

BOOL bOddOnly=0;				/* show odd pages only (if bOddEven != 0) */
BOOL bEvenOnly=0;				/* show even pages only (if bOddEven != 0) */

BOOL bAlternatePages=0;			/* alternate pages only (if bOddEven == 0) */
int pageincrement=1;			/* set up print only call (if bOddEven == 0) */

BOOL bAspectKeep=1;				/* maintain rectangle aspect ratio */

/* Following is used by color coded font code in winanal.c */
int nPaletteOffset=0;			/* offset to add to palette index */
int nMaxBright=183;				/* maximum grey tone accepted for colors */
int bColorStable=0;				/* use f instead of finx[f] in color font */

int nMaxDocuments=9;			/* how many listed in File Menu */

/* Following used if bSpreadFlag != 0 && bCountZero != 0 */

long leftcountzero = 0;	
long rightcountzero = 0;

long leftcurrent = 0;	
long rightcurrent = 0;

int leftdvipage = 0;	
int rightdvipage = 0;

BOOL bRuleFillFlag = 1;		/* fill rule rectangles */
/* int bUseRect=0; */		/* 1 => use FillRect & FrameRect */
							/* which don't work in MetaFile ... */	
int bUseRect=2;				/* use lines, which work in Acrobat PDF */
							/* new default 95/Jan/5 */
BOOL bAvoidZeroWidth = 1;	/* non-zero to avoid zero width rules */
BOOL bOffsetRule=0;			/* offset rule by 1/2 device pixel */
							/* not a good idea 98/Jan/12 */

BOOL bTestFlag=0;			/* use for debugging input */
BOOL bIgnoreSelect=0;		/* use for debugging input */
BOOL bPreserveSpacing=1;	/* non-zero means preserve spacing */
							/* zero means preserve shapes */

UINT uMSH_MOUSEWHEEL = 0;   // Value returned from RegisterWindowMessage() Windows 95 / NT 3.51

int StringLimit=8;			/* max length of contiguous bytes set <= MAXCHARBUF */

#ifdef ALLOWDEMO
BOOL bDemoFlag = 0;					/* non-zero for DEMO model */
									/* as determined by user string DEMO */
time_t dtime=0;						/* seconds since customized, now global */
#define oneday (86400)				/* one day in seconds */
#define onemonth (86400 * 30)		/* one month in seconds */
#endif

int papertype = DMPAPER_LETTER;		// paper size code

/* 0 reserved for custom size 2000 May 27 */
/* DMPAPER_LETTER = 1 */        /* Letter 8 1/2 x 11 in           */
/* DMPAPER_LETTERSMALL = 2 */        /*            */
/* DMPAPER_LEGAL = 5 */         /* Legal 8 1/2 x 14 in            */
/* DMPAPER_A4 = 9 */			/* A4 210 x 297 mm                */
/* DMPAPER_A4SMALL = 10 */			/*                 */
/* DMPAPER_B5 = 13 */           /* B5 176 x 250 mm ?              */

/* DMPAPER_TABLOID = 3 */		/* 11 x 17 */
/* DMPAPER_LEDGER = 4 */			/* 17 * 11 */
/* DMPAPER_STATEMENT = 6 */		/* 5 1/2 x 8 1/2 in */
/* DMPAPER_A3 = 8 */			/* 297 x 420 mm */
/* DMPAPER_A5 = 11 */			/* 148 x 210 mm */
/* DMPAPER_B4 = 12 */			/* 182 x 257 mm */

/* DMPAPER_FOLIO = 14 */
/* DMPAPER_QUARTO = 15 */

/* see DMPAPER_... 1 -- 41 in  print.h ---  or wingdi.h in WIN32 *//* Start of table of paper sizes */

char *papersize[] = {
	"custom",			// 2000 May 27 --- need to treat differently
	"letter", "lettersmall", "tabloid", "ledger", "legal", "statement",
	"executive", "a3", "a4", "a4small", "a5", "b4", "b5",
	"folio", "quarto", /* "10X14", "11X17", "note", */
	""
};

/* "", "letter", "lettersmall", "tabloid", "ledger", "legal", "statement",
"executive", "A3", "A4", "A4small", "A5", "B4", "B5", */

/* fixed 95/July/10 */ /* DVIPSONE / printer want lower case version */

/* Envelopes: Common #9 (19) Common #10 (20) Common #11 (21) */
/* Envelopes: DL (27) C5 (28) Monarch (37) */

int maxpapersize = sizeof(papersize) / sizeof(papersize[0]);

/* default page size for `letter' size in PS points (72 per inch) */

int PageWidth = 612;	/* 8.5 * 72 */
int PageHeight = 792;	/* 11 * 72 */

int GutterOffset = 0;	// allows adjusting gutter in spead mode 00/Jan/09

int CustomPageWidth = 0;	// 2000 May 27
int CustomPageHeight = 0;	// 2000 May 27

BOOL bPassSize=1;					/* pass paper size info 95/Jun/22 */
BOOL bPassOrient=1;					/* pass paper orientation info 95/Jun/24 */
BOOL bPassDuplex=1;					/* pass printer duplex info 96/Nov/17 */

int xLeft, yTop, cxWidth, cyHeight;	/* Main Window Position and Size */

int UsedxLeft=0, UsedyTop=0;		/* Fonts Used Window Position */

int InfoxLeft=0, InfoyTop=0;		/* DVI File Info Window Position */

int CxLeft=0, CyTop=0, CcxWidth=0, CcyHeight=0;	/* Console Window Position and Size */

int QxLeft=0, QyTop=0, QcxWidth=0, QcyHeight=0;	/* TeX Query Window Position and Size */

char *achFile=NULL;					/* Profile file name - dviwindo.ini */

char *achPr   = "Window";			/* Profile file section name */
char *achEnv  = "Environment";		/* Profile file section name */
char *achAp   = "Applications";		/* Profile file section name */
char *achDocs = "Documents";		/* Profile file section name */
char *achDiag = "Diagnostics";		/* Profile file section name */
char *achEsc = "Escapes";			/* Profile file section name */

char *szEncodingVector=NULL;	/*  place for encoding vector name */

char *szReencodingVector=NULL;	/*  ENCODING vector name from ENCODING=... */

char *szReencodingName=NULL;	/*  ptr into szReencodingVector minus path */

DWORD DVIFilterIndex = 1;		/* previously used filter index (DVI files) */

DWORD TeXFilterIndex = 1;		/* previously used filter index (TeX files) */

BOOL bUseMagicScreen = 0;		/* use nice half-tone screen */

int frompage, topage;			/* page range to print - physical pages */

int ScreenFrequency=0;	/* screen frequency times ten unless == 0 */
int ScreenAngle=32767;	/* screen angle times ten unless == 32767 */

/* MAXPRINTERNAME MAXDRIVERNAME MAXPORTNAME from winhead.h */

// can't have OLDPRINTDLG defined in WIN32...

#ifdef IGNORED
// #ifdef OLDPRINTDLG
/* Keep these names around globally when not using CommDlg */
/* char achPrinter[MAXPRINTERNAME]=""; */
char achDevice[MAXPRINTERNAME]="";
char achDriver[MAXDRIVERNAME]="";
char achPort[MAXPORTNAME]="";
#endif

PRINTDLG pd;						/* place for new CommDlg structure */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* some prototypes */

void resetinfodetails(void);
void setupremaptable(void);
int SeparateFile(LPSTR, LPSTR, LPSTR);
void ChangeDefExt(PSTR, PSTR);
void HideScrollBars(HWND);
void ShowEnvironment (void);
void CenterThumbs(HWND);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* HFILE hFile = -1; */		/* DOS file handle		*/
HFILE hFile = HFILE_ERROR;	/* DOS file handle		*/

HFONT hFontOld=NULL;		/* font when we came in */

HFONT hConsoleFont=NULL;	/* font used in console window */

#ifndef LONGNAMES
OFSTRUCT OfStruct;			/* information from OpenFile()	*/
							/* can't use with LONGNAMES !!! */	
#endif

DWORD NewFileSizeLow;		/* 32 low order bits of 64 bit FileSize */
DWORD NewFileSizeHigh;		/* 32 high order bits of 64 bit FileSize */
FILETIME NewFilewTime;		/* 64 bit .dwLowDateTime, .dwHighDateTime */
							/* 100 nano sec interval since 1601 Jan 1 */

/* struct _stat FileStatus; */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int dragstartx, dragstarty;			/* starting point WN_LBUTTONDOWN */
int dragcurrentx, dragcurrenty;		/* current point WM_MOUSEMOVE */
int dragendx, dragendy;				/* final point WM_LBUTTONUP */

/* WORD dragstartx, dragstarty;	*/
/* WORD dragcurrentx, dragcurrenty; */
/* WORD dragendx, dragendy; */

BOOL dragflag=0;					/* non-zero while dragging image around */

int zoomstartx, zoomstarty;		/* starting point WN_LBUTTONDOWN */
int zoomcurrentx, zoomcurrenty;	/* current point WN_MOUSEMOVE */
int zoompreviousx, zoompreviousy;	/* previous point WN_MOUSEMOVE */
int zoomendx, zoomendy;			/* final point WM_LBUTTONUP */
int zoomtemp;						/* for interchanging low and high */

/* WORD zoomstartx, zoomstarty;	 */
/* WORD zoomcurrentx, zoomcurrenty;	 */
/* WORD zoompreviousx, zoompreviousy;	 */
/* WORD zoomendx, zoomendy;	 */
/* WORD zoomtemp;	 */

BOOL bZoomFlag=0;					/* non-zero while defining zoom rect */
									/* negative after WM_ZOOM before drag */
									/* positive while dragging */

/* BOOL bClipFlag=0;	*/			/* non-zero while defining clip rect */
									/* neg after WM_CLIPBOARD before drag */
/* see bRectFlag ! */				/* positive while dragging */

BOOL bRuleFlag=0;					/* non-zero while drawing ruler */
									/* neg after WM_RULE before drag */
									/* positive while dragging */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;

	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	sprintf(debugstr, "EXE FILE CORRUPTED %ld\n", hash);	
	OutputDebugString(debugstr);	/* This will hang some systems !!! Good !!! */
	return hash;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static char *modname = "DVIWINDO"; 

/* static char *modname = __FILE__; */

static void winerror (char *message) {
	HWND hFocus;
	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox(hFocus, message, modname, MB_ICONSTOP | MB_OK);
}

/* static void wininfo(char *message) */
void wininfo (char *message) {
	HWND hFocus;
	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	(void) MessageBox(hFocus, message, modname, MB_ICONINFORMATION | MB_OK);
}

static int wincancel (char *buff) {				/* 1993/March/15 */
	HWND hFocus;
	int flag;

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, buff, modname, MB_ICONSTOP | MB_OKCANCEL);
	if (flag == IDCANCEL) return -1;
	else return 0;
}

int wincancelinfo (char *buff) {				/* 1993/March/15 */
	HWND hFocus;
	int flag;

	if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
	flag = MessageBox(hFocus, buff, modname, MB_ICONINFORMATION | MB_OKCANCEL);
	if (flag == IDCANCEL) return -1;
	else return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for encrypting and decrypting */

unsigned char decryptbyte (unsigned char cipher, unsigned int *crypter) {
	unsigned char plain;
/*	plain = (cipher ^  (unsigned char) (*crypter >> 8)); */
	plain = (unsigned char) ((cipher ^  (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned int)((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return plain;
}

/* 0 box when not customized when starting */
/* 1 IDM_SYSFLAGS */
/* 2 OutputDebugString when starting DVIWindo */

/* void notcustomized (void) */
/* void showconfiguration (int flag) */	/* Modified 1996/April/5 */
void ShowConfiguration (char *buffer) {	/* Modified 1996/Nov/7 */

	sprintf(buffer, "\
Win 95: %d, Win 98: %d, Win NT: %d, New Shell: %d, W2K: %d,\n\
ATM loaded: %d, TTF usable: %d\n",
			bWin95, bWin98, bWinNT, bNewShell, bWinNT5,
			bATMLoaded, bTTUsable);
}

/* sets up a string with video setup properties */

void showscreensize (char *buffer) {
	int nBitsPerPixel, nPlanes, nColorRes, nNumColors;
	int nLogicalX, nLogicalY, nHorzRes, nVertRes;
	HDC hDC;
	char *s;
	
	hDC = GetDC (hwnd);
	nBitsPerPixel = GetDeviceCaps (hDC, BITSPIXEL);
	nPlanes = GetDeviceCaps (hDC, PLANES);
	nColorRes = GetDeviceCaps (hDC, COLORRES);
	nNumColors = GetDeviceCaps (hDC, NUMCOLORS);
	nLogicalX = GetDeviceCaps (hDC, LOGPIXELSX);
	nLogicalY = GetDeviceCaps (hDC, LOGPIXELSY);
	nHorzRes = GetDeviceCaps (hDC, HORZRES);
	nVertRes = GetDeviceCaps (hDC, VERTRES);
	(void) ReleaseDC (hwnd, hDC);
	s = buffer;
	if (nNumColors < 0) nNumColors = 0;
	sprintf(s, "%d BitsPerPixel, %d Plane%s, %d ColorRes, %u %s,\n",
			nBitsPerPixel, nPlanes, (nPlanes > 1) ? "s" : "",
			nColorRes, nNumColors,
		   (nNumColors > 0) ? "Basic Palette" : "True Color");
	s = buffer + strlen(buffer);
	sprintf(s, "%d x %d logical dpi on %d x %d screen\n",
			nLogicalX, nLogicalY, nHorzRes, nVertRes);
}

/* used to have showowner here -> with About Dialog box code in winprint.c */
/* check encrypted owner tampering */
/* decrypted format is "Berthold K.P. Horn@100@1997 May 23 07:43:48\n" */

int checkowner (char *hex, char *buffer, int nlen) { 
	unsigned int cryptma = REEXEC;
	unsigned char e=0;	/* avoid uninitialized message 98/Mar/26 */
	int i, k;
	char *s=hex;
	char *t=buffer;		/* should be enough space there */
	
#ifdef DEBUGINIT
	if (bDebug > 1) OutputDebugString("Check owner\n");
#endif
/*	check first whether it is pre-customized version of executable */
	i = 0;						/* first check on pre-release version */
	for (k = 4; k < 32; k++) {	/* assumes pattern wavelength 4 */
		if (*(s+k) != *(s+k-4)) { 
			i = 1;
			break;
		}
	}
	if (i == 0) {				/* uncustomized */
		ShowConfiguration(str);
		strcat(str, "\n");
		showscreensize(str + strlen(str));
		*(str + strlen(str) - 1) = '\0';	/* trailing \n */
		wininfo(str);
		if (wincancel("SORRY: NOT CUSTOMIZED!") < 0) PostQuitMessage(0);
		if (getenv("CARLISLE") == NULL &&
			  getenv("CONCORD") == NULL &&
			  getenv("CAMBRIDGE") == NULL &&
			  getenv("CONWAY") == NULL)
			return -1;						/* kill it then */
		return 0;
	}
/*	modified 97/May/23 to allow Windows ANSI accented characters, */
/*	but also now disallows control characters, and checks signature */
	for (i = 0; i < 4; i++) {
		e = decryptbyte((unsigned char) *s++, &cryptma);
/*		if (e < 32 || e > 126) */
		if (e < 'a' || e > 'z') {		/* should be all lower case */
#ifdef DEBUGINIT
			if (bDebug > 1) OutputDebugString("Signature\n");
#endif
			return -1;					/* tampered with signature */
		}
	}
	for (i = 4; i < nlen; i++) {
		e = decryptbyte((unsigned char) *s++, &cryptma);
		*t++ = e;								/* assemble line */
		if (e == 0) break;
		if (e < 32) {
			if (e != 10 && e != 13 && e != 9) break;	/* tampered with ! */
		}
		else if (e > 127) {
			if (e < 192) {
				if (e != 138 && e != 140 && e != 154 && e != 156 && e != 159
					&& e != 145 && e != 146)
					break;					/* tampered with ! */
/*			138 (Scaron) 140 (OE) 154 (scaron) 156 (oe) 159 (Ydieresis) */
/*			145 (quoteleft) 146 (quoteright) */
			}
			else if (e == 215 || e == 247) break;	/* tampered with ! */
/*			215 (multiply) 247 (divide) */
		}
	}
	if (e != 0) *t++ = '\0';
#ifdef DEBUGINIT
	if (bDebug > 1) OutputDebugString(buffer);  /* show signature ??? */
#endif
	if (e != 0) return -1;
	if (strchr(buffer, '@') == NULL) return -1;			/* tampered with */
#ifdef ALLOWDEMO
/*	if (strstr(buffer, "DEMO") != NULL) bDemoFlag = 1; */
/*	else bDemoFlag = 0; */
#endif
	if ((s = strstr(buffer, "DEMO")) != NULL) {	/* 98/May/20 */
		if (sscanf(s+4, "%d", &bDemoFlag) == 0)	
			bDemoFlag = 1;
	}
	return 0;	/* seems ok ! */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	if (date == NULL) return;			/* sanity check */
	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

/* Split up default info line in dviwindo.ini into DefPath, DefSpec DefExt */
/* Typically Defaults=D:\working\ *.dvi .dvi
/* First item may be quote delimited "..." */

void splitupdefaults (char *buff, char *DefPath, char *DefSpec, char *DefExt) {
	int done=0;
	char *s, *t, *u;

//	bDebug = 2;				// debugging only !!!
#ifdef DEBUGOPEN
	if (bDebug > 1)	{
		OutputDebugString(buff);
//		wincancel(buff);	// debugging only
	}
#endif
	s = buff;
	if (*s == '\0') return;		/* nothing there ! */

	if (*s == '\"') {			/* handle quoted string here 97/Dec/10 */
		t = strchr(s+1, '\"');
		if (t != NULL) *t = '\0';	// flush second "
		strcpy(s, s+1);				// flush first "
//		s = s + strlen(s);
//		s += 2;
		s = t + 1;					// start of second field
		while (*s <= ' ' && *s > '\0') s++;		// over white space
	}
	else {	/*	Revised 1997/Dec/10 to look for "\ " for paths with spaces */
//		t = s + strlen(s) - 1;	// ???
		if ((s = strstr(s, "\\ ")) != NULL) s++;	/* 97/Dec/10 */
//		else if (*t == '\\' || *t == '/') s = t+1;	// ???
		else {						/* resort to the old way */
			s = buff;				// fixed 2000 June 17
			while (*s > ' ') s++; 	/* find first white space - the old way */
		}
		if (*s == '\0') done = 1;	/* last field ? */
		else *s++ = '\0';			/* mark end of first field */
	}
/*	at this point, s points at start of second field */
	t = buff;
/*	if (strlen(t) < MAXPATHLEN) strcpy(DefPath, t);	 */
/*	if (strlen(t) < sizeof(DefPath)) strcpy(DefPath, t); */
	if (strlen(t) < MAXDIRLEN) strcpy(DefPath, t);
	t = s;
/*  make sure it has a drive in it and ends with backslash */
	if (strchr(DefPath, ':') == NULL &&
		  strncmp(DefPath, "\\\\", 2) != 0)	// 2000 May 22
		DefPath[0] = '\0'; /* protection */
	else {									/* 1993/Dec/21 */
		u = DefPath + strlen(DefPath) - 1;
		if (*u != '\\' && *u != '/') strcat(DefPath, "\\");
	}
	if (bDebug > 1) {
		sprintf(debugstr, "DefPath '%s' s '%s' t '%s'",
			DefPath, s, t);
		OutputDebugString(debugstr);
//		wincancel(debugstr);		// debugging only
	}
//	temporary fix to avoid replication of bad DefPath 2000/March/15
//	if ((u = strstr(DefPath, "\\\\\\")) != NULL) *(u+1) = '\0';
	if (done) return;			/* there was only one field */
/*	at this point, s and t point at start of second field */
	while (*s > ' ') s++;		/* find next blank field separator */
	if (*s == '\0') done = 1;	/* last field ? */
	else *s++ = '\0';			/* mark end of second field */
/*	if (strlen(t) < MAXSPECLEN) strcpy(DefSpec, t); */
/*	if (strlen(t) < sizeof(DefSpec)) strcpy(DefSpec, t); */
	if (strlen(t) < MAXSPECLEN) strcpy(DefSpec, t);
	t = s;
	if (bDebug > 1) {
		sprintf(debugstr, "DefSpec '%s' s '%s' t '%s'",
			DefSpec, s, t);
		OutputDebugString(debugstr);
//		wincancel(debugstr);		// debugging only
	}
	*DefExt = '\0';		/* in case there is no default extension */
	if (done) return;
/*	at this point, s and t point at start of third field */
	while (*s > ' ') s++;		/* find next blank */
	if (*s == '\0') done = 1;	/* last field ? */
	else *s++ = '\0';			/* mark end of field */
/*	if (strlen(t) < MAXEXTLEN) strcpy(DefExt, t); */
/*	if (strlen(t) < sizeof(DefExt))	strcpy(DefExt, t); */
	if (strlen(t) < MAXEXTLEN) strcpy(DefExt, t);
/*	t = s; */
/*	make sure it starts with period */
	if (*DefExt != '.') {
		strcpy(DefExt, ".");
		strcat(DefExt, t);
	}
	t = s;		/* moved here 93/Dec/5 */
	if (bDebug > 1) {
		sprintf(debugstr, "DefExt '%s' s '%s' t '%s'",
			DefExt, s, t);
		OutputDebugString(debugstr);
//		wincancel(debugstr);		// debugging only
	}
	if (bDebug > 1) { 
		sprintf(debugstr, "DefPath '%s' DefSpec '%s' DefExt '%s'\n",
				DefPath, DefSpec, DefExt); 
		OutputDebugString(debugstr);
//		wincancel(debugstr);		// debugging only
	} 
/*	if (done != 0) return; */
}

/* check whether filename given corresponds to currently open DVI file */
/* try both quoted and unquoted version - ignore case, of course */
/* do we also need to bother about \ versus / ? hope not ... 98/Dec/12 */

int IsCurrentFile (char *FileName) {
	if (bFileValid == 0) return 0;
	strcpy(str, "\"");
	strcat(str, DefPath);
	strcat(str, DefSpec);
	strcat(str, "\"");
#ifdef DEBUGHYPER
	if (bDebug > 1) {
		OutputDebugString(str);
		OutputDebugString(FileName);
	}
#endif
	if (_strcmpi(str, FileName) == 0) return 1;
	strcpy(str, DefPath);
	strcat(str, DefSpec);
	if (_strcmpi(str, FileName) == 0) return 1;
	return 0;
}

void setpagesize (int type) {
	switch (type) {
									/* 0 */	 /* custom size - at end */
		case DMPAPER_LETTER:		/* 1 */  /* Letter 8 1/2 x 11 in               */
		case DMPAPER_LETTERSMALL:	/* 2 */  /* Letter Small 8 1/2 x 11 in      */
			PageWidth = 612;		/* 8.5 * 72 */
			PageHeight = 792;		/* 11 * 72 */		/* fixed 96/Sep/22 */
			break;
		case DMPAPER_TABLOID:		/* 3 */  /* Tabloid 11 x 17 in                 */
			PageWidth = 792;		/* 11 * 72 */
			PageHeight = 1224;		/* 17 * 72 */
			break;
		case DMPAPER_LEDGER:		/* 4 */  /* Ledger 17 x 11 in                  */
			PageWidth = 1224;		/* 17 * 72 */
			PageHeight = 792;		/* 11 * 72 */
			break;		
		case DMPAPER_LEGAL:			/* 5 */  /* Legal 8 1/2 x 14 in                */
			PageWidth = 612;		/* 8.5 * 72 */
			PageHeight = 1008;		/* 14 * 72 */
			break;
		case DMPAPER_STATEMENT:		/* 6 */  /* Statement 5 1/2 x 8 1/2 in         */
			PageWidth = 396;		/* 5.5 * 72 */
			PageHeight = 612;		/* 8.5 * 72 */
			break;
		case DMPAPER_EXECUTIVE:		/* 7 */  /* Executive 7 1/4 x 10 1/2 in        */
			PageWidth = 522;		/* 7.25 * 72 */
			PageHeight = 756;		/* 10.5 * 72 */
			break;
		case DMPAPER_A3:			/* 8 */  /* A3 297 x 420 mm                    */
			PageWidth = 842;		/* 297 * 72 / 25.4 */
			PageHeight = 1190;		/* 420 * 72 / 25.4 */
			break;
		case DMPAPER_A4:			/* 9 */  /* A4 210 x 297 mm                    */
		case DMPAPER_A4SMALL:		/* 10 */  /* A4 Small 210 x 297 mm              */
			PageWidth = 596;		/* 210/25.4 * 72 */
			PageHeight = 842;		/* 297/25.4 * 72 */
			break;
		case DMPAPER_A5:			/* 11 */  /* A5 148 x 210 mm                    */
			PageWidth = 420;		/* 148 * 72 / 25.4 */
			PageHeight = 596;		/* 210 * 72 / 25.4 */
			break;
		case DMPAPER_B4:			/* 12 */ /* B4 (JIS) 250 x 354                 */
			PageWidth = 709;		/* 250 * 72 / 25.4 */
			PageHeight = 1003;		/* 354 * 72 / 25.4 */
			break;
		case DMPAPER_B5:			/* 13 */ /* B5 (JIS) 182 x 257 mm              */
			PageWidth = 499;		/* 176/25.4 * 72 ? */
			PageHeight = 709;		/* 250/25.4 * 72 ? */
			break;
		case DMPAPER_FOLIO:			/* 14 */ /* Folio 8 1/2 x 13 in                */
			PageWidth = 612;
			PageHeight = 936;
			break;
		case DMPAPER_QUARTO:		/* 15 */ /* Quarto 215 x 275 mm                */
			PageWidth = 632;
			PageHeight = 780;
			break;
		case DMPAPER_LETTER + IDM_CUSTOMSIZE - IDM_LETTER:		/* 0 */ /* custom size */
			if (CustomPageWidth > 0 && CustomPageHeight > 0) {
				PageWidth = CustomPageWidth;
				PageHeight = CustomPageHeight;
			}
			break;
		default:					/* letter */
			PageWidth = 612;		/* 8.5 * 72 */
			PageHeight = 792;		/* 11 * 72 */
			break;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void setoffsetandzoom (int flag) {			/* 1996/May/12 */
	if (flag) {				/* reset to defaults if flag non-zero */ 
		if (bSpreadFlag == 0) {
			xoffset = pagexoffset;
			yoffset = pageyoffset;
			wantedzoom = pagewantedzoom;
			}
		else {
			xoffset = spreadxoffset;
			yoffset = spreadyoffset;
			wantedzoom = spreadwantedzoom;
		}
	}
	else {					/* switch page to/from spread if flag zero */ 
		if (bSpreadFlag == 0) {	/* switching from spread to normal */
			spreadxoffsetsvd = xoffset;			/* save spread state */
			spreadyoffsetsvd = yoffset;
			spreadwantedzoomsvd = wantedzoom;
			xoffset = pagexoffsetsvd;			/* restore normal state */
			yoffset = pageyoffsetsvd;
			wantedzoom = pagewantedzoomsvd;
		}
		else {					/* switching from normal to spread */
			pagexoffsetsvd = xoffset;			/* save normal state */
			pageyoffsetsvd = yoffset;
			pagewantedzoomsvd = wantedzoom;
			xoffset = spreadxoffsetsvd;			/* restore spread state */
			yoffset = spreadyoffsetsvd;
			wantedzoom = spreadwantedzoomsvd;
		}
	}
}

int sameoffsetandzoom (void) {			/* predicate 1996/May/12 */
	if (bSpreadFlag == 0) {
		if (xoffset == pagexoffset && yoffset == pageyoffset &&
			wantedzoom == pagewantedzoom) return 1;
		else return 0;
	}
	else {
		if (xoffset == spreadxoffset && yoffset == spreadyoffset &&
			wantedzoom == spreadwantedzoom) return 1;
		else return 0;
	}
}

/* include more ? */

void readpreferences(void) {	 /* read stuff from private profile file */
	int k;

	if (achFile == NULL) return;
	(void) GetPrivateProfileString(achPr, "Preferences", "",
		str, sizeof(str), achFile);
	if (*str != '\0')		/* only if there is something 94/Feb/18 */
		sscanf(str, 
"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\
 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\
 %d %d %d",
		   &bSpreadFlag, &bCountZero,       &bLandScape,	   &bGreyText,
		   &bGreyPlus,   &bColorFont,       &bRuleFillFlag,    &bShowBorder,
		   &bShowLiner,  &bResetScale,      &bShowBoxes,       &bCaseSensitive,
		   &bWrapFlag,   &bComplainMissing, &bComplainSpecial, &bIgnoreSpecial, 
		   &bResetPage,  &bUseDVIPSONE,     &bPrintToFile,     &bPassThrough,
		   &papertype,   &bUseCharSpacing,  &bShowButtons,     &bReversePages,
		   &bTrueInch,   &bComplainFiles,   &bShowPreview,     &bCheckEncoding,
		   &bViewExtras, &bDismissFlag,     &bShowViewPorts,   &bShowCalls,
		   &nDuplex,     &bShowTIFF,        &bShowWMF);

	(void) GetPrivateProfileString(achPr, "Preferences2", "",
								   str, sizeof(str), achFile);
	if (*str != '\0')		/* only if there is something 94/Feb/18 */
		sscanf(str, "%d %d %d %d %d %d %d %d %d %d",
			   &bCallBackPass, &bConsoleOpen, &bOpenCloseChannel, &bNewPassThrough,
			   &bForcePassBack, &bDontAskFile, &bOldPassThrough, &bUseDLLs, 
			   &bOpenClosePrinter, &bUseFontName);
			
/*	fixed 97/Feb/18 */
	if (bDebug == 1) bShowCalls = 1;	/* backward compatability ??? */
	if (bGreyText != 0) bGreyPlus = 0;	/* mutually exclusive */
	if (nDuplex == 0) nDuplex = DMDUP_SIMPLEX;
/*	if (papertype > MAXPAPERSIZE) papertype = MAXPAPERSIZE; */
	if (papertype >= maxpapersize) papertype = maxpapersize-1;

/*	get default page position and magnification */
	(void) GetPrivateProfileString(achPr, "Mapping", "",
		str, sizeof(str), achFile);
	sscanf(str, "%ld %ld %d", &pagexoffset, &pageyoffset, &pagewantedzoom); 
/*	set initial saved state for normal display mode 96/May/12 */
	pagexoffsetsvd = pagexoffset;
	pageyoffsetsvd = pageyoffset;
	pagewantedzoomsvd = pagewantedzoom;

/*	get default page position and magnification font showing */ /* 96/Aug/26 */
	(void) GetPrivateProfileString(achPr, "FontMapping", "",
		str, sizeof(str), achFile);
	sscanf(str, "%ld %ld %d", &fontxoffset, &fontyoffset, &fontzoom); 

/*	get default spread position and magnification 1996/May/12 */
	(void) GetPrivateProfileString(achPr, "SpreadMapping", "",
		str, sizeof(str), achFile);
	if (sscanf(str, "%ld %ld %d",
			   &spreadxoffset, &spreadyoffset, &spreadwantedzoom) < 3) {
		spreadxoffset = pagexoffset;
		spreadyoffset = pageyoffset;
		spreadwantedzoom = pagewantedzoom;
	}
/*	set initial saved state for spread display mode 1996/May/12 */
	spreadxoffsetsvd = spreadxoffset;
	spreadyoffsetsvd = spreadyoffset;
	spreadwantedzoomsvd = spreadwantedzoom;

	if (! bFirstInst) {			/* for secondary DVIWindo's ? 95/May/5 */
		(void) GetPrivateProfileString(achPr, "NotFirstMapping", "",
		str, sizeof(str), achFile);
		if (*str != '\0')
			sscanf(str, "%ld %ld %d",
				   &pagexoffset, &pageyoffset, &pagewantedzoom); 
	}
/*	xoffset = pagexoffset;
	yoffset = pageyoffset;
	wantedzoom = pagewantedzoom; */
	setoffsetandzoom(1);			/* 1996/May/12 */

	(void) GetPrivateProfileString(achPr, "Defaults", "",
		str, sizeof(str), achFile);
	if (*str != '\0') splitupdefaults(str, DefPath, DefSpec, DefExt);
	(void) GetPrivateProfileString(achPr, "FileName", "",
		str, sizeof(str), achFile);
/*	if (strlen(str) < MAXFILENAME) strcpy(OpenName, str); */
	if (strlen(str) < sizeof(OpenName)) strcpy(OpenName, str);
	(void) GetPrivateProfileString(achPr, "SourceFileName", "",
		SourceOpenName, sizeof(SourceOpenName), achFile);	/* 93/Dec/8 */
	(void) GetPrivateProfileString(achPr, "SourceDefaults", "",
		str, sizeof(str), achFile);
	if (*str != '\0') splitupdefaults(str,
		SourceDefPath, SourceDefSpec, SourceDefExt);
/*  worry about exceeding MAXFACENAME limits ? */
	(void) GetPrivateProfileString(achPr, "Font", "",
		TestFont, sizeof(TestFont), achFile);
/*  Font Size used in Sample Font Display */			/* 95/Jan/10 */
	TestSize = GetPrivateProfileInt(achPr, "TestSize", 20 * 20, achFile);
/*	Retrieve Filter Indeces for DVI and TeX File Open Dialogs 95/July/6 */
	GetPrivateProfileString(achPr, "Filters", "", str, sizeof(str), achFile);
	if (*str != '\0')
		sscanf (str, "%lu %lu", &DVIFilterIndex, &TeXFilterIndex);
/*	if (DVIFilterIndex > 3) DVIFilterIndex = 1; */
	if (DVIFilterIndex > 4) DVIFilterIndex = 1;
	if (TeXFilterIndex > 5) TeXFilterIndex = 1;

	(void) GetPrivateProfileString(achPr, "AuxWinPos", "",
		str, sizeof(str), achFile);
	if (*str != '\0')					/* 94/Mar/21 */
		sscanf (str, "%d %d %d %d", &UsedxLeft, &UsedyTop, &InfoxLeft, &InfoyTop);

	(void) GetPrivateProfileString(achPr, "ConsoleSize", "",
								   str, sizeof(str), achFile);
	if (*str != '\0')
		sscanf (str, "%d %d %d %d", &CxLeft, &CyTop, &CcxWidth, &CcyHeight);

	(void) GetPrivateProfileString(achPr, "QuerySize", "",
								   str, sizeof(str), achFile);
	if (*str != '\0')
		sscanf (str, "%d %d %d %d", &QxLeft, &QyTop, &QcxWidth, &QcyHeight);

/*	read units for ruler */
	(void) GetPrivateProfileString(achPr, "Units", "pt",
		str, sizeof(str), achFile);
	for (k = 0; k < 32; k++) {
		if (strcmp(unitnames[k], "") == 0) {
			units = 1; break;
		}				/* default - pt */
		if (strcmp(unitnames[k], str) == 0) {
			units = k; break;
		}
	}
	if (units == 9) pcptflag = 1;
	else pcptflag = 0;

	bPauseCalls = GetPrivateProfileInt(achEnv, "DEBUGPAUSE", bPauseCalls, achFile);

	bPreferKnown = 1;
}

// returns 0 if working directory exists
// now returns 1 if working directory exists

int CheckWorking(char *WorkDir) {		/* check working directory */
	int flag=0;
	UINT DriveType;
	char drive[MAXPATHLEN];
	char *s;

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("Check WorkingDirectory\n");
#endif

	if ((s = strchr(WorkDir, ':')) != NULL) {
		strncpy(drive, WorkDir, s - WorkDir + 1);
		*(drive + (s - WorkDir + 1)) = '\0';
		strcat(drive, "\\");
		DriveType = GetDriveType (drive);
		if (DriveType == 0) flag = 1;		/* no drive installed there */		
	}

/*	if (*(WorkDir+1) == ':') {
		c = *WorkDir;
		if (c >= 'a' && c <= 'z') c = c - 'a';
		else if (c >= 'A' && c <= 'Z') c = c - 'A';
		DriveType = GetDriveType (c);
		if (DriveType == 0) flag = 1;
	} */

/*	else winerror("Specify Drive in Working Directory"); */
	if (flag == 0) {
	    if (ChangeDirectory(WorkDir, 0) != 0) flag = 1;
	}
	if (flag) {
	    sprintf(str, "Working Directory `%s' not valid", WorkDir);
	    winerror(str);
	}
	return (flag == 0);
}

/* attempts to split into executable and command line args */
/* str contains executable null delimited, if any */
/* returns pointer to command line args (possible the whole thing) */

char *extractcall (char *str) {			/* 1995/June/26 */
	char *s, *t;

/*	dvipsone.bat -v -b=1 -e=3 for example */	
/*	d:\dvisourc\dvipsont for example */	
	if (*str == '-')
		return str;			/* command line flag, no executable */
	else if ((s = strchr(str, '.')) == NULL)
		return str;			/* is safety escape 95/Sep/12 */
	t = str;
	while (*t > ' ') t++;	/* to white space or end of string */
	if (s > t) return str;			/* if '.' comes after white space */
/*	if (s != NULL && s < t) */	/* does '.' occur before white space */
/*	if (s < t) */					/* does '.' occur before white space ? */
	if (*t != '\0') *t++ = '\0';	/* split into two parts at white space */
									/* careful if nothing follows ... */
#ifdef DEBUGEXTRACT
	if (bDebug > 1) { 
		sprintf(debugstr, "%s %s", str, t);
		OutputDebugString(debugstr);
	}
#endif		/* WARNING: TAKE OUT AGAIN! */
	return t;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* New way of organizing the reading of [Window] section of DVIWINDO.INI */

typedef struct {
	char *name;
	int *value;
} FLAGSTRUCT;

FLAGSTRUCT Flags[] = {
{"BGR",		&bBGRflag},		/* flip color order if it is an error */
{"Remap",	&bATMBugFlag},	/* turn off remapping / or forcing it always on */
{"Share",	&bUseTimer},	/*  provides a way of turning off timer */
{"PreventConflict",	&bPreventConflict},	/* Stop WM_PAINT while WinExec runs */
{"Offset",	&bUseNewPageOffset},	/* revert to old page top scheme */
{"UseRect",	&bUseRect},			/* use alternate rectangle filling */
{"ShowName",	&bShowFileName},	/* turn off EPSF/TIFF name if no preview */
{"ShowImage",	&bShowImage},	/* turn off reading of TIFF image file */
{"HyperText", &bHyperText},		/* turn off hyper text capability */
{"QuickLink",	&bQuickLink},	/* work around QuickLink II FAX/MODEM bug */
{"Screen",	&bUseMagicScreen},	/* force use of nice halftone screen */
{"TTRemap",	&bTTRemap},			/* remap 0 -- 31 in TrueType fonts */
{"TTOnly",	&bTrueTypeOnly},	/* allow only TrueType fonts */
{"T1Only",	&bTypeOneOnly},		/* allow only Adobe Type 1 fonts */
{"PatchBadTTF", &bPatchBadTTF},	/* `text' TTF fonts that aren't 98/Sep/25 */
{"Aspect",	&bAspectKeep},		/* zoom rectangle aspect ratio maintenance */
{"ANSITeX",	&bANSITeXFlag},		/* remapping of low end of ANSI fonts */
{"Escape",	&bEscapeEnable},	/* Escape key exits DVIWindo (use Alt-F4) */
{"Alternate",	&bOddEven},		/* select new odd/even alternate page print */
/* {"CommDlg",	&bAllowCommDlg}, */	/* use COMMDLG.DLL for main open dialog */
{"CommFind",	&bNewSearch},	/*	use CommDlg `Find' Dialog box */
{"TTFontSection",	&bTTFontSection},	/* look at [TTFonts] of WIN.INI */
{"IgnoreBadEncoding",	&bIgnoreBadANSI},	/* ignore check for bad ANSI */
{"IgnoreBadInstall",	&bIgnoreBadInstall}, /*	 ignore TT font name errors */
{"Deslash",	&bUnixify},			/* use slash instead backslash in TeX Menu */
{"CmdShow",	&nCmdShowForce},	/* -1 of CmdShow for WinExec 94/Mar/8 */
{"PaletteOffset",	&nPaletteOffset}, /* offset to palette index 94/Mar/9 */
{"MaxBright",	&nMaxBright},	/* max color bright in fonts 94/Mar/9 */
{"ColorStable",	&bColorStable},	/* color depends TeX font number 94/Mar/10 */
{"FontMap",	&bTeXFontMap},		/* allow use of texfonts.map alias 94/Mar/18 */
{"SmallStep",	&bSmallStep},	/* small magnification increments 94/Apr/19 */
{"SciWord",	&bSciWord},			/* Scientific Word Figure Inc 94/Apr/21 */
{"SnapTo",	&bSnapToFlag},		/* Snapto rule coordinates 94/July/6 */
{"RefreshDelay",	&nRefreshDelay}, /* control delay after change 94/Aug/10 */
{"ISOTROPIC",	&bISOTROPIC},		/* WMF output marked ISOTROPIC 94/Oct/4 */
{"IgnoreFontCase",	&bIgnoreFontCase},	/* Ignore font name case 94/Oct/29 */
{"IgnoreLigatures",	&bIgnoreLigatures},	/* Step over ligs search 94/Oct/29 */
{"ATMPrefer",	&bATMPrefer},			/* Prefer Type 1 fonts 94/Dec/22 */
{"ATMExactWidth",	&bATMExactWidth},	/* Force correct width 94/Dec/22 */
{"AvoidFlood",	&bAvoidFlood},			/* Avoid FloodFill 94/Dec/22 */
{"UseGetProfileATM",	&bUseGetProfileATM}, /* GetProfile ATM.INI 95/Jan/12 */
{"UseGetProfileTT",	&bUseGetProfileTT}, /* GetProfile WIN.INI 95/Jul/12 */
{"UseTTMapping",	&bUseTTMapping},	/* Use mapping table 95/Jul/12 */
{"AllowVersion",	&bAllowVersion},	/* last char mismatch 95/Jul/30 */
{"AllowTruncate",	&bAllowTruncate},	/* allow TFM name trunc 95/Aug/27 */
{"ClipRules",	&bClipRules},			/* Clip rules to Window 95/Aug/26 */
{"FixZeroWidth",	&bFixZeroWidth},	/* Zero Width RectVisible 95/Aug/30 */
{"UseATMINI",	&bUseATMINI},	/* Use ATM.INI to look for T1 fonts */
{"UseTTFINI",	&bUseTTFINI},	/* Use WIN.INI to look for TT fonts */
{"UseUSRINI",	&bUseUSRINI},	/* Use <filename>.FNT for fonts */
{"UseTEXINI",	&bUseTEXINI},	/* Use DVIWINDO.FNT for fonts */
{"PassSize",	&bPassSize},	/* Pass paper size to DVIPSONE 95/Jun/22 */
{"PassOrient",	&bPassOrient},	/* Pass landscape orient DVIPSONE 95/Jun/24 */
{"PassDuplex",	&bPassDuplex},	/* Pass duplex to DVIPSONE 96/Nov/17 */
{"TraceCurveto",	&bTraceCurveto}, /* Trace Curveto in MakeAFM 95/Jun/24 */
{"UseRegistry",	&bUseRegistryFile},	/* Use ttfonts.reg Windows 95 95/Aug/15 */
{"UseRegEnumValue",	&bUseRegEnumValue},	/* Use RegEnumValue Windows 95 */
{"StringLimit", &StringLimit},	/* Limit on string accumulation TextOut */
{"MagBase", &magbase},			/* base of magnification steps */
{"Frequency", &ScreenFrequency},	/* halftone screen frequency */
{"Angle", &ScreenAngle},			/* halftone screen angle */
{"MinWidth", &MinWidth},			/* min rect width RectVisible 95/Sep/3 */
{"ForceTIFF", &bForceTIFF},			/* force .tif file for .eps 95/Sep/7 */
{"MaxDocuments", &nMaxDocuments},	/* remembered files 95/Sep/17 */
{"HelpAtEnd", &bHelpAtEnd},			/* Help menu on right 95/Sep/17 */
{"UseGetExtent", &bUseGetExtent},	/* GetExtent for Char Widths 95/Nov/5 */
{"CommFont", &bCommFont},			/* use CommDlg `Font' Dialog box */
{"LongNames", &bLongNames},			/* show long names in dialog box */
{"KeepPrinterDC", &bKeepPrinterDC},	/* 1995/Dec/14 */
{"KeepDEVMODE", &bKeepDEVMODE},		/* 1996/July/7 */
{"UseDevModeCopies", &bUseDevModeCopies},	/* 1995/Dec/15 */
/* {"CommPrint",	&bCommPrint}, */ /* use CommDlg `Print' Dialog 96/Jan/7 */
/* {"UsePalette", &bUsePalette}, */		/* 1996/March/24 */
/* {"UseFakeFont", &bUseFakeFont}, */	/* 1996/March/31 */
{"DefaultTIFFDPI", &nDefaultTIFFDPI},	/* 1996/Apr/3 */
{"AutoActivate", &bAutoActivate},		/* 1996/May/28 may be overridden ? */
{"KeepZeroKern", &bKeepZero},			/* 1996/July/28 */
{"UseBaseFont", &bUseBaseFont},			/* 1996/July/30 */
{"RawSpecial", &bAllowAndrew},			/* 1996/Aug/22 */
{"CompressColor", &bCompressColor},		/* 1996/Sep/7 */
{"StretchGray", &bStretchGray},			/* 1996/Sep/15 */
{"StretchColor", &bStretchColor},		/* 1996/Sep/15 */
{"ATM4", &bATM4},						/* 1996/June/3 overridden ??? */
{"StripPath", &bStripPath},				/* 1996/Oct/4 */
{"NoScriptSel", &bNoScriptSel},			/* 1996/Oct/28 */
{"WarnOnce", &bT1WarnOnce},				/* 1996/Nov/29 */
{"NewShell", &bNewShell},				/* 1996/Nov/29 */
{"UseATMFontInfo", &bUseATMFontInfo},	/* 1996/July/28 */
#ifdef AllowCTM
{"AdvancedGraphics", &bUseAdvancedGraphics},	/* 1996/Nov/3 */
{"UseCTM", &bUseCTMflag},						/* 1996/Nov/6 */
#endif
#ifdef USEUNICODE
{"NewEncodeTT", &bUseNewEncodeTT},			/* 97/Jan/16 */
{"NewEncodeT1", &bUseNewEncodeT1},			/* 97/Jan/16 */
{"OldLigCodes", &bOldUnicode},				/* 97/Feb/16 */
#endif
{"Decorative",  &bDecorative},				/* 97/Feb/9 */
{"DontCare", &bDontCare},					/* 97/Feb/16 */
{"UseSharedName", &bUseSharedName},			/* 97/Feb/24 */
{"CopyInfo", &bInfoToClip},					/* 97/July/12 */
{"IgnoreRemapped", &bIgnoreRemapped},		/* 97/Sep/11 */
{"FontScale", &nMagicFact},					/* 97/Sep/14 */
{"UpperCase", &bUpperCase},					/* 97/Oct/21 */
{"AvoidZeroWidth", &bAvoidZeroWidth},		/* 98/Jan/12 */
{"OffsetRule", &bOffsetRule},				/* 98/Jan/12 */
{"CarryColor", &bCarryColor},				/* 98/Feb/14 */
{"FlipRotate", &bFlipRotate},				/* 98/Feb/28 */
{"AltHomeEnd", &bAltHomeEnd},				/* 98/Jun/30 */
{"AllowClip", &bAllowClip},					/* 98/Sep/10 */
/* {"LinkToEditor", &bSynchronicity}, */	/* 98/Nov/5 */
{"DDEServer", &bDDEServer},					/* 98/Dec/12 */
{"DrawOutline", &bDrawOutline},				/* 99/Jan/20 */
{"ForceCharSpacing", &bForceCharSpacing},	/* 99/Mar/4 */
{"ShowFullName", &bShowFullName},			/* 99/Apr/21 */
{"MultiInstance", &bMultiInstance},			/* 99/May/31 */
{"GutterOffset", &GutterOffset},			/* 00/Jan/09 */
{"UseBase13", &bUseBase13},					/* 00/May/24 */
{"QuoteAtSign", &bQuoteAtSign},				/* 00/May/27 */
{"ConvertUnderScores", &bConvertUnderScores},	/* 00/Jul/4 */
{"AllowFontNames", &bAllowFontNames},			/* 00/Jul/4 */
// {"AFMtoTFMDLL", &bUseAFMtoTFMDLL},		/* 99/June/13 */
// {"DVIPSONEDLL", &bUseDVIPSONEDLL},		/* 99/June/13 */
// {"YANDYTEXDLL", &bUseYandYTeXDLL},		/* 99/June/13 */
// {"CallBackPass", &bCallBackPass},		/* 99/July/20 */
// {"ConsoleOpen", &bConsoleOpen},			/* 99/July/23 */
{"", NULL}
};

/* {"RememberDocs", &bRememberDocs}, */	/* Remember files opened 95/Sep/17 */
/* {"AllowBadBox",	&bAllowBadBox},	*/	/* Allow % in %% comments 94/Aug/19 */
/* {"ShowViewPort",	&bShowViewPort}, */	/* Show WMF ViewPort 94/Oct/5 */
/* {"ShowMetaFile",	&bShowMetaFile}, */	/* show WMF insertions 94/Oct/1 */
/* {"ShowTIFF",	&bShowTIFF}, */	/* show TIFF insertimage: 97/Jan/5 */
/* {"MapMode", &nMapMode}, */					/* 1996/Aug/1 */

/* Read in PrivateProfileInt's using the above table */

void GetPrivateIntegers (void) {
	int k;
/*	int n; */
	
	if (achFile == NULL) return;
	for (k = 0; k < 1024; k++) {
		if (*Flags[k].name == '\0' || Flags[k].value == NULL) break;
		*Flags[k].value = (int) GetPrivateProfileInt(achPr,
							Flags[k].name, *Flags[k].value, achFile);
	}
}

void FreeCommandStrings(void) {

	if (szDVIPSONE != NULL) {
		free(szDVIPSONE);
		szDVIPSONE = NULL;
	}
	if (szDVIDistiller != NULL) {
		free(szDVIDistiller);
		szDVIDistiller = NULL;
	}
	if (szDVIPSONEcom != NULL) {
		free(szDVIPSONEcom);
		szDVIPSONEcom=NULL;
	}
	if (szAFMtoTFM != NULL) {
		free(szAFMtoTFM);
		szAFMtoTFM = NULL;
	}
	if (szAFMtoTFMcom != NULL) {
		free(szAFMtoTFMcom);
		szAFMtoTFMcom = NULL;
	}
	if (szYandYTeX != NULL) {
		free(szYandYTeX);
		szYandYTeX = NULL;
	}
	if (szYandYTeXcom != NULL) {
		free(szYandYTeXcom);
		szYandYTeXcom = NULL;
	}
}

/* May now be used to reset command line strings */
/* Separated out since can be changed from menu now 97/Apr/3 */

void SetCommandStrings (void) {
	char *t;

	FreeCommandStrings();
	if (achFile == NULL) return;

/*	Command line string to pass to DVIPSONE */
	GetPrivateProfileString(achPr, "DVIPSONE", "", str,
		sizeof(str), achFile);		/* 1994/Mar/7 */
/*	Provide for possibility of calling something other than "DVIPSONE" */
	if (*str != '\0') {
		t = extractcall (str);
		szDVIPSONE = zstrdup(t);					/* 95/July/15 */
		if (t > str) szDVIPSONEcom=zstrdup(str);	/* 95/July/15 */
	}
#ifdef DEBUGMAIN
	if (bDebug > 1 && szDVIPSONE != NULL) OutputDebugString(szDVIPSONE);
#endif

/*	Command line string to pass to DVIPSONE/Distiller */ /* 99/Dec/30 */
	GetPrivateProfileString(achPr, "DVIPSONE/Distiller", "", str,
							sizeof(str), achFile);
/*	Provide for possibility of calling something other than "DVIPSONE" */
	if (*str != '\0') {
		t = extractcall (str);
		szDVIDistiller = zstrdup(t);
//		if (t > str) szDVIPSONEcom=zstrdup(str);
	}
#ifdef DEBUGMAIN
	if (bDebug > 1 && szDVIDistiller != NULL) OutputDebugString(szDVIDistiller);
#endif

/*	Command line string to pass to AFMtoTFM */
	GetPrivateProfileString(achPr, "AFMtoTFM", "", str,
		sizeof(str), achFile);		/* 1995/June/26 */
/*	Provide for possibility of calling something other than "AFMtoTFM" */
	if (*str != '\0') {
		t = extractcall (str);
		szAFMtoTFM = zstrdup(t);
		if (t > str) szAFMtoTFMcom = zstrdup(str);			/* 95/July/15 */
	}
#ifdef DEBUGMAIN
	if (bDebug > 1 && szAFMtoTFM != NULL) OutputDebugString(szAFMtoTFM);
#endif

/*	Command line string to pass to TeX */
	GetPrivateProfileString(achPr, "TeX", "", str,
		sizeof(str), achFile);								/* 94/July/10 */
/*	Provide for possibility of calling something other than "TeX" ??? */
	if (*str != '\0') {
		t = extractcall (str);
		szYandYTeX = zstrdup(str);							/* 95/July/15 */
		if (t > str) szYandYTeXcom = zstrdup(str);
	}
#ifdef DEBUGMAIN
	if (bDebug > 1 && szYandYTeX != NULL) OutputDebugString(szYandYTeX);
#endif
}

/* Need to be able to refresh strings from env vars 98/Dec/24 */
/* Alternative: read all env vars right before use only */

void ReadEnvVars (void) {		/* 98/Dec/24 */
	int count;
	char *s, *t;

	if (achFile == NULL) return;	
/*	Information to communicate with editor via DDE */
/*	EditorDDE=Application;Topic;"+%d %s";... */
/*	Here ; replaced with null */ /*	Use %; to get ; */
	GetPrivateProfileString(achEnv, "EditorDDE", "", str,
							sizeof(str), achFile);		/* 1998/Nov/5 */
	count = 0;
	if (*str != '\0') {
		strcat(str, " ");			/* dummy replaced by null at the end */
		if (szEditorDDE != NULL) free(szEditorDDE);	/* 98/Dec/24 */
		szEditorDDE = zstrdup(str);
		s = szEditorDDE;
		while ((t = strchr(s, ';')) != NULL) {
			if (t == str || *(t-1) != '%') {	/* %; quotes ; */
				*t = '\0';			/* replace ; with null */
				count++;
			}
			t++;
			s = t;
		}
		s += strlen(s);
		*(s-1) = '\0';	/* replace dummy by null so there are two nulls */
	}
	if (count < 2) {		/* need Application,Topic,String */
		free(szEditorDDE);
		szEditorDDE = NULL;
	}

	GetPrivateProfileString(achEnv, "TeXEdit", "", str,
							sizeof(str), achFile);		/* 1998/Nov/5 */
	if (*str != '\0') {
		if (szTeXEdit != NULL) free(szTeXEdit);	/* 98/Dec/24 */
		szTeXEdit = zstrdup(str);
	}
//	CustomPageSize=612bp*792bp
//	GetPrivateProfileString(achEnv, "CustomPaperSize", "", str,
//							sizeof(str), achFile);		// 2000 May 27
//	if (*str != '\0') 
//		(void) decodepapersize(str, &CustomPageWidth, &CustomPageHeight);
//	non-zero values tell it there is a custom size defined
}

void ReadPaths (void) {
	if (achFile == NULL) return;
/*	YANDYPATH unlikely to change! */
	(void) GetPrivateProfileString(achEnv, "YANDYPATH", "",
								   str, sizeof(str), achFile);
	if (*str != '\0') {
		if (szBasePath != NULL) free(szBasePath);
		szBasePath = zstrdup(str);
	}
/*	DVIPATH rarely used ... */
	(void) GetPrivateProfileString(achEnv, "DVIPATH", "",
							   str, sizeof(str), achFile);	
/*	if (*str != '\0') DefPath = _strdup(str); */ /* NO */
	if (*str != '\0') {
		strncpy(DefPath, str, sizeof(DefPath)-2);
		strcat(DefPath, "\\");
	}
	(void) GetPrivateProfileString(achEnv, "VECPATH", "",
							   str, sizeof(str), achFile);	
	if (*str != '\0') {
		if (szVecPath != NULL) free(szVecPath);
		szVecPath = zstrdup(str);
	}
	(void) GetPrivateProfileString(achEnv, "PREPATH", "",
							   str, sizeof(str), achFile);	
	if (*str != '\0') {
		if (szPrePath != NULL) free(szPrePath);
		szPrePath = zstrdup(str);
	}
	(void) GetPrivateProfileString(achEnv, "PSPATH", "",
							   str, sizeof(str), achFile);	
	if (*str != '\0') {
		if (szEPSPath != NULL) free(szEPSPath);
		szEPSPath = zstrdup(str);
	}
	(void) GetPrivateProfileString(achEnv, "TEXDVI", "",
							   str, sizeof(str), achFile);
	if (*str != '\0') {
		if (szTeXDVI != NULL) free(szTeXDVI);
		szTeXDVI = zstrdup(str);
	}
/*	following moved elsewhere to allow for encoding changes */
/*	(void) GetPrivateProfileString(achEnv, "TEXFONTS", "",
			str, sizeof(str), achFile);	*/
/*	if (*str != '\0') TeXFonts = zstrdup(str); */
}

/* Read in those `preferences' controlled only from dviwindo.ini file */
/* (these come from the [Window] section of dviwindo.ini */

void ReadFixedPrefer (void) {		/* read those things that are not saved */
	char *s;
	int n;

	if (achFile == NULL) return;

//	Allow specification working directory (override when DVIWindo starts up) 
//	Use DVIWindo `Working Directory' in `Preview' from `TeX' menu 
	bUseWorkPath = 0;
	bUseSourcePath = 1;						/* default ??? */

	(void) GetPrivateProfileString(achPr, "WorkingDirectory", "",
		str, sizeof(str), achFile);
//	special case test indicating there is *no* WorkingDirectory 94/Jan/21 
	if (_stricmp(str, "nul") == 0 ||
		_stricmp(str, "null") == 0) *str = '\0';

//	Set `UseWorkPath' and reset `UseSourcePath' 
//	if (*str != '\0' && strlen(str) < sizeof(IniDefPath)) 
	if (*str != '\0') {
//		strcpy(IniDefPath, str);
		s = str + strlen(str) - 1;
		if (*s != '\\' && *s != '/') strcat(s, "\\");
//		bUseWorkPath = 1; bUseSourcePath = 0; 
//		CheckWorking(IniDefPath);
//		if (CheckWorking(s) == 0) {		/* 95/Mar/18 */
		if (CheckWorking(str)) {		/* 95/Mar/18 */
			if (IniDefPath != NULL) free(IniDefPath);
			IniDefPath = zstrdup(str);
			bUseWorkPath = 1;
			bUseSourcePath = 0; 
			bWorkingExists = 1;
		}
	}

/*	allow choice of colors for borders */
	(void) GetPrivateProfileString(achPr, "BorderColor", "", str,
								   sizeof(str), achFile); 
	sscanf (str, "%d %d %d", &RBorderPen, &GBorderPen, &BBorderPen); 

/*	allow choice of colors for liners */
	(void) GetPrivateProfileString(achPr, "LinerColor", "", str,
								   sizeof(str), achFile); 
	sscanf (str, "%d %d %d", &RLinerPen, &GLinerPen, &BLinerPen); 

/*	allow choice of colors for figures */
	(void) GetPrivateProfileString(achPr, "FigureColor", "", str,
								   sizeof(str), achFile);
	sscanf (str, "%d %d %d", &RFigurePen, &GFigurePen, &BFigurePen); 

/*	Allow user to customize hot key for Preview */ /* 1994/Jan/22 */
	GetPrivateProfileString(achPr, "PreviewHotKey", szPreviewHotKey,
		szPreviewHotKey, sizeof(szPreviewHotKey), achFile);

/*	SetCommandStrings() */	/* used to be inline here */

/*	Specify name of registry dump file (Windows 95) */
	GetPrivateProfileString(achPr, "RegistryFile", "", str,
							sizeof(str), achFile);			/* 95/Aug/1 */
/*	If user specifies a file name for the registry file, ... */
/*  then always call RegEdit ahead of time (not after TT fonts not found) */
	if (*str != '\0') {
		szRegistry = zstrdup(str);	/* 95/Aug/1 */
/*		*IF* user specified file to write to 95/Aug/18 */
/*		always write ttfonts.reg ahead of time */
/*		if (bWin95) bAlwaysWriteReg = 1; */
		if (bNewShell) {
			bAlwaysWriteReg = 1;		/* bNewShell 96/Oct/2 */
			bUseRegistryFile = 1;		/* 96/Nov/30 */
		}
	}
/*	else if (bWin95) szRegistry="ttfonts.reg"; */	/* default 95/Aug/16 */
	else if (bNewShell)
/*		szRegistry="ttfonts.reg"; */	/* default 95/Aug/16 */
		szRegistry = zstrdup("ttfonts.reg");	/* 98/Dec/25 */

/*	if (*szRegistry == '\0') *//* don't try and use registry file */
	if (szRegistry == NULL)	
		bUseRegistryFile = 0;			/* if there is no file name */

/*	Specify what ports `Use DVIPSONE' should be checked for */
	GetPrivateProfileString(achPr, "UseDVIPSONE", "", str,
		sizeof(str), achFile);
	if (*str == '\0') {
		GetPrivateProfileString(achPr, "DVIPSONEport", "", str,
			sizeof(str), achFile);
	}
	if (*str != '\0') {
/*		if it's not zero or one, it must be a port name */
		if (sscanf(str, "%d", &bUseDVIPSONE) == 0) {
/*			szDVIPSONEport = _strdup(str); */	/* 94/June/3 */
			if (szDVIPSONEport != NULL) free(szDVIPSONEport);
			szDVIPSONEport = zstrdup(str);		/* 95/July/15 */
			if (strstr(szDVIPSONEport, "BOGUS") != NULL)
				bUseDVIPSONE = 0;				/* 97/May/3 */
			else bUseDVIPSONE = 1;
		}
	}
#ifdef DEBUGEXTRACT
/*	if (bUseDVIPSONE != 0) */
	if (bUseDVIPSONE != 0 && szDVIPSONEport != NULL) {
		if (bDebug > 1) { 
			sprintf(debugstr, "szDVIPSONEport=%s", szDVIPSONEport);
			OutputDebugString(debugstr);
		} 
	}  	/* WARNING: TAKE OUT AGAIN! */
#endif

/*	ReadEnvVars(); */					/* split off 98/Dec/24 */

/*	Control mode in which OpenFile happens */
/*	0 COMPAT, 1 EXCLUSIVE, 2 DENY_WRITE, 3 DENY_READ, 4 DENY_NONE */
	n = (int) GetPrivateProfileInt(achPr, "OpenCode", -1, achFile);
	if (n >= 0 && n < 5) OfShareCode = n << 4;
/*	else OfShareCode = OF_SHARE_DENY_WRITE; */		/* leave at default */
	n = (int) GetPrivateProfileInt(achPr, "ExistCode", -1, achFile);
	if (n >= 0 && n < 5) OfExistCode = n << 4;
/*	else OfExistCode = OF_SHARE_DENY_NONE; */		/* leave at default */

	GetPrivateIntegers();							/* read loads of 'em */

/*	fix ups, sanity checks and such: */
	if (magbase < 100) magbase = 100;				/* sanity check */
	if (magbase > 10000) magbase = 10000;			/* sanity check */
	if (StringLimit < 0) StringLimit = 1; 
	else if (StringLimit == 0) StringLimit = MAXCHARBUF; 
	else if (StringLimit > MAXCHARBUF) StringLimit = MAXCHARBUF; 

/*	if (bWin95 == 0) */				/* ignore except in Windows	95 */
	if (bNewShell == 0) {			/* changed 96/Oct/2 */
		bUseRegistryFile = 0;
		bUseRegEnumValue = 0;		/* ??? */
/*		bAutoActivate = 0; */		/* need ATM 4.0 for this */
	}
	if (nMaxDocuments <= 0)  {		// should not happen
//		wininfo("No Documents to Remember");	debugging only
		bRememberDocs = 0;
	}
	else if (nMaxDocuments > 9) nMaxDocuments = 9;
/*	if (bRememberDocs) 	filldocmenu(hwnd); */ /* ??? */

/*	SetCommandStrings(); */		/* moved here 97/Apr/3 */
}

/* come here when exiting secondary DVIWindo with Save Preferences not set */

void WriteNotFirst (void) {					/* 1995/Apr/25 */
	if (achFile == NULL) return;
	if (cxWidth==0) cxWidth = CW_USEDEFAULT;
	if (cxWidth > 200 && cyHeight > 100) {	/* avoid iconized lossage */
/*		sprintf(str, "%d %d %d %d", xLeft, yTop, cxWidth, cyHeight); */
		sprintf(str, "%d %d %d %d %d", xLeft, yTop, cxWidth, cyHeight, bMaximized);
		(void) WritePrivateProfileString(achPr, "NotFirstSize", str, achFile);
	}
/*  record screen mapping and position for secondary DVIWindo's 95/May/5 */
	sprintf(str, "%ld %ld %d", pagexoffset, pageyoffset, pagewantedzoom);
	(void) WritePrivateProfileString(achPr, "NotFirstMapping", str, achFile);
}

void WriteTime (char *achSection, char *szKey) {
	char *s, *send;
	time_t ltime;				/* for time and date */

	if (achFile == NULL) return;
	(void) time(&ltime);		/* get seconds since 1970 */
	if (ltime < 0) return;		/* sanity check 98/Jul/20 */
	s = ctime(&ltime);
	if (s == NULL) return;		/* sanity check 96/Jan/6 */
	lcivilize(s);
/*	if ((send = strchr(s, '\n')) != NULL)
		*send = '\0'; */	/* 1995/Jan/22 */
	send = s + strlen(s) - 1;	/* 1995/Mar/15 */
	while (send > s && *send <= ' ') *send-- = '\0';
//	(void) WritePrivateProfileString(achPr, "LastTime", s, achFile); 
//	(void) WritePrivateProfileString(achPr, szKey, s, achFile);
//	(void) WritePrivateProfileString(achDiag, szKey, s, achFile);
	(void) WritePrivateProfileString(achSection, szKey, s, achFile);
}

void WritePreferences (int flag) {	/* write stuff to private profile file */
//	char *s;
/*	time_t ltime; */				/* for time and date */

	if (achFile == NULL) return;
	if (cxWidth==0) cxWidth = CW_USEDEFAULT;
/*	if (bSavePrefer == 0) return; */			/* 1993/March/16 */
	if (bSavePrefer == 0 && flag == 0) return;		/* 1995/April/22 */
/*	if (bSavePrefer != 0) */	/* only if preferences are to be saved */
	if (cxWidth > 200 && cyHeight > 100) { /* avoid iconized lossage */
/*		sprintf(str, "%d %d %d %d", xLeft, yTop, cxWidth, cyHeight); */
		sprintf(str, "%d %d %d %d %d", xLeft, yTop, cxWidth, cyHeight, bMaximized);
		(void) WritePrivateProfileString(achPr, "Size", str, achFile);
	}
/*	if (UsedxLeft != 0 || UsedyTop != 0 || UsedxLeft != 0 || UsedyTop != 0) */
	if (UsedxLeft != 0 || UsedyTop != 0 || InfoxLeft != 0 || InfoyTop != 0) {
		sprintf(str, "%d %d %d %d", UsedxLeft, UsedyTop, InfoxLeft, InfoyTop);
		(void) WritePrivateProfileString(achPr, "AuxWinPos", str, achFile);
	}

	if (CcxWidth != 0 && CcyHeight != 0) {
		sprintf(str, "%d %d %d %d", CxLeft, CyTop, CcxWidth, CcyHeight);
		(void) WritePrivateProfileString(achPr, "ConsoleSize", str, achFile);
	}

	if (QcxWidth != 0 && QcyHeight != 0) {
		sprintf(str, "%d %d %d %d", QxLeft, QyTop, QcxWidth, QcyHeight);
		(void) WritePrivateProfileString(achPr, "QuerySize", str, achFile);
	}

	sprintf(str, 
"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\
 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\
 %d %d %d",
			bSpreadFlag, bCountZero,       bLandScape,       bGreyText,
			bGreyPlus,   bColorFont,       bRuleFillFlag,    bShowBorder,
			bShowLiner,	 bResetScale,      bShowBoxes,       bCaseSensitive,
			bWrapFlag,   bComplainMissing, bComplainSpecial, bIgnoreSpecial,
			bResetPage,  bUseDVIPSONE,     bPrintToFile,     bPassThrough,
			papertype,   bUseCharSpacing,  bShowButtons,     bReversePages,
			bTrueInch,   bComplainFiles,   bShowPreview,     bCheckEncoding,
			bViewExtras, bDismissFlag,     bShowViewPorts,   bShowCalls,
			nDuplex,     bShowTIFF,        bShowWMF
);
	(void) WritePrivateProfileString(achPr, "Preferences", str, achFile);
	sprintf(str, "%d %d %d %d %d %d %d %d %d %d",
			bCallBackPass,	bConsoleOpen, bOpenCloseChannel, bNewPassThrough,
			bForcePassBack, bDontAskFile, bOldPassThrough, bUseDLLs,
		    bOpenClosePrinter, bUseFontName);
	(void) WritePrivateProfileString(achPr, "Preferences2", str, achFile);
/*	sprintf(str, "%ld %ld %d", xoffset, yoffset, wantedzoom); */
/*	write page position and magnification */
	sprintf(str, "%ld %ld %d", pagexoffset, pageyoffset, pagewantedzoom);
	(void) WritePrivateProfileString(achPr, "Mapping", str, achFile);
/*	write page position and magnification */ /* 96/Aug/26 */
	sprintf(str, "%ld %ld %d", fontxoffset, fontyoffset, fontzoom);
	(void) WritePrivateProfileString(achPr, "FontMapping", str, achFile);
/*	write spread position and magnification 1996/May/12 */
	if (spreadxoffset != pagexoffset || spreadyoffset != pageyoffset ||
		spreadwantedzoom != pagewantedzoom)  {
		sprintf(str, "%ld %ld %d", spreadxoffset, spreadyoffset, spreadwantedzoom);
		(void) WritePrivateProfileString(achPr, "SpreadMapping", str, achFile);
	}
//	winerror(DefPath);			// debugging only
//	winerror(szWinDir);			// debugging only
/*	avoid writing out incomplete path without drive - sanity check */
//	if (strchr(DefPath, ':') == NULL)
//	if for some reason we don't have an absolute path...
	{
		sprintf(debugstr, "DefPath '%s' DefSpec '%s' DefExt '%s'\n",
			DefPath, DefSpec, DefExt); 
		OutputDebugString(debugstr);
//		wincancel(debugstr);		// debugging only
	}
	if (*DefPath == '\0') strcpy(DefPath, szWinDir);	// avoid blank
	if (*DefSpec == '\0') strcpy(DefSpec, "*.dvi");		// avoid blank
	if (*DefExt == '\0') strcpy(DefExt, ".dvi");		// avoid blank
	if (strchr(DefPath, ':') == NULL && strncmp(DefPath, "\\\\", 2) != 0)
		strcpy(DefPath, szWinDir);		/* just to write something! */
	if (strchr(DefPath, ' ') != NULL)	/* quote if spaces 97/Dec/10 */
		sprintf(str, "\"%s\" %s %s", DefPath, DefSpec, DefExt);
	else sprintf(str, "%s %s %s", DefPath, DefSpec, DefExt);
/*		(LPSTR) DefPath, (LPSTR) DefSpec, (LPSTR) DefExt); */
	(void) WritePrivateProfileString(achPr, "Defaults", str, achFile);
/*	sprintf(str, "%s", OpenName); */ /*	(LPSTR) OpenName); */
	strcpy(str, OpenName);						/* 95/Dec/6 */
	if (*str != '\0')
		(void) WritePrivateProfileString(achPr, "FileName", str, achFile);
	if (*SourceDefPath == '\0') strcpy(SourceDefPath, szWinDir);// avoid blank
	if (*SourceDefSpec == '\0') strcpy(SourceDefSpec, "*.tex");	// avoid blank
	if (*SourceDefExt == '\0') strcpy(SourceDefExt, ".tex");	// avoid blank
/*	avoid writing out incomplete path without drive - sanity check */
//	if (strchr(SourceDefPath, ':') == NULL)
//	if for some reason we don't have an absolute path...
	if (strchr(SourceDefPath, ':') == NULL && strncmp(SourceDefPath, "\\\\", 2) != 0)
		strcpy(SourceDefPath, szWinDir); 
	if (strchr(SourceDefPath, ' ') != NULL)	/* quote if spaces  97/Dec/10 */
		sprintf(str, "\"%s\" %s %s", SourceDefPath, SourceDefSpec, SourceDefExt);
	else sprintf(str, "%s %s %s", SourceDefPath, SourceDefSpec, SourceDefExt);
	if (*SourceOpenName != '\0')	/* write only if used */
		(void) WritePrivateProfileString(achPr, "SourceDefaults", str, achFile);
	if (*SourceOpenName != '\0')
		(void) WritePrivateProfileString(achPr, "SourceFileName",
			SourceOpenName, achFile);		
	if (*TestFont != '\0') {
		(void) WritePrivateProfileString(achPr, "Font", TestFont, achFile);
	}
//	sprintf(str, "%d", TestSize);
//	(void) WritePrivateProfileString(achPr, "TestSize", str, achFile);
	(void) WritePrivateProfileInt(achPr, "TestSize", TestSize, achFile);
/*	if (StringLimit == MAXCHARBUF) {
		(void) WritePrivateProfileString(achPr, "StringLimit", NULL,
			achFile);
	}
	else {
		sprintf(str, "%d", StringLimit);
		(void) WritePrivateProfileString(achPr, "StringLimit", str, 
			achFile);
	} */
/*	remember Filter Indeces in File Open for DVI and TeX files */
	sprintf(str, "%lu %lu", DVIFilterIndex, TeXFilterIndex);
	(void) WritePrivateProfileString(achPr, "Filters", str, achFile);
/*	write out units for measurement on `ruler' */
	(void) WritePrivateProfileString(achPr, "Units", 
		unitnames[units], achFile);
//	(void) WritePrivateProfileString(achPr, "Version", VERSION, achFile);
	(void) WritePrivateProfileString(achDiag, "Version", VERSION, achFile);
/*	following added 96/Sep/15 --- not needed 98/Dec/24 --- redundant */
	(void) WritePrivateProfileString(achEnv, "DEBUGPAUSE", bPauseCalls ? "1" : NULL, achFile);
	if (bDebug) WriteTime(achDiag, "LastTime");
}

/* save preferences if the phase of the moon is just right ... */

void maybesaveprefer (void) {
	if (bFlipSave != 0) {
		if (bSavePrefer != 0) bSavePrefer = 0;
		else bSavePrefer = 1; 	/* `temporary' flip bSavePrefer */
	}
/* Only save it if first instance and if not affected by Factory Default */
	else if (bFirstInst != 0 && bFactory == 0) {	
//		sprintf(str, "%d", bSavePrefer);
//		(void) WritePrivateProfileString(achPr, "Save", str, achFile);
		(void) WritePrivateProfileInt(achPr, "Save", bSavePrefer, achFile);
	}
	if (bDebug) {
//		sprintf(str, "%d", bFlipSave);
//		(void) WritePrivateProfileString(achDiag, "Flipped", str, achFile);
		(void) WritePrivateProfileInt(achPr, "Flipped", bFlipSave, achFile);
	}
	if (bSavePrefer) WritePreferences(0);		/* write preferences */
	else if (! bFirstInst) WriteNotFirst();		/* 95/Apr/25 */
}

void TurnTimerOn (HWND hWnd) {
	int interval;
/*	if (bFloppyDisk != 0) winerror("Floppy"); */
	if (bFloppyDisk == 0) interval = TICKTOCK; 
	else interval = TICKTOCK * 8;			/* 1992/Dec/19 */
	bTimerOn = SetTimer(hWnd, 1, interval, NULL);
	nAlarmTicks = 0;						/* 1994/Aug/10 */
}

/* when launched with file name or drag&drop --- not anymore */
/* now used when printing from command line */
/* and in SwitchDVIFile from hypertext command */

void SetupFile (HWND hWnd, int invalidate) {
	int flag;
	char *s;
	
	bReadFile = 0;						/* OK, we can reset this again now */
/*	if (bDebug > 1) OutputDebugString(OpenName); */
//	if file name relative, add current directory
//	if (strchr(OpenName, '\\') == NULL &&
//			strchr(OpenName, '/') == NULL &&
//				strchr(OpenName, ':') == NULL) 
	if (IsRelative(OpenName)) {			// 2000/March/15
			(void) GetCurrentDirectory(sizeof(DefPath), DefPath);

/*			Will the following `DOS' code work in WIN32 ? */
/*			getcwd(DefPath, MAXPATHLEN); */	/* if filename unqualified */
/*			getcwd(DefPath, sizeof(DefPath)); *//* if filename unqualified */

/*			_getcwd(DefPath, sizeof(DefPath)); */

			s = DefPath + strlen(DefPath) - 1;	/* 1995/Dec/18 */
			if (*s != '\\' && *s != '/') strcat(DefPath, "\\");
/*			if unqualified assume its in current directory - file manager interface */
	}
	else {					/* given qualified filename */
		ParseFileName();	/* split OpenName into DefPath, DefSpec, DefExt */
		strcpy(DefSpec, "*");		/* ??? */
		strcat(DefSpec, DefExt);	/* fix up DefSpec */
/*		removeback(DefPath); */		/* 1993/Dec/9 */
		ChangeDirectory(DefPath, 1);	/* change directory and drive */
/*		ChangeDirectory(DefPath, 0); */	/* 1996/Aug/26 trial */
/*		trailingback(DefPath); */	/* 1993/Dec/9 */
	}

	AddExt(OpenName, DefExt);
	hFile = DoOpenFile(hWnd);				/* try and open file */
/*	if (hFile >= 0) */
	if (hFile != HFILE_ERROR) {	
/*		dvipage = 1; */		/* not if called with command line arg */
		flag = DoPreScan(hWnd, hFile);
		if (flag == 0) {
			if (bKeepFileOpen == 0) {
/*				if (hFile < 0) wincancel("File already closed"); */
				if (hFile == HFILE_ERROR) (void) wincancel("File already closed");
/*				if (_lclose(hFile) != 0) winerror("Unable to close file"); */
				else _lclose(hFile);
/*				hFile = -1;	 */
				hFile = HFILE_ERROR;
			}
			usepagetable(dvipage, 0);			/* page is physical page */
			bEnableTermination = 1;
			if (invalidate)						/* 1995/April/30 */
				InvalidateRect(hWnd, NULL, TRUE); 
			if (bUseTimer != 0 && bTimerOn == 0) {
				TurnTimerOn(hwnd);
/*				bTimerOn = SetTimer(hwnd, 1, TICKTOCK, NULL); */
			}
		}
		else { /* 1992/Apr/26 */
/*			if (hFile < 0) wincancel("File already closed"); */
			if (hFile == HFILE_ERROR) {
/*				(void) wincancel("File already closed"); */
				sprintf(debugstr, "%s (%s)\n", "File already closed", "setupfile");
				if (bDebug) {
					if (bDebug > 1) OutputDebugString(debugstr);
					else (void) wincancel(debugstr);
				}
			}
/*			if(_lclose(hFile) != 0)	winerror("Unable to close file"); */
			else _lclose(hFile);
/*			hFile = -1;	 */
			hFile = HFILE_ERROR;
			closeup(hWnd, 1);		/* safe ??? */
#ifdef HYPERFILE
			free_hyperfile();		/* right place ??? winsearc.c */	
#endif
		}
	}
}

/*	Extract device, driver, and port from command line string. Two formats: */

/*	-w="Apple Laser Writer II NT",pscript,COM1: */
/*	-w@Apple@Laser@Writer@II@NT,pscript,COM1: */

/*	Latter first converted to: -w@Apple Laser Writer II NT,pscript,COM1: */

/*  Could do some sanity checks on length, but what the hell ... */
/*	if no arguments, then default printer will be used */

int analyzeprint(char *s) { /* try and extract device, driver, port */
	int c, fakespace;
	char *t;

/*	We use these temporaray when using CommDlg */
/*	char achPrinter[MAXPRINTERNAME]=""; */
	char achDeviceTemp[MAXPRINTERNAME]=""; 
	char achDriverTemp[MAXDRIVERNAME]=""; 
	char achPortTemp[MAXPORTNAME]=""; 

	int n, nlen;
	LPDEVNAMES lpDevNames;

	if (*s++ != 'w') return 0;	/* shouldn't happen */

	c = *s++;
	if (c == '\0') return 1;	/* no arguments, will use default printer */

/*	check whether using fake space character */
	if (c != '=') {
		fakespace = c; t = s;	/* first convert fake spaces to real spaces */
		while ((c = *t++) >= ' ') if (c == fakespace) *(t-1) = ' ';
	}
/*	extract device name */
	c = *s++;
	t = achDeviceTemp;			/* or direct to achDevice ... */
	if (c == '\"') {
		while ((c = *s++) != '\"' && c >= ' ') *t++ = (char) c;
/*		if (c == '\"') c = *s++;	*/ /* gobble the comma ? */
	}
	else {
		*t++ = (char) c;
		while ((c = *s++) != ',' && c >= ' ') *t++ = (char) c;	
	}
	*t = '\0';
	if (c == '\0') return -1;	/* run out of string ... */

/*	extract driver name */
	t = achDriverTemp;			/* or direct to achDriver ... */
	while ((c = *s++) != ',' && c >= ' ') *t++ = (char) c;	
	*t = '\0';
	if (c == '\0') return -1;	/* run out of string ... */	

/*	extract port name */
	t = achPortTemp;			/* or direct to achPort ... */
	while ((c = *s++) != ',' && c >= ' ') *t++ = (char) c;	
	*t = '\0';

// can't have OLDPRINTDLG defined in WIN32...

#ifdef IGNORED
// #ifdef OLDPRINTDLG
/*	we have to copy temporary vales */
	strcpy (achDevice, achDeviceTemp);
	strcpy (achDriver, achDriverTemp);
	strcpy (achPort, achPortTemp);
#endif

	nlen = strlen(achDeviceTemp)+1 + strlen(achDriverTemp)+1 + strlen(achPortTemp)+1;
	pd.hDevNames = GlobalAlloc (GMEM_MOVEABLE, sizeof(DEVNAMES) + nlen);
	if (pd.hDevNames == NULL) {
		winerror ("Unable to allocate memory");
		PostQuitMessage(0);				/* pretty serious */
	}
	lpDevNames = (LPDEVNAMES) GlobalLock(pd.hDevNames);
	n = sizeof(DEVNAMES);
	lpDevNames->wDriverOffset = (WORD) n;
	lstrcpy((LPSTR) (lpDevNames + n), achDeviceTemp);
	n += strlen(achDeviceTemp)+1;
	lpDevNames->wDeviceOffset = (WORD) n;
	lstrcpy((LPSTR) (lpDevNames + n), achDriverTemp);
	n += strlen(achDriverTemp)+1;
	lpDevNames->wOutputOffset = (WORD) n;
	lstrcpy((LPSTR) (lpDevNames + n), achPortTemp);
	n += strlen(achPortTemp)+1;
	GlobalUnlock(pd.hDevNames);		/* link into PrintDlg structure */
/*	GlobalFree (hDevNames); */

	return -1;					/* success (apparently) */
}

int HasExtension (char *szFileName) {
	char *sdot, *slas;
	if ((sdot = strrchr(szFileName, '.')) == NULL) return 0;
	if ((slas = strrchr(szFileName, '\\')) != NULL && sdot < slas) return 0;
	if ((slas = strrchr(szFileName, '/')) != NULL && sdot < slas) return 0;	
	return 1;
}

// Treat filenames with drive letters or ports as absolute (look for :)
// Treat network file names as absolute (start with \\)

int IsRelative (char *szFileName) {
	if (strchr(szFileName, ':') != NULL) return 0;
	if (strncmp(szFileName, "\\\\", 2) == 0) return 0;
	return 1;
}

/* Other Windows programs use /p or -p to ask for printing */
/* but we are already using that for something else ... */

int firstarg=1;		/* index of first file name argument */

/*  See MSJ, May 1991, page 136 for right way to use **__argv */
/*  NOTE: argv[0] is always C:WIN486\system\KRNL386.EXE */
/*	Use GetModuleFileName instead to get path to executable file */
/*  GetModuleFileName(GetModuleHandle("dviwindo.exe"),filename,MAXFILENAME)*/

/*	Following may need rework in UNICODE environment WIN32 */
/*	Use GetCommandLine() in that case in winbase.h WIN32 */

int ReadCommandLine (int firstarg) {
	char *s, *send, *scol;
	int n; 
	int k;

	if (bShowCommand) {
		s = str;
		sprintf(s, "Arguments:\n");
		s = s + strlen(s);
/*		for (k = 1; k < __argc; k++) */
		for (k = 0; k < __argc; k++) {
			if (s + strlen(__argv[k]) + 10 >= str + sizeof(str)) break;
			sprintf(s, "%2d:\t%s\n", k, __argv[k]);
			s = s + strlen(s);
		}

		(void) GetCurrentDirectory(sizeof(buffer), buffer);

/*		_getcwd(buffer, sizeof(buffer)); */

		sprintf(s, "Current:\t%s\n", buffer);
		winerror(str);		/* debugging ??? */
	}

/* 	bFileValid = 0; hClientWindow = NULL; */

/*  sort of illogical to have this override saved default ... */
/*	try and interpret command line arguments and command line flags */
//	firstarg = 1;
	while (firstarg < __argc) {
		s = __argv[firstarg];
		if (*s == '-' || *s == '/') {			/* command line argument ? */
			s++;								/* flush the `-' or `/' */
			if (*s == 'd') bDebug++;			/* turn on debugging mode */
			else if (*s == '1') bCallSelf++;	/* pass call to other instance */
			else if (*s == 's') {
				if ((*(s+1) == '\0') && (firstarg+1 < __argc)) { 
					firstarg++;					/* -s <foo> form */
					s = __argv[firstarg];		/* go to next argument */
				}
				else s++;						/* s=... form, step over '=' */
				if (sscanf(s, "%d%n", &nSourceLine, &n) == 1) {
					s += n;
					while (*s == ' ') s++;
//					ssrc = s;				/* \special{src:...} source file name */
					ssrc = zstrdup(s);		/* \special{src:...} source file name */
					bSearchCall++;			/* it is a search call from editor */
				}
			}
			else if (*s == 'p') {			/* page number (screen) */
				sscanf(s, "p=%d", &dvipage);
				nPageNum=dvipage;
				bPageCall++;				/* 99/Jan/10 */
			}
			else if (*s == 'w')					/* request to print */
				bPrintOnly = analyzeprint(s);
/*			else if (*s == 'j')	bAllowVerbatim = 1; */
			else if (*s == 'r') {			/* 92/Sep/25 */
				if (sscanf(s, "r=%d", &bReversePages) < 1) bReversePages=1;
			}
#ifdef IGNORED
			else if (*s == 's') {			/* steps to increment pages by */
				if (sscanf(s, "s=%d", &pageincrement) < 1) pageincrement=2;
				bOddEven = 0;	/* force old style for Sci Word ? 93/Sep/2 */
			}
#endif
			else if (*s == 'c')			/* number of copies to print */
				sscanf(s, "c=%d", &nCopies);
			else if (*s == 'b')			/* begin page (print) */
				sscanf(s, "b=%d", &beginpage);
			else if (*s == 'e')			/* end page (print) */
				sscanf(s, "e=%d", &endpage);
			else if (*s == 'g') {		/* 1993/Aug/28 */
				bOddOnly = 1;
				bOddEven = 1;	/* in that case force new style */
			}
			else if (*s == 'h') {		/* 1993/Aug/28 */
				bEvenOnly = 1;
				bOddEven = 1;	/* in that case force new style */
			}
			else if (*s == 'm')	{		/* client window */
				sscanf(s, "m=%lu", &hClientWindow);
			}
		}
		else {		/* not a command line argument - so must be filename */
/*			strncpy(OpenName, __argv[k], MAXFILENAME); */
			bReadFile = 1;				/* (at least one) file name on command line */
//			sdvi = __argv[firstarg];	/* save pointer 98/Dec/15 */
			s = __argv[firstarg];
//			new dealing with file relative to current directory 2000/Jan/11
			if (IsRelative(s)) {		// is it relative file name ?
				(void) GetCurrentDirectory(sizeof(str), str);
				send = str + strlen(str) - 1;
				if (*send != '\\' && *send != '/') strcat(str, "\\");
				if (*s != '\\' && *s != '/') strcat(str, s);
				else if ((scol = strchr(str, ':')) != NULL) strcpy(scol+1, s);
				else strcat(str, s+1);
			}
			else strcpy(str, s);
//			force extension if none given 2000/Jan/12
			if (! HasExtension(str)) strcat(str, DefExt);
			sdvi = zstrdup(str);
			if (strstr(sdvi, "dvi_help") != NULL) bReadHelp = 1;
			else bReadHelp = 0;			/* 2000/Jan/5 */
//			winerror(sdvi);			// debugging only
			break;
		}
		firstarg++;
	}
#ifdef DEBUGPRINT
	if (bPrintOnly != 0) {
		if (bDebug > 1) {
			sprintf(debugstr, "Device: %s Driver: %s Port: %s\n", 
				achDevice, achDriver, achPort);
				OutputDebugString(debugstr);
		}
	}
#endif
#ifdef DEBUGCOMMAND
	sprintf(debugstr, "firstarg %d argc %d bDebug %d", firstarg, __argc, bDebug);
	OutputDebugString(debugstr);
#endif
	return firstarg;
}

/* NOTE: we already have read preferences for file name etc */
/*		but command line argument overrides this */
/*		_getcwd(DefPath, sizeof(DefPath)); */
/*		ParseFileName();	*/		/* split OpenName into DefPath, DefSpec, DefExt */
/*		strcpy(DefSpec, "*"); strcat(DefSpec, DefExt); *//* fix up DefSpec */

/*	do we need to be showing the Window for SetupFile(0) to work ??? NO */
/*	if(bReadFile != 0 && OpenName[0] != 0) SetupFile(hwnd); */

/* Above modified slightly to allow specification of multiple print files */

/* following is code for dealing with DEMO versions */

#ifdef ALLOWDEMO
char *demowarn="WARNING: DEMO VERSION.\n";

char *months="JanFebMarAprMayJunJulAugSepOctNovDec";	/* 1994/June/8 */

int monthnumber(char *smonth) {
	int k;
	char *s=months;
	for (k = 0; k < 12; k++) {
		if (strncmp(smonth, s, 3) == 0) return k;
		s += 3;
	}
	return 0;			/* Say what? */
}

void stripcolon(char *s) {	/* replace colons in time with spaces */
/*	while (*s >= ' ') {
		if (*s == ':') *s = ' ';
		s++;
	} */
	while ((s = strchr(s+1, ':')) != NULL) *s = ' ';
}

/* Owner is of form "Berthold K.P. Horn@100@1997 May 23 07:43:48\n" */

time_t checkdemo(char *owner) {		/* now returns seconds since customized */
	time_t ltime, otime;			/* for date and time */
	time_t dtime;					/* seconds since customized */
	struct tm loctim;
	int year, month, day;
	int hour, min, sec;
	char buffer[64];
	char *s;

#ifdef DEBUGINIT
	if (bDebug > 1) {
		OutputDebugString("Check DEMO\n");
		OutputDebugString(str);		/* ??? */
	}
#endif
/*	first calculate compilation time */		/* not used anymore */
/*	sscanf(compiledate, "%s %d %d", buffer, &day, &year); */
	s = owner;							/* use customization time instead */
	if (*s < ' ') return 0;				/* uncustomized */
/*	check that there are two occurences of @ - and step to date part */
	if ((s = strchr(s+1, '@')) == NULL) return -1;
	if ((s = strchr(s+1, '@')) == NULL) return -1;
#ifdef DEBUGINIT
	if (bDebug > 1) OutputDebugString(s+1);
#endif
	stripcolon(s+1);
	if (sscanf(s+1, "%d %s %d %d %d %d",
			  &year, buffer, &day, &hour, &min, &sec) < 6) {
		if (bDebug) winerror(s+1);			/* should not happen */
		return -1;
	}
	if (year > 1900) year = year - 1900; 
	month = monthnumber(buffer); 

	loctim.tm_year = year;
	loctim.tm_mon = month; 
	loctim.tm_mday = day;
#ifdef DEBUGINIT
	if (bDebug > 1) {
		sprintf(debugstr, "%d %d %d\n", year, month, day);
		OutputDebugString(debugstr);
	} 
#endif
/*	stripcolon(compiletime); */	/* extra fancy precision */
/*	sscanf(compiletime, "%d %d %d", &hour, &min, &sec); */ /* not used */
	loctim.tm_hour = hour;
	loctim.tm_min = min;
	loctim.tm_sec = sec;
	loctim.tm_isdst = -1;	/* daylight saving time flag - info not avail */
#ifdef DEBUGINIT
	if (bDebug > 1) {
		sprintf(debugstr, "%d %d %d\n", hour, min, sec);
		OutputDebugString(debugstr);
	}
#endif
	otime = mktime(&loctim);
/*	Note: mktime returns -1 for invalid input data */
/*	This might be off by one hour (3600 sec) because of daylight savings */
	
	(void) time(&ltime);		/* get seconds since 1970 */
/*	Note: time() returns -1 if it can't get the time */
	dtime = ltime - otime;		/* time difference in sec so far */
	if (bDebug > 1) {
		sprintf(debugstr, "dtime %ld = ltime %ld - otime %ld (%lg months)\n",
				dtime, ltime, otime, (double) dtime / (double) onemonth);
		OutputDebugString(debugstr);
	}
	if (dtime > onemonth * 12) {
/*		ExitWindows(EW_REBOOTSYSTEM, 0); */	/* Windows 3.1 */
		ExitWindows(EW_RESTARTWINDOWS, 0);	/* Windows 3.0 */
	}
	return dtime;
}
#endif

/* void showstackused(HWND hWnd, int flag) */
int showstackused(int flag) {		/* 1994/June/21 */
	char *s;
	int iPctOfStackUsed=0;

	if (flag == 0) wsprintf(debugstr, "DVIWindo %s ending --- ", (LPSTR) version);
	else *debugstr = '\0';
	s = debugstr + strlen(debugstr);
	sprintf(s, "%d%% stack used", iPctOfStackUsed);
	if (flag) wininfo(str);				/* show to user */
	else if (bDebug > 1) {				/* fixed 1995/July/10 */
		OutputDebugString(debugstr);	/* write % used in debug output */
	}
	return iPctOfStackUsed;
}

char *convertnewlines (char *str) {
	char *s=str;
	while ((s = strchr(s, '\n')) != NULL) {
		if (*(s+1) == '\0') *s = '\0';
		else *s = ' ';
	}
	return str;
}

void showtaggedchar (void) {
	if ((taggedchar > 32 && taggedchar < 127) ||
		(taggedchar > 160 && taggedchar < 255))
		sprintf(str, "Tag Char was %d (%c)", taggedchar, taggedchar);		
	else sprintf(str, "Tag Char was %d ", taggedchar);
	wininfo(str);
}

void FreeATMTables (void) {
	int k;
#ifdef ATMSOLIDTABLE
	if (hATMTable != NULL) hATMTable = GlobalFree(hATMTable);
	for (k = 0; k < MAXFONTS; k++) encodefont[k] = 0; // ???
#else
	for (k = 0; k < MAXFONTS; k++)
		if (hATMTables[k] != NULL)
			hATMTables[k] = GlobalFree(hATMTables[k]);
#endif
}

void FreeColor (void);

void freeall (void) {
#ifdef HYPERFILE
	free_hyperfile();			/* in winsearc.c */	/* 95/Aug/12 */
#endif
	if (hFaceNames != NULL) FreeFaceNames();
	if (hFullNames != NULL) (void) GlobalFree(hFullNames);	/* 95/July/12 */
	if (hFileNames != NULL) (void) GlobalFree(hFileNames);	/* 95/July/12 */
	hFaceNames = hFullNames = hFileNames = NULL;
	if (hWidths != NULL) (void) GlobalFree(hWidths);
	if (hPages != NULL) (void) GlobalFree(hPages);
	if (hBack != NULL) (void) GlobalFree(hBack);
	hWidths = hPages = hBack = NULL;
	if (hColor != NULL) {
/*		(void) GlobalFree(hColor); */
		FreeColor();			/*		also free saved stacks */
	}
/*	if (hMarks != NULL) (void) GlobalFree(hMarks); */
	if (hTPIC != NULL) (void) GlobalFree(hTPIC);
	if (hEncoding != NULL) (void) GlobalFree(hEncoding);	/* 94/Dec/25 */
	(void) FreeATMTables();
	if (hATMShow != NULL) (void) GlobalFree(hATMShow);		/* 94/Dec/31 */
	hColor = hTPIC = hEncoding = hATMShow = NULL;
	if (szReencodingVector != NULL) free(szReencodingVector); /* 95/Jan/5 */
	if (szEncodingVector != NULL) free (szEncodingVector);	/* 98/Jul/10 */
	FreeFontNames(); 			/* 95/July/7 in winpslog.c */
	if (pd.hDevMode != NULL) GlobalFree(pd.hDevMode);		/* PRINTDLG */
	if (pd.hDevNames != NULL) GlobalFree(pd.hDevNames);		/* PRINTDLG */
	if (szPrintFile != NULL) free(szPrintFile);				/* 95/Dec/25 */
	if (szSource != NULL) free(szSource);					/* 98/Dec/12 */
	if (szEditorDDE != NULL) free(szEditorDDE);
	if (szTeXEdit != NULL) free(szTeXEdit);
	if (szCustom != NULL) free(szCustom);
	if (szCustomPageSize != NULL) free(szCustomPageSize);
	if (szBasePath != NULL) free(szBasePath);
	if (szEPSPath != NULL) free(szEPSPath);
	if (szVecPath != NULL) free(szVecPath);
	if (szPrePath != NULL) free(szPrePath);
	if (szTeXFonts != NULL) free(szTeXFonts);
	if (szTeXDVI != NULL) free(szTeXDVI);
	if (szRegistry != NULL) free(szRegistry);
	if (szWinDir != NULL) free(szWinDir);
//	winerror("Freeing szWinDir");
//	szWinDir = NULL;
	if (szExeWindo != NULL) free(szExeWindo);
	if (logtext != NULL) free(logtext);
	FreeCommandStrings();
}

#ifdef IGNORED
HWND hPreviousWindow=NULL;

BOOL CALLBACK _export EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	int m;
	int n = strlen(TitleText);
#ifdef DEBUGMAIN
	if (bDebug > 1) {
		sprintf(debugstr, "HWND %0lX LPARAM %0lX\n", hwnd, lParam);
		OutputDebugString(debugstr);
	}
#endif
/*	Checking GetWindowTextLength > 0 first avoids exception in Windows NT */
	if ((m = GetWindowTextLength(hwnd)) > 0) {		/* 1996/Aug/4 */
/*		if (bDebug > 1) {
			sprintf(debugstr, "HWND %0lX LPARAM %0lX m %d\n", hwnd, lParam, m);
			OutputDebugString(debugstr);
		} */
		if (GetWindowText(hwnd, str, sizeof(str)) > 0) {
/*			if (bDebug > 1) {
				strcat(debugstr, "\n");
				OutputDebugString(debugstr);
			} */
			if (strncmp(str, TitleText, n) == 0) {
				hPreviousWindow = hwnd;
				return FALSE; 		/* to stop enumerating */
			}
		}
	}
	return TRUE;				/* to continue enumerating */
}	/* lparam unreferenced */
#endif

#ifdef IGNORED
HWND FindPreviousWindow (void) {
	hPreviousWindow=NULL; 
#ifdef DEBUGINIT
	if (bDebug > 1) OutputDebugString("Enter FindPrevious\n");
#endif
/*	(void) EnumWindows((WNDENUMPROC) EnumWindowsProc, (LPARAM) NULL); */
	if (EnumWindows((WNDENUMPROC) EnumWindowsProc, (LPARAM) NULL) == 0)
		return NULL;				/*	returns non-zero if successful */
#ifdef DEBUGINIT
	if (bDebug > 1) OutputDebugString("Exit FindPrevious\n");
#endif
	return hPreviousWindow; 
}
#endif

#ifdef IGNORED
void checklong (void);			/* for test code */
#endif

/*  get path to executable file */		/* moved out 1996/July/16 */

void setupexepath (HINSTANCE hInst) {
	HMODULE hModule;
	HFILE hFile;
	char *s;

	hModule = GetModuleHandle("dviwindo.exe");
	*str = '\0';								/* 95/Dec/1 */
/*	or can we just use hInst instead of hModule ? */
	if (hModule == NULL) {
/*		winerror("Cannot get Module Handle"); */
		if (bDebug > 1)	OutputDebugString("Cannot get Module Handle");
		hModule = hInst;	/* back up in case renamed --- 96/June/5 */
	}
	if (GetModuleFileName(hModule, str, sizeof(str)) > 0) {
/*		strip off executable name and leave path ending in \ */
		if ((s = strrchr(str, '\\')) != NULL) s++;
		else if ((s = strrchr(str, '/')) != NULL) s++;
		else if ((s = strrchr(str, ':')) != NULL) s++;
		else s = str;
		*s = '\0';
/*		exewindo = _strdup(str); */			/* 94/Aug/12 save some space */
		szExeWindo = zstrdup(str);
		if (bDebug > 1) OutputDebugString(szExeWindo);
/*		We assume we are not going back to the old days of no long names */
#ifdef LONGNAMES
/*		Now see whether dviwindo.ini in same directory as dviwindo.exe */
		strcat(str, "dviwindo.ini");
		hFile = _lopen(str, READ | OfExistCode);	/* 96/July/16 */
		if (hFile != HFILE_ERROR) {
			achFile = zstrdup(str);
			_lclose (hFile);
		}
#endif
	}
	else {
		winerror("Cannot get Module File Name");
		szExeWindo = zstrdup("");		/* nonsense, should never happen */
	}

/*	If dviwindo.ini not in same directory as dviwindo.exe, use no path */
/*	if (*achFile == '\0') */
	if (achFile == NULL)
		achFile = zstrdup("dviwindo.ini");		/* default ini file */
	if (bDebug > 1) OutputDebugString(achFile);
}

/* Set up forward search DDE call to existing instance of DVIWindo 98/Dec/15 */

/* See also CallEditor in winanal.c */

int CallOtherInstance (char *ssrc, int nSourceLine, char *sdvi) {
	HSZ hszClientAppName=NULL, hszClientTopic=NULL;	/* Service and Topic */
	HCONV hConv;
	DWORD idClientInst, dwResult, dwTimeout;
	HDDEDATA hddret;
	int errcode;
	int flag = 0;				/* normal return value */

/*	repackage arguments and use DDE to send to self 98/Dec/15 */
/*	DDEOpen("dviwindo.exe", "DVIWindo", "SRCSpecial"); */
/*	DDEExe('[Open("%P\%N.dvi");Source("%n%t");Line(%l)]'); */
/*	DDEClose; */
/*	SetFocus("DVIWindo"); */
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
			flag = -1;		/* failed */
/*			return -1; */	/* should not happen */
		}	
		else if (hConv != NULL) {
			if (bSearchCall) 
/*				DDEExe('[Open("%P\%N.dvi");Source("%n%t");Line(%l)]'); */
				sprintf(str, "[Open(\"%s\");Source(\"%s\");Line(%d)]",
						sdvi, ssrc, nSourceLine);
			else if (bPageCall)
				sprintf(str, "[Open(\"%s\");Page(%d)]",	sdvi, nPageNum);
			else							/* plain ordinary open */
/*				DDEExe('[Open("%P\%N.dvi")]'); */
				sprintf(str, "[Open(\"%s\")]", sdvi);
			dwTimeout = TIMEOUT_ASYNC;	/* asynchronous */
/*			dwTimeout = DDE_TIME; */	/* in milliseconds */
			hddret = DdeClientTransaction((unsigned char *) str,	/* lpbData */
										  strlen(str) + 1,	/* cbDataLen */
										  hConv,			/* hConv */
										  NULL,				/* hszItem */
										  CF_TEXT,			/* wFmt */
										  XTYP_EXECUTE,		/* wType 0 ? */
										  dwTimeout,		/* dwTimeout ??? */
										  &dwResult);
			if (hddret == 0) {				/* execution failed */
#ifdef DEBUGDDE
				DdeFailureReport(idClientInst, "DdeClientTransaction");
#endif
			}

			if (DdeDisconnect(hConv) == 0) {
#ifdef DEBUGDDE
				DdeFailureReport(idClientInst, "DdeDisconnect");
#endif
			}
		} /* end of hConv != NULL */
	} /* end of string handles obtained successfully */

	if (hszClientTopic != NULL) {
		if (DdeFreeStringHandle(idClientInst, hszClientTopic) == 0) { 
#ifdef DEBUGDDE
			DdeFailureReport(idClientInst, "DdeFreeStringHandle");
#endif
		}
	}
	if (hszClientAppName != NULL) {
		if (DdeFreeStringHandle(idClientInst, hszClientAppName) == 0) {
#ifdef DEBUGDDE
			DdeFailureReport(idClientInst, "DdeFreeStringHandle");
#endif
		}
	}
	if (DdeUninitialize(idClientInst) == 0) {
/*		failed to free DDEML library resources */
#ifdef DEBUGDDE
		DdeFailureReport(idClientInst, "DdeUninitialize");
#endif

	}
	return flag;
}

void SetupEncodingMenu (HWND hwnd, int warnflag) {
	HMENU hMenu;
	int n;
	if (GetPrivateProfileString(achEnv, "ENCODING", "", str, sizeof(str),
								achFile) <= 0) {
		if (warnflag)
//			winerror("ENCODING undefined");	/* new 98/Jul/9 */
			winerror("ENCODING env var undefined\nPerhaps dviwindo.ini missing?");
		strcpy(str, "texnansi");		/* make this the default */
	}

		hMenu = GetMenu(hwnd);					/* removepath ? */
		(void) CheckMenuItem (hMenu, IDM_TEXNANSI,
					  (_stricmp(str, "texnansi") == 0) ? MF_CHECKED : MF_UNCHECKED);
		(void) CheckMenuItem (hMenu, IDM_TEX256,
					  (_stricmp(str, "tex256") == 0) ? MF_CHECKED : MF_UNCHECKED);
		(void) CheckMenuItem (hMenu, IDM_ANSINEW,
					  (_stricmp(str, "ansinew") == 0) ? MF_CHECKED : MF_UNCHECKED);
		(void) CheckMenuItem (hMenu, IDM_STANDARD,
					  (_stricmp(str, "standard") == 0) ? MF_CHECKED : MF_UNCHECKED);
		(void) CheckMenuItem (hMenu, IDM_TEXTEXT,
					  (_stricmp(str, "textext") == 0) ? MF_CHECKED : MF_UNCHECKED);
/*		don't bother if no custom encoding 97/Dec/22 */
		if (szCustom != NULL) 
			(void) CheckMenuItem (hMenu, IDM_CUSTOMENCODING,
/*			  (_stricmp(str, "custom") == 0) ? MF_CHECKED : MF_UNCHECKED); */
						  (_stricmp(str, szCustom) == 0) ? MF_CHECKED : MF_UNCHECKED);

		n = GetPrivateProfileString(achEnv, "texnansi", "", str, sizeof(str),
									achFile); 
		(void) EnableMenuItem(hMenu, IDM_TEXNANSI, n ? MF_ENABLED :
							  MF_GRAYED);  
		n = GetPrivateProfileString(achEnv, "tex256", "", str, sizeof(str),
									achFile); 
		(void) EnableMenuItem(hMenu, IDM_TEX256, n ? MF_ENABLED :
							  MF_GRAYED);
		n = GetPrivateProfileString(achEnv, "ansinew", "", str, sizeof(str),
									achFile); 
		(void) EnableMenuItem(hMenu, IDM_ANSINEW, n ? MF_ENABLED :
							  MF_GRAYED);
		n = GetPrivateProfileString(achEnv, "standard", "", str, sizeof(str),
									achFile); 
		(void) EnableMenuItem(hMenu, IDM_STANDARD, n ? MF_ENABLED :
							  MF_GRAYED);
		n = GetPrivateProfileString(achEnv, "textext", "", str, sizeof(str),
									achFile); 
		(void) EnableMenuItem(hMenu, IDM_TEXTEXT, n ? MF_ENABLED :
							  MF_GRAYED);
/*		n = GetPrivateProfileString(achEnv, "custom", "", str, sizeof(str), 
							  achFile); */
/*		don't bother if no custom encoding 97/Dec/22 */
		if (szCustom != NULL) {
			n = GetPrivateProfileString(achEnv, szCustom, "", str, sizeof(str),
										achFile); 
			(void) EnableMenuItem(hMenu, IDM_CUSTOMENCODING, n ? MF_ENABLED :
								  MF_GRAYED);
		}
}

/****************************************************************************

	FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

	PURPOSE: calls initialization function, processes message loop

****************************************************************************/

/* in WIN32 PASCAL => APIENTRY */ /* not clear change needed, since: */
/* #define APIENTRY WINAPI */ /* in WIN16 */
/* #define WINAPI _stdcall */ /* in WIN16 */
/* #define PASCAL _stdcall */ /* in WIN32 */

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpCmdLine, int nCmdShow) {
	int x, y, cx, cy;
	int flag; 
	int n, m;
	int k;
	MSG msg;					/* GetMessage(&msg, ...) */
	HMENU hMenu, hSubMenu, hDebugMenu, hEncodingMenu, hPageSizeMenu;
/*	HMODULE hModule; */				/* 1996/Jan/28 */
	HDC hDC;
	char *s;
	HICON hIcon;

/*	Start enabling OutputDebugString early */

	if ((s = getenv("CAMBRIDGE")) != NULL ||
		(s = getenv("CONCORD")) != NULL ||
		(s = getenv("CARLISLE")) != NULL) {
		bDebug = 2;						/* get early start on enabling */
	}

/*	Set *DOS* WINDEBUG=2 to enable OutputDebugString before command line */

	if ((s = getenv("WINDEBUG")) != NULL) {			/* 1995/Nov/5 */
		if (sscanf(s, "%d", &bDebug) < 1)			/* read value */
			bDebug = 2;								/* default if failed */
	}

// #ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("");
	if (bDebug > 1) OutputDebugString("WinMain\n");
// #endif

	hPrevInst = hPrevInstance;		/* remember for exit - NULL in WIN32 */

/*	Check whether this is the first instance of DVIWindo 98/Dec/15 */

	(void) CreateMutex(NULL, 0, "DVIWindo_Mutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS) bFirstInst=FALSE; // bNotFirst=TRUE;
	else bFirstInst=TRUE; // bNotFirst=FALSE;

	if (bDebug > 1) {
		sprintf(debugstr, "bFirstInst %d", bFirstInst);
		OutputDebugString(debugstr);
	}

//	{
//		int k;
//		for (k=0; k < 43; k++) {
//			sprintf(debugstr, "%d %d", k, GetSystemMetrics(k));
//			winerror(debugstr);		// debugging only
//		}
//	}

//	flag = GetSystemMetrics(SM_MOUSEWHEELPRESENT);
//	if (flag) winerror("Wheel Present");	// debugging only
//	else winerror("Wheel Absent");			// debugging only

	cx = GetSystemMetrics(SM_CXSCREEN);		/* 1994/Jan/7 */
	cy = GetSystemMetrics(SM_CYSCREEN);		/* 1994/Jan/7 */
	if (cx > 640 && cy > 480) bHiresScreen = 1;	/* better than VGA ? */
	else bHiresScreen = 0;						/* no, keep dialogs smaller */

	if (bHiresScreen) {					/* 95/Sep/23 */
		bHelpAtEnd = 1;
		nMaxDocuments = 9;
	}
	else {
		bHelpAtEnd = 0;
		nMaxDocuments = 4;
	}

	setupexepath(hInst);	/* move to start of WinMain so achFile setup */

/*	In WIN32, need to do INITAPPLICATION even if previous instance ... */
/*	In Win32, hPrevInstance always NULL */

	if (! hPrevInstance) {		/* first instance of this application ? */
		if (! InitApplication(hInstance))  return (FALSE);
	}

/*	if (! hPrevInstance) */	/* first instance of this application ? */
	if (bFirstInst) {		/* first instance of this application: */
		x = y = 0;
/*		cx = GetSystemMetrics(SM_CXSCREEN); */
/*		cy = GetSystemMetrics (SM_CYSCREEN) -
				GetSystemMetrics (SM_CYICON) -
					(GetSystemMetrics (SM_CYCAPTION) * 2); */
		cy = cy - GetSystemMetrics(SM_CYICON) -
					(GetSystemMetrics(SM_CYCAPTION) * 2);
		(void) GetPrivateProfileString(achPr, "Size", "",
			str, sizeof(str), achFile);
/*		sscanf(str, "%d %d %d %d", &x, &y, &cx, &cy); */
 		if (sscanf(str, "%d %d %d %d %d", &x, &y, &cx, &cy, &bMaximInit) < 4) {
		}
/*		wininfo(str); */
		bSavePrefer = (int) GetPrivateProfileInt(achPr, "Save",
			bSavePrefer, achFile);
	}
	else {				/* not first instance - use default screen instead */
/*		x = cx = CW_USEDEFAULT; */	/* removed 95/Apr/25 */
/*		y = cy = 0; */				/* removed 95/Apr/25 */
		x = y = 0;					/* use different position and size */
		cy = cy - GetSystemMetrics(SM_CYICON) -
					(GetSystemMetrics(SM_CYCAPTION) * 2);
		(void) GetPrivateProfileString(achPr, "NotFirstSize", "",
			str, sizeof(str), achFile);
/*		if (sscanf(str, "%d %d %d %d", &x, &y, &cx, &cy) < 4) */
		if (sscanf(str, "%d %d %d %d %d", &x, &y, &cx, &cy, &bMaximInit) < 4) {
			x = cx = CW_USEDEFAULT;
			y = cy = 0;
		}							/* revert to old method the first time */
/*		bNotFirst = 1; */			/* remember that we are not first */
/*		bFirstInst = 0; */ 			/* remember that we are not first */
		bSavePrefer = 0;			/* to avoid overwriting Settings later */
//		{
//			sprintf(debugstr, "nCmdShow %d", nCmdShow);
//			OutputDebugString(debugstr);
//			wininfo(debugstr);
//		}			// debugging only
		if (nCmdShow == SW_MAXIMIZE)	/* 3 */
			nCmdShow = SW_NORMAL;		/*  1 kludge for DVI_HELP call from menu */
	}

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("After InitApplication\n");
#endif

/* 	readpreferences(); */			/*	moved into InitInstance */

	if (!InitInstance(hInstance, nCmdShow, x, y, cx, cy))
		return (FALSE);

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("After InitInstance\n");
#endif

/*	move following down past command line reading */

	if (! hPrevInstance) {
		if (bSpinOK == 0) winerror("Unable to register custom spin control!");
	}
	else bSpinOK = 1;	/* assume first instance already registered it! */

/*	extract serial number first from start of newmagic */ /* 94/Dec/15 */

	serialnumber = 0;			/* assumed high order first */
	for (k = 0; k < 4; k++) {
		serialnumber = serialnumber << 8;
		serialnumber = serialnumber | (newmagic[k+4] & 255);
/*		printf("Serial: %ld\n", serialnumber); */
	}
	if (serialnumber == 1651208296) serialnumber = 0;	/* temporary */

/*  see whether this file has been tampered with */
	if ((serialnumber % REEXEC != 0) ||	
/*		checkowner(hexmagic, str) != 0 || */
		checkowner(hexmagic, str, sizeof(str)) != 0 ||
			checkcopyright(copyright) != 0		/* 1993/Aug/28 */
				) {
/*	hwnd created above in InitInstance */
		(void) DestroyWindow (hwnd);			/* 97/Jan/5 */
		UnregisterClass(DviWindoClass, hInst);	/* 97/Jan/5 */
		return (FALSE);
	}
/*	this drastic action leaves behind all sorts of junk ... */
/*	... undeleted brushes, pens, windows, windows classes etc */

/*	following must be after checkowner, which sets DEMO flag */

#ifdef ALLOWDEMO
	if (bDemoFlag) {	/* checkowner above leaves owner info in str */
		dtime = checkdemo(str);
		TitleText="DVIWindo DEMO";	/* change title - need do more ? */
		SetupWindowText(hwnd);		/* ??? */
		sprintf(str, "%ld", dtime);			/* 97/May/25 */
		(void) WritePrivateProfileString(achDiag, "dtime", str, achFile);
	}
/*	else WritePrivateProfileString(achPr, "demo", NULL, achFile); */
#endif

/*	Setting up szExeWindo - has been */
/*	moved into WinMain, so szExeWindo available when ATM being set up */

	dvipage = 1;			/* default page number */
	bFileValid = 0;
	hClientWindow = NULL;
	TaskInfo.hWnd = NULL;	/* reset, just in case 1995/Mar/12 */

#ifdef USEPALETTE
	hPal = NULL;			/* 1996/March/24 */
	nUpdateCount = 0;		/* 1996/March/24 */
#endif

	bPauseCalls = GetPrivateProfileInt(achEnv, "DEBUGPAUSE", 0, achFile);
	if (bPauseCalls) {
		if (*lpCmdLine != '\0') {	/* show command line unless empty */
			int flag;
			flag = MessageBox(hwnd, lpCmdLine, "Command Line",
							  MB_ICONINFORMATION | MB_OK);	
			if (flag == IDCANCEL) {				/* quit if user cancels */
				(void) DestroyWindow (hwnd);			/* 97/Jan/5 */
				UnregisterClass(DviWindoClass, hInst);	/* 97/Jan/5 */
				return (FALSE);
			}
		}
	}

//	wininfo(lpCmdLine);		// debugging only

	bShowCommand = GetPrivateProfileInt(achEnv, "SHOWCOMMAND", 0, achFile);
/*	Check for command line parameters --- file name & page number */
/*	- or use lpCmdLine */ 
	if (__argc > 1) firstarg = ReadCommandLine(firstarg);

	if (bReadFile) {
		if (bSearchCall) {			/* search call from editor */
			if (bDebug > 1) {
				sprintf(debugstr, "Line %d Source `%s' DVI `%s' firstarg %d __argc %d",
						nSourceLine, ssrc, sdvi, firstarg, __argc);
				OutputDebugString(debugstr);
			}
		}
		else if (bPageCall) {
			if (bDebug > 1) {
				sprintf(debugstr, "Page %d DVI `%s' firstarg %d __argc %d",
						nPageNum, sdvi, firstarg, __argc);
				OutputDebugString(debugstr);
			}
		}
		else {						/* just open the DVI file */
			if (bDebug > 1) {
				sprintf(debugstr, "DVI `%s' firstarg %d __argc %d",
					sdvi, firstarg, __argc);
				OutputDebugString(debugstr);
			}
		}
/*		bReadFile = 0; */						/* OK, we can reset this now ? */
	}

/*	This must come before we try and set up DDEServer */

/*	if (bCallSelf && bNotFirst) */
/*	if (bNotFirst && (bCallSelf || bReadFile)) */		/* 98/Dec/25 */
//	if (bNotFirst && (bCallSelf || (bReadFile && ! bMultiInstance)))  99/May/31
	if (! bFirstInst && (bCallSelf ||
					  (bReadFile && ! bReadHelp && ! bMultiInstance))) {
		if (CallOtherInstance(ssrc, nSourceLine, sdvi) < 0) {
			winerror("Unable to call other instance");
/*			now drop through if call to other instance fails 99/May/31 */
		}
		else {
			(void) DestroyWindow (hwnd);			/* ? */
			UnregisterClass(DviWindoClass, hInst);	/* ? */
			return (FALSE);			/* now flush this instance */
		}
	}
/*	if (bCallSelf && bFirstInst) */ /* further down now */

	if (bDebugKernel) {
		if (bDebug > 1)
			OutputDebugString("Running Windows Debug Kernel\n");
	}

	if (bPreferKnown == 0) {			/* 1993/Jan/21 */
		if (bDebug > 1) {
			sprintf(debugstr, "VK_SHIFT %X VK_CONTROL %X VK_MENU %X\n",
				GetAsyncKeyState(VK_SHIFT), 
					GetAsyncKeyState(VK_CONTROL),
						GetAsyncKeyState(VK_MENU));					
			OutputDebugString(debugstr);
		}
	}

#ifdef ALLOWDEMO
	if (bDemoFlag) {
		if (bDebug > 1)	{
			OutputDebugString(demowarn);
			if (dtime != 0) {
				sprintf(debugstr, "dtime %ld sec %lg months\n",
						dtime, (double) dtime / (double) onemonth);;
				OutputDebugString(debugstr);
			}
/*			else OutputDebugString("dtime == 0"); */
		}
	}
#endif

#ifdef DEBUGINIT
	if (bDebug > 1) {
		OutputDebugString("WARNING: DEBUGINIT!\n");
		winerror("WARNING: DEBUGINIT!\n");
	}
#endif

/*	moved down here past command line reading so bPrintOnly can get set */
/*	if (bPrintOnly == 0) {
		(void) ShowWindow(hwnd, nCmdShow);
		UpdateWindow(hwnd);
	} */

	hMenu = GetMenu(hwnd);
	if (hMenu == NULL) {			/* Sanity check 95/Nov/20 */
		winerror("No Menu!");
		return(FALSE);
	}


/*	Following added to set up Papersize pop-up menu 2000 May 27 */

/*	check if CUSTOMPAPERSIZE env variable set 1997/Dec/22 */
/*	note: presently have to restart DVIWindo when changing	CUSTOMPAPERSIZE=... */
	(void) GetPrivateProfileString(achEnv, "CustomPaperSize", "",
								   str, sizeof(str), achFile);
/*	ignore if it contains semicolon */
	if (*str != '\0' && strchr(str, ';') == NULL) {	/* add menu item */
		if (szCustomPageSize != NULL) free(szCustomPageSize);
		szCustomPageSize = zstrdup(str);
		(void) decodepapersize(str, &CustomPageWidth, &CustomPageHeight);
/*		will need to use AppendMenu to insert custom encoding name */
		hMenu = GetMenu(hwnd);
		hSubMenu = GetSubMenu(hMenu, 2); /* third sub menu */
		hPageSizeMenu = GetSubMenu(hSubMenu, 0);	/* first item */
//		strcpy(str, "Custom");
		AppendMenu(hPageSizeMenu, MF_ENABLED | MF_STRING,
				   IDM_CUSTOMSIZE, str);
		checkpaper(hwnd, papertype);	// done earlier before ready
	}

/* WARNING: menu positions are hard-wired in the following: */
/* assumes FONT menu is last *at this point* */
/* and assumes DEBUG submenu is last in FONT menu */
/* removes DEBUG menu when not given -d -d on command line --- */
/* and removes `Copy to ClipBoard' if Windows 3.0 */

/*	Following added to set up encoding pop-up menu 97/Apr/4 */

/*	check if CUSTOM env variable set 1997/Dec/22 */
/*	note: presently have to restart DVIWindo when changing CUSTOM=... */
	(void) GetPrivateProfileString(achEnv, "CUSTOM", "",
			   str, sizeof(str), achFile);
/*	ignore if it contains semicolon */
	if (*str != '\0' && strchr(str, ';') == NULL) {	/* add menu item */
/*		if (bDebug > 1) OutputDebugString(str);	*/
		if (szCustom != NULL) free(szCustom);
		szCustom = zstrdup(str);			/* removepath ? */
/*		will need to use AppendMenu to insert custom encoding name */
		hMenu = GetMenu(hwnd);
		m = GetMenuItemCount(hMenu);
		hSubMenu = GetSubMenu(hMenu, m - 2); /* second last sub menu at this point */
		n = GetMenuItemCount(hSubMenu);
		hEncodingMenu = GetSubMenu(hSubMenu, n - 1);	/* last item */
/*		if (bDebug > 1) {
			sprintf(debugstr, "m %d n %d", m, n);
			OutputDebugString(debugstr);
		} */
		strcpy(str, "Custom");
		strcat(str, "\t");
		strcat(str, szCustom);				/* add menu item "Custom\txxxxx" */
		AppendMenu(hEncodingMenu, MF_ENABLED | MF_STRING,
				   IDM_CUSTOMENCODING, str);
	}

	SetupEncodingMenu(hwnd, 1);

	bDebugMenu = GetPrivateProfileInt(achEnv, "DEBUGMENU", 0, achFile);

//	Remove debug menu unless -d -d on command line or env var DEBUGMENU
//	if (bDebug < 2) 
	if (bDebug < 2 && bDebugMenu == 0) {
		hMenu = GetMenu(hwnd);
		m = GetMenuItemCount(hMenu);
		hSubMenu = GetSubMenu(hMenu, m - 1);	/* last sub menu */
												/* before TeX & Help added */
		n = GetMenuItemCount(hSubMenu);
		hDebugMenu = GetSubMenu(hSubMenu, n - 1);	/* get a hand on it */
		RemoveMenu(hSubMenu, n - 1, MF_BYPOSITION);	/* remove it */
		RemoveMenu(hSubMenu, n - 2, MF_BYPOSITION);	/* Separator */
		DestroyMenu(hDebugMenu);					/* flush it ! */
	}

/*	See whether there is an [Environment] section in dviwindo.ini */
	n = GetPrivateProfileString(achEnv, NULL, "", str, sizeof(str), achFile);
	if (n > 0) 	bEnvironment = 1;
	else bEnvironment = 0;

/*	Need to DestroyWindow later if not associated with a Window ??? */

	nEnvironment=0;
	if (bEnvironment) {
		hMenu = GetMenu(hwnd);
/*		hSubMenu = GetSubMenu(hMenu, 1); */	/* Preferences, second sub menu	*/ 
		hSubMenu = GetSubMenu(hMenu, 2);	/* Preferences, second sub menu	*/ 
		hEnvMenu = CreatePopupMenu();
		AppendMenu(hSubMenu, MF_ENABLED | MF_POPUP, (UINT) hEnvMenu, "Environment");
		fillenvmenu(hwnd); 
	}

/*	See whether there is an [Applications] section in dviwindo.ini */
	n = GetPrivateProfileString(achAp, NULL, "", str, sizeof(str), achFile);
	if (n > 0) 	bApplications = 1;
	else bApplications = 0;

	nApplications = 0;
	if (bApplications) {
		hMenu = GetMenu(hwnd);
		hTeXMenu = CreatePopupMenu();
/*		AppendMenu(hMenu, MF_ENABLED | MF_POPUP, (UINT) hSubMenu, "&TeX"); */
		AppendMenu(hMenu, MF_ENABLED | MF_POPUP, (UINT) hTeXMenu, "&TeX");
/*		don't do before Help Menu is added at end ... 95/Oct/6 */
/*		filltexmenu(hwnd); */  /* fill later when popup is pulled down ? */
/*		bTeXMenuOK=1; */ /* not here since still last on menu bar */
	}

	if (bHelpAtEnd) {
		hMenu = GetMenu(hwnd);
		hHelpMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_ENABLED | MF_POPUP, (UINT) hHelpMenu, "&Help");
		fillhelpmenu(hwnd);
	}

	if (bApplications) { 
/*		don't do before Help Menu is added at end ... 95/Oct/6 */
		filltexmenu(hwnd); 
/*		bTeXMenuOK=1; */ /* not here since still last on menu bar */
	} 

/*	Remove the above if want to always create a [Documents] section */
/*	if (bRememberDocs) 	filldocmenu(hwnd); */

/*	remove the new `Duplex...' in `Preferences...' 96/Nov/17 */
	hMenu = GetMenu(hwnd);
	hSubMenu = GetSubMenu(hMenu, 1);
/*	DeleteMenu (hSubMenu, IDM_DUPLEX, MF_BYCOMMAND); */
	DeleteMenu (hSubMenu, 1, MF_BYPOSITION);

/*	PostMessage: careful not to pass any args that are on the stack !!! */

/*	In case launched from client application via WinExec or LoadModule */
	if (hClientWindow != NULL) {		/* give Client our Window Handle */
/*		PostMessage(hClientWindow, WM_IAMHERE, hwnd, 0L); */
		PostMessage(hClientWindow, WM_IAMHERE, (WPARAM) hwnd, 0L);
	}

/* After 12 month, it will restart Windows */
/* After 3 month, it will exit gracefully */
/* After 1 month, it will start nagging */
/* If time is negative enough it will exit - user is tampering with time */

#ifdef ALLOWDEMO
	if (bDemoFlag) {
		if (dtime < - oneday) return (FALSE);	/* negative time! */
		if (dtime > onemonth * bDemoFlag) {
			winerror("Please contact Y&Y, Inc. for non-DEMO version");
		}
		if (dtime > onemonth * (bDemoFlag - 1)) 
			PostMessage(hwnd, WM_COMMAND, (WPARAM) IDM_ABOUT, 0L);
		if (dtime > onemonth * (bDemoFlag + 2)) {
			winerror("Sorry, but this DEMO version has expired");
			return (FALSE);	/* EXPIRED ! */
		}
	}
#endif

#ifdef DEBUGENVIRONMENT
	if (bDebug > 1) ShowEnvironment();	/* debugging 95/May/15 */
#endif

	if (bDebug > 1) {
		OutputDebugString("");
		wsprintf(debugstr, "DVIWindo %s starting --- ", (LPSTR) version);
		strcat(debugstr, compiledate);
		strcat(debugstr, " ");
		strcat(debugstr, compiletime);
		OutputDebugString(debugstr);
	}
	if (bDebug > 1) {						/* 1996/Mar/24 */
		ShowConfiguration(debugstr);		/* Windows version information */
		OutputDebugString(debugstr);		
		showscreensize(debugstr);
		OutputDebugString(debugstr);
	}
	ShowConfiguration(debugstr);
	convertnewlines(debugstr);
	WritePrivateProfileString(achDiag, "Config", debugstr, achFile);
	showscreensize(debugstr);
	convertnewlines(debugstr);
	WritePrivateProfileString(achDiag, "Screen", debugstr, achFile);	

#ifdef HYPERFILE
	for (k = 0; k < MAXHYPERPUSH; k++) hyperfile[k] = NULL;	 /* 95/Feb/8 */
#endif

#ifdef ATMSOLIDTABLE
	hATMTable = NULL; 
	for (k = 0; k < MAXFONTS; k++) encodefont[k] = 0;	// ???
#else
	for (k = 0; k < MAXFONTS; k++) hATMTables[k] = NULL;
#endif

/*	sprintf(debugstr, "bATMBugFlag %d\n", bATMBugFlag);	
	if (bDebug > 1) OutputDebugString(debugstr); */

/*	Have we been called just to print a file ? */

	if (bPrintOnly && bReadFile) {
		bSinglePage = 0;
/*		if no printer specified, put up printer setup menu */
/*		maybe put up full printing dialog ? DoPrinting ? (after read file) */
		if (bPrintOnly > 0) {
			if (DoPrintSetup(hwnd) == 0)		/* OLD or NEW */
				goto quitwindow;				/* user cancel ? */
		}

/*		New version to deal with spaces in file names 97/June/10 */
/*		Command line args are split at white space ! */
/*		What about "..." - shouldn't caller use that ? */
		*OpenName = '\0';
/*		Stitch in IniDefPath ??? */
		if (IniDefPath != NULL)
			strcpy(DefPath, IniDefPath);			/* ??? 99/Apr/21 */
		for (k = firstarg; k < __argc; k++) {		/* stitch together again */
			strcat(OpenName, __argv[k]);
			if (k+1 < __argc) strcat(OpenName, " ");
		}
		if (OpenName[0] == 0) goto  quitwindow;		/* shouldn't happen */
		AddExt(OpenName, ".dvi");
#ifdef DEBUGCOMMAND
		winerror(OpenName);	 /* debugging */
#endif
		SetupFile(hwnd, 1);
		if (bFileValid != 0) {
			hDC = GetDC(hwnd);
			flag = paintpage(hwnd, hDC);
			(void) ReleaseDC(hwnd, hDC);
			if (flag < 0) {
				sprintf(str, "Failed to open %s", OpenName);
				winerror (str);
			}
			else {
				bBusyFlag++;				/* too early ? */
				(void) DoPrintAll(hwnd);
				bBusyFlag = 0;
			}
		}
		goto quitwindow;	/* done printing so go away */
	}

/*	Never get here if only asked to print */

	if (bMaximInit) {
		nCmdShow = SW_MAXIMIZE;	/* 97/July/25 */
	}

/*	winerror("Show Window"); */			/* debugging */

//	attempt to use nicer icon on window 99/July/12
	hIcon = LoadIcon(hInst, "DviIcon");
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);

	(void) ShowWindow(hwnd, nCmdShow);	/* Show the window   		*/
//	attemp to solve screen maximization problem
//	if (bReadFile)
//		(void) ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);					/* Sends WM_PAINT message	*/

//	sprintf(debugstr, "nCmdShow %d bMaximInit %d bFirstInst %d",
//			nCmdShow, bMaximInit, bFirstInst);
//	wininfo(debugstr);		// debugging only
	
	if (bDDEServer) (void) DDEServerStart();	/* 98/Dec/12 */

/*	sprintf(str, "Maximize %d CmdShow %d", bMaximized, nCmdShow);
	wininfo(str); */ /* DEBUGGING */

#ifdef IGNORED
	checklong(); goto quitwindow; 	/* debugging code 95/Dec/1 */
#endif

/*	moved down here so can happen after everthing is set up ? */

	if (bReadFile) {
		if (! bCallSelf || (bCallSelf && bFirstInst)) {
			if (bDebug > 1)
				OutputDebugString("First Instance and Call Self"); 
/*			repackage arguments and use PostMessage 98/Dec/15 */
			PostMessage(hwnd, WM_COMMAND, IDM_OPEN, (LPARAM) sdvi);
			if (bSearchCall) {
				PostMessage(hwnd, WM_COMMAND, IDM_SOURCEFILE, (LPARAM) ssrc);
				PostMessage(hwnd, WM_COMMAND, IDM_SOURCELINE, (LPARAM) nSourceLine);
			}
			else if (bPageCall) {
				PostMessage(hwnd, WM_COMMAND, IDM_SELECTPAGE, (LPARAM) nPageNum);
			}
		}
	}

//	in Windows 95 and Windows NT 3.51 do the following for MOUSEWHEEL support
//	#include zmouse.h
//	HWND hdlMSHWheel=NULL;
//	UINT msgMSHWheelSupportActive=NULL;
//	BOOL fWheelSupport=FALSE;
//	msgMSHWheelSupportActive = RegisterWindowMessage(MSH_WHEELSUPPORT);
//	hdlMSHWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
//	if (hdlMSHWheel && msgMSHWheelSupportActive) {
//		fWheelSupport = (BOOL)SendMessage(hdlMSHWheel, msgMSHWheelSupportActive, 0, 0);
//	}

	if (bWin95)
		uMSH_MOUSEWHEEL = RegisterWindowMessage(MSH_MOUSEWHEEL);	// 2000 May 27

/*	winerror("Enter Message Loop");	*/	/* debugging */

/*	Main Window Message Loop: */
/*	while (GetMessage(&msg, NULL, NULL, NULL)) */
	while (GetMessage(&msg, NULL, 0, 0)) {
/*		Find Box is modeless and may hang around, but need processing */
		if (hFindBox != NULL && IsDialogMessage(hFindBox, &msg)) 
			continue;					/* 1995/Mar/1 */
		if (hFntBox != NULL && IsDialogMessage(hFntBox, &msg)) 
			continue;					/* 1998/Sep/1 */
		if (hInfoBox != NULL && IsDialogMessage(hInfoBox, &msg)) 
			continue;					/* 1998/Sep/1 */
		if (hConsoleWnd != NULL && IsDialogMessage(hConsoleWnd, &msg)) 
			continue;					/* 1999/June/25 */
/*		Print Abort is modeless and may hang around, but need processing ? */
/*		if (hDlgPrint != NULL && IsDialogMessage(hDlgPrint, &msg)) 
			continue; */
		if (!TranslateAccelerator(hwnd, hAccTable, &msg)) {
			(void) TranslateMessage(&msg);
			(void) DispatchMessage(&msg);
		}
	}

quitwindow:								/*  received a WM_QUIT message */
	if (bTimerOn > 0) KillTimer(hwnd, 1);	/* kill the timer now */
	bTimerOn = 0;
	nAlarmTicks = 0;						/* 1994/Aug/10 */

#ifdef ALLOWDEMO
	if (bDemoFlag) {
		if (dtime > onemonth * 6)
			for(;;) OutputDebugString(demowarn);	/* EXPIRED ! */
	}
#endif

/*  time to get rid of some stuff */  /* => moved back from  WM_DESTROY */
/*	return (int) (msg.wParam); */ /* no, clean up first */

/*	do some cleanup here */
/*	if (hDriver != NULL) FreeLibrary (hDriver);	*/
	if (hTIFFLibrary >= HINSTANCE_ERROR) 
		(void) FreeLibrary(hTIFFLibrary);		/* free TIFFREAD.DLL */

	if (bUseATM && bATMLoaded) UnloadATM();		/* in winsearc.c */

/*  Free up fonts so Debug Kernel doesn't complain */
/*	closeup(hWnd, 0); */
	hDC = GetDC(hwnd);
	if ((UINT) hFontOld > 1) {	/* Avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hDC, hFontOld);
	}
	resetfonts();
	(void) ReleaseDC(hwnd, hDC);

/*  move here from WM_DESTROY */
//	freeall();						/* moved out 95/Dec/15 */
/*	if (hApplications != NULL) (void) GlobalFree(hApplications); */
	if (hPrintDC != NULL) (void) DeleteDC(hPrintDC);
/*	in WIN32 hPrevInstance us always zero */
	if (! hPrevInstance) {			/* if this was first instance ? */
		if (bSpinOK != 0) {			/* Try and unregister window class */
			flag = UnregisterClass(szControlName, hInst);
			if (flag != 0) bSpinOK = 0;
/* UnregisterClass returns zero if some window still registered using class */
		}
	}
#ifdef USEPALETTE
	if (hPal != NULL) DeleteObject(hPal);	/* 1996/March/26 */
#endif
	(void) DeleteObject(hFigurePen);
	(void) DeleteObject(hBorderPen);
	(void) DeleteObject(hLinerPen);
	(void) DeleteObject(hGreyPen);
	(void) DeleteObject(hLightGreyPen);	
	if (hConsoleFont != NULL) DeleteObject(hConsoleFont);
	if (hRulePen != NULL && hRulePen != hRulePenDefault) 
		(void) DeleteObject(hRulePen);
	if (hRulePenDefault != NULL) (void) DeleteObject(hRulePenDefault);
	if (hRuleBrush != NULL && hRuleBrush != hRuleBrushDefault)
		(void) DeleteObject(hRuleBrush);
	if (hRuleBrushDefault != NULL) (void) DeleteObject(hRuleBrushDefault);
/*  don't need to get rid of brushes - they are all stock objects */
	if (hRgn != NULL) DeleteObject(hRgn);			  /* flush region */
	if (hUpdateRgn != NULL) DeleteObject(hUpdateRgn); /* flush region */	
/*	following mostly to keep BoundsChecker happy ... */
	if (hHelpMenu != NULL) DestroyMenu(hHelpMenu);	/* 1995/Sep/17 */
	if (hTeXMenu != NULL) DestroyMenu(hTeXMenu);	/* 1994/Nov/20 */
	if (hEnvMenu != NULL) DestroyMenu(hEnvMenu);	/* 1999/Jan/10 */

	free_fontmap();				/* in winpslog.c */ /* 1994/Nov/20 */

	if (idServerInst != 0) DDEServerStop();		/* 98/Dec/12 */

/*	if (bDebug) winerror("About to do saving"); */	/* debug */

	maybesaveprefer();			/* write preferences if appropriate */

	freeall();						/* moved down 2000/March/15 */

/*	showstackused (hwnd, 0); */	/* 94/June/21 */
	showstackused (0);
//	do only only in NT - not implemented in 95/98
	if (bWinNT && bDebug) {
		if (HeapValidate(GetProcessHeap(), 0, NULL) == 0) 
			winerror("Heap invalid upon exit");
//		else winerror("HEAP OK!");
	}
	(void) DestroyWindow(hwnd); /* ??? 92/01/25 */
	UnregisterClass(DviWindoClass, hInst);	/* 97/Jan/5 */
	return (int) (msg.wParam);  /* returns wParam given to WM_QUIT message */
}	/* end of WinMain */

/* Separated out to setup TEXFONTS or encoding specific env var string */
/* reads env vars ENCODING and whatever the value of that is */

void setupTeXFonts (void) {		/* 97/Apr/20 */	/* separated in case */
/*	char buffer[128]; */		/* space for encoding variable if any */
	char buffer[256];			/* space for encoding variable if any */
	char *s;
	
	if (szTeXFonts != NULL) {	/* this may be called when encoding changes */
		free(szTeXFonts);
		szTeXFonts = NULL;
	}
	*str = '\0';				/* moved here 97/May/10 */
/*	Try and pick up encoding specific environment variable */
	(void) GetPrivateProfileString(achEnv, "ENCODING", "",
								   buffer, sizeof(buffer), achFile);	
/*	if (*buffer, "" != '\0') *//* bug fixed 98/Mar/26 */
	if (*buffer != '\0') {		/* if ENCODING defined, get that env var */
		if ((s = strchr(buffer, '.')) != NULL) *s = '\0';	/* trim extension */
		(void) GetPrivateProfileString(achEnv, buffer, "",
									   str, sizeof(str), achFile);
	}
	if (*str == '\0')	/* if no encoding specific var, use TEXFONTS */
		(void) GetPrivateProfileString(achEnv, "TEXFONTS", "",
									   str, sizeof(str), achFile);	
	if (*str != '\0') szTeXFonts = zstrdup(str);
/*	97/May/10 --- too early? bDebug not set yet */
	if (bDebug > 1 && szTeXFonts != NULL) {
		strcpy(debugstr, "TeXFonts=");
		strcat(debugstr, szTeXFonts);
		OutputDebugString(debugstr);
	}
}

/* use DOS environment vars --- unless defined */
/* in [Environment] section of `dviwindo.ini' */

void CheckEnvironment (void) {
/*  search environ for DVIPATH, PSPATH, VECPATH, PREPATH, TEXFONTS, TEXDVI ? */
/*	use getenv instead ? NO, would be less efficient since repeated search */
	int k = 0;				

/*	start by reading DOS env vars */ /* override by dviwindo.ini env vars later */
	while (_environ[k] != 0) {
		if (strcmp(_environ[k], "") == 0) break;
#ifdef DEBUGENVIRONMENT
		if (bDebug > 1) OutputDebugString(_environ[k]); 
#endif
/*	environment variable DVIPATH pretty much useless at this point ??? */
/*		if (strncmp(_environ[k], "DVIPATH=", 8) == 0) */
		if (strncmp(_environ[k], "DVIPATH", 7) == 0 &&
			_environ[k][7] == ':') {
			if (*DefPath == '\0') {	   /* if not in Private Profile */
				strncpy(DefPath, _environ[k] + 8, sizeof(DefPath)-2);
				strcat(DefPath, "\\");
			}
		}
		else if (strncmp(_environ[k], "VECPATH", 7) == 0 &&
				 _environ[k][7] == ':') {
			szVecPath = zstrdup(_environ[k] + 8);
		}
		else if (strncmp(_environ[k], "PREPATH", 7) == 0 &&
				 _environ[k][7] == ':') {
			szPrePath = zstrdup(_environ[k] + 8);
		}
/* have to use the name PSPATH instead of EPSPATH because of Epsilon ... */
/* NOTE: this can be a real search path, so don't append \\ */
		else if (strncmp(_environ[k], "PSPATH", 6) == 0 &&
				 _environ[k][6] == ':') {
			szEPSPath = zstrdup(_environ[k] + 7);
		}
		else if (strncmp(_environ[k], "TEXFONTS", 8) == 0 &&
				 _environ[k][8] == ':') {
			szTeXFonts = zstrdup(_environ[k] + 9);
		}
		else if (strncmp(_environ[k], "YANDYPATH", 8) == 0 &&
				 _environ[k][8] == ':') {		/* 96/Aug/30 */
			szBasePath = zstrdup(_environ[k] + 9);
/*			strcpy(str, _environ[k] + 9;
			s = str + strlen(str) - 1;
			if (*s == '\\' || *s == '/') *s = '\0';
			strcat (str, "\\");
			szBasePath = zstrdup(str); */ /* ? */
		}
/*		TEXDVI redirects output from Y&YTeX - need for TeX Menu */
		else if (strncmp(_environ[k], "TEXDVI", 6) == 0 &&
				 _environ[k][6] == ':') {
			szTeXDVI = zstrdup(_environ[k] + 7);
/*			bUseTeXDVI = 1;
			if (strcmp(*DefPath == '\0') {
				strncpy(DefPath, _environ[k] + 7, sizeof(DefPath)-2);
				strcat(DefPath, "\\");
			} */ /* done below now 1994/May/23 */
		}
		else if (strncmp(_environ[k], "USEDVIWINDOINI", 14) == 0 &&
				 _environ[k][14] == ':') {				 
			sscanf(_environ[k] + 15, "%d", &bUseDVIWindo);
		}						/* new 1994/June/14 */
		if(k++ > 1024) break;	/* just in case ... */
	}

	if (bUseDVIWindo) {			/* new control 1994/June/14 */
/*	following added 1994/May/23 --- uses dviwindo.ini for initialization */
/*  search dviwindo.ini for DVIPATH, PSPATH, VECPATH, TEXDVI, TEXFONTS ? */
		ReadPaths();			/* separated out 98/Dec/24 */

/*		deal with encoding specific environment variables 97/Apr/20 */
		setupTeXFonts();
/*		end of dealing with encoding specific environment variables */
	}		/* end of [Environment] code section */

/*	if TeXDVI given either in environment or in dviwindo.ini, do some extra */
/*	if (*TeXDVI != '\0') */
	if (szTeXDVI != NULL) {	
		bUseTeXDVI = 1;							/* make a note to use this */
/*		maybe use this for DVIPATH also ? */
		if (*DefPath == '\0') {			/* if not in Private Profile */
			strncpy(DefPath, szTeXDVI, sizeof(DefPath)-2);
			strcat(DefPath, "\\");
		}
	}
}

#ifdef DEBUGENVIRONMENT
void ShowEnvironment (void) {				/* split off 95/May/15 */
	if (bDebug < 2) return;					/* avoid nasty problem */
	if (szBasePath != NULL) {
		sprintf(debugstr, "YANDYPATH=%s\n", szBasePath),	/* 96/Aug/29 */
		OutputDebugString(debugstr);
	}
	if (szEPSPath != NULL) {
		sprintf(debugstr, "PSPATH=%s\n", szEPSPath),
		OutputDebugString(debugstr);
	}
	if (szVecPath != NULL) {
		sprintf(debugstr, "VECPATH=%s\n", szVecPath),
		OutputDebugString(debugstr);
	}
	if (szPrePath != NULL) {
		sprintf(debugstr, "PREPATH=%s\n", szPrePath),
		OutputDebugString(debugstr);
	}
	if (szDefPath != NULL) {
		sprintf(debugstr, "DVIPATH=%s\n", szDefPath),
		OutputDebugString(debugstr);
	}
	if (szTeXDVI != NULL) {
		sprintf(debugstr, "TEXDVI=%s\n", szTeXDVI),
		OutputDebugString(debugstr);
	}
	if (szTeXFonts != NULL) {
		sprintf(debugstr, "TEXFONTS=%s\n", szTeXFonts),
		OutputDebugString(debugstr);
	}
}	/* debugging output only - flush later */
#endif

/****************************************************************************

	FUNCTION: InitApplication(HANDLE)

	PURPOSE: Initializes window data and registers window class

****************************************************************************/

BOOL InitApplication(HINSTANCE hInstance) {
	int flag;
	WNDCLASS  wc;

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("InitApplication\n");
#endif

/*	wc.style = NULL; */
	wc.style = CS_DBLCLKS;
/*	wc.lpfnWndProc = MainWndProc; */
	wc.lpfnWndProc = (WNDPROC) MainWndProc; 
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, "DviIcon");
	wc.hCursor = NULL;			/* so we can draw our own */
/*	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1); */ /* ??? */
	wc.hbrBackground = NULL; 	/* so we can control it 98/Jun/5 */
/*	wc.lpszMenuName =  "DviWindoMenu"; */
	wc.lpszMenuName =  DviWindoMenu;
/*	wc.lpszClassName = "DviWindoWClass"; */
	wc.lpszClassName = DviWindoClass;

	flag = RegisterClass(&wc);	/* register main window class */
	if (flag == 0) return flag;
/*	also register spinner window class */
/*  avoid registering spinner several times if multiple instances */
	bSpinOK = RegisterControlClass(hInstance);
	return flag;
}

void checkpaper(HWND, int);
void checkunits(HWND, int);

void MakeRuleDefault (COLORREF TextColor) {
/*	first remove old ones in case ??? problem if selected in hDC ? */
	if (hRuleBrushDefault != NULL)
		(void) DeleteObject(hRuleBrushDefault);
	if (hRulePenDefault != NULL)
		(void) DeleteObject(hRulePenDefault);
	hRuleBrushDefault = CreateSolidBrush(TextColor); 
	hRulePenDefault = CreatePen(PS_SOLID, 0, TextColor); 	
}

// separated out so can reuse 2000 Aug 2

char *SetupVersionString (OSVERSIONINFO OSVersionInfo, int flag) {
	char *s;

	OSVersionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&OSVersionInfo);

	if (flag) bWin95 = bWin98 = bWinNT = bWinNT5 = bNewShell = 0;	// default

	switch (OSVersionInfo.dwPlatformId) {
		case VER_PLATFORM_WIN32s:				/* UGH */
			if (flag) bWin95 = bWin98 = bWinNT = bWinNT5 = bNewShell = 0;
			s = "Win32s on Windows 3.1 (UGH!)";
			break;
		case VER_PLATFORM_WIN32_WINDOWS:		/* Windows 95/98 */
			if (flag) {
				bWinNT = bWinNT5 = 0;
				bWin95 = bNewShell = 1;
			}
			if (OSVersionInfo.dwMinorVersion > 0) {
				if (flag) bWin98 = 1;
				s = "Win32 on Windows 98";
			}
			else {
				if (flag) bWin98 = 0;
				s = "Win32 on Windows 95";
			}
			break;
		case VER_PLATFORM_WIN32_NT:				/* Windows NT */
			if (flag) {
				bWin95 = bWin98 = 0;
				bWinNT = 1;
			}
			if (OSVersionInfo.dwMajorVersion >= 5) {
				if (flag) bNewShell = bWinNT5 = 1;	/* 98/Nov/15 */
				s = "Windows 2000";
			}
			else if (OSVersionInfo.dwMajorVersion >= 4) {
				if (flag) {
					bNewShell = 1;
					bWinNT5 = 0;
				}
				s = "Windows NT 4";
			}
			else {								/* NT 3.51 */
				if (flag) bNewShell = bWinNT5 = 0;
				s = "Windows NT";
			}
			break;
		default:								/* What ? */
			bWin95 = bWin98 = bWinNT = bWinNT5 = bNewShell = 0;
			s = "Unknown";
	}
	return s;
}


/****************************************************************************

	FUNCTION:  InitInstance(HANDLE, int, int, int, int, int)

	PURPOSE:  Saves instance handle and creates main window

****************************************************************************/

BOOL InitInstance (HINSTANCE hInstance, int nCmdShow,
		int x, int y, int cx, int cy) {
/*	int n; */
/*	UINT k; */
	HDC hDC;
	HFONT hFontSystem;
/*	LOGPEN BorderPen, LinerPen, FigurePen; */
/*	int r, g, b; */
	RASTERIZER_STATUS rsStat;		/* 1995/June/29 --- good place  ?*/
	char *s;
/*	HMODULE hModule; */				/* 1996/Jan/28 */
	OSVERSIONINFO OSVersionInfo;	/* 1996/April/6 */
	WORD n;							/* 1996/April/6 */

	hInst = hInstance;				/* store globally for later use */

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("InitInstance\n");
#endif

/*	hAccTable = LoadAccelerators(hInst, "DviWindoMenu"); */

	xLeft = x; yTop = y; cxWidth = cx, cyHeight = cy;		/* initialize 97/Aug/3 */
	hwnd = CreateWindow(DviWindoClass,	TitleText,
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
/*		AN EXPERIMENT 1993/Feb/21 : */
/*		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL | WS_CLIPCHILDREN, */
		x,			/* CW_USEDEFAULT, */
		y,			/* CW_USEDEFAULT, */
		cx,			/* CW_USEDEFAULT, */
		cy,			/* CW_USEDEFAULT, */
		NULL,		/* no parent */
		NULL,		/* class menu */
		hInstance,	/* creator */
		NULL		/* params */
	);

	if (!hwnd)	return (FALSE);

/*	sprintf(str, "bMaximized %d FOO", bMaximized);
	wininfo(str); */ /* DEBUGGING */

/*	Get cursors */

	hArrow = LoadCursor(NULL, IDC_ARROW);
	hHourGlass = LoadCursor(NULL, IDC_WAIT);
/*	hBlankCursor = LoadCursor(NULL, IDC_CROSS); */
	hBlankCursor = LoadCursor(hInst, "Blank");
	hHandCursor = LoadCursor(hInst, "DviHand");
	hZoomCursor = LoadCursor(hInst, "Magnifier");
	hCopyCursor = LoadCursor(hInst, "Scissors");
	CenterThumbs(hwnd);
	HideScrollBars(hwnd);
	
/*	Now try and set up bWin95, bWin98, bWinNT, bNewShell ! */

/*  Moved here so can set up EncodingVector BEFORE reading preferences */
	CombinedVersion = GetVersion();
/*	Debugging Output 96/Nov/29 */
/*	sprintf(str, "%08X", CombinedVersion); */
/*	(void) WritePrivateProfileString(achPr, "CombinedVersion", str, achFile); */

/*	According to MSDN 1996 April the above works now as follows (WIN32?): */
/*	HIWORD(CombinedVersion) = build number (e.g. 1209 for Windows NT 4.0) */
/*	*except* mask off two high bits of HIWORD which are used for WIN32 info */
/*	These two bits are on in Windows 95, and the build number is zero ... */
/*	LOBYTE(LOWORD(CombinedVersion)) = major version number (e.g. 4) */
/*	HIBYTE(LOWORD(CombinedVersion)) = minor version number (e.g. 0) */	
/*	They recommend using GetVersionEx() instead */

	WindowVersion = LOWORD(CombinedVersion);

/*	Flip Bytes of Version Number and recombine them */ /* 95/Nov/3 */
/*	WinVerNum = (LOBYTE(WindowVersion) << 8) | HIBYTE(WindowVersion); */
/*	WinVerNum = (WORD) (LOBYTE(WindowVersion) << 8) | HIBYTE(WindowVersion); */
	WinVerNum = (WORD) ((LOBYTE(WindowVersion) << 8) | HIBYTE(WindowVersion)); /* 99/Mar/17 */

/*	New Shell if highest bit set ??? */ /* first guess only ... */
	if (CombinedVersion < 0x80000000) bNewShell = FALSE;	/* ??? */
	else bNewShell = TRUE;
/*	May want to base this instead on version number being 4.00 or greater */
/*	The above does *not* work correctly, since those bits are zero in NT */
/*	Currently this flag is only used to shorten file name list... */
/*	and to decide whether ttfonts.reg worth reading */

/*	a debugging experiment only for WIN16 version */
/*	in this case HIWORD is DOS version number */
/*	if (WinVerNum >= 0x035F) bNewShell = TRUE; */
/*	but Windows NT return 0x0310 to WIN16 prog, how tell version ? */

/*	set default dialog box for font list unless overridden from ini file */
	bCommFont = TRUE;		/* unless overridden from ini file */

/*	In WIN32 hiword is 0 if WIN32 and 1 if WIN32S in WIN 3.1 ??? */
/*	Two high bits of HIWORD of CombinedVersion used for WIN32 info: */
/*		WIN32s:		11 */
/*		Windows 95:	01 */
/*		Windows NT: 00 */
	n = (WORD) HIWORD(CombinedVersion);		/* 1996/Apr/6 */
	n = (WORD) (n >> 14);					/* look at top two bits */
	if (n == 0) {
/*		bWin95 = 0; bWinNT = 0; */
		bWin95 = 0; bWinNT = 1;
	}
	else if (n == 1) {
/*		bWin95 = 1; bWinNT = 1; */
		bWin95 = 1; bWinNT = 0;
	}
	else {
		bWin95 = 0; bWinNT = 0;			/* n == 3 in WIN32S ??? */
	}
/*	May need to check this ... */

/*	DOSVersion = HIWORD(CombinedVersion); */

/*	HIBYTE(DOSVersion), LOBYTE(DOSVersion), */
/*	LOBYTE(WindowVersion), HIBYTE(WindowVersion)); */

/*	Following testing may not be reliable ... Windows NT 4.0 same as 95 ? */
	if (WinVerNum >= 0x0400) bWin95 = TRUE;		/* Windows 4.00 or later */
	else bWin95 = FALSE;
//	if (WinVerNum > 0x0400) bWin98 = TRUE;		/* Windows 4.10 or later */
	if (WinVerNum >= 0x0410) bWin98 = TRUE;		/* Windows 4.10 or later */
	else bWin98 = FALSE;

/*	Set bATM4 when loading ATM and getting version number */

/*	How do we tell whether it is running in Windows NT ? */
	if ((s = getenv("OS")) != NULL) {
		if (strcmp(s, "Windows_NT") == 0) bWinNT = TRUE;
		else bWinNT = FALSE;
	}
/*	Presently bWinNT is used only in T1INSTALL error message, bWin95 is used */

/*	New code using GetVersionEx 1996/April/6 */

	OSVersionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&OSVersionInfo);
	s = SetupVersionString(OSVersionInfo, 1);
	
/*	dwBuildNumber is what it says in NT, but has version in HIWORD in 95 */ 
/*	sprintf(str, "%d.%d Build: %d Platform: %s CSD: %s", */
/*	sprintf(str, "%d.%d Build: %d Platform: %s %s %s",
			OSVersionInfo.dwMajorVersion, OSVersionInfo.dwMinorVersion,
			(int) LOWORD(OSVersionInfo.dwBuildNumber), s,
			*(OSVersionInfo.szCSDVersion) != '\0' ? "CSD:" : "",
			OSVersionInfo.szCSDVersion); */
	sprintf(str, "%d.%d Build: %d Platform: %s",
			OSVersionInfo.dwMajorVersion, OSVersionInfo.dwMinorVersion,
			(int) LOWORD(OSVersionInfo.dwBuildNumber), s);
	if (*(OSVersionInfo.szCSDVersion) != '\0') {
		strcat (str, " CSD: ");
		strcat(str,	OSVersionInfo.szCSDVersion);
	}
//	(void) WritePrivateProfileString(achPr, "Windows", str, achFile);
	(void) WritePrivateProfileString(achDiag, "Windows", str, achFile);

/*	Allow override from dviwindo.ini 96/Nov/30 ??? */
	bNewShell = GetPrivateProfileInt(achPr, "NewShell", bNewShell, achFile);

/*	Now set various flags that depend on bWin95 and bWinNT */

#ifdef USEUNICODE
/*	if (bWinNT && bNewShell) */	/* no, works both in NT 3.51 and NT 4.0 */
	bUseNewEncodeTT = bUseNewEncodeT1 = 0;
	if (bWinNT) bUseNewEncodeTT = bUseNewEncodeT1 = 1;
/*	if (bWin95) bUseNewEncodeTT = 1; */	/* ??? */
/*	can override using NewEncodeTT=...	NewEncodeT1=... in dviwindo.ini */
/*	should veto bTTRemap if bUseNewEncodeTT ? */
#endif
/*	In Windows NT GetProfile on [Fonts] refers to registry anyway */
/*	And we don't trust registry much because of mangled FontNames */
	if (bWinNT) bUseRegEnumValue = 0;	/* 97/Feb/15 ? */
	if (bWin95) bUseRegEnumValue = 1;	/* 97/Feb/15 ? */
/*	In Windows NT ATMSelectObject does not work */
	if (bWinNT) bATMPrefer = 0;			/* 97/Mar/2 */
	if (bWin95) bATMPrefer = 1;			/* 97/Mar/2 */

#ifdef AllowCTM
/*	Advanced Graphics only works in Windows NT */
	if (bWinNT) bUseAdvancedGraphics = 1;	/* 97/Feb/26 ? */
	else bUseAdvancedGraphics = 0;
#endif

/*	allow override from dviwindo.ini 96/Nov/30 ??? */
/*	bNewShell = GetPrivateProfileInt(achPr, "NewShell", bNewShell, achFile); */

/*	if (bWinNT == 0) bUseCTMflag = 0; */ /* ??? */

/*	Will the following `DOS' code work in WIN32 ? */
/*	get `working directory' found when DVIWindo starts up */
/*	maybe overwritten later by `WorkingDirectory=' in `dviwindo.ini' */	

/*	_getcwd(IniDefPath, sizeof(IniDefPath)); */

//	DO WE REALLY WANT TO DO THIS ???  What is it for ???
//	REPLACED LATE WITH WORKING DIRECTORY FROM dviwindo.ini ???

//	(void) GetCurrentDirectory(sizeof(IniDefPath), IniDefPath);
	(void) GetCurrentDirectory(sizeof(str), str);
	s = str + strlen(str) - 1;	/* 1995/July/6 */
	if (*s != '\\' && *s != '/') strcat(str, "\\");
//	if (IniDefPath == NULL) free(IniDefPath);	// can't happen
	if (IniDefPath != NULL) free(IniDefPath);	// can't happen
	IniDefPath = zstrdup(str);

/*	if (bDebug > 1) {
		sprintf(debugstr, "Initial Working Directory: %s\n", IniDefPath);
		OutputDebugString(debugstr);
	} */											/* JUST A TEST ! */
/*	This is the directory from the Program Item that launched DVIWindo */
/*	It has no terminating separator, unless it is top level directory */
/*	The above means there is always available *some* Working Dir */

/*	Setting up szExeWindo - has been */
/*	moved into WinMain, so szExeWindo available when ATM being set up */

	CheckEnvironment ();		/* look for environment variables */

/*	for (k=0; k < 16; k++) {
		n = GetDriveType(k);
		sprintf(debugstr, "%c: ", 'A' + k);
		switch (n) {
			case 0: strcat(debugstr, "UNDETERMINED / UNINSTALLED"); break;
			case DRIVE_FIXED: strcat(debugstr, "FIXED / HARD DRIVE"); break;
			case DRIVE_REMOVABLE: strcat(debugstr, "REMOVABLE / FLOPPY"); break;
			case DRIVE_REMOTE: strcat(debugstr, "REMOTE / CD ROM"); break;
		}
		strcat(debugstr, "\n");
		if (bDebug > 1) OutputDebugString(debugstr);
	} */ /* test code for GetDriveType ... */

/* GetDriveType works different in WIN32 --- needs null terminated string */

	nDriveD = GetDriveType("d:\\");	/* get type of drive d: */

/*	nDriveD = GetDriveType(3); */

	bPauseCalls = GetPrivateProfileInt(achEnv, "DEBUGPAUSE", 0, achFile);

/*	following doesn't work - minimizes window instead ... */
/*	if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0) */
/*	following doesn't work - shows properties of icon instead ... */
/*	if ((GetAsyncKeyState(VK_MENU) >= 0) */			
	if (GetAsyncKeyState(VK_CONTROL) >= 0) {
		readpreferences();			/* get preferences from Profile file */
	}
	else {	/* holding control key while double clicking on DVIWindo icon */
		if (wincancel("Not reading preferences?") != 0) {
			DestroyWindow(hwnd);		/* don't start if user CANCELs */
			UnregisterClass(DviWindoClass, hInst);	/* 97/Jan/5 */
			return (FALSE);
		}
		else bSavePrefer = 0;			/* to avoid overwriting later */
	}
	ReadFixedPrefer();		/* read stuff not saved by WritePreferences */
	SetCommandStrings();	/* moved here 97/Apr/3 */
	ReadEnvVars();			/* split off 98/Dec/24 */

	setpagesize(papertype);
	checkpaper(hwnd, papertype);
	checkunits(hwnd, units);	/* 1993/Feb/21 */
	checkduplex(hwnd, nDuplex);			/* duplex check mark */
	checkpreferences(hwnd);		/* make sure the right things are checked */

	nCheckANSI = codefourty("ansinew"); 	/* in winanal.c */

	hEncoding = NULL;			/* reset the global value */
#ifdef USEUNICODE
	if (bUseATM || bUseNewEncodeTT || bUseNewEncodeT1)
		(void) ReadEncoding(0);					/* winsearc.c 97/Jan/17 */
#else
	if (bUseATM) (void) ReadEncoding(0);	/* winsearc.c 97/Jan/17 */
#endif

#ifdef USEUNICODE
/*	if (bUseNewEncodeTT) bTTRemap=0; */		 /* ??? also, too early ??? */
#endif
/*	if (bUseNewEncodeT1) bUseATM = 0; */ /* ??? */

//	ShowConfiguration(debugstr);
//		wincancel(debugstr);		// debugging only
	if (bUseATM) LoadATM();		/* in winsearc.c 1994/Dec/22 */

	hDC = GetDC(hwnd);
	hFontSystem = GetStockObject(SYSTEM_FONT);
	hFontOld = SelectObject(hDC, hFontSystem); /*  remember current font */
	if ((UINT) hFontOld > 1)	/* Avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hDC, hFontOld);

	TextColor = GetSysColor(COLOR_WINDOWTEXT);
	BkColor = OrgBkColor = GetSysColor(COLOR_WINDOW);
/*	if (bDebug > 1) {
		sprintf(debugstr, "TextColor %X BkColor %X", TextColor, BkColor);
		OutputDebugString(debugstr);
	} */
	MakeRuleDefault(TextColor);
/*	hbrBackground = CreateSolidBrush(BkColor); */		/* ??? */
/*	hOldBrush = SelectObject(hDC, hbrBackground); */	/* ??? */

/*	should also catch WM_SYSCOLORCHANGE */

/*	sprintf(debugstr, "Text: %08X (%ld) Background: %08X (%ld)\n", 
		TextColor, BkColor);
	if (bDebug > 1) OutputDebugString(debugstr); */

	resetinfodetails();			/* reset DVI file information */
	setupremaptable();			/* set up remap table for CM fonts */
	initialfonts();				/* reset table of font handles */

/*	hBorderPen = CreatePen(PS_SOLID, 10, RGB(255, 0, 255)); */
	hBorderPen = CreatePen(PS_SOLID, 10, RGB(RBorderPen, GBorderPen, BBorderPen)); 
/*	BorderPen.lopnStyle = PS_SOLID;
	BorderPen.lopnWidth = 10;
	BorderPen.lopnColor = RGB(RBorderPen, GBorderPen, BBorderPen);
	hBorderPen = CreatePenIndirect(&BorderPen); */
/*	hLinerPen = CreatePen(PS_SOLID, 10, RGB(0, 255, 255)); */
	hLinerPen = CreatePen(PS_SOLID, 10, RGB(RLinerPen, GLinerPen, BLinerPen));
/*	LinerPen.lopnStyle = PS_SOLID;
	LinerPen.lopnWidth = 10;
	LinerPen.lopnColor = RGB(RLinerPen, GLinerPen, BLinerPen);
	hLinerPen = CreatePenIndirect(&LinerPen); */
/*	hFigurePen = CreatePen(PS_SOLID, 10, RGB(128, 0, 128)); */
	hFigurePen = CreatePen(PS_SOLID, 10, RGB(RFigurePen, GFigurePen, BFigurePen));
/*	FigurePen.lopnStyle = PS_SOLID;
	FigurePen.lopnWidth = 10;
	FigurePen.lopnColor = RGB(RFigurePen, GFigurePen, BFigurePen);
	hFigurePen = CreatePenIndirect(&FigurePen); */
	FrameColor = RGB(RFigurePen, GFigurePen, BFigurePen);		/* 94/Sep/20 */

	hBlackPen = GetStockObject(BLACK_PEN);			/* used ? */
	hErasePen = GetStockObject(WHITE_PEN);			
	hGreyPen = CreatePen(PS_SOLID, 10, RGB(128, 128, 128)); 
	hLightGreyPen = CreatePen(PS_SOLID, 10, RGB(192, 192, 192)); 
	hBlackBrush = GetStockObject(BLACK_BRUSH);		/* used ? */
	hGreyBrush = GetStockObject(GRAY_BRUSH);
	hLightGreyBrush = GetStockObject(LTGRAY_BRUSH);
/*	hbrDefaultBackground = (HBRUSH) (COLOR_WINDOW+1); */ /* gives white */
	hbrDefaultBackground = CreateSolidBrush(OrgBkColor); /* 98/Sep/6 ??? */
	hbrBackground = hbrDefaultBackground;
/*	hOldBrush = SelectObject(hDC, hBlackBrush); */	/* 1992/May/06 */
	hFaceNames = NULL;			/* indicate no memory allocated */
	hFullNames = NULL;			/* indicate no memory allocated */
	hFileNames = NULL;			/* indicate no memory allocated */
	hWidths = NULL;				/* indicate no memory allocated */
	hPages = NULL;				/* indicate no memory allocated */
	hColor = NULL;				/* indicate no memory allocated */
	hTPIC = NULL;				/* indicate no memory allocated */
/*	hMarks = NULL; */			/* indicate no memory allocated */
	memset (&pd, 0, sizeof(PRINTDLG));	/* reset print dialog info */
	pd.lStructSize = sizeof(PRINTDLG);	/* set up size field */
	pd.hDevMode = NULL;			/* PRINTDLG 1995/Dec/14 */
	pd.hDevNames = NULL;		/* PRINTDLG 1995/Dec/14 */

/*	if (bHyperText != 0) (void) AllocMarks(MAXHYPERPUSH); */

	(void) buildpalettemap(hDC);	/* make palette table for colored fonts */
	(void) ReleaseDC(hwnd, hDC);

/*  look at _argc and _argv ? */

/*	checkpreferences(hwnd);	*//* make sure the right things are checked ... */

/*	if (LOBYTE(GetVersion()) < 3) {
		winerror("DVIWindo requires Windows 3.0 or later");
		return (FALSE);
	} */

/*  use GetDOSEnvironment() instead ? Windows 3.0 ... (only need for DLL) */
/*	need to add underscore to `environ' for C 7.0 compiler */

/*	CheckEnvironment (); */

/*	(void) ShowWindow(hwnd, nCmdShow); */	/* Show the window   		*/
/*	UpdateWindow(hwnd);				*/		/* Sends WM_PAINT message	*/

	if (serialnumber != REEXEC * 101) bPrivate = 0; 
	else bPrivate = 1;		/* unless it is me! */

	if (GetSystemMetrics(SM_DEBUG) == 0) bDebugKernel = 0;
	else bDebugKernel = 1;		/* debug version of Windows running */

/*	get `working directory' found when DVIWindo starts up */
/*	maybe overwritten later by `WorkingDirectory=' in `dviwindo.ini' */	

/*	CheckEnvironment (); */		/* look for environment variables */

/*	WindowVersion = GetVersion(); */
/*	GetWinFlags not supported in WIN32 */
	if (szWinDir == NULL) {
		(void) GetWindowsDirectory(str, sizeof(str));
		szWinDir = zstrdup(str);					/* 96/July/23 */
//		winerror(szWinDir);			// debugging only
	}

/* The GetRasterizerCaps function determines whether the TrueType rasterizer */
/* is enabled and whether any TrueType fonts are actually installed in the */
/* system. The rasterizer's enabled status can be changed only by restarting */
/* Windows, so it remains constant in a session. The availability of TrueType*/
/* fonts in the system can change during a session because fonts can be added. */

/* This is a Windows 3.1 function ... */

	GetRasterizerCaps(&rsStat, sizeof(rsStat));	
	if (rsStat.wFlags & (TT_ENABLED | TT_AVAILABLE) ==
		(TT_ENABLED | TT_AVAILABLE)) bTTUsable = TRUE;
	else bTTUsable = FALSE;

	return (TRUE);
}	/* nCmdShow unreferenced */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void CenterThumbs (HWND hWnd) {
	(void) SetScrollPos(hWnd, SB_VERT, 50, TRUE); /* set thumb */
	(void) SetScrollPos(hWnd, SB_HORZ, 50, TRUE); /* set thumb */
}

void HideScrollBars (HWND hWnd) {
	ShowScrollBar(hWnd, SB_BOTH, FALSE); /* hide scroll bars */
/*	bScrollBars = 0; */
}

void ExposeScrollBars (HWND hWnd) {
	ShowScrollBar(hWnd, SB_BOTH, TRUE); /* show scroll bars */
/*	bScrollBars = 1; */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define MAXREMAP 31 */ /* for old CM fonts (c:\y&yfonts) */

#define MAXREMAP 32

void setupremaptable (void) {
	int k;

 	for (k = 0; k < 10; k++) remaptable[k] = (WORD) (k + 161);
	for (k = 10; k < MAXREMAP+1; k++) remaptable[k] = (WORD) (k + 163);
	for (k = MAXREMAP+1; k < 127; k++) remaptable[k] = (WORD) k;
	remaptable[127] = (WORD) 196;

/* 	for (k = 0; k < 10; k++) remaptable[k] = (unsigned char) (k + 161);
	for (k = 10; k < MAXREMAP+1; k++) 
		remaptable[k] = (unsigned char) (k + 163);
	for (k = MAXREMAP+1; k < 127; k++)
		remaptable[k] = (unsigned char) k;
	remaptable[127] = (unsigned char) 196; */

/*  second half flushed to make space 1995/June/23 */
/*	for (k = 128; k < 256; k++) remaptable[k] = (unsigned char) k; */
}

void resetinfodetails(void) {
	strcpy(dvi_comment, "No DVI file open");
	dvi_num = 25400000; dvi_den = 473628672; dvi_mag = 1000;
	dvi_l = 0; dvi_u = 0; dvi_bytes = 0;
	dvi_s = 0; dvi_t = 0; dvi_fonts = 0; dvi_version = 0;
}

void resetusedetails(void) {
	return;
}

int VScrollPos(HWND);
int HScrollPos(HWND);

void resetgraphicstate(void) {
	graphicindex = 0;
}

void checkmagnify(HWND hWnd) {		/* 1993/March/27 */
	HMENU hMenu;	
	hMenu = GetMenu(hWnd); 
	(void) EnableMenuItem(hMenu, IDM_UNMAGNIFY, 
		(wantedzoom > MINZOOM) ? MF_ENABLED : MF_GRAYED);
	(void) EnableMenuItem(hMenu, IDM_MAGNIFY, 
		(wantedzoom < MAXZOOM) ? MF_ENABLED : MF_GRAYED);
	DrawMenuBar(hWnd); 
}

/* Reset mapping to factory default */

/* void resetmapping(void) */
void resetmapping(HWND hWnd, int flag) {
	xoffset = 0;
	yoffset = 0;
	wantedzoom = 0;
	if (flag != 0) checkmagnify(hWnd);	/* 1993/March/27 */
/*	graphicindex = 0; */		/* resetgraphicstate(); */
/*	savegraphicstate();	 */
/*	setscale(wantedzoom); */
}

/* reset mapping to page default mapping */

/* void resetpagemapping(void) */	/* NEW 1991 Dec 26 */
void resetpagemapping(HWND hWnd) {	
/*	xoffset = pagexoffset;
	yoffset = pageyoffset;
	wantedzoom = pagewantedzoom; */
	setoffsetandzoom(1);
	checkmagnify(hWnd);	/* 1993/March/27 */
/*	graphicindex = 0; */		/* resetgraphicstate(); */
/*	savegraphicstate();	 */
/*	setscale(wantedzoom); */
}

/* adjust xoffset & yoffset when scrolling window */
/* does this actually serve a useful purpose ? */

void adjustoffset(HWND hWnd, int dx, int dy) {	/* scrolling */
	int ret; 						/* not accessed */
	HDC hDC;						/* display-context variable */
	POINT steparr[1];
	
	hDC = GetDC(hWnd);
	if (bCopyFlag == 0)				/* should never be non-zero here ! */
		(void) SetMapMode(hDC, MM_TWIPS);		/* set unit to twips */
	steparr[0].x = dx;	steparr[0].y = dy;
	ret = DPtoLP(hDC, steparr, 1);
	(void) ReleaseDC(hWnd, hDC);
	if (ret == 0) {
		if (bDebug) {
			sprintf(str, "DPtoLP failed dx %d dy %d", dx, dy);
			winerror(str);
		}
	}
	else {
		xoffset += steparr[0].x; yoffset += steparr[0].y;
	}
	if (dx != 0) HScrollPos(hWnd);		/* 1993/Feb/22 */
	if (dy != 0) VScrollPos(hWnd);		/* 1993/Feb/22 */
/*	(void) ReleaseDC(hWnd, hDC); */
}

/* used for going to default coordinates or bottom of page */
/* come here only if wantedzoom == 0 before and bFileValid != 0 */

void scrolladjust(HWND hWnd, long xoff, long yoff) {	/* scrolling */
	int ret; 						/* not accessed */
	HDC hDC;						/* display-context variable */
	int dx=0;						/* avoid uninitialized 98/Mar/26 */
	int dy=0;						/* avoid uninitialized 98/Mar/26 */
	POINT steparr[1];
	RECT UpdateRect;				/* rectangle needing update */
	RECT ClipRect;					/* rectangle of clipping region */
	int width, height;				/* shadowing global  width, height */
/*	unsigned int width, height;	*/	/* 1992/Oct/3 */
	
	if (xoff < -32767 || xoff > 32767 || yoff < -32767 || yoff > 32767) {
/*		xoffset += xoff; yoffset += yoff; */
		InvalidateRect(hWnd, NULL, TRUE); 	/* erase */ /* forget it ! */
	}
	else {
/*		wininfo("Entered scrolladjust"); */
		hDC = GetDC(hWnd);
		(void) GetClipBox(hDC, &ClipRect);
		width = (unsigned int) (ClipRect.right - ClipRect.left);
		height = (unsigned int) (ClipRect.bottom  - ClipRect.top);
/*		sprintf(str, "width %d height %d", width, height);
		wininfo(str); */
		if (bCopyFlag == 0)		/* should never be non-zero here ! */
			(void) SetMapMode(hDC, MM_TWIPS);		/* set unit to twips */
		steparr[0].x = (int) xoff; steparr[0].y = (int) yoff;
		ret = LPtoDP(hDC, steparr, 1);
		if (ret == 0) {
			if (bDebug) {
				sprintf(str, "LPtoDP failed xoff %d yoff %d", xoff, yoff);
				winerror(str); 
			}
		}
		else {
			dx = steparr[0].x; dy = steparr[0].y;
/*			sprintf(str, "dx %d dy %d", dx, dy);
			wininfo(str); */
/*			limit on how big dx and dy are before just repaint */
			if (dy < - height || dy > height || dx < - width || dx > width) {
/*				xoffset += xoff; yoffset += yoff; */
				InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
			}
			else {
				ScrollWindow(hWnd, dx, dy, NULL, NULL);
				(void) GetUpdateRect(hWnd, &UpdateRect, TRUE);
/*				(void) GetUpdateRgn(hwnd, UpdateRgn, TRUE);  */
/*				adjustoffset(hWnd, dx, dy); */
/*				xoffset += xoff; yoffset += yoff; */
			}
		}
		(void) ReleaseDC(hWnd, hDC);
	}
	if (dy != 0) VScrollPos(hWnd);		/* 1993/Feb/22 */
	if (dx != 0) HScrollPos(hWnd);		/* 1993/Feb/22 */
}

/*  flag > 0 => SELECTPAGE and SEARCH to be enabled */
/*  flag < 0 => SELECTPAGE and SEARCH to be disabled */

int oldprevious = -1, oldnext = -1;	/* remember in order to reduce redraw */

void checkcontrols (HWND hWnd, int flag) {
	HMENU hMenu;
	hMenu = GetMenu(hWnd);
	if (bFileValid == 0)  {
		(void) EnableMenuItem(hMenu, IDM_FIRSTPAGE, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_PREVIOUS, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_NEXT, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_LASTPAGE, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_BACK, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_FORWARD, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_SHOWINFO, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_FONTSUSED, MF_GRAYED);
		oldprevious = 0; oldnext = 0;
		}
	else {
		if (bGrayFlag == 0 || dvipage > 1 || bResetPage != 0) {
			(void) EnableMenuItem(hMenu, IDM_FIRSTPAGE, MF_ENABLED);
			(void) EnableMenuItem(hMenu, IDM_PREVIOUS, MF_ENABLED);
			oldprevious = 1;				/* 1993/Feb/14 */
		}
/*		if (dvipage > 1) oldprevious = 1; */
		if (bGrayFlag == 0 || dvipage < dvi_t || bResetPage != 0) {
			(void) EnableMenuItem(hMenu, IDM_NEXT, MF_ENABLED);
			(void) EnableMenuItem(hMenu, IDM_LASTPAGE, MF_ENABLED);
			(void) EnableMenuItem(hMenu, IDM_BACK, MF_ENABLED);
			(void) EnableMenuItem(hMenu, IDM_FORWARD, MF_ENABLED);
			oldnext = 1;					/* 1993/Feb/14 */
		}
/*		if (dvipage < dvi_t) oldnext = 1; */
		(void) EnableMenuItem(hMenu, IDM_SHOWINFO, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_FONTSUSED, MF_ENABLED);
	}

/*	if (bFileValid != 0) */
	if (bFileValid && ! bFontSample) {
		(void) EnableMenuItem(hMenu, IDM_PRINTALL, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_CURRENTPAGE, MF_ENABLED);
	}
	else {
		(void) EnableMenuItem(hMenu, IDM_PRINTALL, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_CURRENTPAGE, MF_GRAYED);
	}

	if (bFileValid || bFontSample) {
/*		checkmagnify(hWnd); */
		(void) EnableMenuItem(hMenu, IDM_UNMAGNIFY, 
			(wantedzoom > MINZOOM) ? MF_ENABLED : MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_MAGNIFY, 
			(wantedzoom < MAXZOOM) ? MF_ENABLED : MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_PRINTVIEW, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_ZOOM, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_OLDVIEW, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_CLIPBOARD, MF_ENABLED);
	}
	else {
		(void) EnableMenuItem(hMenu, IDM_MAGNIFY, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_UNMAGNIFY, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_PRINTVIEW, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_ZOOM, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_OLDVIEW, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_CLIPBOARD, MF_GRAYED);
	}
	if (flag < 0) {
		(void) EnableMenuItem(hMenu, IDM_SELECTPAGE, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_TOP, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_BOTTOM, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_SEARCH, MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_RESEARCH, MF_GRAYED);
	}
	else if (flag > 0) {
		(void) EnableMenuItem(hMenu, IDM_SELECTPAGE, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_TOP, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_BOTTOM, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_SEARCH, MF_ENABLED);
		(void) EnableMenuItem(hMenu, IDM_RESEARCH, MF_ENABLED);
	}
	DrawMenuBar(hWnd);
}

/* void addpagestring(char *s, BOOL bSpreadFlag) */
void addpagestring (char *s) {
	if (bCountZero == 0) {		/* using dvi pages */
		if (dvipage > 0) {
			if (bSpreadFlag != 0)
//				sprintf(s, "dvi pages:  %d and %d", dvipage-1, dvipage);
				sprintf(s, "pages %d and %d", dvipage-1, dvipage);
//			else if (dvipage >= 0) sprintf(s, "dvi page:  %d", dvipage);
			else if (dvipage >= 0) sprintf(s, "page %d", dvipage);
		}
	}
	else if (bSpreadFlag != 0) {  	/* using counter[0] */
		strcat(s, "count0  ");
		s = s + strlen(s);
		if (leftcountzero != -INFINITY)
			sprintf(s, "%ld  ", leftcountzero);
		else strcat(s, "* ");
/*			sprintf(s, "*  "); */
		s = s + strlen(s);
		if (rightcountzero != -INFINITY)
			sprintf(s, "%ld  ", rightcountzero);
		else strcat(s, "* ");
/*			sprintf(s, "*  "); */
		s = s + strlen(s);
	}
	else if (counter[0] !=  -INFINITY)
		sprintf(s, "  count0:  %ld", counter[0]); /* was %d */
}

void MakeTitleString (char *s) {	
	sprintf(s, "%s", TitleText); /* default if nothing else */
	if (bFontSample || bCharacterSample) ShowFontInfo(s, nChar);
	else if (bFileValid) {
/*		s = s + strlen(s); */
/*		sprintf(s, "  [ %s ]   ", OpenName); */
/*		show full file name in Title bar 99/Apr/21 */
		strcat(s, " [ ");
		if (bShowFullName) {
			if (strchr(OpenName, '\\') != NULL ||
				strchr(OpenName, '/') != NULL ||
				strchr(OpenName, ':') != NULL) ;
			else strcat(s, DefPath);
		}
		strcat(s, OpenName);
		strcat(s, " ] ");
		s = s + strlen(s);
		addpagestring(s);
	}
}

void MakeShortTitle (char *s) {	
	if (bFontSample) strcpy(s, TestFont);
	else if (bFileValid) strcpy(s, OpenName);
	else strcpy(s, TitleText);
}

void SetupWindowText (HWND hWnd) {
	char buffer[MAXFILENAME+24];
/*	if (IsIconic(hwnd) != 0) MakeShortTitle(str); */
	if (IsIconic(hwnd) != 0) MakeShortTitle(buffer);
/*	else MakeTitleString(str); */
	else MakeTitleString(buffer);
/*	SetWindowText(hWnd, (LPSTR) str); */
	SetWindowText(hWnd, (LPSTR) buffer);
#ifdef IGNORED
	norg = strlen(str); 
	nwin = GetWindowTextLength(hWnd);
	if (nwin != norg) {
		sprintf(str, "%d != %d", nwin, norg);
		winerror(str);
	}  			/* DEBUGGING only 95/Mar/12 REMOVE FLUSH */
#endif
}

/*****************************************************************************/

/* Following used to be in winpslog.c - but is only used from dviwindo.c */

void clearmetrics (void) {
	int k;
	for (k = 0; k < MAXFONTS; k++) metricsvalid[k] = 0;
} 

/*  To avoid deleting currently selected font: */
/*  make sure you select something safe like hFontOld before calling this */

void clearfonts (void) {
	int k;
	for (k = 0; k < MAXFONTS; k++) {
		fontvalid[k] = 0;
		fontexists[k] = 0;
		if (windowfont[k] != NULL) {
			(void) DeleteObject(windowfont[k]);
			windowfont[k] = NULL;
		}
	}
} 

void resetfonts(void) {		/* get rid of ALL old font information */
/*	(void) SelectObject(hDC, hFontOld); */
	clearfonts();			/* was in winpslog.c */
	clearmetrics();
	if (hWidths != NULL) {
		FreeWidths();
		hWidths = NULL;
	}
/*		hWidths = GlobalFree(hWidths); */
}

/*****************************************************************************/

int bDialogsHidden = 0;

void hidedialogs(HWND hWnd) {
	if (bDialogsHidden) return;					/* nothing to do */
	if (bDebug > 1) OutputDebugString("Hide Dialogs");
	if (bShowInfoExposed != 0 ) {
 		(void) ShowWindow(hInfoBox, SW_HIDE);	/* hide dialog box */
		bShowInfoExposed = 0;
	}
	if (bShowUsedExposed != 0 ) {
		(void) ShowWindow(hFntBox, SW_HIDE);	/* hide dialog box */
		bShowUsedExposed = 0;
	}
	if (bShowSearchExposed != 0 ) {
		(void) ShowWindow(hFindBox, SW_HIDE);	/* hide dialog box */
		bShowSearchExposed = 0;
	}
/* 1999/Jan/12 experiment to see if the next two are really needed */
	if (bShowInfoFlag != 0)	
		(void) SendMessage(hInfoBox, WM_INITDIALOG, 0, 0L); 	/* needed ? */
	if (bShowUsedFlag != 0)	
		(void) SendMessage(hFntBox, WM_INITDIALOG, 0, 0L);		/* needed ? */
	if (bShowSearchFlag != 0)	
		(void) SendMessage(hFindBox, WM_INITDIALOG, 0, 0L);	
/*	possible hide hConsoleWnd also ? */
	bDialogsHidden = 1;
/*	SetFocus(hWnd); */
}

void restoredialogs(HWND hWnd) {
	if (! bDialogsHidden) return;			/* nothing to do */
	if (bDebug > 1) OutputDebugString("Restore Dialogs");
	if (bShowInfoFlag != 0) {
		(void) SendMessage(hInfoBox, WM_INITDIALOG, 0, 0L);
		if (bShowInfoExposed == 0) {
			(void) ShowWindow(hInfoBox, SW_RESTORE);
			bShowInfoExposed = -1;
		}
	}
	if (bShowUsedFlag != 0) {
		(void) SendMessage(hFntBox, WM_INITDIALOG, 0, 0L);
		if (bShowUsedExposed == 0) {
			(void) ShowWindow(hFntBox, SW_RESTORE);
			bShowUsedExposed = -1;
		}
	}
/*	should we do something here with hFindBox also ??? */
	if (bShowSearchFlag != 0) {						/* 95/Mar/1 */
/*		(void) SendMessage(hFindBox, WM_INITDIALOG, 0, 0L); */
		if (bShowSearchExposed == 0) {
			(void) ShowWindow(hFindBox, SW_RESTORE);
			bShowSearchExposed = -1;
		}
	}
/*	possible restore hConsoleWnd also ? */
	bDialogsHidden = 0;
	SetFocus(hWnd);
}

void reverttexhelp (void) {					/* 1993/Dec/28 */
/* maybe check first whether OpenName is `txhlp0.dvi' ? */
	if (strstr(OpenName, "txhlp0") != NULL) {
		strcpy(OpenName, SourceOpenName);
		strcpy(DefPath, SourceDefPath);
/*		strcpy(DefExt, SourceDefExt); */
/*		strcpy(DefSpec, SourceDefSpec); */

/*		if (bDebug > 1) {
			OutputDebugString("TeXHelp file name");
			OutputDebugString(" restored\n");
		} */
	}
	else {

/*		if (bDebug > 1) {		
			OutputDebugString("TeXHelp file name");
			OutputDebugString(" NOT restored\n");
		} */ /* flush later */
	}
	bTeXHideFlag = 0;
}

/* close file, remove info --- reset mapping also if flag non-zero */

void closeup (HWND hWnd, int resetflag) { /* remove info from last file open */
	HDC hDC;
	int closedflag=bFileValid;			/* remember for after its reset */

	if (bTimerOn > 0) KillTimer(hWnd, 1); 	/* kill the timer now */
	bTimerOn = 0;
	nAlarmTicks = 0;						/* 1994/Aug/10 */

/*	hTaskWindow=NULL; */				/* reset, just in case 1993/Jan/5 */
	TaskInfo.hWnd = NULL; 				/* reset, just in case 1995/Mar/12 */

	if (hFile != HFILE_ERROR) {			/* NEW */
/*		if (hFile < 0) winerror("File already closed");	else */
		_lclose(hFile);
		hFile = HFILE_ERROR;		/* finally close it */
	}
	bFileValid = 0;
/*	bFontsUsed = 0; */
	current = -1;
	dvipage = -INFINITY;
	lastsearch = -1;
	
/*	LETS BE SAFE: reset these in case they get stuck on ... 1993/March/29 */
	bBusyFlag = 0;
	bPrintFlag = 0;
	bCopyFlag = 0;
	bSearchFlag = 0;
	bUserAbort = 0;

	bNeedANSIacce=0;				/* 1993/Dec/21 */
	bNeedANSIwarn=0;				/* 1993/Dec/21 */
	bBadFileComplained=0;			/* 1995/Mar/28 */
	bBadSpecialComplained=0;		/* 1996/Feb/4 */
	bUnitsWarned=0;					/* 1995/Aug/14 */
	bZeroWarned=0;					/* 1997/Mar/7 */
#ifdef USEUNICODE
	bFontEncoded=0;					/* 1997/Jan/26 */
#endif
	bAddToMRU=1;					/* reset to default ? */

	hDC = GetDC(hWnd);
	if ((UINT) hFontOld > 1)	/* Avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hDC, hFontOld);
	resetfonts();
	(void) ReleaseDC(hWnd, hDC);

	SetupWindowText(hWnd);
	CenterThumbs(hWnd);
	HideScrollBars(hWnd);
	checkcontrols(hWnd, -1);

	resetinfodetails();				/* remove DVIMetric dialog ? */
	resetusedetails();				/* remove FontUse dialog ? */
	if (resetflag != 0) {				/* reset scale factors & offset */
		if (bResetScale != 0) {
			resetpagemapping(hWnd);
			resetgraphicstate();  /* ??? */
		}
		setscale(wantedzoom);
	}
/*	resetgraphicstate(); */ /* ??? */
	hidedialogs(hWnd);
/*  may loose selection focus - so set it to main window ??? */
	SetFocus(hWnd);					/* 92/Feb/18 */
/*	if (hWidths != NULL) hWidths = GlobalFree(hWidths); */
/*  already done in resetfonts ??? */
	if (hPages != NULL) hPages = GlobalFree(hPages);
	if (hBack != NULL) hBack = GlobalFree(hBack);
	if (hColor != NULL) {
/*		hColor = GlobalFree(hColor); */
		FreeColor();			/*		also free saved stacks */
	}
	FreeATMTables();

	if (bATMReencoded) {										/* 94/Dec/25 */
		if (bDebug > 1) OutputDebugString("Stuck Encoding\n");	/* 95/Jan/19 */
/*		UnencodeFont(hDC, 0); 	 */
/*		bATMReencoded = 0;	  	 */
	}		/* reset ATM encoding state */	/* need hDC ? Oops */
#ifdef IGNORED
	free_hyperfile();		/* right place ??? winsearc.c */	
#endif
	hyperindex = 0; 		/* reset hypertext index */ 
	colorindex = 0;			/* reset color stack */
/*	if (bDebug > 1) OutputDebugString("closeup\n"); *//* debug */
	if (closedflag != 0 && bTeXHideFlag != 0) reverttexhelp();
	setpagesize(papertype);	/* in case \special reset this 98/Jun/30 */
	oneinch = ONEINCH;		/* restore default 99/Apr/18 */
	dvi_num = 25400000;		/* default numerator of scale */
	dvi_den = 473628672;	/* default denominator of scale */
	dvi_mag = 1000;			/* default magnification */
}

/***************************************************************************/

/*	compensatory size changes with magnification 1992/Apr/4 ??? */

void DrawBorderSub (HDC hDC) {
	POINT Box[5];			/* 1993/Jan/22 */
	double pw, ph;			/* big points 99/Apr/18 */
	long dpw, dph;			/* DVI file units 99/Apr/18 */
	int xll, yll, xur, yur;

	xll = mapx(0L);	yll = mapy(0L);
	if (bLandScape == 0) {
		pw = (long) PageWidth;			/* in PS points */
		ph = (long) PageHeight;			/* in PS points */
	}
	else {
		ph = (double) PageWidth;
		pw = (double) PageHeight;
	}
/*	should it be 72.27 or 72.0 ??? */
	dpw = (long) (pw * (254000.0 / 72.0) * ((double) dvi_den / dvi_num));
	dph = (long) (ph * (254000.0 / 72.0) * ((double) dvi_den / dvi_num));
	if (dvi_mag != 1000) {
		dpw = MulDiv(dpw, 1000, dvi_mag);
		dph = MulDiv(dph, 1000, dvi_mag);
	}
/*	Is this assuming standard dvi_num and dvi_den ??? */
/*	xur = mapx((pw * 1000 / dvi_mag) * 65536); */
/*	yur = mapy((ph * 1000 / dvi_mag) * 65536); */
	xur = mapx(dpw);
	yur = mapy(dph);
	Box[0].x = xll; 	Box[0].y = yll;
	Box[1].x = xur; 	Box[1].y = yll;	
	Box[2].x = xur; 	Box[2].y = yur;	
	Box[3].x = xll; 	Box[3].y = yur;	
	Box[4].x = xll; 	Box[4].y = yll;
	Polyline(hDC, Box, 5);				/* 1993/Jan/22 */
}

/*	compensatory size changes with magnification 1992/Apr/4 ??? */

void DrawLinerSub (HDC hDC) {
	POINT Box[5];			/* 1993/Jan/22 */
	int xll, yll, xur, yur;

	if (dvi_mag == 1000) {
		xll = mapx(oneinch);  
		xur = mapx(oneinch + (long) dvi_u);  
		yll = mapy(oneinch);  
		yur = mapy(oneinch + (long) dvi_l);  
	}
	else {										/* 1994/July/18 */
		xll = mapx(MulDiv(oneinch, 1000, dvi_mag));	/* 1 inch across */
		xur = mapx(MulDiv(oneinch, 1000, dvi_mag) + (long) dvi_u); 
		yll = mapy(MulDiv(oneinch, 1000, dvi_mag));	/* 1 inch down */
		yur = mapy(MulDiv(oneinch, 1000, dvi_mag) + (long) dvi_l); 
/*		xll = mapx(oneinch / dvi_mag * 1000);
		xur = mapx(oneinch / dvi_mag * 1000 + (long) dvi_u); 
		yll = mapy(oneinch / dvi_mag * 1000);
		yur = mapy(oneinch / dvi_mag * 1000 + (long) dvi_l); */
	}

	Box[0].x = xll; 	Box[0].y = yll;
	Box[1].x = xur; 	Box[1].y = yll;	
	Box[2].x = xur; 	Box[2].y = yur;	
	Box[3].x = xll; 	Box[3].y = yur;	
	Box[4].x = xll; 	Box[4].y = yll;
	Polyline(hDC, Box, 5);				/* 1993/Jan/22 */
}

#ifdef IGNORED
/* Bounding box given in PostScript (big) points */
/* 65536 * 72.27 / 72 = 65782 */

void DrawBBoxSub (HDC hDC) {
	POINT Box[5];
	int xll, yll, xur, yur;

	if (dvi_mag == 1000) {
		xll = mapx((long) BBxll * 65782);
		xur = mapx((long) BBxur * 65782);
		yll = mapy((long) BByll * 65782);
		yur = mapy((long) BByur * 65782);
	}
	else {
		xll = mapx((long) BBxll * 65782 / dvi_mag * 1000);
		xur = mapx((long) BBxur * 65782 / dvi_mag * 1000);
		yll = mapy((long) BByll * 65782 / dvi_mag * 1000);
		yur = mapy((long) BByur * 65782 / dvi_mag * 1000);
	}

	Box[0].x = xll; 	Box[0].y = yll;
	Box[1].x = xur; 	Box[1].y = yll;	
	Box[2].x = xur; 	Box[2].y = yur;	
	Box[3].x = xll; 	Box[3].y = yur;	
	Box[4].x = xll; 	Box[4].y = yll;
	Polyline(hDC, Box, 5);				/* 1993/Jan/22 */
}
#endif

void DrawBorder (HDC hDC, HPEN hPen) {	
	HPEN hOldPen;
	long xo;

	if (bFileValid == 0) return;	
	hOldPen = SelectObject(hDC, hPen);
	if (bSpreadFlag != 0) {
		xo = xoffset;
/*		xoffset = adjustoffsetleft(xo); */
		xoffset = leftpageoffset(xo);
		DrawBorderSub(hDC);
/*		xoffset = adjustoffsetright(xo); */
		xoffset = rightpageoffset(xo);
		DrawBorderSub(hDC);
		xoffset = xo;
	}
	else DrawBorderSub(hDC);
	if ((UINT) hOldPen > 1)	/* avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hDC, hOldPen);		
}

void DrawLiner (HDC hDC, HPEN hPen) {
	HPEN hOldPen;
	long xo;

	if (bFileValid == 0) return;	
	hOldPen = SelectObject(hDC, hPen);
	if (bSpreadFlag != 0) {
		xo = xoffset;
/*		xoffset = adjustoffsetleft(xo);	 */
		xoffset = leftpageoffset(xo);
		DrawLinerSub(hDC);
/*		xoffset = adjustoffsetright(xo); */
		xoffset = rightpageoffset(xo);
		DrawLinerSub(hDC);
		xoffset = xo;
	}
	else DrawLinerSub(hDC);
	if ((UINT) hOldPen > 1)	/* avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hDC, hOldPen);		
}	

/***************************************************************************/

/* Paper types are all numbered sequentially --- */
/* but 2 (letter small) and 10 (A4 small) are omitted */

void checkpaper (HWND hWnd, int type) {
	HMENU hMenu;
	int k;
	hMenu = GetMenu(hWnd);

	if (bDebug > 1) {
		sprintf(debugstr, "checkpaper %d", type);
		OutputDebugString(debugstr);
//		wincancel(debugstr);	// debugging only
	}
//	for (k = 1; k < maxpapersize; k++) 
	for (k = 0; k < maxpapersize; k++) {	// 0 == custom size
		if (k == 2 || k == 10) continue;
		(void) CheckMenuItem (hMenu, IDM_LETTER + (k-1),
		(type == DMPAPER_LETTER + (k-1)) ? MF_CHECKED : MF_UNCHECKED);
	}
}

#define MAXTEXUNITS 9

/*	This assumes there are 9 TeX measurement units --- numbered sequentially */

void checkunits (HWND hWnd, int units) {
	HMENU hMenu;
	int k;
	hMenu = GetMenu(hWnd);
/*	for (k = 0; k < 9; k++) */
	for (k = 0; k < MAXTEXUNITS; k++) {				/* 1993/Dec/16 */
		(void) CheckMenuItem (hMenu, IDM_SCALEDPOINT + k,
			(units == k) ? MF_CHECKED : MF_UNCHECKED);
	}
}

#define MAXDUPLEX 3

/* #define DMDUP_SIMPLEX    1 */
/* #define DMDUP_VERTICAL   2 */
/* #define DMDUP_HORIZONTAL 3 */

void checkduplex(HWND hWnd, int duplex) {
	HMENU hMenu;
	int k;
	hMenu = GetMenu(hWnd);
	for (k = 0; k < MAXDUPLEX; k++) {				/* 1996/Nov/17 */
		(void) CheckMenuItem (hMenu, IDM_SIMPLEX + k,
			(duplex == DMDUP_SIMPLEX + k) ? MF_CHECKED : MF_UNCHECKED);
	}
}

void checkpreferences(HWND hWnd) {  /* put up checkmarks accordingly */
	HMENU hMenu;
	hMenu = GetMenu(hWnd);

//	(void) EnableMenuItem(hMenu, IDM_USEWORKDIR,
//						  (*IniDefPath == '\0') ? MF_GRAYED : MF_ENABLED); 
//						  (IniDefPath == NULL) ? MF_GRAYED : MF_ENABLED); 

	(void) CheckMenuItem (hMenu, IDM_USEWORKDIR,
						  bUseWorkPath ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SAVEPREFER,
						  bSavePrefer ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SPREAD,
						  bSpreadFlag ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_COUNTZERO,
						  bCountZero ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_LANDSCAPE,
						  bLandScape ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_GREYTEXT,
						  bGreyText ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_GREYPLUS,
						  bGreyPlus ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_COLORFONT,
						  bColorFont ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_RULEFILL,
						  bRuleFillFlag ? MF_CHECKED : MF_UNCHECKED); 
	(void) CheckMenuItem (hMenu, IDM_SHOWCALLS,
						  bShowCalls ? MF_CHECKED : MF_UNCHECKED); 
	(void) CheckMenuItem (hMenu, IDM_PAUSECALLS,
						  bPauseCalls ? MF_CHECKED : MF_UNCHECKED); 
/*	(void) CheckMenuItem (hMenu, IDM_OUTLINERULE,
		bRuleFillFlag ? MF_UNCHECKED : MF_CHECKED); */
	(void) CheckMenuItem (hMenu, IDM_BORDER,
						  bShowBorder ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SHOWHXW,
						  bShowLiner ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_RESETSCALE,
						  bResetScale ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_RESETPAGE,
						  bResetPage ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_TRUEINCH,
						  bTrueInch ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_COMPLAINMISSING,
						  bComplainMissing ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_COMPLAINENCODING,
						  bCheckEncoding ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_COMPLAINSPECIAL,
						  bComplainSpecial ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_COMPLAINFILES,
						  bComplainFiles ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_IGNORESPECIAL,
						  bIgnoreSpecial ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SHOWPREVIEW,
						  bShowPreview ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_STRINGLIMIT,
						  bUseCharSpacing ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SHOWBUTTONS,
						  bShowButtons ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SHOWVIEWPORTS,
						  bShowViewPorts ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SHOWTIFF,
						  bShowTIFF ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SHOWWMF,
						  bShowWMF ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_VIEWEXTRAS,
						  bViewExtras ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_PASSTHROUGH,
						  bPassThrough ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_CALLBACKPASS,
						  bCallBackPass ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_CONSOLEOPEN,
						  bConsoleOpen ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_USEDLLS,
						  bUseDLLs ? MF_CHECKED : MF_UNCHECKED);
//	(void) CheckMenuItem (hMenu, IDM_OPENCLOSECHANNEL,
//						  bOpenCloseChannel ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_OPENCLOSEPRINTER,
						  bOpenClosePrinter ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_NEWPASSTHROUGH,
						  bNewPassThrough ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_FORCEPASSBACK,
						  bForcePassBack ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_FORCEOLDPASSTHROUGH,
						  bOldPassThrough ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_DONTASKFILE,
						  bDontAskFile ? MF_CHECKED : MF_UNCHECKED);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Global Allocation Stuff */

int MaxPage;				/* remember how much space allocated */
int MaxBack;
int MaxColor;
int MaxPath;
int MaxFonts;				/* remember how much we allocated */

void GrabPages (void) {
	if (hPages == NULL) {
		sprintf(debugstr, "No %s to grab", "Pages");
		winerror(debugstr);
		PostQuitMessage(0);		/* pretty serious ! */
	}
	else {
		if (lpPages != NULL) {			/* 99/Jan/10 */
			if (bDebug > 1) OutputDebugString("lpPages != NULL");
			return;
		}
		lpPages = (LPLONG) GlobalLock(hPages);
		if (lpPages == NULL) {
			ShowLastError();
			sprintf(debugstr, "Unable to lock %s", "Pages");
			winerror(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
}

void GrabBack (void) {
	if (hBack == NULL) {
		sprintf(debugstr, "No %s to grab", "Back");
		winerror(debugstr);
		PostQuitMessage(0);		/* pretty serious ! */
	}
	else {
		if (lpBack != NULL) {			/* 99/Jan/10 */
			if (bDebug > 1) OutputDebugString("lpBack != NULL");
			return;
		}
		lpBack = (LPCOLORREF) GlobalLock(hBack);
		if (lpBack == NULL) {
			ShowLastError();
			sprintf(debugstr, "Unable to lock %s", "Back");
			winerror(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
}

void GrabColor (void) {
	if (hColor == NULL) {
		sprintf(debugstr, "No %s to grab", "Color");
		winerror(debugstr);
		PostQuitMessage(0);		/* pretty serious ! */
	}
	else {
		if (lpColor != NULL) {			/* 99/Jan/10 */
			if (bDebug > 1) OutputDebugString("lpColor != NULL");
			return;
		}
		lpColor = (COLORREF **) GlobalLock(hColor);
		if (lpColor == NULL) {
			ShowLastError();
			sprintf(debugstr, "Unable to lock %s", "Color");
			winerror(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
}

void GrabTPIC (void) {
	if (hTPIC == NULL) {
		sprintf(debugstr, "No %s to grab", "TPIC");
		winerror(debugstr);
		PostQuitMessage(0);
	}
	else {
		if (lpTPIC != NULL) {			/* 99/Jan/10 */
			if (bDebug > 1) OutputDebugString("lpTPIC != NULL");
			return;
		}
/*		lpTPIC = (LPLONG) GlobalLock(hTPIC); */
		lpTPIC = (POINT FAR *) GlobalLock(hTPIC);
		if (lpTPIC == NULL) {
			ShowLastError();
			sprintf(debugstr, "Unable to lock %s", "TPIC");
			winerror(debugstr);
			PostQuitMessage(0);
		}
	}
}

/* In WIN32 make following LPSHORT instead ??? */

void GrabWidths (void) {
	if (hWidths == NULL) {
		sprintf(debugstr, "No %s to grab", "Widths");
		winerror(debugstr);
		PostQuitMessage(0);		/* pretty serious ! */
	}
	else {
		if (lpWidths != NULL) {			/* 99/Jan/10 */
			if (bDebug > 1) OutputDebugString("lpWidths != NULL");
			return;
		}
/*		lpWidths = (LPINT) GlobalLock(hWidths); */
		lpWidths = (LPSHORT) GlobalLock(hWidths);
		if (lpWidths == NULL) {
			ShowLastError();
			sprintf(debugstr, "Unable to lock %s", "Widths");
			winerror(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
}

void ReleasePages (void) {
	if (hPages == NULL) {
		sprintf(debugstr, "No %s to release", "Pages");
		winerror(debugstr);
/*		PostQuitMessage(0); */		/* pretty serious ? */
	}
	else {
		if (lpPages == NULL) {
			if (bDebug > 1) OutputDebugString("lpPages == NULL");
			return;
		}
		if (GlobalUnlock(hPages) > 0) {
			sprintf(debugstr, "Lock count Not Zero");
			winerror(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
	lpPages = NULL;		/* 98/Dec/15 */
}

void ReleaseBack (void) {
	if (hBack == NULL) {
		sprintf(debugstr, "No %s to release", "Back");
		winerror(debugstr);
/*		PostQuitMessage(0); */		/* pretty serious ? */
	}
	else {
		if (lpBack == NULL) {
			if (bDebug > 1) OutputDebugString("lpBack == NULL");
			return;
		}
		if (GlobalUnlock(hBack) > 0) {
			sprintf(debugstr, "Lock count Not Zero");
			winerror(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
	lpBack = NULL;		/* 98/Dec/15 */
}

void ReleaseColor (void) {
	if (hColor == NULL) {
		sprintf(debugstr, "No %s to release", "Color");
		winerror(debugstr);
/*		PostQuitMessage(0); */		/* pretty serious ? */
	}
	else {
		if (lpColor == NULL) {
			if (bDebug > 1) OutputDebugString("lpColor == NULL");
			return;
		}
		if (GlobalUnlock(hColor) > 0) {
			sprintf(debugstr, "Lock count Not Zero");
			winerror(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
	lpColor = NULL;		/* 98/Dec/15 */
}

void ReleaseTPIC (void) {
	if (hTPIC == NULL) {
		sprintf(debugstr, "No %s to release", "TPIC");
		winerror(debugstr);
/*		PostQuitMessage(0); */		/* pretty serious ? */
	}
	else {
		if (lpTPIC == NULL) {
			if (bDebug > 1) OutputDebugString("lpTPIC == NULL");
			return;
		}
		if (GlobalUnlock(hTPIC) > 0) {
			sprintf(debugstr, "Lock count Not Zero");
			winerror(debugstr);
			PostQuitMessage(0);
		}
	}
	lpTPIC = NULL;		/* 98/Dec/15 */
} 

void ReleaseWidths (void) {
	if (hWidths == NULL) {
		sprintf(debugstr, "No %s to release", "Widths");
		winerror(debugstr);
/*		PostQuitMessage(0); */		/* pretty serious ? */
	}
	else {
		if (lpWidths == NULL) {			/* 99/Jan/10 */
			if (bDebug > 1) OutputDebugString("lpWidths == NULL");
			return;
		}
		if (GlobalUnlock(hWidths) > 0) {
			sprintf(debugstr, "Lock count Not Zero");
			winerror(debugstr);
			PostQuitMessage(0);		/* pretty serious ! */
		}
	}
	lpWidths = NULL;		/* 98/Dec/15 */
}

void FreePages (void) {			/* not used directly */
	if (hPages == NULL) {
		sprintf(debugstr, "No %s to free", "Pages");
		winerror(debugstr);
	}
	else hPages = GlobalFree(hPages);
	lpPages = NULL;
	hPages = NULL;
}

void FreeBack (void) {			/* not used directly */
	if (hBack == NULL) {
		sprintf(debugstr, "No %s to free", "Back");
		winerror(debugstr);
	}
	else hBack = GlobalFree(hBack);
	lpBack = NULL;
	hBack = NULL;
}

void FreeColor (void) {			/* MaxColor is npages+1 */
	int k;
	if (hColor == NULL) {
		sprintf(debugstr, "No %s to free", "Color");
		winerror(debugstr);
	}
	else {						/* also free saved stacks */
		GrabColor();
		for (k = 0; k < MaxColor; k++) {
			if (lpColor[k] != NULL) {
				free(lpColor[k]);
				lpColor[k]= NULL;
			}
		}
		ReleaseColor();
		hColor = GlobalFree(hColor);
	}
	lpColor = NULL;
	hColor = NULL;
}

void FreeWidths (void) {
	if (hWidths == NULL) {
		sprintf(debugstr, "No %s to free", "Widths");
		winerror(debugstr);
	}
	else hWidths = GlobalFree(hWidths);
	lpWidths = NULL;
	hWidths = NULL;			   
}

void AllocPages (int npages) {	/* grab space for Page Table - if needed */
//	sprintf(debugstr, "%d pages, %ld bytes\n", npages, 
//			(unsigned long) npages * sizeof(long) * 2);
//	winerror(debugstr);
	if (hPages != NULL) {					/* modified 1993 Aug 5 */
		if (npages <= MaxPage) return;		/* already have a table */
		sprintf(debugstr, "%s Allocation Error", "Page");	
		winerror(debugstr); /* but it is too small */
		hPages = GlobalFree(hPages);
	}
	if (npages == 0) npages = 1;		// ???
/*	Entries in page table are long, and there are two sets of entries: */
/*	one for PageStart in the file, the other for count[0] of the page */
	hPages = GlobalAlloc(GMEM_MOVEABLE, 
		(unsigned long) npages * sizeof(long) * 2);  
	if (hPages == NULL) {
		sprintf (debugstr, "Unable to allocate memory");
		winerror(debugstr);
		PostQuitMessage(0);			/* pretty serious ! */
	}
	MaxPage = npages;				/* remember for debug */
}

void ResetColor (int MaxColor) {	/* MaxColor = npages+1 */ 
	int k;
	GrabColor();
	for (k = 0; k < MaxColor; k++) lpColor[k] = NULL;
	ReleaseColor();
}

/* pageno zero based or one based ??? */

void AllocColor (int npages) {	/* grab space for Color Stack Table - if needed */
	if (hColor != NULL) {
		sprintf(debugstr, "%s Allocation Error", "Color");
		winerror(debugstr);
		FreeColor();		/* just in case */
	}
	if (npages == 0) npages = 1;		// ???
/*	Entries in color table are pointers to saved stacks */
	hColor = GlobalAlloc(GMEM_MOVEABLE, 
			(unsigned long) (npages+1) * sizeof(COLORREF *));
	if (hColor == NULL) {
		sprintf (debugstr, "Unable to allocate memory");
		winerror(debugstr);
		PostQuitMessage(0);			/* pretty serious ! */
	}
	MaxColor = npages+1;			/* remember for debug */
	ResetColor(MaxColor);			/* set entries to NULL */
}

void ResetBack (COLORREF Color, int MaxBack) {
	int k;
	GrabBack();
	for (k = 0; k < MaxBack; k++) lpBack[k] = Color;	/* 98/Sep/6 */
	ReleaseBack();
}

void AllocBack (int npages) {	/* grab space for Page Table - if needed */
	if (hBack != NULL) {					/* modified 1993 Aug 5 */
		if (npages <= MaxBack) return;		/* already have a table */
		sprintf(debugstr, "%s Allocation Error", "Page");
		winerror(debugstr);		/* but it is too small */ 
		hBack = GlobalFree(hBack);
	}
	if (npages == 0) npages = 1;		// ???
	hBack = GlobalAlloc(GMEM_MOVEABLE, 
				(unsigned long) (npages+1) * sizeof(COLORREF));
	if (hBack == NULL) {
		sprintf (debugstr, "Unable to allocate memory");
		winerror(debugstr);
		PostQuitMessage(0);			/* pretty serious ! */
	}
	MaxBack = npages+1;				/* remember for debug */
	ResetBack(OrgBkColor, MaxBack);
}

void AllocTPIC (int npath) {		/* 1992/Dec/12 */
	if (hTPIC != NULL) {
		if (npath <= MaxPath) return;
		sprintf (debugstr, "Allocation Error", "TPIC");
		winerror(debugstr);
		hTPIC = GlobalFree(hTPIC);
	}
	if (npath == 0) npath = 1;		// ???
	hTPIC = GlobalAlloc(GMEM_MOVEABLE, 
/*		(unsigned long) npath * 2 * sizeof(long)); */ /* 2 long's per */
		(unsigned long) npath * sizeof(POINT));
	if (hTPIC == NULL) {
		sprintf (debugstr, "Unable to allocate memory");
		winerror(debugstr);
		PostQuitMessage(0);				/* pretty serious */
	}
	MaxPath = npath;
}

/* For WIN32, the allocation of int for a widths may be excessive */

/* In WIN32 make following LPSHORT instead ??? sizeof(short) instead */

void AllocWidths (int fn) {		/* grab space for Widths - if needed */
	if (hWidths != NULL) {				/* modified 1993 Aug 5 */
		if (fn <= MaxFonts) return;
		sprintf(debugstr, "%s Allocation Error", "Font");
		winerror(debugstr);
		hWidths = GlobalFree(hWidths);
	}
/*	Entries are int and there are MAXCHRS of them */
	if (fn == 0) fn = 1;		// ???
	hWidths = GlobalAlloc(GMEM_MOVEABLE, 
/*		(unsigned long) fn * MAXCHRS * 2); */
/*		(unsigned long) fn * MAXCHRS * sizeof(int)); */
		(unsigned long) fn * MAXCHRS * sizeof(short)); 

	if (hWidths == NULL) {
		sprintf (debugstr, "Unable to allocate memory");
		winerror(debugstr);
		bFontSample = 0;
		bCharacterSample = 0;
		PostQuitMessage(0);			/* pretty serious ! */
	}
	MaxFonts = fn;					/* remember how much space */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void checkfontmenu (void) {
	HMENU hMenu;
	hMenu = GetMenu(hwnd);
/*	(void) CheckMenuItem (hMenu, IDM_SELECTFONT,
		(bFontSample != 0) ? MF_CHECKED : MF_UNCHECKED); */
/*	(void) CheckMenuItem (hMenu, IDM_CHARWIDTHS,
		(bShowWidths != 0) ? MF_CHECKED : MF_UNCHECKED); */
	(void) CheckMenuItem (hMenu, IDM_FONTSUSED,
		bShowUsedFlag ? MF_CHECKED : MF_UNCHECKED);
	(void) CheckMenuItem (hMenu, IDM_SHOWINFO,
		bShowInfoFlag ? MF_CHECKED : MF_UNCHECKED);
}

/* +1 => FontSample,   -1 => CharWidths,   0 => NO Font Display */
/* called from IDM_OPEN with zero argument */ /* and from GetFaces */
/* from GetFontSelection in winfonts.c */

void SetFontState (int flag) {
/*	HMENU hMenu; */

/*	if (bDebug > 1) OutputDebugString("SetFontState\n"); */ /* FLUSH */
/*	bFileValid = 0; */
/*	hMenu = GetMenu(hwnd); */
	if (flag > 0) {
		bFontSample = 1;			/* non-zero if want font sample */
		bShowWidths = 0;			/* non-zero if want character widths */
	}
	else if (flag < 0) {
		bFontSample = 1;			/* non-zero if want font sample */
		bShowWidths = 1;			/* non-zero if want character widths */
	}
	else {				/* flag == 0 - switching to file display instead */
		bFontSample = 0;			/* non-zero if want font sample */
/* following removed 95/Aug/21 - possibly dangerous in WriteAllTFMs ... */
/*		if (bWriteAFM == 0) {
			if (hFaceNames != NULL) FreeFaceNames();
		} */
/*  Reenable stuff valid when file bFileValid != 0) ? */
	}
/*	checkfontmenu(); */
}

/* NOTE: Following definition repeated in winsearc.c */

/* Data structure for `encoding' information in ATM - see WINSEARC.C */

typedef struct tagATMDATA {
	WORD	enSetup;			/* set to one to ask ATM to set up fields */
	WORD	enMinus;			/* should be minus one */
	WORD	enOffset;			/* or 20 hex to start at 32 */
	WORD	enNumber;			/* unique enc number assigned by ATM */
	WORD	enGlyphNumber[256];	/* sequential numbers of glyphs */
	WORD	enWindowsCode[256];	/* corresponding ANSI codes or -1 */
	WORD	enWhoKnows[256];	/* who knows what this is! */
	LPSTR	enEncoding;			/* pointer to array of LPSTR *//* fix ??? */
/*	WORD	enReserved; */		/* small buffer zone ? 1995/Nov/12 */
} ATMDATA;

/* about 1548 bytes */ /* can fit only 42 in 65536U bytes ! *//* 95/Nov/12 */

typedef ATMDATA FAR *LPATMDATA;

/* Do prescan of file - return -1 if trouble */

int DoPreScan (HWND hWnd, int hDVIFile) {
	HDC hDC;
	int flag = 0;
	int k;
/*	LPATMDATA lpATMTable; */
/*	LPATMDATA lpATMData; */
#ifdef ATMSOLIDTABLE
	long nlen;	/* only if we allocate monolithic table */
#endif
	
	if (bRuleFlag == 0) {						/* 1993/Feb/22 */
		bHourFlag = -1;
		hSaveCursor = SetCursor(hHourGlass);
	}

	hDC = GetDC(hWnd);
	if ((UINT) hFontOld > 1)	/* avoid Windows 3.0 MetaFile problem */
		(void) SelectObject(hDC, hFontOld);
	resetfonts();

#ifdef IGNORED
	free_hyperfile(); 
#endif
	hyperindex = 0;			/* reset hypertext index */ /* right place ??? */
	bHyperUsed = 0;			/* if DVI page contains hypertext linkage */
	bSourceUsed = 0;		/* if DVI page contains src linkage */
	colorindex = 0;			/* reset color stack */
#ifdef USEUNICODE
	bFontEncoded=0;			/* 1997/Jan/26 paranoia */
#endif

/*	following should not need to be tested, but ... */
	FreeATMTables();

/*	nFileLength = scanlogfile(hDVIFile); */
	nFileLength = scanlogfile(hDVIFile, hDC);

	frompage = 1; topage = dvi_t;	/* set up print range 1993/Aug/28 */

	if (nFileLength < 0) {  /* trouble reading file ? */
		bFileValid = 0;		/* hence not valid connection */
/* OR:	ShowCursor(TRUE); */
		if (bRuleFlag == 0) {						/* 1993/Feb/22 */
			bHourFlag = 0;
			(void) SetCursor(hSaveCursor);		/* restore cursor */
		}
/*		bHourFlag = 0; */
		sprintf(str, "Error reading %s", OpenName); /* (LPSTR) OpenName); */
		{
			sprintf(debugstr, str);	/* redundant ? */
			winerror(debugstr);
		}
		flag = -1;
	}
	else {				/* WAS able to prescan the file successfully */
/*		if (bUseATMINI != 0) {
			standardizenames();	
			(void) readatmini();
			substitutefonts();	
		} */
/*	find Windows Face Names corresponding to font file names */
/*		(void) mapfontnames(); */	/* in winpslog.c */
		mapfontnames(hDC);			/* subfontname[k] <= fontname[k]; */

		bFileValid = 1;
		bFontSample = 0;			/* 2000 August 12 paranoia */
		bCharacterSample = 0;		/* 2000 August 12 paranoia */

		setscale(wantedzoom);		/* new here */
/*		restoredialogs(hWnd); */	/* too early ? moved down 1999/Jan/8  */
		SetupWindowText(hWnd);	
		ExposeScrollBars(hWnd);
		VScrollPos(hWnd);
		HScrollPos(hWnd);
		checkcontrols(hWnd, +1);

/*	Allocate table for ATM reencoding table */
/*	Somewhat extravagant - maybe allocate only for ANSI text fonts ... */
/*  but we don't know at this stage which those are ... */
/*  Also need only one for a font that appears in different sizes ? */
/*	Maybe later use font sub table to allow variable remapping ? */	
/*  Also, takes time to set up entries at this point - */
/*  - instead set up later only as needed */

/* worry about special case when fnext == 0 ??? */

		if (bATMLoaded && hEncoding != NULL && fnext > 0) {	/* 94/Dec/25 */
#ifdef ATMSOLIDTABLE
/*			allocate as one monolithic table - need huge pointers */
/*			hATMTable = GlobalAlloc (GMEM_MOVEABLE,
					(long) fnext * ATMTABLESIZE); */
/*			This assumes old way of doing things, not for WIN32 ... */
			nlen = (long) fnext * sizeof (ATMDATA);			/* 95/Nov/12 */
/*			if more than 65536U need to use huge pointers */
			hATMTable = GlobalAlloc (GMEM_MOVEABLE, nlen);
/*			hATMTable = GlobalAlloc (GMEM_FIXED, nlen); */	/* experiment 95/Jan/5 */
			if (hATMTable == NULL) {
				winerror ("Unable to allocate memory");
/*				winerror ("Unable to allocate memory for reencoding"); */
				PostQuitMessage(0);					/* 1995/Jan/19 ??? */
			}
			else {
/*				lpATMTable = GlobalLock(hATMTable); */
				for (k=0; k < fnext; k++) {
					encodefont[k] = 0;			/* indicate not yet set */
/*					lpATMData = lpATMTable + k; */
/*					lpATMData->enSetup = 1; */    /* Force ATMSelectEncoding setup */
/*					lpATMData->enMinus = 0xFFFF;*//* Always -1 */
/*					lpATMData->enOffset = 0; */	/* CharMin ? */
/*					lpATMData->enNumber = 0; */	/* Unique number will be assigned */ 
				}
/*				(void) GlobalUnlock(hATMTable); */
			}
#else
/*	following should not be needed ... */		/* done above ? */
			for (k=0; k < fnext; k++) {			/* 95/Nov/12 */
				if (hATMTables[k] != NULL)
					hATMTables[k] = GlobalFree(hATMTables[k]);
			}
#endif
		}

		if (fnext > 0) AllocWidths(fnext);
		else AllocWidths(1); 			/* dummy for file with no fonts ! */

/*		else bFileValid = 0; */			/* shouldn't happen */ /* 92/Feb/13 */
	}
	if (bRuleFlag == 0) {						/* 1993/Feb/22 */
		bHourFlag = 0;
		(void) SetCursor(hSaveCursor);		/* restore the cursor */
	}
	(void) ReleaseDC(hWnd, hDC);		/* new ? */
/*	if (bDialogsHidden) restoredialogs(hWnd); */	/* 1999/Jan/8 moved here ? */
	return flag;
}

/* get file position (current) of page start (wantedpage) */
/* returns zero if wantedpage is out of range, table does not exist etc */
/* also sets up counter[0] from table */

long pagecurrent (int wantedpage) {
	LPLONG PageStart;	/* Pointer to Page Start Table */
	LPLONG PageCount;	/* Pointer to Count Zero Table */

	if (bFileValid == 0) return 0;
	if (hPages == NULL) return 0;
	if (wantedpage == -INFINITY) return 0;

	if (wantedpage <= 0 || wantedpage > dvi_t) return 0;
	if (dvi_t != MaxPage) {			/* debugging code 1993/Aug/28 */
		if (bDebug) {
			sprintf(str, "dvi_t %d != MaxPage %d (pagecurrent)",
				dvi_t, MaxPage);
			winerror(str);
		}
	}
	dvipage = wantedpage;
	GrabPages();
	PageStart = lpPages;
	PageCount = lpPages + dvi_t;			/* MaxPage ? */
/*	if (bCarryColor && bColorUsed) GrabColor();	*/	/* ??? */
/*	current = PageStart[wantedpage-1]; */	/* 1993/Aug/28 */
	current = PageStart[dvipage-1];
	counter[0] = PageCount[dvipage-1];
	ReleasePages();
/*	if (bCarryColor && bColorUsed) ReleaseColor(); */	/* ??? */
	return current;
}

/* This sets dvipage to the appropriate bop count in file */
/* wantedpage is either requested dvipage if flag == 0 */
/*		or requested counter[0] if flag != 0 */
/* if flag > 0 then search forward for it */
/* if flag < 0 then search backward for it */
/* returns result in dvipage */

int usepagetable (int wantedpage, int flag) {
	LPLONG PageStart;	/* Pointer to Page Start Table */
	LPLONG PageCount;	/* Pointer to Count Zero Table */
	int k=1;			/* avoid uninitialized 98/Mar/26 */
	int found=0;

	if (bFileValid == 0) return dvipage;			/* 0 */
	if (hPages == NULL) return dvipage;				/* 0 */
	if (wantedpage == -INFINITY) return dvipage;	/* 0 */

	GrabPages();
	PageStart = lpPages;
	PageCount = lpPages + dvi_t;			/* MaxPage ? */
/*	if (bCarryColor && bColorUsed) GrabColor(); */
#ifdef DEBUGPAGETABLE
	if (dvi_t != MaxPage) {			/* debugging code 1993/Aug/28 */
		if (bDebug) {
			sprintf(str, "dvi_t %d != MaxPage %d (usepagetable)",
					dvi_t, MaxPage);
			winerror(str);
		}
	}
#endif
#ifdef DEBUGPAGETABLE
	if (bDebug > 1) {
		sprintf(debugstr, "USEPAGETABLE wanted %d flag %d dvipage %d", wantedpage, flag, dvipage);
		OutputDebugString(debugstr);
	}
#endif
	if (flag != 0) {				/* wantedpage is requested counter[0] */
		if (dvipage < 0) dvipage = 0;			/* sanity check 99/May/31 ? */
		if (dvipage > dvi_t) dvipage = dvi_t+1;	/* sanity check 99/May/31 ? */
		if (flag > 0) {				/* go forward to next page with same counter[0] */
			for (k = dvipage+1; k <= dvi_t; k++) {		/* search forward */
				if (PageCount[k-1] == wantedpage) {	found = 1;	break;	}
			}
			if (found == 0) {	/* wrap around */
				for (k = 1; k <= dvipage; k++) {		/* wrap around */
					if (PageCount[k-1] == wantedpage) {	found = -1; break;	}
				}
			}
		}
		else if (flag < 0) { /* go back to page with same counter[0] */
			for (k = dvipage-1; k >= 1; k--) {			/* search backward */
				if (PageCount[k-1] == wantedpage) {	found = 1;	break;	}
			}
			if (found == 0) {	/* wrap around */
				for (k = dvi_t; k >= dvipage; k--) {	/* wrap around */
					if (PageCount[k-1] == wantedpage) {	found = -1; break;	}
				}
			}
		}
		if (found == 0) {
			if (bDebug > 1) {
				sprintf(debugstr, "%s page %d not found",
						flag ? "Physical" : "Logical", wantedpage);
				OutputDebugString(debugstr);
			}
/*			winerror(debugstr); */	/* bad thing if happens while painting page !!! */
/*			if (flag > 0) dvipage = dvi_t;	
			else dvipage = 1; */ /* maybe just leave it where it was 99/May/31 ??? */
		}
		else dvipage = k;				/* we did find as matching page */
	} /* end of if flag != 0 */
	else dvipage = wantedpage;	/* easy case: not counter[0], but just dvi page */

#ifdef DEBUGPAGETABLE
	if (bDebug > 1) {
		sprintf(debugstr, "USEPAGETABLE wanted %d flag %d dvipage %d  dvi_t %d spread %d count[0] %d\n",
				wantedpage, flag, dvipage, dvi_t, bSpreadFlag, bCountZero);
		OutputDebugString(debugstr);
	}								/* debugging - remove again 97/May/10 */
#endif

	if (bSpreadFlag) {				/* asking for a `spread' */
		if (dvipage > dvi_t) {
/*			if last page is odd */
/*			if ((PageCount[dvi_t] & 1) != 0) dvipage = dvi_t; */
			if ((PageCount[dvi_t-1] & 1) != 0) dvipage = dvi_t;	/* FIXED! 99/May/31 */
		}
		if (dvipage < 2) {
/*			if first page is even */
			if ((PageCount[0] & 1) == 0) dvipage = 2; /* 97/May/10 ? */
		}

#ifdef DEBUGPAGETABLE
		if (bDebug > 1) {
			sprintf(debugstr, "dvipage %d PageCount[0] %d PageCount[%d] %d",
					dvipage, PageCount[0], dvi_t, PageCount[dvi_t-1]);
			OutputDebugString(debugstr);
		}
#endif
		if (dvipage < 2) {			/* beginning of file */
			leftdvipage = -1;
			leftcountzero = -INFINITY;
			leftcurrent = -1;
		}
		else {						/* normal case, there IS a left page, maybe */
			leftdvipage = dvipage - 1;
			if (leftdvipage > 0 && leftdvipage <= dvi_t) { 
				leftcountzero = PageCount[leftdvipage-1];
				leftcurrent = PageStart[leftdvipage-1];
			}
			else {
				leftcountzero = -INFINITY;
				leftcurrent = -1;
			}
#ifdef DEBUGPAGETABLE
			if (bDebug > 1) {
				sprintf(debugstr, "leftcountzero %d leftcurrent %d", leftcountzero, leftcurrent);
				OutputDebugString(debugstr);
			}
#endif
			if (bCountZero != 0 && (leftcountzero & 1) != 0) {
/*				leftdvipage = -1; */
				leftcountzero = -INFINITY;
				leftcurrent = -1;
#ifdef DEBUGPAGETABLE
				if (bDebug > 1) OutputDebugString("Left page not even page");
#endif
			}						/* left page not an even page */
		}

		if (dvipage > dvi_t) { /* end of file */
			rightdvipage = -1;
			rightcountzero = -INFINITY;
			rightcurrent = -1;
		}
		else {					/* not at end of file */
			rightdvipage = dvipage;
			if (rightdvipage <= dvi_t && rightdvipage > 0) {
				rightcountzero = PageCount[rightdvipage-1];
				rightcurrent = PageStart[rightdvipage-1];
			}
			else {
				rightcountzero = -INFINITY;
				rightcurrent = -1;
			}
#ifdef DEBUGPAGETABLE
			if (bDebug > 1) {
				sprintf(debugstr, "rightcountzero %d rightcurrent %d", rightcountzero, rightcurrent);
				OutputDebugString(debugstr);
			}
#endif
			if (bCountZero != 0 && (rightcountzero & 1) == 0) {
/*				rightdvipage = -1; */
				rightcountzero = -INFINITY;
				rightcurrent = -1;
#ifdef DEBUGPAGETABLE
				if (bDebug > 1) OutputDebugString("Right page not odd page");
#endif
			}						/* right page not an odd page */
		}
/*		why is the following darn thing needed ? */
		current = rightcurrent;
/*		counter[0] = rightcountzero; */
		if (rightcountzero > 0 && rightcountzero <= dvi_t)	/* 99/May/31 */
			counter[0] = rightcountzero;	/* ??? */
		else counter[0] = leftcountzero;	/* ??? */
#ifdef DEBUGPAGETABLE
		if (bDebug > 1) {
			sprintf(debugstr, 
	"USEPAGETABLE leftdvi %d rightdvi %d leftcount %d rightcount %d leftcurrent %d rightcurrent %d",
			leftdvipage, rightdvipage, leftcountzero, rightcountzero, leftcurrent, rightcurrent);
			OutputDebugString(debugstr);
		}
#endif
	}			/* end of if bSpreadFlag */
	else {			/* not in spread mode */
		if (dvipage > dvi_t) dvipage = dvi_t;
		else if (dvipage < 1) dvipage = 1;
		current = PageStart[dvipage-1];
		counter[0] = PageCount[dvipage-1];
	}
	ReleasePages();
/*	if (bCarryColor && bColorUsed) ReleaseColor(); */
/*	return wantedpage;	*/ /* unchanged ... */
	return dvipage;
}

/* Returns non-zero if user clicked on menu item or hit a key */
/* NOT SURE ABOUT THIS !!! */
/* Used in winanal.c, winspeci.c, winsearc.c */

int checkuser (void) {	
	MSG peekmsg;
	int x, y;
	WORD id;
	POINTS pts;									/* 95/Nov/2 */

/*	savedmark = 0;
	for (k = 0; k < 1024; k++) marked[k] = '\0'; */

	if (bEnableTermination == 0) return 0;

/* wHitTestCode = wParam;  hit-test code              
xPos = LOWORD(lParam);     horizontal cursor position 
yPos = HIWORD(lParam);     vertical cursor position   
*/ /* WIN16 */

/* nHittest = (INT) wParam;  hit-test value 
pts = MAKEPOINTS(lParam);    position of cursor 
*/

/*  return non-zero if PeekMessage shows user wants something done */
/*  did he press a button in a non-client area ? */
/*	May want to look at wParam WM_NCHITTEST hittest value HTCLIENT ??? */
	if (PeekMessage(&peekmsg, hwnd,
		WM_NCLBUTTONDOWN, WM_NCLBUTTONDOWN, PM_NOREMOVE) != 0)  {
/*		will need rework in Windows NT */ /* WIN32 ??? */

		pts = MAKEPOINTS(peekmsg.lParam);
		x = pts.x;
		y = pts.y;

/*		x = (int) LOWORD(peekmsg.lParam); */
/*		y = (int) HIWORD(peekmsg.lParam); */

		if (bDebug > 1) {
			sprintf(debugstr, "NCLBUTTONDOWN x %d y %d\n", x, y);
			OutputDebugString(debugstr);
		}					/* 1995/Dec/10 */
/*		do more checking ? use coordinates ? click on menu item ? */
		if (y > GetSystemMetrics(SM_CYCAPTION) &&
			y < GetSystemMetrics(SM_CYCAPTION) +
				GetSystemMetrics(SM_CYMENU)) {
/*			if (x > 120 && x < 520) */		/* need a better way ! */
			if (x > 120 && x < 530)		/* 1995/Dec/10 ! */
				return -1;
		}
		if (bDebug > 1) OutputDebugString("IGNORED\n");
	}

	if (PeekMessage(&peekmsg, hwnd,
		WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE) != 0) {

/*	do more checking - accept accelerator keys only ! */
		id = (WORD) GET_WM_COMMAND_ID(peekmsg.wParam, peekmsg.lParam);
/*	debugging check on which keys down are seen and which are ignored */
#ifdef DEBUGMAIN
		if (bDebug > 1) {
			sprintf(debugstr, "KEYDOWN %d\n", id);
			OutputDebugString(debugstr);
		}					/* 1995/Dec/10 */
#endif
/* interrupt repainting for certain keys that indicate users wants to go on */
		if (id == VK_SUBTRACT ||	id == VK_ADD ||
			id == VK_PRIOR ||		id == VK_NEXT ||
			id == VK_MULTIPLY ||	id == VK_ESCAPE ||
			id == VK_INSERT ||		id == VK_DELETE ||
			id == VK_HOME ||		id == VK_END ||
			id == VK_SPACE ||		id == VK_BACK
/*			id == VK_DOWN ||		id == VK_UP || */		
/*			id == VK_LEFT ||		id == VK_RIGHT || */
			) return -1;
#ifdef DEBUGMAIN
		if (bDebug > 1) OutputDebugString("IGNORED\n");
#endif
	}

/*	for (k = 0; k < 1024; k++) {
		if (PeekMessage(&peekmsg, hwnd, k, k, PM_NOREMOVE) != 0) {
			marked[k] = 1;
			savedmark = 1;
		}
	} */
	return 0;			/* no significant action */
}

#ifdef DEBUGMAIN
void ShowFileInfo (char *name, DWORD FileSizeHigh, DWORD FileSizeLow,
				   DWORD FilewTimeHigh, DWORD FilewTimeLow) { 
	sprintf(debugstr, "%s\tFileSize %d %d\tFilewTime %d %d\n", name,
			FileSizeHigh, FileSizeLow, FilewTimeHigh, FilewTimeLow);
	if (bDebug > 1) OutputDebugString(debugstr);
}
#endif

int ReOpenFile(HWND hWnd) {
	HDC hDC;							/* 93/Sep/21 */
	HFILE hFileTemp;					/* 95/Dec/6 */
	DWORD err;							/* 96/Jan/10 */
#ifdef LONGNAMES
/*	char FileName[MAXFILENAME]; */		/* use shared global 95/Dec/6 */
#endif

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("ReOpenFile\n"); /* DEBUGGING */
#endif

	if (hFile != HFILE_ERROR) return hFile;		/* if still open - NEW */

/*	do something unpleasant only if file has actually disappeared ! */
/*  following leads to sharing violations without OfShareCode */
/*	- and is basically useless with OfShareCode ... */
#ifdef LONGNAMES
/*	There may be a problem here if OpenName is fully qualified ! */
/*	strcpy(FileName, DefPath); */
	if (strchr(OpenName, '\\') != NULL ||
		strchr(OpenName, '/') != NULL ||
		strchr(OpenName, ':') != NULL) *FileName = '\0';
	else strcpy(FileName, DefPath);
	strcat(FileName, OpenName);
/*	hFileTemp = _lopen(FileName, READ | OF_SHARE_DENY_NONE); */
	hFileTemp = _lopen(FileName, READ | OfExistCode);	/* 96/May/18 */
	if (hFileTemp == HFILE_ERROR) {
		if (bDebug > 1) {

			err = GetLastError();

/*			err = errno; */

			sprintf(debugstr, "ReOpen exists error code %ld on %s\n",
					err, FileName);
			OutputDebugString(debugstr);
/*			possibly use doserror() in winprint.c to show this ? */
		}
/*		if it seems like *not* sharing violation kill file as below */
/*		check errno: if its 32 can continue without killing file connect */		
		return -1;
	}
	else _lclose (hFileTemp);
/*	not sure it makes sense to close and then reopen ... */
#else
/*	In Windows 95 could use OpenFileEx with structure OPENFILEX */
/*	Documented in Windows 95 DDK (April 1996 Windows Developer Journal) */
/*  Need to use GetProcAddress to access OpenFileEx however */	
	if (OpenFile((LPSTR) NULL, &OfStruct,
		 OF_REOPEN | OF_EXIST | OF_SHARE_DENY_NONE) == HFILE_ERROR) {
		if (bDebug > 1) {
			sprintf(debugstr, "ReOpenFile exist check error %d on %s\n",
					OfStruct.nErrCode, OpenName);
			OutputDebugString(debugstr);
		}
/* assume the file has gone, UNLESS sharing violation */
/* expect error code 2 if `file not found', 32 if sharing violation */
		if (OfStruct.nErrCode != 32) {	/* 0x0020 sharing violation */
			bFileValid = 0;
			closeup(hWnd, 1);
#ifdef HYPERFILE
			free_hyperfile();		/* right place ??? winsearc.c */	
#endif
		}
		return -1;
	}  /* end of if OpenFile in error */
#endif /* end of LONGNAMES */

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("OpenFile\n"); /* DEBUGGING */
#endif

/* seems redundant if we already opened it above using _lopen */
/* except we use different flags here ??? */

#ifdef LONGNAMES
/*	strcpy(FileName, DefPath); */
	if (strchr(OpenName, '\\') != NULL ||
		strchr(OpenName, '/') != NULL ||
		strchr(OpenName, ':') != NULL) *FileName = '\0';
	else strcpy(FileName, DefPath);
	strcat(FileName, OpenName);
/*	hFile = _lopen(FileName, READ | OF_SHARE_DENY_WRITE); */
	hFile = _lopen(FileName, READ | OfShareCode); 
	if (hFile  == HFILE_ERROR) {
		if (bDebug > 1) {

			err = GetLastError();

/*			err = errno; */

			sprintf(debugstr, "ReOpen error open code %ld on %s\n",
					err, FileName);
			OutputDebugString(debugstr);
/*			possibly use doserror() in winprint.c to show this ? */
		}
		return -1;
	}
#else
	hFile = OpenFile((LPSTR) NULL, &OfStruct, 
/*			OF_REOPEN | OF_READ | OF_SHARE_DENY_WRITE */
/*			OF_REOPEN | OF_READ | OF_SHARE_DENY_NONE */
			OF_REOPEN | OF_READ | OfShareCode);
	if (hFile  == HFILE_ERROR) { 
/*	following is commented out as an experiment 1993/Jan/5 */
/*				bFileValid = 0; */
/*		may not be safe to give error message at this time ! */
				if (bDebug > 1) {
					sprintf(debugstr, "ReOpenFile open error %d on %s\n",
							OfStruct.nErrCode, OpenName);
					OutputDebugString(debugstr);
				}
/*	maybe check OfStruct.nErrCode at this point ? */
/*	following is commented out as an experiment 1993/Jan/5 */
/*				closeup(hWnd, 1); */ 			/* file disappeared ! */
				return -1;					/* error indication */
	}
#endif

/*	re-opened successfully - BUT, is it the same file ? */
/*	use OF_VERIFY for this instead ? */ /* or OfStruct.reserved[] */
	if (GetFileTime ((HANDLE) hFile, NULL, NULL, &NewFilewTime) == 0) {
		if (bDebug > 1)
			OutputDebugString("GetFileTime failed\n"); /* Debugging */
/*		use GetLastError() to get more information */
	}
	NewFileSizeLow = GetFileSize ((HANDLE) hFile, &NewFileSizeHigh);
/*	if (bDebug > 1) ShowFileInfo("ReOpenFile", NewFileSizeHigh, NewFileSizeLow,
			 NewFilewTime.dwHighDateTime, NewFilewTime.dwLowDateTime); */

#ifdef IGNORED
/*	(void) _fstat(hFile, &FileStatus); */
	if (_fstat(hFile, &FileStatus) != 0) {
		if (bDebug > 1) OutputDebugString("_fstat failed\n");
		if (errno == EBADF) {
			if (bDebug > 1) OutputDebugString("Bad File Handle\n");
		}			
	}
/*	if (bDebug > 1) {
		sprintf(debugstr, "Drive %c:\n", FileStatus.st_dev + 'A');
		OutputDebugString(debugstr);
		sprintf(debugstr, "File size %d\n", FileStatus.st_size);
		OutputDebugString(debugstr);
		sprintf(debugstr, "Mode %x\n", FileStatus.st_mode);
		OutputDebugString(debugstr);
		sprintf(debugstr, "Time accessed %s\n", ctime( &FileStatus.st_atime));
		OutputDebugString(debugstr);
		sprintf(debugstr, "Time modified %s\n", ctime( &FileStatus.st_mtime));
		OutputDebugString(debugstr);
		sprintf(debugstr, "Time created %s\n", ctime( &FileStatus.st_ctime));
		OutputDebugString(debugstr);
	} */ /* code to test _fstat(); */
#endif

/*	if (NewFileSizeLow < 32 && NewFileSizeHigh == 0) */
	if (NewFileSizeLow < 128 && NewFileSizeHigh == 0) 	

/*	if (FileStatus.st_size < 32) */

		{
		if (bDebug > 1)	OutputDebugString("FileSize < 32\n"); /* DEBUGGING */
/*		closeup(hWnd, 1); */		/* file open for writing ? */
		return -1;					/* error indication */		
	}

	if (FileSizeLow != NewFileSizeLow ||
		FileSizeHigh != NewFileSizeHigh ||
		FilewTime.dwLowDateTime != NewFilewTime.dwLowDateTime ||
		FilewTime.dwHighDateTime != NewFilewTime.dwHighDateTime) 
/* or, use CompareFileTime(FilewTime, NewFilewTime) != 0 */

/*	if (FileSize != FileStatus.st_size ||
		FilemTime != FileStatus.st_mtime) */

	{
		if (bDebug > 1) {
			OutputDebugString("ReOpen: ");
			OutputDebugString("File Size or Time have changed\n"); 
		}

		FileSizeLow = NewFileSizeLow;
		FileSizeHigh = NewFileSizeHigh;
		FilewTime = NewFilewTime;

/*		FileSize = FileStatus.st_size;
		FilemTime = FileStatus.st_mtime; */

/*		file has changed - need to redo prescan */
		bFileValid = 0;
		current = -1;
		lastsearch = -1;		/* no longer meaningful ??? */
#ifdef IGNORED
		free_hyperfile(); 
#endif
		hyperindex = 0; 		/* reset hypertext index */ 
		colorindex = 0;			/* reset color stack index */

/*  may need to reset some other things here also ??? 93/Sep/21 */		
		hidedialogs(hWnd);		/* ??? 99/Feb/28 */
/*	as in closeup ??? FreePages ??? 93/Sep/21 */ /* do more ? */
		if (hPages != NULL) hPages = GlobalFree(hPages);
		if (hBack != NULL) hBack = GlobalFree(hBack);
		if (hColor != NULL) {
/*			also free saved stacks */
			hColor = GlobalFree(hColor);
		}
		hDC = GetDC(hWnd);
		if ((UINT) hFontOld > 1)
			(void) SelectObject(hDC, hFontOld);
		resetfonts();
		(void) ReleaseDC(hWnd, hDC);  /* ??? 93/Sep/21 */

#ifdef DEBUGMAIN
		if (bDebug > 1) OutputDebugString("DoPreScan\n"); /* DEBUGGING */
#endif

		if (DoPreScan(hWnd, hFile) != 0) {	/* try prescanning */
			closeup(hWnd, 1);				/* failed - safe ??? */
#ifdef HYPERFILE
			free_hyperfile();		/* right place ??? winsearc.c */	
#endif
			return -1;						/* error indication */
		}

/*		lastsearch = -1; */			/* no longer meaningful ??? */
		usepagetable(dvipage, 0);	/* setup dvipage etc ??? */
/*		Is this InvalidateRect really needed ? */
/*		if (bWasIconic == 0) InvalidateRect(hWnd, NULL, TRUE); */
/*		The following attempts to kills the double refresh when DVI changes */
		if (bAwakened != 0) bAwakened = 0;		/* 1993/July/12 */
/*		else if (bWasIconic == 0) InvalidateRect(hWnd, NULL, TRUE); */
		else if (bWasIconic == 0) {
/* trial balloon 94/Aug/10 - Use RefreshDelay=0 to revert to old behaviour */
			if (nRefreshDelay == 0)	InvalidateRect(hWnd, NULL, TRUE);
		}
	} /* end of if change in file time  or size */
	bWasIconic = 0;
#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("SUCCESS ReOpenFile\n"); /* debugging */
#endif
	return 0;
}

/* following used by WM_PAINT when bFileValid != 0 */
/* returns -1 if file open failed */

int paintpage(HWND hWnd, HDC hDC) {
/*	int oldrelative, newrelative=1; */
	int readflag;

/*	(void) SetMapMode(hDC, MM_TWIPS);	*/		/* set unit to twips */
/*	now try and reopen the DVI file */

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("Enter paintpage\n");
#endif
/*	if (bDebug > 1) {
		sprintf(debugstr, "ROP2 %d\n", GetROP2(hDC));
		OutputDebugString(debugstr);
	} */

	if (ReOpenFile(hWnd) < 0) return -1;		/* file failed to open */

#ifdef DEBUGMAIN
	if (bDebug > 1) OutputDebugString("Reopened file\n");
#endif

	if (bCopyFlag == 0)					/* should never be non-zero here ! */
		(void) SetMapMode(hDC, MM_TWIPS);			/* set unit to twips */
/*	everything is now just fine and dandy, that is, */
/*	EITHER: it opened and was same file, OR: we redid prescan above */
/*	if (bSpreadFlag != 0 && bCountZero != 0) initialspread(dvipage); */
/*	if (bPrintFlag == 0) */		/* do this only for screen display */
	if (! bPrintFlag && ! bCopyFlag) { /* do only for screen display */
		SetupWindowText(hWnd);
		if (bShowBorder) DrawBorder(hDC, hBorderPen);
		if (bShowLiner) DrawLiner(hDC, hLinerPen);
#ifdef IGNORED
		if (bShowViewPorts != 0) DrawBBox(hDC, hFigurePen); 	/* 96/Sep/29 */
#endif
	}
	if (! bRuleFlag) {						/* 1993/Feb/22 */
		bHourFlag = -1;
		hSaveCursor = SetCursor(hHourGlass);
	}

	GrabWidths();
	readflag =  scandvifile(hFile, hDC, 1);  /* read page and display it */
	ReleaseWidths();

	if (bDialogsHidden) restoredialogs(hWnd);	/* new 1999/Jan/8 moved here ? */
/*	bBusyFlag = 0;	*/		/* indicate request serviced */
	bEnableTermination = 0;
	if (bKeepFileOpen == 0) {
/*		if (hFile < 0) wincancel("File already closed"); */
		if (hFile == HFILE_ERROR) {
/*			wincancel("File already closed"); */
			if (bDebug > 1) {
				sprintf(debugstr, "%s (%s)\n", "File already closed", "paintpage");
				OutputDebugString(debugstr);
			}
		}
/*		if(_lclose(hFile) != 0) winerror("Unable to close file"); */
		else _lclose(hFile);
/*		hFile = -1; */				/* close it again */
		hFile = HFILE_ERROR;		/* close it again */
	}
	if (bRuleFlag == 0) {						/* 1993/Feb/22 */
		bHourFlag = 0;
		(void) SetCursor(hSaveCursor);
	}

/*	was requested page in fact found ? readflag set from scandvifile(...) */
	if (readflag == 0 && bPrintFlag == 0 && bCopyFlag == 0) {
/*		if (bCountZero != 0) sprintf(str, "Page %ld not found", counter[0]); 
		else sprintf(str, "Page %ld not found", dvipage); */
		if (bDebug > 1) {
			sprintf(debugstr, "%s page %d not found", bCountZero ? "Logical" : "Physical",
				bCountZero ? counter[0] : dvipage);
			OutputDebugString(debugstr);
		}
/*		wininfo(debugstr);	*/		/* bad when happens while repainting screen */
/*		set to last seen, unless already there ... */
		if (dvipage == (int) dvi_t)	dvipage = -INFINITY;	
		else dvipage = (int) dvi_t;
		if (dvipage == -INFINITY) checkcontrols(hWnd, 0);
		InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
	}
	return 0;			/* indicate success */
}

/* use new mapping and reset fonts */

void newmapandfonts (HWND hWnd) {
	HDC hDC;

	setscale(wantedzoom);
	hDC = GetDC(hWnd);
	if ((UINT) hFontOld > 1)	/* avoid Windows MetaFile problem */
		(void) SelectObject(hDC, hFontOld);
	clearfonts();				/* but DONT reset metrics */
	(void) ReleaseDC(hWnd, hDC);
	VScrollPos(hWnd);			/* needed ? */
	HScrollPos(hWnd);			/* needed ? */
}

void savegraphicstate (void) {
	int k;
	if (graphicindex == MAXGRAPHICSTACK - 1) {	// avoid overflow
		for (k = 1; k < MAXGRAPHICSTACK; k++) {
			xoffsetsaved[k-1] = xoffsetsaved[k];
			yoffsetsaved[k-1] = yoffsetsaved[k];
			wantedzoomsaved[k-1] = wantedzoomsaved[k];
		}
		graphicindex--;
	}
	if (graphicindex < MAXGRAPHICSTACK - 1) {
		xoffsetsaved[graphicindex] = xoffset;
		yoffsetsaved[graphicindex] = yoffset;
		wantedzoomsaved[graphicindex] = wantedzoom;
		graphicindex++;
	}
}

int restoregraphicstate (void) {
	if (graphicindex > 0) {
		graphicindex--;
		xoffset = xoffsetsaved[graphicindex];
		yoffset = yoffsetsaved[graphicindex];
		wantedzoom = wantedzoomsaved[graphicindex];
		return -1;
	}
	else return 0;
}

void adjusttobottom (HWND hWnd) {		/* used by Page Bottom */
	HDC hDC;
	RECT ClientRect;
	long ybottom, yll;
	
	GetClientRect(hWnd, &ClientRect);
	ClientRect.bottom--;
	hDC = GetDC(hWnd);
	if (bCopyFlag == 0)			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS); /* set unit twips */
	ClientRect = DPtoLPRect(hDC, ClientRect);
	(void) ReleaseDC(hWnd, hDC);
	if (bFontSample == 0)
		yll = oneinch + oneinch/10 + (long) dvi_l;
	else {
/*		sprintf(str, "a %ld d %ld h %ld", chara, chard, charh);
		winerror(str); */ /* debugging */
		yll = (long) oneinch + (long) oneinch/10 + 
			chara + chard + charh * 15 + (charh * 15) / 5;
		yll = yll / dvi_mag * 1000;		/* kludge fix */
	}
	ybottom = - lmapd(yll) + yoffset;
	yoffset += ClientRect.bottom - ybottom;
	VScrollPos(hWnd);		/* 1993/Feb/22 */
/*	HScrollPos(hWnd); */		/* 1993/Feb/22 */
}

void adjusttotop (HWND hWnd) {			/* used by Page Top */
	HDC hDC;
	RECT ClientRect;
	long ytop, yur;
	
/*	use New Page Scale set by user */	/* 1993/Feb/15 */
	if (bUseNewPageOffset != 0 && bFontSample == 0) {
/*		yoffset = pageyoffset; */
		if (bSpreadFlag == 0) yoffset = pageyoffset;
		else yoffset = spreadyoffset;	/* 1996/May/12 fixed */
		VScrollPos(hWnd);		/* 1993/Feb/22 */
/*		HScrollPos(hWnd); */		/* 1993/Feb/22 */
		return;
	}

/* otherwise try to adjust so DVI origin is just below top of Window */

	GetClientRect(hWnd, &ClientRect);
	ClientRect.bottom--;
	hDC = GetDC(hWnd);
	if (bCopyFlag == 0)			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS); 
	ClientRect = DPtoLPRect(hDC, ClientRect);
	(void) ReleaseDC(hWnd, hDC);
/*	if (bLandScape == 0) yur = 0; else yur = 0; */
	if (bFontSample == 0)
		yur = oneinch - oneinch/10;
/*	else yur = 0; */
	else {
/*		sprintf(str, "a %ld d %ld h %ld", chara, chard, charh);
		winerror(str); */ /* debugging */
/*		yur = (long) ONEINCH - (long) ONEINCH/10 + chara - charh - charh/5; */
		yur = (long) oneinch - (long) oneinch/10;
		yur = yur / (long) dvi_mag * 1000;		/* kludge fix */
	}
	ytop = - lmapd(yur) + yoffset;
	yoffset += ClientRect.top - ytop;
/*	possible scaling problems if zooms don't match ? */
	VScrollPos(hWnd);			/* 1993/Feb/22 */
/*	HScrollPos(hWnd); */		/* 1993/Feb/22 */
} 

void EnterFontState (HWND hWnd) {
	xoffsetold = xoffset;
	yoffsetold = yoffset;
	wantedzoomold = wantedzoom; /* save graphics state */
	xoffset = fontxoffset;
	yoffset = fontyoffset;
	wantedzoom = fontzoom;		/* font view state 96/Aug/26 */
/*	newmapandfonts(hWnd) */
/*	bFontSample = 1; */
}

/* restore graphics state after finish showing Font Sample */

void EndFontState (HWND hWnd) {
	fontxoffset = xoffset;
	fontyoffset = yoffset;
	fontzoom = wantedzoom;		/* remember font view state 96/Aug/26 */
	xoffset = xoffsetold;
	yoffset = yoffsetold;
	wantedzoom = wantedzoomold; /* restore graphics state */
	newmapandfonts(hWnd);		/* need to do this ??? */
	bFontSample = 0;
	bCharacterSample = 0;
}

void selectpage (HWND hWnd) {
/*	enablecontrols(hWnd); */
	checkcontrols(hWnd, +1);
	SetupWindowText(hWnd);
	if (bResetPage != 0) {
		resetpagemapping(hWnd);
		newmapandfonts(hWnd);
		resetgraphicstate();
	}
	bBadFileComplained=0;			/* 1995/Mar/28 */
	bBadSpecialComplained=0;		/* 1996/Feb/4 */
}

void graypreviousnext (HWND hWnd) {
	HMENU hMenu;
	int previous = 0, next = 0;

	if (bGrayFlag == 0) return;				/* never grayed, so no need */
	if (dvipage > 1) previous = 1;
	if (dvipage == 1) {						/* 1993/Feb/14 */
		if (bResetPage != 0) previous = 1;
	}
	if (dvipage < dvi_t) next = 1;
	if (dvipage == dvi_t) {					/* 1993/Feb/14 */
		if (bResetPage != 0) next = 1;
	}
	if (previous != oldprevious || next != oldnext)	{
		hMenu = GetMenu(hWnd);
		(void) EnableMenuItem(hMenu, IDM_FIRSTPAGE,
							  (dvipage > 1) ? MF_ENABLED : MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_PREVIOUS, 
							  (dvipage > 1) ? MF_ENABLED : MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_NEXT, 
							  (dvipage < dvi_t) ? MF_ENABLED : MF_GRAYED);
		(void) EnableMenuItem(hMenu, IDM_LASTPAGE, 
							  (dvipage < dvi_t) ? MF_ENABLED : MF_GRAYED);
		DrawMenuBar(hWnd);
		oldprevious = previous; oldnext = next;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* check whether DVIPSONE is (still) running */ /* return non-zero if so */

/* New improved version 95/Mar/12 */ /* May need work in NT */ /* WIN32 ??? */

/* BOOL IsTaskActive(HWND hWnd) */
BOOL IsTaskActive(void) {
	char title[MAXTITLE];

/*	Check if we already forgot about this task */
	if (TaskInfo.hWnd == NULL) return FALSE;

/*	Is it a valid Windows handle ? */
	if (IsWindow(TaskInfo.hWnd) == 0) goto forgettask;

/*	Does the Window belong to the same Instance ? */ /* WIN32 ??? */

	if ((HINSTANCE) GetWindowLong(TaskInfo.hWnd, GWL_HINSTANCE) !=
		TaskInfo.hInstance) goto forgettask;

/*	if ((HINSTANCE) GetWindowWord(TaskInfo.hWnd, GWW_HINSTANCE) !=
		TaskInfo.hInstance) goto forgettask; */

/*	Does the Task belong to the Window ? */
	if (GetWindowTask(TaskInfo.hWnd) != TaskInfo.hTask) goto forgettask;

/*	Check if Window title is now `Inactive' */
	(void) GetWindowText (TaskInfo.hWnd, title, MAXTITLE);
	if (strstr(title, "Inactive") == NULL) return TRUE;	/* it is still alive */

forgettask:
	if (bDebug > 1)	OutputDebugString ("WinExe Task finished\n");
	TaskInfo.hWnd = NULL; 
	return FALSE;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void setupregions (void) { /* reset regions - create new if needed */
	if (hRgn == NULL) hRgn = CreateRectRgn(0, 0, 0, 0);
	else SetRectRgn(hRgn, 0, 0, 0, 0);
	if (hUpdateRgn == NULL)	hUpdateRgn = CreateRectRgn(0, 0, 0, 0); 
	else SetRectRgn(hUpdateRgn, 0, 0, 0, 0); 
}

void addtoregion(HWND hWnd, BOOL flag) { /* grab update region add overall */
	if (hRgn == NULL || hUpdateRgn == NULL)	setupregions(); /* 94/Dec/13 */
	(void) GetUpdateRgn(hWnd, hUpdateRgn, flag);
	CombineRgn(hRgn, hRgn, hUpdateRgn, RGN_OR);
/*	ValidateRgn(hWnd, NULL); */
	ValidateRgn(hWnd, hUpdateRgn);		/* 92/April/19 */
}

void forceregion(HWND hWnd) {		/* finally force repainting ! */
	InvalidateRgn(hWnd, hRgn, TRUE);
	SetRectRgn(hRgn, 0, 0, 0, 0);		/* reset to save memory */
	SetRectRgn(hUpdateRgn, 0, 0, 0, 0); /* reset to save memory */
}

void showsize(HDC hDC, int startx, int starty, int endx, int endy) {
	int width, height;
	int textx, texty;
	int oldmapmode;
	char numbers[32];
	RECT RuleRect;
	int dx, dy;
	double fwidth, fheight;
	int picas, points;		/* for pica + point mode 1993/Dec/16 */
	double fpoints;			/* 1994/Feb/25 */
	int precision=0;

	if (bDisableSize != 0) return;

	dx = 100;				/* use Dialog Units instead ? */
	dy = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);

	if (units > 6) precision = 3;	
	else if (units > 3) precision = 2;
	else if (units > 0) precision = 1;
	else precision = 0;
	if (wantedzoom > 5) precision++;
	else if (wantedzoom < -5) precision--;	
	if (precision < 0) precision = 0;
	else if (precision > 3) precision = 3;
	if (units == 0) precision = 0;			/* scaled points */
	if (units == 9) precision = 0;			/* pica and points */

	RuleRect.left = (startx < endx) ? startx : endx;
	RuleRect.right = (startx < endx) ? endx : startx;
	RuleRect.top = (starty < endy) ? starty : endy;
	RuleRect.bottom = (starty < endy) ? endy : starty;

	oldmapmode = SetMapMode(hDC, MM_TWIPS); 
	DPtoLP(hDC, (LPPOINT) &RuleRect, 2);
	(void) SetMapMode(hDC, oldmapmode); 	

	width = RuleRect.right - RuleRect.left;
	if (width < 0) width = - width; 	
	textx = (startx + endx) / 2; 	
	texty = (starty > endy) ? starty : endy;
/*	Is this assuming standard dvi_num and dvi_den ??? */
	fwidth = (double) unmap(width) / 65536.0;
	if (bTrueInch != 0) fwidth = fwidth * dvi_mag / 1000.0;
	fwidth = fwidth / unitscale[units];
	if (pcptflag) {
		picas = (int) fwidth;
/*		points = (int) ((fwidth - picas) * 12.0); */
/*		sprintf(numbers, " %d pc %d pt ", picas, points  ); */
		points = (int) ((fwidth - picas) * 120.0);	/* tenth of points */
		fpoints = (double) points / 10.0;
		sprintf(numbers, " %d pc %lg pt ", picas, fpoints );
	}
/*	is the following %6.*lf right ??? */
	else sprintf(numbers, " %6.*lf %s ", precision, fwidth, unitnames[units]);
/*	SetWindowPos(hWidth, NULL, textx - dx/2, texty + dy/2, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER); */
	SendMessage(hWidth, WM_SETTEXT, 0, (LPARAM) (LPSTR) &numbers); 

	height = RuleRect.top - RuleRect.bottom;
	if (height < 0) height = - height;
	texty = (starty + endy) / 2; 	
	textx = (startx > endx) ? startx : endx;
/*	Is this assuming standard dvi_num and dvi_den ??? */
	fheight = (double) unmap(height) / 65536.0;
	if (bTrueInch != 0) fheight = fheight * dvi_mag / 1000.0;
	fheight = fheight / unitscale[units];
	if (pcptflag) {
		picas = (int) fheight;
/*		points = (int) ((fheight - picas) * 12.0); */
/*		sprintf(numbers, " %d pc %d pt ", picas, points); */
		points = (int) ((fheight - picas) * 120.0);
		fpoints = (double) points / 10.0;
		sprintf(numbers, " %d pc %lg pt ", picas, fpoints);
	}
	else sprintf(numbers, " %6.*lf %s ", precision, fheight, unitnames[units]);
/*	SetWindowPos(hHeight, NULL, textx + dy/2, texty - dy/2, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER); */
	SendMessage(hHeight, WM_SETTEXT, 0, (LPARAM) (LPCSTR) &numbers); 
} 

/* need to save what is under the digits */
/* need offset from coordinates selected */
/* need measure of text width and height */
/* need font selection above */
/* need text color selection above */
/* need conversion to invariant measure OK */
/* need selection of units of measurement */
/* why does it leave behind arrow cursor ? */

void createrulewindows (HWND hWnd, int x, int y) {
	int xw, yw;
	int xh, yh;
	int dx = 100;
	int dy = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);
	xw = x; yw = y - dy * 2 ;
	xh = x - dx - dy; yh = y;
	hWidth = CreateWindow (
		"STATIC",
		"",						/* "width" */
/*		WS_BORDER | WS_CHILD | SS_SIMPLE | WS_VISIBLE, */
		WS_BORDER | WS_CHILD | SS_LEFT | WS_VISIBLE, 
		xw, 
		yw,
		dx,
		dy,
		hWnd,
		(HMENU) IDR_WIDTH,
		hInst,
		NULL);
	hHeight = CreateWindow (
		"STATIC",
		"",						/* "height" */
/*		WS_BORDER | WS_CHILD | SS_SIMPLE | WS_VISIBLE, */
		WS_BORDER | WS_CHILD | SS_RIGHT | WS_VISIBLE, 
		xh, 
		yh,
		dx,
		dy,
		hWnd,
		(HMENU) IDR_HEIGHT,
		hInst,
		NULL);
	if (hWidth == NULL || hHeight == NULL) {
		 winerror ("Unable to create ruler captions");
			 return;
	}
/*	ShowWindow (hWidth, SW_SHOW); */
/*	ShowWindow (hHeight, SW_SHOW); */
}

void destroyrulewindows (void) {
	if (hWidth != NULL) DestroyWindow(hWidth);
	hWidth = NULL;
	if (hHeight != NULL) DestroyWindow(hHeight);
	hHeight = NULL;
}

/* compute vertical scroll position from yoffset, ClientRect, page size etc */
/* set scroll thumb accordingly */

int VScrollPos(HWND hWnd) {
	int cytop, cybot;
	int scrollpos;
	long pageh; 
	long snum, sden;
	RECT ClientRect;
	HDC hDC;

	GetClientRect(hWnd, &ClientRect);
	ClientRect.bottom--;
	hDC = GetDC(hWnd);
	if (bCopyFlag == 0)			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS); /* set unit twips */
	ClientRect = DPtoLPRect(hDC, ClientRect);
	(void) ReleaseDC(hWnd, hDC);
	cytop = ClientRect.top;
	cybot = ClientRect.bottom;

	if (bLandScape == 0) pageh = PageHeight;
	else pageh = PageWidth;
/*	pageh = (((((long) pageh) << 16) / magden) * magnum) >> 10; */
/*	Is this assuming standard dvi_num and dvi_den ??? */

	pageh = lmapd(MulDiv((long) pageh, 1000 * 65536, dvi_mag));

/*	pageh = lmapd(((long) pageh * 1000 / dvi_mag) * 65536); */

	snum = ((long) yoffset - cytop) * 100;
	sden = pageh - (cytop - cybot); 
	if (sden == 0) sden++;
	scrollpos = (int) (snum / sden);	
	if (scrollpos < 0) scrollpos = 0;
	else if (scrollpos > 100) scrollpos = 100;
	if (sden < 0) scrollpos = 100 - scrollpos;	/* flip sense */
	SetScrollPos(hWnd, SB_VERT, scrollpos, TRUE);
/*	if (bDebug > 1) {
		sprintf(debugstr, "cytop %d cybot %d pageh %ld yoffset %ld scroll %d\n",
			cytop, cybot, pageh, yoffset, scrollpos);
		OutputDebugString(debugstr);
	} */
	return scrollpos;
}

/* compute horizontal scroll position from xoffset, ClientRect, page size */
/* set scroll thumb accordingly */

int HScrollPos(HWND hWnd) {
	int cxleft, cxright;
	int scrollpos;
	long pagew;
	long snum, sden;
	RECT ClientRect;
	HDC hDC;

	GetClientRect(hWnd, &ClientRect);
	ClientRect.bottom--;
	hDC = GetDC(hWnd);
	if (bCopyFlag == 0)			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS); /* set unit twips */
	ClientRect = DPtoLPRect(hDC, ClientRect);
	(void) ReleaseDC(hWnd, hDC);
	cxleft = ClientRect.left;
	cxright = ClientRect.right;

	if (bLandScape == 0) pagew = PageWidth;
	else pagew = PageHeight;
/*	pagew = (((((long) pagew) << 16) / magden) * magnum) >> 10; */
/*	Is this assuming standard dvi_num and dvi_den ??? */

	pagew = lmapd(MulDiv((long) pagew, 65536 * 1000, dvi_mag));

/*	pagew = lmapd(((long) pagew * 1000 / dvi_mag) * 65536); */

	snum = ((long) cxleft - xoffset) * 100;
/*	sden = ((long) cxleft - cxright + pagew); */
	sden = pagew - (cxright - cxleft); 
	if (sden == 0) sden++;
	scrollpos = (int) (snum / sden);	
	if (scrollpos < 0) scrollpos = 0;
	else if (scrollpos > 100) scrollpos = 100;
	if (sden < 0) scrollpos = 100 - scrollpos;	/* flip sense */
	SetScrollPos(hWnd, SB_HORZ, scrollpos, TRUE);
/*	if (bDebug > 1) {
		sprintf(debugstr, "cxleft %d cxright %d pagew %ld xoffset %ld scroll %d\n",
			cxleft, cxright, pagew, xoffset, scrollpos);
		OutputDebugString(debugstr);
	} */
	return scrollpos;
}

/* inverse of the above, compute xoffset and yoffset given scroll positions */ 

long yoffsetscroll(HWND hWnd, int scroll) {
	int cytop, cybot;
	long pageh; 
	long sden;
	RECT ClientRect;
	HDC hDC;
	long offset;
	
	GetClientRect(hWnd, &ClientRect);
	ClientRect.bottom--;
	hDC = GetDC(hWnd);
	if (bCopyFlag == 0)			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS); /* set unit twips */
	ClientRect = DPtoLPRect(hDC, ClientRect);
	(void) ReleaseDC(hWnd, hDC);
	cytop = ClientRect.top;
	cybot = ClientRect.bottom;

	if (bLandScape == 0) pageh = PageHeight;
	else pageh = PageWidth;
/*	Is this assuming standard dvi_num and dvi_den ??? */
	pageh = lmapd(((long) pageh * 1000 / dvi_mag) * 65536);
	sden = pageh - (cytop - cybot); 
	if (sden < 0) scroll = 100 - scroll;
	offset = (long) cytop + sden * scroll / 100;
/*	if (bDebug > 1) {
		sprintf(debugstr, "cytop %d cybot %d pageh %d scroll %ld offset %d\n",
			cytop, cybot, pageh, scroll, offset);
		OutputDebugString(debugstr);
	} */
	return offset;
}

long xoffsetscroll(HWND hWnd, int scroll) {
	int cxleft, cxright;
	long pagew;
	long sden;
	RECT ClientRect;
	HDC hDC;
	long offset;

	GetClientRect(hWnd, &ClientRect);
	ClientRect.bottom--;
	hDC = GetDC(hWnd);
	if (bCopyFlag == 0)			/* this shouldn't ever be non-zero ... */
		(void) SetMapMode(hDC, MM_TWIPS); /* set unit twips */
	ClientRect = DPtoLPRect(hDC, ClientRect);
	(void) ReleaseDC(hWnd, hDC);
	cxleft = ClientRect.left;
	cxright = ClientRect.right;

	if (bLandScape == 0) pagew = PageWidth;
	else pagew = PageHeight;
/*	Is this assuming standard dvi_num and dvi_den ??? */
	pagew = lmapd(((long) pagew * 1000 / dvi_mag) * 65536);
	sden = pagew - (cxright - cxleft); 
	if (sden < 0) scroll = 100 - scroll;
	offset = (long) cxleft - sden * scroll / 100;
/*	if (bDebug > 1) {
		sprintf(debugstr, "cxleft %d cxright %d pagew %ld scroll %d offset %ld\n",
			cxleft, cxright, pagew, scroll, offset);
		OutputDebugString(debugstr);
	} */
	return offset;
}

int isinANSI (int c) {
	if ((c >= 32 && c <= 126) ||
		(c >= 130 && c <= 140) ||
		(c >= 145 && c <= 156) ||
		(c == 159) ||
		(c > 160)) return 1;
	else return 0;
}

void setupcharinfo (void) {			/* 98/Mar/26 */
	int k;
	int chr;
	unsigned int twipsize, twipwidth;
	int hundsize, intsize, fracsize;
	long ltagwidth;
	int hundwidth, intwidth, fracwidth;
	int intem, fracem;
	int tenthascent, intascent, fracascent;
	int tenthdescent, intdescent, fracdescent;
/*	char sencode[32]=""; */
	char *sencode="";
	char *fname="UNKNOWN";
	char *sfname="UNKNOWN";

	k = finx[taggedfont];
	ltagwidth = unmap(taggedwidth);			/* old */
	twipsize = mappoints(fs[k]);			/* new, in winfonts.c */
/*	hundsize = (twipsize + 1) / 2; */
/*	hundsize = twipsize / 2; */
	hundsize = twipsize;
	intsize = (int) (hundsize / 100);
	fracsize = (int) (hundsize - ((unsigned int) intsize) * 100);
/*	ltaggedwidth is in Adobe 1000 per em units */
/*	need to multiply by font at size to get actual width */
	twipwidth = (int) ((long) ltaggedwidth * twipsize / 1000);
/*	above is width in TWIPS * 10 */
/*	if (bDebug > 1) {
		sprintf(debugstr,
			"ltaggedwidth: %d, atsize: %ld, twipsize * 10: %d, twipwidth * 10: %d\n",
				ltaggedwidth, fs[k], twipsize, twipwidth);
		OutputDebugString(debugstr);
	} */ /* not needed now that we have screen output */
/*	hundwidth = (twipwidth + 1) / 2; */
/*	hundwidth = twipwidth / 2; */
	hundwidth = twipwidth;
	intwidth = (int) (hundwidth / 100);
	fracwidth = (int) (hundwidth - ((unsigned int) intwidth) * 100); 
	intem = ltaggedwidth / 1000;
	fracem = (int) ((long) ltaggedwidth - ((unsigned int) intem) * 1000);
/*	need to adjust ascent and descent for scaling ... */
	tenthascent = (fontascent[k] + 1) / 2;				/* twips */
	intascent = tenthascent / 10;
	fracascent = (tenthascent - intascent * 10);
	tenthdescent = (fontdescent[k] + 1) / 2;			/* twips */
	intdescent = tenthdescent / 10;
	fracdescent = (tenthdescent - intdescent * 10);
	chr = taggedchar;
	if (texfont[k]) {									/* a remapped font */
		if (chr >= 161 && chr <= 170) chr = chr - 161;
		else if (chr >= 173 && chr <= 195) chr = chr - 163;
		else if (chr == 196) chr = 127;
	}
	if (ansifont[k]) {									/* 95/Aug/10 */
#ifdef USEUNICODE
		if ((szReencodingName != NULL) &&				/* 98/Dec/25 */
			(fontttf[k] && bUseNewEncodeTT) ||
			(!fontttf[k] && bUseNewEncodeT1))
/*			sprintf(sencode, "(%s)", szReencodingName);	 */
			sencode = szReencodingName;
/*		else if (fontttf[k]) strcpy(sencode, "(ANSI)"); */
		else if (fontttf[k]) sencode ="ANSI";
#else
		
/* 		if (fontttf[k]) strcpy(sencode, "(ANSI)"); */
		if (fontttf[k]) sencode = "ANSI";
#endif
		else if ((szReencodingName != NULL) &&
				 bATMLoaded && hEncoding != NULL)
/*			sprintf(sencode, "(%s)", szReencodingName); */
			sencode = szReencodingName;
/*		else strcpy(sencode, "(ANSI)"); */
		else sencode ="ANSI";
	}

	if (fontname[k] != NULL) fname = fontname[k];
	if (subfontname[k] != NULL) sfname = subfontname[k];
	sprintf(str,
"Char:\t%d\t%c\nFont:\t%s\t(%d)\nFace:\t%s\nStyle:\t%s%s%s%s\t%s%s\n\
at:\t%d.%02d pt\nWidth:\t%d.%02d pt\t(%d.%03d em)",
			chr, isinANSI(chr) ? chr : ' ',
/*				fontname[k], taggedfont, subfontname[k],  */
				fname, taggedfont, sfname,
/*				texfont[k] ? "(remapped)" : "", ansifont[k] ? "(ANSI)" : "", */
				(!fontbold[k] && !fontitalic[k]) ? "Regular" : "",
					fontbold[k] ? "Bold" : "", fontitalic[k] ? "Italic" : "",
						fontttf[k] ? " (TT)" : "",
							texfont[k] ? "remapped" : "",
/*								ansifont[k] ? "(ANSI)" : "", */
								ansifont[k] ? sencode : "", 
/*				(ansifont[k] && bANSITeXFlag) ? "(ANSI)" : "", *//* 94/Dec/25 */
							intsize, fracsize, intwidth, fracwidth,
								intem, fracem);
}
			
/* Nice way to show very large numbers, with three digits comma separated */
/* char *commathousand(char *s, unsigned int n) */
char *commathousand (char *s, unsigned long n) {
/*	unsigned int quotient; */
	unsigned long quotient;
/*	unsigned int thr=1000000000; */
	unsigned long thr=1000000000;

	s += strlen(s);				/* get to end of string */
	if (n == 0) {
		*s++ = '0';
		*s = '\0';
		return s;
	}
	while (thr > 0) {
		if (n >= thr) break;
		thr = thr / 10;
	}
	while (thr > 0) {
		quotient = n / thr;
/*		*s++ = '0' + quotient; */
		*s++ = (char) ('0' + quotient);
		if (thr == 1000 || thr == 1000000 || thr == 1000000000)	*s++ = ',';
		n = n % thr;
		thr = thr / 10;
	}
	*s = '\0';
	return s;
}
/* #endif */

void ShowSystemInfoAux (char *str) {			/* show system info 95/Mar/18 */
	char *s;

	MEMORYSTATUS MemoryStatus;					/* 1995/Nov/2 */
	char *t;
	OSVERSIONINFO OSVersionInfo;				/* 1995/Nov/4 */

	MemoryStatus.dwLength = sizeof(MemoryStatus);
	GlobalMemoryStatus(&MemoryStatus);

/*	sprintf(str, "\
%d%% Memory Load (percent in use)\n\
%d  Total Physical \t%d  Avail Physical\n\
%d  Total Page File\t%d  Avail Page File\n\
%d  Total Virtual  \t%d  Avail Virtual\n", */
	s = str;
	*s = '\0';
	if (bWinNT == 0 || MemoryStatus.dwMemoryLoad > 0) {
		sprintf(s, "%d%%  Memory Load  (percent in use)\n\n",
			MemoryStatus.dwMemoryLoad);		 /* percent of memory in use */
	}
	s = commathousand(s,
			MemoryStatus.dwTotalPhys);  	 /* bytes of physical memory */
	sprintf(s, "  Total Physical \t");
	s = commathousand(s,
			MemoryStatus.dwAvailPhys);     	 /* free physical memory bytes */
	sprintf(s, "  Avail Physical \n");
	s = commathousand(s,
			MemoryStatus.dwTotalPageFile); 	 /* bytes of paging file */
	sprintf(s, "  Total Page File \t");
	s = commathousand(s,
			MemoryStatus.dwAvailPageFile); 	 /* free bytes of paging file */
	sprintf(s, "  Avail Page File \n");
	s = commathousand(s,
			MemoryStatus.dwTotalVirtual);  	 /* user bytes of address space */
	sprintf(s, "  Total Virtual \t");
	s = commathousand(s,
			MemoryStatus.dwAvailVirtual);  	 /* free user bytes 			 */
	sprintf(s, "  Avail Virtual \n\n");
	s += strlen(s);
	OSVersionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&OSVersionInfo);
	t = SetupVersionString(OSVersionInfo, 0);
//	switch (OSVersionInfo.dwPlatformId) {
//		case VER_PLATFORM_WIN32s:
//			t = "Win32s on Windows 3.1";
//			break;
//		case VER_PLATFORM_WIN32_WINDOWS:
//			if (OSVersionInfo.dwMinorVersion > 0) 
//				t = "Win32 on Windows 98";
//			else t = "Win32 on Windows 95";
//			break;
//		case VER_PLATFORM_WIN32_NT:
//			t = "Windows NT";
//			break;
//		default:
//			t = "Unknown";
//	}
/*	OSVersionInfo.dwMajorVersion, OSVersionInfo.dwMinorVersion */
/*	Windows NT SUR:  4  0 (can use features of new shell) */
/*	Windows NT 3.51: 3 51 (without new shell) */
	sprintf(s, "Windows  %d.%d\tBuild  %d\nPlatform ID:\t%s\n%s\t\t%s",
			OSVersionInfo.dwMajorVersion, 	OSVersionInfo.dwMinorVersion,
			LOWORD(OSVersionInfo.dwBuildNumber), t,
			(*OSVersionInfo.szCSDVersion != '\0') ? "CSD:" : "",
			OSVersionInfo.szCSDVersion);
/* CSD = Corrective Service Distribution ! */

/*	wininfo(str); */
}

void stripwhitespaceatend (char *str) {
	char *s=str + strlen(str) - 1;
	while (s >= str && *s <= ' ') *s-- = '\0';
}

void ShowSystemInfo (void) {				/* show system info 95/Mar/18 */
	ShowSystemInfoAux(str);
	if (bInfoToClip && GetAsyncKeyState(VK_SHIFT) < 0) {
		strcat(str, "\n");
		PutStringOnClipboard(hwnd, str);
		stripwhitespaceatend(str);
	}
	wininfo(str);
}

void ShowSysFlagsAux (char *str) {
	ShowConfiguration(str);
	strcat(str, "\n");					/* split in two */
	showscreensize(str + strlen(str));
	*(str + strlen(str) - 1) = '\0';	/* kill trailing \n */
}

void ShowSysFlags (void) {
	ShowSysFlagsAux (str);
	if (bInfoToClip && GetAsyncKeyState(VK_SHIFT) < 0) {
		strcat(str, "\n");
		PutStringOnClipboard(hwnd, str);
		stripwhitespaceatend(str);
	}
	wininfo(str);
}

/* Windows 3.11:	DOS 6.20 Windows 3.10 */
/* Windows NT 3.51: DOS 5.0  Windows 3.10 */
/* Windows 95 (16):	DOS 7.0  Windows 3.95 */
/* Windows 95 (32):			 Windows 4.00 */
/* Windows NT (32) SUR:		 Windows 4.00 */

int ZWidth, ZHeight;		/* width & height of screen rectangle */

/* WORD ZWidth, ZHeight; */
	
/* int bCursorMoved=0; */	/* to prevent SetCursorPos from triggering */
			
int mark;					/* moved out here for PostMessage 1993/Sep/1 */

/* #define DEBUGOPEN */

/* use lpstrCustomFilter & nMaxCustFilter ? DefSpec */

/* worry about OFN_SHAREAWARE ? */

#define FILEOPENORD      1536			/* in dlgs.h */

/* following is from c:\msvcnt\include\commdlg.h */

/* openflag == 1 => open the file in OpenName, openflag == 0 => use dialog */

HFILE GetDVIFile (HWND hWnd, int openflag) {	/* do hairy stuff here later */
	OPENFILENAME ofn;
/*	char szDirName[MAXFILENAME]; */
	char szFile[MAXFILENAME];
	char szFileTitle[MAXFILENAME];	/* on output file name minus path */
/*	char szFilter[MAXFILENAME]; */
	char szFilter[256];			/* 1994/Feb/10 */
	char szExt[16]; 
	UINT i, cbString;
	char chReplace;				/* string separator for szFilter */
/*	HFILE hFile = -1; */
	HFILE hFile = HFILE_ERROR;	/* shadows global hFile ? */
	int flag;
	char *s; 
#ifndef LONGNAMES
	OFSTRUCT OfStruct;		/* needed if openflag != 0 */
#endif

/*	removeback(DefPath); */				/* 1993/Dec/9 */
	ChangeDirectory(DefPath, 1);		/* change directory and drive */
/*	ChangeDirectory(DefPath, 0); */ 	/* 1996/Aug/26 trial */
/*	trailingback(DefPath); */			/* 1993/Dec/9 */

/*	strcpy(szDirName, OpenName);	
	if ((s = strrchr(szDirName, '\\')) != NULL) ;
	else if ((s = strrchr(szDirName, '/')) != NULL) ;
	else if ((s = strrchr(szDirName, ':')) != NULL) ;
	else s = szDirName + strlen(szDirName);
	*s = '\0'; */						/* copy directory from file name */
/*	strcpy(szDirName, DefPath); */		/* OR DO THIS */
/*	szFile[0] = '\0'; */
/*	strcpy(szFile, DefPath); */
/*	if (*(szFile + strlen(szFile) - 1) != '\\') strcat (szFile, "\\"); */
/*	strcat(szFile, strippath(OpenName)); */	/* copy file name */
	strcpy(szFile, removepath(OpenName)); 	/* copy file name */
/*	Now set up file type selection string */
	if ((cbString = LoadString(hInst, IDS_DVIFILTERSTR, szFilter,
		sizeof(szFilter))) == 0) {
		strcpy(szFilter, "DVI Files(*.dvi)|*.dvi|");	/* or use this */
/*		strcpy(szFilter, DefSpec); */		/* or use this ? */
	}
	chReplace = szFilter[cbString - 1];	/* retrieve wild character */
	for (i = 0; szFilter[i] != '\0'; i++) 
		if (szFilter[i] == chReplace)	szFilter[i] = '\0';

//	if ((s = strchr(DefExt, '.')) != NULL)	strcpy(szExt, s+1); 
	if (*DefExt == '.')	strcpy(szExt, DefExt+1); 
	else strcpy(szExt, "dvi");

	if (openflag != 0) { 	/* if asked to just open the file *//* 93/Dec/25 */
#ifdef LONGNAMES
		if (LongOpenFile(szFile, OF_EXIST) != HFILE_ERROR) {
#else
		if (OpenFile(szFile, &OfStruct, OF_EXIST) != HFILE_ERROR) {
#endif
/*			if ((hFile = DoOpenFile(hWnd)) != -1) */		/* 95/Oct/25 fix */
			if ((hFile = DoOpenFile(hWnd)) != HFILE_ERROR)	/* 99/Feb/28 */
				return hFile;							/* and if it exists */
/*			otherwise drop through and show dialog */
		}
	}

/* Some problem here with GetOpenFileName in Windows NT 3.51 ??? */

	memset (&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = szFilter;
/*	ofn.nFilterIndex = 1; */
	ofn.nFilterIndex = DVIFilterIndex;
	ofn.lpstrFile = szFile;				/* have enough space for long name */
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrTitle = "Open File";
/* 	ofn.lpstrInitialDir = szDirName; */
	ofn.lpstrInitialDir = DefPath;		/* direct approach */
/*	ofn.lpstrDefExt = szExt; */			/* default extension */
/*	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; */
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

/*	Use our own template with longer file list */
/*	except in new shell in Windows 95 where we have long file names */

	if (bHiresScreen && (bNewShell == 0)) {

/*	if (bHiresScreen) */

		ofn.hInstance = hInst;
		ofn.Flags = ofn.Flags | OFN_ENABLETEMPLATE;
		ofn.lpTemplateName = MAKEINTRESOURCE(FILEOPENORD);
	}

#ifdef LONGNAMES
	if (bLongNames) {		/* has no effect with Explorer style dialog ... */
		ofn.Flags = ofn.Flags | OFN_LONGNAMES; /* 95/Dec/5 */
/*		if (bDebug > 1) OutputDebugString("OFN_LONGNAMES\n"); */
	}
#endif

/*	if (bDebug > 1) OutputDebugString("Before GetOpenFileName\n"); */
	flag = GetOpenFileName(&ofn);			/* DWINVER=0x300 problem ? */
/*	if (bDebug > 1) OutputDebugString("After GetOpenFileName\n"); */

	if (flag != 0) {						/* user selected file to open */
		DVIFilterIndex = ofn.nFilterIndex;		/* remember filter index */

/*		ParseFileName();		*/	/*  Split OpenName into DefPath, DefSpec, DefExt */
/*		hFile = lopen(ofn.lpstrFile, OF_READ); */
/*		lstrcpy (OpenName, ofn.lpstrFile); */
/*		lstrcpy (OpenName, ofn.lpstrFileTitle); */	/* filename minus path */
#ifdef DEBUGOPEN
		if (bDebug > 1) {
			sprintf (debugstr, "lpstrFile '%s' nFileOffset %d nFileExtension %d",
						  ofn.lpstrFile, ofn.nFileOffset, ofn.nFileExtension);
			OutputDebugString(debugstr);
//			wincancel(debugstr);	// debugging only
		}
#endif
/*		In most cases the following is redundant *//* 1994/Aug/12 */
/*		but if the user types in explicit absolute file name we need it */
/*		or if user types in explicit relative file name ..\foo.dvi we need it */
/*		since OpenFile uses OpenName, not full ofn.nFileOffset */
		NewChangeDirectory(ofn.lpstrFile, ofn.nFileOffset);	/* 1994/Aug/12 */
		lstrcpy (OpenName, ofn.lpstrFile + ofn.nFileOffset);
/*		retrieve extension for future reference ".dvi" */
		if (ofn.nFileExtension > 0) {
			DefExt[0] = '.';
			lstrcpy (DefExt+1, ofn.lpstrFile + ofn.nFileExtension);
/*			retrieve default file specification for future reference "*.dvi" */
			DefSpec[0] = '*';
			DefSpec[1] = '.';
			lstrcpy (DefSpec+2, ofn.lpstrFile + ofn.nFileExtension);
		}
		else {										/* 98/Sep/14 */
			DefExt[0] = '\0';
			DefSpec[0] = '*';
			DefSpec[1] = '\0';
		}
/*		retrieve path for future reference */
		lstrcpy(DefPath, ofn.lpstrFile);
		*(DefPath + ofn.nFileOffset) = '\0';
//		but what if this is a network path ???
#ifdef DEBUGOPEN
		if (bDebug > 1) {
			sprintf(debugstr, "OpenName '%s' DefExt '%s', DefSpec '%s', DefPath '%s'\n",
				OpenName, DefExt, DefSpec, DefPath);
			OutputDebugString(debugstr);
//			wincancel(debugstr);	// debugging only
		}
#endif
		hFile = DoOpenFile(hWnd);
/*	Try and deal with case where user picked file with wrong extension */
		if (hFile != HFILE_ERROR) {	/* 1999/Feb/28 check it is a DVI file */
/*	sanity check - is it a DVI file ? */
			(void) _lread(hFile, buffer, 2);			/* get pre */
			(void) _llseek(hFile, 0, SEEK_SET);			/* rewind again */
			if (*buffer != pre) {
				if ((s = strrchr(OpenName, '.')) != NULL) {
					if (_strnicmp(s+1, "dvi", 3) != 0) {	/* if NOT dvi */
						strcpy(s+1, "dvi");				/* change extension */
						_lclose(hFile);
						hFile = HFILE_ERROR;
						hFile = DoOpenFile(hWnd);		/* now try again */
						if (hFile != HFILE_ERROR) {
							if (bDebug > 1) OutputDebugString("Switched to DVI");
							strcpy(DefExt, ".dvi");
							strcpy(DefSpec, "*.dvi");
						}
					}
				}
			}
		}
	} /* end of - user selected a file */
	return hFile;
}

/* We assume the file name is already in OpenName */
/* we are passed the dvipage number */
/* currently only used by hypertext `push' (jump) or `pop' (restore) */

/* old DVI file name in FileName (not used) - new DVI file name in OpenName */

int SwitchDVIFile (HWND hWnd, int n, int paintflag) {
	HDC hDC;
	int flag=0;
	int hyperindexsvd;

	hyperindexsvd = hyperindex;
	if (bFileValid >= 0) closeup(hWnd, 0); 
	bFileValid = 0;
	hyperindex = hyperindexsvd;

	InvalidateRect(hWnd, NULL, TRUE);  /* ??? */
/*	strcpy(OpenName, s); */
/*	AddExt(OpenName, ".dvi"); */
	bReadFile = 1;		/* ??? */
/*	dvipage = 1;  */
	dvipage = n;
	hyperindexsvd = hyperindex;
	SetupFile(hwnd, 0);						/* Uses OpenName */
	hyperindex = hyperindexsvd;
/*	bFileValid = 1;	*/		/* experiment ??? */
/*	InvalidateRect(hWnd, NULL, TRUE); */ /* ??? */
	if (paintflag) {
		hDC = GetDC(hwnd);
		flag = paintpage(hwnd, hDC);
		(void) ReleaseDC(hwnd, hDC);
		if (flag < 0) {		/* should not happen, because we pretest */
			sprintf(str, "Failed to open %s", OpenName);
			winerror (str);
		}
	}
/*	if (bFileValid == 0) {
		if (bDebug > 1)
			OutputDebugString("bFileValid == 0\n");
	} */
/*	InvalidateRect(hWnd, NULL, TRUE); */
/*	UpdateWindow(hwnd);	*/ /* Send WM_PAINT */
	return flag;
}

// EXE_ERROR strings no longer exist...

/* scom at start of `launch:', s at start of command, samp at next - if any */
/* convert from / to \ ? */
/* removes launch: strings from string after it gets done 98/Dec/2 */

char *hyperlaunch (char *scom) {
	char szBuffer[FILENAME_MAX+FILENAME_MAX];
	char *s, *t, *samp;
	int flag;
	HINSTANCE err;
	int nFile, nTail;

	s = scom+6;							/* step over "launch" / "execute" */
	while (*s != ':' && *s > 0) s++;	/* find : */
	s++;
	while (*s <= ' ' && *s > 0) s++;	/* step over white space */
/*	comma is delimiter between hyper-text commands - replace with null */
	if ((samp = strchr(s, ',')) != NULL) {
		if (*(samp-1) <= ' ') *(samp-1) = '\0';
		*samp++ = '\0';					/* obliterate , */
		while (*samp <= ' ' && *samp > 0) samp++;
	}
	else samp = s + strlen(s);			/* no more items after this */
	strcpy(szBuffer, s);
	while ((t = strchr(szBuffer, '/')) != NULL) *t = '\\'; /* 98/Dec/4 */
	if (bDebug || bShowCalls) {
		flag = MaybeShowStr(szBuffer, "Launch");
/*		Could cancel this if flag == 0 ? */ /* 96/Sep/15 */
	}
/*	Do something more elaborate here ? Like TryWinExe ? Current Dir ? */
//	WritePrivateProfileString(achPr, "LastWinExe", szBuffer, achFile);
	WritePrivateProfileString(achDiag, "LastWinExe", szBuffer, achFile);
	if (bDebug > 1) OutputDebugString(szBuffer);
	err = (HINSTANCE) WinExec(szBuffer, SW_SHOW);
/*	NOTE: WinExec returns task handle of spawned job - or error code <= 32 */

	if (err < HINSTANCE_ERROR) {		/* new 98/Dec/4 */
/*		It was not a known application - see if registered file type */
		if (FindApplication(s, szBuffer, sizeof(szBuffer)) > 0) {
/*			replace argument one with file name given */
			if ((t = strstr(szBuffer, "%1")) != NULL) {
				nTail = strlen(t+2);
				nFile = strlen(s);
				memmove (t+nFile, t+2, nTail+1);
				memcpy (t, s, nFile);
			}
			else {		/* no explicit %1 - just tag file name on end */
				strcat(szBuffer, " ");
				strcat(szBuffer, s);
			}
			if (bDebug || bShowCalls) {
				flag = MaybeShowStr(szBuffer, "Launch");
/*				Could cancel this if flag == 0 ? */ /* 96/Sep/15 */
			}
			if (bDebug > 1) OutputDebugString(szBuffer);
//			WritePrivateProfileString(achPr, "LastWinExe", szBuffer, achFile);
			WritePrivateProfileString(achDiag, "LastWinExe", szBuffer, achFile);
			err = (HINSTANCE) WinExec(szBuffer, SW_SHOW);
/*	NOTE: WinExec returns task handle of spawned job - or error code <= 32 */
		}
	}

	if (err >= HINSTANCE_ERROR) {
/*	execute means launch and then *wait* for it */
		if (strncmp(scom, "execute:", 8) == 0) {
			(void) GetTaskWindow ((HINSTANCE) err);
		}
	}
	else {					/* WinExec failed - set up message */
		strcpy(debugstr, szBuffer);

		ExplainError(debugstr, GetLastError(), 1);

#ifdef IGNORED
		strcat(debugstr, "\n");
		s = debugstr + strlen(debugstr);
/*									n = strlen(debugstr); */
		flag = LoadString(hInst,
/*										(UINT) (EXE_ERROR + err), */
						  (EXE_ERROR + (UINT) err),
/*											debugstr+n, BUFLEN-n); */
						  s, sizeof(debugstr) - strlen(debugstr));
/*									if (flag == 0) sprintf(debugstr+n, */
		if (flag == 0) sprintf(s, "Error %d ", err); 
		strcat(debugstr, " WinExec");
		winerror(debugstr);
#endif
	}
/* move to end if resetflag == 1 95/Aug/12 ? don't do if also mark or file: */
/*								bBusyFlag = 0; */
/*								bSearchFlag = 0; */
/* remove command that we already now dealt with - possibly leave nothing */
	strcpy(scom, samp); /* strip out launch: ... */
	return 0;
}

int GetNewWorkDir (HWND hWnd) {
	int flag=0;

	*str = '\0';
	if (IniDefPath != NULL) strcpy(str, IniDefPath);
	flag = DialogBoxParam(hInst, "FileSelect", hWnd, FileDlg, 3L);
							/* indicate  `Working Directory' mode */
//	flag == 0 if user cancelled dialog
	if (flag) {
//		Use file name that was put in `szHyperFile' by FileDlg 
//		Dialog puts name temporarily in szHyperFile - need to make copy now 
		if (CheckWorking(str)) {
			if (IniDefPath != NULL) free(IniDefPath);
			IniDefPath = zstrdup(str);
			bUseWorkPath = 1;
			bUseSourcePath = 0; 
			bWorkingExists = 1;
		}
		else {
			winerror("BAD DIRECTORY");	// debugging
			flag = 0;
		}
	}
	else winerror("USER CANCELLED");	// debugging
	return flag;
}

/****************************************************************************

	FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)  OR NOW:
	FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

	PURPOSE:  Processes messages

	MESSAGES:
		WM_COMMAND		- application menu
		WM_PAINT		- repaint window
		WM_SIZE			- window size has changed
		WM_DESTROY		- destroy window
		WM_QUIT			- quit from program
		WM_QUERYENDSESSION - willing to end session?
		WM_ENDSESSION	- end Windows session
		WM_CLOSE		- close the window

****************************************************************************/

/* Note that in WIN32, message and wParam have been expanded to 32 bit */

LRESULT APIENTRY MainWndProc(HWND hWnd, UINT message,
				WPARAM wParam, LPARAM lParam) {	

/* DLGPROC lpProcAbout, lpOpenDlg, lpPageDlg, lpSearchDlg; */

	PAINTSTRUCT ps;
	HDC hDC;					/* display-context variable */
	RECT ClipRect; 				/* current clipping region in WM_PAINT */
	RECT ZoomRect;				/* rectangle to zoom to */
	RECT ClientRect;			/* rectangle of client region */
	RECT WinRect;				/* window rectangle - WM_MOVE or WM_RESIZE */
	HMENU hMenu;
	HWND hFocus;
	POINT CurPoint; 			/* Cursor Position while dragging */
	int dx, dy, fwKeys, zDelta;
	int curx, cury;
/*	int mark; */
/*	long ybottom, yll; */
	int newpage, olddvipage;
	int stepsize, flag;
	int resetflag;
	int zoomflag, rectflag, ruleflag;
	BOOL bFileValidOld;
	WORD id, cmd;
	HDROP hDrop;
	int newpapertype;
	int ShiftState, ControlState;
/*	int nPos; */				/* current scroll bar position */
	BOOL bSpaceFlag;			/* space or backspace on keyboard */
	int RWidth, RHeight;		/* width & height of zoom rectangle */
	int nVKey;					/* virtual key code */
	DWORD dwKeyData;			/* repeat, scan code, etc */
	int nIndex, nMenu;			/* 1993/Dec/5 */
	char *s;					/* 1994/Dec/20 */
	char *scom, *samp;			/* 1995/Aug/12 */
	int n, m, k;				/* 1994/Dec/20 */
	FINDREPLACE FAR * lpfr;		/* 1994/Jan/7 */
/*	added new 1995/Nov/2 --- to handle messages from scroll bar */
	int nScrollCode;			/* scroll bar value */
	short int nPos;				/* scroll box position */
	HWND hwndScrollBar;			/* handle of scroll bar */
#ifdef LONGNAMES
	HFILE hFileTemp;			/* 95/Dec/6 and 95/Dec/18 */
#endif
	POINTS pts;
#ifdef USEPALETTE
	HPALETTE hOldPal;	/* 96/March/24 */
#endif
	int xPos, yPos;				/* 97/Jul/25 */
	int fwSizeType, nWidth, nHeight;		/* 97/Jul/25 */

/*	check first whether message for modeless search dialog box */
	if (uFindReplaceMsg != 0 && message == uFindReplaceMsg) {
		lpfr = (FINDREPLACE FAR *) lParam;	/* 1994/Jan/7 */
		if (lpfr->Flags & FR_DIALOGTERM) {	/* Cancel button or close */
			uFindReplaceMsg = 0;			/* invalidate, user is closing */
			hFindBox = NULL;				/* remove Dialog Box pointer */
			bShowSearchExposed = 0;
			bShowSearchFlag = 0;
			return 0;
		}
/*		other possible message: FR_FINDNEXT - we don't bother to check */
/*		we misuse `Match Whole Word' check box for `Wrap' */
		if (lpfr->Flags & FR_WHOLEWORD) bWrapFlag = 1;
		else bWrapFlag = 0;
		if (lpfr->Flags & FR_MATCHCASE) bCaseSensitive = 1;
		else bCaseSensitive = 0;
/*		bDismissFlag already read out in dialog box (IDOK) */
/*		check whether user wants the box to go away now 96/Aug/14 */
		if (bDismissFlag)
			PostMessage(hFindBox, WM_COMMAND, (WPARAM) IDABORT, 0L); 
		goto search;
	}


	switch (message) {
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id)
			{
				case IDM_ABOUT:
					if (bInfoToClip && GetAsyncKeyState(VK_SHIFT) < 0) 
						ShowAppAndSys(hWnd);
/*					(void) DialogBox(hInst, "AboutBox", hWnd, lpProcAbout); */
/*					(void) DialogBox(hInst, "AboutBox", hWnd, (DLGPROC) About); */
					(void) DialogBox(hInst, "AboutBox", hWnd, About);
					break;

				case IDM_DELETE:
					if (bFileValid == 0) break;			/* 1995/Mar/1 */

				case IDM_CLOSE:
					bAddToMRU = 1;			/* set back to default ? */
#ifdef DEBUGMAIN
					if (bDebug > 1)	OutputDebugString("IDM_CLOSE\n"); 
#endif
					if (bCharacterSample) {
						bCharacterSample = 0; /* AND THEN ? */
						InvalidateRect(hWnd, NULL, TRUE);
						break;
//						goto fontcancel;	/* ??? */
					}
					if (bFontSample)
						goto fontcancel;	/* ??? */
					closeup(hWnd, 1);
#ifdef HYPERFILE
					free_hyperfile();		/* right place ??? winsearc.c */	
#endif
					bFontSample = 0;		/* ??? */
					bCharacterSample = 0;		/* ??? */
					colorindex = 0;			/* 97/Feb/18 */
					checkcontrols(hWnd, 0);
					InvalidateRect(hWnd, NULL, TRUE); 	/* MOVED ? */
					break;

				case IDM_INSERT:			/* 98/Jul/9 */
					bAddToMRU = 1;			/* set back to default ? */
					bShiftFlag = (GetAsyncKeyState(VK_SHIFT) < 0);
					bControlFlag = (GetAsyncKeyState(VK_CONTROL) < 0);
					if (bShiftFlag == 0) {	/* open first document in MRU */
						flag = calldocument(0, 0, 0);
						bAddToMRU = 0;		/* bring to front of MRU ? */
/*						changed 99/Apr/23 */
						if (flag < 0) goto idmopen;
						bAddToMRU = 1;		/* set back to default ? */
						break;				/* no MRU entry ? */
					}		/* without shift key, just drop through */
/* NOTE: Shift Insert is same as File > Open from the menu */

/* Can come here from File > Open menu (with lParam = 0L) or */
/* from PostMessage with lParam giving the FileName 98/Dec/12 */

				case IDM_OPEN:
					flag = 0;			/* indicate use FileOpen dialog */
					bAddToMRU = 1;		/* set back to default ? */
					if (bDebug > 1) {
						sprintf(debugstr, "wParam %04X lParam %04X",
								wParam, lParam);
						OutputDebugString(debugstr);
					}

					if (lParam != 0L) {		/* from PostMessage */
/*	open without OpenFile dialog PostMessage from DDE call */
/*	do nothing if DVI file open and its the same file */
/*						SetFocus(hWnd); */	/* 98/Dec/18 ??? */
						SetFocus(hWnd);		/* 99/Apr/23 ??? */
						if (szSource != NULL) free(szSource);
						szSource = NULL;
						nSourceLine = 0;
						if (bFileValid != 0 &&
							IsCurrentFile((LPSTR) lParam)) {
							if (bDebug > 1) OutputDebugString("Same File");
							break;			/* don't do anything */
						}
						strcpy(OpenName, (LPSTR) lParam);
						if (bDebug > 1) {
							sprintf(debugstr, "OpenName %s from lParam",
									OpenName);
							OutputDebugString(debugstr);
						}
/* flush quotes if any */						
						if (*OpenName == '\"') strcpy(OpenName, OpenName+1);
						s = OpenName + strlen(OpenName);
						if ((s > OpenName) && (*(s-1) == '\"')) *(s-1) = '\0';
						if (bDebug > 1) OutputDebugString(OpenName);
						ParseFileName();		/* ??? */
						strcpy(DefSpec, "*");		/* ??? */
						strcat(DefSpec, DefExt);	/* fix up DefSpec */
						flag = -1;		/* don't use FileOpen dialog */
/*						bAddToMRU = 0; */	/* don't add to MRU ??? */
/*						changed 99/Apr/23 */						
					}
					
/*				case IDM_OPEN: */
/*					flag = 0; */		/* don't just open file */
				idmopen:				/* 1993/Dec/ 6 */
#ifdef DEBUGMAIN
					if (bDebug > 1)	OutputDebugString("IDM_OPEN\n");
#endif
					if (bFontSample) EndFontState(hWnd);
					bFileValid = 0;		/* new */
					SetFontState(0);	/* remove check marks if any */
					checkfontmenu();
					if (bResetScale != 0) resetpagemapping(hWnd);/* new here */
					closeup(hWnd, 1);	/* reset things - remove old info */
#ifdef HYPERFILE
					free_hyperfile();		/* right place ??? winsearc.c */	
#endif
					InvalidateRect(hWnd, NULL, TRUE); 	/* => erase */
					if (bAllowCommDlg && bUsingCommDlg) {	/* the new way */
						hFile = GetDVIFile (hWnd, flag);
/*						remember to also open file => hFile */
					}
					else {		/* use our own dialog box - ugh! flush */
/*						Open the file and get its handle */
/*						hFile = DialogBox(hInst, "Open", hWnd, lpOpenDlg); */
/*						hFile = DialogBox(hInst, "Open", hWnd, (DLGPROC) OpenDlg); */
						hFile = DialogBox(hInst, "Open", hWnd, OpenDlg); 
					}
					if (hFile == -1)	{
						break;				 /* ??? */
/*						return (NULL);	*/	 /* we loose */
					}

/*					if (bRememberDocs) */
					if (bRememberDocs && bAddToMRU) {
						if (bKeepSorted) RemoveDocument(DefPath, OpenName);
						AddDocument(DefPath, OpenName); /* in winsearc.c */
					}

/*					if (bResetScale != 0) resetpagemapping(hWnd); */

					bNeedANSIacce = 0;			/* 1993/Dec/21 */
					bNeedANSIwarn = 0;			/* 1993/Dec/21 */
					bBadFileComplained=0;		/* 1995/Mar/28 */
					bBadSpecialComplained=0;	/* 1996/Feb/4 */
					bUnitsWarned=0;				/* 1995/Aug/14 */
					bZeroWarned=0;				/* 1997/Mar/7 */
/*					BBxll = BByll = BBxur = BByur = 0; */	/* 96/Sep/29 */

					dvipage = 1;
					gopage = 0;					/* 1993/Nov/10 */
					bPageNumber = 0;			/* 1994/Mar/22 */
					flag = DoPreScan(hWnd, hFile);
					if (bKeepFileOpen == 0) {
/*						if (hFile < 0) wincancel("File already closed"); */
						if (hFile == HFILE_ERROR) wincancel("File already closed");
/*						if(_lclose(hFile) != 0)	winerror("Unable to close file"); */
						else _lclose(hFile);		/* close again */
/*						hFile = -1; */				/* close again */
						hFile = HFILE_ERROR;
					}
					else { /* 1992/Apr/26 */
/*						if (hFile < 0) wincancel("File already closed"); */
						if (hFile == HFILE_ERROR) wincancel("File already closed");
/*						if(_lclose(hFile) != 0)	winerror("Unable to close file"); */
						else _lclose(hFile);		/* close again */
/*						hFile = -1; */				/* close again */
						hFile = HFILE_ERROR;
						closeup(hWnd, 1);			/* safe ??? */
						break;
					}
/*					dvipage = 1; */
					usepagetable(dvipage, 0);		/* page is physical page */
					bEnableTermination = 1;		/* NEW */
					colorindex = 0;				/* 97/Feb/18 */
					graypreviousnext(hWnd);
					InvalidateRect(hWnd, NULL, TRUE);  /* => WM_PAINT */
					if (bUseTimer != 0 && bTimerOn == 0) TurnTimerOn(hwnd);
/*					graypreviousnext(hWnd); */
					break;

/*	don't really need to make a copy, but maybe in the future... */
				case IDM_SOURCEFILE:			/* 98/Dec/12 */
					if (szSource != NULL) free(szSource);
					szSource = zstrdup((LPSTR) lParam);
					if (bDebug > 1) OutputDebugString(szSource);
					OutputDebugString(szSource);
					break;

				case IDM_SOURCELINE:			/* 98/Dec/12 */
					nSourceLine = (UINT) lParam;
					if (szSource != NULL && nSourceLine > 0) {
						if (*szSource == '\"') strcpy(szSource, szSource+1);
						s = szSource + strlen(szSource);
						if (s > szSource && *(s-1) == '\"') *(s-1) = '\0';
						if (bDebug > 1) {
							sprintf (debugstr, "SOURCE SEARCH `%s' %u",
									 szSource, nSourceLine);
							OutputDebugString(debugstr);
						}
/*	searchandmark flag = 2 */ /* go back to start first ? */
						lastsearch = -1;			/* ? */
						dvipage = 1;				/* ? */
						usepagetable(dvipage, 0);	/* ? */
/*	copy search: for bSearchFlag++ == 0 protection ? */
						if (searchandmark(hWnd, 2) != 0) {/* search & cursor */
							bBusyFlag = 0;
							bSearchFlag = 0;
							if (bDebug > 1)
								OutputDebugString("Searched and Marked\n");
							graypreviousnext(hWnd);
							bEnableTermination = 1;		/* NEW */
							InvalidateRect(hWnd, NULL, TRUE);
						}
						else {
							bBusyFlag = 0;
							bSearchFlag = 0;
							if (bDebug > 1)
								OutputDebugString("No refresh needed\n");
						}
					}
					break;

				case IDM_SCROLLLEFT:
					nPos = 0;				/* avoid uninitialized ? */
					if (GetAsyncKeyState(VK_CONTROL) < 0) {
						goto hcenter;
					}
/*					wParam = SB_LINEUP; */
					nScrollCode = SB_LINEUP;		 /*  scroll bar value */
					goto hscroll;

				case IDM_SCROLLRIGHT:
					nPos = 0;				/* avoid uninitialized ? */
				hcenter:
					if (GetAsyncKeyState(VK_CONTROL) < 0) {
						nPos = 50;
						(void) SetScrollPos(hWnd, SB_HORZ, nPos, TRUE); 
						xoffsetold = xoffset;
						xoffset = xoffsetscroll(hwnd, nPos);	/* ??? */
						scrolladjust(hWnd, xoffset - xoffsetold, 0);
						break;
					}
/*					wParam = SB_LINEDOWN; */
					nScrollCode = SB_LINEDOWN;		 /*  scroll bar value */
					goto hscroll;
				
				case IDM_SCROLLUP:
					nPos = 0;				/* avoid uninitialized ? */
					if (GetAsyncKeyState(VK_CONTROL) < 0) {
						goto vcenter;
					}
/*					wParam = SB_LINEUP; */
					nScrollCode = SB_LINEUP;		 /*  scroll bar value */
					goto vscroll;
				
				case IDM_SCROLLDOWN:
					nPos = 0;				/* avoid uninitialized ? */
				vcenter:
					if (GetAsyncKeyState(VK_CONTROL) < 0) {
						nPos = 50;
						SetScrollPos(hwnd, SB_VERT, nPos, TRUE);
						yoffsetold = yoffset;
						yoffset = yoffsetscroll(hwnd, nPos);	/* ??? */
						scrolladjust(hWnd, 0, yoffset - yoffsetold);
						break;
					}
/*					wParam = SB_LINEDOWN; */
					nScrollCode = SB_LINEDOWN;		 /*  scroll bar value */
					goto vscroll;

				case IDM_HOME:		/* first page */
					if (bAltHomeEnd) {
						if (GetAsyncKeyState(VK_CONTROL) >= 0) {
							(void) PostMessage(hwnd, WM_COMMAND, IDM_TOP, 0L);
							break;
						}
					}
				case IDM_FIRSTPAGE:
/*	check whether control key pressed */
					if (dvipage == 1) {
						if (bResetPage != 0) {
/*							if (xoffset != pagexoffset ||
								yoffset != pageyoffset ||
								wantedzoom != pagewantedzoom) */
							if (!sameoffsetandzoom()) {
/*								if (bDebug > 1)
									OutputDebugString("Home -> Top\n"); */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details */
								(void) PostMessage(hwnd, WM_COMMAND, 
									IDM_TOP, 0L);  /* 92/05/09 */
							}
						}
						if (bUserAbort != 0) {	/* 1995/Dec/10 */
							bUserAbort = 0;
							InvalidateRect(hWnd, NULL, TRUE); /* ??? */
						}
						break;		/* nothing to do */
					}
/* get here if we are not on first page already */
					bUserAbort = 0;					/* 1995/Dec/10 */
					lastsearch = -1;	/* 1992/Aug/9 */
					dvipage = 1;
					usepagetable(dvipage, 0);		/* page is physical page */
					if (bSpreadFlag) {
						if (leftcurrent < 0 && rightcurrent < 0) {
							if (dvipage >= 1) dvipage--;	/* < dvi_t ??? */
/*										else if (dvipage > 0) dvipage--; */
							usepagetable(dvipage, 0);	/* page is physical page */
						}
					}
					selectpage(hWnd);
					graypreviousnext(hWnd);
					lastsearch = -1;			/* 1992/June/24 ??? */
					colorindex = 0;				/* 97/Feb/18 */
					bEnableTermination = 1;		/* NEW */
					InvalidateRect(hWnd, NULL, TRUE);
/*					graypreviousnext(hWnd); */
					break;

				case IDM_END:		/* last page */
					if (bAltHomeEnd) {
/*		check whether control key pressed */
						if (GetAsyncKeyState(VK_CONTROL) >= 0) {
							(void) PostMessage(hwnd, WM_COMMAND, IDM_BOTTOM, 0L);
							break;
						}
					}

				case IDM_LASTPAGE:
					if (dvipage >= dvi_t) {		/* if on last page already */
						if (bResetPage != 0) { 
/*							if (bDebug > 1)	OutputDebugString("End -> Bottom\n"); */
							(void) PostMessage(hwnd, WM_COMMAND,
								IDM_BOTTOM, 0L);		/* 92/02/03 */
						}
						if (bUserAbort != 0) {		/* 1995/Dec/10 */
							bUserAbort = 0;
							InvalidateRect(hWnd, NULL, TRUE); /* ??? */
						}
						break;
					}
/* get here if we are not on last page already */
					bUserAbort = 0;				/* 1995/Dec/10 */
					lastsearch = -1;			/* 1992/Aug/9 */
					dvipage = dvi_t;
					usepagetable(dvipage, 0);	/* page is physical page */
					if (bSpreadFlag) {
						if (leftcurrent < 0 && rightcurrent < 0) {
							if (dvipage <= dvi_t) dvipage++;	/* < dvi_t ??? */
/*										else if (dvipage > 0) dvipage--; */
							usepagetable(dvipage, 0);	/* page is physical page */
						}
					}
					selectpage(hWnd);
					graypreviousnext(hWnd);
					lastsearch = -1;			/* 1992/June/24 ??? */
					bEnableTermination = 1;		/* NEW */
					InvalidateRect(hWnd, NULL, TRUE);
/*					graypreviousnext(hWnd); */
					colorindex = 0;				/* 97/Feb/18 */
					break;

				case IDM_SELECTPAGE:
					olddvipage = dvipage;
					if (lParam != 0) newpage = (UINT) lParam;	/* 1999/Jan/10 */
					else {
/*					newpage = DialogBox(hInst, "PageSelect", hWnd, lpPageDlg); */
/*					newpage = DialogBox(hInst, "PageSelect", hWnd, (DLGPROC) PageDlg); */
						newpage = DialogBox(hInst, "PageSelect", hWnd,  PageDlg);
						if (newpage == -INFINITY) break;  /* user cancelled! */
					}

			selectcommon:							/* 1993/Nov/10 */

					lastsearch = -1;
					colorindex = 0;
					if (bCountZero == 0) {			/* using dvi pages */
						if (newpage == dvipage) break;	/* nothing to do */
						if (newpage < 1) {
							sprintf(str, "Page %d before begin", newpage);
							wininfo(str);
							break;
						}
						else if (newpage > dvi_t) {
							sprintf(str, "Page %d past end at %d",
								newpage, dvi_t);
							wininfo(str);
							break;
						}
						else dvipage = newpage;
						usepagetable(dvipage, 0);		/* page is physical page */
					}
					else {				/* bCountZero non-zero */
						usepagetable((int) newpage, +1); /* logical, forward */
						if (leftcurrent < 0 && rightcurrent < 0
							&& dvipage <= dvi_t) {	/* < dvi_t ??? */
							dvipage++;
							usepagetable(dvipage, 0);	/* page is physical page */
						}
						if (dvipage == olddvipage) break;
					}
/*			selectpage:	*/	/* seems like a legitimate page */
					selectpage(hWnd);
					graypreviousnext(hWnd);
					bEnableTermination = 1;		/* NEW */
					InvalidateRect(hWnd, NULL, TRUE);
/*					graypreviousnext(hWnd); */
					break;

				case IDM_RESEARCH:
					if (bFileValid == 0) break;
					if (*searchtext != '\0') {
						if (bSearchFlag != 0) {
							if (bDebug > 1)
								OutputDebugString("Research reenter\n");
							bSearchFlag = 0; 		/* hack */
							break;
						}
						else goto search;
					}
/*					otherwise drop through and treat as IDM_SEARCH */

				case IDM_SEARCH:
					if (bFileValid == 0) break;
					if (bSearchFlag != 0) {
						if (bDebug > 1)	OutputDebugString("Search reenter\n");
						bSearchFlag = 0; 		/* hack */
						break;
					}
					/* 1994/Jan/7 */ /* 1994/Feb/8 */
					if (bAllowCommDlg && bUsingCommDlg && bNewSearch) {
						if (uFindReplaceMsg == 0) {	/* only if not already */
							CommSearch (hInst, hWnd);
							break;					/* sets up searchtext */
						}
/*						drop through if already active search ??? */
/* 						break; */		/* ??? */
					}
					else {
/*						flag = DialogBox(hInst, "SearchText", hWnd, lpSearchDlg); */
/*						flag = DialogBox(hInst, "SearchText", hWnd, (DLGPROC) SearchDlg); */
						flag = DialogBox(hInst, "SearchText", hWnd, SearchDlg);
						if (flag == 0) break;  /*  cancelled! */
					}
/*					if (flag == 0) break; */  /*  cancelled! */
			search:
					if (bSearchFlag++ == 0) {
						if (hFile != -1 && bKeepFileOpen == 0) { /* debug */
							winerror("File still open");
							bSearchFlag = 0;
							break;
						}
						if (bBusyFlag++ > 0) {
							if (bDebug > 1)	OutputDebugString("Busy Search\n");
							bSearchFlag = 0;
							break;		/* prevent reentry */
						}
						if (searchandmark(hWnd, 0) != 0) {/* text search */
							bBusyFlag = 0;
							bSearchFlag = 0;
							if (bDebug > 1)
								OutputDebugString("Searched and Marked\n");
							graypreviousnext(hWnd);
							bEnableTermination = 1;		/* NEW */
							InvalidateRect(hWnd, NULL, TRUE);
						}
						else {
							bBusyFlag = 0;
							bSearchFlag = 0;
/*							if (bDebug > 1)
								OutputDebugString("No refresh needed\n"); */
						}
/*						graypreviousnext(hWnd); */
					}
					else {
						if (bDebug > 1)	OutputDebugString("Search Hack\n");
						bSearchFlag = 0; 		/* hack */
					}
					break;

				case IDM_ESCAPE:			/* 1993/Aug/17 */
#ifdef IGNORED
/* check first if main window has the focus */ /* 1994/Jan/11 */
					if ((hFocus = GetFocus()) != hWnd && hFocus != NULL) {
/* See whether DVI FIle Info or DVI Font Info modeless dialog have focus */
						HWND hParent = GetParent(hFocus);
/* if so, just make those dialog boxes go away */
/* really need a way to do this for *all* modeless dialog boxes ... */
						if (hParent == hInfoBox || hParent == hFntBox ||
							hParent == hFindBox || hParent == hConsoleWnd)
							PostMessage(hParent, WM_CLOSE, 0, 0L); 
							break;	
					}
#endif
				
/*		If showing font sample, treat Escape as `Close' from `File' menu */
					if (bFontSample) {
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
						(void) PostMessage(hWnd, WM_COMMAND, IDM_CLOSE, 0L); 
						break;
					}
/* Don't crash and burn if user disallows this ... */
					if (bEscapeEnable == 0) break;
/* Else drop through and escape ... */
				
				case IDM_EXIT:
					if (bFileValid != 0 && bTeXHideFlag != 0)
						reverttexhelp();		/* 1993/Dec/29 */
/*					bFlipSave = bShiftFlag; */   /* flip save preference */
/*					bFlipSave = GetAsyncKeyState(VK_SHIFT) & 0x8000; */
					if (GetAsyncKeyState(VK_SHIFT) < 0 ||
						GetAsyncKeyState(VK_CONTROL) < 0) 
							bFlipSave = 1;
					else bFlipSave = 0;
					if (bFlipSave != 0) {	/* 1993/March/16 */
						if (bSavePrefer != 0) {
							if (wincancel("Not saving preferences?") != 0)
								break;		/* don't exit if user CANCELs */
						}
						else {
							if (wincancel("Saving preferences?") != 0)
								break;		/* don't exit if user CANCELs */
						}
					}
/*					(void) DestroyWindow(hWnd); */
					PostQuitMessage(0);		/* experiment 92/01/25 */
/*					return 0; */
					break;

				case IDC_EDIT:				/* this shouldn't happen ? */
					if (cmd == EN_ERRSPACE) {
						if ((hFocus = GetFocus()) == NULL) hFocus = hwnd;
						(void) MessageBox (hFocus, "Out of memory.",
								"Open File", MB_ICONSTOP | MB_OK);
					}
					break;

				case IDM_BORDER:
					hDC = GetDC(hWnd);
					(void) SetMapMode(hDC, MM_TWIPS); /* set unit to twips */
					hMenu = GetMenu(hWnd);
					bShowBorder = ! bShowBorder;
					DrawBorder(hDC, bShowBorder ? hBorderPen : hErasePen);
					(void) CheckMenuItem (hMenu, IDM_BORDER,
						bShowBorder ? MF_CHECKED : MF_UNCHECKED);
					(void) ReleaseDC(hWnd, hDC);
					break;

/* assumes these are numbered IDM_LETTER + (DMPAPER_xyz - DMPAPER_LETTER) */

				case IDM_LETTER:
				case IDM_LEGAL:
				case IDM_TABLOID:
				case IDM_LEDGER:
				case IDM_STATEMENT:
				case IDM_EXECUTIVE:
				case IDM_ATHREE:
				case IDM_AFOUR:
				case IDM_AFIVE:
				case IDM_BFOUR:
				case IDM_BFIVE:
				case IDM_CUSTOMSIZE:
					newpapertype = (id - IDM_LETTER) + DMPAPER_LETTER;
/*					if (bDebug > 1) {
						sprintf (debugstr, "id %d new paper type %d\n", 
							id, newpapertype);
						OutputDebugString(debugstr);
					} */
					hDC = GetDC(hWnd);
					(void) SetMapMode(hDC, MM_TWIPS);	/* set unit to twips */
					if (bShowBorder != 0) DrawBorder(hDC, hErasePen);
					papertype = newpapertype;
					setpagesize(papertype);
					checkpaper(hWnd, papertype);
					if (bShowBorder != 0) DrawBorder(hDC, hBorderPen);
					(void) ReleaseDC(hWnd, hDC);
					VScrollPos(hWnd);			/* 1993/Feb/22 */
					HScrollPos(hWnd);
					break;

/*  this assumes they are sequentially numbered */

				case IDM_SCALEDPOINT:
				case IDM_POINT:
				case IDM_BIGPOINT:
				case IDM_DIDOT:
				case IDM_MILLIMETER:
				case IDM_PICA:
				case IDM_CICERO:
				case IDM_CENTIMETER:
				case IDM_INCH:
				case IDM_PICAPOINT:
					units =	id - IDM_SCALEDPOINT;
					if (units == 9) pcptflag = 1;
					else pcptflag = 0;
					checkunits(hWnd, units);	/* 1993/Feb/21 */
					break;

				case IDM_SIMPLEX:				/* 1996/Nov/17 */
				case IDM_VERTICAL:
				case IDM_HORIZONTAL:
					nDuplex = DMDUP_SIMPLEX + (id - IDM_SIMPLEX);
					checkduplex(hWnd, nDuplex);	/* 1996/Nov/17 */
					break;
								
				case IDM_SHOWHXW:
					hDC = GetDC(hWnd);
					(void) SetMapMode(hDC, MM_TWIPS);/* set unit to twips */
					hMenu = GetMenu(hWnd);
					bShowLiner = ! bShowLiner;
					DrawLiner(hDC, bShowLiner ? hLinerPen : hErasePen);
					(void) CheckMenuItem (hMenu, IDM_SHOWHXW,
						bShowLiner ? MF_CHECKED : MF_UNCHECKED);
					(void) ReleaseDC(hWnd, hDC);
					break;

				case IDM_NEWPAGESCALE:		/* remember current scale */
/*					pagexoffset = xoffset; 
					pageyoffset = yoffset;
					pagewantedzoom = wantedzoom; */
					if (bSpreadFlag == 0) {
						pagexoffset = xoffset; 
						pageyoffset = yoffset;
						pagewantedzoom = wantedzoom;
					}
					else {					/* 1996/May/12 */
						spreadxoffset = xoffset; 
						spreadyoffset = yoffset;
						spreadwantedzoom = wantedzoom;
					}
					break;
					
				case IDM_RESETSCALE:
					hMenu = GetMenu(hWnd);
					bResetScale = ! bResetScale;
					(void) CheckMenuItem (hMenu, IDM_RESETSCALE,
						bResetScale ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_RESETPAGE:
					hMenu = GetMenu(hWnd);
					bResetPage = ! bResetPage;
					(void) CheckMenuItem (hMenu, IDM_RESETPAGE,
						bResetPage ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_TRUEINCH:
					hMenu = GetMenu(hWnd);
					bTrueInch = ! bTrueInch;
					(void) CheckMenuItem (hMenu, IDM_TRUEINCH,
						bTrueInch ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_COMPLAINMISSING:
					hMenu = GetMenu(hWnd);
					bComplainMissing = ! bComplainMissing;
					(void) CheckMenuItem (hMenu, IDM_COMPLAINMISSING,
						  bComplainMissing ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_COMPLAINENCODING:			/* 1995/Jan/12 */
					hMenu = GetMenu(hWnd);
					bCheckEncoding = ! bCheckEncoding;
					(void) CheckMenuItem (hMenu, IDM_COMPLAINENCODING,
						bCheckEncoding ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_COMPLAINFILES:
					hMenu = GetMenu(hWnd);
					bComplainFiles = ! bComplainFiles;
					(void) CheckMenuItem (hMenu, IDM_COMPLAINFILES,
						bComplainFiles ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_COMPLAINSPECIAL:
					hMenu = GetMenu(hWnd);
					bComplainSpecial = ! bComplainSpecial;
					(void) CheckMenuItem (hMenu, IDM_COMPLAINSPECIAL,
						bComplainSpecial ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_IGNORESPECIAL:
					hMenu = GetMenu(hWnd);
					bIgnoreSpecial = ! bIgnoreSpecial;
					(void) CheckMenuItem (hMenu, IDM_IGNORESPECIAL,
						bIgnoreSpecial ? MF_CHECKED : MF_UNCHECKED);
					InvalidateRect(hWnd, NULL, TRUE); /* in case specials */
					break;

				case IDM_SHOWPREVIEW:
					hMenu = GetMenu(hWnd);
					bShowPreview = ! bShowPreview;
					(void) CheckMenuItem (hMenu, IDM_SHOWPREVIEW,
						bShowPreview ? MF_CHECKED : MF_UNCHECKED);
					InvalidateRect(hWnd, NULL, TRUE); /* in case specials */
					break;
					
				case IDM_LANDSCAPE:
					hDC = GetDC(hWnd);
					(void) SetMapMode(hDC, MM_TWIPS);	/* set to twips */
					if (bShowBorder) DrawBorder(hDC, hErasePen);

					hMenu = GetMenu(hWnd);
					bLandScape = ! bLandScape;
					(void) CheckMenuItem (hMenu, IDM_LANDSCAPE,
						bLandScape ? MF_CHECKED : MF_UNCHECKED);

					if (bBusyFlag == 0 && hPrintDC != NULL) {
						(void) DeleteDC(hPrintDC);
						hPrintDC = NULL;
					}

					if (bShowBorder) DrawBorder(hDC, hBorderPen);
					(void) ReleaseDC(hWnd, hDC);

					if (bSpreadFlag) {
						bEnableTermination = 1;		/* NEW */
						InvalidateRect(hWnd, NULL, TRUE);
					}
					break;

 				case IDM_GREYTEXT:
					hMenu = GetMenu(hWnd);
					bGreyText =  ! bGreyText;
					if (bGreyText) {
						bGreyPlus = 0;
						(void) CheckMenuItem (hMenu, IDM_GREYPLUS, MF_UNCHECKED);
					}
					(void) CheckMenuItem (hMenu, IDM_GREYTEXT,
						bGreyText ? MF_CHECKED : MF_UNCHECKED);
/*					bEnableTermination = 1;		 */ /* fast enough */
					InvalidateRect(hWnd, NULL, TRUE);
					break;

 				case IDM_GREYPLUS:
					hMenu = GetMenu(hWnd);
					bGreyPlus = ! bGreyPlus;
					if (bGreyPlus) {
						bGreyText = 0;
						(void) CheckMenuItem (hMenu, IDM_GREYTEXT, MF_UNCHECKED);
					}
					(void) CheckMenuItem (hMenu, IDM_GREYPLUS,
						bGreyPlus ? MF_CHECKED : MF_UNCHECKED);
/*					if (bGreyText)	InvalidateRect(hWnd, NULL, TRUE); */
/*					bEnableTermination = 1;		 */ /* fast enough */
					InvalidateRect(hWnd, NULL, TRUE);
					break;

///////////////////////////////////////////////////////////////////////////////////////

				case IDM_PASSTHROUGH:
					hMenu = GetMenu(hWnd);
					bPassThrough = ! bPassThrough;
					(void) CheckMenuItem (hMenu, IDM_PASSTHROUGH,
										  bPassThrough ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_CALLBACKPASS:
					hMenu = GetMenu(hWnd);
					bCallBackPass = ! bCallBackPass;
					(void) CheckMenuItem (hMenu, IDM_CALLBACKPASS,
										  bCallBackPass ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_CONSOLEOPEN:
					hMenu = GetMenu(hWnd);
					bConsoleOpen = ! bConsoleOpen;
					(void) CheckMenuItem (hMenu, IDM_CONSOLEOPEN,
										  bConsoleOpen ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_USEDLLS:
					hMenu = GetMenu(hWnd);
					bUseDLLs = ! bUseDLLs;
					(void) CheckMenuItem (hMenu, IDM_USEDLLS,
										  bUseDLLs ? MF_CHECKED : MF_UNCHECKED);
					break;

//				case IDM_OPENCLOSECHANNEL:
//					hMenu = GetMenu(hWnd);
//					bOpenCloseChannel = ! bOpenCloseChannel;
//					(void) CheckMenuItem (hMenu, IDM_OPENCLOSECHANNEL,
//										  bOpenCloseChannel ? MF_CHECKED : MF_UNCHECKED);
//					break;

				case IDM_OPENCLOSEPRINTER:
					hMenu = GetMenu(hWnd);
					bOpenClosePrinter = ! bOpenClosePrinter;
					(void) CheckMenuItem (hMenu, IDM_OPENCLOSEPRINTER,
										  bOpenClosePrinter ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_NEWPASSTHROUGH:
					hMenu = GetMenu(hWnd);
					bNewPassThrough = ! bNewPassThrough;
					(void) CheckMenuItem (hMenu, IDM_NEWPASSTHROUGH,
										  bNewPassThrough ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_FORCEPASSBACK:
					hMenu = GetMenu(hWnd);
					bForcePassBack = ! bForcePassBack;
					(void) CheckMenuItem (hMenu, IDM_FORCEPASSBACK,
										  bForcePassBack ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_FORCEOLDPASSTHROUGH:
					hMenu = GetMenu(hWnd);
					bOldPassThrough = ! bOldPassThrough;
					(void) CheckMenuItem (hMenu, IDM_FORCEOLDPASSTHROUGH,
										  bOldPassThrough ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_DONTASKFILE:
					hMenu = GetMenu(hWnd);
					bDontAskFile = ! bDontAskFile;
					(void) CheckMenuItem (hMenu, IDM_DONTASKFILE,
										  bDontAskFile ? MF_CHECKED : MF_UNCHECKED);
					break;

////////////////////////////////////////////////////////////////////////////////////

#ifdef IGNORED
				case IDM_PRINTFRAME:
					hMenu = GetMenu(hWnd);
					bPrintFrame = ! bPrintFrame;
					(void) CheckMenuItem (hMenu, IDM_PRINTFRAME,
						bPrintFrame ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_SNAPTO:
					hMenu = GetMenu(hWnd);
					bSnapToFlag = ! bSnapToFlag;
					(void) CheckMenuItem (hMenu, IDM_SNAPTO,
						bSnapToFlag ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;		 /* NEW */
					InvalidateRect(hWnd, NULL, TRUE);
					break;

				case IDM_DRAWVISIBLE:
					hMenu = GetMenu(hWnd);
					bDrawVisible = ! bDrawVisible;
					(void) CheckMenuItem (hMenu, IDM_DRAWVISIBLE,
						bDrawVisible ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_TEXTOUT:
					hMenu = GetMenu(hWnd);
					bTextOutFlag = ! bTextOutFlag;
					(void) CheckMenuItem (hMenu, IDM_TEXTOUT,
						bTextOutFlag ? MF_CHECKED : MF_UNCHECKED);
					InvalidateRect(hWnd, NULL, TRUE);
					break;
#endif
					
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

				case IDM_SAVENOW:					/* 1995/April/22 */
					WritePreferences(1);
					break;

				case IDM_SAVEPREFER:
					hMenu = GetMenu(hWnd);
					bSavePrefer = ! bSavePrefer;
					(void) CheckMenuItem (hMenu, IDM_SAVEPREFER,
						bSavePrefer ? MF_CHECKED : MF_UNCHECKED);
					bFactory=0;			/* user explicitly set Save Prefer */
					break;

				case IDM_COLORFONT:
					hMenu = GetMenu(hWnd);
					bColorFont = ! bColorFont;
					(void) CheckMenuItem (hMenu, IDM_COLORFONT,
						bColorFont ? MF_CHECKED : MF_UNCHECKED);

					if (bShowUsedFlag) {
						(void) SendMessage(hFntBox, WM_INITDIALOG, 0, 0L);
						SetFocus(hWnd);			/* 99/Jan/11 ??? */
					}
					bEnableTermination = 1;		 /* NEW */
					InvalidateRect(hWnd, NULL, TRUE);
					break;

				case IDM_USEWORKDIR:
					hMenu = GetMenu(hWnd);
					bUseWorkPath = ! bUseWorkPath;
					if (bUseWorkPath) {
//	get working directory from user - default is IniDefPath if exists
//	write result in dviwindo.ini also
//	if user cancels, reset bUseWorkPath and bUseSourcePath
//	check the directory exists
						bUseWorkPath = GetNewWorkDir(hWnd);
					}
					if (bUseWorkPath) {
						sprintf(str, "Writing DVI files into `%s'",
								(IniDefPath != NULL) ? IniDefPath : "");
						wininfo(str);
					}
					else wininfo("Writing DVI files back into source file folders");
					bUseSourcePath = ! bUseWorkPath;	// complement of new value
					if (bUseWorkPath == 0) {
						if (IniDefPath != NULL) free(IniDefPath);
						IniDefPath = NULL;
					}

					(void) CheckMenuItem (hMenu, IDM_USEWORKDIR,
								  bUseWorkPath ? MF_CHECKED : MF_UNCHECKED);

#ifdef IGNORED
					strcpy(str, "Will write output from TeX into ");
					if (bUseWorkPath) {
						if (*IniDefPath != '\0') { 
							bUseWorkPath = 1;
							bUseSourcePath = 0;
						}
						strcat (str, "WorkingDirectory specified in ");
						if (bWorkingExists) strcat (str, "dviwindo.ini:\n\n");
						else strcat (str, "DVIWindo icon:\n\n");
						strcat(str, IniDefPath);
						if (!bWorkingExists)  
							strcat(str, "\n\n(For this session)"); 
					}
					else {
						bUseSourcePath = 1;
						strcat(str, "directory of source file.");
						if (bWorkingExists)  
							strcat(str, "\n\n(For this session)"); 
					}
					wininfo(str);
#endif
					break;

				case IDM_SHOWCOUNTER:					/* 1996/Jan/20 */
					for (n = 9; n >= 0; n--) {			/* last non zero */
						if (counter[n] != 0) break;
					}
					sprintf(str, "%d", counter[0]);		/* always show first */
					s = str + strlen(str);
					if (n > 0) {
						for (k = 1; k < n; k++) {
							sprintf(s, "  %d", counter[k]);
							s = s + strlen(s);
						}
					}
					wininfo(str);
					break;

				case IDM_MAGNIFICATION:
					showmagnification(hWnd);	/* in winanal.c */
					break;

				case IDM_SYSFLAGS:
					ShowSysFlags();
					break;

				case IDM_SELECTFONT:
					if (bDebug > 1) {
						sprintf(debugstr, "wParam %X lParam %X", wParam, lParam);
						OutputDebugString(debugstr);
					}
					bShowWidths = 0;
					bWriteAFM = 0;
			selectfont:
					flag = lParam;		/* controls whether dialog shown */
					colorindex = 0;
/*					if (bFileValid != 0) closeup(hWnd, 1);	*/
					if (! bFontSample) {
						EnterFontState(hWnd);
						checkcontrols(hWnd, -1);
						hidedialogs(hWnd);		/* hide dialog boxes */
						VScrollPos(hWnd); 
						HScrollPos(hWnd); 
						ExposeScrollBars(hWnd);
					}
					bFontSample = 0;
					bCharacterSample = 0;
					bFileValidOld = bFileValid;
					bFileValid = 0;
					InvalidateRect(hWnd, NULL, TRUE);		/* MOVED ? */
/* at this point bFontSample & bFileValid are zero, so should just clear */
/* is there some better way to repaint the background ??? */
/*					if (GetFontSelection(hWnd, bShowWidths) != 0) */
					if (GetFontSelection(hWnd, bShowWidths, flag) != 0) {
						if (bWriteAFM != 0) {
							hDC = GetDC(hWnd);
/*							flag = WriteAFMFile(hWnd, hDC); */
/* bWriteAFM = 1 for WriteAFM..., bWriteAFM = 2 for WriteTFM...*/
/*							flag = WriteAFMFile(hWnd, hDC, bWriteAFM-1, 0); */
							flag = WriteAFMFileSafe(hWnd, hDC, bWriteAFM-1, 0);  
							(void) ReleaseDC(hWnd, hDC);
						}
/* show widths only if asked, or if WriteAFM was successful 93/April/11 */
/*						if (bWriteAFM == 0 || flag == 0) */
						if (bWriteAFM == 0) {				/* 95/Jan/14 */
							bFileValid = bFileValidOld;
							InvalidateRect(hWnd, NULL, TRUE); /* MOVE erase */
							break;					/* normal way out of here */
						}
						else bWriteAFM = 0;					/* 95/Aug/22 */
					}
					bFileValid = bFileValidOld;
			fontcancel:
				    EndFontState(hWnd);
					restoredialogs(hWnd);			/* new */
					if (bFileValid)
						checkcontrols(hWnd, 1); /* reenable menu items */
					else
						checkcontrols(hWnd, -1); /* disable PRINT */ /* NEW */
					bEnableTermination = 1;		 /* NEW */
					InvalidateRect(hWnd, NULL, TRUE);	/* MOVE erase */
					break;

				case IDM_CHARWIDTHS:		/* show char widths instead */
					bShowWidths = 1;
					bWriteAFM = 0;
					goto selectfont;
/*					break; */
					
				case IDM_WRITEAFM:		/* write AFM file */
					bShowWidths = 1; 
/*					bShowWidths = 0; */	/* ??? */
					bWriteAFM = 1;
					goto selectfont;
/*					break; */

				case IDM_WRITETFM:		/* write AFM file and make TFM */
					bShowWidths = 1; 
/*					bShowWidths = 0; */	/* ??? */
					bWriteAFM = 2;
					goto selectfont;
/*					break; */									
					
				case IDM_WRITEAFMS:	/* make AFM files all text fonts */
/*					bShowWidths = 0; */				/* ??? */
/*					bShowWidths = 1; */				/* ??? */
/*					bWriteAFM = 0; */				/* ??? */
					bWriteAFM = 1;					/* ??? */
/*					flag = 0; */
					goto commonwrite;
							
				case IDM_WRITETFMS:	/* make TFM files all text fonts */
/*					bShowWidths = 0; */				/* ??? */
/*					bShowWidths = 1; */				/* ??? */
					bWriteAFM = 2;					/* ??? */
/*					flag = 1; */
				commonwrite:
					bFontSample = 0;
					bCharacterSample = 0;
					bFileValidOld = bFileValid; 
					bFileValid = 0;
					InvalidateRect(hWnd, NULL, TRUE);
/*					(void) WriteAllTFM(hWnd, flag); */		/* in winfonts.c */
					(void) WriteAllTFM(hWnd, bWriteAFM-1);	/* in winfonts.c */
					bWriteAFM = 0;
					bFileValid = bFileValidOld; 
					InvalidateRect(hWnd, NULL, TRUE);
/*					bShowWidths = 0; */
/*					bWriteAFM = 0; */
					break;

				case IDM_RULEFILL: 
/*				case IDM_OUTLINERULE: */
					hMenu = GetMenu(hWnd);
					bRuleFillFlag = ! bRuleFillFlag;
					(void) CheckMenuItem (hMenu, IDM_RULEFILL, 
						bRuleFillFlag ? MF_CHECKED : MF_UNCHECKED); 
					bEnableTermination = 1;		 /* NEW */
					InvalidateRect(hWnd, NULL, TRUE);
					break;

				case IDM_SHOWCALLS:				/* 96/Sep/15 */ 
					hMenu = GetMenu(hWnd);
					bShowCalls = ! bShowCalls;
					(void) CheckMenuItem (hMenu, IDM_SHOWCALLS,
						bShowCalls ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_PAUSECALLS:			/* 96/Sep/15 */ 
					hMenu = GetMenu(hWnd);
					bPauseCalls = ! bPauseCalls;
					(void) CheckMenuItem (hMenu, IDM_PAUSECALLS,
						bPauseCalls ? MF_CHECKED : MF_UNCHECKED);
/* have it take effect right away for YandYTeX and DVIPSONE and AFMtoTFM */
					WritePrivateProfileString(achEnv, "DEBUGPAUSE",
						  bPauseCalls ? "1" : NULL, achFile);
/*					if (bPauseCalls) 
						WritePrivateProfileString(achEnv, "DEBUGPAUSE",
							"1", achFile);
					else WritePrivateProfileString(achEnv, "DEBUGPAUSE",
							NULL, achFile); */
/*					break; */	/* removed 98/Mar/26 ??? */
/*					flush the dviwindo.ini cache in Windows 95 ??? */
					WritePrivateProfileString(NULL, NULL, NULL, achFile);
					break;

				case IDM_SYSTEMINFO:		/* 95/Mar/18 */
					ShowSystemInfo();
					break;

				case IDM_VIEWEXTRAS:		/* 95/Mar/28 */
					hMenu = GetMenu(hWnd);
					bViewExtras = ! bViewExtras;
					(void) CheckMenuItem (hMenu, IDM_VIEWEXTRAS,
						bViewExtras ? MF_CHECKED : MF_UNCHECKED);
					InvalidateRect(hWnd, NULL, TRUE);
					break;

				case IDM_COUNTZERO:
					hMenu = GetMenu(hWnd);
					bCountZero = ! bCountZero;
					if (bCountZero) {	/* use counter[0] */
						if(bSpreadFlag) {
							olddvipage = dvipage;
							usepagetable(dvipage, 0);		/* page is physical page */
							if (leftcurrent < 0 && rightcurrent < 0 &&
								dvipage <= dvi_t) { /* < dvi_t ??? */
								dvipage++;
								usepagetable(dvipage, 0);	/* page is physical page */
							}
							if (dvipage != olddvipage ||
								leftcurrent < 0 || rightcurrent < 0)
									bEnableTermination = 1;		 /* NEW */
									InvalidateRect(hWnd, NULL, TRUE);
						}
					}
					else {						/* use dvi pages */
/*						current = rightcurrent; */
/*						dvipage = rightdvipage; */
						if (leftcurrent < 0 || rightcurrent < 0) {
							usepagetable(dvipage, 0);	/* page is physical page */
							bEnableTermination = 1;		 /* NEW */
							InvalidateRect(hWnd, NULL, TRUE);
						}
					}
					(void) CheckMenuItem (hMenu, IDM_COUNTZERO,
						bCountZero ? MF_CHECKED : MF_UNCHECKED);
					SetupWindowText(hWnd);	/* change title bar */
					break;
					
				case IDM_SPREAD:
					hMenu = GetMenu(hWnd);
					bSpreadFlag = ! bSpreadFlag;
					if (bSpreadFlag) {
						if (bFileValid != 0) {
							rightcurrent = current;
							rightdvipage = dvipage;
							usepagetable(dvipage, 0);		/* page is physical page */
#ifdef DEBUGPAGETABLE
							if (bDebug > 1) {
								sprintf(debugstr,
	"SPREAD dvipage %d leftcurrent %d rightcurrent %d leftdvi %d rightdvi %d",
										dvipage, leftcurrent, rightcurrent, leftdvipage, rightdvipage);
								OutputDebugString(debugstr);
							}
#endif
							if (leftcurrent < 0 && rightcurrent < 0) {
								if (dvipage <= dvi_t) dvipage++;	/* ??? 99/May/31 */
								else if (dvipage >= 1) dvipage--;	/* ??? */
								usepagetable(dvipage, 0);	/* page is physical page */
							}
#ifdef DEBUGPAGETABLE
							if (bDebug > 1) {
								sprintf(debugstr,
	"SPREAD dvipage %d leftcurrent %d rightcurrent %d leftdvi %d rightdvi %d",
										dvipage, leftcurrent, rightcurrent, leftdvipage, rightdvipage);
								OutputDebugString(debugstr);
							}
#endif
							if (leftcurrent < 0 && rightcurrent < 0) {
/*	if *still* unable to win, should take drastic action ??? 99/May/31 */
							}
						}
					}
					else {						/* switch back to non-spread mode */
						if (bFileValid) {
							dvipage = rightdvipage;
							current = rightcurrent;
							usepagetable(dvipage, 0);		/* page is physical page */
						}
					}

					(void) CheckMenuItem (hMenu, IDM_SPREAD,
						bSpreadFlag ? MF_CHECKED : MF_UNCHECKED);
					SetupWindowText(hWnd);	/* change title bar */ /* ? */
					bEnableTermination = 1;		 /* NEW */
/* not sure this does what we want --- it uses default settings 96/May/12*/
/* does not provide for saving and restoring of xoffset & yoffset */
					setoffsetandzoom(0);		/* 96/May/12 */
					newmapandfonts(hWnd);		/* 96/May/12 */
					InvalidateRect(hWnd, NULL, TRUE);
					break;
					
				case IDM_STRINGLIMIT:				/* misnomer */
					hMenu = GetMenu(hWnd);
/*					if (StringLimit == 1) StringLimit = MAXCHARBUF;
					else StringLimit = 1; */
					bUseCharSpacing = ! bUseCharSpacing;
					(void) CheckMenuItem (hMenu, IDM_STRINGLIMIT,
						bUseCharSpacing ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;		
					InvalidateRect(hWnd, NULL, TRUE);
					break; 

				case IDM_SHOWBUTTONS:
					hMenu = GetMenu(hWnd);
					bShowButtons = ! bShowButtons;
					(void) CheckMenuItem (hMenu, IDM_SHOWBUTTONS,
						bShowButtons ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;		
					InvalidateRect(hWnd, NULL, TRUE);
					break; 

				case IDM_SHOWVIEWPORTS:			/* 96/Aug/16 */
					hMenu = GetMenu(hWnd);
					bShowViewPorts = ! bShowViewPorts;
					(void) CheckMenuItem (hMenu, IDM_SHOWVIEWPORTS,
						bShowViewPorts ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;		
					InvalidateRect(hWnd, NULL, TRUE);
					break; 

				case IDM_SHOWTIFF:			/* 97/Jan/7 */
					hMenu = GetMenu(hWnd);
					bShowTIFF = ! bShowTIFF;
					(void) CheckMenuItem (hMenu, IDM_SHOWTIFF,
						bShowTIFF ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;		
					InvalidateRect(hWnd, NULL, TRUE);
					break; 

				case IDM_SHOWWMF:			/* 97/Jan/7 */
					hMenu = GetMenu(hWnd);
					bShowWMF = ! bShowWMF;
					(void) CheckMenuItem (hMenu, IDM_SHOWWMF,
						bShowWMF ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;		
					InvalidateRect(hWnd, NULL, TRUE);
					break; 

				case IDM_TEXFLAGS:
				case IDM_DVIFLAGS:
				case IDM_DVIDISFLAGS:
				case IDM_TFMFLAGS:
					editcommandflags(hWnd, id);
					SetCommandStrings();				/* 97/Apr/3 */
					break;

				case IDM_TEXNANSI:
				case IDM_TEX256:
				case IDM_ANSINEW:
				case IDM_STANDARD:
				case IDM_TEXTEXT:
				case IDM_CUSTOMENCODING:
					if (CheckEncodingMenu(hWnd, id)) {	/* 97/Apr/5 */
						FreeATMTables();
/*						read the newly selected encoding vector */
						(void) ReadEncoding(1);			/* winsearc.c */
/*						redo ATM hEncoding stuff */					
						if (!bWinNT) (void) SetupATMSelectEncoding(1);
						setupTeXFonts();	/* reset TFM dir path 97/Apr/20 */
/*						if (fontmap != NULL) free_fontmap(); */	/* 97/Apr/20 */
						if (bShowUsedFlag != 0) {
							(void) SendMessage(hFntBox, WM_INITDIALOG, 0, 0L);
							SetFocus(hWnd);			/* 99/Jan/11 ??? */
						}
						bEnableTermination = 1;
						InvalidateRect(hWnd, NULL, TRUE); 
					}
/*	also need to restablish metrics for the fonts ? */
					cancelmetrics();
/*	also reset complaint about encoding warned */
					break;

/******************** start of debugging menu ******************************/

/* Don't need to protect with if (bDebug > 1) since menu not accessible */
				case IDM_DEBUGSTRING:
					if (bDebug > 1)
						OutputDebugString("OutputDebugString Test.\n");
					break;

				case IDM_STACKUSED:
/*					showstackused(hWnd, 1); */
					showstackused(1);	/* useless in WIN32 */
					break;					

				case IDM_SHOWSCALE:
					showscaling(hWnd);
					break;

				case IDM_FILESPECS:
					showfilespecs(hWnd);
					break;

				case IDM_PRINTSPECS:
					showprintspecs(hWnd);
					break;					

				case IDM_SHOWMETRICS:
					showmetrics(hWnd);
					break;

				case IDM_TAGGEDCHAR:
/*					showtaggedchar(hWnd); */
					showtaggedchar();
					break;					

				case IDM_IGNORESELECT:
					hMenu = GetMenu(hWnd);
					bIgnoreSelect = ! bIgnoreSelect;
					(void) CheckMenuItem (hMenu, IDM_IGNORESELECT,
							  bIgnoreSelect ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_TESTFLAG:
					hMenu = GetMenu(hWnd);
					bTestFlag = ! bTestFlag;
					(void) CheckMenuItem (hMenu, IDM_TESTFLAG,
							  bTestFlag ? MF_CHECKED : MF_UNCHECKED);
/*					bEnableTermination = 1;	*/
/*					InvalidateRect(hWnd, NULL, TRUE); */
/*					if (bTestFlag != 0) metricsize=1500;
					else metricsize=1000;	*//* Test Hack 1993/June/3 */
					break;

				case IDM_READPREFER:
/*					readpreferences(); */
					ReadFixedPrefer ();	
					SetCommandStrings();
					ReadEnvVars();				/* split off 98/Dec/24 */
					break;

/******************** end of debugging menu ******************************/

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*				case IDM_MAXDRIFT:
					hMenu = GetMenu(hWnd);
					bMaxDriftFlag = ! bMaxDriftFlag;
					(void) CheckMenuItem (hMenu, IDM_MAXDRIFT,
						bMaxDriftFlag ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;	
					InvalidateRect(hWnd, NULL, TRUE);
					break; */

/*				case IDM_DVITYPE:
					hMenu = GetMenu(hWnd);
					bDVITypeFlag = ! bDVITypeFlag;
					(void) CheckMenuItem (hMenu, IDM_DVITYPE,
						bDVITypeFlag ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;		
					InvalidateRect(hWnd, NULL, TRUE);
					break; */

/*				case IDM_PRESPACING:
					hMenu = GetMenu(hWnd);
					bPreserveSpacing = ! bPreserveSpacing;
					clearmetrics();	
					(void) CheckMenuItem (hMenu, IDM_PRESPACING,
						(bPreserveSpacing != 0) ? MF_CHECKED : MF_UNCHECKED);
					bEnableTermination = 1;	
					InvalidateRect(hWnd, NULL, TRUE);
					break; */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

				case IDM_CLIPBOARD:
					if (bBusyFlag != 0) break;	/* busy repainting */
					if (bFileValid == 0 && bFontSample == 0)
						break;
					if (bRectFlag < 0) {	/* a way to cancel COPY */
						bRectFlag = 0; 
/*						(void) SetCursor(hSaveCursor); */
						(void) SetCursor(hOldCursor);
/*						ReleaseCapture(); */
						break;
					}
/*					if (bBusyFlag++ != 0) break; */	/* busy repainting */
					bRectFlag = -1;			/* now wait for WM_LBUTTONDOWN */
/*					(void) DoCopy(hWnd); */	/* in winprint.c */
/*					bBusyFlag = 0;		*/	/* redundant */
/*					hSaveCursor = SetCursor(hCopyCursor); */
					hOldCursor = SetCursor(hCopyCursor); 
/*					(void) SetCapture(hWnd); */ /* grab the mouse */
					break;

				case IDM_SHOWINFO:
					if (bFileValid == 0) break;
					hMenu = GetMenu(hWnd);
					bShowInfoFlag = ! bShowInfoFlag;
					if (bShowInfoFlag) 
						ShowDVIMetric(hWnd);		/* set up dialog box */
					else {							/* or remove it */
						(void) DestroyWindow(hInfoBox);
						hInfoBox = NULL;				/* debugging */
					}
					bShowInfoExposed = bShowInfoFlag;
					(void) CheckMenuItem (hMenu, IDM_SHOWINFO,
						bShowInfoFlag ? MF_CHECKED : MF_UNCHECKED);
					break;

				case IDM_FONTSUSED:
					if (bFileValid == 0) break;
					hMenu = GetMenu(hWnd);
					bShowUsedFlag = ! bShowUsedFlag;
					if (bShowUsedFlag) 
						ShowFontsUsed(hWnd);		/* set up dialog box */
					else {							/* or remove it */
						(void) DestroyWindow(hFntBox);
						hFntBox = NULL;				/* debugging */
					}
					bShowUsedExposed = bShowUsedFlag;
					(void) CheckMenuItem (hMenu, IDM_FONTSUSED,
						bShowUsedFlag ? MF_CHECKED : MF_UNCHECKED);
					break;

/*				case IDM_PREVIOUS:
					if (bControlFlag != 0) {
						xoffsetold = xoffset; yoffsetold = yoffset; 
						wantedzoomold = wantedzoom; 
						resetpagemapping(hWnd); 
						setscale(wantedzoom);
						adjusttobottom(hWnd);
						if ((bFileValid != 0 && dvipage != -INFINITY) ||
							bFontSample != 0) {
							newmapandfonts(hWnd); 
						}
					} */

				case IDM_BACKSPACE:
					if (bFileValid == 0) break;	/* 1993/Sep/20 */
					bSpaceFlag = 1;
					goto commonprevious;

				case IDM_PGUP:
/*					if (bAltHomeEnd) {
						(void) PostMessage(hwnd, WM_COMMAND, IDM_TOP, 0L);
						break;
					} */
				case IDM_PREVIOUS: 
					bSpaceFlag = 0;
				commonprevious:
					if (bFileValid != 0 && dvipage != -INFINITY) {
						lastsearch = -1;
						colorindex = 0;
						olddvipage = dvipage;
						bBadFileComplained=0;			/* 1995/Mar/28 */
						bBadSpecialComplained=0;		/* 1996/Feb/4 */
						if (dvipage <= 1) {			/* changed 92/02/03 */ 
/*							if (bResetPage != 0) */
							if (bResetPage != 0 && bSpaceFlag == 0) { 
/*								if (xoffset != pagexoffset ||
									yoffset != pageyoffset ||
									wantedzoom != pagewantedzoom) */
								if (!sameoffsetandzoom()) {
/*									if (bDebug > 1)
									 OutputDebugString("Previous -> Top\n"); */
/*									(void) PostMessage(hWnd, WM_COMMAND, */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details */
									(void) PostMessage(hwnd, WM_COMMAND, 
										IDM_TOP, 0L);  /* 92/02/03 */
								}
							}
							if (bUserAbort != 0)  {
								bUserAbort = 0;
								InvalidateRect(hWnd, NULL, TRUE); /* ??? */
							}
							break;		/* nothing to do */
						}
						bUserAbort = 0;					/* 1995/Dec/10 */
/*						if (bCountZero != 0 && bShiftFlag != 0) */
						if (bCountZero && GetAsyncKeyState(VK_SHIFT) < 0)
							usepagetable((int) counter[0], -1); /* logical backward */
						else {
							if (bCountZero) {
								if (dvipage > 1) dvipage--;
								usepagetable(dvipage, 0);	/* page is physical page */
								if (bSpreadFlag != 0) {
									if (leftcurrent < 0 && rightcurrent < 0) {
										if (dvipage >= 1) dvipage--;	/* ??? */
/*										else if (dvipage < dvi_t) dvipage++; */
										usepagetable(dvipage, 0);	/* page is physical page */
									}
								}
							}
							else { /* bCountZero == 0 */
/*								if (bSpreadFlag == 0 || bShiftFlag != 0) */
								if (bSpreadFlag == 0 || GetAsyncKeyState(VK_SHIFT) < 0)
									dvipage--;
								else dvipage -= 2;
								if (dvipage < 1) dvipage = 1;
								usepagetable(dvipage, 0);			/* page is physical page */
							}
						}
						bControlFlag = (GetAsyncKeyState(VK_CONTROL) < 0);
/* behavior for `space' key is flipped from `Next' ??? */
/*						if (bSpaceFlag != 0) bControlFlag = ~bControlFlag; */
/* behavior for `space' key is *not* to reset scale */
						if (bSpaceFlag != 0) bControlFlag = bResetPage;
						if ((bResetPage != 0 && bControlFlag == 0) ||
							(bResetPage == 0 && bControlFlag != 0)) {
							resetpagemapping(hWnd);		/* ??? */
/*							adjusttobottom(hWnd); */	/* new 1991 Dec 28 */ 
							newmapandfonts(hWnd);
							adjusttobottom(hWnd);	/* 1996 May 17 */
							resetgraphicstate();
							graypreviousnext(hWnd);
							bEnableTermination = 1;		 /* NEW */
							InvalidateRect(hWnd, NULL, TRUE); /* erase */
						}
						else if (olddvipage != dvipage) {
							graypreviousnext(hWnd);
							bEnableTermination = 1;		 /* NEW */
							InvalidateRect(hWnd, NULL, TRUE); /* erase */
						}
					} /* end of if (bFileValid != 0 && dvipage != -INFINITY) */
					else winerror("No DVI file open");
/*					graypreviousnext(hWnd); */
					break;
				
/*				case IDM_NEXT:
					if (bControlFlag != 0) {
						xoffsetold = xoffset; yoffsetold = yoffset; 
						wantedzoomold = wantedzoom;
						resetpagemapping(hWnd);
						setscale(wantedzoom);
						resetgraphicstate(); 
						if ((bFileValid != 0 && dvipage != -INFINITY) ||
							bFontSample != 0) {
							newmapandfonts(hWnd);
						}
					} */

				case IDM_SPACE:
					if (bFileValid == 0) break;	/* 1993/Sep/20 */
					bSpaceFlag = 1;
					goto commonnext;

				case IDM_PGDN:
/*					if (bAltHomeEnd) {
						(void) PostMessage(hwnd, WM_COMMAND, IDM_BOTTOM, 0L);						
						break;
					} */
				case IDM_NEXT: 
					bSpaceFlag = 0;
				commonnext:
					if (bFileValid != 0 && dvipage != -INFINITY) {
/*	advance page number UNLESS shift down AND counter[0] used */
						lastsearch = -1;
						olddvipage = dvipage;
						bBadFileComplained=0;			/* 1995/Mar/28 */
						bBadSpecialComplained=0;		/* 1996/Feb/4 */
						if (dvipage >= dvi_t) {
/*							if (bResetPage != 0) */
							if (bResetPage != 0 && bSpaceFlag == 0) { 
/*								if (bDebug > 1)
									OutputDebugString("Next -> Bottom\n"); */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
								(void) PostMessage(hwnd, WM_COMMAND,
									IDM_BOTTOM, 0L);		/* 92/02/03 */
							}
							if (bUserAbort != 0) {		/* 1995/Dec/10 */
								bUserAbort = 0;
								InvalidateRect(hWnd, NULL, TRUE);
							}
							break;					/* nothing else to do */
						}
						bUserAbort = 0;					/* 1995/Dec/10 */
/*						if (bCountZero != 0 && bShiftFlag != 0) */
						if (bCountZero && GetAsyncKeyState(VK_SHIFT) < 0)
							usepagetable((int) counter[0], +1); /* forward logical */
						else {
							if (bCountZero) {
								if (dvipage <= dvi_t) dvipage++;	/* < dvi_t ??? */
								usepagetable(dvipage, 0);	/* page is physical page */
								if (bSpreadFlag) {
									if (leftcurrent < 0 && rightcurrent < 0) {
										if (dvipage <= dvi_t) dvipage++;	/* < dvi_t ??? */
/*										else if (dvipage > 0) dvipage--; */
										usepagetable(dvipage, 0);	/* page is physical page */
									}
								}
							}
							else {		/* bCountZero == 0 */
/*								if (bSpreadFlag == 0 || bShiftFlag != 0) */
								if (bSpreadFlag == 0 ||
									GetAsyncKeyState(VK_SHIFT) < 0)
									dvipage++;
								else dvipage += 2;
								if (dvipage > dvi_t) dvipage = dvi_t;
								usepagetable(dvipage, 0);			/* page is physical page */
							}
						}
						bControlFlag = (GetAsyncKeyState(VK_CONTROL) < 0);
/* behavior for `space' key is flipped from `Next' */
/*						if (bSpaceFlag != 0) bControlFlag = ~bControlFlag; */
/* behavior for `space' key is *not* to reset scale */
						if (bSpaceFlag != 0) bControlFlag = bResetPage;
						if ((bResetPage != 0 && bControlFlag == 0) ||
							(bResetPage == 0 && bControlFlag != 0)) {
							resetpagemapping(hWnd);		/* ??? */
/*							adjusttotop(hWnd); */	/* 1992 June 19 ??? */
							newmapandfonts(hWnd);
							adjusttotop(hWnd);  	/* 1996 April 17 */
							resetgraphicstate();
							graypreviousnext(hWnd);
							bEnableTermination = 1;		 /* NEW */
							InvalidateRect(hWnd, NULL, TRUE); /* erase */
						}
						else if (olddvipage != dvipage) {
							graypreviousnext(hWnd);
							bEnableTermination = 1;		 /* NEW */
							InvalidateRect(hWnd, NULL, TRUE); /* erase */
						}
					} /* end of if (bFileValid != 0 && dvipage != -INFINITY) */
					else winerror("No DVI file open");
/*					graypreviousnext(hWnd); */
					break;

				case IDM_FACTORY:
/* (0) work in progress here 95/Feb/13 */
/* (1) reset only things that can be set from Menu */
/* (2) check what other side-effects changing these settings should have */
					bGreyText=bGreyPlus=0;
					bShowBorder=bShowLiner=1;
					bShowBoxes=1;
					bShowButtons=bShowViewPorts=0;
					bShowTIFF=bShowWMF=1;			/* 97/Jan/7 */
					bLandScape=bSpreadFlag=0;
					bCountZero=0;
					bColorFont=0;
					bTrueInch=0;
					bIgnoreSpecial=0;
					bComplainMissing=bComplainSpecial=1;
					bComplainFiles=bCheckEncoding=1;
/*					bUseCharSpacing=0; */
					bRuleFillFlag=bUseCharSpacing=1;
					bShowPreview=bPassThrough=1;
					bViewExtras=1;
/*					bTrueTypeOnly=0; */
/*					bResetScale=0; */
/*					bResetPage=1; */
					bUseDLLs=1;				/* 99/Aug/07 */
					bConsoleOpen=1;			/* 99/Jul/23 */
					bCallBackPass=1;		/* 99/Jul/23 */
					bDontAskFile=1;			/* 99/Aug/02 */
					bOpenClosePrinter=0;	/* 99/Aug/09 */	
					bOpenCloseChannel=0;	/* 99/Jul/23 */	
					bNewPassThrough=0;		/* 99/Jul/23 */
					bOldPassThrough=0;		/* 99/Nov/30 */
					bForcePassBack=0;		/* 99/Jul/31 */
/*					ask user if Preferences should be saved later */
					bSavePrefer=0;			/* turn off bSafePrefer */
					bFactory=1;				/* note *why* bSafePrefer reset */
/*					redo menu item check boxes */
					checkpreferences(hWnd);	
					InvalidateRect(hWnd, NULL, TRUE);
					winerror("Save Preferences has been turned OFF");
					break;

/* drop through to IDM_DEFAULT ??? */

/* should consider whether scrolling is possible to save time here */

				case IDM_DEFAULT:
					if (xoffset == 0 && yoffset == 0 && wantedzoom == 0)
						break;	/* nothing to do */		
					xoffsetold = xoffset; yoffsetold = yoffset; 
					wantedzoomold = wantedzoom;
					resetmapping(hWnd, 1);		/* here we DO reset the scale */
/*					xoffset = 0; yoffset = 0; wantedzoom = 0; */
					setscale(wantedzoom);
					resetgraphicstate();
					if ((bFileValid != 0 && dvipage != -INFINITY) ||
						bFontSample != 0) {
						newmapandfonts(hWnd);
						bEnableTermination = 1;		 /* NEW */
/*						if (wantedzoomold == 0) */	
						if (wantedzoomold == wantedzoom)	/* ??? */
							scrolladjust(hWnd, xoffset - xoffsetold,  /* ? */
								yoffset - yoffsetold);				  /* ? */
						else InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
					}
/*					else winerror("No DVI file open"); */
					break;

/* should consider whether scrolling is possible to save time here */

				case IDM_TOP:
					xoffsetold = xoffset;
					yoffsetold = yoffset;
					wantedzoomold = wantedzoom;
/*					resetpagemapping(hWnd); */	/* restore page top mapping */
/*					xoffset = pagexoffset;
					yoffset = pageyoffset;
					wantedzoom = pagewantedzoom; */
/*					setscale(wantedzoom); */
/*					resetgraphicstate(); */

					adjusttotop(hWnd); 		/* adjust to top of page ??? */
					if ((bFileValid != 0 && dvipage != -INFINITY) ||
						bFontSample != 0) {
/*						newmapandfonts(hWnd); */	/* ??? */
						bEnableTermination = 1;		 /* NEW */
/*						if (wantedzoomold == 0) */
						if (wantedzoomold == wantedzoom)	/* ??? */
							scrolladjust(hWnd, xoffset - xoffsetold,  /* ? */
								yoffset - yoffsetold);
						else InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
					}
/*					else winerror("No DVI file open"); */
					break;

				case IDM_BOTTOM:
					xoffsetold = xoffset; yoffsetold = yoffset; 
					wantedzoomold = wantedzoom; 
/*					resetpagemapping(hWnd);	 */
/*					xoffset = pagexoffset; yoffset = pageyoffset; */
/*					wantedzoom = pagewantedzoom; */ /* 1991 Dec 27 */
/*					setscale(wantedzoom); */		/* 1991 Dec 27 */
/*					resetgraphicstate(); */

					adjusttobottom(hWnd);		/* adjust to bottom of page */
					if ((bFileValid != 0 && dvipage != -INFINITY) ||
						bFontSample != 0) {
/*						newmapandfonts(hWnd); */	/* 1991 Dec 27 */
						bEnableTermination = 1;		/* NEW */
/*						if (wantedzoomold == 0) */
						if (wantedzoomold == wantedzoom)  /* 1991 Dec 27 */
							scrolladjust(hWnd, xoffset - xoffsetold, /* ? */
								yoffset - yoffsetold);
						else InvalidateRect(hWnd, NULL, TRUE); 
					}
/*					else winerror("No DVI file open"); */
					break;

				case IDM_MAGNIFY:
					if ((bFileValid != 0 && dvipage != -INFINITY) ||
						bFontSample != 0) {
						bUserAbort = 0;					/* 1995/Dec/10 */
						if (wantedzoom < MAXZOOM) {
							hDC = GetDC(hWnd);
							(void) SetMapMode(hDC, MM_TWIPS);	/*  twips */
							(void) GetClipBox(hDC, &ClipRect);
/*							if (bShiftFlag != 0) */
							if (GetAsyncKeyState(VK_SHIFT) < 0) {
								savegraphicstate();	/* 92/02/03 */
								(void) redomapping(wantedzoom+4,
									ClipRect, ClipRect);
							}
							else {
								(void) redomapping(wantedzoom+1,
									ClipRect, ClipRect);
							}
							(void) ReleaseDC(hWnd, hDC);
							newmapandfonts(hWnd);
							checkmagnify(hWnd); /* 1993/March/27 */
/* 							if (wantedzoom >= MAXZOOM) {	

								(void) EnableMenuItem(hMenu, IDM_MAGNIFY, 
									MF_GRAYED);
							} */
							bEnableTermination = 1;
							InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
						}
					} /* end of if (bFileValid != 0 && dvipage != -INFINITY) */
					else winerror("No DVI file open");
					break;

				case IDM_UNMAGNIFY:		
					if ((bFileValid != 0 && dvipage != -INFINITY) ||
						bFontSample != 0) {
						bUserAbort = 0;					/* 1995/Dec/10 */
						if (wantedzoom > MINZOOM) {
							hDC = GetDC(hWnd);
							(void) SetMapMode(hDC, MM_TWIPS);	/*  twips */
							(void) GetClipBox(hDC, &ClipRect);
/*							if (bShiftFlag != 0) */
							if (GetAsyncKeyState(VK_SHIFT) < 0) {
								savegraphicstate();	/* 92/02/03 */
								(void) redomapping(wantedzoom-4,
									ClipRect, ClipRect);
							}
							else {
								(void) redomapping(wantedzoom-1,
									ClipRect, ClipRect);
							}
							(void) ReleaseDC(hWnd, hDC);
							newmapandfonts(hWnd); 
							checkmagnify(hWnd);	/* 1993/March/27 */
/*							if (wantedzoom <= MINZOOM) {
								hMenu = GetMenu(hWnd);
								(void) EnableMenuItem(hMenu, IDM_UNMAGNIFY, 
									MF_GRAYED);	
							} */
							bEnableTermination = 1;
							InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
						}
					} /* end of if (bFileValid != 0 && dvipage != -INFINITY) */
					else winerror("No DVI file open");
					break;
			
			case IDM_CURRENTPAGE:					/* 96/Oct/4 */
		    currentpage:
				bSinglePage = 1;
				bPrintOnly = 1;						/* ??? */
/*				frompage = dvipage; */
				beginpage = dvipage;
/*				topage = dvipage; */
				endpage = dvipage;
/*				What if we haven't set up any printer yet ? */
/*				if (DoPrintSetup(hwnd) == 0) break; */ /* ??? */
				goto commonprint;

			case IDM_CTRLP:
				if (GetAsyncKeyState(VK_SHIFT) < 0) {
					goto currentpage;
				}	/* otherwise just drop through */
			case IDM_PRINTALL:						/* Print File */
				bSinglePage = 0;
				bPrintOnly = 0;						/* ??? */
			commonprint:
/*				if (bPrintOnly == 0) */
/*					UpdateWindow(hWnd);	*/			/* try and update NOW */
/*					SetRectEmpty(&UpdateRect);		*//* 1992/Apr/12 */
					setupregions();					/* 1992/Apr/12 */
/*				} */

				if (bFileValid == 0) {	/*  && bFontSample == 0 */
/*					winerror("No DVI file open"); */
					bBusyFlag = 0;
					break;
				}

				if (bBusyFlag != 0) break; 	
/*				bBusyFlag++; */			/* too early ? */
/*				(void) EnableWindow(hWnd, FALSE); */	/* experiment */
				(void) DoPrintAll(hWnd);		/* in winprint.c */
				bBusyFlag = 0;					/* too late ? */
/*				(void) EnableWindow(hWnd, TRUE); */	/* enable main Window */

/*				if (bPrintOnly == 0) */
/*					if (IsRectEmpty(&UpdateRect) == 0)	
						InvalidateRect(hWnd, &UpdateRect, TRUE); */
					forceregion(hWnd);			/* 1992/Apr/12 */
/*				} */
				break;

			case IDM_PRINTVIEW:					/* Print View */
				bSinglePage = 0;
				bPrintOnly = 0;						/* ??? */
/*				UpdateWindow(hWnd); */			/* try and update NOW */
/*				SetRectEmpty(&UpdateRect); */	/* 1992/Apr/12 */
				setupregions();					/* 1992/Apr/12 */

				if (bFileValid == 0 && bFontSample == 0) {
/*					winerror("No DVI file open"); */
					bBusyFlag = 0;
					break;
				}
/*				(void) EnableWindow(hWnd, FALSE); */	/* AN EXPERIMENT */

				if (bBusyFlag != 0) break;	
/*				bBusyFlag++; 	*/	/* too early ? */
/*				(void) EnableWindow(hWnd, FALSE); */	/* 1992/Nov/2 experiment */
//				(void) DoPrinting(hWnd);			/* in winprint.c */
				(void) DoPrintStuff(hWnd, 0);		/* in winprint.c */
				bBusyFlag = 0;				/* redundant */
/*				(void) EnableWindow(hWnd, TRUE); */	/* enable main Window */

/*				if (IsRectEmpty(&UpdateRect) == 0)	
					InvalidateRect(hWnd, &UpdateRect, TRUE); */
				forceregion(hWnd);			/* 1992/Apr/12 */
				break;

			case IDM_PRINTSETUP:
/*				hOldPrintDC = hPrintDC;	*/	/* save old DC if any */
/*				hPrintDC = NULL;		*/  /* pretend there isn't one */
/*				if (DoPrintSetup(hWnd) == 0) {
					hPrintDC = hOldPrintDC;
				}
				else if (hOldPrintDC != NULL) {
					(void) DeleteDC(hOldPrintDC);
					hOldPrintDC = NULL;
				} */
/* above assumed that DoPrintSetup does not actually CreateDC ??? */
				(void) DoPrintSetup(hWnd);	/* OLD or NEW */
				break;

			case IDM_ZOOM:
				if (bFileValid == 0 && bFontSample == 0)
					break;				/* nothing to do ... */
				if (bZoomFlag < 0) {	/* a way to cancel ZOOM */
					bZoomFlag = 0; 
/*					(void) SetCursor(hSaveCursor); */
					(void) SetCursor(hOldCursor);
/*					ReleaseCapture(); */
					break;
				}
				bZoomFlag = -1;			/* now wait for WM_LBUTTONDOWN */
/*				hSaveCursor = SetCursor(hZoomCursor);*/ /* switch to magn */
				hOldCursor = SetCursor(hZoomCursor); /* switch to magn */
/*				(void) SetCapture(hWnd); */ /* grab the mouse */
				break;
				
			case IDM_OLDVIEW:
			oldview:			/* can get here on right mouse click also */
				if (bFileValid == 0 && bFontSample == 0)
					break;	/* nothing to do ... */
				bUserAbort = 0;					/* 1995/Dec/10 */
				if (restoregraphicstate() != 0) {
					newmapandfonts(hWnd);		/* need to do more ??? */
					checkmagnify(hWnd);			/* 1993/May/8 */
					bEnableTermination = 1;		/* NEW */
					InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
				}
				break;

			case IDM_HELP:
/*				if (!bHelpAtEnd) break; */
				if (bDebug > 1) OutputDebugString("Help!\n");
				CallHelp(hWnd);					/* 1995/Sep/18 */
				break;

			case IDM_BACK:						// 99/Aug/21
			case IDM_JUMPBACK:					/* 1996/Feb/25 ??? */
				if (bDebug > 1) OutputDebugString("Jump Back\n");
				if (bFileValid == 0 && bFontSample == 0)
					break;				
				(void) PostMessage(hwnd, WM_RBUTTONDOWN, 0, 0L);
				break;

			case IDM_FORWARD:					// 99/Aug/21 implement ???
				break;

			default:
/*				check whether it is a TeX Menu item */
				if (id >= IDM_APPLICATION && id < IDM_APPLICATION + 100) {
/*				if (id >= IDM_APPLICATION &&
					id < IDM_APPLICATION + (WORD) napplications) */
					bShiftFlag = (GetAsyncKeyState(VK_SHIFT) < 0);
					bControlFlag = (GetAsyncKeyState(VK_CONTROL) < 0);
/* ignore control key if called by hot key */
/*					if (lParam != 0) bShiftFlag = 0; */ /* NO */
					if (lParam != 0) bControlFlag = 0;
/*	if hot key used before TeX Menu pulled down 95/Oct/5 */
/*					if (!bTeXMenuOK) {	
						filltexmenu(hwnd);
						bTeXMenuOK=1;
					} */ /* too late here ! will never get here */
//	flag -1 if it wants a file to be opened (e.g. click TeX > Preview)
//  flag -2 if it is just the help file being opened
					flag = callapplication(hWnd, id - IDM_APPLICATION,
						bShiftFlag, bControlFlag);
//					don't add this type of open to MRU ??					
					if (flag == -2) bAddToMRU = 0;	// just the help file
					if (flag < 0) goto idmopen;		/* go open a file */
					bAddToMRU = 1;			/* set back to default ? */
					break;			/* above is in winsearc.c */
				}
/*	check whether a listed file in `File' menu 95/Sep/17 */
				if (id >= IDM_DOCUMENTS && id < IDM_DOCUMENTS + 16) {
					bShiftFlag = (GetAsyncKeyState(VK_SHIFT) < 0);
					bControlFlag = (GetAsyncKeyState(VK_CONTROL) < 0);
					flag = calldocument(id - IDM_DOCUMENTS,
										bShiftFlag, bControlFlag);
					bAddToMRU = 0;			/* ??? */
					if (flag < 0) goto idmopen;			/* go open a file */
					bAddToMRU = 1;			/* set back to default ? */
					break;
				}
				if (id >= IDM_ENVIRONMENT && id < IDM_ENVIRONMENT + 100) {
					bShiftFlag = (GetAsyncKeyState(VK_SHIFT) < 0);
					bControlFlag = (GetAsyncKeyState(VK_CONTROL) < 0);
					flag = editenvironment(id-IDM_ENVIRONMENT,
									bShiftFlag, bControlFlag);									
					
					ReadEnvVars();	/* in case env vars changed 98/Dec/24 */
					ReadPaths();	/* in case env vars changed 98/Dec/24 */
					SetupEncodingMenu(hwnd, 0);	/* in case vars changed */
					break;
				}
				break;	/* ? */
			}			// end of switch(id) WM_COMMAND 
			break;

/* this is the end of WM_COMMAND -- Phew! */

			case WM_INITMENUPOPUP:
			{
				HMENU hMenuPopup;
				UINT uPos;
				BOOL fSystemMenu;
/*				hmenuPopup = (HMENU) wParam;       
				nIndex = (int) LOWORD(lParam);        
				fSystemMenu = (BOOL) HIWORD(lParam);  */	/* WIN16 */
				hMenuPopup = (HMENU) wParam;         /* handle of pop-up menu */
				uPos = (UINT) LOWORD(lParam);        /* pop-up item position */
				fSystemMenu = (BOOL) HIWORD(lParam); /* System menu flag win32 */

				nIndex = (int) LOWORD(lParam);
				hMenu = GetMenu (hwnd);
				nMenu = GetMenuItemCount (hMenu);
/* or use hmenuPopup retrieved from wParam / lParam ? */
/*				if (!bApplications) break; */			/* 95/Sep/17 */
				if (bApplications) {
/*					hMenuPopUp = (HMENU) wParam; */
/*					nIndex = (int) LOWORD(lParam); */
/*					fSystemMenuu = (BOOL) HIWORD(lParam); */
/*					hMenu = GetMenu (hwnd); */
/*					nMenu = GetMenuItemCount (hMenu); */
/*					if (nIndex != nMenu - 1) break; */	/* 95/Sep/17 */
					if ((!bHelpAtEnd && (nIndex == nMenu - 1)) ||
						(bHelpAtEnd && (nIndex == nMenu - 2))) {
						if (checktexmenu(hwnd) != 0) {
							cleartexmenu(hwnd);
							filltexmenu(hwnd);
/*							bTeXMenuOK=1; */
						}
					}
/*					return(0); */			/* processed this message */
				}
/*				also check whether nIndex == 0 for RememberDocs */
				if (bRememberDocs) {					/* 95/Sep/17 */
					if (nIndex == 0) {
						if (checkdocmenu(hwnd) != 0) {	
							cleardocmenu(hwnd);
							filldocmenu(hwnd);
						}
					}
				}
				if (bEnvironment && hMenuPopup == hEnvMenu) {
					if (checkenvmenu(hwnd) != 0) {
						clearenvmenu(hwnd);
						fillenvmenu(hwnd);
					}
				}
/*				may want to set up ENCODING popup here also ? */
				break;
			} /* end of WM_INITMENUPOPUP */

			case WM_SYSCOLORCHANGE:
				TextColor = GetSysColor(COLOR_WINDOWTEXT);
				BkColor = OrgBkColor = GetSysColor(COLOR_WINDOW);
/* somewhat risky if pen or brush selected into current device context ? */
				MakeRuleDefault(TextColor);
/* also need to redo stack of background colors ? */
				ResetBack(OrgBkColor, MaxBack);
/*				return(0);	*/	/* supposed to return zero */
				break; 

#ifdef USEPALETTE
/* new code 1996/Mar/24 attempt to deal with palette color issues */
			case WM_QUERYNEWPALETTE:
/* Windows sends this message to window about to become active */
/* application should realize its own logical palette if any */				
/* invalidate contents of client window's area, */
/* and return TRUE to inform Windows that it has changed the system palette */
				if (bUsePalette && hPal != NULL) {
					hDC = GetDC(hWnd);
					hOldPal = SelectPalette (hDC, hPal, 0);
/* hOldPal == NULL if SelectPalette failed */
					flag = RealizePalette (hDC);
					ReleaseDC (hWnd, hDC);
/* if palette realization causes palette change, we need to do a full redraw */
					if (flag) {	/* any colors got remapped ? */
						InvalidateRect (hWnd, NULL, TRUE);
						nUpdateCount = 0;
						return (TRUE);
					}
					else return (FALSE);
				}
				break;
#endif

#ifdef USEPALETTE
/* new code 1996/Mar/24 attempt to deal with palette color issues */
			case WM_PALETTECHANGED:
/* Windows sends this message to overlapped and pop-up windows when the */
/* active window changes the system palette by realizing its logical palette */
/* wParam contains handle of the window that is doing this */
/* first check if this is our window to avoid getting trapped in a loop */
/* Then, have three options: (i) do nothing (fast, but colors will be wrong) */
/* (ii) realize logical palette and redraw client area (slow but accurate) */
/* (iii) realize logical palette and irectly update colors in client area */
/* using UpdateColors() - this is a compromise, fast but nor accurate */
				if ((HWND) wParam != hWnd) {	/* ignore if this window */
					if (bUsePalette && hPal != NULL) {
						hDC = GetDC (hWnd);
						hOldPal = SelectPalette (hDC, hPal, 0);
/* hOldPal == NULL if SelectPalette failed */
						flag = RealizePalette (hDC);
						if (flag) {	/* any colors got remapped ? */
							if (bUpdateColors) {
								UpdateColors(hDC);
								nUpdateCount++;;
/* could use threshold on nUpdateCount to force InvalidateRect after too */
/* too many UpdateColors() calls, since quality will decrease each time */
							}
							else InvalidateRect (hWnd, NULL, TRUE);
						}
						ReleaseDC (hWnd, hDC);
					}
				}
				break;
#endif

			case WM_FONTCHANGE:
/* Throw out old list of fonts and force redoing them next time */
/* unless we are in the middle of showing font samples ... */
				if (bFontSample == 0 &&	bWriteAFM == 0) {
					if (hFaceNames != NULL) FreeFaceNames();
				}
//				redisplay ? in case missing font just installed
				InvalidateRect(hWnd, NULL, TRUE);	// 2000/Jan/9
//				some question of whether we need to do more ?
//				for example, styles set to UNKNOWN for these fonts ?
				break;
			
/*			case WM_DEVMODECHANGE: */
/* printer environment has changed for device specified in lParam */
/* throw out old DEVMODE and force redoing this next time ? */
/* maybe only if using default settings ? */
/*				if (strcmp( (LPSTR) lParam, achDevice) == 0) {
					if (bBusyFlag == 0 && hPrintDC != NULL) {
						(void) DeleteDC(hPrintDC);
						hPrintDC = NULL;
					}
				}
				break; */

			case WM_TIMER:
/*  First check all sorts of reasons NOT to be interested in Timer */
				if (bBusyFlag != 0) break;
				if (dragflag != 0) break;
				if (bFontSample != 0) break;
				if (bFileValid == 0) break;
				if (bPrintFlag != 0) break;
				if (bCopyFlag != 0) break;
/*  Ignore timer if iconic - turn timer off in that case ? */
				if (IsIconic(hwnd) != 0) break;
/*  Following should never happen 93/July/23 */
				if (IsWindowVisible(hwnd) == 0) break;
/*  Do not attempt to open file if DVIPSONE is busy with it */
/*				if (hTaskWindow != NULL) */		/* 93/Jan/5 */
				if (TaskInfo.hWnd != NULL) {		/* 95/Mar/12 */
/*					if (IsTaskActive(hwnd)) break; */
					if (IsTaskActive()) break;
/*					else InvalidateRect(hWnd, NULL, TRUE); */ 
				}
				if (nAlarmTicks > 0) {	/* 94/Aug/10 */
					if (nAlarmTicks >= (UINT) nRefreshDelay) {
						nAlarmTicks = 0;
						bAwakened = 1;
/*	should we copy FileSize and FileTime here to avoid double refresh ? */
						InvalidateRect(hWnd, NULL, TRUE);
					}
					else nAlarmTicks++;
					break;
				}
/*  should perhaps also do above for DVIPSONE called from TeX Menu ? */
/*  should perhaps also do above for TeX called from TeX Menu ? */
/*	do nothing if file has (temporarily ?) disappeared */ 
/*  following may lead to sharing violation without OfShareCode */
#ifdef LONGNAMES
/*				strcpy(FileName, DefPath); */
				if (strchr(OpenName, '\\') != NULL ||
					strchr(OpenName, '/') != NULL ||
					strchr(OpenName, ':') != NULL) *FileName = '\0';
				else strcpy(FileName, DefPath);
				strcat(FileName, OpenName);
/*				hFileTemp = _lopen(FileName, READ | OF_SHARE_DENY_NONE); */
				hFileTemp = _lopen(FileName, READ | OfExistCode); /*96/May/18*/
				if (hFileTemp == HFILE_ERROR) break;
				else {
					_lclose(hFileTemp);
					hFileTemp = HFILE_ERROR;		/* 96/May/18 */
				}
/*				strcpy(FileName, DefPath); */
				if (strchr(OpenName, '\\') != NULL ||
					strchr(OpenName, '/') != NULL ||
					strchr(OpenName, ':') != NULL) *FileName = '\0';
				else strcpy(FileName, DefPath);
				strcat(FileName, OpenName);
				hFile = _lopen(FileName, READ | OfShareCode);
#else
				if (OpenFile((LPSTR) NULL, &OfStruct, 
/*					OF_REOPEN | OF_EXIST) < 0) break; */
					OF_REOPEN | OF_EXIST | OF_SHARE_DENY_NONE) == HFILE_ERROR)
						break; 
/*	repaint screen if the file date or time have changed */
				hFile = OpenFile((LPSTR) NULL, &OfStruct, 
/*					OF_REOPEN); */
/*					OF_SHARE_DENY_WRITE | OF_REOPEN); */
/*					OF_SHARE_DENY_NONE | OF_REOPEN); */
					OF_REOPEN | OfShareCode); 
/* add OF_SHARE_DENY_READ, OF_SHARE_DENY_WRITE, OF_SHARE_EXCLUSIVE ??? */
#endif
				if (hFile == HFILE_ERROR) {
/*	maybe check GetLastError or OfStruct.nErrCode at this point ? */
/*					InvalidateRect(hWnd, NULL, TRUE); */
				}
/* It did open, now check whether it is the same file */
/* try using OF_VERIFY instead ? */ /* or OfStruct.reserved[] */
				else {							/* or use fstat */
/*					if (hFile >= 0) */
					if (hFile != HFILE_ERROR) {

						NewFileSizeLow = GetFileSize ((HANDLE) hFile, &NewFileSizeHigh);
						(void) GetFileTime ((HANDLE) hFile, NULL, NULL, &NewFilewTime);
/*						if (bDebug > 1) ShowFileInfo("WM_TIMER", NewFileSizeHigh, NewFileSizeLow,
				 NewFilewTime.dwHighDateTime, NewFilewTime.dwLowDateTime); */

/*						(void) _fstat(hFile, &FileStatus); */

						(void) _lclose(hFile);
/*						hFile = -1; */
						hFile = HFILE_ERROR;
					} /* end of hFile OK */
					else {							/* error !!! */
						wincancel("File already closed");
					}
					if (NewFileSizeLow < 128 && NewFileSizeHigh == 0)

/*					if (FileStatus.st_size < 128) */

					{
/*						sprintf(str, "Size %ld", FileStatus.st_size);
						winerror(str); */
/*	if file is short, it is being written to - DONT try and read it then */
					}
					else {
						if (FileSizeLow != NewFileSizeLow ||
							FileSizeHigh != NewFileSizeHigh ||
							FilewTime.dwLowDateTime != NewFilewTime.dwLowDateTime ||
							FilewTime.dwHighDateTime != NewFilewTime.dwHighDateTime) 
/* or, use CompareFileTime(FilewTime, NewFilewTime) != 0 */

/*						if (FileSize != FileStatus.st_size ||
							FilemTime != FileStatus.st_mtime)  */
						{
						if (bDebug > 1) {
							OutputDebugString("Timer: ");
							OutputDebugString("File Size or Time have changed\n"); 
						}
/*  To tell it that file reopen is result of change in file status */
						if (nRefreshDelay == 0) {
/*  should we copy FileSize and FileTime here to prevent double refresh ? */
							bAwakened = 1;					/* 1993/July/12 */
/*	We already have an InvalidateRect here, don't need later in ReOpenFile */
							InvalidateRect(hWnd, NULL, TRUE);
						}
/*	don't refresh quite yet if nRefreshDelay is non-zero */
						else nAlarmTicks = 1;	/* 1994/Aug/10 */
						}  /* end of if mismatch in time or size */
					}	/* end of file is not less than 128 */
				}	/* end of if HFILE OK */
				break;		/* end of WM_TIMER tick */

			case WM_KEYDOWN:
/* be more selective - look for shift key only */ /* not used ? */
/* may need rework in Windows NT */ /* WIN32 ??? */
/*				if (wParam == VK_CANCEL) PostQuitMessage(0); */ /* needed ? */
/*				if (wParam == VK_ESCAPE) PostQuitMessage(0); */ /* needed ? */
				if (wParam == VK_SHIFT) bShiftFlag = 1;
				else if (wParam == VK_CONTROL) bControlFlag = 1; 
/*				else if (wParam == VK_MENU) altflag = 1; */
				nVKey = wParam;
				dwKeyData = (DWORD) lParam;
/*				if (bDebug > 1) {
					sprintf(debugstr, "DOWN: VK %d KD %lx\n", nVKey, dwKeyData);
					OutputDebugString(debugstr);
				} */ /* too much traffic */
				if (nVKey >= VK_F1 && nVKey <= VK_F24) {	/* 1993/Dec/25 */
/*					bShiftFlag = (GetAsyncKeyState(VK_SHIFT) < 0); */ /* NO */
					bControlFlag = (GetAsyncKeyState(VK_CONTROL) < 0);
					nIndex = translatehotkey(hWnd, nVKey, bControlFlag);
					if (bDebug > 1) {
						sprintf(debugstr, "VK %d nIndex %d\n", nVKey, nIndex);
						OutputDebugString(debugstr);
					}
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
					if (nIndex >= 0) PostMessage(hWnd, WM_COMMAND,
/*							IDM_APPLICATION + nIndex, 0L); */
							IDM_APPLICATION + nIndex, 1L);
					break;
				}
/* application should return zero if it processes this message */
				break;
				
			case WM_KEYUP:
/* be more selective - look for shift key only */ /* not used */
/* may need rework in Windows NT */ /* WIN32 ??? */
				if (wParam == VK_SHIFT) bShiftFlag = 0;
				else if (wParam == VK_CONTROL) bControlFlag = 0;
/*				else if (wParam == VK_MENU) altflag = 0; */
/* application should return zero if it processes this message */
				break;

/* New, used to go to specified page number */

			case WM_CHAR:								/* 1993/Nov/10 */
				nVKey = wParam;
				dwKeyData = (DWORD) lParam;
				if (bDebug > 1) {
					sprintf(debugstr, "CHAR: VK %d KD %lx\n", nVKey, dwKeyData);
					OutputDebugString(debugstr);
				}
				if (bFileValid == 0) break;
				if (nVKey == '\r') {	/* return key pressed */
					bUserAbort = 0;				/* 1995/Dec/10 */
					if (bPageNumber) {
						olddvipage = dvipage;	/* remember this ? */
						newpage = gopage;		/* tell it where to go */
						gopage = 0;				/* reset for next time */
						bPageNumber = 0;		/* 1994/Mar/22 */
						goto selectcommon;
					}
					else {	/* otherwise treat as screen refresh 94/Mar/24 */
/*						(void) PostMessage(hwnd, WM_LBUTTONDBLCLK, 0, 0L); */
						bEnableTermination = 1;
						InvalidateRect(hWnd, NULL, TRUE);	/* 98/Nov/5 */
					}
				}	
				else if (nVKey >= '0' && nVKey <= '9') {
					gopage = gopage * 10 + (nVKey - '0');
					bPageNumber = 1;		/* initicate keyed 94/Mar/22 */
				}
/* application should return zero if it processes this message */
				break;

/* New, use this to pop back from hypertext linkage */
/* New, use this to pop back from zooming */

			case WM_RBUTTONDOWN:
				if (bFileValid == 0 && bFontSample == 0)
					break;
/*				if (bZoomFlag != 0 || bRectFlag != 0) break; */
/* lock cursor to bottom right corner when right button is pushed */
/*				if (bZoomFlag != 0) {
					CurPoint.x = zoomendx; CurPoint.y = zoomendy;
					ClientToScreen(hWnd, &CurPoint);
					SetCursorPos(CurPoint.x, CurPoint.y);
					bCursorMoved = 1;
				} */
/*				if (bZoomFlag != 0 || bClipFlag != 0 || bRuleFlag != 0) break; */
/*				Ignore if busy defining rectangle ... */
				if (bZoomFlag != 0 || bRectFlag != 0 || bRuleFlag != 0) {
					if (bDebug > 1)
						OutputDebugString ("Ignored\n"); /* debug 95/Mar/31 */
					break;
				}
/* if no saved up graphic states, switch back to hyper text jumps, if any */
				if (graphicindex == 0 && hyperindex > 0)	/* 95/Aug/12 */
					bHyperLast = 1;
/* jumpback: */		/* 1996/Feb/25 ??? */
/*			    if (bHyperText != 0 && hyperindex > 0) */ /* 95/Aug/12 */
			    if (bHyperText && hyperindex > 0 && bHyperLast) {
					if (bBusyFlag++ > 0) {
						if (bDebug > 1)	OutputDebugString("Busy Search\n");
						bSearchFlag = 0;
						break;		/* prevent reentry */
					}
					if (searchandmark(hWnd, -1) != 0) { /* hyper pop search */
						bBusyFlag = 0;
						graypreviousnext(hWnd);
						bEnableTermination = 1;		/* NEW */
/*						if (bDebug>1) OutputDebugString("Search non zero\n");*/
						InvalidateRect(hWnd, NULL, TRUE);
					}
					else {
						bBusyFlag = 0;
/* should not need to redraw in this case, but ... */
					}
				}
/*	1993/Aug/26 right mouse button => unzoom */ /* IDM_OLDVIEW */
				else if (graphicindex > 0) goto oldview;
				break; 

//			case WM_RBUTTONUP:
//				winerror("Right Button Up");	// debugging only
//				if (bFileValid == 0 && bFontSample == 0) break;
//				if (bZoomFlag > 0) zoomdragflag = 0;
//				break; 

			case WM_LBUTTONDOWN:
				if (bFileValid == 0 && bFontSample == 0)
					break;
/*		may need rework in Windows NT */ /* WIN32 ??? */
/*		start with ZOOM and COPY rectangle definition */ 
/*		can either start from menu, or by pressing control or control+shift */
				ControlState = GetAsyncKeyState(VK_CONTROL);
				ShiftState = GetAsyncKeyState(VK_SHIFT);
				if ((bZoomFlag < 0) || (bRectFlag < 0) ||
/*						ControlState < 0 ||	 */			/* 1994/Mar/7 */
					(ShiftState == 0 && ControlState < 0) ||
/*					    (bShowUsedExposed == 0 && ControlState < 0) ||   */
					    (! bShowUsedExposed && ControlState < 0) ||
/*					(! bShowUsedExposed && ! bFontSample &&
					 (ControlState < 0 || ShiftState < 0))) */
/*						(bShowUsedExposed == 0 && ShiftState < 0)) */
						(! bShowUsedExposed && ShiftState < 0))
				{

					pts = MAKEPOINTS(lParam);
					zoomendx = zoomstartx = pts.x;
					zoomendy = zoomstarty = pts.y;

/*					zoomendx = zoomstartx = LOWORD(lParam); */
/*					zoomendy = zoomstarty = HIWORD(lParam); */

					if (bRectFlag < 0) bRectFlag = 1;
					else if (bZoomFlag < 0) bZoomFlag = 1;
					else if (bRuleFlag < 0) bRuleFlag = 1;
					else if (ControlState < 0 && ShiftState < 0 ) bRectFlag = 1;
					else if (ControlState < 0) bZoomFlag = 1;
					else if (ShiftState < 0) bRuleFlag = 1;

					if (bZoomFlag != 0) {	/* remember screen aspect ratio */
						GetClientRect(hWnd, &ClientRect);

						ZHeight = (ClientRect.bottom - ClientRect.top);
						ZWidth = (ClientRect.right - ClientRect.left);

/*						ZHeight = (WORD) (ClientRect.bottom - ClientRect.top);
						ZWidth = (WORD) (ClientRect.right - ClientRect.left); */

/*						GetWindowRect(hWnd, &WinRect); 
						ZLeft = WinRect.left;
						ZTop = WinRect.top; */
/*						bCursorMoved = 0; */
					}

					if (bRuleFlag != 0) {

						pts = MAKEPOINTS(lParam);
						curx = pts.x;
						cury = pts.y;

/*						curx = LOWORD(lParam);
						cury = HIWORD(lParam); */

						createrulewindows(hWnd, curx, cury);
					}

/*					if (bZoomFlag != 0) hSaveCursor = SetCursor(hZoomCursor);
					else if (bRectFlag != 0) hSaveCursor = SetCursor(hCopyCursor);
					else if (bRuleFlag != 0) hSaveCursor = SetCursor(hBlankCursor); */
					if (bZoomFlag != 0) hOldCursor = SetCursor(hZoomCursor);
					else if (bRectFlag != 0) hOldCursor = SetCursor(hCopyCursor);
					else if (bRuleFlag != 0) hOldCursor = SetCursor(hBlankCursor);
					(void) SetCapture(hWnd);			 /* grab the mouse */
				} /* end of if ((bZoomFlag < 0) || (bRectFlag < 0) || ... */

/*		start dragging image if Alt key is down */
				else if (GetAsyncKeyState(VK_MENU) < 0) {
					pts = MAKEPOINTS(lParam);
					dragcurrentx = dragstartx = pts.x;
					dragcurrenty = dragstarty = pts.y;

/*					dragcurrentx = dragstartx = LOWORD(lParam);
					dragcurrenty = dragstarty = HIWORD(lParam); */

					setupregions();
					dragflag = 1;
/*					hSaveCursor = SetCursor(hHandCursor); *//* switch to hand */
					hOldCursor = SetCursor(hHandCursor);  /* switch to hand */
					(void) SetCapture(hWnd); /* grab the mouse */
				}
/*				next see whether shift clicking on text to see what font it is */
/*				else if (bShowUsedExposed != 0 && bFileValid != 0 &&
					GetAsyncKeyState(VK_SHIFT) < 0) */
				else if ((bShowUsedExposed && bFileValid &&
						  GetAsyncKeyState(VK_SHIFT) < 0) ||
						 (! bShowUsedExposed && bFontSample)) {	/* 99/Jan/10 */

					if (bCharacterSample) {		// 2000 June 24
						bCharacterSample = 0; /* AND THEN ? */
						InvalidateRect(hWnd, NULL, TRUE);
						break;
					}
					pts = MAKEPOINTS(lParam);
					curx = (int) pts.x;
					cury = (int) pts.y;					

/*					curx = (int) LOWORD(lParam); */
/*					cury = (int) HIWORD(lParam); */

					bMarkSearch = 0;			/* ??? */
//	In font sample mode figure out which character was clicked 99/Jan/10 
					if (bFontSample && ! bCharacterSample) {
//	First time left mouse button down only ! 00/Jun/23
//					if (bFontSample && ! bCharacterSample && nChar < 0) 
						if ((nChar = ShowWhatChar(hWnd, curx, cury)) >= 0) {
							if (bDrawOutline) {
								InvalidateRect(hWnd, NULL, TRUE);
								break;			/* 99/Jan/20 */
							}
						}
					}
/* if Shift-Ctrl => show full pedigree of character */
					else if (GetAsyncKeyState(VK_CONTROL) < 0) {
						flag = TagFont(hWnd, curx, cury, 1);
						if (flag >= 0) { /*94/Mar/7*/
							setupcharinfo();
							if ((hFocus = GetFocus()) == NULL)
								hFocus = hWnd;
							(void) MessageBox(hFocus, str,
								"Character Info", 
								MB_ICONINFORMATION | MB_OK);
						}
/*						markfont = -1; */	/* prevent reverse video */
					} /* end of if CTRL key */
/* if only Shift then reverse video everything using this font redraw */
					else {
						flag = TagFont(hWnd, curx, cury, 0);
						if (flag >= 0) {
/*					sprintf(str, "font %d", flag);	winerror(str); */
							bEnableTermination = 1;		 /* NEW */
							InvalidateRect(hWnd, NULL, TRUE);
						}
					}
				} /* end of bShowUsedExposed && bFileValid && SHIFT */
/*		finally see whether user is trying to push a HYPER TEXT button */
				else if (hClientWindow != NULL && bFileValid != 0) {
					if (bSearchFlag++ == 0) {
						if (bDebug > 1)	OutputDebugString("Client Window\n");

						pts = MAKEPOINTS(lParam);
						curx = pts.x;
						cury = pts.y;

/*						curx = LOWORD(lParam); */
/*						cury = HIWORD(lParam); */

						if (bBusyFlag++ > 0) {
							bSearchFlag = 0;	/* 1993/March/29 */
							break;		/* prevent reentry */
						}
						mark = checkmarkhit(hWnd, curx, cury);
						bBusyFlag = 0;		/* 1993/March/29 */
						bSearchFlag = 0;	/* 1993/March/29 */
						if (mark > 0)
							PostMessage(hClientWindow, WM_MARKHIT, mark, 0L);
					}
				}
/*				else if (bHyperText != 0 && bFileValid != 0) */
				else if (bHyperUsed && bFileValid) {	/* 94/Oct/5 */
/*	check first whether this page has any hyper text linkage ... */
					if (bSearchFlag++ == 0) {
/*						if (bDebug > 1)
							OutputDebugString("Hyper Text Button?\n"); */

						pts = MAKEPOINTS(lParam);
						curx = pts.x;
						cury = pts.y;

/*						curx = LOWORD(lParam); */
/*						cury = HIWORD(lParam); */

						if (bBusyFlag++ > 0) {	/* do we need it here ? */
							if (bDebug > 1)	OutputDebugString("Hyper Busy\n");
							bSearchFlag = 0;	/* 1993/March/29 */
							break;		/* prevent reentry */
						}

						*str = '\0';		/* clear it out 94/Dec/13 */
						mark = checkmarkhit(hWnd, curx, cury);
/* checkmarkhit() drops button name (if any) in buttonlabel[] */
/* checkmarkhit() drops the rest of the \special string into str[] */
/* marknumber is -1 if nothing hit, 0 if hit labelled button, > 0 if number */
						if (mark >= 0) {
/* If left mouse button clicked in active hyper-text area */
/*							if (bDebug) {
								sprintf(debugstr, "HIT BUTTON LABEL %s", buttonlabel);
								winerror(debugstr);
							} */
							if (hFile != -1 && bKeepFileOpen == 0) {/* debug */
								winerror("File still open");
								bSearchFlag = 0;
								break;
							}
/*							if (bBusyFlag++ > 0) {
								bSearchFlag = 0;
								break;	
							} */
#ifdef DEBUGHYPER	 /* debugging output only 95/Aug/12 */
							if (bDebug > 1) {
								if (*buttonlabel != '\0') {
									sprintf(debugstr, "Button Label: %s",
											buttonlabel);
									OutputDebugString(debugstr);
								}
								if (*str != '\0') {
									sprintf(debugstr, "Command String: %s", str);
									OutputDebugString(debugstr);
								}
							}
#endif
/* hyper execute added 94/Dec/13 */
							resetflag = 1;			/* default is to reset */
/*							scom = str; */
/*							samp = NULL; */
/* Need to preserve old name OpenName for savehyperstate() */
							if (strchr(OpenName, '\\') == NULL &&
								strchr(OpenName, '/') == NULL &&
								strchr(OpenName, ':') == NULL) {
/* if OpenName is unqualified (usual case) */
/*								strcpy(FileName, DefPath); */
								strcpy(szHyperFile, DefPath);
								s = szHyperFile + strlen(szHyperFile) - 1;
								if (*s != '\\' && *s != '/')
									strcat(szHyperFile, "\\");
								strcat(szHyperFile, OpenName);
							}
							else strcpy(szHyperFile, OpenName);
/* as well as old dvipage and old file position */ /* in case we jump */
							oldbuttonpos = buttonpos;
							oldbuttondvipage = buttondvipage;

/* Treat launch: ... and exec: ... first --- there may be more than one */
/* scom at start of `launch:', s at start of command, samp at next if any */
							while ((scom = strstr(str, "launch:")) != NULL ||
								   (scom = strstr(str, "execute:")) != NULL) {
								hyperlaunch(scom);
							}	/* end of hyper-text launch: or execute: */

/* Next look for `page: ...'  should be before `file: ... if any 96/May/18 */
							n = 1;  			/* dvipage if not specified */ 
							m = -1;
							scom = str;
							if (strncmp(scom, "page:", 5) == 0) {
								if (sscanf(scom+5, "%d+%d", &n, &m) < 2) {
									m = 0;
									sscanf(scom+5, "%d", &n);
								}
								n += m;
								s = scom;			/* 98/Mar/26 ??? */
/*	comma is delimiter between hyper-text commands */	/* BUG AREA */
/*	what if by mistake starts with comma ??? use s+1 ? */
								if ((samp = strchr(s, ',')) != NULL) { 
									if (*(samp-1) <= ' ') *(samp-1) = '\0';
									*samp++ = '\0';			/* remove , */
									while (*samp <= ' ' && *samp > 0) samp++;
								}
								else samp = s + strlen(s);
								strcpy(scom, samp);
/* are there any actions left ? if not just go to page specified */
								if (*scom == '\0') {	/* 1996/May/18 */
									bBusyFlag = 0;		/* ??? */
									bSearchFlag = 0;	/* ??? */
									if (bDebug > 1) {
										sprintf(debugstr, "Jump to page %d\n", n);
										OutputDebugString(debugstr);
									}
/* like searchandmark(hWnd, 1) ? or more like PAGESELECT ? selectpage */
/*									newpage = n; */
/*									goto selectcommon; */	/* ??? */
/* probably doesn't do the right stuff in SPREAD mode etc ... */
									resetflag = 0;		/* don't want reset */
									if (dvipage != n) {
										dvipage = n;		/* ??? */
										savehyperstate();	/* ??? */
										lastsearch = -1;
										usepagetable(dvipage, 0);			/* page is physical page */
										selectpage(hWnd);
										graypreviousnext(hWnd);
										lastsearch = -1;
										bEnableTermination = 1;		/* NEW */
										InvalidateRect(hWnd, NULL, TRUE);
									}
									break;	/* ??? */
								}
							} /* end of if (strncmp, "page", 5) */

/* Next look for `file: ...' - there should be only one of these */
/* scom at start of `launch:', s at start of command, samp at next if any */
/*							else if (strncmp(str, "file:", 5) == 0) */
/*							if (samp != NULL) scom = samp;
							else scom = str; */
/*							if (*scom != '\0') {
								if (bDebug > 1)	OutputDebugString(scom);
							} */

/*							if (strncmp(scom, "file:", 5) == 0) */
							scom = str;
/*							n = 1; */ 			/* dvipage if not specified */ 
							if (strncmp(scom, "file:", 5) == 0) {
								s = scom+4;
								while (*s != ':' && *s > 0) s++;
								s++;
								while (*s <= ' ' && s > 0) s++;
/*								n = 1; */		/* dvipage if not specified */ 
/*	allow page specification 95/Aug/12 */
								if (sscanf(s, "-p=%d", &n) == 1) {
									while (*s > ' ') s++;
									while (*s <= ' ' && s > 0) s++;
								}
/*								if (*buttonlabel == '\0') */
								if (sscanf(s, "-m=%s", &buttonlabel) == 1) {
									while (*s > ' ') s++;
									while (*s <= ' ' && s > 0) s++;
								}
/*								} */
/*								if (bDebug > 1) {
									sprintf(debugstr, "dvipage %d\n", n);
									OutputDebugString(debugstr);
								} */
/*	comma is delimiter between hyper-text commands */
								if ((samp = strchr(s, ',')) != NULL) {
									if (*(samp-1) <= ' ') *(samp-1) = '\0';
									*samp++ = '\0';			/* remove , */
									while (*samp <= ' ' && *samp > 0) samp++;
								}
								else samp = s + strlen(s);
								if (bDebug || bShowCalls) {
									flag = MaybeShowStr(scom, "File");
/*	Could cancel this if flag == 0 ? */ /* 96/Sep/15 */
								}
/*	set up new file name */
								strcpy(OpenName, s); 
/*	expand ..\foo.dvi notation and .\foo.dvi notation 1996/Jan/30 */
/*								if (*OpenName == '.') expanddots(OpenName); */
								if (*OpenName == '.' ||	*OpenName == '\\'
									|| *OpenName == '/') expanddots(OpenName); 
								AddExt(OpenName, ".dvi");
/*								if (bDebug > 1) {
									sprintf(debugstr, "Open Name: %s", OpenName);
									OutputDebugString(debugstr);
								} */
#ifdef LONGNAMES
/*	switch DVI file, unless its the same file ! 95/Aug/14 */
/*	may fail because of \ versus / usage ... */
/*		if (_stricmp(szHyperFile, OpenName) != 0) */
		if (comparenames(szHyperFile, OpenName) != 0) {
/*	first check whether file exists 95/Dec/18 ??? NEW */
/*			if ((hFileTemp = _lopen(OpenName, READ)) != HFILE_ERROR) */
			if ((hFileTemp = _lopen(OpenName, READ | 
						OfExistCode)) != HFILE_ERROR) { /* 96/May/18 */
			}
			else {
/*				if ((hFileTemp = _lopen(removepath(OpenName), READ)) 
						!= HFILE_ERROR) */
				if ((hFileTemp = _lopen(removepath(OpenName), READ |
						OfExistCode)) != HFILE_ERROR) { /* 96/May/18 */
					strcpy(OpenName, removepath(OpenName));
				}
			}
			if (hFileTemp == HFILE_ERROR) {
				strcpy(str, "Hyper Jump Failed to: ");
				strcat(str, OpenName);
				winerror(str);
/*				strcpy(OpenName, FileName);	*/		/* restore ??? */
				strcpy(OpenName, szHyperFile);		/* restore ??? */
/* somehow leaves it in a bad state if we do this ... */
				bBusyFlag = 0;						/* 1996/Jan/30 */
				bSearchFlag = 0;					/* 1996/Jan/30 */
				break;								/* 1996/Jan/30 */
			}
			else {
				_lclose(hFileTemp);		/* hurrah, it exists */
				hFileTemp = HFILE_ERROR;			/* 96/May/12 */
				if (bDebug > 1) OutputDebugString(OpenName);
				SwitchDVIFile(hWnd, n, 0);
			}
/*			SwitchDVIFile(hWnd, n, 0); */
		}	/* end of szHyperFile and OpenName are different */
		else {
			dvipage = n;		/* ??? */
			bBusyFlag = 0;		/* ??? 1996/Jan/30 */
			bSearchFlag = 0;	/* ??? 1996/Jan/30 */
		}
#else
/* switch DVI file, unless its the same file ! 95/Aug/14 */
/* may fail because of \ versus / usage ... */
/*								if (_stricmp(szHyperFile, OpenName) != 0) */
								if (comparenames(szHyperFile, OpenName) != 0) {
									SwitchDVIFile(hWnd, n, 0);
								}
								else {
									dvipage = n;		/* ??? */
									bBusyFlag = 0;		/* ??? 1996/Jan/30 */
									bSearchFlag = 0;	/* ??? 1996/Jan/30 */
								}
#endif
/* here is the code before long file names ... */
								resetflag = 0;			/* don't want reset */
/* need to save state if no button mark to go to */
								if (*buttonlabel == '\0') 
									savehyperstate();
								strcpy(scom, samp); /* strip out file: ... */
							} /* end of hyper-text file: case */

/*							else if (searchandmark(hWnd, 1) != 0) */
							if (*buttonlabel != '\0') { /* 95/Aug/12 */
/* searchandmark returns 0 if no need InvalidateRect (same page or failed) */
/* returns -1 otherwise */ /* also does a savehyperstate() */
								if (searchandmark(hWnd, 1) != 0) { /* hyper search */
									bBusyFlag = 0;
									bSearchFlag = 0;	/* 1993/March/29 */
									graypreviousnext(hWnd);
									resetflag = 0;		/* don't want reset */
									bEnableTermination = 1;		/* NEW */
									InvalidateRect(hWnd, NULL, TRUE);
								}
/* possibly omit this reset ? */
								else {	/* searchandmark == 0 */
									bBusyFlag = 0;
									bSearchFlag = 0;	/* 1993/March/29 */
								}
							} /* end of buttonlabel != "" */
/* hmm, this is probably not right yet ! */
/*							else if (*str == '\0') {
								bBusyFlag = 0;
								bSearchFlag = 0;
							} */
/*							graypreviousnext(hWnd); */
/*							break; */
							if (resetflag) {
								bBusyFlag = 0;
								bSearchFlag = 0;
							}
/*							if (bDebug > 1)	OutputDebugString("Exiting\n");*/
						}	/* if (mark > 0) */
						else {
							bBusyFlag = 0;			/* 1993/March/29 */
							bSearchFlag = 0;		/* 1993/March/29 */
						}
					}	/* end of  if (bSearchFlag++ == 0) */
				} /* end of  if (bHyperUsed && bFileValid) */
//				else if (bCharacterSample) {		// 2000 June 24
//					bCharacterSample = 0; /* AND THEN ? */
//					InvalidateRect(hWnd, NULL, TRUE);
//					break;
//				}
/*	otherwise just ignore the left button down message ... */
				break;

			case WM_MOUSEMOVE:
/*				if (bZoomFlag > 0 || bRectFlag > 0) *//* defining rect ? */
				if (bZoomFlag > 0 || bRectFlag > 0 || bRuleFlag > 0) {
/*					if (bZoomFlag != 0 && bCursorMoved != 0) {
						bCursorMoved = 0;
						break;
					} */
/*  first remove previous rectangle */
/*	Rect2DrawNotRect(hWnd, zoomstartx, zoomstarty, zoomendx, zoomendy); */
					hDC = GetDC(hWnd);
					(void) SelectObject(hDC, GetStockObject(NULL_BRUSH));
					(void) SetROP2 (hDC, R2_NOT);
					(void) Rectangle(hDC,
						(int) zoomstartx, (int) zoomstarty,
							(int) zoomendx, (int) zoomendy);
/* may need rework in Windows NT */ /* WIN32 ??? */
					if ((wParam & MK_RBUTTON) != 0) {	/* right button ? */

						pts = MAKEPOINTS(lParam);
						zoomcurrentx = pts.x;
						zoomcurrenty = pts.y;
						zoomstartx = (zoomstartx +
									  (zoomcurrentx - zoompreviousx));
						zoomstarty = (zoomstarty + 
									  (zoomcurrenty - zoompreviousy));
						zoomendx = (zoomendx + 
									(zoomcurrentx - zoompreviousx));
						zoomendy = (zoomendy + 
									(zoomcurrenty - zoompreviousy));
#ifdef IGNORED
						zoomcurrentx = LOWORD(lParam);
						zoomcurrenty = HIWORD(lParam);
						zoomstartx = (WORD) (zoomstartx + 
							(zoomcurrentx - zoompreviousx));
						zoomstarty = (WORD) (zoomstarty + 
							(zoomcurrenty - zoompreviousy));
						zoomendx = (WORD) (zoomendx + 
							(zoomcurrentx - zoompreviousx));
						zoomendy = (WORD) (zoomendy + 
							(zoomcurrenty - zoompreviousy));
#endif
						zoompreviousx = zoomcurrentx;	/* 93/Sep/30 */
						zoompreviousy = zoomcurrenty;						
					}			/* end of right button case */
					else {		/* no, just left button down */

						pts = MAKEPOINTS(lParam);
						zoomendx = pts.x;
						zoomendy = pts.y;

/*						zoomendx = LOWORD(lParam); */
/*						zoomendy = HIWORD(lParam); */

/* Remember old cursor position before adjusting - 1993/Sep/30 */
						zoompreviousx = zoomendx;
						zoompreviousy = zoomendy;							
						if (bZoomFlag != 0 && bAspectKeep != 0) {
/* Try and maintain aspect ratio - 1993/Sep/28 */
							RHeight = zoomendy - zoomstarty;
							RWidth = zoomendx - zoomstartx;
							if (labs((long) RWidth * ZHeight) <
								labs((long) RHeight * ZWidth)) {
								if ((RHeight < 0 && RWidth > 0) ||
									(RHeight > 0 && RWidth < 0))
										RHeight = - RHeight;
								RWidth = (int) ((long) RHeight * ZWidth /
									ZHeight);

								zoomendx = (zoomstartx + RWidth);

/*								zoomendx = (WORD) (zoomstartx + RWidth); */
							}
							else {
								if ((RHeight < 0 && RWidth > 0) ||
									(RHeight > 0 && RWidth < 0))
										RWidth = - RWidth;
								RHeight = (int) ((long) RWidth * ZHeight /
									ZWidth);

								zoomendy = (zoomstarty + RHeight);

/*								zoomendy = (WORD) (zoomstarty + RHeight); */

							}
/*							GetCursorPos(&CurPoint); */
/*							CurPoint.x = zoomendx; CurPoint.y = zoomendy;
							ClientToScreen(hWnd, &CurPoint);
							SetCursorPos(CurPoint.x, CurPoint.y);
							bCursorMoved = 1; */
/* Move cursor there also ? */
						}	/* end of keep aspect ratio while zooming */
					}
/* draw new rectangle */
/*	Rect2DrawNotRect(hWnd, zoomstartx, zoomstarty, zoomendx, zoomendy); */
					(void) Rectangle(hDC,
						(int) zoomstartx, (int) zoomstarty,
							(int) zoomendx, (int) zoomendy);
					if (bRuleFlag != 0 && (wParam & MK_RBUTTON) == 0)
						showsize(hDC, 
							zoomstartx, zoomstarty, zoomendx, zoomendy); 
/*					if (bRuleFlag != 0)
						(void) SetCursor(hBlankCursor);	*/
					(void) ReleaseDC(hWnd, hDC);
				}	/* end of three types of rectangle cases */
				else if (bZoomFlag < 0)	
					(void) SetCursor(hZoomCursor);		/* keep magnifier */
				else if (bRectFlag < 0)	
					(void) SetCursor(hCopyCursor);		/* keep scissors */
				else if (bRuleFlag < 0)	
					(void) SetCursor(hBlankCursor);		/* keep blank */
				else if (dragflag != 0) {

					pts = MAKEPOINTS(lParam);
					dragendx = pts.x;
					dragendy = pts.y;

/*					dragendx = LOWORD(lParam);
					dragendy = HIWORD(lParam); */

/* see if following helps speed things up at all ? */ /* check WIN32 */
					GetCursorPos(&CurPoint);
					ScreenToClient(hWnd, &CurPoint);

					dragendx = CurPoint.x;
					dragendy = CurPoint.y; 

/*					dragendx = (WORD) CurPoint.x;
					dragendy = (WORD) CurPoint.y;  */

					dx = (int) dragendx - (int) dragcurrentx;
					dy = (int) dragendy - (int) dragcurrenty;
					if (dx > SCROLLTHRESH || dx < -SCROLLTHRESH ||
						dy > SCROLLTHRESH || dy < -SCROLLTHRESH) {
						dragcurrentx = dragendx;
						dragcurrenty = dragendy;			
						ScrollWindow(hWnd, dx, dy, NULL, NULL);  
/*						(void) GetUpdateRect(hWnd, &UpdateRect, TRUE); */
						OffsetRgn(hRgn, dx, dy);
						addtoregion(hWnd, TRUE);
/*						ValidateRect(hWnd, &UpdateRect); */
					}
				}
				else if (bHourFlag != 0)
					(void) SetCursor(hHourGlass); /* redundant ? */
				else (void) SetCursor(hArrow);
				break;

// In Windows 95 need to instead check if( msg == uMSH_MOUSEWHEEL )
// after uMSH_MOUSEWHEEL = RegisterWindowMessage(MSH_MOUSEWHEEL);
				
			case WM_MOUSEWHEEL:		// 2000 May 22
				fwKeys = GET_KEYSTATE_WPARAM(wParam);
				zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				xPos = GET_X_LPARAM(lParam); 
				yPos = GET_Y_LPARAM(lParam); 
//				if (bDebug > 1) {
//					sprintf(debugstr, "wheel delta %d fwKeys %d xPos %d yPos %d MK_SHIFT %d MK_CONTROL %d",
//							zDelta, fwKeys, xPos, yPos, MK_SHIFT, MK_CONTROL);
//					OutputDebugString(debugstr);
//					winerror(debugstr);	// debugging only
//				}
				if (fwKeys & MK_CONTROL) 
					SendMessage(hwnd, WM_COMMAND, (zDelta > 0) ? IDM_MAGNIFY : IDM_UNMAGNIFY, 0L);
				else if (fwKeys & MK_SHIFT) 
//					SendMessage(hwnd, WM_COMMAND, (zDelta > 0) ? IDM_NEXT : IDM_PREVIOUS, 0L);
					SendMessage(hwnd, WM_COMMAND, (zDelta > 0) ? IDM_SPACE : IDM_BACKSPACE, 0L);
				else if (fwKeys & MK_MBUTTON) 
					SendMessage(hwnd, WM_VSCROLL, (zDelta > 0) ? SB_PAGEUP : SB_PAGEDOWN, 0L);
				else
					SendMessage(hwnd, WM_VSCROLL, (zDelta > 0) ? SB_LINEUP : SB_LINEDOWN, 0L);
				break;

//			case WM_MBUTTONDOWN:
//				winerror("Middle Button Down");	// debugging only
//				break;

//			case WM_MBUTTONUP:
//				winerror("Middle Button Up");	// debugging only
//				break;

			case WM_LBUTTONUP:
/*				if (bZoomFlag > 0 || bRectFlag > 0) */
				bUserAbort = 0;					/* 1995/Dec/10 */
				if (bZoomFlag > 0 || bRectFlag > 0 || bRuleFlag > 0) {
/*					(void) SetCursor(hSaveCursor);*/	/* restore old cursor */
					(void) SetCursor(hOldCursor);	/* restore old cursor */
					ReleaseCapture();		/* release mouse */
/*					if (bZoomFlag != 0) bCursorMoved = 0; */
					rectflag = bRectFlag;		/* remember Zoom versus Copy */
					zoomflag = bZoomFlag;		/* remember Zoom versus Copy */
					ruleflag = bRuleFlag;
					bZoomFlag = 0; bRectFlag = 0; bRuleFlag = 0;
/*	remove rectangle again */
/*	Rect2DrawNotRect(hWnd, zoomstartx, zoomstarty, zoomendx, zoomendy); */
					hDC = GetDC(hWnd);
					(void) SelectObject(hDC, GetStockObject(NULL_BRUSH));
					(void) SetROP2 (hDC, R2_NOT);
					(void) Rectangle(hDC,
						(int) zoomstartx, (int) zoomstarty,
							(int) zoomendx, (int) zoomendy);
					(void) ReleaseDC(hWnd, hDC);
/*					zoomendx = LOWORD(lParam); */
/*					zoomendy = HIWORD(lParam); */
					if (zoomendx < zoomstartx) {
						zoomtemp = zoomstartx;
						zoomstartx = zoomendx;
						zoomendx = zoomtemp;
					}
					if (zoomendy < zoomstarty) {
						zoomtemp = zoomstarty;
						zoomstarty = zoomendy;
						zoomendy = zoomtemp;
					}
					ZoomRect.left = (int) zoomstartx;
					ZoomRect.right = (int) zoomendx;
					ZoomRect.top = (int) zoomstarty;
					ZoomRect.bottom = (int) zoomendy;
/*					if (GetAsyncKeyState(VK_CONTROL) < 0) */
					if (rectflag != 0) {  /* actually is a copy to clipboard */
/*						UpdateWindow(hWnd);		*/	/* try and update NOW */
/*						SetRectEmpty(&UpdateRect); *//* 1992/Apr/12 */
						setupregions();				/* 1992/Apr/12 */
/*						bBusyFlag++; */
						if (bBusyFlag++ > 0) {		/* 1992/Nov/2 */
							bBusyFlag = 0;			/* ??? */
							break;
						}
						(void) DoCopy(hWnd, ZoomRect);
/*							zoomstartx, zoomstarty, zoomendx, zoomendy); */
						/* in winprint.c */
						bBusyFlag = 0;
/*						if (IsRectEmpty(&UpdateRect) == 0)	
							InvalidateRect(hWnd, &UpdateRect, TRUE); */
						forceregion(hWnd);			/* 1992/Apr/12 */
					}		/* end of rectflag != 0 */
					else if (zoomflag != 0) {
						if ((zoomendx > zoomstartx + 10) &&
						     (zoomendy > zoomstarty + 10)) {
/*	only execute zoom if rectangle large enough */
/*	do stuff ... adjust wantedzoom, xoffset, yoffset */
						savegraphicstate();			/* save old state */
/*						ZoomRect.left = (int) zoomstartx;
						ZoomRect.right = (int) zoomendx;
						ZoomRect.top = (int) zoomstarty;
						ZoomRect.bottom = (int) zoomendy; */
						GetClientRect(hWnd, &ClientRect);
						hDC = GetDC(hWnd);
						(void) SetMapMode(hDC, MM_TWIPS); /* set unit twips */
/*						ClientRect = DPtoLPRect(hDC, ClientRect); */
						flag = setnewzoom(hDC, ClientRect, ZoomRect);
						(void) ReleaseDC(hWnd, hDC);
						if (flag != 0) {			/* has zoom changed ? */
							checkmagnify(hWnd);	/* 1993/March/27 */
							newmapandfonts(hWnd);
							bEnableTermination = 1;
							InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
						}
						else (void) restoregraphicstate();
						}
						bHyperLast = 0;	/* last action was zoom 95/Aug/12 */
					}	/* end of zoomflag != 0 */
					else if (ruleflag != 0) {
						destroyrulewindows();
					}
				}	/* end of zoomflag, rectflag or ruleflag */
/*	end of dragging image */
				else if (dragflag != 0) {
/*					(void) SetCursor(hSaveCursor); *//* restore old cursor */
					(void) SetCursor(hOldCursor);	/* restore old cursor */
					ReleaseCapture();				/* release mouse */
					dragflag = 0;

					pts = MAKEPOINTS(lParam);
					dragendx = pts.x;
					dragendy = pts.y;

/*					dragendx = LOWORD(lParam);
					dragendy = HIWORD(lParam); */

					dx = (int) dragendx - (int) dragcurrentx;
					dy = (int) dragendy - (int) dragcurrenty;
					ScrollWindow(hWnd, dx, dy, NULL, NULL);
/* This is only the last time, so not as important as above ... */
/*					(void) GetUpdateRect(hWnd, &UpdateRect, TRUE);  */
/*					(void) GetUpdateRgn(hWnd, hUpdateRgn, TRUE);  
					OffsetRgn(hRgn, dx, dy);
					CombineRgn(hRgn, hRgn, hUpdateRgn, RGN_OR); */
					addtoregion(hWnd, TRUE);
					dx = (int) dragendx - (int) dragstartx;
					dy = (int) dragendy - (int) dragstarty;
					adjustoffset(hWnd, dx, dy); 
/*					adjustoffset(hWnd, (int) dragstartx, (int) dragstarty,
						(int) dragendx, (int) dragendy); */
/*					InvalidateRgn(hWnd, hRgn, TRUE); */	
					forceregion(hWnd);			/* FORCE REPAINT */
				}
/*				else if (hClientWindow != NULL) {

					pts = MAKEPOINTS(lParam);
					curx = pts.x;
					cury = pts.y;

					mark = checkmarkhit(hWnd, curx, cury);
					if (mark > 0)
						PostMessage(hClientWindow, WM_MARKHIT, mark, 0L);
				} */
				break;
				
/* NOTE: also come from typing Enter key */

			case WM_LBUTTONDBLCLK:				/* force screen redraw */
				if (IsIconic(hwnd)) break;		/* Ignore if iconic! */
/* AN EXPERIMENT 1996/Aug/17 */ /* to try and discover WMF blank display */
#ifdef IGNORED 
				hDC = GetDC(hWnd);
				(void) SetMapMode(hDC, MM_TWIPS);
				(void) GetClipBox(hDC, &ClipRect);
				(void) redomapping(wantedzoom, ClipRect, ClipRect);
				(void) ReleaseDC(hWnd, hDC);
				newmapandfonts(hWnd);
#endif
				checkmagnify(hWnd);				/* NO REASON TO DO THIS */
/* #endif */
/*				if (bSynchronicity) */			/* experiment 98/Nov/5 */
				if (szEditorDDE != NULL) {

					pts = MAKEPOINTS(lParam);
					curx = (int) pts.x;
					cury = (int) pts.y;					

/*					curx = (int) LOWORD(lParam);
					cury = (int) HIWORD(lParam); */

					bMarkSearch = 1;			/* "inverse search" */
/*					bInverseSearch = 1; */		/* 98/Dec/20 */
					flag = TagFont(hWnd, curx, cury, 1);	/* in winfonts.c */

					if (bDebug > 1) {
						sprintf(debugstr, "scr `%s' line %d hit %d",
								srcfile, srclineno, srctaghit);
						OutputDebugString(debugstr);
					}
					if (srclineno > 0 && *srcfile != '\0' && srctaghit != 0) {
						if (bDebug > 1) OutputDebugString("CallEditor"); 
						CallEditor(srclineno, srcfile, szEditorDDE, szTeXEdit);
					}
/*					bInverseSearch = 0; */		/* 98/Dec/20 */
					bMarkSearch = 0;			/* ??? */
				}
				else {
					bEnableTermination = 1;
					InvalidateRect(hWnd, NULL, TRUE); 	/* erase */
				}
				break;

			case WM_CREATE:	
/*				hAccTable = LoadAccelerators(hInst, "DviWindoMenu"); */
				hAccTable = LoadAccelerators(hInst, DviWindoMenu);
				if (bAllowDrag != 0) {
					bUsingDrag = 1;
					if (bUsingDrag) DragAcceptFiles(hWnd, TRUE);
				}
				if (bAllowCommDlg != 0) bUsingCommDlg = 1;
				break;

			case WM_DROPFILES:		/* assume only one file ... */
				if (bUsingDrag != 0) {
/*					can flush this stuff now ? */
					if (bFontSample || bCharacterSample) EndFontState(hWnd);
/*					can flush this stuff now ? */
					if (bFileValid >= 0) closeup(hwnd, 1);
					hDrop = (HDROP) wParam;
/*					DragQueryFile(hDrop, 0, OpenName, sizeof(OpenName)); */
					DragQueryFile(hDrop, 0, FileName, sizeof(FileName)); 
#ifdef IGNORED		/* removed 98/Dec/15 */
					bReadFile = 1;				/* ??? */
					dvipage = 1;				/* ??? */
					SetupFile(hwnd, 1);			/* ??? */
#endif
/*	do via PostMessage now 98/Dec/15 --- better test this ! safe to use FileName ? */
					PostMessage(hwnd, WM_COMMAND, IDM_OPEN, (LPARAM) FileName);
/*					if (IsIconic(hwnd) != 0) ShowWindow(hWnd, SW_RESTORE); */
					if (IsIconic(hwnd) != 0) ShowWindow(hwnd, SW_RESTORE);
/*					InvalidateRect(hWnd, NULL, TRUE); */		/* redundant ? flush */
					DragFinish(hDrop);  
				}
				break;

			case WM_ERASEBKGND:	
				bBusyFlag++;
				hDC = (HDC) wParam;
				(void) GetClipBox(hDC, &ClipRect);
/*				if (hBack != NULL && bFileValid && bColorFont == 0)  */
				if (hBack != NULL && bFileValid && bColorFont == 0 && dvipage >= 0 && dvipage < MaxBack) {
					GrabBack();
					clrBackground = lpBack[dvipage];
					ReleaseBack();
/*					if (dvipage >= MaxBack)	clrBackground=RGB(127, 0, 127); */ /* debug removed */
/*					if (clrBackground != (COLOR_WINDOW+1)) */
					if (clrBackground != OrgBkColor)
						hbrBackground = CreateSolidBrush(clrBackground);
					else hbrBackground = hbrDefaultBackground;
				}
				else hbrBackground = hbrDefaultBackground;
/*				if (bDebug > 1) {
					sprintf(debugstr, "C_W+1 %X clrBack %X hbrBack %X hbrBackDef %X",
							(COLOR_WINDOW+1), clrBackground,
							hbrBackground, hbrDefaultBackground);
					OutputDebugString(debugstr);
				} */
				hOldBrush = SelectObject(hDC, hbrBackground);
				PatBlt(hDC, ClipRect.left, ClipRect.top,
				   ClipRect.right-ClipRect.left,
					   ClipRect.bottom-ClipRect.top, PATCOPY); /* ??? */
				(void) SelectObject(hDC, hOldBrush);		/* ??? */
				if (hbrBackground != hbrDefaultBackground)
					DeleteObject(hbrBackground);	/* ??? */
/*				if (dragflag == 0)
					return (DefWindowProc(hWnd, message, wParam, lParam)); */
				bBusyFlag--;
				return TRUE;
				break;

/* DON'T try and put up a MessageBox while in WM_PAINT ! */

			case WM_PAINT:					/* the real thing !!! */
#ifdef DEBUGMAIN
				if (bDebug > 1) OutputDebugString("Enter WM_PAINT\n"); 
#endif
				if (dragflag != 0) {
/* following was there to suppress debug kernel complaint */
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
/*					(void) GetUpdateRgn(hwnd, UpdateRgn, TRUE); */
					break; 
				}
/*				if (bPrintFlag > 0 || bCopyFlag > 0) break;	*//* IGNORE */
				if (bRuleFlag > 0) {
/*					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);*/  /* ??? */
/*					break; */ /* ??? */
				}
/*				if (bPrintFlag > 0 || bCopyFlag > 0) */
				if (bPrintFlag > 0 || bCopyFlag > 0 || bScanFlag) {/* 95/Dec/21*/
/*					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE); */
/*					ValidateRect(hWnd, &UpdateRect); */ /* 1992/April/12 */
/*	this shouldn't be needed anymore now that printing fixed ... */
					addtoregion(hWnd, FALSE);		/* 92/April/12 */
/*					(void) GetUpdateRgn(hwnd, UpdateRgn, TRUE); */
					break;			/* avoid painting at wrong scale !!! */
				}
/*				if (bWriteAFM > 0) {
					addtoregion(hWnd, FALSE);
				} */

/* do following only if SHARE is active ??? */		/* 93/Jan/5 */
/*				if (hTaskWindow != NULL) 	*/
				if (TaskInfo.hWnd != NULL) {		/* 95/Mar/12 */
/*					if (IsTaskActive(hwnd)) */
					if (IsTaskActive()) {
						if (bPreventConflict) {		/* 94/Dec/13 */
							addtoregion(hWnd, FALSE);
							break;
						}
/*						break;			 */	/* hmm ... dangerous */
					}
					else InvalidateRect(hWnd, NULL, TRUE);	/* 94/Dec/17 */
				} 
				if (bBusyFlag++ > 0) {
					if (bDebug > 1) OutputDebugString("Busy Reentry\n");

/*					addtoregion(hWnd, FALSE);		*/ /* ??? */
/*					break;			*/

					if (bBusyFlag % 16 == 0) {			/* 92/Oct/29 */
						if (bDebug > 1) OutputDebugString("Flush Repaint\n");
						hDC = BeginPaint(hWnd, &ps);	
						EndPaint(hWnd, &ps);
						if (bBusyFlag % 1024 == 0) {
							if (bDebug > 1)
								OutputDebugString("Reset bBusyFlag\n");
							(void) EnableWindow(hWnd, TRUE); 	
/* reenable in case disabled */
							bBusyFlag = 0;	
						}
					}
					break;						/* avoid reentry !!! */
				}

#ifdef DEBUGMAIN
				if (bDebug > 1) OutputDebugString("BeginPaint\n"); 
#endif
				hDC = BeginPaint(hWnd, &ps);
/*	ps.rcPaint now has UpdateRect if we want it ... */
/*  and update region has now been emptied ... */
				if (hFile != -1 && bKeepFileOpen == 0) {
/* not safe to give error message at this time ! */
/*					winerror("Reentered WM_PAINT"); */	/* CRASH ! */
					if (bDebug > 1) OutputDebugString("Paint Reentry\n");
					EndPaint(hWnd, &ps);
					bBusyFlag = 0;
					break;						/* reentered */
				}

#ifdef IGNORED
				if (bATMLoaded) {		/* 96/June/4 */
					flag = MyATMMarkDC(APIVersion, hDC, NULL, NULL);
					if (flag != ATM_NOERR) {
						if (bDebug > 1) {
							wsprintf(debugstr, "ATM Error %d (%s)\n",
									flag, (LPSTR) "MarkDC");
							OutputDebugString(debugstr);
						}
					} 
				}
#endif

/* start up text and background in default colors */
/* revised 98/Feb/14 to allow bCarryColor across page borders */

/*				if (colorindex == 0 || bCarryColor == 0) */
				SetTextColor(hDC, TextColor);	/* 92/May/06 */
				CurrentTextColor = TextColor;	/* 95/April/30 */
				SetBkColor(hDC, BkColor);		/* 92/May/06 */
				CurrentBackColor = BkColor;		/* 95/April/30 */
/*				SetROP2 (hDC, R2_COPYPEN); */	/* 96/Aug/31 ??? */

/*				{	
					int r, g, b;
					CurrentTextColor = ColorStack[colorindex];
					(void) SetTextColor(hDC, CurrentTextColor);
					r = GetRValue (CurrentTextColor);
					g = GetGValue (CurrentTextColor);
					b = GetBValue (CurrentTextColor);
					SetupRuleColor(hDC, r, g, b);
				} */ /* if colorindex > 0 && bCarryColor > 0 */

				if (bFontSample || bCharacterSample) { 
					if (bShowWidths) {
						if (! bWriteAFM)		/* 95/Jun/28 */
							ShowCharWidths(hWnd, hDC);
					}
					else if (bCharacterSample) ShowFontSample(hWnd, hDC, nChar);
					else ShowFontSample(hWnd, hDC, -1);	// all chars
					EndPaint(hWnd, &ps);
					bBusyFlag = 0;
					break;
				}
/*				ps.rcPaint is rectangle to be repainted */
				if(bFileValid != 0 && dvipage != -INFINITY) {
					if(paintpage(hWnd, hDC) < 0) {
/* commented out as an experiment 1993/Jan/6 */
/*						bFileValid = 0; */				/* failed open */
/* commented out as an experiment 1993/Jan/6 */
/*						InvalidateRect(hWnd, NULL, TRUE); *//* redundant ? */
/* following added 1993/Jan/6 */
						if (bFileValid == 0) InvalidateRect(hWnd, NULL, TRUE); 
					}
				}
				EndPaint(hWnd, &ps);
#ifdef DEBUGMAIN
				if (bDebug > 1) OutputDebugString("EndPaint\n"); 
#endif
				bBusyFlag = 0;
/* this is perhaps the wrong place for this, but ... */ /* 1994/Jan/22 */
/*				if (!bIgnoreBadANSI && bNeedANSIacce && !bNeedANSIwarn) */
				if (!bIgnoreBadANSI && bCheckEncoding &&
					bNeedANSIacce && !bNeedANSIwarn) {	/* 97/Feb/18 */
					unsigned int size;
					int intsize, fracsize;			/* 95/Jun/23 */
/*					revised to get two decimal points 97/Apr/10 */
					setscale(0);					/* dangerous ? */
/*					size = (unsigned int) mapd((long) fs[nNeedANSIfont]); */
					size = (unsigned int) mapd((long) fs[nNeedANSIfont] * 5);
					size = size + (size * 3 + 399) / 800; /* (7227 / 7200); */
					setscale(wantedzoom);
/*					following similar to code in ShowFontInfo ... */
/*					size = (size + 1) / 2; */	/* from twips to 1/10 pt */
/*					size = size * 5; */			/* from twips to 1/100 pt */
/*					intsize = (int) (size / 10); */
					intsize = (int) (size / 100);
/*					fracsize = (int) (size - ((unsigned) intsize) * 10); */
					fracsize = (int) (size - ((unsigned) intsize) * 100);
					{
						char *fname="UNKNOWN";
						if (fontname[nNeedANSIfont] != NULL)
							fname = fontname[nNeedANSIfont];
						sprintf(str, "Encoding problem in text font %s "
							"at %d.%02d pt char %d (%s?):\n\n",
							fname, /* fontname[nNeedANSIfont], */
							intsize, fracsize,
		nNeedANSIchar, (nNeedANSIchar < 32) ? textext[nNeedANSIchar] : "");
					}
/*					strcat(str, 
 "Font appears ANSI encoded, yet DVI file calls for non ANSI char.\n\
 Apparent mismatch TFM encoding and outline font encoding.\n\
 Or perhaps need `\\input texnansi' or `\\input ansiacce'?"); */
/* Use ENC_ERROR string instead to save space in data segment */
					s = str + strlen(str);
					LoadString(hInst, ENC_ERROR, s, sizeof(str) - strlen(str));
					winerror(str);
					bNeedANSIwarn = 1;
				}
#ifdef DEBUGMAIN
				if (bDebug > 1) OutputDebugString("Exit WM_PAINT\n"); 
#endif
				break;			/* end of WM_PAINT case */

/*		Need to use SetScrollPos to adjust thumb */
/*		Need to account for changed param packing in WIN32 */

		case WM_VSCROLL:
			nScrollCode = (int) LOWORD(wParam);  /*	 scroll bar value */
			nPos = (short int) HIWORD(wParam);   /*	 scroll box position */
			hwndScrollBar = (HWND) lParam;       /*	 handle of scroll bar */

//			if (bDebug > 1) {
//				sprintf(debugstr, "nScrollCode %d nPos %d wParam %d",
//						nScrollCode, nPos, wParam);
//				OutputDebugString(debugstr);
//				wincancel(debugstr);	// debugging only
//			}

/*			nPos only used by SB_THUMBPOSITION */
/*			hwndScrollBar not used at all */
		vscroll:
			hDC = GetDC(hWnd);
/*			(void) SetMapMode(hDC, MM_TWIPS); */	/* set unit to twips */
			(void) GetClipBox(hDC, &ClipRect);

/*			redefine width, height to be int WIN32 */
			width = (ClipRect.right - ClipRect.left); 
			height = (ClipRect.bottom  - ClipRect.top); 

/*			width = (WORD) (ClipRect.right - ClipRect.left);  */
/*			height = (WORD) (ClipRect.bottom  - ClipRect.top); */

/*			if (bShiftFlag != 0) stepsize = 4 * SCROLLSTEP */
			if (GetAsyncKeyState(VK_SHIFT) < 0) stepsize = 4 * SCROLLSTEP;
			else stepsize = SCROLLSTEP;

/*	wParam LOWORD = scroll bar value HIWORD scroll bar position ? */
			switch (nScrollCode) {
				case SB_LINEUP:
					ScrollWindow(hwnd, 0, stepsize, NULL, NULL);
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
					adjustoffset(hWnd, 0, stepsize);
					break;
				case SB_LINEDOWN:
					ScrollWindow(hWnd, 0, - stepsize, NULL, NULL);
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
					adjustoffset(hWnd, 0, - stepsize);
					break;
				case SB_PAGEUP:
					ScrollWindow(hWnd, 0, (int) (height/2), NULL, NULL);
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
					adjustoffset(hWnd, 0, (int) (height/2));
					break;
				case SB_PAGEDOWN:
					ScrollWindow(hWnd, 0, -(int) (height/2), NULL, NULL);
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
					adjustoffset(hWnd, 0, -(int) (height/2));
					break;
				case SB_THUMBPOSITION:
/*					nPos = LOWORD(lParam);	*/			/* 1993/Feb/22 */
					SetScrollPos(hwnd, SB_VERT, nPos, TRUE);
					yoffsetold = yoffset;
					yoffset = yoffsetscroll(hwnd, nPos);	/* ??? */
					scrolladjust(hWnd, 0, yoffset - yoffsetold);
					break;
				default:
					break;	/* ? */
			}
			(void) ReleaseDC(hWnd, hDC);
			break;
			
/*		Need to use SetScrollPos to adjust thumb */
/*		Need to account for changed param packing in WIN32 */

		case WM_HSCROLL:

			nScrollCode = (int) LOWORD(wParam);  /*	 scroll bar value */
			nPos = (short int) HIWORD(wParam);   /*	 scroll box position */
			hwndScrollBar = (HWND) lParam;       /*	 handle of scroll bar */

/* 			nScrollCode = wParam;	 */
/* 			nPos = LOWORD(lParam);	 */
/* 			hwndScrollBar = (HWND) HIWORD(lParam);  */

/*	nPoS only used by SB_THUMBPOSITION */
/*	hwndScrollBar not used at all */
		hscroll:
			hDC = GetDC(hWnd);
/*			(void) SetMapMode(hDC, MM_TWIPS); */	/* set unit to twips */
			(void) GetClipBox(hDC, &ClipRect);

			width = (ClipRect.right - ClipRect.left);
			height = (ClipRect.bottom  - ClipRect.top); 

/* 			width = (WORD) (ClipRect.right - ClipRect.left); */
/*			height = (WORD) (ClipRect.bottom  - ClipRect.top); */

/*			if (bShiftFlag !== 0) stepsize = 4 * SCROLLSTEP; */
			if (GetAsyncKeyState(VK_SHIFT) < 0) stepsize = 4 * SCROLLSTEP;
			else stepsize = SCROLLSTEP;

/*	wParam LOWORD = scroll bar value HIWORD scroll bar position */
			switch (nScrollCode) {
				case SB_LINEUP:
					ScrollWindow(hWnd, stepsize, 0, NULL, NULL);
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
					adjustoffset(hWnd, stepsize, 0);
					break;
				case SB_LINEDOWN:
					ScrollWindow(hWnd, -stepsize, 0, NULL, NULL);
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
					adjustoffset(hWnd, -stepsize, 0);
					break;
				case SB_PAGEUP:
					ScrollWindow(hWnd, (int) (width/2), 0, NULL, NULL);
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
					adjustoffset(hWnd, (int) (width/2), 0);
					break;
				case SB_PAGEDOWN:
					ScrollWindow(hWnd, -(int) (width/2), 0, NULL, NULL);
					(void) GetUpdateRect(hwnd, &UpdateRect, TRUE);
					adjustoffset(hWnd, -(int) (width/2), 0);
					break;
				case SB_THUMBPOSITION:
/*					nPos = LOWORD(lParam); */				/* 1993/Feb/22 */
					SetScrollPos(hwnd, SB_HORZ, nPos, TRUE);
					xoffsetold = xoffset;
					xoffset = xoffsetscroll(hwnd, nPos);	/* ??? */
					scrolladjust(hWnd, xoffset - xoffsetold, 0);
					break;
				default:
					break;	/* ? */
			}
			(void) ReleaseDC(hWnd, hDC);
			break;			

/* will need to use SetScrollPos to adjust thumb */

		case WM_QUERYENDSESSION:			/* message: to end the session? */
			maybesaveprefer();				/* 92/02/22 */ /* desparation ! */
			return (TRUE);					/* yes, if you wish to quit */

		case WM_ENDSESSION:					/* 92/02/02 */ /* doesn't work? */
			if (wParam != 0) {				/* never get here ! */
				maybesaveprefer();			/* 92/02/22 */
/*				do any other cleanup ? */
/*				PostQuitMessage(0);			 */ /* not needed */
/*				(void) DestroyWindow(hwnd); */ /* not needed */
			}
			break;

		case WM_CLOSE:						/* message: close the window	*/
/*			WinHelp(hWnd, lpszHelpFileName, HELP_QUIT, 0); */
/*			bFlipSave = bShiftFlag;  */	/* flip save preference */
			if (GetAsyncKeyState(VK_SHIFT) < 0 ||
				GetAsyncKeyState(VK_CONTROL) < 0) bFlipSave = 1;
			else bFlipSave = 0;
/*			drop through from above */

		case WM_GOODBYE:					/* from client - (for now) */
/*			(void) DestroyWindow(hWnd); */
			PostQuitMessage(0);				/* 92/01/26 */
			break;

		case WM_DESTROY:
/* do some cleanup here */	/* moved to end of WinMain instead */
			if (bUsingDrag != 0) {
				DragAcceptFiles(hwnd, FALSE);   
				bUsingDrag = 0;
			}
			if (bUsingCommDlg != 0) {			/* 1993/Dec/1 */
				bUsingCommDlg = 0;
			}
/*	Cleanup moved to WinMain after Main Message Loop exits ... */
/* new, get rid of cursor that were loaded 1993/Feb/20 */
			if (hHandCursor != NULL) (void) DestroyCursor(hHandCursor);
			if (hZoomCursor != NULL) (void) DestroyCursor(hZoomCursor);
			if (hCopyCursor != NULL) (void) DestroyCursor(hCopyCursor);
			if (hBlankCursor != NULL) (void) DestroyCursor(hBlankCursor);
/* don't need to get rid of brushes - they are all stock objects */
/* write preferences */
/*			if (bFlipSave != 0) {
				if (bSavePrefer != 0) bSavePrefer = 0;
				else bSavePrefer = 1; 	
			}
			else {						
				sprintf(str, "%d", bSavePrefer);
				(void) WritePrivateProfileString(achPr, "Save",
					str, achFile);
			}
			if (bSavePrefer != 0) WritePreferences(0); */
			PostQuitMessage(0);
			break;

/* When main window is disabled, make sure to disable modeless dialogs */
/* Conversely when main windows is reenabled, reenable modeless dialogs */
/* Otherwise face possible reentrancy ? *//* Not needed after all 1995/May/6 */
/*		case WM_ENABLE:
			if (hInfoBox != NULL) EnableWindow(hInfoBox, wParam);
			if (hFntBox != NULL) EnableWindow(hFntBox, wParam);
			if (hFindBox != NULL) EnableWindow(hFindBox, wParam);
			break; */

		case WM_MOVE:
			if (IsZoomed(hwnd)) break;  /* Ignore if maximized */
			if (IsIconic(hwnd)) break;  /* Ignore if iconic! */
			xPos = (int)(short) LOWORD(lParam);    /* h position client */
			yPos = (int)(short) HIWORD(lParam);    /* v position client */
			GetWindowRect(hWnd, &WinRect);
			xLeft = WinRect.left;	/* h position window */
			yTop = WinRect.top;		/* v position window */
/*			sprintf(str,
				"xPos %d yPos %d WM_MOVE lParam\nxLeft %d yTop %d WM_MOVE GetWindowRect", 
					xPos, yPos, xLeft, yTop);
			wininfo(str); */	/* debugging */
/*			wininfo("MOVE"); */
			break;

		case WM_SIZE:
			fwSizeType = wParam;      /* resizing flag */
			nWidth = LOWORD(lParam);  /* width of client area */
			nHeight = HIWORD(lParam); /* height of client area */

			if (IsIconic(hwnd)) bWasIconic = 1;
/* iconic ---use short Window caption */ 
/* not iconic --- normal long Window caption */
			SetupWindowText(hwnd);

			if (IsIconic(hwnd)) {
				if (bTimerOn > 0) {
					KillTimer(hwnd, 1);
					bTimerOn = 0;
					nAlarmTicks = 0;				/* 1994/Aug/10 */
					bSleeping = -1;
				}
				break;  
			}
			else if (bSleeping != 0) {
				if (bUseTimer != 0 && bTimerOn == 0) {
					TurnTimerOn(hwnd);
/*					bTimerOn = SetTimer(hwnd, 1, TICKTOCK, NULL); */
				}
				bSleeping = 0;
			}
			if (IsZoomed(hwnd)) { 
				bMaximized = 1;
				break;
			}
			bMaximized = 0;
/*			Get here only if NOT iconic and NOT full-screen (zoomed) */
			GetWindowRect(hWnd, &WinRect); 
			cxWidth= WinRect.right - WinRect.left;	/* width of Window */
			cyHeight = WinRect.bottom - WinRect.top; /* heigh to window */
/*			sprintf(str,
				"nWidth %d nHeight %d WM_SIZE\ncxWidth %d cyHeight %d WM_SIZE GetWindowRect", 
					nWidth,	nHeight, cxWidth, cyHeight);
			wininfo(str); */ /* debugging */
			break;

		case WM_GOTOPAGE:					/* request from client */
			if (bFileValid == 0) break;		/* ignore if no file open */
			dvipage = wParam;
			usepagetable(dvipage, 0);		/* page is physical page */
			checkcontrols(hWnd, +1);
			SetupWindowText(hWnd);
			if (bResetPage != 0) {
				resetpagemapping(hWnd);
				newmapandfonts(hWnd);
				resetgraphicstate();
			}
			hyperindex = 0;
			colorindex = 0;
			bEnableTermination = 1;		/* NEW */
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		default:	/* if no one wants it, pass to default handler */
			if( bWin95) {
				if (message == uMSH_MOUSEWHEEL ) { //	in Windows 95 and NT 3.51
					zDelta = (int) wParam;	// wheel rotation 
					fwKeys = 0;
					if (GetAsyncKeyState(VK_SHIFT)) fwKeys |= MK_SHIFT;
					if (GetAsyncKeyState(VK_CONTROL)) fwKeys |= MK_CONTROL;
					fwKeys |= (zDelta << 16);	// stick delta into HIWORD
//					convert to WM_MOUSEWHEEL message 
					SendMessage(hwnd, WM_MOUSEWHEEL, fwKeys, lParam);
					break;					// return TRUE ?
				}
			}

			return (DefWindowProc(hWnd, message, wParam, lParam));
	}		/* end of switch (message) */

/*	Following not Kosher in WIN 32 */
/*	return (NULL); */
	return (0);
}

/****************************************************************************/

/* void ChangeDirectory(char *pathname) */
int ChangeDirectory (char *pathname, int verbose) {
	int c, d = 0;
	int flag = 0;
	char *t;
	char szDrive[32];
	int curdrive;

/*	winerror(pathname); */
	t = pathname + strlen(pathname) - 1;	/* 1993/Dec/ 9 */
	if (*t == '\\' || *t == '/') {
		d = *t;			/* remember trailing delimiter */
		*t = '\0';		/* remove trailing backslash */
	}
/*	a test - restore trailing delimiter  --- 1996/Jan/30 */
/*	if (*(t+1) == '\0' && d != 0) *t = (char) d; */
/*	should test *(t-1) == ':' so can check whether top level --- e:\ */
	if (t > pathname && *(t-1) == ':' && d != 0) *t = (char) d;

/*	fix this for WIN32 ??? */
/*	DWORD GetCurrentDirectory(DWORD nBufferLength, LPTSTR lpBuffer); */
/*	returns zero if it fails */

	(void) GetCurrentDirectory(sizeof(szDrive), szDrive);
	curdrive = *szDrive;			/* just for debug output ... */
	if (curdrive >= 'a') curdrive = curdrive - 'a' + 1;
	else curdrive = curdrive - 'A' + 1;

/*	curdrive = _getdrive();	*/

#ifdef DEBUGCHANGE
	if (bDebug > 1) {
/*		if (verbose) OutputDebugString("Change Directory: ");
		else OutputDebugString("Working Directory: "); */
/*		OutputDebugString("Change Directory: "); */
/*		OutputDebugString(pathname); */
		sprintf(debugstr, "CD: %s\n", pathname);
		OutputDebugString(debugstr);
	}
#endif

/*	try and change drive - but only if a drive is specified */
	if (strchr(pathname, ':') != NULL) { 
		if (*(pathname+1) == ':') {
			c = pathname[0]; 
/*	compute drive number given drive letter */
			if (c >= 'A' && c <= 'Z') c = c - 'A' + 1;
			if (c >= 'a' && c <= 'z') c = c - 'a' + 1;
/*			if (bDebug > 1) {
				sprintf(debugstr, "Changing to drive %c: (%d)\n", pathname[0], c);
				OutputDebugString(debugstr);
			} */ 	/* debugging 95/April/20 */
/*			Will the following `DOS' code work in WIN32 ? */

			if (SetCurrentDirectory(pathname) == 0) 	/* WIN32 ??? */

/*			if (_chdrive(c) != 0)  */

			{
				sprintf(debugstr, "Drive %c (%d) not valid (%s) current %c (%d)\n",
					pathname[0], c, pathname, 'a' + curdrive - 1, curdrive);
/*				if (verbose) winerror(str); */
				if (bDebug > 1) OutputDebugString(debugstr);
				flag = 1;
			}
/*			else winerror("CHANGED DRIVE"); */
		}
	} 

/*	try and change directory - but only if a directory is specified ? */
	if (strrchr(pathname, '\\') != NULL || strrchr(pathname, '/') != NULL) {
/*		if (chdir(DefPath) != 0) */
/*		if (_chdir(DefPath) != 0) */
/*		if (bDebug > 1) {
			sprintf(debugstr, "Changing to directory `%s'\n", pathname);
			OutputDebugString(debugstr);
		} */ /* debugging 95/April/20 */
/*		Will the following `DOS' code work in WIN32 ? */

		if (SetCurrentDirectory(pathname) == 0) 

/*		if (_chdir(pathname) != 0)  */
		{
			sprintf(debugstr, "Directory `%s' not found\n", pathname);
			if (verbose) winerror(debugstr);
			else if (bDebug > 1) OutputDebugString(debugstr);
			flag = 1;
		}
	}
/*	add the trailing \ again if it was removed above 1993/Dec/9 */
	if (d != 0) *t = (char) d;
	return flag;
}

int NewChangeDirectory (LPSTR path, int nFileOffset) {	/* 1994/Aug/12 */
	char pathname[MAXFILENAME];

	lstrcpy(pathname, path);
	*(pathname + nFileOffset) = '\0';
	return ChangeDirectory(pathname, 1);
}

void trailingback (char *pathname) { /* make sure it has trailing '\\' */
	char *t;
	t = pathname + strlen(pathname) - 1;
	if (*t != '\\' && *t != '/') strcat(pathname, "\\");
}

void removeback (char *pathname) { /* remove trailing '\\' */
	char *t;
	t = pathname + strlen(pathname) - 1;
	if (*t == '\\' || *t == '/') *t = '\0';
}

void doubledot (char *pathname) {	/* implement ".." in file name */
	char *s, *t;

	t = pathname + strlen(pathname) - 1;
/*	if (*t == '\\') *t-- = '\0'; */
/*	if (*t == '\\' && *t == '/') *t-- = '\0'; */		/* bug, fix 94/Jan/7 */
	if (*t == '\\' || *t == '/') *t-- = '\0';
	if ((s = strrchr(pathname, '\\')) != NULL) ;
	else if ((s = strrchr(pathname, '/')) != NULL) ;	/* added 94/Jan/7 */
	else if ((s = strrchr(pathname, ':')) == NULL) s++;
	else s = t + 1;
	*s = '\0';
	trailingback(pathname);		/* make sure it has trailing \ */
}

/*  Take apart  OpenName => DefPath, DefSpec, DefExt */
/*	NOTE: Overwrites OpenName with just file name itself */
/*	USE: from callpreview and calldocument and winsearc.c */
/*	USE: from OpenDlg in dviwindo.c */

void ParseFileName (void) {
/*	_getcwd(DefPath, MAXPATHLEN);	*/	/* current directory */
	trailingback(DefPath);				/* make sure it has trailing \ ? */
	SeparateFile((LPSTR) str, (LPSTR) DefSpec, (LPSTR) OpenName);
	if (strchr(str, ':') != 0) strcpy(DefPath, str);
	else if (strncmp(str, "\\\\", 2) == 0) strcpy(DefPath, str); // 2000/March/15
	else if (strstr(str, "..") != NULL) doubledot(DefPath);
	else strcat(DefPath, str);			// ???
	strcpy(OpenName, DefSpec);			// here DefSpec is actual file name ?
	ChangeDefExt(DefExt, DefSpec);
//	if (bDebug > 1) {
//		sprintf(debugstr, "Parse '%s': Path '%s' Spec '%s' Ext '%s'",
//				str, DefPath, DefSpec, DefExt);
//		OutputDebugString(debugstr);
//		wincancel(debugstr);	// debugging only
//	}
}

HFILE DoOpenFile (HWND hWnd) {						/* initial file open */
/*	int hFileNew; */
	HFILE hFileNew;
	DWORD err;										/* 1996/Jan/10 */
	char drive[MAXPATHLEN];
	char *s;

#ifdef LONGNAMES
/*	There may be a problem here if OpenName is fully qualified ! */
/*	strcpy(FileName, DefPath); */		/* fixed 1996/Jan/10 */
	if (strchr(OpenName, '\\') != NULL ||
		strchr(OpenName, '/') != NULL ||
		strchr(OpenName, ':') != NULL) *FileName = '\0';
	else strcpy(FileName, DefPath);
	strcat(FileName, OpenName);
/*	hFileNew = _lopen(FileName, READ | OF_SHARE_DENY_WRITE); */
	hFileNew = _lopen(FileName, READ | OfShareCode);
	if (hFileNew == HFILE_ERROR) {
/*		construct error message according to DOS file open error code */
/*		but where does _lopen hide the error information ? */

		err = GetLastError();

/*		err = errno; */
/*		doserror((int) errno, str, sizeof(str), OpenName); */
/*		not sure this makes sense, since err not a DOS error code ... */
/*		doserror((int) err, str, sizeof(str), FileName); 
		(void) MessageBox(hWnd, str, "Open File", MB_ICONSTOP | MB_OK); */
		(void) ExplainError("Open File", err, 1);			/* 99/Jan/3 */
		return -1;
	}
#else /* not LONGNAMES */
/*	if ((hFileNew = OpenFile(OpenName, (LPOFSTRUCT) &OfStruct, */
	hFileNew = OpenFile(OpenName, &OfStruct, 
/*			OF_READ)) == -1)  */
/*			OF_SHARE_DENY_WRITE | OF_READ)) == -1) */
/*			OF_SHARE_DENY_NONE | OF_READ)) == -1)  */
			OF_READ | OfShareCode);
	if (hFileNew == HFILE_ERROR) { 
/*		construct error message according to DOS file open error code */
/*		doserror((int) OfStruct.nErrCode, str, sizeof(str), OpenName);
		(void) MessageBox(hWnd, str, "Open File", MB_ICONSTOP | MB_OK); */
		(void) ExplainError("Open File", err, 1);			/* 99/Jan/3 */
		return -1;			/* invalid DOS file handle */
	}
#endif

/*	try and guess whether file on floppy diskette or not */
/*	treat removable media and remote drives as slow - don't probe as often */

/*	new code - check this works in WIN32 ... */
		if ((s = strchr(DefPath, ':')) != NULL) {
			strncpy(drive, DefPath, s - DefPath + 1);
			*(drive + (s - DefPath + 1)) = '\0';
			strcat(drive, "\\");
			if (GetDriveType (drive) == DRIVE_FIXED) bFloppyDisk=0;
			else bFloppyDisk = 1; 
		}

#ifdef IGNORED
#ifdef LONGNAMES
/*  reinstated this code 95/Dec/6 for non WIN32 case */
		c = DefPath[0];
		if (c >= 'A' && c <= 'Z') c = c  + 'a' - 'A';
/*		Use DriveType = GetDriveType (drive); */
		if (strchr(DefPath, ':') != NULL && 
			GetDriveType(c - 'a') == DRIVE_FIXED) bFloppyDisk=0;
		else bFloppyDisk = 1; 
#else
/* this is smoother way to figure out the same thing ... if OfStruct exist */
		if (OfStruct.fFixedDisk == 0) bFloppyDisk = 1;	
		else bFloppyDisk = 0;
#endif	/* end LONGNAMES */
#endif	/* end not WIN32 */

/* succeeded to open the file - grab some info now (for reopening later) */

		NewFileSizeLow = GetFileSize ((HANDLE) hFileNew, &NewFileSizeHigh);
		(void) GetFileTime ((HANDLE) hFileNew, NULL, NULL, &NewFilewTime);
		FileSizeLow = NewFileSizeLow;
		FileSizeHigh = NewFileSizeHigh;
		FilewTime = NewFilewTime;
/*		if (bDebug > 1) ShowFileInfo("DoOpenFile", NewFileSizeHigh, NewFileSizeLow,
				 NewFilewTime.dwHighDateTime, NewFilewTime.dwLowDateTime); */

/*		(void) _fstat(hFileNew, &FileStatus); */
/*		FileSize = FileStatus.st_size; */ 
/*		FilemTime = FileStatus.st_mtime; */

/*		File is opened, return the handle to the caller.	*/
		return hFileNew;
}

/* New, added in case DVIWINDO.DLG gets edited and ES_OEMSTYLE is lost */

void SetOEMConvertStyle(HWND hDlg, WORD id, BOOL flag) {
	HWND hctl = GetDlgItem(hDlg, id);
	LONG style = GetWindowLong(hctl, GWL_STYLE);
	if (flag) style |= ES_OEMCONVERT;
	else style &= ~ES_OEMCONVERT;
	SetWindowLong(hctl, GWL_STYLE, style);
}

/* following is the old way of doing this */

/****************************************************************************

	FUNCTION: OpenDlg(HWND, unsigned, WORD, LONG)

	PURPOSE: Let user select a file, and open it.

****************************************************************************/

/* BOOL CALLBACK OpenDlg(HWND hDlg, unsigned message, WORD wParam, LONG lParam) */
BOOL CALLBACK _export OpenDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int inx;
	int hFileNew;
	WORD id, cmd;
	char *s;

	switch (message) {

/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
		case WM_COMMAND:
			id = (WORD) GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id) {

				case IDC_LISTBOX:				/* file name list box */
/*					switch (HIWORD(lParam)) */
					switch (cmd) {

						case LBN_SELCHANGE:
							/* If item is a directory name, append DefSpec */
							if (DlgDirSelectEx(hDlg, str, sizeof(str), IDC_LISTBOX))
								strcat(str, DefSpec);	
/* impossible */ /* retrieves current selection */
/*							OemToAnsi(str, str);	*/	/* convert to ANSI */
							SetDlgItemText(hDlg, IDC_EDIT, str);
/*							(void) SendDlgItemMessage(hDlg,
								IDC_EDIT,
								EM_SETSEL,
								0,
								MAKELONG(0, 0x7fff)); */ /* WIN16 only ??? */
/*							(void) SetFocus(GetDlgItem(hDlg, IDC_EDIT)); */
/*							return (FALSE);	*/ /* ??? */
							return (TRUE);	/* ??? */

						case LBN_DBLCLK:
							goto openfile;	/* ouch ! */

						default:
							return (TRUE);	/* ??? */
					}
/*					return (TRUE); */

				case IDC_DIRECTORY:						/* NEW */
/*					switch (HIWORD(lParam)) */
					switch (cmd) {

						case LBN_SELCHANGE: 	/* directory name list box */
							/* If item is a directory name, append "*.*" */
							if (DlgDirSelectEx(hDlg, str, sizeof(str), IDC_DIRECTORY))
								strcat(str, DefSpec);   
/* always directory *//* retrieve curr select */
/*							OemToAnsi(str, str);	*/	/* convert to ANSI */
							SetDlgItemText(hDlg, IDC_EDIT, str);
/*							(void) SendDlgItemMessage(hDlg,
								IDC_EDIT,
								EM_SETSEL,
								0,
								MAKELONG(0, 0x7fff)); */ /* WIN16 only ??? */
/*							(void) SetFocus(GetDlgItem(hDlg, IDC_EDIT)); */
/*							return (FALSE);	*/
							return (TRUE);	/* ??? */

						case LBN_DBLCLK:
							goto openfile;	/* ouch ! */

						default:
							return(TRUE);	/* ??? */
					}
/*					return (TRUE); */

				case IDOK:
				openfile:
					(void) GetDlgItemText(hDlg, IDC_EDIT,
/*						OpenName, MAXFILENAME);		*/	/* get file name */
						OpenName, sizeof(OpenName));	/* get file name */
/*					AnsiToOem (OpenName, OpenName);	*/ /* ??? */
/*					if it contains wildcards, then we are not done yet */
					if (strchr(OpenName, '*') || strchr(OpenName, '?')) {
/* attempt to get directory from IDC_PATH field */
/* but if it is too long, then it may have ellipsis in it ... */
						(void) GetDlgItemText(hDlg, IDC_PATH,
							str, MAXPATHLEN);	/* possibly corrupted ... */
/* the above assumes BUFLEN (str) >= MAXPATHLEN */
						if (strstr(str, "...") == NULL) strcpy(DefPath, str);
/*						else getcwd(DefPath, MAXPATHLEN); */
/*						else getcwd(DefPath, sizeof(DefPath)); */
						else {

							(void) GetCurrentDirectory(sizeof(DefPath), DefPath);

							s = DefPath + strlen(DefPath) - 1;	/* 1995/Dec/18 */
							if (*s != '\\' && *s != '/') strcat(DefPath, "\\");
						}
/*						_getcwd(DefPath, sizeof(DefPath)); */ /* just do */
						ParseFileName();
						strcpy(DefSpec, "*");		/* ??? */
						strcat(DefSpec, DefExt);	/* fix up DefSpec */
/*			force update of list boxes and directory - use wild card */
						UpdateListBox(hDlg);
						return (TRUE);
					}

/*	get here when no wild cards */
					
/*					if (strchr(OpenName, ':') || strchr(OpenName, '\\')) */
					if (strchr(OpenName, ':') ||
						strchr(OpenName, '\\') ||
						strchr(OpenName, '/')) {
/*	get here is user specified full file name directly */
						(void) GetDlgItemText(hDlg, IDC_PATH,
							str, MAXPATHLEN);	/* possibly corrupted ... */
/* the above assumes BUFLEN (str) >= MAXPATHLEN */
						if (strstr(str, "...") == NULL) strcpy(DefPath, str);
/*						else getcwd(DefPath, MAXPATHLEN); */
/*						else getcwd(DefPath, sizeof(DefPath)); */
						else {

							(void) GetCurrentDirectory(sizeof(DefPath), DefPath);

							s = DefPath + strlen(DefPath) - 1;	/* 1995/Dec/18 */
							if (*s != '\\' && *s != '/') strcat(DefPath, "\\");
						}
/*						_getcwd(DefPath, sizeof(DefPath)); */ /*  just do */
						ParseFileName();
						strcpy(DefSpec, "*");
						strcat(DefSpec, DefExt);
/*			force update of list boxes and directory - use wild card */
						UpdateListBox(hDlg);
						SetDlgItemText(hDlg, IDC_EDIT, OpenName);
					}

					if (!OpenName[0]) {
						(void) MessageBox(hDlg, "No filename specified.",
							"Open File", MB_ICONSTOP | MB_OK);
						return (TRUE);
					}

					AddExt(OpenName, DefExt);
					hFileNew = DoOpenFile(hDlg); /* (Try to) Open the file */
					if (hFileNew >= 0) EndDialog(hDlg, hFileNew);
					return(TRUE);

				case IDCANCEL:
/*					EndDialog(hDlg, NULL); */ /* we loose */
					EndDialog(hDlg, -1); /* we loose */
					return (TRUE);
				default:
					return(FALSE);	/* ??? */
			}

/* end of WM_COMMAND case */

		case WM_INITDIALOG:						/* message: initialize	*/
			UpdateListBox(hDlg);
/*			OemToAnsi(DefSpec, str);
			SetDlgItemText(hDlg, IDC_EDIT, str); */
			SetOEMConvertStyle(hDlg, IDC_EDIT, TRUE); /* if dropped from RC */
			SetDlgItemText(hDlg, IDC_EDIT, DefSpec);
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details         */
/*		EM_SETSEL in WIN32 wParam starting position, lParam ending position */
			(void) SendDlgItemMessage(hDlg, 	/* dialog handle	*/
				IDC_EDIT,						/* where to send message */
				EM_SETSEL,						/* select characters	*/
				0, -1
/*				0, MAKELONG(0, 0x7fff) */
			);									/* select entire contents */
			(void) SetFocus(GetDlgItem(hDlg, IDC_EDIT)); /* overridden ? */
/*			somehow select previous file here to start with if possible ? */
/*			weird useage: -1 for (WORD) => 0xFFFF instead */
/*			UINT is 16 bit in WIN16 and 32 bit in WIN32 */
			inx = (int) SendDlgItemMessage(hDlg, IDC_LISTBOX,
					(UINT) LB_FINDSTRING, 0xFFFF, (LONG) (LPSTR) OpenName);
			if (inx != LB_ERR) {
				(void) SendDlgItemMessage(hDlg, IDC_LISTBOX, LB_SETCURSEL,
					(WPARAM) inx, 0L);
/*				OemToAnsi(OpenName, str);
				SetDlgItemText(hDlg, IDC_EDIT, str); */
				(void) SetFocus(GetDlgItem(hDlg, IDC_LISTBOX));	 /* NEW */
				SetDlgItemText(hDlg, IDC_EDIT, OpenName);
/* following was on trial ... */
/*				(void) SendDlgItemMessage(hDlg,
					IDC_EDIT,					
					EM_SETSEL,					
					0,						
					MAKELONG(0, 0X7FFF));	*/ /* WIN16 only ? */
/*				(void) SetFocus(GetDlgItem(hDlg, IDC_EDIT)); */
			}
			return (FALSE); /* Indicates we set the focus to a control */

			default: return (FALSE); /* ??? */
	}
/*	return FALSE; */
}	/* lParam unreferenced */

/****************************************************************************

	FUNCTION: UpdateListBox(HWND);

	PURPOSE: Update the list box of OpenDlg

****************************************************************************/

void UpdateListBox (HWND hDlg) {

	strcpy(str, DefPath);
	strcat(str, DefSpec);
	SetDlgItemText(hDlg, IDC_PATH, DefPath);	/* in case following fails */
	if (DlgDirList(hDlg, str, IDC_DIRECTORY, IDC_PATH, 0xC010) == 0) {
/*	directory & drive listing failed (floppy not ready or bad directory) */
/*  try (A): (suggest moving up in directory tree) */
/*		SendDlgItemMessage(hDlg, IDC_DIRECTORY, LB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDC_DIRECTORY, LB_ADDSTRING,
			0, (LONG) (LPSTR) "[..]"); */
/*	or try (B): (list drives only) */
		DefPath[0] = '\0';
		DlgDirList(hDlg, DefPath, IDC_DIRECTORY, IDC_PATH, 0xC000); /* TRY */
/*		winerror("DIRECTORY DefPath+DefSpec failed"); */
	}
	if (DlgDirList(hDlg, str, IDC_LISTBOX, IDC_PATH, 0x0000) == 0) {
		SendDlgItemMessage(hDlg, IDC_LISTBOX, LB_RESETCONTENT, 0, 0L);
/*	file listing failed (floppy not ready or bad directory ?) */
/*  try (A): (suggest moving up in directory tree) */
		SendDlgItemMessage(hDlg, IDC_DIRECTORY, LB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDC_DIRECTORY, LB_ADDSTRING,
			0, (LONG) (LPSTR) "[..]");
/*	or try (B): (list drives only) */
/*		DlgDirList(hDlg, "", IDC_DIRECTORY, IDC_PATH, 0xC000); */
/*		winerror("LISTBOX DefPath+DefSpec failed"); */
	}

/* 0x4000 => drives & 0x010 => subdirectories  page 4-101 in volume 1 */
/* 0x8000 => exclusive bit ---  use to get separate listbox with drives ? */

/*	To ensure that the listing is made for a subdir of current drive dir */
	if (!strchr (DefPath, ':')) { /* if path does not contain drive	letter */
		SetDlgItemText(hDlg, IDC_PATH, "");	/* if following fails */
		if (DlgDirList(hDlg, DefSpec, IDC_DIRECTORY, IDC_PATH, 0xC010) == 0) {
/*			winerror("DIRECTORY DefSpec failed"); */
		}
		if (DlgDirList(hDlg, DefSpec, IDC_LISTBOX, IDC_PATH, 0x0000) == 0) {
/*			winerror("LISTBOX DefSpec failed"); */
		}
/* possibly modify DefPath ? here by copying part of DefSpec ? */
	}
/*	somehow the drive doesn't get reinserted unless we click on a drive ?? */

	/* Remove the '..' character from path if it exists, since this
	* will make DlgDirList move us up an additional level in the tree
	* when UpdateListBox() is called again.
	*/
/*	GetDlgItemText(hDlg, IDC_PATH, DefPath, sizeof(DefPath)); */  /* ??? */
/*	if (strstr (DefPath, ".."))	DefPath[0] = '\0'; */ /* experiment ??? */
/*	if ((s = strstr(DefPath, "..")) != NULL) strcpy(s, s+2); */
/*	OemToAnsi(DefSpec, str);
	SetDlgItemText(hDlg, IDC_EDIT, str); */
	SetDlgItemText(hDlg, IDC_EDIT, DefSpec);
}

/****************************************************************************

	FUNCTION: ChangeDefExt(PSTR, PSTR);

	PURPOSE: Change the default extension (if extension specified & not wild)

	USE: by ParseFileName only

****************************************************************************/

void ChangeDefExt (CHAR *szExt, CHAR *szName) {
	CHAR *s;

//	s = szName;
//	while (*s != '\0' && *s != '.')	s++;
//	if (*s != '\0')
//		if (!strchr(s, '*') && !strchr(s, '?'))	strcpy(szExt, s);
	if ((s = strrchr(szName, '.')) != NULL) {
		if (! strchr(s, '*') && ! strchr(s, '?'))
			strcpy(szExt, s);		// includes '.'
	}		
}

/****************************************************************************

	FUNCTION: SeparateFile(LPSTR lpPath, LPSTR lpFileName, LPSTR lpSrcFileName)

	PURPOSE: Separate filename and pathname in last argument

	Destination Path, Destination FileName, Source FileName

****************************************************************************/

int SeparateFile (LPSTR lpDestPath, LPSTR lpDestFileName, LPSTR lpSrcFileName) {
	LPSTR lpTmp;
//	char  cTmp;

/*  search backward for last ':' or '\\' or '/' */
	lpTmp = lpSrcFileName + (long) lstrlen(lpSrcFileName);
	while (*lpTmp != ':' && *lpTmp != '\\' && *lpTmp != '/' &&
			lpTmp > lpSrcFileName)
		lpTmp = AnsiPrev(lpSrcFileName, lpTmp);
	if (*lpTmp != ':' && *lpTmp != '\\' && *lpTmp != '/') {	/*  no path */
		lstrcpy(lpDestFileName, lpSrcFileName);
		lpDestPath[0] = '\0';			/* indicate lack of path */
		return 0;
	}
	lstrcpy(lpDestFileName, lpTmp + 1);		// file name
//	cTmp = *(lpTmp + 1);
//	*(lpTmp + 1) = 0; 
	lstrcpy(lpDestPath, lpSrcFileName);		// path
//	*(lpTmp + 1) = cTmp;
	lpDestPath[(lpTmp - lpSrcFileName) + 1] = '\0';
//	winerror(lpDestPath);					// debugging only
	return 1;
}

/****************************************************************************

	FUNCTION: AddExt(PSTR, PSTR);

	PURPOSE: Add default extension (if none present)

***************************************************************************/

void AddExt (char *szName, char *szExt) {
	char *s;

//	s=szName;
//	while (*s != '\0' && *s != '.')	s++;
//	if (*s != '.') strcat(szName, szExt);
	if ((s = strrchr(szName, '.')) == NULL) strcat(szName, szExt);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

BOOL RegisterControlClass (HINSTANCE hInstance) {
	WNDCLASS wc;
	int flag;
	
/*	omitt the CS_GLOBALCLASS ? */
	wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
/*	wc.lpfnWndProc   = SpinWndFn; */
	wc.lpfnWndProc   = (WNDPROC) SpinWndFn;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = CBWNDEXTRA;
	wc.hInstance     = hInstance;
/*	wc.hIcon         = NULL; */
	wc.hIcon         = 0;
/*	wc.hCursor       = LoadCursor(NULL, IDC_ARROW); */
	wc.hCursor       = LoadCursor(NULL, (LPCSTR) IDC_ARROW);
/*	wc.hbrBackground = (COLOR_BTNFACE + 1); */
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szControlName;
	flag = RegisterClass(&wc);
/*	if (flag == NULL) */				/* 1993/Sep/7 */
	if (flag == 0) {				/* 1993/Sep/7 */
/*  If conflict with some other application trying to globally register */
		if (bDebug > 1)	OutputDebugString("Custom Control Register Failed\n");
		szControlName = szControlNameAlt;	/* use alternate class name */
/*		strcpy (szControlName, szControlNameAlt); */	/* ha ha */
		wc.lpszClassName = szControlName;
		flag = RegisterClass(&wc);			/* and try again */
/*		if (flag == NULL) */
		if (flag == 0) {
/*	Desperation: try making task specific class - should never get here! */
			wc.style &= CS_GLOBALCLASS;
			flag = RegisterClass(&wc);		/* and try again */
		}
	}
/*	possible problem now with second instance assuming has been registered */
	return flag;
}

/* An application cannot register a global class if either a global class,
   or a task specific class already exists with the given name */

/* An application can register a task-specific class with the same name
   as a global classs.  The task-specific class overrides the global task */

/****************************************************************************/

// Place for TeX to ask for information from user:
// Gets passed along to yandytex.dll
// puts questionstring and helpstring into str for TeXDlg
// should return 0 if "EOF on terminal"

int ConsoleInput (char *questionstr, char *helpstr, char *buffer) {
	int flag=0;
	char *s;
//	if (questionstr != NULL) winerror(questionstr);
	if (questionstr != NULL) strcpy(str, questionstr);
	else *str = '\0';
//	if (helpstr != NULL) winerror(helpstr);
	s = str + strlen(str) + 1;
	if (helpstr != NULL) strcpy(s, helpstr);	// set up argument
	else *s = '\0';
//	transmit two strings stacked in str
//	flag = DialogBoxParam(hInst, "TeX", hWnd, FileDlg, 4L);
//	flag = DialogBoxParam(hInst, "FileSelect", hwnd, FileDlg, 4L);
	flag = DialogBoxParam(hInst, "TeXinput", hwnd, TeXDlg, 0);
//	returns length of string + 1, or 0 if CANCEL pressed
	if (buffer != NULL) strcpy(buffer, str);	// return result
//	winerror(buffer);
//	sprintf(str, "flag %d", flag);
//	winerror(str);
	if (flag < 0) ShowLastError();
	if (_stricmp(str, "^z") == 0) return 0;	// emergency stop
//	if (wincancel(str)) exit(1);
	if (flag > 0) return 1;			// OK
	else return 0;					// CANCEL
}

// in tex0.c, returning non-zero means EOF on terminal
// in winprint.c FileDlg returns non-zero if OK ???

/****************************************************************************/

#ifdef IGNORED

#include "fcntl.h"

char *shortname =
"d:\\foo_bar_magic_long_name\\continuing_along_the_lines\\build_up_a_very_nasty\\problem_for_file_opening\\and_so_we_keep_on_adding\\more_and_more_stuff\\until_things_just_get_out_of_hand\\not_yet_perhaps_later\\we_must_persist\\even_yet_we_have_not_yet\\foo.txt";

char *longname =
"d:\\foo_bar_magic_long_name\\continuing_along_the_lines\\build_up_a_very_nasty\\problem_for_file_opening\\and_so_we_keep_on_adding\\more_and_more_stuff\\until_things_just_get_out_of_hand\\not_yet_perhaps_later\\we_must_persist\\even_yet_we_have_not_yet\\hello_world.text";

char *spacename = "d:\\foo bar chomp\\hello.txt"; 

/* 0 exist only, 2 write permission, 4 read permission, 6 both */
/* can be used to test for existence of directories also */
/* errno = EACCESS = 13 access denied ENOENT = 2 file or path not found */

#define ACCESSCODE 4

void checkname(char *name) {
/*	FILE *input; */
/*	int c; */
	char buffer[2];
	char *s;
	int nFile;
	HFILE hFile;
	OFSTRUCT of;
	int code, result;
	struct _stat buf;
	
/*	sprintf(str, "Trying to fopen\n%s\nLength %d\n", name, strlen(name));
	wininfo(str);
	input = fopen(name, "r");
	if (input == NULL) {
		winerror("FAILED fopen");
	}
	else {
		s = str;
		while ((c = getc(input)) > 0) *s++ = (char) c;
		*s++ = '\n'; *s = '\0';
		wininfo(str);
		fclose(input);
	} */
	sprintf(str, "Trying to _open\n%s\nLength %d\n", name, strlen(name));
	wininfo(str);
	nFile = _open(name, _O_RDONLY);
	if (nFile < 0) {
		winerror("FAILED _open");
	}
	else {
		s = str;
		while (_read(nFile, buffer, 1) > 0)	*s = *buffer;
		*s++ = '\n'; *s = '\0';
		wininfo(str);
		_close(nFile);
	}
	sprintf(str, "Trying to _lopen\n%s\nLength %d\n", name, strlen(name));
	wininfo(str);
/*	hFile = _lopen(name, READ); */
	hFile = _lopen(name, READ | OfShareCode);	/* 96/May/18 ? */
/*	if (hFile < 0) */
	if (hFile == HFILE_ERROR) {
		winerror("FAILED _lopen");
	}
	else {
		s = str;
		while (_lread(hFile, buffer, 1) > 0) *s++ = *buffer;
		*s++ = '\n'; *s = '\0';
		wininfo(str);
		result = _fstat(hFile, &buf);
		s = ctime(&buf.st_atime);
		if (s != NULL) {	/* s == NULL only if date/time corrupted */
			sprintf(str, "File size %ld\tDrive Number %d\tTime modified %s",
				buf.st_size, buf.st_dev, s);
			wininfo(str);
		}
		_lclose(hFile);
		hFile = HFILE_ERROR;				/* 95/Dec/20 */
	}
	sprintf(str, "Trying to OpenFile\n%s\nLength %d\n", name, strlen(name));
	wininfo(str);
	hFile = OpenFile(name, &of, OF_READ);
/*	if (hFile < 0) */
	if (hFile == HFILE_ERROR) {
		winerror("FAILED OpenFile");
	}
	else {
		s = str;
		while (_lread(hFile, buffer, 1) > 0) *s++ = *buffer;
		*s++ = '\n'; *s = '\0';
		wininfo(str);
		_lclose(hFile);
		hFile = HFILE_ERROR;				/* 95/Dec/20 */
	}
	code = _access(name, ACCESSCODE);
	sprintf(str, "Access code is %d errno %d", code, errno);
	wininfo(str);
}

void checklong (void) {
	checkname(longname);
	checkname(shortname);
	checkname(spacename);
}

#endif

/****************************************************************************/

/* use -Fpi for CL if floating point needed */
/* use WIN87EM.LIB for LINK if floating point needed */

/* REMEMBER TO EXPORT IN THE DEF MODULE ALL CALLBACK FUNCTIONS: */
/*   MainWndProc	@1	About		@2	OpenDlg		@3 */
/*   PageDlg		@4	FontDlg		@5	DVIMetric	@6 */

/* in dragging, stay in tight loop: */
/* do { ... } while  (GetAsyncKeyState(VK_LBUTTON) & 0x8000); */

/* don't bother setting bShiftFlag, bControlFlag etc, use GetAsyncKeyState */

/* use assert(<exp>);  #include <assert.h> */

/* primitive client-server interface using just PostMessage */

/* when starting, client does WinExec of LoadModule with command line */
/* that contains -m=<client window handle> -p=<page number> <file-name> */
/* WM_IAMHERE	send window handle - server back to client */
/* WM_GOTOPAGE  request specific page - client to server */
/* WM_MARKHIT   specified mark has been hit - server to client */
/* WM_GOODBYE	shut down - client to server */

/* The DVI file contains \special{button: <number> <xll> <yll> <xur> <yur>} */
/* WM_MARKHIT is sent when user clicks in specified bounding box */

/* use OF_SHARE_DENY_NONE ??? instead of OF_SHARE_DENY_WRITE ??? */

/* or try OF_SHARE_COMPAT ??? that is 0 - no, that seems to lead to problems */ 
/* bSearchFlag may not be needed - supposed to avoid reentrancy */

/* 640 x 480   800 x 600  1024 x 768  1152 x 900  1280 x 1024 */

/* `texfonts.map' support added 94/March/18 */

/* According to EXEHDR, this has 512 bytes in the heap */
/* and an `extra stack allocation' of 5400 bytes */

/* _lopen expects Windows character set file name */
/* OpenFile expects OEM character set filename */

/* ShowWindow As noted in the discussion of the nCmdShow parameter,
the nCmdShow value is ignored in the first call to ShowWindow
if the program that launched the application specifies
startup information in the STARTUPINFO structure.
In this case, ShowWindow uses the information specified
in the STARTUPINFO structure to show the window.
On subsequent calls, the application must call ShowWindow
with nCmdShow set to SW_SHOWDEFAULT to use the startup information
provided by the program that launched the application.
This behavior is designed for the following situations: */
