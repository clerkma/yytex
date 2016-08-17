/* Copyright 1990,1991,1992,1993,1994,1995,1996,1997,1998,1999 Y&Y, Inc.
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
* Program for decompressing and extracting font files in PFA & PFB format
*
**********************************************************************/

#include "dvipsone.h"

#define ZEROS        512 /* 512 zeros required at end of eexec section */
#define ZEROSPERLINE 64  /* 64 zeros per line for safety */
#define NOTDEF       256 /* special code for .notdef = MAXCHRS */
#define COORDINATE   "+" /* prefix for reencoded font 97/June/1 */

static unsigned short int cryptin;   /* current seed for input decryption */
static unsigned short int cryptout;  /* current seed for output encryption */

static char charseen[MAXCHRS + 1]; /* new - which CharStrings unpacked + NOTDEF */

static int clm;          /* current output column */

static int binaryin = 0;     /* non-zero => input binary, not hex */
static int wantcreation=1;     /* copy creation date through to output */
static int aliasesexist=0;     /* if *alias* found in font substitution file */
static int syntheticsexist=0;    /* if *synthetic* in font substitution file */
              /* normally not set to avoid need to check table */

static int type3flag = 0;      /* non-zero => font file is PKTOPS output */
              /* zero => font file is Adobe Type 1 style */
int mmflag=0;       /* non-zero => Multiple Master Font 94/Dec/6 */

int pssflag = 0;      /* non-zero => `font' file is PSS stub 94/Dec/6 */
int instanceflag = 0;   /* non-zero => MM instanced via PFM 97/June/1 */

int fontform = 0;     /* 0 => unrecog, 1 => old form, 2 => new form */
              /* used only if breakearly is on - NOT USED */
int standard = 0;     /* non-zero => StandardEncoding --- NOT USED */
int texfont = 0;      /* non-zero => Tex font --- NOT USED */

int fontchrs = MAXCHRS;   /* actual number of characters in font */

int hybridflag;       /* 1993/Aug/5 */ /* made global 1994/July/15 */

int bBaseNameDone = 1;    /* if zero, need not add BaseFontName anymore */

/* int stripbadend=0; */  /* for now 95/Mar/1 */

// static char filefontname[MAXFONTNAME]=""; /* from file name */
static char filefontname[FNAMELEN] = ""; /* from file name */
static char realfontname[FNAMELEN] = "";    /* from /FontName */
                      /* but first guess from %!PS line */

static int chrs = -1;        /* current character working on */
static unsigned long len;      /* counter of bytes in binary input */

/* Fonts, that --- in Oblique or Narrow form --- are synthetic: */

static char *syntheticfonts[] =
{
  "Helvetica",
  "Courier",
  "AvantGarde",
  "Univers",
  "Optima",
  "Futura",
  "NewsGothic",
  "EuroStyle",      /* added 97/Oct/23 */
  "TektonMM",       /* added 99/Apr/12 */
  ""
};

/* char *basecharacters="aceinousyzACEINOUSYZ"; *//* just basic 58 - Latin 1 */

/* char *basecharacters="aceinousyzACEINOUSYZdlrtDLRT";*/ /* ISO Latin 1 + 2 */

/* char *notbasecharacters="bfmpqvxBFMPQVX"; */ /* ??? */

/* font substitution table - obtained from file */
/* fontsubprop small enough to keep in near space - rest banished to far */

/* static int fontsubprop[MAXSUBSTITUTE]; *//* resident/forced/remapped/alias */

int fontsubprop[MAXSUBSTITUTE];  /* resident/forced/remapped/alias */

/* 1993/Nov/15 switches from fixed array to pointers into `namestring' */

/* static char charnames[MAXCHRS][MAXCHARNAME]; */  /* names read from encoding */

#define STRINGSPACE (MAXCHRS * MAXCHARNAME / 2)

/* The above comes to 4096 bytes - tex_text = 650, SE = 945, ANSI = 1528 */

char *charnames[MAXCHRS];       /* encoding vector 1993/Nov/15 */

char namestring[STRINGSPACE];     /* for encoding vector 1993/Nov/15 */

int stringindex;            /* index into above space */

/* various encoding vectors needed */

/* is the following really needed ? YES, if font file Encoding is standard */

static char *std_enc[] =
{
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright",
  "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
  "zero", "one", "two", "three", "four", "five", "six", "seven",
  "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
  "at", "A", "B", "C", "D", "E", "F", "G",
  "H", "I", "J", "K", "L", "M", "N", "O",
  "P", "Q", "R", "S", "T", "U", "V", "W",
  "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
  "quoteleft", "a", "b", "c", "d", "e", "f", "g",
  "h", "i", "j", "k", "l", "m", "n", "o",
  "p", "q", "r", "s", "t", "u", "v", "w",
  "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "nbspace", "exclamdown", "cent", "sterling", "fraction", "yen", "florin", "section",
  "currency", "quotesingle", "quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright", "fi", "fl",
  "", "endash", "dagger", "daggerdbl", "periodcentered", "", "paragraph", "bullet",
  "quotesinglbase", "quotedblbase", "quotedblright", "guillemotright", "ellipsis", "perthousand", "", "questiondown",
  "", "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
  "dieresis", "", "ring", "cedilla", "", "hungarumlaut", "ogonek", "caron",
  "emdash", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "AE", "", "ordfeminine", "", "", "", "",
  "Lslash", "Oslash", "OE", "ordmasculine", "", "", "", "",
  "", "ae", "", "", "", "dotlessi", "", "",
  "lslash", "oslash", "oe", "germandbls", "", "", "", "",
};

/*  TeX text encoding vector - is this needed ? Yes, if can't find file */

/* If sharing character names, copy upper part from StandardEncoding */

#ifdef SHAREENCODING
static char *tex_text[TEXCHRS] =
{
  "Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon",
  "Phi", "Psi", "Omega", "ff", "fi", "fl", "ffi", "ffl",
  "dotlessi", "dotlessj", "grave", "acute", "caron", "breve", "macron", "ring",
  "cedilla", "germandbls", "ae", "oe", "oslash", "AE", "OE", "Oslash",
  "suppress"
};
#else
static char *tex_text[TEXCHRS] =
{
  "Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon",
  "Phi", "Psi", "Omega", "ff", "fi", "fl", "ffi", "ffl",
  "dotlessi", "dotlessj", "grave", "acute", "caron", "breve", "macron", "ring",
  "cedilla", "germandbls", "ae", "oe", "oslash", "AE", "OE", "Oslash",
  "suppress", "exclam", "quotedblright", "numbersign", "dollar", "percent", "ampersand", "quoteright",
  "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
  "zero", "one", "two", "three", "four", "five", "six", "seven",
  "eight", "nine", "colon", "semicolon", "exclamdown", "equal", "questiondown", "question",
  "at", "A", "B", "C", "D", "E", "F", "G",
  "H", "I", "J", "K", "L", "M", "N", "O",
  "P", "Q", "R", "S", "T", "U", "V", "W",
  "X", "Y", "Z", "bracketleft", "quotedblleft", "bracketright", "circumflex", "dotaccent",
  "quoteleft", "a", "b", "c", "d", "e", "f", "g",
  "h", "i", "j", "k", "l", "m", "n", "o",
  "p", "q", "r", "s", "t", "u", "v", "w",
  "x", "y", "z", "endash", "emdash", "hungarumlaut", "tilde", "dieresis"
};
#endif

/* "grave", "acute", "circumflex", "tilde", */
/* "macron", "breve", "dotaccent", "dieresis", */
/* "ring", "cedilla", "hungarumlaut", "ogonek", */
/* "caron", "dotlessi", "", "" */

/* If sharing character names, copy lower part from StandardEncoding */

/* NOTE: has non-standard added positions `dotlessi' 157 `caron' 141 */

/* NOTE: if env var ENCODING set then *that* vector is used instead */
/*       it is read in and overrides this Windows ANSI encoding */

/* SHAREENCODING means we copy over some pointers to save memory */
/* We might consider reading this one in from disk if needed */
/* Now that we may not be using it if ENCODING env var is set */

#ifdef SHAREENCODING
static char *ansi_enc[256] =
{
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "quotesinglbase", "florin", "quotedblbase", "ellipsis", "dagger", "daggerdbl",
  "circumflex", "perthousand", "Scaron", "guilsinglleft", "OE", "caron", "", "",
  "", "quoteleft", "quoteright", "quotedblleft", "quotedblright", "bullet", "endash", "emdash",
  "tilde", "trademark", "scaron", "guilsinglright", "oe", "dotlessi", "", "Ydieresis",
  "nbspace", "exclamdown", "cent", "sterling", "currency", "yen", "brokenbar", "section",
  "dieresis", "copyright", "ordfeminine", "guillemotleft", "logicalnot", "hyphen", "registered", "macron",
  "degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu", "paragraph", "periodcentered",
  "cedilla", "onesuperior", "ordmasculine", "guillemotright", "onequarter", "onehalf", "threequarters", "questiondown",
  "Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE", "Ccedilla",
  "Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex", "Idieresis",
  "Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply",
  "Oslash", "Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls",
  "agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae", "ccedilla",
  "egrave", "eacute", "ecircumflex", "edieresis", "igrave", "iacute", "icircumflex", "idieresis",
  "eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis", "divide",
  "oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn", "ydieresis"
};
#else
static char *ansi_enc[] =
{
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle",
  "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
  "zero", "one", "two", "three", "four", "five", "six", "seven",
  "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
  "at", "A", "B", "C", "D", "E", "F", "G",
  "H", "I", "J", "K", "L", "M", "N", "O",
  "P", "Q", "R", "S", "T", "U", "V", "W",
  "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
  "grave", "a", "b", "c", "d", "e", "f", "g",
  "h", "i", "j", "k", "l", "m", "n", "o",
  "p", "q", "r", "s", "t", "u", "v", "w",
  "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "",
  "", "", "quotesinglbase", "florin", "quotedblbase", "ellipsis", "dagger", "daggerdbl",
  "circumflex", "perthousand", "Scaron", "guilsinglleft", "OE", "caron", "", "",
  "", "quoteleft", "quoteright", "quotedblleft", "quotedblright", "bullet", "endash", "emdash",
  "tilde", "trademark", "scaron", "guilsinglright", "oe", "dotlessi", "", "Ydieresis",
  "nbspace", "exclamdown", "cent", "sterling", "currency", "yen", "brokenbar", "section",
  "dieresis", "copyright", "ordfeminine", "guillemotleft", "logicalnot", "hyphen", "registered", "macron",
  "degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu", "paragraph", "periodcentered",
  "cedilla", "onesuperior", "ordmasculine", "guillemotright", "onequarter", "onehalf", "threequarters", "questiondown",
  "Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE", "Ccedilla",
  "Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex", "Idieresis",
  "Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply",
  "Oslash", "Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls",
  "agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae", "ccedilla",
  "egrave", "eacute", "ecircumflex", "edieresis", "igrave", "iacute", "icircumflex", "idieresis",
  "eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis", "divide",
  "oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn", "ydieresis"
};
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int mmcount;                /* how many MM base fonts added */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Stuff for initializing resident encoding vectors - to save memory */

void init_enc(int ansitexflag)
{
  int k;

#ifdef SHAREENCODING
  for (k = 33; k < 123; k++)
    tex_text[k] = std_enc[k];
/*  now for the fixups */
/*  tex_text[32] = "suppress"; */
  tex_text[34] = std_enc[186];  /* "quotedblright" */
  tex_text[60] = std_enc[161];  /* "exclamdown" */
  tex_text[62] = std_enc[191];  /* "questiondown" */
  tex_text[92] = std_enc[170];  /* "quotedblleft" */
  tex_text[94] = std_enc[195];  /* "circumflex" */
  tex_text[95] = std_enc[199];  /* "dotaccent" */
  tex_text[123] = std_enc[177]; /* "endash" */
  tex_text[124] = std_enc[208]; /* "emdash" */
  tex_text[125] = std_enc[205]; /* "hungarumlaut" */
  tex_text[126] = std_enc[196]; /* "tilde" */
  tex_text[127] = std_enc[200]; /* "dieresis" */

  for (k = 32; k < 128; k++)
    ansi_enc[k] = std_enc[k];

  ansi_enc[39] = std_enc[169]; /* "quotesingle" */
  ansi_enc[96] = std_enc[193]; /* "grave" */
/*  copy over accents for Adobe Level 1 PS interpreter bug as in PSCRIPT */
/*  for (k = 0; k < 15; k++) ansi_enc[k] = std_enc[k+193]; */
/*  ansi_enc[15] = std_enc[245]; *//* dotlessi*/
#endif
/*  copy over accents for Adobe Level 1 PS interpreter bug as in PSCRIPT */
/*  actually, mostly we just need `caron', `dotlessi' and maybe `ring' ... */
  if (!ansitexflag)
  {
    for (k = 0; k < 15; k++)
      ansi_enc[k] = std_enc[k+193];

    ansi_enc[15] = std_enc[245]; /* dotlessi*/
  }

  if (ansitexflag)
  {
    /* or do this in PostScript later ??? 93/Dec/18 */
    /* this had a bug that was fixed 93/Dec/28 */
/*    strcpy(ansi_enc[0], ""); */   /* avoid grave repeat */
/*    strcpy(ansi_enc[1], ""); */   /* avoid acute repeat */
/*    strcpy(ansi_enc[4], ""); */   /* avoid macron repeat */
/*    strcpy(ansi_enc[5], ""); */   /* avoid breve repeat */
/*    strcpy(ansi_enc[9], ""); */   /* avoid ring repeat */
/*    strcpy(ansi_enc[10], ""); */  /* avoid cedilla repeat */
/*    strcpy(ansi_enc[14], ""); */  /* avoid caron repeat */
    /* 0  - 10 Greek - not in ANSI */
    /* 11 - 15 f ligatures - not in ANSI */
    /* 16 - 17 dotlessi dotlessj - not ANSI */
/* 16 - 24 dotlessi, dotlessj, grave, acute, caron, breve, macron, ring, cedilla */
/* 25 - 31 germandbls, ae, oe, oslash, AE, OE, Oslash */
/* actually: dotlessj, caron, breve, - and ring - missing in ANSI */ 
    for (k = 16; k < 32; k++)
      ansi_enc[k] = tex_text[k];

    ansi_enc[17] = "";        /* flush `dotlessj' */
    ansi_enc[21] = "";        /* flush `breve' */
/* but keep `caron' and `ring' for PS interpreter bug fix (not in ANSI) */
/* potential problem with some characters now being repeated higher up ? */
  }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for reading in AFM/TFM files and extracting metric information */
/* provides alternate of reading AFM file instead */
/* if file can't be found or read, don't adjust metric information */

/* graceful exit with suitable error message */
/* graceful exit with meaningful error message */
void extgiveup(int code)
{
  char *s = logline;

  if (*task != '\0')
  {
    sprintf(s, " while %s", task);
    s += strlen(s);
  }

  if (chrs >= 0)
  {
    sprintf(s, " for character %d ", chrs);
    s += strlen(s);
  }

  if (*filefontname != '\0')
  {
    sprintf(s, " in font %s", filefontname);
    s += strlen(s);
  }

  strcat(logline, "\n");
  showline(logline, 1);
/*  exit(code); */
  checkexit(code);          /* 1995/Oct/28 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* read four byte length code */
unsigned long readlength(FILE* input)
{
  int c, k;
  unsigned long n = 0L;

  for (k=0; k < 4; k++)
  {
    c = getc(input);
    n = n >> 8; n = n | ((unsigned long) c << 24);
  }

  return n;
}
/* read four byte length code */
unsigned long maclength(FILE *input)
{
  int k;
  unsigned long n = 0L;
  
  for (k = 0; k < 4; k++) n = (n << 8) | getc(input);
  return n;
}

int getnextnon(FILE *input)
{
  int c;
  c = getc(input);
  if (c == '\r' && flushcr != 0)
  {
    c = getc(input);
    if (c != '\n')
    {
      (void) ungetc(c, input);
      c = '\n';
    }
  }
  return c;
}
  
/* read a line up to newline (or return) */ /* returns EOF if at end of file */
/* was char *line */
int extgetline(FILE *input, char *buff)
{
  char *s=buff;
  int c, k=0;

  c = getnextnon(input);

  while (c != '\n')
  {
    if (c == EOF)
      return EOF;

    if (c == 128) /* flush over ASCII section headers */
    {
      binaryin = 1;
      c = getc(input);

      if (c == 3)
        return EOF;

      if (c != 1)
      {
        sprintf(logline,
          " Expecting %s, not %d", "ASCII section code", c);
        showline(logline, 1);
        extgiveup(5);
        return EOF;
      }

      len = readlength(input);  /* read and ignore ! */
      c = getnextnon(input);

      if (c == EOF)
        return EOF; /* never */

      if (c == '\n')
        break;
    }
    else if (c == 0) /* Presumably Mac Style ASCII section code */
    {
      sprintf(logline, " AT BYTE %ld ", ftell(input));  /* debugging */
      showline(logline, 0);
      binaryin = 1;
      (void) ungetc(c, input);
      len = maclength(input);
      c = getc(input);

      if (c == 5)
        return EOF;

      if (c != 1)
      {
        sprintf(logline, " Expecting %s, not %d",
            "Mac ASCII section code", c);
        showline(logline, 1);
        extgiveup(5);
        return EOF;
      }

      c = getc(input);

      if (c != 0)
      {
        sprintf(logline, " Invalid Mac ASCII section code %d", c);
        showline(logline, 1);
        extgiveup(5);
        return EOF;
      }

      len = len -2;
      c = getnextnon(input);

      if (c == EOF)
        return EOF; /* never */

      if (c == '\n')
        break;
    }

    *s++ = (char) c;
    k++;

    if (k >= MAXLINE)
    {
      showline(" Line too long in dviextra getline", 1);
      *(s-1) = '\0';        /* terminate the junk at least */

      if (verboseflag)
      {
        showline("\n", 0);
        showline(buff, 1);
        showline("\n", 0);
      }
      
      /* read to end of line (or EOF) before going on ? */
      while ((c = getnextnon(input)) != '\n')
      {
        if (c == EOF)
          return EOF;
      }

      errcount(0);        /* 93/Aug/13 */
      *buff = '\0';       /* flush this crap ! */

      return 0;
    }

    c = getnextnon(input);  /* flush any returns */
  }

  *s++ = (char) c; k++; /* terminating '\n' */
  *s++ = '\0';

  return k;
}

/* hmm, return value of extgetrealline used to be never used ... */
/* get non-comment, non-blank */
int extgetrealline(FILE *input, char *buff)
{
  int k;

  k = extgetline(input, buff);

  while ((*buff == '%' || *buff == '\n') && k >= 0)
    k = extgetline(input, buff);

  return k;
}

//////////////////////////////////////////////////////////////////////////////

int ksubst=0;         // number of entries in substitution table

char *makespace (char *s, int ndes, int nact)
{
  int k;

  for (k = nact; k < ndes; k++)
    *s++ = ' ';

  *s = '\0';

  return s;
}

char *showproper (char *s, int proper)
{
  if (proper == 0)
    return s;

  if ((proper & C_RESIDENT) != 0)
    sprintf(s, "%s ", RESIDENT);

  if ((proper & C_FORCESUB) != 0)
    sprintf(s, "%s ", FORCESUB);

  if ((proper & C_ALIASED) != 0)
    sprintf(s, "%s ", ALIASED);

  if ((proper & C_MISSING) != 0)
    sprintf(s, "*missing* ");

  if ((proper & C_UNUSED) != 0)
    sprintf(s, "*unused* ");

  if ((proper & C_SYNTHETIC) != 0)
    sprintf(s, "%s ", SYNTHETIC);

  if ((proper & C_MTMI) != 0)
    sprintf(s, "%s ", MTMI);

  if ((proper & C_EPSF) != 0)
    sprintf(s, "%s ", EPSF);

  if ((proper & C_DEPENDENT) != 0)
    sprintf(s, "*new-size* ");

  if ((proper & C_REMAPIT) != 0)
    sprintf(s, "%s ", REMAPIT);

  if ((proper & C_NOTBASE) != 0)
    sprintf(s, "*not-base* ");
 
  return s + strlen(s);
}

/* an experiment */
void show_sub_table(void)
{
  int k;
  char *s;
  char oldname[FNAMELEN];
  char newname[FNAMELEN];
  char vecname[FNAMELEN];

  showline("Font Substitution Table:\n", 0);

  for (k = 0; k < ksubst; k++)
  {
    if (fontsubfrom[k] != NULL)
      strcpy(oldname, fontsubfrom[k]);
    else
      *oldname = '\0';

    if (fontsubto[k] != NULL)
      strcpy(newname, fontsubto[k]);
    else
      *newname = '\0';

    if (fontsubvec[k] != NULL)
      strcpy(vecname, fontsubvec[k]);
    else
      *vecname = '\0';

    s = logline;
    sprintf(s, "%3d %s ", k, oldname);
    s += strlen(s);
    s = makespace(s, 10, strlen(oldname));
    sprintf(s, "=> %s ", newname);
    s += strlen(s);
    s = makespace(s, 16, strlen(newname));
    s = showproper(s, fontsubprop[k]);

    if (strcmp(vecname, "") != 0)
    {
      sprintf(s, "vec: %s", vecname);
      s += strlen(s);
    }

    strcat(s, "\n");
    showline(logline, 0);
  }

  showline("\n", 0);
}

/* recover TeX internal font number */
int original (int k)
{
  int m;

  for (m = 0; m < MAXFONTNUMBERS; m++)
  {
    if (finx[m] == (short) k)
      return m;
  }

  return -1;
}

/* an experiment */
static void show_font_table (void)
{
  int k, flag, originalfont;
  double atsize;
  int proper;
  char oldname[FNAMELEN];
  char newname[FNAMELEN];
  char *s = logline;

  strcpy(s, "Font Table:");
  s += strlen(s);

  if (mag != 1000)
    sprintf(s, " (Magnification %lg)", (double) mag / 1000);

  strcat(s, "\n");
  showline(logline, 0);
  
  for (k = 0; k < fnext; k++)
  {
    s = logline;
    proper = fontproper[k];
    sprintf(s, "%3d ", k);
    s += strlen(s);
    originalfont = original(k);   /* original TeX font number */

    if (originalfont >= 0)
      sprintf(s, "(%3d) ", originalfont);
    else
      s = makespace(s, 6, 0);

    if (fontname[k] != NULL)
      strcpy(oldname, fontname[k]);
    else
      *oldname = '\0';

    if (subfontname[k] != NULL)
      strcpy(newname, subfontname[k]);
    else
      *newname = '\0';

    sprintf(s, "%s ", oldname);
    s += strlen(s);
    s = makespace(s, 10, strlen(oldname));

    if (strcmp(newname, "") != 0 && strcmp(oldname, newname) != 0)
    {
      sprintf(s, "=> %s ", newname);
      s += strlen(s);
      s = makespace(s, 16, strlen(newname));
    }

    atsize = (double) fs[k] * num / den * 72.27 / 254000;

    if (atsize != 0.0)
      sprintf(s, "at:%6.5lg pt ", atsize);
    else if (proper & C_MULTIPLE)
      sprintf(s, "MM base font ");
    else if (proper & C_INSTANCE)
      sprintf(s, "MM instance ");
    else if (originalfont < 0)
      sprintf(s, "base for substitution ");
    else
      s = makespace(s, 13, 0);

    s +=strlen(s);
    flag = fontsubflag[k];

    if (flag >= 0) /* follow substitution pointer */
    {
      sprintf(s, "base: %2d ", flag);
      s +=strlen(s);
      originalfont = original(flag);

      if (originalfont >= 0)
        sprintf(s, "(%3d) ", originalfont);
      else
        s = makespace(s, 6, 0);

      s +=strlen(s);
    }

    s = showproper(s, proper);

    if (fontvector[k] != NULL)
    {
      strcat(s, fontvector[k]);
      s +=strlen(s);
    }

    strcat(s, "\n");
    showline(logline, 0);
  }

  showline("\n", 0);
}

/* New common routine 1992/Nov/28 */
/* does not use backslash if beginning is blank or already ends on : \ or / */

void make_file_name (char filepath[], char *fontname)
{
  char *s;

  if (strcmp(filepath, "") != 0)
  {
    s = filepath + strlen(filepath) - 1;

    if (*s != ':' && *s != '\\' && *s != '/')
      strcat(filepath, "\\");
  }

  strcat(filepath, fontname);
}

/* returns -1 if name was changed - returns 0 if name was not changed */

/* convert font file name to Adobe style */
int underscore (char *filename)
{
  int k, n, m;
  char *s, *t;

  s = removepath(filename);
  n = (int) strlen(s);

  if ((t = strchr(s, '.')) == NULL)
    t = s + strlen(s);

  m = t - s;

  if (m == 8)
  {
    return 0;
  }

  memmove(s + 8, t, (unsigned int) (n - m + 1));

  for (k = m; k < 8; k++)
    s[k] = '_';

  return -1;
}

/* removes underscores at end, assumes there is no file extension ... */
/* returns 0 if there were no underscores to remove - returns -1 otherwise */

/* remove Adobe style underscores */
int remove_under (char *filename)
{
  char *s;

  s = filename + strlen(filename) - 1;

  if (*s != '_')
    return 0;

  while (*s == '_')
    s--;

  *(s + 1) = '\0';        /* overwrite first underscore in seq */

  return -1;
}

/* Consolidated code for TFM, AFM, and PFM in one place 95/Mar/31 */

static FILE *look_for_metrics (char *font, char *extension, char *path)
{
  char fn_met[FNAMELEN];
  FILE *fp_met=NULL;
#ifndef SUBDIRSEARCH
  char *searchpath;
#endif

  if (traceflag)
  {
    sprintf(logline, " Trying %s", path); /* debug 95/Mar/31 */
    showline(logline, 0);
  }

#ifdef SUBDIRSEARCH
  strcpy(fn_met, font);
  forceexten(fn_met, extension);
  fp_met = findandopen(fn_met, path, NULL, "rb", currentfirst);

  if (fp_met == NULL && tryunderscore != 0)
  {
    if (underscore(fn_met))
      fp_met = findandopen(fn_met, path,  NULL, "rb", currentfirst);
  }
#else
  searchpath = path;

  for ( ; ; )
  {
    if ((searchpath=nextpathname(fn_met, searchpath)) == NULL)
      break;

    make_file_name(fn_met, font);     /* 1992/Nov/28 */
    forceexten(fn_met, extension);

    if ((fp_met = fopen(fn_met, "rb")) == NULL)
    {
      if (tryunderscore == 0)
        continue;
      else
      {
        if (underscore(fn_met))
        {
          if ((fp_met = fopen(fn_met, "rb")) != NULL)
            break;
        }

        continue;
      }
    }
    else
      break;             /* 1994/Aug/18 */
  }
#endif

  if (traceflag)
  {
    if(fp_met != NULL)
    {
      sprintf(logline, " Using %s", fn_met);
      showline(logline, 0);
    }
  }

  return fp_met;
}

/* get character widths from .tfm or .afm or .pfm files - for substitution */
/* tfm is searched first because it is compact - hence fast */
/* pfm is searched last because widths are restricted to being integers */
/* returns max numeric code of character in metric info */

static int read_widths (char *font, long widths[])
{
  FILE *fp_met = NULL;
  int k;

  task = "looking for font metrics";

  if (tfmpath != NULL)
  {
    fp_met = look_for_metrics(font, "tfm", tfmpath);

    if (fp_met != NULL)
    {
      k = readtfm(font, fp_met, widths);
      fclose(fp_met);

      return k;
    }
  }

  if (texfonts != NULL)
  {
    fp_met = look_for_metrics(font, "tfm", texfonts);

    if (fp_met != NULL)
    {
      k = readtfm(font, fp_met, widths);
      fclose(fp_met); 

      return k;
    }
  }

  if (afmpath != NULL)
  {
    fp_met = look_for_metrics(font, "afm", afmpath);

    if (fp_met != NULL)
    {
      k = readafm(font, fp_met, widths);
      fclose(fp_met);

      return k;
    }
  }

  if (pfmpath != NULL)
  {
    fp_met = look_for_metrics(font, "pfm", pfmpath);

    if (fp_met != NULL)
    {
      k = readpfm(font, fp_met, widths);
      fclose(fp_met);

      return k;
    }
  }

  sprintf(logline, " WARNING: metrics not found for %s", font);
  showline(logline, 1);
  errcount(0);

  return 0;
}

/* for remapping and substituting font names */

/* stuff for dealing with font file itself */
/* get next byte from input */
static int next_byte_in (FILE *input)
{
  int c, d;

  if (binaryin != 0)
  {
    if (len == 0)
    {
      c = getc(input);

      if (c != 0 && c != 128)
      {
        sprintf(logline, " Expecting %s, not %d", "binary length code", c);
        showline(logline, 1);
        extgiveup(3);
        return -1;
      }

      if (c == 128) /* PC .pfb file   */
      {
        c = getc(input);

        if (c == 1)
        {
          len = readlength(input);
          binaryin = 0;

          return next_byte_in(input); /* try reading in ASCII */
        }
        else if (c != 2)
        {
          sprintf(logline, " Expecting %s, not %d", "binary section code", c);
          showline(logline, 1);
          extgiveup(5);

          return -1;
        }

        len = readlength(input);
      }
      else /* Mac style binary file   c == 0 */
      {
        (void) ungetc(c, input);
        len = maclength(input);
        c = getc(input);

        if (c == 1) /* somewhat unexpected, but... */
        {
          binaryin = 0;

          return next_byte_in(input); /* try reading in ASCII */
        }
        else if (c != 2)
        {
          sprintf(logline, 
            " Expecting %s, not %d", "Mac binary section code", c);
          showline(logline, 1);
          extgiveup(5);

          return -1;
        }

        c = getc(input);

        if (c != 0)
        {
          sprintf(logline, " Invalid Mac style binary record %d", c);
          showline(logline, 1);
          extgiveup(15);

          return -1;
        }

        len = len - 2;
      }
    }

    if ((c = getc(input)) == EOF)
    {
      sprintf(logline, " Unexpected EOF (%s)\n", "next_byte_in");
      showline(logline, 1);
      extgiveup(7);

      return -1;
    }

    len--;

    return c;
  }
  else /* ASCII input */
  {
    c = getc(input);

    while (c <= ' ' && c != EOF)
      c = getc(input);

    if (c == EOF)
    {
      sprintf(logline, " Unexpected EOF (%s)\n", "next_byte_in");
      showline(logline, 1);
      extgiveup(7);

      return -1;
    }

    if (c >= '0' && c <= '9')
      c = c - '0';
    else if (c >= 'A' && c <= 'F')
      c = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
      c = c - 'a' + 10;
    else
    {
      sprintf(logline, " Invalid hex character: %d", c);
      showline(logline, 1);
      extgiveup(7);

      return -1;
    }

    d = getc(input);

    while (d <= ' ' && d != EOF)
      d = getc(input);

    if (d == EOF)
    {
      sprintf(logline, " Unexpected EOF (%s)\n", "next_byte_in");
      showline(logline, 1);
      extgiveup(7);

      return -1;
    }

    if (d >= '0' && d <= '9')
      d = d - '0';
    else if (d >= 'A' && d <= 'F')
      d = d - 'A' + 10;
    else if (d >= 'a' && d <= 'f')
      d = d - 'a' + 10;
    else
    {
      sprintf(logline, " Invalid hex character: %d", d);
      showline(logline, 1);
      extgiveup(7);

      return -1;
    }

    return (c << 4) | d;
  }
}

/* stuff for encrypting and decrypting */

unsigned char decrypt_byte (unsigned char cipher, unsigned short *crypter)
{
  unsigned char plain;

  plain = (unsigned char) ((cipher ^  (unsigned char) (*crypter >> 8)));
  *crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);

  return plain;
}

/* read byte and decrypt */
static unsigned char in_decrypt (FILE *input)
{
  unsigned char cipher;
  unsigned char plain;

  cipher = (unsigned char) next_byte_in(input);
  plain = (unsigned char) ((cipher ^ (unsigned char) (cryptin >> 8)));
  cryptin = (unsigned short) ((cipher + cryptin) * CRYPT_MUL + CRYPT_ADD);

  return plain;
}

// rewritten for efficiency to not use putc but fputs
// now accumulates a line of output in logline
// assumes we are not using logline for something else...

/* stuff for encrypted input and output */

/* encrypt and write */
void out_encrypt (unsigned char plain, FILE *output)
{
  int c, d;
  unsigned char cipher;
  char *s = logline + clm;

  cipher = (unsigned char) ((plain ^ (unsigned char) (cryptout >> 8)));
  cryptout = (unsigned short) ((cipher + cryptout) * CRYPT_MUL + CRYPT_ADD);

  d = cipher & 15;
  c = (cipher >> 4) & 15;
  if (c < 10)
  {
    *s++ = (char) (c + '0');
  }
  else
  {
    *s++ = (char) (c + 'A' - 10);
  }

  clm++;

  if (d < 10)
  {
    *s++ = (char) (d + '0');
  }
  else
  {
    *s++ = (char) (d + 'A' - 10);
  }

  clm++;

  if (clm >= columns)
  {
    *s++ = '\n';
    *s++ = '\0';
    PSputs(logline, output);
    clm = 0;
  }
}

static void flush_encrypt (FILE *output)
{
  char *s = logline + clm;

  if (clm > 0)
  {
    *s++ = '\n';
    *s++ = '\0';
    PSputs(logline, output);
    clm = 0;
  }
}

/* 93/Sep/14 --- avoid using get_en_line for this in case ^M or ^J */
/* get the magic encrypt start bytes */
static int get_magic (FILE *input, char *buff)
{
  char *s = buff;
  int k = 0;

  for (k = 0; k < 4; k++)
    *s++ = (char) in_decrypt(input);

  *s++ = '\0';

  return 4;
}

/* There shouldn't be any returns inside the encrypted part, just newlines */
/* But some stupid fonts disobey this rule => turn return into newline */
/* One problem: can't conveniently look ahead in encrypted part */
/* This gets nasty if there are returns FOLLOWED newlines */
/* so be prepared to see an isolated return or newline at start of line */

/* read encrypted line */
static int get_en_line (FILE *input, char *buff)
{
  char *s = buff;
  int d, k = 0;

  d = in_decrypt(input);
/*  if (d == '\r')  d = in_decrypt(input); */  /* bkph - 91/10/1 */
/*  step over initial return/newline - remanants or blank lines */
  while (d == '\n' || d == '\r')
    d = in_decrypt(input);

  while (d != '\n')
  {
    *s++ = (char) d;
    k++;

    if (k >= MAXLINE)
    {
      *s = '\0';
      showline(" Line: ", 1);
      showline(buff, 1);
      showline("\n", 0);
      sprintf(logline, " too long in encrypted getline (> %d)", MAXLINE);
      showline(logline, 0);
      extgiveup(6);

      return -1;
    }

    d = in_decrypt(input);

    if (d == '\r')
    {
      d = '\n';
    }
  }

  *s++ = (char) d; k++;
  *s++ = '\0';

  return k;
}

// need to remember to flush out logline at end also ...

/* This version assumes string is null terminated */
/* write encrypted line */
static void put_en_line (FILE *output, char *buff)
{
  int d; 

  while ((d = *buff++) != '\0') {
    out_encrypt((unsigned char) d, output);
  }
}

/* This version specifies length rather than null terminated */
/* write encrypted line */
static void put_en_linen (FILE *output, char *buff, int n)
{
  int d, k; 
  
  for (k = 0; k < n; k++)
  {
    d = *buff++;
    out_encrypt((unsigned char) d, output);
  }
}

/* used for reading "/charname n RD" for CharStrings */
/* also used for reading "dup n m RD" for Subrs */
/* normally returns zero, when hit end returns -1 */

/* May want to distinguish Subrs and CharStrings case */
/* because Subrs can end on "readonly def" */
/* but that can occur in CharStrings ... */

/* subrflag != 0 for Subrs reading added 93/Aug/13 */
/* get encrypted tokens */

/* reads until it hits RD, -|, end, ND, |-, noaccess def, readonly def */
/* which may mean it reads past end of line ... */

static int get_char_line (char *buff, FILE *input, int subrflag)
{
  int d;
  char *t = buff, *s = buff;

  d = in_decrypt(input);

  while (d == '\n' || d == '\r' || d == ' ' || d == '\0')
    d = in_decrypt(input);

  for(;;)
  {
    *s++ = (char) d;

    if (d <= ' ')
    {
      if(strncmp(t, "RD", 2) == 0 || strncmp(t, "-|", 2) == 0) /* ready for binary bytes */
      {
        *s = '\0';

        return 0;   /* start of binary section */
      }
      else if ((strncmp(t, "end", 3) == 0) ||  /* end of CharStrings */
           (strncmp(buff, "ND", 2) == 0) ||   /* or end of Subrs */
           (strncmp(buff, "|-", 2) == 0)) /* or end of Subrs */
      {
        *s = '\0';

        return -1;      /* end of Subrs or CharStrings */
      }
      else if (subrflag != 0 &&
        (strncmp(buff, "noaccess def", 12) == 0 ||
        strncmp(buff, "readonly def", 12) == 0))
      {
        *s = '\0';

        return -1;      /* end of Subrs or CharStrings */
      }
      else
        t = s;   /* remember start of next token */

    }

    d = in_decrypt(input);
  }
}

/* flush rest of CharString */
static void flush_char_string (FILE *input, int n)
{
  int k, d;
  
  for (k = 0; k < n; k++)
    (void) in_decrypt(input); /* flush binary part */

  d = in_decrypt(input);

  while (d != '\n')
  {
    d = in_decrypt(input);   /* flush ND or |- up to nl (or rt)*/

    if (d == '\r')
      d = '\n';  /* bkph 91/10/1 */
  }
}

/* copy CharString or Subr string */
/* the fix may be slightly dicey since it may generate blank line at */

static void copy_char_string (FILE *output, FILE *input, int n)
{
  int d, k; /* c, e */

  for (k = 0; k < n; k++) /* copy binary CharString itself */
  {
    d = in_decrypt(input);
    out_encrypt((unsigned char) d, output);
  }

  d = in_decrypt(input);   /* default is to drop space before ND */

  if (d != ' ' || keepgap != 0) /* 1993 August 5 */
    out_encrypt((unsigned char) d, output); 
    
  while (d != '\n') /* copy ND or | - up to nl (or rt)*/
  {
    d = in_decrypt(input);

    if (d == '\r')
      d = '\n';    /* bkph 91/10/1 */

    out_encrypt((unsigned char) d, output);    
  }
}

/* do we want char with this name ? */
/* return negative if not - and character code if yes */ /* changed */

/* Modified for now to keep on looking */ /* allow for multiple encoding ? */ 
/* this will slow things down a bit, but be a lot safer ! */
/* make sure charnames[] is cleaned out before encoding is read from font */

static int want_this_name (char *charname, int k, char *wantchrs)
{
  int i;

  /*  best guess first for speed: */
  if (k >= 0 && k < MAXCHRS && strcmp(charnames[k], charname) == 0)
  {
    if (wantchrs[k] != 0)
      return k; /* nice and easy ! */
  }

  for (i = 0; i < fontchrs; i++)
  {
    if (strcmp(charnames[i], charname) == 0)
    {
      if (wantchrs[i] != 0)
        return i;
    }
  }

  if (wantnotdef != 0 && strcmp(".notdef", charname) == 0)
  {
    return NOTDEF; 
  }

  return -1;
}

/* Adobe PS interpreters yield `invalidfont' errors when  */
/* base or accent of composite character is not in encoding of font */
/* May need to add more characters to basecharacterlist */
/* use `N' command line argument to deactivate this bug work around */

static int copy_subrs (FILE *output, FILE *input)
{
  int subrnum, nbin;
/*  char buffer[FNAMELEN]; */ /* compromise only for hires Subrs line */
  char *s;

  /*  First check whether there are no Subrs ! */
  if (strstr(line, "ND") != NULL)
    return 0;

  if (strstr(line, "|-") != NULL)
    return 0;

  if (strstr(line, "noaccess def") != NULL)
    return 0;

  if (strstr(line, "readonly def") != NULL)
    return 0;

  while (get_char_line(line, input, 1) == 0)
  {
    if (sscanf(line, "dup %d %d RD", &subrnum, &nbin) < 2)
    {
      if (hybridflag == 0)
      {
        sprintf(logline, " Not a Subrs line: %s", line);
        showline(logline, 1);
        extgiveup(9);
        return -1;
      }
      else
      {
/* Note: this `line' (get_char_line) contains multiple lines up to dup ... RD */
        if ((s = strstr(line, "dup ")) != NULL)
        {
          if (sscanf(s, "dup %d %d RD", &subrnum, &nbin) == 2)
          {
            *s = '\0';
            put_en_line(output, line);
            *s = 'd';
            strcpy(line, s);
          }
        }
      }
    }

    put_en_line(output, line);    /* beginning of subr */
    copy_char_string(output, input, nbin);
  }

  s = line + strlen(line) -1; /* to solve problem with \r\n */
  if (*s == '\r')
    *s = '\n';  /* extra blank line maybe */

  put_en_line(output, line);  /* hit the end */

  return 0;
}

/* Tries to find encoding vector first in dvi file directory */
/* - then tries default encoding vector directory */

static FILE * open_vector (char *vector)
{
  FILE *fp_vec;
  char fn_vec[FNAMELEN];
  char *s, *t;
  
/*  if vector contains a path, use it directly - no other trials */
  if (strpbrk(vector, "\\:/") != NULL)
  {
    strncpy(fn_vec, vector, FNAMELEN);
    extension(fn_vec, "vec"); 
    fp_vec = fopen(fn_vec, "r");

    if (fp_vec != NULL)
    {
      if (traceflag)
      {
        sprintf(logline, "Using encoding vector %s\n", fn_vec);
        showline(logline, 0);
      }

      return fp_vec;
    }

    return NULL;
  }

  /*  try first in dvi file directory */
  if (dvipath != NULL)
    strcpy(fn_vec, dvipath);
  else
    strcpy(fn_vec, "");

  make_file_name(fn_vec, vector);
  extension(fn_vec, "vec");

  if ((fp_vec = fopen(fn_vec, "r")) != NULL)
  {
    if (traceflag)
    {
      sprintf(logline, "Using encoding vector %s\n", fn_vec);
      showline(logline, 0);
    }

    return fp_vec;
  }

  s = vecpath;

  for( ; ; )
  {
    if (*s == '\0')
      break;        /* safety valve */

    strcpy(fn_vec, s);
    t = strchr(fn_vec, ';');

    if (t != NULL)
      *t = '\0';     /* isolate one directory path */

    make_file_name(fn_vec, vector);
    extension(fn_vec, "vec");

    if ((fp_vec = fopen(fn_vec, "r")) != NULL)
    {
      if (traceflag)
      {
        sprintf(logline, "Using encoding vector %s\n", fn_vec);
        showline(logline, 0);
      }

      return fp_vec;
    }

    if (t != NULL)
      s += (t - fn_vec) + 1;  /* step over dir and ; */
    else
      break;
  }

  strcpy(fn_vec, vecpath);
  make_file_name(fn_vec, "vec\\");  /* subdirectory "vec" */
  strcat(fn_vec, vector);
  extension(fn_vec, "vec");

  if ((fp_vec = fopen(fn_vec, "r")) != NULL)
  {
    if (traceflag)
    {
      sprintf(logline, "Using encoding vector %s\n", fn_vec);
      showline(logline, 0);
    }

    return fp_vec;
  }

  *fn_vec = '\0';
  make_file_name(fn_vec, vector);
  extension(fn_vec, "vec");

  if ((fp_vec = fopen(fn_vec, "r")) != NULL)
  {
    if (traceflag)
    {
      sprintf(logline, "Using encoding vector %s\n", fn_vec);
      showline(logline, 0);
    }

    return fp_vec;
  }

  return NULL;
}

/*  Clear out charnames */
static void clean_enc (int start, int end)
{
  int k;

  namestring[0] = '\0';         /* empty string */
  stringindex = 1;            /* reset index to next space */

  for (k = start; k < end; k++)
    charnames[k] = namestring;
}

static void copy_enc (char *charnames[], char *encoding[], int n)
{
  int k;

  if (n < 0 || n > 256)
  {
    sprintf(logline, " ERROR in copy_enc %d\n", n);
    showline(logline, 1);

    return;
  }

  for (k = 0; k < n; k++)
    charnames[k] = encoding[k];
}

/* we have duplication here if repeated encoding in vector */

static void add_enc (int k, char *charname)      /* 93/Nov/15 */
{
  int n = strlen(charname) + 1; /* space needed */

  if (stringindex + n >= STRINGSPACE)
  {
    showline(" ERROR: encoding vector too long\n", 1);
  }
  else
  {
    charnames[k] = namestring + stringindex;    /* ptr */
    strcpy (namestring + stringindex, charname);  /* copy */
    stringindex += n;               /* step over */
  }
}

/* now return 0 if successful, non-zero if failed */

static int read_enc (char *vector)
{
  char charname[FNAMELEN];    /* just to be safe */

  FILE *fp_vec;
  int n;              /* not accessed */
  int k;

  clean_enc(0, MAXCHRS);        /* 93/Nov/15 */
  
  if (vector == NULL)
  {
    showline(" WARNING: No encoding vector specified\n", 1);

    vector = "textext";
    copy_enc(charnames, tex_text, TEXCHRS);
    errcount(0);

    return -1;
  }

  if ((fp_vec = open_vector(vector)) != NULL)
  {
    n = 0;

    while (getrealline(fp_vec, line) > 0)
    {
      if (*line == '%' || *line == ';')
        continue;

      if (sscanf(line, "%d %s", &k, charname) < 2)
      {
        showline(" Don't understand encoding line: ", 1);
        showline(logline, 1);
        showline(line, 1);
      } 
      else if (k >= 0 && k < MAXCHRS)
      {
        add_enc(k, charname);
      }

      n++;
    }

    fclose(fp_vec);
  }
  else
  {
    copy_enc(charnames, tex_text, TEXCHRS);
    sprintf(logline, " WARNING: can't find encoding vector %s ", vector);
    showline(logline, 1);
    perrormod(vector);
    errcount(0);

    return -1;        /* failed */
  }

  return 0;         /* succeeded */
}

static void write_vector (FILE *fp_out, char *vector, int n)
{
  int k, knext=0;   /* n is MAXCHRS */
  int kn;
/*  char *s, *svector; */

  if (vector == NULL)
  {
    sprintf(logline, " ERROR in write_vector %d\n", n);
    showline(logline, 1);

    return;
  }

  if (n < 0 || n > 256)
  {
    sprintf(logline, " ERROR in write_vector %d\n", n);
    showline(logline, 1);

    return;
  }

  if (strcmp(vector, "textext") == 0)
  {
    if (textextwritten++ > 0)
      return;
  }

  if (strcmp(vector, textencoding) == 0)
  {
    if (ansiwritten++ > 0)
      return;
  }

  knext = 256;

  if (bSuppressPartial == 0 && bForceFullArr == 0)
  {
    for (k = n-1; k >= 0; k--)
    {
      if(strcmp(charnames[k], "") != 0)
      {
        knext = k + 1;
        break;
      }
    }
  }

  if (knext == 0)
    return;

  if (stripcomment == 0)
  {
    sprintf(logline, "%% %s encoding\n", vector);
    PSputs(logline, fp_out);
  }

  sprintf(logline, "/%s[", removepath(vector));
  PSputs(logline, fp_out);

  for (k = 0; k < knext; k++)
  {
    if (k != 0 && k % 8 == 0)
    {
      PSputc('\n', fp_out);
    }

    if (strcmp(charnames[k], "") != 0)
    {
      sprintf(logline, "/%s", charnames[k]);
      PSputs(logline, fp_out);
    }
    else
    {
      kn = k + 1;

      while (kn < knext && strcmp(charnames[kn], "") == 0)
        kn++;

      if (kn < k + 5)
      {
        for (k=k; k < kn; k++)
        {
          PSputs(" n", fp_out);
        }
      }
      else
      {
        sprintf(logline, " %d notdef", kn-k);
        PSputs(logline, fp_out);
      }

      if ((((kn-1) >> 3) != (k >> 3)) && (kn % 8) != 0)
      {
        PSputc('\n', fp_out);
      }

      k = kn - 1;
    }
  }

  PSputs("]def\n", fp_out);
}

static void write_dvi_start (FILE *fp_out)
{
  PSputs("dvidict begin\n", fp_out);
}

static void write_dvi_end (FILE *fp_out)
{
  PSputs("end", fp_out);

  if (stripcomment == 0)
  {
    PSputs(" % dvidict", fp_out);
  }

  PSputc('\n', fp_out);
}

/* Following used to be in preamble: */

/* /dviencoding 256 array def */
/* /dvicodemake{string cvs dup 0 97 put cvn dviencoding 3 1 roll put}bd */
/* 0 1 9{dup 10 add 2 dvicodemake} for */
/* 10 1 99{dup 100 add 3 dvicodemake} for */
/* 100 1 255{dup 1000 add 4 dvicodemake} for */

void write_dvi_encode (FILE *fp_out)
{
  int k;
  char charname[5];   /* space for a255 + zero */

  write_dvi_start(fp_out);
  clean_enc(0, MAXCHRS);        /* reset string table */

  for (k = 0; k < MAXCHRS; k++) /* fixed 1994/Feb/3 */
  {
    sprintf(charname, "a%d", k);
    add_enc (k, charname);
  }

  write_vector(fp_out, "dviencode", MAXCHRS); 
  write_dvi_end(fp_out);        /* 1993/Sep/30 */
}

void writetextext (FILE *fp_out)
{
  write_dvi_start(fp_out);
  copy_enc(charnames, tex_text, TEXCHRS);

  if (bForceFullArr == 0)
    write_vector(fp_out, "textext", TEXCHRS);
  else
  {
    clean_enc(TEXCHRS, MAXCHRS);  /* redundant ? */
    write_vector(fp_out, "textext", MAXCHRS);
  }

  write_dvi_end(fp_out);
}

/* Write Windows ANSI encoding or what user defined in ENCODING env var */

void write_ansi_code (FILE *fp_out, char *textenconame)  /* 1995/Feb/3 */
{
  write_dvi_start(fp_out);
  copy_enc(charnames, ansi_enc, MAXCHRS);
  write_vector(fp_out, textenconame, MAXCHRS);
  write_dvi_end(fp_out);
}

/*  overwrite Windows ANSI encoding hard-wired in here */

int read_text_encode(char *textencoding)
{
  int k;

  if (read_enc(textencoding) == 0)
  {
    for (k = 0; k < MAXCHRS; k++)
    {
      ansi_enc[k] = zstrdup(charnames[k]);

      if (ansi_enc[k] == NULL)
      {
        showline("Unable to allocate memory\n", 1);
        checkexit(1);

        return -1;
      }
    }
  }

  return 0;
}

#define ACCENTCHRS 15

#define BASECHRS (26+26)

/* Adjust wantchrs table so base and accent of composites are there */
/* (i) we MUST have the base and accent character CharStrings */
/* (ii) for present Adobe interpreters they must ALSO be in encoding */
/* so its easiest to implement this by adjusting wantchrs */

/* keep track of which accents already dealt with in accenthit */
/* (based on StandardEncoding position of accent - 193) */
/* keep track of which base chars already dealt with in basehit */
/* (based on position of base in string `basecharacters') */

/* returns non-zero if any accented character were found */

int expand_accents (char *wantchrs)
{
  char *testname;
  char basename[9];     /* dotlessi is max length */
  int i, j, k, c, n;
  int compflag, foundflag;
  int composedflag=0;     /* any composites ? */
  char *standname;      /* 93/Sep/16 */
  int accenthit[ACCENTCHRS];        /* for accents from 193 to 207 */
  int basehit[BASECHRS];        /* for 26 + 26 base characters */

  for (k = 0; k < ACCENTCHRS; k++)
    accenthit[k] = 0;    /* 94/Feb/17 */

  for (k = 0; k < BASECHRS; k++)
    basehit[k] = 0;      /* 94/Feb/17 */

  for (k = 0; k < MAXCHRS; k++)
  {
    if (wantchrs[k] == 0)
      continue;   /* ignore unwanted characters */

    testname = charnames[k];      /* potential composite char */
    n = strlen(testname);

    if (n < 4 || n > 14 || n == 13)
      continue; /* ldot --- uhungarumlaut */

    c = testname[0];

    if (c < 'A')
      continue;
    else if (c <= 'Z')
      n = c - 'A';
    else if (c < 'a')
      continue;
    else if (c <= 'z')
      n = c - 'a' + 26;
    else
      continue;

    compflag = 0;
    
    /* NOTE: we only need to worry about accents in StandardEncoding ! */
    for (i = 193; i <= 207; i++) /* check through accents */
    {
      if (strcmp(testname + 1, std_enc[i]) == 0)
      {
        compflag = 1;
        break;    /* found accent in table */
      }
    }
    
    /* What about hungarumlaut => hungar and dotaccent => dot */
    if (compflag == 0)
    {
      if (strcmp(testname + 1, "dot") == 0)
      {
        i = 199;    /* dotaccent */
        compflag = 1;
      }
      else if (strcmp(testname + 1, "hungar") == 0)
      {
        i = 205;    /* hungarumlaut */
        compflag = 1;
      }
      else if (strcmp(testname + 1, "dblacute") == 0)
      {
        i = 205;    /* hungarumlaut */
        compflag = 1;
      }
      else if (strcmp(testname + 1, "hacek") == 0)
      {
        i = 207;    /* caron */
        compflag = 1;
      }
    }

    if (compflag != 0) /* So, *is* it a composite character ? */
    {
      composedflag = 1;
      
      /*  speed up: 94/Feb/17 keep track of which already inserted - in basehit */
      if (basehit[n] == 0)  /* see whether already dealt with */
      {
        basehit[n]++;     /* note that we have been here */

        if (c == 'i')
        {
          strcpy(basename, "dotlessi");
          c = 245;  /* standard encoding position for dotlessi */
        }
        else
        {
          basename[0] = (char) c;
          basename[1] = '\0';
        }

        if (strcmp(charnames[c], basename) == 0) /* fast case */
        {
          wantchrs[c] = 1;
        }
        else  /* otherwise have to search for it */
        {
          foundflag = 0;

          for (j = 0; j < MAXCHRS; j++)
          {
            if (strcmp(charnames[j], basename) == 0)
            {
              wantchrs[j] = 1;
              foundflag = 1;
              break;
            }
          }

          if (foundflag == 0)
          {
            sprintf(logline, " `%s' not in encoding", basename);
            showline(logline, 1);
            errcount(0);
          }
        }
      }
      
      /* then request accent character */
      /* speed up: 94/Feb/17 keep track of which already inserted - in accenthit */
      if (accenthit[i - 193] == 0) /* check if already dealt with */
      {
        accenthit[i - 193]++;   /* mark that we dealt with this */
        standname = std_enc[i];      /* 93/Sep/13 */

        if (strcmp(charnames[i], standname) == 0)
        {
          wantchrs[i] = 1;
        }
        else
        {
          foundflag = 0;

          for (j = 0; j < MAXCHRS; j++)
          {
            if (strcmp(charnames[j], standname) == 0)
            {
              wantchrs[j] = 1;
              foundflag = 1;

              break;
            }
          }

          if (foundflag == 0)
          {
            sprintf(logline, " `%s' not in encoding", standname);
            showline(logline, 1);
            errcount(0);
          }
        }
      }
    }
  }

  return composedflag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Check whether font calls for characters *not* found in encoding! */
/* 1995/July/15 */

int missing_chars (char *wantchrs, char *encoding)
{
  int k, unknowns = 0;

  for (k = 0; k < fontchrs; k++)
  {
    if (wantchrs[k] != 0 && charnames[k][0] == '\0')
      unknowns++;
  }

  /*  Encoding passed in may be just "" if read from PFA file 96/May/26 */
  if (unknowns > 0)
  {
    sprintf(logline, " ERROR: %d character%s used not in ",
      unknowns, (unknowns == 1) ? "" : "s");
    showline(logline, 1);

    if (encoding != NULL)
    {
      sprintf(logline, "`%s'", encoding);
      showline(logline, 0);
    }
    else
      showline("encoding", 0);

    errcount(0);
  }

  return unknowns;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* could keep a record of which accent we already complained about */

/* end of code for dealing with composite characters encoding */

/* write out encoding - cheaply if possible */
/* worry about remapped fonts ? */

/* POSSIBLE PROBLEM: synthetic font, where /FontName appears after /Encoding */
/* not sure it is really worth worrying about... */

/* Maybe simplify this if StandardEncoding and no remapping ??? */

/* void write_enc(FILE *output, char wantchrs[], int syntheticflag) { */

/* This also adjusts wantchrs to reflect base and accent characters */

/* why aren't we just doing a return instead of setting `done' */

void write_enc(FILE *output, char *wantchrs, int syntheticflag, char *encoding)
{
  int k, standardok=0, textextok=0, ansiok=0, nullchar=0;
  int composedflag=0;     /* if composite characters used */

/*  unless 'N' flag used, force in base and accent characters */

  if (accentedflag != 0)
    composedflag = expand_accents(wantchrs);
  else
    composedflag = 0;

  if (strcmp(encoding, "standard") == 0)
  {
    PSputs("/Encoding StandardEncoding def\n", output); 
    return;           /* 94/Oct/25 */
  }

  /*  Just refer to encoding by name if permitted to do so */
  if (strcmp(encoding, "") != 0)
  {
    if (bAllowShortEncode)
    {
      sprintf(logline, "/Encoding %s def\n", removepath(encoding));
      PSputs(logline, output);

      return;   /*  done = 1; */
    }
    else
      goto writefull; /* need to write out encoding in full!  */
  }

/*  do not allow use of dviencoding if accented characters seen */

/*  Construct new encoding vector */
/*  Check whether numeric vector will do *//*  most efficient - if allowed */
/*  but we don't like this anymore because of clone problems ... */
/*  if (done == 0 && composedflag == 0 &&  */
  if (composedflag == 0 && 
      syntheticflag == 0 &&   /* do we need to worry about this ? */
        busedviencode != 0)
  {
    PSputs("/Encoding dviencoding def\n", output);
    return;   /*    done = 1; */
  }

/*  added bSuppressPartial == 0 reference 1992/Sep/12 */
/*  added bForceFullArr == 0 reference 1993/Feb/13 */
/*  took out bForceFullArr == 0 reference 1993/Sep/30 by extending tex_text */ 

/*  check whether tex_text will do */
/*  if (done == 0 && composedflag == 0 && bAllowTexText != 0 && */
  if (composedflag == 0 && bAllowTexText != 0 &&
/*      bSuppressPartial == 0 && bForceFullArr == 0) { */
      bSuppressPartial == 0)
  {
    textextok = 1;

    for (k = fontchrs-1; k >= 0; k--)
    {
      if (wantchrs[k] != 0)
      {
        if (k >= TEXCHRS ||       /* 93/Oct/2 */
          charnames[k][0] == '\0' ||  /* 95/July/15 */
          strcmp(charnames[k], tex_text[k]) != 0)
        {
          textextok = 0; break;
        }
      }
    }

    if (textextok != 0)
    {
      PSputs("/Encoding textext def\n", output);
      return;   /*  done = 1; */
    }
  }

/*  check whether Windows ANSI will do 1993/Sep/30 */

  if (bWindowsFlag != 0 && bAllowANSI != 0 &&   /* 94/Oct/25 */
      bSuppressPartial == 0)
  {
    ansiok = 1;

    for (k = fontchrs-1; k >= 0; k--)
    {
      if (wantchrs[k] != 0)
      {
        if (charnames[k][0] == '\0' ||  /* 95/July/15 */
          strcmp(charnames[k], ansi_enc[k]) != 0)
        {
          ansiok = 0; break;
        }
      }
    }

    if (ansiok != 0)
    {
      sprintf(logline, "/Encoding %s def\n",
        textenconame);
      PSputs(logline, output);
      return;   /* done = 1; */
    }
  }

/*  check whether StandardEncoding will do */
/*  if (done == 0 && bAllowStandard != 0) { */
  if (bAllowStandard != 0 &&
      bSuppressPartial == 0)
  {
    standardok = 1;

    for (k = fontchrs-1; k >= 0; k--)
    {
      if (wantchrs[k] != 0)
      {
        if (charnames[k][0] == '\0' ||  /* 95/July/15 */
          strcmp(charnames[k], std_enc[k]) != 0)
        {
          standardok = 0; break;
        }
      }
    }

    if (standardok != 0)
    {
      PSputs("/Encoding StandardEncoding def\n", output);
      return;   /*    done = 1; */
    }
  }

writefull:

/*  Couldn't use standard, tex_text, or Windows ANSI for some reason */

  k = fontchrs-1;         /* use full if partial suppress */

  if (bSuppressPartial == 0 && bForceFullArr == 0)
  {
    for (k = fontchrs-1; k >= 0; k--)
      if (wantchrs[k] != 0)
        break;
  }

  sprintf(logline, "/Encoding %d array", k+1);
  PSputs(logline, output);
  PSputc('\n', output);
  sprintf(logline, "0 1 %d {1 index exch /.notdef put} for\n", k);
  PSputs(logline, output);
    
  for (k = 0; k < fontchrs; k++)
  {
    if (wantchrs[k] != 0 || (bSuppressPartial != 0 && charnames[k][0] != 0))
    {
      if (charnames[k][0] != '\0')
      {
        sprintf(logline, "dup %d /%s put\n", k, charnames[k]);
        PSputs(logline, output);
      }
      else
      {
        nullchar++;
      }
    }
  }

  PSputs("readonly def\n", output);

  if (nullchar > 0)
  {
    if (traceflag)
    {
      sprintf(logline, " %d null char names", nullchar);
      showline(logline, 1);
    }
  }

  return;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*  cm-text-fonts cm
  (r|bx|tt|sltt|vtt|tex|ss|ssi|ssdc|ssbx|ssqi|dunh|bxsl|b|ti|bxti|csc|tcsc)
  ([0-9]+) */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void begin_resource (FILE *output, char *filefontname)
{
  if (stripcomment == 0)
  {
    PSputs("%%BeginResource: ", output);
    PSputs("font ", output);
    PSputs(filefontname, output);
    PSputc('\n', output);
  }
}

int end_resource (FILE *output)
{
  if (stripcomment == 0)
  {
    PSputs("%%EndResource\n", output);
  }

  if (output != NULL && ferror(output))
  {
    showline("\n", 0);
    showline("ERROR in output file", 1);
    perrormod((outputfile != NULL) ? outputfile : "");
    extgiveup(7);

    return -1;
  }

  return 0;
}

/* Just copy if we don't recognize the format ... no remapping  */
/* --- pretty desperate move ! */ /* don't try to be efficient */
/* int i is font number in case needed for error message */

void copy_unknown (FILE *output, FILE *input, int i, char *fontnamek)
{
  int c;

  if (verboseflag)
  {
    showline(fontnamek, 0);
  }
  else
  {
    showline("*", 0);
  }

  showline(" WARNING: Font type not recognized!\n", 1);

  while ((c = getc(input)) != EOF)
  {
    PSputc(c, output);
  }
}

/* copy ascii section of a compressed file - old version */
/* returns EOF if EOF hit */

int copy_ascii (FILE *output, FILE *input, long n, int firstline)
{
  int c;

  c = getc(input);
  n--;

  if (c == '\r' && flushcr != 0)
  {
    c = getc(input);
    n--;

    if (c != '\n')
    {
      (void) ungetc(c, input); n++;
      c = '\n';
    }
  }
  else if (firstline == 0)
  {
    PSputc('\n', output);
  }

  while (c != EOF)
  {
    if (c == 128 || c == 0)
    {
      (void) ungetc(c, input);  n++;
      break;
    }

    if (c == '\n')
      clm = 0;

    PSputc(c, output);
    c = getc(input);
    n--;

    if (c == '\r' && flushcr != 0)
    {
      c = getc(input);
      n--;

      if (c != '\n')
      {
        (void) ungetc(c, input); n++;
        c = '\n';
      }
    }
  }

  if (c == EOF)
  {
    sprintf(logline, " Unexpected EOF (%s)\n", "copyascii");
    showline(logline, 1);

    return EOF;
  }

  if (n != 0)
  {
    sprintf(logline, " Length discrepancy %ld", n);
    showline(logline, 1);
  }

  return 0;
}  

/* copy ascii section of a compressed file - strip {restore}if */
/* returns EOF if EOF hit */ /* special kludge for wrapped synthetic fonts */

int copy_strip (FILE *output, FILE *input, long n, int firstline)
{
  int c;
  char *s, *t;

  c = getc(input);
  n--;

  if (c == '\r' && flushcr != 0)
  {
    c = getc(input);
    n--;

    if (c != '\n')
    {
      (void) ungetc(c, input); n++;
      c = '\n';
    }
  }
  else if (firstline == 0)
  {
    PSputc('\n', output);
  }

  s = line;

  while (c != EOF)
  {
    if (c == 128 || c == 0)
    {
      (void) ungetc(c, input);
      n++;
      break;
    }

    if (c == '\n' || c == '\r')
    {
      *s++ = (char) c;
      *s = '\0';        /* terminate line */

      if ((s = strstr(line, "{restore}")) != NULL)
      {
        if ((t = strstr(line, "cleartomark")) != NULL)
        {
          *s++ = (char) c;
          *s++ = '\0';      /* cut off after cleartomark */
          PSputs(line, output); /* 1992/Aug/20 */
        }

        if (stripcomment == 0)
        {
          PSputs("% font wrapper removed\n", output);
        }
      }
      else
      {
        PSputs(line, output); /* finally output it */
      }

      s = line;       /* reset line buffer */
      clm = 0;
    }
    else
      *s++ = (char) c;   /* check for line length exceeded ? */

    c = getc(input); n--;   /* get next character */

    if (c == '\r' && flushcr != 0)
    {
      c = getc(input);
      n--;

      if (c != '\n')
      {
        (void) ungetc(c, input);
        n++;

        c = '\n';
      }
    }
  }

  if (c == EOF)
  {
    sprintf(logline, " Unexpected EOF (%s)\n", "copystrip");
    showline(logline, 1);

    return EOF;
  }

  if (n != 0)
  {
    sprintf(logline, " Length discrepancy %ld", n);
    showline(logline, 1);
  }

  return 0;
}  

/* copy binary section of a compressed file */

int copy_binary (FILE *output, FILE *input, long n)
{
  int c, d;
  long k;
  
  for (k = 0; k < n; k++)
  {
    c = getc(input);

    if (c == EOF)
    {
      sprintf(logline, " Unexpected EOF (%s)\n", "copybinary");
      showline(logline, 1);

      return -1;
    }

    d = c & 15; c = (c >> 4) & 15;

    if (c > 9) c = c + 'A' - 10; else c = c + '0';

    if (d > 9) d = d + 'A' - 10; else d = d + '0';

    PSputc(c, output);
    PSputc(d, output);

    if ((clm += 2) >= columns)
    {
      PSputc('\n', output);
      clm = 0;
    }
  }

  return 0;
} 

void copy_pfa (FILE *output, FILE *input, int stripflag)
{
  int c;
  char *s, *t;
  
  if (stripflag == 0)
  {
    while ((c = getc(input)) != EOF)
    {
      if (c == '\r' && flushcr != 0)
      {
        c = getc(input);

        if (c != '\n')
        {
          (void) ungetc(c, input);
          c = '\n';
        }
      }

      PSputc(c, output);
    }
  }
  else   /* need to watch for closing wrapper */
  {
    while (extgetline (input, line) != EOF)
    {
      if ((s = strstr(line, "{restore}")) != NULL)
      {
        if ((t = strstr(line, "cleartomark")) != NULL)
        {
          *s++ = '\n';
          *s++ = '\0';      /* cut off after cleartomark */
          PSputs(line, output);
        }

        continue;         /* just flush it */
      }

      PSputs(line, output);
    }
  }
}

/* special purpose hack for fonts in PFA format that suck rocks */
/* flush this eventually ... */

void copy_mtmi (FILE *output, FILE *input, int stripflag)
{
  char *s, *t;

  if (stripflag == 0)
    showline(" no wrapper ", 1);

  while (extgetline (input, line) != EOF)
  {
    if ((s = strstr(line, "{restore}")) != NULL)
    {
      if ((t = strstr(line, "cleartomark")) != NULL)
      {
        *s++ = '\n';
        *s++ = '\0';
        PSputs(line, output);
      }

      continue; 
    }

    PSputs(line, output);
  }
}

/* This is typically called after already wading into the ASCII section */
/* but only if syntheticflag or mtmiflag non-zero */
/* int i is font number in case needed for error message */

void copy_font (FILE *output, FILE *input, int i, int mtmiflag, int stripflag)
{
  int c, d, firstline = 1;
  int ascii=1, binary=0;    /* section counts */ /* presently not used */
  long n;

  firstline = 0;        /* since we already past start of file ! */
  c = getc(input);      /* skip over initial white space */

  while (c == '\n' || c == '\r' || c == ' ')
    c = getc(input); 

  (void) ungetc(c, input);
  clm = 0;

  if (c == 0)    /* assume MacIntosh format */
  {
    n = maclength(input);

    c = getc(input);
    d = getc(input);   /* d should be zero */

    while (c != 5)
    {
      if (c == 1)
      {
        ascii++;

        if (copy_ascii(output, input, n - 2, firstline) != 0)
          break;

        firstline = 0;
      }
      else if (c == 2)
      {
        binary++;

        if (copy_binary(output, input, n - 2) != 0)
          break;
      }
      else
      {
        sprintf(logline, " Unrecognized section code %d", c);
        showline(logline, 1);

        break;
      }

      n = maclength(input);
      c = getc(input);
      d = getc(input);

      if (c == EOF)
      {
        sprintf(logline, " Unexpected EOF (%s)\n", "copy_font");
        showline(logline, 1);

        break;
      }
    }
  }
  else if (c == 128) /* assume PFB format */
  {
    c = getc(input);
    d = getc(input);

    while (d != 3)
    {
      n = readlength(input);

      if (d == 1)    /* ASCII section code */
      {
        ascii++;

        if (stripflag != 0 && ascii > 1)
        {
          if (copy_strip(output, input, n, firstline) != 0)
            break;
        }
        else
        {
          if (copy_ascii(output, input, n, firstline) != 0)
            break;
        }

        firstline = 0;
      }
      else if (d == 2) /* binary section code */
      {
        binary++;

        if (copy_binary(output, input, n) != 0)
          break;
      }
      else
      {
        sprintf(logline, " Unrecognized section code %d", d);
        showline(logline, 1);

        break;
      }

      c = getc(input);
      d = getc(input);

      if (c == EOF)
      {
        sprintf(logline, " Unexpected EOF (%s)\n", "copy_font");
        showline(logline, 1);

        break;
      }
    }
  }
  else if ((c >= '0' && c <= '9') || 
    (c >= 'A' && c <= 'F') ||
    (c >= 'a' && c <= 'f')) /* assume PFA format */
  {
    if (mtmiflag == 0)
      copy_pfa (output, input, stripflag); /* normal PFA */
    else
      copy_mtmi (output, input, stripflag);   /* MTMI */
  }
  else
  {
    sprintf(logline, " Don't understand font file format %d", c);
    showline(logline, 1);
    errcount(0);
  }

  end_resource(output);            /* share code */
}

/* Use the following more in future to avoid dependency on line breaks ? */

/* grab next white space delimited token in line */ /* read line if needed */
/* assumes pump has been primed, i.e. line read and strtok called once */

char *grab_next_token(FILE *input, char *line)
{
  char *str = NULL, *s;

  for ( ; ; )
  {
    while ((s = strtok(str, " \t\n\r")) == NULL)
    {
      for ( ; ; )   /* need to grab a new line then */
      {
        if (extgetrealline(input, line) < 0)
          return NULL; /* EOF */
        
        /* ignore comments and blank lines - continue round the loop */
        if (*line != '%' && *line != '\n' && *line != '\r')
          break;
      }

      str = line;
    }

    if (*s != '%')
      break;   /* escape if not comment */

    /* following added to strip comments off ends of lines 1992/Sep/17 */
    for ( ; ; ) /* need to grab a new line then */
    {
      if (extgetrealline(input, line) < 0)
        return NULL; /* EOF */

      /* ignore comments and blank lines - continue round the loop */
      if (*line != '%' && *line != '\n' && *line != '\r')
        break;
    }

    str = line;
  }

  return s;
}

/* new tokenized version follows */
int gobble_enc (FILE *input)
{
  int chr, c, n;
  int base=10;
  char *s, *t;

/*  clean_enc(0, MAXCHRS);  */

/*  may want to remove some debugging error message output later ... */
  s = strtok(line, " \t\n\r");  /* start the pipeline */

  for ( ; ; ) /*  exit if hit `readonly' or `def' ??? */
  {
    if (strcmp(s, "dup") != 0)
    {
      if (strcmp(s, "readonly") == 0 || strcmp(s, "def") == 0)
        break; /* normal exit */

      sprintf(logline, " Expecting %s, not: `%s' ", "`dup'", s);
      showline(logline, 1);

      break;
    }

    if ((s = grab_next_token(input, line)) == NULL)
      break;

    /* Cater to stupid Adobe Universal Greek font format */ /* 92/Sep/17 */
    if (strchr(s, '#') != NULL) /* stupid encoding vector format */
    {
      (void) sscanf(s, "%d#%n", &base, &n); /* try and get base */
      s += n; chr = 0;

      for (;;)
      {
        c = *s++;
        if (c >= '0' && c <= '9')
          c = c - '0';
        else if (c >= 'A' && c <= 'Z')
          c = c - 'A' + 10;
        else if (c >= 'a' && c <= 'z')
          c = c - 'a' + 10;
        else
        {
          s--; break;
        }

        chr = chr * base + c;
      }
    }
    else if (sscanf(s, "%d", &chr) < 1)
    {
      sprintf(logline, " Expecting %s, not: `%s' ", "number", s);
      showline(logline, 1);

      break;
    }
    
    /* deal with idiotic Fontographer format - no space before /charname */
    if ((t = strchr(s, '/')) != NULL)
      s = t;  /* 1992/Aug/21 */
    else if ((s = grab_next_token(input, line)) == NULL)
      break;

    if (*s != '/') 
    {
      sprintf(logline, "Bad char code `%s' ", s);
      showline(logline, 1);

      break;      // ???
    }
    else
      s++;

    if (chr >= 0 && chr < fontchrs)
    {
      if (strcmp(s, ".notdef") != 0)    /* ignore .notdef 97/Jan/7 */
        add_enc(chr, s);      /* 93/Nov/15 */
    }
    else
    {
      sprintf(logline, "Invalid char number %d ", chr);
      showline(logline, 1);
      break;      // ???
    }

    if ((s = grab_next_token(input, line)) == NULL)
      break;

    if (strcmp(s, "put") != 0)
    {
      sprintf(logline, " Expecting %s not: `%s' ", "`put'", s);
      showline(logline, 1);
    }

    if ((s = grab_next_token(input, line)) == NULL)
      break;
  }
/*  normally come here because line does not contain `dup'  */
/*  but does contain `readonly' or `def' */
/*  attempt to deal with Fontographer 4.0.4 misfeature 94/Nov/9 */
/*  if `readonly' appears on one line and `def' appears on the next */
  if (strcmp(s, "readonly") == 0)
  {
    if ((s = grab_next_token(input, line)) != NULL)
    {
      if (strcmp(s, "def") != 0)
      {
        sprintf(logline, " Expecting %s, not: `%s' ", "`def'", s);
        showline(logline, 1);
      }
    }
  }

  return 0;
}

/* check whether a TeX font - result not really used */
/* used only if resident font names are to be upper cased (-U) */
/* (so maybe the `lowercase' is not needed ?) */ /* or use _strnicmp(...) */
/* May be useful when PostScript FontName same as filename, but uppercase */

int is_tex_font (char *fname)
{
  if (_strnicmp (fname, "cm", 2) == 0 ||  /* Computer Modern (visible) */
    _strnicmp (fname, "lcm", 3) == 0 || /* SliTeX (visible) */
    _strnicmp (fname, "icm", 3) == 0 || /* Computer Modern invisible */
    _strnicmp (fname, "ilcm", 4) == 0 ||  /* SliTeX invisible */
    _strnicmp (fname, "msam", 4) == 0 ||  /* AMS math symbol */
    _strnicmp (fname, "msbm", 4) == 0 ||  /* AMS math symbol */
    _strnicmp (fname, "eu", 2) == 0 ||    /* Euler fonts */
    _strnicmp (fname, "wncy", 4) == 0 ||  /* Washington Cyrillic */ 
    _strnicmp (fname, "logo", 4) == 0 ||  /* logo fonts - METAFONT */
    _strnicmp (fname, "lasy", 4) == 0 ||  /* LaTeX symbol font */
    _strnicmp (fname, "line", 4) == 0 ||  /* LaTeX line fonts */
    _strnicmp (fname, "lcircle", 7) == 0 || /* LaTeX circle fonts */
    _strnicmp (fname, "mtmi", 4) == 0 ||  /* MTMI, MTMIB, MTMIH */
    _strnicmp (fname, "mtmu", 4) == 0 ||  /*     , MTMUB, MTMUH */
    _strnicmp (fname, "mtms", 4) == 0 ||  /* MTMS, MTMSB */
    _strnicmp (fname, "mtsy", 4) == 0 ||  /* MTSY, MTSYB, MTSYH, MTSYN */
    _strnicmp (fname, "mtex", 4) == 0 ||  /* MTEX, MTEXB, MTEXH */
    _strnicmp (fname, "mtgu", 4) == 0 ||  /* MTGU, MTMGUB */
    _strnicmp (fname, "rmtm", 4) == 0   /* RMTMI, RMTMIB, RMTMIH */
                        /*      , RMTMUB, RMTMUH */
    )
    return 1;
  else
    return 0;
}

/* separated out 97/Feb/6 */ /* returns 0 if failed */
/* this should also check for /BaseFontName and suppress replacement ? */
/* WARNING: this writes back into second argument ! */

int find_font_PFB_aux (FILE *input, char *FontName, int nlen)
{
  char token[] = "/FontName ";
  int c, k=0;
  char *s=FontName;

  while ((c = getc(input)) != EOF)
  {
    if (c == '%')       /* Try and avoid comment lines ... */
      while (c >= ' ') c = getc(input); /* Skip to end of line */

    if (c != token[k])
    {
      k = 0;          /* no match, reset ... */
      continue;
    }

    k++;

    if (k >= 10)
    {
      while ((c = getc(input)) != '/' && c != EOF)
      {
        if (c > ' ') {  /* only white space allowed here 97/June/1 */
          k = 0;
          continue; /* ignore /FontName *not* followed by /xxxxx */
        }
      }

      k = 0; 
      s = FontName;
/*      while ((c = getc(input)) > ' ' && k < MAXFONTNAME) */
      while ((c = getc(input)) > ' ' && k++ < nlen)
/*        buffer[k++] = (char) c; */
        *s++ = (char) c;
/*      fclose (input); */
/*      if (c != EOF && k < MAXFONTNAME) { */
      if (c != EOF && k < nlen) {
/*        buffer[k] = '\0'; */
        *s = '\0';
/*        strcpy (FontName, buffer); */
#ifdef DEBUG
        if (traceflag) {
          sprintf(logline, "Found /FontName %s in %s\n", FontName, FileName);
          showline(logline, 0);
        }
#endif
        return 1;   /* success */
      }
      else return 0;    /* failed, EOF in FontName or name too long */
    }
  }
  return 0;     /* failed - EOF read the whole file */
}

// FILE *open_font(char *font, int flag);

/* Attempt to find FontName from PFB file when forcereside != 0 */
/* An experiment 1993/Sep/30 */
/* WARNING: this writes back into second argument ! */

int find_font_PFB (char *FileName, char *FontName, int nlen) /* 1993/Sep/30 */
{
  int flag;
  FILE *input;

  if ((input = open_font(FileName, 0)) == NULL)
  {
#ifdef DEBUG
    if (traceflag) {
      sprintf(logline, "Unable to find font file for %s\n", FileName);  /* debug ? */
      showline(logline, 0);
    }
#endif
    return 0;
  }

  flag = find_font_PFB_aux(input, FontName, nlen);
  fclose (input);

  if (flag == 0)
  {
    sprintf(logline, "Unable to find FontName in %s\n", FileName);
    showline(logline, 1);
  }

  return flag;
}

/* finding PS FontName from PFM file */
/* WARNING: this writes back into second argument ! */

int find_font_PFM (char *FileName, char *FontName, int nlen) /* 1997/June/1 */
{
  FILE *input;
  int flag;

  if (traceflag)
  {
    sprintf(logline, "find_font_PFM nlen %d\n", nlen);
    showline(logline, 0);
  }

  if ((input = open_pfm(FileName)) == NULL)
  {
#ifdef DEBUG
    if (traceflag)
    {
      sprintf(logline, "Unable to find font file for %s\n", FileName);  /* debug ? */
      showline(logline, 0);
    }
#endif
    return 0;
  }

  flag = NamesFromPFM(input, NULL, 0, FontName, nlen, FileName);
  fclose (input);

  return flag;
}

/* Try and find font name in PFB / PFA file or in PFM file */
/* WARNING: this writes back into second argument ! */

int FindFontName (char *FileName, char *FontName, int nlen) /* 1993/Sep/30 */
{
  if (traceflag)
  {
    sprintf(logline, " FindFontName `%s' %d\n", FileName, nlen);
    showline(logline, 0); // debugging only
  }

  if (find_font_PFB (FileName, FontName, nlen) == 0)
  {
    if (find_font_PFM(FileName, FontName, nlen) == 0)
      return 0;
  }

  return 1;
}

/*  Split out 1992/Oct/8 - avoid assumption FontName occurs before Encoding */
/*  Now use even earlier - avoid assumption FontName comes after FontInfo */
/*  returns non-zero if font appears to be synthetic font */

/*  drops real PostScript FontName in realfontname if found */

int parse_line(char *line, FILE *output, int syntheticflag, int mtmiflag)
{
  double m11, m12, m21, m22, m31, m32;
  int ftp, k, n;
  char *s;

  /*  Try and pick out FontType */
  if ((s = strstr(line, "/FontType")) != NULL)
  {
    if(sscanf(s, "/FontType %d", &ftp) == 1)
    {
      if (ftp != 1)
      {
        sprintf(logline, " Not a Type 1 font: %s", line);
        showline(logline, 1);
        errcount(0);  
      }
    }
    PSputs(line, output);
  }
  else if ((s = strstr(line, "/FontName")) != NULL) /*  Try and pick out FontName */
  {
    s += 9; /* step over `/FontName' */

    while (*s != '/' && *s != '\0')
      s++;
    
    /* Verify that the FontName we got from %! line is correct 97/Jan/30 */
    if (bAddBaseName && *realfontname != '\0')
    {
      if (*s == '/')
      {
        n = strlen(realfontname);

        if (strncmp(s+1, realfontname, n) != 0)
        {
          sprintf(logline, " ERROR: %s != ", realfontname);
          showline(logline, 1);

          if (sscanf(s, "/%s", realfontname) > 0)
          {
            sprintf(logline, "%s ", realfontname);
          }
          else
          {
            sprintf(logline, "%s ", s);
          }

          showline(logline, 0);
        }
      }
    }

    if (sscanf(s, "/%s", realfontname) > 0)
    {
/* Don't trust `Oblique' (and `Narrow') fonts */ 
/* Helvetica and Courier may be the only common cases of such fonts ??? */
/* Helvetica, Helvetica-Light, Helvetica-Black, Helvetica-Narrow */
/* Maybe also `Slanted' and  `Narrow' and `Condensed' and `Expanded' ??? */
/* Inverted order of tests for efficiency -- 1993/Nov/4 */
      if (syntheticsafe != 0)
      {
        if (strstr(s, "Oblique") != NULL ||
          strstr(s, "BoldObl") != NULL ||
          strstr(s, "Narrow") != NULL)
        {
          for (k = 0; k < 16; k++)
          {
            if (strcmp("", syntheticfonts[k]) == 0)
              break;

            if (strstr(s, syntheticfonts[k]) != NULL)
            {
              syntheticflag = 1;

              if (verboseflag)
              {
                showline(" assumed synthetic", 0);
              }

              break;
            }
          }
        } 
      }
    }

    texfont = 0;

    if (is_tex_font(realfontname) != 0)
    {
      /* also check if actually one of 75 TeX fonts (what about LATEX, SliTeX) ? */
      texfont = 1;
      standard = 0;
    }
/* Should we replace /FontName or not ? *//* Presently hard wired to do this */
/* May need to be more careful here - assuming one line */
/*    if (bSubRealName == 0) { */     /* 1995/Aug/22 */
    if (bSubRealName == 0 || mmflag != 0)
    {
      if (fontprefix != NULL)
      {
        *s = '\0';          /* s points at /<fname> */
        PSputs(line, output);
        *s = '/';
        PSputc('/', output);
        PSputs(fontprefix, output);
        PSputs(s+1, output);
      }
      else
      {
        PSputs(line, output); /* no prefix, just copy over */
      }
    }
/* Don't mess with FontName of old MTMI */
    else if (mtmiflag != 0)
    {
      PSputs(line, output);
    }
/* Don't mess with FontName of MM base font ? old method */ /* 1994/Dec/6 */
/* Don't touch: 2 copy exch /FontName exch put */   /* 95/May/14 */
    else if (strstr(line, "/FontName exch") != NULL)
    {
      PSputs(line, output);
    }
    else
    {
      PSputs("/FontName /", output);
/* don't need to check here whether its resident or not ! */
      if (fontprefix != NULL)
      {
        PSputs(fontprefix, output);
      }
/* following inserted 1992/Dec/22 - for new `U' flag */
      if (uppercaseflag != 0 && texfont != 0)
        uppercase(filefontname, filefontname);

      PSputs(filefontname, output);
      PSputs(" def\n", output);
    }
  }
/*  Try and pick out FontMatrix */
/*  Following usually doesn't get triggered because FontName seen first */
/*  Can't be more selective here, since don't know FontName ... */
  else if ((s = strstr(line, "/FontMatrix")) != NULL)
  {
    if (sscanf(s, "/FontMatrix[%lg %lg %lg %lg %lg %lg]",
      &m11, &m12, &m21, &m22, &m31, &m32) == 6)
    {
      if (syntheticsafe != 0 && syntheticflag == 0)
      {
        if (m11 != m22 || m21 != 0.0 || m12 != 0.0)
        {
          showline(" WARNING: use *synthetic* in sub file", 1);
          syntheticflag = 1;
        }
      }
    }

    PSputs(line, output);
  }
  else
  {
    PSputs(line, output);
  }

  return syntheticflag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long checkdefault = 0x59265920;  /* default signature */

/* Takes first six letters of encoding vector name and compresses */
/* into four bytes using base 40 coding */ /* 40^6 < s^32 */
/* Treats lower case and upper case the same, ignores non alphanumerics */
/* Except handles -, &, and _ */

unsigned long codefourty(char *codingvector)
{
  unsigned long result=0;
  int c, k;
  char *s=codingvector;

  if (strcmp(codingvector, "") == 0)
  {
    codingvector = "native";    /* if not specified ... */
    return checkdefault;      /* use default signature */
  }

  for (k = 0; k < 6; k++)
  {
    for ( ; ; )
    {
      c = *s++;

      if (c >= 'A' && c <= 'Z')
        c = c - 'A';
      else if (c >= 'a' && c <= 'z')
        c = c - 'a';
      else if (c >= '0' && c <= '9')
        c = (c - '0') + ('Z' - 'A') + 1;
      else if (c == '-')
        c = 36;
      else if (c == '&')
        c = 37;
      else if (c == '_')
        c = 38;
      else
        c = 39;          /* none of the above */

      result = result * 40 + c;

      break;
    }
  }

  return result;
}

int decodefourty(unsigned long checksum, char *codingvector)
{
  int c;
  int k;

  if (checksum == 0)
  {
    strcpy(codingvector, "unknown");
    return 1;
  }
  else if ((checksum >> 8) == (checkdefault >> 8))
  {
    strcpy (codingvector,  "native");   /* if not specified ... */
    return 1;               /* no info available */
  }
  else
  {
    for (k = 0; k < 6; k++)
    {
      c = (int) (checksum % 40);
      checksum = checksum / 40;

      if (c <= 'z' - 'a' )
        c = c + 'a';
      else if (c < 36)
        c = (c + '0') - ('z' - 'a') - 1;
      else if (c == 36)
        c = '-';
      else if (c == 37)
        c = '&';
      else if (c == 38)
        c = '_';
      else
        c = '.';       /* unknown */

      codingvector[5 - k] = (char) c;
    }

    codingvector[6] = '\0';
  }

  return 0;               /* encoding info returned */
}

int check_enc(int k)
{
  char checksumvector[8];

  if (fc[k] == nCheckSum)
    return 0;

  if (decodefourty(fc[k], checksumvector) != 0)
    return 0;

  if (_strnicmp(checksumvector, textenconame, 6) == 0)
    return 0;

  showline(" WARNING: encoding mismatch ", 1);
  sprintf(logline,  "TFM: `%s..' versus PFB: `%s'", 
    checksumvector, textenconame);
  showline(logline, 1);

  return 1;
}

int check_remap_enc(int k, char *fontnamek)
{
  char checksumvector[8];

  if (decodefourty(fc[k], checksumvector) != 0)
    return 0;

  if (fontvector[k] != NULL &&
      _strnicmp(checksumvector, fontvector[k], 6) == 0) 
    return 0;

  showline(" WARNING: encoding mismatch ", 1);
  sprintf(logline, " in %s ", fontnamek);
  showline(logline, 1);
  sprintf(logline,  "TFM: `%s..' versus vector: `%s'\n", 
      checksumvector, fontvector[k] != NULL ? fontvector[k] : "");
  showline(logline, 1);

  return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* share some code between extract_type1 and extract_type3 ??? */
/* e.g. for BeginResource and EndResource ? OK, that has been done */
/* actually, appears to be some divergence now ... */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* decompress a Type 1 font file and send it to specified output */

int extract_type1 (FILE *output, FILE *input, int fn, char *fontnamek)
{
  int nvec, ns, ne, c, d, k, nbin, n, nchrs, flag;
  int count, property, syntheticflag, mtmiflag;
  int stripflag = 0;
  int l;      /* 1994/Jan/30 for `unseen' repeat appearance */
  int guess;    /* 1995/Oct/25 */
  char *s, *t, *vector;
  char charname[FNAMELEN];      /* just to be safe */
  char subfontnamek[FNAMELEN];
  char *wantchrs;

  hybridflag=0;
  binaryin = 0;
  wantchrs = fontchar[fn];

  if (wantchrs == NULL)
    showline(" BAD wantchrs", 1);

  property = fontproper[fn];
  vector = fontvector[fn];
  syntheticflag = (property & C_SYNTHETIC);
  mtmiflag = (property & C_MTMI);
  mmflag = (property & C_MULTIPLE);

  if (subfontname[fn] != NULL)
    strcpy(subfontnamek, subfontname[fn]);
  else
    *subfontnamek = '\0';

  if (bSuppressPartial != 0)
    syntheticflag = 1;

  if (mtmiflag != 0)
    showline(" NOT Type 1", 1);

  count = 0;        /* preliminary - may update later with accents */

  for (k = 0; k < MAXCHRS; k++)
    if (wantchrs[k] != 0)
      count++;

  if (count == 0)
  {
    if (traceflag)
      showline(" No characters used in font", 1);

    fontproper[fn] |= C_UNUSED;

    return -1;    /* shouldn't ever happen ? unless page sub range ... */
  }
  else if (verboseflag)
  {
    showline(fontnamek, 0);
  }
  else
  {
    showline("*", 0);
  }


  /*  Now actually start looking at the file! */
  if (bAddBaseName)  /* try and get /FontName 97/Feb/6 */
  {
    bBaseNameDone = 1;
    *realfontname = '\0';

    if (find_font_PFB_aux (input, realfontname, sizeof(realfontname)))
      bBaseNameDone = 0;  /* found it, so use it */

    rewind(input);
  }

  c = getc(input);
  (void) ungetc(c, input);

  if (c == 0)  /* check if this is a Mac style font file */
  {
/*    macstyle = 1; */ /* try and position at start of ASCII length field */
/*    need to figure out where we are in Mac file */
/*    could be (a) at start of MacBinary header */
/*         (b) at start of resource fork (128 byte later) */
/*         (c) at start of actual resource (256 byter after that) */
    binaryin = 1;
    (void) getc(input);         /* first byte (0) again */

    if ((c = getc(input)) != 0) /* start of MacBinary ? */
    {
      for (k = 2; k < 384; k++) (void)
        getc(input); /* yes */

      c = getc(input);
      (void) ungetc(c, input);

      if (c != 0)    /* try and verify format */
      {
        showline(" ERROR: Not a valid PC (or Mac) style font file ", 1);
        errcount(0);

        return 0;   /* errcount ? */
      }
    }
    else
    {
      /* now try and check whether at start of resource fork, */    
      /* which starts with (0, 0, 1, 0) = 256 length */
      c = getc(input);
      d = getc(input);

      if (c == 1 && d == 0)
      {
        for (k = 4; k < 256; k++)
          (void) getc(input);

        c = getc(input);
        (void) ungetc(c, input);

        if (c != 0) /* try and verify format */
        {
          showline(" ERROR: Not a valid PC (or Mac) style font file ",1 );
          errcount(0);

          return 0;   /* errcount ? */
        }
      }
      else
      {
        /* try and imagine we are right there actually */
        (void) getc(input);
        (void) getc(input);  /* skip length */
      }       
    }
  } /* end of first byte is zero in file case */

  chrs = -1;
  task = "copying heading line";
  k = extgetline(input, line);    /* get first line of font */

  if (*line != '%' || *(line + 1) != '!')
  {
    showline(" Nonstandard font file start: ", 1);
    showline(line, 1);
    errcount(0);
  }

  if (strstr(line, "Type3") != NULL || strstr(line, "Font-3") != NULL)
  {
    type3flag = 1;    /* well, maybe lets try Type 3 then ! */
    rewind(input);

    return 0;     /* indicate lack of success ! */
  }

  if (stripcomment == 0)
  {
    if (mmflag)
      begin_resource(output, subfontnamek);
    else
      begin_resource(output, filefontname);

    PSputs(line, output);

    if (wantcreation != 0)
    {
      k = extgetline(input, line);  /* check second line of font */

      if (strncmp(line, "%%CreationDate", 14) == 0)
        PSputs(line, output);
    }
  }
  
  task = "looking for FontInfo";

  while (strstr(line, "/FontInfo") == NULL && k >= 0)
  {
/*  Try and strip out that `font wrapper' stuff checking for existing font ! */
/*  ===> NOTE: this is in outer level <=== */
/*  Used in Courier, Helvetica for example */
/*  Also junk in Fontographer style fonts - up to three lines long */
    if (stripchecking != 0)
    {
      if (strstr(line, "FontDirectory") != NULL)
      {
        stripflag++;
        k = extgetline(input, line);

        if (strstr(line, "/FontType") != NULL)
        {
          k = extgetline(input, line);
        }

        if (strstr(line, "{save true}") != NULL)
        {
          k = extgetline(input, line);
        }

        while (strstr(line, "dict") == NULL)
        {
          k = extgetline(input, line); 

          if (k == EOF)
            break;
        }

        if (stripcomment == 0)
        {
          PSputs("% font wrapper removed\n", output);
        }
      }
    }
    
    /*  ignore comment lines OTHER than copyright or trademark line */
    if (*line == '%')
    {
      if (strstr(line, "opyright") != NULL ||
        strstr(line, "rademark") != NULL ||
        strstr(line, "(c)") != NULL)
        PSputs(line, output);
    }
    else if (*line != '\n')
    {
      syntheticflag = parse_line(line, output, syntheticflag, mtmiflag);
    }

    k = extgetline(input, line);
  }

  if (bAddBaseName && !bBaseNameDone)
  {
    flag = 0;

    if ((s = strstr(line, "/FontInfo")) != NULL)
    {
      s +=10;

      if (sscanf(s, "%d%n dict", &k, &n) == 1)
      {
        char buffer[FNAMELEN];  /* long enough ? */
        strcpy(buffer, s+n);
        sprintf(s, "%d", k+1);
        strcat(s, buffer);
        flag++;
      }
    }

    if (flag == 0)
    {
      showline(" ERROR: unable to extend FontInfo dictionary\n", 1);
      bBaseNameDone++;
    }
  }

  task = "stripping out FontInfo"; 
/*  Now strip info ONLY if busedviencode is set --- 1992/July/18 */
/*  It would be dangerous to retain /Notice in /FontInfo directory only, */
/*    since /Notice may be multiline  --- changes 1992/July/18 */
/*  This has a somewhat flakey termination test ... */
/*  usually the end is `end readonly def' */
    
  while (strstr(line, "end ") == NULL && k >= 0)
  {
    if (stripinfo == 0 || busedviencode == 0)
    {
      PSputs(line, output);
    }
    else if ((s = strstr(line, "/Notice")) != NULL)
    {
      sprintf(logline, "%% %s", s); /* s+1 ? */
      PSputs(logline, output);
    }

    if (strstr(line, "/BaseFontName ") != NULL)
      bBaseNameDone++;

    k = extgetrealline(input, line);
  }

  /*  If haven't placed BaseFontName yet, do it here */ /* 97/Jan/30 */
  if (bAddBaseName && !bBaseNameDone)
  {
    sprintf(logline, "/BaseFontName (%s) def\n", realfontname);
    PSputs(logline, output);
    bBaseNameDone++;
  } 

  if (stripinfo == 0 || busedviencode == 0)
  {
    PSputs(line, output);
  }

  task = "looking for Encoding & FontName";
  k = extgetrealline(input, line);

  while (strstr(line, "/Encoding") == NULL && k >= 0)
  {
    syntheticflag = parse_line(line, output, syntheticflag, mtmiflag);
    k = extgetrealline(input, line);
  }
  
  /*  special case hack when "def" is on line *after* StandardEncoding 98/Oct/8 */
  if (strstr(line, "StandardEncoding") != NULL)
  {
    if (strstr(line, "def") == NULL)
    {
      s = line + strlen(line);
      *(s - 1) = ' ';     /* turn line termination into space */
      k = extgetrealline(input, s);
    }
  }

  if (mtmiflag != 0)
  {
    PSputs(line, output);
  }
  else if ((property & C_REMAPIT) == 0)
  {
    if (strstr(line, "StandardEncoding") != NULL)
    {
      standard = 1;
      texfont = 0;
      fontchrs = MAXCHRS;

      if (bWindowsFlag != 0)
      {
        if (verboseflag)
        {
          showline("~", 0);
        }

        copy_enc(charnames, ansi_enc, fontchrs);
        missing_chars(wantchrs, textenconame);
        write_enc(output, wantchrs, syntheticflag, textenconame);

        if (bCheckEncoding)
          (void) check_enc(fn);

        nansified++;
      }
      else
      {
        copy_enc(charnames, std_enc, fontchrs);
        missing_chars(wantchrs, "standard");
        write_enc(output, wantchrs, syntheticflag, "standard");
      }
    }
    else
    {
      standard = 0;

      if(sscanf(line, "/Encoding %d array", &nvec) < 1)
      {
        sprintf(logline, " Don't understand encoding vector: %s", 
          line);
        showline(logline, 1);
        extgiveup(9);

        return -1;
      }

      k = extgetrealline(input, line);
/*      have to ignore "0 1 255 {1 index exch /.notdef put} for" line */
      if (sscanf(line, "%d 1 %d {", &ns, &ne) < 2)
      {
        showline(" No /.notdef line", 1); 
      }
      else
        k = extgetrealline(input, line);

/*      grabbed next line (or hung onto this, if NOT  /.notdef line) */
/*      clear out charnames - background of blanks */
      clean_enc(0, MAXCHRS);

      task = "reading Encoding"; 
      gobble_enc(input);
      missing_chars(wantchrs, "");
      write_enc(output, wantchrs, syntheticflag, "");
    }
  }
  else
  {
    /* now for what to do if non-resident font remapped C_REMAPIT */
    if (verboseflag)
    {
      showline("^", 0);
    }

    /*    first flush old encoding vector */
    if (strstr(line, "StandardEncoding") == NULL)
    {
/*    scan encoding - ends on "readonly def" usually */
/*    has to ignore "0 1 255 {1 index exch /.notdef put} for" line */
/*    skip over encoding in font - ignore it totally */
/*    Encoding ends with token `def' or `readonly' */ 
/*    and token `def' and `readonly should not occur in Encoding */
      while ((strstr(line, "def") == NULL ||
        strstr(line, "put") != NULL) && k >= 0)
      {
        k = extgetrealline(input,line);
      }
    }

    read_enc(vector);
    missing_chars(wantchrs, vector);
    write_enc(output, wantchrs, syntheticflag, "");
  }

  task = "copying font dict up to eexec";
  k = extgetrealline(input, line);

  while (strstr(line, "eexec") == NULL && k >= 0)
  {
    syntheticflag = parse_line(line, output, syntheticflag, mtmiflag);
    k = extgetrealline(input, line);
  }

  PSputs(line, output);

  if (bSubRealName == 0)
  {
    if (subfontname[fn] != NULL)
      free(subfontname[fn]);

    subfontname[fn] = zstrdup(realfontname);
  }

  if (k == EOF)
  {
    showline(" Premature EOF", 1);
    showline(" in font file", 1);
    errcount(0);

    return -1;    /* NEW --- shouldn't happen ! */
  }

  if (syntheticflag != 0 || mtmiflag != 0)
  {
    copy_font(output, input, fn, mtmiflag, stripflag);

    return 1;
  }

  cryptin = REEXEC;
  cryptout = REEXEC;
  clm = 0;        // important for out_encrypt

  task = "entering encrypted section";

  c = getc(input);

  while (c <= ' ' && c > 0)
    c = getc(input);  /* skip over white space */

  if (c == 128) /* see whether .pfb input format */
  {
    binaryin = 1;
    c = getc(input);

    if (c != 2)
    {
      sprintf(logline, 
        " Expecting %s, not %d", "binary section code", c);
      showline(logline, 1);
      extgiveup(5);

      return -1;
    }

    len = readlength(input);
  }
  else if (c == 0) /* see whether Mac binary input format */
  {
    binaryin = 1;
    (void) ungetc(c, input);
    len = maclength(input);
    c = getc(input);

    if (c != 2)
    {
      sprintf(logline, " Expecting %s, not %d", "Mac binary section code", c);
      showline(logline, 1);
      extgiveup(5);
      return -1;
    }

    c = getc(input);

    if (c != 0)
    {
      sprintf(logline, " Invalid Mac Binary section code %d", c);
      showline(logline, 1);
      extgiveup(5);

      return -1;
    }

    len = len -2;
  }
  else if (c > 128) /* see whether raw binary - totally flakey ! */
  {
    binaryin = 1;
    len = (1U << 31); /* conversion to unsigned long ... */
    (void) ungetc(c, input);
  }
  else
    (void) ungetc(c, input);

/* copying across the four random encoding bytes at start also */

  n = get_magic(input, line);
  put_en_linen(output, line, n);
  
  task = "looking for CharString dict";
  n = get_en_line(input, line);

  while ((s = strstr(line, "/CharStrings")) == NULL)
  {
    /*  Omit UniqueID if font remapped AND dviencoding in use  */
    if ((property & C_REMAPIT) != 0)
    {
      if (busedviencode != 0 && strstr(line, "/UniqueID") != NULL)
      {
        n = get_en_line(input, line);
        continue; 
      }
    }

    put_en_linen(output, line, n);

    if (strstr(line, "/Subrs") != NULL)
    {
      copy_subrs(output, input);

      if (abortflag)
        return -1;
    }

    n = get_en_line(input, line);
/* check on possibility of Synthetic Font */
/* possibly suppress discarding of unwanted characters ? */
/* Synthetic fonts won't work when remapped ? */
/* ===> NOTE: this is in encrypted level <=== */
/* possibly have to deal differently with the ending ? */
/* need to transfer the rest of the font just as is because of byte count */
    if (strstr(line, "FontDirectory") != NULL)
    {
/* don't complain if already noted that it was synthetic ... */
      if (syntheticsafe != 0 && syntheticflag == 0)
      {
        showline(" ERROR: use *synthetic* in sub file", 1);
        errcount(0);
        syntheticflag = -1;   /* is it safe to do this now ? */
      }
    }

    if (strstr(line, "hires") != NULL)
    {
      if (hybridflag == 0)
        showline(" hybrid", 0);  /* first time */

      hybridflag++;
    }
  }

  if (strncmp(line, "2 index", 7) == 0)
    fontform = 2;
  else if (strncmp(line, "dup", 3) == 0)
    fontform = 1;
  else
    fontform = 0;

  if (sscanf(s, "/CharStrings %d%n", &nchrs, &n) < 1)
  {
    put_en_line(output, line);
    n = get_en_line(input, line);
    s = line;

    if (strstr(line, "dict dup") == NULL ||
      sscanf(s, "%d%n", &nchrs, &n) < 1)
    {
      sprintf(logline, " Don't understand CharStrings line: %s", line);
      showline(logline, 1);
      extgiveup(2);

      return -1;
    }
  }
  else  /* normal case: /CharString <n> dict dup on line */
  {
    *(s + 13) = '\0';       /* terminate after `/CharStrings' */
    put_en_line(output, line);  /* start of modified /CharString line */
  }

charagain:

  if (accentedflag != 0)
  {
    count = 0;

    for (k = 0; k < MAXCHRS; k++)
      if (wantchrs[k] != 0)
        count++;
  }

  if (wantnotdef != 0)
    count++;

  if (syntheticflag != 0)
    count = nchrs;

  nchrs = count;

  sprintf(line, "%d", nchrs);
  put_en_line(output, line);  /* middle of modified /CharString line */
  *(s + n) = ' ';       /* fix up, in case it became `\0' */
  put_en_line(output, s + n); /* end of modified /CharString line */

  task = "scanning CharStrings";
  chrs = 0;

  for (k = 0; k < MAXCHRS + 1; k++)
    charseen[k] = 0;

  for ( ; ; )
  {
    if (get_char_line(line, input, 0) != 0)
      break;

    /* the token "end" indicates the end of the CharString section */
    if (sscanf(line, "/%s %d %n", charname, &nbin, &n) < 2)
    {
      sprintf(logline, " Not a CharString line: %s", line);
      showline(logline, 1);

      if (get_char_line(line, input, 0) != 0)
        break;

      errcount(0);
    }

    guess = -1;

    /*    single character charnames equal their char code 95/Oct/28 */
    if (*(charname + 1) == '\0')
      guess = *charname;

    k = want_this_name(charname, guess, wantchrs);

    if (k >= 0 || syntheticflag != 0)
    {
      if (syntheticflag != 0)
        k = NOTDEF;
      else
        charseen[k]++;

      /*  for dviencoding, change name to numeric code, unless its .notdef */
      if (busedviencode != 0 && k != NOTDEF) /* change charname */
      {
        if (strstr(line + n, "RD") != NULL)
          sprintf(line, "/a%d %d RD ", k, nbin);
        else
          sprintf(line, "/a%d %d -| ", k, nbin);
      }

      put_en_line(output, line);
      copy_char_string(output, input, nbin);
      /* this may not be accurate if repeated encoding and both char codes used .. */
      count--;
    }
    else
      flush_char_string(input, nbin);

    chrs++;

    if (bAbort)
      abortjob();

    if (abortflag)
      return -1;
  }
  
/*  believe that count should be zero here ! */
  if (count > 0)
  {
/*  first check whether `missing' characters appear twice in encoding */    
/*  this is important now that we use TEXANSI and extend encoding at bottom */
/*  can only be equal to character lower in code, so don't start at zero */
    for (k = 1; k < MAXCHRS; k++) /* check the suspects */
    {
      if ((wantchrs[k] != 0) && (charseen[k] == 0))
      {
/*  don't play with characters that have no names ! 96/May/26 */
        if (strcmp(charnames[k], "") == 0)
          continue;

        for (l = 0; l < k; l++)
        {
          if (strcmp(charnames[l], "") == 0)
            continue;

          if (strcmp(charnames[k], charnames[l]) == 0)
          {
            charseen[k]++;
            count--;

            break;
          }
        }
      }
    }
  }

  if (count > 0)
  {
    sprintf(logline, " %d character%s (out of %d) not found: ", 
      count, (count == 1) ? "" : "s", nchrs);
    showline(logline, 1);

    for (k = 0; k < MAXCHRS; k++)
    {
      if ((wantchrs[k] != 0) && (charseen[k] == 0))
      {
/*  characters may have no names if not in encoding 96/May/26 */
        if (strcmp(charnames[k], "") != 0)
          sprintf(logline, " %s (%d)", charnames[k], k); 
        else
          sprintf(logline, " (%d)", k);

        showline(logline, 1);
      }
    }

    errcount(0); /* ??? */
  }

  chrs = -1;
  task = "copying end of CharString dict def";
  put_en_line(output, line);      /* copy beginning of end line */
  n = get_en_line(input, line);     /* copy rest of line */
  put_en_line(output, line);

/*  1993 August 5 - deal with `hybrid' font - second set of CharStrings */
  if (strstr(line, "hires") != NULL)
  {
    n = get_en_line(input, line);   /* get potential <n> dict dup line */
    s = line;

    if (strstr(line, "dict dup") != NULL &&
      sscanf(s, "%d%n", &nchrs, &n) == 1)
      goto charagain;

    if ((s = strstr(line, "dict dup")) != NULL)
    {
      while (s > line && *(s-1) == ' ')
        s--;

      while (s > line && *(s-1) > ' ')
        s--; /* Try and step back */

      if (sscanf(s, "%d%n", &nchrs, &n) == 1)
      {
        *s = '\0';        /* terminate after `/CharStrings' */
        put_en_line(output, line); /* start modified /CharString line */

        goto charagain;
      }
    }
  }
  else
    n = get_en_line(input, line);      /* copy /FontName line */
  
/* if by mistake we wade into second /CharStrings of hybrid font, following */
/* goes wrong because it copies binary stuff and treats 0 char as end string */

  task = "copying end of font dict def";

  while (strstr(line, "closefile") == NULL)
  {
    put_en_line(output, line);
    n = get_en_line(input, line);
  }

  put_en_line(output, line);
  flush_encrypt(output);           // ???

/*  A problem here if the font has junk at the end of binary section ... */
/*  Will this `fix' screw up treatment of Mac fonts ? */

/*  96/Feb/22 deal with junk at end of encrypted section */
/*  Looking for M-@C-A (ASCII section heading) for PFB */
/*  Looking for EOL for PFA */
/*  Looking for C-@C-B for Mac */
  if (bStripBadEnd)
  {
    long current = ftell(input);
    c = getc(input);

    while (c != 128 && c != '0' && c != 0 && c >= ' ')
    {
      sprintf(logline, " JUNK %d (%c) at byte %ld", c, c, current);
      showline(logline, 1);
      current = ftell(input);
      c = getc(input);
    }

    ungetc(c, input); /* ??? */
  }

  task = "copying ASCII section at end";
  c = getc(input);

  if (c != '\n')
    (void) ungetc(c, input);

  PSputc('\n', output);             /* for PFB not for PFA ? */

  while (extgetline(input, line) != EOF)
  {
/* new way to finish off */
/*    fputs(line, output);  */
/*    Try and strip out that old Adobe crap checking for existing font ! */
/*    Also junk in Fontographer style fonts */  /* 1992/Aug/21 */
/*    Used in Courier, Helvetica for example */
    if (stripchecking != 0)
    {
      if ((s = strstr(line, "{restore}")) != NULL)
      {
        stripflag--;

        if ((t = strstr(line, "cleartomark")) != NULL)
        {
          *s++ = '\n';
          *s++ = '\0';      /* cut off after cleartomark */
          PSputs(line, output);
        }

        if (stripcomment == 0)
        {
          PSputs("% font wrapper removed\n", output);
        }

        continue; 
      }
    }

    PSputs(line, output);
  }

  end_resource(output);      /* share some code */

  if (stripflag != 0) 
    showline(" WARNING: broken wrapper ", 1);

  return 1;             /* indicate success */
}
  
/* This is very specific for output from PKTOPS */
/* Does not provide for encoding vector */
/* Does not provide for remapping */
/* First cues on `cleartomark' after encrypted stuff */
/*    (OR: cues on second line that starts with "}def" if unencrypted */
/* Then cues on `definefont' at end of font */

/* returns 0 if failed right away */

int extract_type3 (FILE *output, FILE *input, int i, char *fontnamek)
{
  int nchar, copyflag, endflag, count;
  char *s;
  char *wantchrs;

  if (verboseflag)
  {
    showline(fontnamek, 0);
  }
  else
  {
    showline("*", 0);
  }

  wantchrs = fontchar[i];

  if (wantchrs == NULL)
    showline(" BAD wantchrs", 1);
  
  (void) getline(input, line);

  if (*line != '%' || *(line + 1) != '!')
  {
    showline(" Nonstandard font file start: ", 1);
    showline(line, 1);
    errcount(0);
  }

  if (strstr(line, "Type1") != NULL || strstr(line, "Font-1") != NULL)
  {
    type3flag = 0;    /* well, maybe let try Type 1 then ! */
    rewind(input);

    return 0;     /* indicate lack of success */
  }

  if (verboseflag)
    showline(" BITMAP", 0); 

  if (stripcomment == 0)
  {
    begin_resource(output, filefontname);
    PSputs(line, output);

    if (wantcreation != 0)
    {
      (void) getline(input, line);

      if (*line != '%' || strncmp(line, "%%CreationDate", 14) == 0)
        PSputs(line, output);
    }
  }

  count = 0;    /* number of lines starting with "}def" seen so far */

  for ( ; ; )
  {
    if (getrealline(input, line) == 0)
    {
      sprintf(logline, " Unexpected EOF (%s)\n", "extract_type3");
      showline(logline, 1);
      errcount(0);

      break;      /* was: return -1;  */
    }

    if (strstr(line, "serverdict") == NULL) /* omit serverdict line ! */
    {
      PSputs(line, output);
    }

    if (strstr(line, "cleartomark") != NULL)
      break; /* end preamble ? */

    if (strncmp(line, "}def", 4) == 0)
    {
      if (count++ >= 1)
        break;
    }
  }

/*  now copy across character bitmaps */
  for ( ; ; )
  {
    endflag = 0;      /* set when `definefont' seen ... */

    for ( ; ; )
    {
      if (getrealline(input, line) == 0) /* EOF */
      {
        endflag = 1;
        break;
      }

      if (*line == '/')
      {
        if ((s = strstr(line, "/FontName")) != NULL)
        {
          s += 9;           /* step over `/FontName' */

          while (*s != '/' && *s != '\0') s++;

          if (sscanf(s, "/%s", realfontname) > 0)
          {
            lowercase(realfontname, realfontname);  /* why ? */
          }
          
          /* check whether TeX font */
          texfont = 0;

          if (is_tex_font(realfontname) != 0)
          {
/* also check if actually one of 75 TeX fonts (what about LATEX, SliTeX) ? */
            texfont = 1;
            standard = 0;
          }

          if (bSubRealName != 0)
          {
            strcpy(line, "/FontName /");

            if (fontprefix != NULL)
              strcat(line, fontprefix);

            if (uppercaseflag != 0 && texfont != 0)
              uppercase(filefontname, filefontname);

            strcat(line, filefontname);
            strcat(line, " def\n");     /* down to here */
          }
        }
        else
          break;
      }

      PSputs(line, output);

      if (strstr(line, "definefont") != NULL)
      {
        endflag = 1;
        break;
      }
    }

    if (endflag != 0)
    {
      break;
    }
    
    /* presently assuming simple numeric code - no encoding vector */
    if (sscanf(line, "/a%d ", &nchar) == 1)
    {
      copyflag = wantchrs[nchar]; 

      if (copyflag != 0)
      {
        PSputs(line, output);
      }

      for ( ; ; )
      {
        if (getrealline(input, line) == 0)
        {
          sprintf(logline, " Unexpected EOF (%s)\n", "extract_type3");
          showline(logline, 1);

          break;      /* was:   return -1; */
        }

        if (copyflag != 0)
        {
          PSputs(line, output);
        }

        if (*line == '>')
          break;    /* end of this CharDef */
      }
    }
    else
    {
      PSputs(line, output);
    }

    if (bAbort)
      abortjob();
  }

  end_resource(output);

  return 1;               /* indicate success */
}
  
void complain_bad_font (int flag)
{
  sprintf(logline, " Bad Font File %d", flag);
  showline(logline, 1);
  errcount(0);
  checkexit(1);
}

void new_copy_ascii (FILE *output, FILE *input, unsigned long len)
{
  size_t n, m;  
  char buffer[MAXLINE+1];
  char *s;

  m = sizeof(buffer - 1) / sizeof(buffer[0]);   // leave space for null

  if (len < m)
    m = (size_t) len;

  while ((n = fread(buffer, 1, m, input)) != 0)
  {
    s = buffer;

    while ((s = strchr(s, '\r')) != NULL)
      *s = '\n';

    buffer[n] = '\0';     // terminate for PSputs
    PSputs(buffer, output);   // 99/Aug/13
    len -= n;

    if (len <= 0)
      break;

    if (len < m)
      m = (size_t) len;    // last bit
  }
}

#define COLUMNS 78

void new_copy_binary (FILE *output, FILE *input, unsigned long len)
{
  size_t n, m;
  unsigned char inbuffer[COLUMNS / 2];
  char outbuffer[COLUMNS + 2];      // space for \n and null
  int c, d, kk;
  unsigned int k;

  m = sizeof(inbuffer);

  if (len < m)
    m = (size_t) len;

  while ((n = fread(inbuffer, 1, m, input)) != 0)
  {
    kk = 0;

    for (k = 0; k < n; k++)
    {
      c = inbuffer[k];
      d = c & 15;
      c = c >> 4;

      if (c > 9)
        c = c + 'A' - 10;
      else
        c = c + '0';

      if (d > 9)
        d = d + 'A' - 10;
      else
        d = d + '0';

      outbuffer[kk++] = (char) c;
      outbuffer[kk++] = (char) d;
    }

    outbuffer[kk++] = '\n';
    outbuffer[kk++] = '\0';     // terminate for PSPuts
    PSputs(outbuffer, output);
    len -= n;

    if (len <= 0)
      break;

    if (len < m)
      m = (size_t) len;
  }
}

/* Needed in dvispeci.c to deal with %%IncludeResource: font & %%IncludeFont */
/* Do the dumbest possible thing - treat only plain PFA & PFB formats */
/* called from dvispeci.c */

int decompress_font (FILE *output, FILE *input, char *FontName)
{
  int c, d;
  int eof = 0;
  long len;

  begin_resource(output, FontName);
  c = getc(input);
  ungetc(c, input);

  if (c != 128) /* we have to assume it is PFA format */
  {
    new_copy_ascii(output, input, 0xFFFFFFFF);
  }
  else
  {
    for ( ; ; )
    {
      c = getc(input);
      d = getc(input);

      if (c != 128)
      {
        complain_bad_font(c);
        return -1;
      }

      switch (d)
      {
        case 1: /* ASCII */
          len = readlength(input);
          new_copy_ascii(output, input, len);
          break;

        case 2: /* Binary */
          len = readlength(input);
          new_copy_binary(output, input, len);
          break;

        case 3: /* EOF */
          eof = 1;
          break;

        default:
          complain_bad_font(d);
          return -1;
      }

      if (eof)
        break;
    }
  }

  end_resource(output);

  return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* check whether we need any font substitution */
int need_substitute (void)
{
  int k, proper;

  for (k = 0; k < fnext; k++)
  {
    if (fontsubflag[k] >= 0)
    {
      proper = fontproper[k];

      if ((proper & C_RESIDENT) != 0)
        continue;

      if ((proper & C_REMAPIT) != 0)
        continue;

      if ((proper & C_DEPENDENT) != 0)
        continue;

      return -1;
    }
  }

  return 0;
}

/* check to see if some resident fonts are remapped */
/* only if remapped font is also printer resident */
int need_remap(void)
{
  int k;
  int winflag = 0;
  int proper;

/*  return positive if bWindowsFlag != 0 and some residents 93/Oct/4 */
/*  returns negative if at least one font needs some other remapping */

  for (k=0; k < fnext; k++)
  {
    proper = fontproper[k];

    if ((proper& C_RESIDENT) != 0)
    {
      if ((proper & C_REMAPIT) != 0)
        return -1;

      if (bWindowsFlag != 0)
        winflag++;
    }
  }

  return winflag;
}

/* returns total number of substitutes so far */
/* read font substitution */
int read_substitutes (FILE *input)
{
  char oldname[FNAMELEN], newname[FNAMELEN], vector[FNAMELEN];
  int n, nlen, nnames, property;
  int k = ksubst;
  int count = ksubst;
  char *s, *u, *sd;

  if (traceflag)
  {
    sprintf(logline, "Reading `%s'\n", fontsubfile);
    showline(logline, 0);
  }

  while (getrealline(input, line) != 0)
  {
    if (*line == '%' || *line == ';')
      continue;

    property = 0;       /* reset resident/forced/remap flag */
    s = line;

    while (*s == ' ' || *s == '\t')
      s++;

    nnames = sscanf(s, "%s %s%n", oldname, newname, &n);

    if (avoidreside != 0 && nnames == 2)
    {
      strcpy(newname, oldname); /* replace PS FontName ??? */
    }

    if (nnames == 1)
    {
      /* just one name given */
      if (complainflag) /* eventually flush this error message ? */
      {
        sprintf(logline, "No substitute specified: %s", line);
        showline(logline, 1);
      }

      strcpy(newname, "");
      property |= C_FORCESUB;
    }
    else if (strcmp(newname, oldname) == 0)
    {
      /* `old' name same as `new' name */
      if (complainflag) /* eventually flush this error message ? */
      {
        sprintf(logline, "Substitute same as original: %s", line);
        showline(logline, 1);
      }

      property |= C_FORCESUB;
    }

    if (strchr(newname, '*') != NULL)
    {
      /* only one name given */
      if (complainflag) /* eventually flush this error message ? */
      {
        sprintf(logline, "No substitute specified: %s", line);
        showline(logline, 1);
      }

      strcpy(newname, oldname);
      n = (int) strlen(oldname);    /* step only over first name */
      property |=C_FORCESUB;
    }

    if ((nlen = strlen(oldname)) > sizeof(oldname)-1)
    {
      sprintf(logline, " Font name too long: %s (%d > %d)",
          oldname, nlen, sizeof(oldname)-1);
      showline(logline, 1);
      *(oldname + sizeof(oldname) - 1) = '\0';
      continue;
    }

    if ((nlen = strlen(newname)) > sizeof(newname)-1)
    {
      sprintf(logline, " Font name too long: %s (%d > %d)",
          newname, nlen, sizeof(newname)-1);
      showline(logline, 1);
      *(newname + sizeof(newname) - 1) = '\0';
      continue;
    }

/* try and analyze flags of form  *xyz*  on substitution line */

    s += n;

    if (*s <= ' ')
      *s++ = '\0';     /* isolate start of line */

    strcpy(vector, "");
    sd = strtok(s, " \t\n\r");      /* get next token */

    while (sd != NULL)
    {
      if (*sd == '%')
        break;

      if (*sd == ';')
        break;

      if (strchr(sd, '*') == NULL)
      {
        sprintf(logline, 
          "Do not recognize `%s' in font substitution for: %s\n", 
            sd, line);
        showline(logline, 1);
        errcount(0);
      }
      else if (strcmp(sd, RESIDENT) == 0)
        property |= C_RESIDENT;
      else if (strcmp(sd, FORCESUB) == 0)
        property |= C_FORCESUB;
      else if (strcmp(sd, ALIASED) == 0)
      {
        property |= C_ALIASED;
        aliasesexist++;
      }
      else if (strcmp(sd, SYNTHETIC) == 0 ||
            strcmp(sd, COMPOUND) == 0 ||
              strcmp(sd, HYBRID) == 0)
      {
        property |= C_SYNTHETIC;
        syntheticsexist++;
      }
      else if (strcmp(sd, MTMI) == 0)
        property |= C_MTMI;
      else if (strcmp(sd, EPSF) == 0)
        property |= C_EPSF;
      else if (strcmp(sd, REMAPIT) == 0)
      {
        property |= C_REMAPIT;      /* remap */
        strcpy(vector, "tex_text");    /* default */
        sd = strtok(NULL, " \t\n\r"); /* try and find vector */

        if (sd != NULL)
        {
          /* if no encoding vector given, then use default and go on */
          if (strchr(sd, '*') != NULL)
            continue;

          strncpy(vector, sd, 12 + 1);  /* file-name + extension */

          /* strip off extension, if any - assumes it is "vec" ... */
          if ((u = strchr(vector, '.')) != NULL)
            *u = '\0';

          /* limit to eight characters - otherwise not file name */
          if (strlen(vector) > 8)
            vector[8] = '\0';
        }
      }
      else if (strncmp(sd, "*user-", 6) == 0)
      {
        /* Paul Anagnostopolous memorial hack to allow user-defined markers */
      }
      else
      {
        sprintf(logline,
          "Do not recognize `%s' in font substitution for: %s\n", 
          sd, line);
        showline(logline, 1);
        errcount(0);  
      }

      sd = strtok(NULL, " \t\n\r"); /* advance to next token */
    }

/*  avoid problem when only one font name, or first name equals second name */
    if ((property & C_SYNTHETIC) != 0)
    {
      property &= ~C_FORCESUB;
    }

    if (forcereside != 0)
    {
      property |= C_RESIDENT;
      property |= C_FORCESUB;
    }

    if (avoidreside != 0)
    {
      property &= ~C_RESIDENT;
    }

/* above checks that all items where recognized ??? */

    *(oldname + sizeof(oldname) - 1) = '\0';  /* prevent disaster */
    *(newname + sizeof(newname) - 1) = '\0';  /* prevent disaster */
    *(vector + sizeof(vector) - 1) = '\0';  /* prevent disaster */

    if (fontsubfrom[k] != NULL)
      free(fontsubfrom[k]);

    fontsubfrom[k] = zstrdup(oldname);

    if (fontsubto[k] != NULL)
      free(fontsubto[k]);

    fontsubto[k] = zstrdup(newname);

    if (fontsubvec[k] != NULL)
      free(fontsubvec[k]);

    fontsubvec[k] = zstrdup(vector);
    fontsubprop[k] = property;

/*    if we are being forced to download & no other flags on, ignore */
/*    if (avoidreside != 0 && (property & ~C_FORCESUB) == 0) */
/*    if no substitution & at most *force* on, then ignore it */
    if (strcmp(newname, oldname) == 0 && (property & ~C_FORCESUB) == 0)
      continue;

    k++;
    count++;

    if (k >= maxsubstitute - 1)
    {
      k--;
    } 
  } 

  if (count >= maxsubstitute-1)
  {
    sprintf(logline, " WARNING: Too many font subs (%d > %d)\n",
        count, maxsubstitute);
    showline(logline, 1);
    errcount(0);
  }

  ksubst = k;   /* remember number of entries in substitution table */
 
  return ksubst;
}

/* See whether font file can be found */

/* This tries each pathname on the searchpath - for each pathname: */
/* - tries in order .pfb, .pfa, no extension (Mac format), */
/* - then tries underscores: __.pfb, and __.pfa */
/* - then tries .pss (Multiple Master Stub File) 94/Dec/6 */
/* - and last tries .ps - assumed to be font produced by PKTOPS */
/*  leaves result bits in instanceflag (MM), pssflag (MM), type3flag */

#ifdef SUBDIRSEARCH

/* try and open a font */
FILE *open_font_sub (char *font)
{
  FILE *fp_in = NULL;
  static char fn_in[FNAMELEN];          /* preserve ? why ? */

  task = "trying to open font file";

  if (font == NULL)
    return NULL;

  type3flag = 0;        /* start by assuming Type 1 font */
  pssflag = 0;        /* start by assuming not PSS stub */
  instanceflag = 0;     /* start by assuming not MM instance via PFM */

  if (fontpath == NULL)
    return NULL;

  strcpy(fn_in, font);
  forceexten(fn_in, "pfb"); /* Try PFB form */

  if ((fp_in = findandopen(fn_in, fontpath, NULL, "rb", currentfirst)) != NULL)
    return fp_in;

  forceexten(fn_in, "pfa"); /* Try PFA form */

  if((fp_in = findandopen(fn_in, fontpath, NULL, "rb", currentfirst)) != NULL)
    return fp_in;

  removeexten(fn_in);     /* Try Mac form */

  if ((fp_in = findandopen(fn_in, fontpath, NULL, "rb", currentfirst)) != NULL)
    return fp_in;

  if (tryunderscore != 0)
  {
    if (underscore(fn_in))
    {
      forceexten(fn_in, "pfb"); /* Try PFB form */

      if((fp_in = findandopen(fn_in, fontpath,  NULL, "rb", currentfirst)) != NULL)
        return fp_in;

      forceexten(fn_in, "pfa"); /* Try PFA form */

      if((fp_in = findandopen(fn_in, fontpath, NULL, "rb", currentfirst)) != NULL)
        return fp_in;
    }
  }

/*  Now deal with ATM 4.0 way of doing multiple masters 97/June/1 */
  removeexten(fn_in);         /* remove extension again */
  remove_under(fn_in);         /* remove underscores again */
  forceexten(fn_in, "pfm");     /* MM PFM file 97/June/1 */
/*  check in listed directories *and* PFM subdirectories */
  if ((fp_in = findandopen(fn_in, fontpath, NULL, "rb", -1)) != NULL)
  {
    /* check whether ordinary PFM or MM instance PFM file */
    if (traceflag)
    {
      sprintf(logline, "Found %s\n", fn_in);  /* debugging */
      showline(logline, 0);
    }

    instanceflag = 1;

    return fp_in;
  }

  if (tryunderscore != 0)
  {
    if (underscore(fn_in))
    {
      if ((fp_in = findandopen(fn_in, fontpath, NULL, "rb", currentfirst)) != NULL)
      {
        /* check whether ordinary PFM or MM instance PFM file */
        if (traceflag)
        {
          sprintf(logline, "Found %s\n", fn_in);  /* debugging */
          showline(logline, 0);
        }

        instanceflag = 1;

        return fp_in;
      }
    }
  }

/*  Now deal with older way of doing multiple master fonts */
  removeexten(fn_in);         /* remove extension again */
  remove_under(fn_in);         /* remove underscores again */
  forceexten(fn_in, "pss");     /* MM PSS stub file 94/Dec/6 */

  if ((fp_in = findandopen(fn_in, fontpath, NULL, "rb", currentfirst)) != NULL)
  {
    pssflag = 1;          /* make a note */
    return fp_in;
  }

/*  Now deal with old Type 3 PK font files made by PKTOPS */
  forceexten(fn_in, "ps");      /* try PKTOPS output format */

  if ((fp_in = findandopen(fn_in, fontpath, NULL, "rb", currentfirst)) != NULL)
  {
    type3flag = 1;          /* not reliable - test later */

    return fp_in;
  }

  return NULL;              /* all variations failed ! */
}

#else
/* try and open a font */
FILE *open_font_sub (char *font)
{
  FILE *fp_in;
  char *searchpath, *s;
  static char fn_in[FNAMELEN];          /* preserve ? why ? */
  
  task = "trying to open font file";
  if (font == NULL) return NULL;
  type3flag = 0;        /* start by assuming Type 1 font */
  if (fontpath == NULL) return NULL;
  searchpath = fontpath;
  for(;;) {
    if ((searchpath=nextpathname(fn_in, searchpath)) == NULL) break;
    s = fn_in + strlen(fn_in) - 1;
    if (*s != ':' && *s != '\\' && *s != '/') strcat(fn_in, "\\");
    strcat(fn_in, font);        /* use make_file_name ??? */
    forceexten(fn_in, "pfb");     /* try .pfb first */
    if ((fp_in = fopen(fn_in, "rb")) != NULL) return fp_in;
    forceexten(fn_in, "pfa");     /* try .pfa next */
    if ((fp_in = fopen(fn_in, "rb")) != NULL) return fp_in;
    removeexten(fn_in);         /* try Mac form next */
    if ((fp_in = fopen(fn_in, "rb")) != NULL) return fp_in;
    if (tryunderscore != 0) {     /* try underscore form ? */
/*      underscore(fn_in); */
      if (underscore(fn_in)) {    /* 95/May/28 */
      forceexten(fn_in, "pfb");     /* try .pfb first */
      if ((fp_in = fopen(fn_in, "rb")) != NULL) return fp_in;
      forceexten(fn_in, "pfa");     /* try .pfa next */
      if ((fp_in = fopen(fn_in, "rb")) != NULL) return fp_in;
      }
    }
    removeexten(fn_in);         /* remove extension again */
    remove_under(fn_in);         /* remove underscores again */
    forceexten(fn_in, "ps");      /* try PKTOPS output format */
    if ((fp_in = fopen(fn_in, "rb")) != NULL) { /* "r" ? */
      type3flag = 1;  return fp_in;   /* not reliable - test later */
    }
  }
  return NULL;              /* all variations failed ! */
}

#endif

typedef struct ATMRegRec
{
  unsigned char nMMM;   // directory path index for MMMName
  unsigned char nPFB;   // directory path index for PFBName
  unsigned char nPFM;   // directory path index for PFMName
  unsigned char MMMflag;  // 0 => TTF, 1 => T1, 2 => MM Master, 4 => MM Instance
  char *FontName;     // PostScript FontName
  char *MMMName;      // File Name of PFM (T1 or MMM instance), MMM (for MM master)
  char *PFBName;      // File Name of PFB (blank for MM instance)
  char *PFMName;      // File Name of PFM (for MM master), MMM (for MM instance)
} ATMRegRec;

extern struct ATMRegRec *ATMFonts;

// New 2000 July 6

FILE *open_font_reg (char *font)
{
  static char fn_in[FNAMELEN];          /* preserve ? why ? */
  char *s;
  int k;
  FILE *input;

  task = "trying to open font file";

  if (font == NULL)
    return NULL;

  type3flag = 0;        /*  Type 1 font */
  pssflag = 0;
  instanceflag= 0;

  if (useatmreg == 0)
    return NULL;

  //  First look for match between DVI TFM and PFB file name
  for (k = 0; k < ATMfontindex; k++)
  {
    strcpy(fn_in, ATMFonts[k].PFBName);

    if ((s = strrchr(fn_in, '.')) != NULL)
      *s = '\0';

    if (_strcmpi(font, fn_in) == 0)
    {
      if (ATMFonts[k].MMMflag & 4)
        instanceflag = 1;

      if (! instanceflag)
      {
        strcpy(fn_in, DirPaths[ATMFonts[k].nPFB]);
        strcat(fn_in, ATMFonts[k].PFBName);   // PFB file name
      }
      else
      {
        strcpy(fn_in, DirPaths[ATMFonts[k].nMMM]);
        strcat(fn_in, ATMFonts[k].MMMName);   // PFM file name
      }

      input = fopen(fn_in, "rb");

      return input;
    }
  }

  if (! usepsfontname)
    return NULL;

  //  Then allow for (exact) match between DVI TFM and PS FontName 
  for (k = 0; k < ATMfontindex; k++)
  {
    if (strcmp(font, ATMFonts[k].FontName) == 0)
    {
      if (ATMFonts[k].MMMflag & 4)
        instanceflag = 1;

      if (! instanceflag)
      {
        strcpy(fn_in, DirPaths[ATMFonts[k].nPFB]);
        strcat(fn_in, ATMFonts[k].PFBName);   // PFB file name
      }
      else
      {
        strcpy(fn_in, DirPaths[ATMFonts[k].nMMM]);
        strcat(fn_in, ATMFonts[k].MMMName);   // PFM file name
      }

      input = fopen(fn_in, "rb");

      return input;
    }
  }

  return NULL;
}

/*  Rewritten 1994/March/18 to allow use of `texfonts.map' */
/*  Note that the name in the font-tables is *not* changed */
/*  We only grab a file under a different name and open it */
/*  On screen output, replaced PS FontName etc uses the name in the DVI file */

/* try and open a font */
FILE *open_font (char *font, int aliasflag)
{
  FILE *input=NULL;
  char *aliasname;

  if (font == NULL)
    return NULL;    /* sanity check */

  if (useatmreg)
    input = open_font_reg(font);

  if (input != NULL)
    return input;

  input = open_font_sub(font);

  if (input != NULL)
    return input;

/*  time for aliases? allowed to use texfonts.map? or have we already used? */
  if (aliasflag && bTexFontsMap && ! bForceFontsMap)
  {
    aliasname = alias(font);      /* in dvipslog.c */

    if (aliasname != NULL)
    {
      if (traceflag)
      {
        sprintf(logline, " font %s alias %s\n", font, aliasname);
        showline(logline, 0);
      }

      if (useatmreg)
        input = open_font_reg(font);

      if (input != NULL)
        return input;

      input = open_font_sub(aliasname);  /* with some luck we find it */

      if (input != NULL)
        return input;
    }
  }

  return input;
}

char fn_pfm[FNAMELEN];      /* preserve ? why ? */

/* try and open a PFM file */
FILE *open_pfm (char *font)
{
  FILE *fp_in=NULL;

#ifndef SUBDIRSEARCH
  char *s, *t;
  char *searchpath;
#endif
  
  if (fontpath == NULL)
    return NULL;

#ifdef SUBDIRSEARCH
  strcpy(fn_pfm, font);
  forceexten(fn_pfm, "pfm");      /* force .pfm */
  fp_in = findandopen(fn_pfm, fontpath, NULL, "rb", currentfirst);

  if (fp_in != NULL)
    return fp_in;

  if (tryunderscore)
  {
    if (underscore (fn_pfm))
    {
      fp_in = findandopen(fn_pfm, fontpath, NULL, "rb", currentfirst);

      if (fp_in != NULL)
        return fp_in;
    }
  }

  strcpy(fn_pfm, "pfm\\");
  strcat(fn_pfm, font);
  forceexten(fn_pfm, "pfm");      /* force .pfm */
  fp_in = findandopen(fn_pfm, fontpath, NULL, "rb", currentfirst);

  if (fp_in != NULL)
    return fp_in;

  if (tryunderscore)
  {
    if (underscore (fn_pfm))
    {
      fp_in = findandopen(fn_pfm, fontpath, NULL, "rb", currentfirst);

      if (fp_in != NULL)
        return fp_in;
    }
  }
#else
  searchpath = fontpath;

  for ( ; ; )
  {
    if ((searchpath=nextpathname(fn_pfm, searchpath)) == NULL)
      break;

    s = fn_pfm + strlen(fn_pfm) - 1;

    if (*s != ':' && *s != '\\' && *s != '/')
      strcat(fn_pfm, "\\");

    t = s + strlen(s);
    strcat(fn_pfm, font);       /* use make_file_name ??? */
    forceexten(fn_pfm, "pfm");      /* force .pfm */

    if ((fp_in = fopen(fn_pfm, "rb")) != NULL)
      return fp_in;

    if (tryunderscore != 0)
    {
      if (underscore(fn_pfm))
      {
        forceexten(fn_pfm, "pfm");

        if ((fp_in = fopen(fn_pfm, "rb")) != NULL)
          return fp_in;
      }
    }

    strcpy(t, "pfm\\");         /* try in subdirectory */
    strcat(fn_pfm, font);       /* use make_file_name ??? */
    forceexten(fn_pfm, "pfm");      /* force .pfm */

    if ((fp_in = fopen(fn_pfm, "rb")) != NULL)
      return fp_in;

    if (tryunderscore != 0)
    {
      if (underscore(fn_pfm))
      {
        forceexten(fn_pfm, "pfm");

        if ((fp_in = fopen(fn_pfm, "rb")) != NULL)
          return fp_in;
      }
    }
  }
#endif
  if (traceflag)
  {
    sprintf(logline, "Failed to open %s\n", fn_pfm);    /* debugging only */
    showline(logline, 0);
  }

  return NULL;            /* all variations failed ! */
}

/* first look for same font different sizes */
void flag_duplicates (void)
{
  int k, i, j;
  char *fromfont;
  char *tofont;

  for (k = 0; k < fnext; k++)
  {
    if (fontsubflag[k] >= 0)
      continue;

    for (i = k + 1; i < fnext; i++)
    {
      if (fontname[k] != NULL && fontname[i] != NULL &&
          strcmp(fontname[k], fontname[i]) == 0)
      {     
        if (subfontname[i] != NULL)
          free(subfontname[i]);

        subfontname[i] = zstrdup(fontname[k]);
        fontproper[i] |= C_DEPENDENT;

/* flag it as dependent - that is, another size of the same thing */
/* following USED to be the way to test: */
/* font k occurs at other size already if fontname[k] == subfontname[k] */

        fontsubflag[i] = k;     /* point to substitute */
        fromfont = fontchar[i];  
        tofont = fontchar[k]; 

        if (fromfont == NULL)
        {
          sprintf(logline, " BAD fromfont %d ", i);
          showline(logline, 1);
        }

        if (tofont == NULL)
        {
          sprintf(logline, " BAD tofont %d ", k);
          showline(logline, 1);
        }

        for (j = 0; j < MAXCHRS; j++)
          if (fromfont[j] != 0)
            tofont[j] = 1;  /* combine req */
      }
    }
  }
}

/* look up in substitution table what this font maps to */

int font_remap_sub (char *font)
{
  int k;

  if (font == NULL)
    return -1;

  for (k = 0; k < ksubst; k++)
  {
    if (fontsubfrom[k] != NULL &&
        strcmp(font, fontsubfrom[k]) == 0) 
      return k;
  }

  return -1;        /* didn't find a substitute */
}

/* WARNING: following writes back into argument */
/* NOT ANYMORE IT DOESN'T - now caller needs to pick up fontsubto[k] */

/* expects lower case fontname  */
int font_remap (char *font)
{
  int k;
  char oldname[FNAMELEN];
  char newname[FNAMELEN];

  if (font == NULL)
    return -1;
  if ((k = font_remap_sub(font)) >= 0)
  {
    strcpy(newname, fontsubto[k]);

    if (verboseflag && quietflag == 0 &&
      ((fontsubprop[k] & C_RESIDENT) == 0))
    {
      strcpy(oldname, font);
      sprintf(logline, "Substituting %s for %s\n", newname, oldname);
      showline(logline, 1);
    }
  }

  return k;   /* pointer to substitute */
}

/* need also copy across fontsubprop[k] to fontproper[.] ? */
/* not announce if its just a remap ? */
/* not announce if its just printer resident ? */

/*  see whether this font is being forced to be substituted */
/* expects lower case fontname  */
int forced_substitute (char *font)
{
  int k;

  if (font == NULL)
    return -1;

  for (k = 0; k < ksubst; k++)
  {
    if ((fontsubprop[k] & C_FORCESUB) != 0 && 
        fontsubfrom[k] != NULL &&
        strcmp(font, fontsubfrom[k]) == 0)
    {
      return k;   /* pointer to substitute */
    }
  }

  return -1;        /* didn't find a forcing substitution */
} 

/*  is this font synthetic based on substitution table -- 1993/Nov/6 */
/* expects lower case fontname  */
int check_synthetic (char *font)
{
  int k;

  if (font == NULL)
    return -1;

  for (k = 0; k < ksubst; k++)
  {
    if ((fontsubprop[k] & C_SYNTHETIC) != 0 && 
        fontsubfrom[k] != NULL &&
        strcmp(font, fontsubfrom[k]) == 0)
    {
      return k;   /* pointer to synthetic */
    }
  }

  return -1;
} 

/*  see whether font name is being aliased based on font substitution table */
/* replace aliases with real names */
void replace_alias (void)
{
  int k, n;
  char newname[FNAMELEN];
  char oldname[FNAMELEN];
  
  task = "looking for aliases";

  for (k = 0; k < fnext; k++)
  {
    if (fontname[k] != NULL)
      strcpy(oldname, fontname[k]);

    *oldname = '\0';

    if ((n = font_remap_sub(oldname)) >= 0 &&
      (fontsubprop[n] & C_ALIASED) != 0)
    {
      strcpy(newname, fontsubto[n]);

      if (verboseflag && quietflag == 0)
      {
        sprintf(logline, "Using %s for %s\n", newname, oldname);  
        showline(logline, 0);
      }

      if (fontname[k] != NULL)
        free(fontname[k]);

      fontname[k] = zstrdup(newname);
    }
  }
}

/*  see whether font name is being aliased based on texfonts.map 95/Dec/29 */
/* replace aliases with real names */
void replace_tex_fonts_map (void)
{
  int k;
  char *s;
  char newname[FNAMELEN];
  char oldname[FNAMELEN];
  
  task = "looking for aliases";

  for (k = 0; k < fnext; k++)
  {
    if (fontname[k] != NULL)
      strcpy(oldname, fontname[k]);
    else
      *oldname = '\0';

    if ((s = alias(oldname)) != NULL)
    {
      strcpy(newname, s);

      if (verboseflag && quietflag == 0)
      {
        sprintf(logline, "Using %s for %s\n", newname, oldname);  
        showline(logline, 0);
      }

      if (fontname[k] != NULL)
        free(fontname[k]);

      fontname[k] = zstrdup(newname);  /* copy real name */
    }
  }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* returns 0 normally, -1 on EOF, +1 if start of new section (128) */

int read_a_line (char *line, FILE *input)
{
  int c;
  char *s = line;

  *line = '\0';
  c = getc(input);
  ungetc(c, input);

  if (c == EOF)
    return -1;    /* is first character  -1 ? */

  if (c == 128)
    return +1;    /* is first character 128 ? */

  while ((c = getc(input)) != EOF && c != '\r' && c != '\n')
    *s++ = (char) c;

  if (c == '\r')
  {
    c = getc(input);

    if (c != '\n')
    {
      ungetc(c, input);
      c = '\n';
    }
  }

  *s++ = (char) c;
  *s = '\0';

  return 0;
}

/*    find MM base file name from PS FontName of base font */
/*    find base font file name */ /* WORK IN PROGRESS !!!! */
/*    only called if bMMShortCut == 0 ??? */
/*    returns -1 if not OK */
/*    picks up PS FontName from subfontname[k] */
/*    puts base font file name in fontname[k] */

/* Could we do this using the new ATMFonts table ??? 2000 July 7 */

int FindMMBaseFile (int k)
{
  char FontName[FNAMELEN];    /* 99/Nov/6 */
  char pfbname[FNAMELEN];     /* 95/May/25 */
  FILE *mmfile = NULL;
  char *s, *sl, *pe;
  int c;

  *pfbname = '\0';

  if (bMMShortCut != 0)
  {
    return 0;         /* just use first 5 chars + "___" ??? */
  }

  if (subfontname[k] != NULL)
    strcpy(FontName, subfontname[k]);
  else
    *FontName = '\0';

#ifdef DEBUG
  if (traceflag)
  {
    sprintf(logline, "Trying to find MM base font PS name for %s\n", FontName);
    showline(logline, 0);
  }
#endif

  if ((s = strchr(FontName, '_')) != NULL && s > FontName)
  {
    if (traceflag)
    {
      sprintf(logline, "Stripping Instance Name %s ", FontName);
      showline(logline, 0);
    }

    *s = '\0';

    if (traceflag)
    {
      sprintf(logline, "=> %s\n", FontName);
      showline(logline, 0);
    }
  }
//  PFB file name written back into second argument
  mmfile = fopenfont(FontName, pfbname, 1); // in dvispeci.c

  if (mmfile != NULL)
  {
    if (traceflag)
    {
      sprintf(logline, "FILE %s ", pfbname);  /* 95/May/27 */
      showline(logline, 0);
    }

    sl = removepath(pfbname);       /* remove file path */

    if ((pe = strchr(sl, '.')) == NULL)
      pe = sl + strlen(sl);       /* ignore extension */

    pe--;

    while (*pe == '_')
      pe--;        /* step back over _ */

    pe++;
    c = *pe;
    *pe = '\0';               /* temporarily terminate */

    if (traceflag)
    {
      sprintf(logline, "=> %s\n", sl);  /* 95/May/27 */
      showline(logline, 0);
    }

    if (strlen(sl) < FNAMELEN)
    {
      if (fontname[k] != NULL)
        free(fontname[k]);

      fontname[k] = zstrdup(sl);
    }
    else
    {
      sprintf(logline, "Name too long: %s\n", sl);
      showline(logline, 1);
    }

    if (pe != NULL)
      *pe = (char) c;     /* restore what was there */

    fclose(mmfile);

    return 0;         /* OK */
  }
  else
  {
    if (traceflag)
    {
      sprintf(logline, " WARNING: MM base `%s' not found ", FontName);
      showline(logline, 1);
    }

/*    just a WARNING: since it drops back to extending with "___" */
/*    assumed to be of form kprg_xyz --- should we check for _ ??? */

    if (fontname[k] != NULL)
      strcpy (pfbname, fontname[k]);
    else
      *pfbname = '\0';

    if (*(pfbname+4) == '_')
    {
      strcpy (pfbname + 5, "___");  /* last three characters */

      if (traceflag)
      {
        sprintf(logline, "using %s ", pfbname);
        showline(logline, 1);
      }

      if (fontname[k] != NULL)
        free(fontname[k]);

      fontname[k] = zstrdup(pfbname);
    }

    return -1;            /* NOT ok */
  }
}

/* get some compiler error here ??? */
/* dviextra.c(4379) : fatal error C1001: internal compiler error */
/*    (compiler file 'msc3.cpp', line 429) */

/* Called with MM instance font number */
/* Finds corresponding MM base font - or adds it if not found */
/* Adds characters needed by new instance */

/* add the corresponding MM base 94/Dec/6 */
int AddMMBase (int m)
{
  int k, i;
  char *basewantchrs; /* wanted chars array of MM base font */
  char *instwantchrs; /* wanted chars array of MM instance */
  char psfontname[FNAMELEN];
  char psbasename[FNAMELEN];
  char *s;

  if (subfontname[m] != NULL)
    strcpy(psfontname, subfontname[m]);
  else
    *psfontname = '\0';

  /*  Truncate MM instance PS FontName if that is what we have 97/June/1 */
  if ((s = strchr(psfontname, '_')) != NULL && s > psfontname)
  {
    *s = '\0';
  }

  instwantchrs = fontchar[m];

  if (fontchar[m] == NULL)
    showline(" BAD instwantchrs", 1);

/*  check whether we have already added this MM base font */

  for (k = 0; k < fnext; k++)
  {
    if ((fontproper[k] & C_MULTIPLE) == 0)
      continue;

    if (subfontname[k] != NULL)
      strcpy(psbasename, subfontname[k]);
    else
      *psbasename = '\0';
/*  old way check whether matches on first five characters of font file name */
#ifdef DEBUG
    if (traceflag)
    {
      sprintf(logline, "COMPARE %s and %s\n", psbasename, psfontname);
      showline(logline, 0);
    }
#endif
    if (bMMShortCut)
    {
      if (fontname[k] != NULL && fontname[m] != NULL &&
          strncmp(fontname[k], fontname[m], 5) != 0)
          continue;
    }
    else if (strcmp(psbasename, psfontname) != 0)
      continue;

#ifdef DEBUG
    if (traceflag)
    {
      sprintf(logline, "Already installed MM base font %s %d\n", psbasename, k);
      showline(logline, 0);
    }
#endif
    if (fontchar[k] == NULL)
      setupfontchar(k);

    basewantchrs = fontchar[k];

    for (i = 0; i < MAXCHRS; i++)
      if (instwantchrs[i] != 0)
        basewantchrs[i] = 1;

    return k;
  }   

#ifdef DEBUG
  if (traceflag)
  {
    sprintf(logline, "Need to construct new MM base entry %s\n", psfontname);
    showline(logline, 0);
  }
#endif

/*  Get here only if MM base font not found, have to add a new MM base font */
/*  We construct MM base font file name from stub PSS file name */
/*  Typically MM base font has same name, but ends in ___ ??? */
//  strcpy(fontname + fnext * MAXTEXNAME, fontname + m * MAXTEXNAME);
//  strcpy(fontname + fnext * MAXTEXNAME + 5, "___");
  if (fontname[fnext] != NULL)
    free(fontname[fnext]);

  if (fontname[m] != NULL)
  {
    char buffer[FNAMELEN];
    strcpy(buffer, fontname[m]);
    strcpy(buffer+5, "___");    /* MM Master ends in ___ */
    fontname[fnext] = zstrdup(buffer);
  }
  else
    fontname[fnext] = NULL;

#ifdef DEBUG
  if (traceflag)
  {
    sprintf(logline, "New base font with PS FontName %s\n", psfontname);
    showline(logline, 0);
  }
#endif

  if (subfontname[fnext] != NULL)
    free(subfontname[fnext]);

  subfontname[fnext] = zstrdup(psfontname);
/*  hope this does not create problems with tests on subfontname ... */
/*  should we call FindMMBaseFile here to fill in fontname[fnext] ? */
/*  or do this later when inserting base font ? */

  fontproper[fnext] = C_MULTIPLE;
  fontsubflag[fnext] = -1;
  if (fontchar[fnext] == NULL)
    setupfontchar(fnext);

  basewantchrs = fontchar[fnext];

  for (i = 0; i < MAXCHRS; i++) /* just copy, first instance */
    basewantchrs[i] = instwantchrs[i];

  if (bMMShortCut == 0)
  {
    if (FindMMBaseFile(fnext) != 0)
    {
      /* fprintf(errout, " WARNING: MM base `%s' not found\n", FontName); */
    }
  }

  fnext++;
  mmcount++;                  /* how many MM base fonts */

  if (fnext >= maxfonts - 1)
  {
    sprintf(logline, " ERROR: Too many MM base fonts (%d)\n", mmcount);
    showline(logline, 1);
    errcount(0);
    fnext--;
  }

  return fnext;
}

/* Now add in the base fonts for MM instances - separated out 95/May/12 */
/* Add this to font table *after* base fonts for substitution */
/* Do we get into any problems because of not checking for substitution ? */
/* Not used anymore ? */

/* Old style PSS stubs look like this: */

/* 128, 1, x, x, x, x (length)
%!PS-AdobeFont-1.0 TektonMM_100_LT_564_NO 001.000
/TektonMM_100_LT_564_NO /TektonMM findfont [.63058 0 .36942 0] makeblendedfont
dup /FontName /TektonMM_100_LT_564_NO put
definefont pop
128, 3 */

/* which becomes the following */

/* %!PS-AdobeFont-1.0 TektonMM_100_LT_564_NO 001.000
/zjrg_001 /zjrg____ findfont [.63058 0 .36942 0] makeblendedfont
dup /FontName /zjrg_001 put
definefont pop */

/*  Get PS FontName from PSS stub file and stick in subfontname[k] */
/*  NOTE: in case of PSS stub, this will be the *base font* PS FontName */
/*  Get PS FontName from PFM metric file and stick in subfontname[k] */
/*  NOTE: in case of PFM file, this will be the *instance* PS FontName */

/* Could we do this using the new ATMFonts table ??? 2000 July 7 */

//int AddMMFontName (FILE *input, int k, int pssflag, char *FileName) 
int AddMMFontName (FILE *input, int k, int pssflag, char *FileName, int nlen)
{
  int c, m, flag;
  char *s;

  if (subfontname[k] != NULL) /* reset in case not found */
  {
    free(subfontname[k]);
    subfontname[k] = NULL;
  }

  /*  PSS file case first --- now outdated since ATM NT no longer */
  if (pssflag)    /* read PSS file to extract MM base FontName */
  {
    if ((c = getc(input)) != 128)
      return -1;  /* sanity test */

    if ((c = getc(input)) != 1)
      return -1;    /* sanity test */

    for (m = 0; m < 4; m++)
      (void) getc(input); /* skip over length */

    while (read_a_line(line, input) == 0)
    {
      if (*line == '%')
        continue;       /* ignore comments */

      if ((s = strstr(line, "findfont")) != NULL)
      {
        s--;

        while (*s <= ' ' && s >= line)
          *s-- = '\0';

        if ((s = strrchr(line, '/')) == NULL)
          return -1;

        s++;

        if (subfontname[k] != NULL)
          free(subfontname[k]);

        subfontname[k] = zstrdup(s);

        return 0;
      }
    }

    return -1;
  }
  else
  {
    flag = NamesFromPFM(input, NULL, 0, line, nlen, FileName);

    if (flag != 0 && strchr(line, '_') != NULL)
    { 
      if (subfontname[k] != NULL)
        free(subfontname[k]);

      subfontname[k] = zstrdup(line);

#ifdef DEBUG
      if (traceflag)
      {
        sprintf(logline, "MM PS FontName %s\n", line);
        showline(logline, 0);
      }
#endif

      return 0;
    }
    else
    {  
      /* NamesFromPFM failed or no _ in name returned */
      /* this message now comes up for missing fonts ... */
      sprintf(logline,
          " Not a PFM file for a MM instance (%s) ", line);
      showline(logline, 1);
    }

    return -1;
  }
}

int notvalidPSS (int pssflag)  /* PSS or PFM file */
{
  sprintf(logline, " ERROR: not a valid %s file ",
       pssflag ? "MM PS Stub" : "PFM");
  showline(logline, 1);

  return -1;
}

/* This tries to open font files to see which actually exist */
/* also checks forced substitution flags */ /* should check resident ? */

/* we are assuming this happens *before* we start adding MM base fonts */

void check_substitute (void)
{
  FILE *fp_in;
  int k, j, i, n, baseexist, wipeflag;
  char fontnamek[FNAMELEN];
  char subfontnamek[FNAMELEN];          /* fix 1995/July/15 */
  char masterfontname[FNAMELEN];        /* saved 1995/July/15 */
  char *fromfont;
  char *tofont;

  task = "looking for substitutions";

  for (k = 0; k < fnext; k++)
  {
    wipeflag = 0;

    if ((fontproper[k] & C_UNUSED) != 0)
      continue;

    /* don't bother if this font is another size of an existing one */
    if ((fontproper[k] & C_DEPENDENT) != 0)
      continue;

    if (fontname[k] != NULL)
      strcpy(fontnamek, fontname[k]);
    else
      *fontnamek = '\0';

    if ((n = forced_substitute(fontnamek)) >= 0 ||
      (fp_in = open_font(fontnamek, 1)) == NULL)
    {
      /* the following is a hack to deal with conflicing use of subfontname */
      /* both for substitution and for MM PS FontName ... */
      /* attempt to allow remapping (without name change) */

      if (subfontname[k] != NULL)
      {
        wipeflag = 1;
        strcpy(masterfontname, subfontname[k]);

        if (traceflag)
        {
          sprintf(logline, "Wiping out %s with %s (%d)\n",
               masterfontname, fontnamek, fontproper[k]);
          showline(logline, 0);
        }
      }
    
      if (subfontname[k] != NULL)
        free(subfontname[k]);

      if (fontname[k] != NULL)
        subfontname[k] = zstrdup(fontname[k]);  /* copy real name */
      else
        subfontname[k] = NULL;

/* in forced substitution we actually already know n ... */
/* WARNING: font_remap writes new name back into given argument ... */
/*      so need to pass actual  argument */

      if ((n = font_remap(subfontname[k])) >= 0)
      {
        if (subfontname[k] != NULL)
          free(subfontname[k]);

        subfontname[k] = zstrdup(fontsubto[n]);   // 99/Nov/17
        fontproper[k] |= fontsubprop[n];  /* check */

        if ((fontproper[k] & C_REMAPIT) == 0)
          fontsubflag[k] = n;
        else
        {
          if (fontvector[k] != NULL)
            free(fontvector[k]);

          fontvector[k] = zstrdup(fontsubvec[n]);

          if (bCheckEncoding)
            check_remap_enc(k, fontnamek);
        }

        if ((fontproper[k] & C_FORCESUB) != 0 && 
          (fontproper[k] & C_RESIDENT) == 0)
        {
          if (subfontname[k] != NULL)
            strcpy(subfontnamek, subfontname[k]);
          else
            *subfontnamek = '\0';

          if ((fp_in = open_font(subfontnamek, 1)) == NULL)
          {
            sprintf(logline, 
             "Font %s (substituted for %s) could not be found\n",
              subfontnamek, fontnamek);
            showline(logline, 1);
          }
          else
          {
            if (traceflag)
            {
              sprintf(logline, "Substituting * %s for %s\n", 
                 subfontnamek, fontnamek);/* debug 95/Mar/31 */
              showline(logline, 0);
            }

            fclose(fp_in);
          }
        }
      }
      else
      {
        fontproper[k] |= C_MISSING;   /* new */
        sprintf(logline, 
          " WARNING: Can't find font %s (and no substitute)\n", 
            fontnamek);
        showline(logline, 1);
        errcount(0);
      }

      if (wipeflag)
      {
        if (subfontname[k] != NULL)
          free(subfontname[k]);

        subfontname[k] = zstrdup(masterfontname);

        if (traceflag)
        {
          sprintf(logline, "Restoring %s (%d)\n", masterfontname, fontproper[k]);
          showline(logline, 0);
        }

        wipeflag = 0;
      }
    }
    else
    {
      if (instanceflag != 0)
      {
        fontproper[k] |= C_INSTANCE;
        fontproper[k] |= C_NOTBASE; 

        if (AddMMFontName(fp_in, k, pssflag,
                  fontnamek, sizeof(fontnamek)) != 0)
        {
          notvalidPSS(pssflag); /* specify which PSS file ? */
          sprintf(logline, "`%s' ", fontnamek);
          showline(logline, 0);
        }

        AddMMBase(k);   /* add corresponding MM base */
      }

      if (pssflag != 0)
      {
        fontproper[k] |= C_INSTANCE;

        if (AddMMFontName(fp_in, k, pssflag, 
                  fontnamek, sizeof(fontnamek)) != 0)
        {
          notvalidPSS(pssflag); /* specify which PSS file ? */
          sprintf(logline, "`%s' ", fontnamek);
          showline(logline, 0);
        }

        AddMMBase(k);   /* add corresponding MM base */
      }

      fclose(fp_in);

      if (syntheticsexist)
      {
        if (check_synthetic(fontnamek) >= 0)
        {
          fontproper[k] |= C_SYNTHETIC;
        }
      }
    }
  }

  task = "checking whether base font exists";

  for (k = 0; k < fnext; k++)
  {
    if (fontsubflag[k] >= 0)
    {
      baseexist = 0;

      if ((fontproper[k] & C_RESIDENT) != 0)
        baseexist = 1;
      else if ((fontproper[k] & C_REMAPIT) != 0)
        baseexist = 1; 
      else
      {
        for (i = 0; i < fnext; i++)
        {
          if (i == k)
            continue;

          if (fontname[i] != NULL && subfontname[k] != NULL &&
              strcmp(fontname[i], subfontname[k]) == 0)
          {
            fontsubflag[k] = i; baseexist = -1;
            fromfont = fontchar[k];
            tofont = fontchar[i];

            if (fromfont == NULL)
            {
              sprintf(logline, " BAD fromfont %d ", k);
              showline(logline, 1);
            }

            if (tofont == NULL)
            {
              sprintf(logline, " BAD tofont %d ", i);
              showline(logline, 1);
            }

            for (j = 0; j < MAXCHRS; j++)
              if (fromfont[j] != 0)
                tofont[j] = 1;

            break;
          }
        }
      }

      if (baseexist == 0) /* base font does not exist - create it */
      {
        task = "adding in base font";

        if (fnext >= maxfonts - 1)
        {
          sprintf(logline, 
             "Too many base fonts and substitutions (%d)\n", fnext);
          showline(logline, 1);
          errcount(0);
          fnext--;
        }

        fontsubflag[k] = fnext;

        if (fontname[fnext] != NULL)
          free(fontname[fnext]);

        if (subfontname[k] != NULL)
          fontname[fnext] = zstrdup(subfontname[k]);
        else
          fontname[fnext] = NULL;

        if (subfontname[fnext] != NULL)
        {
          free(subfontname[fnext]);
          subfontname[fnext] = NULL;
        }

/* following may have been exposed by fixing bug in previous comment line .. */
        fontproper[fnext] = fontproper[k];
        fontsubflag[fnext] = -1;

        if (fontchar[fnext] == NULL)
          setupfontchar(fnext);

        fromfont = fontchar[k]; 
        tofont = fontchar[fnext];

        if (fromfont == NULL)
        {
          sprintf(logline, " BAD fromfont %d ", k);
          showline(logline, 1);
        }

        if (tofont == NULL)
        {
          sprintf(logline, " BAD tofont %d ", fnext);
          showline(logline, 1);
        }
        
        /*    copy across to base font those chars used in font substituted for */
        for (j = 0; j < MAXCHRS; j++)
          tofont[j] = fromfont[j];

        fnext++;
      }
    }
  }
}

/* Font substitution file is first searched for in dvi directory */
/* then in default substitution file directory */
/* now pass in subfile as argument so we can read more than one 1994/Feb/4 */

/* FILE *OpenFontsub(void) {  */
FILE *OpenFontsub (char *subfile)
{
  FILE *input;

  if (strcmp(subfile, "") == 0)
    return NULL;

  if (strpbrk(subfile, "\\/:") != NULL)
  {
    strcpy(fontsubfile, subfile); 
    extension(fontsubfile, "sub"); 

    return(fopen(fontsubfile, "r"));
  }

/*  if not fully qualified name, try in DVI file directory */
  if (dvipath != NULL)
    strcpy(fontsubfile, dvipath);
  else
    strcpy(fontsubfile, "");

  make_file_name(fontsubfile, subfile);
  extension(fontsubfile, "sub");

  if ((input = fopen(fontsubfile, "r")) != NULL)
    return input;

/*  try in specified font substitution `path' */
  strcpy(fontsubfile, fontsubpath);
  make_file_name(fontsubfile, subfile);   /* 1992/Nov/28 */
  extension(fontsubfile, "sub");  

  if ((input = fopen(fontsubfile, "r")) != NULL)
    return input;

/*  try in SUBDIRECTORY of font substitution `path' *//* 1992/Nov/28 */
  strcpy(fontsubfile, fontsubpath);
  make_file_name(fontsubfile, "sub\\");   /* "sub" sundirectory */
  strcat(fontsubfile, subfile);
  extension(fontsubfile, "sub");

  if ((input = fopen(fontsubfile, "r")) != NULL)
    return input;

/*  try in current directory */
  *fontsubfile = '\0';
  make_file_name(fontsubfile, subfile);
  extension(fontsubfile, "sub");

  if ((input = fopen(fontsubfile, "r")) != NULL)
    return input;

  return NULL;
}

/* see whether font actually used */
int char_needed (char *wantchrs)
{
  int k;

  if (wantchrs == NULL)
    return 0;

  for (k = 64; k < 128; k++)
    if (wantchrs[k])
      return -1;

  for (k = 32; k < 64; k++)
    if (wantchrs[k])
      return -1;

  for (k = 0; k < 32; k++)
    if (wantchrs[k])
      return -1;

  for (k = 160; k < fontchrs; k++)
    if (wantchrs[k])
      return 1; /* G2 */

  for (k = 128; k < 160; k++)
    if (wantchrs[k])
      return 1;  /* C2 */

  return 0;
}

/* read font substitution table */  /* returns non-zero if success */

/* return value not used ? */
int GetSubstitutes (void)
{
  FILE *input;
  char nextfontsub[FNAMELEN];     /* need for multi sub files */
  int k, flag=0;              /* 1994/Feb/5 */
  char *s, *sc, *sl;              /* 1994/Feb/5 */

  ksubst = 0;             /* reset table 1994/Feb/4 */
/*  note this may modify the command line, but that is not reused ... */
  for (k = 0; k < subfontfileindex; k++) {  /* 1994/Feb/8 */
/*    s = fontsubrest; */
    s = subfontfile[k];           /* 1994/Feb/8 */
    while (s != NULL) {
      if ((sc = strchr(s, ',')) != NULL) *sc = '\0';
/*      if ((input = OpenFontsub()) != NULL) { */
/*      input = OpenFontsub(fontsubrest); */
      strcpy(nextfontsub, s);
/*      if ((input = OpenFontsub(s)) != NULL) { */
      if ((input = OpenFontsub(nextfontsub)) != NULL)
      {
/*        if (traceflag)
          printf("Reading %s\n", nextfontsub); */ /* debugging */
        read_substitutes(input);   /* read font substitution file */
        fclose(input);                /* 1994/Feb/4 */
        flag++;
/*        if (traceflag) show_sub_table(stdout); */ /* an experiment */ 
/*        return -1; */         /* OK, specified file worked */
      }
      else
      {
        sl = removepath(fontsubfile);
/*        fprintf(errout, "WARNING: Can't find font subst file: "); */
        sprintf(logline, " WARNING: Can't find %s subst file %s\n",
             "font", sl);
        showline(logline, 1);
        perrormod (sl);
/*        putc('\n', errout); */
        errcount(0);
      }
      if (sc != NULL) s = sc+1; /* can't use strtok ... */
      else s = NULL;
    }
  }

/*  else {  */
  if (flag == 0) {      /* if no substitution files specified */
/*    substitute = 0;  */ /* NO: still have built in table ... */
/*    fprintf(errout, "WARNING: Can't find font substitution file\n"); */
/*    perrormod(fontsubfile);  */
/*    perrormod(removepath(fontsubfile)); */      /* 1992/Nov/28 */
/*    errcount(0);  */
/*  then try standard substitution file */
/*    if (strcmp(fontsubrest, "standard") == 0) return 0; *//* failed */
/*    if (strcmp(fontsubrest, "standard") != 0) { */ /* 1994/Feb/10 */
/*      strcpy(fontsubrest, "standard"); */
/*      if((input = OpenFontsub()) != NULL) { */
      if((input = OpenFontsub("standard")) != NULL)
      {
        read_substitutes(input);
        fclose(input);                /* 1994/Feb/4 */
/*        if (traceflag) show_sub_table(stdout); */ /* an experiment */ 
/*        return -1; */         /* OK, standard file worked */
        flag++;
      }
      else
      {
        sl = removepath(fontsubfile);
/*        substitute = 0;  */ /* NO: still have built in table ? */
        sprintf(logline, " WARNING: Can't find %s subst file %s\n",
            "standard", sl);
        showline(logline, 1);
/*        perrormod(fontsubfile);  */
        perrormod(sl);    /* 1992/Nov/28 */
//        errcount(0);
      }
/*    }  */ /* if strcmp fontsubrest standard removed 1994/Feb/10 */
/*    return 0;  */ /* No success ! */
  }
/*  if (traceflag) show_sub_table(stdout); */ /* an experiment */ 
  if (showsubflag)
  {
    show_sub_table(); /* an experiment */
//    if (logfileflag) show_sub_table(logfile);
  }

  return flag;  /* non zero if some substitution table has been read */
}

/* separated out 94/Oct/5 */
int markunusedfonts0 (void)
{
  int k, unused=0;

/*  fontproper[k] &= ~C_UNUSED; */
  for (k = 0; k < fnext; k++)
  {
/*    if (char_needed(fontchar[k]) == 0)  */
//    if (fontproper[k] | C_MULTIPLE) continue; /* for now 94/Dec/6 ??? */
    if (fontproper[k] & C_MULTIPLE) continue; /* fixed 2000 July 4 */
//    if (char_needed(fontchar + MAXCHRS * k) == 0) 
    if (char_needed(fontchar[k]) == 0) {
      fontproper[k] |= C_UNUSED;
      unused++;
    }
  }
  return unused;
}

/* following called from dvipsone.c before calling extract itself */
/* task = "checking for duplicate & substitutions"; */

/* get font tables straightened out first */
void preextract (void)
{
  int k;
/*  FILE *input; */
//  char fontnamek[MAXTEXNAME];
  char fontnamek[FNAMELEN];
//  char subfontnamek[MAXTEXNAME];
  char subfontnamek[FNAMELEN];

/*  if (fnext == 0) return; */      /* no fonts - nothing to do ??? */
/*  mmbase = fnext; */          /* start of MM base fonts if any */
  mmcount = 0;            /* number of MM base fonts added */

  chrs = -1;
/*  Do we want to read substitution table if already forced all reside ? */
/*  YES: need to get PS FontNames if possible etc */
/*  following moved to dvipsone.c 95/Oct/15 */
/*  if (substituteflag) (void) GetSubstitutes(); */ 
/*  if (GetSubstitutes() < 0) fprintf(errout, " Can't get substitutes"); */
  if (fnext == 0) return;       /* moved down here 95/May/8 */

/*  replace aliased names right away based on substitution table */
  if (aliasesexist) replace_alias(); /* replace aliased names right away */

/*  replace aliased names right away based on texfonts.map 1995/Dec/29 */
  if (bForceFontsMap) replace_tex_fonts_map(); /* 1995/Dec/29 */

  flag_duplicates(); /* look for same font at different magnifications */
/*  mark unused fonts here */
/*  markunusedfonts0();  */ /* this was before 94/Oct/5 - problem ? */

  if (substituteflag)   // try to open fonts and 
    check_substitute();  // work out which fonts will be substituted for 

/*  mmbase = fnext; */      /* update start of MM base fonts 95/Mar/31 ??? */
/*  if (bMMNewFlag == 0) AddInBaseFonts();  */ /* old way  95/May/12 */
/*  mark unused fonts here ? */ /* after fonts in different sizes combined */
  markunusedfonts0();   /* new place on 94/Oct/5 */

/*  Deal with situtation where all fonts considered resident */
/*  moved down here 1992/July/4 */  /* do this LAST */
  if (forcereside != 0)
  {
    for (k = 0; k < fnext; k++)
    {
/*      debugging output 95/Sep/16 */
/*      if (fontsubflag[k] < 0) { */      /* 1992/July/4 */
/*  Don't mark fonts substituted for as resident */
/*  Don't mark MM font instances resident.  But, don't know base FontName ? */
/*      if (fontsubflag[k] < 0) { */
      if (fontsubflag[k] < 0 &&
        (fontproper[k] & C_INSTANCE) == 0)  {   /* 1995/Sep/16 */
        fontproper[k] |= C_RESIDENT; 
/*        fontproper[k] |= C_FORCESUB; */
      }
/* Try and get the real PS FontName */ /* Maybe do earlier ??? */
/* Avoid repetition ??? - don't do if C_DEPENDENT flag is on ??? */
/* Don't mess with it if a substitute has already been found in table ??? */
/* Don't do if C_FORCESUB is on ??? */
/*      if (strcmp(subfontname[k], fontname[k]) == 0)   */
/*      if ((fontproper[k] & C_DEPENDENT) == 0 &&
        (fontproper[k] & C_FORCESUB) == 0) */
/*      if ((fontproper[k] & C_DEPENDENT) == 0) { */
/* In case of font instance, don't want to overwrite MM base name */
/* Also, don't really need this for MM base, since already done ... */
      if ((fontproper[k] & C_DEPENDENT) == 0 &&
        (fontproper[k] & C_INSTANCE) == 0) {    /* 1995/Sep/16 */
//        strcpy (fontnamek, fontname + k * MAXTEXNAME);
        if (fontname[k] != NULL) strcpy(fontnamek, fontname[k]);
        else *fontnamek = '\0';
//        strcpy (subfontnamek, subfontname + k * MAXTEXNAME);
        if (subfontname[k] != NULL) strcpy(subfontnamek, subfontname[k]);
        else *subfontnamek = '\0';
#ifdef DEBUG
        if (traceflag)
        {
          sprintf(logline, "%d %s %s %d\n", k,  fontnamek, subfontnamek,
            fontproper[k]);   /* debugging */
          showline(logline, 0);
        }
#endif
/*        WARNING: FindFontName writes back into second argument ! */
/*        FindFontName(fontname[k], subfontname[k]);*//* 1993/Sep/30 */
//        FindFontName(fontnamek, subfontnamek, MAXFONTNAME); /* 1994/Feb/2 */
        FindFontName(fontnamek, subfontnamek, sizeof(subfontnamek)); /* FNAMELEN */
/*        printf("%d %s %s %d\n", k, fontnamek, subfontnamek,
            fontproper[k]); */  /* debugging */
/* need to copy back result 1994/Feb/3 */
//        strcpy(subfontname + k * MAXTEXNAME, subfontnamek);
        if (subfontname[k] != NULL) free(subfontname[k]);
        subfontname[k] = zstrdup(subfontnamek);
      }
    }
  }
}

/* following contains kludgy test to avoid repeating vector */

void constructvectors(FILE *fp_out)
{
  int k, i, flag;

/*  maybe nothing to do here if just need ansinew due to `-X' flag */
/*  we should already have the ansinew vector dumped out */

  write_dvi_start(fp_out);        /* 93/Sep/30 */
  for (k=0; k < fnext; k++)
  {
    if ((fontproper[k] & C_REMAPIT) != 0 &&
        (fontproper[k] & C_RESIDENT) != 0)
    {
      flag = 0;
      for (i = 0; i < k; i ++)
      {
        if ((fontproper[i] & C_REMAPIT) != 0 &&
            (fontproper[i] & C_RESIDENT) != 0 &&
/*          strcmp(fontvector + i * MAXVECNAME, fontvector + k * MAXVECNAME) == 0) */
            fontvector[i] != NULL &&
            fontvector[k] != NULL &&
            strcmp(fontvector[i], fontvector[k]) == 0)
            {
              flag = 1; break;
            }
      }
      if (flag == 0) {
//        read_enc(fontvector + k * MAXVECNAME);
        read_enc(fontvector[k]);
//        write_vector(fp_out, fontvector[k], MAXCHRS);
//        write_vector(fp_out, fontvector + k * MAXVECNAME, MAXCHRS);
        write_vector(fp_out, fontvector[k], MAXCHRS);
      }
    }
  }
/*  fprintf(fp_out, "%s\n", tex_text); */  /* for text fonts */
/*  fprintf(fp_out, "%s\n", textype); */  /* for typewriter fonts */

  write_dvi_end(fp_out);        /* 1992/Nov/17 */
}

/* Check whether font itself, or its base font are resident */

int fontreside (int f)              /* 1995/July/5 */
{
  int k;
  if ((fontproper[f] & C_RESIDENT) != 0) return 1;
  if ((fontproper[f] & C_DEPENDENT) != 0) {
    k = fontsubflag[f];
    if (k >= 0) {
      if ((fontproper[k] & C_RESIDENT) != 0) return 1;
    }
  }
  return 0;
}

/* suppress above remapped font is not resident ? */

/* f is always the short font number */
/* fn is the internal font number if bShortFont == 0 || bUseInternal != 0 */
/* fn is the short font number if bShortFont != 0 && bUseInternal == 0 */

void dofont (FILE *fp_out, int f, int fn)
{
//  char fname[MAXFONTNAME];
  char fname[FNAMELEN];
  int n;
  int property;

  if (f < 0 || f >= maxfonts) {       /* 93/May/23 */
    sprintf(logline, "Bad font index %d\n", f); /* debugging */
    showline(logline, 1);
/*    should we skip it in this case to avoid access errors ? */
  }

  property = fontproper[f];         /* 97/June/1 */
/*  if (traceflag)
    printf("font %d fontname %s subfontname %s fontsubflag %d\n",
      f, fontname[f], subfontname[f], fontsubflag[f]); */
//  strcpy(fname, fontname + f * MAXTEXNAME);
  if (fontname[f] != NULL) strcpy(fname, fontname[f]);
  else *fname = '\0';

/*  if (strcmp(subfontname[f], "") == 0) { */
//  if (*(subfontname + f * MAXFONTNAME) == '\0') /* 1994/Feb/2 */
  if (subfontname[f] == NULL) {
/*    strcpy(fname, fontname[f]); */
/*    strcpy(fname, fontname + f * MAXTEXNAME); */
  }
  else {          /* i.e. subfontname[f] is not empty */
/*    strcpy(fname, fontname[f]); */        /* if nothing else */
/*    strcpy(fname, fontname + f * MAXTEXNAME); */  /* if nothing else */
    if ((property & C_DEPENDENT) != 0) {    /* NEW */
/*      if (strcmp(fontname[f], subfontname[f]) == 0) {  */
/*      if this font already used earlier at other size */
      n = fontsubflag[f];
#ifdef DEBUG
      sprintf(logline, "%d %s (%d)", f, fname, n); /* debugging */
      showline(logline, 0);
//      sprintf(logline, "%s", (subfontname + f * MAXFONTNAME));
      sprintf(logline, "%s", subfontname[f]);
      showline(logline, 0);
      showline("\n", 0);
#endif
      if ((fontproper[n] & C_RESIDENT) != 0) { /* what ? */
        if ((fontproper[n] & C_REMAPIT) == 0) { /* ? */
/*          strcpy(fname, fontname[n]); */    /* 1992/July/4 */
//          strcpy(fname, fontname + n * MAXTEXNAME); /* 94/Feb/2 */
          if (fontname[n] != NULL) 
            strcpy(fname, fontname[n]); /* 94/Feb/2 */
          else *fname = '\0';
/*          if (strcmp(subfontname[n], "") != 0) */ /* 1992/July/4 */
//          if (*(subfontname + n * MAXFONTNAME) != '\0') /* 94/Feb/2 */
          if (subfontname[n] != NULL) {
/*            strcpy(fname, subfontname[n]);  */
            strcpy(fname, subfontname[n]);
          }
        }
/*        printf("Remapped %d to %s\n", f, fname); */
      }
    }       /* end of (proper & C_DEPENDENT) != 0 case */
    else if ((property & C_RESIDENT) != 0) {
/*    if this font used here for first time */
      if ((property & C_REMAPIT) == 0) {
/*        strcpy(fname, subfontname[f]);  */
//        strcpy(fname, subfontname + f * MAXFONTNAME); 
        if (subfontname[f] != NULL) strcpy(fname, subfontname[f]); 
        else *fname = '\0';
      }
/*        printf("Remapped %d to %s\n", f, fname);  */
    } /* end of (proper & C_RESIDENT) != 0 case */
  }

/*  Use remapped name instead of FontName if resident and ANSI */
  if ((property & C_RESIDENT) != 0 &&
    (property & C_REMAPIT) == 0 && bWindowsFlag != 0) {
/* special case sy = Symbol and zd = ZapfDingbats ? */  /* 94/Feb/3 */
/*      strcpy(fname, fontname[f]); */  /* experiment 93/Oct/4 */
//      strcpy(fname, fontname + f * MAXTEXNAME); /* 94/Feb/2 */
    if (fontname[f] != NULL) strcpy(fname, fontname[f]);  /* 94/Feb/2 */
    else *fname = '\0';
  }
  if ((property & C_DEPENDENT) != 0) {  /* experiment 93/Oct/7 */
    n = fontsubflag[f];         /* what are we dependent on */
/*  Use remapped name instead of FontName if dependent of resident and ANSI */
    if ((fontproper[n] & C_RESIDENT) != 0 &&
      (fontproper[n] & C_REMAPIT) == 0 && bWindowsFlag) {
/* special case sy = Symbol and zd = ZapfDingbats ? */  /* 94/Feb/3 */
/*        strcpy(fname, fontname[n]); */    /* experiment 93/Oct/7 */
//        strcpy(fname, fontname + n * MAXTEXNAME); /* 94/Feb/2 */
      if (fontname[n] != NULL) strcpy(fname, fontname[n]);  /* 94/Feb/2 */
      else *fname = '\0';
    }
/*    subfontname is MM *instance* FontName, not MM base */
    if ((fontproper[n] & C_NOTBASE) != 0) {
//      strcpy(fname, subfontname + n * MAXTEXNAME);  /* 97/Dec/4 */
      if (subfontname[n] != NULL) strcpy(fname, subfontname[n]);
      else *fname = '\0';
    }
  }

  if (fs[f] != 0) {     /* zero if font not used directly */
/*  if ((property & C_ALIASED) != 0)
    strcpy(fname, subfontname[f]); */
    if ((property & C_UNUSED) == 0) { /* if actually used */
      if (strcmp(fname, "") == 0) {
/* fprintf(errout, " ERROR: Empty name: fn %d f %d fontname %s subfontname %s\n", 
          fn, f, fontname[f], subfontname[f]); */
        sprintf(logline, " ERROR: Empty name: fn %d f %d\n",  fn, f);
        showline(logline, 1);
      }
/*      fprintf(fp_out, "/fn%d /%s %ld mf ", fn, fname, fs[f]); */
/*      fprintf(fp_out, "/fn%d /", fn); */ /* 1994/June/7 */
      sprintf(logline, "/fn%d /", bUseInternal ? fn : f);
      PSputs(logline, fp_out);
/*  Is this use of prefix always OK? What if we are not dealing with PS name */
/*      if (strcmp(fontprefix, "") != 0) fputs(fontprefix, fp_out); */
//      if (strcmp(fontprefix, "") != 0)      /* 95/July/5 */
      if (fontprefix != NULL) {
        if (fontreside(f) == 0) {
/*          possibly modify if bRandomPrefix is set ??? */
//          fputs(fontprefix, fp_out);
          PSputs(fontprefix, fp_out);
        }
      }
      if (uppercaseflag != 0 && is_tex_font(fname) != 0) 
        uppercase(fname, fname);        /* 92/Dec/22 */
/* This is where we insert  fname  after the /fn... */
/* #ifdef ALLOWSCALE
      if (outscaleflag)
        fprintf(fp_out, "%s %.9lg mf ", fname, fs[f] / outscale);
      else
  #endif */
      if (bSubRealName == 0) {    /* a hack to hack something */
/*      we don't normally come here these days ... */
/*      use real PS FontName --- unless new size of another font */
        if ((property & C_DEPENDENT) == 0) {
//          strcpy(fname, subfontname + f * MAXFONTNAME);
          if (subfontname[f] != NULL) strcpy(fname, subfontname[f]);
          else *fname = '\0';
        }
        else {
          n = fontsubflag[f];   /* what are we dependent on ? */
//          strcpy(fname, subfontname + n * MAXFONTNAME);
          if (subfontname[n] != NULL) strcpy(fname, subfontname[n]);
          else *fname = '\0';
        }
      }
      if (property & C_INSTANCE) {
/*        strcpy(fname, subfontname + f * MAXFONTNAME); */
        *fname = '\0';

//        strcat(fname, subfontname + f * MAXFONTNAME);
        if (subfontname[f] != NULL) strcat(fname, subfontname[f]);
#ifdef DEBUG
        if (traceflag) {
          sprintf(logline, "subfontname (%d): %s ", f, fname);
          showline(logline, 0);
        }
#endif
        if (traceflag) {      /* debugging */
//          if (logfileflag) showproper(logfile, fontproper[f]);
          showproper(logline, fontproper[f]);
          showline(logline, 0);
        }
/* not sure this is the best way for MM instances - mayby use subfontname */
      }
/* This is where we insert  fname  after the /fn... */
#ifdef ALLOWSCALE
      if (outscaleflag) {
        sprintf(logline, "%s %.9lg mf ", fname, (double) fs[f] /
            outscale);
      }
      else {
        sprintf(logline, "%s %ld mf ", fname, fs[f]);
      }
      PSputs(logline, fp_out);      
#else
      {
        sprintf(logline, "%s %ld mf ", fname, fs[f]);
        PSputs(logline, fp_out);
      }
#endif
/*      fputs(fname, fp_out); */
/*      fprintf(fp_out, " %ld mf ", fs[f]); */

      sprintf(logline, "/f%d{fn%d sf}def\n",
        bShortFont ? f : fn, bUseInternal ? fn : f); 
      PSputs(logline, fp_out);
/*      fprintf(fp_out, "/f%d{fn%d sf}def\n", fn, fn);*/ /* 1994/June/7 */
#ifdef DEBUG
/*      printf("fontname %s subfontname %s fontproper %0x\n",
        fontname[f], subfontname[f], property); */
#endif
    }
/*    else printf("Font %d noted as C_UNUSED\n", f); */ /* 95/Feb/1 */
  }
}

/* Replace PostScript FontName in PSS stub with new one */

int replacename (char *line, char *fname)
{
  char *s, *t;
  char buffer[FNAMELEN];

  if ((s = strchr(line, '/')) == NULL) return -1;
  s++;
  if ((t = strchr(s, ' ')) == NULL) t = strchr(s, '/');
  if (t == NULL) return -1;
  if (strlen(t) >= sizeof buffer)   /* FNAMELEN */
    return -1;
  strcpy(buffer, t);
  strcpy(s, fname);
  strcat(s, buffer);
  return 0;
}

/* copy and process Multiple Master Instance PSS stub file */
/* fname is file name of MM instance stub */
/* bname is file name of MM base font */
/* encode is optional encoding vector - no longer used */
/* returns -1 if file format bad or parsing error */

/* int copyPSS_sub(FILE *output, FILE *input, char *fname, char *encode) { */
/* int copyPSS_sub(FILE *output, FILE *input, char *fname, char *bname, char *encode) { */
int copyPSS_sub(FILE *output, FILE *input, char *fname, char *bname, char *encode, int proper)
{
/*  int c; */
  char *s;
//  char basename[MAXTEXNAME];  /*  char basename[16]; */
  char basename[FNAMELEN];  /*  char basename[16]; */

  if (getc(input) != 128) return -1;    /* check ASCII section */
  if (getc(input) != 1) return -1;    /* should be 128, 1 */
/*  skip over 4 byte length code */
  getc(input); getc(input); getc(input); getc(input);
  if (read_a_line(line, input) != 0) return -1;
  while (*line == '%') {          /* copy comment lines over */
//    fputs(line, output);
    PSputs(line, output);
    if (read_a_line(line, input) != 0) return -1;
  }
/*  while ((s = strchr(line, '/')) == NULL) {  */
  while ((s = strstr(line, "findfont")) == NULL) {
//    fputs(line, output);
    PSputs(line, output);
    if (read_a_line(line, input) != 0) return -1;
  }
  if ((s = strchr(line, '/')) == NULL) return -1;
/*  replace PostScript Multiple Master *Instance* FontName */
  if (replacename (s, fname) != 0) return -1;
/*  if (bMMNewFlag) { */
  if ((s = strchr(line+1, '/')) == NULL) return -1;
/*  Construct MM base font file name from stub PSS file name */
  if (bMMShortCut) {            /* disallow this ? */
    strcpy(basename, fname);
    strcpy(basename+5, "___");
/*    MM base font has same name, but ends in ___ simple minded ? */
  }
  else strcpy(basename, bname);
/*  Replace PostScript Multiple Master *Base* FontName */
/*  Should more generally check whether Base Font is resident ? */
/*  if (forcereside == 0) */        /* 95/Sep/16 */
  if ((proper & C_RESIDENT) == 0)     /* 95/Sep/16 later */
    if (replacename (s, basename) != 0) return -1;
/*  } */
  while ((s = strstr(line, "/FontName")) == NULL) {
//    fputs(line, output);
    PSputs(line, output);
    if (read_a_line(line, input) != 0) return -1;
  }
/*  if (replacename (s+9, fname) != 0) return -1; */
  if ((s = strchr(s+1, '/')) == NULL) return -1;
/*  replace PostScript Multiple Master *Instance* FontName */
  if (replacename (s, fname) != 0) return -1;
  while ((s = strstr(line, "definefont")) == NULL) {
//    fputs(line, output);
    PSputs(line, output);
    if (read_a_line(line, input) != 0) return -1;
  }
/*  if (bMMNewFlag == 0) {*/  /* Insert new encoding vector in PSS 95/May/13 */
/*  Should more generally check whether Base Font is resident ? */
/*  if (forcereside != 0) { */  /* new encoding vector in PSS 95/Sep/16 */
  if (proper & C_RESIDENT) {  /* new encoding vector in PSS 95/Sep/16 */
    if (encode != NULL && *encode != '\0') {
      sprintf(logline, "dup /Encoding %s put\n", encode);
      PSputs(logline, output);
    }
    else if (bWindowsFlag != 0) {
      sprintf(logline, "dup /Encoding get StandardEncoding eq{");
      PSputs(logline, output);
      sprintf(logline, "dup /Encoding %s put\n", textenconame);   
      PSputs(logline, output);
      PSputs("}if\n", output);
    }
  }
//  fputs(line, output);
  PSputs(line, output);
  return 0;
}

/* Danger above is that MM font that is not a text font can get reencoded! */
/* Fixed 96/Sep/29 */

/* use copyPSS_sub(output, input, fname, encode); to copy and expand PSS */

/* compressfont(output, input, FontName) to copy Multiple Master itself ? */ 
/* how to we get the FontName ??? */ /* needed for BeginResource ... */
/* what about those comments about FontNeeded, FontProvided etc ? */

/* work in progress: deal with PSS stub for Multiple Master */

int copyPSS(FILE *fp_out, FILE *fp_in, int k)
{
  int flag, m;
  int proper=0;
  char *s;
//  char vecname[MAXVECNAME];     /* 1994/Feb/4 */
  char vecname[FNAMELEN];
/*  char basename[MAXFONTNAME]; */    /* 1995/May/25 */
//  char basename[MAXTEXNAME];      /* 1997/June/1 */
  char basename[FNAMELEN];
//  char psfontname[MAXFONTNAME];   /* 1997/June/1 */
  char psfontname[FNAMELEN];

/*  strcpy(oldname, fontsubfrom + k * MAXTEXNAME); */
/*  strcpy(newname, fontsubto + k * MAXFONTNAME); */
//  strcpy(vecname, fontsubvec + k * MAXVECNAME);
  if (fontsubvec[k] != NULL) strcpy(vecname, fontsubvec[k]);
  else *vecname = '\0';
/*  file name of corresponding MM base font - old way and default */
//  strcpy(psfontname, subfontname + k * MAXFONTNAME);  /* 97/June/1 */
  if (subfontname[k] != NULL) strcpy(psfontname, subfontname[k]); /* 97/June/1 */
  else *psfontname = '\0';
  if ((s = strchr(psfontname, '_')) != NULL && s > psfontname) {
    return 0;   /* actual instance PS FontName - redundant */
  }
//  strcpy(basename, fontname + k * MAXTEXNAME);    /* 95/May/25 */
  if (fontname[k] != NULL) strcpy(basename, fontname[k]);   /* 95/May/25 */
  else *basename = '\0';
  strcpy(basename + 5, "___");            /* 95/May/25 */
  if (bMMShortCut == 0) {     /* find corresponding MM base entry */
    flag = 0;
    for (m = 0; m < fnext; m++) {
      if ((fontproper[m] & C_MULTIPLE) == 0) continue;
//      if (strcmp (subfontname + k * MAXFONTNAME,
//        subfontname + m * MAXFONTNAME) == 0) 
      if (subfontname[k] != NULL && subfontname[m] != NULL &&
          strcmp (subfontname[k], subfontname[m]) == 0) {
/* we assume that the actual MM font file name has been placed here already */
/* which will not be the case if the darn thing is declared resident ? */
        proper = fontproper[m];   /* pick up properties of base */
//        strcpy (basename, fontname + m * MAXTEXNAME);
        if (fontname[m] != NULL) strcpy (basename, fontname[m]);
        else *basename = '\0';
        flag = 1;
        break;
      }
    }
    if (flag == 0) {          /* should not happen - debugging */
      sprintf(logline, " Assuming MM base `%s'", basename);
      showline(logline, 1);
    }
  }

  begin_resource(fp_out, filefontname); /* %%BeginResource: font zjrg_001 */
/*  flag = copyPSS_sub(fp_out, fp_in, filefontname, vecname); */
  flag = copyPSS_sub(fp_out, fp_in, filefontname, basename, vecname, proper);
  if (flag != 0) {
    notvalidPSS(1);   /* pssflag == 1 */
    sprintf(logline, "`%s' ", filefontname);
    showline(logline, 0);
  }
  end_resource(fp_out);         /* %%EndResource */
  return flag;
}

int IsItUsed (char *wantchrs)
{
  int k;
  if (wantchrs == NULL) return 0;
/*  test in decreasing order of expected use */
//  for (k = 0; k < fontchrs; k++)  if (wantchrs[k] != 0) return -1;
  for (k = 64;  k < 128; k++) if (wantchrs[k]) return 1;  /* G1b */
  for (k = 32;  k < 64;  k++) if (wantchrs[k]) return 1;  /* G1a */
  for (k = 0;   k < 32;  k++) if (wantchrs[k]) return 1;  /* C1 */
  for (k = 160; k < fontchrs; k++) if (wantchrs[k]) return 1; /* G2 */
  for (k = 128; k < 160; k++) if (wantchrs[k]) return 1;  /* C2 */
  return 0;
}

/* See if can mark unused fonts (in jobs with less than full page range) */

int MarkUnusedFonts (void)        /* an experiment 95/Mar/5 */
{
  int fn, flag=0;
/*  int k, count, property; */
//  char fontnamek[MAXTEXNAME];
  char fontnamek[FNAMELEN];
  char *wantchrs;

/*  for (fn = 0; fn < mmbase; fn++) { */  /* ignore MM base fonts for now */
  for (fn = 0; fn < fnext; fn++) {    
/*    if (fontproper[k] & C_MULTIPLE) continue; */ /* ignore MM base fonts */
//    wantchrs = fontchar + MAXCHRS * fn;
    wantchrs = fontchar[fn];
/*    property = fontproper[fn]; */
/*    count = 0;
    for (k = 0; k < MAXCHRS; k++) {
      if (wantchrs[(k + 32) % 255] != 0) {
        count++;  break;
      }
    } */
/*    count = IsItUsed(wantchrs); */
/*    if (count == 0) { */
    if (! IsItUsed(wantchrs)) {
      if (traceflag) {
/*        strcpy (fontnamek, fontname + k * MAXTEXNAME);   */
//        strcpy (fontnamek, fontname + fn * MAXTEXNAME); 
        if (fontname[fn] != NULL) strcpy (fontnamek, fontname[fn]); 
        else *fontnamek = '\0';
        sprintf(logline, "Marking %s as unused\n", fontnamek);
        showline(logline, 0);
      }
      fontproper[fn] |= C_UNUSED;
      flag++;
    }
  }
  return flag;
}

/* following split off 95/May/13 to improve readability ... */

int extractafont (FILE *fp_out, int k)
{
  FILE *fp_in;
  int flag=0;
  int proper;         /* 1992/Aug/23 */
//  char fontnamek[MAXTEXNAME];
  char fontnamek[FNAMELEN];
//  char subfontnamek[MAXFONTNAME];
  char subfontnamek[FNAMELEN];

  proper = fontproper[k];
//  strcpy(fontnamek, fontname + k * MAXTEXNAME);   /* 94/Feb/2 */
  if (fontname[k] != NULL) strcpy(fontnamek, fontname[k]);    /* 94/Feb/2 */
  else *fontnamek = '\0';
/*  if ((proper & C_UNUSED) != 0) return;  */
  if (fontsubflag[k] < 0) {   /* not a font substituted for */
/*    don't bother if font not used - no characters */
    if ((proper & C_UNUSED) != 0) return 0; 
/*    don't bother if already known not to exist  */
    if ((proper & C_MISSING) != 0) return 0; 
    if ((proper & C_RESIDENT) != 0) return 0; /* never */
    if ((proper & C_DEPENDENT) != 0) return 0;  /* NEW */
/*    if (strcmp(fontname[k], subfontname[k]) == 0) continue; */ /* ? */
/*    if ((proper & C_ALIASED) != 0)  
        strcpy(filefontname, subfontname[k]); */
/*    strcpy(filefontname, fontname[k]); */
//    strcpy(filefontname, fontname + k * MAXTEXNAME);
    if (fontname[k] != NULL) strcpy(filefontname, fontname[k]);
    else *filefontname = '\0';
    if ((proper & C_REMAPIT) != 0) { /* remapped font */
//      strcpy (subfontnamek, subfontname + k * MAXFONTNAME);
      if (subfontname[k] != NULL) strcpy (subfontnamek, subfontname[k]);
      else *subfontnamek = '\0';
/*      printf("\n%s AND %s ", filefontname, subfontnamek); */
      if (proper & C_MULTIPLE) { /* kludge for remapped MM base 95/Jul/15 */
//        strcpy (subfontnamek, fontname + k * MAXFONTNAME);
        if (fontname[k] != NULL)
          strcpy (subfontnamek, fontname[k]);
        else *subfontnamek = '\0';
      }
/*      if ((fp_in = open_font(subfontname[k])) == NULL) {  */
      if ((fp_in = open_font(subfontnamek, 1)) == NULL) { 
/*        fprintf(errout, " WARNING: "); */ /* REDUNDANT ? */
        sprintf(logline, " WARNING: %s ", subfontnamek);  /* REDUNDANT ? */
        showline(logline, 1);
/*        perrormod(subfontname[k]);   */
        perrormod(subfontnamek);  
        errcount(0); 
        return 0;
      }
    }
    else { /* not a remapped font */
/* can we assume fontnamek set correctly ? */
/*      if ((fp_in = open_font(fontname[k])) == NULL) {  */
      if ((fp_in = open_font(fontnamek, 1)) == NULL) { 
/*        fprintf(errout, " WARNING: "); */ /* REDUNDANT ? */
        sprintf(logline, " WARNING: %s ", fontnamek); /* REDUNDANT ? */
        showline(logline, 1);
/*        perrormod(fontname[k]);  */
        perrormod(fontnamek); 
        errcount(0);
        return 0;
      } /* shouldn't happen ? */
    }
    if (verboseflag) {
//      putc('[', stdout);  
//      if (logfileflag) putc('[', logfile);  
      showline("[", 0);
    }
/*    else putc('*', stdout); */          /* 95/Mar/1 */

    if (proper & C_MULTIPLE) {
      mmflag = 1;
/*      (void) FindMMBaseFile(k);  */ /* done earlier already */
/*      strcpy (fontnamek, fontname + k * MAXTEXNAME);  */
    } 
    if (proper & C_INSTANCE) {    /* Is this MM Instance PSS stub ?*/
      if (verboseflag) showline(fontnamek, 0);    /* 95/May/13 */
      if ((proper & C_NOTBASE) != 0) 
        copyPSS (fp_out, fp_in, k);   /* 94/Dec/6 do later ? */
    }
    else if (type3flag != 0) {
/* How would type3flag ever get set here ? - By font open mechanism */
      if (extract_type3(fp_out, fp_in, k, fontnamek) == 0) {
        if (extract_type1(fp_out, fp_in, k, fontnamek) == 0)
          copy_unknown(fp_out, fp_in, k, fontnamek);
      }
    }
    else {
      if (extract_type1(fp_out, fp_in, k, fontnamek) == 0) {
        if(extract_type3(fp_out, fp_in, k, fontnamek) == 0)
          copy_unknown(fp_out, fp_in, k, fontnamek); 
      }
    }
    if (abortflag) return -1;
/*    extractfont(fp_out, fp_in, k); */
/*    nfonts++; */
    flag = 1;
    fclose (fp_in);
    if (verboseflag) showline("] ", 0);
  }   /* if (fontsubflag[k] < 0) */
  return flag;
}

/* main entry point - extract and decompress font files */
/* once it has been verified this works correctly, strip out IGNORE part */
/* present order is: MM base fonts, PSS stub files, other fonts ... */

int extractfonts (FILE *fp_out) /* was called by main */
{
  int k;
  int proper;         /* 1992/Aug/23 */
/*  int oldsuppress; */

/*  init_enc(); */  /* should be done earlier dvipsone.c */

  nfonts=0; nsubstitute=0; nremapped=0; nansified=0;
  if (fnext == 0) return 0;     /* nothing to do ??? */

/*  fprintf(fp_out, "dvidict begin\n"); */
  write_dvi_start(fp_out);        /* 93/Sep/30 */
/*  if (bMarkUnused) MarkUnusedFonts(); */    /* 95/March/5 */
  if (showfontflag) {
    show_font_table();  /* an experiment */
//    if (logfileflag) show_font_table(logfile);
  }

  task = "decompressing font files";

/*  some redundancy with code below */ /* download MM base fonts first */
/*  if (fnext > mmbase && forcereside == 0) { */
  if (mmcount > 0 && forcereside == 0) {    /* ??? */
    mmflag = 1;               /* flag that these are MM */
/*    oldsuppress = bSuppressPartial; */
/*    if (bMMNewFlag == 0) bSuppressPartial = 1; *//* backward compat */
/*    for (k = mmbase; k < fnext; k++) { */   /* do MM fonts first */
    for (k = 0; k < fnext; k++) {       /* do MM fonts first */
      proper = fontproper[k];
      if ((proper & C_MULTIPLE) == 0) continue;
      if (extractafont(fp_out, k) != 0) nfonts++;
      if (bAbort) abortjob();         /* 1992/Nov/24 */
      if (abortflag) return -1;
    }               /* end of for loop over MM fonts */
/*    if (bMMNewFlag == 0) bSuppressPartial = oldsuppress;  */
    mmflag = 0;
  }
  
/*  extract PSS stub files next */

  for (k = 0; k < fnext; k++)
  {
    proper = fontproper[k];
    if ((proper & C_INSTANCE) == 0) continue;
    if (extractafont(fp_out, k) != 0) nfonts++; /* do we count these ? */
    if (bAbort) abortjob();         /* 1992/Nov/24 */
    if (abortflag) return -1;
  }

/*  finally do non MM fonts ... */    /* MM base fonts already done above */

/*  for (k = 0; k < mmbase; k++) { */
  for (k = 0; k < fnext; k++) {   /* 1994/Dec/6 */
    proper = fontproper[k];
    if (proper & C_MULTIPLE) continue;  /* MM base fonts already done */
    if (proper & C_INSTANCE) continue;  /* MM instances already done */
    if (extractafont(fp_out, k) != 0) nfonts++;
    if (bAbort) abortjob();         /* 1992/Nov/24 */
    if (abortflag) return -1;
  }     /* for (k=0; k < fnext; k++) */
  if (verboseflag) showline("\n", 0);

  write_dvi_end(fp_out);        /* 1992/Nov/17 */

  if (verboseflag) {        /* rewrote 1993/Feb/6 */
    if (nfonts > 0) 
/*      printf("Processed %d font files ", nfonts);  */
      sprintf(logline, "Processed %d font file%s ", nfonts,
        (nfonts == 1) ? "" : "s");    /* 1994/Feb/1 */
    else sprintf(logline, "No font files ");
    showline(logline, 0);
  }
  return 0;
}

/* Note: fstart / fend can occur twice (% Font Remap and % Font Defs) */
/* so checking for existing fonts may list only those remapped, miss others */
/* Because `fend' terminates when it has found one or more bad fonts */
    
/* separated out from the above 94/Mar/3 ************************** */

void fontsetup (FILE *fp_out)
{
  int k, i, ne, f;
/*  int n; */
  int count, flag;
  int property;           /* 1992/Aug/23 */
//  char fname[MAXFONTNAME];    /* 1993/Feb/15 */
  char fname[FNAMELEN];
//  char fontnamef[MAXTEXNAME];
  char fontnamef[FNAMELEN];
//  char fontnamek[MAXTEXNAME];
  char fontnamek[FNAMELEN];
//  char subfontnamek[MAXFONTNAME];
  char subfontnamek[FNAMELEN];
  static long widths[MAXCHRS];  /* why is this static ? */
/*  long widths[MAXCHRS]; */    /* problem with stack space ? */
  char *fontptr;

/*  now for the substitutions */
  if (substituteflag && need_substitute() != 0) {   
    task = "constructing substituted fonts";
    nsubstitute = 0;
    if (stripcomment == 0) {
//      fputs("% Font Subs:\n", fp_out);
      PSputs("% Font Subs:\n", fp_out);
    }
/*    fprintf(fp_out, "%s", "dvidict begin\n"); */    /* new */
/*    fputs("dvidict begin\n", fp_out); */  /* 1992/July/18 */
    write_dvi_start(fp_out);        /* 93/Sep/30 */
    for (k = 0; k < fnext; k++) {
      property = fontproper[k];
//      strcpy (fontnamek, fontname + k * MAXTEXNAME);  /* 1994/Feb/2 */
      if (fontname[k] != NULL) strcpy (fontnamek, fontname[k]); /* 1994/Feb/2 */
      else *fontnamek = '\0';
//      strcpy (subfontnamek, subfontname + k * MAXFONTNAME); /* 1997/June/1 */
      if (subfontname[k] != NULL) strcpy (subfontnamek, subfontname[k]);  /* 1997/June/1 */
      else *subfontnamek = '\0';
      if ((property & C_REMAPIT) != 0) continue;    /* ? */
      if ((property & C_RESIDENT) != 0) continue;   /* ? */
      if ((property & C_DEPENDENT) != 0) continue;    /* NEW */
      if ((property & C_UNUSED) != 0) continue;     /* 95/Sep/9 ??? */
/*      if (strcmp(fontname[k], subfontname[k]) == 0) continue; */ /* rep */
      if ((f = fontsubflag[k]) >= 0) { 
//        strcpy (fontnamef, fontname + f * MAXTEXNAME); /* 94/Feb/2 */
        if (fontname[f] != NULL) strcpy (fontnamef, fontname[f]); /* 94/Feb/2 */
        else *fontnamef = '\0';
/*        fprintf(output, "%% FONTSUBFLAG %d\n",
          fontsubflag[k]); */  /* debugging */
/*        f = fontsubflag[k]; */
/*        following moved out here 1995/Sep/9 so we can check usage */
/*        fontptr = fontchar + MAXCHRS * k;
        count = 0;
        ne = MAXCHRS;
        for (i= 0; i < ne; i++) if (fontptr[i] != 0) count++;
        if (count == 0) {
          fontproper[k] |= C_UNUSED;
          continue;   
        } */ /* should not happen ! */
/*        above moved out here 1995/Sep/9 so we can check usage */

        if (stripcomment == 0) {
/*          fprintf(fp_out, "%%%%BeginFont: %s\n", fontname[k]); */
/*          fprintf(fp_out, "%% BeginFont: %s\n", fontname[k]);  */
          if (property & C_MULTIPLE) {        /* 97/June/1 */
            sprintf(logline, "%% BeginFont: %s\n", subfontnamek);
          }
          else {
            sprintf(logline, "%% BeginFont: %s\n", fontnamek);
          }
          PSputs(logline, fp_out);
        }
/*        fprintf(fp_out, "/%s fs ", fontname[f]); */
//        putc('/', fp_out);
        PSputc('/', fp_out);
//        if (strcmp(fontprefix, "") != 0)    /* 1995/July/5 */
        if (fontprefix != NULL) {   /* 1995/July/5 */
          if (fontreside(f) == 0) {
/*  possibly modify if bRandomPrefix is set ??? */
//            fputs(fontprefix, fp_out);
            PSputs(fontprefix, fp_out);
          }
        }
/* shouldn't this depend on whether it is a texfont or not ??? */
/*        if (uppercaseflag) */           /* 1995/July/1 */
        if (uppercaseflag && is_tex_font(fontnamef))  /* 1996/June/20 */
          uppercase(fontnamef, fontnamef);
/*        fprintf(fp_out, "/%s fs ", fontnamef); */
//        fputs(fontnamef, fp_out);       /* 1995/July/5 */
        PSputs(fontnamef, fp_out);        /* 1995/July/5 */
//        fputs(" fs ", fp_out);
        PSputs(" fs ", fp_out);
/*  insert new font metrics - (and possibly new uniqueID ?) */
        if (insertmetrics != 0 && (property & C_MTMI) == 0) {
/*          if ((ne = read_widths(fontname[k], widths)) != 0) { */
          if ((ne = read_widths(fontnamek, widths)) != 0) {
/*            fontptr = fontchar[k]; */
/*            following moved up above to avoid zero count fonts */
//            fontptr = fontchar + MAXCHRS * k;
            fontptr = fontchar[k];
            count = 0;
            for (i= 0; i < ne; i++) if (fontptr[i] != 0) count++;
            if (count == 0) 
              showline(" zero count", 1); /* 95/Sep/9 */

            sprintf(logline, "%d fa\n", count);
            PSputs(logline, fp_out);
            for (i = 0; i < ne; i++) {
              if (fontptr[i] != 0) {
#ifdef ALLOWSCALE
/* following flushed 95/June/6 because fm PS proc *not* scaled */
/* character width in design size units * 2^20 */
/*                if (outscaleflag) 
                  fprintf(fp_out, "%d %.9lg fm\n",
                    i, widths[i] / outscale);
                else */
#endif
                sprintf(logline, "%d %ld fm\n", i, widths[i]);
                PSputs(logline, fp_out);
              }
            }
            PSputs("fb ", fp_out);
          }
        }
/*        fprintf(fp_out, "/%s fe\n", fontname[k]); */
//        putc('/', fp_out);  
        PSputc('/', fp_out);  
//        if (strcmp(fontprefix, "") != 0) 
        if (fontprefix != NULL) {
/*          if ((fontproper[k] & C_RESIDENT) == 0) */ /* NO ? */
/*  possibly modify if bRandomPrefix is set ??? */
//            fputs(fontprefix, fp_out);    /* 1995/July/5 */
          PSputs(fontprefix, fp_out);   /* 1995/July/5 */
        }
/* shouldn't this depend on whether it is a texfont or not ??? */
/*        if (uppercaseflag) */           /* 1995/July/1 */
        if (uppercaseflag && is_tex_font(fontnamek))  /* 1996/June/20 */
          uppercase(fontnamek, fontnamek);          
//        fputs(fontnamek, fp_out);
        PSputs(fontnamek, fp_out);
//        fputs(" fe\n", fp_out);
        PSputs(" fe\n", fp_out);
        if (stripcomment == 0) 
/*          fprintf(fp_out, "%%%%EndFont\n");  */
/*          fprintf(fp_out, "%% EndFont\n");  */
//          fputs("% EndFont\n", fp_out); 
          PSputs("% EndFont\n", fp_out); 
        nsubstitute++;
      }
    }

    write_dvi_end(fp_out);        /* 1992/Nov/17 */

    if (verboseflag && nsubstitute != 0) {
/*      printf("Substituted for %d fonts\n", nsubstitute); */
/*      printf("- substituted for %d fonts ", nsubstitute); */
      sprintf(logline, "- substituted for %d font%s ",
         nsubstitute, (nsubstitute == 1) ? "" : "s"); /* 95/July/15 */
      showline(logline, 0);
    }
  }
    
/*  if (need_remap() != 0) {  */
  if ((flag = need_remap()) != 0) { 
/*    constructvectors(fp_out); */
    if (flag < 0) constructvectors(fp_out);
    task = "constructing remapped fonts";
    nremapped = 0;
    if (stripcomment == 0) {
//      fputs("% Font Remap:\n", fp_out);
      PSputs("% Font Remap:\n", fp_out);
    }
/*    fprintf(fp_out, "%s", "dvidict begin\n"); */    /* new */
/*    fputs("dvidict begin\n", fp_out); */  /* 1992/July/18 */
    write_dvi_start(fp_out);          /* 93/Sep/30 */
//    fputs("fstart\n", fp_out);
    PSputs("fstart\n", fp_out);
    for (k = 0; k < fnext; k++) {
      property = fontproper[k];
//      strcpy (fontnamek, fontname + k * MAXTEXNAME);  /* 94/Feb/2 */
      if (fontname[k] != NULL) strcpy (fontnamek, fontname[k]); /* 94/Feb/2 */
      else *fontnamek = '\0';
//      strcpy (subfontnamek, subfontname + k * MAXFONTNAME); /* 97/June/1 */
      if (subfontname[k] != NULL) strcpy (subfontnamek, subfontname[k]);  /* 97/June/1 */
      else *subfontnamek = '\0';
/*      printf("k %d proper ", k); */       /* debugging */
      if ((property & C_DEPENDENT) != 0) continue;
      if ((property & C_UNUSED) != 0) continue;   /* 1995/Mar/27 */
/*      if (strcmp(fontname[k], subfontname[k]) == 0) continue; */ /* ? */
/*      if ((property & C_REMAPIT) != 0 &&
        (property & C_RESIDENT) != 0) { */
/* Shouldn't really bother doing this for MM Base Font if it is resident */
/* Since the PSS stub then refers to PS FontName, not this new name ... */
/*      only do this for printer resident fonts */
/*      if ((property & C_RESIDENT) == 0) continue; */
/*      only do this for printer resident fonts and MM base fonts */
      if ((property & C_RESIDENT) == 0 && (property & C_MULTIPLE) == 0) continue;
/*      only do this if font needs to be remapped */
      if ((property & C_REMAPIT) == 0 && bWindowsFlag == 0) continue;
/*      if ((property & C_RESIDENT) != 0 &&
        ((property & C_REMAPIT) != 0 ||
          bWindowsFlag != 0)) { */
        if (stripcomment == 0) {
          if (property & C_MULTIPLE) {        /* 97/June/1 */
            sprintf(logline, "%% BeginFont: %s\n", subfontnamek);
          }
          else {
            sprintf(logline, "%% BeginFont: %s\n", fontnamek);
          }
          PSputs(logline, fp_out);
        }
//        putc('/', fp_out);  
        PSputc('/', fp_out);
/*        if (strcmp(fontprefix, "") != 0) fputs(fontprefix, fp_out); */
//        if (strcmp(fontprefix, "") != 0)    /* 1995/July/5 */
        if (fontprefix != NULL) {   /* 1995/July/5 */
          if (fontreside(k) == 0) {
/*            possibly modify if bRandomPrefix is set ??? */
//            fputs(fontprefix, fp_out);
            PSputs(fontprefix, fp_out);
          }
        }
/*        strcpy(fname, fontname + k * MAXTEXNAME); */  /* 1994/Feb/2 */
        if (property & C_MULTIPLE) {      /* 97/June/1 */
          *fname = '\0';

//          strcat(fname, subfontname + k * MAXFONTNAME);
          if (subfontname[k] != NULL) strcat(fname, subfontname[k]);
        }
        else {
//          strcpy(fname, fontname + k * MAXTEXNAME); /* 1994/Feb/2 */
          if (fontname[k] != NULL) strcpy(fname, fontname[k]);  /* 1994/Feb/2 */
          else *fname = '\0';
        }
        if (uppercaseflag != 0 && is_tex_font(fname) != 0) 
          uppercase(fname, fname);
/*        fprintf(fp_out, "/%s %s /%s rmf\n",  
          fontname[k], fontvector[k], subfontname[k]); */
//        strcpy (subfontnamek, subfontname + k * MAXFONTNAME);/* 94/Feb/2 */
        if (subfontname[k] != NULL) strcpy (subfontnamek, subfontname[k]);/* 94/Feb/2 */
        else *subfontnamek = '\0';
        if ((property & C_REMAPIT) != 0) {
          sprintf(logline, "%s %s /%s rmf\n",
/*            fname, fontvector[k], subfontname[k]); */
//            fname, fontvector + k * MAXVECNAME, subfontnamek);
            fname, fontvector[k], subfontnamek);
        }
        else {          /* just due to Windows ANSI remap */
          sprintf(logline, "%s %s /%s amf\n",
/*            fname, "ansinew", subfontname[k]); */ /* 93/Oct/4 */
/*            fname, "ansinew", subfontnamek); */ /* 94/Feb/2 */
/*            fname, textencoding, subfontnamek); */  /* 94/Dec/17 */
            fname, textenconame, subfontnamek);   /* 95/Feb/6 */
        }
        PSputs(logline, fp_out);
/* copy font directory and insert new encoding vector */
        if (stripcomment == 0) {
/*          fprintf(fp_out, "%%%%EndFont\n");  */
/*          fprintf(fp_out, "%% EndFont\n"); */
//          fputs("% EndFont\n", fp_out);
          PSputs("% EndFont\n", fp_out);
        }
        nremapped++; 
/*      } */ /* end of resident and remapped */
    } /* end of for loop stepping through fonts */

//    fputs("fend\n", fp_out);
    PSputs("fend\n", fp_out);
    write_dvi_end(fp_out);        /* 1992/Nov/17 */
  } /* end of need remapped */

/*  NOTE: this only counts the resident fonts that get remapped ? */
/*  if (verboseflag && nremapped != 0)
      printf("- remapped %d fonts ", nremapped); */
/*  n = nremapped + nansified; */
/*  if (verboseflag && n != 0) */
  if (verboseflag) {
    if (nremapped > 0 || nansified > 0) {
      if (nremapped > 0) {
        sprintf(logline, " - remapped %d font%s", nremapped,
          (nremapped == 1) ? "" : "s");
        showline(logline, 0);
      }
      if (nansified) {
        sprintf(logline, " - remapped %d font%s to `%s' encoding", nansified,
          (nansified == 1) ? "" : "s", textencoding);
        showline(logline, 0);
      }
/*      putc('.', stdout); */
/*      putc('\n', stdout); */
      showline(".\n", 0);
    }
/* suppress the above if remapped font is not resident ? */
  }
/* now it is time to actually do the setfonts ! */

  if (stripcomment == 0) {
/*    fputs("%%BeginSetup\n, fp_out); */ /* ??? */
//    fputs("% Font Defs\n", fp_out);
    PSputs("% Font Defs\n", fp_out);
  }
  
/*  fprintf(fp_out, "dvidict begin\n"); */
  write_dvi_start(fp_out);        /* 93/Sep/30 */
//  fprintf(fp_out, "fstart\n");    /* 93/Nov/2 */
  PSputs("fstart\n", fp_out);
/*  if (bShortFont != 0) { */
  if (bShortFont != 0 && bUseInternal == 0) {   /* 1994/June/7 */
    for (f = 0; f < fnext; f++) dofont(fp_out, f, f);
  }
/*  the first chunck of old code above came from here ... */
  else { /* not using short font numbers */  
    for (k=0; k < MAXFONTNUMBERS; k++) 
/*      if ((f = finx[k]) >= 0) dofont(fp_out, f, k); */ /* 93/Dec/11 */
      if ((f = finx[k]) != BLANKFONT) dofont(fp_out, f, k);
  }
  
/* the second chunck of old code above came from here ... */
//  fprintf(fp_out, "fend\n");      /* 93/Nov/2 */
  PSputs("fend\n", fp_out);

  write_dvi_end(fp_out);        /* 1992/Nov/17 */

/*  fputs("%%EndSetup\n", fp_out); */ /* ??? */
}

/* Following called from dvispeci.c */
/* Don't expand %%IncludeFont for printer resident fonts */
/* Use a fixed list ??? */
/* tir, tii, tib, tibi, hv, hvo, hvb, hvbo, com, coo, cob, cobo, sy, zd */

int ResidentFont (char *FileName)       /* 1994/Feb/10 */
{
  char *s, *t;
  int k;

#ifdef DEBUG
  if (traceflag)
  {
    sprintf(logline, "ResidentFont %s?\n", FileName); /* debugging 97/June/5 */
    showline(logline, 0);
  }
#endif
/*  extract just font file name - get rid of path */
/*  if ((s = strrchr(FileName, '\\')) != NULL) s++;
  else if ((s = strrchr(FileName, '/')) != NULL) s++;
  else if ((s = strrchr(FileName, ':')) != NULL) s++;
  else s = FileName; */
  s = removepath(FileName);           /* 95/May/28 */
/*  get rid of extension */
  if ((t = strrchr(s, '.')) != NULL) *t ='\0';
  t = s + strlen(s) - 1;
  while (*t == '_') *t-- = '\0';          /* strip underscores */
#ifdef DEBUG
  if (traceflag) {
    sprintf(logline, "Looking for font file: `%s'\n", s);/* debugging 97/June/5 */
    showline(logline, 0);
  }
#endif
  for (k = 0; k < ksubst; k++) {
    if (fontsubprop[k] & C_RESIDENT) {
/*      if (strcmp (s, fontsubfrom + k * MAXTEXNAME) == 0) */
//      if (stricmp (s, fontsubfrom + k * MAXTEXNAME) == 0)
      if (fontsubfrom[k] != NULL &&
          stricmp (s, fontsubfrom[k]) == 0)
        return 1; /* is resident in table */
    }
  }
  return 0;           /* not in table as resident font */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* should earlier BeginSetup and EndSetup be moved to after loading fonts ? */

/* should tokenize input ? - remove dependency on lines */
/* definitely ignoring synthetic and hybrid fonts ? */
/* change font dictionary allocation ? first line after end of FontInfo OK */
/* abbreviate dup and put ? */
/* flush space before ND or | - NOT SAFE ?, make selectable ? */
/* flush \r both before encryption and after - make selectable ? */

/* extract actual FontName from font directory OK, not used */
/* replace actual FontName with what was passed along from dvianal OK */
/* make up copies of fonts with different metrics, if substituting OK */
/* need to read tfm files for this OK */

/* need to check EOF in calls to getline */

/* RD could be -| and ND could be | OK */
/* check on how this treats Subrs and OtherSubrs !!! */

/* do the right thing with StandardEncoding ? */
/* will this every be used with fonts other than TeX fonts ? */
/* distinguish TeX fonts ? */

/* possibly copy over that stuff from fontone.c */
/* for constructing Encoding from StandardEncoding ? efficiency */
/* break up big functions to allow global optimizations */

/* make default directory c:\psfonts instead of c:\cm ? */

/* allow for cmr10___.pfb format of file name OK */

/* make deletion of input files default - controlled by command line flag */

/* decrypt, remove first four bytes of each CharString reencrypt lenIV NO */
/* allow for removal of four leading bytes in CharStrings ? NO */

/* make flushing of FontInfo optional ... */

/* give warning if ASCII section has wrong length ? */

/* send control D if output direct to printer, or if requested.  OK */

/* stick in %!PS-Adobe-2.0 etc etc at beginning ? OK */

/* in case only page interval output is selected, slight weirdness: */
/* will set up fonts that are not used and hence not loaded */
/* but no error results, Courier is substituted, but no one knows */
/* since font never actually used ... */ /* to be fixed in merge OK ? */

/* may need facility to generate new uniqueIDs */
/* better to leave uniqueID off copy of font ? YES */
/* helpful generating new uniqueID ? NO */

/* distinguish force substitution from substitution when file not found */
/* invoke substitution when file not found ? OK */

/* some time break up "extractfont" and "extract" */

/* use table for dehexing hex and for creating hex ? */

/* right now there is no check whether font substituted exists ! */
/* if it is not found try using table again ? loops ? */

/* specify encoding vector in substitution file */

/* problems remain with remapping resident fonts ... */

/* deal with *force* in substitution table OK */
/* deal with *remap* in substitution table OK */

/* maybe check whether font file already was .pfb before trying that ext OK */

/* search for *//* make parallel to above ! */

/* worry about possible problems from assumed font ending - breakearly ? */
/* breakearly != 0 may be too risky - don't know what weird things happen */

/* should tokenize input ? - remove dependency on lines */

/* what about .pfb files that have return as line terminator ? OK */
/* need to fix getline and getrealline to deal with this */

/* need to treat /Subr strings specially */
/* particularly since they may be long */

/* output font file name in [..] always - even for remapped fonts ? */

/* major space user is charnames[MAXCHRS][MAXCHARNAME] = 8192 bytes */
/* OK, cut in half by using pointers into namestring instead */

/* check if font is really needed before bothering to extract it OK */

/* `Substituting' Times-Roman for psmtr - isn't quite appropriate phrasing */

/* check that  long checksum = 0x424B5048; - add brownie points ? */

/* should *reside* imply *force* ? */ /* no, use *force* explicitely */
/* should *remap* imply force ? */

/* maybe remove default font substitution table ? - always read it in ? */

/* don't reread font substitution table for each DVI file processed ? */
/* NO, have to, since it may be on DVI file path ! */

/* simplify all that hair for making up encoding vectors ... */

/* don't use tex_text vec if output is to be previewed ? use flag to control */

/* assuming it is either RD or -| and ND or |- */

/* do we need to cripple UniqueID when reencoding and using dviencoding ? */

/* command line flag 'k' not tested (forces printer resident for all) */

/* Compound Fonts are `Oblique' and `Narrow' fonts.  Examples: */
/* Courier-Oblique, Courier-BoldOblique */
/* Helvetica-Oblique, Helvetica-BoldOblique */
/* Helvetica-Narrow, Helvetica-Narrow-Bold */
/* Helvetica-Narrow-Oblique, Helvetica-Narrow-BoldOblique */
/* Typically these have Fontmatrix m11 != m22 or m21 != 0 */

/* What's the problem with synthetic fonts ? Why be paranoid ? */
/* 1. They may pick up base font that is partial (but FontName changed?) */ 
/* 2. They may create a font that is partial (but has original name) */
/* 3. There may be a problem if font is remapped ? */
/*    but only if the base font is picked up by some other font ... */

/* Some interpreters fail on tex_text encoding, cause its not 256 element */
/* Some interpreters fail on shortened encoding, cause its not 256 element */
/* examples: ALW II NTJ */

/* complain if `hires' so-called hybrid font ??? */
/* ask user to use *synthetic* flag */ /* Not any more, hybrid font fixed */

/* problems with fonts that have wrappings mostly fixed */
/* can handle wrapped synthetic fonts now */
/* may be able to handle wrapped PFA fonts also */

/* Typical wrapping format: */

/* FontDirectory/Symbol known{/Symbol findfont dup/UniqueID known{dup */
/* /UniqueID get 5016107 eq exch/FontType get 1 eq and}{pop false}ifelse */
/* {save true}{false}ifelse}{false}ifelse */

/* cleartomark */
/* {restore}if */

/* cleartomark{restore}if */

/* Note: In DOS, TFM file names are not case sensitive in TeX */
/* however, here in dealing with font substitution file, things ARE */
/* So, its possible to get confused because a font is found by TeX */
/* but not matched in substitution table */

/* Note `k' turns on *force* and *reside* which may not be desirable for */
/* lines like `cmr8 cmr7' which should only trigger if cmr8 is not found */
/* However, the new scheme for getting FontNames from PFB files solves */
/* this problem ... */

/* MM font support incomplete */
/* presently downloads full font - need to provide for partial font */
/* collect wantchrs contributions from MM instances for base */
/* remap / encoding not checked out */

/* if forcereside on, we should still download/expand the PSS stubs ... */

/* extract_type3, does fontname get printed ? */
/* mac style font, does fontname get printed ? */
/* what is font can't be found does font name get printed ? */
/* what is some error in extraction does font name get printed ? */
/* repeated output of font name if switch between type3 and type1 */

/* Presently we reencode the PSS stub files if needed */
/* We do not reencode the MM master file */
/* Which means we can't do selected down loading quite yet */
/* Also seems like we are reencoding to ENCODING the PSS stub */
/* without knowing whether the base font is a text font! */

/*  if we want to use partial font downloading, then need to reencode */
/*  and then we have to assume that all instances encoded the same ? */
/*  we also then need to set up wantchrs[] as or of instances ... */
