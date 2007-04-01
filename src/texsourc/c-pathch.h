/* c-pathch.h: define the characters which separate components of
   pathnames and environment variable paths.

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

#ifndef C_PATHCH_H
#define C_PATHCH_H

/* What separates pathname components?  */
#ifndef PATH_SEP
#ifdef VMS
#define PATH_SEP ':'
#define PATH_SEP_STRING ":"
#else
#ifdef MSDOS
#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#else
#ifdef VMCMS
#define PATH_SEP ' '
#define PATH_SEP_STRING " "
#else
#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#endif /* not VM/CMS */
#endif /* not DOS */
#endif /* not VMS */
#endif /* not PATH_SEP */

/* What separates elements in environment variable path lists?  */
#ifndef PATH_DELIMITER
#ifdef VMS
#define PATH_DELIMITER ','
#define PATH_DELIMITER_STRING ","
#else
#ifdef MSDOS
#define PATH_DELIMITER ';'
#define PATH_DELIMITER_STRING ";"
#else
#ifdef VMCMS
#define PATH_DELIMITER ' '
#define PATH_DELIMITER_STRING " "
#else
#define PATH_DELIMITER ':'
#define PATH_DELIMITER_STRING ":"
#endif /* not VM/CMS */
#endif /* not DOS */
#endif /* not VMS */
#endif /* not PATH_DELIMITER */

#endif /* not C_PATHCH_H */
