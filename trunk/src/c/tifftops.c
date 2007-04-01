/* Copyright 2007 TeX Users Group

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

/* Make PS file from TIFF image file 1, 2, 4, 8 bit pixels only */
/* Support pallette color images => expands to RGB */
/* Support 24 bit color images converts RBG to gray if colorimage not avail */
/* Supports: no compression, TIFF_CCITT, LZW, and PACK_BITS */
/* Does not support BitsPerSample other than power of 2 */
/* Does not support compression schemes 3 and 4 (CCITT) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h> 
/* #include <conio.h> */				/* only for _getch() */

typedef int  BOOL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define TIFF_VERSION 42			/* The magic TIFF `version' number */

#define TYPE_BYTE 1
#define TYPE_ASCII 2
#define TYPE_SHORT 3
#define TYPE_LONG 4
#define TYPE_RATIO 5

/* most of the following not actually used ... */

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
#define	PLANARCONFIGURATION	284
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

/* We'll support compression schemes 1, 2, 5 and 32773 */

#define NOCOMPRESSION     1
#define TIFF_CCITT        2
#define CCITT_GROUP3      3
#define CCITT_GROUP4      4
#define LZW_COMPRESSION   5
#define PACK_BITS         32773

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXFILENAME 128

#define MAXIMAGE 4096		/* maximum rows & columns - sanity check */

int verboseflag=0;

int traceflag=0;

int wantcntrld=1;			/* want control D at end of output */

int centerflag=0;			/* center output image on page */

int pagewidth = 72 * 17 /2;	/* 8.5 inches */

int pageheight = 72 * 11;	/* 11 inches */

int spi = 50;				/* samples per inch in output */

int xll = 72;				/* lower left corner */

int yll = 72;				/* lower left corner */

int xur, yur;

int pwidth, pheight;		/* width of image output */

char *program;				/* pointer to argv[0] */

BOOL bInvertImage=0;		/* should gray levels be inverted ? */

BOOL bAllowColor = 0;		/* allow use of colorimage operator */

BOOL bColorImage = 0;		/* is it a color image ? */

BOOL bExpandColor = 0;	 	/* color palette image - expand to 24 bit */

BOOL bCompressColor = 0; 	/* compress 24 bit color to one byte */

int PaletteSize = 0;		/* number of entries in palette */

unsigned int _far *Palette=NULL; /* pointer to pallette */

/* int lookup[256]; */		/* remapping table for color stretch ? used ? */

/* BOOL bBGRflag=0; */		/* interchange r and b *//* ? */

/* BOOL bStretchColor=1; */	/* expand color range to use dynamic range */

/* unsigned long filelength;  */	/* not used - could be sanity check ? */

/* halftone screen stuff next */

int wantmagic=1;

int frequency=53;

int angle=45;

char *eps_magic = "\
currentscreen pop\n\
{1 add 180 mul cos 1 0.08 add mul exch 2 add 180 mul cos 1 0.08 sub\n\
mul add 2 div} bind setscreen % (c) Y&Y 1989\n\
";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Code imported from DVIWindo winspeci.c */

/* Stuff for reading simple uncompressed TIFF files (& PackBits compressed) */

unsigned int TIFFVersion;			/* TIFF version number */

unsigned int LeastFirst=1;			/* least significant first */

unsigned int IFDCount;				/* number of items image file directory */

unsigned long IFDPosition;			/* position of image file directory */

static unsigned int ureadtwo(FILE *input) {
	unsigned int c, d;
	c = (unsigned int) getc(input);
	d = (unsigned int) getc(input);	
/*	if (c == EOF || d == EOF) return 0; */
	if (LeastFirst != 0) return ((d << 8) | c);
	else return ((c << 8) | d);
}

static unsigned long ureadfour(FILE *input) {
	unsigned long c, d, e, f;
	c = (unsigned long) getc(input);
	d = (unsigned long) getc(input);	
	e = (unsigned long) getc(input);	
	f = (unsigned long) getc(input);		
/*	if (c == EOF || d == EOF || e == EOF || f == EOF) return 0; */
	if (LeastFirst != 0) return ((((((f << 8) | e) << 8) | d) << 8) | c);
	else return ((((((c << 8) | d) << 8) | e) << 8) | f);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* char *typename[6] = {"", "BYTE ", "ASCII", "SHORT", "LONG ", "RATIO"}; */

int typesize[6] = {0, 1, 1, 2, 4, 8}; 	/* units of length/count */

unsigned long TIFFOffset, TIFFLength;

long PSOffset, PSLength, MetaOffset, MetaLength; /* not used ? */

/* we are ignoring the possibility here that length > 1 and such ... */

long indirectvalue(unsigned int type, long length, long offset, FILE *input) {
	long present, val=0;

/*	UNUSED (length); */
	if (length > 1 && traceflag) printf("Length %d\n", length);
	present = ftell(input);			/* remember where we are */
	if (fseek(input, (long) (offset + TIFFOffset), SEEK_SET) != 0) 
		fputs("Error in seek to indirect value\n", stderr);
	if (type == TYPE_LONG) val = (long) ureadfour(input);
	else if (type == TYPE_SHORT) val = ureadtwo(input);
	else if (type == TYPE_BYTE) val = getc(input);
	else fputs("Invalid Indirect Value\n", stderr);
	fseek(input, present, SEEK_SET);	/* return to where we were */
	return val;
}

/* get value of a tag field in TIFF file */

long extractvalue(unsigned int type, unsigned long length, 
				long offset, FILE *input) {
	if (length == 0) return 0;
	switch(type) {
		case TYPE_BYTE:
			if (length <= 4) return offset;
			else return indirectvalue(type, (long) length, (long) offset, input);
		case TYPE_SHORT:
			if (length <= 2) return offset;
			else return indirectvalue(type, (long) length, (long) offset, input);
		case TYPE_LONG:
			if (length == 1) return offset;
			else return indirectvalue(type, (long) length, (long) offset, input);
		default:
			return -1;
	}
}

int skipthisimage (FILE *input, unsigned long ifdpos) {
	int k, j;
	if(fseek(input,	(long) ifdpos + TIFFOffset, SEEK_SET) != 0) {
		fputs("Error in seek while skipping image\n", stderr);
		return -1;
	}
	IFDCount = ureadtwo(input);			/* How many tags in this IFD */
	for (k = 0; k < (int) IFDCount; k++) {	/* read to end of IFD */
		for (j = 0; j < 12; j++) (void) getc(input);
	}
	IFDPosition = ureadfour(input);		/* get next IFD offset in file */
	if (IFDPosition == 0) return -1; 			/*  no more IFDs !!! */
	else return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

long ImageWidth;						/* vital ImageWidth */
long ImageLength;						/* vital ImageLength */

int bytes;								/* total bytes per row output */

int	SamplesPerPixel = 1;				/* vital */
int BitsPerSample = 1;					/* vital (may be more than 1) */
unsigned int compression = 0;			/* vital */
int orientation = 1;
int predictor = 1;						/* for LZW only */

long StripOffset = -1;					/* vital (first strip offset) */
long StripOffsetsPtr;
int StripOffsetsType;
int StripOffsetsLength;

long StripByteCount = -1;				/* first StripByteCount */
long StripByteCountsPtr;
int StripByteCountsType;
int StripByteCountsLength;

int RowsPerStrip = 0;

int StripsPerImage;						/* computed from above */

long ColorMapPtr = 0;	/* pointer to map in case of Palette Color Images */

int PhotometricInterpretation = 1;	/* default */

int PlanarConfiguration=1;

int BitsPerPixel;			/* BitsPerSample * SamplePerPixel */

long InRowLength; 			/* number of source bytes from file */
long BufferLength;			/* number of bytes of intermediate data */
long OutRowLength;			/* number of processed bytes for output */

/* long BytesPerRow; */		/* number of destination bytes in bitmap */

/* NOTE: MinSampleValue and MaxSampleValue should not affect appearance */

int MinSampleValue, MaxSampleValue;			/* never used ? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* read the tag fields in the TIFF file, ignore ones we don't care about */

int readfields (FILE *input, unsigned long ifdpos) {
	unsigned int k, tag, type;
	unsigned long length, offset;
	int c;

	if (traceflag) printf("Now reading TIFF images fields\n");

	if (fseek(input, (long) ifdpos + TIFFOffset, SEEK_SET) != 0) {
		fputs("Error in seek while reading tags\n", stderr);
		return -1;
	}
	IFDCount = ureadtwo(input);			/* How many tags in this IFD */
	
	ImageWidth = ImageLength = -1;
	SamplesPerPixel = BitsPerSample = 1;
	compression = 0; orientation = 1;
	PlanarConfiguration = 1; predictor = 1;
	StripOffset = -1; StripByteCount = -1; RowsPerStrip = 0;
	PhotometricInterpretation = 1;	/* default */
	ColorMapPtr = 0;	/* pointer to map in case of Palette Color Images */
	MinSampleValue = -1; MaxSampleValue = -1;

	for (k = 0; k < IFDCount; k++) {
		tag = ureadtwo(input);		/* tag - key */
		type = ureadtwo(input);		/* value type */
		if ((tag == 0 && type == 0) || type > 5) {
			c = getc(input); ungetc(c, input);
			fprintf(stderr, "Tag: %u Type: %u (k %d c %d)\n", tag, type, k, c);
			break;
		}
		length = ureadfour(input);	/* `length' (better named `count') */
		if (length == 1) {
			if (type == TYPE_LONG) offset = ureadfour(input);
			else if (type  == TYPE_SHORT) {
				offset = ureadtwo(input);	
				(void) ureadtwo(input);		/* should be zero */
			}
			else if (type == TYPE_BYTE) {
				offset = getc(input);
				(void) getc(input);(void) getc(input);(void) getc(input);
			}
			else offset = ureadfour(input);	/* for ratio e.g. */
		}
		else offset = ureadfour(input);	/* value */

		switch (tag) {
			case IMAGEWIDTH:
				ImageWidth = extractvalue(type, length, (long) offset, input);
				break;
			case IMAGELENGTH:
				ImageLength = extractvalue(type, length, (long) offset, input);
				break;
			case BITSPERSAMPLE:
				BitsPerSample =
					(int) extractvalue(type, length, (long) offset, input);
				break;
			case COMPRESSION:
				compression =
					(unsigned int) extractvalue(type, length, (long) offset, input);
				break;
			case PREDICTOR:
				predictor =
					(int) extractvalue(type, length, (long) offset, input);
				break;
			case SAMPLESPERPIXEL:
				SamplesPerPixel =
					(int) extractvalue(type, length, (long) offset, input);
				break;
			case STRIPOFFSETS:
				StripOffsetsPtr = offset;
			    StripOffsetsType = type;
			    StripOffsetsLength = (int) length;
				StripOffset = extractvalue(type, length, (long) offset, input);
				break;
			case ROWSPERSTRIP:
				RowsPerStrip =
					(int) extractvalue(type, length, (long) offset, input);
				break;
			case STRIPBYTECOUNTS:
				StripByteCountsPtr = offset;
			    StripByteCountsType = type;
			    StripByteCountsLength = (int) length;
				StripByteCount = extractvalue(type, length, (long) offset, input);
				break;
			case ORIENTATION:
				orientation =
					(int) extractvalue(type, length, (long) offset, input);
				break;
			case MINSAMPLEVALUE:
				MinSampleValue = (int) extractvalue(type, length, (long) offset, input);
				break;
			case MAXSAMPLEVALUE:
				MaxSampleValue = (int) extractvalue(type, length, (long) offset, input);
				break;
			case PHOTOMETRICINTERPRETATION:
				PhotometricInterpretation = 
					(int) extractvalue(type, length, (long) offset, input);
				break;
			case PLANARCONFIGURATION:
				 PlanarConfiguration = 
					(int) extractvalue(type, length, (long) offset, input);
				break;
			case COLORMAP:
/*				ColorMap = extractvalue(type, length, (long) offset, input); */
				ColorMapPtr = offset;
/*		Assume the type is SHORT (16 bit per entry) */
/*		Assume the length is 3 * 2 ^ BitsPerSample - 1 */
				break;
			default:
				break;
		}
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int computeheader(void);

void writepsheader(FILE *);

void writepstrailer(FILE *);

int readpackbits (unsigned char *lpBuffer, FILE *input, int RowLength);

int huffmanrow (unsigned char *lpBuffer, FILE *input, int width);

int DecodeLZW (FILE *output, FILE *input, unsigned char *lpBuffer);

void expandcolor (unsigned char *s, long width);

void compresscolor (unsigned char *s, long width);

int writearow (FILE *output, unsigned char *s, unsigned long width);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Remember to deallocate at end */

int ReadColorMap (FILE *input, long ColorMapPtr, int BitsPerSample) {
	long present;
	unsigned int _far *PalettePtr;
	int k, n, nint;
	unsigned int _far *PaletteRed;	/* for debugging */
	unsigned int _far *PaletteGreen;
	unsigned int _far *PaletteBlue;

	if (traceflag) printf("Reading Color Map\n");
/*	need three tables each of 2 ^ BitsPerSample integers */
	n = 1 << BitsPerSample;		/* integers per table */
	PaletteSize = n;			/* remember for later */
	nint = n * 3;				/* total number of integers */
	Palette = (unsigned int _far *) _fmalloc(nint * 2);	/* bytes */
	if (Palette == NULL) {
		fputs("ERROR: unable to allocate palette\n", stderr);
		return -1;
	}
	present = ftell(input);
	if (traceflag) printf("Going to ColorMap at %ld + %lu\n",
		ColorMapPtr, TIFFOffset);
	if (fseek (input, (long) ColorMapPtr + TIFFOffset, SEEK_SET) != 0)
		fputs("ERROR: in seek to ColorMap\n", stderr);
	PalettePtr = Palette;
	for (k = 0; k < nint; k++) *PalettePtr++ = ureadtwo (input);
	fseek (input, present, SEEK_SET);
	if (!traceflag) return 0;
	PaletteRed = Palette;
	PaletteGreen = PaletteRed + PaletteSize;
	PaletteBlue = PaletteGreen + PaletteSize;
	for (k = 0; k < n; k++)
		if (traceflag) printf ("Index %d red %u green %u blue %u\n",
			k, PaletteRed[k], PaletteGreen[k], PaletteBlue[k]);
	return 0;
}

int ProcessRow (FILE *output, unsigned char *lpBuffer, long InRowLength,
	long BufferLength, long OutRowLength) {
	if (bExpandColor) expandcolor(lpBuffer, InRowLength);
	if (bCompressColor) compresscolor(lpBuffer, BufferLength);
	if (writearow(output, lpBuffer, OutRowLength) != 0) return -1;
	else return 0;
}

int readtifffile (FILE *output, FILE *input, int nifd, int readflag) {
/*	int i, j, flag; */
	int i, flag;
/*	long present; */
/*	int k, color, n; */
	unsigned long ImageSize;
	unsigned int nread;
/*	unsigned char *u; */ 					/* pointer into buffer */
	unsigned char *lpBuffer=NULL;

	TIFFVersion = ureadtwo(input);
	if (TIFFVersion != TIFF_VERSION) {
		fputs("Incorrect TIFF version code\n", stderr);
		return -1;		/* bad version number for TIFF file */
	}
	
/*	Skip to desired image (if not first in file) */

	IFDPosition = ureadfour(input);		/* get first IFD offset in file */
	while (nifd-- > 1) {
		if (skipthisimage(input, IFDPosition) < 0) {
			fprintf(stderr, "ERROR: Subimage %d not found", nifd);
			return -1;
		}
	}

/*	Now at desired image */
	(void) readfields(input, IFDPosition);	/* read tag fields in TIFF file */

	IFDPosition = ureadfour(input);		/* get next IFD offset in file */

	if (readflag == 0) return -1;	/*  only scanning for width & height */

	bColorImage = 0;			/* if image is RGB or Palette color */
	bExpandColor = 0;			/* comes on if Palette color image */
	bCompressColor = 0;			/* if image is colored and not `colorimage'  */
	if (ColorMapPtr) {					/* is it a palette color image ? */
		(void) ReadColorMap(input, ColorMapPtr, BitsPerSample);
		bExpandColor = 1;
		bColorImage = 1;
	}

	if (traceflag)
	printf("Width %ld, Height %ld, BitsPerSample %d, SamplesPerPixel %d\n", 
		   ImageWidth, ImageLength, BitsPerSample, SamplesPerPixel); 

   if (traceflag) printf("Compression %u PhotometricInterpretation %d\n",
	   compression, PhotometricInterpretation);

	if (verboseflag && predictor != 1) printf("Predictor %d\n", predictor);

	BitsPerPixel = BitsPerSample * SamplesPerPixel;
	if (RowsPerStrip > 0)
	StripsPerImage = (int) ((ImageLength + RowsPerStrip - 1) / RowsPerStrip);
	else StripsPerImage = 1;

	InRowLength = (ImageWidth * BitsPerPixel + 7) / 8;	/* row length file */

/*	if (bExpandColor) BufferLength = InRowLength * 3; */
	if (bExpandColor) BufferLength = ImageWidth * 3;
	else BufferLength = InRowLength;

/*	do compression of 24 bit color if use of `colorimage' not allowed */
/*	if (bitsperpixel == 24 && bCompressFlag != 0 && forceice == 0) { */
/*	if (BitsPerPixel == 24 && bCompressFlag != 0) { */
	if ((BitsPerPixel == 24 || bExpandColor) && bAllowColor == 0) {
		bCompressColor = 1;
		OutRowLength = BufferLength / 3;
	}
	else OutRowLength = BufferLength;
		
	if (traceflag)
		printf("InRowLength %ld BufferLength %ld OutRowLength %ld\n",
			InRowLength, BufferLength, OutRowLength);

	if (computeheader () != 0) return -1;	/* noticed format not supported? */

	writepsheader(output);

/*	following should already be taken care of in `computeheader' */
/*	if (compression > 1 && compression != PACK_BITS) {  */
	if (compression > TIFF_CCITT && compression != LZW_COMPRESSION &&
		compression != PACK_BITS) { 
		fprintf(stderr,	"ERROR: Unknown compression scheme (%d)", compression);
		return -1; 
	}
	
	ImageSize = (unsigned long) InRowLength * ImageLength;
/*	check whether values reasonable */	
	if (ImageSize == 0) {
		fprintf(stderr, "ERROR: Zero image size");
		return -1;
	}
	if (ImageWidth > MAXIMAGE || ImageLength > MAXIMAGE ||	/* bad data ? */
		ImageSize > 4000000) {	/* arbitrary limits (to catch bad files) */
		fprintf(stderr,
			"ERROR: TIFF file too large\n(%ld x %ld (%d) => %ld bytes)", 
				ImageWidth, ImageLength, BitsPerPixel, ImageSize);
		return -1;
	}
	if (ImageWidth < 0 || ImageLength < 0 ||		/* missing fields */
				StripOffset < 0) {					/* missing fields */
		fprintf(stderr, "ERROR: TIFF file missing required tags");
		return -1;
	}		

	if (fseek(input, (long) StripOffset + TIFFOffset, SEEK_SET) != 0)  {
		fputs("ERROR in seek to StripOffset\n", stderr);
		return -1;
	}

/*	maybe use far memory ? */	/* maybe don't use if LZW compression ? */
/*	if (compression != LZW_COMPRESSION) { */
	if ((lpBuffer = malloc((int) BufferLength)) == NULL) { 
		fprintf(stderr, "ERROR: Unable to allocate %d bytes\n",
			InRowLength);
		return -1;
	}
/*	} */

/*	in case minsamplevalue and maxsamplevalue not given in tags */
/*	if (MinSampleValue < 0 || MaxSampleValue < 0 ||
			MinSampleValue == MaxSampleValue) {
		MinSampleValue = 0; MaxSampleValue = (1 << BitsPerSample) - 1;
	} */	/* never used */

/*  Actually go and read the file now ! */

/*	following should already be taken care of in `computeheader' */

	if (compression > TIFF_CCITT && compression != LZW_COMPRESSION &&
		compression != PACK_BITS) {  
		fprintf(stderr,	"ERROR: Unknown compression scheme (%d)", compression);
		return -1;
	}
	else if (verboseflag) {
		if (compression == PACK_BITS) printf("Using PACK_BITS\n");
		else if (compression == TIFF_CCITT) printf("Using TIFF_CCITT\n");
		else if (compression == LZW_COMPRESSION) printf("Using LZW\n");
	}

	flag = 0;								/* flag gets set if EOF hit */

/*	LZW needs to be done by strips, the others can be done by row */

	if (compression == LZW_COMPRESSION) {
		DecodeLZW (output, input, lpBuffer);
	}
	else {		/* else not LZW compression */

		for (i = 0; i < (int) ImageLength; i++) {	/* read image lines */

			if (compression == PACK_BITS) {		/* compressed binary file */
				if (readpackbits (lpBuffer, input, (int) InRowLength) != 0) {
					flag = -1;
					break;
				}
			}
			else if (compression == TIFF_CCITT) {	/* CCITT 1D compression */
				if (huffmanrow (lpBuffer, input, (int) ImageWidth) != 0) {
					flag = -1;
					break;
				}
			}
			else {								/* uncompressed file */
				nread = fread(lpBuffer, 1, (unsigned int) InRowLength, input);
				if (nread != (unsigned int) InRowLength) {	/* read a row */
					flag = -1;
					break;
				} 
			}
			if (ProcessRow (output, lpBuffer, InRowLength, BufferLength,
				OutRowLength) != 0) break;
/*			if (flag != 0) break;	*//* hit EOF or other error */
/*			check on abort flag here somewhere also ? */
		}	/* end of this row */
	} /* end of not LZW compression */
	
/* now finished reading */	/* maybe use far memory for lpBuffer ? */

	if (lpBuffer != NULL) {
		free(lpBuffer);
		lpBuffer = NULL;
	}
	if (Palette != NULL) {
		_ffree(Palette);
		Palette = NULL;
	}

	writepstrailer(output);
	return flag;
}

/* Try and see whether EPSF file and read header info if so: */
/* fills in TIFFOffset and PSOffset and MetaOffset and lengths */
/* returns zero if not an EPSF file */ /* file position is end of EPSF head */

int readepsfhead(FILE *special) {
	int c, d, e, f;

	PSOffset = 0;				/* redundant */
	MetaOffset = 0;				/* redundant */
	TIFFOffset = 0;				/* redundant */

	c = getc(special); d = getc(special);
	e = getc(special); f = getc(special);
	if (c == 'E' + 128 && d == 'P' + 128 &&
		e == 'S' + 128 && f == 'F' + 128) {
		LeastFirst = 1;
		PSOffset = (long) ureadfour(special);	/* read PS start offset */ 
		PSLength = (long) ureadfour(special);	/* read PS length */
		MetaOffset = (long) ureadfour(special);	/* read MF start offset */
		MetaLength = (long) ureadfour(special);	/* read MF length */
		TIFFOffset = (long) ureadfour(special);	/* read TIFF start offset */
		TIFFLength = (long) ureadfour(special);	/* read TIFF length */
		(void) ureadtwo(special);			/* should be 255, 255 */
		return -1;
	}
	else return 0;							/* not an EPSF file */
}

/* rewrite this later to use findepsfile */

int readimagefile (FILE *output, FILE *special, int nifd, int readflag) {
/*	char infilename[MAXFILENAME]; */
/*	long present; */
	int c, d; 

/*	strcpy(infilename, filename);
	if ((special = findepsfile(infilename, -1, "tif")) == NULL) {
		return -1;					
	} */

	TIFFOffset = 0;				/* normally beginning of file */
	PSOffset = 0;
	MetaOffset = 0;

	c = getc(special); (void) ungetc(c, special);
	if (c > 128) {				/* see whether perhaps EPSF file */
		if (readepsfhead(special) != 0) {			/* is it EPSF file ? */
			if (TIFFOffset == 0 || TIFFLength == 0) {
				fputs("ERROR: Zero TIFF offset or length\n", stderr);
				return -1;
			}
			if(fseek(special, TIFFOffset, SEEK_SET) != 0)  {
				fputs("ERROR: Error in seek to TIFFOffset\n", stderr);
				return -1;
			}
		}
		else {
			fprintf(stderr, "ERROR: Not a valid EPSF or TIFF file\n"); 
			return -1;
		}
	}

/*	Try and deal with PostScript ASCII stuff */
	c = getc(special); d = getc(special);
	if (c == '%' && d == '!') {		/* ordinary PS file ? */
		fputs("ERROR: Ordinary PS file, not TIFF\n", stderr);
		return -1;
	}
	else if (c == 'I' && d == 'I') LeastFirst = 1;	/* PC style TIFF file */
	else if (c == 'M' && d == 'M') LeastFirst = 0;	/* Mac style TIFF file */
	else {
		fputs("ERROR: Not a valid EPSF or TIFF file\n", stderr); 
		return -1;		/* not a TIFF subfile !!! */
	}

/*	now have decided that this is a TIFF file (or TIFF preview) */
	(void) readtifffile(output, special, nifd, readflag);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *tiffpath="c:\\tiff";

int tifftops (char *filename, int nifd) {
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	FILE *input, *output;
	int flag;

	strcpy(infilename, filename);
	extension(infilename, "tif");
	
	if ((input = fopen(infilename, "rb")) == NULL) {
		strcpy(infilename, tiffpath);
		strcat(infilename, "\\");
		strcat(infilename, filename);
		extension(infilename, "tif");
		if ((input = fopen(infilename, "rb")) == NULL) {
			strcpy(infilename, tiffpath);
			strcat(infilename, "\\");
			strcat(infilename, "images");
			strcat(infilename, "\\");
			strcat(infilename, filename);
			extension(infilename, "tif");
			if ((input = fopen(infilename, "rb")) == NULL) {
				perror(infilename);
				return -1;
			}
		}
	}
/*	fseek (input, 0, SEEK_END);	
	filelength = ftell(input);	
	fseek (input, 0, SEEK_SET);	 */
/*	printf("Analysis of file %s (%d bytes)\n\n", infilename, filelength); */

	strcpy(outfilename, stripname(filename));
	forceexten(outfilename, "ps");
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);
		fclose(input);
		return -1;
	}

	if (verboseflag) printf("%s => %s\n", infilename, outfilename);

	flag = readimagefile(output, input, nifd, 1);

	fclose (output); 
	fclose (input); 

	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int column;

/* Bitspersample can be 1, 2, 4, 8 for ease of conversion to PS */
/* Each row takes an integral number of bytes */

/* void converttohex (FILE *output, FILE *input) {
	int c, d, i, j;

	column = 0;
	for (j = 0; j < ImageLength; j++) {
		for (i = 0; i < bytes; i++) {
			if (column >= 39) {
				putc('\n', output);
				column = 0;
			}
			c = getc(input);
			d = c & 15;
			c = c >> 4;
			if (c > 9) putc(c + 'A' - 10, output);
			else putc(c + '0', output);
			if (d > 9) putc(d + 'A' - 10, output);
			else putc(d + '0', output);
			column++;
		}
		putc('\n', output);
		column = 0;
		if (ferror(output)) break;
	}
} */	/* not used */

/* Expand palette color image into full RGB color image */
/* Presently only set up for 8 bit palettes ... !!! */

void expandcolor (unsigned char *lpBuffer, long width) {
	int k, n, ns;
	unsigned int _far *PaletteRed;
	unsigned int _far *PaletteGreen;
	unsigned int _far *PaletteBlue;
	unsigned char *s;
	unsigned char *t;
	unsigned int red=0, green=0, blue=0;
	int PaletteIndex, Mask;
	int Byte = 0, BitsLeft = 0;

	if (traceflag) printf("Palette Color => Color ");

	if (Palette == NULL) {
		fputs("ERROR: missing palette information\n", stderr);
		return;
	}
	PaletteRed = Palette;
	PaletteGreen = PaletteRed + PaletteSize;
	PaletteBlue = PaletteGreen + PaletteSize;
	Mask = (1 << BitsPerSample) - 1;

/*	First slide input data upwards to top of buffer area */

	n = (int) width;				/* InRowLength */
	ns = ImageWidth;				/* samples per row */

	for (k = n-1; k >= 0; k--)
		lpBuffer[k + BufferLength - InRowLength] = lpBuffer[k];
	
	s = lpBuffer + BufferLength - InRowLength;	/* start of input data */
	if (BitsPerSample != 8) {					/* nibble code init */
		Byte = 0; BitsLeft = 0;
	}
	t = lpBuffer;								/* start of output data */
	for (k = 0; k < ns; k++) {
		if (BitsPerSample != 8) {				/* slow, but inefficient ! */
			if (BitsLeft <= 0) {
				Byte = *s++; BitsLeft = 8;
			}
			BitsLeft -= BitsPerSample;
/*			PaletteIndex = Byte >> (BitsPerSample - BitsLeft); */
			PaletteIndex = Byte >> BitsLeft;
			PaletteIndex = PaletteIndex & Mask;
/*			BitsLeft -= BitsPerSample; */
		}
		else PaletteIndex = *s++;
		if (PaletteIndex < PaletteSize) {
			red = PaletteRed[PaletteIndex];
			green = PaletteGreen[PaletteIndex];
			blue = PaletteBlue[PaletteIndex];
		}
		else fprintf(stderr, "ERROR: Bad Palette Index %d >= %d\n",
			PaletteIndex, PaletteSize);
/*		if (traceflag) printf ("Index %d red %u green %u blue %u\n",
			PaletteIndex, red, green, blue); */
		*t++ = (unsigned char) (red >> 8); 
		*t++ = (unsigned char) (green >> 8); 
		*t++ = (unsigned char) (blue >> 8); 
	}	
}

/* gray = 0.3 * red + 0.59 * green + 0.11 * blue */

 void compresscolor (unsigned char *lpBuffer, long width) {
	int k, n;
	unsigned char *s;
	unsigned char *t;
	unsigned int red, green, blue, gray;

	if (traceflag) printf("Color => Gray ");
	s = lpBuffer; t = lpBuffer;
	n = (int) (width / 3);
	for (k = 0; k < n; k++) {
		red = *s++; green = *s++; blue = *s++;
		gray = (int) (((long) blue * 28 + (long) red * 77 + (long) green * 151)
			>> 8);
		*t++ = (unsigned char) gray;
	}	
} 

int writearow (FILE *output, unsigned char *s, unsigned long width) {
	unsigned int c, d, old=0;
	int k, n;

	n = (int) width;
	column = 0;
	for (k = 0; k < n; k++) {
/*		if (column >= 38) { */
		if (column >= 39) {
			putc('\n', output);
			column = 0;
		}
		c = *s++;
		if (predictor == 2 && compression == LZW_COMPRESSION) {
			c = (c + old) & 255;
			old = c;
		}					/* LZW predictor hack */
		d = c & 15;
		c = c >> 4;
		if (c > 9) putc(c + 'A' - 10, output);
		else putc(c + '0', output);
		if (d > 9) putc(d + 'A' - 10, output);
		else putc(d + '0', output);
		column++;
	}
	putc('\n', output);
	column = 0;
	if (ferror(output)) {
		fputs("ERROR: Output error\n", stderr);
		return -1;
	}
	return 0;
}

int computeheader (void) {
	int changeflag= 0;

	if (traceflag) printf("Now computing header information\n");

/*	Now for some sanity checks */
	if (PlanarConfiguration != 1) {
		fputs("ERROR: Multiple color planes not supported\n", stderr);
		return -1; 
	}
	if (BitsPerSample > 8) {
		fprintf(stderr, "ERROR: Maximum of 8 bits per sample (%d)\n",
			BitsPerSample);
		return -1;
	}
	if ((BitsPerSample & (BitsPerSample-1)) != 0) {
		fprintf(stderr, "ERROR: Bits per sample must be power of two (%d)\n",
			BitsPerSample);
		return -1;
	}
	if (compression > TIFF_CCITT && compression != LZW_COMPRESSION &&
		compression != PACK_BITS ) {
		fprintf(stderr,	"ERROR: Unknown compression scheme (%d)", compression);
		return -1;
	}

	if (PhotometricInterpretation == 3) {	/* that is palette color */
		if (SamplesPerPixel != 1) {
			fprintf(stderr,
				"ERROR: Palette color must have one sample per pixel\n");
			return -1;
		}
	}
/*	check whether need to use `colorimage' operator instead of `image' */
	if (SamplesPerPixel != 1) {
		if (PhotometricInterpretation != 2)
			fputs("More than one sample per pixel, but not RGB?\n", stderr);
		bColorImage = 1;
		if (!bAllowColor) {
			fprintf(stderr, "WARNING: More than one sample per pixel (%d)\n",
				SamplesPerPixel);
/*			return -1; */
			bCompressColor = 1;			/* then need to compress colors */
		}
	}
	if (verboseflag && PhotometricInterpretation > 3) {
		printf("Photometricinterpretation %d\n", PhotometricInterpretation);
	}
/*	0 => 0 will be white and 2^n-1 black */
/*	1 => 0 will be black and 2^n-1 white (default) */
/*	2 => RGB model */
/*	3 => palette color */
/*	4 => transparency mask */
/*	Fix this one later */
/*	if (PhotometricInterpretation == 3) bColorImage = 1;  */
/*	if (ColorMap != 0) 	bColorImage = 1; */
	if (PhotometricInterpretation == 3 && ColorMapPtr == 0) {
		fputs("ERROR: Palette Color Image must have Palette!\n", stderr);
		return -1;
	}
	if (PhotometricInterpretation == 2) {
		if (SamplesPerPixel == 1) 
			fputs("RGB, but not more than one sample per pixel?\n", stderr);
		if (!bAllowColor) {
			fprintf(stderr, "WARNING: RGB color image (%d)\n",
				SamplesPerPixel);
			bCompressColor = 1;
		}
		bColorImage = 1;
	}
/*	0 => 0 will be white and 2^n-1 black */
/*	1 => 0 will be black and 2^n-1 white (default) */

	if (PhotometricInterpretation == 0) {
		if (traceflag) printf("Image grey levels will be inverted\n");
		bInvertImage = 1;
	}
	else bInvertImage = 0;

	if (traceflag)
		printf("ExpandColor %d CompressColor %d InvertImage %d\n",
			bExpandColor, bCompressColor, bInvertImage);

/*	Following just for test output from TIFFTOPS - not for DVIPSONE */
	for (;;) {			/* try and adjust scale to fit paper */
		pwidth = (int) (((long) ImageWidth * 72) / spi);
		pheight = (int) (((long) ImageLength * 72) / spi);
		if (pwidth + xll < pagewidth && pheight + yll < pageheight) break;
		spi = spi * 6 / 5;
		changeflag++;
	}
	if (changeflag)
		printf("Increased samples per inch in output to %d\n", spi);
	else printf("Using %d samples per inch\n", spi);

	if (centerflag) {	/* try and center on page */
		xll = (pagewidth -  pwidth) / 2;
		yll = (pageheight - pheight) / 2;
	}

	xur = xll + pwidth;  yur = yll + pheight;

/*	compute bytes per row (both input and output?) */
/*	if (bColorImage)bytes = (int) (((long) ImageWidth * BitsPerPixel + 7) / 8);
	else bytes = (int) (((long) ImageWidth * BitsPerSample + 7) / 8); */
/*	Following only correct if no expansion or contraction of color images */
/*	bytes = (int) (((long) ImageWidth * BitsPerPixel + 7) / 8); */
	bytes = (int) OutRowLength;
	if (traceflag) printf("%d bytes per row\n", bytes); 

	return 0;
}

char *invert=
"[{1 exch sub} /exec load currentransfer /exec load] cvx settransfer\n";

/* compress some of this code into DVIPREAMB.PS ? */

void writepsheader (FILE *output) {
	int bits;
	if (traceflag) printf("Now writing out header information\n");
	fputs("%!PS-Adobe-2.0\n", output);
	fprintf(output, "%%%%Creator: %s\n", program);
	fprintf(output, "%%%%BoundingBox: %d %d %d %d\n", xll, yll, xur, yur);
	fprintf(output, "save\n");
	if (!bColorImage) {
		if (bInvertImage)
/*			fprintf(output, "{1 exch sub} settransfer\n"); */
			fputs(output, invert);
		if (wantmagic)	fputs(eps_magic, output);
		fprintf(output,
			"currentscreen 3 1 roll pop pop %d %d 3 -1 roll setscreen\n", 
				frequency, angle);
	}
	if (bExpandColor) bits = 8;
	else bits = BitsPerSample;
/*	create the string before or after the `save' ? */
	fprintf(output, "/picstr %d string def\n", bytes); 
	fprintf(output, "%d %d translate ", xll, yll);
	fprintf(output, "%d %d scale\n", pwidth, pheight);
	fprintf(output, "%ld %ld %d\n", ImageWidth, ImageLength, bits);
	fprintf(output, "[%ld 0 0 %ld 0 %ld]\n",
		ImageWidth, -ImageLength, ImageLength);
	fputs("{currentfile picstr readhexstring pop} bind\n", output);
/*	samples per pixel better be 1 (grey), 3 (RGB) or 4 (CMYK) */
	if (bColorImage && bAllowColor)
		fprintf(output, "false %d colorimage\n", SamplesPerPixel);
/*	else if (bCompressColor) fputs("image\n", output); */
	else fputs("image\n", output);	/* either not color, or color compressed */
}

void writepstrailer (FILE *output) {
	if (traceflag) printf("Now writing trailer\n");
	fputs("restore\n", output);
	fputs("showpage\n", output);
	fputs("%%EOF\n", output);
	if (wantcntrld) putc(4, output);
}


void showusage (char *argv[]) {
	exit(1);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Read a row using PACK_BITS compression scheme */
/* returns non-zero if problems encountered */

int readpackbits (unsigned char *lpBuffer, FILE *input, int RowLength) {
	unsigned char *u=lpBuffer;
	int c, k, n, total=0, flag=0;
	
	for(;;) {
		if ((n = getc(input)) < 0) {	/* premature EOF */
			fprintf(stderr, "Premature EOF");
			flag = -1;
			break;	
		}
		else if (n < 128) {			/* use next (n+1) bytes as is */
			for (k=0; k < n+1; k++) *u++ = (char) getc(input);  
			total += n+1;
		}
		else if (n > 128) {			/* repeat next byte (257 - n) times */
			c = getc(input);
			for (k=0; k < (257 - n); k++) *u++ = (char) c; 	 
			total += (257 - n);
		}
/*		and n == 128 is a NOP */
		if (total == RowLength) break;	/* enough bytes yet ? */
		if (total > RowLength) {		/* too many bytes ? */
			fprintf(stderr, "Too many bytes in compressed row\n");
			flag = -1;
			break;
		}
	}
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Implement 1d CCITT Huffman code decompression (minus EOL) */
/* ==> implies BitsPerSample == 1 */
/* white runs = 0, black runs = 1 */
/* normal PhotometricInterpretation == 0 */
/* invert black/white if PhotometricInterpretation == 1 */

int byte, bitsleft;				/* input buffering in splitting bits */

int getbit (FILE *input) {
	if (bitsleft-- <= 0) {
		if ((byte = getc(input)) == EOF) {
			fprintf(stderr, "Unexpected EOF\n");
/*			exit(1); */			/* Wait for it to create bad code */
		}
		bitsleft = 7;
	}
	byte = byte << 1;
/*	if (traceflag) printf("%d", (byte & 256) ? 1 : 0); */
	if (byte & 256) return 1;
	else return 0;
}

#pragma optimize ("lge", off) 

/* Actually commonmake may be OK with compiler optimizations ON */

int commonmake (FILE *input) { /* common black/white make up codes (7 zeros) */
/*	if (traceflag) printf ("commonmake entry "); */
	if (getbit(input)) {	/* 00000001 */
		if (getbit(input)) {	/* 000000011 */
			if (getbit(input)) {	/* 0000000111 */
				if (getbit(input)) {	/* 00000001111 */
					if (getbit(input)) {	/* 000000011111 */
						return 2560;
					}
					else { /* 000000011110 */
						return 2496;
					}
				}
				else { /* 00000001110 */
					if (getbit(input)) {	/* 000000011101 */
						return 2432;
					}
					else { /* 000000011100 */
						return 2368;
					}
				}
			}
			else { /* 0000000110 */
				if (getbit(input)) {	/* 00000001101 */
					return 1920;
				}
				else { /* 00000001100 */
					return 1856;
				}
			}
		}
		else { /* 000000010 */
			if (getbit(input)) {	/* 0000000101 */
				if (getbit(input)) {	/* 00000001011 */
					if (getbit(input)) {	/* 000000010111 */
						return 2304;
					}
					else { /* 000000010110 */
						return 2240;
					}
				}
				else { /* 00000001010 */
					if (getbit(input)) {	/* 000000010101 */
						return 2176;
					}
					else { /* 000000010100 */
						return 2112;
					}
				}
			}
			else { /* 0000000100 */
				if (getbit(input)) {	/* 00000001001 */
					if (getbit(input)) {	/* 000000010011 */
						return 2048;
					}
					else { /* 000000010010 */
						return 1984;
					}
				}
				else { /* 00000001000 */
					return 1792;
				}
			}
		}
	}
	else { /* 00000000 */
/*	Actually, EOL code is not supposed to be used in TIFF compression 2 */
		if (!getbit(input)) { /* 000000000 */
			if (!getbit(input)) { /* 0000000000 */
				if (!getbit(input)) { /* 00000000000 */
					if (getbit(input)) { /* 000000000001 */
						fprintf(stderr, "EOL ");
						return 0;				/* ??? */
					}
				}
			}
		}
	}
	fprintf(stderr, "Impossible make-up run\n");
	return -1;	/* error */
}

/* Compiler screws up the following code if optimizations turned on */

int whiterun (FILE *input) {
/*	if (traceflag) printf ("whiterun entry "); */
	if (getbit(input)) {	/* 1 */
		if (getbit(input)) {	/* 11 */
			if (getbit(input)) {	/* 111 */
				if (getbit(input)) {	/* 1111 */
					return 7;
				}
				else {			/* 1110 */
					return 6;
				}
			}
			else {			/* 110 */
				if (getbit(input)) {	/* 1101 */
					if (getbit(input)) { /* 11011 */
						return 64;		/* make up */
					}
					else {			/* 11010 */
						if (getbit(input)) { /* 110101 */
							return 15;
						}
						else {			/* 110100 */
							return 14;
						}
					}
				}
				else {			/* 1100 */
					return 5;
				}
			}
		}
		else {			/* 10 */
			if (getbit(input)) {	/* 101 */
				if (getbit(input)) {	/* 1011 */
					return 4;
				}
				else {			/* 1010 */
					if (getbit(input)) { /* 10101 */
						if (getbit(input)) { /* 101011 */
							return 17;
						}
						else {			/* 101010 */
							return 16;
						}
					}
					else {			/* 10100 */
						return 9;
					}
				}
			}
			else {			/* 100 */
				if (getbit(input)) {	/* 1001 */
					if (getbit(input)) { /* 10011 */
						return 8;
					}
					else {			/* 10010 */
						return 128;	/* make up */
					}
				}
				else {			/* 1000 */
					return 3;
				}
			}
		}
	}
	else {			/* 0 */
		if (getbit(input)) {	/* 01 */
			if (getbit(input)) {	/* 011 */
				if (getbit(input)) {	/* 0111 */
					return 2;
				}
				else {			/* 0110 */
					if (getbit(input)) { /* 01101 */
						if (getbit(input)) { /* 011011 */
							if (getbit(input)) { /* 0110111 */
								return 256;	/* make up */
							}
							else {			/* 0110110 */
								if (getbit(input)) {	/* 01101101 */
									if (getbit(input)) {	/* 011011011 */
										return 1408;	/* make up */
									}
									else { /*  011011010 */
										return 1344;	/* make up */
									}
								}
								else {			/* 01101100 */
									if (getbit(input)) {	/* 011011001 */
										return 1280;	/* make up */
									}
									else { /* 011011000 */
										return 1216;	/* make up */
									}
								}
							}
						}
						else { /* 011010 */
							if (getbit(input)) { /* 0110101 */
								if (getbit(input)) { /* 01101011 */
									if (getbit(input)) { /* 011010111 */
										return 1152;	/* make up */
									}
									else {			/* 011010110 */
										return 1088;	/* make up */
									}
								}
								else { /* 01101010 */
									if (getbit(input)) { /* 011010101 */
										return 1024;	/* make up */
									}
									else { /* 011010100 */
										return 960; /* make up */
									}
								}
							}
							else { /* 0110100 */
								if (getbit(input)) { /* 01101001 */
									if (getbit(input)) { /* 011010011 */
										return 896;	/* make up */
									}
									else { /* 011010010 */
										return 832;	/* make up */
									}
								}
								else { /* 01101000 */
									return 576;	/* make up */
								}
							}
						}
					}
					else { /* 01100 */
						if (getbit(input)) { /* 011001 */
							if (getbit(input)) { /* 0110011 */
								if (getbit(input)) { /* 01100111 */
									return 640;	/* make up */
								}
								else { /* 01100110 */
									if (getbit(input)) { /* 011001101 */
										return 768;	/* make up */
									}
									else { /* 011001100 */
										return 704;	/* make up */
									}
								}
							}
							else { /* 0110010 */
								if (getbit(input)) { /* 01100101 */
									return 512;	/* make up */
								}
								else { /* 01100100 */
									return 448;	/* make up */
								}
							}
						}
						else { /* 011000 */
							return 1664;	/* make up */
						}
					}
				}
			}
			else {			/* 010 */
				if (getbit(input)) {	/* 0101 */
					if (getbit(input)) { /* 01011 */
						if (getbit(input)) { /* 010111 */
							return 192;		/* make up */
						}
						else { /* 010110 */
							if (getbit(input)) { /* 0101101 */
								if (getbit(input)) { /* 01011011 */
									return 58;
								}
								else { /* 01011010 */
									return 57;
								}
							}
							else { /* 0101100 */
								if (getbit(input)) { /* 01011001 */
									return 56;
								}
								else { /* 01011000 */
									return 55;
								}
							}
						}
					}
					else {			/* 01010 */
						if (getbit(input)) { /* 010101 */
							if (getbit(input)) { /* 0101011 */
								return 25;
							}
							else {			/* 0101010 */
								if (getbit(input)) { /* 01010101 */
									return 52;
								}
								else { /* 01010100 */
									return 51;
								}
							}
						}
						else {			/* 010100 */
							if (getbit(input)) { /* 0101001 */
								if (getbit(input)) { /* 01010011 */
									return 50;
								}
								else { /* 01010010 */
									return 49;
								}
							}
							else {			/* 0101000 */
								return 24;
							}
						}
					}
				}
				else {			/* 0100 */
					if (getbit(input)) {	/* 01001 */
						if (getbit(input)) { /* 010011 */
							if (getbit(input)) { /* 0100111 */
								return 18;
							}
							else {			/* 0100110 */
								if (getbit(input)) {	/* 01001101 */
									if (getbit(input)) {	/* 010011011 */
										return 1728;	/* make up */
									}
									else { /* 010011010 */
										return 1600;	/* make up */
									}
								}
								else { /* 01001100 */
									if (getbit(input)) {	/* 010011001 */
										return 1536;	/* make up */
									}
									else { /* 010011000 */
										return 1472;	/* make up */
									}
								}
							}
						}
						else {			/* 010010 */
							if (getbit(input)) { /* 0100101 */
								if (getbit(input)) { /* 01001011 */
									return 60;
								}
								else { /* 01001010 */
									return 59;
								}
							}
							else {			/* 0100100 */
								return 27;
							}
						}
					}
					else {			/* 01000 */
						return 11;
					}
				}
			}
		}
		else {			/* 00 */
			if (getbit(input)) {	/* 001 */
				if (getbit(input)) {	/* 0011 */
					if (getbit(input)) {	/* 00111 */
						return 10;
					}
					else {			/* 00110 */
						if (getbit(input)) { /* 001101 */
							if (getbit(input)) { /* 0011011 */
								if (getbit(input)) {	/* 0110111 */
									return 384; /* make up */
								}
								else { /* 0110110 */
									return 320; /* make up */
								}
							}
							else { /* 0011010 */
								if (getbit(input)) { /* 00110101 */
									return 0;
								}
								else { /* 00110100 */
									return 63;
								}
							}
						}
						else {			/* 001100 */
							if (getbit(input)) { /* 0011001 */
								if (getbit(input)) { /* 00110011 */
									return 62;
								}
								else { /* 00110010 */
									return 61;
								}
							}
							else {			/* 0011000 */
								return 28;
							}
						}
					}
				}
				else {			/* 0010 */
					if (getbit(input)) { /* 00101 */
						if (getbit(input)) { /* 001011 */
							if (getbit(input)) { /* 0010111 */
								return 21;
							}
							else {			/* 0010110 */
								if (getbit(input)) { /* 00101101 */
									return 44;
								}
								else { /* 00101100 */
									return 43;
								}
							}
						}
						else {			/* 001010 */
							if (getbit(input)) { /* 0010101 */
								if (getbit(input)) { /* 00101011 */
									return 42;
								}
								else { /* 00101010 */
									return 41;
								}
							}
							else { /* 0010100 */
								if (getbit(input)) { /* 00101001 */
									return 40;
								}
								else { /* 00101000 */
									return 39;
								}
							}
						}
					}
					else {			/* 00100 */
						if (getbit(input)) { /* 001001 */
							if (getbit(input)) { /* 0010011 */
								return 26;
							}
							else {			/* 0010010 */
								if (getbit(input)) { /* 00100101 */
									return 54;
								}
								else { /* 00100100 */
									return 53;
								}
							}
						}
						else {			/* 001000 */
							return 12;
						}
					}
				}
			}
			else {		/* 000 */
				if (getbit(input)) {	/* 0001 */
					if (getbit(input)) {	/* 00011 */
						if (getbit(input)) { /* 000111 */
							return 1;
						}
						else {			/* 000110 */
							if (getbit(input)) { /* 0001101 */
								if (getbit(input)) { /* 00011011 */
									return 32;
								}
								else { /* 00011010 */
									return 31;
								}
							}
							else {		/* 0001100 */
								return 19;
							}
						}
					}
					else {		/* 00010 */
						if (getbit(input)) { /* 000101 */
							if (getbit(input)) { /* 0001011 */
								if (getbit(input)) { /* 00010111 */
									return 38;
								}
								else { /* 00010110 */
									return 37;
								}
							}
							else { /* 0001010 */
								if (getbit(input)) { /* 00010101 */
									return 36;
								}
								else { /* 00010100 */
									return 35;
								}
							}
						}
						else {			/* 000100 */
							if (getbit(input)) { /* 0001001 */
								if (getbit(input)) { /* 00010011 */
									return 34;
								}
								else { /* 00010010 */
									return 33;
								}
							}
							else {			/* 0001000 */
								return 20;
							}
						}
					}
				}
				else {		/* 0000 */
					if (getbit(input)) { /* 00001 */
						if (getbit(input)) { /* 000011 */
							return 13;
						}
						else {			/* 000010 */
							if (getbit(input)) { /* 0000101 */
								if (getbit(input)) { /* 00001011 */
									return 48;
								}
								else { /* 00001010 */
									return 47;
								}
							}
							else {			/* 0000100 */
								return 23;
							}
						}
					}
					else {		/* 00000 */
						if (getbit(input)) { /* 000001 */
							if (getbit(input)) { /* 0000011 */
								return 22;
							}
							else {			/* 0000010 */
								if (getbit(input)) { /* 00000101 */
									return 46;
								}
								else { /* 00000100 */
									return 45;
								}
							}
						}
						else {			/* 000000 */
							if (getbit(input)) { /* 0000001 */
								if (getbit(input)) { /* 00000011 */
									return 30;
								}
								else { /* 00000010 */
									return 29;
								}
							}
							else { /* 0000000 */
								return commonmake(input);	/* seven zeros */
							}
						}
					}
				}
			}
		}
	}
	fprintf(stderr, "Impossible white run\n");
	return -1;	/* error */
}

/* Compiler screws up the following code if optimizations turned on */

int blackzero (FILE *input) {		/* black run code starts with four zeros */
/*	if (traceflag) printf (" blackzero entry "); */
	if (getbit(input)) { /* 00001 */
		if (getbit(input)) { /* 000011 */
			if (getbit(input)) { /* 0000111 */
				return 12;
			}
			else {			/* 0000110 */
				if (getbit(input)) {	/* 00001101 */
					if (getbit(input)) { /* 000011011 */
						if (getbit(input)) { /* 0000110111 */
							return 0;
						}
						else {			/* 0000110110 */
							if (getbit(input)) { /* 00001101101 */
								if (getbit(input)) { /* 000011011011 */
									return 43;
								}
								else {			/* 000011011010 */
									return 42;
								}
							}
							else {			/* 00001101100 */
								return 21;
							}
						}
					}
					else {			/* 000011010 */
						if (getbit(input)) { /* 0000110101 */
							if (getbit(input)) { /* 00001101011 */
								if (getbit(input)) { /* 000011010111 */
									return 39;
								}
								else {			/* 000011010110 */
									return 38;
								}
							}
							else {			/* 00001101010 */
								if (getbit(input)) { /* 000011010101 */
									return 37;
								}
								else {			/* 000011010100 */
									return 36;
								}
							}
						}
						else {			/* 0000110100 */
							if (getbit(input)) { /* 00001101001 */
								if (getbit(input)) { /* 000011010011 */
									return 35;
								}
								else { /* 000011010010 */
									return 34;
								}
							}
							else {			/* 00001101000 */
								return 20;
							}
						}
					}
				}
				else {			/* 00001100 */
					if (getbit(input)) {	/* 000011001 */
						if (getbit(input)) { /* 0000110011 */
							if (getbit(input)) { /* 00001100111 */
								return 19;
							}
							else {			/* 00001100110 */
								if (getbit(input)) { /* 000011001101 */
									return 29;
								}
								else {			/* 000011001100 */
									return 28;
								}
							}
						}
						else {			/* 0000110010 */
							if (getbit(input)) { /* 00001100101 */
								if (getbit(input)) { /* 000011001011 */
									return 27;
								}
								else {			/* 000011001010 */
									return 26;
								}
							}
							else {			/* 00001100100 */
								if (getbit(input)) { /* 000011001001 */
									return 192;	/* make up */
								}
								else {			/* 000011001000 */
									return 128;	/* make up */
								}
							}
						}
					}
					else {			/* 000011000 */
						return 15;
					}
				}
			}
		}
		else {			/* 000010 */
			if (getbit(input)) { /* 0000101 */
				return 11;
			}
			else {			/* 0000100 */
				return 10;
			}
		}
	}
	else {		/* 00000 */
		if (getbit(input)) { /* 000001 */
			if (getbit(input)) { /* 0000011 */
				if (getbit(input)) {	/* 00000111 */
					return 14;
				}
				else {			/* 00000110 */
					if (getbit(input)) { /* 000001101 */
						if (getbit(input)) { /* 0000011011 */
							if (getbit(input)) { /* 00000110111 */
								return 22;
							}
							else {			/* 00000110110 */
								if (getbit(input)) { /* 000001101101 */
									return 41;
								}
								else {			/* 000001101100 */
									return 40;
								}
							}
						}
						else {			/* 0000011010 */
							if (getbit(input)) {  /* 00000110101 */
								if (getbit(input)) { /* 000001101011 */
									return 33;
								}
								else {			/* 000001101010 */
									return 32;
								}
							}
							else {			/* 00000110100 */
								if (getbit(input)) { /* 000001101001 */
									return 31;
								}
								else {			/* 000001101000 */
									return 30;
								}
							}
						}
					}
					else {			/* 000001100 */
						if (getbit(input)) { /* 0000011001 */
							if (getbit(input)) { /* 00000110011 */
								if (getbit(input)) { /* 000001100111 */
									return 63;
								}
								else {			/* 000001100110 */
									return 62;
								}
							}
							else {			/* 00000110010 */
								if (getbit(input)) { /* 000001100101 */
									return 49;
								}
								else {			/* 000001100100 */
									return 48;
								}
							}
						}
						else {			/* 0000011000 */
							return 17;
						}
					}
				}
			}
			else {			/* 0000010 */
				if (getbit(input)) {	/* 00000101 */
					if (getbit(input)) { /* 000001011 */
						if (getbit(input)) { /* 0000010111 */
							return 16;
						}
						else {			/* 0000010110 */
							if (getbit(input)) { /* 00000101101 */
								if (getbit(input)) { /* 000001011011 */
									return 256;		/* make up */
								}
								else {			/* 000001011010 */
									return 61;
								}
							}
							else {			/* 00000101100 */
								if (getbit(input)) { /* 000001011001 */
									return 58;
								}
								else {			/* 000001011000 */
									return 57;
								}
							}
						}
					}
					else {			/* 000001010 */
						if (getbit(input)) { /* 0000010101 */
							if (getbit(input)) { /* 00000101011 */
								if (getbit(input)) { /* 000001010111 */
									return 47;
								}
								else {			/* 000001010110 */
									return 46;
								}
							}
							else {			/* 00000101010 */
								if (getbit(input)) { /* 000001010101 */
									return 45;
								}
								else {			/* 000001010100 */
									return 44;
								}
							}
						}
						else {			/* 0000010100 */
							if (getbit(input)) { /* 00000101001 */
								if (getbit(input)) { /* 000001010011 */
									return 51;
								}
								else {			/* 000001010010 */
									return 50;
								}
							}
							else {			/* 00000101000 */
								return 23;
							}
						}
					}
				}
				else {			/* 00000100 */
					return 13;
				}
			}
		}
		else {			/* 000000 */
			if (getbit(input)) { /* 0000001 */
				if (getbit(input)) { /* 00000011 */
					if (getbit(input)) { /* 000000111 */
						if (getbit(input)) { /* 0000001111 */
							return 64;	/* make up */
						}
						else {			/* 0000001110 */
							if (getbit(input)) { /* 00000011101 */
								if (getbit(input)) { /* 000000111011 */
									if (getbit(input)) { /* 0000001110111 */
										return 1216;
									}
									else {			/* 0000001110110 */
										return 1152;
									}
								}
								else {			/* 000000111010 */
									if (getbit(input)) { /* 0000001110101 */
										return 1088;
									}
									else {			/* 0000001110100 */
										return 1024;
									}

								}
							}
							else {			/* 00000011100 */
								if (getbit(input)) { /* 000000111001 */
									if (getbit(input)) { /* 0000001110011 */
										return 960;
									}
									else {			/* 0000001110010 */
										return 896;
									}
								}
								else {			/* 000000111000 */
									return 54;
								}
							}
						}
					}
					else {			/* 000000110 */
						if (getbit(input)) { /* 0000001101 */
							if (getbit(input)) { /* 00000011011 */
								if (getbit(input)) { /* 000000110111 */
									return 53;
								}
								else {			/* 000000110110 */
									if (getbit(input)) { /* 0000001101101 */
										return 576;	/*make up */
									}
									else {			/* 0000001101100 */
										return 512;	/* make up */
									}
								}
							}
							else {			/* 00000011010 */
								if (getbit(input)) { /* 000000110101 */
									return 448;	/* make up */
								}
								else {			/* 000000110100 */
									return 384;	/* make up */
								}
							}
						}
						else {			/* 0000001100 */
							if (getbit(input)) { /* 00000011001 */
								if (getbit(input)) { /* 000000110011 */
									return 320;	/* make up */
								}
								else {			/* 000000110010 */
									if (getbit(input)) { /* 0000001100101 */
										return 1728;
									}
									else {			/* 0000001100100 */
										return 1664;
									}
								}
							}
							else {			/* 00000011000 */
								return 25;
							}
						}
					}
				}
				else { /* 00000010 */
					if (getbit(input)) { /* 000000101 */
						if (getbit(input)) { /* 0000001011 */
							if (getbit(input)) { /* 00000010111 */
								return 24;
							}
							else {			/* 00000010110 */
								if (getbit(input)) { /* 000000101101 */
									if (getbit(input)) { /* 0000001011011 */
										return 1600;
									}
									else {			/* 0000001011010 */
										return 1536;
									}
								}
								else {			/* 000000101100 */
									return 60;
								}
							}
						}
						else {			/* 0000001010 */
							if (getbit(input)) { /* 00000010101 */
								if (getbit(input)) { /* 000000101011 */
									return 59;
								}
								else {			/* 000000101010 */
									if (getbit(input)) { /* 0000001010101 */
										return 1472;
									}
									else {			/* 0000001010100 */
										return 1408;
									}
								}
							}
							else {			/* 00000010100 */
								if (getbit(input)) { /* 000000101001 */
									if (getbit(input)) { /* 0000001010011 */
										return 1344;
									}
									else {			/* 0000001010010 */
										return 1280;
									}
								}
								else {			/* 000000101000 */
									return 56;
								}
							}
						}
					}
					else {			/* 000000100 */
						if (getbit(input)) { /* 0000001001 */
							if (getbit(input)) { /* 00000010011 */
								if (getbit(input)) { /* 000000100111 */
									return 55;
								}
								else {			/* 000000100110 */
									if (getbit(input)) { /* 0000001001101 */
										return 832;
									}
									else {			/* 0000001001100 */
										return 768;
									}
								}
							}
							else {			/* 00000010010 */
								if (getbit(input)) { /* 000000100101 */
									if (getbit(input)) { /* 0000001001011 */
										return 704;
									}
									else {			/* 0000001001010 */
										return 640;
									}
								}
								else {			/* 000000100100 */
									return 52;
								}
							}
						}
						else {			/* 0000001000 */
							return 18;
						}
					}
				}
			}
			else { /* 0000000 */
				return commonmake(input);	/* seven zeros */
			}
		}
	}
	fprintf(stderr, "Impossible black run (starting with four zeros)\n");
	return -1;	/* error */
}

/* blackrun may actually be OK with compiler optimizations turned on */

int blackrun (FILE *input) {
/*	if (traceflag) printf ("blackrun entry "); */
	if (getbit(input)) {	/* 1 */
		if (getbit(input)) {	/* 11 */
			return 2;
		}
		else {			/* 10 */
			return 3;
		}
	}
	else {			/* 0 */
		if (getbit(input)) {	/* 01 */
			if (getbit(input)) {	/* 011 */
				return 4;
			}
			else {			/* 010 */
				return 1;
			}
		}
		else {			/* 00 */
			if (getbit(input)) {	/* 001 */
				if (getbit(input)) {	/* 0011 */
					return 5;
				}
				else {			/* 0010 */
					return 6;
				}
			}
			else {		/* 000 */
				if (getbit(input)) {	/* 0001 */
					if (getbit(input)) {	/* 00011 */
						return 7;
					}
					else {		/* 00010 */
						if (getbit(input)) { /* 000101 */
							return 8;
						}
						else {			/* 000100 */
							return 9;
						}
					}
				}
				else {		/* 0000 */
					return blackzero(input);	/* four zeros */
				}
			}
		}
	}
	fprintf(stderr, "Impossible black run\n");
	return -1;	/* error */
}

#pragma optimize ("lge", on) 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int index, bitinx;	/* index into row of bytes and bits within them */

int total;			/* maybe long ? */

/* Following could be speeded up some ... */

int writewhite (unsigned char *lpBuffer, int run, int width) {
	if (run == 0) return 0;			/* nothing to do */
	if (run < 0) return -1;			/* hit invalid run */
	total += run;
/*	if (traceflag) printf("run %d total %d\n", run, total); */
	if (total > width) return -1;	/* error */
/*	just advance pointers */
	while (run > 0) {
		if (bitinx == 0) {
			while (run >= 8) {	
				index++;
/*				if (traceflag) printf("index %d run %d ", index, run); */
/*				*(lpBuffer+index) = 0; */	/* already zeroed out */
				run -= 8;
			}
		}
		if (run > 0) {
			if (bitinx-- <=  0) {
				index++; bitinx = 7;
			}
/*			if (traceflag) printf("index %d bitinx %d ", index, bitinx); */
/*			*(lpBuffer + index) &= ~(1 << bitinx); */
			run--;
		}
	}
	if (total == width) return 1;	/* EOL */
	else return 0;
}

int writeblack (unsigned char *lpBuffer, int run, int width) {
	if (run == 0) return 0;			/* nothing to do */
	if (run < 0) return -1;			/* hit invalid run */
	total += run;
/*	if (traceflag) printf("run %d total %d\n", run, total); */
	if (total > width) return -1;	/* error */
	while (run > 0) {
		if (bitinx == 0) {
			while (run >= 8) {	
				index++;
/*				if (traceflag) printf("index %d run %d ", index, run); */
				*(lpBuffer+index) = 255;	/* write a byte at a time */
				run -= 8;
			}
		}
		if (run > 0) {
			if (bitinx-- <=  0) {
				index++; bitinx = 7;
			}
/*			if (traceflag) printf("index %d bitinx %d ", index, bitinx); */
			*(lpBuffer + index) |= (1 << bitinx);	/* write a bit at a time */
			run--;
		}
	}
	if (total == width) return 1;	/* EOL */
	else return 0;
}

/* make width long ? */

int huffmanrow (unsigned char *lpBuffer, FILE *input, int width) {	
	int k, bytes;
	int run;
/*	int total = 0; */							/* long total ? */

/*	if (lpBuffer == NULL) {
		fprintf(stderr, "Bad buffer pointer\n");
		return -1;
	} */
	total = 0;
	index = -1; bitinx = 0;		/* output buffering */
	byte = 0; bitsleft = 0;		/* input buffering */

	bytes = (width + 7) / 8;	/* preset with zeros */
/*	if (traceflag) printf ("Cleaning out %d bytes\n", bytes); */
	for (k = 0; k < bytes; k++) lpBuffer[k] = 0;

	for (;;) {
/*		if (traceflag) printf("Looking for white run\n"); */
		while ((run = whiterun (input)) >= 64) {
/*			if (traceflag) printf(" W %d ", run); */
			if (writewhite(lpBuffer, run, width) < 0) break;
		}
		if (total >= width) break;
/*		if (traceflag) printf(" W %d\n", run); */
		if (writewhite(lpBuffer, run, width) != 0) break;	 /* terminal run */

/*		if (traceflag) printf("Looking for black run\n"); */
		while ((run = blackrun (input)) >= 64) {
/*			if (traceflag) printf(" B %d ", run); */
			if (writeblack(lpBuffer, run, width) < 0) break;
		}
		if (total >= width) break;
/*		if (traceflag) printf(" B %d\n", run); */
		if (writeblack(lpBuffer, run, width) != 0) break;	 /* terminal run */
	}
	if (total != width) {
		fprintf(stderr, "Sum of runs %d not equal width %d\n", total, width);
		return -1;
	}
/*	else if (traceflag ) printf("Sum of runs equal to width %d\n", width); */
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Here is the code for LZW (compression scheme number 5) */

#define MAXTABLE 4096					/* to deal with up to 12 bit codes */

#define MAXCHR 256						/* 2^8 possible bytes */

#define CLEAR 256						/* clear string table code */

#define EOD 257							/* end of data code */

#define FIRST 258						/* first code available - new string */

/* char *StringTable[MAXTABLE]; */		/* pointers to char strings */

/* char _far *StringTable[MAXTABLE]; */	/* pointers to char strings */

int _far *StringTable=NULL;				/* contains indeces to char strings */

int _far *StringLength=NULL;			/* contains lenghts of char strings */

int TableIndex=FIRST;					/* index into above String Table */

int CodeLength=9;						/* current code length */

/* Typically need about 20,000 bytes of string memory */

#define INIMEMORY 4000U				/* do this in stages ? realloc ? */

#define INCMEMORY 4000U				/* do this in stages ? realloc ? */

/* #define MAXMEMORY 42000U */		/* do this in stages ? realloc ? */

unsigned int FreeIndex=0;			/* next available byte in following */

/* char Memory[MAXMEMORY]; */		/* our own memory allocation */

char _far *Memory=NULL;				/* contains actual strings */
									/* based on index in StringTable */
									/* with length in StringLength */

unsigned int MemorySize;			/* remember size presently allocated */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void DeAllocStrings (void) {				/* remember to do this at end */
	if (Memory != NULL) {
		_ffree(Memory); Memory = NULL;
	}
	if (StringTable != NULL) {
		_ffree(StringTable); StringTable = NULL;
	}
	if (StringLength != NULL) {
		_ffree(StringLength); StringLength = NULL;
	}
}

void AllocStrings (void) {				
	if (Memory == NULL) {		/* memory for strings in string table */
/*		Memory = (char _far *) _fmalloc(MAXMEMORY); */
		Memory = (char _far *) _fmalloc(INIMEMORY);
		MemorySize = INIMEMORY;		/* initial allocation */
	}
	if (StringTable == NULL) {	/* allocate string table indeces */
		StringTable = (int _far *) _fmalloc(MAXTABLE * sizeof(unsigned int));
	}
	if (StringLength == NULL) {	/* allocate string table lengths */
		StringLength = (int _far *) _fmalloc(MAXTABLE * sizeof(unsigned int));
	}
	if (Memory == NULL || StringTable == NULL ||  StringLength == NULL) {
			fprintf(stderr, "ERROR: Unable to allocate memory\n");
			exit(1);
	}
}

int ExpandMemory (void) {			/* Try and get more space for strings */
	char _far *NewMemory;
	unsigned int NewMemorySize;

	if (MemorySize > 65535U - INCMEMORY) return -1;
	NewMemorySize = MemorySize + INCMEMORY;
	if (verboseflag) printf ("Growing Memory from %u to %u\n",
		MemorySize, NewMemorySize);
	NewMemory = (char _far *) _frealloc (Memory, NewMemorySize);
	if (NewMemory == NULL) {
		return -1;			/* old Memory and pointers still intact */
	}
	else {
		Memory = NewMemory;
		MemorySize = NewMemorySize;
		return 0;
	}
}

void InitializeStringTable (void) {		/* set up string table initially */
	int k;

	AllocStrings();					/* grab memory for tables if needed */
	for (k = 0; k < MAXCHR; k++) {	/* 256 single byte strings */
		Memory[k] = (char) k; 
		StringTable[k] = k;
		StringLength[k] = 1;
	}
/*  following not really needed */
/*	for (k = MAXCHR; k < MAXTABLE; k++)	{	
		StringTable[k] = 0;
		StringLength[k] = 0;
	} */
	FreeIndex = MAXCHR;
	TableIndex = FIRST;
	CodeLength = 9;
}

void ResetStringTable (int quietflag) {		/* clear string table */
/*	int k; */

	if (!quietflag) {
		if (verboseflag) printf("CLEAR ");
/*		if (traceflag) printf("\n"); */
		if (traceflag && TableIndex > 258)
			printf("TableIndex %d FreeIndex %u CodeLength %d\n",
				TableIndex, FreeIndex, CodeLength);
	}
/*  following not really needed */
/*	for (k = FIRST; k < TableIndex; k++) { 
		StringTable[k] = 0;
		StringLength[k] = 0;
	} */
	FreeIndex = MAXCHR;
	TableIndex = FIRST;
	CodeLength = 9;
}

/*				AddTableEntry(StringTable(OldCode)
					+ FirstChar(StringFromCode(Code);)); */
/*				AddNewEntry(OldCode, Code); */

/* Add a new entry to the string table equal to string specified by OldCode */
/* plus first character from next Code */

void AddNewEntry(int OldCode, int Code) {
	char _far *s;
	char _far *t;
	int k, len;

	len = StringLength[OldCode] + 1;			/* length of new string */
/*	if (traceflag)
		printf("Add string %d bytes, TableIndex %d FreeIndex %u\n",
			len, TableIndex, FreeIndex); */
	while (FreeIndex + len > MemorySize) {
		if (ExpandMemory() != 0) {
			fprintf(stderr, "Run out of string memory (%lu)\n", MemorySize);
			exit(1);
		}
	}
	StringTable[TableIndex] = FreeIndex;		/* enter new one in table */
	StringLength[TableIndex] = len;
	TableIndex++;
	s = Memory + FreeIndex;
	t = Memory + StringTable[OldCode];
	for (k = 0; k < len-1; k++) *s++ = *t++;	/* copy old string over */
	t = Memory + StringTable[Code];	
	*s = *t;					/* last byte comes from next code string */
	FreeIndex += len;

	if (TableIndex == 511 || TableIndex == 1023 || TableIndex == 2047) {  
		CodeLength++;
		if (traceflag) printf("LENGTH %d (%d) ", TableIndex, CodeLength);
	} 
	if (TableIndex > 4096) {
		fprintf(stderr, "Table overflow\n");
		exit(1); 
	}
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int IsInTable(int Code) {			/* is code already in table ? */
	return (Code >= 0 && Code < TableIndex);
}

/* copy string from table by code number */

unsigned char _far *WriteString (unsigned char _far *s, int Code) { 
	int k, len;
	char _far *t;

	t = Memory + StringTable[Code];
	len = StringLength[Code];
	for (k = 0; k < len; k++) *s++ = *t++;
	return s;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long OldByte=0;		/* cadaver being eaten */
int OldBitsLeft=0;				/* how many bits left in OldByte */

int GetNextCode (FILE *input) {	/* get next LZW code number from input */
	int bits;
	int c;
	unsigned long k;

	bits = OldBitsLeft;					/* how many bits do we have */
	k = OldByte;						/* start with old bits */
	
	while (bits < CodeLength) {
		if ((c = getc(input)) == EOF) { 
			fprintf(stderr, "Unexpected EOF (GetNextCode)\n");
			exit(1);
		} 
/*		c = GetNextByte(input); */
/*		if (c == -1) {
			fprintf(stderr, "Emergency Escape\n");
			return -1;		
		} */
		k = (k << 8) | c;
		bits += 8;
	}
	OldByte = k;
	OldBitsLeft = bits - CodeLength;		/* extra bits not used */
/*	OldByte = OldByte & ((1 << OldBitsLeft) - 1); */ /* redundant */
	k = k >> OldBitsLeft;					/* shift out extra bits */
	k = k & ((1 << CodeLength) - 1);		/* mask out high order */
/*	if (traceflag) printf("CODE %d ", k); */
	return (int) k;
}

void LZWdecompress (unsigned char _far *s, FILE *input) {
	int Code, OldCode;
	
	OldCode = 0;						/* to keep compiler happy */
/*	InitializeStringTable(); */			/* assume already done once */
/*	ResetStringTable(1); */				/* not needed - first code CLEAR */
	while ((Code = GetNextCode (input)) != EOD) {
		if (Code == -1) {
			fprintf(stderr, "Premature end of ZLW\n");
			return;
		}
/*		if (traceflag) printf("%d ", Code); */			/* debugging */
		if (Code == CLEAR) {
			ResetStringTable(0);
			Code = GetNextCode (input);
			if (Code == -1) {
				fprintf(stderr, "Premature end of ZLW (after CLEAR)\n");
				return;
			}
			if (Code == EOD) break;
			s = WriteString (s, Code);
			OldCode = Code;
		}								/* end of CLEAR case */
		else {
			if (IsInTable(Code)) {
/*				AddTableEntry(StringTable(OldCode)
					+ FirstChar(StringFromCode(Code);)); */
				AddNewEntry(OldCode, Code);
				s = WriteString(s, Code);
				OldCode = Code;
			}							/* end of Code in Table case */
			else {						/* Code is *not* in table */
/*				OutString = StringFromCode (OldCode) +
					+ FirstChar(StringFromCode(Code);));  */
				if (Code > TableIndex) {
					fprintf(stderr, "Code (%d) > TableIndex (%d) ",
						Code, TableIndex);
				}
/*				strcpy(Omega, StringFromCode(OldCode));
				ConcatOneChar(Omega, StringFromCode(OldCode));
				WriteString(Omega, output);
				AddTableEntry(Omega); */
				AddNewEntry(OldCode, OldCode); 
				s = WriteString(s, Code);
				OldCode = Code;
			} /* end of Code *not* in Table case */
		} /* end of *not* CLEAR case */
	} /* end of not EOD loop */
	if (verboseflag) printf("EOD ");
/*	reset table for next one */
	ResetStringTable(0);
	if (traceflag) printf("Now at byte %ld in file\n", ftell(input));
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***  */

/* Write a row of data using far pointer - used by LZW stripwise code */

/* int writearowfar (FILE *output, unsigned char _far *s, unsigned long width) {
	unsigned int c, d, old=0;
	int k, n;

	n = (int) width;
	column = 0;
	for (k = 0; k < n; k++) {
		if (column >= 39) {
			putc('\n', output);
			column = 0;
		}
		c = *s++;
		if (predictor == 2) {
			c = (c + old) & 255;
			old = c;
		}
		d = c & 15;
		c = c >> 4;
		if (c > 9) putc(c + 'A' - 10, output);
		else putc(c + '0', output);
		if (d > 9) putc(d + 'A' - 10, output);
		else putc(d + '0', output);
		column++;
	}
	putc('\n', output);
	column = 0;
	if (ferror(output)) {
		fputs("ERROR: Output error\n", stderr);
		return -1;
	}
	return 0;
} */

/* void expandcolorfar (unsigned char _far *lpBuffer, unsigned long width) {
	int k, n;
	unsigned int _far *PaletteRed;
	unsigned int _far *PaletteGreen;
	unsigned int _far *PaletteBlue;
	unsigned char _far *s;
	unsigned char _far *t;
	unsigned int red, green, blue, paletteindex;

	if (traceflag) printf("Expanding Palette Color\n");

	if (Palette == NULL) {
		fputs("ERROR: missing palette information\n", stderr);
		return;
	}
	PaletteRed = Palette;
	PaletteGreen = PaletteRed + PaletteSize;
	PaletteBlue = PaletteGreen + PaletteSize;

	n = (int) width;
	s = lpBuffer + width - 1; t = lpBuffer + width * 3 - 1;
	for (k = n-1; k >= 0; k--) {
		paletteindex = *s--;
		if (paletteindex < PaletteSize) {
			red = PaletteRed[paletteindex];
			green = PaletteGreen[paletteindex];
			blue = PaletteBlue[paletteindex];
		}
		else fprintf(stderr, "ERROR: Bad Palette Index %d >= %d\n",
			paletteindex, PaletteSize);
		*t-- = blue >> 8;
		*t-- = green >> 8;
		*t-- = red >> 8;
	}	
} */

/* gray = 0.3 * red + 0.59 * green + 0.11 * blue */

/* void compresscolorfar (unsigned char _far *lpBuffer, unsigned long width) {
	int k, n;
	unsigned char _far *s;
	unsigned char _far *t;
	unsigned int red, green, blue, gray;

	if (traceflag) printf("Compressing Color to Gray\n");
	s = lpBuffer; t = lpBuffer;
	n = (int) (width / 3);
	for (k = 0; k < n; k++) {
		red = *s++; green = *s++; blue = *s++;
		gray = (int) (((long) blue * 28 + (long) red * 77 + (long) green * 151)
			>> 8);
		*t++ = gray;
	}	
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Set up StripOffset & StripByteCount for strip number k */
/* and go to start of data for strip number k */
/* First one was already set up - don't disturb in case not indirect */
/* In practice, strips are usually contiguous */

void SetupStrip (FILE *input, int k) {
	if (k > 0) {
		StripOffset = indirectvalue(StripOffsetsType, 1,
			StripOffsetsPtr + k * typesize[StripOffsetsType], input);
		StripByteCount = indirectvalue(StripByteCountsType, 1,
			StripByteCountsPtr + k * typesize[StripByteCountsType], input);
	}
	if (traceflag) printf("Strip %d Offset %ld ByteCount %ld\n",
		k, StripOffset, StripByteCount);
	fseek (input, StripOffset, SEEK_SET);
}

/* Copy a row from far space used by LZW to near space used for output */

void CopyRow(unsigned char *lpBuffer, unsigned char _far *StripData,
		long InRowLength) { 
	int k, n;
	unsigned char *s=lpBuffer;
	unsigned char _far *t=StripData;
	n = (int) InRowLength;
	for (k = 0; k < n; k++) *s++ = *t++;
}

/* A whole strip is treated as one unit for LZW encoding ... */
/* So need to do once per strip and need memory for strip output */

int DecodeLZW (FILE *output, FILE *input, unsigned char *lpBuffer) {
	int k, row = 0, i, n, flag = 0;
	unsigned char _far *StripData;

	StripData = (unsigned char _far *)
/*		_fmalloc((unsigned int) (InRowLength * RowsPerStrip)); */
		_fmalloc((unsigned int) (BufferLength * RowsPerStrip));
	if (StripData == NULL) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(1);
	}
	InitializeStringTable(); 
	row = 0;
	for (k = 0; k < StripsPerImage; k++) {
		SetupStrip(input, k);
		OldByte = 0;	OldBitsLeft = 0;
		ResetStringTable(1);			/* redundant ? */
		LZWdecompress (StripData, input);
		n = RowsPerStrip;
		if (row + n > ImageLength) n = (int) (ImageLength - row);
		for (i = 0; i < n; i++) {
			CopyRow(lpBuffer, StripData + i * InRowLength, InRowLength);
			if (ProcessRow (output, lpBuffer, InRowLength, BufferLength,
				OutRowLength) != 0) break;
/*			if (bCompressColor)
				compresscolorfar(StripData + i * BufferLength, InRowLength);
			if (writearowfar (output, StripData + i * BufferLength,
				OutRowLength) != 0) {
				flag = 1;
				break;
			}
			if (flag) break; */
		}
		row += n;
	}
	ResetStringTable(1);			/* redundant ? */
	_ffree(StripData);
	return flag;
}

/* InitializeStringTable AllocString - may need to DeAllocString eventually */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int commandline (int argc, char *argv[], int firstarg) {
	char *s;
	
	while (firstarg < argc && *argv[firstarg] == '-') {
		s = argv[firstarg];
		if (strcmp(s, "-v") == 0) {
			verboseflag = ~verboseflag;
		}
		else if (strcmp(s, "-t") == 0) {
			traceflag = ~traceflag;
		}
		else if (strcmp(s, "-3") == 0) {	/* three color - RGB */
			bAllowColor = ~bAllowColor;
		}
		else fputs(s, stderr);
		firstarg++;
	}
	return firstarg;
}

/* #define DEBUGFLUSH */

int main (int argc, char *argv[]) {
	int firstarg = 1;

	if (argc < 2) showusage (argv);

	firstarg = commandline(argc, argv, firstarg);

	if (argc < firstarg + 1) showusage (argv);

	program = argv[0];

#ifdef DEBUGFLUSH
 	setbuf(stdout, NULL);  
#endif 

	if (tifftops (argv[firstarg], 1) != 0) exit(1);
	if ((m = _fheapchk ()) != _HEAPOK) {		/* 1994/Feb/18 */
		fprintf(stderr, "WARNING: Near heap corrupted (%d)\n", m);
	}
	return 0;
}

/* change long => int where sensible */

/* complain if samplesperpixel other than one */ /* no color support */
/* quit early (before writing header) if 24 bit image */

/* implement CCIT Huffman decompression also OK */
/* implement LZW decompression also */ /* need predictor also ? */

/* implement gray scale response curve ? */ /* although ignored by some apps */

/* II*  or MM*  followed by four byte offset to first IFD (often == 8) */

/* IFD: 2 byte count of fields, followed by 12 byte tag fields */
/* after that, four byte offset to next IFD (or zero) (or junk!) */

/* Field entries sorted in ascending order on tag: */

/* Byte 0 & 1: tag */
/* Byte 2 & 3: type */
/* Byte 4 - 7: `length' (better called `count') */
/* Byte 8 -11: value offset -- always even  (or value itself if it fits) */

/* if bAllowColor is false, maybe just use green component and `image' */
