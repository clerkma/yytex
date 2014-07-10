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

enum
{
  out_dvi_flag = (1 << 0),
  out_pdf_flag = (1 << 1),
  out_xdv_flag = (1 << 2),
  out_dpx_flag = (1 << 3),
};

/* Read a line of input as quickly as possible.  */
extern boolean input_line (FILE * f);
#define input_ln(stream, flag) input_line(stream)
/* sec 0027 */
#define a_open_in(f, p) open_input  (&(f), p, FOPEN_R_MODE)
#define a_open_out(f)   open_output (&(f), FOPEN_W_MODE)
#define b_open_in(f)    open_input  (&(f), TFMFILEPATH, FOPEN_RBIN_MODE)
#define b_open_out(f)   open_output (&(f), FOPEN_WBIN_MODE)
#define w_open_in(f)    open_input  (&(f), TEXFORMATPATH, FOPEN_RBIN_MODE)
#define w_open_out      b_open_out
#define a_close(f)	    (void) check_fclose(f)
#define b_close         a_close
#define w_close         a_close
#define gz_w_close      gzclose

/* If we're running under Unix, use system calls instead of standard I/O
   to read and write the output files; also, be able to make a core dump. */ 
#ifndef unix
  #define dumpcore() exit(1)
#else /* unix */
  #define dumpcore abort
#endif

#ifdef COMPACTFORMAT
extern int do_dump   (char * p, int item_size, int nitems, gzFile out_file);
extern int do_undump (char * p, int item_size, int nitems, gzFile out_file);
#define dump_file gz_fmt_file
#else
extern int do_dump   (char * p, int item_size, int nitems, FILE * out_file);
extern int do_undump (char * p, int item_size, int nitems, FILE * out_file);
#define dump_file fmt_file
#endif

#define dumpthings(base, len)           \
  do_dump   ((char *) &(base), sizeof (base), (int) (len), dump_file)

#define undumpthings(base, len)         \
  do_undump ((char *) &(base), sizeof (base), (int) (len), dump_file)

/* Use the above for all the other dumping and undumping. */
#define generic_dump(x)   dumpthings(x, 1)
#define generic_undump(x) undumpthings(x, 1)

#define dump_wd     generic_dump
#define undump_wd   generic_undump
#define dump_hh     generic_dump
#define undump_hh   generic_undump
#define dump_qqqq   generic_dump
#define undump_qqqq generic_undump

#define dump_int(x)         \
  do                        \
    {                       \
      integer x_val = (x);  \
      generic_dump (x_val); \
    }                       \
  while (0)

#define undump_int  generic_undump

extern void t_open_in();