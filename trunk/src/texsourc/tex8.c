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

/* mathfraction etc used to be in tex7.c */

void mathfraction ( ) 
{mathfraction_regmem 
  smallnumber c  ; 
  c = curchr ; 
  if ( curlist .auxfield .cint != 0 ) 
  {
    if ( c >= 3 ) 
    {
      scandelimiter ( memtop - 12 , false ) ; 
      scandelimiter ( memtop - 12 , false ) ; 
    } 
    if ( c % 3 == 0 ) 
    scandimen ( false , false , false ) ; 
	ABORTCHECK;
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 1147 ) ;		/* Ambiguous; you need another { and } */
    } 
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 1148 ;	/* I'm ignoring this fraction specification, since I don't */
      helpline [ 1 ] = 1149 ;	/* know whether a construction like `x \over y \over z' */
      helpline [ 0 ] = 1150 ;	/* means `{x \over y} \over z' or `x \over {y \over z}'. */
    } 
    error () ; 
	ABORTCHECK;
  } 
  else {
      
    curlist .auxfield .cint = getnode ( 6 ) ; 
    mem [ curlist .auxfield .cint ] .hh.b0 = 25 ; 
    mem [ curlist .auxfield .cint ] .hh.b1 = 0 ; 
    mem [ curlist .auxfield .cint + 2 ] .hh .v.RH = 3 ; 
    mem [ curlist .auxfield .cint + 2 ] .hh .v.LH = mem [ curlist .headfield ] 
    .hh .v.RH ; 
    mem [ curlist .auxfield .cint + 3 ] .hh = emptyfield ; 
    mem [ curlist .auxfield .cint + 4 ] .qqqq = nulldelimiter ; 
    mem [ curlist .auxfield .cint + 5 ] .qqqq = nulldelimiter ; 
    mem [ curlist .headfield ] .hh .v.RH = 0 ; 
    curlist .tailfield = curlist .headfield ; 
    if ( c >= 3 ) 
    {
      scandelimiter ( curlist .auxfield .cint + 4 , false ) ; 
      scandelimiter ( curlist .auxfield .cint + 5 , false ) ; 
    } 
    switch ( c % 3 ) 
    {case 0 : 
      {
	scandimen ( false , false , false ) ; 
	ABORTCHECK;
	mem [ curlist .auxfield .cint + 1 ] .cint = curval ; 
      } 
      break ; 
    case 1 : 
      mem [ curlist .auxfield .cint + 1 ] .cint = 1073741824L ;  /* 2^30 */
      break ; 
    case 2 : 
      mem [ curlist .auxfield .cint + 1 ] .cint = 0 ; 
      break ; 
    } 
  } 
} 

void mathleftright ( ) 
{mathleftright_regmem 
  smallnumber t  ; 
  halfword p  ; 
  t = curchr ; 
  if ( ( t == 31 ) && ( curgroup != 16 ) ) 
  {
    if ( curgroup == 15 ) 
    {
      scandelimiter ( memtop - 12 , false ) ; 
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;		/* !  */
	print ( 773 ) ;			/* Extra  */
      } 
      printesc ( 871 ) ;	/* right */
      {
	helpptr = 1 ; 
	helpline [ 0 ] = 1151 ;		/* I'm ignoring a \right that had no matching \left. */
      } 
      error () ; 
	  ABORTCHECK;
    } 
    else {
		offsave () ;
		ABORTCHECK;
	}
  } 
  else {
      
    p = newnoad () ; 
    mem [ p ] .hh.b0 = t ; 
    scandelimiter ( p + 1 , false ) ; 
    if ( t == 30 ) 
    {
      pushmath ( 16 ) ; 
      mem [ curlist .headfield ] .hh .v.RH = p ; 
      curlist .tailfield = p ; 
    } 
    else {
	
      p = finmlist ( p ) ; 
      unsave () ; 
      {
	mem [ curlist .tailfield ] .hh .v.RH = newnoad () ; 
	curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
      mem [ curlist .tailfield ] .hh.b0 = 23 ; 
      mem [ curlist .tailfield + 1 ] .hh .v.RH = 3 ; 
      mem [ curlist .tailfield + 1 ] .hh .v.LH = p ; 
    } 
  } 
} 

void aftermath ( ) 
{aftermath_regmem 
  booleane l  ; 
  booleane danger  ; 
  integer m  ; 
  halfword p  ; 
  halfword a  ; 
  halfword b  ; 
  scaled w  ; 
  scaled z  ; 
  scaled e  ; 
  scaled q  ; 
  scaled d  ; 
  scaled s  ; 
  smallnumber g1, g2  ; 
  halfword r  ; 
  halfword t  ; 
  danger = false ; 
  if ( ( fontparams [ eqtb [ (hash_size + 1837) ] .hh .v.RH ] < 22 ) ||
	   ( fontparams [ eqtb [ (hash_size + 1853) ] .hh .v.RH ] < 22 ) ||
	   ( fontparams [ eqtb [ (hash_size + 1869) ] .hh .v.RH ] < 22 ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 1152 ) ;		/* Math formula deleted: Insufficient symbol fonts */
    } 
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 1153 ;	/* Sorry, but I can't typeset math unless \textfont Sorry, but I can't typeset math unless \textfont 2 */ 
      helpline [ 1 ] = 1154 ;	/* and \scriptfont 2 and \scriptscriptfont 2 have and \scriptfont 2 and \scriptscriptfont 2 have all */
      helpline [ 0 ] = 1155 ;	/* the \fontdimen values needed in math symbol the \fontdimen values needed in math symbol fonts.. */
    } 
    error () ; 
	ABORTCHECK;
    flushmath () ; 
    danger = true ; 
  } 
  else if ( ( fontparams [ eqtb [ (hash_size + 1838) ] .hh .v.RH ] < 13 ) ||
			( fontparams [ eqtb [ (hash_size + 1854) ] .hh .v.RH ] < 13 ) ||
			( fontparams [ eqtb [ (hash_size + 1870) ] .hh .v.RH ] < 13 ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 1156 ) ;		/* Math formula deleted: Insufficient extension fonts */
    } 
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 1157 ;	/* Sorry, but I can't typeset math unless \textfont 3 */
      helpline [ 1 ] = 1158 ;	/* and \scriptfont 3 and \scriptscriptfont 3 have all */
      helpline [ 0 ] = 1159 ;	/* the \fontdimen values needed in math extension fonts. */
    } 
    error () ; 
	ABORTCHECK;
    flushmath () ; 
    danger = true ; 
  } 
  m = curlist .modefield ; 
  l = false ; 
  p = finmlist ( 0 ) ; 
  if ( curlist .modefield == - (integer) m ) 
  {
    {
      getxtoken () ; 
	  ABORTCHECK;
      if ( curcmd != 3 ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 1160 ) ;		/* Display math should end with $$ */
	} 
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1161 ;	/* The `$' that I just saw supposedly matches a previous `$$'. */
	  helpline [ 0 ] = 1162 ;	/* So I shall assume that you typed `$$' both times. */
	} 
	backerror () ; 
	ABORTCHECK;
      } 
    } 
    curmlist = p ; 
    curstyle = 2 ; 
    mlistpenalties = false ; 
    mlisttohlist () ; 
    a = hpack ( mem [ memtop - 3 ] .hh .v.RH , 0 , 1 ) ; 
    unsave () ; 
    decr ( saveptr ) ; 
    if ( savestack [ saveptr + 0 ] .cint == 1 ) 
    l = true ; 
    danger = false ; 
    if ( ( fontparams [ eqtb [ (hash_size + 1837) ] .hh .v.RH ] < 22 ) ||
		 ( fontparams [ eqtb [ (hash_size + 1853) ] .hh .v.RH ] < 22 ) ||
		 ( fontparams [ eqtb [ (hash_size + 1869) ] .hh .v.RH ] < 22 ) ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 1152 ) ;	/* Math formula deleted: Insufficient symbol fonts */
      } 
      {
	helpptr = 3 ; 
	helpline [ 2 ] = 1153 ;		/* Sorry, but I can't typeset math unless \textfont 2 */ 
	helpline [ 1 ] = 1154 ;		/* and \scriptfont 2 and \scriptscriptfont 2 have all */
	helpline [ 0 ] = 1155 ;		/* the \fontdimen values needed in math symbol fonts. */
      } 
      error () ; 
	  ABORTCHECK;
      flushmath () ; 
      danger = true ; 
    } 
    else if ( ( fontparams [ eqtb [ (hash_size + 1838) ] .hh .v.RH ] < 13 ) ||
			  ( fontparams [ eqtb [ (hash_size + 1854) ] .hh .v.RH ] < 13 ) || 
			  ( fontparams [ eqtb [ (hash_size + 1870) ] .hh .v.RH ] < 13 ) ) 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 1156 ) ;	/* Math formula deleted: Insufficient extension fonts */
      } 
      {
	helpptr = 3 ; 
	helpline [ 2 ] = 1157 ;		/* Sorry, but I can't typeset math unless \textfont 3 */
	helpline [ 1 ] = 1158 ;		/* and \scriptfont 3 and \scriptscriptfont 3 have all */
	helpline [ 0 ] = 1159 ;		/* the \fontdimen values needed in math extension fonts. */
      } 
      error () ; 
	  ABORTCHECK;
      flushmath () ; 
      danger = true ; 
    } 
    m = curlist .modefield ; 
    p = finmlist ( 0 ) ; 
  } 
  else a = 0 ; 
  if ( m < 0 ) 
  {
    {
      mem [ curlist .tailfield ] .hh .v.RH = newmath ( eqtb [ (hash_size + 3731) ] .cint , 
      0 ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    curmlist = p ; 
    curstyle = 2 ; 
    mlistpenalties = ( curlist .modefield > 0 ) ; 
    mlisttohlist () ; 
    mem [ curlist .tailfield ] .hh .v.RH = mem [ memtop - 3 ] .hh .v.RH ; 
    while ( mem [ curlist .tailfield ] .hh .v.RH != 0 ) curlist .tailfield = 
    mem [ curlist .tailfield ] .hh .v.RH ; 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newmath ( eqtb [ (hash_size + 3731) ] .cint , 
      1 ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    curlist .auxfield .hh .v.LH = 1000 ; 
    unsave () ; 
  } 
  else {
      
    if ( a == 0 ) 
    {
      getxtoken () ; 
	  ABORTCHECK;
      if ( curcmd != 3 )  {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 1160 ) ;		/* Display math should end with $$ */
	} 
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1161 ;	/* The `$' that I just saw supposedly matches a previous `$$'. */
	  helpline [ 0 ] = 1162 ;	/* So I shall assume that you typed `$$' both times. */
	} 
	backerror () ; 
	ABORTCHECK;
      } 
    } 
    curmlist = p ; 
    curstyle = 0 ; 
    mlistpenalties = false ; 
    mlisttohlist () ; 
    p = mem [ memtop - 3 ] .hh .v.RH ; 
    adjusttail = memtop - 5 ; 
    b = hpack ( p , 0 , 1 ) ; 
    p = mem [ b + 5 ] .hh .v.RH ; 
    t = adjusttail ; 
    adjusttail = 0 ; 
    w = mem [ b + 1 ] .cint ; 
    z = eqtb [ (hash_size + 3744) ] .cint ; 
    s = eqtb [ (hash_size + 3745) ] .cint ; 
    if ( ( a == 0 ) || danger ) 
    {
      e = 0 ; 
      q = 0 ; 
    } 
    else {
	
      e = mem [ a + 1 ] .cint ; 
      q = e + fontinfo [ 6 + parambase [ eqtb [ (hash_size + 1837) ] .hh .v.RH ] ] .cint ; 
    } 
    if ( w + q > z ) 
    {
      if ( ( e != 0 ) && ( ( w - totalshrink [ 0 ] + q <= z ) || ( totalshrink 
      [ 1 ] != 0 ) || ( totalshrink [ 2 ] != 0 ) || ( totalshrink [ 3 ] != 0 ) 
      ) ) 
      {
	freenode ( b , 7 ) ; 
	b = hpack ( p , z - q , 0 ) ; 
      } 
      else {
	  
	e = 0 ; 
	if ( w > z ) 
	{
	  freenode ( b , 7 ) ; 
	  b = hpack ( p , z , 0 ) ; 
	} 
      } 
      w = mem [ b + 1 ] .cint ; 
    } 
    d = half ( z - w ) ; 
    if ( ( e > 0 ) && ( d < 2 * e ) ) 
    {
      d = half ( z - w - e ) ; 
      if ( p != 0 ) 
      if ( ! ( p >= himemmin ) ) 
      if ( mem [ p ] .hh.b0 == 10 ) 
      d = 0 ; 
    } 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( eqtb [ (hash_size + 3174) ] .cint 
      ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    if ( ( d + s <= eqtb [ (hash_size + 3743) ] .cint ) || l ) 
    {
      g1 = 3 ; 
      g2 = 4 ; 
    } 
    else {
	
      g1 = 5 ; 
      g2 = 6 ; 
    } 
    if ( l && ( e == 0 ) ) 
    {
      mem [ a + 4 ] .cint = s ; 
      appendtovlist ( a ) ; 
      {
	mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( 10000 ) ; 
	curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
    } 
    else {
	
      mem [ curlist .tailfield ] .hh .v.RH = newparamglue ( g1 ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    if ( e != 0 ) 
    {
      r = newkern ( z - w - e - d ) ; 
      if ( l ) 
      {
	mem [ a ] .hh .v.RH = r ; 
	mem [ r ] .hh .v.RH = b ; 
	b = a ; 
	d = 0 ; 
      } 
      else {
	  
	mem [ b ] .hh .v.RH = r ; 
	mem [ r ] .hh .v.RH = a ; 
      } 
      b = hpack ( b , 0 , 1 ) ; 
    } 
    mem [ b + 4 ] .cint = s + d ; 
    appendtovlist ( b ) ; 
    if ( ( a != 0 ) && ( e == 0 ) && ! l ) 
    {
      {
	mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( 10000 ) ; 
	curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
      mem [ a + 4 ] .cint = s + z - mem [ a + 1 ] .cint ; 
      appendtovlist ( a ) ; 
      g2 = 0 ; 
    } 
    if ( t != memtop - 5 ) 
    {
      mem [ curlist .tailfield ] .hh .v.RH = mem [ memtop - 5 ] .hh .v.RH ; 
      curlist .tailfield = t ; 
    } 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( eqtb [ (hash_size + 3175) ] .cint 
      ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    if ( g2 > 0 ) 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newparamglue ( g2 ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    resumeafterdisplay () ; 
  } 
} 

void resumeafterdisplay ( ) 
{resumeafterdisplay_regmem 

   if ( curgroup != 15 ) {
		confusion ( 1163 ) ;		/* display */
		return;				// abortflag set
	}
  unsave () ; 
  curlist .pgfield = curlist .pgfield + 3 ; 
  pushnest () ; 
  curlist .modefield = 102 ; 
  curlist .auxfield .hh .v.LH = 1000 ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* was   curlist .auxfield .hh .v.RH = 0 ; etc in 3.141 new stuff follows */
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
  {
    getxtoken () ; 
	ABORTCHECK;
    if ( curcmd != 10 )  backinput () ; 
  } 
  if ( nestptr == 1 ) {
	  buildpage () ;
  }
} 

void getrtoken ( ) 
{/* 20 */ getrtoken_regmem 
lab20:
	do {
		gettoken () ; 
	} while ( ! ( curtok != 2592 ) ) ; 
/*  if ( ( curcs == 0 ) || ( curcs > (hash_size + 514) ) ) */	/* 95/Jan/10 */
  if ( ( curcs == 0 ) || ( curcs > (hash_size + hash_extra + 514) ) ) {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 1178 ) ;		/* Missing control sequence inserted */
    } 
    {
      helpptr = 5 ; 
      helpline [ 4 ] = 1179 ;	/* Please don't say `\def cs{...}', say `\def\cs{...}'. */
      helpline [ 3 ] = 1180 ;	/* I've inserted an inaccessible control sequence so that your */
      helpline [ 2 ] = 1181 ;	/* definition will be completed without mixing me up too badly. */
      helpline [ 1 ] = 1182 ;	/* You can recover graciously from this error, if you're */
      helpline [ 0 ] = 1183 ;	/* careful; see exercise 27.2 in The TeXbook. */
    } 
    if ( curcs == 0 ) 
    backinput () ; 
/*    curtok = (hash_size + 4609) ;  */
/*    curtok = (hash_size + 4095 + 514) ;  */
    curtok = (hash_size + hash_extra + 4095 + 514) ; /* 96/Jan/10 */
    inserror () ; 
	ABORTCHECK;
    goto lab20 ; 
  } 
} 

void trapzeroglue ( ) 
{trapzeroglue_regmem 
  if ( ( mem [ curval + 1 ] .cint == 0 ) && ( mem [ curval + 2 ] .cint == 0 ) 
  && ( mem [ curval + 3 ] .cint == 0 ) ) 
  {
    incr ( mem [ 0 ] .hh .v.RH ) ;	/* mem [ membot ] ? */ /* mem [ null ] ? */
    deleteglueref ( curval ) ; 
    curval = 0 ; 
  } 
} 

void zdoregistercommand ( a ) 
smallnumber a ; 
{/* 40 10 */ doregistercommand_regmem 
  halfword l, q, r, s  ; 
  char p  ; 
  q = curcmd ; 
  {
    if ( q != 89 ) 
    {
      getxtoken () ; 
	  ABORTCHECK;
      if ( ( curcmd >= 73 ) && ( curcmd <= 76 ) ) 
      {
	l = curchr ; 
	p = curcmd - 73 ; 
	goto lab40 ; 
      } 
      if ( curcmd != 89 ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 682 ) ;		/* You can't use ` */
	} 
	printcmdchr ( curcmd , curchr ) ; 
	print ( 683 ) ;			/* ' after  */
	printcmdchr ( q , 0 ) ; 
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 1204 ;	/* I'm forgetting what you said and not changing anything. */
	} 
	error () ; 
	ABORTCHECK;
	return ; 
      } 
    } 
    p = curchr ; 
    scaneightbitint () ; 
	ABORTCHECK;
    switch ( p ) 
    {case 0 : 
      l = curval + (hash_size + 3218) ; 
      break ; 
    case 1 : 
      l = curval + (hash_size + 3751) ; 
      break ; 
    case 2 : 
      l = curval + (hash_size + 800) ; 
      break ; 
    case 3 : 
      l = curval + (hash_size + 1056) ; 
      break ; 
    } 
  } 
  lab40: ; 
  if ( q == 89 ) 
  scanoptionalequals () ; 
  else if ( scankeyword ( 1200 ) )		/* by */
  ; 
  aritherror = false ; 
  if ( q < 91 ) 
  if ( p < 2 ) 
  {
    if ( p == 0 ) {
		scanint () ;
		ABORTCHECK;
	}
    else {
		scandimen ( false , false , false ) ;
		ABORTCHECK;
	}
    if ( q == 90 ) 
    curval = curval + eqtb [ l ] .cint ; 
  } 
  else {
      
    scanglue ( p ) ; 
	ABORTCHECK;
    if ( q == 90 ) {
      q = newspec ( curval ) ; 
      r = eqtb [ l ] .hh .v.RH ; 
      deleteglueref ( curval ) ; 
      mem [ q + 1 ] .cint = mem [ q + 1 ] .cint + mem [ r + 1 ] .cint ; 
      if ( mem [ q + 2 ] .cint == 0 ) 
      mem [ q ] .hh.b0 = 0 ; 
      if ( mem [ q ] .hh.b0 == mem [ r ] .hh.b0 ) 
      mem [ q + 2 ] .cint = mem [ q + 2 ] .cint + mem [ r + 2 ] .cint ; 
      else if ( ( mem [ q ] .hh.b0 < mem [ r ] .hh.b0 ) && ( mem [ r + 2 ] 
      .cint != 0 ) ) 
      {
	mem [ q + 2 ] .cint = mem [ r + 2 ] .cint ; 
	mem [ q ] .hh.b0 = mem [ r ] .hh.b0 ; 
      } 
      if ( mem [ q + 3 ] .cint == 0 ) 
      mem [ q ] .hh.b1 = 0 ; 
      if ( mem [ q ] .hh.b1 == mem [ r ] .hh.b1 ) 
      mem [ q + 3 ] .cint = mem [ q + 3 ] .cint + mem [ r + 3 ] .cint ; 
      else if ( ( mem [ q ] .hh.b1 < mem [ r ] .hh.b1 ) && ( mem [ r + 3 ] 
      .cint != 0 ) ) 
      {
	mem [ q + 3 ] .cint = mem [ r + 3 ] .cint ; 
	mem [ q ] .hh.b1 = mem [ r ] .hh.b1 ; 
      } 
      curval = q ; 
    } 
  } 
  else {
    scanint () ; 
	ABORTCHECK;
    if ( p < 2 ) 
    if ( q == 91 ) 
    if ( p == 0 ) 
    curval = multandadd ( eqtb [ l ] .cint , curval , 0 , 2147483647L ) ; 
/*	2^31 - 1 */
    else curval = multandadd ( eqtb [ l ] .cint , curval , 0 , 1073741823L ) ; 
/*	2^30 - 1 */
    else curval = xovern ( eqtb [ l ] .cint , curval ) ; 
    else {
      s = eqtb [ l ] .hh .v.RH ;	/* l may be used ... */
      r = newspec ( s ) ; 
      if ( q == 91 ) 
      {
	mem [ r + 1 ] .cint = multandadd ( mem [ s + 1 ] .cint , curval , 0 , 
	1073741823L ) ;  /* 2^30 - 1 */
	mem [ r + 2 ] .cint = multandadd ( mem [ s + 2 ] .cint , curval , 0 , 
	1073741823L ) ;  /* 2^30 - 1 */
	mem [ r + 3 ] .cint = multandadd ( mem [ s + 3 ] .cint , curval , 0 , 
	1073741823L ) ;  /* 2^30 - 1 */
      } 
      else {
	mem [ r + 1 ] .cint = xovern ( mem [ s + 1 ] .cint , curval ) ; 
	mem [ r + 2 ] .cint = xovern ( mem [ s + 2 ] .cint , curval ) ; 
	mem [ r + 3 ] .cint = xovern ( mem [ s + 3 ] .cint , curval ) ; 
      } 
      curval = r ; 
    } 
  } 
  if ( aritherror ) {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 1201 ) ;		/* Arithmetic overflow */
    } 
    {
      helpptr = 2 ; 
      helpline [ 1 ] = 1202 ;	/* I can't carry out that multiplication or division, */
      helpline [ 0 ] = 1203 ;	/* since the result is out of range. */
    } 
    error () ; 
	ABORTCHECK;
    return ; 
  } 
  if ( p < 2 ) 
  if ( ( a >= 4 ) ) 
  geqworddefine ( l , curval ) ; 
  else eqworddefine ( l , curval ) ; 
  else {
      
    trapzeroglue () ; 
    if ( ( a >= 4 ) ) 
    geqdefine ( l , 117 , curval ) ; 
    else eqdefine ( l , 117 , curval ) ; 
  } 
} 

/* called only from itex.c */

void alteraux ( ) 
{alteraux_regmem 
  halfword c  ; 
  if ( curchr != abs ( curlist .modefield ) ) {
	  reportillegalcase () ;
	  ABORTCHECK;
  }
  else {
    c = curchr ; 
    scanoptionalequals () ; 
    if ( c == 1 ) 
    {
      scandimen ( false , false , false ) ; 
	  ABORTCHECK;
      curlist .auxfield .cint = curval ; 
    } 
    else {
      scanint () ; 
	  ABORTCHECK;
      if ( ( curval <= 0 ) || ( curval > 32767 ) ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 1207 ) ;		/* Bad space factor */
	} 
	{
	  helpptr = 1 ; 
	  helpline [ 0 ] = 1208 ;	/* I allow only values in the range 1..32767 here. */
	} 
	interror ( curval ) ; 
	ABORTCHECK;
      } 
      else curlist .auxfield .hh .v.LH = curval ; 
    } 
  } 
} 

void alterprevgraf ( ) 
{alterprevgraf_regmem 
  integer p  ; 
  nest [ nestptr ] = curlist ; 
  p = nestptr ; 
  while ( abs ( nest [ p ] .modefield ) != 1 ) decr ( p ) ; 
  scanoptionalequals () ; 
  scanint () ; 
  ABORTCHECK;
  if ( curval < 0 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ; /* ! */
      print ( 949 ) ;	/* Bad */
    } 
    printesc ( 529 ) ;	/* prevgraf */
    {
      helpptr = 1 ; 
      helpline [ 0 ] = 1209 ;	/* I allow only nonnegative values here. */
    } 
    interror ( curval ) ; 
	ABORTCHECK;
  } 
  else {
      
    nest [ p ] .pgfield = curval ; 
    curlist = nest [ nestptr ] ; 
  } 
} 

void alterpagesofar ( ) 
{alterpagesofar_regmem 
  char c  ; 
  c = curchr ; 
  scanoptionalequals () ; 
  scandimen ( false , false , false ) ; 
  ABORTCHECK;
  pagesofar [ c ] = curval ; 
} 

void alterinteger ( ) 
{alterinteger_regmem 
  char c  ; 
  c = curchr ; 
  scanoptionalequals () ; 
  scanint () ; 
  ABORTCHECK;
  if ( c == 0 ) deadcycles = curval ; 
  else insertpenalties = curval ; 
} 

void alterboxdimen ( ) 
{alterboxdimen_regmem 
  smallnumber c  ; 
  eightbits b  ; 
  c = curchr ; 
  scaneightbitint () ; 
  ABORTCHECK;
  b = curval ; 
  scanoptionalequals () ; 
  scandimen ( false , false , false ) ; 
  ABORTCHECK;
  if ( eqtb [ (hash_size + 1578) + b ] .hh .v.RH != 0 ) 
  mem [ eqtb [ (hash_size + 1578) + b ] .hh .v.RH + c ] .cint = curval ; 
} 

void znewfont ( a ) 
smallnumber a ; 
{/* 50 */ newfont_regmem 
  halfword u  ; 
  scaled s  ; 
  internalfontnumber f  ; 
  strnumber t  ; 
  char oldsetting  ; 
  strnumber flushablestring  ; 
  if ( jobname == 0 ) openlogfile () ; 
  getrtoken () ; 
  u = curcs ; 
  if ( u >= 514 )			/* if u >= hash_base then t <- text(u); p.1257 */
	  t = hash [ u ] .v.RH ; 
  else if ( u >= 257 )		/* if u >= single_base then ... */
/*    if u=null_cs then t:="FONT"@+else t:=u-single_base */
  if ( u == 513 ) 
	  t = 1213 ;			/* FONT */
  else t = u - 257 ;		/* else t <- u - single_base */
  else {
    oldsetting = selector ; 
    selector = 21 ; 
    print ( 1213 ) ;		/* FONT */
    print ( u - 1 ) ; 
    selector = oldsetting ; 
    {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
	  if ( poolptr + 1 > currentpoolsize ) 
		  strpool = reallocstrpool (incrementpoolsize);
	  if ( poolptr + 1 > currentpoolsize ) {			/* 94/Jan/24 */
		  overflow ( 257 , currentpoolsize - initpoolptr ) ; /* 97/Mar/9 */
		  return;			// abortflag set
	  }
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
      if ( poolptr + 1 > poolsize ) {
		  overflow ( 257 , poolsize - initpoolptr ) ; /* pool size */
		  return;			// abortflag set
	  }
#endif
    } 
    t = makestring () ; 
  } 
  if ( ( a >= 4 ) ) geqdefine ( u , 87 , 0 ) ; 
  else eqdefine ( u , 87 , 0 ) ; 
  scanoptionalequals () ; 
  scanfilename () ; 

/* paragraph 1258 */
  nameinprogress = true ; 
  if ( scankeyword ( 1214 ) )	/* at */
  {
    scandimen ( false , false , false ) ; 
	ABORTCHECK;
    s = curval ; 
    if ( ( s <= 0 ) || ( s >= 134217728L ) ) /* 2^27 */
    {
      {
	if ( interaction == 3 ) 	; 
	printnl ( 262 ) ;	/* ! */
	print ( 1216 ) ;	/* Improper `at' size ( */
      } 
      printscaled ( s ) ; 
      print ( 1217 ) ;	/* pt), replaced by 10pt */
      {
	helpptr = 2 ; 
	helpline [ 1 ] = 1218 ;	/* I can only handle fonts at positive sizes that are */
	helpline [ 0 ] = 1219 ;	/* less than 2048pt, so I've changed what you said to 10pt. */
      } 
      error () ; 
	  ABORTCHECK;
      s = 10 * 65536L ;		/* 10pt */
    } 
  } 
  else if ( scankeyword ( 1215 ) )	/* scaled */
  {
    scanint () ; 
	ABORTCHECK;
    s = - (integer) curval ; 
    if ( ( curval <= 0 ) || ( curval > 32768L ) )  {
      {
	if ( interaction == 3 ) 	; 
	printnl ( 262 ) ;	/* ! */
	print ( 549 ) ;		/* Illegal magnification has been changed to 1000 */
      } 
      {
	helpptr = 1 ; 
	helpline [ 0 ] = 550 ;	/* The magnification ratio must be between 1 and 32768. */
      } 
      interror ( curval ) ; 
	  ABORTCHECK;
      s = -1000 ; 
    } 
  } 
  else s = -1000 ; 
  nameinprogress = false ; 

  flushablestring = strptr - 1 ; 
  if (traceflag)   {					/* debugging stuff only 98/Oct/5 */
	  int i, k1, k2, l1, l2;
	  char *sch=logline;
	  k1 = strstart [ curarea ];
	  k2 = strstart [ curname ];
	  l1 = strstart [ curarea + 1] - strstart [ curarea ];
	  l2 = strstart [ curname + 1] - strstart [ curname ];
	  showchar('\n');
	  showline("FONT ", 0);
	  for (i = 0; i < l1; i++) {
		  *sch++ = strpool[i+k1];
	  }
	  for (i = 0; i < l2; i++) {
		  *sch++ = strpool[i+k2];
	  }
	  *sch++ = ' ';
	  *sch++ = '\0';
	  showline(logline, 0);
  }
/*  if (ignorefrozen) goto lab69; */			/* 98/Oct/5 */

/* paragraph 1260 for f <- fontbase+1 to font_ptr do */
  {
	  register integer for_end; 
	  f = 1 ; 
	  for_end = fontptr ; 
	  if ( f <= for_end) 
		  do 
/* if str_eq_str(font_name[f],cur_name) ^ str_eq_str(font_area[f],cur_area) */
			  if ( streqstr ( fontname [ f ] , curname ) &&
				   streqstr ( fontarea [ f ] , curarea ) )  {
				  if ( curname == flushablestring ) {
					  {
						  decr ( strptr ) ; 
						  poolptr = strstart [ strptr ] ; 
					  } 
					  curname = fontname [ f ] ; 
				  } 
/*	  if (ignorefrozen) continue; */ 			/* 98/Oct/5 */
				  if ( s > 0 )  {		/* if pt size was specified */
					  if ( s == fontsize [ f ] ) {
/*						  if (ignorefrozen == 0)  */
						  if (ignorefrozen == 0 || f > frozenfontptr) { /* 99/Mar/26 */
							  if (traceflag) {
								  sprintf(logline, "SKIPPING %ld ", s);
								  showline(logline, 0);
							  }
							  goto lab50 ;
						  }
					  }
				  } 
/* else if font_size[f] = xn_over_d(font_dsize[f],-s,1000) then goto common_ending */
				  else if ( fontsize [ f ] == xnoverd ( fontdsize [ f ] ,
					  - (integer) s , 1000 ) ) {	/* if using design size */
/*					  if (ignorefrozen == 0) */
					  if (ignorefrozen == 0 || f > frozenfontptr) { /* 99/Mar/26 */
						  if (traceflag) {
							  sprintf(logline, "SKIPPING %ld ", s);
							  showline(logline, 0);
						  }
						  goto lab50 ;
					  }
				  }
			  } 
	  while ( f++ < for_end ) ;
  } 

/* lab69: */	/* 98/Oct/5 */
/* end of paragraph 1257 */
  if (traceflag) showline("READING ", 0);	/* debugging only */
  f = readfontinfo ( u , curname , curarea , s ) ; 
  ABORTCHECK;

/* common_ending: equiv[u] <- f;  */	/* use existing font info */
  lab50:
  if (traceflag) {
	  sprintf(logline, "NEW FONT %d ", f); /* debugging only */
	  showline(logline, 0);
  }
  eqtb [ u ] .hh .v.RH = f ; 
/*  eqtb [ (hash_size + 524) + f ] = eqtb [ u ] ;*/ /* eqtb[frozen_null+font+f] */
  eqtb [ (hash_size + hash_extra + 524) + f ] = eqtb [ u ] ; /* 96/Jan/10 */
#ifdef SHORTHASH						/*	debugging only  1996/Jan/20 */
  if (t > 65535L) {
	  sprintf(logline, "ERROR: %s too large %d\n",  "hashused", t);
	  showline(logline, 1);
  }
#endif
/*  hash [ (hash_size + 524) + f ] .v.RH = t ;  */
  hash [ (hash_size + hash_extra + 524) + f ] .v.RH = t ; /* 96/Jan/10 */
} 

void newinteraction ( ) 
{newinteraction_regmem 
  println () ; 
  interaction = curchr ; 
  if ( interaction == 0 )  selector = 16 ; 
  else selector = 17 ; 
  if ( logopened )  selector = selector + 2 ; 
} 

void doassignments ( ) 
{/* 10 */ doassignments_regmem 
  while ( true ) {
    do {
		getxtoken () ; 
		ABORTCHECK;
    } while ( ! ( ( curcmd != 10 ) && ( curcmd != 0 ) ) ) ; 
    if ( curcmd <= 70 ) 
		return ; 
    setboxallowed = false ; 
    prefixedcommand () ; 
	ABORTCHECK;
    setboxallowed = true ; 
  } 
} 

void openorclosein ( ) 
{openorclosein_regmem 
  char c  ; 
  char n  ; 
  c = curchr ; 
  scanfourbitint () ; 
  ABORTCHECK;
  n = curval ; 
  if ( readopen [ n ] != 2 ) 
  {
    (void) aclose ( readfile [ n ] ) ; 
    readopen [ n ] = 2 ; 
  } 
  if ( c != 0 ) 
  {
    scanoptionalequals () ; 
    scanfilename () ; 
    packfilename ( curname , curarea , curext ) ; 
/* *** some changes in following in 3.14159 *** */
/*	if current extension is *not* empty, try to open using name as is */
/*	string 335 is "" the empty string */
    if ( ( curext != 335 ) && aopenin ( readfile [ n ] , TEXINPUTPATH ) ) 
		readopen [ n ] = 1 ; 
/*	we get here if extension is "", or file with extension failed to open */
/*	if current extension is not `tex,' and `tex' is not irrelevant, try it */
/*	string 785 is  .tex */
    else if ( ( curext != 785 ) && ( namelength + 5 < PATHMAX ) && 
/* *** some changes in above file name handling *** */
/*			  ( ! extensionirrelevantp ( nameoffile , "tex" ) ) )  */
			  ( ! extensionirrelevantp ( nameoffile , namelength , "tex" ) ) )
	{
      nameoffile [ namelength + 1 ] = 46 ;		/* .tex  */
      nameoffile [ namelength + 2 ] = 116 ; 
      nameoffile [ namelength + 3 ] = 101 ; 
      nameoffile [ namelength + 4 ] = 120 ; 
      nameoffile [ namelength + 5 ] = 32 ; 
      namelength = namelength + 4 ; 
      if ( aopenin ( readfile [ n ] , TEXINPUTPATH ) ) 
		  readopen [ n ] = 1 ; 
      else {
/*    more changes here in 3.14159 *** */
		  namelength = namelength - 4 ;			/* remove ".tex" again */
		  nameoffile [ namelength + 1 ] = 32 ;	/* '  '  */
/*	  string 335 is "" the empty string */
		  if ( ( curext == 335 ) && aopenin ( readfile [ n ] , TEXINPUTPATH ) ) 
			  readopen [ n ] = 1 ; 
		  else if ( maketextex () && aopenin ( readfile [ n ] , TEXINPUTPATH ) ) 
			  readopen [ n ] = 1 ; 
	  } 
    } 
  } 
} 

void issuemessage ( ) 
{issuemessage_regmem 
  char oldsetting  ; 
  char c  ; 
  strnumber s  ; 
  c = curchr ; 
  mem [ memtop - 12 ] .hh .v.RH = scantoks ( false , true ) ; 
  ABORTCHECK;
  oldsetting = selector ; 
  selector = 21 ; 
  tokenshow ( defref ) ; 
  selector = oldsetting ; 
  flushlist ( defref ) ; 
  {
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
	if ( poolptr + 1 > currentpoolsize ) 
		  strpool = reallocstrpool (incrementpoolsize);
	if ( poolptr + 1 > currentpoolsize ) {	/* in case it failed 94/Jan/24 */
		overflow ( 257 , currentpoolsize - initpoolptr ) ; /* 97/Mar/7 */
		return;			// abortflag set
	}
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    if ( poolptr + 1 > poolsize ) {
		overflow ( 257 , poolsize - initpoolptr ) ; /* pool size */
		return;			// abortflag set
	}
#endif
  } 
  s = makestring () ; 
  if ( c == 0 ) 
  {
    if ( termoffset + ( strstart [ s + 1 ] - strstart [ s ] ) > maxprintline - 
    2 ) 
    println () ; 
    else if ( ( termoffset > 0 ) || ( fileoffset > 0 ) ) 
    printchar ( 32 ) ;		/*   */
    slowprint ( s ) ; 
#ifndef f_WINDOWS
    fflush ( stdout ) ; 
#endif
  } 
  else {
      
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 335 ) ;		/* */
    } 
    slowprint ( s ) ; 
    if ( eqtb [ (hash_size + 1321) ] .hh .v.RH != 0 ) 
    useerrhelp = true ; 
    else if ( longhelpseen ) 
    {
      helpptr = 1 ; 
      helpline [ 0 ] = 1226 ;	/* (That was another \errmessage.) */
    } 
    else {
	
      if ( interaction < 3 ) 
      longhelpseen = true ; 
      {
	helpptr = 4 ; 
	helpline [ 3 ] = 1227 ;		/* This error message was generated by an \errmessage */
	helpline [ 2 ] = 1228 ;		/* command, so I can't give any explicit help. */
	helpline [ 1 ] = 1229 ;		/* Pretend that you're Hercule Poirot: Examine all clues, */
	helpline [ 0 ] = 1230 ;		/* and deduce the truth by order and method. */
      } 
    } 
    error () ; 
	ABORTCHECK;
    useerrhelp = false ; 
  } 
  {
    decr ( strptr ) ; 
    poolptr = strstart [ strptr ] ; 
  } 
} 

void shiftcase ( ) 
{shiftcase_regmem 
  halfword b  ; 
  halfword p  ; 
  halfword t  ; 
  eightbits c  ; 
  b = curchr ; 
  p = scantoks ( false , false ) ; 
  ABORTCHECK;
  p = mem [ defref ] .hh .v.RH ; 
  while ( p != 0 ) {			/* while p <> null ... p.1288 */
      
    t = mem [ p ] .hh .v.LH ;	/* t <- info(p) p.1289 */ 
/*    if ( t < 4352 )  */
    if ( t < 4095 + 257) 		/* 4095 + 257 = cs_tokenflag + single_base */
    {
      c = t % 256 ;  
/*      c = t & 255 ;  */	/* last 8 bits */
      if ( eqtb [ b + c ] .hh .v.RH != 0 ) 
      mem [ p ] .hh .v.LH = t - c + eqtb [ b + c ] .hh .v.RH ; 
    } 
    p = mem [ p ] .hh .v.RH ; 
  } 
  begintokenlist ( mem [ defref ] .hh .v.RH , 3 ) ; 
  {
    mem [ defref ] .hh .v.RH = avail ; 
    avail = defref ; 
	;
#ifdef STAT
    decr ( dynused ) ; 
#endif /* STAT */
  } 
} 
void showwhatever ( ) 
{/* 50 */ showwhatever_regmem 
  halfword p  ; 
  switch ( curchr ) 
  {case 3 : 
    {
      begindiagnostic () ; 
      showactivities () ; 
    } 
    break ; 
  case 1 : 
    {
      scaneightbitint () ; 
	  ABORTCHECK;
      begindiagnostic () ; 
      printnl ( 1248 ) ;	/* > \box */
      printint ( curval ) ; 
      printchar ( 61 ) ;	/* = */
      if ( eqtb [ (hash_size + 1578) + curval ] .hh .v.RH == 0 ) 
      print ( 407 ) ;		/* void */
      else showbox ( eqtb [ (hash_size + 1578) + curval ] .hh .v.RH ) ; 
    } 
    break ; 
  case 0 : 
    {
      gettoken () ; 
      if ( interaction == 3 ) 
      ; 
      printnl ( 1242 ) ;	/* >  */
      if ( curcs != 0 ) 
      {
	sprintcs ( curcs ) ; 
	printchar ( 61 ) ;		/* = */
      } 
      printmeaning () ; 
      goto lab50 ; 
    } 
    break ; 
    default: 
    {
      p = thetoks () ; 
      if ( interaction == 3 ) 
      ; 
      printnl ( 1242 ) ;	/* 	>  */
      tokenshow ( memtop - 3 ) ; 
      flushlist ( mem [ memtop - 3 ] .hh .v.RH ) ; 
      goto lab50 ; 
    } 
    break ; 
  } 
  enddiagnostic ( true ) ; 
  {
    if ( interaction == 3 ) 
    ; 
    printnl ( 262 ) ;	/* !  */
    print ( 1249 ) ;	/* OK */
  } 
  if ( selector == 19 ) 
  if ( eqtb [ (hash_size + 3192) ] .cint <= 0 ) 
  {
    selector = 17 ; 
    print ( 1250 ) ;	/*  (see the transcript file) */
    selector = 19 ; 
  } 
  lab50: if ( interaction < 3 ) 
  {
    helpptr = 0 ; 
    decr ( errorcount ) ; 
  } 
  else if ( eqtb [ (hash_size + 3192) ] .cint > 0 ) 
  {
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 1237 ;	/* This isn't an error message; I'm just \showing something. */
      helpline [ 1 ] = 1238 ;	/* Type `I\show...' to show more (e.g., \show\cs, */
      helpline [ 0 ] = 1239 ;	/* \showthe\count10, \showbox255, \showlists). */
    } 
  } 
  else {
      
    {
      helpptr = 5 ; 
      helpline [ 4 ] = 1237 ;	/* This isn't an error message; I'm just \showing something. */
      helpline [ 3 ] = 1238 ;	/* Type `I\show...' to show more (e.g., \show\cs, */
      helpline [ 2 ] = 1239 ;	/* \showthe\count10, \showbox255, \showlists). */
      helpline [ 1 ] = 1240 ;	/* And type `I\tracingonline=1\show...' to show boxes and */
      helpline [ 0 ] = 1241 ;	/* lists on your terminal as well as in the transcript file. */
    } 
  } 
  error () ; 
//	ABORTCHECK;
} 

void znewwhatsit ( s , w ) 
smallnumber s ; 
smallnumber w ; 
{newwhatsit_regmem 
  halfword p  ; 
  p = getnode ( w ) ; 
  mem [ p ] .hh.b0 = 8 ; 
  mem [ p ] .hh.b1 = s ; 
  mem [ curlist .tailfield ] .hh .v.RH = p ; 
  curlist .tailfield = p ; 
} 

void znewwritewhatsit ( w ) 
smallnumber w ; 
{newwritewhatsit_regmem 
  newwhatsit ( curchr , w ) ; 
  if ( w != 2 ) {
	  scanfourbitint () ;
	  ABORTCHECK;
  }
  else {
    scanint () ; 
	ABORTCHECK;
    if ( curval < 0 ) curval = 17 ; 
    else if ( curval > 15 ) curval = 16 ; 
  } 
  mem [ curlist .tailfield + 1 ] .hh .v.LH = curval ; 
} 

void doextension ( ) 
{doextension_regmem 
/*  integer i, j, k  ;  */
  integer k  ; 
/*  halfword p, q, r  ;  */
  halfword p  ; 
  switch ( curchr ) 
  {case 0 : 
    {
      newwritewhatsit ( 3 ) ; 
      scanoptionalequals () ; 
      scanfilename () ; 
      mem [ curlist .tailfield + 1 ] .hh .v.RH = curname ; 
      mem [ curlist .tailfield + 2 ] .hh .v.LH = curarea ; 
      mem [ curlist .tailfield + 2 ] .hh .v.RH = curext ; 
    } 
    break ; 
  case 1 : 
    {
      k = curcs ; 
      newwritewhatsit ( 2 ) ; 
      curcs = k ; 
      p = scantoks ( false , false ) ; 
	  ABORTCHECK;
      mem [ curlist .tailfield + 1 ] .hh .v.RH = defref ; 
    } 
    break ; 
  case 2 : 
    {
      newwritewhatsit ( 2 ) ; 
      mem [ curlist .tailfield + 1 ] .hh .v.RH = 0 ; 
    } 
    break ; 
  case 3 : 
    {
      newwhatsit ( 3 , 2 ) ; 
      mem [ curlist .tailfield + 1 ] .hh .v.LH = 0 ; 
      p = scantoks ( false , true ) ; 
	  ABORTCHECK;
      mem [ curlist .tailfield + 1 ] .hh .v.RH = defref ; 
    } 
    break ; 
  case 4 : 
    {
      getxtoken () ; 
	  ABORTCHECK;
      if ( ( curcmd == 59 ) && ( curchr <= 2 ) ) 
      {
	p = curlist .tailfield ; 
	doextension () ; 
	ABORTCHECK;
	outwhat ( curlist .tailfield ) ;
	ABORTCHECK;
	flushnodelist ( curlist .tailfield ) ; 
	curlist .tailfield = p ; 
	mem [ p ] .hh .v.RH = 0 ; 
      } 
      else backinput () ; 
    } 
    break ; 
  case 5 : 
    if ( abs ( curlist .modefield ) != 102 ) {
		reportillegalcase () ;
		ABORTCHECK;
	}
    else {
	
      newwhatsit ( 4 , 2 ) ; 
      scanint () ; 
	  ABORTCHECK;
      if ( curval <= 0 )  curlist .auxfield .hh .v.RH = 0 ; 
      else if ( curval > 255 ) 
      curlist .auxfield .hh .v.RH = 0 ; 
      else curlist .auxfield .hh .v.RH = curval ; 
      mem [ curlist .tailfield + 1 ] .hh .v.RH = curlist .auxfield .hh .v.RH ; 
      mem [ curlist .tailfield + 1 ] .hh.b0 = normmin ( eqtb [ (hash_size + 3214) ] .cint ) 
      ; 
      mem [ curlist .tailfield + 1 ] .hh.b1 = normmin ( eqtb [ (hash_size + 3215) ] .cint ) 
      ; 
    } 
    break ; 
    default: 
		{
			confusion ( 1285 ) ;	/* display */
			return;				// abortflag set
		}
		break ; 
  } 
} 

void fixlanguage ( ) 
{fixlanguage_regmem 
/*  ASCIIcode l  ;  */
  int l  ;									/* 95/Jan/7 */
  if ( eqtb [ (hash_size + 3213) ] .cint <= 0 ) 
  l = 0 ; 
  else if ( eqtb [ (hash_size + 3213) ] .cint > 255 ) 
  l = 0 ; 
  else l = eqtb [ (hash_size + 3213) ] .cint ; 
  if ( l != curlist .auxfield .hh .v.RH ) 
  {
    newwhatsit ( 4 , 2 ) ; 
    mem [ curlist .tailfield + 1 ] .hh .v.RH = l ; 
    curlist .auxfield .hh .v.RH = l ; 
    mem [ curlist .tailfield + 1 ] .hh.b0 = normmin ( eqtb [ (hash_size + 3214) ] .cint ) ; 
    mem [ curlist .tailfield + 1 ] .hh.b1 = normmin ( eqtb [ (hash_size + 3215) ] .cint ) ; 
  } 
} 

void handlerightbrace ( ) 
{handlerightbrace_regmem 
  halfword p, q  ; 
  scaled d  ; 
  integer f  ; 

  switch ( curgroup ) 
  {case 1 : 
    unsave () ; 
    break ; 
  case 0 : 
    {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 1038 ) ;	/* Too many }'s */
      } 
      {
	helpptr = 2 ; 
	helpline [ 1 ] = 1039 ;		/* You've closed more groups than you opened. */
	helpline [ 0 ] = 1040 ;		/* Such booboos are generally harmless, so keep going. */
      } 
      error () ; 
	  ABORTCHECK;
    } 
    break ; 
  case 14 : 
  case 15 : 
  case 16 : 
    extrarightbrace () ; 
	ABORTCHECK;
    break ; 
  case 2 : 
    package ( 0 ) ; 
    break ; 
  case 3 : 
    {
      adjusttail = memtop - 5 ; 
      package ( 0 ) ; 
    } 
    break ; 
  case 4 : 
    {
      endgraf () ; 
      package ( 0 ) ; 
    } 
    break ; 
  case 5 : 
    {
      endgraf () ; 
      package ( 4 ) ; 
    } 
    break ; 
  case 11 : 
    {
      endgraf () ; 
      q = eqtb [ (hash_size + 792) ] .hh .v.RH ; 
      incr ( mem [ q ] .hh .v.RH ) ; 
      d = eqtb [ (hash_size + 3736) ] .cint ; 
      f = eqtb [ (hash_size + 3205) ] .cint ; 
      unsave () ; 
      decr ( saveptr ) ; 
      p = vpackage ( mem [ curlist .headfield ] .hh .v.RH , 0 , 1 , 
      1073741823L ) ;  /* 2^30 - 1 */
      popnest () ; 
      if ( savestack [ saveptr + 0 ] .cint < 255 ) 
      {
	{
	  mem [ curlist .tailfield ] .hh .v.RH = getnode ( 5 ) ; 
	  curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
	} 
	mem [ curlist .tailfield ] .hh.b0 = 3 ; 
	mem [ curlist .tailfield ] .hh.b1 = savestack [ saveptr + 0 ] .cint ; 
	mem [ curlist .tailfield + 3 ] .cint = mem [ p + 3 ] .cint + mem [ p + 
	2 ] .cint ; 
	mem [ curlist .tailfield + 4 ] .hh .v.LH = mem [ p + 5 ] .hh .v.RH ; 
	mem [ curlist .tailfield + 4 ] .hh .v.RH = q ; 
	mem [ curlist .tailfield + 2 ] .cint = d ; 
	mem [ curlist .tailfield + 1 ] .cint = f ; 
      } 
      else {
	  
	{
	  mem [ curlist .tailfield ] .hh .v.RH = getnode ( 2 ) ; 
	  curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
	} 
	mem [ curlist .tailfield ] .hh.b0 = 5 ; 
	mem [ curlist .tailfield ] .hh.b1 = 0 ; 
	mem [ curlist .tailfield + 1 ] .cint = mem [ p + 5 ] .hh .v.RH ; 
	deleteglueref ( q ) ; 
      } 
      freenode ( p , 7 ) ; 
      if ( nestptr == 0 ) {
		  buildpage () ;
		  ABORTCHECK;
	  }
    } 
    break ; 
  case 8 : 
    {
      if ( ( curinput .locfield != 0 ) || ( ( curinput .indexfield != 6 ) && ( 
      curinput .indexfield != 3 ) ) ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 1004 ) ;		/* Unbalanced output routine */
	} 
	{
	  helpptr = 2 ; 
	  helpline [ 1 ] = 1005 ;	/* Your sneaky output routine has problematic {'s and/or }'s. */
	  helpline [ 0 ] = 1006 ;	/* I can't handle that very well; good luck. */
	} 
	error () ; 
	ABORTCHECK;
	do {
	    gettoken () ; 
	} while ( ! ( curinput .locfield == 0 ) ) ; 
      } 
      endtokenlist () ; 
	  ABORTCHECK;
      endgraf () ; 
      unsave () ; 
      outputactive = false ; 
      insertpenalties = 0 ; 
      if ( eqtb [ (hash_size + 1833) ] .hh .v.RH != 0 ) 
      {
		  {
			  if ( interaction == 3 ) 
				  ; 
			  printnl ( 262 ) ;		/* !  */
			  print ( 1007 ) ;		/* Output routine didn't use all of  */
		  } 
		  printesc ( 406 ) ;		/* box */
		  printint ( 255 ) ; 
		  {
			  helpptr = 3 ; 
			  helpline [ 2 ] = 1008 ;	/* Your \output commands should empty \box255, */
			  helpline [ 1 ] = 1009 ;	/* e.g., by saying `\shipout\box255'. */
			  helpline [ 0 ] = 1010 ;	/* Proceed; I'll discard its present contents. */
		  } 
		  boxerror ( 255 ) ; 
		  ABORTCHECK;
	  }
      if ( curlist .tailfield != curlist .headfield ) 
      {
	mem [ pagetail ] .hh .v.RH = mem [ curlist .headfield ] .hh .v.RH ; 
	pagetail = curlist .tailfield ; 
      } 
      if ( mem [ memtop - 2 ] .hh .v.RH != 0 ) 
      {
	if ( mem [ memtop - 1 ] .hh .v.RH == 0 ) 
	nest [ 0 ] .tailfield = pagetail ; 
	mem [ pagetail ] .hh .v.RH = mem [ memtop - 1 ] .hh .v.RH ; 
	mem [ memtop - 1 ] .hh .v.RH = mem [ memtop - 2 ] .hh .v.RH ; 
	mem [ memtop - 2 ] .hh .v.RH = 0 ; 
	pagetail = memtop - 2 ; 
      } 
      popnest () ; 
      buildpage () ; 
	  ABORTCHECK;
    } 
    break ; 
  case 10 : 
    builddiscretionary () ; 
	ABORTCHECK;
    break ; 
  case 6 : 
/* align_group: begin back_input; cur_tok:=cs_token_flag+frozen_cr; */
    {
      backinput () ; 
/*      curtok = (hash_size + 4610) ;  */
/*      curtok = (hash_size + 4095 + 515) ;  */
      curtok = (hash_size + hash_extra + 4095 + 515) ; 
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 622 ) ;		/* Missing  */
      } 
      printesc ( 893 ) ;	/* cr */
      print ( 623 ) ;		/* inserted */
      {
	helpptr = 1 ; 
	helpline [ 0 ] = 1119 ;		/* I'm guessing that you meant to end an alignment here. */
      } 
      inserror () ; 
	  ABORTCHECK;
    } 
    break ; 
  case 7 : 
    {
      endgraf () ; 
      unsave () ; 
      alignpeek () ; 
	  ABORTCHECK;
    } 
    break ; 
  case 12 : 
    {
      endgraf () ; 
      unsave () ; 
      saveptr = saveptr - 2 ; 
      p = vpackage ( mem [ curlist .headfield ] .hh .v.RH , savestack [ 
      saveptr + 1 ] .cint , savestack [ saveptr + 0 ] .cint ,
			1073741823L ) ;   /* 2^30 - 1 */
      popnest () ; 
      {
	mem [ curlist .tailfield ] .hh .v.RH = newnoad () ; 
	curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
      mem [ curlist .tailfield ] .hh.b0 = 29 ; 
      mem [ curlist .tailfield + 1 ] .hh .v.RH = 2 ; 
      mem [ curlist .tailfield + 1 ] .hh .v.LH = p ; 
    } 
    break ; 
  case 13 : 
    buildchoices () ; 
    break ; 
  case 9 : 
    {
      unsave () ; 
      decr ( saveptr ) ; 
      mem [ savestack [ saveptr + 0 ] .cint ] .hh .v.RH = 3 ; 
      p = finmlist ( 0 ) ; 
      mem [ savestack [ saveptr + 0 ] .cint ] .hh .v.LH = p ; 
      if ( p != 0 ) 
      if ( mem [ p ] .hh .v.RH == 0 ) 
      if ( mem [ p ] .hh.b0 == 16 ) 
      {
	if ( mem [ p + 3 ] .hh .v.RH == 0 ) 
	if ( mem [ p + 2 ] .hh .v.RH == 0 ) 
	{
	  mem [ savestack [ saveptr + 0 ] .cint ] .hh = mem [ p + 1 ] .hh ; 
	  freenode ( p , 4 ) ; 
	} 
      } 
      else if ( mem [ p ] .hh.b0 == 28 ) 
      if ( savestack [ saveptr + 0 ] .cint == curlist .tailfield + 1 ) 
      if ( mem [ curlist .tailfield ] .hh.b0 == 16 ) 
      {
	q = curlist .headfield ; 
	while ( mem [ q ] .hh .v.RH != curlist .tailfield ) q = mem [ q ] .hh 
	.v.RH ; 
	mem [ q ] .hh .v.RH = p ; 
	freenode ( curlist .tailfield , 4 ) ; 
	curlist .tailfield = p ; 
      } 
    } 
    break ; 
    default: 
		{
			confusion ( 1041 ) ;	/* rightbrace */
			return;				// abortflag set
		}
		break ; 
  } 
} 

/* main control loop */

void maincontrol ( ) 
{/* 60 21 70 80 90 91 92 95 100 101 110 111 112 120 10 */ maincontrol_regmem 
    integer t  ; 
	integer bSuppress ;		/* 199/Jan/5 */

	if ( eqtb [ (hash_size + 1319) ] .hh .v.RH != 0 ) /* everyjob */
		begintokenlist ( eqtb [ (hash_size + 1319) ] .hh .v.RH , 12 ) ; 

lab60:
	ABORTCHECK;
	getxtoken () ;				/* big_switch */
lab21:
	ABORTCHECK;
	if ( interrupt != 0 ) 
		if ( OKtointerrupt ) {
			backinput () ; 
			{
				if ( interrupt != 0 ) {
					pauseforinstructions () ;
					ABORTCHECK;
				}
			}	 
			goto lab60 ; 
		} 
	;
#ifdef DEBUG
	if ( panicking ) checkmem ( false ) ; 
#endif /* DEBUG */
	if ( eqtb [ (hash_size + 3199) ] .cint > 0 ) 
		showcurcmdchr () ; 

/*	the big switch --- don't bother to test abortflag ??? */
	switch ( abs ( curlist .modefield ) + curcmd ) {
		   case 113 : 
		   case 114 : 
		   case 170 : 
			   goto lab70 ; 
			   break ; 
		   case 118 : 
		   {
			   scancharnum () ; 
			   ABORTCHECK;
			   curchr = curval ; 
			   goto lab70 ; 
		   } 
		   break ; 
		   case 167 : 
		   {
			   getxtoken () ; 
			   ABORTCHECK;
			   if ( ( curcmd == 11 ) || ( curcmd == 12 ) || ( curcmd == 68 ) || ( 
				   curcmd == 16 ) ) 
				   cancelboundary = true ; 
			   goto lab21 ; 
		   } 
		   break ; 
  case 112 : 
    if ( curlist .auxfield .hh .v.LH == 1000 ) goto lab120 ; 
    else {
		appspace () ;
		ABORTCHECK;
	}
    break ; 
  case 166 : 
  case 267 : 
    goto lab120 ; 
    break ; 
  case 1 : 
  case 102 : 
  case 203 : 
  case 11 : 
  case 213 : 
  case 268 : 
    ; 
    break ; 
  case 40 : 
  case 141 : 
  case 242 : 
    {
      do {
		  getxtoken () ; 
		  ABORTCHECK;
      } while ( ! ( curcmd != 10 ) ) ; 
      goto lab21 ; 
    } 
    break ; 
  case 15 : 
    if ( itsallover () ) return ; 
    break ; 
  case 23 : 
  case 123 : 
  case 224 : 
  case 71 : 
  case 172 : 
  case 273 : 
  case 39 : 
  case 45 : 
  case 49 : 
  case 150 : 
  case 7 : 
  case 108 : 
  case 209 : 
    reportillegalcase () ; 
	ABORTCHECK;
    break ; 
  case 8 : 
  case 109 : 
  case 9 : 
  case 110 : 
  case 18 : 
  case 119 : 
  case 70 : 
  case 171 : 
  case 51 : 
  case 152 : 
  case 16 : 
  case 117 : 
  case 50 : 
  case 151 : 
  case 53 : 
  case 154 : 
  case 67 : 
  case 168 : 
  case 54 : 
  case 155 : 
  case 55 : 
  case 156 : 
  case 57 : 
  case 158 : 
  case 56 : 
  case 157 : 
  case 31 : 
  case 132 : 
  case 52 : 
  case 153 : 
  case 29 : 
  case 130 : 
  case 47 : 
  case 148 : 
  case 212 : 
  case 216 : 
  case 217 : 
  case 230 : 
  case 227 : 
  case 236 : 
  case 239 : 
    insertdollarsign () ; 
	ABORTCHECK;
    break ; 
  case 37 : 
  case 137 : 
  case 238 : 
    {
      {
		  mem [ curlist .tailfield ] .hh .v.RH = scanrulespec () ; 
		  ABORTCHECK;
		  curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
      if ( abs ( curlist .modefield ) == 1 ) 
		  curlist .auxfield .cint = -65536000L ; 
      else if ( abs ( curlist .modefield ) == 102 ) 
		  curlist .auxfield .hh .v.LH = 1000 ; 
    } 
    break ; 
  case 28 : 
  case 128 : 
  case 229 : 
  case 231 : 
    appendglue () ; 
	ABORTCHECK;
    break ; 
  case 30 : 
  case 131 : 
  case 232 : 
  case 233 : 
    appendkern () ; 
	ABORTCHECK;
    break ; 
  case 2 : 
  case 103 : 
    newsavelevel ( 1 ) ; 
	ABORTCHECK;
    break ; 
  case 62 : 
  case 163 : 
  case 264 : 
    newsavelevel ( 14 ) ; 
	ABORTCHECK;
    break ; 
  case 63 : 
  case 164 : 
  case 265 : 
    if ( curgroup == 14 )  unsave () ; 
    else offsave () ;
	ABORTCHECK;
    break ; 
  case 3 : 
  case 104 : 
  case 205 : 
    handlerightbrace () ; 
	ABORTCHECK;
    break ; 
  case 22 : 
  case 124 : 
  case 225 : 
    {
      t = curchr ; 
      scandimen ( false , false , false ) ; 
	  ABORTCHECK;
      if ( t == 0 ) scanbox ( curval ) ;
      else scanbox ( - (integer) curval ) ;
	  ABORTCHECK;
    } 
    break ; 
  case 32 : 
  case 133 : 
  case 234 : 
/* scan_box(leader_flag-a_leaders+cur_chr); */
    scanbox ( 1073742237L + curchr ) ; /* 2^30 + 513 - 100 ? */
	ABORTCHECK;
    break ; 
  case 21 : 
  case 122 : 
  case 223 : 
    beginbox ( 0 ) ; 
	ABORTCHECK;
    break ; 
  case 44 : 
    newgraf ( curchr > 0 ) ; 
	ABORTCHECK;
    break ; 
  case 12 : 
  case 13 : 
  case 17 : 
  case 69 : 
  case 4 : 
  case 24 : 
  case 36 : 
  case 46 : 
  case 48 : 
  case 27 : 
  case 34 : 
  case 65 : 
  case 66 : 
    {
      backinput () ; 
      newgraf ( true ) ; 
	  ABORTCHECK;
    } 
    break ; 
  case 145 : 
  case 246 : 
    indentinhmode () ; 
	ABORTCHECK;
    break ; 
  case 14 : 
    {
      normalparagraph () ; 
	  ABORTCHECK;
      if ( curlist .modefield > 0 ) {
		  buildpage () ;
		  ABORTCHECK;
	  }
    } 
    break ; 
  case 115 : 
    {
      if ( alignstate < 0 ) {
		  offsave () ;
		  ABORTCHECK;
	  }
      endgraf () ; 
	  ABORTCHECK;
      if ( curlist .modefield == 1 ) {
		  buildpage () ;
		  ABORTCHECK;
	  }
    } 
    break ; 
  case 116 : 
  case 129 : 
  case 138 : 
  case 126 : 
  case 134 : 
    headforvmode () ; 
	ABORTCHECK;
    break ; 
  case 38 : 
  case 139 : 
  case 240 : 
  case 140 : 
  case 241 : 
    begininsertoradjust () ; 
	ABORTCHECK;
    break ; 
  case 19 : 
  case 120 : 
  case 221 : 
    makemark () ; 
	ABORTCHECK;
    break ; 
  case 43 : 
  case 144 : 
  case 245 : 
    appendpenalty () ; 
	ABORTCHECK;
    break ; 
  case 26 : 
  case 127 : 
  case 228 : 
    deletelast () ; 
	ABORTCHECK;
    break ; 
  case 25 : 
  case 125 : 
  case 226 : 
    unpackage () ; 
	ABORTCHECK;
    break ; 
  case 146 : 
    appenditaliccorrection () ; 
	ABORTCHECK;
    break ; 
  case 247 : 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newkern ( 0 ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
	  ABORTCHECK;
    } 
    break ; 
  case 149 : 
  case 250 : 
    appenddiscretionary () ; 
	ABORTCHECK;
    break ; 
  case 147 : 
    makeaccent () ; 
	ABORTCHECK;
    break ; 
  case 6 : 
  case 107 : 
  case 208 : 
  case 5 : 
  case 106 : 
  case 207 : 
    alignerror () ; 
	ABORTCHECK;
    break ; 
  case 35 : 
  case 136 : 
  case 237 : 
    noalignerror () ; 
	ABORTCHECK;
    break ; 
  case 64 : 
  case 165 : 
  case 266 : 
    omiterror () ; 
	ABORTCHECK;
    break ; 
  case 33 : 
  case 135 : 
    initalign () ; 
	ABORTCHECK;
    break ; 
  case 235 : 
    if ( privileged () ) 
    if ( curgroup == 15 ) initalign () ;
    else offsave () ;
	ABORTCHECK;

    break ; 
  case 10 : 
  case 111 : 
    doendv () ; 
	ABORTCHECK;
    break ; 
  case 68 : 
  case 169 : 
  case 270 : 
    cserror () ; 
	ABORTCHECK;
    break ; 
  case 105 : 
    initmath () ; 
	ABORTCHECK;
    break ; 
  case 251 : 
    if ( privileged () ) 
    if ( curgroup == 15 )  starteqno () ; 
    else offsave () ;
	ABORTCHECK;
    break ; 
  case 204 : 
    {
      {
	mem [ curlist .tailfield ] .hh .v.RH = newnoad () ; 
	curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
      backinput () ; 
      scanmath ( curlist .tailfield + 1 ) ; 
    } 
	ABORTCHECK;
    break ; 
  case 214 : 
  case 215 : 
  case 271 : 
    setmathchar ( eqtb [ (hash_size + 2907) + curchr ] .hh .v.RH ) ; 
	ABORTCHECK;
    break ; 
  case 219 : 
    {
      scancharnum () ; 
	  ABORTCHECK;
      curchr = curval ; 
      setmathchar ( eqtb [ (hash_size + 2907) + curchr ] .hh .v.RH ) ; 
    } 
	ABORTCHECK;
    break ; 
  case 220 : 
    {
      scanfifteenbitint () ; 
	  ABORTCHECK;
      setmathchar ( curval ) ; 
    } 
	ABORTCHECK;
    break ; 
  case 272 : 
    setmathchar ( curchr ) ; 
	ABORTCHECK;
    break ; 
  case 218 : 
    {
      scantwentysevenbitint () ; 
	  ABORTCHECK;
      setmathchar ( curval / 4096 ) ;  
/*      setmathchar ( curval >> 12 ) ; */
    } 
	ABORTCHECK;
    break ; 
  case 253 : 
    {
      {
		  mem [ curlist .tailfield ] .hh .v.RH = newnoad () ; 
		  curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
      mem [ curlist .tailfield ] .hh.b0 = curchr ; 
      scanmath ( curlist .tailfield + 1 ) ; 
    } 
	ABORTCHECK;
    break ; 
  case 254 : 
    mathlimitswitch () ; 
	ABORTCHECK;
    break ; 
  case 269 : 
    mathradical () ; 
	ABORTCHECK;
    break ; 
  case 248 : 
  case 249 : 
    mathac () ; 
	ABORTCHECK;
    break ; 
  case 259 : 
    {
      scanspec ( 12 , false ) ; 
      normalparagraph () ; 
      pushnest () ; 
	  ABORTCHECK;
      curlist .modefield = -1 ; 
      curlist .auxfield .cint = -65536000L ; 
      if ( eqtb [ (hash_size + 1318) ] .hh .v.RH != 0 ) /* everyvbox */
		  begintokenlist ( eqtb [ (hash_size + 1318) ] .hh .v.RH , 11 ) ; 
    } 
	ABORTCHECK;
    break ; 
  case 256 : 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newstyle ( curchr ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
	ABORTCHECK;
    break ; 
  case 258 : 
    {
      {
		  mem [ curlist .tailfield ] .hh .v.RH = newglue ( 0 ) ; 
		  curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
      mem [ curlist .tailfield ] .hh.b1 = 98 ; 
    } 
	ABORTCHECK;
    break ; 
  case 257 : 
    appendchoices () ; 
	ABORTCHECK;
    break ; 
  case 211 : 
  case 210 : 
    subsup () ; 
	ABORTCHECK;
    break ; 
  case 255 : 
    mathfraction () ; 
	ABORTCHECK;
    break ; 
  case 252 : 
    mathleftright () ; 
	ABORTCHECK;
    break ; 
  case 206 : 
    if ( curgroup == 15 ) aftermath () ; 
    else offsave () ;
	ABORTCHECK;
    break ; 
  case 72 : 
  case 173 : 
  case 274 : 
  case 73 : 
  case 174 : 
  case 275 : 
  case 74 : 
  case 175 : 
  case 276 : 
  case 75 : 
  case 176 : 
  case 277 : 
  case 76 : 
  case 177 : 
  case 278 : 
  case 77 : 
  case 178 : 
  case 279 : 
  case 78 : 
  case 179 : 
  case 280 : 
  case 79 : 
  case 180 : 
  case 281 : 
  case 80 : 
  case 181 : 
  case 282 : 
  case 81 : 
  case 182 : 
  case 283 : 
  case 82 : 
  case 183 : 
  case 284 : 
  case 83 : 
  case 184 : 
  case 285 : 
  case 84 : 
  case 185 : 
  case 286 : 
  case 85 : 
  case 186 : 
  case 287 : 
  case 86 : 
  case 187 : 
  case 288 : 
  case 87 : 
  case 188 : 
  case 289 : 
  case 88 : 
  case 189 : 
  case 290 : 
  case 89 : 
  case 190 : 
  case 291 : 
  case 90 : 
  case 191 : 
  case 292 : 
  case 91 : 
  case 192 : 
  case 293 : 
  case 92 : 
  case 193 : 
  case 294 : 
  case 93 : 
  case 194 : 
  case 295 : 
  case 94 : 
  case 195 : 
  case 296 : 
  case 95 : 
  case 196 : 
  case 297 : 
  case 96 : 
  case 197 : 
  case 298 : 
  case 97 : 
  case 198 : 
  case 299 : 
  case 98 : 
  case 199 : 
  case 300 : 
  case 99 : 
  case 200 : 
  case 301 : 
  case 100 : 
  case 201 : 
  case 302 : 
  case 101 : 
  case 202 : 
  case 303 : 
    prefixedcommand () ; 
	ABORTCHECK;
    break ; 
  case 41 : 
  case 142 : 
  case 243 : 
    {
      gettoken () ; 
      aftertoken = curtok ; 
    } 
	ABORTCHECK;
    break ; 
  case 42 : 
  case 143 : 
  case 244 : 
    {
      gettoken () ; 
      saveforafter ( curtok ) ; 
    } 
	ABORTCHECK;
    break ; 
  case 61 : 
  case 162 : 
  case 263 : 
    openorclosein () ; 
	ABORTCHECK;
    break ; 
  case 59 : 
  case 160 : 
  case 261 : 
    issuemessage () ; 
	ABORTCHECK;
    break ; 
  case 58 : 
  case 159 : 
  case 260 : 
    shiftcase () ; 
	ABORTCHECK;
    break ; 
  case 20 : 
  case 121 : 
  case 222 : 
    showwhatever () ; 
	ABORTCHECK;
    break ; 
  case 60 : 
  case 161 : 
  case 262 : 
	  doextension () ; 
	  ABORTCHECK;
	  break ; 
	} /* end of big switch */
	ABORTCHECK;		// do it here --- once
	goto lab60 ; /*	main_loop */

lab70:
	ABORTCHECK;
  mains = eqtb [ (hash_size + 2651) + curchr ] .hh .v.RH ; 
  if ( mains == 1000 ) 
	  curlist .auxfield .hh .v.LH = 1000 ; 
  else if ( mains < 1000 ) 
  {
    if ( mains > 0 ) 
		curlist .auxfield .hh .v.LH = mains ; 
  } 
  else if ( curlist .auxfield .hh .v.LH < 1000 ) 
	  curlist .auxfield .hh .v.LH = 1000 ; 
  else curlist .auxfield .hh .v.LH = mains ; 
  mainf = eqtb [ (hash_size + 1834) ] .hh .v.RH ; 
  bchar = fontbchar [ mainf ] ; 
  falsebchar = fontfalsebchar [ mainf ] ; 
  if ( curlist .modefield > 0 ) 
  if ( eqtb [ (hash_size + 3213) ] .cint != curlist .auxfield .hh .v.RH ) 
  fixlanguage () ; 
  {
    ligstack = avail ; 
    if ( ligstack == 0 ) 
		ligstack = getavail () ; 
    else {
      avail = mem [ ligstack ] .hh .v.RH ; 
      mem [ ligstack ] .hh .v.RH = 0 ; 
	;
#ifdef STAT
      incr ( dynused ) ; 
#endif /* STAT */
    } 
  } 
  mem [ ligstack ] .hh.b0 = mainf ; 
  curl = curchr ; 
  mem [ ligstack ] .hh.b1 = curl ; 
  curq = curlist .tailfield ; 
  if ( cancelboundary ) 
  {
/*  begin cancel_boundary:=false; main_k:=non_address; l.20093 */
    cancelboundary = false ; 
/* main_k:=non_address 3.14159 */
/*    maink = fontmemsize ; */		/* OK ? 1993/Nov/29 */
    maink = non_address ;			/* i.e. --- 1995/Jan/15 3.14159 */
  } 
  else maink = bcharlabel [ mainf ] ; 
/* if main_k=non_address then goto main_loop_move+2; l.20096 */
/*  if ( maink == fontmemsize )  */
  if ( maink == non_address )		/* i.e. 0 --- 1995/Jan/15 */
/* cur_r:=cur_l; cur_l:=non_char; */
  goto lab92 ; 
  curr = curl ; 
  curl = 256 ;						/* cur_l:=non_char; */
/* goto main_lig_loop+1; l.20071 */
  goto lab111 ; 

/* main_loop_wrapup:@<Make a ligature node, if |ligature_present|;
  insert a null discretionary, if appropriate@>; */
/* @d wrapup(#)==if cur_l<non_char then */
/*	main_loop_wrapup */
lab80: if ( curl < 256 ) 
  {
/*  begin if character(tail)=qi(hyphen_char[main_f]) then
	if link(cur_q)>null then ... l.20107 */
    if ( mem [ curlist .tailfield ] .hh.b1 == hyphenchar [ mainf ] ) 
/*    if ( mem [ curq ] .hh .v.RH > 0 )  */ /* NO! */
    if ( mem [ curq ] .hh .v.RH != 0 )	/* BUG FIX l.20107 */
    insdisc = true ; 
    if ( ligaturepresent ) 
    {
      mainp = newligature ( mainf , curl , mem [ curq ] .hh .v.RH ) ; 
      if ( lfthit ) 
      {
	mem [ mainp ] .hh.b1 = 2 ; 
	lfthit = false ; 
      } 
      if ( rthit ) 
      if ( ligstack == 0 ) 
      {
	incr ( mem [ mainp ] .hh.b1 ) ; 
	rthit = false ; 
      } 
      mem [ curq ] .hh .v.RH = mainp ; 
      curlist .tailfield = mainp ; 
      ligaturepresent = false ; 
    } 
/*   if ins_disc then l.20110 */
    if ( insdisc ) 
    {
      insdisc = false ; 
/*   if mode>0 then tail_append(new_disc); l.20112 */
      if ( curlist .modefield > 0 ) 
      {
	mem [ curlist .tailfield ] .hh .v.RH = newdisc () ; 
	curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
    } 
  } 

/*	main_loop_move */
lab90:
  if ( ligstack == 0 )  goto lab21 ; 
  curq = curlist .tailfield ; 
  curl = mem [ ligstack ] .hh.b1 ; 

lab91:
  if ( ! ( ligstack >= himemmin ) ) goto lab95 ; 

lab92: if ( ( curchr < fontbc [ mainf ] ) || ( curchr > fontec [ mainf ] ) ) 
  {
    charwarning ( mainf , curchr ) ; 
    {
      mem [ ligstack ] .hh .v.RH = avail ; 
      avail = ligstack ; 
	;
#ifdef STAT
      decr ( dynused ) ; 
#endif /* STAT */
    } 
    goto lab60 ; 
  } 
  maini = fontinfo [ charbase [ mainf ] + curl ] .qqqq ; 
  if ( ! ( maini .b0 > 0 ) ) 
  {
    charwarning ( mainf , curchr ) ; 
    {
      mem [ ligstack ] .hh .v.RH = avail ; 
      avail = ligstack ; 
	;
#ifdef STAT
      decr ( dynused ) ; 
#endif /* STAT */
    } 
    goto lab60 ; 
  } 
  {
    mem [ curlist .tailfield ] .hh .v.RH = ligstack ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 

/*	main_loop_lookahead */
lab100:
  ABORTCHECK;
  getnext () ; 
  ABORTCHECK;
  if ( curcmd == 11 )  goto lab101 ; 
  if ( curcmd == 12 )  goto lab101 ; 
  if ( curcmd == 68 )  goto lab101 ; 
  xtoken () ; 
  ABORTCHECK;
  if ( curcmd == 11 )  goto lab101 ; 
  if ( curcmd == 12 )  goto lab101 ; 
  if ( curcmd == 68 )  goto lab101 ; 
  if ( curcmd == 16 )  {
    scancharnum () ; 
	ABORTCHECK;
    curchr = curval ; 
    goto lab101 ; 
  } 
  if ( curcmd == 65 )  bchar = 256 ; 
  curr = bchar ; 
  ligstack = 0 ; 
  goto lab110 ; 

lab101: mains = eqtb [ (hash_size + 2651) + curchr ] .hh .v.RH ; 
  if ( mains == 1000 ) 
  curlist .auxfield .hh .v.LH = 1000 ; 
  else if ( mains < 1000 ) 
  {
    if ( mains > 0 ) 
    curlist .auxfield .hh .v.LH = mains ; 
  } 
  else if ( curlist .auxfield .hh .v.LH < 1000 ) 
  curlist .auxfield .hh .v.LH = 1000 ; 
  else curlist .auxfield .hh .v.LH = mains ; 
  {
    ligstack = avail ; 
    if ( ligstack == 0 ) 
		ligstack = getavail () ; 
    else {
      avail = mem [ ligstack ] .hh .v.RH ; 
      mem [ ligstack ] .hh .v.RH = 0 ; 
	;
#ifdef STAT
      incr ( dynused ) ; 
#endif /* STAT */
    } 
  } 
  mem [ ligstack ] .hh.b0 = mainf ; 
  curr = curchr ; 
  mem [ ligstack ] .hh.b1 = curr ; 
  if ( curr == falsebchar ) 
	  curr = 256 ; 

// main_lig_loop:@<If there's a ligature/kern command relevant to |cur_l| and
//  |cur_r|, adjust the text appropriately; exit to |main_loop_wrapup|@>;
lab110:
/*	if char_tag(main_i)<>lig_tag then goto main_loop_wrapup; */
  if ( ( ( maini .b2 ) % 4 ) != 1 ) 
	  goto lab80 ; 
/*	main_k:=lig_kern_start(main_f)(main_i); */
  maink = ligkernbase [ mainf ] + maini .b3 ; 
/*	main_j:=font_info[main_k].qqqq; */
  mainj = fontinfo [ maink ] .qqqq ; 
/* if skip_byte(main_j)<=stop_flag then goto main_lig_loop+2; */
  if ( mainj .b0 <= 128 ) goto lab112 ; 
/* main_k:=lig_kern_restart(main_f)(main_j); */
  maink = ligkernbase [ mainf ] + 256 * mainj .b2 + mainj .b3 + 32768L - 256 * 
  ( 128 ) ; 

/* main_lig_loop+1:main_j:=font_info[main_k].qqqq; */
lab111: mainj = fontinfo [ maink ] .qqqq ; 

/* main_lig_loop+2:if next_char(main_j)=cur_r then l.20184 */
lab112:
/*	provide for suppression of f-ligatures 99/Jan/5 */
	bSuppress = 0;
	if ( suppressfligs && mainj .b1 == curr && mainj .b2 == 0) {
		if (curl == 'f') 
				bSuppress = 1;
	}

/*	if ( mainj .b1 == curr ) */
	if ( mainj .b1 == curr && bSuppress == 0)	/* 99/Jan/5 */
/*  if skip_byte(main_j)<=stop_flag then l.20185 */
//   @<Do ligature or kern command, returning to |main_lig_loop|
//	  or |main_loop_wrapup| or |main_loop_move|@>;
  if ( mainj .b0 <= 128 ) 
  {
/* begin if op_byte(main_j)>=kern_flag then l.20225 */
    if ( mainj .b2 >= 128 ) 
    {
/* @d wrapup(#)==if cur_l<non_char then */
      if ( curl < 256 ) 
      {
/* if character(tail)=qi(hyphen_char[main_f]) then if link(cur_q)>null */
	if ( mem [ curlist .tailfield ] .hh.b1 == hyphenchar [ mainf ] ) 
/*	if ( mem [ curq ] .hh .v.RH > 0 )  */	/* 94/Mar/22 ?????????????? */
	if ( mem [ curq ] .hh .v.RH != 0 )  /* BUG FIX l.20107l.20186  */
	insdisc = true ; 
/*   if ligature_present then pack_lig(#); */
	if ( ligaturepresent ) 
	{
	  mainp = newligature ( mainf , curl , mem [ curq ] .hh .v.RH ) ; 
	  if ( lfthit ) 
	  {
	    mem [ mainp ] .hh.b1 = 2 ; 
	    lfthit = false ; 
	  } 
	  if ( rthit ) 
	  if ( ligstack == 0 ) 
	  {
	    incr ( mem [ mainp ] .hh.b1 ) ; 
	    rthit = false ; 
	  } 
	  mem [ curq ] .hh .v.RH = mainp ; 
	  curlist .tailfield = mainp ; 
	  ligaturepresent = false ; 
	} 
	if ( insdisc ) 
	{
	  insdisc = false ; 
	  if ( curlist .modefield > 0 ) 
	  {
	    mem [ curlist .tailfield ] .hh .v.RH = newdisc () ; 
	    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
	  } 
	} 
      } 
      {
	mem [ curlist .tailfield ] .hh .v.RH = newkern ( fontinfo [ kernbase [ 
	mainf ] + 256 * mainj .b2 + mainj .b3 ] .cint ) ; 
	curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
      } 
      goto lab90 ; 
    } 
/* begin if cur_l=non_char then lft_hit:=true; */
    if ( curl == 256 ) 
    lfthit = true ; 
    else if ( ligstack == 0 ) 
    rthit = true ; 
    {
      if ( interrupt != 0 ) {
		  pauseforinstructions () ;
		  ABORTCHECK;
	  }
    } 
    switch ( mainj .b2 ) 
    {case 1 : 
    case 5 : 
      {
	curl = mainj .b3 ; 
	maini = fontinfo [ charbase [ mainf ] + curl ] .qqqq ; 
	ligaturepresent = true ; 
      } 
      break ; 
    case 2 : 
    case 6 : 
      {
	curr = mainj .b3 ; 
	if ( ligstack == 0 ) 
	{
	  ligstack = newligitem ( curr ) ; 
	  bchar = 256 ; 
	} 
	else if ( ( ligstack >= himemmin ) ) 
	{
	  mainp = ligstack ; 
	  ligstack = newligitem ( curr ) ; 
	  mem [ ligstack + 1 ] .hh .v.RH = mainp ; 
	} 
	else mem [ ligstack ] .hh.b1 = curr ; 
      } 
      break ; 
    case 3 : 
      {
	curr = mainj .b3 ; 
	mainp = ligstack ; 
	ligstack = newligitem ( curr ) ; 
	mem [ ligstack ] .hh .v.RH = mainp ; 
      } 
      break ; 
    case 7 : 
    case 11 : 
      {
	if ( curl < 256 )	/* if cur_l<non_char then  */
/*  begin if character(tail)=qi(hyphen_char[main_f]) then if link(cur_q)>null
then */
	{
	  if ( mem [ curlist .tailfield ] .hh.b1 == hyphenchar [ mainf ] ) 
/*	  if ( mem [ curq ] .hh .v.RH > 0 )  */  /* 94/Mar/22 */
	  if ( mem [ curq ] .hh .v.RH != 0 )	/* BUG FIX ???????????? */
	  insdisc = true ; 
	  if ( ligaturepresent ) 
	  {
	    mainp = newligature ( mainf , curl , mem [ curq ] .hh .v.RH ) ; 
	    if ( lfthit ) 
	    {
	      mem [ mainp ] .hh.b1 = 2 ; 
	      lfthit = false ; 
	    } 
/*	    if ( false )
			if ( ligstack == 0 ) {
				incr ( mem [ mainp ] .hh.b1 ) ; 
				rthit = false ; 
			} */							/* removed 99/Jan/6 */
		mem [ curq ] .hh .v.RH = mainp ; 
	    curlist .tailfield = mainp ; 
	    ligaturepresent = false ; 
	  } 
	  if ( insdisc ) 
	  {
	    insdisc = false ; 
	    if ( curlist .modefield > 0 ) 
	    {
	      mem [ curlist .tailfield ] .hh .v.RH = newdisc () ; 
	      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
	    } 
	  } 
	} 
	curq = curlist .tailfield ; 
	curl = mainj .b3 ; 
	maini = fontinfo [ charbase [ mainf ] + curl ] .qqqq ; 
	ligaturepresent = true ; 
      } 
      break ; 
      default: 
      {
	curl = mainj .b3 ; 
	ligaturepresent = true ; 
	if ( ligstack == 0 ) 
	goto lab80 ; 
	else goto lab91 ; 
      } 
      break ; 
    } 
    if ( mainj .b2 > 4 ) 
    if ( mainj .b2 != 7 ) 
    goto lab80 ; 
    if ( curl < 256 ) 
    goto lab110 ; 
    maink = bcharlabel [ mainf ] ; 
    goto lab111 ; 
  } 
  if ( mainj .b0 == 0 ) 
  incr ( maink ) ; 
  else {
    if ( mainj .b0 >= 128 ) 
    goto lab80 ; 
    maink = maink + mainj .b0 + 1 ; 
  } 
  goto lab111 ; 

/*	main_move_log */
lab95: mainp = mem [ ligstack + 1 ] .hh .v.RH ; 
/* if main_p>null then tail_append(main_p); l.20137 */
/*  if ( mainp > 0 )  */	/* 92/Mar/22 */
  if ( mainp != 0 )			/* BUG FIX */
  {
    mem [ curlist .tailfield ] .hh .v.RH = mainp ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  tempptr = ligstack ; 
  ligstack = mem [ tempptr ] .hh .v.RH ; 
  freenode ( tempptr , 2 ) ; 
  maini = fontinfo [ charbase [ mainf ] + curl ] .qqqq ; 
  ligaturepresent = true ; 
  if ( ligstack == 0 ) 
/*   if main_p>null then goto main_loop_lookahead l.20142 */
/*  if ( mainp > 0 )  */ /* 94/Mar/2 */
  if ( mainp != 0 )		/* BUG FIX */
  goto lab100 ; 
  else curr = bchar ; 
  else curr = mem [ ligstack ] .hh.b1 ; 
  goto lab110 ; 

/*	append_normal_space */
lab120: if ( eqtb [ (hash_size + 794) ] .hh .v.RH == 0 ) 
  {
    {
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
    tempptr = newglue ( mainp ) ; 
  } 
  else tempptr = newparamglue ( 12 ) ; 
  mem [ curlist .tailfield ] .hh .v.RH = tempptr ; 
  curlist .tailfield = tempptr ; 
  goto lab60 ; 
} /* end of maincontrol */

/* giveerrhelp etc followed here in the old tex8.c */