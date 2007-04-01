/* config.h: Master configuration file.  This is included by common.h,
   which everyone includes.

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

#ifndef CONFIG_H
#define CONFIG_H

/* System dependencies that are figured out by `configure'.  */
#include "c-auto.h"

/* ``Standard'' headers.  */
#include "c-std.h"

/* Everybody uses PATH_MAX.  */
#include "c-pathmx.h"

/* How to open files with fopen.  */
#include "c-fopen.h"

/* Macros to discard or keep prototypes.  */
#include "c-proto.h"


/* The smallest signed type: use `signed char' if ANSI C, `short' if
   char is unsigned, otherwise `char'.  */
#ifndef SCHAR_TYPE
#ifdef __STDC__
#define SCHAR_TYPE signed char
#else /* not __STDC */
#ifdef __CHAR_UNSIGNED__
#define SCHAR_TYPE short
#else
#define SCHAR_TYPE char
#endif
#endif /* not __STDC__ */
#endif /* not SCHAR_TYPE */
typedef SCHAR_TYPE schar;

/* The type `integer' must be a signed integer capable of holding at
   least the range of numbers (-2^31)..(2^31-1).  If your compiler goes
   to great lengths to make programs fail, you might have to change this
   definition.  If this changes, you will probably have to modify
   web2c/fixwrites.c, since it generates code to do integer output using
   "%ld", and casts all integral values to be printed to `long'.  */
#ifndef INTEGER_TYPE
#define INTEGER_TYPE long
#endif
typedef INTEGER_TYPE integer;

/* `volatile' is only used in Metafont to avoid bugs in the MIPS C
   compiler.  If this definition goes wrong somehow, just get rid of it
   and the two corresponding substitutions in mf/convert.  */
/* #ifndef __STDC__ */
/* #define volatile */
/* #endif */	/* flushed 1994/March/20 to protect `int interrupt' */

/* System-dependent hacks.  */

/* Hack to get around High C on an IBM RT treating `char' differently
   than normal compilers, etc.   */
#if defined (__HIGHC__) && defined (ibm032)
pragma	Off(Char_default_unsigned);
pragma	On(Char_is_rep);
pragma	On(Parm_warnings);
pragma	On(Pointers_compatible);
pragma	On(Pointers_compatible_with_ints);
#endif	/* __HIGHC__ and ibm032 */


/* Some definitions of our own.  */
#include "common.h"

#endif /* not CONFIG_H */

#ifdef MSDOS
extern booleane traceflag;	/* redefinition ? */
extern booleane opentraceflag;	/* redefinition ? */
#endif
