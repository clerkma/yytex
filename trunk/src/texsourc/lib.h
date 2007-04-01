/* lib.h: declarations for shared routines.

   Copyright 1992, 1993 Karl Berry
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

#ifndef LIB_H
#define LIB_H

#include "types.h"

/* Define common sorts of messages.  */

/* This should be called only after a system call fails.  */
// #define FATAL_PERROR(s) do { perror (s); exit (errno); } while (0)
#define FATAL_PERROR(s) do { perrormod (s); exit (errno); } while (0)

#define START_FATAL() do { fputs ("fatal: ", stderr)
#define END_FATAL() fputs (".\n", stderr); exit (1); } while (0)

#define FATAL(str)							\
  START_FATAL (); fprintf (stderr, "%s", str); END_FATAL ()
#define FATAL1(str, e1)							\
  START_FATAL (); fprintf (stderr, str, e1); END_FATAL ()
#define FATAL2(str, e1, e2)						\
  START_FATAL (); fprintf (stderr, str, e1, e2); END_FATAL ()
#define FATAL3(str, e1, e2, e3)						\
  START_FATAL (); fprintf (stderr, str, e1, e2, e3); END_FATAL ()
#define FATAL4(str, e1, e2, e3, e4)					\
  START_FATAL (); fprintf (stderr, str, e1, e2, e3, e4); END_FATAL ()


#define START_WARNING() do { fputs ("warning: ", stderr)
#define END_WARNING() fputs (".\n", stderr); fflush (stderr); } while (0)

#define WARNING(str)							\
  START_WARNING (); fprintf (stderr, "%s", str); END_WARNING ()
#define WARNING1(str, e1)						\
  START_WARNING (); fprintf (stderr, str, e1); END_WARNING ()
#define WARNING2(str, e1, e2)						\
  START_WARNING (); fprintf (stderr, str, e1, e2); END_WARNING ()
#define WARNING3(str, e1, e2, e3)					\
  START_WARNING (); fprintf (stderr, str, e1, e2, e3); END_WARNING ()
#define WARNING4(str, e1, e2, e3, e4)					\
  START_WARNING (); fprintf (stderr, str, e1, e2, e3, e4); END_WARNING ()

/* I find this easier to read.  */
#define STREQ(s1, s2) (strcmp (s1, s2) == 0)

/* This is the maximum number of numerals that result when a 64-bit
   integer is converted to a string, plus one for a trailing null byte,
   plus one for a sign.  */
#define MAX_INT_LENGTH 21

/* Return a fresh copy of S1 followed by S2, et al.  */
extern string concat(string s1, string s2);
extern string concat3 (string, string, string);
extern string concat4 (string, string, string, string);
extern string concat5 (string, string, string, string, string);


/* A fresh copy of just S.  */
extern string xstrdup (string s);


/* True if FILENAME1 and FILENAME2 are the same file.  If stat fails on
   either name, returns false, with no error message.  */
extern booleane same_file_p (string filename1, string filename2);


/* If NAME has a suffix, return a pointer to its first character (i.e.,
   the one after the `.'); otherwise, return NULL.  */
extern string find_suffix (string name);

/* Return NAME with any suffix removed.  */
extern string remove_suffix (string name);

/* Return S with the suffix SUFFIX, removing any suffix already present.
   For example, `make_suffix ("/foo/bar.baz", "karl")' returns
   `/foo/bar.karl'.  Returns a string allocated with malloc.  */
extern string make_suffix (string s, string suffix);

/* Return NAME with STEM_PREFIX prepended to the stem. For example,
   `make_prefix ("/foo/bar.baz", "x")' returns `/foo/xbar.baz'.
   Returns a string allocated with malloc.  */
extern string make_prefix (string stem_prefix, string name);

/* If NAME has a suffix, simply return it; otherwise, return
   `NAME.SUFFIX'.  */
extern string extend_filename (string name, string suffix);


/* Like their stdio counterparts, but abort on error, after calling
   perror(3) with FILENAME as its argument.  */
extern FILE *xfopen (string filename, string mode);
// extern void xfclose (FILE *, string filename);
extern int xfclose (FILE *, string filename);
extern void xfseek (FILE *, long, int, string filename);
extern long xftell (FILE *, string filename);


/* These call the corresponding function in the standard library, and
   abort if those routines fail.  Also, `xrealloc' calls `xmalloc' if
   OLD_ADDRESS is null.  */
extern address xmalloc (unsigned size);
extern address xrealloc (address old_address, unsigned new_size);
extern address xcalloc (unsigned nelem, unsigned elsize);

/* (Re)Allocate N items of type T using xmalloc/xrealloc.  */
#define XTALLOC(n, t) (t *) xmalloc ((n) * sizeof (t))
#define XTALLOC1(t) XTALLOC (1, t)
#define XRETALLOC(addr, n, t) ((addr) = (t *) xrealloc (addr, (n) * sizeof(t)))

#endif /* not LIB_H */
