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

/* zrebox used to be in tex4.c */

halfword zrebox ( b , w ) 
halfword b ; 
scaled w ; 
{register halfword Result; rebox_regmem 
  halfword p  ; 
  internalfontnumber f  ; 
  scaled v  ; 
/* begin if (width(b)<>w)and(list_ptr(b)<>null) then l.14010 */
  if ( ( mem [ b + 1 ] .cint != w ) && ( mem [ b + 5 ] .hh .v.RH != 0 ) ) 
  {
    if ( mem [ b ] .hh.b0 == 1 ) 
    b = hpack ( b , 0 , 1 ) ; 
    p = mem [ b + 5 ] .hh .v.RH ; 
/*  if (is_char_node(p))and(link(p)=null) then l.14013 */
    if ( ( ( p >= himemmin ) ) && ( mem [ p ] .hh .v.RH == 0 ) ) 
    {
      f = mem [ p ] .hh.b0 ; 
      v = fontinfo [ widthbase [ f ] + fontinfo [ charbase [ f ] + mem [ p ] 
      .hh.b1 ] .qqqq .b0 ] .cint ; 
      if ( v != mem [ b + 1 ] .cint ) 
      mem [ p ] .hh .v.RH = newkern ( mem [ b + 1 ] .cint - v ) ; 
    } 
    freenode ( b , 7 ) ; 
    b = newglue ( 12 ) ; 
    mem [ b ] .hh .v.RH = p ; 
/*   while link(p)<>null do p:=link(p); l.14019 */
    while ( mem [ p ] .hh .v.RH != 0 ) p = mem [ p ] .hh .v.RH ; 
    mem [ p ] .hh .v.RH = newglue ( 12 ) ; 
    Result = hpack ( b , w , 0 ) ; 
  } 
  else {
      
    mem [ b + 1 ] .cint = w ; 
    Result = b ; 
  } 
  return Result ; 
} 

/* This is to be the start of tex5.c */

halfword zmathglue ( g , m ) 
halfword g ; 
scaled m ; 
{register halfword Result; mathglue_regmem 
  halfword p  ; 
  integer n  ; 
  scaled f  ; 
  n = xovern ( m , 65536L ) ; 
  f = texremainder ; 
  if ( f < 0 ) 
  {
    decr ( n ) ; 
    f = f + 65536L ; 
  } 
  p = getnode ( 4 ) ; 
  mem [ p + 1 ] .cint = multandadd ( n , mem [ g + 1 ] .cint , xnoverd ( mem [ 
  g + 1 ] .cint , f , 65536L ) , 1073741823L ) ;  /* 2^30 - 1 */
  mem [ p ] .hh.b0 = mem [ g ] .hh.b0 ; 
  if ( mem [ p ] .hh.b0 == 0 ) 
  mem [ p + 2 ] .cint = multandadd ( n , mem [ g + 2 ] .cint , xnoverd ( mem [ 
  g + 2 ] .cint , f , 65536L ) , 1073741823L ) ;  /* 2^30 - 1 */
  else mem [ p + 2 ] .cint = mem [ g + 2 ] .cint ; 
  mem [ p ] .hh.b1 = mem [ g ] .hh.b1 ; 
  if ( mem [ p ] .hh.b1 == 0 ) 
  mem [ p + 3 ] .cint = multandadd ( n , mem [ g + 3 ] .cint , xnoverd ( mem [ 
  g + 3 ] .cint , f , 65536L ) , 1073741823L ) ;  /* 2^30 - 1 */
  else mem [ p + 3 ] .cint = mem [ g + 3 ] .cint ; 
  Result = p ; 
  return Result ; 
} 
void zmathkern ( p , m ) 
halfword p ; 
scaled m ; 
{mathkern_regmem 
  integer n  ; 
  scaled f  ; 
  if ( mem [ p ] .hh.b1 == 99 ) 
  {
    n = xovern ( m , 65536L ) ; 
    f = texremainder ; 
    if ( f < 0 ) 
    {
      decr ( n ) ; 
      f = f + 65536L ; 
    } 
    mem [ p + 1 ] .cint = multandadd ( n , mem [ p + 1 ] .cint , xnoverd ( mem 
    [ p + 1 ] .cint , f , 65536L ) , 1073741823L ) ;  /* 2^30 - 1 */
/*    mem [ p ] .hh.b1 = 0 ;  */
    mem [ p ] .hh.b1 = 1 ;	/* changed in 3.14159 */
  } 
} 

void flushmath ( ) 
{flushmath_regmem 
  flushnodelist ( mem [ curlist .headfield ] .hh .v.RH ) ; 
  flushnodelist ( curlist .auxfield .cint ) ; 
  mem [ curlist .headfield ] .hh .v.RH = 0 ; 
  curlist .tailfield = curlist .headfield ; 
  curlist .auxfield .cint = 0 ; 
} 

halfword zcleanbox ( p , s ) 
halfword p ; 
smallnumber s ; 
{/* 40 */ register halfword Result; cleanbox_regmem 
  halfword q  ; 
  smallnumber savestyle  ; 
  halfword x  ; 
  halfword r  ; 
  switch ( mem [ p ] .hh .v.RH ) 
  {case 1 : 
    {
      curmlist = newnoad () ; 
      mem [ curmlist + 1 ] = mem [ p ] ; 
    } 
    break ; 
  case 2 : 
    {
      q = mem [ p ] .hh .v.LH ; 
      goto lab40 ; 
    } 
    break ; 
  case 3 : 
    curmlist = mem [ p ] .hh .v.LH ; 
    break ; 
    default: 
    {
      q = newnullbox () ; 
      goto lab40 ; 
    } 
    break ; 
  } 
  savestyle = curstyle ; 
  curstyle = s ; 
  mlistpenalties = false ; 
  mlisttohlist () ; 
  q = mem [ memtop - 3 ] .hh .v.RH ; 
  curstyle = savestyle ; 
  {
    if ( curstyle < 4 ) 
    cursize = 0 ; 
    else cursize = 16 * ( ( curstyle - 2 ) / 2 ) ; 
    curmu = xovern ( fontinfo [ 6 +
		parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint , 18 ) ; 
  } 
  lab40: if ( ( q >= himemmin ) || ( q == 0 ) ) 
  x = hpack ( q , 0 , 1 ) ; 
  else if ( ( mem [ q ] .hh .v.RH == 0 ) && ( mem [ q ] .hh.b0 <= 1 ) && ( mem 
  [ q + 4 ] .cint == 0 ) ) 
  x = q ; 
  else x = hpack ( q , 0 , 1 ) ; 
  q = mem [ x + 5 ] .hh .v.RH ; 
  if ( ( q >= himemmin ) ) 
  {
    r = mem [ q ] .hh .v.RH ; 
/*   if r<>null then if link(r)=null then l.14140 */
    if ( r != 0 ) 
    if ( mem [ r ] .hh .v.RH == 0 ) 
    if ( ! ( r >= himemmin ) ) 
    if ( mem [ r ] .hh.b0 == 11 ) 
    {
      freenode ( r , 2 ) ; 
      mem [ q ] .hh .v.RH = 0 ;		/* link(q):=null; */
    } 
  } 
  Result = x ; 
  return Result ; 
} 

void zfetch ( a ) 
halfword a ; 
{fetch_regmem 
  curc = mem [ a ] .hh.b1 ; 
  curf = eqtb [ (hash_size + 1835) + mem [ a ] .hh.b0 + cursize ] .hh .v.RH ; 
  if ( curf == 0 ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 335 ) ;		/* */
    } 
    printsize ( cursize ) ; 
    printchar ( 32 ) ;		/*   */
    printint ( mem [ a ] .hh.b0 ) ; 
    print ( 878 ) ;			/* is undefined (character  */
    print ( curc ) ; 
    printchar ( 41 ) ;		/* ) */
    {
      helpptr = 4 ; 
      helpline [ 3 ] = 879 ;	/* Somewhere in the math formula just ended, you used the */
      helpline [ 2 ] = 880 ;	/* stated character from an undefined font family. For example, */
      helpline [ 1 ] = 881 ;	/* plain TeX doesn't allow \it or \sl in subscripts. Proceed, */
      helpline [ 0 ] = 882 ;	/* and I'll try to forget that I needed that character. */
    } 
    error () ; 
	ABORTCHECK;
    curi = nullcharacter ; 
    mem [ a ] .hh .v.RH = 0 ; 
  } 
  else {
      
    if ( ( curc >= fontbc [ curf ] ) && ( curc <= fontec [ curf ] ) ) 
    curi = fontinfo [ charbase [ curf ] + curc ] .qqqq ; 
    else curi = nullcharacter ; 
    if ( ! ( ( curi .b0 > 0 ) ) ) 
    {
      charwarning ( curf , curc ) ; 
      mem [ a ] .hh .v.RH = 0 ; 
    } 
  } 
} 

void zmakeover ( q ) 
halfword q ; 
{makeover_regmem 
  mem [ q + 1 ] .hh .v.LH = overbar ( cleanbox ( q + 1 , 2 * ( curstyle / 2 ) 
  + 1 ) , 3 * fontinfo [ 8 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] 
  ] .cint , fontinfo [ 8 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] 
  .cint ) ; 
  mem [ q + 1 ] .hh .v.RH = 2 ; 
} 
void zmakeunder ( q ) 
halfword q ; 
{makeunder_regmem 
  halfword p, x, y  ; 
  scaled delta  ; 
  x = cleanbox ( q + 1 , curstyle ) ; 
  p = newkern ( 3 * fontinfo [ 8 +
	  parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint ) ; 
  mem [ x ] .hh .v.RH = p ; 
  mem [ p ] .hh .v.RH = fractionrule ( fontinfo [ 8 +
	  parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint ) ; 
  y = vpackage ( x , 0 , 1 , 1073741823L ) ; /* 2^30 - 1 */
  delta = mem [ y + 3 ] .cint + mem [ y + 2 ] .cint + fontinfo [ 8 +
	  parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint ; 
  mem [ y + 3 ] .cint = mem [ x + 3 ] .cint ; 
  mem [ y + 2 ] .cint = delta - mem [ y + 3 ] .cint ; 
  mem [ q + 1 ] .hh .v.LH = y ; 
  mem [ q + 1 ] .hh .v.RH = 2 ; 
} 

void zmakevcenter ( q ) 
halfword q ; 
{makevcenter_regmem 
  halfword v  ; 
  scaled delta  ; 
  v = mem [ q + 1 ] .hh .v.LH ; 
  if ( mem [ v ] .hh.b0 != 1 ) {
	  confusion ( 536 ) ;		/* vcenter */
	  return;					// abortflag set
  }
  delta = mem [ v + 3 ] .cint + mem [ v + 2 ] .cint ; 
  mem [ v + 3 ] .cint = fontinfo [ 22 +
		parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint + half ( delta ) ; 
  mem [ v + 2 ] .cint = delta - mem [ v + 3 ] .cint ; 
} 

void zmakeradical ( q ) 
halfword q ; 
{makeradical_regmem 
  halfword x, y  ; 
  scaled delta, clr  ; 
  x = cleanbox ( q + 1 , 2 * ( curstyle / 2 ) + 1 ) ; 
  if ( curstyle < 2 ) 
  clr = fontinfo [ 8 +
	parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint +
		( abs ( fontinfo [ 5 + parambase [ eqtb [ (hash_size + 1837) +
				cursize ] .hh .v.RH ] ] .cint ) / 4 ) ; 
  else {
      
    clr = fontinfo [ 8 +
		  parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint ; 
    clr = clr + ( abs ( clr ) / 4 ) ; 
  } 
  y = vardelimiter ( q + 4 , cursize , mem [ x + 3 ] .cint + mem [ x + 2 ] 
  .cint + clr + fontinfo [ 8 +
		parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint ) ; 
  delta = mem [ y + 2 ] .cint - ( mem [ x + 3 ] .cint + mem [ x + 2 ] .cint + 
  clr ) ; 
  if ( delta > 0 ) 
  clr = clr + half ( delta ) ; 
  mem [ y + 4 ] .cint = - (integer) ( mem [ x + 3 ] .cint + clr ) ; 
  mem [ y ] .hh .v.RH = overbar ( x , clr , mem [ y + 3 ] .cint ) ; 
  mem [ q + 1 ] .hh .v.LH = hpack ( y , 0 , 1 ) ; 
  mem [ q + 1 ] .hh .v.RH = 2 ; 
} 

void zmakemathaccent ( q ) 
halfword q ; 
{/* 30 31 */ makemathaccent_regmem 
  halfword p, x, y  ; 
  integer a  ; 
  quarterword c  ; 
  internalfontnumber f  ; 
  ffourquarters i  ; 
  scaled s  ; 
  scaled h  ; 
  scaled delta  ; 
  scaled w  ; 
  fetch ( q + 4 ) ; 
  if ( ( curi .b0 > 0 ) ) 
  {
    i = curi ; 
    c = curc ; 
    f = curf ; 
    s = 0 ; 
    if ( mem [ q + 1 ] .hh .v.RH == 1 ) 
    {
      fetch ( q + 1 ) ; 
      if ( ( ( curi .b2 ) % 4 ) == 1 ) 
      {
	a = ligkernbase [ curf ] + curi .b3 ; 
	curi = fontinfo [ a ] .qqqq ; 
	if ( curi .b0 > 128 ) 
	{
	  a = ligkernbase [ curf ] + 256 * curi .b2 + curi .b3 + 32768L - 256 
	  * ( 128 ) ; 
	  curi = fontinfo [ a ] .qqqq ; 
	} 
	while ( true ) {
	  if ( curi .b1 == skewchar [ curf ] ) 
	  {
	    if ( curi .b2 >= 128 ) 
	    if ( curi .b0 <= 128 ) 
	    s = fontinfo [ kernbase [ curf ] + 256 * curi .b2 + curi .b3 ] 
	    .cint ; 
	    goto lab31 ; 
	  } 
	  if ( curi .b0 >= 128 ) 
	  goto lab31 ; 
	  a = a + curi .b0 + 1 ; 
	  curi = fontinfo [ a ] .qqqq ; 
	} 
      } 
    } 
    lab31: ; 
    x = cleanbox ( q + 1 , 2 * ( curstyle / 2 ) + 1 ) ; 
    w = mem [ x + 1 ] .cint ; 
    h = mem [ x + 3 ] .cint ; 
    while ( true ) {
      if ( ( ( i .b2 ) % 4 ) != 2 ) 
      goto lab30 ; 
      y = i .b3 ; 
      i = fontinfo [ charbase [ f ] + y ] .qqqq ; 
      if ( ! ( i .b0 > 0 ) ) 
      goto lab30 ; 
      if ( fontinfo [ widthbase [ f ] + i .b0 ] .cint > w ) 
      goto lab30 ; 
/*	  long to unsigned short ... */
      c = y ; 
    } 
    lab30: ; 
    if ( h < fontinfo [ 5 + parambase [ f ] ] .cint ) 
    delta = h ; 
    else delta = fontinfo [ 5 + parambase [ f ] ] .cint ; 
    if ( ( mem [ q + 2 ] .hh .v.RH != 0 ) || ( mem [ q + 3 ] .hh .v.RH != 0 ) 
    ) 
    if ( mem [ q + 1 ] .hh .v.RH == 1 ) 
    {
      flushnodelist ( x ) ; 
      x = newnoad () ; 
      mem [ x + 1 ] = mem [ q + 1 ] ; 
      mem [ x + 2 ] = mem [ q + 2 ] ; 
      mem [ x + 3 ] = mem [ q + 3 ] ; 
      mem [ q + 2 ] .hh = emptyfield ; 
      mem [ q + 3 ] .hh = emptyfield ; 
      mem [ q + 1 ] .hh .v.RH = 3 ; 
      mem [ q + 1 ] .hh .v.LH = x ; 
      x = cleanbox ( q + 1 , curstyle ) ; 
      delta = delta + mem [ x + 3 ] .cint - h ; 
      h = mem [ x + 3 ] .cint ; 
    } 
    y = charbox ( f , c ) ; 
    mem [ y + 4 ] .cint = s + half ( w - mem [ y + 1 ] .cint ) ; 
    mem [ y + 1 ] .cint = 0 ; 
    p = newkern ( - (integer) delta ) ; 
    mem [ p ] .hh .v.RH = x ; 
    mem [ y ] .hh .v.RH = p ; 
    y = vpackage ( y , 0 , 1 , 1073741823L ) ;  /* 2^30 - 1 */
    mem [ y + 1 ] .cint = mem [ x + 1 ] .cint ; 
    if ( mem [ y + 3 ] .cint < h ) 
    {
      p = newkern ( h - mem [ y + 3 ] .cint ) ; 
      mem [ p ] .hh .v.RH = mem [ y + 5 ] .hh .v.RH ; 
      mem [ y + 5 ] .hh .v.RH = p ; 
      mem [ y + 3 ] .cint = h ; 
    } 
    mem [ q + 1 ] .hh .v.LH = y ; 
    mem [ q + 1 ] .hh .v.RH = 2 ; 
  } 
} 
void zmakefraction ( q ) 
halfword q ; 
{makefraction_regmem 
  halfword p, v, x, y, z  ; 
  scaled delta, delta1, delta2, shiftup, shiftdown, clr  ; 
  if ( mem [ q + 1 ] .cint == 1073741824L )  /* 2^30 */
  mem [ q + 1 ] .cint = fontinfo [ 8 +
	  parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint ; 
  x = cleanbox ( q + 2 , curstyle + 2 - 2 * ( curstyle / 6 ) ) ; 
  z = cleanbox ( q + 3 , 2 * ( curstyle / 2 ) + 3 - 2 * ( curstyle / 6 ) ) ; 
  if ( mem [ x + 1 ] .cint < mem [ z + 1 ] .cint ) 
  x = rebox ( x , mem [ z + 1 ] .cint ) ; 
  else z = rebox ( z , mem [ x + 1 ] .cint ) ; 
  if ( curstyle < 2 ) 
  {
    shiftup = fontinfo [ 8 +
	    parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint ; 
    shiftdown = fontinfo [ 11 +
		parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint ; 
  } 
  else {
      
    shiftdown = fontinfo [ 12 +
		parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint ; 
    if ( mem [ q + 1 ] .cint != 0 ) 
    shiftup = fontinfo [ 9 +
		parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint ; 
    else shiftup = fontinfo [ 10 +
		parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint ;
  } 
  if ( mem [ q + 1 ] .cint == 0 ) 
  {
    if ( curstyle < 2 ) 
    clr = 7 * fontinfo [ 8 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] 
    ] .cint ; 
    else clr = 3 * fontinfo [ 8 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh 
    .v.RH ] ] .cint ; 
    delta = half ( clr - ( ( shiftup - mem [ x + 2 ] .cint ) - ( mem [ z + 3 ] 
    .cint - shiftdown ) ) ) ; 
    if ( delta > 0 ) 
    {
      shiftup = shiftup + delta ; 
      shiftdown = shiftdown + delta ; 
    } 
  } 
  else {
      
    if ( curstyle < 2 ) 
    clr = 3 * mem [ q + 1 ] .cint ; 
    else clr = mem [ q + 1 ] .cint ; 
    delta = half ( mem [ q + 1 ] .cint ) ; 
    delta1 = clr - ( ( shiftup - mem [ x + 2 ] .cint ) - ( fontinfo [ 22 + 
    parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint + delta ) ) ; 
    delta2 = clr - ( ( fontinfo [ 22 + parambase [ eqtb [ (hash_size + 1837) + cursize ] 
    .hh .v.RH ] ] .cint - delta ) - ( mem [ z + 3 ] .cint - shiftdown ) ) ; 
    if ( delta1 > 0 ) 
    shiftup = shiftup + delta1 ; 
    if ( delta2 > 0 ) 
    shiftdown = shiftdown + delta2 ; 
  } 
  v = newnullbox () ; 
  mem [ v ] .hh.b0 = 1 ; 
  mem [ v + 3 ] .cint = shiftup + mem [ x + 3 ] .cint ; 
  mem [ v + 2 ] .cint = mem [ z + 2 ] .cint + shiftdown ; 
  mem [ v + 1 ] .cint = mem [ x + 1 ] .cint ; 
  if ( mem [ q + 1 ] .cint == 0 ) 
  {
    p = newkern ( ( shiftup - mem [ x + 2 ] .cint ) - ( mem [ z + 3 ] .cint - 
    shiftdown ) ) ; 
    mem [ p ] .hh .v.RH = z ; 
  } 
  else {
      
    y = fractionrule ( mem [ q + 1 ] .cint ) ; 
    p = newkern ( ( fontinfo [ 22 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh 
    .v.RH ] ] .cint - delta ) - ( mem [ z + 3 ] .cint - shiftdown ) ) ; 
    mem [ y ] .hh .v.RH = p ; 
    mem [ p ] .hh .v.RH = z ; 
    p = newkern ( ( shiftup - mem [ x + 2 ] .cint ) - ( fontinfo [ 22 + 
    parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint + delta ) ) ; 
    mem [ p ] .hh .v.RH = y ; 
  } 
  mem [ x ] .hh .v.RH = p ; 
  mem [ v + 5 ] .hh .v.RH = x ; 
  if ( curstyle < 2 ) 
  delta = fontinfo [ 20 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] 
  .cint ; 
  else delta = fontinfo [ 21 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH 
  ] ] .cint ; 
  x = vardelimiter ( q + 4 , cursize , delta ) ; 
  mem [ x ] .hh .v.RH = v ; 
  z = vardelimiter ( q + 5 , cursize , delta ) ; 
  mem [ v ] .hh .v.RH = z ; 
  mem [ q + 1 ] .cint = hpack ( x , 0 , 1 ) ; 
} 

/***************************************************************************/

/* moved to end to avoid questions about pragma optimize 96/Sep/12 */

scaled zmakeop (halfword);

/***************************************************************************/

void zmakeord ( q ) 
halfword q ; 
{/* 20 10 */ makeord_regmem 
  integer a  ; 
  halfword p, r  ; 
  lab20: if ( mem [ q + 3 ] .hh .v.RH == 0 ) 
  if ( mem [ q + 2 ] .hh .v.RH == 0 ) 
  if ( mem [ q + 1 ] .hh .v.RH == 1 ) 
  {
    p = mem [ q ] .hh .v.RH ; 
    if ( p != 0 ) 
    if ( ( mem [ p ] .hh.b0 >= 16 ) && ( mem [ p ] .hh.b0 <= 22 ) ) 
    if ( mem [ p + 1 ] .hh .v.RH == 1 ) 
    if ( mem [ p + 1 ] .hh.b0 == mem [ q + 1 ] .hh.b0 ) 
    {
      mem [ q + 1 ] .hh .v.RH = 4 ; 
      fetch ( q + 1 ) ; 
      if ( ( ( curi .b2 ) % 4 ) == 1 ) 
      {
	a = ligkernbase [ curf ] + curi .b3 ; 
	curc = mem [ p + 1 ] .hh.b1 ; 
	curi = fontinfo [ a ] .qqqq ; 
	if ( curi .b0 > 128 ) 
	{
	  a = ligkernbase [ curf ] + 256 * curi .b2 + curi .b3 + 32768L - 256 
	  * ( 128 ) ; 
	  curi = fontinfo [ a ] .qqqq ; 
	} 
	while ( true ) {
	  if ( curi .b1 == curc ) 
	  if ( curi .b0 <= 128 ) 
	  if ( curi .b2 >= 128 ) 
	  {
	    p = newkern ( fontinfo [ kernbase [ curf ] + 256 * curi .b2 + curi 
	    .b3 ] .cint ) ; 
	    mem [ p ] .hh .v.RH = mem [ q ] .hh .v.RH ; 
	    mem [ q ] .hh .v.RH = p ; 
	    return ; 
	  } 
	  else {
	      
	    {
	      if ( interrupt != 0 ) {
			  pauseforinstructions () ;
			  ABORTCHECK;
		  }
	    } 
	    switch ( curi .b2 ) 
	    {case 1 : 
	    case 5 : 
	      mem [ q + 1 ] .hh.b1 = curi .b3 ; 
	      break ; 
	    case 2 : 
	    case 6 : 
	      mem [ p + 1 ] .hh.b1 = curi .b3 ; 
	      break ; 
	    case 3 : 
	    case 7 : 
	    case 11 : 
	      {
		r = newnoad () ; 
		mem [ r + 1 ] .hh.b1 = curi .b3 ; 
		mem [ r + 1 ] .hh.b0 = mem [ q + 1 ] .hh.b0 ; 
		mem [ q ] .hh .v.RH = r ; 
		mem [ r ] .hh .v.RH = p ; 
		if ( curi .b2 < 11 ) 
		mem [ r + 1 ] .hh .v.RH = 1 ; 
		else mem [ r + 1 ] .hh .v.RH = 4 ; 
	      } 
	      break ; 
	      default: 
	      {
		mem [ q ] .hh .v.RH = mem [ p ] .hh .v.RH ; 
		mem [ q + 1 ] .hh.b1 = curi .b3 ; 
		mem [ q + 3 ] = mem [ p + 3 ] ; 
		mem [ q + 2 ] = mem [ p + 2 ] ; 
		freenode ( p , 4 ) ; 
	      } 
	      break ; 
	    } 
	    if ( curi .b2 > 3 ) 
	    return ; 
	    mem [ q + 1 ] .hh .v.RH = 1 ; 
	    goto lab20 ; 
	  } 
	  if ( curi .b0 >= 128 ) 
	  return ; 
	  a = a + curi .b0 + 1 ; 
	  curi = fontinfo [ a ] .qqqq ; 
	} 
      } 
    } 
  } 
} 

/***************************************************************************/

/* moved to end to avoid questions about pragma optimize 96/Sep/12 */

void zmakescripts (halfword, scaled);

/***************************************************************************/

smallnumber zmakeleftright ( q , style , maxd , maxh ) 
halfword q ; 
smallnumber style ; 
scaled maxd ; 
scaled maxh ; 
{register smallnumber Result; makeleftright_regmem 
  scaled delta, delta1, delta2  ; 
  if ( style < 4 ) 
  cursize = 0 ; 
  else cursize = 16 * ( ( style - 2 ) / 2 ) ; 
  delta2 = maxd + fontinfo [ 22 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh 
  .v.RH ] ] .cint ; 
  delta1 = maxh + maxd - delta2 ; 
  if ( delta2 > delta1 ) 
  delta1 = delta2 ; 
  delta = ( delta1 / 500 ) * eqtb [ (hash_size + 3181) ] .cint ; 
  delta2 = delta1 + delta1 - eqtb [ (hash_size + 3740) ] .cint ; 
  if ( delta < delta2 ) 
  delta = delta2 ; 
  mem [ q + 1 ] .cint = vardelimiter ( q + 1 , cursize , delta ) ; 
  Result = mem [ q ] .hh.b0 - ( 10 ) ; 
  return Result ; 
} 

void mlisttohlist ( ) 
{/* 21 82 80 81 83 30 */ mlisttohlist_regmem 
  halfword mlist  ; 
  booleane penalties  ; 
  smallnumber style  ; 
  smallnumber savestyle  ; 
  halfword q  ; 
  halfword r  ; 
/*  smallnumber rtype  ;  */
  int rtype  ;						/* 95/Jan/7 */
/*  smallnumber t  ; */
  int t  ;							/* 95/Jan/7 */
  halfword p, x, y, z  ; 
  integer pen  ; 
  smallnumber s  ; 
  scaled maxh, maxd  ; 
  scaled delta  ; 
  mlist = curmlist ; 
  penalties = mlistpenalties ; 
  style = curstyle ; 
  q = mlist ; 
  r = 0 ; 
  rtype = 17 ; 
  maxh = 0 ; 
  maxd = 0 ; 
  {
    if ( curstyle < 4 ) 
    cursize = 0 ; 
    else cursize = 16 * ( ( curstyle - 2 ) / 2 ) ; 
    curmu = xovern ( fontinfo [ 6 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh 
    .v.RH ] ] .cint , 18 ) ; 
  } 
  while ( q != 0 ) {
      
    lab21: delta = 0 ; 
    switch ( mem [ q ] .hh.b0 ) {
		case 18 : 
      switch ( rtype ) 
      {case 18 : 
      case 17 : 
      case 19 : 
      case 20 : 
      case 22 : 
      case 30 : 
	{
	  mem [ q ] .hh.b0 = 16 ; 
	  goto lab21 ; 
	} 
	break ; 
	default: 
	; 
	break ; 
      } 
      break ; 
    case 19 : 
    case 21 : 
    case 22 : 
    case 31 : 
      {
	if ( rtype == 18 ) 	mem [ r ] .hh.b0 = 16 ; 
	if ( mem [ q ] .hh.b0 == 31 ) 	goto lab80 ; 
      } 
      break ; 
    case 30 : 
      goto lab80 ; 
      break ; 
    case 25 : 
      {
	makefraction ( q ) ; 
	goto lab82 ; 
      } 
      break ; 
    case 17 : 
      {
	delta = makeop ( q ) ; 
	if ( mem [ q ] .hh.b1 == 1 ) 
	goto lab82 ; 
      } 
      break ; 
    case 16 : 
      makeord ( q ) ; 
	  ABORTCHECK;
      break ; 
    case 20 : 
    case 23 : 
      ; 
      break ; 
    case 24 : 
      makeradical ( q ) ; 
      break ; 
    case 27 : 
      makeover ( q ) ; 
      break ; 
    case 26 : 
      makeunder ( q ) ; 
      break ; 
    case 28 : 
      makemathaccent ( q ) ; 
      break ; 
    case 29 : 
      makevcenter ( q ) ; 
      break ; 
    case 14 : 
      {
	curstyle = mem [ q ] .hh.b1 ; 
	{
	  if ( curstyle < 4 ) 
	  cursize = 0 ; 
	  else cursize = 16 * ( ( curstyle - 2 ) / 2 ) ; 
	  curmu = xovern ( fontinfo [ 6 + parambase [ eqtb [ (hash_size + 1837) + cursize ] 
	  .hh .v.RH ] ] .cint , 18 ) ; 
	} 
	goto lab81 ; 
      } 
      break ; 
    case 15 : 
      {
	switch ( curstyle / 2 ) 
	{case 0 : 
	  {
	    p = mem [ q + 1 ] .hh .v.LH ; 
	    mem [ q + 1 ] .hh .v.LH = 0 ; 
	  } 
	  break ; 
	case 1 : 
	  {
	    p = mem [ q + 1 ] .hh .v.RH ; 
	    mem [ q + 1 ] .hh .v.RH = 0 ; 
	  } 
	  break ; 
	case 2 : 
	  {
	    p = mem [ q + 2 ] .hh .v.LH ; 
	    mem [ q + 2 ] .hh .v.LH = 0 ; 
	  } 
	  break ; 
	case 3 : 
	  {
	    p = mem [ q + 2 ] .hh .v.RH ; 
	    mem [ q + 2 ] .hh .v.RH = 0 ; 
	  } 
	  break ; 
	} 
	flushnodelist ( mem [ q + 1 ] .hh .v.LH ) ; 
	flushnodelist ( mem [ q + 1 ] .hh .v.RH ) ; 
	flushnodelist ( mem [ q + 2 ] .hh .v.LH ) ; 
	flushnodelist ( mem [ q + 2 ] .hh .v.RH ) ; 
	mem [ q ] .hh.b0 = 14 ; 
	mem [ q ] .hh.b1 = curstyle ; 
	mem [ q + 1 ] .cint = 0 ; 
	mem [ q + 2 ] .cint = 0 ; 
	if ( p != 0 ) /* if p<>null then l.14317 */
	{
	  z = mem [ q ] .hh .v.RH ; 
	  mem [ q ] .hh .v.RH = p ; 
/*   while link(p)<>null do p:=link(p); */
	  while ( mem [ p ] .hh .v.RH != 0 ) p = mem [ p ] .hh .v.RH ; 
	  mem [ p ] .hh .v.RH = z ; 
	} 
	goto lab81 ; 
      } 
      break ; 
    case 3 : 
    case 4 : 
    case 5 : 
    case 8 : 
    case 12 : 
    case 7 : 
      goto lab81 ; 
      break ; 
    case 2 : 
      {
	if ( mem [ q + 3 ] .cint > maxh ) 
	maxh = mem [ q + 3 ] .cint ; 
	if ( mem [ q + 2 ] .cint > maxd ) 
	maxd = mem [ q + 2 ] .cint ; 
	goto lab81 ; 
      } 
      break ; 
    case 10 : 
      {
	if ( mem [ q ] .hh.b1 == 99 ) 
	{
	  x = mem [ q + 1 ] .hh .v.LH ; 
	  y = mathglue ( x , curmu ) ; 
	  deleteglueref ( x ) ; 
	  mem [ q + 1 ] .hh .v.LH = y ; 
	  mem [ q ] .hh.b1 = 0 ; 
	} 
	else if ( ( cursize != 0 ) && ( mem [ q ] .hh.b1 == 98 ) ) 
	{
	  p = mem [ q ] .hh .v.RH ; 
/*   if p<>null then if (type(p)=glue_node)or(type(p)=kern_node) then */
	  if ( p != 0 ) 
	  if ( ( mem [ p ] .hh.b0 == 10 ) || ( mem [ p ] .hh.b0 == 11 ) ) 
	  {
	    mem [ q ] .hh .v.RH = mem [ p ] .hh .v.RH ; 
	    mem [ p ] .hh .v.RH = 0 ; 
	    flushnodelist ( p ) ; 
	  } 
	} 
	goto lab81 ; 
      } 
      break ; 
    case 11 : 
      {
	mathkern ( q , curmu ) ; 
	goto lab81 ; 
      } 
      break ; 
      default: 
		  {
			  confusion ( 883 ) ;	/* mlist1 */
			  return;				// abortflag set
		  }
		  break ; 
	} /* end of switch */

    switch ( mem [ q + 1 ] .hh .v.RH ) {
	case 1 : 
    case 4 : 
      {
	fetch ( q + 1 ) ; 
	if ( ( curi .b0 > 0 ) ) 
	{
	  delta = fontinfo [ italicbase [ curf ] + ( curi .b2 ) / 4 ] .cint ; 
	  p = newcharacter ( curf , curc ) ; 
	  if ( ( mem [ q + 1 ] .hh .v.RH == 4 ) && ( fontinfo [ 2 + parambase 
	  [ curf ] ] .cint != 0 ) ) 
	  delta = 0 ; 
	  if ( ( mem [ q + 3 ] .hh .v.RH == 0 ) && ( delta != 0 ) ) 
	  {
	    mem [ p ] .hh .v.RH = newkern ( delta ) ; 
	    delta = 0 ; 
	  } 
	} 
	else p = 0 ; 
      } 
      break ; 
    case 0 : 
      p = 0 ; 
      break ; 
    case 2 : 
      p = mem [ q + 1 ] .hh .v.LH ; 
      break ; 
    case 3 : 
      {
	curmlist = mem [ q + 1 ] .hh .v.LH ; 
	savestyle = curstyle ; 
	mlistpenalties = false ; 
	mlisttohlist () ; 
	curstyle = savestyle ; 
	{
	  if ( curstyle < 4 ) 
	  cursize = 0 ; 
	  else cursize = 16 * ( ( curstyle - 2 ) / 2 ) ; 
	  curmu = xovern ( fontinfo [ 6 + parambase [ eqtb [ (hash_size + 1837) + cursize ] 
	  .hh .v.RH ] ] .cint , 18 ) ; 
	} 
	p = hpack ( mem [ memtop - 3 ] .hh .v.RH , 0 , 1 ) ; 
      } 
      break ; 
      default: 
		  {
			  confusion ( 884 ) ;	/* mlist2 */
			  return;				// abortflag set
		  }
		  break ; 
	} /* end of switch */
	
    mem [ q + 1 ] .cint = p ;	/* p may be used without ... */
    if ( ( mem [ q + 3 ] .hh .v.RH == 0 ) && ( mem [ q + 2 ] .hh .v.RH == 0 ) 
    ) 
    goto lab82 ; 
    makescripts ( q , delta ) ; 
    lab82: z = hpack ( mem [ q + 1 ] .cint , 0 , 1 ) ; 
    if ( mem [ z + 3 ] .cint > maxh ) 
    maxh = mem [ z + 3 ] .cint ; 
    if ( mem [ z + 2 ] .cint > maxd ) 
    maxd = mem [ z + 2 ] .cint ; 
    freenode ( z , 7 ) ; 
    lab80: r = q ; 
    rtype = mem [ r ] .hh.b0 ; 
    lab81: q = mem [ q ] .hh .v.RH ; 
  }
  if ( rtype == 18 ) 
  mem [ r ] .hh.b0 = 16 ; 
  p = memtop - 3 ; 
  mem [ p ] .hh .v.RH = 0 ; 
  q = mlist ; 
  rtype = 0 ; 
  curstyle = style ; 
  {
    if ( curstyle < 4 ) 
    cursize = 0 ; 
    else cursize = 16 * ( ( curstyle - 2 ) / 2 ) ; 
    curmu = xovern ( fontinfo [ 6 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh 
    .v.RH ] ] .cint , 18 ) ; 
  } 
  while ( q != 0 ) {
      
    t = 16 ; 
    s = 4 ; 
    pen = 10000 ; 
    switch ( mem [ q ] .hh.b0 ) 
    {case 17 : 
    case 20 : 
    case 21 : 
    case 22 : 
    case 23 : 
      t = mem [ q ] .hh.b0 ; 
      break ; 
    case 18 : 
      {
	t = 18 ; 
	pen = eqtb [ (hash_size + 3172) ] .cint ; 
      } 
      break ; 
    case 19 : 
      {
	t = 19 ; 
	pen = eqtb [ (hash_size + 3173) ] .cint ; 
      } 
      break ; 
    case 16 : 
    case 29 : 
    case 27 : 
    case 26 : 
      ; 
      break ; 
    case 24 : 
      s = 5 ; 
      break ; 
    case 28 : 
      s = 5 ; 
      break ; 
    case 25 : 
      {
	t = 23 ; 
	s = 6 ; 
      } 
      break ; 
    case 30 : 
    case 31 : 
      t = makeleftright ( q , style , maxd , maxh ) ; 
      break ; 
    case 14 : 
      {
	curstyle = mem [ q ] .hh.b1 ; 
	s = 3 ; 
	{
	  if ( curstyle < 4 ) 
	  cursize = 0 ; 
	  else cursize = 16 * ( ( curstyle - 2 ) / 2 ) ; 
	  curmu = xovern ( fontinfo [ 6 + parambase [ eqtb [ (hash_size + 1837) + cursize ] 
	  .hh .v.RH ] ] .cint , 18 ) ; 
	} 
	goto lab83 ; 
      } 
      break ; 
    case 8 : 
    case 12 : 
    case 2 : 
    case 7 : 
    case 5 : 
    case 3 : 
    case 4 : 
    case 10 : 
    case 11 : 
      {
	mem [ p ] .hh .v.RH = q ; 
	p = q ; 
	q = mem [ q ] .hh .v.RH ; 
	mem [ p ] .hh .v.RH = 0 ; 
	goto lab30 ; 
      } 
      break ; 
      default: 
		  {
			  confusion ( 885 ) ;	/* mlist3 */
			  return;				// abortflag set
		  }
		  break ; 
    } 
    if ( rtype > 0 ) 
    {
      switch ( strpool [ rtype * 8 + t + magicoffset ] ) 
      {case 48 : 
	x = 0 ; 
	break ; 
      case 49 : 
	if ( curstyle < 4 ) 
	x = 15 ; 
	else x = 0 ; 
	break ; 
      case 50 : 
	x = 15 ; 
	break ; 
      case 51 : 
	if ( curstyle < 4 ) 
	x = 16 ; 
	else x = 0 ; 
	break ; 
      case 52 : 
	if ( curstyle < 4 ) 
	x = 17 ; 
	else x = 0 ; 
	break ; 
	default: 
		{
			confusion ( 887 ) ;		/* mlist4 */
			return;				// abortflag set
		}
		break ; 
      } 
      if ( x != 0 ) 
      {
	y = mathglue ( eqtb [ (hash_size + 782) + x ] .hh .v.RH , curmu ) ;	/* gluebase + x */
	z = newglue ( y ) ; 
	mem [ y ] .hh .v.RH = 0 ; 
	mem [ p ] .hh .v.RH = z ; 
	p = z ; 
	mem [ z ] .hh.b1 = x + 1 ;		/* x may be used without ... */
      } 
    } 
    if ( mem [ q + 1 ] .cint != 0 ) 
    {
      mem [ p ] .hh .v.RH = mem [ q + 1 ] .cint ; 
      do {
	  p = mem [ p ] .hh .v.RH ; 
      } while ( ! ( mem [ p ] .hh .v.RH == 0 ) ) ; 
    } 
    if ( penalties ) 
    if ( mem [ q ] .hh .v.RH != 0 ) 
    if ( pen < 10000 ) 
    {
      rtype = mem [ mem [ q ] .hh .v.RH ] .hh.b0 ; 
      if ( rtype != 12 ) 
      if ( rtype != 19 ) 
      {
	z = newpenalty ( pen ) ; 
	mem [ p ] .hh .v.RH = z ; 
	p = z ; 
      } 
    } 
    rtype = t ; 
    lab83: r = q ; 
    q = mem [ q ] .hh .v.RH ; 
    freenode ( r , s ) ; 
    lab30: ; 
  } 
} 

void pushalignment ( ) 
{pushalignment_regmem 
  halfword p  ; 
  p = getnode ( 5 ) ; 
  mem [ p ] .hh .v.RH = alignptr ; 
  mem [ p ] .hh .v.LH = curalign ; 
  mem [ p + 1 ] .hh .v.LH = mem [ memtop - 8 ] .hh .v.RH ; 
  mem [ p + 1 ] .hh .v.RH = curspan ; 
  mem [ p + 2 ] .cint = curloop ; 
  mem [ p + 3 ] .cint = alignstate ; 
  mem [ p + 4 ] .hh .v.LH = curhead ; 
  mem [ p + 4 ] .hh .v.RH = curtail ; 
  alignptr = p ; 
  curhead = getavail () ; 
} 
void popalignment ( ) 
{popalignment_regmem 
  halfword p  ; 
  {
    mem [ curhead ] .hh .v.RH = avail ; 
    avail = curhead ; 
	;
#ifdef STAT
    decr ( dynused ) ; 
#endif /* STAT */
  } 
  p = alignptr ; 
  curtail = mem [ p + 4 ] .hh .v.RH ; 
  curhead = mem [ p + 4 ] .hh .v.LH ; 
  alignstate = mem [ p + 3 ] .cint ; 
  curloop = mem [ p + 2 ] .cint ; 
  curspan = mem [ p + 1 ] .hh .v.RH ; 
  mem [ memtop - 8 ] .hh .v.RH = mem [ p + 1 ] .hh .v.LH ; 
  curalign = mem [ p ] .hh .v.LH ; 
  alignptr = mem [ p ] .hh .v.RH ; 
  freenode ( p , 5 ) ; 
} 

void getpreambletoken ( ) 
{/* 20 */ getpreambletoken_regmem 
  lab20: gettoken () ; 
  while ( ( curchr == 256 ) && ( curcmd == 4 ) ) {
      
    gettoken () ; 
    if ( curcmd > 100 ) 
    {
      expand () ; 
	  ABORTCHECK;
      gettoken () ; 
    } 
  } 
  if ( curcmd == 9 ) {
	  fatalerror ( 592 ) ;	/* (interwoven alignment preambles are not allowed) */
	  return;			// abortflag set
  }
  if ( ( curcmd == 75 ) && ( curchr == (hash_size + 793) ) ) 
  {
    scanoptionalequals () ; 
    scanglue ( 2 ) ; 
	ABORTCHECK;
    if ( eqtb [ (hash_size + 3206) ] .cint > 0 ) 
		geqdefine ( (hash_size + 793) , 117 , curval ) ; 
    else eqdefine ( (hash_size + 793) , 117 , curval ) ; 
    goto lab20 ; 
  } 
} 

void initalign ( ) 
{/* 30 31 32 22 */ initalign_regmem 
  halfword savecsptr  ; 
  halfword p  ; 
  savecsptr = curcs ; 
  pushalignment () ; 
  alignstate = -1000000L ; 
  if ( ( curlist .modefield == 203 ) && ( ( curlist .tailfield != curlist 
  .headfield ) || ( curlist .auxfield .cint != 0 ) ) ) 
  {
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 677 ) ;		/* Improper */
    } 
    printesc ( 517 ) ;		/* halign */
    print ( 888 ) ;			/* inside $$ */
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 889 ; /* Displays can use special alignments (like \eqalignno) */
      helpline [ 1 ] = 890 ; /* only if nothing but the alignment itself is between $$'s. */
      helpline [ 0 ] = 891 ; /* So I've deleted the formulas that preceded this alignment. */
    } 
    error () ; 
	ABORTCHECK;
    flushmath () ; 
  } 
  pushnest () ; 
  if ( curlist .modefield == 203 ) 
  {
    curlist .modefield = -1 ; 
    curlist .auxfield .cint = nest [ nestptr - 2 ] .auxfield .cint ; 
  } 
  else if ( curlist .modefield > 0 ) 
/*	  long to short ... */
  curlist .modefield = - (integer) curlist .modefield ; 
  scanspec ( 6 , false ) ; 
  mem [ memtop - 8 ] .hh .v.RH = 0 ; 
  curalign = memtop - 8 ; 
  curloop = 0 ; 
  scannerstatus = 4 ; 
  warningindex = savecsptr ; 
  alignstate = -1000000L ; 
  while ( true ) {
    mem [ curalign ] .hh .v.RH = newparamglue ( 11 ) ; 
    curalign = mem [ curalign ] .hh .v.RH ; 
    if ( curcmd == 5 ) 
    goto lab30 ; 
    p = memtop - 4 ; 
    mem [ p ] .hh .v.RH = 0 ; 
    while ( true ) {
      getpreambletoken() ; 
	  ABORTCHECK;
      if ( curcmd == 6 ) 
      goto lab31 ; 
      if ( ( curcmd <= 5 ) && ( curcmd >= 4 ) && ( alignstate == -1000000L ) ) 
      if ( ( p == memtop - 4 ) && ( curloop == 0 ) && ( curcmd == 4 ) ) 
      curloop = curalign ; 
      else {
	  
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 897 ) ;		/* Missing # inserted in alignment preamble */
	} 
	{
	  helpptr = 3 ; 
	  helpline [ 2 ] = 898 ;	/* There should be exactly one # between &'s, when an */
	  helpline [ 1 ] = 899 ;	/* \halign or \valign is being set up. In this case you had */
	  helpline [ 0 ] = 900 ;	/* none, so I've put one in; maybe that will work. */
	} 
	backerror () ; 
	ABORTCHECK;
	goto lab31 ; 
      } 
      else if ( ( curcmd != 10 ) || ( p != memtop - 4 ) ) 
      {
	mem [ p ] .hh .v.RH = getavail () ; 
	p = mem [ p ] .hh .v.RH ; 
	mem [ p ] .hh .v.LH = curtok ; 
      } 
    } 
    lab31: ; 
    mem [ curalign ] .hh .v.RH = newnullbox () ; 
    curalign = mem [ curalign ] .hh .v.RH ; 
    mem [ curalign ] .hh .v.LH = memtop - 9 ; 
    mem [ curalign + 1 ] .cint = -1073741824L ;  /* - 2^30 */
    mem [ curalign + 3 ] .cint = mem [ memtop - 4 ] .hh .v.RH ; 
    p = memtop - 4 ; 
    mem [ p ] .hh .v.RH = 0 ; 
    while ( true ) {
lab22:
		getpreambletoken() ; 
		ABORTCHECK;
      if ( ( curcmd <= 5 ) && ( curcmd >= 4 ) && ( alignstate == -1000000L ) ) 
		  goto lab32 ; 
      if ( curcmd == 6 ) 
      {
	{
	  if ( interaction == 3 ) 
	  ; 
	  printnl ( 262 ) ;		/* !  */
	  print ( 901 ) ;		/* Only one # is allowed per tab */
	} 
	{
	  helpptr = 3 ; 
	  helpline [ 2 ] = 898 ;	/* There should be exactly one # between &'s, when an */
	  helpline [ 1 ] = 899 ;	/* \halign or \valign is being set up. In this case you had */
	  helpline [ 0 ] = 902 ;	/* more than one, so I'm ignoring all but the first. */
	} 
	error () ; 
	ABORTCHECK;
	goto lab22 ; 
      } 
      mem [ p ] .hh .v.RH = getavail () ; 
      p = mem [ p ] .hh .v.RH ; 
      mem [ p ] .hh .v.LH = curtok ; 
    } 
    lab32: mem [ p ] .hh .v.RH = getavail () ; 
    p = mem [ p ] .hh .v.RH ; 
/*    mem [ p ] .hh .v.LH = (hash_size + 4614) ;  */
/*    mem [ p ] .hh .v.LH = (hash_size + 4095 + 519) ;  */
    mem [ p ] .hh .v.LH = (hash_size + hash_extra + 4095 + 519) ; /* 96/Jan/10 */
    mem [ curalign + 2 ] .cint = mem [ memtop - 4 ] .hh .v.RH ; 
  } 
  lab30: scannerstatus = 0 ; 
  newsavelevel ( 6 ) ; 
/* if every_cr<>null then begin_token_list(every_cr,every_cr_text); l.15665 */
  if ( eqtb [ (hash_size + 1320) ] .hh .v.RH != 0 ) /* everycr */
	  begintokenlist ( eqtb [ (hash_size + 1320) ] .hh .v.RH , 13 ) ; 
  alignpeek () ; 
} 

void zinitspan ( p ) 
halfword p ; 
{initspan_regmem 
  pushnest () ; 
  if ( curlist .modefield == -102 ) 
  curlist .auxfield .hh .v.LH = 1000 ; 
  else {
      
    curlist .auxfield .cint = -65536000L ; 
    normalparagraph () ; 
  } 
  curspan = p ; 
} 

void initrow ( ) 
{initrow_regmem 
  pushnest () ; 
  curlist .modefield = ( -103 ) - curlist .modefield ; 
  if ( curlist .modefield == -102 ) 
  curlist .auxfield .hh .v.LH = 0 ; 
  else curlist .auxfield .cint = 0 ; 
  {
    mem [ curlist .tailfield ] .hh .v.RH = newglue ( mem [ mem [ memtop - 8 ] 
    .hh .v.RH + 1 ] .hh .v.LH ) ; 
    curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
  } 
  mem [ curlist .tailfield ] .hh.b1 = 12 ; 
  curalign = mem [ mem [ memtop - 8 ] .hh .v.RH ] .hh .v.RH ; 
  curtail = curhead ; 
  initspan ( curalign ) ; 
} 
void initcol ( ) 
{initcol_regmem 
  mem [ curalign + 5 ] .hh .v.LH = curcmd ; 
  if ( curcmd == 63 ) 
  alignstate = 0 ; 
  else {
      
    backinput () ; 
    begintokenlist ( mem [ curalign + 3 ] .cint , 1 ) ; 
  } 
} 
/* fincol() moved to end to hide in pragma (g, "off") */
/* may need to move more ? everything calling newnullbox() ? */
void finrow ( ) 
{finrow_regmem 
  halfword p  ; 
  if ( curlist .modefield == -102 ) 
  {
    p = hpack ( mem [ curlist .headfield ] .hh .v.RH , 0 , 1 ) ; 
    popnest () ; 
    appendtovlist ( p ) ; 
    if ( curhead != curtail ) 
    {
      mem [ curlist .tailfield ] .hh .v.RH = mem [ curhead ] .hh .v.RH ; 
      curlist .tailfield = curtail ; 
    } 
  } 
  else {
      
    p = vpackage ( mem [ curlist .headfield ] .hh .v.RH , 0 , 1 ,
		1073741823L ) ;   /* 2^30 - 1 */
    popnest () ; 
    mem [ curlist .tailfield ] .hh .v.RH = p ; 
    curlist .tailfield = p ; 
    curlist .auxfield .hh .v.LH = 1000 ; 
  } 
  mem [ p ] .hh.b0 = 13 ; 
  mem [ p + 6 ] .cint = 0 ; 
  if ( eqtb [ (hash_size + 1320) ] .hh .v.RH != 0 ) /* everycr */
	  begintokenlist ( eqtb [ (hash_size + 1320) ] .hh .v.RH , 13 ) ; 
  alignpeek () ; 
} 

void finalign ( ) 
{finalign_regmem 
  halfword p, q, r, s, u, v  ; 
  scaled t, w  ; 
  scaled o  ; 
  halfword n  ; 
  scaled rulesave  ; 
  memoryword auxsave  ; 

  ABORTCHECK;
  
  if ( curgroup != 6 ) {
	  confusion ( 909 ) ;	/* align1 */
	  return;				// abortflag set
  }
  unsave () ; 
  if ( curgroup != 6 ) {
	  confusion ( 910 ) ;	/* align0 */
	  return;				// abortflag set
  }
  unsave () ; 
  if ( nest [ nestptr - 1 ] .modefield == 203 ) 
  o = eqtb [ (hash_size + 3745) ] .cint ; 
  else o = 0 ; 
  q = mem [ mem [ memtop - 8 ] .hh .v.RH ] .hh .v.RH ; 
  do {
      flushlist ( mem [ q + 3 ] .cint ) ; 
    flushlist ( mem [ q + 2 ] .cint ) ; 
    p = mem [ mem [ q ] .hh .v.RH ] .hh .v.RH ; 
    if ( mem [ q + 1 ] .cint == -1073741824L )  /* - 2^30 */
    {
      mem [ q + 1 ] .cint = 0 ; 
      r = mem [ q ] .hh .v.RH ; 
      s = mem [ r + 1 ] .hh .v.LH ; 
      if ( s != 0 ) 
      {
	incr ( mem [ 0 ] .hh .v.RH ) ;	/* mem [ membot ] ? mem [ null ] ? */
	deleteglueref ( s ) ; 
	mem [ r + 1 ] .hh .v.LH = 0 ; 
      } 
    } 
    if ( mem [ q ] .hh .v.LH != memtop - 9 ) 
    {
      t = mem [ q + 1 ] .cint + mem [ mem [ mem [ q ] .hh .v.RH + 1 ] .hh 
      .v.LH + 1 ] .cint ; 
      r = mem [ q ] .hh .v.LH ; 
      s = memtop - 9 ; 
      mem [ s ] .hh .v.LH = p ; 
      n = 1 ; 
      do {
	  mem [ r + 1 ] .cint = mem [ r + 1 ] .cint - t ; 
	u = mem [ r ] .hh .v.LH ; 
	while ( mem [ r ] .hh .v.RH > n ) {
	    
	  s = mem [ s ] .hh .v.LH ; 
	  n = mem [ mem [ s ] .hh .v.LH ] .hh .v.RH + 1 ; 
	} 
	if ( mem [ r ] .hh .v.RH < n ) 
	{
	  mem [ r ] .hh .v.LH = mem [ s ] .hh .v.LH ; 
	  mem [ s ] .hh .v.LH = r ; 
	  decr ( mem [ r ] .hh .v.RH ) ; 
	  s = r ; 
	} 
	else {
	    
	  if ( mem [ r + 1 ] .cint > mem [ mem [ s ] .hh .v.LH + 1 ] .cint ) 
	  mem [ mem [ s ] .hh .v.LH + 1 ] .cint = mem [ r + 1 ] .cint ; 
	  freenode ( r , 2 ) ; 
	} 
	r = u ; 
      } while ( ! ( r == memtop - 9 ) ) ; 
    } 
    mem [ q ] .hh.b0 = 13 ; 
    mem [ q ] .hh.b1 = 0 ; 
    mem [ q + 3 ] .cint = 0 ; 
    mem [ q + 2 ] .cint = 0 ; 
    mem [ q + 5 ] .hh.b1 = 0 ; 
    mem [ q + 5 ] .hh.b0 = 0 ; 
    mem [ q + 6 ] .cint = 0 ; 
    mem [ q + 4 ] .cint = 0 ; 
    q = p ; 
  } while ( ! ( q == 0 ) ) ; 
  saveptr = saveptr - 2 ; 
  packbeginline = - (integer) curlist .mlfield ; 
  if ( curlist .modefield == -1 ) 
  {
    rulesave = eqtb [ (hash_size + 3746) ] .cint ; 
    eqtb [ (hash_size + 3746) ] .cint = 0 ; 
    p = hpack ( mem [ memtop - 8 ] .hh .v.RH , savestack [ saveptr + 1 ] .cint 
    , savestack [ saveptr + 0 ] .cint ) ; 
    eqtb [ (hash_size + 3746) ] .cint = rulesave ; 
  } 
  else {
      
    q = mem [ mem [ memtop - 8 ] .hh .v.RH ] .hh .v.RH ; 
    do {
	mem [ q + 3 ] .cint = mem [ q + 1 ] .cint ; 
      mem [ q + 1 ] .cint = 0 ; 
      q = mem [ mem [ q ] .hh .v.RH ] .hh .v.RH ; 
    } while ( ! ( q == 0 ) ) ; 
    p = vpackage ( mem [ memtop - 8 ] .hh .v.RH , savestack [ saveptr + 1 ] 
    .cint , savestack [ saveptr + 0 ] .cint , 1073741823L ) ;  /* 2^30 - 1 */
    q = mem [ mem [ memtop - 8 ] .hh .v.RH ] .hh .v.RH ; 
    do {
	mem [ q + 1 ] .cint = mem [ q + 3 ] .cint ; 
      mem [ q + 3 ] .cint = 0 ; 
      q = mem [ mem [ q ] .hh .v.RH ] .hh .v.RH ; 
    } while ( ! ( q == 0 ) ) ; 
  } 
  packbeginline = 0 ; 
  q = mem [ curlist .headfield ] .hh .v.RH ; 
  s = curlist .headfield ; 
  while ( q != 0 ) {		/* while q<>null l.15794 OK */
      
    if ( ! ( q >= himemmin ) )	/*   begin if not is_char_node(q) then */
    if ( mem [ q ] .hh.b0 == 13 ) 
    {
      if ( curlist .modefield == -1 ) 
      {
	mem [ q ] .hh.b0 = 0 ; 
	mem [ q + 1 ] .cint = mem [ p + 1 ] .cint ; 
      } 
      else {
	  
	mem [ q ] .hh.b0 = 1 ; 
	mem [ q + 3 ] .cint = mem [ p + 3 ] .cint ; 
      } 
      mem [ q + 5 ] .hh.b1 = mem [ p + 5 ] .hh.b1 ; 
      mem [ q + 5 ] .hh.b0 = mem [ p + 5 ] .hh.b0 ; 
      mem [ q + 6 ] .gr = mem [ p + 6 ] .gr ; 
      mem [ q + 4 ] .cint = o ; 
      r = mem [ mem [ q + 5 ] .hh .v.RH ] .hh .v.RH ; 
      s = mem [ mem [ p + 5 ] .hh .v.RH ] .hh .v.RH ; 
      do {
	  n = mem [ r ] .hh.b1 ; 
	t = mem [ s + 1 ] .cint ; 
	w = t ; 
	u = memtop - 4 ; 
	while ( n > 0 ) {
	    
	  decr ( n ) ; 
	  s = mem [ s ] .hh .v.RH ; 
	  v = mem [ s + 1 ] .hh .v.LH ; 
	  mem [ u ] .hh .v.RH = newglue ( v ) ; 
	  u = mem [ u ] .hh .v.RH ; 
	  mem [ u ] .hh.b1 = 12 ; 
	  t = t + mem [ v + 1 ] .cint ; 
	  if ( mem [ p + 5 ] .hh.b0 == 1 ) 
	  {
	    if ( mem [ v ] .hh.b0 == mem [ p + 5 ] .hh.b1 ) 
	    t = t + round ( mem [ p + 6 ] .gr * mem [ v + 2 ] .cint ) ; 
	  } 
	  else if ( mem [ p + 5 ] .hh.b0 == 2 ) 
	  {
	    if ( mem [ v ] .hh.b1 == mem [ p + 5 ] .hh.b1 ) 
	    t = t - round ( mem [ p + 6 ] .gr * mem [ v + 3 ] .cint ) ; 
	  } 
	  s = mem [ s ] .hh .v.RH ; 
	  mem [ u ] .hh .v.RH = newnullbox () ; 
	  u = mem [ u ] .hh .v.RH ; 
	  t = t + mem [ s + 1 ] .cint ; 
	  if ( curlist .modefield == -1 ) 
	  mem [ u + 1 ] .cint = mem [ s + 1 ] .cint ; 
	  else {
	      
	    mem [ u ] .hh.b0 = 1 ; 
	    mem [ u + 3 ] .cint = mem [ s + 1 ] .cint ; 
	  } 
	} 
	if ( curlist .modefield == -1 ) 
	{
	  mem [ r + 3 ] .cint = mem [ q + 3 ] .cint ; 
	  mem [ r + 2 ] .cint = mem [ q + 2 ] .cint ; 
	  if ( t == mem [ r + 1 ] .cint ) 
	  {
	    mem [ r + 5 ] .hh.b0 = 0 ; 
	    mem [ r + 5 ] .hh.b1 = 0 ; 
	    mem [ r + 6 ] .gr = 0.0 ; 
	  } 
	  else if ( t > mem [ r + 1 ] .cint ) 
	  {
	    mem [ r + 5 ] .hh.b0 = 1 ; 
	    if ( mem [ r + 6 ] .cint == 0 ) 
	    mem [ r + 6 ] .gr = 0.0 ; 
	    else mem [ r + 6 ] .gr = ( t - mem [ r + 1 ] .cint ) / ((double) 
	    mem [ r + 6 ] .cint ) ; 
	  } 
	  else {
	      
	    mem [ r + 5 ] .hh.b1 = mem [ r + 5 ] .hh.b0 ; 
	    mem [ r + 5 ] .hh.b0 = 2 ; 
	    if ( mem [ r + 4 ] .cint == 0 ) 
	    mem [ r + 6 ] .gr = 0.0 ; 
	    else if ( ( mem [ r + 5 ] .hh.b1 == 0 ) && ( mem [ r + 1 ] .cint - 
	    t > mem [ r + 4 ] .cint ) ) 
	    mem [ r + 6 ] .gr = 1.0 ; 
	    else mem [ r + 6 ] .gr = ( mem [ r + 1 ] .cint - t ) / ((double) 
	    mem [ r + 4 ] .cint ) ; 
	  } 
	  mem [ r + 1 ] .cint = w ; 
	  mem [ r ] .hh.b0 = 0 ; 
	} 
	else {
	    
	  mem [ r + 1 ] .cint = mem [ q + 1 ] .cint ; 
	  if ( t == mem [ r + 3 ] .cint ) 
	  {
	    mem [ r + 5 ] .hh.b0 = 0 ; 
	    mem [ r + 5 ] .hh.b1 = 0 ; 
	    mem [ r + 6 ] .gr = 0.0 ; 
	  } 
	  else if ( t > mem [ r + 3 ] .cint ) 
	  {
	    mem [ r + 5 ] .hh.b0 = 1 ; 
	    if ( mem [ r + 6 ] .cint == 0 ) 
	    mem [ r + 6 ] .gr = 0.0 ; 
	    else mem [ r + 6 ] .gr = ( t - mem [ r + 3 ] .cint ) / ((double) 
	    mem [ r + 6 ] .cint ) ; 
	  } 
	  else {
	      
	    mem [ r + 5 ] .hh.b1 = mem [ r + 5 ] .hh.b0 ; 
	    mem [ r + 5 ] .hh.b0 = 2 ; 
	    if ( mem [ r + 4 ] .cint == 0 ) 
	    mem [ r + 6 ] .gr = 0.0 ; 
	    else if ( ( mem [ r + 5 ] .hh.b1 == 0 ) && ( mem [ r + 3 ] .cint - 
	    t > mem [ r + 4 ] .cint ) ) 
	    mem [ r + 6 ] .gr = 1.0 ; 
	    else mem [ r + 6 ] .gr = ( mem [ r + 3 ] .cint - t ) / ((double) 
	    mem [ r + 4 ] .cint ) ; 
	  } 
	  mem [ r + 3 ] .cint = w ; 
	  mem [ r ] .hh.b0 = 1 ; 
	} 
	mem [ r + 4 ] .cint = 0 ; 
	if ( u != memtop - 4 ) 
	{
	  mem [ u ] .hh .v.RH = mem [ r ] .hh .v.RH ; 
	  mem [ r ] .hh .v.RH = mem [ memtop - 4 ] .hh .v.RH ; 
	  r = u ; 
	} 
	r = mem [ mem [ r ] .hh .v.RH ] .hh .v.RH ; 
	s = mem [ mem [ s ] .hh .v.RH ] .hh .v.RH ; 
      } while ( ! ( r == 0 ) ) ; 
    } 
    else if ( mem [ q ] .hh.b0 == 2 ) 
    {
      if ( ( mem [ q + 1 ] .cint == -1073741824L ) )  /* 2^30  */
      mem [ q + 1 ] .cint = mem [ p + 1 ] .cint ; 
      if ( ( mem [ q + 3 ] .cint == -1073741824L ) )  /* 2^30  */
      mem [ q + 3 ] .cint = mem [ p + 3 ] .cint ; 
      if ( ( mem [ q + 2 ] .cint == -1073741824L ) )  /* 2^30  */
      mem [ q + 2 ] .cint = mem [ p + 2 ] .cint ; 
      if ( o != 0 ) 
      {
	r = mem [ q ] .hh .v.RH ; 
	mem [ q ] .hh .v.RH = 0 ; 
	q = hpack ( q , 0 , 1 ) ; 
	mem [ q + 4 ] .cint = o ; 
	mem [ q ] .hh .v.RH = r ; 
	mem [ s ] .hh .v.RH = q ; 
      } 
    } 
    s = q ; 
    q = mem [ q ] .hh .v.RH ; 
  } 
  flushnodelist ( p ) ; 
  popalignment () ; 
  auxsave = curlist .auxfield ; 
  p = mem [ curlist .headfield ] .hh .v.RH ; 
  q = curlist .tailfield ; 
  popnest () ; 
  if ( curlist .modefield == 203 ) 
  {
    doassignments () ; 
	ABORTCHECK;
    if ( curcmd != 3 )  {
      {
	if ( interaction == 3 ) 
	; 
	printnl ( 262 ) ;	/* !  */
	print ( 1164 ) ;	/* Missing $$ inserted */
      } 
      {
	helpptr = 2 ; 
	helpline [ 1 ] = 889 ;	/* Displays can use special alignments (like \eqalignno) */
	helpline [ 0 ] = 890 ;	/* only if nothing but the alignment itself is between $$'s. */
      } 
      backerror () ; 
	  ABORTCHECK;
    } 
    else {
	
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
    popnest () ; 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( eqtb [ (hash_size + 3174) ] .cint 
      ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newparamglue ( 3 ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    mem [ curlist .tailfield ] .hh .v.RH = p ; 
    if ( p != 0 ) 
    curlist .tailfield = q ; 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newpenalty ( eqtb [ (hash_size + 3175) ] .cint 
      ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newparamglue ( 4 ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    curlist .auxfield .cint = auxsave .cint ; 
    resumeafterdisplay () ; 
  } 
  else {
      
    curlist .auxfield = auxsave ; 
    mem [ curlist .tailfield ] .hh .v.RH = p ; 
    if ( p != 0 )		/*   if p<>null then tail:=q; l.15926 */
    curlist .tailfield = q ; 
    if ( curlist .modefield == 1 ) {
		buildpage () ;
		ABORTCHECK;
	}
  } 
} 

/* used to be alignpeek, zfintieshrink, etc in old tex5.c */

/************************************************************************/

/* moved down here to avoid questions about pragma optimize */

#pragma optimize("g", off) 					/* for MC VS compiler */

/*  Moved down here 96/Oct/12 in response to problem with texerror.tex */
/*	pragma optimize("a", off) not strong enough - this may slow things */

booleane fincol ( ) 
{/* 10 */ register booleane Result; fincol_regmem 
  halfword p  ; 
  halfword q, r  ; 
  halfword s  ; 
  halfword u  ; 
  scaled w  ; 
  glueord o  ; 
  halfword n  ; 
  if ( curalign == 0 ) {
	  confusion ( 903 ) ;	/* endv */
	  return 0;				// abortflag set
  }
  q = mem [ curalign ] .hh .v.RH ; 
  if ( q == 0 ) {
	  confusion ( 903 ) ;	/* endv */
	  return 0;				// abortflag set
  }
  if ( alignstate < 500000L )	{			/* ??? */
	  fatalerror ( 592 ) ;	/* (interwoven alignment preambles are not allowed) */
	  return 0;			// abortflag set
  }
  p = mem [ q ] .hh .v.RH ;					/* p <- link(q) p.791 */
/* if (p = null) ^ (extra_info(cur_align) < cr_code) then p.792 */
  if ( ( p == 0 ) && ( mem [ curalign + 5 ] .hh .v.LH < 257 ) ) 
  if ( curloop != 0 ) 
  {
/*	potential problem here if newnullbox causes memory reallocation ??? */
/*	compiler optimization does not refresh `mem' loaded in registers ? */
    mem [ q ] .hh .v.RH = newnullbox () ; 
    p = mem [ q ] .hh .v.RH ; 
    mem [ p ] .hh .v.LH = memtop - 9 ; 
    mem [ p + 1 ] .cint = -1073741824L ;  /* - 2^30 */
    curloop = mem [ curloop ] .hh .v.RH ; 
    q = memtop - 4 ; 
    r = mem [ curloop + 3 ] .cint ; 
    while ( r != 0 ) {
	
      mem [ q ] .hh .v.RH = getavail () ; 
      q = mem [ q ] .hh .v.RH ; 
      mem [ q ] .hh .v.LH = mem [ r ] .hh .v.LH ; 
      r = mem [ r ] .hh .v.RH ; 
    } 
    mem [ q ] .hh .v.RH = 0 ; 
    mem [ p + 3 ] .cint = mem [ memtop - 4 ] .hh .v.RH ; 
    q = memtop - 4 ; 
    r = mem [ curloop + 2 ] .cint ; 
    while ( r != 0 ) {
	
      mem [ q ] .hh .v.RH = getavail () ; 
      q = mem [ q ] .hh .v.RH ; 
      mem [ q ] .hh .v.LH = mem [ r ] .hh .v.LH ; 
      r = mem [ r ] .hh .v.RH ; 
    } 
    mem [ q ] .hh .v.RH = 0 ; 
    mem [ p + 2 ] .cint = mem [ memtop - 4 ] .hh .v.RH ; 
    curloop = mem [ curloop ] .hh .v.RH ; 
    mem [ p ] .hh .v.RH = newglue ( mem [ curloop + 1 ] .hh .v.LH ) ; 
  } 
  else {
      
    {
      if ( interaction == 3 ) 
      ; 
      printnl ( 262 ) ;		/* !  */
      print ( 904 ) ;		/* Extra alignment tab has been changed to  */
    } 
    printesc ( 893 ) ;		/* cr */
    {
      helpptr = 3 ; 
      helpline [ 2 ] = 905 ;	/* You have given more \span or & marks than there were */
      helpline [ 1 ] = 906 ;	/* in the preamble to the \halign or \valign now in progress. */
      helpline [ 0 ] = 907 ;	/* So I'll assume that you meant to type \cr instead. */
    } 
/* extra_info(cur_align) < cr_code) ? */
    mem [ curalign + 5 ] .hh .v.LH = 257 ; 
    error () ; 
	ABORTCHECKZERO;
  } 
  if ( mem [ curalign + 5 ] .hh .v.LH != 256 ) 
  {
    unsave () ; 
    newsavelevel ( 6 ) ; 
    {
      if ( curlist .modefield == -102 ) 
      {
	adjusttail = curtail ; 
	u = hpack ( mem [ curlist .headfield ] .hh .v.RH , 0 , 1 ) ; 
	w = mem [ u + 1 ] .cint ; 
	curtail = adjusttail ; 
	adjusttail = 0 ; 
      } 
      else {
	  
	u = vpackage ( mem [ curlist .headfield ] .hh .v.RH , 0 , 1 , 0 ) ; 
	w = mem [ u + 3 ] .cint ; 
      } 
      n = 0 ; 
      if ( curspan != curalign ) 
      {
	q = curspan ; 
	do {
	    incr ( n ) ; 
	  q = mem [ mem [ q ] .hh .v.RH ] .hh .v.RH ; 
	} while ( ! ( q == curalign ) ) ; 
/*	if n > max_quarterword then confusion("256 spans"); p.798 */
/*	if ( n > 255 )  */						/* 94/Apr/4 ? */
	if ( n > maxquarterword) {				/* 96/Oct/12 ??? */
		confusion ( 908 ) ;		/* 256 spans --- message wrong now, but ... */
		return 0;				// abortflag set
	}
	q = curspan ; 
	while ( mem [ mem [ q ] .hh .v.LH ] .hh .v.RH < n ) q = mem [ q ] .hh 
	.v.LH ; 
	if ( mem [ mem [ q ] .hh .v.LH ] .hh .v.RH > n ) 
	{
	  s = getnode ( 2 ) ; 
	  mem [ s ] .hh .v.LH = mem [ q ] .hh .v.LH ; 
	  mem [ s ] .hh .v.RH = n ; 
	  mem [ q ] .hh .v.LH = s ; 
	  mem [ s + 1 ] .cint = w ; 
	} 
	else if ( mem [ mem [ q ] .hh .v.LH + 1 ] .cint < w ) 
	mem [ mem [ q ] .hh .v.LH + 1 ] .cint = w ; 
      } 
      else if ( w > mem [ curalign + 1 ] .cint ) 
      mem [ curalign + 1 ] .cint = w ; 
      mem [ u ] .hh.b0 = 13 ; 
      mem [ u ] .hh.b1 = n ; 
      if ( totalstretch [ 3 ] != 0 ) 
      o = 3 ; 
      else if ( totalstretch [ 2 ] != 0 ) 
      o = 2 ; 
      else if ( totalstretch [ 1 ] != 0 ) 
      o = 1 ; 
      else o = 0 ; 
      mem [ u + 5 ] .hh.b1 = o ; 
      mem [ u + 6 ] .cint = totalstretch [ o ] ; 
      if ( totalshrink [ 3 ] != 0 ) 
      o = 3 ; 
      else if ( totalshrink [ 2 ] != 0 ) 
      o = 2 ; 
      else if ( totalshrink [ 1 ] != 0 ) 
      o = 1 ; 
      else o = 0 ; 
      mem [ u + 5 ] .hh.b0 = o ; 
      mem [ u + 4 ] .cint = totalshrink [ o ] ; 
      popnest () ; 
      mem [ curlist .tailfield ] .hh .v.RH = u ; 
      curlist .tailfield = u ; 
    } 
    {
      mem [ curlist .tailfield ] .hh .v.RH = newglue ( mem [ mem [ curalign ] 
      .hh .v.RH + 1 ] .hh .v.LH ) ; 
      curlist .tailfield = mem [ curlist .tailfield ] .hh .v.RH ; 
    } 
    mem [ curlist .tailfield ] .hh.b1 = 12 ; 
/* if (extra_info(cur_align) >= cr_code) then p.792 */
    if ( mem [ curalign + 5 ] .hh .v.LH >= 257 ) 
    {
      Result = true ; 
      return(Result) ; 
    } 
    initspan ( p ) ; 
  } 
  alignstate = 1000000L ; 
  do {
      getxtoken () ; 
	  ABORTCHECKZERO;
  } while ( ! ( curcmd != 10 ) ) ; 
  curalign = p ; 
  initcol () ; 
  Result = false ; 
  return Result ; 
} 

/* #pragma optimize ("g", on) */				/* for MC VS compiler */
/* #pragma optimize ("g",) */					/* 94/Jan/25 */
/* #pragma optimize ("", on) */						/* 96/Sep/12 */

/* #pragma optimize("g", off) */					/* for MC VS compiler */

scaled zmakeop ( q ) 
halfword q ; 
{register scaled Result; makeop_regmem 
  scaled delta  ; 
  halfword p, v, x, y, z  ; 
  quarterword c  ; 
  ffourquarters i  ; 
  scaled shiftup, shiftdown  ; 
  if ( ( mem [ q ] .hh.b1 == 0 ) && ( curstyle < 2 ) ) 
  mem [ q ] .hh.b1 = 1 ; 
  if ( mem [ q + 1 ] .hh .v.RH == 1 ) 
  {
    fetch ( q + 1 ) ; 
    if ( ( curstyle < 2 ) && ( ( ( curi .b2 ) % 4 ) == 2 ) ) 
    {
      c = curi .b3 ; 
      i = fontinfo [ charbase [ curf ] + c ] .qqqq ; 
      if ( ( i .b0 > 0 ) ) 
      {
	curc = c ; 
	curi = i ; 
	mem [ q + 1 ] .hh.b1 = c ; 
      } 
    } 
    delta = fontinfo [ italicbase [ curf ] + ( curi .b2 ) / 4 ] .cint ; 
    x = cleanbox ( q + 1 , curstyle ) ; 
    if ( ( mem [ q + 3 ] .hh .v.RH != 0 ) && ( mem [ q ] .hh.b1 != 1 ) ) 
    mem [ x + 1 ] .cint = mem [ x + 1 ] .cint - delta ; 
    mem [ x + 4 ] .cint = half ( mem [ x + 3 ] .cint - mem [ x + 2 ] .cint ) - 
    fontinfo [ 22 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint ; 
    mem [ q + 1 ] .hh .v.RH = 2 ; 
    mem [ q + 1 ] .hh .v.LH = x ; 
  } 
  else delta = 0 ; 
  if ( mem [ q ] .hh.b1 == 1 ) 
  {
    x = cleanbox ( q + 2 , 2 * ( curstyle / 4 ) + 4 + ( curstyle % 2 ) ) ; 
    y = cleanbox ( q + 1 , curstyle ) ; 
    z = cleanbox ( q + 3 , 2 * ( curstyle / 4 ) + 5 ) ; 
    v = newnullbox () ; 
    mem [ v ] .hh.b0 = 1 ; 
    mem [ v + 1 ] .cint = mem [ y + 1 ] .cint ; 
    if ( mem [ x + 1 ] .cint > mem [ v + 1 ] .cint ) 
    mem [ v + 1 ] .cint = mem [ x + 1 ] .cint ; 
    if ( mem [ z + 1 ] .cint > mem [ v + 1 ] .cint ) 
    mem [ v + 1 ] .cint = mem [ z + 1 ] .cint ; 
    x = rebox ( x , mem [ v + 1 ] .cint ) ; 
    y = rebox ( y , mem [ v + 1 ] .cint ) ; 
    z = rebox ( z , mem [ v + 1 ] .cint ) ; 
    mem [ x + 4 ] .cint = half ( delta ) ; 
    mem [ z + 4 ] .cint = - (integer) mem [ x + 4 ] .cint ; 
    mem [ v + 3 ] .cint = mem [ y + 3 ] .cint ; 
    mem [ v + 2 ] .cint = mem [ y + 2 ] .cint ; 
    if ( mem [ q + 2 ] .hh .v.RH == 0 ) 
    {
      freenode ( x , 7 ) ; 
      mem [ v + 5 ] .hh .v.RH = y ; 
    } 
    else {
	
      shiftup = fontinfo [ 11 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH 
      ] ] .cint - mem [ x + 2 ] .cint ; 
      if ( shiftup < fontinfo [ 9 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh 
      .v.RH ] ] .cint ) 
      shiftup = fontinfo [ 9 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH 
      ] ] .cint ; 
      p = newkern ( shiftup ) ; 
      mem [ p ] .hh .v.RH = y ; 
      mem [ x ] .hh .v.RH = p ; 
      p = newkern ( fontinfo [ 13 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh 
      .v.RH ] ] .cint ) ; 
      mem [ p ] .hh .v.RH = x ; 
      mem [ v + 5 ] .hh .v.RH = p ; 
      mem [ v + 3 ] .cint = mem [ v + 3 ] .cint + fontinfo [ 13 + parambase [ 
      eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint + mem [ x + 3 ] .cint + mem 
      [ x + 2 ] .cint + shiftup ; 
    } 
    if ( mem [ q + 3 ] .hh .v.RH == 0 ) 
    freenode ( z , 7 ) ; 
    else {
	
      shiftdown = fontinfo [ 12 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh 
      .v.RH ] ] .cint - mem [ z + 3 ] .cint ; 
      if ( shiftdown < fontinfo [ 10 + parambase [ eqtb [ (hash_size + 1838) + cursize ] 
      .hh .v.RH ] ] .cint ) 
      shiftdown = fontinfo [ 10 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh 
      .v.RH ] ] .cint ; 
      p = newkern ( shiftdown ) ; 
      mem [ y ] .hh .v.RH = p ; 
      mem [ p ] .hh .v.RH = z ; 
      p = newkern ( fontinfo [ 13 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh 
      .v.RH ] ] .cint ) ; 
      mem [ z ] .hh .v.RH = p ; 
      mem [ v + 2 ] .cint = mem [ v + 2 ] .cint + fontinfo [ 13 + parambase [ 
      eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH ] ] .cint + mem [ z + 3 ] .cint + mem 
      [ z + 2 ] .cint + shiftdown ; 
    } 
    mem [ q + 1 ] .cint = v ; 
  } 
  Result = delta ; 
  return Result ; 
} 

/* #pragma optimize ("g", on) */				/* for MC VS compiler */
/* #pragma optimize ("g",) */					/* 94/Jan/25 */
/* #pragma optimize ("", on) */						/* 96/Sep/12 */

/* #pragma optimize ("g", off) */

void zmakescripts ( q , delta ) 
halfword q ; 
scaled delta ; 
{makescripts_regmem 
  halfword p, x, y, z  ; 
  scaled shiftup, shiftdown, clr  ; 
  smallnumber t  ; 
  p = mem [ q + 1 ] .cint ; 
  if ( ( p >= himemmin ) ) 
  {
    shiftup = 0 ; 
    shiftdown = 0 ; 
  } 
  else {
      
    z = hpack ( p , 0 , 1 ) ; 
    if ( curstyle < 4 ) 
    t = 16 ; 
    else t = 32 ; 
    shiftup = mem [ z + 3 ] .cint - fontinfo [ 18 + parambase [ eqtb [ (hash_size + 1837) + 
    t ] .hh .v.RH ] ] .cint ; 
    shiftdown = mem [ z + 2 ] .cint + fontinfo [ 19 + parambase [ eqtb [ (hash_size + 1837) 
    + t ] .hh .v.RH ] ] .cint ; 
    freenode ( z , 7 ) ; 
  } 
  if ( mem [ q + 2 ] .hh .v.RH == 0 ) 
  {
    x = cleanbox ( q + 3 , 2 * ( curstyle / 4 ) + 5 ) ; 
    mem [ x + 1 ] .cint = mem [ x + 1 ] .cint + eqtb [ (hash_size + 3742) ] .cint ; 
    if ( shiftdown < fontinfo [ 16 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh 
    .v.RH ] ] .cint ) 
    shiftdown = fontinfo [ 16 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH 
    ] ] .cint ; 
    clr = mem [ x + 3 ] .cint - ( abs ( fontinfo [ 5 + parambase [ eqtb [ 
    (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint * 4 ) / 5 ) ; 
    if ( shiftdown < clr ) 
    shiftdown = clr ; 
    mem [ x + 4 ] .cint = shiftdown ; 
  } 
  else {
      
    {
      x = cleanbox ( q + 2 , 2 * ( curstyle / 4 ) + 4 + ( curstyle % 2 ) ) ; 
      mem [ x + 1 ] .cint = mem [ x + 1 ] .cint + eqtb [ (hash_size + 3742) ] .cint ; 
      if ( odd ( curstyle ) ) 
      clr = fontinfo [ 15 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] 
      .cint ; 
      else if ( curstyle < 2 ) 
      clr = fontinfo [ 13 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] 
      .cint ; 
      else clr = fontinfo [ 14 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh 
      .v.RH ] ] .cint ; 
      if ( shiftup < clr ) 
      shiftup = clr ; 
      clr = mem [ x + 2 ] .cint + ( abs ( fontinfo [ 5 +
		parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh .v.RH ] ] .cint ) / 4 ) ; 
      if ( shiftup < clr ) 
      shiftup = clr ; 
    } 
    if ( mem [ q + 3 ] .hh .v.RH == 0 ) 
    mem [ x + 4 ] .cint = - (integer) shiftup ; 
    else {
	
      y = cleanbox ( q + 3 , 2 * ( curstyle / 4 ) + 5 ) ; 
      mem [ y + 1 ] .cint = mem [ y + 1 ] .cint + eqtb [ (hash_size + 3742) ] .cint ; 
      if ( shiftdown < fontinfo [ 17 + parambase [ eqtb [ (hash_size + 1837) + cursize ] 
      .hh .v.RH ] ] .cint ) 
      shiftdown = fontinfo [ 17 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh 
      .v.RH ] ] .cint ; 
      clr = 4 * fontinfo [ 8 + parambase [ eqtb [ (hash_size + 1838) + cursize ] .hh .v.RH 
      ] ] .cint - ( ( shiftup - mem [ x + 2 ] .cint ) - ( mem [ y + 3 ] .cint 
      - shiftdown ) ) ; 
      if ( clr > 0 ) 
      {
	shiftdown = shiftdown + clr ; 
	clr = ( abs ( fontinfo [ 5 + parambase [ eqtb [ (hash_size + 1837) + cursize ] .hh 
	.v.RH ] ] .cint * 4 ) / 5 ) - ( shiftup - mem [ x + 2 ] .cint ) ; 
	if ( clr > 0 ) 
	{
	  shiftup = shiftup + clr ; 
	  shiftdown = shiftdown - clr ; 
	} 
      } 
      mem [ x + 4 ] .cint = delta ; 
      p = newkern ( ( shiftup - mem [ x + 2 ] .cint ) - ( mem [ y + 3 ] .cint 
      - shiftdown ) ) ; 
      mem [ x ] .hh .v.RH = p ; 
      mem [ p ] .hh .v.RH = y ; 
      x = vpackage ( x , 0 , 1 , 1073741823L ) ;  /* 2^30 - 1 */
      mem [ x + 4 ] .cint = shiftdown ; 
    } 
  } 
  if ( mem [ q + 1 ] .cint == 0 ) 
  mem [ q + 1 ] .cint = x ; 
  else {
      
    p = mem [ q + 1 ] .cint ; 
    while ( mem [ p ] .hh .v.RH != 0 ) p = mem [ p ] .hh .v.RH ; 
    mem [ p ] .hh .v.RH = x ; 
  } 
} 

/* #pragma optimize ("g", on) */				/* 96/Sep/12 */
/* #pragma optimize ("g") */					/* 94/Jan/25 */
#pragma optimize ("", on)						/* 96/Sep/12 */

/***************************************************************************/