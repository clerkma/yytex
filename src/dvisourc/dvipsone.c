/* Copyright 1990, 1991, 1992 Y&Y, Inc.
   Copyright 2007 TeX Users Group
   Copyright 2014, 2015, 2016 Clerk Ma

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


/**********************************************************************
*
* DVI to PS convertor for Adobe Type 1 (ATM compatible) fonts
* top level part, provides main and common routines
* calls upon DVILOG, DVIEXTRA, DVIANAL & DVISPECI, in that order
* see showusage for command line arguments
*
**********************************************************************/

#include "dvipsone.h"
#include <windows.h>

#define NEEDATMINI
#define CONTROLBREAK    /* handle control break */
#define USELOGFILE      /* allow writing of log file */

/* #define DEBUGFLUSH */  /* flush output buffers for debugging */

/* In the new way, DVISETUP modifies the EXE file directly */
/* newmagic is the old hexmagic string with 8 preliminary bytes added */
/* four are the author signature, four are the serial number * REEXEC */

/* now required by packdatetime for PDFmark 95/Feb/25 */

char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";  /* 1994/June/8 */

int dvidictsize = DVIDICT;  /* size of `dvidict' */

/* NOTE: the defaults for these must match those in changeflag calls ... */

int verboseflag  = 0; /* -v non-zero => lots of output */
int traceflag    = 0; /* -*t non-zero => even more verbose */
int showfontflag = 0; /* -V non-zero => show font tables */
int showsubflag  = 0; /* -S non-zero => even more verbose */
int detailflag   = 0; /* -? non-zero => show detail in usage */
int reverseflag  = 0; /* -r non-zero => do pages in reverse order */
int quietflag    = 0; /* -q non-zero => suppress \special complaints *//* -qqq also used to show hidden information */
int logfileflag  = 0; /* -n write log file 99/Apr/20 */
int copies=1;         /* -c number of copies to print */
int collated=0;       /* -C non-zero => collated copies */

int const statisticsflag = 1; /* non-zero => output stack depth, fonts used */
int const complainflag   = 0; /* non-zero implies complain sub table warnings */
int const timingflag     = 1; /* non-zero => show timing information */
int directprint          = 0; /* non-zero => output directly to printer */
int stripcomment         = 0; /* non-zero => strip out comments */

int bUseCounters=0;     /* -*4 non-zero counter[1]-counter[2] 96/Jan/28 */
int passcomments=1;     /* default changed 94/Mar/7 for Adobe Illustrator EPS */
int dontstrip=0;      /* non-zero => don't strip comments *when direct* */
int verbatimflag=0;     /* non-zero => allow verbatim PostScript */
int preservefont=1;     /* preserve font across verbatim */ 
int bSmartCopyFlag=1;   /* non-zero => look for %%EOF and such in EPSF */
int bKeepTermination=0;   /* keep line terminations intact in EPSF */
int bConvertReturn=1;   /* convert isolated \r to \n when copying EPS */
int bPassControls=0;    /* pass through control characters in EPSF */
/* int showpageinfo=0; */ /* non-zero => show VMUsage and rendering time */
int countzeroflag=1;    /* non-zero => page limits based on TeX /count0 */
int retplusnew=0;     /* non-zero => use `return' + `newline' output */
/* default changed 1996/Aug/26 for safe handling of binary data in EPS files */
/* int usetexbounds=1; */ /* use TeX DVI file specified page bounds */ 
int allowtpic=1;      /* non-zero => allow TPIC \specials */
int needtpic=0;       /* non-zero => include TPIC header file */
int textextwritten=0;   /* non-zero => tex_text encoding already written */
int ansiwritten=0;      /* non-zero => ansi encoding already written */
int bShortFont=1;     /* non-zero => use shorter font numbers for /f..*/
int bUseInternal=1;     /* non-zero => use internal number for /fn... */
              /* otherwise use sequential number assigned here */
int bRemapControl=0;    /* 0 - 9 => 161 - 170, 10 - 32 => 173 -- 195 */
int bRemapSpace=0;      /* 32 => 195, 13 => 176, 0 => 161 95/Oct/17 */
int bRemapFont=0;     /* remap just this one font ... unfinished */

int bPassEPSF=1;      /* non-zero => pass through EPSF files */
int bIgnoreSpecials=0;    /* non-zero => ignore all \specials */
int bOldDVIPSFlag=0;    /* wrap ps: code --- unlike ps:: 96/Oct/8 *//* default change - increase DVIPS compatability */
int bOldDVIPSFlip=0;    /* flip coordinates in ps: code 96/Nov/7 *//* now handled instead via `neg rotate' */
int bQuotePage=1;     /* suppress showpage in \special{"..."} */
int bProtectStack=1;    /* wrap PS in [ ... cleartomark 97/Nov/24 */
int bStoppedContext=1;    /* add { ... } stopped included PS 97/Nov/24 */
int bAllowInclude=1;    /* expand %%IncludeResource: font ... */
int bAllowBadBox=1;     /* allow single % comments before BoundingBox *//* now frozen at 1 1996/May/12 */
/* int sebastianflag=0; */    /* no gsave/grestore in postscript= */
int bWrapSpecial=1;     /* prevent long lines in \special */
int bTexFontsMap=1;     /* non-zero => allowed to use `texfonts.map'*//* use only in findopen - do not replace name */
int bForceFontsMap=0;   /* non-zero => replace aliases `texfonts.map'*//* use up front - replace name - overrides above */
int bSciWord=0;       /* non-zero => treat ps: filename case */
// int bMaxSub=0;       /* non-zero => reduce subtitution table size */
// int bMaxFont=0;        /* non-zero => reduce font table size */
int bStripBadEnd=1;     /* read to EOL at end of encrypted 96/Feb/22 */
int bPDFmarks=1;      /* write pdfmarks into PS file 96/Feb/25 */
int bBackGroundFlag=1;    /* support \special{background ...} 98/Jun/30 */
int bBackUsed=0;      /* non-zero of \special{background ...} used */
int BBxll, BByll, BBxur, BByur; /* crop box given in \special 96/May/4 */
int outscaleflag=0;     /* rescale scaled points on output 94/Sep/13 *//* used to work around Emerald RIP bugs */
int bMarkUnused=1;      /* mark unused fonts early on 95/Mar/5 */
/* int bMMNewFlag=1; */   /* treat MM fonts like others 95/May/13 fixed */
int bMMShortCut=0;      /* simple way construct base file name 95/May/25 *//* remove this option ? */
int bDVICopyReduce=1;   /* -2^31 => 0 sr from DVICOPY 1995/Sep/16 */
int bRandomPrefix=0;    /* add random prefix to file names 95/Sep/30 */
int bAddBaseName=1;     /* add BaseFontName for Distiller 97/Jan/30 */
int bFirstNull=1;     /* non-zero => flush \special starts with null */
int bGhostHackFlag=0;   /* make 1 byte adjustement for GhostScript */
int nMaxErrors=MAXERRORS; /* errors before giving up 95/Dec/28 */
int bSuppressBack=0;    /* suppress background in imagemask 95/Dec/31 */
int bDuplex=0;        /* ask for duplex printing new 96/May/6 */
int bTumble=0;        /* ask printer to tumble in duplex 96/Dec/20 */
// int bLevel2=0;     /* if allowed to use level 2 features 96/Dec/20 */
int bLevel2=1;        /* if allowed to use level 2 features 00/Jun/17 */
int bAllowCompression=1;  /* allow use of compression of images from TIFF & BMP *//* Runlenght for bi-level and Packbits, LZW otherwise */
int distillerlogflag=0;   // output (%%[ ... ]%%) = flush - optional 2000 Aug 15
int nDefaultTIFFDPI=72;   /* if resolution not specified in TIFF */
int bInsertImage=0;     /* if \special{insertimage: ... } seen dvipslog */
long postposition;      /* position of post op in DVI */
int badpathwarn=0;      /* warned yet about bad path in search ? */

int textures=0;       /* non-zero => textures style DVI file */
/* int flipflag=0; */   /* non-zero => flip horizontally */
int makeepsf=0;       /* non-zero => output intended as EPS file */

int beginflag=0;      /* non-zero => next arg is beginning page */
int endflag=0;        /* non-zero => next arg is ending page */

/* FILE *errout=stderr; */  /* where to send error output */
FILE *errout;   /* where to send error output change 97/Oct/28 */

/* int usehexflag=1;  */  /* non-zero => encrypted capsule in binary form */
/* int breakearly=0;  */  /* skip out font file interpretation early */
/* int wantchksum=0;  */  /* want chcksum output */

/* float CurrentA, CurrentB, CurrentC, CurrentD; */ /* current color state */

COLORSPEC CurrColor;

int colorindex = 0;   /* index into color stack 95/Mar/1 */

int clipstackindex = 0; /* clip push pop index 98/Sep/12 */

int colortypeflag = 0;  /* non-zero => using cmyk */

int bCarryColor = 1;  /* carry color across pages 98/Feb/14 */

int bColorUsed = 0;   /* on if \special{color ... } seen */

int wantmagictiff=1;    /* want to use our own screen function (TIFF) */

double outscale=1.0;    /* what to scale scaled points with 94/Sep/13 */

int frequency=-1, angle=45; /* frequency and angle of halftone screen */

int freqrelation=0;     /* +1 => maximum, -1 => minimum, 0 => force */

int nfonts, nsubstitute, nremapped, nansified;  /* counts from dviextra */

int volatile bAbort = 0;  /* set by user control-C */ /* 1992/Nov/24 */
              /* and ONLY that way */

int abortflag = 0;      /* new, used in DLL version */

long dvistart;        /* where DVI part of Textures file starts */

/* MAXREMAP can be 32 or 128 */
/* If it is 32, then code 32 does not get mapped to 195 and 127 not to 196 ? */

int remaptable[MAXREMAP];         /* 1994/June/20 */

/* The following three arrays must be `parallel' */

/* int npapertypes = 7 + 6; */    /* number of papertypes flush 94/May/6 */

/* Also now handle Letter, A4, A5 etc as landscape versions 94/July/1 */

/* added A2 and B3 1994/Dec/10 */

typedef struct {
  char * type;
  double width;
  double height;
} paper_size_t;

static paper_size_t paper_struct [] =
{
  {"letter",    8.5,      11},
  {"note",      8.5,      11},
  {"legal",     8.5,      14},
  {"folio",     8.5,      13},
  {"ledger",    11,       17},
  {"tabloid",   11,       17},
  {"executive", 7.25,     10.5},
  {"a2",        420/25.4, 594/25.4},
  {"a3",        297/25.4, 420/25.4},
  {"a4",        210/25.4, 297/25.4},
  {"a5",        148/25.4, 210/25.4},
  {"b3",        354/25.4, 500/25.4},
  {"b4",        250/25.4, 354/25.4},
  {"b5",        176/25.4, 250/25.4},
  {"quarto",    215/25.4, 275/25.4},
  {"",          1,        1}
};

/* `statement' 5 1/2 x 8 1/2 */

char *paper_type    = NULL; /* "letter" or "" ? from command line */
char *bounding_type = NULL; /* "letter" or "" ? 94/May/6 */
char *paper_size    = NULL; /* Custom paper size specified via special */
char *dsc_extra     = NULL; /* extra DSC comment to add in 94/May/6 */

/* compute later using command line page size argument */

double pageheight;      /* page height in bp */
double pagewidth;     /* page width in bp */

int currentdirect=1;  /* non-zero => place output current directory */

int magniflag =0;    /* next arg => user specified magnification */
int rotateflag=0;   /* next arg => user specified rotation */
int xoffsetflag=0;    /* next arg => user specified xoffset */
int yoffsetflag=0;    /* next arg => user specified yoffset */
int copiesflag=0;   /* next arg => user specified copies */
int pathflag=0;     /* next arg => outline font file path */
int subfileflag=0;    /* next arg => font substitution file */
int paperflag=0;    /* next arg => paper type */
int boundingflag=0;   /* next arg => paper size for bounding box */
int dscextraflag=0;   /* next arg => extra DSC comment to include */
int emeraldflag=0;    /* next arg => scale factor for output */
int versionflag=0;    /* next arg => bCheckVersion */
int repeatflag=0;   /* next arg => nRepeatCount */
int minruleflag=0;    /* next arg => nMinRule */
int defdpiflag=0;   /* next arg => nDefaultTIFFDPI */
int afmflag=0;      /* next arg => afm file path */
int tfmflag=0;      /* next arg => tfm file path */
int pfmflag=0;      /* next arg => pfm file path */
int epsflag=0;      /* next arg => eps file path */
int outputflag=0;   /* next arg => output file */
int prologflag=0;   /* next arg => prolog file */
int prefixflag=0;   /* next arg => FontName prefix */
int dvidictflag=0;    /* next arg => size of dvidict */
int vecpathflag = 0;  /* next arg => vec file path */
int subpathflag = 0;  /* next arg => sub file path */
int prepathflag = 0;  /* next arg => preamble file path */
int encodeflag = 0;   /* next arg => global encoding name 95/Aug/14 ? */
int halftoneflag = 0; /* next arg => halftone screen info */

int wantcontrold=0;   /* non-zero => want control D added at end */
int tryunderscore=1;  /* non-zero => try underscore form of font file name */

int bAllowStandard=1; /* allow use of StandardEncoding - if it works */
int bAllowTexText=1;  /* allow use of TeXtext encoding - if it works */
int bAllowANSI=1;   /* allow use of ANSI encoding - if it works */
int bAllowShortEncode=1;  /* allow name of Encoding instead of list */
int busedviencode=0;    /* (was default 1) use numeric reencoding fonts ! */
              /* frozen at 0 1996/May/12 */
int bSubRealName=1;     /* substitute file name for FontName in PFA */
              /* not good idea to set to zero ... */
int accentedflag=1;     /* deal with accented / composite chars */
/* int compoundsafe=1; */ /* be paranoid about synthetic fonts */
int syntheticsafe=1;    /* be paranoid about synthetic fonts */
int bSuppressPartial=0;   /* suppress partial font downloading */
int bForceFullArr=1;    /* force full 256 element array 1993/May/27 */
int keepgap=0;        /* keep space bytes before ND 1993/Aug/5 */
int stripchecking=1;    /* strip garbage from nasty old Adobe fonts */
int showcommandflag=0;    /* show command line (for debugging/DVIWindo) */

int columns=78;     /* 128 maximum length of output line in hex */
            /* used in out_encrypt  dviextra.c */
int bBackWardFlag=1;  /* force long form of octal escape in strings */
            /* required for old Apple Laser Writer */
int bForwardFlag=1;   /* use \b, \t, \n, ... \f, \r in strings 93/Sep/29 */
int bWindowsFlag=0;   /* reencode StandardEncoding to Windows ANSI */
            /* *or* by specified textencoding - ENCODING env var */
int bCheckEncoding = 1; /* check match in checksum specified encoding vector */
int bANSITeX=1;     /* splice in bottom of tex_text in ansi vector */
int bAllowColor = 0;  /* allow use of colorimage operator for TIFF */
/* int bKeepBlack = 1; */ /* suppress textcolor, rulecolor, figurecolor */
int bKeepBlack = 0;   /* suppress textcolor, rulecolor, figurecolor 95/Mar/1 */
int bBindFlag = 0;    /* bindall at top of page 1996/Oct/12 */
int bCheckFonts = 0;  /* check fonts 1993/Nov/2 */
/* int bCheckVersion = 1; */  /* check preamble in PS  1993/Nov/4 */
/* int bCheckVersion = 3; */  /* check preamble in DVIPSONE  1994/Nov/17 */
int bCheckVersion = 2;  /* check preamble version 1994/Nov/17 orred bits */
            /* 1 => check in PS file, 2 => check in DVIPSONE */
int usealtGoToR=0;    /* use alternate /GoToR action in PDF 98/Jun/30 */
            /* not yet, need to debug first */
int wantnotdef=1;   /* non-zero => want .notdef CharString */
            /* may be safer on non-Adobe RIPs */
int flushcr=1;      /* try and get rid of those pesky \r in input */
/* int retainnotice=1; */ /* retain /Notice in /FontInfo directory */
int stripinfo=1;    /* strip out rest of /FontInfo directory */
int substituteflag=1; /* non-zero => do font substitution */
int forcereside=0;    /* non-zero => force printer resident */
int avoidreside=0;    /* non-zero => veto printer resident */
int insertmetrics=1;  /* insert new font metrics in substituted */
            /* the above is now hard-wired to be on */
/* int wantpagetrailer=1; */  /* want %%PageTrailer 94/Mar/3 */
int bsetoverprint=0;  /* setoverprint true if on - 99/Sep/6 */
int bOptionalDSC=1;   /* generate highly optional PS DSC code */
            /* now frozen at 1 96/May/12 */
int dosetupearly=0;   /* want document setup before fonts - old way */
            /* now frozen at zero 96/May/12 */
int newbopflag=1;   /* want LogicalPage PhysicalPage */
int bLandScape=0;   /* asked for landscape mode -*O 95/Jun/24 */

int pagetpic=0;     /* non-zero if TPIC special used on this page */

char *fontprefix=NULL;  /* prefix to use on FontName */

char szRandomPrefix[8]="ABCDEF+";  /* Random prefix for Adobe Reader */
            /* avoid partial font subfont cache problem */
            /* 6 characters followed by + and terminating null */

int uppercaseflag=0;  /* non-zero => convert font names to upper case */

/* int pageorder=1; */  /* +1 normally, is -1 if reverseflag is on */

int errlevel=0;     /* returned at end */
int maxerror=0;     /* maximum of above */
int skipflag=0;     /* on when page to be skipped */
int complainedaboutj=0;     /* reset at top of page */

long beginpage=-LINFINITY;  /* first page to print - command line */
long endpage=LINFINITY;   /* last page to print - command line  */

int beginorend=0;     /* +1 => last was begin -1 => last was end */

int rangeindex=0;     /* index into following table */

long *beginpages = NULL;    /* table of pagebegins */
long *endpages   = NULL;    /* table of pageends - */

// int pagerangeseq[MAXRANGES];   /* which instance of page range desired ? */
int *pagerangeseq=NULL;   /* which instance of page range desired ? */
/* int pagerangehit[MAXRANGES];*/ /* which instance of page range hit so far ? */

int maxranges=0;      /* adjusted dynamically now */

int evenpageflag=0;     /* print only even pages */
int oddpageflag=0;      /* print only odd pages */

long numpages=0;      /* number of pages actually processed */

double magnification=0.0; /* user specified magnification */
double xmagnification=0.0;  /* user specified magnification */
double ymagnification=0.0;  /* treated as relative to center of page */

double rotation=0.0;    /* user specified rotation */
              /* treated as about center of page */

double xoffset=0.0;     /* x offset read off command line */
double yoffset=0.0;     /* y offset read off command line */

#define UNKNOWNOFFSET -32767.0

double xoffsete = UNKNOWNOFFSET; /* x offset even pages */
double yoffsete = UNKNOWNOFFSET; /* y offset even pages*/
double xoffseto = UNKNOWNOFFSET; /* x offset odd pages */
double yoffseto = UNKNOWNOFFSET; /* y offset odd pages */

int evenoddoff = 0;     /* non-zero if offset differs even/odd pages */

int nRepeatIndex=0;
int nRepeatCount=1;     /* new way to make multiple copies 95/Aug/27 */

long nMinRule=0;      /* min rule thickness (Adobe Reader fix) 95/Oct/10 */

int version=2;
int revision=2;       /* now obtained from progversion string */
int subrevision=4;

char *progversion = "2.2.4";      /* 2000/May/24 */

/* WARNING: remember to *also* change version number in DVIPREAM.PS !!! */
/* look for line: /checkversion{1 2 3 hashversion 4 1 roll hashversion ne */
/* AND look for line: % DVIPSONE preamble version 1.2.3 (1995 June 25) */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\nCopyright (C) 1990--2000, Y&Y, Inc.\n"
                  "Copyright (C) 2007, TeX Users Group.\n"
                  "Copyright (C) 2014, Clerk Ma.\n";
                  //"This program is free software; you can redistribute it and/or modify\n"
                  //"it under the terms of the GNU General Public License as published by\n"
                  //"the Free Software Foundation; either version 2 of the License, or\n"
                  //"(at your option) any later version.\n\n";


char *company = "Y&Y, Inc.";  /* 97/Apr/29 */
char *URL     = ""; /* string in binary ... */

char *filenamex=NULL;     /* remember full file name here */

char *task="Starting Up";   /* useful for error messages */

char *BasePath=NULL;  /* non blank if new directory structure 96/Aug/28 */

/*  Following initial guesses are set up at very beginning of main() */
/*  May then be overridden by program path, command line, or by env vars */

char * fontsubpath = "c:\\yandy\\dvipsone\\sub";   /* default font substitution file path */
char * procsetpath = "c:\\yandy\\dvipsone\\pre";   /* default preamble file path */
char * vecpath     = "c:\\yandy\\fonts\\encoding"; /* default encoding vectors paths */
char * tfmpath     = "c:\\yandy\\fonts\\tfm";      /* default TFM files for metrics */
char * texfonts    = "c:\\yandy\\fonts\\tfm";      /* default place for texfonts.map */
char * epspath     = "c:\\ps;c:\\eps;c:\\tiff;c:\\wmf";        /* pretty random! */
char * afmpath     = "c:\\psfonts\\afm;c:\\yandy\\fonts\\afm"; /* pretty random! */

char * fontsubrest = "standard";  /* default font substitution file name */
char fontsubfile[FNAMELEN];
char * procsetrest = "dvipream";  // default preamble file name
char procsetfile[FNAMELEN];      // complete file name for preamble
char * tpicrest    = "dvitpics";
char * textencoding = "ansinew";   /* default encoding to use text fonts *//* may differ from ansinew 94/Dec/17 */
char * textenconame = "ansinew";   /* just vector name itself, no path */

unsigned long nCheckSum=0;      /* checksum for ENCODING= *//* 95/Feb/3 */

// char programpath[MAXPATHLEN]=""; /* pathname of program */
char *programpath = NULL;       /* pathname of program */

// char dvipath[MAXPATHLEN]="";   /* pathname of dvi file - command */

char *dvipath = NULL;         /* pathname of dvi file - command */

/* PFB files could also be in c:\\winnt\\fonts ... */

/*  may be given on command line, env var, or use PFB_Dir in ATM.INI */
/*  char *deffontpath = "c:\\psfonts;c:\\dvipsone\\pfb"; */ /* 1993/Nov/15 */
char *deffontpath = "c:\\yandy\\psfonts"; /* 1993/Nov/15 */
char *fontpath = NULL;                    /* 1994/May/23 */

/*  may be given on command line, env var, or use PFM_Dir in ATM.INI */
char *defpfmpath = "c:\\yandy\\psfonts\\pfm";
char *pfmpath=NULL;                   /* 1994/May/23 */

char *outputfile=NULL;      /* output file or empty */

int subfontfileindex=0;     /* pointer into following */

/* char *subfontfile = ""; */   /* user subfile file or empty (-s=...) */
char *subfontfile[MAXSUBFILES];   /* user subfile file or empty (-s=...) */

int prologfileindex=0;      /* pointer into following */

/* char *prologfile = ""; */    /* user prolog file or empty (-w=...) */
char *prologfile[MAXPROLOGS];     /* user prolog file or empty (-w=...) */

char *dscfile=NULL;     /* name of (single) DSC header file */

/* provides for multiple \special{header=...} --- comma separated list */
int nheaderlength=0;    // 99/July/14 - length of accumulater headerfile info
char *headerfile=NULL;    // name of header file(s) from \special

int headertextlen=0;    // how much has been accumulated in following 
char *headertext=NULL;    // accumulated header PS text from headertext= 
              // need to free at end of file 

int dsccustomlen=0;     /* how much has been accumulated in following */
char *dsccustom=NULL;   /* accumulated DSC header text from DSCcomment= */
              /* need to free at end of file */

int commandspeclen=0;   /* how much has been accumulated in the following */
char *commandspec=NULL;   /* command line stuff from DVI file */

int keywordslen=0;      /* how much has been accumulated in following */
char *keywords      = NULL; /* accumulated keywords from PDF: Keywords ... */
char *creatorstring = NULL; /* Creator for DocInfo in Adobe Reader */
char *titlestring   = NULL; /* Title for DocInfo in Adobe Reader */
char *subjectstring = NULL; /* Subject for DocInfo in Adobe Reader */
char *authorstring  = NULL; /* Author for DocInfo in Adobe Reader */
char *basestring    = NULL; /* Base for DocView in Adobe Reader */
char *pagemode      = NULL; /* Base for DocView in Adobe Reader */


char line[MAXLINE];       /* general purpose input `buffer' 512 bytes */

#ifdef USELOGFILE
char logline[MAXLINE];      /* place for log file output 512 bytes */
#endif

/* char comment[MAXCOMMENT]; */ /* space for comment - TeX date/time - trim ?*/

char *comment=NULL;     /* space for comment - 1995/July/15 - 256 => 27 */

/* Keep convention that lower case letters are straight DVI command trans - */
/* while upper case letters are combinations of sorts */

/* Code to show VM usage and page rendering time: */ /* flushed 1996/May/6 */

/* % add processing to beginning and end of page:\n\ */

/* char *duplexcode = "\
true statusdict /setduplexmode get exec\n\
false statusdict /settumble get exec\n"; */

/* num/den units of measurement from DVI file */
/* multiply all quantities in DVI file by this to get 10^{-7} meters */
/* in TeX, this is normally set to 25400000/473628672 */ 

unsigned long num, den, mag;

// unsigned long mag;   /* Tex \mag => 1000 times desired magnification */

/* the following are found in post at end of file */

unsigned long dvi_l, dvi_u; /* max page height + depth & max width *//* NA */

int dvi_s, dvi_t;   /* stack depth & number of pages (bop) in DVI file */

// int pagenumber=0;    /* counting pages seen */
long pagenumber=0;    /* counting pages seen */

long counter[10];   /* \count0 ... \count9 in TeX */

long previous=-1;   /* pointer to previous bop in this file */

int ff;         /* current font - changed by fnt and fnt_num - reset by bop */
int fnt;        /* fnt =  finx[ff] */

int fnext=0;      /* next slot to use normal fonts */

// int finx[MAXFONTNUMBERS];    /* index into next now default */
short finx[MAXFONTNUMBERS];     /* index into next */
char *fontname[MAXFONTS];     /* 1999/Nov.6 */
char *subfontname[MAXFONTS];    /* 1999/Nov/6 */
char *fontvector[MAXFONTS];     /* font remapping vector if any */
char *fontchar[MAXFONTS];     /* which characters seen */
int fontsubflag[MAXFONTS];  /* non-negative if substitute font to be used */
int fontproper[MAXFONTS]; /* code if font is resident/forced/remapped */
unsigned long fc[MAXFONTS]; /* checksum of TFM file (use for encoding info) */
unsigned long fs[MAXFONTS];   /* at size - 512 bytes */ 

/* unsigned long fd[MAXFONTS]; */ /* design size */ /* NOT ACCESSED ? */
                  /* flushed 95/Jan/10 */

/* font k is to be used at mag * s / (1000 * d) times its normal size */

// char *fontsubfrom;
char *fontsubfrom[MAXSUBSTITUTE];
// char *fontsubto;
char *fontsubto[MAXSUBSTITUTE];
// char *fontsubvec;
char *fontsubvec[MAXSUBSTITUTE];

int maxsubstitute=MAXSUBSTITUTE;  /* initial value 1994/May/23 */

int maxfonts=MAXFONTS;        /* initial value 1994/May/23 */

/* char *currentfont; */      /* pointer to current font */

/* int maxstack=0; */       /* max stack depth allowed on printer */

int currentfirst=0;   /* look in current directory for files 98/Jul/20 */
            /* make permanently 1 sometime ??? */

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

int usefontmap=1;         /* used in dvipslog.c 1996/July/30 */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*  if (usedviwindo)  setupdviwindo();  */ /* need to do this before use */

int usedviwindo = 1;    /* use [Environment] section in `dviwindo.ini' */
              /* reset if setup of dviwindo.ini file fails */

char *dviwindo = NULL;    /* full file name for dviwindo.ini, with path */

#ifdef USELOGFILE
char *logfilename = "dvipsone.log";
#endif

FILE * logfile = NULL;  /* 1999/Apr/20 */
FILE * input   = NULL;  /* used by following */
FILE * output  = NULL;  /* used by `interrupt handler' */

char fn_in[FNAMELEN], fn_out[FNAMELEN];   /* 1994/Mar/1 */

void perrormod (char *s)
{
  sprintf(logline, "`%s': %s\n", s, strerror(errno));
  showline(logline, 1);
}

void pause (void)
{
#ifndef _WINDOWS
  fflush(stdout);     /* ??? */
  fflush(stderr);     /* ??? */
  (void) _getch();    /* ??? */
#endif
}

void checkpause (int flag) /* 95/Oct/28 */
{
  char *s;
  int debugpause=0;

  s = grabenv("DEBUGPAUSE");

  if (s != NULL)
    sscanf(s, "%d", &debugpause);
//  printf("DEBUGPAUSE=%d flag %d\n", debugpause, flag);  // debugging only

  if (flag < 0)
    return;

  if (debugpause)
  {
    if (debugpause || flag > 0)
    {
      showline("\n", 0);
#ifndef _WINDOWS
      showline("Press any key to continue . . .\n", 0);
      pause();
#endif
    }
  }
}

void checkenter (int argc, char *argv[]) /* 95/Oct/28 */
{
  int m;
  char current[FILENAME_MAX];

  if (grabenv("DEBUGPAUSE") != NULL)
  {
    (void) _getcwd(current, sizeof(current)); /* FILENAME_MAX */
    sprintf(logline, "Current directory: `%s'\n", current);
    showline(logline, 0);
    for (m = 0; m < argc; m++)
    {
      sprintf(logline, "%2d: `%s'\n", m, argv[m]);
      showline(logline, 0);
    }
    checkpause(-1);
  }
}

// Note: in DLL version this returns and sets abortflag - not anymore

void checkexit (int n) /* 95/Oct/28 */
{
  checkpause(1);

#ifdef USELOGFILE
  if (logfile != NULL)
  {
    fclose(logfile);
    logfile = NULL;
  }
#endif

  uexit(n);
}

/*************************************************************************/

// Note: in DLL version this returns NULL and sets abortflag if it fails

char *zstrdup (char *s)    /* new central location 1996/Aug/28 */
{
  char *new = _strdup(s);

  if (new != NULL)
    return new;

  sprintf(logline, " ERROR: Unable to allocate memory for %s\n", s);
  showline(logline, 1);

#ifndef _WINDOWS
  showline("Press any key to continue . . .\n", 0);
  pause();
//  exit(1);        // pretty serious !
#endif
  uexit(1);
}

/*****************************************************************************/

// when run as console application we read INI file directly

// split off filename generation 98/Jan/9 so can reuse for ATMREG.ATM also

void setupinifilesub (char *ininame, char *fullfilename)
{
  char *windir;

  *fullfilename = '\0';
/*  Easy to find Windows directory if Windows runs */
/*  Or if user kindly set WINDIR environment variable */
/*  Or if running in Windows NT */
  if ((windir = getenv("windir"))     != NULL || /* 1994/Jan/22 */
      (windir = getenv("WINDIR"))     != NULL ||
      (windir = getenv("winbootdir")) != NULL || /* 1995/Aug/14 */
      (windir = getenv("SystemRoot")) != NULL || /* 1995/Jun/23 */
      (windir = getenv("SYSTEMROOT")) != NULL)   /* 1995/Jun/23 */
  {
    strcpy(fullfilename, windir);
    strcat(fullfilename, "\\");
    strcat(fullfilename, ininame);
/*    printf("Using WINDIR %s\n", fullfilename); */ /* 1992/Jan/22 */
  }
  else
  {
    _searchenv (ininame, "PATH", fullfilename);
/*    printf("Using SEARCHENV %s\n", fullfilename); *//* 1992/Jan/22 */
  }

  if (*fullfilename == '\0')    /* ugh, then try standard place */
  {
    strcpy(fullfilename, "c:\\winnt");
    strcat(fullfilename, "\\");
    strcat(fullfilename, ininame);
  }
}

/* above also used for atmreg.atm from dvipslog.c */

#ifndef _WINDOWS

int dviwindoinisetup = 0;   /* turned on after setup */

/* set up full file name for ini file and check for specified section */
/* e.g. dviwindo = setupinifile("dviwindo.ini", "[Environment]") */
/* e.g. atmini = setupinifile("atm.ini", "[Setup]") */

char *setupinifile (char *ininame, char *section)
{
  char fullfilename[FNAMELEN]="";
  char line[MAXLINE];
  FILE *input;
  int m;

  setupinifilesub(ininame, fullfilename);

  if (traceflag)
  {
    sprintf(logline, "Trying to open %s\n", fullfilename);
    showline(logline, 0);
  }

/*  check whether ini file actually has required section */
  input = fopen(fullfilename, "r");

  if (input != NULL)
  {
    m = strlen(section);
    while (fgets (line, sizeof(line), input) != NULL) { /* MAXLINE */
      if (*line == ';') continue;
/*      if (strncmp(line, section, m) == 0) { */ /* case insensitive */
      if (_strnicmp(line, section, m) == 0) { /* 95/June/23 */
        fclose(input);
        return zstrdup(fullfilename);
      }
    }
    fclose(input);
  }
  return NULL;    // failed, for one reason or another
}

char *envsection = "[Environment]";

int setupdviwindo (void)
{
  if (! usedviwindo)
    return 0; /* already tried and failed */

  if (dviwindo != NULL && *dviwindo != '\0')
    return 1; /* already tried and succeeded */

  dviwindo = setupinifile("dviwindo.ini", envsection);

  if (dviwindo != NULL && *dviwindo != '\0')
    return (*dviwindo != '\0');
  else
  {
    usedviwindo = 0; /* failed, don't try this again */
    return 0;
  }
}
#endif  // end of #ifndef _WINDOWS

#ifdef NEEDATMINI

/* grab setting from `atm.ini' 94/June/15 */

/*  if (useatmini)  setupatmini();  */ /* need to do this before use */

int useatmini = 1;       /* use [Setup] section in `atm.ini' */
                         /* reset if setup of atm.ini file fails */
char *atmini=NULL;       /* full file name for atm.ini, with path */
int useatmreg=1;         /* use ATMREG.ATM for %%IncludeFont */
                         /* reset if setup of ATMREG.ATM file fails */
int usepsfontname=1;     /* allow use of PS FontName in DVI TFM */
char *szATMRegAtm=NULL;  /* full file name for ATMREG.ATM, with path */
int useatmfontsmap=1;    /* zero if tried and failed to find atmfonts.map */
char *atmfontsmap=NULL; /* file name and path to ATMFONTS.MAP */

char *atmsection="[Setup]";   /* ATM.INI section */

#ifndef _WINDOWS
int setupatmini (void)
{
  if (! useatmini)
    return 0; /* already tried and failed */

  if (atmini != NULL && *atmini != '\0')
    return 1; /* already tried and succeeded */

  atmini = setupinifile("atm.ini", atmsection);

  if (atmini != NULL && *atmini != '\0')
    return (*atmini != '\0');
  else
  {
    useatmini = 0;  /* failed, don't try this again */
    return 0;
  }
}
#endif  // end of ifNdef _WINDOWS

#endif  // end of ifdef NEEDATMINI

/* NOTE: getenv *is* sensitive to case of env variable, although, */
/* in DOS user has no way to define anything but upper case */

/* NOTE: GetPrivateProfileString is *not* sensitive to case of */
/* lpSection *or* lpEntry */

/* It seems that a `section' in an INI file may contain a blank line */
/* The end of a section is apparently the start of the next section */

/* if obtained from ini file it will use string dup to make safe copy */

/* WARNING: this will return NULL if not found anywhere - just like getenv */

char *grabenvvar (char *varname, char *inifile, char *section, int useini)
{
  FILE *input;
  char line[MAXLINE];
  char *s;
  int m, n;
/*  char *linedup; */

  if (useini == 0 || *inifile == '\0')
  {
    return getenv(varname); /* give up, get from environment *//* not strdup if non-NULL ? */
  }

  input = fopen(inifile, "r");

  if (input != NULL)
  {
    m = strlen(section);
    /* search for [...] section */ /* should be case insensitive */
    while (fgets (line, sizeof(line), input) != NULL) {
      if (*line == ';')
        continue;

      if (*line == '\n')
        continue;

      if (_strnicmp(line, section, m) == 0) /* 95/June/23 */
      {
        /* search for varname=... */ /* should be case sensitive ? */
        n = strlen(varname);

        while (fgets (line, sizeof(line), input) != NULL) {
          if (*line == ';')
            continue;

          if (*line == '[')
            break;

          if (*line <= ' ')
            continue;   /* 95/June/23 */

          if (_strnicmp(line, varname, n) == 0 &&
              *(line+n) == '=') /* found it ? */
          {
              fclose (input);
              /* flush trailing white space */
              s = line + strlen(line) - 1;

              while (*s <= ' ' && s > line)
                *s-- = '\0';

              s = line + n + 1;

              if (traceflag)
              {
                sprintf(logline, "%s=%s\n", varname, s);
                showline(logline, 0);
              }
              return zstrdup(s);
          } /* end of matching varname */
        } /* end of while fgets */
      }       /* end of search for [Environment] section */
    }
    fclose (input);
  } /* end of if fopen */
/*  useini = 0; */      /* so won't try this again ! need & then */
  return getenv(varname); /* failed, so try and get from environment */
              /* no tusing strdup when non-NULL ? */
}             /* this will return NULL if not found anywhere */


/* WARNING: this will return NULL if not found anywhere - just like getenv */
char *grabenv (char *varname)
{
  if (usedviwindo && ! dviwindoinisetup)
    setupdviwindo();   /* 99/June/16 */

  return grabenvvar (varname, dviwindo, "[Environment]", usedviwindo);
}

/***************************************************************************/

/* returns zero when reached EOF and line empty - char count otherwise */
/* modified 94/Feb/23 to return number of input characters read */
/* used to return number of characters assembled in buffer */
/* can handle different line terminations \n, \r, \n followed by \r */

/* get line from normal ASCII file */
int getline (FILE *input, char *buff)
{
  int c, k = 0, n = 0;
  char *s = buff;

  c = getc(input);
  n++;                    /* 1994/Feb/23 */
  if (c == EOF) {
    *s = '\0';
    return 0;
  }
/*  go until line terminator or EOF */ /* 0 added for EPSF file safety ...*/
/*  while ((c != '\n' && c != '\r' && c != EOF)) { */
  while ((c != '\n' && c != '\r' && c != EOF && c != 0)) {
    if (c < ' ' && c != '\t')
    {
/*  ignore control characters other than \t (and \n, \r)  */
/*      c = getc(input); */         /* ignore control chars */
/*      n++; */               /* 1994/Feb/23 */
    }
    else
    {
      *s++ = (char) c;          /* normal char - assemble */
      k++;
/*      c = getc(input); */
/*      n++;  */              /* 1994/Feb/23 */
    }
    c = getc(input);          /* get next */
    n++;                /* 1994/Feb/23 */
  }
/*  now either have line terminator or EOF */ /* is it \r followed by \n ? */
  if (c == '\r' && flushcr != 0)
  {
    c = getc(input);            /* look ahead */
    if (c != '\n')
    {
      (void) ungetc(c, input);      /* forget it again */
      c = '\r';   /* put back the return at least 92/Oct/30 */
    }
    else
      n++;               /* use the \n */
  }
/*  else if (c == EOF) { */
  else if (c == EOF || c == 0)        /* 1994/Feb/23 */
  {
    if (k > 0)
      c = '\n';         /* stick in a newline */
    else
    {
      *s = '\0';              /* forget it - nothing there */
      return 0;             /* blank line at EOF */
    }
  }
  *s++ = (char) c;
  k++;
  *s = '\0';          /* terminate it */
/*  return k; */        /* number of bytes assembled */
  return n;         /* number of bytes read 94/Feb/23 */
}

/* get non-comment, non-blank */
int getrealline (FILE *input, char *buff)
{
  int k;

  k = getline(input, buff);

  while ((*buff == '%' || *buff == '\n') && k > 0)
    k = getline(input, buff);

  return k;
}

/* supply extension if none */
void extension (char *fname, char *ext)
{
  char *s, *t;

  if ((s = strrchr(fname, '.')) == NULL || ((t = strrchr(fname, '\\')) != NULL && s < t))
  {
    strcat(fname, ".");
    strcat(fname, ext);
  }
}

/* change extension if present */
void forceexten (char *fname, char *ext)
{
  char *s, *t;

  if ((s = strrchr(fname, '.')) == NULL || ((t = strrchr(fname, '\\')) != NULL && s < t))
  {
    strcat(fname, ".");
    strcat(fname, ext);
  }
  else
    strcpy(s + 1, ext);    /* give it default extension */
}

/* remove extension if present */
void removeexten (char *fname)
{
  char *s, *t;
  if ((s = strrchr(fname, '.')) != NULL)
  {
    if ((t = strrchr(fname, '\\')) == NULL || s > t)
      *s = '\0';
  }
}

/* return pointer to file name - minus path - returns pointer to filename */

char *extractfilename (char *pathname)
{
  char *s;

  if ((s=strrchr(pathname, '\\')) != NULL)
    s++;
  else
  {
    if ((s=strrchr(pathname, '/')) != NULL)
      s++;
    else
    {
      if ((s=strrchr(pathname, ':')) != NULL)
        s++;
      else
        s = pathname;
    }
  }
  return s;
}

void showusage (char *program)
{
  int k;
  char *s = logline;

  sprintf(s, "%s [-{vrgh}] [-b=<begin>] [-e=<end>]\n"
      "[-m=<mag>] [-o=<orient>] [-x=<xoff>] [-y=<yoff>] [-c=<num>]\n"
      "[-f=<fontpath>] [-i=<epspath>] [-s=<sub>] [-l=<paper>]\n"
      "[-d=<destination>] <dvi-file-1> ... \n",
      program);
  showline(logline, 0);

  if (! detailflag)
  {
    return;
  }
//  s += strlen(s);
  sprintf(s, ""
      "    -?    show this usage summary\n"
      "    v:    verbose mode\n"
      "    r:    reverse order pages\n"
      "    g:    print only odd pages\n"
      "    h:    print only even pages (in reverse order)\n"
      "    b:    begin at given page\n"
      "    e:    end at given page\n");
  s += strlen(s);

  sprintf(s, "    m:    magnify by given factor  (default ");
  s += strlen(s);
  if (xmagnification != ymagnification)
  {
    sprintf(s, "%lg x %lg)\n", xmagnification, ymagnification);
  }
  else
  {
    sprintf(s, "%lg)\n", xmagnification);
  }
  s += strlen(s);

  sprintf(s, ""
      "    o:    re-orient by given angle  (default %lg degrees)\n"
      "    x:    horizontal offset  (right positive - default %lg pts)\n"
      "    y:    vertical offset  (down positive - default %lg pts)\n"
      "    c:    number of copies  (default %d)\n",
      rotation, xoffset, yoffset, copies);
  s += strlen(s);

  sprintf(s, "    f:    font file path  (default `%s')\n",
      (fontpath != NULL) ? fontpath : "");
  s += strlen(s);

  sprintf(s, "    i:    illustration file path  (default `%s')\n",
      (epspath != NULL) ? epspath : "");
  s += strlen(s);

  sprintf(s, "    s:    substitution file name  (default `");
  s += strlen(s);

  if (subfontfileindex > 0) /* did user specify subs ? */
  {
    for (k  = 0; k < subfontfileindex; k++)
    {
      sprintf(s, "%s", subfontfile[k]);
      s += strlen(s);
      if (k < subfontfileindex-1)
      {
        sprintf(s, ", ", errout);
        s += strlen(s);
      }
    }
  }
  else
  {
    sprintf(s, "%s", fontsubrest);
    s += strlen(s);
  }
  sprintf(s, "')\n");
  s += strlen(s);
  showline(s, 0);
  sprintf(s, "    l:    paper size type (default `%s')\n",
      (paper_type != NULL) ? paper_type : "");
  s += strlen(s);
  sprintf(s, "    d:    destination ");
  s += strlen(s);
  if (outputfile == NULL)
  {
    sprintf(s, "(PRN, LPT1 ... , AUX, COM1 ... , NUL or file)\n\
       (default is file with same name, but extension `ps')\n");
    s += strlen(s);
  }
  else
  {
    sprintf(s, "(default `%s')\n", outputfile);
    s += strlen(s);
  }
  showline(logline, 1);
#ifndef _WINDOWS
  uexit(1);     // has this been set up yet ???
#endif
}

/* not used yet: ? and numbers 2 - 9, still available */

/* ready for recycling: n u R o ? */

/* u J */

/* may be reuse *o (bQuotePage) *v (bProtectStack) *y (bStoppedContext) */

/*  if toggle == 0, then toggle */
/*  if toggle < 0 then reset to default. if toggle > 0 then set to opposite */
/*  last argument is default state - which should match static value above*/

/* void changeflag (int *flag, int toggle) { */
void changeflag (int *flag, int toggle, int dflt)
{
  if (toggle == 0)
    *flag = (1 - *flag); /* toggle it */
  else
  {
    if (toggle < 0)
      *flag = dflt;  /* reset it (back to default) */
    else
      *flag = !dflt; /* set it (not default setting) */
  }
}

//    returns 0 for flag (no args needed)
//    returns 1 for command (args needed)
//    returns -1 if bad flag or arg

/* int decodeflag (int c) { */
int decodeflag (int c, int toggle)
{
  switch(c) {
    case '?': detailflag = 1; return 0;
    case 'q': quietflag++; return 0;
    case 'v': changeflag(&verboseflag, toggle, 0); return 0; 
    case 'V': changeflag(&showfontflag, toggle, 0); return 0;
    case 'r': changeflag(&reverseflag, toggle, 0); return 0;
    case 'g': changeflag(&oddpageflag, toggle, 0); return 0;
    case 'h': changeflag(&evenpageflag, toggle, 0); 
          changeflag(&reverseflag, toggle, 0);  return 0;
    case 'z': changeflag(&makeepsf, toggle, 0); return 0;
/*    case 'k': forcereside = (1 - forcereside); return 0;*/
    case 'k': changeflag(&forcereside, toggle, 0); return 0;
/*    case 'u': showpageinfo = (1 - showpageinfo); return 0;*/
/*    case 'u': changeflag(&showpageinfo, toggle, 0); return 0; */
    case 'u': changeflag(&bsetoverprint, toggle, 0); return 0;
/*    case 'j': verbatimflag = (1 - verbatimflag); return 0;*/
    case 'j': changeflag(&verbatimflag, toggle, 0); return 0;
/*    case 'n': busedviencode = (1 - busedviencode); return 0; */
/*    case 'n': changeflag(&busedviencode, toggle, 0); return 0; */
                 /* killed busedviencode 1996/May/12 */
    case 'n': changeflag(&logfileflag, toggle, 0); return 0; /* 99/Apr/28 */
/*    case 'K': avoidreside = (1 - avoidreside); return 0;*/
    case 'K': changeflag(&avoidreside, toggle, 0); return 0;
/*    case 'Z': passcomments = (1 - passcomments); return 0;*/
    case 'Z': changeflag(&passcomments, toggle, 1); return 0;
/*    case 'N': accentedflag = (1 - accentedflag); return 0; */
    case 'N': changeflag(&accentedflag, toggle, 1); return 0; 
/*    case 'R': retplusnew = (1 - retplusnew); return 0; */
/*    case 'R': changeflag(&retplusnew, toggle, 1); return 0; */
    case 'R': changeflag(&distillerlogflag, toggle, 0); return 0; // 00 Aug 15
/*    removed 1996/Aug/26 since default has changed */
/*          flushcr = (1 - flushcr); */  /* NO */
/*    case 'S': showsubflag = (1 - showsubflag); return 0; */
    case 'S': changeflag(&showsubflag, toggle, 0); return 0;
/*    case 'P': bSuppressPartial = (1 - bSuppressPartial);*/
    case 'P': changeflag(&bSuppressPartial, toggle, 0);
/*          if (bSuppressPartial) {
             bForceFullarr = 1;
             bAllowTexText = 0;
             bAllowANSI = 0; 
             bAllowStandard = 0;
          } */ /* in effect already happens ? make explicit ? */
          return 0;             /* 1992/Sep/12 */
/*    case 'Q': stripchecking = (1 - stripchecking);  */
    case 'Q': changeflag(&stripchecking, toggle, 0);  
          return 0;             /* 1992/Oct/7 */
/*    case 'J': bForceFullArr = (1 - bForceFullArr);*/
/*    case 'J': changeflag(&bForceFullArr, toggle, 1);
          return 0; */            /* 1992/Oct/22 */
/*    case 'I': ignorespecials = (1 - ignorespecials);*/
    case 'I': changeflag(&bIgnoreSpecials, toggle, 0);
          return 0;             /* 1992/Oct/25 */   
/*    case 'W': wantcontrold = (1 - wantcontrold); */
    case 'W': changeflag(&wantcontrold, toggle, 0); 
          return 0;             /* 1992/Nov/24 */
/*    case 'T': preservefont = (1 - preservefont);*/
    case 'T': changeflag(&preservefont, toggle, 1);
          return 0;             /* 1992/Dec/20 */
/*    case 'U': uppercaseflag = (1 - uppercaseflag); */
    case 'U': changeflag(&uppercaseflag, toggle, 0); 
          return 0;             /* 1992/Dec/22 */
/*    case 'Y': dontstrip = (1 - dontstrip); */
    case 'Y': changeflag(&dontstrip, toggle, 0); 
          return 0;             /* 1993/Mar/5 */
/*    case 'L': keepgap = (1 - keepgap);*/
    case 'L': changeflag(&keepgap, toggle, 0);
          return 0;             /* 1993/Aug/5 */
/*    case 'M': showcommandflag = (1 - showcommandflag);*/
    case 'M': changeflag(&showcommandflag, toggle, 0);
          return 0;             /* 1993/Aug/28 */
/*    case 'O': bForwardFlag = (1 - bForwardFlag);*/
    case 'O': changeflag(&bForwardFlag, toggle, 1);
          return 0;             /* 1993/Sep/28 */
/*    case 'X': backwardflag = (1 - backwardflag); */
/*    case 'X': bWindowsFlag = (1 - bWindowsFlag); */
    case 'X': changeflag(&bWindowsFlag, toggle, 0); 
          return 0;             /* 1993/Sep/28 */
    case '2': changeflag(&currentfirst, toggle, 0);
            return 0;           /* 1998/Jul/20 */
/*  following are the extended codes using `*' */
/*    case '0': */    /* now needed for forcing a flag OFF */
/*    case 128 + 'b': bKeepBlack = (1 - bKeepBlack);*/
/*    case 128 + 'b': changeflag(&bKeepBlack, toggle, 1); */
    case 128 + 'b': changeflag(&bKeepBlack, toggle, 0);
          return 0;             /* 1993/Oct/22 */
/*    case '1': */    /* now needed for forcing a flag ON */
/*    case 128 + 'a': bANSITeX = (1 - bANSITeX); */
//    case 128 + 'a': changeflag(&bANSITeX, toggle, 1);
//          return 0;             /* 1993/Dec/18 */
//    case 128 + 't': changeflag(&traceflag, toggle, 0);
//          return 0;             /* 1994/Feb/17 */
    case 128 + 'c': changeflag(&bAllowColor, toggle, 0);
          return 0;             /* 1993/Oct/18 */
    case 128 + 'f': changeflag(&bCheckFonts, toggle, 0);
          return 0;             /* 1993/Nov/2 */
/*    case 128 + 'o': changeflag(&bOptionalDSC, toggle, 1);
          return 0; */            /* 1994/Mar/1 exp */
    case 128 + 'o': changeflag(&bQuotePage, toggle, 1);
          return 0;             /* 1997/Mar/7 exp */
    case 128 + 'v': changeflag(&bProtectStack, toggle, 1);
          return 0;             /* 1997/Nov/24 exp */
    case 128 + 'y': changeflag(&bStoppedContext, toggle, 1);
          return 0;             /* 1997/Nov/24 exp */
    case 128 + 'Y': changeflag(&bKeepTermination, toggle, 0);
          return 0;             /* 1998/Feb/8 exp */
/*    case 128 + 'e': changeflag(&dosetupearly, toggle, 0);
          return 0; */            /* 1994/Mar/2 exp */
    case 128 + 'e': changeflag(&bConvertReturn, toggle, 0);
          return 0;             /* 1996/Aug/22 */
    case 128 + 's': changeflag(&bSmartCopyFlag, toggle, 1);
          return 0;             /* 1994/Feb/23 */
    case 128 + 'i': changeflag(&bAllowInclude, toggle, 1);  
          return 0;             /* 1993/Dec/21 exp */
    case 128 + 'n': changeflag(&bShortFont, toggle, 1);
          return 0;             /* 1993/Dec/11 */
    case 128 + 'N': changeflag(&bUseInternal, toggle, 1);
          return 0;             /* 1994/June/7 */
    case 128 + 'R': changeflag(&bRemapControl, toggle, 0);
          return 0;             /* 1994/June/20 */
    case 128 + '2': changeflag(&bRemapSpace, toggle, 0);
          return 0;             /* 1995/Oct/17 */
//    case 128 + 'p': changeflag(&bPassEPSF, toggle, 1);
    case 'p': changeflag(&bPassEPSF, toggle, 1);
          return 0;             /* 1994/Mar/9 */
    case 128 + 'l': changeflag(&bBackWardFlag, toggle, 1);
          return 0;             /* 1994/Mar/16 */
    case 128 + 'm': changeflag(&bTexFontsMap, toggle, 1);
          return 0;             /* 1994/Mar/19 */
    case 128 + 'S': changeflag(&bSciWord, toggle, 0);
          return 0;             /* 1994/Apr/21 */
//    case 128 + 'M': changeflag(&bMaxSub, toggle, 0);
//          return 0;             /* 1994/May/23 */
//    case 128 + 'F': changeflag(&bMaxFont, toggle, 0);
//          return 0;             /* 1994/May/23 */
    case 128 + 'W': changeflag(&bWrapSpecial, toggle, 1);
          return 0;             /* 1994/June/25 */
//    case 128 + 'B': changeflag(&bAllowBadBox, toggle, 1);
//          return 0;             /* 1994/Aug/19 */
    case 128 + 'E': changeflag(&bCheckEncoding, toggle, 1);
          return 0;             /* 1995/Jan/10 */
    case 128 + 'u': changeflag(&bMarkUnused, toggle, 1);
          return 0;             /* 1995/Mar/5 */
//    case 128 + 'g': changeflag(&bMMNewFlag, toggle, 1);
//          return 0;           /* 1995/May/13 */
    case 128 + 'g': changeflag(&bMMShortCut, toggle, 0);
          return 0;             /* 1995/May/25 */
//    case 128 + 'd': changeflag(&newbopflag, toggle, 1);
//          return 0;   // recycle 98/Feb/14 
    case 128 + 'd': changeflag(&bCarryColor, toggle, 1);
          return 0;   /*  98/Feb/14 */
    case 128 + 'T':   /* force standard form of encoding vector */
          bAllowShortEncode = 0;
          bAllowTexText = 0;
          bAllowANSI = 0;
          bForceFullArr = 1;
          return 0;             /* 1994/Oct/25 */
    case 128 + 'O': changeflag(&bLandScape, toggle, 0);
          return 0;             /* 1995/Jun/24 */
    case 128 + 'z': changeflag(&bSubRealName, toggle, 1);
          return 0;             /* 95/Aug/22 */
    case 128 + 'j': changeflag(&bDVICopyReduce, toggle, 1);
          return 0;             /* 95/Sep/16 */
    case 128 + 'x': changeflag(&bRandomPrefix, toggle, 0);
          return 0;             /* 95/Sep/27 */
    case 128 + 'G': changeflag(&bGhostHackFlag, toggle, 0);
          return 0;             /* 95/Nov/10 */
    case 128 + 'Q': changeflag(&bPassControls, toggle, 0);
          return 0;             /* 95/Dec/19 */
    case 128 + 'X': changeflag(&bForceFontsMap, toggle, 0);
          return 0;             /* 95/Dec/29 */
    case 128 + '3': changeflag(&nMaxErrors, toggle, 0);
          return 0;             /* 95/Dec/28 */
    case 128 + '4': changeflag(&bUseCounters, toggle, 0);
          return 0;             /* 96/Jan/28 */
    case 128 + '5': changeflag(&bStripBadEnd, toggle, 1);
          return 0;             /* 96/Feb/22 */
    case 128 + '6': changeflag(&bPDFmarks, toggle, 1);
          return 0;             /* 96/Feb/25 */
    case 128 + '7': changeflag(&bSuppressBack, toggle, 0);
          return 0;             /* 95/Dec/31 */
/*    case 128 + '8': errout = stdout; */
    case 128 + '8': errout = stderr;    /* reversed 97/Oct/26 */
          return 0;             /* 96/Mar/2 */
    case 128 + 'r': changeflag(&bDuplex, toggle, 0);
          return 0;             /* 96/May/5 */
    case 128 + 'w': changeflag(&bTumble, toggle, 0);
          return 0;             /* 96/Dec/20 */
/*    case 128 + 'B': retplusnew = (1 - retplusnew); */
    case 128 + 'B': changeflag(&retplusnew, toggle, 0);
          return 0;             /* 96/Aug/26 */
/*    case 128 + 'H': bOldDVIPSFlag = (1 - bOldDVIPSFlag); */
    case 128 + 'H': changeflag(&bOldDVIPSFlag, toggle, 0);
          return 0;             /* 96/Oct/4 */
    case 128 + 'K': changeflag(&bOldDVIPSFlip, toggle, 0);
          return 0;             /* 96/Nov/7 */
    case 128 + 'I': changeflag(&bBindFlag, toggle, 0);
          return 0;             /* 1996/Oct/12 */
    case 128 + 'L': changeflag(&bLevel2, toggle, 0);
          return 0;             /* 1996/Dec/20 */
    case 128 + 'Z': changeflag(&bAllowCompression, toggle, 1);
          return 0;             /* temp 1996/Dec/24 */
    case 128 + 'U': changeflag(&bAddBaseName, toggle, 1);
          return 0;             /* 97/Jan/30 */
    case 128 + 'V': changeflag(&usealtGoToR, toggle, 1);
          return 0;             /* 98/Jun/30 */
    case 't': traceflag = 1; return 0; 
/*    case 'O': duplexflag = (1 - duplexflag); return 0; */
/*    case 'k': currentdirect = 0; return 0; */
/*    case 'n': wantnotdef = 1; return 0; */
/*    case 'm': insertmetrics = 0; return 0; */
/*    case 'r': flushcr = 0; return 0; */
/*    case 'i': wantpreamble = 0; return 0; */
/*    case 'l': wantlogflag = 0; return 0; */
/*    case 'd': deepstack++; return 0; */

/*    the rest of these need arguments */
    case 'b': beginflag = 1; countzeroflag = 1; return 1;
    case 'B': beginflag = 1; countzeroflag = 0; return 1;
    case 'e': endflag = 1; countzeroflag = 1; return 1;
    case 'E': endflag = 1; countzeroflag = 0; return 1;
    case 'm': magniflag = 1; return 1;
    case 'o': rotateflag = 1; return 1;
    case 'x': xoffsetflag = 1; return 1;
    case 'y': yoffsetflag = 1; return 1;
    case 'c': copiesflag = 1; collated = 0; return 1;
    case 'C': copiesflag = 1; collated = 1; return 1;
    case 'f': pathflag = 1; return 1;   /* font search path */
    case 'i': epsflag = 1; return 1;    /* EPS insert search path */
//    case 't': tfmflag = 1; return 1;
    case 128 + 't': tfmflag = 1; return 1;
//    case 'a': afmflag = 1; return 1;
    case 128 + 'a': afmflag = 1; return 1;
//    case 'p': pfmflag = 1; return 1;
    case 128 + 'p': pfmflag = 1; return 1;
    case 's': subfileflag = 1; return 1;  
    case 'd': outputflag = 1; return 1;
    case 'l': paperflag = 1; return 1;  
    case 'w': prologflag = 1; return 1;
    case 'F': prefixflag = 1; return 1;
    case 'D': dvidictflag = 1; return 1;
    case 'G': vecpathflag = 1; return 1;
    case 'H': subpathflag = 1; return 1;
    case 'A': prepathflag = 1; return 1;
/*    case 'h': headerflag = 1; return 1;   */
/*    case 'w': vecflag = 1; return 1; */
/*    case 'e': extenflag = 1; return 1; */
/*    case 'c': clmnflag = 1; return 1; */
    case 128 + 'h': halftoneflag = 1; return 1;
    case 128 + 'P': boundingflag = 1; return 1; /* 94/May/6 */
    case 128 + 'D': dscextraflag = 1; return 1; /* 94/May/6 */
    case 128 + 'A': emeraldflag = 1; return 1;    /* 94/Sep/13 */
    case 128 + 'k': encodeflag = 1; return 1;   /* 95/Aug/14 */
    case 128 + '9': versionflag = 1; return 1;    /* 95/Jul/15 */
    case 128 + 'q': repeatflag = 1; return 1;   /* 95/Aug/27 */
    case 128 + 'C': minruleflag = 1; return 1;    /* 95/Oct/10 */
    case 128 + 'J': defdpiflag = 1; return 1;   /* 96/Apr/4 */
    default: {
       char *s;
       strcpy(logline, "ERROR: Invalid command line flag ");
       s = logline + strlen(logline);
       if (c < 32) sprintf(s, "`C-%c'", c+64);
       else if (c < 128) sprintf(s, "`%c'", c);
       else sprintf(s, "`*%c'", c-128);
       strcat(s, "\n");
       showline(logline, 1);
       detailflag = 1;
//       showusage(argv[0]);    // ???
//       return 0;
       return -1;
    }
  }
//  return -1;    /* need argument */
}

/*     *O landscape (default protrait)\n" */

/*     i: illustration (EPS) file path  (default `%s')\n", */
/*     c: number of copies  (default 1)\n\ */

/*    [-t=<tfm>] [-a=<afm>] [-p=<pfm>]\n\ */
/*    tfmpath, afmpath, pfmpath, */

/* These are rarely used flags for which showusage has no space */

/* \tt: TFM metric file path  (default `%s')\n\ */
/* \ta: AFM metric file path  (default `%s')\n\ */
/* \tp: PFM metric file path  (default `%s')\n\ */

/* \tn: make output `ATM safe' - don't used dviencode or tex_text \n\ */
/* \tn: do not use numeric reencoding of fonts\n\ */
/* \tq: quietly ignore unrecognized \special's\n\ */
/* \tz: output is meant to be EPS file\n\ */
/* \tu: show VM usage and page rendering time\n\ */
/* \tw: insert specified user prolog file\n\ */
/* \tj: allow insertion of verbatim PostScript\n\ */

/* These are older, now no longer used flag interpretations */

/* \tn: output should be ATM compatible (i.e. meant for previewing)\n\ */
/* \tw: flip horizontally (emulsion up printing)\n\ */
/* \th: header file name  (default `%s')\n\ */ /* procsetrest */
/* \tw: VEC reencoding file directory (default `%s')\n\ */
/* \tf: want control D at end of file\n\ */
/* \tk: do not write output in current directory\n\ */
/* \th: generate histogram of DVI commands seen\n\ */
/* \ti: do not insert preamble\n\ */
/* \tl: do not generate log of fonts used\n\ */
/* \tu: do not convert font names to upper case\n\ */
/* \ts: force substitution for fonts\n\ */
/* \ts: do not substitute fonts\n\ */
/* \tt: show DVIPSONE processing timing information\n\ */

/* used in giveup(...) and in dvipslog.c */

void tellwhere (FILE *input, int errflag)
{
  long place;
  char *s=logline;
/*  if (input == NULL) return;
  place = ftell(input);
  if (place > 0)
    fprintf(outfile, " at byte %ld in DVI file\n", place-1); */
  if (input != NULL)
  {
    place = ftell(input);

    if (place > 0)
    {
      sprintf(s, " at byte %ld in DVI file", place-1);
      s += strlen(s);
    }
  }
  strcat(logline, "\n");
  showline(logline, errflag);

  if (errflag > 0)
    errcount(0);
}

/* graceful exit with meaningful error message */
void giveup (int code)
{
  sprintf(logline, " while %s", task);
  showline(logline, 1);
  tellwhere(input, -1);
  checkexit(code);    /* 1995/Oct/28 */
}

/* possibly futile attempt to let output die */
void waitasec (int delay)
{
  clock_t starttime;

  starttime = clock();
  for(;;)
  {
    if (clock() > starttime + (delay * CLOCKS_PER_SEC) / 1000) break;
  }
}

// Is this OK in DLL version ???
// output will be NULL in DLL version ???

void cleanup (void)
{
  if (output != NULL)
  {
    if (directprint)
    {
#ifndef _WINDOWS
      fflush(stdout);           /* ??? 98/Jun/30 */
#endif

      if (wantcontrold != 0 || stripcomment != 0)
      {
        PSputc(4, output);      /* send control-D */
      }

      fflush(output); /* ??? 98/Jun/30 */
      fclose(output); /* close output */
      waitasec(1000); /* give it time to clear out ? */
    }
    else
    {
      if (wantcontrold)
      {
        PSputc(4, output);      /* send control-D */
      }

      fclose(output);       /* close output */
      (void) remove(fn_out);    /* and remove bad file */
    }
  }
}

void abortjob (void)
{
  cleanup();
  checkexit(3);
}

/* called each time `survivable' error encountered */
void errcount (int flag)
{
  if (flag)
    errlevel++;

  if (errlevel > nMaxErrors && nMaxErrors > 0)  /* 95/Dec/28 */
  {
    showline("\n", 0);
    sprintf(logline, "Too many errors (> %d) - giving up", nMaxErrors);
    showline(logline, 1);
    cleanup();
    giveup(1);
  }
}

/***********************************************************************/
/* flag = 0 => DocumentFonts */ /* => no longer used */
/* flag = 1 => DocumentSuppliedFonts */  /* => DocumentSuppliedResources */
/* this should also write out fonts that have not been found when flag = 0 */
/* this should list only base fonts, not remapped versions */
/* ideally this should list PS FontNames - but actually lists file names */ 
/* (except for printer resident fonts) */

int writefontlist (FILE *outfile, int flag, int column)
{
  int k, want;
  int wanted = 0, missing = 0, resident = 0, dependent = 0, unused = 0, instance = 0;

//  char ftemp[MAXTEXNAME];
  char ftemp[FNAMELEN];
  int property;
  
/*  printf("WRITING FONT LIST!\n"); */
  for (k = 0; k < fnext; k++)
  {
    property = fontproper[k];
    if ((property & C_RESIDENT) != 0)
      resident++;
    else
    {
      if ((property & C_MISSING) != 0)
        missing++;
      else
      {
        if ((property & C_DEPENDENT) != 0)
          dependent++; /* 92/Sep/2 */
        else
        {
          if ((property & C_INSTANCE) != 0)
            instance++;  /* 97/June/1 */
          else
          {
            if ((property & C_UNUSED) != 0)
              unused++;  /* 95/Mar/5 */
            else
              wanted++;
          }
        }
      }
    }
  }
/*  printf("WANT %d K %d FNEXT %d ", want, k, fnext); */
/*  no output if all fonts are resident (or missing) */
  if (wanted == 0)
  {
    PSputc('\n', outfile);
    return (resident + missing);      /* 1992/Oct/12 */
  }
  PSputs("font ", outfile);
  column += 5;
  for (k = 0; k < fnext; k++)
  {
    property = fontproper[k];
    want=0; 
/*    don't mention fonts already mentioned elsewhere */
/*    if (strcmp(fontname[k], subfontname[k]) != 0 || */
    if (fontname[k] == NULL || subfontname[k] == NULL ||
        strcmp(fontname[k], subfontname[k]) != 0 ||
        (property & C_MISSING) != 0)
    {
/*    don't mention resident remapped fonts ??? */
/*      if ((property & C_REMAPIT) != 0 &&
        (property & C_RESIDENT) != 0) ; 
      else */
      if (flag == 0 ||
          ((property & C_MISSING) == 0 && (property & C_RESIDENT) == 0))
      {
         if ((property & C_UNUSED) == 0)
         {
           if ((property & C_RESIDENT) == 0)
             want = 1;
           else
             if (flag == 0)
               want=-1;
           if (fontsubflag[k] >= 0) /* 1992/May/03 */
             want = 0;
         }
      }
    }

    if ((fontname[k] != NULL && subfontname[k] != NULL && strcmp(fontname[k], subfontname[k])) == 0 &&
        (property & C_REMAPIT) != 0 &&
        (property & C_DEPENDENT) == 0)
    {
      want = 1;
    }   /* experiment 1993/July/15 */
    
    if (property & C_INSTANCE)
      want = 0;  /* 97/June/1 */

/*    if (want > 0) fprintf(outfile, "%s ", fontname[k]); 
    else if (want < 0) fprintf(outfile, "%s ", subfontname[k]); */ /* ? */
    if (want != 0)
    {
      if ((property & C_MULTIPLE))     /* 97/June/1 */
      {
        if (subfontname[k] != NULL)
          strcpy (ftemp, subfontname[k]);
        else
          *ftemp = '\0';
      }
      else
      {
        if (fontname[k] != NULL)
          strcpy (ftemp, fontname[k]);
        else
          *ftemp = '\0';
      }
      if ((column += strlen(ftemp) + 1) > 76)
      {
        PSputs("\n%%+ font ", outfile);
        column = 9 + strlen(ftemp) + 1;
      }
      PSputs(ftemp, outfile);
      PSputc(' ', outfile);
    }
  }
  PSputc('\n', outfile);
  return (resident + missing);    /* 1992/Oct/12 */
}

/* seem to have a problem here with resident fonts */
/* don't want to list multiple times, yet must list once */
/* seems to have a problem with -k flag forcing recidency */

void writemissing (FILE *outfile, int column)
{
  int k;
//  char ftemp[MAXTEXNAME];
  char ftemp[FNAMELEN];
  int property;

  PSputs("font ", outfile);
  column += 5;

  for (k = 0; k < fnext; k++)
  {
    property = fontproper[k];

    if ((property & C_MISSING) != 0 ||
      (property & C_RESIDENT) != 0)
    {
      if ((property & C_UNUSED) != 0)
        continue;   /* 95/Mar/5 */
/*      don't list MM instances, since we'll include stubs after all */
/*      too early, we don't know yet that this is a PSS stub ??? */
      if ((property & C_INSTANCE) != 0)
        continue; /* 95/Sep/16 */

      if (subfontname[k] == NULL)
      {
        if (fontname[k] != NULL)
          strcpy(ftemp, fontname[k]);
        else
          *ftemp = '\0';
      }
      else
      {
        strcpy(ftemp, subfontname[k]);
      }

      if ((column += strlen(ftemp) + 1) > 76)
      {
        PSputs("\n%%+ font ", outfile);
        column = 9 + strlen(ftemp) + 1;
      }

      PSputs(ftemp, outfile);
      PSputc(' ', outfile);
    }
  }
  PSputc('\n', outfile);
  return;
}

/* work in progress in the above ... */

/* include remapped non-resident fonts without listing base fonts ? */

/* possibly also above ignore fonts substituted for */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* what type of page is it - device specific */

/* `statement' 5 1/2 x 8 1/2 */

/* a4tray, b5tray, legaltray, lettertray  - or setpageparams ? */

/* \special{paper_size=5.04in,3.75in} */

int decode_paper_size (char *paper_size, double *pagewidth, double *pageheight)
{
  double multiple;
  double width, height;
  char units[3];
  char *s=paper_size;
  int n;

  if (paper_size == NULL || *paper_size == '\0')
    return -1;

  if(sscanf(s, "%lg%n", &width, &n) > 0)
  {
    s +=  n;
    units[0] = *s++;
    units[1] = *s++;
    units[2] = '\0';
    multiple = decodeunits(units);
    width = width * multiple;
  }
  else
  {
    sprintf(logline, "Don't understand paper_size %s\n", paper_size);
    showline(logline, 1);
    return -1;
  }

  while (*s != ',' && *s != '*' && *s != '\0')
    s++;

  if (*s != '\0')
    s++;

  if(sscanf(s, "%lg%n", &height, &n) > 0)
  {
    s +=  n;
    units[0] = *s++;
    units[1] = *s++;
    units[2] = '\0';
    multiple = decodeunits(units);
    height = height * multiple;
  }
  else
  {
    sprintf(logline, "Don't understand paper_size %s\n", paper_size);
    showline(logline, 1);
    return -1;
  }

  *pagewidth = width;   /* in bp */
  *pageheight = height; /* in bp */

  if (traceflag)
  {
    sprintf(logline, "pagewidth %lg pageheight %lg\n", *pagewidth, *pageheight);
    showline(logline, 0);
  }

  return 0;
}

/* may need to do something here to tell printer what is going on also ? */
/* PS letter, note, legal, ledger ? */
/* use table for neatness */

int analpapertype (char *paper_type, double *pagewidth, double *pageheight)
{
  int k;

  if (paper_type == NULL)
    return 0;

  for (k= 0; k < 32; k++)
  {
    if (strcmp(paper_struct[k].type, "") == 0)
      break;

    if (strcmp(paper_struct[k].type, paper_type) == 0)
    {
      *pagewidth = paper_struct[k].width * 72;
      *pageheight = paper_struct[k].height * 72;
      return 0;
    }
  }

  for (k= 0; k < 32; k++)
  {
    if (strcmp(paper_struct[k].type, "") == 0)
      break;
/*  Try landscape versions ... (In which first letter upper case) 94/July/1 */
    if (_strcmpi(paper_struct[k].type, paper_type) == 0)
    {
      *pageheight = paper_struct[k].width * 72;
      *pagewidth = paper_struct[k].height * 72;
      return 0;
    }
  }
/*  new option to deal with large and peculiar sizes 1994/Dec/16 */
/*  -l=<width>*<height> where width and height are in PostScript points */
/*  if (sscanf(paper_type, "%lg*%lg", &pagewidth, &pageheight) == 2) { *//*NO*/
  if (sscanf(paper_type, "%lg*%lg", pagewidth, pageheight) == 2)
  {
    return 0;
  }

//  insert code to interpret 4in*5in e.g. ???
  if (decode_paper_size(paper_type, pagewidth, pageheight) == 0)
    return 0;
  sprintf(logline, "Don't understand paper_type: %s\n", paper_type);
  showline(logline, 1);
  errcount(0);

  return -1;
}

/* Sep 27 1990 => 1990 Sep 27 */

void scivilize (char *date)
{
  int k;
  char year[6];

  if (date == NULL)
    return;     /* sanity check */

  strcpy (year, date + 7);

  for (k = 5; k >= 0; k--)
    date[k + 5] = date[k];

/*  date[11] = '\0'; */
  for (k = 0; k < 4; k++)
    date[k] = year[k];

  date[4] = ' ';

  return;
}

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */
void lcivilize (char *date)
{
  int k;
  char year[6];

  if (date == NULL)
    return;     /* sanity check */

  strcpy (year, date + 20);

  for (k = 18; k >= 0; k--)
    date[k + 1] = date[k];

/*  date[20] = '\n'; */
/*  date[21] = '\0'; */
  date[20] = '\0';

  for (k = 0; k < 4; k++)
    date[k] = year[k];

  date[4] = ' ';

  return;
}

// include date if dateflag set, include serial if serialflag set

char *stampit (char *line, int dateflag)
{
  char *s=line;
  char date[11 + 1];

  strcpy(date, compiledate);
  scivilize(date);

//  following revised for Adobe Reader 4.0 so Creator shows URL
  strcpy(s, "DVIPSONE ");

  s += strlen(s);

  sprintf(s, "%d.%d.%d ", version, revision, subrevision);
  s += strlen(s);

  if (dateflag)
    sprintf(s, " %s %s", date, compiletime);
  s += strlen(s);

  if (! dateflag)
  {
    strcat(s, " ");
    strcat(s, URL);
  }
  s += strlen(s);

  return s;
}

//  Have to make copy of comma separated headerfile list 
//  because strtok messes it up, and the list is needed again later

void prolog_comments (char *headerfile, FILE *outfile) /* 1993/Nov/15 */
{
  char *s;
  char filename[MAXLINE];     /* need to work on copy since modify */

  if (headerfile == NULL)
    return;

  if (strlen(headerfile) >= sizeof(filename))
    return; // sanity check

  strcpy(filename, headerfile);
  s = strtok(filename, ",;");

  while (s != NULL) {
    PSputs("%%+ procset ", outfile);
    sprintf(logline, "%s %d %d\n", s, 1, 1);
    PSputs(logline, outfile);
    s = strtok(NULL, ",;");
  }
}

void copyDSCfile (FILE *output, char *name)
{
  FILE *input;
  int c, d=0;

  input = findepsfile(name, "dsc", 1, 1);

  if (input == NULL)
  {
    perrormod(name);
    PSputc('\n', output);
    return;
  }

  while ((c = getc(input)) != EOF) {
    PSputc(c, output);
    d = c;
  }

  if (d >= ' ')
  {
    PSputc('\n', output);
  }

  fclose(input);
}

void showownerout(char *);

void addescapes (char *sline, char *filename, int nlen) // puts result in line
{
  char *s = sline;
  char *t = filename;

  while (*t != '\0') {
    if (*t == '\\' || *t == '(' || *t == ')')
      *s++ = '\\';

    *s++ = *t++;

    if ((s - sline) >= nlen)
      break;   // 2000 July 12th
  }

  *s = '\0';
}

/* Write DSC header */
void writestruct (FILE *outfile)
{
  time_t ltime; /* for time and date */
  long pages;
  int missing;
  char *s;
  int k, c;
  double boundwidth, boundheight;
  int xll, yll, xur, yur;         /* 1995/Sep/16 */
  char *u;              /* 1995/July/15 */

  PSputs("%!PS-Adobe-3.0\n", outfile);

  if (stripcomment != 0)
    return;

  (void) time(&ltime);

  if (ltime < 0)
  {
    sprintf(logline, "ERROR: Time not available (%0ld)!\n", ltime);
    showline(logline, 1);
  }

  s = ctime(&ltime);

  if (s == NULL)
  {
    sprintf(logline, "ERROR: Cannot convert time (%0ld)!\n", ltime);
    showline(logline, 1);
    s = "Thu Jan 18 22:14:00 2038";
  }

  lcivilize(s); // date, used below

  PSputs("%%Title: ", outfile);

  if (filenamex != NULL)
  {
    addescapes(line, filenamex, sizeof(line));
    PSputs(line, outfile);
  }

  PSputc('\n', outfile);

  PSputs("%%Creator: ", outfile);
  stampit(line, 0);
  PSputs(line, outfile);
  PSputc('\n', outfile);

  PSputs("%%For: ", outfile);
  showownerout(line);
  strcat(line, "\n");
  PSputs(line, output);

  if (s != NULL)
  {
    PSputs("%%CreationDate: ", outfile);
    sprintf(logline, "%s\n", s);
    PSputs(logline, output);
  }

  PSputs("%%BoundingBox: ", outfile);

  if (BBxll != 0 || BByll != 0 || BBxur != 0 || BByur != 0)
  {
    /* should really convert from TeX coordinates to PS coordinates */
    xll = BBxll; yll = BByll;
    xur = BBxur; yur = BByur;
  }
  else
  {
    if (bounding_type == NULL)
    {
      xll = 72;
      yll = (int) ((double) pageheight - 72 - (unsigned long) (dvi_l / 65781));
      xur = (int) ((double) 72 + (unsigned long) (dvi_u / 65781));
      yur = (int) ((double) pageheight - 72);
    }
    else
    {
      boundheight = 11 * 72;  boundwidth = 8.5 * 72;    /* default */

      if (bounding_type != NULL)
        analpapertype(bounding_type, &boundwidth, &boundheight);

      xll = 0;
      yll = 0;
      xur = (int) boundwidth;
      yur = (int) boundheight;
    }
  }

  sprintf(logline, "%d %d %d %d\n", xll, yll, xur, yur);
  PSputs(logline, output);

  pages = numpages;

  if (collated)
    pages = pages * copies;

  if (nRepeatCount > 1)
    pages = pages * nRepeatCount;

  PSputs("%%Pages: ", outfile);
  sprintf(logline, "%ld\n", pages); 
  PSputs(logline, output);

  /*  %%PageOrder is highly optional ... */
  if (bOptionalDSC)
  {
    PSputs("%%PageOrder: ", outfile);
    sprintf(logline, "%s\n", reverseflag ? "Descend" : "Ascend");
    PSputs(logline, output);
  }

  PSputs("%%DocumentSuppliedResources: ", outfile);
  missing = writefontlist(outfile, 1, 29);

  PSputs("%%+ procset ", outfile);
  sprintf(logline, "%s %d %d\n", procsetrest, version, revision);
  PSputs(logline, output);

  if (needtpic != 0)
  {
    PSputs("%%+ procset ", outfile);
    sprintf(logline, "%s %d %d\n", tpicrest, version, revision);
    PSputs(logline, output);
  }

  for (k = 0; k < prologfileindex; k++)  /* 1993/Dec/21 */
  {
    prolog_comments (prologfile[k], outfile);
  }

  /*  should have version and revision? */
  if (headerfile != NULL)
    prolog_comments(headerfile, outfile);  /* 1993/Nov/15 */

  if (missing != 0) /* not reliable - may be non-zero yet nothing? */
  {
    PSputs("%%DocumentNeededResources: ", outfile);
    writemissing(outfile, 27);
  }

  /*  Extra DSC code supplied on command line using -*D=... */
  /*  If first character is not %, then convert all matching chars to space */
  /*  If --- after that --- first char is not %, insert %% before it */
  if (dsc_extra != NULL)
  {
    c = *dsc_extra; /* look at first character */

    if (c != '%')
    {
      s = dsc_extra;

      while (*(s+1) != '\0') {
        if (*(s+1) == c)
          *s = ' ';  /* replace magic with space */
        else
          *s = *(s+1); /* shift left one character */

        s++;
      }

      *s = '\0';            /* terminate */
      c = *dsc_extra;
    }

    if (c != '%')
    {
      PSputs("%%", output);
    }

    PSputs(dsc_extra, output);
    PSputc('\n', output);
  }

  /*  Extra DSC code supplied in \special{DSCtext=...} */
  /*  We are not checking whether lines start with %% and so on */
  if (dsccustom != NULL) /* 1995/July/15 */
  {
    u = dsccustom;

    while ((c = *u++) != '\0') {
      PSputc(c, output);
    }

    free(dsccustom);
    dsccustom = NULL;
    dsccustomlen = 0;
  }

/*  Extra DSC code supplied in \special{DSChead=...} 1995/July/15 */
  if (dscfile != NULL)
    copyDSCfile(output, dscfile);

  PSputs("%%Copyright: (C) 1990--2000, Y&Y, Inc.\n", output);
  PSputs("%%Copyright: (C) 2007, TeX Users Group.\n", output);
  PSputs("%%Copyright: (C) 2014, 2015, 2016 Clerk Ma.\n", output);
  PSputs("%%EndComments", outfile);    /* may omit if next line ... */
  PSputc('\n', outfile);
}

/* Made common routines to save strig space */
void dvidictbegin(FILE *outfile)
{
  PSputs("dvidict begin\n", outfile);
}

void dvidictend(FILE *outfile)
{
  PSputs("end", outfile);

  if (stripcomment == 0)
  {
    PSputs(" % dvidict\n", outfile);
  }
  else
  {
    PSputc('\n', outfile);
  }
}

void writeparams(FILE *outfile)
{
  if (stripcomment == 0)
  {
    PSputs("% Doc Setup\n", outfile);
    PSputs("% Transform\n", outfile);
  }

  dvidictbegin(outfile);
  sprintf(logline, "/num %lu def ", num); /* must use ! */
  PSputs(logline, output);

#ifdef ALLOWSCALE
  if (outscaleflag)
  {
    sprintf(logline, "/den %.9lg def ", (double) den / outscale);
  }
  else
  {
    sprintf(logline, "/den %lu def ", den);
  }

  PSputs(logline, output);
#else
  {
    sprintf(logline, "/den %lu def ", den); /* must use ! */
    PSputs(logline, output);
  }
#endif

  if (verboseflag)
  {
    if (num != 25400000 || den != 473628672)
    {
      sprintf(logline, "Unusual DVI units: %lu/%lu per 0.1 micrometer\n",
        num, den);
      showline(logline, 0);
    }
  }

  sprintf(logline, "/mag %lu def\n", mag); /* TeX magnification * 1000 */
  PSputs(logline, output);

  /*  now for user specified transformations: */
  if (evenoddoff == 0)  /* only if the same on even and odd pages */
    sprintf(logline, "/xoffset %lg def /yoffset %lg def\n", xoffset, yoffset);

  PSputs(logline, output);
  sprintf(logline, "/xmagnif %lg def /ymagnif %lg def /rotation %lg def\n",
    xmagnification, ymagnification, rotation);
  PSputs(logline, output);
  sprintf(logline, "/PageHeight %lg def /PageWidth %lg def\n",
    pageheight, pagewidth);
  PSputs(logline, output);

  if (bBindFlag)
  {
    PSputs("/bindflag true def\n", output);
  }

  if (bCheckFonts != 0)
  {
    PSputs("/checkfonts true def\n", output);
  }
  else /* following is redundant - since it is already set to false in preamble .. */
  {
    PSputs("/checkfonts false def\n", output);
  }

  if (bLandScape != 0)
  {
    PSputs("/landscape true def\n", output);    /* fixed 95/Aug/12 */
  }

  if (copies != 1 && collated == 0)
  {
    sprintf(logline, "/#copies %d def\n", copies);  /* inside Setup */
    PSputs(logline, output);
  }

  if (bCheckVersion & 1)
  {
    sprintf(logline, "%d %d %d checkversion\n",
      version, revision, subrevision);    /* 1993/Nov/3 */
    PSputs(logline, output);
  }

  /*  Does user want to explicitly set screen frequency ? 94/March/18 */
  if (frequency >= 0)
  {
    if (frequency > 0)  /* did user specified frequency & angle ? */
    {
      if (freqrelation != 0)  /* specified limiting frequency ? */
      {
        PSputs("currentscreen pop pop", output);
        sprintf(logline, " %d 2 copy %s{exch}if pop %d",
          frequency, (freqrelation > 0) ? "lt" : "gt", angle);
      }
      else
      {
        sprintf(logline, "%d %d", frequency, angle);
      }
      PSputs(logline, output);
    }
    else
    {
      PSputs("currentscreen pop", output); /* frequency == 0 */
    }
    PSputs(" dviscreen\n", output);
  }

  dvidictend (outfile);
}

// checkflag  --- check version number of preamble file

// returns -1 if it fails

int copy_preamble (FILE *output, FILE *input, char *procsetfile, int checkflag)
{
  int c, d, column;
  int goodflag = 0;         /* 93/Mar/31 */
  unsigned int k, length;
  int preversion = 1, prerevision = 2, presubrevision = 0;
  char *s;

  if ((bCheckVersion & 2) && checkflag)         /* 1995/May/30 */
  {
    while ((c = getc(input)) != EOF && c != '%') ;  /* find first line */
    while ((c = getc(input)) != EOF && c >= ' ') ;  /* skip to end line */
    while ((c = getc(input)) != EOF && c < ' ') ; /* skip to next line */
    s = line;
    *s++ = (char) c;

    while ((c = getc(input)) != EOF && c >= ' ' && s < line + sizeof(line))
      *s++ = (char) c;              /* read the line */

    *s++ = '\n'; *s = '\0';

    if ((s = strstr(line, "version")) != NULL)
    {
      if (sscanf (s + 7, "%d.%d.%d", &preversion, &prerevision, &presubrevision) == 3)
      {
        if (preversion == version && prerevision == revision && presubrevision == subrevision)
        {
          goodflag = 1;       /* 95/Mar/31 */

          if (traceflag)        /* debug only */
          {
            sprintf(logline, "VERSION OK (%s)\n", procsetfile);
            showline(logline, 0);
          }

          if (distillerlogflag)   // made optional 2000 Aug 15
          {
            strcpy(logline, "(%%[ Preamble Version: ");
            s = logline + strlen(logline);
            sprintf(s, "%d.%d.%d", preversion, prerevision, presubrevision);
            strcat(logline, " ]%%) = flush % for Distiller log\n");
            PSputs(logline, output);
          }
        }
      }
    }

    if (goodflag == 0)
    {
      sprintf(logline, "ERROR: Incorrect preamble version (%s) need %s:\n",
           procsetfile, progversion);
      showline(logline, 1);
      showline(line, 1);          /* line read from file */
      checkexit(1);
      //return -1;              // failed
    }
    rewind (input);
  }

  c = getc(input);
  ungetc(c, input);
  if (c != 128)           /* plain ASCII format */
  {
    if (fgets(line, MAXLINE, input) != NULL)
    {
      if (*line != '%')
      {
        PSputs(line, output);   /* flush copyright */
      }

      while (fgets(line, MAXLINE, input) != NULL) {
        if ((s = strchr(line, '\r')) != NULL)
        {
          if (*(s+1) == '\n')
            strcpy(s, s+1);
        }

        if (stripcomment == 0 || *line != '%')
        {
          PSputs(line, output);
        }

        if (bAbort)
          abortjob();     /* 92/Nov/24 */

        if (abortflag)
          return -1;
      }
      if (strchr(line, '\n') == NULL)
      {
        PSputc('\n', output);
      }
    }
    return 0;
  }

  while ((c = getc(input)) == 128) {      /* PFB style format */
    c = getc(input); /* get section code */
    /* assumes ASCII and binary sections no longer than 65535 bytes */
    if (c == 3)
      return 0; /* end of data */

    length = (unsigned int) readlength(input);

    if (c == 1) /* ASCII */
    {
      for (k = 0; k < length; k++)
      {
        c = getc(input);
        if (c == '\r')
          c = '\n';  /* ??? */
        PSputc(c, output);
      }
    }
    else
    {
      if (c == 2)   /* binary */
      {
        column = 0;

        for (k = 0; k < length; k++)
        {
          if (column >= 39)     /* 78 columns */
          {
            PSputc('\n', output);
            column = 0;
          }

          c = getc(input);
          d = c & 15;
          c = c >> 4;

          if (c > 9)
            c = c + 'A' - 10;
          else
            c = c + '0';
          PSputc(c, output);

          if (d > 9)
            d = d + 'A' - 10;
          else
            d = d + '0';
          PSputc(d, output);
          column++;
        }
        PSputc('\n', output);
      }
      else
        break;             /* invalid code */
    }
    if (bAbort)
      abortjob();     /* 1992/Nov/24 */

    if (abortflag)
      return -1;
  }
  sprintf(logline, "ERROR: Corrupted Preamble %s\n", procsetfile); /* ??? */
  showline(logline, 1);
  checkexit(1);
  return -1;
}

void expand_separators (char *line)
{
  char *s = line;

  while ((s = strchr(s, '\\')) != NULL)
  {
    memmove(s+1, s, strlen(s)+1); // "\" => "\\"
    s += 2;
  }
}

// checkflag check version number of preamble

// returns -1 if error

int writepreamble (FILE *outfile, char *procsetfile, char *procsetrest, int checkflag)
{
  FILE *infile;
  int ret = 0;

  if (traceflag)
    printf("Processing %s (%s)\n", procsetfile, procsetrest);

  if (stripcomment == 0)
    PSputs("%%BeginResource: ", outfile);

  if (stripcomment == 0)
  {
    sprintf(logline, "procset %s %d %d\n", procsetrest, version, revision);
    PSputs(logline, output);
  }

  infile = fopen(procsetfile, "rb");

  if (input == NULL)
  {
    sprintf(logline, "ERROR: Can't find preamble file `%s'\n", procsetfile);
    showline(logline, 1);
    perrormod(procsetfile);
    errcount(0);      /* pretty serious error ? */
    ret = -1;
  }
  else
  {
    if (distillerlogflag)  // made optional 2000 Aug 15
    {
      strcpy(logline, "(%%[ Reading: ");
      strcat(logline, procsetfile);
      expand_separators(logline);
      strcat(logline, " ]%%) = flush % for Distiller log\n");
      PSputs(logline, outfile);
    }
    ret = copy_preamble(outfile, infile, procsetfile, checkflag);
    fclose(infile);
    if (abortflag) ret = -1;
  }

  if (stripcomment == 0)
    PSputs("%%EndResource\n", outfile);

  return ret;
}

void copyprologfilesub (char *filename, FILE *outfile)
{
  FILE * infile;

  if (verboseflag)
  {
    showline("[Header", 0);
  }

  if ((infile = findepsfile(filename, "ps", 1, 0)) == NULL)
  {
  }
  else
  {
    if (stripcomment == 0)
    {
      PSputs("%%BeginResource: ", outfile);
      sprintf(logline, "procset %s\n", filename);   /* should have version and revision? */
      PSputs(logline, output);
    }

    dvidictbegin(outfile);
    copyepssimple(outfile, infile);       /* 1993/Jan/24 */
    fclose(infile);
    PSputc('\n', outfile);
    dvidictend (outfile);

    if (stripcomment == 0)
      PSputs("%%EndResource\n", outfile);
  }

  if (verboseflag)
    showline("] ", 0);
}

/* Can handle comma separated list of prolog file names */
/* This DOES mess up the `filename' list passed, but it won't be used again */

void copyprologfiles(char *filenames, FILE *outfile) /* 1993/Nov/15 */
{
  char *s;

  if (filenames == NULL)
    return;

  s = strtok(filenames, ",;");
  while (s != NULL) {
    copyprologfilesub(s, outfile);
    s = strtok(NULL, ",;");
  }
}

int tryencandps(char *restofname)
{
  FILE *infile;

  strcat(procsetfile, restofname);
  extension(procsetfile, "enc");

  if (traceflag)
  {
    sprintf(logline, "Trying: %s\n", procsetfile);
    showline(logline, 0);
  }

  infile = fopen(procsetfile, "r");

  if (infile != NULL)
  {
    fclose(infile);
    return 0;
  }

  forceexten(procsetfile, "ps");

  if (traceflag)
  {
    sprintf(logline, "Trying: %s\n", procsetfile);
    showline(logline, 0);
  }

  infile = fopen(procsetfile, "r");

  if (infile != NULL)
  {
    fclose(infile);
    return 0;
  }

  return -1;      // failed
}

//  First check whether path specified - if so use directly
//  - then look for header file in dvi file directory
//  - then look in default header file directory
//  - then look in PREPATH directory
//  - then look in dvipsone directory
//  within each directory, look for "foo.enc", then "foo.ps"
//  leaves successful file name in procsetfile
//  returns non-zero if it fails

/* figure out procsetfile */
int setupprocset (char *procsetrest)
{
  FILE *infile;
  char *s;

  //  if explicit path given use that
  if (strpbrk(procsetrest, "\\/:") != NULL)
  {
    strcpy(procsetfile, procsetrest);
    
    infile = fopen(procsetfile, "r");

    if (infile != NULL)
    {
      fclose(infile);
      return 0;
    }
  }
  else
  {
    if (dvipath != NULL)
      strcpy(procsetfile, dvipath); /* try dvi file path first */
    else
      strcpy(procsetfile, "");

    if (*procsetfile != '\0')      /* 1992/Oct/30 */
    {
      s = procsetfile + strlen(procsetfile) - 1;
      if (*s != ':' && *s != '\\' && *s != '/')
        strcat(procsetfile, "\\"); 
    }

    if (tryencandps(procsetrest) == 0)
      return 0;

    strcpy(procsetfile, procsetpath); /* now try default path */
    strcat(procsetfile, "\\");

    if (tryencandps(procsetrest) == 0)
      return 0;

    if (programpath != NULL)
    {
      strcpy(procsetfile, programpath); /* now try default path */
      strcat(procsetfile, "\\");
      if (tryencandps(procsetrest) == 0)
        return 0;
    }
  }
  return -1;  // failed
}

/* the following needs more work ... structuring conventions and such */
/* also no coalesced mostly into on preamble => make file ? */

int writeheader (FILE *outfile)
{
  int c, k;
  char *u;              /* 1993/Dec/29 */
  int column = 0;
  int nesting = 0;          /* 1999/Feb/28 */
  int escape = 0;           /* 1999/Feb/28 */

  sprintf(logline, "/dvidict %d dict def\n", dvidictsize + fnext);
  PSputs(logline, output);

  if (setupprocset(procsetrest) != 0)
  {
    sprintf(logline, "ERROR: Problem with %s\n", procsetrest);
    showline(logline, 1);
    return -1;
  }

  if (writepreamble(outfile, procsetfile, procsetrest, 1) != 0)
  {
    sprintf(logline, "ERROR: Problem with %s\n", procsetfile);
    showline(logline, 1);
    return -1;
  }

  if (abortflag)
    return -1;

/*  copy TPIC preamble file */
  if (needtpic != 0)
  {
    if (setupprocset(tpicrest) != 0)
    {
      sprintf(logline, "ERROR: Problem with %s\n", tpicrest);
      showline(logline, 1);
      return -1;
    }

    if (writepreamble(output, procsetfile, tpicrest, 0) != 0)
    {
      sprintf(logline, "ERROR: Problem with %s\n", procsetfile);
      showline(logline, 1);
      return -1;
    }

    if (abortflag)
      return -1;
  }

/*  copy user specified prolog file */
/*  warn about name conflicts ? */
/*  search for it in various directories ? */
/*  add to list in structure comments ? */
/*  if (strcmp(prologfile, "") != 0) copyprologfiles(prologfile, output); */
/*  first do the ones stipulated on the command line */
  for (k = 0; k < prologfileindex; k++)       /* 1993/Dec/21 */
  {
    copyprologfiles(prologfile[k], output);
    if (abortflag)
      return -1;
  }

/*  then do the ones specified in \specials */
  if (headerfile != NULL)
  {
    copyprologfiles(headerfile, output);
    if (abortflag)
      return -1;
  }

  if (headertext != NULL) /* verbatim headers 1993/Dec/29 */
  {
    dvidictbegin(outfile);
    if (bWrapSpecial == 0) // just copy it across
    {
      PSputs(headertext, output);
    }
    else
    {
      column = 0;
      u = headertext;
      c = *u++;     /* do something different if this is  " ? */
/* The following wraps lines to avoid problems in PS output from \special */
/* But it does run the danger of putting a line break in a string ... */
      while (c != '\0') {     /* 1994/June/27 */
        PSputc(c, output);
        c = *u++;
/* crude effort to prevent exessively long lines - wrap line at space */
        if (c == '\n')
          column = 0;
        else
          column++;

        if (escape == 0)
        {
          if (c == '(')
            nesting++;
          else
          {
            if (c == ')')
              nesting--;
            else
            {
              if (c == '\\')
                escape++;
            }
          }
        }
        else
          escape = 0;

        if (nesting == 0 && column > WRAPCOLUMN && c == ' ')
        {
          c = '\n';
          column = 0;
        }
      }
    }

    dvidictend(outfile);
    free(headertext);
    headertext = NULL;
    headertextlen = 0;
  }

  return 0;
}

/* should following be somewhat conditional on bAllowShortEncode ? */
/* after all, the vectors are not referred to if bAllowShortEncode == 0 */
/* except possibly tex_text is needed for something or other */

void writeencodingvecs (FILE *outfile)  /* separated from above 94/Mar/3 */
{
/*  int k; */
/*  NOTE: following wrapped in begin/end on `dvidict' */
/*  dvidictbegin(outfile); */
  if (busedviencode != 0)
    write_dvi_encode(outfile);  /*  NOT in dvipream */
  else
  {
    if (bAllowTexText != 0)
      writetextext(outfile);
  }

  if (bWindowsFlag != 0)
  {
    if (strcmp(textencoding, "ansinew") != 0)
    {
      read_text_encode(textencoding); /* user selected ENCODING 94/Dec/17*/
    }

    textenconame = removepath(textencoding);        /* 95/Feb/3 */
    nCheckSum = codefourty(textenconame);         /* 95/Feb/3 */

    if (bAllowANSI)
      write_ansi_code(outfile, textenconame); /* 95/Feb/3 */
  }
/*  maybe make conditional on bAllowANSI ? */
/*  dvidictend(outfile); */
}

void writetrailer (FILE *outfile)
{
  if (stripcomment == 0)
  {
    PSputs("%%Trailer\n", outfile);
    PSputs("%%EOF\n", outfile);
  }

  if (wantcontrold || (directprint && stripcomment))    /* 1993/Mar/5 */
    PSputc(4, outfile); /* C-D */
}

// The decrypted owner string may contain \xyz to indicate char <xyz>
// Or it may contain DOS 850 accented characters

void showownerout (char *oline)
{
  char *s = oline;

  *s = '\0';
  strcpy(s, getenv("TEXAUTH"));
}

double roundtime(long, long);

int expandpageranges (void)
{
  int oldmaxranges = maxranges;
  int k;

  if (maxranges == 0)
    maxranges = MAXRANGES;  // first time
  else
    maxranges = maxranges * 2;       // expansion

  beginpages = (long *) realloc(beginpages, maxranges * sizeof(long));
  endpages = (long *) realloc(endpages, maxranges * sizeof(long));
  pagerangeseq = (int *) realloc(pagerangeseq, maxranges * sizeof(long));

  if (beginpages == NULL || endpages == NULL || pagerangeseq == NULL)
  {
    sprintf(logline, " ERROR: unable to allocate memory for %d page ranges\n", maxranges);
    showline(logline, 1);
    maxranges = oldmaxranges;
    return -1;
  }

  for (k = oldmaxranges; k < maxranges; k++)
  {
    beginpages[k] = -LINFINITY;
    endpages[k] = LINFINITY;
    pagerangeseq[k] = -1;
  }

  return 0;
}

/* present piggybacks on 'p' (PFM file path) flag */
/* presently only supports physical page ranges (countzeroflag = 0) */
/* pages output in order they appear in DVI file, *not* in page-range list */
/* so it works somewhat differently from changed.c */
/* adds to page ranges already established using -B= -E= etc */

char *delimiters = ",.;:+_|~^/#!*"; /* imported from changed.c */

int newpageranges (char *pages) /* 1994/July/6 */
{
  char *s;
  int page, bpage, epage, temp;

  beginorend = -1;        /* matched begin / end pair */
  s = strtok (pages, delimiters);

  while (s != NULL) {
    if (rangeindex >= maxranges)
    {
      if (expandpageranges() < 0)
        break;
    }

    if (rangeindex >= maxranges)
    {
      sprintf(logline, " ERROR: Too many page ranges (> %d): %s\n",
          maxranges, s);
      showline(logline, 1);
      break;
    }

    if (sscanf(s, "%d-%d", &bpage, &epage) == 2)
    {
      if (bpage > epage)
      {
        temp = epage;
        epage = bpage;
        bpage = temp;
      }

      beginpages[rangeindex] = (long) bpage;
      endpages[rangeindex] = (long) epage;
      pagerangeseq[rangeindex] = -1;
      rangeindex++;
    }
    else
    {
      if (sscanf(s, "%d", &page) == 1)
      {
        beginpages[rangeindex] = (long) page;
        endpages[rangeindex] = (long) page;
        pagerangeseq[rangeindex] = -1;
        rangeindex++;
      }
      else
      {
        sprintf(logline, " Error in page range: %s", s);
        showline(logline, 1);
      }
    }

    s = strtok (NULL, delimiters);
  }

  return rangeindex;
}

/* save some space by making this common 1992/Nov/17 */

void complaincommand (char *command, char *s)
{
  sprintf(logline, "Don't understand: %s ", command);

  if (s != NULL && strlen(s) < sizeof(logline) - strlen(logline))
    strcat(logline, s);
  showline(logline, 1);
  showline("\n", 0);
}

/* Flags and Arguments start with `-' */
/* Also allow use of `/' for convenience */
/* Normal use of `=' for command line arguments */
/* Also allow use of `:' for convenience */
/* Archaic: use space to separate - only for backward compatability */

int decodearg (char *command, char *next, int firstarg)
{
  char *s;
  char *sarg = command;
  int c, n, flag;
  int instance;
  int toggle;
  int temp;
  
  if (*sarg == '-' || *sarg == '/')
    sarg++; /* step over `-' or `/' */

  if (*sarg == '\0')
    return firstarg+1;   // 2000/April/4

  while ((c = *sarg++) != '\0') {       /* until end of string */
    toggle = 0;               /* toggle state */

    if (c == '0')
    {
      c = *sarg++; toggle = -1;     /* force off */
    }
    else
    {
      if (c == '1')
      {
        c = *sarg++;
        toggle = +1;     /* force on */
      }
    }

    if (c == '*')
      c = *sarg++ + 128;    /* escape 94/Mar/4 */
    else
    {
      if (c == '$') /* new escape 96/Apr/4 */
      {
        c = *sarg++;

        if (c >= 96 && c < 128)
          c = c + 128 - 96; /* use 128 -- 159 */
        else
        {
          if (c >= 64 && c < 96)
            c = c + 0 - 64; /* use 0 -- 31 */
          else
          {
            complaincommand(command, NULL);
            return firstarg;
          }
        }
      }
    }

    flag = decodeflag(c, toggle);

    if (flag < 0)
      return -1;

    if (flag > 0)   /* flag requires argument ? */
    {
      if (*sarg != '=' && *sarg != ':' && *sarg != '#') /* 93/Sep/8 */
      {
        if (next != NULL)
        {
          firstarg++; s = next; /* when `=' or `:' is NOT used */
        }
        else
        {
          complaincommand(command, NULL);
          return firstarg;
        }
      }
      else
        s = sarg+1;  /* when `=' or `:' or `#' IS used */

/* now analyze the various flags that could have gotten set */
      if (beginflag != 0)
      {
        instance=-1;                /* 1994/Jan/16 */

        if (sscanf(s, "%ld:%d", &beginpage, &instance) < 1)
          complaincommand(command, s);
        else
        {
          if (beginorend > 0)
          {
            showline(" WARNING: begin follows begin\n", 1);
          }
          else
          {
            beginorend = 1;

            if (countzeroflag == 0)
              instance = -1;

            if (rangeindex >= maxranges)
            {
              if (expandpageranges() < 0) break;
            }

            if (rangeindex >= maxranges)
            {
              showline(" WARNING: too many begins\n", 1);
            }
            else
            {
              if (rangeindex < maxranges)
              {
                pagerangeseq[rangeindex] = instance;
                beginpages[rangeindex] = (long) beginpage;
              }
            }
          }
        }
        beginflag = 0;
      }
      else if (endflag != 0)
      {
        instance=-1;

        if (sscanf(s, "%ld:%d", &endpage, &instance) < 1)
          complaincommand(command, s);
        else if (beginorend < 0)
        {
          showline(" WARNING: end follows end\n", 1);
        }
        else
        {
          beginorend = -1;

          if (countzeroflag == 0)
            instance = -1;

          if (rangeindex >= maxranges)
          {
            if (expandpageranges() < 0)
              break;
          }

          if (rangeindex >= maxranges)
          {
            showline(" WARNING: too many ends\n", 1);
          }
          else if (rangeindex < maxranges)
          {
            if (instance >= 0)
              pagerangeseq[rangeindex] = instance;

            endpages[rangeindex++] = (long) endpage;
          }
        }
        endflag = 0;
      }
      else if (copiesflag != 0)
      {
        if (sscanf(s, "%d", &copies) < 1)
        {
          complaincommand(command, s);
        }
        copiesflag = 0;
      }
      else if (magniflag != 0)
      {
        if(sscanf(s, "%lg", &magnification) < 1)
        {
          complaincommand(command, s);
        }

        if (magnification > 33.33)  /* 1992/July/16 */
          magnification =  magnification/1000.0;

        if (xmagnification == 0.0)
          xmagnification = magnification;
        else if (ymagnification == 0.0)
          ymagnification = magnification;
        else
        {
          showline("Use `m' no more than twice\n", 1);
          complaincommand(command, s);
        }
        magniflag = 0;
      }
      else if (rotateflag != 0)
      {
        if(sscanf(s, "%lg", &rotation) < 1)
        {
          complaincommand(command, s);
        }
        rotateflag = 0;
      }
      else if (xoffsetflag != 0)
      {
        if(sscanf(s, "%lg", &xoffset) < 1)
        {
          complaincommand(command, s);
        }

        if (xoffsete != UNKNOWNOFFSET)
        {
          if (xoffseto != UNKNOWNOFFSET)
          {
            sprintf(logline, "Too many %s offsets\n", "x");
            showline(logline, 1);
          }
          else
            xoffseto= xoffset;
        }
        else
          xoffsete= xoffset;
        xoffsetflag = 0;
      }
      else if (yoffsetflag != 0)
      {
        if(sscanf(s, "%lg", &yoffset) < 1)
        {
          complaincommand(command, s);
        }

        if (yoffsete != UNKNOWNOFFSET)
        {
          if (yoffseto != UNKNOWNOFFSET)
          {
            sprintf(logline, "Too many %s offsets\n", "y");
            showline(logline, 1);
          }
          else
            yoffseto= yoffset;
        }
        else
          yoffsete= yoffset;
        yoffsetflag = 0;
      }
      else if (dvidictflag != 0)
      {
        if (sscanf(s, "%d", &dvidictsize) < 1)
          complaincommand(command, s);

        dvidictflag = 0;
      }
      else if (versionflag != 0)
      {
        if (sscanf(s, "%d", &bCheckVersion) < 1)
          complaincommand(command, s);

        versionflag = 0;
      }
      else if (repeatflag != 0)
      {
        if (sscanf(s, "%d", &nRepeatCount) < 1)
          complaincommand(command, s);

        if (nRepeatCount < 0)
          nRepeatCount = - nRepeatCount;

        repeatflag = 0;
      }
      else if (minruleflag != 0)
      {
        if (sscanf(s, "%ld", &nMinRule) < 1)
          complaincommand(command, s);

        minruleflag = 0;
      }
      else if (defdpiflag != 0)
      {
        if (sscanf(s, "%d", &nDefaultTIFFDPI) < 1)
          complaincommand(command, s);

        defdpiflag = 0;
      }
      else if (emeraldflag != 0)
      {
        if (sscanf(s, "%lg", &outscale) < 1)
        {
          complaincommand(command, s);
        }
        else
          outscaleflag=1;

        emeraldflag = 0;
      }
      else if (halftoneflag != 0)
      {
        n = 0;
        if (sscanf(s, "%d%c%d", &frequency, &n, &angle) < 1)
        {
          complaincommand(command, s);
        }
        else
        {
          if (n == '+')
            freqrelation = +1; /* min frequency */
          else if (n == '-')
            freqrelation = -1; /* max frequency */
          else
            freqrelation = 0;  /* force frequen */
        }
        halftoneflag = 0;
      }
      else if (pathflag != 0)
      {
        fontpath = s;
        pathflag = 0;
      }
      else if (subfileflag != 0)
      {
        if (subfontfileindex == MAXSUBFILES)
        {
          sprintf(logline, "Too many %s files\n", "substitution");
          showline(logline, 1);
        }
        else
          subfontfile[subfontfileindex++] = s;
        subfileflag = 0;
      }
      else if (paperflag != 0)
      {
        paper_type = s;
        paperflag = 0;
      }
      else if (boundingflag != 0)
      {
        bounding_type = s;
        boundingflag = 0;
      }
      else if (dscextraflag != 0)
      {
        dsc_extra = s;
        dscextraflag = 0;
      }
      else if (afmflag != 0)
      {
        afmpath = s;
        afmflag = 0;
      }
      else if (tfmflag != 0)
      {
        tfmpath = s;
        tfmflag = 0;
      }
      else if (pfmflag != 0)
      {
        if (sscanf(s, "%d", &temp) == 1)
          newpageranges(s);
        else
          pfmpath = s;

        pfmflag = 0;
      }
      else if (vecpathflag != 0)
      {
        vecpath = s;
        vecpathflag = 0;
      }
      else if (subpathflag != 0)
      {
        fontsubpath = s;
        subpathflag = 0;
      }
      else if (prepathflag != 0)
      {
        procsetpath = s;
        prepathflag = 0;
      }
      else if (encodeflag != 0)
      {
        textencoding = s;

        if ((s = strrchr(textencoding, '.')) != NULL)
          *s = '\0';

        bWindowsFlag = 1;
        encodeflag = 0;
      }
      else if (outputflag != 0)
      {
        outputfile = s;
        outputflag = 0;
      }
      else if (epsflag != 0)
      {
        epspath = s;
        epsflag = 0;
      }
      else if (prologflag != 0)
      {
        if (prologfileindex == MAXPROLOGS)
        {
          sprintf(logline, "Too many %s files\n", "header");
          showline(logline, 1);
        }
        else
          prologfile[prologfileindex++] = s;

        prologflag = 0;
      }
      else if (prefixflag != 0)
      {
        fontprefix = s;
        prefixflag = 0;
      }
      break;  /* default - no flag set */
    }
  }
  return firstarg;
}

/*  check command line flags and command line arguments */

int commandline (int argc, char *argv[], int firstarg)
{
  int c, flag;
  char *s;

  if (argc < firstarg + 1)
  {
    return -1;        // 2000 June 21
  }

  c = argv[firstarg][0];

  while (c == '-' || c == '/' || c == '@') {  /* 1993/Sep/7 */
    if (c == '@')
    {
      s = argv[firstarg]+1;
      flag = readcommands(s);

      if (flag < 0)
        return -1;

      if (flag == 0)
      {
        sprintf(logline, " ERROR: Cannot find command file %s\n", s);
        showline(logline, 1);
        errcount(0);
      }

      firstarg++;
    }
    else
    {
      /* - or / */
      firstarg = decodearg(argv[firstarg], argv[firstarg+1], firstarg+1);

      if (firstarg < 0)
        return -1;
    }

    if (firstarg >= argc)
      break;      /* safety valve */

    c = argv[firstarg][0];
  }
  return firstarg;
}

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath (char *pathname)
{
  char *s;

  if ((s=strrchr(pathname, '\\')) != NULL) s++;
  else if ((s=strrchr(pathname, '/')) != NULL) s++;
  else if ((s=strrchr(pathname, ':')) != NULL) s++;
  else s = pathname;
  return s;
}

/* remove file name - keep only path - inserts '\0' to terminate */

void stripname (char *pathname)
{
  char *s;

  if ((s=strrchr(pathname, '\\')) != NULL) ;
  else if ((s=strrchr(pathname, '/')) != NULL) ;
  else if ((s=strrchr(pathname, ':')) != NULL) s++;
  else s = pathname;

  *s = '\0';
}

void replaceletter (char *s, int old, int new)
{
  int c;

  while ((c = *s) != '\0') {
    if (c == old)
      *s = (char) new;
    s++;
  }
}

void lowercase (char *t, char *s)
{
  int c;

  while ((c = *s++) != '\0') {
    if (c >= 'A' && c <= 'Z')
      *t++ = (char) (c + ('a' - 'A'));
    else
      *t++ = (char) c;
  }
  *t = '\0';
}

void uppercase (char *t, char *s)
{
  int c;

  while ((c = *s++) != '\0') {
    if (c >= 'a' && c <= 'z')
      *t++ = (char) (c - ('a' - 'A'));
    else
      *t++ = (char) c;
  }
  *t = '\0';
}

#undef SIG_IGN
#define SIG_IGN (void (__cdecl *)(int))1L

#ifdef CONTROLBREAK
// void __cdecl ctrlbreak(int err) {
void ctrlbreak(int err)
{
/*  by the way: err should be SIGINT at this point --- err never used */
  (void) signal(SIGINT, SIG_IGN);     /* disallow control-C */

  printf("\nSIG: %d.\n", err);

  if (bAbort++ >= 3)
    exit(3);       /* emergency exit */

  (void) signal(SIGINT, ctrlbreak);   /* reenable */
}
#endif

char *dvipsonecmd = "DVIPSONE.CMD";   /* name of command line file */

/* Try and read default command file - DVIPSONE.CMD */
/* in current directory and then in directory of DVIPSONE */
/* (does not make sense in DVI file directory) */
/* since we don't yet know where that is ! */
/* can't conveniently include this in output file either - not open yet */

/* used both for dvipsone.cmd and @ indirect command line files */
/* can this be reentered ? */

char commandfile[FNAMELEN];   /* keep around so can open later */

int scancommands (char *line)
{
  char *s, *sn;
  int flag;
  
  s = strtok(line, " \t\n\r");        /* first token */

  while (s != NULL) {
    sn = strtok(NULL, " \t\n\r");     /* go to next token */

    if (*s == '-')
    {
      /* is it a command line argument or flag ? */
      flag = decodearg(s, sn, 0);

      if (flag < 0)
        return -1;

      if (flag > 0)
      {
        sn = strtok(NULL, " \t"); /* go to next token */
      }
    }
    s = sn;
  }
/*    If you want to see command lines in file - put -v in the file */
  return 0;
}

/* 1993/Nov/15 need to make copy of command line in local memory ! */

int readcommands (char *filename)
{
/*  char commandfile[FNAMELEN]; */
  FILE *command;
  char line[MAXLINE];   /* why not use global ? reentry ? */
  char *linedup;      /* need to copy line to preserve args */
//  char *s, *sn;

/*  Try first in current directory (or use full name as specified) */
  strcpy(commandfile, filename);
  extension(commandfile, "cmd");
  command = fopen(commandfile, "r");

  if (command == NULL)
  {
/*    If that fails, try in DVIPSONE program directory */
    if (programpath != NULL)
      strncpy(commandfile, programpath, MAXPATHLEN);
    else
      strcpy(commandfile, "");

    strcat(commandfile, "\\");
    strcat(commandfile, filename);
    extension(commandfile, "cmd");

    command = fopen(commandfile, "r");

    if (command == NULL)
    {
      strcpy(commandfile, "");  /* indicate failed - 94/Mar/4 */
      return 0;       /* no command line file DVIPSONE.CMD */
    }
  }

/*  allow for multiple lines --- ignore args that don't start with `-' */
  while (fgets(line, MAXLINE, command) != NULL) {
    if (*line == '%' || *line == ';' || *line == '\n')
      continue;

    if (strchr(line, '\n') == NULL) strcat(line, "\n");

    linedup = zstrdup(line);

    if (linedup != NULL)
    {
      if (scancommands(linedup) < 0) return -1; // failure
    }
  }
  fclose(command);    /* no longer needed */
  return 1;       // success
}

/* new 1992/Dec/22 */

/* flag is zero for on-screen output, non-zero for PS file output */

void showcommand (FILE *output, int argc, char *argv[], int flag)
{
  int k;
  FILE *command;
  int cmdflag = 0;
  char *s;

/*  Following added 94/Mar/4 - show dvipsone.cmd */
  if (strcmp(commandfile, "") != 0)      /*  command file ? */
  {
    command = fopen(commandfile, "r");

    if (command != NULL)
    {
      while (fgets(line, MAXLINE, command) != NULL) {
        if (*line == '%' || *line == ';' || *line == '\n')
          continue;

        if (strchr(line, '\n') == NULL) strcat(line, "\n");

        if (cmdflag++ == 0) /* first time only */
        {
          if (flag)
            strcpy(logline, "% ");
          else
            *logline = '\0';

          strcat(logline, "From ");
          strcat(logline, commandfile);
          strcat(logline, "\n");

          if (flag)
            PSputs(logline, output);
          else
            showline(logline, 0);
        }

        if (flag)
          strcpy(logline, "% ");
        else
          *logline = '\0';

        if (strlen(line) + 3 < sizeof(logline)) /* MAXLINE */
          strcat(logline, line);

        if (flag)
          PSputs(logline, output);
        else
          showline(logline, 0);
      }
      fclose(command);
    }
    else
    {
      sprintf(logline, "ERROR: Can't open commandfile %s\n", commandfile);
      showline(logline, 1);
      perrormod(commandfile);
    }
  }

/*  Now for the command line itself */
  if (flag)
    strcpy(logline, "% ");
  else
    *logline = '\0';
  strcat(logline, "Command:\n");

  if (flag)
    PSputs(logline, output);  // PS output stream
  else
    showline(logline, 0);      // on screen

  if (flag)
    strcpy(logline, "% ");
  else
    *logline = '\0';

  s = logline;

  for (k = 0; k < argc; k++)
  {
    if (s - logline + strlen(argv[k] + 3) >= sizeof(logline))   /* MAXLINE */
      break;

    strcat(s, argv[k]);
    strcat(s, " ");
    s += strlen(s);
  }
  strcat(s, "\n");
  if (flag)
    PSputs(logline, output);
  else
    showline(logline, 0);

  if (commandspec != NULL)
  {
    if (flag)
      strcpy(logline, "% ");
    else
      *logline = '\0';

    strcat(logline, "Special:\n");

    if (flag)
      PSputs(logline, output);  // PS output stream
    else
      showline(logline, 0);      // on screen

//    split into separate lines
    s = strtok(commandspec, "\n\r");    // first line 
    while (s != NULL) {
      if (flag)
        strcpy(logline, "% ");
      else
        *logline = '\0';

      strcat(logline, s);
      strcat(logline, "\n");

      if (flag)
        PSputs(logline, output);  // PS output stream
      else
        showline(logline, 0);      // on screen

      s = strtok(NULL, "\n\r"); // go to next line 
    }
  }
}

/* 1995 Sep 27 06:26:35 => 19950927062635 */

void packdatetime (char *date)  /* rewrite date and time in PDF format */
{
  int k;
  char *s;

  while ((s = strchr(date, ' ')) != NULL)
    strcpy(s, s+1);

  while ((s = strchr(date, ':')) != NULL)
    strcpy(s, s+1);
/*  Now have 1990Sep27062635 */
  for (k = 0; k < 12; k++)
    if (strncmp(months+k*3, date+4, 3) == 0)
      break;

  sprintf(date + 4, "%02d", k + 1);
  strcpy(date + 6, date  + 7);
}

void writedocinfo (FILE *output)
{
  time_t ltime;   /* for time and date */
  char *s = "";
  int xll, yll, xur, yur;

  if (makeepsf)
    return;       /* don't do this for EPS file */

  if (directprint)
    return;      /* don't do this when direct */

  if (titlestring == NULL)
  {
    if (filenamex != NULL)
      addescapes(line, filenamex, sizeof(line));  /* MAXLINE */
    else
      *line = '\0';
  }
  else
  {
    strcpy(line, titlestring);  /* 96/July/4 */
    free(titlestring);
    titlestring = NULL;
  }

  sprintf(logline, "[ /Title (%s)\n", line);      /* optional */
  PSputs(logline, output);
  (void) time(&ltime);

  if (ltime > 0)
  {
    s = ctime(&ltime);

    if (s != NULL)
    {
      lcivilize(s);
      packdatetime(s);      /* convert to PDF format */
      sprintf(logline, "  /CreationDate (D:%s)\n", s);  /* optional */
      PSputs(logline, output);
    }
  }

  if (creatorstring == NULL)      // 99/Dec/18
  {
    stampit(line, 0);
  }
  else
  {
    strcpy(line, creatorstring);
    free(creatorstring);
    creatorstring = NULL;
  }

  if (*line != '\0')
  {
    sprintf(logline, "  /Creator (%s)\n", line);
    PSputs(logline, output);
  }

  if (subjectstring == NULL)
  {
    if (comment != NULL) strcpy(line, comment); /* TeX's comment */
    else strcpy(line, "");
  }
  else                        /* 96/July/4 */
  {
    strcpy(line, subjectstring);
    free(subjectstring);
    subjectstring = NULL;
  }

  if (*line != '\0')
  {
    sprintf(logline, "  /Subject (%s)\n", line);      /* optional */
    PSputs(logline, output);
  }

  /*  Allow insertion of keywords using \special{keywords=...} 96/May/10 */
  if (keywords != NULL)
  {
    *line = '\0';
    if (strlen(keywords) < MAXLINE) strcpy(line, keywords);
    free(keywords);
    keywords = NULL;
    keywordslen = 0;
    sprintf(logline, "  /Keywords (%s)\n", line);
    PSputs(logline, output);
  }

/*  Might want to cut short showowner output before date ... */
  if (authorstring == NULL)
  {
    PSputs("  /Author (", output);
    showownerout(line);  /* MAXLINE */
    strcat(line, ")\n");
    PSputs(line, output);
  }
  else                    /* 96/July/4 */
  {
    strcpy(line, authorstring);
    free(authorstring);
    authorstring = NULL;
    sprintf(logline, "  /Author (%s)\n", line);
    PSputs(logline, output);
  }

  PSputs("/DOCINFO pdfmark\n", output);       /* required */

  if (BBxll != 0 || BByll != 0 || BBxur != 0 || BByur != 0) /* 96/May/4 */
  {
    /* should really convert from TeX coordinates to PS coordinates */
    xll = BBxll; yll = BByll;
    xur = BBxur; yur = BByur;
  }
  else
  {
    xll = 0;
    yll = 0;
    xur = (int) pagewidth;
    yur = (int) pageheight;
  }
  sprintf(logline, "[ /CropBox [ %d %d %d %d]\n", xll, yll, xur, yur);
  PSputs(logline, output);
  PSputs("/PAGES pdfmark\n", output);
/*  Above should go between %%BeginSetup and %%EndSetup */
}

void writedocview (FILE *output)    /* write DOCVIEW pdfmark */
{
  if (makeepsf)
    return;       /* don't do this for EPS file */

  if (directprint)
    return;      /* don't do this when direct */

  if (basestring == NULL && pagemode == NULL)
    return;

  /* only if PageMode or Base URL specified */
  PSputs("[", output);

  if (pagemode != NULL)
  {
    strcpy(line, pagemode);
    free(pagemode);
    pagemode = NULL;
    sprintf(logline, " /PageMode %s\n", line);
    PSputs(logline, output);
  }

  if (basestring != NULL)
  {
    strcpy(line, basestring);
    free(basestring);
    basestring = NULL;
    sprintf(logline, " /URI << /Base (%s) >>\n", line);
    PSputs(logline, output);
  }

  PSputs("/DOCVIEW pdfmark\n", output);
/*  Could also have PageMode (UseNone, UseOutlines, UseThumbs, FullScreen) */
/*  (Default is UseNone) --- see Table 10 in TN 5150 */
/*  But these require destination (view, actions, or named destination) */
/*  GoTo, GoToR, Launch, Article --- see Table 11 in TN 5150 */ 
}

/* separated out 94/Mar/3 */
void writesetup (FILE *output, char *filename)
{
  time_t ltime;   /* for time and date */
  int c;        /* for first letter of paper_type */
  char *s;

/*  Don't bother with following if writing direct to printer ? */
  if (bPDFmarks)
  {
    writedocinfo(output);   /* new 95/Feb/25 */
    writedocview(output);   /* new 96/Jul/21 */
  }
  writeparams(output);

/*  fprintf(output, "statusdict begin 0 setjobtimeout end\n"); */

  if (makeepsf == 0)
  {
    PSputs("statusdict /jobname\n(DVIPSONE ", output);
    sprintf(logline, "%d.%d.%d ", version, revision, subrevision);
    PSputs(logline, output);
    PSputs(filename, output);
    PSputc(' ', output);
/*    not clear need to really avoid jobname setting in EPS mode... */

/*    stick in current date and time */
    (void) time(&ltime);        /* get seconds since 1970 */
    if (ltime == -1)
    {
      sprintf(logline, "Time not available (%0ld)!\n", ltime);
      showline(logline, 1);
/*      ltime = 0; *//* 901621283 1998 July 28 06:21:00 */
    }
    s = ctime(&ltime);

    if (s == NULL)
    {
      sprintf(logline, "Cannot convert time (%0ld)!\n", ltime); /* 96/Jan/4 */
      showline(logline, 1);
      s = "Thu Jan 18 22:14:00 2038";
/*      exit(1); */
    }

    lcivilize(s);         /* ??? */
    replaceletter(s, ':', '-');   /* to avoid colons in time */
    PSputs(s, output);
    PSputs(")\nput\n", output);

    if (paper_type != NULL || paper_size != NULL)
    {
      PSputs("[{\n", output);
      if (stripcomment == 0)
      {
        PSputs("%%BeginFeature: ", output);
        PSputs("*PageSize", output);

        if (paper_type != NULL && strchr(paper_type, '*') == NULL)
        {
          PSputc(' ', output);
          c = *paper_type;

          if (c >= 'a' && c <= 'z')
            c = c + 'A' - 'a';
          PSputc(c, output);
          PSputs(paper_type + 1, output);
        }
        PSputc('\n', output);
      }

/*  don't output page command if `custom size' -l=<width>*<height> 94/Dec/16 */
/*      if (strcmp(paper_type, "custom") != 0) */
/*      statusdict /lettertray get exec ??? */
      if (paper_type != NULL && strchr(paper_type, '*') == NULL)
      {
        PSputs(paper_type, output);  /* since already stopped context */
      }
      else
      {
        /* new, make use of setpageparams 1996/Feb/10 */
        /* custom paper size */
        if (bLevel2) /* for level 2 use setpagedevice 98/Apr/4 */
        {
          PSputs("<<", output);
          sprintf(logline,
              "/PageSize [%d %d] /Orientation 0 /ImagingBBox null",
              (int) (pagewidth + 0.5), (int) (pageheight + 0.5));
          PSputs(logline, output);
          PSputs(">> setpagedevice", output);
        }
        else
        {
          /* for level 1 see if this works */
          sprintf(logline, "%d %d 0 1 setpagesize",
              (int) (pagewidth + 0.5), (int) (pageheight + 0.5));
          PSputs(logline, output);
          /* defined in prolog to be essentially `statusdict begin setpageparams end' */
        }
      }
      PSputc('\n', output);

      if (stripcomment == 0)
      {
        PSputs("%%EndFeature\n", output);
      }
      PSputs("} stopped cleartomark\n", output);
    }
/*    NOTE: following includes %%BeginFeature / %%EndFeature */
    if (bDuplex)
    {
      PSputs("[{\n", output);

      if (stripcomment == 0)
      {
        PSputs("%%BeginFeature: ", output);
        PSputs("*Duplex ", output);

        if (bTumble)
        {
          PSputs("DuplexTumble\n", output);
        }
        else
        {
          PSputs("DuplexNoTumble\n", output);
        }
      }
      if (bLevel2)
      {
        PSputs("<<", output);
        PSputs("/Duplex true ", output);

        if (bTumble)
        {
          PSputs("/Tumble true", output);
        }
        else
        {
          PSputs("/Tumble false", output);
        }
        PSputs(">> setpagedevice", output);
      }
      else
      {
        PSputs("true ", output);
        PSputs("statusdict /setduplexmode get exec\n", output);

        if (bTumble)
        {
          PSputs("true ", output);
        }
        else
        {
          PSputs("false ", output);
        }
        PSputs("statusdict /settumble get exec\n", output);
      }
      if (stripcomment == 0)
      {
        PSputs("%%EndFeature\n", output);
      }
      PSputs("} stopped cleartomark\n", output);
    } /* end if bDuplex != 0 */

    if (bsetoverprint)  // 99/Sep/6  a level 2 color separator feature
    {
      PSputs("[{\n", output);

      if (stripcomment == 0)
      {
        PSputs("%%BeginFeature: ", output);
        PSputs("setoverprint\n", output);
      }
      PSputs("true ", output);  // PS default is false ...
      PSputs("setoverprint\n", output);

      if (stripcomment == 0)
      {
        PSputs("%%EndFeature\n", output);
      }
      PSputs("} stopped cleartomark\n", output);
    }

/*    Not clear this is a good place to test since we don't know if used? */
/*    Yet, we don't want to print half the file before finding out! */
/*    OK, now we check for TIFF image inclusion in dvipslog prescan */
/*    cause error message on printer if not PS level 2 */
    if (bLevel2 && bInsertImage)
    {
      dvidictbegin(output);
      sprintf(logline, "CheckLevel2\n");  /* 1996/Dec/20 */
      PSputs(logline, output);
      dvidictend(output);
    }
  } /* end if makeepsf == 0 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void setup_remap (void)
{
  int k;
#if MAXREMAP > 32
  for (k = 0; k < 10; k++)
    remaptable[k] = (unsigned char) (161 + k);

  for (k = 10; k < 32; k++)
    remaptable[k] = (unsigned char) (163 + k);

  for (k = 32; k < MAXREMAP; k++)
    remaptable[k] = (unsigned char) k;

  remaptable[32] = 195;
#if MAXREMAP > 127
  remaptable[127] = 196;
#endif
#else /* MAXREMAP <= 32 */
  for (k = 0; k < 10; k++)
    remaptable[k] = 161 + k;

  for (k = 10; k < 32; k++)
    remaptable[k] = 163 + k;
#endif
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// void showpedigree(FILE *output)
char *showpedigree (char *line, int nlen)
{
  char *s = line;

  stampit(line, 1);
  strcat(line, "\n");
  s += strlen(s);
  showownerout(s);
  strcat(s, "\n");
  s += strlen(s);
  sprintf(s, "%s\n", copyright);
  s += strlen(s);

  if (traceflag)
    printf("\nSTR: %d\n", nlen);

  return s;
}

/* called once only when program starts up */

int dodefaults (void)     /* moved out 1994/May/23 */
{
  char *s;            /* 1996/July/30 */
  int flag=1;


  setup_remap ();        /* may as well always do this 95/Oct/15 */

  if (xmagnification == 0.0)
    xmagnification = 1.0;

  if (ymagnification == 0.0)
    ymagnification = xmagnification;

  if (avoidreside != 0)
    forcereside = 0;  /* makes no sense ! */

  if (beginpages[rangeindex] != -LINFINITY)
    rangeindex++;

  if (xoffseto != UNKNOWNOFFSET || yoffseto != UNKNOWNOFFSET)
    evenoddoff = 1;
  else
    evenoddoff = 0;

  if (xoffsete == UNKNOWNOFFSET)
    xoffsete = 0.0;

  if (xoffseto == UNKNOWNOFFSET)
    xoffseto = xoffsete;

  if (yoffsete == UNKNOWNOFFSET)
    yoffsete = 0.0;

  if (yoffseto == UNKNOWNOFFSET)
    yoffseto = yoffsete;

  if (quietflag == 3)     /* dvipsone -qqq */
  {
    showpedigree(line, sizeof(line)); /* MAXLINE */
    showline(line, 0);
    checkexit(0);       /* 96/Feb/25 */
//    return 0;
  }

  quietflag = (quietflag & 1);      /* only depend on even/odd */

/*  font paths and such 1994/May/23 */
/*  get from ATM.INI if possible - won't work in NT since there is no */

  if (fontpath == NULL)
  {
/*    try and get it from atm.ini */
/*    [Setup] PFB_Dir=c:\psfonts */
#ifndef _WINDOWS
    flag = setupatmini();
#else
    atmini = "atm.ini";     // zstrdup ?
#endif
    if (flag != 0)
    {
      s = grabenvvar("PFB_Dir", atmini, "[Setup]", 1);

      if (s != NULL)
        fontpath = s;      /* 1996/July/30 */

      if (traceflag && fontpath != NULL)
      {
        sprintf(logline, "PFB_Dir %s\n", fontpath);
        showline(logline, 0);
      }
    }

    if (fontpath == NULL)
      fontpath = deffontpath;
  }

  if (pfmpath == NULL)
  {
/*    try and get it from atm.ini */
/*    [Setup] PFM_Dir=c:\psfonts\pfm */
#ifndef _WINDOWS
    flag = setupatmini();
#else
    atmini = "atm.ini";     // zstrdup ?
#endif
    if (flag != 0)
    {
      s = grabenvvar("PFM_Dir", atmini, "[Setup]", 1);

      if (s != NULL)
        pfmpath = s;     /* 1996/July/30 */

      if (traceflag && (pfmpath != NULL))
      {
        sprintf(logline, "PFM_Dir %s\n", pfmpath);
        showline(logline, 0);
      } 
    }

    if (pfmpath == NULL)
      pfmpath = defpfmpath;
  }
  return 0;
}

/* set up wantchrs for ONE font */
void setupfontchar (int fnt)
{
  fontchar[fnt] = (char *) malloc(MAXCHRS);

  if (fontchar[fnt] == NULL)
  {
    showline(" Unable to allocate memory\n", 1);
    checkexit(1);
    return;
  }

  memset(fontchar[fnt], 0, MAXCHRS);
}

/* now allocate some things in space */
void initialtables (void)
{
  int k;

  for (k = 0; k < MAXSUBSTITUTE; k++)
    fontsubfrom[k] = fontsubto[k] = fontsubvec[k] = NULL;

  for (k = 0; k < MAXFONTS; k++)
    fontname[k] = subfontname[k] = fontvector[k] = fontchar[k] = NULL;
}

void freememory (void)  /*   check that heap was not corrupted */
{
  int k;

  for (k = 0; k < MAXSUBSTITUTE; k++)
  {
    if (fontsubfrom[k] != NULL)
    {
      free(fontsubfrom[k]);
      fontsubfrom[k] = NULL;
    }

    if (fontsubto[k] != NULL)
    {
      free(fontsubto[k]);
      fontsubto[k] = NULL;
    }

    if (fontsubvec[k] != NULL)
    {
      free(fontsubvec[k]);
      fontsubvec[k] = NULL;
    }
  }

  for (k = 0; k < MAXFONTS; k++)
  {
    if (fontname[k] != NULL)
    {
      free(fontname[k]);
      fontname[k] = NULL;
    }

    if (subfontname[k] != NULL)
    {
      free(subfontname[k]);
      subfontname[k] = NULL;
    }

    if (fontvector[k] != NULL)
    {
      free(fontvector[k]);
      fontvector[k] = NULL;
    }

    if (fontchar[k] != NULL)
    {
      free(fontchar[k]);
      fontchar[k] = NULL;
    }
  }
}

/* moved here to avoid compiler error ! */

double roundtime (long numer, long denom)
{
  double x;
  x = ((double) numer) / ((double) denom);
  return ((double) ((long) (x * 1000.0 + 0.5))) / 1000.0;
}

char * get_path(void)
{
  static char s[1024];
  HMODULE h = GetModuleHandle(NULL);
  GetModuleFileName(h, s, 1024);
  return strdup(s);
}
/* top level control separated out for jumpout */

int dvibody (int argc, char *argv[])
{
  int m, k, firstarg;
  char *s, *t, *filename;
  clock_t sclock, lclock, fclock, eclock;
  char *mode;
  unsigned long starttime;
  int count = 0;

#ifdef USELOGFILE
  if (logfileflag)
  {

    logfile = fopen(logfilename, "w");

    if (logfile == NULL)
    {
      perrormod(logfilename);
      logfileflag=0;
    }

  }
#endif

#ifdef CONTROLBREAK
  (void) signal(SIGINT, ctrlbreak);
#endif

  verboseflag = traceflag = 0;
  bAbort = 0;                   // redundant

//  fontsubfrom = fontsubto = fontsubvec = NULL;
//  fontname = subfontname = fontvector = NULL;

  if (getenv("DVIDEBUG") != NULL)
    traceflag = 1; /* pre command line */

  if (sscanf(progversion, "%d.%d.%d", &version, &revision, &subrevision) < 3)
  {
    sprintf(logline, "%s ???\n", progversion);
    showline(logline, 1);
  }
  
//  following assumes dvipsone and dviwindo are subdirectories of same thing

  programpath = get_path();
  //programpath = zstrdup(argv[0]);
  //stripname(programpath);
  //
  //strcpy(programpath, getenv("DVIPSONE")); // CM 20140401

/*  if programpath doesn't exist - try and guess - not really likely ! */
  if (programpath == NULL || *programpath == '\0')
  {
    strcpy(programpath, "c:\\yandy\\dvipsone");
  }

  task = "analyzing command line";

  if ((s = getenv("USEDVIWINDOINI")) != NULL)
    sscanf (s, "%d", &usedviwindo);

#ifndef _WINDOWS
  if (usedviwindo)
    setupdviwindo(); // set up access to dviwindo.ini
#endif

  checkenter(argc, argv);

/*  Setup defaults for various paths in case not specified later */
  if ((s = grabenv("YANDYPATH")) != NULL)
  {
    BasePath = zstrdup(s);

    if (programpath != NULL)
      strcpy(line, programpath);
    else
      *line = '\0';

    strcat(line, "\\sub");
    fontsubpath = zstrdup(line);    /* c:\\yandy\\dvipsone\\sub */
    procsetpath = programpath;      /* c:\\yandy\\dvipsone      */
    strcpy(line, BasePath);
    strcat(line, "\\fonts\\encoding");  /* c:\\yandy\\fonts\\encoding */
    vecpath = zstrdup(line);
    strcpy(line, BasePath);
    strcat(line, "\\fonts\\tfm");   /* ??? 96/Aug/29 */
    tfmpath = texfonts = zstrdup(line);
  }
  else                  /* release 1.2 (not NT) */
  {
    /* if programpath exists, use as default for fontsub, procset, and vec */
    fontsubpath = procsetpath = vecpath = programpath;

    if (programpath != NULL)
      strcpy(line, programpath);
    else
      *line = '\0';

    if ((s = strrchr(line, '\\')) != NULL)
      *s = '\0';

    strcat(line, "\\yandytex\\tfm");
/*    tfmpath = texfonts = "c:\\yandytex\\tfm"; */
    tfmpath = texfonts = zstrdup(line);
  }

/*  Now check other environment variables */

  if ((s = grabenv("PSFONTS"))  != NULL) fontpath = s;
  if ((s = grabenv("PSPATH"))   != NULL) epspath = s;
  if ((s = grabenv("AFMPATH"))  != NULL) afmpath = s;
  if ((s = grabenv("TFMPATH"))  != NULL) tfmpath = s;
  if ((s = grabenv("PFMPATH"))  != NULL) pfmpath = s;
  if ((s = grabenv("VECPATH"))  != NULL) vecpath = s;
  if ((s = grabenv("SUBPATH"))  != NULL) fontsubpath = s;
  if ((s = grabenv("PREPATH"))  != NULL) procsetpath = s;
  if ((s = grabenv("TEXFONTS")) != NULL) texfonts = s;  /* 95/Mar/31 */

  if (traceflag)
  {
    sprintf(line, "procsetpath %s\n", procsetpath);   // debugging only
    showline(line, 0);
  }

/*  Force all plain vanilla text fonts to use Windows ANSI encoding */
  if ((s = grabenv("TEXANSI")) != NULL)
    bWindowsFlag = atoi(s);

/*  New 1994/Dec/17 force all text fonts to use specified encoding */
  if ((s = grabenv("ENCODING")) != NULL)
  {
    textencoding = s;

/*    get rid of extension, if any, so can safely use name later 95/Feb/3 */
    if ((s = strrchr(textencoding, '.')) != NULL)
      *s = '\0';

    bWindowsFlag = 1;

/*    environment specific TEXFONTS variables ? 97/May/10 */
    if ((s = grabenv(textencoding)) != NULL)
    {
      texfonts = s;             /* set tfmpath also ? */
    }
  }                       /* override 94/Dec/27 */

/*  rangeindex = 0; */    /* no page ranges specified yet */
/*  beginorend = 0; */    /* last not defined */

//  set up page ranges and install defaults */
  if (expandpageranges() < 0)
    return -1;

  prologfileindex = 0;        /* 1993/Dec/21 */
  
  readcommands(dvipsonecmd);      /* dvipsone.cmd file *first* if any */

  firstarg = commandline(argc, argv, 1);  /* then do command line */
//  if (firstarg <= 0) return -1;     // 2000 June 18

  if (abortflag)
    return -1;

  dodefaults();           /* 1994/May/23 */

  if (quietflag == 3)
    return 0;

  if (firstarg < 0 || firstarg >= argc)
  {
    showusage(argv[0]);
    return -1;
  }

/* following must be *after* checkowner, and *before* stampit */

  stampit(line, 1);
  strcat(line, "\n");
  showline(line, 0);
//  if (logfileflag) stampit(logfile, 1, 1);
//  showline("\n", 0);
  showline(copyright, 0);
  showline("\n", 0);

  if (verboseflag)
    showline("\n", 0);

/*  Code to make up random prefix to avoid Adobe Reader partial font problem */
  if (bRandomPrefix)
  {
    starttime = (unsigned long) time(NULL);
    s = szRandomPrefix+5;

    while (s >= szRandomPrefix) {
      *s-- = (char) ('A' +  (int) (starttime % 26));
      starttime = (starttime / 26);
    }

    if (verboseflag)
    {
      sprintf(logline, "Random Prefix %s\n", szRandomPrefix);
      showline(logline, 0);
    }

    fontprefix = szRandomPrefix;      /* make it point here */
  }

/*  if (bDemoFlag) dtime = checkdemo(); */

  if (evenpageflag != 0 && oddpageflag != 0) /* this is silly ! */
  {
    evenpageflag = 0; oddpageflag = 0;
  }

  if (showcommandflag)
  {
    showcommand(stdout, argc, argv, 0); /* show command line 94/Mar/8 */

    if (logfileflag != 0)
      showcommand(logfile, argc, argv, 0); /* show command line 94/Mar/8 */
  }

  initialtables();
  init_enc(bANSITeX); /* tex_text and ansi encoding from SE */

  if (traceflag)
  {
    if (bWindowsFlag)
    {
      sprintf(logline, "Will use `%s' encoding for text fonts\n", textencoding);
      showline(logline, 0);
    }
  }

  task = "opening output file";

//  deal with quoted file names now 2000 May 24
  if (outputfile != NULL && *outputfile == '\"')
  {
    strcpy(outputfile, outputfile+1); // get rid of first quote
    s = outputfile + strlen(outputfile) - 1;

    if (*s == '\"')
      *s = '\0';
  }

/*  if output file specified, open only ONCE (for all input files) */
/*  also, don't force extension if an extension was specified */
/*  also, try and figure out if going direct to a printer */

  if (outputfile != NULL)
  {
    strcpy(fn_out, outputfile);

    directprint = 0;
    m = strlen(fn_out);             /* LPT1: COM1: */

    if (m == 5 && *(fn_out+m-1) == ':')
    {
      if (_strnicmp(fn_out, "com", 3) == 0 ||
          _strnicmp(fn_out, "lpt", 3) == 0)
        *(fn_out+m - 1) = '\0';     /* strip the colon */
    }

/*  Maybe flush this sometime?  How many CAS Intellifax boards left ? */
/*  Try and guess whether direct to printer */

    if (_strcmpi(fn_out, "prn") == 0 ||
        _strcmpi(fn_out, "aux") == 0 ||
        _strcmpi(fn_out, "ept") == 0 ||
        ((strlen(fn_out) == 4) && (_strnicmp(fn_out, "lpt", 3) == 0 || _strnicmp(fn_out, "com", 3) == 0) &&  *(fn_out+3) >= '0' && *(fn_out+3) <= '9'))
    {
      directprint = 1;
    }
/*    Try and deal with Windows network printer, like:  \\ast\hp4mplus */
/*    Try and deal with Windows network printer, like:  \\fasecon\3rdfloor */
/*    Try and deal with Windows network printer, like:  \\foo.lpr\bar\odd */
/*    Heuristic: starts with two backslashes and has no file extension */
    if (strncmp(fn_out, "\\\\", 2) == 0) /* possible network printer ?*/
    {
      if ((s = strrchr(fn_out, '.')) == NULL ||
          ((t = strrchr(fn_out, '\\')) != NULL && s < t))
        directprint = 1;
    }

/*    Deal with network printers of form NE01: ... */
    s = fn_out + strlen(fn_out) - 1;    /* 1995/Dec/20 */

    if (*s == ':')
    {
      if (s > fn_out + 1)
        *s = '\0';    /* strip the colon ??? */

      directprint = 1;      /* 1995/Dec/20 */
    }

    if (directprint)
    {
      if (dontstrip)
        stripcomment = 0;
      else
        stripcomment = 1;        /* normally strip comments */
    }
    else /* apparently not a printer */
    {
      stripcomment = 0;
    }

/*    maybe open output in binary mode to avoid \r before \n ? */
    if (retplusnew == 0)
      mode = "wb";
    else
      mode = "w";

    output = fopen(fn_out, mode);

    if (output == NULL)
    {
      sprintf(logline, "ERROR: Can't make output file `%s' (%s)\n",
          fn_out, mode);
      showline(logline, 1);
      perrormod(fn_out);
      giveup(3);
      //return -1;          // failure
    }
  }

  if (bAbort)
    abortjob();

  if (abortflag)
    return -1;

  for (m = firstarg; m < argc; m++)  /* do each file in turn */
  {
    sclock = clock();       /* get starting time */
    s = argv[m];

    if (*s == '\"')  // remove "..." wrapper 2000 June 5
    {
      strcpy(s, s+1);
      s = s + strlen(s) - 1;

      if (*s == '\"')
        *s = '\0';
    }

    filenamex = argv[m];
    errlevel = 0;         /* start counting errors anew */
    colorindex = 0;         /* right place ? */
    colortypeflag = 0;        /* default using rgb */
/*    CurrentA = CurrentB = CurrentC = 0.0F; CurrentD = -1.0F; */
    CurrColor.A = CurrColor.B = CurrColor.C = 0.0F;
    CurrColor.D = -1.0F;
    bColorUsed = 0;         /* 98/Feb/14 */
    fnext=0;
    BBxll = BByll = BBxur = BByur = 0;  /* crop box given 96/May/4 */
    bInsertImage = 0;         /* whether \special{insertimage: ..} */
    badpathwarn = 0;

/*    numpages = 0; */        /* reset page counter */

    task = "opening files";
    strcpy(fn_in, argv[m]);       /* for convenience */

    filename = removepath(fn_in);   /* strip off path of file */
//    if (strcmp(outputfile, "") == 0)  /* output file specified ? */

    if (outputfile == NULL)
    {
      if (currentdirect != 0)     /* write in current directory ? */
        strcpy(fn_out, filename);
      else
        strcpy(fn_out, fn_in);   /* write in same place as input */

      forceexten(fn_out, OUTPUTEXT);
      directprint = 0;
      stripcomment = 0;

/*      maybe open output in binary mode to avoid \r before \n ? */
      if (retplusnew == 0)
        mode = "wb";
      else
        mode = "w";

      output = fopen(fn_out, mode);

      if (output == NULL)
      {
        sprintf(logline, "ERROR: Can't make output file `%s' (%s)\n",
            fn_out, mode);
        showline(logline, 1);
        perrormod(fn_out);
        giveup(3); 
        //return -1;
      }
    }

#ifdef DEBUGFLUSH
    if (output != NULL) setbuf(output, NULL); /* serious stuff ! */
#endif

    strcpy(fn_in, argv[m]);
//    strcpy(dvipath, fn_in);         /* possible use by dvispeci */
    dvipath = zstrdup(fn_in);
    stripname(dvipath);

    input = fopen(fn_in, "rb");
    if (input == NULL) /* try name as is */
    {
      forceexten(fn_in, INPUTEXT);
      input = fopen(fn_in, "rb");

      if (input == NULL)    /* try .dvi */
      {
        forceexten(fn_in, "tex");
        input = fopen(fn_in, "rb");

        if (input == NULL) /* try .tex */
        {
          sprintf(logline, "Can't find input file `%s'\n", argv[m]);
          showline(logline, 1);
          perrormod(argv[m]);

          if (outputfile == NULL)  /* output specified ? */
          {
            if (output != NULL)
            {
              fclose(output); /* close output */
              output = NULL;
            }
            (void) remove(fn_out);    /* and remove bad file */
          }
          maxerror++; /* errcount(0); */
          break;    /* giveup(3); ? */
        }
      }
    }
/*    don't do this once per file... */
/*    if (showcommandflag != 0) 
      showcommand(stdout, argc, argv, 0); */ /* show command line  */
    sprintf(logline, "Processing DVI file %s ", fn_in); 
    showline(logline, 0);

//    directprint = 1;    // debugging only
//    stripcomment = 1;   // debugging only

    sprintf(logline, "(%sto %s)\n", directprint ? "direct " : "",
        fn_out);
    showline(logline, 0);
    
/*    input = fp_in;          */
/*    output = fp_out;        */

    task = "pre-scanning DVI file"; /*  for font usage */

/*    sets up headerfile also */ /* and picks off verbatim headers */
/*    and extra command line args for DVIPSONE */
    if (scanlogfile(input) != 0)
    {
      fclose(input);
      continue;           /* failed - not DVI file */
    }

//    can this mess things up by changing settings assumed above ???
//    can this mess things up for the next file by changing state ???
    if (commandspec != NULL)      // new 99/Sep/6
    {
//      need not split into separate lines
      scancommands(s);
    }

    rewind(input);          /* get set for next pass */

    task = "checking for duplicate & substitutions";

    if (substituteflag)       // this is done once per file now
      (void) GetSubstitutes();  // read font substitution table

    preextract();

/*    do the following only once ? not needed for every file ? */ 
/*    analpapertype(paper_type); */  
/*    get PageHeight and PageWidth */   /* 94/May/6 */
    pageheight = 11*72; pagewidth = 8.5*72;   /* default */

    if (paper_size != NULL)
      decode_paper_size(paper_size, &pagewidth, &pageheight);

    if (paper_type != NULL)
      analpapertype(paper_type, &pagewidth, &pageheight);

    lclock = clock();       /* end of prescan - start of fonts */

    if (bMarkUnused)
      MarkUnusedFonts(); /* 95/Mar/5 */

    task = "writing header";    /* write file header */

    writestruct(output);    /* do after number of pages known */

/*    comments between %%EndComments and %%BeginProlog */
    if (stripcomment == 0)
      showcommand(output, argc, argv, 1);   /* 92/Dec/.. */

    if (commandspec != NULL)
    {
      free(commandspec);
      commandspeclen = 0;
      commandspec = NULL;
    }

    if (stripcomment == 0 && comment != NULL)
    {
      PSputs("% ", output);
      PSputs(comment, output);
      PSputc('\n', output);
    }

/* Note: %%BeginProlog is highly optional ... */
    if (bOptionalDSC)
    {
      if (stripcomment == 0)
      {
        PSputs("%%BeginProlog\n", output);
      }
    }

    if (writeheader(output) != 0)
    {
      showline("ERROR: Problem writing header files\n", 1);
      break;
    }

    if (abortflag)
    {
      showline("PREMATURE ABORT!", 1);  // debugging only
      break;    // ???
    }

    textextwritten = 0;           /* 1992/Nov/19 */
    ansiwritten = 0;            /* 1993/Sep/30 */
/*    textencwritten = 0; */          /* 1994/Dec/17 */

    if (dosetupearly)           /* 1994/Mar/4 */
    {
      if (stripcomment == 0)
      {
        PSputs("%%EndProlog\n", output);
      }
    }

    writeencodingvecs(output);

    if (abortflag)
    {
      showline("PREMATURE ABORT!", 1);  // debugging only
      break;    // ???
    }

    if (dosetupearly == 0)        /* 1994/Mar/4 new default */
    {
      if (stripcomment == 0)
      {
        PSputs("%%EndProlog\n", output);
      }
    }

    if (stripcomment == 0)
    {
      PSputs("%%BeginSetup\n", output);
    }

    if (dosetupearly)    /* no longer the default 94/Mar/3 */
    {
      writesetup(output, filename);

      if (stripcomment == 0)
      {
        PSputs("%%EndSetup\n", output);
      }
    }

/*    header files  already taken care of in writeheader() */

    task = "decompressing font files";  /* put these fonts in the output */

/*    extract(output);  */      /* split up 1994/Mar/3 */
    extractfonts(output);

    if (abortflag)
    {
      showline("PREMATURE ABORT!", 1);  // debugging only
      break;    // ???
    }

    if (dosetupearly == 0)   /* new default */
    {
      writesetup(output, filename);
    }

    fontsetup(output);        /* do remapping, naming etc */

    if (dosetupearly == 0)  /* new default */
    {
      if (stripcomment == 0)
      {
        PSputs("%%EndSetup\n", output);
      }
    }

//    if (strcmp(comment, "") != 0)   /* moved down here 96/Feb/25 */
    if (comment != NULL)  /* moved down here 96/Feb/25 */
    {
      free(comment);
      comment = NULL;
    }               /* free, not used later */

    fclock = clock();       /* get time at end of fonts */
  
    task = "translating DVI file";  /* translate DVI to PS */

    if (collated != 0 && copies > 1)
    {
      for (k = 1; k < copies; k++)
      {
        scan_dvi_file(output, input, 0);
        rewind(input);

        if (abortflag)
          break;
      }
    }

    if (abortflag)
    {
      showline("PREMATURE ABORT!", 1);  // debugging only
      break;
    }

    scan_dvi_file(output, input, 1);

    if (abortflag)
    {
      showline("PREMATURE ABORT!", 1);  // debugging only
      break;
    }

    task = "writing trailer";

    writetrailer(output);

    task = "closing files";

    fclose(input);

/*    if output file was NOT specified close output for each input file */
//    if (strcmp(outputfile, "") == 0) /* output file specified ? */
    if (outputfile == NULL) /* output file specified ? */
    {
      if (output != NULL && ferror(output))
      {
        showline("\n", 0);
        sprintf(logline, " ERROR in output file %s\n", fn_out);
        showline(logline, 1);
        perrormod(fn_out);
        errcount(0);
      }
      else
      {
        if (output != NULL)
        {
          fclose(output);
          output = NULL;
        }
      }
    }

    eclock = clock();       /* get ending time */

    if (! verboseflag)
      showline("\n", 0);

/*    Paul Anagnostopolous memorial statement: */

    if (verboseflag && numpages != dvi_t)
    {
      sprintf(logline, "Processed %ld page%s\n", numpages,
        (numpages == 1) ? "" : "s");  /* 1994/Feb/1 */
      showline(logline, 0);
    }

    if (timingflag)
    {
      sprintf(logline, "Time: %lg sec ",
        roundtime(eclock - sclock, (long) CLOCKS_PER_SEC));
      showline(logline, 0);

/*      not so meaningful when directprint != 0 */
/*      not so meaningful when page selection activated */
      if (collated != 0 && copies > 1)
        numpages = numpages * copies;

/* following rewritten 1993 Aug 28 */
      if (nfonts > 0 || numpages > 0)
      {
        *logline = '\0';
        strcat(logline, "(");
        s = logline + strlen(logline);

        if (nfonts > 0)
          sprintf(s, "%lg sec per font file",
              roundtime(fclock - lclock, (long)  CLOCKS_PER_SEC * (long) nfonts));

        if (nfonts > 0 && numpages > 0)
          strcat(logline, " + ");

        s = logline + strlen(logline);
        if (numpages > 0)
          sprintf(s, "%lg sec per page output",
              roundtime(eclock - fclock + lclock - sclock, (long) CLOCKS_PER_SEC * (long) numpages));
        strcat(logline, ")");
        showline(logline, 0);
      }
      showline("\n", 0);
    } /* end of if timingflag != 0 */

    if (errlevel > maxerror)
      maxerror = errlevel;

    if (bBackGroundFlag && bBackUsed)
      freebackground(); /* 98/Jun/30 */

    if (bCarryColor && bColorUsed)
      freecolorsave(); // 2000 May 27

    if (paper_size != NULL)
    {
      free(paper_size);
      paper_size = NULL;
    }

    count++;        /* 99/Mar/17 */
    fflush(stdout);       /* ??? 98/Jun/30 */

    freememory();       // moved here to happen every file

    if (bAbort)
      abortjob();

    if (abortflag)
      break;
  } /* end of for loop over files */
  
//  if (abortflag > 0) return -1;

  task = "closing output file";

/*  if output file specified, close only ONCE (for all input files) */
//  if (strcmp(outputfile, "") != 0)  /* output file specified ? */
  if (outputfile != NULL)
  {
    if (output != NULL && ferror(output))
    {
      showline("\n", 0);
      sprintf(logline, " ERROR in output file %s\n", fn_out);
      showline(logline, 1);
      perrormod(fn_out);
      errcount(0);
    }
    else
    {
      if (output != NULL)
      {
        fclose(output);
        output = NULL;
      }
    }
  }

//  does this work in _WINDOWS ???
  if ((m = _heapchk ()) != _HEAPOK)    /* 1995/Nov/10 */
  {
    sprintf(logline, " ERROR: heap corrupted (%d)\n", m);
    showline(logline, 1);
    exit(9);      /* terminate with extreme prejudice */
  }

  DeAllocStringsOut();  /* in dvitiff.c new 96/Dec/28 */
  DeAllocStringsIn();   /* in dvitiff.c new 96/Dec/28 */

//  freememory();     // already done after each file

/* also free Background color stack ? */
/* also free saved color stacks ? */
/* also free beginpages, endpages, etc. ? */

/*  if (argc > firstarg + 1) 
    printf("Processed %d DVI files\n", argc - firstarg); */
  if (count > 1)
  {
    sprintf(logline, "Processed %d DVI files\n", count);
    showline(logline, 0);
  }
/* only need plural version in the above ... */

  if (maxerror > 0)
  {
    if (maxerror == 1)
      sprintf(logline, "One error encountered\n");
    else if (maxerror > 1)
      sprintf(logline, "%d errors encountered\n", maxerror);

    showline(logline, 1);
    errlevel--;       // undo errlevel increment by showline
/*    can't do the above just by tacking `s' on the end of `error' */
  }

  checkpause(maxerror);   /* 97/Nov/28 */
//  checkpause(1);        /* 2000 June 5 */
#ifdef USELOGFILE
  if (logfile != NULL)
  {
    fclose(logfile);
    logfile = NULL;
  }
#endif

  if (abortflag) showline("Fatal Error in DVIPSONE conversion\n", 1);
  return maxerror;
}

int jumpused=0;     // avoid misues of jump buffer

jmp_buf jumpbuffer;   // for non-local jumps

int uexit (int code)
{
#ifndef _WINDOWS
  fflush(stdout);
#endif
  if (jumpused)
  {
    sprintf(logline, "ERROR: Jump Buffer used %d\n", jumpused);
    showline(logline, 1);
    exit(1);
  }
  jumpused++;
  longjmp(jumpbuffer, code + 1);    // 1999/Nov/7
}

int main (int argc, char *argv[]) /* main program entry point */
{
  int flag = 0, ret;

  errout = stdout;
//  now creates jump buffer for non-local goto's  99/Nov/7
  jumpused = 0;
  ret = setjmp(jumpbuffer);

  if (ret == 0) // get here when setting up jumpbuffer
  {
    flag = dvibody(argc, argv);

    if (traceflag)
    {
      sprintf(logline, "EXITING at %s flag %d ret %d jumpused %d\n",
          "MAIN", flag, ret, jumpused);
      showline(logline, 0);
    }
  }
  else // get here from non-local jump via jumpbuffer - if any
  {
    if (traceflag)
    {
      sprintf(logline, "EXITING at %s flag %d ret %d jumpused %d\n",
          "JUMPOUT",  flag, ret, jumpused);
      showline(logline, 0);
    }
    freememory();     // already done after each file
  }
#ifdef USELOGFILE
  if (logfile != NULL)
  {
    fclose(logfile);
    logfile = NULL;
  }
#endif

  if (flag == 0)
    return 0;
  else
    exit (flag);
}

//////////////////////////////////////////////////////////////////////////////

/* Things to do or to worry about: */

/* Structuring: %%PageFonts: ? UGH ! %%PageBoundingBox: UGH ! */

/* Doesn't make sense to have %%For: since user unknown ? */

/* If importing a nested EPS file use %%BeginDocument and %%EndDocument OK */

/* Probably bad idea to make .bak files of output .ps files if exist ? OK */

/* Do something about setting pageheight and pagewidth maybe ? OK */
/* ONLY need it for placing default TeX origin at 1in 1in from top left */
/* and for rotating if rotation specified */

/* Check that everything gets initialized properly for MULTIPLE files OK */

/* Possible open .err output file after first ERROR and send complaints NO */

/* Include stock preamble in case preamble file not found ? NO */

/* Have %%Pages: <n> also in trailer ? NOT IF AT BEGINNING */

/* should have some command line flag to disable breakearly DISABLED */
/* watch that breakearly idea - possible variability in end NOT SAFE */

/* CHECK OUT BACKWARD COMPATABILITY WITH BRAIN-DAMAGED INTERPRETERS: */

/* For example, abbreviating \<octal> may not always work right */
/* but (\12) is ok */

/* When adding Metrics entry to font, may need to also copy of BBox */
/* just so something is different */
/* (this is only a potential problem if BOTH original and remapped is used) */

/* readline may expect newline, not return */
/* worry about readline over AppleTalk link ? */

/* set numpages properly in dvianal ? */

/* terminate early in dvianal (and dvipslog?) when b and e specified ? */
/* only if based on page counting rather than counter[0] */

/* Remember: if many files left open, things can slow down by factor 10 */

/* Try .tex file after .dvi file is not found ? OK */

/* How to deal with file names that do not have extensions */

/* if output is to printer, forgo the analysis of time per font OK */

/* allows '/' in place of '-' for command line flags ? NO */

/* fontsubpath, procsetpath and vecpath default to directory of program */

/* following are device specific ... */
/* letter (8.5 x 11), legal (8.5 x 14), ledger (11 x 17) */
/* a4, b5, note, envelope */
/* PS letter, note, legal, ledger ? */

/* major space user is charnames[MAXCHRS][MAXCHARNAME] = 8192 bytes DVIEXTRA*/

/* major space user is fontchar[MAXFONTS][MAXCHRS] = 16384 bytes DVIPSLOG */

/* major space user is fontname[MAXFONTS][MAXTEXNAME] = 4352 => 8448 */

/* major space user is subfontname[MAXFONTS][MAXFONTNAME] = 4352 => 8448 */

/* move compiletime and compiledate to a separate module that is always new? */

/* quietflag now controls both \special complaints and font substitution */

/* protect against more bad behaviour in eps files ? see Adobe list ? */

/* -C print n copies, collated - don't redo fonts  ? */

/* sort pages command line flag ? */ 

/* crop mark option ? */  /* invert B/W ? */

/* manual feed option flag ? */ /* tray selection option ? */

/* Maybe introduce other encoding vectors ? */

/* Include TeXtext, Typewrite, Mathitalic, Mathsymbol ? */

/* Omit TeXitalic, Typeitalic, TeXascii, TeXtype, Mathextended ? */

/* check that order of transformations is sensible */

/* check that hpxl and vpxl are computed ok when rotated */

/* use -C flag to print collated copies ? (don't repeat fonts) */

/* use PS stroke for rules instead of box ? */

/* check if font is really needed before putting in writefontlist  OK */
/* add a flag to property which is set once to indicate no chars used */

/* using a b c d e f g h i (j) (k) l m n o p q r s t u v w x y z */

/*  stick current date and time into jobname ? OK */

/* deal with lower case LPT, COM, PRN, EPT ? */

/* argument of %%Pages: is incorrect when subset of pages selected */

/* use kbhit() in conio() to trap used termination */

/* use SIBBREAK in signal() */

/* use memset */

/* why does it slow down so much if many files called by wild-card ? */
/* - try dvipsone -d=NUL -s=arborres c:\springer\*.dvi */

/* does not implement inheritance of DocumentResourceNeeded */
/* from included EPSF files  */

/* maybe use (atend) for a lot of things ? */

/* maybe NOT use BeginResource / EndResource for fonts - since not complete */

/* page range messages appear BEFORE program title announced */
/* this would be ok if they terminated operation, but they don't... */

/* had to move fontchar (char hit table) out of local memory to make space */

/* use showowner somewhere in generating PS file also ? */

/* make 'J' (usefullarr != 0) default ? */ /* YES 1993/May/27 */

/* according to EXEHDR, this needs 177k bytes when first loaded 94/July/7 */
/* according to EXEHDR, this needs 193k bytes when first loaded 95/Dec/30 */
/* then it tries to allocate quite a bit for tables, maybe 80 k bytes */
/* stack size is 3000 bytes (0BB8 hex) */

/* Long file name support under Windows 95 possible */
/* see 1996 April Windows Developer Journal page 50--56 */
/* need code given there - applies only to fopen / fclose streams */
/* that code does not set errno, but does set _doserrno */
/* needs lfn_findclose() to terminate lfn_findfirst and lfn_findnext */
/* lfn_open() works for "rb" mode open, not "r" */

/* This should accept DDE messages for opening and for printing */
/* DDE [FileOpen("%1")] for open */
/* DDE [FilePrint("%1")] for open */

/* Should also accept commmand line open, print, and printo arguments */
/* open "%1" */
/* print /p "%1" */
/* printo /t "%1" "%2" "%3" "%4" */
/* where %1 is the file name */
/* and %2 %3 %4 are printer, driver, and port presumably */
