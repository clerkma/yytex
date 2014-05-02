/* Copyright 2007 TeX Users Group
   Copyright 2014 Clerk Ma   

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

#ifdef _WINDOWS
  #define NOCOMM
  #define NOSOUND
  #define NODRIVERS
  #define STRICT
  #pragma warning(disable:4115) // kill rpcasync.h complaint
  #include <windows.h>
  #define MYLIBAPI __declspec(dllexport)
#endif

#pragma warning(disable:4996)
#pragma warning(disable:4131) // old style declarator
#pragma warning(disable:4135) // conversion between different integral types 
#pragma warning(disable:4127) // conditional expression is constant

#include <kpathsea/config.h>
#include <kpathsea/c-ctype.h>
#include <kpathsea/line.h>
#include <kpathsea/readable.h>
#include <kpathsea/variable.h>
#include <kpathsea/absolute.h>

#include <setjmp.h>

#define EXTERN extern

#include "texd.h"

/* Most Y & Y changes are localized here -- init() */

#define USEOUREALLOC      /* 96/Jan/20 */

#define USEMEMSET       /* 98/Jan/23 */

#ifndef _WINDOWS
/* #define HEAPWALK */      /* debugging 96/Oct/22 */
#endif

#ifdef USEOUREALLOC
  #define REALLOC ourrealloc
#else
  #define REALLOC realloc
#endif

#include <time.h>
#include <malloc.h> /* _msize, _expand, HEAPOK et.c */
#include <direct.h> /* for _getcwd() */

#pragma warning(disable:4032) // different type when promoted
#ifndef _WINDOWS
  #include <conio.h>            /* for _getch() */
#endif
#pragma warning(default:4032) // different type when promoted


/* Argument handling, etc.  */ /* from common.h - setup `main' in texmf.c */
/* extern int gargc; */
/* extern char **gargv; */

int wantcopyrght=1;

/* appears in reverse order in EXE file */

char *compiletime  =  __TIME__;
char *compiledate  =  __DATE__;
char *www          = "http://www.tug.org/yandy";
char *rights       = "All Rights Reserved.";
char *copyright    = "\nCopyright (C) 1993--2000 Y&Y, Inc.\n"
                     "Copyright (C) 2007 TeX Users Group.\n"
                     "Copyright (C) 2014 Clerk Ma.\n\n"
                     "This program is free software; you can redistribute it and/or modify\n"
                     "it under the terms of the GNU General Public License as published by\n"
                     "the Free Software Foundation; either version 2 of the License, or\n"
                     "(at your option) any later version.\n\n  ";
char *yandyversion = "2.2.3";
char *application  = "Y&Y TeX"; /* 96/Jan/17 */
char *tex_version  = "This is TeX, Version 3.14159265";

/* #define COPYHASH 1890382 */
/* #define COPYHASH 13862905 */
/* #define COPYHASH 10558802 */
/* #define COPYHASH 7254699 */
/* #define COPYHASH 3950596 */
/* #define COPYHASH 646493 */
#define COPYHASH 12905299

clock_t start_time, main_time, finish_time;

char * dvi_directory = ""; /* user specified directory for dvi file */
char * log_directory = ""; /* user specified directory for log file */
char * aux_directory = ""; /* user specified directory for aux file */
char * fmt_directory = ""; /* user specified directory for fmt file */
char * pdf_directory = ""; /* user specified directory for pdf file */

char * texpath = "";   /* path to executable - used if env vars not set */

// #define MAXLINE 256

char log_line[MAXLINE];  // used also in tex9.c

int mem_spec_flag     = 0;    /* non-zero if `-m=...' was used */ 
int format_spec       = 0;    /* non-zero if a format specified on command line */
int closed_already    = 0;    /* make sure we don't try this more than once */
bool reorder_arg_flag = true; /* put command line flags/arguments first */

/* Mapping from Windows ANSI to DOS code page 850 96/Jan/20 */
/* Used in tex0.c with wintodos[c-128]                      */

unsigned char wintodos[128] = {
    0,   0,   0, 159,   0,   0,   0,   0,
   94,   0,   0,   0,   0,   0,   0,   0,
    0,  96,  39,   0,   0,   7,   0,   0,
  126,   0,   0,   0,   0,   0,   0,   0,
   32, 173, 189, 156, 207, 190, 221,  21,
    0, 184, 166, 174, 170,  45, 169,   0,
  248, 241, 253, 252,   0, 230,  20, 250,
    0, 251, 167, 175, 172, 171, 243, 168,
  183, 181, 182, 199, 142, 143, 146, 128,
  212, 144, 210, 211, 222, 214, 215, 216,
  209, 165, 227, 224, 226, 229, 153, 158,
  157, 235, 233, 234, 154, 237, 232, 225,
  133, 160, 131, 198, 132, 134, 145, 135,
  138, 130, 136, 137, 141, 161, 140, 139,
  208, 164, 149, 162, 147, 228, 148, 246,
  155, 151, 163, 150, 129, 236, 231, 152
};  

void show_usage (void)
{
  char * s = log_line;
  sprintf (s, "\n"
      " yandytex [-?ivnwdrzpK] [-m=ini_mem] [-e=hyph_size] [-h=trie_size]\n"
      "          [-x=xchr_file] [-k=key_file] [-o=dvi_dir] [-l=log_dir] [-a=aux_dir]\n"
      "          [+format_file] [tex_file]\n\n"
      "    -?    show this usage summary\n"
      "    -i    start up as iniTeX (create format file)\n"
      "    -v    be verbose (show implementation version number)\n"
      "    -n    do not allow `non ASCII' characters in input files (complain instead)\n"
      "    -w    do not show `non ASCII' characters in hexadecimal (show as is)\n"
      "    -d    do not allow DOS style file names - i.e. do not convert \\ to /\n"
      "    -r    do not allow Mac style termination - i.e. do not convert \\r to \\n\n"
      "    -p    allow use of \\patterns after loading format (iniTeX only)\n"
      "    -K    disable all extensions to basic TeX\n"
      "    -m    initial main memory size in kilo words (iniTeX only)\n"
      "    -e    hyphenation exception dictionary size (iniTeX only)\n"
      "    -h    hyphenation pattern trie size (iniTeX only)\n"
      "    -x    use `non ASCII' character mapping (xchr[]) defined in file\n"
      "    -k    use `key replacement' defined in file\n"
      "    -o    write DVI file in specified directory (default current directory)\n"
      "    -l    write LOG file in specified directory (default current directory)\n"
      "    -a    write AUX file in specified directory (default current directory)");
  strcat(s, "\n");
  show_line(log_line, 1);

#ifndef _WINDOWS
  uexit(1);     // has this been setup yet ???
#endif
}

/* -z    do not discard control-Z at end of input file (treat as character)\n\ */

/* -c    prepend current directory (.) to TFM directory list\n\ */
/* -b    check that files with read access are not actually directories\n\ */

/* \t-d\tallow DOS style file names - i.e. convert \\ to / \n\ */
/* \t\t(applies to file name and format file name, if present)\n\ */
/* \t-r\tallow Mac style line termination - i.e. convert \\r to \\n \n\ */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Sep 27 1990 => 1990 Sep 27 */
/* 012456789      0123456789  */

void scivilize (char * date)
{
  int k;
  char pyear[6];

  strcpy (pyear, date + 7);

  for (k = 5; k >= 0; k--)
    date[k + 5] = date[k];

  for (k = 0; k < 4; k++)
    date[k] = pyear[k];

  date[4] = ' ';

  if (date[9] == ' ')
    date[9] = '0'; /* replace space by '0' */

  return;
}

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char * date)
{
  int k;
  char pyear[6];

  strcpy (pyear, date + 20);

  for (k = 18; k >= 0; k--)
    date[k+1] = date[k];

  date[20] = '\0';

  for (k = 0; k < 4; k++)
    date[k] = pyear[k];

  date[4] = ' ';

  return;
}

// void stamp_it (FILE *outfile)
// now writes result into given buffer
void stamp_it (char *s)
{
  char date[11 + 1];

  strcpy(date, compiledate);
  scivilize(date);
  sprintf(s, "%s %s ", application, yandyversion);
  s += strlen(s);
  sprintf(s, "(compiled time: %s %s)", date, compiletime);
  s += strlen(s);
}

void stampcopy (char *s)
{
  if (wantcopyrght)
  {
    sprintf(s, "%s %s", copyright, www);    /* 99/Oct/25 */
  }
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXCHRS 256
#define NOTDEF  127

void read_xchr_sub (FILE * xchr_input)
{
  char buffer[PATH_MAX];
  int k, from, to, count = 0;
  char *s;

#ifdef USEMEMSET
  memset (xchr, NOTDEF, MAXCHRS);           /* mark unused */
#else
  for (k = 0; k < MAXCHRS; k++)
    xchr[k] = -1; /* mark unused */
#endif

#ifdef USEMEMSET
  memset (xord, NOTDEF, MAXCHRS);           /* mark unused */
#else
  for (k = 0; k < MAXCHRS; k++)
    xord[k] = -1;  /* mark unused */
#endif

#ifdef ALLOCATEBUFFER
  while (fgets(buffer, current_buf_size, xchr_input) != NULL)
#else
  while (fgets(buffer, sizeof(buffer), xchr_input) != NULL)
#endif
  {
    if (*buffer == '%' || *buffer == ';' || *buffer == '\n')
      continue;

    from = (int) strtol (buffer, &s, 0);
    to = (int) strtol (s, NULL, 0);

    if (from >= 0 && from < MAXCHRS && to >= 0 && to < MAXCHRS)
    {
      if (xchr[from]== (unsigned char) NOTDEF)
      {
        xchr[from]= (unsigned char) to;
      }
      else
      {
        sprintf(log_line, "NOTE: %s collision: %d => %d, %d\n", "xchr", from, xchr[from], to);
        show_line(log_line, 0);
      }

      if (xord[to]== NOTDEF)
      {
        xord[to]= (unsigned char) from;
      }
      else
      {
        sprintf(log_line, "NOTE: %s collision: %d => %d, %d\n", "xord", to, xord[to], from);
        show_line(log_line, 0);
      }
      count++;
    }
  }

/*  now fill in the gaps */ /* not clear this is a good idea ... */
  for (k = 0; k < MAXCHRS; k++)
  {
    if (xchr[k]== NOTDEF)   /* if it has not been filled */
    {
      if (xord[k]== NOTDEF) /* see whether used already */
      {
        xchr[k]= (unsigned char) k; /* no, so make identity */
        xord[k]= (unsigned char) k; /* no, so make identity */
      }
    }
  }

  xchr[NOTDEF]= NOTDEF;         /* fixed point of mapping */

  if (trace_flag)
  {
    sprintf(log_line, "Read %d xchr[] pairs:\n", count);
    show_line(log_line, 0);

    for (k = 0; k < MAXCHRS; k++)
    {
      if (xchr[k]!= NOTDEF)
      {
        sprintf(log_line, "%d => %d\n", k, xchr[k]);
        show_line(log_line, 0);
      }
    }
  }
}

char *replacement[MAXCHRS];     /* pointers to replacement strings */

void read_repl_sub (FILE * repl_input)
{
  int k, n, m, chrs;
  char buffer[PATH_MAX];
  char charname[128];
  int charnum[10];
  char *s, *t;
  
#ifdef USEMEMSET
  memset(replacement, 0, MAXCHRS * sizeof(replacement[ 0]));
#else
  for (k = 0; k < MAXCHRS; k++)
    replacement[k] = NULL;
#endif

  while (fgets(buffer, PATH_MAX, repl_input) != NULL) {
    if (*buffer == '%' || *buffer == ';' || *buffer == '\n')
      continue;

    if ((m = sscanf (buffer, "%d%n %s", &chrs, &n, &charname)) == 0)
      continue;
    else if (m == 2) {
      if (*charname == '"') {   /* deal with quoted string "..." */
        s = buffer + n;
        t = charname;
        while (*s != '"' && *s != '\0') s++;  /* step up to " */
        if (*s++ == '\0') continue;       /* sanity check */
        while (*s != '\0') {  
          if (*s == '"') {
            s++;            /* is it "" perhaps ? */
            if (*s != '"') break;   /* no, end of string */
          }
          *t++ = *s++;          /* copy over */
        }
        *t = '\0';              /* and terminate */
      }
      if (chrs >= 0 && chrs < MAXCHRS)
        replacement[chrs] = xstrdup(charname);
    }
/*    presently the following can never get triggered */
/*    which is good, because it is perhaps not right ... */
    else if ((m = sscanf (buffer, "%d %d %d %d %d %d %d %d %d %d %d",
      &chrs, charnum, charnum+1, charnum+2, charnum+3, charnum+4,
        charnum+5, charnum+6, charnum+7, charnum+8, charnum+9)) > 1) {
/*      for (k = 0; k < n-1; k++) charname[k] = (char) charnum; */
      for (k = 0; k < n-1; k++) charname[k] = (char) charnum[k];
      charname[m] = '\0';
      if (chrs >= 0 && chrs < MAXCHRS)
        replacement[chrs] = xstrdup(charname);      
    }
    else {
      sprintf(log_line, "ERROR: don't understand %s", buffer);
      show_line(log_line, 1);
    }
  }
  if (trace_flag) {                  /* debugging output */
    show_line("Key replacement table\n", 0);
    for (k = 0; k < MAXCHRS; k++) {
      if (replacement[k] != NULL) {
        sprintf(log_line, "%d\t%s\n", k, replacement[k]);
        show_line(log_line, 0);
      }
    }
  }
}

/* Following used both to read xchr[] file and key replacement file */
/* the flag is 0 for -x=... and the flag is 1 for -k=... */

int read_xchr_file (char *filename, int flag, char *argv[])
{
  FILE *pinput;
  char infile[PATH_MAX];
  char *s;

  if (filename == NULL)
    return -1;

  if (trace_flag)
  {
    sprintf(log_line, "Reading xchr/repl %s\n", filename);
    show_line(log_line, 0);
  }

/*  first try using file as specified */
  strcpy(infile, filename);

  if (trace_flag)
  {
    sprintf(log_line, "Trying %s\n", infile);
    show_line(log_line, 0);
  }

  if (share_flag == 0)
    pinput = fopen (infile, "r");
  else
    pinput = _fsopen (infile, "r", share_flag);    /* 94/July/12 */

  if (pinput == NULL)
  {
    if (strrchr(infile, '.') == NULL)
    {
      if (flag == 0)
        strcat(infile, ".map");
      else
        strcat(infile, ".key");

      if (trace_flag)
      {
        sprintf(log_line, "Trying %s\n", infile);
        show_line(log_line, 0);
      }

      if (share_flag == 0)
        pinput = fopen (infile, "r");
      else
        pinput = _fsopen (infile, "r", share_flag);  /* 94/July/12 */
    }
  }

  if (pinput == NULL)
  {
    strcpy (infile, argv[0]);     /* try TeX program path */

    if ((s = strrchr (infile, '\\')) != NULL) *(s+1) = '\0';
    else if ((s = strrchr (infile, '/')) != NULL) *(s+1) = '\0';
    else if ((s = strrchr (infile, ':')) != NULL) *(s+1) = '\0';

    strcat (infile, filename);

    if (trace_flag)
    {
      sprintf(log_line, "Trying %s\n", infile);
      show_line(log_line, 0);
    }

    if (share_flag == 0)
      pinput = fopen (infile, "r");
    else
      pinput = _fsopen (infile, "r", share_flag);    /* 94/July/12 */

    if (pinput == NULL)
    {
      if (strchr(infile, '.') == NULL)
      {
        if (flag == 0)
          strcat(infile, ".map");
        else
          strcat(infile, ".key");

        if (trace_flag)
        {
          sprintf(log_line, "Trying %s\n", infile);
          show_line(log_line, 0);
        }

        if (share_flag == 0)
          pinput = fopen (infile, "r");
        else
          pinput = _fsopen (infile, "r", share_flag); /* 94/July/12 */
      }
    }
  }

  if (pinput == NULL)   /* 97/July/31 */
  {
    strcpy (infile, argv[0]);     /* try TeX program path */

    if ((s = strrchr (infile, '\\')) != NULL) *(s+1) = '\0';
    else if ((s = strrchr (infile, '/')) != NULL) *(s+1) = '\0';
    else if ((s = strrchr (infile, ':')) != NULL) *(s+1) = '\0';

    strcat (infile, "keyboard\\");
    strcat (infile, filename);

    if (trace_flag)
    {
      sprintf(log_line, "Trying %s\n", infile);
      show_line(log_line, 0);
    }

    if (share_flag == 0)
      pinput = fopen (infile, "r");
    else
      pinput = _fsopen (infile, "r", share_flag);

    if (pinput == NULL)
    {
      if (strchr(infile, '.') == NULL)
      {
        if (flag == 0)
          strcat(infile, ".map");
        else
          strcat(infile, ".key");

        if (trace_flag)
        {
          sprintf(log_line, "Trying %s\n", infile);
          show_line(log_line, 0);
        }

        if (share_flag == 0)
          pinput = fopen (infile, "r");
        else
          pinput = _fsopen (infile, "r", share_flag);
      }
    }
  }
/*  Note: can't look in TeX source file dir, since that is not known yet */
  if (pinput == NULL)
  {
    sprintf(log_line, "ERROR: Sorry, cannot find %s file %s",
        flag ? " xchr[]" : "key mapping", filename);
    show_line(log_line, 1);
    perrormod (filename);
    return 0;         // failed
  }

  if (flag == 0)
    read_xchr_sub (pinput);
  else
    read_repl_sub (pinput);

  (void) fclose (pinput);
  return 1;
}

/* need to also set `key_replace' here based on command line */
/* need to also allocate `buffercopy' here and free at end */
/* need to call `readreplace' in appropriate place */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following may be useful if link without floating point emulation */

#ifdef DEBUG
void testfloating (void)
{
  double dx = 1.0;
  double dxold = 0.0;
  int k = 0;

  while (1.0 + dx != 1.0)
  {
    dxold = dx;
    dx = dx / 2.0;
    k++;
  }
  sprintf(log_line, "Floating test: dx = %lg (k = %d)\n", dxold, k - 1);
  show_line(log_line, 0);
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *debugfile;      /* NULL or name of file to try and open */

#ifdef SHOWHEAPERROR
 char *heapstrings[] =
 {
    "", "Empty", "OK", "Bad Begin", "Bad Node", "End", "Bad Pointer"
 };
#endif

/* Attempt to get at problem with eqtb ... temporarily abandoned */
#ifdef CHECKEQTB
void check_eqtb (char *act)
{
  int k, count=0;
  memory_word *eqtb = zeqtb;

  for (k = hash_size + 780 + hash_extra; k < hash_size + 780 + eqtb_extra; k++)
  {
    if (eqtb[k].cint != 0)
    {
      if (count == 0)
      {
        show_char('\n');
        show_line("EQTB ", 0);
      }

      sprintf(log_line, "%d ", k);
      show_line(log_line, 0);

      if (count++ > 256)
        break;
    }
  }

  if (count != 0)
    show_char('\n');
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXSPLITS 3

/* ad hoc default minimum growth in memory realloc is 62% */
/* golden ratio (1 + \sqrt{5}) / 2 = 1.618033989... */
int percent_grow    = 62; /* default minimum growth in memory realloc is 62% */
int total_allocated = 0;  /* total memory allocated so far */
int ini_max_address = 0;  /* maximum address when starting */
int max_address     = 0;  /* maximum address seen in allocated memory */

/* see texd.h */

// DON'T USE THIS in DLL VERSION

#ifndef _WINDOWS
#ifdef HEAPWALK
unsigned int heapthreshold = 0; /* smallest size block interested in ... */

unsigned int heap_dump (FILE *output, int verbose)
{
  unsigned int total=0;
  struct _heapinfo hinfo;
  int heapstatus;
  int end_block=0;
  int n;

  if (verbose)
    fprintf(output, "HEAP DUMP:\n");

/*  if ((n = _heapchk ()) != _HEAPOK) { */
  n = _HEAPOK;
#ifdef SHOWHEAPERROR
  n = _heapchk();
#endif
  if (n != _HEAPOK)
  {
    fprintf(stderr, "WARNING: Heap corrupted (%d)\n", n);
#ifdef SHOWHEAPERROR
    fprintf(stderr, "HEAP %s (%s)\n", heapstrings[-n], "heap_dump");
#endif
  }
  hinfo._pentry = NULL;

  while ((heapstatus = _heapwalk(&hinfo)) == _HEAPOK)
  {
    if (end_block > 0 && (int) hinfo._pentry > end_block + 1024)
    {
//      if (verbose) printf("GAP of %d bytes!\n", (int) hinfo._pentry - end_block);
    }
    end_block = (int) hinfo._pentry + hinfo._size;
    if (hinfo._useflag == _USEDENTRY) total += hinfo._size;
    if (hinfo._size >= heapthreshold && verbose)
      fprintf(output, "%6s block at %p (%7d) of size %6X (%7d) => (%7d)\n",
          (hinfo._useflag == _USEDENTRY ? "USED" : "...."),
          hinfo._pentry, hinfo._pentry, hinfo._size, hinfo._size,
          end_block);
  }

  switch (heapstatus)
  {
    case _HEAPEMPTY:
      if (verbose) fprintf(output, "OK - empty heap\n");
      break;
    case _HEAPEND:
      if (verbose) fprintf(output, "OK - end of heap (%u bytes used)\n", total);
      break;
    case _HEAPBADPTR:
      fprintf(output, "ERROR - %s\n", "bad pointer to heap");
      break;
    case _HEAPBADBEGIN:
      fprintf(output, "ERROR - %s\n", "bad start of heap");
      break;
    case _HEAPBADNODE:
      fprintf(output, "ERROR - %s\n", "bad node in heap");
      break;
  }
  return total;
}
#endif
#endif

void show_maximums (FILE *output)
{
#ifdef HEAPWALK
  unsigned heaptotal = 0;           /* no longer used */
  heaptotal = heap_dump(stdout, 0);      /* 94/Apr/3 */
#endif

  sprintf(log_line, "Max allocated %d --- max address %d\n", total_allocated, max_address);
//  if (output != NULL) fputs(log_line, output); // log file
//  else if (flag == 0) show_line(log_line, 0); // informative
//  else if (flag == 1) show_line(log_line, 1); // error
  if (output == stderr)
    show_line(log_line, 1);
  else if (output == stdout)
    show_line(log_line, 0);
  else
    fputs(log_line, output);
}

/* our own version of realloc --- avoid supposed MicroSoft version bug */
/* also tries _expand first, which can avoid address growth ... */

#ifdef USEOUREALLOC 
void *ourrealloc (void *old, size_t new_size)
{
  void * mnew;
  size_t old_size, overlap;

/*  round up to nearest multiple of four bytes *//* avoid unlikely alignment */
  if ((new_size % 4) != 0)
    new_size = ((new_size / 4) + 1) * 4;

  if (old == NULL)
    return malloc (new_size);  /* no old block - use malloc */

  old_size = _msize (old);

  if (old_size >= new_size && old_size < new_size + 4)
    return old;
/*  _heapmin(); */  /* release unused heap space to the system - no op ? */
#ifdef HEAPSHOW
  if (trace_flag)
  {
    show_line("BEFORE REALLOC: \n", 0);
#ifdef HEAPWALK
    (void) heap_dump(stdout, 1);     /* debugging 96/Jan/18 */
#endif
  }
#endif
  mnew = _expand (old, new_size);      /* first try and expand in place */

  if (mnew != NULL)
  {
    if (trace_flag)
    {
      sprintf(log_line, "EXPANDED! %d (%d) == %d (%d)\n",
          mnew, new_size, old, old_size);
      show_line(log_line, 0);
    }
    return mnew;
  }
/*  *********************************************************************** */
/*  do this if you want to call the real realloc next -  */
  mnew = realloc (old, new_size);
#ifdef HEAPSHOW
  if (trace_flag)
  {
    show_line("AFTER REALLOC: \n", 0);
#ifdef HEAPWALK
    (void) heap_dump(stdout, 1);     /* debugging 96/Jan/18 */
#endif
  }
#endif
  if (mnew != NULL)
    return mnew;
/*  we are screwed typically if we ever drop through here - no more space */
/*  *********************************************************************** */
  mnew = malloc (new_size);          /* otherwise find new space */

  if (mnew == NULL)
    return mnew;        /* if unable to allocate */

  if (old_size < new_size)
    overlap = old_size;
  else
    overlap = new_size;

  memcpy (mnew, old, overlap);         /* copy old data to new area */
  free(old);                  /* free the old area */
  return mnew;
}
#endif

void memory_error (char *s, int n)
{
  if (log_opened)
  {
    fprintf(log_file, "\n! Unable to allocate %d bytes for %s\n", n, s);
    show_maximums(log_file);

#ifdef HEAPWALK
    if (heap_flag)
      (void) heap_dump(log_file, 1);
#endif
  }

  sprintf(log_line, "\n! Unable to allocate %d bytes for %s\n", n, s);
  show_line(log_line, 1);
  show_maximums(stderr);

#ifdef HEAPWALK
  if (heap_flag)
    (void) heap_dump(stderr, 1);
#endif
/*  exit (1); */      /* 94/Jan/22 */
/*  return to let TeX do its thing (such as complain about runaway) */
/*  don't set abort_flag here */
}

void trace_memory (char *s, int n)
{
  sprintf(log_line, "Allocating %d bytes for %s\n", n, s);
  show_line(log_line, 0);
}

void update_statistics (int address, int size, int oldsize)
{
  if (address + size > max_address)
    max_address = address + size;

  total_allocated =  total_allocated + size - oldsize;
}

void probe_memory (void)
{
  char *s;

  s = (char *) malloc (4); /* get current top address */
  free(s);
  update_statistics ((int) s, 0, 0); /* show where we are */
}

void probe_show (void)
{
  probe_memory();
  show_maximums(stdout);

#ifdef HEAPWALK
  if (heap_flag)
    (void) heap_dump(stdout, 1);
#endif
}

size_t roundup (size_t n)
{
  if ((n % 4) == 0)
    return n;
  else
    return ((n / 4) + 1) * 4;
}

#ifdef ALLOCATETRIES
/* using allocating hyphenation trie slows things down maybe 1%              */
/* but saves typically (270k - 55k) = 215k of memory                         */
/* NOTE: it's safe to allocate based on the trie_max read from fmt file      */
/* since hyphenation trie cannot be extended (after iniTeX)                  */
/* for iniTeX, however, we need to allocate the full trie_size ahead of time */
/*                                                                           */
/* NOTE: we don't ever reallocate these                                      */
/* returns -1 if it fails                                                    */

int allocate_tries (int trie_max)
{
  int n, nl, no, nc;
/*  if (trie_max > trie_size) {
    sprintf(log_line, "ERROR: invalid trie size (%d > %d)\n",
      trie_max, trie_size);
      show_line(log_line, 1);
    exit (1);
  } */ /* ??? removed 1993/dec/17 */
  if (trie_max > 1000000)
    trie_max = 1000000; /* some sort of sanity limit */
/*  important + 1 because original was halfword trie_trl[trie_size + 1] etc. */
  nl = (trie_max + 1) * sizeof(halfword);    /* trie_trl[trie_size + 1] */
  no = (trie_max + 1) * sizeof(halfword);    /* trie_tro[trie_size + 1] */
  nc = (trie_max + 1) * sizeof(quarterword); /* trie_trc[trie_size + 1] */
  n = nl + no + nc;

  if (trace_flag)
    trace_memory("hyphen trie", n);

  trie_trl = (halfword *)    malloc (roundup(nl));
  trie_tro = (halfword *)    malloc (roundup(no));
  trie_trc = (quarterword *) malloc (roundup(nc));

  if (trie_trl == NULL || trie_tro == NULL || trie_trc == NULL)
  {
    memory_error("hyphen trie", n);
    return -1;
  }

  if (trace_flag)
  {
    sprintf(log_line, "Addresses trie_trl %d trie_tro %d trie_trc %d\n", trie_trl, trie_tro, trie_trc);
    show_line(log_line, 0);
  }

  update_statistics ((int) trie_trl, nl, 0);
  update_statistics ((int) trie_tro, no, 0);
  update_statistics ((int) trie_trc, nc, 0);
/*  sprintf(log_line, "trie_size %d trie_max %d\n", trie_size, trie_max); */ /* debug */
  trie_size = trie_max;           /* BUG FIX 98/Jan/5 */

  if (trace_flag)
    probe_show();     /* 94/Mar/25 */
  return 0;               // success
}
#endif

#ifdef ALLOCATEHYPHEN
bool prime (int);       /* test function later in this file */

int current_prime = 0;         /* remember in case reallocated later */

/* we don't return an address here, since TWO memory regions allocated */
/* plus, we don't really reallocate, we FLUSH the old information totally */
/* returns -1 if it fails */

int realloc_hyphen (int hyphen_prime)
{
  int n, nw, nl;

  if (!prime(hyphen_prime))
  {
    sprintf(log_line, "ERROR: non-prime hyphen exception number (%d)\n", hyphen_prime);
    show_line(log_line, 1);
    return -1;
  }
/*  need not/cannot preserve old contents when hyphen prime is changed */
/*  if (hyph_list != NULL) free(hyph_list); */
/*  if (hyph_word != NULL) free(hyph_word); */
/*  important + 1 since str_number hyph_word[hyphen_prime + 1]  in original etc. */
  nw = (hyphen_prime + 1) * sizeof(str_number);
  nl = (hyphen_prime + 1) * sizeof(halfword);
  n = nw + nl;

  if (trace_flag)
    trace_memory("hyphen exception", n);

/*  initially hyph_word will be NULL so this acts like malloc */
/*  hyph_word = (str_number *) malloc (nw); */
  hyph_word = (str_number *) REALLOC (hyph_word, nw);  /* 94/Mar/24 */
/*  initially hyph_list will be NULL so this acts like malloc */
/*  hyph_list = (halfword *) malloc (nl); */
  hyph_list = (halfword *) REALLOC (hyph_list, nl);   /* 94/Mar/24 */

  if (hyph_word == NULL || hyph_list == NULL)
  {
    memory_error("hyphen exception", n);
    return -1;
  }

  if (trace_flag)
  {
    sprintf(log_line, "Addresses hyph_word %d hyph_list %d\n", hyph_word, hyph_list);
    show_line(log_line, 0);
  }
/*  cannot preserve old contents when hyphen prime is changed */
#ifdef USEMEMSET
  memset(hyph_word, 0, (hyphen_prime + 1) * sizeof (hyph_word[0]));
#else
  for (k = 0; k <= hyphen_prime; k++) hyph_word[k]= 0;
#endif

#ifdef USEMEMSET
  memset(hyph_list, 0, (hyphen_prime + 1) * sizeof (hyph_list[0]));
#else
  for (k = 0; k <= hyphen_prime; k++) hyph_list[k]= 0;
#endif
  hyph_count = 0;   /* or use reset_hyphen() in itex.c */

  if (current_prime != 0)
  {
    update_statistics ((int) hyph_word, nw, (current_prime + 1) * sizeof(str_number));
    update_statistics ((int) hyph_list, nl, (current_prime + 1) * sizeof(halfword));
  }
  else
  {
    update_statistics ((int) hyph_word, nw, 0);
    update_statistics ((int) hyph_list, nl, 0);
  }
  current_prime = hyphen_prime;

  if (trace_flag)
    probe_show();     /* 94/Mar/25 */

  return 0;               // success
}
#endif

int current_mem_size = 0;   /* current total words in main mem allocated -1 */

/* this gets called from itex.c when it figures out what mem_top is */
/* or gets called from here when in ini_TeX mode */ /* and nowhere else */
/* initial allocation only, may get expanded later */
/* NOTE: we DON't use ALLOCATEHIGH & ALLOCATELOW anymore */
/* returns NULL if it fails */

#ifdef ALLOCATEMAIN   
/* initial main memory alloc - mem_top */
memory_word *allocate_main_memory (int size)
{
  int n;

/*  Using -i *and* pre-loading format */ /* in this case get called twice */
/*  Get rid of initial blank memory again or use realloc ... */
/*  Could we avoid this by detecting presence of & before allocating ? */
/*  Also, if its already large enough, maybe we can avoid this ? */
/*  don't bother if current_mem_size == mem_max - mem_start ? */
  if (mainmemory != NULL)
  {
    if (trace_flag)
      show_line("Reallocating initial memory allocation\n", 1);
  }

  mem_top = mem_bot + size;

#ifdef ALLOCATEHIGH         /* NOT USED ANYMORE */
  if (mem_extra_high != 0 && !is_initex)
    mem_max = mem_top + mem_extra_high;
#endif

  mem_max = mem_top;

#ifdef ALLOCATELOW          /* NOT USED ANYMORE */
  if (mem_extra_low != 0 && !is_initex)
    mem_start = mem_bot - mem_extra_low;  /* increase main memory */
#endif

  mem_start = 0;     /* bottom of memory allocated by system */
/*  mem_min = mem_start; */ /* bottom of area made available to TeX */
  mem_min = 0;       /* bottom of area made available to TeX */
  n = (mem_max - mem_start + 1) * sizeof (memory_word); /* 256k * 8 = 2000 k */

  if (trace_flag)
    trace_memory("main memory", n);

/*  mainmemory = (memory_word *) malloc (n); */  /* 94/March/24 */
/*  normally mainmemory == NULL here so acts like malloc ... */
  mainmemory = (memory_word *) REALLOC (mainmemory, n);

  if (mainmemory == NULL)
  {
    memory_error("initial main memory", n);
//    exit (1);             /* serious error */
    return NULL;
  }

  if (trace_flag)
  {
    sprintf(log_line, "Address main memory == %d\n", mainmemory);
    show_line(log_line, 0);
  }

  zzzaa = mainmemory;

  if (mem_start != 0 && !is_initex)
    zzzaa = mainmemory - mem_start;

  if (trace_flag)
  {
    sprintf(log_line, "Offset address main memory == %d\n", zzzaa);
    show_line(log_line, 0);
  }

  update_statistics ((int) mainmemory, n,
    (current_mem_size + 1) * sizeof (memory_word));
/*  current_mem_size = (mem_max - mem_start + 1); */
  current_mem_size = mem_max - mem_start;   /* total number of words - 1 */

  if (trace_flag)
    probe_show();     /* 94/Mar/25 */
  return zzzaa;             /* same as zmem, mem 94/Jan/24 */
}
#endif  /* end of ALLOCATEMAIN */

#ifdef ALLOCATEMAIN
/* int firstallocation = 1; */

/* increase main memory allocation at low end and high end */
/* called only from tex0.c *//* called with one of losize or hisize == 0 */
/* returns NULL if it fails */

memory_word *realloc_main (int losize, int hisize)
{  
  int k, minsize;
  int newsize = 0;        /* to quieten compiler */
  int n = 0;          /* to quieten compiler */
  memory_word * newmemory = NULL; /* to quieten compiler */

/*  if (losize == 0 && hisize > 0) runawayflag = 1;
  else runawayflag = 0; */ /* 94/Jan/22 */

  if (trace_flag)
  {
    sprintf(log_line, "WARNING: Entering realloc_main lo %d hi %d\n", losize, hisize);
    show_line(log_line, 0);
  }

  if (is_initex)
  {
    show_line("ERROR: Cannot extent main memory in iniTeX\n", 1);

    if (! knuth_flag)
      show_line("Please use `-m=...' on command line\n", 0);
//    abort_flag++;  // ???
    return NULL;
  }

  if (trace_flag)
  {
    sprintf(log_line, "Old Address %s == %d\n", "main memory", mainmemory);
    show_line(log_line, 0);
  }

  if (current_mem_size + 1 == max_mem_size) /* if we REALLY run up to limit ! */
  {
    memory_error("main memory", (max_mem_size + 1) * sizeof(memory_word));
//    abort_flag++;  // ???
    return NULL;
  }
/*  first allocation should expand *both* lo and hi */
  if (hisize == 0 && mem_end == mem_max)
    hisize = losize;

  if (losize == 0 && mem_start == mem_min)
    losize = hisize;

/*  try and prevent excessive frequent reallocations */
/*  while avoiding over allocation by too much */
  minsize = current_mem_size / 100 * percent_grow;

  if (losize + hisize < minsize)
  {
    if (losize > 0 && hisize > 0)
    {
      losize = minsize / 2;
      hisize = minsize / 2;
    }
    else if (losize > 0)
      losize = minsize;
    else if (hisize > 0)
      hisize = minsize;
  }

  if (losize > 0 && losize < mem_top / 2)
    losize = mem_top / 2;

  if (hisize > 0 && hisize < mem_top / 2)
    hisize = mem_top / 2;

  for (k = 0; k < MAXSPLITS; k++)
  {
    newsize = current_mem_size + losize + hisize;

    if (newsize >= max_mem_size) /* bump against limit - ha ha ha */
    {
      while (newsize >= max_mem_size) {
        losize = losize / 2; hisize = hisize / 2;
        newsize = current_mem_size + losize + hisize;
      }
    }

    n = (newsize + 1) * sizeof (memory_word);

    if (trace_flag)
      trace_memory("main memory", n);

    newmemory = (memory_word *) REALLOC (mainmemory, n);

    if (newmemory != NULL)
      break; /* did we get it ? */

    if (current_mem_size == 0)
      break; /* in case we ever use for initial */

    losize = losize / 2; hisize = hisize / 2;
  }

  if (newmemory == NULL)
  {
    memory_error("main memory", n);
    return zzzaa;           /* try and continue with TeX !!! */
  }

  if (trace_flag)
  {
    sprintf(log_line, "New Address %s == %d\n", "main memory", newmemory);
    show_line(log_line, 0);
  }

  if (losize > 0)
  {
/*  shift everything upward to make space for new low area */
    if (trace_flag)
    {
      sprintf(log_line, "memmove %d %d %d \n", newmemory + losize,
          newmemory, (current_mem_size + 1) * sizeof(memory_word));
      show_line(log_line, 0);
    }
    memmove (newmemory + losize, newmemory,
      (current_mem_size + 1) * sizeof(memory_word));
/*  could reduce words moved by (mem_max - mem_end) */
  }
  mainmemory = newmemory;       /* remember for free later */

  if (losize > 0)
    mem_start = mem_start - losize; /* update lower limit */

  if (hisize > 0)
    mem_max = mem_max + hisize;   /* update upper limit */

  update_statistics ((int) mainmemory, n,
    (current_mem_size + 1) * sizeof (memory_word));
  current_mem_size = newsize;

  if (current_mem_size != mem_max - mem_start)
  {
    show_line("ERROR: Impossible Memory Error\n", 1);
  }

  if (mem_start != 0)
    zzzaa = mainmemory - mem_start; /* ??? sign ??? */
  else
    zzzaa = mainmemory;

  if (trace_flag)
    probe_show();     /* 94/Mar/25 */

  return zzzaa;
}
#endif

#ifdef ALLOCATEFONT
/* font_mem_size = 10000L ==> font_info array 100k * 8 = 800 kilobytes */

int current_font_mem_size=0;

/* fmemoryword can be either halfword or memory_word */
fmemoryword * realloc_font_info (int size)
{
  fmemoryword *newfontinfo = NULL;
  int k, minsize;
  int newsize = 0;  /* to quieten compiler */
  int n = 0;        /* to quieten compiler */

  if (trace_flag)
  {
    sprintf(log_line, "Old Address %s == %d\n",  "font_info", font_info);
    show_line(log_line, 0);
  }
/*  during initial allocation, font_info == NULL - realloc acts like malloc */
/*  during initial allocation current_font_mem_size == 0 */
  if (current_font_mem_size == font_mem_size)  /* if we REALLY run up to limit */
  {
/*    memory_error("font", (font_mem_size + 1) * sizeof(memory_word)); */
    return font_info;    /* pass it back to TeX 99/Fabe/4 */
  }
/*  try and prevent excessive frequent reallocations */
/*  while avoiding over allocation by too much */
/*  minsize = current_font_mem_size / 2; */
  minsize = current_font_mem_size / 100 * percent_grow;

  if (size < minsize)
    size = minsize;

  if (size < initial_font_mem_size)
    size = initial_font_mem_size;

  for (k=0; k < MAXSPLITS; k++)
  {
    newsize = current_font_mem_size + size;

    if (newsize > font_mem_size)
      newsize = font_mem_size; /* bump against limit */

/*    important + 1 since fmemoryword font_info[font_mem_size + 1]  original */
    n = (newsize + 1) * sizeof (fmemoryword);

    if (trace_flag)
      trace_memory("font_info", n);

    newfontinfo = (fmemoryword *) REALLOC (font_info, n);

    if (newfontinfo != NULL)
      break;   /* did we get it ? */

    if (current_font_mem_size == 0)
      break; /* initial allocation must work */

    size = size / 2;
  }

  if (newfontinfo == NULL)
  {
    memory_error("font", n);
    return font_info;        /* try and continue !!! */
  }

  font_info = newfontinfo;

  if (trace_flag)
  {
    sprintf(log_line, "New Address %s == %d\n", "font_info", font_info);
    show_line(log_line, 0);
  }

  update_statistics ((int) font_info, n, current_font_mem_size * sizeof(fmemoryword));
  current_font_mem_size = newsize;

  if (trace_flag)
    probe_show();     /* 94/Mar/25 */
  return font_info;
}
#endif

#ifdef ALLOCATESTRING
int current_pool_size = 0;

packed_ASCII_code * realloc_str_pool (int size)
{
  int k, minsize;
  int newsize=0;
  int n=0;
  packed_ASCII_code *newstrpool=NULL;

  if (trace_flag)
  {
    sprintf(log_line, "Old Address %s == %d\n", "string pool", str_pool);
    show_line(log_line, 0);
  }

  if (current_pool_size == pool_size)
  {
/*    memory_error ("string pool", (pool_size + 1) * sizeof(packed_ASCII_code)); */
/*    exit (1); */
    return str_pool;   /* pass it back to TeX 99/Fabe/4 */
  }
/*  minsize =  current_pool_size / 2; */
  minsize =  current_pool_size / 100 * percent_grow;
  if (size < minsize) size = minsize;
  if (size < initial_pool_size) size = initial_pool_size;

  for (k = 0; k < MAXSPLITS; k++) {
    newsize = current_pool_size + size;
    if (newsize > pool_size) newsize = pool_size;
/* important + 1 since  packed_ASCII_code str_pool[pool_size + 1]; in original */
    n = (newsize + 1) * sizeof (packed_ASCII_code);
    if (trace_flag) trace_memory("str_pool", n);
    newstrpool = (packed_ASCII_code *) REALLOC (str_pool, n); /* 95/Sep/24 */
    if (newstrpool != NULL) break;    /* did we get it ? */
    if (current_pool_size == 0) break;  /* initial allocation must work */
    size = size / 2;          /* else can retry smaller */
  }

  if (newstrpool == NULL) {
    memory_error("string pool", n);
    return str_pool;           /* try and continue !!! */
  }
  str_pool = newstrpool;
  update_statistics ((int) str_pool, n, current_pool_size);
  current_pool_size = newsize;
  if (trace_flag) {
    sprintf(log_line, "New Address %s == %d\n", "string pool", str_pool);
    show_line(log_line, 0);
  }
  if (trace_flag)  probe_show();     /* 94/Mar/25 */
  return str_pool;
}
#endif

#ifdef ALLOCATESTRING
int current_max_strings=0;

pool_pointer *realloc_str_start (int size)
{
  int k, minsize;
  int n=0;
  int newsize=0;
  pool_pointer *newstrstart=NULL;

  if (trace_flag)
  {
    sprintf(log_line, "Old Address %s == %d\n", "string start", str_start);
    show_line(log_line, 0);
  }

  if (current_max_strings == max_strings)
  {
/*    memory_error ("string pointer", (max_strings + 1) * sizeof(pool_pointer)); */
/*    exit (1); */
    return str_start;    /* pass it back to TeX 99/Fabe/4 */
  }
/*  minsize = current_max_strings / 2; */
  minsize = current_max_strings / 100 * percent_grow;
  if (size < minsize) size = minsize;
  if (size < initial_max_strings) size = initial_max_strings;

  for (k = 0; k < MAXSPLITS; k++)
  {
    newsize = current_max_strings + size;
    if (newsize > max_strings) newsize = max_strings;
/*    important + 1 since str_start[maxstring + 1] originally */
    n = (newsize + 1) * sizeof (pool_pointer);
    if (trace_flag) trace_memory("str_start", n);
    newstrstart = (pool_pointer *) REALLOC (str_start, n);
    if (newstrstart != NULL) break;   /* did we get it ? */
    if (current_max_strings == 0) break;  /* initial allocation must work */
    size = size / 2;          /* otherwise can try smaller */
  }

  if (newstrstart == NULL) {
    memory_error("string pointer", n);
    return str_start;          /* try and continue */
  }
  str_start = newstrstart;
  update_statistics((int) str_start, n, current_max_strings * sizeof (pool_pointer));
  current_max_strings = newsize;
  if (trace_flag) {
    sprintf(log_line, "New Address %s == %d\n", "string start", str_start);
    show_line(log_line, 0);
  }
  if (trace_flag)  probe_show();     /* 94/Mar/25 */
  return str_start;
}
#endif

#ifdef ALLOCATEINI

/* returns -1 if it fails */

int allocate_ini (int size)
{    /* size == trie_size */
  int n, nl, no, nc, nr, nh, nt;
    nh = nr = nl = (size + 1) *  sizeof(trie_pointer);
    no = (size + 1) *  sizeof(trie_op_code);
    nc = (size + 1) *  sizeof(packed_ASCII_code);
/*    nt = (size + 1) *  sizeof(bool); */
    nt = (size + 1) *  sizeof(char);
    n = nl + no + nc + nr + nh + nt;
/*    n = (size + 1) * (sizeof(packed_ASCII_code) + sizeof(trie_op_code) +
      3 *  sizeof(trie_pointer) + sizeof (char)); */
    if (trace_flag) trace_memory ("iniTeX hyphen trie", n);
    trie_l = (trie_pointer *) malloc (roundup(nl));
    trie_o = (trie_op_code *) malloc (roundup(no));
    trie_c = (packed_ASCII_code *) malloc (roundup(nc));
    trie_r = (trie_pointer *) malloc (roundup(nr));
    trie_hash = (trie_pointer *) malloc (roundup(nh));
/*    trie_taken = (bool *) malloc (nt); */
    trie_taken = (char *) malloc (roundup(nt));
    if (trie_c == NULL || trie_o == NULL || trie_l == NULL || trie_r == NULL ||
      trie_hash == NULL || trie_taken == NULL) {
      memory_error("iniTeX hyphen trie", n);
//      exit (1);           /* serious error */     
      return -1;
    }
    if (trace_flag) {
      sprintf(log_line, "Addresses trie_l %d trie_o %d trie_c %d\n", 
          trie_l, trie_o, trie_c);
      show_line(log_line, 0);
      sprintf(log_line, "Addresses trie_r %d trie_hash %d trie_taken %d\n", 
          trie_r, trie_hash, trie_taken);
      show_line(log_line, 0);
    }
    update_statistics ((int) trie_l, nl, 0);
    update_statistics ((int) trie_o, no, 0);
    update_statistics ((int) trie_c, nc, 0);
    update_statistics ((int) trie_r, nr, 0);
    update_statistics ((int) trie_hash, nh, 0);
    update_statistics ((int) trie_taken, nt, 0);
/*    trie_size = size; */ /* ??? */
    if (trace_flag)  probe_show();     /* 94/Mar/25 */
    return 0;               // success
}
#endif

#ifdef ALLOCATESAVESTACK
int current_save_size=0;

memory_word *realloc_save_stack (int size)
{
  int k, minsize;
  int n=0, newsize=0;
  memory_word *newsave_stack=NULL;

  if (trace_flag) {
    sprintf(log_line, "Old Address %s == %d\n", "save stack", save_stack);
    show_line(log_line, 0);
  }
  if (current_save_size == save_size) {  /* arbitrary limit */
/*    memory_error ("save stack", (save_size + 1) * sizeof(memory_word)); */
/*    exit (1); */
    return save_stack;       /* let TeX handle the error */
  }
  minsize =  current_save_size / 100 * percent_grow;
  if (size < minsize) size = minsize;
  if (size < initial_save_size) size = initial_save_size;

  for (k = 0; k < MAXSPLITS; k++) {
    newsize = current_save_size + size;
    if (newsize > save_size) newsize = save_size;
    n = (newsize + 1) * sizeof (memory_word); /* save_stack[save_size + 1] */
    if (trace_flag) trace_memory("save_stack", n);
    newsave_stack = (memory_word *) REALLOC (save_stack, n);
    if (newsave_stack != NULL) break;    /* did we get it ? */
    if (current_save_size == 0) break;  /* initial allocation must work */
    size = size / 2;          /* else can retry smaller */
  }

  if (newsave_stack == NULL) {
    memory_error("save stack", n);
    return save_stack;           /* try and continue !!! */
  }
  save_stack = newsave_stack;
  update_statistics ((int) save_stack, n, current_save_size);
  current_save_size = newsize;
  if (trace_flag) {
    sprintf(log_line, "Current%s %d\n", "save_size", current_save_size);
    show_line(log_line, 0);
    sprintf(log_line, "New Address %s == %d\n", "save stack", save_stack);
    show_line(log_line, 0);
  }
  if (trace_flag) probe_show();      /* 94/Mar/25 */
  return save_stack;
}
#endif

#ifdef ALLOCATEINPUTSTACK
int current_stack_size=0;       /* input stack size */

in_state_record *realloc_input_stack (int size)
{
  int k, minsize;
  int n=0, newsize=0;
  in_state_record *newinputstack=NULL;

  if (trace_flag) {
    sprintf(log_line, "Old Address %s == %d\n",  "input stack", input_stack);
    show_line(log_line, 0);
  }
  if (current_stack_size == stack_size) {  /* arbitrary limit */
/*    memory_error ("input stack", (stack_size + 1) * sizeof(in_state_record)); */
/*    exit (1); */
    return input_stack;
  }
  minsize =  current_stack_size / 100 * percent_grow;
  if (size < minsize) size = minsize;
  if (size < initial_stack_size) size = initial_stack_size;

  for (k = 0; k < MAXSPLITS; k++) {
    newsize = current_stack_size + size;
    if (newsize > stack_size) newsize = stack_size;
    n = (newsize + 1) * sizeof (in_state_record); /* input_stack[stack_size + 1] */
    if (trace_flag) trace_memory("input_stack", n);
    newinputstack = (in_state_record *) REALLOC (input_stack, n);
    if (newinputstack != NULL) break;   /* did we get it ? */
    if (current_stack_size == 0) break; /* initial allocation must work */
    size = size / 2;          /* else can retry smaller */
  }

  if (newinputstack == NULL) {
    memory_error("input stack", n);
    return input_stack;            /* try and continue !!! */
  }
  input_stack = newinputstack;
  update_statistics ((int) input_stack, n, current_stack_size);
  current_stack_size = newsize;
  if (trace_flag) {
    sprintf(log_line, "Current%s %d\n", "stack_size", current_stack_size);
    show_line(log_line, 0);
    sprintf(log_line, "New Address %s == %d\n", "input stack", input_stack);
    show_line(log_line, 0);
  }
  if (trace_flag)  probe_show();     /* 94/Mar/25 */
  return input_stack;
}
#endif

#ifdef ALLOCATENESTSTACK
int current_nest_size=0;        /* current nest size */

list_state_record *realloc_nest_stack (int size)
{
  int k, minsize;
  int n=0, newsize=0;
  list_state_record *newnest=NULL;

  if (trace_flag) {
    sprintf(log_line, "Old Address %s == %d\n",  "nest stack", nest);
    show_line(log_line, 0);
  }
  if (current_nest_size == nest_size) {  /* arbitrary limit */
/*    memory_error ("nest stack", (nest_size + 1) * sizeof(list_state_record)); */
/*    exit (1); */
    return nest;        /* let TeX handle the error */
  }
  minsize =  current_nest_size / 100 * percent_grow;
  if (size < minsize) size = minsize;
  if (size < initial_nest_size) size = initial_nest_size;

  for (k = 0; k < MAXSPLITS; k++) {
    newsize = current_nest_size + size;
    if (newsize > nest_size) newsize = nest_size;
    n = (newsize + 1) * sizeof (list_state_record); /* nest[nest_size + 1] */
    if (trace_flag) trace_memory("nest stack", n);
    newnest = (list_state_record *) REALLOC (nest, n);
    if (newnest != NULL) break;   /* did we get it ? */
    if (current_nest_size == 0) break;  /* initial allocation must work */
    size = size / 2;          /* else can retry smaller */
  }

  if (newnest == NULL) {
    memory_error("nest stack", n);
    return nest;            /* try and continue !!! */
  }
  nest = newnest;
  update_statistics ((int) nest, n, current_nest_size);
  current_nest_size = newsize;
  if (trace_flag) {
    sprintf(log_line, "Current%s %d\n", "nest_size", current_nest_size);
    show_line(log_line, 0);
    sprintf(log_line, "New Address %s == %d\n", "nest stack", nest);
    show_line(log_line, 0);
  }
  if (trace_flag)  probe_show();     /* 94/Mar/25 */
  return nest;
}
#endif

#ifdef ALLOCATEPARAMSTACK
int current_param_size=0;       /* current param size */

halfword *realloc_param_stack (int size)
{
  int k, minsize;
  int n=0, newsize=0;
  halfword *newparam=NULL;

  if (trace_flag) {
    sprintf(log_line, "Old Address %s == %d\n",  "param stack", param_stack);
    show_line(log_line, 0);
  }
  if (current_param_size == param_size) {  /* arbitrary limit */
/*    memory_error ("param stack", (param_size + 1) * sizeof(halfword)); */
/*    exit (1); */
    return param_stack;        /* let TeX handle the error */
  }
  minsize =  current_param_size / 100 * percent_grow;
  if (size < minsize) size = minsize;
  if (size < initial_param_size) size = initial_param_size;

  for (k = 0; k < MAXSPLITS; k++) {
    newsize = current_param_size + size;
    if (newsize > param_size) newsize = param_size;
    n = (newsize + 1) * sizeof (halfword); /* param_stack[param_size + 1] */
    if (trace_flag) trace_memory("param stack", n);
    newparam = (halfword *) REALLOC (param_stack, n); 
    if (newparam != NULL) break;    /* did we get it ? */
    if (current_param_size == 0) break; /* initial allocation must work */
    size = size / 2;          /* else can retry smaller */
  }

  if (newparam == NULL) {
    memory_error("param stack", n);
    return param_stack;            /* try and continue !!! */
  }
  param_stack = newparam;
  update_statistics ((int) param_stack, n, current_param_size);
  current_param_size = newsize;
  if (trace_flag) {
    sprintf(log_line, "Current%s %d\n", "param_size", current_param_size);
    show_line(log_line, 0);
    sprintf(log_line, "New Address %s == %d\n", "param stack", param_stack);
    show_line(log_line, 0);
  }
  if (trace_flag)  probe_show();     /* 94/Mar/25 */
  return param_stack;
}
#endif

#ifdef ALLOCATEBUFFER
int current_buf_size=0;

ASCII_code *realloc_buffer (int size)
{
  int k, minsize;
  int n=0, newsize=0;
  ASCII_code *newbuffer=NULL;

  if (trace_flag) {
    sprintf(log_line, "Old Address %s == %d\n", "buffer", buffer);
    show_line(log_line, 0);
  }
  if (current_buf_size == buf_size) {  /* arbitrary limit */
/*    memory_error ("buffer", buf_size); */
/*    exit (1); */
    return buffer;    /* pass it back to TeX 99/Fabe/4 */
  }
  minsize =  current_buf_size / 100 * percent_grow;
  if (size < minsize) size = minsize;
  if (size < initial_buf_size) size = initial_buf_size;

  for (k = 0; k < MAXSPLITS; k++) {
    newsize = current_buf_size + size;
    if (newsize > buf_size) newsize = buf_size;
    n = (newsize + 1) * sizeof(ASCII_code);  /* buffer[buf_size + 1] */
    if (trace_flag) trace_memory("buffer", n);
    newbuffer = (ASCII_code *) REALLOC (buffer, n);
    if (newbuffer != NULL) break;   /* did we get it ? */
    if (current_buf_size == 0) break;   /* initial allocation must work */
    size = size / 2;          /* else can retry smaller */
  }

  if (newbuffer == NULL) {
    memory_error("buffer", n);
    return buffer;            /* try and continue !!! */
  }
  buffer = newbuffer;
  update_statistics ((int) buffer, n, current_buf_size);
#ifdef USEMEMSET
  memset(buffer + current_buf_size, 0, newsize - current_buf_size);
#else
  for (k = current_buf_size; k < newsize; k++) buffer[k]= 0;
#endif
  current_buf_size = newsize;
  if (trace_flag) {
    sprintf(log_line, "Current%s %d\n", "buffer", current_buf_size);
    show_line(log_line, 0);
    sprintf(log_line, "New Address %s == %d\n", "buffer", buffer);
    show_line(log_line, 0);
  }
  if (trace_flag)  probe_show();     /* 94/Mar/25 */
  return buffer;
}
#endif

/* we used to allocate this one only to reduce the size of the PE file */
/* not used anymore - NO advantage */

#ifdef ALLOCATEDVIBUF
eight_bits *allocatedvibuf (int size)
{
  eight_bits *dvi_buf;
  int n;

  n = (size + 1) * sizeof(eight_bits);
  if (trace_flag) trace_memory("dvi_buf", n);
  dvi_buf = (eight_bits *) malloc (roundup(n));
  if (dvi_buf == NULL) {
    memory_error("dvi_buf", n);
    return NULL;
  }
  if (trace_flag) {
    sprintf(log_line, "Address dvi_buf %d\n", dvi_buf);
    show_line(log_line, 0);
  }
  update_statistics ((int) dvi_buf, n, 0);
  if (trace_flag)  probe_show();     /* 94/Mar/25 */
  return dvi_buf;
}
#endif

/* we used to allocate this one only to reduce the size of the PE file */
/* it can be done without loss in performance, since register eqtb = zeqtb */
#ifdef ALLOCATEZEQTB
memory_word *allocatezeqtb (int k)
{
  memory_word *zeqtb;
  int n;

  n = k * sizeof (memory_word);  /* 13507 * 8 = 108 kilobytes */
  if (trace_flag)  trace_memory("eqtb", n);
  zeqtb = (memory_word *) malloc (roundup(n));
  if (zeqtb == NULL)  {
    memory_error("eqtb", n);
//    exit (1);           /* serious error */   
    return NULL;
  }
  if (trace_flag) {
    sprintf(log_line, "Address zeqtb %d\n", zeqtb);
    show_line(log_line, 0);
  }
  update_statistics ((int) zeqtb, n, 0);
  if (trace_flag)  probe_show();     /* 94/Mar/25 */
  return zeqtb;
}
#endif

/* here is the main memory allocation routine -- calls the above */
/* returns -1 if it fails */
/* allocate rather than static 93/Nov/26 */
int allocate_memory (void)
{
/*  int n;  */
#ifdef PREALLOCHOLE
  char *holeadr = malloc (300000);  /* testing - preallocate 95/Jan/20 */
#endif

#ifdef ALLOCATEHASH
  #error ERROR: Not ready for ALLOCATEHASH...
#endif

/* probably not worth while/not a good idea allocating following */
/* they are all rather small, and typically don't need expansion */
/* WE ASSUME THIS DOESN'T HAPPEN, SO WON'T BOTHER WITH UPDATESTATISTICS */
#ifdef ALLOCATEHASH
/*  n = 9767 * sizeof (twohalves);  *//* 60 kilo bytes */   
/*  n = (hash_size + 267) * sizeof (twohalves); */  /* 60 kilo bytes */
/*  n = (9767 + eqtb_extra) * sizeof (twohalves); */
#ifdef SHORTHASH
  n = (hash_size + 267 + eqtb_extra) * sizeof (htwohalves);   /* 95/Feb/19 */
  zzzae = (htwohalves *) malloc (roundup(n));
#else
  n = (hash_size + 267 + eqtb_extra) * sizeof (twohalves);  /* 95/Feb/19 */
  zzzae = (twohalves *) malloc (roundup(n));
#endif
  if (trace_flag)  trace_memory("hash table", n);
/*  zzzae = (twohalves *) malloc ((hash_size + 267) * sizeof (twohalves)); */
  if (zzzae == NULL)
  {
    memory_error("hash table", n);
//    exit (1);           /* serious error */
    return -1;            /* serious error */
  }

  n = (inputsize + 1) * sizeof(memory_word);

  if (trace_flag)
    trace_memory("input_stack", n);

/*  input_stack = (memory_word *) malloc ((inputsize + 1) * sizeof (memory_word)); */
  input_stack = (memory_word *) malloc (roundup(n));

  if (input_stack == NULL)
  {
    memory_error("input_stack", n);
//    exit (1);           /* serious error */
    return -1;            /* serious error */
  }
#endif

/* no real reason to allocate dvi_buf - no need to ever grow it */
#ifdef ALLOCATEDVIBUF
/*  zdvibuf = NULL; */
  zdvibuf = allocatedvibuf (dvi_buf_size);
  if (zdvibuf == NULL) return -1;
#endif

#ifdef ALLOCATEZEQTB
/*  zeqtb = NULL; */
#ifdef INCREASEFONTS
/*  zeqtb = allocatezeqtb (13507 + eqtb_extra); */  /* 94/Mar/29 */
  zeqtb = allocatezeqtb (hash_size + 4007 + eqtb_extra);  /* 94/Mar/29 */
#else
/*  zeqtb = allocatezeqtb (13507); */
  zeqtb = allocatezeqtb (hash_size + 4007); 
#endif
#endif

#ifdef ALLOCATEINPUTSTACK
  input_stack = NULL;        /* new 1999/Jan/21 */
  current_stack_size = 0;
  input_stack = realloc_input_stack (initial_stack_size);  /* + 1 */
#endif

#ifdef ALLOCATENESTSTACK
  nest = NULL;          /* new 1999/Jan/21 */
  current_nest_size = 0;
  nest = realloc_nest_stack (initial_nest_size);  /* + 1 */
#endif

#ifdef ALLOCATEPARAMSTACK
  param_stack = NULL;          /* new 1999/Jan/21 */
  current_param_size = 0;
  param_stack = realloc_param_stack (initial_param_size); /* + 1 */
#endif

#ifdef ALLOCATESAVESTACK
  save_stack = NULL;       /* new 1999/Jan/7 */
  current_save_size = 0;
  save_stack = realloc_save_stack (initial_save_size);
#endif

#ifdef IGNORED
  buffer = NULL;        /* need to do earlier */
  current_buf_size = 0;
  buffer = realloc_buffer (initial_buf_size);
#endif

#ifdef ALLOCATESTRING
  str_pool = NULL;
  current_pool_size = 0;
  str_start = NULL;
  current_max_strings = 0;
/*  need to create space because iniTeX writes in before reading pool file */
/*  for a start, puts in strings for 256 characters */
/*  maybe taylor allocations to actual pool file 1300 strings 27000 bytes ? */
  if (is_initex) {
    if (trace_flag) show_line("ini TeX pool and string allocation\n", 0);
    str_pool = realloc_str_pool (initial_pool_size); 
    str_start = realloc_str_start (initial_max_strings);
  }
#endif

/* the following can save a lot of the usual 800k fixed allocation */
#ifdef ALLOCATEFONT
  font_info = NULL;
  current_font_mem_size = 0;
/*  if not iniTeX, then do initial allocation on fmt file read in itex.c */
/*  if ini-TeX we need to do it here - no format file read later */
  if (is_initex) font_info = realloc_font_info (initial_font_mem_size);
#endif
    
#ifdef ALLOCATEMAIN
  mainmemory = NULL;
  zzzaa = NULL;
  mem_min = mem_bot;        /* just to avoid complaints in texbody */
  mem_top = mem_initex;
  mem_max = mem_top;
/*  allocate main memory here if this is iniTeX */
/*  otherwise wait for format undumping in itex.c ... */  
  if (is_initex) {
/*    avoid this if format specified on command line ??? */
/*    allocate_main_memory(mem_initex); */   /* made variable ! */
    mem = allocate_main_memory(mem_initex);  /* made variable ! */
    if (mem == NULL)
//      exit (1);
      return -1;            /* serious error */
  }
#endif

/* now for the hyphenation exception stuff */
#ifdef ALLOCATEHYPHEN
  hyph_word = NULL;
  hyph_list = NULL;
/*  this will be overridden later by what is in format file */
  hyphen_prime = default_hyphen_prime;
/*  non ini-TeX use assumes format will be read and that specifies size */
  if (is_initex)
  {
    if (new_hyphen_prime)
      hyphen_prime = new_hyphen_prime;
    if (realloc_hyphen(hyphen_prime)) /* allocate just in case no format */
      return -1;
  }
#endif

/*  now for memory for the part of the hyphenation stuff that always needed */
/*  if iniTeX, need to allocate pre-determined fixed amount - trie_size */
/*  if iniTeX not selected, allocate only enough later - undump in itex.c ! */
#ifdef ALLOCATETRIES
  if (is_initex)
  {
    if (allocate_tries (trie_size))
      return -1;
  }
#endif

/*  now for memory for hyphenation stuff needed only when running iniTeX */
#ifdef ALLOCATEINI
  if (is_initex)
  {
    if (allocate_ini(trie_size))
      return -1;
  }
  else
  {
    trie_l = trie_r = trie_o = trie_hash = NULL; /* (trie_size + 1) * integer */
    trie_c = NULL;       /* (trie_size + 1) * char */
    trie_taken = NULL;     /* (trie_size + 1) * bool */
  }
#endif
#ifdef PREALLOCHOLE
  free(holeadr);          /* create the hole */
#endif
  return 0;           // success
}

/* returns non-zero if error - done to test integrity of stack mostly */
/* free in reverse order 93/Nov/26 */
int free_memory (void)
{
  int n;
  unsigned heaptotal=0;
/*  unsigned total; */

  if (trace_flag) show_line("free_memory ", 0);

  if (verbose_flag || trace_flag)
    show_maximums(stdout); 

  if (trace_flag)
  {
    sprintf(log_line, "Heap total: %u bytes --- max address %u\n", 
        heaptotal, max_address);
    show_line(log_line, 0);
  }

  if (trace_flag) {
    sprintf(log_line, "Main Memory: variable node %d (%d - %d) one word %d (%d - %d)\n",
      lo_mem_max - mem_min, mem_min, lo_mem_max, mem_end  - hi_mem_min, hi_mem_min, mem_end);
    show_line(log_line, 0);
  }
/*  following only needed to check consistency of heap ... useful debugging */
  if (trace_flag) show_line("Freeing memory again\n", 0);

/*  if (trace_flag)
    show_line(log_line, "Zero Glue Reference Count %d\n", mem[0].hh.v.RH); */

/*  the following checks the heap integrity */

/*  if ((n = _heapchk ()) != _HEAPOK) { */      /* 94/Feb/18 */
  n = _HEAPOK;
#ifdef SHOWHEAPERROR
  n = _heapchk();
  if (n != _HEAPOK) {     /* 94/Feb/18 */
    sprintf(log_line, "WARNING: Heap corrupted (%d)\n", n);
    show_line(log_line, 1);
    sprintf(log_line, "HEAP %s (%s)\n", heapstrings[-n], "free_memory");
    show_line(log_line, 0);
    return n;   /* non-zero and negative */ /* unreachable ??? */
  }
#endif
/*  only free memory if safe ... additional check */
#ifdef ALLOCATEINI
  if (is_initex)
  {
    if (trie_taken != NULL) free(trie_taken);
    if (trie_hash  != NULL) free(trie_hash);
    if (trie_r     != NULL) free(trie_r);
    if (trie_c     != NULL) free(trie_c);
    if (trie_o     != NULL) free(trie_o);
    if (trie_l     != NULL) free(trie_l);
    trie_taken = NULL;
    trie_hash = trie_l = trie_r = NULL;
    trie_c = NULL;
    trie_o = NULL;
  }
#endif  
#ifdef ALLOCATETRIES
  if (trie_trc != NULL) free (trie_trc);
  if (trie_tro != NULL) free (trie_tro);
  if (trie_trl != NULL) free (trie_trl);
  trie_trc = NULL;
  trie_tro = trie_trl = NULL;
#endif
#ifdef ALLOCATEHYPHEN
  if (hyph_list != NULL) free(hyph_list);
  if (hyph_word != NULL) free(hyph_word);
  hyph_list = NULL;
  hyph_word = NULL;
#endif
#ifdef ALLOCATEMAIN
/*  if (zzzaa != NULL) free(zzzaa); */  /* NO: zzzaa may be offset ! */
  if (mainmemory != NULL) free(mainmemory);
  mainmemory = NULL;
#endif
#ifdef ALLOCATEFONT
  if (font_info != NULL) free(font_info);
  font_info = NULL;
#endif
#ifdef ALLOCATESTRING
  if (str_start != NULL) free(str_start);
  if (str_pool != NULL) free(str_pool);
  str_start = NULL;
  str_pool = NULL;
#endif

#ifdef ALLOCATEHASH
  if (zzzae != NULL) free(zzzae);
  zzzae = NULL;
#endif

#ifdef ALLOCATEDVIBUF
  if (zdvibuf != NULL) free(zdvibuf);
  zdvibuf = NULL;
#endif
#ifdef ALLOCATEZEQTB
  if (zeqtb != NULL) free(zeqtb);
  zeqtb = NULL;
#endif

#ifdef ALLOCATEPARAMSTACK
  if (param_stack != NULL) free(param_stack);
  param_stack = NULL;
#endif
#ifdef ALLOCATENESTSTACK
  if (nest != NULL) free(nest);
  nest = NULL;
#endif
#ifdef ALLOCATEINPUTSTACK
  if (input_stack != NULL) free(input_stack);
  input_stack = NULL;
#endif
#ifdef ALLOCATESAVESTACK
  if (save_stack != NULL) free(save_stack);
  save_stack = NULL;
#endif
/*  if (buffercopy != NULL) free (buffercopy); */ /* 94/Jun/27 */
  if (format_file != NULL) free(format_file);   /* 96/Jan/16 */
  if (string_file != NULL) free(string_file);   /* 96/Jan/16 */
  if (source_direct != NULL) free(source_direct); /* 98/Sep/29 */
  format_file = string_file = source_direct = NULL;
  if (dvi_file_name != NULL) free(dvi_file_name);
  if (log_file_name != NULL) free(log_file_name);
  log_file_name = dvi_file_name = NULL;       /* 00/Jun/18 */
  return 0;
}

bool prime (int x)
{
  int k;
  int sum = 1;    /* 1 + 3 + 5 + k = (k + 1) * (k + 1) / 4 */
  if (x % 2 == 0) return false;
  for (k = 3; k < x; k = k + 2) {
    if (x % k == 0) return false;
/*    if (k * k > x) return true; */
    if (sum * 4 > x) return true;
    sum += k;
  }
  return true;
}

int quitflag=0;
bool show_use=false;
bool floating=false;

void complainarg (int c, char *s)
{
  sprintf(log_line, "ERROR: Do not understand `%c' argument value `%s'\n", c, s);
  show_line(log_line, 1);
  show_use = 1;           // 2000 June 21
}

/* following is list of allowed command line flags and args */

/* char *allowedargs="+vitrdcyzpsqnwbfXABCDFGKLMNOQRSTYWZ?g=m=u=e=o=a=x=k=h=l=u=E=H="; */

/* only  01234567.9 still left to take ... maybe recycle u */

char *allowedargs="+bcdfijnpqrstvwyzABCDFGIJKLMNOPQRSTVWXYZ23456789?a=e=g=h=k=l=m=o=u=x=E=H=P=U=";

/* char takeargs="gmueoazhluEH"; */ /* subset that takes args! needed here */

void reorderargs (int ac, char **av)
{      /* put in 1993/Dec/28 */
  int n, m;
  char *s, *t;
//  char takeargs[128];   /* large enough for all command line arg chars */
  char takeargs[256];   /* large enough for all command line arg chars */

/*  assumes arg pointers av[] are writeable */
/*  for (n = 1; n < ac; n++) sprintf(log_line, "%s ", av[n]); */

  if (ac < 3) { /* need more than one arg to reorder anything 94/Feb/25 */
/*    show_line("No arguments?\n", 0); */  /* debugging */
    return;             /* no args ! */
  }

  s = allowedargs;
  t = takeargs;   /* list of those that take args */
  while (*s != '\0' && *(s+1) != '\0') {
    if (*(s+1) == '=') *t++ = *s++;   /* copy over --- without the = */
    s++;
  }
  *t = '\0';
  if (trace_flag) {
    show_line(takeargs, 0);
    show_char('\n');
  }
  
  n = 1;
  for (;;) {              /* scan to end of command line args */
    if (*av[n] != '-') break;
/*    does it take an argument ? and is this argument next ? */
    if (n+1 < ac &&
      *(av[n]+2) == '\0' &&
/*        strchr("gmuhleoxE", *(av[n]+1)) != NULL) */
        strchr(takeargs, *(av[n]+1)) != NULL)
          n += 2; /* step over it */
    else n++;
    if (n == ac) break;
  }

  for (;;) {              /* look for more command line args */
    if (n == ac) break;
    m = n;
/*    while (*av[m] != '-' && m < ac) m++; */ /* first command */
    while (m < ac && *av[m] != '-') m++;  /* first command */
    if (m == ac) break;
/* does it take an argument ? and is this argument next ? */
/* check first whether the `-x' is isolated, or arg follows directly */
/* then check whether this is one of those that takes an argument */
    if (m+1 < ac &&
      *(av[m]+2) == '\0' &&
        strchr(takeargs, *(av[m]+1)) != NULL) {
      s = av[m];      /*  move command down before non-command */
      t = av[m+1];
      for (; m > n; m--)  av[m+1] = av[m-1];
      av[n] = s;
      av[n+1] = t;
      n += 2;       /* step over moved args */
    }
    else {
      s = av[m];      /*  move command down before non-command */
      for (; m > n; m--)  av[m] = av[m-1];
      av[n] = s;
      n++;        /* step over moved args */
    }
  }
}

int test_align (int address, int size, char *name)
{
  int n;
  if (size > 4) n = address % 4;
  else n = address % size;
  if (n != 0) {
    sprintf(log_line, "OFFSET %d (ELEMENT %d) in %s\n", n, size, name);
    show_line(log_line, 0);
  }
  return n;
}

/* activate detailed checking of alignment when trace_flag is set */

void check_fixed_align (int flag)
{
  if (test_align ((int) &mem_top, 4, "FIXED ALIGNMENT")) {
    show_line("PLEASE RECOMPILE ME!\n", 1);
  }
#ifdef CHECKALIGNMENT
  if (!flag) return;
  test_align ((int) &mem_top, 4, "mem_top");
  test_align ((int) &mem_max, 4, "mem_max");
  test_align ((int) &mem_min, 4, "mem_min");
  test_align ((int) &bad, 4, "bad");
  test_align ((int) &trie_size, 4, "trie_size");
  test_align ((int) &xord, sizeof(xord[0]), "xord"); /* no op */
  test_align ((int) &xchr, sizeof(xchr[0]), "xchr"); /* no op */
  test_align ((int) &name_length, 4, "name_length");
  test_align ((int) &first, 4, "first");
  test_align ((int) &last, 4, "last");
  test_align ((int) &max_buf_stack, 4, "max_buf_stack");
  test_align ((int) &pool_ptr, 4, "pool_ptr");
  test_align ((int) &str_ptr, 4, "str_ptr");
  test_align ((int) &init_pool_ptr, 4, "init_pool_ptr");
  test_align ((int) &init_str_ptr, 4, "init_str_ptr");
  test_align ((int) &log_file, 4, "log_file");
  test_align ((int) &tally, 4, "tally");
  test_align ((int) &term_offset, 4, "term_offset");
  test_align ((int) &file_offset, 4, "file_offset");
  test_align ((int) &trick_count, 4, "trick_count");
  test_align ((int) &first_count, 4, "first_count");
  test_align ((int) &deletions_allowed, 4, "deletions_allowed");
  test_align ((int) &set_box_allowed, 4, "set_box_allowed");
  test_align ((int) &help_line, sizeof(help_line[0]), "help_line");
  test_align ((int) &use_err_help, 4, "use_err_help");
  test_align ((int) &interrupt, 4, "interrupt");
  test_align ((int) &OK_to_interrupt, 4, "OK_to_interrupt");
  test_align ((int) &arith_error, 4, "arith_error");
  test_align ((int) &tex_remainder, 4, "tex_remainder");
  test_align ((int) &temp_ptr, 4, "temp_ptr");
  test_align ((int) &lo_mem_max, 4, "lo_mem_max");
  test_align ((int) &hi_mem_min, 4, "hi_mem_min");
  test_align ((int) &var_used, 4, "var_used");
  test_align ((int) &dyn_used, 4, "dyn_used");
  test_align ((int) &avail, 4, "avail");
  test_align ((int) &mem_end, 4, "mem_end");
  test_align ((int) &mem_start, 4, "mem_start");
  test_align ((int) &rover, 4, "rover");
  test_align ((int) &font_in_short_display, 4, "font_in_short_display");
  test_align ((int) &depth_threshold, 4, "depth_threshold");
  test_align ((int) &breadth_max, 4, "breadth_max");
  test_align ((int) &nest, sizeof(nest[0]), "nest");

#ifdef ALLOCZEQTB
  test_align ((int) &zeqtb, sizeof(zeqtb[0]), "zeqtb");  /* not any more ? */
#endif
/*  test_align ((int) &xeq_level, sizeof(xeq_level[0]), "xeq_level"); */
  test_align ((int) &zzzad, sizeof(zzzad[0]), "zzzad");
/*  test_align ((int) &hash, sizeof(hash[0]), "hash"); */
  test_align ((int) &zzzae, sizeof(zzzae[0]), "zzzae");

  test_align ((int) &save_stack, sizeof(save_stack[0]), "save_stack");
  test_align ((int) &input_stack, sizeof(input_stack[0]), "input_stack");
  test_align ((int) &input_file, sizeof(input_file[0]), "input_file");
  test_align ((int) &line_stack, sizeof(line_stack[0]), "line_stack");
  test_align ((int) &param_stack, sizeof(param_stack[0]), "param_stack");
  test_align ((int) &cur_mark, sizeof(cur_mark[0]), "cur_mark");
  test_align ((int) &pstack, sizeof(pstack[0]), "pstack");
  test_align ((int) &read_file, sizeof(read_file[0]), "read_file");

  test_align ((int) &font_check, sizeof(font_check[0]), "font_check");
  test_align ((int) &font_size, sizeof(font_size[0]), "font_size");
  test_align ((int) &font_dsize, sizeof(font_dsize[0]), "font_dsize");
  test_align ((int) &font_params, sizeof(font_params[0]), "font_params");
  test_align ((int) &font_name, sizeof(font_name[0]), "font_name");
  test_align ((int) &font_area, sizeof(font_area[0]), "font_area");
  test_align ((int) &font_bc, sizeof(font_bc[0]), "font_bc");
  test_align ((int) &font_ec, sizeof(font_ec[0]), "font_ec");
  test_align ((int) &font_glue, sizeof(font_glue[0]), "font_glue");
  test_align ((int) &font_used, sizeof(font_used[0]), "font_used");
  test_align ((int) &hyphen_char, sizeof(hyphen_char[0]), "hyphen_char");
  test_align ((int) &skew_char, sizeof(skew_char[0]), "skew_char");
  test_align ((int) &bchar_label, sizeof(bchar_label[0]), "bchar_label");
  test_align ((int) &font_bchar, sizeof(font_bchar[0]), "font_bchar");
  test_align ((int) &font_false_bchar, sizeof(font_false_bchar[0]), "font_false_bchar");
  test_align ((int) &char_base, sizeof(char_base[0]), "char_base");
  test_align ((int) &width_base, sizeof(width_base[0]), "width_base");
  test_align ((int) &height_base, sizeof(height_base[0]), "height_base");
  test_align ((int) &depth_base, sizeof(depth_base[0]), "depth_base");
  test_align ((int) &italic_base, sizeof(italic_base[0]), "italic_base");
  test_align ((int) &lig_kern_base, sizeof(lig_kern_base[0]), "lig_kern_base");
  test_align ((int) &kern_base, sizeof(kern_base[0]), "kern_base");
  test_align ((int) &exten_base, sizeof(exten_base[0]), "exten_base");
  test_align ((int) &param_base, sizeof(param_base[0]), "param_base");

#ifdef ALLOCATEDVIBUF
  test_align ((int) &zdvibuf, sizeof(zdvibuf[0]), "zdvibuf"); /* no op */
#endif
  test_align ((int) &total_stretch, sizeof(total_stretch[0]), "total_stretch");
  test_align ((int) &total_shrink, sizeof(total_shrink[0]), "total_shrink");
  test_align ((int) &active_width, sizeof(active_width[0]), "active_width");
  test_align ((int) &cur_active_width, sizeof(cur_active_width[0]), "cur_active_width");
  test_align ((int) &background, sizeof(background[0]), "background");
  test_align ((int) &break_width, sizeof(break_width[0]), "break_width");
  test_align ((int) &minimal_demerits, sizeof(minimal_demerits[0]), "minimal_demerits");
  test_align ((int) &best_place, sizeof(best_place[0]), "best_place");
  test_align ((int) &best_pl_line, sizeof(best_pl_line[0]), "best_pl_line");
  test_align ((int) &hc, sizeof(hc[0]), "hc");
  test_align ((int) &hu, sizeof(hu[0]), "hu");
  test_align ((int) &hyf, sizeof(hyf[0]), "hyf");
/*  test_align ((int) &x, sizeof(x[0]), "x"); */

  test_align ((int) &hyf_distance, sizeof(hyf_distance[0]), "hyf_distance");
  test_align ((int) &hyf_num, sizeof(hyf_num[0]), "hyf_num");
  test_align ((int) &hyf_next, sizeof(hyf_next[0]), "hyf_next");
  test_align ((int) &op_start, sizeof(op_start[0]), "op_start");

/*  test_align ((int) &trie_op_hash, sizeof(trie_op_hash[0]), "trie_op_hash"); */
  test_align ((int) &zzzaf, sizeof(zzzaf[0]), "zzzaf");
  test_align ((int) &trie_used, sizeof(trie_used[0]), "trie_used");
/*  test_align ((int) &trie_op_lang, sizeof(trie_op_lang[0]), "trie_op_lang");*/
  test_align ((int) &trie_op_val, sizeof(trie_op_val[0]), "trie_op_val");

  test_align ((int) &trie_min, sizeof(trie_min[0]), "trie_min");
  test_align ((int) &page_so_far, sizeof(page_so_far[0]), "page_so_far");
  test_align ((int) &write_file, sizeof(write_file[0]), "write_file");
  test_align ((int) &write_open, sizeof(write_open[0]), "write_open");
#endif
}

void check_alloc_align (int flag) {
  if (test_align ((int) eqtb, sizeof(eqtb[0]), "ALLOCATED ALIGNMENT"))
    show_line("PLEASE RECOMPILE ME!\n", 1);
#ifdef CHECKALIGNMENT
  if (!flag) return;
#ifndef ALLOCZEQTB
  test_align ((int) zeqtb, sizeof(zeqtb[0]), "zeqtb"); 
#endif
#ifndef ALLOCATEDVIBUF
  test_align ((int) &zdvibuf, sizeof(zdvibuf[0]), "zdvibuf");  /* no op */
#endif
  test_align ((int) str_pool, sizeof(str_pool[0]), "str_pool"); /* no op */
  test_align ((int) str_start, sizeof(str_start[0]), "str_start");
  test_align ((int) zmem, sizeof(zmem[0]), "main memory");
  test_align ((int) font_info, sizeof(font_info[0]), "font memory");
  test_align ((int) trie_trl, sizeof(trie_trl[0]), "trie_trl");
  test_align ((int) trie_tro, sizeof(trie_tro[0]), "trie_tro");
  test_align ((int) trie_trc, sizeof(trie_trc[0]), "trie_trc");
  test_align ((int) hyph_word, sizeof(hyph_word[0]), "hyph_word");
  test_align ((int) hyph_list, sizeof(hyph_list[0]), "hyph_list");
/*  test_align ((int) trie_c, sizeof(trie_c[0]), "trie_c"); *//* no op */
  test_align ((int) trie_o, sizeof(trie_o[0]), "trie_o");
  test_align ((int) trie_l, sizeof(trie_l[0]), "trie_l");
  test_align ((int) trie_r, sizeof(trie_r[0]), "trie_r");
  test_align ((int) trie_hash, sizeof(trie_hash[0]), "trie_hash");
  test_align ((int) trie_taken, sizeof(trie_taken[0]), "trie_taken");
#endif
}

#ifdef HEAPSHOW
void showaddresses (void)
{         /* 96/Jan/20 */
  int c;
  int d;
  sprintf(log_line, "STACK %d %d (grows %s) ", &c, &d, (&d > &c) ? "upward" : "downward");
  show_line(log_line, 0);
  sprintf(log_line, "eqtb %d hash %d ", zeqtb, zzzae);
  show_line(log_line, 0);
  sprintf(log_line, "dvi_buf %d xchr %d xord %d nest %d\n", zdvibuf, xchr, xord, nest);
  show_line(log_line, 0);
  sprintf(log_line, "save_stack %d input_stack %d line_stack %d param_stack %d\n",
       save_stack, input_stack, line_stack, param_stack);
  show_line(log_line, 0);
  sprintf(log_line, "font_check %d font_size %d font_dsize %d font_params %d font_name %d\n",
       font_check, font_size, font_dsize, font_params, font_name);
  show_line(log_line, 0);
  sprintf(log_line, "main %d font_info %d str_pool %d str_start %d hyph_word %d hyph_list %d\n",
      zmem, font_info, str_pool, str_start, hyph_word, hyph_list);
  show_line(log_line, 0);
}
#endif

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env var' from `dviwindo.ini' - or from DOS environment 94/May/19  */
/* controlled by USEDVIWINDOINI environment variable            94/June/19 */

bool usedviwindo        = true;               /* use [Environment] section in `dviwindo.ini' */
bool backwardflag       = false;              /* don't cripple all advanced features */
bool shorten_file_name  = false;              /* don't shorten file names to 8+3 for DOS */
char *inifilename       = "dviwindo.ini";     /* name of ini file we look for */
char *dviwindo          = "";                 /* full file name for dviwindo.ini with path */
char *envsection        = "[Environment]";    /* Env var section in `dviwindo.ini' */
char *wndsection        = "[Window]";         /* Window section in `dviwindo.ini' */
char *workdirect        = "WorkingDirectory"; /* key in [Window] section */
bool usesourcedirectory = true;               /* use source file directory as local when WorkingDirectory is set */
bool workingdirectory   = false;              /* if working directory set in ini */

/* cache to prevent allocating twice in a row */

char *lastname=NULL, *lastvalue=NULL;

/* get value of env var - try first in dviwindo.ini then DOS env */
/* returns allocated string -- these strings are not freed again */
/* is it safe to do that now ? 98/Jan/31 */
char *grabenv (char *varname)
{
  char line[PATH_MAX];
  FILE *pinput;
  char *s;
  int m, n;

  if (varname == NULL) return NULL;    /* sanity check */
  if (*varname == '\0') return NULL;   /* sanity check */
/*  speedup to avoid double lookup when called from set_paths in ourpaths.c */
/*  if (lastname != NULL && strcmp(lastname, varname) == 0) { */
  if (lastname != NULL && _strcmpi(lastname, varname) == 0)
  {
    if (trace_flag)
    {
      sprintf(log_line, "Cache hit: %s=%s\n", lastname, lastvalue);
      show_line(log_line, 0);
    }
/*    return lastvalue; */        /* save some time here */
    return xstrdup(lastvalue);
/*    duplicate so can free safely 98/Jan/31 */
  }

/*  hmm, following was not xstrdup(...) */ /* not cached */
  if (usedviwindo == 0 || *dviwindo == '\0') {
/*    return getenv(varname); */
    s = getenv(varname);
    if (s == NULL) return NULL;
    else return xstrdup(s);       /* make safe 98/Jan/31 */
  }

  if (share_flag == 0) pinput = fopen(dviwindo, "r");
  else pinput = _fsopen(dviwindo, "r", share_flag);

  if (pinput != NULL) {
    m = strlen(envsection);
/*    search for [Environment] section */ /* should be case insensitive */
    while (fgets (line, sizeof(line), pinput) != NULL) {
      if (*line == ';') continue;
      if (*line == '\n') continue;
      if (_strnicmp(line, envsection, m) == 0) {  /* 98/Jan/31 */
/*        search for varname=... */ /* should be case sensitive ? */
        n = strlen(varname);
        while (fgets (line, sizeof(line), pinput) != NULL) {
          if (*line == ';') continue;
          if (*line == '[') break;
/*          if (*line == '\n') break; */  /* ??? */
          if (*line <= ' ') continue;   /* 95/June/23 */
/*          if (strncmp(line, varname, n) == 0 && */
          if (_strnicmp(line, varname, n) == 0 &&
            *(line+n) == '=') { /* found it ? */
              (void) fclose (pinput);
/*              flush trailing white space */
              s = line + strlen(line) - 1;
              while (*s <= ' ' && s > line) *s-- = '\0';
              if (trace_flag) { /* DEBUGGING ONLY */
                sprintf(log_line, "%s=%s\n", varname, line+n+1);
                show_line(log_line, 0);
              }
              s = line+n+1;
              if (lastname != NULL) free(lastname);
              lastname = xstrdup (varname);
              if (lastvalue != NULL) free(lastvalue);
              lastvalue = xstrdup(s);
              return xstrdup(s);    /* 98/Jan/31 */
          }   /* end of matching varname */
        }     /* end of while fgets */
/*        break; */ /* ? not found in designated section */    
      }       /* end of search for [Environment] section */
    }
    (void) fclose (pinput);
  }           /* end of if fopen */
  s = getenv(varname);    /* failed, so try and get from environment */
/*  if (s != NULL) return s;  */
  if (s != NULL) {
/*    sdup = xstrdup(s); */   /* just to be safe --- 1995/Jan/31 */
    if (lastname != NULL) free(lastname);
    lastname = xstrdup (varname);
    if (lastvalue != NULL) free(lastvalue);
    lastvalue = xstrdup(s);   /* remember in case asked again ... */
/*    return sdup; */
    return xstrdup(s);    /* 98/Jan/31 */
  }
  else return NULL;   /* return NULL if not found anywhere */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void flush_trailing_slash (char *directory)
{
  char *s;
/*  flush trailing \ or / in directory, if any 1993/Dec/12 */
  if (strcmp(directory, "") != 0) {
    s = directory + strlen(directory) - 1;
    if (*s == '\\' || *s == '/') *s = '\0';
  }
}

void knuthify (void)
{
/*  show_current = false; */ /* show ultimate limits */
/*  reorder_arg_flag = false; */ /* don't reorder command line */
/*  deslash = false; */ /* don't unixify file names */
/*  return_flag = false; */ /* don't allow just ^^M termination */
/*  trimeof = false; */ /* don't trim ^^Z Ctrl-Z at end of file */
  restrict_to_ascii = false; /* don't complain non ASCII */
  allow_patterns    = false; /* don't allow pattern redefinition */
  show_in_hex       = true;  /* show character code in hex */
  show_in_dos       = false; /* redundant with previous */
  show_numeric      = false; /* don't show character code decimal */
  show_missing      = false; /* don't show missing characters */
  civilize_flag     = false; /* don't reorder date fields */
  c_style_flag      = false; /* don't add file name to error msg */
  show_fmt_flag     = false; /* don't show format file in log */
  show_tfm_flag     = false; /* don't show metric file in log */
/* font_max = 255; */ /* revert to TeX 82 limit */
/* if you want this, change in tex.h definition of font_max to `int' */
/* and add define FONTMAX 511, and in local.c add font_max = FONTMAX; */
  tab_step = 0;
  show_line_break_stats = false;   /* do not show line break stats */
  show_fonts_used = false;
  default_rule = 26214;      /* revert to default rule thickness */
  pseudo_tilde = false;
  pseudo_space = false;
  show_texinput_flag = false;
  truncate_long_lines = false;
  allow_quoted_names = false;
  show_cs_names = false;
  font_dimen_zero = false;      /* 98/Oct/5 */
  ignore_frozen = false;     /* 98/Oct/5 */
  suppress_f_ligs = false;      /* 99/Jan/5 */
  full_file_name_flag = false;   // 00 Jun 18
  save_strings_flag = false;    // 00 Aug 15
  knuth_flag = true;       /* so other code can know about this */
} /* end of knuthify */

/* following have already been used up */

/* abcdefghijklmnopqrstuvwxyz */

/* ABCDEFGHIJKLMNOPQRSTUVWXYZ */

/* ........ */

int nohandler = 0;    /* experiment to avoid Ctrl-C interrupt handler */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following made global so analyze_flag can be made separate procedure */

// char *xchrfile=""; /* save space use xstrdup */
char *xchrfile = NULL; /* save space use xstrdup */
// char *replfile="";/* save space use xstrdup */
char *replfile = NULL;/* save space use xstrdup */

/* abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ */

/* analyze command line flag or argument */
/* c is the flag letter itself, while optarg is start of arg if any */

/* when making changes, revise allowedargs */


int analyze_flag (int c, char *optarg)
{
  switch (c)
  {
    case 'v':
      want_version = true;
      verbose_flag = true;
      break;
    case 'i':
      is_initex = true;
      break;
    case 'Q':
      interaction = batch_mode; /* quiet mode */
      break;
    case 'R':
      interaction = nonstop_mode; /* run mode */
      break;
    case 'S':
      interaction = scroll_mode; /* scroll mode */
      break;
    case 'T':
      interaction = error_stop_mode; /* tex mode */
      break;
    case 'K':
      backwardflag = true; /* 94/Jun/15 */
      knuthify();         /* revert to `standard' Knuth TeX */
      break;
    case 'L':
      c_style_flag = true; /* C style error msg 94/Mar/21 */
      break;
    case 'Z':
      show_tfm_flag = true; /* show TFM in log file 94/Jun/21 */
      break;
    case 'c':
      current_tfm = false; /* not look current dir for TFM */
      break;
    case 'C':
      current_flag = false; /* not look current dir for files */
      break;
    case 'M':
      show_missing = false; /* do not show missing 94/June/10 */
      break;
    case 'd':
      deslash = false; /* flipped 93/Nov/18 */
      /* pseudo_tilde = 0; */ /* new 95/Sep/26 */
      break;
    case 'p':
      allow_patterns = true; /* 93/Nov/26 */
      /* reset_exceptions = true; */ /* 93/Dec/23 */
      break;
/*  case 'w':  show_in_hex = false; */ /* 94/Jan/26 */
    case 'w':
      show_in_hex = true; /* flipped 00/Jun/18 */
      break;
    case 'j':
      show_in_dos = true; /* 96/Jan/26 */
      break;
    case 'n':
      restrict_to_ascii = true; /* 0 - 127 1994/Jan/21 */
      break;
    case '6':
      workingdirectory = true; /* use source dir 98/Sep/29 */
      break;
    case '7':
      usesourcedirectory = false; /* use working dir 98/Sep/29 */
      break;
    case 'f':
      show_fonts_used = false; /* 97/Dec/24 */
      break;
    case '8':
      shorten_file_name = true; /* 95/Feb/20 */
      break;
    case '9':
      show_cs_names = true; /* 98/Mar/31 */
      break;
    case '4':
      ignore_frozen = true; /* 98/Oct/5 */
      break;
    case '5':
      font_dimen_zero = false; /* 98/Oct/5 */
      break;
    case 'F':
      show_texinput_flag = false; /* 98/Jan/28 */
      break;
/*  case 'X':  truncate_long_lines = false; */ /* 98/Feb/2 */
              /* break; */
    case 'W':
      usedviwindo = false; /* 94/May/19 */
      break;
    case 'J':
      show_line_break_stats = false; /* 96/Feb/8 */
      break;
    case 'O':
      show_fmt_flag = false; /* 94/Jun/21 */
      break;
    case 'I':
      format_specific = false; /* 95/Jan/7 */
      break;
    case '3':
      encoding_specific = false; /* 98/Oct/5 */
      break;
    case '2':
      suppress_f_ligs = true; /* 99/Jan/5 f-lig */
      break;
/* following are pretty obscure */
/*  case 'y': cache_file_flag = false; */ /* 96/Nov/16 */
/*            break; */
/*  case 'r': return_flag = false; */ /* flipped 93/Nov/18 */
/*            break; */
/*  case 'z': trimeof = false; */ /* 93/Nov/24 */
/*            break; */
    case 'z':
      full_file_name_flag = false; // 00 Jun 18
      break;
    case 'X':
      save_strings_flag = false; // 00 Aug 15
      break;
/* following are unannounced options */ /* some may be recycled ... */
    case 't':
      trace_flag = true;
      break;
    case 'q':
      quitflag++; /* 93/Dec/16 */
      break;
/* The following are really obscure and should not be advertized */
    case 's':
      show_current = false; /* tex8 93/Dec/14 */
      break;
    case 'N':
      show_numeric = false; /* 93/Dec/21 */
      break;
    case 'A':
      civilize_flag = false; /* 93/Dec/16 */
      break; 
    case 'B':
      open_trace_flag = true; /* openinou 1994/Jan/8 */
      break;
    case 'Y':
      reorder_arg_flag = false; /* local */
      break;
    case 'b':
      test_dir_access = false; /* 94/Feb/10 */
      break;
    case 'D':
      dir_method = false; /* 94/Feb/10 */
      break;
    case 'G':
      file_method = false; /* 94/Feb/13 */
      break;
//  case 'V': share_flag = _SH_DENYNO; break; /* 0x40 - deny none mode */ 
/*  case 'X': nohandler++; break; */
/*  case 'f': waitflush = false; break; */
/*  case 'F': floating = true; break; */
/* *********** following command line options take arguments **************  */
    case 'm':
      if (optarg == 0)
        mem_initex = mem_top;
      else
        mem_initex = atoi(optarg) * 1024; /* 93 Dec/1 */
      if (mem_initex == 0)
        complainarg(c, optarg);
      mem_spec_flag = 1;
      break;
#ifdef VARIABLETRIESIZE
    case 'h':
      if (optarg == 0)
        trie_size = default_trie_size;
      else
        trie_size = atoi(optarg); /* 93 Dec/1 */
      if (trie_size == 0)
        complainarg(c, optarg);
      break;
#endif
#ifdef ALLOCATEHYPHEN
    case 'e':
      if (optarg == 0)
        new_hyphen_prime = hyphen_prime * 2;
      else
        new_hyphen_prime = atoi(optarg); /* 93/Nov/26 */
      if (new_hyphen_prime == 0)
        complainarg(c, optarg);
      break;
#endif
#ifdef ALLOCATEDVIBUF
    case 'u':
      if (optarg == 0)
        dvi_buf_size = default_dvi_buf_size;
      else
        dvi_buf_size = atoi(optarg); /* 94/Mar/24 */
      if (dvi_buf_size == 0)
        complainarg(c, optarg);
      break;
#endif
    case 'g':
      if (optarg == 0)
        percent_grow = 62;
      else
        percent_grow = atoi(optarg); /* 93/Dec/11 */
      if (percent_grow == 0)
        complainarg(c, optarg);
      break;
    case 'U':
      if (optarg == 0)
        pseudo_tilde = 0;
      else
        pseudo_tilde = atoi(optarg); /* 95/Sep/26 */
      if (pseudo_tilde > 255)
        pseudo_tilde = 255;
      else if (pseudo_tilde < 128)
        pseudo_tilde = 128;
      break;
/*  case 'H':  if (optarg == 0) heapthreshold = 1024; else heapthreshold = atoi(optarg);
 *             if (heapthreshold == 0) complainarg(c, optarg); else heap_flag = 1;
 *             break; */
    case 'H':
      if (optarg == 0)
        tab_step = 8;
      else
        tab_step = atoi(optarg); /* 94/July/3 */
      if (tab_step == 0)
        complainarg(c, optarg);
      break;
    case 'x':
      if (optarg == 0)
        xchrfile = xstrdup("xchr.map");
      else
        xchrfile = xstrdup(optarg);
      if (xchrfile == NULL || *xchrfile == '\0')
        complainarg(c, optarg);
      break;
    case 'k':
      if (optarg == 0)
        replfile = xstrdup("repl.key");
      else
        replfile = xstrdup(optarg);
      if (replfile == NULL || *replfile == '\0')
        complainarg(c, optarg);
      break;
/* more obscure stuff - ppssibly recycle */
    case 'P':
      if (optarg == 0)
        default_rule = 26214; /* 95/Oct/9 */
      else
        default_rule = atoi(optarg); /* 95/Oct/9 */
      if (default_rule == 0)
        complainarg(c, optarg);
      break;
    case 'E':
      if (optarg != 0)
        putenv(optarg);
      else
        complainarg(c, optarg);
      break;
    case 'o':
      if (optarg == 0)
        dvi_directory = "";
      else
        dvi_directory = xstrdup(optarg);
      if (strcmp(dvi_directory, "") == 0)
        complainarg(c, optarg);
      break;
    case 'l':
      if (optarg == 0)
        log_directory = "";
      else
        log_directory = xstrdup(optarg);
      if (strcmp(log_directory, "") == 0)
        complainarg(c, optarg);
      break;
    case 'a':
      if (optarg == 0)
        aux_directory = "";
      else
        aux_directory = xstrdup(optarg);
      if (strcmp(aux_directory, "") == 0)
        complainarg(c, optarg);
      break;
    case '?':
    default:
      show_use = true;
      return -1; // failed to recognize
      break;
  }
  return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
//char *yytexcmd="yandytex.cmd";
char *yytexcmd="YANDYTEX.CMD";    /* name of command line file */

/* Try and read default command file - YANDYTEX.CMD */
/* in current directory and then in directory of YANDYTEX */
/* (does not make sense in TeX file directory) */
/* since we don't yet know where that is ! */
/* can't conveniently include this in output file either - not open yet */

/* used both for yytex.cmd and @ indirect command line files */
/* can this be reentered ? */

/* supply extension if none */
void yy_extension (char *fname, char *ext)
{
  char *s, *t;
  if ((s = strrchr(fname, '.')) == NULL ||
    ((t = strrchr(fname, '\\')) != NULL && s < t)) {
      strcat(fname, ".");
      strcat(fname, ext);
  }
}

/* remove file name - keep only path - inserts '\0' to terminate */

void strip_name (char *pathname)
{
  char *s;
  if ((s = strrchr(pathname, '\\')) != NULL);
  else if ((s = strrchr(pathname, '/')) != NULL);
  else if ((s = strrchr(pathname, ':')) != NULL) s++;
  else s = pathname;
  *s = '\0';
}

/* char commandfile[PATH_MAX]; */ /* keep around so can open later */

char *programpath = ""; /* pathname of program */
                    /* redundant with texpath ? */

/* The following does not deslashify arguments ? Do we need to ? */

int read_commands (char *filename)
{
  char commandfile[PATH_MAX]; 
  FILE *command;
  char line[PATH_MAX];
  char *linedup;      /* need to copy line to preserve args */
  char *s;
/*  char *sn; */
  char *optarg;
  int c;

/*  Try first in current directory (or use full name as specified) */
  strcpy(commandfile, filename);
  yy_extension(commandfile, "cmd");
  if (share_flag == 0)
	  command = fopen(commandfile, "r");
  else
	  command = _fsopen(commandfile, "r", share_flag);
  if (command == NULL) {
/*    If that fails, try in YANDYTeX program directory */
    strcpy(commandfile, programpath);
/*    don't need fancy footwork, since programpath non-empty */
    strcat(commandfile, "\\");
    strcat(commandfile, filename);
    yy_extension(commandfile, "cmd");
    if (share_flag == 0)
		command = fopen(commandfile, "r");
    else
		command = _fsopen(commandfile, "r", share_flag);
    if (command == NULL) {
/*      perrormod(commandfile); */      /* debugging only */
/*      strcpy(commandfile, ""); */   /* indicate failed */
      return 0;       /* no command line file YYTEX.CMD */
    }
  }

/*  allow for multiple lines --- ignore args that don't start with `-' */
  while (fgets(line, PATH_MAX, command) != NULL) {
/*    sprintf(log_line, "From %s:\t%s", commandfile, line); */
/*    skip over comment lines and blank lines */
    if (*line == '%' || *line == ';' || *line == '\n') continue;
    if (strchr(line, '\n') == NULL) strcat(line, "\n");
/*    sfplogline, rintf("From %s:\t%s", commandfile, line); */
    linedup = xstrdup (line);         /* 93/Nov/15 */
    if (linedup == NULL) {
      show_line("ERROR: out of memory\n", 1);    /* read_commands */
//      exit(1);
      return -1;    // failure
    }
    s = strtok(linedup, " \t\n\r");       /* 93/Nov/15 */
    while (s != NULL) {
      if (*s == '-' || *s == '/') {
        c = *(s+1);
        optarg = s+2;
/*        if (*optarg = '=') optarg++; */
        if (*optarg == '=') optarg++;
        if (analyze_flag(c, optarg) < 0) return -1;  // failure ???
      }
/*      else break; */              /* ignore non-flag items */
      s = strtok(NULL, " \t\n\r");      /* go to next token */
    }
/*    If you want to see command lines in file - put -v in the file */
/*    if (verbose_flag != 0) sprintf(log_line, "From %s:\t%s", commandfile, line); */
  }
  (void) fclose(command);   /* no longer needed */
  return 1;       // success
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* try and read commands on command line */
int read_command_line (int ac, char **av)
{ 
  int c;
  char *optargnew;  /* equal to optarg, unless that starts with `='      */
                    /* in which case it is optarg+1 to step over the `=' */
                    /* if optarg = 0, then optargnew = 0 also            */

//  show_line("read_command_line\n", 0);
  if (ac < 2) return 0;     /* no args to analyze ? 94/Apr/10 */

/*  while ((c = getopt(ac, av, "+vitrdczp?m:h:x:E:")) != EOF) {              */
/*  NOTE: keep `Y' in there for `do not reorder arguments !                  */
/*  WARNING: if adding flags, change also `allowedargs' and  `takeargs' !!!! */
  while ((c = getopt(ac, av, allowedargs)) != EOF) {
    if (optarg != 0 && *optarg == '=')
      optargnew = optarg+1;
    else
      optargnew = optarg;
    analyze_flag (c, optargnew);
  }
  if (show_use || quitflag == 3) {
//    showversion (stdout);
    stamp_it(log_line);
    strcat(log_line, "\n");
    show_line(log_line, 0);
    stampcopy(log_line);
    strcat(log_line, "\n");
    show_line(log_line, 0);
    if (show_use) show_usage();
    else if (quitflag == 3) {
      strcat(log_line, "\n");
      show_line(log_line, 0);
    }
//    exit (0);
    return -1;        // failure
  } 
#ifdef DEBUG
  if (floating) testfloating();   /* debugging */
#endif

  if (replfile != NULL && *replfile != '\0') {  /* read user defined replacement */
    if (read_xchr_file(replfile, 1, av)) {
      if (trace_flag) show_line("KEY REPLACE ON\n", 0);
      key_replace = true;
    }
  } 
/*  key_replace used in texmf.c (input_line) */
  if (xchrfile != NULL && *xchrfile != '\0') {  /* read user defined xchr[] */
    if (read_xchr_file(xchrfile, 0, av)) {
      if (trace_flag) show_line("NON ASCII ON\n", 0);
      non_ascii = true;
    }
  } 
/*  non_ascii used in texmf.c (t_open_in & input_line & call_edit) */
/*  see also xchr [] & xord [] use in tex3.c and itex.c */
  return 0;
}

int init_commands (int ac, char **av)
{
/*  NOTE: some defaults changed 1993/Nov/18 */
/*  want_version = show_use = switchflag = return_flag = false;
  is_initex = trace_flag = deslash = non_ascii = false; */
  is_initex         = false; /* check for dumping format file */
  allow_patterns    = false; /* using \pattern after format file loaded */
  reset_exceptions  = false;
  non_ascii         = false;
  key_replace       = false;
  want_version      = false;
  open_trace_flag   = false;
  trace_flag        = false;
  verbose_flag      = false;
  heap_flag         = false;
  restrict_to_ascii = false;
  show_in_hex       = false; /* default is not to show as hex code ^^ 00/Jun/18 */
  show_in_dos       = false; /* default is not to translate to DOS 850 */ 
  return_flag       = true;  // hard wired now
  trimeof           = true;  // hard wired now
  deslash           = true;
  pseudo_tilde      = 254;   /* default '~' replace 95/Sep/26 filledbox DOS 850 */
  pseudo_space      = 255;   /* default ' ' replace 97/June/5 nbspace DOS 850 */
  default_rule      = 26214; /* default rule variable 95/Oct/9 */
  show_current      = true;
  civilize_flag     = true;
  show_numeric      = true;
  show_missing      = true;
  current_flag      = true;
  current_tfm       = true;  /* search for TFMs in current dir as well */
  test_dir_access   = true;  /* test if readable item is perhaps a sub-dir */
  dir_method        = true;  /* in dir_p: _findfirst instead of use fopen (nul) */
  file_method       = true;  /* use file_p (_findfirst) not readable (access) */
/*  waitflush = true; */  /* flushed 97/Dec/24 */
  c_style_flag      = false; /* use c-style error output */
  show_fmt_flag     = true;  /* show format file in log */
  show_tfm_flag     = false; /* don't show metric file in log */
  shorten_file_name     = false; /* don't shorten file names to 8+3 */
  show_texinput_flag    = true;  /* show TEXINPUTS and TEXFONTS */
  truncate_long_lines   = true; /* truncate long lines */
  tab_step              = 0;      /* do not replace tabs with spaces */
  format_specific       = true;  /* do format specific TEXINPUTS 95/Jan/7 */
  encoding_specific     = true;  /* do encoding specific TEXFONTS 98/Jan/31 */
  show_line_break_stats = true;  /* show line break statistics 96/Feb/8 */
  show_fonts_used       = true; /* show fonts used in LOG file 97/Dec/24 */
  allow_quoted_names    = true;  /* allow quoted names with spaces 98/Mar/15 */
  show_cs_names         = false;  /* don't show csnames on start 98/Mar/31 */
  knuth_flag            = false;    /* allow extensions to TeX */
  cache_file_flag       = true; /* default is to cache full file names 96/Nov/16 */
  full_file_name_flag   = true;  /* new default 2000 June 18 */
  save_strings_flag     = true; // 2000 Aug 15
  errout                = stdout;    /* as opposed to stderr say --- used ??? */
  abort_flag            = 0;      // not yet hooked up ???
  err_level             = 0;     // not yet hooked up ???
  new_hyphen_prime      = 0;
#ifdef VARIABLETRIESIZE
/*  trie_size = default_trie_size; */
  trie_size = 0;
#endif
  mem_extra_high = 0;
  mem_extra_low  = 0;
  mem_initex     = 0;
#ifdef ALLOCATEDVIBUF
  dvi_buf_size = 0;
#endif
/*  share_flag = _SH_DENYNO; */ /* 0x40 - deny none mode */
/*  share_flag = _SH_COMPAT; */ /* 0x00 - compatability mode */
  share_flag = 0; /* revert to fopen for now */

/*  strncpy(programpath, argv[0], PATH_MAX); */ /* 94/July/12 */
  programpath = xstrdup(av[0]); /* extract path executable */
  strip_name(programpath); /* strip off yandytex.exe */

  format_name = "plain"; /* format name if specified on command line */

  encoding_name = "";

  if (read_command_line(ac, av) < 0)  /* move out to subr 94/Apr/10 */
    return -1;            // in case of error

  if (optind == 0)
    optind = ac;

  if (want_version)
  {
    stamp_it(log_line);
    strcat(log_line, "\n");
    show_line(log_line, 0);
    stampcopy(log_line);
    strcat(log_line, "\n");
    show_line(log_line, 0);
  }

/*  if we aren't including current directory in any directory lists */
/*  then makes no sense to avoid them separately for TFM files ... */
/*  (that is, the ./ is already omitted from the dir list in that case */
  if (!current_flag && !current_tfm)
    current_tfm = true; /* 94/Jan/24 */

  return 0;
}

/* E sets environment variable ? */

void initial_memory (void)
{
  /* set initial memory allocations */
  if (mem_extra_high < 0)
    mem_extra_high = 0;

  if (mem_extra_low < 0)
    mem_extra_low = 0;

  if (mem_initex < 0)
    mem_initex = 0;

  if (is_initex)
  {
 #if defined(ALLOCATEHIGH) || defined(ALLOCATELOW)
    if (mem_extra_high != 0 || mem_extra_low != 0)
    {
      show_line("ERROR: Cannot extend main memory in iniTeX\n", 1);
      mem_extra_high = 0;   mem_extra_low = 0;
    }
#endif
  }
  else
  {
    if (mem_initex != 0)
    {
      show_line("ERROR: Can only set initial main memory size in iniTeX\n", 1);
      mem_initex = 0;
    }

    if (trie_size != 0)
    {
      show_line("ERROR: Need only set hyphenation trie size in iniTeX\n", 1);
/* trie_size = 0; */
    }
  }
  if (mem_initex == 0)
    mem_initex = default_mem_top;
  if (trie_size == 0)
    trie_size = default_trie_size;
/* Just in case user mistakenly specified words instead of kilo words */
  if (mem_extra_high > 10000L * 1024L) mem_extra_high = mem_extra_high / 1024;
  if (mem_extra_low > 10000L * 1024L) mem_extra_low = mem_extra_low / 1024;
  if (mem_initex > 10000L * 1024L) mem_initex = mem_initex / 1024;
#ifdef ALLOCATEHIGH         /* not used anymore */
  if (mem_extra_high > 2048L * 1024L) { /* extend SW area by 16 mega byte! */
    show_line("WARNING: There may be no benefit to asking for so much memory\n", 0);
    mem_extra_high = 2048 * 1024; /* limit to SW to 4 x VLR */
  }
#endif
#ifdef ALLOCATELOW          /* not used anymore */
  if (mem_extra_low > 2048L * 1024L) { /* extend VL area by 16 mega byte! */
    show_line("WARNING: There may be no benefit to asking for so much memory\n", 0);
    mem_extra_low = 2048 * 1024; /* limit VLR to 4 x SW */
  }
#endif
  if (mem_initex > 2048L * 1024L) { /* extend main memory by 16 mega byte! */
    show_line("WARNING: There may be no benefit to asking for so much memory\n", 0);
/* mem_initex = 2048 * 1024; */
  }
 #ifdef ALLOCATEDVIBUF
  if (dvi_buf_size == 0) dvi_buf_size = default_dvi_buf_size;
/* if less than 1024 assume user specified kilo-bytes, not bytes */
  if (dvi_buf_size < 1024) dvi_buf_size = dvi_buf_size * 1024;
  if (dvi_buf_size % 8 != 0)        /* check multiple of eight */
    dvi_buf_size = (dvi_buf_size / 8 + 1) * 8;
 #endif
  if (new_hyphen_prime < 0) new_hyphen_prime = 0;
  if (new_hyphen_prime > 0) {
    if (! is_initex)
      show_line("ERROR: Can only set hyphen prime in iniTeX\n", 1);
    else {
      if (new_hyphen_prime % 2 == 0) new_hyphen_prime++;
      while (!prime(new_hyphen_prime)) new_hyphen_prime = new_hyphen_prime+2;
      if (trace_flag) {
        sprintf(log_line, "Using %d as hyphen prime\n", new_hyphen_prime);
        show_line(log_line, 0);
      }
    }
  }
  if (percent_grow > 100) percent_grow = percent_grow - 100;
  if (percent_grow > 100) percent_grow = 100;   /* upper limit - double */
  if (percent_grow < 10) percent_grow = 10;   /* lower limit - 10% */
}

/**********************************************************************/

void perrormod (char *s)
{
  sprintf(log_line, "`%s': %s\n", s, strerror(errno));
  show_line(log_line, 1);
}

void pause (void)
{
#ifndef _WINDOWS
  fflush(stdout);     /* ??? */
  fflush(stderr);     /* ??? */
  (void) _getch();    /* ??? */
#endif
}

void checkpause (int flag)
{            /* 95/Oct/28 */
  char *s;
  int debug_pause = 0;
/*  don't stop if in Q (quiet) or R (run) mode */
/*  stop only in S (scroll) and T (TeX) mode */
  if (interaction >= 0 && interaction < 2)
    flag = 0;    /* 98/Jun/30 */
  s = grabenv("DEBUGPAUSE");
  if (s != NULL) sscanf(s, "%d", &debug_pause);
  if (flag < 0) return;
  if (debug_pause) {
    if (debug_pause || flag > 0) {
      show_line("\n", 0);
#ifndef _WINDOWS
      show_line("Press any key to continue . . .\n", 0);
      pause();
#endif
    }
  }
}


/*************************************************************************/

/* convert tilde to pseudo_tilde to hide it from TeX --- 95/Sep/26 */
/* convert space to pseudo_space to hide it from TeX --- 97/Jun/5 */
/* called only if pseudo_tilde != 0 or pseudo_space != 0 */
/* this is then undone in tex3.c both for fopen input and output */
/* not ideal, since pseudo name appears in log and in error messages ... */

void hidetwiddle (char *name)
{
  char *s = name;

#ifdef DEBUGTWIDDLE
  if (trace_flag)
  {
    sprintf(log_line, "Hidetwiddle %s", name);
    show_line(log_line, 0);
  }
#endif
/*  while (*s != '\0' && *s != ' ') { */
  while (*s != '\0')
  {
    if (*s == '~' && pseudo_tilde != 0)
      *s = (char) pseudo_tilde;  /* typically 254 */
    else if (*s == ' ' && pseudo_space != 0)
      *s = (char) pseudo_space;  /* typically 255 */
    s++;
  }
#ifdef DEBUGTWIDDLE
  if (trace_flag)
  {
    sprintf(log_line, "=> %s\n", name);
    show_line(log_line, 0);
  }
#endif
}

void deslash_all (int ac, char **av)
{
  char buffer[PATH_MAX];  
  char *s;

/* environment variables for output directories (as in PC TeX) */

  if ((s = grabenv("TEXDVI")) != NULL) dvi_directory = s;
  if ((s = grabenv("TEXLOG")) != NULL) log_directory = s;
  if ((s = grabenv("TEXAUX")) != NULL) aux_directory = s;
  if ((s = grabenv("TEXFMT")) != NULL) fmt_directory = s;
  if ((s = grabenv("TEXPDF")) != NULL) pdf_directory = s;

  strcpy(buffer, av[0]);            /* get path to executable */

  if ((s = strrchr(buffer, '\\')) != NULL) *(s+1) = '\0';
  else if ((s = strrchr(buffer, '/')) != NULL) *(s+1) = '\0';
  else if ((s = strrchr(buffer, ':')) != NULL) *(s+1) = '\0';
  s = buffer + strlen(buffer) - 1;
  if (*s == '\\' || *s == '/') *s = '\0';   /* flush trailing PATH_SEP */
  texpath = xstrdup(buffer);

/*  Hmm, we may be operating on DOS environment variables here !!! */

  if (strcmp(dvi_directory, "") != 0) flush_trailing_slash (dvi_directory);
  if (strcmp(log_directory, "") != 0) flush_trailing_slash (log_directory);
  if (strcmp(aux_directory, "") != 0) flush_trailing_slash (aux_directory);
  if (strcmp(fmt_directory, "") != 0) flush_trailing_slash (fmt_directory);
  if (strcmp(pdf_directory, "") != 0) flush_trailing_slash (pdf_directory);

  if (deslash)
  {
      unixify (texpath);          /* 94/Jan/25 */
/* if output directories given, deslashify them also 1993/Dec/12 */
      if (strcmp(dvi_directory, "") != 0) unixify(dvi_directory);
      if (strcmp(log_directory, "") != 0) unixify(log_directory);
      if (strcmp(aux_directory, "") != 0) unixify(aux_directory);
      if (strcmp(fmt_directory, "") != 0) unixify(fmt_directory);
      if (strcmp(pdf_directory, "") != 0) unixify(pdf_directory);
  }

/*  deslash TeX source file (and format, if format specified) */
/*  and check args to see whether format was specified */
  format_spec = 0;
/*  NOTE: assuming that command line arguments are in writable memory ! */
/*  if (trace_flag || debug_flag)
    sprintf(log_line, "optind %d ac %d\n", optind, ac); */   /* debugging */ 
/*  if (optind < ac) { */           /* bkph */
  if (optind < ac && optind > 0) {      /* paranoia 94/Apr/10 */
    if (deslash) {
      if (trace_flag || debug_flag) {
        sprintf(log_line, "deslash: k %d argv[k] %s (argc %d)\n",
          optind, av[optind], ac);
        show_line(log_line, 0);
      }
      unixify(av[optind]);
    }
    if (pseudo_tilde != 0 || pseudo_space != 0)
      hidetwiddle (av[optind]);     /* 95/Sep/25 */
/* For Windows NT, lets allow + instead of & for format specification */
    if (*av[optind] == '&' || *av[optind] == '+') {
      format_spec = 1; /* format file specified */
      format_name = xstrdup(av[optind]+1); /* 94/Oct/25 */
/*      uppercase (format_name); */    /* why ? 98/Jan/31 */
      if (optind + 1 < ac) {
        if (deslash) {
          if (trace_flag || debug_flag) {
            sprintf(log_line, "deslash: k %d argv[k] %s (argc %d)\n",
              optind+1, av[optind+1], ac);
            show_line(log_line, 0);
          }
          unixify(av[optind+1]);
        }
        if (pseudo_tilde != 0 || pseudo_space != 0)
          hidetwiddle (av[optind+1]); /* 95/Sep/25 */
      }
    }         
  }
}

/* The above seems to assume that arguments that don't start with '-' */
/* are file names or format names - what if type in control sequences? */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* main entry point follows */

/* this gets called pretty much right away in `main' in texmf.c */

/* note: those optarg == 0 test don't really work ... */
/* note: optarg starts at = in case of x=... */

int main_init (int ac, char **av)
{
  char initbuffer[PATH_MAX];
  int k;

  kpse_set_program_name(av[0], NULL);

  if (sizeof(memory_word) != 8) /* compile time test */
  {
    sprintf(log_line, "ERROR: Bad word size %d!\n", sizeof(memory_word));
    show_line(log_line, 1);
  }

  start_time = clock();    /* get time */
  main_time = start_time;   /* fill in, in case file never opened */

  initbuffer[0] = '\0';         /* paranoia 94/Apr/10 */

/*  reset all allocatable memory pointers to NULL - in case we drop out */
  mainmemory = NULL;
  font_info = NULL;
  str_pool = NULL;
  str_start = NULL;

#ifdef ALLOCATESAVESTACK
  save_stack = NULL; 
#endif

#ifdef ALLOCATEBUFFER
  buffer = NULL;        /* new 1999/Jan/7 need to do early */
  current_buf_size = 0;
  buffer = realloc_buffer (initial_buf_size);
#endif

  hyph_list = NULL;
  hyph_word = NULL;
  trie_taken = NULL;
  trie_hash = NULL;
  trie_r = NULL;
  trie_c = NULL;
  trie_o = NULL;
  trie_l = NULL;
  trie_trc = NULL;
  trie_tro = NULL;
  trie_trl = NULL;

  log_opened = false;       /* so can tell whether opened */
  interaction = -1;         /* default state => 3 */
  missing_characters = 0;   /* none yet! */
  workingdirectory = false; /* set from dviwindo.ini & command line */
  font_dimen_zero = true;   /* \fontdimen0 for checksum 98/Oct/5 */
  ignore_frozen = false;    /* default is not to ignore 98/Oct/5 */
  suppress_f_ligs = false;  /* default is not to ignore f-ligs */

  if (ac > 1 && !strncmp(av[1], "-Y", 2))
    reorder_arg_flag = false;

  if (reorder_arg_flag)
    reorderargs(ac, av);  

  if (init_commands(ac, av))
    return -1;          // failure

  check_fixed_align(trace_flag);       /* sanity check 1994/Jan/8 */

  format_file   = NULL;       /* to be set in openinou.c 94/Jun/21 */
  string_file   = NULL;       /* to be set in openinou.c 96/Jan/15 */
  source_direct = NULL;       /* to be set in openinou.c 98/Sep/29 */
  dvi_file_name = NULL;       /* to be set in openinou.c 00/Jun/18 */
  log_file_name = NULL;       /* to be set in openinou.c 00/Jun/18 */

  first_pass_count  = 0;
  second_pass_count = 0;
  final_pass_count  = 0;
  paragraph_failed  = 0;
  single_line       = 0;
  overfull_hbox     = 0;
  underfull_hbox    = 0;
  overfull_vbox     = 0;
  underfull_vbox    = 0;

  closed_already=0;        // so can only do once

  if (trace_flag)
    show_line("Entering init (local)\n", 0);

/*   Print version *after* banner ? */ /* does this get in log file ? */

  probe_memory();             /* show top address */
  ini_max_address = max_address;       /* initial max address */

  if (trace_flag)
    show_maximums(stdout);

  initial_memory();

  deslash_all(ac, av);    /* deslash and note if format specified */

  no_interrupts = 0;

  if (format_spec && mem_spec_flag)
  {
    show_line("WARNING: Cannot change initial main memory size when format specified", 1);
  }

  if (allocate_memory() != 0)   /* NOW, try and ALLOCATE MEMORY if needed */
    return -1;         // if failed to allocate

/*   following is more or less useless since most all things not yet alloc */
   check_alloc_align(trace_flag);    /* sanity check 1994/Jan/8 */

  if (trace_flag)
    show_line("Leaving init (local)\n", 0);

  return 0;         // success
}

/* #define CLOCKS_PER_SEC 1000 */
#define CLK_TCK  CLOCKS_PER_SEC

/* void show_inter_val (clock_t start, clock_t end) { */
void show_inter_val (clock_t interval)
{
  int seconds, tenths, hundredth, thousands;

  if (interval >= CLK_TCK * 10)
  {
    tenths = (interval * 10 + CLK_TCK / 2) / CLK_TCK; 
    seconds = tenths / 10; 
    tenths = tenths % 10;
    sprintf(log_line, "%d.%d", seconds, tenths);
    show_line(log_line, 0);
  }
  else
    if (interval >= CLK_TCK)     /* 94/Feb/25 */
    {
      hundredth = (interval * 100 + CLK_TCK / 2) / CLK_TCK;
      seconds = hundredth / 100;
      hundredth = hundredth % 100;
      sprintf(log_line, "%d.%02d", seconds, hundredth);
      show_line(log_line, 0);
    }
    else
      if (interval > 0)         /* 94/Oct/4 */
      {
        thousands = (interval * 1000 + CLK_TCK / 2) / CLK_TCK;
        seconds = thousands / 1000;
        thousands = thousands % 1000;
        sprintf(log_line, "%d.%03d", seconds, thousands);
        show_line(log_line, 0);
      }
      else
        show_line("0", 0);          /* 95/Mar/1 */
}

/* final cleanup opportunity */ /* flag is non-zero if error exit */
/* shows various times, warning about missing chars */

int endit (int flag)
{
  finish_time = clock();

  if (missing_characters != 0)
    flag = 1;

  if (missing_characters)
  {
    sprintf(log_line, "! There %s %d missing character%s --- see log file\n",
      (missing_characters == 1) ? "was" : "were",  missing_characters,
      (missing_characters == 1) ? "" : "s");
    show_line(log_line, 0);
  }

  if (free_memory() != 0)
    flag++;

  if (verbose_flag)
  {
    show_line("Total ", 0);
    show_inter_val(finish_time - start_time);
    show_line(" sec (", 0);
    show_inter_val(main_time - start_time);
    show_line(" format load + ", 0);
    show_inter_val(finish_time - main_time);
    show_line(" processing) ", 0);

    if (total_pages > 0)
    {
      show_inter_val ((finish_time - main_time) / total_pages);
      show_line(" sec per page", 0);
    }

    show_line("\n", 0);
  }

  checkpause(flag);

  return flag;
}

/********************************************************************************/

/* addition 98/Mar/31 print_csnames Frank Mittelbach */

#define MAXCOLUMN 78

void print_cs_name (FILE *output, int h)
{
  int c, textof, n;

  memset(log_line, 0, sizeof(log_line));

  textof = hash[h].v.RH;

  if (textof == 0)
    return;

  c = sprintf(log_line, "(%d), ", h);
  n = length(textof);

  memmove(log_line + c, str_pool + str_start[textof], n);
  memmove(log_line + c + n, "\n", 2);

  if (output == stderr)
  {
    show_line(log_line, 1);
  }
  else
  {
    if (output == stdout)
      show_line(log_line, 0);
    else
      fprintf(output, log_line);
  }

}

int compare_strn (int, int, int, int); /* in tex9.c */

/* compare two csnames in qsort */

int compare_cs (const void *cp1, const void *cp2)
{
  int c1, c2, l1, l2, k1, k2, textof1, textof2;

  c1 = *(int *)cp1;
  c2 = *(int *)cp2;
  textof1 = hash[c1].v.RH;
  textof2 = hash[c2].v.RH;
  l1 = length(textof1); 
  l2 = length(textof2); 
  k1 = str_start[textof1]; 
  k2 = str_start[textof2]; 

  return compare_strn (k1, l1, k2, l2);
}

char *csused = NULL;

/* Allocate table of indeces to allow sorting on csname */
/* Allocate flags to remember which ones already listed at start */
/* pass = 0 --> fmt */
/* pass = 1 --> after */
void print_cs_names (FILE *output, int pass)
{
  int h, k, ccount, repeatflag;
  int *cnumtable;
  int nfcs = frozen_control_sequence;

  if (pass == 0 && csused == NULL)
  {
    csused = (char *) malloc (nfcs);

    if (csused == NULL)
      return;

#ifdef USEMEMSET
    memset(csused, 0, nfcs);
#else
    for (h = 0; h < (hash_size + 780); h++)
      csused[h] = 0;
#endif
  }

  ccount = 0;

  for (h = hash_base + 1; h < nfcs; h++)
  {
    if (pass == 1 && csused[h])
      continue;

    if (hash[h].v.RH != 0)
    {
      if (pass == 0)
        csused[h] = 1;

      ccount++;
    }
  }

  sprintf(log_line, "\n%d %s multiletter control sequences:\n",
      ccount, (pass == 1) ? "new" : "");

  if (output == stderr)
  {
    show_line(log_line, 1);
  }
  else
  {
    if (output == stdout)
      show_line(log_line, 0);
    else
      fprintf(output, log_line);
  }

  if (ccount > 0) /* don't bother to get into trouble */
  {
    cnumtable = (int *) malloc (ccount * sizeof(int));

    if (cnumtable == NULL)
      return;

    ccount = 0;
/*    for (h = 515; h < (hash_size + 780); h++) { */
    for (h = hash_base + 1; h < nfcs; h++)
    {
      if (pass == 1 && csused[h])
        continue;

      if (hash[h].v.RH != 0)
        cnumtable[ccount++] = h;
    }

    //qsort ((void *)cnumtable, ccount, sizeof (int), &compare_cs);

    repeatflag = 0;

    for (k = 0; k < ccount; k++)
    {
      h = cnumtable[k];

      if (pass == 1 && csused[h])
        continue;

      print_cs_name(output, h);
    }

    sprintf(log_line, "\n");

    if (output == stderr)
    {
      show_line(log_line, 1);
    }
    else
    {
      if (output == stdout)
        show_line(log_line, 0);
      else
        fprintf(output, log_line);
    }

    free((void *)cnumtable);
  }

  if (pass == 1 && csused != NULL)
  {
    free(csused);
    csused = NULL;
  }
}

/***************** font info listing moved from TEX9.C ******************/
/* compare two strings in str_pool (not null terminated) */
/* k1 and k2 are positions in string pool */
/* l1 and l2 are lengths of strings */

int compare_strn (int k1, int l1, int k2, int l2)
{
  int c1, c2;

  while (l1 > 0 && l2 > 0)
  {
    c1 = str_pool[k1];
    c2 = str_pool[k2];

    if (c1 > c2)
      return 1;
    else if (c2 > c1)
      return -1;

    l1--; l2--;
    k1++; k2++;
  }

  if (l1 > 0)
    return 1;   /* first string longer */
  else if (l2 > 0)
    return -1; /* second string longer */

  return 0;         /* strings match */
}

/* compare two font names and their at sizes in qsort */

int compare_fnt (const void *fp1, const void *fp2)
{
  int f1, f2, l1, l2, k1, k2, s;

  f1 = *(short *)fp1;
  f2 = *(short *)fp2;
  l1 = length(font_name[f1]);
  l2 = length(font_name[f2]);
  k1 = str_start[font_name[f1]]; 
  k2 = str_start[font_name[f2]]; 

  s = compare_strn (k1, l1, k2, l2);

  if (s != 0)
    return s;

  if (font_size[f1] > font_size[f2])
    return 1;
  else if (font_size[f1] < font_size[f2])
    return -1;

  return 0;         /* should not ever get here */
}

/* compare two font names */

int compare_fnt_name (int f1, int f2)
{
  int l1, l2, k1, k2, s;

  l1 = length(font_name[f1]);
  l2 = length(font_name[f2]); 
  k1 = str_start[font_name[f1]]; 
  k2 = str_start[font_name[f2]]; 

  s = compare_strn (k1, l1, k2, l2);

  return s;
}

/* decode checksum information */

unsigned long checkdefault = 0x59265920;  /* default signature */

int decode_fourty (unsigned long checksum, char *codingvector)
{
  int c;
  int k;
/*  char codingvector[6+1]; */

  if (checksum == 0)
  {
    strcpy(codingvector, "unknwn");
    return 1;
  }
  else
    if ((checksum >> 8) == (checkdefault >> 8))  /* last byte random */
    {
/*    strcpy (codingvector,  "native"); */  /* if not specified ... */
      strcpy (codingvector,  "fixed ");   /* if not specified ... */
      return 1;               /* no info available */
    }
    else
    {
      for (k = 0; k < 6; k++)
      {
        c = (int) (checksum % 40);
        checksum = checksum / 40;
        if (c <= 'z' - 'a') c = c + 'a';
        else if (c < 36) c = (c + '0') - ('z' - 'a') - 1;
        else if (c == 36) c = '-';
        else if (c == 37) c = '&';
        else if (c == 38) c = '_';
        else c = '.';       /* unknown */
        codingvector[5-k] = (char) c;
      }
      codingvector[6] = '\0';
    }
/*  sprintf(log_line, "Reconstructed vector %s\n", codingvector); */
  return 0;         /* encoding info returned in codingvector */
}

double sclpnt (long x)
{
  double pt;

  pt = (double) x / 65536.0;
  pt = (double) ((int) (pt * 1000.0 + 0.5)) / 1000.0;

  return (pt);
}

// Shows list of fonts in log file

void dvi_font_show(internal_font_number f, int suppressname)
{
  int a, l, k, n;
  unsigned long checksum;
  char checksumvector[8];
  char buffer[32];

  putc(' ', log_file);

  if (suppressname == 0)
  {
    a = length(font_area[f]);
    l = length(font_name[f]);

    k = str_start[font_area[f]];

    memcpy(buffer, str_pool + k, length(font_area[f]));
    fwrite(buffer, sizeof(char), length(font_area[f]), log_file);

    k = str_start[font_name[f]];

    memcpy(buffer, str_pool + k, length(font_name[f]));
    fwrite(buffer, sizeof(char), length(font_name[f]), log_file);
  }
  else a = l = 0;

  for (k = a + l; k < 16; k++)
    putc(' ', log_file);

  sprintf(buffer, "at %lgpt ", sclpnt(font_size[f]));
  fputs(buffer, log_file);

  if (suppressname == 0)
  {
    n = strlen(buffer);

    for (k = n; k < 16; k++)
      putc(' ', log_file);

    checksum = (((font_check[f].b0) << 8 | font_check[f].b1) << 8 | font_check[f].b2) << 8 | font_check[f].b3;
    decode_fourty(checksum, checksumvector);
    fprintf(log_file, "encoding: %s..", checksumvector);
  }

  putc('\n', log_file);
}

/* Allocate table of indeces to allow sorting on font name */

void show_font_info (void)
{
  int k, m, fcount, repeatflag;
  short *fnumtable;

  fcount = 0;

  for (k = 1; k <= font_ptr; k++)
    if (font_used[k])
      fcount++;

  if (fcount == 0)
    return;

  fnumtable = (short *) malloc (fcount * sizeof(short));

  fprintf(log_file, "\nUsed %d font%s:\n", fcount, (fcount == 1) ? "" : "s");

  fcount = 0;

  for (k = 1; k <= font_ptr; k++) 
    if (font_used[k])
      fnumtable[fcount++] = (short) k;

  qsort ((void *)fnumtable, fcount, sizeof (short), &compare_fnt);

  repeatflag = 0;

  for (m = 0; m < fcount; m++)
  {
    if (m > 0)
    {
      if (compare_fnt_name(fnumtable[m-1], fnumtable[m]) == 0)
        repeatflag = 1;
      else
        repeatflag = 0;
    }

    dvi_font_show(fnumtable[m], repeatflag);
  }

  free((void *)fnumtable);
}

/*  NOTE: current_tfm = false (-c)
  not checking for TFM in current directory saves 0.1 sec
  (0.2 if file_method = false (-G) */

/*  NOTE: test_dir_access = false (-b):
  not checking whether readable file is a directory saves maybe 0.5 sec
  BUT only if file_method = false (-G) - otherwise its irrelevant */

/*  NOTE: dir_method = false (-D) --- method for checking whether directory
  using fopen instead of _findfirst in dir_p slows job maybe 0.05 sec
  BUT not if current_tfm = false (-c) */

/*  NOTE: file_method = false (-G) --- method for checking file accessible
  using _access (readable) instead of _findfirst (file_p) costs 0.5 sec */

/*  Fast flag combinations: nothing, bG, bcG, bcDG */

/* constants for _heapchk/_heapset/_heapwalk routines */
/* #define _HEAPEMPTY (-1) */
/* #define _HEAPOK  (-2) */
/* #define _HEAPBADBEGIN  (-3) */
/* #define _HEAPBADNODE (-4) */
/* #define _HEAPEND (-5) */
/* #define _HEAPBADPTR  (-6) */

/* new arg -H=8 for horizontal tab *//* tab_step = 0 means no tab replacement */

/* buffercopy no longer used */

/* To Knuthian reset right when command line interpreted */
