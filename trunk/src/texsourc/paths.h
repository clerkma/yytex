/* Copyright 1992 Karl Berry
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

/* Generated from paths.h.in (Mon Mar  8 04:40:51 SST 1993).  */
/* Paths.  */

/* The meanings of these paths are described in the man pages.
   This file is created from the defaults in Makefile.in.  */

/* #define TEXPATH "c:/texas/" */

/* #define TEXPATH "c:/y&ytex/" */

/* #define TEXPATH "c:/yandytex/" */

#define TEXPATH "c:/yandy/yandytex/" /* changed default 97/Mar/22 */

#ifndef TEXFONTS
/* #define TEXFONTS "c:/texas/tfm//" */
#define TEXFONTS TEXPATH "tfm"
#endif

#ifndef TEXFORMATS
/* #define TEXFORMATS "c:/texas/fmt" */
#define TEXFORMATS TEXPATH "fmt"
#endif

#ifndef TEXINPUTS
/* #define TEXINPUTS "c:/texas/latex;c:/tex//" */
/* #define TEXINPUTS TEXPATH "latex;" "c:/tex;" "c:/texinputs" */
#define TEXINPUTS TEXPATH "tex//;" "c:/tex;" "c:/texinput" /* 97/Mar/22 */
#endif

#ifndef TEXPOOL
/* #define TEXPOOL "c:/texas/fmt" */
#define TEXPOOL TEXPATH "fmt"
#endif

#ifndef BIBINPUTS
/* #define BIBINPUTS "c:/texas/bib//" */
#define BIBINPUTS TEXPATH "bib//"
#endif

#ifndef BSTINPUTS
/* #define BSTINPUTS ".;c:/texas/bib//" */
#define BSTINPUTS ".;" TEXPATH "bib//"
#endif

#ifndef MFBASES
#define MFBASES ".;c:/mf/bases"
#endif

#ifndef MFINPUTS
#define MFINPUTS "c:/mf/inputs//"
#endif

#ifndef MFPOOL
#define MFPOOL "c:/mf/bases"
#endif

/* Of course, TeX itself doesn't give a damn about any of the following: */

#ifndef GFFONTS
#define GFFONTS ""
#endif

#ifndef PKFONTS
/* #define PKFONTS "c:/texas/pixels//" */
#define PKFONTS TEXPATH "pixels//"
#endif

#ifndef VFFONTS
/* #define VFFONTS "c:/texas/fonts//" */
#define VFFONTS TEXPATH "fonts//"
#endif

