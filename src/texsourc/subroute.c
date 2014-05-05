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

#pragma warning(disable:4996)
#pragma warning(disable:4131) // old style declarator
#pragma warning(disable:4135) // conversion between different integral types
#pragma warning(disable:4127) // conditional expression is constant

#define EXTERN extern

#include "texd.h"

#define PATH_SEP '/'

// used only in jump_out in tex0.c, and in texbody in itex.c
// and main in texmf.c and a few other abort situations in texmf.c
/* texk/web2c/lib/uexit.c */
void uexit (int unix_code)
{
  int final_code;

#ifndef _WINDOWS
  fflush(stdout);
#endif

  if (unix_code == 0)
    final_code = EXIT_SUCCESS;
  else if (unix_code == 1)
    final_code = EXIT_FAILURE;
  else
    final_code = unix_code;

  if (jump_used)
  {
    show_line("Jump Buffer already used\n", 1);
    exit(1);
  }

  jump_used++;
  exit(final_code);
}
/* texk/web2c/lib/zround.c */
integer zround (double r)
{
  integer i;

  if (r > 2147483647.0)
    i = 2147483647;
  else if (r < -2147483647.0)
    i = -2147483647;
  else if (r >= 0.0)
    i = (integer) (r + 0.5);
  else
    i = (integer) (r - 0.5);

  return i;
}
/* texk/web2c/lib/eofeoln.c */
bool eoln (FILE * file)
{
  register int c;

  if (feof (file))
    return true;

  c = getc (file);

  if (c != EOF)
    (void) ungetc (c, file);

  return c == '\n' || c == '\r' || c == EOF;
}

char * read_a_line (FILE *f,  char *line, int limit)
{
  int c;
  int loc = 0;

  while ((c = getc (f)) != EOF)
  {
    if (c == '\n' || c == '\r')
    {
      if (loc > 0) break;
      else continue;        /* ignore \r\n and blank lines */
    }

    line[loc] = (char) c;
    loc++;

    if (loc == limit - 1)
    {
      sprintf(log_line, " ERROR: line too long\n");
      show_line(log_line, 1);
      show_line(line, 0);
      show_line("\n", 0);
      break;
    }
  }

  if (c != EOF || loc > 0)
  {
    line[loc] = '\0';       /* terminate */
    return line;          /* and return */
  }
  else
    return(NULL);          /* true EOF */
}

/* Unixify filename and path (turn \ into /) --- assumes null terminated */
/* NEED HACK! */
char *unixify (char * t)
{
  char * s = t;

  if (s == NULL)
    return s;    /* paranoia -- 1993/Apr/10 */

#ifdef MSDOS
  if (t != '\0')
  {
    while (*s != '\0') {        /* paranoia -- 1997/Oct/23 */
/*      if (*s == '\\') *s = '/'; */
      if (*s == '\\')
        *s = PATH_SEP;

      s++;
    }       /* endwhile */
  }
// #ifdef MYDEBUG
  if (trace_flag)
  {
    sprintf(log_line, "Unixified name: %s\n", t);
    show_line(log_line, 0);
  }
// #endif
#endif /* DOS */
  return t;
}