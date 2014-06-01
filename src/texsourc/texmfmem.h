/* Copyright 1992 Karl Berry
   Copyright 2007 TeX Users Group
   Copyright 2014 Clerk Ma

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

/* texmfmem.h: the memory_word type, which is too hard to translate
   automatically from Pascal.  We have to make sure the byte-swapping
   that the (un)dumping routines do suffices to put things in the right
   place in memory.

   A memory_word can be broken up into a `two_halves' or a
   `four_quarters', and a `two_halves' can be further broken up.  Here is
   a picture.  ..._M = most significant byte, ..._L = least significant
   byte.
   
   If BigEndian:
   two_halves.v:  RH_M  RH_L  LH_M  LH_L
   two_halves.u:  JNK1  JNK2    B0    B1
   four_quarters:   B0    B1    B2    B3
   
   If LittleEndian:
   two_halves.v:  LH_L  LH_M  RH_L  RH_M
   two_halves.u:    B1    B0  JNK1  JNK2
   four_quarters:   B3    B2    B1    B0
   
   The halfword fields are four bytes if we are building a TeX or MF;
   this leads to further complications:
   
   BigEndian:
   two_halves.v:  RH_MM RH_ML RH_LM RH_LL LH_MM LH_ML LH_LM LH_LL
   two_halves.u:  ---------JUNK----------  B0    B1
   four_quarters:   B0    B1    B2    B3

   LittleEndian:
   two_halves.v:  LH_LL LH_LM LH_ML LH_MM RH_LL RH_LM RH_ML RH_MM
   two_halves.u:  junkx junky  B1    B0
   four_quarters: ---------JUNK----------  B3    B2    B1    B0

   I guess TeX and Metafont never refer to the B1 and B0 in the
   four_quarters structure as the B1 and B0 in the two_halves.u structure.
   
   This file can't be part of texmf.h, because texmf.h gets included by
   {tex,mf}d.h before the `halfword' etc. types are defined.  So we
   include it from the change file instead.
*/

/*
  meaning      structure                      TeX                 Y&Y TeX
               ----------------------------------------------------------------------
  integer      |            int            || 4: long           | 8: long long      |   min_quarterword 0
               ---------------------------------------------------------------------- max_quarterword FFFF
  scaled       |            sc             || 4: long           | 8: long long      |   min_halfword
               ----------------------------------------------------------------------
  glue_ratio   |            gr             || 4: float          | 8: double         |
               ----------------------------------------------------------------------
  halfword     |     lh      |     rh      || 2: unsigned short | 4: unsigned long  |
               ----------------------------------------------------------------------
  half+quarter |  b0  |  b1  |     rh      ||                                       |
               ----------------------------------------------------------------------
  quarter      |  b0  |  b1  |  b2  |  b3  || 1: unsigned char  | 2: unsigned short |
               ----------------------------------------------------------------------
*/

typedef struct
{
#ifdef WORDS_BIGENDIAN
  halfword rh;

  union
  {
    halfword lh;

    struct
    {
      quarterword b0, b1;
    };
  };
#endif
} two_halves;

typedef struct
{
#ifdef WORDS_BIGENDIAN
  quarterword b0, b1, b2, b3;
#else
  quarterword b3, b2, b1, b0;
#endif
} four_quarters;

typedef union
{
  glue_ratio gr;
  two_halves hh;
  integer cint;
  four_quarters qqqq;
} memory_word;

#ifndef WORDS_BIGENDIAN
#define cint u.CINT
#define qqqq v.QQQQ
#endif