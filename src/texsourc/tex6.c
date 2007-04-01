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

/* #pragma optimize("a", off) */					/* 98/Dec/10 experiment */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void alignpeek ( ) 
{/* 20 */ alignpeek_regmem 
  lab20: alignstate = 1000000L ; 
  do {
      getxtoken () ; 
	  ABORTCHECK;
  } while ( ! ( curcmd != 10 ) ) ; 
  if ( curcmd == 34 ) {
    scanleftbrace () ; 
	ABORTCHECK;
    newsavelevel ( 7 ) ; 
    if ( curlist .modefield == -1 )  normalparagraph () ; 
  } 
  else if ( curcmd == 2 ) {
	  finalign () ;
	  ABORTCHECK;
  }
  else if ( ( curcmd == 5 ) && ( curchr == 258 ) ) 
  goto lab20 ; 
  else {
    initrow () ; 
    initcol () ; 
  } 
} 

/* used in itex.c only */

halfword zfiniteshrink ( p ) 
halfword p ; 
{register halfword Result; finiteshrink_regmem 
  halfword q  ; 
  if ( noshrinkerroryet ) {
    noshrinkerroryet = false ; 
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 911 ) ;		/* Infinite glue shrinkage found in a paragraph */
    } 
    {
      helpptr = 5 ; 
      helpline [ 4 ] = 912 ;	/* The paragraph just ended includes some glue that has */
      helpline [ 3 ] = 913 ;	/* infinite shrinkability, e.g., `\hskip 0pt minus 1fil'. */
      helpline [ 2 ] = 914 ;	/* Such glue doesn't belong there---it allows a paragraph */
      helpline [ 1 ] = 915 ;	/* of any length to fit on one line. But it's safe to proceed, */
      helpline [ 0 ] = 916 ;	/* since the offensive shrinkability has been made finite. */
    } 
    error () ; 
	ABORTCHECKZERO;
  } 
  q = newspec ( p ) ; 
  mem [ q ] .hh.b1 = 0 ; 
  deleteglueref ( p ) ; 
  Result = q ; 
  return Result ; 
} 

void ztrybreak ( pi , breaktype ) 
integer pi ; 
smallnumber breaktype ; 
{/* 10 30 31 22 60 */ trybreak_regmem 
  halfword r  ; 
  halfword prevr  ; 
  halfword oldl  ; 
  booleane nobreakyet  ; 
  halfword prevprevr  ; 
  halfword s  ; 
  halfword q  ; 
  halfword v  ; 
  integer t  ; 
  internalfontnumber f  ; 
  halfword l  ; 
  booleane noderstaysactive  ; 
  scaled linewidth  ; 
  char fitclass  ; 
  halfword b  ;				/* current badness */
  integer d  ; 
  booleane artificialdemerits  ; 
  halfword savelink  ; 
  scaled shortfall  ; 
  if ( abs ( pi ) >= 10000 ) 
  if ( pi > 0 ) 
  goto lab10 ; 
  else pi = -10000 ; 
  nobreakyet = true ; 
  prevr = memtop - 7 ; 
  oldl = 0 ; 
  curactivewidth [ 1 ] = activewidth [ 1 ] ; 
  curactivewidth [ 2 ] = activewidth [ 2 ] ; 
  curactivewidth [ 3 ] = activewidth [ 3 ] ; 
  curactivewidth [ 4 ] = activewidth [ 4 ] ; 
  curactivewidth [ 5 ] = activewidth [ 5 ] ; 
  curactivewidth [ 6 ] = activewidth [ 6 ] ; 
  while ( true ) {
    lab22: r = mem [ prevr ] .hh .v.RH ; 
    if ( mem [ r ] .hh.b0 == 2 ) 
    {
      curactivewidth [ 1 ] = curactivewidth [ 1 ] + mem [ r + 1 ] .cint ; 
      curactivewidth [ 2 ] = curactivewidth [ 2 ] + mem [ r + 2 ] .cint ; 
      curactivewidth [ 3 ] = curactivewidth [ 3 ] + mem [ r + 3 ] .cint ; 
      curactivewidth [ 4 ] = curactivewidth [ 4 ] + mem [ r + 4 ] .cint ; 
      curactivewidth [ 5 ] = curactivewidth [ 5 ] + mem [ r + 5 ] .cint ; 
      curactivewidth [ 6 ] = curactivewidth [ 6 ] + mem [ r + 6 ] .cint ; 
      prevprevr = prevr ; 
      prevr = r ; 
      goto lab22 ; 
    } 
    {
      l = mem [ r + 1 ] .hh .v.LH ; 
      if ( l > oldl ) 
      {
	if ( ( minimumdemerits < 1073741823L ) &&  /* 2^30 - 1 */
		 ( ( oldl != easyline ) || ( r == memtop - 7 ) ) ) 
	{
	  if ( nobreakyet ) 
	  {
	    nobreakyet = false ; 
	    breakwidth [ 1 ] = background [ 1 ] ; 
	    breakwidth [ 2 ] = background [ 2 ] ; 
	    breakwidth [ 3 ] = background [ 3 ] ; 
	    breakwidth [ 4 ] = background [ 4 ] ; 
	    breakwidth [ 5 ] = background [ 5 ] ; 
	    breakwidth [ 6 ] = background [ 6 ] ; 
	    s = curp ; 
/* if break_type>unhyphenated then if cur_p<>null then l.16451 */
	    if ( breaktype > 0 ) 
	    if ( curp != 0 ) 
	    {
	      t = mem [ curp ] .hh.b1 ; 
	      v = curp ; 
	      s = mem [ curp + 1 ] .hh .v.RH ; 
	      while ( t > 0 ) {
		  
		decr ( t ) ; 
		v = mem [ v ] .hh .v.RH ; 
		if ( ( v >= himemmin ) ) 
		{
		  f = mem [ v ] .hh.b0 ; 
		  breakwidth [ 1 ] = breakwidth [ 1 ] - fontinfo [ widthbase [ 
		  f ] + fontinfo [ charbase [ f ] + mem [ v ] .hh.b1 ] .qqqq 
		  .b0 ] .cint ; 
		} 
		else switch ( mem [ v ] .hh.b0 ) 
		{case 6 : 
		  {
		    f = mem [ v + 1 ] .hh.b0 ; 
		    breakwidth [ 1 ] = breakwidth [ 1 ] - fontinfo [ widthbase 
		    [ f ] + fontinfo [ charbase [ f ] + mem [ v + 1 ] .hh.b1 ] 
		    .qqqq .b0 ] .cint ; 
		  } 
		  break ; 
		case 0 : 
		case 1 : 
		case 2 : 
		case 11 : 
		  breakwidth [ 1 ] = breakwidth [ 1 ] - mem [ v + 1 ] .cint ; 
		  break ; 
		  default: 
			  {
				  confusion ( 917 ) ;	/* disc1 */
				  return;				// abortflag set
			  }
			  break ; 
		} 
	      } 
	      while ( s != 0 ) {	/* while s<>null do l.16453 */
		  
		if ( ( s >= himemmin ) ) 
		{
		  f = mem [ s ] .hh.b0 ; 
		  breakwidth [ 1 ] = breakwidth [ 1 ] + fontinfo [ widthbase [ 
		  f ] + fontinfo [ charbase [ f ] + mem [ s ] .hh.b1 ] .qqqq 
		  .b0 ] .cint ; 
		} 
		else switch ( mem [ s ] .hh.b0 ) 
		{case 6 : 
		  {
		    f = mem [ s + 1 ] .hh.b0 ; 
		    breakwidth [ 1 ] = breakwidth [ 1 ] + fontinfo [ widthbase 
		    [ f ] + fontinfo [ charbase [ f ] + mem [ s + 1 ] .hh.b1 ] 
		    .qqqq .b0 ] .cint ; 
		  } 
		  break ; 
		case 0 : 
		case 1 : 
		case 2 : 
/* didn't used to drop through to case 11 from case 2 in  3.141 */
		case 11 : 
/* used to be some extra code here in case 11 in 3.141 */
		  breakwidth [ 1 ] = breakwidth [ 1 ] + mem [ s + 1 ] .cint ; 
		  break ; 
		  default: 
			  {
				  confusion ( 918 ) ;	/* disc1 */
				  return;				// abortflag set
			  }
			  break ; 
		} 
/* used to be an increment t here in 3.141 */
		s = mem [ s ] .hh .v.RH ; 
	      } 
	      breakwidth [ 1 ] = breakwidth [ 1 ] + discwidth ; 
/* used to be a test on t here instead in 3.141 */
	      if ( mem [ curp + 1 ] .hh .v.RH == 0 ) 
	      s = mem [ v ] .hh .v.RH ; 
	    } 
	    while ( s != 0 ) {
		
	      if ( ( s >= himemmin ) ) 
	      goto lab30 ; 
	      switch ( mem [ s ] .hh.b0 ) 
	      {case 10 : 
		{
		  v = mem [ s + 1 ] .hh .v.LH ; 
		  breakwidth [ 1 ] = breakwidth [ 1 ] - mem [ v + 1 ] .cint ; 
		  breakwidth [ 2 + mem [ v ] .hh.b0 ] = breakwidth [ 2 + mem [ 
		  v ] .hh.b0 ] - mem [ v + 2 ] .cint ; 
		  breakwidth [ 6 ] = breakwidth [ 6 ] - mem [ v + 3 ] .cint ; 
		} 
		break ; 
	      case 12 : 
		; 
		break ; 
	      case 9 : 
/* case 9 used to drop through to case 11 in 3.141 */
		breakwidth [ 1 ] = breakwidth [ 1 ] - mem [ s + 1 ] .cint ; 
		break ; 
	      case 11 : 
/* changes here in nature of test since 3.141 */
		if ( mem [ s ] .hh.b1 != 1 ) 
		goto lab30 ; 
		else breakwidth [ 1 ] = breakwidth [ 1 ] - mem [ s + 1 ] .cint 
		; 
		break ; 
		default: 
		goto lab30 ; 
		break ; 
	      } 
	      s = mem [ s ] .hh .v.RH ; 
	    } 
	    lab30: ; 
	  } 
	  if ( mem [ prevr ] .hh.b0 == 2 ) 
	  {
	    mem [ prevr + 1 ] .cint = mem [ prevr + 1 ] .cint - curactivewidth 
	    [ 1 ] + breakwidth [ 1 ] ; 
	    mem [ prevr + 2 ] .cint = mem [ prevr + 2 ] .cint - curactivewidth 
	    [ 2 ] + breakwidth [ 2 ] ; 
	    mem [ prevr + 3 ] .cint = mem [ prevr + 3 ] .cint - curactivewidth 
	    [ 3 ] + breakwidth [ 3 ] ; 
	    mem [ prevr + 4 ] .cint = mem [ prevr + 4 ] .cint - curactivewidth 
	    [ 4 ] + breakwidth [ 4 ] ; 
	    mem [ prevr + 5 ] .cint = mem [ prevr + 5 ] .cint - curactivewidth 
	    [ 5 ] + breakwidth [ 5 ] ; 
	    mem [ prevr + 6 ] .cint = mem [ prevr + 6 ] .cint - curactivewidth 
	    [ 6 ] + breakwidth [ 6 ] ; 
	  } 
	  else if ( prevr == memtop - 7 ) 
	  {
	    activewidth [ 1 ] = breakwidth [ 1 ] ; 
	    activewidth [ 2 ] = breakwidth [ 2 ] ; 
	    activewidth [ 3 ] = breakwidth [ 3 ] ; 
	    activewidth [ 4 ] = breakwidth [ 4 ] ; 
	    activewidth [ 5 ] = breakwidth [ 5 ] ; 
	    activewidth [ 6 ] = breakwidth [ 6 ] ; 
	  } 
	  else {
	      
	    q = getnode ( 7 ) ; 
	    mem [ q ] .hh .v.RH = r ; 
	    mem [ q ] .hh.b0 = 2 ; 
	    mem [ q ] .hh.b1 = 0 ; 
	    mem [ q + 1 ] .cint = breakwidth [ 1 ] - curactivewidth [ 1 ] ; 
	    mem [ q + 2 ] .cint = breakwidth [ 2 ] - curactivewidth [ 2 ] ; 
	    mem [ q + 3 ] .cint = breakwidth [ 3 ] - curactivewidth [ 3 ] ; 
	    mem [ q + 4 ] .cint = breakwidth [ 4 ] - curactivewidth [ 4 ] ; 
	    mem [ q + 5 ] .cint = breakwidth [ 5 ] - curactivewidth [ 5 ] ; 
	    mem [ q + 6 ] .cint = breakwidth [ 6 ] - curactivewidth [ 6 ] ; 
	    mem [ prevr ] .hh .v.RH = q ; 
	    prevprevr = prevr ; 
	    prevr = q ; 
	  } 
	  if ( abs ( eqtb [ (hash_size + 3179) ] .cint ) >= 1073741823L - minimumdemerits ) 
	  minimumdemerits = 1073741822L ; /* 2^30 - 2 */
	  else minimumdemerits = minimumdemerits + abs ( eqtb [ (hash_size + 3179) ] .cint 
	  ) ; 
	  {
		  register integer for_end; 
		  fitclass = 0 ; 
		  for_end = 3 ; 
		  if ( fitclass <= for_end) do 
		  {
	      if ( minimaldemerits [ fitclass ] <= minimumdemerits ) 
	      {
		q = getnode ( 2 ) ; 
		mem [ q ] .hh .v.RH = passive ; 
		passive = q ; 
		mem [ q + 1 ] .hh .v.RH = curp ; 
	;
#ifdef STAT
		incr ( passnumber ) ; 
		mem [ q ] .hh .v.LH = passnumber ; 
#endif /* STAT */
		mem [ q + 1 ] .hh .v.LH = bestplace [ fitclass ] ; 
		q = getnode ( 3 ) ; 
		mem [ q + 1 ] .hh .v.RH = passive ; 
		mem [ q + 1 ] .hh .v.LH = bestplline [ fitclass ] + 1 ; 
		mem [ q ] .hh.b1 = fitclass ; 
		mem [ q ] .hh.b0 = breaktype ; 
		mem [ q + 2 ] .cint = minimaldemerits [ fitclass ] ; 
		mem [ q ] .hh .v.RH = r ; 
		mem [ prevr ] .hh .v.RH = q ; 
		prevr = q ; 
	;
#ifdef STAT
		if ( eqtb [ (hash_size + 3195) ] .cint > 0 ) 
		{
		  printnl ( 919 ) ;			/* @@ */
		  printint ( mem [ passive ] .hh .v.LH ) ; 
		  print ( 920 ) ;	/* : line  */
		  printint ( mem [ q + 1 ] .hh .v.LH - 1 ) ; 
		  printchar ( 46 ) ;	/* . */
		  printint ( fitclass ) ; 
		  if ( breaktype == 1 ) 
		  printchar ( 45 ) ;	/* - */
		  print ( 921 ) ;	/* t= */
		  printint ( mem [ q + 2 ] .cint ) ; 
		  print ( 922 ) ;	/* -> @@ */
		  if ( mem [ passive + 1 ] .hh .v.LH == 0 ) 
		  printchar ( 48 ) ;	/* 0 */
		  else printint ( mem [ mem [ passive + 1 ] .hh .v.LH ] .hh 
		  .v.LH ) ; 
		} 
#endif /* STAT */
	      } 
	      minimaldemerits [ fitclass ] = 1073741823L ; /* 2^30 - 1 */
	    } 
	  while ( fitclass++ < for_end ) ; } 
	  minimumdemerits = 1073741823L ;	/* 2^30 - 1 */
	  if ( r != memtop - 7 ) 
	  {
	    q = getnode ( 7 ) ; 
	    mem [ q ] .hh .v.RH = r ; 
	    mem [ q ] .hh.b0 = 2 ; 
	    mem [ q ] .hh.b1 = 0 ; 
	    mem [ q + 1 ] .cint = curactivewidth [ 1 ] - breakwidth [ 1 ] ; 
	    mem [ q + 2 ] .cint = curactivewidth [ 2 ] - breakwidth [ 2 ] ; 
	    mem [ q + 3 ] .cint = curactivewidth [ 3 ] - breakwidth [ 3 ] ; 
	    mem [ q + 4 ] .cint = curactivewidth [ 4 ] - breakwidth [ 4 ] ; 
	    mem [ q + 5 ] .cint = curactivewidth [ 5 ] - breakwidth [ 5 ] ; 
	    mem [ q + 6 ] .cint = curactivewidth [ 6 ] - breakwidth [ 6 ] ; 
	    mem [ prevr ] .hh .v.RH = q ; 
	    prevprevr = prevr ; 
	    prevr = q ; 
	  } 
	} 
	if ( r == memtop - 7 ) 
	goto lab10 ; 
	if ( l > easyline ) 
	{
	  linewidth = secondwidth ; 
	  oldl = 262142L ;				/* 2^18 - 2 ? */
	} 
	else {
	    
	  oldl = l ; 
	  if ( l > lastspecialline ) 
	  linewidth = secondwidth ; 
	  else if ( eqtb [ (hash_size + 1312) ] .hh .v.RH == 0 ) 
	  linewidth = firstwidth ; 
	  else linewidth = mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH + 2 * l ] .cint ; 
	} 
      } 
    } 
    {
      artificialdemerits = false ; 
      shortfall = linewidth - curactivewidth [ 1 ] ;	/* linewidth may be ... */
      if ( shortfall > 0 ) 
      if ( ( curactivewidth [ 3 ] != 0 ) || ( curactivewidth [ 4 ] != 0 ) || ( 
      curactivewidth [ 5 ] != 0 ) ) 
      {
	b = 0 ; 
	fitclass = 2 ; 
      } 
      else {
	  
	if ( shortfall > 7230584L ) 
	if ( curactivewidth [ 2 ] < 1663497L ) 
	{
	  b = 10000 ; 
	  fitclass = 0 ; 
	  goto lab31 ; 
	} 
	b = badness ( shortfall , curactivewidth [ 2 ] ) ; 
	if ( b > 12 ) 
	if ( b > 99 ) 
	fitclass = 0 ; 
	else fitclass = 1 ; 
	else fitclass = 2 ; 
	lab31: ; 
      } 
      else {
	  
	if ( - (integer) shortfall > curactivewidth [ 6 ] ) 
	b = 10001 ; 
	else b = badness ( - (integer) shortfall , curactivewidth [ 6 ] ) ; 
	if ( b > 12 ) 
	fitclass = 3 ; 
	else fitclass = 2 ; 
      } 
      if ( ( b > 10000 ) || ( pi == -10000 ) ) 
      {
	if ( finalpass && ( minimumdemerits == 1073741823L ) &&  /* 2^30 - 1 */
		 ( mem [ r ] .hh .v.RH == memtop - 7 ) && ( prevr == memtop - 7 ) ) 
	artificialdemerits = true ; 
	else if ( b > threshold ) 
	goto lab60 ; 
	noderstaysactive = false ; 
      } 
      else {
	  
	prevr = r ; 
	if ( b > threshold ) 
	goto lab22 ; 
	noderstaysactive = true ; 
      } 
      if ( artificialdemerits ) 
      d = 0 ; 
      else {
	  
	d = eqtb [ (hash_size + 3165) ] .cint + b ; 
	if ( abs ( d ) >= 10000 ) 
	d = 100000000L ; 
	else d = d * d ; 
	if ( pi != 0 ) 
	if ( pi > 0 ) 
	d = d + pi * pi ; 
	else if ( pi > -10000 ) 
	d = d - pi * pi ; 
	if ( ( breaktype == 1 ) && ( mem [ r ] .hh.b0 == 1 ) ) 
	if ( curp != 0 ) 
	d = d + eqtb [ (hash_size + 3177) ] .cint ; 
	else d = d + eqtb [ (hash_size + 3178) ] .cint ; 
	if ( abs ( toint ( fitclass ) - toint ( mem [ r ] .hh.b1 ) ) > 1 ) 
	d = d + eqtb [ (hash_size + 3179) ] .cint ; 
      } 
	;
#ifdef STAT
      if ( eqtb [ (hash_size + 3195) ] .cint > 0 ) 
      {
	if ( printednode != curp ) 
	{
	  printnl ( 335 ) ;		/*  */
	  if ( curp == 0 ) 
	  shortdisplay ( mem [ printednode ] .hh .v.RH ) ; 
	  else {
	      
	    savelink = mem [ curp ] .hh .v.RH ; 
	    mem [ curp ] .hh .v.RH = 0 ; 
	    printnl ( 335 ) ;	/*  */
	    shortdisplay ( mem [ printednode ] .hh .v.RH ) ; 
	    mem [ curp ] .hh .v.RH = savelink ; 
	  } 
	  printednode = curp ; 
	} 
	printnl ( 64 ) ;		/* @ */
	if ( curp == 0 ) 
		printesc ( 594 ) ;	/* par */
	else if ( mem [ curp ] .hh.b0 != 10 ) 
	{
	  if ( mem [ curp ] .hh.b0 == 12 ) 
		  printesc ( 528 ) ;	/* penalty */
	  else if ( mem [ curp ] .hh.b0 == 7 ) 
		  printesc ( 346 ) ;	/* discretionary */
	  else if ( mem [ curp ] .hh.b0 == 11 ) 
		  printesc ( 337 ) ;	/* kern */
	  else printesc ( 340 ) ;	/* math */
	} 
	print ( 923 ) ;		/* via @@ */
	if ( mem [ r + 1 ] .hh .v.RH == 0 ) 
	printchar ( 48 ) ;	/* 0 */
	else printint ( mem [ mem [ r + 1 ] .hh .v.RH ] .hh .v.LH ) ; 
	print ( 924 ) ;		/* b= */
	if ( b > 10000 ) 
	printchar ( 42 ) ; 	/* * */
	else printint ( b ) ; 
	print ( 925 ) ;		/* p= */
	printint ( pi ) ; 
	print ( 926 ) ;		/* d= */
	if ( artificialdemerits ) 
	printchar ( 42 ) ; 	/* * */
	else printint ( d ) ; 
      } 
#endif /* STAT */
      d = d + mem [ r + 2 ] .cint ; 
      if ( d <= minimaldemerits [ fitclass ] ) 
      {
	minimaldemerits [ fitclass ] = d ; 
	bestplace [ fitclass ] = mem [ r + 1 ] .hh .v.RH ; 
	bestplline [ fitclass ] = l ; 
	if ( d < minimumdemerits ) 
	minimumdemerits = d ; 
      } 
      if ( noderstaysactive ) 
      goto lab22 ; 
      lab60: mem [ prevr ] .hh .v.RH = mem [ r ] .hh .v.RH ; 
      freenode ( r , 3 ) ; 
      if ( prevr == memtop - 7 ) 
      {
	r = mem [ memtop - 7 ] .hh .v.RH ; 
	if ( mem [ r ] .hh.b0 == 2 ) 
	{
	  activewidth [ 1 ] = activewidth [ 1 ] + mem [ r + 1 ] .cint ; 
	  activewidth [ 2 ] = activewidth [ 2 ] + mem [ r + 2 ] .cint ; 
	  activewidth [ 3 ] = activewidth [ 3 ] + mem [ r + 3 ] .cint ; 
	  activewidth [ 4 ] = activewidth [ 4 ] + mem [ r + 4 ] .cint ; 
	  activewidth [ 5 ] = activewidth [ 5 ] + mem [ r + 5 ] .cint ; 
	  activewidth [ 6 ] = activewidth [ 6 ] + mem [ r + 6 ] .cint ; 
	  curactivewidth [ 1 ] = activewidth [ 1 ] ; 
	  curactivewidth [ 2 ] = activewidth [ 2 ] ; 
	  curactivewidth [ 3 ] = activewidth [ 3 ] ; 
	  curactivewidth [ 4 ] = activewidth [ 4 ] ; 
	  curactivewidth [ 5 ] = activewidth [ 5 ] ; 
	  curactivewidth [ 6 ] = activewidth [ 6 ] ; 
	  mem [ memtop - 7 ] .hh .v.RH = mem [ r ] .hh .v.RH ; 
	  freenode ( r , 7 ) ; 
	} 
      } 
      else if ( mem [ prevr ] .hh.b0 == 2 ) 
      {
	r = mem [ prevr ] .hh .v.RH ; 
	if ( r == memtop - 7 ) 
	{
	  curactivewidth [ 1 ] = curactivewidth [ 1 ] - mem [ prevr + 1 ] 
	  .cint ; 
	  curactivewidth [ 2 ] = curactivewidth [ 2 ] - mem [ prevr + 2 ] 
	  .cint ; 
	  curactivewidth [ 3 ] = curactivewidth [ 3 ] - mem [ prevr + 3 ] 
	  .cint ; 
	  curactivewidth [ 4 ] = curactivewidth [ 4 ] - mem [ prevr + 4 ] 
	  .cint ; 
	  curactivewidth [ 5 ] = curactivewidth [ 5 ] - mem [ prevr + 5 ] 
	  .cint ; 
	  curactivewidth [ 6 ] = curactivewidth [ 6 ] - mem [ prevr + 6 ] 
	  .cint ; 
	  mem [ prevprevr ] .hh .v.RH = memtop - 7 ;	/* prevprevr may be used ... */
	  freenode ( prevr , 7 ) ; 
	  prevr = prevprevr ; 
	} 
	else if ( mem [ r ] .hh.b0 == 2 ) 
	{
	  curactivewidth [ 1 ] = curactivewidth [ 1 ] + mem [ r + 1 ] .cint ; 
	  curactivewidth [ 2 ] = curactivewidth [ 2 ] + mem [ r + 2 ] .cint ; 
	  curactivewidth [ 3 ] = curactivewidth [ 3 ] + mem [ r + 3 ] .cint ; 
	  curactivewidth [ 4 ] = curactivewidth [ 4 ] + mem [ r + 4 ] .cint ; 
	  curactivewidth [ 5 ] = curactivewidth [ 5 ] + mem [ r + 5 ] .cint ; 
	  curactivewidth [ 6 ] = curactivewidth [ 6 ] + mem [ r + 6 ] .cint ; 
	  mem [ prevr + 1 ] .cint = mem [ prevr + 1 ] .cint + mem [ r + 1 ] 
	  .cint ; 
	  mem [ prevr + 2 ] .cint = mem [ prevr + 2 ] .cint + mem [ r + 2 ] 
	  .cint ; 
	  mem [ prevr + 3 ] .cint = mem [ prevr + 3 ] .cint + mem [ r + 3 ] 
	  .cint ; 
	  mem [ prevr + 4 ] .cint = mem [ prevr + 4 ] .cint + mem [ r + 4 ] 
	  .cint ; 
	  mem [ prevr + 5 ] .cint = mem [ prevr + 5 ] .cint + mem [ r + 5 ] 
	  .cint ; 
	  mem [ prevr + 6 ] .cint = mem [ prevr + 6 ] .cint + mem [ r + 6 ] 
	  .cint ; 
	  mem [ prevr ] .hh .v.RH = mem [ r ] .hh .v.RH ; 
	  freenode ( r , 7 ) ; 
	} 
      } 
    } 
  } 
  lab10: 
	;
#ifdef STAT
  if ( curp == printednode ) 
  if ( curp != 0 ) 
  if ( mem [ curp ] .hh.b0 == 7 ) {
    t = mem [ curp ] .hh.b1 ; 
    while ( t > 0 ) {
      decr ( t ) ; 
      printednode = mem [ printednode ] .hh .v.RH ; 
    } 
  } 
#endif /* STAT */
/*	must exit here, there are no internal return - except for confusion */
/*  savedbadness = b; */			/* 96/Feb/9 - for test in itex.c */
} 

/* end of the old tex5.c here */

void zpostlinebreak ( finalwidowpenalty ) 
integer finalwidowpenalty ; 
{/* 30 31 */ postlinebreak_regmem 
  halfword q, r, s  ; 
  booleane discbreak  ; 
  booleane postdiscbreak  ; 
  scaled curwidth  ; 
  scaled curindent  ; 
  quarterword t  ; 
  integer pen  ; 
  halfword curline  ; 
  q = mem [ bestbet + 1 ] .hh .v.RH ; 
  curp = 0 ; 
  do {
      r = q ; 
    q = mem [ q + 1 ] .hh .v.LH ; 
    mem [ r + 1 ] .hh .v.LH = curp ; 
    curp = r ; 
  } while ( ! ( q == 0 ) ) ; 
  curline = curlist .pgfield + 1 ; 
  do {
      q = mem [ curp + 1 ] .hh .v.RH ; 
    discbreak = false ; 
    postdiscbreak = false ; 
    if ( q != 0 ) 
    if ( mem [ q ] .hh.b0 == 10 ) 
    {
      deleteglueref ( mem [ q + 1 ] .hh .v.LH ) ; 
      mem [ q + 1 ] .hh .v.LH = eqtb [ (hash_size + 790) ] .hh .v.RH ; 
      mem [ q ] .hh.b1 = 9 ; 
      incr ( mem [ eqtb [ (hash_size + 790) ] .hh .v.RH ] .hh .v.RH ) ; 
      goto lab30 ; 
    } 
    else {
	
      if ( mem [ q ] .hh.b0 == 7 ) 
      {
	t = mem [ q ] .hh.b1 ; 
	if ( t == 0 ) 
	r = mem [ q ] .hh .v.RH ; 
	else {
	    
	  r = q ; 
	  while ( t > 1 ) {
	      
	    r = mem [ r ] .hh .v.RH ; 
	    decr ( t ) ; 
	  } 
	  s = mem [ r ] .hh .v.RH ; 
	  r = mem [ s ] .hh .v.RH ; 
	  mem [ s ] .hh .v.RH = 0 ; 
	  flushnodelist ( mem [ q ] .hh .v.RH ) ; 
	  mem [ q ] .hh.b1 = 0 ; 
	} 
	if ( mem [ q + 1 ] .hh .v.RH != 0 ) 
	{
	  s = mem [ q + 1 ] .hh .v.RH ; 
	  while ( mem [ s ] .hh .v.RH != 0 ) s = mem [ s ] .hh .v.RH ; 
	  mem [ s ] .hh .v.RH = r ; 
	  r = mem [ q + 1 ] .hh .v.RH ; 
	  mem [ q + 1 ] .hh .v.RH = 0 ; 
	  postdiscbreak = true ; 
	} 
	if ( mem [ q + 1 ] .hh .v.LH != 0 ) 
	{
	  s = mem [ q + 1 ] .hh .v.LH ; 
	  mem [ q ] .hh .v.RH = s ; 
	  while ( mem [ s ] .hh .v.RH != 0 ) s = mem [ s ] .hh .v.RH ; 
	  mem [ q + 1 ] .hh .v.LH = 0 ; 
	  q = s ; 
	} 
	mem [ q ] .hh .v.RH = r ; 
	discbreak = true ; 
      } 
      else if ( ( mem [ q ] .hh.b0 == 9 ) || ( mem [ q ] .hh.b0 == 11 ) ) 
      mem [ q + 1 ] .cint = 0 ; 
    } 
    else {
	
      q = memtop - 3 ; 
      while ( mem [ q ] .hh .v.RH != 0 ) q = mem [ q ] .hh .v.RH ; 
    } 
    r = newparamglue ( 8 ) ; 
    mem [ r ] .hh .v.RH = mem [ q ] .hh .v.RH ; 
    mem [ q ] .hh .v.RH = r ; 
    q = r ; 
    lab30: ; 
    r = mem [ q ] .hh .v.RH ; 
    mem [ q ] .hh .v.RH = 0 ; 
    q = mem [ memtop - 3 ] .hh .v.RH ; 
    mem [ memtop - 3 ] .hh .v.RH = r ; 
    if ( eqtb [ (hash_size + 789) ] .hh .v.RH != 0 ) 
    {
      r = newparamglue ( 7 ) ; 
      mem [ r ] .hh .v.RH = q ; 
      q = r ; 
    } 
    if ( curline > lastspecialline ) 
    {
      curwidth = secondwidth ; 
      curindent = secondindent ; 
    } 
    else if ( eqtb [ (hash_size + 1312) ] .hh .v.RH == 0 ) 
    {
      curwidth = firstwidth ; 
      curindent = firstindent ; 
    } 
    else {
	
      curwidth = mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH + 2 * curline ] .cint ; 
      curindent = mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH + 2 * curline - 1 ] .cint ; 
    } 
    adjusttail = memtop - 5 ; 
    justbox = hpack ( q , curwidth , 0 ) ; 
    mem [ justbox + 4 ] .cint = curindent ; 
    appendtovlist ( justbox ) ; 
/* if adjust_head<>adjust_tail then l.17346 */
    if ( memtop - 5 != adjusttail ) 
    {
      mem [ curlist .tailfield ] .hh .v.RH = mem [ memtop - 5 ] .hh .v.RH ; 
      curlist .tailfield = adjusttail ; 
    } 
    adjusttail = 0 ; /* adjust_tail:=null */
    if ( curline + 1 != bestline ) 
    {
      pen = eqtb [ (hash_size + 3176) ] .cint ; 
      if ( curline == curlist .pgfield + 1 ) 
      pen = pen + eqtb [ (hash_size + 3168) ] .cint ; 
      if ( curline + 2 == bestline ) 
      pen = pen + finalwidowpenalty ; 
      if ( discbreak ) 
      pen = pen + eqtb [ (hash_size + 3171) ] .cint ; 
      if ( pen != 0 ) 
      {
	r = newpenalty ( pen ) ; 
	mem [ curlist .tailfield ] .hh .v.RH = r ; 
	curlist .tailfield = r ; 
      } 
    } 
    incr ( curline ) ; 
    curp = mem [ curp + 1 ] .hh .v.LH ; 
    if ( curp != 0 ) 
    if ( ! postdiscbreak ) 
    {
      r = memtop - 3 ; 
      while ( true ) {
	q = mem [ r ] .hh .v.RH ; 
	if ( q == mem [ curp + 1 ] .hh .v.RH ) 
	goto lab31 ; 
	if ( ( q >= himemmin ) ) 
	goto lab31 ; 
	if ( ( mem [ q ] .hh.b0 < 9 ) ) 
	goto lab31 ; 
/* change in tests here from 3.141 != 1 instead of == 2 */
	if ( mem [ q ] .hh.b0 == 11 ) 
	if ( mem [ q ] .hh.b1 != 1 ) 
	goto lab31 ; 
	r = q ; 
      } 
      lab31: if ( r != memtop - 3 ) 
      {
	mem [ r ] .hh .v.RH = 0 ; 
	flushnodelist ( mem [ memtop - 3 ] .hh .v.RH ) ; 
	mem [ memtop - 3 ] .hh .v.RH = q ; 
      } 
    } 
  } while ( ! ( curp == 0 ) ) ; 
  if ( ( curline != bestline ) || ( mem [ memtop - 3 ] .hh .v.RH != 0 ) ) {
	  confusion ( 933 ) ;		/* disc2 */
	  return;				// abortflag set
  }
  curlist .pgfield = bestline - 1 ; 
} 

/* Reconstitute ligatures during hyphenation pass */

smallnumber zreconstitute ( j , n , bchar , hchar ) 
smallnumber j ; 
smallnumber n ; 
halfword bchar ; 
halfword hchar ; 
{/* 22 30 */ register smallnumber Result; reconstitute_regmem 
  halfword p  ; 
  halfword t  ; 
  ffourquarters q  ; 
  halfword currh  ; 
  halfword testchar  ; 
  scaled w  ; 
  fontindex k  ; 
  hyphenpassed = 0 ;						/* paragraph 907 ? */
  t = memtop - 4 ; 
  w = 0 ; 
  mem [ memtop - 4 ] .hh .v.RH = 0 ; 
  curl = hu [ j ] ; 
  curq = t ; 
  if ( j == 0 ) 
  {
    ligaturepresent = initlig ; 
    p = initlist ; 
    if ( ligaturepresent ) 
		lfthit = initlft ; 
/*   while p>null do l.17772 */
/*    while ( p > 0 ) { */	/* NO! */
    while ( p != 0 ) {					/* 94/Mar/22 BUG FIX */
      {
/*    append_charnode_to_t(character(p)); */ 
	mem [ t ] .hh .v.RH = getavail () ; 
	t = mem [ t ] .hh .v.RH ; 
	mem [ t ] .hh.b0 = hf ; 
	mem [ t ] .hh.b1 = mem [ p ] .hh.b1 ; 
      } 
      p = mem [ p ] .hh .v.RH ; /* p:=link(p); */
    } 
  } 
  else if ( curl < 256 ) 
  {
    mem [ t ] .hh .v.RH = getavail () ; 
    t = mem [ t ] .hh .v.RH ; 
    mem [ t ] .hh.b0 = hf ; 
    mem [ t ] .hh.b1 = curl ; 
  } 
  ligstack = 0 ;		/* lig_stack:=null; */
  {
    if ( j < n ) 
		curr = hu [ j + 1 ] ; 
    else curr = bchar ; 
    if ( odd ( hyf [ j ] ) ) 
		currh = hchar ; 
    else currh = 256 ; 
  } 
  lab22: if ( curl == 256 )		/* if cur_l = non_char then */
  {
    k = bcharlabel [ hf ] ;		/* begin k:=bchar_label[hf]; */
/*  if k=non_address then goto done@+else q:=font_info[k].qqqq; l.17812 */
/*    if ( k == fontmemsize )  */
    if ( k == non_address )  		/* i.e. 0 ---  96/Jan/15 */
    goto lab30 ; 
    else q = fontinfo [ k ] .qqqq ; 
  } 
  else {
    q = fontinfo [ charbase [ hf ] + curl ] .qqqq ; 
    if ( ( ( q .b2 ) % 4 ) != 1 ) 
    goto lab30 ; 
    k = ligkernbase [ hf ] + q .b3 ; 
    q = fontinfo [ k ] .qqqq ; 
    if ( q .b0 > 128 ) 
    {
      k = ligkernbase [ hf ] + 256 * q .b2 + q .b3 + 32768L - 256 * ( 128 ) ; 
      q = fontinfo [ k ] .qqqq ; 
    } 
  } 
  if ( currh < 256 )	/* if cur_rh < non_char then test_char:=cur_rh */
	  testchar = currh ; 
  else testchar = curr ;	/* else test_char:=cur_r; l.17817 */
/* loop@+begin if next_char(q)=test_char then if skip_byte(q)<=stop_flag then */
  while ( true ) {
    if ( q .b1 == testchar ) 
    if ( q .b0 <= 128 ) 
    if ( currh < 256 )		/*  if cur_rh<non_char then */
    {
      hyphenpassed = j ; 
      hchar = 256 ; 
      currh = 256 ; 
      goto lab22 ;			/* goto continue; */
    } 
    else {		/* else begin if hchar<non_char then if odd(hyf[j]) then */
	  if ( hchar < 256 ) 
      if ( odd ( hyf [ j ] ) ) 
      {
	hyphenpassed = j ; 
	hchar = 256 ; 
      } 
      if ( q .b2 < 128 )	/* if op_byte(q)<kern_flag then */
      {
// @<Carry out a ligature replacement, updating the cursor structure
//	and possibly advancing~|j|; |goto continue| if the cursor doesn't
//	advance, otherwise |goto done|@>;  => l.17869
	if ( curl == 256 )		/* begin if cur_l=non_char then lft_hit:=true; */
		lfthit = true ; 
	if ( j == n ) 
		if ( ligstack == 0 ) /* if lig_stack=null ? */
			rthit = true ; 
	{
	  if ( interrupt != 0 ) {
		  pauseforinstructions () ;
		  ABORTCHECKZERO;
	  }
	} 
	switch ( q .b2 )	/* case op_byte(q) of */
	{case 1 : 
	case 5 : 
	  {
	    curl = q .b3 ; 
	    ligaturepresent = true ; 
	  } 
	  break ; 
	case 2 : 
	case 6 : 
	  {
	    curr = q .b3 ; 
/*   if lig_stack>null then character(lig_stack):=cur_r */
/*	    if ( ligstack > 0 )  */			/* 94/Mar/22 */
	    if ( ligstack != 0 )			/* line breaking?? */
			mem [ ligstack ] .hh.b1 = curr ;	/* l.17877 ? */
	    else {
	      ligstack = newligitem ( curr ) ; 
	      if ( j == n ) 
	      bchar = 256 ; 
	      else {
		p = getavail () ; 
		mem [ ligstack + 1 ] .hh .v.RH = p ; 
		mem [ p ] .hh.b1 = hu [ j + 1 ] ; 
		mem [ p ] .hh.b0 = hf ; 
	      } 
	    } 
	  } 
	  break ; 
	case 3 : 
	  {
	    curr = q .b3 ; 
	    p = ligstack ; 
	    ligstack = newligitem ( curr ) ; 
	    mem [ ligstack ] .hh .v.RH = p ; 
	  } 
	  break ; 
	case 7 : 
	case 11 : 
	  {
	    if ( ligaturepresent ) 
	    {
	      p = newligature ( hf , curl , mem [ curq ] .hh .v.RH ) ; 
	      if ( lfthit ) 
	      {
		mem [ p ] .hh.b1 = 2 ; 
		lfthit = false ; 
	      } 
/*	      if ( false ) 
			  if ( ligstack == 0 ) {
				  incr ( mem [ p ] .hh.b1 ) ; 
				  rthit = false ; 
			  } */							/* removed 99/Jan/6 */
		  mem [ curq ] .hh .v.RH = p ; 
		  t = p ; 
	      ligaturepresent = false ; 
	    } 
	    curq = t ; 
	    curl = q .b3 ; 
	    ligaturepresent = true ; 
	  } 
	  break ; 
	  default: 
	  {
/* othercases begin cur_l:=rem_byte(q);
		ligature_present:=true; {\.{=:}} l.17869 */
	    curl = q .b3 ; 
	    ligaturepresent = true ; 
/*   if lig_stack>null then pop_lig_stack l.17870 => l.17828 */
/*	    if ( ligstack > 0 ) */				/* 94/Mar/22 ??? */
	    if ( ligstack != 0 )				/* BUG FIX  */
	    {
/*	      if ( mem [ ligstack + 1 ] .hh .v.RH > 0 )  */ /* 94/Mar/22  */
	      if ( mem [ ligstack + 1 ] .hh .v.RH != 0 )	/* l.17828 ? */
	      {
		mem [ t ] .hh .v.RH = mem [ ligstack + 1 ] .hh .v.RH ; 
		t = mem [ t ] .hh .v.RH ; 
		incr ( j ) ; 
	      } 
	      p = ligstack ; 
	      ligstack = mem [ p ] .hh .v.RH ; 
	      freenode ( p , 2 ) ; 
	      if ( ligstack == 0 )	/* if ligstack=null ? */
	      {
		if ( j < n ) 
		curr = hu [ j + 1 ] ; 
		else curr = bchar ; 
		if ( odd ( hyf [ j ] ) ) 
		currh = hchar ; 
		else currh = 256 ; 
	      } 
	      else curr = mem [ ligstack ] .hh.b1 ; 
	    } 
	    else if ( j == n ) 
	    goto lab30 ; 
	    else {
		
	      {
		mem [ t ] .hh .v.RH = getavail () ; 
		t = mem [ t ] .hh .v.RH ; 
		mem [ t ] .hh.b0 = hf ; 
		mem [ t ] .hh.b1 = curr ; 
	      } 
	      incr ( j ) ; 
	      {
		if ( j < n ) 
		curr = hu [ j + 1 ] ; 
		else curr = bchar ; 
		if ( odd ( hyf [ j ] ) ) 
		currh = hchar ; 
		else currh = 256 ; 
	      } 
	    } 
	  } 
	  break ; 
	} 
	if ( q .b2 > 4 ) 
	if ( q .b2 != 7 ) 
	goto lab30 ; 
	goto lab22 ; 
      } 
      w = fontinfo [ kernbase [ hf ] + 256 * q .b2 + q .b3 ] .cint ; 
      goto lab30 ; 
    } 
    if ( q .b0 >= 128 ) 
    if ( currh == 256 ) 
    goto lab30 ; 
    else {
	
      currh = 256 ; 
      goto lab22 ; 
    } 
    k = k + q .b0 + 1 ; 
    q = fontinfo [ k ] .qqqq ; 
  } 
  lab30: ; 
  if ( ligaturepresent ) 
  {
    p = newligature ( hf , curl , mem [ curq ] .hh .v.RH ) ; 
    if ( lfthit ) 
    {
      mem [ p ] .hh.b1 = 2 ; 
      lfthit = false ; 
    } 
    if ( rthit ) 
    if ( ligstack == 0 )	/* if ligstack=null ? */
    {
      incr ( mem [ p ] .hh.b1 ) ; 
      rthit = false ; 
    } 
    mem [ curq ] .hh .v.RH = p ; 
    t = p ; 
    ligaturepresent = false ; 
  } 
  if ( w != 0 ) 
  {
    mem [ t ] .hh .v.RH = newkern ( w ) ; 
    t = mem [ t ] .hh .v.RH ; 
    w = 0 ; 
  } 
/*  if ( ligstack > 0 ) */			/* 94/Mar/22 ??? l.17870 */
  if ( ligstack != 0 )				/* l.17841 */
  {
/* begin cur_q:=t; cur_l:=character(lig_stack);
	ligature_present:=true; l.17842 */
    curq = t ; 
    curl = mem [ ligstack ] .hh.b1 ; 
    ligaturepresent = true ; 
    {
/*   pop_lig_stack; goto continue; l.17843 => l.17828 */
/*      if ( mem [ ligstack + 1 ] .hh .v.RH > 0 )  *//* 94/Mar/22 */
      if ( mem [ ligstack + 1 ] .hh .v.RH != 0 ) 	/* BUG FIX */
      {
	mem [ t ] .hh .v.RH = mem [ ligstack + 1 ] .hh .v.RH ; 
	t = mem [ t ] .hh .v.RH ; 
	incr ( j ) ; 
      } 
      p = ligstack ; 
      ligstack = mem [ p ] .hh .v.RH ; 
      freenode ( p , 2 ) ; 
      if ( ligstack == 0 ) 
      {
	if ( j < n ) 
	curr = hu [ j + 1 ] ; 
	else curr = bchar ; 
	if ( odd ( hyf [ j ] ) ) 
	currh = hchar ; 
	else currh = 256 ; 
      } 
      else curr = mem [ ligstack ] .hh.b1 ; 
    } 
    goto lab22 ; 
  } 
  Result = j ; 
  return Result ; 
}

/* #pragma optimize ("g", off) */ /* not needed for MSVC it seems ... */

void hyphenate ( ) 
{/* 50 30 40 41 42 45 10 */ hyphenate_regmem 
/*  char i, j, l  ;  */
  char i, j  ; 
  int l  ;							/* 95/Jan/7 */
  halfword q, r, s  ; 
  halfword bchar  ; 
  halfword majortail, minortail  ; 
/*  ASCIIcode c  ;  */
  int c  ;							/* 95/Jan/7 */
  char cloc  ; 
/*  integer rcount  ; */
  int rcount  ;						/* 95/Jan/7 */
  halfword hyfnode  ; 
  triepointer z  ; 
  integer v  ; 
  hyphpointer h  ; 
  strnumber k  ; 
  poolpointer u  ; 
  {
	  register integer for_end; 
	  j = 0 ; 
	  for_end = hn ; 
	  if ( j <= for_end) do 
		  hyf [ j ] = 0 ; 
	  while ( j++ < for_end ) ;
  } 
  h = hc [ 1 ] ; 
  incr ( hn ) ; 
  hc [ hn ] = curlang ; 
  {
	  register integer for_end; 
	  j = 2 ; 
	  for_end = hn ; 
	  if ( j <= for_end) do 
/*    h = ( h + h + hc [ j ] ) % 607 ;  */
		  h = ( h + h + hc [ j ] ) % hyphen_prime ; 
	  while ( j++ < for_end ) ;
  } 
  while ( true ) {
    k = hyphword [ h ] ; 
    if ( k == 0 ) 
    goto lab45 ; 
    if ( ( strstart [ k + 1 ] - strstart [ k ] ) < hn ) 
    goto lab45 ; 
    if ( ( strstart [ k + 1 ] - strstart [ k ] ) == hn ) 
    {
      j = 1 ; 
      u = strstart [ k ] ; 
      do {
	  if ( strpool [ u ] < hc [ j ] ) 
	goto lab45 ; 
	if ( strpool [ u ] > hc [ j ] ) 
	goto lab30 ; 
	incr ( j ) ; 
	incr ( u ) ; 
      } while ( ! ( j > hn ) ) ; 
      s = hyphlist [ h ] ; 
      while ( s != 0 ) {	/* while s<>null do l.18173 */
	  
	hyf [ mem [ s ] .hh .v.LH ] = 1 ; 
	s = mem [ s ] .hh .v.RH ; 
      } 
      decr ( hn ) ; 
      goto lab40 ; 
    } 
    lab30: ; 
    if ( h > 0 ) 
    decr ( h ) ; 
/*    else h = 607 ;  */
    else h = hyphen_prime ; 
  } 
  lab45: decr ( hn ) ; 
  if ( trietrc [ curlang + 1 ] != curlang ) 
  return ; 
  hc [ 0 ] = 0 ; 
  hc [ hn + 1 ] = 0 ; 
  hc [ hn + 2 ] = 256 ; 
  {
	  register integer for_end; 
	  j = 0 ; 
	  for_end = hn - rhyf + 1 ; 
	  if ( j <= for_end) do 
	  {
		  z = trietrl [ curlang + 1 ] + hc [ j ] ; 
		  l = j ; 
		  while ( hc [ l ] == trietrc [ z ] ) {
	  	if ( trietro [ z ] != mintrieop ) 
	{
	  v = trietro [ z ] ; 
	  do {
	      v = v + opstart [ curlang ] ; 
	    i = l - hyfdistance [ v ] ; 
	    if ( hyfnum [ v ] > hyf [ i ] ) 
	    hyf [ i ] = hyfnum [ v ] ; 
	    v = hyfnext [ v ] ; 
	  } while ( ! ( v == mintrieop ) ) ; 
	} 
	incr ( l ) ; 
	z = trietrl [ z ] + hc [ l ] ; 
      } 
    } 
  while ( j++ < for_end ) ; } 
  lab40: {
      register integer for_end; 
	  j = 0 ; 
	  for_end = lhyf - 1 ; 
	  if ( j <= for_end) do 
		  hyf [ j ] = 0 ; 
	  while ( j++ < for_end ) ;
		 } 
  {
	  register integer for_end; 
	  j = 0 ; 
	  for_end = rhyf - 1 ; 
	  if ( j <= for_end) do 
		  hyf [ hn - j ] = 0 ; 
	  while ( j++ < for_end ) ;
  } 
/* was j = 0 ; for_end = rhyf - 1; in 3.141 */
  {
	  register integer for_end; 
	  j = lhyf ; 
	  for_end = hn - rhyf ; 
	  if ( j <= for_end) do 
/* lost if (j <= for_end) do since 3.141 */
		  if ( odd ( hyf [ j ] ) ) 
			  goto lab41 ; 
	  while ( j++ < for_end ) ;
  } 
  return ; 
  lab41: ; 
  q = mem [ hb ] .hh .v.RH ; 
  mem [ hb ] .hh .v.RH = 0 ; 
  r = mem [ ha ] .hh .v.RH ; 
  mem [ ha ] .hh .v.RH = 0 ; 
  bchar = hyfbchar ; 
  if ( ( ha >= himemmin ) ) 
  if ( mem [ ha ] .hh.b0 != hf ) 
  goto lab42 ; 
  else {
      
    initlist = ha ; 
    initlig = false ; 
    hu [ 0 ] = mem [ ha ] .hh.b1 ; 
  } 
  else if ( mem [ ha ] .hh.b0 == 6 ) 
  if ( mem [ ha + 1 ] .hh.b0 != hf ) 
  goto lab42 ; 
  else {
      
    initlist = mem [ ha + 1 ] .hh .v.RH ; 
    initlig = true ; 
    initlft = ( mem [ ha ] .hh.b1 > 1 ) ; 
    hu [ 0 ] = mem [ ha + 1 ] .hh.b1 ; 
    if ( initlist == 0 ) 
    if ( initlft ) 
    {
      hu [ 0 ] = 256 ; 
      initlig = false ; 
    } 
    freenode ( ha , 2 ) ; 
  } 
  else {
      
    if ( ! ( r >= himemmin ) ) 
    if ( mem [ r ] .hh.b0 == 6 ) 
    if ( mem [ r ] .hh.b1 > 1 ) 
    goto lab42 ; 
    j = 1 ; 
    s = ha ; 
    initlist = 0 ; 
    goto lab50 ; 
  } 
  s = curp ; 
  while ( mem [ s ] .hh .v.RH != ha ) s = mem [ s ] .hh .v.RH ; 
  j = 0 ; 
  goto lab50 ; 
  lab42: s = ha ; 
  j = 0 ; 
  hu [ 0 ] = 256 ; 
  initlig = false ; 
  initlist = 0 ; 
  lab50: flushnodelist ( r ) ; 
  do {
      l = j ; 
    j = reconstitute ( j , hn , bchar , hyfchar ) + 1 ; 
    if ( hyphenpassed == 0 ) 
    {
      mem [ s ] .hh .v.RH = mem [ memtop - 4 ] .hh .v.RH ; 
/*      while ( mem [ s ] .hh .v.RH > 0 ) */	/* 94/Mar/22 */
      while ( mem [ s ] .hh .v.RH != 0 )		/* l.17903 */
		  s = mem [ s ] .hh .v.RH ; 
      if ( odd ( hyf [ j - 1 ] ) ) 
      {
	l = j ; 
	hyphenpassed = j - 1 ; 
	mem [ memtop - 4 ] .hh .v.RH = 0 ; /* link(hold_head):=null; */
      } 
    } 
    if ( hyphenpassed > 0 ) 
    do {
	r = getnode ( 2 ) ; 
      mem [ r ] .hh .v.RH = mem [ memtop - 4 ] .hh .v.RH ; 
      mem [ r ] .hh.b0 = 7 ; 
      majortail = r ; 
      rcount = 0 ; 
/* while link(major_tail)>null do advance_major_tail; l.17929 */
/*      while ( mem [ majortail ] .hh .v.RH > 0 ) { */	/* 94/Mar/22 */
      while ( mem [ majortail ] .hh .v.RH != 0 ) {		/* ??? */
	  
	majortail = mem [ majortail ] .hh .v.RH ; 
	incr ( rcount ) ; 
      } 
      i = hyphenpassed ; 
      hyf [ i ] = 0 ; 
/* minor_tail:=null; pre_break(r):=null; l.17943 */
      minortail = 0 ; 
      mem [ r + 1 ] .hh .v.LH = 0 ; 
/* hyf_node:=new_character(hf,hyf_char); */
      hyfnode = newcharacter ( hf , hyfchar ) ; 
      if ( hyfnode != 0 )	/* if hyf_node<>null then */
      {
	incr ( i ) ; 
	c = hu [ i ] ; 
	hu [ i ] = hyfchar ; 
	{
	  mem [ hyfnode ] .hh .v.RH = avail ; 
	  avail = hyfnode ; 
	;
#ifdef STAT
	  decr ( dynused ) ; 
#endif /* STAT */
	} 
      } 
      while ( l <= i ) {
/*  begin l:=reconstitute(l,i,font_bchar[hf],non_char)+1; l.17948 */
	l = reconstitute ( l , i , fontbchar [ hf ] , 256 ) + 1 ; 
/*  if link(hold_head)>null then l.17949 */
/*	if ( mem [ memtop - 4 ] .hh .v.RH > 0 ) */	/* 94/Mar/22 */
	if ( mem [ memtop - 4 ] .hh .v.RH != 0 )	/* BUG FIX ??? */
	{
	  if ( minortail == 0 ) /*      if minor_tail=null then */
	  mem [ r + 1 ] .hh .v.LH = mem [ memtop - 4 ] .hh .v.RH ; 
	  else mem [ minortail ] .hh .v.RH = mem [ memtop - 4 ] .hh .v.RH ; 
	  minortail = mem [ memtop - 4 ] .hh .v.RH ; 
/*    while link(minor_tail)>null do minor_tail:=link(minor_tail); l.17953 */
/*	  while ( mem [ minortail ] .hh .v.RH > 0 ) */ /* 94/Mar/22 */
	  while ( mem [ minortail ] .hh .v.RH != 0 )	/* BUG FIX */
		  minortail = mem [ minortail ] .hh .v.RH ; 
	} 
      } 
      if ( hyfnode != 0 )  /* if hyf_node<>null then l.17956 */
      {
	hu [ i ] = c ; 
	l = i ; 
	decr ( i ) ; 
      } 
/* minor_tail:=null; post_break(r):=null; c_loc:=0; l.17964 */
      minortail = 0 ; 
      mem [ r + 1 ] .hh .v.RH = 0 ; 
      cloc = 0 ; 
/* if bchar_label[hf]<non_address then l.17991 3.1415 */
/*      if ( bcharlabel [ hf ] < fontmemsize )  */
/* if bchar_label[hf]<>non_address then l.17991 3.14159 */
      if ( bcharlabel [ hf ] != non_address )	/* i.e. 0 --- 96/Jan/15 */
      {
	decr ( l ) ; 
	c = hu [ l ] ; 
	cloc = l ; 
	hu [ l ] = 256 ; 
      } 
      while ( l < j ) {
	  
	do {
	    l = reconstitute ( l , hn , bchar , 256 ) + 1 ; 
	  if ( cloc > 0 ) 
	  {
	    hu [ cloc ] = c ;		/* c may be used ... */
	    cloc = 0 ; 
	  } 
/*   if link(hold_head)>null then l. l.17973 */
/*	  if ( mem [ memtop - 4 ] .hh .v.RH > 0 )	*/		/* 94/Mar/22 ??? */
	  if ( mem [ memtop - 4 ] .hh .v.RH != 0 )			/* BUG FIX */
	  {
	    if ( minortail == 0 ) /*     begin if minor_tail=null then */
	    mem [ r + 1 ] .hh .v.RH = mem [ memtop - 4 ] .hh .v.RH ; 
	    else mem [ minortail ] .hh .v.RH = mem [ memtop - 4 ] .hh .v.RH ; 
	    minortail = mem [ memtop - 4 ] .hh .v.RH ; 
/*     while link(minor_tail)>null do minor_tail:=link(minor_tail); l.17977 */
/*	    while ( mem [ minortail ] .hh .v.RH > 0 ) */	/* 94/Mar/22 */
	    while ( mem [ minortail ] .hh .v.RH != 0 )		/* ??? */
			minortail = mem [ minortail ] .hh .v.RH ; 
	  } 
	} while ( ! ( l >= j ) ) ; 
	while ( l > j ) {
	    
	  j = reconstitute ( j , hn , bchar , 256 ) + 1 ; 
	  mem [ majortail ] .hh .v.RH = mem [ memtop - 4 ] .hh .v.RH ; 
/* while link(major_tail)>null do advance_major_tail; l.17987 */
/*	  while ( mem [ majortail ] .hh .v.RH > 0 ) { */	/* 94/Mar/22 */
	  while ( mem [ majortail ] .hh .v.RH != 0 ) {		/* ??? */
	    majortail = mem [ majortail ] .hh .v.RH ; 
	    incr ( rcount ) ; 
	  } 
	} 
      } 
      if ( rcount > 127 ) 
      {
	mem [ s ] .hh .v.RH = mem [ r ] .hh .v.RH ; 
	mem [ r ] .hh .v.RH = 0 ;	/* link(r):=null */
	flushnodelist ( r ) ; 
      } 
      else {
	  
	mem [ s ] .hh .v.RH = r ; 
	mem [ r ] .hh.b1 = rcount ; 
      } 
      s = majortail ; 
      hyphenpassed = j - 1 ; 
      mem [ memtop - 4 ] .hh .v.RH = 0 ;  /* link(hold_head):=null; */
    } while ( ! ( ! odd ( hyf [ j - 1 ] ) ) ) ; 
  } while ( ! ( j > hn ) ) ; 
  mem [ s ] .hh .v.RH = q ; 
  flushlist ( initlist ) ; 
} 

/* #pragma optimize ("g", off) */ /* not needed for MSVC it seems ... */

/* used only in itex.c */

void newhyphexceptions ( ) 
{/* 21 10 40 45 */ newhyphexceptions_regmem 
/*  smallnumber n  ;  */ /* in 3.141 */
  char n  ; 
/*  smallnumber j  ;  */ /* in 3.141 */
  char j  ; 
  hyphpointer h  ; 
  strnumber k  ; 
  halfword p  ; 
  halfword q  ; 
  strnumber s, t  ; 
  poolpointer u, v  ; 
  scanleftbrace () ; 
  ABORTCHECK;
  if ( eqtb [ (hash_size + 3213) ] .cint <= 0 ) 
	  curlang = 0 ; 
  else if ( eqtb [ (hash_size + 3213) ] .cint > 255 ) 
	  curlang = 0 ; 
  else curlang = eqtb [ (hash_size + 3213) ] .cint ; 
  n = 0 ; 
  p = 0 ; 

  while ( true ) {
    getxtoken () ; 
	ABORTCHECK;

lab21: switch ( curcmd ) {
    case 11 : 
    case 12 : 
    case 68 : 
      if ( curchr == 45 ) 
      {
	if ( n < 63 ) 
	{
	  q = getavail () ; 
	  mem [ q ] .hh .v.RH = p ; 
	  mem [ q ] .hh .v.LH = n ; 
	  p = q ; 
	} 
      } 
      else {
	  
	if ( eqtb [ (hash_size + 2139) + curchr ] .hh .v.RH == 0 ) 
	{
	  {
	    if ( interaction == 3 ) 
	    ; 
	    printnl ( 262 ) ;	/* !  */
	    print ( 939 ) ;		/* Not a letter */
	  } 
	  {
	    helpptr = 2 ; 
	    helpline [ 1 ] = 940 ;	/* Letters in \hyphenation words must have \lccode>0. */
	    helpline [ 0 ] = 941 ;	/* Proceed; I'll ignore the character I just read. */
	  } 
	  error () ; 
	  ABORTCHECK;
	} 
	else if ( n < 63 ) 
	{
	  incr ( n ) ; 
	  hc [ n ] = eqtb [ (hash_size + 2139) + curchr ] .hh .v.RH ; 
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
    case 10 : 
    case 2 : 
      {
	if ( n > 1 ) 
	{
	  incr ( n ) ; 
	  hc [ n ] = curlang ; 
	  {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
	    if ( poolptr + n > currentpoolsize ) 
/*			strpool = reallocstrpool (incrementpoolsize); */
			strpool = reallocstrpool (incrementpoolsize + n);
	    if ( poolptr + n > currentpoolsize ) { /* in case it failed 94/Jan/24 */
			overflow ( 257 , currentpoolsize - initpoolptr ) ; /* 97/Mar/7 */
			return;			// abortflag set
		}
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	    if ( poolptr + n > poolsize ) {
			overflow ( 257 , poolsize - initpoolptr ) ; /* pool size */
			return;			// abortflag set
		}
#endif
	  } 
	  h = 0 ; 
	  {
		  register integer for_end; 
		  j = 1 ; 
		  for_end = n ; 
		  if ( j <= for_end) do 
		  {
/*			h = ( h + h + hc [ j ] ) % 607 ;  */
			  h = ( h + h + hc [ j ] ) % hyphen_prime ; 
			  {
				  strpool [ poolptr ] = hc [ j ] ; 
				  incr ( poolptr ) ; 
			  } 
		  } 
		  while ( j++ < for_end ) ;
	  } 
	  s = makestring () ; 
/*	  if ( hyphcount == 607 ) */
	  if ( hyphcount == hyphen_prime ) {
/*		  overflow ( 942 , 607 ) ;  */	
		  overflow ( 942 , hyphen_prime ) ; /* exception dictionary - NOT DYNAMIC */
/*	  not dynamic ---- but can be set -e=... from command line in ini-TeX */
		  return;			// abortflag set
	  }
	  incr ( hyphcount ) ; 
	  while ( hyphword [ h ] != 0 ) {
	      
	    k = hyphword [ h ] ; 
	    if ( ( strstart [ k + 1 ] - strstart [ k ] ) < ( strstart [ s + 1 
	    ] - strstart [ s ] ) ) 
	    goto lab40 ; 
	    if ( ( strstart [ k + 1 ] - strstart [ k ] ) > ( strstart [ s + 1 
	    ] - strstart [ s ] ) ) 
	    goto lab45 ; 
	    u = strstart [ k ] ; 
	    v = strstart [ s ] ; 
	    do {
		if ( strpool [ u ] < strpool [ v ] ) 
	      goto lab40 ; 
	      if ( strpool [ u ] > strpool [ v ] ) 
	      goto lab45 ; 
	      incr ( u ) ; 
	      incr ( v ) ; 
	    } while ( ! ( u == strstart [ k + 1 ] ) ) ; 
	    lab40: q = hyphlist [ h ] ; 
	    hyphlist [ h ] = p ; 
	    p = q ; 
	    t = hyphword [ h ] ; 
	    hyphword [ h ] = s ; 
	    s = t ; 
	    lab45: ; 
	    if ( h > 0 ) 
	    decr ( h ) ; 
/*	    else h = 607 ;  */
	    else h = hyphen_prime ; 
	  } 
	  hyphword [ h ] = s ; 
	  hyphlist [ h ] = p ; 
	} 
	if ( curcmd == 2 ) 
	return ; 
	n = 0 ; 
	p = 0 ; 
      } 
      break ; 
      default: 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;	/* !  */
	  print ( 677 ) ;	/* Improper  */
	} 
	printesc ( 935 ) ;	/* hyphenation */
	print ( 936 ) ;		/* will be flushed */
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 937 ;	/* Hyphenation exceptions must contain only letters */
	  helpline [ 0 ] = 938 ;	/* and hyphens. But continue; I'll forgive and forget. */
	} 
	error () ; 
	ABORTCHECK;
	  }
      break ; 
	} /* end of switch */
  }
} 

halfword zprunepagetop ( p ) 
halfword p ; 
{register halfword Result; prunepagetop_regmem 
  halfword prevp  ; 
  halfword q  ; 
  prevp = memtop - 3 ; 
  mem [ memtop - 3 ] .hh .v.RH = p ; 
/* while p<>null do l.18803 */
  while ( p != 0 ) switch ( mem [ p ] .hh.b0 ) 
  {case 0 : 
  case 1 : 
  case 2 : 
    {
      q = newskipparam ( 10 ) ; 
      mem [ prevp ] .hh .v.RH = q ; 
      mem [ q ] .hh .v.RH = p ; 
      if ( mem [ tempptr + 1 ] .cint > mem [ p + 3 ] .cint ) 
      mem [ tempptr + 1 ] .cint = mem [ tempptr + 1 ] .cint - mem [ p + 3 ] 
      .cint ; 
      else mem [ tempptr + 1 ] .cint = 0 ; 
      p = 0 ;	/* p:=null */
    } 
    break ; 
  case 8 : 
  case 4 : 
  case 3 : 
    {
      prevp = p ; 
      p = mem [ prevp ] .hh .v.RH ; 
    } 
    break ; 
  case 10 : 
  case 11 : 
  case 12 : 
    {
      q = p ; 
      p = mem [ q ] .hh .v.RH ; 
      mem [ q ] .hh .v.RH = 0 ; 
      mem [ prevp ] .hh .v.RH = p ; 
      flushnodelist ( q ) ; 
    } 
    break ; 
    default: 
		{
			confusion ( 953 ) ;		/* pruning */
			return 0;				// abortflag set
		}
		break ; 
  } 
  Result = mem [ memtop - 3 ] .hh .v.RH ; 
  return Result ; 
} 

halfword zvertbreak ( p , h , d ) 
halfword p ; 
scaled h ; 
scaled d ; 
{/* 30 45 90 */ register halfword Result; vertbreak_regmem 
  halfword prevp  ; 
  halfword q, r  ; 
  integer pi  ; 
  integer b  ; 
  integer leastcost  ; 
  halfword bestplace  ; 
  scaled prevdp  ; 
/*  smallnumber t  ;  */
  int t  ;							/* 95/Jan/7 */
  prevp = p ; 
  leastcost = 1073741823L ;  /* 2^30 - 1 */
  activewidth [ 1 ] = 0 ; 
  activewidth [ 2 ] = 0 ; 
  activewidth [ 3 ] = 0 ; 
  activewidth [ 4 ] = 0 ; 
  activewidth [ 5 ] = 0 ; 
  activewidth [ 6 ] = 0 ; 
  prevdp = 0 ; 
  while ( true ) {
    if ( p == 0 )	/* if p=null l.18879 */
    pi = -10000 ; 
    else switch ( mem [ p ] .hh.b0 ) 
    {case 0 : 
    case 1 : 
    case 2 : 
      {
	activewidth [ 1 ] = activewidth [ 1 ] + prevdp + mem [ p + 3 ] .cint ; 
	prevdp = mem [ p + 2 ] .cint ; 
	goto lab45 ; 
      } 
      break ; 
    case 8 : 
      goto lab45 ; 
      break ; 
    case 10 : 
      if ( ( mem [ prevp ] .hh.b0 < 9 ) ) 
      pi = 0 ; 
      else goto lab90 ; 
      break ; 
    case 11 : 
      {
	if ( mem [ p ] .hh .v.RH == 0 ) /* if link(p)=null l.18903 */
	t = 12 ; 
	else t = mem [ mem [ p ] .hh .v.RH ] .hh.b0 ; 
	if ( t == 10 ) 
	pi = 0 ; 
	else goto lab90 ; 
      } 
      break ; 
    case 12 : 
      pi = mem [ p + 1 ] .cint ; 
      break ; 
    case 4 : 
    case 3 : 
      goto lab45 ; 
      break ; 
      default: 
		  {
			  confusion ( 954 ) ;	/* vertbreak */
			  return 0;				// abortflag set
		  }
		  break ; 
    } 
    if ( pi < 10000 )			/* pi may be used ... */
    {
      if ( activewidth [ 1 ] < h ) 
      if ( ( activewidth [ 3 ] != 0 ) || ( activewidth [ 4 ] != 0 ) || ( 
      activewidth [ 5 ] != 0 ) ) 
      b = 0 ; 
      else b = badness ( h - activewidth [ 1 ] , activewidth [ 2 ] ) ; 
      else if ( activewidth [ 1 ] - h > activewidth [ 6 ] ) 
      b = 1073741823L ;  /* 2^30 - 1 */
      else b = badness ( activewidth [ 1 ] - h , activewidth [ 6 ] ) ; 
      if ( b < 1073741823L )  /* 2^30 - 1 */
      if ( pi <= -10000 ) 
      b = pi ; 
      else if ( b < 10000 ) 
      b = b + pi ; 
      else b = 100000L ; 
      if ( b <= leastcost ) 
      {
	bestplace = p ; 
	leastcost = b ; 
	bestheightplusdepth = activewidth [ 1 ] + prevdp ; 
      } 
      if ( ( b == 1073741823L ) || ( pi <= -10000 ) )  /* 2^30 - 1 */
      goto lab30 ; 
    } 
    if ( ( mem [ p ] .hh.b0 < 10 ) || ( mem [ p ] .hh.b0 > 11 ) ) 
    goto lab45 ; 
    lab90: if ( mem [ p ] .hh.b0 == 11 ) 
    q = p ; 
    else {
	
      q = mem [ p + 1 ] .hh .v.LH ; 
      activewidth [ 2 + mem [ q ] .hh.b0 ] = activewidth [ 2 + mem [ q ] 
      .hh.b0 ] + mem [ q + 2 ] .cint ; 
      activewidth [ 6 ] = activewidth [ 6 ] + mem [ q + 3 ] .cint ; 
      if ( ( mem [ q ] .hh.b1 != 0 ) && ( mem [ q + 3 ] .cint != 0 ) ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 955 ) ;	/* Infinite glue shrinkage found in box being split */
	} 
	{
	  helpptr = 4 ; 
	  helpline [ 3 ] = 956 ;	/* The box you are \vsplitting contains some infinitely */
	  helpline [ 2 ] = 957 ;	/* shrinkable glue, e.g., `\vss' or `\vskip 0pt minus 1fil'. */
	  helpline [ 1 ] = 958 ;	/* Such glue doesn't belong there; but you can safely proceed, */
	  helpline [ 0 ] = 916 ;	/* since the offensive shrinkability has been made finite. */
	} 
	error () ; 
	ABORTCHECKZERO;
	r = newspec ( q ) ; 
	mem [ r ] .hh.b1 = 0 ; 
	deleteglueref ( q ) ; 
	mem [ p + 1 ] .hh .v.LH = r ; 
	q = r ; 
      } 
    } 
    activewidth [ 1 ] = activewidth [ 1 ] + prevdp + mem [ q + 1 ] .cint ; 
    prevdp = 0 ; 
    lab45: if ( prevdp > d ) 
    {
      activewidth [ 1 ] = activewidth [ 1 ] + prevdp - d ; 
      prevdp = d ; 
    } 
    prevp = p ; 
    p = mem [ prevp ] .hh .v.RH ; 
  } 
  lab30: Result = bestplace ;	/* bestplace may be used ... */
  return Result ; 
} 

/* called only from tex7.c */

halfword zvsplit ( n , h ) 
eightbits n ; 
scaled h ; 
{/* 10 30 */ register halfword Result; vsplit_regmem 
  halfword v  ; 
  halfword p  ; 
  halfword q  ; 
  v = eqtb [ (hash_size + 1578) + n ] .hh .v.RH ; 
  if ( curmark [ 3 ] != 0 ) 
  {
    deletetokenref ( curmark [ 3 ] ) ; 
    curmark [ 3 ] = 0 ; 
    deletetokenref ( curmark [ 4 ] ) ; 
    curmark [ 4 ] = 0 ; 
  } 
  if ( v == 0 )		/* if v=null then l.18999 */
  {
    Result = 0 ;	/*   begin vsplit:=null; return; */
    return(Result) ; 
  } 
  if ( mem [ v ] .hh.b0 != 1 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 335 ) ;	/*  */

    } 
    printesc ( 959 ) ;	/* vsplit */
    print ( 960 ) ;		/* needs a  */
    printesc ( 961 ) ;	/* vbox */
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 962 ;	/* The box you are trying to split is an \hbox. */
      helpline [ 0 ] = 963 ;	/* I can't split such a box, so I'll leave it alone. */
    } 
    error () ; 
	ABORTCHECKZERO;
    Result = 0 ; 
    return(Result) ; 
  } 
  q = vertbreak ( mem [ v + 5 ] .hh .v.RH , h , eqtb [ (hash_size + 3736) ] .cint ) ; 
  ABORTCHECKZERO;
  p = mem [ v + 5 ] .hh .v.RH ; 
  if ( p == q ) 
  mem [ v + 5 ] .hh .v.RH = 0 ; 
  else while ( true ) {
    if ( mem [ p ] .hh.b0 == 4 ) 
    if ( curmark [ 3 ] == 0 ) 
    {
      curmark [ 3 ] = mem [ p + 1 ] .cint ; 
      curmark [ 4 ] = curmark [ 3 ] ; 
      mem [ curmark [ 3 ] ] .hh .v.LH = mem [ curmark [ 3 ] ] .hh .v.LH + 2 ; 
    } 
    else {
      deletetokenref ( curmark [ 4 ] ) ; 
      curmark [ 4 ] = mem [ p + 1 ] .cint ; 
      incr ( mem [ curmark [ 4 ] ] .hh .v.LH ) ; 
    } 
    if ( mem [ p ] .hh .v.RH == q ) 
    {
      mem [ p ] .hh .v.RH = 0 ; 
      goto lab30 ; 
    } 
    p = mem [ p ] .hh .v.RH ; 
  } 
  lab30: ; 
  q = prunepagetop ( q ) ; 
  p = mem [ v + 5 ] .hh .v.RH ; 
  freenode ( v , 7 ) ; 
  if ( q == 0 )		/* if q=null l.18993 */
  eqtb [ (hash_size + 1578) + n ] .hh .v.RH = 0 ;  /* then box(n):=null */
  else eqtb [ (hash_size + 1578) + n ] .hh .v.RH =
	  vpackage ( q , 0 , 1 , 1073741823L ) ;  /* 2^30 - 1 */
  Result = vpackage ( p , h , 0 , eqtb [ (hash_size + 3736) ] .cint ) ; 
  return Result ; 
} 

void printtotals ( ) 
{printtotals_regmem 
  printscaled ( pagesofar [ 1 ] ) ; 
  if ( pagesofar [ 2 ] != 0 ) 
  {
    print ( 310 ) ;		/*  plus  */
    printscaled ( pagesofar [ 2 ] ) ; 
    print ( 335 ) ;		/*  */
  } 
  if ( pagesofar [ 3 ] != 0 ) 
  {
    print ( 310 ) ;		/*  plus  */
    printscaled ( pagesofar [ 3 ] ) ; 
    print ( 309 ) ;		/* fil */
  } 
  if ( pagesofar [ 4 ] != 0 ) 
  {
    print ( 310 ) ;		/*  plus  */
    printscaled ( pagesofar [ 4 ] ) ; 
    print ( 972 ) ;		/* fill */
  } 
  if ( pagesofar [ 5 ] != 0 ) 
  {
    print ( 310 ) ;		/* plus */
    printscaled ( pagesofar [ 5 ] ) ; 
    print ( 973 ) ;		/* filll */
  } 
  if ( pagesofar [ 6 ] != 0 ) 
  {
    print ( 311 ) ;		/*  minus  */
    printscaled ( pagesofar [ 6 ] ) ; 
  } 
} 
void zfreezepagespecs ( s ) 
smallnumber s ; 
{freezepagespecs_regmem 
  pagecontents = s ; 
  pagesofar [ 0 ] = eqtb [ (hash_size + 3734) ] .cint ; 
  pagemaxdepth = eqtb [ (hash_size + 3735) ] .cint ; 
  pagesofar [ 7 ] = 0 ; 
  pagesofar [ 1 ] = 0 ; 
  pagesofar [ 2 ] = 0 ; 
  pagesofar [ 3 ] = 0 ; 
  pagesofar [ 4 ] = 0 ; 
  pagesofar [ 5 ] = 0 ; 
  pagesofar [ 6 ] = 0 ; 
  leastpagecost = 1073741823L ;  /* 2^30 - 1 */
	;
#ifdef STAT
  if ( eqtb [ (hash_size + 3196) ] .cint > 0 ) 
  {
    begindiagnostic () ; 
    printnl ( 981 ) ;	/* might split */
    printscaled ( pagesofar [ 0 ] ) ; 
    print ( 982 ) ;		/* , max depth= */
    printscaled ( pagemaxdepth ) ; 
    enddiagnostic ( false ) ; 
  } 
#endif /* STAT */
} 

void zboxerror ( n ) 
eightbits n ; 
{boxerror_regmem 
    error () ; 
	ABORTCHECK;
	begindiagnostic () ; 
	printnl ( 830 ) ;	/* The following box has been deleted: */
	showbox ( eqtb [ (hash_size + 1578) + n ] .hh .v.RH ) ; 
	enddiagnostic ( true ) ; 
	flushnodelist ( eqtb [ (hash_size + 1578) + n ] .hh .v.RH ) ; 
	eqtb [ (hash_size + 1578) + n ] .hh .v.RH = 0 ; 
} 

void zensurevbox ( n ) 
eightbits n ; 
{ensurevbox_regmem 
  halfword p  ; 
  p = eqtb [ (hash_size + 1578) + n ] .hh .v.RH ; 
  if ( p != 0 )  /* if p<>null then if type(p)=hlist_node then l.19324 */
  if ( mem [ p ] .hh.b0 == 0 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 983 ) ;		/* Insertions can only be added to a vbox */
    } 
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 984 ;	/* Tut tut: You're trying to \insert into a */
      helpline [ 1 ] = 985 ;	/* \box register that now contains an \hbox. */
      helpline [ 0 ] = 986 ;	/* Proceed, and I'll discard its present contents. */
    } 
    boxerror ( n ) ; 
//	ABORTCHECK;
  } 
} 

/* called only from tex7.c */

void zfireup ( c ) 
halfword c ; 
{/* 10 */ fireup_regmem 
  halfword p, q, r, s  ; 
  halfword prevp  ; 
/*  unsigned char n  ;  */
  unsigned int n  ;					/* 95/Jan/7 */
  booleane wait  ; 
  integer savevbadness  ; 
  scaled savevfuzz  ; 
  halfword savesplittopskip  ; 
  if ( mem [ bestpagebreak ] .hh.b0 == 12 ) 
  {
    geqworddefine ( (hash_size + 3202) , mem [ bestpagebreak + 1 ] .cint ) ; 
    mem [ bestpagebreak + 1 ] .cint = 10000 ; 
  } 
  else geqworddefine ( (hash_size + 3202) , 10000 ) ; 
  if ( curmark [ 2 ] != 0 ) 
  {
    if ( curmark [ 0 ] != 0 ) 
    deletetokenref ( curmark [ 0 ] ) ; 
    curmark [ 0 ] = curmark [ 2 ] ; 
    incr ( mem [ curmark [ 0 ] ] .hh .v.LH ) ; 
    deletetokenref ( curmark [ 1 ] ) ; 
    curmark [ 1 ] = 0 ; 
  } 
  if ( c == bestpagebreak ) 
  bestpagebreak = 0 ; 
  if ( eqtb [ (hash_size + 1833) ] .hh .v.RH != 0 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 335 ) ;		/*  */
    } 
    printesc ( 406 ) ;	/* box */
    print ( 997 ) ;		/* 255 is not void */
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 998 ;	/* You shouldn't use \box255 except in \output routines. */
      helpline [ 0 ] = 986 ;	/* Proceed, and I'll discard its present contents. */
    } 
    boxerror ( 255 ) ; 
	ABORTCHECK;
  } 
  insertpenalties = 0 ; 
  savesplittopskip = eqtb [ (hash_size + 792) ] .hh .v.RH ; 
  if ( eqtb [ (hash_size + 3216) ] .cint <= 0 ) 
  {
    r = mem [ memtop ] .hh .v.RH ; 
    while ( r != memtop ) {
	
      if ( mem [ r + 2 ] .hh .v.LH != 0 ) 
      {
	n = mem [ r ] .hh.b1 ; 
	ensurevbox ( n ) ; 
	ABORTCHECK;
/*    if box(n)=null then box(n):=new_null_box; l.19759 */
	if ( eqtb [ (hash_size + 1578) + n ] .hh .v.RH == 0 ) 
	eqtb [ (hash_size + 1578) + n ] .hh .v.RH = newnullbox () ; 
	p = eqtb [ (hash_size + 1578) + n ] .hh .v.RH + 5 ; 
	while ( mem [ p ] .hh .v.RH != 0 ) p = mem [ p ] .hh .v.RH ; 
	mem [ r + 2 ] .hh .v.RH = p ; 
      } 
      r = mem [ r ] .hh .v.RH ; 
    } 
  } 
  q = memtop - 4 ; 
  mem [ q ] .hh .v.RH = 0 ; 
  prevp = memtop - 2 ; 
  p = mem [ prevp ] .hh .v.RH ; 
  while ( p != bestpagebreak ) {
      
    if ( mem [ p ] .hh.b0 == 3 ) 
    {
      if ( eqtb [ (hash_size + 3216) ] .cint <= 0 ) 
      {
	r = mem [ memtop ] .hh .v.RH ; 
	while ( mem [ r ] .hh.b1 != mem [ p ] .hh.b1 ) r = mem [ r ] .hh .v.RH 
	; 
/*	if best_ins_ptr(r)=null then wait:=true l.19783 */
	if ( mem [ r + 2 ] .hh .v.LH == 0 ) 
	wait = true ; 
	else {
	    
	  wait = false ; 
	  s = mem [ r + 2 ] .hh .v.RH ; 
	  mem [ s ] .hh .v.RH = mem [ p + 4 ] .hh .v.LH ; 
	  if ( mem [ r + 2 ] .hh .v.LH == p ) 
	  {
	    if ( mem [ r ] .hh.b0 == 1 ) 
	    if ( ( mem [ r + 1 ] .hh .v.LH == p ) && ( mem [ r + 1 ] .hh .v.RH 
	    != 0 ) ) 
	    {
	      while ( mem [ s ] .hh .v.RH != mem [ r + 1 ] .hh .v.RH ) s = mem 
	      [ s ] .hh .v.RH ; 
	      mem [ s ] .hh .v.RH = 0 ; 
	      eqtb [ (hash_size + 792) ] .hh .v.RH = mem [ p + 4 ] .hh .v.RH ; 
	      mem [ p + 4 ] .hh .v.LH = prunepagetop ( mem [ r + 1 ] .hh .v.RH 
	      ) ; 
	      if ( mem [ p + 4 ] .hh .v.LH != 0 ) 
	      {
		tempptr = vpackage ( mem [ p + 4 ] .hh .v.LH , 0 , 1 , 
		1073741823L ) ;  /* 2^30 - 1 */
		mem [ p + 3 ] .cint = mem [ tempptr + 3 ] .cint + mem [ 
		tempptr + 2 ] .cint ; 
		freenode ( tempptr , 7 ) ; 
		wait = true ; 
	      } 
	    } 
	    mem [ r + 2 ] .hh .v.LH = 0 ; 
	    n = mem [ r ] .hh.b1 ; 
	    tempptr = mem [ eqtb [ (hash_size + 1578) + n ] .hh .v.RH + 5 ] .hh .v.RH ; 
	    freenode ( eqtb [ (hash_size + 1578) + n ] .hh .v.RH , 7 ) ; 
	    eqtb [ (hash_size + 1578) + n ] .hh .v.RH = vpackage ( tempptr , 0 , 1 , 
	    1073741823L ) ;  /* 2^30 - 1 */
	  } 
	  else {
	      
	    while ( mem [ s ] .hh .v.RH != 0 ) s = mem [ s ] .hh .v.RH ; 
	    mem [ r + 2 ] .hh .v.RH = s ; 
	  } 
	} 
	mem [ prevp ] .hh .v.RH = mem [ p ] .hh .v.RH ; 
	mem [ p ] .hh .v.RH = 0 ; 
	if ( wait ) 
	{
	  mem [ q ] .hh .v.RH = p ; 
	  q = p ; 
	  incr ( insertpenalties ) ; 
	} 
	else {
	    
	  deleteglueref ( mem [ p + 4 ] .hh .v.RH ) ; 
	  freenode ( p , 5 ) ; 
	} 
	p = prevp ; 
      } 
    } 
    else if ( mem [ p ] .hh.b0 == 4 ) 
    {
      if ( curmark [ 1 ] == 0 ) 
      {
	curmark [ 1 ] = mem [ p + 1 ] .cint ; 
	incr ( mem [ curmark [ 1 ] ] .hh .v.LH ) ; 
      } 
      if ( curmark [ 2 ] != 0 ) 
      deletetokenref ( curmark [ 2 ] ) ; 
      curmark [ 2 ] = mem [ p + 1 ] .cint ; 
      incr ( mem [ curmark [ 2 ] ] .hh .v.LH ) ; 
    } 
    prevp = p ; 
    p = mem [ prevp ] .hh .v.RH ; 
  } 
  eqtb [ (hash_size + 792) ] .hh .v.RH = savesplittopskip ; 
  if ( p != 0 )		/* if p<>null then l.19730 */
  {
    if ( mem [ memtop - 1 ] .hh .v.RH == 0 ) /* if link(contrib_head)=null then */
    if ( nestptr == 0 ) 
    curlist .tailfield = pagetail ; 
    else nest [ 0 ] .tailfield = pagetail ; 
    mem [ pagetail ] .hh .v.RH = mem [ memtop - 1 ] .hh .v.RH ; 
    mem [ memtop - 1 ] .hh .v.RH = p ; 
    mem [ prevp ] .hh .v.RH = 0 ; /*   link(prev_p):=null; */
  } 
  savevbadness = eqtb [ (hash_size + 3190) ] .cint ; 
  eqtb [ (hash_size + 3190) ] .cint = 10000 ; 
  savevfuzz = eqtb [ (hash_size + 3739) ] .cint ; 
  eqtb [ (hash_size + 3739) ] .cint = 1073741823L ;  /* 2^30 - 1 */
  eqtb [ (hash_size + 1833) ] .hh .v.RH = vpackage ( mem [ memtop - 2 ] .hh .v.RH , 
  bestsize , 0 , pagemaxdepth ) ; 
  eqtb [ (hash_size + 3190) ] .cint = savevbadness ; 
  eqtb [ (hash_size + 3739) ] .cint = savevfuzz ; 
/*  if ( lastglue != 262143L )  */
  if ( lastglue != emptyflag ) 
  deleteglueref ( lastglue ) ; 
  pagecontents = 0 ; 
  pagetail = memtop - 2 ; 
  mem [ memtop - 2 ] .hh .v.RH = 0 ; 
/*  lastglue = 262143L ;  */
  lastglue = emptyflag ; 
  lastpenalty = 0 ; 
  lastkern = 0 ; 
  pagesofar [ 7 ] = 0 ; 
  pagemaxdepth = 0 ; 
  if ( q != memtop - 4 ) 
  {
    mem [ memtop - 2 ] .hh .v.RH = mem [ memtop - 4 ] .hh .v.RH ; 
    pagetail = q ; 
  } 
  r = mem [ memtop ] .hh .v.RH ; 
  while ( r != memtop ) {
      
    q = mem [ r ] .hh .v.RH ; 
    freenode ( r , 4 ) ; 
    r = q ; 
  } 
  mem [ memtop ] .hh .v.RH = memtop ; 
/* if (top_mark<>null)and(first_mark=null) then l.19654 */
  if ( ( curmark [ 0 ] != 0 ) && ( curmark [ 1 ] == 0 ) ) 
  {
    curmark [ 1 ] = curmark [ 0 ] ; 
    incr ( mem [ curmark [ 0 ] ] .hh .v.LH ) ; 
  } 
/* if output_routine<>null then */
  if ( eqtb [ (hash_size + 1313) ] .hh .v.RH != 0 ) 
  if ( deadcycles >= eqtb [ (hash_size + 3203) ] .cint ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 999 ) ;		/* Output loop--- */
    } 
    printint ( deadcycles ) ; 
    print ( 1000 ) ;		/*  consecutive dead cycles */
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 1001 ;	/* I've concluded that your \output is awry; it never does  */
      helpline [ 1 ] = 1002 ;	/* \shipout, so I'm shipping \box255 out myself. Next  */
      helpline [ 0 ] = 1003 ;	/* increase \maxdeadcycles if you want me to be more patient! */
    } 
    error () ; 
	ABORTCHECK;
  } 
  else {
    outputactive = true ; 
    incr ( deadcycles ) ; 
    pushnest () ; 
    curlist .modefield = -1 ; 
    curlist .auxfield .cint = -65536000L ; 
    curlist .mlfield = - (integer) line ; 
    begintokenlist ( eqtb [ (hash_size + 1313) ] .hh .v.RH , 6 ) ; /* output */
    newsavelevel ( 8 ) ; 
    normalparagraph () ; 
    scanleftbrace () ; 
	ABORTCHECK;
    return ; 
  } 
  {
    if ( mem [ memtop - 2 ] .hh .v.RH != 0 ) 
    {
      if ( mem [ memtop - 1 ] .hh .v.RH == 0 ) 
      if ( nestptr == 0 ) 
      curlist .tailfield = pagetail ; 
      else nest [ 0 ] .tailfield = pagetail ; 
      else mem [ pagetail ] .hh .v.RH = mem [ memtop - 1 ] .hh .v.RH ; 
      mem [ memtop - 1 ] .hh .v.RH = mem [ memtop - 2 ] .hh .v.RH ; 
      mem [ memtop - 2 ] .hh .v.RH = 0 ; 
      pagetail = memtop - 2 ; 
    } 
    shipout ( eqtb [ (hash_size + 1833) ] .hh .v.RH ) ; 
	ABORTCHECK;
    eqtb [ (hash_size + 1833) ] .hh .v.RH = 0 ; 
  } 
} 

/* used to continue here with buildpage etc in tex6.c */

