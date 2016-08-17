/* Common header information for DVIPSONE 

   Copyright 1990,1991,1992,1993,1994,1995,1996,1997,1998,1999 Y&Y, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <setjmp.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>   /* for _stat etc. */
#include <direct.h>     /* for _getcwd(); */
#include <conio.h>

#ifdef WIN32
#pragma warning(disable:4996)
#pragma warning(disable:4127) // conditional expression is constant
#endif

#ifdef _WINDOWS
  #define PSputs(str,output) psputs(str, output)
// #define PSputc(chr,output) psputc(str, output);
#define PSputc(chr,output)              \
  do{                                   \
    psbuffer[psbufpos++] = (char) chr;  \
     if ((psbufpos+2 >= PSBUFLEN) ||    \
         (chr == '\n'))                 \
    sendpsbuffer(output);               \
  } while(0)
#define showline(str,flag) ShowLine(str,flag)
#else
  #define PSputs(str,output) fputs(str, output)
  #define PSputc(chr,output) putc(chr,output)
#define showline(str,flag)  \
  do{                       \
    fputs(str,stdout);      \
    if(logfileflag)         \
    fputs(str,logfile);     \
  } while(0)
#endif

#define ID_BYTE 2     /* for version of DVI files that we understand */

/* Introduce new convenient structure for color info 98/Jul/18 */

typedef struct
{
  float A;
  float B;
  float C;
  float D;
} COLORSPEC;

#define MAXFILENAME FILENAME_MAX
#define FNAMELEN    FILENAME_MAX  /* max file name length */
#define MAXPATHLEN  FILENAME_MAX  /* max path name length */
#define MAXCOMMENT  256           /* max length of TeX comment in DVI file */
#define MAXLINE     512           /* max length of input line 1994/Feb/24 */
#define MAXCHARNAME 32            /* (25?) max length of character name - align */
/* MAXCHARNAME no longer limits length of char name just space allocation */
#define MAXCHRS     256           /* max number of characters codes */
#define TEXCHRS     128           /* max number of characters in a TeX font */
#define MAXFONTNUMBERS  1024      /* 1999/Feb/22 */
#define MAXFONTS        512U      /* 1999/Nov/3 for Larry Tseng */
#define MAXSUBSTITUTE   512       /* max fonts in substitution table */
#define WRAPCOLUMN      64        /* were to start thinking about wrapping special */
#define MAXSHOWONLINE   6         /* max shows in a row before newline */
#define MAXERRORS       64        /* error count before giving up */
#define MAXRANGES       16        /* max number of page ranges - now expands */
#define DVIDICT         300       /* allocation for DVIDICT in printer */
/* dvidict needs less to 256 (plus space for fonts */
/* but user may define a few extra things in dvidict ... */
/* can use command line -D=... to increase */

#define MAXREMAP 128      /* MAXREMAP can be 32 or 128 */
/* #define MAXREMAP 32 */   /* MAXREMAP can be 32 or 128 */

/* If it is 32, then code 32 does not get mapped to 195 and 127 not to 196 */

#define MAXSUBFILES 16      /* 1994/Feb/8 */
#define MAXPROLOGS 16     /* 1993/Dec/21 */

/* #define MAXSUBFILES 8 */   /* 1994/Feb/8 */
/* #define MAXPROLOGS 8 */    /* 1993/Dec/21 */

#define MAXCOLORSTACK 16    /* max depth of color stack */
/* #define MAXCOLORSTACK 8 */ /* max depth of color stack */

// #define BLANKFONT 255    /* this font number in DVIPSONE not used */
#define BLANKFONT -1    /* this font number in DVIPSONE not used */

#define INPUTEXT  "dvi"   /* default extension on input file */
#define OUTPUTEXT "ps"  /* extension on output file */

#define RESIDENT  "*reside*"    /* font resident in printer */
#define FORCESUB  "*force*"     /* force substitution */
#define REMAPIT   "*remap*"     /* remap to specified encoding */
#define ALIASED   "*alias*"     /* just another name for same thing */
#define SYNTHETIC "*synthetic*" /* don't mess with innards ! */
#define COMPOUND  "*compound*"  /* backward compatability */
#define HYBRID    "*hybrid*"    /* don't mess with innards */
#define MTMI      "*mtmi*"      /* font sucks rocks ! */
#define EPSF      "*epsf*"      /* allow use for EPSF %%IncludeFont: */
/* #define CONTROL    "*control*" */  /* remap control character range */

#define C_RESIDENT  1        /* code for RESIDENT font */
#define C_FORCESUB  2        /* code for FORCESUB font */
#define C_REMAPIT   4        /* code for REMAPIT font */
#define C_ALIASED   8        /* code for ALIASED font */
#define C_MISSING   16       /* code for missing font */
#define C_UNUSED    32       /* code for unused font */
#define C_DEPENDENT 64       /* code for font in other size */
/* #define C_COMPOUND 128 */ /* font is synthetic */
/* #define C_HYBRID 128 */   /* font is synthetic */
#define C_SYNTHETIC 128      /* font is synthetic */
#define C_MTMI      256      /* font sucks rocks */
#define C_EPSF      512      /* allow use for EPSF %%IncludeFont: */
#define C_MULTIPLE  1024     /* Multiple Master Base Font */
#define C_INSTANCE  2048     /* Multiple Master Instance */
#define C_NOTBASE   4096     /* subfontname is MM instance, not MM base */
/* #define C_CONTROL 8192 */ /* remap control character range */

#define CRYPT_MUL 52845u  /* pseudo-random number generator multiplier */
#define CRYPT_ADD 22719u  /* pseudo-random number generator addend */

#define REEXEC 55665      /* seed constant for eexec encryption */
/* #define RCHARSTRING 4330 */  /* seed constant for charstring encryption */

/* #define SINFINITY 32767 */ /* 2^{15} - 1 */
#define LINFINITY 2147483647  /* 2^{31} - 1 */

#define SUBDIRSEARCH  /* 1994/Aug/18 provide for sub-directory search */

#define TPIC      /* insert code for dealing with TPIC \specials */

#define TIFF      /* insert code for dealing with TIFF \specials */

#define ALLOWSCALE    /* allow scaling of output DVI coordinates */

/* DVI one byte commands: */

enum dvicom 
{
  set1 = 128, set2, set3, set4,
  set_rule = 132, 
  put1 = 133, put2, put3, put4,
  put_rule = 137,
  nop = 138, bop, eop, push, pop,
  right1 = 143, right2, right3, right4,
  w0 = 147, w1, w2, w3, w4,
  x0 = 152, x1, x2, x3, x4,
  down1 = 157, down2, down3, down4,
  y0 = 161, y1, y2, y3, y4,
  z0 = 166, z1, z2, z3, z4,
  /* fnt_num_0 = 171, font_num_1, font_num_2, font_num_3, */
  fnt1 = 235, fnt2, fnt3, fnt4,
  xxx1 = 239, xxx2, xxx3, xxx4,
  fnt_def1 = 243, fnt_def2, fnt_def3, fnt_def4,
  pre = 247, post, post_post
};

/* what DVI commands translate to: */

/*
#define SHOW      "s"
#define RIGHT     "r"
#define DOWN      "d"
#define WRIGHT    "w"
#define WSETRIGHT "W"
#define XRIGHT    "x"
#define XSETRIGHT "X"
#define YDOWN     "y"
#define YSETDOWN  "Y"
#define ZDOWN     "z"
#define ZSETDOWN  "Z"
#define PUSHSTATE "u"
#define POPSTATE  "o"
#define MAKEFONT  "mf"
*/

/* AND: O => oo, U => u u, M => o u */

/* in dviextra.c */
extern unsigned char decrypt_byte(unsigned char cipher, unsigned short * crypter);
extern void preextract(void);
extern void writetextext(FILE * fp_out);
extern void write_ansi_code(FILE * fp_out, char * textenconame);
extern int read_text_encode(char * textencoding);
extern void write_dvi_encode(FILE * fp_out);
extern int extractfonts(FILE * fp_out);
extern void fontsetup(FILE * fp_out);
extern unsigned long readlength(FILE * input);
extern void make_file_name(char * filepath, char * fontname);
extern int underscore(char * filename);
extern int remove_under(char * filename);
extern int ResidentFont(char * FileName);
extern int FindFileName(char * fontname, char * filename);
extern int MarkUnusedFonts(void);
extern int GetSubstitutes(void);
/* in dvipslog.c */
extern int scanlogfile(FILE * fp_in);
extern void resetpagerangehit(int flag);
extern char * alias(char * name);
extern char * nextpathname(char * pathname, char * searchpath);
extern int searchalongpath(char * filename, char * pathlist, char * pathname, int current);
extern FILE * findandopen(char * filename, char * pathlist, char * pathname, char * mode, int current);
extern int LookupATMReg(char * szPSFontName, char * szPSFileName);
extern int SetupATMReg(void);
extern void freebackground(void);
extern int readtfm(char * font, FILE * fp_tfm, long widths[]);
extern int readafm(char * font, FILE * fp_tfm, long widths[]);
extern int readpfm(char * font, FILE * fp_tfm, long widths[]);
extern int NamesFromPFM (FILE * input, char * FaceName, int nface, char * FontName, int nfont, char * FileName);
/* in dvianal.c */
extern int scan_dvi_file(FILE * output, FILE * input, int lastflag);
extern long goto_post(FILE * input);
/* in dvispeci.c */
extern FILE * findepsfile(char * name, char * ext, int warnflag, int readflag);
extern int scan_special(FILE * input, char * buff, int nmax);
extern int scan_special_raw(FILE * input, char * buff, int nmax);
extern FILE * fopenfont(char * FontName, char * FileName, int mmflag);
extern int FindMMBaseFile(int k);
extern int checkCTM(FILE * output);
extern int checkColorStack(FILE * output);
extern int doColorPopAll(int pageno);
extern int doColorPop(int pageno);
extern int doColorPush(int pageno);
extern void doColorSet(FILE * output, int popflag);
extern double decodeunits(char * units);
/* in dvipsone.c */
extern int readcommands(char *filename);
extern void errcount(int flag);
extern void giveup(int code);
extern void tellwhere(FILE * input, int errflag);
extern int get_alpha_token(FILE * input, char * token, int nmax);
extern int get_token(FILE * input, char * buff, int nmax);
extern void flush_special(FILE * input);
extern int skip_this_page(long pageno);
extern int readspecial(FILE * output, FILE * input, unsigned long ns);
extern void prereadspecial(FILE * input, unsigned long ns);
extern void lowercase(char * t, char * s);
extern void uppercase(char * t, char * s);
extern void extension(char * fname, char * ext);
extern void forceexten(char * fname, char * ext);
extern char * extractfilename(char * pathname);
extern void removeexten(char * fname);
extern int getline(FILE * input, char * buff);
extern int getrealline(FILE * input, char * buff);
extern char * removepath(char * pathname);
extern int copyepssimple(FILE * output, FILE * special);
extern int setupatmini(void);
extern void checkexit(int n);
extern char * zstrdup(char * s);
extern void abortjob(void);
extern char * grabenv(char * varname);
extern void setupinifilesub(char * ininame, char * fullfilename);
extern int uexit(int code);
extern void setupfontchar(int fnt);
extern void map850topdf(char * buffer, int nlen);
extern void complainspecial(FILE * input); 
extern void init_enc(int ansitexflag);
extern int decompress_font(FILE * output, FILE * input, char * FontName);
extern int newspecials(FILE * output, FILE * input);
extern int dohptag(FILE * output, FILE * input);
extern unsigned long codefourty(char * codingvecctor);
extern void DeAllocStringsIn(void);
extern void DeAllocStringsOut(void);
extern void doColor(FILE * output, FILE * input, int c, int outflag);
extern void RestoreColorStack(int page);
extern void freecolorsave(void);
extern void doClipBoxPopAll(FILE * output);
void scivilize(char * date);
void lcivilize(char * date);
void perrormod(char * s);
FILE * open_font(char * font, int flag);
FILE * open_pfm(char * font);
int LoadATMREG (void);

////////////////////////////////////////////////////////////////////

extern FILE * errout;    /* where to send error output */
extern int logfileflag;   /* write log file 99/Apr/20 */
extern FILE * logfile;   /* 1999/Apr/20 */
extern FILE * input;       /* used by tellwhere */
extern int volatile bAbort;   /* set by user control-C */ /* 1992/Nov/24 */
extern int abortflag;
extern char * task;      /* current task -  for error message */

extern int const statisticsflag;  /* non-zero => output stack depth, fonts used */
extern int const complainflag;    /* non-zero implies complain sub table warnings */
extern int const timingflag;    /* non-zero => show timing information */

extern int verboseflag;   /* non-zero => lots of output */
extern int traceflag;   /* non-zero => lots of output */
extern int quietflag;     /* non-zero => suppress \special complaints */
extern int reverseflag;   /* non-zero => do pages in reverse order */
extern int skipflag;    /* on when page to be skipped reading forward */
extern int countzeroflag; /* non-zero => page limits based on TeX /count0 */
extern int stripcomment;  /* non-zero => strip comments */
extern int bUseCounters;  /* non-zero counter[1]-counter[2] 96/Jan/28 */
extern int bShortFont;    /* use shorter font numbers for /f... */
extern int bBackWardFlag; /* force long form of octal escape in strings */
extern int bForwardFlag;    /* use \b, \t, \n, ... \f, \r in strings 93/Sep/29 */
extern int textures;    /* non-zero => textures style DVI file */
extern int bBackGroundFlag; /* support \special{background ...} 98/Jun/30 */
extern int bBackUsed;   /* non-zero of \special{background ...} used */
extern int oddpageflag;   /* print only odd pages */
extern int evenpageflag;  /* print only even pages */
extern double xoffsete;     /* x offset even pages */
extern double yoffsete;     /* y offset even pages*/
extern double xoffseto;     /* x offset odd pages */
extern double yoffseto;     /* y offset odd pages */
extern int evenoddoff;    /* non-zero if offsets differ on even/odd page */
extern int pagetpic;    /* non-zero if TPIC special used on this page */
extern int complainedaboutj;  /* reset at top of page */
extern int colorindex;    /* color stack index 96/Nov/3 */
extern int clipstackindex;  /* clip push pop index 98/Sep/12 */
extern int bColorUsed;    /* non-zero if \special{color ...} seen */
extern int bCarryColor;   /* carry color across pages 98/Feb/14 */
extern int CTMstackindex; /* CTM stack index 96/Nov/3 */
extern int newbopflag;    /* want LogicalPage PhysicalPage */
extern int bOptionalDSC;  /* want %%PageTrailer comments */
extern int bRemapControl; /* non-zero => 0 - 32, 127 => 161 -- 196 */
extern int bRemapSpace;   /* non-zero => 32 => 195, 0 => 161 95/Oct/17 */
extern int bRemapFont;    /* remap just this one font ... 95/Oct/15 */
extern int nRepeatIndex;  /* 95/Aug/27 */
extern int nRepeatCount;  /* 95/Aug/27 */

extern long pageno;     /* for convenience in error messages dvipslog.c */
extern long pagenumber;   /* count of pages actually seen */
extern long numpages;   /* number of pages actually processed */
extern long dvistart;   /* where DVI part of Textures file starts */
extern long nMinRule;   /* min rule thickness (Adobe Reader fix) 95/Oct/10 */
extern long previous;   /* pointer to previous bop in this file */
extern int finish;      /* when seen last eop */

extern long counter[10];

extern char line[MAXLINE];  /* general purpose input `buffer' 512 bytes */
extern char logline[MAXLINE]; /* used for showline */

extern char *outputfile;    /* output file or empty */
extern char *filenamex;     /* remember file name here */
extern char *epspath;     /* directory to look for EPF files */
extern char *dvipath;     /* directory of dvi file */

extern char *fontsubpath;
extern char *fontsubrest;   /* font substitution file on command line */

extern char fontsubfile[FNAMELEN];    /* font substitution file */
extern char *subfontfile[MAXSUBFILES];  /* user subfile or empty (-s=...) */

extern int remaptable[MAXREMAP];  /* 1994/June/20 */

// extern int finx[MAXFONTNUMBERS];   /* indeces into next few */
extern short finx[MAXFONTNUMBERS];  /* indeces into next few */

extern int fontsubflag[MAXFONTS]; /* non-zero if substitute font used */
extern int fontproper[MAXFONTS];  /* code resident/forced/remap/alias */
extern unsigned long fc[MAXFONTS];  /* checksum of TFM file (encoding info) */
extern unsigned long fs[MAXFONTS];    /* at size */ 

// extern char *fontname;     /* 1994/Feb/2 */
extern char *fontname[MAXFONTS];  /* 1999/Nov/6 */
// extern char *subfontname;    /* 1994/Feb/2 */
extern char *subfontname[MAXFONTS]; /* 1999/Nov/6 */
// extern char *fontvector;     /* font remapping vector */
extern char *fontvector[MAXFONTS];      /* font remapping vector */
// extern char *fontchar;     /* which characters needed */
extern char *fontchar[MAXFONTS];      /* which characters needed */

// extern char *fontsubfrom;
extern char *fontsubfrom[MAXSUBSTITUTE];  /* 1999/Nov/6 */
// extern char *fontsubto;
extern char *fontsubto[MAXSUBSTITUTE];    /* 1999/Nov/6 */
// extern char *fontsubvec;
extern char *fontsubvec[MAXSUBSTITUTE];   /* 1999/Nov/6 */

extern int dvi_s, dvi_t;  /* stack depth & no of pages (bop) in DVI file */

extern unsigned long dvi_l, dvi_u;  /* max page height + depth & max width *//* NA */

extern unsigned long num, den;  /* from DVI file */
extern unsigned long mag;   /* Tex \mag => 1000 times desired magnification */

extern int ff;    /* current font - changed by fnt and fnt_num - reset by bop */
extern int fnt;   /* fnt =  finx[ff] */

extern int fnext;         /* next slot to use */
extern int flast;         /* last including MM fonts */

#ifdef ALLOWSCALE
extern int outscaleflag;
extern double outscale;
#endif

// dvitiff.c

extern int bTextColor;
extern double text_red;
extern double text_green;
extern double text_blue; 
extern int bRuleColor;
extern double rule_red;
extern double rule_green;
extern double rule_blue; 
extern int bFigureColor;
extern double figurered;
extern double figuregreen;
extern double figureblue; 
extern int bReverseVideo;
extern int bDVICopyReduce;        /* 1995/Sep/16 */
extern long postposition; /* position of post op in DVI */

extern COLORSPEC CurrColor;
extern COLORSPEC *BackColors;

extern COLORSPEC ColorStack[MAXCOLORSTACK];   /* current stack 98/Jul/18 */

extern int bPDFmarks;   /* write pdfmarks into PS file 95/Feb/25 */
extern int bGhostHackFlag;
extern int bSuppressBack; /* suppress background in imagemask 95/Dec/31 */
extern int nDefaultTIFFDPI; /* use if resolution not specified in TIFF */
extern int bLevel2;     /* if allowed to use level 2 features 96/Dec/20 */ 
extern int bAllowCompression; /* allow use of compression of TIFF images */
extern int bAllowColor; /* allow use of colorimage operator */
extern int bKeepBlack;    /* suppress textcolor, rulecolor, figurecolor */
extern int bInsertImage;  /* if \special{insertimage: ... } seen */
extern int bLandScape;    /* asked for landscape mode -*O 95/Jun/24 */

extern int wantmagictiff;   /* TIFF: want to use our own screen function */
extern int frequency, angle;  /* frequency and angle */
extern int usealtGoToR;     /* use alternate /GoToR action in PDF */

// dvispeci.c

extern int directprint;     /* non-zero => output directly to printer */
extern int bSmartCopyFlag;    /* non-zero => look for %%EOF and such */
extern int bPassControls;   /* pass through control characters in EPSF */
extern int bKeepTermination;  /* keep line terminations intact in EPS */
extern int passcomments;    /* non-zero => do not flush comment lines EPS */
extern int verbatimflag;    /* non-zero => allow verbatim PostScript */
extern int preservefont;    /* preserve font across verbatim */
extern int bWrapSpecial;    /* prevent long linesin \special */

extern int bSciWord;      /* treat ps: filename case */
extern int bIgnoreSpecials;   /* non-zero => ignore \specials */
extern int bPassEPSF;     /* non-zero => pass EPSF files (normal) */
extern int allowtpic;     /* non-zero => allow TPIC \specials */
extern int needtpic;      /* non-zero => need TPIC header */
extern int bOldDVIPSFlag;   /* non-zero => wrap ps: code unlike ps:: */
extern int bOldDVIPSFlip;   /* flip coordinates in ps: code 96/Nov/7 */
extern int bQuotePage;      /* suppress showpage in \special{"..."} */
extern int bProtectStack;   /* add [ ... cleartomark 97/Nov/24 */
extern int bStoppedContext;   /* add { ... } stopped 97/Nov/24 */

extern int bAllowInclude;   /* expand %%IncludeResource: font ... */
extern int bAllowBadBox;    /* allow single % comment before BoundingBox */
extern int bConvertReturn;    /* convert isolated \r in EPS to \n */
extern int reverseflag;     /* non-zero => do pages in reverse order */
extern int showcount;     /* on when last sent out "set" or "put" */
extern int freshflag;     /* at start of new line from PS special 99/Dec/19 */

extern int useatmini;     /* use [Setup] section in `atm.ini' */
extern int useatmreg;     /* use armreg.atm */
extern int usepsfontname;   /* allow use of PS Fontnames in DVI TFM */
extern int bFirstNull;      /* non-zero => flush \special starts with null */

extern char *szATMRegAtm;   /* full file name of atmreg.atm */
extern char *atmini;      /* full file name for atm.ini, with path */
extern char *tpiccommands;
extern char *fontpath;      /* from PSFONTS env var in dvipsone.c */
extern char *afmpath;   /* default path for AFM font metric files */
extern char *tfmpath;   /* default path for TFM font metric files */
extern char *pfmpath;   /* default path for PFM font metric files */
extern char *vecpath;   /* default place for encoding vectors */
extern char *texfonts;    /* default place for TFM font metric files */

extern int currentfirst;  /* look in current directory for files */
extern int useatmfontsmap;  /* zero if tried and failed to find atmfonts.map */
extern char *atmfontsmap; /* file name and path to ATMFONTS.MAP */

extern int colortypeflag; /* 0 => gray, 1 => rgb, 2 => cmyk */
extern int colorindex;    /* points at next slot to push into */
extern int clipstackindex;        /* set to zero at start of page */

extern int columns;     /* maximum length of output line */
extern int subfontfileindex;      /* pointer into following */

extern int showsubflag;     /* non-zero => show substitution table */
extern int wantnotdef;      /* non-zero => want .notdef CharString */
extern int flushcr;       /* try and get rid of those pesky \r */
extern int stripinfo;     /* strip out rest of /FontInfo directory */
extern int substituteflag;    /* non-zero => do font substitution */
extern int forcereside;     /* non-zero => force printer resident */
extern int avoidreside;     /* non-zero => veto printer resident */
extern int bWindowsFlag;    /* reencode StandardEncoding to Windows ANSI */
            /* *or* by specified textencoding - ENCODING env var */
extern int bCheckEncoding;    /* check checksum for match with encoding */
extern int insertmetrics;   /* insert new font metrics in substituted */

extern int tryunderscore;   /* try underscore form of font file name */
extern int bUseInternal;    /* non-zero => use internal number for /fn...*/
extern int bAllowShortEncode; /* allow name of Encoding instead of list */
extern int bAllowStandard;    /* allow use StandardEncoding if it works */
extern int bAllowTexText;   /* allow use TeXtext encoding if it works */
extern int bAllowANSI;      /* allow use ANSI encoding if it works */
extern int busedviencode;   /* use dvipsone reencoding of all fonts ! */
extern int accentedflag;      /* deal with accented / composite chars */
extern int syntheticsafe;   /* be paranoid about synthetic fonts */
extern int bSuppressPartial;  /* suppress partial font downloading */
extern int bForceFullArr;   /* force full 256 element Encoding array */
extern int keepgap;       /* keep space bytes before ND 93/Aug/5*/
extern int showfontflag;    /* non-zero => show font tables */
extern int stripchecking;   /* strip garbage from nasty old Adobe fonts */
extern int textextwritten;    /* non-zero tex_text encoding already written */
extern int ansiwritten;     /* non-zero ansi encoding already written */
extern int bTexFontsMap;    /* non-zero => allowed to use `texfonts.map'*/
extern int bForceFontsMap;    /* non-zero => replace aliases `texfonts.map'*/
extern int bMarkUnused;     /* non-zero => mark unused fonts early */
extern int bMMShortCut;     /* simple way to construct base file name */

extern int bSubRealName;    /* substitute file name for FontName in PFA */
                /* not good idea to set to zero ... */
extern int bRandomPrefix;   /* add random prefix to file names 95/Sep/30 */

extern int bAddBaseName;    /* add BaseFontName for Distiller 97/Jan/30 */

extern char *textencoding;    /* encoding used by plain vanilla text fonts */
                /* may override "ansinew" 94/Dec/17 */
extern char *textenconame;    /* just vector name itself 95/Feb/3 */

extern unsigned long nCheckSum; /* expected checksum ENCODING= *//* 95/Feb/3 */

extern int nfonts, nsubstitute, nremapped, nansified;

extern int uppercaseflag;   /* convert font names to upper case */
extern int maxsubstitute;   /* usually equal to MAXSUBSTITUTE */
extern int maxfonts;      /* usually equal to MAXFONTS */
extern int bStripBadEnd;    /* read to EOL at end of encrypted 96/Feb/22 */

extern char *fontprefix;    /* prefix to use on FontName */

extern int BBxll, BByll, BBxur, BByur;  /* crop box given 96/May/4 */

extern int rangeindex;      /* index into following table */

// extern long beginpages[MAXRANGES]; /* table of pagebegins */
extern long *beginpages;  /* table of pagebegins */
// extern long endpages[MAXRANGES]; /* table of pageends */
extern long *endpages;  /* table of pageends */

// extern int pagerangeseq[MAXRANGES];/* which instance of page range desired ? */
extern int *pagerangeseq;/* which instance of page range desired ? */

extern char fn_in[FNAMELEN], fn_out[FNAMELEN];    /* 1994/Mar/1 */

extern long nspecial;   /* byte count of special */
extern long nspecialsav;  /* byte count of special */
extern long specstart;    /* saved start of \special for error message */

extern char *comment;     /* pointer to place for TeX comment */

extern char *paper_size;     /* Custom paper size specified via special */

extern int nheaderlength;   // 99/July/14
extern char *headerfile;    /* name of user supplied header file(s) */

extern int headertextlen;   /* how much has been accumulated */
extern char *headertext;    /* accum header PS text from headertext= */

extern int commandspeclen;
extern char *commandspec;

extern int dsccustomlen;    /* how much has been accumulated */
extern char *dsccustom;     /* accum DSC header text from DSCcomment= */

extern int keywordslen;     /* how much has been accumulated in following */
extern char *keywords;      /* accumulated keywords from keywords= */

extern char *dscfile;     /* name of (single) DSC header file */

extern char *creatorstring;   /* creator for DocInfo in Adobe Reader */
extern char *titlestring;   /* title for DocInfo in Adobe Reader */
extern char *subjectstring;   /* Subject for DocInfo in Adobe Reader */
extern char *authorstring;    /* Author for DocInfo in Adobe Reader */
extern char *basestring;    /* Base for DocView in Adobe Reader */
extern char *pagemode;      /* Base for DocView in Adobe Reader */

extern int stinx;       /* stack index */ /* just for verification */
extern int maxstinx;      /* max stack index seen */
extern int usefontmap;      /* 1996/July/30 */

extern int dirindex;      // number of dir paths in ATMREG.ATM
extern int ATMfontindex;    // number of font entries from ATMREG.ATM

extern int usecallbackflag;

extern int ksubst;        // entries in substitution table

extern int fontsubprop[MAXSUBSTITUTE];  /* resident/forced/remapped/alias */

extern int badpathwarn;

extern char **DirPaths;

extern jmp_buf jumpbuffer;    // for non-local jumps
