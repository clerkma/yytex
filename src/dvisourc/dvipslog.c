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
* DVI analyzer to extract font and character usage
* part of DVIPSONE
*
**********************************************************************/

#include "dvipsone.h"

#include <io.h> /* for _access */
                /* for _findfirst, _findnext, _findclose */

/* #define DEBUGALIAS */

/* #define DEBUGCOLORSTACK */

COLORSPEC CurrColor;

/* tables to store background color for each page on prescan 98/Jun/30 */

COLORSPEC *BackColors=NULL;

/* font k is to be used at mag * s / (1000 * d) times its normal size */

int fonthit[MAXFONTS];      /* which fonts have been seen - NOT ACCESSED */

/* char *currentfont;  */       /* pointer to current font */
char *currentfont;        /* pointer to current font */

long pageno;  /* for convenience in error messages - may be logical page */

/* now for the scan of the DVI file for font character log generation */
/* now for the scan of the DVI file for PS output generation */

void reset_stack(void)
{
  stinx = 0;
}

void check_stack(int pageno)
{
  if (stinx != 0)
  {
    sprintf(logline, " ERROR: stack not empty at EOP: %d on page %d ", stinx, pageno); /* pageno ? logical page */
    showline(logline, 1);
    tellwhere(input, 1);
  }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
 
/* we don't have to worry about sign extension here - no need for short int */

/* static unsigned int ureadone (FILE *infile) {
  return getc(infile);
} */

static unsigned int ureadtwo (FILE *infile)
{
  return (getc(infile) << 8) | getc(infile);
}

static unsigned long ureadthree (FILE *infile)
{
  int c, d, e; 
/*  unsigned int c, d, e; */
  c = getc(infile); d = getc(infile); e = getc(infile);
  return ((((unsigned long) c << 8) | d) << 8) | e;
}

static unsigned long ureadfour (FILE *infile)
{
  int c, d, e, f;
  c = getc(infile); d = getc(infile);
  e = getc(infile); f = getc(infile);
  return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
}

/* we do have to worry about sign extension here - use short int if needed */

/* static int sreadone (FILE *infile) {
  int c;
  c = getc(infile);
  if (c > 127) return (c - 256);
  else return c;
} */

/* avoid possible compiler optimization error */

static int sreadtwo (FILE *input)    /* experiment 98/Feb/7 */
{
  int c, d;
  c = getc(input);  d = getc(input);
  if (c > 127) c = c - 256;
  return  (c << 8) | d;
}

/* static long sreadthree (FILE *infile) {
  int c, d, e;
  c = getc(infile); d = getc(infile); e = getc(infile);
  if (c > 127) c = c - 256; 
  return ((((long) c << 8) | d) << 8) | e;
 } */

static long sreadfour (FILE *infile)
{
  int c, d, e, f;
  c = getc(infile); d = getc(infile);
  e = getc(infile); f = getc(infile);
  return ((((((long) c << 8) | (long) d) << 8) | e) << 8) | f;
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int MaxColor;   /* size of save area allocated for color stacks */
/* int MaxBack; */    /* size of save area allocated for background color */

COLORSPEC **ColorStacks=NULL; /* array of saved color stacks, one per page */

/*  restore at start of page what was current at end of page-1 */
/*  called from dvianal.c */ /* page is physical DVI page from start of file */

void RestoreColorStack (int page)
{
  COLORSPEC *SavedStack;
  int k;
/*  don't bother if color \special{...} was never used */
  if (bCarryColor == 0 || bColorUsed == 0) return;
  if (page >= MaxColor) return;         /* sanity check */
  page--;     /* restore what was saved at eop of *previous* */
  if (page < 1) { /* nothing saved to restore at start of first page */
    colorindex = 0;
    return;
  }
  if (ColorStacks == NULL) return;    /* sanity check */
/*  GrabColor(); */
  SavedStack = ColorStacks[page];
  if (SavedStack == NULL)
  {
    sprintf(logline, " Bad Color Restore page %d (%d)",
        page, MaxColor-1);
    showline(logline, 1);
    colorindex = 0;
    return;
  }
  colorindex = (int) (SavedStack[0].D + 0.5); /* depth of saved stack */
#ifdef DEBUGCOLORSTACK
  if (traceflag)
  {
    sprintf(logline, " RestoreColorStack from page-1 %d colorindex %d\n",
              page, colorindex);
    showline(logline, 0);
  }
#endif
  if (colorindex > 0 && colorindex < MAXCOLORSTACK) {
    for (k = 0; k < colorindex; k++) ColorStack[k] = SavedStack[k+1];
  }
  else {
    sprintf(logline, " ERROR: colorindex %d", colorindex);  /* BUG */
    showline(logline, 1);
  }
/*  ReleaseColor(); */
}

/*  Save at end of page for start of page+1 */
/*  called from logdo_eop dvipslog.c */ /* page is DVI page from start of file */

void SaveColorStack (int page, int colorindex)
{
  COLORSPEC *SavedStack;
  int k;
/*  if (bCarryColor == 0 || bColorUsed == 0) return; */
  if (bCarryColor == 0) return;
/*  if (page < 0 || page >= MaxColor) return; */
/*  GrabColor(); */
  if (ColorStacks == NULL) return;    /* sanity check */
  if (ColorStacks[page] != NULL) {
    sprintf(logline, " Bad Color Save page %d (%d) %08x",
        page, MaxColor, ColorStacks[page]);
    showline(logline, 1);
/*    free(lpColor[page]); */
  }
#ifdef DEBUGCOLORSTACK
  if (traceflag)
  {
    sprintf(logline, " SaveColorStack page %d colorindex %d\n",
        page, colorindex);
    showline(logline, 0);
  }
#endif
  if (colorindex == 0) return;    /* nothing to save ??? */
  SavedStack = (COLORSPEC *) malloc ((colorindex+1) * sizeof(COLORSPEC));
  ColorStacks[page] = SavedStack;
  SavedStack[0].A = SavedStack[0].B = SavedStack[0].C = 0.0F;
  SavedStack[0].D = (float) colorindex;
  for (k = 0; k < colorindex; k++) SavedStack[k+1] = ColorStack[k];
/*  ReleaseColor(); */
}

/* format of allocated area is count followed by stack dump */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void logdo_push(void)
{
  if (skipflag == 0) {
    stinx++;
    if (stinx > maxstinx) maxstinx = stinx;
/*    if (stinx >= maxstack - 1) {
      showline(" WARNING: The stack will overflow\n", 1);
      errcount(0);
    } */
  }
}

void logdo_pop(void)
{
  if (skipflag == 0)
  {
    stinx--;
    if (stinx < 0)
    {
      sprintf(logline,
           " ERROR: The stack will underflow on page %d ",
           pageno); /* pagenumber ??? logical page */
      showline(logline, 1);
      tellwhere(input, 1);
/*      errcount(0); */
    }
  }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void invalidset(int c)
{
  sprintf(logline, " ERROR: Setting char %d without font on page %d ",
      c, pageno); /* pagenumber ??? logical page */
  showline(logline, 1);
  tellwhere(input, 1);
/*  errcount(0); */
}

/* common subroutine for set2 set3 set4 --- which should not occur */

void logdo_setsub(unsigned long c)
{
  if (skipflag == 0) {
/*    if (ff < 0) invalidset((int) c); */
    if (ff == BLANKFONT) invalidset((int) c);
    else if (c < 256) {
/*      if (bRemapControl && c < MAXREMAP) c = remaptable[c]; */
      if (bRemapControl || bRemapFont) {
        if (c < MAXREMAP) c = remaptable[c];
#if MAXREMAP < 128
        else if (c == 32) c = 195;
        else if (c == 127) c = 196;
#endif
      }
/* NOTE: this must match corresponding code in DVIANAL.C */
      else if (bRemapSpace && c <= 32) {      /* 1995/Oct/17 */
        if (c == 32) c = 195;   /* not 160 */
        else if (c == 13) c = 176;  /* 1996/June/4 */
        else if (c == 10) c = 173;  /* 1996/June/4 */
        else if (c == 9) c = 170;   /* 1996/June/4 */
        else if (c == 0) c = 161;
      }
      currentfont[c] = 1;
    }
  }
}

/* For speed we keep the set1 case separate since it occurs often */

void logdo_set1(FILE *infile)
{
  unsigned int c;
/*  c = ureadone(infile); */
  c = getc(infile);
  if (skipflag == 0) {
/*    if (ff < 0) invalidset((int) c); */
    if (ff == BLANKFONT) invalidset((int) c);
    else {
/*      if (bRemapControl && c < MAXREMAP) c = remaptable[c]; */
      if (bRemapControl || bRemapFont) {
        if (c < MAXREMAP) c = remaptable[c];
#if MAXREMAP < 128
        else if (c == 32) c = 195;
        else if (c == 127) c = 196;
#endif
      }
      else if (bRemapSpace && c <= 32) {      /* 1995/Oct/17 */
        if (c == 32) c = 195;   /* not 160 */
        else if (c == 13) c = 176;  /* 1996/June/4 */
        else if (c == 10) c = 173;  /* 1996/June/4 */
        else if (c == 9) c = 170;   /* 1996/June/4 */
        else if (c == 0) c = 161;
      }
      currentfont[c] = 1;
    }
  }
}

/* simplified 95/Oct/17 by using logo_setsub for logdo_set2 */

void logdo_set2(FILE *infile)
{
  logdo_setsub(ureadtwo(infile));
}

void logdo_set3(FILE *infile)
{
  logdo_setsub(ureadthree(infile));
}

void logdo_set4(FILE *infile)
{
  logdo_setsub(ureadfour(infile));
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* common subroutine for put2, put3, pu4 --- which should not occur */

void logdo_putsub(unsigned long c)
{
  if (skipflag == 0)
  {
/*    if (ff < 0) invalidset((int) c); */
    if (ff == BLANKFONT) invalidset((int) c);
    else if (c < 256) {
/*      if (bRemapControl && c < MAXREMAP) c = remaptable[c]; */
      if (bRemapControl || bRemapFont) {
        if (c < MAXREMAP) c = remaptable[c];
#if MAXREMAP < 128
        else if (c == 32) c = 195;
        else if (c == 127) c = 196;
#endif
      }
      else if (bRemapSpace && c <= 32) {      /* 1995/Oct/17 */
        if (c == 32) c = 195;   /* not 160 */
        else if (c == 13) c = 176;  /* 1996/June/4 */
        else if (c == 10) c = 173;  /* 1996/June/4 */
        else if (c == 9) c = 170;   /* 1996/June/4 */
        else if (c == 0) c = 161;
      }
      currentfont[c] = 1;
    }
  }
}

/* For speed we keep the set1 case separate since it occurs sometimes */

void logdo_put1(FILE *infile)
{
  unsigned int c;
/*  c = ureadone(infile); */
  c = getc(infile);
  if (skipflag == 0) {
/*    if (ff < 0) invalidset((int) c); */
    if (ff == BLANKFONT) invalidset((int) c);
    else {
/*      if (bRemapControl && c < MAXREMAP) c = remaptable[c]; */
      if (bRemapControl || bRemapFont) {
        if (c < MAXREMAP) c = remaptable[c];
#if MAXREMAP < 128
        else if (c == 32) c = 195;
        else if (c == 127) c = 196;
#endif
      }
      else if (bRemapSpace && c <= 32) {      /* 1995/Oct/17 */
        if (c == 32) c = 195;   /* not 160 */
        else if (c == 13) c = 176;  /* 1996/June/4 */
        else if (c == 10) c = 173;  /* 1996/June/4 */
        else if (c == 9) c = 170;   /* 1996/June/4 */
        else if (c == 0) c = 161;
      }
      currentfont[c] = 1;
    }
  }
}

/* simplified 95/Oct/17 by using logo_putsub for logdo_put2 */

void logdo_put2(FILE *infile)
{
  logdo_putsub(ureadtwo(infile));
}

void logdo_put3(FILE *infile)
{
  logdo_putsub(ureadthree(infile));
}

void logdo_put4(FILE *infile)
{
  logdo_putsub(ureadfour(infile));
}

void logdo_set_rule(FILE *infile)
{
  int k;
  for (k=0; k < 8; k++) (void) getc(infile);
}

void logdo_put_rule(FILE *infile)
{
  int k;
  for (k=0; k < 8; k++) (void) getc(infile);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int currentrange; */ /* range currently working in */
long currentpage;   /* page currently working in */
int prescanflag;    /* prescan is always forward */
int pagesequence;   /* count of ascending page sequence */

/* returns zero if page is to be printed */ /* non-zero if to be skipped */

int skip_this_page(long pageno)
{
  int k;
/*  int hitrange=0; */
/*  int wantpage=0; */

  if (rangeindex == 0) return 0;    /* easy, no page ranges specified! */

  if (countzeroflag != 0) {
    if (prescanflag != 0 || reverseflag == 0) {
/* going forward, we remain in same page sequence if: */
/* (positive and ascending) or (negative and descending) */
      if (currentpage == -LINFINITY) ;  /* first time */
      else if (currentpage >= 0 && pageno >= 0 && pageno > currentpage) ;
      else if (currentpage <= 0 && pageno <= 0 && pageno < currentpage) ;
      else pagesequence++;
    }
    else if (reverseflag != 0) {
/* going backward, we remain in same page sequence if: */
/* (positive and descending) or (negative and ascending) */
      if (currentpage == -LINFINITY) ;  /* first time */
      else if (currentpage >= 0 && pageno >= 0 && pageno < currentpage) ;
      else if (currentpage <= 0 && pageno <= 0 && pageno > currentpage) ;
      else pagesequence--;
    }
    currentpage = pageno;           /* remember page number */
  }

  for (k = 0; k < rangeindex; k++) { 
/*    is current page in this page range ? */
    if ((pageno >= beginpages[k]) && (pageno <= endpages[k]))  {
      if (countzeroflag == 0) return 0; /* easy, not using count[0] */
      if (pagerangeseq[k] < 0)      /* no instance specified */
        return 0;           /* means always print */
      if (pagerangeseq[k] == pagesequence) /* matches instance */
        return 0;           /* OK, print it */
    }
  }
  return 1;             /* not inside any specified range */
}

void logdo_bop(FILE *infile) /* beginning of page */
{
  int k;
  long current;       /* ??? */

  current = ftell(input) - 1;

  pagenumber++;     /* increment pages seen - going forward here */
  reset_stack();      /*  stinx = 0; */
  ff = -1;        /* undefined font */
  fnt = finx[0];      /* just in case - not clear could be -1 ! or 255 */
//  currentfont = fontchar;         /* just in case */
  currentfont = fontchar[0];        /* just in case ??? */

  for(k=0; k < 10; k++) counter[k] = sreadfour(infile);

  previous = sreadfour(infile);
/*  skipflag = 0; */
  if (countzeroflag != 0) pageno = counter[0];
  else pageno = (long) pagenumber;
  skipflag = skip_this_page(pageno);  /* figure out if skipped */
/*  if (beginpage != -LINFINITY && pageno < beginpage)  skipflag++;
  if (endpage != LINFINITY && pageno > endpage) skipflag++;  */
/*  following is the logic for two-sided printing */
  if (skipflag == 0) {
    if (oddpageflag != 0 && (counter[0] & 1) == 0) skipflag++;
    if (evenpageflag != 0 && (counter[0] & 1) == 1) skipflag++;
  }
/*  what about first page ? */
  if (bCarryColor) {
    if (colorindex > 0)       /* avoid error on first page */
      doColorPop(pagenumber);   /* 98/Feb/15 to match ColorPush in eop */
  }
  if (bBackGroundFlag) {        /* carry background color 99/Apr/05 */
    if (pagenumber > 0)       /* avoid on first page */
      BackColors[pagenumber] = BackColors[pagenumber-1];
/*    else if (pagenumber == 0) {
      BackColors[0].A = BackColors[0].B = BackColors[0].C = -1.0F;
      BackColors[0].D = -1.0F;
    } */
  }
}

void logdo_eop(FILE *infile)
{
  int c;

  if (bAbort) abortjob();         /* 1992/Nov/24 */
  if (abortflag) return;
  check_stack(pagenumber);
  if (bCarryColor) {
    doColorPush(pagenumber);          /* 98/Feb/15 ??? */
    SaveColorStack(pagenumber, colorindex);   /* 98/Feb/19 ??? */
  }
  if (textures != 0) 
    (void) ureadfour(infile); /* flush Textures length code */
/*  may want to check whether length is something reasonable ? */
  c = getc(infile); (void) ungetc(c, infile);   /* peek ahead */
/*  here we expect to see bop, nop or fnt_def's ONLY */
  if (c >= 0 && c <= 127) {
    sprintf(logline, " ERROR: Invalid DVI code (%d) between EOP and BOP ", c);
    showline(logline, 1);
    tellwhere(infile, 1);
/*    errcount(0); */
    finish = -1;
  }
  if (skipflag == 0)  numpages++;       /* 94/Oct/12 */
  skipflag = 0;
}

void logdo_right1(FILE *infile)
{
  (void) getc(infile);
}

void logdo_right2(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
} 

void logdo_right3(FILE *infile)
{
  (void) getc(infile); (void) getc(infile); (void) getc(infile);
} 

void logdo_right4(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
  (void) getc(infile); (void) getc(infile);
} 

void logdo_w0(void)
{
}

void logdo_w1(FILE *infile) /* rare */
{
  (void) getc(infile);
}

void logdo_w2(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
} 

void logdo_w3(FILE *infile)
{
  (void) getc(infile); (void) getc(infile); (void) getc(infile);
} 

void logdo_w4(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
  (void) getc(infile); (void) getc(infile);
}

void logdo_x0(void)
{
}

void logdo_x1(FILE *infile) /* rare */
{
  (void) getc(infile);
}

void logdo_x2(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
} 

void logdo_x3(FILE *infile)
{
  (void) getc(infile); (void) getc(infile); (void) getc(infile);
}

void logdo_x4(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
  (void) getc(infile); (void) getc(infile);
}

void logdo_down1(FILE *infile) /* rare */
{
  (void) getc(infile);
}

void logdo_down2(FILE *infile) /* rare */
{
  (void) getc(infile); (void) getc(infile);
}

void logdo_down3(FILE *infile)
{
  (void) getc(infile); (void) getc(infile); (void) getc(infile);
}

void logdo_down4(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
  (void) getc(infile); (void) getc(infile);
}

void logdo_y0(void)
{
}

void logdo_y1(FILE *infile) /* rare */
{
  (void) getc(infile);
}

void logdo_y2(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
}

void logdo_y3(FILE *infile)
{
  (void) getc(infile); (void) getc(infile); (void) getc(infile);
} 

void logdo_y4(FILE *infile) /* not used */
{
  (void) getc(infile); (void) getc(infile);
  (void) getc(infile); (void) getc(infile);
}

void logdo_z0(void)
{
}

void logdo_z1(FILE *infile)  /* rare */
{
  (void) getc(infile);
}

void logdo_z2(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
} 

void logdo_z3(FILE *infile)
{
  (void) getc(infile); (void) getc(infile); (void) getc(infile);
} 

void logdo_z4(FILE *infile)
{
  (void) getc(infile); (void) getc(infile);
  (void) getc(infile); (void) getc(infile);
}

void log_switch_font(int fn, FILE *infile) /* switching to other font */
{
  int c;
  ff = fn;      /* set state */
  fnt = finx[fn];
/*  if (fnt < 0) { */
  if (fnt == BLANKFONT) {               /* 93/Dec/11 */
    if (fn == 52) {
      c = getc(infile); (void) ungetc(c, infile);
      if (c == 171 + 52) {
        sprintf(logline, " ERROR: Unexpected encounter of DVI trailer on page %d ", pagenumber);
        showline(logline, 1);
/*        errcount(0); */
/*        finish = -1; */
        giveup(9); 
        return;
      }
    }
    sprintf(logline, " ERROR: switch to undefined font (%d) on page %d ",
        fn, pagenumber);
    showline(logline, 1);
    tellwhere(infile, 1);
/*    errcount(0); */
    fnt = 0; 
  }
//  currentfont = fontchar + MAXCHRS * fnt;
  if (fontchar[fnt] == NULL) setupfontchar(fnt);
  currentfont = fontchar[fnt]; 
  fonthit[fnt] = 1;   /* even if skipflag != 0 ? */
}

void logdo_fnt1(FILE *infile) /* switch fonts */
{
  int fn;
/*  fn = ureadone(infile); */
  fn = getc(infile);
/*  if (skipflag == 0) */
  log_switch_font(fn, infile);
}

void logdo_fnt2(FILE *infile) /* switch fonts */
{
  unsigned int fn;
  fn = ureadtwo(infile);
/*  if (skipflag == 0) */
  if (fn >= MAXFONTNUMBERS) fn = MAXFONTNUMBERS-1;
  log_switch_font((int) fn, infile);
}

void logdo_fntsub(unsigned long fn, FILE *infile) /* switch fonts */
{
/*  if (skipflag == 0) */
  if (fn >= MAXFONTNUMBERS) fn = MAXFONTNUMBERS-1;
  log_switch_font((int) fn, infile);
}

void logdo_fnt3(FILE *infile) /* switch fonts */
{
/*  unsigned long fn;
  fn = ureadthree(infile); */
  logdo_fntsub(ureadthree(infile), infile);
}

void logdo_fnt4(FILE *infile) /* switch fonts */
{
  long fn;
  fn = sreadfour(infile);
/*  if (skipflag == 0) */
  if (fn < 0) fn = 0;
  logdo_fntsub((unsigned long) fn, infile);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void get_header_name (FILE *infile)
{
  char fname[FNAMELEN];     /* buffer to get token into */
  char *s;

  if (get_token(infile, fname, sizeof(fname)) == 0)
  {
    showline(" Can't find header\n", 1);
    errcount(0); 
    return;
  }

  if (headerfile != NULL)
  {
    if ((s = strstr(headerfile, fname)) != NULL)
    {
      s += strlen(fname);
    
      if (*s == '\0' || *s == ',')
      {
        return;
      }
    }
  }

//  check whether there is enough space for the new name to add 
//  if (strlen(headerfile) + strlen(fname) + 2 >= sizeof(headerfile)) {
//    sprintf(logline, " No more space for HEADER (> %d)\n", sizeof(headerfile));
//    showline(logline, 1);
//    errcount(0);
//    return;
//  }

  if (headerfile == NULL) {
//    nheaderlength = strlen(headerfile) + 2;
    nheaderlength = strlen(fname) + 2;
    headerfile = malloc(nheaderlength);
    if (headerfile == NULL) return;     // allocation error - complain ???
    *headerfile = '\0';
  }

  while (strlen(headerfile) + strlen(fname) + 2 > (size_t) nheaderlength) {
    nheaderlength += strlen(fname) + 2;
    headerfile = realloc(headerfile, nheaderlength);
  }
  if (headerfile == NULL) return;   // allocation error - complain ???

//  add a comma separator, unless this is the first one 
  if (*headerfile != '\0') {
    s = headerfile + strlen(headerfile);
    *s++ = ',';           // comma
    *s = '\0';            // not needed
  }
  else s = headerfile;        // first time

//  finally: append the header file name  ...
  strcpy(s, fname);
}

/* get name of file with DSC header comments */ /* only one allowed */

void get_custom_name (FILE *infile)
{
  if (dscfile != NULL)
  {
    showline(" More than one DSCheader", 1);
    errcount(0);
    return;
  }

  if (get_token(infile, line, sizeof(line)) == 0) { /* MAXLINE */
    showline(" Can't find header\n", 1);
    errcount(0); 
  }
  else {
    dscfile = zstrdup(line);    /* remember single file name */
  }
}

/* accumulate verbatim PS header text for prolog */

void get_header_text (FILE *infile)   /* new 1993/Dec/29 */
{
  char *headernew;
  char *u;
  int n;

/*  if (headertext == NULL) headernew = malloc (nspecial+2); else */
/*  first time around, headertext will be NULL, so it acts like malloc */
  n = headertextlen + (int) nspecial + 2;
/*  headernew = realloc (headertext, n); */
  headernew = realloc (headertext, n);
  if (headernew == NULL) {
    showline(" Unable to allocate memory\n", 1);
/*    flush_special(infile); */
/*    errcount(0); */
/*    return; */
    checkexit(1);             /* 1995/July/15 */
//    more serious exit(1) ???
  }
  headertext = headernew;
  u = headernew + headertextlen;
  headertextlen = headertextlen + (int) nspecial + 1;
  while (nspecial-- > 0) *u++ = (char) getc(infile);
  *u++ = '\n'; *u++ = '\0';       /* terminating linefeed and \0 */
}

/* accumulate command line args for DVIPSONE - passed through DVI file */

void get_command_spec (FILE *infile)    // 99/Sept/6
{
  char *commandnew;
  char *u;
  int n;

/*  first time around, commandspec will be NULL, so it acts like malloc */
  n = commandspeclen + (int) nspecial + 2;
  commandnew = realloc (commandspec, n);
  if (commandnew == NULL) {
    showline(" Unable to allocate memory\n", 1);
    checkexit(1);
  }
  commandspec = commandnew;
  u = commandnew + commandspeclen;
  commandspeclen = commandspeclen + (int) nspecial + 1;
  while (nspecial-- > 0) *u++ = (char) getc(infile);
  *u++ = '\n'; *u++ = '\0';   // terminating linefeed and \0 
 }

/* accumulate verbatim PS header text for prolog */

void get_custom_text (FILE *infile)     /* new 1995/July/15 */
{
  int c, n, needpercent=0;
  char *customnew;
  char *u;

  c = getc(infile); ungetc(c, infile);
  if (c != '%') needpercent = 1;
/*  if (dsccustom == NULL) customnew = malloc (nspecial+2); else */
/*  first time around, dsccustom will be NULL, so it acts like malloc */
  n = dsccustomlen + (int) nspecial + 2;
  if (needpercent) n = n+2;
/*  customnew = realloc (dsccustom, n); */
  customnew = realloc (dsccustom, n);
  if (customnew == NULL) {
    showline(" Unable to allocate memory\n", 1);
/*    flush_special(infile); */
/*    errcount(0); */
/*    return; */
    checkexit(1);             /* 1995/July/15 */
//    more serious exit(1) ???
  }
  dsccustom = customnew;
  u = customnew + dsccustomlen;
  dsccustomlen = dsccustomlen + (int) nspecial + 1;
  if (needpercent) {
    *u++ = '%';   *u++ = '%';
    dsccustomlen = dsccustomlen + 2;
  }
  while (nspecial-- > 0) *u++ = (char) getc(infile);
  *u++ = '\n'; *u++ = '\0';       /* terminating linefeed and \0 */
}

void get_bbox (FILE *infile) /* Use for CropBox pdfmark not tested */
{
/*  Right now this is in PS coordinates, should be in TeX coordinates */
  if (get_token(infile, line, sizeof(line)) != 0) { /* MAXLINE */
    sscanf(line, "%d", &BBxll);
  }
  if (get_token(infile, line, sizeof(line)) != 0) { /* MAXLINE */
    sscanf(line, "%d", &BByll);
  }
  if (get_token(infile, line, sizeof(line)) != 0) { /* MAXLINE */
    sscanf(line, "%d", &BBxur);
  }
  if (get_token(infile, line, sizeof(line)) != 0) { /* MAXLINE */
    sscanf(line, "%d", &BByur);
  }
} 

/* accumulate Keywords for DOCINFO pdfmark */

void get_keywords (FILE *infile)     /* 1996/May/10 */
{
  char *keywordsnew;
  char *u;
  int n, c;
  int needcomma=0;    /* 0 or 2 if comma and space needed */

/*  if (headertext == NULL) headernew = malloc (nspecial+2); else */
/*  will add comma separator unless first, or comma or space already */
  if (keywordslen == 0) needcomma = 0;
  else {
    c = *(keywords + keywordslen - 1);
    if (c == ',' || c == ' ' || c == '\t') needcomma = 0;
    else needcomma = 2;
  }
/*  n = keywordslen + (int) nspecial + 2; */  /* space for , and \0 */
  n = keywordslen + (int) nspecial + needcomma + 1; 
/*  first time around, keywords will be NULL, so it acts like malloc(n) */
  keywordsnew = realloc (keywords, n);
  if (keywordsnew == NULL) {
    showline(" Unable to allocate memory\n", 1);
/*    flush_special(infile); */
/*    errcount(0); */
/*    return; */
    checkexit(1);             /* 1995/July/15 */
//    more serious exit(1) ???
  }
  keywords = keywordsnew;
  u = keywordsnew + keywordslen;
  if (needcomma > 0) {
    *u++ = ',';     /* add , if needed */
    *u++ = ' ';     /* add   if needed */
  }
  keywordslen = keywordslen + (int) nspecial + needcomma;
  while (nspecial-- > 0) *u++ = (char) getc(infile);
  *u++ = '\0';        /* terminating \0 */
}

// void get_common_string (FILE *infile, char *newstring) 
char *get_common_string (FILE *infile)
{
  char *u;
  char *newstring = (char *)malloc ((size_t) (nspecial+1));
  if (newstring == NULL) {
    showline(" Unable to allocate memory\n", 1);
    checkexit(1);
    return NULL;
  }
  u = newstring;
  while (nspecial-- > 0) *u++ = (char) getc(infile);
  *u++ = '\0';        /* terminating \0 */  
  return newstring;
}

// unadvertized ability to change Creator fieldin DocInfo

void get_creator (FILE *infile)
{
  if (creatorstring != NULL) return;  /* ignore all but first */
//  creatorstring = malloc ((size_t) (nspecial+1));
//  get_common_string(infile, creatorstring);
  creatorstring = get_common_string(infile);
}

void get_title (FILE *infile)
{
  if (titlestring != NULL) return;  /* ignore all but first */
//  titlestring = malloc ((size_t) (nspecial+1));
//  get_common_string(infile, titlestring);
  titlestring = get_common_string(infile);
}

void get_subject (FILE *infile)
{
  if (subjectstring != NULL) return;  /* ignore all but first */
//  subjectstring = malloc ((size_t) (nspecial+1));
//  get_common_string(infile, subjectstring);
  subjectstring = get_common_string(infile);
}

void get_author (FILE *infile)
{
  if (authorstring != NULL) return; /* ignore all but first */
//  authorstring = malloc ((size_t) (nspecial+1));
//  get_common_string(infile, authorstring);
  authorstring = get_common_string(infile);
}

void getbase (FILE *infile)
{
  if (basestring != NULL) return; /* ignore all but first */
//  basestring = malloc ((size_t) (nspecial+1));
//  get_common_string(infile, basestring);
  basestring = get_common_string(infile);
}

void get_page_mode (FILE *infile)
{
  if (pagemode != NULL) return; /* ignore all but first */
//  pagemode = malloc ((size_t) (nspecial+1));
//  get_common_string(infile, pagemode);
  pagemode = get_common_string(infile);
}

/* example \special{paper_size=5.04in,3.751in} */

void get_paper_size (FILE *infile)
{
//  if (strcmp(paper_size,"") != 0) return;  /* ignore all but first */
  if (paper_size != NULL) return;
//  paper_size = malloc ((size_t) (nspecial+1));
//  get_common_string(infile, paper_size);
  paper_size = get_common_string(infile);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int bComplainSpecial=1;

/* Attempt at \special{background rgb 0 0 1} support 98 June 30 */

void DoBackGround (FILE *infile, int c)
{
  char *s;
  int n, m;
  int setcolor=0;
  COLORSPEC SavedColor;

  if (bBackGroundFlag == 0)
  {
    flush_special(infile);
    return;
  }

  s = line + strlen(line);
  if (c > 0) *s++ = (char) c;       /* stick in terminator */
  *s = '\0';                /* just in case */
  (void) scan_special(input, line, MAXLINE);

  if (traceflag)
  {
    sprintf(logline, "\n%s %c (%d) ", line, c, c);
    showline(logline, 0);
  }

/*  if (c > 0) return; */         /* only do in prescan ! */
  s = line;
  if (*s == '\0') return;
  if (bKeepBlack) return;         /* 96/Nov/3 */

  SavedColor = CurrColor;         /* 99/Apr/06 */
  if (strncmp(s, "rgb", 3) == 0)
  {
    s += 3;
    m = sscanf(s, "%g %g %g%n\n", &CurrColor.A, &CurrColor.B, &CurrColor.C, &n);
    if (m == 3)
    {
      CurrColor.D = -1.0F;
      setcolor=1;
      s += n;
    }
    else
    {
      complainspecial(input);
      return;
    }
  }
  else if (strncmp(s, "cmyk", 4) == 0) {
    s += 4;
    m = sscanf(s, "%g %g %g %g%n",
         &CurrColor.A, &CurrColor.B, &CurrColor.C, &CurrColor.D, &n);
    if (m == 4) {
      setcolor=1;
      s += n;
    }
    else {
      complainspecial(input);
      return;
    }
  }
  else if (strncmp(s, "gray", 4) == 0) {
    s += 4;
    m = sscanf(s, "%g%n", &CurrColor.A, &n);
    if (m == 1) {
/*      CurrentC = CurrentB = CurrentA; CurrentD = -2.0F; */
      CurrColor.C = CurrColor.B = CurrColor.A;
      CurrColor.D = -2.0F;
      setcolor = 1;
      s += n;
    }
    else {
      complainspecial(input);
      return;
    }
  }
  else if (_strnicmp(s, "black", 5) == 0) {
    s += 5;
    setcolor=1;
/*    CurrentC = CurrentB = CurrentA = 0.0F; CurrentD = 1.0F; */
    CurrColor.C = CurrColor.B = CurrColor.A = 0.0F;
    CurrColor.D = 1.0F;   
  }
  else if (_strnicmp(s, "white", 5) == 0) {
    s += 5;
    setcolor=1;
/*    CurrentC = CurrentB = CurrentA = 0.0F;
    CurrentD = 0.0F; */
    CurrColor.C = CurrColor.B = CurrColor.A = 0.0F;
    CurrColor.D = 0.0F; 
  }
  else {
    complainspecial(input); /* 1995/April/15 */
    return;
  }

  if (traceflag)
  {
    sprintf(logline, " PAGENUMBER %d (%d %d) ", pagenumber, dvi_t, MaxColor);
    showline(logline, 0);
  }
  
/*  if (pagenumber < 0 || pagenumber >= dvi_t)  */
  if (pagenumber < 0 || pagenumber > dvi_t) {     /* 99/Feb/21 */
    sprintf(logline, " ERROR: bad page number %d\n", pagenumber);
    showline(logline, 1);
    return;
  }

  if (traceflag)
  {
    sprintf(logline, "\npage %d %g %g %g %g\n",
         pagenumber, CurrColor.A, CurrColor.B, CurrColor.C,
         CurrColor.D);
    showline(logline, 0);
  }
  BackColors[pagenumber] = CurrColor;     /* ??? */
  bBackUsed = 1;        /* mark background color used */
  CurrColor = SavedColor;         /* 99/Apr/06 */
}

  
#ifdef TPIC

/* TPIC commands */

/* char *tpiccommand[] = {
  "pa", "fp", "ip", "da", "dt", "sp", "pn", "ar", "ia", 
    "sh", "wh", "bk", "tx", ""
}; */

char *tpiccommands = "pa fp ip da dt sp pn ar ia sh wh bk tx";

#endif

/* Maybe check for TPIC specials only in selected page range ... */
/* Hmm, may want to search for inserted figure files here and extract */
/* DocumentNeededResources, DocumentFonts to save for writing in header ... */
/* No, use IncludeResource: font ... and IncludeFont: ... ? */
/* also check for \special{insertimage: ...} TIFF images re level2 96/Dec/20 */
/* also check for \special{color ...} */
/* also check for \special{background ...} 98/Jun/30 */

void logdo_com (FILE *infile)
{
  int c;
/*  int k=0; */

  if (bIgnoreSpecials)
  {
    flush_special(infile);
    return;
  }
  nspecialsav = nspecial;       /* 99/Feb/21 */
  specstart = ftell(input);     /* for complainspecial 99/Feb/21 */
  c = getc(infile);         /* peek ahead for ! */
  ungetc (c, infile);
  if (c == 0 && bFirstNull) {     /* is first byte null ? 96/Aug/29 */
    flush_special(infile);
    return;
  }
  if (c == '!') {
/*    if (verbatimflag != 0) { */   /* flushed 97/Mar/9 */
      c = getc(infile); nspecial--;
      get_header_text(infile);
/*    } */
    flush_special(infile);
    return;
  }
/*  c = get_alpha_token(infile, line, MAXLINE); */
  c = get_alpha_token(infile, line, sizeof(line)); /* MAXLINE */
#ifdef TPIC
/*  check whether maybe a TPIC \special */
  if (allowtpic != 0 && needtpic == 0 && (c == ' ' || c == 0)) {
/*    while (strcmp(tpiccommand[k], "") != 0) {
      if (strcmp(line, tpiccommand[k]) == 0) {
        needtpic++; break;
      }
      k++;
    } */
    if (strlen(line) == 2 && strstr(tpiccommands, line) != NULL)
        needtpic++; 
  }
#endif
  if (c == ' ' || c == ':') {
    if (strcmp(line, "color") == 0) {
      doColor(NULL, input, c, 0);   /* no PS output */
      bColorUsed = 1;   /* 98/Feb/14 */
    }
    else if (strcmp(line, "background") == 0) {
      DoBackGround (infile, c);
/*      bColorUsed = 1; */ /* ? */
    }
    else if (strcmp(line, "landscape") == 0) {    /* 99/Apr/5 foils.cls */
      bLandScape = ! bLandScape;          /* sets globally ! */
    }
  }
/*  check whether a special calling for a header or prolog file */
/*  if(c == '=' && strcmp(line, "header") == 0) get_header_name(infile); */
/*  Separator is `=' */
  if(c == '=') {            /* extended 93/Dec/29 */
/*    if (strcmp(line, "header") == 0) get_header_name(infile); */
    if (_strcmpi(line, "header") == 0) get_header_name(infile);
/*    else if (strcmp(line, "headertext") == 0) get_header_text(infile); */
    else if (_strcmpi(line, "headertext") == 0) get_header_text(infile);
/*    following added in 1995 July */
    else if (strcmp(line, "DSCheader") == 0) get_custom_name(infile);
    else if (strcmp(line, "DSCtext") == 0) get_custom_text(infile);
    else if (strcmp(line, "paper_size") == 0) get_paper_size(infile);
    else if (strcmp(line, "DVIPSONE") == 0) get_command_spec(infile);
    else if (strcmp(line, "DVIWindo") == 0) flush_special(infile);
/*    else complain ??? */
  }
/*  else if (c == ':' && strcmp(line, "dvitops") == 0) { */
/*  Separator is `:' */
  else if (c == ':') {
    if (strcmp(line, "dvitops") == 0) {
/*      (void) get_alpha_token(infile, line, MAXLINE); */
      (void) get_alpha_token(infile, line, sizeof(line)); /* MAXLINE */
      if (strcmp(line, "prolog") == 0) get_header_name(infile);
    }
    else if (strcmp(line, "PDF") == 0) {    /* 1996/July/4 */
      c = get_alpha_token(infile, line, sizeof(line));   /* MAXLINE */
      if (c == ' ' || c == '=') {
        if (_strcmpi(line, "Keywords") == 0) get_keywords(infile);
        else if (strcmp(line, "BBox") == 0) get_bbox(infile); 
        else if (_strcmpi(line, "Creator") == 0) get_creator(infile);
        else if (_strcmpi(line, "Title") == 0) get_title(infile);
        else if (_strcmpi(line, "Subject") == 0) get_subject(infile);
        else if (_strcmpi(line, "Author") == 0) get_author(infile);
        else if (_strcmpi(line, "Base") == 0) getbase(infile);
        else if (_strcmpi(line, "PageMode") == 0) get_page_mode(infile);
      }
    }
/*    check whether TIFF image inserted re level2 features 96/Dec/20 */
    else if (strcmp(line, "insertimage") == 0)  bInsertImage++;
  }
  flush_special(infile);
}

void logdo_xxxi(FILE *infile, unsigned int n)
{
/*  unsigned int k; */
  nspecial = (long) n;
  logdo_com(infile);
/*  for(k = 0; k < n; k++)  getc(infile); */
}

void logdo_xxx1(FILE *infile) /* for /special */
{
  unsigned int k;
  k = getc(infile);
  logdo_xxxi(infile, k);
}

void logdo_xxx2(FILE *infile) /* for /special */
{
  unsigned int k;
  k = ureadtwo(infile);
  logdo_xxxi(infile, k);
}

void logdo_xxxl(FILE *infile, unsigned long n)
{
/*  unsigned long k; */
  nspecial=(long) n;
  logdo_com(infile);
/*  for(k = 0; k < n; k++)  getc(infile); */
}

void logdo_xxx3(FILE *infile)
{
  logdo_xxxl(infile, ureadthree(infile));
}

void logdo_xxx4(FILE *infile)
{
  logdo_xxxl(infile, ureadfour(infile));
}

/* need to do this even if skipping pages */

void logfnt_def(FILE *infile, unsigned int k)
{
  int fn;
  unsigned int na, nl, i;
  int newfont=1;    /* if this is a new one (not defined before) */
//  char *tempfont;
  char *fp;
  char namebuffer[FNAMELEN];

  if (finx[k] != BLANKFONT) {   /* seen this font before !!! */
    sprintf(logline, " ERROR: Font %d being redefined ", k);
    showline(logline, 1);
    tellwhere(infile, 1);
/*    errcount(0); */
    newfont = 0;
    fn = finx[k];
  }
  else {        /* definition of font not seen before */
    fn = fnext++;  /* grab next slot */
    finx[k] = (short) fn;
    if (fnext > maxfonts) {     /* 94/May/23 */
      sprintf(logline, " ERROR: More than %d fonts in use\n", maxfonts);
      showline(logline, 1);
      fnext--;
/*      errcount(0); */
      checkexit(1);     /* 1993/Dec/11 */
    }
  }
  
  fc[fn] = ureadfour(infile);   /* read checksum (encoding info) */
/*  (void) ureadfour(infile); */
  fs[fn] = ureadfour(infile);   /* read at size */
/*  fd[fn] = ureadfour(infile); */  /* design size */
  (void) ureadfour(infile);   /* skip over design size */
  na = getc(infile);
  nl = getc(infile);
  if (newfont == 0) { /* just skip over if already defined */
    for (i = 0; i < na + nl; i++) (void) getc(infile);
    return;
  }
/*  fp = fontname[fn]; */
  fp = namebuffer;
  if (na + nl >= sizeof(namebuffer)-1) {  /* FNAMELEN */
    sprintf(logline, " Font name too long: %d (> %d) ",
        na + nl, sizeof(namebuffer)-1);
    showline(logline, 1);
    showline("\n", 0);
//    errcount(0);
    tellwhere(infile, 1);
    for (i = 0; i < na+nl; i++) (void) getc(infile);
  }
  else {
    for (i = 0; i < na+nl; i++) *fp++ = (char) getc(infile);
  }
  *fp++ = '\0';
  if (fontname[fn] != NULL) free(fontname[fn]);
  fontname[fn] = zstrdup(namebuffer);
/*  strcpy(subfontname[fn], ""); */   /* blank it out */
  if (subfontname[fn] != NULL) {    /* blank it out */
    free(subfontname[fn]);
    subfontname[fn] = NULL; 
  }
//  strcpy(fontvector[fn], "");     /* 1992/May/4 */
//  *(fontvector + fn * MAXVECNAME) = '\0';     /* blank it out */
  if (fontvector[fn] != NULL) {
    free(fontvector[fn]);
    fontvector[fn] = NULL;      /* blank it out */
  }
  fontsubflag[fn] = -1;       /* all this goes to extract now */
  fontproper[fn] = 0;         /* 1992/May/4 */
/*  possibly determine whether we need to reencode *control* range ??? */
/*  if (substitute != 0) fontsubflag[fn] = font_remap(fontname[fn]);
  if (uppercaseflag != 0) uppercase(font, fontname[fn]);  else */ 
/*  strcpy(font, fontname[fn]); */    /* what for ??? */
//  tempfont = fontchar[fn];      /* reset character counts */
//  tempfont = fontchar + MAXCHRS * fn;
/*  for (i = 0; i < MAXCHRS; i++) tempfont[i] = 0;  */
//  for (i = 0; i < MAXCHRS; i++) *tempfont++ = 0;    /* 1994/Feb/3 */
  if (fontchar[fn] == NULL) {
    fontchar[fn] = (char *) malloc(MAXCHRS);
    if (fontchar[fn] == NULL) {
      showline(" Unable to allocate memory\n", 1);
      checkexit(1);
      return;
    }
  }
  memset(fontchar[fn], 0, MAXCHRS);
}

void logdo_fnt_def1(FILE *infile) /* define font */
{
  unsigned int k;
/*  k = ureadone(infile); */
  k = getc(infile);
  logfnt_def(infile, k);
}

void logdo_fnt_def2(FILE *infile) /* define font */
{
  unsigned int k;
  k = ureadtwo(infile);
  if (k >= MAXFONTNUMBERS) k = MAXFONTNUMBERS-1;
  logfnt_def(infile, k);
}

void logdo_fnt_defsub(FILE *infile, unsigned long k)
{
  if (k >= MAXFONTNUMBERS) k = MAXFONTNUMBERS-1;
  logfnt_def(infile, (unsigned int) k);
}

void logdo_fnt_def3(FILE *infile) /* define font */
{
/*  unsigned long k;
  k = ureadthree(infile); */
  logdo_fnt_defsub(infile, ureadthree(infile));
}

void logdo_fnt_def4(FILE *infile) /* define font */
{
  long k;
  k = sreadfour(infile);
  if (k < 0) k = 0;
  logdo_fnt_defsub(infile, (unsigned long) k);
}

/* need to do this even if skipping pages */

void logdo_pre(FILE *infile)
{
  unsigned int i, k, j;
  int c;
  char *s;
  
/*  i = ureadone(infile); */
  i = getc(infile);
  if (i < 1 || i > 3)
  {
    showline("Not a valid DVI file ", 1);
    giveup(3); 
    return;
  }
  else if (i != ID_BYTE)
  {
    sprintf(logline, "File is DVI version %d - *not* %d\n",
      i, ID_BYTE);
    showline(logline, 1);
    errcount(0);
  }
  num = ureadfour(infile);
  den = ureadfour(infile);
  mag = ureadfour(infile);
/*  k = ureadone(infile); */
  k = getc(infile);         /* bytes needed for TeX's comment */
/*  s = comment; */           /* was to char comment[MAXCOMMENT] */
//  if (strcmp(comment, "") != 0) {   /* free if still in use */
  if (comment != NULL) {    /* free if still in use */
    free(comment);
//    comment = "";
    comment = NULL;
  }
  comment = malloc(k+1);
  if (comment == NULL)
  {
    showline(" Unable to allocate memory\n", 1);
    checkexit(1);
//    more serious exit(1) ???
  }
  s = comment;
/*  if (traceflag) fprintf(stdout, "Comment:"); */
  c = getc(infile);         /* try and discard initial space */
  if (c == ' ') k--;
  else (void) ungetc(c, infile);
  for (j=0; j < k; j++)
  {
    c = getc(infile);
    if (j < MAXCOMMENT) *s++ = (char) c;
/*    if (verboseflag) putc(c, stdout); */
  }
  *s++ = '\0';
  if (verboseflag) {
    showline(comment, 0);
    showline("\n", 0);
  }
  if (textures != 0) 
    (void) ureadfour(infile); /* flush length code */
}

/* need to do this even if skipping pages */

void logdo_post(FILE *infile)
{
/*  int k; */
  previous = sreadfour(infile); /* was ureadfour ... */
  num = ureadfour(infile);
  den = ureadfour(infile);
  mag = ureadfour(infile);

  if (traceflag)
  {
    sprintf(logline, " POST: previous %ld num %ld den %ld mag %ld\n",
              previous, num, den, mag);
    showline(logline, 0);
  }
/* compare these with what was in preamble ? */

  dvi_l = ureadfour(infile);      /* max page height plus depth */
  dvi_u = ureadfour(infile);      /* max page width */
  dvi_s = (int) ureadtwo(infile);   /* max stack depth */
  dvi_t = (int) ureadtwo(infile);   /* number bops limit 65535 */
  if (traceflag) {
    sprintf(logline, "l %ld u %ld s %ld t %ld\n",
              dvi_l, dvi_u, dvi_s, dvi_t);
    showline(logline, 0);
  }
/*  here l and u could be used for bbox info ? */
/*  except: don't include headers and footers and other problems */ 
  finish = -1; 
}

/* could do this even in forward mode to check on number of pages ? */

void logdo_post_post(FILE *infile) /* only in reverse ? */
{
/*  unsigned long q;   */
/*  unsigned int i;    */

  if (traceflag) showline("Hit POSTPOST!\n", 0);

/*  q = ureadfour(infile); */
  (void) ureadfour(infile);
/*  i = ureadone(infile); */
  (void) getc(infile);
/*  check ID_BYTE again ? */
/*  followed by at least four 223's */
/*  if (reverseflag != 0) fseek(infile, previous, SEEK_SET);
  else fputs("%% This is really the end !\n", output); */
}

/* This version scans for Textures length code followed by pre & DVI code */
/* could do something more fancy to get quickly to resource fork */
/* should be fairly safe, unless initial length code is > 256 */
/* Search for 3 or more zeros in a row, followed by dont-care (length) */
/* - followed by pre and ID_BYTE */

int readovertext(FILE *infile)
{
  int c, n;

  c = getc(infile);
  for(;;) {
/*    if ((c = getc(infile)) == 0) { */
    if (c == 0) {
      n = 1;
      while ((c = getc(infile)) == 0) n++;
      if (c == EOF) return 0;
      if (n >= 3) {
        if((c = getc(infile)) == (int) pre) {
          (void) ungetc(c, infile);
          dvistart = ftell(infile);
          c = getc(infile);
          if ((c = getc(infile)) == ID_BYTE) {
            if (fseek(infile, dvistart, SEEK_SET) != 0)
              return 0;   /* seek error */
            else return -1;   /* think we found it ! */
          }
        }
      }
    }
    else if ((c = getc(infile)) == EOF) return 0;
  } 
}

void resetpagerangehit (int flag)
{
/*  int k; */
/*  for (k = 0; k < rangeindex; k++) pagerangehit[k] = 0; */  /* 1994/Jan/16 */
/*  currentrange = -1; */
  currentpage = -LINFINITY; /* indicate first time */
  prescanflag = flag;     /* remember whether in prescan or not */
  if (prescanflag != 0 || reverseflag == 0)
     pagesequence = 1;    /* 1994/Feb/16 */
/* NOTE: don't reset page sequence instance if going in reverse order */
}

/***************************************************************************/

void alloccolorsave (int npages)
{
  int k;

  if (ColorStacks != NULL)
  {
    showline(" ERROR: color save stacks allocation\n", 1);
    freecolorsave();
  }
#ifdef DEBUGCOLORSTACK
  if (traceflag)
  {
    sprintf(logline, "Allocating color save stack for %d pages\n", npages);
    showline(logline, 0);
  }
#endif
  if (npages == 0) npages = 1;
  ColorStacks = (COLORSPEC **) malloc((npages+1) * sizeof(COLORSPEC *));
  if (ColorStacks == NULL) {
    showline(" Unable to allocate memory\n", 1);
    checkexit(1);
//    more serious exit(1) ???
  }
  for (k = 0; k <= npages; k++) ColorStacks[k] = NULL;
  MaxColor = npages+1;      /* make note of size of allocation */
}

void freecolorsave (void)
{
  int k, npages = MaxColor;
  
  if (ColorStacks == NULL) return;
#ifdef DEBUGCOLORSTACK
  if (traceflag) showline("Freeing Saved Color Stacks\n", 0);
#endif
  for (k = 0; k < npages; k++) {
    if (ColorStacks[k] != NULL) {
      free(ColorStacks[k]);
      ColorStacks[k] = NULL;
    }
  }
  if (ColorStacks != NULL) {
    free(ColorStacks);
    ColorStacks = NULL;
  }
}

#ifdef DEBUGCOLORSTACK
void dumpcolorsave (void)    /* debugging only */
{
  int k, m, i, npages = MaxColor-1;
  COLORSPEC *ColorSaved;

  if (ColorStacks == NULL)
  {
    showline(" No saved color stacks to show\n", 1);
    return;
  }
  sprintf(logline, " Saved color stacks for %d pages after prescan:\n", npages);
  showline(logline, 1);
/*  for (k = 0; k < npages; k++) { */
  for (k = 1; k <= npages; k++) {
    if (ColorStacks[k] != NULL) {
      sprintf(logline, "For page %d:\n", k);
      showline(logline, 1);
      ColorSaved = ColorStacks[k];
      m = (int) (ColorSaved[0].D + 0.5);
      for (i = 1; i <= m; i++) {
        sprintf(logline, "%d\t%g\t%g\t%g\t%g\n", i,
             ColorSaved[i].A,  ColorSaved[i].B,
             ColorSaved[i].C,  ColorSaved[i].D);
        showline(logline, 1);
      }
    }
    else {
      sprintf(logline, " ERROR: ColorStack[%d] is NULL\n", k);
      showline(logline, 1);
    }
  }
}
#endif

void allocbackground (int npages)
{
  int k;
  if (BackColors != NULL) {
    showline(" ERROR: background allocation\n", 1);
    freebackground();
  }
  if (npages == 0) npages = 1;
/*  BackColors = (COLORSPEC *) malloc(npages * sizeof(COLORSPEC)); */
  BackColors = (COLORSPEC *) malloc((npages+1) * sizeof(COLORSPEC));
  if (BackColors == NULL) {
    showline(" Unable to allocate memory\n", 1);
    checkexit(1);
//    more serious exit(1) ???
  }
/*  for (k = 0; k < npages; k++) */
  for (k = 0; k <= npages; k++) {     /* may not be needed */
    BackColors[k].A = BackColors[k].B = BackColors[k].C = -1.0F;
    BackColors[k].D = -1.0F;
  }
}

void freebackground (void) {
  if (BackColors != NULL) free(BackColors);
  BackColors = NULL;
}

/***************************************************************************/

int scanlogfileaux(FILE *fp_in)
{
  int c, k, fn;
/*  long filptr; */

//  strcpy (headerfile, "");        /* reset to no headers seen */
  if (headerfile != NULL) free(headerfile);
  headerfile = NULL;
  if (countzeroflag) resetpagerangehit (1);

  numpages = 0;     /* number of pages actually processed 94/Oct/12 */
  pagenumber = 0;     /* pages seen in scan */

  ff = -1;            /* redundant */
  for (k = 0; k < MAXFONTS; k++) fonthit[k] = 0;
/*  for (k = 0; k < maxfonts; k++) fonthit[k] = 0; */ /* ah what the hell */
//  currentfont = fontchar;     /* just in case */
  currentfont = fontchar[0];    /* just in case ??? */
  fnext = 0;
  for (k = 0; k < MAXFONTNUMBERS; k++)  /* reset status to unused */
    finx[k] = (short) BLANKFONT;

/*  Get dvi_t up front 98/Jun/30 */
  postposition = -1;
  bBackUsed=0;      /* non-zero of \special{background ...} used */
  bColorUsed = 0;     /* assume no color \special until ... 98/Feb/15 */

  if (bCarryColor || bBackGroundFlag)
  {
    postposition = goto_post(input); /* in dvianal.c */
    (void) getc(input);       /* absorb the post byte */
    logdo_post(input);
    rewind(input);
    if (bCarryColor)
      alloccolorsave(dvi_t);    /* allocated space for color table */
    if (bBackGroundFlag)
      allocbackground(dvi_t);   /* allocate background color table */
    pagenumber = 0;
    pageno = 0;
    finish = 0;
  }

  finish = 0;
  stinx = 0; maxstinx = 0;    /* redundant, hopefully */
  
  textures=0;     /* start off by assuming normal DVI file */
  c = getc(fp_in);
  (void) ungetc(c, fp_in);
  if (c != (int) pre) { /* not standard DVI file - can figure out ? */
    if (readovertext(fp_in) == 0) {
      if (strstr(fn_in, ".tex") != NULL) {      /* 1994/Feb/24 */
        showline("Can't find DVI file ", 1);
      }
      else {
        showline("Not a valid DVI (or Textures) file ", 1);
      }
      input = NULL;  /* to stop at byte message ? */
      giveup(3);
      return -1;
    }
    else {
      if (verboseflag) {
        showline("Textures DVI file - ", 0);
      }
      textures=1;
    }
  }

/* in the above, may also want to look 100 bytes into the file for start */
/* some Mac files come that way... */

  for(;;) {
    c = getc(fp_in);
    if (c == EOF) {
      sprintf(logline, " Unexpected EOF (%s)\n", "scanlogfile");
      showline(logline, 1);
      errcount(0);
      finish = -1;
/*      giveup(13); */
    }
    if (c < 128) {
      if (skipflag == 0) {
        if (ff < 0) invalidset((int) c);
        else {
/*          if (bRemapControl && c < MAXREMAP) c = remaptable[c]; */
          if (bRemapControl || bRemapFont) {
            if (c < MAXREMAP) c = remaptable[c];
#if MAXREMAP < 128
            else if (c == 32) c = 195;
            else if (c == 127) c = 196;
#endif
          }
          else if (bRemapSpace && c <= 32) {      /* 1995/Oct/17 */
            if (c == 32) c = 195;   /* not 160 */
            else if (c == 13) c = 176;  /* 1996/June/4 */
            else if (c == 10) c = 173;  /* 1996/June/4 */
            else if (c == 9) c = 170;   /* 1996/June/4 */
            else if (c == 0) c = 161;
          }
          currentfont[c]=1;
        }
      }
    }
    else if (c >= 171 && c <= 234) { /* switch to font (c - 171) */
      fn = (c - 171);
      log_switch_font(fn, fp_in);
    }
    else {
      switch(c) {
        case set1: logdo_set1(fp_in); break;
        case set2: logdo_set2(fp_in); break;  /* silly */
        case set3: logdo_set3(fp_in); break;  /* silly */
        case set4: logdo_set4(fp_in); break;  /* silly */
        case set_rule: logdo_set_rule(fp_in); break;
        case put1: logdo_put1(fp_in); break ;
        case put2: logdo_put2(fp_in); break;  /* silly */
        case put3: logdo_put3(fp_in); break;  /* silly */
        case put4: logdo_put4(fp_in); break;  /* silly */
        case put_rule: logdo_put_rule(fp_in); break;  
        case nop: break;      /* do nothing */
        case bop: logdo_bop(fp_in); break;
        case eop: logdo_eop(fp_in); break;
        case push: logdo_push(); break;
        case pop: logdo_pop(); break;
        case right1: logdo_right1(fp_in); break;
        case right2: logdo_right2(fp_in); break;  
        case right3: logdo_right3(fp_in); break; 
        case right4: logdo_right4(fp_in); break; 
        case w0: logdo_w0(); break;
        case w1: logdo_w1(fp_in); break;
        case w2: logdo_w2(fp_in); break; 
        case w3: logdo_w3(fp_in); break; 
        case w4: logdo_w4(fp_in); break;  /* not used ? */
        case x0: logdo_x0(); break;
        case x1: logdo_x1(fp_in); break;
        case x2: logdo_x2(fp_in); break; 
        case x3: logdo_x3(fp_in); break; 
        case x4: logdo_x4(fp_in); break;  /* not used ? */
        case down1: logdo_down1(fp_in); break;
        case down2: logdo_down2(fp_in); break; 
        case down3: logdo_down3(fp_in); break; 
        case down4: logdo_down4(fp_in); break; 
        case y0: logdo_y0(); break;
        case y1: logdo_y1(fp_in); break;
        case y2: logdo_y2(fp_in); break; 
        case y3: logdo_y3(fp_in); break; 
        case y4: logdo_y4(fp_in); break;  /* not used ? */
        case z0: logdo_z0(); break;
        case z1: logdo_z1(fp_in); break;
        case z2: logdo_z2(fp_in); break; 
        case z3: logdo_z3(fp_in); break; 
        case z4: logdo_z4(fp_in); break;  /* not used ? */
        case fnt1: logdo_fnt1(fp_in); break;
        case fnt2: logdo_fnt2(fp_in); break;  /* silly */
        case fnt3: logdo_fnt3(fp_in); break;  /* silly */
        case fnt4: logdo_fnt4(fp_in); break;  /* silly */
        case xxx1: logdo_xxx1(fp_in); break;
        case xxx2: logdo_xxx2(fp_in); break; /* not used ? */
        case xxx3: logdo_xxx3(fp_in); break; /* not used ? */
        case xxx4: logdo_xxx4(fp_in); break; 
        case fnt_def1: logdo_fnt_def1(fp_in); break;
        case fnt_def2: logdo_fnt_def2(fp_in); break;  /* silly */
        case fnt_def3: logdo_fnt_def3(fp_in); break;  /* silly */
        case fnt_def4: logdo_fnt_def4(fp_in); break;  /* silly */
        case post: logdo_post(fp_in); break;
        case pre: logdo_pre(fp_in); break;
        case post_post: logdo_post_post(fp_in); break;
  
        default: {
          sprintf(logline, 
            " ERROR: Unrecognized DVI command: %d", c);
          showline(logline, 1);
          tellwhere(fp_in, 1);
/*          errcount(0); */
          finish = -1;    /* ? */
/*          giveup(7); */
         }
         break;
      }
    }
    if (finish != 0) break;
    if (bAbort) abortjob(); /* fine grained */
    if (abortflag) break;
  }
/*  if (maxstinx >= maxstack-1) {
    showline( WARNING: The PS stack will probably overflow %d > %d\n",
        maxstinx, maxstack -1 );
    errcount(0);
  }  */
  if (abortflag) return -1;
  return 0;
}

/* main entry point, prescan DVI file font usage, \specials */

int scanlogfile (FILE *fp_in)
{
  int c, d;
  input = fp_in;      /* remember file handle */

  if (traceflag) showline("Start PreScan DVI file\n", 0);
/*  strcpy (headerfile, ""); */ /* reset to no headers seen */
  c = getc(fp_in);
  d = getc(fp_in);
  rewind(fp_in);
//  we now forget about Textures files 99/July/14
  if (c != pre || d != ID_BYTE)
  {
    sprintf(logline, " Not a proper DVI file `%s'\n",
        (filenamex != NULL) ? filenamex : "");
    showline(logline, 1);
    errcount(0);
    return -1;
  }
  scanlogfileaux(fp_in);
/*  if (showlogflag != 0) showlog(stdout); */
  if (bBackGroundFlag != 0 && bBackUsed == 0)
    freebackground();     /* not needed in this case */
  if (bCarryColor != 0 && bColorUsed == 0)
    freecolorsave();      /* not needed in this case */
  if (traceflag) showline("End PreScan DVI file\n", 0);
#ifdef DEBUGCOLORSTACK
  if (traceflag) dumpcolorsave();
#endif
  if (abortflag) return -1;
  return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Use `texfonts.map' in directory path TEXFONTS for aliasing */

#define FONTMAP

#ifdef FONTMAP

typedef struct map_element_struct {   /* list element key . value pair */
  char *key;
  char *value;
  struct map_element_struct *next;
} map_element_type;

typedef map_element_type **map_type;

/* **************************** auxiliary functions *********************** */

static void complain_mem (unsigned int size)  /* out of memory */
{
  sprintf(logline, "Unable to honor request for %u bytes.\n", size);
  showline(logline, 1);
  checkexit(1);
}

static void *xmalloc (unsigned int size)
{
  void *new_mem = (void *) malloc (size);
  if (new_mem == NULL) complain_mem(size);
  return new_mem;
}

static void *xrealloc (void *old_ptr, unsigned int size)
{
  void *new_mem;
  if (old_ptr == NULL)
/*    new_mem = xmalloc (size); *//* could just let realloc do this case? */
    new_mem = malloc (size);  /* could just let realloc do this case? */
  else {
    new_mem = (void *) realloc (old_ptr, size);
/*    if (new_mem == NULL) complain_mem(size); */
  }
  if (new_mem == NULL) complain_mem(size);
  return new_mem;
}

static char *xstrdup (char *s)
{
  char *new_string = (char *) xmalloc (strlen (s) + 1);
  return strcpy (new_string, s);
}

/* static char *concat3 (char *s1, char *s2, char *s3) {
  char *answer
    = (char *) xmalloc (strlen (s1) + strlen (s2) + strlen (s3) + 1);
  strcpy (answer, s1);
  strcat (answer, s2);
  strcat (answer, s3);
  return answer;
} */  /* used by extend_filename only */

static void *xcalloc (unsigned int nelem, unsigned int elsize)
{
  void *new_mem = (void *) calloc (nelem, elsize);
  if (new_mem == NULL) complain_mem (nelem * elsize);
  return new_mem;
}

/*  Here we work only with suffix-free names - so this is silly */

/* static char *find_suffix (char *name) {
  char *dot_pos; 
  char *slash_pos; 
  
  dot_pos = strrchr (name, '.');
  if (dot_pos == NULL) return NULL; 
  if ((slash_pos = strrchr (name, '/')) != NULL) ;
  else if ((slash_pos = strrchr (name, '\\')) != NULL) ;  
  else if ((slash_pos = strrchr (name, ':')) != NULL) ;
  else slash_pos = name;
  if (dot_pos < slash_pos) return NULL;
  return dot_pos + 1;
} */

/* static char *extend_filename (char *name, char *default_suffix) {
  char *suffix = find_suffix (name);
  char *new_s;
  if (suffix != NULL) return name;  
  new_s = concat3 (name, ".", default_suffix);
  return new_s;           
} */

/* static char *remove_suffix (char *s) {
  char *ret;
  char *suffix = find_suffix (s);
  
  if (suffix == NULL) return NULL;
  suffix--;
  ret = (char *) xmalloc (suffix - s + 1);
  strncpy (ret, s, suffix - s);
  ret[suffix - s] = 0;
  return ret; 
} */

/* only used by fontmap.c */ /* why not just use fgets on global line ? */

#define BLOCK_SIZE 40

static char *read_line (FILE *f)
{
  int c;
  unsigned int limit = BLOCK_SIZE;
  unsigned int loc = 0;
  char *line;

  line = (char *) xmalloc (limit);
  
  while ((c = getc (f)) != EOF && c != '\n') {
    line[loc] = (char) c;
    loc++;
    if (loc == limit) {
      limit += BLOCK_SIZE;
      line = (char *) xrealloc (line, limit);
    }
  }
  
  if (c != EOF) line[loc] = 0;    /* not EOF */
  else if (loc > 0) line[loc] = 0;  /* c == EOF, but line not empty */
  else {    /* c == EOF and nothing on the line --- at end of file.  */
    free (line);
    line = NULL;
  }
  return line;
}

/* ************************************************************************* */

/* Fontname mapping.  We use a straightforward hash table. Should be prime? */

#define MAP_SIZE 307

/* The hash function.  We go for simplicity here.  */

static unsigned int map_hash (char *key)
{
  unsigned int n = 0;
  char *s = key;
/*  There are very few font names which are anagrams of each other
  so no point in weighting the characters.  */
  while (*s != 0) n += *s++;
  n %= MAP_SIZE;
#ifdef DEBUGALIAS
  if (traceflag) {
    sprintf(logline, "hash %u for %s\n", n, key);
    showline(logline, 0);
  }
#endif
  return n;
}

/* Look up STR in MAP.  Return the corresponding `value' or NULL.  */

static char *map_lookup_str (map_type map, char *key)
{
  unsigned int n = map_hash (key);
  map_element_type *p;
  
  for (p = map[n]; p != NULL; p = p->next) {
#ifdef DEBUGALIAS
    if (traceflag) {
      sprintf(logline, "Trying %s against %s\n", key, p->key);
      showline(logline, 0);
    }
#endif
    if (strcmp (key, p->key) == 0) return p->value;
#ifdef DEBUGALIAS
    if (traceflag) {
      sprintf(logline, "p->next %p\n", p->next);
      showline(logline, 0);
    }
#endif
  }
#ifdef DEBUGALIAS
  if (traceflag) {
    sprintf(logline, " failed to find %s\n", key);
    showline(logline, 0);
  }
#endif
  return NULL;          /* failed to find it */
}

#ifdef DEBUGALIAS
static void map_show (map_type map) /* debugging tool */
{
  map_element_type *p;
  unsigned int n;
  
  for (n = 0; n < MAP_SIZE; n++) {
    for (p = map[n]; p != NULL; p = p->next) {
      sprintf(logline, "n %u key %s next %p\n", n, p->key, p->next);
      showline(logline, 0);
    }
  }
}
#endif

/*  Look up KEY in MAP; if it's not found, remove any suffix from KEY and
  try again.  Then paste key back into answer ... */

/* OK, the original KEY didn't work.  Let's check for the KEY without
    an extension -- perhaps they gave foobar.tfm, but the mapping only
    defines `foobar'.  */

/* Append the same suffix we took off, if necessary.  */
/*  if (ret) ret = extend_filename (ret, suffix); */

char *map_lookup (map_type map, char *key)
{
  char *ret = map_lookup_str(map, key);
/*  char *suffix; */
  
/*  lets assume we never have to deal with names that have extensions */
/*  suffix = find_suffix (key); 
  if (!ret) {
    if (suffix) {
      char *base_key = remove_suffix (key);
      ret = map_lookup_str(map, base_key);
      free (base_key);
    }
  }
  if (ret && suffix) ret = extend_filename (ret, suffix);  */

  return ret;
}

/* If KEY is not already in MAP, insert it and VALUE.  */
/* This was a total mess! Fixed 1994/March/18 */

static void map_insert (map_type map, char *key, char *value)
{
  unsigned int n = map_hash (key);
  map_element_type **ptr = &map[n];

  while (*ptr != NULL && !(strcmp(key, (*ptr)->key) == 0))
    ptr = &((*ptr)->next);

  if (*ptr == NULL) {
    *ptr = (map_element_type *) xmalloc (sizeof(map_element_type));
    (*ptr)->key = xstrdup (key);
    (*ptr)->value = xstrdup (value);
    (*ptr)->next = NULL;
  }
}

/* Open and read the mapping file FILENAME, putting its entries into
   MAP. Comments begin with % and continue to the end of the line.  Each
   line of the file defines an entry: the first word is the real
   filename (e.g., `ptmr'), the second word is the alias (e.g.,
   `Times-Roman'), and any subsequent words are ignored.  .tfm is added
   if either the filename or the alias have no extension.  This is the
   same order as in Dvips' psfonts.map; unfortunately, we can't have TeX
   read that same file, since most of the real filenames start with an
   `r', because of the virtual fonts Dvips uses.  
   And what a load of bull! And who cares about DVIPS and VF files !*/

static void map_file_parse (map_type map, char *map_filename)
{
  char *l;
  unsigned int map_lineno = 0;
  unsigned int entries = 0;
  FILE *f = fopen(map_filename, "r");
  
  if (f == NULL) {
    sprintf(logline, " ERROR: Can't open %s\n", map_filename);
    showline(logline, 1);
    perrormod(map_filename);
    return;
  }

  while ((l = read_line(f)) != NULL) {
    char *filename;
    char *comment_loc;

/*    comment_loc = strrchr (l, '%'); */
    comment_loc = strchr(l, '%');         /* 96/Aug/20 */
/*    if (comment_loc == NULL) comment_loc = strrchr (l, ';'); */
    if (comment_loc == NULL) comment_loc = strchr (l, ';');
      
/*    Ignore anything after a % or ;  */
    if (comment_loc)  *comment_loc = 0;
      
    map_lineno++;
      
/*    If we don't have any filename, that's ok, the line is blank.  */
    filename = strtok (l, " \t");
    if (filename) {
      char *alias = strtok (NULL, " \t");
          
      /* But if we have a filename and no alias, something's wrong.  */
      if (alias == NULL || *alias == 0) {
        sprintf(logline,
          " font name `%s', but no alias (line %u in `%s').\n",
            filename, map_lineno, map_filename);
        showline(logline, 1);
      }
      else {       /* We've got everything.  Insert the new entry.  */
        map_insert (map, alias, filename); /* alias is the key */
        entries++;
      }
    }
    free (l);
  }
/*  xfclose (f, map_filename); */
  fclose (f);
#ifdef DEBUGALIAS
  if (traceflag) {
    sprintf(logline, "%u entries\n", entries);
    showline(logline, 0);
  }
#endif
}

/* Look for the file `texfonts.map' in each of the directories in
   TEXFONTS.  Entries in earlier files override later files.  */

/* uses _searchenv ? */

/* map_type map_create (char *envvar) { old version
  char filename[_MAX_PATH];
  map_type map;
      
  _searchenv ("texfonts.map", envvar, filename);
  if (*filename == '\0') return NULL;

  map = (map_type) xcalloc (MAP_SIZE, sizeof (map_element_type *));
  map_file_parse (map, filename);
#ifdef DEBUG
  if (traceflag) map_show(map);
#endif
  return map;
} */

/* Look for the file `texfonts.map' in each of the directories in
   TEXFONTS.  Entries in earlier files override later files.  */

/* void oursearchenv (char *mapname, char *envvar, char *pathname) { */
void oursearchenv (char *mapname, char *searchpath, char *pathname)
{
/*  char *searchpath; */          /* 97/May/10 */
  int foundfile=0;            /* set, but not used ? */
#ifndef SUBDIRSEARCH
  FILE *input;
  char *s;
#endif

/*  searchpath = _getenv(envvar); */
/*  searchpath = grabenv(envvar); */    /* 97/May/10 */
  if (searchpath == NULL) {       /* 1996/July/30 */
    *pathname = '\0';         /* failed env var lookup */
    return;
  }                   /* new sanity check */
#ifdef SUBDIRSEARCH
  if (searchalongpath(mapname, searchpath, pathname, 0) != 0)
    *pathname = '\0';
  else foundfile = 1;           /* 1994/Aug/18 */
#else
  for (;;) {
    if ((searchpath = nextpathname(pathname, searchpath)) == NULL) {
/*      foundfile = 0; */
      break;
    }
    s = pathname + strlen(pathname) - 1;
    if (*s != '\\' && *s != '/') strcat(pathname, "\\"); 
    strcat(pathname, mapname);
    if ((input = fopen(pathname, "r")) != NULL) {
      foundfile = 1;
      fclose (input);
      break;
    }
  }
#endif
}

/* Returns NULL if it failed for some reason */

/* map_type map_create (char *envvar) { */    /* 94/May/23 */
map_type map_create (char *texfonts)      /* 97/May/10 */
{
  char pathname[_MAX_PATH];
  map_type map;
      
#ifdef DEBUGALIAS
  if (traceflag) showline("Creating alias table\n", 0);
#endif
/*  oursearchenv ("texfonts.map", envvar, pathname); */
  oursearchenv ("texfonts.map", texfonts, pathname);
  if (*pathname == '\0') {
#ifdef DEBUGALIAS
    if (traceflag) {
      sprintf(logline, "Could not find %s in\n", "texfonts.map", texfonts);
      showline(logline, 0);
    }
#endif
    return NULL;
  }

  map = (map_type) xcalloc (MAP_SIZE, sizeof(map_element_type *));
  map_file_parse (map, pathname);
#ifdef DEBUGALIAS
  if (traceflag) map_show(map);
#endif
  return map;
}

/* ************************************************************************* */

/*  if we didn't find the font, maybe its alias to be found in texfonts.map */

map_type fontmap = NULL;      /* static - keep around once set */

/*  returns NULL if failed to find an alias */

char *alias (char *name)
{
/*  static map_type fontmap = NULL; */  /* static - keep around once set */
  char *mapped_name;
      
  if (usefontmap == 0) return NULL; /* failed to find it before */
/*  Now fault in the mapping if necessary.  */
  if (fontmap == NULL) {
/*    fontmap = map_create ("TEXFONTS"); */
    fontmap = map_create (texfonts);    /* 97/May/10 */
    if (fontmap == NULL) {    /* check if it worked */
      usefontmap = 0;     /* don't try this again */
      return NULL;
    }
  }
      
/*  Now look for our filename in the mapping.  */
  mapped_name = map_lookup(fontmap, name);
  return mapped_name;           /* possibly NULL */
}

/* ************************************************************** */

#endif

/***************************************************************************/

#ifndef SUBDIRSEARCH

/* Moved from DVIPSONE.C since DVIPSLOG.C module is much smaller */

/* Extract next pathname from a searchpath - and write into pathname */
/* return NULL if there are no more pathnames, */
/* otherwise returns pointer to NEXT pathname in searchpath */
/* searchpath = pathname1;pathname2; ... ;pathnamen */

/* used for pfb search path and eps search path */
/* this version also allows space as separator */

char *nextpathname(char *pathname, char *searchpath)
{
  int n;
  char *s;

  if (*searchpath == '\0') return NULL; /* nothing left */
  else if (((s = strchr(searchpath, ';')) != NULL) ||
         ((s = strchr(searchpath, ' ')) != NULL)) {
    n = (s - searchpath);
    if (n >= MAXPATHLEN) {
      sprintf(logline, " Path too long %s\n", searchpath);
      showline(logline, 1);
      return NULL;
    }
    strncpy(pathname, searchpath, (unsigned int) n);
    *(pathname + n) = '\0';       /* terminate it */
    return(s + 1);            /* next pathname or NULL */
  }
  else {
    n = (int) strlen(searchpath);
    if (n >= MAXPATHLEN) {
      sprintf(logline, " Path too long %s\n", searchpath);
      showline(logline, 1);
      return NULL;
    }
    strcpy(pathname, searchpath);
    return(searchpath + n);
  }
}

#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* struct _find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
    }; */ /* 16 bit world --- in dos.h */

/* struct _finddata_t {
    unsigned  attrib;
    time_t  time_create;  -1 for FAT file systems 
    time_t  time_access;  -1 for FAT file systems 
    time_t  time_write;
    _fsize_t  size;
    char  name[260];
}; */ /* 32 bit world --- in io.h */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef SUBDIRSEARCH

/* Code to implement (possibly recursive) sub-directory search 94/Aug/18 */

#define ACCESSCODE 4

#define SEPARATOR "\\"

/* This is the (possibly recursive) sub-directory searching code */

/* We come in here with the directory name in pathname */
/* Which is also where we plan to return the result when we are done */
/* We search in this directory first, */
/* and we search one level subdirectories (if it ended in \), */
/* or recursively if the flag is set (when it ends in \\). */
/* File name searched for is in filename */
/* Returns zero if successful --- return non-zero if not found */

/*  struct _finddata_t c_file; */
/*  _finddata_t structure *can* be reused, unlike _find_t 95/Jan/31 */
/*  so save on stack space, by having one copy, not one per expand_subdir */
/*  possibly allocate this in the caller of findsubpath ? not static ? */

int findsubpath (char *filename, char *pathname, int recurse)
{
  char *s;
  int code;
  int ret;
/*  struct _finddata_t c_file; */ 
  static struct _finddata_t c_file;   /* structure can be reused (?) */
  long hFind;
#endif

  s = pathname + strlen(pathname);  /* remember the end of the dirpath */
  if (traceflag) {
    sprintf(logline, " Entering findsubpath: %s %s %d\n",
        filename, pathname, recurse);
    showline(logline, 0);
  }

/*  First try directly in this directory (may want this conditional) */
  strcat (pathname, SEPARATOR);   /* \ or / */
  strcat (pathname, filename);

/*  Try for match in this directory first - precedence over files in subdirs */
/*  if (_access(pathname, ACCESSCODE) == 0) */    /* check for read access */
  code = _access(pathname, ACCESSCODE);
  if (code == 0) {
    if (traceflag) {
      sprintf(logline, " SUCCESS: %s\n", pathname);
      showline(logline, 0);
    }
    return 0;           /* success */
  }
  if (traceflag) {          /* debugging */
/*    code = _access(pathname, ACCESSCODE); */
    sprintf(logline, " File %s Access %d\n", pathname, code);
    showline(logline, 0);
  }

  *s = '\0';            /* strip off the filename again */
/*  no luck, so try and find subdirectories */
  strcat (pathname, SEPARATOR); /* \ or / */
  strcat (pathname, "*.*");
/*  if (_dos_findfirst (pathname, _A_NORMAL | _A_SUBDIR, &c_file) != 0) { */
/*  if (_dos_findfirst (pathname,
    _A_NORMAL | _A_SUBDIR | _A_RDONLY, &c_file) != 0)  */
  hFind = _findfirst (pathname, &c_file);
  if (hFind > 0)  ret = 0;    /* found something */
  else ret = -1;          /* did not find path ? */
/*  ret = _dos_findfirst (pathname, _A_NORMAL | _A_SUBDIR | _A_RDONLY, &c_file); */

  if (ret) {            /* nothing found ? */
    if (badpathwarn++ == 0) {
      sprintf(logline, "WARNING: bad path `%s' for `%s':\n", pathname, filename);
      showline(logline, 1);
      perrormod(filename);    /* debugging only ? bad path given */
    }
    return -1;          /* failure */
  }
  *s = '\0';            /* strip off the \*.* again */
/*  Step through sub-directories */
  for (;;) {
/*    Ignore all but sub-directories here - also ignore . and .. */
    if ((c_file.attrib & _A_SUBDIR) == 0 || *c_file.name == '.') {
/*      if (_dos_findnext (&c_file) != 0) break; */
      ret = _findnext (hFind, &c_file); /* success == TRUE ??? */
/*      need to flip polarity of ret ? apparently not ... */
/*      ret = _dos_findnext (&c_file);  */  /* success == 0 */

      if (ret != 0) break;  /* give up, found nothing more */
      continue;       /* did find something else, go on */
    }
/*    extend pathname with subdir name */
    strcat(pathname, SEPARATOR);
    strcat(pathname, c_file.name);
    if (traceflag) {
      sprintf(logline, " Checking subdir: %s\n", pathname);
      showline(logline, 0);
    }
/*    OK, now try for match in this directory */
    if (recurse) {              /* recursive version */
      if (findsubpath(filename, pathname, recurse) == 0) {
        _findclose(hFind);
        hFind = 0;
        return 0;           /* succeeded */
      }
    }
    else {                  /* not recursive */
      strcat (pathname, SEPARATOR);
      strcat (pathname, filename);
      if (traceflag) {
        sprintf(logline, " Checking file: %s\n", pathname);
        showline(logline, 0);
      }
/*      if (_access(pathname, ACCESSCODE) == 0)   */
      code = _access(pathname, ACCESSCODE); /* check read access */
      if (code == 0) { 
        if (traceflag) {
          sprintf(logline, " SUCCESS: %s\n", pathname);
          showline(logline, 0);
        }
        _findclose(hFind);
        hFind = 0;
        return 0;           /* success */
      }
      if (traceflag) {              /* debugging */
/*        code = _access(pathname, ACCESSCODE); */
        sprintf(logline, " File %s Access %d\n", pathname, code);
        showline(logline, 0);
      }
    }

/*    No match in this directory, so continue */
    *s = '\0';
    if (traceflag) {
      sprintf(logline, "Ready for dos_findnext: %s %s %d\n",
          filename, pathname, recurse);
      showline(logline, 0);
    }
/*    if (_dos_findnext (&c_file) != 0) break; */
    ret = _findnext (hFind, &c_file);   /* success == TRUE ??? */
/*    need to flip polarity of ret ? apparently not ... */
/*    ret = _dos_findnext (&c_file); */     /* success == 0 */

    if (ret != 0) break;          /* found no more */
  } /* end of for{;;} loop */
  if (hFind > 0) {
    _findclose (hFind);
    hFind = 0;
  }
  return -1;                  /* failed */
}

/* Our searchalongpath is (somewhat) analogous to DOS _searchenv */
/* The name of the desired file is given in `filename' */
/* The list of paths is given in `pathlist' */
/* searchalongpath returns the full pathname of first match in `pathname' */
/* (make sure there is enough space there!) */
/* If the file is not found, then pathname contains "" */
/* and it also returns non-zero if it fails. */
/* It first searches in the current directory if currentflag > 0 */
/* It also searches PFM subdirectories if currentflag < 0 97/June/1 */
/* If a path in `pathlist' ends in \, then its sub-directories are searched, */
/* (after the specified directory) */
/* If a path in `pathlist' ends in \\, then this works recursively */
/* (which may be slow and cause stack overflows ...) */

int searchalongpath (char *filename, char *pathlist, char *pathname, int current)
{
/*  struct _find_t c_file; */      /* need to preserve across calls to DOS */
  char *s, *t, *u, *send;
  int c, n;
  int recurse;
#ifdef DEBUGSEARCHPATH
  int code;
#endif
  
  if (current > 0) {  /*  Try for exact match in current directory first ? */
    strcpy(pathname, filename);
    if (_access(pathname, ACCESSCODE) == 0) { /* check for read access */
      if (traceflag) {
        sprintf(logline, " File %s SUCCESS\n", pathname);
        showline(logline, 0);
      }
      return 0;             /* success */
    }
#ifdef DEBUGSEARCHPATH
    if (traceflag) {              /* debugging */
      code = _access(pathname, ACCESSCODE);
      sprintf(logline, " File %s Access %d\n",
        pathname, _access(pathname, ACCESSCODE));
      showline(logline, 0);
    }
#endif
  }

/*  Now step through paths in pathlist */
  s = pathlist;
  for (;;) {
    if (*s == '\0') break;        /* sanity check */
    if ((t = strchr(s, ';')) == NULL)
      t = s + strlen(s);        /* if last path */
    n = t - s;
    strncpy(pathname, s, n);
    u = pathname + n;
    *u-- = '\0';            /* null terminate */
    c = *u;               /* check whether ends on \ */
    if (c == '\\' || c == '/') {    /* yes it does -> subdir search */
      *u-- = '\0';          /* remove it */
      c = *u;             /* check whether ends on \\ */
      if (c == '\\' || c == '/') {  /* yes it does */
        *u-- = '\0';        /* remove it */
        recurse = 1;        /* recursive subdir search */
      }
      else recurse = 0;
      if (traceflag) {
        sprintf(logline, " Trying subdir: %s\n", pathname);
        showline(logline, 0);
      }
      if (findsubpath (filename, pathname, recurse) == 0)
        return 0; /* success */
    }
    else {                  /* its just a directory */
      send = pathname + strlen(pathname); /* remember end for below */
      strcat (pathname, SEPARATOR);   /* \ or / */
      strcat (pathname, filename);
      if (_access (pathname, ACCESSCODE) == 0) {
        if (traceflag) {
          sprintf(logline, " File %s SUCCESS\n", pathname);
          showline(logline, 0);
        }
        return 0;           /* success */
      }
#ifdef DEBUGSEARCHPATH
      if (traceflag) {          /* debugging */
        code = _access(pathname, ACCESSCODE);
        sprintf(logline, " File %s Access %d\n",
          pathname, _access(pathname, ACCESSCODE));
        showline(logline, 0);
      }
#endif
      if (current < 0) {  /* try in PFM sub-directory also 97/June/1 */
        *send = '\0';         /* snip off file name again */
        strcat (pathname, SEPARATOR);   /* \ or / */
        strcat (pathname, "PFM");     /* splice in PFM */
        strcat (pathname, SEPARATOR);   /* \ or / */
        strcat (pathname, filename);
        if (_access (pathname, ACCESSCODE) == 0) {
          if (traceflag) {
            sprintf(logline, " File %s SUCCESS\n", pathname);
            showline(logline, 0);
          }
          return 0;           /* success */
        }
#ifdef DEBUGSEARCHPATH
        if (traceflag) {          /* debugging */
          code = _access(pathname, ACCESSCODE);
          sprintf(logline, " File %s Access %d\n",
               pathname, _access(pathname, ACCESSCODE));
          showline(logline, 0);
        }
#endif
      }
    }

    s = t;            /* move on to next item in list */
    if (*s == ';') s++;     /* step over separator */
    else break;         /* we ran off the end */
  }
  strcpy(pathname, "");     /* failed to find it */
  return -1;
}

/*  search for file in path list and open it if found */
/*  return full path name in third arg unless third arg is NULL */
/*  if third arg is NULL, a local temporary place is used for the name */
/*  if current > 0, look in current directory first */
/*  if current < 0, look in PFM sub directories also */
//  only place we use _alloca ...

FILE *findandopen (char *filename, char *pathlist, char *pathname, char *mode, int current)
{
  FILE *file;

  if (pathname == NULL) {
    pathname = (char *) _alloca (FNAMELEN);
    if (pathname == NULL) checkexit(1);
  }
  if (searchalongpath(filename, pathlist, pathname, current) == 0) {
    file = fopen(pathname, mode);
    return file;
  }
  else return NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* dviextra.c(5813) : fatal error C1001: internal compiler error */
/*    (compiler file 'msc2.cpp', line 1011) */

/* dvipslog.c(2008) : fatal error C1001: internal compiler error
    (compiler file 'msc2.cpp', line 1011) */

/* stuff for reading .afm files */

int readafm(char *font, FILE *fp_afm, long widths[])
{
  double fwidth;
  int chr, k=0;

/*  if (fp_afm == NULL) checkexit(5);  */
  (void) getrealline(fp_afm, line);

  while (strstr(line, "StartCharMetrics") == NULL) {
    if(getrealline(fp_afm, line) == 0) {
      sprintf(logline, 
        " Can't find CharMetrics in AFM file for %s\n", font);
      showline(logline, 1);
      errcount(0);
      return 0;
    }

/* could extract UniqueID, BBox and FontInfo stuff at this point */
  }
  (void) getrealline(fp_afm, line);
  while(strstr(line, "EndCharMetrics") == NULL) {
    if(strstr(line, "EndCharMetrics") != NULL) break;
    if (sscanf(line," C %d ; WX %lg", &chr, &fwidth) < 2) {
      sprintf(logline, 
        " Parse error in line from AFM file for %s: %s", 
          font, line);
      showline(logline, 1);
      errcount(0);
      return 0;
    }
    if (chr >= 0 && chr < MAXCHRS) {
      if (chr > k) k = chr;
      widths[chr] = (long) ((fwidth/1000.0) * 1048576.0 + 0.5);
    }
    (void) getrealline(fp_afm, line);
  }
  return k;
}

/* moved here from dviextra.c */

/* stuff for reading .tfm files */ /* OK for new form TFM files ? OK */

/* lf, lh, nw, nh, nd, ni, nl, nk, ne are numbers of words */

int readtfm(char *font, FILE *fp_tfm, long widths[])
{
  static long qwidths[MAXCHRS];  /* 256 */
  int lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np;
  int k;
  int wdinx;
/*  int chksum, dsgnsize;   */
/*  int hdinx, itinx, rdinx;  */

/*  if (fp_tfm == NULL) checkexit(5);  */
    lf = sreadtwo(fp_tfm);  lh = sreadtwo(fp_tfm);
  bc = sreadtwo(fp_tfm);  ec = sreadtwo(fp_tfm);
  nw = sreadtwo(fp_tfm);  nh = sreadtwo(fp_tfm);
  nd = sreadtwo(fp_tfm);  ni = sreadtwo(fp_tfm);
  nl = sreadtwo(fp_tfm);  nk = sreadtwo(fp_tfm);
  ne = sreadtwo(fp_tfm);  np = sreadtwo(fp_tfm);
/* first try and make sure this is a TFM file ! */
  if (lf < 0 || lh < 0 || nw < 0 || nw > 255 || 
    bc < 0 || ec < 0 || ec > 255 || bc > ec + 1 || 
    lf != 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np)
    {
    sprintf(logline, " BAD TFM file for %s", font);
    showline(logline, 1);
    showline("\n", 0);
/*    sprintf(logline, "Header: %d %d %d %d %d %d %d %d %d %d %d %d\n",
      lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np);
    showline(logline, 1); */
    errcount(0);
    return 0;
  }
/* now for the header */
/*  chksum = sreadtwo(fp_tfm); */   /* should we bother to verify ? */
  (void) sreadtwo(fp_tfm);    /* check sum */
/*  dsgnsize = sreadtwo(fp_tfm);  */
  (void) sreadtwo(fp_tfm);  /* design size */
/* discard rest of header */
  for (k = 2; k < lh; k++) {  
    (void) getc(fp_tfm); (void) getc(fp_tfm);
  }
/* now read the actual widths */
  fseek(fp_tfm, (long) ((6 + lh + (ec - bc + 1)) << 2), SEEK_SET);
  for (k = 0; k < nw; k++) {
    qwidths[k] = sreadfour(fp_tfm);
/* character width in design size units * 2^20 */
  }
  if (qwidths[0] != 0) {  /* qwidths[0] is supposed to be zero */
    sprintf(logline, " BAD TFM file for %s", font);
    showline(logline, 1);
    showline("- ", 0);
    showline("width[0] not zero", 0); /* qwidths[0] */
    errcount(0);
    return 0;
  }
/* now go back and read character information */
  fseek(fp_tfm, (long) ((6 + lh) << 2), SEEK_SET);
  for (k = bc; k <= ec; k++) {
    wdinx = getc(fp_tfm); 
    (void) getc(fp_tfm);  /*    hdinx = getc(fp_tfm);  */
    (void) getc(fp_tfm);  /*    itinx = getc(fp_tfm);  */
    (void) getc(fp_tfm);  /*    rdinx = getc(fp_tfm);  */
    if (wdinx >= nw) {
      sprintf(logline, " BAD TFM file for %s", font);
      showline(logline, 1);
      showline(" - ", 0);
      sprintf(logline, "width index %d (char %d) > width table %d",
        wdinx, k, nw);
      showline(logline, 0);
      errcount(0);
      return 0;
    }
    widths[k] = qwidths[wdinx];
  }
  return (int) (ec + 1);
}

/* stuff for reading widths from .pfm files */

int readpfm(char *font, FILE *fp_pfm, long widths[])
{
  unsigned long length, offset;
/*  double fwidth; */
  long lwidth;
  int bc, ec, c, k, n;

/*  if (fp_pfm == NULL) checkexit(5);   */
/* first check that this is a PFM file - start with version number */
  if ((c = getc(fp_pfm)) != 0 || (c = getc(fp_pfm)) != 1)
  {
    sprintf(logline, " Not a proper PFM file %s\n", font);
    showline(logline, 1);
    errcount(0);
    return 0;
  }
  length = 0L;    /* read length of PFM file  */
  for (k = 0; k < 4; k++)         /* from byte 2 to byte 6 */
    length = length | (getc(fp_pfm) << (k * 8));
  for (k = 6; k < 66; k++) (void) getc(fp_pfm); /* ignore copyright */  
  for (k = 66; k < 95; k++) (void) getc(fp_pfm); /* ignore assorted */  
  bc = getc(fp_pfm); ec = getc(fp_pfm);  /* first and last character */
  for (k = 97; k < 117; k++) (void) getc(fp_pfm); /* skip to end header */
  for (k = 117; k < 119; k++) (void) getc(fp_pfm);  /* size PFMEXTENSION */
  for (k = 119; k < 123; k++) (void) getc(fp_pfm);  /* ptr EXTEXTMETRICS */
  offset = 0L;               /* offset of charwidth table */
  for (k = 0; k < 4; k++) offset = offset | (getc(fp_pfm) << (k * 8));
  if (offset > 8192) {
    sprintf(logline, " Offset too long in %s\n", font);
    showline(logline, 1);
    errcount(0);
    return 0;
  }
  n = (int) offset;
  for (k = 127; k < n; k++) c = getc(fp_pfm);
  if (c == EOF) {
    showline(" Premature EOF", 1);
    sprintf(logline, " in PFM file %s\n", font);
    showline(logline, 0);
    errcount(0);
    return 0;
  }
  for (k = bc; k <= ec; k++) {
/*    fwidths = (double) (getc(fp_pfm) | (getc(fp_pfm) << 8));
    widths[k] = (long) ((fwidth/1000.0) * 1048576.0 + 0.5); */
    lwidth = (long) (getc(fp_pfm) | (getc(fp_pfm) << 8));
/*    widths[k] = (long) (((lwidth << 20) + 500) / 1000);  */
    widths[k] = (long) (((lwidth << 17) + 62) / 125);  
/*    if (lwidth != 0) showline(logline, "w[%d] %ld ", k, lwidth); */
/*    if (widths[k] != 0) showline(logline, " %ld ", k, widths[k]); */
  }
  if (traceflag) {
    sprintf(logline, "bc = %d ec = %d ", bc, ec);
    showline(logline, 0);
  }
  return ec;
}

#define DEBUGMMPFM

/* check PFM file for MM instance and extract PostScript FontName */
/* WARNING: this writes back into second argument ! */
/* make sure FontName has enough space for FontName ( > 32 ) 97/June/1 */
/* In typical use, FaceName == NULL and nface == 0 */
/* return 0 if fails in some way */

/* int pfminstance (FILE *input, char *FontName, int nlen) { */
int NamesFromPFM (FILE *input, char *FaceName, int nface,
          char *FontName, int nfont, char *FileName)
{
  short version;
  long length, offset;
  int n, ndrive;
  char DriverType[16];  /* space for "PostScript" */
  char *s;
  
//  if (traceflag) {
//    sprintf(logline, " NamesFromPFM nface %d nfont %d\n", nface, nfont);
//    showline(logline, 0);     // debugging only
//  }

#ifdef DEBUGMMPFM
  if (traceflag)  {
    sprintf(logline, " Read `%s' for MM instance info\n", FileName); /* debugging */
    showline(logline, 0);
  }
#endif
  fread(&version, sizeof(version), 1, input);
  if (version != 256) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Bad version code (%d) in PFM %s ", version, FileName);
    showline(logline, 1);
#endif
    return 0;       /* not PFM file */
  }
  fread(&length, sizeof(length), 1, input);
  if (fseek(input, 101, SEEK_SET) != 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Seek to %ld failed in PFM %s ", 101L, FileName);
    showline(logline, 1);
#endif
    return 0;
  }
  fread(&offset, sizeof(offset), 1, input); /* offset to Driver Type */
  if (offset >= length || offset == 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Bad offset %ld (%d) in PFM %s ", offset, length, FileName);
    showline(logline, 1);
#endif
    return 0; /* not PFM file */
  }
  if (fseek(input, offset, SEEK_SET) != 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Seek to %ld failed in PFM %s ", offset, FileName);
    showline(logline, 1);
#endif
    return 0;
  }
  ndrive = sizeof(DriverType);
  s = DriverType;               /* temporary space */
  n = 0;
  while ((*s++ = (char) getc(input)) != '\0') { 
    if (n++ >= ndrive) {
#ifdef DEBUGMMPFM
      sprintf(logline, " %s too long >= %ld in %s ", "DriverType", ndrive, FileName);
      showline(logline, 1);
#endif
      return 0;
    }
  }
  *s = '\0';                  /* terminate */
  if (strcmp(DriverType, "PostScript") != 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Driver not %s in %s ", "PostScript", FileName);
    showline(logline, 1);

#endif
    return 0; /* Not PS font */
  }
  if (fseek(input, 105, SEEK_SET) != 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Seek to %ld failed in PFM %s ", 105L, FileName);
    showline(logline, 1);
#endif
    return 0;
  }
  fread(&offset, sizeof(offset), 1, input); /* offset Windows Face Name */
  if (offset >= length || offset == 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Bad offset %ld (%ld) in PFM %s ", offset, length, FileName);
    showline(logline, 1);
#endif
    return 0; /* not PFM file */
  }
  if (fseek(input, offset, SEEK_SET) != 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Seek to %ld failed in PFM %s ", offset, FileName);   
    showline(logline, 1);
#endif
    return 0;
  }
  if (FaceName != NULL) {
    s = FaceName;
    n = 0;
    while ((*s++ = (char) getc(input)) != '\0')
      if (n++ >= nface) {
#ifdef DEBUGMMPFM
      sprintf(logline, " %s too long >= %ld in %s ", "FaceName", nface, FileName);
      showline(logline, 1);
#endif
      return 0;
    }
    *s = '\0';    /* terminate */
    if (strchr(FaceName, ' ') == NULL) {
#ifdef DEBUGMMPFM
      sprintf(logline, " Not MM %s %s? ", "FaceName", FaceName);
      showline(logline, 1);
#endif
/*      return 0; */  /* not MM instance */
    }
  }
  if (fseek(input, 139, SEEK_SET) != 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Seek to %ld failed in PFM ", 139L);
    showline(logline, 1);
#endif
    return 0;
  }
  fread(&offset, sizeof(offset), 1, input); /* offset to PS FontName */
  if (offset >= length || offset == 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Bad offset %ld (%ld) in PFM ", offset, length);
    showline(logline, 1);
#endif
    return 0; /* not PFM file */
  }
  if (fseek(input, offset, SEEK_SET) != 0) {
#ifdef DEBUGMMPFM
    sprintf(logline, " Seek to %ld failed in PFM ", offset);
    showline(logline, 1);
#endif
    return 0;
  }

/*  write name back into second argument */
  if (FontName != NULL) {
    s = FontName;
    n = 0;
    while ((*s++ = (char) getc(input)) != '\0')
      if (n++ >= nfont) {
#ifdef DEBUGMMPFM
      sprintf(logline, " %s too long >= %ld in %s ", "FontName", nfont, FileName);
      showline(logline, 1);
#endif
      return 0;
    }
    *s = '\0';    /* terminate */
/*    We assume FontName for MM instance must have underscores */
    if (strchr(FontName, '_') == NULL) {
#ifdef DEBUGMMPFM
      sprintf(logline, " Not MM %s %s? ", "FontName", FontName);
      showline(logline, 1);
#endif
      /* return 0; */ /* not MM instance */
    }
#ifdef DEBUGMMPFM
    if (traceflag) {
      sprintf(logline, " FontName from PFM: %s\n", FontName);/* debugging output */
      showline(logline, 0);
    }
#endif
  }
  return 1; /* OK, FaceName and FontName returned */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for reading ATMREG.ATM imported from winpslog.c 98/Jan/9 */

unsigned int xreadtwo (FILE *input)
{
  unsigned int c, d, n;
  c = getc(input);
  d = getc(input);
  n = (d << 8) | c; 
  return n;
}

unsigned long xreadfour (FILE *input)
{
  unsigned int a, b, c, d;
  unsigned long n;
  a = getc(input);
  b = getc(input);
  c = getc(input);
  d = getc(input);
  n = (d << 8) | c;
  n = (n << 16) | (b << 8) | a;
  return n;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int bATM41=0;     // needs to be set based on ATMREG.ATM header

/* read string from ATMREG.ATM up to null, string may be empty */
/* returns -1 if hit EOF or name too long */

int ReadString (FILE *input, char *name, int nlen)
{
  int c;
  int n=0;
  char *s=name;

  *s = '\0';        /* in case we pop out early */
  for (;;) {        /* read string up to null */
    c = getc(input);
    if (bATM41)       // 2000 July 3 ATM 4.1 ???
      (void) getc(input); // discard second byte of UNICODE
    if (c == EOF) {   /* EOF */
      *s++ = '\0';
      return -1;
    }
    *s++ = (char) c;
    if (c == 0) break;
    n++;
    if (n >= nlen) {  /* too long */
      sprintf(logline, " String in ATMREG.ATM too long %d (> %d)\n", n, nlen);
      showline(logline, 1);
      *name = '\0'; /* flush the name */
        return -1;
    }
  }
  return 0;
}

#define LF_FACENAME 32
#define LF_FULLFACENAME 64

/* #define PFMName 16 */    /* >= 8+1+3+1 */
/* #define PFBName 16 */    /* >= 8+1+3+1 */

/**************************************************************************/

/* New code for using ATMREG.ATM */

unsigned long startfontlist, endfontlist;
unsigned long startdirlist, enddirlist;
unsigned long startsetlist, endsetlist;
int nDirs, nFonts, nSets;

typedef struct ATMRegRec {
  unsigned char nMMM;   // directory path index for MMMName
  unsigned char nPFB;   // directory path index for PFBName
  unsigned char nPFM;   // directory path index for PFMName
  unsigned char MMMflag;  // 0 => TTF, 1 => T1, 2 => MM Master, 4 => MM Instance
  char *FontName;     // PostScript FontName
  char *MMMName;      // File Name of PFM (T1 or MMM instance), MMM (for MM master)
  char *PFBName;      // File Name of PFB (blank for MM instance)
  char *PFMName;      // File Name of PFM (for MM master), MMM (for MM instance)
} ATMRegRec;

struct ATMRegRec *ATMFonts=NULL;

int ATMfontindex=0;   // number of font entries

// char *DirPaths[MAXDIRS];

char **DirPaths=NULL;

int dirindex=0;

void FreeDirs (void)
{
  int k;
  if (DirPaths == NULL) return;
  for (k = 0; k <= nDirs; k++) {
    if (DirPaths[k] != NULL) free(DirPaths[k]);
    DirPaths[k] = NULL;
  }
  dirindex=1;
  free(DirPaths);
  DirPaths = NULL;
}

int AllocDirs (int nDirs)
{
  int k, nlen;
  if (DirPaths != NULL) FreeDirs();
  nlen = (nDirs + 1) * sizeof(char *);
  DirPaths = (char **) malloc (nlen);
  if (DirPaths == NULL) {
    sprintf(logline,"ERROR: unable to allocate %d bytes for %d dir entries\n",
         nlen, nDirs);
    showline(logline, 1);
    return -1;
  }
  DirPaths[0] = _strdup("");
  for (k = 1; k <= nDirs; k++) DirPaths[k] = NULL;
  dirindex=1;
  return 0;
}

void FreeFonts (void)
{
  int k;
  if (ATMFonts == NULL) return;
  for (k = 0; k < ATMfontindex; k++) {
    if (ATMFonts[k].FontName != NULL) free(ATMFonts[k].FontName);
    if (ATMFonts[k].MMMName != NULL) free(ATMFonts[k].MMMName);
    if (ATMFonts[k].PFBName != NULL) free(ATMFonts[k].PFBName);
    if (ATMFonts[k].PFBName != NULL) free(ATMFonts[k].PFMName);
  }
  free(ATMFonts);
  ATMFonts = NULL;
}

int AllocFonts (int nFonts)
{
  int nlen;
  if (ATMFonts != NULL) FreeFonts();
  nlen = nFonts * sizeof(struct ATMRegRec);
  ATMFonts = (struct ATMRegRec *) malloc (nlen);
  if (ATMFonts == NULL) {
    sprintf(logline, " Unable to allocate %d bytes for %d fonts\n",
        nlen, nFonts);
    showline(logline, 1);
//    checkexit(1);
    return -1;
  }
//  sprintf(logline, "Allocated %d bytes for %d fonts\n", nlen, nFonts);  // debugging only
//  showline(logline, 0);
  ATMfontindex=0;
  return 0;
}

void ShowATMREG (void)
{
  int k;
  char *szType;
  sprintf(logline, "ATMREG has %d T1 font entries (out of %d total):\n", ATMfontindex, nFonts);
  showline(logline, 0);
  for (k = 0; k < ATMfontindex; k++) {
    switch(ATMFonts[k].MMMflag) {
      case 0: szType = "TTF "; break;
      case 1: szType = "T1  "; break;
      case 2: szType = "MMM "; break;
      case 4: szType = "MMI "; break;
      default: szType = "ERR "; break;
    }
    sprintf(logline, "%s Fontname: `%s' MMMName: `%s%s' PFBName: `%s%s' PFMName: `%s%s'\n",
        szType, ATMFonts[k].FontName,
        DirPaths[ATMFonts[k].nMMM], ATMFonts[k].MMMName,
        DirPaths[ATMFonts[k].nPFB], ATMFonts[k].PFBName,
        DirPaths[ATMFonts[k].nPFM], ATMFonts[k].PFMName);
    showline(logline, 0);
  }
}

/**********************************************************************************/

int SetupDirs (FILE *input, unsigned long startdirlist, unsigned long enddirlist)
{
  int c, k, noff, nlen;
  int npath=0;
  unsigned long noffset;
  char pathname[ _MAX_PATH];
  char *s;

/*  if (fseek(input, 24, SEEK_SET) >= 0) noffset = xreadfour(input); */

  noffset = startdirlist;     // 36

  for (;;) {
    if (noffset >= enddirlist) break; /* normal exit from this */
    if (fseek(input, noffset, SEEK_SET) < 0) {
      if (traceflag) {
        sprintf(logline, " Seek to %ld failed\n", noffset);
        showline(logline, 0);
      }
      break;  /*        return -1; */
    }
    noff = xreadtwo(input);
    if (noff != 8) {
      if (traceflag) {
        sprintf(logline, " noff %lu != 8\n", noff);
        showline(logline, 0);
      }
/*      break; */ /* new sanity check */
    }
    nlen = xreadtwo(input);
    if (nlen == 0) {
      if (traceflag) {
        sprintf(logline, " nlen == 0\n");
        showline(logline, 0);
      }
      break;    /* sanity check */
    }
    if (nlen > _MAX_PATH) {
      if (traceflag) {
        sprintf(logline, " nlen > %d\n", _MAX_PATH);
        showline(logline, 0);
      }
      break;    /* new sanity check */
    }
    noffset = xreadfour(input);
    if (noffset == 0) {
      if (traceflag) {
        sprintf(logline, " noffset == 0\n");
        showline(logline, 0);
      }
      break;  /* sanity check */
    }
    s = pathname;
    for (k = 0; k < nlen; k++) {
      c = getc(input);
      if (bATM41) (void) getc(input);
      if (c == EOF) {
        if (traceflag) {
          sprintf(logline, " Unexpected EOF (%s)\n", "setupdirs");
          showline(logline, 0);
        }
        return -1;
      }
      *s++ = (char) c;
      if (c == 0) break;
    }
    npath++;
/*    if (noiseflag) printf("%d\t%s\n", npath, pathname); */
//    if (dirindex >= MAXDIRS) {
    if (dirindex > nDirs) {
      if (traceflag) {
        sprintf(logline, " Too many paths (> %d)\n", nDirs);
        showline(logline, 0);
      }
      return -1;
    }
    DirPaths[dirindex] = xstrdup(pathname);
    dirindex++;
  }
  return 0;
}

/* New version uses ATMFonts structure */ /* First argument is PS FontName */
/* WRITES BACK INTO SECOND ARGUMENT */
/* returns 0 if found, -1 if not found */

int SearchATMReg (char *szPSFontName, char *szPFBFileName)
{
  int k;
  for (k = 0; k < ATMfontindex; k++) {
    if (strcmp(szPSFontName, ATMFonts[k].FontName) == 0) {
      strcpy(szPFBFileName, DirPaths[ATMFonts[k].nPFB]);
      strcat(szPFBFileName, ATMFonts[k].PFBName);   // PFB file name
      return 0;
    }
  }
  *szPFBFileName = '\0';    /* wipe clean */
  return -1;          /* failed to find */
}

/* Create new ATMFonts data structure 2000 July */

int ScanATMReg (FILE *input, unsigned long endfontlist)
{
  int c, k;
  unsigned int stroffset, nlen;
  unsigned long next;
  int boldflag, italicflag;   /* style bits */
  int ttfflag;
/*  following just used for statistics - could remove to save time */
  int psflag, mmmflag, mmiflag, genflag;    /* font type bits */
  int nMMM, nPFB, nPFM;       /* index into dir path table */
  unsigned int flag[16];        /* 16 bytes of flags */
  char FaceName[LF_FACENAME+1];   /* Windows Face Name - not used */
  char StyleName[LF_FACENAME+1];    /* Style Name for TT font - not used */
  char FullName[LF_FULLFACENAME+1]; /* Full Name - not used */
  char FontName[LF_FULLFACENAME+1]; /* Font Name - used in comparison (T1 only) */
  char MMMName[LF_FACENAME+1];    /* PFM file or TTF file or MMM file */
  char PFBName[LF_FACENAME+1];    /* PFB file or PSS file - not used */
  char PFMName[LF_FACENAME+1];    /* PFM file of MMM font - not used */
//  char *s;

  if (ATMFonts == NULL) {
    if (AllocFonts(nFonts)) return -1;
  }
  
  ATMfontindex = 0;
  
//  sprintf(logline"SEEK TO %d\n", startfontlist);  // debugging only
//  showline(logline, 0);
  fseek(input, startfontlist, SEEK_SET);

/*  positioned at start of font list at this point */

  for (;;) {
    c = getc(input);        /* check for end of file 99/Mar/1 */
    if (c == EOF) {
      break;
    }
    ungetc(c, input);
    stroffset = xreadtwo(input);  /* offset to first string == 44 */
    nlen = xreadtwo(input);     /* length of this record in bytes */
    next = xreadfour(input);    /* pointer to next record */
    for (k = 0; k < (28 - 8); k++) (void) getc(input);
    for (k = 0; k < 16; k++) flag[k] = getc(input);
    boldflag = flag[1];
    if (boldflag == 0 || boldflag > 2) {
      if (boldflag > 2) boldflag = 1; /* pretend it is OK */
/*      break; */  /* impossible */ /* `fixed' 97/Sep/14 */
    }
    else boldflag = boldflag - 1;
    italicflag = flag[2];
    if (italicflag > 1) {
/*      break; */ /* impossible */  /* `fixed' 97/Sep/14 */
    }
    ttfflag = psflag = mmmflag = mmiflag = genflag = 0;
/*    ttfflag = flag[5]; */
    if (flag[4] == 0) ttfflag = 1;
    else if (flag[4] == 1) psflag = 1;
    else if (flag[4] == 2) mmmflag = 1;
    else if (flag[4] == 4) mmiflag = 1;
    if (flag[6] == 10) {
      genflag = 1;
      mmmflag = 0;
    }
    nMMM = flag[8] | (flag[9] << 8);  /* index into path name table */
    nPFB = flag[10] | (flag[11] << 8);  /* index into path name table */
    nPFM = flag[12] | (flag[13] << 8);  /* index into path name table */
/*    mmflag = flag[12]; */

//    if (ttfflag) ttfcount++;
//    else if (genflag) gencount++;
//    else if (mmiflag) mmicount++;
//    else if (mmmflag) mmmcount++;
//    else pscount++;

/*    These used to all continue when they hit trouble */
/*    Windows Face Name */
    if (ReadString(input, FaceName, sizeof(FaceName)) < 0) goto donext;
/*    Style Name (will be empty string for PS SM or MM font) */
    if (ReadString(input, StyleName, sizeof(StyleName)) < 0) goto donext;
/*    Full Name  (will be empty string for PS SM or MM font) */
    if (ReadString(input, FullName, sizeof(FullName)) < 0) goto donext;
/*    Font Name  (may be empty if font file not yet read by ATM) */
    if (ReadString(input, FontName, sizeof(FontName)) < 0) goto donext;
/*    Name of MMM file or PFM file or TTF file */ 
    if (ReadString(input, MMMName, sizeof(MMMName)) < 0) goto donext;
/*    Name of PFB file or PSS file */ 
    if (ReadString(input, PFBName, sizeof(PFBName)) < 0) goto donext;
/*    Name of PFM file in case of MMM font */ 
    if (ReadString(input, PFMName, sizeof(PFMName)) < 0) goto donext;
/*    Flush extension from file name --- MMMName is file name */
//    if ((s = strchr(MMMName, '.')) != NULL) *s = '\0';
/*    Remove underscores from file name */
/*    removeunderscores(MMMName); */
/*    if (testflag == 0) removeunderscores (MMMName); */  /* ??? */
/*    Make all uppercase ? It's a file name so its safe at least */
/*    makeuppercase (MMMName); */ /* ??? */
    if (ttfflag) goto donext;
    ATMFonts[ATMfontindex].nMMM = (unsigned char) nMMM;
    ATMFonts[ATMfontindex].nPFB = (unsigned char) nPFB;
    ATMFonts[ATMfontindex].nPFM = (unsigned char) nPFM;
    ATMFonts[ATMfontindex].MMMflag = (unsigned char) flag[4];
    ATMFonts[ATMfontindex].FontName = xstrdup(FontName);  // PS FontName
    ATMFonts[ATMfontindex].MMMName = xstrdup(MMMName);    // PFM/MMM File Name
    ATMFonts[ATMfontindex].PFBName = xstrdup(PFBName);    // PFB File Name
    ATMFonts[ATMfontindex].PFMName = xstrdup(PFMName);
    ATMfontindex++;
donext:     /* 1999/Mar/1 */
    if (next >= endfontlist) break;
    if (fseek(input, next, SEEK_SET) < 0) break;
  }
  return ATMfontindex;
}

/* sets up pointers to sections of ATMREG.ATM */
/* also determines whether wide strings are used (ATM 4.1) */
/* also reads in directory path table */

unsigned long ReadPointers (FILE *input)
{
  (void) fseek(input, 6, SEEK_SET);
  nDirs = xreadtwo(input);      /* 6 number of directory paths */
  nFonts = xreadtwo(input);     /* 8 number of font entries */
  nSets = xreadtwo(input);      /* 10 number of font sets (?) */
//  sprintf(logline, "%d Dir Paths %d Font Entries %d Font Sets\n", nDirs, nFonts, nSets);  // debugging only
//  showline(logline, 0);
//  (void) fseek(input, 12, SEEK_SET);  /* start of pointers into file */
  enddirlist = xreadfour(input);    /* 12 enddirlist */
  (void) xreadfour(input);      /* 16 mystery ??? */
  startfontlist = xreadfour(input); /* 20 startfontlist */
  startdirlist = xreadfour(input);  /* 24 startdirlist */
  startsetlist = xreadfour(input);  /* 28 endfontlist */
  endfontlist = startsetlist;
  endsetlist = xreadfour(input);    /* 32 endsetlist */

//  See whether strings in ATMREG.ATM are in UNICODE format
  (void) fseek(input, endsetlist, SEEK_SET);
  (void) getc(input);
  if (getc(input) == 0) bATM41 = 1;
  else bATM41 = 0;
  if (traceflag) {
    sprintf(logline, " bATM41 %d\n", bATM41);
    showline(logline, 0);
  }
  return endfontlist;
}

int SetupATMReg (void)
{
  char szFullFileName[FNAMELEN]="";

  if (useatmreg == 0) return -1;    /* tried already and failed */
  if (szATMRegAtm == NULL) {
    setupinifilesub("atmreg.atm", szFullFileName);
    if (traceflag) {
      sprintf(logline, " atmreg.atm: %s ", szFullFileName);
      showline(logline, 0);
    }
    if (*szFullFileName == '\0') {
      useatmreg = 0;        /* don't try again */
      return -1;
    }
    szATMRegAtm = xstrdup(szFullFileName);
  }
  return 0;
}

// LOAD information from ATMREG.ATM in convenient form 2000 July 6 

int LoadATMREG (void)
{
  FILE *input;
  int count;

  if (! useatmreg) return -1;     // tried before and failed
  if (szATMRegAtm == NULL) {
    if (SetupATMReg()) return -1; // failed to setup now
  }
  if (szATMRegAtm == NULL) return -1; // sanity check
  input = fopen(szATMRegAtm, "rb"); // open in binary mode for reading 
  if (input == NULL) {
    useatmreg = 0;
    return -1;    // probably because not found
  }
  if (traceflag) {
    sprintf(logline, "Scanning %s ", szATMRegAtm);
    showline(logline, 0);
  }
  (void) ReadPointers(input);
  if (AllocDirs(nDirs)) {
    useatmreg = 0;
    return -1;
  }
  SetupDirs(input, startdirlist, enddirlist);
  if (AllocFonts(nFonts)) {
    useatmreg = 0;
    return -1;
  }
  count = ScanATMReg(input, endfontlist);
  fclose(input);
  if (traceflag) ShowATMREG();    // debugging output
  return count;
}

/* First arg is PS FontName */ /* WRITES BACK INTO SECOND ARG */

int LookupATMReg (char *szPSFontName, char *szPSFileName)
{
  int n;
  if (! useatmreg) return -1;   // tried before and failed
  if (szATMRegAtm == NULL) {    // create ATMFonts structure
    if (LoadATMREG() < 0) return -1;  // failed
  }
  n = SearchATMReg(szPSFontName, szPSFileName);
  if (traceflag) {
    sprintf(logline, " LookupATMReg %s %s %d\n",
        szPSFontName, szPSFileName, n);
    showline(logline, 0);   // debugging only
  }
  return n;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* could perhaps be speeded up slightly - using table of chars to skip */

/* check that fonts that have no characters used */
/* (because of page limits) are NOT loaded in dviextra.c */

/* check that nothing but nop and font defs happen between eop and bop ? */

/* doesn't complain if pre not encountered ? IT DOES */

/* search for start of DVI in Textures file a bit crude ? */

/* try and avoid actually giving up if at all possible */

/* catch DVI commands before PRE - OK */

/* catch DVI commands after POST */

/* is TeX comment ever used later ? */  /* yes in PS file */

/* may also want to look 100 bytes into the file for start */
/* some Mac files come that way... */

/* bRemapSpace remaps 32 => 195, 13 to 176, 10 => 173, 9 => 170, 0 => 161 */
/* Rational is that text fonts reencoded to TeX 'n ANSI do not use 0 */
/* or 9 or 10, and from now on text fonts will not use 13 for fl, */
/* and TeX does not use 32 in text fonts */
/* But math fonts do use 0, 9, 10, 13 and 32 */
/* but math fonts always have the repetition of 0 - 32 higher up */
/* And for some versions of Acrobat it may be best not to do this */
/* for example transfer material to clipboard is null terminated */
/* 9 is treated as tab, 10 as newline, 13 ignored and 32 ignored */
