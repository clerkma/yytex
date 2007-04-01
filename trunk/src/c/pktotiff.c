/* Copyright 1990, 1991, 1992 Y&Y, Inc.
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

/* Grovel through PK font files and produce TIFF image files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <conio.h>					/* for debugging user interaction */
/* #include <time.h> */					/* for CreationDate */

/* #define FNAMELEN 80 */
#define MAXCOMMENT 257
#define MAXCHARS 256
#define INFINITY 65535
#define SINFINITY 32767
#define NUMLIM 137
/* #define MAXPATHLEN 64 */		/* maximum path name length */
#define OUTPUTEXT "tif"
#define CONTROLBREAK 1
/* #define MAXLINE 256	*/		/* maximum length of input line (afm - tfm) */
/* #define MAXCOLUMNS 78 */
/* #define WANTCONTROLD 1 */

#define MAXBITLEN 16384		/* maximum number of bits per row */
#define MAXBYTELEN 2048		/* maximum number of bytes per row */

/* command op codes found in PK files: */

#define xxx1 240
#define xxx2 241
#define xxx3 242
#define xxx4 243
#define yyy 244
#define post 245
#define nop 246
#define pre 247

#define pk_id 89

int verboseflag = 0;	/* provide verbose comments */
int traceflag = 0;		/* more detail than you want */
int debugflag = 0;		/* output in hexadecimal */
int noiseflag = 1;		/* non-zero => messages about potential problems */
int detailflag = 0;		/* give detail in showusage */
int correctwidth = 0;	/* try and correct tfm widths */
int commentflag = 1; 	/* non-zero => add comment header to each char */

/* int uppercaseflag = 1; */	/* produce uppcase font name */
/* int fontdictsize=32;	*/ /* size of font dict to allocate */
/* int permanent = 1; */		/* non-zero => permanent downloading */
/* int showrows = 1; */		/* split hexadecimal output across rows */
/* int directprint=0; */		/* direct to printer if non-zero */

int copyflag;		/* non-zero if this character should be copied */
int warnflag;		/* warning about this character - need code */
int prepass;		/* non-zero means first pass -  suppress output */
int chrs;			/* current character working on */
int chrcount;		/* total characters seen */
int chrproc;		/* total characters actually processed */
int maxchrs;		/* highest character code seen */
int selectflag;		/* non-zero means character selection invoked */
int hexflag;		/* next arg is hexadecimal code for selected chars */
int rangeflag;		/* next arg is range of selected characters */
int stringflag;		/* next arg is selected characters */
int outputflag;		/* next arg is output destination */
int leftover; 		/* whats left over after removing one nybble */
int bitsleft;		/* whats left over after removing some bits */
int repeatcount;	/* how many times to repeat this row */

/* int rightend; */		/* max safe column number to go to output */

int x, y;			/* coordinates relative to top left */
int bitpos;		/* count of bits in accumulated in output byte */
int byte;		/* byte being accumulated for output */

/* int column; */		/* column used for hexadecimal output */

int fxll, fyll, fxur, fyur;	/* Font BBox */

int wantcpyrght = 1;

volatile int bAbort = 0;		/* set by use interrupt routine */

static  char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
static  FILE *input, *output;

char *programversion = "PKTOTIFF version 0.6";

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright="\
Copyright (C) 1990, 1991  Y&Y.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1990, 1991  Y&Y. All rights reserved. (978) 371-3286\ */

/* Copyright (C) 1991 Berthold K.P. Horn, Y&Y. All rights reserved.\ */

/* char programpath[MAXPATHLEN] = "c:\\dvipsone"; */
char programpath[FILENAME_MAX] = "c:\\yandy\\dvipsone";

char *outputfile = "";			/* output file or empty */

/* char *procsetrest = "dvifont3"; */

char *procsetpath = "";				/* preamble file path from argv[0] */

char *franticpath = "c:\\dvipsone";	/* emergency preamble file path */

/* char procsetfile[FILENAME_MAX]; */

/* static char line[MAXLINE]; */	/* buffer for compying procset */

static char row[MAXBITLEN];	/* copy of present row working on (for repeat) */
							/* lazy way, one byte per bit actually */

int charprocessed=0;		/* how many characters processed */

int currentchar;			/* character working on this time */

int rowpos;					/* pointer into rowout */

static unsigned char rowout[MAXBYTELEN];	/* current output row assembly */ 

static unsigned char runout[MAXBYTELEN];	/* runs in output row */ /* NEW */

int rowbytes;				/* total number of bytes in row */

static char comment[MAXCOMMENT];	/* was: comment from PK file */ 

static char fontname[FILENAME_MAX];		/* derived from file name */

static char wantchrs[MAXCHARS];		/* flags to indicate which chars wanted */

/* long ds, cs, hppp, vppp; */

unsigned long ds, cs;		/* design size (dvi units) and checksum  */

unsigned long hppp, vppp;	/* horizontal and vertical resolution */

int numchar=256;		/* if possible, obtain from file */ /* NA */

double ptsize;			/* design size computed from ds */ /* NA */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned int leastout=1;	/* non-zero => PC style, zero => Mac style */

unsigned int leastin=1;		/* non-zero => PC style, zero => Mac style */

unsigned int version=42;	/* TIFF version */

unsigned int ifdcount;		/* how many items in IFD */

unsigned int tagcount;		/* haw many actually written */

unsigned long ifdposition;	/* position of IFD */

int flipflag=0;

int invertflag=1;			/* invert black/white polarity */

int whimpflag=0;			/* try and accomodate Old Quark on Mac */

int stretchflag=0;

int resolution=0;			/* include xresolution and yresolution fields */

int numscale = 144;			/* XRES = YRES = numscale / denscale */
int denscale = 1;			/* XRES = YRES = resscale / denscale */

int iheight, iwidth;		/* remember to set up from h and v */

int bitspersample=1;

unsigned int mingrey=0, maxgrey=1;

unsigned long xrespos, yrespos, stripoffsets;

void abortjob(void);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void uwritetwo(unsigned long offset, FILE *output) {
	if (leastout != 0) {
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;
	}
	else {
		putc((int) ((offset >> 8) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 8) & 255), output); offset = offset << 8;
	}
}

void uwritefour(unsigned long offset, FILE *output) {
	if (leastout != 0) {
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;	
	}
	else {
		putc((int) ((offset >> 24) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 24) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 24) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 24) & 255), output); offset = offset << 8;	
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define TYPE_BYTE 1
#define TYPE_ASCII 2
#define TYPE_SHORT 3
#define TYPE_LONG 4
#define TYPE_RATIO 5

char *typename[6] = {"", "BYTE ", "ASCII", "SHORT", "LONG ", "RATIO"};

int typesize[6] = {0, 1, 1, 2, 4, 8};	/* units of length/count */

struct tagname {
	unsigned int tag; char *name;
};

#define MAXTAGS 512

#define MAXTAGNAME 25

#define LZW 5
#define PACKBITS 32773

struct tagname tagnames[MAXTAGS] = {
{254, "NewSubfileType"},
{255, "SubfileType"},
{256, "ImageWidth"},
{257, "ImageLength"},
{258, "BitsPerSample"},
{259, "Compression"},
{262, "PhotometricInterpretation"},
{263, "Threshholding"},
{264, "CellWidth"},
{265, "CellLength"},
{266, "FillOrder"},
{269, "DocumentName"},
{270, "ImageDescription"},
{271, "Make"},
{272, "Model"},
{273, "StripOffsets"},
{274, "Orientation"},
{277, "SamplesPerPixel"},
{278, "RowsPerStrip"},
{279, "StripByteCounts"},
{280, "MinSampleValue"},
{281, "MaxSampleValue"},
{282, "XResolution"},
{283, "YResolution"},
{284, "PlanarConfiguration"},
{285, "PageName"},
{286, "XPosition"},
{287, "YPosition"},
{288, "FreeOffsets"},
{289, "FreeByteCounts"},
{290, "GrayResponseUnit"},
{291, "GrayResponseCurve"},
{292, "Group3Options"},
{293, "Group4Options"},
{296, "ResolutionUnit"},
{297, "PageNumber"},
{301, "ColorResponseCurves"},
{305, "Software"},
{306, "DateTime"},
{315, "Artist"},
{316, "HostComputer"},
{317, "Predictor"},
{318, "White Point"},
{319, "ColorList"},
{319, "PrimaryChromaticities"},
{320, "ColorMap"},
{0, "Unknown"}
};

#define NEWSUBFILETYPE 254
#define SUBFILETYPE 255
#define IMAGEWIDTH 256
#define IMAGELENGTH 257
#define BITSPERSAMPLE 258
#define COMPRESSION 259
#define PHOTOMETRICINTERPRETATION 262
#define FILLORDER 266
#define DOCUMENTNAME 269
#define STRIPOFFSETS 273
#define ORIENTATION 274
#define SAMPLESPERPIXEL 277
#define ROWSPERSTRIP 278 
#define STRIPBYTECOUNTS 279
#define MINSAMPLEVALUE 280
#define MAXSAMPLEVALUE 281
#define XRESOLUTION 282
#define YRESOLUTION 283
#define PLANARCONFIGURATION 284
#define ARTIST 315

char *lookupname (unsigned int tag) {
	unsigned int n, k;
	n = sizeof(tagnames);
	for (k = 0; k < n; k++) {	
		if (tagnames[k].tag == 0) return tagnames[k].name;
		if (tagnames[k].tag == tag) return tagnames[k].name;
	}
	return "Unknown";
}

void writetagfield(unsigned int tag, unsigned int type, 
	unsigned long length, unsigned long offset, FILE *output) {

	uwritetwo(tag, output);
	uwritetwo(type, output);
	uwritefour(length, output);
	if (length ==1) {
		if (type == TYPE_LONG) uwritefour(offset, output);
		else if (type == TYPE_SHORT) {
			uwritetwo(offset, output);
			uwritetwo(0, output);
		}
		else if (type == TYPE_BYTE) {
			putc((int) offset, output);
			putc(0, output); putc(0, output); putc(0, output);
		}
		else  uwritefour(offset, output);
	}
	else uwritefour(offset, output);
	tagcount++;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void tiffheader(FILE *outfile) { /*  create header for TIFF file */
	char *s;
	int c;

	if (leastout != 0) {
		putc('I', outfile); 	putc('I', outfile);		/* leastout = 1 */
	}
	else {
		putc('M', outfile); 	putc('M', outfile);		/* leastout = 0 */
	}
	uwritetwo(version, outfile);		/* version number */
	if (whimpflag != 0) ifdposition = 2 + 2 + 4; 	/* right after header */
	else ifdposition = 2 + 2 + 4 + strlen(comment) + 1;
	uwritefour(ifdposition, outfile);
	if (whimpflag == 0) {
		s = comment;
		while((c = *s++) != '\0') putc(c, outfile);
		putc('\0', outfile);
	}

/*  recount */
	ifdcount = 13;							/* number of tag fields */
	if (whimpflag != 0) 
/*		ifdcount += (4 - 6); */	/* extra for whimpy format */
		ifdcount -= (6 - 4);	/* extra for whimpy format */
	if (resolution != 0) {
		ifdcount += 2;		/* extra for XRES & YRES tag fields */
		xrespos = ifdposition + 2 + ifdcount * 12 + 4;
		yrespos = ifdposition + 2 + ifdcount * 12 + 4 + 8;
	}
	stripoffsets =	ifdposition + 2 + ifdcount * 12 + 4;
	if (resolution != 0) stripoffsets += 2 * 8;	 /* space for two RATIOs */

	uwritetwo(ifdcount, outfile);
	tagcount = 0;
/*	FORMAT: writetagfield(tag, type, length, offset, outfile); */
	if (whimpflag != 0)
		writetagfield(SUBFILETYPE, TYPE_SHORT, 1, 1, outfile);		/* REQ */
	else 
		writetagfield(NEWSUBFILETYPE, TYPE_SHORT, 1, 0, outfile);	/* REQ */
	writetagfield(IMAGEWIDTH, TYPE_SHORT, 1, iwidth, outfile);		/* REQ */
	writetagfield(IMAGELENGTH, TYPE_SHORT, 1, iheight, outfile);	/* REQ */
	writetagfield(BITSPERSAMPLE, TYPE_SHORT, 1, bitspersample, outfile);
/*	if (whimpflag == 0) */
/*		writetagfield(COMPRESSION, TYPE_SHORT, 1, 1, outfile); */
		writetagfield(COMPRESSION, TYPE_SHORT, 1, PACKBITS, outfile);
	writetagfield(PHOTOMETRICINTERPRETATION, TYPE_SHORT, 1, 1, outfile);
	if (whimpflag != 0)
		writetagfield(FILLORDER, TYPE_SHORT, 1, 1, outfile);
	writetagfield(STRIPOFFSETS, TYPE_LONG, 1, stripoffsets, outfile); /* REQ */
	if (flipflag != 0) writetagfield(ORIENTATION, TYPE_SHORT, 1, 4, outfile);
	else writetagfield(ORIENTATION, TYPE_SHORT, 1, 1, outfile);
	writetagfield(SAMPLESPERPIXEL, TYPE_SHORT, 1, 1, outfile); 
	if (whimpflag == 0)
		writetagfield(ROWSPERSTRIP, TYPE_SHORT, 1, iheight, outfile);
	if (whimpflag != 0)
		writetagfield(STRIPBYTECOUNTS, TYPE_LONG, 1, 
			(long) iheight * ((iwidth * bitspersample + 7) / 8), outfile);
	if (stretchflag != 0 && bitspersample == 4) {
			writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, 0, outfile);
			writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, 15, outfile);
	}
	else if (stretchflag != 0 && bitspersample == 8) {
			writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, 0, outfile);
			writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, 255, outfile);
	}
	else if (whimpflag == 0) {
		writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, mingrey, outfile);
		writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, maxgrey, outfile);
	}
/*	XRESOLUTION & YRESOLUTION */
	if (resolution != 0) {
		writetagfield(XRESOLUTION, TYPE_RATIO, 1, xrespos, outfile);	
		writetagfield(YRESOLUTION, TYPE_RATIO, 1, yrespos, outfile);	
	}
	if (whimpflag != 0)
		writetagfield(PLANARCONFIGURATION, TYPE_SHORT, 1, 1, outfile);		
	if (whimpflag == 0)
		writetagfield(ARTIST, TYPE_ASCII, strlen(comment), 8, outfile);
	uwritefour(0, outfile);		/* indicate end of IFD fields */

	if (tagcount != ifdcount)
		fprintf(stderr, "IFDCOUNT (%u) <> TAGCOUNT (%u)\n",
			ifdcount, tagcount);

	if (resolution != 0) {
		uwritefour(numscale, outfile); uwritefour(denscale, outfile);
		uwritefour(numscale, outfile); uwritefour(denscale, outfile);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

struct ratio { /* structure for rational numbers */
	long numer;
	long denom;
};

/* compute good rational approximation to floating point number */
/* assumes number given is positive */

struct ratio rational(double fx, long nlimit, long dlimit) {
	long p0=0, q0=1, p1=1, q1=0, p2, q2;
	double s=fx, dq;
	struct ratio res;
/*	printf("Entering rational %g %ld %ld\n", fx, nlimit, dlimit);  */
	
	if (fx == 0.0) {
			res.numer = 0; res.denom = 1;
			return res;		/* the answer is 0/1 */
	}
	for(;;) {
		p2 = p0 + (long) s * p1;
		q2 = q0 + (long) s * q1;
/*		printf("%ld %ld %ld %ld %ld %ld %g\n", p0, q0, p1, q1, p2, q2, s);  */
		if (p2 > nlimit || q2 > dlimit || p2 < 0 || q2 <= 0) {
			p2 = p1; q2 = q1; break;
		}
		if ((double) p2 / (double) q2 == fx) break;
		dq = s - (double) ((long) s);
		if (dq == 0.0) break;
		p0 = p1; q0 = q1; p1 = p2; q1 = q2;	s = 1/dq;
/*		catch large s that will overflow when converted to long */
		if (s > 1000000000.0 || s < -1000000000.0) break; /* 1992/Dec/6 */
		if (bAbort != 0) abortjob();
	}
	res.numer = p2; res.denom = q2;
	return res;		/* the answer is p2/q2 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for reading signed and unsigned numbers of various lengths */

unsigned int ureadone (FILE *infile) {	/* NA */
	return getc(infile);
}

int sreadone (FILE *infile) {
	int c;
	c = getc(infile);
	if (c > 127) return (c - 256);
	else return c;
}

unsigned int ureadtwo (FILE *infile) {
	return (getc(infile) << 8) | getc(infile);
}

int sreadtwo (FILE *infile) {
	return (getc(infile) << 8) | getc(infile);
}

unsigned long ureadthree (FILE *infile) {
/*	int c, d, e; */
	unsigned int c, d, e;
	c = getc(infile);	d = getc(infile);	e = getc(infile);
	return ((((unsigned long) c << 8) | d) << 8) | e;
}

long sreadthree (FILE *infile) {			/* NA */
	int c, d, e;
	c = getc(infile);	d = getc(infile);	e = getc(infile);
	if (c > 127) c = c - 256; 
	return ((((long) c << 8) | d) << 8) | e;
}

unsigned long ureadfour (FILE *infile) {
/*	int c, d, e, f; */
	unsigned int c, d, e, f;
	c = getc(infile);	d = getc(infile);
	e = getc(infile);	f = getc(infile);
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
}

long sreadfour (FILE *infile) {
	int c, d, e, f;
	c = getc(infile);	d = getc(infile);
	e = getc(infile);	f = getc(infile);
	return ((((((long) c << 8) | (long) d) << 8) | e) << 8) | f;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for extracting bits and nybbles from input file */

int getbit(FILE *infile){
	int c;

	if (bitsleft-- <= 0)  {
		leftover = getc(infile);
		bitsleft = 7;
	}
	c = (leftover >> 7);		/* shift right of signed ... */
	leftover = leftover << 1; 
	return (c & 1);
}

int getnybble(FILE *infile){
	int c;

	if (leftover < 0) {
		c = getc(infile);
		leftover = c & 15;
		if (traceflag != 0) printf("N %d ", (c >> 4)); /* shift right signe */
		return (c >> 4);							/* shift right signe */
	}
	else {
		c = leftover;
		leftover = -1;
		if (traceflag != 0) printf("N %d ", c);
		return c;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*  The following does the PackBits compression */

/*  This sets up an array in which each byte marks how long a run it is in */

void setupruns(void) {
	int cb, cn, k, j, rep;

	rep = 0;
	cb = -1;
	for (k = 0; k < rowbytes; k++) {
		runout[k] = 0;
 		if ((cn = rowout[k]) != cb || rep >= 128) { /* end run */
			j = k-1;
			while (j >= 0 && runout[j] == 0) 
				runout[j--] = (unsigned char) rep;
			rep = 1;
			cb = cn;
		}
		else rep++;
	}
	j = rowbytes - 1;
	while (j >= 0 && runout[j] == 0) runout[j--] = (unsigned char) rep;
}

/*  This removes runs of length 2 that occur between runs of length 1 */

void removetwins(void) {
	int k;

	for (k = 1; k < rowbytes-2; k++) {
		if (runout[k] == 2 && runout[k-1] == 1 && runout[k+2] == 1) {
			runout[k] = 1; runout[k+1] = 1;
		}
	}
}

void showit(int c, FILE *output) { /* debugging code */
	fprintf(output, "%01X ", c);
} 

void showcount(int c, FILE *output) { /* debugging code */
	fprintf(output, "%d: ", c);
} 

void makepackbits(FILE *outfile) { /* This computes the PackBits codes */
	int j, n, k = 0;

/*	if (debugflag != 0)	putc('\n', outfile); */
	for(;;) {
		if (k >= rowbytes) break;
		if ((n = runout[k]) > 1) {		/* have a run here ? */
			if (n > 128) fprintf(stderr, "Run %d > 128\n", n);
			while (n > 128) {			/* shouldn't happen */
				if (debugflag != 0) showcount(-128 + 1, outfile); 
				else putc(-128 + 1, outfile);
				if (debugflag != 0) showit(rowout[k], outfile);
				else if (invertflag != 0) putc(~rowout[k], outfile);
				else putc(rowout[k], outfile);
				n -= 128;
			}
			if (debugflag != 0) showcount(-n + 1, outfile); 
			else putc(-n + 1, outfile);
			if (debugflag != 0) showit(rowout[k], outfile);
			else if (invertflag != 0) putc(~rowout[k], outfile);
			else putc(rowout[k], outfile);
			k += n;
		}
		else {	/* not a run - so copy straight */
			j = k;
			while (j < rowbytes && runout[j] == 1 && j < k + 128) j++;
			n = j - k;
			if (n > 128) fprintf(stderr, "Non run %d > 128\n", n);
			if (debugflag != 0) showcount(n - 1, outfile); 
			else putc (n - 1, outfile);
			while (k < j) {
				if (debugflag != 0) showit(rowout[k++], outfile);
				else if (invertflag != 0) putc(~rowout[k++], outfile);
				else putc(rowout[k++], outfile);
			}
		}
		if (bAbort != 0) abortjob();
	}
	if (debugflag != 0)	putc('\n', outfile);
}

/* void showarow(FILE *outfile) {
	int k;
	putc('\n', outfile);
	for(k = 0; k < rowbytes; k++) {
		putc(rowout[k], outfile);
	}
} */

/* void showruns(FILE *outfile) {
	int k=0, n;
	putc('\n', outfile);
	for(;;) {
		if (k >= rowbytes) break;
		n = runout[k];
		putc (n, outfile);
		putc (rowout[k], outfile);
		k += n;
	}
} */

void packbitsout(FILE *outfile) {
	setupruns();
	removetwins();
	makepackbits(outfile);
/*	putc('.', stdout); */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for writing bits into output file */

void writebit(FILE *outfile, int bit) {
/*	int c, d; */
	byte = (byte << 1) | bit;
	bitpos++;
	if (bitpos == 8) {
/*		c = (byte >> 4); d = byte & 15;
		if (c > 9) putc(c - 10 + 'A', outfile); 
		else putc(c + '0', outfile); 
		if (d > 9) putc(d - 10 + 'A', outfile); 
		else putc(d + '0', outfile); */
		rowout[rowpos++] = (unsigned char) byte;			/* NEW */
		bitpos = 0; byte = 0;
/*		column = column + 2;
		if (column >= MAXCOLUMNS) {
			putc('\n', outfile);
			column = 0;
		} */
	}
	return;
}

void setupbits(FILE *outfile) {	/* start of new row going out */
	bitpos = 0; byte = 0;
	rowpos = 0;				/* reset row position */
/*	column = 0; */
/*	fprintf(outfile, "charbitmaps %d ", chrs); */
/*	putc('<', outfile); */
/*	if (showrows != 0) 	putc('\n', outfile); */
}

void clearoutbits(FILE *outfile) { /* end of a row processing */
	while (bitpos != 0) writebit(outfile, 0);
	rowbytes = rowpos;				/* remember byte count */
/*	if (showrows != 0) {
		if (column >= rightend) { 
			putc('\n', outfile);
			column = 0;	
		} else {
			putc(' ', outfile); column++;
		} 
	} */
	packbitsout(outfile);	/* write the PackBits code */
	rowpos = 0;				/* reset row position ??? */
}

/* Now do all the PackBits coding an such */

void finishbits(FILE *outfile) {
/*	if (column > 0) {
		putc('\n', outfile); 
		column = 0;	 
	}
	putc('>', outfile); */
/*	fprintf(outfile, " put\n"); */
/*	putc('\n', outfile); */
/*	column = 0;	 */
/*	packbitsout(outfile);	*/	/* write the PackBits code */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* read number in hairy packed form use in PK files */

int readpackednum(FILE *infile, int dynf) {
	int i, j;
	i = getnybble(infile);
	if (i == 0) {
		for(;;) {
			j = getnybble(infile);
			i++;
			if (j != 0) break;
		}
		while (i > 0) {
			j = (j << 4) + getnybble(infile);
			i--;
		}
		return (j - 15 + ((13 - dynf) << 4) + dynf);
	}
	else if (i <= dynf) return(i);
	else if (i < 14) 
		return(((i - dynf - 1) << 4) + getnybble(infile) + dynf + 1);
	else {
		if (repeatcount != 0) {
			fprintf(stderr, "Second repeat count for this row!\n");
			exit(4);
		}
		if (i == 14) repeatcount = readpackednum(infile, dynf);
		else repeatcount = 1;	/* for i == 15 */
/*		printf("True %d ", repeatcount); */
		return readpackednum(infile, dynf);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Bitmapped format of character */

void readbits(FILE *infile, FILE *outfile, unsigned int w, unsigned int h) {
	unsigned int xx, yy;
	int bit;

	bitsleft = 0;
/*	rightend = MAXCOLUMNS - ((((w-1) >> 3) + 1) << 1) - 1;  */
	setupbits(outfile);
/*	if (verboseflag != 0) putc('\n', stdout); */
	for (yy = 0; yy < h; yy++)	{
		for (xx = 0; xx < w; xx++) {
			bit	= getbit(infile);
/*			if (verboseflag != 0) {
				if (bit != 0) putc('Z', stdout); 
				else putc('-', stdout);
				putc(' ', stdout);
			} */
			writebit(outfile, bit);
		}
/*		if (verboseflag != 0) putc('\n', stdout); */
		clearoutbits(outfile);
	}
	finishbits(outfile);
}

void spitbit(FILE *outfile, int black) {
	row[x] = (char) black;
/*	if (verboseflag != 0) {
		if (black != 0) putc('X', stdout);
		else putc('.', stdout);
		putc(' ', stdout);
	} */
	writebit(outfile, black);
}

/* repeat the previous row */

void repeatrow(FILE *outfile, int w) {
	int i, black;
	for(i = 0; i < w; i++) {
		black = row[i];
/*		if (verboseflag != 0) {
			if (black != 0) putc('Y', stdout);
			else putc(',', stdout);
			putc(' ', stdout); 
		} */
		writebit(outfile, black);
	} 
	clearoutbits(outfile);
/*	if (verboseflag != 0) putc('\n', stdout); */
	x = 0; y++;
}

/* spit out a run - and if needed a repeat of a row */

void spitout(FILE *outfile, int black, int count, int w) {
	while (count > 0) {
		spitbit(outfile, black);
		x++;
		if (x >= w) {
			x = 0; y++; 
			clearoutbits(outfile);
/*			if (verboseflag != 0) putc('\n', stdout); */
			while (repeatcount > 0) {
				repeatrow(outfile, w);
				repeatcount--;
			}
		}
		count--;
	}
}

/* read a character in hairy PK packed format */

void readpacked(FILE *infile, FILE *outfile, 
				int dynf, int black, int w, int h) {
	int count;

	leftover = -1;
/*	rightend = MAXCOLUMNS - ((((w-1) >> 3) + 1) << 1) - 1; */
	repeatcount = 0; 
	x = 0; y = 0;
	setupbits(outfile);
/*	if (verboseflag != 0) putc('\n', stdout); */
	for(;;) {
		count = readpackednum(infile, dynf);
		if (traceflag != 0) {
			printf("count %d (%d) ", count, black);
			if (repeatcount != 0) printf("repeat %d\n", repeatcount);
			(void) _getch();
		}
		spitout(outfile, black, count, w);
		black = 1 - black;
		if (traceflag != 0) printf("x %d y %d ", x, y);
/*		if (x == (w - 1) && y == (h - 1)) break; */
		if (x == 0 && y == h) break;
		if (y >= h) {
			printf("\nx %d y %d\n", x, y);
			(void) _getch();
			break;
		}
		if (bAbort != 0) abortjob();
	}	
	finishbits(outfile);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

void stripextension(char *fname) { /* strip ext if present */
	char *s;
	if ((s = strchr(fname, '.')) != NULL) {
		*s = '\0';
	}
}

#ifdef IGNORE
void extension(char *fname, char *ext) { /* supply ext if none present */
	if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change ext if one present */
	char *s;

    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}
#endif

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

/* First check whether path specified - if so use directly */
/* - then look in default header file directory */
/* within each directory, look for "dvifont3.enc", then "dvifont3.ps" */

/* void setupprocset(void) {	
	FILE *infile;

	if (strpbrk(procsetrest, "\\/:") != NULL) { 
		strcpy(procsetfile, procsetrest);
		if ((infile = fopen(procsetfile, "r")) != NULL) {
			fclose(infile); 
			return;
		}		
	}
	else {
		strcpy(procsetfile, procsetpath);	
		strcat(procsetfile, "\\");
		strcat(procsetfile, procsetrest);
		extension(procsetfile, "enc");		
		if ((infile = fopen(procsetfile, "r")) != NULL) {		
			fclose(infile); return;
		}		
		forceexten(procsetfile, "ps");		
		if ((infile = fopen(procsetfile, "r")) != NULL) {
			fclose(infile); return;
		}
		strcpy(procsetfile, franticpath);
		strcat(procsetfile, "\\");
		strcat(procsetfile, procsetrest);
		extension(procsetfile, "enc");		
		if ((infile = fopen(procsetfile, "r")) != NULL) {		
			fclose(infile); return;
		}		
		forceexten(procsetfile, "ps");		
		if ((infile = fopen(procsetfile, "r")) != NULL) {
			fclose(infile); return;
		}
	}
	return;
} */

/* void copyheader(FILE *outfile) {
	FILE *infile; 
	if ((infile = fopen(procsetfile, "r")) == NULL) {
		fprintf(stderr, "WARNING: Can't find prolog file ");
		perror(procsetfile); exit(1);
	}
	else {		
		(void) fgets(line, MAXLINE, infile); 	
		while (fgets(line, MAXLINE, infile) != NULL) fputs(line, outfile);
		fclose(infile);
	}
} */

void writeheader(FILE *outfile) {

	return;
/*  rewrite and/or remove this stuff in future */	
}

void writetrailer(FILE *outfile) {

	return;

/*  rewrite and/or remove this stuff in future */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void do_pre(FILE *infile, FILE *outfile) {
	int c, k, i;

	c = getc(infile);	/* i = 89 */
	if (c != pk_id) {
		fprintf(stderr, "Not a PK file\n");
		return;
	}	
	k = getc(infile);	/* k */
	for (i = 0; i < k; i++) {
		c = getc(infile);
		comment[i] = (char) c;
	}
	comment[k]='\0';

	ds = ureadfour(infile);
	cs = ureadfour(infile);	
	hppp = ureadfour(infile);
	vppp = ureadfour(infile);
/*	ptsize = ds; */ 
	ptsize = ((double) ds) / 1048576.0; 
	if (prepass != 0) return;
/*	if (verboseflag != 0) {
		printf("COMMENT: %s\n", comment);
		printf("DESIGN SIZE: %lg ", ((double) ds) / 1048576.0);
		printf("CHECKSUM: %lX ", cs); 
		printf("HPPP: %lg ", ((double) hppp) / 65536.0);
		printf("VPPP: %lg ", ((double) vppp) / 65536.0);
		putc('\n', stdout);		putc('\n', stdout);
	} */
	writeheader(outfile);
}

/* common subroutine to deal with * special * */

void do_xxx_sub(FILE *infile, FILE *outfile, unsigned long k) {
	unsigned long i;
	int c;

	for (i = 0; i < k; i++) {
		c = getc(infile);
/*		if (prepass == 0) putc(c, outfile); */
	}
/*	if (prepass == 0) putc('\n', outfile); */
}

void do_xxx1(FILE *infile, FILE *outfile) {
/*	if (prepass == 0) fprintf(outfile, "%% XXX1: ");  */
	do_xxx_sub(infile, outfile, (long) getc(infile));
}

void do_xxx2(FILE *infile, FILE *outfile) {
/*	if (prepass == 0) fprintf(outfile, "%% XXX2: ");  */
	do_xxx_sub(infile, outfile, (long) ureadtwo(infile));
}

void do_xxx3(FILE *infile, FILE *outfile) {
/*	if (prepass == 0) fprintf(outfile, "%% XXX3: ");  */
	do_xxx_sub(infile, outfile, ureadthree(infile));
}

void do_xxx4(FILE *infile, FILE *outfile) {
/*	if (prepass == 0) fprintf(outfile, "%% XXX4: ");  */
	do_xxx_sub(infile, outfile, ureadfour(infile));
}

void do_yyy(FILE *infile, FILE *outfile) {
	unsigned long k;
	k = ureadfour(infile);
/*	if (prepass == 0) fprintf(outfile, "%% YYY:  %lX\n", k);  */
}

void adjustbbox(int w, int h, int hoff, int voff) {
	int cxll, cyll, cxur, cyur;
	cxll = -hoff; 
	cyll = voff - h;
	cxur = w - hoff;
	cyur = voff;
	if (cxll < fxll) fxll = cxll;
	if (cyll < fyll) fyll = cyll;
	if (cxur > fxur) fxur = cxur;
	if (cyur > fyur) fyur = cyur;	
}

void commonform(FILE *infile, FILE *outfile, unsigned int cc, long tfm, 
			unsigned int w, unsigned int h, int hoff, int voff, 
				int	dynf, int black) {
	double width;
/*	double pwidth; */
	long numerwidth, denomwidth;
	struct ratio rwidth;
	
/*	chrproc++;	*/
	if (w > MAXBITLEN) {
		fprintf(stderr, "Width of character %d greater than %d\n", 
			w, MAXBITLEN);
		exit(13);
	}
	width = 1000.0 * ((double) tfm) / 1048576.0;
	if (correctwidth != 0) {
		rwidth = rational(width, 2000000, (long) NUMLIM); 
		numerwidth = rwidth.numer; denomwidth = rwidth.denom;
		width = ((double) numerwidth) / ((double) denomwidth);
	}
/*	pwidth = (width * hppp * ptsize) / 1000.0; */

/*	if (verboseflag != 0) {
		printf("width %lg ", width);
		printf("w %d h %d hoff %d voff %d dynf %d ", w, h, hoff, voff, dynf);
	}	*/
	
/*	if (commentflag != 0)  {
		fprintf(outfile, "width %lg ", width);
		fprintf(outfile, "w %d h %d hoff %d voff %d\n", w, h, hoff, voff);
	} */
	
	if (commentflag != 0) {
		sprintf(comment, "%s char %d width %lg w %d h %d hoff %d voff %d\n", 
			fontname, currentchar, width, w, h, hoff, voff); /* chrs */
	}
	
	iwidth = w; iheight = h;
	tiffheader(outfile);			/* This is it ! */
	
	adjustbbox(w, h, hoff, voff);
/*	fprintf(outfile, "/a%d %lg %d %d %d %d\n", 
			cc, width, w, h, hoff, voff); */

	if (dynf == 14) readbits(infile, outfile, w, h);
	else readpacked(infile, outfile, dynf, black, w, h);
		
/*	fprintf(outfile, " %d makechar\n", cc); */

/*	for (i = 0; i < pl - 8; i++) getc(infile);  skip over char - short */
/*	for (i = 0; i < pl - 13; i++) getc(infile); skip over char - long */

/*	if (verboseflag != 0) putc('\n', stdout); */

/*	putc('\n', outfile); */ /* ? */

}

void longform(FILE *infile, FILE *outfile, int lencat, int dynf, int black) {
	long hoff, voff;
	unsigned long dx, dy, w, h;
	int dm;					/* not accessed */
	unsigned long i;
	unsigned long pl, cc, tfm;

	pl = ureadfour(infile);		/* packet length */
	cc = ureadfour(infile);		/* character code */
	if (cc > INFINITY) {
		fprintf(stderr, "Ridiculously large char code %ld\n", cc);
	}
	chrs = (int) cc; chrcount++;
	if (chrs > maxchrs) maxchrs = chrs;
	copyflag = wantchrs[chrs];
	if (copyflag != 0) chrproc++;
	
/*	if (verboseflag != 0) printf("Long Form "); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0)
		fprintf(outfile, "%% Long Form "); */
	if (dynf == 14) {
/*		if (verboseflag != 0) printf("Bits ");		 */
/*		if (commentflag != 0 && prepass == 0 && copyflag != 0) 
			fprintf(outfile, "Bits "); */
	}
	if (lencat != 0) {		/* problem if this happens ? */
		fprintf(stderr, "WARNING: Lencat in long form not zero ");
		warnflag++;
	}

	if (warnflag != 0 && prepass == 0) {
		fprintf(stderr, "%ld\n", cc);
		warnflag = 0;
	}
	
/*	pl = (((long) lencat) << 16) + pl; */
/*	if (verboseflag != 0) printf("char %ld ", cc); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0) 
		fprintf(outfile, "char %ld ", cc); */

	if (prepass != 0 || copyflag == 0) {
		for (i = 0; i < pl; i++) (void) getc(infile);  /* skip over char */
		return;
	}

	tfm = ureadfour(infile);		/* horizontal escapement */
	dx = ureadfour(infile);			/* escapement in pixels * 2^16 */
	dy = ureadfour(infile);			/* escapement in pixels * 2^16 */
	w = ureadfour(infile);			/* width of bbox in pixels */
	h = ureadfour(infile);			/* height of bbox in pixels */
	hoff = sreadfour(infile);
	voff = sreadfour(infile);

	if ((w > INFINITY) || (h > INFINITY) || 
		(hoff > INFINITY) || (hoff < -INFINITY) ||
		(voff > INFINITY) || (voff < -INFINITY)) {
		fprintf(stderr, "Ridiculously large parameters in char %ld\n", cc);
	}
	if (dy != 0) {
		fprintf(stderr, "Can't handle vertical escapment dy %ld\n", dy);
	}
	dm = (int) (dx >> 16);

	commonform(infile, outfile, (unsigned int) cc, tfm, 
		(unsigned int) w, (unsigned int) h, (int) hoff, (int) voff,
			dynf, black);
}

void extendedform(FILE *infile, FILE *outfile, 
			int lencat, int dynf, int black) {
	int cc, hoff, voff;
	unsigned int dm;			/* not accessed */
	unsigned int w, h;
	long i, pl;
	unsigned long tfm;

	pl = (long) ureadtwo(infile);   /* packet length */
	cc = getc(infile);	/* character code */
	chrs = cc; chrcount++;
	if (chrs > maxchrs) maxchrs = chrs;
	copyflag = wantchrs[chrs];
	if (copyflag != 0) chrproc++;

/*	if (verboseflag != 0) printf("Extended Form "); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0)
		fprintf(outfile, "%% Extended Form "); */
	if (dynf == 14) {
/*		if (verboseflag != 0) printf("Bits ");	 */
/*		if (commentflag != 0 && prepass == 0 && copyflag != 0) 
			fprintf(outfile, "Bits "); */
	}
	if (warnflag != 0 && prepass == 0) {
		fprintf(stderr, "%d\n", cc);
		warnflag = 0;
	}
	
	pl = (((long) lencat) << 16) + pl;
/*	if (verboseflag != 0) printf("char %d ", cc); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0) 
		fprintf(outfile, "char %d ", cc); */

	if (prepass != 0 || copyflag == 0) {
		for (i = 0; i < pl; i++) (void) getc(infile);  /* skip over char */
		return;
	}

	tfm = ureadthree(infile);		/* horizontal escapement */
	dm = ureadtwo(infile);			/* escapement in pixels ? */
	w = ureadtwo(infile);			/* width of bbox in pixels */
	h = ureadtwo(infile);			/* height of bbox in pixels */
	hoff = sreadtwo(infile);
	voff = sreadtwo(infile);

	commonform(infile, outfile, cc, tfm, w, h, hoff, voff, dynf, black);
}

void shortform(FILE *infile, FILE *outfile, int lencat, int dynf, int black) {
	int i, pl, cc, w, h, hoff, voff;
	int dm;				/* not accessed */
	unsigned long tfm;

	pl = getc(infile);   /* packet length */
	cc = getc(infile);	/* character code */
	chrs = cc; chrcount++;
	if (chrs > maxchrs) maxchrs = chrs;
	copyflag = wantchrs[chrs];
	if (copyflag != 0) chrproc++;

/*	if (verboseflag != 0) printf("Short Form "); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0) 
		fprintf(outfile, "%% Short Form "); */
	if (dynf == 14) {
/*		if (verboseflag != 0) printf("Bits ");		 */
/*		if (commentflag != 0 && prepass == 0 && copyflag != 0) 
			fprintf(outfile, "Bits "); */
	}
	if (warnflag != 0 && prepass == 0) {
		fprintf(stderr, "%d\n", cc);
		warnflag = 0;
	}
	
	pl = (lencat << 8) + pl;
/*	if (verboseflag != 0) printf("char %d ", cc); */
/*	if (commentflag != 0 && prepass == 0 && copyflag != 0) 
		fprintf(outfile, "char %d ", cc); */

	if (prepass != 0 || copyflag == 0) {
		for (i = 0; i < pl; i++) (void) getc(infile);  /* skip over char */
		return;
	}

	tfm = ureadthree(infile);	/* horizontal escapement */
	dm = getc(infile);			/* escapement in pixels ? */
	w = getc(infile);			/* width of bbox in pixels */
	h = getc(infile);			/* height of bbox in pixels */
	hoff = sreadone(infile);
	voff = sreadone(infile);

/*	don't need escapement in pixels - dm ? */
	commonform(infile, outfile, cc, tfm, w, h, hoff, voff, dynf, black);
}

/* Unfortunately don't know character code yet to use in error message */

void do_char(FILE *infile, FILE *outfile, int flag) {
	int dynf, black, longflag, twobyte, lencat;

	warnflag = 0;
	dynf = flag >> 4;
	if (dynf > 14 || dynf < 0) {	/* basically impossible */
		if (noiseflag != 0 && prepass == 0) {
			fprintf(stderr, "WARNING: Invalid dynf value %d char ", dynf);
			warnflag++;
		}
	}
	if ((flag & 7) == 7) longflag = 1;	else longflag = 0;
	black = (flag >> 3) & 1;		/* zero for bitmapped chars */
	if (dynf == 14 && black != 0) {
		if (noiseflag != 0 && prepass == 0) { /* not serious */
			fprintf(stderr, "WARNING: Black run start for bitmap char "); 
			warnflag++;
		}
	}
	twobyte = (flag >> 2) & 1;		/* shift of signed quantity */
	lencat = flag & 3;
	if (lencat != 0) { /* normal, actually */
/*		if (noiseflag != 0 && prepass == 0) {
			fprintf(stderr, "WARNING: Lencat bits nonzero %d char ", lencat);
			warnflag++; 
		} */
	}
	if (longflag != 0) longform(infile, outfile, lencat, dynf, black);
	else if (twobyte != 0) extendedform(infile, outfile, lencat, dynf, black);
	else shortform(infile, outfile, lencat, dynf, black);
}

/* void showstatistics(void) {
	printf("Seen %d characters - highest code %d ", chrcount, maxchrs);
	if (chrproc != chrcount)
		printf("- processed %d characters ", chrproc);
	putc('\n', stdout);
	printf("FontBBox %d %d %d %d\n", fxll, fyll, fxur, fyur);
} */

void pkanal(FILE *infile, FILE *outfile) {
	int c;
	long place;
	
	prepass = 0;
	fxll = SINFINITY; fyll = SINFINITY; fxur = -SINFINITY; fyur = -SINFINITY;
	c = getc(infile);	/* pre = 247 */
	if (c != pre) {
		fprintf(stderr, "Not a PK file\n");
		return;
	}
	else do_pre(infile, outfile);
	chrcount = 0;
	chrproc = 0;
	maxchrs = 0;
	leftover = -1;
/*	skipflag = 0; */
	while ((c = getc(infile)) != EOF) {
		if (c < 240) {
			do_char(infile, outfile, c);
			if (chrproc > 0) break;		/* new escape clause ! */ 
		}
		else if (c == post) break;
		else if (c == nop) continue;
		else if (c == xxx1) do_xxx1(infile, outfile);
		else if (c == xxx2) do_xxx2(infile, outfile);
		else if (c == xxx3) do_xxx3(infile, outfile);
		else if (c == xxx4) do_xxx4(infile, outfile);
		else if (c == yyy) do_yyy(infile, outfile);		
		else {
			fprintf(stderr, "Unrecognized command code %d ", c);
			place = ftell(infile);
			fprintf(stderr, "at byte %ld in PK file\n", place);
			return;
		}
		if (bAbort != 0) abortjob();
	}
	if (c == EOF) {
		fprintf(stderr, "Premature EOF\n");
		exit(12);
	}
/*	if (verboseflag != 0) printf("Struck POST\n"); */
	writetrailer(outfile);
/*	if (verboseflag != 0) showstatistics(); */
}

void preanal(FILE *infile, FILE *outfile) { /* scan for character codes only */
	int c;
/*	long place; */
	
	prepass = -1;
/* 	fxll = SINFINITY; fyll = SINFINITY; fxur = -SINFINITY; fyur = -SINFINITY; */
	chrcount = 0;
	chrproc = 0;
	maxchrs = 0;
/*	leftover = -1; */
/*	skipflag = 0; */
	c = getc(infile);	/* pre = 247 */
	if (c != pre) {
/*		fprintf(stderr, "Not a PK file\n"); */
		return;
	}
	else do_pre(infile, outfile);
	while ((c = getc(infile)) != EOF) {
		if (c < 240) {
			do_char(infile, outfile, c);
		}
		else if (c == post) break;
		else if (c == nop) continue;
		else if (c == xxx1) do_xxx1(infile, outfile);
		else if (c == xxx2) do_xxx2(infile, outfile);
		else if (c == xxx3) do_xxx3(infile, outfile);
		else if (c == xxx4) do_xxx4(infile, outfile);
		else if (c == yyy) do_yyy(infile, outfile);		
		else {
/*			fprintf(stderr, "Unrecognized command code %d ", c);
			place = ftell(infile);
			fprintf(stderr, "at byte %ld in PK file\n", place); */
			return;
		}
		if (bAbort != 0) abortjob();
	}
/*	if (verboseflag != 0) printf("Struck POST\n"); */
/*	writetrailer(outfile); */
/*	if (verboseflag != 0) showstatistics(); */
}

void showusage(char *s) {
	fprintf (stderr, "\
%s [-{v}] [-a=\"<string>\"] [-r=<start>-<end>] [-x=<hexlist>] <pk-file>\n", 
	s);
	if (detailflag == 0) exit(1);
	fprintf (stderr, "\
\tv: verbose mode\n\
\ta: include characters in given string\n\
\tr: include characters in given range\n\
\tx: include characters specified by hexadecimal packed code\n\
\t   (default is to include all characters)\n");
	exit(1);
}

/* \tc: insert comment lines in output file\n\ */
/* \tl: do not force font name to be upper case\n\ */
/* \td: destination (PRN, LPT1 ... , AUX, COM1 ... , NUL or file),\n\ */
/* \t   (default is file in current directory with extension `ps')\n\ */

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': if (verboseflag != 0) traceflag = 1; else verboseflag = 1; 
			return 0; 
/*		case 'c': commentflag = 1; return 0;  */
/*		case 'l': uppercaseflag = 0; return 0;  */
		case 'x': hexflag = 1; return -1; 
		case 'r': rangeflag = 1; return -1; 
/*		case 's': stringflag = 1; return -1;  */
		case 'a': stringflag = 1; return -1; 
/*		case 'd': outputflag = 1; return -1;  */
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
/*	return 0;	*/ /* ??? */
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c, m, bchar, echar;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (rangeflag != 0) {
					if ((sscanf(s, "%d-%d", &bchar, &echar) < 2) 
						&& (sscanf (s, "%d", &echar) < 1)) {
							fprintf(stderr, "Don't understand range %s\n", s);
					}
					else {
						if (bchar < 0) bchar = 0;
						if (echar > (MAXCHARS-1)) echar = MAXCHARS-1;
						for (m = bchar; m <= echar; m++) wantchrs[m]++;
						selectflag++;
					}
					rangeflag = 0; 
				}
				else if (stringflag != 0) {
/*					m = *s++;		pick off delimiter */
					while ((c = *s++) != '\0') wantchrs[c]++;
					selectflag++; stringflag = 0; 
				}
				else if (hexflag != 0) {
					m = 0;
					while ((c = *s++) != '\0') {
						if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
						else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
						else if (c >= '0' && c <= '9') c = c - '0';
						else fprintf(stderr, "Bad hex character %c\n", c);
						wantchrs[m++] = (char) ((c >> 3) & 1);
						wantchrs[m++] = (char) ((c >> 2) & 1);
						wantchrs[m++] = (char) ((c >> 1) & 1);
						wantchrs[m++] = (char) ((c >> 0) & 1);
					}
					selectflag++; hexflag = 0; 
				}
				else if (outputflag != 0) {
 					outputfile = s;		outputflag = 0;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

/* remove file name - keep only path - inserts '\0' to terminate */
void removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}

void lowercase(char *t, char *s) {
	int c;
	while ((c = *s++) != '\0') {
		if (c >= 'A' && c <= 'Z') *t++ = (char) (c - 'A' + 'a');
		else *t++ = (char) c;
	}
	*t = '\0';
}

/* Sep 27 1990 => 1990 Sep 27 */

void scivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 7);
	for (k = 5; k >= 0; k--) date[k+5] = date[k];
/*	date[11] = '\0'; */
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

/* void uppercase(char *s, char *t) { 
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
} */

void stampit(FILE *outfile) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);	 
	fprintf(outfile, "%s (%s %s)\n", 
		programversion, date, compiletime);
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;

	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == 2670755) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
/*	(void) getch();  */
	return hash;
}

#ifdef CONTROLBREAK
void cleanup(void) {
	if (output != NULL) {
/*		if (directprint != 0) {
			putc(4, output); fclose(output);	
		}
		else { */
			fclose(output);	(void) remove(fn_out);
/*		} */
	}
/*	fcloseall();  */
}
#endif

void abortjob(void) {
	fprintf(stderr, "\nUser Interrupt - Exiting\n"); 
	cleanup();
/*	if (renameflag != 0) rename(fn_in, fn_bak); */
	exit(3);
}

#ifdef CONTROLBREAK
void ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
/*	cleanup(); */
/*	fprintf(stderr, "\nUser Interrupt"); */
	if (bAbort++ > 4) exit(3); 
/*	abort(); */
	(void) signal(SIGINT, ctrlbreak);
}
#endif

int nextwanted(void) { /* find next character desired */
	int k;
	for (k = 0; k < MAXCHARS; k++) {
		if (wantchrs[k] != 0) {
			return k;
		}
	}
	return -1;
}

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

#define MAXLINE 256

/* set up full file name for ini file and check for specified section */
/* e.g. dviwindo = setupinifile ("dviwindo.ini", "[Environment]") */

char *setupinifile (char *ininame, char *section) {	
	char fullfilename[FILENAME_MAX];
	FILE *input;
	char *windir;
	char line[MAXLINE];
	int m;

/*	Easy to find Windows directory if Windows runs */
/*	Or if user kindly set WINDIR environment variable */
/*	Or if running in Windows NT */	
	if ((windir = getenv("windir")) != NULL ||	/* 1994/Jan/22 */
		(windir = getenv("WINDIR")) != NULL ||
		(windir = getenv("winbootdir")) != NULL ||
		(windir = getenv("SystemRoot")) != NULL ||
 		(windir = getenv("SYSTEMROOT")) != NULL) { /* 1995/Jun/23 */
		strcpy(fullfilename, windir);
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);
	}
	else _searchenv (ininame, "PATH", fullfilename);

	if (strcmp(fullfilename, "") == 0) {		/* ugh, try standard place */
		strcpy(fullfilename, "c:\\windows");
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);		
	}

/*	if (*fullfilename != '\0') { */
/*	check whether ini file actually has required section */
		if ((input = fopen(fullfilename, "r")) != NULL) {
			m = strlen(section);
			while (fgets (line, sizeof(line), input) != NULL) {
				if (*line == ';') continue;
/*				if (strncmp(line, section, m) == 0) { */
				if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
					fclose(input);
					return _strdup(fullfilename);
				}
			}					
			fclose(input);
		}
/*	} */
	return "";							/* failed, for one reason or another */
}

int setupdviwindo (void) {
	if (usedviwindo == 0) return 0;		/* already tried and failed */
	if (*dviwindo != '\0') return 1;	/* already tried and succeeded */
/*	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");  */
	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");
	if (*dviwindo == '\0') usedviwindo = 0; /* failed, don't try this again */
	return (*dviwindo != '\0');
}

char *grabenvvar (char *varname, char *inifile, char *section, int useini) {
	FILE *input;
	char line[MAXLINE];
	char *s;
	int m, n;

	if (useini == 0 || *inifile == '\0')
		return getenv(varname);	/* get from environment */
	if ((input = fopen(inifile, "r")) != NULL) {
		m = strlen(section);
/* search for [...] section */
		while (fgets (line, sizeof(line), input) != NULL) {
			if (*line == ';') continue;
/*			if (strncmp(line, section, m) == 0) { */
			if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
/* search for varname=... */
				n = strlen(varname);
				while (fgets (line, sizeof(line), input) != NULL) {
					if (*line == ';') continue;
					if (*line == '[') break;
/*					if (*line == '\n') break; */	/* ??? */
					if (*line <= ' ') continue;		/* 95/June/23 */
/*					if (strncmp(line, varname, n) == 0 && */
					if (_strnicmp(line, varname, n) == 0 &&
						*(line+n) == '=') {	/* found it ? */
							fclose (input);
							/* flush trailing white space */
							s = line + strlen(line) - 1;
							while (*s <= ' ' && s > line) *s-- = '\0';
							if (traceflag)  /* DEBUGGING ONLY */
								printf("%s=%s\n", varname, line+n+1);
							return _strdup(line+n+1);
					}							
				}	/* end of while fgets */
			}	/* end of search for [Environment] section */
		}	/* end of while fgets */
		fclose (input);
	}	/* end of if fopen */
/*	useini = 0; */				/* so won't try this again ! need & then */
	return getenv(varname);	/* failed, so try and get from environment */
}							/* this will return NULL if not found anywhere */

char *grabenv (char *varname) {	/* get from [Environment] in dviwindo.ini */
	return grabenvvar (varname, dviwindo, "[Environment]", usedviwindo);
}

#ifdef NEEDATMINI
/* grab setting from `atm.ini' 94/June/15 */

/*	if (useatmini)  setupatmini(); 	*/ /* need to do this before use */

int useatmini = 1;			/* use [Setup] section in `atm.ini' */
							/* reset if setup of atm.ini file fails */

char *atmininame = "atm.ini";		/* name of ini file we are looking for */

char *atmsection = "[Setup]";		/* ATM.INI section */

char *atmini = "";				/* full file name for atm.ini with path */

int setupatmini (void) {
	if (useatmini == 0) return 0;		/* already tried and failed */
	if (*atmini != '\0') return 1;		/* already tried and succeeded */
/*	atmini = setupinifile ("atm.ini", "[Setup]");  */
	atmini = setupinifile (atmininame, atmsection);
	if (*atmini == '\0') useatmini = 0;	/* failed, don't try this again */
	return (*atmini != '\0');
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {		/* main program entry point */
	char *s;
	int m;
	int firstarg = 1;

#ifdef CONTROLBREAK
	(void) signal(SIGINT, ctrlbreak);
#endif

/*	program = argv[0]; */					/* not accessed */

	strncpy(programpath, argv[0], sizeof(programpath)); 
	removepath(programpath);

/*	if programpath exists, use as default for procset */

	if (strcmp(programpath, "") != 0) procsetpath = programpath;	

	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("PREPATH")) != NULL) procsetpath = s; 
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("PREPATH")) != NULL) procsetpath = s;
	}

	selectflag = 0;

	for (m = 0; m < MAXCHARS; m++) wantchrs[m] = 0;
	
/*	if (argc < 2) showusage(argv[0]); */

	firstarg = commandline(argc, argv, 1);

    if (firstarg > argc -1) showusage(argv[0]);

/*	scivilize(compiledate);	 */
	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright); 
	printf("\n");

	if (selectflag == 0) {
/*		for (m = 0; m < MAXCHARS; m++) wantchrs[m]++;	 */
		fprintf(stderr, "No character selected for processing\n");
		exit(1);
	}

/*	if ((s = getenv("PREPATH")) != NULL) procsetpath = s;   */

/*	setupprocset();  */

	if (checkcopyright(copyright) != 0) exit(1);

	if (whimpflag != 0) leastout=0;				/* Mac style output */
	if (whimpflag != 0) resolution=1;			/* XRES & YRES */

	m = firstarg;
	for(;;) {
/*	for (m = firstarg; m < argc; m++) { */
		strcpy(fn_in, argv[m]);
		extension(fn_in, "pk");
/*		printf("Processing PK file %s\n", fn_in); */

		if ((currentchar = nextwanted()) < 0) break;

		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
		strcpy (fontname, s);   	/* copy input file name minus path */
		stripextension(fontname);
/*		if (uppercaseflag != 0) uppercase(fontname, fontname); */
		
		if ((input = fopen(fn_in, "rb")) == NULL) {
			perror(fn_in); exit(2);
		}

		strcpy(fn_out, fontname);
		if ((s = strpbrk(fn_out, "0123456789")) != NULL) 
			*s = '\0';				/* flush numeric codes */
		fn_out[5] = '\0';			/* in case fontname too long */
		s = fn_out + strlen(fn_out);
		sprintf(s, "%03d", currentchar);
		extension(fn_out, "tif");

		if ((output = fopen(fn_out, "wb")) == NULL) {
			perror(fn_out); exit(2);
		}
		
		preanal(input, output);
		numchar = maxchrs + 1;

		rewind(input);
		pkanal(input, output);

		fclose(input);
/*		if (directprint == 0) { */
			if (ferror(output) != 0) {
				perror(fn_out); exit(7);
			}
			else fclose(output);		
/*		} else  */
/*		permanent = 0; */ /* to avoid exiting again */
			wantchrs[currentchar] = 0;
			charprocessed++;
		if (verboseflag != 0) putc('*', stdout);	/* indicate progress */
		if (bAbort != 0) abortjob();
	}
	
	if (ferror(output) != 0) {
		perror(fn_out); exit(7);
	}
	else fclose(output);		

/*	if (argc > firstarg +1) 
		printf("Processed %d PK files\n", argc - firstarg); */
	if (charprocessed > 1) 
		printf("Processed %d characters\n", charprocessed); 
	return 0;
}

/* c:\pctex\pixel\dpi300\cmr10.pk */

/* c:\tex\cmpixel\canon300\dpi1075\cmssi8.pk */

/* will need stuff from FONTONE to correct character widths ? */

/* will need stuff from FONTONE to get good ratios ? */

/* do something with:

% XXX1: identifier CMR
% XXX1: codingscheme TeX text
% XXX1: fontfacebyte
% YYY:  EA0000

*/

/* Accumulate font bounding box information as you go OK - first pass */

/* Set FontName from filename OK */

/* Leave width in TFM units ? */

/* do something about vertical escapments */

/* add remapping 0--31 to above 160 ? */

/* calculate numchar from max of code ? need two passes then OK */

/* don't insert \n before '>' if already at column zero OK */

/* make common parts - of short and extended and long - a subroutine OK */

/* pre calculate terms in FontMatrix ? */

/* add first pass that just extracts cc's to calculate numchar, chrcount OK */

/* check alignment of characters with respect to baseline */

/* add FontInfo dictionary ? */

/* do something about UniqueID ? */

/* command line arguments ? */

/* showusage ? */

/* check for EOF in input ? */

/* check for ferror after every font character on output ? */

/* remove exit() so can process multiple files even if one crashes */

/* need exitserver code up front OK */

/* fix line overflow when line is just full ? */

/* provide for command line specification of font name ? */

/* encrypt prolog ? */

/* optionally add control-D to end ? */

/* should there be a snapto to avoid too fat output ? Test on ALW ? */

/* set PREPATH if needed to point to c:\dvipsone */

/* add comment containing the code of wantchrs ? */

/* allow output direct to printer ? */

/* deal with lower case LPT, COM, PRN, EPT ? */

/* this does not correct inaccurate char escapments in PK file */

/* this uses simple numeric encoding scheme a0, a1, ... */

