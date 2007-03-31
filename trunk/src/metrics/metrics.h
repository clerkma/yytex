/* Header file for c:\metrics\*.c
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

#define EXTRACHAR 4			/* junk at beginning of eexec */
#define LENIV 4				/* required junk at beginning CharString */

#define MAXCHRS 256			/* characters in 8-bit byte character set */

#define MAXADJUST (MAXCHRS+MAXCHRS)	/* max code allowed in SEAC */
									/* adding 256 for non-ASE chars */

#define MAXCHARNAME 32		/* maximum character name length */

/* #define CHARNAME_MAX 80 */		/* extra space for scan in */
#define CHARNAME_MAX 128		/* extra space for scan in */

#define UNKNOWN -32767		/* unknown width */

#define MAXNUMBERS 32		/* max numbers on stack in decrypt */

#define MAXTOKEN 256		/* longest possible PS token 1024 ? */

/* #define MAXLINE 512 */  		/* input line buffer 1024 ? */
#define MAXLINE 1024  		/* input line buffer 1024 ? */

/* #define CHARSTRINGLEN 1024 */	/* max charstring in encrypt output */
/* #define CHARSTRINGLEN 2048 */	/* max charstring in encrypt output */
/* #define CHARSTRINGLEN 4096 */	/* max charstring in encrypt output */
#define CHARSTRINGLEN 8192	/* max charstring in encrypt output */

/* #define MAXCHARINFONT 512 */	/* maximum number of characters in font ? */
/* #define MAXCHARINFONT 2048 */	/* maximum number of characters in font ? */
#define MAXCHARINFONT 16384	/* maximum number of characters in font ? */

/* #define MAXCOMPOSITES 256 */	/* maximum number of composites in font */
/* #define MAXCOMPOSITES 1024 */ /* maximum number of composites in font */

#define NCOMPOSITES 512		/* initial allocation */

#define NCHARACTERS 512		/* initial allocation */

/* #define MAXSUBRS 512 */		/* maximum number of Subrs in font 1024 ? */
/* #define MAXSUBRS 1024 */		/* maximum number of Subrs in font ? */

/* rational approximation constants */

#define NUMLIM 32767		/* maximum numerator in hsbw div arguments */

#ifdef ONEBYTELIM
#define DENLIM 107 			/* one byte encoding limit on denominator */
#else
#define DENLIM 1131			/* two byte encoding limit on denominator */
#endif

/* encryption constants */

#define CRYPT_MUL 52845u	/* pseudo-random number generator constant */
#define CRYPT_ADD 22719u	/* pseudo-random number generator constant */
#define REEXEC 55665 		/* seed constant for eexec encryption */
#define RCHARSTRING 4330 	/* seed constant for charstring encryption */

