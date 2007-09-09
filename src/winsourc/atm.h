/* $Header:   M:\winatm\lib32\atm.h_v   1.16   18 Apr 1997 13:39:38   DARNOLD  $ */
/*
** ATM.h
**
** 32-bit Version 4.02 -- 04-01-97
** Adobe Type Manager is a trademark of Adobe Systems Incorporated.
** Copyright 1983-1997 Adobe Systems Incorporated
** All Rights Reserved

The Adobe parts of this file are publicly available at
http://partners.adobe.com/public/developer/ps/sdk/index_archive.html#atmapi
(as of September 2007).

The Y&Y-specific parts are hereby placed in the public domain.
*/

#ifndef _H_ATM
#define _H_ATM

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ATMAPI
#define ATMAPI       __stdcall
#define ATMCALLBACK  __stdcall
#define ATMFAR
#endif

#define BD_VERSION_260  3  /* Backdoor version number. */
#define BD_VERSION_300  3
#define BD_VERSION_301  3
#define BD_VERSION_400  4
#define BD_VERSION_401  4
#define BD_VERSION_402  4
#define BD_VERSION      BD_VERSION_402

/* Define return values for non-BOOL functions. */
#define ATM_NOERR               (0)    /* Normal return. */
#define ATM_INVALIDFONT         (-1)   /* Font not consistent. */
#define ATM_CANTHAPPEN          (-2)   /* Some kind of internal error. */
#define ATM_BADMATRIX           (-3)   /* Matrix inverse undefined. */
#define ATM_MEMORY              (-4)   /* Out of memory. */
#define ATM_NOTSUPPORTED        (-5)   /* Running ATM doesn't support call. */
#define ATM_NOTRUNNING          (-6)   /* ATM is not running. */
#define ATM_CANCELLED           (-8)   /* Client halted operation. */
#define ATM_NOCHAR              (-9)   /* No outline for char code in font. */
#define ATM_BADPROC             (-100) /* Bad callback proc address. */
#define ATM_CANTDRAW            (-101) /* Error in imaging; a bad rop? */
#define ATM_BADPARM             (-102) /* Bad parameter passed in. */
#define ATM_SELECTED            (-200) /* For compatibility with old headers.
                                       ** See ATMSelectObject.
                                       */
#define ATM_NEWFONTSELECTED     (-200) /* See ATMSelectObject. */
#define ATM_OLDFONTSELECTED     (-201) /* See ATMSelectObject. */
#define ATM_FOREIGNFONTSELECTED (-202) /* See ATMSelectObject. */
#define ATM_SELECTERROR         (-204) /* See ATMSelectObject. */

#define ATM_ERROR               (-219) /* General error, such as out of memory,
                                       ** or input string too long. 
                                       */

#define ATM_NOTFOUND            (-223) /* Font not found. */
#define ATM_NOFONTS             (-224) /* No fonts were enumerated. */
#define ATM_SYSTEM              (-226) /* Operating system call error. */
#define ATM_BUSY                (-227) /* This reentry of the library requires
                                       ** a resource that is still in use.
                                       */

#define ATM_PATHTOOLONG         (-400) /* At least one path is too long. */
#define ATM_BADFONTTYPE         (-401) /* The font is uncompressed. */
#define ATM_BADSTYLE            (-402) /* A style with bits other than 
                                       ** ATM_ITALIC or ATM_BOLD.
                                       */
#define ATM_NOROOM              (-403) /* Can't add to internal tables. */
#define ATM_NOFONT              (-404) /* At least one path doesn't point to
                                       ** the correct file.
                                       */
#define ATM_BADMENUNAME         (-405) /* Missing/invalid menu name given. */
#define ATM_FONTINUSE           (-406) /* The paths and menu name specify an
                                       ** existing font that is in use.
                                       */
#define ATM_FONTPRESENT         (-407) /* The paths and menu name specify an
                                       ** existing font which is NOT in use.
                                       */
#define ATM_FONTDIFFERENT       (-408) /* The font, as specified by its menu
                                       ** name and styles, exists but its data
                                       ** files are different from those
                                       ** specified by lpMetricsFile and
                                       ** lpFontFile parameters.
                                       */
#define ATM_FONTABSENT          (-409) /* There is no such font installed. */
#define ATM_ADDERROR            (-410) /* General ATMAddFont error. */
#define ATM_CREATEFILE          (-415) /* Couldn't create file. */
#define ATM_USEFILE             (-416) /* Couldn't read, write, or close file.*/
#define ATM_ADDDELAYED          (-417) /* Add incomplete, pending file access.*/
#define ATM_MMMISSING           (-418) /* Add and/or FontStatus could not
                                       ** process instance because base font
                                       ** not installed.
                                       */
#define ATM_NOMMMFORMM          (-419) /* An attempt was made to add an MM Base
                                       ** font with a pfm/pfb pair:  an mmm is
                                       ** required for this.
                                       */
#define ATM_ALREADYINSTALLED    (-2000)/* Font was already installed. */
#define ATM_NOFONTFILE          (-2001)/* Bad or missing .pfb file. */
#define ATM_NOMETRICSFILE       (-2002)/* Bad or missing .pfm or .mmm file. */

// return values for font database
#define ATM_BADPSNAME           (-2100) // Invalid PostScript font name.
#define ATM_NODATABASE          (-2101) // No database was found at ATM startup,
                                        // or ATM was unable to initialize the
                                        // database.
#define ATM_DBERROR             (-2102) // Database error, typically a corrupt or
                                        // malformed database. 

// return values for PostScript stub and .pfm generation
#define ATM_NOMM                (-2200)/* Font is not MM instance or substitute.*/
#define ATM_MMMVERSION          (-2201)/* Old version of MMM file */
#define ATM_RESOURCE            (-2202)/* Resource error. */


/* Define additional flags for the wOption parameter of the ...XYShow... */
/* routines.  These flags are NOT supported in all versions of the ATM   */
/* backdoor.  Check the version numbers for each option flag below.      */
#define ATM_USELOGCOLOR         (0x0100) /* In ATM backdoor since version 2.01
                                         ** This flag causes ATM to use the
                                         ** logical color set for text
                                         ** foreground in the DC even when
                                         ** it would normally switch to the
                                         ** nearest pure color based on the
                                         ** settings in ATM.INI.
                                         ** This is mutually exclusive with
                                         ** ATM_USEPURECOLOR.  Incorrect usage
                                         ** will cause an error.
                                         */
#define ATM_USEPURECOLOR        (0x0200) /* In ATM backdoor since version 2.01
                                         ** This flag causes ATM to use the
                                         ** nearest pure color instead of the
                                         ** actual logical color set for text
                                         ** foreground in the DC.
                                         ** This is mutually exclusive with
                                         ** ATM_USELOGCOLOR.  Incorrect usage
                                         ** will cause an error.
                                         */
#define ATM_MAKEVISIBLE         (0x0400) /* In ATM backdoor since version 2.01
                                         ** This flag is used for B/W devices
                                         ** and causes ATM to use either white
                                         ** or black depending on the color of
                                         ** background in order to make the
                                         ** text visible.
                                         ** If this flag is specified,
                                         ** ATM_USELOGCOLOR & ATM_USEPURECOLOR
                                         ** are ignored.
                                         */


/* Define flags for the 'options' parameter of the ATMSelectObject routine */
/* These flags are NOT supported in all versions of ATM.  Check the        */
/* version numbers for each option flag below.                             */
#define ATM_DEFER               (0x0001) /* In ATM backdoor since version 2.01
                                         ** This flag causes ATM to defer to a
                                         ** device font when selecting a font
                                         ** handle into a DC the first time.
                                         ** This is mutually exclusive with
                                         ** ATM_SELECT.  Incorrect usage will
                                         ** cause an error.
                                         */
#define ATM_SELECT              (0x0002) /* In ATM backdoor since version 2.01
                                         ** This flag forces ATM to render a
                                         ** font even when normally it would
                                         ** be deferred to the device.  This
                                         ** can happen only when selecting a
                                         ** handle into a DC the first time.
                                         ** This is mutually exclusive with
                                         ** ATM_DEFER.  Incorrect usage will
                                         ** cause an error.
                                         */
#define ATM_USEEXACTWIDTH       (0x0004) /* In ATM backdoor since version 2.01
                                         ** This flag causes ATM to create the
                                         ** font being selected into a screen
                                         ** DC without adjusting the width
                                         ** data.  This can happen only when
                                         ** selecting a handle into a DC for
                                         ** the first time.
                                         */


#define ATM_ITALIC              (0x0001) /* In ATM backdoor since version 2.6.
                                         ** This flag is used in calls to and
                                         ** return values from ATM to indicate
                                         ** the style of a font's face name.
                                         ** If set, it indicates that the style
                                         ** associated with the font is ITALIC.
                                         */
#define ATM_BOLD                (0x0002) /* In ATM backdoor since version 2.6.
                                         ** This flag is used in calls to and
                                         ** return values from ATM to indicate
                                         ** the style of a font's face name.
                                         ** If set, it indicates that the style
                                         ** associated with the font is BOLD.
                                         */
#define ATM_BOLDITALIC          (ATM_BOLD | ATM_ITALIC)


/* Type bits for ATMEnumFonts() and ATMAddFont() */
#define ATM_TYPE1               (0x0100) /* Normal Type 1 pfb file. */
#define ATM_MMTYPE1             (0x0200) /* Multiple Master Type 1 pfb file. */
#define ATM_MMINSTANCE          (0x0400) /* Instance of Multiple Master font.*/
#define ATM_DATABASE            (0x0800) /* Database substitution font. */
#define ATM_TEMPORARY           (0x1000) /* Can be ORed with the other bits. */
#define ATM_REGISTRY            (0x2000) // Can be ORed with the other bits. 

#define ATM_PSNAMESIZE          (64)     /* max length PostScript font name */
#define ATM_MAXPATHLEN          (260)    /* max length for .pfb & .pfm paths */
                            

/* mmFlags for ATMMMMetricsHeader structure */
#define MM_SUBSTOK              (0x01)   /* Font usable for substitution. */
#define MM_TEXTINSTOK           (0x02)   /* Font usable for text instances. */
#define MM_IS_PI                (0x04)   /* Font is decorative. */
#define MM_IS_FIXED_PITCH       (0x08)   /* Font is fixed pitch. */

#define ATM_MAXAXES             (4)      /* Max number of axes for MM font */
#define ATM_MAXMASTERS          (16)     /* Max number of master designs for
                                            a MM font. */

/* mmAxisAttrs for ATMMMMetricsHeader structure */
#define MM_IS_ITALIC            (0x001)
#define MM_IS_BOLD              (0x002)
#define MM_IS_SERIF             (0x004)
#define MM_ITALIC_AXIS          (0x100)
#define MM_WEIGHT_AXIS          (0x200)
#define MM_SERIF_AXIS           (0x400)


/* Define bits for various flags supported by ATMSetFlags. */
#define ATM_DOWNLOAD            (0x0001)// enable soft fonts for printer drivers
#define ATM_USEDEVFONTS         (0x0002)// defer to device fonts
#define ATM_SUBSTITUTE          (0x0004)// enable font substitution
#define ATM_ANTIALIAS           (0x0008)// enable anti-alias for the screen
#define ATM_AUTOACTIVATE        (0x0010)// enable automatic font activation
#define ATM_GDIFONTS            (0x0020)// enable "gdi" fonts for screen drivers

/* Define return values for ATMFontAvailable, *lpFromOutline */
#define ATM_SYNTH               (0)     // style may be synthethized
#define ATM_REAL                (1)     // exact font & style are available
#define ATM_SUBST               (2)     // substitute font & style available

#define MI_FACESIZE LF_FACESIZE


typedef long int ATMFixed;  /* 16.16 signed number.  Integer in hi word. */

typedef struct
  {
  ATMFixed a, b, c, d, tx, ty;
  } ATMFixedMatrix, ATMFAR *LPATMFixedMatrix;

typedef struct
  {
  ATMFixed x, y;
  } ATMFixedPoint, ATMFAR *LPATMFixedPoint;

typedef struct
  {
  short int x, y;
  } ATMShortPoint, ATMFAR *LPATMShortPoint;

typedef struct
  {
  ATMShortPoint ll, ur;
  } ATMBBox, ATMFAR *LPATMBBox;

typedef struct
  {
  char faceName[LF_FACESIZE];
  WORD styles;
  } ATMFontSpec, ATMFAR *LPATMFontSpec;

typedef struct
  {
  WORD  mmVersion;
  WORD  mmFlags;
  char  mmCopyright[72];
  char  mmFontVersion[8];
  char  mmPSFontName[80];
  WORD  mmAxisAttrs;
  BYTE  mmFamily;   // MS family designations.
  BYTE  mmEncoding;
  WORD  mmCharSet;
  WORD  mmFirstChar;
  WORD  mmLastChar;
  WORD  mmDefaultChar;
  WORD  mmBreakChar;
  WORD  mmNumKernPairs;
  WORD  mmNumKernTracks;
  WORD  mmNumUnencodedChars;
  WORD  mmSizeUnencodedCharNames; // In Bytes
  WORD  mmNumUnencodedKernPairs;
  short mmMasterUnits;
  short mmMasterHeight;
  DWORD mmSizeDesignPoint; // Size of one MMDesignPoint for this font.
  WORD  mmNumAxes;
  WORD  mmNumBaseDesigns;
  WORD  mmNumPrimaryDesigns;
  char  mmFileNamePrefix[6];
  char  mmMenuName[MI_FACESIZE];  // Has to be < MI_FACESIZE!
  DWORD mmSizeDVSubrs;            // Includes MMDVSubrs struct
  DWORD mmReserved[19];
  } ATMMMMetricsHeader, ATMFAR *LPATMMMMetricsHeader;


typedef struct
  {
  LPSTR lpszInstName;
  WORD  styles;
  } ATMEnumMMInstInfo, ATMFAR *LPATMEnumMMInstInfo;

// lpEnumData is type LPMMMetricsHeader when ATMEnumMMFonts is called with NULL
// lpEnumData is type LPEnumMMInstInfo when ATMEnumMMFonts is called with name 

typedef struct
  {
  char pfbPath[ATM_MAXPATHLEN]; 
  char pfmPath[ATM_MAXPATHLEN];
  char mmmPath[ATM_MAXPATHLEN];
  char pssPath[ATM_MAXPATHLEN];
  } ATMFontPathInfo, ATMFAR *LPATMFontPathInfo;

typedef struct
  {
  DWORD	dwATMVersionInfoSize;		// set to sizeof(ATMVersionInfo)
  DWORD	dwDrvMajorVersion;			// ATM.DLL for Win95; ATMDRVR.DLL for WinNT
  DWORD dwDrvMinorVersion;
  DWORD dwLibMajorVersion;			// zero for Win95; ATMLIB.DLL for WinNT
  DWORD dwLibMinorVersion;
  char  szBuildStrings[128];		// one or two null terminated build strings
									// Driver followed by Library
  } ATMVersionInfo, ATMFAR *LPATMVersionInfo;


#define ATMINTTOFIXED(n)        ((ATMFixed) (((long int) (n)) << 16))

#ifndef ATMBD_IMPL

/*
** This variable gives the compilation time stamp of the static library
*/
extern ATMFAR char ATMLibDateTime [];

/* 
** NOTE: Unless otherwise noted, the following functions that return ints
** use ATM_NOERR to indicate success. In addition to the specific values 
** noted with each function, these functions can all return the following 
** error values:
**
**      ATM_ERROR
**      ATM_CANTHAPPEN
**      ATM_NOTSUPPORTED
**      ATM_NOTRUNNING
**      ATM_BADPARM
**      ATM_BADSTYLE
*/


extern BOOL ATMAPI ATMProperlyLoaded (void);
        /*
        ** In ATM backdoor since version 1.0
        **
        ** This routine initializes the backdoor and returns true if all
        ** goes well and if ATM is loaded and running.  It will return false
        ** if ATM is not installed or loaded.
        **
        ** NOTE: You must have a call to 'ATMProperlyLoaded',
        **       or all other library calls will fail.
        **
        ** This function is not designed for nesting. After a successful call
        ** to ATMProperlyLoaded, subsequent calls are ignored and return
        ** ATM_NOERR.
        */


extern int ATMAPI ATMFinish (void);
        /* 
        ** In ATM backdoor since version 3.02 (Win32 only)
        **
        ** This routine unloads the backdoor and cleans things up.
        **
        ** NOTE: You must call ATMFinish when your program exits
        **       or you will have a memory leak!
        **
        ** This function is not designed for nesting. The first call
        ** to ATMFinish terminates the library, and subsequent calls return
        ** ATM_NOTRUNNING.
        **
        ** Failure return values are:
        **     ATM_NOTRUNNING   (Library is not initialized)
        **     ATM_BUSY         (A callback has not returned)
        */


extern int ATMAPI ATMBeginFontChange (void);
        /* 
        ** In ATM backdoor since version 2.5
        **
        ** Starts a series of font change operations consisting of calls to
        ** the routines ATMAddFont and ATMRemoveFont.  Must be called before
        ** calling ATMAddFont or ATMRemoveFont.
        */


// backdoor API
#define ATMFontStatus(lpMenuName, styleAndType, lpMetricsFile, lpFontFile) \
    libATMFontStatus(BD_VERSION, lpMenuName, styleAndType, lpMetricsFile, \
                      lpFontFile)

// libATMFontStatus is for internal use only.  Applications should call
// ATMFontStatus.
extern int ATMAPI libATMFontStatus (
                                WORD  wBdVersion,
                                LPSTR lpMenuName,
                                WORD  styleAndType,
                                LPSTR lpMetricsFile,
                                LPSTR lpFontFile);
        /* 
        ** extern int ATMAPI ATMFontStatus (
        **                      LPSTR lpMenuName,
        **                      WORD  styleAndType,
        **                      LPSTR lpMetricsFile,
        **                      LPSTR lpFontFile);
        **
        ** In ATM backdoor since version 2.5
        **
        ** Note: this function is not required in Win95 ATM 4.0, except to 
        ** determine if a font is in use. It is not supported in WinNT.
        **
        ** This routine determines whether or not a given font is currently
        ** installed and/or in use by ATM.  It should be called prior to
        ** installing a new font.
        ** If the return value is ATM_FONTINUSE, then no attempt should be made
        ** to install the font.
        ** 
        ** The lpMenuName contains the Windows name of the font. 
        **
        ** The styleAndType parameter contains none, one or more of:
        **     ATM_BOLD            -- a bold font
        **     ATM_ITALIC          -- an italic font
        **     ATM_TYPE1           -- a normal Type 1 font
        **     ATM_MMTYPE1         -- a Multiple Master Type 1 font
        ** Note that the 2.5 version of this routine allowed only the two style
        ** flags (ATM_BOLD and ATM_ITALIC) to be specified.
        **
        ** The lpMetricsFile contains the full pathname to the metrics file
        ** for the font.  This is a .pfm file for an ATM_TYPE1 or an .mmm file
        ** for an ATM_MMTYPE1.
        **
        ** The lpFontFile contains the full pathname to the font file for
        ** the font.  This is a .pfb file for both ATM_TYPE1 and ATM_MMTYPE1.
        **
        ** Successful return values are:
        **     ATM_FONTINUSE            
        **     ATM_FONTPRESENT          
        **     ATM_FONTDIFFERENT        
        **     ATM_FONTABSENT           
        **
        ** Failure return values are:
        **     ATM_PATHTOOLONG          
        **     ATM_BADFONTTYPE          
        **     ATM_NOFONT               
        **     ATM_BADMENUNAME          
        */


// backdoor API
// libATMAddFont is for internal use only.  Applications should call
// ATMAddFont.
#define ATMAddFont(lpMenuName, styleAndType, lpMetricsFile, lpFontFile) \
    libATMAddFont(BD_VERSION, lpMenuName, styleAndType, lpMetricsFile,  \
                   lpFontFile)
extern int ATMAPI libATMAddFont (
                                WORD   wBdVersion,
                                LPSTR  lpMenuName,
                                WORD   styleAndType,
                                LPSTR  lpMetricsFile,
                                LPSTR  lpFontFile);

        /* 
        ** extern int ATMAPI ATMAddFont (
        **                      LPSTR lpMenuName,
        **                      WORD  styleAndType,
        **                      LPSTR lpMetricsFile,
        **                      LPSTR lpFontFile);
        **
        ** In ATM backdoor since version 2.5
        **
        ** This routine makes a newly installed font available to ATM.
        ** 
        ** The lpMenuName contains the Windows name of the font. 
        **
        ** The styleAndType parameter contains none, one or more of:
        **     ATM_BOLD            -- a bold font
        **     ATM_ITALIC          -- an italic font
        **     ATM_TYPE1           -- a normal Type 1 font
        **     ATM_MMTYPE1         -- a Multiple Master Type 1 base font
        **     ATM_MMINSTANCE      -- a Multiple Master instance
        **     ATM_TEMPORARY       -- can be ORed with the other bits 
        **     ATM_REGISTRY        -- can be ORed with the other bits
        **
        ** Note that the 2.5 version of this routine allowed only the two style
        ** flags (ATM_BOLD and ATM_ITALIC) to be specified.
        **
        ** The lpMetricsFile contains the full pathname to the metrics file
        ** for the font.  This is a .pfm file for an ATM_TYPE1. This is
        ** an .mmm file for an ATM_MMTYPE1.
        **
        ** The lpFontFile contains the full pathname to the font file for
        ** the font.  This is a .pfb file for both ATM_TYPE1 and ATM_MMTYPE1.
        **
        ** ATM_TEMPORARY bit may be set to add a font that is not enumerated.
        ** Note: this bit is not supported on WinNT ATM 4.0, and will cause
        ** an error return of ATM_BADPARM.
        **
        ** ATM_REGISTRY bit may be set to automatically create a Windows 
        ** registry entry after successfully adding the font. The registry 
        ** entry causes the font to be added automatically whenever Windows 
        ** boots. This bit is supported only on WinNT. On Win95, the caller
        ** may separately add appropriate entries to ATM.INI and WIN.INI to
        ** achieve the same result.
        **
        ** An MM base font is added by setting ATM_MMTYPE1 and passing the
        ** paths of the MMM and PFB files. An MM instance is added by setting
		** ATM_MMINSTANCE and passing the paths of the PFM and PFB files.
		** The default instance (i.e., an instance with no coordinates
        ** in the name) should be added for compatibility with the PostScript
        ** driver or other components.
        **
        ** Failure return values:
        **     ATM_INVALIDFONT          
        **     ATM_PATHTOOLONG          
        **     ATM_BADFONTTYPE          
        **     ATM_NOROOM               
        **     ATM_NOFONT               
        **     ATM_BADMENUNAME          
        **     ATM_FONTINUSE
        **     ATM_MMMVERSION
        **                              
        */


// backdoor API
#define ATMRemoveFont(lpMenuName, style) \
    libATMRemoveFont(BD_VERSION, lpMenuName, style)

// libATMRemoveFont is for internal use only.  Applications should call
// ATMRemoveFont.
extern int ATMAPI libATMRemoveFont (
                                WORD  wBdVersion,
                                LPSTR lpMenuName,
                                WORD  style);
        /* 
        ** extern int ATMAPI ATMRemoveFont (
        **                      LPSTR lpMenuName,
        **                      WORD  style);
        ** 
        ** In ATM backdoor since version 2.5
        ** 
        ** This call makes a font inaccessible to ATM. Any subsequent
        ** requests to ATM for this font will fail.
        ** 
        ** The lpMenuName contains the Windows name of the font. 
        **
        ** The style parameter contains none, one or both of the flags
        ** ATM_ITALIC and ATM_BOLD.
        **
        ** ATM_REGISTRY bit may be set in the style parameter to automatically
        ** remove a Windows registry entry after successfully removing the font.
        ** This bit is supported only on WinNT. On Win95, the caller may
        ** separately remove appropriate entries from ATM.INI and/or WIN.INI to
        ** achieve the same result.
		**
		** MM instances should be removed before removing the base MM font.
        **
        ** Failure return values include:
        **     ATM_NOTFOUND
        **     ATM_BADMENUNAME          
        */


extern int ATMAPI ATMForceFontChange (void);
        /* 
        ** In ATM backdoor since version 3.0
        ** 
        ** Forces a WM_FONTCHANGE message to be sent when ATMEndFontChange()
        ** is called regardless of whether a font was added or not.
        */


extern int ATMAPI ATMEndFontChange (void);
        /* 
        ** In ATM backdoor since version 2.5
        ** 
        ** Finishes a series of font change operations consisting of calls to
        ** the routines ATMAddFont and ATMRemoveFont.
        ** Call this function after font changes are completed, or your font
        ** changes may not take effect.
        */

// types for ATMEnumFonts callback
typedef BOOL (ATMCALLBACK ATMEnumFontProc)(LPLOGFONTA lpLogFont,
                                           LPSTR      lpPostScriptName,
                                           WORD       flags,
                                           DWORD      dwUserData);

typedef ATMEnumFontProc ATMFAR *LPATMEnumFontProc;

// backdoor API
#define ATMEnumFonts(lpProc, dwUserData) \
    libATMEnumFonts(BD_VERSION, lpProc, dwUserData)

// libATMEnumFonts is for internal use only.  Applications should call
// ATMEnumFonts.
extern int ATMAPI libATMEnumFonts (
                                WORD              wBdVersion,
                                LPATMEnumFontProc lpProc,
                                DWORD             dwUserData);
        /*
        ** extern int ATMAPI ATMEnumFonts(
        **                      LPATMEnumFontProc lpProc,
        **                      DWORD             dwUserData);
        **
        ** In ATM backdoor since version 2.6
        **
        ** This routine enumerates all of ATM's fonts. These fonts may
        ** be Type 1 fonts, Multiple Master Type 1 fonts, 
        ** Multiple Master instances, or database fonts.
        **
        ** The callback procedure lpProc is called once for each 
        ** font. The callback is called with a LOGFONT for the font,
        ** a pointer to the PostScript name of the font, and a flag 
        ** that indicates the type of the font.
        **
        ** Font types are: ATM_TYPE1, ATM_MMTYPE1 and ATM_MMINSTANCE.
        **
        ** If the callback returns zero, the enumeration halts.
        ** 
        ** Returns ATM_CANCELLED if the callback returns zero.
        **
        ** Returns ATM_BUSY if another callback has not returned.
        **
        ** NOTE: If no fonts are installed, the callback will not be 
        ** called, and this function will return ATM_NOFONTS.
        */


// types for ATMEnumMMFonts callback
typedef BOOL (ATMCALLBACK ATMEnumMMFontProc)(LPVOID   lpEnumData,
                                             DWORD    dwUserData);

typedef ATMEnumMMFontProc ATMFAR *LPATMEnumMMFontProc;

// backdoor API
#define ATMEnumMMFonts(lpszMMMenuName, lpProc, dwUserData) \
    libATMEnumMMFonts(BD_VERSION, lpszMMMenuName, lpProc, dwUserData)

// libATMEnumMMFonts is for internal use only.  Applications should call
// ATMEnumMMFonts.
extern int ATMAPI libATMEnumMMFonts (
                                WORD                wBdVersion,
                                LPSTR               lpszMMMenuName,
                                LPATMEnumMMFontProc lpProc,
                                DWORD               dwUserData);
        /* 
        ** extern int ATMAPI ATMEnumMMFonts (
        **                      LPSTR               lpszMMMenuName,
        **                      LPATMEnumMMFontProc lpProc,
        **                      DWORD               dwUserData);
        **
        ** In ATM backdoor since version 2.6
        ** 
        ** This function enumerates installed multiple master base fonts and 
        ** instances.  To enumerate base fonts, the caller sets lpszMMMenuName
        ** to NULL, and the callback procedure receives a pointer to an
        ** ATMMMMetricsHeader structure.  To enumerate instances, the caller
        ** passes in a multiple master base font menu name, and the callback
        ** procedure receives a pointer to an ATMEnumMMInstInfo structure.
        **
        ** Returns ATM_BUSY if another callback has not returned.
        **
        */

extern BOOL ATMAPI ATMFontAvailable (
                                LPSTR        lpFacename,
                                int          nWeight,
                                BYTE         cItalic,
                                BYTE         cUnderline,
                                BYTE         cStrikeOut,
                                int ATMFAR * lpFromOutline);
        /*
        ** In ATM backdoor since version 1.0
        ** Semantics of *lpFromOutline extended in 4.0
        **
        ** Returns TRUE if ATM can render some style of the specified font.
        ** *lpFromOutline is non-zero if ATM has correct metrics for the style.
        ** When ATMFontAvailable is TRUE, the values for *lpFromOutline are:
        **   ATM_SYNTH (0) - ATM can render a font in the same family. The style
        **       may be synthesized by smearing and/or skewing.
        **       Metrics are not exact.
        **   ATM_REAL  (1) - ATM can render the requested font and style from a
        **       true outline. The metrics are correct.
        **   ATM_SUBST (2) - ATM can create a substitute font for the requested
        **       font and style. The metrics are correct.
        */


extern BOOL ATMAPI ATMFontSelected (HDC hDC);
        /* 
        ** In ATM backdoor since version 1.0
        **
        ** Returns TRUE if the currently selected font in the hDC is an
        ** ATM font.
        */


extern void ATMAPI ATMGetBuildStr (LPSTR lpBldStr);
        /* 
        ** In ATM backdoor since version 4.00
        **
        **  Returns the build string in *lpBldStr.
        **  The build string is intended for display to users, and includes
        **  encoded information on version number, DLL type, build number,
        **  release name, etc. For example, the string for ATM 4.0 is
        **  "R v4.00-32S058G06NN".
        */


extern int ATMAPI ATMGetFontBBox (
                                HDC       hDC,
                                LPATMBBox lpFontBBox);
        /* 
        ** In ATM backdoor since version 2.01
        **
        ** This routine obtains the font bounding box for the font currently
        ** selected into 'hDC'.
        **
        ** lpFontBBox is expressed in unscaled character units.  If NULL,
        ** ATM_BADPARM is returned.
        */
          

// backdoor API
#define ATMGetFontPaths(lpFontSpec, lpFontPaths) \
    libATMGetFontPaths(BD_VERSION, lpFontSpec, lpFontPaths)

// libATMGetFontPaths is for internal use only.  Applications should call
// ATMGetFontPaths.
extern int ATMAPI libATMGetFontPaths (
                                WORD              wBdVersion,
                                LPATMFontSpec     lpFontSpec,
                                LPATMFontPathInfo lpFontPaths);
        /* 
        ** extern int ATMAPI ATMGetFontPaths (
        **                      LPATMFontSpec     lpFontSpec,
        **                      LPATMFontPathInfo lpFontPaths);
        **
        ** In ATM backdoor since version 2.6
        **
        ** Given a font name and style via the lpFontSpec, this routine
        ** returns the font path and file names in the structure pointed
        ** to by lpFontPaths.
        **
        ** The following table indicates which paths will be nonempty
        ** for particular types of fonts. An 'x' indicates that this
        ** path will be always set. A 'y' indicates that this path is set 
		** on Windows NT versions only.
        **                      
        **                      PFB     PFM     MMM     PSS
        ** ATM_TYPE1             x       x
        ** ATM_MMTYPE1           x       y       x
        ** ATM_MMINSTANCE        x               x        
        **
        ** Failure return values include:
        **     ATM_NOFONT
        **     ATM_NOFONTFILE
        **     
        */


// backdoor API
#define ATMGetMenuName(lpPostScriptName, lpFont) \
    libATMGetMenuName(BD_VERSION, lpPostScriptName, lpFont)

// libATMGetMenuName is for internal use only.  Applications should call
// ATMGetMenuName.
extern int ATMAPI libATMGetMenuName(
                                WORD            wBdVersion,
                                LPSTR           lpPostScriptName,
                                LPATMFontSpec   lpFont);
       /* 
       ** extern int ATMAPI ATMGetMenuName(
       **                       LPSTR           lpPostScriptName,
       **                       LPATMFontSpec   lpFont);
       **
       ** In ATM backdoor since version 2.6
       **
       ** Takes a PostScript name and looks up the Windows name.
       ** This function looks at the list of currently installed
       ** fonts and potentially at the font database.
       **
       ** The lpPostScriptName is the input argument, which is a pointer 
       ** to a string containing the PostScript name of the font in question. 
       ** The Windows name and style bits are returned in the LPATMFontSpec 
       ** argument. The ATM_ITALIC and ATM_BOLD bits indicate the styles.
       **
       ** Possible return values include:
       **    ATM_NOTFOUND
       **    ATM_NODATABASE
       **    ATM_DBERROR
       **    ATM_BADPSNAME
       */


// types for ATMGetOutline callbacks
// ClosePath proc types
typedef BOOL (ATMCALLBACK ATMClosePathProc)(
                                DWORD           dwUserData);
typedef ATMClosePathProc *LPATMClosePathProc;

// MoveTo proc types
typedef BOOL (ATMCALLBACK ATMMoveToProc)(
                                LPATMFixedPoint lpFixPnt,
                                DWORD           dwUserData);
typedef ATMMoveToProc *LPATMMoveToProc;

// LineTo proc types
typedef BOOL (ATMCALLBACK ATMLineToProc)(
                                LPATMFixedPoint lpFixPnt,
                                DWORD           dwUserData);
typedef ATMLineToProc *LPATMLineToProc;

// CurveTo proc types
typedef BOOL (ATMCALLBACK ATMCurveToProc)(
                                LPATMFixedPoint lpFixPnt1,
                                LPATMFixedPoint lpFixPnt2,
                                LPATMFixedPoint lpFixPnt3,
                                DWORD           dwUserData);
typedef ATMCurveToProc *LPATMCurveToProc;

// backdoor API
#define ATMGetOutline2 ATMGetOutline	// for compatibility with old libraries
extern int ATMAPI ATMGetOutlineA(
                                HDC                 hDC,
                                WORD                wc,		// MultiByte Code
                                LPATMFixedMatrix    lpMatrix,
                                LPATMMoveToProc     lpProcMoveTo,
                                LPATMLineToProc     lpProcLineTo,
                                LPATMCurveToProc    lpProcCurveTo,
                                LPATMClosePathProc  lpProcClosePath,
                                DWORD               dwUserData);
extern int ATMAPI ATMGetOutlineW(
                                HDC                 hDC,
                                WORD                wc,		// Unicode
                                LPATMFixedMatrix    lpMatrix,
                                LPATMMoveToProc     lpProcMoveTo,
                                LPATMLineToProc     lpProcLineTo,
                                LPATMCurveToProc    lpProcCurveTo,
                                LPATMClosePathProc  lpProcClosePath,
                                DWORD               dwUserData);
#ifdef UNICODE
 #define ATMGetOutline ATMGetOutlineW
#else
 #define ATMGetOutline ATMGetOutlineA
#endif
        /* 
        ** In ATM backdoor since version 1.0
        **
        ** Reports the outline of the specified character.
        **
        ** Beginning with ATM 4.0, the outline returned is HINTED.
        ** The effects of hinting can be minimized by requesting an outline
        ** at the EM size of the font.
        **
        ** If lpMatrix is NULL, the outline is determined by the original
        ** LOGFONT.
        **
        ** If a matrix is supplied, the effects differ on Win95 and WinNT.
        ** On Win95, the supplied matrix REPLACES the information from the
        ** LOGFONT. On WinNT, the supplied matrix is CONCATENATED with the
        ** matrix derived from the original LOGFONT.
        **
        ** Returns ATM_BUSY if another callback has not returned.
        **
        */

// backdoor API
#define ATMGetPostScriptName(lpFontSpec, lpPostScriptName) \
    libATMGetPostScriptName(BD_VERSION, lpFontSpec, lpPostScriptName)

// libATMGetPostScriptName is for internal use only.  Applications should
// call ATMGetPostScriptName.
extern int ATMAPI libATMGetPostScriptName(
                                WORD            wBdVersion,
                                LPATMFontSpec   lpFontSpec,
                                LPSTR           lpPostScriptName);
        /* 
        ** extern int ATMAPI ATMGetPostScriptName(
        **                     LPATMFontSpec   lpFontSpec,
        **                     LPSTR           lpPostScriptName);
        **
        ** In ATM backdoor since version 2.6
        **
        ** Given a font name and style via the lpFontSpec, this routine
        ** returns the postscript name in the buffer pointer to by 
        ** lpPostScriptName. This buffer must be ATM_PSNAMESIZE bytes in
        ** size.
        **
        ** Possible return values include:
        **   ATM_NOTFOUND
        */


extern WORD ATMAPI ATMGetVersion (void);
        /* 
        ** In ATM backdoor since version 1.0
        **
        ** Returns the version number of the currently running ATM.  Minor
        ** number in high byte and major number in low byte.
        */

extern BOOL ATMAPI ATMGetVersionEx (LPATMVersionInfo);
	    /* 
		** In ATM backdoor since library version 4.02
		**
		** Returns ATMVersionInfo structure (similar to Windows OSVERSIONINFO)
		*/


// backdoor API
#define ATMMakePFM(lpFaceName, wStyles, hFile, lpFileName) \
    libATMMakePFM(BD_VERSION, lpFaceName, wStyles, hFile, lpFileName)

// libATMMakePFM is for internal use only.  Applications should
// call ATMMakePFM.
extern int ATMAPI libATMMakePFM (
                                WORD  wBdVersion,
                                LPSTR lpFaceName,
                                WORD  wStyles,
                                HFILE hFile,
                                LPSTR lpFileName);
        /* 
        ** extern int ATMAPI ATMMakePFM (
        **                      LPSTR lpFaceName,
        **                      WORD  wStyles,
        **                      HFILE hFile,
        **                      LPSTR lpFileName);
        **
        ** In ATM backdoor since version 2.6
        **
        ** Given a font name and style for a Multiple Master font instance,
        ** ATMMakePFM constructs a .PFM file with blended metrics.
        **
        ** If hFile == HFILE_ERROR, the function creates a temporary output file
        ** whose path is returned in *lpFileName.
        ** In ATM 2.6 and 3.x, the file is created in the directory specified
        ** by ATM.INI, [Settings], TmpDir=. In ATM 4.0 and later, the system
        ** call, GetTempFileName, is used.
		** Beginning with ATM 4.0 for NT, the filename is returned on 
		** ATM_CREATEFILE as well as on ATM_NOERROR.
        **
        ** If hFile != HFILE_ERROR it should be the handle of a file opened by
        ** the caller for writing. In this case, lpFileName is not used.
        ** Note: For 32-bit callers, the hFile parameter must be HFILE_ERROR.
        ** 
        ** The caller is responsible for closing or deleting the output file
        ** after a successful return.
        **
        ** Failure return values include:
        **      ATM_MEMORY
        **      ATM_USEFILE
        **      ATM_CREATEFILE
        **      ATM_NOTFOUND
        **      ATM_SYSTEM
        **      ATM_DBERROR
        **      ATM_MMMVERSION
        **      ATM_NOMM
        */


// backdoor API
#define ATMMakePSS(lpFaceName, wStyles, hFile, lpFileName) \
    libATMMakePSS(BD_VERSION, lpFaceName, wStyles, hFile, lpFileName)

// libATMMakePSS is for internal use only.  Applications should
// call ATMMakePSS.
extern int ATMAPI libATMMakePSS (
                                WORD  wBdVersion,
                                LPSTR lpFaceName,
                                WORD  wStyles,
                                HFILE hFile,
                                LPSTR lpFileName);                               
        /* 
        ** extern int ATMAPI ATMMakePSS (
        **                      LPSTR lpFaceName,
        **                      WORD  wStyles,
        **                      HFILE hFile,
        **                      LPSTR lpFileName);                               
        **
        ** In ATM backdoor since version 2.6
        **
        ** Note: this function is not supported in WinNT (PSS files are not
        ** required on WinNT).
        **
        ** Given a font name and style for a Multiple Master font instance,
        ** ATMMakePSS constructs a .PSS file for the PS driver.
        **
        ** If hFile == HFILE_ERROR, the function creates a temporary output file
        ** whose path is returned in *lpFileName.
        ** In ATM 2.6 and 3.x, the file is created in the directory specified
        ** by ATM.INI, [Settings], TmpDir=. In ATM 4.0 and later, the system
        ** call, GetTempFileName, is used.
		** Beginning with ATM 4.0 for NT, the filename is returned on 
		** ATM_CREATEFILE as well as on ATM_NOERROR.
        **
        ** If hFile != HFILE_ERROR it should be the handle of a file opened by
        ** the caller for writing. In this case, lpFileName is not used.
        ** Note: For 32-bit callers, the hFile parameter must be HFILE_ERROR.
        ** 
        ** The caller is responsible for closing or deleting the output file
        ** after a successful return.
        **
        ** Failure return values include:
        **      ATM_RESOURCE
        **      ATM_USEFILE
        **      ATM_SYSTEM
        **      ATM_CREATEFILE
        **      ATM_MEMORY
        */


extern int ATMAPI ATMSelectObject (
                                HDC      hDC,
                                HANDLE   hNewObject,
                                WORD     options,
                                LPHANDLE lphOldObject);
        /* 
        ** In ATM backdoor since version 2.01
        **
        ** Note: this function is not supported in WinNT.
        **
        ** The handle being selected into the DC is passed in 'hNewObject'.
        ** The handle to the old object is returned in 'lphOldObject'.  If
        ** this parameter is NULL, the old object is not returned.
        **
        ** Successful return values are:
        **      ATM_NEWFONTSELECTED     hNewObject selected for the 1st time.
        **      ATM_OLDFONTSELECTED     hNewObject had previously been selected
        **                              into the same DC.  'options' will NOT
        **                              have any effect in this case.
        **      ATM_FOREIGNFONTSELECTED hNewObject represents a non-ATM font
        **                              (such as when ATM_DEFER flag is given)
        **
        **   NOTE that versions 2.01 and 2.02 of ATM had another successful
        **        return value for this routine: ATM_NONFONTSELECTED.  This
        **        is no longer supported.
        **
        ** Failure return values are:
        **      ATM_NOTRUNNING          ATM is not running.
        **      ATM_BADPARM             Conflicting 'options' flags.
        **      ATM_SELECTERROR         'hDC' or 'hNewObject' is bad/invalid.
        */
                              

extern WORD ATMAPI ATMSetFlags (
                               WORD flags,
                               WORD mask);
        /* 
        ** In ATM backdoor since version 2.6
        **
        ** Note: this function is supported in WinNT, but all bits are reserved
        ** for future use. (None of the Win95 bits are supported.)
        **
        ** This routine is used to modify some of ATM's internal operating 
        ** flags. The mask parameter specifies which flags are to be changed.
        ** The flags parameter specifies the new values (on or off) for the
        ** flags specified by the mask parameter.
        ** 
        ** The return value indicates the former state of the specified flags.
        ** Beginning with ATM 4.0, a mask value of zero returns the value of
        ** all flags (without changing any of them).
        */


extern BOOL ATMAPI ATMXYShowText (
                                HDC              hDC,
                                int              x,
                                int              y,
                                WORD             wOptions,
                                LPRECT           lpRect,
                                LPSTR            lpString,
                                int              nCount,
                                LPATMFixedPoint  lpPoints,
                                LPATMFixedMatrix lpMatrix);
        /* 
        ** In ATM backdoor since version 1.0
        **
        ** Note: this function is not supported in WinNT.
        **
        ** Renders text given an arbitrary transformation matrix (lpMatrix)
        ** and specific widths (lpPoints) for precise positioning of each
        ** character in the string.  All but the last two parameters
        ** correspond to and are interpreted the same as the parameters of
        ** GDI's ExtTextOut with the following exceptions:
        **
        **    1) the wOptions parameter may include additional ATM-specific
        **       values ATM_USELOGCOLOR, ATM_USEPURECOLOR, or ATM_MAKEVISIBLE.
        **    2) Opaquing text is not supported.  That is, if wOptions includes
        **       the ETO_OPAQUE option, it will be ignored.
        **
        ** Returns TRUE if the call is successful, FALSE otherwise.
        **
        ** lpPoints is expressed in DEVICE units.  If NULL, default character
        ** widths are used.
        ** lpMatrix is expressed in DEVICE units.  If NULL, the font is used
        ** unmodified.
        **
        ** NOTE: x and y are the coordinates of the upper left corner of the
        **       text box without taking the matrix into consideration.  In
        **       other words, x,y defines the origin of a normal TextOut or
        **       ExtTextOut call which would use the font unmodified (i.e.
        **       with default matrix).  This means that the baseline position
        **       is determined relative to the unmodified font.  Also note
        **       that x and y are expressed in LOGICAL units (just as the x
        **       and y parameters of the TextOut and ExtTextOut routines).
        **       However, the character widths passed in 'lpPoints' are
        **       expressed in DEVICE units and are not subject to the current
        **       mapping mode in effect for 'hDC'.  This means that for
        **       devices that are capable of rendering graphics at lower than
        **       normal resolutions (such as the PCL driver's ability to
        **       switch to 150 or 75 dpi), these widths are expressed at the
        **       graphics resolution in effect.
        **       lpRect is expressed in LOGICAL units.
        */


extern BOOL ATMAPI ATMBBoxBaseXYShowText (
                                HDC              hDC,
                                int              x,
                                int              y,
                                WORD             wOptions,
                                LPRECT           lpRect,
                                LPSTR            lpString,
                                int              nCount,
                                LPATMFixedPoint  lpPoints,
                                LPATMFixedMatrix lpMatrix,
                                BOOL             bDoOutput,
                                LPATMBBox        lpBBox,
                                LPATMFixedPoint  lpDelta);
        /* 
        ** In ATM backdoor since version 1.15
        **
        ** Note: this function is not supported in WinNT.
        **
        ** This routine is quite similar to the ATMXYShowText routine with
        ** some slight differences.  First, x and y specify the coordinates
        ** of the start of the baseline (rather than the upper left corner).
        ** Similar to ATMXYShowText, x and y are interpreted as LOGICAL units,
        ** lpRect is expressed in LOGICAL units, and lpPoints are given in
        ** DEVICE units.
        ** If bDoOutput is TRUE then actual output is produced in the same
        ** manner as ATMXYShowText.
        ** lpBBox will receive the bounding box (or extent), in LOGICAL units,
        ** of the string.
        ** lpDelta will receive the deltas that should be applied to x,y in
        ** order to make a following call to this routine continue at the
        ** correct point on the baseline.  lpDelta is given in DEVICE units.
        **
        ** Returns TRUE if the call is successful, FALSE otherwise.
        **
        ** lpBBox is expressed in LOGICAL units.  If NULL, bbox is not
        ** returned.
        ** lpDelta is expressed in DEVICE units.  If NULL, delta is not
        ** returned.
        */

#endif /* ATMBD_IMPL */

#ifdef __cplusplus
}
#endif

#endif /* _H_ATM */
