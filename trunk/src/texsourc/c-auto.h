/* lib/c-auto.h.  Generated automatically by configure.  */
/* c-auto.h.in: template for c-auto.h.  */

/* Copyright 2007 TeX Users Group

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

/* First, here are user-dependent definitions.  configure doesn't change
   any of these by default; if you want different values, the most
   convenient thing to do is edit this file you run configure.
   
   Alternatively, you can set an environment variable DEFS before
   running configure, as in:
	DEFS="-DSMALLTeX -DNO_FMTBASE_SWAP"
   
   Another alternative is to redefine values via CFLAGS when you
   compile, although then of course whatever configure might have done
   automatically based on these definitions won't happen.  */


/* Define these if you want to compile the small (64K memory) TeX/MF.
   The default is to compile the big (260K memory) versions.
   Similarly for BibTeX.  */

#ifndef SMALLTeX
#undef SMALLTeX
#endif
#ifndef SMALLMF
#undef SMALLMF
#endif
#ifndef SMALLBibTeX
#undef SMALLBibTeX
#endif

/* Metafont online output support: More than one may be defined, except
   that you can't have both X10 and X11 support (because there are
   conflicting routine names in the libraries), or both X11 and XView
   support, for the same reason.
   
   If you want X11 support, see the `Online output from Metafont'
   section in README before compiling.  */
#ifndef HP2627WIN
#undef HP2627WIN	/* HP 2627 */
#endif
#ifndef SUNWIN
#undef SUNWIN		/* SunWindows */
#endif
#ifndef TEKTRONIXWIN
#undef TEKTRONIXWIN	/* Tektronix 4014 */
#endif
#ifndef UNITERMWIN
#undef UNITERMWIN	/* Uniterm Tektronix  */
#endif
#ifndef XVIEWWIN
#undef XVIEWWIN		/* Sun OpenWindows  */
#endif
#ifndef X10WIN
#undef X10WIN		/* X Version 10 */
#endif
#ifndef X11WIN
#undef X11WIN		/* X Version 11 */
#endif

#ifdef XVIEWWIN
#undef X11WIN
#endif

/* If you want to invoke MakeTeX{TeX,TFM,MF} when files can't be found,
   define this.  If you don't have such scripts, the downside is an
   extra fork/exec on nonexistent files.  */
#ifndef NO_MAKETEX
#undef NO_MAKETEX
#endif

/* Default editor command string: `%d' expands to the line number where
   TeX or Metafont found an error and `%s' expands to the name of the
   file.  The environment variables TEXEDIT and MFEDIT override this.  */
#ifndef EDITOR
/* #define EDITOR "vi +%d %s" */
/* #define EDITOR "epsilon +%d %s" */		/* better default for DOS bkph */
#define EDITOR "c:\\yandy\\WinEdt\\WinEdt.exe [Open('%s');SelLine(%d,7)]"
#endif

/* If you don't want to be able to potentially share format/base files
   across architectures, define NO_FMTBASE_SWAP.  Sharable files load
   slower on LittleEndian machines.  */
#ifndef NO_FMTBASE_SWAP
#undef NO_FMTBASE_SWAP
#endif

/* #define NO_FMTBASE_SWAP *//* Don't want to share format files bkph ? */

/* If you want to be able to produce a core dump (to make a preloaded
   TeX/MF) with the input filename `HackyInputFileNameForCoreDump.tex'.
   define this.  */
#ifndef FUNNY_CORE_DUMP
#undef FUNNY_CORE_DUMP
#endif

/* Redefine this only if you are using some non-standard TeX
   variant which has a different string pool, e.g., Michael Ferguson's
   MLTeX.  You may also need to uncomment the line defining $(tex9_o)
   in `tex/Makefile'.  */
#ifndef TEXPOOLNAME
#define TEXPOOLNAME "tex.pool"
#endif

/* Our character set is 8-bit ASCII unless NONASCII is defined; put
   another way, `xord' and `xchr' are *ignored* unless NONASCII is
   defined.  For other character sets, make sure that first_text_char
   and last_text_char are defined correctly (they're 0 and 255,
   respectively, by default).  In the *.defines files, change the
   indicated range of type `char' to be the same as
   first_text_char..last_text_char, `#define NONASCII', and retangle and
   recompile everything.  */
#ifndef NONASCII
#undef NONASCII
#endif

/* above has been changed to command line controlled flag - bkph */

/* On some systems, explicit register declarations make a big
   difference.  On others, they make no difference at all.  The GNU C
   compiler ignores them when optimizing.  */
#ifndef REGFIX
#undef REGFIX
#endif

/* This is end of new stuff - rest is like c-auto.h in melvin bkph */


/* And second, here are system-dependent definitions.  configure does
   try to figure these out.  */

/* Define as the proper declaration for yytext.  */
/* #define DECLARE_YYTEXT extern char  *yytext; */
#define DECLARE_YYTEXT extern char yytext[];	/* bkph */

/* Define if you have dirent.h.  */
#define DIRENT 1

/* Define if you have unistd.h.  */
/* #define HAVE_UNISTD_H 1 */ /* ours does not - bkph */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you don't have dirent.h, but have sys/dir.h.  */
#undef SYSDIR

/* Define if you don't have dirent.h, but have sys/ndir.h.  */
#undef SYSNDIR

/* Define if the closedir function returns void instead of int.  */
#undef VOID_CLOSEDIR

/* Define if your processor stores words with the most significant
   byte first (like Motorola and Sparc, unlike Intel and VAX).  */
#define WORDS_BIGENDIAN 0 

/* #undef WORDS_BIGENDIAN */		/* surely on MSDOS - bkph ??? */

/* Define if on AIX 3.  */
#undef _ALL_SOURCE

/* Define if on MINIX.  */
#undef _MINIX

/* Define if on MINIX.  */
#undef _POSIX_1_SOURCE

/* Define if you need to in order for stat and other things to work.  */
#undef _POSIX_SOURCE

/* Define if type char is unsigned and you are not using gcc.  */
#undef __CHAR_UNSIGNED__

/* Define to empty if the keyword does not work.  */
#undef const

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if your system lacks <limits.h>.  */
#undef LIMITS_H_MISSING

/* Likewise, for <float.h>.  */
#undef FLOAT_H_MISSING

/* Define as `float' if making a ``small'' TeX.  */
#undef GLUERATIO_TYPE
