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

/* #pragma optimize("a", off) */ 					/* 98/Dec/10 experiment */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void zshowbox ( p ) 
halfword p ; 
{showbox_regmem 
  depththreshold = eqtb [ (hash_size + 3188) ] .cint ; 
  breadthmax = eqtb [ (hash_size + 3187) ] .cint ; 
  if ( breadthmax <= 0 ) 
  breadthmax = 5 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
  if ( poolptr + depththreshold >= currentpoolsize )	/* ??? 93/Nov/28 */
	  strpool = reallocstrpool (incrementpoolsize);		/* ??? 94/Jan/24 */
  if ( poolptr + depththreshold >= currentpoolsize )	/* in case it failed */
	  depththreshold = currentpoolsize - poolptr - 1 ; 
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( poolptr + depththreshold >= poolsize ) 
	  depththreshold = poolsize - poolptr - 1 ; 
#endif
  shownodelist ( p ) ; 
  println () ; 
} 

void zdeletetokenref ( p ) 
halfword p ; 
{deletetokenref_regmem 
  if ( mem [ p ] .hh .v.LH == 0 ) 
  flushlist ( p ) ; 
  else decr ( mem [ p ] .hh .v.LH ) ; 
} 

void zdeleteglueref ( p ) 
halfword p ; 
{deleteglueref_regmem 
  if ( mem [ p ] .hh .v.RH == 0 ) 
  freenode ( p , 4 ) ; 
  else decr ( mem [ p ] .hh .v.RH ) ; 
} 

void zflushnodelist ( p ) 
halfword p ; 
{/* 30 */ flushnodelist_regmem 
  halfword q  ; 
  while ( p != 0 ) {			/* while p<>null */
      
    q = mem [ p ] .hh .v.RH ; 
    if ( ( p >= himemmin ) ) 
    {
      mem [ p ] .hh .v.RH = avail ; 
      avail = p ; 
	;
#ifdef STAT
      decr ( dynused ) ; 
#endif /* STAT */
    } 
    else {
	
      switch ( mem [ p ] .hh.b0 ) 
      {case 0 : 
      case 1 : 
      case 13 : 
	{
	  flushnodelist ( mem [ p + 5 ] .hh .v.RH ) ; 
	  freenode ( p , 7 ) ; 
	  goto lab30 ; 
	} 
	break ; 
      case 2 : 
	{
	  freenode ( p , 4 ) ; 
	  goto lab30 ; 
	} 
	break ; 
      case 3 : 
	{
	  flushnodelist ( mem [ p + 4 ] .hh .v.LH ) ; 
	  deleteglueref ( mem [ p + 4 ] .hh .v.RH ) ; 
	  freenode ( p , 5 ) ; 
	  goto lab30 ; 
	} 
	break ; 
      case 8 : 
	{
	  switch ( mem [ p ] .hh.b1 ) 
	  {case 0 : 
	    freenode ( p , 3 ) ; 
	    break ; 
	  case 1 : 
	  case 3 : 
	    {
	      deletetokenref ( mem [ p + 1 ] .hh .v.RH ) ; 
	      freenode ( p , 2 ) ; 
	      goto lab30 ; 
	    } 
	    break ; 
	  case 2 : 
	  case 4 : 
	    freenode ( p , 2 ) ; 
	    break ; 
	  default: 
		  {
			  confusion ( 1289 ) ;		/* ext3 */
			  return;					// abortflag set
		  }
		  break ; 
	  } 
	  goto lab30 ; 
	} 
	break ; 
      case 10 : 
	{
	  {
	    if ( mem [ mem [ p + 1 ] .hh .v.LH ] .hh .v.RH == 0 ) 
	    freenode ( mem [ p + 1 ] .hh .v.LH , 4 ) ; 
	    else decr ( mem [ mem [ p + 1 ] .hh .v.LH ] .hh .v.RH ) ; 
	  } 
/*     if leader_ptr(p)<>null then flush_node_list(leader_ptr(p)); */
	  if ( mem [ p + 1 ] .hh .v.RH != 0 ) 
	  flushnodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
	} 
	break ; 
      case 11 : 
      case 9 : 
      case 12 : 
	; 
	break ; 
      case 6 : 
	flushnodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
	break ; 
      case 4 : 
	deletetokenref ( mem [ p + 1 ] .cint ) ; 
	break ; 
      case 7 : 
	{
	  flushnodelist ( mem [ p + 1 ] .hh .v.LH ) ; 
	  flushnodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
	} 
	break ; 
      case 5 : 
	flushnodelist ( mem [ p + 1 ] .cint ) ; 
	break ; 
      case 14 : 
	{
	  freenode ( p , 3 ) ; 
	  goto lab30 ; 
	} 
	break ; 
      case 15 : 
	{
	  flushnodelist ( mem [ p + 1 ] .hh .v.LH ) ; 
	  flushnodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
	  flushnodelist ( mem [ p + 2 ] .hh .v.LH ) ; 
	  flushnodelist ( mem [ p + 2 ] .hh .v.RH ) ; 
	  freenode ( p , 3 ) ; 
	  goto lab30 ; 
	} 
	break ; 
      case 16 : 
      case 17 : 
      case 18 : 
      case 19 : 
      case 20 : 
      case 21 : 
      case 22 : 
      case 23 : 
      case 24 : 
      case 27 : 
      case 26 : 
      case 29 : 
      case 28 : 
	{
	  if ( mem [ p + 1 ] .hh .v.RH >= 2 ) 
	  flushnodelist ( mem [ p + 1 ] .hh .v.LH ) ; 
	  if ( mem [ p + 2 ] .hh .v.RH >= 2 ) 
	  flushnodelist ( mem [ p + 2 ] .hh .v.LH ) ; 
	  if ( mem [ p + 3 ] .hh .v.RH >= 2 ) 
	  flushnodelist ( mem [ p + 3 ] .hh .v.LH ) ; 
	  if ( mem [ p ] .hh.b0 == 24 ) 
	  freenode ( p , 5 ) ; 
	  else if ( mem [ p ] .hh.b0 == 28 ) 
	  freenode ( p , 5 ) ; 
	  else freenode ( p , 4 ) ; 
	  goto lab30 ; 
	} 
	break ; 
      case 30 : 
      case 31 : 
	{
	  freenode ( p , 4 ) ; 
	  goto lab30 ; 
	} 
	break ; 
      case 25 : 
	{
	  flushnodelist ( mem [ p + 2 ] .hh .v.LH ) ; 
	  flushnodelist ( mem [ p + 3 ] .hh .v.LH ) ; 
	  freenode ( p , 6 ) ; 
	  goto lab30 ; 
	} 
	break ; 
	  default: 
		  {
			  confusion ( 350 ) ;		/* flushing */
			  return;					// abortflag set
		  }
		  break ; 
      } 
      freenode ( p , 2 ) ; 
      lab30: ; 
    } 
    p = q ; 
  } 
} 

halfword zcopynodelist ( p ) 
halfword p ; 
{register halfword Result; copynodelist_regmem 
  halfword h  ; 
  halfword q  ; 
  halfword r  ; 
  char words  ; 
  h = getavail () ; 
  q = h ; 
  while ( p != 0 ) {			/* while p<>null do l.3969 */
      
    words = 1 ; 
    if ( ( p >= himemmin ) ) 
    r = getavail () ; 
    else switch ( mem [ p ] .hh.b0 ) 
    {case 0 : 
    case 1 : 
    case 13 : 
      {
	r = getnode ( 7 ) ; 
	mem [ r + 6 ] = mem [ p + 6 ] ; 
	mem [ r + 5 ] = mem [ p + 5 ] ; 
	mem [ r + 5 ] .hh .v.RH = copynodelist ( mem [ p + 5 ] .hh .v.RH ) ; 
	words = 5 ; 
      } 
      break ; 
    case 2 : 
      {
	r = getnode ( 4 ) ; 
	words = 4 ; 
      } 
      break ; 
    case 3 : 
      {
	r = getnode ( 5 ) ; 
	mem [ r + 4 ] = mem [ p + 4 ] ; 
	incr ( mem [ mem [ p + 4 ] .hh .v.RH ] .hh .v.RH ) ; 
	mem [ r + 4 ] .hh .v.LH = copynodelist ( mem [ p + 4 ] .hh .v.LH ) ; 
	words = 4 ; 
      } 
      break ; 
    case 8 : 
      switch ( mem [ p ] .hh.b1 ) 
      {case 0 : 
	{
	  r = getnode ( 3 ) ; 
	  words = 3 ; 
	} 
	break ; 
      case 1 : 
      case 3 : 
	{
	  r = getnode ( 2 ) ; 
	  incr ( mem [ mem [ p + 1 ] .hh .v.RH ] .hh .v.LH ) ; 
	  words = 2 ; 
	} 
	break ; 
      case 2 : 
      case 4 : 
	{
	  r = getnode ( 2 ) ; 
	  words = 2 ; 
	} 
	break ; 
	  default: 
		  {
			  confusion ( 1288 ) ;		/* ext2 */
			  return 0;					// abortflag set
		  }
		  break ; 
      } 
      break ; 
    case 10 : 
      {
	r = getnode ( 2 ) ; 
	incr ( mem [ mem [ p + 1 ] .hh .v.LH ] .hh .v.RH ) ; 
	mem [ r + 1 ] .hh .v.LH = mem [ p + 1 ] .hh .v.LH ; 
	mem [ r + 1 ] .hh .v.RH = copynodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
      } 
      break ; 
    case 11 : 
    case 9 : 
    case 12 : 
      {
	r = getnode ( 2 ) ; 
	words = 2 ; 
      } 
      break ; 
    case 6 : 
      {
	r = getnode ( 2 ) ; 
	mem [ r + 1 ] = mem [ p + 1 ] ; 
	mem [ r + 1 ] .hh .v.RH = copynodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
      } 
      break ; 
    case 7 : 
      {
	r = getnode ( 2 ) ; 
	mem [ r + 1 ] .hh .v.LH = copynodelist ( mem [ p + 1 ] .hh .v.LH ) ; 
	mem [ r + 1 ] .hh .v.RH = copynodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
      } 
      break ; 
    case 4 : 
      {
	r = getnode ( 2 ) ; 
	incr ( mem [ mem [ p + 1 ] .cint ] .hh .v.LH ) ; 
	words = 2 ; 
      } 
      break ; 
    case 5 : 
      {
	r = getnode ( 2 ) ; 
	mem [ r + 1 ] .cint = copynodelist ( mem [ p + 1 ] .cint ) ; 
      } 
      break ; 
	default: 
		{
			confusion ( 351 ) ;		/* copying */
			return 0;				// abortflag set
		}
		break ; 
    } 
    while ( words > 0 ) {
      decr ( words ) ; 
      mem [ r + words ] = mem [ p + words ] ; /* r may be used without having ... */
    } 
    mem [ q ] .hh .v.RH = r ; 
    q = r ; 
    p = mem [ p ] .hh .v.RH ; 
  } 
  mem [ q ] .hh .v.RH = 0 ; 
  q = mem [ h ] .hh .v.RH ; 
  {
    mem [ h ] .hh .v.RH = avail ; 
    avail = h ; 
	;
#ifdef STAT
    decr ( dynused ) ; 
#endif /* STAT */
  } 
  Result = q ; 
  return Result ; 
} 

void zprintmode ( m ) 
integer m ; 
{printmode_regmem 
  if ( m > 0 ) 
  switch ( m / ( 101 ) ) 
  {case 0 : 
    print ( 352 ) ;		/* vertical */
    break ; 
  case 1 : 
    print ( 353 ) ;		/* horizontal */
    break ; 
  case 2 : 
    print ( 354 ) ;		/* display math */
    break ; 
  } 
  else if ( m == 0 ) 
  print ( 355 ) ;		/* no */
  else switch ( ( - (integer) m ) / ( 101 ) ) 
  {case 0 : 
    print ( 356 ) ;		/* internal vertical */
    break ; 
  case 1 : 
    print ( 357 ) ;		/* restricted horizontal */
    break ; 
  case 2 : 
    print ( 340 ) ;		/* math */
    break ; 
  } 
  print ( 358 ) ;		/*  mode */
} 

void pushnest ( ) 
{pushnest_regmem 
  if ( nestptr > maxneststack ) 
  {
    maxneststack = nestptr ; 
#ifdef ALLOCATEINPUTSTACK
	if ( nestptr == currentnestsize )
		nest = reallocneststack (incrementnestsize);
	if ( nestptr == currentnestsize ) {	/* check again after allocation */
		overflow ( 359 , currentnestsize ) ;
		return;			// abortflag set
	}
#else
	if ( nestptr == nestsize ) {
		overflow ( 359 , nestsize ) ;	/* semantic next size - not dynamic */
		return;			// abortflag set
	}
#endif
  } 
  nest [ nestptr ] = curlist ; 
  incr ( nestptr ) ; 
  curlist .headfield = getavail () ; 
  curlist .tailfield = curlist .headfield ; 
  curlist .pgfield = 0 ; 
  curlist .mlfield = line ; 
} 

void popnest ( ) 
{popnest_regmem 
  {
    mem [ curlist .headfield ] .hh .v.RH = avail ; 
    avail = curlist .headfield ; 
	;
#ifdef STAT
    decr ( dynused ) ; 
#endif /* STAT */
  } 
  decr ( nestptr ) ; 
  curlist = nest [ nestptr ] ; 
} 

void showactivities ( ) 
{showactivities_regmem 
  integer p  ; 
  short m  ; 
  memoryword a  ; 
  halfword q, r  ; 
  integer t  ; 
  nest [ nestptr ] = curlist ; 
  printnl ( 335 ) ;		/*  */
  println () ; 
  {
	  register integer for_end; 
	  p = nestptr ; 
	  for_end = 0 ; 
	  if ( p >= for_end) do 
		  {
			  m = nest [ p ] .modefield ; 
			  a = nest [ p ] .auxfield ; 
			  printnl ( 360 ) ;		/* ###  */
			  printmode ( m ) ;
			  print ( 361 ) ;		/*  entered at line  */
			  printint ( abs ( nest [ p ] .mlfield ) ) ; 
			  if ( m == 102 ) 
/* ************************************************************************ */
/* major change from 3.141 -> 3.14159 in following */
/* .pgfield instead of .lhmfield and .rhmfield */
/* WAS if ( ( nest [ p ] .lhmfield != 2 ) || ( nest [ p ] .rhmfield != 3 ) ) */
      if ( nest [ p ] .pgfield != 8585216L )	/* 830000 hex ??? */
      {
	print ( 362 ) ;		/*  (language */
	printint ( nest [ p ] .pgfield % 65536L ) ;  	/* last 16 bits */
/*	printint ( nest [ p ] .pgfield & 65535L ) ;  */
	print ( 363 ) ;		/* :hyphenmin */
	printint ( nest [ p ] .pgfield / 4194304L ) ; 	/* 400000 hex ??? */
/*	printint ( nest [ p ] .pgfield >> 22 ) ; */	/* top 10 bits */
	printchar ( 44 ) ;	/* , */
	printint ( ( nest [ p ] .pgfield / 65536L ) % 64 ) ; 
/*	printint ( ( nest [ p ] .pgfield >> 16 ) & 63 ) ; */ /* next 8 bits */
/*  this used to refer to .lhmfield and .rhmfield ... */
/* ********************************************************************* */
	printchar ( 41 ) ;	/* ) */
      } 
      if ( nest [ p ] .mlfield < 0 ) 
      print ( 364 ) ;			/*  (\output routine) */
      if ( p == 0 ) 
      {
	if ( memtop - 2 != pagetail ) 
	{
	  printnl ( 974 ) ;			/* ### current page: */
	  if ( outputactive ) 
		  print ( 975 ) ;		/*  (held over for next output) */
	  showbox ( mem [ memtop - 2 ] .hh .v.RH ) ; 
	  if ( pagecontents > 0 ) 
	  {
	    printnl ( 976 ) ;		/* total height  */
	    printtotals () ; 
	    printnl ( 977 ) ;		/*  goal height  */
	    printscaled ( pagesofar [ 0 ] ) ; 
	    r = mem [ memtop ] .hh .v.RH ; 
	    while ( r != memtop ) {
		
	      println () ; 
	      printesc ( 327 ) ;	/* insert */
	      t = mem [ r ] .hh.b1 ; 
	      printint ( t ) ; 
	      print ( 978 ) ;		/*  adds  */
	      t = xovern ( mem [ r + 3 ] .cint , 1000 ) *
			  eqtb [ (hash_size + 3218) + t ] .cint ; 
	      printscaled ( t ) ; 
	      if ( mem [ r ] .hh.b0 == 1 ) 
	      {
		q = memtop - 2 ; 
		t = 0 ; 
		do {
		    q = mem [ q ] .hh .v.RH ; 
		  if ( ( mem [ q ] .hh.b0 == 3 ) && ( mem [ q ] .hh.b1 == mem 
		  [ r ] .hh.b1 ) ) 
		  incr ( t ) ; 
		} while ( ! ( q == mem [ r + 1 ] .hh .v.LH ) ) ; 
		print ( 979 ) ;			/* , # */
		printint ( t ) ; 
		print ( 980 ) ;			/*  might split */
	      } 
	      r = mem [ r ] .hh .v.RH ; 
	    } 
	  } 
	} 
/*  if link(contrib_head)<>null then l.4393 */
	if ( mem [ memtop - 1 ] .hh .v.RH != 0 ) 
	printnl ( 365 ) ;		/*  (\output routine) */
      } 
      showbox ( mem [ nest [ p ] .headfield ] .hh .v.RH ) ; 
      switch ( abs ( m ) / ( 101 ) ) 
      {case 0 : 
	{
	  printnl ( 366 ) ;			/* ### recent contributions: */
	  if ( a .cint <= -65536000L ) 
	  print ( 367 ) ;			/* ignored */
	  else printscaled ( a .cint ) ; 
	  if ( nest [ p ] .pgfield != 0 ) 
	  {
	    print ( 368 ) ;			/* , prevgraf  */
	    printint ( nest [ p ] .pgfield ) ; 
	    print ( 369 ) ;			/*  line */
	    if ( nest [ p ] .pgfield != 1 ) 
	    printchar ( 115 ) ;		/* s */
	  } 
	} 
	break ; 
      case 1 : 
	{
	  printnl ( 370 ) ;			/* spacefactor  */
	  printint ( a .hh .v.LH ) ; 
	  if ( m > 0 ) 
	  if ( a .hh .v.RH > 0 ) 
	  {
	    print ( 371 ) ;			/* , current language  */
	    printint ( a .hh .v.RH ) ; 
	  } 
	} 
	break ; 
      case 2 : 
	if ( a .cint != 0 ) 
	{
	  print ( 372 ) ;			/* this will be denominator of: */
	  showbox ( a .cint ) ; 
	} 
	break ; 
      } 
    } 
  while ( p-- > for_end ) ; } 
} 

void zprintparam ( n ) 
integer n ; 
{printparam_regmem 
  switch ( n ) 
  {case 0 : 
    printesc ( 417 ) ;		/* pretolerance */
    break ; 
  case 1 : 
    printesc ( 418 ) ;		/* tolerance */
    break ; 
  case 2 : 
    printesc ( 419 ) ;		/* linepenalty */
    break ; 
  case 3 : 
    printesc ( 420 ) ;		/* hyphenpenalty */
    break ; 
  case 4 : 
    printesc ( 421 ) ;		/* exhyphenpenalty */
    break ; 
  case 5 : 
    printesc ( 422 ) ;		/* clubpenalty */
    break ; 
  case 6 : 
    printesc ( 423 ) ;		/* widowpenalty */
    break ; 
  case 7 : 
    printesc ( 424 ) ;		/* displaywidowpenalty */
    break ; 
  case 8 : 
    printesc ( 425 ) ;		/* brokenpenalty */
    break ; 
  case 9 : 
    printesc ( 426 ) ;		/* binoppenalty */
    break ; 
  case 10 : 
    printesc ( 427 ) ;		/* relpenalty */
    break ; 
  case 11 : 
    printesc ( 428 ) ;		/* predisplaypenalty */
    break ; 
  case 12 : 
    printesc ( 429 ) ;		/* postdisplaypenalty */
    break ; 
  case 13 : 
    printesc ( 430 ) ;		/* interlinepenalty */
    break ; 
  case 14 : 
    printesc ( 431 ) ;		/* doublehyphendemerits */
    break ; 
  case 15 : 
    printesc ( 432 ) ;		/* finalhyphendemerits */
    break ; 
  case 16 : 
    printesc ( 433 ) ;		/* adjdemerits */
    break ; 
  case 17 : 
    printesc ( 434 ) ;		/* mag */
    break ; 
  case 18 : 
    printesc ( 435 ) ;		/* delimiterfactor */
    break ; 
  case 19 : 
    printesc ( 436 ) ;		/* looseness */
    break ; 
  case 20 : 
    printesc ( 437 ) ;		/* time */
    break ; 
  case 21 : 
    printesc ( 438 ) ;		/* day */
    break ; 
  case 22 : 
    printesc ( 439 ) ;		/* month */
    break ; 
  case 23 : 
    printesc ( 440 ) ;		/* year */
    break ; 
  case 24 : 
    printesc ( 441 ) ;		/* showboxbreadth */
    break ; 
  case 25 : 
    printesc ( 442 ) ;		/* showboxdepth */
    break ; 
  case 26 : 
    printesc ( 443 ) ;		/* hbadness */
    break ; 
  case 27 : 
    printesc ( 444 ) ;		/* vbadness */
    break ; 
  case 28 : 
    printesc ( 445 ) ;		/* pausing */
    break ; 
  case 29 : 
    printesc ( 446 ) ;		/* tracingonline */
    break ; 
  case 30 : 
    printesc ( 447 ) ;		/* tracingmacros */
    break ; 
  case 31 : 
    printesc ( 448 ) ;		/* tracingstats */
    break ; 
  case 32 : 
    printesc ( 449 ) ;		/* tracingparagraphs */
    break ; 
  case 33 : 
    printesc ( 450 ) ;		/* tracingpages */
    break ; 
  case 34 : 
    printesc ( 451 ) ;		/* tracingoutput */
    break ; 
  case 35 : 
    printesc ( 452 ) ;		/* tracinglostchars */
    break ; 
  case 36 : 
    printesc ( 453 ) ;		/* tracingcommands */
    break ; 
  case 37 : 
    printesc ( 454 ) ;		/* tracingrestores */
    break ; 
  case 38 : 
    printesc ( 455 ) ;		/* uchyph */
    break ; 
  case 39 : 
    printesc ( 456 ) ;		/* outputpenalty */
    break ; 
  case 40 : 
    printesc ( 457 ) ;		/* maxdeadcycles */
    break ; 
  case 41 : 
    printesc ( 458 ) ;		/* hangafter */
    break ; 
  case 42 : 
    printesc ( 459 ) ;		/* floatingpenalty */
    break ; 
  case 43 : 
    printesc ( 460 ) ;		/* globaldefs */
    break ; 
  case 44 : 
    printesc ( 461 ) ;		/* fam */
    break ; 
  case 45 : 
    printesc ( 462 ) ;		/* escapechar */
    break ; 
  case 46 : 
    printesc ( 463 ) ;		/* defaulthyphenchar */
    break ; 
  case 47 : 
    printesc ( 464 ) ;		/* defaultskewchar */
    break ; 
  case 48 : 
    printesc ( 465 ) ;		/* endlinechar */
    break ; 
  case 49 : 
    printesc ( 466 ) ;		/* newlinechar */
    break ; 
  case 50 : 
    printesc ( 467 ) ;		/* language */
    break ; 
  case 51 : 
    printesc ( 468 ) ;		/* lefthyphenmin */
    break ; 
  case 52 : 
    printesc ( 469 ) ;		/* righthyphenmin */
    break ; 
  case 53 : 
    printesc ( 470 ) ;		/* holdinginserts */
    break ; 
  case 54 : 
    printesc ( 471 ) ;		/* errorcontextlines */
    break ; 
  default: 
    print ( 472 ) ;			/* [unknown integer parameter!] */
    break ; 
  } 
} 

void begindiagnostic ( ) 
{begindiagnostic_regmem 
    oldsetting = selector ; 
	if ( ( eqtb [ (hash_size + 3192) ] .cint <= 0 ) && ( selector == 19 ) ) {
		decr ( selector ) ; 
		if ( history == 0 ) history = 1 ; 
	} 
} 

void zenddiagnostic ( blankline ) 
booleane blankline ; 
{enddiagnostic_regmem 
    printnl ( 335 ) ;		/*  */
	if ( blankline ) println () ; 
	selector = oldsetting ; 
} 

void zprintlengthparam ( n ) 
integer n ; 
{printlengthparam_regmem 
  switch ( n ) 
  {case 0 : 
    printesc ( 475 ) ;		/* parindent */
    break ; 
  case 1 : 
    printesc ( 476 ) ;		/* mathsurround */
    break ; 
  case 2 : 
    printesc ( 477 ) ;		/* lineskiplimit */
    break ; 
  case 3 : 
    printesc ( 478 ) ;		/* hsize */
    break ; 
  case 4 : 
    printesc ( 479 ) ;		/* vsize */
    break ; 
  case 5 : 
    printesc ( 480 ) ;		/* maxdepth */
    break ; 
  case 6 : 
    printesc ( 481 ) ;		/* splitmaxdepth */
    break ; 
  case 7 : 
    printesc ( 482 ) ;		/* boxmaxdepth */
    break ; 
  case 8 : 
    printesc ( 483 ) ;		/* hfuzz */
    break ; 
  case 9 : 
    printesc ( 484 ) ;		/* vfuzz */
    break ; 
  case 10 : 
    printesc ( 485 ) ;		/* delimitershortfall */
    break ; 
  case 11 : 
    printesc ( 486 ) ;		/* nulldelimiterspace */
    break ; 
  case 12 : 
    printesc ( 487 ) ;		/* scriptspace */
    break ; 
  case 13 : 
    printesc ( 488 ) ;		/* predisplaysize */
    break ; 
  case 14 : 
    printesc ( 489 ) ;		/* displaywidth */
    break ; 
  case 15 : 
    printesc ( 490 ) ;		/* displayindent */
    break ; 
  case 16 : 
    printesc ( 491 ) ;		/* overfullrule */
    break ; 
  case 17 : 
    printesc ( 492 ) ;		/* hangindent */
    break ; 
  case 18 : 
    printesc ( 493 ) ;		/* hoffset */
    break ; 
  case 19 : 
    printesc ( 494 ) ;		/* voffset */
    break ; 
  case 20 : 
    printesc ( 495 ) ;		/* emergencystretch */
    break ; 
  default: 
    print ( 496 ) ;			/* [unknown dimen parameter!] */
    break ; 
  } 
} 

void zprintcmdchr ( cmd , chrcode ) 
quarterword cmd ; 
halfword chrcode ; 
{printcmdchr_regmem 
  switch ( cmd ) 
  {case 1 : 
    {
      print ( 554 ) ;		/* begin-group character  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 2 : 
    {
      print ( 555 ) ;		/* end-group character  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 3 : 
    {
      print ( 556 ) ;		/* math shift character  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 6 : 
    {
      print ( 557 ) ;		/* macro parameter character  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 7 : 
    {
      print ( 558 ) ;		/* superscript character  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 8 : 
    {
      print ( 559 ) ;		/* subscript character  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 9 : 
    print ( 560 ) ;			/* end of alignment template */
    break ; 
  case 10 : 
    {
      print ( 561 ) ;		/* blank space  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 11 : 
    {
      print ( 562 ) ;		/* the letter  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 12 : 
    {
      print ( 563 ) ;		/* the character  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 75 : 
  case 76 : 
/* if chr_code<skip_base then print_skip_param(chr_code-glue_base) */
    if ( chrcode < (hash_size + 800) ) 
		printskipparam ( chrcode - (hash_size + 782) ) ;	/* lineskip */
/* else if chr_code<mu_skip_base then
    begin print_esc("skip"); print_int(chr_code-skip_base); */
    else if ( chrcode < (hash_size + 1056) ) 
    {
      printesc ( 392 ) ;		/* skip */
      printint ( chrcode - (hash_size + 800) ) ; 
    } 
    else {
/*   else  begin print_esc("muskip"); print_int(chr_code-mu_skip_base); */  
      printesc ( 393 ) ;		/* muskip */
      printint ( chrcode - (hash_size + 1056) ) ; 
    } 
    break ; 
  case 72 : 
    if ( chrcode >= (hash_size + 1322) ) 
    {
      printesc ( 404 ) ;		/* toks */
      printint ( chrcode - (hash_size + 1322) ) ; 
    } 
    else switch ( chrcode ) 
    {
	case (hash_size + 1313) : 
      printesc ( 395 ) ;		/* output */
      break ; 
    case (hash_size + 1314) : 
      printesc ( 396 ) ;		/* everypar */
      break ; 
    case (hash_size + 1315) : 
      printesc ( 397 ) ;		/* everymath */
      break ; 
    case (hash_size + 1316) : 
      printesc ( 398 ) ;		/* everydisplay */
      break ; 
    case (hash_size + 1317) : 
      printesc ( 399 ) ;		/* everyhbox */
      break ; 
    case (hash_size + 1318) : 
      printesc ( 400 ) ;		/* everyvbox */
      break ; 
    case (hash_size + 1319) : 
      printesc ( 401 ) ;		/* everyjob */
      break ; 
    case (hash_size + 1320) : 
      printesc ( 402 ) ;		/* everycr */
      break ; 
	default: 
      printesc ( 403 ) ;		/* errhelp */
      break ; 
    } 
    break ; 
  case 73 : 
    if ( chrcode < (hash_size + 3218) ) 
    printparam ( chrcode - (hash_size + 3163) ) ; 
    else {
      printesc ( 473 ) ;		/* count */
      printint ( chrcode - (hash_size + 3218) ) ; 
    } 
    break ; 
  case 74 : 
    if ( chrcode < (hash_size + 3751) ) 
    printlengthparam ( chrcode - (hash_size + 3730) ) ; 
    else {
	
      printesc ( 497 ) ;		/* dimen */
      printint ( chrcode - (hash_size + 3751) ) ; 
    } 
    break ; 
  case 45 : 
    printesc ( 505 ) ;		/* accent */
    break ; 
  case 90 : 
    printesc ( 506 ) ;		/* advance */
    break ; 
  case 40 : 
    printesc ( 507 ) ;		/* afterassignment */
    break ; 
  case 41 : 
    printesc ( 508 ) ;		/* aftergroup */
    break ; 
  case 77 : 
    printesc ( 516 ) ;		/* fontdimen */
    break ; 
  case 61 : 
    printesc ( 509 ) ;		/* begingroup */
    break ; 
  case 42 : 
    printesc ( 528 ) ;		/* penalty */
    break ; 
  case 16 : 
    printesc ( 510 ) ;		/* char */
    break ; 
  case 107 : 
    printesc ( 501 ) ;		/* csname */
    break ; 
  case 88 : 
    printesc ( 515 ) ;		/* font */
    break ; 
  case 15 : 
    printesc ( 511 ) ;		/* delimiter */
    break ; 
  case 92 : 
    printesc ( 512 ) ;		/* divide */
    break ; 
  case 67 : 
    printesc ( 502 ) ;		/* endcsname */
    break ; 
  case 62 : 
    printesc ( 513 ) ;		/* endgroup */
    break ; 
  case 64 : 
    printesc ( 32 ) ;		/*   */
    break ; 
  case 102 : 
    printesc ( 514 ) ;		/* expandafter */
    break ; 
  case 32 : 
    printesc ( 517 ) ;		/* halign */
    break ; 
  case 36 : 
    printesc ( 518 ) ;		/* hrule */
    break ; 
  case 39 : 
    printesc ( 519 ) ;		/* ignorespaces */
    break ; 
  case 37 : 
    printesc ( 327 ) ;		/* insert */
    break ; 
  case 44 : 
    printesc ( 47 ) ;		/* / */
    break ; 
  case 18 : 
    printesc ( 348 ) ;		/* mark */
    break ; 
  case 46 : 
    printesc ( 520 ) ;		/* mathaccent */
    break ; 
  case 17 : 
    printesc ( 521 ) ;		/* mathchar */
    break ; 
  case 54 : 
    printesc ( 522 ) ;		/* mathchoice */
    break ; 
  case 91 : 
    printesc ( 523 ) ;		/* multiply */
    break ; 
  case 34 : 
    printesc ( 524 ) ;		/* noalign */
    break ; 
  case 65 : 
    printesc ( 525 ) ;		/* noboundary */
    break ; 
  case 103 : 
    printesc ( 526 ) ;		/* noexpand */
    break ; 
  case 55 : 
    printesc ( 332 ) ;		/* nonscript */
    break ; 
  case 63 : 
    printesc ( 527 ) ;		/* omit */
    break ; 
  case 66 : 
    printesc ( 530 ) ;		/* radical */
    break ; 
  case 96 : 
    printesc ( 531 ) ;		/* read */
    break ; 
  case 0 : 
    printesc ( 532 ) ;		/* relax */
    break ; 
  case 98 : 
    printesc ( 533 ) ;		/* setbox */
    break ; 
  case 80 : 
    printesc ( 529 ) ;		/* prevgraf */
    break ; 
  case 84 : 
    printesc ( 405 ) ;		/* parshape */
    break ; 
  case 109 : 
    printesc ( 534 ) ;		/* the */
    break ; 
  case 71 : 
    printesc ( 404 ) ;		/* toks */
    break ; 
  case 38 : 
	 printesc ( 349 ) ;		/* vadjust */
    break ; 
  case 33 : 
    printesc ( 535 ) ;		/* valign */
    break ; 
  case 56 : 
    printesc ( 536 ) ;		/* vcenter */
    break ; 
  case 35 : 
    printesc ( 537 ) ;		/* vrule */
    break ; 
  case 13 : 
    printesc ( 594 ) ;		/* par */
    break ; 
  case 104 : 
    if ( chrcode == 0 ) 
		printesc ( 626 ) ;		/* input */
    else printesc ( 627 ) ;		/* endinput */
    break ; 
  case 110 : 
    switch ( chrcode ) 
    {case 1 : 
      printesc ( 629 ) ;		/* firstmark */
      break ; 
    case 2 : 
      printesc ( 630 ) ;		/* botmark */
      break ; 
    case 3 : 
      printesc ( 631 ) ;		/* splitfirstmark */
      break ; 
    case 4 : 
      printesc ( 632 ) ;		/* splitbotmark */
      break ; 
	default: 
      printesc ( 628 ) ;		/* topmark */
      break ; 
    } 
    break ; 
  case 89 : 
    if ( chrcode == 0 ) 
		printesc ( 473 ) ;		/* count */
    else if ( chrcode == 1 ) 
		printesc ( 497 ) ;		/* dimen */
    else if ( chrcode == 2 ) 
		printesc ( 392 ) ;		/* skip */
    else printesc ( 393 ) ;		/* muskip */
    break ; 
  case 79 : 
    if ( chrcode == 1 ) 
		printesc ( 666 ) ;		/* prevdepth */
    else printesc ( 665 ) ;		/* spacefactor */
    break ; 
  case 82 : 
    if ( chrcode == 0 ) 
		printesc ( 667 ) ;		/* deadcycles */
    else printesc ( 668 ) ;		/* insertpenalties */
    break ; 
  case 83 : 
    if ( chrcode == 1 ) 
		printesc ( 669 ) ;		/* wd */
    else if ( chrcode == 3 ) 
		printesc ( 670 ) ;		/* ht */
    else printesc ( 671 ) ;		/* dp */
    break ; 
  case 70 : 
    switch ( chrcode ) 
    {case 0 : 
      printesc ( 672 ) ;		/* lastpenalty */
      break ; 
    case 1 : 
      printesc ( 673 ) ;		/* lastkern */
      break ; 
    case 2 : 
      printesc ( 674 ) ;		/* lastskip */
      break ; 
    case 3 : 
      printesc ( 675 ) ;		/* inputlineno */
      break ; 
	default: 
      printesc ( 676 ) ;		/* badness */
      break ; 
    } 
    break ; 
  case 108 : 
    switch ( chrcode ) 
    {case 0 : 
      printesc ( 732 ) ;		/* number */
      break ; 
    case 1 : 
      printesc ( 733 ) ;		/* romannumeral */
      break ; 
    case 2 : 
      printesc ( 734 ) ;		/* string */
      break ; 
    case 3 : 
      printesc ( 735 ) ;		/* meaning */
      break ; 
    case 4 : 
      printesc ( 736 ) ;		/* fontname */
      break ; 
    default: 
      printesc ( 737 ) ;		/* jobname */
      break ; 
    } 
    break ; 
  case 105 : 
    switch ( chrcode ) 
    {case 1 : 
      printesc ( 754 ) ;		/* ifcat */
      break ; 
    case 2 : 
      printesc ( 755 ) ;		/* ifnum */
      break ; 
    case 3 : 
      printesc ( 756 ) ;		/* ifdim */
      break ; 
    case 4 : 
      printesc ( 757 ) ;		/* ifodd */
      break ; 
    case 5 : 
      printesc ( 758 ) ;		/* ifvmode */
      break ; 
    case 6 : 
      printesc ( 759 ) ;		/* ifhmode */
      break ; 
    case 7 : 
      printesc ( 760 ) ;		/* ifmmode */
      break ; 
    case 8 : 
      printesc ( 761 ) ;		/* ifinner */
      break ; 
    case 9 : 
      printesc ( 762 ) ;		/* ifvoid */
      break ; 
    case 10 : 
      printesc ( 763 ) ;		/* ifhbox */
      break ; 
    case 11 : 
      printesc ( 764 ) ;		/* ifvbox */
      break ; 
    case 12 : 
      printesc ( 765 ) ;		/* ifx */
      break ; 
    case 13 : 
      printesc ( 766 ) ;		/* ifeof */
      break ; 
    case 14 : 
      printesc ( 767 ) ;		/* iftrue */
      break ; 
    case 15 : 
      printesc ( 768 ) ;		/* iffalse */
      break ; 
    case 16 : 
      printesc ( 769 ) ;		/* ifcase */
      break ; 
	default: 
      printesc ( 753 ) ;		/* if */
      break ; 
    } 
    break ; 
  case 106 : 
    if ( chrcode == 2 ) 
		printesc ( 770 ) ;		/* fi */
    else if ( chrcode == 4 ) 
		printesc ( 771 ) ;		/* or */
    else printesc ( 772 ) ;		/* else */
    break ; 
  case 4 : 
    if ( chrcode == 256 )	/* pool size */ /* maxquarterword + 1 ? */
    printesc ( 892 ) ;		/* span */
    else {
      print ( 896 ) ;		/* alignment tab character  */
      print ( chrcode ) ; 
    } 
    break ; 
  case 5 : 
    if ( chrcode == 257 )		/* cr_code */
		printesc ( 893 ) ;		/* cr */
    else printesc ( 894 ) ;		/* crcr */
    break ; 
  case 81 : 
    switch ( chrcode ) 
    {case 0 : 
      printesc ( 964 ) ;		/* pagegoal */
      break ; 
    case 1 : 
      printesc ( 965 ) ;		/* pagetotal */
      break ; 
    case 2 : 
      printesc ( 966 ) ;		/* pagestretch */
      break ; 
    case 3 : 
      printesc ( 967 ) ;		/* pagefilstretch */
      break ; 
    case 4 : 
      printesc ( 968 ) ;		/* pagefillstretch */
      break ; 
    case 5 : 
      printesc ( 969 ) ;		/* pagefilllstretch */
      break ; 
    case 6 : 
      printesc ( 970 ) ;		/* pageshrink */
      break ; 
	default: 
      printesc ( 971 ) ;		/* pagedepth */
      break ; 
    } 
    break ; 
  case 14 : 
    if ( chrcode == 1 ) 
		printesc ( 1020 ) ;			/* dump */
    else printesc ( 1019 ) ;		/* end */
    break ; 
  case 26 : 
    switch ( chrcode ) 
    {case 4 : 
      printesc ( 1021 ) ;		/* hskip */
      break ; 
    case 0 : 
      printesc ( 1022 ) ;		/* hfil */
      break ; 
    case 1 : 
      printesc ( 1023 ) ;		/* hfill */
      break ; 
    case 2 : 
      printesc ( 1024 ) ;		/* hss */
      break ; 
	default: 
      printesc ( 1025 ) ;		/* hfilneg */
      break ; 
    } 
    break ; 
  case 27 : 
    switch ( chrcode ) 
    {case 4 : 
      printesc ( 1026 ) ;		/* vskip */
      break ; 
    case 0 : 
      printesc ( 1027 ) ;		/* vfil */
      break ; 
    case 1 : 
      printesc ( 1028 ) ;		/* vfill */
      break ; 
    case 2 : 
      printesc ( 1029 ) ;		/* vss */
      break ; 
	default: 
      printesc ( 1030 ) ;		/* vfilneg */
      break ; 
    } 
    break ; 
  case 28 : 
    printesc ( 333 ) ;			/* mskip */
    break ; 
  case 29 : 
    printesc ( 337 ) ;			/* kern */
    break ; 
  case 30 : 
    printesc ( 339 ) ;			/* mkern */
    break ; 
  case 21 : 
    if ( chrcode == 1 ) 
		printesc ( 1048 ) ;			/* moveleft */
    else printesc ( 1049 ) ;		/* moveright */
    break ; 
  case 22 : 
    if ( chrcode == 1 ) 
		printesc ( 1050 ) ;			/* raise */
    else printesc ( 1051 ) ;		/* lower */
    break ; 
  case 20 : 
    switch ( chrcode ) 
    {case 0 : 
      printesc ( 406 ) ;		/* box */
      break ; 
    case 1 : 
      printesc ( 1052 ) ;		/* copy */
      break ; 
    case 2 : 
      printesc ( 1053 ) ;		/* lastbox */
      break ; 
    case 3 : 
      printesc ( 959 ) ;		/* vsplit */
      break ; 
    case 4 : 
      printesc ( 1054 ) ;		/* vtop */
      break ; 
    case 5 : 
      printesc ( 961 ) ;		/* vbox */
      break ; 
	default: 
      printesc ( 1055 ) ;		/* hbox */
      break ; 
    } 
    break ; 
  case 31 : 
    if ( chrcode == 100 ) 
		printesc ( 1057 ) ;			/* leaders */
    else if ( chrcode == 101 ) 
		printesc ( 1058 ) ;			/* cleaders */
    else if ( chrcode == 102 ) 
		printesc ( 1059 ) ;			/* xleaders */
    else printesc ( 1056 ) ;		/* shipout */
    break ; 
  case 43 : 
    if ( chrcode == 0 ) 
		printesc ( 1075 ) ;			/* noindent */
    else printesc ( 1074 ) ;		/* indent */
    break ; 
  case 25 : 
    if ( chrcode == 10 ) 
		printesc ( 1086 ) ;			/* unskip */
    else if ( chrcode == 11 ) 
		printesc ( 1085 ) ;			/* unkern */
    else printesc ( 1084 ) ;		/* unpenalty */
    break ; 
  case 23 : 
    if ( chrcode == 1 ) 
		printesc ( 1088 ) ;			/* unhcopy */
    else printesc ( 1087 ) ;		/* unhbox */
    break ; 
  case 24 : 
    if ( chrcode == 1 ) 
		printesc ( 1090 ) ;			/* unvcopy */
    else printesc ( 1089 ) ;		/* unvbox */
    break ; 
  case 47 : 
    if ( chrcode == 1 ) 
		printesc ( 45 ) ;			/* - */
    else printesc ( 346 ) ;			/* discretionary */
    break ; 
  case 48 : 
    if ( chrcode == 1 ) 
		printesc ( 1122 ) ;			/* leqno */
    else printesc ( 1121 ) ;		/* eqno */
    break ; 
  case 50 : 
    switch ( chrcode ) 
    {case 16 : 
      printesc ( 860 ) ;		/* mathord */
      break ; 
    case 17 : 
      printesc ( 861 ) ;		/* mathop */
      break ; 
    case 18 : 
      printesc ( 862 ) ;		/* mathbin */
      break ; 
    case 19 : 
      printesc ( 863 ) ;		/* mathrel */
      break ; 
    case 20 : 
      printesc ( 864 ) ;		/* mathopen */
      break ; 
    case 21 : 
      printesc ( 865 ) ;		/* mathclose */
      break ; 
    case 22 : 
      printesc ( 866 ) ;		/* mathpunct */
      break ; 
    case 23 : 
      printesc ( 867 ) ;		/* mathinner */
      break ; 
    case 26 : 
      printesc ( 869 ) ;		/* underline */
      break ; 
	default: 
      printesc ( 868 ) ;		/* overline */
      break ; 
    } 
    break ; 
  case 51 : 
    if ( chrcode == 1 ) 
		printesc ( 872 ) ;		/* limits */
    else if ( chrcode == 2 ) 
		printesc ( 873 ) ;		/* nolimits */
    else printesc ( 1123 ) ;	/* displaylimits */
    break ; 
  case 53 : 
    printstyle ( chrcode ) ; 
    break ; 
  case 52 : 
    switch ( chrcode ) 
    {case 1 : 
      printesc ( 1142 ) ;		/* over */
      break ; 
    case 2 : 
      printesc ( 1143 ) ;		/* atop */
      break ; 
    case 3 : 
      printesc ( 1144 ) ;		/* abovewithdelims */
      break ; 
    case 4 : 
      printesc ( 1145 ) ;		/* overwithdelims */
      break ; 
    case 5 : 
      printesc ( 1146 ) ;		/* atopwithdelims */
      break ; 
	default: 
      printesc ( 1141 ) ;		/* above */
      break ; 
    } 
    break ; 
  case 49 : 
    if ( chrcode == 30 ) 
		printesc ( 870 ) ;		/* left */
    else printesc ( 871 ) ;		/* right */
    break ; 
  case 93 : 
    if ( chrcode == 1 ) 
		printesc ( 1165 ) ;			/* long */
	else if ( chrcode == 2 ) 
		printesc ( 1166 ) ;			/* outer */
    else printesc ( 1167 ) ;		/* global */
    break ; 
  case 97 : 
    if ( chrcode == 0 ) 
		printesc ( 1168 ) ;			/* def */
    else if ( chrcode == 1 ) 
		printesc ( 1169 ) ;			/* gdef */
    else if ( chrcode == 2 ) 
		printesc ( 1170 ) ;			/* edef */
    else printesc ( 1171 ) ;		/* xdef */
    break ; 
  case 94 : 
    if ( chrcode != 0 ) 
		printesc ( 1186 ) ;			/* futurelet */
    else printesc ( 1185 ) ;		/* let */
    break ; 
  case 95 : 
    switch ( chrcode ) 
    {case 0 : 
      printesc ( 1187 ) ;		/* chardef */
      break ; 
    case 1 : 
      printesc ( 1188 ) ;		/* mathchardef */
      break ; 
    case 2 : 
      printesc ( 1189 ) ;		/* countdef */
      break ; 
    case 3 : 
      printesc ( 1190 ) ;		/* dimendef */
      break ; 
    case 4 : 
      printesc ( 1191 ) ;		/* skipdef */
      break ; 
    case 5 : 
      printesc ( 1192 ) ;		/* muskipdef */
      break ; 
	default: 
      printesc ( 1193 ) ;		/* toksdef */
      break ; 
    } 
    break ; 
  case 68 : 
    {
      printesc ( 510 ) ;		/* char */
      printhex ( chrcode ) ; 
    } 
    break ; 
  case 69 : 
    {
      printesc ( 521 ) ;		/* mathchar */
      printhex ( chrcode ) ; 
    } 
    break ; 
  case 85 : 
    if ( chrcode == (hash_size + 1883) ) 
		printesc ( 412 ) ;		/* catcode */
    else if ( chrcode == (hash_size + 2907) )
		printesc ( 416 ) ;		/* mathcode */
    else if ( chrcode == (hash_size + 2139) )
		printesc ( 413 ) ;		/* lccode */
    else if ( chrcode == (hash_size + 2395) )
		printesc ( 414 ) ;		/* uccode */
    else if ( chrcode == (hash_size + 2651) )
		printesc ( 415 ) ;		/* sfcode */
    else printesc ( 474 ) ;		/* delcode */
    break ; 
  case 86 : 
    printsize ( chrcode - (hash_size + 1835) ) ; /* chr - math_font_base */
    break ; 
  case 99 : 
    if ( chrcode == 1 ) 
		printesc ( 947 ) ;		/* patterns */
    else printesc ( 935 ) ;		/* hyphenation */
    break ; 
  case 78 : 
    if ( chrcode == 0 ) 
		printesc ( 1211 ) ;		/* hyphenchar */
    else printesc ( 1212 ) ;	/* skewchar */
    break ; 
  case 87 : 
    {
      print ( 1220 ) ;		/* select font  */
      slowprint ( fontname [ chrcode ] ) ; 
      if ( fontsize [ chrcode ] != fontdsize [ chrcode ] ) 
      {
		  print ( 738 ) ;			/*  at  */
		  printscaled ( fontsize [ chrcode ] ) ; 
		  print ( 394 ) ;			/* pt */
      } 
    } 
    break ; 
  case 100 : 
    switch ( chrcode ) 
    {case 0 : 
      printesc ( 272 ) ;	/* batchmode */
      break ; 
    case 1 : 
      printesc ( 273 ) ;	/* nonstopmode */
      break ; 
    case 2 : 
      printesc ( 274 ) ;	/* scrollmode */
      break ; 
	default: 
      printesc ( 1221 ) ;	/* errorstopmode  */
      break ; 
    } 
    break ; 
  case 60 : 
    if ( chrcode == 0 ) 
		printesc ( 1223 ) ;			/* closein */
    else printesc ( 1222 ) ;		/* openin */
    break ; 
  case 58 : 
    if ( chrcode == 0 ) 
		printesc ( 1224 ) ;			/* message */
    else printesc ( 1225 ) ;		/* errmessage */
    break ; 
  case 57 : 
    if ( chrcode == (hash_size + 2139) ) 
		printesc ( 1231 ) ;			/* lowercase */
    else printesc ( 1232 ) ;		/* uppercase */
    break ; 
  case 19 : 
    switch ( chrcode ) 
    {case 1 : 
      printesc ( 1234 ) ;		/* showbox */
      break ; 
    case 2 : 
      printesc ( 1235 ) ;		/* showthe */
      break ; 
    case 3 : 
      printesc ( 1236 ) ;		/* showlists */
      break ; 
	default: 
      printesc ( 1233 ) ;		/* show */
      break ; 
    } 
    break ; 
  case 101 : 
    print ( 1243 ) ;			/* undefined */
	break ;
  case 111 : 
    print ( 1244 ) ;			/* macro */
    break ; 
  case 112 : 
    printesc ( 1245 ) ;			/* long macro */
    break ; 
  case 113 : 
    printesc ( 1246 ) ;			/* outer macro */
    break ; 
  case 114 : 
    {
      printesc ( 1165 ) ;		/* long */
      printesc ( 1246 ) ;		/* outer macro */
    } 
    break ; 
  case 115 : 
    printesc ( 1247 ) ;			/* outer endtemplate */
    break ; 
  case 59 : 
    switch ( chrcode ) 
    {case 0 : 
      printesc ( 1279 ) ;		/* openout */
      break ; 
    case 1 : 
      printesc ( 591 ) ;		/* write */
      break ; 
    case 2 : 
      printesc ( 1280 ) ;		/* closeout */
      break ; 
    case 3 : 
      printesc ( 1281 ) ;		/* special */
      break ; 
    case 4 : 
      printesc ( 1282 ) ;		/* immediate */
      break ; 
    case 5 : 
      printesc ( 1283 ) ;		/* setlanguage */
      break ; 
    default: 
      print ( 1284 ) ;			/* [unknown extension!] */
      break ; 
    } 
    break ; 
  default: 
		print ( 564 ) ;			/* [unknown command code!] */
		break ; 
  } 
} 

#ifdef STAT
void zshoweqtb ( n ) 
halfword n ; 
{showeqtb_regmem 
  if ( n < 1 ) 
  printchar ( 63 ) ; 			/* ? */
  else if ( n < (hash_size + 782) )		/* lineskip */
  {
    sprintcs ( n ) ; 
    printchar ( 61 ) ; 			/* = */
    printcmdchr ( eqtb [ n ] .hh.b0 , eqtb [ n ] .hh .v.RH ) ; 
    if ( eqtb [ n ] .hh.b0 >= 111 ) 
    {
      printchar ( 58 ) ; 		/* : */
      showtokenlist ( mem [ eqtb [ n ] .hh .v.RH ] .hh .v.RH , 0 , 32 ) ; 
    } 
  } 
  else if ( n < (hash_size + 1312) ) 
  if ( n < (hash_size + 800) ) 
  {
    printskipparam ( n - (hash_size + 782) ) ;	/* lineskip */
    printchar ( 61 ) ; 			/* = */
    if ( n < (hash_size + 797) ) 
		printspec ( eqtb [ n ] .hh .v.RH , 394 ) ;	/* pt */
    else printspec ( eqtb [ n ] .hh .v.RH , 334 ) ;	/* mu */
  } 
  else if ( n < (hash_size + 1056) ) 
  {
    printesc ( 392 ) ;		/* skip */
    printint ( n - (hash_size + 800) ) ; 
    printchar ( 61 ) ;		/* = */
    printspec ( eqtb [ n ] .hh .v.RH , 394 ) ;		/* pt */
  } 
  else {
    printesc ( 393 ) ;		/* muskip */
    printint ( n - (hash_size + 1056) ) ; 
    printchar ( 61 ) ;		/* = */
    printspec ( eqtb [ n ] .hh .v.RH , 334 ) ;		/* mu */
  } 
  else if ( n < (hash_size + 3163) ) 
  if ( n == (hash_size + 1312) ) 
  {
    printesc ( 405 ) ;		/* parshape */
    printchar ( 61 ) ;		/* = */
    if ( eqtb [ (hash_size + 1312) ] .hh .v.RH == 0 ) 
    printchar ( 48 ) ; 		/* 0 */
    else printint ( mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH ] .hh .v.LH ) ; 
  } 
  else if ( n < (hash_size + 1322) ) 
  {
    printcmdchr ( 72 , n ) ; /* H */
    printchar ( 61 ) ;		 /* = */
    if ( eqtb [ n ] .hh .v.RH != 0 ) 
    showtokenlist ( mem [ eqtb [ n ] .hh .v.RH ] .hh .v.RH , 0 , 32 ) ; 
  } 
  else if ( n < (hash_size + 1578) ) 
  {
    printesc ( 404 ) ;		/* toks */
    printint ( n - (hash_size + 1322) ) ; 
    printchar ( 61 ) ; 		/* = */
    if ( eqtb [ n ] .hh .v.RH != 0 ) 
    showtokenlist ( mem [ eqtb [ n ] .hh .v.RH ] .hh .v.RH , 0 , 32 ) ; 
  } 
  else if ( n < (hash_size + 1834) ) 
  {
    printesc ( 406 ) ;		/* box */
    printint ( n - (hash_size + 1578) ) ; 
    printchar ( 61 ) ;		/* = */
    if ( eqtb [ n ] .hh .v.RH == 0 ) 
    print ( 407 ) ;			/* void */
    else {
	
      depththreshold = 0 ; 
      breadthmax = 1 ; 
      shownodelist ( eqtb [ n ] .hh .v.RH ) ; 
    } 
  } 
  else if ( n < (hash_size + 1883) )		/* cat_code_base ... */
  {
    if ( n == (hash_size + 1834) ) 
		print ( 408 ) ;			/* current font */
    else if ( n < (hash_size + 1851) ) 
    {
      printesc ( 409 ) ;		/* textfont */
      printint ( n - (hash_size + 1835) ) ; 
    } 
    else if ( n < (hash_size + 1867) ) 
    {
      printesc ( 410 ) ;	/* scriptfont */
      printint ( n - (hash_size + 1851) ) ; 
    } 
    else {
      printesc ( 411 ) ;	/* scriptscriptfont */
      printint ( n - (hash_size + 1867) ) ; 
    } 
    printchar ( 61 ) ;		/* = */
/*    printesc ( hash [ (hash_size + 524) + eqtb [ n ] .hh .v.RH ] .v.RH ) ; */
    printesc ( hash [ (hash_size + hash_extra + 524) + eqtb [ n ] .hh .v.RH ] .v.RH ) ; 
								/* 96/Jan/10 */
  } 
  else if ( n < (hash_size + 2907) ) 
  {
    if ( n < (hash_size + 2139) ) 
    {
      printesc ( 412 ) ;		/* catcode */
      printint ( n - (hash_size + 1883) ) ; 
    } 
    else if ( n < (hash_size + 2395) ) 
    {
      printesc ( 413 ) ;		/* lccode */
      printint ( n - (hash_size + 2139) ) ; 
    } 
    else if ( n < (hash_size + 2651) ) 
    {
      printesc ( 414 ) ;		/* uccode */
      printint ( n - (hash_size + 2395) ) ; 
    } 
    else {
	
      printesc ( 415 ) ;		/* sfcode */
      printint ( n - (hash_size + 2651) ) ; 
    } 
    printchar ( 61 ) ;			/* = */
    printint ( eqtb [ n ] .hh .v.RH ) ; 
  } 
  else {
      
    printesc ( 416 ) ;		/* mathcode */
    printint ( n - (hash_size + 2907) ) ; 
    printchar ( 61 ) ;  		/* = */
    printint ( eqtb [ n ] .hh .v.RH ) ; 
  } 
  else if ( n < (hash_size + 3730) ) 
  {
    if ( n < (hash_size + 3218) ) 
    printparam ( n - (hash_size + 3163) ) ; 
    else if ( n < (hash_size + 3474) ) 
    {
      printesc ( 473 ) ;		/* count */
      printint ( n - (hash_size + 3218) ) ; 
    } 
    else {
	
      printesc ( 474 ) ;		/* delcode */
      printint ( n - (hash_size + 3474) ) ; 
    } 
    printchar ( 61 ) ;  		/* = */
    printint ( eqtb [ n ] .cint ) ; 
  } 
  else if ( n <= (hash_size + 4006) ) 
  {
    if ( n < (hash_size + 3751) ) 
    printlengthparam ( n - (hash_size + 3730) ) ; 
    else {
	
      printesc ( 497 ) ;		/* dimen */
      printint ( n - (hash_size + 3751) ) ; 
    } 
    printchar ( 61 ) ;			/* = */
    printscaled ( eqtb [ n ] .cint ) ; 
    print ( 394 ) ;				/* pt */
  } 
  else printchar ( 63 ) ;		/* = */
} 
#endif /* STAT */

halfword zidlookup ( j , l ) 
integer j ; 
integer l ; 
{/* 40 */ register halfword Result; idlookup_regmem 
    integer h  ; 
	integer d  ; 
	halfword p  ; 
	halfword k  ; 
	h = buffer [ j ] ; 
	{
		register integer for_end; 
		k = j + 1 ; 
		for_end = j + l - 1 ; 
		if ( k <= for_end) do {
			h = h + h + buffer [ k ] ; 
			while ( h >= hash_prime ) h = h - hash_prime ; /* buffer size hash prime */
		} 
		while ( k++ < for_end ) ;
	} 
	p = h + 514 ;						/* h + hash_base */
	while ( true ) {
		if ( hash [ p ] .v.RH > 0 ) 
			if ( ( strstart [ hash [ p ] .v.RH + 1 ] - strstart [ hash [ p ] .v.RH ] ) 
				 == l ) 
				if ( streqbuf ( hash [ p ] .v.RH , j ) ) 
					goto lab40 ; 
		if ( hash [ p ] .v.LH == 0 ) {
			if ( nonewcontrolsequence )
				p = (hash_size + 781) ;		/* undefine_control_sequence */
			else {
				if ( hash [ p ] .v.RH > 0 ) {
					do {
						if ( ( hashused == 514 ) ) {	/* if hashused = hashbase ... */ 
/*				we can't expand the hash table ... */
/*				overflow ( 500 , hash_size ) ; */ /* hash size - NOT DYNAMIC */
							overflow ( 500 , hash_size + hash_extra ) ; /* 96/Jan/10 */
							return 0;			// abortflag set
						}
						decr ( hashused ) ; 
					} while ( ! ( hash [ hashused ] .v.RH == 0 ) ) ; 
#ifdef SHORTHASH
					if (hashused > 65535L) {			/* debugging only 1996/Jan/20 */
						sprintf(logline, "ERROR: %s too large %d\n",
								"hash entry", hashused);
						showline(logline, 1);
					}
#endif
					hash [ p ] .v.LH = hashused ; 
					p = hashused ; 
				} 
#ifdef CHECKPOOL
				if (checkpool(NULL)) showline("after hashused\n", 0); 
#endif
				{
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
		if ( poolptr + l > currentpoolsize ) 
/*			  strpool = reallocstrpool (incrementpoolsize); */
			strpool = reallocstrpool (incrementpoolsize + 1);
		if ( poolptr + l > currentpoolsize ) {	/* in case it failed 97/Mar/7 */
			overflow ( 257 , currentpoolsize - initpoolptr ) ; /* pool size */
			return 0;			// abortflag set
		}
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
		if ( poolptr + l > poolsize ) {
			overflow ( 257 , poolsize - initpoolptr ) ; /* pool size - not dynamic */
			return;			// abortflag set
		}
#endif
				} 
				d = ( poolptr - strstart [ strptr ] ) ; 
				while ( poolptr > strstart [ strptr ] ) {
					decr ( poolptr ) ; 
					strpool [ poolptr + l ] = strpool [ poolptr ] ; 
				} 
#ifdef CHECKPOOL
				if (checkpool(NULL)) showline("after poolptr\n", 0);
#endif
				{
					register integer for_end; 
					k = j ; 
					for_end = j + l - 1 ; 
					if ( k <= for_end) do {
						strpool [ poolptr ] = buffer [ k ] ; 
						incr ( poolptr ) ; 
					} 
					while ( k++ < for_end ) ;
				} 
#ifdef SHORTHASH
				{
					poolpointer tempstring = makestring () ;
					if (tempstring > 65535L) {			/* cannot happen */
						sprintf(logline, "ERROR: %s too large %d\n",
								"string ptr", tempstring);
						showline(logline, 1);
					}
					hash [ p ] .v.RH = tempstring ;
				}
#else
				hash [ p ] .v.RH = makestring () ; 
#endif
#ifdef CHECKPOOL
				if (checkpool(NULL)) showline("after makestring\n", 0); 
#endif
				poolptr = poolptr + d ; 
				;
#ifdef STAT
				incr ( cscount ) ; 
				if (traceflag) {
					strpool[poolptr] = '\0';
					sprintf(logline, " tex1 cscount++ '%s' ", &strpool[poolptr-l-d]);
					showline(logline, 0);			/* debugging */
				}
#endif /* STAT */
			} 
#ifdef CHECKPOOL
			if (checkpool(NULL)) showline("after cscount++\n", 0); 
#endif
			goto lab40 ; 
		} 
		p = hash [ p ] .v.LH ; 
	} 
#ifdef CHECKPOOL
	if (checkpool(NULL)) showline("before return\n", 0); 
#endif
lab40:
	Result = p ; 
	return Result ; 
} 

void znewsavelevel ( c ) 
groupcode c ; 
{newsavelevel_regmem 
   if ( saveptr > maxsavestack )				/* check_full_save_stack; p.274 */
  {
    maxsavestack = saveptr ; 
#ifdef ALLOCATESAVESTACK
	if ( maxsavestack > currentsavesize - 6 ) 
		savestack = reallocsavestack (incrementsavesize);
	if ( maxsavestack > currentsavesize - 6 ) {	/* check again after allocation */
		overflow ( 538 , currentsavesize ) ;
		return;			// abortflag set
	}
#else
	if ( maxsavestack > savesize - 6 ) {	/* save size - not dynamic */
		overflow ( 538 , savesize ) ;
		return;			// abortflag set
	}
#endif
  } 
/* save_type(save_ptr) <- level_boundary; */
  savestack [ saveptr ] .hh.b0 = 3 ; 
/*  savestack [ saveptr ] .hh.b1 = curgroup ;  *//* keep compiler happy */
  savestack [ saveptr ] .hh.b1 = (quarterword) curgroup ; 
/* save_index(save_ptr):=cur_boundary; */
  savestack [ saveptr ] .hh .v.RH = curboundary ; 
/* if cur_level = max_quarterword then ... p.274 */
/*  if ( curlevel == 255 ) */	 		/* 94/Apr/4 */
  if ( curlevel == maxquarterword)	{
/* { quit if (cur_level + 1) is too large to store in eqtb } */
/*	overflow("grouping levels", max_quarterword - min_quarterword); */
/*	overflow ( 539 , 255 ) ; */			/* grouping levels - not dynamic */
	  overflow ( 539 , maxquarterword - minquarterword ) ;	/* 96/Oct/12 ??? */
	  return;			// abortflag set
  }
/* cur_boundary <- save_ptr */
  curboundary = saveptr ; 
  incr ( curlevel ) ; 
  incr ( saveptr ) ; 
  curgroup = c ; 
} 

void zeqdestroy ( w ) 
memoryword w ; 
{eqdestroy_regmem 
  halfword q  ; 
  switch ( w .hh.b0 ) 
  {case 111 : 
  case 112 : 
  case 113 : 
  case 114 : 
    deletetokenref ( w .hh .v.RH ) ; 
    break ; 
  case 117 : 
    deleteglueref ( w .hh .v.RH ) ; 
    break ; 
  case 118 : 
    {
      q = w .hh .v.RH ; 
      if ( q != 0 )		/* if q<>null then free_node(... l.5937 */
      freenode ( q , mem [ q ] .hh .v.LH + mem [ q ] .hh .v.LH + 1 ) ; 
    } 
    break ; 
  case 119 : 
    flushnodelist ( w .hh .v.RH ) ; 
    break ; 
  default: 
    ; 
    break ; 
  } 
} 

void zeqsave ( p , l ) 
halfword p ; 
quarterword l ; 
{eqsave_regmem 
  if ( saveptr > maxsavestack ) 
  {
    maxsavestack = saveptr ; 
#ifdef ALLOCATESAVESTACK
	if ( maxsavestack > currentsavesize - 6 ) 
		savestack = reallocsavestack (incrementsavesize);
	if ( maxsavestack > currentsavesize - 6 ) {	/* check again after allocation */
		overflow ( 538 , currentsavesize ) ;
		return;			// abortflag set
	}
#else
	if ( maxsavestack > savesize - 6 ) {	/* save size not dynamic */
		overflow ( 538 , savesize ) ;
		return;			// abortflag set
	}
#endif
  } 
  if ( l == 0 ) 
  savestack [ saveptr ] .hh.b0 = 1 ; 
  else {
      
    savestack [ saveptr ] = eqtb [ p ] ; 
    incr ( saveptr ) ; 
    savestack [ saveptr ] .hh.b0 = 0 ; 
  } 
  savestack [ saveptr ] .hh.b1 = l ; 
  savestack [ saveptr ] .hh .v.RH = p ; 
  incr ( saveptr ) ; 
} 

void zeqdefine ( p , t , e ) 
halfword p ; 
quarterword t ; 
halfword e ; 
{eqdefine_regmem 
  if ( eqtb [ p ] .hh.b1 == curlevel ) 
  eqdestroy ( eqtb [ p ] ) ; 
  else if ( curlevel > 1 ) 
  eqsave ( p , eqtb [ p ] .hh.b1 ) ; 
/*  eqtb [ p ] .hh.b1 = curlevel ;  */
  eqtb [ p ] .hh.b1 = (quarterword) curlevel ; /* because curlevel padded out */
  eqtb [ p ] .hh.b0 = t ; 
  eqtb [ p ] .hh .v.RH = e ; 
} 

void zeqworddefine ( p , w ) 
halfword p ; 
integer w ; 
{eqworddefine_regmem 
  if ( xeqlevel [ p ] != curlevel ) 
  {
    eqsave ( p , xeqlevel [ p ] ) ; 
/*    xeqlevel [ p ] = curlevel ;  */
    xeqlevel [ p ] = (quarterword) curlevel ; /* because curlevel padded out */
  } 
  eqtb [ p ] .cint = w ; 
} 

void zgeqdefine ( p , t , e ) 
halfword p ; 
quarterword t ; 
halfword e ; 
{geqdefine_regmem 
  eqdestroy ( eqtb [ p ] ) ; 
  eqtb [ p ] .hh.b1 = 1 ; 
  eqtb [ p ] .hh.b0 = t ; 
  eqtb [ p ] .hh .v.RH = e ; 
} 

void zgeqworddefine ( p , w ) 
halfword p ; 
integer w ; 
{geqworddefine_regmem 
  eqtb [ p ] .cint = w ; 
  xeqlevel [ p ] = 1 ; 
} 

void zsaveforafter ( t ) 
halfword t ; 
{saveforafter_regmem 
  if ( curlevel > 1 ) 
  {
    if ( saveptr > maxsavestack ) 
    {
      maxsavestack = saveptr ; 
#ifdef ALLOCATESAVESTACK
	  if ( maxsavestack > currentsavesize - 6 ) 
		  savestack = reallocsavestack (incrementsavesize);
	  if ( maxsavestack > currentsavesize - 6 ) {	/* check again after allocation */
		  overflow ( 538 , currentsavesize ) ;
		  return;			// abortflag set
	  }
#else
	  if ( maxsavestack > savesize - 6 ) {		/* save satck - not dynamic */
		  overflow ( 538 , savesize ) ;
		  return;			// abortflag set
	  }
#endif
	} 
    savestack [ saveptr ] .hh.b0 = 2 ; 
    savestack [ saveptr ] .hh.b1 = 0 ; 
    savestack [ saveptr ] .hh .v.RH = t ; 
    incr ( saveptr ) ; 
  } 
} 

/* zrestoretrace, unsave followed in the old tex1.c */