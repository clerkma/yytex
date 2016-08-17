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
* DVI to PS convertor for Adobe Type 1 (ATM compatible) fonts
* This is the part that actually converts DVI commands to PS
*
**********************************************************************/

#include "dvipsone.h"

/* NOTE: S = s w, T = s x, W = w<n>, X = x<n>, Y = y<n>, Z = z<n> */
/* bp = bop, ep = eop, pr = put_rule, sr = set_rule */

static int firstpage = 0; /* non-zero when nothing has been output yet */
static int skiptoend = 0; /* non-zero => still need to skip to last page */
int finish    = 0; /* non-zero => have hit end of DVI file */
int showcount = 0; /* on when last sent out "set" or "put" */
int freshflag = 0; /* on after fresh line is started (\n) */

int stinx;        /* stack index - to avoid overflow */ 
int maxstinx;     /* max stack index seen  - not used here */

static long currentpagestart;    /* 95/Aug/27 */

/* int escapecode[14] = {'b', 't', 'n', 'v', 'f', 'r'}; */

static char *escapecode = "btnvfr";  /* special codes for 8, 9, 10, 12, and 13 */

/* we don't have to worry about sign extension here - no need for short int */

static unsigned int ureadtwo (FILE *input)
{
  return (getc(input) << 8) | getc(input);
}

static unsigned long ureadthree (FILE *input)
{
  int c, d, e;
  c = getc(input);
  d = getc(input);
  e = getc(input);
  return ((((unsigned long) c << 8) | d) << 8) | e;
}

static unsigned long ureadfour (FILE *input)
{
  int c, d, e, f;
  c = getc(input);
  d = getc(input);
  e = getc(input);
  f = getc(input);
  return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
}

/* we do have to worry about sign extension here - use short int if needed */

static int sreadone (FILE *input)
{
  int c;

  c = getc(input);
  if (c > 127)
    return (c - 256);
  else
    return c;
}

/* possible compiler optimization bug worked around 98/Feb/8 */

static int sreadtwo (FILE *input)
{
  int c, d;
  c = getc(input);
  d = getc(input);
  if (c > 127) c = c - 256;
  return c << 8 | d;
}

static long sreadthree (FILE *input)
{
  int c, d, e;
  c = getc(input);
  d = getc(input);
  e = getc(input);
  if (c > 127) c = c - 256;
  return ((((long) c << 8) | d) << 8) | e;
}

static long sreadfour (FILE *input)
{
  int c, d, e, f;
  c = getc(input);
  d = getc(input);
  e = getc(input);
  f = getc(input);
  return ((((((long) c << 8) | (long) d) << 8) | e) << 8) | f;
}

/* don't need to optimize `push pop' since doesn't happen ever? */

static void do_push (FILE *output, FILE *input)
{
  int c;

  if (skipflag == 0)
  {
    stinx++;

    if (stinx % 64 == 0)    /* see if stack is getting full */
    {
      PSputs(" pushstack\n", output);
      showcount = 0;
      return;
    }

    c = getc(input);

    if (c == (int) push && ((stinx + 1) % 64 != 0))
    {
      stinx++;
      PSputs(" U", output); /* u u */
    }
    else
    {
      (void) ungetc(c, input);
      PSputs(" u", output);   /* statepush */
    }
    showcount = 0;
  }
}

/* Are we sure the `O' = `o o' works when at edge of 64 size stack ? */

static void do_pop (FILE *output, FILE *input)
{
  int c;

  if (skipflag == 0)
  {
    if (stinx % 64 == 0)    /* time to retrieve saved stack ? */
    {
      stinx--;
      PSputs(" popstack\n", output);
      showcount = 0; 
      return;
    }

    stinx--;
    c = getc(input);

    if (c == (int) pop && (stinx % 64 != 0))
    {
      stinx--;
      PSputs(" O", output);   /* o o */
    }
    else
    {
      if (c == (int) push)
      {
        stinx++;
        PSputs(" M", output);
      }   /* o u - OK if M defined as `o u' */
      else
      {
        (void) ungetc(c, input);
        PSputs(" o", output); /* statepop */
      }
    }
    showcount = 0;
  }
}

static void complain_char_code (unsigned long c)
{
  sprintf(logline, " Character code %lu > 255\n", c);
  showline(logline, 1);
}

/* come here either with character code 0 - 127 --- or from set1 */
/* in the case of set1 we need to read the next byte, which likely is > 127 */

static void normal_char (FILE *output, FILE *input, int c)
{
  int d;

  if (skipflag == 0)
  {
    if (showcount > MAXSHOWONLINE)    /* too much on one line ? */
    {
      PSputc('\n', output);
      showcount = 0;  /* first go to new line */
    } 
    PSputc('(', output);
  }

  while (c < 128 || c == (int) set1)    /* changed ! */
  {
    if (c == (int) set1)
      c = getc(input); /* new ! read next byte */

    if (skipflag == 0)
    {
      if (bRemapControl || bRemapFont)
      {
        if (c < MAXREMAP)
          c = remaptable[c];
#if MAXREMAP < 128
        else if (c == 32)
          c = 195;
        else if (c == 127)
          c = 196;
#endif
      }
/* NOTE: this must match corresponding code in DVIPSLOG.C */
      else if (bRemapSpace && c <= 32)  /* 1995/Oct/17 */
      {
        if (c == 32) c = 195;   /* not 160 */
        else if (c == 13) c = 176;  /* 1996/June/4 */
        else if (c == 10) c = 173;  /* 1996/June/4 */
        else if (c == 9) c = 170;   /* 1996/June/4 */
        else if (c == 0) c = 161;
      }

      if (c < 32 || c >= 127 || c == 37)
      {
        if (c <= 13 && c >= 8 && c != 11 && bForwardFlag != 0)
        {
/* use special escape \b \t \n ... \f \r for 8 ... 13 1993/Sep/29 */
          PSputc('\\', output);
          PSputc(escapecode[c - 8], output);
        }
        else if (bBackWardFlag == 1) /* compatibility with old ALW */
        {
          d = getc(input); (void) ungetc(d, input); /* peek  */

          if ((d >= 32 && d <= 127) || d == (int) set1)
          {
//            fprintf(output, "\\%03o", c);  /* just to be safe */
            sprintf(logline, "\\%03o", c);  /* just to be safe */
          }
          else
          {
            sprintf(logline, "\\%o", c);
          }
          
          PSputs(logline, output);
        }
        else {    /* following not always safe for old ALW ... */
          d = getc(input); (void) ungetc(d, input); /* peek  */
          if ((d >= '0' && d <= '7') || d == (int) set1) {
//            fprintf(output, "\\%03o", c);  /* just to be safe */
            sprintf(logline, "\\%03o", c);  /* just to be safe */
          }
          else {
            sprintf(logline, "\\%o", c);
          }
          PSputs(logline, output);
        }
      }
      else /* not control characters */
      {
        if (c == '\\' || c == '(' || c == ')')
        {
          PSputc('\\', output);
        }

        PSputc(c, output);
      }
    }

    c = getc(input);    /* get the next byte in DVI file */

    if (c < 0)
      break;   /* trap EOF - avoid loop */
  } /* end of while (c < 128 ... ) loop */

/* analyze next DVI command to see whether can combine with above */
  if (skipflag != 0)
    (void) ungetc(c, input);
  else
  {
    if (c == (int) w0)
    {
      PSputs(")S", output);
    }
    else if (c == (int) x0)
    {
      PSputs(")T", output);
    }
    else
    {
      PSputs(")s", output);
      (void) ungetc(c, input);  /* can't use it, put it back */
    }

    showcount++;
  }
/*  also need to increase h by total width */
/*  fprintf(output, "currentpoint pop /h exch def\n"); */
}

/* Following need *not* be efficient since it is rarely used */
/* Put single character - called by put1/put2/put3/put4 */
/* Set single character - also called by set2/set3/set4 */
/* duplicates a lot of code form normal_char() above */

/* separated out 1995/June/30 */  /* third arg is `s' or `p' */

static void do_charsub (FILE *output, unsigned long c, char code)
{
  if (skipflag == 0)
  {
    if (bRemapControl || bRemapFont)
    {
      if (c < MAXREMAP)
        c = remaptable[c];

#if MAXREMAP < 128
      else if (c == 32)
        c = 195;
      else if (c == 127)
        c = 196;
#endif
    }
    else if (bRemapSpace && c <= 32)
    {
      if (c == 32) c = 195;   /* not 160 */
      else if (c == 13) c = 176;  /* 1996/June/4 */
      else if (c == 10) c = 173;  /* 1996/June/4 */
      else if (c == 9) c = 170;   /* 1996/June/4 */
      else if (c == 0) c = 161;
    }

    if (c >= 256)
    {
      complain_char_code(c);
      return;       /* ignore it - should never happen */
    }

    PSputc('(', output); 

    if (c < 32 || c >= 127 || c == 37)
    {
      if (c <= 13 && c >= 8 && c != 11 && bForwardFlag != 0)
      {
        /* use special escape \b \t \n ... \f \r for 8 ... 13 1993/Sep/29 */
        PSputc('\\', output);
        PSputc(escapecode[c - 8], output);
      }
      else if (bBackWardFlag == 1)  /* compatibility with old ALW */
      {
        sprintf(logline, "\\%03o", (unsigned int) c);
        PSputs(logline, output);
      }
      else  /* following not always safe for old ALW ... */
      {
        sprintf(logline, "\\%o", (unsigned int) c);
        PSputs(logline, output);
      }
    }
    else
    {
      if (c == '\\' || c == '(' || c == ')')
      {
        PSputc('\\', output);
      }

      PSputc((unsigned int) c, output);
    }

    PSputc(')', output); 
    PSputc(code, output); /* 'p' or 's' */
    showcount++;
  }
}

/* could be more efficient here if we ever see several in a row OK */
/* model on "normal_char" if needed OK */

static void do_set1 (FILE *output, FILE *input)
{
  if (skipflag == 0)
  {
    normal_char(output, input, (int) set1);
  }
  else
    (void) getc(input);

/* read following byte and throw it away */
/* set character c and increase h by width of character */
/* used (normally only) for characters in range 128 to 255 */
}

static void do_set2 (FILE *output, FILE *input)
{
  do_charsub(output, ureadtwo(input), 's');
}

static void do_set3 (FILE *output, FILE *input)
{
  do_charsub(output, ureadthree(input), 's');
}

static void do_set4 (FILE *output, FILE *input)
{
  do_charsub(output, ureadfour(input), 's');
}

/* set character c and DO NOT increase h by width of character */

static void do_put1 (FILE *output, FILE *input)
{
  do_charsub(output, getc(input), 'p');
}

static void do_put2 (FILE *output, FILE *input)
{
  do_charsub(output, ureadtwo(input), 'p');
}

static void do_put3 (FILE *output, FILE *input)
{
  do_charsub(output, ureadthree(input), 'p');
}

static void do_put4 (FILE *output, FILE *input)
{
  do_charsub(output, ureadfour(input), 'p');
}

/* For PDF problems we adjust height of horizontal rule 95/Oct/15 */
/* but we don't adjust the position of the rule ... and */
/* if we were to adjust width of vertical rules we'd need to adjust position */

static void do_common_rule (FILE *output, FILE *input, char *s)
{
  long a, b; /* height, width */

  a = sreadfour(input);
  b = sreadfour(input);

  if (skipflag == 0)
  {
    if (nMinRule != 0 && a > 0)
    {
      /* Make sure we don't get zero width rules in PDF output... */
      /* ... compensate for truncating down instead of rounding in pdf */
      if (a < nMinRule)
        a = nMinRule;
      else if (a > nMinRule)
        a = a + nMinRule / 2;
    }

    if (bRuleColor)
    {
      if (! freshflag)
        PSputc('\n', output);

      sprintf(logline, "%lg %lg %lg rgb ", rule_red, rule_green, rule_blue);
      PSputs(logline, output);
      freshflag = 0;
    }
    else
    {
      if (bTextColor)
      {
        if (! freshflag) PSputc('\n', output);
        PSputs("black ", output);
        freshflag = 0;
      }
    }

    /* some silly nonsense about using a height = -2^31 in set_rule */
    if (bDVICopyReduce && -a == 2147483648L)
    {
      /* need to do nothing for pr, no output, no motion */
      if (strcmp (s, "sr") == 0)
      {
        if (! freshflag)
          PSputc('\n', output);

#ifdef ALLOWSCALE
        if (outscaleflag)
        {
          sprintf(logline, "%.9lg r", (double) b / outscale);
        }
        else
#endif
        {
          sprintf(logline, "%ld r", b); /* setrule => right */
        }

        PSputs(logline, output);
        freshflag = 0;
      }
    }
    else
    {
      if (! freshflag)
        PSputc('\n', output);

#ifdef ALLOWSCALE
      if (outscaleflag)
      {
        sprintf(logline, "%.9lg %.9lg %s",
            (double) a / outscale, (double) b / outscale, s);
      }
      else
#endif
      {
        sprintf(logline, "%ld %ld %s", a, b, s); /* setrule or putrule */
      }

      PSputs(logline, output);
      freshflag = 0;
    }

    if (bTextColor)
    {
      if (! freshflag)
        PSputc('\n', output);

      sprintf(logline, "%lg %lg %lg rgb ", text_red, text_green, text_blue);
      PSputs(logline, output);
      freshflag = 0;
    }
    else
    {
      if (bRuleColor)
      {
        if (! freshflag) PSputc('\n', output);
        PSputs("black ", output);
        freshflag = 0;
      }
    }

    showcount = 0;
  }
}

static void do_set_rule (FILE *output, FILE *input)
{
  do_common_rule (output, input, "sr");
}

static void do_put_rule (FILE *output, FILE *input)
{
  do_common_rule (output, input, "pr");
}

/* write TeX /counter's */
static char *show_counters (char *s)
{
  int k;
  int kmax = 0;

  sprintf(s, "%ld", counter[0]); /* *always* write first one */
  s += strlen(s);

  for (k = 10-1; k > 0; k--)
  {
    if (counter[k] != 0)
    {
      kmax = k;
      break;
    }
  }

  for (k = 1; k <= kmax; k++) /* write others if non-zero */
  {
    sprintf(s, " %ld", counter[k]);
    s += strlen(s);
  }

  return s;
}

/*** code for working way into back end of file looking for post ***/

#define BUFSIZE  128 /* buffer size to read in at one time */
#define NUMSTEPS 32  /* number of buffers to try from end of file */
#define MAGIC    223 /* magic code used by TeX at end of DVI file */

/* This does some things to work around possible crap at end of file */
/* The way to loose is get garbage at end that comes from other DVI file ! */

/* search for post at end of file */
long goto_post(FILE *input)
{
  unsigned long n;
  int c, d, e, f, k, i, j, count;
  int buffer[BUFSIZE];
  long nlen;

  if (fseek(input, - (long) BUFSIZE, SEEK_END) < 0)
  {
    rewind(input);      /* possibly because file shorter than BUFSIZE ? */
  }

  for (j = 0; j < NUMSTEPS; j++) /* let's not go on forever ! */
  {
    for (k = 0; k < BUFSIZE; k++)
    {
      buffer[k] =  getc(input);
    }

    k = BUFSIZE - 1;

    while (k > 10)
    {
      count=0;            /* count MAGIC codes seen */

      for (i = k; i >= 5; i--)    /* need at least seq of four */
      {
        if (buffer[i] == MAGIC)
        {
          count++;

          if (count == 4)
            break;
        }
        else
          count = 0;
      }

      k = i;

      if (count == 4)    /* found sequence of four */
      {
        for (i = k; i >= 5; i--)   /* but there can be many more */
          if (buffer[i] != MAGIC)
            break;

        k = i;             /* first non MAGIC - ID_BYTE ? */

        if (buffer[k] != MAGIC) /* did see end of MAGIC stuff */
        {
          if (buffer[k-5] == (int) post_post)
          { 
            k = k - 5;    /* step back to post_post */
            c = buffer[k+1];
            d = buffer[k+2];
            e = buffer[k+3];
            f = buffer[k+4];
            n = ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
            fseek(input, (long) n, SEEK_SET); /* go to post ! */
            c = getc(input); (void) ungetc(c, input);

            if (c != (int) post)
            {
              showline("ERROR: Unable to find pointer to POST", 1);
              giveup(5);
              return 0;
            }
            else
            {
              if (traceflag)
              {
                sprintf(logline, "Found POST at %ld\n", n);
                showline(logline, 0);
              }

              return n;   /* seem to be in good shape */
            }
          }
        }
      }
    }

    if (fseek(input, - (long) (BUFSIZE * 2 - 10), SEEK_CUR) != 0)
    {
      break;
    }
  }

  sprintf(logline, "ERROR: Can't find proper ending of DVI file `%s'\n",
      filenamex);
  showline(logline, 1);
  fseek(input, 0, SEEK_END);
  nlen = ftell(input);        /* get length of file 99/Mar/21 */
  sprintf(logline, "Searched near end of file of %ld bytes\n", nlen);
  showline(logline, 0);
  giveup(5);

  return 0;
}

static void insert_blank (FILE *output, long page)
{
  PSputs("dvidict begin\n", output);

  if (newbopflag)
  {
    sprintf(logline, "%ld %ld bop eop end ",
       counter[0], page);
    PSputs(logline, output);
  }
  else
  {
    PSputs("bp ep end ", output);
  }

  PSputs("% blank page\n", output);
}

static void do_counter_comment (FILE *output)
{
  char *s;

  s = logline;
  strcpy(s, "% [");
  s += strlen(s);
  s = show_counters(s);
  strcat(s, "]");
  PSputs(logline, output);
}

/* beginning of page */
static void do_bop (FILE *output, FILE *input)
{
  int k;
  long pageno;        /* page number logical or physical */
  long page;          /* always dvi page count from start of file */
  double xoffset, yoffset;  /* 1992/July/11 */
  COLORSPEC SavedColor;   /* 1999/Apr/06 */

  if (skiptoend != 0)
  {
    goto_post(input);
    skiptoend = 0;
    return;
  }

  /*  Normally bRepeatMode == 0 */
  if (nRepeatCount > 1)
  {
    if (nRepeatIndex == 0)
      currentpagestart = ftell(input) - 1;  /* right at bop */
    else
      pagenumber--;            /* compensate */
  }

  pagenumber++;       /* DVI page count from start of job */

  if (reverseflag)
    page = dvi_t - pagenumber + 1;
  else
    page = pagenumber;

  stinx = 0;
  ff = -1; 
  pagetpic = 0;
  complainedaboutj=0;

  if (bCarryColor == 0)
    colorindex=0;

  if (bCarryColor && bColorUsed)
  {
    RestoreColorStack(page);
  }

  clipstackindex = 0;     /* reset push pop stack 98/Sep/12 */
  CTMstackindex= 0;   /* reset CTM stack pointer in dvispeci.c */

  for (k = 0; k < 10; k++)
    counter[k] = sreadfour(input); 

  previous = sreadfour(input);
  showcount = 0;

  if (countzeroflag != 0)
    pageno = counter[0];
  else
    pageno = (long) page;

  skipflag = skip_this_page(pageno);

  if (skipflag != 0)
    firstpage = -1;
  else if (skipflag == 0)  /* page in valid range */
  {
    if (oddpageflag != 0)   /* if printing only odd pages */
    {
      if ((counter[0] & 1) == 0) /* seen even numbered page */
      {
        if (firstpage != 0)
          insert_blank(output, page);    /* matching blank */

        skipflag++; 
      }

      firstpage = 0;      
    }

    if (evenpageflag != 0)  /* if printing only even pages */
    {
      if ((counter[0] & 1) == 1) /* seen odd numbered page */
      {
        if (firstpage != 0)
          insert_blank(output, page);  /* matching blank */

        skipflag++;
      }

      firstpage = 0;
    }
  }

  if (skipflag != 0) /* skipping this page */
  {
    if (reverseflag != 0)
    {
      if (previous > 0)
        fseek(input, previous, SEEK_SET);
      else
        finish = -1; 
    }

    return;
  }
  else  /* not skipping this page */
  {
    if (verboseflag)
    { 
      showline("[", 0);
      show_counters(logline);
      showline(logline, 0); 
    } 
    else
    {
      showline(".", 0);
    }
/*    note: first item after Page: is a page label - here counter[0] */
/*    (or counter[1]-counter[2]) 1996/Jan/28 */
/*    note: first item after Page: need not be a number */
/*    note: second item after Page: is sequential page number */
/*    An experiment 1995/Aug/27 */
/*    page = numpages + 1; */
    if (stripcomment == 0)
    {
      PSputs("%%Page: ", output);

      if (bUseCounters)
      {
        sprintf(logline, "%ld-%ld %ld\n",
          counter[1], counter[2], numpages+1);  /* 1996/Jan/20 */
      }
      else
      {
        sprintf(logline, "%ld %ld\n", counter[0], numpages+1);
      }

      PSputs(logline, output);
    }

    PSputs("dvidict begin ", output);

    if (evenoddoff != 0)
    {
      if ((counter[0] & 1) == 1) /* seen odd numbered page */
      {
        xoffset = xoffseto;
        yoffset = yoffseto;
      }
      else /* seen even numbered page */
      {
        xoffset = xoffsete;
        yoffset = yoffsete;
      }

      sprintf(logline, 
        "/xoffset %lg def /yoffset %lg def\n", xoffset, yoffset);
      PSputs(logline, output);
    }

    PSputc('\n', output);   // always start new line for this

    if (newbopflag)
    {
      sprintf(logline, "%ld %ld bop ", counter[0], numpages+1);
      PSputs(logline, output);
    }
    else
      PSputs("bp ", output);

    if (stripcomment == 0)
      do_counter_comment (output);

    if (bBackGroundFlag && bBackUsed)
    {
      if (BackColors[page].A != -1.0 ||
        BackColors[page].B != -1.0 ||
        BackColors[page].C != -1.0)
      {
        PSputc('\n', output);
        PSputs("gsave clippath ", output);
        SavedColor = CurrColor;     /* save around following */
        CurrColor = BackColors[page];
        doColorSet(output, 3);      /* background - in dvispeci.c */            
        PSputs("fill grestore ", output);
        CurrColor = SavedColor;     /* restore after 99/Apr/06 */
      }
    }

    /* now pop color pushed at bottom of previous page (restored stack up above) */
    if (bColorUsed && (colorindex > 0))
    {
      doColorPop(page); 
      doColorSet(output, 2); /* bop - in dvispeci.c */
    }
    else
    {
      PSputc('\n', output); /* omission slightly risky ... */
    }
  }
/*  maybe also do "structuring conventions" stuff ? */
}

/* end of page */
static void do_eop (FILE *output, FILE *input)
{
  int c;

  if (bAbort)
    abortjob();

  if (abortflag)
    return;

  showcount = 0;

  if (colorindex > 0)
    checkColorStack(output);

  if (clipstackindex > 0)
    doClipBoxPopAll(output);

  if (skipflag == 0)
  {
    if (CTMstackindex != 0)
      checkCTM(output);

    PSputc('\n', output); // always start new line

    if (newbopflag)
      PSputs("eop ", output);
    else
      PSputs("ep ", output);

    if (stripcomment == 0)
      do_counter_comment(output);

    PSputc('\n', output);
    PSputs("end", output);

    if (stripcomment == 0)
      PSputs(" % dvidict", output);

    PSputc('\n', output);

    /* %%PageTrailer comments highly optional ... */
    if (bOptionalDSC)
    {
      if (stripcomment == 0)
      {
        PSputs("%%PageTrailer\n", output);
      }
    }

    if (verboseflag)
      showline("] ", 0);

    numpages++;   /* update number of pages actually processed */
  }

  if (output != NULL && ferror(output))
  {
    showline("\n", 0);
    showline("ERROR in output file", 1);
    perrormod((outputfile != NULL) ? outputfile : "");
    giveup(7);
    return;
  }

  /*  Normally bRepeatMode == 0 */
  if (nRepeatCount > 1)
  {
    nRepeatIndex++;

    if (nRepeatIndex == nRepeatCount)
      nRepeatIndex = 0;
    else
    {
      fseek (input, currentpagestart, SEEK_SET);
      return;
    }
  }

  skipflag = 0;

  if (reverseflag != 0) /* go back if reading in reverse */
  {
    if (previous > 0)
      fseek(input, previous, SEEK_SET);
    else
      finish = -1;
  }

  if (textures != 0)
    (void) ureadfour(input); /* skip over length code */

  /*  may also want to check whether length is something reasonable ? */
  c = getc(input); (void) ungetc(c, input);   /* peek ahead */

  if (c >= 0 && c <= 127)
  {
    sprintf(logline, " invalid code (%d)\n", c);
    showline(logline, 1);
    finish = -1;
  }
}

/* rare */
static void do_right1 (FILE *output, FILE *input)
{
  int b;

  b = sreadone(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg r", (double) b / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%d r", b); /* right */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + b; */
  }
}

static void do_right2 (FILE *output, FILE *input)
{
  int b;

  b = sreadtwo(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg r", (double) b / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%d r", b);  /* right */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + b; */
  }
} 

static void do_rightsub (FILE *output, long b)
{
  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg r", (double) b / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld r", b); /* right */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + b; */
  }
}

static void do_right3 (FILE *output, FILE *input)
{
  do_rightsub(output, sreadthree(input));
}

static void do_right4 (FILE *output, FILE *input)
{
  do_rightsub(output, sreadfour(input));
}

static void do_w0 (FILE * output)
{
  if (skipflag == 0)
  {
    PSputs(" w", output); /* wright */
    showcount = 0;
/*    h = h + w; */
  }
}

/* rare */
static void do_w1 (FILE *output, FILE *input)
{
  long w; /* trial */

  w = sreadone(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg W", (double) w / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld W", w); /* wsetright */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + w; */
  }
}

static void do_w2 (FILE *output, FILE *input)
{
  long w; /* trial */

  w = sreadtwo(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg W", (double) w / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld W", w); /* wsetright */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + w; */
  }
}

static void do_wsub (FILE *output, long w)
{
  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg W", (double) w / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld W", w); /* wsetright */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + w; */
  }
}

static void do_w3 (FILE *output, FILE *input)
{
  do_wsub(output, sreadthree(input));
}

static void do_w4 (FILE *output, FILE *input)
{
  do_wsub(output, sreadfour(input));
}

static void do_x0 (FILE *output)
{
  if (skipflag == 0)
  {
    PSputs(" x", output); /* xright */
    showcount = 0;
/*    h = h + x; */
  }
}

/* rare */
static void do_x1 (FILE *output, FILE *input)
{
  long x; /* trial */

  x = sreadone(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg X", (double) x / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld X", x); /*  xsetright */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + x; */
  }
}

static void do_x2 (FILE *output, FILE *input)
{
  long x; /* trial */

  x = sreadtwo(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg X", (double) x / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld X", x); /* xsetright */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + x; */
  }
}

static void do_xsub (FILE *output, long x)
{
  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg X", (double) x / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld X", x); /* xsetright */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  h = h + x; */
  }
}

static void do_x3 (FILE *output, FILE *input)
{
  do_xsub(output, sreadthree(input));
}

static void do_x4 (FILE *output, FILE *input)
{
  do_xsub(output, sreadfour(input));
}

/* rare */
static void do_down1 (FILE *output, FILE *input)
{
  int a;

  a = sreadone(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg d", (double) a / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%d d", a); /* down */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + a; */
  }
}

/* rare */
static void do_down2 (FILE *output, FILE *input)
{
  int a;

  a = sreadtwo(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg d", (double) a / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%d d", a); /* down */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + a; */
  }
}

static void do_downsub (FILE *output, long a)
{
  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg d", (double) a / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld d", a); /* down */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + a; */
  }
}

static void do_down3 (FILE *output, FILE *input)
{
  do_downsub(output, sreadthree(input));
}

static void do_down4 (FILE *output, FILE *input)
{
  do_downsub(output, sreadfour(input));
}

static void do_y0 (FILE *output)
{
  if (skipflag == 0)
  {
    PSputs(" y", output);
    showcount = 0;
/*    v = v + y; */
  }
}

/* rare */
static void do_y1 (FILE *output, FILE *input)
{
  long y; /* trial */

  y = sreadone(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg Y", (double) y / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld Y", y); /* ysetdown */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + y; */
  }
}

static void do_y2 (FILE *output, FILE *input)
{
  long y; /* trial */

  y = sreadtwo(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg Y", (double) y / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld Y", y); /*  ysetdown */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + y; */
  }
}

static void do_ysub (FILE *output, long y)
{
  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg Y", (double) y / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld Y", y); /* ysetdown */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + y; */
  }
}

static void do_y3 (FILE *output, FILE *input)
{
  do_ysub(output, sreadthree(input));
}

/* not used */
static void do_y4 (FILE *output, FILE *input)
{
  do_ysub(output, sreadfour(input));
} 

static void do_z0 (FILE *output)
{
  if (skipflag == 0)
  {
    PSputs(" z", output);
    showcount = 0;
/*    v = v + z; */
  }
}

/* rare */
static void do_z1 (FILE *output, FILE *input)
{
  long z; /* trial */

  z = sreadone(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg Z", (double) z / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld Z", z); /* zsetdown */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + z; */
  }
}

static void do_z2 (FILE *output, FILE *input)
{
  long z; /* trial */

  z = sreadtwo(input);

  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg Z", (double) z / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld Z", z); /* zsetdown */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + z; */
  }
}

static void do_zsub (FILE *output, long z)
{
  if (skipflag == 0)
  {
    if (! freshflag)
      PSputc('\n', output);

#ifdef ALLOWSCALE
    if (outscaleflag)
    {
      sprintf(logline, "%.9lg Z", (double) z / outscale);
    }
    else
#endif
    {
      sprintf(logline, "%ld Z", z); /* zsetdown */
    }

    PSputs(logline, output);
    freshflag = 0;
    showcount = 0;
/*  v = v + z; */
  }
}

static void do_z3 (FILE *output, FILE *input)
{
  do_zsub(output, sreadthree(input));
}

static void do_z4 (FILE *output, FILE *input)
{
  do_zsub(output, sreadfour(input));
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static void complain_font_code (unsigned long fs)
{
  sprintf(logline, " Bad font code %lu (> %u)\n", fs, MAXFONTNUMBERS-1);
  showline(logline, 1);
}

/* switching to other font */
static void switch_font (FILE *output, int fs)
{
  int fn;

  if (fs < 0)
  {
    sprintf(logline, "Negative font number %d\n", fs);
    showline(logline, 1);
  }

  if (fs >= MAXFONTNUMBERS)
    complain_font_code(fs);

  ff = fs; /* set state */

  if (skipflag == 0)
  {
    if (bShortFont != 0)
      fn = finx[ff];
    else
      fn = fs;

    PSputc('\n', output);   // always on new line
    sprintf(logline, "f%d", fn);
    PSputs(logline, output);
  }

  showcount = 0;
}

/* switch fonts */
static void do_fnt1 (FILE *output, FILE *input)
{
  unsigned int fs;

  fs = getc(input);
  switch_font(output, (int) fs);
}

static void do_fnt2 (FILE *output, FILE *input)
{
  unsigned int fs;

  fs = ureadtwo(input);

  if (fs >= MAXFONTNUMBERS)
  {
    complain_font_code (fs);
    fs = MAXFONTNUMBERS - 1;
  }

  switch_font(output, (int) fs);
}

static void do_fntsub (FILE *output, unsigned long fs)
{
  if (fs >= MAXFONTNUMBERS)
  {
    complain_font_code (fs);
    fs = MAXFONTNUMBERS-1;
  }

  switch_font(output, (int) fs);
}

static void do_fnt3 (FILE *output, FILE *input)
{
  do_fntsub(output, ureadthree(input));
}

static void do_fnt4 (FILE *output, FILE *input)
{
  long fs;

  fs = sreadfour(input);

  if (fs < 0)
  {
    sprintf(logline, "Font code %ld < 0\n", fs);
    showline(logline, 1);
    fs = 0;
  }

  do_fntsub(output, (unsigned long) fs);
}

static void do_xxxi (FILE *output, FILE *input, unsigned int n)
{
/*  int c; */
  unsigned int k;

  if (skipflag)
  {
    if (bCarryColor)
      prereadspecial(input, n);
    else for(k = 0; k < n; k++)
      (void) getc(input);
  }
  else
    readspecial(output, input, (unsigned long) n);

  showcount = 0;
}

static void do_xxx1 (FILE *output, FILE *input)
{
  unsigned n;

  n = getc(input);
  do_xxxi(output, input, n);
}

static void do_xxx2 (FILE *output, FILE *input)
{
  unsigned int n;

  n = ureadtwo(input);
  do_xxxi(output, input, n);
}

static void do_xxxl (FILE *output, FILE *input, unsigned long n)
{
  unsigned long k;

  if (skipflag)
  {
    if (bCarryColor)
      prereadspecial(input, n);
    else for(k = 0; k < n; k++) (void) getc(input);
  }
  else
    readspecial(output, input, n);

  showcount = 0;
}

static void do_xxx3 (FILE *output, FILE *input)
{
  unsigned long n;

  n = ureadthree(input);
  do_xxxl(output, input, n);
}

static void do_xxx4 (FILE *output, FILE *input)
{
  unsigned long n;

  n = ureadfour(input);
  do_xxxl(output, input, n);
}

/* need to do this even if skipping pages */

/* nothing much should actually happen here !!! */

static void fnt_def (FILE *output, FILE *input, unsigned int k)
{
  unsigned int na, nl, i;
  int f, newfont = 1;
  char namebuffer[FNAMELEN];
  char *fp;

  if (finx[k] != BLANKFONT)
  {
    newfont = 0;
    f = finx[k];
  }
  else
  {
    f = fnext;

    if (finx[k] != (short) f)
    {
      showline(" ERROR: Inconsistency between passes\n", 1);
      errcount(0);
    }

    fnext++;
  }

  for (k = 0; k < 12; k++)
    (void) getc(input);

  na = getc(input);
  nl = getc(input);

  if (newfont == 0)
  {
    for (i = 0; i < na + nl; i++)
      (void) getc(input);
  }
  else
  {
    sprintf(logline, " ERROR: Redefining font %d\n", f);
    showline(logline, 1);
/*    fp = fontname[f]; */
    fp = namebuffer;

    if (na + nl >= sizeof(namebuffer)-1)
    {
      sprintf(logline, "Font name too long: %d (> %d) ",
          na + nl, sizeof(namebuffer)-1);
      showline(logline, 1);
      showline("\n", 0);
      errcount(0);
      tellwhere(input, 1);

      for (i = 0; i < na + nl; i++)
        (void) getc(input);
    }
    else
    {
      for (i = 0; i < na + nl; i++)
        *fp++ = (char) getc(input);
    }

    *fp++ = '\0';

    if (fontname[f] != NULL)
      free(fontname[f]);

    fontname[f] = zstrdup(namebuffer);
    fontsubflag[f] = -1; /* all this goes to extract */
  }
}

static void do_fnt_def1 (FILE *output, FILE *input)
{
  unsigned int k;

  k = getc(input);
  fnt_def(output, input, k);
}

static void do_fnt_def2 (FILE *output, FILE *input)
{
  unsigned int k;

  k = ureadtwo(input);

  if (k >= MAXFONTNUMBERS)
  {
    complain_font_code (k);
    k = MAXFONTNUMBERS-1;
  }

  fnt_def(output, input, (unsigned int) k);
}

static void do_fnt_defsub (FILE *output, FILE *input, unsigned long k)
{
  if (k >= MAXFONTNUMBERS)
  {
    complain_font_code (k);
    k = MAXFONTNUMBERS-1;
  }

  fnt_def(output, input, (unsigned int) k);
}

static void do_fnt_def3 (FILE *output, FILE *input)
{
  do_fnt_defsub(output, input, ureadthree(input));
}

static void do_fnt_def4 (FILE *output, FILE *input)
{
  long k;

  k = sreadfour(input);

  if (k < 0)
  {
    sprintf(logline, "Font code %ld < 0\n", k);
    showline(logline, 1);
    k = 0;
  }

  do_fnt_defsub(output, input, (unsigned long) k);
}

/* need to do this even if skipping pages */
/* doesn't do output */
static void do_pre (FILE *output, FILE *input)
{
  unsigned int k, j;

  (void) getc(input);

  for (j = 0; j < 12; j++)
    (void) getc(input);

  k = getc(input);

  for (j = 0; j < k; j++)
    (void) getc(input);

  if (textures != 0)
    (void) ureadfour(input); /* skip over length code */
}

/* need to do this even if skipping pages */
/* doesn't do output */
static void do_post (FILE *output, FILE *input)
{
  int k;

  previous = sreadfour(input);  /* was ureadfour ... */

  if (traceflag)
    showline("Hit POST!\n", 0);

  for (k = 0; k < 12; k++)
    (void) getc(input);

  for (k = 0; k < 8; k++)
    (void) getc(input);

  for (k = 0; k < 4; k++)
    (void) getc(input);

  if (reverseflag == 0)
    finish = -1;

  if (reverseflag != 0)
    fseek(input, previous, SEEK_SET); /* 98/Jul/20 ??? */
}

/* only in reverse ? */
static void do_post_post (FILE *output, FILE *input)
{
  unsigned long previous;
  unsigned int id;

  if (traceflag)
    showline("Hit POSTPOST!\n", 0);  /* never ? */

  previous = ureadfour(input);
  id = getc(input);

  if (reverseflag != 0)
    fseek(input, previous, SEEK_SET); /* go to POST? */
  else
  {
    PSputs("% This is really the end !\n", output); // never!
  }

  if (reverseflag == 0)
    finish = -1;  /* 98/Jul/20 */

  showcount = 0;
}

// main entry point to this part of the program
// lastflag indicates last in set of copies of same page

int scan_dvi_file (FILE *output, FILE *input, int lastflag)
{
  int c, fs;
  long filptr;

#ifdef DEBUGGING
  if (output == NULL)
  {
    sprintf(logline, " NULL %s file\n", "output"); /* debug */
    showline(logline, 1);
  }

  if (input == NULL)
  {
    sprintf(logline, " NULL %s file\n", "input"); /* debug */
    showline(logline, 1);
  }
#endif

  if (countzeroflag)
    resetpagerangehit (0);

  numpages = 0;   /* number of pages actually processed */
  firstpage = -1;   /* flag for two sided printing case */

  if (textures != 0)
    fseek(input, dvistart, SEEK_SET);

  if (reverseflag != 0)
    skiptoend = -1;
  else
    skiptoend = 0;

  pagenumber = 0;     /* value from earlier scan already used */

  finish = 0;
  stinx = 0;

  if (nRepeatCount > 1)
    nRepeatIndex = 0;

  for ( ; ; )
  {
    c = getc(input);

    if (c == EOF)
    {
      sprintf(logline, " Unexpected EOF (%s)\n", "scandvi");
      showline(logline, 1);

      {
        long current = ftell(input);
        sprintf(logline, " at byte %d\n", current);
        showline(logline, 1);
      }

      finish = -1;
      break;
    }

    if (c < 128)
    {
      normal_char(output, input, c);
    }
    else if (c >= 171 && c <= 234)
    {
      fs = (c - 171);
      switch_font(output, fs);
    }
    else
    {
      switch(c)
      {
        case set1:
          do_set1(output, input);
          break;

        case set2:
          do_set2(output, input);
          break;

        case set3:
          do_set3(output, input);
          break;

        case set4:
          do_set4(output, input);
          break;

        case set_rule:
          do_set_rule(output, input);
          break;

        case put1:
          do_put1(output, input);
          break;

        case put2:
          do_put2(output, input);
          break;

        case put3:
          do_put3(output, input);
          break;

        case put4:
          do_put4(output, input);
          break;

        case put_rule:
          do_put_rule(output, input);
          break;

        case nop:
          break;

        case bop:
          do_bop(output, input);
          break;

        case eop:
          do_eop(output, input);
          break;

        case push:
          do_push(output, input);
          break;

        case pop:
          do_pop(output, input);
          break;

        case right1:
          do_right1(output, input);
          break;

        case right2:
          do_right2(output, input);
          break;

        case right3:
          do_right3(output, input);
          break;

        case right4:
          do_right4(output, input);
          break;

        case w0:
          do_w0(output);
          break;

        case w1:
          do_w1(output, input);
          break;

        case w2:
          do_w2(output, input);
          break;

        case w3:
          do_w3(output, input);
          break;

        case w4:
          do_w4(output, input);
          break;

        case x0:
          do_x0(output);
          break;

        case x1:
          do_x1(output, input);
          break;

        case x2:
          do_x2(output, input);
          break;

        case x3:
          do_x3(output, input);
          break;

        case x4:
          do_x4(output, input);
          break;

        case down1:
          do_down1(output, input);
          break;

        case down2:
          do_down2(output, input);
          break;

        case down3:
          do_down3(output, input);
          break;

        case down4:
          do_down4(output, input);
          break;

        case y0:
          do_y0(output);
          break;

        case y1:
          do_y1(output, input);
          break;

        case y2:
          do_y2(output, input);
          break;

        case y3:
          do_y3(output, input);
          break;

        case y4:
          do_y4(output, input);
          break;

        case z0:
          do_z0(output);
          break;

        case z1:
          do_z1(output, input);
          break;

        case z2:
          do_z2(output, input);
          break;

        case z3:
          do_z3(output, input);
          break;

        case z4:
          do_z4(output, input);
          break;

        case fnt1:
          do_fnt1(output, input);
          break;

        case fnt2:
          do_fnt2(output, input);
          break;

        case fnt3:
          do_fnt3(output, input);
          break;

        case fnt4:
          do_fnt4(output, input);
          break;

        case xxx1:
          do_xxx1(output, input);
          break;

        case xxx2:
          do_xxx2(output, input);
          break;

        case xxx3:
          do_xxx3(output, input);
          break;

        case xxx4:
          do_xxx4(output, input);
          break;

        case fnt_def1:
          do_fnt_def1(output, input);
          break;

        case fnt_def2:
          do_fnt_def2(output, input);
          break;

        case fnt_def3:
          do_fnt_def3(output, input);
          break;

        case fnt_def4:
          do_fnt_def4(output, input);
          break;

        case post:
          do_post(output, input);
          break;

        case pre:
          do_pre(output, input);
          break;

        case post_post:
          do_post_post(output, input);
          break;
  
        default:
        {
          finish = -1;
          sprintf(logline, " ERROR: Unrecognized DVI command: %d", c);
          showline(logline, 1);
          filptr = ftell(input);

          if (filptr > 0)
          {
            sprintf(logline, " at byte %ld in DVI file", filptr - 1);
            showline(logline, 0);
          }

          errcount(0);
        }

        break;
      }
    }

    if (c < xxx1 || c > xxx4)
      freshflag = 0;

    if (finish != 0)
      break;

    if (bAbort)
      abortjob(); // fine grained

    if (abortflag)
      break;
  }

  if (abortflag)
    return -1;

  if (verboseflag && lastflag)
  {
    char *s;
    showline("\n", 0);
    s = logline;

    if (statisticsflag)
    {
      sprintf(s, "Max stack depth %d - ", dvi_s);
      s += strlen(s);
      sprintf(s, "%d font slot%s used - ",
        fnext, (fnext == 1) ? "" : "s");
      s += strlen(s);
    }

    // we have a problem if there are more than 65535 pages
    sprintf(s, "DVI file contains %d page%s\n", dvi_t,
      (dvi_t == 1) ? "" : "s");
    showline(logline, 0);
  }

  return 0;
}

/* deal with CMINCH */ /* deal with MANFNT */
/* add in PostScript 2.0 structuring convention bullshit */
/* can use either box or line to draw rules - don't need both */
/* combine /font9 def and /f9 def ? what ? */
/* reduce size of scanfile to allow optimize */
/* deal with other page formats ? A4 ? portrait ? */
/* precompute the scale factor used on each BOP - don't recompute it */
/* quick way to get fonts: go to end of file - NOT NEEDED */
/* alternate way of specifying pages (actual pages versus counter[0] ? */
/* OK use upper case B and E instead of lower case b and e */
/* maybe control how much goes on one output line ? */
/* presently somewhat kludgy in allowing MAXSHOWONLINE items */
/* rather count character columns and check before output if it will fit */
/* avoid bind def in some cases to allow redefinition via special ? */
/* may need to align lower left of rule to underlying grid... */
/* do tests with rules spaced along page to see effect */
/* shorten code for set_rule and put_rule and bp and ep */
/* set up scale constant to use at top of page */
/* improve mf  & set font */
/* should bop include "dvidict begin" and eop include "end" ? */
/* but watch out, fnt_def1 and xxx1 can appear between eop and bop */
/* further compression coding to exploit common sequences ? */
/* d u u u - s o y u - s o o o o - s o o u - s o o o o o */
/* exploit - o u - o o u u - sequences ? - u o already is absent */
/* o o => O,  u u => U,  o u => K,  d u => D */
/* s o y u => M, s o z u => N */
/* also helps make undecypherable ! have under control of flag ? */
/* write as post-processing rules ? use output line buffer ? */
/* also s o y u <n> r is common pattern with fixed <n> */
/* note also common patterns like r(char)s and r(char)S */
/* keep convention that lower case letters are straight DVI command trans */
/* while upper case letters are combinations of sorts */
/* access w, x, y, z off stack (i.e. keep in stack not register) ? */
/* consider sequences of set1 commands - treat like normal_char ? */
/* check on fonthit even when font switch on page that is not printed ? */
/* check on redundant operations that dvipslog already does anyway */
/* for set1 and put1, use number, not string! then use =string ? */
/* check that nothing but nop and font defs happen between eop and bop ? */
/* Implement %%PageTable: ? */
/* avoid shortening octal codes for old interpretors maybe */
/* try and avoid actually giving up if at all possible */
/* when print even pages only, print a blank at first if last page is odd */
/* when print odd pages only, print a blank at first if first is even */
/* when stack gets too deep insert a savestack  - and matching restorestack */
/* bRemapSpace remaps 32 => 195, 13 to 176, 10 => 173, 9 => 170, 0 => 161 */
/* Rational is that text fonts reencoded to TeX 'n ANSI do not use 0 */
/* or 9 or 10, and from now on text fonts will not use 13 for fl, */
/* and TeX does not use 32 in text fonts */
/* But math fonts do use 0, 9, 10, 13 and 32 */
/* but math fonts always have the repetition of 0 - 32 higher up */
/* And for some versions of Acrobat it may be best not to do this */
/* for example transfer material to clipboard is null terminated */
/* 9 is treated as tab, 10 as newline, 13 ignored and 32 ignored */
/* Bytes 250 and 251 are used for left to right typesetting. For instance,
   what follows is the definition of these commands in omega:
   250. Begin a (possibly recursive) reflected segment.
   251. End a (possibly recursive) reflected segment.
   When a DVI-IVD driver encounters a \\{begin\_reflect} command, it should
   skim ahead (as previously described) until finding the matching
   \\{end\_reflect}; these will be properly nested with respect to each
   other and with respect to \\{push} and \\{pop}.  After skimming has
   located a segment of material to be reflected, that segment should be
   re-scanned and obeyed in mirror-image mode as described earlier.  The
   reflected segment might recursively involve
   $\\{begin\_reflect}/\\{end\_reflect}$ pairs that need to be reflected
   again. */
