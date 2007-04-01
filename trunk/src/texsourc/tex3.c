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

/* #pragma optimize("a", off) */			/* 98/Dec/10 experiment */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void scanint ( ) 
{/* 30 */ scanint_regmem 
  booleane negative  ; 
  integer m  ; 
  smallnumber d  ; 
  booleane vacuous  ; 
  booleane OKsofar  ; 
  radix = 0 ; 
  OKsofar = true ; 
  negative = false ; 
  do {
      do {
		  getxtoken () ; 
		  ABORTCHECK;
    } while ( ! ( curcmd != 10 ) ) ; 
    if ( curtok == 3117 ) 
    {
      negative = ! negative ; 
      curtok = 3115 ; 
    } 
  } while ( ! ( curtok != 3115 ) ) ; 
  if ( curtok == 3168 ) 
  {
    gettoken () ; 
    if ( curtok < 4095 ) 
    {
      curval = curchr ; 
      if ( curcmd <= 2 ) 
      if ( curcmd == 2 ) 
      incr ( alignstate ) ; 
      else decr ( alignstate ) ; 
    } 
/* else if cur_tok<cs_token_flag+single_base then ... */
    else if ( curtok < 4352 )	/* 4095 + 257 */
/*   cur_val:=cur_tok-cs_token_flag-active_base */
    curval = curtok - 4096 ;	/* 4095 + 1 */
/* else cur_val:=cur_tok-cs_token_flag-single_base; */
    else curval = curtok - 4352 ; 	/* 4095 + 257 */
    if ( curval > 255 ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* ! */
	print ( 695 ) ;		/* Improper alphabetic constant */
      } 
      {
	helpptr = 2 ; 
	helpline [ 1 ] = 696 ; /* A one-character control sequence belongs after a ` mark. */
	helpline [ 0 ] = 697 ; /* So I'm essentially inserting \0 here. */
      } 
      curval = 48 ; 
      backerror () ; 
	  ABORTCHECK;
    } 
    else {
      getxtoken () ; 
	  ABORTCHECK;
      if ( curcmd != 10 ) 
      backinput () ; 
    } 
  } 
  else if ( ( curcmd >= 68 ) && ( curcmd <= 89 ) ) {
	  scansomethinginternal ( 0 , false ) ;
	  ABORTCHECK;
  }
  else {
      
/* begin radix:=10; m:=214748364; l.8734 */
    radix = 10 ; 
    m = 214748364L ;		/* 7FFFFFFF hex */
    if ( curtok == 3111 ) 
    {
      radix = 8 ; 
      m = 268435456L ;		/* 2^28 */
      getxtoken () ; 
	  ABORTCHECK;
    } 
    else if ( curtok == 3106 ) 
    {
      radix = 16 ; 
      m = 134217728L ;		/* 2^27 8000000 hex */
      getxtoken () ; 
	  ABORTCHECK;
    } 
    vacuous = true ; 
    curval = 0 ; 
    while ( true ) {
      if ( ( curtok < 3120 + radix ) && ( curtok >= 3120 ) && ( curtok <= 3129 
      ) ) 
      d = curtok - 3120 ; 
      else if ( radix == 16 ) 
      if ( ( curtok <= 2886 ) && ( curtok >= 2881 ) ) 
      d = curtok - 2871 ; 
      else if ( ( curtok <= 3142 ) && ( curtok >= 3137 ) ) 
      d = curtok - 3127 ; 
      else goto lab30 ; 
      else goto lab30 ; 
      vacuous = false ; 
      if ( ( curval >= m ) && ( ( curval > m ) || ( d > 7 ) || ( radix != 10 ) 
      ) ) 
      {
	if ( OKsofar ) 
	{
	  {
	    if ( interaction == 3 ) 
	    ; 
	    printnl ( 262 ) ;	/* ! */
	    print ( 698 ) ;		/* Number too big */
	  } 
	  {
	    helpptr = 2 ; 
	    helpline [ 1 ] = 699 ; /* I can only go up to 2147483647='17777777777="7FFFFFFF, */
	    helpline [ 0 ] = 700 ; /* so I'm using that number instead of yours. */
	  } 
	  error () ; 
	  ABORTCHECK;
	  curval = 2147483647L ; 		/* 7FFFFFFF hex */
	  OKsofar = false ; 
	} 
      } 
      else curval = curval * radix + d ; 
      getxtoken () ; 
	  ABORTCHECK;
    } 
    lab30: ; 
    if ( vacuous ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;		/* ! */
	print ( 661 ) ;			/* Missing number, treated as zero */
      } 
      {
	helpptr = 3 ; 
	helpline [ 2 ] = 662 ; /* A number should have been here; I inserted `0'. */
	helpline [ 1 ] = 663 ; /* (If you can't figure out why I needed to see a number, */
	helpline [ 0 ] = 664 ; /* look up `weird error' in the index to The TeXbook.) */
      } 
      backerror () ; 
	  ABORTCHECK;
    } 
    else if ( curcmd != 10 ) 
    backinput () ; 
  } 
  if ( negative ) 
  curval = - (integer) curval ; 
} 

void zscandimen ( mu , inf , shortcut ) 
booleane mu ; 
booleane inf ; 
booleane shortcut ; 
{/* 30 31 32 40 45 88 89 */ scandimen_regmem 
  booleane negative  ; 
  integer f  ; 
  integer num, denom  ; 
  smallnumber k, kk  ; 
  halfword p, q  ; 
  scaled v  ; 
  integer savecurval  ; 
  f = 0 ; 
  aritherror = false ; 
  curorder = 0 ; 
  negative = false ; 
  if ( ! shortcut ) 
  {
    negative = false ; 
    do {
	do {
		getxtoken () ; 
		ABORTCHECK;
      } while ( ! ( curcmd != 10 ) ) ; 
      if ( curtok == 3117 ) 
      {
	negative = ! negative ; 
	curtok = 3115 ; 
      } 
    } while ( ! ( curtok != 3115 ) ) ; 
    if ( ( curcmd >= 68 ) && ( curcmd <= 89 ) ) 
    if ( mu ) 
    {
      scansomethinginternal ( 3 , false ) ; 
	  ABORTCHECK;
      if ( curvallevel >= 2 ) {
		  v = mem [ curval + 1 ] .cint ; 
		  deleteglueref ( curval ) ; 
		  curval = v ; 
      } 
      if ( curvallevel == 3 ) 
      goto lab89 ; 
      if ( curvallevel != 0 ) {
		  muerror () ; 
		  ABORTCHECK;
	  }
    } 
    else {
      scansomethinginternal ( 1 , false ) ; 
	  ABORTCHECK;
      if ( curvallevel == 1 ) goto lab89 ; 
    } 
    else {
	
      backinput () ; 
      if ( curtok == 3116 ) 
      curtok = 3118 ; 
      if ( curtok != 3118 ) {
		  scanint () ;
		  ABORTCHECK;
	  }
      else {
	  
	radix = 10 ; 
	curval = 0 ; 
      } 
      if ( curtok == 3116 ) 
      curtok = 3118 ; 
      if ( ( radix == 10 ) && ( curtok == 3118 ) ) 
      {
	k = 0 ; 
	p = 0 ;			/* p:=null l.8883 */
	gettoken () ; 
	while ( true ) {
	  getxtoken () ; 
	  ABORTCHECK;
	  if ( ( curtok > 3129 ) || ( curtok < 3120 ) ) 
	  goto lab31 ; 
	  if ( k < 17 ) 
	  {
	    q = getavail () ; 
	    mem [ q ] .hh .v.RH = p ; 
	    mem [ q ] .hh .v.LH = curtok - 3120 ; 
	    p = q ; 
	    incr ( k ) ; 
	  } 
	} 
	lab31: {
	    register integer for_end; kk = k ; for_end = 1 ; if ( kk >= 
	for_end) do 
	  {
/*		long to char ... */
	    dig [ kk - 1 ] = mem [ p ] .hh .v.LH ; 
	    q = p ; 
	    p = mem [ p ] .hh .v.RH ; 
	    {
	      mem [ q ] .hh .v.RH = avail ; 
	      avail = q ; 
	;
#ifdef STAT
	      decr ( dynused ) ; 
#endif /* STAT */
	    } 
	  } 
	while ( kk-- > for_end ) ; } 
	f = rounddecimals ( k ) ; 
	if ( curcmd != 10 ) 
	backinput () ; 
      } 
    } 
  } 
  if ( curval < 0 ) 
  {
    negative = ! negative ; 
    curval = - (integer) curval ; 
  } 
  if ( inf ) 
  if ( scankeyword ( 309 ) )		/* fil */
  {
    curorder = 1 ; 
    while ( scankeyword ( 108 ) ) {	/* l */
	
      if ( curorder == 3 )  {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 702 ) ;		/* Illegal unit of measure ( */
	} 
	print ( 703 ) ;			/* replaced by filll) */
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 704 ; /* I dddon't go any higher than filll. */
	} 
	error () ; 
	ABORTCHECK;
	  } 
      else incr ( curorder ) ; 
    } 
    goto lab88 ; 
  } 
  savecurval = curval ; 
  do {
      getxtoken () ; 
	  ABORTCHECK;
  } while ( ! ( curcmd != 10 ) ) ; 
  if ( ( curcmd < 68 ) || ( curcmd > 89 ) ) 
  backinput () ; 
  else {
      
    if ( mu ) 
    {
      scansomethinginternal ( 3 , false ) ; 
	  ABORTCHECK;
      if ( curvallevel >= 2 ) {
	v = mem [ curval + 1 ] .cint ; 
	deleteglueref ( curval ) ; 
	curval = v ; 
      } 
      if ( curvallevel != 3 ) {
		  muerror () ; 
		  ABORTCHECK;
	  }
    } 
    else {
		scansomethinginternal ( 1 , false ) ;
		ABORTCHECK;
	}
    v = curval ; 
    goto lab40 ; 
  } 
  if ( mu ) 
  goto lab45 ; 
  if ( scankeyword ( 705 ) )		/* em */
  v = ( fontinfo [ 6 + parambase [ eqtb [ (hash_size + 1834) ] .hh .v.RH ] ] .cint ) ; 
  else if ( scankeyword ( 706 ) )	/* ex */
  v = ( fontinfo [ 5 + parambase [ eqtb [ (hash_size + 1834) ] .hh .v.RH ] ] .cint ) ; 
  else goto lab45 ; 
  {
    getxtoken () ; 
	ABORTCHECK;
    if ( curcmd != 10 )   backinput () ; 
  } 
  lab40: curval = multandadd ( savecurval , v , xnoverd ( v , f , 65536L ) , 
  1073741823L ) ; 	/* 2^30 - 1 */
  goto lab89 ; 
  lab45: ; 
  if ( mu ) 
  if ( scankeyword ( 334 ) )	/* mu */
  goto lab88 ; 
  else {
      
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 702 ) ;		/* Illegal unit of measure ( */
    } 
    print ( 707 ) ;			/* mu inserted) */
    {
      helpptr = 4 ; 
      helpline [ 3 ] = 708 ; /* The unit of measurement in math glue must be mu. */
      helpline [ 2 ] = 709 ; /* To recover gracefully from this error, it's best to */
      helpline [ 1 ] = 710 ; /* delete the erroneous units; e.g., type `2' to delete */
      helpline [ 0 ] = 711 ; /* two letters. (See Chapter 27 of The TeXbook.) */
    } 
    error () ; 
	ABORTCHECK;
    goto lab88 ; 
  } 
  if ( scankeyword ( 701 ) )	/* true */
  {
    preparemag () ; 
	ABORTCHECK;
    if ( eqtb [ (hash_size + 3180) ] .cint != 1000 ) 
    {
      curval = xnoverd ( curval , 1000 , eqtb [ (hash_size + 3180) ] .cint ) ; 
      f = ( 1000 * f + 65536L * texremainder ) /
				  eqtb [ (hash_size + 3180) ] .cint ; 
      curval = curval + ( f / 65536L ) ; 
/*      curval = curval + ( f >> 16 ) ; */   /* f positive ??? */
      f = f % 65536L ; 
/*      f = f & 65535L ; */					/* f positive ??? */
    } 
  } 
  if ( scankeyword ( 394 ) )		/* pt */
  goto lab88 ; 
  if ( scankeyword ( 712 ) )		/* in */
  {
    num = 7227 ; 
    denom = 100 ; 
  } 
  else if ( scankeyword ( 713 ) )	/* pc */
  {
    num = 12 ; 
    denom = 1 ; 
  } 
  else if ( scankeyword ( 714 ) )	/* cm */
  {
    num = 7227 ; 
    denom = 254 ; 
  } 
  else if ( scankeyword ( 715 ) )	/* mm */
  {
    num = 7227 ; 
    denom = 2540 ; 
  } 
  else if ( scankeyword ( 716 ) )	/* bp */
  {
    num = 7227 ; 
    denom = 7200 ; 
  } 
  else if ( scankeyword ( 717 ) )	/* dd */
  {
    num = 1238 ; 
    denom = 1157 ; 
  } 
  else if ( scankeyword ( 718 ) )	/* cc */
  {
    num = 14856 ;					/* numerator */
    denom = 1157 ;					/* denominator */ 
  } 
  else if ( scankeyword ( 719 ) )	/* sp */
  goto lab30 ; 
  else {
      
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 702 ) ;		/* Illegal unit of measure ( */
    } 
    print ( 720 ) ;			/* pt inserted) */
    {
      helpptr = 6 ; 
      helpline [ 5 ] = 721 ; /* Dimensions can be in units of em, ex, in, pt, pc, */
      helpline [ 4 ] = 722 ; /* cm, mm, dd, cc, bp, or sp; but yours is a new one! */
      helpline [ 3 ] = 723 ; /* I'll assume that you meant to say pt, for printer's points. */
      helpline [ 2 ] = 709 ; /* To recover gracefully from this error, it's best to */
      helpline [ 1 ] = 710 ; /* delete the erroneous units; e.g., type `2' to delete */
      helpline [ 0 ] = 711 ; /* two letters. (See Chapter 27 of The TeXbook.) */
    } 
    error () ; 
	ABORTCHECK;
    goto lab32 ; 
  } 
  curval = xnoverd ( curval , num , denom ) ; 
  f = ( num * f + 65536L * texremainder ) / denom ; 
  curval = curval + ( f / 65536L ) ; 
/*  curval = curval + ( f >> 16 ) ; */   /* f positive ??? */
  f = f % 65536L ; 
/*  f = f & 65535L ; */					/* f positive ??? */
  lab32: ; 
  lab88: if ( curval >= 16384 )			/* 2^14 */
  aritherror = true ; 
  else curval = curval * 65536L + f ; 
/*  else curval = curval << 16 + f ; */   /* f positive ?? */
  lab30: ; 
  {
    getxtoken () ; 
	ABORTCHECK;
    if ( curcmd != 10 )  backinput () ; 
  } 
lab89:
  if ( aritherror || ( abs ( curval ) >= 1073741824L ) ) /* 2^30 */
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 724 ) ;		/* Dimension too large */
    } 
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 725 ; /* I can't work with sizes bigger than about 19 feet. */
      helpline [ 0 ] = 726 ; /* Continue and I'll use the largest value I can. */
    } 
    error () ; 
	ABORTCHECK;
    curval = 1073741823L ; 	/* 2^30 - 1 */
    aritherror = false ; 
  } 
  if ( negative ) 
  curval = - (integer) curval ; 
} 
void zscanglue ( level ) 
smallnumber level ; 
{/* 10 */ scanglue_regmem 
  booleane negative  ; 
  halfword q  ; 
  booleane mu  ; 
  mu = ( level == 3 ) ; 
  negative = false ; 
  do {
      do {
		  getxtoken () ; 
		  ABORTCHECK;
    } while ( ! ( curcmd != 10 ) ) ; 
    if ( curtok == 3117 ) 
    {
      negative = ! negative ; 
      curtok = 3115 ; 
    } 
  } while ( ! ( curtok != 3115 ) ) ; 
  if ( ( curcmd >= 68 ) && ( curcmd <= 89 ) ) 
  {
    scansomethinginternal ( level , negative ) ; 
	ABORTCHECK;
    if ( curvallevel >= 2 ) {
      if ( curvallevel != level ) {
		  muerror () ;
		  ABORTCHECK;
	  }
      return ; 
    } 
    if ( curvallevel == 0 ) {
		scandimen ( mu , false , true ) ;
		ABORTCHECK;
	}
    else if ( level == 3 ) {
		muerror () ;
		ABORTCHECK;
	}
  } 
  else {
    backinput () ; 
    scandimen ( mu , false , false ) ; 
	ABORTCHECK;
    if ( negative ) curval = - (integer) curval ; 
  } 
  q = newspec ( 0 ) ; 
  mem [ q + 1 ] .cint = curval ; 
  if ( scankeyword ( 727 ) )	/* plus */
  {
    scandimen ( mu , true , false ) ; 
	ABORTCHECK;
    mem [ q + 2 ] .cint = curval ; 
    mem [ q ] .hh.b0 = curorder ;  
  } 
  if ( scankeyword ( 728 ) )	/* minus */
  {
    scandimen ( mu , true , false ) ; 
	ABORTCHECK;
    mem [ q + 3 ] .cint = curval ; 
    mem [ q ] .hh.b1 = curorder ;  
  } 
  curval = q ; 
} 

halfword scanrulespec ( ) 
{/* 21 */ register halfword Result; scanrulespec_regmem 
  halfword q  ; 
  q = newrule () ; 
/* if cur_cmd=vrule then width(q):=default_rule */
/* @d default_rule=26214 {0.4\thinspace pt} */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( curcmd == 35 ) 						/* cur_cmd == vrule */
/*  mem [ q + 1 ] .cint = 26214 ; */		/* width := 0.4pt */
  mem [ q + 1 ] .cint = defaultrule ; 		/* 95/Oct/9 */
  else {
      
/*   mem [ q + 3 ] .cint = 26214 ; */		/* height := 0.4pt */
    mem [ q + 3 ] .cint = defaultrule ;		/* 95/Oct/9 */
    mem [ q + 2 ] .cint = 0 ;				/* depth  := 0.0pt */
  } 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  lab21: if ( scankeyword ( 729 ) )		/* width */
  {
    scandimen ( false , false , false ) ; 
	ABORTCHECKZERO;
    mem [ q + 1 ] .cint = curval ; 
    goto lab21 ; 
  } 
  if ( scankeyword ( 730 ) )			/* height */
  {
    scandimen ( false , false , false ) ; 
	ABORTCHECKZERO;
    mem [ q + 3 ] .cint = curval ; 
    goto lab21 ; 
  } 
  if ( scankeyword ( 731 ) )			/* depth */
  {
    scandimen ( false , false , false ) ; 
	ABORTCHECKZERO;
    mem [ q + 2 ] .cint = curval ; 
    goto lab21 ; 
  } 
  Result = q ; 
  return Result ; 
} 

halfword zstrtoks ( b ) 
poolpointer b ; 
{register halfword Result; strtoks_regmem 
  halfword p  ; 
  halfword q  ; 
  halfword t  ; 
  poolpointer k  ; 
  {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
	if ( poolptr + 1 > currentpoolsize)
		strpool = reallocstrpool (incrementpoolsize);
	if ( poolptr + 1 > currentpoolsize)	{	/* in case it failed 94/Jan/22 */
		overflow ( 257 , currentpoolsize - initpoolptr ) ; /* 97/Mar/7 */
		return 0;			// abortflag set
	}
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if ( poolptr + 1 > poolsize ) {
		overflow ( 257 , poolsize - initpoolptr ) ; /* pool size */
		return;			// abortflag set
	}
#endif
  } 
  p = memtop - 3 ; 
  mem [ p ] .hh .v.RH = 0 ;		/* link(p):=null l.9135 */
  k = b ; 
  while ( k < poolptr ) {
      
    t = strpool [ k ] ; 
    if ( t == 32 ) 
    t = 2592 ; 
    else t = 3072 + t ; 
    {
      {
	q = avail ; 
	if ( q == 0 ) 
	q = getavail () ; 
	else {
	    
	  avail = mem [ q ] .hh .v.RH ; 
	  mem [ q ] .hh .v.RH = 0 ; 
	;
#ifdef STAT
	  incr ( dynused ) ; 
#endif /* STAT */
	} 
      } 
      mem [ p ] .hh .v.RH = q ; 
      mem [ q ] .hh .v.LH = t ; 
      p = q ; 
    } 
    incr ( k ) ; 
  } 
  poolptr = b ; 
  Result = p ; 
  return Result ; 
} 

halfword thetoks ( ) 
{register halfword Result; thetoks_regmem 
  char oldsetting  ; 
  halfword p, q, r  ; 
  poolpointer b  ; 
  getxtoken () ; 
  ABORTCHECKZERO;
  scansomethinginternal ( 5 , false ) ; 
  ABORTCHECKZERO;
  if ( curvallevel >= 4 ) 
  {
    p = memtop - 3 ; 
    mem [ p ] .hh .v.RH = 0 ; 
    if ( curvallevel == 4 ) 
    {
      q = getavail () ; 
      mem [ p ] .hh .v.RH = q ; 
      mem [ q ] .hh .v.LH = 4095 + curval ; 
      p = q ; 
    } 
    else if ( curval != 0 ) /* else if cur_val<>null then l.9176 */
    {
      r = mem [ curval ] .hh .v.RH ; 
      while ( r != 0 ) { /*   while r<>null do l.9178 */
	  
	{
	  {
	    q = avail ; 
	    if ( q == 0 ) 
	    q = getavail () ; 
	    else {
		
	      avail = mem [ q ] .hh .v.RH ; 
	      mem [ q ] .hh .v.RH = 0 ; 
	;
#ifdef STAT
	      incr ( dynused ) ; 
#endif /* STAT */
	    } 
	  } 
	  mem [ p ] .hh .v.RH = q ; 
	  mem [ q ] .hh .v.LH = mem [ r ] .hh .v.LH ; 
	  p = q ; 
	} 
	r = mem [ r ] .hh .v.RH ; 
      } 
    } 
    Result = p ; 
  } 
  else {
      
    oldsetting = selector ;  
    selector = 21 ; 
    b = poolptr ; 
    switch ( curvallevel ) 
    {case 0 : 
      printint ( curval ) ; 
      break ; 
    case 1 : 
      {
	printscaled ( curval ) ; 
	print ( 394 ) ;				/* pt */
      } 
      break ; 
    case 2 : 
      {
	printspec ( curval , 394 ) ; /* pt */
	deleteglueref ( curval ) ; 
      } 
      break ; 
    case 3 : 
      {
	printspec ( curval , 334 ) ; /* mu */
	deleteglueref ( curval ) ; 
      } 
      break ; 
    } 
    selector = oldsetting ; 
    Result = strtoks ( b ) ; 
  } 
  return Result ; 
} 

void insthetoks ( ) 
{insthetoks_regmem 
  mem [ memtop - 12 ] .hh .v.RH = thetoks () ; 
  begintokenlist ( mem [ memtop - 3 ] .hh .v.RH , 4 ) ; 
} 

void convtoks ( ) 
{convtoks_regmem 
  char oldsetting  ; 
  char c  ; 
  smallnumber savescannerstatus  ; 
  poolpointer b  ; 
  c = curchr ; 
  switch ( c ) 
  {case 0 : 
  case 1 : 
  {
    scanint () ;
	ABORTCHECK;
  }
    break ; 
  case 2 : 
  case 3 : 
    {
      savescannerstatus = scannerstatus ;  
      scannerstatus = 0 ; 
      gettoken () ; 
      scannerstatus = savescannerstatus ; 
    } 
    break ; 
  case 4 : 
    scanfontident () ; 
	ABORTCHECK;
    break ; 
  case 5 : 
    if ( jobname == 0 ) openlogfile () ; 
    break ; 
  } 
  oldsetting = selector ; 
  selector = 21 ; 
  b = poolptr ; 
  switch ( c ) 
  {case 0 : 
    printint ( curval ) ; 
    break ; 
  case 1 : 
    printromanint ( curval ) ; 
    break ; 
  case 2 : 
    if ( curcs != 0 ) 
		sprintcs ( curcs ) ; 
    else printchar ( curchr ) ; 
    break ; 
  case 3 : 
    printmeaning () ; 
    break ; 
  case 4 : 
    {
      print ( fontname [ curval ] ) ; 
      if ( fontsize [ curval ] != fontdsize [ curval ] ) 
      {
		  print ( 738 ) ;		/* at  */
		  printscaled ( fontsize [ curval ] ) ; 
		  print ( 394 ) ;		/* pt */
      } 
    } 
    break ; 
  case 5 : 
    print ( jobname ) ; 
    break ; 
  } 
  selector = oldsetting ; 
  mem [ memtop - 12 ] .hh .v.RH = strtoks ( b ) ; 
  begintokenlist ( mem [ memtop - 3 ] .hh .v.RH , 4 ) ; 
} 

halfword zscantoks ( macrodef , xpand ) 
booleane macrodef ; 
booleane xpand ; 
{/* 40 30 31 32 */ register halfword Result; scantoks_regmem 
  halfword t  ; 
  halfword s  ; 
  halfword p  ; 
  halfword q  ; 
  halfword unbalance  ; 
  halfword hashbrace  ; 
  if ( macrodef ) 
  scannerstatus = 2 ; 
  else scannerstatus = 5 ; 
  warningindex = curcs ; 
  defref = getavail () ; 
  mem [ defref ] .hh .v.LH = 0 ; 
  p = defref ; 
  hashbrace = 0 ; 
  t = 3120 ; 
  if ( macrodef ) 
  {
    while ( true ) {
      gettoken () ; 
      if ( curtok < 768 ) 
      goto lab31 ; 
      if ( curcmd == 6 ) 
      {
	s = 3328 + curchr ; 
	gettoken () ; 
	if ( curcmd == 1 ) 
	{
	  hashbrace = curtok ; 
	  {
	    q = getavail () ; 
	    mem [ p ] .hh .v.RH = q ; 
	    mem [ q ] .hh .v.LH = curtok ; 
	    p = q ; 
	  } 
	  {
	    q = getavail () ; 
	    mem [ p ] .hh .v.RH = q ; 
	    mem [ q ] .hh .v.LH = 3584 ; 
	    p = q ; 
	  } 
	  goto lab30 ; 
	} 
	if ( t == 3129 ) 
	{
	  {
	    if ( interaction == 3 ) 
	    ; 
	    printnl ( 262 ) ;	/* !  */
	    print ( 741 ) ;		/* You already have nine parameters */
	  } 
	  {
	    helpptr = 1 ; 
	    helpline [ 0 ] = 742 ; /* I'm going to ignore the # sign you just used. */
	  } 
	  error () ; 
	  ABORTCHECKZERO;
	} 
	else {
	    
	  incr ( t ) ; 
	  if ( curtok != t ) 
	  {
	    {
	      if ( interaction == 3 ) 
	      ; 
	      printnl ( 262 ) ;		/* !  */
	      print ( 743 ) ;		/* Parameters must be numbered consecutively */
	    } 
	    {
	      helpptr = 2 ; 
	      helpline [ 1 ] = 744 ; /* I've inserted the digit you should have used after the #. */
	      helpline [ 0 ] = 745 ; /* Type `1' to delete what you did use. */
	    } 
	    backerror () ; 
		ABORTCHECKZERO;
	  } 
	  curtok = s ; 
	} 
      } 
      {
	q = getavail () ; 
	mem [ p ] .hh .v.RH = q ; 
	mem [ q ] .hh .v.LH = curtok ; 
	p = q ; 
      } 
    } 
    lab31: {
	
      q = getavail () ; 
      mem [ p ] .hh .v.RH = q ; 
      mem [ q ] .hh .v.LH = 3584 ; 
      p = q ; 
    } 
    if ( curcmd == 2 ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 654 ) ;		/* Missing { inserted */
      } 
      incr ( alignstate ) ; 
      {
	helpptr = 2 ; 
	helpline [ 1 ] = 739 ; /* Where was the left brace? You said something like `\def\a}', */
	helpline [ 0 ] = 740 ; /* which I'm going to interpret as `\def\a{}'. */
      } 
      error () ; 
	  ABORTCHECKZERO;
      goto lab40 ; 
    } 
    lab30: ; 
  } 
  else {
	  scanleftbrace () ;
	  ABORTCHECKZERO;
  }
  unbalance = 1 ; 
  while ( true ) {
    if ( xpand ) {
      while ( true ) {
		  getnext () ; 
		  ABORTCHECKZERO;
		  if ( curcmd <= 100 )  goto lab32 ; 
		  if ( curcmd != 109 ) {
			  expand () ;
			  ABORTCHECKZERO;
		  }
		  else {
			  
	  q = thetoks () ; 
/*     if link(temp_head)<>null then l.9376 */
	  if ( mem [ memtop - 3 ] .hh .v.RH != 0 ) 
	  {
	    mem [ p ] .hh .v.RH = mem [ memtop - 3 ] .hh .v.RH ; 
	    p = q ; 
	  } 
	} 
      } 
lab32:
	  xtoken () ; 
	  ABORTCHECKZERO;
    } 
    else gettoken () ; 
    if ( curtok < 768 ) 
    if ( curcmd < 2 )  incr ( unbalance ) ; 
    else {
      decr ( unbalance ) ; 
      if ( unbalance == 0 ) 
      goto lab40 ; 
    } 
    else if ( curcmd == 6 ) 
    if ( macrodef )  {
      s = curtok ; 
      if ( xpand ) getxtoken () ; 
      else gettoken () ; 
	  ABORTCHECKZERO;
      if ( curcmd != 6 ) 
      if ( ( curtok <= 3120 ) || ( curtok > t ) )   {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 746 ) ; /* Illegal parameter number in definition of */
	} 
	sprintcs ( warningindex ) ; 
	{
	  helpptr = 3 ; 
	  helpline [ 2 ] = 747 ; /* You meant to type ## instead of #, right? */
	  helpline [ 1 ] = 748 ; /* Or maybe a } was forgotten somewhere earlier, and things */
	  helpline [ 0 ] = 749 ; /* are all screwed up? I'm going to assume that you meant ##. */
	} 
	backerror () ; 
	ABORTCHECKZERO;
	curtok = s ; 
      } 
      else curtok = 1232 + curchr ; 
    } 
    {
      q = getavail () ; 
      mem [ p ] .hh .v.RH = q ; 
      mem [ q ] .hh .v.LH = curtok ; 
      p = q ; 
    } 
  } 
  lab40: scannerstatus = 0 ; 
  if ( hashbrace != 0 ) 
  {
    q = getavail () ; 
    mem [ p ] .hh .v.RH = q ; 
    mem [ q ] .hh .v.LH = hashbrace ; 
    p = q ; 
  } 
  Result = p ; 
  return Result ; 
} 

/* used only in ITEX.C */

void zreadtoks ( n , r ) 
integer n ; 
halfword r ; 
{/* 30 */ readtoks_regmem 
  halfword p  ; 
  halfword q  ; 
  integer s  ; 
/*  smallnumber m  ;  */
  int m  ;						/* 95/Jan/7 */
  scannerstatus = 2 ; 
  warningindex = r ; 
  defref = getavail () ; 
  mem [ defref ] .hh .v.LH = 0 ; 
  p = defref ; 
  {
	  q = getavail () ; 
	  mem [ p ] .hh .v.RH = q ; 
	  mem [ q ] .hh .v.LH = 3584 ; 
	  p = q ; 
  } 
  if ( ( n < 0 ) || ( n > 15 ) )  m = 16 ; 
  else m = n ; 
  s = alignstate ; 
  alignstate = 1000000L ; 
  do {
      beginfilereading () ; 
	  curinput .namefield = m + 1 ; 
	  if ( readopen [ m ] == 2 ) 
		  if ( interaction > 1 ) 
			  if ( n < 0 )  {
				  ; 
				  print ( 335 ) ;	/* "" */
				  terminput ( 335, 0 ) ; 
				  ABORTCHECK;
			  } 
			  else {
				  ; 
				  println () ; 
				  sprintcs ( r ) ; 
				  {
					  ; 
					  print ( 61 ) ;		/* = */
					  terminput ( 61, 0 ) ; 
					  ABORTCHECK;
				  } 
				  n = -1 ; 
			  } 
		  else {
			  fatalerror ( 750 ) ;	/* *** (cannot \read from terminal in nonstop modes) */
			  return;			// abortflag set
		  }
	  else if ( readopen [ m ] == 1 ) 
		  if ( inputln ( readfile [ m ] , false ) ) 
			  readopen [ m ] = 0 ; 
		  else {
			  (void) aclose ( readfile [ m ] ) ; 
			  readopen [ m ] = 2 ; 
		  } 
	  else {
		  if ( ! inputln ( readfile [ m ] , true ) )  {
			  (void) aclose ( readfile [ m ] ) ; 
			  readopen [ m ] = 2 ; 
			  if ( alignstate != 1000000L )  {
				  runaway () ; 
				  {
					  if ( interaction == 3 ) 
						  ; 
					  printnl ( 262 ) ;	/* !  */
					  print ( 751 ) ;		/* File ended within */
				  } 
				  printesc ( 531 ) ;	/* read */
				  {
					  helpptr = 1 ; 
					  helpline [ 0 ] = 752 ; /* This \read has unbalanced braces. */
				  } 
				  alignstate = 1000000L ; 
				  error () ; 
				  ABORTCHECK;
			  } 
		  } 
	  } 
    curinput .limitfield = last ; 
    if ( ( eqtb [ (hash_size + 3211) ] .cint < 0 ) || ( eqtb [ (hash_size + 3211) ] .cint > 255 ) ) 
    decr ( curinput .limitfield ) ; 
/*	long to unsigned char ... */
    else buffer [ curinput .limitfield ] = eqtb [ (hash_size + 3211) ] .cint ; 
    first = curinput .limitfield + 1 ; 
    curinput .locfield = curinput .startfield ; 
    curinput .statefield = 33 ; 
    while ( true ) {
      gettoken () ; 
      if ( curtok == 0 ) 
		  goto lab30 ; 
      if ( alignstate < 1000000L ) 
      {
	do {
	    gettoken () ; 
	} while ( ! ( curtok == 0 ) ) ; 
	alignstate = 1000000L ; 
	goto lab30 ; 
      } 
      {
	q = getavail () ; 
	mem [ p ] .hh .v.RH = q ; 
	mem [ q ] .hh .v.LH = curtok ; 
	p = q ; 
      } 
    } 
    lab30: endfilereading () ; 
  } while ( ! ( alignstate == 1000000L ) ) ; 
  curval = defref ; 
  scannerstatus = 0 ; 
  alignstate = s ; 
} 

void passtext ( ) 
{/* 30 */ passtext_regmem 
  integer l  ; 
  smallnumber savescannerstatus  ; 
  savescannerstatus = scannerstatus ;  
  scannerstatus = 1 ; 
  l = 0 ; 
  skipline = line ; 
  while ( true ) {
	  getnext () ; 
	  ABORTCHECK;
	  if ( curcmd == 106 ) {
		  if ( l == 0 )  goto lab30 ; 
		  if ( curchr == 2 ) 
			  decr ( l ) ; 
	  } 
	  else if ( curcmd == 105 )  incr ( l ) ; 
  } 
lab30: scannerstatus = savescannerstatus ; 
} 

void zchangeiflimit ( l , p ) 
smallnumber l ; 
halfword p ; 
{/* 10 */ changeiflimit_regmem 
  halfword q  ; 
  if ( p == condptr ) 
  iflimit = l ; 
  else {
    q = condptr ; 
    while ( true ) {
      if ( q == 0 )	{	/* begin if q=null then confusion("if"); l.9674 */
		  confusion ( 753 ) ;	/* if */
		  return;				// abortflag set
	  }
      if ( mem [ q ] .hh .v.RH == p ) 
      {
	mem [ q ] .hh.b0 = l ; 
	return ; 
      } 
      q = mem [ q ] .hh .v.RH ; 
    } 
  } 
} 

/* called from tex2.c */

void conditional ( ) 
{/* 10 50 */ conditional_regmem 
  booleane b  ; 
  char r  ; 
  integer m, n  ; 
  halfword p, q  ; 
  smallnumber savescannerstatus  ; 
  halfword savecondptr  ; 
  smallnumber thisif  ; 

  {
/* begin p:=get_node(if_node_size); */
    p = getnode ( 2 ) ;					/* p <- get_node(if_node_size); p.495*/
/* link(p):=cond_ptr; */
    mem [ p ] .hh .v.RH = condptr ; 
/* type(p):=if_limit; */
    mem [ p ] .hh.b0 = iflimit ;  
/* subtype(p):=cur_if; */
    mem [ p ] .hh.b1 = curif ;  
/* if_line_field(p):=if_line; */
    mem [ p + 1 ] .cint = ifline ; 
    condptr = p ; 
    curif = curchr ; 
    iflimit = 1 ; 
    ifline = line ; 
  } 
  savecondptr = condptr ;				/* save_cond_ptr <- cond_ptr; p.498 */
  thisif = curchr ; 
  switch ( thisif ) 
  {case 0 : 
  case 1 : 
    {
      {
	getxtoken () ; 
	ABORTCHECK;
	if ( curcmd == 0 )		/* if cur_cmd = relax then .. p.506 */
	if ( curchr == 257 )	/* if cur_chr = no_expand_flag then ... p.506 */
	{
	  curcmd = 13 ; 
	  curchr = curtok - 4096 ; 
	} 
      } 
      if ( ( curcmd > 13 ) || ( curchr > 255 ) ) 
      {
	m = 0 ; 
	n = 256 ; 
      } 
      else {
	  
	m = curcmd ; 
	n = curchr ; 
      } 
      {
	getxtoken () ; 
	ABORTCHECK;
	if ( curcmd == 0 )		/* if cur_cmd = relax then .. p.506 */
	if ( curchr == 257 )	/* if cur_chr = no_expand_flag then ... p.506 */
	{
	  curcmd = 13 ; 
	  curchr = curtok - 4096 ; 
	} 
      } 
      if ( ( curcmd > 13 ) || ( curchr > 255 ) ) 
      {
	curcmd = 0 ; 
	curchr = 256 ; 
      } 
      if ( thisif == 0 ) 
      b = ( n == curchr ) ; 
      else b = ( m == curcmd ) ; 
    } 
    break ; 
  case 2 : 
  case 3 : 
    {
      if ( thisif == 2 ) {
		  scanint () ;
		  ABORTCHECK;
	  }
      else {
		  scandimen ( false , false , false ) ;
		  ABORTCHECK;
	  }
      n = curval ; 
      do {
		  getxtoken () ; 
		  ABORTCHECK;
      } while ( ! ( curcmd != 10 ) ) ; 
      if ( ( curtok >= 3132 ) && ( curtok <= 3134 ) ) 
      r = curtok - 3072 ; 
      else {
	  
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 777 ) ;		/* Missing = inserted for  */
	} 
	printcmdchr ( 105 , thisif ) ;	/* i */
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 778 ; /* I was expecting to see `<', `=', or `>'. Didn't. */
	} 
	backerror () ; 
	ABORTCHECK;
	r = 61 ; 
      } 
      if ( thisif == 2 ) {
		  scanint () ;
		  ABORTCHECK;
	  }
      else {
		  scandimen ( false , false , false ) ;
		  ABORTCHECK;
	  }	  
      switch ( r ) 
      {case 60 : 
	b = ( n < curval ) ; 
	break ; 
      case 61 : 
	b = ( n == curval ) ; 
	break ; 
      case 62 : 
	b = ( n > curval ) ; 
	break ; 
      } 
    } 
    break ; 
  case 4 : 
    {
      scanint () ; 
	  ABORTCHECK;
      b = odd ( curval ) ; 
    } 
    break ; 
  case 5 : 
    b = ( abs ( curlist .modefield ) == 1 ) ; 
    break ; 
  case 6 : 
    b = ( abs ( curlist .modefield ) == 102 ) ; 
    break ; 
  case 7 : 
    b = ( abs ( curlist .modefield ) == 203 ) ; 
    break ; 
  case 8 : 
    b = ( curlist .modefield < 0 ) ; 
    break ; 
  case 9 : 
  case 10 : 
  case 11 : 
    {
      scaneightbitint () ; 
	  ABORTCHECK;
      p = eqtb [ (hash_size + 1578) + curval ] .hh .v.RH ; 
      if ( thisif == 9 ) 
      b = ( p == 0 ) ; 
      else if ( p == 0 ) 
      b = false ; 
      else if ( thisif == 10 ) 
      b = ( mem [ p ] .hh.b0 == 0 ) ; 
      else b = ( mem [ p ] .hh.b0 == 1 ) ; 
    } 
    break ; 
  case 12 : 
    {
      savescannerstatus = scannerstatus ;  
      scannerstatus = 0 ; 
      getnext () ; 
	  ABORTCHECK;
      n = curcs ; 
      p = curcmd ; 
      q = curchr ; 
      getnext () ; 
	  ABORTCHECK;
      if ( curcmd != p )   b = false ; 
      else if ( curcmd < 111 )   b = ( curchr == q ) ; 
      else {
	  
	p = mem [ curchr ] .hh .v.RH ; 
	q = mem [ eqtb [ n ] .hh .v.RH ] .hh .v.RH ; 
	if ( p == q ) 
	b = true ; 
	else {
/* else begin while (p<>null)and(q<>null) do l.9840 */
	  while ( ( p != 0 ) && ( q != 0 ) ) if ( mem [ p ] .hh .v.LH != mem [ 
	  q ] .hh .v.LH ) 
	  p = 0 ;	/* p:=null */
	  else {
	      
	    p = mem [ p ] .hh .v.RH ; 
	    q = mem [ q ] .hh .v.RH ; 
	  } 
	  b = ( ( p == 0 ) && ( q == 0 ) ) ;  /*   b:=((p=null)and(q=null)); */
	} 
      } 
      scannerstatus = savescannerstatus ; 
    } 
    break ; 
  case 13 : 
    {
      scanfourbitint () ; 
	  ABORTCHECK;
      b = ( readopen [ curval ] == 2 ) ; 
    } 
    break ; 
  case 14 : 
    b = true ; 
    break ; 
  case 15 : 
    b = false ; 
    break ; 
  case 16 : 
    {
      scanint () ; 
	  ABORTCHECK;
      n = curval ; 
      if ( eqtb [ (hash_size + 3199) ] .cint > 1 ) 
      {
	begindiagnostic () ; 
	print ( 779 ) ; /* {case */
	printint ( n ) ; 
	printchar ( 125 ) ;		/* } */
	enddiagnostic ( false ) ; 
      } 
      while ( n != 0 ) {
	  
	passtext () ; 
	if ( condptr == savecondptr ) 
	if ( curchr == 4 ) 
	decr ( n ) ; 
	else goto lab50 ; 
	else if ( curchr == 2 ) 
	{
	  p = condptr ; 
	  ifline = mem [ p + 1 ] .cint ; 
	  curif = mem [ p ] .hh.b1 ; 
	  iflimit = mem [ p ] .hh.b0 ; 
	  condptr = mem [ p ] .hh .v.RH ; 
	  freenode ( p , 2 ) ; 
	} 
      } 
      changeiflimit ( 4 , savecondptr ) ; 
      return ; 
    } 
    break ; 
  } 
  if ( eqtb [ (hash_size + 3199) ] .cint > 1 ) 
  {
    begindiagnostic () ; 
    if ( b ) 
		print ( 775 ) ;		/* {true}*/
    else print ( 776 ) ;	/* {false}*/
    enddiagnostic ( false ) ; 
  } 
  if ( b )			/* b may be used without ... */
  {
    changeiflimit ( 3 , savecondptr ) ; 
    return ; 
  } 
  while ( true ) {
    passtext () ; 
    if ( condptr == savecondptr ) 
    {
      if ( curchr != 4 ) 
      goto lab50 ; 
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 773 ) ;		/* Extra */
      } 
      printesc ( 771 ) ; /* or */
      {
	helpptr = 1 ; 
	helpline [ 0 ] = 774 ; /* I'm ignoring this; it doesn't match any \if. */
      } 
      error () ; 
	  ABORTCHECK;
    } 
    else if ( curchr == 2 ) 
    {
      p = condptr ; 
      ifline = mem [ p + 1 ] .cint ; 
      curif = mem [ p ] .hh.b1 ; 
      iflimit = mem [ p ] .hh.b0 ; 
      condptr = mem [ p ] .hh .v.RH ; 
      freenode ( p , 2 ) ; 
    } 
  } 
  lab50: if ( curchr == 2 ) 
  {
    p = condptr ; 
    ifline = mem [ p + 1 ] .cint ; 
    curif = mem [ p ] .hh.b1 ; 
    iflimit = mem [ p ] .hh.b0 ; 
    condptr = mem [ p ] .hh .v.RH ; 
    freenode ( p , 2 ) ; 
  } 
  else iflimit = 2 ; 
} 

void beginname ( ) 
{beginname_regmem 
  areadelimiter = 0 ;	/* index between `file area' and `file name' */
  extdelimiter = 0 ;	/* index between `file name' and `file extension' */
} 

/* This gathers up a file name and makes a string of it */
/* Also tries to break it into `file area' `file name' and `file extension' */
/* Used by scanfilename and promptfilename */
/* We assume tilde has been converted to pseudotilde and space to pseudospace */
/* returns false if it is given a space character - end of file name */

booleane zmorename ( c ) 
ASCIIcode c ; 
{register booleane Result; morename_regmem 
   
/*  if ( c == 32 ) */		/* white space delimits file name ... */
  if (quotedfilename == 0 && c == 32)
		Result = false ; 
  else if (quotedfilename != 0 && c == '"') {
	    quotedfilename = 0;	/* catch next space character */
		Result = true ;		/* accept ending quote, but throw away */
  }
  else {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*	convert pseudo tilde back to '~' 95/Sep/26 */ /* moved here 97/June/5 */
/*	if (pseudotilde != 0 && c == pseudotilde) c = '~'; */
/*	convert pseudo space back to ' ' 97/June/5 */ /* moved here 97/June/5 */
/*	if (pseudospace != 0 && c == pseudospace) c = ' '; */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */      
    {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
      if ( poolptr + 1 > currentpoolsize)
		 strpool = reallocstrpool (incrementpoolsize);
      if ( poolptr + 1 > currentpoolsize) {	/* in case it failed 94/Jan/24 */
		  overflow ( 257 , currentpoolsize - initpoolptr ) ; /* 97/Mar/7 */
		  return 0;			// abortflag set
	  }
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
      if ( poolptr + 1 > poolsize ) {
		  overflow ( 257 , poolsize - initpoolptr ) ; /* pool size */
		  return 0;			// abortflag set
	  }
#endif
    } 
    {
      strpool [ poolptr ] = c ; 
      incr ( poolptr ) ; 
    } 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
//  if ( ( c == 47 ) )		/* / */
//	for DOS/Windows
    if ( ( c == '/' || c == '\\' || c == ':' ) )	/* 94/Mar/1 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    {
		areadelimiter = ( poolptr - strstart [ strptr ] ) ; 
		extdelimiter = 0 ; 
    } 
    else if ( c == 46 )		/* . */
		extdelimiter = ( poolptr - strstart [ strptr ] ) ; 
    Result = true ; 
  } 
  return Result ; 
} 

/********************************* 2000 August 15th start ***********************/

// The following code is to save string space used by TeX for filenames
// Not really critical in a system that has dynamic memory allocation
// And may slow it down slightly - although this linear search only
// occurs when opening a file, which is somewhat slow inany case...

// see if string from strpool[start] to strpool[end]
// occurs elsewhere in string pool - returns string number
// returns -1 if not found in string pool 2000 Aug 15

int find_string (int start, int end) {
	int k, nlen=end-start;
	char *s;

//	int traceflag = 1;			// debugging only

	if (traceflag) {
		sprintf(logline, "\nLOOKING for string (strptr %d nlen %d) ", strptr, end-start);
		s = logline + strlen(logline);
		strncpy(s, strpool+start, nlen);
		strcpy(s+nlen, "");
		showline(logline, 0);
	}

//  avoid problems with ( curname == flushablestring ) by going only up to strptr-1
//  code in newfont (tex8.c) will take care of reuse of font name already
//	for (k = 0; k < strptr; k++) {
	for (k = 0; k < strptr-1; k++) {
		if ((strstart[k+1] - strstart[k]) != nlen) continue;
		if (strncmp(strpool+start, strpool+strstart[k], nlen) == 0) {
			if (traceflag) {
				sprintf(logline, "\nFOUND the string %d (%d) ", k, strstart[k+1]-strstart[k]);
				s = logline + strlen(logline);
				strncpy(s, strpool+start, nlen);
				strcpy(s+nlen, "\n");
				showline(logline, 0);
			}
			return k;			// return number of matching string
		}
	}
	if (traceflag) {
		sprintf(logline, "\nNOT FOUND string ");
		s = logline + strlen(logline);
		strncpy(s, strpool+start, nlen);
		strcpy(s+nlen, "\n");
		showline(logline, 0);
	}
	return -1;					// no match found
}

// snip out the string from strpool[start] to strpool[end]
// and move everything above it down 2000 Aug 15

void remove_string (int start, int end) {
	int nlen = poolptr-end;	// how many bytes to move down
	char *s;
	
//	int traceflag=1;		// debugging only
//	if (end < start) showline("\nEND < START", 1);
//	if (poolptr < end) showline("\nPOOLPTR < END", 1);

	if (traceflag) {
		int n = end-start;
		sprintf(logline, "\nSTRIPPING OUT %d %d ", n, nlen);
		s = logline + strlen(logline);
		strncpy(s, strpool+start, n);
		strcpy(s+n, "\n");
		showline(logline, 0);
	}
	if (nlen > 0) memcpy(strpool+start, strpool+end, nlen);
	poolptr = start + nlen;		// poolprt - (end-start);
}

void show_string (int k) {		// debugging code
	int nlen = strstart[k+1] - strstart[k];
	char *s;
	
	sprintf(logline, "\nSTRING %5d (%3d) %5d--%5d ",
			k, nlen, strstart[k], strstart[k+1]);
	s = logline + strlen(logline);			
	strncpy(s, strpool+strstart[k], nlen);
	strcpy(s + nlen, "");
	showline(logline, 0);
}

void show_all_strings (void) {		// debugging code
	int k;
	for (k = 0; k < strptr; k++) show_string(k);
}

// int notfirst=0;		// debugging only

/********************************** 2000 August 15 end ****************************/

void endname ( ) 
{endname_regmem 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
  if ( strptr + 3 > currentmaxstrings ) 
/*	  strstart = reallocstrstart ( incrementmaxstrings); */
	  strstart = reallocstrstart ( incrementmaxstrings + 3);
  if ( strptr + 3 > currentmaxstrings ) {	/* in case it failed 94/Jan/24 */
	  overflow ( 258 , currentmaxstrings - initstrptr ) ;  /* 97/Mar/7 */
	  return;			// abortflag set
  }
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( strptr + 3 > maxstrings ) {
	  overflow ( 258 , maxstrings - initstrptr ) ; /* number of strings */
	  return;			// abortflag set
  }
#endif

//  if (notfirst++ == 0) show_all_strings();	// debugging only
  
  if ( areadelimiter == 0 )		// no area delimiter ':' '/' or '\' found
	  curarea = 335 ;			// "" default area 
  else {
	  if (savestringsflag &&
			(curarea = find_string(strstart[strptr], strstart[strptr]+areadelimiter)) > 0) {
		  remove_string(strstart[strptr], strstart[strptr]+areadelimiter);
		  areadelimiter = 0;	// areadelimiter - areadelimiter;
		  if (extdelimiter != 0) extdelimiter = extdelimiter - areadelimiter;
//		  strstart [ strptr + 1 ] = strstart [ strptr ] + areadelimiter; // test only
//		  incr ( strptr ) ;		// test only
	  }
	  else {					// carve out string for "curarea"
		  curarea = strptr ; 
		  strstart [ strptr + 1 ] = strstart [ strptr ] + areadelimiter ; 
		  incr ( strptr ) ;
	  }
  } 
  if ( extdelimiter == 0 ) {	// no extension delimiter '.' found
    curext = 335 ;				// "" default extension 
	if (savestringsflag &&
		  (curname = find_string(strstart[strptr], poolptr)) > 0) {
		remove_string(strstart[strptr], poolptr);
//		(void) makestring () ;	// test only
	}
	else 						// Make string from strstart [ strptr ] to poolptr
		curname = makestring () ;
  } 
  else {						// did find an extension
	if (savestringsflag &&
		  (curname = find_string(strstart[strptr], strstart[strptr]+extdelimiter-areadelimiter-1)) > 0) {
		remove_string(strstart[strptr], strstart[strptr]+extdelimiter-areadelimiter-1);
//		strstart [ strptr + 1 ] = strstart [ strptr ] + extdelimiter - areadelimiter - 1 ; 	// test only
//		incr ( strptr ) ;		// test only
	}
	else {						// carve out string for "curname"
		curname = strptr ; 
		strstart [ strptr + 1 ] = strstart [ strptr ] + extdelimiter - areadelimiter - 1 ; 
		incr ( strptr ) ;
	}
	if (savestringsflag &&
		  (curext = find_string(strstart[strptr], poolptr)) > 0) {
		remove_string(strstart[strptr], poolptr);
//		(void) makestring () ;	// test only
	}
	else 						// Make string from strstart [ strptr ] to poolptr
		curext = makestring () ;
  }
}

/* n current name, a current area, e current extension */
/* result in nameoffile[] */

void zpackfilename ( n , a , e ) 
strnumber n ; 
strnumber a ; 
strnumber e ; 
{packfilename_regmem 
  integer k  ; 
  ASCIIcode c  ; 
  poolpointer j  ; 
  k = 0 ; 
  {register integer for_end; j = strstart [ a ] ;
	  for_end = strstart [ a + 1 ] - 1 ; if ( j <= for_end) do 
    {
      c = strpool [ j ] ; 
      incr ( k ) ; 
      if ( k <= PATHMAX ) 
		  nameoffile [ k ] = xchr [ c ] ; 
    } 
  while ( j++ < for_end ) ; } 
  {register integer for_end; j = strstart [ n ] ;
	  for_end = strstart [ n + 1 ] - 1 ; if ( j <= for_end) do 
    {
      c = strpool [ j ] ; 
      incr ( k ) ; 
      if ( k <= PATHMAX ) 
		  nameoffile [ k ] = xchr [ c ] ; 
    } 
  while ( j++ < for_end ) ; } 
  {register integer for_end; j = strstart [ e ] ;
	  for_end = strstart [ e + 1 ] - 1 ; if ( j <= for_end) do 
    {
      c = strpool [ j ] ; 
      incr ( k ) ; 
      if ( k <= PATHMAX ) 
		  nameoffile [ k ] = xchr [ c ] ; 
    } 
  while ( j++ < for_end ) ; } 
  if ( k < PATHMAX ) namelength = k ; 
  else namelength = PATHMAX - 1 ; 
/*	pad it out with spaces ... what for ? in case we modify and forget  ? */
  {register integer for_end; k = namelength + 1 ; for_end = PATHMAX ;
  if ( k <= for_end) do nameoffile [ k ] = ' ' ; 
  while ( k++ < for_end ) ; } 
  nameoffile [ PATHMAX ] = '\0';		/* paranoia 94/Mar/24 */
  {
	  nameoffile [namelength+1] = '\0';
	  if (traceflag) {
		  sprintf(logline, " packfilename `%s' (%d) ",
				  nameoffile+1, namelength);	/* debugging */
		  showline(logline, 0);
	  }
	  nameoffile [namelength+1] = ' ';
  }
} 

/* Called only from two places tex9.c for format name - specified and default */
/* for specified format name args are 0, a, b name in buffer[a] --- buffer[b] */
/* for default args are formatdefaultlength-4, 1, 0 */
void zpackbufferedname ( n , a , b ) 
smallnumber n ; 
integer a ; 
integer b ; 
{packbufferedname_regmem 
  integer k  ; 
  ASCIIcode c  ; 
  integer j  ; 
  if ( n + b - a + 5 > PATHMAX ) 
  b = a + PATHMAX - n - 5 ; 
  k = 0 ; 
/*	This loop kicks in when we want the default format name */
  {register integer for_end; j = 1 ; for_end = n ; if ( j <= for_end) do 
    {
      c = xord [ TEXformatdefault [ j ] ] ; 
      incr ( k ) ; 
      if ( k <= PATHMAX ) 
      nameoffile [ k ] = xchr [ c ] ; 
    } 
  while ( j++ < for_end ) ; } 
/*	This loop kicks in when we want a specififed format name */
  {register integer for_end; j = a ; for_end = b ; if ( j <= for_end) do 
    {
      c = buffer [ j ] ; 
      incr ( k ) ; 
      if ( k <= PATHMAX ) 
      nameoffile [ k ] = xchr [ c ] ; 
    } 
  while ( j++ < for_end ) ; } 
/*  This adds the extension from the default format name */
  {register integer for_end; j = formatdefaultlength - 3 ; for_end = 
  formatdefaultlength ; if ( j <= for_end) do 
    {
      c = xord [ TEXformatdefault [ j ] ] ; 
      incr ( k ) ; 
      if ( k <= PATHMAX ) 
      nameoffile [ k ] = xchr [ c ] ; 
    } 
  while ( j++ < for_end ) ; } 
  if ( k < PATHMAX ) 
  namelength = k ; 
  else namelength = PATHMAX - 1 ; 
/*	pad it out with spaces ... what for ? */
  {register integer for_end; k = namelength + 1 ; for_end = PATHMAX ; if ( k 
  <= for_end) do 
    nameoffile [ k ] = ' ' ; 
  while ( k++ < for_end ) ; } 
  nameoffile [ PATHMAX ] = '\0';		/* paranoia 94/Mar/24 */
} 

strnumber makenamestring ( ) 
{register strnumber Result; makenamestring_regmem 
  integer k  ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
	if ( poolptr + namelength > currentpoolsize)
/*		strpool = reallocstrpool (incrementpoolsize); */
		strpool = reallocstrpool (incrementpoolsize + namelength);
	if ( strptr == currentmaxstrings ) 
		strstart = reallocstrstart ( incrementmaxstrings);
	if ( ( poolptr + namelength > currentpoolsize ) ||
		( strptr == currentmaxstrings ) ||
			( ( poolptr - strstart [ strptr ] ) > 0 ) ) 
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( ( poolptr + namelength > poolsize ) || ( strptr == maxstrings ) ||
	   ( ( poolptr - strstart [ strptr ] ) > 0 ) ) 
#endif
  Result = 63 ; 
  else {
      
    {register integer for_end; k = 1 ; for_end = namelength ; if ( k <=  for_end) do 
      {
		  strpool [ poolptr ] = xord [ nameoffile [ k ] ] ; 
//		  sprintf(logline, "%d => %d ", nameoffile[k], xord[nameoffile[k]]);
//		  showline(logline, 0);	// debugging only
		  incr ( poolptr ) ; 
      } 
    while ( k++ < for_end ) ; } 
    Result = makestring () ; 
  } 
  return Result ; 
} 

strnumber zamakenamestring ( f ) 
alphafile * f ; 
{register strnumber Result; amakenamestring_regmem 
  Result = makenamestring () ; 
  return Result ; 
} 	/* f unreferenced ? bkph */

strnumber zbmakenamestring ( f ) 
bytefile * f ; 
{register strnumber Result; bmakenamestring_regmem 
  Result = makenamestring () ; 
  return Result ; 
} 	/* f unreferenced ? bkph */

strnumber zwmakenamestring ( f ) 
wordfile * f ; 
{register strnumber Result; wmakenamestring_regmem 
  Result = makenamestring () ; 
  return Result ; 
} 	/* f unreferenced ? bkph */

/* Used by startinput to scan file name on command line */
/* Also in tex8.c znewfont, openorclosein, and doextension */

void scanfilename ( ) 
{/* 30 */ scanfilename_regmem 
  nameinprogress = true ; 
  beginname () ; 
  do {
	  getxtoken () ; 
	  ABORTCHECK;
  } while ( ! ( curcmd != 10 ) ) ;		/* until cur_cmd != spacer */
  quotedfilename = 0;					/* 98/March/15 */
  if (allowquotednames) {				/* check whether quoted name */
	  if ( curchr == '"') {
		  quotedfilename = 1;
		  getxtoken () ; 
		  ABORTCHECK;
	  }	
  }
  while ( true ) {
    if ( ( curcmd > 12 ) || ( curchr > 255 ) ) 
    {					/* (cur_cmd > otherchar) OR (cur_chr > 255) */
      backinput () ;	/* not a character put it back and leave */
      goto lab30 ; 
    } 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*	convert tilde '~' to pseudo tilde */
/*	if (pseudotilde != 0 && curchr == '~') curchr = pseudotilde; */
/*	convert space ' ' to pseudo space */
/*	if (pseudospace != 0 && curchr == ' ') curchr = pseudospace; */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */      
    if ( ! morename ( curchr ) )		/* up to next white space */
		goto lab30 ; 
    getxtoken () ; 
	ABORTCHECK;
  } 
lab30:
  endname () ;
  nameinprogress = false ; 
} 

/* argument is string .fmt, .log, or .dvi */

void zpackjobname ( s ) 
strnumber s ; 
{packjobname_regmem 
  curarea = 335 ;				/* "" */
  curext = s ; 
  curname = jobname ; 
  packfilename ( curname , curarea , curext ) ; 
} 

/**********************************************************************/

/* show TEXINPUTS=... or format specific  */
/* only show this if name was not fully qualified ? */
void showtexinputs (void) {			/* 98/Jan/28 */
	char *s, *t, *v;
	s = "TEXINPUTS";				/* default */
	if (formatspecific) {
		s = formatname;								/* try specific */
		if (grabenv(s) == NULL) s = "TEXINPUTS";	/* no format specific */
	}

	if (grabenv(s) == NULL) s = "TEXINPUT";			/* 94/May/19 */

	printnl ( 32 );			/*   */
	printchar ( 32 );		/*   */
	printchar ( 40 );		/* ( */
	t = s;
	while (*t > '\0') printchar( *t++ );
	printchar ( 61 );		/* = */
	v = grabenv(s);
	if (v != NULL) {
		t = v;
		while (*t > '\0') printchar( *t++ );
	}
	printchar ( 41 );		/* ) */
}

/**********************************************************************/

void zpromptfilename ( s , e ) /*  s - what can't be found, e - default */
strnumber s ; 
strnumber e ; 
{/* 30 */ promptfilename_regmem 
  integer k  ; 
  if ( interaction == 2 ) 
	  ; 
  if ( s == 781 ) {		/* input file name */
    if ( interaction == 3 ) ; 
    printnl ( 262 ) ;	/* ! */
    print ( 782 ) ;		/* I can't find file ` */
  } 
  else {
    if ( interaction == 3 ) ; 
    printnl ( 262 ) ;	/*  ! */
    print ( 783 ) ;		/* I can't write on file ` */
  } 
  printfilename ( curname , curarea , curext ) ; 
  print ( 784 ) ;		/* '. */
  if ( s == 781 ) {		/* input file name */
	  if (curarea == 335) {		/* "" only if path not specified */
		  if (showtexinputflag) showtexinputs();
	  }
  }
  if ( e == 785 )		/* .tex */
	  showcontext () ; 
  printnl ( 786 ) ;		/* Please type another  */
  print ( s ) ; 
  if ( interaction < 2 ) {
      fatalerror ( 787 ) ; /* *** (job aborted, file error in nonstop mode) */
	  return;			// abortflag set
  }
  if (! knuthflag)
#ifdef _WINDOWS
//	  showline(" (or ^Z to exit)\n", 0);
	  showline(" (or ^z to exit)", 0);
#else
	  showline(" (or Ctrl-Z to exit)", 0);
#endif
  {
    ; 
    print ( 565 ) ;		/* : */
    terminput ( 565, 0 ) ; 
	ABORTCHECK;
  } 
/*	should we deal with tilde and space in file name here ??? */
  {
    beginname () ; 
    k = first ; 
/*	step over leading spaces ... */
    while ( ( buffer [ k ] == 32 ) && ( k < last ) ) incr ( k ) ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	quotedfilename = 0;					/* 98/March/15 */
	if (allowquotednames && k < last) {	/* check whether quoted name */
		if ( buffer [ k ] == '"') {
			quotedfilename = 1;
			incr ( k );
		}
	}
    while ( true ) {
      if ( k == last )  goto lab30 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*	convert tilde '~' to pseudo tilde */
	if (pseudotilde != 0 && buffer [ k ] == '~') buffer [ k ] = pseudotilde;
/*	convert space ' ' to pseudo space */
	if (pseudospace != 0 && buffer [ k ] == ' ') buffer [ k ] = pseudospace;
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */      
      if ( ! morename ( buffer [ k ] ) ) goto lab30 ; 
      incr ( k ) ; 
    } 
    lab30: endname () ; 
  } 
  if ( curext == 335 )		/* "" */
	  curext = e ;			/* use default extension */
  packfilename ( curname , curarea , curext ) ; 
} 

void openlogfile ( ) 
{openlogfile_regmem 
  char oldsetting  ; 
  integer k  ; 
  integer l  ; 
  ccharpointer months  ; 

  oldsetting = selector ;  

  if ( jobname == 0 )  jobname = 790 ;		/* default:  texput */
  packjobname ( 791 ) ;		/* .log */
  while ( ! aopenout ( logfile ) ) {
    selector = 17 ; 
    promptfilename ( 793 , 791 ) ;	/* transcript file name  texput */
	ABORTCHECK;
  } 
  texmflogname = amakenamestring ( logfile ) ; 
  selector = 18 ;					/* log file only */
  logopened = true ; 
  {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
//	for our version DOS/Windows
	if (want_version) {
//		showversion (logfile);				/* in local.c - bkph */
//		showversion (stdout);
		stampit(logline);					// ??? use logline ???
		strcat(logline, "\n");
		(void) fputs( logline , logfile );
//		showline(buffer, 0);				// ??? show on screen as well
//		println (); 
		stampcopy(logline);
		strcat(logline, "\n");
//		showline(buffer, 0);				// ??? show on screen as well
		(void) fputs( logline , logfile );
//		println (); 
	}
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*	also change following in itex.c - bkph */
	(void) fputs( texversion ,  logfile ) ; 
	(void) fprintf( logfile , " (%s %s)", application, yandyversion);
	if (formatident > 0) slowprint ( formatident ) ; 		/* bkph */
    print ( 794 ) ; /* '  ' (i.e. two spaces) */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	if (civilizeflag) printint ( eqtb [ (hash_size + 3186) ] .cint ) ; /* year */
    else 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
		printint ( eqtb [ (hash_size + 3184) ] .cint ) ;		/* day */
    printchar ( 32 ) ;		/*   */
    months = " JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC" ; 
    {register integer for_end; k = 3 * eqtb [ (hash_size + 3185) ] .cint - 2 ; for_end = 3 
    * eqtb [ (hash_size + 3185) ] .cint ; if ( k <= for_end) do 
      (void) putc ( months [ k ] ,  logfile );
    while ( k++ < for_end ) ; }				/* month */
    printchar ( 32 ) ;		/*   */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	if (civilizeflag) printint ( eqtb [ (hash_size + 3184) ] .cint ) ;/* day */
	else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
		printint ( eqtb [ (hash_size + 3186) ] .cint ) ;	/* year */
    printchar ( 32 ) ;		/*   */
    printtwo ( eqtb [ (hash_size + 3183) ] .cint / 60 ) ;	/* hour */
    printchar ( 58 ) ;		/* : */
    printtwo ( eqtb [ (hash_size + 3183) ] .cint % 60 ) ;	/* minute */
  } 
  inputstack [ inputptr ] = curinput ; 
  printnl ( 792 ) ;			/* ** */
  l = inputstack [ 0 ] .limitfield ; 
  if ( buffer [ l ] == eqtb [ (hash_size + 3211) ] .cint ) 
  decr ( l ) ; 
  {register integer for_end; k = 1 ; for_end = l ; if ( k <= for_end) do 
    print ( buffer [ k ] ) ; 
  while ( k++ < for_end ) ; } 
  println () ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* a good place to show the fmt file name or pool file name ? 94/June/21 */
  if (showfmtflag) {
	  if (stringfile != NULL) {
		  fprintf(logfile, "(%s)\n", stringfile);
		  free(stringfile); 	/* this was allocated by strdup in openinou */
		  stringfile = NULL; 	/* for safety */
	  }
	  if (formatfile != NULL) {
		  fprintf(logfile, "(%s)\n", formatfile);
		  free(formatfile); 	/* this was allocated by strdup in openinou */
		  formatfile = NULL; 	/* for safety */
	  }
  }
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  selector = oldsetting + 2 ; 
} 

/**************************** start of insertion 98/Feb/7 **************/

// Attempt to deal with foo.bar.tex given as foo.bar on command line
// Makes copy of jobname with extension

void morenamecopy ( ASCIIcode c ) {
#ifdef ALLOCATESTRING
	if ( poolptr + 1 > currentpoolsize)
		strpool = reallocstrpool (incrementpoolsize);
	if ( poolptr + 1 > currentpoolsize)	 { /* in case it failed 94/Jan/24 */
		overflow ( 257 , currentpoolsize - initpoolptr ) ; /* 97/Mar/7 */
		return;			// abortflag set
	}
#else
	if ( poolptr + 1 > poolsize ) {
		overflow ( 257 , poolsize - initpoolptr ) ; /* pool size */
		return;			// abortflag set
	}
#endif
	strpool [ poolptr ] = c ; 
	incr ( poolptr ) ; 
} 

int endnamecopy ( void ) {
#ifdef ALLOCATESTRING
	  if ( strptr + 1 > currentmaxstrings ) 
		  strstart = reallocstrstart ( incrementmaxstrings + 1);
	  if ( strptr + 1 > currentmaxstrings )	{	/* in case it failed 94/Jan/24 */
		  overflow ( 258 , currentmaxstrings - initstrptr ) ;  /* 97/Mar/7 */
		  return 0;			// abortflag set
	  }
#else
	  if ( strptr + 1 > maxstrings ) {
		  overflow ( 258 , maxstrings - initstrptr ) ; /* number of strings */
		  return 0;			// abortflag set
	  }
#endif
	  return makestring () ;
} 

void jobnameappend (void) {	/* add extension to jobname */
	int k, n;
/*	copy jobname */
	k = strstart [ jobname ];
	n = strstart [ jobname + 1];
	while (k < n) morenamecopy ( strpool [ k++ ]);
/*  copy `extension' */
	k = strstart [ curext ];
	n = strstart [ curext + 1];
	while (k < n) morenamecopy ( strpool [ k++ ]);
	jobname = endnamecopy ();
}

/**************************** end of insertion 98/Feb/7 **************/

void startinput ( ) 
{/* 30 */ startinput_regmem 
  booleane addedextension = false;
  scanfilename () ; 
  packfilename ( curname , curarea , curext ) ; 

  while ( true ) {				/* loop until we get a valid file name */      
    addedextension = false;
    beginfilereading () ; 
/* *** *** *** *** *** following is new in 3.14159 *** *** *** *** *** *** */
/*	if current extension is *not* empty, try to open using name as is */
/*	string 335 is "" the empty string */
    if ( ( curext != 335 ) && aopenin ( inputfile [ curinput .indexfield ] , 
			TEXINPUTPATH ) ) 
    goto lab30 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*	we get here if extension is "", or file with extension failed to open */
/*	if current extension is not `tex,' and `tex' is not irrelevant, try it */
/*	string 785 is .tex */
    if ( ( curext != 785 ) && ( namelength + 5 < PATHMAX ) && 
/*		 ( ! extensionirrelevantp ( nameoffile , "tex" ) ) ) { */
		 ( ! extensionirrelevantp ( nameoffile , namelength , "tex" ) ) ) {
      nameoffile [ namelength + 1 ] = 46 ;	/* .tex  */
      nameoffile [ namelength + 2 ] = 116 ; 
      nameoffile [ namelength + 3 ] = 101 ; 
      nameoffile [ namelength + 4 ] = 120 ; 
      nameoffile [ namelength + 5 ] = 32 ;	/* 96/Jan/20 ??? */
      namelength = namelength + 4 ; 
	  addedextension = true;
/* *** *** *** ***  following new in 3.14159 *** *** *** *** *** *** *** */
      if ( aopenin ( inputfile [ curinput .indexfield ] , TEXINPUTPATH ) ) 
		  goto lab30 ; 
      namelength = namelength - 4 ;			/* strip extension again */
      nameoffile [ namelength + 1 ] = 32 ;	/* ' ' */
	  addedextension = false;
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    } 
/* *** *** *** *** major changes here in 3.14159 *** *** *** *** *** *** */
/*	string 335 is "" the empty string */
    if ( ( curext == 335 ) && aopenin ( inputfile [ curinput .indexfield ] , 
			TEXINPUTPATH ) ) 
    goto lab30 ; 
    if ( maketextex () && aopenin ( inputfile [ curinput .indexfield ] , 
			TEXINPUTPATH ) ) 
    goto lab30 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    endfilereading () ; 
    promptfilename ( 781 , 785 ) ;	/* input file name  .tex */
	ABORTCHECK;
  }		/* end of while ( true ) trying to get valid file name */

/* maybe set  pseudotilde = 0  at this point ? 95/Sep/26 */
  lab30: curinput .namefield =
		amakenamestring ( inputfile [ curinput .indexfield ] ) ; 
  if ( jobname == 0 )				/* only the first time */
  {
    jobname = curname ;				/* here we set the jobname */
/*	did file name have an `extension' already and we added ".tex" ? */
	if (curext != 335 && addedextension)			/* 98/Feb/7 */
		jobnameappend ();		/* append `extension' to jobname */
    openlogfile () ; 
  } 
  if ( termoffset + ( strstart [ curinput .namefield + 1 ] - strstart [ 
  curinput .namefield ] ) > maxprintline - 2 )	/* was 3 ? */  
  println () ; 
  else if ( ( termoffset > 0 ) || ( fileoffset > 0 ) ) 
  printchar ( 32 ) ;			/*   */
  printchar ( 40 ) ;			/* ( */
//  printchar ( 64 );				// debugging only marker
  incr ( openparens ) ; 
  if ( openparens > maxopenparens)
	  maxopenparens = openparens; 		/* 1999/Jan/17 */
  slowprint ( curinput .namefield ) ; 
//  printchar ( 64 );				// debugging only marker
#ifndef _WINDOWS
  fflush ( stdout ) ; 
#endif
  curinput .statefield = 33 ; 
  {
    line = 1 ; 
    if ( inputln ( inputfile [ curinput .indexfield ] , false ) ) 
		; 
    firmuptheline () ; 
	ABORTCHECK;
    if ( ( eqtb [ (hash_size + 3211) ] .cint < 0 ) ||
		 ( eqtb [ (hash_size + 3211) ] .cint > 255 ) ) 
    decr ( curinput .limitfield ) ; 
/*	long to unsigned char ... */
    else buffer [ curinput .limitfield ] = eqtb [ (hash_size + 3211) ] .cint ; 
    first = curinput .limitfield + 1 ; 
    curinput .locfield = curinput .startfield ; 
  } 
} 

/**********************************************************************/
/* show TEXFONTS=... or format specific  */
/* only show this if name was not fully qualified ? */
void showtexfonts(void) {			/* 98/Jan/28 */
	char *s, *t, *v, *u;
	int n;
	s = "TEXFONTS";
	if (encodingspecific) {
		u = encodingname;								/* try specific */
		if ((t = grabenv(u)) != NULL) {
			if (strchr(t, ':') != NULL &&
				sscanf(t, "%d", &n) == 0) {
				s = u;				/* look here instead of TEXFONTS=... */
			}
		}
	}
	printnl ( 32 );			/*   */
	printchar ( 32 );		/*   */
	printchar ( 40 );		/* ( */
	t = s;
	while (*t > '\0') printchar( *t++ );
	printchar ( 61 );		/* = */
	v = grabenv(s);
	if (v != NULL) {
		t = v;
		while (*t > '\0') printchar( *t++ );
	}
	printchar ( 41 );		/* ) */
}

/**********************************************************************/

/* called only from tex8.c */

internalfontnumber zreadfontinfo ( u , nom , aire , s ) 
halfword u ; 
strnumber nom ; 
strnumber aire ; 
scaled s ; 
{/* 30 11 45 */ register internalfontnumber Result; readfontinfo_regmem 
  fontindex k  ; 
  booleane fileopened  ; 
/*  halfword lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np  ;  */
  halfword lf, lh, nw, nh, nd, ni, nl, nk, ne, np  ;
/*  halfword bc, ec; */
  int bc, ec;							/* 95/Jan/7 */
  internalfontnumber f  ; 
  internalfontnumber g  ; 
  eightbits a, b, c, d  ; 
  ffourquarters qw  ; 
  scaled sw  ; 
  integer bchlabel  ; 
  short bchar  ; 
  scaled z  ; 
  integer alpha  ; 
  char beta  ; 
  g = 0 ; 
  fileopened = false ; 
  packfilename ( nom , aire , 805 ) ;	/* .tfm */
  if ( ! bopenin ( tfmfile ) ) 
  { 	/* new in C version d */
    if ( maketextfm () ) 
    {
      if ( ! bopenin ( tfmfile ) ) 
		  goto lab11 ; 
    } 
    else goto lab11 ; 
  } 
/*   was just: goto lab11 ; */
  fileopened = true ; 
  {
/*  tfmtemp = getc ( tfmfile ) ;  */ /* done already in open_input, but why? */
    {
      lf = tfmtemp ; 
      if ( lf > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      lf = lf * 256 + tfmtemp ; 
    } 
	tfmtemp = getc ( tfmfile ) ; 
    {
      lh = tfmtemp ; 
      if ( lh > 127 ) 
      goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      lh = lh * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      bc = tfmtemp ; 
      if ( bc > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      bc = bc * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      ec = tfmtemp ; 
      if ( ec > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      ec = ec * 256 + tfmtemp ; 
    } 
    if ( ( bc > ec + 1 ) || ( ec > 255 ) ) 
		goto lab11 ; 
    if ( bc > 255 ) 
    {
      bc = 1 ; 
      ec = 0 ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      nw = tfmtemp ; 
      if ( nw > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      nw = nw * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      nh = tfmtemp ; 
      if ( nh > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      nh = nh * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      nd = tfmtemp ; 
      if ( nd > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      nd = nd * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      ni = tfmtemp ; 
      if ( ni > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      ni = ni * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      nl = tfmtemp ; 
      if ( nl > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      nl = nl * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      nk = tfmtemp ; 
      if ( nk > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      nk = nk * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      ne = tfmtemp ; 
      if ( ne > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      ne = ne * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      np = tfmtemp ; 
      if ( np > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      np = np * 256 + tfmtemp ; 
    } 
    if ( lf != 6 + lh + ( ec - bc + 1 ) + nw + nh + nd + ni + nl + nk + ne + 
    np ) 
		goto lab11 ; 
  } 
  lf = lf - 6 - lh ; 
  if ( np < 7 ) 
	  lf = lf + 7 - np ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEFONT
  if ( ( fmemptr + lf > currentfontmemsize ) ) 	/* 93/Nov/28 */
	  fontinfo = reallocfontinfo (incrementfontmemsize + lf);
  if ( ( fontptr == fontmax ) || ( fmemptr + lf > currentfontmemsize ) ) 
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( ( fontptr == fontmax ) || ( fmemptr + lf > fontmemsize ) ) 
#endif
  {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

	  if (traceflag) {
		  sprintf(logline, "fontptr %d fontmax %d fmemptr %d lf %d fontmemsize %d\n",
				  fontptr, fontmax, fmemptr, lf, fontmemsize);	/* debugging */
		  showline(logline, 0);
	  }

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* ! */
      print ( 796 ) ;		/* Font  */
    } 
    sprintcs ( u ) ; 
    printchar ( 61 ) ;		/* = */
    printfilename ( nom , aire , 335 ) ; /* "" */
    if ( s >= 0 ) 
    {
      print ( 738 ) ;	/*  at  */
      printscaled ( s ) ; 
      print ( 394 ) ;	/* pt */
    } 
    else if ( s != -1000 ) 
    {
      print ( 797 ) ;	/*  scaled  */
      printint ( - (integer) s ) ; 
    } 
    print ( 806 ) ;		/*  not loaded: Not enough room left */
    {
      helpptr = 4 ; 
      helpline [ 3 ] = 807 ;	/* I'm afraid I won't be able to make use of this font, */
      helpline [ 2 ] = 808 ;	/* because my memory for character-size data is too small. */
      helpline [ 1 ] = 809 ;	/* If you're really stuck, ask a wizard to enlarge me. */
      helpline [ 0 ] = 810 ;	/* Or maybe try `I\font<same font id>=<name of loaded font>'. */
    } 
    error () ; 
	ABORTCHECKZERO;
    goto lab30 ; 
  } 
  f = fontptr + 1 ; 
  charbase [ f ] = fmemptr - bc ; 
  widthbase [ f ] = charbase [ f ] + ec + 1 ; 
  heightbase [ f ] = widthbase [ f ] + nw ; 
  depthbase [ f ] = heightbase [ f ] + nh ; 
  italicbase [ f ] = depthbase [ f ] + nd ; 
  ligkernbase [ f ] = italicbase [ f ] + ni ; 
  kernbase [ f ] = ligkernbase [ f ] + nl - 256 * ( 128 ) ; 
  extenbase [ f ] = kernbase [ f ] + 256 * ( 128 ) + nk ; 
  parambase [ f ] = extenbase [ f ] + ne ; 
  {
    if ( lh < 2 ) 
		goto lab11 ; 
/*	build the font checksum now */
    {
      tfmtemp = getc ( tfmfile ) ; 
      a = tfmtemp ; 
      qw .b0 = a ; 
      tfmtemp = getc ( tfmfile ) ; 
      b = tfmtemp ; 
      qw .b1 = b ; 
      tfmtemp = getc ( tfmfile ) ; 
      c = tfmtemp ; 
      qw .b2 = c ; 
      tfmtemp = getc ( tfmfile ) ; 
      d = tfmtemp ; 
      qw .b3 = d ; 
      fontcheck [ f ] = qw ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    {
      z = tfmtemp ; 
      if ( z > 127 ) 
		  goto lab11 ; 
      tfmtemp = getc ( tfmfile ) ; 
      z = z * 256 + tfmtemp ; 
    } 
    tfmtemp = getc ( tfmfile ) ; 
    z = z * 256 + tfmtemp ; 
    tfmtemp = getc ( tfmfile ) ; 
    z = ( z * 16 ) + ( tfmtemp / 16 ) ; 
    if ( z < 65536L ) 
		goto lab11 ; 
    while ( lh > 2 ) {
      tfmtemp = getc ( tfmfile ) ; 
      tfmtemp = getc ( tfmfile ) ; 
      tfmtemp = getc ( tfmfile ) ; 
      tfmtemp = getc ( tfmfile ) ; 
      decr ( lh ) ; 
    } 
    fontdsize [ f ] = z ; 
    if ( s != -1000 ) 
    if ( s >= 0 ) 
		z = s ; 
    else z = xnoverd ( z , - (integer) s , 1000 ) ; 
    fontsize [ f ] = z ; 
  } 
  {register integer for_end; k = fmemptr ; for_end = widthbase [ f ] - 1 
  ; if ( k <= for_end) do 
    {
      {
	tfmtemp = getc ( tfmfile ) ; 
	a = tfmtemp ; 
	qw .b0 = a ; 
	tfmtemp = getc ( tfmfile ) ; 
	b = tfmtemp ; 
	qw .b1 = b ; 
	tfmtemp = getc ( tfmfile ) ; 
	c = tfmtemp ; 
	qw .b2 = c ; 
	tfmtemp = getc ( tfmfile ) ; 
	d = tfmtemp ; 
	qw .b3 = d ; 
	fontinfo [ k ] .qqqq = qw ; 
      } 
      if ( ( a >= nw ) || ( b / 16 >= nh ) || ( b % 16 >= nd ) || ( c / 4 >= 
      ni ) ) 
		  goto lab11 ; 
      switch ( c % 4 ) 
      {case 1 : 
	if ( d >= nl ) 
		  goto lab11 ; 
	break ; 
      case 3 : 
	if ( d >= ne ) 
		goto lab11 ; 
	break ; 
      case 2 : 
	{
	  {
	    if ( ( d < bc ) || ( d > ec ) ) 
			goto lab11 ; 
	  } 
	  while ( d < k + bc - fmemptr ) {
	      
	    qw = fontinfo [ charbase [ f ] + d ] .qqqq ; 
	    if ( ( ( qw .b2 ) % 4 ) != 2 ) 
	    goto lab45 ; 
	    d = qw .b3 ; 
	  } 
	  if ( d == k + bc - fmemptr ) 
		  goto lab11 ; 
	  lab45: ; 
	} 
	break ; 
	default: 
	; 
	break ; 
      } 
    } 
  while ( k++ < for_end ) ; } 
  {
    {
      alpha = 16 ; 
      while ( z >= 8388608L ) {		/* 2^23 */
	  
	z = z / 2 ; 
	alpha = alpha + alpha ; 
      } 
/*      beta = 256 / alpha ;  */ /* keep compiler happy */
      beta = (char) (256 / alpha) ; 
      alpha = alpha * z ; 
    } 
    {register integer for_end; k = widthbase [ f ] ; for_end = ligkernbase [ 
    f ] - 1 ; if ( k <= for_end) do 
      {
	tfmtemp = getc ( tfmfile ) ; 
	a = tfmtemp ; 
	tfmtemp = getc ( tfmfile ) ; 
	b = tfmtemp ; 
	tfmtemp = getc ( tfmfile ) ; 
	c = tfmtemp ; 
	tfmtemp = getc ( tfmfile ) ; 
	d = tfmtemp ; 
	sw = ( ( ( ( ( d * z ) / 256 ) + ( c * z ) ) / 256 ) + ( b * z ) ) / beta ; 
	if ( a == 0 ) 
	fontinfo [ k ] .cint = sw ; 
	else if ( a == 255 ) 
	fontinfo [ k ] .cint = sw - alpha ; 
	else goto lab11 ; 
      } 
    while ( k++ < for_end ) ; } 
    if ( fontinfo [ widthbase [ f ] ] .cint != 0 ) 
		goto lab11 ; 
    if ( fontinfo [ heightbase [ f ] ] .cint != 0 ) 
		goto lab11 ; 
    if ( fontinfo [ depthbase [ f ] ] .cint != 0 ) 
		goto lab11 ; 
    if ( fontinfo [ italicbase [ f ] ] .cint != 0 ) 
		goto lab11 ; 
  } 
/*  read ligature/kern program */
  bchlabel = 32767 ;			/* '77777 */
  bchar = 256 ; 
  if ( nl > 0 ) 
  {
/*   begin for k:=lig_kern_base[f] to kern_base[f]+kern_base_offset-1 do */
    {register integer for_end; k = ligkernbase [ f ] ; for_end = kernbase [ f 
    ] + 256 * ( 128 ) - 1 ; if ( k <= for_end) do 
      {
	{
	  tfmtemp = getc ( tfmfile ) ; 
	  a = tfmtemp ; 
	  qw .b0 = a ; 
	  tfmtemp = getc ( tfmfile ) ; 
	  b = tfmtemp ; 
	  qw .b1 = b ; 
	  tfmtemp = getc ( tfmfile ) ; 
	  c = tfmtemp ; 
	  qw .b2 = c ; 
	  tfmtemp = getc ( tfmfile ) ; 
	  d = tfmtemp ; 
	  qw .b3 = d ; 
	  fontinfo [ k ] .qqqq = qw ; /* store_four_quarters(font_info[k].qqqq */
	} 
	if ( a > 128 ) 
	{
	  if ( 256 * c + d >= nl ) 
		  goto lab11 ;				/* error in TFM, abort */
	  if ( a == 255 ) 
		  if ( k == ligkernbase [ f ] ) 
			  bchar = b ; 
	} 
	else {
	  if ( b != bchar ) 
	  {
	    {
	      if ( ( b < bc ) || ( b > ec ) )	/* check-existence(b) */
			  goto lab11 ;					/* error in TFM, abort */
	    } 
	    qw = fontinfo [ charbase [ f ] + b ] .qqqq ; 
	    if ( ! ( qw .b0 > 0 ) ) 
			goto lab11 ;					/* error in TFM, abort */
	  } 
	  if ( c < 128 ) 
	  {
	    {
	      if ( ( d < bc ) || ( d > ec ) )	/* check-existence(d) */
			  goto lab11 ;  				/* error in TFM, abort */
	    } 
	    qw = fontinfo [ charbase [ f ] + d ] .qqqq ; 
	    if ( ! ( qw .b0 > 0 ) ) 
			goto lab11 ;	 				/* error in TFM, abort */
	  } 
	  else if ( 256 * ( c - 128 ) + d >= nk ) 
		  goto lab11 ;		 				/* error in TFM, abort */
	  if ( a < 128 ) 
		  if ( k - ligkernbase [ f ] + a + 1 >= nl ) 
			  goto lab11 ;  				/* error in TFM, abort */
	} 
      } 
    while ( k++ < for_end ) ; } 
    if ( a == 255 ) 
		bchlabel = 256 * c + d ; 
  } 
/*  for k:=kern_base[f]+kern_base_offset to exten_base[f]-1 do */
/*	  store_scaled(font_info[k].sc); */
  {register integer for_end; k = kernbase [ f ] + 256 * ( 128 ) ; for_end = 
  extenbase [ f ] - 1 ; if ( k <= for_end) do 
    {
      tfmtemp = getc ( tfmfile ) ; 
      a = tfmtemp ; 
      tfmtemp = getc ( tfmfile ) ; 
      b = tfmtemp ; 
      tfmtemp = getc ( tfmfile ) ; 
      c = tfmtemp ; 
      tfmtemp = getc ( tfmfile ) ; 
      d = tfmtemp ; 
      sw = ( ( ( ( ( d * z ) / 256 ) + ( c * z ) ) / 256 ) + ( b * z ) ) / beta ; 
      if ( a == 0 ) 
		  fontinfo [ k ] .cint = sw ; 
      else if ( a == 255 ) 
		  fontinfo [ k ] .cint = sw - alpha ; 
      else goto lab11 ; 
    } 
  while ( k++ < for_end ) ; } 
/*	read extensible character recipes */
/*  for k:=exten_base[f] to param_base[f]-1 do */
  {register integer for_end; k = extenbase [ f ] ; for_end = parambase [ f ] 
  - 1 ; if ( k <= for_end) do 
    {
      {
	tfmtemp = getc ( tfmfile ) ; 
	a = tfmtemp ; 
	qw .b0 = a ; 
	tfmtemp = getc ( tfmfile ) ; 
	b = tfmtemp ; 
	qw .b1 = b ; 
	tfmtemp = getc ( tfmfile ) ; 
	c = tfmtemp ; 
	qw .b2 = c ; 
	tfmtemp = getc ( tfmfile ) ; 
	d = tfmtemp ; 
	qw .b3 = d ; 
/*	  store_four_quarters(font_info[k].qqqq); */
	fontinfo [ k ] .qqqq = qw ; 
      } 
      if ( a != 0 ) 
      {
	{
	  if ( ( a < bc ) || ( a > ec ) ) 
		  goto lab11 ; 
	} 
	qw = fontinfo [ charbase [ f ] + a ] .qqqq ; 
	if ( ! ( qw .b0 > 0 ) ) 
		goto lab11 ; 
      } 
      if ( b != 0 ) 
      {
	{
	  if ( ( b < bc ) || ( b > ec ) ) 
		  goto lab11 ; 
	} 
	qw = fontinfo [ charbase [ f ] + b ] .qqqq ; 
	if ( ! ( qw .b0 > 0 ) ) 
		goto lab11 ; 
      } 
      if ( c != 0 ) 
      {
	{
	  if ( ( c < bc ) || ( c > ec ) ) 
		  goto lab11 ; 
	} 
	qw = fontinfo [ charbase [ f ] + c ] .qqqq ; 
	if ( ! ( qw .b0 > 0 ) ) 
		goto lab11 ; 
      } 
      {
	{
	  if ( ( d < bc ) || ( d > ec ) ) 
		  goto lab11 ; 
	} 
	qw = fontinfo [ charbase [ f ] + d ] .qqqq ; 
	if ( ! ( qw .b0 > 0 ) ) 
		goto lab11 ; 
      } 
    } 
  while ( k++ < for_end ) ; } 
  {
    {register integer for_end; k = 1 ; for_end = np ; if ( k <= for_end) do 
      if ( k == 1 ) 
      {
	tfmtemp = getc ( tfmfile ) ; 
	sw = tfmtemp ; 
	if ( sw > 127 ) 
		sw = sw - 256 ; 
	tfmtemp = getc ( tfmfile ) ; 
	sw = sw * 256 + tfmtemp ; 
	tfmtemp = getc ( tfmfile ) ; 
	sw = sw * 256 + tfmtemp ; 
	tfmtemp = getc ( tfmfile ) ; 
	fontinfo [ parambase [ f ] ] .cint = ( sw * 16 ) + ( tfmtemp / 16 ) ; 
      } 
      else {
	  
	tfmtemp = getc ( tfmfile ) ; 
	a = tfmtemp ; 
	tfmtemp = getc ( tfmfile ) ; 
	b = tfmtemp ; 
	tfmtemp = getc ( tfmfile ) ; 
	c = tfmtemp ; 
	tfmtemp = getc ( tfmfile ) ; 
	d = tfmtemp ; 
	sw = ( ( ( ( ( d * z ) / 256 ) + ( c * z ) ) / 256 ) + ( b * z ) ) / beta ; 
	if ( a == 0 ) 
		fontinfo [ parambase [ f ] + k - 1 ] .cint = sw ; 
	else if ( a == 255 ) 
		fontinfo [ parambase [ f ] + k - 1 ] .cint = sw - alpha ; 
	else goto lab11 ; 
      } 
    while ( k++ < for_end ) ; } 
/*	use test_eof ( ) here instead ? */
    if ( feof ( tfmfile ) ) 
		goto lab11 ; 
    {register integer for_end; k = np + 1 ; for_end = 7 ; if ( k <= for_end) 
    do 
      fontinfo [ parambase [ f ] + k - 1 ] .cint = 0 ; 
    while ( k++ < for_end ) ; } 
  } 
/* @<Make final adjustments...@>= l.11174 */
  if ( np >= 7 ) 
  fontparams [ f ] = np ; 
  else fontparams [ f ] = 7 ; 
  hyphenchar [ f ] = eqtb [ (hash_size + 3209) ] .cint ; /*  default_hyphen_char */
  skewchar [ f ] = eqtb [ (hash_size + 3210) ] .cint ; /*  default_skew_char */
  if ( bchlabel < nl ) 
  bcharlabel [ f ] = bchlabel + ligkernbase [ f ] ; 
/* bchar_label[f]:=non_address; */ /* 3.14159 */
/*  else bcharlabel [ f ] = fontmemsize ; */		/* OK ??? 93/Nov/28 */
  else bcharlabel [ f ] = non_address ; /* i.e. 0 --- 96/Jan/15 */
  fontbchar [ f ] = bchar ; 
  fontfalsebchar [ f ] = bchar ; 
  if ( bchar <= ec ) 
  if ( bchar >= bc ) 
  {
    qw = fontinfo [ charbase [ f ] + bchar ] .qqqq ; 
    if ( ( qw .b0 > 0 ) ) 
    fontfalsebchar [ f ] = 256 ; 
  } 
  fontname [ f ] = nom ; 
  fontarea [ f ] = aire ; 
  fontbc [ f ] = bc ; 
  fontec [ f ] = ec ; 
  fontglue [ f ] = 0 ;	/* font_glue[f]:=null; l.11184 */
  charbase [ f ] = charbase [ f ] ; 
  widthbase [ f ] = widthbase [ f ] ; 
  ligkernbase [ f ] = ligkernbase [ f ] ; 
  kernbase [ f ] = kernbase [ f ] ; 
  extenbase [ f ] = extenbase [ f ] ; 
  decr ( parambase [ f ] ) ; 
  fmemptr = fmemptr + lf ; 
  fontptr = f ; 
  g = f ; 
  goto lab30 ; 

  lab11: {				/* error in reading font TFM */
    if ( interaction == 3 )
		; 
    printnl ( 262 ) ;	/* !  */
    print ( 796 ) ;		/* Font */
  } 
  sprintcs ( u ) ; 
  printchar ( 61 ) ;	/* = */
  printfilename ( nom , aire , 335 ) ;	/* "" */
  if ( s >= 0 ) 
  {
    print ( 738 ) ;		/* at */
    printscaled ( s ) ; 
    print ( 394 ) ;		/* pt */
  } 
  else if ( s != -1000 ) 
  {
    print ( 797 ) ;		/* scaled */
    printint ( - (integer) s ) ; 
  } 
  if ( fileopened ) print ( 798 ) ;	/* not loadable: Bad metric (TFM) file */
  else print ( 799 ) ;	/* not loadable: Metric (TFM) file not found */
  if (aire == 335) {		/* "" only if path not specified */
	  if (showtexinputflag) showtexfonts();		/* 98/Jan/31 */
  }
  {
    helpptr = 5 ; 
    helpline [ 4 ] = 800 ;	/* I wasn't able to read the size data for this font, */
    helpline [ 3 ] = 801 ;	/* so I will ignore the font specification. */
    helpline [ 2 ] = 802 ;	/* [Wizards can fix TFM files using TFtoPL/PLtoTF.] */
    helpline [ 1 ] = 803 ;	/* You might try inserting a different font spec; */
    helpline [ 0 ] = 804 ;	/* e.g., type `I\font<same font id>=<substitute font name>'. */
  } 
  error () ; 
 // ABORTCHECKZERO;

lab30:
  if ( fileopened ) bclose ( tfmfile ) ; 
  Result = g ; 
  return Result ; 
}
