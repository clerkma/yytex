/* Copyright 2014 Clerk Ma

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

#ifndef _YANDYTEX_H
#define _YANDYTEX_H

#define WORDS_BIGENDIAN 0

//#pragma warning(disable:4032)
#pragma warning(disable:4996) //
#pragma warning(disable:4701) //
//#pragma warning(disable:4100)
//#pragma warning(disable:4244)
#pragma warning(disable:4131) // old style declarator
#pragma warning(disable:4135) // conversion between different integral types
#pragma warning(disable:4127) // conditional expression is constant

/* ``Standard'' headers.  */
#include <kpathsea/c-auto.h>
#include <kpathsea/c-std.h>
#include <kpathsea/c-pathmx.h>
#include <kpathsea/c-fopen.h>
#include <kpathsea/c-ctype.h>
#include <kpathsea/c-proto.h>
#include <kpathsea/config.h>
#include <kpathsea/getopt.h>
#include <kpathsea/lib.h>
#include <kpathsea/line.h>
#include <kpathsea/readable.h>
#include <kpathsea/types.h>
#include <kpathsea/tex-file.h>
#include <kpathsea/variable.h>
#include <kpathsea/absolute.h>
#include <stdarg.h>
#include <setjmp.h>
#include <time.h>
#include <signal.h>
#ifdef MSDOS
#include <kpathsea/win32lib.h>
#include <conio.h>
#define getch _getch
#else
#include <unistd.h>
#endif
#include "hpdf.h"
#include "hpdf_utils.h"
#include "avl.h"

typedef int         integer;
typedef double      glue_ratio;
typedef boolean     bool;
typedef double      real;
typedef FILE *      alpha_file;
typedef unsigned char ASCII_code;
typedef unsigned short KANJI_code;
typedef unsigned char eight_bits;
typedef unsigned short sixteen_bits;
typedef integer pool_pointer;
typedef integer str_number;
typedef unsigned char packed_ASCII_code;
typedef integer scaled;
typedef integer nonnegative_integer;
typedef char small_number;

typedef enum
{
  NO_FILE_PATH = -1,
  TEXFORMATPATH,
  TEXINPUTPATH,
  TFMFILEPATH,
  LAST_PATH
} path_constant_type;

#define TEXFORMATPATHBIT (1 << TEXFORMATPATH)
#define TEXINPUTPATHBIT  (1 << TEXINPUTPATH)
#define TFMFILEPATHBIT   (1 << TFMFILEPATH)

#ifdef link
  #undef link
#endif

#define abs(x)   ((integer)(x) >= 0 ? (integer)(x) : (integer)-(x))
#define chr(x)   (x)
#define ord(x)   (x)
#define odd(x)   ((x) % 2)
#define round(x) zround ((double) (x))
#define decr(x)  --(x)
#define incr(x)  ++(x)
#define fabs(x)  ((x) >= 0.0 ? (x) : -(x))
#define PATHMAX  PATH_MAX
#define toint(x) ((integer) (x))
#define a_open_in(f,p) open_input (&(f), p, FOPEN_R_MODE)
#define a_open_out(f)   open_output (&(f), FOPEN_W_MODE)
#define a_close(f)	check_fclose (f)

#define MAXLINE 256         // for log_line buffer

extern bool trace_flag;
extern bool open_trace_flag;

extern integer zround (double);
extern bool eoln (FILE * file);
extern bool open_input (FILE **f, path_constant_type path_index, char *fopen_mode);
extern bool open_output (FILE **f, char *fopen_mode);
extern int check_fclose (FILE * f);

#define show_line(str,flag) fputs(str,stdout)
#define show_char(chr) putc(chr, stdout)
extern char log_line[];

#endif