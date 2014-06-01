/* Main include file for TeX in C.  Originally by Tim Morgan,
   December 23, 1987.  These routines are also used by Metafont (with
   some name changes).

   Copyright 1992 Karl Berry
   Copyright 2007 TeX Users Group
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

#include "yandytex.h"

enum {
  out_dvi_flag = (1 << 0),
  out_pdf_flag = (1 << 1),
  out_xdv_flag = (1 << 2),
};

#define INLINE inline

#define dump_file  fmt_file
#define out_file   dvi_file

/* Read a line of input as quickly as possible.  */
extern boolean input_line (FILE *);
#define input_ln(stream, flag) input_line(stream)

/* `b_open_in' (and out) is used only for reading (and writing) .tfm
   files; `w_open_in' (and out) only for dump files.  The filenames are
   passed in as a global variable, `name_of_file'.  */
   
#define b_open_in(f)  open_input  (&(f), TFMFILEPATH, FOPEN_RBIN_MODE)
#define w_open_in(f)  open_input  (&(f), TEXFORMATPATH, FOPEN_RBIN_MODE)
#define b_open_out(f) open_output (&(f), FOPEN_WBIN_MODE)
#define w_open_out    b_open_out
#define b_close       a_close
#define w_close       a_close
#define gz_w_close    gzclose

/* sec 0241 */
#define fix_date_and_time() get_date_and_time (&(tex_time), &(day), &(month), &(year))

/* If we're running under Unix, use system calls instead of standard I/O
   to read and write the output files; also, be able to make a core dump. */ 
#ifndef unix
  #define dumpcore() exit(1)
#else /* unix */
  #define dumpcore abort
#endif

#define write_dvi(a, b)                                           \
  if ((size_t) fwrite ((char *) &dvi_buf[a], sizeof (dvi_buf[a]), \
         (size_t) ((size_t)(b) - (size_t)(a) + 1), dvi_file)      \
         != (size_t) ((size_t)(b) - (size_t)(a) + 1))             \
     FATAL_PERROR ("\n! dvi file")

extern int do_dump (char *, int, int, FILE *);
extern int do_undump (char *, int, int, FILE *);

/* Reading and writing the dump files.  `(un)dumpthings' is called from
   the change file.*/
#define dumpthings(base, len)           \
  do_dump ((char *) &(base), sizeof (base), (int) (len), dump_file)

#define undumpthings(base, len)           \
  do_undump ((char *) &(base), sizeof (base), (int) (len), dump_file)

/* Use the above for all the other dumping and undumping.  */
#define generic_dump(x)   dumpthings (x, 1)
#define generic_undump(x) undumpthings (x, 1)

#define dump_wd     generic_dump
#define undump_wd   generic_undump
#define dump_hh     generic_dump
#define undump_hh   generic_undump
#define dump_qqqq   generic_dump
#define undump_qqqq generic_undump

/* `dump_int' is called with constant integers, so we put them into a
   variable first.  */
#define dump_int(x)         \
  do                        \
    {                       \
      integer x_val = (x);  \
      generic_dump (x_val); \
    }                       \
  while (0)

/* web2c/regfix puts variables in the format file loading into
   registers.  Some compilers aren't willing to take addresses of such
   variables.  So we must kludge.  */
#ifdef REGFIX
#define undump_int(x)         \
  do                          \
    {                         \
      integer x_val;          \
      generic_undump (x_val); \
      x = x_val;              \
    }                         \
  while (0)
#else
#define undump_int  generic_undump
#endif


/* If we're running on an ASCII system, there is no need to use the
   `xchr' array to convert characters to the external encoding.  */

#define Xchr(x) xchr[x]

/* following added from new texmf.c file 1996/Jan/12 */
/* these, of course are useless definitions since parameters not given */

/* Declare routines in texmf.c.  */
extern void t_open_in();