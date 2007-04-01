/*
 * Copyright (C) 1992, 1993, Metis Technology, Inc.
 *
 * @(#)mac2bdf.c	1.2	3/20/93
 *
 * STANDARD DISCLAIMERS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the Metis General Public License as published
 * by Metis Technology, Inc.; either version 2, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Metis Library General Public License for more details.
 *
 * In order to receive a copy of the Metis General Public License,
 * write to Metis Technology, Inc., 358 Windsor St, Cambridge, MA 02141, USA.
 * This license may also be retrieved by anonymous FTP from the file
 * "pub/PublicLicense" on the host METIS.COM [140.186.33.40].
 *
 * PROGRAM DESCRIPTION
 *
 * Name:     MAC2BDF
 * Summary:  Utility for extracting bitmap fonts in Macintosh files,
 *           and converting these fonts to Adobe Bitmap Distribution
 *	     Format (BDF) files.  The resulting BDF files can then
 *           be converted into X11 Font Formats, e.g., by using bdftosnf.
 *           Both FONT and NFNT resources are extracted.  FOND resources
 *	     are used to select the appropriate font and font file name;
 *	     however, FONT and NFNT resources which are orphaned (i.e.,
 *	     have no governing FOND) are extracted to temporary font
 *	     names.
 * Usage:    mac2bdf [-n] [-q] [-v] file
 * Options:  -n    Don't do anything, just report what would be done.
 *	     -q    Don't report dumped fonts.
 *	     -v    Enable verbose reporting.
 * Input:    A Macintosh file in MacBinary format.
 * Output:   Zero or more Adobe BDF files, one for each font resource.  The
 *	     names chosen for output font files are generated based on the
 *	     font family, style, and size.  Existing files with the same names
 *	     are silently replaced.  If you wish to know which files will be
 *	     produced, use the [-n] option prior to doing the real conversion.
 * Comments: The Mac font format does not have all of the information one
 *	     would normally place in a BDF file; e.g., glyph names are not
 *           specified in the Mac font resources.  Consequently, the names
 *           given to glyphs in the BDF file are dynamically assigned in a
 *           unique manner.
 * History:  11/04/92 -- Created
 *	     11/27/92 -- Modified to find font resources from FONDs.
 *	     11/27/92 -- Modified to load one font resource at a time.
 *	     12/08/92 -- Fixed bug with locTab & owTab indexing; this
 *	              -- corrected the Ajmer core dump problem.  Change
 *		      -- font name handling to substitute hyphens for
 *		      -- whitespace.  Handle overflows of OWTLoc offset
 *		      -- field for large bitmaps.
 * Notes:    1. Orphaned font resources are not yet handled.
 *	     2. Prefixes B,I,BI,Sb,SbI need to be removed.
 *	     3. Add option to specify family, size, style to dump.
 * Author:   Glenn Adams <glenn@metis.com>
 *
 * WARNINGS
 *
 * 1. This program was quickly hacked together to get its job done quickly
 *    with little error checking or view towards portability.
 * 2. This program has only been compiled and run on SunOS 4.1.1. It may not
 *    compile on other platforms, let alone even run.
 * 3. The only input this program accepts is a MacBinary format file which
 *    was created by using NCSA Telnet to FTP standard Mac files (containing
 *    both data and resource forks) to a Sun system. It may also work with
 *    MacBinary files as transferred by other terminal emulator programs;
 *    however, this has not been tested.
 * 4. This program is being released in this form in the hope that it will
 *    be useful to someone without all the frills one would expect from a
 *    robust program; this program does not make any claims to robustness.
 */

/* #include <alloca.h> */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define alloca malloc	/* ??? */

#define  DEVXRES	72	/* Macintosh X-Resolution */
#define  DEVYRES	72	/* Macintosh Y-Resolution */

typedef	char	INT8;
typedef	short	INT16;
typedef	long	INT32;

typedef	unsigned char	CARD8;
typedef	unsigned short	CARD16;
typedef	unsigned long	CARD32;

typedef struct _MacBinHdrRec MacBinHdrRec, *MacBinHdr;
struct _MacBinHdrRec {
	CARD8	fnLen		[   2 ];
	CARD8	fnName      [  63 ];
	CARD8	fnType      [   4 ];
	CARD8	fnCreator   [   4 ];
	CARD8	fnFlags     [   2 ];
	CARD8	pad1        [   2 ];
	CARD8	pad2        [   2 ];
	CARD8	pad3        [   4 ];
	CARD8	fnDataLen   [   4 ];
	CARD8   pad4        [  41 ];
};

#define RHDRLEN	256	/* total header length */

typedef struct _RsrcHdrRec RsrcHdrRec, *RsrcHdr;
struct _RsrcHdrRec {
	CARD8	rhDataOffset [   4 ];
	CARD8	rhMapOffset  [   4 ];
	CARD8	rhDataLen    [   4 ];
	CARD8	rhMapLen     [   4 ];
};

typedef struct _RsrcMapRec RsrcMapRec, *RsrcMap;
struct _RsrcMapRec {
	RsrcHdrRec	rmHdrCopy;
	CARD8	rmNextMap    [   4 ];
	CARD8	rmFileRef    [   2 ];
	CARD8	rmFileAttr   [   2 ];
	CARD8	rmTypeOffset [   2 ];
	CARD8	rmNameOffset [   2 ];
};

typedef struct _RsrcTypeRec RsrcTypeRec, *RsrcType;
struct _RsrcTypeRec {
	CARD8	rtName       [   4 ];
	CARD8	rtCount      [   2 ];
	CARD8	rtRefOffset  [   2 ];
};

typedef struct _RsrcRefRec RsrcRefRec, *RsrcRef;
struct _RsrcRefRec {
	CARD8	rrIdent      [   2 ];
	CARD8	rrNameOffset [   2 ];
	CARD8	rrAttr       [   1 ];
	CARD8	rrDataOffset [   3 ];
	CARD8	rrReserved   [   4 ];
};

typedef struct _FontRsrcRec FontRsrcRec, *FontRsrc;
struct _FontRsrcRec {
	CARD8	ftFontType   [   2 ];
	CARD8	ftFirstChar  [   2 ];
	CARD8	ftLastChar   [   2 ];
	CARD8	ftWidMax     [   2 ];
	CARD8	ftKernMax    [   2 ];
	CARD8	ftNDescent   [   2 ];
	CARD8	ftFRectWidth [   2 ];
	CARD8	ftFRectHeight[   2 ];
	CARD8	ftOWTLoc     [   2 ];
	CARD8	ftAscent     [   2 ];
	CARD8	ftDescent    [   2 ];
	CARD8	ftLeading    [   2 ];
	CARD8	ftRowWords   [   2 ];
};

typedef struct _FondRsrcRec FondRsrcRec, *FondRsrc;
struct _FondRsrcRec {
	CARD8	fdFlags		[   2 ];
	CARD8	fdFamID		[   2 ];
	CARD8	fdFirst		[   2 ];
	CARD8	fdLast		[   2 ];
	CARD8	fdAscent    [   2 ];
	CARD8	fdDescent   [   2 ];
	CARD8	fdLeading   [   2 ];
	CARD8	fdWidMax    [   2 ];
	CARD8	fdWTabOff   [   4 ];
	CARD8	fdKernOff   [   4 ];
	CARD8	fdStylOff   [   4 ];
	CARD8	fdProperty  [  18 ];
	CARD8	fdIntl		[   4 ];
	CARD8	fdVersion   [   2 ];
};

typedef struct _FontNameRec FontNameRec, *FontName;
struct _FontNameRec {
	char *name;
	int	resource_id;
	int	size;
	int	style;
	FontName	next;
};

char *progname;
int	nodump;
int	quiet;
int	verbose;
FontName	fontnames;

CARD16  toushort (CARD8 *p) {
	return (CARD16) ( ( p[0] << 8 ) | p [ 1 ] );
}

INT16  toshort (CARD8 *p) {
	return (INT16) toushort ( p );
}

CARD32  toulong (CARD8 *p) {
	return (CARD32) ( ( p[0] << 24 ) | ( p[1] << 16 ) | ( p[2] << 8 ) | p[3] );
}

INT32  tolong (CARD8 *p) {
	return (INT32) toulong ( p );
}

char *strdup(char *s) {	/* ??? */
	int n = strlen(s);
	char *t = malloc(n+1);
	return t;	
}

void  FontShow (FontRsrc fp) {
	(void) printf ( "  Type:        0x%04x\n",  toushort ( fp->ftFontType    ) );
	(void) printf ( "  First Glyph: 0x%02x\n",  toushort ( fp->ftFirstChar   ) );
	(void) printf ( "  Last Glyph:  0x%02x\n",  toushort ( fp->ftLastChar    ) );
	(void) printf ( "  Max Width:   %d\n",      toshort  ( fp->ftWidMax      ) );
	(void) printf ( "  Max Kern:    %d\n",      toshort  ( fp->ftKernMax     ) );
	(void) printf ( "  Neg Descent: %d\n",      toshort  ( fp->ftNDescent    ) );
	(void) printf ( "  Rect Width:  %d\n",      toshort  ( fp->ftFRectWidth  ) );
	(void) printf ( "  Rect Height: %d\n",      toshort  ( fp->ftFRectHeight ) );
	(void) printf ( "  OWT Offset:  %d\n",      toushort ( fp->ftOWTLoc      ) );
	(void) printf ( "  Ascent:      %d\n",      toshort  ( fp->ftAscent      ) );
	(void) printf ( "  Descent:     %d\n",      toshort  ( fp->ftDescent     ) );
	(void) printf ( "  Leading:     %d\n",      toshort  ( fp->ftLeading     ) );
	(void) printf ( "  Row Words:   %d\n",      toshort  ( fp->ftRowWords    ) );
}

/*
 * Find font bounding box and total number of glyphs.
 */
void  FontInfo (FontRsrc fp, INT16 *ret_top, INT16 *ret_left, 
					INT16 *ret_bottom, INT16 *ret_right, INT16 *ret_ng ) {
	register int i, j, bit;
	register CARD16 *bp;
	register CARD8  *gp;
	CARD16   g, fg, lg, coff0, coff1, ow, wo;
	INT16    mk, wd, ht, rw, xoff, top, bot, left, right, ng;
	CARD8 *  bitImage;
	CARD8 *  locTable;
	CARD8 *  owTable;

	top = bot = left = right = 0;

	fg = toushort ( fp->ftFirstChar   );
	lg = toushort ( fp->ftLastChar    );
	mk = toshort  ( fp->ftKernMax     );
	wd = toshort  ( fp->ftFRectWidth  );
	ht = toshort  ( fp->ftFRectHeight );
	wo = toushort ( fp->ftOWTLoc      );
	rw = toshort  ( fp->ftRowWords    );

	bitImage = (CARD8 * ) & fp [ 1 ];
	locTable = & bitImage     [ ( rw * ht ) << 1     ];
#ifdef notdef
	owTable  = & fp->ftOWTLoc [ wo          << 1     ];
#else
	owTable  = & locTable     [ ( lg - fg + 3 ) << 1 ];
#endif

	if ( lg == fg )
		return;

	if ( (CARD32) bitImage & 0x1 ) {	/* near pointer => long int ? */
		bp = (CARD16 *) alloca ( ht * rw * 2 );
		(void) memcpy ( (char *) bp, (char *) bitImage, ht * rw * 2 );
	} else
		bp = (CARD16 *) bitImage;

	gp = (CARD8 *) alloca ( wd * ht );
	(void) memset ( (char *) gp, 0, wd * ht );

	for ( g = fg, ng = 0; g <= lg; g++ ) {

		/*
		 * Get starting and ending offset columns in bit image.
		 */
		coff0 = toushort ( & locTable [ ( ( g - fg ) + 0 ) << 1 ] );
		coff1 = toushort ( & locTable [ ( ( g - fg ) + 1 ) << 1 ] );
		if ( coff0 == coff1 )
			continue;

		/*
		 * Get basepoint offset and escapement.
		 */
		ow    = toushort ( & owTable  [ ( g - fg ) << 1 ] );
		xoff  = ( ( ow >> 8 ) & 0xff ) + mk;

		/*
		 * Overlay glyph image.
		 */
		for ( i = 0; i < ht; i++ ) {
			for ( j = coff0; j < coff1; j++ ) {	/* signed/unsigned ? */
	if ( (int) ( ( j - coff0 ) + xoff ) < 0 )
		continue;
	gp [ i * wd + ( j - coff0 ) + xoff ] |=
		( bp [ i * rw + ( j / 16 ) ] >> ( 15 - ( j % 16 ) ) ) & 1;
			}
		}
		ng++;
	}

	/*
	 * Find top and bottom of bounding box.
	 */
	top = ht;
	bot = 0;
	for ( i = 0; i < ht; i++ ) {
		for ( j = 0; j < wd; j++ ) {
			bit = gp [ i * wd + j ];
			if ( bit && ( i < top ) )
	top = i;
			if ( bit && ( i > bot ) )
	bot = i;
		}
	}

	/*
	 * Find left and right of bounding box.
	 */
	left  = wd;
	right = 0;
	for ( i = 0; i < ht; i++ ) {
		for ( j = 0; j < wd; j++ ) {
			bit = gp [ i * wd + j ];
			if ( bit && ( j < left ) )
	left  = j;
			if ( bit && ( j > right ) )
	right = j;
		}
	}

#ifdef notdef
	if ( ( ( right - left ) + 1 ) != wd )
		(void) fprintf ( stderr,
				"%s: warning: bbox width mismatch, got %d, expected %d\n",
				progname, ( right - left ) + 1, wd );
	if ( ( ( bot - top ) + 1 ) != ht )
		(void) fprintf ( stderr,
				"%s: warning: bbox height mismatch, got %d, expected %d\n",
				progname, ( bot - top ) + 1, ht );
#endif

	*ret_top    = top;
	*ret_left   = left + mk;
	*ret_bottom = bot;
	*ret_right  = right + mk;
	*ret_ng     = ng;
}

char *FontStyleName (int style) {
	static char sname [ 128 ];

	sname [ 0 ] = '\0';
	if ( style & 0001 )
		(void) strcat ( sname, "Bold" );
	if ( style & 0002 )
		(void) strcat ( sname, "Italic" );
	if ( style & 0004 )
		(void) strcat ( sname, "Underlined" );
	if ( style & 0010 )
		(void) strcat ( sname, "Outlined" );
	if ( style & 0020 )
		(void) strcat ( sname, "Shadowed" );
	if ( style & 0040 )
		(void) strcat ( sname, "Condensed" );
	if ( style & 0100 )
		(void) strcat ( sname, "Extended" );
	return sname;
}

int  FontDump (FontRsrc fp, char *name, int style, int size) {
	register int i, j, bit, bits;
	register CARD16 *bp;
	register CARD8  *gp;
	CARD16	g, fg, lg, coff0, coff1, ow, wo;
	INT16	mk, wd, ht, rw, top, bot, left, right, ng;
	CARD8	*bitImage;
	CARD8	*locTable;
	CARD8	*owTable;
	FILE	*fout;
	char 	fname [ 128 ];

	if ( ! fp || ! name || ! size )
		return 1;

	fg = toushort ( fp->ftFirstChar   );
	lg = toushort ( fp->ftLastChar    );
	mk = toshort  ( fp->ftKernMax     );
	wd = toshort  ( fp->ftFRectWidth  );
	ht = toshort  ( fp->ftFRectHeight );
	wo = toushort ( fp->ftOWTLoc      );
	rw = toshort  ( fp->ftRowWords    );

	bitImage = (CARD8 *) & fp [ 1 ];
	locTable = & bitImage     [ ( rw * ht ) << 1 ];
#ifdef notdef
	owTable  = & fp->ftOWTLoc [ wo          << 1     ];
#else
	owTable  = & locTable     [ ( lg - fg + 3 ) << 1 ];
#endif

	/*
	 * If no glyphs are present, don't dump anything.
	 */
	if ( lg == fg )
		return 1;

	/*
	 * At least one glyph is present; create BDF file.
	 */
	(void) sprintf ( fname, "%s%s-%d.bdf", name, FontStyleName ( style ), size );
	if ( ! ( fout = fopen ( fname, "w+" ) ) ) {
		(void) fprintf ( stderr, "%s: can't create output file \"%s\"\n",
				progname, fname );
		return 0;
	}

	/*
	 * Obtain per-font information and dump BDF font header.
	 */
	FontInfo ( fp, & top, & left, & bot, & right, & ng );

	if ( ! quiet )
		(void) printf  ( "Dumping %d glyphs to \"%s%s-%d.bdf\"\n",
				ng, name, FontStyleName ( style ), size );

	(void) fprintf ( fout, "STARTFONT 2.1\n" );
	(void) fprintf ( fout, "FONT %s%s-%d\n",
			name, FontStyleName ( style ), size );
	(void) fprintf ( fout, "SIZE %d %d %d\n", size, DEVXRES, DEVYRES );
	(void) fprintf ( fout, "FONTBOUNDINGBOX %d %d %d %d\n",
			( right - left ) + 1,
			( bot - top ) + 1,
			mk,
			( ht - toshort ( fp->ftDescent ) ) - ( bot + 1 ) );
	(void) fprintf ( fout, "STARTPROPERTIES 2\n" );
	(void) fprintf ( fout, "FONT_ASCENT %d\n", toshort  ( fp->ftAscent ) );
	(void) fprintf ( fout, "FONT_DESCENT %d\n", toshort  ( fp->ftDescent ) );
	(void) fprintf ( fout, "ENDPROPERTIES\n" );
	(void) fprintf ( fout, "CHARS %d\n", ng );

	if ( (CARD32) bitImage & 0x1 ) {	/* near pointer => long int ? */
		bp = (CARD16 *) alloca ( ht * rw * 2 );
		(void) memcpy ( (char *) bp, (char *) bitImage, ht * rw * 2 );
	} else
		bp = (CARD16 *) bitImage;

	gp = (CARD8 *) alloca ( wd * ht );
	(void) memset ( (char *) gp, 0, wd * ht );

	for ( g = fg; g <= lg; g++ ) {

		/*
		 * Get starting and ending offset columns in bit image.
		 */
		coff0 = toushort ( & locTable [ ( ( g - fg ) + 0 ) << 1 ] );
		coff1 = toushort ( & locTable [ ( ( g - fg ) + 1 ) << 1 ] );
		if ( coff0 == coff1 )
			continue;

		/*
		 * Get basepoint offset and escapement.
		 */
		ow    = toushort ( & owTable  [ ( g - fg ) << 1 ] );

		/*
		 * Extract glyph image.
		 */
		(void) memset ( (char *) gp, 0, wd * ht );
		for ( i = 0; i < ht; i++ ) {
			for ( j = coff0; j < coff1; j++ ) { 	/* signed/unsigned ? */
				bit = ( bp [ i * rw + ( j / 16 ) ] >> ( 15 - ( j % 16 ) ) ) & 1;
	gp [ i * wd + ( j - coff0 ) ] = bit;
			}
		}
		
		/*
		 * Find top and bottom of bounding box.
		 */
		top = ht;
		bot = 0;
		for ( i = 0; i < ht; i++ ) {
			for ( j = 0; j < wd; j++ ) {
	bit = gp [ i * wd + j ];
	if ( bit && ( i < top ) )
		top = i;
	if ( bit && ( i > bot ) )
		bot = i;
			}
		}
		
		(void) fprintf ( fout, "STARTCHAR GCID%02X\n", g );
		(void) fprintf ( fout, "ENCODING %d\n", g );
		(void) fprintf ( fout, "SWIDTH %d %d\n", ( ow & 0xff ) * 720, 0 );
		(void) fprintf ( fout, "DWIDTH %d %d\n", ( ow & 0xff ), 0 );
		(void) fprintf ( fout, "BBX %d %d %d %d\n",
			( coff1 - coff0 ),
			( bot - top ) + 1,
			( ( ow >> 8 ) & 0xff ) + mk,
			( ht - toshort ( fp->ftDescent ) ) - ( bot + 1 ) );
		(void) fprintf ( fout, "BITMAP\n" );
		for ( i = top; i <= bot; i++ ) {
			for ( j = 0, bits = 0; j < ( coff1 - coff0 ); j++, bits <<= 1 ) {
													/* signed/unsigned ? */
	bits |= gp [ i * wd + j ];
	if ( ( j & 7 ) == 7 ) {
		(void) fprintf ( fout, "%02x", bits );
		bits = 0;
	}
			}
			bits <<= 7 - ( j % 8 );
			if ( j & 7 )
	(void) fprintf ( fout, "%02x\n", bits );
			else
	(void) fprintf ( fout, "\n" );
		}
		(void) fprintf ( fout, "ENDCHAR\n" );
	}
	(void) fprintf ( fout, "ENDFONT\n" );
	(void) fclose ( fout );
	return 1;
}

void FondShow (FondRsrc fp) {
	CARD8 *assocTab;
	int n, nf;

	(void) printf ( "  Flags:       0x%04x\n",  toushort ( fp->fdFlags   ) );
	(void) printf ( "  Family Id:   %d\n",      toshort  ( fp->fdFamID   ) );
	(void) printf ( "  First Char:  0x%02x\n",  toushort ( fp->fdFirst   ) );
	(void) printf ( "  Last Char:   0x%02x\n",  toushort ( fp->fdLast    ) );
	(void) printf ( "  WT Offset:   0x%08x\n",  toulong  ( fp->fdWTabOff ) );
	(void) printf ( "  KT Offset:   0x%08x\n",  toulong  ( fp->fdKernOff ) );
	(void) printf ( "  ST Offset:   0x%08x\n",  toulong  ( fp->fdStylOff ) );
	(void) printf ( "  Version:     %d\n",      toushort ( fp->fdVersion ) );

	assocTab  = (CARD8 *) & fp [ 1 ];
	nf        = toshort ( assocTab ) + 1;
	(void) printf ( "  Fonts:       %d\n",        nf );
	assocTab += sizeof (CARD16);
	for ( n = 0; n < nf; n++ ) {
		register CARD8 *ap = & assocTab [ n * 6 ];
		(void) printf ( "  Font %d\n", n );
		(void) printf ( "    Size:      %d\n",     toushort ( & ap [ 0 ] ) );
		(void) printf ( "    Style:     0x%04x\n", toushort ( & ap [ 2 ] ) );
		(void) printf ( "    RSC ID:    %d\n",     toshort  ( & ap [ 4 ] ) );
	}
}

int FondFontCount (FondRsrc fp) {
	return toshort ( (CARD8 *) & fp [ 1 ] ) + 1;
}

void FondFontInfo (FondRsrc fp, int nf, 
				INT16 *sizes, INT16 *styles, INT16 *ids) {
	register CARD8 *assocTab  = (CARD8 *) & fp [ 1 ];
	register int n;

	assocTab += sizeof (CARD16);
	for ( n = 0; n < nf; n++ ) {
		register CARD8 *ap = & assocTab [ n * 6 ];
		sizes  [ n ] = toshort ( & ap [ 0 ] );
		styles [ n ] = toshort ( & ap [ 2 ] );
		ids    [ n ] = toshort ( & ap [ 4 ] );
	}
}

void AddFontToFontNames (FontName fn) {
	register FontName *fnp;

	for ( fnp = & fontnames; *fnp; fnp = & (*fnp)->next )
		continue;
	*fnp = fn;
}

void FondAddFonts (FondRsrc fp, char *fondname, int fondnamelen) {
	register int nf, num_fonts;
	register FontName fn;
	register char *cp;
	INT16 *sizes;
	INT16 *styles;
	INT16 *identifiers;
	char   namebuf [ 132 ];

	if ( ! ( num_fonts = FondFontCount ( fp ) ) )
		return;

	sizes       = (INT16 *) alloca ( num_fonts * sizeof (INT16) );
	styles      = (INT16 *) alloca ( num_fonts * sizeof (INT16) );
	identifiers = (INT16 *) alloca ( num_fonts * sizeof (INT16) );
	FondFontInfo ( fp, num_fonts, sizes, styles, identifiers );
	for ( nf = 0; nf < num_fonts; nf++ ) {
		if ( ! sizes [ nf ] )
			continue;
		if ( ! ( fn = (FontName) malloc ( sizeof (*fn) ) ) ) {
			(void) fprintf ( stderr, "%s: out of memory: fond add fonts\n",
					progname );
			return;
		}
		(void) strncpy ( namebuf, fondname, fondnamelen );
		namebuf [ fondnamelen ] = '\0';
		fn->name        = strdup ( namebuf );
		for ( cp = fn->name; *cp; cp++ )
			if ( isspace ( *cp) )
	*cp = '-';
		fn->size        = sizes       [ nf ];
		fn->style       = styles      [ nf ];
		fn->resource_id = identifiers [ nf ];
		fn->next        = (FontName) NULL;
		AddFontToFontNames ( fn );
	}
}

CARD8 *_LoadResource (FILE *rf, CARD32 doff, CARD32 rdoff, int *ret_length) {
	INT32	rdlen;
	CARD8	*bp;
	CARD8 	buf [ 4 ];
	
	if ( fseek ( rf, doff + rdoff, 0 ) < 0 ) {
		(void) fprintf ( stderr, "%s: resource seek error, offset %ld\n",
				progname, doff + rdoff );
		return (CARD8 *) NULL;
	}

	if ( fread ( (char *) buf, sizeof (buf), 1, rf ) != 1 ) {
		(void) fprintf ( stderr, "%s: resource length read error\n", progname );
		return (CARD8 *) NULL;
	}
	rdlen = tolong ( buf );

	if ( ! ( bp = (CARD8 *) malloc ( rdlen * sizeof (CARD8) ) ) ) { 
						/* size mismatch - rdlen is long */
		(void) fprintf ( stderr, "%s: memory request failed, %d bytes\n",
				progname );
		return (CARD8 *) NULL;
	}
	
	if ( fread ( (char *) bp, rdlen, 1, rf ) != 1 ) {
						/* size mismatch - rdlen is long */
		(void) fprintf ( stderr, "%s: resource read error\n", progname );
		return (CARD8 *) NULL;
	}

	if ( ret_length )
		*ret_length = (int) rdlen;
	return bp;
}

CARD8 *LoadResource (FILE *rf, CARD32 doff, CARD8 *rmap, 
			char *type, int id, int *ret_length) {
	register RsrcType	tp, etp;
	register RsrcRef	rp, erp;
	CARD32		typeoff;
	CARD8  		buf [ 4 ];

	typeoff = toushort ( ( (RsrcMap) rmap ) -> rmTypeOffset );
	tp  = (RsrcType) & rmap [ typeoff + 2 ];
	etp = & tp [ toushort ( & rmap [ typeoff ] ) + 1 ];
	for ( ; tp < etp; tp++ ) {
		if ( memcmp ( (char *) tp->rtName, (char *) type, 4 ) != 0 )
			continue;
		rp  = (RsrcRef) & rmap [ typeoff + toushort ( tp->rtRefOffset ) ];
		erp = & rp [ toushort ( tp->rtCount ) + 1 ];
		for ( ; rp < erp; rp++ ) {
			if ( (int) toshort ( rp->rrIdent ) != id )
	continue;
			(void) memcpy ( (char *) buf, (char *) rp->rrAttr, sizeof (buf) );
			buf [ 0 ] = 0;
			return _LoadResource ( rf, doff, toulong ( buf ), ret_length );
		}
	}
	return (CARD8 *) NULL;
}

/*ARGSUSED*/
void FreeResource (CARD8 *rp, int length) {		/* length not referenced ? */
	if ( rp )
		(void) free ( (char *) rp );
}

/* int */
void DumpFonts (FILE *rf, CARD32 doff, CARD8 *rmap) {
	register FontName fn;
	FontRsrc fp;
	int length;

	for ( fn = fontnames; fn; fn = fn->next ) {
		fp = (FontRsrc)
			LoadResource ( rf, doff, rmap, "NFNT", fn->resource_id, & length );
		if ( ! fp ) {
			fp = (FontRsrc)
	LoadResource ( rf, doff, rmap, "FONT", fn->resource_id, & length );
			if ( ! fp ) {
	(void) fprintf ( stderr, "%s: can't find font, name \"%s\", id %d\n",
			progname, fn->name, fn->resource_id );
	continue;
			}
		}
		if ( verbose ) {
			(void) printf (
				"Font \"%s\", Style %s, Size %d, Resource Id %d, Length %d\n",
				fn->name,
				FontStyleName ( fn->style ),
				fn->size,
				fn->resource_id,
				length );
			FontShow ( fp );
		}
		if ( nodump ) {
			(void) printf  ( "Would dump \"%s%s-%d.bdf\"\n",
					fn->name, FontStyleName ( fn->style ), fn->size );
			continue;
		}
		if ( ! FontDump ( fp, fn->name, fn->style, fn->size ) ) {
			(void) fprintf ( stderr, "%s: warning: dump failed, font \"%s%s-%d\"\n",
					fn->name, FontStyleName ( fn->style ), fn->size );
		}
		FreeResource ( (CARD8 *) fp, length );
	}
}

/*ARGSUSED*/
int main (int argc, char *argv[]) {
	FILE *		rf		= (FILE *) NULL;
	CARD32		datalen;
	CARD32		dataoff;
	CARD32		rmaplen;
	CARD32		typeoff;
	CARD32		nameoff;
	int			namelen;
	char *		name;
	MacBinHdrRec		mbrec;
	RsrcHdrRec		rhrec;
	register RsrcType	tp, etp;
	register RsrcRef	rp, erp;
	CARD8 *		rmap;

	progname = argv [ 0 ];

	while ( --argc && *++argv && ( argv[0][0] == '-' ) ) {
		switch ( argv[0][1] ) {
		case 'n':
			nodump  = 1;
			break;
		case 'q':
			quiet   = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			break;
		}
	}

	if ( ! argc ) {
		(void) fprintf ( stderr,
				"usage: %s [-n] [-v] infile\n", progname );
		return 1;
	}

	if ( ! ( rf = fopen ( argv [ 0 ], "r" ) ) ) {
		(void) fprintf ( stderr, "%s: can't open file \"%s\"\n",
				progname, argv [ 0 ] );
		return 1;
	}

	/*
	 * Read MacBinary header
	 */
	if ( fread ( (char *) & mbrec, sizeof (mbrec), 1, rf ) != 1 ) {
		(void) fprintf ( stderr, "%s: can't read MacBinary header\n", progname );
		goto err;
	}

	if ( ! toushort ( mbrec . fnFlags ) )
		return 0;

	/*
	 * Seek to resource fork.
	 */
	datalen = tolong ( mbrec . fnDataLen );
	if ( fseek ( rf, datalen, 1 ) == -1 ) {
		(void) fprintf ( stderr, "%s: can't seek to resource fork\n", progname );
		goto err;
	}

	/*
	 * Read resource header
	 */
	if ( fread ( (char *) & rhrec, sizeof (rhrec), 1, rf ) != 1 ) {
		(void) fprintf ( stderr, "%s: can't read resource header\n", progname );
		goto err;
	}

	/*
	 * Skip system and application data in resource header.
	 */
	if ( fseek ( rf, RHDRLEN - sizeof (rhrec), 1 ) == -1 ) {
		(void) fprintf ( stderr, "%s: can't seek to resource data\n", progname );
		goto err;
	}

	/*
	 * Skip over resource data
	 */
	dataoff = ftell ( rf );
	datalen = tolong ( rhrec . rhDataLen );
	if ( fseek ( rf, datalen, 1 ) < 0 ) {
		(void) fprintf ( stderr, "%s: seek error while skipping data\n",
				progname );
		goto err;
	}

	/*
	 * Read resource map
	 */
	rmaplen = tolong ( rhrec . rhMapLen );
	if ( ! ( rmap = (CARD8 *) malloc ( rmaplen ) ) ) {
						/* size mismatch - rmaplen is long */
		(void) fprintf ( stderr, "%s: out of memory: resource map\n", progname );
		goto err;
	}
	if ( fread ( (char *) rmap, rmaplen, 1, rf ) != 1 ) {
						/* size mismatch - rmaplen is long */
		(void) fprintf ( stderr, "%s: can't read resource map\n", progname );
		goto err;
	}

	/*
	 * Iterate over types and type references.
	 */
	nameoff = toushort ( ( (RsrcMap) rmap ) -> rmNameOffset );
	typeoff = toushort ( ( (RsrcMap) rmap ) -> rmTypeOffset );
	tp  = (RsrcType) & rmap [ typeoff + 2 ];
	etp = & tp [ toushort ( & rmap [ typeoff ] ) + 1 ];
	for ( ; tp < etp; tp++ ) {
		rp  = (RsrcRef) & rmap [ typeoff + toushort ( tp->rtRefOffset ) ];
		erp = & rp [ toushort ( tp->rtCount ) + 1 ];
		for ( ; rp < erp; rp++ ) {
			INT16    nmoff;
			int      rid, rdlen;
			FondRsrc fp;

			nmoff     = toshort ( rp->rrNameOffset );
			namelen   = ( nmoff >= 0 ) ?
				(int) rmap [ nameoff + nmoff ] : 0;
			name      = ( nmoff >= 0 ) ?
				(char *) & rmap [ nameoff + nmoff + 1 ] : "";
			if ( memcmp ( (char *) tp->rtName, "FOND", 4 ) != 0 )
	continue;
			rid = toshort ( rp->rrIdent );
			fp  = (FondRsrc)
	LoadResource ( rf, dataoff, rmap, "FOND", rid, & rdlen );
			if ( ! fp )
	continue;
			if ( verbose ) {
	(void) printf ( "%.4s %6d %6d   %.*s\n",
			(char *) tp->rtName,
			rid,
			rdlen,
			namelen,
			name );
	FondShow ( fp );
			}
			FondAddFonts ( fp, name, namelen );
			FreeResource ( (CARD8 *) fp, rdlen );
		}
	}

	DumpFonts ( rf, dataoff, rmap );
	(void) fclose ( rf );

	return 0;

 err:
	if ( rf )
		(void) fclose ( rf );
	return 1;
}
