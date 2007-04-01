/* Main include file for TeX in C.  Originally by Tim Morgan,
   December 23, 1987.  These routines are also used by Metafont (with
   some name changes).

   Copyright 1992 Karl Berry
   Copyright 2007 TeX Users Group

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

#include "config.h"

/* need io.h for write() - bkph */

#ifdef MSDOS
/* #include <io.h> */ /* conflict with read() in pascal.h ??? */
int __cdecl write(int, const void *, unsigned int);
#endif

#ifdef TeX
#define dump_file fmtfile
#define dump_path TEXFORMATPATH
#define write_out writedvi
#define out_file dvifile
#define out_buf dvibuf
#else /* not TeX */
#define dump_file basefile
#define dump_path MFBASEPATH
#define write_out writegf
#define out_file gffile
#define out_buf gfbuf
#endif /* not TeX */



/* File types.  */
typedef FILE *bytefile, *wordfile;



/* Read a line of input as quickly as possible.  */
#define	inputln(stream, flag)	input_line (stream)
extern booleane input_line (FILE *);
/* was extern booleane input_line (); */


/* We need to read an integer from stdin if we're debugging.  */
#ifdef DEBUG
#define getint()  inputint (stdin)
#else
#define getint()
#endif



/* `bopenin' (and out) is used only for reading (and writing) .tfm
   files; `wopenin' (and out) only for dump files.  The filenames are
   passed in as a global variable, `nameoffile'.  */
   
#define bopenin(f)	open_input (&(f), TFMFILEPATH, FOPEN_RBIN_MODE)
#define wopenin(f)	open_input (&(f), dump_path, FOPEN_RBIN_MODE)
#define bopenout(f)	open_output (&(f), FOPEN_WBIN_MODE)
#define wopenout	bopenout
#define bclose		aclose
#define wclose		aclose

/* This routine has to return four values.  */
#define	dateandtime(i, j, k, l)	get_date_and_time (&(i), &(j), &(k), &(l))



/* If we're running under Unix, use system calls instead of standard I/O
   to read and write the output files; also, be able to make a core dump. */ 
#ifndef unix
#define	dumpcore()	exit (1)
#else /* unix */
#define	dumpcore	abort
#endif

/* #ifndef unix */				/* change bkph 93/Nov/26 */
/* #if ! defined unix && ! defined MSDOS */
#ifndef unix					/* changed back bkph 93/Dec/28 fixed macro */

/* This was horseshit, since for MSDOS it did no errors checking! - bkph */
#ifdef TeX
/* #define	writedvi(a, b)							\
  (void) fwrite ((char *) &dvibuf[a], sizeof (dvibuf[a]),		\
                 (int) ((b) - (a) + 1), dvifile) */
#define	writedvi(a, b)							\
  if (fwrite ((char *) &dvibuf[a], sizeof (dvibuf[a]),		\
         (int) ((b) - (a) + 1), dvifile) != (size_t) ((b) - (a) + 1))   \
		 FATAL_PERROR ("\n! dvi file")
#else
#define	writegf(a, b)							\
/*  (void) fwrite ((char *) &gfbuf[a], sizeof (gfbuf[a]),			\
                 (int) ((b) - (a) + 1), gffile) */
  if (fwrite ((char *) &gfbuf[a], sizeof (gfbuf[a]),			\
         (int) ((b) - (a) + 1), gffile) != (size_t) ((end) - (start) + 1))     \
		 FATAL_PERROR ("\n! gf file")
#endif /* not TeX */

#else /* unix */

/* #error WE APPEAR TO BE USING SYSTEM CALLS WITH ERROR CHECKING! */
/* USE fwrite(...) instead of write(fileno (...)) ! */

#ifdef TeX
#define	writedvi(start, end)						\
  if (write (fileno (dvifile), (char *) &dvibuf[start],			\
             (int) ((end) - (start) + 1))				\
      != (int) ((end) - (start) + 1))					\
    FATAL_PERROR ("\n! dvi file")
#else
#define	writegf(start, end)						\
  if (write (fileno (gffile), (char *) &gfbuf[start],			\
             (int) ((end) - (start) + 1))				\
      != (int) ((end) - (start) + 1))					\
    FATAL_PERROR ("\n! gf file")
#endif /* not TeX */
#endif /* unix */


/* Reading and writing the dump files.  `(un)dumpthings' is called from
   the change file.*/
#define	dumpthings(base, len)						\
  do_dump ((char *) &(base), sizeof (base), (int) (len), dump_file)

#define	undumpthings(base, len)						\
  do_undump ((char *) &(base), sizeof (base), (int) (len), dump_file)

/* We define the routines to do the actual work in texmf.c.  */
// extern void do_dump (char *, int, int, FILE *);
extern int do_dump (char *, int, int, FILE *);
/* extern void do_dump (); */
// extern void do_undump (char *, int, int, FILE *);
extern int do_undump (char *, int, int, FILE *);
/* extern void do_undump (); */

/* Use the above for all the other dumping and undumping.  */
#define generic_dump(x) dumpthings (x, 1)
#define generic_undump(x) undumpthings (x, 1)

#define dumpwd		generic_dump
#define undumpwd	generic_undump
#define dumphh		generic_dump
#define undumphh	generic_undump
#define dumpqqqq   	generic_dump
#define	undumpqqqq	generic_undump

/* `dump_int' is called with constant integers, so we put them into a
   variable first.  */
#define	dumpint(x)							\
  do									\
    {									\
      integer x_val = (x);						\
      generic_dump (x_val);						\
    }									\
  while (0)

/* web2c/regfix puts variables in the format file loading into
   registers.  Some compilers aren't willing to take addresses of such
   variables.  So we must kludge.  */
#ifdef REGFIX
#define undumpint(x)							\
  do									\
    {									\
      integer x_val;							\
      generic_undump (x_val);						\
      x = x_val;							\
    }									\
  while (0)
#else
#define	undumpint	generic_undump
#endif

/* Metafont wants to write bytes to the TFM file.  The casts in these
   routines are important, since otherwise memory is clobbered in some
   strange way, which causes ``13 font metric dimensions to be
   decreased'' in the trap test, instead of 4.  */

#define bwritebyte(f, b)    putc ((char) (b), f)
#define bwrite2bytes(f, h)						\
  do									\
    {									\
      integer v = (integer) (h);					\
      putc (v >> 8, f);  putc (v & 0xff, f);				\
    }									\
  while (0)
#define bwrite4bytes(f, w)						\
  do									\
    {									\
      integer v = (integer) (w);					\
      putc (v >> 24, f); putc (v >> 16, f);				\
      putc (v >> 8, f);  putc (v & 0xff, f);				\
    }									\
  while (0)



/* If we're running on an ASCII system, there is no need to use the
   `xchr' array to convert characters to the external encoding.  */
/* #ifdef NONASCII */
/* #define	Xchr(x)		xchr[x] */
/* #else */
/* #define	Xchr(x)		((char) (x)) */
/* #endif */

/* Following used in tex0.c - make possible to use non ASCII */

#define	Xchr(x)		xchr[x]

/* following added from new texmf.c file 1996/Jan/12 */
/* these, of course are useless definitions since parameters not given */

/* Declare routines in texmf.c.  */
extern void get_date_and_time ();
extern void topenin ();
// extern void calledit ();
// extern int calledit ();
extern booleane extensionirrelevantp ();
extern booleane input_line ();
// extern void do_dump ();
// extern void do_undump ();

#ifdef TeX
extern booleane maketextex ();
extern booleane maketextfm ();
#else
extern void main_body ();
extern booleane initscreen ();
extern booleane maketexmf ();
extern void updatescreen ();
extern void blankrectangle ();
extern void paintrow ();
#endif /* not TeX */
