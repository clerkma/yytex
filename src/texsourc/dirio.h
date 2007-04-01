/* dirio.h: checked directory operations.

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

#ifndef MSDOS

#ifndef DIRIO_H
#define DIRIO_H

/* #if DIRENT || _POSIX_VERSION */
#if defined(DIRENT) || defined(_POSIX_VERSION)			/* 93/Dec/8 */
#include <dirent.h>
#define NLENGTH(dirent) strlen ((dirent)->d_name)
#else /* not (DIRENT or _POSIX_VERSION) */
#define dirent direct
#define NLENGTH(dirent) ((dirent)->d_namlen)

#ifdef SYSNDIR
#include <sys/ndir.h>
#endif /* not SYSNDIR */

#ifdef NDIR
#include <ndir.h>
#endif /* not NDIR */

#ifdef SYSDIR
#include <sys/dir.h>
#endif /* not SYSDIR */

#endif /* not (DIRENT or _POSIX_VERSION) */

/* Like opendir, closedir, and chdir, but abort if DIRNAME can't be opened.  */
extern DIR *xopendir P1H(string);
extern void xclosedir P1H(DIR *);
#endif /* not DIRIO_H */

#endif /* not DOS */ /* was: not DIRIO_H, which is wrong - bkph */

/* #if !defined(DOS) || defined(PHARLAP) */ /* 1999/Jan/9 */

/* Returns true if FN is a directory (or a symlink to a directory).  */
extern booleane dir_p P1H(string fn);

/* Returns true if FN is directory with no subdirectories.  */
extern booleane leaf_dir_p P1H(string fn);

/* #endif */ /* not DOS, or DOS with PHARLAP */

