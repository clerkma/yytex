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

#define huge
#define _export
/* #define FAR */
/* #define NEAR */
/* #define PASCAL WINAPI */

#define USEMEMCPY

/* #define HINSTANCE_ERROR 32 */			/* from winbase.h 32 bit */
#undef HINSTANCE_ERROR
#define HINSTANCE_ERROR ((HINSTANCE)32)		/* from windows.h 16 bit */

/* Note: cannot use IGNORE, since that is defined to be 0 in winbase.h */

/* functions in DVIWINDO.C */
/* int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int); */
/* BOOL InitApplication(HANDLE); */
BOOL InitApplication(HINSTANCE);
/* BOOL InitInstance(HANDLE, int, int, int, int, int); */
BOOL InitInstance(HINSTANCE, int, int, int, int, int);
/* WNDPROC's follow */
/* long FAR PASCAL MainWndProc(HWND, unsigned, WORD, LONG); */
/* long FAR PASCAL MainWndProc(HWND, UINT, WPARAM, LPARAM); */
/* long CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM); */
LRESULT CALLBACK _export MainWndProc(HWND, UINT, WPARAM, LPARAM);
/* BOOL FAR PASCAL OpenDlg(HWND, unsigned, WORD, LONG); */
/* BOOL FAR PASCAL OpenDlg(HWND, UINT, WPARAM, LPARAM); */
/* BOOL CALLBACK OpenDlg(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export OpenDlg(HWND, UINT, WPARAM, LPARAM);
/* CALLBACKS above */
/* static BOOL NEAR PASCAL RegisterControlClass (HANDLE); */
/* BOOL RegisterControlClass (HANDLE); */
BOOL RegisterControlClass (HINSTANCE);
/* void SeparateFile(HWND, LPSTR, LPSTR, LPSTR); */
void UpdateListBox(HWND);
/* void AddExt(PSTR, PSTR); */
/* void ChangeDefExt(PSTR, PSTR); */
void AllocPages(int);
void GrabPages(void);
void ReleasePages(void);
void AllocColor(int);
void GrabColor(void);
void FreeColor(void);
void ReleaseColor(void);
void AllocBack(int);
void GrabBack(void);
void ReleaseBack(void);
/* void AllocMarks(int); */
/* void GrabMarks(void); */
/* void ReleaseMarks(void); */
void AllocTPIC(int);
void GrabTPIC(void);
void ReleaseTPIC(void);
void GrabWidths (void);
void ReleaseWidths (void);
void RestoreBack(int);
int checkuser(void);
/* void HideScrollBars(HWND); */
/* void ExposeScrollBars(HWND); */
void checkcontrols(HWND, int);
/* void resetmapping(void); */
void resetmapping(HWND, int);
void SetupWindowText(HWND);
int paintpage(HWND, HDC);
unsigned char decryptbyte (unsigned char, unsigned int *);
void newmapandfonts(HWND);
void wininfo(char *);
int wincancelinfo(char *);
/* */
void TurnTimerOn (HWND);
void AddExt(PSTR, PSTR);
int DoOpenFile(HWND);
int DoPreScan(HWND, int);
/* void usepagetable(int, int); */
int usepagetable(int, int);
/* void ChangeDirectory(char *); */
int ChangeDirectory(char *, int);
int NewChangeDirectory(LPSTR, int);
void removeback(char *);
void trailingback(char *);
void resetfonts(void);
void closeup(HWND, int);
/* BOOL IsTaskActive(HWND); */
BOOL IsTaskActive(void);
int SwitchDVIFile(HWND, int, int);
void WriteTime (char *, char *);		/* 95/Aug/15 */
void ParseFileName(void);				/* 95/Sep/17 */
void checkpaper(HWND, int);				/* 95/Dec/15 */
void checkunits(HWND, int);
void checkduplex(HWND, int);			/* 96/Nov/17 */
void checkpreferences(HWND);			/* 95/Dec/15 */
void ShowSystemInfoAux(char *);			/* show system info 95/Mar/18 */
void ShowSysFlagsAux(char *);			/* show system info 95/Mar/18 */
// int ConsoleInput(char *, char *);		// 99/Oct/20
int ConsoleInput(char *, char *, char *);		// 99/Oct/20

/* functions in WINPSLOG */
/* long scanlogfile(int); */
long scanlogfile(int, HDC);
void errcount(void);
void initialfonts(void);
void clearfonts(void);
void clearmetrics(void);
void makeuppercase(char *);
void makelowercase(char *);
/* void standardizenames(void); */
/* int readatmini(void); */
/* int standardsub(char *, char *); */
/* int mapfontnames(void); */
int mapfontnames(HDC);
void ShowFontNames(void);
BOOL istexfont(char *);
char *alias(char *);
char *nextpathname(char *, char *);
int searchalongpath (char *, char *, char *, int);
void FreeFontNames(void);
void FreeWidths(void);
void free_fontmap(void);				/* 1994/Nov/22 */
char *zstrdup(char *);					/* 1995/July/15 */
int WriteRegFile(char *);				/* 1995/Aug/15 */
int ReadATMReg(int);					/* 1997/Jan/21 */
#ifdef LONGNAMES
HFILE findandopen(char *, char *, char *, char *, int);
#else
FILE *findandopen(char *, char *, char *, char *, int);
#endif
void RestoreColorStack(int);
void SaveColorStack(int, int);
int FindApplication(char *, char *, int);
void checkfontname(int);
int IsRelative(char *);
int decodepapersize (char *, int *, int *);

/* int CALLBACK ConsoleText (HWND, UINT, WPARAM, LPARAM); */	/* 99/June/25 */
HWND CreateConsole(HINSTANCE, HWND);	/* 99/June/25 */

/* functions in WINANAL */
int scandvifile(int, HDC, int);
int scandvipage(int, HDC, int);
HFONT createatmfont(char *, int, int, int, int, int);
void setscale(int);			/* move to dviwindo.c ? */
/* void setupremaptable(void); */
int mapx(long);
int mapy(long);
int mapd(long);
long lmapd(long);
long unmap(int);
long lunmap(long);
long leftpageoffset(long);
long rightpageoffset(long);
RECT DPtoLPRect(HDC, RECT); 
int redomapping(int, RECT, RECT); 
/* void redomapping(int, RECT, RECT); */
int setnewzoom(HDC, RECT, RECT);
DWORD ChangeColor(HDC, int);
/* BOOL istexfont(char *); */
void colorandreverse(void);
int buildpalettemap(HDC); 
/* void touchallfontsub(HDC); */
void touchallfonts(HWND);
BOOL InterSectRect(const RECT FAR *, const RECT FAR *);
/* BOOL isansi (TEXTMETRIC); */
BOOL isansi (TEXTMETRIC, char *, int);
unsigned long codefourty(char *);
void DrawRule(long, long);
void showmagnification(HWND);

/* int GetWidth(HDC, int, int, LPINT); */
/* typedef short int far   *LPSHORT; */
int GetWidth(HDC, int, int, short int far *);
/* BOOL isremapped(LPINT, TEXTMETRIC, int); */
/* typedef short int far   *LPSHORT; */
BOOL isremapped(short int far *, TEXTMETRIC, int);

int gettoken(int, char *, int); 
int getalphatoken(int, char *, int);
// void readspecial(int, char *, int);
void readspecial(int, char *, unsigned int);
void flushspecial(int);

int an_wingetc(int); 
void an_unwingetc(int, int); 
long an_winseek(int, long); 
long an_wintell(int); 
int an_wininit(int); 

int ExplainError(char *, int, int);

int CallEditor(int, char *, char *, char *);
int DDEServerStart (void);
int DDEServerStop (void);

void cancelmetrics(void);

void CheckHeap(char *, int);
char *convertnewlines(char *);

/* functions in WINFONTS */
/* BOOL CALLBACK EncodingDlg(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export EncodingDlg(HWND, UINT, WPARAM, LPARAM);
/* BOOL FAR PASCAL FontDlg(HWND, unsigned, WORD, LONG); */
/* BOOL CALLBACK FontDlg(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export FontDlg(HWND, UINT, WPARAM, LPARAM);
/* int CALLBACK EnumFunc(const LOGFONT FAR *, const TEXTMETRIC FAR *, int,
LPARAM); */
/* int CALLBACK _export EnumFunc(const LOGFONT FAR *, const TEXTMETRIC FAR *, int, LPARAM); */
/* int CALLBACK _export EnumFunc(const ENUMLOGFONT FAR *, const NEWTEXTMETRIC FAR *, int, LPARAM); */
/* long FAR PASCAL SpinWndFn (HWND, unsigned, WORD, LONG); */
/* long CALLBACK SpinWndFn (HWND, UINT, WPARAM, LPARAM); */
LRESULT CALLBACK _export SpinWndFn (HWND, UINT, WPARAM, LPARAM);
/* BOOL FAR PASCAL FontsUsed(HWND, unsigned, WORD, LONG); */
/* BOOL CALLBACK FontsUsed(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export FontsUsed(HWND, UINT, WPARAM, LPARAM);
/* CALLBACKS above */
void ShowFontInfo(char *, int);
void ShowFontSample (HWND, HDC, int);
void ShowCharWidths(HWND, HDC);
/* void WriteAFMFile(HWND, HDC); */
/* int WriteAFMFile(HWND, HDC); */
/* int WriteAFMFile(HWND, HDC, int);  */
int WriteAFMFile(HWND, HDC, int, int);
int WriteAFMFileSafe(HWND, HDC, int, int); 
void showscaling(HWND);
void showmetrics(HWND);
void showfilespecs(HWND);
int GetFontSelection(HWND, int, int);
void FreeFaceNames(void);
void FreeFullNames(void);				/* 1995/July/7 */
void FreeFileNames(void);				/* 1995/July/7 */
void SetFontState(int);
void checkfontmenu(void);
/* int TagFont(HWND, int, int); */
int TagFont(HWND, int, int, int);
HGLOBAL getencoding(char *);
/* int writealltfm(HWND); */
int WriteAllTFM(HWND, int);
int FullFromFace(HDC, char *, int, int);
/* int FaceFromFull(HDC, char *); */
int SetupTTMapping(HDC);
LPSTR GrabFileNames(void);
void ReleaseFileNames(void);
unsigned long mappoints(unsigned long);		/* 97/Sep/16 */
int ShowWhatChar(HWND, int, int);
char *ConstructFontKey (char *s);

/* functions in WINPRINT */
BOOL FAR  PASCAL PrSetupDialog  (HWND, UINT, WPARAM, LPARAM);
/* HDC PASCAL GetDefPrinterDC (void); */
/* int FAR PASCAL AbortDlg(HWND, unsigned, WORD, LONG); */
/* int CALLBACK AbortDlg(HWND, UINT, WPARAM, LPARAM); */
/* int CALLBACK _export PrintDlg(HWND, UINT, WPARAM, LPARAM); */
/* int CALLBACK _export DVIPrintDlg(HWND, UINT, WPARAM, LPARAM); */
int CALLBACK _export PrintDlgProc(HWND, UINT, WPARAM, LPARAM);
/* int FAR PASCAL AbortProc(HDC, int); */
BOOL CALLBACK _export AbortProc(HDC, int);  /* 92/Nov/3 */
/* BOOL FAR PASCAL PageDlg(HWND, unsigned, WORD, LONG); */
/* BOOL CALLBACK PageDlg(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export PageDlg(HWND, UINT, WPARAM, LPARAM);
/* BOOL FAR PASCAL RangeDlg(HWND, unsigned, WORD, LONG); */
/* BOOL CALLBACK RangeDlg(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export RangeDlg(HWND, UINT, WPARAM, LPARAM);
/* int FAR PASCAL FileDlg(HWND, unsigned, WORD, LONG); */
/* int CALLBACK FileDlg(HWND, UINT, WPARAM, LPARAM); */
int CALLBACK _export FileDlg(HWND, UINT, WPARAM, LPARAM);
int CALLBACK _export TeXDlg(HWND, UINT, WPARAM, LPARAM);
/* BOOL FAR PASCAL SearchDlg(HWND, unsigned, WORD, LONG); */
/* BOOL CALLBACK SearchDlg(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export SearchDlg(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK _export EditItemDlg(HWND, UINT, WPARAM, LPARAM);
/* BOOL FAR PASCAL DVIMetric(HWND, unsigned, WORD, LONG); */
/* BOOL CALLBACK DVIMetric(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export DVIMetric(HWND, UINT, WPARAM, LPARAM);
/* BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG); */
/* BOOL CALLBACK About(HWND, UINT, WPARAM, LPARAM); */
BOOL CALLBACK _export About(HWND, UINT, WPARAM, LPARAM);
/* CALLBACKS above */
void ShowAppAndSys(HWND);
// int DoPrinting(HWND);
int DoPrintStuff(HWND, int);
int DoPrintAll(HWND);
int DoCopy(HWND, RECT);
int DoPrintSetup(HWND);
/* int searchdvifile(int, long, char *); */
int ReOpenFile(HWND);
int searchandmark(HWND, BOOL);
long pagecurrent(int);
/* char *addpagestring(char *); */
void addpagestring(char *);
/* long leftpageoffset(long); */
/* long rightpageoffset(long); */
/* void flushDVIMetric(HWND); */
/* void flushFontsUsed(HWND); */
void ShowDVIMetric(HWND);
void ShowFontsUsed(HWND);
/* void doserror(int, char *, char *); */
void doserror(int, char *, int, char *);
void showprintspecs(HWND);
/* int isitpscrpt(HDC); */
int IsItPSCRPT(HDC);
/* void passthrough(HDC, char *); */
// void passthrough(HDC, char *, int);
// int passthrough(HDC, const char *, int);
// int passthrough(HDC, char *, int);
int passthrough(HDC, char *, unsigned int);
/* void GetDVIPSONEWindow (HTASK); */
/* HWND GetTaskWindow (HTASK); */
HWND GetTaskWindow (HINSTANCE);
/* int TryWinExec (char *, int, char *); */
HINSTANCE TryWinExec (char *, int, char *);
/* void MaybeShowStr (char *, char *); */
int MaybeShowStr (char *, char *);
char *quotestring(char *);				/* 98/Jan/10 */
void WritePrivateProfileInt (char *, char *, int, char *);
int RunDVIPSONEDLL(HWND, char *, int);

/* functions in WINSEARC.C */
int checkmarkhit(HWND, int, int);
int callapplication(HWND, int, int, int);
void filltexmenu(HWND);
void cleartexmenu(HWND);
int checktexmenu(HWND);
int translatehotkey(HWND, int, int);
void fillenvmenu(HWND);				/* 98/Aug/28 */
void clearenvmenu(HWND);
int checkenvmenu(HWND);
void CommSearch(HINSTANCE, HWND);
char *grabenv (char *);
int LoadATM(void);
void UnloadATM(void);
/* int ReencodeFont(HDC, int); */
int ReencodeFont(HDC, int, int);
/* void UnencodeFont(HDC, int); */
int UnencodeFont(HDC, int, int);
/* HGLOBAL SetupEncodingSub (LPSTR); */
// HGLOBAL CompressEncodingSub (LPSTR);
HGLOBAL CompressEncodingSub(LPSTR *);

void free_hyperfile(void);				/* 1995/Aug/12 */
void savehyperstate(void);				/* 1995/Aug/12 */
void restorehyperstate(void);
void WriteError(char *);				/* 1995/Aug/15 */
int RemoveDocument(char *, char *);		/* 1996/June/5 */
int AddDocument(char *, char *);		/* 1995/Sep/17 */
void filldocmenu(HWND);					/* 1995/Sep/17 */
void cleardocmenu(HWND);				/* 1995/Sep/17 */
int checkdocmenu(HWND);					/* 1995/Sep/17 */
int calldocument(int, int, int);		/* 1995/Sep/17 */
int editenvironment(int, int, int);		/* 1998/Aug/26 */
void fillhelpmenu(HWND);				/* 1995/Sep/18 */
void CallHelp(HWND);					/* 1995/Sep/18 */
int adjustsize(int, int);				/* 1995/Oct/1 */
int expanddots(char *);					/* 1996/Jan/30 */
int comparenames(char *, char *);		/* 1996/Jan/30 */
int ReadEncoding(int);					/* 1996/Jan/17 */
int CheckEncodingMenu(HWND, WORD);		/* 1997/Apr/4 */
int SetupATMSelectEncoding(int);		/* 1997/Apr/5 */
int editcommandflags(HWND, WORD);		/* 1996/Mar/9 */
int PutStringOnClipboard(HWND, LPSTR);	/* 1997/July/12 */
#ifdef LONGNAMES
HFILE LongOpenFile(char *, int);		/* 1995/Dec/7 */
#endif
int sizeofUID(void);					/* 1998/Feb/28 */
LPSTR WINAPI lstrncpy(LPSTR, LPCSTR, int);

/* functions in WINSPECI.C */
/* int CALLBACK CopyLineFun(LPSTR, int, LONG); */
int CALLBACK _export CopyLineFun(unsigned char *, int, LONG);
/* CALLBACKS above */
void dospecial(HDC, int, unsigned long);
/* char *strippath(char *); */
char *removepath(char *);
/* char *nextpathname(char *, char *); */
void extension(char *, char *);		/* in winspeci.c */
void forceexten(char *, char *);	/* in winspeci.c */
void CheckColorStack(HDC);			/* in winspeci.c */
int checkCTM(HDC);					/* in winspeci.c */
void resetCTM(HDC);					/* in winspeci.c */
void copynearimage(unsigned char huge *, unsigned char *, unsigned long);
#ifdef LONGNAMES
HFILE findepsfile(char *, int, char *);
#else
FILE *findepsfile(char *, int, char *);
#endif
void SetupRuleColor (HDC, int, int, int);
void doColorPush(HDC);
void doColorPop(HDC);
void DoColor (HDC, int, int);
void DoBackGround (HDC, int, int);
double decodeunits(char *);
void doClipBoxPopAll(HDC);
int RemoveExtraSamples (unsigned char *, int);	/* 99/May/10 */

/* void ShowMetaFileHeader (void *, char *); */	/* 95/Nov/5 */

void ShowLastError(void);			/* in winsearc.c */

/* following only if LONGNAMES defined ... */

#ifdef LONGNAMES
char *lfgets(char *, int, HFILE);
int lfputs(const char *, HFILE);
HFILE lfopen(const char *, const char *);
int lfclose(HFILE);
long lftell(HFILE);
int lfseek(HFILE, long, int);
void lfrewind(HFILE);
int lfgetc(HFILE);
int lfungetc(int, HFILE);
#endif

#ifdef LONGNAMES
#define fgets lfgets
#define fputs lfputs
#define fopen lfopen
#define fclose lfclose
#define fseek lfseek
#define ftell lftell
#define rewind lfrewind
#undef getc
#define getc lfgetc
#undef ungetc
#define ungetc lfungetc
#endif

#ifdef LONGNAMES
#define BAD_FILE HFILE_ERROR
#else
#define BAD_FILE NULL
#endif

/////////////////////////////////////////////////////////////////


typedef short int far   *LPSHORT;

#ifdef USEUNICODE
extern BOOL bFontEncoded;			/* non-zero if current font is reencoded */
#endif

#ifdef LONGNAMES
extern HFILE special;
#else
extern FILE *special;
#endif

#ifdef MAKEUNICODE
typedef struct {
	char *glyphname;
	unsigned short UID;
} UNICODE_MAP;

extern const UNICODE_MAP unicodeMap[];
#endif

typedef struct _EXECAPPINFO {
	HINSTANCE hInstance;            /* The instance value */
	HWND hWnd;                      /* The main window handle */
	HTASK hTask;                    /* The task handle */
} EXECAPPINFO, FAR *LPEXECAPPINFO;

EXECAPPINFO TaskInfo;				/* place for Task info */


#ifdef AllowCTM
extern int CTMstackindex;				/* CTM stack index 96/Nov/3 */
extern BOOL bUseAdvancedGraphics;		/* on to allow use of GM_ADVANCED */
extern BOOL bAdvancedGraphicsMode;		/* in if currently in GM_ADVANCED */
extern BOOL bUseCTMflag;				/* new 1996/Nov/5 */
#endif

#ifdef USEUNICODE
extern BOOL bUseNewEncodeTT;	/* Use new method for encoding 97/Jan/16 */
extern BOOL bUseNewEncodeT1;	/* Use new method for encoding 97/Jan/16 */
extern BOOL bOldUnicode;		/* Use old UNICODE assignments */
#endif

/* NOTE: ATMSelectObject does not work in Windows NT */
#define MyATMSelectObject ATMSelectObject
#define MyATMFontSelected ATMFontSelected

/* extern int (WINAPI *MyATMSelectObject) (HDC, HFONT, WORD, HFONT FAR *); */
/* extern BOOL (WINAPI *MyATMFontSelected) (HDC); */

#ifdef ATMSOLIDTABLE
extern char encodefont[MAXFONTS];		/* non-zero if encoding already set up */
#else
extern HGLOBAL hATMTables[MAXFONTS];	/* non-NULL if encoding already set up */
#endif

#ifdef ALLOWSCALE
extern int outscaleflag;
extern double outscale;
#endif

///////////////////////////////////////////////////////////////////

extern LPSHORT lpWidths;		/* FAR pointer to char widths table */
extern LPSHORT lpCharWidths;	/* FAR pointer to width table for font */
extern LPSHORT lpCharTemp;		/* FAR pointer to width table for font */

extern char str[BUFLEN];			/* temporary for random strings */
extern char debugstr[DEBUGLEN];		/* temporary for OutputDebugString */
extern char buffer[BUFFERLEN];		/* 512 buffer for low level input routines */
extern char buttonlabel[MAXMARKS+1];		/* label of last button hit */

extern unsigned char charbuf[MAXCHARBUF+1]; /* buff accumulated characters */

extern long counter[TEXCOUNTERS];  /*  \count0 ... \count9 in TeX */

// extern WORD finx[MAXFONTNUMBERS];		/* indeces into next few */
extern short finx[MAXFONTNUMBERS];		/* indeces into next few */

extern char *fontname[MAXFONTS];		/* TeX TFM font names */
extern char *subfontname[MAXFONTS];		/* font names substituted for */
extern int fontdescent[MAXFONTS];		/* drop below baseline */
extern int fontascent[MAXFONTS];		/* rise above baseline */
extern unsigned long fs[MAXFONTS];		/* at size */ 
extern unsigned long fc[MAXFONTS];		/* checksum  TFM file (useless ?) */
extern HFONT windowfont[MAXFONTS];		/* handles to scaled fonts */
extern char fontbold[MAXFONTS];			/* info from ATM.INI or WIN.INI */
extern char fontitalic[MAXFONTS];		/* info from ATM.INI or WIN.INI */
extern char fontttf[MAXFONTS];			/* info from ATM.INI or WIN.INI */
extern char texfont[MAXFONTS];		/* non-zero if font is TeX font */
extern char ansifont[MAXFONTS];		/* non-zero for text font */
extern char fontvalid[MAXFONTS];	/* non-zero if handle valid */
extern char metricsvalid[MAXFONTS];	/* non-zero if metrics valid */
extern char fontexists[MAXFONTS];	/* non-zero if font found by ATM */
extern char fontwarned[MAXFONTS];	/* non-zero => absence noted */

extern char *unitnames[];	/* names of measurement units - winspeci.c */

extern double unitscale[];			/* corresponding scale factors */

extern int pcptflag;			/* winspeci.c decompose picas and points */

extern int nMagicFact;			/* adjust nHeight of fonts 97/Sep/14 */

extern int buttondvipage;		/* page of button last pressed 95/Mar/10 */
extern int oldbuttondvipage;		/* page of button last pressed 95/Mar/10 */

extern long lastsearch;				/* position in file last search ended */

extern int fnext;			/* next font slot to use (total number at end) */

extern int taggedfont;				/* font of rectangle enclosing */
extern int taggedchar;				/* the character hit (if only one) */
extern int taggedwidth;				/* the width of the tagged char*/
extern long ltaggedwidth;			/* the width of the tagged char*/

extern long buttonpos;			/* position of button last pressed */
extern long oldbuttonpos;			/* position of button last pressed */

extern long charh, chara, chard;	/* FontSample height ascent descent */

extern PRINTDLG pd;				/* place for COMMDLG structure 95/Dec/14 */

extern HINSTANCE hInst;					/* Instance handle */
extern HINSTANCE hTIFFLibrary;		/* handle of TIFF library if loaded */

extern HWND hwnd;
extern HWND hFindBox;		/* handle to Find Dialog box */
extern HWND hConsoleWnd;	/* Console Text Window Handle */
extern HWND hDlgPrint;				/* handle to abort spooling box */ 
extern HWND hInfoBox;				/* handle to DVIMetric dialog box */
extern HWND hFntBox;				/* handle to FontsUsed dialog box */

extern HDC hPrintDC;				/* printer DC if any */
extern HDC hdc;						/* device-context variable */

extern HCURSOR hArrow;					/* handle to normal cursor */
extern HCURSOR hSaveCursor;				/* current cursor handle	*/
extern HCURSOR hHourGlass;				/* handle to hourglass cursor	*/

extern HMENU hTeXMenu;					/* made global 1994/Nov/20 */
extern HMENU hEnvMenu;					/* made global 1994/Nov/20 */
extern HMENU hHelpMenu;					/* made global 1995/Sep/17 */

extern HFONT hFontOld;		/* font when we came in */
extern HFONT hConsoleFont;			// font used in Console Window

extern HBRUSH hGreyBrush;
extern HBRUSH hLightGreyBrush;
extern HBRUSH hBlackBrush;
extern HBRUSH hRuleBrush, hRuleBrushDefault;
extern HBRUSH hbrBackground;		/* in dviwindo.c */
extern HBRUSH hbrDefaultBackground;	/* in dviwindo.c */

extern HPEN hGreyPen;
extern HPEN hLightGreyPen;
extern HPEN hBlackPen;
extern HPEN hRulePen, hRulePenDefault;
extern HPEN hBorderPen;
extern HPEN hLinerPen;
extern HPEN hFigurePen;			/* for drawing ViewPort Box 94/Oct/5 */

extern HGLOBAL hWidths;			/* handle to memory for char widths */
extern HGLOBAL hEncoding;	/* Global Encoding if ENCODING defined 94/Dec/25 */
extern HGLOBAL hATMTable;	/* Global ATM Data for Reencoding 94/Dec/25 */
extern HGLOBAL hATMShow;	/* Global ATM Data for Show Fonts 94/Dec/25 */
extern HGLOBAL hPages;		/* handle to memory for page table */
extern HGLOBAL hTPIC;			/* handle to memory for TPIC path */
extern HGLOBAL hMarks;			/* handle to memory for hyptertext marks */
extern HGLOBAL hFaceNames;		/* handle to memory for font names */
extern HGLOBAL hFullNames;		/* handle to memory for full names (TT) */
extern HGLOBAL hFileNames;		/* handle to memory for TFM file names */

extern RECT CopyClipRect;			/* Clipping Rectangle when copying */

extern BOOL bWin95;				/* non-zero if Windows 95 */
extern BOOL bWinNT;				/* non-zero if Windows NT */
extern BOOL bNewShell;			/* non-zero if new shell (Windows 95 user inter) */
extern BOOL bWinNT5;			/* non-zero if Windows NT 5 */
extern BOOL bATM4;				/* non-zero => ATM 4.0 */

extern TEXTMETRIC TextMetric;		/* used by font sample module */

extern BOOL bLandScape;				/* paper orientation */
extern BOOL bShowCalls;				/* non-zero => show WinExec calls */


extern BOOL bGreyText;				/* `greek' text */
extern BOOL bGreyPlus;				/* `greek' text plus text on top */
extern BOOL bComplainMissing;		/* complain about missing fonts */
extern BOOL bIgnoreFontCase;		/* ignore case of Windows Face name */

extern BOOL bTagFlag;				/* figure out font at given point */
extern BOOL bMarkFlag;				/* figure out mark at given point */

extern BOOL bSmallStep;				/* non-zero for small zoom steps */

extern BOOL bRuleFillFlag;			/* fill rule rectangles */
extern BOOL bIgnoreSelect;			/* use for debugging input */
extern BOOL bColorFont;				/* non-zero means color fonts */
extern BOOL bPreserveSpacing;		/* non-zero means preserve spacing */
extern int  bUseRect;				/* use FillRect & FrameRect */

extern BOOL bTrueTypeOnly;			/* if want only TrueType */
extern BOOL bTypeOneOnly;			/* if want only Adobe Type 1 */

extern BOOL bTextOutFlag;			/* show text using TextOut */
extern BOOL bSnapToFlag;			/* non-zero means do snapto rules */
extern BOOL bDrawVisible;			/* non-zero means refresh only visible */

extern BOOL bUseSourcePath;			/* use directory of file TeX worked on */

extern BOOL bUseCharSpacing;		/* use accurate character spacing */
									/* `Favour Position' */ 
extern BOOL bClipRules;				/* clip rules as well as 95/Aug/26 */
extern BOOL bFixZeroWidth;			/* fix zero widths RectVisible problem */

extern BOOL bShowFlag;		/* non-zero if showing text on screen */
extern BOOL bShowFileName;	/* show name of file for figure */
extern BOOL bShowPreview;	/* non-zero => Show TIFF and EPSI previews */

extern BOOL bUserAbort;		/* if PeekMessage noted user wanted to interfere */
extern BOOL bWrapFlag;		/* permit wrap around at end of file */
extern BOOL bPrintFlag;				/* non-zero when printing */
extern BOOL bCopyFlag;				/* non-zero when copying to ClipBoard */
extern BOOL bScanFlag;				/* non-zero when in scandvi... */

extern BOOL bDebug;			/* non-zero => debugging mode request */
extern BOOL bDebugKernel;	/* non-zero => running in Windows debug kernel */
extern BOOL bDebugPause;	/* DEBUGPAUSE env var set */
extern BOOL bDebugMenu;		/* DEBUGMENU env var set */

extern int MinWidth;			/* safe minimum visible rect widths 95/Sep/3 */

extern BOOL bIgnoreBadANSI;			/* do not check ANSI encoding violations */
extern BOOL bNeedANSIacce;			/* set if char < 32 in ANSI font */
extern BOOL bNeedANSIwarn;			/* set if already warned about this */
extern int  nNeedANSIfont;			/* what font last violated encoding */
extern int  nNeedANSIchar;			/* what char violated ANSI encoding */

extern BOOL bHyperUsed;				/* DVI page contains hypertext linkage */
extern BOOL bHyperLast;			/* last action was hypertext jump */
extern BOOL bSourceUsed;		/* DVI page contains src linkage */

extern BOOL bATMPrefer;				/* try and Force Select ATM font */
extern BOOL bATMExactWidth;			/* try and Force ExactWidth for ATM font */
extern BOOL bATMLoaded;				/* if loaded properly and links setup */
extern BOOL bATMReencoded;			/* non-zero when ATM in reencoded state */

extern BOOL bUseBaseFont;			/* share metric info same face and style */

extern BOOL bCheckEncoding;			/* check checksum encoding info 95/Jan/12 */

extern BOOL bCarryColor;			/* carry color across page boundary */
extern BOOL bAllowClip;				/* allow clipping push and pop */
extern BOOL bPassThrough;	/* allow EPS file pass through if PS printer */

extern int bColorUsed;				/* This DVI file has \special{color ...} */
extern int colorindex;				/* points at next slot to push into */

extern int nDCstackindex;			/* DC stack index 98/Sep/10 */

extern BOOL bUseGetExtent;			/* GetExtent for char widths 95/Nov/5 */

extern BOOL bUseATMFontInfo;		/* use ATMGetFontInfo */

extern char *szReencodingName;		/* name of ENCODING minus path */

extern unsigned long nCheckSum;		/* expected TFM checksum */
extern unsigned long nCheckANSI;	/* expected TFM checksum ANSI NEW */

extern int markfont;				/* selected font to highlight */
extern int tagx, tagy;				/* place tagged with mouse */

extern int xposition, yposition;	/* where to place text on screen */

extern int nPaletteOffset;		/* offset to add to palette index */
extern int nMaxBright;			/* maximum grey tone accepted for colors */
extern int bColorStable;		/* use f instead of finx[f] in color font */

extern char DefPath[MAXDIRLEN];	/* default path from DVIPATH */
extern char FileName[MAXFILENAME];		/* used by _lopen */

extern WORD remaptable[128];					/* cut down 1995/June/23 */

extern int textures;		/* non-zero => textures style DVI file */

extern int dvipage;			/* desired bop count from beginning */

extern int finish;			/* non-zero => have hit end of DVI file */

extern int penwidth;		/* pen width from TPIC `pn' command */

extern BOOL bSpreadFlag;	/* non-zero implies show two pages */

extern BOOL bPatchBadTTF;	/* deal with `text' TTF fonts that aren't */

extern int magbase;			/* user selected base magnification */

extern int metricsize;		/* size used for getting metrics of font */

extern int leftdvipage;		/* use if bSpreadFlag != 0 && bCountZero != 0 */
extern int rightdvipage;	/* use if bSpreadFlag != 0 && bCountZero != 0 */

extern long leftcurrent;
extern long rightcurrent;

extern int bUserAbort;		/* if PeekMessage noted user wanted to interfere */

extern BOOL bMarkSearch;			/* 1 for inverse search */

extern int StringLimit;		/* MUST be LESS than or equal to MAXCHARBUF */

extern BOOL bMaxDriftFlag;	/* non-zero implement max drift rule */

extern BOOL bAvoidZeroWidth;	/* non-zero to avoid zero width rules */
							/* default 1 - makes min size 1 device pixel */
							/* 2 => min size is *2* device pixels */

extern BOOL bOffsetRule;	/* offset rule by 1/2 device pixel */
							/* not a good idea 98/Jan/12 */


extern BOOL bATMBugFlag;	/* non-zero => need to correct for ATM bug */
							/* remap so old ATM can handle double encoding */
extern int bTTRemap;		/* remap TrueType font control character range */

extern BOOL bEnableTermination;			/* permit early termination */

extern BOOL bANSITeXFlag;	/* remap low part of ANSI encoded fonts */

extern BOOL bUseFakeFont; 		/* use fake font (Times) for missing fonts */

extern BOOL bCheckReencode;		/* reencode font being shown 97/Jan/16 */

extern int fakeindex;		/* index into following table of fake font names */

extern int fakettf;			/* is fake font TrueType or Type 1 1995/August/2 */

extern char *FakeFonts[];		/* plausible fake font names */

extern char *szApplication; /* = "DVIWindo"; in dviwindo.c */
extern char *szTopic;		/* = "SRCSpecial" in dviwindo.c */

extern int srclineno;					/* reset to zero */
extern char srcfile[FILENAME_MAX];		/* reset to "" */
extern int srctaghit;					/* have we hit src \special yet */

extern char *achFile;				/* Profile file name - dviwindo.ini */
extern char *achPr;					/* Profile file section name */

extern unsigned long oneinch;		/* normally ONEINCH */
extern unsigned long dvi_num;			/* numerator of scale */
extern unsigned long dvi_den;			/* denominator of scale */
extern unsigned long dvi_mag;			/* magnification */

extern int dvi_f;	/* current font */

extern long current;	/* pointer to current bop in this file */

extern int MaxFonts;	/* remember how much we allocated for Widths */

extern char *bufptr;				/* pointer to next byte to take out */

extern int buflen;					/* bytes still left in buffer */

extern int ungotten;				/* result of unwingetc - or -1 */

extern long filepos;			/* position of byte in input file */

extern long xoffset;			/* x offset in mapping to screen */
extern long yoffset;			/* y offset in mapping to screen */

extern DWORD idServerInst;

extern long ImageWidth, ImageLength;					/* vital */

extern long BitsPerSample;				/* vital */

extern long	SamplesPerPixel;			/* vital (may be more than 1) */
extern long	SamplesPerPixelX;			/* vital (may be more than 1) */

extern long ExtraSamples;				/* new TIFF 6.0 1999/May/10 */

extern long compression;				/* vital */

extern long Orientation;

extern long StripOffset;				/* vital (first strip offset) */

extern long StripByteCount;				/* first StripByteCount */

extern long RowsPerStrip;

extern long PhotometricInterpretation;

extern long BitsPerPixel;		/* BitsPerSample * SamplePerPixel */
extern long BitsPerPixelX;		/* BitsPerSample * (SamplePerPixel-ExtraSamples) */

extern long StripsPerImage; 	/* computed from above */

extern long InRowLength; 		/* number of source bytes from file */
extern long InRowLengthX; 		/* ditto after removing ExtraSamples */
extern long OutRowLength;		/* number of processed bytes for output */

extern long Predictor;			/* for LZW only, 1 => none 2 => difference */

extern BOOL bCompressColor; /* compress color to one byte */ /* 96/Sep/7 */

extern int ResolutionUnit;		/* 1 no dimensions, 2 per inch, 3 per cm */

extern unsigned long xresnum, xresden;		/* x resolution */
extern unsigned long yresnum, yresden;		/* y resolution */

extern int hptagflag;					/* non-zero if call from HPTAG */

extern long psoffset, pslength;			/* info from EPSF header */
extern long metaoffset, metalength;
extern long tiffoffset, tifflength;

extern long StripOffsetsPtr;
extern int StripOffsetsType;
extern int StripOffsetsLength;

extern long StripByteCountsPtr;
extern int StripByteCountsType;
extern int StripByteCountsLength;

extern long PlanarConfiguration;		/* NOTE: cannot handle 2 ... */

extern BOOL bGhostHackFlag;			/* for now */

extern int typesize[6]; /* in winspeci.c */

extern char OpenName[MAXDIRLEN];		/* copied from IDC_EDIT box */
// extern char IniDefPath[MAXDIRLEN];		/* working directory */
extern char *IniDefPath;				/* working directory */
extern char DefSpec[MAXDIRLEN];			/* default file specification */
extern char DefExt[MAXDIRLEN];			/* default extension */
extern char *szTeXDVI;					/* Y&YTeX DVI output redirection */
extern char *szExeWindo;				/* file path of executable */
extern char SourceOpenName[MAXDIRLEN];	/* for calling app - space ? */
extern char SourceDefPath[MAXDIRLEN];	/* default path for application */
extern char SourceDefSpec[MAXDIRLEN];	/* default file specification */
extern char SourceDefExt[MAXDIRLEN];	/* default extension */

extern char szHyperFile[MAXFILENAME];	/* expensive solution */
extern char TestFont[MAXFACENAME];		/* font name used in tests */

extern char *szYandYTeX;				/* passed to Y&YTeX 94/July/10 */
extern char *szCustom;					/* custom encoding name 97/Dec/22 */
extern char *szCustomPageSize;			/* custom page size string 2000 May 27 achEnv */
extern int hFile;						/* DOS file handle		*/
extern UINT bTimerOn;
extern BOOL bKeepFileOpen;				/* keep file open - untested */
extern BOOL bUseTimer;
extern BOOL bCountZero;		/* non-zero => page numbers are TeX counter[0] */
extern BOOL bApplications;	/* [Applications] section found in DVIWINDO.INI */
extern BOOL bEnvironment;	/* [Environment] section found in DVIWINDO.INI */
extern BOOL bUnixify;		/* convert \ to / when expanding # in TeX Menu */
extern BOOL bHiresScreen;	/* if better than VGA - expand open dialog box */
extern BOOL bStripPath;		/* strip path from ? menu call 96/Oct/4 */
extern BOOL bShowTIFF;		/* allow showing of TIFF insertimage: */
extern BOOL bShowWMF;		/* allow showing of Windows MetaFiles */
extern int nCmdShowForce;	/* negative, or CmdShow for WinExec */
extern int OfShareCode;		/* OF_SHARE_DENY_WRITE default for Open File */
extern int OfExistCode;		/* OF_SHARE_DENY_NONE default for File Exist */

extern BOOL bRememberDocs;	/* remember opened documents */
extern BOOL bKeepSorted;	/* keep opened documents sorted 96/Jun/5 */
extern BOOL bSortEnv;		/* keep environment sorted 98/Sep/1 */
extern BOOL bHelpAtEnd;		/* Help menu entry at end */

extern BOOL bAllowCommDlg;	/* non-zero if COMMDLG.DLL functions allowed */
extern BOOL bUsingCommDlg;	/* non-zero if COMMDLG.DLL functions available */
extern BOOL bLongNames;		/* show long names in Open File dialog 95/Dec/1 */
extern BOOL bIgnoreLigatures;	/* in search, step over ligatures */
extern BOOL bBusyFlag;			/* non-zero while repainting */
extern BOOL bShowSearchFlag;	/* show modeless Search Dialog Box */
extern BOOL bShowSearchExposed;	/* (hidden if not connected to file) */
extern BOOL bUseNewEncodeTT;	/* Use new method for encoding 97/Jan/16 */

extern int nSourceLine;			/* line number to search for */

extern char *achAp;			/* Profile section name [Applications] */
extern char *achEnv;		/* Profile section name [Environment] */
extern char *achDocs;		/* Profile section name [Documents] */
extern char *achDiag;		/* Profile section name [Diagnostics] */
extern char *achEsc;		/* Profile section name [Escapes] */

extern char *szReencodingVector;		/* value of ENCODING env var */

extern BOOL bTeXANSI;		/* value of TEXANSI env var - default now 0 */
extern BOOL bUseDVIWindo;	/* use [Environment] section of `dviwindo.ini' */

extern WORD basefont[MAXFONTS];		/* font of same name and style */
extern long dvi_h, dvi_v;
extern int dvi_hh, dvi_vv;	

extern long dvi_spot_h, dvi_spot_v;	/* x marks the spot in search */
extern long xoffsetold;			/* saved base offset */
extern long dvistart;		/* where DVI part of (TeXtures) file starts */
extern BOOL bHourFlag;			/* non-zero while hourglass displayed */
extern BOOL bCaseSensitive;		/* is search case sensitive ? */
extern BOOL bMarkSpotFlag;			/* indicated we wish to mark the spot */
extern BOOL bCurrentFlag;		/* search in current directory first */
extern BOOL bFileValid;		/* non-zero if connected to file */
extern int nApplications;			/* number of [Applications] */
extern int nEnvironment;			/* number of [Environment] */

extern char *szSource;		/* source file name to search for */

extern unsigned long dvi_l;			/* max page height + depth */
extern unsigned long dvi_bytes;		/* size of file in bytes */
extern int dvi_s;			/* max stack depth */
extern int dvi_t;			/* number of pages (bop) in DVI file */
extern int dvi_version;		/* DVI version from header */
extern int dvi_fonts;		/* number of fonts seen */
extern BOOL finish;		/* non-zero => have hit end of DVI file */
extern BOOL bShowViewPorts;	/* debugging output 94/Oct/5 */

extern long hyperpos[];				/* saved position in file */
extern int hyperpage[];				/* saved page number in file */
extern int hyperindex;				/* index into the above */

extern DWORD TeXFilterIndex;	/* previously used filter index (TeX files) */
extern UINT uFindReplaceMsg;		/* registered window message */
extern int bDismissFlag;			/* Dismiss dialog box on search */
extern BOOL bTeXHelpFlag;		/* last call from tex menu was to texhelp */
extern BOOL bTeXHideFlag;		/* original DVI file saved while TeXHelp */
extern BOOL bUseWorkPath;		/* use DVIWindo `working directory' */
extern BOOL bUseTeXDVI;			/* TEXDVI environment variable set */

extern int xLogExt, yLogExt;			/* will be set in winprint.c */
extern BOOL bUseATM;		/* try and connect up to ATM */ /* ATM API */
extern BOOL bAutoActivate;	/* use ATMREG.ATM if possible 96/June/4 */
extern BOOL bUseBase13;		/* use hard wired info for Base 13 fonts */
extern int nMaxDocuments;				/* in dviwindo.c */

extern BOOL bReverseVideo;
extern BOOL bEnableTermination;
extern BOOL bComplainSpecial;		/* complain about bad \special's */
extern BOOL bIgnoreSpecial;			/* ignore all \special's */
extern BOOL bComplainFiles;			/* complain about missing files */
extern BOOL bShowButtons;			/* show hypertext buttons if on */
extern BOOL bElCheapo;				/* fast and cheap TPIC dashes/splines */
extern BOOL bAllowClosed;			/* allow closure of splined figures */
extern BOOL bAllowTPIC;				/* non-zero => allow TPIC \specials */

extern BOOL bSciWord;				/* Scientific Word Figure Inclusion */
extern BOOL bViewExtras;			/* View preview info only 95/Mar/28 */
extern BOOL bFirstNull;				/* flush \special start with null */

extern int marknumber;				/* number of mark found */
extern BOOL bAllowBadBox;	/* allow single % comment before %%BoundingBox */
extern BOOL bForceTIFF;		/* force search for .tif file for .eps file */
extern BOOL bHyperText;		/* allow hypertext linkage 94/Oct/5 */
extern BOOL bFlipRotate;	/* flip rotation in PS code from DVIPS */ 
extern int nFileMax;		/* maximum file number findepsfile */
extern BOOL bBadFileComplained;	/* have complained aready ? 95/Mar/28 */
extern BOOL bBadSpecialComplained;	/* have complained aready ? 96/Feb/4 */
extern long OldBack, OldFore;

// extern DWORD FigureColor, BackColor;
// extern DWORD TextColor, BkColor;	
// extern DWORD FrameColor;			/* default EPS frame color */

extern COLORREF TextColor, BkColor;
extern COLORREF FigureColor, BackColor;
extern COLORREF RuleColor;
extern COLORREF CurrentTextColor, CurrentBackColor;
extern COLORREF FrameColor;			/* default EPS frame color */
extern COLORREF TextColor;		/* RGB(0, 0, 0); default Window Text Color */
extern COLORREF OrgBkColor;		/* RGB(255, 255, 255); default Background Color */
extern COLORREF clrBackground;

extern LPCOLORREF lpBack;			/* FAR pointer to page table for current file */

extern COLORREF ColorStack[MAXCOLORSTACK];

extern int cp_valid;	/* non-zero when last sent out "set" (NOT "put") */ 

extern char *szEPSPath; 
extern BOOL bBGRflag;	/* interchange r and b before CreateDIBitmap *//* ? */
extern BOOL bStretchGray;	/* expand (mingrey - maxgrey) to full range */
extern BOOL bStretchColor;	/* expand color to use dynamic range maximally */

extern BOOL bUnitsWarned;	/* if warned yet about bad units in \special */
extern BOOL bZeroWarned;	/* if warned yet about zero width / height */
extern BOOL bShowImage;			/* show ESPF or TIFF image */
extern int nDefaultTIFFDPI;		/* if resolution not specified in TIFF */

extern unsigned long nspecial;			/* byte count of special */
extern int	pageno;			/* current pagenumber - normally zero based */
extern BOOL bAllowAndrew;	/* try and allow Andrew Trevorrow style */

extern int bCarryColor;			/* carry color across pages 98/Feb/14 */
extern int bColorUsed;			/* This DVI file has \special{color ...} */
extern int bBackUsed;			/* This DVI file has \special{background ...} */

extern BOOL bUpperCase;				/* convert font names to upper case */
extern LPLONG lpPages;			/* FAR pointer to page table for current file */
extern char *szWinDir;				/* 96/July/23 */

extern BOOL bCheckBoldFlag;			/* checked font desired is bold */
extern BOOL bCheckItalicFlag;		/* checked font desired is italic */
extern char *szTeXFonts;		/* TEXFONTS env variable */

extern unsigned long dvi_u;			/* max width */
extern LPLONG lpMarks;			/* FAR pointer to marks table */
extern int maxmarks;			/* maximum number of marks fit in memory */
extern int bUseATMINI;		/* use information from ATM.INI for Type 1 */
extern int bUseTTFINI;		/* use information from WIN.INI for TrueTypes */
extern int bUseUSRINI;		/* use substitution file <filename>.fnt */
extern int bUseTEXINI;		/* use generic substitution dviwindo.fnt */

extern BOOL bTTUsable;		/* Is TrueType Enabled ? */
extern BOOL bTTFontSection;	/* allow for [TTFonts] as well as [Fonts] */

extern BOOL bIgnoreRemapped; /* do not make special case of remapped fonts */
extern BOOL bTeXFontMap;	/* allow use of `texfonts.map' for aliasing */
extern BOOL bUseGetProfileATM;	/* use GetPrivateProfile on atm.ini */
extern BOOL bUseGetProfileTT;	/* use GetPrivateProfile on win.ini */
extern BOOL bUseTTMapping;	/* Using new mapping table for TT */
extern BOOL bAllowVersion;	/* allow mismatch in last char of TT file name */
extern BOOL bAllowTruncate;	/* allow for TFM file name truncation */
extern LPSTR lpFileNames;	/* table of FaceName+Style => FileName */
extern int nFileIndex;		/* index into File Table - number loaded */

extern int MaxPage;				/* remember how much space allocated */
extern int MaxColor;
extern int MaxBack;

extern int BBxll, BByll, BBxur, BByur; 	/* crop \special 96/May/4 */
extern int PageWidth;
extern int PageHeight;
extern int GutterOffset;
extern BOOL bBoldFlag, bItalicFlag;				/* Style Bits */
extern BOOL bUseRegistryFile;		/* It is OK to try and call RegEdit */
extern BOOL bRegistrySetup;			/* RegEdit has already been called */

extern char *szRegistry;		/* in dviwindo.c (or from dviwindo.ini) */

extern int CxLeft, CyTop, CcxWidth, CcyHeight;	// position and size of console window
extern int QxLeft, QyTop, QcxWidth, QcyHeight;	// position and size of console window

extern BOOL bSpinOK;			/* tells whether spinner registered */
extern WORD WindowVersion;		/* set up when starting */

extern BOOL bCommFont;			/* non-zero if new font dialog */
extern BOOL bAlwaysWriteReg;	/* Always dump ttfonts.reg ahead of time */
extern BOOL bUseRegEnumValue;	/* Use KRNL386 imports in Windows 95 */
extern BOOL bNoScriptSel;		/* CD_NOSCRIPTSEL winfonts.c */
extern BOOL bTraceCurveto;		/* find extrema in curveto in WriteAFM */
extern BOOL bAvoidFlood;		/* do not fill triangles */
extern BOOL bUseNewEncodeTT;	/* Use new method for encoding 97/Jan/16 */

extern char *szBasePath;	/* non blank if new directory structure 96/Aug/29 */
extern char *szEncodingVector;			/*  place for encoding vector name */

extern int nEncodingBytes;				/* in winsearc.c */
extern BOOL bCustomEncoding;			/* in winsearc.c */

extern char *szVecPath;					/* default path from VECPATH */
extern char *szPrePath;					/* default path from PREPATH */
extern char *szAFMtoTFMcom;				/* first item if contains . 95/June/23 */
extern char *szAFMtoTFM;				/* passed to AFMtoTFM 95/June/26 */

extern BOOL bShowBoxes;			/* show character boxes */
extern BOOL bResetScale;		/* reset scale each open */
extern BOOL bFontSample;		/* non-zero if want font sample */
extern BOOL bCharacterSample;	/* non-zero if want character sample */
// extern UINT nChar;				/* which character to show */
extern int nChar;				/* which character to show */
extern BOOL bShowWidths;		/* non-zero if want character widths */
extern BOOL bWriteAFM;			/* non-zero if want AFM file */
extern BOOL bKeepZero;			/* keep zero size kern pairs in WriteAFM */
extern BOOL bFontEncoded;		/* non-zero if current font is reencoded */
extern BOOL bDecorative;		/* Family == FF_DECORATIVE => NOT text font */
extern BOOL bDontCare;			/* Family == FF_DONTCARE => not text font */

extern BOOL bATMShowRefresh;	/* non-zero if bATMShow has to be (re)set up */

extern int wantedzoom;			/* current zoom value */

extern int bATMBugFlag;			/* non-zero => need to correct for ATM bug */

extern BOOL extentflag;
extern BOOL getcharflag;		/* where do widths come from in GetWidth */
extern BOOL bTestFlag;				/* use for debugging input */
extern BOOL bIgnoreBadInstall;		/* do not warn about T1INSTALL problem */
extern BOOL bT1InstallWarn;			/* warned about T1INSTALL yet ? */
extern BOOL bT1WarnOnce;			/* warn just for first font */
extern unsigned int TestSize;		/* size to use in tests (TWIPS) winfonts */
extern BOOL bTryATMRegFlag;			/* default for now */
extern char *szControlName;

extern BOOL bShowUsedFlag;			/* show DVI file used in window */
extern BOOL bShowUsedExposed;		/* showused window is not hidden */
extern int UsedxLeft, UsedyTop;	/* window position for `Fonts Used' dialog */
extern BOOL bDrawOutline;
extern BOOL bDrawKnots;
extern BOOL bDrawControls;

extern char *copyright;					/* 95/Mar/28 */

// extern BOOL nDriveD;			/* type of drive D: */
extern UINT nDriveD;				/* type of drive D: */

extern BOOL bSinglePage;		/* `Print Current Page' instead of `Print' */
extern BOOL bISOTROPIC;			/* use MM_ISOTROPIC in MetaFile output */
extern BOOL bPrintOnly;			/* print specified file - no screen display */
extern BOOL bPrintFrame;		/* non-zero if want alignment frames */
extern BOOL bUseDVIPSONE;		/* Use DVIPSONE for printing */
extern BOOL bRunMinimized;		/* Run DVIPSONE and AFMtoTFM minimized */
extern BOOL bPrintToFile;		/* Have DVIPSONE print to file */
extern BOOL bCollateFlag;		/* Collate multiple copy output */
extern BOOL bUseDevModeCopies;	/* Only copies/collate if driver support */
extern BOOL bUseSharedName;		/* Use `shared name' for Ne00: etc */

// extern BOOL bUseAFMtoTFMDLL;	/* AFMtoTFM.DLL 99/June/13 */
// extern BOOL bUseDVIPSONEDLL;	/* DVIPSONE.DLL 99/June/13 */
// extern BOOL bUseYandYTeXDLL;	/* YANDYTEX.DLL 99/June/13 */
extern BOOL bUseDLLs;			/* Use DLLs if available */
extern BOOL bCallBackPass;		/* callback from dvipsone.dll and passthrough */

extern char *szDVIPSONE;		/* passed to DVIPSONE 94/June/3 */
extern char *szDVIDistiller;	/* passed to DVIPSONE/Distiller 99/Dec/30 */
extern char *szDVIPSONEcom;		/* first item if contains . 95/June/23 */
extern char *szDVIPSONEport;	/* port on which DVIPSONE to be used */ 

extern char *szYandYTeXcom;		/* not used ??? */

extern long leftcountzero;	
extern long rightcountzero;
extern int beginpage, endpage;	/* page range in print only call */
extern int pageincrement;		/* count to increment pages by */
extern int bReversePages;		/* non-zero to reverse page order */
extern BOOL bOddEven;			/* invokes new odd/even page scheme */
extern int bAlternatePages;		/* alternate pages only (if bOddEven == 0) */
extern int bOddOnly;			/* show odd pages only (if bOddEven != 0) */
extern int bEvenOnly;			/* show even pages only (if bOddEven != 0) */
extern BOOL bKeepPrinterDC;		/* keep printer DC open - 0 untested */
extern BOOL bKeepDEVMODE;		/* keep DEVMODE when opening new printer */
extern int papertype;			/* DMPAPER_LETTER etc */
extern int maxpapersize;
extern BOOL bPassSize;			/* pass paper size info 95/Jun/22 */
extern BOOL bPassOrient;		/* pass paper orientation info 95/Jun/24 */
extern BOOL bPassDuplex;		/* pass printer duplex info 97/Nov/17 */
extern int ScreenFrequency, ScreenAngle;

extern char *szPrintFile;			/* print to file name (allocated) */
extern int nCopies;				/* number of copies to print */
extern int nDuplex;				/* 96/Nov/17 */
extern int frompage, topage;	/* page range to print - physical pages */
extern BOOL bUseMagicScreen;		/* use nice half-tone screen */
extern BOOL bShowInfoFlag;			/* show DVI file info in window */
extern BOOL bShowInfoExposed;		/* showinfo window is not hidden */

extern int InfoxLeft, InfoyTop;		/* DVI File Info Window Position */
extern char *TitleText; /* "DVIWindo" */
extern char *version;	/*  = VERSION */
extern long serialnumber; 

extern char line[MAXLINE];		/* buffer for reading lines from EPS file */

extern char newmagic[97];			/* coordinate with winsetup.c */
extern char szPreviewHotKey[10];	/* default preview hotkey */

// extern char szRegistryFonts[];	/* in winfonts.c - initially "" */
extern char *szRegistryFonts;		/* in winfonts.c - initially NULL */

extern char encodefont[MAXFONTS];	/* non-zero if encoding already set up */
extern char szFaceName[LF_FACESIZE];			/* Face Name */
extern char szFullName[LF_FULLFACESIZE];		/* Full Name */
extern char dvi_comment[MAXCOMMENT];	/* for TeX comment */
extern char searchtext[MAXSEARCHTEXT];		/* place for search text */
extern char szMenuKey[MAXKEY];		/* 1994/Feb/25 */

extern char *papersize[];
extern char *hyperfile[];			/* saved file name if any - else NULL */

extern BOOL bCallBackPass;	/* callback from dvipsone.dll and passthrough */
extern BOOL bOpenClosePrinter;	/* OpenPrinter/ClosePrinter, not STARTDOC/ENDDOC */extern BOOL bConsoleOpen;	/* keep Console Window Open */
extern BOOL bOpenCloseChannel;	/* OPENCHANNEL/CLOSECHANNEL, not STARTDOC/ENDDOC */
extern BOOL bNewPassThrough;	/* POSTSCRIPT_PASSTHROUGH, not PASSTHROUGH */
extern BOOL bForcePassBack;	/* force call back even when printing to file */
extern BOOL bOldPassThrough;	/* force PASSTHROUGH even for Distiller */
extern BOOL bDontAskFile;	/* don't ask for file name when printing to file */

extern WORD WinVerNum;			/* rearranged bytes of Windows Version 95/Nov/3 */
extern BOOL bQuoteAtSign;			// replace @.tex with "@.tex" automatically
extern BOOL bConvertUnderScores;	// replace _ with space in TFM names
extern BOOL bAllowFontNames;		// allow use of FontNames as TFM names
extern BOOL bUseFontName;			// write TFMs using FontNames, not file names
