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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following bit used to be end of tex1.c */

#ifdef STAT
void zrestoretrace ( p , s ) 
halfword p ; 
strnumber s ; 
{restoretrace_regmem 
  begindiagnostic () ; 
  printchar ( 123 ) ;	/* { */
  print ( s ) ; 
  printchar ( 32 ) ;	/*   */
  showeqtb ( p ) ; 
  printchar ( 125 ) ;	/* } */
  enddiagnostic ( false ) ; 
} 
#endif /* STAT */
void unsave ( ) 
{/* 30 */ unsave_regmem 
  halfword p  ; 
  quarterword l  ; 
  halfword t  ; 
  if ( curlevel > 1 ) 
  {
    decr ( curlevel ) ; 
    while ( true ) {
      decr ( saveptr ) ; 
      if ( savestack [ saveptr ] .hh.b0 == 3 ) 
      goto lab30 ; 
      p = savestack [ saveptr ] .hh .v.RH ; 
      if ( savestack [ saveptr ] .hh.b0 == 2 ) 
      {
	t = curtok ; 
	curtok = p ; 
	backinput () ; 
	curtok = t ; 
      } 
      else {
	  
	if ( savestack [ saveptr ] .hh.b0 == 0 ) 
	{
	  l = savestack [ saveptr ] .hh.b1 ; 
	  decr ( saveptr ) ; 
	} 
	else savestack [ saveptr ] = eqtb [ (hash_size + 781) ] ; 
										/* undefine_control_sequence */
	if ( p < (hash_size + 3163) ) 
	if ( eqtb [ p ] .hh.b1 == 1 ) 
	{
	  eqdestroy ( savestack [ saveptr ] ) ; 
	;
#ifdef STAT
	  if ( eqtb [ (hash_size + 3200) ] .cint > 0 ) 
	  restoretrace ( p , 541 ) ; /* retaining */
#endif /* STAT */
	} 
	else {
	    
	  eqdestroy ( eqtb [ p ] ) ; 
	  eqtb [ p ] = savestack [ saveptr ] ; 
	;
#ifdef STAT
	  if ( eqtb [ (hash_size + 3200) ] .cint > 0 ) 
	  restoretrace ( p , 542 ) ; /* restoring */
#endif /* STAT */
	} 
	else if ( xeqlevel [ p ] != 1 ) 
	{
	  eqtb [ p ] = savestack [ saveptr ] ; 
	  xeqlevel [ p ] = l ;			/* l may be used without having been ... */
	;
#ifdef STAT
	  if ( eqtb [ (hash_size + 3200) ] .cint > 0 ) 
	  restoretrace ( p , 542 ) ; /* restoring */
#endif /* STAT */
	} 
	else {
	    
	;
#ifdef STAT
	  if ( eqtb [ (hash_size + 3200) ] .cint > 0 ) 
	  restoretrace ( p , 541 ) ; /* retaining */
#endif /* STAT */
	} 
      } 
    } 
    lab30: curgroup = savestack [ saveptr ] .hh.b1 ; 
    curboundary = savestack [ saveptr ] .hh .v.RH ; 
  } 
  else {
	  confusion ( 540 ) ;	/* curlevel */
	  return;				// abortflag set
  }
} 

/* This is where the old tex2.c used to start */

void preparemag ( ) 
{preparemag_regmem 
  if ( ( magset > 0 ) && ( eqtb [ (hash_size + 3180) ] .cint != magset ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 266  ) ;	/* ! */
      print ( 544 ) ;		/* Incompatible magnification ( */
    } 
    printint ( eqtb [ (hash_size + 3180) ] .cint ) ; 
    print ( 545 ) ;			/* ) */
    printnl ( 546 ) ;		/*  the previous value will be retained */
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 547 ; /* I can handle only one magnification ratio per job. */
      helpline [ 0 ] = 548 ; /* So I've reverted to the magnification you used earlier on this run. */

    } 
    interror ( magset ) ; 
	ABORTCHECK;
    geqworddefine ( (hash_size + 3180) , magset ) ; 
  } 
  if ( ( eqtb [ (hash_size + 3180) ] .cint <= 0 ) ||
	   ( eqtb [ (hash_size + 3180) ] .cint > 32768L ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* ! */
      print ( 549 ) ;		/* Illegal magnification has been changed to 1000 */
    } 
    {
      helpptr = 1 ; 
      helpline [ 0 ] = 550 ; /* The magnification ratio must be between 1 and 32768. */
    } 
    interror ( eqtb [ (hash_size + 3180) ] .cint ) ; 
	ABORTCHECK;
    geqworddefine ( (hash_size + 3180) , 1000 ) ; 
  } 
  magset = eqtb [ (hash_size + 3180) ] .cint ; 
} 

void ztokenshow ( p ) 
halfword p ; 
{tokenshow_regmem 
/* begin if p<>null then show_token_list(link(p),null,10000000); l.6289 */
  if ( p != 0 ) 
  showtokenlist ( mem [ p ] .hh .v.RH , 0 , 10000000L ) ; 
} 
void printmeaning ( ) 
{printmeaning_regmem 
  printcmdchr ( curcmd , curchr ) ; 
  if ( curcmd >= 111 ) 
  {
    printchar ( 58 ) ;	/* : */
    println () ; 
    tokenshow ( curchr ) ; 
  } 
  else if ( curcmd == 110 ) 
  {
    printchar ( 58 ) ;	/* : */
    println () ; 
    tokenshow ( curmark [ curchr ] ) ; 
  } 
} 
void showcurcmdchr ( ) 
{showcurcmdchr_regmem 
  begindiagnostic () ; 
  printnl ( 123 ) ;		/* { */
  if ( curlist .modefield != shownmode ) 
  {
    printmode ( curlist .modefield ) ; 
    print ( 565 ) ; /* :  */
    shownmode = curlist .modefield ; 
  } 
  printcmdchr ( curcmd , curchr ) ; 
  printchar ( 125 ) ;	/* } */
  enddiagnostic ( false ) ; 
} 
void showcontext ( ) 
{/* 30 */ showcontext_regmem 
  char oldsetting  ; 
  integer nn  ; 
  booleane bottomline  ; 
  integer i  ; 
  integer j  ; 
  integer l  ; 
  integer m  ; 
  integer n  ; 
  integer p  ; 
  integer q  ; 
  baseptr = inputptr ; 
  inputstack [ baseptr ] = curinput ; 
  nn = -1 ; 
  bottomline = false ; 
  while ( true ) {
    curinput = inputstack [ baseptr ] ; 
    if ( ( curinput .statefield != 0 ) ) 
    if ( ( curinput .namefield > 17 ) || ( baseptr == 0 ) ) 
		bottomline = true ; 
    if ( ( baseptr == inputptr ) || bottomline ||
		 ( nn < eqtb [ (hash_size + 3217) ] .cint ) ) 
    {
/* begin if (base_ptr=input_ptr) or (state<>token_list) or
   (token_type<>backed_up) or (loc<>null) then
    {we omit backed-up token lists that have already been read} l.6761 */
      if ( ( baseptr == inputptr ) || ( curinput .statefield != 0 ) || ( 
      curinput .indexfield != 3 ) || ( curinput .locfield != 0 ) ) 
      {
	tally = 0 ; 
	oldsetting = selector ;  
	if ( curinput .statefield != 0 ) 
	{
	  if ( curinput .namefield <= 17 ) 
	  if ( ( curinput .namefield == 0 ) ) 
		  if ( baseptr == 0 ) printnl ( 571 ) ;		/* <*> */
		  else printnl ( 572 ) ;	/* <insert>  */
	  else {
	    printnl ( 573 ) ;		/* <read  */
	    if ( curinput .namefield == 17 ) 
			printchar ( 42 ) ;		/* * */
	    else printint ( curinput .namefield - 1 ) ; 
			printchar ( 62 ) ;		/* > */
	  } 
	  else {
/*	    printnl ( 574 ) ; */
/*	    printint ( line ) ; */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
		  if (cstyleflag) {					/* 94/Mar/21 */
			  println ( ) ;					/* new line */
			  /* show current input file name - ignore if from terminal */
			  if (curinput .namefield > 17)	/* redundant ? */
				  print ( curinput .namefield );
			  printchar ( 40 );				/* ( */
			  printint ( line ) ;			/* line number */
			  printchar ( 41 );				/* ) */
			  printchar ( 32 );				/*   */
			  printchar ( 58 );				/* : */
		  }
		  else {
			  printnl ( 574 ) ;				/* l. ? 573 ????? 98/Dec/8 check ! */
			  printint ( line ) ;			/* line number */
		  }
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  } 
	  printchar ( 32 ) ;		/*   */
	  {
	    l = tally ; 
	    tally = 0 ; 
	    selector = 20 ; 
	    trickcount = 1000000L ; 
	  } 
	  if ( buffer [ curinput .limitfield ] == eqtb [ (hash_size + 3211) ] .cint ) 
	  j = curinput .limitfield ; 
	  else j = curinput .limitfield + 1 ; 
	  if ( j > 0 ) 
	  {
		  register integer for_end; 
		  i = curinput .startfield ; 
		  for_end = j - 1 ; 
		  if ( i <= for_end) do 
		  {
			  if ( i == curinput .locfield ) 
			  {
				  firstcount = tally ; 
				  trickcount = tally + 1 + errorline - halferrorline ; 
				  if ( trickcount < errorline ) 
					  trickcount = errorline ; 
			  } 
			  print ( buffer [ i ] ) ; 
		  } 
		  while ( i++ < for_end ) ;
	  } 
	} 
	else {
	    
	  switch ( curinput .indexfield ) 
	  {case 0 : 
	    printnl ( 575 ) ; /* <argument>  */	
		break ; 
	  case 1 : 
	  case 2 : 
	    printnl ( 576 ) ; /* <template>  */
	    break ; 
	  case 3 : 
	    if ( curinput .locfield == 0 ) 
			printnl ( 577 ) ;	/* <recently read>  */
	    else printnl ( 578 ) ;	/* <to be read again>  */
	    break ; 
	  case 4 : 
	    printnl ( 579 ) ; /* <inserted text>  */
	    break ; 
	  case 5 : 
	    {
	      println () ; 
	      printcs ( curinput .namefield ) ; 
	    } 
	    break ; 
	  case 6 : 
	    printnl ( 580 ) ; /* <output>  */
	    break ; 
	  case 7 : 
	    printnl ( 581 ) ; /* <everypar>  */
	    break ; 
	  case 8 : 
	    printnl ( 582 ) ; /* <everymath>  */
	    break ; 
	  case 9 : 
	    printnl ( 583 ) ; /* <everydisplay>  */
	    break ; 
	  case 10 : 
	    printnl ( 584 ) ; /* <everyhbox>  */
	    break ; 
	  case 11 : 
	    printnl ( 585 ) ; /* <everyvbox>  */
	    break ; 
	  case 12 : 
	    printnl ( 586 ) ; /* <everyjob>  */
	    break ; 
	  case 13 : 
	    printnl ( 587 ) ; /* <everycr>  */
	    break ; 
	  case 14 : 
	    printnl ( 588 ) ; /* <mark>  */
	    break ; 
	  case 15 : 
	    printnl ( 589 ) ; /* <write>  */
	    break ; 
	    default: 
	    printnl ( 63 ) ;  /* ? */
	    break ; 
	  } 
	  {
	    l = tally ; 
	    tally = 0 ; 
	    selector = 20 ; 
	    trickcount = 1000000L ; 
	  } 
	  if ( curinput .indexfield < 5 ) 
	  showtokenlist ( curinput .startfield , curinput .locfield , 100000L 
	  ) ; 
	  else showtokenlist ( mem [ curinput .startfield ] .hh .v.RH , 
	  curinput .locfield , 100000L ) ; 
	} 
	selector = oldsetting ; 
	if ( trickcount == 1000000L ) 
	{
	  firstcount = tally ; 
	  trickcount = tally + 1 + errorline - halferrorline ; 
	  if ( trickcount < errorline ) 
		  trickcount = errorline ; 
	} 
	if ( tally < trickcount ) 
		m = tally - firstcount ; 
	else m = trickcount - firstcount ; 
	if ( l + firstcount <= halferrorline ) {
	  p = 0 ; 
	  n = l + firstcount ; 
	} 
	else {
	  print ( 275 ) ; 			/* ... */
	  p = l + firstcount - halferrorline + 3 ; 
	  n = halferrorline ; 
	} 
	{
		register integer for_end; 
		q = p ; 
		for_end = firstcount - 1 ; 
		if ( q 	<= for_end) do 
			printchar ( trickbuf [ q % errorline ] ) ; 
		while ( q++ < for_end ) ;
	} 
	println () ; 
	{
		register integer for_end; 
		q = 1 ; 
		for_end = n ; 
		if ( q <= for_end) 
			do printchar ( 32 ) ;		/*   */
		while ( q++ < for_end ) ;
	} 
	if ( m + n <= errorline ) 
	p = firstcount + m ; 
	else p = firstcount + ( errorline - n - 3 ) ; 
	{
		register integer for_end; 
		q = firstcount ; 
		for_end = p - 1 ; 
		if ( q 	<= for_end) do 
			printchar ( trickbuf [ q % errorline ] ) ; 
		while ( q++ < for_end ) ;
	} 
	if ( m + n > errorline ) 
		print ( 275 ) ; 			/* ... */
	incr ( nn ) ; 
      } 
    } 
    else if ( nn == eqtb [ (hash_size + 3217) ] .cint ) 
    {
      printnl ( 275 ) ;			/* ... */
      incr ( nn ) ; 
    } 
    if ( bottomline ) 
    goto lab30 ; 
    decr ( baseptr ) ; 
  } 
  lab30: curinput = inputstack [ inputptr ] ; 
}

#pragma optimize("g", off) 					/* 98/Dec/10 experiment */
void zbegintokenlist ( p , t ) 
halfword p ; 
quarterword t ; 
{begintokenlist_regmem 
  {
    if ( inputptr > maxinstack ) 
    {
      maxinstack = inputptr ; 
#ifdef ALLOCATEINPUTSTACK
	  if ( inputptr == currentstacksize)
		  inputstack = reallocinputstack (incrementstacksize);
	  if ( inputptr == currentstacksize ) {		/* check again after allocation */
		  overflow ( 590 , currentstacksize ) ;
		  return;			// abortflag set
	  }
#else
	  if ( inputptr == stacksize )	{	/* input stack - not dynamic */
		  overflow ( 590 , stacksize ) ;
		  return;			// abortflag set
	  }
#endif
    } 
    inputstack [ inputptr ] = curinput ; 
    incr ( inputptr ) ; 
  } 
  curinput .statefield = 0 ; 
  curinput .startfield = p ; 
  curinput .indexfield = t ; 
  if ( t >= 5 ) 
  {
    incr ( mem [ p ] .hh .v.LH ) ; 
    if ( t == 5 ) 
    curinput .limitfield = paramptr ; 
    else {
	
      curinput .locfield = mem [ p ] .hh .v.RH ; 
      if ( eqtb [ (hash_size + 3193) ] .cint > 1 ) 
      {
	begindiagnostic () ; 
	printnl ( 335 ) ;	 /* */
	switch ( t ) 
	{case 14 : 
	  printesc ( 348 ) ; /* mark */
	  break ; 
	case 15 : 
	  printesc ( 591 ) ; /* write */
	  break ; 
	  default: 
	  printcmdchr ( 72 , t + (hash_size + 1307) ) ;	/* H */
	  break ; 
	} 
	print ( 553 ) ; /* -> */
	tokenshow ( p ) ; 
	enddiagnostic ( false ) ; 
      } 
    } 
  } 
  else curinput .locfield = p ; 
} 
#pragma optimize("", on) 					/* 98/Dec/10 experiment */

void endtokenlist ( ) 
{endtokenlist_regmem 
  if ( curinput .indexfield >= 3 ) 
  {
    if ( curinput .indexfield <= 4 ) 
    flushlist ( curinput .startfield ) ; 
    else {
      deletetokenref ( curinput .startfield ) ; 
      if ( curinput .indexfield == 5 ) 
      while ( paramptr > curinput .limitfield ) {
		  decr ( paramptr ) ; 
		  flushlist ( paramstack [ paramptr ] ) ; 
      } 
    } 
  } 
  else if ( curinput .indexfield == 1 ) 
  if ( alignstate > 500000L ) alignstate = 0 ; 
  else {
	  fatalerror ( 592 ) ; /* (interwoven alignment preambles are not allowed) */
	  return;			// abortflag set
  }
  {
    decr ( inputptr ) ; 
    curinput = inputstack [ inputptr ] ; 
  } 
  {
    if ( interrupt != 0 ) {
		pauseforinstructions () ;
	}
  } 
}

void backinput ( ) 
{backinput_regmem 
  halfword p  ; 
	while ( ( curinput .statefield == 0 ) && ( curinput .locfield == 0 ) ) {
		endtokenlist () ;
		ABORTCHECK;
	}
	p = getavail () ; 
	mem [ p ] .hh .v.LH = curtok ; 
	if ( curtok < 768 ) 
		if ( curtok < 512 ) 
			decr ( alignstate ) ; 
		else incr ( alignstate ) ; 
	{
		if ( inputptr > maxinstack ) 
		{
			maxinstack = inputptr ; 
#ifdef ALLOCATEINPUTSTACK
			if ( inputptr == currentstacksize)
				inputstack = reallocinputstack (incrementstacksize);
			if ( inputptr == currentstacksize ) {	/* check again after allocation */
				overflow ( 590 , currentstacksize ) ;
				return;			// abortflag set
			}
#else
			if ( inputptr == stacksize )	{	/* stack size - not dynamic */
				overflow ( 590 , stacksize ) ;
				return;			// abortflag set
			}
#endif
		} 
		inputstack [ inputptr ] = curinput ; 
		incr ( inputptr ) ; 
	} 
	curinput .statefield = 0 ; 
	curinput .startfield = p ; 
	curinput .indexfield = 3 ; 
	curinput .locfield = p ; 
} 

void backerror ( ) 
{backerror_regmem 
	OKtointerrupt = false ; 
	backinput () ; 
	OKtointerrupt = true ; 
	error () ; 
} 

void inserror ( ) 
{inserror_regmem 
    OKtointerrupt = false ; 
	backinput () ; 
	curinput .indexfield = 4 ; 
	OKtointerrupt = true ; 
	error () ; 
} 

void beginfilereading ( ) 
{beginfilereading_regmem 
    if ( inopen == maxinopen ) {
	    overflow ( 593 , maxinopen ) ; /* text input levels - NOT DYNAMIC */
		return;			// abortflag set
	}
#ifdef ALLOCATEBUFFER
    if ( first == currentbufsize ) 
		buffer = reallocbuffer (incrementbufsize);
	if ( first == currentbufsize )	{		/* check again after allocation */
		overflow ( 256 , currentbufsize ) ;
		return;			// abortflag set
	}
#else
	if ( first == bufsize ) {
		overflow ( 256 , bufsize ) ;	/* buffer size - not dynamic */
		return;			// abortflag set
	}
#endif

  incr ( inopen ) ; 
  if (inopen > highinopen)			/* 1999 Jan 17 */
	  highinopen = inopen;
  {
    if ( inputptr > maxinstack ) 
    {
      maxinstack = inputptr ; 
#ifdef ALLOCATEINPUTSTACK
	  if ( inputptr == currentstacksize)
		  inputstack = reallocinputstack (incrementstacksize);
	  if ( inputptr == currentstacksize ) {
		  overflow ( 590 , currentstacksize ) ;	/* check again after allocation */
		  return;			// abortflag set
	  }
#else
	  if ( inputptr == stacksize ) {
		  overflow ( 590 , stacksize ) ; 		/* input stack - not dynamic */
		  return;			// abortflag set
	  }
#endif
    } 
    inputstack [ inputptr ] = curinput ; 
    incr ( inputptr ) ; 
  } 
  curinput .indexfield = inopen ; 
  linestack [ curinput .indexfield ] = line ; 
  curinput .startfield = first ; 
  curinput .statefield = 1 ; 
  curinput .namefield = 0 ; 
} 

void endfilereading ( ) 
{endfilereading_regmem 
  first = curinput .startfield ; 
  line = linestack [ curinput .indexfield ] ; 
  if ( curinput .namefield > 17 ) 
	  (void) aclose ( inputfile [ curinput .indexfield ] ) ; 
  {
    decr ( inputptr ) ; 
    curinput = inputstack [ inputptr ] ; 
  } 
  decr ( inopen ) ; 
} 

/* called only form tex0.c */

void clearforerrorprompt ( ) 
{clearforerrorprompt_regmem 
  while ( ( curinput .statefield != 0 ) &&
		  ( curinput .namefield == 0 ) &&
		  ( inputptr > 0 ) &&
		  ( curinput .locfield > curinput .limitfield ) ) 
	endfilereading () ; 
  println () ; 
} 

void checkoutervalidity ( ) 
{checkoutervalidity_regmem 
  halfword p  ; 
  halfword q  ; 
  if ( scannerstatus != 0 ) 
  {
    deletionsallowed = false ; 
    if ( curcs != 0 ) 
    {
      if ( ( curinput .statefield == 0 ) || ( curinput .namefield < 1 ) ||
		   ( curinput .namefield > 17 ) ) 
      {
/*     begin p:=get_avail; info(p):=cs_token_flag+cur_cs; */
	p = getavail () ; 
	mem [ p ] .hh .v.LH = 4095 + curcs ; 
	begintokenlist ( p , 3 ) ; 
      } 
      curcmd = 10 ; 
      curchr = 32 ; 
    } 
    if ( scannerstatus > 1 ) 
    {
      runaway () ; 
      if ( curcs == 0 ) 
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* ! */
	print ( 601 ) ;		/* File ended */
      } 
      else {
	  
	curcs = 0 ; 
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ; /* ! */
	  print ( 602 ) ;	/* Forbidden control sequence found */
	} 
      } 
      print ( 603 ) ; /*  while scanning  */
      p = getavail () ; 
      switch ( scannerstatus ) 
      {case 2 : 
	{
	  print ( 567 ) ; /* definition */
	  mem [ p ] .hh .v.LH = 637 ; 
	} 
	break ; 
      case 3 : 
	{
	  print ( 609 ) ; /* use */
	  mem [ p ] .hh .v.LH = partoken ; 
	  longstate = 113 ; 
	} 
	break ; 
      case 4 : 
	{
	  print ( 569 ) ; /* preamble */
	  mem [ p ] .hh .v.LH = 637 ; 
	  q = p ; 
	  p = getavail () ; 
	  mem [ p ] .hh .v.RH = q ; 
/*	  mem [ p ] .hh .v.LH = (hash_size + 4610) ;  */
/*	  mem [ p ] .hh .v.LH = (hash_size + 4095 + 515) ;  */
	  mem [ p ] .hh .v.LH = (hash_size + hash_extra + 4095 + 515) ; /*96/Jan/10*/
	  alignstate = -1000000L ; 
	} 
	break ; 
      case 5 : 
	{
	  print ( 570 ) ; /* text */
	  mem [ p ] .hh .v.LH = 637 ; 
	} 
	break ; 
      } 
      begintokenlist ( p , 4 ) ; 
      print ( 604 ) ; /*  of  */
      sprintcs ( warningindex ) ; 
      {
	helpptr = 4 ; 
	helpline [ 3 ] = 605 ; /* I suspect you have forgotten a `}', causing me */
	helpline [ 2 ] = 606 ; /* to read past where you wanted me to stop. */
	helpline [ 1 ] = 607 ; /* I'll try to recover; but if the error is serious, */
	helpline [ 0 ] = 608 ; /* you'd better type `E' or `X' now and fix your file. */
      } 
      error () ; 
	  ABORTCHECK;
    } 
    else {
	
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* ! */
	print ( 595 ) ;		/* Incomplete  */
      } 
      printcmdchr ( 105 , curif ) ;	/* i */
      print ( 596 ) ; /* ; all text was ignored after line  */
      printint ( skipline ) ; 
      {
	helpptr = 3 ; 
	helpline [ 2 ] = 597 ; /* A forbidden control sequence occurred in skipped text. */
	helpline [ 1 ] = 598 ; /* This kind of error happens when you say `\if...' and forget */
	helpline [ 0 ] = 599 ; /* the matching `\fi'. I've inserted a `\fi'; this might work. */
      } 
      if ( curcs != 0 ) 
      curcs = 0 ; 
      else helpline [ 2 ] = 600 ; /* The file ended while I was skipping conditional text. */
/*      curtok = (hash_size + 4613) ;  */
/*      curtok = (hash_size + 4095 + 518) ;  */
      curtok = (hash_size + hash_extra + 4095 + 518) ; /* 96/Jan/10 */
      inserror () ; 
	  ABORTCHECK;
	}
    deletionsallowed = true ; 
  }
} 

/*****************************************************************************/

/* getnext() moved from here to end for pragma optimize reasons 96/Sep/12 */

void getnext(void);

/*****************************************************************************/

void firmuptheline ( ) 
{firmuptheline_regmem 
  integer k  ; 
  curinput .limitfield = last ; 
  if ( eqtb [ (hash_size + 3191) ] .cint > 0 ) 
	  if ( interaction > 1 )  {
		  ; 
		  println () ; 
		  if ( curinput .startfield < curinput .limitfield )  {
			  register integer for_end; 
			  k = curinput .startfield ; 
			  for_end = curinput .limitfield - 1 ; 
			  if ( k <= for_end) do print ( buffer [ k ] ) ; 
			  while ( k++ < for_end ) ;
		  } 
		  first = curinput .limitfield ; 
		  {
			  ; 
			  print ( 615 ) ; /* => */
			  terminput ( 615, 0 ) ; 
			  ABORTCHECK;
		  } 
		  if ( last > first ) {
			  {
				  register integer for_end; 
				  k = first ; 
				  for_end = last - 1 ; 
				  if ( k <= for_end) do 
					  buffer [ k + curinput .startfield - first ] = buffer [ k ] ; 
				  while ( k++ < for_end ) ;
			  } 
			  curinput .limitfield = curinput .startfield + last - first ; 
		  } 
	  } 
} 

void gettoken ( ) 
{gettoken_regmem 
	nonewcontrolsequence = false ; 
	getnext () ; 
	ABORTCHECK;
	nonewcontrolsequence = true ; 
	if ( curcs == 0 ) curtok = ( curcmd * 256 ) + curchr ; 
	else curtok = 4095 + curcs ; 
} 

void macrocall ( ) 
{/* 10 22 30 31 40 */ macrocall_regmem 
  halfword r  ; 
  halfword p  ; 
  halfword q  ; 
  halfword s  ; 
  halfword t  ; 
  halfword u, v  ; 
  halfword rbraceptr  ; 
  smallnumber n  ; 
  halfword unbalance  ; 
  halfword m  ; 
  halfword refcount  ; 
  smallnumber savescannerstatus  ; 
  halfword savewarningindex  ; 
  ASCIIcode matchchr  ; 

  savescannerstatus = scannerstatus ;  
  savewarningindex = warningindex ; 
  warningindex = curcs ; 
  refcount = curchr ; 
  r = mem [ refcount ] .hh .v.RH ; 
  n = 0 ; 
  if ( eqtb [ (hash_size + 3193) ] .cint > 0 ) 
  {
    begindiagnostic () ; 
    println () ; 
    printcs ( warningindex ) ; 
    tokenshow ( refcount ) ; 
    enddiagnostic ( false ) ; 
  } 
  if ( mem [ r ] .hh .v.LH != 3584 ) 
  {
    scannerstatus = 3 ; 
    unbalance = 0 ; 
    longstate = eqtb [ curcs ] .hh.b0 ; 
    if ( longstate >= 113 ) 
    longstate = longstate - 2 ; 
    do {
	mem [ memtop - 3 ] .hh .v.RH = 0 ; /* repeat link(temp_head):=null; */
      if ( ( mem [ r ] .hh .v.LH > 3583 ) || ( mem [ r ] .hh .v.LH < 3328 ) ) 
      s = 0 ; /* s:=null l.7984 */
      else {
	  
	matchchr = mem [ r ] .hh .v.LH - 3328 ; 
	s = mem [ r ] .hh .v.RH ; 
	r = s ; 
	p = memtop - 3 ; 
	m = 0 ; 
      } 
      lab22: gettoken () ; 
      if ( curtok == mem [ r ] .hh .v.LH ) 
      {
	r = mem [ r ] .hh .v.RH ; 
	if ( ( mem [ r ] .hh .v.LH >= 3328 ) && ( mem [ r ] .hh .v.LH <= 3584 
	) ) 
	{
	  if ( curtok < 512 ) 
	  decr ( alignstate ) ; 
	  goto lab40 ; 
	} 
	else goto lab22 ; 
      } 
      if ( s != r ) 
      if ( s == 0 ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 647 ) ;		/* Use of  */
	} 
	sprintcs ( warningindex ) ; 
	print ( 648 ) ;			/*  doesn't match its definition */
	{
	  helpptr = 4 ; 
	  helpline [ 3 ] = 649 ; /* If you say, e.g., `\def\a1{...}', then you must always */
	  helpline [ 2 ] = 650 ; /* put `1' after `\a', since control sequence names are */
	  helpline [ 1 ] = 651 ; /* made up of letters only. The macro here has not been */
	  helpline [ 0 ] = 652 ; /* followed by the required stuff, so I'm ignoring it. */
	} 
	error () ; 
	ABORTCHECK;
	goto lab10 ; 
      } 
      else {
	  
	t = s ; 
	do {
	    { 
	    q = getavail () ; 
	    mem [ p ] .hh .v.RH = q ; 
	    mem [ q ] .hh .v.LH = mem [ t ] .hh .v.LH ; 
	    p = q ; 
	  } 
	  incr ( m ) ; 
	  u = mem [ t ] .hh .v.RH ; 
	  v = s ; 
	  while ( true ) {
	    if ( u == r ) 
	    if ( curtok != mem [ v ] .hh .v.LH ) 
	    goto lab30 ; 
	    else {
		
	      r = mem [ v ] .hh .v.RH ; 
	      goto lab22 ; 
	    } 
	    if ( mem [ u ] .hh .v.LH != mem [ v ] .hh .v.LH ) 
	    goto lab30 ; 
	    u = mem [ u ] .hh .v.RH ; 
	    v = mem [ v ] .hh .v.RH ; 
	  } 
	  lab30: t = mem [ t ] .hh .v.RH ; 
	} while ( ! ( t == r ) ) ; 
	r = s ; 
      } 
      if ( curtok == partoken ) 
      if ( longstate != 112 ) 
      {
	if ( longstate == 111 ) 
	{
	  runaway () ; 
	  {
	    if ( interaction == 3 ) 
	    ; 
	    printnl ( 262 ) ;	/* !  */
	    print ( 642 ) ;		/* Paragraph ended before  */
	  } 
	  sprintcs ( warningindex ) ; 
	  print ( 643 ) ;		/* was complete */
	  {
	    helpptr = 3 ; 
	    helpline [ 2 ] = 644 ; /* I suspect you've forgotten a `}', causing me to apply this */
	    helpline [ 1 ] = 645 ; /* control sequence to too much text. How can we recover? */
	    helpline [ 0 ] = 646 ; /* My plan is to forget the whole thing and hope for the best. */
	  } 
	  backerror () ; 
	  ABORTCHECK;
	} 
	pstack [ n ] = mem [ memtop - 3 ] .hh .v.RH ; 
	alignstate = alignstate - unbalance ; 
	{
		register integer for_end; 
		m = 0 ; 
		for_end = n ; 
		if ( m <= for_end) do 
			flushlist ( pstack [ m ] ) ; 
		while ( m++ < for_end ) ;
	} 
	goto lab10 ; 
      } 
      if ( curtok < 768 ) 
      if ( curtok < 512 ) 
      {
	unbalance = 1 ; 
	while ( true ) {
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
	    mem [ q ] .hh .v.LH = curtok ; 
	    p = q ; 
	  } 
	  gettoken () ; 
	  if ( curtok == partoken ) 
	  if ( longstate != 112 ) 
	  {
	    if ( longstate == 111 ) 
	    {
	      runaway () ; 
	      {
		if ( interaction == 3 ) 
		; 
		printnl ( 262 ) ;	/* !  */
		print ( 642 ) ;		/* Paragraph ended before  */
	      } 
	      sprintcs ( warningindex ) ; 
	      print ( 643 ) ; /*  was complete */
	      {
		helpptr = 3 ; 
		helpline [ 2 ] = 644 ; /* I suspect you've forgotten a `}', causing me to apply this */
		helpline [ 1 ] = 645 ; /* control sequence to too much text. How can we recover? */
		helpline [ 0 ] = 646 ; /* My plan is to forget the whole thing and hope for the best. */
	      } 
	      backerror () ; 
		  ABORTCHECK;
	    } 
	    pstack [ n ] = mem [ memtop - 3 ] .hh .v.RH ; 
	    alignstate = alignstate - unbalance ; 
	    {
			register integer for_end; 
			m = 0 ; 
			for_end = n ; 
			if ( m <= for_end) do 
				flushlist ( pstack [ m ] ) ; 
			while ( m++ < for_end ) ;
		} 
	    goto lab10 ; 
	  } 
	  if ( curtok < 768 ) 
	  if ( curtok < 512 ) 
	  incr ( unbalance ) ; 
	  else {
	      
	    decr ( unbalance ) ; 
	    if ( unbalance == 0 ) 
	    goto lab31 ; 
	  } 
	} 
	lab31: rbraceptr = p ; 
	{
	  q = getavail () ; 
	  mem [ p ] .hh .v.RH = q ; 
	  mem [ q ] .hh .v.LH = curtok ; 
	  p = q ; 
	} 
      } 
      else {
	  
	backinput () ; 
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 634 ) ;		/* Argument of  */
	} 
	sprintcs ( warningindex ) ; 
	print ( 635 ) ;			/*  has an extra } */
	{
	  helpptr = 6 ; 
	  helpline [ 5 ] = 636 ; /* I've run across a `}' that doesn't seem to match anything. */
	  helpline [ 4 ] = 637 ; /* For example, `\def\a#1{...}' and `\a}' would produce */
	  helpline [ 3 ] = 638 ; /* this error. If you simply proceed now, the `\par' that */
	  helpline [ 2 ] = 639 ; /* I've just inserted will cause me to report a runaway */
	  helpline [ 1 ] = 640 ; /* argument that might be the root of the problem. But if */
	  helpline [ 0 ] = 641 ; /* your `}' was spurious, just type `2' and it will go away. */
	} 
	incr ( alignstate ) ; 
	longstate = 111 ; 
	curtok = partoken ; 
	inserror () ; 
	ABORTCHECK;
      } 
      else {
	  
	if ( curtok == 2592 ) 
	if ( mem [ r ] .hh .v.LH <= 3584 ) 
	if ( mem [ r ] .hh .v.LH >= 3328 ) 
	goto lab22 ; 
	{
	  q = getavail () ; 
	  mem [ p ] .hh .v.RH = q ;		/* p may be used without having ... */
	  mem [ q ] .hh .v.LH = curtok ; 
	  p = q ; 
	} 
      } 
      incr ( m ) ;					/* m may be used without having been ... */
      if ( mem [ r ] .hh .v.LH > 3584 ) 
		  goto lab22 ; 
      if ( mem [ r ] .hh .v.LH < 3328 ) 
		  goto lab22 ; 
      lab40: if ( s != 0 ) 
      {
	if ( ( m == 1 ) && ( mem [ p ] .hh .v.LH < 768 ) && ( p != memtop - 3 
	) ) 
	{
	  mem [ rbraceptr ] .hh .v.RH = 0 ; /* rbraceptr may be used without ... */
	  {
	    mem [ p ] .hh .v.RH = avail ; 
	    avail = p ; 
	;
#ifdef STAT
	    decr ( dynused ) ; 
#endif /* STAT */
	  } 
	  p = mem [ memtop - 3 ] .hh .v.RH ; 
	  pstack [ n ] = mem [ p ] .hh .v.RH ; 
	  {
	    mem [ p ] .hh .v.RH = avail ; 
	    avail = p ; 
	;
#ifdef STAT
	    decr ( dynused ) ; 
#endif /* STAT */
	  } 
	} 
	else pstack [ n ] = mem [ memtop - 3 ] .hh .v.RH ; 
	incr ( n ) ; 
	if ( eqtb [ (hash_size + 3193) ] .cint > 0 ) 
	{
	  begindiagnostic () ; 
	  printnl ( matchchr ) ;	/* matchchar may be used without ... */
	  printint ( n ) ; 
	  print ( 653 ) ; /* <- */
	  showtokenlist ( pstack [ n - 1 ] , 0 , 1000 ) ; 
	  enddiagnostic ( false ) ; 
	} 
      } 
    } while ( ! ( mem [ r ] .hh .v.LH == 3584 ) ) ; 
  } 
/* while (state=token_list)and(loc=null) do end_token_list; l.7956 */
  while ( ( curinput .statefield == 0 ) && ( curinput .locfield == 0 ) ) 
  endtokenlist () ; 
  ABORTCHECK;
  begintokenlist ( refcount , 5 ) ; 
  curinput .namefield = warningindex ; 
  curinput .locfield = mem [ r ] .hh .v.RH ; 
  if ( n > 0 ) 
  {
    if ( paramptr + n > maxparamstack ) 
    {
      maxparamstack = paramptr + n ; 
#ifdef ALLOCATEPARAMSTACK
	  if ( maxparamstack > currentparamsize ) 
		  paramstack = reallocparamstack (incrementparamsize);
	  if ( maxparamstack > currentparamsize ) {	/* check again after allocation */
		  overflow ( 633 , currentparamsize ) ;
		  return;			// abortflag set
	  }
#else
	  if ( maxparamstack > paramsize ) {
		  overflow ( 633 , paramsize ) ; /* parameter stack - not dynamic */
		  return;			// abortflag set
	  }
#endif
    } 
    {
		register integer for_end; 
		m = 0 ; 
		for_end = n - 1 ; 
		if ( m <= for_end) 
			do paramstack [ paramptr + m ] = pstack [ m ] ; 
		while ( m++ < for_end ) ;
	} 
    paramptr = paramptr + n ; 
  } 
  lab10: scannerstatus = savescannerstatus ; 
  warningindex = savewarningindex ; 
}

void insertrelax ( ) 
{insertrelax_regmem 
/* begin cur_tok:=cs_token_flag+cur_cs; back_input; */
  curtok = 4095 + curcs ; 
  backinput () ; 
/* cur_tok:=cs_token_flag+frozen_relax; back_input; token_type:=inserted; */
/*  curtok = (hash_size + 4616) ;  */
/*  curtok = (hash_size + 4095 + 521) ;  */
  curtok = (hash_size + hash_extra + 4095 + 521) ;	/* 96/Jan/10 */
  backinput () ; 
  curinput .indexfield = 4 ; 
} 

void expand ( ) 
{expand_regmem 
  halfword t  ; 
  halfword p, q, r  ; 
  integer j  ; 
  integer cvbackup  ; 
  smallnumber cvlbackup, radixbackup, cobackup  ; 
  halfword backupbackup  ; 
  smallnumber savescannerstatus  ; 

  cvbackup = curval ; 
  cvlbackup = curvallevel ;  
  radixbackup = radix ;  
  cobackup = curorder ;  
  backupbackup = mem [ memtop - 13 ] .hh .v.RH ; 
  if ( curcmd < 111 ) 
  {
    if ( eqtb [ (hash_size + 3199) ] .cint > 1 ) 
    showcurcmdchr () ; 
    switch ( curcmd ) 
    {case 110 : 
      {
/* begin if cur_mark[cur_chr]<>null then l.7881 */
	if ( curmark [ curchr ] != 0 ) 
	begintokenlist ( curmark [ curchr ] , 14 ) ; 
      } 
      break ; 
    case 102 : 
      {
	gettoken () ; 
	t = curtok ; 
	gettoken () ; 
	if ( curcmd > 100 ) {
		expand () ;
		ABORTCHECK;
	}
	else backinput () ; 
	curtok = t ; 
	backinput () ; 
      } 
      break ; 
    case 103 : 
      {
	savescannerstatus = scannerstatus ;  
	scannerstatus = 0 ; 
	gettoken () ; 
	scannerstatus = savescannerstatus ; 
	t = curtok ; 
	backinput () ; 
	if ( t >= 4095 )		/* if t>=cs_token_flag then */
	{
/*   begin p:=get_avail; info(p):=cs_token_flag+frozen_dont_expand; */
	  p = getavail () ; 
/*	  mem [ p ] .hh .v.LH = (hash_size + 4618) ;  */
/*	  mem [ p ] .hh .v.LH = (hash_size + 4095 + 523) ; */
	  mem [ p ] .hh .v.LH = (hash_size + hash_extra + 4095 + 523) ; /*96/Jan/10*/
	  mem [ p ] .hh .v.RH = curinput .locfield ; 
	  curinput .startfield = p ; 
	  curinput .locfield = p ; 
	} 
      } 
      break ; 
    case 107 : 
      {
	r = getavail () ; 
	p = r ; 
	do {
	    getxtoken () ; 
		ABORTCHECK;
	  if ( curcs == 0 ) {
	    q = getavail () ; 
	    mem [ p ] .hh .v.RH = q ; 
	    mem [ q ] .hh .v.LH = curtok ; 
	    p = q ; 
	  } 
	} while ( ! ( curcs != 0 ) ) ; 
	if ( curcmd != 67 ) 
	{
	  {
	    if ( interaction == 3 ) 
	    ; 
	    printnl ( 262 ) ;	/* !  */
	    print ( 622 ) ;		/* Missing  */
	  } 
	  printesc ( 502 ) ;	/* endcsname */
	  print ( 623 ) ;		/*  inserted */
	  {
	    helpptr = 2 ; 
	    helpline [ 1 ] = 624 ; /* The control sequence marked <to be read again> should */
	    helpline [ 0 ] = 625 ; /* not appear between \csname and \endcsname. */
	  } 
	  backerror () ; 
	  ABORTCHECK;
	} 
	j = first ; 
	p = mem [ r ] .hh .v.RH ; 
	while ( p != 0 ) {	/* while p<>null do l.7742 */
	    
	  if ( j >= maxbufstack ) 
	  {
	    maxbufstack = j + 1 ; 
#ifdef ALLOCATEBUFFER
		if ( maxbufstack == currentbufsize )
			buffer = reallocbuffer (incrementbufsize);
		if ( maxbufstack == currentbufsize ) {	/* check again after allocation */
			overflow ( 256 , currentbufsize ) ;
			return;			// abortflag set
		}
#else
		if ( maxbufstack == bufsize ) {
			overflow ( 256 , bufsize ) ; /* buffer size - not dynamic */
			return;			// abortflag set
		}
#endif
	  } 
	  buffer [ j ] = mem [ p ] .hh .v.LH % 256 ; 
/*	  buffer [ j ] = mem [ p ] .hh .v.LH & 255 ; */	/* last 8 bits */
	  incr ( j ) ; 
	  p = mem [ p ] .hh .v.RH ; 
	} 
	if ( j > first + 1 ) 
	{
	  nonewcontrolsequence = false ; 
	  curcs = idlookup ( first , j - first ) ; 
	  nonewcontrolsequence = true ; 
	} 
	else if ( j == first ) 
	curcs = 513 ; 
/* else cur_cs:=single_base+buffer[first] {the list has length one} */
	else curcs = 257 + buffer [ first ] ; 
	flushlist ( r ) ; 
	if ( eqtb [ curcs ] .hh.b0 == 101 ) 
	{
	  eqdefine ( curcs , 0 , 256 ) ; 
	} 
	curtok = curcs + 4095 ; 
	backinput () ; 
      } 
      break ; 
    case 108 : 
      convtoks () ; 
      break ; 
    case 109 : 
      insthetoks () ; 
      break ; 
    case 105 : 
      conditional () ; 
	  ABORTCHECK;
      break ; 
    case 106 : 
      if ( curchr > iflimit ) 
      if ( iflimit == 1 ) 
      insertrelax () ; 
      else {
	  
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 773 ) ;		/* Extra  */
	} 
	printcmdchr ( 106 , curchr ) ;	/* j */
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 774 ; /* I'm ignoring this; it doesn't match any \if. */
	} 
	error () ; 
	ABORTCHECK;
      } 
      else {
	  
	while ( curchr != 2 ) passtext () ; 
	{
	  p = condptr ; 
	  ifline = mem [ p + 1 ] .cint ; 
	  curif = mem [ p ] .hh.b1 ; 
	  iflimit = mem [ p ] .hh.b0 ; 
	  condptr = mem [ p ] .hh .v.RH ; 
	  freenode ( p , 2 ) ; 
	} 
      } 
      break ; 
    case 104 : 
      if ( curchr > 0 ) forceeof = true ; 
      else if ( nameinprogress ) insertrelax () ; 
      else startinput () ; 
      break ; 
      default: 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 616 ) ;		/* Undefined control sequence */
	} 
	{
	  helpptr = 5 ; 
	  helpline [ 4 ] = 617 ; /* The control sequence at the end of the top line */
	  helpline [ 3 ] = 618 ; /* of your error message was never \def'ed. If you have */
	  helpline [ 2 ] = 619 ; /* misspelled it (e.g., `\hobx'), type `I' and the correct */
	  helpline [ 1 ] = 620 ; /* spelling (e.g., `I\hbox'). Otherwise just continue, */
	  helpline [ 0 ] = 621 ; /* and I'll forget about whatever was undefined. */
	} 
	error () ; 
	ABORTCHECK;
      } 
      break ; 
    } 
  } 
  else if ( curcmd < 115 ) {
	  macrocall () ;
	  ABORTCHECK;
  }
  else {
      
/*    curtok = (hash_size + 4615) ;  */
/*    curtok = (hash_size + 4095 + 520) ;  */
    curtok = (hash_size + hash_extra + 4095 + 520) ; /* 96/Jan/10 */
    backinput () ; 
  } 
  curval = cvbackup ; 
  curvallevel = cvlbackup ; 
  radix = radixbackup ; 
  curorder = cobackup ; 
  mem [ memtop - 13 ] .hh .v.RH = backupbackup ; 
} 

void getxtoken ( ) 
{/* 20 30 */ getxtoken_regmem 

lab20:
	ABORTCHECK;
	getnext () ; 
	ABORTCHECK;
	if ( curcmd <= 100 )  goto lab30 ; 
	if ( curcmd >= 111 ) 
		if ( curcmd < 115 ) {
			macrocall () ;
			ABORTCHECK;
		}
		else {
/*			curcs = (hash_size + 520) ;  */
			curcs = (hash_size + hash_extra + 520) ;	/* 96/Jan/10 */
			curcmd = 9 ; 
			goto lab30 ; 
		} 
	else {
		expand () ;
		ABORTCHECK;
	}
	goto lab20 ; 
lab30: if ( curcs == 0 ) 
		   curtok = ( curcmd * 256 ) + curchr ; 
	   else curtok = 4095 + curcs ; 
} 

void xtoken ( ) 
{xtoken_regmem 
  while ( curcmd > 100 ) {
    expand () ; 
	ABORTCHECK;
    getnext () ; 
	ABORTCHECK;
  } 
  if ( curcs == 0 ) 
  curtok = ( curcmd * 256 ) + curchr ; 
  else curtok = 4095 + curcs ; 
} 

void scanleftbrace ( ) 
{scanleftbrace_regmem 
  do {
      getxtoken () ; 
	  ABORTCHECK;
  } while ( ! ( ( curcmd != 10 ) && ( curcmd != 0 ) ) ) ; 
  if ( curcmd != 1 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 654 ) ;		/* Missing { inserted */
    } 
    {
      helpptr = 4 ; 
      helpline [ 3 ] = 655 ; /* A left brace was mandatory here, so I've put one in. */
      helpline [ 2 ] = 656 ; /* You might want to delete and/or insert some corrections */
      helpline [ 1 ] = 657 ; /* so that I will find a matching right brace soon. */
      helpline [ 0 ] = 658 ; /* (If you're confused by all this, try typing `I}' now.) */
    } 
    backerror () ; 
	ABORTCHECK;
    curtok = 379 ; 
    curcmd = 1 ; 
    curchr = 123 ; 
    incr ( alignstate ) ; 
  } 
} 

void scanoptionalequals ( ) 
{scanoptionalequals_regmem 
  do {
      getxtoken () ; 
	  ABORTCHECK;
  } while ( ! ( curcmd != 10 ) ) ; 
  if ( curtok != 3133 ) backinput () ; 
} 

booleane zscankeyword ( s ) 
strnumber s ; 
{/* 10 */ register booleane Result; scankeyword_regmem 
  halfword p  ; 
  halfword q  ; 
  poolpointer k  ; 
  p = memtop - 13 ; 
  mem [ p ] .hh .v.RH = 0 ; 
  k = strstart [ s ] ; 
  while ( k < strstart [ s + 1 ] ) {
    getxtoken () ; 
	ABORTCHECKZERO;
    if ( ( curcs == 0 ) && ( ( curchr == strpool [ k ] ) || ( curchr == 
		strpool [ k ] - 32 ) ) )  {
      {
	q = getavail () ; 
	mem [ p ] .hh .v.RH = q ; 
	mem [ q ] .hh .v.LH = curtok ; 
	p = q ; 
      } 
      incr ( k ) ; 
    } 
    else if ( ( curcmd != 10 ) || ( p != memtop - 13 ) ) 
    {
      backinput () ; 
      if ( p != memtop - 13 ) 
		  begintokenlist ( mem [ memtop - 13 ] .hh .v.RH , 3 ) ; 
      Result = false ; 
      return(Result) ; 
    } 
  } 
  flushlist ( mem [ memtop - 13 ] .hh .v.RH ) ; 
  Result = true ; 
  return Result ; 
} 

void muerror ( ) 
{muerror_regmem 
  {
    if ( interaction == 3 ) 
    ; 
    printnl ( 262 ) ;	/* !  */
    print ( 659 ) ;		/* Incompatible glue units */
  } 
  {
    helpptr = 1 ; 
    helpline [ 0 ] = 660 ; /* I'm going to assume that 1mu=1pt when they're mixed. */
  } 
  error () ; 
//	ABORTCHECK;
} 

void scaneightbitint ( ) 
{scaneightbitint_regmem 
    scanint () ; 
	ABORTCHECK;
  if ( ( curval < 0 ) || ( curval > 255 ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 684 ) ;		/* Bad register code */
    } 
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 685 ; /* A register number must be between 0 and 255. */
      helpline [ 0 ] = 686 ; /* I changed this one to zero. */
    } 
    interror ( curval ) ; 
//	ABORTCHECK;
    curval = 0 ; 
  } 
} 

void scancharnum ( ) 
{scancharnum_regmem 
    scanint () ; 
	ABORTCHECK;
  if ( ( curval < 0 ) || ( curval > 255 ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 687 ) ;		/* Bad character code */
    } 
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 688 ; /* A character number must be between 0 and 255. */
      helpline [ 0 ] = 686 ; /* I changed this one to zero. */
    } 
    interror ( curval ) ; 
//	ABORTCHECK;
    curval = 0 ; 
  } 
} 

void scanfourbitint ( ) 
{scanfourbitint_regmem 
    scanint () ; 
	ABORTCHECK;
  if ( ( curval < 0 ) || ( curval > 15 ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 689 ) ;		/* Bad number */
    } 
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 690 ; /* Since I expected to read a number between 0 and 15, */
      helpline [ 0 ] = 686 ; /* I changed this one to zero. */
    } 
    interror ( curval ) ; 
//	ABORTCHECK;
    curval = 0 ; 
  } 
} 

void scanfifteenbitint ( ) 
{scanfifteenbitint_regmem 
    scanint () ; 
	ABORTCHECK;
  if ( ( curval < 0 ) || ( curval > 32767 ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 691 ) ;		/* Bad mathchar */
    } 
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 692 ; /* A mathchar number must be between 0 and 32767. */
      helpline [ 0 ] = 686 ; /* I changed this one to zero. */
    } 
    interror ( curval ) ; 
//	ABORTCHECK;
    curval = 0 ; 
  } 
} 

void scantwentysevenbitint ( ) 
{scantwentysevenbitint_regmem 
    scanint () ; 
	ABORTCHECK;
  if ( ( curval < 0 ) || ( curval > 134217727L ) ) /* 2^27 - 1 */
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 693 ) ;		/* Bad delimiter code */
    } 
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 694 ; /* A numeric delimiter code must be between 0 and 2^{27}-1. */
      helpline [ 0 ] = 686 ; /* I changed this one to zero. */
    } 
    interror ( curval ) ; 
//	ABORTCHECK;
    curval = 0 ; 
  } 
} 

void scanfontident ( ) 
{scanfontident_regmem 
  internalfontnumber f  ; 
  halfword m  ; 
  do {
      getxtoken () ; 
	  ABORTCHECK;
  } while ( ! ( curcmd != 10 ) ) ; 
  if ( curcmd == 88 ) 
  f = eqtb [ (hash_size + 1834) ] .hh .v.RH ; 
  else if ( curcmd == 87 ) 
  f = curchr ; 
  else if ( curcmd == 86 ) 
  {
    m = curchr ; 
    scanfourbitint () ; 
	ABORTCHECK;
    f = eqtb [ m + curval ] .hh .v.RH ; 
  } 
  else {
      
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 811 ) ;		/* Missing font identifier */
    } 
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 812 ; /* I was looking for a control sequence whose */
      helpline [ 0 ] = 813 ; /* current meaning has been defined by \font. */
    } 
    backerror () ; 
//	ABORTCHECK;
    f = 0 ; 
  }
  curval = f ; 
} 

void zfindfontdimen ( writing ) 
booleane writing ; 
{findfontdimen_regmem 
  internalfontnumber f  ; 
  integer n  ; 
  scanint () ; 
  ABORTCHECK;
  n = curval ; 
  scanfontident () ; 
  ABORTCHECK;
  f = curval ; 
/*  if ( n <= 0 ) */						/* change 98/Oct/5 */
  if ( n < 0 || (n == 0 && fontdimenzero == 0))
	  curval = fmemptr ; 
  else {
/* else  begin if writing and(n<=space_shrink_code)and@|
    (n>=space_code)and(font_glue[f]<>null) then
    begin delete_glue_ref(font_glue[f]); l.11225 */
    if ( writing && ( n <= 4 ) && ( n >= 2 ) && ( fontglue [ f ] != 0 ) ) 
    {
		deleteglueref ( fontglue [ f ] ) ; 
		fontglue [ f ] = 0 ;	/* font_glue[f]:=null */
    } 
    if ( n > fontparams [ f ] ) 
		if ( f < fontptr ) 
			curval = fmemptr ; 
		else {
			do {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
 #ifdef ALLOCATEFONT
				if ( fmemptr == currentfontmemsize )  {	/* 93/Nov/28 ??? */
					fontinfo = reallocfontinfo(incrementfontmemsize);
				}
				if ( fmemptr == currentfontmemsize ) {		/* 94/Jan/24 */
					overflow ( 818 , currentfontmemsize ) ; /* font memory */
					return;			// abortflag set
				}
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
				if ( fmemptr == fontmemsize ) {
					overflow ( 818 , fontmemsize ) ; /* font memory */
					return;			// abortflag set
				}
#endif
				fontinfo [ fmemptr ] .cint = 0 ; 
				incr ( fmemptr ) ; 
				incr ( fontparams [ f ] ) ; 
			} while ( ! ( n == fontparams [ f ] ) ) ; 
			curval = fmemptr - 1 ; 
		} 
/*	else curval = n + parambase [ f ] ;   */			/* 98/Oct/5 */
	else if (n > 0) curval = n + parambase [ f ] ;		/* 98/Oct/5 */
	else curval = &fontcheck [ f ] - &fontinfo [ 0 ];	/* 98/Oct/5 */
/*	checksum =  (((fontcheck [ f ] .b0) << 8 | fontcheck [ f ] .b1) << 8 |
				fontcheck [ f ] .b2) << 8 | fontcheck [ f ] .b3; */
  } 
/* compiler error: '-' : incompatible types - from 'union fmemoryword *' to 'struct fourunsignedchars *' */
  if ( curval == fmemptr ) {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 796 ) ;		/* Font  */
    } 
/*    printesc ( hash [ (hash_size + 524) + f ] .v.RH ) ; */
    printesc ( hash [ (hash_size + hash_extra + 524) + f ] .v.RH ) ; /*96/Jan/10*/
    print ( 814 ) ; /* has  only  */
    printint ( fontparams [ f ] ) ; 
    print ( 815 ) ; /*  fontdimen parameters */
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 816 ; /* To increase the number of font parameters, you must */
      helpline [ 0 ] = 817 ; /* use \fontdimen immediately after the \font is loaded. */
    } 
    error () ; 
//	ABORTCHECK;
  }
} 

/* NOTE: the above use of /fontdimen0 to access the checksum is a kludge */
/* In future would be better to do this by allocating one more slot for */
/* for parameters when a font is read rather than carry checksum separately */
/* The above gets the value byte order reversed ... 98/Oct/5 */

void zscansomethinginternal ( level , negative ) 
smallnumber level ; 
booleane negative ; 
{scansomethinginternal_regmem 
  halfword m  ; 
  integer p  ; 
  m = curchr ; 
  switch ( curcmd ) 
  {case 85 : 
    {
      scancharnum () ; 
	  ABORTCHECK;
      if ( m == (hash_size + 2907) ) 
      {
	curval = eqtb [ (hash_size + 2907) + curval ] .hh .v.RH ; 
	curvallevel = 0 ; 
      } 
      else if ( m < (hash_size + 2907) ) 
      {
	curval = eqtb [ m + curval ] .hh .v.RH ; 
	curvallevel = 0 ; 
      } 
      else {
	  
	curval = eqtb [ m + curval ] .cint ; 
	curvallevel = 0 ; 
      } 
    } 
    break ; 
  case 71 : 
  case 72 : 
  case 86 : 
  case 87 : 
  case 88 : 
    if ( level != 5 ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 661 ) ;		/* Missing number, treated as zero */
      } 
      {
	helpptr = 3 ; 
	helpline [ 2 ] = 662 ; /* A number should have been here; I inserted `0'. */
	helpline [ 1 ] = 663 ; /* (If you can't figure out why I needed to see a number, */
	helpline [ 0 ] = 664 ; /* look up `weird error' in the index to The TeXbook.) */
      } 
      backerror () ; 
	  ABORTCHECK;
      {
		  curval = 0 ; 
		  curvallevel = 1 ; 
      } 
    } 
    else if ( curcmd <= 72 ) 
    {
      if ( curcmd < 72 ) 
      {
	scaneightbitint () ; 
	ABORTCHECK;
	m = (hash_size + 1322) + curval ; 
      } 
      {
	curval = eqtb [ m ] .hh .v.RH ; 
	curvallevel = 5 ; 
      } 
    } 
    else {
	
      backinput () ; 
      scanfontident () ; 
	  ABORTCHECK;
      {
/*	curval = (hash_size + 524) + curval ;  */
	curval = (hash_size + hash_extra + 524) + curval ; /* 96/Jan/10 */
	curvallevel = 4 ; 
      } 
    } 
    break ; 
  case 73 : 
    {
      curval = eqtb [ m ] .cint ; 
      curvallevel = 0 ; 
    } 
    break ; 
  case 74 : 
    {
      curval = eqtb [ m ] .cint ; 
      curvallevel = 1 ; 
    } 
    break ; 
  case 75 : 
    {
      curval = eqtb [ m ] .hh .v.RH ; 
      curvallevel = 2 ; 
    } 
    break ; 
  case 76 : 
    {
      curval = eqtb [ m ] .hh .v.RH ; 
      curvallevel = 3 ; 
    } 
    break ; 
  case 79 : 
    if ( abs ( curlist .modefield ) != m ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 677 ) ;		/* Improper  */
      } 
      printcmdchr ( 79 , m ) ;	/* O */
      {
	helpptr = 4 ; 
	helpline [ 3 ] = 678 ; /* You can refer to \spacefactor only in horizontal mode; */
	helpline [ 2 ] = 679 ; /* you can refer to \prevdepth only in vertical mode; and */
	helpline [ 1 ] = 680 ; /* neither of these is meaningful inside \write. So */
	helpline [ 0 ] = 681 ; /* I'm forgetting what you said and using zero instead. */
      } 
      error () ; 
	  ABORTCHECK;
      if ( level != 5 )  {
		  curval = 0 ; 
		  curvallevel = 1 ; 
      } 
      else {
	  
	curval = 0 ; 
	curvallevel = 0 ; 
      } 
    } 
    else if ( m == 1 ) 
    {
      curval = curlist .auxfield .cint ; 
      curvallevel = 1 ; 
    } 
    else {
	
      curval = curlist .auxfield .hh .v.LH ; 
      curvallevel = 0 ; 
    } 
    break ; 
  case 80 : 
    if ( curlist .modefield == 0 ) 
    {
      curval = 0 ; 
      curvallevel = 0 ; 
    } 
    else {
	
      nest [ nestptr ] = curlist ; 
      p = nestptr ; 
      while ( abs ( nest [ p ] .modefield ) != 1 ) decr ( p ) ; 
      {
	curval = nest [ p ] .pgfield ; 
	curvallevel = 0 ; 
      } 
    } 
    break ; 
  case 82 : 
    {
      if ( m == 0 ) 
      curval = deadcycles ; 
      else curval = insertpenalties ; 
      curvallevel = 0 ; 
    } 
    break ; 
  case 81 : 
    {
      if ( ( pagecontents == 0 ) && ( ! outputactive ) ) 
      if ( m == 0 ) 
      curval = 1073741823L ;	/* 2^30 - 1 */
      else curval = 0 ; 
      else curval = pagesofar [ m ] ; 
      curvallevel = 1 ; 
    } 
    break ; 
  case 84 : 
    {
      if ( eqtb [ (hash_size + 1312) ] .hh .v.RH == 0 ) 
      curval = 0 ; 
      else curval = mem [ eqtb [ (hash_size + 1312) ] .hh .v.RH ] .hh .v.LH ; 
      curvallevel = 0 ; 
    } 
    break ; 
  case 83 : 
    {
      scaneightbitint () ; 
	  ABORTCHECK;
      if ( eqtb [ (hash_size + 1578) + curval ] .hh .v.RH == 0 ) 
      curval = 0 ; 
      else curval = mem [ eqtb [ (hash_size + 1578) + curval ] .hh .v.RH + m ] .cint ; 
      curvallevel = 1 ; 
    } 
    break ; 
  case 68 : 
  case 69 : 
    {
      curval = curchr ; 
      curvallevel = 0 ; 
    } 
    break ; 
  case 77 : 
    {
      findfontdimen ( false ) ; 
      fontinfo [ fmemptr ] .cint = 0 ; 
      {
	curval = fontinfo [ curval ] .cint ; 
	curvallevel = 1 ; 
      } 
    } 
    break ; 
  case 78 : 
    {
      scanfontident () ; 
	  ABORTCHECK;
      if ( m == 0 ) {
	curval = hyphenchar [ curval ] ; 
	curvallevel = 0 ; 
      } 
      else {
	  
	curval = skewchar [ curval ] ; 
	curvallevel = 0 ; 
      } 
    } 
    break ; 
  case 89 : 
    {
      scaneightbitint () ; 
	  ABORTCHECK;
      switch ( m ) 
      {case 0 : 
	curval = eqtb [ (hash_size + 3218) + curval ] .cint ; 
	break ; 
      case 1 : 
	curval = eqtb [ (hash_size + 3751) + curval ] .cint ; 
	break ; 
      case 2 : 
	curval = eqtb [ (hash_size + 800) + curval ] .hh .v.RH ; 
	break ; 
      case 3 : 
	curval = eqtb [ (hash_size + 1056) + curval ] .hh .v.RH ; 
	break ; 
      } 
      curvallevel = m ; 
    } 
    break ; 
  case 70 : 
    if ( curchr > 2 ) 
    {
      if ( curchr == 3 ) 
      curval = line ; 
      else curval = lastbadness ; 
      curvallevel = 0 ; 
    } 
    else {
	
      if ( curchr == 2 ) 
      curval = 0 ; 
      else curval = 0 ; 
      curvallevel = curchr ; 
      if ( ! ( curlist .tailfield >= himemmin ) && ( curlist .modefield != 0 ) 
      ) 
      switch ( curchr ) 
      {case 0 : 
	if ( mem [ curlist .tailfield ] .hh.b0 == 12 ) 
	curval = mem [ curlist .tailfield + 1 ] .cint ; 
	break ; 
      case 1 : 
	if ( mem [ curlist .tailfield ] .hh.b0 == 11 ) 
	curval = mem [ curlist .tailfield + 1 ] .cint ; 
	break ; 
      case 2 : 
	if ( mem [ curlist .tailfield ] .hh.b0 == 10 ) 
	{
	  curval = mem [ curlist .tailfield + 1 ] .hh .v.LH ; 
	  if ( mem [ curlist .tailfield ] .hh.b1 == 99 ) 
	  curvallevel = 3 ; 
	} 
	break ; 
      } 
      else if ( ( curlist .modefield == 1 ) && ( curlist .tailfield == curlist 
      .headfield ) ) 
      switch ( curchr ) 
      {case 0 : 
	curval = lastpenalty ; 
	break ; 
      case 1 : 
	curval = lastkern ; 
	break ; 
      case 2 : 
/*	if ( lastglue != 262143L )  */	/* NO! */
	if ( lastglue != emptyflag ) 
	curval = lastglue ; 
	break ; 
      } 
    } 
    break ; 
    default: 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 682 ) ;		/* You can't use ` */
      } 
      printcmdchr ( curcmd , curchr ) ; 
      print ( 683 ) ;	/* ' after  */
      printesc ( 534 ) ; /* the */
      {
	helpptr = 1 ; 
	helpline [ 0 ] = 681 ; /* I'm forgetting what you said and using zero instead. */
      } 
      error () ; 
	  ABORTCHECK;
      if ( level != 5 ) {
		  curval = 0 ; 
		  curvallevel = 1 ; 
      } 
      else {
		  curval = 0 ; 
		  curvallevel = 0 ; 
      } 
    } 
    break ; 
  } 
  while ( curvallevel > level ) {
      
    if ( curvallevel == 2 ) 
		curval = mem [ curval + 1 ] .cint ; 
    else if ( curvallevel == 3 ) {
		muerror () ; 
		ABORTCHECK;
	}
    decr ( curvallevel ) ; 
  } 
  if ( negative ) 
  if ( curvallevel >= 2 ) 
  {
    curval = newspec ( curval ) ; 
    {
      mem [ curval + 1 ] .cint = - (integer) mem [ curval + 1 ] .cint ; 
      mem [ curval + 2 ] .cint = - (integer) mem [ curval + 2 ] .cint ; 
      mem [ curval + 3 ] .cint = - (integer) mem [ curval + 3 ] .cint ; 
    } 
  } 
  else curval = - (integer) curval ; 
  else if ( ( curvallevel >= 2 ) && ( curvallevel <= 3 ) ) 
  incr ( mem [ curval ] .hh .v.RH ) ; 
} 

/*****************************************************************************/

/* Moved here to avoid question about pragma optimize 96/Sep/12 */

#pragma optimize ("a", off) 

void getnext ( ) 
{/* 20 25 21 26 40 10 */ getnext_regmem 
  integer k  ; 
  halfword t  ; 
/*  char cat  ;	*/		/* make this an int ? */
  int cat  ;			/* make this an int ? 95/Jan/7 */
  ASCIIcode c, cc  ; 
  char d  ; 

lab20:
  ABORTCHECK;
  curcs = 0 ; 
  if ( curinput .statefield != 0 )  {
    lab25: if ( curinput .locfield <= curinput .limitfield )  {
      curchr = buffer [ curinput .locfield ] ; 
      incr ( curinput .locfield ) ; 
      lab21: curcmd = eqtb [ (hash_size + 1883) + curchr ] .hh .v.RH ; 
      switch ( curinput .statefield + curcmd ) 
      {case 10 : 
      case 26 : 
      case 42 : 
      case 27 : 
      case 43 : 
	goto lab25 ; 
	break ; 
      case 1 : 
      case 17 : 
      case 33 : 
	{
	  if ( curinput .locfield > curinput .limitfield ) 
	  curcs = 513 ; 
	  else {
	      
	    lab26: k = curinput .locfield ; 
	    curchr = buffer [ k ] ; 
	    cat = eqtb [ (hash_size + 1883) + curchr ] .hh .v.RH ; 
	    incr ( k ) ; 
	    if ( cat == 11 ) 
	    curinput .statefield = 17 ; 
	    else if ( cat == 10 ) 
	    curinput .statefield = 17 ; 
	    else curinput .statefield = 1 ; 
	    if ( ( cat == 11 ) && ( k <= curinput .limitfield ) ) 
	    {
	      do {
		  curchr = buffer [ k ] ; 
		cat = eqtb [ (hash_size + 1883) + curchr ] .hh .v.RH ; 
		incr ( k ) ; 
	      } while ( ! ( ( cat != 11 ) || ( k > curinput .limitfield ) ) ) 
	      ; 
	      {
		if ( buffer [ k ] == curchr ) 
		if ( cat == 7 ) 
		if ( k < curinput .limitfield ) 
		{
		  c = buffer [ k + 1 ] ; 
		  if ( c < 128 ) 
		  {
		    d = 2 ; 
		    if ( ( ( ( c >= 48 ) && ( c <= 57 ) ) || ( ( c >= 97 ) && 
		    ( c <= 102 ) ) ) ) 
		    if ( k + 2 <= curinput .limitfield ) 
		    {
		      cc = buffer [ k + 2 ] ; 
		      if ( ( ( ( cc >= 48 ) && ( cc <= 57 ) ) || ( ( cc >= 97 
		      ) && ( cc <= 102 ) ) ) ) 
		      incr ( d ) ; 
		    } 
		    if ( d > 2 ) 
		    {
		      if ( c <= 57 ) 
		      curchr = c - 48 ; 
		      else curchr = c - 87 ; 
		      if ( cc <= 57 ) 
		      curchr = 16 * curchr + cc - 48 ; 
		      else curchr = 16 * curchr + cc - 87 ; 
		      buffer [ k - 1 ] = curchr ; 
		    } 
		    else if ( c < 64 ) 
		    buffer [ k - 1 ] = c + 64 ; 
		    else buffer [ k - 1 ] = c - 64 ; 
		    curinput .limitfield = curinput .limitfield - d ; 
		    first = first - d ; 
		    while ( k <= curinput .limitfield ) {
			
		      buffer [ k ] = buffer [ k + d ] ; 
		      incr ( k ) ; 
		    } 
		    goto lab26 ; 
		  } 
		} 
	      } 
	      if ( cat != 11 ) 
	      decr ( k ) ; 
	      if ( k > curinput .locfield + 1 ) 
	      {
		curcs = idlookup ( curinput .locfield , k - curinput .locfield 
		) ; 
		curinput .locfield = k ; 
		goto lab40 ; 
	      } 
	    } 
	    else {
		
	      if ( buffer [ k ] == curchr ) 
	      if ( cat == 7 ) 
	      if ( k < curinput .limitfield ) 
	      {
		c = buffer [ k + 1 ] ; 
		if ( c < 128 )							/* ? */
		{
		  d = 2 ; 
		  if ( ( ( ( c >= 48 ) && ( c <= 57 ) ) || ( ( c >= 97 ) && ( 
		  c <= 102 ) ) ) ) 
		  if ( k + 2 <= curinput .limitfield ) 
		  {
		    cc = buffer [ k + 2 ] ; 
		    if ( ( ( ( cc >= 48 ) && ( cc <= 57 ) ) || ( ( cc >= 97 ) 
		    && ( cc <= 102 ) ) ) ) 
		    incr ( d ) ; 
		  } 
		  if ( d > 2 ) 
		  {
		    if ( c <= 57 ) 
				curchr = c - 48 ; 
		    else curchr = c - 87 ; 
		    if ( cc <= 57 )					/* cc may be used without ... */
				curchr = 16 * curchr + cc - 48 ; 
		    else curchr = 16 * curchr + cc - 87 ; 
		    buffer [ k - 1 ] = curchr ; 
		  } 
		  else if ( c < 64 ) 
			  buffer [ k - 1 ] = c + 64 ; 
		  else buffer [ k - 1 ] = c - 64 ; 
		  curinput .limitfield = curinput .limitfield - d ; 
		  first = first - d ; 
		  while ( k <= curinput .limitfield ) {
		    buffer [ k ] = buffer [ k + d ] ; 
		    incr ( k ) ; 
		  } 
		  goto lab26 ; 
		} 
	      } 
	    } 
/*   cur_cs:=single_base+buffer[loc]; incr(loc); */
	    curcs = 257 + buffer [ curinput .locfield ] ; 
	    incr ( curinput .locfield ) ; 
	  } 
	  lab40: curcmd = eqtb [ curcs ] .hh.b0 ; 
	  curchr = eqtb [ curcs ] .hh .v.RH ; 
	  if ( curcmd >= 113 ) {
		  checkoutervalidity () ;
		  ABORTCHECK;
	  }
	} 
	break ; 
      case 14 : 
      case 30 : 
      case 46 : 
	{
	  curcs = curchr + 1 ; 
	  curcmd = eqtb [ curcs ] .hh.b0 ; 
	  curchr = eqtb [ curcs ] .hh .v.RH ; 
	  curinput .statefield = 1 ; 
	  if ( curcmd >= 113 ) {
		  checkoutervalidity () ;
		  ABORTCHECK;
	  }
	} 
	break ; 
      case 8 : 
      case 24 : 
      case 40 : 
	{
	  if ( curchr == buffer [ curinput .locfield ] ) 
	  if ( curinput .locfield < curinput .limitfield ) 
	  {
	    c = buffer [ curinput .locfield + 1 ] ; 
	    if ( c < 128 ) 
	    {
	      curinput .locfield = curinput .locfield + 2 ; 
	      if ( ( ( ( c >= 48 ) && ( c <= 57 ) ) || ( ( c >= 97 ) && ( c <= 
	      102 ) ) ) ) 
	      if ( curinput .locfield <= curinput .limitfield ) 
	      {
		cc = buffer [ curinput .locfield ] ; 
		if ( ( ( ( cc >= 48 ) && ( cc <= 57 ) ) || ( ( cc >= 97 ) && ( 
		cc <= 102 ) ) ) ) 
		{
		  incr ( curinput .locfield ) ; 
		  if ( c <= 57 ) 
		  curchr = c - 48 ; 
		  else curchr = c - 87 ; 
		  if ( cc <= 57 ) 
		  curchr = 16 * curchr + cc - 48 ; 
		  else curchr = 16 * curchr + cc - 87 ; 
		  goto lab21 ; 
		} 
	      } 
	      if ( c < 64 ) 
	      curchr = c + 64 ; 
	      else curchr = c - 64 ; 
	      goto lab21 ; 
	    } 
	  } 
	  curinput .statefield = 1 ; 
	} 
	break ; 
      case 16 : 
      case 32 : 
      case 48 : 
	{
	  {
	    if ( interaction == 3 ) 
	    ; 
	    printnl ( 262 ) ;	/* !  */
	    print ( 610 ) ;		/* Text line contains an invalid character */
	  } 
	  {
	    helpptr = 2 ; 
	    helpline [ 1 ] = 611 ; /* A funny symbol that I can't read has just been input. */
	    helpline [ 0 ] = 612 ; /* Continue, and I'll forget that it ever happened. */
	  } 
	  deletionsallowed = false ; 
	  error () ; 
	  ABORTCHECK;
	  deletionsallowed = true ; 
	  goto lab20 ; 
	} 
	break ; 
      case 11 : 
	{
	  curinput .statefield = 17 ; 
	  curchr = 32 ; 
	} 
	break ; 
      case 6 : 
	{
	  curinput .locfield = curinput .limitfield + 1 ; 
	  curcmd = 10 ; 
	  curchr = 32 ; 
	} 
	break ; 
      case 22 : 
      case 15 : 
      case 31 : 
      case 47 : 
	{
	  curinput .locfield = curinput .limitfield + 1 ; 
	  goto lab25 ; 
	} 
	break ; 
      case 38 : 
	{
	  curinput .locfield = curinput .limitfield + 1 ; 
	  curcs = parloc ; 
	  curcmd = eqtb [ curcs ] .hh.b0 ; 
	  curchr = eqtb [ curcs ] .hh .v.RH ; 
	  if ( curcmd >= 113 ) {
		  checkoutervalidity () ;
		  ABORTCHECK;
	  }
	} 
	break ; 
      case 2 : 
	incr ( alignstate ) ; 
	break ; 
      case 18 : 
      case 34 : 
	{
	  curinput .statefield = 1 ; 
	  incr ( alignstate ) ; 
	} 
	break ; 
      case 3 : 
	decr ( alignstate ) ; 
	break ; 
      case 19 : 
      case 35 : 
	{
	  curinput .statefield = 1 ; 
	  decr ( alignstate ) ; 
	} 
	break ; 
      case 20 : 
      case 21 : 
      case 23 : 
      case 25 : 
      case 28 : 
      case 29 : 
      case 36 : 
      case 37 : 
      case 39 : 
      case 41 : 
      case 44 : 
      case 45 : 
	curinput .statefield = 1 ; 
	break ; 
	default: 
	; 
	break ; 
      } 
    } 
    else {
      curinput .statefield = 33 ; 
      if ( curinput .namefield > 17 )  {
		  incr ( line ) ; 
		  first = curinput .startfield ; 
		  if ( ! forceeof ) {
			  if ( inputln ( inputfile [ curinput .indexfield ] , true ) ) {
				  firmuptheline () ;
				  ABORTCHECK;
			  }
			  else forceeof = true ; 
		  } 
		  if ( forceeof ) {
			  printchar ( 41 ) ;		/* ) */
			  decr ( openparens ) ; 
#ifndef _WINDOWS
			  fflush ( stdout ) ; 
#endif
			  forceeof = false ; 
			  endfilereading () ; 
			  checkoutervalidity () ; 
			  ABORTCHECK;
			  goto lab20 ; 
		  } 
		  if ( ( eqtb [ (hash_size + 3211) ] .cint < 0 ) ||
			   ( eqtb [ (hash_size + 3211) ] .cint > 255 ) ) 
			  decr ( curinput .limitfield ) ; 
/*		long to unsigned char ... */
		  else buffer [ curinput .limitfield ] = eqtb [ (hash_size + 3211) ] .cint ; 
		  first = curinput .limitfield + 1 ; 
		  curinput .locfield = curinput .startfield ; 
	  } 
	  else {
		  if ( ! ( curinput .namefield == 0 ) ) {
			  curcmd = 0 ; 
			  curchr = 0 ; 
			  return ; 
		  } 
		  if ( inputptr > 0 ) {
			  endfilereading () ; 
			  goto lab20 ; 
		  } 
		  if ( selector < 18 ) 	openlogfile () ; 
		  if ( interaction > 1 ) {
			  if ( ( eqtb [ (hash_size + 3211) ] .cint < 0 ) ||
				   ( eqtb [ (hash_size + 3211) ] .cint > 255 ) 
				 ) 
				  incr ( curinput .limitfield ) ; 
			  if ( curinput .limitfield == curinput .startfield ) 
				  printnl ( 613 ) ;		/* (Please type a command or say `\end') */
			  println () ; 
			  first = curinput .startfield ; 
			  {
				  ; 
				  print ( 42 ) ;		/* * */
				  terminput ( 42, 0 ) ; 
				  ABORTCHECK;
			  } 
			  curinput .limitfield = last ; 
			  if ( ( eqtb [ (hash_size + 3211) ] .cint < 0 ) ||
				   ( eqtb [ (hash_size + 3211) ] .cint > 255 ) 
				 ) 
				  decr ( curinput .limitfield ) ; 
/*		long to unsigned char ... */
			  else buffer [ curinput .limitfield ] = eqtb [ (hash_size + 3211) ] .cint ; 
			  first = curinput .limitfield + 1 ; 
			  curinput .locfield = curinput .startfield ; 
		  } 
		  else {
			  fatalerror ( 614 ) ;	/* *** (job aborted, no legal \end found) */
			  return;			// abortflag set
		  }
	  } 
	  {
		  if ( interrupt != 0 ) {
			  pauseforinstructions () ;
			  ABORTCHECK;
		  }
      } 
      goto lab25 ; 
    } 
  } 
  else if ( curinput .locfield != 0 ) 
  {
    t = mem [ curinput .locfield ] .hh .v.LH ; 
    curinput .locfield = mem [ curinput .locfield ] .hh .v.RH ; 
    if ( t >= 4095 ) 
    {
      curcs = t - 4095 ; 
      curcmd = eqtb [ curcs ] .hh.b0 ; 
      curchr = eqtb [ curcs ] .hh .v.RH ; 
      if ( curcmd >= 113 ) 
      if ( curcmd == 116 ) 
      {
	curcs = mem [ curinput .locfield ] .hh .v.LH - 4095 ; 
	curinput .locfield = 0 ; 
	curcmd = eqtb [ curcs ] .hh.b0 ; 
	curchr = eqtb [ curcs ] .hh .v.RH ; 
	if ( curcmd > 100 ) 
	{
	  curcmd = 0 ; 
	  curchr = 257 ; 
	} 
      } 
      else {
		  checkoutervalidity () ;
		  ABORTCHECK;
	  }
    } 
    else {
	
      curcmd = t / 256 ; 
/*      curcmd = t >> 8 ; */	/* top 8 bits */
      curchr = t % 256 ; 
/*      curchr = t & 255 ; */	/* last 8 bits */
      switch ( curcmd ) 
      {case 1 : 
	incr ( alignstate ) ; 
	break ; 
      case 2 : 
	decr ( alignstate ) ; 
	break ; 
      case 5 : 
	{
	  begintokenlist ( paramstack [ curinput .limitfield + curchr - 1 ] , 
	  0 ) ; 
	  goto lab20 ; 
	} 
	break ; 
	default: 
	; 
	break ; 
      } 
    } 
  } 
  else {
    endtokenlist () ; 
	ABORTCHECK;
    goto lab20 ; 
  } 
  if ( curcmd <= 5 ) 
  if ( curcmd >= 4 ) 
  if ( alignstate == 0 ) 
  {
    if ( scannerstatus == 4 ) {
		fatalerror ( 592 ) ;	/* (interwoven alignment preambles are not allowed) */
		return;			// abortflag set
	}

    curcmd = mem [ curalign + 5 ] .hh .v.LH ; 
    mem [ curalign + 5 ] .hh .v.LH = curchr ; 
    if ( curcmd == 63 ) 
    begintokenlist ( memtop - 10 , 2 ) ; 
    else begintokenlist ( mem [ curalign + 2 ] .cint , 2 ) ; 
    alignstate = 1000000L ; 
    goto lab20 ; 
  } 
} 

#pragma optimize ("", on)							/* 96/Sep/12 */

/*****************************************************************************/