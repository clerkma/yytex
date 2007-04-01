/* types.h: general types.

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

#ifndef TYPES_H
#define TYPES_H

/* Booleans.  */
typedef enum { false = 0, true = 1 } booleane;

// NOTE: boolean conflicts with rpcndr.h in Windows --- unsigned char

/* The X11 library defines `FALSE' and `TRUE', and so we only want to
   define them if necessary.  */
#ifndef FALSE
#define FALSE false
#define TRUE true
#endif /* FALSE */

/* The usual null-terminated string.  */
typedef char *string;

/* A generic pointer in ANSI C.  */
typedef void *address;

#endif /* not TYPES_H */
