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

/* #pragma optimize("a", off) */				/* 98/Dec/10 experiment */

#ifdef IGNORED
static void winerror (char *message) {
	(void) MessageBox(NULL, message, "YandYTeX.DLL", MB_ICONSTOP | MB_OK);
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* selector 1 - 15 => write to specified output file, */
/* selector 16 => null 17 => screen, */
/* selector 18 => logfile, 19 => logfile + screen, */
/* selector 20 => null 21 => null */

// print newline

void println ( void ) 
{println_regmem 
   switch ( selector ) {
	   case 19 :	// log file and on screen
	   {
		   showchar('\n');
		   termoffset = 0 ; 
		   (void) putc ('\n',  logfile );
		   fileoffset = 0 ; 
	   } 
	   break ; 
	   case 18 :	// log file only
	   {
		   (void) putc ('\n',  logfile );
		   fileoffset = 0 ; 
	   } 
	   break ; 
	   case 17 :	// on screen only
	   {
		   showchar('\n');
		   termoffset = 0 ; 
	   } 
	   break ; 
	   case 16 : 
	   case 20 : 
	   case 21 : 
		   ; 
		   break ; 
	   default: 
		   (void) putc ('\n',  writefile [ selector ] );
		   break ; 
	} 
} 

void zprintchar ( s ) 
ASCIIcode s ; 
{/* 10 */ printchar_regmem 
    if ( s == eqtb [ (hash_size + 3212) ] .cint ) 
		if ( selector < 20 ) {
			println () ; 
			return ; 
		} 
	switch ( selector ) {
		case 19 :	//log file and on screen
		{
			(void) showchar ( Xchr(s) );
			incr ( termoffset ) ; 
			(void) putc ( Xchr ( s ) ,  logfile );
			incr ( fileoffset ) ; 
			if ( termoffset == maxprintline ) {
				showchar('\n');
				termoffset = 0 ; 
			} 
			if ( fileoffset == maxprintline ) {
				(void) putc ('\n',  logfile );
				fileoffset = 0 ; 
			} 
		} 
		break ; 
		case 18 : // log file only
		{
			(void) putc ( Xchr ( s ) ,  logfile );
			incr ( fileoffset ) ; 
			if ( fileoffset == maxprintline ) println () ; 
		} 
		break ; 
		case 17 : // on screen only
		{
			(void) showchar ( Xchr( s ));
			incr ( termoffset ) ; 
			if ( termoffset == maxprintline ) println () ; 
		} 
		break ; 
		case 16 : 
			; 
			break ; 
		case 20 : 
			if ( tally < trickcount ) 
				trickbuf [ tally % errorline ] = s ; 
			break ; 
		case 21 : 
		{
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
			if ( poolptr + 1 > currentpoolsize )	{	
				strpool = reallocstrpool (incrementpoolsize);	/* 94/Jan/24 */
			}
			if ( poolptr < currentpoolsize )					/* 94/Jan/24 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
				if ( poolptr < poolsize ) 
#endif
				{
					strpool [ poolptr ] = s ; 
					incr ( poolptr ) ; 
				} 
		} 
		break ; 
		default: 
			(void) putc ( Xchr ( s ) ,  writefile [ selector ] );
			break ; 
	}	/* end of switch(selector) */
	incr ( tally ) ; 
} 

/* This could be made more efficient using fputs ? ... bkph */
void zprint ( integer s ) 
{/* 10 */ print_regmem 
    poolpointer j  ; 
	integer nl  ; 
	if ( s >= strptr ) 	s = 259 ;				/* ??? */
	else if ( s < 256 ) 
		if ( s < 0 )  s = 259 ; 				/* ??? */
		else {
			if ( selector > 20 ) {
				printchar ( s ) ; 
				return ; 
			} 
			if ( ( s == eqtb [ (hash_size + 3212) ] .cint ) ) 
				if ( selector < 20 ) {
					println () ; 
					return ; 
				} 
			nl = eqtb [ (hash_size + 3212) ] .cint ;	/* save eol */
			eqtb [ (hash_size + 3212) ] .cint = -1 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*			if (!showinhex && s < 256)  */			/* show control chars also ?? */
			if (! showinhex && s < 256 && s >= 32) {			/* 94/Jan/26 */
/*				following added 1996/Jan/20 */
				if (showindos && s > 127) {			/* translate ANSI to DOS 850 */
					if (wintodos[s-128] > 0) printchar (wintodos[s-128]);
					else {							/* print in hex after all */
						j = strstart [ s ] ; 
						while ( j < strstart [ s + 1 ] ) {
							printchar ( strpool [ j ] ) ; 
							incr ( j ) ; 
						} 
					}
				}
				else printchar ( s );				/* don't translate to hex */
			}
			else {									/* not just a character */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
				j = strstart [ s ] ; 
				while ( j < strstart [ s + 1 ] ) {
					printchar ( strpool [ j ] ) ; 
					incr ( j ) ; 
				} 
			}
			eqtb [ (hash_size + 3212) ] .cint = nl ;	/* restore eol */
			return ; 
		}
/*  we get here with s > 256 - i.e. not a single character */
	j = strstart [ s ] ; 
	while ( j < strstart [ s + 1 ] ) {
		printchar ( strpool [ j ] ) ; 
		incr ( j ) ; 
	}
} 

// print string number s from string pool by calling zprint

void zslowprint ( integer s ) 
{slowprint_regmem 
    poolpointer j  ; 
	if ( ( s >= strptr ) || ( s < 256 ) ) print ( s ) ; 
	else {
		j = strstart [ s ] ; 
		while ( j < strstart [ s + 1 ] ) {
//			if (strpool [ j ] >= 128) print ( strpool [ j ] -128); // debugging only
//			if (strpool [ j ] == 0) print ( 36 ); // debugging only
			print ( strpool [ j ] ) ; 
			incr ( j ) ; 
		} 
	} 
} 

// print newline followed by string number s (unless at start of line)

void zprintnl ( strnumber s ) 
{printnl_regmem 
   if ( ( ( termoffset > 0 ) && ( odd ( selector ) ) ) ||
		( ( fileoffset > 0 ) && ( selector >= 18 ) ) ) 	println () ; 
	print ( s ) ; 
} 

// print string number s preceded by escape character

void zprintesc ( strnumber s ) 
{printesc_regmem 
    integer c  ; 
	c = eqtb [ (hash_size + 3208) ] .cint ; 
	if ( c >= 0 ) 
		if ( c < 256 ) 	print ( c ) ; 
	slowprint ( s ) ; 
} 

// print the digits in the array dig[...]

void zprintthedigs ( k ) 
eightbits k ; 
{printthedigs_regmem 
  while ( k > 0 ) {
    decr ( k ) ; 
    if ( dig [ k ] < 10 ) 
    printchar ( 48 + dig [ k ] ) ; 
    else printchar ( 55 + dig [ k ] ) ; 
  } 
} 

// print integer n

void zprintint ( n ) 
integer n ; 
{printint_regmem 
  char k  ; 
  integer m  ; 
  k = 0 ; 
  if ( n < 0 ) 
  {
    printchar ( 45 ) ;			/* - */
    if ( n > -100000000L ) 
    n = - (integer) n ; 
    else {
      m = -1 - n ; 
      n = m / 10 ; 
      m = ( m % 10 ) + 1 ; 
      k = 1 ; 
      if ( m < 10 ) 
/*      dig [ 0 ] = m ; */			/* keep compiler happy */
      dig [ 0 ] = (char) m ; 
      else {
		  dig [ 0 ] = 0 ; 
		  incr ( n ) ; 
      } 
    } 
  } 
  do {
/*	  dig [ k ] = n % 10 ;  */		/* keep compiler happy */
	  dig [ k ] = (char) (n % 10) ; 
	  n = n / 10 ; 
	  incr ( k ) ; 
  } while ( ! ( n == 0 ) ) ; 
  printthedigs ( k ) ; 
} 

// print control sequence

void zprintcs ( p ) 
integer p ; 
{printcs_regmem 
  if ( p < 514 )				/* if p < hash_base then ... p.262 */
  if ( p >= 257 )				/* if p > single_base then ... p.262 */
  if ( p == 513 )				/* if p = null_cs then ... p.262 */
  {
    printesc ( 501 ) ;			/* csname */
    printesc ( 502 ) ;			/* endcsname */
  } 
  else {
    printesc ( p - 257 ) ;		/* p - single_base */
/*  if cat_code(p - single_base) = letter then ... p.262 */
    if ( eqtb [ (hash_size + 1883) + p - 257 ] .hh .v.RH == 11 ) 
		printchar ( 32 ) ;			/*   */
  } 
  else if ( p < 1 ) 
	  printesc ( 503 ) ;				/* IMPOSSIBLE */
  else print ( p - 1 ) ; 
  else if ( p >= (hash_size + 781) )	/* undefined_control_sequence */
	  printesc ( 503 ) ;				/* IMPOSSIBLE */
  else if ( ( hash [ p ] .v.RH >= strptr ) ) 
	  printesc ( 504 ) ;				/* NONEXISTENT */
  else {
    printesc ( hash [ p ] .v.RH ) ; 
    printchar ( 32 ) ;			/*    */
  } 
} 

void zsprintcs ( p ) 
halfword p ; 
{sprintcs_regmem 
  if ( p < 514 )				/* if p < hash_base then ... p.263 */
  if ( p < 257 )				/* if p < single_base then ... p.263 */
	  print ( p - 1 ) ;			/* print (p - active_base); */
  else if ( p < 513 )			/* else if p < null_cs then ... */
	  printesc ( p - 257 ) ;	/* print (p - single_base); */
  else {
    printesc ( 501 ) ;			/* csname */
    printesc ( 502 ) ;			/* endcsname */
  } 
  else printesc ( hash [ p ] .v.RH ) ; 
} 

/* ! I can't find file `  c:/foo/  accents  .tex  '. */
void zprintfilename ( n , a , e ) 
integer n ; 
integer a ; 
integer e ; 
{printfilename_regmem 
/*  sprintf(logline, "\na %d n %d e %d\n", a, n, e); */
/*	showline(logline, 0); */
//  printchar ( 33 ) ;  // debugging only
  slowprint ( a ) ; 
//  printchar ( 33 ) ;  // debugging only
  slowprint ( n ) ; 
//  printchar ( 33 ) ;  // debugging only
  slowprint ( e ) ; 
//  printchar ( 33 ) ;  // debugging only
}

void zprintsize ( s ) 
integer s ; 
{printsize_regmem 
  if ( s == 0 ) printesc ( 409 ) ;			/* textfont */
  else if ( s == 16 )  printesc ( 410 ) ;	/* scriptfont */
  else printesc ( 411 ) ;					/* scriptscriptfont */
} 

void zprintwritewhatsit ( s , p ) 
strnumber s ; 
halfword p ; 
{printwritewhatsit_regmem 
  printesc ( s ) ; 
  if ( mem [ p + 1 ] .hh .v.LH < 16 ) 
	  printint ( mem [ p + 1 ] .hh .v.LH ) ; 
  else if ( mem [ p + 1 ] .hh .v.LH == 16 ) 
	  printchar ( 42 ) ;		/* * */
  else printchar ( 45 ) ;		/* - */
} 

#ifdef DEBUG
#endif /* DEBUG */

// called from itex.c and tex0.c only  NASTY NASTY!
// now uses uses non-local goto (longjmp) 1999/Nov/7

void jumpout ( ) 
{jumpout_regmem 
	closefilesandterminate () ; 
	{
		int code;
#ifndef _WINDOWS
		fflush ( stdout ) ; 
#endif
		readyalready = 0 ; 

		if (traceflag) showline("EXITING at JUMPOUT\n", 0);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
//		if (endit(history) != 0) history = 2;	/* 93/Dec/26 in local.c */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

//		abortflag++;				// TURN OFF THE POWER ???

		if ( ( history != 0 ) && ( history != 1 ) )	code = 1 ;
		else code = 0 ;
		uexit ( code ) ; 
//		longjmp(jumpbuffer, code+1);
	}
}

// deal with error by asking for user response 0-9, D, E, H, I, X, Q, R, S
// NOTE: this may JUMPOUT either via X, or because of too many errors

void error ( void ) 
{/* 22 10 */ error_regmem 
    ASCIIcode c  ; 
	integer s1, s2, s3, s4  ; 
	if ( history < 2 )  history = 2 ;

	printchar ( 46 ) ;	 	/* . */
	showcontext () ; 

	if ( interaction == 3 ) 
		while ( true ) {
lab22:					/* loop */
			ABORTCHECK;
			clearforerrorprompt () ; 
			{
				; 
				print ( 264 ) ;		/* ?  */
				terminput ( 264, helpptr ) ; 
				ABORTCHECK;
			} 
			if ( last == first ) return ;		// no input
			c = buffer [ first ] ;				// analyze first letter typed
			if ( c >= 97 )						// uppercase letter first
/*				c = c - 32 ;  */				/* keep compiler happy */
				c = (unsigned char) (c - 32) ; 
			switch ( c ) {
				case 48 :				/* 0 */
				case 49 :				/* 1 */
				case 50 : 
				case 51 : 
				case 52 : 
				case 53 : 
				case 54 : 
				case 55 : 
				case 56 : 
				case 57 :				/* 9 */
					if ( deletionsallowed ) {
						s1 = curtok ; 
						s2 = curcmd ; 
						s3 = curchr ; 
						s4 = alignstate ; 
						alignstate = 1000000L ; 
						OKtointerrupt = false ; 
						if ( ( last > first + 1 ) && ( buffer [ first + 1 ] >= 48 ) &&
							 ( buffer [ first + 1 ] <= 57 ) ) 
/*							c = c * 10 + buffer [ first + 1 ] - 48 * 11 ; */
							c = (unsigned char) (c * 10 + buffer [ first + 1 ] - 48 * 11) ; 
/*						else c = c - 48 ;  */	/* keep compiler happy */
						else c = (unsigned char) (c - 48);
						while ( c > 0 ) {
							gettoken () ; 
							decr ( c ) ; 
						} 
						curtok = s1 ; 
						curcmd = s2 ; 
						curchr = s3 ; 
						alignstate = s4 ; 
						OKtointerrupt = true ; 
						{
							helpptr = 2 ; 
							helpline [ 1 ] = 277 ; /* I have just deleted some text, as you asked. */
							helpline [ 0 ] = 278 ; /* You can now delete more, or insert, or whatever. */
						} 
						showcontext () ; 
						goto lab22 ;			/* loop again */
					} 
					break ; 
					;
#ifdef DEBUG
				case 68 :			/* D */
				{
					debughelp () ; 
					goto lab22 ;				/* loop again */
				} 
				break ; 
#endif /* DEBUG */
				case 69 :			/* E */
					if ( baseptr > 0 ) {
						editnamestart = strstart [ inputstack [ baseptr ] .namefield ] ; 
						editnamelength = strstart [ inputstack [ baseptr ] .namefield + 1 ] - 
										 strstart [ inputstack [ baseptr ] .namefield ] ; 
						editline = line ; 
						jumpout();
//						return ;			// can drop through now 99/Oct/20
					} 
					break ; 
				case 72 :			/* H */
				{
					if ( useerrhelp ) 
					{
						giveerrhelp () ; 
						useerrhelp = false ; 
					} 
					else {
						if ( helpptr == 0 ) 
						{
							helpptr = 2 ; 
							helpline [ 1 ] = 279 ; /* Sorry, I don't know how to help in this situation. */
							helpline [ 0 ] = 280 ; /* Maybe you should try asking a human? */
						} 
						do {
							decr ( helpptr ) ; 
							print ( helpline [ helpptr ] ) ; 
							println () ; 
						} while ( ! ( helpptr == 0 ) ) ; 
					} 
					{
						helpptr = 4 ; 
						helpline [ 3 ] = 281 ; /* Sorry, I already gave what help I could... */
						helpline [ 2 ] = 280 ; /* Maybe you should try asking a human? */
						helpline [ 1 ] = 282 ; /* An error might have occurred before I noticed any problems. */
						helpline [ 0 ] = 283 ; /* ``If all else fails, read the instructions.'' */
					} 
					goto lab22 ;	/* loop again */
				} 
				break ; 
				case 73 :			/* I */
				{
					beginfilereading () ; 
					if ( last > first + 1 ) 
					{
						curinput .locfield = first + 1 ; 
						buffer [ first ] = 32 ; 
					} 
					else {
						{
							; 
							print ( 276 ) ; /* insert> */
							terminput ( 276, 0 ) ; 
							ABORTCHECK;
						} 
						curinput .locfield = first ; 
					} 
					first = last ; 
					curinput .limitfield = last - 1 ; 
					return ; 
				} 
				break ; 
				case 81 :			/* Q, R, S */
				case 82 : 
				case 83 : 
				{
					errorcount = 0 ; 
					interaction = 0 + c - 81 ; /* Q = 0, R = 1, S = 2, T = 3 */
					print ( 271 ) ;			/* OK, entering  */
					switch ( c ) {
						case 81 :				/* Q */
						{
							printesc ( 272 ) ;	/* batchmode */
							decr ( selector ) ; 
						} 
						break ; 
						case 82 :				/* R */
							printesc ( 273 ) ;	/* nonstopmode */
							break ; 
						case 83 :				/* S */
							printesc ( 274 ) ;	/*  scrollmode */
							break ; 
					} 
					print ( 275 ) ;			/* ... */
					println () ; 
#ifndef _WINDOWS
					fflush ( stdout ) ; 
#endif
					return ; 
				} 
				break ; 
				case 88 :				/* X */
				{
					interaction = 2 ; 
					jumpout();
//					return ;			// can drop through now 99/Oct/20	  
				} 
				break ; 
				default: 
					; 
					break ; 
			}						/* end of switch analysing response character */
			{
				print ( 265 ) ;		/* Type <return> to proceed, S to scroll future error messages, */
				printnl ( 266 ) ;	/* R to run without stopping, Q to run quietly, */
				printnl ( 267 ) ;	/* I to insert something,  */
				if ( baseptr > 0 ) print ( 268 ) ;	/*  E to edit your file,  */
				if ( deletionsallowed ) printnl ( 269 ) ;	/* 1 or ... or 9 to ignore the next 1 to 9 tokens of input, */
				printnl ( 270 ) ;	/* H for help, X to quit. */
			} 
		}		/* end of while(true) loop */

	incr ( errorcount ) ; 
	if ( errorcount == 100 )  {
		printnl ( 263 ) ;	/* (That makes 100 errors; please try again.) */
		history = 3 ; 
		jumpout();
//		return ;			// can drop through now 99/Oct/20	  
	} 
	if ( interaction > 0 )  decr ( selector ) ; 
	if ( useerrhelp )  {
		println () ; 
		giveerrhelp () ; 
	} 
	else while ( helpptr > 0 ) {
		decr ( helpptr ) ; 
		printnl ( helpline [ helpptr ] ) ; 
	} 
	println () ; 
	if ( interaction > 0 ) incr ( selector ) ; 
	println () ; 
} 

void zfatalerror ( strnumber s ) 
{fatalerror_regmem 
   normalizeselector () ; 
	{
		if ( interaction == 3 ) 
			; 
		printnl ( 262 ) ;	/* ! */
		print ( 285 ) ;		/* Emergency stop */
	} 
	{
		helpptr = 1 ; 
		helpline [ 0 ] = s ;	// given string goes into help line
	} 
	{
		if ( interaction == 3 ) interaction = 2 ; 
		if ( logopened ) {
			error () ;
			ABORTCHECK;
		}
		;
#ifdef DEBUG
		if ( interaction > 0 ) debughelp () ; 
#endif /* DEBUG */
		history = 3 ; 
		jumpout();
//		return;			// can drop through now 99/Oct/20	  
	} 
}

void zoverflow ( strnumber s , integer n ) 
{overflow_regmem 
   normalizeselector () ; 
	{
		if ( interaction == 3 ) 
			; 
		printnl ( 262 ) ;	/* ! */
		print ( 286 ) ;		/* TeX capacity exceeded, sorry [ */
	} 
	print ( s ) ; 
	printchar ( 61 ) ;	/* '=' */
	printint ( n ) ; 
	printchar ( 93 ) ;	/* ']' */
	{
		helpptr = 2 ; 
		helpline [ 1 ] = 287 ; /* If you really absolutely need more capacity, */
		helpline [ 0 ] = 288 ; /*  you can ask a wizard to enlarge me. */
	} 
	if (! knuthflag) {		/*	Additional comments 98/Jan/5 */
	  if (s == 945 && n == triesize) {
		  sprintf(logline, "\n  (Maybe use -h=... on command line in ini-TeX)\n");
		  showline(logline, 0);
	  }
	  else if (s == 942 && n == hyphen_prime) {
		  sprintf(logline, "\n  (Maybe use -e=... on command line in ini-TeX)\n");
		  showline(logline, 0);
	  }
	}
	if ( interaction == 3 ) interaction = 2 ; 
	if ( logopened ) {
		error () ;
		ABORTCHECK;
	}
	;
#ifdef DEBUG
    if ( interaction > 0 ) debughelp () ; 
#endif /* DEBUG */
	history = 3 ; 
	jumpout();
//	return;			// can drop through now 99/Oct/20	  
}


void zconfusion ( strnumber s ) 
{confusion_regmem 
   normalizeselector () ; 
	if ( history < 2 )  {
		{
			if ( interaction == 3 ) 
				; 
			printnl ( 262 ) ; /* ! */
			print ( 289 ) ;	/* This can't happen ( */
		} 
		print ( s ) ; 
		printchar ( 41 ) ;	/* ) */
		{
			helpptr = 1 ; 
			helpline [ 0 ] = 290 ; /* I'm broken. Please show this to someone who can fix can fix */
		} 
	} 
	else {
		
		{
			if ( interaction == 3 ) 
				; 
			printnl ( 262 ) ; /* ! */
			print ( 291 ) ;	/* I can't go on meeting you like this */
		} 
		{
			helpptr = 2 ; 
			helpline [ 1 ] = 292 ; /* One of your faux pas seems to have wounded me deeply... */
			helpline [ 0 ] = 293 ; /* in fact, I'm barely conscious. Please fix it and try again. */
		} 
	} 
	{
		if ( interaction == 3 ) interaction = 2 ; 
		if ( logopened ) {
			error () ;
			ABORTCHECK;
		}
		;
#ifdef DEBUG
		if ( interaction > 0 ) debughelp () ; 
#endif /* DEBUG */
		history = 3 ; 
		jumpout();
//		return;			// can drop through now 99/Oct/20	  
	} 
} 

booleane initterminal ( ) 
{/* 10 */ register booleane Result; initterminal_regmem 
	int flag;
	topenin () ; 

	if ( last > first )   {
		curinput .locfield = first ; 
		while ( ( curinput .locfield < last ) &&
				( buffer [ curinput .locfield ] == ' ' ) )
			incr ( curinput .locfield ) ;		// step over initial white space
		if ( curinput .locfield < last ) {
			Result = true ; 
			return Result ;		// there is an input file name
		} 
	} 

//	failed to find input file name
	while ( true ) {
		; 
#ifdef _WINDOWS
		flag = ConsoleInput("**",
							"Please type a file name or a control sequence\r\n(or ^z to exit)",
							(char *) &buffer[first]);
		last = first + strlen((char *) &buffer[first]);	/* -1 ? */
//		may need to be more elaborate see input_line in texmf.c
#else
		(void) fputs( "**", stdout ) ;			/* ** PROMPT */
		fflush ( stdout ) ; 
		flag = inputln ( stdin , true );
#endif
		if ( ! flag ) {
			showchar('\n');
			showline("! End of file on the terminal... why?\n", 1); 
			Result = false ; 
			return Result ; 
		} 

		curinput .locfield = first ; 
		while ( ( curinput .locfield < last ) &&
				( buffer [ curinput .locfield ] == ' ' ) )
			incr ( curinput .locfield ) ;		// step over intial white space
		if ( curinput .locfield < last ) {
			Result = true ; 
			return Result ;		// there is an input file name
		} 
		sprintf(logline , "%s\n",  "Please type the name of your input file." ) ; 
		showline(logline, 1);
	}
//	return Result ; 
} 

// Make string from strstart [ strptr ] to poolptr

strnumber makestring ( ) 
{register strnumber Result; makestring_regmem 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
  if ( strptr == currentmaxstrings )
	  strstart = reallocstrstart ( incrementmaxstrings );
  if ( strptr == currentmaxstrings ) {				/* 94/Jan/24 */
//	  printf("**********MAKESTRING**********");		// debugging only
 	  overflow ( 258 , currentmaxstrings - initstrptr ) ; /* 97/Mar/9 */
	  return 0;			// abortflag set
  }
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if ( strptr == maxstrings ) {
	  overflow ( 258 , maxstrings - initstrptr ) ; /* number of strings */
	  return 0;			// abortflag set
  }
#endif
  incr ( strptr ) ; 
  strstart [ strptr ] = poolptr ; 
  Result = strptr - 1 ; 
  return Result ; 
} 

booleane zstreqbuf ( s , k ) 
strnumber s ; 
integer k ; 
{/* 45 */ register booleane Result; streqbuf_regmem 
  poolpointer j  ; 
  booleane result  ; 
  j = strstart [ s ] ; 
  while ( j < strstart [ s + 1 ] ) {
      
    if ( strpool [ j ] != buffer [ k ] ) 
    {
      result = false ; 
      goto lab45 ; 
    } 
    incr ( j ) ; 
    incr ( k ) ; 
  } 
  result = true ; 
  lab45: Result = result ; 
  return Result ; 
} 

booleane zstreqstr ( s , t ) 
strnumber s ; 
strnumber t ; 
{/* 45 */ register booleane Result; streqstr_regmem 
  poolpointer j, k  ; 
  booleane result  ; 
  result = false ; 
  if ( ( strstart [ s + 1 ] - strstart [ s ] ) != ( strstart [ t + 1 ] - 
  strstart [ t ] ) ) 
  goto lab45 ; 
  j = strstart [ s ] ; 
  k = strstart [ t ] ; 
  while ( j < strstart [ s + 1 ] ) {
      
    if ( strpool [ j ] != strpool [ k ] ) 
    goto lab45 ; 
    incr ( j ) ; 
    incr ( k ) ; 
  } 
  result = true ; 
  lab45: Result = result ; 
  return Result ; 
} 

void zprinttwo ( n ) 
integer n ; 
{printtwo_regmem 
  n = abs ( n ) % 100 ; 
  printchar ( 48 + ( n / 10 ) ) ; 
  printchar ( 48 + ( n % 10 ) ) ; 
} 

void zprinthex ( n ) 
integer n ; 
{printhex_regmem 
  char k  ; 
  k = 0 ; 
  printchar ( 34 ) ;  	/* " */
  do {
/*      dig [ k ] = n % 16 ; */
	  dig [ k ] = (unsigned char) (n % 16) ;  
/*      dig [ k ] = n & 15 ; */
    n = n / 16 ; 
/*    n = n >> 4 ;  */
    incr ( k ) ; 
  } while ( ! ( n == 0 ) ) ; 
  printthedigs ( k ) ; 
} 

void zprintromanint ( n ) 
integer n ; 
{/* 10 */ printromanint_regmem 
  poolpointer j, k  ; 
  nonnegativeinteger u, v  ; 
  j = strstart [ 260 ] ;	/*  m2d5c2l5x2v5i */
  v = 1000 ; 
  while ( true ) {
    while ( n >= v ) {
      printchar ( strpool [ j ] ) ; 
      n = n - v ; 
    } 
    if ( n <= 0 ) 
    return ; 
    k = j + 2 ; 
    u = v / ( strpool [ k - 1 ] - 48 ) ; 
    if ( strpool [ k - 1 ] == 50 ) 
    {
      k = k + 2 ; 
      u = u / ( strpool [ k - 1 ] - 48 ) ; 
    } 
    if ( n + u >= v ) 
    {
      printchar ( strpool [ k ] ) ; 
      n = n + u ; 
    } 
    else {
      j = j + 2 ; 
      v = v / ( strpool [ j - 1 ] - 48 ) ; 
    } 
  } 
} 

void printcurrentstring ( void ) 
{printcurrentstring_regmem 
  poolpointer j  ; 
  j = strstart [ strptr ] ; 
  while ( j < poolptr ) {
    printchar ( strpool [ j ] ) ; 
    incr ( j ) ; 
  } 
} 

int stringlength (int strptr) {
	int nstart, nnext;
	nstart = strstart[ strptr ];
	nnext = strstart[ strptr + 1 ];
	return (nnext - nstart) + 2;
}

char *addstring (char *s, int strptr) {
	int nstart, nnext, n;
	nstart = strstart[ strptr ];
	nnext = strstart[ strptr + 1 ];
	n = nnext - nstart;
	memcpy(s, &strpool[nstart], n);
	s += n;
	strcpy(s, "\r\n");
	s += 2;
	return s;
}

int addextrahelp=1;

// make one long \r\n separated string out of help lines 
// strpool is packedASCIIcode *

char *makeuphelpstring (int nhelplines) {
	char *helpstring, *s;
	int k, nlen=0;
	
//	get length of help for this specific message
	for (k = nhelplines-1; k >= 0; k--) {
		nlen += stringlength(helpline[ k ]);
	}
	nlen += 2;		// for blank line separator
	if (addextrahelp) {
		nlen += stringlength(265);
		nlen += stringlength(266);
		nlen += stringlength(267);
		if ( baseptr > 0 ) nlen += stringlength(268);
		if ( deletionsallowed ) nlen += stringlength(269);
		nlen += stringlength(270);
	}
	helpstring = (char *) malloc(nlen+1);
	s = helpstring;
	for (k = nhelplines-1; k >= 0; k--) {
		s = addstring(s, helpline[k]);
	}
	if (addextrahelp) {
		strcpy(s, "\r\n");
		s += 2;
		s = addstring(s, 265);		/* Type <return> to proceed, S to scroll future error messages, */
		s = addstring(s, 266);		/* R to run without stopping, Q to run quietly, */
		s = addstring(s, 267);		/* I to insert something,  */
		if ( baseptr > 0 ) s = addstring(s, 268) ;	/*  E to edit your file,  */
		if ( deletionsallowed ) s = addstring(s,  269 ) ;	/* 1 or ... or 9 to ignore the next 1 to 9 tokens of input, */
		s = addstring(s, 270);		/* H for help, X to quit. */
	}
	return helpstring;
}

char *makeupquerystring (int promptstr) {
	char *querystr;
	int nstart, nnext, n;
	char *s;
	nstart = strstart[ promptstr ];
	nnext = strstart[ promptstr + 1 ];
	n = nnext - nstart;
	querystr = (char *) malloc(n + 1);
	s = querystr;
	memcpy(s, &strpool[nstart], n);	
	s += n;
	*s = '\0';
	return querystr;	
}

// abortflag set if input_line / ConsoleInput returns non-zero
// should set interrupt instead ???
// called from tex0.c, tex2.c, tex3.c

// void terminput ( void ) 
void terminput ( int promptstr, int nhelplines ) 
{terminput_regmem 
    integer k  ;
	int flag;
	char *helpstring=NULL;
	char *querystring=NULL;
//	if (nhelplines != 0) {
//		helpstring = makeuphelpstring (nhelplines);
//		printf(helpstring);
//		free(helpstring);
//	}
	showline("\n", 0);		// force it to show what may be buffered up ???
	helpstring = NULL;	
#ifdef _WINDOWS
	if (promptstr != 0) querystring = makeupquerystring (promptstr);
	if (nhelplines != 0) helpstring = makeuphelpstring (nhelplines);
	if (helpstring == NULL && querystring != NULL) {
		if (strcmp(querystring, ": ") == 0)
			helpstring = xstrdup("Please type another file name (or ^z to exit):");
		else if (strcmp(querystring, "=>") == 0)		// from firmuptheline
			helpstring = xstrdup("Please type <enter> to accept this line\r\nor type a replacement line");
		else if (strcmp(querystring, "insert>") == 0)	// from error() after "I"
			helpstring = xstrdup("Please type something to insert here");
		else if (strcmp(querystring, "") == 0)			// from readtoks
			helpstring = xstrdup("Please type a control sequence");
		else if (strcmp(querystring, "= ") == 0)		// from readtoks
			helpstring = xstrdup("Please type a token");
		else if (strcmp(querystring, "*") == 0)		// getnext
			helpstring = xstrdup("Please type a control sequence\r\n(or ^z to exit)");
//		else if (strcmp(querystring, "**") == 0)	// initterminal
//			helpstring = xstrdup("Please type a control sequence or a file name\r\n(or ^z to exit)");			
//		else if (strcmp(querystring, "? ") == 0)	// from error()
//			helpstring = xstrdup("Please type a character to select an action");
	}
	flag = ConsoleInput(querystring, helpstring, (char *) &buffer[first]);	// ???
//	flag == 0 means trouble --- EOF on terminal
	if (querystring != NULL) free(querystring);
	if (helpstring != NULL) free(helpstring);
	helpstring = querystring = NULL;

	last = first + strlen((char *) &buffer[first]);	/* -1 ? */
//	flag = (last > first);
//	may need to be more elaborate see input_line in texmf.c ???
//	sprintf(logline, "first %d last %d flag %d - %s",
//			first, last, flag, (char *) &buffer[first]);
//	winerror(logline);
#else
	fflush ( stdout ) ; 
	flag = inputln ( stdin , true );
#endif
	if ( ! flag ) {
		fatalerror ( 261 ) ;	/* End of file on the terminal! */
		return;					// abortflag set
	}
	termoffset = 0 ; 
#ifdef _WINDOWS
// echo what was typed into Console buffer also
	if ( last != first ) 
		{register integer for_end; k = first ; for_end = last - 1 ;
	if ( k <=  for_end) do
		print ( buffer [ k ] ) ; 
	while ( k++ < for_end ) ;
		} 
	println () ; 
#else
	decr ( selector ) ;			// shut off echo
	if ( last != first ) 
		{register integer for_end; k = first ; for_end = last - 1 ;
	if ( k <=  for_end) do
		print ( buffer [ k ] ) ; 
	while ( k++ < for_end ) ;
		} 
	println () ; 
	incr ( selector ) ;			// reset selector again
#endif
} 

void zinterror ( integer n ) 
{interror_regmem 
    print ( 284 ) ;	 /*  ( */
	printint ( n ) ; 
	printchar ( 41 ) ; /* ) */
	error () ; 
} 

void normalizeselector ( ) 
{normalizeselector_regmem 
    if ( logopened ) 	selector = 19 ; 
	else selector = 17 ; 
	if ( jobname == 0 )	openlogfile () ; 
	if ( interaction == 0 ) decr ( selector ) ; 
} 

void pauseforinstructions ( ) 
{pauseforinstructions_regmem 
   if ( OKtointerrupt ) {
	  interaction = 3 ; 
	  if ( ( selector == 18 ) || ( selector == 16 ) ) incr ( selector ) ; 
	  {
		  if ( interaction == 3 ) 
			  ; 
		  printnl ( 262 ) ;		/* ! */
		  print ( 294 ) ;		/* Interruption */
	  } 
	  {
		helpptr = 3 ; 
		helpline [ 2 ] = 295 ; /* You rang? */
		helpline [ 1 ] = 296 ; /* Try to insert some instructions for me (e.g.,`I\showlists'), */
		helpline [ 0 ] = 297 ; /* unless you just want to quit by typing `X'. */
	  } 
	  deletionsallowed = false ; 
	  error () ; 
	  ABORTCHECK;
	  deletionsallowed = true ; 
	  interrupt = 0 ; 
	} 
} 

integer zhalf ( x ) 
integer x ; 
{register integer Result; half_regmem 
  if ( odd ( x ) ) 
	Result = ( x + 1 ) / 2 ; 
  else Result = x / 2 ; 
  return Result ; 
} 

scaled zrounddecimals ( k ) 
smallnumber k ; 
{register scaled Result; rounddecimals_regmem 
  integer a  ; 
  a = 0 ; 
  while ( k > 0 ) {
    decr ( k ) ; 
    a = ( a + dig [ k ] * 131072L ) / 10 ;	/* 2^17 */
  } 
  Result = ( a + 1 ) / 2 ; 
  return Result ; 
} 

/* This has some minor speedup changes - no real advantage probably ... */
void zprintscaled ( s ) 
scaled s ; 
{printscaled_regmem 
  scaled delta  ; 
  if ( s < 0 ) 
  {
    printchar ( 45 ) ;						/* '-' */
    s = - (integer) s ; 
  } 
  printint ( s / 65536L ) ; 
/*  printint ( s >> 16 ) ;  */
  printchar ( 46 ) ;						/* '.' */
  s = 10 * ( s % 65536L ) + 5 ; 
/*  s = 10 * ( s & 65535L ) + 5 ; */
  delta = 10 ; 
  do {
      if ( delta > 65536L ) 
    s = s - 17232 ;							/* 2^15 - 50000 - rounding */
    printchar ( 48 + ( s / 65536L ) ) ;		/* '0' + */
/*    printchar ( 48 + ( s >> 16 ) ) ; */
    s = 10 * ( s % 65536L ) ; 
/*    s = 10 * ( s & 65535L ) ; */
    delta = delta * 10 ; 
  } while ( ! ( s <= delta ) ) ; 
} 

scaled zmultandadd ( n , x , y , maxanswer ) 
integer n ; 
scaled x ; 
scaled y ; 
scaled maxanswer ; 
{register scaled Result; multandadd_regmem 
  if ( n < 0 ) 
  {
    x = - (integer) x ; 
    n = - (integer) n ; 
  } 
  if ( n == 0 ) 
  Result = y ; 
  else if ( ( ( x <= ( maxanswer - y ) / n ) && ( - (integer) x <= ( maxanswer 
  + y ) / n ) ) ) 
  Result = n * x + y ; 
  else {
    aritherror = true ; 
    Result = 0 ; 
  } 
  return Result ; 
} 

scaled zxovern ( x , n ) 
scaled x ; 
integer n ; 
{register scaled Result; xovern_regmem 
  booleane negative  ; 
  negative = false ; 
  if ( n == 0 ) 
  {
    aritherror = true ; 
    Result = 0 ; 
    texremainder = x ; 
  } 
  else {
      
    if ( n < 0 ) 
    {
      x = - (integer) x ; 
      n = - (integer) n ; 
      negative = true ; 
    } 
    if ( x >= 0 ) 
    {
      Result = x / n ; 
      texremainder = x % n ; 
    } 
    else {
	
      Result = - (integer) ( ( - (integer) x ) / n ) ; 
      texremainder = - (integer) ( ( - (integer) x ) % n ) ; 
    } 
  } 
  if ( negative ) 
  texremainder = - (integer) texremainder ; 
  return Result ; 
} 

scaled zxnoverd ( x , n , d ) 
scaled x ; 
integer n ; 
integer d ; 
{register scaled Result; xnoverd_regmem 
  booleane positive  ; 
  nonnegativeinteger t, u, v  ; 
  if ( x >= 0 ) 
  positive = true ; 
  else {
      
    x = - (integer) x ; 
    positive = false ; 
  } 
/*  t = ( x % 32768L ) * n ;  */
  t = ( x & 32767L ) * n ; 
/*  u = ( x / 32768L ) * n + ( t / 32768L ) ;  */
  u = ( x >> 15 ) * n + ( t >> 15 ) ; 
/*  v = ( u % d ) * 32768L + ( t % 32768L ) ;  */
  v = ( ( u % d ) << 15 ) + ( t & 32767L ) ; 
  if ( u / d >= 32768L ) 
  aritherror = true ; 
/*  else u = 32768L * ( u / d ) + ( v / d ) ;  */ 
  else u = ( ( u / d ) << 15 ) + ( v / d ) ;	
  if ( positive ) 
  {
    Result = u ; 
    texremainder = v % d ; 
  } 
  else {
      
    Result = - (integer) u ; 
    texremainder = - (integer) ( v % d ) ; 
  } 
  return Result ; 
} 

halfword zbadness ( t , s ) 
scaled t ; 
scaled s ; 
{register halfword Result; badness_regmem 
  integer r  ; 
  if ( t == 0 ) 
  Result = 0 ; 
  else if ( s <= 0 ) 
  Result = 10000 ; 
  else {
    if ( t <= 7230584L ) 
    r = ( t * 297 ) / s ; 
    else if ( s >= 1663497L ) 
    r = t / ( s / 297 ) ; 
    else r = t ; 
    if ( r > 1290 ) 
    Result = 10000 ; 
/*    safe to assume that r is positive ? */
/*    else Result = ( r * r * r + 131072L ) / 262144L ;  */
    else Result = ( r * r * r + 131072L ) >> 18 ;	/* 2^17 */
  } 
  return Result ; 
} 

#ifdef DEBUG
void zprintword ( w ) 
memoryword w ; 
{printword_regmem 
  printint ( w .cint ) ; 
  printchar ( 32 ) ;			/*   */
  printscaled ( w .cint ) ; 
  printchar ( 32 ) ;		 	/*   */
  printscaled ( round ( 65536L * w .gr ) ) ; 
  println () ; 
  printint ( w .hh .v.LH ) ; 
  printchar ( 61 ) ;			/* = */
  printint ( w .hh.b0 ) ; 
  printchar ( 58 ) ;			/* : */
  printint ( w .hh.b1 ) ; 
  printchar ( 59 ) ;			/* ; */
  printint ( w .hh .v.RH ) ; 
  printchar ( 32 ) ;			/*   */
  printint ( w .qqqq .b0 ) ; 
  printchar ( 58 ) ;			/* : */
  printint ( w .qqqq .b1 ) ; 
  printchar ( 58 ) ;			/* : */
  printint ( w .qqqq .b2 ) ; 
  printchar ( 58 ) ;			/* : */
  printint ( w .qqqq .b3 ) ; 
} 

/* need this version only if SHORTFONTINFO defined */

void zprintfword ( w ) 
fmemoryword w ; 
{printword_regmem 
   printint ( w .cint ) ; 
printchar ( 32 ) ;			/*   */
printscaled ( w .cint ) ; 
printchar ( 32 ) ;		 	/*   */
printscaled ( round ( 65536L * w .gr ) ) ; 
println () ; 
printint ( w .hh .v.LH ) ; 
printchar ( 61 ) ;			/* = */
printint ( w .hh.b0 ) ; 
printchar ( 58 ) ;			/* : */
printint ( w .hh.b1 ) ; 
printchar ( 59 ) ;			/* ; */
printint ( w .hh .v.RH ) ; 
printchar ( 32 ) ;			/*   */
printint ( w .qqqq .b0 ) ; 
printchar ( 58 ) ;			/* : */
printint ( w .qqqq .b1 ) ; 
printchar ( 58 ) ;			/* : */
printint ( w .qqqq .b2 ) ; 
printchar ( 58 ) ;			/* : */
printint ( w .qqqq .b3 ) ; 
} 
#endif /* DEBUG */

void zshowtokenlist ( p , q , l ) 
integer p ; 
integer q ; 
integer l ; 
{/* 10 */ showtokenlist_regmem 
  integer m, c  ; 
  ASCIIcode matchchr  ; 
  ASCIIcode n  ; 
  matchchr = 35 ; 
  n = 48 ; 
  tally = 0 ; 
/* while (p<>null) and (tally<l) do l.6239 */
  while ( ( p != 0 ) && ( tally < l ) ) {
      
    if ( p == q ) 
    {
      firstcount = tally ; 
      trickcount = tally + 1 + errorline - halferrorline ; 
      if ( trickcount < errorline ) 
      trickcount = errorline ; 
    } 
    if ( ( p < himemmin ) || ( p > memend ) ) 
    {
      printesc ( 307 ) ;	/* CLOBBERED. */
      return ; 
    } 
    if ( mem [ p ] .hh .v.LH >= 4095 ) 
    printcs ( mem [ p ] .hh .v.LH - 4095 ) ; 
    else {
	
      m = mem [ p ] .hh .v.LH / 256 ; 
/*      m = mem [ p ] .hh .v.LH >> 8 ;  */
      c = mem [ p ] .hh .v.LH % 256 ; 
/*      c = mem [ p ] .hh .v.LH & 255 ;  */
      if ( mem [ p ] .hh .v.LH < 0 ) 
      printesc ( 552 ) ;			/* BAD. */
      else switch ( m ) 
      {case 1 : 
      case 2 : 
      case 3 : 
      case 4 : 
      case 7 : 
      case 8 : 
      case 10 : 
      case 11 : 
      case 12 : 
	print ( c ) ; 
	break ; 
      case 6 : 
	{
	  print ( c ) ; 
	  print ( c ) ; 
	} 
	break ; 
      case 5 : 
	{
	  print ( matchchr ) ; 
	  if ( c <= 9 ) 
	  printchar ( c + 48 ) ; 
	  else {
	    printchar ( 33 ) ;		/* ! */
	    return ; 
	  } 
	} 
	break ; 
      case 13 : 
	{
/*	  matchchr =  c ; */		/* keep compiler happy */
	  matchchr = (ASCIIcode) c ; 
	  print ( c ) ; 
	  incr ( n ) ; 
	  printchar ( n ) ; 
	  if ( n > 57 ) 
		  return ; 
	} 
	break ; 
      case 14 : 
	print ( 553 ) ;			/* -> */
	break ; 
	default: 
	printesc ( 552 ) ;		/* BAD.  */
	break ; 
      } 
    } 
    p = mem [ p ] .hh .v.RH ; 
  } 
/* if p<>null then print_esc("ETC."); l.6244 */
  if ( p != 0 ) 
  printesc ( 551 ) ;			/* ETC. */
} 

void runaway ( ) 
{runaway_regmem 
  halfword p  ; 
  if ( scannerstatus > 1 ) 
  {
    printnl ( 566 ) ;				/* Runaway  */
    switch ( scannerstatus ) 
    {case 2 : 
      {
	print ( 567 ) ;					/* definition */
	p = defref ; 
      } 
      break ; 
    case 3 : 
      {
	print ( 568 ) ;					/* argument */
	p = memtop - 3 ; 
      } 
      break ; 
    case 4 : 
      {
	print ( 569 ) ;					/* preamble */
	p = memtop - 4 ; 
      } 
      break ; 
    case 5 : 
      {
	print ( 570 ) ;					/* text */
	p = defref ; 
      } 
      break ; 
    } 
    printchar ( 63 ) ;				/* ? */
    println () ; 
/*	p may be used without being initialized -- OK */
    showtokenlist ( mem [ p ] .hh .v.RH , 0 , errorline - 10 ) ; 
  } 
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* first try list of available nodes (avail != NULL) */
/* then see if can go upwards (memend < memmax) */
/* then see if can go downwards (himemmin > lomemmax) */
/* if not, extend memory at the top and grab from there --- new */
/* else fail ! */	/* paragraph 120 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
halfword getavail ( ) 
{register halfword Result; getavail_regmem 
  halfword p  ; 
  p = avail ; 
  if ( p != 0 )								/* while p<>null do */
  avail = mem [ avail ] .hh .v.RH ; 
  else if ( memend < memmax )				/* memend + 1 < memmax ? NO */
  {
    incr ( memend ) ; 
    p = memend ; 
  } 
  else { 
    decr ( himemmin ) ; 
    p = himemmin ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if ( himemmin <= lomemmax ) {		/* have we run out in middle ? */
		incr ( himemmin ) ;				/* undo the change */
/*		reallocmain (0, memtop/2); */	/* extend main memory at hi end */
		mem = reallocmain (0, memtop/2);	/* zzzaa = zmem = mem */
		if (mem == NULL) {
			return 0;
		}
/* presumably now memend < memmax - but need test in case allocation fails */
		if (memend >= memmax)    {
			runaway () ; 
			overflow ( 298 , memmax + 1 - memmin ) ; /* main memory size */
			return 0;						// abortflag set
		} 
		incr ( memend ) ;				/* then grab from new area */
		p = memend ;					/* 1993/Dec/14 */
	}
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  }	
  mem [ p ] .hh .v.RH = 0 ;				/* link(p) = null !!! */
	;
#ifdef STAT
  incr ( dynused ) ; 
#endif /* STAT */
  Result = p ; 
  return Result ; 
} 

void zflushlist ( p )					/* paragraph 123 */
halfword p ; 
{flushlist_regmem 
  halfword q, r  ; 
  if ( p != 0 )							/* null !!! */
  {
    r = p ; 
    do {
	q = r ; 
      r = mem [ r ] .hh .v.RH ; 
	;
#ifdef STAT
      decr ( dynused ) ; 
#endif /* STAT */
    } while ( ! ( r == 0 ) ) ;			/* r != null */
    mem [ q ] .hh .v.RH = avail ; 
    avail = p ; 
  } 
} 

halfword zgetnode ( s ) 
integer s ; 
{/* 40 10 20 */ register halfword Result; getnode_regmem 
  halfword p  ; 
  halfword q  ; 
  integer r  ; 
  integer t  ; 
  lab20: p = rover ; 
  do {
      q = p + mem [ p ] .hh .v.LH ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*    while ( ( mem [ q ] .hh .v.RH == 262143L ) ) { */ /* NO! */
    while ( ( mem [ q ] .hh .v.RH == emptyflag ) ) {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

	  if (q == 0) {
/* should never happen, since this field is reference count for zeroglue */
	  }  /* debugging code 93/DEC/15 */ /* eventually remove */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
      t = mem [ q + 1 ] .hh .v.RH ; 
      if ( q == rover ) 
      rover = t ; 
      mem [ t + 1 ] .hh .v.LH = mem [ q + 1 ] .hh .v.LH ; 
      mem [ mem [ q + 1 ] .hh .v.LH + 1 ] .hh .v.RH = t ; 
      q = q + mem [ q ] .hh .v.LH ; 
    } 
    r = q - s ; 
    if ( r > toint ( p + 1 ) ) 
    {
      mem [ p ] .hh .v.LH = r - p ; 
      rover = p ; 
      goto lab40 ; 
    } 
    if ( r == p ) 
    if ( mem [ p + 1 ] .hh .v.RH != p ) 
    {
      rover = mem [ p + 1 ] .hh .v.RH ; 
      t = mem [ p + 1 ] .hh .v.LH ; 
      mem [ rover + 1 ] .hh .v.LH = t ; 
      mem [ t + 1 ] .hh .v.RH = rover ; 
      goto lab40 ; 
    } 
    mem [ p ] .hh .v.LH = q - p ; 
    p = mem [ p + 1 ] .hh .v.RH ; 
  } while ( ! ( p == rover ) ) ; 
  if ( s == 1073741824L )		/* 2^30 - special case - merge adjacent */
  {
/*    Result = 262143L ;  */ /* NO ! */
    Result = emptyflag ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	if (traceflag) showline("Merged adjacent multi-word nodes\n", 0);
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    return Result ; 
  } 
/*	maybe try downward epxansion first instead ? */
  if ( lomemmax + 2 < himemmin ) 

/*  if ( lomemmax + 2 <= 262143L )  */	/* NO! */
  if ( lomemmax + 2 <= membot + maxhalfword ) 	/* silly ? flush 93/Dec/16 */

  {
/*    if ( himemmin - lomemmax >= 1998 )  */
    if ( himemmin - lomemmax >= (blocksize + blocksize - 2) ) 
/*    t = lomemmax + 1000 ;  */
    t = lomemmax + blocksize ; 
    else t = lomemmax + 1 + ( himemmin - lomemmax ) / 2 ; 
    p = mem [ rover + 1 ] .hh .v.LH ; 
    q = lomemmax ; 
    mem [ p + 1 ] .hh .v.RH = q ; 
    mem [ rover + 1 ] .hh .v.LH = q ; 

/*    if ( t > 262143L )    t = 262143L ;  */ /* NO! */
    if ( t > membot + maxhalfword )
		t = membot + maxhalfword ;			/* silly ? flush 93/Dec/16 */

    mem [ q + 1 ] .hh .v.RH = rover ; 
    mem [ q + 1 ] .hh .v.LH = p ; 
/*    mem [ q ] .hh .v.RH = 262143L ; */ /* NO! */
    mem [ q ] .hh .v.RH = emptyflag ; 
    mem [ q ] .hh .v.LH = t - lomemmax ; /* block size */
    lomemmax = t ; 
    mem [ lomemmax ] .hh .v.RH = 0 ; 
    mem [ lomemmax ] .hh .v.LH = 0 ; 
    rover = q ; 
    goto lab20 ; 
  } 
/*  overflow ( 298 , memmax + 1 - memmin ) ;  */ /* what used to happen! */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* we've run out of space in the middle for variable length blocks */
/* try and add new block from below membot *//* first check if space ! */
  if (memmin - (blocksize + 1) <= memstart)	{/* extend lower memory downwards */
/*	  reallocmain (memtop/2, 0); */
	  mem = reallocmain (memtop/2 + blocksize, 0);	/* zzzaa = zmem = mem */
	  if (mem == NULL) {
		  return 0;
	  }
  }
/*  if (traceflag) showline("Extending downwards by %d\n", blocksize, 0); */
  if (memmin - (blocksize + 1) <= memstart)	{ /* check again */
	  if (traceflag) {
		  sprintf(logline, "memmin %d, memstart %d, blocksize %d\n",
				  memmin, memstart, blocksize);
		  showline(logline, 0);
	  }
	  overflow ( 298 , memmax + 1 - memmin ) ;	/* darn: allocation failed ! */
	  return 0;			// abortflag set
  }
/* avoid function call in following ? */
  addvariablespace (blocksize);	/* now to be found in itex.c */
  goto lab20 ;					/* go try getnode again */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

  lab40: mem [ r ] .hh .v.RH = 0 ; 
	;
#ifdef STAT
  varused = varused + s ; 
#endif /* STAT */

/*  if (traceflag) {
	  if (r == 0) showline("r IS ZERO in ZGETNODE!\n", 0);
  } */ 			/* debugging code 93/dec/15 */

  Result = r ; 
  return Result ; 
} 

void zfreenode ( p , s ) 
halfword p ; 
halfword s ; 
{freenode_regmem 
  halfword q  ; 
  mem [ p ] .hh .v.LH = s ; 
/*  mem [ p ] .hh .v.RH = 262143L ;  */	/* NO! */
  mem [ p ] .hh .v.RH = emptyflag ; 
  q = mem [ rover + 1 ] .hh .v.LH ; 
  mem [ p + 1 ] .hh .v.LH = q ; 
  mem [ p + 1 ] .hh .v.RH = rover ; 
  mem [ rover + 1 ] .hh .v.LH = p ; 
  mem [ q + 1 ] .hh .v.RH = p ; 
	;
#ifdef STAT
  varused = varused - s ; 
#endif /* STAT */
} 

halfword newnullbox ( ) 
{register halfword Result; newnullbox_regmem 
  halfword p  ; 
  p = getnode ( 7 ) ; 
  mem [ p ] .hh.b0 = 0 ; 
  mem [ p ] .hh.b1 = 0 ; 
  mem [ p + 1 ] .cint = 0 ; 
  mem [ p + 2 ] .cint = 0 ; 
  mem [ p + 3 ] .cint = 0 ; 
  mem [ p + 4 ] .cint = 0 ; 
  mem [ p + 5 ] .hh .v.RH = 0 ; 
  mem [ p + 5 ] .hh.b0 = 0 ; 
  mem [ p + 5 ] .hh.b1 = 0 ; 
  mem [ p + 6 ] .gr = 0.0 ; 
  Result = p ; 
  return Result ; 
} 

// @ A new rule node is delivered by the |new_rule| function. It
//   makes all the dimensions ``running,'' so you have to change the
//   ones that are not allowed to run.

halfword newrule ( ) 
{register halfword Result; newrule_regmem 
  halfword p  ; 
  p = getnode ( 4 ) ;					/* rule_node_size */
  mem [ p ] .hh.b0 = 2 ;				/* rule_node type */
  mem [ p ] .hh.b1 = 0 ;				/* sub_type zero */
  mem [ p + 1 ] .cint = -1073741824L ;	/* -2^30 null_flag width */
  mem [ p + 2 ] .cint = -1073741824L ;	/* -2^30 null_flag depth */
  mem [ p + 3 ] .cint = -1073741824L ;	/* -2^30 null_flag height */
  Result = p ; 
  return Result ; 
} 

// @ The |new_ligature| function creates a ligature node having given
//   contents of the |font|, |character|, and |lig_ptr| fields.

halfword znewligature ( f , c , q ) 
quarterword f ; 
quarterword c ; 
halfword q ; 
{register halfword Result; newligature_regmem 
  halfword p  ; 
  p = getnode ( 2 ) ;			/* small_node_size */
  mem [ p ] .hh.b0 = 6 ;		/* ligature_node type */
  mem [ p + 1 ] .hh.b0 = f ;	/* font */
  mem [ p + 1 ] .hh.b1 = c ;	/* character */
  mem [ p + 1 ] .hh .v.RH = q ;	/* pointer */
  mem [ p ] .hh.b1 = 0 ;		/* subtype zero */
  Result = p ; 
  return Result ; 
} 

//	We also have a |new_lig_item| function, which returns a two-word
//	node having a given |character| field. Such nodes are used for
//	temporary processing as ligatures are being created.

halfword znewligitem ( c ) 
quarterword c ; 
{register halfword Result; newligitem_regmem 
  halfword p  ; 
  p = getnode ( 2 ) ;			/* small_node_size */
  mem [ p ] .hh.b1 = c ;		/* character */ 
  mem [ p + 1 ] .hh .v.RH = 0 ; /* lig_ptr(p):=null; */
  Result = p ; 
  return Result ; 
} 

halfword newdisc ( ) 
{register halfword Result; newdisc_regmem 
  halfword p  ; 
  p = getnode ( 2 ) ; 
  mem [ p ] .hh.b0 = 7 ; 
  mem [ p ] .hh.b1 = 0 ; 
  mem [ p + 1 ] .hh .v.LH = 0 ; /* pre_break(p):=null; */
  mem [ p + 1 ] .hh .v.RH = 0 ; /* post_break(p):=null; */
  Result = p ; 
  return Result ; 
} 

halfword znewmath ( w , s ) 
scaled w ; 
smallnumber s ; 
{register halfword Result; newmath_regmem 
  halfword p  ; 
  p = getnode ( 2 ) ; 
  mem [ p ] .hh.b0 = 9 ; 
  mem [ p ] .hh.b1 = s ; 
  mem [ p + 1 ] .cint = w ; 
  Result = p ; 
  return Result ; 
} 

halfword znewspec ( p ) 
halfword p ; 
{register halfword Result; newspec_regmem 
  halfword q  ; 
  q = getnode ( 4 ) ; 
  mem [ q ] = mem [ p ] ; 
  mem [ q ] .hh .v.RH = 0 ; 
  mem [ q + 1 ] .cint = mem [ p + 1 ] .cint ; 
  mem [ q + 2 ] .cint = mem [ p + 2 ] .cint ; 
  mem [ q + 3 ] .cint = mem [ p + 3 ] .cint ; 
  Result = q ; 
  return Result ; 
} 

halfword znewparamglue ( n ) 
smallnumber n ; 
{register halfword Result; newparamglue_regmem 
  halfword p  ; 
  halfword q  ; 
  p = getnode ( 2 ) ; 
  mem [ p ] .hh.b0 = 10 ; 
  mem [ p ] .hh.b1 = n + 1 ;	/* conversion int to unsigned short */
  mem [ p + 1 ] .hh .v.RH = 0 ; 
  q = eqtb [ (hash_size + 782) + n ] .hh .v.RH ; /* gluebase + n */
  mem [ p + 1 ] .hh .v.LH = q ; 
  incr ( mem [ q ] .hh .v.RH ) ; 
  Result = p ; 
  return Result ; 
} 

halfword znewglue ( q ) 
halfword q ; 
{register halfword Result; newglue_regmem 
  halfword p  ; 
  p = getnode ( 2 ) ; 
  mem [ p ] .hh.b0 = 10 ; 
  mem [ p ] .hh.b1 = 0 ; 
  mem [ p + 1 ] .hh .v.RH = 0 ; 
  mem [ p + 1 ] .hh .v.LH = q ; 
  incr ( mem [ q ] .hh .v.RH ) ; 
  Result = p ; 
  return Result ; 
} 

halfword znewskipparam ( n ) 
smallnumber n ; 
{register halfword Result; newskipparam_regmem 
  halfword p  ; 
  tempptr = newspec ( eqtb [ (hash_size + 782) + n ] .hh .v.RH ) ; /* gluebase + n */
  p = newglue ( tempptr ) ; 
  mem [ tempptr ] .hh .v.RH = 0 ; 
  mem [ p ] .hh.b1 = n + 1 ; 	/* conversion int to unsigned short */
  Result = p ; 
  return Result ; 
} 

halfword znewkern ( w ) 
scaled w ; 
{register halfword Result; newkern_regmem 
  halfword p  ; 
  p = getnode ( 2 ) ; 
  mem [ p ] .hh.b0 = 11 ; 
  mem [ p ] .hh.b1 = 0 ; 
  mem [ p + 1 ] .cint = w ; 
  Result = p ; 
  return Result ; 
} 

halfword znewpenalty ( m ) 
integer m ; 
{register halfword Result; newpenalty_regmem 
  halfword p  ; 
  p = getnode ( 2 ) ; 
  mem [ p ] .hh.b0 = 12 ; 
  mem [ p ] .hh.b1 = 0 ; 
  mem [ p + 1 ] .cint = m ; 
  Result = p ; 
  return Result ; 
} 

#ifdef DEBUG
void zcheckmem ( printlocs ) 
booleane printlocs ; 
{/* 31 32 */ checkmem_regmem 
  halfword p, q  ; 
  booleane clobbered  ; 
  {register integer for_end; p = memmin ; for_end = lomemmax ; if ( p <= 
  for_end) do 
    freearr [ p ] = false ; 
  while ( p++ < for_end ) ; } 
  {register integer for_end; p = himemmin ; for_end = memend ; if ( p <= 
  for_end) do 
    freearr [ p ] = false ; 
  while ( p++ < for_end ) ; } 
  p = avail ; 
  q = 0 ; 
  clobbered = false ; 
  while ( p != 0 ) {		/* while p<>null do */
      
    if ( ( p > memend ) || ( p < himemmin ) ) 
    clobbered = true ; 
    else if ( freearr [ p ] ) 
    clobbered = true ; 
    if ( clobbered ) 
    {
      printnl ( 299 ) ;		/* AVAIL list clobbered at  */
      printint ( q ) ; 
      goto lab31 ; 
    } 
    freearr [ p ] = true ; 
    q = p ; 
    p = mem [ q ] .hh .v.RH ; 
  } 
  lab31: ; 
  p = rover ; 
  q = 0 ;				/* q:=null */
  clobbered = false ; 
  do {
      if ( ( p >= lomemmax ) || ( p < memmin ) ) 
    clobbered = true ; 
    else if ( ( mem [ p + 1 ] .hh .v.RH >= lomemmax ) || ( mem [ p + 1 ] .hh 
    .v.RH < memmin ) ) 
    clobbered = true ; 
/*    else if ( ! ( ( mem [ p ] .hh .v.RH == 262143L ) ) || ( mem [ p ] .hh *//*NO!*/
    else if ( ! ( ( mem [ p ] .hh .v.RH == emptyflag ) ) || ( mem [ p ] .hh 
    .v.LH < 2 ) || ( p + mem [ p ] .hh .v.LH > lomemmax ) || ( mem [ mem [ p + 
    1 ] .hh .v.RH + 1 ] .hh .v.LH != p ) ) 
    clobbered = true ; 
    if ( clobbered ) 
    {
      printnl ( 300 ) ;		/* Double-AVAIL list clobbered at  */
      printint ( q ) ; 
      goto lab32 ; 
    } 
    {register integer for_end; q = p ; for_end = p + mem [ p ] .hh .v.LH - 1 
    ; if ( q <= for_end) do 
      {
	if ( freearr [ q ] ) 
	{
	  printnl ( 301 ) ;		/* Doubly free location at  */
	  printint ( q ) ; 
	  goto lab32 ; 
	} 
	freearr [ q ] = true ; 
      } 
    while ( q++ < for_end ) ; } 
    q = p ; 
    p = mem [ p + 1 ] .hh .v.RH ; 
  } while ( ! ( p == rover ) ) ; 
  lab32: ; 
  p = memmin ; 
  while ( p <= lomemmax ) {
      
/*    if ( ( mem [ p ] .hh .v.RH == 262143L ) )  */	/* NO! */
    if ( ( mem [ p ] .hh .v.RH == emptyflag ) ) 
    {
      printnl ( 302 ) ;			/* Bad flag at  */
      printint ( p ) ; 
    } 
    while ( ( p <= lomemmax ) && ! freearr [ p ] ) incr ( p ) ; 
    while ( ( p <= lomemmax ) && freearr [ p ] ) incr ( p ) ; 
  } 
  if ( printlocs ) 
  {
    printnl ( 303 ) ;			/* New busy locs: */
    {register integer for_end; p = memmin ; for_end = lomemmax ; if ( p <= 
    for_end) do 
      if ( ! freearr [ p ] && ( ( p > waslomax ) || wasfree [ p ] ) ) 
      {
	printchar ( 32 ) ;			/*   */
	printint ( p ) ; 
      } 
    while ( p++ < for_end ) ; } 
    {register integer for_end; p = himemmin ; for_end = memend ; if ( p <= 
    for_end) do 
      if ( ! freearr [ p ] && ( ( p < washimin ) || ( p > wasmemend ) || 
      wasfree [ p ] ) ) 
      {
	printchar ( 32 ) ;			/*   */
	printint ( p ) ; 
      } 
    while ( p++ < for_end ) ; } 
  } 
  {register integer for_end; p = memmin ; for_end = lomemmax ; if ( p <= 
  for_end) do 
    wasfree [ p ] = freearr [ p ] ; 
  while ( p++ < for_end ) ; } 
  {register integer for_end; p = himemmin ; for_end = memend ; if ( p <= 
  for_end) do 
    wasfree [ p ] = freearr [ p ] ; 
  while ( p++ < for_end ) ; } 
  wasmemend = memend ; 
  waslomax = lomemmax ; 
  washimin = himemmin ; 
} 
#endif /* DEBUG */

#ifdef DEBUG
void zsearchmem ( p ) 
halfword p ; 
{searchmem_regmem 
  integer q  ; 
  {register integer for_end; q = memmin ; for_end = lomemmax ; if ( q <= 
  for_end) do 
    {
      if ( mem [ q ] .hh .v.RH == p ) 
      {
	printnl ( 304 ) ;	/* LINK( */
	printint ( q ) ; 
	printchar ( 41 ) ;	/* ) */
      } 
      if ( mem [ q ] .hh .v.LH == p ) 
      {
	printnl ( 305 ) ;	/* INFO( */
	printint ( q ) ; 
	printchar ( 41 ) ; /* ) */
      } 
    } 
  while ( q++ < for_end ) ; } 
  {register integer for_end; q = himemmin ; for_end = memend ; if ( q <= 
  for_end) do 
    {
      if ( mem [ q ] .hh .v.RH == p ) 
      {
	printnl ( 304 ) ;	/* LINK( */
	printint ( q ) ; 
	printchar ( 41 ) ; /* ) */
      } 
      if ( mem [ q ] .hh .v.LH == p ) 
      {
	printnl ( 305 ) ;	/* INFO( */
	printint ( q ) ; 
	printchar ( 41 ) ;	/* ) */
      } 
    } 
  while ( q++ < for_end ) ; } 
  {register integer for_end; q = 1 ; for_end = (hash_size + 1833) ; if ( q <= for_end) do 
    {
      if ( eqtb [ q ] .hh .v.RH == p ) 
      {
	printnl ( 498 ) ;		/* EQUIV( */
	printint ( q ) ; 
	printchar ( 41 ) ;		/* ) */
      } 
    } 
  while ( q++ < for_end ) ; } 
  if ( saveptr > 0 ) 
  {register integer for_end; q = 0 ; for_end = saveptr - 1 ; if ( q <= 
  for_end) do 
    {
      if ( savestack [ q ] .hh .v.RH == p ) 
      {
	printnl ( 543 ) ;			/* SAVE( */
	printint ( q ) ; 
	printchar ( 41 ) ;			/* ) */
      } 
    } 
  while ( q++ < for_end ) ; } 
/*  {register integer for_end; q = 0 ; for_end = 607 ; if ( q <= for_end) do */
  {register integer for_end; q = 0 ; for_end = hyphen_prime ; if ( q <= for_end) do 
    {
      if ( hyphlist [ q ] == p ) 
      {
	printnl ( 934 ) ;			/* HYPH( */
	printint ( q ) ; 
	printchar ( 41 ) ;			/* ) */
      } 
    } 
  while ( q++ < for_end ) ; } 
} 
#endif /* DEBUG */

void zshortdisplay ( p ) 
integer p ; 
{shortdisplay_regmem 
  integer n  ; 
/*  while ( p > memmin ) { */
  while ( p != 0 ) {			/* want p != null here bkph 93/Dec/15 !!! */
								/* NOTE: still not fixed in 3.14159 ! */
     if ( ( p >= himemmin ) )	/* is_char_node(p) */
    {
      if ( p <= memend ) 
      {
	if ( mem [ p ] .hh.b0 != fontinshortdisplay )	/* font(p) */
	{
	  if ( ( mem [ p ] .hh.b0 > fontmax ) ) 
	  printchar ( 42 ) ;		/* * */
/*	  else printesc ( hash [ (hash_size + 524) + mem [ p ] .hh.b0 ] .v.RH ) ; */
	  else printesc ( hash [ (hash_size + hash_extra + 524) + mem [ p ] .hh.b0 ] .v.RH ) ; 
													  /* 96/Jan/10 */
	  printchar ( 32 ) ;		/*   */
	  fontinshortdisplay = mem [ p ] .hh.b0 ; 
	} 
	print ( mem [ p ] .hh.b1 ) ;					/* character(p) */
      } 
    } 
    else switch ( mem [ p ] .hh.b0 ) 
    {case 0 : 
    case 1 : 
    case 3 : 
    case 8 : 
    case 4 : 
    case 5 : 
    case 13 : 
      print ( 306 ) ;		/* [] */
      break ; 
    case 2 : 
      printchar ( 124 ) ;	/* | */
      break ; 
    case 10 : 
      if ( mem [ p + 1 ] .hh .v.LH != 0 ) 
      printchar ( 32 ) ;	/*   */
      break ; 
    case 9 : 
      printchar ( 36 ) ;	/* $ */
      break ; 
    case 6 : 
      shortdisplay ( mem [ p + 1 ] .hh .v.RH ) ; 
      break ; 
    case 7 : 
      {
	shortdisplay ( mem [ p + 1 ] .hh .v.LH ) ; 
	shortdisplay ( mem [ p + 1 ] .hh .v.RH ) ; 
	n = mem [ p ] .hh.b1 ; 
	while ( n > 0 ) {
	    
	  if ( mem [ p ] .hh .v.RH != 0 )	/* if link(p)<>null then */
	  p = mem [ p ] .hh .v.RH ; 
	  decr ( n ) ; 
	} 
      } 
      break ; 
      default: 
      ; 
      break ; 
    } 
    p = mem [ p ] .hh .v.RH ; 
  } 
} 

void zprintfontandchar ( p ) 
integer p ; 
{printfontandchar_regmem 
  if ( p > memend ) 
  printesc ( 307 ) ;	/* CLOBBERED. */
  else {
      
    if ( ( mem [ p ] .hh.b0 > fontmax ) )	/* font(p) */
    printchar ( 42 ) ;		/* * */
/*    else printesc ( hash [ (hash_size + 524) + mem [ p ] .hh.b0 ] .v.RH ) ; */
    else printesc ( hash [ (hash_size + hash_extra + 524) + mem [ p ] .hh.b0 ] .v.RH ) ;  
											/* 96/Jan/10 */
    printchar ( 32 ) ;		/*   */
    print ( mem [ p ] .hh.b1 ) ;			/* character(p) */
  } 
} 

void zprintmark ( p ) 
integer p ; 
{printmark_regmem 
  printchar ( 123 ) ;		/* { */
  if ( ( p < himemmin ) || ( p > memend ) ) 
	  printesc ( 307 ) ;	/* CLOBBERED. */
  else showtokenlist ( mem [ p ] .hh .v.RH , 0 , maxprintline - 10 ) ; 
  printchar ( 125 ) ;		/* } */
} 

void zprintruledimen ( d ) 
scaled d ; 
{printruledimen_regmem 
  if ( ( d == -1073741824L ) )	/* - 2^30 */
  printchar ( 42 ) ;		/* * */
  else printscaled ( d ) ; 
} 

void zprintglue ( d , order , s ) 
scaled d ; 
integer order ; 
strnumber s ; 
{printglue_regmem 
  printscaled ( d ) ; 
  if ( ( order < 0 ) || ( order > 3 ) ) 
  print ( 308 ) ;	/* foul */
  else if ( order > 0 ) 
  {
    print ( 309 ) ; /* fil */
    while ( order > 1 ) {
	
      printchar ( 108 ) ; /* l */
      decr ( order ) ; 
    } 
  } 
  else if ( s != 0 ) 
  print ( s ) ; 
} 

void zprintspec ( p , s ) 
integer p ; 
strnumber s ; 
{printspec_regmem 
  if ( ( p < memmin ) || ( p >= lomemmax ) ) 
  printchar ( 42 ) ;		/* * */
  else {
      
    printscaled ( mem [ p + 1 ] .cint ) ; 
    if ( s != 0 ) 
    print ( s ) ; 
    if ( mem [ p + 2 ] .cint != 0 ) 
    {
      print ( 310 ) ; /*  plus */
      printglue ( mem [ p + 2 ] .cint , mem [ p ] .hh.b0 , s ) ; 
    } 
    if ( mem [ p + 3 ] .cint != 0 ) 
    {
      print ( 311 ) ; /*  minus */
      printglue ( mem [ p + 3 ] .cint , mem [ p ] .hh.b1 , s ) ; 
    } 
  } 
} 

void zprintfamandchar ( p ) 
halfword p ; 
{printfamandchar_regmem 
  printesc ( 461 ) ;			/* fam */
  printint ( mem [ p ] .hh.b0 ) ; 
  printchar ( 32 ) ;			/*    */
  print ( mem [ p ] .hh.b1 ) ; 
} 

void zprintdelimiter ( p ) 
halfword p ; 
{printdelimiter_regmem 
  integer a  ; 
  a = mem [ p ] .qqqq .b0 * 256 + mem [ p ] .qqqq .b1 ; 
  a = a * 4096 + mem [ p ] .qqqq .b2 * 256 + mem [ p ] .qqqq .b3 ; 
  if ( a < 0 ) 
  printint ( a ) ; 
  else printhex ( a ) ; 
} 

void zprintsubsidiarydata ( p , c ) 
halfword p ; 
ASCIIcode c ; 
{printsubsidiarydata_regmem 
  if ( ( poolptr - strstart [ strptr ] ) >= depththreshold ) 
  {
    if ( mem [ p ] .hh .v.RH != 0 ) 
    print ( 312 ) ; /* [] */
  } 
  else {
      
    {
      strpool [ poolptr ] = c ; 
      incr ( poolptr ) ; 
    } 
    tempptr = p ; 
    switch ( mem [ p ] .hh .v.RH ) 
    {case 1 : 
      {
	println () ; 
	printcurrentstring () ; 
	printfamandchar ( p ) ; 
      } 
      break ; 
    case 2 : 
      showinfo () ; 
      break ; 
    case 3 : 
      if ( mem [ p ] .hh .v.LH == 0 ) 
      {
	println () ; 
	printcurrentstring () ; 
	print ( 854 ) ;				/* {} */
      } 
      else showinfo () ; 
      break ; 
      default: 
      ; 
      break ; 
    } 
    decr ( poolptr ) ; 
  } 
} 

void zprintstyle ( c ) 
integer c ; 
{printstyle_regmem 
  switch ( c / 2 ) 
  {case 0 : 
    printesc ( 855 ) ;		/* displaystyle  */
    break ; 
  case 1 : 
    printesc ( 856 ) ;		/* textstyle */
    break ; 
  case 2 : 
    printesc ( 857 ) ;		/* scriptstyle */
    break ; 
  case 3 : 
    printesc ( 858 ) ;		/* scriptscriptstyle */
    break ; 
    default: 
    print ( 859 ) ;			/* Unknown */
    break ; 
  } 
} 

void zprintskipparam ( n ) 
integer n ; 
{printskipparam_regmem 
  switch ( n ) 
  {case 0 : 
    printesc ( 373 ) ;		/* lineskip */
    break ; 
  case 1 : 
    printesc ( 374 ) ;		/* baselineskip */
    break ; 
  case 2 : 
    printesc ( 375 ) ;		/* parskip */
    break ; 
  case 3 : 
    printesc ( 376 ) ;		/* abovedisplayskip */
    break ; 
  case 4 : 
    printesc ( 377 ) ;		/* belowdisplayskip */
    break ; 
  case 5 : 
    printesc ( 378 ) ;		/* abovedisplayshortskip */
    break ; 
  case 6 : 
    printesc ( 379 ) ;		/* belowdisplayshortskip */
    break ; 
  case 7 : 
    printesc ( 380 ) ;		/* leftskip */
    break ; 
  case 8 : 
    printesc ( 381 ) ;		/* rightskip */
    break ; 
  case 9 : 
    printesc ( 382 ) ;		/* topskip */
    break ; 
  case 10 : 
    printesc ( 383 ) ;		/* splittopskip */
    break ; 
  case 11 : 
    printesc ( 384 ) ;		/* tabskip */
    break ; 
  case 12 : 
    printesc ( 385 ) ;		/* spaceskip */
    break ; 
  case 13 : 
    printesc ( 386 ) ;		/* xspaceskip */
    break ; 
  case 14 : 
    printesc ( 387 ) ;		/* parfillskip */
    break ; 
  case 15 : 
    printesc ( 388 ) ;		/* thinmuskip */
    break ; 
  case 16 : 
    printesc ( 389 ) ;		/* medmuskip */
    break ; 
  case 17 : 
    printesc ( 390 ) ;		/* thickmuskip */
    break ; 
    default: 
    print ( 391 ) ;			/* [unknown glue parameter!] */
    break ; 
  } 
} 

void zshownodelist ( p ) 
integer p ; 
{/* 10 */ shownodelist_regmem 
  integer n  ; 
  real g  ; 
/* begin if cur_length>depth_threshold then */
  if ( ( poolptr - strstart [ strptr ] ) > depththreshold ) 
  {
/*    if ( p > 0 )  */	/* was p>null !!! line 3662 in tex.web */
    if ( p != 0 )		/* fixed 94/Mar/23 BUG FIX */
						/* NOTE: still not fixed in 3.14159 ! */
    print ( 312 ) ;		/* [] */
    return ; 
  } 
  n = 0 ; 
/*  while ( p > memmin ) { */	/* was p>memmin !!! line 3667 in tex.web */
  while ( p != 0 ) {			/* want p != null - bkph 93/Dec/15 */
								/* NOTE: still not fixed in 3.14159 ! */
    println () ; 
    printcurrentstring () ; 
    if ( p > memend ) 
    {
      print ( 313 ) ; /* Bad link, display aborted. */
      return ; 
    } 
    incr ( n ) ; 
    if ( n > breadthmax ) 
    {
      print ( 314 ) ;	/* etc. */
      return ; 
    } 
    if ( ( p >= himemmin ) ) 
		printfontandchar ( p ) ; 
    else switch ( mem [ p ] .hh.b0 ) 
    {case 0 : 
    case 1 : 
    case 13 : 
      {
	if ( mem [ p ] .hh.b0 == 0 ) 
		printesc ( 104 ) ;		/* h */
	else if ( mem [ p ] .hh.b0 == 1 ) 
		printesc ( 118 ) ;		/* v */
	else printesc ( 316 ) ;		/* unset */
	print ( 317 ) ;				/* box( */
	printscaled ( mem [ p + 3 ] .cint ) ; 
	printchar ( 43 ) ;			/* + */
	printscaled ( mem [ p + 2 ] .cint ) ; 
	print ( 318 ) ;				/* , shifted  */
	printscaled ( mem [ p + 1 ] .cint ) ; 
	if ( mem [ p ] .hh.b0 == 13 ) 
	{
	  if ( mem [ p ] .hh.b1 != 0 ) 
	  {
	    print ( 284 ) ;			/*  ( */
	    printint ( mem [ p ] .hh.b1 + 1 ) ; 
	    print ( 320 ) ;			/*  columns) */
	  } 
	  if ( mem [ p + 6 ] .cint != 0 ) 
	  {
	    print ( 321 ) ;			/* , stretch */
	    printglue ( mem [ p + 6 ] .cint , mem [ p + 5 ] .hh.b1 , 0 ) ; 
	  } 
	  if ( mem [ p + 4 ] .cint != 0 ) 
	  {
	    print ( 322 ) ;			/* , shrink */
	    printglue ( mem [ p + 4 ] .cint , mem [ p + 5 ] .hh.b0 , 0 ) ; 
	  } 
	} 
	else {
	    
	  g = mem [ p + 6 ] .gr ; 
	  if ( ( g != 0.0 ) && ( mem [ p + 5 ] .hh.b0 != 0 ) ) 
	  {
	    print ( 323 ) ;		/* , glue set */
	    if ( mem [ p + 5 ] .hh.b0 == 2 ) 
	    print ( 324 ) ;		/* -  */
	    if ( fabs ( g ) > 20000.0 ) 
	    {
	      if ( g > 0.0 ) 
	      printchar ( 62 ) ;	/* '>' */
	      else print ( 325 ) ;	/* < - */
	      printglue ( 20000 * 65536L , mem [ p + 5 ] .hh.b1 , 0 ) ; 
	    } 
	    else printglue ( round ( 65536L * g ) , mem [ p + 5 ] .hh.b1 , 0 ) 
	    ; 
	  } 
	  if ( mem [ p + 4 ] .cint != 0 ) 
	  {
	    print ( 319 ) ;		/* shifted */
	    printscaled ( mem [ p + 4 ] .cint ) ; 
	  } 
	} 
	{
	  {
	    strpool [ poolptr ] = 46 ; 
	    incr ( poolptr ) ; 
	  } 
	  shownodelist ( mem [ p + 5 ] .hh .v.RH ) ; 
	  decr ( poolptr ) ; 
	} 
      } 
      break ; 
    case 2 : 
      {
	printesc ( 326 ) ;	/* rule( */
	printruledimen ( mem [ p + 3 ] .cint ) ; 
	printchar ( 43 ) ;	/* '+' */
	printruledimen ( mem [ p + 2 ] .cint ) ; 
	print ( 318 ) ;		/* )x */
	printruledimen ( mem [ p + 1 ] .cint ) ; 
      } 
      break ; 
    case 3 : 
      {
	printesc ( 327 ) ;	/* insert */
	printint ( mem [ p ] .hh.b1 ) ; 
	print ( 328 ) ;		/* , natural size */
	printscaled ( mem [ p + 3 ] .cint ) ; 
	print ( 329 ) ;		/* ; split( */
	printspec ( mem [ p + 4 ] .hh .v.RH , 0 ) ; 
	printchar ( 44 ) ;	/* ',' */
	printscaled ( mem [ p + 2 ] .cint ) ; 
	print ( 330 ) ;		/* (; float cost */
	printint ( mem [ p + 1 ] .cint ) ; 
	{
	  {
	    strpool [ poolptr ] = 46 ; 
	    incr ( poolptr ) ; 
	  } 
	  shownodelist ( mem [ p + 4 ] .hh .v.LH ) ; 
	  decr ( poolptr ) ; 
	} 
      } 
      break ; 
    case 8 : 
      switch ( mem [ p ] .hh.b1 ) 
      {case 0 : 
	{
	  printwritewhatsit ( 1279 , p ) ;		/* debug # (-1 to exit): */
	  printchar ( 61 ) ;	/* = */
	  printfilename ( mem [ p + 1 ] .hh .v.RH , mem [ p + 2 ] .hh .v.LH , 
	  mem [ p + 2 ] .hh .v.RH ) ; 
	} 
	break ; 
      case 1 : 
	{
	  printwritewhatsit ( 591 , p ) ;	/* write */
	  printmark ( mem [ p + 1 ] .hh .v.RH ) ; 
	} 
	break ; 
      case 2 : 
	printwritewhatsit ( 1280 , p ) ;	/* closeout */
	break ; 
      case 3 : 
	{
	  printesc ( 1281 ) ;				/* special */
	  printmark ( mem [ p + 1 ] .hh .v.RH ) ; 
	} 
	break ; 
      case 4 : 
	{
	  printesc ( 1283 ) ;		/* setlanguage */
	  printint ( mem [ p + 1 ] .hh .v.RH ) ; 
	  print ( 1286 ) ;			/*  (hyphenmin */
	  printint ( mem [ p + 1 ] .hh.b0 ) ; 
	  printchar ( 44 ) ;		/* , */
	  printint ( mem [ p + 1 ] .hh.b1 ) ; 
	  printchar ( 41 ) ;		/* ) */
	} 
	break ; 
	default: 
	print ( 1287 ) ;		/* whatsit */
	break ; 
      } 
      break ; 
    case 10 : 
      if ( mem [ p ] .hh.b1 >= 100 ) 
      {
	printesc ( 335 ) ;	/*  */
	if ( mem [ p ] .hh.b1 == 101 ) 
	printchar ( 99 ) ;	/* c */
	else if ( mem [ p ] .hh.b1 == 102 ) 
	printchar ( 120 ) ; /* x */
	print ( 336 ) ;		/* leaders  */
	printspec ( mem [ p + 1 ] .hh .v.LH , 0 ) ; 
	{
	  {
	    strpool [ poolptr ] = 46 ; 
	    incr ( poolptr ) ; 
	  } 
	  shownodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
	  decr ( poolptr ) ; 
	} 
      } 
      else {
	  
	printesc ( 331 ) ;	/* glue */
	if ( mem [ p ] .hh.b1 != 0 ) 
	{
	  printchar ( 40 ) ;	/* ( */
	  if ( mem [ p ] .hh.b1 < 98 ) 
		  printskipparam ( mem [ p ] .hh.b1 - 1 ) ; 
	  else if ( mem [ p ] .hh.b1 == 98 ) 
		  printesc ( 332 ) ;	/* nonscript */
	  else printesc ( 333 ) ; /* mskip */
	  printchar ( 41 ) ;	/* ) */
	} 
	if ( mem [ p ] .hh.b1 != 98 ) 
	{
	  printchar ( 32 ) ;	/*   */
	  if ( mem [ p ] .hh.b1 < 98 ) 
	  printspec ( mem [ p + 1 ] .hh .v.LH , 0 ) ; 
	  else printspec ( mem [ p + 1 ] .hh .v.LH , 334 ) ; /* mu */
	} 
      } 
      break ; 
    case 11 : 
      if ( mem [ p ] .hh.b1 != 99 ) 
      {
	printesc ( 337 ) ;	/* kern */
	if ( mem [ p ] .hh.b1 != 0 ) 
	printchar ( 32 ) ;		/*   */
	printscaled ( mem [ p + 1 ] .cint ) ; 
	if ( mem [ p ] .hh.b1 == 2 ) 
	print ( 338 ) ;		/*  (for accent) */
      } 
      else {
	printesc ( 339 ) ;	/* mkern */
	printscaled ( mem [ p + 1 ] .cint ) ; 
	print ( 334 ) ;		/* mu */
      } 
      break ; 
    case 9 : 
      {
	printesc ( 340 ) ;	/* math */
	if ( mem [ p ] .hh.b1 == 0 ) 
		print ( 341 ) ;		/* on */
	else print ( 342 ) ;	/* off */ 
	if ( mem [ p + 1 ] .cint != 0 ) 
	{
	  print ( 343 ) ;	/* , surrounded */
	  printscaled ( mem [ p + 1 ] .cint ) ; 
	} 
      } 
      break ; 
    case 6 : 
      {
	printfontandchar ( p + 1 ) ; 
	print ( 344 ) ;		/* (ligature */
	if ( mem [ p ] .hh.b1 > 1 ) 
	printchar ( 124 ) ; /* | */
	fontinshortdisplay = mem [ p + 1 ] .hh.b0 ; 
	shortdisplay ( mem [ p + 1 ] .hh .v.RH ) ; 
	if ( odd ( mem [ p ] .hh.b1 ) ) 
	printchar ( 124 ) ; /* | */
	printchar ( 41 ) ;	/* ) */
      } 
      break ; 
    case 12 : 
      {
	printesc ( 345 ) ;	/* penalty  */
	printint ( mem [ p + 1 ] .cint ) ; 
      } 
      break ; 
    case 7 : 
      {
	printesc ( 346 ) ;	/* discretionary */
	if ( mem [ p ] .hh.b1 > 0 ) 
	{
	  print ( 347 ) ;	/*  replacing  */
	  printint ( mem [ p ] .hh.b1 ) ; 
	} 
	{
	  {
	    strpool [ poolptr ] = 46 ; 
	    incr ( poolptr ) ; 
	  } 
	  shownodelist ( mem [ p + 1 ] .hh .v.LH ) ; 
	  decr ( poolptr ) ; 
	} 
	{
	  strpool [ poolptr ] = 124 ; 
	  incr ( poolptr ) ; 
	} 
	shownodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
	decr ( poolptr ) ; 
      } 
      break ; 
    case 4 : 
      {
	printesc ( 348 ) ;	/* mark */
	printmark ( mem [ p + 1 ] .cint ) ; 
      } 
      break ; 
    case 5 : 
      {
	printesc ( 349 ) ;	/* vadjust */
	{
	  {
	    strpool [ poolptr ] = 46 ; 
	    incr ( poolptr ) ; 
	  } 
	  shownodelist ( mem [ p + 1 ] .cint ) ; 
	  decr ( poolptr ) ; 
	} 
      } 
      break ; 
    case 14 : 
      printstyle ( mem [ p ] .hh.b1 ) ; 
      break ; 
    case 15 : 
      {
	printesc ( 522 ) ;		/* mathchoice */
	{
	  strpool [ poolptr ] = 68 ; 
	  incr ( poolptr ) ; 
	} 
	shownodelist ( mem [ p + 1 ] .hh .v.LH ) ; 
	decr ( poolptr ) ; 
	{
	  strpool [ poolptr ] = 84 ; 
	  incr ( poolptr ) ; 
	} 
	shownodelist ( mem [ p + 1 ] .hh .v.RH ) ; 
	decr ( poolptr ) ; 
	{
	  strpool [ poolptr ] = 83 ; 
	  incr ( poolptr ) ; 
	} 
	shownodelist ( mem [ p + 2 ] .hh .v.LH ) ; 
	decr ( poolptr ) ; 
	{
	  strpool [ poolptr ] = 115 ; 
	  incr ( poolptr ) ; 
	} 
	shownodelist ( mem [ p + 2 ] .hh .v.RH ) ; 
	decr ( poolptr ) ; 
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
    case 30 : 
    case 31 : 
      {
	switch ( mem [ p ] .hh.b0 ) 
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
	case 27 : 
	  printesc ( 868 ) ;		/* overline */
	  break ; 
	case 26 : 
	  printesc ( 869 ) ;		/* underline */
	  break ; 
	case 29 : 
	  printesc ( 536 ) ;		/* vcenter */
	  break ; 
	case 24 : 
	  {
	    printesc ( 530 ) ;		/* radical */
	    printdelimiter ( p + 4 ) ; 
	  } 
	  break ; 
	case 28 : 
	  {
	    printesc ( 505 ) ;		/* accent */
	    printfamandchar ( p + 4 ) ; 
	  } 
	  break ; 
	case 30 : 
	  {
	    printesc ( 870 ) ;		/* left */
	    printdelimiter ( p + 1 ) ; 
	  } 
	  break ; 
	case 31 : 
	  {
	    printesc ( 871 ) ;		/* right */
	    printdelimiter ( p + 1 ) ; 
	  } 
	  break ; 
	} 
	if ( mem [ p ] .hh.b1 != 0 ) 
	if ( mem [ p ] .hh.b1 == 1 ) 
		printesc ( 872 ) ;		/* limits */
	else printesc ( 873 ) ;			/* nolimits */
	if ( mem [ p ] .hh.b0 < 30 ) 
	printsubsidiarydata ( p + 1 , 46 ) ; 
	printsubsidiarydata ( p + 2 , 94 ) ; 
	printsubsidiarydata ( p + 3 , 95 ) ; 
      } 
      break ; 
    case 25 : 
      {
	printesc ( 874 ) ;		/* fraction */
	if ( mem [ p + 1 ] .cint == 1073741824L )	/* 2^30 */
	print ( 875 ) ;			/* = default */
	else printscaled ( mem [ p + 1 ] .cint ) ; 
	if ( ( mem [ p + 4 ] .qqqq .b0 != 0 ) || ( mem [ p + 4 ] .qqqq .b1 != 
	0 ) || ( mem [ p + 4 ] .qqqq .b2 != 0 ) || ( mem [ p + 4 ] .qqqq .b3 
	!= 0 ) ) 
	{
	  print ( 876 ) ;		/* , left */
	  printdelimiter ( p + 4 ) ; 
	} 
	if ( ( mem [ p + 5 ] .qqqq .b0 != 0 ) || ( mem [ p + 5 ] .qqqq .b1 != 
	0 ) || ( mem [ p + 5 ] .qqqq .b2 != 0 ) || ( mem [ p + 5 ] .qqqq .b3 
	!= 0 ) ) 
	{
	  print ( 877 ) ;		/* , right */
	  printdelimiter ( p + 5 ) ; 
	} 
	printsubsidiarydata ( p + 2 , 92 ) ; 
	printsubsidiarydata ( p + 3 , 47 ) ; 
      } 
      break ; 
      default: 
      print ( 315 ) ; /* Unknown node type! */
      break ; 
    } 
    p = mem [ p ] .hh .v.RH ; 
  } 
} 

/* NOTE: 262143L should be emptyflag */