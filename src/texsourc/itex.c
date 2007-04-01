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

#ifdef _WINDOWS
#define NOCOMM
#define NOSOUND
#define NODRIVERS
#define STRICT
#pragma warning(disable:4115)	// kill rpcasync.h complaint
#include <windows.h>
#endif

#ifdef _WINDOWS
// We must define MYLIBAPI as __declspec(dllexport) before including
// texwin.h, then texwin.h will see that we have already
// defined MYLIBAPI and will not (re)define it as __declspec(dllimport)
#define MYLIBAPI __declspec(dllexport)
// #include "texwin.h"
#endif

#include "texwin.h"

#pragma warning(disable:4131)	// old style declarator
#pragma warning(disable:4135)	// conversion between different integral types 
#pragma warning(disable:4127)	// conditional expression is constant

#include <setjmp.h>

#pragma hdrstop

#define EXTERN extern

#include "texd.h"

#pragma warning(disable:4244)				/* 96/Jan/10 */

#include <time.h>							// needed for clock_t

/* #define CHECKPOOL */						/* debugging only */

#define ENDFMTCHECKSUM 69069L

#define BEGINFMTCHECKSUM 367403084L

clock_t starttime, maintime, finishtime;	/* in local.c */

/* imported from pascal.h */
/* localized here to avoid conflict with io.h in other code */

#define read(f, b)	((b) = getc (f)) 
#define	readln(f)	{ register c; \
                          while ((c = getc (f)) != '\n' && c != EOF); } 
/* #define	readln(f)	{ while (getc (f) >= ' '); } ? use fgets ? */

#ifdef INITEX
void do_initex (void);					/* later in this file */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void initialize (void) 
{initialize_regmem 
  integer i  ; 
  integer k  ; 
  integer flag ;						/* bkph */
#ifndef ALLOCATEHYPHEN
  hyphpointer z  ;  
#endif

/* We already initialized xchr if we are using nonascii setup */

if (!nonascii) {

/*	Normally, there is no translation of character code numbers */

  {
	  register integer for_end;
	  i = 0 ;
	  for_end = 255 ;
	  if ( i <= for_end)
		  do 
/*			  xchr [ i ] = chr ( i ) ;  */
			  xchr [ i ] = (char) i ;						/* 95/Jan/7 */
	  while ( i++ < for_end ) ;
  } 

/* This is a kind of joke, since the characters are given as numeric codes ! */

#ifdef JOKE
  xchr [ 32 ] = ' ' ; 
  xchr [ 33 ] = '!' ; 
  xchr [ 34 ] = '"' ; 
  xchr [ 35 ] = '#' ; 
  xchr [ 36 ] = '$' ; 
  xchr [ 37 ] = '%' ; 
  xchr [ 38 ] = '&' ; 
  xchr [ 39 ] = '\'' ; 
  xchr [ 40 ] = '(' ; 
  xchr [ 41 ] = ')' ; 
  xchr [ 42 ] = '*' ; 
  xchr [ 43 ] = '+' ; 
  xchr [ 44 ] = ',' ; 
  xchr [ 45 ] = '-' ; 
  xchr [ 46 ] = '.' ; 
  xchr [ 47 ] = '/' ; 
  xchr [ 48 ] = '0' ; 
  xchr [ 49 ] = '1' ; 
  xchr [ 50 ] = '2' ; 
  xchr [ 51 ] = '3' ; 
  xchr [ 52 ] = '4' ; 
  xchr [ 53 ] = '5' ; 
  xchr [ 54 ] = '6' ; 
  xchr [ 55 ] = '7' ; 
  xchr [ 56 ] = '8' ; 
  xchr [ 57 ] = '9' ; 
  xchr [ 58 ] = ':' ; 
  xchr [ 59 ] = ';' ; 
  xchr [ 60 ] = '<' ; 
  xchr [ 61 ] = '=' ; 
  xchr [ 62 ] = '>' ; 
  xchr [ 63 ] = '?' ; 
  xchr [ 64 ] = '@' ; 
  xchr [ 65 ] = 'A' ; 
  xchr [ 66 ] = 'B' ; 
  xchr [ 67 ] = 'C' ; 
  xchr [ 68 ] = 'D' ; 
  xchr [ 69 ] = 'E' ; 
  xchr [ 70 ] = 'F' ; 
  xchr [ 71 ] = 'G' ; 
  xchr [ 72 ] = 'H' ; 
  xchr [ 73 ] = 'I' ; 
  xchr [ 74 ] = 'J' ; 
  xchr [ 75 ] = 'K' ; 
  xchr [ 76 ] = 'L' ; 
  xchr [ 77 ] = 'M' ; 
  xchr [ 78 ] = 'N' ; 
  xchr [ 79 ] = 'O' ; 
  xchr [ 80 ] = 'P' ; 
  xchr [ 81 ] = 'Q' ; 
  xchr [ 82 ] = 'R' ; 
  xchr [ 83 ] = 'S' ; 
  xchr [ 84 ] = 'T' ; 
  xchr [ 85 ] = 'U' ; 
  xchr [ 86 ] = 'V' ; 
  xchr [ 87 ] = 'W' ; 
  xchr [ 88 ] = 'X' ; 
  xchr [ 89 ] = 'Y' ; 
  xchr [ 90 ] = 'Z' ; 
  xchr [ 91 ] = '[' ; 
  xchr [ 92 ] = '\\' ; 
  xchr [ 93 ] = ']' ; 
  xchr [ 94 ] = '^' ; 
  xchr [ 95 ] = '_' ; 
  xchr [ 96 ] = '`' ; 
  xchr [ 97 ] = 'a' ; 
  xchr [ 98 ] = 'b' ; 
  xchr [ 99 ] = 'c' ; 
  xchr [ 100 ] = 'd' ; 
  xchr [ 101 ] = 'e' ; 
  xchr [ 102 ] = 'f' ; 
  xchr [ 103 ] = 'g' ; 
  xchr [ 104 ] = 'h' ; 
  xchr [ 105 ] = 'i' ; 
  xchr [ 106 ] = 'j' ; 
  xchr [ 107 ] = 'k' ; 
  xchr [ 108 ] = 'l' ; 
  xchr [ 109 ] = 'm' ; 
  xchr [ 110 ] = 'n' ; 
  xchr [ 111 ] = 'o' ; 
  xchr [ 112 ] = 'p' ; 
  xchr [ 113 ] = 'q' ; 
  xchr [ 114 ] = 'r' ; 
  xchr [ 115 ] = 's' ; 
  xchr [ 116 ] = 't' ; 
  xchr [ 117 ] = 'u' ; 
  xchr [ 118 ] = 'v' ; 
  xchr [ 119 ] = 'w' ; 
  xchr [ 120 ] = 'x' ; 
  xchr [ 121 ] = 'y' ; 
  xchr [ 122 ] = 'z' ; 
  xchr [ 123 ] = '{' ; 
  xchr [ 124 ] = '|' ; 
  xchr [ 125 ] = '}' ; 
  xchr [ 126 ] = '~' ; 

  {
	  register integer for_end;
	  i = 0 ;
	  for_end = 31 ;
	  if ( i <= for_end)
		  do  xchr [ i ] = chr ( i ) ; 
	  while ( i++ < for_end ) ;
  } 

  {
	  register integer for_end;
	  i = 127 ;
	  for_end = 255 ;
	  if ( i <= for_end)
		  do xchr [ i ] = chr ( i ) ; 
	  while ( i++ < for_end ) ;
  }
  #endif /* end of JOKE */
} /* end of plain ASCII case (otherwise have read in xchr vector before) */

/*	Fill in background of `delete' for inverse mapping */

  {
	  register integer for_end;
	  i = 0 ;
	  for_end = 255 ;
	  if ( i <= for_end)
		  do xord [ chr ( i ) ] = 127 ; 
	  while ( i++ < for_end ) ;
  } 

#ifdef JOKE
  {
	  register integer for_end;
	  i = 128 ;
	  for_end = 255 ;
	  if ( i <= for_end)
		  do xord [ xchr [ i ] ] = i ; 
	  while ( i++ < for_end ) ;
  } 

  {
	  register integer for_end;
	  i = 0 ;
	  for_end = 126 ;
	  if ( i <= for_end)
		  do xord [ xchr [ i ] ] = i ; 
	  while ( i++ < for_end ) ;
  } 
#endif
  
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*	Now invert the xchr mapping to get xord mapping */

  {
	  register integer for_end;
	  i = 0 ;
	  for_end = 255 ;
	  if ( i <= for_end)
		  do xord [ xchr [ i ] ] = (char) i ;  
	  while ( i++ < for_end ) ;
  } 

  xord [ 127 ] = 127;		/* hmm, a fixed point ? why ? */

  flag = 0;					/* 93/Dec/28 - bkph */
  if (traceflag != 0) {
	  for (k = 0; k < 256; k++)		/*  entries in xord / xchr */
		  if (xord [ k ] != k ) {
			  flag = 1;
			  break;
		  }
	  if (flag) {		/* 127 here means mapping undefined */
		  showline("Inverted mapping xord[] pairs:\n", 0);
		  for (k = 0; k < 256; k++) {	/*  entries in xord / xchr */
			  if (xord [ k ] != 127) {
				  sprintf(logline, "%d => %d\n", k, xord [ k ]);
				  showline(logline, 0);
			  }
		  }
	  }
  }
  if (interaction < 0)				/* may now set in local.c bkph */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  interaction = 3 ; 
  deletionsallowed = true ; 
  setboxallowed = true ; 
  errorcount = 0 ; 
  helpptr = 0 ; 
  useerrhelp = false ; 
  interrupt = 0 ; 
  OKtointerrupt = true ; 
	;
/* darn, need to know memtop, memmax etc for the following ... */
#ifdef DEBUG
  wasmemend = memmin ; 
/*  waslomax = memmin ;  */
  waslomax = membot ; 
/*  washimin = memmax ; */
  washimin = memtop ; 
  panicking = false ; 
#endif /* DEBUG */
  nestptr = 0 ; 
  maxneststack = 0 ; 
  curlist .modefield = 1 ; 
  curlist .headfield = memtop - 1 ; 
  curlist .tailfield = memtop - 1 ; 
  curlist .auxfield .cint = -65536000L ; 
  curlist .mlfield = 0 ; 
/* *********************************************************************** */
/*  curlist .lhmfield = 0 ; */ /* removed in 3.14159 */
/*  curlist .rhmfield = 0 ; */ /* removed in 3.14159 */
/* *********************************************************************** */
  curlist .pgfield = 0 ; 
  shownmode = 0 ; 
  pagecontents = 0 ; 
  pagetail = memtop - 2 ; 
#ifdef ALLOCATEMAIN
  if (is_initex)			/* in iniTeX we did already allocate mem [] */
#endif
	  mem [ memtop - 2 ] .hh .v.RH = 0 ; 
/*  lastglue = 262143L ;  */ /* NO! */
  lastglue = emptyflag ; 
  lastpenalty = 0 ; 
  lastkern = 0 ; 
  pagesofar [ 7 ] = 0 ; 
  pagemaxdepth = 0 ; 
/* from int_base to eqtb_size */
  {
	  register integer for_end;
	  k = (hash_size + 3163) ;
	  for_end = (hash_size + 4006) ;
	  if ( k <= for_end) 
		  do  xeqlevel [ k ] = 1 ; 
	  while ( k++ < for_end ) ;
  } 
  nonewcontrolsequence = true ; 
  hash [ 514 ] .v.LH = 0 ;			/* next(hash_base):= 0 */
  hash [ 514 ] .v.RH = 0 ;			/* text(hash_base):= 0 */
/* 514 + hash_size + 266 = hashbase + hashsize + 10 + fontmax  */
/* for k <- hashbase+1 to undefined_control_sequence - 1 do ... p.257 */
  {
	  register integer for_end;
	  k = 515 ;
	  for_end = (hash_size + 780) ;
	  if ( k <= for_end) 
		  do hash [ k ] = hash [ 514 ] ; 
	  while ( k++ < for_end ) ;
  } 
  saveptr = 0 ; 
  curlevel = 1 ; 
  curgroup = 0 ; 
  curboundary = 0 ; 
  maxsavestack = 0 ; 
  magset = 0 ; 
  curmark [ 0 ] = 0 ; 
  curmark [ 1 ] = 0 ; 
  curmark [ 2 ] = 0 ; 
  curmark [ 3 ] = 0 ; 
  curmark [ 4 ] = 0 ; 
  curval = 0 ; 
  curvallevel = 0 ; 
  radix = 0 ; 
  curorder = 0 ; 
  {
	  register integer for_end;
	  k = 0 ;
	  for_end = 16 ;
	  if ( k <= for_end)
		  do readopen [ k ] = 2 ; 
	  while ( k++ < for_end ) ;
  } 
  condptr = 0 ; 
  iflimit = 0 ; 
  curif = 0 ; 
  ifline = 0 ; 
  {
	  register integer for_end;
	  k = 0 ;
	  for_end = fontmax ;
	  if ( k <= for_end) 
		  do fontused [ k ] = false ; 
	  while ( k++ < for_end ) ;
  } 
  nullcharacter .b0 = 0 ; 
  nullcharacter .b1 = 0 ; 
  nullcharacter .b2 = 0 ; 
  nullcharacter .b3 = 0 ; 
  totalpages = 0 ; 
  maxv = 0 ; 
  maxh = 0 ; 
  maxpush = 0 ; 
  lastbop = -1 ; 
  doingleaders = false ; 
  deadcycles = 0 ; 
  curs = -1 ; 
  halfbuf = dvibufsize / 2 ; 
  dvilimit = dvibufsize ; 
  dviptr = 0 ; 
  dvioffset = 0 ; 
  dvigone = 0 ; 
/* down_ptr:=null; right_ptr:=null; l.12027 */
  downptr = 0 ; 
  rightptr = 0 ; 
  adjusttail = 0 ; 
  lastbadness = 0 ; 
  packbeginline = 0 ; 
  emptyfield .v.RH = 0 ; 
  emptyfield .v.LH = 0 ; 
  nulldelimiter .b0 = 0 ; 
  nulldelimiter .b1 = 0 ; 
  nulldelimiter .b2 = 0 ; 
  nulldelimiter .b3 = 0 ; 
  alignptr = 0 ; 
  curalign = 0 ; 
  curspan = 0 ; 
  curloop = 0 ; 
  curhead = 0 ; 
  curtail = 0 ; 
/*	*not* OK with ALLOCATEHYPHEN, since may not be allocated yet */
#ifndef ALLOCATEHYPHEN
/*  {register integer for_end; z = 0 ; for_end = 607 ; if ( z <= for_end) do */
  {
	  register integer for_end;
	  z = 0 ;
	  for_end = hyphen_prime ;
	  if ( z <= for_end) do 
	  {
		  hyphword [ z ] = 0 ; 
		  hyphlist [ z ] = 0 ; 
	  } 
	  while ( z++ < for_end ) ;
  } 
#endif
  hyphcount = 0 ; 
  outputactive = false ; 
  insertpenalties = 0 ; 
  ligaturepresent = false ; 
  cancelboundary = false ; 
  lfthit = false ; 
  rthit = false ; 
  insdisc = false ; 
  aftertoken = 0 ; 
  longhelpseen = false ; 
  formatident = 0 ; 
  {
	  register integer for_end;
	  k = 0 ;
	  for_end = 17 ;
	  if ( k <= for_end)
		  do  writeopen [ k ] = false ; 
	  while ( k++ < for_end ) ;
  } 
  editnamestart = 0 ; 
	;
#ifdef INITEX
/* initex stuff split off for convenience of optimization adjustments */
  if (is_initex)  do_initex ();
#else
/* trap the -i on command line situation if INITEX was NOT defined */
  if (is_initex) {
	  showline("Sorry, somebody forgot to make an INITEX!\n", 1);
  }
#endif /* not INITEX */
} 

/* do the part of initialize() that requires memtop, memmax or mem [ ] */
/* do this part after allocating main memory */

#ifdef ALLOCATEMAIN
void initialize_aux(void) {
	initialize_regmem								/* ??? */
#ifdef DEBUG
  wasmemend = memmin ; 
/*  waslomax = memmin ; */
  waslomax = membot ; 
/*  washimin = memmax ;  */
  washimin = memtop ; 
  panicking = false ; 
#endif /* DEBUG */
/*  nestptr = 0 ; */
/*  maxneststack = 0 ; */
  curlist .modefield = 1 ; 
  curlist .headfield = memtop - 1 ; 
  curlist .tailfield = memtop - 1 ; 
  curlist .auxfield .cint = -65536000L ; 
  curlist .mlfield = 0 ; 
/* *********************************************************************** */
/*  curlist .lhmfield = 0 ; */	 /* removed in 3.14159 */
/*  curlist .rhmfield = 0 ; */	 /* removed in 3.14159 */
/* *********************************************************************** */
  curlist .pgfield = 0 ; 
/*  shownmode = 0 ;  */
/*  pagecontents = 0 ;  */
  pagetail = memtop - 2 ; 
  mem [ memtop - 2 ] .hh .v.RH = 0 ;
}
#endif	// end of ifdef ALLOCATEMAIN

void zlinebreak ( finalwidowpenalty ) 
integer finalwidowpenalty ; 
{/* 30 31 32 33 34 35 22 */ linebreak_regmem 
  booleane autobreaking  ; 
  halfword prevp  ; 
  halfword q, r, s, prevs  ; 
  internalfontnumber f  ; 
/*  smallnumber j  ;  */
  int j  ;								/* 95/Jan/7 */
/*  unsigned char c  ;  */
  unsigned int c  ;						/* 95/Jan/7 */

/*  savedbadness = 0; */				/* 96/Feb/9 */
  packbeginline = curlist .mlfield ; 
  mem [ memtop - 3 ] .hh .v.RH = mem [ curlist .headfield ] .hh .v.RH ; 
  if ( ( curlist .tailfield >= himemmin ) ) 
  {
    mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( 10000 ) ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  else if ( mem [ curlist .tailfield ] .hh.b0 != 10 ) 
  {
    mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( 10000 ) ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  else {
      
    mem [ curlist .tailfield ] .hh.b0 = 12 ; 
    deleteglueref ( mem [ curlist .tailfield + 1 ] .hh .v.LH ) ; 
    flushnodelist ( mem [ curlist .tailfield + 1 ] .hh .v.RH ) ; 
    mem [ curlist .tailfield + 1 ] .cint = 10000 ; 
  } 
  mem [ curlist .tailfield ] .hh .v.RH = newparamglue ( 14 ) ; 
/* *********************************************************************** */
/* following is new in 3.14159 */
  initcurlang = curlist .pgfield % 65536L ; 
/*  initcurlang = curlist .pgfield & 65535L ; */	/* last 16 bits */
  initlhyf = curlist .pgfield / 4194304L ;			/* 2^22 */
/*  initlhyf = curlist .pgfield >> 22 ; */			/* top 10 bits */
  initrhyf = ( curlist .pgfield / 65536L ) % 64 ; 
/*  initrhyf = ( curlist .pgfield >> 16 ) & 65 ; */ /* next to last 6 bits */
/* *********************************************************************** */
  popnest () ; 
  noshrinkerroryet = true ; 
  if ( ( mem [ eqtb [ (hash_size + 789) ] .hh .v.RH ] .hh.b1 != 0 ) &&
	   ( mem [ eqtb [ (hash_size + 789) ] .hh .v.RH + 3 ] .cint != 0 ) ) 
  {
    eqtb [ (hash_size + 789) ] .hh .v.RH =
		finiteshrink ( eqtb [ (hash_size + 789) ] .hh .v.RH ) ; 
	ABORTCHECK;
  } 
  if ( ( mem [ eqtb [ (hash_size + 790) ] .hh .v.RH ] .hh.b1 != 0 ) &&
	   ( mem [ eqtb [ (hash_size + 790) ] .hh .v.RH + 3 ] .cint != 0 ) ) 
  {
    eqtb [ (hash_size + 790) ] .hh .v.RH =
		finiteshrink ( eqtb [ (hash_size + 790) ] .hh .v.RH ) ; 
	ABORTCHECK;
  } 
  q = eqtb [ (hash_size + 789) ] .hh .v.RH ; 
  r = eqtb [ (hash_size + 790) ] .hh .v.RH ; 
  background [ 1 ] = mem [ q + 1 ] .cint + mem [ r + 1 ] .cint ; 
  background [ 2 ] = 0 ; 
  background [ 3 ] = 0 ; 
  background [ 4 ] = 0 ; 
  background [ 5 ] = 0 ; 
  background [ 2 + mem [ q ] .hh.b0 ] = mem [ q + 2 ] .cint ; 
  background [ 2 + mem [ r ] .hh.b0 ] = background [ 2 + mem [ r ] .hh.b0 ] + 
  mem [ r + 2 ] .cint ; 
  background [ 6 ] = mem [ q + 3 ] .cint + mem [ r + 3 ] .cint ; 
  minimumdemerits = 1073741823L ;		/* 2^30 - 1 */	/* 40000000 hex - 1 */
  minimaldemerits [ 3 ] = 1073741823L ; 
  minimaldemerits [ 2 ] = 1073741823L ; 
  minimaldemerits [ 1 ] = 1073741823L ; 
  minimaldemerits [ 0 ] = 1073741823L ; 
  if ( eqtb [ (hash_size + 1312) ] .hh .v.RH == 0 ) 
  if ( eqtb [ (hash_size + 3747) ] .cint == 0 ) 
  {
    lastspecialline = 0 ; 
    secondwidth = eqtb [ (hash_size + 3733) ] .cint ; 
    secondindent = 0 ; 
  } 
  else {
      
    lastspecialline = abs ( eqtb [ (hash_size + 3204) ] .cint ) ; 
    if ( eqtb [ (hash_size + 3204) ] .cint < 0 ) 
    {
      firstwidth = eqtb [ (hash_size + 3733) ] .cint -
		  abs ( eqtb [ (hash_size + 3747) ] .cint ) ; 
      if ( eqtb [ (hash_size + 3747) ] .cint >= 0 ) 
      firstindent = eqtb [ (hash_size + 3747) ] .cint ; 
      else firstindent = 0 ; 
      secondwidth = eqtb [ (hash_size + 3733) ] .cint ; 
      secondindent = 0 ; 
    } 
    else {
	
      firstwidth = eqtb [ (hash_size + 3733) ] .cint ; 
      firstindent = 0 ; 
      secondwidth = eqtb [ (hash_size + 3733) ] .cint -
		   abs ( eqtb [ (hash_size + 3747) ] .cint ) ; 
      if ( eqtb [ (hash_size + 3747) ] .cint >= 0 ) 
      secondindent = eqtb [ (hash_size + 3747) ] .cint ; 
      else secondindent = 0 ; 
    } 
  } 
  else {
      
    lastspecialline = mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH ] .hh .v.LH - 1 ; 
    secondwidth = mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH + 2 * ( lastspecialline + 1 ) 
    ] .cint ; 
    secondindent = mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH + 2 * lastspecialline + 1 ] 
    .cint ; 
  } 
/* if looseness=0 then easy_line:=last_special_line */
  if ( eqtb [ (hash_size + 3182) ] .cint == 0 ) 
  easyline = lastspecialline ; 
/*  else easyline = 262143L ;  */ /* NO! */
  else easyline = emptyflag ; 
/* threshold:=pretolerance; */
  threshold = eqtb [ (hash_size + 3163) ] .cint ; 
  if ( threshold >= 0 ) 
  {
	;
#ifdef STAT
/*   if tracing_paragraphs>0 then */
    if ( eqtb [ (hash_size + 3195) ] .cint > 0 ) 
    {
      begindiagnostic () ; 
      printnl ( 927 ) ;			/* @firstpass */
    } 
#endif /* STAT */
    secondpass = false ; 
    finalpass = false ; 
	firstpasscount++;					/* 96 Feb 9 */
  } 
  else {
/*  threshold:=tolerance; second_pass:=true; */      
    threshold = eqtb [ (hash_size + 3164) ] .cint ; 
    secondpass = true ; 
/*   final_pass:=(emergency_stretch<=0); */
    finalpass = ( eqtb [ (hash_size + 3750) ] .cint <= 0 ) ; 
	;
#ifdef STAT
    if ( eqtb [ (hash_size + 3195) ] .cint > 0 ) 
    begindiagnostic () ; 
#endif /* STAT */
  } 
  while ( true ) {
/*  if threshold>inf_bad then threshold:=inf_bad; */
    if ( threshold > 10000 ) 
		threshold = 10000 ; 
    if ( secondpass ) 
    {
	;
#ifdef INITEX
  if (is_initex)  {	/* bkph */
      if ( trienotready ) inittrie () ; 
  }					/* bkph */
#endif /* INITEX */
/* ********************************************************************* */
/* following has changed in 3.14159 */
/*      curlang = 0 ;  */
      curlang = initcurlang ; 
/*      lhyf = curlist .lhmfield ; */
      lhyf = initlhyf ; 
/*      rhyf = curlist .rhmfield ;  */
      rhyf = initrhyf ; 
/* ********************************************************************* */
    } 
    q = getnode ( 3 ) ; 
    mem [ q ] .hh.b0 = 0 ; 
    mem [ q ] .hh.b1 = 2 ; 
    mem [ q ] .hh .v.RH = memtop - 7 ; 
    mem [ q + 1 ] .hh .v.RH = 0 ; 
    mem [ q + 1 ] .hh .v.LH = curlist .pgfield + 1 ; 
    mem [ q + 2 ] .cint = 0 ; 
    mem [ memtop - 7 ] .hh .v.RH = q ; 
    activewidth [ 1 ] = background [ 1 ] ; 
    activewidth [ 2 ] = background [ 2 ] ; 
    activewidth [ 3 ] = background [ 3 ] ; 
    activewidth [ 4 ] = background [ 4 ] ; 
    activewidth [ 5 ] = background [ 5 ] ; 
    activewidth [ 6 ] = background [ 6 ] ; 
/*	passive:=null; printed_node:=temp_head; pass_number:=0; */
    passive = 0 ; 
    printednode = memtop - 3 ; 
    passnumber = 0 ; 
    fontinshortdisplay = 0 ; 
    curp = mem [ memtop - 3 ] .hh .v.RH ; 
    autobreaking = true ; 
    prevp = curp ; 
/*  while (cur_p<>null)and(link(active)<>last_active) do */
    while ( ( curp != 0 ) && ( mem [ memtop - 7 ] .hh .v.RH != memtop - 7 ) ) 
    {
      if ( ( curp >= himemmin ) ) 
      {
	prevp = curp ; 
	do {
	    f = mem [ curp ] .hh.b0 ; 
	  activewidth [ 1 ] = activewidth [ 1 ] + fontinfo [ widthbase [ f ] + 
	  fontinfo [ charbase [ f ] + mem [ curp ] .hh.b1 ] .qqqq .b0 ] .cint 
	  ; 
	  curp = mem [ curp ] .hh .v.RH ; 
	} while ( ! ( ! ( curp >= himemmin ) ) ) ; 
      } 
      switch ( mem [ curp ] .hh.b0 ) 
      {case 0 : 
      case 1 : 
      case 2 : 
	activewidth [ 1 ] = activewidth [ 1 ] + mem [ curp + 1 ] .cint ; 
	break ; 
      case 8 : 
	if ( mem [ curp ] .hh.b1 == 4 ) 
	{
	  curlang = mem [ curp + 1 ] .hh .v.RH ;	/* ASCIIcode */
	  lhyf = mem [ curp + 1 ] .hh.b0 ; 
	  rhyf = mem [ curp + 1 ] .hh.b1 ; 
	} 
	break ; 
      case 10 : 
	{
	  if ( autobreaking ) 
	  {
	    if ( ( prevp >= himemmin ) ) 
	    trybreak ( 0 , 0 ) ; 
	    else if ( ( mem [ prevp ] .hh.b0 < 9 ) ) 
	    trybreak ( 0 , 0 ) ; 
/* *********************************************************************** */
/* following is new in 3.14159 */
	    else if ( ( mem [ prevp ] .hh.b0 == 11 ) && ( mem [ prevp ] .hh.b1 
	    != 1 ) ) 
	    trybreak ( 0 , 0 ) ; 
/* *********************************************************************** */
	  } 
	  if ( ( mem [ mem [ curp + 1 ] .hh .v.LH ] .hh.b1 != 0 ) && ( mem [ 
	  mem [ curp + 1 ] .hh .v.LH + 3 ] .cint != 0 ) ) 
	  {
	    mem [ curp + 1 ] .hh .v.LH = finiteshrink ( mem [ curp + 1 ] .hh .v.LH ) ; 
		ABORTCHECK;
	  } 
	  q = mem [ curp + 1 ] .hh .v.LH ; 
	  activewidth [ 1 ] = activewidth [ 1 ] + mem [ q + 1 ] .cint ; 
	  activewidth [ 2 + mem [ q ] .hh.b0 ] = activewidth [ 2 + mem [ q ] 
	  .hh.b0 ] + mem [ q + 2 ] .cint ; 
	  activewidth [ 6 ] = activewidth [ 6 ] + mem [ q + 3 ] .cint ; 
	  if ( secondpass && autobreaking ) 
	  {
	    prevs = curp ; 
	    s = mem [ prevs ] .hh .v.RH ; 
	    if ( s != 0 ) 
	    {
	      while ( true ) {
		if ( ( s >= himemmin ) ) 
		{
		  c = mem [ s ] .hh.b1 ; 
		  hf = mem [ s ] .hh.b0 ; 
		} 
		else if ( mem [ s ] .hh.b0 == 6 ) 
		if ( mem [ s + 1 ] .hh .v.RH == 0 ) 
		goto lab22 ; 
		else {
		    
		  q = mem [ s + 1 ] .hh .v.RH ; 
		  c = mem [ q ] .hh.b1 ; 
		  hf = mem [ q ] .hh.b0 ; 
		} 
		else if ( ( mem [ s ] .hh.b0 == 11 ) && ( mem [ s ] .hh.b1 == 
		0 ) ) 
		goto lab22 ; 
		else if ( mem [ s ] .hh.b0 == 8 ) 
		{
		  if ( mem [ s ] .hh.b1 == 4 ) 
		  {
		    curlang = mem [ s + 1 ] .hh .v.RH ; 	/* ASCIIcode */
		    lhyf = mem [ s + 1 ] .hh.b0 ; 
		    rhyf = mem [ s + 1 ] .hh.b1 ; 
		  } 
		  goto lab22 ; 
		} 
		else goto lab31 ; 
		if ( eqtb [ (hash_size + 2139) + c ] .hh .v.RH != 0 ) 
/*		signed unsigned mismatch ? (c is unsigned) */
		if ( ( eqtb [ (hash_size + 2139) + c ] .hh .v.RH == c ) || 
			 ( eqtb [ (hash_size + 3201) ] .cint > 0 ) ) 
		goto lab32 ; 
		else goto lab31 ; 
		lab22: prevs = s ; 
		s = mem [ prevs ] .hh .v.RH ; 
	      } 
	      lab32: hyfchar = hyphenchar [ hf ] ; 
/* if hyf_char<0 then goto done1; */
	      if ( hyfchar < 0 )  goto lab31 ; 
/* if hyf_char>255 then goto done1; */
	      if ( hyfchar > 255 )  goto lab31 ; /* ? */
	      ha = prevs ; 
	      if ( lhyf + rhyf > 63 )  goto lab31 ; 
	      hn = 0 ; 
	      while ( true ) {
		if ( ( s >= himemmin ) ) {
		  if ( mem [ s ] .hh.b0 != hf ) 
		  goto lab33 ; 
		  hyfbchar = mem [ s ] .hh.b1 ; 
		  c = hyfbchar ;			/*  unsigned char c  ;  */
		  if ( eqtb [ (hash_size + 2139) + c ] .hh .v.RH == 0 ) 
		  goto lab33 ; 
		  if ( hn == 63 ) 
		  goto lab33 ; 
		  hb = s ; 
		  incr ( hn ) ; 
		  hu [ hn ] = c ; 
/*		  long to short ... */
		  hc [ hn ] = eqtb [ (hash_size + 2139) + c ] .hh .v.RH ; 
		  hyfbchar = 256 ;		/* special mark */
		} 
		else if ( mem [ s ] .hh.b0 == 6 ) 
		{
		  if ( mem [ s + 1 ] .hh.b0 != hf ) 
		  goto lab33 ; 
/* j:=hn; q:=lig_ptr(s); l.17554 */ 
		  j = hn ; 
		  q = mem [ s + 1 ] .hh .v.RH ; 
/* if q>null then hyf_bchar:=character(q); l.17554  BUG ??? */
/*		  if ( q > 0 )  */
		  if ( q != 0 )						/* 94/Mar/23 BUG FIX */
		  hyfbchar = mem [ q ] .hh.b1 ; 
/* while q>null do l.17555 BUG ??? */
/*		  while ( q > 0 ) */
		  while ( q != 0 ) {				/* 94/Mar/23 BUG FIX */
		    c = mem [ q ] .hh.b1 ; 
		    if ( eqtb [ (hash_size + 2139) + c ] .hh .v.RH == 0 ) 
		    goto lab33 ; 
		    if ( j == 63 ) 
		    goto lab33 ; 
		    incr ( j ) ; 
		    hu [ j ] = c ; 
/*			long to short ... */
		    hc [ j ] = eqtb [ (hash_size + 2139) + c ] .hh .v.RH ; 
		    q = mem [ q ] .hh .v.RH ; 
		  } 
		  hb = s ; 
		  hn = j ; 
		  if ( odd ( mem [ s ] .hh.b1 ) ) 
		  hyfbchar = fontbchar [ hf ] ; 
		  else hyfbchar = 256 ; 		/* special mark */
		} 
		else if ( ( mem [ s ] .hh.b0 == 11 ) && ( mem [ s ] .hh.b1 == 
		0 ) ) 
		{	
		  hb = s ; 
/* ******************************************************************** */
		  hyfbchar = fontbchar [ hf ] ; 		/* new code in 3.14159 */
/* ******************************************************************** */
		} 
		else goto lab33 ; 
		s = mem [ s ] .hh .v.RH ; 
	      } 
	      lab33: ; 
	      if ( hn < lhyf + rhyf ) 
	      goto lab31 ; 
	      while ( true ) {
		if ( ! ( ( s >= himemmin ) ) ) 
		switch ( mem [ s ] .hh.b0 ) 
		{case 6 : 
		  ; 
		  break ; 
		case 11 : 
		  if ( mem [ s ] .hh.b1 != 0 ) 
		  goto lab34 ; 
		  break ; 
		case 8 : 
		case 10 : 
		case 12 : 
		case 3 : 
		case 5 : 
		case 4 : 
		  goto lab34 ; 
		  break ; 
		  default: 
		  goto lab31 ; 
		  break ; 
		} 
		s = mem [ s ] .hh .v.RH ; 
	      } 
	      lab34: ; 
	      hyphenate () ; 
	    } 
	    lab31: ; 
	  } 
	} 
	break ; 
      case 11 : 
/* ******************************************************************* */
	if ( mem [ curp ] .hh.b1 == 1 ) /*  change in 3.14159 */
/* ******************************************************************* */
	{
	  if ( ! ( mem [ curp ] .hh .v.RH >= himemmin ) && autobreaking ) 
	  if ( mem [ mem [ curp ] .hh .v.RH ] .hh.b0 == 10 ) 
	  trybreak ( 0 , 0 ) ; 
	  activewidth [ 1 ] = activewidth [ 1 ] + mem [ curp + 1 ] .cint ; 
	} 
/* ******************************************************************* */
/*  change in 3.14159 */
	else activewidth [ 1 ] = activewidth [ 1 ] + mem [ curp + 1 ] .cint ; 
/* ******************************************************************* */
	break ; 
      case 6 : 
	{
	  f = mem [ curp + 1 ] .hh.b0 ; 
	  activewidth [ 1 ] = activewidth [ 1 ] + fontinfo [ widthbase [ f ] + 
	  fontinfo [ charbase [ f ] + mem [ curp + 1 ] .hh.b1 ] .qqqq .b0 ] 
	  .cint ; 
	} 
	break ; 
      case 7 : 
	{
	  s = mem [ curp + 1 ] .hh .v.LH ; 
	  discwidth = 0 ; 
	  if ( s == 0 ) 
	  trybreak ( eqtb [ (hash_size + 3167) ] .cint , 1 ) ; 
	  else {
	      
	    do {
		if ( ( s >= himemmin ) ) 
	      {
		f = mem [ s ] .hh.b0 ; 
		discwidth = discwidth + fontinfo [ widthbase [ f ] + fontinfo 
		[ charbase [ f ] + mem [ s ] .hh.b1 ] .qqqq .b0 ] .cint ; 
	      } 
	      else switch ( mem [ s ] .hh.b0 ) 
	      {case 6 : 
		{
		  f = mem [ s + 1 ] .hh.b0 ; 
		  discwidth = discwidth + fontinfo [ widthbase [ f ] + 
		  fontinfo [ charbase [ f ] + mem [ s + 1 ] .hh.b1 ] .qqqq .b0 
		  ] .cint ; 
		} 
		break ; 
	      case 0 : 
	      case 1 : 
	      case 2 : 
	      case 11 : 
		discwidth = discwidth + mem [ s + 1 ] .cint ; 
		break ; 
		default: 
			{
				confusion ( 931 ) ;		/* disc3 */
				return;				// abortflag set
			}
			break ; 
	      } 
	      s = mem [ s ] .hh .v.RH ; 
	    } while ( ! ( s == 0 ) ) ; 
	    activewidth [ 1 ] = activewidth [ 1 ] + discwidth ; 
	    trybreak ( eqtb [ (hash_size + 3166) ] .cint , 1 ) ; 
	    activewidth [ 1 ] = activewidth [ 1 ] - discwidth ; 
	  } 
	  r = mem [ curp ] .hh.b1 ; 
	  s = mem [ curp ] .hh .v.RH ; 
	  while ( r > 0 ) {
	    if ( ( s >= himemmin ) ) 
	    {
	      f = mem [ s ] .hh.b0 ; 
	      activewidth [ 1 ] = activewidth [ 1 ] + fontinfo [ widthbase [ f 
	      ] + fontinfo [ charbase [ f ] + mem [ s ] .hh.b1 ] .qqqq .b0 ] 
	      .cint ; 
	    } 
	    else switch ( mem [ s ] .hh.b0 ) 
	    {case 6 : 
	      {
		f = mem [ s + 1 ] .hh.b0 ; 
		activewidth [ 1 ] = activewidth [ 1 ] + fontinfo [ widthbase [ 
		f ] + fontinfo [ charbase [ f ] + mem [ s + 1 ] .hh.b1 ] .qqqq 
		.b0 ] .cint ; 
	      } 
	      break ; 
	    case 0 : 
	    case 1 : 
	    case 2 : 
	    case 11 : 
	      activewidth [ 1 ] = activewidth [ 1 ] + mem [ s + 1 ] .cint ; 
	      break ; 
	      default:
			  {
				  confusion ( 932 ) ;	/* disc4 */
				  return;				// abortflag set
			  }
			  break ; 
	    } 
	    decr ( r ) ; 
	    s = mem [ s ] .hh .v.RH ; 
	  } 
	  prevp = curp ; 
	  curp = s ; 
	  goto lab35 ; 
	} 
	break ; 
      case 9 : 
	{
	  autobreaking = ( mem [ curp ] .hh.b1 == 1 ) ; 
	  {
	    if ( ! ( mem [ curp ] .hh .v.RH >= himemmin ) && autobreaking ) 
	    if ( mem [ mem [ curp ] .hh .v.RH ] .hh.b0 == 10 ) 
	    trybreak ( 0 , 0 ) ; 
	    activewidth [ 1 ] = activewidth [ 1 ] + mem [ curp + 1 ] .cint ; 
	  } 
	} 
	break ; 
      case 12 : 
	trybreak ( mem [ curp + 1 ] .cint , 0 ) ; 
	break ; 
      case 4 : 
      case 3 : 
      case 5 : 
	; 
	break ; 
	default: 
		{
			confusion ( 930 ) ;		/* paragraph */
			return;				// abortflag set
		}
		break ; 
      } 
      prevp = curp ; 
      curp = mem [ curp ] .hh .v.RH ; 
      lab35: ; 
    } 
    if ( curp == 0 ) 
    {
      trybreak ( -10000 , 1 ) ; 
      if ( mem [ memtop - 7 ] .hh .v.RH != memtop - 7 ) 
      {
	r = mem [ memtop - 7 ] .hh .v.RH ; 
	fewestdemerits = 1073741823L ; /* 2^30 - 1 */
	do {
	    if ( mem [ r ] .hh.b0 != 2 ) 
	  if ( mem [ r + 2 ] .cint < fewestdemerits ) 
	  {
	    fewestdemerits = mem [ r + 2 ] .cint ; 
	    bestbet = r ; 
	  } 
	  r = mem [ r ] .hh .v.RH ; 
	} while ( ! ( r == memtop - 7 ) ) ; 
	bestline = mem [ bestbet + 1 ] .hh .v.LH ; 
/*  if looseness=0 then goto done; */
	if ( eqtb [ (hash_size + 3182) ] .cint == 0 ) {
/*		if (finalpass && eqtb [ (hash_size + 3750) ] .cint > 0 ) { */
/*			paragraphfailed++; */
/*		} */
		goto lab30 ;						/* normal exit */
	}
	{
/* r:=link(active); actual_looseness:=0; */
	  r = mem [ memtop - 7 ] .hh .v.RH ; 
	  actuallooseness = 0 ; 
	  do {
	      if ( mem [ r ] .hh.b0 != 2 ) 
	    {
/*   line_diff:=line_number(r)-best_line; */
	      linediff = toint ( mem [ r + 1 ] .hh .v.LH ) - toint ( bestline 
	      ) ; 
/*   if ((line_diff<actual_looseness)and(looseness<=line_diff))or@|
        ((line_diff>actual_looseness)and(looseness>=line_diff)) then */
	      if ( ( ( linediff < actuallooseness ) &&
				( eqtb [ (hash_size + 3182) ] .cint 
	      <= linediff ) ) || ( ( linediff > actuallooseness ) &&
				( eqtb [ (hash_size + 3182) ] .cint >= linediff ) ) ) 
	      {
		bestbet = r ; 
		actuallooseness = linediff ; 
		fewestdemerits = mem [ r + 2 ] .cint ; 
	      } 
	      else if ( ( linediff == actuallooseness ) && ( mem [ r + 2 ] 
	      .cint < fewestdemerits ) ) 
	      {
		bestbet = r ; 
		fewestdemerits = mem [ r + 2 ] .cint ; 
	      } 
	    } 
	    r = mem [ r ] .hh .v.RH ; 
	  } while ( ! ( r == memtop - 7 ) ) ; 
	  bestline = mem [ bestbet + 1 ] .hh .v.LH ; 
	} 
/*  if (actual_looseness=looseness)or final_pass then goto done; */
/*	if ( ( actuallooseness == eqtb [ (hash_size + 3182) ] .cint ) || finalpass ) */
	if ( ( actuallooseness == eqtb [ (hash_size + 3182) ] .cint ) ) {
		goto lab30 ;
	}
	if ( finalpass ) {
		goto lab30 ;
	}
      } 
    } 
    q = mem [ memtop - 7 ] .hh .v.RH ; 
    while ( q != memtop - 7 ) {
      curp = mem [ q ] .hh .v.RH ; 
      if ( mem [ q ] .hh.b0 == 2 ) 
      freenode ( q , 7 ) ; 
      else freenode ( q , 3 ) ; 
      q = curp ; 
    } 
    q = passive ; 
    while ( q != 0 ) {
      curp = mem [ q ] .hh .v.RH ; 
      freenode ( q , 2 ) ; 
      q = curp ; 
    } 
    if ( ! secondpass ) 
    {
	;
#ifdef STAT
      if ( eqtb [ (hash_size + 3195) ] .cint > 0 ) 
      printnl ( 928 ) ;			/* @secondpass */
#endif /* STAT */
/* threshold:=tolerance; */
      threshold = eqtb [ (hash_size + 3164) ] .cint ; 
      secondpass = true ; 
	  secondpasscount++;					/* 96 Feb 9 */
/*   final_pass:=(emergency_stretch<=0); */
      finalpass = ( eqtb [ (hash_size + 3750) ] .cint <= 0 ) ; 
    } 
    else {
	
	;
#ifdef STAT
      if ( eqtb [ (hash_size + 3195) ] .cint > 0 )	/* tracing_paragraphs */
      printnl ( 929 ) ;			/* @emergencypass */
#endif /* STAT */
/*     can only get here is \emergencystretch has been set positive */
/*     background[2]:=background[2]+emergency_stretch; final_pass:=true; */
      background [ 2 ] = background [ 2 ] + eqtb [ (hash_size + 3750) ] .cint ;
	  finalpass = true ; 
	  finalpasscount++;					/* 96 Feb 9 */
	} /* end of if secondpass */
  } /* end of while ( true ) do */
/* cannot drop through from above loop */
  lab30:								/* common exit point */
/*  if (badness > (pre)tolerance) */
/*    if ( lastbadness > threshold ) { */
/*    if ( savedbadness > threshold ) { */	/* do we believe this ??? */ 
/*		paragraphfailed++;	*/	/* 96/Feb/9 */
/*	} */
/*	  if (prevgraf == 1) singleline++; */
/*	  if (nest [ nestptr ] .pgfield == 1) singleline++; */
/*	  At this point bestline is number of lines in paragraph + 1 */
	  if (bestline == 2) singleline++;
	;
#ifdef STAT
  if ( eqtb [ (hash_size + 3195) ] .cint > 0 ) 
  {
    enddiagnostic ( true ) ; 
    normalizeselector () ; 
  } 
#endif /* STAT */
  postlinebreak ( finalwidowpenalty ) ; 
  q = mem [ memtop - 7 ] .hh .v.RH ; 
  while ( q != memtop - 7 ) {
    curp = mem [ q ] .hh .v.RH ; 
    if ( mem [ q ] .hh.b0 == 2 ) 
    freenode ( q , 7 ) ; 
    else freenode ( q , 3 ) ; 
    q = curp ; 
  } 
  q = passive ; 
  while ( q != 0 ) { /* while q<>null do l.16979 */
    curp = mem [ q ] .hh .v.RH ; 
    freenode ( q , 2 ) ; 
    q = curp ; 
  } 
  packbeginline = 0 ; 
} 

void prefixedcommand (void) 
{/* 30 10 */ prefixedcommand_regmem 
  smallnumber a  ; 
  internalfontnumber f  ; 
  halfword j  ; 
  fontindex k  ; 
  halfword p, q  ; 
  integer n  ; 
  booleane e  ; 
  a = 0 ; 
  while ( curcmd == 93 ) {
    if ( ! odd ( a / curchr ) ) 
    a = a + curchr ;			/*   smallnumber a  ;  */
    do {
	getxtoken () ; 
	ABORTCHECK;
    } while ( ! ( ( curcmd != 10 ) && ( curcmd != 0 ) ) ) ; 
    if ( curcmd <= 70 ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 1173 ) ;	/* You can't use a prefix with ` */
      } 
      printcmdchr ( curcmd , curchr ) ; 
      printchar ( 39 ) ;	/* ' */
      {
	helpptr = 1 ; 
	helpline [ 0 ] = 1174 ;	/* I'll pretend you didn't say \long or \outer or \global. */
      } 
      backerror () ; 
//	  ABORTCHECK;
      return ; 
    } 
  } 
  if ( ( curcmd != 97 ) && ( a % 4 != 0 ) ) {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* ! */
      print ( 682 ) ;		/* You can't use ` */
    } 
    printesc ( 1165 ) ;		/* long */
    print ( 1175 ) ;		/* ' or ` */
    printesc ( 1166 ) ;		/* outer */
    print ( 1176 ) ;		/* ' with ` */
    printcmdchr ( curcmd , curchr ) ; 
    printchar ( 39 ) ;		/* ' */
    {
      helpptr = 1 ; 
      helpline [ 0 ] = 1177 ;	/* I'll pretend you didn't say \long or \outer here. */
    } 
    error () ; 
	ABORTCHECK;
  } 
  if ( eqtb [ (hash_size + 3206) ] .cint != 0 ) 
  if ( eqtb [ (hash_size + 3206) ] .cint < 0 ) 
  {
    if ( ( a >= 4 ) ) 
    a = a - 4 ; 
  } 
  else {
      
    if ( ! ( a >= 4 ) ) 
    a = a + 4 ; 
  } 

  ABORTCHECK;

  switch ( curcmd ) {
	  case 87 : 
/* set_font: define(cur_font_loc,data,cur_chr); */
    if ( ( a >= 4 ) ) 
    geqdefine ( (hash_size + 1834) , 120 , curchr ) ; 
    else eqdefine ( (hash_size + 1834) , 120 , curchr ) ; 
    break ; 
  case 97 : 
    {
      if ( odd ( curchr ) && ! ( a >= 4 ) && 
		   ( eqtb [ (hash_size + 3206) ] .cint >= 0 ) ) 
      a = a + 4 ; 
      e = ( curchr >= 2 ) ; 
      getrtoken () ; 
      p = curcs ; 
      q = scantoks ( true , e ) ; 
	  ABORTCHECK;
      if ( ( a >= 4 ) ) geqdefine ( p , 111 + ( a % 4 ) , defref ) ; 
      else eqdefine ( p , 111 + ( a % 4 ) , defref ) ; 
    } 
    break ; 
  case 94 : 
    {
      n = curchr ; 
      getrtoken () ; 
      p = curcs ; 
      if ( n == 0 ) 
      {
	do {
	    gettoken () ; 
	} while ( ! ( curcmd != 10 ) ) ; 
	if ( curtok == 3133 ) 
	{
	  gettoken () ; 
	  if ( curcmd == 10 ) 
	  gettoken () ; 
	} 
      } 
      else {
	  
	gettoken () ; 
	q = curtok ; 
	gettoken () ; 
	backinput () ; 
	curtok = q ; 
	backinput () ; 
      } 
      if ( curcmd >= 111 ) 
      incr ( mem [ curchr ] .hh .v.LH ) ; 
      if ( ( a >= 4 ) ) 
      geqdefine ( p , curcmd , curchr ) ; 
      else eqdefine ( p , curcmd , curchr ) ; 
    } 
    break ; 
  case 95 : 
    {
      n = curchr ; 
      getrtoken () ; 
      p = curcs ; 
      if ( ( a >= 4 ) )  geqdefine ( p , 0 , 256 ) ; 
      else eqdefine ( p , 0 , 256 ) ; 
      scanoptionalequals () ; 
	  ABORTCHECK;
      switch ( n ) {
		  case 0 : 
	{
	  scancharnum () ; 
	  ABORTCHECK;
	  if ( ( a >= 4 ) ) 
	  geqdefine ( p , 68 , curval ) ; 
	  else eqdefine ( p , 68 , curval ) ; 
	} 
	break ; 
      case 1 : 
	{
	  scanfifteenbitint () ; 
	  ABORTCHECK;
	  if ( ( a >= 4 ) ) 
	  geqdefine ( p , 69 , curval ) ; 
	  else eqdefine ( p , 69 , curval ) ; 
	} 
	break ; 
	default: 
	{
	  scaneightbitint () ; 
	  ABORTCHECK;
	  switch ( n ) {
		  case 2 : 
	    if ( ( a >= 4 ) ) 
	    geqdefine ( p , 73 , (hash_size + 3218) + curval ) ; 
	    else eqdefine ( p , 73 , (hash_size + 3218) + curval ) ; 
	    break ; 
	  case 3 : 
	    if ( ( a >= 4 ) ) 
	    geqdefine ( p , 74 , (hash_size + 3751) + curval ) ; 
	    else eqdefine ( p , 74 , (hash_size + 3751) + curval ) ; 
	    break ; 
	  case 4 : 
	    if ( ( a >= 4 ) ) 
	    geqdefine ( p , 75 , (hash_size + 800) + curval ) ; 
	    else eqdefine ( p , 75 , (hash_size + 800) + curval ) ; 
	    break ; 
	  case 5 : 
	    if ( ( a >= 4 ) ) 
	    geqdefine ( p , 76 , (hash_size + 1056) + curval ) ; 
	    else eqdefine ( p , 76 , (hash_size + 1056) + curval ) ; 
	    break ; 
	  case 6 : 
	    if ( ( a >= 4 ) ) 
	    geqdefine ( p , 72 , (hash_size + 1322) + curval ) ; 
	    else eqdefine ( p , 72 , (hash_size + 1322) + curval ) ; 
	    break ; 
	  } 
	} 
	break ; 
      } 
    } 
    break ; 
  case 96 : 
    {
      scanint () ; 
	  ABORTCHECK;
      n = curval ; 
      if ( ! scankeyword ( 836 ) )	/* to */
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* ! */
	  print ( 1067 ) ;		/* Missing `to' inserted */
	} 
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1194 ;	/* You should have said `\read<number> to \cs'. */
	  helpline [ 0 ] = 1195 ;	/* I'm going to look for the \cs now. */
	} 
	error () ; 
	ABORTCHECK;
      } 
      getrtoken () ; 
      p = curcs ; 
	  readtoks ( n , p ) ; 
	  ABORTCHECK;
      if ( ( a >= 4 ) ) geqdefine ( p , 111 , curval ) ; 
      else eqdefine ( p , 111 , curval ) ; 
    } 
    break ; 
  case 71 : 
  case 72 : 
    {
      q = curcs ; 
      if ( curcmd == 71 ) 
      {
	scaneightbitint () ; 
	ABORTCHECK;
	p = (hash_size + 1322) + curval ; 
      } 
      else p = curchr ; 
      scanoptionalequals () ; 
      do {
	  getxtoken () ; 
	  ABORTCHECK;
      } while ( ! ( ( curcmd != 10 ) && ( curcmd != 0 ) ) ) ; 
      if ( curcmd != 1 ) 
      {
	if ( curcmd == 71 ) 
	{
	  scaneightbitint () ; 
	  ABORTCHECK;
	  curcmd = 72 ; 
	  curchr = (hash_size + 1322) + curval ; 
	} 
	if ( curcmd == 72 ) 
	{
	  q = eqtb [ curchr ] .hh .v.RH ; 
	  if ( q == 0 ) 
	  if ( ( a >= 4 ) ) 
	  geqdefine ( p , 101 , 0 ) ; 
	  else eqdefine ( p , 101 , 0 ) ; 
	  else {
	      
	    incr ( mem [ q ] .hh .v.LH ) ; 
	    if ( ( a >= 4 ) ) 
	    geqdefine ( p , 111 , q ) ; 
	    else eqdefine ( p , 111 , q ) ; 
	  } 
	  goto lab30 ; 
	} 
      } 
      backinput () ; 
      curcs = q ; 
      q = scantoks ( false , false ) ; 
	  ABORTCHECK;
      if ( mem [ defref ] .hh .v.RH == 0 ) {
	if ( ( a >= 4 ) ) 
	geqdefine ( p , 101 , 0 ) ; 
	else eqdefine ( p , 101 , 0 ) ; 
	{
	  mem [ defref ] .hh .v.RH = avail ; 
	  avail = defref ; 
	;
#ifdef STAT
	  decr ( dynused ) ; 
#endif /* STAT */
	} 
      } 
      else {
	  
	if ( p == (hash_size + 1313) )			/* output ? */
	{
	  mem [ q ] .hh .v.RH = getavail () ; 
	  q = mem [ q ] .hh .v.RH ; 
	  mem [ q ] .hh .v.LH = 637 ; 
	  q = getavail () ; 
	  mem [ q ] .hh .v.LH = 379 ;
	  mem [ q ] .hh .v.RH = mem [ defref ] .hh .v.RH ; 
	  mem [ defref ] .hh .v.RH = q ; 
	} 
	if ( ( a >= 4 ) ) 
	geqdefine ( p , 111 , defref ) ; 
	else eqdefine ( p , 111 , defref ) ; 
      } 
    } 
    break ; 
  case 73 : 
    {
      p = curchr ; 
      scanoptionalequals () ; 
      scanint () ; 
	  ABORTCHECK;
      if ( ( a >= 4 ) ) 
      geqworddefine ( p , curval ) ; 
      else eqworddefine ( p , curval ) ; 
    } 
    break ; 
  case 74 : 
    {
      p = curchr ; 
      scanoptionalequals () ; 
      scandimen ( false , false , false ) ; 
	  ABORTCHECK;
      if ( ( a >= 4 ) ) 
      geqworddefine ( p , curval ) ; 
      else eqworddefine ( p , curval ) ; 
    } 
    break ; 
  case 75 : 
  case 76 : 
    {
      p = curchr ; 
      n = curcmd ; 
      scanoptionalequals () ; 
      if ( n == 76 )  scanglue ( 3 ) ;
      else scanglue ( 2 ) ;
	  ABORTCHECK;
      trapzeroglue () ; 
      if ( ( a >= 4 ) ) 
      geqdefine ( p , 117 , curval ) ; 
      else eqdefine ( p , 117 , curval ) ; 
    } 
    break ; 
  case 85 : 
    {
      if ( curchr == (hash_size + 1883) ) 
      n = 15 ; 
      else if ( curchr == (hash_size + 2907) ) 
      n = 32768L ; 								/* 2^15 */
      else if ( curchr == (hash_size + 2651) ) 
      n = 32767 ; 								/* 2^15 - 1*/
      else if ( curchr == (hash_size + 3474) ) 
      n = 16777215L ; 							/* 2^24 - 1 */
      else n = 255 ;							/* 2^8 - 1 */
      p = curchr ; 
      scancharnum () ; 
	  ABORTCHECK;
      p = p + curval ; 
      scanoptionalequals () ; 
      scanint () ; 
	  ABORTCHECK;
      if ( ( ( curval < 0 ) && ( p < (hash_size + 3474) ) ) || ( curval > n ) ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* ! */
	  print ( 1196 ) ;		/* Invalid code ( */
	} 
	printint ( curval ) ; 
	if ( p < (hash_size + 3474) ) 
		print ( 1197 ) ;	/* ), should be in the range 0.. */
	else print ( 1198 ) ;	/* ), should be at most  */
	printint ( n ) ; 
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 1199 ;	/* I'm going to use 0 instead of that illegal code value. */
	} 
	error () ; 
	ABORTCHECK;
	curval = 0 ; 
      } 
      if ( p < (hash_size + 2907) ) 
      if ( ( a >= 4 ) ) 
      geqdefine ( p , 120 , curval ) ; 
      else eqdefine ( p , 120 , curval ) ; 
      else if ( p < (hash_size + 3474) ) 
      if ( ( a >= 4 ) ) 
      geqdefine ( p , 120 , curval ) ; 
      else eqdefine ( p , 120 , curval ) ; 
      else if ( ( a >= 4 ) ) 
      geqworddefine ( p , curval ) ; 
      else eqworddefine ( p , curval ) ; 
    } 
    break ; 
  case 86 : 
    {
      p = curchr ; 
      scanfourbitint () ; 
	  ABORTCHECK;
      p = p + curval ; 
      scanoptionalequals () ; 
      scanfontident () ; 
	  ABORTCHECK;
      if ( ( a >= 4 ) ) 
      geqdefine ( p , 120 , curval ) ; 
      else eqdefine ( p , 120 , curval ) ; 
    } 
    break ; 
  case 89 : 
  case 90 : 
  case 91 : 
  case 92 : 
    doregistercommand ( a ) ; 
	ABORTCHECK;
    break ; 
  case 98 : 
    {
      scaneightbitint () ; 
	  ABORTCHECK;
      if ( ( a >= 4 ) ) 
      n = 256 + curval ; 
      else n = curval ; 
      scanoptionalequals () ; 
      if ( setboxallowed ) {
		  scanbox ( 1073741824L + n ) ; /* 2^30 + n */ /* box_flag + n */
		  ABORTCHECK;
	  }
      else {
	  
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* ! */
	  print ( 677 ) ;		/* Improper  */
	} 
	printesc ( 533 ) ;		/* setbox */
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1205 ;	/* Sorry, \setbox is not allowed after \halign in a display, */
	  helpline [ 0 ] = 1206 ;	/* or between \accent and an accented character. */
	} 
	error () ; 
	ABORTCHECK;
      } 
    } 
    break ; 
  case 79 : 
    alteraux () ; 
	ABORTCHECK;
    break ; 
  case 80 : 
    alterprevgraf () ; 
	ABORTCHECK;
    break ; 
  case 81 : 
    alterpagesofar () ; 
	ABORTCHECK;
    break ; 
  case 82 : 
    alterinteger () ; 
	ABORTCHECK;
    break ; 
  case 83 : 
    alterboxdimen () ; 
	ABORTCHECK;
    break ; 
  case 84 : 
    {
      scanoptionalequals () ; 
      scanint () ; 
	  ABORTCHECK;
      n = curval ; 
      if ( n <= 0 ) 
      p = 0 ; 
      else {
	  
	p = getnode ( 2 * n + 1 ) ; 
	mem [ p ] .hh .v.LH = n ; 
	{
		register integer for_end;
		j = 1 ;
		for_end = n ;
		if ( j <= for_end) do 
		{
			scandimen ( false , false , false ) ; 
			ABORTCHECK;
			mem [ p + 2 * j - 1 ] .cint = curval ; 
			scandimen ( false , false , false ) ; 
			ABORTCHECK;
			mem [ p + 2 * j ] .cint = curval ; 
		} 
		while ( j++ < for_end ) ;
	} 
      } 
      if ( ( a >= 4 ) ) 
      geqdefine ( (hash_size + 1312) , 118 , p ) ; 
      else eqdefine ( (hash_size + 1312) , 118 , p ) ; 
    } 
    break ; 
  case 99 : 
    if ( curchr == 1 ) 
    {
	;
#ifdef INITEX
  if (is_initex) {		/* bkph */
      newpatterns () ; 
      goto lab30 ; 
  }						/* bkph */
#endif /* INITEX */
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* ! */
	print ( 1210 ) ;	/* Patterns can be loaded only by INITEX */
      } 
      helpptr = 0 ; 
      error () ; 
	  ABORTCHECK;
      do {
		  gettoken () ; 
      } while ( ! ( curcmd == 2 ) ) ; 
      return ; 
    } 
    else {
	
      newhyphexceptions () ; 
	  ABORTCHECK;
      goto lab30 ; 
    } 
    break ; 
  case 77 : 
    {
      findfontdimen ( true ) ; 
      k = curval ; 
      scanoptionalequals () ; 
      scandimen ( false , false , false ) ; 
	  ABORTCHECK;
      fontinfo [ k ] .cint = curval ; 
    } 
    break ; 
  case 78 : 
    {
      n = curchr ; 
      scanfontident () ; 
	  ABORTCHECK;
      f = curval ; 
      scanoptionalequals () ; 
      scanint () ; 
	  ABORTCHECK;
      if ( n == 0 ) hyphenchar [ f ] = curval ; 
      else skewchar [ f ] = curval ; 
    } 
    break ; 
  case 88 : 
    newfont ( a ) ; 
	ABORTCHECK;
    break ; 
  case 100 : 
    newinteraction () ; 
    break ; 
    default: 
	{
		confusion ( 1172 ) ;	/* prefix */
		return;				// abortflag set
	}
	break ; 
  } /* end of curcmd switch */

  ABORTCHECK;

lab30:
  if ( aftertoken != 0 )  {
	  curtok = aftertoken ; 
	  backinput () ; 
	  aftertoken = 0 ; 
  } 
} 

/* added following explanations 96/Jan/10 */

void badformatorpool (char *name, char *defaultname, char *envvar) {
	if (name == NULL) name = defaultname;
	sprintf(logline, "(Perhaps %s is for an older version of TeX)\n", name); 
	showline(logline, 0);
	nameoffile[namelength + 1] = '\0';	/* null terminate */
	sprintf(logline, "(Alternatively, %s may have been corrupted)\n", nameoffile+1);
	showline(logline, 0);
	nameoffile[namelength + 1] = ' ';	/* space terminate */
	sprintf(logline,
		"(Perhaps your %s environment variable is not set correctly)\n", envvar);
	showline(logline, 0);
	{
		char *s;						/* extra info 99/April/28 */
		if ((s = grabenv(envvar)) != NULL) {
			sprintf(logline, "(%s=%s)\n", envvar, s);
			showline(logline, 0);
		}
		else {
			sprintf(logline, "%s environment variable not set\n", envvar);
			showline(logline, 0);
		}
	}
#ifndef _WINDOWS
	fflush ( stdout ) ; 
#endif
} 

booleane loadfmtfile ( ) 
{/* 6666 10 */ register booleane Result; loadfmtfile_regmem 
  integer j, k  ; 
  halfword p, q  ; 
  integer x  ; 

  undumpint ( x ) ;				/* CHECKSUM */
  if ( x != BEGINFMTCHECKSUM )  /* magic FMT file start 4C 20 E6 15 hex */
  goto lab6666 ; 

  undumpint ( x ) ;				/* membot */
/*  if ( x != 0 )  */
  if ( x != membot ) 
  goto lab6666 ; 

  undumpint ( x ) ;				/* memtop */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEMAIN
/*	we already read this once earlier to grab memtop */
	if (traceflag) {
		sprintf(logline, "Read from fmt file memtop = %d TeX words\n", x);
		showline(logline, 0);
	}
/*    allocatemainmemory (x); */	/* allocate main memory at this point */
    mem = allocatemainmemory(x);	/* allocate main memory at this point */
	if (mem == NULL) exit(1);		/* redundant sanity test ! */
	initialize_aux();				/* do `mem' part of initialize */
/*	mem = zmem; */					/* update pointer to main memory */
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( x != memtop ) 
  goto lab6666 ; 

  undumpint ( x ) ;					/* eqtbsize */
  if ( x != (hash_size + 4006) )	/* eqtbsize */
  goto lab6666 ; 
  undumpint ( x ) ;					/* hash_prime */
  if ( x != hash_prime ) 
  goto lab6666 ; 
  undumpint ( x ) ;					/* hyphen_prime */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEHYPHEN
/*  allow format files dumped with arbitrary (prime) hyphenation exceptions */
	reallochyphen (x);				/*	resethyphen(); */
	hyphen_prime = x;
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*  if ( x != 607 ) */
  if ( x != hyphen_prime ) 
  goto lab6666 ; 
  {
    undumpint ( x ) ;				/* poolsize */
    if ( x < 0 ) 
    goto lab6666 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
	if ( x > currentpoolsize) {
		if (traceflag) {
			sprintf(logline, "undump string pool reallocation (%d > %d)\n",
			   x, currentpoolsize);
			showline(logline, 0);
		}
		strpool = reallocstrpool (x - currentpoolsize + incrementpoolsize);
	}
	if ( x > currentpoolsize)						/* 94/Jan/24 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if ( x > poolsize ) 
#endif
    {
      ; 
      sprintf(logline, "%s%s\n",  "---! Must increase the " , "string pool size" ) ; 
	  showline(logline, 0);
      goto lab6666 ; 
    } 
    else poolptr = x ; 
  } 
  {
    undumpint ( x ) ;				/* maxstrings */
    if ( x < 0 ) 
		goto lab6666 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
    if ( x > currentmaxstrings ) {
		if (traceflag) {
			sprintf(logline, "undump string pointer reallocation (%d > %d)\n",
				  x, currentmaxstrings);
			showline(logline, 0);
		}
		strstart = reallocstrstart ( x - currentmaxstrings + incrementmaxstrings);
	}
    if ( x > currentmaxstrings )					/* 94/Jan/24 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if ( x > maxstrings ) 
#endif
    {
      ; 
      sprintf(logline,  "%s%s\n",  "---! Must increase the " , "max strings" ) ; 
	  showline(logline, 0);
      goto lab6666 ; 
    } 
    else strptr = x ; 
  } 

  if (undumpthings ( strstart [ 0 ] , strptr + 1 )  /* undump string ptrs */
	 ) return -1;
  if (undumpthings ( strpool [ 0 ] , poolptr )  /*	undump string pool */
	 ) return -1;

  initstrptr = strptr ; 
  initpoolptr = poolptr ; 
/*	undump the dynamic memory - paragraph 1312 in the book */
  {
    undumpint ( x ) ; 
    if ( ( x < 1019 ) || ( x > memtop - 14 ) ) 
    goto lab6666 ; 
    else lomemmax = x ; 
  } 
  {
    undumpint ( x ) ; 
    if ( ( x < 20 ) || ( x > lomemmax ) ) 
    goto lab6666 ; 
    else rover = x ; 
  } 
  p = 0 ;									/* mem_bot */
  q = rover ; 
  do {
      if (undumpthings ( mem [ p ] , q + 2 - p )  
		 ) return -1;
    p = q + mem [ q ] .hh .v.LH ; 
    if ( ( p > lomemmax ) || ( ( q >= mem [ q + 1 ] .hh .v.RH ) && ( mem [ q + 
    1 ] .hh .v.RH != rover ) ) ) 
    goto lab6666 ; 
    q = mem [ q + 1 ] .hh .v.RH ; 
  } while ( ! ( q == rover ) ) ; 
  if (undumpthings ( mem [ p ] , lomemmax + 1 - p )  
	 ) return -1;
/*  if ( memmin < -2 )  */
  if ( memmin < membot - 2 )					/*  ? splice in block below */
  {
/*	or call addvariablespace(membot - (memmin + 1)) */

    if (traceflag) showline("Splicing in memmin space in undump!\n", 0);

    p = mem [ rover + 1 ] .hh .v.LH ; 
    q = memmin + 1 ; 
    mem [ memmin ] .hh .v.RH = 0 ;				/* null */
    mem [ memmin ] .hh .v.LH = 0 ;				/* null */
    mem [ p + 1 ] .hh .v.RH = q ; 
    mem [ rover + 1 ] .hh .v.LH = q ; 
    mem [ q + 1 ] .hh .v.RH = rover ; 
    mem [ q + 1 ] .hh .v.LH = p ; 
/*    mem [ q ] .hh .v.RH = 262143L ;  */	/* NO! */
    mem [ q ] .hh .v.RH = emptyflag ; 
/*    mem [ q ] .hh .v.LH = -0 - q ;  */
    mem [ q ] .hh .v.LH = membot - q ; 		/* ? size of block  */
  } 
  {
    undumpint ( x ) ; 
    if ( ( x < lomemmax + 1 ) || ( x > memtop - 13 ) ) 
    goto lab6666 ; 
    else himemmin = x ; 
  } 
  {
    undumpint ( x ) ; 
/*    if ( ( x < 0 ) || ( x > memtop ) )  */
    if ( ( x < membot ) || ( x > memtop ) ) 
    goto lab6666 ; 
    else avail = x ; 
  } 
  memend = memtop ; 
  if (undumpthings ( mem [ himemmin ] , memend + 1 - himemmin )  
	 ) return -1;
  undumpint ( varused ) ; 
  undumpint ( dynused ) ; 
  k = 1 ; 
  do {
      undumpint ( x ) ; 
    if ( ( x < 1 ) || ( k + x > (hash_size + 4007) ) ) 
    goto lab6666 ; 
    if (undumpthings ( eqtb [ k ] , x )  
	   ) return -1;
    k = k + x ; 
    undumpint ( x ) ; 
    if ( ( x < 0 ) || ( k + x > (hash_size + 4007) ) ) 
    goto lab6666 ; 
    {
		register integer for_end;
		j = k ;
		for_end = k + x - 1 ;
		if ( j <= for_end) do 
			eqtb [ j ] = eqtb [ k - 1 ] ; 
		while ( j++ < for_end ) ;
	} 
    k = k + x ; 
  } while ( ! ( k > (hash_size + 4006) ) ) ; 
  {
    undumpint ( x ) ; 
/*	if ( ( x < hashbase ) || ( x > hashbase + hashsize ) ) */
/*    if ( ( x < 514 ) || ( x > (hash_size + 514) ) )  */
    if ( ( x < 514 ) || ( x > (hash_size + hash_extra + 514) ) )  /*96/Jan/10*/
    goto lab6666 ; 
    else parloc = x ; 
  } 
  partoken = 4095 + parloc ; 
  {
    undumpint ( x ) ; 
/*  if ( ( x < hashbase ) || ( x > hashbase + hashsize ) ) */
/*    if ( ( x < 514 ) || ( x > (hash_size + 514) ) )  */
    if ( ( x < 514 ) || ( x > (hash_size + hash_extra + 514) ) ) /*96/Jan/10*/
    goto lab6666 ; 
    else
    writeloc = x ; 
  } 
  {
    undumpint ( x ) ; 
/*  if ( ( x < hashbase ) || ( x > hashbase + hashsize ) ) */
/*    if ( ( x < 514 ) || ( x > (hash_size + 514) ) )  */
    if ( ( x < 514 ) || ( x > (hash_size + hash_extra + 514) ) ) /*96/Jan/10*/
    goto lab6666 ; 
    else hashused = x ; 
  } 
  p = 513 ; 
  do {
      { 
      undumpint ( x ) ; 
      if ( ( x < p + 1 ) || ( x > hashused ) ) 
      goto lab6666 ; 
      else p = x ; 
    } 
    undumphh ( hash [ p ] ) ; 
  } while ( ! ( p == hashused ) ) ; 
  if (undumpthings ( hash [ hashused + 1 ] , (hash_size + 780) - hashused )  
	 ) return -1;

  undumpint ( cscount ) ;			/* cscount */
  if (traceflag) {
	  sprintf(logline, "itex undump cscount %d ", cscount); /* debugging */
	  showline(logline, 0);
  }
  {
    undumpint ( x ) ;				/* fontmemsize */
    if ( x < 7 ) 
    goto lab6666 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEFONT
	if (traceflag) {
		sprintf(logline, "Read from fmt fmemptr = %d\n", x);
		showline(logline, 0);
	}
    if ( x > currentfontmemsize ) {	/* 93/Nov/28 dynamic allocate fontinfo */
		if (traceflag) {
			sprintf(logline, "Undump realloc fontinfo (%d > %d)\n",
				   x, currentfontmemsize);
			showline(logline, 0);
		}
		fontinfo = reallocfontinfo ( x - currentfontmemsize + incrementfontmemsize );
	}
    if ( x > currentfontmemsize )   /* in case allocation failed 94/Jan/24 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if ( x > fontmemsize ) 
#endif
    {
      ; 
      sprintf(logline, "%s%s\n",  "---! Must increase the " , "font mem size" ) ; 
	  showline(logline, 0);
      goto lab6666 ; 
    } 
    else fmemptr = x ; 
  } 
  {
    if (undumpthings ( fontinfo [ 0 ] , fmemptr )  
	   ) return -1;
    {
      undumpint ( x ) ;			/* fontmax */
      if ( x < 0 ) 
		  goto lab6666 ; 
      if ( x > fontmax ) 
      {
	; 
		sprintf(logline, "%s%s\n",  "---! Must increase the " , "font max" ) ; 
		showline(logline, 0);
		goto lab6666 ; 
      } 
      else fontptr = x ; 
    } 
	frozenfontptr = fontptr;	/* remember number of fonts frozen into format */
    if (undumpthings ( fontcheck [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontsize [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontdsize [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontparams [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( hyphenchar [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( skewchar [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontname [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontarea [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontbc [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontec [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( charbase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( widthbase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( heightbase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( depthbase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( italicbase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( ligkernbase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( kernbase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( extenbase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( parambase [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontglue [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( bcharlabel [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontbchar [ 0 ] , fontptr + 1 )  
	   ) return -1;
    if (undumpthings ( fontfalsebchar [ 0 ] , fontptr + 1 )  
	   ) return -1;
  } 

/*	log not opened yet, so can't show fonts frozen into format */
  
/* May be able to avoid the following since we switched to */
/* non_address from font_mem_size to 0 96/Jan/15 ??? */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEFONT
/* deal with fmt files dumped with *different* fontmemsize 93/Nov/29 */
 { int count = 0, oldfontmemsize = 0;
	  for (x = 0; x <= fontptr; x++) {
		  if(bcharlabel[x] > oldfontmemsize) oldfontmemsize = bcharlabel[x];
	  }
/* somewhat arbitrary sanity check ... */
/*	  if (oldfontmemsize != fontmemsize && oldfontmemsize > fontmax) { */
	  if (oldfontmemsize != non_address && oldfontmemsize > fontmax) {	/* 96/Jan/16 */
		  for (x = 0; x <= fontptr; x++) {
			  if(bcharlabel[x] == oldfontmemsize) {
/*				  bcharlabel[x] = fontmemsize; */
				  bcharlabel[x] = non_address;		/* 96/Jan/16 */
				  count++;
			  }
		  }

		  if (traceflag) {
			  sprintf(logline,
					  "oldfontmemsize is %d --- hit %d times. Using non_address %d\n",
					  oldfontmemsize, count, non_address);
			  showline(logline, 0);
		  }

	  }
 }
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* undump(0)(hyph_size)(hyph_count); */
  {
    undumpint ( x ) ; 
/*    if ( ( x < 0 ) || ( x > 607 ) )  */
    if ( ( x < 0 ) || ( x > hyphen_prime ) ) 
    goto lab6666 ; 
    else hyphcount = x ; 
  } 
/* undump hypenation exception tables p.1325 */
  {
	  register integer for_end;
	  k = 1 ;
	  for_end = hyphcount ;
	  if ( k <= for_end) 
		  do 
		  {
			  {
	undumpint ( x ) ; 
/*	if ( ( x < 0 ) || ( x > 607 ) )  */
	if ( ( x < 0 ) || ( x > hyphen_prime ) ) 
	goto lab6666 ; 
	else j = x ; 
      } 
/*   undump(0)(str_ptr)(hyph_word[j]); */
      {
	undumpint ( x ) ; 
	if ( ( x < 0 ) || ( x > strptr ) ) 
	goto lab6666 ; 
	else hyphword [ j ] = x ; 
      } 
/*   undump(min_halfword)(max_halfword)(hyph_list[j]); */
      {
	undumpint ( x ) ; 
/*	if ( ( x < 0 ) || ( x > 262143L ) ) */
	if ( ( x < 0 ) || ( x > maxhalfword ) )		/* memtop ? no p.1325 */
	goto lab6666 ; 
	else hyphlist [ j ] = x ; 
      } 
    } 
  while ( k++ < for_end ) ;
  } 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEHYPHEN
/* if user specified new hyphen prime - flush existing exception patterns ! */
/* but, we can reclaim the string storage wasted ... */
	if (is_initex) {
		if (newhyphenprime != 0) {
			reallochyphen(newhyphenprime);	/*	resethyphen(); */
			hyphen_prime = newhyphenprime;
		}
	}
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  {
    undumpint ( x ) ; 
    if ( x < 0 ) 
    goto lab6666 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATETRIES
	if (!is_initex) {
		allocatetries(x);	/* allocate only as much as is needed */
/*		triesize = x; */	/* ??? */
	}
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if ( x > triesize ) 
    {
      ; 
      sprintf(logline, "%s%s\n",  "---! Must increase the " , "trie size" ) ; 
	  showline(logline, 0);
      goto lab6666 ; 
    } 
    else j = x ; 
  } 
	;
#ifdef INITEX
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (is_initex)			/* bkph */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  triemax = j ; 
#endif /* INITEX */
  if (undumpthings ( trietrl [ 0 ] , j + 1 )  
	 ) return -1;
  if (undumpthings ( trietro [ 0 ] , j + 1 )  
	 ) return -1;
  if (undumpthings ( trietrc [ 0 ] , j + 1 )  
	 ) return -1;
  {
    undumpint ( x ) ; 
    if ( x < 0 ) 
    goto lab6666 ; 
    if ( x > trieopsize ) 
    {
      ; 
      sprintf(logline, "%s%s\n",  "---! Must increase the " , "trie op size" ) ; 
	  showline(logline, 0);
      goto lab6666 ; 
    } 
    else j = x ; 
  } 
	;
#ifdef INITEX
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (is_initex)				/* bkph */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  trieopptr = j ; 
#endif /* INITEX */
/* for k:=1 to j do
  begin undump(0)(63)(hyf_distance[k]); {a |small_number|}
  undump(0)(63)(hyf_num[k]);
  undump(min_quarterword)(max_quarterword)(hyf_next[k]); end; */
  if (undumpthings ( hyfdistance [ 1 ] , j )  
	 ) return -1;
  if (undumpthings ( hyfnum [ 1 ] , j )  
	 ) return -1;
  if (undumpthings ( hyfnext [ 1 ] , j )  
	 ) return -1;
	;
#ifdef INITEX
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (is_initex) {					/* bkph */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  {
	  register integer for_end;
	  k = 0 ;
	  for_end = 255 ;
	  if ( k <= for_end) do 
		  trieused [ k ] = 0 ; 
	  while ( k++ < for_end ) ;
  } 
 }
#endif /* INITEX */
  k = 256 ; 
  while ( j > 0 ) {
/* undump(0)(k-1)(k) */      
    {
      undumpint ( x ) ; 
      if ( ( x < 0 ) || ( x > k - 1 ) ) 
      goto lab6666 ; 
      else k = x ; 
    } 
/* undump(1)(j)(x) */
    {
      undumpint ( x ) ; 
      if ( ( x < 1 ) || ( x > j ) ) 
      goto lab6666 ; 
      else x = x ; 
    } 
	;
#ifdef INITEX
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (is_initex)					/* bkph */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    trieused [ k ] = x ; 
#endif /* INITEX */
/*   j:=j-x; op_start[k]:=qo(j); */
    j = j - x ; 
    opstart [ k ] = j ; 
  } 
	;
#ifdef INITEX
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (is_initex)					/* bkph */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  trienotready = false 
#endif /* INITEX */
  ; 
  {
    undumpint ( x ) ; 
    if ( ( x < 0 ) || ( x > 3 ) ) 
    goto lab6666 ; 
/*    else interaction = x ;  */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if (interaction < 0)		/* may now set in local.c bkph 94/Jan/8 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
		interaction = x ;
  } 
  {
    undumpint ( x ) ; 
    if ( ( x < 0 ) || ( x > strptr ) ) 
    goto lab6666 ; 
    else formatident = x ; 
  } 
  undumpint ( x ) ; 
/*	test_eof ( fmtfile ) ? */
  if ( ( x != ENDFMTCHECKSUM ) || feof ( fmtfile ) ) /* magic checksum --- change ? */
  goto lab6666 ; 

  Result = true ; 
  return(Result) ; 

  lab6666: ; 
  sprintf(logline, "(Fatal format file error; I'm stymied)\n" ) ; 
  showline(logline, 1);
/* added following bit of explanation 96/Jan/10 */
  if (! knuthflag)
	  badformatorpool(formatfile, "the format file", "TEXFORMATS");
  Result = false ; 
  return Result ; 
} 

void finalcleanup (void) 
{/* 10 */ finalcleanup_regmem 
  smallnumber c  ; 
  c = curchr ;					/* smallnumber  c */
  if ( jobname == 0 ) openlogfile () ; 
/* ******************************** 3.14159 ***************************** */
  while ( inputptr > 0 )
	  if ( curinput .statefield == 0 ) {
		  endtokenlist () ;
		  ABORTCHECK;
	  }
	  else endfilereading () ; 
/* *********************************************************************** */
  while ( openparens > 0 ) {
	  print ( 1270 ) ;	/*   ) */
	  decr ( openparens ) ; 
  } 
  if ( curlevel > 1 )   {
    printnl ( 40 ) ;		/* ( */
    printesc ( 1271 ) ;		/* end occurred  */
    print ( 1272 ) ;		/* inside a group at level  */
    printint ( curlevel - 1 ) ; 
    printchar ( 41 ) ;		/* ) */
  } 
  while ( condptr != 0 ) {
    printnl ( 40 ) ;		/* ( */
    printesc ( 1271 ) ;		/* end occurred  */
    print ( 1273 ) ;		/* when  */
    printcmdchr ( 105 , curif ) ;	/* i */
    if ( ifline != 0 ) 
    {
      print ( 1274 ) ;		/* on line  */
      printint ( ifline ) ; 
    } 
    print ( 1275 ) ;		/*  was incomplete) */
    ifline = mem [ condptr + 1 ] .cint ; 
    curif = mem [ condptr ] .hh.b1 ; 
/* *********************************************************************** */
    tempptr = condptr ;						/* new in 3.14159 */
    condptr = mem [ condptr ] .hh .v.RH ; 
	freenode ( tempptr , 2 ) ; 				/* new in 3.14159 */
/* *********************************************************************** */
  } 
  if ( history != 0 ) 
  if ( ( ( history == 1 ) || ( interaction < 3 ) ) ) 
  if ( selector == 19 ) 
  {
    selector = 17 ; 
    printnl ( 1276 ) ;	/* 	(see the transcript file for additional information) */
    selector = 19 ; 
  } 
  if ( c == 1 ) {
	;
#ifdef INITEX
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if (is_initex) {					/* bkph */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* *************************** new in 3.14159 ***************************** */
    {
		register integer for_end;
		c = 0 ;
		for_end = 4 ;
		if ( c <= for_end) do 
			if ( curmark [ c ] != 0 ) 
				deletetokenref ( curmark [ c ] ) ; 
		while ( c++ < for_end ) ;
	} 
/* *********************************************************************** */
		storefmtfile () ;	// returns a value ?
	}
#endif /* INITEX */
	if (!is_initex) 			/* 2000/March/3 */
		printnl ( 1277 ) ;		/* 	(\dump is performed only by INITEX) */
  } 
}

// debugging code for checking the string pool

#ifdef CHECKPOOL
int checkpool (char *task) {	
	int c, i, k, n, st, flag, bad=0;

	if (task != NULL) {
		sprintf(logline, "Check on string pool start (%s)\n", task);
		showline(logline, 0);
	}
	for (k = 0; k < 32; k++) {
		if (strstart[k] != 3 * k) {
			sprintf(logline, "k %d strstart[k] %d\n", k, strstart[k]);
			showline(logline, 0);
		}
	}
	for (k = 32; k < 128; k++) {
		if (strstart[k] != 96 + (k - 32)) {
			sprintf(logline, "k %d strstart[k] %d\n", k, strstart[k]);
			showline(logline, 0);
		}
	}
	for (k = 128; k < 256; k++) {
		if (strstart[k] != 194 + 4 * (k - 128) ) {
			sprintf(logline, "k %d strstart[k] %d\n", k, strstart[k]);
			showline(logline, 0);
		}
	}
	if (task != NULL) {
		sprintf(logline, "Check on string pool (%s)\n", task);
		showline(logline, 0);
	}
	for (k = 0; k < strptr; k++) {
		if (strstart[k+1] == 0) break;
		n = strstart[k+1] - strstart[k];
		if (n < 0) break;
		st = strstart[k];
		flag = 0;
		for (i = 0; i < n; i++) {
			if (strpool[st + i] == 0) {
				flag = 1; break;
			}
			if (strpool[st + i] > 255) {
				flag = 1; break;
			}
		}
		if (flag) {
			bad++;
			sprintf(logline, "BAD %d (start at %d): ", k, st);
			showline(logline, 0);
			s = logline;
			for (i = 0; i < n; i++) {
				c = strpool[st + i];
				if (c > 255) {
					sprintf(s, "%d ", c);
					s += strlen(s);
					continue;
				}
				if (c >= 128) {
					c -= 128;
					sprintf(s, "M-");
					s += strlen(s);
				}
				if (c < 32) {
					c += 64;
					sprintf(s, "C-");
					s += strlen(s);
				}
//				putc(c, stdout);
				*s++ = c;		// ???
			}
			*s++ = '\n';
			*s++ = '\0';
			showline(logline, stdout);
		}
	}
	sprintf(logline, "end of check (%s)\n", bad ? "BAD" : "OK");
	showline(logline, 0);
	if (bad) {
		if (task == NULL) return bad;
		else exit(1);
	}
	return bad;
}  /* DEBUGGING ONLY */
#endif

void showfrozen (void) {
	int i, j, n;
	fprintf(logfile, "\n");
	fprintf(logfile, "(%d fonts frozen in format file:\n", fontptr);
/*	ignore font 0 which is nullfont */
/*	for (i = 1; i < fontptr+1; i++) */
	for (i = 1; i <= fontptr; i++) {
		if (i > 1) fprintf(logfile, ", ");
		if ((i % 8) == 0) fprintf(logfile, "\n");
		n = strstart[fontname[i]+1]-strstart[fontname[i]];
		for (j = 0; j < n; j++) {
			putc(strpool[strstart[fontname[i]]+j], logfile);
/*				 strpool[strstart[fontname[i]]+j] = '?'; */
		}
	}
	fprintf(logfile, ") ");
}

/* Main entry point called from texmf.c int main(int ac, char *av[]) */
/* call from main_program in texmf.c */
/* This in turn calls initialize() */

#pragma warning(disable:4127)		// conditional expression is constant

/* void texbody ( )  */
int texbody (void)					/* now returns a value --- bkph */
{texbody_regmem 
  history = 3 ; 

  setpaths ( TEXFORMATPATHBIT + TEXINPUTPATHBIT + TEXPOOLPATHBIT + TFMFILEPATHBIT ) ; 

  if ( readyalready == 314159L ) goto lab1 ; /* magic number */
/*	These tests are almost all compile time tests and so could be eliminated */
  bad = 0 ; 
  if ( ( halferrorline < 30 ) || ( halferrorline > errorline - 15 ) ) 
	  bad = 1 ; 
  if ( maxprintline < 60 ) 	bad = 2 ; 
  if ( dvibufsize % 8 != 0 )  bad = 3 ; 
  if ( 1100 > memtop ) bad = 4 ;		/* not compile time */
/*  if ( hash_prime > hash_size )  */
  if ( hash_prime > (hash_size + hash_extra) )  bad = 5 ; 
  if ( maxinopen >= 128 ) bad = 6 ;		/* p.14 */
  if ( memtop < 267 ) bad = 7 ;		/* where from ? *//* not compile time */
	;
#ifdef INITEX
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (is_initex) {					/* bkph */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  if ( ( memmin != 0 ) || ( memmax != memtop ) )   bad = 10 ;
  }
#endif /* INITEX */
/*  if ( ( memmin > 0 ) || ( memmax < memtop ) )  */
  if ( ( memmin > membot ) || ( memmax < memtop ) )  bad = 10 ;
/*  if ( ( 0 > 0 ) || ( 255 < 127 ) )  */
  if ( ( minquarterword > 0 ) || ( maxquarterword < 255 ) )  bad = 11 ;
/*  if ( ( 0 > 0 ) || ( 262143L < 32767 ) )  */
  if ( ( minhalfword > 0 ) || ( maxhalfword < 32767 ) )  bad = 12 ;
/*  if ( ( 0 < 0 ) || ( 255 > 262143L ) )  */
  if ( ( minquarterword < minhalfword ) || ( maxquarterword > maxhalfword ) ) 
	  bad = 13 ; 
/*  if ( ( memmin < 0 ) || ( memmax >= 262143L ) || ( -0 - memmin > 262144L ) )  */
  if ( ( memmin < minhalfword ) || ( memmax >= maxhalfword ) || ( membot - memmin >= maxhalfword ) ) 
	  bad = 14 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( memmax > memtop + memextrahigh )			/* not compile time */
	  bad = 14;				/* ha ha */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*  if ( ( 0 < 0 ) || ( fontmax > 255 ) ) */
  if ( ( 0 < minquarterword ) || ( fontmax > maxquarterword ) )  /* 93/Dec/3 */
	  bad = 15 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef INCREASEFONTS
/*  if ( fontmax > 512 ) */							/* 93/Dec/3 */
  if ( fontmax > 1024 )								/* 96/Jan/17 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( fontmax > 256 )  
#endif
	  bad = 16 ; 
/*  if ( ( savesize > 262143L ) || ( maxstrings > 262143L ) ) */
  if ( ( savesize > maxhalfword ) || ( maxstrings > maxhalfword ) ) 
	  bad = 17 ; 
/*  if ( bufsize > 262143L ) */
  if ( bufsize > maxhalfword ) bad = 18 ; 
/*  if ( 255 < 255 )  */
  if ( maxquarterword - minquarterword < 255 )  bad = 19 ; 
/* if cs_token_flag + undefined_control_sequence > max_halfword then bad <- 21;*/
/*  if ( (hash_size + 4876) > 262143L ) */
/*  if ( (hash_size + 4876) > maxhalfword )  */
  if ( (hash_size + 4095 + 781) > maxhalfword )  bad = 21 ; 
  if ( formatdefaultlength > PATHMAX )	bad = 31 ; 
/*  if ( 2 * 262143L < memtop - memmin )  */
  if ( maxhalfword < (memtop - memmin) / 2 )  bad = 41 ; 
  if ( bad > 0 )  {
    sprintf(logline,  "%s%s%ld\n",  "Ouch---my internal constants have been clobbered!" ,
			"---case " , (long) bad ) ; 
	showline(logline, 1);
    if (! knuthflag) 
		badformatorpool(formatfile, "the format file", "TEXFORMATS");	/* 96/Jan/17 */
    goto lab9999 ;			// abort
  } 
  initialize () ; 
	;
#ifdef INITEX
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (is_initex) {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  if ( ! getstringsstarted () )  goto lab9999 ;		// abort
	  initprim () ; 
	  initstrptr = strptr ; 
	  initpoolptr = poolptr ; 
	  dateandtime ( eqtb [ (hash_size + 3183) ] .cint ,
			eqtb [ (hash_size + 3184) ] .cint ,
				eqtb [ (hash_size + 3185) ] .cint ,
					eqtb [ (hash_size + 3186) ] .cint ) ; 
  }
#endif /* INITEX */
  readyalready = 314159L ;			/* magic number */

lab1:			/* get here directly if readyalready already set ... */
  selector = 17 ; 
  tally = 0 ; 
  termoffset = 0 ; 
  fileoffset = 0 ; 
  showline(texversion, 0) ; 
  sprintf(logline, " (%s %s)", application, yandyversion);
  showline(logline, 0);
  if ( formatident > 0 ) slowprint ( formatident ) ; 
  println () ; 
#ifndef _WINDOWS
  fflush ( stdout ) ; 
#endif
  jobname = 0 ; 
  nameinprogress = false ; 
  logopened = false ; 
  outputfilename = 0 ; 
  {
    {
      inputptr = 0 ; 
      maxinstack = 0 ; 
      inopen = 0 ; 
	  highinopen = 0 ;	/* maxinopen name already used 1999 Jan 17 */
      openparens = 0 ; 
	  maxopenparens = 0;	/* maxopenparens */
      maxbufstack = 0 ; 
      paramptr = 0 ; 
      maxparamstack = 0 ; 
#ifdef ALLOCATEBUFFER
/*	  first = currentbufsize ; */
	  memset (buffer, 0, currentbufsize);	/* redundant */
#else
/*	  first = bufsize ; */
	  memset (buffer, 0, bufsize);			/* redundant ? */
#endif
/*      do {
		  buffer [ first ] = 0 ; 
		  decr ( first ) ; 
      } while ( ! ( first == 0 ) ) ; */
	  first = 0;							/* 1999/Jan/22 */

      scannerstatus = 0 ; 
      warningindex = 0 ; /* warning_index:=null; l.7068 */
      first = 1 ; 
      curinput .statefield = 33 ; 
      curinput .startfield = 1 ; 
      curinput .indexfield = 0 ; 
      line = 0 ; 
      curinput .namefield = 0 ; 
      forceeof = false ; 
      alignstate = 1000000L ; 
      if ( ! initterminal () ) goto lab9999 ;	// abort
      curinput .limitfield = last ; 
      first = last + 1 ; 
    } 
/*    if ( ( formatident == 0 ) || ( buffer [ curinput .locfield ] == 38 ) ) */
/*		For Windows NT, lets allow + instead of & for format specification */
    if ( ( formatident == 0 ) ||
		( buffer [ curinput .locfield ] == '&' ) ||
		( buffer [ curinput .locfield ] ==  '+' ) ) 
    {
      if ( formatident != 0 ) initialize () ; 
      if ( ! openfmtfile () ) goto lab9999 ; // abort
      if ( ! loadfmtfile () ) {
		  wclose ( fmtfile ) ; 
		  goto lab9999 ;	// abort
      } 
      wclose ( fmtfile ) ; 
      while ( ( curinput .locfield < curinput .limitfield ) &&
			  ( buffer [ curinput .locfield ] == 32 ) )
		  incr ( curinput .locfield ) ; 
    } 
#ifdef CHECKEQTB
	if (debugflag) checkeqtb("after format");	/* debugging 94/Apr/5 */
#endif
    if ( ( eqtb [ (hash_size + 3211) ] .cint < 0 ) || 
		 ( eqtb [ (hash_size + 3211) ] .cint > 255 ) ) 
		decr ( curinput .limitfield ) ; 
/*	long to unsigned char ... */
    else buffer [ curinput .limitfield ] = eqtb [ (hash_size + 3211) ] .cint ; 
    dateandtime ( eqtb [ (hash_size + 3183) ] .cint ,
		eqtb [ (hash_size + 3184) ] .cint ,
			  eqtb [ (hash_size + 3185) ] .cint ,
				  eqtb [ (hash_size + 3186) ] .cint ) ; 
    magicoffset = strstart [ 886 ] - 9 * 16 ;	/* following: */
/*	"0234000122*4000133**3**344*0400400*000000234000111*1111112341011" */
    if ( interaction == 0 )	selector = 16 ; 
    else selector = 17 ; 
     if ( ( curinput .locfield < curinput .limitfield ) &&
		 ( eqtb [ (hash_size + 1883) + 
		   buffer [ curinput .locfield ] ] .hh .v.RH != 0 ) ) 
    startinput () ; 
  }

/*	show font TFMs frozen into format file */
/*	moved here after startinput to ensure the log file is open */
  if (showtfmflag && logopened && fontptr > 0) 		/* 98/Sep/28 */
	  showfrozen();


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  maintime = clock();
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  history = 0 ; 

  if (showcsnames) printcsnames(stdout, 0);		/* 98/Mar/31 */

  maincontrol();			/* read-eval-print loop :-) in tex8.c */

//	what if abortflag is set now ?

  if (showcsnames) printcsnames(stdout, 1);		/* 98/Mar/31 */

//	what if abortflag is set now ?

  finalcleanup();

//  if (abortflag) return -1;

  closefilesandterminate() ; 

  lab9999:
  {
	  int code;
#ifndef _WINDOWS
	  fflush ( stdout ) ; 
#endif
	  readyalready = 0 ; 
	  if ( ( history != 0 ) && ( history != 1 ) ) code = 1;
	  else code = 0;
//	  now return instead of exit to allow cleanup in local.c
	  return code;
//	  uexit ( code ) ;  
  } 
}	/* end of texbody */

#ifdef ALLOCATEMAIN
/* add a block of variable size node space below membot */
void addvariablespace(int size) {	/* used in tex0.c, local.c, itex.c */
	initialize_regmem 
	halfword p ;
	halfword q ;
	integer t ;

	if (memmin == 0) t = memmin;	/* bottom of present block */
	else t = memmin + 1; 
	memmin = t - (size + 1);			/* first word in new block - 1 */
/*	memmin = memstart; */				/* allocate all of it at once */
	if (memmin < memstart) {			/* sanity test */
		if (traceflag) showline("WARNING: memmin < memstart!\n", 0);
		memmin = memstart;
	}
	p = mem [ rover + 1 ] .hh .v.LH ; 
	q = memmin + 1 ; 
	mem [ memmin ] .hh .v.RH = 0 ;	/* insert blank word below ??? */
	mem [ memmin ] .hh .v.LH = 0 ;	/* insert blank word below ??? */
	mem [ p + 1] .hh .v.RH = q;
	mem [ rover + 1 ] .hh .v.LH = q ; 
	mem [ q + 1 ] .hh .v.RH = rover ; 
	mem [ q + 1 ] .hh .v.LH = p ; 
	mem [ q ] .hh .v.RH = emptyflag ; 
	mem [ q ] .hh .v.LH = t - q ;	/* block size */
	rover = q ; 
}
#endif

/*************************************************************************/

/* All ini TeX code is here at end so can use same pragma optimize for it */

/* Ini-TeX code is rarely needed/used so make it small rather than fast */

#pragma optimize("t", off)
/* #pragma optimize("2", off) */
#pragma optimize("s", on)
/* #pragma optimize("1", on) */

#ifdef INITEX
/* split out to allow sharing of code from do_initex and newpattern */
void resettrie (void) {
	integer k;
	{
		register integer for_end;
		k = - (integer) trieopsize ;
		for_end = trieopsize ;
		if ( k <= for_end)
			do trieophash [ k ] = 0 ; 
		while ( k++ < for_end ) ;
	} 
	{
		register integer for_end;
		k = 0 ; for_end = 255 ;
		if ( k <= for_end)
			do   trieused [ k ] = mintrieop ; 
		while ( k++ < for_end ) ;
	} 
	maxopused = mintrieop ; 
	trieopptr = 0 ; 
	trienotready = true ; 
	triel [ 0 ] = 0 ; 
	triec [ 0 ] = 0 ; 
	trieptr = 0 ; 
	trienotready = true ;
}

void resethyphen (void) {	/* borrowed code from initialize() */
	hyphpointer z ;
  {
	  register integer for_end;
	  z = 0 ;
	  for_end = hyphen_prime ;
	  if ( z <= for_end) do 
	  {
		  hyphword [ z ] = 0 ; 
		  hyphlist [ z ] = 0 ; /* hyph_list[z]:=null; l.18131 */
	  } 
	  while ( z++ < for_end ) ;
  } 
  hyphcount = 0 ;
}

void do_initex (void) {	/* split out to allow optimize for space, not time */
	initialize_regmem
/*  integer i  ; */
  integer k  ; 
/*  hyphpointer z  ; */

/* for k:=mem_bot+1 to lo_mem_stat_max do mem[k].sc:=0; p.164 */
  {
	  register integer for_end;
	  k = 1 ;
	  for_end = 19 ;
	  if ( k <= for_end)
		  do mem [ k ] .cint = 0 ; 
	  while ( k++ < for_end ) ;
  } 
  k = 0 ; 
  while ( k <= 19 ) {			/* while k <= lo_mem_stat-max ... */
/*  glue_ref_count(k):=null+1; */
    mem [ k ] .hh .v.RH = 1 ; 
    mem [ k ] .hh.b0 = 0 ; 
    mem [ k ] .hh.b1 = 0 ; 
    k = k + 4 ; 
  } 
  mem [ 6 ] .cint = 65536L ; 
  mem [ 4 ] .hh.b0 = 1 ; 
  mem [ 10 ] .cint = 65536L ; 
  mem [ 8 ] .hh.b0 = 2 ; 
  mem [ 14 ] .cint = 65536L ; 
  mem [ 12 ] .hh.b0 = 1 ; 
  mem [ 15 ] .cint = 65536L ; 
  mem [ 12 ] .hh.b1 = 1 ; 
  mem [ 18 ] .cint = -65536L ; 
  mem [ 16 ] .hh.b0 = 1 ; 
  rover = 20 ;					/* rover = lo_mem_stat-max + 1 ... */
/*  mem [ rover ] .hh .v.RH = 262143L ;  */
  mem [ rover ] .hh .v.RH = emptyflag ; 
/*  mem [ rover ] .hh .v.LH = 1000 ; */
  mem [ rover ] .hh .v.LH = blocksize ; 
  mem [ rover + 1 ] .hh .v.LH = rover ; 
  mem [ rover + 1 ] .hh .v.RH = rover ; 
/*  lomemmax = rover + 1000 ; */
  lomemmax = rover + blocksize ;
  mem [ lomemmax ] .hh .v.RH = 0 ; 
  mem [ lomemmax ] .hh .v.LH = 0 ; 
/* for k <- hi_mem_stat_min to mem_top do mem[k] = mem[lo_mem_max]; */
  {
	  register integer for_end;
	  k = memtop - 13 ;
	  for_end = memtop ;
	  if ( k <= for_end)
		  do  mem [ k ] = mem [ lomemmax ] ; 
	  while ( k++ < for_end ) ;
  } 
/* info(omit_template) <- end_template_token; p.790 */
/* @d end_template_token==cs_token_flag+frozen_end_template */
/*  mem [ memtop - 10 ] .hh .v.LH = 14114 ;  */
/*  mem [ memtop - 10 ] .hh .v.LH = 10019 + 4095 ; */  /* + eqtbextra ? NO */
/*  mem [ memtop - 10 ] .hh .v.LH = (hash_size + 4614) ; */
/*  mem [ memtop - 10 ] .hh .v.LH = (hash_size + 4095 + 519) ; */
  mem [ memtop - 10 ] .hh .v.LH = (hash_size + hash_extra + 4095 + 519) ; 
/* link(end_span) <- max_quarterword + 1 p.797 */
/*  mem [ memtop - 9 ] .hh .v.RH = 256 ;  */			/* 94/Apr/4 ? */
  mem [ memtop - 9 ] .hh .v.RH = maxquarterword + 1 ;   /* 96/Oct/12 ??? */
/* info(end_span) <- null p.797 */
  mem [ memtop - 9 ] .hh .v.LH = 0 ; 
/* type(last_active) <- hyphenated; p.820 */
  mem [ memtop - 7 ] .hh.b0 = 1 ; 
/* line_number(last_active) <- max_halfword; p.820 */
/*  mem [ memtop - 6 ] .hh .v.LH = 262143L ;  */
  mem [ memtop - 6 ] .hh .v.LH = emptyflag ;	/* maxhalfword ? */
/* subtype(last_active) <- 0; p.820 */
  mem [ memtop - 7 ] .hh.b1 = 0 ; 
/* subtype(page_ins_head) <- 255; p.981 */
  mem [ memtop ] .hh.b1 = 255 ;	/* subtype(page_ins_head) = qi(255)  p.981 */ 
/* type(page_ins_head) <- split_up; p.981 */
  mem [ memtop ] .hh.b0 = 1 ; 
/* link(page_ins_head) <- page_ins_head; p.981 */
  mem [ memtop ] .hh .v.RH = memtop ; 
/* type(page_head) <- glue_node; p. 988 */
  mem [ memtop - 2 ] .hh.b0 = 10 ; 
/* subtype(page_head) <- normal; p. 988 */
  mem [ memtop - 2 ] .hh.b1 = 0 ; 
  avail = 0 ;							/* avail <- null p.164 */
  memend = memtop ;						/* mem_end <- mem_top */
  himemmin = memtop - 13 ;				/* hi_mem_min <- hi_mem_stat_min */
  varused = 20 ;						/* var_used <- lo_mem_stat_max */
  dynused = 14 ;						/* dyn_used <- hi_mem_stat_usage */
/* eq_type(undefined_control_sequence) <- undefined_cs; */
/* equiv(undefined_control_sequence) <- null; */
/* eq_level(undefined_control_sequence) <- level_zero; */
  eqtb [ (hash_size + 781) ] .hh.b0 = 101 ; 
  eqtb [ (hash_size + 781) ] .hh .v.RH = 0 ; 
  eqtb [ (hash_size + 781) ] .hh.b1 = 0 ; 
/* for k <- active_base to undefined_control_sequence -1 do */
/*				eqtb[k] <- eqtb(undefined_control_sequence); */
/*  {register integer for_end; k = 1 ; for_end = 10280 ; if ( k <= for_end) do */
  {
	  register integer for_end;
	  k = 1 ;
	  for_end = (hash_size + 780) ;
	  if ( k <= for_end) do 
		  eqtb [ k ] = eqtb [ (hash_size + 781) ] ; 
	  while ( k++ < for_end ) ;
  } 
  eqtb [ (hash_size + 782) ] .hh .v.RH = 0 ; /* glue_base (hash_size + 782) */
  eqtb [ (hash_size + 782) ] .hh.b1 = 1 ; 
  eqtb [ (hash_size + 782) ] .hh.b0 = 117 ; 
/* equiv(glue_base):=zero_glue; eq_level(glue_base):=level_one; */
/* eq_type(glue_base):=glue_ref; */
/*  {register integer for_end; k = 10283 ; for_end = 10811 ; if ( k <= for_end) */
  {
	  register integer for_end;
	  k = (hash_size + 783) ;
	  for_end = (hash_size + 1311) ;
	  if ( k <= for_end) 
		  do eqtb [ k ] = eqtb [ (hash_size + 782) ] ; 
	  while ( k++ < for_end ) ;
  } 
/* glue_ref_count(zero_glue):=glue_ref_count(zero_glue)+local_base-glue_base; */
/* local_base - glue_base = 530 = 17 glue_pars + 256 skip + 256 mu_skip */
  mem [ 0 ] .hh .v.RH = mem [ 0 ] .hh .v.RH + 530 ; /* mem [ membot ] ? */
/* box(0):=null; eq_type(box_base):=box_ref; eq_level(box_base):=level_one; */
  eqtb [ (hash_size + 1312) ] .hh .v.RH = 0 ; 
  eqtb [ (hash_size + 1312) ] .hh.b0 = 118 ; 
  eqtb [ (hash_size + 1312) ] .hh.b1 = 1 ; 
/* for k:=box_base+1 to box_base+255 do eqtb[k]:=eqtb[box_base]; */
/*  {register integer for_end; k = 10813 ; for_end = 11077 ; if ( k <= for_end) */
  {
	  register integer for_end;
	  k = (hash_size + 1313) ;
	  for_end = (hash_size + 1577) ;
	  if ( k <= for_end) 
		  do  eqtb [ k ] = eqtb [ (hash_size + 781) ] ; 
	  while ( k++ < for_end ) ;
  } 
  eqtb [ (hash_size + 1578) ] .hh .v.RH = 0 ; 
  eqtb [ (hash_size + 1578) ] .hh.b0 = 119 ; 
  eqtb [ (hash_size + 1578) ] .hh.b1 = 1 ; 
/*  {register integer for_end; k = 11079 ; for_end = 11333 ; if ( k <= for_end) */
  {
	  register integer for_end;
	  k = (hash_size + 1579) ;
	  for_end = (hash_size + 1833) ;
	  if ( k <= for_end) 
		  do eqtb [ k ] = eqtb [ (hash_size + 1578) ] ; 
	  while ( k++ < for_end ) ;
  } 
  eqtb [ (hash_size + 1834) ] .hh .v.RH = 0 ; 
  eqtb [ (hash_size + 1834) ] .hh.b0 = 120 ; 
  eqtb [ (hash_size + 1834) ] .hh.b1 = 1 ; 
/*  {register integer for_end; k = 11335 ; for_end = 11382 ; if ( k <= for_end) */
  {
	  register integer for_end;
	  k = (hash_size + 1835) ;
	  for_end = (hash_size + 1882) ;
	  if ( k <= for_end) 
		  do  eqtb [ k ] = eqtb [ (hash_size + 1834) ] ; 
	  while ( k++ < for_end ) ;
  } 
  eqtb [ (hash_size + 1883) ] .hh .v.RH = 0 ; 
  eqtb [ (hash_size + 1883) ] .hh.b0 = 120 ; 
  eqtb [ (hash_size + 1883) ] .hh.b1 = 1 ; 
/* initialize cat_code, lc_code, uc_code, sf_code, math_code 256 * 5 */
/*  {register integer for_end; k = 11384 ; for_end = 12662 ; if ( k <= for_end) */
  {
	  register integer for_end;
	  k = (hash_size + 1884) ;
	  for_end = (hash_size + 3162) ;
	  if ( k <= for_end) 
		  do eqtb [ k ] = eqtb [ (hash_size + 1883) ] ; 
	  while ( k++ < for_end ) ;
  } 
/* cat_code(k) <- other_char; math_code(k) <- hi(k); sf_code(k) = 1000; */
  {
	  register integer for_end;
	  k = 0 ;
	  for_end = 255 ;
	  if ( k <= for_end) do 
	  {
		  eqtb [ (hash_size + 1883) + k ] .hh .v.RH = 12 ; 
		  eqtb [ (hash_size + 2907) + k ] .hh .v.RH = k ; 
		  eqtb [ (hash_size + 2651) + k ] .hh .v.RH = 1000 ; 
	  } 
	  while ( k++ < for_end ) ;
  } 
/* cat_base == 11383 */
/* cat_code(carriage_return) <- car_ret; cat_code(" ") <- space */
/* cat_code("\") <- escape; cat_code("%") <- comment; ... */  
  eqtb [ (hash_size + 1896) ] .hh .v.RH = 5 ; 
  eqtb [ (hash_size + 1915) ] .hh .v.RH = 10 ; 
  eqtb [ (hash_size + 1975) ] .hh .v.RH = 0 ; 
  eqtb [ (hash_size + 1920) ] .hh .v.RH = 14 ; 
  eqtb [ (hash_size + 2010) ] .hh .v.RH = 15 ; 
  eqtb [ (hash_size + 1883) ] .hh .v.RH = 9 ; 
/* for k:="0" to "9" do math_code(k):=hi(k+var_code); */
  {
	  register integer for_end;
	  k = 48 ;
	  for_end = 57 ;
	  if ( k <= for_end) do 
		  eqtb [ (hash_size + 2907) + k ] .hh .v.RH = k + 28672 ; /* '70000 */
	  while ( k++ < for_end ) ;
  } 
/* cat_code of uppercase and lowercase letters ... */
  {
	  register integer for_end;
	  k = 65 ;
	  for_end = 90 ;
	  if ( k <= for_end) do 
    {
/* cat_code ... */
      eqtb [ (hash_size + 1883) + k ] .hh .v.RH = 11 ; 
      eqtb [ (hash_size + 1883) + k + 32 ] .hh .v.RH = 11 ; 
/* mathcode(k) = hi(k + var_code + "100); */ /* '70000 + 256 */
      eqtb [ (hash_size + 2907) + k ] .hh .v.RH =
		  k + 28928 ; 	/* '70000 + 256 */
/* mathcode(k + "a" - "A") = hi(k + "a" - "A" + var_code + "100); */
      eqtb [ (hash_size + 2907) + k + 32 ] .hh .v.RH =
		  k + 28960 ;  	/* '70000 + 256 + 32 */
/* lc_code ... */
      eqtb [ (hash_size + 2139) + k ] .hh .v.RH = k + 32 ; 
      eqtb [ (hash_size + 2139) + k + 32 ] .hh .v.RH = k + 32 ; 
/* uc_code ... */
      eqtb [ (hash_size + 2395) + k ] .hh .v.RH = k ; 
      eqtb [ (hash_size + 2395) + k + 32 ] .hh .v.RH = k ; 
/* sf_code */
      eqtb [ (hash_size + 2651) + k ] .hh .v.RH = 999 ; 
    } 
  while ( k++ < for_end ) ;
  } 
/*  {register integer for_end; k = 12663 ; for_end = 12973 ; if ( k <= for_end) */
  {
	  register integer for_end;
	  k = (hash_size + 3163) ;
	  for_end = (hash_size + 3473) ;
	  if ( k <= for_end) 
		  do 
			  eqtb [ k ] .cint = 0 ; 
	  while ( k++ < for_end ) ;
  } 
  eqtb [ (hash_size + 3180) ] .cint = 1000 ; 
  eqtb [ (hash_size + 3164) ] .cint = 10000 ; 
  eqtb [ (hash_size + 3204) ] .cint = 1 ; 
  eqtb [ (hash_size + 3203) ] .cint = 25 ; 
  eqtb [ (hash_size + 3208) ] .cint = 92 ; 
  eqtb [ (hash_size + 3211) ] .cint = 13 ; 
/*  {register integer for_end; k = 13230 ; for_end = 13506 ; if ( k <= for_end) */
  {
	  register integer for_end;
	  k = 0 ;
	  for_end = 255 ;
	  if ( k <= for_end) do 
		  eqtb [ (hash_size + 3474) + k ] .cint = -1 ; 
	  while ( k++ < for_end ) ;
  } 
  eqtb [ (hash_size + 3520) ] .cint = 0 ; 
/*  {register integer for_end; k = 13230 ; for_end = 13506 ; if ( k <= for_end) */
  {
	  register integer for_end;
	  k = (hash_size + 3730) ;
	  for_end = (hash_size + 4006) ;
	  if ( k <= for_end) 
		  do  eqtb [ k ] .cint = 0 ; 
	  while ( k++ < for_end ) ;
  } 
/*  hashused = 10014 ;  */ /*   hashused = frozen_control_sequence */
/* frozen_control_sequence =  hashsize + hashbase p.222 */
/*  hashused = (hash_size + 514) ;  */
  hashused = (hash_size + hash_extra + 514) ;	/* 96/Jan/10 */
  cscount = 0 ; 
  if (traceflag) showline("itex cscount=0 ", 0);		/* debugging */
/* eq_type(frozen_dont_expand) <- dont_expand; */
/*  eqtb [ 10023 ] .hh.b0 = 116 ;  */
/*  eqtb [ (hash_size + 523) ] .hh.b0 = 116 ;  */
  eqtb [ (hash_size + hash_extra + 523) ] .hh.b0 = 116 ; 
/*  hash [ (hash_size + 523) ] .v.RH = 499 ;  */
  hash [ (hash_size + hash_extra + 523) ] .v.RH = 499 ;	/* notexpanded */
/* @<Initialize table...@>= l.10750 */
  fontptr = 0 ;				/* font_ptr:=null_font; */
  fmemptr = 7 ;				/* fmem_ptr:=7; */
  fontname [ 0 ] = 795 ;	/* nullfont */
  fontarea [ 0 ] = 335 ;	/* "" */
  hyphenchar [ 0 ] = 45 ;	/* - */
  skewchar [ 0 ] = -1 ; 
/* ************************************************************************ */
/* bchar_label[null_font]:=non_address; */ /* 3.14159 */
/*  bcharlabel [ 0 ] = fontmemsize ; */	/* OK ? 93/Nov/26 */
  bcharlabel [ 0 ] = non_address ;	/* i.e. 0 --- 96/Jan/16  */
/* ************************************************************************ */
  fontbchar [ 0 ] = 256 ;	/* font_bchar[null_font]:=non_char; */
  fontfalsebchar [ 0 ] = 256 ; /* font_false_bchar[null_font]:=non_char; */
  fontbc [ 0 ] = 1 ; 
  fontec [ 0 ] = 0 ; 
  fontsize [ 0 ] = 0 ; 
  fontdsize [ 0 ] = 0 ; 
  charbase [ 0 ] = 0 ; 
  widthbase [ 0 ] = 0 ; 
  heightbase [ 0 ] = 0 ; 
  depthbase [ 0 ] = 0 ; 
  italicbase [ 0 ] = 0 ; 
  ligkernbase [ 0 ] = 0 ; 
  kernbase [ 0 ] = 0 ; 
  extenbase [ 0 ] = 0 ; 
  fontglue [ 0 ] = 0 ; 
  fontparams [ 0 ] = 7 ; 
  parambase [ 0 ] = -1 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***  */
  resettrie ();					/* shared 93/Nov/26 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***  */
/*  {register integer for_end; k = 0 ; for_end = 6 ; if ( k <= for_end) do 
    fontinfo [ k ] .cint = 0 ; 
  while ( k++ < for_end ) ; } 
  {register integer for_end; k = - (integer) trieopsize ; for_end = 
  trieopsize ; if ( k <= for_end) do 
    trieophash [ k ] = 0 ; 
  while ( k++ < for_end ) ; } 
  {register integer for_end; k = 0 ; for_end = 255 ; if ( k <= for_end) do 
    trieused [ k ] = mintrieop ; 
  while ( k++ < for_end ) ; } 
  maxopused = mintrieop ; 
  trieopptr = 0 ; 
  trienotready = true ; 
  triel [ 0 ] = 0 ; 
  triec [ 0 ] = 0 ; 
  trieptr = 0 ; */
/* text(frozen_protection):="inaccessible"; */
/*  hash [ 10014 ] .v.RH = 1183 ; */
/*  hash [ (hash_size + 514) ] .v.RH = 1184 ;  */
  hash [ (hash_size + hash_extra + 514) ] .v.RH = 1184 ; /* 1183 */
  formatident = 1251 ;	/* 1250 */
/*  hash [ (hash_size + 522) ] .v.RH = 1290 ; */ /* 1288 */
  hash [ (hash_size + hash_extra + 522) ] .v.RH = 1290 ; /* 1288 */
/*  eqtb [ (hash_size + 522) ] .hh.b1 = 1 ;  */
  eqtb [ (hash_size + hash_extra + 522) ] .hh.b1 = 1 ; 
/*  eqtb [ (hash_size + 522) ] .hh.b0 = 113 ;  */
  eqtb [ (hash_size + hash_extra + 522) ] .hh.b0 = 113 ; 
/*  eqtb [ (hash_size + 522) ] .hh .v.RH = 0 ;  */
  eqtb [ (hash_size + hash_extra + 522) ] .hh .v.RH = 0 ; 
} 
#endif /* INITEX */

#ifdef INITEX
/* starts string pool with strings for 256 chars and then reads tex.pool */
/* adjusted to try both "tex.pool" and "tex.poo" 95/Feb/19 */
booleane getstringsstarted ( ) 
{/* 30 10 */ register booleane Result; getstringsstarted_regmem 
  unsigned char k, l  ; 
  ASCIIcode m, n  ; 
  strnumber g  ; 
  integer a  ; 
  booleane c  ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  int flag ;										/* 95/Feb/19 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  poolptr = 0 ; 
  strptr = 0 ; 
  strstart [ 0 ] = 0 ; 
  {
	  register integer for_end;
	  k = 0 ;
	  for_end = 255 ;
	  if ( k <= for_end) do 
	  {
		  if ( ( ( k < 32 ) || ( k > 126 ) ) ) 
		  {
			  {
				  strpool [ poolptr ] = 94 ; /* ^ */
				  incr ( poolptr ) ; 
			  } 
			  {
				  strpool [ poolptr ] = 94 ; /* ^ */
				  incr ( poolptr ) ; 
			  } 
			  if ( k < 64 ) 
			  {
				  strpool [ poolptr ] = k + 64 ; 
				  incr ( poolptr ) ; 
			  } 
			  else if ( k < 128 ) 
			  {
				  strpool [ poolptr ] = k - 64 ; 
				  incr ( poolptr ) ; 
			  } 
			  else {
				  l = k / 16 ;  
/*					l = k >> 4 ;  */
				  if ( l < 10 ) 
				  {
					  strpool [ poolptr ] = l + 48 ;	/* '0' */
					  incr ( poolptr ) ; 
				  } 
				  else {
					  strpool [ poolptr ] = l + 87 ;	/* 'a' - 10 */
					  incr ( poolptr ) ; 
				  } 
				  l = k % 16 ;  
/*					l = k & 15 ; */
				  if ( l < 10 ) 
				  {
					  strpool [ poolptr ] = l + 48 ;	/* '0' */
					  incr ( poolptr ) ; 
				  } 
				  else {
					  strpool [ poolptr ] = l + 87 ;	/* 'a' - 10 */
					  incr ( poolptr ) ; 
				  } 
			  } 
		  } 
		  else {
			  strpool [ poolptr ] = k ; 
			  incr ( poolptr ) ; 
		  } 
		  g = makestring () ; 
	  } 
	  while ( k++ < for_end ) ;
  } 
  vstrcpy ( nameoffile + 1 , poolname ) ; 
  nameoffile [ 0 ] = ' ' ; 
  nameoffile [ strlen ( poolname ) + 1 ] = ' ' ; 
  namelength = strlen ( poolname ) ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*  if ( aopenin ( poolfile , TEXPOOLPATH ) )  */
  flag = aopenin ( poolfile , TEXPOOLPATH ) ;
  if (flag == 0) {							/* 95/Feb/19 */
	  poolname [namelength - 1] = '\0'; 	/* `tex.pool' => `tex.poo' */
	  vstrcpy ( nameoffile + 1 , poolname ) ; 
	  nameoffile [ 0 ] = ' ' ; 
	  nameoffile [ strlen ( poolname ) + 1 ] = ' ' ; 
	  namelength = strlen ( poolname ) ;	  
	  flag = aopenin ( poolfile , TEXPOOLPATH ) ;
  }
  if (flag) 
  {
    c = false ; 
    do {
	{ 
/*	if ( eof ( poolfile ) )	*/			/* feof ( poolfile ) ??? */
	if ( test_eof ( poolfile ) ) 
	{
	  ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  showline( "! string pool file has no check sum.\n", 1) ; 
/*	  added following bit of explanation 96/Jan/16 */
	  if (! knuthflag) 
	      badformatorpool(stringfile, "the pool file", "TEXPOOL");
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  (void) aclose ( poolfile ) ; 
	  Result = false ; 
	  return(Result) ; 
	} 
	read ( poolfile , m ) ; 
	read ( poolfile , n ) ; 
	if ( m == '*' )					/* last line starts with * */
	{
	  a = 0 ; 
	  k = 1 ; 
	  while ( true ) {
	    if ( ( xord [ n ] < 48 ) || ( xord [ n ] > 57 ) ) 
	    {
	      ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
		  showline( "! string pool file check sum doesn't have nine digits.\n", 1);
/*		  added following bit of explanation 96/Jan/16 */
		  if (! knuthflag) 
		      badformatorpool(stringfile, "the pool file", "TEXPOOL");
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	      (void) aclose ( poolfile ) ; 
	      Result = false ; 
	      return(Result) ; 
	    } 
	    a = 10 * a + xord [ n ] - 48 ; 
	    if ( k == 9 ) 
	    goto lab30 ; 
	    incr ( k ) ; 
	    read ( poolfile , n ) ; 
	  } 
/*	  tex.pool file check sum *367403084 */
	  lab30: if ( a != BEGINFMTCHECKSUM ) {
	    ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
		showline( "! string pool check sum doesn't match; tangle me again.\n", 1) ;
/*	    added following bit of explanation 96/Jan/16 */
		if (! knuthflag) 
		    badformatorpool(stringfile, "the pool file", "TEXPOOL");
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	    (void) aclose ( poolfile ) ; 
	    Result = false ; 
	    return(Result) ; 
	  } 
	  c = true ; 
	} 
	else {
	    
	  if ( ( xord [ m ] < 48 ) || ( xord [ m ] > 57 ) || ( xord [ n ] < 48 
	  ) || ( xord [ n ] > 57 ) ) 
	  {
	    ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
		showline( "! string pool line doesn't begin with two digits.\n", 1);
/*		added following bit of explanation 96/Jan/16 */
		if (! knuthflag) 
		    badformatorpool(stringfile, "the pool file", "TEXPOOL");
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	    (void) aclose ( poolfile ) ; 
	    Result = false ; 
	    return(Result) ; 
	  } 
	  l = xord [ m ] * 10 + xord [ n ] - 48 * 11 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
/* can freely extend memory, so we need not be paranoid - stringvacancies */
/*	  if ( poolptr + l + stringvacancies > currentpoolsize ) */
	  if ( poolptr + l + stringmargin > currentpoolsize ) {
		  if (traceflag) showline("String margin reallocation\n", 0);
/*		  strpool =  reallocstrpool (poolptr + l + stringvacancies */
		  strpool =  reallocstrpool (poolptr + l + stringmargin 
					- currentpoolsize  + incrementpoolsize);
	  }
	  if ( poolptr + l + stringmargin > currentpoolsize )	/* 94/Jan/24 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  if ( poolptr + l + stringvacancies > poolsize ) 
#endif
	  {
	    ; 
	    showline("! You have to increase POOLSIZE.\n", 1) ;
	    (void) aclose ( poolfile ) ; 
	    Result = false ; 
	    return(Result) ; 
	  } 
	  {
		  register integer for_end;
		  k = 1 ;
		  for_end = l ;
		  if ( k <= for_end) 
			  do 
			  {
				  if ( eoln ( poolfile ) ) 
					  m = ' ' ; 
				  else read ( poolfile , m ) ; 
				  {
					  strpool [ poolptr ] = xord [ m ] ; 
					  incr ( poolptr ) ; 
				  } 
			  } 
		  while ( k++ < for_end ) ;
	  } 
	  readln ( poolfile ) ;		/* flush rest to end of file / end of line */
	  g = makestring () ; 
	} 
      } 
    } while ( ! ( c ) ) ; 
    (void) aclose ( poolfile ) ; 
    Result = true ; 
  }
  else {
      
    ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    sprintf(logline, "! I can't read %s.\n" , poolname ) ; 
	showline(logline, 1);
	if (! knuthflag)
	showline( "  (Was unable to find tex.poo or tex.pool)\n", 0); /* extra explain */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    Result = false ; 
    return(Result) ; 
  } 
  return Result ; 
} 
#endif /* INITEX */

#ifdef INITEX
void sortavail ( ) 
{sortavail_regmem 
  halfword p, q, r  ; 
  halfword oldrover  ; 
  p = getnode ( 1073741824L ) ;			/* 2^30 merge adjacent free nodes */
  p = mem [ rover + 1 ] .hh .v.RH ; 
/*  mem [ rover + 1 ] .hh .v.RH = 262143L ;  */ /* NO! */
  mem [ rover + 1 ] .hh .v.RH = emptyflag ; 
  oldrover = rover ; 
  while ( p != oldrover ) if ( p < rover ) 
  {
    q = p ; 
    p = mem [ q + 1 ] .hh .v.RH ; 
    mem [ q + 1 ] .hh .v.RH = rover ; 
    rover = q ; 
  } 
  else {
      
    q = rover ; 
    while ( mem [ q + 1 ] .hh .v.RH < p ) q = mem [ q + 1 ] .hh .v.RH ; 
    r = mem [ p + 1 ] .hh .v.RH ; 
    mem [ p + 1 ] .hh .v.RH = mem [ q + 1 ] .hh .v.RH ; 
    mem [ q + 1 ] .hh .v.RH = p ; 
    p = r ; 
  } 
  p = rover ; 
/*  while ( mem [ p + 1 ] .hh .v.RH != 262143L ) { */	/* NO! */
  while ( mem [ p + 1 ] .hh .v.RH != emptyflag ) {
      
    mem [ mem [ p + 1 ] .hh .v.RH + 1 ] .hh .v.LH = p ; 
    p = mem [ p + 1 ] .hh .v.RH ; 
  } 
  mem [ p + 1 ] .hh .v.RH = rover ; 
  mem [ rover + 1 ] .hh .v.LH = p ; 
} 
#endif /* INITEX */

#ifdef INITEX
void zprimitive ( s , c , o ) 
strnumber s ; 
quarterword c ; 
halfword o ; 
{primitive_regmem 
  poolpointer k  ; 
  smallnumber j  ; 
/*  smallnumber l  ;  */
  int l  ;						/* 95/Jan/7 */
  if ( s < 256 ) 
	  curval = s + 257 ;	/* cur_val <- s + single_base; p.264 */
  else {
    k = strstart [ s ] ; 
    l = strstart [ s + 1 ] - k ;			/* smallnumber l */
    {
		register integer for_end;
		j = 0 ;
		for_end = l - 1 ;
		if ( j <= for_end) 
			do buffer [ j ] = strpool [ k + j ] ; 
		while ( j++ < for_end ) ;
	} 
    curval = idlookup ( 0 , l ) ; 
    {
      decr ( strptr ) ; 
      poolptr = strstart [ strptr ] ; 
    } 
/*	**********************  debugging only  96/Jan/20 should not happen */
#ifdef SHORTHASH
	if (s > 65535L) showline("ERROR: hash entry too large\n", 1);
#endif
    hash [ curval ] .v.RH = s ;
  }
  eqtb [ curval ] .hh.b1 = 1 ; 
  eqtb [ curval ] .hh.b0 = c ; 
  eqtb [ curval ] .hh .v.RH = o ; 
} 
#endif /* INITEX */

/* more weird constants ? page 394 */

#ifdef INITEX
trieopcode znewtrieop ( d , n , v ) 
smallnumber d ; 
smallnumber n ; 
trieopcode v ; 
{/* 10 */ register trieopcode Result; newtrieop_regmem 
  integer h  ; 
  trieopcode u  ; 
  integer l  ; 
/* the 313, 361 and 1009 are hard-wired constants here p.944 */
/* begin h:=abs(n+313*d+361*v+1009*cur_lang) mod (trie_op_size+trie_op_size) */
  h = abs ( toint ( n ) + 313 * toint ( d ) + 361 * toint ( v ) + 1009 *
		toint ( curlang ) ) % ( trieopsize - negtrieopsize ) + negtrieopsize ; 
  while ( true ) {

/*  if l=0 then {empty position found for a new op} */
    l = trieophash [ h ] ; 
    if ( l == 0 ) 
    {
      if ( trieopptr == trieopsize ) {
		  overflow ( 943 , trieopsize ) ;	/* pattern memory ops  - NOT DYNAMIC */
		  return 0;			// abortflag set
	  }
      u = trieused [ curlang ] ; 
/*    if u=max_quarterword then ??? */
      if ( u == maxtrieop ) {
/*		  overflow("pattern memory ops per language",
		  max_quarterword-min_quarterword); ??? */
		  overflow ( 944 , maxtrieop - mintrieop ) ; /* pattern memory ops per language */
		  return 0;			// abortflag set
	  }
      incr ( trieopptr ) ; 
      incr ( u ) ; 
      trieused [ curlang ] = u ; 
      if ( u > maxopused ) 
      maxopused = u ; 
      hyfdistance [ trieopptr ] = d ; 
      hyfnum [ trieopptr ] = n ; 
      hyfnext [ trieopptr ] = v ; 
      trieoplang [ trieopptr ] = curlang ; 
      trieophash [ h ] = trieopptr ; 
      trieopval [ trieopptr ] = u ; 
      Result = u ; 
      return(Result) ; 
    } 
    if ( ( hyfdistance [ l ] == d ) && ( hyfnum [ l ] == n ) && ( hyfnext [ l 
    ] == v ) && ( trieoplang [ l ] == curlang ) ) 
    {
      Result = trieopval [ l ] ; 
      return(Result) ; 
    } 
    if ( h > - (integer) trieopsize ) 
		decr ( h ) ; 
    else h = trieopsize ; 
  } 
/*  return Result ;  */	/* unreachable code */
} 

/* what are those horrible constants there ? page 395 */

triepointer ztrienode ( p ) 
triepointer p ; 
{/* 10 */ register triepointer Result; trienode_regmem 
  triepointer h  ; 
  triepointer q  ; 
/* the 1009, 2718, 3142 are hard-wired constants here (not hyphen_prime) */
  h = abs ( toint ( triec [ p ] ) + 1009 * toint ( trieo [ p ] ) + 2718 * 
  toint ( triel [ p ] ) + 3142 * toint ( trier [ p ] ) ) % triesize ; 
  while ( true ) {
    q = triehash [ h ] ; 
    if ( q == 0 ) 
    {
      triehash [ h ] = p ; 
      Result = p ; 
      return(Result) ; 
    } 
    if ( ( triec [ q ] == triec [ p ] ) && ( trieo [ q ] == trieo [ p ] ) && ( 
    triel [ q ] == triel [ p ] ) && ( trier [ q ] == trier [ p ] ) ) 
    {
      Result = q ; 
      return(Result) ; 
    } 
    if ( h > 0 ) 
		decr ( h ) ; 
    else h = triesize ; 
  } 
/*  return Result ;  */	/* unreachable code */
} 
triepointer zcompresstrie ( p ) 
triepointer p ; 
{register triepointer Result; compresstrie_regmem 
  if ( p == 0 ) 
  Result = 0 ; 
  else {
      
    triel [ p ] = compresstrie ( triel [ p ] ) ; 
    trier [ p ] = compresstrie ( trier [ p ] ) ; 
    Result = trienode ( p ) ; 
  } 
  return Result ; 
} 

void zfirstfit ( p ) 
triepointer p ; 
{/* 45 40 */ firstfit_regmem 
  triepointer h  ; 
  triepointer z  ; 
  triepointer q  ; 
  ASCIIcode c  ; 
  triepointer l, r  ; 
  short ll  ; 
  c = triec [ p ] ; 
  z = triemin [ c ] ; 
  while ( true ) {
    h = z - c ; 
    if ( triemax < h + 256 ) 
    {
      if ( triesize <= h + 256 ) { 	
		  overflow ( 945 , triesize ) ;  /* pattern memory - NOT DYNAMIC */
/*		  not dynamic ---- but can be set -h=... from command line in ini-TeX */
		  return;			// abortflag set
	  }
      do {
	  incr ( triemax ) ; 
	trietaken [ triemax ] = false ; 
	trietrl [ triemax ] = triemax + 1 ; 
	trietro [ triemax ] = triemax - 1 ; 
      } while ( ! ( triemax == h + 256 ) ) ; 
    } 
    if ( trietaken [ h ] ) 
    goto lab45 ; 
    q = trier [ p ] ; 
    while ( q > 0 ) {
      if ( trietrl [ h + triec [ q ] ] == 0 ) 
      goto lab45 ; 
      q = trier [ q ] ; 
    } 
    goto lab40 ; 
    lab45: z = trietrl [ z ] ; 
  } 
lab40:
  trietaken [ h ] = true ; /* h may be used without ... */
  triehash [ p ] = h ; 
  q = p ; 
  do {
      z = h + triec [ q ] ; 
    l = trietro [ z ] ; 
    r = trietrl [ z ] ; 
    trietro [ r ] = l ; 
    trietrl [ l ] = r ; 
    trietrl [ z ] = 0 ; 
    if ( l < 256 ) 
    {
      if ( z < 256 ) 
      ll = z ;					/* short ll */
      else ll = 256 ; 
      do {
	  triemin [ l ] = r ; 
	incr ( l ) ; 
      } while ( ! ( l == ll ) ) ; 
    } 
    q = trier [ q ] ; 
  } while ( ! ( q == 0 ) ) ; 
} 

void ztriepack ( p ) 
triepointer p ; 
{triepack_regmem 
  triepointer q  ; 
  do {
      q = triel [ p ] ; 
    if ( ( q > 0 ) && ( triehash [ q ] == 0 ) ) 
    {
      firstfit ( q ) ; 
      triepack ( q ) ; 
    } 
    p = trier [ p ] ; 
  } while ( ! ( p == 0 ) ) ; 
} 
void ztriefix ( p ) 
triepointer p ; 
{triefix_regmem 
  triepointer q  ; 
  ASCIIcode c  ; 
  triepointer z  ; 
  z = triehash [ p ] ; 
  do {
      q = triel [ p ] ; 
    c = triec [ p ] ; 
    trietrl [ z + c ] = triehash [ q ] ; 
    trietrc [ z + c ] = c ; 
    trietro [ z + c ] = trieo [ p ] ; 
    if ( q > 0 ) 
    triefix ( q ) ; 
    p = trier [ p ] ; 
  } while ( ! ( p == 0 ) ) ; 
} 
void newpatterns ( ) 
{/* 30 31 */ newpatterns_regmem 
/* ******************************************************************* */
/*  was smallnumber k, l  ;  in 3.141 */
  char k, l  ; 
/* ******************************************************************* */
  booleane digitsensed  ; 
  trieopcode v  ; 
  triepointer p, q  ; 
  booleane firstchild  ; 
/*  ASCIIcode c  ;  */
  int c  ;								/* 95/Jan/7 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (! trienotready) {					/* new stuff */
	  if (allowpatterns) {	
		  if (traceflag) showline("Resetting patterns\n", 0);
		  resettrie();					/* RESET PATTERNS -  93/Nov/26 */
		  if (resetexceptions) {
			  if (traceflag) showline("Resetting exceptions\n", 0);
			  resethyphen();			/* RESET HYPHENEXCEPTIONS -  93/Nov/26 */
		  }
	  }
  }
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( trienotready ) 
  {
    if ( eqtb [ (hash_size + 3213) ] .cint <= 0 ) 
    curlang = 0 ; 
    else if ( eqtb [ (hash_size + 3213) ] .cint > 255 ) 
    curlang = 0 ; 
    else curlang = eqtb [ (hash_size + 3213) ] .cint ; 
    scanleftbrace () ; 
	ABORTCHECK;
    k = 0 ; 
    hyf [ 0 ] = 0 ; 
    digitsensed = false ; 
    while ( true ) {
      getxtoken () ; 
	  ABORTCHECK;
      switch ( curcmd ) 
      {case 11 : 
      case 12 : 
	if ( digitsensed || ( curchr < 48 ) || ( curchr > 57 ) ) 
	{
	  if ( curchr == 46 )		/* . */
	  curchr = 0 ; 
	  else {
	      
	    curchr = eqtb [ (hash_size + 2139) + curchr ] .hh .v.RH ; 
	    if ( curchr == 0 ) 
	    {
	      {
		if ( interaction == 3 ) 
		; 
		printnl ( 262 ) ;	/* ! */
		print ( 951 ) ;		/* Nonletter */
	      } 
	      {
		helpptr = 1 ; 
		helpline [ 0 ] = 950 ;	/* (See Appendix H.) */
	      } 
	      error () ; 
		  ABORTCHECK;
	    } 
	  } 
	  if ( k < 63 ) 
	  {
	    incr ( k ) ; 
	    hc [ k ] = curchr ; 
	    hyf [ k ] = 0 ; 
	    digitsensed = false ; 
	  } 
	} 
	else if ( k < 63 ) 
	{
	  hyf [ k ] = curchr - 48 ; 
	  digitsensed = true ; 
	} 
	break ; 
      case 10 : 
      case 2 : 
	{
	  if ( k > 0 ) 
	  {
	    if ( hc [ 1 ] == 0 ) 
	    hyf [ 0 ] = 0 ; 
	    if ( hc [ k ] == 0 ) 
	    hyf [ k ] = 0 ; 
	    l = k ; 
	    v = mintrieop ; 
	    while ( true ) {
	      if ( hyf [ l ] != 0 ) 
	      v = newtrieop ( k - l , hyf [ l ] , v ) ; 
	      if ( l > 0 ) 
	      decr ( l ) ; 
	      else goto lab31 ; 
	    } 
	    lab31: ; 
	    q = 0 ; 
	    hc [ 0 ] = curlang ; 
	    while ( l <= k ) {
	      c = hc [ l ] ; 
	      incr ( l ) ; 
	      p = triel [ q ] ; 
	      firstchild = true ; 
	      while ( ( p > 0 ) && ( c > triec [ p ] ) ) {
		q = p ; 
		p = trier [ q ] ; 
		firstchild = false ; 
	      } 
	      if ( ( p == 0 ) || ( c < triec [ p ] ) ) 
	      {
		if ( trieptr == triesize ) {
			overflow ( 945 , triesize ) ;	/* pattern memory - NOT DYNAMIC */
/*			not dynamic ---- but can be set -h=... from command line in ini-TeX */
			return;			// abortflag set
		}
		incr ( trieptr ) ; 
		trier [ trieptr ] = p ; 
		p = trieptr ; 
		triel [ p ] = 0 ; 
		if ( firstchild ) 
		triel [ q ] = p ; 
		else trier [ q ] = p ; 
		triec [ p ] = c ; 
		trieo [ p ] = mintrieop ; 
	      } 
	      q = p ; 
	    } 
	    if ( trieo [ q ] != mintrieop ) 
	    {
	      {
		if ( interaction == 3 ) 
		; 
		printnl ( 262 ) ;	/* ! */
		print ( 952 ) ;		/* Duplicate pattern */
	      } 
	      {
		helpptr = 1 ; 
		helpline [ 0 ] = 950 ;	/* (See Appendix H.) */
	      } 
	      error () ; 
		  ABORTCHECK;
	    } 
	    trieo [ q ] = v ; 
	  } 
	  if ( curcmd == 2 ) 
	  goto lab30 ; 
	  k = 0 ; 
	  hyf [ 0 ] = 0 ; 
	  digitsensed = false ; 
	} 
	break ; 
	default: 
	{
	  {
	    if ( interaction == 3 ) 
	    ; 
	    printnl ( 262 ) ;	/* ! */
	    print ( 949 ) ;		/* Bad  */
	  } 
	  printesc ( 947 ) ;	/* patterns */
	  {
	    helpptr = 1 ; 
	    helpline [ 0 ] = 950 ;	/* (See Appendix H.) */
	  } 
	  error () ; 
	  ABORTCHECK;
	} 
	break ; 
      } 
    } 
    lab30: ; 
  } 
  else {
      
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ; /* ! */
      print ( 946 ) ;	/* Too late for  */
    } 
    printesc ( 947 ) ;	/* patterns */
    {
      helpptr = 1 ; 
      helpline [ 0 ] = 948 ;	/* All patterns must be given before typesetting begins. */
    } 
    error () ; 
	ABORTCHECK;
    mem [ memtop - 12 ] .hh .v.RH = scantoks ( false , false ) ; 
	ABORTCHECK;
    flushlist ( defref ) ; 
  } 
} 

void inittrie ( ) 
{inittrie_regmem 
  triepointer p  ; 
/*  integer j, k, t  ;  */
  integer j, k  ; 
  int t;									/* 95/Jan/7 */
  triepointer r, s  ; 
  opstart [ 0 ] = - (integer) mintrieop ; 
  {
	  register integer for_end;
	  j = 1 ;
	  for_end = 255 ;
	  if ( j <= for_end) do 
		  opstart [ j ] = opstart [ j - 1 ] + trieused [ j - 1 ] ; 
	  while ( j++ < for_end ) ;
  } 
  {
	  register integer for_end;
	  j = 1 ;
	  for_end = trieopptr ;
	  if ( j <= for_end) 
		  do 
			  trieophash [ j ] = opstart [ trieoplang [ j ] ] + trieopval [ j ] ; 
	  while ( j++ < for_end ) ;
  } 
  {
	  register integer for_end;
	  j = 1 ;
	  for_end = trieopptr ;
	  if ( j <= for_end) 
		  do 
	  while ( trieophash [ j ] > j ) {
		  k = trieophash [ j ] ; 
		  t = hyfdistance [ k ] ; 
		  hyfdistance [ k ] = hyfdistance [ j ] ; 
		  hyfdistance [ j ] = t ; 
		  t = hyfnum [ k ] ; 
		  hyfnum [ k ] = hyfnum [ j ] ; 
		  hyfnum [ j ] = t ; 
		  t = hyfnext [ k ] ; 
		  hyfnext [ k ] = hyfnext [ j ] ; 
		  hyfnext [ j ] = t ; 
		  trieophash [ j ] = trieophash [ k ] ; 
		  trieophash [ k ] = k ; 
	  } 
	  while ( j++ < for_end ) ;
  } 
  {
	  register integer for_end;
	  p = 0 ;
	  for_end = triesize ;
	  if ( p <= for_end) 
		  do triehash [ p ] = 0 ; 
	  while ( p++ < for_end ) ;
  } 
  triel [ 0 ] = compresstrie ( triel [ 0 ] ) ; 
  {
	  register integer for_end;
	  p = 0 ;
	  for_end = trieptr ;
	  if ( p <= for_end) 
		  do triehash [ p ] = 0 ; 
	  while ( p++ < for_end ) ;
  } 
  {
	  register integer for_end;
	  p = 0 ;
	  for_end = 255 ;
	  if ( p <= for_end) do 
		  triemin [ p ] = p + 1 ; 
	  while ( p++ < for_end ) ;
  } 
  trietrl [ 0 ] = 1 ; 
  triemax = 0 ; 
  if ( triel [ 0 ] != 0 ) 
  {
    firstfit ( triel [ 0 ] ) ; 
    triepack ( triel [ 0 ] ) ; 
  } 
  if ( triel [ 0 ] == 0 ) 
  {
    {
		register integer for_end;
		r = 0 ;
		for_end = 256 ;
		if ( r <= for_end) do 
		{
			trietrl [ r ] = 0 ; 
			trietro [ r ] = mintrieop ; 
			trietrc [ r ] = 0 ; 
		} 
		while ( r++ < for_end ) ;
	} 
    triemax = 256 ; 
  } 
  else {
    triefix ( triel [ 0 ] ) ; 
    r = 0 ; 
    do {
	s = trietrl [ r ] ; 
      {
	trietrl [ r ] = 0 ; 
	trietro [ r ] = mintrieop ; 
	trietrc [ r ] = 0 ; 
      } 
      r = s ; 
    } while ( ! ( r > triemax ) ) ; 
  } 
  trietrc [ 0 ] = 63 ; 
  trienotready = false ; 
} 
#endif /* INITEX */

#ifdef INITEX
void storefmtfile ( ) 
{/* 41 42 31 32 */ storefmtfile_regmem 
  integer j, k, l  ; 
  halfword p, q  ; 
  integer x  ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (!is_initex) {			/* redundant check 94/Feb/14 */
	  showline("! \\dump is performed only by INITEX\n", 1);
	  if (! knuthflag)
		  showline("  (Use -i on the command line)\n", 0);
	  abortflag++;
	  return ;
  }
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( saveptr != 0 )  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ; /* ! */
      print ( 1252 ) ;	/* You can't dump inside a group */
    } 
    {
      helpptr = 1 ; 
      helpline [ 0 ] = 1253 ;	/* `{...\dump}' is a no-no.. */
    } 
    {
      if ( interaction == 3 )  interaction = 2 ; 
      if ( logopened ) {
		  error () ;
		  ABORTCHECK;
	  }
	;
#ifdef DEBUG
      if ( interaction > 0 )  debughelp () ; 
#endif /* DEBUG */
      history = 3 ; 
      jumpout () ;		// trying to \dump inside a group
//	  return;			// drops through now
	}
  } /* end of if saveptr != 0 */

  selector = 21 ; 
  print ( 1266 ) ;		/*  (format= */
  print ( jobname ) ; 
  printchar ( 32 ) ;	/*   */
/*  printint ( eqtb [ (hash_size + 3186) ] .cint % 100 ) ;  */	/* Y2K */
  printint ( eqtb [ (hash_size + 3186) ] .cint ) ;		/* 98/Oct/12 */
/*  {
	  int n= eqtb [ (hash_size + 3186) ] .cint;
	  sprintf(logline, "YEAR: %ld\n", n);
	  showline(logline, 0);
  } */
  printchar ( 46 ) ; /* . */
  printint ( eqtb [ (hash_size + 3185) ] .cint ) ; 
  printchar ( 46 ) ; /* . */
  printint ( eqtb [ (hash_size + 3184) ] .cint ) ; 
  printchar ( 41 ) ; /* ) */
  if ( interaction == 0 ) selector = 18 ; 
  else selector = 19 ; 
  {
#ifdef ALLOCATESTRING
    if ( poolptr + 1 > currentpoolsize )
		strpool = reallocstrpool (incrementpoolsize);
    if ( poolptr + 1 > currentpoolsize ) { /* in case it failed 94/Jan/24 */
		overflow ( 257 , currentpoolsize - initpoolptr ) ; /* 97/Mar/9 */
		return;			// abortflag set
	}
#else
    if ( poolptr + 1 > poolsize ) {
		overflow ( 257 , poolsize - initpoolptr ) ; /* pool size */
		return;			// abortflag set
	}
#endif
  } 
  formatident = makestring () ; 
  packjobname ( 780 ) ;		/* .fmt */
  while ( ! wopenout ( fmtfile ) ) {
	  promptfilename ( 1267 , 780 ) ; /* format file name  .fmt */
	  ABORTCHECK;
  }
  printnl ( 1268 ) ;		/* 	Beginning to dump on file  */
  slowprint ( wmakenamestring ( fmtfile ) ) ; 
  {
    decr ( strptr ) ; 
    poolptr = strstart [ strptr ] ; 
  } 
  printnl ( 335 ) ;		/* */
  slowprint ( formatident ) ; 
  dumpint ( BEGINFMTCHECKSUM ) ; /* magic FMT file start 4C 20 E6 15 hex */
/*  dumpint ( 0 ) ; */
  dumpint ( membot ) ; 
  dumpint ( memtop ) ; 
  dumpint ( (hash_size + 4006) ) ;	/* eqtbsize */
  dumpint ( hash_prime ) ; 
/*  dumpint ( 607 ) ;  */
  dumpint ( hyphen_prime ) ;		/* bkph */
  dumpint ( poolptr ) ; 
  dumpint ( strptr ) ; 
  if (dumpthings ( strstart [ 0 ] , strptr + 1 )  
	 ) return;
  if (dumpthings ( strpool [ 0 ] , poolptr )  
	 ) return;
  println () ; 
  printint ( strptr ) ; 
  print ( 1254 ) ;	/* strings of total length  */
  printint ( poolptr ) ; 
  sortavail () ; 
  varused = 0 ; 
  dumpint ( lomemmax ) ; 
  dumpint ( rover ) ; 
  p = 0 ; 
  q = rover ; 
  x = 0 ; 
  do {
      if (dumpthings ( mem [ p ] , q + 2 - p )  
		 ) return;
    x = x + q + 2 - p ; 
    varused = varused + q - p ; 
    p = q + mem [ q ] .hh .v.LH ; 
    q = mem [ q + 1 ] .hh .v.RH ; 
  } while ( ! ( q == rover ) ) ; 
  varused = varused + lomemmax - p ; 
  dynused = memend + 1 - himemmin ; 
  if (dumpthings ( mem [ p ] , lomemmax + 1 - p )  
	 ) return;
  x = x + lomemmax + 1 - p ; 
  dumpint ( himemmin ) ; 
  dumpint ( avail ) ; 
  if (dumpthings ( mem [ himemmin ] , memend + 1 - himemmin )  
	 ) return;
  x = x + memend + 1 - himemmin ; 
  p = avail ; 
  while ( p != 0 ) {
    decr ( dynused ) ; 
    p = mem [ p ] .hh .v.RH ; 
  } 
  dumpint ( varused ) ; 
  dumpint ( dynused ) ; 
  println () ; 
  printint ( x ) ; 
  print ( 1255 ) ;	/* memory locations dumped; current usage is  */
  printint ( varused ) ; 
  printchar ( 38 ) ;	/* & */
  printint ( dynused ) ; 
  k = 1 ; 
  do {
      j = k ; 
    while ( j < (hash_size + 3162) ) {
      if ( ( eqtb [ j ] .hh .v.RH == eqtb [ j + 1 ] .hh .v.RH ) && ( eqtb [ j 
      ] .hh.b0 == eqtb [ j + 1 ] .hh.b0 ) && ( eqtb [ j ] .hh.b1 == eqtb [ j + 
      1 ] .hh.b1 ) ) 
      goto lab41 ; 
      incr ( j ) ; 
    } 
    l = (hash_size + 3163) ; 
    goto lab31 ; 
    lab41: incr ( j ) ; 
    l = j ; 
    while ( j < (hash_size + 3162) ) {
      if ( ( eqtb [ j ] .hh .v.RH != eqtb [ j + 1 ] .hh .v.RH ) || ( eqtb [ j 
      ] .hh.b0 != eqtb [ j + 1 ] .hh.b0 ) || ( eqtb [ j ] .hh.b1 != eqtb [ j + 
      1 ] .hh.b1 ) ) 
      goto lab31 ; 
      incr ( j ) ; 
    } 
    lab31: dumpint ( l - k ) ; 
    if (dumpthings ( eqtb [ k ] , l - k )  
	   ) return;
    k = j + 1 ; 
    dumpint ( k - l ) ; 
  } while ( ! ( k == (hash_size + 3163) ) ) ; 
  do {
      j = k ; 
    while ( j < (hash_size + 4006) ) {
      if ( eqtb [ j ] .cint == eqtb [ j + 1 ] .cint ) 
      goto lab42 ; 
      incr ( j ) ; 
    } 
    l = (hash_size + 4007) ; 
    goto lab32 ; 
    lab42: incr ( j ) ; 
    l = j ; 
    while ( j < (hash_size + 4006) ) {
      if ( eqtb [ j ] .cint != eqtb [ j + 1 ] .cint ) 
      goto lab32 ; 
      incr ( j ) ; 
    } 
    lab32: dumpint ( l - k ) ; 
    if (dumpthings ( eqtb [ k ] , l - k )  
	   ) return;
    k = j + 1 ; 
    dumpint ( k - l ) ; 
  } while ( ! ( k > (hash_size + 4006) ) ) ; 
  dumpint ( parloc ) ; 
  dumpint ( writeloc ) ; 
  dumpint ( hashused ) ; 
/*  cs_count:=frozen_control_sequence-1-hash_used; */
/*  cscount = (hash_size + 513) - hashused ;  */
  cscount = (hash_size + hash_extra + 513) - hashused ; 
/*  cscount = (hash_size + 780) - hashused ;  */ /* ??? */
  if (traceflag) {
	  sprintf(logline, "itex cscount %d hash_size %d hash_extra %d hashused %d",
		 cscount, hash_size, hash_extra, hashused);			/* debugging */
	  showline(logline, 0);
  }
/*	for p <- hash_base to hash_used do */
  {
	  register integer for_end;
	  p = 514 ;
	  for_end = hashused ;
	  if ( p <= for_end) do 
		  if ( hash [ p ] .v.RH != 0 ) {
			  dumpint ( p ) ; 
			  dumphh ( hash [ p ] ) ; 
			  incr ( cscount ) ; 
			  if (traceflag) {
				  sprintf(logline, "itex cscount++ ");
				  showline(logline, 0); /* debugging */
			  }
		  } 
	  while ( p++ < for_end ) ;
  } 
/*	??? */
/* for p <- hash_used+1 to undefined_control_sequence-1 do dump_hh(hash[p]) */
  if (dumpthings ( hash [ hashused + 1 ] , (hash_size + 780) - hashused )  
	 ) return;
  dumpint ( cscount ) ; 
  println () ; 
  printint ( cscount ) ; 
  print ( 1256 ) ;	/*   multiletter control sequences */
  dumpint ( fmemptr ) ; 
  {
    if (dumpthings ( fontinfo [ 0 ] , fmemptr )  
	   ) return;
/*	frozenfontptr = fontptr; */		/* number of fonts frozen into format */
    dumpint ( fontptr ) ; 
    if (dumpthings ( fontcheck [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontsize [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontdsize [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontparams [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( hyphenchar [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( skewchar [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontname [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontarea [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontbc [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontec [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( charbase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( widthbase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( heightbase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( depthbase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( italicbase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( ligkernbase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( kernbase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( extenbase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( parambase [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontglue [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( bcharlabel [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontbchar [ 0 ] , fontptr + 1 )  
	   ) return;
    if (dumpthings ( fontfalsebchar [ 0 ] , fontptr + 1 )  
	   ) return;
    {
		register integer for_end;
		k = 0 ;
		for_end = fontptr ;
		if ( k <= for_end) 
			do 
      {
	printnl ( 1259 ) ;	/* \font */
/*	printesc ( hash [ (hash_size + 524) + k ] .v.RH ) ;  */
	printesc ( hash [ (hash_size + hash_extra + 524) + k ] .v.RH ) ; 
	printchar ( 61 ) ; /* = */
	printfilename ( fontname [ k ] , fontarea [ k ] , 335 ) ; 
	if ( fontsize [ k ] != fontdsize [ k ] ) 
	{
	  print ( 738 ) ;	/*   at  */
	  printscaled ( fontsize [ k ] ) ; 
	  print ( 394 ) ;	/* pt */
	} 
      } 
    while ( k++ < for_end ) ;
	} 
  } 
  println () ; 
  printint ( fmemptr - 7 ) ; 
  print ( 1257 ) ;		/* words of font info for */
  printint ( fontptr - 0 ) ; 
  print ( 1258 ) ;		/*  preloaded font */
  if ( fontptr != 1 ) 
	  printchar ( 115 ) ;	/* s */
  dumpint ( hyphcount ) ; 
/*  {register integer for_end; k = 0 ; for_end = 607 ; if ( k <= for_end) do */
  {
	  register integer for_end;
	  k = 0 ;
	  for_end = hyphen_prime ;
	  if ( k <= for_end) do 
		  if ( hyphword [ k ] != 0 ) 
		  {
			  dumpint ( k ) ; 
			  dumpint ( hyphword [ k ] ) ; 
			  dumpint ( hyphlist [ k ] ) ; 
		  } 
	  while ( k++ < for_end ) ;
  } 
  println () ; 
  printint ( hyphcount ) ; 
  print ( 1260 ) ;	/*  hyphenation exception */
  if ( hyphcount != 1 ) 
  printchar ( 115 ) ;	/* s */
  if ( trienotready ) 
  inittrie () ; 
  dumpint ( triemax ) ; 
  if (dumpthings ( trietrl [ 0 ] , triemax + 1 )  
	 ) return;
  if (dumpthings ( trietro [ 0 ] , triemax + 1 )  
	 ) return;
  if (dumpthings ( trietrc [ 0 ] , triemax + 1 )  
	 ) return;
  dumpint ( trieopptr ) ; 
  if (dumpthings ( hyfdistance [ 1 ] , trieopptr )  
	 ) return;
  if (dumpthings ( hyfnum [ 1 ] , trieopptr )  
	 ) return;
  if (dumpthings ( hyfnext [ 1 ] , trieopptr )  
	 ) return;
  printnl ( 1261 ) ;	/* Hyphenation trie of length  */
  printint ( triemax ) ; 
  print ( 1262 ) ;		/* has */
  printint ( trieopptr ) ; 
  print ( 1263 ) ;		/* op */
  if ( trieopptr != 1 ) 
  printchar ( 115 ) ;	/* s */
  print ( 1264 ) ;		/* out of */
  printint ( trieopsize ) ; 
  {
	  register integer for_end;
	  k = 255 ;
	  for_end = 0 ;
	  if ( k >= for_end) do 
		  if ( trieused [ k ] > 0 ) 
		  {
			  printnl ( 794 ) ;		/*    */
			  printint ( trieused [ k ] ) ; 
			  print ( 1265 ) ;		/* for language */
			  printint ( k ) ; 
			  dumpint ( k ) ; 
			  dumpint ( trieused [ k ] ) ; 
		  } 
	  while ( k-- > for_end ) ;
  } 
  dumpint ( interaction ) ; 
  dumpint ( formatident ) ; 
  dumpint ( ENDFMTCHECKSUM ) ;	/* magic checksum end of FMT file --- change ??? */ 
  eqtb [ (hash_size + 3194) ] .cint = 0 ;	/* tracingstats  */
  wclose ( fmtfile ) ; 
//  return 0;
} /* end of storefmtfile */
#endif /* INITEX */

#ifdef INITEX
void initprim ( ) 
{initprim_regmem 
  nonewcontrolsequence = false ; 
  primitive ( 373 , 75 , (hash_size + 782) ) ; /* lineskip */
  primitive ( 374 , 75 , (hash_size + 783) ) ; /* baselineskip */
  primitive ( 375 , 75 , (hash_size + 784) ) ; /* parskip */
  primitive ( 376 , 75 , (hash_size + 785) ) ; /* abovedisplayskip */
  primitive ( 377 , 75 , (hash_size + 786) ) ; /* belowdisplayskip */
  primitive ( 378 , 75 , (hash_size + 787) ) ; /* abovedisplayshortskip */
  primitive ( 379 , 75 , (hash_size + 788) ) ; /* belowdisplayshortskip */
  primitive ( 380 , 75 , (hash_size + 789) ) ; /* leftskip */
  primitive ( 381 , 75 , (hash_size + 790) ) ; /* rightskip */
  primitive ( 382 , 75 , (hash_size + 791) ) ; /* topskip */
  primitive ( 383 , 75 , (hash_size + 792) ) ; /* splittopskip */
  primitive ( 384 , 75 , (hash_size + 793) ) ; /* tabskip */
  primitive ( 385 , 75 , (hash_size + 794) ) ; /* spaceskip */
  primitive ( 386 , 75 , (hash_size + 795) ) ; /* xspaceskip */
  primitive ( 387 , 75 , (hash_size + 796) ) ; /* parfillskip */
  primitive ( 388 , 76 , (hash_size + 797) ) ; /* thinmuskip */
  primitive ( 389 , 76 , (hash_size + 798) ) ; /* medmuskip */
  primitive ( 390 , 76 , (hash_size + 799) ) ; /* thickmuskip */
  primitive ( 395 , 72 , (hash_size + 1313) ) ; /* output */
  primitive ( 396 , 72 , (hash_size + 1314) ) ; /* everypar */
  primitive ( 397 , 72 , (hash_size + 1315) ) ; /* everymath */
  primitive ( 398 , 72 , (hash_size + 1316) ) ; /* everydisplay */
  primitive ( 399 , 72 , (hash_size + 1317) ) ; /* everyhbox */
  primitive ( 400 , 72 , (hash_size + 1318) ) ; /* everyvbox */
  primitive ( 401 , 72 , (hash_size + 1319) ) ; /* everyjob */
  primitive ( 402 , 72 , (hash_size + 1320) ) ; /* everycr */
  primitive ( 403 , 72 , (hash_size + 1321) ) ; /* errhelp */
  primitive ( 417 , 73 , (hash_size + 3163) ) ; /* pretolerance */
  primitive ( 418 , 73 , (hash_size + 3164) ) ; /* tolerance */
  primitive ( 419 , 73 , (hash_size + 3165) ) ; /* linepenalty */
  primitive ( 420 , 73 , (hash_size + 3166) ) ; /* hyphenpenalty */
  primitive ( 421 , 73 , (hash_size + 3167) ) ; /* exhyphenpenalty */
  primitive ( 422 , 73 , (hash_size + 3168) ) ; /* clubpenalty */
  primitive ( 423 , 73 , (hash_size + 3169) ) ; /* widowpenalty */
  primitive ( 424 , 73 , (hash_size + 3170) ) ; /* displaywidowpenalty */
  primitive ( 425 , 73 , (hash_size + 3171) ) ; /* brokenpenalty */
  primitive ( 426 , 73 , (hash_size + 3172) ) ; /* binoppenalty */
  primitive ( 427 , 73 , (hash_size + 3173) ) ; /* relpenalty */
  primitive ( 428 , 73 , (hash_size + 3174) ) ; /* predisplaypenalty */
  primitive ( 429 , 73 , (hash_size + 3175) ) ; /* postdisplaypenalty */
  primitive ( 430 , 73 , (hash_size + 3176) ) ; /* interlinepenalty */
  primitive ( 431 , 73 , (hash_size + 3177) ) ; /* doublehyphendemerits */
  primitive ( 432 , 73 , (hash_size + 3178) ) ; /* finalhyphendemerits */
  primitive ( 433 , 73 , (hash_size + 3179) ) ; /* adjdemerits */
  primitive ( 434 , 73 , (hash_size + 3180) ) ; /* mag */
  primitive ( 435 , 73 , (hash_size + 3181) ) ; /* delimiterfactor */
  primitive ( 436 , 73 , (hash_size + 3182) ) ; /* looseness */
  primitive ( 437 , 73 , (hash_size + 3183) ) ; /* time */
  primitive ( 438 , 73 , (hash_size + 3184) ) ; /* day */
  primitive ( 439 , 73 , (hash_size + 3185) ) ; /* month */
  primitive ( 440 , 73 , (hash_size + 3186) ) ; /* year */
  primitive ( 441 , 73 , (hash_size + 3187) ) ; /* showboxbreadth */
  primitive ( 442 , 73 , (hash_size + 3188) ) ; /* showboxdepth */
  primitive ( 443 , 73 , (hash_size + 3189) ) ; /* hbadness */
  primitive ( 444 , 73 , (hash_size + 3190) ) ; /* vbadness */
  primitive ( 445 , 73 , (hash_size + 3191) ) ; /* pausing */
  primitive ( 446 , 73 , (hash_size + 3192) ) ; /* tracingonline */
  primitive ( 447 , 73 , (hash_size + 3193) ) ; /* tracingmacros */
  primitive ( 448 , 73 , (hash_size + 3194) ) ; /* tracingstats */
  primitive ( 449 , 73 , (hash_size + 3195) ) ; /* tracingparagraphs */
  primitive ( 450 , 73 , (hash_size + 3196) ) ; /* tracingpages */
  primitive ( 451 , 73 , (hash_size + 3197) ) ; /* tracingoutput */
  primitive ( 452 , 73 , (hash_size + 3198) ) ; /* tracinglostchars */
  primitive ( 453 , 73 , (hash_size + 3199) ) ; /* tracingcommands */
  primitive ( 454 , 73 , (hash_size + 3200) ) ; /* tracingrestores */
  primitive ( 455 , 73 , (hash_size + 3201) ) ; /* uchyph */
  primitive ( 456 , 73 , (hash_size + 3202) ) ; /* outputpenalty */
  primitive ( 457 , 73 , (hash_size + 3203) ) ; /* maxdeadcycles */
  primitive ( 458 , 73 , (hash_size + 3204) ) ; /* hangafter */
  primitive ( 459 , 73 , (hash_size + 3205) ) ; /* floatingpenalty */
  primitive ( 460 , 73 , (hash_size + 3206) ) ; /* globaldefs */
  primitive ( 461 , 73 , (hash_size + 3207) ) ; /* fam */
  primitive ( 462 , 73 , (hash_size + 3208) ) ; /* escapechar */
  primitive ( 463 , 73 , (hash_size + 3209) ) ; /* defaulthyphenchar */
  primitive ( 464 , 73 , (hash_size + 3210) ) ; /* defaultskewchar */
  primitive ( 465 , 73 , (hash_size + 3211) ) ; /* endlinechar */
  primitive ( 466 , 73 , (hash_size + 3212) ) ; /* newlinechar */
  primitive ( 467 , 73 , (hash_size + 3213) ) ; /* language */
  primitive ( 468 , 73 , (hash_size + 3214) ) ; /* lefthyphenmin */
  primitive ( 469 , 73 , (hash_size + 3215) ) ; /* righthyphenmin */
  primitive ( 470 , 73 , (hash_size + 3216) ) ; /* holdinginserts */
  primitive ( 471 , 73 , (hash_size + 3217) ) ; /* errorcontextlines */
  primitive ( 475 , 74 , (hash_size + 3730) ) ; /* parindent */
  primitive ( 476 , 74 , (hash_size + 3731) ) ; /* mathsurround */
  primitive ( 477 , 74 , (hash_size + 3732) ) ; /* lineskiplimit */
  primitive ( 478 , 74 , (hash_size + 3733) ) ; /* hsize */
  primitive ( 479 , 74 , (hash_size + 3734) ) ; /* vsize */
  primitive ( 480 , 74 , (hash_size + 3735) ) ; /* maxdepth */
  primitive ( 481 , 74 , (hash_size + 3736) ) ; /* splitmaxdepth */
  primitive ( 482 , 74 , (hash_size + 3737) ) ; /* boxmaxdepth */
  primitive ( 483 , 74 , (hash_size + 3738) ) ; /* hfuzz */
  primitive ( 484 , 74 , (hash_size + 3739) ) ; /* vfuzz */
  primitive ( 485 , 74 , (hash_size + 3740) ) ; /* delimitershortfall */
  primitive ( 486 , 74 , (hash_size + 3741) ) ; /* nulldelimiterspace */
  primitive ( 487 , 74 , (hash_size + 3742) ) ; /* scriptspace */
  primitive ( 488 , 74 , (hash_size + 3743) ) ; /* predisplaysize */
  primitive ( 489 , 74 , (hash_size + 3744) ) ; /* displaywidth */
  primitive ( 490 , 74 , (hash_size + 3745) ) ; /* displayindent */
  primitive ( 491 , 74 , (hash_size + 3746) ) ; /* overfullrule */
  primitive ( 492 , 74 , (hash_size + 3747) ) ; /* hangindent */
  primitive ( 493 , 74 , (hash_size + 3748) ) ; /* hoffset */
  primitive ( 494 , 74 , (hash_size + 3749) ) ; /* voffset */
  primitive ( 495 , 74 , (hash_size + 3750) ) ;	/* emergencystretch */
  primitive ( 32 , 64 , 0 ) ;	/*   */
  primitive ( 47 , 44 , 0 ) ;	/* / */
  primitive ( 505 , 45 , 0 ) ;	/* accent */
  primitive ( 506 , 90 , 0 ) ;	/* advance */
  primitive ( 507 , 40 , 0 ) ;	/* afterassignment */
  primitive ( 508 , 41 , 0 ) ;	/* aftergroup */
  primitive ( 509 , 61 , 0 ) ;	/* begingroup */
  primitive ( 510 , 16 , 0 ) ;	/* char */
  primitive ( 501 , 107 , 0 ) ;		/* csname */
  primitive ( 511 , 15 , 0 ) ;	/* delimiter */
  primitive ( 512 , 92 , 0 ) ;	/* divide */
  primitive ( 502 , 67 , 0 ) ;	/* endcsname */
  primitive ( 513 , 62 , 0 ) ;	/* endgroup */
/*  hash [ (hash_size + 516) ] .v.RH = 513 ; */	
  hash [ (hash_size + hash_extra + 516) ] .v.RH = 513 ;	/* endgroup */
/*  eqtb [ (hash_size + 516) ] = eqtb [ curval ] ; */
  eqtb [ (hash_size + hash_extra + 516) ] = eqtb [ curval ] ; 
  primitive ( 514 , 102 , 0 ) ;		/* expandafter */
  primitive ( 515 , 88 , 0 ) ;	/* font */
  primitive ( 516 , 77 , 0 ) ;	/* fontdimen */
  primitive ( 517 , 32 , 0 ) ;	/* halign */
  primitive ( 518 , 36 , 0 ) ;	/* hrule */
  primitive ( 519 , 39 , 0 ) ;	/* ignorespaces */
  primitive ( 327 , 37 , 0 ) ; /* insert */
  primitive ( 348 , 18 , 0 ) ; /* mark */
  primitive ( 520 , 46 , 0 ) ;	/* mathaccent */
  primitive ( 521 , 17 , 0 ) ;	/* mathchar */
  primitive ( 522 , 54 , 0 ) ;	/* mathchoice */
  primitive ( 523 , 91 , 0 ) ;	/* multiply */
  primitive ( 524 , 34 , 0 ) ;	/* noalign */
  primitive ( 525 , 65 , 0 ) ;	/* noboundary */
  primitive ( 526 , 103 , 0 ) ;		/* noexpand */
  primitive ( 332 , 55 , 0 ) ; /* nonscript */
  primitive ( 527 , 63 , 0 ) ;	/* omit */
  primitive ( 405 , 84 , 0 ) ;	/* parshape */
  primitive ( 528 , 42 , 0 ) ;	/* penalty */
  primitive ( 529 , 80 , 0 ) ;	/* prevgraf */
  primitive ( 530 , 66 , 0 ) ;	/* radical */
  primitive ( 531 , 96 , 0 ) ;	/* read */
  primitive ( 532 , 0 , 256 ) ;		/* primitive("relax",relax,256); */
/*  hash [ (hash_size + 521) ] .v.RH = 532 ; */ 
  hash [ (hash_size + hash_extra + 521) ] .v.RH = 532 ;  /* read */
/*  eqtb [ (hash_size + 521) ] = eqtb [ curval ] ;  */
  eqtb [ (hash_size + hash_extra + 521) ] = eqtb [ curval ] ; 
  primitive ( 533 , 98 , 0 ) ;	/* setbox */
  primitive ( 534 , 109 , 0 ) ;		/* the */
  primitive ( 404 , 71 , 0 ) ;	/* toks */
  primitive ( 349 , 38 , 0 ) ; /* vadjust */
  primitive ( 535 , 33 , 0 ) ;	/* valign */
  primitive ( 536 , 56 , 0 ) ;	/* vcenter */
  primitive ( 537 , 35 , 0 ) ;	/* vrule */
  primitive ( 594 , 13 , 256 ) ;	/* par */
  parloc = curval ; 
  partoken = 4095 + parloc ; 
  primitive ( 626 , 104 , 0 ) ;		/* input */
  primitive ( 627 , 104 , 1 ) ;		/* endinput */
  primitive ( 628 , 110 , 0 ) ;		/* topmark */
  primitive ( 629 , 110 , 1 ) ;		/* firstmark */
  primitive ( 630 , 110 , 2 ) ;		/* botmark */
  primitive ( 631 , 110 , 3 ) ;		/* splitfirstmark */
  primitive ( 632 , 110 , 4 ) ;		/* splitbotmark */
  primitive ( 473 , 89 , 0 ) ;	/* count */
  primitive ( 497 , 89 , 1 ) ;	/* dimen */
  primitive ( 392 , 89 , 2 ) ;	/* skip */
  primitive ( 393 , 89 , 3 ) ;	/* muskip */
  primitive ( 665 , 79 , 102 ) ;	/* spacefactor */
  primitive ( 666 , 79 , 1 ) ;	/* prevdepth */
  primitive ( 667 , 82 , 0 ) ;	/* deadcycles */
  primitive ( 668 , 82 , 1 ) ;	/* insertpenalties */
  primitive ( 669 , 83 , 1 ) ;	/* wd */
  primitive ( 670 , 83 , 3 ) ;	/* ht */
  primitive ( 671 , 83 , 2 ) ;	/* dp */
  primitive ( 672 , 70 , 0 ) ;	/* lastpenalty */
  primitive ( 673 , 70 , 1 ) ;	/* lastkern */
  primitive ( 674 , 70 , 2 ) ;	/* lastskip */
  primitive ( 675 , 70 , 3 ) ;	/* inputlineno */
  primitive ( 676 , 70 , 4 ) ;	/* badness */
  primitive ( 732 , 108 , 0 ) ;		/* number */
  primitive ( 733 , 108 , 1 ) ;		/* romannumeral */
  primitive ( 734 , 108 , 2 ) ;		/* string */
  primitive ( 735 , 108 , 3 ) ;		/* meaning */
  primitive ( 736 , 108 , 4 ) ;		/* fontname */
  primitive ( 737 , 108 , 5 ) ;		/* jobname */
  primitive ( 753 , 105 , 0 ) ;		/* if */
  primitive ( 754 , 105 , 1 ) ;		/* ifcat */
  primitive ( 755 , 105 , 2 ) ;		/* ifnum */
  primitive ( 756 , 105 , 3 ) ;		/* ifdim */
  primitive ( 757 , 105 , 4 ) ;		/* ifodd */
  primitive ( 758 , 105 , 5 ) ;		/* ifvmode */
  primitive ( 759 , 105 , 6 ) ;		/* ifhmode */
  primitive ( 760 , 105 , 7 ) ;		/* ifmmode */
  primitive ( 761 , 105 , 8 ) ;		/* ifinner */
  primitive ( 762 , 105 , 9 ) ;		/* ifvoid */
  primitive ( 763 , 105 , 10 ) ;	/* ifhbox */
  primitive ( 764 , 105 , 11 ) ;	/* ifvbox */
  primitive ( 765 , 105 , 12 ) ;	/* ifx */
  primitive ( 766 , 105 , 13 ) ;	/* ifeof */
  primitive ( 767 , 105 , 14 ) ;	/* iftrue */
  primitive ( 768 , 105 , 15 ) ;	/* iffalse */
  primitive ( 769 , 105 , 16 ) ;	/* ifcase */
  primitive ( 770 , 106 , 2 ) ;		/* fi */
/*  hash [ (hash_size + 518) ] .v.RH = 770 ; */ 
  hash [ (hash_size + hash_extra + 518) ] .v.RH = 770 ;		/* fi */
/*  eqtb [ (hash_size + 518) ] = eqtb [ curval ] ;  */
  eqtb [ (hash_size + hash_extra + 518) ] = eqtb [ curval ] ; 
  primitive ( 771 , 106 , 4 ) ;		/* or */
  primitive ( 772 , 106 , 3 ) ;		/* else */
  primitive ( 795 , 87 , 0 ) ;		/* nullfont */
/*  hash [ (hash_size + 524) ] .v.RH = 795 ; */	/* hash[frozen_null_font] */
  hash [ (hash_size + hash_extra + 524) ] .v.RH = 795 ;	/* nullfont */
/*  eqtb [ (hash_size + 524) ] = eqtb [ curval ] ;  */
  eqtb [ (hash_size + hash_extra + 524) ] = eqtb [ curval ] ; 
  primitive ( 892 , 4 , 256 ) ;		/* span */
		  /* primitive("span",tab_mark,span_code); */
  primitive ( 893 , 5 , 257 ) ;		/* cr */
		  /* primitive("cr",car_ret,cr_code); */
/*  hash [ (hash_size + 515) ] .v.RH = 893 ; */	
  hash [ (hash_size + hash_extra + 515) ] .v.RH = 893 ;		/* cr */
/*  eqtb [ (hash_size + 515) ] = eqtb [ curval ] ;  */
  eqtb [ (hash_size + hash_extra + 515) ] = eqtb [ curval ] ; 
  primitive ( 894 , 5 , 258 ) ;			/* cr cr */
/*  hash [ (hash_size + 519) ] .v.RH = 895 ;  */
  hash [ (hash_size + hash_extra + 519) ] .v.RH = 895 ; /* endtemplate */
/*  hash [ (hash_size + 520) ] .v.RH = 895 ;  */
  hash [ (hash_size + hash_extra + 520) ] .v.RH = 895 ; /* endtemplate */
/*  eqtb [ (hash_size + 520) ] .hh.b0 = 9 ;  */
  eqtb [ (hash_size + hash_extra + 520) ] .hh.b0 = 9 ; 
/*  eqtb [ (hash_size + 520) ] .hh .v.RH = memtop - 11 ;  */
  eqtb [ (hash_size + hash_extra + 520) ] .hh .v.RH = memtop - 11 ; 
/*  eqtb [ (hash_size + 520) ] .hh.b1 = 1 ;  */
  eqtb [ (hash_size + hash_extra + 520) ] .hh.b1 = 1 ; 
/*  eqtb [ (hash_size + 519) ] = eqtb [ (hash_size + 520) ] ;  */
  eqtb [ (hash_size + hash_extra + 519) ] =
		eqtb [ (hash_size + hash_extra + 520) ] ; 
/*  eqtb [ (hash_size + 519) ] .hh.b0 = 115 ;  */
  eqtb [ (hash_size + hash_extra + 519) ] .hh.b0 = 115 ; 
  primitive ( 964 , 81 , 0 ) ;	/* pagegoal */
  primitive ( 965 , 81 , 1 ) ;	/* pagetotal */
  primitive ( 966 , 81 , 2 ) ;	/* pagestretch */
  primitive ( 967 , 81 , 3 ) ;	/* pagefilstretch */
  primitive ( 968 , 81 , 4 ) ;	/* pagefillstretch */
  primitive ( 969 , 81 , 5 ) ;	/* pagefilllstretch */
  primitive ( 970 , 81 , 6 ) ;	/* pageshrink */
  primitive ( 971 , 81 , 7 ) ;	/* pagedepth */
  primitive ( 1019 , 14 , 0 ) ;		/* end */
  primitive ( 1020 , 14 , 1 ) ;		/* dump */
  primitive ( 1021 , 26 , 4 ) ;		/* hskip */
  primitive ( 1022 , 26 , 0 ) ;		/* hfil */
  primitive ( 1023 , 26 , 1 ) ;		/* hfill */
  primitive ( 1024 , 26 , 2 ) ;		/* hss */
  primitive ( 1025 , 26 , 3 ) ;		/* hfilneg */
  primitive ( 1026 , 27 , 4 ) ;		/* vskip */
  primitive ( 1027 , 27 , 0 ) ;		/* vfil */
  primitive ( 1028 , 27 , 1 ) ;		/* vfill */
  primitive ( 1029 , 27 , 2 ) ;		/* vss */
  primitive ( 1030 , 27 , 3 ) ;		/* vfilneg */
  primitive ( 333 , 28 , 5 ) ; /* mskip */
  primitive ( 337 , 29 , 1 ) ; /* kern */
  primitive ( 339 , 30 , 99 ) ; /* mkern */
  primitive ( 1048 , 21 , 1 ) ;		/* moveleft */
  primitive ( 1049 , 21 , 0 ) ;		/* moveright */
  primitive ( 1050 , 22 , 1 ) ;		/* raise */
  primitive ( 1051 , 22 , 0 ) ;		/* lower */
  primitive ( 406 , 20 , 0 ) ;	/* box */
  primitive ( 1052 , 20 , 1 ) ;		/* copy */
  primitive ( 1053 , 20 , 2 ) ;		/* lastbox */
  primitive ( 959 , 20 , 3 ) ;	/* vsplit */
  primitive ( 1054 , 20 , 4 ) ;		/* vtop */
  primitive ( 961 , 20 , 5 ) ;	/* vbox */
  primitive ( 1055 , 20 , 106 ) ;	/* hbox */
  primitive ( 1056 , 31 , 99 ) ;	/* shipout */
  primitive ( 1057 , 31 , 100 ) ;	/* leaders */
  primitive ( 1058 , 31 , 101 ) ;	/* cleaders */
  primitive ( 1059 , 31 , 102 ) ;	/* xleaders */
  primitive ( 1074 , 43 , 1 ) ;		/* indent */
  primitive ( 1075 , 43 , 0 ) ;		/* noindent */
  primitive ( 1084 , 25 , 12 ) ;	/* unpenalty */
  primitive ( 1085 , 25 , 11 ) ;	/* unkern */
  primitive ( 1086 , 25 , 10 ) ;	/* unskip */
  primitive ( 1087 , 23 , 0 ) ;		/* unhbox */
  primitive ( 1088 , 23 , 1 ) ;		/* unhcopy */
  primitive ( 1089 , 24 , 0 ) ;		/* unvbox */
  primitive ( 1090 , 24 , 1 ) ;		/* unvcopy */
  primitive ( 45 , 47 , 1 ) ;		/* - */
  primitive ( 346 , 47 , 0 ) ; /* discretionary */
  primitive ( 1121 , 48 , 0 ) ;		/* eqno */
  primitive ( 1122 , 48 , 1 ) ;		/* leqno */
  primitive ( 860 , 50 , 16 ) ;		/* mathord */
  primitive ( 861 , 50 , 17 ) ; /* mathop */
  primitive ( 862 , 50 , 18 ) ; /* mathbin */
  primitive ( 863 , 50 , 19 ) ; /* mathrel */
  primitive ( 864 , 50 , 20 ) ; /* mathopen */
  primitive ( 865 , 50 , 21 ) ; /* mathclose */
  primitive ( 866 , 50 , 22 ) ; /* mathpunct */
  primitive ( 867 , 50 , 23 ) ; /* mathinner */
  primitive ( 869 , 50 , 26 ) ; /* underline */
  primitive ( 868 , 50 , 27 ) ; /* overline */
  primitive ( 1123 , 51 , 0 ) ; /* displaylimits */
  primitive ( 872 , 51 , 1 ) ; /* limits */
  primitive ( 873 , 51 , 2 ) ; /* nolimits */
  primitive ( 855 , 53 , 0 ) ; /* displaystyle */
  primitive ( 856 , 53 , 2 ) ; /* textstyle */
  primitive ( 857 , 53 , 4 ) ; /* scriptstyle */
  primitive ( 858 , 53 , 6 ) ; /* scriptscriptstyle */
  primitive ( 1141 , 52 , 0 ) ; /* above */
  primitive ( 1142 , 52 , 1 ) ; /* over */
  primitive ( 1143 , 52 , 2 ) ; /* atop */
  primitive ( 1144 , 52 , 3 ) ; /* abovewithdelims */
  primitive ( 1145 , 52 , 4 ) ; /* overwithdelims */
  primitive ( 1146 , 52 , 5 ) ; /* atopwithdelims */
  primitive ( 870 , 49 , 30 ) ; /* left */
  primitive ( 871 , 49 , 31 ) ; /* right */
/*  hash [ (hash_size + 517) ] .v.RH = 871 ;  */
  hash [ (hash_size + hash_extra + 517) ] .v.RH = 871 ;		/* right */
/*  eqtb [ (hash_size + 517) ] = eqtb [ curval ] ;  */
  eqtb [ (hash_size + hash_extra + 517) ] = eqtb [ curval ] ; 
  primitive ( 1165 , 93 , 1 ) ; /* long */
  primitive ( 1166 , 93 , 2 ) ; /* outer */
  primitive ( 1167 , 93 , 4 ) ; /* global */
  primitive ( 1168 , 97 , 0 ) ; /* def */
  primitive ( 1169 , 97 , 1 ) ; /* gdef */
  primitive ( 1170 , 97 , 2 ) ; /* edef */
  primitive ( 1171 , 97 , 3 ) ; /* xdef */
  primitive ( 1185 , 94 , 0 ) ; /* let */
  primitive ( 1186 , 94 , 1 ) ; /* futurelet */
  primitive ( 1187 , 95 , 0 ) ; /* chardef */
  primitive ( 1188 , 95 , 1 ) ; /* mathchardef */
  primitive ( 1189 , 95 , 2 ) ; /* countdef */
  primitive ( 1190 , 95 , 3 ) ; /* dimendef */
  primitive ( 1191 , 95 , 4 ) ; /* skipdef */
  primitive ( 1192 , 95 , 5 ) ; /* muskipdef */
  primitive ( 1193 , 95 , 6 ) ; /* toksdef */
  primitive ( 412 , 85 , (hash_size + 1883) ) ; /* catcode */
  primitive ( 416 , 85 , (hash_size + 2907) ) ; /* mathcode */
  primitive ( 413 , 85 , (hash_size + 2139) ) ; /* lccode */
  primitive ( 414 , 85 , (hash_size + 2395) ) ; /* uccode */
  primitive ( 415 , 85 , (hash_size + 2651) ) ; /* sfcode */
  primitive ( 474 , 85 , (hash_size + 3474) ) ; /* delcode */
  primitive ( 409 , 86 , (hash_size + 1835) ) ; /* textfont */
  primitive ( 410 , 86 , (hash_size + 1851) ) ; /* scriptfont */
  primitive ( 411 , 86 , (hash_size + 1867) ) ; /* scriptscriptfont */
  primitive ( 935 , 99 , 0 ) ; /* hyphenation */
  primitive ( 947 , 99 , 1 ) ; /* patterns */
  primitive ( 1211 , 78 , 0 ) ; /* hyphenchar */
  primitive ( 1212 , 78 , 1 ) ; /* skewchar */
  primitive ( 272 , 100 , 0 ) ; /* batchmode */
  primitive ( 273 , 100 , 1 ) ; /* nonstopmode */
  primitive ( 274 , 100 , 2 ) ; /* scrollmode */
  primitive ( 1221 , 100 , 3 ) ; /* errorstopmode */
  primitive ( 1222 , 60 , 1 ) ; /* openin */
  primitive ( 1223 , 60 , 0 ) ; /* closein */
  primitive ( 1224 , 58 , 0 ) ; /* message */
  primitive ( 1225 , 58 , 1 ) ; /* errmessage */
  primitive ( 1231 , 57 , (hash_size + 2139) ) ; /* lowercase */
  primitive ( 1232 , 57 , (hash_size + 2395) ) ; /* uppercase */
  primitive ( 1233 , 19 , 0 ) ; /* show */
  primitive ( 1234 , 19 , 1 ) ; /* showbox */
  primitive ( 1235 , 19 , 2 ) ; /* showthe */
  primitive ( 1236 , 19 , 3 ) ; /* showlists */
  primitive ( 1279 , 59 , 0 ) ; /* openout */
  primitive ( 591 , 59 , 1 ) ; /* write */
  writeloc = curval ; 
  primitive ( 1280 , 59 , 2 ) ;		/* closeout */
  primitive ( 1281 , 59 , 3 ) ;		/* special */
  primitive ( 1282 , 59 , 4 ) ;		/* immediate */
  primitive ( 1283 , 59 , 5 ) ;		/* setlanguage */
  nonewcontrolsequence = true ; 
} 
#endif /* INITEX */

#pragma optimize("s", off)						/* 96/Sep/12 */
/* #pragma optimize("1", off) */
#pragma optimize("t", on)						/* 96/Sep/12 */
/* #pragma optimize("2", on) */

