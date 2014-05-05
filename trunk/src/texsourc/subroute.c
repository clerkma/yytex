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

#define EXTERN extern

#include "texd.h"

#define PATH_SEP '/'

// used only in jump_out in tex0.c, and in texbody in itex.c
// and main in texmf.c and a few other abort situations in texmf.c
// texk/web2c/lib/uexit.c
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
// texk/web2c/lib/zround.c
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
// texk/web2c/lib/eofeoln.c
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
// Unixify filename and path (turn \ into /)
// --- assumes null terminated
char * unixify (char * t)
{
  char * s = t;

  if (s == NULL)
    return s;

  if (t != '\0')
  {
    while (*s != '\0')
    {
      if (*s == '\\')
        *s = PATH_SEP;

      s++;
    }
  }

  if (trace_flag)
  {
    printf("Unixified name: %s\n", t);
    //sprintf(log_line, "Unixified name: %s\n", t);
    //show_line(log_line, 0);
  }

  return t;
}