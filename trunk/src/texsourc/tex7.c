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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void buildpage ( ) 
{/* 10 30 31 22 80 90 */ buildpage_regmem 
  halfword p  ; 
  halfword q, r  ; 
  integer b, c  ; 
  integer pi  ; 
/*  unsigned char n  ;  */
  unsigned int n  ;							/* 95/Jan/7 */
  scaled delta, h, w  ; 

  ABORTCHECK;

/* begin if (link(contrib_head)=null)or output_active then return; l.19351 */
  if ( ( mem [ memtop - 1 ] .hh .v.RH == 0 ) || outputactive ) 
  return ; 
  do {
      lab22: p = mem [ memtop - 1 ] .hh .v.RH ; 
/*    if ( lastglue != 262143L )  */
    if ( lastglue != emptyflag ) 
    deleteglueref ( lastglue ) ; 
    lastpenalty = 0 ; 
    lastkern = 0 ; 
    if ( mem [ p ] .hh.b0 == 10 ) 
    {
      lastglue = mem [ p + 1 ] .hh .v.LH ; 
      incr ( mem [ lastglue ] .hh .v.RH ) ; 
    } 
    else {
	
/*      lastglue = 262143L ;  */
      lastglue = emptyflag ; 
      if ( mem [ p ] .hh.b0 == 12 ) 
      lastpenalty = mem [ p + 1 ] .cint ; 
      else if ( mem [ p ] .hh.b0 == 11 ) 
      lastkern = mem [ p + 1 ] .cint ; 
    } 
    switch ( mem [ p ] .hh.b0 ) 
    {case 0 : 
    case 1 : 
    case 2 : 
      if ( pagecontents < 2 ) 
      {
	if ( pagecontents == 0 ) 
	freezepagespecs ( 2 ) ; 
	else pagecontents = 2 ; 
	q = newskipparam ( 9 ) ; 
	if ( mem [ tempptr + 1 ] .cint > mem [ p + 3 ] .cint ) 
	mem [ tempptr + 1 ] .cint = mem [ tempptr + 1 ] .cint - mem [ p + 3 ] 
	.cint ; 
	else mem [ tempptr + 1 ] .cint = 0 ; 
	mem [ q ] .hh .v.RH = p ; 
	mem [ memtop - 1 ] .hh .v.RH = q ; 
	goto lab22 ; 
      } 
      else {
	  
	pagesofar [ 1 ] = pagesofar [ 1 ] + pagesofar [ 7 ] + mem [ p + 3 ] 
	.cint ; 
	pagesofar [ 7 ] = mem [ p + 2 ] .cint ; 
	goto lab80 ; 
      } 
      break ; 
    case 8 : 
      goto lab80 ; 
      break ; 
    case 10 : 
      if ( pagecontents < 2 ) 
      goto lab31 ; 
      else if ( ( mem [ pagetail ] .hh.b0 < 9 ) ) 
      pi = 0 ; 
      else goto lab90 ; 
      break ; 
    case 11 : 
      if ( pagecontents < 2 ) 
      goto lab31 ; 
      else if ( mem [ p ] .hh .v.RH == 0 ) 
      return ; 
      else if ( mem [ mem [ p ] .hh .v.RH ] .hh.b0 == 10 ) 
      pi = 0 ; 
      else goto lab90 ; 
      break ; 
    case 12 : 
      if ( pagecontents < 2 ) 
      goto lab31 ; 
      else pi = mem [ p + 1 ] .cint ; 
      break ; 
    case 4 : 
      goto lab80 ; 
      break ; 
    case 3 : 
      {
	if ( pagecontents == 0 ) 
	freezepagespecs ( 1 ) ; 
	n = mem [ p ] .hh.b1 ; 
	r = memtop ; 
	while ( n >= mem [ mem [ r ] .hh .v.RH ] .hh.b1 ) r = mem [ r ] .hh 
	.v.RH ; 
	n = n ; 
	if ( mem [ r ] .hh.b1 != n ) 
	{
	  q = getnode ( 4 ) ; 
	  mem [ q ] .hh .v.RH = mem [ r ] .hh .v.RH ; 
	  mem [ r ] .hh .v.RH = q ; 
	  r = q ; 
	  mem [ r ] .hh.b1 = n ; 
	  mem [ r ] .hh.b0 = 0 ; 
	  ensurevbox ( n ) ; 
	  ABORTCHECK;
	  if ( eqtb [ (hash_size + 1578) + n ] .hh .v.RH == 0 ) 
	  mem [ r + 3 ] .cint = 0 ; 
	  else mem [ r + 3 ] .cint = mem [ eqtb [ (hash_size + 1578) + n ] .hh .v.RH + 3 ] 
	  .cint + mem [ eqtb [ (hash_size + 1578) + n ] .hh .v.RH + 2 ] .cint ; 
	  mem [ r + 2 ] .hh .v.LH = 0 ; 
	  q = eqtb [ (hash_size + 800) + n ] .hh .v.RH ; 
	  if ( eqtb [ (hash_size + 3218) + n ] .cint == 1000 ) 
	  h = mem [ r + 3 ] .cint ; 
	  else h = xovern ( mem [ r + 3 ] .cint , 1000 ) * eqtb [ (hash_size + 3218) + n ] 
	  .cint ; 
	  pagesofar [ 0 ] = pagesofar [ 0 ] - h - mem [ q + 1 ] .cint ; 
	  pagesofar [ 2 + mem [ q ] .hh.b0 ] = pagesofar [ 2 + mem [ q ] 
	  .hh.b0 ] + mem [ q + 2 ] .cint ; 
	  pagesofar [ 6 ] = pagesofar [ 6 ] + mem [ q + 3 ] .cint ; 
	  if ( ( mem [ q ] .hh.b1 != 0 ) && ( mem [ q + 3 ] .cint != 0 ) ) 
	  {
	    {
	      if ( interaction == 3 ) 
	      ; 
	      printnl ( 262 ) ;		/* ! */
	      print ( 992 ) ;		/* Infinite glue shrinkage inserted from */
	    } 
	    printesc ( 392 ) ;		/* skip*/
	    printint ( n ) ; 
	    {
	      helpptr = 3 ; 
	      helpline [ 2 ] = 993 ;	/* The correction glue for page breaking with insertions */
	      helpline [ 1 ] = 994 ;	/* must have finite shrinkability. But you may proceed, */
	      helpline [ 0 ] = 916 ;	/* since the offensive shrinkability has been made finite. */
	    } 
	    error () ; 
		ABORTCHECK;
	  } 
	} 
	if ( mem [ r ] .hh.b0 == 1 ) 
	insertpenalties = insertpenalties + mem [ p + 1 ] .cint ; 
	else {
	    
	  mem [ r + 2 ] .hh .v.RH = p ; 
	  delta = pagesofar [ 0 ] - pagesofar [ 1 ] - pagesofar [ 7 ] + 
	  pagesofar [ 6 ] ; 
	  if ( eqtb [ (hash_size + 3218) + n ] .cint == 1000 ) 
	  h = mem [ p + 3 ] .cint ; 
	  else h = xovern ( mem [ p + 3 ] .cint , 1000 ) * eqtb [ (hash_size + 3218) + n ] 
	  .cint ; 
	  if ( ( ( h <= 0 ) || ( h <= delta ) ) && ( mem [ p + 3 ] .cint + mem 
	  [ r + 3 ] .cint <= eqtb [ (hash_size + 3751) + n ] .cint ) ) 
	  {
	    pagesofar [ 0 ] = pagesofar [ 0 ] - h ; 
	    mem [ r + 3 ] .cint = mem [ r + 3 ] .cint + mem [ p + 3 ] .cint ; 
	  } 
	  else {
	      
	    if ( eqtb [ (hash_size + 3218) + n ] .cint <= 0 ) 
	    w = 1073741823L ;  /* 2^30 - 1 */
	    else {
		
	      w = pagesofar [ 0 ] - pagesofar [ 1 ] - pagesofar [ 7 ] ; 
	      if ( eqtb [ (hash_size + 3218) + n ] .cint != 1000 ) 
	      w = xovern ( w , eqtb [ (hash_size + 3218) + n ] .cint ) * 1000 ; 
	    } 
	    if ( w > eqtb [ (hash_size + 3751) + n ] .cint - mem [ r + 3 ] .cint ) 
	    w = eqtb [ (hash_size + 3751) + n ] .cint - mem [ r + 3 ] .cint ; 
	    q = vertbreak ( mem [ p + 4 ] .hh .v.LH , w , mem [ p + 2 ] .cint ) ; 
		ABORTCHECK;
	    mem [ r + 3 ] .cint = mem [ r + 3 ] .cint + bestheightplusdepth ; 
	;
#ifdef STAT
	    if ( eqtb [ (hash_size + 3196) ] .cint > 0 ) 
	    {
	      begindiagnostic () ; 
	      printnl ( 995 ) ;		/* % split */
	      printint ( n ) ; 
	      print ( 996 ) ;		/*  to */
	      printscaled ( w ) ; 
	      printchar ( 44 ) ;	/* , */
	      printscaled ( bestheightplusdepth ) ; 
	      print ( 925 ) ;		/*  p= */
	      if ( q == 0 )			/* if q=null l.19614 */
			  printint ( -10000 ) ; 
	      else if ( mem [ q ] .hh.b0 == 12 ) 
			  printint ( mem [ q + 1 ] .cint ) ; 
	      else printchar ( 48 ) ;	/* 0 */
	      enddiagnostic ( false ) ; 
	    } 
#endif /* STAT */
	    if ( eqtb [ (hash_size + 3218) + n ] .cint != 1000 ) 
	    bestheightplusdepth = xovern ( bestheightplusdepth , 1000 ) *
		eqtb [ (hash_size + 3218) + n ] .cint ; 
	    pagesofar [ 0 ] = pagesofar [ 0 ] - bestheightplusdepth ; 
	    mem [ r ] .hh.b0 = 1 ; 
	    mem [ r + 1 ] .hh .v.RH = q ; 
	    mem [ r + 1 ] .hh .v.LH = p ; 
	    if ( q == 0 ) 
	    insertpenalties = insertpenalties - 10000 ; 
	    else if ( mem [ q ] .hh.b0 == 12 ) 
	    insertpenalties = insertpenalties + mem [ q + 1 ] .cint ; 
	  } 
	} 
	goto lab80 ; 
      } 
	  break ; 
      default:
	  {
		  confusion ( 987 ) ;	/* page */
		  return;				// abortflag set
	  }
		  break ; 
	} 
    if ( pi < 10000 )	/* pi may be used ... */
    {
      if ( pagesofar [ 1 ] < pagesofar [ 0 ] ) 
      if ( ( pagesofar [ 3 ] != 0 ) || ( pagesofar [ 4 ] != 0 ) || ( pagesofar 
      [ 5 ] != 0 ) ) 
      b = 0 ; 
      else b = badness ( pagesofar [ 0 ] - pagesofar [ 1 ] , pagesofar [ 2 ] ) 
      ; 
      else if ( pagesofar [ 1 ] - pagesofar [ 0 ] > pagesofar [ 6 ] ) 
      b = 1073741823L ;  /* 2^30 - 1 */
      else b = badness ( pagesofar [ 1 ] - pagesofar [ 0 ] , pagesofar [ 6 ] ) 
      ; 
      if ( b < 1073741823L )  /* 2^30 - 1 */
      if ( pi <= -10000 ) 
      c = pi ; 
      else if ( b < 10000 ) 
      c = b + pi + insertpenalties ; 
      else c = 100000L ; 
      else c = b ; 
      if ( insertpenalties >= 10000 ) 
      c = 1073741823L ;  /* 2^30 - 1 */
	;
#ifdef STAT
      if ( eqtb [ (hash_size + 3196) ] .cint > 0 ) 
      {
	begindiagnostic () ; 
	printnl ( 37 ) ;	/* % */
	print ( 921 ) ;		/*  t= */
	printtotals () ; 
	print ( 990 ) ;		/*  g= */
	printscaled ( pagesofar [ 0 ] ) ; 
	print ( 924 ) ;		/*  b= */
	if ( b == 1073741823L )  /* 2^30 - 1 */
	printchar ( 42 ) ;	/* * */
	else printint ( b ) ; 
	print ( 925 ) ;		/*  p= */
	printint ( pi ) ; 
	print ( 991 ) ;		/*  c= */
	if ( c == 1073741823L )  /* 2^30 - 1 */
	printchar ( 42 ) ;	/* * */
	else printint ( c ) ; 
	if ( c <= leastpagecost ) 
	printchar ( 35 ) ;	/* # */
	enddiagnostic ( false ) ; 
      } 
#endif /* STAT */
      if ( c <= leastpagecost ) 
      {
	bestpagebreak = p ; 
	bestsize = pagesofar [ 0 ] ; 
	leastpagecost = c ; 
	r = mem [ memtop ] .hh .v.RH ; 
	while ( r != memtop ) {
	    
	  mem [ r + 2 ] .hh .v.LH = mem [ r + 2 ] .hh .v.RH ; 
	  r = mem [ r ] .hh .v.RH ; 
	} 
      } 
      if ( ( c == 1073741823L ) || ( pi <= -10000 ) )  /* 2^30 - 1 */
      {
	fireup ( p ) ; 
	ABORTCHECK;
	if ( outputactive ) return ; 
	goto lab30 ; 
      } 
    } 
    if ( ( mem [ p ] .hh.b0 < 10 ) || ( mem [ p ] .hh.b0 > 11 ) ) 
    goto lab80 ; 
    lab90: if ( mem [ p ] .hh.b0 == 11 ) 
    q = p ; 
    else {
	
      q = mem [ p + 1 ] .hh .v.LH ; 
      pagesofar [ 2 + mem [ q ] .hh.b0 ] = pagesofar [ 2 + mem [ q ] .hh.b0 ] 
      + mem [ q + 2 ] .cint ; 
      pagesofar [ 6 ] = pagesofar [ 6 ] + mem [ q + 3 ] .cint ; 
      if ( ( mem [ q ] .hh.b1 != 0 ) && ( mem [ q + 3 ] .cint != 0 ) ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* ! */
	  print ( 988 ) ;		/* Infinite glue shrinkage found on current page */
	} 
	{
	  helpptr = 4 ; 
	  helpline [ 3 ] = 989 ;	/* The page about to be output contains some infinitely */
	  helpline [ 2 ] = 957 ;	/* shrinkable glue, e.g., `\vss' or `\vskip 0pt minus 1fil'. */
	  helpline [ 1 ] = 958 ;	/* Such glue doesn't belong there; but you can safely proceed, */
	  helpline [ 0 ] = 916 ;	/* since the offensive shrinkability has been made finite. */
	} 
	error () ; 
	ABORTCHECK;
	r = newspec ( q ) ; 
	mem [ r ] .hh.b1 = 0 ; 
	deleteglueref ( q ) ; 
	mem [ p + 1 ] .hh .v.LH = r ; 
	q = r ; 
      } 
    } 
    pagesofar [ 1 ] = pagesofar [ 1 ] + pagesofar [ 7 ] + mem [ q + 1 ] .cint 
    ; 
    pagesofar [ 7 ] = 0 ; 
    lab80: if ( pagesofar [ 7 ] > pagemaxdepth ) 
    {
      pagesofar [ 1 ] = pagesofar [ 1 ] + pagesofar [ 7 ] - pagemaxdepth ; 
      pagesofar [ 7 ] = pagemaxdepth ; 
    } 
    mem [ pagetail ] .hh .v.RH = p ; 
    pagetail = p ; 
    mem [ memtop - 1 ] .hh .v.RH = mem [ p ] .hh .v.RH ; 
    mem [ p ] .hh .v.RH = 0 ; 
    goto lab30 ; 
    lab31: mem [ memtop - 1 ] .hh .v.RH = mem [ p ] .hh .v.RH ; 
    mem [ p ] .hh .v.RH = 0 ; 
    flushnodelist ( p ) ; 
    lab30: ; 
  } while ( ! ( mem [ memtop - 1 ] .hh .v.RH == 0 ) ) ; 
  if ( nestptr == 0 ) 
  curlist .tailfield = memtop - 1 ; 
  else nest [ 0 ] .tailfield = memtop - 1 ; 
} 

void appspace ( ) 
{appspace_regmem 
  halfword q  ; 
  if ( ( curlist .auxfield .hh .v.LH >= 2000 ) &&
	   ( eqtb [ (hash_size + 795) ] .hh .v.RH != 0 ) ) 
  q = newparamglue ( 13 ) ; 
  else {
      
    if ( eqtb [ (hash_size + 794) ] .hh .v.RH != 0 ) 
    mainp = eqtb [ (hash_size + 794) ] .hh .v.RH ; 
    else {
	
      mainp = fontglue [ eqtb [ (hash_size + 1834) ] .hh .v.RH ] ; 
      if ( mainp == 0 ) 
      {
	mainp = newspec ( 0 ) ; 
	maink = parambase [ eqtb [ (hash_size + 1834) ] .hh .v.RH ] + 2 ; 
	mem [ mainp + 1 ] .cint = fontinfo [ maink ] .cint ; 
	mem [ mainp + 2 ] .cint = fontinfo [ maink + 1 ] .cint ; 
	mem [ mainp + 3 ] .cint = fontinfo [ maink + 2 ] .cint ; 
	fontglue [ eqtb [ (hash_size + 1834) ] .hh .v.RH ] = mainp ; 
      } 
    } 
    mainp = newspec ( mainp ) ; 
    if ( curlist .auxfield .hh .v.LH >= 2000 ) 
    mem [ mainp + 1 ] .cint = mem [ mainp + 1 ] .cint + fontinfo [ 7 + 
    parambase [ eqtb [ (hash_size + 1834) ] .hh .v.RH ] ] .cint ; 
    mem [ mainp + 2 ] .cint = xnoverd ( mem [ mainp + 2 ] .cint , curlist 
    .auxfield .hh .v.LH , 1000 ) ; 
    mem [ mainp + 3 ] .cint = xnoverd ( mem [ mainp + 3 ] .cint , 1000 , 
    curlist .auxfield .hh .v.LH ) ; 
    q = newglue ( mainp ) ; 
    mem [ mainp ] .hh .v.RH = 0 ; 
  } 
  mem [ curlist .tailfield ] .hh .v.RH = q ; 
  curlist .tailfield = q ; 
} 

/* called from tex8.c only */

void insertdollarsign ( ) 
{insertdollarsign_regmem 
  backinput () ; 
  curtok = 804 ; 
  {
    if ( interaction == 3 ) 
    ; 
    printnl ( 262 ) ;	/* ! */
    print ( 1011 ) ;	/* Proceed; I'll discard its present contents. */
  } 
  {
    helpptr = 2 ; 
    helpline [ 1 ] = 1012 ;		/* I've inserted a begin-math/end-math symbol since I think */
    helpline [ 0 ] = 1013 ;		/* you left one out. Proceed, with fingers crossed. */
  } 
  inserror () ; 
//  ABORTCHECK;
} 

void youcant ( ) 
{youcant_regmem 
  {
    if ( interaction == 3 ) 
    ; 
    printnl ( 262 ) ;	/* ! */
    print ( 682 ) ;		/* You can't use ` */
  } 
  printcmdchr ( curcmd , curchr ) ; 
  print ( 1014 ) ;		/* ' in  */
  printmode ( curlist .modefield ) ; 
} 

void reportillegalcase ( ) 
{reportillegalcase_regmem 
  youcant () ; 
  {
    helpptr = 4 ; 
    helpline [ 3 ] = 1015 ;		/* Sorry, but I'm not programmed to handle this case; */
    helpline [ 2 ] = 1016 ;		/* I'll just pretend that you didn't ask for it. */
    helpline [ 1 ] = 1017 ;		/* If you're in the wrong mode, you might be able to */
    helpline [ 0 ] = 1018 ;		/* return to the right one by typing `I}' or `I$' or `I\par'. */
  } 
  error () ; 
//  ABORTCHECK;
} 

booleane privileged ( ) 
{register booleane Result; privileged_regmem 
  if ( curlist .modefield > 0 ) Result = true ; 
  else {
    reportillegalcase () ; 
    Result = false ; 
  } 
  return Result ; 
} 

booleane itsallover ( ) 
{/* 10 */ register booleane Result; itsallover_regmem 
  if ( privileged () ) 
  {
    if ( ( memtop - 2 == pagetail ) && ( curlist .headfield == curlist 
    .tailfield ) && ( deadcycles == 0 ) ) 
    {
      Result = true ; 
      return(Result) ; 
    } 
    backinput () ; 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newnullbox () ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    mem [ curlist .tailfield + 1 ] .cint = eqtb [ (hash_size + 3733) ] .cint ; 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newglue ( 8 ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( -1073741824L ) ; 
	  /* - 2^30  */
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    buildpage () ; 
  } 
  Result = false ; 
  return Result ; 
} 

void appendglue ( ) {appendglue_regmem 
   smallnumber s  ; 
	s = curchr ; 
	switch ( s ) 
	{case 0 : 
			curval = 4 ; 
	break ; 
	case 1 : 
		curval = 8 ; 
		break ; 
	case 2 : 
		curval = 12 ; 
		break ; 
	case 3 : 
		curval = 16 ; 
		break ; 
	case 4 : 
		scanglue ( 2 ) ; 
		ABORTCHECK;
				break ; 
	case 5 : 
		scanglue ( 3 ) ; 
		ABORTCHECK;
				break ; 
	} 
	{
		mem [ curlist .tailfield ] .hh .v.RH = newglue ( curval ) ; 
		curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
	} 
	if ( s >= 4 ) 
	{
		decr ( mem [ curval ] .hh .v.RH ) ; 
		if ( s > 4 ) 
			mem [ curlist .tailfield ] .hh.b1 = 99 ; 
	} 
} 

void appendkern ( ) {appendkern_regmem 
	quarterword s  ; 
	s = curchr ; 
	scandimen ( s == 99 , false , false ) ; 
	ABORTCHECK;
	{
		mem [ curlist .tailfield ] .hh .v.RH = newkern ( curval ) ; 
		curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
	} 
	mem [ curlist .tailfield ] .hh.b1 = s ; 
} 

void offsave ( ) 
{offsave_regmem 
  halfword p  ; 
  if ( curgroup == 0 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* ! */
      print ( 773 ) ;		/* Extra  */
    } 
    printcmdchr ( curcmd , curchr ) ; 
    {
      helpptr = 1 ; 
      helpline [ 0 ] = 1037 ;	/* Things are pretty mixed up, but I think the worst is over. */
    } 
    error () ; 
	ABORTCHECK;
  } 
  else {
      
    backinput () ; 
    p = getavail () ; 
    mem [ memtop - 3 ] .hh .v.RH = p ; 
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* ! */
      print ( 622 ) ;		/* Missing  */
    } 
    switch ( curgroup ) 
    {case 14 : 
      {
/*	mem [ p ] .hh .v.LH = (hash_size + 4611) ;  */
/*	mem [ p ] .hh .v.LH = (hash_size + 4095 + 516) ;  */
	mem [ p ] .hh .v.LH = (hash_size + hash_extra + 4095 + 516) ; /* 96/Jan/10 */
	printesc ( 513 ) ;	/* endgroup */
      } 
      break ; 
    case 15 : 
      {
	mem [ p ] .hh .v.LH = 804 ; 
	printchar ( 36 ) ; 
      } 
      break ; 
    case 16 : 
      {
/*	mem [ p ] .hh .v.LH = (hash_size + 4612) ;  */
/*	mem [ p ] .hh .v.LH = (hash_size + 4095 + 517) ;  */
	mem [ p ] .hh .v.LH = (hash_size + hash_extra + 4095 + 517) ; /* 96/Jan/10 */
	mem [ p ] .hh .v.RH = getavail () ; 
	p = mem [ p ] .hh .v.RH ; 
	mem [ p ] .hh .v.LH = 3118 ; 
	printesc ( 1036 ) ;		/* right. */
      } 
      break ; 
      default: 
      {
	mem [ p ] .hh .v.LH = 637 ; 
	printchar ( 125 ) ;		/* } */
      } 
      break ; 
    } 
    print ( 623 ) ;		/*  inserted */
    begintokenlist ( mem [ memtop - 3 ] .hh .v.RH , 4 ) ; 
    {
      helpptr = 5 ; 
      helpline [ 4 ] = 1031 ;	/* I've inserted something that you may have forgotten. */
      helpline [ 3 ] = 1032 ;	/* (See the <inserted text> above.) */
      helpline [ 2 ] = 1033 ;	/* With luck, this will get me unwedged. But if you */
      helpline [ 1 ] = 1034 ;	/* really didn't forget anything, try typing `2' now; then */
      helpline [ 0 ] = 1035 ;	/* my insertion and my current dilemma will both disappear. */
    } 
    error () ; 
//  ABORTCHECK;
  } 
} 

/* only called from tex8.c */

void extrarightbrace ( ) 
{extrarightbrace_regmem 
  {
    if ( interaction == 3 ) 
    ; 
    printnl ( 262 ) ; 	/* ! */
    print ( 1042 ) ;	/* Extra }, or forgotten */
  } 
  switch ( curgroup ) 
  {case 14 : 
    printesc ( 513 ) ;	/* endgroup */
    break ; 
  case 15 : 
    printchar ( 36 ) ; /* $ */
    break ; 
  case 16 : 
    printesc ( 871 ) ;	/* right */
    break ; 
  } 
  {
    helpptr = 5 ; 
    helpline [ 4 ] = 1043 ;		/* I've deleted a group-closing symbol because it seems to be */
    helpline [ 3 ] = 1044 ;		/* spurious, as in `$x}$'. But perhaps the } is legitimate and */
    helpline [ 2 ] = 1045 ;		/* you forgot something else, as in `\hbox{$x}'. In such cases */
    helpline [ 1 ] = 1046 ;		/* the way to recover is to insert both the forgotten and the */
    helpline [ 0 ] = 1047 ;		/* deleted material, e.g., by typing `I$}'. */
  } 
  error () ; 
//  ABORTCHECK;
  incr ( alignstate ) ; 
} 

void normalparagraph ( ) 
{normalparagraph_regmem 
/* if looseness<>0 then eq_word_define(int_base+looseness_code,0); */
  if ( eqtb [ (hash_size + 3182) ] .cint != 0 ) 
  eqworddefine ( (hash_size + 3182) , 0 ) ; 
  if ( eqtb [ (hash_size + 3747) ] .cint != 0 ) 
  eqworddefine ( (hash_size + 3747) , 0 ) ; 
  if ( eqtb [ (hash_size + 3204) ] .cint != 1 ) 
  eqworddefine ( (hash_size + 3204) , 1 ) ; 
  if ( eqtb [ (hash_size + 1312) ] .hh .v.RH != 0 ) 
  eqdefine ( (hash_size + 1312) , 118 , 0 ) ; 
} 

void zboxend ( boxcontext ) 
integer boxcontext ; 
{boxend_regmem 
  halfword p  ; 
/* if box_context<box_flag then ... 1073741824 2^30 */
  if ( boxcontext < 1073741824L ) 
  {
    if ( curbox != 0 ) 
    {
      mem [ curbox + 4 ] .cint = boxcontext ; 
      if ( abs ( curlist .modefield ) == 1 ) 
      {
	appendtovlist ( curbox ) ; 
	if ( adjusttail != 0 ) 
	{
	  if ( memtop - 5 != adjusttail ) 
	  {
	    mem [ curlist .tailfield ] .hh .v.RH = mem [ memtop - 5 ] .hh 
	    .v.RH ; 
	    curlist .tailfield = adjusttail ; 
	  } 
	  adjusttail = 0 ; 
	} 
	if ( curlist .modefield > 0 ) {
		buildpage () ;
		ABORTCHECK;
	}
      } 
      else {
	  
	if ( abs ( curlist .modefield ) == 102 ) 
	curlist .auxfield .hh .v.LH = 1000 ; 
	else {
	    
	  p = newnoad () ; 
	  mem [ p + 1 ] .hh .v.RH = 2 ; 
	  mem [ p + 1 ] .hh .v.LH = curbox ; 
	  curbox = p ; 
	} 
	mem [ curlist .tailfield ] .hh .v.RH = curbox ; 
	curlist .tailfield = curbox ; 
      } 
    } 
  } 

/* following fixed 1994/Apr/5 1 day anno Yang --- moby sigh ... */

/* else if box_context<box_flag+512 then ... */
/*  else if ( boxcontext < 1073742336L ) */		/* 2^30 + 512 */ 
  else if ( boxcontext < (1073741824L + 512) )		/* 2^30 + 512 */ 
/* else if box_context<box_flag+256 then ... */
/*  if ( boxcontext < 1073742080L )	*/ /* 2^30 + 256 */ 
  if ( boxcontext < (1073741824L + 256) )	/* 2^30 + 256 */ 
/* eq_define(box_base-box_flag+box_context,box_ref,cur_box) */
/* eqdefine ( (hash_size - 1073740246L) + boxcontext , 119 , curbox ) ; */
  eqdefine ( (hash_size + 1578 - 1073741824L) + boxcontext , 119 , curbox ) ; 
/* else geq_define(box_base-box_flag-256+box_context,box_ref,cur_box) */
/* else geqdefine ( (hash_size - 1073740502L) + boxcontext , 119 , curbox ) ; */
  else geqdefine ( (hash_size + 1322 - 1073741824L) + boxcontext , 119 , curbox ) ; 
  else if ( curbox != 0 ) 
/*  if ( boxcontext > 1073742336L ) */	/* 2^30 + 512 */ 
  if ( boxcontext > (1073741824L + 512)  )	/* 2^30 + 512 */ 
  {
    do {
		getxtoken () ; 
		ABORTCHECK;
    } while ( ! ( ( curcmd != 10 ) && ( curcmd != 0 ) ) ) ; 
    if ( ( ( curcmd == 26 ) && ( abs ( curlist .modefield ) != 1 ) ) || ( ( 
    curcmd == 27 ) && ( abs ( curlist .modefield ) == 1 ) ) || ( ( curcmd == 
    28 ) && ( abs ( curlist .modefield ) == 203 ) ) ) 
    {
/*   begin append_glue; subtype(tail):=box_context-(leader_flag-a_leaders); */
      appendglue () ; 
	  ABORTCHECK;
/*      - ( 2^30 + 513 - 100)  */
      mem [ curlist .tailfield ] .hh.b1 = boxcontext - ( 1073742237L ) ; 
      mem [ curlist .tailfield + 1 ] .hh .v.RH = curbox ; 
    } 
    else {
	
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ; 	/* ! */
	print ( 1060 ) ;	/* Leaders not followed by proper glue */
      } 
      {
	helpptr = 3 ; 
	helpline [ 2 ] = 1061 ;		/* You should say `\leaders <box or rule><hskip or vskip>'. */
	helpline [ 1 ] = 1062 ;		/* I found the <box or rule>, but there's no suitable */
	helpline [ 0 ] = 1063 ;		/* <hskip or vskip>, so I'm ignoring these leaders. */
      } 
      backerror () ; 
	  ABORTCHECK;
      flushnodelist ( curbox ) ; 
	}
  }
  else shipout ( curbox ) ; 
} 

/* called only from tex8.c */

void zbeginbox ( boxcontext ) 
integer boxcontext ; 
{/* 10 30 */ beginbox_regmem 
  halfword p, q  ; 
  quarterword m  ; 
  halfword k  ; 
  eightbits n  ; 
  switch ( curchr ) {
  case 0 : 
    {
      scaneightbitint () ; 
	  ABORTCHECK;
      curbox = eqtb [ (hash_size + 1578) + curval ] .hh .v.RH ; 
      eqtb [ (hash_size + 1578) + curval ] .hh .v.RH = 0 ; 
    } 
    break ; 
  case 1 : 
    {
      scaneightbitint () ; 
	  ABORTCHECK;
      curbox = copynodelist ( eqtb [ (hash_size + 1578) + curval ] .hh .v.RH ) ; 
    } 
    break ; 
  case 2 : 
    {
      curbox = 0 ; 
      if ( abs ( curlist .modefield ) == 203 ) 
      {
	youcant () ; 
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 1064 ;	/* Sorry; this \lastbox will be void. */
	} 
	error () ; 
	ABORTCHECK;
      } 
      else if ( ( curlist .modefield == 1 ) && ( curlist .headfield == curlist 
      .tailfield ) ) 
      {
	youcant () ; 
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1065 ;	/* Sorry...I usually can't take things from the current page. */
	  helpline [ 0 ] = 1066 ;	/* This \lastbox will therefore be void. */
	} 
	error () ; 
	ABORTCHECK;
      } 
      else {
	if ( ! ( curlist .tailfield >= himemmin ) ) 
	if ( ( mem [ curlist .tailfield ] .hh.b0 == 0 ) || ( mem [ curlist 
	.tailfield ] .hh.b0 == 1 ) ) 
	{
	  q = curlist .headfield ; 
	  do {
	      p = q ; 
	    if ( ! ( q >= himemmin ) ) 
	    if ( mem [ q ] .hh.b0 == 7 ) 
	    {
	      {
			  register integer for_end; 
			  m = 1 ; 
			  for_end = mem [ q ] .hh.b1 ; 
			  if ( m <= for_end) do 
				  p = mem [ p ] .hh .v.RH ; 
			  while ( m++ < for_end ) ;
		  } 
	      if ( p == curlist .tailfield ) 
	      goto lab30 ; 
	    } 
	    q = mem [ p ] .hh .v.RH ; 
	  } while ( ! ( q == curlist .tailfield ) ) ; 
	  curbox = curlist .tailfield ; 
	  mem [ curbox + 4 ] .cint = 0 ; 
	  curlist .tailfield = p ; 
	  mem [ p ] .hh .v.RH = 0 ; 
	  lab30: ; 
	} 
      } 
    } 
    break ; 
  case 3 : 
    {
      scaneightbitint () ; 
	  ABORTCHECK;
      n = curval ; 
      if ( ! scankeyword ( 836 ) )	/* to */
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ; 	/* ! */
	  print ( 1067 ) ;		/* Missing `to' inserted */
	} 
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1068 ;	/* I'm working on `\vsplit<box number> to <dimen>'; */
	  helpline [ 0 ] = 1069 ;	/* will look for the <dimen> next. */
	} 
	error () ; 
	ABORTCHECK;
      } 
      scandimen ( false , false , false ) ; 
	  ABORTCHECK;
      curbox = vsplit ( n , curval ) ; 
	  ABORTCHECK;
    } 
    break ; 
    default: 
    {
      k = curchr - 4 ; 
      savestack [ saveptr + 0 ] .cint = boxcontext ; 
      if ( k == 102 ) 
      if ( ( boxcontext < 1073741824L ) && /* 2^30 */
		   ( abs ( curlist .modefield ) == 1 ) 
      ) 
      scanspec ( 3 , true ) ; 
      else scanspec ( 2 , true ) ; 
      else {
	  
	if ( k == 1 ) 
	scanspec ( 4 , true ) ; 
	else {
	    
	  scanspec ( 5 , true ) ; 
	  k = 1 ; 
	} 
	normalparagraph () ; 
      } 
      pushnest () ; 
      curlist .modefield = - (integer) k ; 
      if ( k == 1 ) 
      {
	curlist .auxfield .cint = -65536000L ; 
	if ( eqtb [ (hash_size + 1318) ] .hh .v.RH != 0 ) /* everyhbox */
	begintokenlist ( eqtb [ (hash_size + 1318) ] .hh .v.RH , 11 ) ; 
      } 
      else {
	  
	curlist .auxfield .hh .v.LH = 1000 ; 
	if ( eqtb [ (hash_size + 1317) ] .hh .v.RH != 0 ) /* everyhbox */
		begintokenlist ( eqtb [ (hash_size + 1317) ] .hh .v.RH , 10 ) ; 
      } 
      return ; 
    } 
    break ; 
  } 
  boxend ( boxcontext ) ; 
} 

void zscanbox ( boxcontext ) 
integer boxcontext ; 
{scanbox_regmem 
  do {
      getxtoken () ; 
	  ABORTCHECK;
  } while ( ! ( ( curcmd != 10 ) && ( curcmd != 0 ) ) ) ; 
  if ( curcmd == 20 ) {
	  beginbox ( boxcontext ) ;
	  ABORTCHECK;
  }
  else if ( ( boxcontext >= 1073742337L ) && /*  ( 2^30 + 512 + 1)  */
			( ( curcmd == 36 ) || ( curcmd == 35 ) ) ) 
  {
    curbox = scanrulespec () ; 
	ABORTCHECK;
    boxend ( boxcontext ) ; 
	ABORTCHECK;
  } 
  else {
      
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ; 		/* ! */
      print ( 1070 ) ;			/* A <box> was supposed to be here */
    } 
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 1071 ;	/* I was expecting to see \hbox or \vbox or \copy or \box or */
      helpline [ 1 ] = 1072 ;	/* something like that. So you might find something missing in */
      helpline [ 0 ] = 1073 ;	/* your output. But keep trying; you can fix this later. */
    } 
    backerror () ; 
//  ABORTCHECK;
  }
} 

/****************************************************************************/

void zpackage (smallnumber);

/****************************************************************************/

smallnumber znormmin ( h ) 
integer h ; 
{register
/*  smallnumber Result; */
  int Result;								/* 95/Jan/7 */
  normmin_regmem 
  if ( h <= 0 ) 
	  Result = 1 ; 
  else if ( h >= 63 ) 
	  Result = 63 ; 
  else Result = h ; 
  return Result ; 
} 

void znewgraf ( indented ) 
booleane indented ; 
{newgraf_regmem 
  curlist .pgfield = 0 ; 
  if ( ( curlist .modefield == 1 ) || ( curlist .headfield != curlist 
  .tailfield ) ) 
  {
    mem [ curlist .tailfield ] .hh .v.RH = newparamglue ( 2 ) ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
/* used to be followingin 3.141 */
/*  curlist .lhmfield = normmin ( eqtb [ (hash_size + 3214) ] .cint ) ; */
/*  curlist .rhmfield = normmin ( eqtb [ (hash_size + 3215) ] .cint ) ; */
  pushnest () ; 
  curlist .modefield = 102 ; 
  curlist .auxfield .hh .v.LH = 1000 ; 
/* changes here since 3.141 */
  if ( eqtb [ (hash_size + 3213) ] .cint <= 0 ) 
  curlang = 0 ; 
  else if ( eqtb [ (hash_size + 3213) ] .cint > 255 ) 
  curlang = 0 ; 
  else curlang = eqtb [ (hash_size + 3213) ] .cint ; 
  curlist .auxfield .hh .v.RH = curlang ; 
  curlist .pgfield = ( normmin ( eqtb [ (hash_size + 3214) ] .cint ) * 64 + 
	   normmin ( eqtb [ (hash_size + 3215) ] .cint ) ) * 65536L + curlang ; 
/* eqtb ??? hash_size ? hash_size + hash_extra ? normmin etc */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( indented ) 
  {
    curlist .tailfield = newnullbox () ; 
    mem [ curlist .headfield ] .hh .v.RH = curlist .tailfield ; 
    mem [ curlist .tailfield + 1 ] .cint = eqtb [ (hash_size + 3730) ] .cint ; 
  } 
  if ( eqtb [ (hash_size + 1314) ] .hh .v.RH != 0 )		/* everypar */
	  begintokenlist ( eqtb [ (hash_size + 1314) ] .hh .v.RH , 7 ) ; 
  if ( nestptr == 1 ) {
	  buildpage () ;
	  ABORTCHECK;
  }
} 

/* procedure indent_in_hmode; l.21058 */
void indentinhmode ( ) 
{indentinhmode_regmem 
  halfword p, q  ; 
  if ( curchr > 0 ) 
  {
    p = newnullbox () ; 
    mem [ p + 1 ] .cint = eqtb [ (hash_size + 3730) ] .cint ; 
    if ( abs ( curlist .modefield ) == 102 ) 
    curlist .auxfield .hh .v.LH = 1000 ; 
    else {
	
      q = newnoad () ; 
      mem [ q + 1 ] .hh .v.RH = 2 ; 
      mem [ q + 1 ] .hh .v.LH = p ; 
      p = q ; 
    } 
    {
      mem [ curlist .tailfield ] .hh .v.RH = p ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
  } 
} 

/* only called from tex8.c */

void headforvmode ( ) 
{headforvmode_regmem 
  if ( curlist .modefield < 0 ) 
  if ( curcmd != 36 ) {
	  offsave () ;
	  ABORTCHECK;
  }
  else {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ; 	/* ! */
      print ( 682 ) ;	/* You can't use ` */
    } 
    printesc ( 518 ) ;	/* hrule */
    print ( 1076 ) ;	/* ' here except with leaders */
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 1077 ;	/* To put a horizontal rule in an hbox or an alignment, */
      helpline [ 0 ] = 1078 ;	/* you should use \leaders or \hrulefill (see The TeXbook). */
    } 
    error () ; 
	ABORTCHECK;
  } 
  else {
      
    backinput () ; 
    curtok = partoken ; 
    backinput () ; 
    curinput .indexfield = 4 ; 
  } 
} 

void endgraf ( ) 
{endgraf_regmem 
  if ( curlist .modefield == 102 ) 
  {
    if ( curlist .headfield == curlist .tailfield ) 
    popnest () ; 
    else linebreak ( eqtb [ (hash_size + 3169) ] .cint ) ; 
    normalparagraph () ; 
    errorcount = 0 ; 
  } 
} 

/* only called form tex8.c */

void begininsertoradjust ( ) 
{begininsertoradjust_regmem 
  if ( curcmd == 38 ) 
  curval = 255 ; 
  else {
      
    scaneightbitint () ; 
	ABORTCHECK;
    if ( curval == 255 ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* ! */
	print ( 1079 ) ;	/* You can't  */
      } 
      printesc ( 327 ) ; /* insert */
      printint ( 255 ) ; 
      {
	helpptr = 1 ; 
	helpline [ 0 ] = 1080 ;		/* I'm changing to \insert0; box 255 is special. */
      } 
      error () ; 
	  ABORTCHECK;
      curval = 0 ; 
    } 
  } 
  savestack [ saveptr + 0 ] .cint = curval ; 
  incr ( saveptr ) ; 
  newsavelevel ( 11 ) ; 
  scanleftbrace () ; 
  ABORTCHECK;
  normalparagraph () ; 
  pushnest () ; 
  curlist .modefield = -1 ; 
  curlist .auxfield .cint = -65536000L ; 
} 

void makemark ( ) 
{makemark_regmem 
  halfword p  ; 
  p = scantoks ( false , true ) ; 
  ABORTCHECK;
  p = getnode ( 2 ) ; 
  mem [ p ] .hh.b0 = 4 ; 
  mem [ p ] .hh.b1 = 0 ; 
  mem [ p + 1 ] .cint = defref ; 
  mem [ curlist .tailfield ] .hh .v.RH = p ; 
  curlist .tailfield = p ; 
} 

void appendpenalty ( ) 
{appendpenalty_regmem 
    scanint () ; 
ABORTCHECK;
  {
    mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( curval ) ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  if ( curlist .modefield == 1 ) {
	  buildpage () ;
  }
} 

/* only called from tex8.c */

void deletelast ( ) 
{/* 10 */ deletelast_regmem 
  halfword p, q  ; 
  quarterword m  ; 
  if ( ( curlist .modefield == 1 ) && ( curlist .tailfield == curlist 
  .headfield ) ) 
  {
/*    if ( ( curchr != 10 ) || ( lastglue != 262143L ) ) */
    if ( ( curchr != 10 ) || ( lastglue != emptyflag ) ) 
    {
      youcant () ; 
      {
	helpptr = 2 ; 
	helpline [ 1 ] = 1065 ;		/* Sorry...I usually can't take things from the current page. */
	helpline [ 0 ] = 1081 ;		/* Try `I\vskip-\lastskip' instead. */
      } 
      if ( curchr == 11 ) 
		  helpline [ 0 ] = ( 1082 ) ;	/* Try `I\kern-\lastkern' instead. */
      else if ( curchr != 10 ) 
		  helpline [ 0 ] = ( 1083 ) ;	/* Perhaps you can make the output routine do it. */
      error () ; 
	  ABORTCHECK;
    } 
  } 
  else {
      
    if ( ! ( curlist .tailfield >= himemmin ) ) 
    if ( mem [ curlist .tailfield ] .hh.b0 == curchr ) 
    {
      q = curlist .headfield ; 
      do {
	  p = q ; 
	if ( ! ( q >= himemmin ) ) 
	if ( mem [ q ] .hh.b0 == 7 ) 
	{
	  {
		  register integer for_end;
		  m = 1 ;
		  for_end = mem [ q ] .hh.b1 ;
		  if ( m <= for_end) do 
			  p = mem [ p ] .hh .v.RH ; 
		  while ( m++ < for_end ) ;
	  } 
	  if ( p == curlist .tailfield ) 
	  return ; 
	} 
	q = mem [ p ] .hh .v.RH ; 
      } while ( ! ( q == curlist .tailfield ) ) ; 
      mem [ p ] .hh .v.RH = 0 ; 
      flushnodelist ( curlist .tailfield ) ; 
      curlist .tailfield = p ; 
    } 
  } 
} 

/* only called from tex8.c */

/* procedure unpackage; l.21256 */
void unpackage ( ) 
{/* 10 */ unpackage_regmem 
  halfword p  ; 
  char c  ; 
  c = curchr ; 
  scaneightbitint () ; 
  ABORTCHECK;
  p = eqtb [ (hash_size + 1578) + curval ] .hh .v.RH ; 
  if ( p == 0 ) /* if p=null then return; l.21261 */
  return ; 
  if ( ( abs ( curlist .modefield ) == 203 ) || ( ( abs ( curlist .modefield ) 
  == 1 ) && ( mem [ p ] .hh.b0 != 1 ) ) || ( ( abs ( curlist .modefield ) == 
  102 ) && ( mem [ p ] .hh.b0 != 0 ) ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ; 	/* ! */
      print ( 1091 ) ;		/* Incompatible list can't be unboxed */
    } 
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 1092 ;	/* Sorry, Pandora. (You sneaky devil.) */
      helpline [ 1 ] = 1093 ;	/* I refuse to unbox an \hbox in vertical mode or vice versa. */
      helpline [ 0 ] = 1094 ;	/* And I can't open any boxes in math mode. */
    } 
    error () ; 
//  ABORTCHECK;
    return ; 
  } 
  if ( c == 1 ) 
  mem [ curlist .tailfield ] .hh .v.RH = copynodelist ( mem [ p + 5 ] .hh 
  .v.RH ) ; 
  else {
      
    mem [ curlist .tailfield ] .hh .v.RH = mem [ p + 5 ] .hh .v.RH ; 
    eqtb [ (hash_size + 1578) + curval ] .hh .v.RH = 0 ; 
    freenode ( p , 7 ) ; 
  } 
  while ( mem [ curlist .tailfield ] .hh .v.RH != 0 ) curlist .tailfield = mem 
  [ curlist .tailfield ] .hh .v.RH ; 
} 
void appenditaliccorrection ( ) 
{/* 10 */ appenditaliccorrection_regmem 
  halfword p  ; 
  internalfontnumber f  ; 
  if ( curlist .tailfield != curlist .headfield ) 
  {
    if ( ( curlist .tailfield >= himemmin ) ) 
    p = curlist .tailfield ; 
    else if ( mem [ curlist .tailfield ] .hh.b0 == 6 ) 
    p = curlist .tailfield + 1 ; 
    else return ; 
    f = mem [ p ] .hh.b0 ; 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newkern ( fontinfo [ italicbase [ 
      f ] + ( fontinfo [ charbase [ f ] + mem [ p ] .hh.b1 ] .qqqq .b2 ) / 4 ] 
      .cint ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    mem [ curlist .tailfield ] .hh.b1 = 1 ; 
  } 
} 

void appenddiscretionary ( ) 
{appenddiscretionary_regmem 
  integer c  ; 
  {
    mem [ curlist .tailfield ] .hh .v.RH = newdisc () ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  if ( curchr == 1 ) 
  {
    c = hyphenchar [ eqtb [ (hash_size + 1834) ] .hh .v.RH ] ; 
    if ( c >= 0 ) 
    if ( c < 256 ) 
    mem [ curlist .tailfield + 1 ] .hh .v.LH = newcharacter ( eqtb [ (hash_size + 1834) ] 
    .hh .v.RH , c ) ; 
  } 
  else {
      
    incr ( saveptr ) ; 
    savestack [ saveptr - 1 ] .cint = 0 ; 
    newsavelevel ( 10 ) ; 
    scanleftbrace () ; 
	ABORTCHECK;
    pushnest () ; 
    curlist .modefield = -102 ; 
    curlist .auxfield .hh .v.LH = 1000 ; 
  } 
} 

/* only called form tex8.c */

void builddiscretionary ( ) 
{/* 30 10 */ builddiscretionary_regmem 
  halfword p, q  ; 
  integer n  ; 
  unsave () ; 
  q = curlist .headfield ; 
  p = mem [ q ] .hh .v.RH ; 
  n = 0 ; 
  while ( p != 0 ) {
      
    if ( ! ( p >= himemmin ) ) 
    if ( mem [ p ] .hh.b0 > 2 ) 
    if ( mem [ p ] .hh.b0 != 11 ) 
    if ( mem [ p ] .hh.b0 != 6 ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ; 		/* ! */
	print ( 1101 ) ;		/* Improper discretionary list */
      } 
      {
	helpptr = 1 ; 
	helpline [ 0 ] = 1102 ;		/* Discretionary lists must contain only boxes and kerns. */
      } 
      error () ; 
	  ABORTCHECK;
      begindiagnostic () ; 
      printnl ( 1103 ) ;	/* The following discretionary sublist has been deleted: */
      showbox ( p ) ; 
      enddiagnostic ( true ) ; 
      flushnodelist ( p ) ; 
      mem [ q ] .hh .v.RH = 0 ; 
      goto lab30 ; 
    } 
    q = p ; 
    p = mem [ q ] .hh .v.RH ; 
    incr ( n ) ; 
  } 
  lab30: ; 
  p = mem [ curlist .headfield ] .hh .v.RH ; 
  popnest () ; 
  switch ( savestack [ saveptr - 1 ] .cint ) 
  {case 0 : 
    mem [ curlist .tailfield + 1 ] .hh .v.LH = p ; 
    break ; 
  case 1 : 
    mem [ curlist .tailfield + 1 ] .hh .v.RH = p ; 
    break ; 
  case 2 : 
    {
      if ( ( n > 0 ) && ( abs ( curlist .modefield ) == 203 ) ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ; 	/* ! */
	  print ( 1095 ) ;		/* Illegal math  */
	} 
	printesc ( 346 ) ;  /* discretionary */
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1096 ;	/* Sorry: The third part of a discretionary break must be */
	  helpline [ 0 ] = 1097 ;	/* empty, in math formulas. I had to delete your third part. */
	} 
	flushnodelist ( p ) ; 
	n = 0 ; 
	error () ; 
	ABORTCHECK;
      } 
      else mem [ curlist .tailfield ] .hh .v.RH = p ; 
/* if n <= max_quarterword then replace_count(tail) <- n; p.1120 */
/*      if ( n <= 255 )  */				/* 94/Apr/4 ? */
      if ( n <= maxquarterword)			/* 96/Oct/12 ??? */
      mem [ curlist .tailfield ] .hh.b1 = n ; 
      else {
	  
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ; 	/* ! */
	  print ( 1098 ) ;		/* Discretionary list is too long */
	} 
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1099 ;	/* Wow---I never thought anybody would tweak me here. */
	  helpline [ 0 ] = 1100 ;	/* You can't seriously need such a huge discretionary list? */
	} 
	error () ; 
	ABORTCHECK;
      } 
      if ( n > 0 ) 
      curlist .tailfield = q ; 
      decr ( saveptr ) ; 
      return ; 
    } 
    break ; 
  } 
  incr ( savestack [ saveptr - 1 ] .cint ) ; 
  newsavelevel ( 10 ) ; 
  scanleftbrace () ; 
  ABORTCHECK;	
  pushnest () ; 
  curlist .modefield = -102 ; 
  curlist .auxfield .hh .v.LH = 1000 ; 
} 

/* called only from tex8.c */

void makeaccent ( ) 
{makeaccent_regmem 
  real s, t  ; 
  halfword p, q, r  ; 
  internalfontnumber f  ; 
  scaled a, h, x, w, delta  ; 
  ffourquarters i  ; 
  scancharnum () ; 
  ABORTCHECK;
  f = eqtb [ (hash_size + 1834) ] .hh .v.RH ; 
  p = newcharacter ( f , curval ) ; 
  if ( p != 0 ) 
  {
    x = fontinfo [ 5 + parambase [ f ] ] .cint ; 
    s = fontinfo [ 1 + parambase [ f ] ] .cint / ((double) 65536.0 ) ; 
    a = fontinfo [ widthbase [ f ] + fontinfo [ charbase [ f ] + mem [ p ] 
    .hh.b1 ] .qqqq .b0 ] .cint ; 
    doassignments () ; 
	ABORTCHECK;
    q = 0 ; 
    f = eqtb [ (hash_size + 1834) ] .hh .v.RH ; 
    if ( ( curcmd == 11 ) || ( curcmd == 12 ) || ( curcmd == 68 ) ) 
    q = newcharacter ( f , curchr ) ; 
    else if ( curcmd == 16 ) 
    {
      scancharnum () ; 
	  ABORTCHECK;
      q = newcharacter ( f , curval ) ; 
    } 
    else backinput () ; 
    if ( q != 0 ) 
    {
      t = fontinfo [ 1 + parambase [ f ] ] .cint / ((double) 65536.0 ) ; 
      i = fontinfo [ charbase [ f ] + mem [ q ] .hh.b1 ] .qqqq ; 
      w = fontinfo [ widthbase [ f ] + i .b0 ] .cint ; 
      h = fontinfo [ heightbase [ f ] + ( i .b1 ) / 16 ] .cint ; 
      if ( h != x ) 
      {
	p = hpack ( p , 0 , 1 ) ; 
	mem [ p + 4 ] .cint = x - h ; 
      } 
      delta = round ( ( w - a ) / ((double) 2.0 ) + h * t - x * s ) ; 
      r = newkern ( delta ) ; 
      mem [ r ] .hh.b1 = 2 ; 
      mem [ curlist .tailfield ] .hh .v.RH = r ; 
      mem [ r ] .hh .v.RH = p ; 
      curlist .tailfield = newkern ( - (integer) a - delta ) ; 
      mem [ curlist .tailfield ] .hh.b1 = 2 ; 
      mem [ p ] .hh .v.RH = curlist .tailfield ; 
      p = q ; 
    } 
    mem [ curlist .tailfield ] .hh .v.RH = p ; 
    curlist .tailfield = p ; 
    curlist .auxfield .hh .v.LH = 1000 ; 
  } 
} 

void alignerror ( ) 
{alignerror_regmem 
  if ( abs ( alignstate ) > 2 ) {
    {
      if ( interaction == 3 ) 
		  ; 
      printnl ( 262 ) ; 		/* ! */
      print ( 1108 ) ;			/* Misplaced  */
    } 
    printcmdchr ( curcmd , curchr ) ; 
    if ( curtok == 1062 )  {
      {
	helpptr = 6 ; 
	helpline [ 5 ] = 1109 ;		/* I can't figure out why you would want to use a tab mark */
	helpline [ 4 ] = 1110 ;		/* here. If you just want an ampersand, the remedy is */
	helpline [ 3 ] = 1111 ;		/* simple: Just type `I\&' now. But if some right brace */
	helpline [ 2 ] = 1112 ;		/* up above has ended a previous alignment prematurely, */
	helpline [ 1 ] = 1113 ;		/* you're probably due for more error messages, and you */
	helpline [ 0 ] = 1114 ;		/* might try typing `S' now just to see what is salvageable. */
      } 
    } 
    else {
      {
	helpptr = 5 ; 
	helpline [ 4 ] = 1109 ;		/* I can't figure out why you would want to use a tab mark */
	helpline [ 3 ] = 1115 ;		/* or \cr or \span just now. If something like a right brace */
	helpline [ 2 ] = 1112 ;		/* up above has ended a previous alignment prematurely, */
	helpline [ 1 ] = 1113 ;		/* you're probably due for more error messages, and you */
	helpline [ 0 ] = 1114 ;		/* might try typing `S' now just to see what is salvageable. */
      } 
    } 
    error () ; 
	ABORTCHECK;
  } 
  else {
    backinput () ; 
    if ( alignstate < 0 )  {
      {
	if ( interaction == 3 ) 
		; 
	printnl ( 262 ) ; 		/* ! */
	print ( 654 ) ;			/* Missing { inserted */
      } 
      incr ( alignstate ) ; 
      curtok = 379 ;		/* belowdisplayshortskip ? */
    } 
    else {
      {
	if ( interaction == 3 ) 
		; 
	printnl ( 262 ) ; 		/* ! */
	print ( 1104 ) ;		/* Missing } inserted */
      } 
      decr ( alignstate ) ; 
      curtok = 637 ; 
    } 
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 1105 ;	/* I've put in what seems to be necessary to fix */
      helpline [ 1 ] = 1106 ;	/* the current column of the current alignment. */
      helpline [ 0 ] = 1107 ;	/* Try to go on, since this might almost work. */
    } 
    inserror () ; 
//	ABORTCHECK;
  }
} 

void noalignerror ( ) 
{noalignerror_regmem 
  {
    if ( interaction == 3 ) 
		; 
    printnl ( 262 ) ; 		/* ! */
    print ( 1108 ) ;		/* Misplaced  */
  } 
  printesc ( 524 ) ;		/* noalign */
  {
    helpptr = 2 ; 
    helpline [ 1 ] = 1116 ;		/* I expect to see \noalign only after the \cr of */
    helpline [ 0 ] = 1117 ;		/* an alignment. Proceed, and I'll ignore this case. */
  } 
  error () ; 
//	ABORTCHECK;
} 

/* only called from tex8.c */

void omiterror ( ) 
{omiterror_regmem 
  {
    if ( interaction == 3 ) 
    ; 
    printnl ( 262 ) ; 		/* ! */
    print ( 1108 ) ;		/* Misplaced  */
  } 
  printesc ( 527 ) ;		/* omit */
  {
    helpptr = 2 ; 
    helpline [ 1 ] = 1118 ;		/* I expect to see \omit only after tab marks or the \cr of */
    helpline [ 0 ] = 1117 ;		/* an alignment. Proceed, and I'll ignore this case. */
  } 
  error () ; 
//	ABORTCHECK;
} 

void doendv ( ) 
{doendv_regmem 
  if ( curgroup == 6 )  {
    endgraf () ; 
    if ( fincol () ) finrow () ;
  } 
  else offsave () ;
} 

/* only called form tex8.c */

void cserror ( ) 
{cserror_regmem 
  {
    if ( interaction == 3 ) 
    ; 
    printnl ( 262 ) ; 	/* ! */
    print ( 773 ) ;		/* Extra  */
  } 
  printesc ( 502 ) ;	/* endcsname */
  {
    helpptr = 1 ; 
    helpline [ 0 ] = 1120 ;		/* I'm ignoring this, since I wasn't doing a \csname. */
  } 
  error () ; 
//	ABORTCHECK;
} 

void zpushmath ( c ) 
groupcode c ; 
{pushmath_regmem 
  pushnest () ; 
  curlist .modefield = -203 ; 
  curlist .auxfield .cint = 0 ; 
  newsavelevel ( c ) ; 
} 

void initmath ( ) 
{/* 21 40 45 30 */ initmath_regmem 
  scaled w  ; 
  scaled l  ; 
  scaled s  ; 
  halfword p  ; 
  halfword q  ; 
  internalfontnumber f  ; 
  integer n  ; 
  scaled v  ; 
  scaled d  ; 
  gettoken () ; 
  if ( ( curcmd == 3 ) && ( curlist .modefield > 0 ) ) 
  {
    if ( curlist .headfield == curlist .tailfield ) 
    {
      popnest () ; 
      w = -1073741823L ; /* - (2^30 - 1) */
    } 
    else {
	
      linebreak ( eqtb [ (hash_size + 3170) ] .cint ) ; 
      v = mem [ justbox + 4 ] .cint + 2 * fontinfo [ 6 +
		  parambase [ eqtb [ (hash_size + 1834) ] .hh .v.RH ] ] .cint ; 
      w = -1073741823L ;  /* - (2^30 - 1) */
      p = mem [ justbox + 5 ] .hh .v.RH ; 
      while ( p != 0 ) {
	  
	lab21: if ( ( p >= himemmin ) ) 
	{
	  f = mem [ p ] .hh.b0 ; 
	  d = fontinfo [ widthbase [ f ] + fontinfo [ charbase [ f ] + mem [ p 
	  ] .hh.b1 ] .qqqq .b0 ] .cint ; 
	  goto lab40 ; 
	} 
	switch ( mem [ p ] .hh.b0 ) 
	{case 0 : 
	case 1 : 
	case 2 : 
	  {
	    d = mem [ p + 1 ] .cint ; 
	    goto lab40 ; 
	  } 
	  break ; 
	case 6 : 
	  {
	    mem [ memtop - 12 ] = mem [ p + 1 ] ; 
	    mem [ memtop - 12 ] .hh .v.RH = mem [ p ] .hh .v.RH ; 
	    p = memtop - 12 ; 
	    goto lab21 ; 
	  } 
	  break ; 
	case 11 : 
	case 9 : 
	  d = mem [ p + 1 ] .cint ; 
	  break ; 
	case 10 : 
	  {
	    q = mem [ p + 1 ] .hh .v.LH ; 
	    d = mem [ q + 1 ] .cint ; 
	    if ( mem [ justbox + 5 ] .hh.b0 == 1 ) 
	    {
	      if ( ( mem [ justbox + 5 ] .hh.b1 == mem [ q ] .hh.b0 ) && ( mem 
	      [ q + 2 ] .cint != 0 ) ) 
	      v = 1073741823L ;  /* - (2^30 - 1) */
	    } 
	    else if ( mem [ justbox + 5 ] .hh.b0 == 2 ) 
	    {
	      if ( ( mem [ justbox + 5 ] .hh.b1 == mem [ q ] .hh.b1 ) && ( mem 
	      [ q + 3 ] .cint != 0 ) ) 
	      v = 1073741823L ;  /* - (2^30 - 1) */
	    } 
	    if ( mem [ p ] .hh.b1 >= 100 ) 
	    goto lab40 ; 
	  } 
	  break ; 
	case 8 : 
	  d = 0 ; 
	  break ; 
	  default: 
	  d = 0 ; 
	  break ; 
	} 
	if ( v < 1073741823L )  /* - (2^30 - 1) */
	v = v + d ; 
	goto lab45 ; 
	lab40: if ( v < 1073741823L )  /* - (2^30 - 1) */
	{
	  v = v + d ; 
	  w = v ; 
	} 
	else {
	    
	  w = 1073741823L ;  /* - (2^30 - 1) */
	  goto lab30 ; 
	} 
	lab45: p = mem [ p ] .hh .v.RH ; 
      } 
      lab30: ; 
    } 
    if ( eqtb [ (hash_size + 1312) ] .hh .v.RH == 0 ) 
    if ( ( eqtb [ (hash_size + 3747) ] .cint != 0 ) && ( ( ( eqtb [ (hash_size + 3204) ] .cint >= 0 ) && 
    ( curlist .pgfield + 2 > eqtb [ (hash_size + 3204) ] .cint ) ) || ( curlist .pgfield + 
    1 < - (integer) eqtb [ (hash_size + 3204) ] .cint ) ) ) 
    {
      l = eqtb [ (hash_size + 3733) ] .cint - abs ( eqtb [ (hash_size + 3747) ] .cint ) ; 
      if ( eqtb [ (hash_size + 3747) ] .cint > 0 ) 
      s = eqtb [ (hash_size + 3747) ] .cint ; 
      else s = 0 ; 
    } 
    else {
	
      l = eqtb [ (hash_size + 3733) ] .cint ; 
      s = 0 ; 
    } 
    else {
	
      n = mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH ] .hh .v.LH ; 
      if ( curlist .pgfield + 2 >= n ) 
      p = eqtb [ (hash_size + 1312) ] .hh .v.RH + 2 * n ; 
      else p = eqtb [ (hash_size + 1312) ] .hh .v.RH + 2 * ( curlist .pgfield + 2 ) ; 
      s = mem [ p - 1 ] .cint ; 
      l = mem [ p ] .cint ; 
    } 
    pushmath ( 15 ) ; 
    curlist .modefield = 203 ; 
    eqworddefine ( (hash_size + 3207) , -1 ) ; 
    eqworddefine ( (hash_size + 3743) , w ) ; 
    eqworddefine ( (hash_size + 3744) , l ) ; 
    eqworddefine ( (hash_size + 3745) , s ) ; 
    if ( eqtb [ (hash_size + 1316) ] .hh .v.RH != 0 ) /* everydisplay */
		begintokenlist ( eqtb [ (hash_size + 1316) ] .hh .v.RH , 9 ) ; 
    if ( nestptr == 1 ) {
		buildpage () ;
		ABORTCHECK;
	}
  } 
  else {
      
    backinput () ; 
    {
      pushmath ( 15 ) ; 
      eqworddefine ( (hash_size + 3207) , -1 ) ; 
      if ( eqtb [ (hash_size + 1315) ] .hh .v.RH != 0 ) /* everymath */
      begintokenlist ( eqtb [ (hash_size + 1315) ] .hh .v.RH , 8 ) ; 
    } 
  } 
} 
void starteqno ( ) 
{starteqno_regmem 
  savestack [ saveptr + 0 ] .cint = curchr ; 
  incr ( saveptr ) ; 
  {
    pushmath ( 15 ) ; 
    eqworddefine ( (hash_size + 3207) , -1 ) ; 
    if ( eqtb [ (hash_size + 1315) ] .hh .v.RH != 0 ) /* everymath */
    begintokenlist ( eqtb [ (hash_size + 1315) ] .hh .v.RH , 8 ) ; 
  } 
} 

void zscanmath ( p ) 
halfword p ; 
{/* 20 21 10 */ scanmath_regmem 
  integer c  ; 
  lab20: do {
      getxtoken () ; 
	  ABORTCHECK;
  } while ( ! ( ( curcmd != 10 ) && ( curcmd != 0 ) ) ) ; 
  lab21: switch ( curcmd ) 
  {case 11 : 
  case 12 : 
  case 68 : 
    {
      c = eqtb [ (hash_size + 2907) + curchr ] .hh .v.RH ; 
      if ( c == 32768L ) 
      {
	{
	  curcs = curchr + 1 ; 
	  curcmd = eqtb [ curcs ] .hh.b0 ; 
	  curchr = eqtb [ curcs ] .hh .v.RH ; 
	  xtoken () ; 
	  ABORTCHECK;
	  backinput () ; 
	} 
	goto lab20 ; 
      } 
    } 
    break ; 
  case 16 : 
    {
      scancharnum () ; 
	  ABORTCHECK;
      curchr = curval ; 
      curcmd = 68 ; 
      goto lab21 ; 
    } 
    break ; 
  case 17 : 
    {
      scanfifteenbitint () ; 
	  ABORTCHECK;
      c = curval ; 
    } 
    break ; 
  case 69 : 
    c = curchr ; 
    break ; 
  case 15 : 
    {
      scantwentysevenbitint () ; 
	  ABORTCHECK;
      c = curval / 4096 ;  
/*      c = curval >> 12 ; */
    } 
    break ; 
    default: 
    {
      backinput () ; 
      scanleftbrace () ; 
	  ABORTCHECK;
      savestack [ saveptr + 0 ] .cint = p ; 
      incr ( saveptr ) ; 
      pushmath ( 9 ) ; 
      return ; 
    } 
    break ; 
  } 
  mem [ p ] .hh .v.RH = 1 ; 
  mem [ p ] .hh.b1 = c % 256 ; 
/*  mem [ p ] .hh.b1 = c & 255 ; */ /* last 8 bits */
  if ( ( c >= 28672 ) && /* 32768 - 4096 ??? if (c>=var_code) and ... */
	   ( ( eqtb [ (hash_size + 3207) ] .cint >= 0 ) &&
		 ( eqtb [ (hash_size + 3207) ] .cint < 16 ) ) ) 
  mem [ p ] .hh.b0 = eqtb [ (hash_size + 3207) ] .cint ; 
  else mem [ p ] .hh.b0 = ( c / 256 ) % 16 ; 
/*  else mem [ p ] .hh.b0 = ( c >> 8 ) & 15 ; */ /* 4 bits to left */
} 

void zsetmathchar ( c ) 
integer c ; 
{setmathchar_regmem 
  halfword p  ; 
  if ( c >= 32768L ) 
  {
    curcs = curchr + 1 ;					/* ??? */
/*    curcmd = eqtb [ eqtbextra + curcs ] .hh.b0 ;  */ /* was wrong ??? */
    curcmd = eqtb [ curcs ] .hh.b0 ; 
/*    curchr = eqtb [ curcs ] .hh .v.RH ; */ /* should have been eqtbextra ? */
    curchr = eqtb [ curcs ] .hh .v.RH ; 
    xtoken () ; 
	ABORTCHECK;
    backinput () ; 
  } 
  else {
      
    p = newnoad () ; 
    mem [ p + 1 ] .hh .v.RH = 1 ; 
    mem [ p + 1 ] .hh.b1 = c % 256 ;  
/*    mem [ p + 1 ] .hh.b1 = c & 255 ;  */ /* last 8 bits */
    mem [ p + 1 ] .hh.b0 = ( c / 256 ) % 16 ; 
/*    mem [ p + 1 ] .hh.b0 = ( c >> 8 ) & 15 ;  */ /* 4 bits to left */
    if ( c >= 28672 )		/* 32768 - 4096 ? */
    {
      if ( ( ( eqtb [ (hash_size + 3207) ] .cint >= 0 ) && ( eqtb [ (hash_size + 3207) ] .cint < 16 ) ) 
      ) 
      mem [ p + 1 ] .hh.b0 = eqtb [ (hash_size + 3207) ] .cint ; 
      mem [ p ] .hh.b0 = 16 ; 
    } 
    else mem [ p ] .hh.b0 = 16 + ( c / 4096 ) ;  
/*    else mem [ p ] .hh.b0 = 16 + ( c >> 12 ) ;  */
    mem [ curlist .tailfield ] .hh .v.RH = p ; 
    curlist .tailfield = p ; 
  } 
} 

void mathlimitswitch ( ) 
{/* 10 */ mathlimitswitch_regmem 
  if ( curlist .headfield != curlist .tailfield ) 
  if ( mem [ curlist .tailfield ] .hh.b0 == 17 ) 
  {
    mem [ curlist .tailfield ] .hh.b1 = curchr ; 
    return ; 
  } 
  {
    if ( interaction == 3 ) 
    ; 
    printnl ( 262 ) ; 		/* ! */
    print ( 1124 ) ;		/* Limit controls must follow a math operator */
  } 
  {
    helpptr = 1 ; 
    helpline [ 0 ] = 1125 ;		/* I'm ignoring this misplaced \limits or \nolimits command. */
  } 
  error () ; 
//	ABORTCHECK;
} 

void zscandelimiter ( p , r ) 
halfword p ; 
booleane r ; 
{scandelimiter_regmem 
   if ( r ) {
	 scantwentysevenbitint () ;
	 ABORTCHECK;
	}
  else {
      
    do {
	getxtoken () ; 
	ABORTCHECK;
    } while ( ! ( ( curcmd != 10 ) && ( curcmd != 0 ) ) ) ; 
    switch ( curcmd ) 
    {case 11 : 
    case 12 : 
      curval = eqtb [ (hash_size + 3474) + curchr ] .cint ; 
      break ; 
    case 15 : 
      scantwentysevenbitint () ; 
	  ABORTCHECK;
      break ; 
      default: 
      curval = -1 ; 
      break ; 
    } 
  } 
  if ( curval < 0 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ; 	/* ! */
      print ( 1126 ) ;		/* Missing delimiter (. inserted) */
    } 
    {
      helpptr = 6 ; 
      helpline [ 5 ] = 1127 ;	/* I was expecting to see something like `(' or `\{' or */
      helpline [ 4 ] = 1128 ;	/* `\}' here. If you typed, e.g., `{' instead of `\{', you */
      helpline [ 3 ] = 1129 ;	/* should probably delete the `{' by typing `1' now, so that */
      helpline [ 2 ] = 1130 ;	/* braces don't get unbalanced. Otherwise just proceed. */
      helpline [ 1 ] = 1131 ;	/* Acceptable delimiters are characters whose \delcode is */
      helpline [ 0 ] = 1132 ;	/* nonnegative, or you can use `\delimiter <delimiter code>'. */
    } 
    backerror () ; 
	ABORTCHECK;
    curval = 0 ; 
  }
/* attempt to speed up - bkph */	/* is compiler smart enough already ? */
  mem [ p ] .qqqq .b0 = ( curval / 1048576L ) % 16 ;   /* 2^20 */
/*  mem [ p ] .qqqq .b0 = ( curval >> 20 ) & 15 ;  */
  mem [ p ] .qqqq .b1 = ( curval / 4096 ) % 256 ; 
/*  mem [ p ] .qqqq .b1 = ( curval >> 12 ) & 255 ; */
  mem [ p ] .qqqq .b2 = ( curval / 256 ) % 16 ; 
/*  mem [ p ] .qqqq .b2 = ( curval >> 8 ) & 15 ; */
  mem [ p ] .qqqq .b3 = curval % 256 ; 
/*  mem [ p ] .qqqq .b3 = curval & 255 ;  */
} 

void mathradical ( ) 
{mathradical_regmem 
  {
    mem [ curlist .tailfield ] .hh .v.RH = getnode ( 5 ) ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  mem [ curlist .tailfield ] .hh.b0 = 24 ; 
  mem [ curlist .tailfield ] .hh.b1 = 0 ; 
  mem [ curlist .tailfield + 1 ] .hh = emptyfield ; 
  mem [ curlist .tailfield + 3 ] .hh = emptyfield ; 
  mem [ curlist .tailfield + 2 ] .hh = emptyfield ; 
  scandelimiter ( curlist .tailfield + 4 , true ) ; 
  scanmath ( curlist .tailfield + 1 ) ; 
} 

void mathac ( ) 
{mathac_regmem 
  if ( curcmd == 45 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ; 	/* ! */
      print ( 1133 ) ;		/* Please use  */
    } 
    printesc ( 520 ) ;		/* mathaccent */
    print ( 1134 ) ;		/*  for accents in math mode */
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 1135 ;	/* I'm changing \accent to \mathaccent here; wish me luck. */
      helpline [ 0 ] = 1136 ;	/* (Accents are not the same in formulas as they are in text.) */
    } 
    error () ; 
	ABORTCHECK;
  } 
  {
    mem [ curlist .tailfield ] .hh .v.RH = getnode ( 5 ) ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  mem [ curlist .tailfield ] .hh.b0 = 28 ; 
  mem [ curlist .tailfield ] .hh.b1 = 0 ; 
  mem [ curlist .tailfield + 1 ] .hh = emptyfield ; 
  mem [ curlist .tailfield + 3 ] .hh = emptyfield ; 
  mem [ curlist .tailfield + 2 ] .hh = emptyfield ; 
  mem [ curlist .tailfield + 4 ] .hh .v.RH = 1 ; 
  scanfifteenbitint () ; 
  ABORTCHECK;
  mem [ curlist .tailfield + 4 ] .hh.b1 = curval % 256 ;  
/*  mem [ curlist .tailfield + 4 ] .hh.b1 = curval & 255 ; */
  if ( ( curval >= 28672 ) && /* 32768 - 4096 ? */
	   ( ( eqtb [ (hash_size + 3207) ] .cint >= 0 ) &&
		 ( eqtb [ (hash_size + 3207) ] .cint < 16 ) ) ) 
  mem [ curlist .tailfield + 4 ] .hh.b0 = eqtb [ (hash_size + 3207) ] .cint ; 
  else mem [ curlist .tailfield + 4 ] .hh.b0 = ( curval / 256 ) % 16 ; 
/*  else mem [ curlist .tailfield + 4 ] .hh.b0 = ( curval >> 8 ) & 15 ; */
  scanmath ( curlist .tailfield + 1 ) ; 
} 

void appendchoices ( ) 
{appendchoices_regmem 
  {
    mem [ curlist .tailfield ] .hh .v.RH = newchoice () ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  incr ( saveptr ) ; 
  savestack [ saveptr - 1 ] .cint = 0 ; 
  pushmath ( 13 ) ; 
  scanleftbrace () ; 
  ABORTCHECK;
} 

halfword zfinmlist ( p ) 
halfword p ; 
{register halfword Result; finmlist_regmem 
  halfword q  ; 
  if ( curlist .auxfield .cint != 0 ) 
  {
    mem [ curlist .auxfield .cint + 3 ] .hh .v.RH = 3 ; 
    mem [ curlist .auxfield .cint + 3 ] .hh .v.LH = mem [ curlist .headfield ] 
    .hh .v.RH ; 
    if ( p == 0 ) 
    q = curlist .auxfield .cint ; 
    else {
	
      q = mem [ curlist .auxfield .cint + 2 ] .hh .v.LH ; 
      if ( mem [ q ] .hh.b0 != 30 ) {
		  confusion ( 871 ) ;	/* right */
		  return 0;				// abortflag set
	  }
      mem [ curlist .auxfield .cint + 2 ] .hh .v.LH = mem [ q ] .hh .v.RH ; 
      mem [ q ] .hh .v.RH = curlist .auxfield .cint ; 
      mem [ curlist .auxfield .cint ] .hh .v.RH = p ; 
    } 
  } 
  else {
      
    mem [ curlist .tailfield ] .hh .v.RH = p ; 
    q = mem [ curlist .headfield ] .hh .v.RH ; 
  } 
  popnest () ; 
  Result = q ; 
  return Result ; 
} 

void buildchoices ( ) 
{/* 10 */ buildchoices_regmem 
  halfword p  ; 
  unsave () ; 
  p = finmlist ( 0 ) ; 
  switch ( savestack [ saveptr - 1 ] .cint ) 
  {case 0 : 
    mem [ curlist .tailfield + 1 ] .hh .v.LH = p ; 
    break ; 
  case 1 : 
    mem [ curlist .tailfield + 1 ] .hh .v.RH = p ; 
    break ; 
  case 2 : 
    mem [ curlist .tailfield + 2 ] .hh .v.LH = p ; 
    break ; 
  case 3 : 
    {
      mem [ curlist .tailfield + 2 ] .hh .v.RH = p ; 
      decr ( saveptr ) ; 
      return ; 
    } 
    break ; 
  } 
  incr ( savestack [ saveptr - 1 ] .cint ) ; 
  pushmath ( 13 ) ; 
  scanleftbrace () ; 
  ABORTCHECK;
} 

void subsup ( ) 
{subsup_regmem 
/*  smallnumber t  ; */
  int t  ;							/* 95/Jan/7 */
  halfword p  ; 
  t = 0 ; 
  p = 0 ; 
  if ( curlist .tailfield != curlist .headfield ) 
  if ( ( mem [ curlist .tailfield ] .hh.b0 >= 16 ) &&
	   ( mem [ curlist .tailfield ] .hh.b0 < 30 ) ) 
  {
    p = curlist .tailfield + 2 + curcmd - 7 ; 
    t = mem [ p ] .hh .v.RH ; 
  } 
  if ( ( p == 0 ) || ( t != 0 ) ) 
  {
    {
      mem [ curlist .tailfield ] .hh .v.RH = newnoad () ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    p = curlist .tailfield + 2 + curcmd - 7 ; 
    if ( t != 0 ) 
    {
      if ( curcmd == 7 ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ; 	/* ! */
	  print ( 1137 ) ;		/* Double superscript */
	} 
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 1138 ;	/* I treat `x^1^2' essentially like `x^1{}^2'.  */
	} 
      } 
      else {
	  
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ; 	/* ! */
	  print ( 1139 ) ;		/* Double subscript */
	} 
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 1140 ;	/* I treat `x_1_2' essentially like `x_1{}_2'. */
	} 
      } 
      error () ; 
	  ABORTCHECK;
    } 
  } 
  scanmath ( p ) ; 
} 

/* used to continue here with mathfraction etc in tex7.c */

/*****************************************************************************/

/* moved down here to avoid pragma optimize questions 96/Sep/12 */

/* #pragma optimize("g", off) */ 					/* for MC VS compiler */
#pragma optimize("a", off)  					/* for MC VS compiler */

void zpackage ( c ) 
smallnumber c ; 
{package_regmem 
  scaled h  ; 
  halfword p  ; 
  scaled d  ; 
  d = eqtb [ (hash_size + 3737) ] .cint ; 
  unsave () ; 
  saveptr = saveptr - 3 ; 
  if ( curlist .modefield == -102 ) 
  curbox = hpack ( mem [ curlist .headfield ] .hh .v.RH , savestack [ saveptr 
  + 2 ] .cint , savestack [ saveptr + 1 ] .cint ) ; 
  else {
      
    curbox = vpackage ( mem [ curlist .headfield ] .hh .v.RH , savestack [ 
    saveptr + 2 ] .cint , savestack [ saveptr + 1 ] .cint , d ) ; 
    if ( c == 4 ) 
    {
      h = 0 ; 
      p = mem [ curbox + 5 ] .hh .v.RH ; 
      if ( p != 0 ) 
      if ( mem [ p ] .hh.b0 <= 2 ) 
      h = mem [ p + 3 ] .cint ; 
      mem [ curbox + 2 ] .cint = mem [ curbox + 2 ] .cint - h + mem [ curbox + 
      3 ] .cint ; 
      mem [ curbox + 3 ] .cint = h ; 
    } 
  } 
  popnest () ; 
  boxend ( savestack [ saveptr + 0 ] .cint ) ; 
} 

/* #pragma optimize("g", on) */					/* for MC VS compiler */
/* #pragma optimize("a", on) */					/* for MC VS compiler */
/* #pragma optimize("a",) */					/* 94/Jan/25 */
#pragma optimize ("", on)						/* 96/Sep/12 */

/****************************************************************************/