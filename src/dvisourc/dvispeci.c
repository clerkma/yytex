/* dvispec.c - dealing with TeX's \special commands

   Copyright 1990, 1991, 1992 Y&Y, Inc.
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
   02110-1301 USA.
****************************************************************************/

#include "dvipsone.h"

#define AllowCTM

#define PI (3.141592653)

#define CONVDEG(x) ((x) * 180 / 3.141592653)

/* num/den units of measurement from DVI file */
/* multiply all quantities in DVI file by this to get 10^{-7} meters */
/* in TeX, this is normally set to 25400000/473628672 */ 

int psfigstyle = 0;   /* non-zero if new style DVI2PS special */
int pstagstyle = 0;   /* non-zero if new style DVIPS special */
int pssvbstyle = 0;   /* non-zero if new style DVIPS special */

static char filename[FNAMELEN]; /* name of special file for error message */

struct _stat statbuf;     /* struct stat statbuf;  99/June/26 */

static long fliteral=0;     /* place in file where literal was */
static long nliteral=0;     /* nspecial at start of literal */

/* int flushinit=1; */ /* flush initial colon in textures special */

/* disable other things ? */ /* check stack at specialend ? */

long nspecial;    /* byte count of special */
long nspecialsav; /* saved byte count of special */
long specstart;   /* saved start of \special for error message */

long psoffset, pslength;    /* 96/Sep/12 PS in EPSF with preview */

char moreline[MAXLINE];   // for tokens in special

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*  figure inclusion parameters new style */
/* long pswidth, psheight, psllx, pslly, psurx, psury;  */

/*  bounding box read from EPS file */ /* should be integers - but... */
double xll, yll, xur, yur;

/* error message output of beginning of long item */
char *showbeginning (char *s, char *t, int n)
{
  int k;

  for (k=0; k < n; k++)
    *s++ = *t++;
//  fputs("...\n", output);
  strcat(s, "..\n");
  s += strlen(s);
  return s;
}

/* read next alphanumeric token from special - stop when not alphanumeric */
/* returns 0 if no more tokens found - else returns terminator */
/* what to do if error ? */

int get_alpha_token (FILE *input, char *token, int nmax)
{
  int c, k = 0;
  char *s = token;

  *s = '\0';              /* in case we pop out 96/Aug/29 */
  if (nspecial <= 0) return 0;    /* nothing more in \special */
  if (nmax <= 0) {          /* 95/Aug/30 */
    flush_special(input);
    return 0;           /* error overflow */
  }
  c = getc(input); --nspecial;
  if (c == 0) {
    flush_special(input);
    return 0;           /* first byte is null 96/Aug/29 */
  }
  while (c <= ' ' && nspecial > 0) {  /* over initial white space */
    c = getc(input); --nspecial;
  }
  if (c <= ' ') return 0;       /* nothing more 1993/Sep/7 */
/*  if (nspecial <= 0) return 0; */   /* nothing more */

  while (((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
       (c >= '0' && c <= '9'))) { 
/*    *s++ = (char) c; */
    if (k++ >= nmax) {
      sprintf(logline, " Token in special too long (> %d)\n", nmax);
      showline(logline, 1);
//      showbeginning(errout, token, nmax/2);
      showbeginning(logline, token, nmax/2);
      showline(logline, 1);
//      if (logfileflag) showbeginning(logfile, token, nmax/2);
      errcount(0);    /* ??? */
      flush_special(input);          /* 95/Aug/30 */
      c = 0;      /* so we return 0 */  /* 95/Aug/30 */
/*      c = getc(input); --nspecial;
      while (((c >= 'a' && c <= 'z')  || (c >= 'A' && c <= 'Z')  ||
        (c >= '0' && c <= '9')) && nspecial > 0) {  
        c = getc(input); --nspecial;
      } */
      break;
    }
    *s++ = (char) c;    /* moved down 95/Aug/30 */
    if (nspecial <= 0) {  /* hit end of special ? */
      c = ' ';      /* pretend finished with space */
      break;
    }
    c = getc(input); --nspecial;
  }
  *s = '\0';
/*  if (verboseflag)
    printf("Token %s, terminator `%c' (%d)\n",
      buff, c, c); */   /* DEBUGGING ONLY */
  return c;   /* return terminator */
}

/* flush rest of special to get back to rest of DVI code */

void flush_special (FILE *input)
{
  int c;
  if (nspecial <= 0) return;
  c = getc(input); nspecial--;
  while (nspecial > 0 && c != EOF) {
    c = getc(input); nspecial--;
  }
}
/* read next token from special */ 
/* - either up to white space - or - double quote delimited */
/* returns 0 if no more tokens found - else returns terminator */

int get_token (FILE *input, char *buff, int nmax)
{
  int c, k=0, marker=' ';   // end of token marker
  char *s=buff;

  *s = '\0';            // in case pop out right away
  if (nspecial <= 0) return 0;    /* nothing more in \special */
  if (nmax <= 0) {          /* 95/Aug/30 */
    flush_special(input);
    return 0;           /* error overflow */
  }
  c = getc(input);
  --nspecial;
  while(c <= ' ' && nspecial > 0) { /* ignore initial white space */
    c = getc(input);
    --nspecial;
  }
  if (c <= ' ') return 0;       /* nothing more 1993/Sep/7 */
//  if (nspecial <= 0) return 0;    /* nothing more */

  if (c == '\"') {          /* deal with quoted string */
    marker = '\"';
    if (nspecial <= 0) return 0;  /* nothing more */ /* 93/Sep/7 */
    c = getc(input);
    --nspecial;
  }
//  if (nspecial <= 0) return 0;    /* nothing more */

/*  while (c > ' ' && nspecial >= 0) {    */
  while (c != marker && c >= ' ') {
/*    *s++ = (char) c; */
    if (k++ >= nmax) {
      sprintf(logline, " Token in special too long (> %d)\n", nmax);
      showline(logline, 1);
      showbeginning(logline, buff, nmax/2);
      showline(logline, 1);
//      if (logfileflag) showbeginning(logfile, buff, nmax/2);
      errcount(0);    /* ??? */
      flush_special(input);          /* 95/Aug/30 */
      c = 0;      /* so we return 0 */  /* 95/Aug/30 */
/*      c = getc(input); --nspecial;
      while (c > ' ' && nspecial > 0) { 
        c = getc(input); --nspecial;
      } */
      break;
    }
    *s++ = (char) c;    /* moved down 95/Aug/30 */ 
    if (nspecial <= 0) {  /* hit end of special ? */
      if (marker != ' ') {  /* complain if marker is not ` ' ...*/
        sprintf(logline, " Missing `%c' in special\n", marker);
        showline(logline, 1);
      }         /* above added 1993/Sep/7 */
/*      c = ' ';  */    /* pretend finished with space */
      c = marker;     /* pretend hit desired marker */
      break;
    }
    c = getc(input);
    --nspecial;
  }
  *s = '\0';
  return c;   /* return terminator */
}


/* copy special into line buffer for sscanf */
/* - either double quote delimited - or - up to end of special */

int scan_special (FILE *input, char *buff, int nmax)
{
  int c, k=0;
  char *s=buff;

  *s = '\0';          /* in case we pop out early ... 92/Oct/23 */
  if (nspecial <= 0) return 0;    /* nothing there ? */
  c = getc(input); --nspecial;
  while (c <= ' ' && nspecial > 0) { /* skip over initial white space */
    c = getc(input); --nspecial;
  } 
/*  if (nspecial <= 0) return 0; */   /* BUG ! */
  if (c <= ' ' && nspecial <= 0) return 0;  /* nothing there 92/Oct/23 */

  ungetc(c, input); nspecial++; /* 1993/Jan/23 */ /* step back to first */
  fliteral = ftell(input);    /* remember where this was ??? */
  nliteral = nspecial;
  c = getc(input); --nspecial;  /* 1993/Jan/23 */

  if (c != '\"')  { /* straight text */
    while (nspecial > 0) {
      k++;
      if (k >= nmax) {
/*        showbeginning(errout, buff, nmax/2); */
/*        errcount(0); */ /* ??? */
        flush_special(input);
        return nmax;
      }
      *s++ = (char) c;
      c = getc(input); --nspecial;
    }
    *s++ = (char) c;  /* last one read */
  }
  else { /* double quote delimited string */
    c = getc(input); --nspecial;
    while (c != '\"' && nspecial > 0) {
      k++;
      if (k >= nmax) {
/*        showbeginning(errout, buff, nmax/2); */
/*        errcount(0); */ /* ??? */
        flush_special(input);
        return nmax;
      }
      *s++ = (char) c;
      c = getc(input); --nspecial;
    }
  }
  *s = '\0';
  return k;
}

/* As above, but do not fiddle with "..." 97/Nov/11 needed for "mark" */

int scan_special_raw (FILE *input, char *buff, int nmax)
{
  int c, k=0;
  char *s=buff;

  *s = '\0';          /* in case we pop out early ... 92/Oct/23 */
  if (nspecial <= 0) return 0;    /* nothing there ? */
  c = getc(input); --nspecial;
  while (c <= ' ' && nspecial > 0) { /* skip over initial white space */
    c = getc(input); --nspecial;
  } 
/*  if (nspecial <= 0) return 0; */   /* BUG ! */
  if (c <= ' ' && nspecial <= 0) return 0;  /* nothing there 92/Oct/23 */

  ungetc(c, input); nspecial++; /* 1993/Jan/23 */ /* step back to first */
  fliteral = ftell(input);    /* remember where this was ??? */
  nliteral = nspecial;
  c = getc(input); --nspecial;  /* 1993/Jan/23 */

  while (nspecial > 0) {
    k++;
    if (k >= nmax) {
/*      showbeginning(errout, buff, nmax/2); */
/*      errcount(0); */ /* ??? */
      flush_special(input);
      return nmax;
    }
    *s++ = (char) c;
    c = getc(input); --nspecial;
  }
  *s++ = (char) c;  /* last one read */
  *s = '\0';
  return k;
}

/* read (short) double-quote-delimited string from special string */
/* possibly just use scan_special or get_token instead ? */
/* return value seems to be mostly ignored */

int get_string (FILE *input, char *buff, int nmax)
{
  int c, k = 0;
  char *s=buff;

  *s = '\0';          /* in case we pop out early ... 92/Oct/23 */
  if (nspecial <= 0) return 0;
  c = getc(input); --nspecial;
  while(c != '\"' && nspecial > 0) { /* scan up to double quote */
    c = getc(input); --nspecial;
  }
  if (nspecial <= 0) return 0;
  c = getc(input); --nspecial;
  while(c != '\"' && nspecial > 0) {
    *s++ = (char) c;
    if(k++ >= nmax) {
      sprintf(logline, " String in special too long (> %d)\n", nmax);
      showline(logline, 1);
      showbeginning(logline, buff, nmax/2);
      showline(logline, 1);
//      if (logfileflag) showbeginning(logfile, buff, nmax/2);
      errcount(0); /* ??? */
      c = getc(input); --nspecial;
      while(c > '\"' && nspecial > 0) {
        c = getc(input); --nspecial;
      }
      break;
    }
    c = getc(input); --nspecial;
  }
  *s = '\0';
  return k;
}

/* skip forward to comma in special string */
void skip_to_comma(FILE *input)
{
  int c;
  if (nspecial <= 0) return;
  c = getc(input); --nspecial;
  while (c != ',' && nspecial > 0) {
    c = getc(input); --nspecial;
  }
}
  
/* skip over double-quote-delimited string in special string */
void flush_string (FILE *input)
{
  int c;
  if (nspecial <= 0) return;
  c = getc(input); --nspecial;
  while (c != '\"' && nspecial > 0) {
    c = getc(input); --nspecial;
  }
  if (nspecial == 0) return;
  c = getc(input); --nspecial;
  while (c != '\"' && nspecial > 0 && c != EOF) {
    c = getc(input); --nspecial;
  }
}

/* Copy string from special to output */
/* if first non-blank is indeed a " then copy up to the next " */
/* - otherwise copy to end of special */

/* This skips over leading white space */
/* copy_string starts by emitting '\n' */
/* copy_string ends by emitting '\n' when it is done */

void copy_string (FILE *output, FILE *input)
{
  int c;
  int column=0;
  int nesting=0;          /* 1999/Feb/28 */
  int escape=0;         /* 1999/Feb/28 */

  if (nspecial == 0) return;    /* nothing left to do ... */
  c = getc(input);
  --nspecial;
  while (c == ' ' && nspecial > 0) { /* search for non-blank */
    c = getc(input);
    --nspecial;
  }
  if (nspecial == 0) return;    /* all just blanks ? */
  if (! freshflag) PSputc('\n', output);    /* paranoia --- old */

  if (c == '\"') {      /* double quote delimited ? */
    c = getc(input); --nspecial;
    while (c != '\"' && nspecial > 0 && c != EOF) {
//      putc(c, output);
      PSputc(c, output);
      c = getc(input);
      --nspecial;
    }

//    what happens if nspecial > 0 when we get here ? 99/Dec/19
//    if (nspecial > 0) {
//      c = getc(input);
//      --nspecial;
//      while (nspecial > 0 && c != EOF) {
//        PSputc(c, output);
//        c = getc(input);
//        --nspecial;
//      }
//    }

//    putc('\n', output);  
    PSputc('\n', output);
  }
  else if (bWrapSpecial == 0) {   // just copy it 99/Dec/19
    while (nspecial > 0 && c != EOF) {
//      putc(c, output);
      PSputc(c, output);
      c = getc(input);
      --nspecial; 
    }
    if (c != EOF) {
//      putc(c, output);
      PSputc(c, output);
    }
//    putc('\n', output); 
    PSputc('\n', output); 
  }
  else {              //    LINE WRAPPING
    while (nspecial > 0 && c != EOF) {
/*  The following wraps lines to avoid problems in PS output from \special */
/*  But it does run the danger of putting a line break in a string ... */ 
//      putc(c, output);
      PSputc(c, output);
      c = getc(input);
      --nspecial; 
/* crude effort to prevent exessively long lines - wrap line at space */
      if (c == '\n') column = 0;
      else column++;
      if (escape) escape = 0;
      else if (c == '(') nesting++;
      else if (c == ')') nesting--;
      else if (c == '\\') escape++;
/*      if (bWrapSpecial && column > WRAPCOLUMN && c == ' ') */
      if (column > WRAPCOLUMN && nesting == 0 && c == ' ') {
        c = '\n';
        column = 0;
      }
    }
    if (c != EOF) {
//      putc(c, output);
      PSputc(c, output);
    }
    if (column > 0) {
//      putc('\n', output); 
      PSputc('\n', output); 
    }
  }
  freshflag = 1;    // all of the above end in \n
  showcount = 0;
}

/* copy verbatim PostScript - but strip bracket enclosed crap first */
/* global | local | inline | asis | begin | end <user PS> ??? */
 
void strip_bracket (FILE *output, FILE *input)
{
  int c;

  c = getc(input); nspecial--;
  if (c == '[')  {
    while (c != ']' && nspecial > 0) {
      c = getc(input);
      nspecial--;
    }
    if (nspecial == 0) return;
  }
  else {
    (void) ungetc(c, input);
    nspecial++;
  }
/*  putc('\n', output);  */ /* 94/Jun/25 */
  copy_string(output, input); 
/*  putc('\n', output);   */    /* 1993/June/3 */
}

/* code to find deferred %%BoundingBox: (atend) if required */

#define STEPSIZE 512  /* step back this far at one time */
#define NUMBERSTEPS 8 /* number of steps to try from end of file */

/* try and find bbox at end of file */
/* the following may inefficiently read stuff several times, but, so what */ 

int find_bbox_at_end (FILE *special, char *fname, long pslength)
{
  int k, foundit = 0;

/*  if (pslength > 0) fseek(special, pslength - (long) STEPSIZE, SEEK_SET); */
  if (pslength > 0) {               /* EPSF with header ? */
    if (psoffset > 0)             /* 1996/Sep/12 */
      fseek(special, psoffset + pslength - (long) STEPSIZE, SEEK_SET);
    else fseek(special, pslength - (long) STEPSIZE, SEEK_SET);
  }
  else fseek(special, - (long) STEPSIZE, SEEK_END);
  if (getline(special, line) == 0) return -1;       /* EOF */
  for (k = 0; k < NUMBERSTEPS; k++) {
    while (getline(special, line) != 0) {     
      if (strncmp(line, "%%BoundingBox", 13) == 0) { /* : */
        foundit = 1; break;
      }
    }
    if (foundit != 0) break;
    fseek(special, - (long) (STEPSIZE * 2), SEEK_CUR);
  }
  if (foundit == 0) {
    strcpy(logline, " Can't find %%BoundingBox ");
    strcat(logline, "at end ");
    if (! verboseflag) {
      char *s;
      s = logline + strlen(logline);
      sprintf(s, "in: %s", fname);
    }
    showline(logline, 1);           /* 95/July/15 */
    errcount(0);
    return 0;
  }
  return -1;
}

int skiphpjunkathead (FILE *, int);

/* extract bounding box from inserted eps file and offset */

int read_bbox (FILE *special, char *fname, long pslength)
{
  char *s;
  int c, k;

  c = getc(special);
  ungetc(c, special);
  if (c == 27) skiphpjunkathead(special, 0);    // 99/Oct/14
  
/*  we assume we are at start of PS section ? psoffset ? */
/*  if (psoffset > 0) fseek (special, psoffset, SEEK_SET); */
  k = getline(special, line);     /* step over initial blank lines */
  while (*line < ' ' && k > 0) k = getline(special, line); /* 96/Sep/12 */
/*  Should we be less fussy and also accept say %% here as well as %! ? */
  if (strncmp(line, "%!", 2) == 0) k = getline(special, line);
  else {
    if (! verboseflag) {
      sprintf(logline, "File %s ", fname);
      showline(logline, 1);
    }
/*    fprintf(errout, " does not start with %%!: %s\n", line);  */
    showline(" does not start with %!: ", 1);
    showline("\n", 0);
    showline(line, 0);
/*    putc('\n', errout); */        /* redundant ? 96/Sep/12 */
/*    errcount(0); ? */
  }
/*  scan until a line is found that does not start with %% */
/*  or with %X where X is not space, tab or newline (page 631 PS manual) */
  for (;;) {
/*    if (strncmp(line, "%", 1) != 0) { k=0; break; } */ /* too whimpy */
/*    if (strncmp(line, "%%", 2) != 0) { k=0; break; }  */
    if (*line != '%') { k=0; break; }     /* not a comment */
/*    c = *(line+1); */
/*    if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') */
    if (bAllowBadBox == 0) {          /* 1994/Aug/19 */
      if (*(line+1) <= ' ') { k=0; break; } /* but DSC requires ! */
    }
    if (strncmp(line, "%%EndComments", 13) == 0) { k=0; break; }
    if (strncmp(line, "%%BoundingBox", 13) == 0) {
/*      if (strstr(line, "(at end)") != NULL) */ /* BoundingBox at end */
      if (strstr(line, "(atend)") != NULL ||
        strstr(line, "(at end)") != NULL)   /* 94/Aug/12 */
        k = find_bbox_at_end(special, fname, pslength);
      break;
    }
    if ((k = getline(special, line)) == 0) break;   /* EOF */
  }
  if (k == 0) {
    showline(" Can't find %%BoundingBox ", 1);
    if (! verboseflag) {
      sprintf(logline, "in: %s", fname);
      showline(logline, 1);
    }
    errcount(0);
    rewind(special);
    return 0;
  } 

/*  try and allow for lack of ':' after BoundingBox */
  s = strstr(line, "BoundingBox") + 12;
  if (sscanf(s, "%lg %lg %lg %lg", &xll, &yll, &xur, &yur) < 4) {
    showline(" Don't understand BoundingBox: ", 1);
    showline(line, 1);
    errcount(0);
    rewind(special);
    return 0;
  }
  rewind(special);
  return 1;     /* apparently successful ! */
}

/* Actually use BoundingBox information */
/* if needshift > 0 then (xll, yll) is at TeX's current point */
/* if needshift = 0 then (0,0) is at Tex's current point */
/* if needshift < 0 then (xll, yur) is at TeX's current point  */
/* - last one only used by DVIALW ? */

void deal_with_bbox (FILE *output, FILE *special, char *fname, long pslength, int needshift)
{
  if (read_bbox(special, fname, pslength) > 0) {
/*    now perform shift - if asked for it */
    if (needshift > 0) {
//      fprintf(output, "%lg %lg translate ", -xll, -yll); 
      sprintf(logline, "%lg neg %lg neg translate ", xll, yll);
      PSputs(logline, output);
    }
    else if (needshift < 0) {
//      fprintf(output, "%lg %lg translate ", -xll, -yur); 
      sprintf(logline, "%lg neg %lg neg translate ", xll, yur);
      PSputs(logline, output);
    }
  }
/*  rewind(special); */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Come in with PostScript FontName, try and retrieve font file name */
/* from substitution table entry with *epsf* line */
/* changed 98/Jan/9 to work for all entries in sub table ??? */
/* used in `fopenfont'in dvispeci.c 1994/Aug/15 */
/* returns 0 if not found, +1 if found and not resident, -1 if resident */
/* file name returned in second arg */ /* first arg *is* case sensitive */

int FindFileName (char *fontname, char *filename) /* experiment 94/Aug/15 */
{
  int k;

  if (fontname == NULL) return 0;
  for (k = 0; k < ksubst; k++) {
/*    check whether *epsf* entry of *resident* entry */
/*    do we really need to be this paranoid ? */
/*    if ((fontsubprop[k] & C_EPSF) == 0) continue; */ /* remove 98/Jan/9 */
    if ((fontsubprop[k] & C_EPSF) == 0 && 
        (fontsubprop[k] & C_RESIDENT) == 0) continue; 
/*    strcpy(newname, fontsubto + k * MAXFONTNAME); */
/*    if (strcmp(newname, fontname) == 0) { */
//    if (strcmp(fontname, fontsubto + k * MAXFONTNAME) == 0) {
    if (fontsubto[k] != NULL &&
        strcmp(fontname, fontsubto[k]) == 0) {
/*      strcpy(oldname, fontsubfrom + k * MAXTEXNAME); */
/*      strcpy(filename, oldname); */
//      strcpy(filename, fontsubfrom + k * MAXTEXNAME);
      strcpy(filename, fontsubfrom[k]);
      if ((fontsubprop[k] | C_RESIDENT) != 0) 
        return -1;      /* found it, resident */
      else 
/*        if ((fontsubprop[k] & C_EPSF) != 0) */
        return 1;     /* found it, not resident */
    }     
  }
//  strcpy(filename, "");
  *filename = '\0';
  return 0;         /* not found */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* This tries to find a PFB file corresponding to the given PS FontName */
/* It is quite heuristic, since ATM.INI contains Windows Face Names */
/* Also, it tries to find ATM.INI by trying to find the Windows dir */

/*  Courier=c:\psfonts\pfm\com_____.pfm,c:\psfonts\com_____.pfb */
/*  Courier,BOLD=c:\psfonts\pfm\cob_____.pfm,c:\psfonts\cob_____.pfb */
/*  Courier,BOLDITALIC=c:\psfonts\pfm\cobo____.pfm,c:\psfonts\cobo____.pfb */
/*  Courier,ITALIC=c:\psfonts\pfm\coo_____.pfm,c:\psfonts\coo_____.pfb */

/* atmflag is non-zero if reading ATM.INI which uses FaceName not FontName */
/* atmflag is zero if we are reading user supplied file with real FontName */
/* the result is written back into pfbname argument */

/* Note that pfbname is font file name with full path FNAMELEN */

/* Emits on stdout the font file name (minus trailing _) unless mmflag set */

char *fopenfont_sub (FILE *atmfile, char *FontName, char *pfbname, int atmflag, int mmflag)
{
/*  FILE *atmfile; */
/*  char atmini[FNAMELEN]; */
/*  char *windir; */
  char *s, *co, *eq;
  char *bo, *it, *ro, *en;
  char *sl, *pe;
  int winitalic, winbold;
  int namitalic=0, nambold=0; 
  int c, n, nf;
  
/*  get some info from FontName first (only if reading ATM.INI) */
  if (atmflag) {  /* try for Face Name + style from FontName ! */
    if ((ro = strstr(FontName, "Roman")) != NULL ||
      (ro = strstr(FontName, "Regular")) != NULL) ;
    namitalic = 0; nambold = 0;     /* guess style from FontName */
    if ((bo = strstr(FontName, "Bold")) != NULL ||
      (bo = strstr(FontName, "Demi")) != NULL) nambold = 1;
/*  maybe also "Black", "Heavy", "Ultra" */
    if ((it = strstr(FontName, "Italic")) != NULL ||
      (it = strstr(FontName, "Oblique")) != NULL) namitalic = 1;
/* Try and deal with Multiple Master fonts ... Avoid CMM* however */
    if ((s = strstr(FontName+2, "MM")) != NULL) {
/* UGH, we can't just go and modify the damn FontName string ??? */
/* Shorten Prefix using Mac 5 + 3 + 3 rule ... */ 
      if (s - FontName > 5) {
        strcpy(FontName + 5, s); 
/*        printf("Shortened MM name to %s\n", FontName); */
/*        debugging 95/May/25 */
      }
      if ((s = strstr(FontName, "MM-")) != NULL) {
        strcpy(s+2, s+3); /* flush the hyphen */
/*        printf("Flushed hyphen in MM name %s\n", FontName); */
/*        debugging 95/May/25 */
      }
/* MM Italic and Swash fonts */ /* 95/May/27 */
      if ((s = strstr(FontName, "MMIt")) != NULL) {
        namitalic = 1;
        strcpy(s+2,s+4);  /* flush the "It" */
      }
      else if ((s = strstr(FontName, "MMSw")) != NULL) {
        namitalic = 1;
      }
    }

/*  maybe also just "Obl" */
/*  Try and extract base FontName length (minus style modifiers) */
    en = FontName + strlen(FontName);
    if (ro != NULL && ro < en) en = ro;
    if (bo != NULL && bo < en) en = bo;
    if (it != NULL && it < en) en = it;
    if (*(en-1) == '-') en--;       /* ignore hyphens */
    nf = en - FontName;           /* length of meaningful part */
/*  skip forward to [Fonts] section of ATM.INI */
    while ((fgets (line, sizeof(line), atmfile)) != NULL) { /* MAXLINE */
      if (*line != '[') continue;
      if (strncmp(line+1, "Fonts", 5) == 0) break;
    }
  }
  else nf = strlen(FontName); /* atmflag = 0 => if file with full FontName*/

  strcpy(pfbname, "");            /* in case we drop out */
  while ((fgets (line, sizeof(line), atmfile)) != NULL) { /* MAXLINE */
    if (*line == ';' || *line == '%') continue;   /* ignore comment */
    if (atmflag && *line <= ' ') break;   /* end of [Fonts] section */
    co = strchr(line, ',');     /* comma, if any */
    eq = strchr(line, '=');     /* equal */
    if (eq == NULL) continue;   /* line makes NO sense ! */
    n = eq - line;          /* isolate Face Name itself */
/*   first try for exact match all the way to equal sign */
/*    if (strncmp(FontName, line, n) == 0) { */
    if (n == nf && nambold == 0 && namitalic == 0 &&
      strncmp(FontName, line, n) == 0) {
/* we have an exact match ! jump to extracting PFB file name ! */
    }
    else if (atmflag) {       /* do this only for ATM.INI */
      if (co != NULL && co < eq) n = co - line;
/*      if (strncmp(FontName, line, n) != 0) continue;   */
      if (n != nf || strncmp(FontName, line, n) != 0) continue; 
      winitalic = 0; winbold = 0;   /* get style from ATM.INI */
      if (co < eq) {
        if (strncmp(co+1, "BOLD", 4) == 0) {
          winbold = 1;
          co = co + 4;
        }
        if (strncmp(co+1, "ITALIC", 6) == 0) winitalic = 1;
      }   
/*    fail if mismatch on either bold or italic */
      if ((winbold && !nambold) || (!winbold && nambold)) continue;
      if ((winitalic && !namitalic) || (!winitalic && namitalic)) continue;
    }
    else continue;        /* not exact match *and* not ATM.INI */
/*    Now finally think we may have a match !  Try and locate PFB name */
/*    if (verboseflag)
      printf(" line (%d): %s", n, line); */ /* debugging show match */
    co = strchr(eq+1, ',');
    if (co != NULL) strcpy(pfbname, co+1);  /* easy way ! both PFM & PFB */
    else {
      strcpy(pfbname, eq+1);
/*  check whether PFM file name */ /* attempt to convert to PFB */
      if ((s = strstr(pfbname, ".pfm")) != NULL) {
        *(s+3) = 'b';         /* convert to PFB name */
      }
      if ((s = strstr(pfbname, "pfm")) != NULL) { /* splice out `pfm/' */
        if (*(s-1) == '\\' && *(s+3) == '\\') strcpy(s-1, s+3);
      }
    }
    break;                  /* no need to look further ! */
  }

/*  special case if it is marked printer resident ... */
  if (strstr(line, "*resident*") != NULL) return "nul"; /* 1994/Feb/17 */
  if (strstr(line, "*reside*") != NULL) return "nul";   /* 1995/May/8 */

  if (strcmp(pfbname, "") == 0) return NULL;  /* not found - bad luck ! */
  s = pfbname;
  while (*s > ' ' && *s != '\0') s++;     /* trim off line terminator */
  *s = '\0';

  if (mmflag) { /* do not show MM base font file name - 95/May/27 */
  }
  else if (verboseflag) {           /*  show font file name */
    sl = removepath(pfbname);
/*    if ((sl = strrchr(pfbname, '\\')) != NULL) sl++;
    else if ((sl = strrchr(pfbname, '/')) != NULL) sl++;
    else if ((sl = strrchr(pfbname, ':')) != NULL) sl++;
    else sl = pfbname; */
    if ((pe = strchr(sl, '.')) == NULL) pe = sl + strlen(sl);
    pe--;
    while (*pe == '_') pe--;        /* step back over _ */
    pe++;
    c = *pe;
    *pe = '\0';               /* temporarily terminate */
    showline(" ", 0);
    showline(sl, 0);
    if (pe != NULL) *pe = (char) c;     /* restore what was there */
  }
//  else putc('*', stdout);           /* note passage of font */
  else showline("*", 0);            /* note passage of font */
  return pfbname;               /* non-NULL for success */
}

/*  Now try and open the PFB file */
/*  Called from fopenfont --- split this off ! 1994/Aug/15 */
/*  First tries name exactly as given, then looks in PSFONTS path */
/*  writes file name back over second argument */
/*  Here we assume the PFB file name is known - search along path */

#ifdef SUBDIRSEARCH

/* maybe work directly in FileName instead of filename ? */
/* maybe don't look along PSFONTS path if pfbname contains \ or / ? */
/* changed to look in current directory first 98/Jul/20 */

FILE * find_pfb_file (char *pfbname, char *FileName)
{
  FILE *pfbfile;
  char filename[FNAMELEN]; /* used for PFB file name ? */

  strcpy(filename, pfbname);
  extension(filename, "pfb");

  if ((pfbfile = fopen(filename, "rb")) != NULL)
  {
    strcpy (FileName, filename);
    return pfbfile;
  }

  if (fontpath == NULL)
    return NULL;

  if (traceflag)
    showline(fontpath, 0);

  pfbfile = findandopen(filename, fontpath, NULL, "rb", currentfirst);

  if (pfbfile != NULL)
  {
    strcpy(FileName, filename);
    return pfbfile;
  }

  if (underscore(filename))
  {
    pfbfile = findandopen(filename, fontpath, NULL, "rb", currentfirst);

    if (pfbfile != NULL)
    {
      strcpy(FileName, filename);
      return pfbfile;
    }
  }

  return NULL;
}

#else

FILE *find_pfb_file (char *pfbname, char *FileName)
{
  FILE *pfbfile;
  char *searchpath;
  char filename[FNAMELEN];      /* used for PFB file name ? */
  char pfbfilename[FNAMELEN];

  strcpy(filename, pfbname);
  extension(filename, "pfb");
  if ((pfbfile = fopen(filename, "rb")) != NULL) {
    strcpy (FileName, filename);  /* 1994/Feb/10 */
    return pfbfile;       /* we found it */
  }
  else {              /* not found as given */
/*    printf("PFB %s not found as is\n", filename);*//* debug */
    strcpy(pfbfilename, filename);  /* remember basic name out of way */
    if (fontpath == NULL) return NULL;
    searchpath = fontpath;      /* use PSFONTS path */
    for(;;) {
      if ((searchpath =
        nextpathname(filename, searchpath)) == NULL) break;
      make_file_name(filename, pfbfilename);
/*      extension(filename, "pfb"); */
/*      printf("Trying %s\n", filename); */ /* debugging */
      if ((pfbfile = fopen(filename, "rb")) != NULL) {
        strcpy (FileName, filename);  /* 1994/Feb/10 */
        return pfbfile;       /* we found it */
      }
/*      underscore(filename); */    /* try with underscores */
      if (underscore(filename)) {   /* 95/May/28 */
/*      printf("Trying %s\n", filename); */ /* debugging */
        if ((pfbfile = fopen(filename, "rb")) != NULL) {
          strcpy (FileName, filename);  /* 1994/Feb/10 */
          return pfbfile;       /* we found it */
        }
      }
/*      remove_under(filename); */
      } /* end of loop for PFB file through directory PSFONTS */
  }
  return NULL;        /* PFB not found in any of the directories */
}
#endif

/* find the atmfonts.map file and get its full path */

int setup_atm_fonts_map (void)
{
  FILE *atmfile=NULL;
  char atmfilename[FNAMELEN];
#ifndef SUBDIRSEARCH
  char *searchpath;
#endif

  if (useatmfontsmap == 0) return 0;    /* already tried and failed */
//  if (*atmfontsmap != '\0') return 1;   /* already tried and succeeded */
  if (atmfontsmap != NULL) return 1;    /* already tried and succeeded */
  if (fontpath == NULL) return 0;
#ifdef SUBDIRSEARCH
/*  atmfile = findandopen("atmfonts.map", fontpath, NULL, "r", 0); */
  atmfile = findandopen("atmfonts.map", fontpath, atmfilename, "r", currentfirst);
#else
  searchpath = fontpath;    /* step through directories in PSFONTS */
  atmfile = NULL;       /* 1994/Aug/18 */
  for (;;) {
    if ((searchpath = nextpathname(atmfilename, searchpath)) == NULL) break;
    make_file_name(atmfilename, "atmfonts.map");      /* 1993/Dec/24 */
/*  try and open ATMFONTS.MAP file */
    if ((atmfile = fopen(atmfilename, "r")) != NULL) break;
  } /* end of loop for ATMFONTS.MAP through directory path in PSFONTS */
#endif
  if (traceflag) {
    sprintf(logline, " atmfonts.map: `%s' ", atmfilename);  /* 98/Jan/9 */
    showline(logline, 0);
  }
  if (atmfile != NULL) fclose(atmfile);
  if (*atmfilename == '\0') useatmfontsmap = 0; /* failed, don't try again */
  else {
/*    atmfontsmap = _strdup(atmfilename);
    if (atmfontsmap == NULL) {
      fputs("Unable to allocate memory\n", errout);
      checkexit(1);
    } */
    atmfontsmap = zstrdup(atmfilename);
  }
  if (atmfontsmap == NULL) return 0;
  else return (*atmfontsmap != '\0');
}

/* This is not normally used for fonts requested by DVI file */
/* This is used when meeting %%IncludeResoure: ... DSC */
/* *and* this is used to find MM base font PFB file --- FindMMBaseFile */

/* Note: references to ATM.INI  won't work in NT since there is none */
/* Note: references to ATMREG.ATM work with ATM 4.0 only since 3.0 doesn't */

/* (1) This first looks for entries in font substitution file */
/* which give mapping between font file names and PS FontNames */
/* ignores entries other than *epsf* and *reside* */
/* (2) If this fails it looks for `ATMFONTS.MAP' in directory on PSFONTS list */
/* PFB file names in `ATMFONTS.MAP' may be unqualified */
/* (3) If this fails it looks for ATMREG.ATM (ATM 4.0 feature) */
/* it then reads this and tries to find an entry for the font */
/* (4) If this fails then it looks for `ATM.INI' in Windows directory */
/* and uses that *heuristically* (Face Name listed, not Font Name) */
/* Tries to return constructed FileName in second argument */
/* Returns NULL if font file not found */

/* FILE *fopenfont (char *FontName, char *FileName) */
FILE *fopenfont (char *FontName, char *FileName, int mmflag)
{
  FILE *atmfile=NULL;
  FILE *pfbfile=NULL;
  char pfbfilename[FNAMELEN];     /* use for full PFB file name */
/*  char *windir; */
  char *s;
  int n;

  if (*FontName == '\0') return NULL;   /* sanity check 97/June/1 */

  if (traceflag) {
    sprintf(logline, " fopenfont %s mmflag %d\n", FontName, mmflag);
    showline(logline, 0); // debugging only
  }

  if (traceflag) showline(" STAGE I", 1);   /* debugging 95/May/25 */
/*  Stage I Stage I Stage I Stage I Stage I Stage I Stage I Stage I */
/*  following 1994/Aug/15 use *epsf* tagged lines in font sub file */
/*  revised 1998/Jan/9 to use all lines in sub file ... */
/*  uses pfbfilename for temporary place to store font file name */
  
  if ((n = FindFileName(FontName, pfbfilename)) != 0) {
    if (n < 0) {
      strcpy(FileName, pfbfilename);
      return NULL;
    }     /* special case - font is resident ??? 98/Jan/9 */
    pfbfile = find_pfb_file(pfbfilename, FileName); /* look for PFB file */
    return pfbfile;     /* NULL if not found */
  }

  if (traceflag) showline(" STAGE II", 1);  /* debugging 95/May/25 */
/*  Stage II Stage II Stage II Stage II Stage II Stage II Stage II Stage II */
/*  Try and use ATMFONTS.MAP */
  if (useatmfontsmap) setup_atm_fonts_map();

  if ((atmfile = fopen(atmfontsmap, "r")) != NULL) {
/*  try and find line `FontName=<pfb-file>' - PFB name return in pfbfilename */
    s = fopenfont_sub(atmfile, FontName, pfbfilename, 0, mmflag);
    fclose(atmfile);          /* close ATMFONTS.MAP again */
/*    moved inside s != NULL case 95/Nov/14 */
/*    if (strcmp(s, "nul") == 0)  */    /* special case 1994/Feb/17 */
/*      return NULL;          *//* *resident* ??? */
    if (s != NULL)  {         /* found in this file ? */
      if (strcmp(s, "nul") == 0)    /* special case 1994/Feb/17 */
        return NULL;        /* *resident* ??? */
      pfbfile = find_pfb_file(pfbfilename, FileName);
      return pfbfile;         /* NULL if not found */
    }     /* end of pfb file name found in ATMFONTS.MAP */
  }

  if (traceflag) showline(" STAGE III", 1);   /* debugging 95/May/25 */
/*  Stage III Stage III Stage III Stage III Stage III Stage III Stage III */
/*  Try and find and read ATMREG.ATM */
//  if (useatmreg) SetupATMReg();   /* New 98/Jan/9 */

  if (LookupATMReg(FontName, pfbfilename) == 0) {
    if (traceflag) {
      sprintf(logline, "File: %s ", pfbfilename);
      showline(logline, 1);
    }
    if ((pfbfile = fopen(pfbfilename, "rb")) != NULL) {
      strcpy (FileName, pfbfilename); /* 1994/Feb/10 */
      return pfbfile;       /* we found it */
    }
    else {
      sprintf(logline, " ERROR: Can't open %s ", pfbfilename);
      showline(logline, 1);
//      perror(pfbfilename);    /* debugging */
      perrormod(pfbfilename);   /* debugging */
      return NULL;        /* PFB not found */
    }
  }

  if (traceflag) showline(" STAGE IV", 1);  /* debugging 95/May/25 */
/*  Stage IV Stage IV Stage IV Stage IV Stage IV Stage IV Stage IV */
/*  sigh, failed to find ATMFONTS.MAP in directories on PSFONTS */
/*  *OR* failed to find the FontName there that we are looking for */

/*  following stuff simplified 95/May/25 */
/*  else _searchenv("ATM.INI", "PATH", atmini); */

#ifndef _WINDOWS
  if (useatmini) setupatmini();         /* redundant ? */
#endif

/*  atmini now contains full file name for atm.ini, with path */

/*  if ((atmfile = fopen(atmini, "r")) == NULL) return NULL; */
  if ((atmfile = fopen(atmini, "r")) != NULL) { /* 1994/Aug/15 */

/*    try and find line `Facename,BOLDITALIC=<pfm-file>,<pfb-file> */
    s = fopenfont_sub(atmfile, FontName, pfbfilename, 1, mmflag);
    fclose(atmfile);          /* close ATM.INI again */
    if (s != NULL)  {         /* found in this file ? */
/*    try and open the PFB file */ /* don't play with PSFONTS for this */
      if ((pfbfile = fopen(pfbfilename, "rb")) != NULL) {
        strcpy (FileName, pfbfilename); /* 1994/Feb/10 */
        return pfbfile;       /* we found it */
      }
      else return NULL;       /* PFB not found */
    }
  }   /* end of ATM.INI opened OK case */


/*  Desperation: Try filename = fontname in fontpath */
/*  Works for CM, AMS, EM, extra LaTeX, MathTime */

  strcpy(pfbfilename, FontName);
  pfbfile = find_pfb_file(pfbfilename, FileName); /* look for PFB file */
  if (pfbfile != NULL) return pfbfile;

  return NULL;          /* entry not found in ATM.INI either */
}

/* Could conceivably read all PFB files and extract FontName ... */
/* Could have table for Base13 + LucidaBright ??? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Check for EPSF files - with MetaFile or TIFF bitmap image */
/* Returns 0 if plain ASCII format  - or length of PS section if not */
/* Also advances file to start of PostScript part */

long checkpcform (FILE *special, char *fname)
{
  unsigned long n, m;
/*  unsigned long i; */
  int c, k;

  psoffset = pslength = 0;      /* in case we pop out 96/Sep/12 */
  c = getc(special); (void) ungetc(c, special);
  if (c == '%') return 0;       /* looks like %!PS-Adobe ... */
  if (c < 128) return 0;        /* looks like plain ASCII */
  else if (getc(special) == 'E' + 128 &&
       getc(special) == 'P' + 128 &&
       getc(special) == 'S' + 128 &&
       getc(special) == 'F' + 128) {
    n = 0;
    for (k=0; k < 4; k++) { /* read PS code start position */
      n = n >> 8;
      c = getc(special);
      n = n | ((unsigned long) c) << 24;  /* check this */
    }
    m = 0;
    for (k=0; k < 4; k++) { /* read PS code length */
      m = m >> 8;
      c = getc(special);
      m = m | ((unsigned long) c) << 24; /* check this */
    }
/*    should we salt away n and m globally for psoffset and pslength ? */
    psoffset = n; pslength = m;       /* 96/Sep/12 */
    if (traceflag) {
      sprintf(logline, " PS start %ld PS length %ld", n, m);
      showline(logline, 1);
    }
/*    read up to PS part: */
/*    for (i=4+4+4; i < n; i++)  (void) getc(special); */
/*    position at start of PS section */
    fseek (special, n, SEEK_SET); /* use fseek instead 95/July/12 */
    return (long) m;      /* return pslength */
  }
  else {
    sprintf(logline, " File %s not ASCII PS file\n", fname);
    showline(logline, 1);
    errcount(0);
    return -1;
  }
/*  for weird file format, do we need to flush stuff at end also ? */
/*  go only up to %%EOF (counting BeginDocument and EndDocument) ? */
}

/****************************************************************************
*                                                                           *
*   The EPSF header has the following structure:                            *
*                                                                           *
* 0-3 the first four bytes are the letters EPSF with the meta-bit on -      *
*   that is, hex C5D0D3C6.                                                  *
* 4-7 the next four are the byte position of start of the PS section        *
* 8-11  The next four are the length of the PostScript part of file;        *
* 12-15 The next four are the byte position of start of MetaFile version;   *
* 16-19 The next four are the length of the MetaFile version;               *
* 20-23 The next four are the byte position of start of TIFF version;       *
* 24-27 The next four are the length of the TIFF version;                   *
* 28-29 The next two bytes are the header checksum or hex FFFF              *
*       that is, two bytes that are all ones (meta-control-? = 255)         *
*                                                                           *
*   IN EACH CASE THE LOW ORDER BYTE IS GIVEN FIRST                          *
*                                                                           *
*   Either the MetaFile length or the TIFF length or both are zero          *
*                                                                           *
*   If there is no MetaFile or Tiff version, the PS code starts at 30       *
*                                                                           *
* The file produced as plain PS by Designer, instead starts and ends        *
*       on control-D.  The first control-D is followed by %<space>,         *
*       which is non-standard, to say the least.                            *
*                                                                           *
****************************************************************************/

/* FreeHand produces EPS file only %%EndDocument, no %%BeginDocument ! */
/* Canvas produces EPS file that end with %%EndDocument: */

void warnnesting (int nesting)
{
  if (nesting != 0) {
    sprintf(logline, " nesting level %d at end", nesting);
    showline(logline, 1);
  }
}

/* following used in dvipsone.c for prologs/headers */
/* and also for EPS when -*s on command line */
/* flush blank lines ? flush comment lines ? <- NOT SAFE */

int copyepssimple (FILE *output, FILE *special) /* 1993/Jan/24 */
{
  int c;
  while ((c = getc(special)) != EOF) {
    if (c >= 128) {
      sprintf(logline, " char %d in EPS", c); /* 1995/July/25 */
      showline(logline, 1);
      errcount(0);
      return -1;          /*  avoid junk in trailer ??? */
    }
    else if (c < ' ') {     /* avoid most control characters ??? */
      if (c == '\n' || c == '\r' || c == '\t' ||
        c == '\b' || c == '\f') {
//        putc(c, output);
        PSputc(c, output);
      }
      else {
        sprintf(logline, " C-%c", c+64);
//        showline(logline, 1);
        showline(logline, 0);
        if (bPassControls != 0) {
//          putc(c, output);
          PSputc(c, output);
        }
      }
      if (bAbort) abortjob(); /* only at end of line for speed */
      if (abortflag) return -1;
    }
    else {
//      putc(c, output);
      PSputc(c, output);
    }
  }
  return 0;
}

void reincludefont (FILE *output, char *FontName)
{
/*  fputs(line, output); */     /* line has been changed */
//  fputs("%%", output);
  PSputs("%%", output);
//  fputs("IncludeResource: font", output);
//  PSputs("IncludeFont", output);        /* 12 characters */
  PSputs("IncludeResource: font", output);  /* 21 characters */
  PSputc(' ', output);
//  fputs(FontName, output);
  PSputs(FontName, output);
//  putc('\n', output); 
  PSputc('\n', output); 
}

/* Separated out 1995/July/25 */

/*  Try and deal with %%BeginBinary 1994/Sep/15 */
/*  Note: may be problems here with C-J => C-M C-J ? */
/*  Unless -R on command line sets retplusnew - forces fopen output rb mode */
/*  Should perhaps assume first line following is ASCII and read as such ? */
/*  Should perhaps scan forward to %%EndBinary at the end ? */

/*  Should really implement more modern %%BeginData also ? */

int CopyBinary (FILE *output, FILE *special, long nbytes)
{
  long kk;
  int c=0;
  
/*  printf(" %ld line: %s", nbytes, line); */ /* debug */
  if (verboseflag) showline(" Binary", 0);

/*  could try and read first line here --- something like beginbinary */
/*  and then fidged with line terminator in case it is wrong */
  for (kk=0; kk < nbytes; kk++) {
/*    putc(getc(special), output); */
    c = getc(special);
    if (c == EOF) {
      showline(" Premature EOF", 1); /* 95/July/26 */
      errcount(0);
      return -1;
    }
//    putc(c, output);
    PSputc(c, output);
  }
/*  end line unless it just ended */
  if (c != '\n') {
//    putc('\n', output); 
    PSputc('\n', output); 
  }
/*  c = getc(special);
  if (c != '\n') ungetc(c, special); */ /* 95/July/26 */
/*  hopefully next line is %%EndBinary ... */
/*  could try and search forward until find %%EndBinary line ? */
/*  need to update pslength by nbytes afterwards */
  return 0;
}

/* Skip %%BeginPreview: .... to %%EndPreview */

long SkipEPSI (FILE *special)    /* 97/June/5 */
{
  long nbytes=0;
  int k;
  for (;;) {
    if ((k = getline(special, line)) == 0) break;
    nbytes += k;
    if (*line != '%') break;  /* skip out if not comment */
    if (*(line+1) != '%') continue; /* look for double %% comments */
/*    if (strncmp(line+2, "EndPreview", 10) == 0) break; */
    break;     /* actually just skip out on any %% comment ... */
  }
  return nbytes;
}

//  Flush stupid HP printer junk at head of file
//  %-12345X@PJL JOB
//  @PJL SET RESOLUTION = 600
//  @PJL ENTER LANGUAGE = POSTSCRIPT 

//  %-12345X@PJL EOJ
//  %-12345X

int skiphpjunkathead (FILE *special, int flag)  // 99/Oct/14
{
  int c;
  c = getc(special);      // escape character (27)
  c = getc(special);      // %
  while (c != '@' && c >= 32) c = getc(special);
  for(;;) {
//    if (c == '%') break;
    if (c != '@') break;
    while (c >= 32) c = getc(special);
    while (c > 0 && c < 32) c = getc(special);
    ungetc(c, special);
  }
  if (flag) showline(" HP", 0);
  return 1;
}

/* The following attempts to allow long lines other than in comments */
/* This may be more complex than really called for... */
/* Result of paranoia about running into junk at end of file */
/* The PS specification says no line should be longer than 255, BUT ... */

/* generalized to read PS code in middle of EPS file with TIFF header */
/* 1994/Feb/23 - means keeping track of byte count */
/* => modification of getline to return bytes read, not bytes assembled */

/* void copyepsfilesub(FILE *output, FILE *special) { */ /* 94/Feb/23 */

/* called only by copyepsfileaux --- copyepsfilesub does not close file */

void copyepsfilesub (FILE *output, FILE *special, long pslength)
{
  int k, nesting=0;
  int c=0;              /* keep down complaints */
  int hpjunkflag=0;         // set if escape (27) at start
  FILE *fontfile;
  char *s, *t;
  long nbytes;
  char FontName[FNAMELEN]="";
  char FileName[FNAMELEN];
  int plainpsflag=0;

  if (! bPassEPSF) {        /* 1994/Mar/9 */
    flush_special(special);
    return;
  }

/*  This now handles both plain ASCII and EPSF with TIFF preview 94/Feb/23 */

  if (pslength == 0) {
    pslength = LINFINITY; /* ASCII file humungous amount */
    plainpsflag = 1;    /* plain ASCII file */
  }

/*  Definitely should NOT allow C-C, C-D, C-Q, C-S, C-T through --- */
/*  and files often contain things like C-D, C-Z (and FreeHand bug: C-E) */
  
/*  if (bSmartCopyFlag == 0) copyepssimple(output, specialf);*//* 93/Jan/24 */
/*  `bSmartCopyFlag'   try and be a bit more sophisticated here... */
/*  else  */

  c = getc(special);
  ungetc(c, special);
  if (c == 27)          /* HP Printer Escape 99/Oct/14 */
    hpjunkflag = skiphpjunkathead (special, 1);

  for (;;) {            /* tries to read a line at a time */
    if (pslength <= 0)      /* break out if hit end of PS section */
      break;          /* 96/Aug/21 */

    c = getc(special);      /* check out first character in line */
    pslength--;
    if (c == EOF) break;    /* hit end of file */
    if (hpjunkflag && c == 27) break; // HP escape 99/Oct/14
    if (c >= 128) {       /* hit junk */
      sprintf(logline, " char %d in EPS", c);
      s = logline + strlen(logline);
      if (c > (128 + 32)) sprintf(s, " (M-%c)", c-128);
      showline(logline, 1);
      errcount(0);
/*      if (nesting != 0) warnnesting(nesting); */
/*      return; */        /* avoid junk in trailer ??? */
      break;
    }

    if (! bKeepTermination) {   /* 98/Feb/8 */
      if (c == '\r') {    /* flush C-R C-J sequences */
        c = getc(special);
        pslength--;       /* 94/Feb/13 */
        if (c != '\n') {    /* put it back if not \n */
          (void) ungetc(c, special);    /* 96/Aug/21 */
          pslength++;           /* 96/Aug/21 */
/*          putc('\r', output); */      /* put out the '\r' */
          if (bConvertReturn) c = '\n'; /* 96/Aug/22 */
          else c = '\r';
/*          if (traceflag) putc('*', stdout); */ /* DEBUGGING */
        }
      }

    }     /* end of bKeepTermination == 0 */

    if (c == '\n') {    /* check overrun lines (starts with terminator) */
/*      continue; */    /* ignore blank line */
      if (pslength <= 0) break; 
    }
    else if (c == '\r') { /* check overrun lines (starts with terminator) */
/*      continue; */    /* ignore blank line */
      if (pslength <= 0) break;
    }

    if (c == '%') {         /* see if comment (% or %%) */
      c = getc(special);
      pslength--;         /* 94/Feb/13 */
      if (c == EOF) {
/*        if (nesting != 0) warnnesting(nesting); */
/*        return; */
        break;
      }
      if (c == '%') {         /* see if %% comment */
/*        deal with %% PS document structuring comments */
        if ((k = getline(special, line)) == 0) {
/*          return; */          /* return if hit EOF */
          c = EOF;
          break;
        }
/*        turn terminating return into newline */
        if (bConvertReturn) {
          if (*(line+k-1) == '\r') {  /* modify what getline gave */
            *(line+k-1) = '\n';
/*            if (traceflag) putc('@', stdout); */
          }
        }
        pslength = pslength - k;  /* adjust bytes left to look at */
/*        NOTE: line length limit here ! */
/* following added 93/Nov/15 */
/* ignore (atend) --- and ignore if only white space present */
/* correct solution: note this stuff in prescan */
/* then *** somehow find font file name from PS FontName *** */
/* and load PFB file in header - avoid conflict with other font use */
/* flush the following now that we support %%Include */
/* NOTE: DocumentNeeded MUST appear if %%Include appears */
/*        if (strncmp(line, "DocumentNeeded", 14) == 0 ||
          strncmp(line, "DocumentFont", 12) == 0) {
          if (strstr(line, "(atend)") == NULL &&
            strstr(line, "(at end)") == NULL) { 
            if ((s = strchr(line, ':')) != NULL) {
            s++;
              while ((c = *s++) != '\0') if (c > ' ') break;
              if (c != '\0')
                fprintf(errout, "\nWARNING: %s", line);
            }
          }
        } */  /* flush this, use %%IncludeFont instead */
/* following added 1993/Dec/21 */
/* deal with %%IncludeResource: font and %%IncludeFont: comments */
        if (bAllowInclude &&
          (strncmp(line, "IncludeResource: font", 21) == 0 ||
            strncmp(line, "IncludeFont:", 12) == 0)) {
          if (strncmp(line+7, "Font:", 5) == 0) s = line + 12;
          else s = line + 21;
/* now advance over white space */
          while (*s <= ' ' && *s != '\0') s++;  
          t = s;
/* advance up to white space */
          while (*t > ' ') t++;
          *t = '\0';      /* terminate it */
/*          strncpy(FontName, s, sizeof(FontName)); */
          if (strlen(s) < sizeof(FontName)) /* FNAMELEN */
            strcpy(FontName, s);
/* maybe instead, show font file name later for debugging purposes ? */
/* now figure out what the *font file* is ! */
/* go off and include font called for right at this point */
/*          if ((fontfile = fopenfont(FontName)) != NULL)  */
          if ((fontfile = fopenfont(FontName, FileName, 0)) != NULL) {
            lowercase(FileName, FileName);  /* 97/June/5 */
/* then decompress from PFA to PFB format */ /* unless it is resident */
            if (! ResidentFont(FileName)) { /* 1994/Feb/10 */
              decompress_font(output, fontfile, FontName);
            }
            else {            /* 1994/Feb/10 */
/*      insert the %%Include comment again if unable to service ??? */
              reincludefont(output, FontName);
              if (verboseflag) {
                sprintf(logline, " %s", FontName);  /* 1994/Aug/17 */
                strcat(logline, " resident");
                showline(logline, 0);
              }
            }
            fclose (fontfile);
          }  /* end of fopen succeeds */
          else {    /* failed to find font file --- NULL*/
            if (! ResidentFont(FileName)) {   /* 1994/Feb/23 */
              sprintf(logline, " %s", FontName);
              strcat(logline, " not found");
              showline(logline, 1);
              errcount(0);
            }
            else {
/*              reincludefont (output, FontName);  */
              if (verboseflag) {
                sprintf(logline, " %s", FontName);  /* 1994/Aug/17 */
                strcat(logline, " resident");
                showline(logline, 0);
              }
            }
/*       insert the %%Include comment again if unable to service ??? */
            reincludefont (output, FontName); 
          } /* end of failed to find font file */
        }   /* end of allow  %%Include... case */
        else if (strncmp(line, "BeginPreview", 12) == 0) {
          nbytes = SkipEPSI(special); /* skip over EPSI preview */
          pslength = pslength - nbytes;
        }
//        else if (! stripcomment) {  /* moved down here */
        else {            /* pass %% in EPS 2000 July 12th */
          PSputs("%%", output);
          PSputs(line, output);
        }           /* just copy across the %%Include */

        if (strncmp(line, "BeginBinary", 11) == 0) {
          if (sscanf(line+12, "%ld", &nbytes) == 1) {
            (void) CopyBinary(output, special, nbytes);
            pslength = pslength - nbytes;
          }
          else {
/*            putc('%', errout); */   /* reconstruct %% */
/*            putc('%', errout); */
            strcpy(logline, "%%");
            strcat(logline, line);
            showline(logline, 1);
          }
        }               /* end of %%BeginBinary */
/*        figure nesting level & see if end of included document */
/*        this prevents wading into junk in trailer */
        if (strncmp(line, "BeginDocument", 13) == 0) nesting++;
        else if (strncmp(line, "EndDocument", 10) == 0) {
          if (strchr(line, ':') != NULL) { /* 97/Dec/26 */
//            showline(" WARNING: bad DSC ", 1);
//            showline(line, 1);
            sprintf(logline, " WARNING: bad DSC %s", line);
            showline(line, 0);  // 2000 June 8
            *line = '\0'; /* flush it 97/Dec/26 */
          }
          else nesting--;
        }
        else if (nesting <= 0 && strncmp(line, "EOF", 3) == 0) {
/*          return; */
          break;
        } 
      }     /* end of DSC %% comment case */
      else {      /* ordinary comment - skip to end of line */
        if (passcomments) {   /* 1992/March/18 */
//          putc('%', output);
          PSputc('%', output);
/*          for(;;)  */
/*            if (c == '\n' || c == '\r' || c == EOF || c == 0)
              break; */     /* 95/May/8 deal with % \r */
/*          while (c != '\n' && c != '\r' && c != EOF && c != 0) */
          while (c != '\n' && c != '\r' && c != EOF) { /* 95/Dec/19*/
/*            putc(c, output); */ /* was just this --- 1994/May/19 */
/* Following added to deal with stupid Mathematica files with %^D */
            if (c < ' ') {    /* flush bad control characters */
              if (c == '\t' || c == '\b' || c == '\f') {
//                  putc(c, output);
                PSputc(c, output);
              }
              else {
                sprintf(logline, " C-%c", c+64);
//                showline(logline, 1);
                showline(logline, 0);
                if (bPassControls != 0) {
//                  putc(c, output);
                  PSputc(c, output);
                }
              }
            }
            else {
//              putc(c, output);
              PSputc(c, output);
            }
            c = getc(special);
            pslength--;       /* 94/Feb/13 */
/*            if (c == '\n' || c == '\r' || c == EOF) break; */
/*            if (c == '\n' || c == '\r' || c == EOF || c == 0)
              break; */   /* now redundant ... */
          }
          if (bKeepTermination == 0) {    /* 98/Feb/8 */
            if (c == '\r') {  /* absorb C-M C-J sequence */
              c = getc(special);
              pslength--;
              if (c != '\n') {
                (void) ungetc(c, special);
                pslength++;
                if (bConvertReturn) c = '\n'; /* 96/Aug/22 */
                else c = '\r';
/*                if (traceflag) putc('#', stdout); */
              }
            }
          }
          if (c != EOF) {
//            putc(c, output);
            PSputc(c, output);
          }
        }           /* end of passcomments != 0 */
        else {          /* flush comment line */
/*          while (c != '\n' && c != '\r' && c != EOF && c != 0) */
          while (c != '\n' && c != '\r' && c != EOF) { /*95/Dec/19*/
            c = getc(special);
            pslength--;       /* 94/Feb/13 */
          }
          if (bKeepTermination == 0) {    /* 98/Feb/8 */
            if (c == '\r') {  /* absorb C-M C-J sequence */
              c = getc(special);
              pslength--;
              if (c != '\n') {
                (void) ungetc(c, special);
                pslength++;
                if (bConvertReturn) c = '\n'; /* 96/Aug/22 */
                else c = '\r';
/*                if (traceflag) putc('$', stdout); */
              }
            }
          }
        }

/*        if (c == EOF || c == 0) */  /* null same as EOF ??? */
        if (c == EOF) {             /* 95/Dec/19 */
/*          if (nesting != 0) warnnesting(nesting); */
/*          return; */
          break;
        }
      }     /* end of ordinary % comment case */
    }       /* end of general % or %% comment case */

    else {    /* normal text line - not a % or %% comment - copy it */
      if (c == EOF) {
/*        if (nesting != 0) warnnesting(nesting); */
/*        return; */
        break;
      }
/*      while (c != '\n' && c != '\r' && c != EOF && c != 0) */
      while (c != '\n' && c != '\r' && c != EOF) { /* 95/Dec/19 */
        if (c < ' ') {    /* flush bad control characters */
          if (c == '\t' || c == '\b' || c == '\f') {
//            putc(c, output);
            PSputc(c, output);
          }
          else {
            sprintf(logline, " C-%c", c+64);
//            showline(logline, 1);
            showline(logline, 0);
            if (bPassControls) {
//              putc(c, output);  /* ??? */
              PSputc(c, output);  /* ??? */
            }
          }
        }
        else {
//          putc(c, output);
          PSputc(c, output);
        }
        c = getc(special);
        pslength--;       /* 94/Feb/13 */
      }
/*      broke out - reached end of line or end of file */
/*      c here is \n or \r or EOF */

      if (c == EOF) {       /* 95/Dec/19 */
/*        if (nesting != 0) warnnesting(nesting); */
/*        return; */
        break;
      }
      else if (c == '\n') ;
      else if (bKeepTermination == 0) {
        if (c == '\r') {    /* turn C-M C-J into C-J */
          c = getc(special);
          pslength--;
          if (c != '\n') {
            (void) ungetc(c, special);
            pslength++;
            if (bConvertReturn) c = '\n'; /* 96/Aug/22 */
            else c = '\r';
          }
        }
      }
//      putc(c, output);      /* \n or \r */
      PSputc(c, output);      /* \n or \r */
    } /* end of normal text line case */
    if (bAbort) abortjob(); /* 1992/Nov/24 */
/*    ready for the next line ...*/     
  } /* end of for(;;) loop */

/*  moved down here 96/Aug/21 */
  if (pslength < 0) {
    if (verboseflag) {
      sprintf(logline, " PS part overrun %ld", -pslength);
//      showline(logline, 1);
      showline(logline, 0); // make less obnoxious 2000 June 8
    }
  }
  if (plainpsflag == 0 && pslength > 0 && c == EOF) {
    if (verboseflag) {
      sprintf(logline, " PS part early EOF %ld", pslength);
//      showline(logline, 1);
      showline(logline, 0); // make less obnoxious 2000 June 8
    }
  }
  if (nesting) warnnesting(nesting);
  return;
}
  
/* Use in EPSF file, when length of PostScript part is specified */
/* Is there any problem here because of C-M and C-J ??? */
/* Input now is `rb' mode, output possibly in `w' (or `wb' mode) */

int copyepsfilesimple (FILE *output, FILE *special, long pslength)
{
  int c;

  if (! bPassEPSF) {        /* 1994/Mar/9 */
    flush_special(special);
    return 0;
  }

  if (pslength == 0) pslength = LINFINITY; /* ASCII file humungous amount */

  while (pslength-- > 0 && (c = getc(special)) != EOF) {
//    putc(c, output);
    PSputc(c, output);
/*    if (bAbort) abortjob(); */
  }
  return 0;
}            /* worry about character count being inaccurate ? */

/*  checks whether given filename has more than 8 characters 97/Nov/28 */
/*  do similar hack for extension longer than 3 characters ? */
/*  trunactes file name is long */

int islongname (char *name)
{
  char *s, *t;
  if ((t = strrchr(name, '.')) == NULL) t = name + strlen(name);
  s = t;
  while (s > name && *s != '\\' && *s != '/' && *s != ':') s--;
  if (s > name) s++;
  if ((t - s) > 8) {
    strcpy(s+8, t);     /* truncate it */
/*    winerror(s);  */    /* debugging */
    return 1;
  }
  else return 0;
}

/* Inserted EPS file is searched for as follows: */ /* OLD VERSION */
/* If no epspath is specified, then the file name is used as given */
/* If an epspath is given, directory part of the file name is stripped off */
/* - then each pathname in the search path is tried in turn */
/* if file is not found, it is next looked for in the dvi file's directory */
/* finally an attempt is made to find it in the current directory */

/* Inserted EPS file is searched for as follows: */ /* NEW VERSION */
/* If fully qualified, full filename is tried first */
/* If an epspath is given, directory part of the file name is stripped off */
/* - then each pathname in the search path is tried in turn */
/* If file is not found, it is next looked for in the dvi file's directory */
/* Finally an attempt is made to find it in the current directory */

/* shouldn't it look in DVI file directory first ? if unqualified ? */

/* copy the EPS file to be included or overlayed */
/* incflag is non-zero if "included", else "overlaid" */
/* needshift > 0 if need to shift by (-xll, -yll) */  /* Textures ? */
/* needshift = 0 if no shift needed */          /* DVI2PS ? */
/* needshift < 0 if need to shift by (-xll, -yur) */  /* DVIALW ? */

/* FINDEPSFILE:  Try to open the file to be included (or overlayed) */

/* FILE *findepsfile(char *name, int warnflag) { */ /* 1993/Oct/13 */
FILE *findepsfile (char *name, char *ext, int warnflag, int readflag)
{
  FILE *special=NULL;
  char *s;
  char *epsname="";     /* may be used before defined ??? */
  int foundfile = 0;
  char epsfilename[FNAMELEN];     /* NEW */
#ifndef SUBDIRSEARCH
  char *searchpath;
#endif
  int pass = 0;           /* 99/June/26 */

/* tryagain: */             /* 97/Nov/28 */
  
  while (pass < 2) {    /* may do twice, once with longname shortened */

    strcpy(epsfilename, name);      /*  filename as given */
/*    extension(epsfilename, "eps"); */ /*  add extension if none there */
    extension(epsfilename, ext);    /*  add extension if none there */
    epsname = removepath(epsfilename);  /*  strip path from eps file name */

    if (traceflag) {
      sprintf(logline, " name `%s' ext `%s' epsfilename `%s' epsname `%s'\n",
          name, ext, epsfilename, epsname); /* DEBUGGING ONLY 94/Nov/10 */
      showline(logline, 0);
    }

/*    printf("epsfilename  %s epsname = %s\n", epsfilename, epsname);  */

/*    maybe consider only fully qualified if it contains `:' ??? */
/*    if fully qualified name, try that first */
    if (strchr(epsfilename, '\\') != NULL ||
      strchr(epsfilename, '/') != NULL ||
      strchr(epsfilename, ':') != NULL) {   /* fully qualified name */
      strcpy(filename, epsfilename);      /* try using as is ! */
      if ((special = fopen(filename, "rb")) != NULL) foundfile = 1; 
    }
    else {  /* not fully qualified try anyway 99/July/3 */
      strcpy(filename, epsfilename);      /* try using as is ! */
      if ((special = fopen(filename, "rb")) != NULL) foundfile = 1; 
    }

/*    if not successful, try each path in EPSPATH in turn */

    if (foundfile == 0 && strcmp(epspath, "") != 0) {
#ifdef SUBDIRSEARCH
/*      if ((special = findandopen(epsfilename, */
      if ((special =
        findandopen(epsname, epspath, filename, "rb", currentfirst)) != NULL) {
        lowercase(filename, filename);
        foundfile = 1;
      }
#else
      searchpath = epspath;
      for(;;) {
        if ((searchpath=nextpathname(filename, searchpath)) == NULL) {
          foundfile = 0; break;
        }
/*        printf("NEXTPATH %s %d", filename, strlen(filename)); */
        s = filename + strlen(filename) - 1;
        if (*s != '\\' && *s != '/') strcat(filename, "\\"); 
        strcat(filename, epsname);
/*        extension(epsfilename, "eps"); */
        if ((special = fopen(filename, "rb")) != NULL) { /* "r" ? */
          foundfile = 1; break;
        }
      }
#endif
    }

    if (foundfile == 0) {       /* if not found on search path */
      if (dvipath != NULL) 
        strcpy(filename, dvipath);  /* try in directory of dvi file */
      else strcpy(filename, "");
      if (*filename != '\0') {
        s = filename + strlen(filename) - 1;
        if (*s != '\\' && *s != '/') strcat(filename, "\\");
      }
      strcat(filename, epsfilename);    /* 1992/May/05 */
      if ((special = fopen(filename, "rb")) != NULL)  foundfile = 1; 
    }

    if (foundfile == 0 && strcmp(epsfilename, epsname) != 0) {
      if (dvipath != NULL) 
        strcpy(filename, dvipath);  /* try in directory of dvi file */
      else strcpy(filename, "");
      if (*filename != '\0') {
        s = filename + strlen(filename) - 1;
        if (*s != '\\' && *s != '/') strcat(filename, "\\");
      }
      strcat(filename, epsname);      /* try qualified name */
      if ((special = fopen(filename, "rb")) != NULL) foundfile = 1; 
    } 

    if (foundfile == 0) {
      strcpy(filename, epsfilename);  /* try in current directory */
      if ((special = fopen(filename, "rb")) != NULL) foundfile = 1;
    }
    
    if (foundfile == 0 && strcmp(epsfilename, epsname) != 0) {
      strcpy(filename, epsname);    /* try in current directory */
      if ((special = fopen(filename, "rb")) != NULL)  foundfile = 1;
    }
    
/*    if (foundfile == 0) {   
      if (islongname(name) != 0)  
        goto tryagain;  
        ) } */ /* try with shortened name - 97/Nov/28 */
    
    if (foundfile != 0) break;        /* break out if found */
    else if (islongname(name) == 0) break;  /* not long name */
    
    pass++;     /*  try with shortened name - 97/Nov/28 */
  } /* end of while (pass < 2) loop */

  if (foundfile == 0) {
    if (warnflag) {
      sprintf(logline, " Can't find %s", epsname);
//      showline(logline, 1);
      showline(logline, 0); 
/*      file to be inserted */  /*  perror(epsname); */
      errcount(0);
    }
    return NULL;            /* failed */
  }

/*  if (verboseflag) */
  if (verboseflag && readflag == 0) {
    sprintf(logline, " %s", filename);        /* announce it */
    showline(logline, 0);
  }
/*  if ((ret = _stat(filename, &statbuf)) != 0) {
    sprintf(logline, "ERROR: Unable to obtain info on %s\n", filename);
  } */

/*  printf("- FILENAME %s - EPSNAME %s - ", filename, epsname); */
  return special;
}

void copyepsfileaux (FILE *output, FILE *special, char *fname)
{
  long pslength;        /* zero or length of PS section in EPSF file */
  int ret;
  char *s;
  
  PSputc('\n', output);       // always start on new line ?
//  Put % FileDate and % FileSize comments *before* %%BeginDocument
//  to get around Distiller bug with negative logical page numbers 
  if (! stripcomment) {
    if ((ret = _fstat(_fileno(special), &statbuf)) == 0) {
      s = ctime(&statbuf.st_mtime);
      if (s != NULL) {
        lcivilize(s);
        sprintf(logline, "%% FileDate: %s\n", s);
        PSputs(logline, output);
      }
      sprintf(logline, "%% FileSize: %ld bytes\n", statbuf.st_size);
      PSputs(logline, output);
    }
    else {
/*      sprintf(logline, "ERROR: Unable to obtain info on %s\n", filename); */
    }
  }
  if (! stripcomment) {
//    PSputc('\n', output);     // always start on new line ?
    PSputs("%%BeginDocument: ", output);
    PSputs(fname, output);          /* 1995/July/15 */
    PSputc('\n', output);     
  }
//  PSputc('\n', output);
//  if (! stripcomment) {
//    if ((ret = _fstat(_fileno(special), &statbuf)) == 0) {
//      s = ctime(&statbuf.st_mtime);
//      if (s != NULL) {
//        lcivilize(s);
//        sprintf(logline, "%% FileDate: %s\n", s);
//        PSputs(logline, output);
//      }
//      sprintf(logline, "%% FileSize: %ld bytes\n", statbuf.st_size);
//      PSputs(logline, output);
//    }
//  }

  pslength = checkpcform(special, fname);

/*  if (pslength > 0) copyepsfilesimple(output, special, pslength); */
/*  else copyepsfilesub(output, special); */ /* finally copy inserted file */
  if (bSmartCopyFlag) copyepsfilesub(output, special, pslength);  /* 1994/Feb/23 */
  else copyepsfilesimple(output, special, pslength);    /* 1994/Feb/23 */

/*  NOTE: pslength = 0 if it is just a plain ASCII file */
  PSputc('\n', output);       /* just to be sure  - paranoia */
//  freshflag = 1;    // ?
  
/*  printf(" END OF SPECIAL"); */   /* debugging */

//  fclose(special);        // removed 99/July/14
//  if (ferror(output) != 0) {  /* see if running out of space! */
  if (output != NULL && ferror(output)) {
    showline ("\n", 0);
//    sprintf(logline, " ERROR in output file %s\n", outputfile);
    showline("ERROR in output file", 1);
//    perror(outputfile);
    perrormod((outputfile != NULL) ? outputfile : "");
    giveup(3);
    return;           // now returns in DLL 
  }

  if (stripcomment == 0) {
    PSputs("%%EndDocument", output);
    PSputc('\n', output);       /* just to be sure - paranoia */
//    freshflag = 1;    // ?
  }
/*  printf("nspecial = %d\n", nspecial); */
}

/* somewhat wastefully just reads bounding box and closes file again */
/* used by DVIALW to set up BoxHeight and BoxWidth */
/*
int setupbbox(char *epsfilename) {
  long pslength;
  FILE *special;

  if((special = findepsfile(epsfilename, "eps", 0, 0)) == NULL) return 0;
  pslength = checkpcform(special, epsfilename);
  read_bbox(special, epsfilename, pslength);
  fclose(special);
} */

/* incflag >  0 =>  include - i.e. reposition according to needshift */
/* incflag == 0 =>  overlay - i.e. use coordinates as in file */
/* incflag <  0 =>  include - but don't read bbox from EPS file again */

/* if needshift >  0 then (xll, yll) is at TeX's current point */
/* if needshift == 0 then (0,0) is at Tex's current point */
/* if needshift <  0 then (xll, yur) is at TeX's current point  */

/*  Note: copyepsfile also opens and closes the special file */

void copyepsfile (FILE *output, char *epsfilename, int inclflag, int needshift)
{
  FILE *special;
  long pslength;
  
/*  if ((special = findepsfile(epsfilename, 1, "eps")) == NULL) return; */
  if ((special = findepsfile(epsfilename, "eps", 1, 0)) == NULL) return;
/*  shouldn't this trigger an error - or is already taken care of ? */
  if (inclflag) { /* include - rather than overlay */
/*    fprintf(output, "undscl ");  */ /* currentpoint translate ? */
    if (needshift) {
      pslength =  checkpcform(special, filename);   
      deal_with_bbox(output, special, filename, pslength, needshift);
/*      rewind(special);  */
    }
  }
/*  else fprintf(output, "undscl "); */ /* overlay: revert original coords ??? */
  copyepsfileaux(output, special, epsfilename);
  fclose(special);
//  if (abortflag) return;
}

/* copy rest of special to get back to rest of DVI code (used by Textures) */

void colontoslash (char *name, char *buff)
{
  int c; 
  char *s=name; 
  char *t=buff;
  
/*  if (flushinit != 0) */
  if (*buff == ':') t++;    /* flush initial ':' */
  
/*  strcpy(name, t); 
  replaceletter(name, ':', '\\'); */

  while ((c = *t++) != 0) {
    if (c == ':' && *t != '\\' && *t != '/') *s++ = '\\';
    else *s++ = (char) c;
  } 
  *s = '\0';
}

void startspecial (FILE *output)
{
  PSputc('\n', output);   // always in new line ?
  PSputs("dvispsav undsclx dvispbeg ", output);
} 

void startspecial1 (FILE *output)
{
  PSputc('\n', output);   // always in new line ?
  PSputs("dvispsav ", output);
} 

void startspecial2 (FILE *output)
{
  PSputc('\n', output);   // always in new line ?
  PSputs("dvispbeg ", output);
} 

/* dvidict /dvispend get exec <== 92/Nov/26 */

void endspecial (FILE *output)
{
//  PSputc('\n', output);   // always in new line ?
  PSputs("dvidict /dvispend get exec\n", output);
}

#define MAXCOMPLAIN 127

void complainspecial (FILE *input) /* list contents and do flush_special */
{
  int c, k=0;
/*  long specnow; */

  if (quietflag != 0) {     /* easy if output suppressed */
    flush_special(input); return;
  }
/*  specnow = ftell(input); */      /* save current position */

  showline(" Don't understand special: ", 1);   /* 93/Jan/23 */
  if (traceflag) {
    sprintf(logline, "(at %d size %d) ",
               specstart, nspecial);  /* 99/Feb/21 */
    showline(logline, 1);
  }
/*  now go back and print it out ... */
  fseek(input, specstart, SEEK_SET);  /* go back to start of special */
  nspecial = nspecialsav;       /* restore count */
  if (nspecial <= 0) return;  /* shouldn't happen */
  c = getc(input); nspecial--;
  while (nspecial > 0 && c != EOF) {
    k++;
    if (k < MAXCOMPLAIN) {
      *logline = (char) c;
      *(logline+1) = '\0';
      showline(logline, 0);
    }
    c = getc(input); nspecial--;
  }
  if (k >= MAXCOMPLAIN) {
    showline("...", 1);
  }
  else {
    *logline = (char) c;
    *(logline+1) = '\0';
    showline(logline, 0);
  }
  showline(" ", 0);   // ???
/*  fseek(input, specnow, SEEK_SET); */ /* and restore count ? */
}

void complainjflag (FILE *input)          /* 1993/Oct/17 */
{
/*  if (complainedaboutj++ > 0) return; */
  if (complainedaboutj++ == 0) 
    showline(" WARNING: verbatim PS - use `j' flag?", 1);
  if (nspecial > 0) flush_special(input);      /* 1999/Mar/18 */
}

/* Textures style include eps file */ /* added "scaled <double>" ? */

void readtextures (FILE *output, FILE *input) /* Texture style special ? */
{
  int c;
  int clipflag = 0;
  double scale=1.0;
  char epsname[MAXLINE]="";

/*  c = get_token(input, line, MAXLINE); */
  c = get_token(input, line, sizeof(line));  /* MAXLINE */

  colontoslash(epsname, line);  /* deal with Mac font names */

  startspecial(output); 
  if (c > 0 && nspecial > 0) { /* anything left in special ? */
/*     read next token, see whether perhaps `scaled' or `clip' */
/*    (void) get_token(input, line, MAXLINE); */
    (void) get_token(input, line, sizeof(line)); /* MAXLINE */
    if (strcmp(line, "scaled") == 0) {
/*      (void) get_token(input, line, MAXLINE); */
      (void) get_token(input, line, sizeof(line)); /* MAXLINE */
      if(sscanf(line, "%lg", &scale) > 0) {
/*        printf(" SCALE %lg ", scale); */
        if (scale > 33.333) scale = scale/1000.0;
/*        delay the scaling to after clipping ... */
/*        if (scale != 1.0) fprintf(output, "%lg dup scale\n", scale);*/
      }
/*      else {  } */ /* error, can't read the scale */
/*       read next token, see whether perhaps `clip' */
/*      if (nspecial > 0) (void) get_token(input, line, MAXLINE); */
      if (nspecial > 0) (void) get_token(input, line, sizeof(line));
    }
    if (strcmp(line, "clip") == 0) clipflag = 1;  /* 1995/July/12 */
  }
/*  Make sure sequence of scale and clip is correct ! */
  if (scale != 1.0) {
    sprintf(logline, "%lg dup scale\n", scale);
    PSputs(logline, output);
  }
  if (clipflag) {               /* new code 95/July/12 */
    FILE *special;
    long pslength;
/*  Somewhat inefficient to pre-read first for bbox, but what the hell */
    if ((special = findepsfile(epsname, "eps", 1, 1)) != NULL) {
      xll = yll = xur = yur = 0.0;
      pslength = checkpcform(special, epsname);
      if (read_bbox(special, epsname, pslength) == 0) {
        sprintf(logline, "BoundingBox not found in %s ", epsname);
        showline(logline, 1);
      }
      else {              /* we did find the bbox */
//        putc('\n', output);
        PSputc('\n', output);
/*  Need to subtract xll, yll, since origin at current point and */
/*  copyepsfile immediately emits xll neg yll neg translate */
        sprintf(logline,  "newpath %lg %lg moveto %lg %lg lineto\n",
/*          xll, yll, xur, yll); */
          0.0, 0.0, xur-xll, 0.0);
        PSputs(logline, output);
        sprintf(logline,
          "%lg %lg lineto %lg %lg lineto closepath clip newpath\n",
/*          xur, yur, xll, yur); */
          xur-xll, yur-yll, 0.0, yur-yll);
        PSputs(logline, output);
      }
      fclose (special);
    }
  } /* end of clipflag code introduced 95/July/12 */
  copyepsfile(output, epsname, 1, 1);   /* include and shift */
  endspecial(output);
  flush_special(input);          /* flush whatever is left */
} 

/* Textures style "postscript" - direct inclusion of PostScript code */

/* void copypostscript(FILE *output, FILE *input) {*/ /* Texture style special ? */
void copypostscript (FILE *output, FILE *input, int rawflag) /* 1994/July/4 */
{
/*  startspecial(output);  */
/*  why not just use undscl ??? */
/*  fputs("\nrevscl ", output); */
/*  fputs("gsave currentpoint translate\n", output); */
/*  if (sebastianflag) putc('\n', output);  else */
//  putc('\n', output);
  PSputc('\n', output);
  if (rawflag == 0) {
/*    fputs("gsave undscl ", output); */  /* replace above 93/Oct/17 ? */
//    fputs("gsave undsclx ", output);
    PSputs("gsave undsclx ", output);
  }
/*  copy_string now inserts a `\n' here */   /* 1994/June/25 */
  copy_string(output, input);
/*  putc('\n', output); */        /* 93/June/3 */
/*  if (sebastianflag) putc('\n', output); else */
  if (rawflag == 0) {           /* 94/July/3 */
//    fputs("grestore ", output);
    PSputs("grestore ", output);
  }
/*  fputs("forscl\n", output); */   /* removed 1993/Oct/17 */
/*  putc('\n', output); */        /* just in case! */
/*  endspecial(output); */
} 

/* Textures style "postscriptfile" inclusion of PostScript file - no BBox */
/* can this really take a scale factor ? */ 
/* should this neuter stuff ? and use save-restore pair ? */
/* void readpostscript(FILE *output, FILE *input) { */ /* Texture style special ? */
void readpostscript (FILE *output, FILE *input, int rawflag) /* 1994/July/5 */
{
/*  int c; */
  double scale=1.0;
  char epsname[FNAMELEN]="";
  
/*  (void) get_token(input, epsname, FNAMELEN); */
  (void) get_token(input, epsname, sizeof(epsname)); /* FNAMELEN */
/*  if (get_token(input, line, MAXLINE) != 0 && */
  if (get_token(input, line, sizeof(line)) != 0 &&
    strcmp(line, "scaled") == 0 &&
      get_token(input, line, sizeof(line)) != 0 &&
        sscanf(line, "%lg", &scale) > 0) {
  }
/*  fprintf(output, "\nsb "); */
/*  startspecial(output);  */
//  putc('\n', output);
  PSputc('\n', output);
  if (rawflag == 0) {
/*    fputs("revscl ", output); */  /* switch to default coord ? */
//    fputs("revsclx ", output);
    PSputs("revsclx ", output);
//    fputs("gsave currentpoint translate ", output);
    PSputs("gsave currentpoint translate ", output);
  }
  if (scale > 33.33) scale = scale/1000.0;
  if (scale != 1.0) {
    sprintf(logline, "%lg dup scale ", scale);
    PSputs(logline, output);
  }
/*  copyepsfile(output, epsname, 1, 0);  */ /* included and no shift */
  copyepsfile(output, epsname, 0, 0);   /* no incl and no shift */
  if (rawflag == 0) {
//    fputs("grestore ", output);
    PSputs("grestore ", output);
/*    fputs("forscl", output); */   /* switch to DVI coord ? */
//    fputs("forsclx", output);   /* DVI coord ? 97/Apr/25 */
    PSputs("forsclx", output);    /* DVI coord ? 97/Apr/25 */
  }
//  putc('\n', output);
  PSputc('\n', output);
/*  endspecial(output); */
} 

/*  Try Andrew Trevorrow's OzTeX and Psprint Vax VMS syntax */
/*  - returns zero if this doesn't work */
/*  There may be a problem if /magnification != 1000 */
/*  - since OzTeX uses absolute 72 per inch scaling ??? */

int readandrew (FILE *output, FILE *input)
{
  FILE *special;
  char epsfilename[FNAMELEN]="";

  fseek(input, specstart, SEEK_SET);  /* start over again */
  nspecial = nspecialsav;       /* restore length */
/*  if (get_token(input, epsfilename, FNAMELEN) == 0) { */
  if (get_token(input, epsfilename, sizeof(epsfilename)) == 0) { /* FNAMELEN */
    flush_special(input);
    return 0;       /* fail, no token following */
  }
/*  if ((special = findepsfile(epsfilename, 0, "eps")) == NULL) { */
  if ((special = findepsfile(epsfilename, "eps", 0, 0)) == NULL) {
    flush_special(input);
    if (traceflag) {      /* debug output 95/July/15 */
      sprintf(logline, " can't find %s, or unknown \\special ",
          epsfilename); /* 95/June/21 */
      showline(logline, 1);
    }
    return 0;       /* fail, couldn't find file to insert */
  }

  startspecial(output);
/*  fprintf(output, "undscl "); */    /* currentpoint translate ? */
/*  pslength = checkpcform(special, filename);     */
/*  deal_with_bbox(output, special, filename, pslength, 1); */ 
/* -1, 0, or +1 ? */
/*  putc('\n', output); */      /* 1994/June/25 */
  copy_string(output, input);    /* copy rest of special */
/*  putc('\n', output); */      /* 1993/June/3 */
/*  copyepsfile(output, epsfilename, 1, 0);  */
/*  copyepsfile(output, epsfilename, 0, 0);   */
  copyepsfileaux(output, special, epsfilename); 
  fclose(special);
  endspecial(output);
  if (abortflag) return 0;  /* failed */
  return -1;            /* OK, it was OzTeX special (maybe) */
}

/* separator is ` ' (space) */ /* DVIALW style special ? */
int readdvialw (FILE *output, FILE *input)
{
  char epsfilename[FNAMELEN]="";
  long flitpos=0;       /* place in file where literal was */
  long fendspec;        /* saved pointer to end of special */
  int includeflag=1;      /* zero => overlay, otherwise => include */
  int firsttime=1;      /* already read first token */
  int fileflag=0;       /* non-zero include or overlay */
  int bboxflag=0;       /* bounding box specified */

  while (nspecial > 0) {    /* gather up information first */
/*    if (firsttime != 0) firsttime = 0; 
    else if (get_alpha_token(input, line, MAXLINE) == 0) break; */
    if (firsttime == 0)
/*      if (get_alpha_token(input, line, MAXLINE) == 0) break;  */
      if (get_alpha_token(input, line, sizeof(line)) == 0) break; 
    if (strcmp(line, "language") == 0) { 
/*      strcmp(line, "LANGUAGE") == 0) { */
      (void) get_string(input, line, MAXLINE);
      if (strcmp(line, "PS") == 0 ||
        strcmp(line, "PostScript") == 0) {
        /* we like PS, so no need to do anything ! */
      }
      else {
        complainspecial(input);
        break; 
      }
    }
    else if (strcmp(line, "include") == 0) {
/*           || strcmp(line, "INCLUDE") == 0) { */
      (void) get_string(input, epsfilename, FNAMELEN);
      includeflag = 1; fileflag = 1;
    }
    else if (strcmp(line, "overlay") == 0) {
/*        || strcmp(line, "OVERLAY") == 0 ) { */
      (void) get_string(input, epsfilename, FNAMELEN);
      includeflag = 0;  fileflag = 1;
    }
    else if (strcmp(line, "literal") == 0) {
/*        || strcmp(line, "LITERAL") == 0) { */
      flitpos = ftell(input);   /* remember where this was */
      nliteral = nspecial;
/*      copy_string(output, input); */
    }
/*    GRAPHICS and OPTIONS not yet defined - so flag as errors */   
    else if (strcmp(line, "boundingbox") == 0) {
/*  actually, this may involve TeX dimensions, see decodeunits? */
/*  - that seems truly bizarre, so ignore that possibility */
      (void) get_string(input, line, MAXLINE);
      if (sscanf(line, "%lg %lg %lg %lg", &xll, &yll, &xur, &yur) == 4)
/*  this should override bounding box in eps file if given */
        bboxflag = 1;
      else {
        sprintf(logline, 
          "Don't understand DVIALW bounding box: %s", line);
        showline(logline, 1);
        errcount(0);
      }
    }
    else if (strcmp(line, "message") == 0) {
      (void) get_string(input, line, MAXLINE);
/*      printf("%s\n", line); */
/*      putc(' ', stdout); */   /* 93/June/3 */
      showline(" ", 0);
      showline(line, 0);
/*      putc('\n', stdout);  */ /* 93/June/3 */
    }
/*    Andrew Treverrow file inclusion allows only if -j used */
    else if (verbatimflag && firsttime && readandrew(output, input)) {
      if (abortflag) return -1;
    } 
    else {
      if (!quietflag) {
        sprintf(logline,  /*  DVIALW  */
          " Unrecognized \\special keyword: %s ", line);
        showline(logline, 1);
      }
      flush_string(input); /*  errcount(0);  */
    }
    if (nspecial == 0) break;
    else skip_to_comma(input);  /* look for next key value pair */
    if (firsttime != 0) firsttime = 0; 
  }
  
/*  not implemented: `graphics', `options', `position' */
/*  now actually do something */
/*  startspecial(output); */  /* no ? may need different scale */
/*  fprintf(output, "\ndvispbegin "); */
/*  fputs("\ndvispbegin ", output); */
/*  fputs("\ndvispsav dvispbeg ", output); */
//  fputs("\ndvispsav ", output);   /* split 1992/Nov/26 */
  PSputc('\n', output);       // always on new line ?
  PSputs("dvispsav ", output);    /* split 1992/Nov/26 */

/*  include or overlay ? */
/*  if (includeflag != 0) fprintf(output, "undscl "); */ /* include */
/*  else fprintf(output, "dviso ");         */  /* overlay */
/*  if (includeflag != 0) fputs("undscl ", output); */  /* include ? */
/*  else fputs("revscl ", output); */         /* overlay ? */
  if (includeflag != 0) {
//    fputs("undsclx ", output);  /* include ? */
    PSputs("undsclx ", output); /* include ? */
  }
  else {
//    fputs("revsclx ", output);      /* overlay ? */
    PSputs("revsclx ", output);     /* overlay ? */
  }
//  fputs("dvispbeg ", output);   /* split 1992/Nov/26 */
  PSputs("dvispbeg ", output);    /* split 1992/Nov/26 */

  if (flitpos != 0) {     /* literal before included file if any */
    fendspec = ftell(input);  /* remember this place (end of special) */
    fseek(input, flitpos, SEEK_SET);  /* go back to where literal was */
    nspecial = nliteral;        /* reset nspecial */
/*    putc('\n', output); */      /* 1994/June/25 */
    copy_string(output, input);
/*    putc('\n', output); */        /* 1993/June/3 */
    fseek(input, fendspec, SEEK_SET); /* back to end of special */
    nspecial = 0;
  }
  if (fileflag != 0) {
/* should take into account given bounding box if bboxflag != 0 ? */
/*    copyepsfile(output, epsfilename, includeflag, 0); *//* no shift ? */
    if (includeflag != 0) {
      if (bboxflag == 0) copyepsfile(output, epsfilename, 1, -1);
      else {
        sprintf(logline, "%lg neg %lg neg translate\n", xll, yur);
        PSputs(logline, output);
        copyepsfile(output, epsfilename, 1, 0); 
      }
    }
    else copyepsfile(output, epsfilename, 0, 0);
  }
/*  if (fileflag != 0) */
  endspecial(output);
  return 0;
}

/* DVIALW also sets up PaperHeight = PageHeight PaperWidth = PageWidth OK */
/* DVIALW also sets up CurrentX and CurrentY OK */
/* DVIALW also sets up BoxHeight and BoxWidth OK if given */
/* DVIALW also sets up BoxHeight and BoxWidth need to read file */
/* DVIALW allows specification of both an included and an overlay file ? */
/* expects SB, SE, BPtoPX, FRAME, RESIZE and other stuff to be defined! */

/*  clip the epsf illustration if requested */

/* following needed to be made to work with userdict on top of dict stack */

void doclip (FILE *output) {            /* 92/Nov/28 */
  PSputc('\n', output);       // always on new line ?
  PSputs("dvidict begin ", output);
  PSputs("doclip end ", output);  
} 

void texlandscape(FILE *output) {       /* 92/Nov/28 */
  PSputc('\n', output);       // always on new line ?
  PSputs("dvidict begin ", output);
  PSputs("revscl Texlandscape forscl end\n", output);
}

void endtexfig(FILE *output) {          /* 92/Nov/28 */
  PSputc('\n', output);       // always on new line ?
  PSputs("dvidict /endTexFig get exec ", output);
}

/* Set things up for included figure (Trevor Darrell style) */
/*    these parameters are all in DVI units ! */
/*    this ignores BBox in file - uses bounding box in pstext special */
/* This is now redundant ...
void starttexfig(FILE *output, long pswidth, long psheight,
      long psllx, long pslly, long psurx, long psury) {
    if (psurx == psllx || psury == pslly) {
      fprintf(errout, "Zero area BoundingBox %ld %ld %ld %ld\n",
        psllx, pslly, psurx, psury);
      errcount(0); 
    }
    fprintf(output, "\n%ld %ld %ld %ld %ld %ld startTexFig",
      pswidth, psheight, psllx, pslly, psurx, psury);
} */

/* 1994/Sep/13 allow for scaling of printer coordinate system */
/* ... need to touch up coords in startTexFig line */

#ifdef ALLOWSCALE
void rescaletexfig (char *line)
{
  char *s;
  int n;
  long w, h, llx, lly, urx, ury;
/*  char temp[MAXCOMMENT]; */
  char temp[256]="";      /* guess it's long enough startTexFig line */

  if (outscaleflag == 0) return;      /* sanity check */
  if ((s = strpbrk(line, "-+1234567890")) == NULL) {
    return;               /* where are the numbers ? */
  }
/*  printf(" %d rest %s", s - line, s); */    /* debugging */
  if (sscanf(s, "%ld %ld %ld %ld %ld %ld%n",
    &w, &h, &llx, &lly, &urx, &ury, &n) < 6) {
    return;               /* not enough numbers ? */
  }
/*  printf(" %d tail %s", n, s+n); */   /* debugging */
  if (strlen(s+n) < sizeof(temp))     /* 1995/July/27 */
    strcpy (temp, s+n);   /* save the tail end of the line */
  sprintf(s, "%.9lg %.9lg %.9lg %.9lg %.9lg %.9lg",
    w / outscale, h / outscale,
      llx / outscale, lly / outscale, urx / outscale, ury / outscale);
/*  printf(" new %s\n", line); */       /* debugging */
  strcat(s, temp);    /* copy the tail back again */
/*  printf(" new %s\n", line); */       /* debugging */
}
#endif

void cantfind (FILE *input, char *s, char *line) { /* complain about missing field */
  sprintf(logline, " Can't find `%s' (in `%s') ", s, line);
  showline(logline, 1);
  flush_special(input);
  errcount(0); 
}

/* output verbatim what is in buffer - except leading white space */
/* rewritten 1994/June/27 for crude wrapping of long lines */

void verbout (FILE *output, char *str)
{
  char *s=str;          // work from string
  int c;
  int column=0;
  int nesting=0;          /* 1999/Feb/24 */
  int escape=0;         /* 1999/Feb/28 */
  
  if (*s == '\0') return;     /* nothing left to do ... */
  while (*s == ' ') s++;      /* step over leading white space */
  if (*s == '\0') return;     /* all just blanks ? */
  if (! freshflag) PSputc('\n', output);      /* paranoia --- old */

  if (bWrapSpecial == 0) {    // just copy it 
//    fputs(s, output);     /* fast, old version */
    PSputs(s, output);      /* fast, old version */
    PSputc('\n', output);   // paranoia 99/Dec/19
  }
  else {
    while ((c = *s++) != '\0') {
      if (c == '\n') column = 0;
      else column++;
      if (escape) escape = 0;
      else if (c == '(') nesting++;
      else if (c == ')') nesting--;
      else if (c == '\\') escape++;
/*      if (column > WRAPCOLUMN && c == ' ') */ /* 1994/Jun/27 */
      if (column > WRAPCOLUMN && nesting == 0 && c == ' ') {
        c = '\n';
        column = 0;
      }
//      putc(c, output);
      PSputc(c, output);
    }
    if (column > 0) {
//      putc('\n', output);   /* paranoia 1994/June/27 */
      PSputc('\n', output);   /* paranoia 1994/June/27 */
    }
  }
  freshflag = 1;    // all of the above end in /n
  showcount = 0;
}

/* split up the following - its too darn long ! */

/* Separator is `='     dvi2ps style */
int readdvi2ps (FILE *output, FILE *input)
{
  char epsfilename[FNAMELEN]="";
/*  int includeflag=1;  */    /* always include instead of overlay */
  int firsttime=1;      /* already read first token */
  int psepsfstyle=0;      /* non-zero =>  EPSF style */
  int clipflag=0;       /* for EPSF style 1994/Mar/1 */
/*  double llx, lly, urx, ury; */
/*  long fxll, fyll, fxur, fyur, frwi, frhe;  */
  double rwi=0.0, rhe=0.0;  /* for EPSF style */
/*  parameters old DVI2PS style */
  double hsize=0.0, vsize=0.0, hoffset=0.0, voffset=0.0;
  double hscale=1.0, vscale=1.0, rotation=0.0;
/*  FILE *special; */

  while (nspecial > 0) {    /* gather up parameters */
    if (firsttime != 0) firsttime = 0;
/*    else if (get_alpha_token(input, line, MAXLINE) == 0) break; */
    else if (get_alpha_token(input, line, sizeof(line)) == 0) break;
/*    deal with new PSFIG DVI2PS style useage */
    if (strcmp(line, "pstext") == 0) {
/*      ||  strcmp(line, "PSTEXT") == 0) { */
      if (scan_special(input, line, MAXLINE) == MAXLINE) {
        if (verbatimflag) {       /* long pstext= verbatim */
          fseek(input, fliteral, SEEK_SET);
          nspecial = nliteral;
/*          copy_string now emits '\n' here */   /* 1994/June/25 */
          copy_string(output, input);  /* revscl - forscl PSTEXT */
/*          putc('\n', output); */    /* 1993/June/3 */
        }
        else complainspecial(input); /* TOO DAMN LONG ! */
      }
/*      check whether this is a startTexFig  */
      else if (psfigstyle == 0 && /* avoid repeat  startTexFig */
        strstr(line, "startTexFig") != NULL) {
/*      if (sscanf(line, "%ld %ld %ld %ld %ld %ld", 
       &pswidth, &psheight, &psllx, &pslly, &psurx, &psury) == 6) { 
        starttexfig(output, pswidth, psheight, 
          psllx, pslly, psurx, psury); */
#ifdef ALLOWSCALE
        if (outscaleflag) rescaletexfig (line);
#endif
        verbout(output, line);    /* copy verbatim output */
        psfigstyle=1;       /* note we are inside */
      }
      else if (psfigstyle != 0 && /* avoid repeat  endTexFig */
        strstr(line, "endTexFig") != NULL) {
/*        verbout(output, line); */
        endtexfig(output);      /* 1992/Nov/28 */
        psfigstyle=0; 
      }
      else if (psfigstyle != 0 && /* otherwise makes no sense */
        strstr(line, "doclip") != NULL) {
/*        clip the epsf illustration if requested */
/*        verbout(output, line); */
        doclip(output);       /* 1992/Nov/28 */
      }
/*      maybe just copy the darn thing to the output ??? */
      else if (verbatimflag) verbout(output, line); 
      else complainjflag(input);    /* 1993/Oct/17 */
/*      else complainspecial(input); */
      flush_special(input);    /* ignore the rest */
    } /* end of PSTEXT= code */
/*    kludge to allow insertion of header information - NO PROTECTION ! */
    else if (_strcmpi(line, "header") == 0 ||
         _strcmpi(line, "headertext") == 0 || /* 93/Dec/29 */
         strcmp(line, "DSCheader") == 0 ||    /* 95/July/15 */
         strcmp(line, "DSCtext") == 0 ||    /* 95/July/15 */
         strcmp(line, "papersize") == 0 ||    /* 98/June/28 */
         strcmp(line, "DVIPSONE") == 0 ||   /* 99/Sep/6 */
         strcmp(line, "DVIWindo") == 0      /* 99/Sep/6 */
        ) {
/*    ignore now, already taken care of in dvipslog ! */
        flush_special(input);  /* taken care of in dvipslog */
    }
/*    kludge to change to PS default coordinates - NO PROTECTION ! */
    else if (strcmp(line, "verbatim") == 0 
/*        || strcmp(line, "VERBATIM") == 0 */
        ) {
      PSputc('\n', output);       // always on new line ?
      PSputs("revsclx ", output);
      if (preservefont) PSputs("currentfont ", output);
      copy_string(output, input);
      if (preservefont) PSputs("setfont ", output);
      PSputs("forsclx ", output);
    }
/*    deal with old PSFIG DVI2PS style useage - case sensitive */
    else if (strcmp(line, "psfile") == 0) {
/*      if (get_token(input, epsfilename, MAXFILENAME) == 0) */
      if (get_token(input, epsfilename, sizeof(epsfilename)) == 0) /* FNAMELEN */
        cantfind(input, line, epsfilename);
      else psepsfstyle=0;
    }
    else if (strcmp(line, "hsize") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &hsize) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=0;
    }
    else if (strcmp(line, "vsize") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &vsize) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=0;
    }
    else if (strcmp(line, "hoffset") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &hoffset) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=0;
    }
    else if (strcmp(line, "voffset") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &voffset) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=0;
    }
    else if (strcmp(line, "hscale") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &hscale) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=0;
    }
    else if (strcmp(line, "vscale") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
          sscanf(line, "%lg", &vscale) == 0) 
        cantfind(input, line, moreline);
      else psepsfstyle=0;
    }
    else if (strcmp(line, "rotation") == 0
        || strcmp(line, "angle") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &rotation) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=0;
    }
/* Now for Rokicki EPSF.TEX style --- case sensitive */
    else if (strcmp(line, "PSfile") == 0) { /* EPSF style - note uc/lc */
/*      if (get_token(input, epsfilename, MAXFILENAME) == 0)  */
      if (get_token(input, epsfilename, sizeof(epsfilename)) == 0) 
        cantfind(input, line, moreline);
      else psepsfstyle=1;
    }
    else if (strcmp(line, "llx") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &xll) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=1;
    }
    else if (strcmp(line, "lly") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &yll) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=1;
    }
    else if (strcmp(line, "urx") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &xur) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=1;
    }
    else if (strcmp(line, "ury") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &yur) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=1;
    }
    else if (strcmp(line, "rwi") == 0) {
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &rwi) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=1;
    }
/*    else if (strcmp(line, "rhe") == 0) { */   /* ??? */
    else if (strcmp(line, "rhe") == 0 ||
           strcmp(line, "rhi") == 0) {    /* 1994/Mar/1 */
/*      if (get_token(input, line, MAXLINE) == 0 || */
      if (get_token(input, moreline, sizeof(moreline)) == 0 ||
        sscanf(line, "%lg", &rhe) == 0) 
          cantfind(input, line, moreline);
      else psepsfstyle=1;
    }       
    else if (strcmp(line, "clip") == 0) clipflag = 1; /* 1994/Mar/1 */
    else complainspecial(input);      /* not one of the above */
  }
/*  now actually go and do something - unless `pstext=' style */
/*  - that is, if `psfile=' and filename present not empty */
  if (*epsfilename == '\0') return 0; /* NEW:  no file specified */
/*  NOTE possible conflict: two different uses of psfile= */
/*  one is old style DVI2PS, the other is by Rokicki's DVIPS */
  if (psfigstyle != 0)  {           /* old style DVI2PS */
/*    actually, now do nothing except copy the file ! */
    copyepsfile(output, epsfilename, 1, 0); /* include & no shift */
  }
/*  NOW: Tom Rokicki's DVIPS style (EPSF) ? */
  else if (psepsfstyle) {
//    sprintf(logline, "PSfile=%s, llx=%lg lly=%lg urx=%lg ury=%lg rwi=%lg, rhe=%lg",
//        epsfilename, xll, yll, xur, yur, rwi, rhe);
//    showline(logline, 1); // debugging only
    if (rwi != 0.0 && rhe == 0.0) {
      if (xur != xll)
        rhe = rwi * (yur - yll) / (xur - xll);
    }
    else if (rwi == 0.0 && rhe != 0.0) {
      if (yur != yll)
        rwi = rhe * (xur - xll) / (yur - yll);
    }
    rwi = rwi / 10.0; rhe = rhe / 10.0;   /* units are tenth pt */
    if (rwi == 0.0) rwi = xur - xll;
    if (rhe == 0.0) rhe = yur - yll;
    startspecial(output); 
//    putc('\n', output);
    PSputc('\n', output);
    if (xur != xll && yur != yll) {     /* 94/Mar/1 */
      sprintf(logline, "%lg %lg %lg sub div ", rwi, xur, xll);
      PSputs(logline, output);
      sprintf(logline, "%lg %lg %lg sub div scale\n", rhe, yur, yll);
      PSputs(logline, output);
    }
    else {                  /* 94/Mar/1 */
      sprintf(logline, " Bad BoundingBox: %lg %lg %lg %lg",
          xll, yll, xur, yur);
      showline(logline, 1);
    }
    sprintf(logline, "%lg neg %lg neg translate\n", xll, yll);
    PSputs(logline, output);
    if (clipflag) {             /* 94/Mar/1 */
      sprintf(logline, "newpath %lg %lg moveto %lg %lg lineto\n",
        xll, yll, xur, yll);
      PSputs(logline, output);
      sprintf(logline,
          " %lg %lg lineto %lg %lg lineto closepath clip newpath\n",
        xur, yur, xll, yur);
      PSputs(logline, output);
    }
    copyepsfile(output, epsfilename, 0, 0);
    endspecial(output);
    flush_special(input);
  }
/*  NOW old UNIX DVI2PS (psadobe) ? */
  else {
    startspecial(output);   
    if (hoffset != 0.0 || voffset != 0.0) {
      sprintf(logline, "%lg %lg translate ", hoffset, voffset);
      PSputs(logline, output);
    }
/* NOTE: in Rokicki DVIPS style, scale is given as percentage ... */
    if (hscale > 8.0) hscale = hscale/100.0;  /* Rokicki style ? */
    if (vscale > 8.0) vscale = vscale/100.0;  /* Rokicki style ? */
/* Using rather arbitrary threshold above  ... */
    if (hscale != 1.0 || vscale != 1.0)  {
/*      if (hscale == vscale)
        fprintf(output, "%lg dup scale ", hscale); 
      else */
      sprintf(logline, "%lg %lg scale ", hscale, vscale);
      PSputs(logline, output);
    }
    if (rotation != 0.0) {
      sprintf(logline, "%lg rotate ", rotation);
      PSputs(logline, output);
    }
    if (hsize != 0.0 && vsize != 0.0) { /* both must be given */
      sprintf(logline,
        "0 0 %lg %lg clipfig\n", hsize, vsize); 
      PSputs(logline, output);
    }
    else if (hsize != 0.0 || vsize != 0.0) {
      showline(" Specify both HSIZE and VSIZE or neither", 1);
/*      maybe allow separate specification later ? */
      errcount(0); 
    }
    copyepsfile(output, epsfilename, 1, 0);  /* include & no shift */
/*    copyepsfileaux(output, special, epsfilename); */
/*    the above needs testing ? */
    endspecial(output);   
  }
  flush_special(input);    /* ignore the rest ? */
  if (abortflag) return -1;
  else return 0;
}

/* example: \special{picture screen0 scaled 500} */
/* To do this, one would need to extract a bit-map */

void readpicture (FILE *output, FILE *input)
{
  complainspecial(input);
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  */

/* This attempts to do Textures style color calls */

/* Does not do anything about rule color, only text color */
/* Does not do anything about BackgroundColor only TextColor */
/* Does not do anything fancy about CMYK instead of RGB */
/* Does not do anything about page boundaries - should it ? */

/* Not implemented in DVIPSONE yet - use setcymkcolor unless undefined */
/* 1 - .30 * C - .59 * M - .11 * Y - K => setgray on B/W devices */

COLORSPEC ColorStack[MAXCOLORSTACK];

COLORSPEC CurrColor;

/* The following were separated out for convenience 96/Nov/3 */

int doColorPop (int pageno)
{
  int flag = 0;
  if (colorindex <= 0) {
    sprintf(logline, " %s stack underflow on page %d\n", "color", pageno);
    showline(logline, 1);
    colorindex = 1;
    flag = -1;
    errcount(0);    /* ??? */
/*    return flag; */
  }
/*  CurrentA = ColorA[colorindex];
  CurrentB = ColorB[colorindex];
  CurrentC = ColorC[colorindex];
  CurrentD = ColorD[colorindex]; */
  CurrColor = ColorStack[--colorindex];
  if (colorindex == 0) {        /* hit bottom of stack ? */
    if (CurrColor.A == 0.0 && CurrColor.B == 0.0 && CurrColor.C == 0.0)
    {
      if (colortypeflag == 2)     /* cmyk */
        CurrColor.D = 1.0F;
      else if (colortypeflag == 1)  /* rgb */
        CurrColor.D = -1.0F;
/*      else if (colortypeflag == 0)  */
      else              /* gray */
        CurrColor.D = -2.0F;
    }
  }
/*  printf("POPPED: %g %g %g %g ", CurrColor.A, CurrColor.B, CurrColor.C, CurrColor.D); */
  return flag;
}

int doColorPush (int pageno)
{
/*  ColorA[colorindex] = CurrentA;
  ColorB[colorindex] = CurrentB;
  ColorC[colorindex] = CurrentC;
  ColorD[colorindex] = CurrentD;
  colorindex++; */
/*  printf("PUSHED: %g %g %g %g ", CurrColor.A, CurrColor.B, CurrColor.C, CurrColor.D); */
  ColorStack[colorindex++] = CurrColor;
  if (colorindex >= MAXCOLORSTACK) {
    sprintf(logline, " %s stack overflow on page %d\n", "color", pageno);
    showline(logline, 1);
    colorindex = MAXCOLORSTACK-1;
    errcount(0);      /* ??? */
    return -1;
  }
  else return 0;
}

/*  Actually output color setting commands */
/*  CurrentD == -1.0 use RGB *//* CurrentD >= 0.0 use CMYK *//* else use GRAY */
/*  popflag == 0 => normal color setting */
/*  popflag == 1 => result of color pop */
/*  popflag == 2 => beginning of page color setting */
/*  popflag == 3 => background color setting at top of page */

void doColorSet (FILE *output, int popflag)
{
  if (! freshflag) PSputc('\n', output);

  if (CurrColor.D >= 0.0)
  {
    sprintf(logline, "%g %g %g %g cmyk ",
      CurrColor.A, CurrColor.B, CurrColor.C, CurrColor.D);
    PSputs(logline, output);
  }
  else if (CurrColor.D == -1.0F)
  {
    sprintf(logline, "%g %g %g rgb ",
      CurrColor.A, CurrColor.B, CurrColor.C);
    PSputs(logline, output);
  }
/*  else if (CurrColor.D == -2.0F)   */
  else {
    sprintf(logline, "%g gray ",  CurrColor.A);
    PSputs(logline, output);
  }
  if (popflag && ! directprint)
  {
    if (popflag == 1) {
      PSputs("% pop", output);
    }
    else if (popflag == 2) {
      PSputs("% bop", output);
    }
    else if (popflag == 3) {
      PSputs("% background", output);
    }
  }
  PSputc('\n', output);

  freshflag = 1;    // all of the above end in \n
  showcount = 0;
}

int doColorPopAll (int pageno)       /* 1996/Nov/3 */
{
  if (colorindex == 0) return -1;
  while (colorindex > 0) doColorPop(pageno);
  return 0;
}

/* called from dvianal.c if color stack not empty at EOP */

int checkColorStack (FILE *output)   /* 96/Nov/3 */
{
  if (colorindex == 0) return 0;
  if (bCarryColor == 0) {       /* 98/Feb/14 */
    showline(" color stack not empty at EOP", 1);
    if (doColorPopAll(pageno) == 0)
      doColorSet (output, 1);   /* result of pop */
    errcount(0);    /* ??? */
  }
  return -1;
}

/* Do DVIPS / Textures style color calls */
/* \special{color cmyk <c> <m> <y> <k>} */
/* \special{color rgb <r> <g> <b>} */
/* \special{color gray <d>} */
/* \special{color push} and \special{color pop} */
/* also allow combinations such as: \special{color push rgb <r> <g> <b>} */

/* outflag controls whether emitting PS output - off when skipflag set */

void doColor (FILE *output, FILE *input, int c, int outflag)  /* 95/Mar/1 */
{
  char *s;
  int n, m, setcolor=0, popflag = 0;
  long page;

  if (reverseflag) page = dvi_t - pagenumber + 1;
  else page = pagenumber;

  s = line + strlen(line);
  *s++ = (char) c;            /* stick in terminator */
  *s = '\0';                /* just in case */
/*  (void) scan_special(input, s, MAXLINE); */     /* read rest of line ? */
  (void) scan_special(input, line, MAXLINE);   /* read rest of line */
  if (traceflag) {
    sprintf(logline, "\n(index %d outflag %d) %s ", colorindex, outflag, line); /* debugging */
    showline(logline, 0);
  }
  s = line;
  if (bKeepBlack) return;         /* 96/Nov/3 */
  for(;;) {               /* allow multiple commands */
    while (*s <= ' ' && *s != '\0') s++;  /* step over white space */
    if (*s == '\0') break;        /* processed all of them */
    if (strncmp(s, "pop", 3) == 0) {
      if (doColorPop(page) == 0) setcolor = 1;
      popflag = 1;          /* pop comment flag */
      s += 3;
    }
    else if (strncmp(s, "popall", 6) == 0) {
      if (doColorPopAll(page) == 0) setcolor = 1;
      popflag = 1;          /* pop comment flag */
      s += 6;
    }
    else if (strncmp(s, "push", 4) == 0) {
      (void) doColorPush(page);
      s += 4;
    }
    else if (strncmp(s, "rgb", 3) == 0) {
      s += 3;
      m = sscanf(s, "%g %g %g%n\n", &CurrColor.A, &CurrColor.B, &CurrColor.C, &n);
      if (m == 3) {
        s += n;
        CurrColor.D = -1.0F;
        setcolor = 1;
        if (colorindex == 1) colortypeflag = 1; /* mark RGB */
      }
      else {
        complainspecial(input);
        break;
      }
    }
    else if (strncmp(s, "cmyk", 4) == 0) {
      s += 4;
      m = sscanf(s, "%g %g %g %g%n",
             &CurrColor.A, &CurrColor.B, &CurrColor.C, &CurrColor.D, &n);
      if (m == 4) {
        setcolor = 1;
        if (colorindex == 1) colortypeflag = 2; /* mark CMYK */
        s += n;
      }
      else {
        complainspecial(input);
        break;
      }
    }
    else if (strncmp(s, "gray", 4) == 0) {  /* 1995/April/30 */
      s += 4;
      m = sscanf(s, "%g%n", &CurrColor.A, &n);
      if (m == 1) {
        CurrColor.C = CurrColor.B = CurrColor.A;
        CurrColor.D = -2.0F;
        setcolor = 1;
        if (colorindex == 1) colortypeflag = 0; /* mark gray */
        s += n;
      }
      else {
        complainspecial(input);
        break;
      }
    }
    else if (_strnicmp(s, "black", 5) == 0) { /* 96/Feb/4 */
      s += 5;
      CurrColor.C = CurrColor.B = CurrColor.A = 0.0F;
      CurrColor.D = 1.0F;
      setcolor = 1;
      if (colorindex == 1) colortypeflag = 2; /* mark cmyk */
    }
    else if (_strnicmp(s, "white", 5) == 0) { /* 96/Feb/4 */
      s += 5;
      CurrColor.C = CurrColor.B = CurrColor.A = 0.0F;
      CurrColor.D = 0.0F;
      setcolor = 1;
      if (colorindex == 1) colortypeflag = 2; /* mark cmyk */
    }
    else {
      complainspecial(input); /* 1995/April/15 */
      break;
    }
  }   /* end of for(;;) loop */

/*  if (setcolor) */
  if (setcolor && outflag)    /* 98/Feb/14 */
    doColorSet(output, popflag); /* actually output color setting code */
}

/* Eventually use preamble /tranCP and /untranCP etc. for this */
/* Can only have coordinate transform in between, no TeX output */

/* Move origin to current point before rotate or scale */
char *tranCP=" currentpoint currentpoint translate\n";
/* Move origin back after rotate or scale */
char *untranCP=" neg exch neg exch translate\n";

/* push CTM */
char *saveCTM=" gsave\n";
/* pop CTM preserve current point */
// char *endCTM=" currentpoint grestore moveto\n";
char *restoreCTM=" currentfont currentpoint grestore moveto setfont\n"; // 2000 May 27
/* pop CTM do not preserve current point */
// char *endCTMshort=" grestore\n";
char *restoreCTMnoCP=" currentfont grestore setfont\n"; // 2000 May 27
// char *endallCTM=" grestoreall\n";
char *restoreCTMall=" curentfont grestoreall setfont\n";  // 2000 May 27

/************************ support for CTM transformations ****************/

#ifdef AllowCTM

int CTMstackindex= 0;   /* try and avoid underflow at least */
              /* should reset at top of page */
              /* should check at bottom of page */

/* don't need to maintain our copy of CTM after all ... */

/* flag != 0 (pop*) means do not preserve current point */

int popCTM (FILE *output, int flag)
{
  CTMstackindex--;
  if (CTMstackindex < 0) {
    sprintf(logline, " %s stack underflow on page %d\n", "CTM", pageno);
    showline(logline, 1);
    CTMstackindex = 0;
    errcount(0);    /* ??? */
    return -1;
  }
  if (flag == 0) {
//    PSputs(" currentpoint grestore moveto\n", output);
    PSputs(restoreCTM, output);
  }
  else {
//    PSputs(" grestore\n", output);
    PSputs(restoreCTMnoCP, output);
  }
  return 0;
}

void popallCTM (FILE *output, int flag)
{
//  PSputs(" grestoreall\n", output);
//  PSputs(" curentfont grestoreall setfont\n", output);
  PSputs(restoreCTMall, output);
  CTMstackindex = 0;
}

int checkCTM (FILE *output)    /* CALL AT END OF PAGE */
{
  if (CTMstackindex == 0) return 0;
  showline(" CTM stack not empty at EOP", 1);
  popallCTM(output, 0);
  errcount(0);    /* ??? */
  return -1;
}

int pushCTM (FILE *output, int flag)
{
  CTMstackindex++;
//  PSputs(" gsave\n", output);
  PSputs(saveCTM, output);
  return 0;
}

/* flag != 0 (translate*) means reverse direction */
/* NOTE: This has different semantics from rotate, scale, and concat */
/* If would make no sense to make it relative to current point! */

void translateCTM (FILE *output, double dx, double dy, int flag)
{
  PSputs(" currentpoint\n", output);  // remember current point
  dy = - dy;      /* upside down coordinate system 96/Nov/5 */
  if (flag == 0) {
    sprintf(logline, "%lg %lg translate ", dx, dy);
  }
  else {
    sprintf(logline, "%lg neg %lg neg translate ", dx, dy);
  }
  PSputs(logline, output);
  PSputs(" moveto\n", output);  // restore current point
}

/* flag != 0 (scale*) means scale by inverse of given factors */

int scaleCTM (FILE *output, double sx, double sy, int flag)
{
  PSputs(tranCP, output);
//  PSputc('\n', output);
  if (flag == 0) {
    sprintf(logline, "%lg %lg scale ", sx, sy);
  }
  else {
    if (sx == 0 || sy == 0) {
      showline(" divide by zero in scale*\n", 1);
      return -1;
    }
    sprintf(logline, "1 %lg div 1 %lg div scale ", sx, sy);
  }
  PSputs(logline, output);
  PSputs(untranCP, output);
//  PSputc('\n', output);
  return 0;
}

/* flag != 0 (rotate*) means rotate in opposite direction */
/* NOTE: we are in a left-hand coordinate system so flip sign of angle */

void rotateCTM (FILE *output, double theta, int flag)
{
  PSputs(tranCP, output);
//  PSputc('\n', output);
  if (flag == 0) {
    sprintf(logline, "%lg neg rotate ", theta);
  }
  else {
    sprintf(logline, "%lg rotate ", theta);
  }
  PSputs(logline, output);
  PSputs(untranCP, output);
//  PSputc('\n', output);
}

/* We don't allow translation in concat --- for reason see translate */
/* NOTE: we are in a left-hand coordinate system so flip sign of m12 m21 */

void concatCTM (FILE* output,
  double m11, double m12, double m21, double m22, double m31, double m32,
      int flag)
{
  double det, n11, n12, n21, n22, n31, n32;
  PSputs(tranCP, output);
//  PSputc('\n', output);
  if (flag == 0) {
    sprintf(logline, "[%lg %lg neg %lg neg %lg %lg %lg] concat ",
      m11, m12, m21, m22, 0.0, 0.0);
    PSputs(logline, output);
  }
  else {
    det = m11 * m22 - m21 * m12;
    if (det == 0.0) {
      sprintf(logline, " Zero determinant in concat* (%lg %lg %lg %lg)",
           m11, m12, m21, m22);
      showline(logline, 1);     
    }
    else {
      n11 = m22 / det; n12 = - m12 / det;
      n21 = -m21 / det; n22 = m11 / det;
      n31 = (m21 * m32 - m22 * m31) / det;
      n32 = (m12 * m31 - m11 * m32) / det;
      sprintf(logline, "[%lg %lg neg %lg neg %lg %lg %lg] concat ",
          n11, n12, n21, n22, 0.0, 0.0);
      PSputs(logline, output);
    }
  }
  PSputs(untranCP, output);
//  PSputc('\n', output);
}

/* Deal with \special{CTM: ...} push pop rotate scale translate concat */
/* Some of these have alternate forms indicated by trailing `*' */

void doCTM (FILE *output, FILE *input)  /* \special{CTM: } 96/Oct/10 */
{
  char *s;
  int n, flag;
  double dx, dy, sx, sy, theta, m11, m12, m21, m22, m31, m32;

  (void) scan_special(input, line, MAXLINE); /* read rest of line */
  s = line;
//  putc('\n', output);           /* separate from previous */
  PSputc('\n', output);           /* separate from previous */
  for(;;) {               /* allow multiple commands */
    while (*s <= ' ' && *s != '\0') s++;  /* step over white space */
    if (*s == '\0') break;        /* processed all of them */
    n = 0;                /* no of characters to step over */
    flag = 0;             /* set if * follows command */
    if (traceflag) {
      sprintf(logline, "REMAINDER: %s\n", s); /* debugging only */
      showline(logline, 0);
    }
    if (strncmp(s, "pop", 3) == 0) {
      s += 3;
      if (*s == '*') {        /* pop* */
        flag++;       s++;
      }
/*      if (popCTM(output, flag) != 0) break; */
      (void) popCTM(output, flag);
    }
    else if (strncmp(s, "popall", 6) == 0) {
      s += 6;
      if (*s == '*') {        /* popall* */
        flag++;       s++;
      }
      popallCTM(output, flag);
    }
    else if (strncmp(s, "push", 4) == 0) {
      s += 4;
      if (*s == '*') {        /* push* */
        flag++;       s++;
      }
      if (pushCTM(output, flag) != 0) break;
    }
    else if (strncmp(s, "translate", 9) == 0) {
      s += 9;
      if (*s == '*') {        /* translate* */
        flag++;       s++;
      }
      if (sscanf(s, "%lg %lg%n", &dx, &dy, &n) < 2) {
        complainspecial(input);
        break;
      }
      translateCTM(output, dx, dy, flag);
    }
    else if (strncmp(s, "scale", 5) == 0) {
      s += 5;
      if (*s == '*') {        /* scale* */
        flag++;       s++;
      }
      if (sscanf(s, "%lg %lg%n", &sx, &sy, &n) < 2) {
        complainspecial(input);
        break;
      }
      if (scaleCTM(output, sx, sy, flag) != 0) break;
    }   
    else if (strncmp(s, "rotate", 6) == 0) {
      s += 6;
      if (*s == '*') {        /* rotate* */
        flag++;       s++;
      }
      if (sscanf(s, "%lg%n", &theta, &n) < 1) {
        complainspecial(input);
        break;
      }
      rotateCTM(output, theta, flag);
    }
    else if (strncmp(s, "concat", 6) == 0) {
      s += 6;
      if (*s == '*') {
        flag++;       s++;
      }
      m31 = m32 = 0.0;
      if (sscanf(s, "%lg %lg %lg %lg%n",
             &m11, &m12, &m21, &m22, &n) < 4) { 
        complainspecial(input); /* must have at least 4 arguments */
        break;
      }
      else {        /*  or could have 6 arguments */
        sscanf(s, "%lg %lg %lg %lg %lg %lg%n",
             &m11, &m12, &m21, &m22, &m31, &m32, &n);
      }
      concatCTM(output, m11, m12, m21, m22, m31, m32, flag);
    }
    else {
      complainspecial(input);
      break;
    }
    s += n;               /* step over arguments */
  }
/*  putc('\n', output); */          /* separate from following */
  flush_special(input);
}
#endif

/*******************************************************************/

/* logical coordinates upper left corner, lower right corner */

void clipbox (FILE *output, long dwidth, long dheight, int flag)
{
/*  int ret; */

  PSputc('\n', output);       // always on new line ?
  sprintf(logline, "%ld %ld %d clipbox ", dwidth, dheight, flag);
  PSputs(logline, output);
/*  if (flag) {
    ret = ExcludeClipRect(output,  xll, yll, xur, yur);
  }
  else {
    ret = IntersectClipRect(output, xll, yll, xur, yur);
  } */
}

void doClipBoxPush (FILE *output)
{
//  PSputc('\n', output);       // always on new line ?
//  PSputs("gsave ", output);
//  PSputs(" gsave\n", output); // saveCTM
  PSputs(saveCTM, output);
  clipstackindex++;
}

void doClipBoxPop (FILE *output)
{
  if (clipstackindex <= 0) return;  /* avoid stack underflow */
  clipstackindex--;
//  PSputc('\n', output);       // always on new line ?
//  PSputs("currentfont currentpoint grestore moveto setfont ", output); 
//  PSputs(" currentfont currentpoint grestore moveto setfont\n", output); // restoreCTM
  PSputs(restoreCTM, output);
}

void doClipBoxPopAll (FILE *output)
{
  while (clipstackindex > 0) doClipBoxPop(output);
}

/* int c is terminating character */

void doClipBox (FILE *output, FILE *input, int c) /* 98/Sep/8 */
{
  char *s;
  long dwidth, dheight;
/*  long cxll, cyll, cxur, cyur; */
  int n, flag, subtract, stroke;

/*  if (c == '*') subtract=1; */    /* subtract rather than intersect */
  (void) scan_special(input, line, MAXLINE);   /* read rest of line */
  s = line;
/*  printf("LINE: %s\n", line); */
/*  while (*s != ' ' && *s != '\0') s++; */
  flag = subtract = stroke = 0;
  for(;;) {               /* allow multiple commands */
    while (*s <= ' ' && *s != '\0') s++;  /* step over white space */
    if (*s == '\0') break;        /* processed all of them */
/*    printf("S: %s\n", s); */
/*    cxll = cyll = cxur = cyur = 0; */
    dwidth = dheight = 0;
    if (strncmp(s, "pop", 3) == 0) {
      (void) doClipBoxPop(output);
      s += 3;
      continue;
    }
    if (strncmp(s, "push", 4) == 0) {
      (void) doClipBoxPush(output);
      s += 4;
      continue;
    }
    if (strncmp(s, "popall", 6) == 0) {
      (void) doClipBoxPopAll(output);
      s += 6;
      continue;
    }
    if (strncmp(s, "stroke", 6) == 0) {
      stroke=1;
      s += 6;
      continue;
    }
/*    if (sscanf(s, "%ld %ld%ld %ld%n",
           &cxll, &cyll, &cxur, &cyur, &n) == 4) {
      s += n;
      clipbox(output, cxll, cyur, cxur, cyll, flag);
    } */
    if (strncmp(s, "box", 3) == 0) {
      s += 3;
      if (*s == '*') {
        s++;
        subtract=1;
      }
      while (*s <= ' ' && *s != '\0') s++;  /* step over white space */
      if (sscanf(s, "%ld %ld%n", &dwidth, &dheight, &n) == 2) {
        s += n;
        flag = (stroke << 1) | subtract;
/*        printf("FLAG %d STROKE %d SUBTRACT %d\n",
           flag, stroke, subtract); */
        clipbox(output, dwidth, dheight, flag);
      }
      else {
        complainspecial(input); /* must have at least 2 arguments */
        break;
      }
    }
    else {
      complainspecial(input);
      break;
    }
  }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Units of measure recognized by TeX */

char *unitnames[] = {
  "pt", "pc", "in", "bp", "cm", "mm", "dd", "cc", "sp"
};

double unitscale[] = {
  (72.0 / 72.27),
  12.0 * (72.0 / 72.27), 72.0, 1.0,
  (72.0 / 2.54), (72.0 / 25.4), (1238.0  / 1157.0) * (72.0 / 72.27),
  12.0 * (1238.0  / 1157.0) * (72.0 / 72.27),
  (72.0 / 72.27) / 65536.0
};

double decodeunits (char *units)
{
  int k;
  for (k = 0; k < 9; k++) {
    if (strcmp(unitnames[k], units) == 0)
      return unitscale[k];
  }
  sprintf(logline, " Don't understand units: `%s' ", units);
  showline(logline, 1);
  return 1.0;
}

/* make common to save string space */

void dvitopsmiss (void)
{
  showline(" File name missing in DVITPS, DVITOPS or PC-TeX special ", 1);
}

/*    DVITOPS specials for figure insertion \special{dvitops: ...}  */
/* Not implemented: begin, end, origin, rotate, transform ... regions color */
/*    added PC-TeX \special{eps: <filename> <x=2in> <y=3cm>} 94/Jun/17 */

/* void readdvitops(FILE *output, FILE *input) { */ /* 1994/June/17 */
int readdvitops (FILE *output, FILE *input, int pctexflag)
{
  double width=0.0, height=0.0, widthd, heightd;
  double hscale, vscale, scale, multiple;
  double xshift, yshift;
  char epsfilename[FNAMELEN]="";
  char units[3];
  int n;
  char *s, *sn;
  long pslength;
  FILE *special;

  if (pctexflag == 0) { /* if dvitops: check "import" next */
/*    if (get_alpha_token(input, line, MAXLINE) == 0) */
    if (get_alpha_token(input, line, sizeof(line)) == 0) {
      complainspecial(input);
      return -1;
    }
  }
/*  if (strcmp(line, "import") == 0) */
  if (pctexflag || strcmp(line, "import") == 0) {
/*    if(get_token(input, epsfilename, FNAMELEN) == 0) */
    if(get_token(input, epsfilename, sizeof(epsfilename)) == 0) {  /* FNAMELEN */
      dvitopsmiss();
      errcount(0);
      flush_special(input);
      return -1;
    }
/*    see if width and height specified in dvitops: import */
    if (scan_special(input, line, MAXLINE) == MAXLINE) {
      complainspecial(input); /* TOO DAMN LONG ! */
    }
/*    scale specified width and height to PS points */
    else {
      s = line;
/*      step over "x=" in PC-TeX style 94/June/17 */
      if (pctexflag && (sn = strstr(s, "x=")) != NULL) s = sn + 2;
      if (sscanf(s, "%lg%n", &width, &n) != 0) {
        s +=  n;
        units[0] = *s++; units[1] = *s++; units[2] = '\0';
        multiple = decodeunits(units);
        width = width * multiple;
      }
/*      else fprintf(errout, "Width not specified"); */
/*      step over "y=" in PC-TeX style 94/June/17 */
      if (pctexflag && (sn = strstr(s, "y=")) != NULL) s = sn + 2;
      if (sscanf(s, "%lg%n", &height, &n) != 0) {
        s +=  n;
        units[0] = *s++; units[1] = *s++; units[2] = '\0';
        multiple = decodeunits(units);
        height = height * multiple;
      }
/*      else fprintf(errout, "Height not specified");  */
/*      printf(" Height %lg Width %lg ", height, width); */

      xll = yll = xur = yur = 0.0;  /* 1993/Nov/7 */

/*      if ((special = findepsfile(epsfilename, 1, "eps")) == NULL) */
      if ((special = findepsfile(epsfilename, "eps", 1, 0)) == NULL) 
        return -1;
      pslength = checkpcform(special, epsfilename);   /* ??? */
/*      should failure to read BBox be considered OK here ? */
      if (read_bbox(special, epsfilename, pslength) == 0) {
/*        may want to suppress error message ? */
/*        fprintf(errout, "BBox not found in %s ", epsfilename); */
        sprintf(logline, "BoundingBox not found in %s ", epsfilename);
        showline(logline, 1);
      }
/*      else { */
/*        rewind(special); */
      startspecial(output);
      widthd = xur - xll; heightd = yur - yll;
/* following two lines added 1993/Nov/7 -- only make sense if BBox missed */
      if (widthd == 0.0) widthd = width;    /* in case no BBox */
      if (heightd ==0.0) heightd = height;  /* in case no BBox */
      if (width == 0.0) width = widthd;   /* in case not given */
      if (height == 0.0) height = heightd;  /* in case not given */
      if (widthd == 0.0 || heightd == 0.0) {
        sprintf(logline, "Zero area BoundingBox %lg %lg %lg %lg\n",
            xll, yll, xur, yur);
        showline(logline, 1);
        errcount(0);
        return -1;
      }
      hscale = width / widthd; vscale = height / heightd;
      xshift = 0.0; yshift = 0.0;     /* 1993/Nov/7 */
      if (hscale <= vscale) { /* horizontal scale limit */
/*        shift = (height - heightd * hscale) / 2.0; */
        yshift = (height - heightd * hscale) / 2.0;
/*        fprintf(output, "0 %lg translate ", shift); */
        if (yshift != 0.0) {
          sprintf(logline, "0 %lg translate ", yshift);
          PSputs(logline, output);
        }
/*        fprintf(output, "%lg dup scale\n", hscale); */
        scale = hscale;
      }
      else {          /* vertical scale limit */
/*        shift = (width - widthd * vscale) / 2.0; */
        xshift = (width - widthd * vscale) / 2.0;
/*        fprintf(output, "%lg 0 translate ", shift); */
        if (xshift != 0.0) {
          sprintf(logline, "%lg 0 translate ", xshift);
          PSputs(logline, output);
        }
/*        fprintf(output, "%lg dup scale\n", vscale); */
        scale = vscale;
      }
      if (scale != 1.0) {
        sprintf(logline, "%lg dup scale\n", scale);
        PSputs(logline, output);
      }
      if (xll != 0.0 || yll != 0.0) {
        sprintf(logline, "%lg neg %lg neg translate ", xll, yll);
        PSputs(logline, output);
      }
      copyepsfileaux(output, special, epsfilename);
/*      copyepsfile(output, epsfilename, 0, 0); */
      endspecial(output); 
      fclose(special);
/*      } */
    }
  }
  else if (strcmp(line, "inline") == 0) {
    if (verbatimflag) {     /* 1993/Oct/17 */
/* NOTE: dvitops has DVI coordinates in effect for `inline' commands */
/* - so DONT use forscl and revscl here ! */
//      putc('\n', output);     /* just in case! */
      PSputc('\n', output);     /* just in case! */
/*      if (preservefont != 0) fprintf(output, "currentfont "); *//*  NEW */
      if (preservefont) {
//        fputs("currentfont ", output); 
        PSputs("currentfont ", output); 
      }
/*      putc('\n', output); */      /* 1994/June/25 */
      copy_string(output, input);  /* just copy what follows ! */
/*      putc('\n', output); */      /* 1993/June/3 */
/*      if (preservefont) fprintf(output, "setfont "); */
      if (preservefont) {
//        fputs("setfont ", output);
        PSputs("setfont ", output);
      }
/*      putc('\n', output); */
/*      copyspecial(output, input); */
    }
    else complainjflag(input);      /* 1993/Oct/17 */
  }
  else if (strcmp(line, "landscape") == 0) {
    texlandscape(output);         /* 1992/Nov/28 */
  }
  else complainspecial(input);
  flush_special(input);
  if (abortflag) return -1;
  else return 0;
}

void reademtex(FILE *, FILE *);

/*    DVITPS specials for figure insertion --- 1993/March/24  */
/*    Not done as carefully as it should be, see DVI2PS and DVIPS code */

int readdvitps (FILE *output, FILE *input)
{
  char epsfilename[FNAMELEN]="";
/*  char units[3]; */
  long pslength;
  FILE *special;
/*  char *s=line; */
  
/*  if (get_alpha_token(input, line, MAXLINE) == 0) {*//* get DVITPS command */
  if (get_alpha_token(input, line, sizeof(line)) == 0) {  /* get DVITPS command */
    complainspecial(input);
    return -1;
  }
  if (strcmp(line, "Include1") == 0) {      /* insert figure here */
/*    if(get_token(input, epsfilename, FNAMELEN) == 0) {   */
    if(get_token(input, epsfilename, sizeof(epsfilename)) == 0) {  
/*      fputs(" File name missing in DVITPS special ", errout); */
      dvitopsmiss();
      errcount(0);
      flush_special(input);
      return -1;
    }
/*    see if width and height specified in dvitops: import */
    if(scan_special(input, line, MAXLINE) == MAXLINE) {
      complainspecial(input); /* TOO DAMN LONG ! */
    }
/*    if ((special = findepsfile(epsfilename, 1, "eps")) == NULL) */
    if ((special = findepsfile(epsfilename, "eps", 1, 0)) == NULL)
      return -1;
    pslength = checkpcform(special, epsfilename);   /* ??? */
/*    if (read_bbox(special, epsfilename, pslength) == 0) {
      fprintf(errout, "BoundingBox not found in %s ", epsfilename);
    }  */
    copyepsfileaux(output, special, epsfilename);
    fclose(special);
  }
  else if (strcmp(line, "Include0") == 0) { /* read PS header or such */
/*    if(get_token(input, epsfilename, FNAMELEN) == 0) {   */
    if(get_token(input, epsfilename, sizeof(epsfilename)) == 0) {  
      dvitopsmiss();
      errcount(0);
      flush_special(input);
      return -1;
    }
/*    if ((special = findepsfile(epsfilename, 1, "eps")) == NULL) */
    if ((special = findepsfile(epsfilename, "eps", 1, 0)) == NULL)
      return -1;
    copyepsfileaux(output, special, epsfilename);
    fclose(special);
  }
  else if (strcmp(line, "Literal") == 0) {
/*    putc('\n', output); */    /* 1994/June/25 */
    copy_string(output, input);  /* just copy what follows ! */
/*    putc('\n', output); */    /* 1993/June/3 */
  }
  else complainspecial(input);
  flush_special(input);
  if (abortflag) return -1;
  else return 0;
}

/* Special starts with `ps:' or `ps::' - lots of possibilities ... */
/* DVIPS style? - some DVIPS stuff not implemented */
/* Also DVIPS operates in left-handed coordinate system */
int readdvips (FILE *output, FILE *input)
{
  int c;
  double scale=1.0;
  char *s, *t;
  int flag;
  char epsfilename[FNAMELEN]="";

  c = getc(input); --nspecial;  /* peek at next character ps: or ps:: */

  if (c == ':') {   /* a second colon --- deal with ps:: type command */
/*  global | local | inline | asis | begin | end <user PS> ??? */
/*    (void) get_token(input, line, MAXLINE); */ 
/*    if (verbatimflag) strip_bracket(output, intput);  */
    fliteral = ftell(input);    /* remember where this was */
    nliteral = nspecial;      /* and how many bytes left */
    if (scan_special(input, line, MAXLINE) == MAXLINE) {
      if (verbatimflag) {     /* long ps:: type verbatim */
        fseek(input, fliteral, SEEK_SET);
        nspecial = nliteral;
        strip_bracket(output, input);   /* revscl - forscl ??? */
      }
/*      complainspecial(input); */      /* TOO DAMN LONG ! */
/*      else complainspecial(input); */   /* 1993/Jan/23 */
      else complainjflag(input);      /* 1994/Feb/2 */
    }
/*    check whether this is a startTexFig  */
    else if (pstagstyle == 0 &&   /* avoid repeat call to startTexFig */
        strstr(line, "startTexFig") != NULL) {
      if ((s = strstr(line, "[begin]")) != NULL) s +=  7;
      else s = line;
#ifdef ALLOWSCALE
        if (outscaleflag) rescaletexfig (s);
#endif
      verbout(output, s);
      pstagstyle=1;   /* note that we have begin for this */
    }
/*    check whether this is a endTexFig */
    else if (pstagstyle != 0 &&   /* avoid repeat call to endTexFig */
        strstr(line, "endTexFig") != NULL) {
      if ((s = strstr(line, "[end]")) != NULL) s +=  5;
      else s = line;
/*      verbout(output, s);  */
      endtexfig(output);      /* 1992/Nov/28 */
      pstagstyle=0;
    }
    else if (pstagstyle != 0 &&   /* otherwise makes no sense */ 
        strstr(line, "doclip") != NULL) {
/*      verbout(output, line); */
      doclip(output);       /* 1992/Nov/28 */
    }
    else if (pstagstyle != 0 &&   /* otherwise makes no sense */ 
      strstr(line, "rotate") != NULL) { /* 1992/Sep/28 */
      verbout(output, line);
    }
/*    another possibility ps:: <angle> rotate */
/*    maybe just copy the darn thing to the output ??? */
/*    This verbatim WITHOUT: revscl - forscl */
/*    else if (verbatimflag) verbout(output, line); */
    else verbout(output, line);   /* 98/Jun/5 */
/*    verbatim PostScript --- 1993/Aug/15 */
/*    else complainjflag(input); */ /* 98/Jun/5 */
    flush_special(input);  /* flush the rest if any */
  } /* end of code for ps:: case */
  else {    /*  deal with ps: plotfile or ps: overlay etc */
    (void) ungetc(c, input); nspecial++;
    fliteral = ftell(input);    /* remember where this was */
    nliteral = nspecial;      /* 1992/Sep/25 */ 
/*    if (get_alpha_token(input, line, MAXLINE) == 0) { */
/*    if (bSciWord) flag = get_token(input, line, MAXLINE); */
    if (bSciWord) flag = get_token(input, line, sizeof(line));
/*    else flag = get_alpha_token(input, line, MAXLINE); */
    else flag = get_alpha_token(input, line, sizeof(line));
    if (flag == 0) {        /* 1994/Apr/22 */
      showline("Premature end of special\n", 1);
      return -1;
    }
    if (strcmp(line, "plotfile") == 0) { /* deal with plotfile */
/*      while (get_alpha_token(input, line, MAXLINE) > 0) { */
/*  deal with filename | global | local | inline | asis ??? */
/*      (void) get_token(input, epsfilename, FNAMELEN); */ /* file name */
      (void) get_token(input, epsfilename, sizeof(epsfilename));  /* file name */
      if (pstagstyle != 0)  { /*  only if between [begin] & [end] */
/*        actually, now do nothing except copy the file ! */
        copyepsfile(output, epsfilename, 1, 0); /* incl & no shift */
      }
    } /* end of "plotfile" case */
    else if (strcmp(line, "overlay") == 0) { /* deal with overlay */
/*      while (get_alpha_token(input, line, MAXLINE) > 0) { */
/*  deal with filename | on | off ??? */
/*      (void) get_token(input, epsfilename, FNAMELEN); */  /* file name */ 
      (void) get_token(input, epsfilename, sizeof(epsfilename));   /* file name */
      if (pstagstyle != 0)  { /*  only if between [begin] & [end] */
/*        actually, now do nothing except copy the file ! */
/*        fprintf(output, "dviso "); ? */
        copyepsfile(output, epsfilename, 0, 0); /* over & no shift */
/*        putc('\n', output); */
      }
    }   /* end of "overlay" case */
    else if (strcmp(line, "epsfile") == 0) {     /* ArborText ? */
/*         strcmp(line, "EPSFILE") == 0  */
/*      if(get_token(input, epsfilename, FNAMELEN) > 0) { */ /* file name */
      if(get_token(input, epsfilename, sizeof(epsfilename)) > 0) {  /* file name */
        startspecial(output);
/* ArborText scale may be per mille */
/*        if(get_token(input, line, MAXLINE) > 0) { */ /* scale ? */ 
        if(get_token(input, line, sizeof(line)) > 0) { /* scale ? */
          if(sscanf(line, "%lg", &scale) > 0) {
            if (scale > 33.33) scale = scale/1000.0;
            sprintf(logline, "%lg dup scale \n", scale);
            PSputs(logline, output);
          }
        }
/*        copyepsfile(output, epsfilename, 1, 0); *//* inc & no shift */
        copyepsfile(output, epsfilename, 1, 1); /* inc & shift */
        endspecial(output); 
      }
      else complainspecial(input);
    }   /* end of "epsfile" case */
    else if (strcmp(line, "include") == 0) {    
/*      if(get_token(input, epsfilename, FNAMELEN) > 0) {   */
      if(get_token(input, epsfilename, sizeof(epsfilename)) > 0) {  
        if (pssvbstyle != 0) /* only if after startTexFig */
          copyepsfile(output, epsfilename, 0, 0); 
          /* no inc & no shift */
      }
      else complainspecial(input);
    }   /* end of "include" case */
    else if (strcmp(line, "psfiginit") == 0) {
/*      pssvbstyle = 1; */
/*    ignore - NOP ? (DVI2PS-SVB) */
    } /* ignoring "psfiginit" */
/*  deal with DV2PS-SVB literal */
    else if (strcmp(line, "literal") == 0) { /* DVI2PS-SVB */
/*      printf(" LITERAL: pssvbstyle %d", pssvbstyle); */
/*      if (verbatimflag != 0) {
        copy_string(output, input);  
      } */
      if (scan_special(input, line, MAXLINE) == MAXLINE) {
        if (verbatimflag != 0) { /* long ps: literal verbatim */
          fseek(input, fliteral, SEEK_SET);
          nspecial = nliteral;
          PSputc('\n', output);       // always on new line ?
          PSputs("revsclx ", output);
          if (preservefont) PSputs("currentfont ", output);
          copy_string(output, input);
          if (preservefont) PSputs("setfont ", output);
          PSputs("forsclx ", output); 
        }
        complainspecial(input); /* TOO DAMN LONG ! */
      }
      else if (pssvbstyle == 0 && 
        strstr(line, "startTexFig") != NULL) {
#ifdef ALLOWSCALE
        if (outscaleflag) rescaletexfig (line);
#endif
        verbout(output, line);
        pssvbstyle = 1;
      }
      else if (pssvbstyle != 0 && strstr(line, "doclip") != NULL) {
/*        verbout(output, line); */
        doclip(output);       /* 1992/Nov/28 */
      }
      else if (pssvbstyle != 0 && 
        strstr(line, "endTexFig") != NULL) {
/*        verbout(output, line); */
        endtexfig(output);          /* 1992/Nov/28 */
        pssvbstyle = 0;
      }
      else if (verbatimflag) {
/*        fprintf(output, "\nrevscl ");   */ /* ? */
        verbout(output, line);
/*        fprintf(output, "forscl\n");  */ /* ? */
      }
      else complainjflag(input);      /* 93/Oct/17 */
/*      else complainspecial(input); */
/*      if (pssvbstyle == 0) fprintf(output, "\nrevscl\n");  */
/*      copy_string(output, input);   */
/*      if (pssvbstyle == 0) fprintf(output, "forscl\n");  */
    } /* end of "literal" case */
/*    could check for presence of : or / here */
/*    how does SciWord deal with scaling ? */
    else if (bSciWord != 0) { /* just has the EPS file name here */
/*      if (get_token(input, epsfilename, FNAMELEN) > 0) {   */
      if (strcmp(line, "") != 0 && strlen(line) < sizeof(epsfilename)) {
        strcpy(epsfilename, line);
        startspecial(output);
        copyepsfile(output, epsfilename, 1, -1); /* inc & shift */
        endspecial(output); 
      }
      else complainspecial(input);  /* 1994/April/21 experiment */
    }
/*    token after ps: is not recognized, must be raw PostScript */
/*    else if (verbatimflag != 0) { */
/*    treat as raw PostScript - no longer require -j */
/*    and if bOldDVIPSFLag == 0 treat just like ps:: above */
/*    except flip y axis to deal with rotate problem */
    else {          /* 1992/Sep/25 - so DVIPS manual runs */
      if (bOldDVIPSFlag) {        /* 96/Oct/4 */
        PSputc('\n', output);       // always on new line ?
        PSputs("revsclx ", output);
        if (preservefont) PSputs("currentfont ", output);
      }
      else if (bOldDVIPSFlip) {     /* 1996/Nov/7 */
        PSputc('\n', output);
        PSputs(tranCP, output);
        PSputs("1 -1 scale ", output);
        PSputs(untranCP, output);
      }
      PSputc('\n', output);     /* 1994/June/25 */
      freshflag = 1;      // ?

      fseek(input, fliteral, SEEK_SET);
      nspecial = nliteral;
      if (bOldDVIPSFlag || bOldDVIPSFlip) { /* the old way */
        copy_string(output, input);  
      }
      else {              /* new 96/Nov/11 */
        if (scan_special(input, line, MAXLINE) != MAXLINE) {
          t = s = line;
/* precede each `rotate' by `neg' */
          while ((s = strstr(t, "rotate")) != NULL) {
            *s = '\0';
//            fputs(t, output);
            PSputs(t, output);
            *s = 'r';
/* check that rotate is token, not part of longer string */
            if ((s == t || *(s-1) <= ' ') &&
              (s+6 == line + MAXLINE || *(s+6) <= ' ')) {
//              fputs("neg ", output);
              PSputs("neg ", output);
            }
//            fputs("rotate", output);
            PSputs("rotate", output);
            t = s+6;
          }
//          fputs(t, output);   /* final piece */
          PSputs(t, output);    /* final piece */
        }
        else {    /* it was too long - so just copy the damn thing */
          fseek(input, fliteral, SEEK_SET);
          nspecial = nliteral;
          copy_string(output, input);
        }
      }
      if (bOldDVIPSFlag) {      /* 94/Oct/4 */
        if (preservefont) {
//          fputs("setfont ", output);
          PSputs("setfont ", output);
        }
/*        fputs("forscl ", output); */  /* ? */
        PSputs("forsclx ", output); 
      }
      else if (bOldDVIPSFlip) {   /* 1996/Nov/7 */
        PSputc('\n', output);
        PSputs(tranCP, output);
        PSputs("1 -1 scale ", output);
        PSputs(untranCP, output);
//        PSputc('\n', output);     /* 1993/June/3 */
      }
      else PSputc('\n', output);      /* 1993/June/3 */
    }
/*    else complainspecial(input); */
  }
  flush_special(input);
  return 0;
}

/* support \special{src123file.tex} */

void DoScr (FILE *output, FILE *input)     /* 98/Nov/4 */
{
  int lineno=0;
  char srcfile[FILENAME_MAX]="";

  (void) scan_special(input, line, MAXLINE); /* read rest of line */
  if (sscanf(line, "%d%s", &lineno, srcfile) > 1) {
    if (stripcomment == 0) {
      PSputc('\n', output); // always on new line ?
      sprintf(logline, "%% l.%d %s\n", lineno, srcfile);
      PSputs(logline, output);
    }
    showcount = 0; 
  }
}

/* Terminated by :  =>  ArborText style special or DVITOPS or DVITPS or EM */
/* Separator is `:' */ /* or HP TAG tiff: */

int readdvilaserps (FILE *output, FILE *input)
{
  if (strcmp(line, "ps") == 0) readdvips(output, input);
  else if (strcmp(line, "dvitops") == 0) readdvitops(output, input, 0);
  else if (strcmp(line, "eps") == 0) readdvitops(output, input, 1);
  else if (strcmp(line, "dvitps") == 0) readdvitps(output, input);
  else if (strcmp(line, "em") == 0) reademtex(output, input);
  else if (strcmp(line, "tiff") == 0) dohptag(output, input);
/*  Is it one of DVIWINDO's specials ? */ /* colon after `color' 95/June/21 */
  else if (strcmp(line, "color") == 0) doColor(output, input, ':', 1);
  else if (strcmp(line, "clip") == 0) doClipBox(output, input, ':');
  else if (strcmp(line, "comment") == 0) flush_special(input); /* 95/June/21*/
  else if (strcmp(line, "PDF") == 0) flush_special(input);   /* 95/July/4 */
#ifdef AllowCTM
  else if (strcmp(line, "CTM") == 0) doCTM(output, input);  /* 95/Oct/10 */
#endif
  else if (strcmp(line, "message") == 0) {  /* 1995/June/21 */
    showline(" ", 0);
    (void) scan_special(input, line, MAXLINE);
    showline(line, 0);
  }
#ifdef TIFF
  else if (newspecials (output, input) != 0) ; /* in dvitiff.c */
#else 
  else if (strcmp(line, "textcolor") == 0 ||
    strcmp(line, "rulecolor") == 0 ||
    strcmp(line, "figurecolor") == 0 ||
    strcmp(line, "reversevideo") == 0 ||
    strcmp(line, "button") == 0 ||
    strcmp(line, "mark") == 0 ||      
    strcmp(line, "insertimage") == 0) { 
      flush_special(input);
      if (quietflag == 0)
        showline(" Ignoring DVIWindo \\special", 1);
/*    strcmp(line, "insertmf") == 0) {} */ /* error in DVIPSONE */
  }
#endif
  else if (strcmp(line, "revset") == 0) { /* HP TAG revset: 4pt \special */
    flush_special(input);
    if (traceflag) showline(" Ignoring revset", 1);
  }
  else if (strcmp(line, "src") == 0) {  /* 98/Nov/4 */
    if (stripcomment == 0)  DoScr(output, input);
    else flush_special(input);
  }
/*  Try Andrew Treverrow file inclusion, but only if -j used */
  else if (verbatimflag && readandrew(output, input)) {  /*  try OzTeX ? go south ! */
    if (abortflag) return -1;
  }
  else complainspecial(input);  /* don't understand! */
  return 0;
}

#ifdef TPIC

/*  * 0.001  => in */ /*  * 72.27 => points */ /* * 65536 => scaled points */

long convscal (long z) /* convert to scaled point from 0.001 inch */
{
/*  return (long) ((double) z * 0.001 * 72.27 * 65536); */
/*  return (long) ((double) z * 4736.28672);  */
  return z * 4736;  /* avoid round-off error accumulation & floating */
}

long convscaldouble (double z)  /* as above, but for positive double */
{
  if (z < 0.0) return - (long) (-z * 4736.0 + 0.5);
  else return (long) (z * 4736.0 + 0.5);
}

long xold=0, yold=0;      /* last point placed in path */

#define MINMILLI 1.0      /* smallest measure in milli-inches */

// Now adds a space at end in case raw PS follows from \special 2000/Feb/16
// On entry contains TPIC command token in line
// If it takes arguments, scan_special is used to read in the rest

int readtpic (FILE *output, FILE *input)
{
  long x=xold, y=yold;
  double z=0.0;
  long xr=0, yr=0;
  double sa=0.0, ea=0.0;
  double r, g, b, grey=0.0;
  double sad, ead, dif;
  int bitsone, bitstot;
  int c, k;
  char *s;
  char temp[3];
  char bits[17]="0112122312232334"; /* "0123456789ABCDEF" */

//  check whether it could be TPIC command
  if (strlen(line) != 2) return 0;          /* quick exit */
/*  if (strstr(tpiccommands, line) == NULL) return 0; */ /* quick exit */

/*  maybe only if actually needed ? *//*  omit for `fp', ... ? */
//  putc('\n', output);
  PSputc('\n', output);

  if (strcmp(line, "pn") == 0) {  /* pn n - set pen width */
    (void) scan_special(input, line, MAXLINE);
    if (sscanf (line, "%lg", &z) < 1) { /* complain if < 1 ? */
    }
/*    pen width NORMALLY is in milli-inches */
    if (z > -MINMILLI && z < MINMILLI)  z = z * 1000.0; /* arg in inches */
/*    warn if pen-width is zero ? */
#ifdef ALLOWSCALE
    if (outscaleflag) {
      sprintf(logline, "%.9lg pn", (double) convscaldouble(z) / outscale);
      PSputs(logline, output);
    }
    else
#endif
    sprintf(logline, "%ld pn", convscaldouble(z));
    PSputs(logline, output);
  }
  else if (strcmp(line, "pa") == 0) { /* pa x y - add point to path */
    (void) scan_special(input, line, MAXLINE);
    if (sscanf (line, "%ld %ld", &x, &y) < 2) { /* complain if < 2 ? */
    }
#ifdef ALLOWSCALE
    if (outscaleflag) {
      sprintf(logline, "%.9lg %.9lg pa",
        (double) convscal(x - xold) / outscale,
          (double) convscal(y - yold) / outscale);
      PSputs(logline, output);
    }
    else
#endif
    {
      sprintf(logline, "%ld %ld pa", convscal(x - xold), convscal(y - yold));
      PSputs(logline, output);
    }
    xold = x; yold = y;
  }
  else if (strcmp(line, "da") == 0 ||
         strcmp(line, "dt") == 0) { /* da/dt l - stroke dashed/dotted */
    strcpy(temp, line);
    (void) scan_special(input, line, MAXLINE); /* complain if < 1 ? */
    if (sscanf (line, "%lg", &z) < 1) {
    }
/*    dash/dot interval NORMALLY is in inches */
    if (z > -MINMILLI && z < MINMILLI)  z = z * 1000.0; /* arg in inches */
/*    warn if dot/dash length is zero ? */
#ifdef ALLOWSCALE
    if (outscaleflag) {
      sprintf(logline, "%.9lg %s",
          (double) convscaldouble(z) / outscale, temp);
      PSputs(logline, output);
    }
    else
#endif
    {
      sprintf(logline, "%ld %s", convscaldouble(z), temp);
      PSputs(logline, output);
    }
    xold = 0; yold = 0;       /* reset path */
  }
  else if (strcmp(line, "sp") == 0) { /* sp [l] - stroke quadratic spline */
    (void) scan_special(input, line, MAXLINE);
    if (sscanf (line, "%lg", &z) > 0) {   // has an argument ?
/*      dash/dot interval NORMALLY is in inches */
      if (z > -MINMILLI && z < MINMILLI)  z = z * 1000.0; /* inches */
/*      warn if dot/dash length is zero ? */
#ifdef ALLOWSCALE
      if (outscaleflag) {
        sprintf(logline, "%.9lg sp",
            (double) convscaldouble(z) / outscale);
        PSputs(logline, output);
      }
      else
#endif
      {
        sprintf(logline, "%ld sp", convscaldouble(z));
        PSputs(logline, output);
      }
    }
    else {              // no argument
//      fputs("spsld", output);   /* solid spline case */
      PSputs("spsld ", output);   /* solid spline case */
    }
    xold = 0; yold = 0;       /* reset path */
  }
  else if (strcmp(line, "ar") == 0 ||
         strcmp(line, "ia") == 0) { /* ar/ia x y xr yr sa ea */
    strcpy(temp, line);       // either "ar" or "ia"
    (void) scan_special(input, line, MAXLINE);
    sa = 0.0; ea = 2.0 * PI;      /* in case angles omitted */
    if (sscanf(line, "%ld %ld %ld %ld %lg %lg",
      &x, &y, &xr, &yr, &sa, &ea) < 6) {
    }
/*  allow for dotted/dashed ellipses ? */
    sad = CONVDEG(sa); ead = CONVDEG(ea);
/*  round off angles for complete circle */
    dif = (ead - sad) - 360.0;
    if (dif > -0.02 && dif < 0.02 ) ead = sad + 360.0;
#ifdef ALLOWSCALE
    if (outscaleflag) {
      sprintf(logline, "%.9lg %.9lg %.9lg %.9lg %lg %lg %s",
        (double) convscal(x) / outscale, (double) convscal(y) / outscale,
          (double) convscal(xr) / outscale, (double) convscal(yr) / outscale,
            sad, ead, temp);
      PSputs(logline, output);
    }
    else
#endif
    {
      sprintf(logline, "%ld %ld %ld %ld %lg %lg %s",
          convscal(x), convscal(y), convscal(xr), convscal(yr),
/*        convdeg(sa), convdeg(ea),line); */
/*        CONVDEG(sa), CONVDEG(ea), temp); */
        sad, ead, temp);
      PSputs(logline, output);
    }
  }
  else if (strcmp(line, "sh") == 0 ||
    strcmp(line, "cl") == 0) { /* sh [s] */
    (void) scan_special(input, line, MAXLINE);
/*    deal with color specification using three values ? */
/*    grey = 0.5; */
    if (sscanf(line, "%lg %lg %lg", &r, &g, &b) < 3) {
      if (sscanf (line, "%lg", &grey) < 1) {
//        fputs("gr", output);    /* no shade specified use 0.5 */
        PSputs("gr ", output);    /* no shade specified use 0.5 */
      }
      else {
        if (grey > 1.0) grey = grey / 255.0;  /* 1992/Dec/12 */
        sprintf(logline, "%lg sh", 1.0 - grey);
        PSputs(logline, output);
      }
    }
    else {
      if (r > 1.0) r = r / 255.0;
      if (g > 1.0) g = g / 255.0;
      if (b > 1.0) b = b / 255.0;
      sprintf(logline, "%lg %lg %lg cl", 1-r, 1-g, 1-b); 
      PSputs(logline, output);
/*      grey = (r + r + g + g + g + b) / 6.0; */
/*      if (grey > 1.0) grey = grey / 255.0; */
/*      fprintf(output, "%lg sh", grey); */
    }
  }
  else if (strcmp(line, "fp") == 0 ||
       strcmp(line, "ip") == 0) { /* fp/ip */
    xold = 0; yold = 0;       /* reset path */
//    fputs(line, output);
    PSputs(line, output);
  }
  else if (strcmp(line, "wh") == 0 ||
       strcmp(line, "bk") == 0) { /* wh/bk */
//    fputs(line, output);
    PSputs(line, output);
  }
  else if (strcmp(line, "tx") == 0) { /* tx */
    (void) scan_special(input, line, MAXLINE);
/* need to count bits in hexadecimal mask pattern here */
    bitsone=0; bitstot=0;
    s = line;
    while ((c = *s++) != 0) {
      if (c >= '0' && c <= '9') k = c - '0';
      else if (c >= 'A' && c <= 'Z') k = c + 10 - 'A';
      else if (c >= 'a' && c <= 'z') k = c + 10 - 'a';
      else k = 0;
      if (k < 16) bitsone += bits[k] - '0';
      bitstot += 4;
    }
    sprintf(logline, "%lg sh", (double) bitsone / bitstot);
    PSputs(logline, output);
  }
  else return 0;      /* not a TPIC special */
  PSputc(' ', output);      // 2000/Feb/15 
  if (pagetpic++ == 0) {
    if (verboseflag) showline(" TPIC", 0);    /* debugging */
  }
  if (needtpic == 0) 
    showline(" ERROR: TPIC failure\n", 1); /* debugging */
  flush_special(input);  /* clean out the rest */
  return -1;
}

#endif

/* read and analysize special command called from dvianal.c xxx */

int readspecial (FILE *output, FILE *input, unsigned long ns)
{
  int c;

  if (bIgnoreSpecials != 0) { /* ignore \specials ? */
    flush_special(input);
    return 0;
  }

  nspecial = (long) ns;     /* assuming we never have 2Gbyte file ! */
  nspecialsav = nspecial;     /* save length for error message output */
  specstart = ftell(input);   /* save start for error message */
  if (traceflag) {
    sprintf(logline, "\nSpecial of length %ld at %ld\n", nspecial, specstart);
    showline(logline, 0);
  }

/*  look at first byte */
  c = getc(input);
  --nspecial;
  if (c == 0 && bFirstNull) {   /* flush if first byte is null 96/Aug/29 */
    flush_special(input);
    return 0;
  }

/*  support for `literal graphics' kludge in dvips - start with " */
  if (c == '\"') {   /*  && verbatimflag != 0 */  /* took out 1992/Oct/12 */
    PSputc('\n', output);
    PSputs("save undsclx ", output);        /* 97/Apr/25 ? */
/*    so that we can safely include gnuplot garbage 97/Mar/9 */
    if (bQuotePage) 
      PSputs("/showpage{}def ", output);  /* 97/Mar/9 */
    if (bProtectStack) PSputs("[ ", output);      /* 97/Nov/24 */
    if (bStoppedContext) PSputs("{ ", output);    /* 97/Nov/24 */
    copy_string(output, input);
    if (bStoppedContext) PSputs(" } stopped", output);  /* 97/Nov/24 */
    if (bProtectStack) PSputs(" cleartomark", output);  /* 97/Nov/24 */
    PSputs(" restore", output);       /* 93/Oct/17 */
    PSputc('\n', output);
    freshflag = 1;    // ?
    return 0;
  }

/*  support for `literal macros' kludge in dvips - start with ! */
  if (c == '!') {               /* 94/Jan/28 */
/*    ignore now, already taken care of in dvipslog ! as headertext=... */
    flush_special(input);
    return 0;
  }         

  (void) ungetc(c, input);      /* put back the first byte */
  nspecial++;
  if ((c = get_alpha_token(input, line, sizeof(line))) == 0)
  {
    if (quietflag == 0) showline(" Blank special", 1);
    return 0;         /* found nothing ! */
  }
/*  if (traceflag) sprintf(logline, "token: %s, terminator: %c\n", line, c); */

  if (strncmp(line, "if", 2) == 0) {  /* new conditional option 99/July/2 */
    if (strcmp(line+2, "print") != 0) { /* ignore all but ifprint */
      flush_special(input);  /* not for DVIPSONE */
      return 0;
    }
    else if ((c = get_alpha_token(input, line, sizeof(line))) == 0) { /* recurse */
      if (quietflag == 0) showline(" Blank special", 1);
      return 0;         /* found nothing ! */
    }
  } /* drop through here normally */

/*  Try and recognize style from terminator of first token and first token */
  if (c == '=') readdvi2ps(output, input);      /* DVI2PS style */
  else if (c == ':') readdvilaserps(output, input); /* DVIPS style ? */
/*  If not = or :, then try Textures styles (' ' terminated) */
/*  but do this before trying TPIC or DVIALW styles ... */
  else if (strcmp(line, "illustration") == 0) readtextures(output, input);
  else if (strcmp(line, "postscript") == 0) copypostscript(output, input, 0);
  else if (strcmp(line, "rawpostscript") == 0) {  /* 1994/July/3 */
    if (verbatimflag) copypostscript(output, input, 1);
    else complainjflag(input);
  }
  else if (strcmp(line, "postscriptfile") == 0) 
    readpostscript(output, input, 0);
  else if (strcmp(line, "rawpostscriptfile") == 0) { /* 1994/July/3 */
    if (verbatimflag) readpostscript(output, input, 0);
    else complainjflag(input);
  }
  else if (strcmp(line, "picture") == 0) readpicture(output, input);
  else if (strcmp(line, "color") == 0) doColor(output, input, c, 1); 
  else if (strcmp(line, "clip") == 0) doClipBox(output, input, c); /* 98/Sep/12 */
  else if (strcmp(line, "background") == 0) /* used only during prescan ? */
    flush_special(input);    /* 98/Jun/30 */
  else if (strcmp(line, "landscape") == 0)
/*    texlandscape(output); */  /* no, used only in prescan foils.cls */
    flush_special(input);    /* 99/Apr/5 */
/*  have to do DVIPS / Texture style FIRST, or they get caught by following: */
  else if (c == ' ' && nspecial > 0) {
    if (allowtpic == 0 || readtpic(output, input) == 0) {
      readdvialw(output, input); /* DVIALW */
    }
  }
  else if (verbatimflag && readandrew(output, input)) { /*  try OzTeX ? go south ! */
    if (abortflag) return -1;
  }
  else if (allowtpic && readtpic(output, input)) ;
  else complainspecial(input);      /* don't understand special! */
/*  else if (verbatimflag == 0 || 
       readandrew(output, input) == 0) { 
    if (allowtpic == 0 || readtpic(output, input) == 0) {
      complainspecial(input);
    }
  } */
  if (abortflag) return -1;
  return 0;
}

/* Use this while skipping pages *//* used just to keep track of color stack */
/* all other \specials are ignored *//* added 98/Feb/14 */

void prereadspecial (FILE *input, unsigned long ns)
{
  int c;

  if (bIgnoreSpecials != 0) { /* ignore \specials ? */
    flush_special(input);
    return;
  }

  nspecial = (long) ns;     /* assuming we never have 2Gbyte file ! */
  nspecialsav = nspecial;     /* save length for error message output */
  specstart = ftell(input);   /* save start for error message */
  if (traceflag) {
    sprintf(logline, "\nSpecial of length %ld at %ld\n", nspecial, specstart);
    showline(logline, 0);
  }

  c = getc(input); --nspecial;  /* look at first character */
  while (c == ' ') {        /* flush leading spaces - 1999/Apr/23 */
    c = getc(input); --nspecial;
  }

  if (c == 0 && bFirstNull) {   /* flush if first byte is null 96/Aug/29 */
    flush_special(input);
    return;
  }
/*  support for `literal graphics' kludge in dvips - start with " */
  if (c == '\"') {  
    flush_special(input);
    return;
  }
/*  support for `literal macros' kludge in dvips - start with ! */
  if (c == '!') { 
    flush_special(input);  /* taken care of in dvipslog */
    return;
  }         
  (void) ungetc(c, input); nspecial++;  /* put it back */
  if ((c = get_alpha_token(input, line, sizeof(line))) == 0) {
    return;         /* found nothing ! */
  }
  if (c == ' ' || c == ':') {
    if (strcmp(line, "color") == 0) { /* color: or color */
      doColor(NULL, input, c, 0);   /* no PS output */
    }
  }
  else flush_special(input);
  return;
}

/* moved down here to try and avoid compiler bug ! */

/* em:message xxx --- output message xxx right away on console */
/* em:linewidth w ---- set linewidth for subsequent lines 0.4pt default */
/* em:moveto --- remember current point as start for next line */
/* em:lineto --- draw line from previous moveto or lineto */

/* em:graph xxx ---- MSP or PCX image top-left corner at current point */
/* em:point n --- remember current coordinates for point n */
/* em:line a[h|v|p],b[h|v|p][,w] draw line thickness w from point a to b */

void reademtex (FILE *output, FILE *input)
{
  double linewidth, multiple;
  long emline = 26214;          /* default 0.4pt * 65536 */
  char *s;
  char units[3];
  int n;

/*  if (get_alpha_token(input, line, MAXLINE) == 0) { */
  if (get_alpha_token(input, line, sizeof(line)) == 0) {
    complainspecial(input);
    return;
  }
  if (strcmp(line, "message") == 0) {
    showline(" ", 0);
    (void) scan_special(input, line, MAXLINE);
    showline(line, 0);
  }
  else if (strcmp(line, "linewidth") == 0) {
    (void) scan_special(input, line, MAXLINE);
    s = line;
    if (sscanf(s, "%lg%n", &linewidth, &n) == 0) {
      showline("linewidth not specified", 0); /* used default */
    }
    else {
      s +=  n;
      units[0] = *s++; units[1] = *s++; units[2] = '\0';
      multiple = decodeunits (units);   /* conversion to PS pt's */
      linewidth = linewidth * multiple * (72.27 / 72.0) * 65536.0;
      emline = (long) (linewidth + 0.5);  /* convert to scaled pt's */
    }
    PSputc('\n', output);
#ifdef ALLOWSCALE
    if (outscaleflag) {
      sprintf(logline, "%.9lg emw", (double) emline / outscale);
      PSputs(logline, output);
    }
    else
#endif
    {
      sprintf(logline, "%ld emw", emline);
      PSputs(logline, output);
    }
    showcount = 0;    
  }
  else if (strcmp(line, "moveto") == 0) {
    PSputs(" emm", output);
    showcount++;
  }
  else if (strcmp(line, "lineto") == 0) {
    PSputs(" eml", output);
    showcount++;
  }
  else complainspecial(input);
  if (showcount >=MAXSHOWONLINE) {
    PSputc('\n', output);
    freshflag = 1;    // ?
    showcount = 0;
  }
}

/****************************************************************************/

/*  39  => 144 */
/*  96  => 143 */

unsigned char dos850topdf[] = { /* starts at 128 */
199, /* 128 % Ccedilla */
252, /* 129 % udieresis */
233, /* 130 % eacute */
226, /* 131 % acircumflex */
228, /* 132 % adieresis */
224, /* 133 % agrave */
229, /* 134 % aring */
231, /* 135 % ccedilla */
234, /* 136 % ecircumflex */
235, /* 137 % edieresis */
232, /* 138 % egrave */
239, /* 139 % idieresis */
238, /* 140 % icircumflex */
236, /* 141 % igrave */
196, /* 142 % Adieresis */
197, /* 143 % Aring */
201, /* 144 % Eacute */
230, /* 145 % ae */
198, /* 146 % AE */
244, /* 147 % ocircumflex */
246, /* 148 % odieresis */
242, /* 149 % ograve */
251, /* 150 % ucircumflex */
249, /* 151 % ugrave */
255, /* 152 % ydieresis */
214, /* 153 % Odieresis */
220, /* 154 % Udieresis */
248, /* 155 % oslash */
163, /* 156 % sterling */
216, /* 157 % Oslash */
215, /* 158 % multiply */
134, /* 159 % florin */
225, /* 160 % aacute */
237, /* 161 % iacute */
243, /* 162 % oacute */
250, /* 163 % uacute */
241, /* 164 % ntilde */
209, /* 165 % Ntilde */
170, /* 166 % ordfeminine */
186, /* 167 % ordmasculine */
191, /* 168 % questiondown */
174, /* 169 % registered */
172, /* 170 % logicalnot */
189, /* 171 % onehalf */
188, /* 172 % onequarter */
161, /* 173 % exclamdown */
171, /* 174 % guillemotleft */
187, /* 175 % guillemotright */
0,   /* 176 % ltshade  */
0,   /* 177 % shade */
0,   /* 178 % dkshade */
0,   /* 179 % 2502 */
0,   /* 180 % 2524 */
193, /* 181 % Aacute */
194, /* 182 % Acircumflex */
192, /* 183 % Agrave */
169, /* 184 % copyright */
0,   /* 185 % 2563 */
0,   /* 186 % 2551 */
0,   /* 187 % 2557 */
0,   /* 188 % 255d */
162, /* 189 % cent */
165, /* 190 % yen */
0,   /* 191 % 2510 */
0,   /* 192 % 2514 */
0,   /* 193 % 2534 */
0,   /* 194 % 252c */
0,   /* 195 % 251c */
0,   /* 196 % 2500 */
0,   /* 197 % 253c */
227, /* 198 % atilde */
195, /* 199 % Atilde */
0,   /* 200 % 255a */
0,   /* 201 % 2554 */
0,   /* 202 % 2569 */
0,   /* 203 % 2566 */
0,   /* 204 % 2560 */
0,   /* 205 % 2550 */
0,   /* 206 % 256c */
164, /* 207 % currency */
240, /* 208 % eth */
208, /* 209 % Eth */
202, /* 210 % Ecircumflex */
203, /* 211 % Edieresis */
200, /* 212 % Egrave */
154, /* 213 % dotlessi */
205, /* 214 % Iacute */
206, /* 215 % Icircumflex */
207, /* 216 % Idieresis */
0,   /* 217 % 2518 */
0,   /* 218 % 250c */
0,   /* 219 % block */
0,   /* 220 % dnblock */
166, /* 221 % brokenbar */
204, /* 222 % Igrave */
0,   /* 223 % upblock */
211, /* 224 % Oacute */
223, /* 225 % germandbls */
212, /* 226 % Ocircumflex */
210, /* 227 % Ograve */
245, /* 228 % otilde */
213, /* 229 % Otilde */
181, /* 230 % mu */
254, /* 231 % thorn */
222, /* 232 % Thorn */
218, /* 233 % Uacute */
219, /* 234 % Ucircumflex */
217, /* 235 % Ugrave */
253, /* 236 % yacute */
221, /* 237 % Yacute */
175, /* 238 % macron */
180, /* 239 % acute */
0,   /* 240 % sfthyphen */
177, /* 241 % plusminus */
0,   /* 242 % underscoredbl */
190, /* 243 % threequarters */
247, /* 246 % divide */
184, /* 247 % cedilla */
176, /* 248 % degree */
168, /* 249 % dieresis */
183, /* 250 % periodcentered */
185, /* 251 % onesuperior */
179, /* 252 % threesuperior */
178, /* 253 % twosuperior */
0,   /* 254 % filledbox */
0,   /* 255 % nbspace */
};

/* The owner information is in DOS 850 encoding --- */
/* translate to PDF encoding *and* use \xyz octal notation where needed */
/* to make sure PS output is 7-bit safe */

/* Currently Distiller does not support PDF Encoding in the DocInfo fields */

void map850topdf (char *buffer, int nlen) {   /* 97/May/24 */
  char *s=buffer;
  int c, n, i, j, k;

  while ((c = (unsigned char) *s) != '\0') {
    if (c > 127) c = dos850topdf[c-128];
    else if (c == 39) c = 144;  /* quoteright */
    else if (c == 96) c = 143;  /* quoteleft */
    if (c == 0) *s = ' ';   /* suppress `bad' characters */
    if (c > 127) {
      n = strlen(s);
      if (n + (s - buffer) + 4 >= nlen-1) break;  /* out of space */
      memmove(s+3, s, n+1); /* make space for \xyz in octal */
      *s++ = '\\';
      k = c & 7;
      j = (c >> 3) & 7;
      i = (c >> 6) & 7;
      *s++ = (char) (i + '0');
      *s++ = (char) (j + '0');
      *s++ = (char) (k + '0');
    }
    else s++;
  }
}

/* Note: in the above, characters that do not appear in PDF encoding are */
/* not translated (for these dos850topdf[c-128] == 0) */
/* but these do not appear in customization string in any case */

/****************************************************************************/

/* two different literals ? one relative to present position, one absolute */

/* might use fgets and fputs if trust line lengths in included file */

/* skip over header of stupid PC EPS files OK */
/* avoid trailer of stupid PC EPS files HMMM */

/* move some stuff setting up special to preamble ? */

/* disable other things ? */ 

/* protect more in special - against flushfile for example */
/* protect better against nasty EPS files */

/* strip out control D at end of inserted file OK */

/* make resistant to junk left on stack by inserted file OK */

/* may not need to be able to read BBox info ? OOPS: */
/* need to be able to read BBox info for Textures/Freehand inclusions */
/* What is this DeviceFill bullshit (Steve Ward special kludge) ? */

/* might use getrealline to strip out comments from included file ? */
/* allow stripping comments out of included eps files ? */
/* but potential problems with % buried in strings ! */
/* watch out for long lines ! */
/* presently strips out comment lines starting with % => smartcopy */

/* implement PCTeX specials also ? */

/* look in same directory as DVI for inserted figures before giving up ? */
/* maybe also look in current directory if you get desperate ? */

/* decide whether to try and implement DVIPS style specials NO */

/* present clean stack to inserted file ? */ /* invalidrestore */

/* psfile=<file-name> llx=<llx> lly=<lly> urx=<urx> ury=<ury> rwi=<rwi> */

/* Theoretically should check included EPS for fonts needed */
/* - and then add these to the fonts needed here */
/* - but this would require additional passes through everything! */

/* Presently not using PS length code in EPSF file format */

/* allow for \special{ps: plotfile logo.eps scaled 760} ? conflict ? */

/* Also:  \special{ps: epsfile <file-name> <scale>} */

/* Also:  \special{header=<file-name>} */

/* Also:  \special{verbatim="<PS-code>"} */

/* Also:  DVI2PS (Tom Li)
      pstext="<w> <h> <llx> <lly> <urx> <ury> startTexFig"
      pstext="doclip"
      psfile=<prolog-file>
      psfile=<epsf-file>      
      psfile=<postlog-file>     
      pstext=endTexFig */

/* Also:  dvi2ps-svb (Stephan Bechtolstein)
      ps: psfiginit
      ps: literal <w> <h> <llx> <lly> <urx> <ury> startTexFig
      ps: literal "doclip "
      ps: include <prolog-file>
      ps: include <epsf-file>
      ps: include <postlog-file>
      ps: literal "endTexFig " */

/* Also:  DVIPS (Rokicki) & ArborText ?
      ps::[begin] <w> <h> <llx> <lly> <urx> <ury> startTexFig
      ps:: doclip
      ps: plotfile <prolog-file>
      ps: plotfile <epsf-file>
      ps: plotfile <postlog-file>
      ps::[end] endTexFig */

/* check that this all works if /magnification/magstep1 !!! */

/* Andrew Trevorrow's OzTeX for MacIntosh ? */
/* Andrew Trevorrow's Psprint for VaX VMS ? */

/* \special{<eps-file> <x-scale> <y-scale> scale} */

/* OzTeX does not apply magnification to inserted PS ! Ugh */
/* OzTex uses 300 units per inch default ? Ugh ! */

/* special file should be opened in "rb" mode, so seek positioning accurate */ 

/* possible problem with userdict on top when calling: */
/* dvispend - OK fixed */
/* endTexFig - OK fixed */
/* doclip - OK fixed */
/* TeXlandscape - OK fixed */

/* undscl puts origin at current point and */
/* undscl goes to underlying PS coordinate system */
/* undsclx goes to underlying PS coordinate system if there was no mag */

/* revscl puts origin at default PS coordinate origin */
/* revscl goes to underlying PS coordinate system */
/* revsclx goes to underlying PS coordinate system if there was no mag */

/* forscl goes to TeX coordinate system origin */
/* forscl goes to TeX coordinates including magnification */
/* forsclx goes to TeX coordinates as if there was no mag */

/* Use `non x' versions if you need the actual PS coords (e.g. pdfmarks) */
/* Use `x' versions if need to get expected behaviour when mag changes */

/* NOTE: use -*d to get color continuity across page boundaries */
/* Result is page-independent PS 98/Feb/14 */
/* Don't do page subranges (fix by reading \special when skipping) */
/* Don't do reverse order printing or collated copies */
