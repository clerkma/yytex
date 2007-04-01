/* c-std.h: the first header files.

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

#ifndef C_STD_H
#define C_STD_H

/* Header files that essentially all of our sources need, and
   that all implementations have.  We include these first, to help with
   NULL being defined multiple times.  */
#include <math.h>
#include <stdio.h>

/* POSIX.1 says that <unistd.h> may require <sys/types.h>.  */
#include <sys/types.h>

/* This is the symbol that X uses to determine if <sys/types.h> has been
   read, so we define it.  */
#define __TYPES__

/* X uses this symbol to say whether we have <stddef.h> etc.  */
#ifndef STDC_HEADERS
#define X_NOT_STDC_ENV
#endif

/* Be sure we have constants from <unistd.h>.  */
#include "c-unistd.h"

/* Include <stdlib.h> first to help avoid NULL redefinitions.  */
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#else /* end of STDC_HEADERS */
extern char *getenv (const char *); /* extern char *getenv (); */

#ifndef ALLOC_RETURN_TYPE
#ifdef MSDOS
#include  <malloc.h>
#define ALLOC_RETURN_TYPE void
#else /* end of DOS case */
#define ALLOC_RETURN_TYPE char
#endif /* not DOS */
#endif /* not ALLOC_RETURN_TYPE */

/* extern ALLOC_RETURN_TYPE *calloc (), *malloc (), *realloc (); */
extern ALLOC_RETURN_TYPE *calloc (size_t, size_t);
extern ALLOC_RETURN_TYPE *malloc (size_t);
extern ALLOC_RETURN_TYPE *realloc (void *, size_t);
#endif /* not STDC_HEADERS */

/* strchr vs. index, memcpy vs. bcopy, etc.  */
#include "c-memstr.h"

/* Error numbers and errno declaration.  */
#include "c-errno.h"

/* Numeric minima and maxima.  */
#include "c-minmax.h"

/* popen is part of POSIX.2, not POSIX.1.  So STDC_HEADERS isn't enough.  */
/* extern FILE *popen (); */
/* extern double hypot (); */
/* extern double hypot (double, double); */	/* see math.h */

#endif /* not C_STD_H */
