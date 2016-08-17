/* Copyright 1994,1995,1996,1997,1998,1999 Y&Y, Inc.
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


/****************************************************************************/
/* Make PS code from TIFF image file 1, 2, 4, 8 bit pixels only */
/* Support palette color images => expands to RGB */
/* Support 24 bit color images converts RBG to gray if colorimage not avail */
/* Supports: input side - no compression, TIFF_CCITT, LZW, and PACK_BITS */
/* Does not support BitsPerSample other than power of 2 */
/* Does not support compression schemes 3 and 4 (CCITT) */
/* Supports: output side - no compression, ASCII85, LZW, and PACK_BITS */
/****************************************************************************/

/* Also, code for some other DVIWindo \specials */

#include "dvipsone.h"
/* START */
/* This is mostly stuff for our own TIFF reading code in winspeci.c */

/* The magic TIFF `version' number */

#define TIFF_VERSION 42

/* TIFF file value type codes: */
#define TYPE_BYTE       1
#define TYPE_ASCII      2
#define TYPE_SHORT      3
#define TYPE_LONG       4
#define TYPE_RATIO      5
/* following added in TIFF 6.0 */
#define TYPE_SBYTE      6  /* An 8-bit signed (twos-complement) integer */
#define TYPE_UNDEFINED  7  /* An 8-bit byte that may contain anything */
#define TYPE_SSHORT     8  /* A 16-bit (2-byte) signed (twos-complement) integer */
#define TYPE_SLONG      9  /* A 32-bit (4-byte) signed (twos-complement) integer */
#define TYPE_SRATIONAL  10 /* Two SLONG’s: numerator and denominator */
#define TYPE_FLOAT      11 /* Single precision (4-byte) IEEE format */
#define TYPE_DOUBLE     12 /* Double precision (8-byte) IEEE format */

/* TIFF file tags */

#define NEWSUBFILETYPE            254
#define SUBFILETYPE               255
#define IMAGEWIDTH                256
#define IMAGELENGTH               257
#define BITSPERSAMPLE             258
#define COMPRESSION               259
#define PHOTOMETRICINTERPRETATION 262
#define THRESHHOLDING             263
#define CELLWIDTH                 264
#define CELLLENGTH                265
#define FILLORDER                 266
#define DOCUMENTNAME              269
#define IMAGEDESCRIPTION          270
#define MAKE                      271
#define MODEL                     272
#define STRIPOFFSETS              273
#define ORIENTATION               274
#define SAMPLESPERPIXEL           277
#define ROWSPERSTRIP              278
#define STRIPBYTECOUNTS           279
#define MINSAMPLEVALUE            280
#define MAXSAMPLEVALUE            281
#define XRESOLUTION               282
#define YRESOLUTION               283
#define PLANARCONFIG              284
#define PAGENAME                  285
#define XPOSITION                 286
#define YPOSITION                 287
#define FREEOFFSETS               288
#define FREEBYTECOUNTS            289
#define GRAYRESPONSEUNIT          290
#define GRAYRESPONSECURVE         291
#define GROUP3OPTIONS             292
#define GROUP4OPTIONS             293
#define RESOLUTIONUNIT            296
#define PAGENUMBER                297
#define COLORRESPONSECURVES       301
#define SOFTWARE                  305
#define DATETIME                  306
#define ARTIST                    315
#define HOSTCOMPUTER              316
#define PREDICTOR                 317
#define WHITE                     318
#define COLORLIST                 319
#define PRIMARYCHROMATICITIES     319
#define COLORMAP                  320
/* following added in TIFF 6.0 */
#define HALFTONEHINTS             321
#define TILEWIDTH                 322
#define TILELENGTH                323
#define TILEOFFSETS               324
#define TILEBYTECOUNTS            325
#define INKSETS                   332
#define INKNAMES                  333
#define NUMBEROFINKS              334
#define DOTRANGE                  336
#define TARGETPRINTER             337
#define EXTRASAMPLES              338
#define SMINSAMPLEVALUE           340
#define SMAXSAMPLEVALUE           341
#define TRANSFERRANGE             342

#define JPEGPROC                    512
#define JPEGINTERCHANGEFORMAT       513
#define JPEGINTERCHANGEFORMATLNGTH  514
#define JPEGRESTARTINTERVAL         515
#define JPEGLOSSLESSPREDICTORS      517
#define JPEGPOINTTRANSFORMS         518
#define JPEGQTABLES                 519
#define JPEGDCTABLES                520
#define JPEGACTABLES                521
#define YCBCRCOEFFICIENTS           529
#define YCBCRSUBSAMPLING            530
#define YCBCRPOSITIONING            531
#define REFERENCEBLACK              532

#define COPYRIGHT     33432

#define NOCOMPRESSION     1
#define TIFF_CCITT        2
#define CCITT_GROUP3      3
#define CCITT_GROUP4      4
#define LZW_COMPRESSION   5
#define JPEG_COMPRESSION  6
#define PACK_BITS         32773
/* END   */

#define DEBUGTIFF         /* to get in extra tracing code */

/* #define DEBUGBMP */

/* #define DEBUGLZWENCODE */

/* #define DEBUGRUNLENGTH */

/* #define DEBUGCLEANUP */

#define PSLEVEL2

#define LZWCOMPRESSION

#define MAXCOLUMNHEX 72

#define MAXCOLUMNASCII 75

#ifdef TIFF

/* constants for LZW compression/decompression tables */

#define MAXCODES  4096 /* to deal with up to 12 bit codes */
#define MAXCHR    256  /* 2^8 possible bytes */
#define CLEAR     256  /* clear string table code */
#define EOD       257  /* end of data code */
#define FIRSTCODE 258  /* first code available - new string */

#ifdef PSLEVEL2
  int bRunLengthFlag;   /* Use RunLengthDecode filter 96/Dec/24 */
  #ifdef LZWCOMPRESSION
  int bLZWFlag;         /* Use LZWDecode filter 96/Dec/30 */
  #endif
/*  can have either of the above set, or none, but not both */
#endif

unsigned char *StripData;  /* address of Strip Buffer */

unsigned int StripDataLen; /* length of Strip Buffer */

long InStripLen;        /* length of Strip in file 96/Nov/17 */

typedef int BOOL;

char infilename[FILENAME_MAX] = "";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* using definitions in dvispeci.h */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define MAXIMAGE 4096 */   /* maximum rows & columns - sanity check */

#define MAXIMAGE 65536    /* maximum rows & columns - sanity check */

BOOL bInvertImage   = 0;  /* should gray levels be inverted ? */
BOOL bColorImage    = 0;  /* is it a color image ? */
                          /* i.e. RGB or color palette */
BOOL bExpandColor   = 0;  /* color palette image - expand to 24 bit */
                          /* set when ColorMapPtr non-NULL */
BOOL bCompressColor = 0;  /* compress 24 bit color to one byte */
                          /* for non-color PS output device ? */
                          /* and for BMP with gray palette */
BOOL bExpandGray    = 0;  /* has Palette, but Palette is gray */
BOOL bGrayFlag      = 0;  /* non-zero if Palette is gray BMP palette */
BOOL bLinearFlag    = 0;  /* non-zero if palette is linear BMP palette */
int PaletteSize     = 0;  /* number of entries in palette */

/* For Windows NT could declare USESHORTINT since 16 bit quantities */

/* #define USESHORTINT */

#ifdef USESHORTINT
unsigned short int *Palette = NULL; /* pointer to palette */
#else
unsigned int *Palette = NULL;       /* pointer to palette */
#endif

/* char *eps_magic = "\
currentscreen pop\n\
{1 add 180 mul cos 1 0.08 add mul exch 2 add 180 mul cos 1 0.08 sub\n\
mul add 2 div} bind setscreen % Copyright (C) Y&Y 1989\n\
"; */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* set to 0.0 0.0 0.0 at top of page ? */

int bTextColor=0;

double text_red=0.0;     /* color of text */
double text_green=0.0;
double text_blue=0.0;

int bRuleColor=0;

double rule_red=0.0;     /* color of rules */
double rule_green=0.0;
double rule_blue=0.0;

int bFigureColor=0;     /* figure color has been specified */

double figurered=0.0;   /* foreground of figure */
double figuregreen=0.0;
double figureblue=0.0;

double backred=1.0;     /* background of figure */
double backgreen=1.0;
double backblue=1.0;

int bReverseVideo=0;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Some code imported from DVIWindo winspeci.c */

/* Stuff for reading simple uncompressed TIFF files (& PackBits compressed) */

unsigned int TIFFVersion;     /* TIFF version number */
unsigned int bLeastFirst=1;     /* least significant first */
unsigned int IFDCount;        /* number of items image file directory */
unsigned long IFDPosition;      /* position of image file directory */

//#pragma optimize ("lge", off)

/* static unsigned short int ureadtwo(FILE *input) {
  unsigned short int c, d;
  c = (unsigned short int) getc(input);
  d = (unsigned short int) getc(input); 
  if (bLeastFirst != 0) return ((d << 8) | c);
  else return ((c << 8) | d);
} */

static unsigned int ureadtwo(FILE *input)
{
  unsigned int c, d;
  c = (unsigned int) getc(input);
  d = (unsigned int) getc(input);
  if (bLeastFirst != 0)
    return ((d << 8) | c);
  else
    return ((c << 8) | d);
}

static unsigned long ureadfour(FILE *input)
{
  unsigned long c, d, e, f;
  c = (unsigned long) getc(input);
  d = (unsigned long) getc(input);
  e = (unsigned long) getc(input);
  f = (unsigned long) getc(input);
  if (bLeastFirst != 0)
    return ((((((f << 8) | e) << 8) | d) << 8) | c);
  else
    return ((((((c << 8) | d) << 8) | e) << 8) | f);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* char *typename[6] = {"", "BYTE ", "ASCII", "SHORT", "LONG ", "RATIO"}; */

int typesize[6] = {0, 1, 1, 2, 4, 8};   /* units of length/count */

unsigned long TIFFOffset, TIFFLength;

long PSOffset, PSLength, MetaOffset, MetaLength; /* not used ? */

/* we are ignoring the possibility here that length > 1 and such ... */

long indirectvalue (unsigned int type, long length, long offset, FILE *input)
{
  long present, val=0;

/*  UNUSED (length); */
#ifdef DEBUGTIFF
  if (length > 1 && traceflag)
  {
    sprintf(logline, "Length %d\n", length);
    showline(logline, 0);
  }
#endif
  present = ftell(input);     /* remember where we are */
  if (fseek(input, (long) (offset + TIFFOffset), SEEK_SET) != 0)
  {
    sprintf(logline, " Error in seek %s\n", " to indirect value\n");
    showline(logline, 1);
  }
  if (type == TYPE_LONG) val = (long) ureadfour(input);
  else if (type == TYPE_SHORT) val = ureadtwo(input);
  else if (type == TYPE_BYTE) val = getc(input);
  else showline(" Invalid Indirect Value\n", 1);
  fseek(input, present, SEEK_SET);  /* return to where we were */
  return val;
}

/* get value of a tag field in TIFF file */

long extractvalue (unsigned int type, unsigned long length, long offset, FILE *input)
{
  if (length == 0)
    return 0;

  switch(type)
  {
    case TYPE_BYTE:
      if (length <= 4)
        return offset;
      else
        return indirectvalue(type, (long) length, (long) offset, input);
    case TYPE_SHORT:
      if (length <= 2)
        return offset;
      else
        return indirectvalue(type, (long) length, (long) offset, input);
    case TYPE_LONG:
      if (length == 1)
        return offset;
      else
        return indirectvalue(type, (long) length, (long) offset, input);
    default:
      return -1;
  }
}

int skipthisimage (FILE *input, unsigned long ifdpos)
{
  int k, j;

  if(fseek(input, (long) ifdpos + TIFFOffset, SEEK_SET) != 0)
  {
    sprintf(logline, " Error in seek %s\n", " while skipping image\n");
    showline(logline, 1);
    return -1;
  }
  IFDCount = ureadtwo(input);     /* How many tags in this IFD */
  for (k = 0; k < (int) IFDCount; k++)  /* read to end of IFD */
  {
    for (j = 0; j < 12; j++)
      (void) getc(input);
  }
  IFDPosition = ureadfour(input);   /* get next IFD offset in file */
  if (IFDPosition == 0) return -1;      /*  no more IFDs !!! */
  else return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Must have: ImageWidth, ImageLength, StripOffset, SamplesPerPixel, BitsPerSample */

long ImageWidth;            /* vital ImageWidth */
long ImageLength;           /* vital ImageLength */

int bytes;                /* total bytes per row output used ? */

int SamplesPerPixel = 1;        /* vital (may be more than 1) */
int SamplesPerPixelX = 1;       /* SamplesPerPixel-ExtraSamples */

int BitsPerSample = 1;          /* vital */

int ExtraSamples = 0;         /* new 99/May/10 */

unsigned int compression = 0;     /* vital */
int Orientation = 1;
int Predictor = 1;            /* for LZW only */
                    /* 2 implies differencing applied */
long StripOffset = -1;          /* vital (first strip offset) */
long StripOffsetsPtr;
int StripOffsetsType;
int StripOffsetsLength;

long StripByteCount = -1;       /* first StripByteCount */
long StripByteCountsPtr;
int StripByteCountsType;
int StripByteCountsLength;

int RowsPerStrip = 0;

int StripsPerImage;           /* computed from above */

long ColorMapPtr = 0; /* pointer to map in case of Palette Color Images */

int PhotometricInterpretation = 1;  /* default */

int PlanarConfig=1;   /* cannot handle 2 ... */

int BitsPerPixel;   /* BitsPerSample * SamplePerPixel */
int BitsPerPixelX;    /* BitsPerSample * (SamplePerPixel-ExtraSamples) */

long InRowLength;     /* number of source bytes from file */
long InRowLengthX;    /* --- after ExtraSamples removed */

long InRowRead;     /* bytes to read from input */
long BufferLength;    /* number of bytes of intermediate data */
            /* length of lpBuffer allocated space */
long OutRowLength;    /* number of processed bytes for output */

int CurrentStrip;   /* for debugging purposes */

/* NOTE: MinSampleValue and MaxSampleValue should not affect appearance */

/* int MinSampleValue, MaxSampleValue;  */    /* never used ? */

int ResolutionUnit=2;   /* 1 no dimensions, 2 per inch, 3 per cm */

/* long xresnum, xresden; */    /* x resolution */
/* long yresnum, yresden; */    /* y resolution */

unsigned long xresnum, xresden;   /* x resolution */  /* 95/Oct/10 */
unsigned long yresnum, yresden;   /* y resolution */  /* 95/Oct/10 */

int hptagflag=0;          /* non-zero if call from HPTAG */
/* may need something more elaborate hleft / hright and vhigh / vlow */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* read the tag fields in the TIFF file, ignore ones we don't care about */

int readfields (FILE *input, unsigned long ifdpos)
{
  unsigned int k, tag, type;
  unsigned long length, offset;
  int c;

#ifdef DEBUGTIFF
  if (traceflag) showline("Now reading TIFF images fields\n", 0);
#endif

  if (fseek(input, (long) ifdpos + TIFFOffset, SEEK_SET) != 0)
  {
    sprintf(logline, " Error in seek %s\n", " while reading tags");
    showline(logline, 1);
    return -1;
  }
  IFDCount = ureadtwo(input);     /* How many tags in this IFD */
  
  ImageWidth = ImageLength = -1;
  SamplesPerPixel = BitsPerSample = 1;
  ExtraSamples = compression = 0;
  Orientation = 1;
  StripOffset = StripByteCount = -1;
  RowsPerStrip = 0;
  PhotometricInterpretation = 1;  /* default */
  PlanarConfig = Predictor = 1;
  ColorMapPtr = 0;  /* pointer to map in case of Palette Color Images */
/*  MinSampleValue = -1; MaxSampleValue = -1; */
/*  xresnum = yresnum = 72; */
  xresnum = yresnum = nDefaultTIFFDPI;
  xresden = yresden = 1;
  ResolutionUnit = 2; 

  for (k = 0; k < IFDCount; k++)
  {
    tag = ureadtwo(input);    /* tag - key */
    type = ureadtwo(input);   /* value type */
    if (tag == 0 && type == 0) {  /* invalid */
      c = getc(input); ungetc(c, input);
      sprintf(logline, " Tag: %u Type: %u (k %d c %d)\n",
          tag, type, k, c);
      showline(logline, 1);
      break;
    }
    if (type > 5) {
      c = getc(input); ungetc(c, input);
#ifdef DEBUGTIFF
      if (traceflag)
      {
        sprintf(logline, " Tag: %u Type: %u (k %d c %d)\n", tag, type, k, c);
        showline(logline, 1);
      }
#endif
/*      break; */ /* removed 98/Sep/22 */
    }
    length = ureadfour(input);  /* `length' (better named `count') */
    if (length == 1) {
      if (type == TYPE_LONG) offset = ureadfour(input);
      else if (type  == TYPE_SHORT) {
        offset = ureadtwo(input);
        (void) ureadtwo(input);   /* should be zero */
      }
      else if (type == TYPE_BYTE) {
        offset = getc(input);
        (void) getc(input);(void) getc(input);(void) getc(input);
      }
      else offset = ureadfour(input); /* for ratio e.g. */
    }
    else offset = ureadfour(input); /* value */

    switch (tag) {
      case IMAGEWIDTH:
        ImageWidth = extractvalue(type, length, (long) offset, input);
        break;
      case IMAGELENGTH:
        ImageLength = extractvalue(type, length, (long) offset, input);
        break;
      case BITSPERSAMPLE:
        BitsPerSample =
              (int) extractvalue(type, length, (long) offset, input);
        break;
      case EXTRASAMPLES:            /* 1999/May/10 */
        ExtraSamples =
              (int) extractvalue(type, length, (long) offset, input);
        break;        
      case COMPRESSION:
        compression =
          (unsigned int) extractvalue(type, length, (long) offset, input);
        break;
      case PREDICTOR:
        Predictor =
          (int) extractvalue(type, length, (long) offset, input);
        break;
      case SAMPLESPERPIXEL:
        SamplesPerPixel =
          (int) extractvalue(type, length, (long) offset, input);
        break;
      case ROWSPERSTRIP:
        RowsPerStrip =
          (int) extractvalue(type, length, (long) offset, input);
        break;
      case STRIPOFFSETS:
        StripOffsetsPtr = offset;
          StripOffsetsType = type;
          StripOffsetsLength = (int) length;
        StripOffset = extractvalue(type, length, (long) offset, input);
        break;
      case STRIPBYTECOUNTS:
        StripByteCountsPtr = offset;
          StripByteCountsType = type;
          StripByteCountsLength = (int) length;
        StripByteCount = extractvalue(type, length, (long) offset, input);
        break;
      case ORIENTATION:
        Orientation =
          (int) extractvalue(type, length, (long) offset, input);
        break;
/*      case MINSAMPLEVALUE:
        MinSampleValue = (int) extractvalue(type, length, (long) offset, input); 
        break; */
/*      case MAXSAMPLEVALUE:
        MaxSampleValue = (int) extractvalue(type, length, (long) offset, input);
        break; */
      case XRESOLUTION:
        xresnum = indirectvalue(TYPE_LONG, 1, (long) offset, input); 
        xresden = indirectvalue(TYPE_LONG, 1, (long) offset+4, input); 
        break; 
      case YRESOLUTION:
        yresnum = indirectvalue(TYPE_LONG, 1, (long) offset, input); 
        yresden = indirectvalue(TYPE_LONG, 1, (long) offset+4, input); 
        break; 
      case RESOLUTIONUNIT:
        ResolutionUnit = (int) extractvalue(type, length, (long) offset, input); 
        break; 
      case PHOTOMETRICINTERPRETATION:
        PhotometricInterpretation = 
          (int) extractvalue(type, length, (long) offset, input);
        break;
      case PLANARCONFIG:
         PlanarConfig = 
          (int) extractvalue(type, length, (long) offset, input);
        break;
      case COLORMAP:
/*        ColorMap = extractvalue(type, length, (long) offset, input); */
        ColorMapPtr = offset;
/*    Assume the type is SHORT (16 bit per entry) */
/*    Assume the length is 3 * 2 ^ BitsPerSample - 1 */
        break;
      default:              /* ignore unknown tags */
        break;
    }
  }
  return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int computeheader(void);
void writepsheader(FILE *, long, long);
void writepstrailer(FILE *);
int readpackbits(unsigned char *lpBuffer, FILE *input, int RowLength);
int huffmanrow(unsigned char *lpBuffer, FILE *input, int width);
int DecodeLZW(FILE *output, FILE *input, unsigned char *lpBuffer);
void expandcolor(unsigned char *s, long width);
void expandgray(unsigned char *s, long width);
void compresscolor(unsigned char *s, long width);
int writearow(FILE *output, unsigned char *s, unsigned long width);

#ifdef PSLEVEL2
void ASCIIinitfilter(FILE *output);
void ASCIIflushfilter(FILE *output);
void RUNinitfilter(FILE *output);
void RUNflushfilter(FILE *output);
#ifdef LZWCOMPRESSION
void LZWinitfilter(FILE *output);
void LZWflushfilter(FILE *output);
void LZWput(int, FILE *);
#endif
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Remember to deallocate at end */ /* Read color map from TIFF image */

/* do we need to use unsigned short for NT ? */

int ReadColorMap (FILE *input, long ColorMapPtr, int BitsPerSample)
{
  long present;
  int k, n, nint;

/*  should make all these unsigned short int instead for NT */
#ifdef USESHORTINT
  unsigned short int *PalettePtr;
#else
  unsigned int *PalettePtr;
#endif

#ifdef DEBUGTIFF
#ifdef USESHORTINT
  unsigned short int *PaletteRed;
  unsigned short int *PaletteGreen;
  unsigned short int *PaletteBlue;
#else
  unsigned int *PaletteRed;
  unsigned int *PaletteGreen;
  unsigned int *PaletteBlue;
#endif
#endif

/*  need three tables each of 2 ^ BitsPerSample integers */
  n = 1 << BitsPerSample;   /* integers per table */
  PaletteSize = n;      /* remember for later */
  nint = n * 3;       /* total number of integers */
#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "Reading Color Map of size %d\n", n);
    showline(logline, 1);
  }
#endif
/*  check following allocation in NT ? should we use short int ? */
#ifdef USESHORTINT
  Palette = (unsigned short int *)
        malloc(nint * sizeof(unsigned short int));
#else
/*  Palette = (unsigned int *) malloc(nint * 2); */ /* bytes */
  Palette = (unsigned int *) malloc(nint * sizeof(unsigned int));
#endif
  if (Palette == NULL)
  {
    showline(" ERROR: Unable to allocate memory\n", 1);
    checkexit(1);
//    or more serious exit(1) ???
  }
  present = ftell(input);
#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "Going to ColorMap at %ld + %lu\n",
         ColorMapPtr, TIFFOffset);
    showline(logline, 1);
  }
#endif
  if (fseek (input, (long) ColorMapPtr + TIFFOffset, SEEK_SET) != 0)
  {
    sprintf(logline, " Error in seek %s\n", " to ColorMap");
    showline(logline, 1);
  }
  PalettePtr = Palette;
/*  Following shoud work in NT even if we use int instead of short */
  for (k = 0; k < nint; k++) *PalettePtr++ = ureadtwo (input);
  fseek (input, present, SEEK_SET);
/*  return 0; */

  PaletteRed = Palette;
  PaletteGreen = PaletteRed + PaletteSize;
  PaletteBlue = PaletteGreen + PaletteSize;

  bGrayFlag = 1;    // non-zero if palette is all grays
  for (k = 0; k < n; k++)
  {
    if (PaletteRed[k] != PaletteGreen[k] ||
        PaletteGreen[k] != PaletteBlue[k])
    {
      bGrayFlag = 0;
      break;
    }
  }         // bGrayFlag presently not used

  bLinearFlag = 0;  // non-zero if Palette is simply linear
  if (bGrayFlag) {
    bLinearFlag = 1;
    for (k = 0; k < n; k++)
    {
      if (PaletteRed[k] != (unsigned int) (k * 255 / (n-1))) {
        bLinearFlag = 0;
        break;
      }
    }
  }         // bLinearFlag presently not used

//  bGrayFlag and bLinearFlag could be used to produce smaller output files

#ifdef DEBUGTIFF
  if (traceflag)
  {
    PaletteRed = Palette;
    PaletteGreen = PaletteRed + PaletteSize;
    PaletteBlue = PaletteGreen + PaletteSize;
    for (k = 0; k < n; k++) {
      if (traceflag) {
        sprintf(logline, "INX %d\tR %u\tG %u\tB %u\n",
             k, PaletteRed[k], PaletteGreen[k], PaletteBlue[k]);
        showline(logline, 0);
      }
    }
  }
#endif
  return 0;
}

/* returns -1 if output error */    /* given InRowLengthX */
/* expand color if needed - or expand gray if needed */
/* compress color if needed */
/* write out a row of image */

int ProcessRow (FILE *output, unsigned char *lpBuffer, long InRowLengthX,
  long BufferLength, long OutRowLength)
{
/*  if (traceflag)
    printf("ProcessRow InRowLengthX %d\n", InRowLengthX); */ /* DEBUGGING */
/*  if (traceflag)
    printf("bExpandColor %d bExpandGray %d bCompressColor %d\n",
       bExpandColor, bExpandGray, bCompressColor); */ /* DEBUGGING */
#ifndef _WINDOWS
  if (traceflag) fflush(stdout);    /* DEBUGGING ONLY */
#endif
  if (bExpandColor) expandcolor(lpBuffer, InRowLengthX);
  else if (bExpandGray) expandgray(lpBuffer, InRowLengthX);
  if (bCompressColor) compresscolor(lpBuffer, BufferLength);
#ifdef DEBUGTIFF
  if (traceflag) {      /* DEBUGGING */
    sprintf(logline, "ProcessRow OutRowLength %d\n", OutRowLength);
    showline(logline, 0);
  }
#endif
  if (writearow(output, lpBuffer, OutRowLength) != 0) return -1;
  else return 0;
}

/* Throw out ExtraSamples in SamplesPerPixel */
/* IMPORTANT NOTE: assume for the moment samples are one byte */

int RemoveExtraSamples (unsigned char *lpBuffer, int InRowLength)
{
  unsigned char *s = lpBuffer;
  unsigned char *t = lpBuffer;
  int i, k, n = InRowLength / SamplesPerPixel;

#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "RemoveExtraSamples InRowLength %d\n", InRowLength);
    showline(logline, 1);
  }
#endif
  if (ExtraSamples == 0) return InRowLength;
/*  check that BitsPerSample == 8 ??? */  
  for (k = 0; k < n; k++) {
    for (i = 0; i < SamplesPerPixelX; i++) *t++ = *s++;
    s += ExtraSamples;
  }
  return (int) (t - lpBuffer);
}

/* readflag != 0 when reading file only to get tag fields */

int readTIFFfile (FILE *output, FILE *input,
  long dwidth, long dheight, int nifd, int readflag)
{
/*  int i, j, flag; */
  int i, flag;
/*  long present; */
/*  int k, color, n; */
  unsigned long ImageSize;
  unsigned int nread;
  unsigned char *lpBuffer=NULL;
  int nLen;

  TIFFVersion = ureadtwo(input);
  if (TIFFVersion != TIFF_VERSION)
  {
    sprintf(logline, " Incorrect TIFF version code %d\n", TIFFVersion);
    showline(logline, 1);
    return -1;    /* bad version number for TIFF file */
  }
  
/*  Skip to desired image (if not first in file) */

  IFDPosition = ureadfour(input);   /* get first IFD offset in file */
  while (nifd-- > 1) {
    if (skipthisimage(input, IFDPosition) < 0)
    {
      sprintf(logline, " ERROR: Subimage %d not found", nifd);
      showline(logline, 1);
      return -1;
    }
  }

/*  Now at desired image */
  (void) readfields(input, IFDPosition);  /* read tag fields in TIFF file */

  IFDPosition = ureadfour(input);   /* get next IFD offset in file */

/*  if (readflag == 0) return -1; */  /*  only scanning for width & height */
  if (readflag != 0) return -1; /*  only scanning for width & height */

  bColorImage = 0;      /* if image is RGB or Palette color */
  bExpandColor = 0;     /* comes on if Palette color image */
  bExpandGray = 0;      /* comes on if Palette is gray */
  bCompressColor = 0;     /* if image is colored and not `colorimage'  */
  if (ColorMapPtr) {      /* is it a palette color image ? */
    (void) ReadColorMap(input, ColorMapPtr, BitsPerSample);
    bExpandColor = 1;
    bColorImage = 1;
  }

#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "Width %ld, Height %ld, BitsPerSample %d, SamplesPerPixel %d\n", 
       ImageWidth, ImageLength, BitsPerSample, SamplesPerPixel);
    showline(logline, 1);
  }
#endif

#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "Compression %u PhotometricInterpretation %d\n",
     compression, PhotometricInterpretation);
    showline(logline, 1);
  }
#endif

#ifdef DEBUGTIFF
  if (traceflag && Predictor != 1)
  {
    sprintf(logline, "Predictor %d\n", Predictor);
    showline(logline, 1);
  }
#endif

  if (ExtraSamples > 0) SamplesPerPixelX = SamplesPerPixel-ExtraSamples;
  else SamplesPerPixelX = SamplesPerPixel;
  
  BitsPerPixel = BitsPerSample * SamplesPerPixel;
  if (ExtraSamples > 0) BitsPerPixelX = BitsPerSample * SamplesPerPixelX;
  else BitsPerPixelX = BitsPerPixel;

  InRowLength = (ImageWidth * BitsPerPixel + 7) / 8;  /* row length file */
  if (ExtraSamples > 0)InRowLengthX = (ImageWidth * BitsPerPixelX + 7) / 8;
  else InRowLengthX = InRowLength;

  if (RowsPerStrip > 0)
    StripsPerImage = (int) ((ImageLength + RowsPerStrip - 1) / RowsPerStrip);
  else StripsPerImage = 1;

  if (bExpandColor) BufferLength = ImageWidth * 3;
  else if (bExpandGray) BufferLength = ImageWidth;
  else BufferLength = InRowLength;

/*  do compression of 24 bit color if use of `colorimage' not allowed */
/*  if (bitsperpixel == 24 && bCompressFlag != 0 && forceice == 0) */
/*  if (BitsPerPixel == 24 && bCompressFlag != 0) */
/*  if ((BitsPerPixel == 24 || bExpandColor) && bAllowColor == 0)  */
  if ((BitsPerPixelX == 24 || bExpandColor) && ! bAllowColor)
  {
    sprintf(logline, " color image (bits %d expand %d): use `*c' flag?",
         BitsPerPixelX, bExpandColor);  /* 96/Aug/15 */
//    showline(logline, 1);
    showline(logline, 0);     // reduce severity 2000 March 16
    bCompressColor = 1;
    OutRowLength = BufferLength / 3;
  }
  else OutRowLength = BufferLength;

#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "InRowLength %ld BufferLength %ld OutRowLength %ld\n",
      InRowLength, BufferLength, OutRowLength);
    showline(logline, 1);
  }
#endif

/*  deal with GhostScript TIFF file format with gaps */   /* 94/Dec/16 */
/*  shouldn't this only kick in if we are not using (LZW) compression ? */
  if (bGhostHackFlag) {       /*  made conditional 95/Nov/10 */
    if (InRowLength + 1 == StripByteCount)
    {
      InRowLength++;        /* a hack */
    }
  }
/*  should also increase BufferLength allocation ? 95/Nov/10 */

#ifdef PSLEVEL2
  bRunLengthFlag = 0;
  if (bAllowCompression) {
/*    Use runlength encoding if monochrome *or* if original uses it */
/*    if (BitsPerSample == 1 && SamplesPerPixel == 1) */
    if (BitsPerSample == 1 && SamplesPerPixelX == 1)
      bRunLengthFlag = 1;
    if (compression == PACK_BITS) bRunLengthFlag = 1;
#ifdef LZWCOMPRESSION
    bLZWFlag = 0;
/*    Use LZW if runlength encoding not chosen */
    if (bRunLengthFlag == 0)
    {
//      if (compression == LZW_COMPRESSION) bLZWFlag = 1;
//      else; bLZWFlag = 1          /* otherwise use LZW */
      bLZWFlag = 1;
    }
#endif
  }
#endif

  if (computeheader() != 0) return -1;  /* noticed format not supported? */

  writepsheader(output, dwidth, dheight);

/*  following should already be taken care of in `computeheader' */
/*  if (compression > 1 && compression != PACK_BITS) */
  if (compression > TIFF_CCITT && compression != LZW_COMPRESSION &&
    compression != PACK_BITS)
  {
    sprintf(logline, " ERROR: Unknown compression scheme (%d)", compression);
    showline(logline, 1);
    return -1;
  }

/*  ImageSize = (unsigned long) InRowLength * ImageLength; */
  ImageSize = (unsigned long) InRowLengthX * ImageLength;
/*  check whether values reasonable */
  if (ImageSize == 0)
  {
    sprintf(logline, " ERROR: Zero image size, %d %d (%s)",
        InRowLengthX, ImageLength, "readTIFFfile");
    showline(logline, 1);
    return -1;
  }
  if (ImageWidth > MAXIMAGE || ImageLength > MAXIMAGE ||  /* bad data ? */
    ImageSize > 67108864) { /* arbitrary limits (to catch bad files) */
    sprintf(logline,
      " ERROR: image file too large\n(%ld x %ld (%d) => %ld bytes)",
        ImageWidth, ImageLength, BitsPerPixel, ImageSize);
    showline(logline, 1);
    return -1;
  }
  if (ImageWidth < 0 || ImageLength < 0 ||    /* missing fields */
        StripOffset < 0) {          /* missing fields */
    showline(" ERROR: TIFF file missing required tags", 1);
    return -1;
  }

  if (fseek(input, (long) StripOffset + TIFFOffset, SEEK_SET) != 0)
  {
    sprintf(logline, " Error in seek %s\n", " to StripOffset\n");
    showline(logline, 1);
    return -1;
  }

/*  Maybe use far memory ? */ /* lot of changes if we do this ... */
/*  Maybe don't use if LZW compression ? */
/*  if (compression != LZW_COMPRESSION)  */
/*  if ((lpBuffer = malloc((int) BufferLength)) == NULL) {  */
/*  Accomodate GhostScript gap bug work around ... just in case 95/Nov/10 */
/*  nLen = BufferLength + 1; */
  nLen = (int) ((BufferLength + 3) / 4) * 4;
  if ((lpBuffer = malloc(nLen)) == NULL)
  {
    sprintf(logline, " ERROR: Unable to allocate %d bytes\n", nLen);
    showline(logline, 1);
    checkexit(1);         /* 1995/July/15 ? */
//    or more serious exit(1) ???
  }

/*  in case minsamplevalue and maxsamplevalue not given in tags */
/*  if (MinSampleValue < 0 || MaxSampleValue < 0 ||
      MinSampleValue == MaxSampleValue) {
    MinSampleValue = 0; MaxSampleValue = (1 << BitsPerSample) - 1;
  } */  /* never used */

/*  Actually go and read the file now ! */

/*  following should already be taken care of in `computeheader' */

  if (compression > TIFF_CCITT && compression != LZW_COMPRESSION &&
    compression != PACK_BITS)
  {
    sprintf(logline, " ERROR: Unknown compression scheme (%d)", compression);
    showline(logline, 1);
    return -1;
  }
#ifdef DEBUGTIFF
  if (traceflag)
  {
    if (compression == PACK_BITS) sprintf(logline, "Using PACK_BITS\n");
    else if (compression == TIFF_CCITT) sprintf(logline, "Using TIFF_CCITT\n");
    else if (compression == LZW_COMPRESSION) sprintf(logline, "Using LZW\n");
    showline(logline, 1);
  }
#endif

  flag = 0;               /* flag gets set if EOF hit */

#ifdef PSLEVEL2
  if (bLevel2)
  {
    ASCIIinitfilter(output);            /* 96/Dec/20 */
    if (bRunLengthFlag) RUNinitfilter(output);    /* 96/Dec/24 */
#ifdef LZWCOMPRESSION
    else if (bLZWFlag) LZWinitfilter(output);   /* 96/Dec/28 */
#endif
  }
#endif

/*  LZW needs to be done by strips, the others can be done by row */

  if (compression == LZW_COMPRESSION) DecodeLZW(output, input, lpBuffer);

  else {    /* else not LZW compression */

    for (i = 0; i < (int) ImageLength; i++) { /* read image lines */

      if (compression == PACK_BITS) {   /* compressed binary file */
        if (readpackbits (lpBuffer, input, (int) InRowLength) != 0) {
          flag = -1;
          break;
        }
      }
      else if (compression == TIFF_CCITT) { /* CCITT 1D compression */
        if (huffmanrow (lpBuffer, input, (int) ImageWidth) != 0) {
          flag = -1;
          break;
        }
      }
      else {                /* uncompressed file */
        nread = fread(lpBuffer, 1, (unsigned int) InRowLength, input);
        if (nread != (unsigned int) InRowLength) {  /* read a row */
          flag = -1;
          showline("Premature EOF ", 1);  /* ??? */
          break;
        } 
      }
#ifdef DEBUGTIFF
      if (traceflag)
      {
        sprintf(logline, "readTIFFFile InRowLength %d\n", InRowLength);
        showline(logline, 1);
      }
#endif
      if (ExtraSamples > 0)
        (void) RemoveExtraSamples(lpBuffer, InRowLength); /* 99/May/10 */

#ifdef DEBUGTIFF
      if (traceflag)
      {
        sprintf(logline, "readTIFFFile OutRowLength %d BufferLength %d\n",
             OutRowLength, BufferLength);
        showline(logline, 1);
      }
#endif

/*      if (ProcessRow (output, lpBuffer, InRowLength, BufferLength, */
      if (ProcessRow (output, lpBuffer, InRowLengthX, BufferLength,
        OutRowLength) != 0)
      {
        showline("\n", 0);
        showline("ERROR: Output error ", 1);
        perrormod((outputfile != NULL) ? outputfile : "");
        break;
      }
/*      if (flag != 0) break; *//* hit EOF or other error */
/*      check on abort flag here somewhere also ? */
    } /* end of this row */
  } /* end of not LZW compression */

//  Have to flush filters in reverse order
#ifdef PSLEVEL2
  if (bLevel2)
  {
    if (bRunLengthFlag) RUNflushfilter(output); /* 96/Dec/24 */
#ifdef LZWCOMPRESSION
    else if (bLZWFlag) LZWflushfilter(output);  /* 96/Dec/28 */
#endif
    ASCIIflushfilter(output);       /* 96/Dec/20 */
  }
#endif
  
/* now finished reading */  /* maybe use far memory for lpBuffer ? */

  if (lpBuffer != NULL) {
    free(lpBuffer);
    lpBuffer = NULL;
  }
  if (Palette != NULL) {
    free(Palette);
    Palette = NULL;
  }
  writepstrailer(output);
  return flag;
}    /* end of readTIFFfile(...) */

/**********************************************************************/

/* following is from windef.h */

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef int                 INT;
typedef unsigned int        UINT;

typedef long        LONG;

/* following is from wingdi.h */

#ifndef _WINDOWS

#define FAR

#include <pshpack2.h>
typedef struct tagBITMAPFILEHEADER {
  WORD    bfType;
  DWORD   bfSize;
  WORD    bfReserved1;
  WORD    bfReserved2;
  DWORD   bfOffBits;
} BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#include <poppack.h>

typedef struct tagBITMAPINFOHEADER {
  DWORD      biSize;
  LONG       biWidth;
  LONG       biHeight;
  WORD       biPlanes;
  WORD       biBitCount;
  DWORD      biCompression;
  DWORD      biSizeImage;
  LONG       biXPelsPerMeter;
  LONG       biYPelsPerMeter;
  DWORD      biClrUsed;
  DWORD      biClrImportant;
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

#include <pshpack1.h>
typedef struct tagRGBTRIPLE {
  BYTE    rgbtBlue;
  BYTE    rgbtGreen;
  BYTE    rgbtRed;
} RGBTRIPLE;
#include <poppack.h>

typedef struct tagRGBQUAD {
  BYTE    rgbBlue;
  BYTE    rgbGreen;
  BYTE    rgbRed;
  BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD FAR* LPRGBQUAD;

#endif

BITMAPFILEHEADER bmfh;

BITMAPINFOHEADER bmih;

/* RGBQUAD colortable[256]; */

int dpifrom(int res)
{
  double dres = (double) res * 25.4 / 1000.0;
  dres = (double) ((int) (dres * 10.0 + 0.5)) / 10.0;
  return (int) (dres + 0.5);
}

int colormaplength;   /* ??? */
int mingrey, maxgrey; /* ??? */
int nColors;      /* ??? */
long OffBits;     /* ??? */
long ImageSize;     /* ??? */

/* readflag is non-zero in prescan */

int readBMPfields (FILE *input, int readflag)
{
  long nLen;
  double dw, dh;

  ImageWidth = ImageLength = -1;
  SamplesPerPixel = BitsPerSample = 1;
  ExtraSamples = 0;
  compression = 0; Orientation = 1;
  StripOffset = -1; StripByteCount = -1;
  PhotometricInterpretation = 1;  /* default */
  PlanarConfig = 1; Predictor = 1;
  ColorMapPtr = 0;  /* pointer to map in case of Palette Color Images */
  mingrey = -1; maxgrey = -1;
  xresnum = yresnum = nDefaultTIFFDPI;    /* 96/Apr/3 */  
  xresden = yresden = 1;        /* 93/Oct/20 */
  ResolutionUnit = 2;         /* default dpi */ /* 93/Oct/20 */

  fseek(input, 0, SEEK_END);
  nLen = ftell(input);        /* Length of file */
//  fseek(input, 0, SEEK_SET);      /* rewind */
  rewind(input);
  fread(&bmfh, sizeof(bmfh), 1, input); /*  read file header */
  if (bmfh.bfType != (77 * 256 + 66)) {     /*  "BM" */
    sprintf(logline, "Not BMP file %X\n", bmfh.bfType);
    showline(logline, 1);
    return -1;
  }
/* "Size of header %lu\n", bmih.biSize compare to nLen ? */
  OffBits = bmfh.bfOffBits;
/* "Offset to image %lu\n", bmih.bfOffBits */
#ifdef DEBUGBMP
  if (traceflag && sreadflag)
  {
    sprintf(logline, "\nnLen %ld bfSize %ld bfOffBits %ld ",
       nLen, bmfh.bfSize, bmfh.bfOffBits);
    showline(logline, 1);
  }
#endif
/*  bmfh.bfSize file size in words ? NO */
/*  bmfh.bfOffBits offset from end of header ? NO */

  fread(&bmih, sizeof(bmih), 1, input); /*  read bitmap info header */

/*  if (bmih.biClrUsed > 0) nColor = bmih.biClrUsed;
  else if (bmih.biBitCount < 24) nColor = 1 << bmih.biBitCount; */
  if (bmih.biClrUsed > 0) nColors = bmih.biClrUsed;
  else if (bmih.biBitCount < 24) nColors = 1 << bmih.biBitCount;
  else nColors = 0;
  ImageWidth = bmih.biWidth;
  ImageLength = bmih.biHeight;
  if (bmih.biBitCount < 24) {
    BitsPerSample = bmih.biBitCount;
    SamplesPerPixel = 1;
  }
  else {
    BitsPerSample = 8;
    SamplesPerPixel = 3;
  }
#ifdef DEBUGBMP
  if (traceflag && readflag) {
    sprintf(logline, "\nBMIH: %d x %d bits %d x %d",
      ImageWidth, ImageLength, SamplesPerPixel, BitsPerSample);
    showline(logline, 1);
  }
  if (traceflag && readflag) {
    sprintf(logline, "\nBMIH: Size %ld Planes %d Compression %d SizeImage %ld ColorsUsed %d",
        bmih.biSize, bmih.biPlanes,
        bmih.biCompression, bmih.biSizeImage, bmih.biClrUsed);
    showline(logline, 1);

  }
#endif

/* "Number of image planes %u\n", bmih.biPlanes */
/* "Compression %lu\n", bmih.biCompression */
/* "Size of compressed image %lu\n", bmih.biSizeImage */
/* "Number of colors used %lu %d\n", bmih.biClrUsed */

/* "Horizontal pixel per meter %ld\n", bmih.biXPelsPerMeter */
  xresnum = dpifrom(bmih.biXPelsPerMeter);
  if (xresnum == 0) xresnum = nDefaultTIFFDPI;
  dw = (double) ImageWidth / xresnum;
/* "%d dpi (horizontal)\twidth %lg inch\n", xresnum, dw */

/* "Vertical pixel per meter %ld\n", bmih.biYPelsPerMeter */
  yresnum = dpifrom(bmih.biYPelsPerMeter);
  if (yresnum == 0) yresnum = nDefaultTIFFDPI;
  dh = (double) ImageLength / yresnum;
/* "%d dpi (vertical)\theight %lg inch\n", yresnum, dh */

  compression = bmih.biCompression; /* save it */
  ColorMapPtr = 0;
  if (nColors > 0) {    /*  read color table unless 24 bit */
    if (nColors > 256) {
      sprintf(logline, " ERROR: too many colors: %d\n", nColors);
      showline(logline, 1);
      nColors = 256;
      return -1;
    }
    ColorMapPtr = ftell(input); /* color map starts here */
    TIFFOffset = 0;
    colormaplength = sizeof(RGBQUAD) * nColors;
/*    fread(&colortable, sizeof(RGBQUAD), nColors, input); */
  }
/*  ColorMapPtr should be zero for 24 bit RGB images */
  OffBits= sizeof(bmfh) + sizeof(bmih) + sizeof(RGBQUAD) * nColors;
  fseek(input, OffBits, SEEK_SET);
  if (bmih.biSizeImage > 0) ImageSize = bmih.biSizeImage;
  else ImageSize = nLen - OffBits;
#ifdef DEBUGBMP
  if (traceflag && readflag)
  {
    sprintf(logline, "\nImage at %ld size %ld bytes ", OffBits, ImageSize);
    showline(logline, 1);
  }
#endif
  return 0;
}

int readBMPPalette (FILE *input, long ColorMapPtr, int BitsPerSample)
{
  long present;
  int k, n, nint;
  RGBQUAD rgb;
/*  should make all these unsigned short int instead for NT */

#ifdef USESHORTINT
  unsigned short int *PaletteRed;
  unsigned short int *PaletteGreen;
  unsigned short int *PaletteBlue;
#else
  unsigned int *PaletteRed;
  unsigned int *PaletteGreen;
  unsigned int *PaletteBlue;
#endif

/*  need three tables each of 2 ^ BitsPerSample integers */
  n = 1 << BitsPerSample;   /* integers per table */
#ifdef DEBUGBMP
  if (traceflag) {
    sprintf(logline, "Reading Color Map of size %d\n", n);
    showline(logline, 1);
  }
#endif
  PaletteSize = n;      /* remember for later */
  nint = n * 3;       /* total number of integers */
/*  check following allocation in NT ? should we use short int ? */
#ifdef USESHORTINT
  Palette = (unsigned short int *)
        malloc(nint * sizeof(unsigned short int));
#else
  Palette = (unsigned int *) malloc(nint * sizeof(unsigned int)); 
#endif
  if (Palette == NULL) {
    showline("Unable to allocate memory\n", 1);
    checkexit(1);
//    or more serious exit(1) ???
  }
  PaletteRed = Palette;
  PaletteGreen = PaletteRed + PaletteSize;
  PaletteBlue = PaletteGreen + PaletteSize;

  present = ftell(input);
#ifdef DEBUGBMP
  if (traceflag) {
    sprintf(logline, "Going to ColorMap at %ld\n", ColorMapPtr);
    showline(logline, 1);
  }
#endif
  if (fseek (input, (long) ColorMapPtr, SEEK_SET) != 0)
  {
    sprintf(logline, " Error in seek %s\n", " to ColorMap\n");
    showline(logline, 1);
  }
  for (k = 0; k < n; k++) {
    fread (&rgb, sizeof(RGBQUAD), 1, input);
    PaletteRed[k] = rgb.rgbRed; 
/*    PaletteRed[k] = rgb.rgbRed << 8; */
    PaletteGreen[k] = rgb.rgbGreen; 
/*    PaletteGreen[k] = rgb.rgbGreen << 8; */
    PaletteBlue[k] = rgb.rgbBlue; 
/*    PaletteBlue[k] = rgb.rgbBlue << 8; */
  }
  bGrayFlag = 1;      /* non-zero if palette has only grays */
  for (k = 0; k < n; k++) {
    if (PaletteRed[k] != PaletteGreen[k] ||
      PaletteGreen[k] != PaletteBlue[k]) {
      bGrayFlag = 0;
      break;
    }
  }
  bLinearFlag = 0;    /* non-zero if Palette is simply linear */
  if (bGrayFlag) {
    bLinearFlag = 1;
    for (k = 0; k < n; k++) {
      if (PaletteRed[k] != (unsigned int) (k * 255 / (n-1))) {
#ifdef DEBUGBMP
        if (traceflag) {
          sprintf(logline, "R[%d] = %d not %d\n",
            k, PaletteRed[k], (k * 255 / (n-1)));
          showline(logline, 1);
        }
#endif
        bLinearFlag = 0;
        break;
      }
    }
  }
  
/*  fseek (input, present, SEEK_SET); */
  return 0; 
}

/* readflag != 0 when prescanning to get fields */

int readBMPfile (FILE *output, FILE *input,
         long dwidth, long dheight, int readflag)
{
  unsigned char *lpBuffer=NULL;
  int i, flag;
  int nLen;
  unsigned int nread;
  unsigned char *s;
  int c, d, e=0;      /* keep compiler happy */
  int k, n;
  int evenflag=0;
  int nibble;

  (void) readBMPfields(input, readflag);  /* read tag fields in BMP file */
  BitsPerPixel = BitsPerSample * SamplesPerPixel; /* for bmp only */
  BitsPerPixelX = BitsPerPixel;
  if (readflag != 0) return 0;  /* if only reading for info */
  bColorImage = 0;      /* if image is RGB or Palette color */
  bExpandColor = 0;     /* comes on if Palette color image */
  bExpandGray = 0;      /* comes on if Palette is gray */
  bCompressColor = 0;     /* if image is colored and not `colorimage'  */
  if (ColorMapPtr > 0) {      /* do not read if 24 bit color */
    readBMPPalette (input, ColorMapPtr, BitsPerSample);
/*    check on bGrayFlag and bLinearFlag here ... */
#ifdef DEBUGBMP
    if (traceflag) {
      sprintf(logline, "bGrayFLag %d bLinearFlag %d\n",
          bGrayFlag, bLinearFlag);
      showline(logline, 1);
    }
#endif
    if (bGrayFlag) bColorImage = 0;
    else bColorImage = 1;
    if (! bLinearFlag) {
      if (bGrayFlag) bExpandGray = 1;
      else bExpandColor = 1;
    }
    if (BitsPerPixel == 1 && bGrayFlag != 0) {
      bExpandGray = 0;    /*    treat as MonoChrome */
      if (bLinearFlag) bInvertImage = 0;
      else bInvertImage = 1;
    }
/*    ??? check this */
  }
  else PaletteSize = 0; /* not Palette Color */
  if (bLinearFlag) PaletteSize = 0; /* no need to use Palette ??? */

#ifdef DEBUGBMP
  if (traceflag) {
    sprintf(logline, 
  "bColorImage %d bExpandColor %d bCompressColor %d PaletteSize %d\n", 
         bColorImage, bExpandColor, bCompressColor, PaletteSize);
    showline(logline, 1);
  }
  if (traceflag) {
    sprintf(logline, "Width %ld, Height %ld, BitsPerSample %d, SamplesPerPixel %d\n", 
         ImageWidth, ImageLength, BitsPerSample, SamplesPerPixel);
    showline(logline, 1);
  }
#endif

/* copied pretty much from readTIFF ... */

  SamplesPerPixelX = SamplesPerPixel;
  
  BitsPerPixel = BitsPerSample * SamplesPerPixel;   /* only BMP */
  BitsPerPixelX = BitsPerPixel;
  ExtraSamples = 0;
  InRowLength = (ImageWidth * BitsPerPixel + 7) / 8;  /* row length file */

  InRowRead = ((InRowLength + 3) / 4) * 4;  /* multiple of 4 bytes */
  if (bExpandColor) BufferLength = ImageWidth * 3;
  else if (bExpandGray) BufferLength = ImageWidth;
  else BufferLength = InRowLength;
  
  if ((BitsPerPixel == 24 || bExpandColor) && ! bAllowColor) {
    sprintf(logline, " color image (bits %d expand %d): use `*c' flag?",
         BitsPerPixel, bExpandColor); /* 96/Aug/15 */
    showline(logline, 1);
    bCompressColor = 1;
    OutRowLength = BufferLength / 3;
  }
  else OutRowLength = BufferLength;

#ifdef DEBUGBMP
  if (traceflag)
  {
    sprintf(logline, "InRowLength %ld BufferLength %ld OutRowLength %ld\n",
         InRowLength, BufferLength, OutRowLength);
    showline(logline, 1);
  }
#endif
  
#ifdef PSLEVEL2
  bRunLengthFlag = 0;
  if (bAllowCompression) {
/*    Use runlength encoding if monochrome *or* if original uses it */
/*    if (BitsPerSample == 1 && SamplesPerPixel == 1) */
    if (BitsPerSample == 1 && SamplesPerPixelX == 1)
      bRunLengthFlag = 1;
    if (compression == PACK_BITS) bRunLengthFlag = 1;
#ifdef LZWCOMPRESSION
    bLZWFlag = 0;
/*    Use LZW if runlength encoding not chosen */
    if (bRunLengthFlag == 0) {
//      if (compression == LZW_COMPRESSION) bLZWFlag = 1;
//      else bLZWFlag = 1;          /* otherwise use LZW */
      bLZWFlag = 1;
    }
#endif
  }
#endif

  if (computeheader () != 0) return -1; /* noticed format not supported? */

  writepsheader(output, dwidth, dheight);

  ImageSize = (unsigned long) InRowLength * ImageLength; /* for bmp only */

/*  check whether values reasonable */  
  if (ImageSize == 0) {
    sprintf(logline, " ERROR: Zero image size %d %d (%s)",
        InRowLength, ImageLength, "readBMPfile");
    showline(logline, 1);
    return -1;
  }
  if (ImageWidth > MAXIMAGE || ImageLength > MAXIMAGE ||  /* bad data ? */
    ImageSize > 67108864) { /* arbitrary limits (to catch bad files) */
    sprintf(logline,
        " ERROR: image file too large\n(%ld x %ld (%d) => %ld bytes)", 
        ImageWidth, ImageLength, BitsPerPixel, ImageSize);
    showline(logline, 1);
    return -1;
  }

#ifdef DEBUGBMP
  if (traceflag)
  {
    sprintf(logline, "Seeking to %ld\n", OffBits);
    showline(logline, 1);
  }
#endif
  if (fseek(input, (long) OffBits, SEEK_SET) != 0)
  {
    sprintf(logline, " Error in seek %s\n", " to OffBits\n");
    showline(logline, 1);
    return -1;
  }
  nLen = (int) ((BufferLength + 3) / 4) * 4;
  lpBuffer = malloc(nLen);
  if (lpBuffer == NULL)
  {
    sprintf(logline, " ERROR: Unable to allocate %d bytes\n", nLen);
    showline(logline, 1);
    checkexit(1);
//    or more serious exit(1) ???
  }

  flag = 0;               /* flag gets set if EOF hit */

#ifdef PSLEVEL2
  if (bLevel2)
  {
    ASCIIinitfilter(output);            /* 96/Dec/20 */
    if (bRunLengthFlag) RUNinitfilter(output);    /* 96/Dec/24 */
#ifdef LZWCOMPRESSION
    else if (bLZWFlag) LZWinitfilter(output);   /* 96/Dec/28 */
#endif
  }
#endif

  for (i = 0; i < (int) ImageLength; i++) { /* read image lines */

/*    May need to uncompress here if compression != 0 */
/*    nread = fread(lpBuffer, 1, (unsigned int) InRowLength, input); */
    if (compression == 0)
      nread = fread(lpBuffer, 1, (unsigned int) InRowRead, input);
    else if (BitsPerPixel == 4) { /* decompress, four bit images */
      evenflag = 1;       /* even nibble is next (output) */
      s = lpBuffer;
      while (s <= lpBuffer + InRowRead) {
        c = getc(input);
#ifdef DEBUGBMP
        if (traceflag) {
          sprintf(logline, " %d", c);
          showline(logline, 1);
        }
#endif
        if (c > 0) {      /* repeating group pixel count */
          d = getc(input);  /* pixel */
          e = d & 15;     /* decompose right nibble */
          d = d >> 4;     /* decompose left nibble */
#ifdef DEBUGBMP
          if (traceflag) {
            sprintf(logline, " repeat %d %d", d, e);
            showline(logline, 1);
          }
#endif
          for (k = 0; k < c; k++) {
            nibble = (k % 2 == 0) ? d : e;
/*            now stuff nibble */
            if (evenflag) *s = (unsigned char) (nibble << 4);
/*            else *s++ = *s | nibble; */
            else *s++ = (unsigned char) (*s | nibble);
            evenflag = 1 - evenflag;
          }
#ifdef DEBUGBMP
          if (traceflag) {
            sprintf(logline, " buffered: %d,", s - lpBuffer);
            showline(logline, 1);
          }
#endif
        }
        else {    /* c == 0 */
          c = getc(input);  /* pixel count */
#ifdef DEBUGBMP
          if (traceflag) {
            sprintf(logline, " literal %d", c);
            showline(logline, 1);
          }
#endif
          if (c > 2) {    /* literal group */
            for (k = 0; k < c; k++) {
              if (k % 2 == 0) {
                d = getc(input);
                e = d & 15;     /* decompose right nibble */
                d = d >> 4;     /* decompose left nibble */
#ifdef DEBUGBMP
                if (traceflag) {
                  sprintf(logline, " %d %d", d, e);
                  showline(logline, 1);
                }
#endif
                nibble = d;
              }
              else nibble = e;
/*              now stuff nibble */
              if (evenflag) *s = (unsigned char) (nibble << 4);
/*              else *s++ = *s | nibble; */
              else *s++ = (unsigned char) (*s | nibble);
              evenflag = 1 - evenflag;
            }
            n = 2 + (c+1)/2;    /* bytes read */
            if ((n % 2) != 0) {   /* padded out to even bytes */
              d = getc(input);
#ifdef DEBUGBMP
              if (traceflag) {
                sprintf(logline, " discard: %d", d);
                showline(logline, 1);
              }
#endif
            }
#ifdef DEBUGBMP
            if (traceflag) {
              sprintf(logline, " buffered: %d,", s - lpBuffer);
              showline(logline, 1);
            }
#endif
          }
          else if (c == 0) {
            if (evenflag++ == 0) s++; /* pad last nibble */
#ifdef DEBUGBMP
            if (traceflag) {
              sprintf(logline, " EOL\n");
              showline(logline, 1);
            }
#endif
            if (s < lpBuffer + InRowLength) {
              sprintf(logline, " Did not fill row %d %d\n",
                  s - lpBuffer, InRowLength);
              showline(logline, 1);
              while (s < lpBuffer + InRowLength) *s++ = '\0';
            }
            while (s < lpBuffer + InRowRead) *s++ = '\0';
            break;  /* end of row */
          }
          else if (c == 1) {
#ifdef DEBUGBMP
            if (traceflag) {
              showline(" EOI\n", 0);
            }
#endif
            if (i != (int) ImageLength - 1) {
#ifdef DEBUGBMP
              if (traceflag) {
                sprintf(logline, "i+1 %d ImageLength %d\n",
                    i+1, ImageLength);
                showline(logline, 0);
              }
#endif
              showline(" Premature end of image\n", 1);
            }
            break;  /* end of picture */
          }
          else if (c == 2) {  /* rats! */
            showline(" Skips not implemented\n", 1);
          }
        }
      }
      nread = s - lpBuffer;
    }
    else {  /* decompress, eight bit images only so far */
      s = lpBuffer;
      while (s <= lpBuffer + InRowRead) {
        c = getc(input);
#ifdef DEBUGBMP
        if (traceflag) {
          sprintf(logline, " %d", c);
          showline(logline, 0);
        }
#endif
        if (c > 0) {      /* repeating group */
          d = getc(input);  /* pixel */
#ifdef DEBUGBMP
          if (traceflag) {
            sprintf(logline, " repeat: %d", d);
            showline(logline, 0);
          }
#endif
/*          for (k = 0; k < c; k++) *s++ = d; */
          for (k = 0; k < c; k++) *s++ = (unsigned char) d;
#ifdef DEBUGBMP
          if (traceflag) {
            sprintf(logline, " buffered: %d,", s - lpBuffer);
            showline(logline, 0);
          }
#endif
        }
        else {
#ifdef DEBUGBMP
          if (traceflag) {
            sprintf(logline, " buffered: %d,", s - lpBuffer);
            showline(logline, 0);
          }
#endif
          c = getc(input);  /* pixel count */
#ifdef DEBUGBMP
          if (traceflag) {
            sprintf(logline, " %d", c);
            showline(logline, 0);
          }
#endif
          if (c > 2) {    /* literal group */
            for (k = 0; k < c; k++) {
              d = getc(input);
              *s++ = (unsigned char) d;
#ifdef DEBUGBMP
              if (traceflag) {
                sprintf(logline, " %d", d);
                showline(logline, 0);
              }
#endif
            }
            if ((c % 2) != 0) {   /* padded out to even bytes */
              d = getc(input);
#ifdef DEBUGBMP
              if (traceflag) {
                sprintf(logline, " %d", d);
                showline(logline, 0);
              }
#endif
            }
          }
          else if (c == 0) {
            if (evenflag++ == 0) *s++ = '\0'; /* pad last byte */
#ifdef DEBUGBMP
            if (traceflag) {
              sprintf(logline, " EOL\n");
              showline(logline, 0);
            }
#endif
            if (s < lpBuffer + InRowLength) {
              sprintf(logline, " Did not fill row %d %d\n",
                   s - lpBuffer, InRowLength);
              showline(logline, 1);
              while (s < lpBuffer + InRowLength) *s++ = '\0';
            }
            while (s < lpBuffer + InRowRead) *s++ = '\0';
            break;  /* end of row EOL */
          }
          else if (c == 1) {
            if (evenflag++ == 0) *s++ = '\0';;    /* pad last byte */
#ifdef DEBUGBMP
            if (traceflag) showline(" EOI\n", 0);
#endif
            if (i != (int) ImageLength - 1) {
              showline(" Premature end of image\n", 1);
            }
            break;  /* end of picture EOF */
          }
          else if (c == 2) {  /* rats! */
            showline(" Skips not implemented\n", 1);
          }
        }
      }
      nread = s - lpBuffer;
    }
/*    if (nread != (unsigned int) InRowLength) */
    if (nread != (unsigned int) InRowRead) {
      flag = -1;
#ifdef DEBUGBMP
      if (traceflag) {
        sprintf(logline, "nread %d InRowRead %d\n", nread, InRowRead);
        showline(logline, 1);
      }
#endif
      showline(" ERROR: Premature EOF\n", 1);
      break;
    } 

    if (ProcessRow (output, lpBuffer, InRowLength, BufferLength,
            OutRowLength) != 0) {
      showline(" Output error\n", 1);     /* BMP */
      perrormod((outputfile != NULL) ? outputfile : "");
      break;
    }
/*      if (flag != 0) break; *//* hit EOF or other error */
/*      check on abort flag here somewhere also ? */
  } /* end of this row */

//  Have to flush filters in reverse order
#ifdef PSLEVEL2
  if (bLevel2) {
    if (bRunLengthFlag) RUNflushfilter(output); /* 96/Dec/24 */
#ifdef LZWCOMPRESSION
    else if (bLZWFlag) LZWflushfilter(output);  /* 96/Dec/28 */
#endif
    ASCIIflushfilter(output);       /* 96/Dec/20 */
  }
#endif

/* now finished reading */  /* maybe use far memory for lpBuffer ? */

  if (lpBuffer != NULL) {
    free(lpBuffer);
    lpBuffer = NULL;
  }
  if (Palette != NULL) {
    free(Palette);
    Palette = NULL;
  }
  writepstrailer(output);
  return flag;
/*  UNDER CONSTRUCTION !!! */
}    /* end of readBMPfile(...) */
  

/**********************************************************************/

/* Try and see whether EPSF file and read header info if so: */
/* fills in TIFFOffset and PSOffset and MetaOffset and lengths */
/* returns zero if not an EPSF file */ /* file position is end of EPSF head */

int readepsfhead (FILE *special)
{
  int c, d, e, f;

  PSOffset = 0;       /* redundant */
  MetaOffset = 0;       /* redundant */
  TIFFOffset = 0;       /* redundant */

  c = getc(special); d = getc(special);
  e = getc(special); f = getc(special);
  if (c == 'E' + 128 && d == 'P' + 128 &&
    e == 'S' + 128 && f == 'F' + 128) {
    bLeastFirst = 1;
    PSOffset = (long) ureadfour(special); /* read PS start offset */ 
    PSLength = (long) ureadfour(special); /* read PS length */
    MetaOffset = (long) ureadfour(special); /* read MF start offset */
    MetaLength = (long) ureadfour(special); /* read MF length */
    TIFFOffset = (long) ureadfour(special); /* read TIFF start offset */
    TIFFLength = (long) ureadfour(special); /* read TIFF length */
    (void) ureadtwo(special);     /* should be 255, 255 */
    return -1;
  }
  else return 0;              /* not an EPSF file */
}

int BMPflag=0;    /* non-zero while processing BMP image */

int readimagefilesub (FILE *output, FILE *special,
  long dwidth, long dheight, int nifd, int readflag)
{
/*  long present; */
  int c, d; 

  TIFFOffset = 0;       /* normally beginning of file */
  PSOffset = 0;
  MetaOffset = 0;
  BMPflag=0;

  c = getc(special);
  (void) ungetc(c, special);
  if (c > 128) {        /* see whether perhaps EPSF file */
    if (readepsfhead(special) != 0) {     /* is it EPSF file ? */
/*    If valid EPSF header with pointer to TIFF we get here */
    if (TIFFOffset == 0 || TIFFLength == 0) {
        showline(" ERROR: Zero TIFF offset or length\n", 1);
        return -1;
      }
      if (fseek(special, TIFFOffset, SEEK_SET) != 0)
      {
        sprintf(logline, " Error in seek %s\n", "to TIFFOffset\n");
        showline(logline, 1);
        return -1;
      }
    }
    else {
      showline(" ERROR: Not a valid EPSF or TIFF file\n", 1);
      return -1;
    }
  }             /* end of c > 128 (EPSF file) */

/*  Try and deal with PostScript ASCII stuff */
/*  Or may meet TIFF / BMP preamble */ 
  c = getc(special); d = getc(special);
  if (c == '%' && d == '!') {   /* ordinary PS file ? */
    showline(" ERROR: Ordinary PS file, not TIFF\n", 1);
    return -1;
  }
  else if (c == 'I' && d == 'I') bLeastFirst = 1; /* PC style TIFF file */
  else if (c == 'M' && d == 'M') bLeastFirst = 0; /* Mac style TIFF file */
  else if (c == 'B' && d == 'M') BMPflag = 1;   /* BMP 98/Jun/28 */
  else {
    showline(" ERROR: Not a valid EPSF, TIFF or BMP file\n", 1);
    return -1;    /* not a TIFF subfile !!! */
  }

/*  now have decided that this is a TIFF file (or TIFF preview) or BMP */
  if (BMPflag)
    (void) readBMPfile(output, special, dwidth, dheight, readflag);
  else
    (void) readTIFFfile(output, special, dwidth, dheight, nifd, readflag);
  return 0;
}

/*  In prescan to read TIFF tags, readflag = 1 */
/*  Later, when actually using the file, readflag = 0 */
/*  Prescan only happens if dheight and dwidth are not already given */
/*  Returns -1 if file not found */

int readimagefile (FILE *output, char *filename,
  long dwidth, long dheight, int nifd, int readflag)
{
/*  char infilename[FILENAME_MAX]; */ /* make global */
  FILE *special;
/*  long present;  */
  int flag;

  strcpy(infilename, filename);
/*  if ((special = findepsfile(infilename, -1, "tif")) == NULL) { */
/*  if ((special = findepsfile(infilename, "tif", 1, readflag)) == NULL) */
/*  1996/Mar/2 don't warn during prescan when readflag = 1 ??? */
  if ((special = findepsfile(infilename, "tif", !readflag, readflag)) == NULL)
    return -1;          

#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "\nManaged to open file %s\n", infilename);
    showline(logline, 0);
  }
#endif
  flag = readimagefilesub (output, special, dwidth, dheight, nifd, readflag);
  fclose(special);
  return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int column;

/* Bitspersample can be 1, 2, 4, 8 for ease of conversion to PS */
/* Each row takes an integral number of bytes */

/* Expand palette color image into full RGB color image */
/* Presently only set up for 8 bit palettes ... !!! */

void expandcolor (unsigned char *lpBuffer, long width)
{
  int k, n, ns;
#ifdef USESHORTINT
  unsigned short int *PaletteRed;
  unsigned short int *PaletteGreen;
  unsigned short int *PaletteBlue;
#else
  unsigned int *PaletteRed;
  unsigned int *PaletteGreen;
  unsigned int *PaletteBlue;
#endif
  unsigned char *s;
  unsigned char *t;
  unsigned int red=0, green=0, blue=0;
  int PaletteIndex, Mask;
  int Byte = 0, BitsLeft = 0;

#ifdef DEBUGTIFF
/*  if (traceflag) printf("Palette Color => Color "); */
#endif

  if (Palette == NULL)
  {
    showline(" ERROR: missing palette information\n", 1);
    return;
  }
  PaletteRed = Palette;
  PaletteGreen = PaletteRed + PaletteSize;
  PaletteBlue = PaletteGreen + PaletteSize;
  Mask = (1 << BitsPerSample) - 1;

/*  First slide input data upwards to top of buffer area */

  n = (int) width;          /* InRowLengthX */
  ns = (int) ImageWidth;        /* samples per row */

/*  if (BufferLength <= InRowLength) */
  if (BufferLength <= InRowLengthX) {
    showline(" ERROR: buffer overflow\n", 1);
    return;
  }

/*  possibly use InRowLengthX ? in next two places */

  for (k = n-1; k >= 0; k--)  /* move it up to end of lpBuffer space */
/*    lpBuffer[k + BufferLength - InRowLength] = lpBuffer[k]; */
    lpBuffer[k + BufferLength - InRowLengthX] = lpBuffer[k];
  
/*  s = lpBuffer + BufferLength - InRowLength; */ /* start of input data */
  s = lpBuffer + BufferLength - InRowLengthX; /* start of input data */
  if (BitsPerSample != 8) {         /* nibble code init */
    Byte = 0; BitsLeft = 0;
  }
  t = lpBuffer;               /* start of output data */
  for (k = 0; k < ns; k++) {
    if (BitsPerSample != 8) {       /* slow, but inefficient ! */
      if (BitsLeft <= 0) {
        Byte = *s++; BitsLeft = 8;
      }
      BitsLeft -= BitsPerSample;
/*      PaletteIndex = Byte >> (BitsPerSample - BitsLeft); */
      PaletteIndex = Byte >> BitsLeft;
      PaletteIndex = PaletteIndex & Mask;
/*      BitsLeft -= BitsPerSample; */
    }
    else PaletteIndex = *s++;
    if (PaletteIndex < PaletteSize) {
      red = PaletteRed[PaletteIndex];
      green = PaletteGreen[PaletteIndex];
      blue = PaletteBlue[PaletteIndex];
    }
    else {
      sprintf(logline, " ERROR: Bad Palette Index %d >= %d\n",
          PaletteIndex, PaletteSize);
      showline(logline, 1);
    }
#ifdef DEBUGTIFF
    if (traceflag) {
      sprintf(logline, "INX %d\tR %u\tG %u\tB %u\n",
                PaletteIndex, red, green, blue); 
      showline(logline, 0);
    }
#endif
    if (BMPflag) {        /* BMP */
      *t++ = (unsigned char) red; 
      *t++ = (unsigned char) green; 
      *t++ = (unsigned char) blue;
    }
    else {            /* TIFF */
      *t++ = (unsigned char) (red >> 8); 
      *t++ = (unsigned char) (green >> 8); 
      *t++ = (unsigned char) (blue >> 8);
    }
  } 
}

/* This is called for BMP image when palette is used, but it is gray */
/* (i.e. R=G=B) and it is *not* linear (i.e. Palette[k] == k) */

void expandgray (unsigned char *lpBuffer, long width)
{
  int k, n, ns;
#ifdef USESHORTINT
  unsigned short int *PaletteRed;
  unsigned short int *PaletteGreen;
  unsigned short int *PaletteBlue;
#else
  unsigned int *PaletteRed;
  unsigned int *PaletteGreen;
  unsigned int *PaletteBlue;
#endif
  unsigned char *s;
  unsigned char *t;
  unsigned int red, green, blue;
  int PaletteIndex, Mask;
  int Byte = 0, BitsLeft = 0;

#ifdef DEBUGBMP
/*  if (traceflag) printf("Palette Gray => Gray ");  */
#endif

  if (Palette == NULL)
  {
    showline(" ERROR: missing palette information\n", 1);
    return;
  }
  PaletteRed = Palette;
  PaletteGreen = PaletteRed + PaletteSize;
  PaletteBlue = PaletteGreen + PaletteSize;
  Mask = (1 << BitsPerSample) - 1;

/*  First slide input data upwards to top of buffer area */

  n = (int) width;          /* InRowLength */
  ns = (int) ImageWidth;        /* samples per row */

/*  if (BufferLength <= InRowLength) */
  if (BufferLength <= InRowLengthX)
  {
    showline(" ERROR: buffer overflow\n", 1);
    return;
  }

  for (k = n-1; k >= 0; k--)
/*    lpBuffer[k + BufferLength - InRowLength] = lpBuffer[k]; */
    lpBuffer[k + BufferLength - InRowLengthX] = lpBuffer[k];

/*  s = lpBuffer + BufferLength - InRowLength; */ /* start of input data */
  s = lpBuffer + BufferLength - InRowLengthX;   /* start of input data */
  if (BitsPerSample != 8) {         /* nibble code init */
    Byte = 0; BitsLeft = 0;
  }
  t = lpBuffer;               /* start of output data */
  for (k = 0; k < ns; k++) {
    if (BitsPerSample != 8) {       /* slow, but inefficient ! */
      if (BitsLeft <= 0) {
        Byte = *s++; BitsLeft = 8;
      }
      BitsLeft -= BitsPerSample;
/*      PaletteIndex = Byte >> (BitsPerSample - BitsLeft); */
      PaletteIndex = Byte >> BitsLeft;
      PaletteIndex = PaletteIndex & Mask;
/*      BitsLeft -= BitsPerSample; */
    }
    else PaletteIndex = *s++;
    if (PaletteIndex < PaletteSize) {
      red = PaletteRed[PaletteIndex];
      green = PaletteGreen[PaletteIndex];   /* not used */
      blue = PaletteBlue[PaletteIndex];   /* not used */
    }
    else {
      sprintf(logline, " ERROR: Bad Palette Index %d >= %d\n",
           PaletteIndex, PaletteSize);
      showline(logline, 1);
      red = green = blue = 0;         /* keep compiler happy */
    }
#ifdef DEBUGTIFF
/*    if (traceflag) printf("Index %d red %u green %u blue %u\n", */
    if (traceflag) {
      sprintf(logline, "INX %d\tR %u\tG %u\tB %u\n",
          PaletteIndex, red, green, blue); 
      showline(logline, 0);
    }
#endif
    if (BMPflag) *t++ = (unsigned char) red; /* BMP */
    else *t++ = (unsigned char) (red >> 8);  /* TIFF */
  } 
}

/* gray = 0.3 * red + 0.59 * green + 0.11 * blue */

void compresscolor (unsigned char *lpBuffer, long width)
{
  int k, n;
  unsigned char *s;
  unsigned char *t;
  unsigned int red, green, blue, gray;

#ifdef DEBUGTIFF
  if (traceflag) showline("Color => Gray ", 0);
#endif
  s = lpBuffer; t = lpBuffer;
  n = (int) (width / 3);
  for (k = 0; k < n; k++) {
    red = *s++; green = *s++; blue = *s++;
    gray = (int) (((long) blue * 28 + (long) red * 77 + (long) green * 151)
      >> 8);
    *t++ = (unsigned char) gray;
  } 
} 

/*****************************************************************************/

#ifdef PSLEVEL2

//#pragma optimize ("lge", on)    /* 2000 June 17 */

/* use ASCII85Encode if bLevel2 enabled */

/* ASCII85 encoding (b1 b2 b3 b4) => (c1 c2 c3 c4 c5) */

/* b1 * 256^3 + b2 * 256^2 + b3 * 256  + b4 = 
   c1 * 85^4  + c2 * 85^3  + c3 * 85^2 + c4 * 85 + c5 */

/* Add `!' =  33 to make ASCII code. ! represents 0, u represent 84. */

/* If all five digits are zero, use z instead of !!!!! - special case. */

/* At end, given n = 1, 2, 3 bytes of binary data, append 4 - n zeros.
   Encode this in usual way (but *without* z special case). 
   Write first (n+1) bytes.  Follow by -> (EOD) */

unsigned long asciinum = 0;   /* accumulated so far up to four bytes */
int asciicount = 0;       /* how many bytes accumulated so far */  

/* Output 4 bytes --- 32 bit unsigned int --- as 5 ASCII 85 bytes */
/* nbytes is number of bytes accumulated, which is 4 until the last time */
/* Move this inline for speed ? */

void ASCIIlong (FILE *output, unsigned long n, int nbytes)
{
  unsigned int c[5];
  int k;

//  sprintf(logline, "ASCIIlong %08X %d ", n, nbytes);
//  showline(logline, 0);   // debugging only
  if (nbytes == 0) nbytes--;      /* nothing to do at end - avoid ! */
  if (n == 0 && nbytes == 4) {    /* special case !!!!! => z */
    PSputc('z', output);      /* shorthand for !!!!! */
    column++;
    if (column >= MAXCOLUMNASCII) {
      PSputc('\n', output);
      column = 0;
    }
    return;
  }
  for (k = 4; k >= 0; k--) {    /* get base 85 digits reverse order */
    c[k] = (unsigned int) (n % 85);
    n = n / 85;
  }
  for (k = 0; k < 5; k++) {   /* spit them out in correct order */
    if (nbytes-- < 0) {         /* last one, finish with EOD */
      if (column+1 >= MAXCOLUMNASCII) { /* see if space for ~> */
        PSputc('\n', output);
        column = 0;
      }
      PSputs("~>\n", output); /* EOD */
      column = 0;
      return;
    }
    PSputc((char) (c[k] + '!'), output);
    column++;
    if (column >= MAXCOLUMNASCII) {
      PSputc('\n', output);
      column = 0;
    }
  } /* end of for k = 0; k < 5; k++ */
}

/* Output single byte (accumulate until four seen, then call ASCIIlong) */
/* Move this inline for speed ? */

void ASCIIout (FILE *output, unsigned int x)
{
//  sprintf(logline, "ASCII %d ", x);
//  showline(logline, 0);     // debugging only
  if (x > 255) x = x & 255;   // sanity check ???
  asciinum = (asciinum << 8) | x;
  asciicount++;
  if (asciicount >= 4) {
    ASCIIlong(output, asciinum, asciicount);
    asciinum = 0;
    asciicount = 0;
  }
}

/* initialize ASCII85 filter */

void ASCIIinitfilter (FILE *output)   /* 96/Dec/20 */
{
//  showline("ASCIIinitfilter ", 0);  // debugging only
  asciinum = 0;
  asciicount = 0;
  column = 0;     /* ??? */
}

void RUNinitfilter (FILE *output)    /* 95/Dec/24 */
{
/*  apparently nothing special to do */
}

/* flush out anything left in ASCII85 filter */

void ASCIIflushfilter (FILE *output)  /* 96/Dec/20 */
{
  int k;
//  showline("ASCIIflushfilter ", 0); // debugging only
  for (k = asciicount; k < 4; k++) {
    asciinum = (asciinum << 8);   /* append n-4 zeros */
  }
  ASCIIlong(output, asciinum, asciicount);
  asciinum = 0;
  asciicount = 0;
}

void RUNflushfilter (FILE *output)
{
  ASCIIout(output, 128);          /* EOD */
}

/*****************************************************************************/

/* write a row in ASCII85 format */

void writearowASCII (FILE *output, unsigned char *s, unsigned long width)
{
  unsigned int c;
  int k, n, i;
  unsigned long num;

  n = (int) width;
  k = 0;
  if (asciicount > 0) { /* finish off group of four bytes first */
    for (i = asciicount; i < 4; i++) {
      ASCIIout(output, *s++);
      k++;
      if (k >= n) break;  /* in case of miniscule width! */
    }
  }
  while (k < n-3) {   /* then do groups of four bytes */
    num = 0;
    for (i = 0; i < 4; i++) {
      c = *s++;
      num = (num << 8) | c;
    }
    ASCIIlong(output, num, 4);
    k += 4;
  }
  while (k < n) {     /* then do remaining bytes at tail of row */
    ASCIIout(output, *s++);
    k++;
  }
}

/**************************************************************************/

#if (defined(DEBUGLZWENCODE) || defined(DEBUGRUNLENGTH))
int debugflag=1;      /* debugging output */
#endif

/* Do run encoding per row (rather than image) --- easier, if not optimal */

void dumprun(FILE *output, int nlen, int previous)
{
#ifdef DEBUGRUNLENGTH
  int i;
  if (nlen <= 0) {
    sprintf(logline, "Zero length run %d ", nlen);
    showline(logline, 0);
    return;
  }
  else if (nlen < 3) {
    sprintf(logline, " ERROR: run %d < 3\n", nlen);
    showline(logline, 1);
  }
  else if (nlen > 128) {
    sprintf(logline, " ERROR: run %d > 128", nlen);
    showline(logline, 1);
  }
  for (i = 0; i < nlen; i++) {
    sprintf(logline, "%02X", previous);
    showline(logline, 1);
  }
#endif
  ASCIIout(output, 257 - nlen);
  ASCIIout(output, previous);
}

void dumpnonrun(FILE *output, int nlen, unsigned char *buffer)
{
  int i;
  unsigned char *s;

#ifdef DEBUGRUNLENGTH
  if (nlen <= 0) {
    sprintf(logline, "Zero length nonrun %d ", nlen);
    showline(logline, 0);
    return;
  }
  if (nlen > 128) {
    sprintf(logline, " ERROR: nonrun %d > 128", nlen);
    showline(logline, 1);
  }
  if (debugflag) {
    s = buffer;
    for (i = 0; i < nlen; i++) {
      int c;
      c = *s++;
      sprintf(logline, "%02X", c);
      showline(logline, 1);
    }
  }
#endif
  ASCIIout(output, nlen - 1);
  s = buffer;
  for (i = 0; i < nlen; i++) ASCIIout(output, *s++);
}

/* RunLengthEncode filter */

/* This basically has two states: */
/* (1) accumulating non-run (runflag == 0) */
/* (2) accumulating run (runflag != 0) */
/* Here (1) is the starting state */
/* Dumps out when (i) state changes (ii) 128 bytes seen (iii) end input */

void writearowrun (FILE *output, unsigned char *s, unsigned long width)
{
  int runflag;      /* non-zero if in accumulating run state */
  int previous;     /* character that appears to be repeating */
  int repeat;       /* how many times we have seen previous */
  int bcount;       /* count of bytes buffered up (s-new) */
  unsigned char *new;   /* points to part of string not yet dumped */
  int n, k, c;

  n = (int) width;
  k = 0;
  bcount = 0;       /* non run length accumulated */
  new = s;        /* points to first byte not yet output */
  previous = -1;      /* previous character in a run */
  repeat = 0;       /* how many repetetions of previous */
  runflag = 0;      /* start in non run state */

  while (k < n) {
    c = *s;       /* grab next byte */
    if (runflag) {        /* accumulating a run ? */
      if (repeat >= 128) {  /* run too long ? */
        dumprun(output, repeat, previous);
        new = s;
        previous = c;
        repeat = 1;
        bcount = 1;
        runflag = 0;      /* switch back to non run state */
      }
      else if (c == previous) { /* continue the run ? */
        repeat++;
      }
      else {            /* c != previous --- end of a run */
        dumprun(output, repeat, previous);
        new = s;
        previous = c;
        repeat = 1;
        bcount = 1;
        runflag = 0;      /* switch back to non run state */
      }
    }
    else {            /* runflag == 0 accumulating a non run */
      if (bcount >= 128) {  /* accumulated too much ? */
        dumpnonrun(output, bcount, new);
        new = s;
        previous = c;
        bcount = 1;
        repeat = 1;
/*        and stay in non run mode */
      }
      else if (c == previous) {
        repeat++;
        if (repeat >= 4) {  /* end of non-run */
          if (bcount+1 > repeat)
            dumpnonrun(output, bcount-repeat+1, new);
          bcount = 0;         /* needed ? */
          runflag = 1;  /* switch to accumulating run state */
/*          don't change previous, repeat */
        }
        if (runflag == 0) bcount++;
      }
      else {
        bcount++;
        previous = c;
/*        don't change new, bcount */
        repeat = 1;
      }
    }
    s++;
    k++;
  } /* end of while loop */
#ifdef DEBUGRUNLENGTH
/*  if (debugflag) showline("EOL ", 0); */
#endif
  if (runflag) dumprun(output, repeat, previous);
  else if (repeat >= bcount  && repeat > 1)
    dumprun(output, repeat, previous);
  else dumpnonrun(output, bcount, new);
#ifdef DEBUGRUNLENGTH
  if (debugflag) showline("\n", 0);
#endif
}

/**************************************************************************/

/* code for LZW compression on OUTPUT side  LZWcompress rewrite 2000 June 17 */

#ifdef LZWCOMPRESSION

struct NODE {
  int chr;      // byte to get to this node --- or -1
//  NODE nextinlist;  // next in list at this level ptr
  int nextinlist;   // next in list at this level
//  NODE nextlevel;   // first at next level ptr
  int nextlevel;    // first at next level
};

// May get some added performance by making the last two components be
// pointers rather than indeces...

struct NODE *node=NULL;   // array of nodes --- allocated

int currentnode;  // code of node currently at
int nextnode;   // code of next node to be used
int codelength;   // how many bits needed for code at this stage 9, 10, 11, 12

void DeAllocStringsOut (void) // called from dvibody() in dvipsone.c
{
  if (node != NULL) {
    free(node);
    node = NULL;
  }
}

void CleanOut (FILE *output, int n)
{
  int k;

//  sprintf(logline, "CLEANOUT %d\n", n);
//  showline(logline, 0);

//  0 to MAXCHR, and CLEAR and EOD
  for (k = 0; k < FIRSTCODE; k++) {
    node[k].chr = -1;
    node[k].nextinlist = -1;
    node[k].nextlevel = -1;
  }
  LZWput(CLEAR, output);    // using current code length
  nextnode = FIRSTCODE;   // next node available
  currentnode = n;      // string in progress
  codelength = 9;       // reset code length now
}

int SetupNodes (FILE *output)
{
  int nlen;

  if (node == NULL) {
    nlen = MAXCODES * sizeof(struct NODE);  // 49,152 bytes
    if (traceflag) {
      sprintf(logline, "Allocating %d bytes\n", nlen);  // debugging only
      showline(logline, 0);
    }
    node = (struct NODE *) malloc(nlen);
    if (node == NULL) {
      sprintf(logline, "Unable to allocate %d bytes for NODE table\n", nlen);
      showline(logline, 1);
      checkexit(1);
    }
  }
//  LZWput(CLEAR, output);      // do separately
//  nextnode = FIRSTCODE;
//  currentnode = -1;
  codelength = 9;         // need to initialize
//  CleanOut(output, -1);     // do separately
  return 0;
}

// We are adding a new node for byte chr

void NewNode (FILE *output, int chr, int previous) {
  if (nextnode < 0 || nextnode >= MAXCODES) showline("TABLE OVERFLOW", 1);
//  set up new node being added
//  node[nextnode].code = nextnode;
//  sprintf(logline, "NEW NODE %d %d\n", chr, previous);
//  showline(logline, 1);
//  sprintf(logline, "NEW NODE %d for %d from %d ", nextnode, chr, previous);
//  showline(logline, 1);   // debugging only
  node[nextnode].chr = chr;
  node[nextnode].nextinlist = -1;
  node[nextnode].nextlevel = -1;
  nextnode++;
  if (nextnode == 512 || nextnode == 1024 || nextnode == 2048) {
//    sprintf(logline, "CODELENGTH %d ", codelength);
//    showline(logline, 0);
    codelength++;
  }
  else if (nextnode == 4096) CleanOut(output, chr);
}

// Process next incoming byte

void DoNextByte (FILE *output, int chr) { // called from  writearowLZW
  int k, klast;
//  sprintf(logline, "(%d) ", chr);   // debugging only
//  showline(logline, 0);
  if (currentnode < 0) {  // starting from scratch ? (empty string)
    currentnode = chr;
    return;
  }
  k = node[currentnode].nextlevel;
  if (k < 0) {  // does next level exist ?
//    AddaNode(currentnode, n);
    LZWput(currentnode, output);    // new string NOT in table
    node[currentnode].nextlevel = nextnode; // NEXT LEVEL
//    sprintf(logline, "NEW LEVEL %d %d %d ", currentnode, nextnode, chr);
//    showline(logline, 0);       // debugging only
    NewNode(output, chr, currentnode);  // new node at nextnode
    currentnode = chr;          // string of one byte
    return;
  }
//  there is a list at the next level --- search it
//  k = node[currentnode].nextlevel;
  while (k >= 0 && node[k].chr != chr) {
    klast = k;
    k = node[klast].nextinlist;
  }
  if (k < 0) {              // hit end of list ?
//    AddaNode(currentnode, n);
    LZWput(currentnode, output);    // new string NOT in table
    node[klast].nextinlist = nextnode;  // NEXT IN LIST
//    sprintf(logline, "NEW ELEM %d (%d) %d %d ", currentnode, klast, nextnode, chr);
//    showline(logline, 0);       // debugging only
    NewNode(output, chr, currentnode);  // new node at nextnode
    currentnode = chr;          // string of one byte
    return;
  }
//  we did find this string in the table
//  if (k < 0 || k >= MAXCODES) showline("ERROR ", 0);  // debugging only
  currentnode = k;  // it IS in table, no output
}

/***************************************************************************/

int perrow = 100;

long leftover;    /* byte accumulated so far - need up to 12 + 12 bits */
int bitsused;   /* bits used so far */

/* stuff codelength bits of c into output stream */
/* may produce one or two bytes ready to spit out */
/* bitsused is always < 8 before and always < 8 after this */

void LZWput (int code, FILE *output)
{
  int c;

//  if (bitsused < 0 || bitsused >= 8) showline("ERROR ", 1);
#ifdef DEBUGLZWENCODE
  if (debugflag)
  {
    if (code >= (1 << codelength))
    sprintf(logline, "code %d too long for code length %d\n",
        code, codelength);
    showline(logline, 1);
  }
#endif
//  sprintf(logline, "LEFT %08X USED %d ", leftover, bitsused);
//  showline(logline, 0);   // debugging only
  leftover = (leftover << codelength) | code;
  bitsused += codelength;
  if (bitsused >= 16) {
    c = (int) (leftover >> (bitsused - 8)); /* get left most 8 bits */
    ASCIIout(output, c & 255);      // expand ?
    bitsused -= 8;
  }
  c = (int) (leftover >> (bitsused - 8)); /* get left most 8 bits */
  ASCIIout(output, c & 255);        // expand ?
  bitsused -= 8;
//  if (bitsused < 0 || bitsused >= 8) showline("ERROR ", 1);
}

void LZWputinit (FILE *output) {  /* called from LZWinitfilter */
//  showline("LZWputinit ", 0); // debugging only
  leftover = 0;
  bitsused = 0;
}

void LZWputflush (FILE *output)
{
  int c;
//  showline("LZWputflush ", 0);  // debugging only
  if (bitsused == 0) return;    /* nothing left to push out */
  c = (int) (leftover << (8 - bitsused));
/*  c = (int) (leftover >> (bitsused - 8)); */  /* same thing ? */
  ASCIIout(output, c & 255);      /* fill last byte with zeros */
/*  leftover = 0; */
/*  bitsused = 0; */
}

void LZWinitfilter (FILE *output) /*  initialization  */
{
//  showline("LZWinitfilter ", 0);    // debugging only
  LZWputinit(output);
//  LZWput(CLEAR, output);  /* write CLEAR */
  SetupNodes(output);
  CleanOut(output, -1);
/*  codelength = 9; */
}

void LZWflushfilter (FILE *output) /* termination */
{
//  showline("LZWflushfilter ", 0);   // debugging only
  if (currentnode >= 0) {
    LZWput(currentnode, output);
#ifdef DEBUGLZWENCODE
    if (debugflag > 1) showcode(currentnode);
#endif
  }
  LZWput(EOD, output);
#ifdef DEBUGLZWENCODE
  if (debugflag) showline("EOD ", 1);
  if (debugflag) {
    sprintf(logline, "%d entries ", nextnode);
    showline(logline, 1);
  }
#endif
  LZWputflush(output);
/*  CleanOut(output, -1); */
}

/* Basic LZW encoding for output */ /* encode width bytes starting at *s */
/* Initialization and Termination taken care of elsewhere */
/* int code; */ /* code of the string matched so far to input */
/* int last; */ /* last character of input string */

int writearowLZW (FILE *output, unsigned char *s, unsigned long width)
{
  unsigned char *send = s + width;

/*  This picks up unfinished business --- currentnode >= 0 */
  while (s < send) {
    DoNextByte(output, *s++);
  }
  return 0;
/*  This may leave unfinished business --- currentnode >= 0 */
}

#endif /* end of ifdef LZWCOMPRESSION */

/*************************************************************************/

//#pragma optimize ("lge", off)

#endif /* end of ifdef PSLEVEL2 */

/* write row in hex format */

void writearowhex (FILE *output, unsigned char *s, unsigned long width)
{
  unsigned int c, d;
  int k, n;

  n = (int) width;
  column = 0;
  for (k = 0; k < n; k++) {
    if (column >= MAXCOLUMNHEX) {
//      putc('\n', output);
      PSputc('\n', output);
      column = 0;
    }
    c = *s++;
    d = c & 15;
    c = c >> 4;
    if (c > 9) {
//      putc(c + 'A' - 10, output);
      PSputc((char) (c + 'A' - 10), output);
    }
    else {
//      putc(c + '0', output);
      PSputc((char) (c + '0'), output);
    }
    column++;
    if (d > 9) {
//      putc(d + 'A' - 10, output);
      PSputc((char) (d + 'A' - 10), output);
    }
    else {
//      putc(d + '0', output);
      PSputc((char) (d + '0'), output);
    }
    column++;
  }
//  putc('\n', output);   /* also start each image row on new line */
  PSputc('\n', output);   /* also start each image row on new line */
  column = 0;
}

/* returns -1 if output error */  /* write a row of data in hex */

int writearow (FILE *output, unsigned char *s, unsigned long width)
{
#ifdef PSLEVEL2
  if (bLevel2) {
    if (bRunLengthFlag) writearowrun(output, s, width);
#ifdef LZWCOMPRESSION
    else if (bLZWFlag) writearowLZW(output, s, width);
#endif
    else writearowASCII(output, s, width);
  }
  else writearowhex(output, s, width);
#else
  writearowhex(output, s, width);
#endif

//  if (ferror(output)) 
  if (output != NULL && ferror(output))
  {
    showline(" ERROR: Output error\n", 1);
    perrormod((outputfile != NULL) ? outputfile : "");
    return -1;
  }
  if (bAbort) abortjob();   /* 97/Mar/1 ? */
  if (abortflag) return -1;
  return 0;
}

int computeheader (void) {
/*  int changeflag= 0; */

#ifdef DEBUGTIFF
  if (traceflag) showline("Now computing header information\n", 0);
#endif

/*  Now for some sanity checks */
  if (PlanarConfig != 1) {
    showline(" ERROR: Multiple color planes not supported\n", 1);
    return -1; 
  }
  if (BitsPerSample > 8) {
    sprintf(logline, " ERROR: Maximum of 8 bits per sample (%d)\n",
      BitsPerSample);
    showline(logline, 1);
    return -1;
  }
  if ((BitsPerSample & (BitsPerSample-1)) != 0) {
    sprintf(logline, " ERROR: Bits per sample must be power of two (%d)\n",
      BitsPerSample);
    showline(logline, 1);
    return -1;
  }
  if (compression > TIFF_CCITT && compression != LZW_COMPRESSION &&
    compression != PACK_BITS ) {
    sprintf(logline,  " ERROR: Unknown compression scheme (%d)", compression);
    showline(logline, 1);
    return -1;
  }

  if (PhotometricInterpretation == 3) { /* that is palette color */
/*    if (SamplesPerPixel != 1) */
    if (SamplesPerPixelX != 1) {
      showline(" ERROR: Palette color must have one sample per pixel", 1);
      return -1;
    }
  }
/*  check whether need to use `colorimage' operator instead of `image' */
/*  if (SamplesPerPixel != 1) */
  if (SamplesPerPixelX != 1) {
//    if (PhotometricInterpretation != 2)
    if (PhotometricInterpretation != 2 && PhotometricInterpretation != 5)
//      showline(" More than one sample per pixel, but not RGB?", 1);
      showline(" More than one sample per pixel, but not RGB or CMYK?", 1);
    bColorImage = 1;
    if (! bAllowColor) {
      sprintf(logline, " WARNING: More than one sample per pixel (%d) use `*c' flag?",
        SamplesPerPixel);
      showline(logline, 0);
/*      return -1; */
      bCompressColor = 1;     /* then need to compress colors */
    }
  }
/*  if (verboseflag && PhotometricInterpretation > 3) {
    printf("Photometricinterpretation %d\n", PhotometricInterpretation);
  }  */
/*  0 => 0 will be white and 2^n-1 black */
/*  1 => 0 will be black and 2^n-1 white (default) */
/*  2 => RGB model */
/*  3 => palette color */
/*  4 => transparency mask */
/*  Fix this one later */
/*  if (PhotometricInterpretation == 3) bColorImage = 1;  */
/*  if (ColorMapPtr != 0)   bColorImage = 1; */
  if (PhotometricInterpretation == 3 && ColorMapPtr == 0) {
    showline(" ERROR: Palette Color Image must have Palette!\n", 1);
    return -1;
  }
  if (PhotometricInterpretation == 2)
  {
/*    if (SamplesPerPixel == 1)  */
    if (SamplesPerPixelX == 1)
    {
      sprintf(logline, " %s, but not more than one sample per pixel?\n", "RGB");
      showline(logline, 1);
    }
    if (! bAllowColor)
    {
      sprintf(logline, " WARNING: %s color image (%d) use `*c' flag?", "RGB", SamplesPerPixel);
//      showline(logline, 1);
      showline(logline, 0);
      bCompressColor = 1;
    }
    bColorImage = 1;
  }
  if (PhotometricInterpretation == 5) { // 2000 May 27
/*    if (SamplesPerPixel == 1)  */
    if (SamplesPerPixelX == 1)
    {
      sprintf(logline, " %s, but not more than one sample per pixel?\n", "CMYK");
      showline(logline, 1);
    }
    if (! bAllowColor)
    {
      sprintf(logline, " WARNING: %s color image (%d) use `*c' flag?", "CMYK", SamplesPerPixel);
//      showline(logline, 1);
      showline(logline, 0);
      bCompressColor = 1;
    }
    bColorImage = 1;
  }
/*  0 => 0 will be white and 2^n-1 black */
/*  1 => 0 will be black and 2^n-1 white (default) */

  if (PhotometricInterpretation == 0)
  {
#ifdef DEBUGTIFF
    if (traceflag) showline("Image grey levels will be inverted\n", 0); 
#endif
    bInvertImage = 1;
  }
  else bInvertImage = 0;

#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "ExpandColor %d CompressColor %d InvertImage %d\n",
      bExpandColor, bCompressColor, bInvertImage);
    showline(logline, 0);
  }
#endif

/*  compute bytes per row (both input and output?) */
/*  if (bColorImage)bytes = (int) (((long) ImageWidth * BitsPerPixel + 7) / 8);
  else bytes = (int) (((long) ImageWidth * BitsPerSample + 7) / 8); */
/*  Following only correct if no expansion or contraction of color images */
/*  bytes = (int) (((long) ImageWidth * BitsPerPixel + 7) / 8); */
  bytes = (int) OutRowLength;     /* ever used ? */
#ifdef DEBUGTIFF
  if (traceflag) {
    sprintf(logline, "%d bytes per row\n", bytes);
    showline(logline, 0);
  }
#endif
  return 0;
}

/* long xll=0, yll=0, xur=0, yur=0;  */

/*  {1 exch sub} settransfer */
/*  currenttranser {1 exch sub 0 exec} exch 4 put bind */
/*  char *invertgray=
  "[{1 exch sub} /exec load currentransfer /exec load] cvx settransfer\n"; */

/* compress some of this code into DVIPREAM.PS ? */

void writepsheader (FILE *output, long dwidth, long dheight)
{
  int bits=1;         /* bits per pixel in image */
  int samples=3;        /* samples per pixel 96/July/7 */
  int bMonoChrome=0;      /* if need to use image mask */
/*  int paintback=0; */     /* if need to paint background */
/*  long XOffset, YOffset; */
  long YOffset;

#ifdef DEBUGTIFF
  if (traceflag) showline("Now writing out header information\n", 0);
#endif
/*  if (BitsPerSample == 1) bMonoChrome = 1; */ /* use image mask */

  if (bExpandColor) bits = 8;
  else if (bExpandGray) bits = 8;
  else bits = BitsPerSample;

  if (bits == 1) bMonoChrome = 1;       /* use image mask */

  if (bExpandColor) samples = 3;
  else if (bExpandGray) samples = 1;
/*  else samples = SamplesPerPixel; */      /* 96/July/7 */
  else samples = SamplesPerPixelX;

//  fprintf(output, "\nsave\n");
//  putc('\n', output);
  PSputc('\n', output);
//  fputs("save ", output);
  PSputs("save ", output);
  if (stripcomment == 0) {
    sprintf(logline, "%% %s\n", infilename);  /* 98/Jul/12 */
    PSputs(logline, output);
  }
  else {
//    putc('\n', output);
    PSputc('\n', output);
  }
  if (! bColorImage) {    /* only use screen for B/W */
/*    if (bInvertImage) fputs("invert\n", output); */
    if (bInvertImage && !bMonoChrome) {
//      fputs("invert\n", output);
      PSputs("invert\n", output);
    }
/*    if (wantmagic)  fputs(eps_magic, output); */
/*    fprintf(output, "currentscreen pop dviscreen\n"); */
    if (wantmagictiff) {
      if (frequency > 0) {  /* did user specified frequency & angle ? */
        sprintf(logline, "%d %d dviscreen\n", frequency, angle);
        PSputs(logline, output);
      }
      else {
//        fputs("currentscreen pop dviscreen\n", output);
        PSputs("currentscreen pop dviscreen\n", output);
      }
    }
  }
/*  for monochrome, paint background first, then use imagemask */
/*  if (BitsPerSample == 1) {
    if (bFigureColor) 
      fprintf(output, "%lg %lg %lg rgb\n",
        figurered, figuregreen, figureblue); 
    else if (bTextColor) fputs("black\n", output);
    paintback = 1;
  } */
/*  create the string before or after the `save' ? */
/*  fprintf(output, "/picstr %d string def\n", bytes);  */
/*  string not needed if using ASCII85Decode filter 96/Dec/20 */
  if (! bLevel2) {
    sprintf(logline, "/picstr %d string def\n", bytes);
    PSputs(logline, output);
  }
//  fprintf(output, "%d %d translate ", xll, yll);
//  fputs("currentpoint translate ", output);
  PSputs("currentpoint translate ", output);
  if (dwidth == 0 || dheight == 0) {
    showline(" Zero width or height ", 1);
  }
#ifdef ALLOWSCALE
  if (outscaleflag) {
    sprintf(logline, "%.9lg %.9lg scale\n",
      (double) dwidth / outscale, (double)  dheight / outscale);
  }
  else {
    sprintf(logline, "%ld %ld scale\n", dwidth, dheight);
  }
#else
  sprintf(logline, "%ld %ld scale\n", dwidth, dheight);
#endif
  PSputs(logline, output);

/*  if (bFigureColor && paintback) { */   /* need to paint background */
/*  proper support for \special{figurecolor: . . . . . .} 94/Mar/14 */
  if (BitsPerSample == 1) {       /* need to paint background ? */
/*    fputs("gsave\n", output); */    /* not needed */
/*    paint background here in background color */
/*    if (backred != 1.0 || backgreen != 1.0 || backblue != 1.0)  */
    if (bFigureColor) {
      sprintf(logline, "%lg %lg %lg rgb\n",
        backred, backgreen, backblue);    /* color for background */
      PSputs(logline, output);
    }
    else {
//      fputs("white\n", output);
      PSputs("white\n", output);
    }
/*  now paint background in this color *//* use `box' in preamble ? */
    if (bSuppressBack == 0) {
//      fputs("0 0 moveto 1 0 lineto ", output);
      PSputs("0 0 moveto 1 0 lineto ", output);
/*      fputs("0 0 moveto 1 0 lineto 1 -1 lineto 0 -1 lineto closepath fill\n", output); */
/*      background seems to be painted above not on image 95/Dec/31 */
/*      fputs("0 0 moveto 1 0 lineto 1 1 lineto 0 1 lineto closepath fill\n", output); */
      if (hptagflag != 0) {
//        fputs("1 1 lineto 0 1 lineto ", output);
        PSputs("1 1 lineto 0 1 lineto ", output);
      }
/*      background seems to be painted below not on image 96/May/6 */
      else {
//        fputs("1 -1 lineto 0 -1 lineto ", output);
        PSputs("1 -1 lineto 0 -1 lineto ", output);
      }
//      fputs("closepath fill\n", output);
      PSputs("closepath fill\n", output);
    }
    if (bFigureColor) {
      sprintf(logline, "%lg %lg %lg rgb\n",
        figurered, figuregreen, figureblue); /* color for foreground */
      PSputs(logline, output);
    }
    else {
//      fputs("black\n", output);
      PSputs("black\n", output);
    }
/*    fputs("grestore\n", output); */   /* not needed */
  }
//  fprintf(output, "%ld %ld %d\n", ImageWidth, ImageLength, bits);
  sprintf(logline, "%ld %ld ", ImageWidth, ImageLength);
  PSputs(logline, output);
  if (bMonoChrome) {
    if (bInvertImage) {
//      fputs("true\n", output);
      PSputs("true\n", output);
    }
    else {
//      fputs("false\n", output);
      PSputs("false\n", output);
    }
  }
  else {
    sprintf(logline, "%d\n", bits);       /* not monochrome */
    PSputs(logline, output);
  }
  if (BMPflag) {              /* new mode 98/Jul/9 */
    sprintf(logline, "[%ld 0 0 %ld 0 %ld]\n",
                ImageWidth, -ImageLength, 0);
    PSputs(logline, output);
  }
  else {
    if (hptagflag != 0) YOffset = 0;
    else YOffset = ImageLength;
    sprintf(logline, "[%ld 0 0 %ld 0 %ld]\n",
        ImageWidth, ImageLength, YOffset);
    PSputs(logline, output);
  }
/*  fputs("{currentfile picstr readhexstring pop} bind\n", output); */
  if (bLevel2) {
//    fputs("currentfile", output);
    PSputs("currentfile", output);
//    fputs(" /ASCII85Decode filter", output);
    PSputs(" /ASCII85Decode filter", output);
    if (bRunLengthFlag) {
//      fputs(" /RunLengthDecode filter", output);
      PSputs(" /RunLengthDecode filter", output);
    }
#ifdef LZWCOMPRESSION
    else if (bLZWFlag) {
//      fputs(" /LZWDecode filter", output);
      PSputs(" /LZWDecode filter", output);
    }
#endif
//    putc('\n', output);
    PSputc('\n', output);
  }
  else {
//    fputs("{currentfile picstr readhexstring pop} bind\n", output);
    PSputs("{currentfile picstr readhexstring pop} bind\n", output);
  }
/*  samples per pixel better be 1 (grey), 3 (RGB) or 4 (CMYK) */
  if (bColorImage && bAllowColor) {
//    fprintf(output, "false %d colorimage\n", SamplesPerPixel); 
    sprintf(logline, "false %d colorimage", samples); /* 96/July/7 */
    PSputs(logline, output);
  }
/*  else if (bCompressColor) fputs("image\n", output); */
  else if (bMonoChrome) {
//    fputs("imagemask", output); /* monochrome */
    PSputs("imagemask", output);  /* monochrome */
  }
  else {
//    fputs("image", output); /* either not color, or color compressed */
    PSputs("image", output);  /* either not color, or color compressed */
  }
//  putc('\n', output);
  PSputc('\n', output);
/*  image data must follow right away */
}

void writepstrailer (FILE *output)
{
#ifdef DEBUGTIFF
  if (traceflag) showline("Now writing trailer\n", 0);
#endif
/*  if (BitsPerSample == 1) {
    if (bTextColor) fprintf(output, "%lg %lg %lg rgb\n",
          Textred, Textgreen, Textblue);
    else if(bFigureColor) fputs("black\n");
  } */              /* monochrome - not needed */
//  fputs("restore\n", output);
  PSputs("restore\n", output);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Read a row using PACK_BITS compression scheme */
/* returns non-zero if problems encountered */

int readpackbits (unsigned char *lpBuffer, FILE *input, int RowLength)
{
  unsigned char *u=lpBuffer;
  int c, k, n, total=0, flag=0;
  
  for(;;) {
    if ((n = getc(input)) < 0) {  /* premature EOF */
      showline(" Premature EOF", 1);
      flag = -1;
      break;  
    }
    else if (n < 128) {     /* use next (n+1) bytes as is */
      for (k=0; k < n+1; k++) *u++ = (char) getc(input);  
      total += n+1;
    }
    else if (n > 128) {     /* repeat next byte (257 - n) times */
      c = getc(input);
      for (k=0; k < (257 - n); k++) *u++ = (char) c;   
      total += (257 - n);
    }
/*    and n == 128 is a NOP */
    if (total == RowLength) break;  /* enough bytes yet ? */
    if (total > RowLength) {    /* too many bytes ? */
      showline(" Too many bytes in compressed row\n", 1);
      flag = -1;
      break;
    }
  }
  return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Implement 1d CCITT Huffman code decompression (minus EOL) */
/* ==> implies BitsPerSample == 1 */
/* white runs = 0, black runs = 1 */
/* normal PhotometricInterpretation == 0 */
/* invert black/white if PhotometricInterpretation == 1 */

int bytex, bitsleft;        /* input buffering in splitting bits */

int getbit (FILE *input)
{
  if (bitsleft-- <= 0) {
    if ((bytex = getc(input)) == EOF) {
      sprintf(logline, " Unexpected EOF (%s)\n", "getbit TIFF");
      showline(logline, 0);
/*      checkexit(1); */      /* Wait for it to create bad code */
    }
    bitsleft = 7;
  }
  bytex = bytex << 1;
#ifdef DEBUGTIFF
  if (traceflag) {
    sprintf(logline, "%d", (bytex & 256) ? 1 : 0);
    showline(logline, 0);
  }
#endif
  if (bytex & 256) return 1;
  else return 0;
}

//#pragma optimize ("lge", on)

/* It's the Huffman code stuff that wants to be not optimized */

//#pragma optimize ("lge", off)

/* Actually commonmake may be OK with compiler optimizations ON */

int commonmake (FILE *input) /* common black/white make up codes (7 zeros) */
{
#ifdef DEBUGTIFF
  if (traceflag) {
    showline("commonmake entry ", 0);
  }
#endif
  if (getbit(input)) {  /* 00000001 */
    if (getbit(input)) {  /* 000000011 */
      if (getbit(input)) {  /* 0000000111 */
        if (getbit(input)) {  /* 00000001111 */
          if (getbit(input)) {  /* 000000011111 */
            return 2560;
          }
          else { /* 000000011110 */
            return 2496;
          }
        }
        else { /* 00000001110 */
          if (getbit(input)) {  /* 000000011101 */
            return 2432;
          }
          else { /* 000000011100 */
            return 2368;
          }
        }
      }
      else { /* 0000000110 */
        if (getbit(input)) {  /* 00000001101 */
          return 1920;
        }
        else { /* 00000001100 */
          return 1856;
        }
      }
    }
    else { /* 000000010 */
      if (getbit(input)) {  /* 0000000101 */
        if (getbit(input)) {  /* 00000001011 */
          if (getbit(input)) {  /* 000000010111 */
            return 2304;
          }
          else { /* 000000010110 */
            return 2240;
          }
        }
        else { /* 00000001010 */
          if (getbit(input)) {  /* 000000010101 */
            return 2176;
          }
          else { /* 000000010100 */
            return 2112;
          }
        }
      }
      else { /* 0000000100 */
        if (getbit(input)) {  /* 00000001001 */
          if (getbit(input)) {  /* 000000010011 */
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
/*  Actually, EOL code is not supposed to be used in TIFF compression 2 */
    if (!getbit(input)) { /* 000000000 */
      if (!getbit(input)) { /* 0000000000 */
        if (!getbit(input)) { /* 00000000000 */
          if (getbit(input)) { /* 000000000001 */
            showline("EOL ", 1);
            return 0;       /* ??? */
          }
        }
      }
    }
  }
  showline(" Impossible make-up run\n", 1);
  return -1;  /* error */
}

/* Compiler screws up the following code if optimizations turned on */

int whiterun (FILE *input)
{
#ifdef DEBUGTIFF
  if (traceflag)
  {
    showline("whiterun entry ", 0);
  }
#endif
  if (getbit(input)) {  /* 1 */
    if (getbit(input)) {  /* 11 */
      if (getbit(input)) {  /* 111 */
        if (getbit(input)) {  /* 1111 */
          return 7;
        }
        else {      /* 1110 */
          return 6;
        }
      }
      else {      /* 110 */
        if (getbit(input)) {  /* 1101 */
          if (getbit(input)) { /* 11011 */
            return 64;    /* make up */
          }
          else {      /* 11010 */
            if (getbit(input)) { /* 110101 */
              return 15;
            }
            else {      /* 110100 */
              return 14;
            }
          }
        }
        else {      /* 1100 */
          return 5;
        }
      }
    }
    else {      /* 10 */
      if (getbit(input)) {  /* 101 */
        if (getbit(input)) {  /* 1011 */
          return 4;
        }
        else {      /* 1010 */
          if (getbit(input)) { /* 10101 */
            if (getbit(input)) { /* 101011 */
              return 17;
            }
            else {      /* 101010 */
              return 16;
            }
          }
          else {      /* 10100 */
            return 9;
          }
        }
      }
      else {      /* 100 */
        if (getbit(input)) {  /* 1001 */
          if (getbit(input)) { /* 10011 */
            return 8;
          }
          else {      /* 10010 */
            return 128; /* make up */
          }
        }
        else {      /* 1000 */
          return 3;
        }
      }
    }
  }
  else {      /* 0 */
    if (getbit(input)) {  /* 01 */
      if (getbit(input)) {  /* 011 */
        if (getbit(input)) {  /* 0111 */
          return 2;
        }
        else {      /* 0110 */
          if (getbit(input)) { /* 01101 */
            if (getbit(input)) { /* 011011 */
              if (getbit(input)) { /* 0110111 */
                return 256; /* make up */
              }
              else {      /* 0110110 */
                if (getbit(input)) {  /* 01101101 */
                  if (getbit(input)) {  /* 011011011 */
                    return 1408;  /* make up */
                  }
                  else { /*  011011010 */
                    return 1344;  /* make up */
                  }
                }
                else {      /* 01101100 */
                  if (getbit(input)) {  /* 011011001 */
                    return 1280;  /* make up */
                  }
                  else { /* 011011000 */
                    return 1216;  /* make up */
                  }
                }
              }
            }
            else { /* 011010 */
              if (getbit(input)) { /* 0110101 */
                if (getbit(input)) { /* 01101011 */
                  if (getbit(input)) { /* 011010111 */
                    return 1152;  /* make up */
                  }
                  else {      /* 011010110 */
                    return 1088;  /* make up */
                  }
                }
                else { /* 01101010 */
                  if (getbit(input)) { /* 011010101 */
                    return 1024;  /* make up */
                  }
                  else { /* 011010100 */
                    return 960; /* make up */
                  }
                }
              }
              else { /* 0110100 */
                if (getbit(input)) { /* 01101001 */
                  if (getbit(input)) { /* 011010011 */
                    return 896; /* make up */
                  }
                  else { /* 011010010 */
                    return 832; /* make up */
                  }
                }
                else { /* 01101000 */
                  return 576; /* make up */
                }
              }
            }
          }
          else { /* 01100 */
            if (getbit(input)) { /* 011001 */
              if (getbit(input)) { /* 0110011 */
                if (getbit(input)) { /* 01100111 */
                  return 640; /* make up */
                }
                else { /* 01100110 */
                  if (getbit(input)) { /* 011001101 */
                    return 768; /* make up */
                  }
                  else { /* 011001100 */
                    return 704; /* make up */
                  }
                }
              }
              else { /* 0110010 */
                if (getbit(input)) { /* 01100101 */
                  return 512; /* make up */
                }
                else { /* 01100100 */
                  return 448; /* make up */
                }
              }
            }
            else { /* 011000 */
              return 1664;  /* make up */
            }
          }
        }
      }
      else {      /* 010 */
        if (getbit(input)) {  /* 0101 */
          if (getbit(input)) { /* 01011 */
            if (getbit(input)) { /* 010111 */
              return 192;   /* make up */
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
          else {      /* 01010 */
            if (getbit(input)) { /* 010101 */
              if (getbit(input)) { /* 0101011 */
                return 25;
              }
              else {      /* 0101010 */
                if (getbit(input)) { /* 01010101 */
                  return 52;
                }
                else { /* 01010100 */
                  return 51;
                }
              }
            }
            else {      /* 010100 */
              if (getbit(input)) { /* 0101001 */
                if (getbit(input)) { /* 01010011 */
                  return 50;
                }
                else { /* 01010010 */
                  return 49;
                }
              }
              else {      /* 0101000 */
                return 24;
              }
            }
          }
        }
        else {      /* 0100 */
          if (getbit(input)) {  /* 01001 */
            if (getbit(input)) { /* 010011 */
              if (getbit(input)) { /* 0100111 */
                return 18;
              }
              else {      /* 0100110 */
                if (getbit(input)) {  /* 01001101 */
                  if (getbit(input)) {  /* 010011011 */
                    return 1728;  /* make up */
                  }
                  else { /* 010011010 */
                    return 1600;  /* make up */
                  }
                }
                else { /* 01001100 */
                  if (getbit(input)) {  /* 010011001 */
                    return 1536;  /* make up */
                  }
                  else { /* 010011000 */
                    return 1472;  /* make up */
                  }
                }
              }
            }
            else {      /* 010010 */
              if (getbit(input)) { /* 0100101 */
                if (getbit(input)) { /* 01001011 */
                  return 60;
                }
                else { /* 01001010 */
                  return 59;
                }
              }
              else {      /* 0100100 */
                return 27;
              }
            }
          }
          else {      /* 01000 */
            return 11;
          }
        }
      }
    }
    else {      /* 00 */
      if (getbit(input)) {  /* 001 */
        if (getbit(input)) {  /* 0011 */
          if (getbit(input)) {  /* 00111 */
            return 10;
          }
          else {      /* 00110 */
            if (getbit(input)) { /* 001101 */
              if (getbit(input)) { /* 0011011 */
                if (getbit(input)) {  /* 0110111 */
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
            else {      /* 001100 */
              if (getbit(input)) { /* 0011001 */
                if (getbit(input)) { /* 00110011 */
                  return 62;
                }
                else { /* 00110010 */
                  return 61;
                }
              }
              else {      /* 0011000 */
                return 28;
              }
            }
          }
        }
        else {      /* 0010 */
          if (getbit(input)) { /* 00101 */
            if (getbit(input)) { /* 001011 */
              if (getbit(input)) { /* 0010111 */
                return 21;
              }
              else {      /* 0010110 */
                if (getbit(input)) { /* 00101101 */
                  return 44;
                }
                else { /* 00101100 */
                  return 43;
                }
              }
            }
            else {      /* 001010 */
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
          else {      /* 00100 */
            if (getbit(input)) { /* 001001 */
              if (getbit(input)) { /* 0010011 */
                return 26;
              }
              else {      /* 0010010 */
                if (getbit(input)) { /* 00100101 */
                  return 54;
                }
                else { /* 00100100 */
                  return 53;
                }
              }
            }
            else {      /* 001000 */
              return 12;
            }
          }
        }
      }
      else {    /* 000 */
        if (getbit(input)) {  /* 0001 */
          if (getbit(input)) {  /* 00011 */
            if (getbit(input)) { /* 000111 */
              return 1;
            }
            else {      /* 000110 */
              if (getbit(input)) { /* 0001101 */
                if (getbit(input)) { /* 00011011 */
                  return 32;
                }
                else { /* 00011010 */
                  return 31;
                }
              }
              else {    /* 0001100 */
                return 19;
              }
            }
          }
          else {    /* 00010 */
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
            else {      /* 000100 */
              if (getbit(input)) { /* 0001001 */
                if (getbit(input)) { /* 00010011 */
                  return 34;
                }
                else { /* 00010010 */
                  return 33;
                }
              }
              else {      /* 0001000 */
                return 20;
              }
            }
          }
        }
        else {    /* 0000 */
          if (getbit(input)) { /* 00001 */
            if (getbit(input)) { /* 000011 */
              return 13;
            }
            else {      /* 000010 */
              if (getbit(input)) { /* 0000101 */
                if (getbit(input)) { /* 00001011 */
                  return 48;
                }
                else { /* 00001010 */
                  return 47;
                }
              }
              else {      /* 0000100 */
                return 23;
              }
            }
          }
          else {    /* 00000 */
            if (getbit(input)) { /* 000001 */
              if (getbit(input)) { /* 0000011 */
                return 22;
              }
              else {      /* 0000010 */
                if (getbit(input)) { /* 00000101 */
                  return 46;
                }
                else { /* 00000100 */
                  return 45;
                }
              }
            }
            else {      /* 000000 */
              if (getbit(input)) { /* 0000001 */
                if (getbit(input)) { /* 00000011 */
                  return 30;
                }
                else { /* 00000010 */
                  return 29;
                }
              }
              else { /* 0000000 */
                return commonmake(input); /* seven zeros */
              }
            }
          }
        }
      }
    }
  }
  showline(" Impossible white run\n", 1);
  return -1;  /* error */
}

/* Compiler screws up the following code if optimizations turned on */

int blackzero (FILE *input)   /* black run code starts with four zeros */
{
#ifdef DEBUGTIFF
  if (traceflag) showline (" blackzero entry ", 0);
#endif
  if (getbit(input)) { /* 00001 */
    if (getbit(input)) { /* 000011 */
      if (getbit(input)) { /* 0000111 */
        return 12;
      }
      else {      /* 0000110 */
        if (getbit(input)) {  /* 00001101 */
          if (getbit(input)) { /* 000011011 */
            if (getbit(input)) { /* 0000110111 */
              return 0;
            }
            else {      /* 0000110110 */
              if (getbit(input)) { /* 00001101101 */
                if (getbit(input)) { /* 000011011011 */
                  return 43;
                }
                else {      /* 000011011010 */
                  return 42;
                }
              }
              else {      /* 00001101100 */
                return 21;
              }
            }
          }
          else {      /* 000011010 */
            if (getbit(input)) { /* 0000110101 */
              if (getbit(input)) { /* 00001101011 */
                if (getbit(input)) { /* 000011010111 */
                  return 39;
                }
                else {      /* 000011010110 */
                  return 38;
                }
              }
              else {      /* 00001101010 */
                if (getbit(input)) { /* 000011010101 */
                  return 37;
                }
                else {      /* 000011010100 */
                  return 36;
                }
              }
            }
            else {      /* 0000110100 */
              if (getbit(input)) { /* 00001101001 */
                if (getbit(input)) { /* 000011010011 */
                  return 35;
                }
                else { /* 000011010010 */
                  return 34;
                }
              }
              else {      /* 00001101000 */
                return 20;
              }
            }
          }
        }
        else {      /* 00001100 */
          if (getbit(input)) {  /* 000011001 */
            if (getbit(input)) { /* 0000110011 */
              if (getbit(input)) { /* 00001100111 */
                return 19;
              }
              else {      /* 00001100110 */
                if (getbit(input)) { /* 000011001101 */
                  return 29;
                }
                else {      /* 000011001100 */
                  return 28;
                }
              }
            }
            else {      /* 0000110010 */
              if (getbit(input)) { /* 00001100101 */
                if (getbit(input)) { /* 000011001011 */
                  return 27;
                }
                else {      /* 000011001010 */
                  return 26;
                }
              }
              else {      /* 00001100100 */
                if (getbit(input)) { /* 000011001001 */
                  return 192; /* make up */
                }
                else {      /* 000011001000 */
                  return 128; /* make up */
                }
              }
            }
          }
          else {      /* 000011000 */
            return 15;
          }
        }
      }
    }
    else {      /* 000010 */
      if (getbit(input)) { /* 0000101 */
        return 11;
      }
      else {      /* 0000100 */
        return 10;
      }
    }
  }
  else {    /* 00000 */
    if (getbit(input)) { /* 000001 */
      if (getbit(input)) { /* 0000011 */
        if (getbit(input)) {  /* 00000111 */
          return 14;
        }
        else {      /* 00000110 */
          if (getbit(input)) { /* 000001101 */
            if (getbit(input)) { /* 0000011011 */
              if (getbit(input)) { /* 00000110111 */
                return 22;
              }
              else {      /* 00000110110 */
                if (getbit(input)) { /* 000001101101 */
                  return 41;
                }
                else {      /* 000001101100 */
                  return 40;
                }
              }
            }
            else {      /* 0000011010 */
              if (getbit(input)) {  /* 00000110101 */
                if (getbit(input)) { /* 000001101011 */
                  return 33;
                }
                else {      /* 000001101010 */
                  return 32;
                }
              }
              else {      /* 00000110100 */
                if (getbit(input)) { /* 000001101001 */
                  return 31;
                }
                else {      /* 000001101000 */
                  return 30;
                }
              }
            }
          }
          else {      /* 000001100 */
            if (getbit(input)) { /* 0000011001 */
              if (getbit(input)) { /* 00000110011 */
                if (getbit(input)) { /* 000001100111 */
                  return 63;
                }
                else {      /* 000001100110 */
                  return 62;
                }
              }
              else {      /* 00000110010 */
                if (getbit(input)) { /* 000001100101 */
                  return 49;
                }
                else {      /* 000001100100 */
                  return 48;
                }
              }
            }
            else {      /* 0000011000 */
              return 17;
            }
          }
        }
      }
      else {      /* 0000010 */
        if (getbit(input)) {  /* 00000101 */
          if (getbit(input)) { /* 000001011 */
            if (getbit(input)) { /* 0000010111 */
              return 16;
            }
            else {      /* 0000010110 */
              if (getbit(input)) { /* 00000101101 */
                if (getbit(input)) { /* 000001011011 */
                  return 256;   /* make up */
                }
                else {      /* 000001011010 */
                  return 61;
                }
              }
              else {      /* 00000101100 */
                if (getbit(input)) { /* 000001011001 */
                  return 58;
                }
                else {      /* 000001011000 */
                  return 57;
                }
              }
            }
          }
          else {      /* 000001010 */
            if (getbit(input)) { /* 0000010101 */
              if (getbit(input)) { /* 00000101011 */
                if (getbit(input)) { /* 000001010111 */
                  return 47;
                }
                else {      /* 000001010110 */
                  return 46;
                }
              }
              else {      /* 00000101010 */
                if (getbit(input)) { /* 000001010101 */
                  return 45;
                }
                else {      /* 000001010100 */
                  return 44;
                }
              }
            }
            else {      /* 0000010100 */
              if (getbit(input)) { /* 00000101001 */
                if (getbit(input)) { /* 000001010011 */
                  return 51;
                }
                else {      /* 000001010010 */
                  return 50;
                }
              }
              else {      /* 00000101000 */
                return 23;
              }
            }
          }
        }
        else {      /* 00000100 */
          return 13;
        }
      }
    }
    else {      /* 000000 */
      if (getbit(input)) { /* 0000001 */
        if (getbit(input)) { /* 00000011 */
          if (getbit(input)) { /* 000000111 */
            if (getbit(input)) { /* 0000001111 */
              return 64;  /* make up */
            }
            else {      /* 0000001110 */
              if (getbit(input)) { /* 00000011101 */
                if (getbit(input)) { /* 000000111011 */
                  if (getbit(input)) { /* 0000001110111 */
                    return 1216;
                  }
                  else {      /* 0000001110110 */
                    return 1152;
                  }
                }
                else {      /* 000000111010 */
                  if (getbit(input)) { /* 0000001110101 */
                    return 1088;
                  }
                  else {      /* 0000001110100 */
                    return 1024;
                  }

                }
              }
              else {      /* 00000011100 */
                if (getbit(input)) { /* 000000111001 */
                  if (getbit(input)) { /* 0000001110011 */
                    return 960;
                  }
                  else {      /* 0000001110010 */
                    return 896;
                  }
                }
                else {      /* 000000111000 */
                  return 54;
                }
              }
            }
          }
          else {      /* 000000110 */
            if (getbit(input)) { /* 0000001101 */
              if (getbit(input)) { /* 00000011011 */
                if (getbit(input)) { /* 000000110111 */
                  return 53;
                }
                else {      /* 000000110110 */
                  if (getbit(input)) { /* 0000001101101 */
                    return 576; /*make up */
                  }
                  else {      /* 0000001101100 */
                    return 512; /* make up */
                  }
                }
              }
              else {      /* 00000011010 */
                if (getbit(input)) { /* 000000110101 */
                  return 448; /* make up */
                }
                else {      /* 000000110100 */
                  return 384; /* make up */
                }
              }
            }
            else {      /* 0000001100 */
              if (getbit(input)) { /* 00000011001 */
                if (getbit(input)) { /* 000000110011 */
                  return 320; /* make up */
                }
                else {      /* 000000110010 */
                  if (getbit(input)) { /* 0000001100101 */
                    return 1728;
                  }
                  else {      /* 0000001100100 */
                    return 1664;
                  }
                }
              }
              else {      /* 00000011000 */
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
              else {      /* 00000010110 */
                if (getbit(input)) { /* 000000101101 */
                  if (getbit(input)) { /* 0000001011011 */
                    return 1600;
                  }
                  else {      /* 0000001011010 */
                    return 1536;
                  }
                }
                else {      /* 000000101100 */
                  return 60;
                }
              }
            }
            else {      /* 0000001010 */
              if (getbit(input)) { /* 00000010101 */
                if (getbit(input)) { /* 000000101011 */
                  return 59;
                }
                else {      /* 000000101010 */
                  if (getbit(input)) { /* 0000001010101 */
                    return 1472;
                  }
                  else {      /* 0000001010100 */
                    return 1408;
                  }
                }
              }
              else {      /* 00000010100 */
                if (getbit(input)) { /* 000000101001 */
                  if (getbit(input)) { /* 0000001010011 */
                    return 1344;
                  }
                  else {      /* 0000001010010 */
                    return 1280;
                  }
                }
                else {      /* 000000101000 */
                  return 56;
                }
              }
            }
          }
          else {      /* 000000100 */
            if (getbit(input)) { /* 0000001001 */
              if (getbit(input)) { /* 00000010011 */
                if (getbit(input)) { /* 000000100111 */
                  return 55;
                }
                else {      /* 000000100110 */
                  if (getbit(input)) { /* 0000001001101 */
                    return 832;
                  }
                  else {      /* 0000001001100 */
                    return 768;
                  }
                }
              }
              else {      /* 00000010010 */
                if (getbit(input)) { /* 000000100101 */
                  if (getbit(input)) { /* 0000001001011 */
                    return 704;
                  }
                  else {      /* 0000001001010 */
                    return 640;
                  }
                }
                else {      /* 000000100100 */
                  return 52;
                }
              }
            }
            else {      /* 0000001000 */
              return 18;
            }
          }
        }
      }
      else { /* 0000000 */
        return commonmake(input); /* seven zeros */
      }
    }
  }
  showline(" Impossible black run", 1);
  showline(" (starting with four zeros)\n", 1);
  return -1;  /* error */
}

/* blackrun may actually be OK with compiler optimizations turned on */

int blackrun (FILE *input)
{
#ifdef DEBUGTIFF
  if (traceflag)
  {
    showline ("blackrun entry ", 0);
  }
#endif
  if (getbit(input)) {  /* 1 */
    if (getbit(input)) {  /* 11 */
      return 2;
    }
    else {      /* 10 */
      return 3;
    }
  }
  else {      /* 0 */
    if (getbit(input)) {  /* 01 */
      if (getbit(input)) {  /* 011 */
        return 4;
      }
      else {      /* 010 */
        return 1;
      }
    }
    else {      /* 00 */
      if (getbit(input)) {  /* 001 */
        if (getbit(input)) {  /* 0011 */
          return 5;
        }
        else {      /* 0010 */
          return 6;
        }
      }
      else {    /* 000 */
        if (getbit(input)) {  /* 0001 */
          if (getbit(input)) {  /* 00011 */
            return 7;
          }
          else {    /* 00010 */
            if (getbit(input)) { /* 000101 */
              return 8;
            }
            else {      /* 000100 */
              return 9;
            }
          }
        }
        else {    /* 0000 */
          return blackzero(input);  /* four zeros */
        }
      }
    }
  }
  showline(" ERROR: Impossible black run", 1);
  showline("\n", 0);
  return -1;  /* error */
}

//#pragma optimize ("lge", on)

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* FOLLOWING IS EXPERIMENT 97/OCT/21 */

//#pragma optimize ("lge", off)

int index, bitinx;  /* index into row of bytes and bits within them */

int total;      /* maybe long ? */

/* Following could be speeded up some ... */

int writewhite (unsigned char *lpBuffer, int run, int width)
{
  if (run == 0) return 0;     /* nothing to do */
  if (run < 0) return -1;     /* hit invalid run */
  total += run;
#ifdef DEBUGTIFF
  if (traceflag) {
    sprintf(logline, "run %d total %d\n", run, total);
    showline(logline, 0);
  }
#endif
  if (total > width) return -1; /* error */
/*  just advance pointers */
  while (run > 0) {
    if (bitinx == 0) {
      while (run >= 8) {  
        index++;
#ifdef DEBUGTIFF
        if (traceflag) {
          sprintf(logline, "index %d run %d ", index, run);
          showline(logline, 0);
        }
#endif
/*        *(lpBuffer+index) = 0; */ /* already zeroed out */
        run -= 8;
      }
    }
    if (run > 0) {
      if (bitinx-- <=  0) {
        index++; bitinx = 7;
      }
#ifdef DEBUGTIFF
      if (traceflag) {
        sprintf(logline, "index %d bitinx %d ", index, bitinx);
        showline(logline, 0);
      }
#endif
/*      *(lpBuffer + index) &= ~(1 << bitinx); */
      run--;
    }
  }
  if (total == width) return 1; /* EOL */
  else return 0;
}

int writeblack (unsigned char *lpBuffer, int run, int width)
{
  if (run == 0) return 0;     /* nothing to do */
  if (run < 0) return -1;     /* hit invalid run */
  total += run;
#ifdef DEBUGTIFF
  if (traceflag) {
    sprintf(logline, "run %d total %d\n", run, total);
    showline(logline, 0);
  }
#endif
  if (total > width) return -1; /* error */
  while (run > 0) {
    if (bitinx == 0) {
      while (run >= 8) {  
        index++;
#ifdef DEBUGTIFF
        if (traceflag) {
          sprintf(logline, "index %d run %d ", index, run);
          showline(logline, 0);
        }
#endif
        *(lpBuffer+index) = 255;  /* write a byte at a time */
        run -= 8;
      }
    }
    if (run > 0) {
      if (bitinx-- <=  0) {
        index++; bitinx = 7;
      }
#ifdef DEBUGTIFF
      if (traceflag) {
        sprintf(logline, "index %d bitinx %d ", index, bitinx);
        showline(logline, 0);
      }
#endif
      *(lpBuffer + index) |= (1 << bitinx); /* write a bit at a time */
      run--;
    }
  }
  if (total == width) return 1; /* EOL */
  else return 0;
}

/* make width long ? */

int huffmanrow (unsigned char *lpBuffer, FILE *input, int width)
{
  int k, bytes;
  int run;
/*  int total = 0; */             /* long total ? */

/*  if (lpBuffer == NULL) {
    showline(" Bad buffer pointer\n", 1);
    return -1;
  } */
  total = 0;
  index = -1; bitinx = 0;   /* output buffering */
  bytex = 0; bitsleft = 0;    /* input buffering */

  bytes = (width + 7) / 8;  /* preset with zeros */
#ifdef DEBUGTIFF
  if (traceflag) {
    sprintf(logline, "Cleaning out %d bytes\n", bytes);
    showline(logline, 0);
  }
#endif
  for (k = 0; k < bytes; k++) lpBuffer[k] = 0;

  for (;;) {
#ifdef DEBUGTIFF
    if (traceflag) {
      sprintf(logline, "Looking for white run\n");
      showline(logline, 0);
    }
#endif
    while ((run = whiterun (input)) >= 64) {
#ifdef DEBUGTIFF
      if (traceflag) {
        sprintf(logline, " W %d ", run);
        showline(logline, 0);
      }
#endif
      if (writewhite(lpBuffer, run, width) < 0) break;
    }
    if (total >= width) break;
#ifdef DEBUGTIFF
    if (traceflag) {
      sprintf(logline, " W %d\n", run);
      showline(logline, 0);
    }
#endif
    if (writewhite(lpBuffer, run, width) != 0) break;  /* terminal run */

#ifdef DEBUGTIFF
    if (traceflag) {
      sprintf(logline, "Looking for black run\n");
      showline(logline, 0);
    }
#endif
    while ((run = blackrun (input)) >= 64) {
#ifdef DEBUGTIFF
      if (traceflag) {
        sprintf(logline, " B %d ", run);
        showline(logline, 0);
      }
#endif
      if (writeblack(lpBuffer, run, width) < 0) break;
    }
    if (total >= width) break;
#ifdef DEBUGTIFF
    if (traceflag) {
      sprintf(logline, " B %d\n", run);
      showline(logline, 0);
    }
#endif
    if (writeblack(lpBuffer, run, width) != 0) break;  /* terminal run */
  }
  if (total != width) {
    sprintf(logline, " Sum of runs %d not equal width %d\n", total, width);
    showline(logline, 1);
    return -1;
  }
#ifdef DEBUGTIFF
  else if (traceflag ) {
    sprintf(logline, "Sum of runs equal to width %d\n", width);
    showline(logline, 0);
  }
#endif
  return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

//#pragma optimize ("lge", on)    /* experiment 2000 June 17 */

/* Here is the code for LZW (compression scheme number 5) input side */

char *StringByte=NULL;      /* contains last byte of string */

char *StringFirst=NULL;     /* contains first byte of string */
                    /* to speed up processing ... */

#ifdef USESHORTINT
short int *StringPrevious=NULL; /* points to previous char of string */
short int *StringLength=NULL;   /* length of string */
                    /* to speed up processing ... */
#else
int *StringPrevious=NULL;     /* points to previous char of string */
int *StringLength=NULL;     /* length of string */
                    /* to speed up processing ... */
#endif

int TableIndex=FIRSTCODE;         /* index into above String Tables */
                    /* next available string entry */

int CodeLength=9;           /* current code length INPUT */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following is never called ? now called from dvipsone.c when exiting */

void DeAllocStringsIn (void)   /* remember to do this at end */
{
/*  if (Memory != NULL) {
    free(Memory); Memory = NULL;
  } */
  if (StringByte != NULL) {
    free(StringByte); StringByte = NULL;
  }
  if (StringFirst != NULL) {
    free(StringFirst); StringFirst = NULL;
  }
  if (StringPrevious != NULL) {
    free(StringPrevious); StringPrevious = NULL;
  }
  if (StringLength != NULL) {
    free(StringLength); StringLength = NULL;
  }
}

/* worry here about allocation in NT ? sizeof(int) */

int AllocStringsIn (void)
{
/*  if (Memory == NULL) {
    Memory = (char *) malloc(INIMEMORY);
    MemorySize = INIMEMORY;
  } */ /* memory for strings in string table */
  if (StringByte == NULL) { /* allocate string table indeces */
    StringByte = (char *) malloc(MAXCODES * sizeof(char));
  }
  if (StringFirst == NULL) {  /* allocate string table indeces */
    StringFirst = (char *) malloc(MAXCODES * sizeof(char));
  }
/*  should this be short int in NT ? */
#ifdef USESHORTINT
  if (StringPrevious == NULL) { /* allocate string table lengths */
    StringPrevious = (short int *)
             malloc(MAXCODES * sizeof(short int));
  }
  if (StringLength == NULL) { /* allocate string table lengths */
    StringLength = (short int *)
             malloc(MAXCODES * sizeof(short int));
  }
#else
  if (StringPrevious == NULL) { /* allocate string table lengths */
    StringPrevious = (int *) malloc(MAXCODES * sizeof(int));
  }
  if (StringLength == NULL) { /* allocate string table lengths */
    StringLength = (int *) malloc(MAXCODES * sizeof(int));
  }
#endif
/*  if (Memory == NULL || StringTable == NULL ||  StringLength == NULL) { */
  if (StringByte == NULL || StringFirst == NULL ||
    StringPrevious == NULL || StringLength == NULL) {
    showline("Unable to allocate memory\n", 1);
    checkexit(1);
//    or more serious exit(1) ???
  }
  return 0;
}

void InitializeStringTable (void)  /* set up string table initially */
{
  int k;

  AllocStringsIn();         /* grab memory for tables if needed */
//  What if it failed ???
  for (k = 0; k < MAXCHR; k++) {  /* 256 single byte strings */
    StringByte[k] = (char) k;
    StringFirst[k] = (char) k;
    StringPrevious[k] = -1;   /* indicate beginning of string */
    StringLength[k] = 1;
  }
/*  FreeIndex = MAXCHR; */
  TableIndex = FIRSTCODE;
  CodeLength = 9;         /* initial code length */
}

void ResetStringTable (int quietflag) /* clear string table */
{
/*  int k; */

#ifdef DEBUGTIFF
  if (!quietflag)
  {
    if (traceflag) showline("CLEAR ", 0);
    if (traceflag) showline("\n", 0);
    if (traceflag && TableIndex > FIRSTCODE)
    {
/*      printf("TableIndex %d FreeIndex %u CodeLength %d\n",
        TableIndex, FreeIndex, CodeLength); */
      sprintf(logline, "TableIndex %d CodeLength %d\n", TableIndex, CodeLength);
      showline(logline, 0);
    }
  } 
#endif
/*  following not really needed */
/*  for (k = FIRSTCODE; k < TableIndex; k++) {
    StringTable[k] = 0;
    StringLength[k] = 0;
  } */
/*  FreeIndex = MAXCHR; */
  TableIndex = FIRSTCODE;
  CodeLength = 9;
}

#ifdef USESHORTINT
void AddNewEntry (short int OldCode, short int Code)
{
#else
void AddNewEntry (int OldCode, int Code)
{
#endif
/*  char *s; */
/*  char *t; */
/*  int k; */

#ifdef DEBUGLZW
  if (traceflag)
  {
    sprintf(logline, "Add string TableIndex %4d (%d)\n",
         TableIndex, StringLength[OldCode] + 1);
    showline(logline, 0);
  }
#endif

/*  This is where we enter new one in table */
/*  StringTable[TableIndex] = FreeIndex; */
/*  StringLength[TableIndex] = len; */
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
/*  s = Memory + FreeIndex; */
/*  t = Memory + StringTable[OldCode]; */
/*  for (k = 0; k < len-1; k++) *s++ = *t++; */
/*  t = Memory + StringTable[Code];  */
/*  *s = *t; */         /* last byte comes from next code string */
/*  FreeIndex += len; */

  if (TableIndex == 511 || TableIndex == 1023 || TableIndex == 2047)
  {
    CodeLength++;
#ifdef DEBUGTIFF
    if (traceflag)
    {
      sprintf(logline, "LENGTH %d (%d)\n", TableIndex, CodeLength);
      showline(logline, 0);
    }
#endif
  }

  if (TableIndex > MAXCODES)
  {
    showline(" ERROR: Table overflow\n", 1);
    checkexit(1);
  }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int IsInTable (int Code)  /* is code already in table ? */
{
  return (Code >= 0 && Code < TableIndex);
}

/* copy string from table by code number */

unsigned char *WriteString (unsigned char *buffer, int Code)
{
/*  int k; */
  int n, len=0;
  unsigned char *s;
  unsigned char *t;

/*  s = buffer; */
/*  t = Memory + StringTable[Code]; */
//  if (Code < TableIndex)
  len = StringLength[Code];
  t = buffer + len;         /* byte after last one to copy */
  if (t > StripData + StripDataLen) {
/*    special case kludge if terminates right after code length switch */
    if (((unsigned int) (buffer - StripData) == StripDataLen) &&
// kludge to try and deal with lack of code length increase just before EOI
        ((Code == (EOD*2)) || (Code == (EOD*4))))  // 2000/Feb/4
    {
      return buffer;
    } /* 98/Sep/22 */
/*    showline(" ERROR: ran off end of Strip Buffer\n", 1); */
    sprintf(logline, 
      "ERROR: Strip Buffer len %d code %d EOD %d CodeLength %d TableIndex %d",
        len, Code, EOD, CodeLength, TableIndex);
    showline(logline, 0);
    if (traceflag) {
      sprintf(logline,
      "code %d len %d buffer %ld buffer + len %ld stripdatalen %ld\n",
         Code, len, buffer-StripData, t-StripData, StripDataLen);
      showline(logline, 0);
    }
    return buffer;
  }
  s = t - 1;              /* last byte to copy */
/*  for (k = 0; k < len; k++) *s++ = *t++; */
  n = Code;
/*  for (k = 0; k < len; k++) */
/*  for (k = len; k > 0; k--) */
  while (n >= 0) {
/*    if (k != StringLength[n])
      printf("k %d <> len %d ", k, StringLength[n]); */ /* check */
    *s-- = StringByte[n];           /* copy the byte */
/*    if (StringPrevious[n] < 0 && k != 1)
      printf("k %d n %d Code %d ", k, n, Code); */
    n = StringPrevious[n];
/*    if (n < 0) break; */            /* termination */
  }
  if ((s + 1) != buffer) {            /* sanity check */
    int err;
    if ((s+1) > buffer) err = (s+1) - buffer;
    else err = buffer - (s+1);
    sprintf(logline, "Off by %d (len %d Code %d)\n", err, len, Code);
    showline(logline, 1);
  }
/*  s = buffer;
  while (s < t) {
    if (*s++ != 0) putc('1', stdout);
    else putc('0', stdout);
    if (counter++ % 1024 == 0) putc('\n', stdout);
  }
  return buffer; */                 /* TESTING HACK! */
  return t;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long OldByte=0;    /* cadaver being eaten */
int OldBitsLeft=0;        /* how many bits left in OldByte */

#ifdef USESHORTINT
short int GetNextCode (FILE *input) /* get next LZW code number from input  */
{
#else
int GetNextCode (FILE *input) /* get next LZW code number from input  */
{
#endif
  int bits;
  int c;
  unsigned long k;

  bits = OldBitsLeft;         /*  how many bits do we have */
  k = OldByte;            /*  start with old bits */

  while (bits < CodeLength) {     /* need more bits ? */
    if (InStripLen-- <= 0) {    /* treat as EOD 96/Nov/17 */
      if (traceflag) {
        sprintf(logline, " No EOD at end of strip %d ", CurrentStrip);
        showline(logline, 1);
      }
      return EOD;
    }
    if ((c = getc(input)) == EOF)
    {
      sprintf(logline, " Unexpected EOF (%s)\n", "getnextcode");
      showline(logline, 1);
      checkexit(1);
//      finish = -1;
//      return -1;          /* EOF serious error ... */
    } 
    k = (k << 8) | c;
    bits += 8;
  }
  OldByte = k;
  OldBitsLeft = bits - CodeLength;    /* extra bits not used */
/*  OldByte = OldByte & ((1 << OldBitsLeft) - 1); *//* redundant */
  k = k >> OldBitsLeft;         /* shift out extra bits */
  k = k & ((1 << CodeLength) - 1);    /* mask out high order */
#ifdef DEBUGLZW
  if (traceflag) {
    sprintf(logline, "CODE %4d ", k);
    showline(logline, 0);
  }
#endif
#ifdef USESHORTINT
  return (short int) k;
#else
  return (int) k;
#endif
}

void LZWdecompress (unsigned char *StripData, FILE *input)
{
#ifdef USESHORTINT
  short int Code, OldCode;
#else
  int Code, OldCode;
#endif
  unsigned int nlen;
  unsigned char *s = StripData;

  OldCode = 0;            /* to keep compiler happy */
/*  InitializeStringTable(); */     /* assume already done once */
/*  ResetStringTable(1); */       /* not needed - first code CLEAR */
  while ((Code = GetNextCode(input)) != EOD) {
    if (Code == -1) {       // error check only
      showline(" ERROR: Premature end of LZW", 1);
      showline("\n", 0);
      return;
    }
// #ifdef DEBUGLZW
//    if (traceflag) {
//      sprintf(logline, "%4d ", Code);       /* debugging */
//      showline(logline, 0);
//    }
// #endif
    if (Code == CLEAR) {
      ResetStringTable(0);
      Code = GetNextCode(input);
      if (traceflag) showline("\n", 0);
      if (Code == -1) {     // error check only
        showline(" ERROR: Premature end of LZW", 1);
        showline(" (after CLEAR)", 0);
        showline("\n", 0);
        return;
      }
      if (Code == EOD) break;
      s = WriteString(s, Code);
      OldCode = Code;
    }               /* end of CLEAR case */
    else {
//      if (IsInTable(Code)) {
      if (Code >= 0 && Code < TableIndex) {
/*        AddTableEntry(StringTable(OldCode)
          + FirstChar(StringFromCode(Code);)); */
        AddNewEntry(OldCode, Code);
        s = WriteString(s, Code);
        OldCode = Code;
      }             /* end of Code in Table case */
      else {            /* Code is *not* in table */
/*        OutString = StringFromCode (OldCode) +
          + FirstChar(StringFromCode(Code);)); */
        if (Code > TableIndex) {
// kludge to try and deal with lack of code length increase just before EOI
          Code = Code / 2;
          OldBitsLeft++;
//          CodeLength--;
          if (traceflag) {
            sprintf(logline,
              " Code (%d) > TableIndex (%d) CodeLength %d ",
              Code, TableIndex, CodeLength);
            showline(logline, 1);
          }
//          break;      /* ugh! */
        }         // error check only
/*        strcpy(Omega, StringFromCode(OldCode));
        ConcatOneChar(Omega, StringFromCode(OldCode));
        WriteString(Omega, output);
        AddTableEntry(Omega); */
        AddNewEntry(OldCode, OldCode); 
        s = WriteString(s, Code);
        OldCode = Code;
      } /* end of Code *not* in Table case */
    } /* end of *not* CLEAR case */
  } /* end of not EOD loop */

/*  if (bExpandColor) nlen = StripDataLen / 3;
  else nlen = StripDataLen; */ /* ??? */
  if (traceflag) {  /*  NOTE: mismatch on last strip is OK */
    if (CurrentStrip < StripsPerImage-1) {        /* 96/Nov/17 */
      if (bExpandColor) nlen = StripDataLen / 3;
      else nlen = StripDataLen;
      if ((unsigned int) (s - StripData) != nlen) {
        sprintf(logline,
        " Strip data mismatch %u < %u bytes (Strip %d of %ld)\n",
            (s - StripData), nlen, CurrentStrip, StripsPerImage);
        showline(logline, 1);
      }
    }
  } 

/*  if (verboseflag) printf("EOD "); */
/*  reset table for next one */
  ResetStringTable(0);
#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, "Now at byte %ld in file\n", ftell(input));
    showline(logline, 0);
  }
#endif
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***  */

/* Set up StripOffset & StripByteCount for strip number k */
/* and go to start of data for strip number k */
/* First one was already set up - don't disturb in case not indirect */
/* In practice, strips are usually contiguous, but no guarantee... */

long SetupStrip (FILE *input, int k)      /* 96/Nov/17 */
{
  if (k > 0) {
    StripOffset = indirectvalue(StripOffsetsType, 1,
      StripOffsetsPtr + k * typesize[StripOffsetsType], input);
    StripByteCount = indirectvalue(StripByteCountsType, 1,
      StripByteCountsPtr + k * typesize[StripByteCountsType], input);
  }
#ifdef DEBUGTIFF
  if (traceflag) {
    sprintf(logline, "Strip %d Offset %ld ByteCount %ld\n",
    k, StripOffset, StripByteCount);
    showline(logline, 0);
  }
#endif
  if (fseek (input, StripOffset, SEEK_SET))
  {
    showline("Error in seek to StripOffset", 1);  /* ??? */
    finish = -1;
    return -1;
  }
  if (k == 0 && StripByteCount < 0)
    StripByteCount = 0XFFFFFF;    // kludge if missing 2000/Apr/13
  return StripByteCount;            /* 96/Nov/17 */
}

/* Copy a row from far space used by LZW to near space used for output */

void CopyRow (unsigned char *lpBuffer, unsigned char *StripData,
    long InRowLength)
{
  int k, n;
  unsigned char *s=lpBuffer;
  unsigned char *t=StripData;
  n = (int) InRowLength;    /* assuming row not longer than 32k */
/*  if (InRowLength > BufferLength) { */ /* DEBUGGING 95/Nov/10 */
  if (InRowLength > BufferLength+1) {  /* DEBUGGING 95/Nov/10 */
    sprintf(logline, " ERROR: InRowLength %ld > BufferLength %ld\n",
        InRowLength, BufferLength);
    showline(logline, 1);
/*    return; */
    n = (int) BufferLength;      /* prevent heap corruption */
  }
  for (k = 0; k < n; k++) *s++ = *t++; 
}

/* A whole strip is treated as one unit for LZW encoding ... */
/* So need to do once per strip and need memory for strip output */

int DecodeLZW (FILE *output, FILE *input, unsigned char *lpBuffer)
{
  int k, row = 0, i, j, n, m, flag = 0;
/*  unsigned char *StripData; */
  long nlen;

  nlen = (long) BufferLength * RowsPerStrip;

  StripDataLen = (unsigned int) nlen;
#ifdef DEBUGTIFF
  if (traceflag) {            /* debugging */
    sprintf(logline, " Allocating %u bytes for Strip Buffer\n", StripDataLen);
    showline(logline, 0);
  }
#endif
  StripData = (unsigned char *) malloc(StripDataLen);
/*    malloc((unsigned int) (InRowLength * RowsPerStrip)); */
/*    malloc((unsigned int) (BufferLength * RowsPerStrip)); */
  if (StripData == NULL) {
//    showline(" ", 0);
    showline(" Unable to allocate memory\n", 1);  /* 1995/July/15 */
    checkexit(1);
//    or more serious exit(1) ???
  }
  InitializeStringTable(); 
/*  checkheap("AFTER INITIALIZE", 0); */  /* debugging only 1995/Nov/10 */
  row = 0;
  for (k = 0; k < StripsPerImage; k++) {
    CurrentStrip = k;         /* global for debug output */
/*    SetupStrip(input, k); */
    InStripLen = SetupStrip(input, k);  /* save GetNextCode 96/Nov/17 */
    OldByte = 0;  OldBitsLeft = 0;
    ResetStringTable(1);      /* redundant ? */
    LZWdecompress(StripData, input);
    n = RowsPerStrip;
    if (row + n > ImageLength) n = (int) (ImageLength - row);
    for (i = 0; i < n; i++) {
      CopyRow(lpBuffer, StripData + i * InRowLength, InRowLength);
/*      Following new 1996/Sep/9 */
      if (Predictor == 2) {   /* only applies to LZW images */
/*        We will assume here that BitsPerSample == 8 ... */
/*        if (SamplesPerPixel == 1) */      /* gray level */
        if (SamplesPerPixelX == 1) {      /* gray level */
          for (j = 1; j < InRowLength; j++)
/*            lpBuffer[j] += lpBuffer[j-1]; */
            lpBuffer[j] = (unsigned char) (lpBuffer[j] + lpBuffer[j-1]);
        }
        else {                /* RGB (3) or CMYK (4) */
/*          for (j = SamplesPerPixel; j < InRowLength; j++) */
          for (j = SamplesPerPixelX; j < InRowLength; j++)
/*            lpBuffer[j] += lpBuffer[j-SamplesPerPixel]; */
            lpBuffer[j] =
/*              (unsigned char) (lpBuffer[j] + lpBuffer[j-SamplesPerPixel]); */
              (unsigned char) (lpBuffer[j] + lpBuffer[j-SamplesPerPixelX]);
        }
      }   
      if (ExtraSamples > 0)
        (void) RemoveExtraSamples(lpBuffer, InRowLength); /* 99/May/10 */
/*      if (ProcessRow (output, lpBuffer, InRowLength, BufferLength, */
      if (ProcessRow (output, lpBuffer, InRowLengthX, BufferLength,
        OutRowLength) != 0) break;  /* break if output error */
/*      if (bCompressColor)
        compresscolorfar(StripData + i * BufferLength, InRowLength);
      if (writearowfar (output, StripData + i * BufferLength,
        OutRowLength) != 0) {
        flag = 1;
        break;
      }
      if (flag) break; */
    }
    row += n;
  }
  ResetStringTable(1);      /* redundant ? */
#ifdef DEBUGTIFF
  if (traceflag) {          /* debugging */
    sprintf(logline, " Freeing %u bytes Strip Buffer\n", StripDataLen);
    showline(logline, 0);
  }
#endif
  free(StripData);
  StripData = NULL;       /* debugging 95/Nov/10 */
  if (traceflag) { /* debugging code added */ /* should work also in NT */
    if ((m = _heapchk ()) != _HEAPOK) {     /* 1995/Nov/10 */
      sprintf(logline, " ERROR: heap corrupted (%d)\n", m);
/*      sprintf(logline, " ERROR: __near heap corrupted (%d)\n", m); */
      showline(logline, 1);
/*      checkexit(9); */
      exit(9);      /* terminate with extreme prejudice */
    }
  }
  return flag;
}

/* InitializeStringTable AllocString - may need to DeAllocString eventually */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* change long => int where sensible */

/* complain if samplesperpixel other than one */ /* no color support */
/* quit early (before writing header) if 24 bit image */

/* implement CCIT Huffman decompression also OK */
/* implement LZW decompression also */ /* need Predictor also ? */

/* implement gray scale response curve ? */ /* although ignored by some apps */

/* II* or MM* followed by four byte offset to first IFD (often == 8) */

/* IFD: 2 byte count of fields, followed by 12 byte tag fields */
/* after that, four byte offset to next IFD (or zero) (or junk!) */

/* Field entries sorted in ascending order on tag: */

/* Byte 0 & 1: tag */
/* Byte 2 & 3: type */
/* Byte 4 - 7: `length' (better called `count') */
/* Byte 8 -11: value offset -- always even  (or value itself if it fits) */

/* if bAllowColor is false, maybe just use green component and `image' */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

BOOL bTIFFAllow=1;

/* x * (numer / denom) 1995/Oct/12 */
/* or use MulDiv ??? */

long MulRatio (long x, unsigned long numer, unsigned long denom)
{
  long result;
  if (denom == 0 || numer == 0) return 0;
  if ((numer % denom) == 0) {
    numer = numer / denom;
    denom = 1;
  }
  if ((denom % numer) == 0) {
    denom = denom / numer;
    numer = 1;
  }
  while (denom > 65536 || numer > 65536) {
    denom = denom >> 1;
    numer = numer >> 1;
  }
  result = x / denom * numer;
  return result;    
}

// long MulRatio (long x, unsigned long numer, unsigned long denom) {
//  if (denom == 0 || numer == 0) return 0;
//  else return(MulDiv(x, numer, denom));
// }

/* actually insert the TIFF figure here */
/* negative nifd indicates figure missing - draw box and filename only */
/* if dheight == 0 => calculate dheight from dwidth based on aspect ratio */
/* if dwidth == 0 => calculate dwidth & dheight from xres and yres */

/* actually insert the TIFF figure here */
/* negative nifd indicates figure missing - draw box and filename only */
/* if dheight == 0 => calculate dheight from dwidth based on aspect ratio */

int showtiffhere (FILE *output, char *filename, long dwidth, long dheight,
         double xscale, double yscale,  int nifd)
{
  int flag;
  if (bTIFFAllow != 0) {
/*    if (dheight == 0) { */  /* need aspect ratio from TIFF file */
    if (dheight == 0 || dwidth == 0) {
      if (traceflag) {
        sprintf(logline, " Prescanning %s\n", filename);
        showline(logline, 0);
      }
      flag = readimagefile(output, filename, dwidth, dheight, nifd, 1);
/*      if (flag < 0) return; */  /* was unable to find file 96/Dec/27 */
/*      if (traceflag) printf("FLAG %d\n", flag); */
      if (flag == 0) {      /* 96/Dec/27 */
        if (dwidth == 0 && dheight == 0) {      /* fix 95/Oct/12 */
/* convert information in image to scaled points */
/*          dwidth = (ImageWidth * xresden / xresnum) << 16; */
/*          dheight = (ImageLength * yresden / yresnum) << 16; */
/*          if (traceflag)
            printf(" ImageWidth %ld (%lu / %lu) ImageLength %ld (%lu / %lu)\n",
            ImageWidth, xresnum, xresden, ImageLength, yresnum, yresden); */
          if (xresnum == 0 || yresnum == 0) {   /* 96/Apr/4 */
            sprintf(logline,
                " xres %lu / %lu yres %lu / %lu unit %d\n",
                xresnum, xresden, yresnum, yresden, ResolutionUnit);
            showline(logline, 1);
/*            xresnum = yresnum = 72; */ /* 96/Apr/3 */
            xresnum = yresnum = nDefaultTIFFDPI;
/*            return; */          /* 96/Apr/4 */
          }

/*          dwidth = (ImageWidth << 16) / xresnum * xresden; */
          dwidth = MulRatio(ImageWidth << 16, xresden, xresnum);
/*          dheight = (ImageLength << 16) / yresnum * yresden; */
          dheight = MulRatio(ImageLength << 16, yresden, yresnum);
/*          if (traceflag)
            printf(" dwidth %ld dheight %ld\n", dwidth, dheight); */
          if (ResolutionUnit == 1) {      /* what to do ? */
            showline(" resolution == 1", 1);
          }
          else if (ResolutionUnit == 2) {   /* 72.27 pt per in */
            dwidth = dwidth / 100 * 7227;
            dheight = dheight / 100 * 7227;
          }
          else if (ResolutionUnit == 3) {   /* 28.45 pt per cm */
            dwidth = dwidth / 100 * 2845;
            dheight = dheight / 100 * 2845;
          }
        }
/* adjusted order of mul and div to avoid potential overflow 94/Dec/16 */
/* or use MulDiv ??? */ /* switched to MulRatio 98/Jul/9 */
        else if (dheight == 0) {
          if (ImageWidth != 0) 
/*            dheight = (ImageLength * dwidth) / ImageWidth; */
/*            dheight = (dwidth / ImageWidth) * ImageLength;  */
            dheight = MulRatio(dwidth, ImageLength, ImageWidth);
        }
        else if (dwidth == 0) {
          if (ImageLength != 0)
/*            dwidth = (ImageWidth * dheight) / ImageLength; */
/*            dwidth = (dheight / ImageLength) * ImageWidth; */
            dwidth = MulRatio(dheight, ImageWidth, ImageLength); 
        }
/*      added 99/July/2 */
        if (xscale != 0.0) dwidth = (long) ((double) dwidth * xscale);
        if (yscale != 0.0) dheight = (long) ((double) dheight * yscale);
#ifdef DEBUGTIFF
        if (traceflag) {
          sprintf(logline, " dwidth %ld dheight %ld", dwidth, dheight);
          showline(logline, 0);
        }
#endif
    }
/*    xll = mapx(dvi_h); yll = mapy(dvi_v); */
/*    xll = dvi_h; yll = dvi_v; */
/*    xur = mapx(dvi_h + dwidth); yur = mapy(dvi_v - dheight); */
/*    xur = dvi_h + dwidth; yur = dvi_v - dheight; */
    }     /* 96/Dec/27 */
/*    if (nifd >= 0) readimagefile(output, filename, dwidth, dheight, nifd, 1); */
    if (nifd >= 0)
      flag = readimagefile(output, filename, dwidth, dheight, nifd, 0);
    
  }
  return 0;
}

/* have the \special in line at this point - entry point from dvispeci.c */

/*  else if (strcmp(line, "textcolor") == 0 ||
    strcmp(line, "rulecolor") == 0 ||
    strcmp(line, "figurecolor") == 0 ||
    strcmp(line, "reversevideo") == 0 ||
    strcmp(line, "button") == 0 ||
    strcmp(line, "mark") == 0 ||      
    strcmp(line, "viewrule") == 0 ||
    strcmp(line, "viewtext") == 0 ||
    strcmp(line, "insertimage") == 0) { */

/* made common to save string space */

void dontunderline (void)
{
//  showline(" ERROR: don't understand ", 1);
//  showline(line, 1);
  sprintf(logline, " ERROR: don't understand: %s", line);
  showline(logline, 1);
}

/* name => buffer convert \ to / and change : to / and so on */

void platformindependent (char *buffer, char *name) { /* 96/Mar/3 */
  char *s;
  strcpy(buffer, name);
/*  avoid trouble with file names that contain spaces - truncate 'em! */
  if ((s = strchr(buffer, ' ')) != NULL) *s = '\0';
/*  if ((s = strchr(buffer, ':')) != NULL) { */
  if ((s = strchr(buffer, ':')) != NULL &&
    *(s+1) != '\\' && *(s+1) != '/') {      /* 98/Jun/30 */
    *s = '/';
    memmove(buffer+1, buffer, strlen(buffer)+1);
    *buffer = '/';
  }
/*  which is not really much use unless it is a relative file name ... */
/*  while ((s = strchr(buffer, '\\')) != NULL) *s = '/'; */
  s = buffer;           /* now convert \ to / */
  while ((s = strchr(s, '\\')) != NULL) *s = '/';
}

/* clean up hyper-text mark name for PostScript Distiller 95/Feb/25 */

/* Remove characters from name to make it valid PS name */
/* Valid name may not contain white space or delimiters (pgs 30, 27 PS bible) */
/* White space is null (0), tab (9), linefeed (10), formfeed (12), */
/* return (13), and space (32) */
/* Delimiters are () <> [] {} / % */
/* Note: some question about allowing \ here, since in a *string* */
/* backslash is used as an escape - so if name converted to string ... */

/* Implementation limit on name length is 127 (Appendix B.1 in PS reference) */
/* Implementation limit on file name length is 100 */

/* #define MAXPSNAME 32 */  /* old limit */
#define MAXPSNAME 63  /* 98/Jul/16 */

/* This removes punctuation also . , : ; ! ? etc  NOT ANYMORE! 97/Feb/23 */
/* Removes white space and delimiters - limits to MAXPSNAME char */
/* NOTE: must use cleanupname *both* in mark: and button: */
/* and URL calls with named destinations 97/Feb/23 ! */

/* what if the string is quoted ? */ /* \b is not strictly forbidden */

/* what about #?  not allowed in PDF name. but we want to pass it here? */

/* char *badcharinname=" \t\n\r\f()<>[]{}/%"; */
char *badcharinname=" \b\t\n\f\r()<>[]{}/%";

void cleanupname (char *name)
{
  char *s;
#ifdef DEBUGCLEANUP
  if (traceflag)
  {
    sprintf(logline, " CLEANUPNAME: %s", name);     /* debugging */
    showline(logline, 0);
  }
#endif
  if (*name == '\0')
  {
    showline(" ERROR: empty mark or button", 1);
  }
  s = name;
  while ((s = strpbrk(s, badcharinname)) != NULL)
    strcpy(s, s+1);
  if (strlen(name) > MAXPSNAME) *(name+MAXPSNAME) = '\0'; /* 96/Aug/12 */
#ifdef DEBUGCLEANUP
  if (traceflag)
  {
    sprintf(logline, " CLEANEXIT: %s", name);   /* debugging */
    showline(logline, 0);
  }
#endif
  if (*name == '\0') showline(" ERROR: empty mark or button", 1);
}

/* support for old textcolor / rulecolor specials */

int oldcolor (FILE *output, FILE  *input)
{
  char *s;
  int n;
/*  old text color support */
  if (strcmp(line, "textcolor") == 0) {
    (void) scan_special (input, line, MAXLINE);
    if (traceflag) {
      sprintf(logline, " %s ", line);
      showline(logline, 0);
    }
    if (!bKeepBlack) {
      if (strstr(line, "revert") != NULL) {
        text_red = 0.0; text_green = 0.0; text_blue = 0.0;
//        fputs("\nblack\n", output);
        PSputs("\nblack\n", output);
        bTextColor = 0;
      }
      else if (sscanf(line, "%lg %lg %lg",
            &text_red, &text_green, &text_blue) == 3) {
/*        see whether floating point or not */
        if ((s = strchr(line, '.')) == NULL || *(s+1) < '0') {
          text_red = text_red / 255.0;
          text_green = text_green / 255.0;
          text_blue = text_blue / 255.0;
        }
        if (text_red == 0.0 && text_green == 0.0 && text_blue == 0.0) {
//          fputs("\nblack\n", output);
          PSputs("\nblack\n", output);
          bTextColor = 0;
        }   /* treat 0 0 0 same as revert ? */
        else {
          sprintf(logline, "\n%lg %lg %lg rgb\n",
              text_red, text_green, text_blue);
          PSputs(logline, output);
          bTextColor = 1;
        }
      }
      else {
        dontunderline();
        return 1;
      }
//      fprintf(output, "\n%lg %lg %lg setrgbcolor\n",
    }
    return 1;
  }
  /*  old rule color support */
  else if (strcmp(line, "rulecolor") == 0) {
    (void) scan_special (input, line, MAXLINE);
#ifdef DEBUGTIFF
    if (traceflag) {
      sprintf(logline, " %s ", line);
      showline(logline, 0);
    }
#endif
    if (!bKeepBlack) {
      if (strstr(line, "revert") != NULL) {
        rule_red = 0.0; rule_green = 0.0; rule_blue = 0.0;
        bRuleColor = 0;
      }
      else if (sscanf(line, "%lg %lg %lg", &rule_red, &rule_green, &rule_blue) == 3) {
/* see whether floating point or not */
        if ((s = strchr(line, '.')) == NULL || *(s+1) < '0') {
          rule_red = rule_red / 255.0;
          rule_green = rule_green / 255.0;
          rule_blue = rule_blue / 255.0;
        }
        if (rule_red == 0.0 && rule_green == 0.0 && rule_blue == 0.0)
          bRuleColor = 0;
        else bRuleColor = 1; 
      }
      else {
        dontunderline();
        return 1;
      }
    }
    return 1;
  }
/*  old figurecolor support */
  else if (strcmp(line, "figurecolor") == 0) {
    (void) scan_special (input, line, MAXLINE);
#ifdef DEBUGTIFF
    if (traceflag) {
      sprintf(logline, " %s ", line);
      showline(logline, 0);
    }
#endif
    if (!bKeepBlack) {
      if (strstr(line, "revert") != NULL) {
        figurered = 0.0; figuregreen = 0.0; figureblue = 0.0;
        backred = 1.0; backgreen = 1.0; backblue = 1.0;
        bFigureColor = 0;
      }
      else if ((n = sscanf(line, "%lg %lg %lg %lg %lg %lg",
               &figurered, &figuregreen, &figureblue,
               &backred, &backgreen, &backblue)) >= 3) {
        bFigureColor = 1;
/* see whether floating point or not */
        if ((s = strchr(line, '.')) == NULL || *(s+1) < '0') {
          figurered = figurered / 255.0;
          figuregreen = figuregreen / 255.0;
          figureblue = figureblue / 255.0;
          if (n == 6) {
            backred = backred / 255.0;
            backgreen = backgreen / 255.0;
            backblue = backblue / 255.0;
          }
        }
        if (n < 6) {      /* default background color */
          backred = 1.0; backgreen = 1.0; backblue = 1.0;
        }

        if (figurered == 0.0 && figuregreen == 0.0 && figureblue == 0.0 
          && backred == 1.0 && backgreen == 1.0 && backblue == 1.0) 
          bFigureColor = 0; 
        else bFigureColor = 1;
      }       /* end of three or more numbers found case */
      else {
        dontunderline();
        return 1;
      }
    }
    return 1;
  }
/*  old reverse video support */
  else if (strcmp(line, "reversevideo") == 0) {
    (void) scan_special (input, line, MAXLINE);
#ifdef DEBUGTIFF
    if (traceflag) {
      sprintf(logline, " %s ", line);
      showline(logline, 0);
    }
#endif
    if (!bKeepBlack) {
      if (strstr(line, "on") != NULL) bReverseVideo = 1;
      else if (strstr(line, "off") != NULL) bReverseVideo = 0;
      else if (strstr(line, "toggle") != NULL)
      bReverseVideo = ~bReverseVideo;
/*        ignore reversevideo for the moment ??? */
      if (bReverseVideo) {
//        fputs("{1 exch sub} settransfer\n", output);
        PSputs("{1 exch sub} settransfer\n", output);
      }
      else {
//        fputs("{} settransfer\n", output);
        PSputs("{} settransfer\n", output);
      }
    }
    return 1;
  }
  else dontunderline();
  return 0;
}

/* new to allow quoted file names with spaces 98/Jul/9 */

char *scaninsert(char *line, char *filename)
{
  char *s = line;
  char *t = filename;

  *filename = '\0';

  while (*s <= ' ' && *s != '\0') s++;  /* step over white space */

  if (*s == '\0') return 0;

  if (*s == '\"') {     /* is it quoted file name ? */
    s++;          /* step over initial " */
    while (*s != '\"' && *s != '\0') *t++ = *s++;
    if (*s != '\0') s++;          /* step over final " */
  }
  else {            /* normal file name */
    while (*s > ' ' && *s != '\0') *t++ = *s++;
  }
  *t = '\0';
  if (*s <= ' ' && *s != '\0') s++; /* step over white space after name */
/*  special hack to convert ## to # */
  if ((t = strstr(filename, "##")) != NULL) strcpy(t, t+1);
  if (traceflag) {
    sprintf(logline, "line: %s filename: %s\n",
              line, filename);
    showline(logline, 0);
  }
  return s;
}

/* returns zero if *not* one of our DVIWindo \specials */

int newspecials (FILE *output, FILE *input)
{
  int nifd=1;
  long dheight, dwidth;
  char filename[FILENAME_MAX];
  char *sname=NULL;
  char *sfile=NULL;
  char *slaunch=NULL;
  char *sparams=NULL;
  char *spage=NULL;           /* 96/May/4 */
  char *s, *t;
  int npage=0, noffset=0;         /* 96/May/4 */
  int n, np=0;
  int URLflag=0;              /* 97/Jan/7 */
  double xscale, yscale;

#ifdef DEBUGTIFF
  if (traceflag)
  {
    sprintf(logline, " %s:", line);
    showline(logline, 0);
  }
#endif

/*  insert TIFF or BMP image */
  if (strcmp(line, "insertimage") == 0)
  {
/*    (void) scan_special (input, line, MAXLINE); */
    (void) scan_special_raw (input, line, MAXLINE);
#ifdef DEBUGTIFF
    if (traceflag)
    {
      sprintf(logline, " %s ", line);
      showline(logline, 0);
    }
#endif
    nifd = 1;         /* n-th (sub-)image in TIFF file */
    dwidth = dheight = 0;   /* NEW - use all info from file */
    xscale = yscale = 0.0;    /* 99/July/2 */
/*    if (sscanf(line, "%s %ld %ld %d", filename, &dwidth, &dheight, &nifd) */
    s = scaninsert(line, filename);
    if ((t = strstr(s, "scaled")) == NULL)
    {
      sscanf(s, "%ld %ld %d", &dwidth, &dheight, &nifd);  /* normal */
    }
    else {  /* new case 99/July/2 */
      sscanf(t+7, "%lg %lg", &xscale, &yscale);
      if (xscale > 33.33) xscale = xscale / 1000.0;
      if (yscale > 33.33) yscale = yscale / 1000.0;
      if (xscale != 0.0 && yscale == 0.0) yscale = xscale;
      if (xscale == 0.0) xscale = 1.0;
      if (yscale == 0.0) yscale = 1.0;
    }
    if (*filename != '\0') {  /* need at least file name */
//      clock_t sclock, eclock;
//      sclock = clock();
      hptagflag = 0;
      showtiffhere(output, filename, dwidth, dheight, xscale, yscale, nifd);
//      eclock = clock();
//      sprintf(logline, " TIFF %lg sec\n", (double) (eclock - sclock) / CLOCKS_PER_SEC);
//      showline(logline, 0); // debugging only
    }
    else dontunderline();
    return 1;
  }

/*  old textcolor, rulecolor \specials{...}s */
  else if (strstr(line, "color") != NULL ||
       strstr(line, "video") != NULL)
    return oldcolor(output, input);

/*  mark: name --- mark: "<File Open>" */ /* turn into named destination */
  else if (strcmp(line, "mark") == 0) {
    if (bPDFmarks == 0) {
      flush_special(input);
      return 1;   /* we recognize it, but ignore it */
    }
/*    (void) scan_special (input, line, MAXLINE);  */
    (void) scan_special_raw (input, line, MAXLINE); /* fix 97/Nov/11 */
    if (traceflag) {
      sprintf(logline, " %s ", line);
      showline(logline, 0);
    }
/*    deal with <FOO BAR> style --- flush all < and > and ' ' in string */
    if (usealtGoToR == 0)
      cleanupname(line);    /* should only contain mark name */
                  /* and nothing after the mark */
//    putc('\n', output);   /* ??? */
    PSputc('\n', output);   /* ??? */
    if (*line < ' ') {
      showline(" ERROR: empty destination %s ", 1);
      showline(line, 1);
    }
/*    need alternate form of /Dest ? */
/*    fprintf(stdout, "[ /Dest /%s\n", line); */
    sprintf(logline, "[ /Dest /%s\n", line);
    PSputs(logline, output);
/*    need to find current position in *default* user space */
/*    move up 20 point, ignore x component --- i.e. keep the old x (null) */
/*    keep original zoom (null) */ /* 20 point is arbitrary */
//    fputs(
    PSputs(
"  /View [ /XYZ gsave revscl currentpoint grestore 20 add exch pop null exch null]\n",
        output);  /* revscl ? */
//    fputs("/DEST pdfmark\n", output);
    PSputs("/DEST pdfmark\n", output);
    return 1;
  }

/*  button: <width> <height> name --- button: 1138360 433848 "<DVIfile>" */
/*  we are ignoring for the moment file: ... and launch: ... */
  else if (strcmp(line, "button") == 0) {
    if (bPDFmarks == 0) {
      flush_special(input);
      return 1;   /* we recognize it, but ignore it */
    }
/*    (void) scan_special (input, line, MAXLINE); */
    (void) scan_special_raw (input, line, MAXLINE); 
    if (traceflag) {
      sprintf(logline, " %s ", line);
      showline(logline, 0);
    }
    if (sscanf(line, "%ld %ld%n", &dwidth, &dheight, &n) < 2) {
      showline(" ERROR: button ", 1); /* error output */
      showline(line, 1);
      flush_special(input);
      return 1;   /* it does not have required width and height ! */
    }
/*    if (strstr(line, "launch:") != NULL) {
      flush_special(input);
      return 1;
    } */
    sname = line + n;         /* now after width and height */
    while (*sname == ' ') sname++;    /* step over white space */
/*    catch case where no mark given, only file: , launch: or page: ... */
/*    if ((strncmp(sname, "file:", 5) == 0)) sname = NULL; */
/*    if ((strncmp(sname, "launch:", 7) == 0)) sname = NULL; */
/*    if (strncmp(sname, "file:", 5) == 0 || */ /* avoid file://... */
    if ((strncmp(sname, "file:", 5) == 0 && *(sname+5) != '/') ||
      strncmp(sname, "launch:", 7) == 0 ||
      strncmp(sname, "execute:", 8) == 0 ||
      strncmp(sname, "page:", 5) == 0) sname = NULL;
/*    check whether page number is given 96/May/12 */
    if ((spage = strstr(line, "page:")) != NULL) {
      noffset = 0;
      npage = 0;
      if (sscanf(spage+5, "%d+%d", &npage, &noffset) < 2) {
        if (sscanf(spage+5, "%d", &npage) < 1) {
          sprintf(logline, " ERROR: %s ", spage); /* error output */
          showline(logline, 1);
        }
      }
/*      if (traceflag)  printf("npage %d noffset %d spage-line %d\n",
           npage, noffset, spage-line); */
    }
/*    check whether file: given */
/*    if ((sfile = strstr(line, "file:")) != NULL) { */ /* avoid file:// */
    if ((sfile = strstr(line, "file:")) != NULL && *(sfile+5) != '/') {
      sfile = sfile+5;
      while (*sfile == ' ') sfile++;
/*      if (traceflag) printf("sfile-line %d\n", sfile-line); */
      if (sscanf(sfile, "-p=%d%n", &np, &n) == 1) { /* 98/Dec/8 */
        spage = sfile+3;
/*        what if npage has already been set ? */
        npage = np;
        sfile += n;
        while (*sfile == ' ') sfile++;  
/*        if (traceflag)
          printf("npage %d sfile-line %d\n", npage, sfile-line); */
      }
    }
/*    check whether launch: given */
    if ((slaunch = strstr(line, "launch:")) != NULL) {
      slaunch = slaunch+7;
      while (*slaunch == ' ') slaunch++;
    }
    if (sfile != NULL) slaunch = NULL;    /* can do only one ??? */
//    putc('\n', output);   /* ??? */
    PSputc('\n', output);   /* ??? */
//    fprintf(output,
    sprintf(logline,
        "[ /Rect [currentpoint 2 copy exch %ld add exch %ld sub]\n",
        dwidth, dheight);
    PSputs(logline, output);
/*    Note: default is /Action /GoTo */
    if (sname != NULL) {    /* was a named destination given ? */
      if ((s = strchr(sname, ',')) != NULL) *s = '\0';
/*      should we make a new case for this http: action: PDF: ? */
/*      special case `named destination' http:// ... */
/*      This form of URL link supported under Acrobat 2.1 and later */
/*      use http://www.adobe.com/test.pdf#name for named destination */
      if (strncmp(sname, "http:", 5) == 0 ||
        strncmp(sname, "ftp:", 4) == 0 ||
        strncmp(sname, "news:", 5) == 0 ||
        strncmp(sname, "mailto:", 7) == 0 ||
/*        strncmp(sname, "file://localhost", 16) == 0 || */
/*        strncmp(sname, "file:///|c/...", 16) == 0 || */
        strncmp(sname, "file://", 7) == 0 ||
        strncmp(sname, "gopher:", 7) == 0
         ) {  /* 1996/July/4 */
/*  Now deal with *relative* URL given as http: pdf_from.pdf e.g. 97/May/3 */
/*  Assumes base URL set via \special{PDF: base http://www.YandY.com}% */
        if ((s = strchr(sname, ':')) != NULL && *(s+1) == ' ') {
          sname = s+1;
          while (*sname == ' ') sname++;
        }
/*        TeX changes # => ## so change back ## => # 97/Jan/8 */
        if ((s = strstr(sname, "##")) != NULL) strcpy(s, s+1);
        if ((s = strchr(sname, '#')) != NULL) { /* 97/Feb/23 */
          if (usealtGoToR == 0)
            cleanupname(s+1); /* clean up named destination */
        }
        sprintf(logline, "  /Action << /Subtype /URI /URI (%s) >>\n",
            sname);
        PSputs(logline, output);
      }
      else {          /* not a URL type of thing */
/*  terminate the mark name - in case file: after - 97/Apr/23 */
/*  but avoid truncating a destination that happens to include a : */
        if ((s = strchr(sname, ':')) != NULL) {
          while (s > sname && *s > ' ') s--;
          if (s > sname)      /* fix 97/Nov/11 */
            *s = '\0';      /* terminate the mark name */
        }
        if (usealtGoToR == 0)
          cleanupname(sname);   /* clean up the destination */
        if (*sname < ' ') {
          sprintf(logline, " ERROR: empty destination %s ", sname);
          showline(logline, 1);
        }
/*        fprintf(stdout, "  /Dest /%s\n", sname); */
        if (! usealtGoToR) {
          sprintf(logline, "  /Dest /%s\n", sname);
          PSputs(logline, output);
        }
      }
    } /* end of named destination given */
/*    97/Jan/7 Acrobat 3.0 wants page number in GoToR ? 97/Jan/7 */
    if (sfile != NULL && spage == NULL) {
      npage = 1; noffset = 0; /* go to page 1 as default */
      spage = sfile;      /* pretend there was a number */
    }
    if (spage != NULL) {    /* was a page number given ? 96/May/12 */
      if (noffset != 0) npage += noffset;
/*      fprintf(output, "  /Page %d\n", npage + nPDFoffset); */
      sprintf(logline, "  /Page %d\n", npage);
      PSputs(logline, output);
//      fputs("  /View [ /XYZ -4 PageHeight 4 add null]\n", output);
      PSputs("  /View [ /XYZ -4 PageHeight 4 add null]\n", output);
    }
/*    case where a file or URL is specified */
    if (sfile != NULL) {
      URLflag = 0;
/*      check whether the `file' specified is a URL */
      if (strncmp(sfile, "http:", 5) == 0 ||
        strncmp(sfile, "ftp:", 4) == 0 ||
        strncmp(sfile, "news:", 5) == 0 ||
        strncmp(sfile, "mailto:", 7) == 0 ||
/*        strncmp(sfile, "file://localhost", 16) == 0 || */
/*        strncmp(sfile, "file:///|c/...", 16) == 0 || */
        strncmp(sfile, "file://", 7) == 0 ||
        strncmp(sfile, "gopher:", 7) == 0
         ) URLflag=1; /* treat URL style things different 97/Jan/7 */
      if (usealtGoToR == 0) {     /* for now */
//        fputs("  /Action /GoToR\n", output);
        PSputs("  /Action /GoToR\n", output);
        if ((s = strchr(sfile, ',')) != NULL) *s = '\0';  /* 96/May/4 */
        if (URLflag == 0) {   /* don't mess with extension if URL ? */ 
/*          remove extension if any (typically .dvi) use .pdf instead */
/*          be  careful about ../foo if there is no extension 97/Jan/7 */
          if ((s = strrchr(sfile, '.')) != NULL &&
            ((t = strrchr(sfile, '/')) == NULL || s > t) &&
            ((t = strrchr(sfile, '\\')) == NULL || s > t))
            *s = '\0';
          strcat(sfile, ".pdf");  /* default extension in PS output */
        } /* end of if URLflag == 0 */
/*        massage into Acrobat `platform independent' format */
        platformindependent (filename, sfile);  /* reuse filename */
        sprintf(logline, "  /File (%s)\n", filename);
        PSputs(logline, output);
//        fprintf(output, "  /DosFile (%s)\n", sfile); 
        sprintf(logline, "  /DOSFile (%s)\n", sfile); /* 98/Jun/20 */
        PSputs(logline, output);
      }
      else {  /* use alternative format */
        sprintf(logline, " /Action << /S /GoToR /D (%s) /F (%s) >>/n",
            sname, sfile);  /* ??? 98/Jun/30 */
        PSputs(logline, output);
      }
    }
    if (slaunch != NULL) {
      if ((s = strchr(slaunch, ',')) != NULL) *s = '\0';
//      fputs("  /Action /Launch\n", output);
      PSputs("  /Action /Launch\n", output);
/*      massage into Acrobat `platform independent' format */
      platformindependent (filename, slaunch);
/*      do special things for Windows call ... */
      sprintf(logline, "  /File (%s)\n", filename);
      PSputs(logline, output);
      if ((s = strchr(slaunch, ' ')) != NULL) {
        *s = '\0';
        sparams = s+1;
      }
/*      Acrobat Reader uses Windows ShellExecute(...) */
      sprintf(logline, "  /WinFile (%s)\n", slaunch);
      PSputs(logline, output);
      if (sparams != NULL) {
        sprintf(logline, "  /Params (%s)\n", sparams);
        PSputs(logline, output);
      }
/*      optional key /Dir (current directory) */
/*      optional key /Op  (open or print) */
    }

/*    Default border is [0 0 1] - we want it to be invisible instead */
//    fputs("  /Border [ 0 0 0]\n", output);
    PSputs("  /Border [ 0 0 0]\n", output);
//    fputs("  /Subtype /Link\n", output);
    PSputs("  /Subtype /Link\n", output);
//    fputs("/ANN pdfmark\n", output);
    PSputs("/ANN pdfmark\n", output);
    return 1;
  }
  else if (strcmp(line, "viewrule") == 0 || /* 95/Mar/27 */
       strcmp(line, "viewtext") == 0) { /* 95/Mar/27 */
     flush_special(input);
     return 1;    /* we recognize it, but ignore it */
  }
#ifdef DEBUGTIFF
  else if (traceflag) showline(line, 1);
#endif
  return 0;       /* not a DVIWindo special */
}

#endif

/* proper support for figurecolor \special / bilevel images 1994/March/14 */

/* \special{tiff:/mount/mktpub_d1/users/temp_jobs/59625100nl/procroms.tif
   lib=control.glb xmag=1 ymag=1 hleft vhigh  bclip=0.000000 lclip=0.000000
   rclip=0.000000 tclip=0.000000 nostrip */
/* New stuff for HPTAG */ /* 95/Oct/12 */

int dohptag (FILE *output, FILE *input)
{
/*  double bclip=0, lclip=0, rclip=0, tclip=0; */
/*  int hleft=1, hright=0, vhigh=1, vlow=0; */
/*  double xmag=1, ymag=1; */
/*  int nostrip=1; */
  char *s, *t;
  char filename[FILENAME_MAX];

  (void) scan_special (input, line, MAXLINE);
  if ((s = strchr(line, ' ')) != NULL) *s = '\0';
  if ((t = strrchr(line, '/')) != NULL) strcpy(filename, t+1);
  else if (strlen(line) < sizeof(filename)) strcpy(filename, line);
  else return 0;              /* failure */
/*  now analyze the rest of the line starting at s+1 */
/*  showtiffhere(output, filename, dwidth, dheight, 0, 0, nifd); */
  hptagflag = 1;
  BMPflag = 0;
  showtiffhere(output, filename, 0, 0, 0, 0, 1);
  return 1;               /* success */
}

/* for setting hyper-text buttons we want the *real* underlying PS */
/* (not what we would have seen had mag = 1000 */
/* hence use revscl, *not* revsclx */

/* #pragma optimize ("lge", on) */

/* END OF EXPERIMENT */

/* int WINAPI MulDiv(int nNumber, int nNumerator, int nDenominator); */
