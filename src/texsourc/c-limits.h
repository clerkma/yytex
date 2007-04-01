/* c-limits.h: include the system parameter file.

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

#ifndef C_LIMITS_H
#define C_LIMITS_H

#include "c-std.h"

#ifdef LIMITS_H_MISSING
#include <sys/param.h>
#else
#include <limits.h>
#endif

#if !defined (FLOAT_H_MISSING) && !defined (FLT_MAX)
#include <float.h>
#endif

#endif /* not C_LIMITS_H */
