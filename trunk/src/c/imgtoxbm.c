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

void writexbmhead (FILE *output) {
	fprintf(output, "#define noname_width %d\n", iwidth);
	fprintf(output, "#define noname_height %d\n", iheight);
	fprintf(output, "static char noname_bits[] = {\n");
} 

/*  0x00,0x10,0xc8,0x76,0x00,0x00,0x00,0x20, */

void copyimage (FILE *output, FILE *input) {
	int i, j, k, c;
	unsigned int d;
	int n, thres;
	thres = 1 << (bitspersamplein - 1);
	for (i = 0 ; i < iheight; i++) {
		putc(' ', output);
		for (j = 0 ; j < iwidth/8; j++) {
			d = 0;
			for (k = 0; k < 8; k++) {
/*				d = d << 1; */
				d = d >> 1; 
				c = getc(input);
				if (c == EOF) {
					fprintf(stderr, "EOF\n");
					return;
				}
/*				if (c >= thres) d |= 1; */
/*				if (c >= thres) d |= 128; */
				if (c < thres) d |= 128;
			}
			if (d == 0) fprintf(output, "0x00,");
			else fprintf(output, "%#02.2x,",  d);
		}
		putc('\n', output);
	}
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

	input = fopen(infilename, "rb");
	if (input == NULL) {
		perror(infilename);
		exit(2);
	}
	printf("Conversion of IMG file %s\n\n", infilename);

	readimghead(input);

	if (bitspersamplein != 1) {
		fprintf(stderr, "%d bits per sample...\n", bitspersamplein);
/*		exit(1); */
	}

/*  now open output file */

	strcpy(outfilename, stripname(argv[firstarg]));
	forceexten(outfilename, "xbm");
	output = fopen(outfilename, "wb");
	if (output == NULL) {
		perror(outfilename);
		exit(2);
	}

	writexbmhead (output);

	fseek (input, 64, SEEK_SET);

	copyimage (output, input);

	fprintf(output, "};");

	fclose (output);
	fclose (input);

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
