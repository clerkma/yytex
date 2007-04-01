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

// extern char logline[];			// in local.c

/* #pragma optimize("a", off) */ 					/* 98/Dec/10 experiment */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* end of the old tex8.c */

void giveerrhelp ( ) 
{giveerrhelp_regmem 
  tokenshow ( eqtb [ (hash_size + 1321) ] .hh .v.RH ) ; 
} 

booleane openfmtfile ( ) 
{/* 40 10 */ register booleane Result; openfmtfile_regmem 
  integer j  ; 
  j = curinput .locfield ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*		For Windows NT, lets allow + instead of & for format specification */
/*  if ( buffer [ curinput .locfield ] == 38 )  */	/* 95/Jan/22 */
  if ( buffer [ curinput .locfield ] == '&' ||
	   buffer [ curinput .locfield ] == '+' )
  {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*	User specified a format name on the command line */
    incr ( curinput .locfield ) ; 
    j = curinput .locfield ; 
    buffer [ last ] = 32 ; 
    while ( buffer [ j ] != 32 ) incr ( j ) ; 
    packbufferedname ( 0 , curinput .locfield , j - 1 ) ; 
    if ( wopenin ( fmtfile ) ) 
		goto lab40 ;	// format file opened OK
	
//	format file open failed
	if (knuthflag) {
		(void) sprintf(logline , "%s;%s\n",
			"Sorry, I can't find that format" , " will try the default." ) ;
		showline(logline, 1);
	}
	else {
		char *s=logline;
/*		null_terminate (nameoffile + 1); */
		nameoffile[namelength + 1] = '\0';	/* null terminate */
		(void) sprintf( s, "%s (%s);%s\n",
			"Sorry, I can't find that format" , nameoffile+1,
					" will try the default.") ; 
/*		space_terminate (nameoffile + 1); */
		nameoffile[namelength + 1] = ' ';	/* space terminate */
		s += strlen(s);
		(void) sprintf(s,
			"(Perhaps your %s environment variable is not set correctly)\n",
						"TEXFORMATS");
		s += strlen(s);
		{
			char *t;						/* extra info 97/June/13 */
			if ((t = grabenv("TEXFORMATS")) != NULL) {
			    sprintf(s, "(%s=%s)\n", "TEXFORMATS", t);
			}
			else {
				sprintf(s, "%s environment variable not set\n", "TEXFORMATS");
			}
		}
		showline(logline, 1);	// show all three lines at once
	}
#ifndef _WINDOWS
    fflush ( stdout ) ; 
#endif
  } 
/*	Try the default format (either because no format specified or failed) */
  packbufferedname ( formatdefaultlength - 4 , 1 , 0 ) ; 
  if ( ! wopenin ( fmtfile ) ) 
  {
    ; 
	if (knuthflag) {
	    (void) sprintf( logline , "%s!\n",
			"I can't find the default format file" ) ;
		showline(logline, 1);
	}
	else {
		char *s=logline;
/*		null_terminate (nameoffile + 1); */
		nameoffile[namelength + 1] = '\0';	/* null terminate */
		(void) sprintf( s, "%s (%s)!\n",
			"I can't find the default format file" , nameoffile+1) ; 
/*		space_terminate (nameoffile + 1); */
		nameoffile[namelength + 1] = ' ';	/* space terminate */
		s += strlen(s);
		(void) sprintf(s,
			"(Perhaps your %s environment variable is not set correctly)\n", 
						"TEXFORMATS");
		s += strlen(s);
		{
			char *t;						/* extra info 97/June/13 */
			if ((t = grabenv("TEXFORMATS")) != NULL) {
				sprintf(s, "(%s=%s)\n", "TEXFORMATS", t);
			}
			else {
				sprintf(s, "%s environment variable not set\n", "TEXFORMATS");
			}
		}
		showline(logline, 1);		// show all three lines at once
	}
    Result = false ; 
    return(Result) ; 
  } 
  lab40: curinput .locfield = j ; 
  Result = true ; 
  return Result ; 
} 

/**************************************************************************/

void printcharstring(unsigned char *s) {			// 2000 Jun 18
	while (*s > 0) printchar(*s++);
}

void showfontinfo (void);		// now in local.c

extern int closedalready;			// make sure we don't try this more than once

/* The following needs access to zdvibuf of ALLOCATEDVIBUF 94/Mar/24 */
/* done in closefilesandterminate_regmem  in coerce.h */

void closefilesandterminate ( )
{closefilesandterminate_regmem 
  integer k  ; 

	if (closedalready++) {
		showline("CLOSEDFILESANDTERMINATED ALREADY ", 0);
		return;			// sanity check
	}
	if (traceflag) showline("\nCLOSEFILESANDTERMINATE ", 0);
//	close all open files
	{
	  register integer for_end; 
	  k = 0 ; 
	  for_end = 15 ;				/* CHECK LIMIT */
	  if ( k <= for_end) do 
		  if ( writeopen [ k ] ) {
			  (void) aclose ( writefile [ k ] ) ;
		  }
	  while ( k++ < for_end ) ;
	} 
	;

#ifdef STAT
/* if tracing_stats>0 then @<Output statistics about this job@>; */
/*  if ( eqtb [ (hash_size + 3194) ] .cint > 0 )  */
  if ( eqtb [ (hash_size + 3194) ] .cint > 0 ||
/*	   traceflag != 0) *//* 93/Nov/30 - bkph */
	   verboseflag != 0)	/* 93/Nov/30 - bkph */
  if ( logopened )  {
/*	 used to output paragraph breaking statistics here */
	  (void) fprintf( logfile , "%c\n",  ' ' ) ; 
	  (void) fprintf( logfile , "\n") ; 
	  (void) fprintf( logfile , "%s%s\n",  "Here is how much of TeX's memory" , " you used:" ) ; 
	  (void) fprintf( logfile , "%c%ld%s",  ' ' , (long)strptr - initstrptr , " string" ) ; 
	  if ( strptr != initstrptr + 1 ) 
		  (void) putc ( 's' ,  logfile );
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
	if (showcurrent)
		(void) fprintf( logfile , "%s%ld\n",  " out of " ,
					(long) currentmaxstrings - initstrptr ) ; 
	else
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    (void) fprintf( logfile , "%s%ld\n",  " out of " ,
					(long) maxstrings - initstrptr ) ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
	if (showcurrent)
		(void) fprintf( logfile , "%c%ld%s%ld\n",  ' ' ,
					(long) poolptr - initpoolptr ,
					" string characters out of " ,
					(long) currentpoolsize - initpoolptr ) ;
	else
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    (void) fprintf( logfile , "%c%ld%s%ld\n",  ' ' ,
					(long) poolptr - initpoolptr ,
					" string characters out of " ,
					(long) poolsize - initpoolptr ) ;
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEMAIN
	if (showcurrent)
		(void) fprintf( logfile , "%c%ld%s%ld\n",  ' ' , (long)lomemmax - memmin + memend - himemmin + 2 ,     " words of memory out of " , (long)currentmemsize ) ; 
	else
#endif
		(void) fprintf( logfile , "%c%ld%s%ld\n",  ' ' , (long)lomemmax - memmin + memend - himemmin + 2 ,     " words of memory out of " , (long)memend + 1 - memmin ) ;
/*    (void) fprintf( logfile , "%c%ld%s%ld\n",  ' ' , (long)lomemmax - memmin + memend - himemmin + 2 ,     " words of memory out of " , (long)maxmemsize ) ; */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    (void) fprintf( logfile , "%c%ld%s%ld\n",  ' ' , (long)cscount ,
					" multiletter control sequences out of " , (long)(hash_size + hash_extra) ) ; 
    (void) fprintf( logfile , "%c%ld%s%ld%s",  ' ' , (long)fmemptr ,
					" words of font info for " , (long)fontptr - 0     , " font" ) ; 
    if ( fontptr != 1 ) 
		(void) putc ( 's' ,  logfile );
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEFONT
	if (showcurrent)
		(void) fprintf( logfile , "%s%ld%s%ld\n",
			", out of " , (long)currentfontmemsize , " for " , (long)fontmax - 0 ) ; 
	 else
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    (void) fprintf( logfile , "%s%ld%s%ld\n",
			", out of " , (long)fontmemsize , " for " , (long)fontmax - 0 ) ; 
    (void) fprintf( logfile , "%c%ld%s",  ' ' , (long)hyphcount , " hyphenation exception" ) ; 
    if ( hyphcount != 1 ) (void) putc ( 's' ,  logfile );
/*  (void) fprintf( logfile , "%s%ld\n",  " out of " , (long)607 ) ;  */
    (void) fprintf( logfile , "%s%ld\n",  " out of " , (long) hyphen_prime ) ; 
	(void) fprintf( logfile , " ");
    (void) fprintf( logfile , "%ld%s", (long)maxinstack, "i,");
	(void) fprintf( logfile , "%ld%s", (long)maxneststack , "n,");
	(void) fprintf( logfile , "%ld%s", (long)maxparamstack , "p,");
	(void) fprintf( logfile , "%ld%s", (long)maxbufstack + 1 , "b,");
	(void) fprintf( logfile , "%ld%s", (long)maxsavestack + 6 , "s");
	(void) fprintf( logfile , " stack positions out of ");
#ifdef ALLOCATESAVESTACK
	if (showcurrent)
		(void) fprintf( logfile , "%ld%s", (long)currentstacksize , "i,");
	else
#endif
		(void) fprintf( logfile , "%ld%s", (long)stacksize , "i,");
#ifdef ALLOCATENESTSTACK
	if (showcurrent)
		(void) fprintf( logfile , "%ld%s", (long)currentnestsize , "n,");
	else
#endif
		(void) fprintf( logfile , "%ld%s", (long)nestsize , "n,");
#ifdef ALLOCATEPARAMSTACK
	if (showcurrent)
		(void) fprintf( logfile , "%ld%s", (long)currentparamsize , "p,");
	else
#endif
		(void) fprintf( logfile , "%ld%s", (long)paramsize , "p,");
#ifdef ALLOCATEBUFFER
	if (showcurrent)
		(void) fprintf( logfile , "%ld%s", (long)currentbufsize , "b,");
	else
#endif
		(void) fprintf( logfile , "%ld%s", (long)bufsize , "b,");
#ifdef ALLOCATESAVESTACK
	if (showcurrent)
		(void) fprintf( logfile , "%ld%s", (long)currentsavesize , "s" ) ; 
	else
#endif
		(void) fprintf( logfile , "%ld%s", (long)savesize , "s" ) ; 
	(void) fprintf( logfile , "\n");
/************************************************************************/
	if (! knuthflag) fprintf( logfile ,
	 " (i = instack, n = neststack, p = paramstack, b = bufstack, s = savestack)\n");
/************************************************************************/
	if (! knuthflag)					/* 1999/Jan/17 */
		fprintf( logfile ,
				 " %d inputs open max out of %d\n",	/* (%d max parens open) */
					 highinopen, maxinopen);	/* maxopenparens */
/************************************************************************/
/*	Modified 98/Jan/14 to leave out lines with zero counts */
	if (showlinebreakstats && firstpasscount > 0) {			/* 96/Feb/8 */
		int firstcount, secondcount, thirdcount;
		(void) fprintf( logfile , "\nSuccess at breaking %d paragraph%s:",
					  firstpasscount, (firstpasscount == 1) ? "" : "s");
		if (singleline > 0)
			(void) fprintf( logfile , "\n %d single line `paragraph%s'",
							singleline, (singleline == 1) ? "" : "s");	/* 96/Apr/23 */
		firstcount = firstpasscount-singleline-secondpasscount;
		if (firstcount < 0) firstcount = 0;				/* sanity check */
		secondcount = secondpasscount-finalpasscount;
		thirdcount = finalpasscount-paragraphfailed;
		if (firstcount != 0 || secondcount != 0 || thirdcount != 0) 
			(void) fprintf( logfile , "\n %d first pass (\\pretolerance = %d)",
						firstcount, eqtb [ (hash_size + 3163) ] .cint);
		if (secondcount != 0 || thirdcount != 0) 
			(void) fprintf( logfile , "\n %d second pass (\\tolerance = %d)",
						secondcount, eqtb [ (hash_size + 3164) ] .cint);
		if (finalpasscount > 0 || eqtb [ (hash_size + 3750) ] .cint  > 0) {
			(void) fprintf( logfile , "\n %d third pass (\\emergencystretch = %lgpt)",
							thirdcount,
							(double) eqtb [ (hash_size + 3750) ] .cint / 65536.0);
/*			above converted from scaled points to printer's points */
		}
		if (paragraphfailed > 0)
			(void) fprintf( logfile, "\n %d failed", paragraphfailed);
		(void) putc('\n', logfile);
		if (overfullhbox > 0) 
			(void) fprintf( logfile, "\n %d overfull \\hbox%s",
							overfullhbox, (overfullhbox > 1) ? "es" : ""); 
		if (underfullhbox > 0) 
			(void) fprintf( logfile, "\n %d underfull \\hbox%s",
							underfullhbox, (underfullhbox > 1) ? "es" : ""); 
		if (overfullvbox > 0) 
			(void) fprintf( logfile, "\n %d overfull \\vbox%s",
							overfullvbox, (overfullvbox > 1) ? "es" : ""); 
		if (underfullvbox > 0)
			(void) fprintf( logfile, "\n %d underfull \\vbox%s",
							underfullvbox, (underfullvbox > 1) ? "es" : ""); 
		if (overfullhbox || underfullhbox || overfullvbox || underfullvbox)
			(void) putc('\n', logfile);
	}
/************************************************************************/
  } /* end of if (logopened) */ 
#endif /* STAT */
  while ( curs > -1 ) {
      
    if ( curs > 0 ) {
      dvibuf [ dviptr ] = 142 ; 
      incr ( dviptr ) ; 
      if ( dviptr == dvilimit ) dviswap () ; 
    } 
    else {
      {
		  dvibuf [ dviptr ] = 140 ; 
		  incr ( dviptr ) ; 
		  if ( dviptr == dvilimit ) dviswap () ; 
      } 
      incr ( totalpages ) ; 
    } 
    decr ( curs ) ; 
  } 

  if ( totalpages == 0 ) printnl ( 831 ) ;	/* No pages of output. */
  else {
    {
		dvibuf [ dviptr ] = 248 ;		/* post - start of postamble */
		incr ( dviptr ) ; 
		if ( dviptr == dvilimit ) dviswap () ; 
	} 
    dvifour ( lastbop ) ; 
    lastbop = dvioffset + dviptr - 5 ; 
    dvifour ( 25400000L ) ;			/* magic DVI scale factor */ 
    dvifour ( 473628672L ) ;		/* 7227 * 65536 */
    preparemag () ;					/* in tex2.c */
	ABORTCHECK;
    dvifour ( eqtb [ (hash_size + 3180) ] .cint ) ;		/* mag */
    dvifour ( maxv ) ;				/* max height + depth */
    dvifour ( maxh ) ;				/* max width */
    {
		dvibuf [ dviptr ] = maxpush / 256 ; 
		incr ( dviptr ) ; 
		if ( dviptr == dvilimit ) dviswap () ; 
    } 
    {
		dvibuf [ dviptr ] = maxpush % 256 ;  
		incr ( dviptr ) ; 
		if ( dviptr == dvilimit ) dviswap () ; 
    } 
	if (totalpages >= 65536) {		// 99/Oct/10 dvi_t 16 bit problem
		sprintf(logline, "\nWARNING: page count (dvi_t) in DVI file will be %ld not %ld\n",
			   (totalpages % 65536), totalpages);
		if (logopened) fputs (logline, logfile);
		showline(logline, 1);
	}
    {
		dvibuf [ dviptr ] = ( totalpages / 256 ) % 256 ;  
		incr ( dviptr ) ; 
		if ( dviptr == dvilimit ) dviswap () ; 
    } 
    {
		dvibuf [ dviptr ] = totalpages % 256 ;  
		incr ( dviptr ) ; 
		if ( dviptr == dvilimit )  dviswap () ; 
    } 

	if (showfontsused && logopened) 		/* 97/Dec/24 */
		showfontinfo();						// now in local.c

    while ( fontptr > 0 ) {
		if ( fontused [ fontptr ] ) dvifontdef ( fontptr ) ;
		decr ( fontptr ) ; 
    } 
    {
		dvibuf [ dviptr ] = 249 ;		/* post_post end of postamble */
		incr ( dviptr ) ; 
		if ( dviptr == dvilimit ) dviswap () ; 
    } 
    dvifour ( lastbop ) ; 
    {
		dvibuf [ dviptr ] = 2 ; 
		incr ( dviptr ) ; 
		if ( dviptr == dvilimit ) dviswap () ; 
    } 
    k = 4 + ( ( dvibufsize - dviptr ) % 4 ) ; 
    while ( k > 0 ) {
      {
		  dvibuf [ dviptr ] = 223 ;	/* four to seven bytes of 223 */
		  incr ( dviptr ) ; 
		  if ( dviptr == dvilimit ) dviswap () ; 
	  } 
	  decr ( k ) ; 
    } 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	if (traceflag) {					/* 93/Dec/28 - bkph */
		sprintf(logline, "\ndviwrite %d", dvigone);
		showline(logline, 0);
	}
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if ( dvilimit == halfbuf ) writedvi ( halfbuf , dvibufsize - 1 ) ; 
    if ( dviptr > 0 ) writedvi ( 0 , dviptr - 1 ) ; 
    printnl ( 832 ) ;		/* Output written on  */
	if (fullfilenameflag && dvifilename != NULL) 
		printcharstring(dvifilename);
	else slowprint ( outputfilename ) ; 
    print ( 284 ) ;			/*  ( */
    printint ( totalpages ) ; 
    print ( 833 ) ;			/*  page */
    if ( totalpages != 1 ) 	printchar ( 115 ) ;		/* s */
    print ( 834 ) ;			/* ,  */
    printint ( dvioffset + dviptr ) ; 
    print ( 835 ) ;			/* bytes). */
    bclose ( dvifile ) ; 
  } 
  if ( logopened )  {
    (void) putc ('\n',  logfile );
    (void) aclose ( logfile ) ; 
    selector = selector - 2 ; 
    if ( selector == 17 )  {
		printnl ( 1269 ) ;	/* 	Transcript written on  */
		if (fullfilenameflag && logfilename != NULL) 
			printcharstring(logfilename);
		else slowprint ( texmflogname ) ; 
		printchar ( 46 ) ;	/* . */
    } 
  } 
  println () ; 
  if ( ( editnamestart != 0 ) && ( interaction > 0 ) ) {
	  calledit ( strpool , editnamestart , editnamelength , editline ) ;
  }
} /* end of closefilesandterminate */

#ifdef DEBUG
void debughelp ( ) 
{/* 888 10 */ debughelp_regmem 
  integer k, l, m, n  ; 
  while ( true ) {
      
    ; 
    printnl ( 1278 ) ;	/* 	debug # (-1 to exit): */
#ifndef _WINDOWS
    fflush ( stdout ) ; 
#endif
    read ( stdin , m ) ;	// ???
    if ( m < 0 ) return ; 
    else if ( m == 0 ) 
		dumpcore () ; 
    else {
      read ( stdin , n ) ;	// ???
      switch ( m ) 
      {case 1 : 
	printword ( mem [ n ] ) ; 
	break ; 
      case 2 : 
	printint ( mem [ n ] .hh .v.LH ) ; 
	break ; 
      case 3 : 
	printint ( mem [ n ] .hh .v.RH ) ; 
	break ; 
      case 4 : 
	printword ( eqtb [ n ] ) ; 
	break ; 
      case 5 : 
#ifdef SHORTFONTINFO
	printscaled ( fontinfo [ n ] .sc ) ;  printchar ( ' ' );
	printint ( fontinfo [ n ] .qqq.b0 ) ;  printchar ( ':' );
	printint ( fontinfo [ n ] .qqq.b1 ) ;  printchar ( ':' );
	printint ( fontinfo [ n ] .qqq.b2 ) ;  printchar ( ':' );
	printint ( fontinfo [ n ] .qqq.b3 ) ;  
#else
	printword ( fontinfo [ n ] ) ; 
#endif
	break ; 
      case 6 : 
	printword ( savestack [ n ] ) ; 
	break ; 
      case 7 : 
	showbox ( n ) ; 
	break ; 
      case 8 : 
	{
	  breadthmax = 10000 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
/* About to output node list make some space in string pool 97/Mar/9 */
	if ( poolptr + 32000 > currentpoolsize)
		strpool = reallocstrpool (incrementpoolsize);
/* We don't bother to check whether this worked */
#endif
#ifdef ALLOCATESTRING
	  depththreshold = currentpoolsize - poolptr - 10 ; 
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	  depththreshold = poolsize - poolptr - 10 ; 
#endif
	  shownodelist ( n ) ; 
	} 
	break ; 
      case 9 : 
	showtokenlist ( n , 0 , 1000 ) ; 
	break ; 
      case 10 : 
	slowprint ( n ) ; 
	break ; 
      case 11 : 
	checkmem ( n > 0 ) ; 
	break ; 
      case 12 : 
	searchmem ( n ) ; 
	break ; 
      case 13 : 
	{
	  read ( stdin , l ) ;	// ???
	  printcmdchr ( n , l ) ; 
	} 
	break ; 
      case 14 : 
	{
		register integer for_end; 
		k = 0 ; 
		for_end = n ; 
		if ( k <= for_end) 
			do print ( buffer [ k ] ) ; 
		while ( k++ < for_end ) ;
	} 
	break ; 
      case 15 : 
	{
	  fontinshortdisplay = 0 ; 
	  shortdisplay ( n ) ; 
	} 
	break ; 
      case 16 : 
	panicking = ! panicking ; 
	break ; 
	default: 
	print ( 63 ) ;		/* ? */
	break ; 
      } 
    } 
  } 
} 
#endif /* DEBUG */
