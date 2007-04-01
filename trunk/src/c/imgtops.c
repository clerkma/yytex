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

/* Make PS file from IMG format file 1, 2, 4, 8 bit pixels */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXFILENAME 64

int verboseflag=0;

int wantcntrld=1;			/* want control D at end of output */

int binaryflag = 0;			/* convert to monochrome if set */

int stretchflag=0;			/* stretch greylevels using mingrey & maxgrey */

int flipflag=0;

int nasaflag = 0;

unsigned int leastin=1;		/* non-zero => PC style, zero => Mac style */

int numscale = 50;			/* XRES = YRES = numscale / denscale */
int denscale = 1;			/* XRES = YRES = resscale / denscale */

int resolution=1;			/* include xresolution and yresolution fields */

int centerflag=0;			/* center output image on page */

int pagewidth = 72 * 17 /2;	/* 8.5 inches */

int pageheight = 72 * 11;	/* 11 inches */

int spi = 50;				/* samples per inch in output */

int xll = 72;				/* lower left corner */

int yll = 72;				/* lower left corner */

int xur, yur;

int pwidth, pheight;

int headlen, textoffset;
int iwidth, iheight, bytes;
unsigned int mingrey, maxgrey, avergrey;
int bitspersamplein, bitspersampleout;

char comment[64];

char *program;					/* pointer to argv[0] */

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

void readimghead(FILE *input) { /*	first read IMG file header */
	int c, d, e, k;
	char *s;
	
	if (!nasaflag) {
		c = getc(input); d = getc(input); e = getc(input);
		if (c == 'I' && d == 'M' && e == 'G') ;
		else {
			printf("File starts with `%c%c%c' instead of `IMG''\n", c, d, e);
			exit(1);
		}
		headlen =  getc(input);
		if (headlen != 64) printf("Head length not 64\n");
		textoffset = getc(input);
		if (textoffset != 16) printf("Text offset not 16\n");	

		iheight = ureadtwo(input);
		iwidth  = ureadtwo(input);
		bitspersamplein = getc(input);

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

	if (bitspersamplein != 1 && binaryflag != 0) {
		printf("Will attempt to convert to monochrome\n");
		bitspersampleout = 1;
	}
	else bitspersampleout = bitspersamplein;

	avergrey = (mingrey + maxgrey) / 2;		/* used for monochrome only */

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
		else if (strcmp(s, "-d") == 0) {
/*			resolution = ~resolution; */
			if (sscanf(s, "-d=%d", &numscale) < 1) resolution=0;
			else resolution = 1;
		}
		else fputs(s, stderr);
		firstarg++;
	}
	return firstarg;
}

int column;

/* Bitspersample can be 1, 2, 4, 8 for ease of conversion to PS */
/* Each row takes an integral number of bytes */

void converttohex (FILE *output, FILE *input) {
	int c, d, i, j;

	column = 0;
	for (j = 0; j < iheight; j++) {
		for (i = 0; i < bytes; i++) {
			if (column >= 38) {
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

}

void writepsheader (FILE *output) {
	fputs("%!PS-Adobe-2.0\n", output);
	fprintf(output, "%%%%Creator: %s\n", program);
	fprintf(output, "%%%%BoundingBox: %d %d %d %d\n", xll, yll, xur, yur);
	fprintf(output, "/picstr %d string def\n", bytes);
	fprintf(output, "%d %d translate\n", xll, yll);
	fprintf(output, "%d %d scale\n", pwidth, pheight);
	fprintf(output, "%d %d %d\n", iwidth, iheight, bitspersampleout);
	fprintf(output, "[%d 0 0 %d 0 %d]\n", iwidth, -iheight, iheight);
	fputs("{currentfile picstr readhexstring pop}\n", output);
	fputs("image\n", output);
}

void writepstrailer (FILE *output) {
	fputs("showpage\n", output);
	fputs("%%EOF\n", output);
	if (wantcntrld) putc(4, output);
}

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

int imgtops (char *filename) {
	FILE *input, *output;
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	int changeflag;

	strcpy(infilename, filename);
	extension(infilename, "img");

	if (strstr(infilename, "imq") != NULL) nasaflag=1;	/* hack for now */
	else nasaflag=0;
	
	input = fopen(infilename, "rb");
	if (input == NULL) {
		perror(infilename);
		return -1;
	}

	if (verboseflag)
		printf("Conversion of IMG file %s\n\n", infilename);

	if (!nasaflag)
		readimghead(input);			/* read image header */

	changeflag = 0;
	for (;;) {
		pwidth = (int) (((long) iwidth * 72) / spi);
		pheight = (int) (((long) iheight * 72) / spi);
		if (pwidth + xll < pagewidth && pheight + yll < pageheight) break;
		spi = spi * 6 / 5;
		changeflag++;
	}
	if (changeflag)
		printf("Increased samples per inch in output to %d\n", spi);
	else printf("Using %d samples per inch\n", spi);

	if (centerflag) {
		xll = (pagewidth -  pwidth) / 2;
		yll = (pageheight - pheight) / 2;
	}

	xur = xll + pwidth;  yur = yll + pheight;

	if (bitspersampleout > 8) {
		fprintf(stderr, "Maximum of 8 bits per sample supported\n");
		fclose(input);
		return -1;
	}
	if ((bitspersampleout & (bitspersampleout-1)) != 0) {
		fprintf(stderr, "Bits per sample must be power of two\n");
		fclose(input);
		return -1;
	}

/*	compute bytes per row */
	bytes = (int) (((long) iwidth * bitspersampleout + 7) / 8);

/*  now open output file */

	strcpy(outfilename, stripname(filename));
	forceexten(outfilename, "ps");
	output = fopen(outfilename, "w");
	if (output == NULL) {
		perror(outfilename);
		fclose(input);
		return -1;
	}

	writepsheader (output);

	converttohex (output, input);

	writepstrailer (output);

	return 0;
}

void showusage (char *argv[]) {
	exit(1);
}

int main (int argc, char *argv[]) {
	int firstarg = 1;

	if (argc < 2) showusage (argv);

	firstarg = commandline(argc, argv, firstarg);

	if (argc < firstarg + 1) showusage (argv);

	program = argv[0];

	if (imgtops (argv[firstarg]) != 0) exit(1);
	else return 0;

}

