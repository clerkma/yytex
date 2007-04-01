/* c-name_max.h: define NAME_MAX, the maximum length of a single
   component in a pathname.

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

#ifndef C_NAME_MAX_H
#define C_NAME_MAX_H

#include "c-limits.h"

#ifdef _POSIX_VERSION
#ifndef NAME_MAX
#define NAME_MAX pathconf ("/", _PC_NAME_MAX)
#endif
#endif /* not _POSIX_VERSION */

/* Most likely the system will truncate filenames if it is not POSIX,
   and so we can use the BSD value here.  */
#ifndef _POSIX_NAME_MAX
#define _POSIX_NAME_MAX 255
#endif

#ifndef NAME_MAX
#define NAME_MAX _POSIX_NAME_MAX
#endif

#endif /* not C_NAME_MAX_H */
