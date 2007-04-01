/* c-memstr.h: memcpy, strchr, etc.

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

#ifndef C_MEMSTR_H
#define C_MEMSTR_H

#include "c-std.h"

/* Just to be complete, we make both the system V/ANSI and the BSD
   versions of the string functions available.  */
#if STDC_HEADERS || HAVE_STRING_H
#include <string.h>

/* An ANSI string.h and pre-ANSI memory.h might conflict.  */
#if !STDC_HEADERS && HAVE_MEMORY_H
#include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */

#define index strchr
#define rindex strrchr

#ifndef bcmp
#define bcmp(s1, s2, len) memcmp ((s1), (s2), (len))
#endif
#ifndef bcopy
#define bcopy(from, to, len) memcpy ((to), (from), (len))
#endif
#ifndef bzero
#define bzero(s, len) memset ((s), 0, (len))
#endif

#else /* not STDC_HEADERS and not HAVE_STRING_H */

#include <strings.h>

#define strchr index
#define strrchr rindex

#define memcmp(s1, s2, n) bcmp ((s1), (s2), (n))
#define memcpy(to, from, len) bcopy ((from), (to), (len))

extern char *strtok(char *, const char *); /* extern char *strtok (); */
extern char *strstr(const char *, const char *); /* extern char *strstr (); */

#endif /* not STDC_HEADERS and not HAVE_STRING_H */

#endif /* not C_MEMSTR_H */
