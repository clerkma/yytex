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

/* Program to convert IMG file to TIFF file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #define FNAMELEN 128 */

unsigned int leastout=1;	/* non-zero => PC style, zero => Mac style */

unsigned int leastin=1;		/* non-zero => PC style, zero => Mac style */

unsigned int version=42;	/* TIFF version */

unsigned int ifdcount;		/* how many items in IFD */

unsigned int tagcount;		/* haw many actually written */

unsigned long ifdposition;	/* position of IFD */

int verboseflag=0;

int binaryflag = 0;			/* convert to monochrome if set */

int stretchflag=0;			/* stretch greylevels using mingrey & maxgrey */

int flipflag=0;

int whimpflag=0;			/* try and accomodate Old Quark on Mac */

int resolution=1;			/* include xresolution and yresolution fields */

int nasaflag=0;				/* NASA CD format */

/* int numscale = 144; */	/* XRES = YRES = numscale / denscale */
int numscale = 188;			/* XRES = YRES = numscale / denscale */
int denscale = 1;			/* XRES = YRES = resscale / denscale */

int addleft = 0;
int addright = 0;
int addtop = 0;
int addbottom = 0;

int stripeflag = 0;			/* add black stripe at bottom */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned int ureadtwo(FILE *input) {
	unsigned int c, d;
	c = (unsigned int) getc(input);
	d = (unsigned int) getc(input);	
	if (leastin != 0) return ((d << 8) | c);
	else return ((c << 8) | d);
}

unsigned long ureadfour(FILE *input) {
	unsigned int c, d, e, f;
	c = (unsigned int) getc(input);
	d = (unsigned int) getc(input);	
	e = (unsigned int) getc(input);	
	f = (unsigned int) getc(input);		
	if (leastin != 0) return ((((((f << 8) | e) << 8) | d) << 8) | c);
	else return ((((((c << 8) | d) << 8) | e) << 8) | f);
}

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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

void showfields(FILE *input, unsigned long ifdpos) {
	unsigned int j, k, n, tag, type;
	unsigned long length, offset;
	char *s;

	if(fseek(input, (long) ifdpos, SEEK_SET) != 0) {
		printf("SEEK ERROR %lu\n", ifdpos);
		return;
	}
	ifdcount = ureadtwo(input);
	
	for (k = 0; k < ifdcount; k++) {
		tag = ureadtwo(input);
		type = ureadtwo(input);
		length = ureadfour(input);	/* count */
		offset = ureadfour(input);	/* value */

		printf("TAG:%5u ", tag);
		s = lookupname(tag);
		printf ("(%s) ", s);
		n = strlen(s);
		for (j = n; j < MAXTAGNAME; j++) putc(' ', stdout);
		printf ("TYPE:%2u ", type);
		if (type <= 5) printf("(%s) ", typename[type]);
		else printf("     ");
		printf("LENGTH:%3lu ", length);
		printf("OFFSET:%6lu", offset);
		printf("\n");
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
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

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int headlen, textoffset;
int iwidth, iheight, bitspersamplein;
unsigned int mingrey, maxgrey, avergrey;
int bitspersampleout;

void blankrow (FILE *output, int value) {
	int i, n;
	unsigned int outbyte;
/*	int bitcount, color; */

/*	if (value == 0) color = 1;
	else color = 0;
	outbyte = 0;
	bitcount = 0; */
/*	for (i = 0; i < n; i++) {
		outbyte = (outbyte << 1) | color;
		bitcount++;
		if ((bitcount % 8) == 0) putc(outbyte, output);
	} */
/*	if (bitcount != 0) outbyte = outbyte << (8 - bitcount); */
/*	putc(outbyte, output); */

	n = (iwidth + 7) / 8;
	if (value == 0) outbyte = (unsigned int) -1;
	else outbyte = (unsigned int) 0;
	for (i = 0; i < n; i++) putc(outbyte, output);
}

/* works for 8-bit grey only right now */
/* assumes packing bits in left to right for now */

void convertmono(FILE *output, FILE *input) {
	int i, j;
	unsigned int grey;
	unsigned int outbyte;
	int bitcount;

	for (j = 0; j < addtop; j++) blankrow(output, 0);

	for (j = 0; j < iheight - addtop - addbottom; j++) {
		outbyte = 0; bitcount = 0;
		for (i = 0; i < addleft; i++) {
			outbyte = (outbyte << 1) | 1;
			bitcount++;
			if ((bitcount % 8) == 0) putc(outbyte, output);
		}
		for (i = 0; i < iwidth - addleft - addright; i++) {
			grey = getc(input);
			if (grey > avergrey) outbyte = (outbyte << 1) | 1;
			else outbyte = (outbyte << 1);
			bitcount++;
			if ((bitcount % 8) == 0) putc(outbyte, output);
		}
		for (i = 0; i < addright; i++) {
			outbyte = (outbyte << 1) | 1;
			bitcount++;
			if ((bitcount % 8) == 0) putc(outbyte, output);
		}
/*		if (bitcount != 0) outbyte = outbyte << (8 - bitcount); */
/*		putc(outbyte, output); */
		if ((bitcount % 8) != 0)  {
			for (i = bitcount; (i % 8) != 0; i++) {
				outbyte = (outbyte << 1) | 1;
				bitcount++;
				if ((bitcount % 8) == 0)  {
					putc(outbyte, output);
					break;
				}
			}
		}
	}
	if (stripeflag == 0 || addbottom == 0)
		for (j = 0; j < addbottom; j++) blankrow(output, 0);
	else {
		for (j = 0; j < addbottom-1; j++) blankrow(output, 0);
		blankrow(output, 1);
	}
}

/* The IMAGE file header contains the following:
 * the characters 'IMG'
 * a byte giving the total number of bytes in the header (64)
 * a byte giving the position within the header of the title string (16)
 * two bytes giving the number of rows in the image (low order first)
 * two bytes giving the number of columns in the image (low order first)
 * a byte giving the number of bits per pixel (8 maximum)
 * a byte giving the minimum grey level (0)
 * a byte giving the maximum grey level (255)
 * a byte containing various flags (unused so far)
 * a series of null bytes up to (reserved for future expansion)
 * the null-terminated ASCII title (up to MAXTITLE in length)
 * a series of null bytes to pad out the header
 */

unsigned long xrespos, yrespos, stripoffsets;

char comment[64]="";

void writetiffheader(FILE *output) {	/*  create header for TIFF file */
	char *s;
	int c;
	
	if (leastout != 0) {
		putc('I', output); 	putc('I', output);		/* leastout = 1 */
	}
	else {
		putc('M', output); 	putc('M', output);		/* leastout = 0 */
	}
	uwritetwo(version, output);		/* version number */
	if (whimpflag != 0) ifdposition = 2 + 2 + 4; 	/* right after header */
	else ifdposition = 2 + 2 + 4 + strlen(comment) + 1;
	uwritefour(ifdposition, output);
	if (whimpflag == 0) {
		s = comment;
		while((c = *s++) != '\0') putc(c, output);
		putc('\0', output);
	}

	ifdcount = 13;							/* number of tag fields */
/*	if (whimpflag != 0) ifdcount += 4 - 6; */	/* extra for whimpy format */
	if (whimpflag != 0) ifdcount -= 6 - 4; 	/* extra for whimpy format */
	if (resolution != 0) {
		ifdcount += 2;		/* extra for XRES & YRES tag fields */
		xrespos = ifdposition + 2 + ifdcount * 12 + 4;
		yrespos = ifdposition + 2 + ifdcount * 12 + 4 + 8;
	}
	stripoffsets =	ifdposition + 2 + ifdcount * 12 + 4;
	if (resolution != 0) stripoffsets += 2 * 8;	 /* space for two RATIOs */

	uwritetwo(ifdcount, output);
	tagcount = 0;
/*	writetagfield(tag, type, length, offset, output); */
	if (whimpflag != 0)
		writetagfield(SUBFILETYPE, TYPE_SHORT, 1, 1, output);		/* REQ */
	else 
		writetagfield(NEWSUBFILETYPE, TYPE_SHORT, 1, 0, output);	/* REQ */
	writetagfield(IMAGEWIDTH, TYPE_SHORT, 1, iwidth, output);		/* REQ */
	writetagfield(IMAGELENGTH, TYPE_SHORT, 1, iheight, output);		/* REQ */
	writetagfield(BITSPERSAMPLE, TYPE_SHORT, 1, bitspersampleout, output);
	if (whimpflag == 0)
		writetagfield(COMPRESSION, TYPE_SHORT, 1, 1, output);
	writetagfield(PHOTOMETRICINTERPRETATION, TYPE_SHORT, 1, 1, output);
	if (whimpflag != 0)
		writetagfield(FILLORDER, TYPE_SHORT, 1, 1, output);
	writetagfield(STRIPOFFSETS, TYPE_LONG, 1, stripoffsets, output); /* REQ */
	if (flipflag != 0) writetagfield(ORIENTATION, TYPE_SHORT, 1, 4, output);
	else writetagfield(ORIENTATION, TYPE_SHORT, 1, 1, output);
	writetagfield(SAMPLESPERPIXEL, TYPE_SHORT, 1, 1, output); 
	if (whimpflag == 0)
		writetagfield(ROWSPERSTRIP, TYPE_SHORT, 1, iheight, output);
	if (whimpflag != 0)
		writetagfield(STRIPBYTECOUNTS, TYPE_LONG, 1, 
			(long) iheight * ((iwidth * bitspersampleout + 7) / 8), output);
	if (binaryflag != 0) {				/* ??? */
			writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, 0, output);
			writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, 1, output);
	}
	else if (stretchflag != 0 && bitspersampleout == 4) {
			writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, 0, output);
			writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, 15, output);
	}
	else if (stretchflag != 0 && bitspersampleout == 8) {
			writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, 0, output);
			writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, 255, output);
	}
	else if (whimpflag == 0) {
		writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, mingrey, output);
		writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, maxgrey, output);
	}
/*	XRESOLUTION & YRESOLUTION */
	if (resolution != 0) {
		writetagfield(XRESOLUTION, TYPE_RATIO, 1, xrespos, output);	
		writetagfield(YRESOLUTION, TYPE_RATIO, 1, yrespos, output);	
	}
	if (whimpflag != 0)
		writetagfield(PLANARCONFIGURATION, TYPE_SHORT, 1, 1, output);		
	if (whimpflag == 0)
		writetagfield(ARTIST, TYPE_ASCII, strlen(comment), 8, output);
	uwritefour(0, output);		/* indicate end of IFD fields */

	if (tagcount != ifdcount)
		fprintf(stderr, "IFDCOUNT (%u) <> TAGCOUNT (%u)\n",
			ifdcount, tagcount);

	if (resolution != 0) {
		uwritefour(numscale, output); uwritefour(denscale, output);
		uwritefour(numscale, output); uwritefour(denscale, output);
	}

}

void readimghead(FILE *input) { /*	first read IMG file header */
	int c, d, e, k;
	char *s;
	
	if (nasaflag == 0) {
	c = getc(input); d = getc(input); e = getc(input);
	if (c == 'I' && d == 'M' && e == 'G') ;
	else {
		printf("File starts with `%c%c%c' instead of `IMG''\n", c, d, e);
		exit(3);
	}
	headlen =  getc(input);
	if (headlen != 64) printf("Head length not 64\n");
	textoffset = getc(input);
	if (textoffset != 16) printf("Text offset not 16\n");	

	iheight = ureadtwo(input);
	iwidth  = ureadtwo(input);
	bitspersamplein = getc(input);

	if (binaryflag != 0) {
		if (addleft > 0 || addright > 0) {
			printf("Extending width from %d to %d\n",
				iwidth, iwidth + addleft + addright);
			iwidth = iwidth + addleft + addright;
		}
		if (addtop > 0 || addbottom > 0) {
			printf("Extending height from %d to %d\n",
				iheight, iheight + addtop + addbottom);
			iheight = iheight + addtop + addbottom;
		}
	}

	mingrey = getc(input);
	maxgrey = getc(input);

	for (k=12; k < textoffset; k++) (void) getc(input);
	s = comment;
	for (k=16; k < headlen; k++) *s++ = (char) getc(input);

	printf("headlen %d textoffset %d ",	headlen, textoffset);
	printf("width %d height %d\n", iwidth, iheight);
	printf("bitspersample %d mingrey %u maxgrey %u\n",
			bitspersamplein, mingrey, maxgrey);
	}
	else {		/* nasaflag != 0 */
		iwidth = 800; iheight = 800;
		bitspersamplein = 8;
		mingrey = 0; maxgrey = 255;
	}

	if (bitspersamplein != 1 && binaryflag != 0) {	/* 1993/July/4 */
		printf("Will attempt to convert to monochrome\n");
		bitspersampleout = 1;
	}
	else bitspersampleout = bitspersamplein;

	avergrey = (mingrey + maxgrey) / 2;	/* used for monochrome only */

	if (maxgrey == mingrey) stretchflag = 0;

}

int commandline (int argc, char *argv[], int firstarg) {
	char *s;
	
	while (firstarg < argc && argv[firstarg][0] == '-') {
		s = argv[firstarg];
		if (strcmp(s, "-v") == 0) {
			verboseflag = 1;
		}
		else if (strcmp(s, "-m") == 0) {
			binaryflag = ~binaryflag;
		}
		else if (strcmp(s, "-s") == 0) {
			stripeflag = ~stripeflag;
		}
		else if (strcmp(s, "-d") == 0) {
/*			resolution = ~resolution; */
			if (sscanf(s, "-d=%d", &numscale) < 1) resolution=0;
			else resolution = 1;
		}
		else if (strncmp(s, "-l", 2) == 0) {
			if (sscanf(s, "-l=%d", &addleft) < 1)
				fputs(s, stderr);
			binaryflag = ~binaryflag;
		}
		else if (strncmp(s, "-r", 2) == 0) {
			if (sscanf(s, "-r=%d", &addright) < 1)
				fputs(s, stderr);
			binaryflag = ~binaryflag;
		}
		else if (strncmp(s, "-t", 2) == 0) {
			if (sscanf(s, "-t=%d", &addtop) < 1)
				fputs(s, stderr);
			binaryflag = 1;
		}
		else if (strncmp(s, "-b", 2) == 0) {
			if (sscanf(s, "-b=%d", &addbottom) < 1)
				fputs(s, stderr);
			binaryflag = 1;
		}
		else fputs(s, stderr);
		firstarg++;
	}
	return firstarg;
}

void showusage (char *argv[]) {
	printf("%s [-v] [-m] [-s] [-d=<dpi>]\n", argv[0]);
	printf("\t [-l=<left>] [-r=<right>] [-t=<top>] [-b=<bottom>]\n");
	printf("\t <IMG file>\n");
	printf("\tv verbose mode\n");
	printf("\tm binary (monochrome) output\n");	
	printf("\ts add single row of black pixels at bottom\n");
	printf("\td specify resolution (default %d dpi)\n", resolution);
	printf("\tl add columns on left side\n");
	printf("\tr add columns on right side\n");
	printf("\tt add rows on top\n");
	printf("\tb add rows on bottom\n");
	
	exit(1);
}

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	int c, d;
/*	int e, k; */
/*	char *s; */
	int firstarg = 1;

	if (argc < 2) showusage (argv);

	firstarg = commandline(argc, argv, firstarg);

	if (argc < firstarg + 1) showusage (argv);

	strcpy(infilename, argv[firstarg]);
	extension(infilename, "img");

	if (strstr(infilename, "imq") != NULL) nasaflag=1;	/* hack for now */
	else nasaflag=0;
	
	if (whimpflag != 0) leastout=0;				/* Mac style output */
	if (whimpflag != 0) stretchflag=0;			/* no min and max grey */
	if (whimpflag != 0) resolution=1;			/* XRES & YRES */

	input = fopen(infilename, "rb");
	if (input == NULL) {
		perror(infilename);
		exit(2);
	}
	printf("Conversion of IMG file %s\n\n", infilename);
	readimghead(input);

/*  now open output file */

	strcpy(outfilename, stripname(argv[firstarg]));
	forceexten(outfilename, "tif");
	output = fopen(outfilename, "wb");
	if (output == NULL) {
		perror(outfilename);
		exit(2);
	}

	writetiffheader (output);

/*	copy the actual image data across */
	if (binaryflag != 0) convertmono(output, input);
	else if (stretchflag == 0 ||
		(bitspersampleout != 8 && bitspersampleout != 4))
			while((c = getc(input)) >= 0) putc(c, output);
	else if (bitspersampleout == 8) {
		while((c = getc(input)) >= 0) {
			c = (((unsigned int) c - mingrey) * 255) / (maxgrey - mingrey);
			if (c < 0) c = 0;
			else if (c > 255) c = 255;
			putc(c, output);
		}
	}
	else if (bitspersampleout == 4) {
		while((c = getc(input)) >= 0) {
			d = (c & 15); c = c >> 4;
			c = ((c - mingrey) * 15) / (maxgrey - mingrey);
			d = ((d - mingrey) * 15) / (maxgrey - mingrey);
			c = (c << 4) | d;
			putc(c, output);
		}
	}
	fclose(input);
	fclose(output);
	return 0;
}

/* NOTE: if offset is used directly for value */
/* then value is packed left justified */
/* so above is only `correct' if type is `II' - not if type is `MM' */

/* stretching for bitspersamplein == 4 has not been tested */

/* -v verbose mode */
/* -m convert to monochrome */
/* -d do not add resolution fields to TIFF (fields needed for TIFFView) */
/* -l add white space on left */
/* -r add white space on right */
/* -t add white space on top */
/* -b add white space on bottom */
/* -s add black stripe at bottom (prevent fax software from swalling space */
