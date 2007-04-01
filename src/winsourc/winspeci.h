/* Copyright 1991,1992 Y&Y, Inc.
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

/* This is mostly stuff for our own TIFF reading code in winspeci.c */

/* The magic TIFF `version' number */

#define TIFF_VERSION 42

/* TIFF file value type codes: */

#define TYPE_BYTE 1
#define TYPE_ASCII 2
#define TYPE_SHORT 3
#define TYPE_LONG 4
#define TYPE_RATIO 5

/* following added in TIFF 6.0 */

#define TYPE_SBYTE 6		/* An 8-bit signed (twos-complement) integer */
#define TYPE_UNDEFINED 7	/* An 8-bit byte that may contain anything */
#define TYPE_SSHORT 8		/* A 16-bit (2-byte) signed (twos-complement) integer */
#define TYPE_SLONG 9		/* A 32-bit (4-byte) signed (twos-complement) integer */
#define TYPE_SRATIONAL 10	/* Two SLONG’s: numerator and denominator */
#define TYPE_FLOAT 11		/* Single precision (4-byte) IEEE format */
#define TYPE_DOUBLE 12		/* Double precision (8-byte) IEEE format */

/* TIFF file tags */

#define	NEWSUBFILETYPE		254
#define	SUBFILETYPE			255
#define	IMAGEWIDTH			256
#define	IMAGELENGTH			257
#define	BITSPERSAMPLE		258
#define	COMPRESSION			259
#define	PHOTOMETRICINTERPRETATION	262
#define	THRESHHOLDING		263
#define	CELLWIDTH			264
#define	CELLLENGTH			265
#define	FILLORDER			266
#define	DOCUMENTNAME		269
#define	IMAGEDESCRIPTION	270
#define	MAKE				271
#define	MODEL				272
#define	STRIPOFFSETS		273
#define	ORIENTATION			274
#define	SAMPLESPERPIXEL		277
#define	ROWSPERSTRIP		278
#define	STRIPBYTECOUNTS		279
#define	MINSAMPLEVALUE		280
#define	MAXSAMPLEVALUE		281
#define	XRESOLUTION			282
#define	YRESOLUTION			283
#define	PLANARCONFIG		284
#define	PAGENAME			285
#define	XPOSITION			286
#define	YPOSITION			287
#define	FREEOFFSETS			288
#define	FREEBYTECOUNTS		289
#define	GRAYRESPONSEUNIT	290
#define	GRAYRESPONSECURVE	291
#define	GROUP3OPTIONS		292
#define	GROUP4OPTIONS		293
#define	RESOLUTIONUNIT		296
#define	PAGENUMBER			297
#define	COLORRESPONSECURVES	301
#define	SOFTWARE			305
#define	DATETIME			306
#define	ARTIST				315
#define	HOSTCOMPUTER		316
#define	PREDICTOR			317
#define	WHITE				318
#define	COLORLIST			319
#define	PRIMARYCHROMATICITIES	319
#define	COLORMAP			320

/* following added in TIFF 6.0 */

#define HALFTONEHINTS		321
#define TILEWIDTH			322
#define TILELENGTH			323
#define TILEOFFSETS			324
#define TILEBYTECOUNTS		325
#define INKSETS				332
#define INKNAMES			333
#define NUMBEROFINKS		334
#define DOTRANGE			336
#define TARGETPRINTER		337
#define EXTRASAMPLES		338
#define SMINSAMPLEVALUE		340
#define SMAXSAMPLEVALUE		341
#define TRANSFERRANGE		342

#define JPEGPROC			512
#define JPEGINTERCHANGEFORMAT	513
#define JPEGINTERCHANGEFORMATLNGTH	514
#define JPEGRESTARTINTERVAL	515
#define JPEGLOSSLESSPREDICTORS	517
#define JPEGPOINTTRANSFORMS	518
#define JPEGQTABLES			519
#define JPEGDCTABLES		520
#define JPEGACTABLES		521
#define YCBCRCOEFFICIENTS	529
#define YCBCRSUBSAMPLING	530
#define YCBCRPOSITIONING	531
#define REFERENCEBLACK		532

#define COPYRIGHT			33432

#define NOCOMPRESSION     1
#define TIFF_CCITT        2
#define CCITT_GROUP3      3
#define CCITT_GROUP4      4
#define LZW_COMPRESSION   5
#define JPEG_COMPRESSION  6
#define PACK_BITS         32773

