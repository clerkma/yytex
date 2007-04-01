/**********************************************************************
   Common header information for DVIWINDO
   Copyright 1991,1992 Y&Y, Inc.
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

#define UNUSED(p) (void)(p)

/* #define FNAMELEN 128 */		/* 80 maximum file name length WINEXTRA */
#define FNAMELEN FILENAME_MAX

/* #define MAXLINE 256 */
#define MAXLINE 512		/* maximum length of input line afm tfm eps */

/* #define MAXTEXNAME 13 */	/* maximum length of font name in  TeX */
/* #define MAXTEXNAME (32+1) *//* 16 maximum length of font name in  TeX ??? */
// #define MAXTEXNAME 32			/* modified 95/Dec/25 to speed computing */
/* #define MAXFONTNAME (32+6+1) */	/* maximum length of substitute fontname */
/* #define MAXFONTNAME (32+8) */	/* need extra space for " (TT)" */
#define MAXFACENAME (32+8)		/* need extra space for " (TT)" */
/* must be at least  LF_FACESIZE  32 */
#define MAXFULLNAME	(64)
/* must be at least  LF_FULLFACESIZE  64 */

/* #define MAXTFMNAME 8 */		/* max DOS file name minus extension ??? */
#define MAXTFMNAME 32			/* max TFM file name minus extension ??? */
								/* used only in TTMapping table ... */
								/* Allow for some stupid PS FontName use ! */

/* #define LF_FACESIZE	       32 *//* LOGFONT structure <- EnumFont */
/* #define LF_FULLFACESIZE     64 *//* ENUMLOGFONT structure <- EnumFontFam */

/* Local memory used  2 * MAXFONTNUMBER bytes */
/* Local memory used 37 * MAXFONTS bytes */

#define MAXFONTS 512		/* 99/Nov/3 Larry Tseng */
/* #define MAXFONTS 256 */		/* 128 >= 64 max number of fonts in DVI file */
#define MAXFONTNUMBERS 1024	/* >= 256 maximum number assigned to a font */

/* #define MAXFONTS 128 */
/* #define MAXFONTNUMBERS 512 */

#define MAXDVISTACK 256		/* stack for h, v, x, y, z in winanal.c */
							/* a bit large, for that one test file	... */

// #define BLANKFONT 255		/* 93/Dec/11 */
#define BLANKFONT -1		/* 99/Nov/5 */

#define STACKINC 8			/* increment in stack on push */

#define MAXMARKS 32			/* maximum size of label in hyptertext */

#define MAXHYPERPUSH 16		/* maximum size of push down table */
#define MAXCOLORSTACK 16	/* maximum size of color push down table */
#define MAXGRAPHICSTACK 16	/* maximum depth of graphics stack */

/* #define MAXHYPERPUSH 8 */
/* #define MAXCOLORSTACK 8 */

/* #define TEXCHRS 128 */	/* maximum number of characters in a CM font */
/* #define MAXVECNAME 9 */	/* maximum length of encoding vector name */
/* #define MAXSUBSTITUTE 128 */	/* max fonts in substitution table */
/* #define MAXPAGES 1024 */	/* maximum number of pages in page table */

#define ID_BYTE 2			/* for version of DVI files that we understand */

#define MAXERRORS 8			/* error count before giving up */

/* following used in Print Setup */

#define MAXPRINTERNAME 48	/* 32 "Apple LaserWriter II NT" say */
							/* same as CCHDEVICENAME in print.h which is 32 */
#define MAXDRIVERNAME 32	/* 80 max printer driver name PSCRIPT say */
#define MAXPORTNAME 64		/* 32 max port name (was 20) COM1: say */
							/* or may be file name ??? */

/* #define MAXKEYBUFFER 512 */	/* 300 max buffer for key in getting `devices' */

#define MAXKEYBUFFER 256	/* initial buffer for key in getting `devices' */
							/* reduced 97/June/5 because it gets grown now */

#define SIZETEMPBUFF 80		/* used in printer setup */

/* typedef WORD FAR PASCAL FNOLDDEVMODE(HWND, HANDLE, LPSTR, LPSTR); */
/* typedef FNOLDDEVMODE FAR * LPFNOLDDEVMODE; */

#define SUBDIRSEARCH 1		/* 1994/Aug/18 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define RESIDENT "*reside*"		/* font resident in printer */
#define FORCESUB "*force*"		/* force substitution */
#define REMAPIT "*remap*"		/* remap to specified encoding */
#define ALIASED "*alias*"		/* just another name for same thing */

#define CRESIDENT 1				/* code for RESIDENT font */
#define CFORCESUB 2				/* code for FORCESUB font */
#define CREMAPIT 4				/* code for REMAPIT font */
#define CALIASED 8				/* code for ALIASED font */
#define CMISSING 16				/* code for missing font */
#define CUNUSED 32				/* code for unused font */

/* #define SINFINITY 32767 */		/* 2^{15} - 1 */
/* #define LINFINITY 2147483647 */	/* 2^{31} - 1 */

/* DVI one byte commands: */

enum dvicom {
/* set_char_0 = 0, set_char_1, set_char_2, */
set1 = 128, set2, set3, set4,
set_rule = 132, 
put1 = 133, put2, put3, put4,
put_rule = 137,
nop = 138, bop, eop, push, pop,
right1 = 143, right2, right3, right4,
w0 = 147, w1, w2, w3, w4,
x0 = 152, x1, x2, x3, x4,
down1 = 157, down2, down3, down4,
/* y0 = 161, y1, y2, y3, y4, */
y5 = 161, y6, y2, y3, y4,
z0 = 166, z1, z2, z3, z4,
/* fnt_num_0 = 171, font_num_1, font_num_2, font_num_3, */
fnt1 = 235, fnt2, fnt3, fnt4,
xxx1 = 239, xxx2, xxx3, xxx4,
fnt_def1 = 243, fnt_def2, fnt_def3, fnt_def4,
pre = 247, post, post_post
};

/* srefl = 250, erefl = 251 used for `right-to-left' languages in TeX-XeT */
/* need for these was later removed in TeX--XeT */

#define MAXMESSAGE 256 /* max length of error message */

/* #define DVITYPEINC 20 */	/* ignore movement this small (in TWIPS) */
#define DVITYPEINC 0	/* ignore movement this small (in TWIPS) */

#define MAGICFACT 10324			/* 1.0324 scale factor on ATM font size */

#define METRICSIZE 1000			/* size at which to create metric font */

#define BUFFERLEN 512			/* good size buffer for WINGETC  */

/*** code for working way into back end of file looking for post ***/

#define BUFSIZE 128		/* buffer size to read in at one time */
#define NUMSTEPS 32		/* number of buffers to try from end of file */
#define MAGIC 223		/* magic code used by TeX at end of DVI file */
#define MINMAGIC 4		/* official minimum of magic codes required */

/* constraint in winpslog.c */

#if BUFSIZE > BUFFERLEN
#error ERROR: BUFSIZE > BUFFERLEN
#endif

/* #define MAXMSFONTNAME 32	*/	/* maximum MS Windows font name */

#define MAXMSEXTRA 12			/* extra space for BOLD, ITALIC */

#define MAXKEY 64				/* max length of key in TeX Menu */

/* *** *** *** *** *** *** *** *** *** spin button window class stuff: */

/* Spin Button's class-specific window styles. */
#define SPNS_WRAP          0x0001L 

/* Spin Button's class-specific window messages. */
#define SPNM_SETRANGE      (WM_USER + 0) 
#define SPNM_GETRANGE      (WM_USER + 1) 
#define SPNM_SETCRNTVALUE  (WM_USER + 2)
#define SPNM_GETCRNTVALUE  (WM_USER + 3)

/* Spin Button's notification codes sent in HIWORD of lParam  */
/* during a WM_COMMAND message. */
#define SPNN_VALUECHANGE   (1)

/* need to modify the number of Window extra bytes in WIN32 ? */

#define CBWNDEXTRA			(12)
#define GWL_RANGE           (0)
#define GWL_CRNTVALUE       (4)
#define GWL_TRIANGLEDOWN    (8)

/* #define CBWNDEXTRA			(8)	*/
/* #define GWL_RANGE           (0) */
/* #define GWW_CRNTVALUE       (4) */
/* #define GWW_TRIANGLEDOWN    (6) */

/* need to modify these offsets in WIN32 ? */

#define SPNM_SCROLLVALUE      (WM_USER + 500)

/* Time delay between scrolling events in milliseconds. was 150 */
#define TIME_DELAY            (50)				
/* Time delay before first scrolling event in milliseconds. was - 150 */
#define EXTRA_DELAY           (200) 

/* how often to check for mouse clicks */

#define HOWOFTEN 512	/* how often to check for mouse clicks - 2^n */

#define TWIPLIM 32000	/* to try and prevent overflow problems */
