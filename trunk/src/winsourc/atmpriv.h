/* $Header:   M:\winatm\lib32\atmpriv.h_v   1.5   01 Mar 1996 10:25:28   DARNOLD  $ */

/*
** ATMPRIV.h -- ATM interface, private to Adobe applications
**
** Version 3.02 -- 09-24-95
** Adobe Type Manager is a trademark of Adobe Systems Incorporated.
** Copyright 1983-1994 Adobe Systems Incorporated
** All Rights Reserved

The Adobe parts of this file are publicly available at
http://partners.adobe.com/public/developer/ps/sdk/index_archive.html#atmapi
(as of September 2007).

The Y&Y-specific parts are hereby placed in the public domain.
*/

#ifndef _H_ATMPRIV
#define _H_ATMPRIV

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ATMAPI
  #define ATMAPI       __stdcall
  #define ATMCALLBACK  __stdcall
#endif

/* Define return values for non-BOOL functions. */
#define ATM_NOWIDTHS            (-411) // Widths field in lpSubstInfo=NULL
#define ATM_NOKEY               (-412) // First char of PSName = NULL
#define ATM_REALFONTINSTALLED   (-413) // A font with the requested PS name is 
                                       // already installed.
#define ATM_DBMETRICSUSED       (-414) // The database metrics of the requested
                                       // PS name were used.
#define ATM_CREATEFILE          (-415) // Couldn't create file.
#define ATM_USEFILE             (-416) // Couldn't read, write, or close file.
#define ATM_ADDDELAYED          (-417) // Add incomplete, pending file access.

// return values for font database
#define ATM_BADPSNAME           (-2100) // Invalid PostScript font name.
#define ATM_NODATABASE          (-2101) // No database was found at ATM startup,
                                        // or ATM was unable to initialize the
                                        // database.
#define ATM_DBERROR             (-2102) // Database error, typically a corrupt or
                                        // malformed database. 

// return values for PostScript stub and .pfm generation
#define ATM_NOMM                (-2200) // Font is not MM instance or substitute.
#define ATM_MMMVERSION          (-2201) // Old version of MMM file
#define ATM_RESOURCE            (-2202) // Resource error.

// return values for ATMClient
#define ATM_NOTPRIVATE          (-2300) // System ATM is running.
#define ATM_NOTREGISTERED       (-2301) // Not registered being unregistered.

/*
 *  Mask constants used to access the ATMInstanceInfo.flags field below
 *  (from dbinterf.h)
 */

#define    IS_FIXED_WIDTH      0x00000001  // Is it fixed pitch?
#define    IS_SERIF            0x00000002  // Is it a serif font?
#define    IS_PI               0x00000004  // Is it a PI font?   
#define    IS_SCRIPT           0x00000008  // Is it a script font?   
#define    IS_ADOBE_FONT       0x00000010  // Is it from Adobe?   
#define    IS_STD_ENCODING     0x00000020  // Does it use Standard Encoding?
#define    IS_ITALIC           0x00000040  // Is it an italic font?    
#define    HAS_ALT_NAME        0x00000080  // Does it have an alternative name?
#define    CATEGORY_MASK       0x00000700  // Mask for Windows family     
#define    IS_MULTIPLE_MASTER  0x00001000  // Is it a Multiple Master font?   
#define    IS_MM_COMPONENT     0x00002000  
#define    IS_WINDOWS_BOLD     0x00004000  // Is it a bold font?
#define    IS_TRUE_TYPE        0x00008000  // Is it a TrueType font?
#define    IS_ALL_CAP          0x00010000  // Is it an all caps font?
#define    IS_SMALL_CAP        0x00020000  // Is it a small caps font?
#define    IS_FORCE_BOLD       0x00040000
#define    IS_VISIBLE          0x00080000


/*
 * More flags for ATMInstanceInfo.flags field.
 * The following five constants are used in conjunction with
 * CATEGORY_MASK above. They indicate the PitchAndFamily family values.
 */
 
#define     IS_ROMAN_TYPE       0x00000100
#define     IS_SWISS_TYPE       0x00000200
#define     IS_MODERN_TYPE      0x00000300
#define     IS_SCRIPT_TYPE      0x00000400
#define     IS_DECORATIVE_TYPE  0x00000500


/* Define bits for various flags supported by ATMSetFlags. */
#define ATMDOWNLOAD        0x0001
#define ATMUSEDEVFONTS     0x0002
#define ATMSUBSTITUTE      0x0004
#define ATMANTIALIAS       0x0008
#define ATMAUTOACTIVATE    0x0010
#define ATMGDIFONTS        0x0020


/* Flags for ATMGetFontInfo */
#define ATM_GETWIDTHS      0x0001
#define ATM_GETKERNPAIRS   0x0002


/* Flags for ATMClient */
#define ATM_REGISTER       0x0001
#define ATM_UNREGISTER     0x0002


/* Glyph pointer values for ATMEncodingInfo.szGlyphNames when no glyph name
** needs to be specified.
*/
#define ATM_PREVENCODING   (0)      // don't change current glyph
#define ATM_NOTENCODED     (1)      // don't assign any glyph
#define ATM_NORMALENCODING (2)      // use font's orig. encoding
                                    //  (not yet implemented)

typedef struct {

    WORD        char_1;     /* First character in the kern pair */
    WORD        char_2;     /* Second character in kern pair    */
    short       distance;   /* Kern dist; <0 tighter, >0 looser */

} ATMKernPair, *LPATMKernPair;

/*
** Note: All values are in EM units, where EM height is 1000.
*/


typedef struct {
    DWORD       flags;                /* Use IS_SCRIPT, etc. to get flags */
    WORD        num_chars;            /* Number of characters in font     */
    WORD        num_kern_pairs;       /* Number of kerning pairs in font  */

    short       font_bb_left;         /* Font bounding box - left     */
    short       font_bb_top;          /* Font bounding box - top      */
    short       font_bb_right;        /* Font bounding box - right    */
    short       font_bb_bottom;       /* Font bounding box - bottom   */
    short       missing_width;        /* Width of missing character   */
                                      // width of .notdef when default_char is
                                      // not encoded
    WORD        default_char;         /* index for missing characters */
                                      // may be unencoded, in which case
                                      // .notdef is used
    short       break_char;           /* index for break character    */
                                      /* default and break are relative to 0 */
    WORD        encoding;             /* Character encoding index     */
    
    short       stem_v;               /* Vertical stem width          */
    short       stem_h;               /* Horizontal stem width        */
    short       cap_height;           /* Capital height           */
    short       x_height;             /* X height             */
    short       figure_height;        /* Digit height             */
    short       ascent;               /* Max ascender height          */
    short       descent;              /* Max descender depth (usually neg.) */
    short       leading;              /* Additional leading between lines */
    short       max_width;            /* Maximum character width      */
    short       avg_width;            /* Average character width      */ 
    short       italic_angle;         /* Italic angle in deg ccw from vert */
    short       superior_baseline;    /* SuperiorBaseline */ 
    short       underline_position;   /* UnderlinePosition as found in AFM*/ 
    short       underline_thickness;  /* UnderlineThickness as found in AFM*/ 

    short       StrikeOutOffset;      /* strike-out location relative to 
                                      ** baseline 
                                      */
    short       StrikeOutThickness;   /* strike-out thickness */

    WORD        widthsOffset;         /* Offset of widths for characters    */
    WORD        kern_pairsOffset;     /* Offset of sorted list of kern pairs*/
} ATMInstanceInfo, *LPATMInstanceInfo;


typedef struct {
   char              PSName[ATM_PSNAMESIZE]; /* PostScript name (key) */
   char              mmFont[LF_FACESIZE];    /* what MM face to use */
   WORD              mmStyles;               /* what MM style to use */
   LPATMInstanceInfo lpInstInfo;             /* font metrics & info */
   } ATMSubstRequest, *LPATMSubstRequest;

typedef struct {
   char              substName[LF_FACESIZE]; /* Name to use with CreateFont */
   WORD              substStyles;            /* if ATM_ITALIC, set lfItalic */
   char              mmName[LF_FACESIZE];    /* MM face being used */
   WORD              mmStyles;               /* style of MM face */
   } ATMSubstResult, *LPATMSubstResult;

typedef struct {
   short             needInit;               /* nonzero if this is the first
                                             ** time this structure is used
                                             ** or, it is being used for 
                                             ** a new font, or, the glyph
                                             ** names have changed.
                                             */
   WORD              defaultChar;            /* new default character */
                                             // defaultChar is relative to zero
                                             // may be unencoded, in which case
                                             // .notdef is used
   WORD              breakChar;              /* new break character */
                                             // breakChar is relative to zero
   WORD              atmReserved1;           
   WORD              atmBlock[3*256];
   DWORD             offGlyphNames;          /* Offset, relative to start of 
                                             ** ATMEncodingInfo, to an array of
                                             ** 256 DWORD offsets (each relative
                                             ** to ATMEncodingInfo), to null-
                                             ** terminated glyphNames for                                             ** 
                                             ** codepoints.
                                             ** A glyphName offset of ATM_NOTDEF
                                             ** specifies an unencoded codepoint.
                                             ** ATM_NOREENCODE specifies the
                                             ** current encoding for the 
                                             ** codepoint.
                                             */
   DWORD             offCharWidths;          /* Offset, relative to start of
                                             ** ATMEncodingInfo, to an array of
                                             ** 256 WORD new character widths;
                                             */
   } ATMEncodingInfo, *LPATMEncodingInfo;

typedef BOOL (ATMCALLBACK *LPATMEnumDBProc)(
                   LPATMFontSpec lpFontSpec,
                   DWORD         refNum,
                   DWORD         flags,
                   DWORD         userData);

typedef int (ATMCALLBACK *LPATMGetGlyphProc) (
                               LPSTR glyphName, 
                               short currCodepoint,
                               short normalCodepoint,
                               short fontCodepoint,
                               DWORD userData);

#ifndef ATMBD_IMPL

/* 
** NOTE: The following functions all return ATM_NOERR to indicate success.
** In addition to the specific values noted with each function,
** these functions can all return the following error values:
**
**      ATM_ERROR
**      ATM_CANTHAPPEN
**      ATM_NOTSUPPORTED
**      ATM_NOTRUNNING
**      ATM_BADPARM
**      ATM_BADSTYLE
*/


extern int ATMAPI ATMClient (DWORD flags);
        /*
        ** In ATM backdoor since versions 3.01
        **
        ** Register/unregister a client of private ATM.
        ** A user of this routine MUST register itself BEFORE initializing
        ** the backdoor library by a call to ATMProperlyLoaded.  When ATM
        ** is running in private mode, a call to ATMProperlyLoaded will fail
        ** if the caller is not registered.
        **
        ** Return values are:
        **     ATM_NOERR
        **     ATM_NOTSUPPORTED
        **     ATM_NOTRUNNING
        **     ATM_NOTREGISTERED
        **     ATM_NOTPRIVATE
        **     ATM_BADPARM
        **     ATM_ERROR
        */


// backdoor API
#define ATMEnumDBFonts(flags, enumProc, userData) \
    libATMEnumDBFonts(BD_VERSION, flags, enumProc, userData)

// for internal use only
extern int ATMAPI libATMEnumDBFonts (
                                WORD              wBdVersion,
                                DWORD             flags,
                                LPATMEnumDBProc   enumProc,
                                DWORD             userData);
        /* 
        ** extern int ATMAPI ATMEnumDBFonts (
        **                      DWORD             flags,
        **                      LPATMEnumDBProc   enumProc,
        **                      DWORD             userData);
        **
        ** In ATM backdoor since version 4.0
        ** 
        */

// backdoor API
void ATMAPI ATMGetBuildStr (LPSTR bldStr);
        /* 
        **	Return the build string in bldStr
        */


// backdoor API
#define ATMGetDBFont(lpFont, lpdwRefNum) \
    libATMGetDBFont(BD_VERSION, lpFont, lpdwRefNum)

// for internal use only
extern int ATMAPI libATMGetDBFont (
                                WORD              wBdVersion,
                                LPATMFontSpec     lpFont,
                                LPDWORD           lpdwRefNum);
        /* 
        ** extern int ATMAPI ATMGetDBFont (
        **                      LPATMFontSpec     lpFont,
        **                      LPDWORD           lpdwRefNum);
        **
        ** In ATM backdoor since version 4.0
        **
        ** returns:  ATM_NOERR
        **           ATM_NOTFOUND
        **           ATM_CANTHAPPEN
        **           ATM_BADPARM
        **           ATM_NOTSUPPORTED
        */

#ifdef DATABASE_ON
// backdoor API
#define ATMGetDBFontInfo(lpInfo, dwRefNum) \
    libATMGetDBFontInfo(BD_VERSION, lpInfo, dwRefNum)

// for internal use only
extern int ATMAPI libATMGetDBFontInfo (
                                WORD              wBdVersion,
                                LPDBFontInfo      lpInfo,
                                DWORD             dwRefNum);
        /* 
        ** extern int ATMAPI ATMGetDBFontInfo (
        **                      LPDBFontInfo      lpInfo,
        **                      DWORD             dwRefNum);
        **
        ** In ATM backdoor since version 4.0
        **
        ** NOTE: Must include dbinterf.h and dbglue.h for LPDBFontInfo
        **       definition. 
        **
        ** returns:  ATM_NOERR
        **           ATM_DBERROR = not found, bad refnum, bad file
        **           ATM_CANTHAPPEN
        **           ATM_BADPARM
        **           ATM_NOTSUPPORTED
        */
#endif // DATABASE_ON

// backdoor API
#define ATMGetDBMenuName(lpPostScriptName, lpFont, lpdwRefNum) \
    libATMGetDBMenuName(BD_VERSION, lpszPostScriptName, lpFont, lpdwRefNum)

// for internal use only
extern int ATMAPI libATMGetDBMenuName (
                                WORD              wBdVersion,
                                LPSTR             lpszPostScriptName,
                                LPATMFontSpec     lpFont,
                                LPDWORD           lpdwRefNum);
        /* 
        ** extern int ATMAPI ATMGetDBMenuName (
        **                      LPSTR             lpszPostScriptName,
        **                      LPATMFontSpec     lpFont,
        **                      LPDWORD           lpdwRefNum);
        **
        ** In ATM backdoor since version 4.0
        **
        ** returns:  ATM_NOERR
        **           ATM_NODATABASE
        **           ATM_BADPSNAME
        **           ATM_NOTFOUND
        **           ATM_DBERROR
        **           ATM_BADPARM
        **           ATM_NOTSUPPORTED
        */



// backdoor API
#define ATMGetDBMMCompFonts(dwRefNum, lpdwRefNum) \
    libATMGetDBMMCompFonts(BD_VERSION, dwRefNum, lpdwRefNum)

// for internal use only
extern int ATMAPI libATMGetDBMMCompFonts (
                                WORD              wBdVersion,
                                DWORD             dwRefNum,
                                LPDWORD           lpdwRefNum);
        /* 
        ** extern int ATMAPI ATMGetDBMMCompFonts (
        **                      DWORD             dwRefNum,
        **                      LPDWORD           lpdwRefNum);
        **
        ** In ATM backdoor since version 4.0
        **
        ** returns:  ATM_NOERR
        **           ATM_DBERROR = not found, bad refnum, bad file, not MM
        **           ATM_CANTHAPPEN
        **           ATM_BADPARM
        **           ATM_NOTSUPPORTED
        */


// backdoor API
#define ATMGetDBNameTable(lplpNameTable, lpwNumGlyphs, lpdwTableSize) \
    libATMGetDBNameTable(BD_VERSION, lplpNameTable, lpwNumGlyphs, lpdwTableSize)

// for internal use only
extern int ATMAPI libATMGetDBNameTable (
                                WORD              wBdVersion,
                                LPSTR FAR *       lplpNameTable,
                                LPWORD            lpwNumGlyphs,
                                LPDWORD           lpdwTableSize);
        /* 
        ** extern int ATMAPI ATMGetDBNameTable (
                                LPLPSTR           lplpNameTable,
                                LPWORD            lpwNumGlyphs,
                                LPDWORD           lpdwTableSize);
        **
        ** In ATM backdoor since version 4.0
        **
        ** returns:  ATM_NOERR
        **           ATM_DBERROR = not found, bad refnum, bad file
        **           ATM_CANTHAPPEN
        **           ATM_BADPARM
        **           ATM_NOTSUPPORTED
        */


// backdoor API
#define ATMGetFontInfo(hDC, flags, lpFaceName, lpInstInfo, lpBufSize) \
    libATMGetFontInfo(BD_VERSION, hDC, flags, lpFaceName, lpInstInfo, \
                       lpBufSize)

// for internal use only
extern int ATMAPI libATMGetFontInfo (
                                WORD              wBdVersion,
                                HDC               hDC,
                                WORD              flags,
                                LPSTR             lpFaceName,
                                LPATMInstanceInfo lpInstInfo,
                                LPWORD            lpBufSize);
        /* 
        ** extern int ATMAPI ATMGetFontInfo (
        **                      HDC               hDC,
        **                      WORD              flags,
        **                      LPSTR             lpFaceName,
        **                      LPATMInstanceInfo lpInstInfo,
        **                      LPWORD            lpBufSize);
        ** 
        ** In ATM backdoor since version 2.6
        ** 
        ** This routine returns information about the font in the hDC.
        ** Information will be returned for Type 1 fonts, Multiple Master
        ** instances, database substitute fonts, and application-installed
        ** substitute fonts. The call will fail with an ATM_FOREIGNFONTSELECTED
        ** return value if a non-ATM font is selected into the hDC.
        **
        ** The routine must be called twice.  On the first call, NULL is
        ** passed for the lpInstInfo parameter.  If the call succeeds, the
        ** required size of the ATMInstanceInfo structure will be returned in
        ** lpBufSize.  This size includes space for the character widths and
        ** kern pairs, if widths or pairs are requested.
        **
        ** On the second call, lpInstInfo will point to a buffer of the size
        ** retunred in lpBufSize on the first call to this function.  On the
        ** second call, the lpFaceName parameter should point to a character
        ** buffer at least LF_FACESIZE bytes in length.
        **
        ** The flags parameter is used to specify whether character widths
        ** or kern pairs are requested, using the #defines ATM_GETWIDTHS
        ** and ATM_GETKERNPAIRS. 
        ** 
        ** Failure return values are:
        **     ATM_FOREIGNFONTSELECTED
        **     ATM_MEMORY             
        **     ATM_DBERROR            
        **     ATM_NOMETRICSFILE      
        **     ATM_NOFONTFILE         
        */


// backdoor API
#define ATMGetGlyphList(hdc, lpProc, userData) \
    libATMGetGlyphList(BD_VERSION, hdc, lpProc, userData)

// for internal use only
extern int ATMAPI libATMGetGlyphList (
                                WORD              wBdVersion,
                                HDC               hdc,
                                LPATMGetGlyphProc lpProc,
                                DWORD             userData);
        /* 
        ** extern int ATMAPI ATMGetGlyphList (
        **                      HDC               hdc,
        **                      LPATMGetGlyphProc lpProc,
        **                      DWORD             userData);
        ** 
        ** In ATM backdoor since version 2.6
        ** 
        ** This procedure enumerates all of the glyphs that are in the
        ** font selected into the hdc. lpProc is called once for each
        ** codepoint (character value) in the current encoding, and
        ** is then called once for each glyph in the font that is
        ** not in the current encoding. lpProc is called with the 
        ** following arguments:
        **     glyphName   a string containing the name of the glyph
        **                 for example, "A", or "comma"
        **     current codepoint
        **                 what character value currently corresponds to the 
        **                 glyph
        **     normal codepoint
        **                 what character value corresponds to the glyph
        **                 in the font's normal encoding, i.e., when the font
        **                 is NOT reencoded using ATMSelectEncoding 
        **     font encoding
        **                 if the font uses Adobe StandardEncoding, 
        **                 this value will be -1. Otherwise it will be the
        **                 current codepoint.
        **     userData
        **                 the argument passed to ATMGetGlyphList.
        **
        ** If the lpProc procedure returns 0, the enumeration process will stop.
        **
        ** Failure return values include:
        **     ATM_FOREIGNFONTSELECTED
        **     ATM_MEMORY
        **     ATM_CANCELLED
        */


// backdoor API
#define ATMInstallSubstFont(lpRequest, lpResult) \
    libATMInstallSubstFont(BD_VERSION, lpRequest, lpResult)

// for internal use only
extern int ATMAPI libATMInstallSubstFont (
                                WORD              wBdVersion,
                                LPATMSubstRequest lpRequest,
                                LPATMSubstResult  lpResult);
        /* 
        ** extern int ATMAPI ATMInstallSubstFont (
        **                      LPATMSubstRequest lpRequest,
        **                      LPATMSubstResult  lpResult);
        ** 
        ** In ATM backdoor since version 2.6
        ** 
        ** This routine passes font metrics to ATM, and requests that ATM
        ** use these metrics to create a synthetic, or substitute, font.
        ** In addition to metrics, a specific multiple master font may 
        ** be requested to be used for performing the substitution. The
        ** caller must specify a PostScript name in the request. This name
        ** must also be used in the ATMRemoveSubstFont call.
        **
        ** The font is created with the encoding correspoding to the
        ** Windows' ANSI_CHARSET. If the application is going to 
        ** re-encode this font, it may pass all zeroes for the character
        ** widths, and then specify the re-encoded widths in the 
        ** ATMSelectEncoding call.
        ** 
        ** ATM returns to the caller a font name and styles information which
        ** the caller must use in the Windows CreateFont or CreateFontIndirect 
        ** calls. It also returns the name and style of the Multiple Master
        ** font that is actually used for the substitution.
        **
        ** If the caller's requested PostScript name is installed as a Type 1
        ** font, the call to ATMInstallSubstFont will be ignored. If the metrics
        ** of the caller's requested PostScript name are found in the font
        ** database, the caller's metrics will be ignored.
        **
        ** Successful return values are:
        **     ATM_NOERR       
        **     ATM_ALREADYINSTALLED
        **     ATM_REALFONTINSTALLED
        **     ATM_DBMETRICSUSED
        **
        ** Failure return values include:
        **     ATM_NOWIDTHS
        **     ATM_NOKEY
        */


// backdoor API
#define ATMMakePFM(lpFaceName, wStyles, hFile, lpFileName) \
    libATMMakePFM(BD_VERSION, lpFaceName, wStyles, hFile, lpFileName)

// for internal use only
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
        ** TODO: describe interface here
        ** Failure return values include:
        **      ATM_MEMORY
        **      ATM_USEFILE
        **      ATM_CREATEFILE
        **      ATM_NOTFOUND
        **      ATM_NOMMI
        **      ATM_SYSTEM
        **      ATM_DBERROR
        **      ATM_MMMVERSION
        **      ATM_NOMM
        */


// backdoor API
#define ATMMakePSS(lpFaceName, wStyles, hFile, lpFileName) \
    libATMMakePSS(BD_VERSION, lpFaceName, wStyles, hFile, lpFileName)

// for internal use only
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
        ** TODO: describe interface here
        **
        ** Failure return values include:
        **      ATM_RESOURCE
        **      ATM_USEFILE
        **      ATM_SYSTEM
        **      ATM_CREATEFILE
        **      ATM_MEMORY
        */


// backdoor API
#define ATMMarkDC(hdc, lpIn, lpOut) \
    libATMMarkDC(BD_VERSION, hdc, lpIn, lpOut)

// for internal use only
extern int ATMAPI libATMMarkDC ( WORD   bdVersion,
                                     HDC    hDC,
                                     LPWORD lpIn,
                                     LPWORD lpOut );
        /*
        ** In ATM backdoor since version 3.01
        **
        ** Inserts a special marker in the output stream going to hDC
        ** in order to identify the client's job as one that should be
        ** serviced by ATM in metafile situations. It also specifies that
        ** ATM should not defer to device fonts subsequently realized for
        ** this DC.
        ** lpIn and lpOut are for future use and may be NULL for 3.01.
        ** This function requires that at least one ATM font be installed.
        **
        ** Return values are:
        **     ATM_NOERR
        **     ATM_NOTSUPPORTED
        **     ATM_NOTRUNNING
        **     ATM_ERROR
        **     ATM_NOTFOUND         (no ATM fonts are installed)
        */

        
// backdoor API
#define ATMRemoveSubstFont(lpPSName) \
    libATMRemoveSubstFont(BD_VERSION, lpPSName)

// for internal use only
extern int ATMAPI libATMRemoveSubstFont (
                                WORD  wBdVersion,
                                LPSTR lpPSName);
        /* 
        ** extern int ATMAPI ATMRemoveSubstFont (
        **                      LPSTR lpPSName);
        ** 
        ** In ATM backdoor since version 2.6
        ** 
        ** This routine removes a previously installed substitute font.
        ** lpPSName points to the PSName field of the the ATMSubstRequest
        ** structure which was passed to ATMInstallSubstFont().
        **
        ** Failure return values include:
        **     ATM_BADPSNAME  -- if lpPSName doesn't specify a previously
        **                       installed substitute font
        */

// for backdoor version control
#define ATMSelectEncoding(hdc, lpEncodingInfo) \
    libATMSelectEncoding(BD_VERSION, hdc, lpEncodingInfo)

// for internal use only
extern int ATMAPI libATMSelectEncoding (
                                WORD              wBdVersion,
                                HDC               hdc,
                                LPATMEncodingInfo lpEncodingInfo);
        /* 
        ** In ATM backdoor since version 2.6
        ** 
        ** Specifies a new encoding vector for the currently selected font.
        **
        ** Optionally, the caller can specify the character widths 
        ** correspoding to the new encoding. 
        **
        ** The lpEncodingInfo contains atmBlock, a large array of words,
        ** which will be used by ATM to cache the processed re-encoding
        ** information. The first time ATMSelectEncoding is called for
        ** a font, lpEncodingInfo->needInit should be set to TRUE.
        **
        ** Ideally, an application will keep one ATMEncodingInfo structure
        ** for each re-encoded font it is using. In this context, a font
        ** refers to a specific face name and style. The same ATMEncodingInfo
        ** structure can be used for different point sizes and matrices.
        **
        ** If the width of a glyph in the encoding array is already known
        ** internally by ATM, the newly specified width will be ignored. This
        ** may occur when the font is in the font database, the given glyph
        ** belongs to the normal encoding, or the glyph was listed in a 
        ** previous call to ATMSelectEncoding.
        **
        ** If a glyph name is given as ATM_NOREENCODE, the corresponding code
        ** point will maintain its association with the glyph currently mapped
        ** to that codepoint.
        **
        ** If a glyph name is given as ATM_NOTDEF, the corresponding codepoint
        ** will indicate an unencoded character, for which the default 
        ** character will be substituted.
        **
        ** The defaultChar field in the ATMEncodingInfo structure is used to
        ** to establish a new default character for this font.
        **
        ** Failure return values include:
        **     ATM_NOWIDTHS
        **     ATM_MEMORY
        **     ATM_FOREIGNFONTSELECTED
        */


extern WORD ATMAPI ATMSetFlags (
                               WORD flags,
                               WORD mask);
        /* 
        ** In ATM backdoor since version 2.6
        **
        ** This routine is used to modify some of ATM's internal operating 
        ** flags. The mask parameter specifies which flags are to be changed.
        ** The flags parameter specifies the new values (on or off) for the
        ** flags specified by the mask parameter.
        ** 
        ** The return value indicates the former state of the specified flags.
        */


#endif /* ATMBD_IMPL */

/* ===== MultiByte Character Set support ===== */

extern int ATMAPI ATMGetOutline2 (
                                HDC              hDC,
                                WORD             c,
                                LPATMFixedMatrix lpMatrix,
                                FARPROC          lpProcMoveTo,
                                FARPROC          lpProcLineTo,
                                FARPROC          lpProcCurveTo,
                                FARPROC          lpProcClosePath,
                                LPSTR            lpData);
        /* 
        ** In ATM backdoor since version 2.51J, 3.01M
        **
        ** Reports the outline of the specified character.
        **
        ** This is a double byte version of ATMGetOutline. 
        */

extern int ATMAPI ATMGetCharacterByteLength (
                                HDC              hDC,
                                LPSTR            lpString,
                                int              nCount);
        /* 
        ** In ATM backdoor since version 3.01M
        **
        ** Return a byte length of the first character in lpString
        ** under the current font. It returns either 1,2,3, or 4.
        */

#define RANGEITEM unsigned long int
typedef struct {
  RANGEITEM RangeStart;
  RANGEITEM RangeEnd;
} MBCSRANGEREC;
typedef MBCSRANGEREC FAR * LPMBCSRANGEREC;

extern WORD ATMAPI ATMMBCSRanges ( LPMBCSRANGEREC RangeTab,
                                       WORD           SizeOfRangeTab );
        /* 
        ** In ATM backdoor since version 3.01M
        **
        ** 'SizeOfRangeTab' is an actual array size of 'RangeTab'.
        ** The function returns a required array size.
        ** 'SizeOfRangeTab=0' can be used to obtain the required size
        ** of 'RangeTab'.                                       
        ** It returns 0 when no range table is obtained.       
        ** In case the current font is a Roman font, the table always
        ** contains 1 record with 0 and 255 as the starting and
        ** ending values respectively. 
        */

#ifdef __cplusplus
}
#endif

#endif /* _H_ATMPRIV */


