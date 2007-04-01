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

/* Program to show images in files on VGA screen */

#include <graph.h>	/* graphics functions */
#include <stdio.h>	/* printf */
#include <string.h>	/* strcpy */
#include <conio.h>	/* getch */
#include <stdlib.h>  /* exit */

#define DEBUG 0			/* non-zero means debugging output enabled */
#define VGABITS 6		/* number of bits in VGA color levels */
#define VGAMAXGREY 256	/* maximum number of grey-levels in any VGA mode */
#define CENTER 1		/* non-zero means center the image */
#define IMGMAXGREY 256	/* maximum number of grey-levels in images */
/* #define FNAMELEN 80 */
#define HEADER 1		/* zero means there is no header - DANGER */
#define MAXTITLE 64		/* zero means there is no title - DANGER */
#define MAXLINE 1140	/* maximum number of bytes in image row */

/* The header contains the following:
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

int videomode; /* e.g. _MAXRESMODE _MAXCOLORMODE */

int columns, rows, numgrey, numbits; /* video adapter properties */

int xo=0, yo=0; /* offset of image corner on screen */

int imgrows, imgcolumns, imggrey, imgbits, headlen, titlepos;
int mingrey, maxgrey, imgflags; /* image file properties */

void giveup(char *s) { /* reset video state and post given message */
	_setvideomode(_DEFAULTMODE); 
	fprintf(stderr, "%s", s);
	exit(1);
}

void showstate(struct videoconfig vc) { /* show state of video adapter */
	printf( "Adapter code %x\n", vc.adapter);
	printf( "Mode code %d\n", vc.mode);
	printf( "Monitor code %x\n", vc.monitor);
	printf( "Bits per pixel %i.\n", vc.bitsperpixel);
	printf( "Memory in kbytes %i.\n", vc.memory);
	printf( "Number of color indeces %i.\n", vc.numcolors);
	printf( "Text columns = %i.\n", vc.numtextcols);
	printf( "Text rows = %i.\n", vc.numtextrows);
	printf( "Video pages = %i.\n", vc.numvideopages);
	printf( "Pixels in row = %i.\n", vc.numxpixels);
	printf( "Pixels in column = %i.\n", vc.numypixels);
}

/* do this more efficiently some time ? */

void greypalette(int numbits, int numgrey) {  /* set up straight grey scale */
	int i, gry, shft;
	long palette[VGAMAXGREY];

	shft = VGABITS - numbits;
#if DEBUG != 0
/*	printf("Shift is %d ", shft); getch(); */
#endif
	for (i=0; i < numgrey; i++) {
		if (shft >= 0)	gry = i << shft;
		else gry = i >> -shft;
		palette[i] = 0x3F3F3FL & ((long) (gry) << 16 | (gry) << 8 |	(gry)); 
	}
	if (_remapallpalette(palette) == 0)	giveup("Couldn't remap palette\n");
}

void paintfile(FILE *input) {
	int irow, icol, grey, ogrey=-1;
	int pack, k, chr, chrs, msk;
	int rnggrey, i, c;
	unsigned char remap[IMGMAXGREY];
	unsigned char buffer[MAXLINE];
	unsigned char *bufptr; 

	if (imgbits != 8) {
		msk = imggrey - 1; 
		pack = 8 / imgbits;
		chrs = imgcolumns / pack;
	}
	else chrs = imgcolumns;
	if (chrs > MAXLINE) giveup("Too many bytes per image row\n");
	rnggrey = maxgrey - mingrey;
	for (i = 0; i < imggrey; i++) { /* make lookup table */
		if (i < mingrey) c = 0;
		else if (i > maxgrey) c = numgrey - 1;
		else c = (int) ((long) (i - mingrey) * (numgrey - 1)/rnggrey);
		remap[i] = (unsigned char) c;
	}
/*	for (i = 0; i < imggrey; i++) 
		printf ("(%d, %d) ", i, remap[i]);  */

	for (irow=0; irow < imgrows; irow++) {
		if (kbhit() && getch() == '\035') giveup("User Interrupt\n");
		k = fread(&buffer, 1, chrs, input); /* read next line */
		if (ferror(input)) {        
			perror("Input file");   /* input file error */
			giveup("Input file error");
		}
		if (k == 0) break; 			/* hit EOF ? */
		bufptr = buffer;			/* pointer into buffer */
		if (imgbits == 8) {
			for (icol=0; icol < imgcolumns; icol++) {
				c = *bufptr++;
				grey = remap[c];
				if (grey != ogrey) _setcolor(grey);
				ogrey = grey;
				_setpixel(xo + icol, yo + irow);
			}
		}
		else {
			icol = 0;
			for (chr=0; chr < chrs; chr++) {
				c = *bufptr++;
				for (k=0; k < pack; k++) {
					grey = remap[c & msk];
					if (grey != ogrey) _setcolor(grey);
					ogrey = grey;
					_setpixel(xo + icol - k - k, yo + irow); /* hack */
					c = c >> imgbits;
					icol++;
				}
			}
		}
	}
}

void showhead(int imgrows, int imgcolumns, int imgbits, char *title) {
	printf("\r%s  %d x %d (%d)", 
		title, imgcolumns, imgrows, imgbits);
}

void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) strcat(fname, "."); strcat(fname, ext);
}

int main(int argc, char *argv[]) {
    FILE *fp_in;
    char fn_in[FILENAME_MAX], title[MAXTITLE];
	struct videoconfig vc;
	short xnw, ynw, xse, yse;
	int c, d, i, ii;
	
	if (argc < 2) {
		fprintf(stderr, "Missing file argument\n");
		exit(5);
	}
	strcpy(fn_in, argv[1]); 	/* get file name */
	extension(fn_in, "img");	/* add extension if needed */

	if ((fp_in = fopen(fn_in, "rb")) == NULL) {
		perror(fn_in); exit(2);
	}

/* This assumes for now that there IS a header */
#if HEADER != 0
	if (getc(fp_in) != 'I' || getc(fp_in) != 'M' || getc(fp_in) != 'G') {
		fprintf(stderr, "Not an IMG file\n");
		exit(4);
	}
#if DEBUG != 0
	printf("IMG: "); 
#endif
	headlen = getc(fp_in); 			/* 64 for example */
	titlepos = getc(fp_in);			/* 16 for example */
#if DEBUG != 0
	printf("headlen = %d, titlepos = %d\n", headlen, titlepos); 
#endif
	if (titlepos < 13 || titlepos > headlen) {
		fprintf(stderr, "Invalid title position %d\n", titlepos);
		exit(5);
	}
	c = getc(fp_in); d = getc(fp_in);
	imgrows = (d << 8) + c;
	c = getc(fp_in); d = getc(fp_in);
	imgcolumns = (d << 8) + c;
#if DEBUG != 0
	printf("imgrows = % d, imgcolumns = %d, ", imgrows, imgcolumns);
#endif
	imgbits = getc(fp_in);			/* 8 for example */
	if (imgbits > 8) {
		fprintf(stderr, "Bits per pixel too large %d", imgbits);
		exit(7);
	}
#if DEBUG != 0
	printf("imgbits = % d\n", imgbits);
#endif
	imggrey = 1 << imgbits; 		/* number of possible grey-levels */
	mingrey = getc(fp_in);
	maxgrey = getc(fp_in);
	if (maxgrey == 0) maxgrey = imggrey - 1; /* default, if maxgrey omitted */
#if DEBUG != 0
	printf("mingrey = % d, maxgrey = %d, ", mingrey, maxgrey);
#endif
	imgflags = getc(fp_in);		/* image flags, not used yet */
#if DEBUG != 0
	printf("imflags = % d\n", imgflags);
#endif
	ii = 13;
	/* read 3 + 1 + 1 + 2 + 2 + 1 + 1 + 1 + 1 = 13 bytes so far */
	for (i = ii; i < titlepos; i++) getc(fp_in); /* skip to title */
	ii = MAXTITLE;
	for (i = 0; i < MAXTITLE; i++) {
		c = getc(fp_in);
		title[i] = (char) c;
		if (c == 0) {ii = i + 1; break;}
	}
#if DEBUG != 0
	printf("Title %s", title); getch();
#endif
	for (i = ii + titlepos; i < headlen; i++) getc(fp_in); 
	/* flush rest of header */
#endif
/* above leaves variables undefined if there is no header ... */
/* 	printf("TITLE: %s", title);	getch(); */
		
	if (imgrows <= 200 && imgcolumns <= 320) videomode= _MAXCOLORMODE;
	else videomode = _MAXRESMODE;

	if(_setvideomode(videomode) != 0) {
		_getvideoconfig( &vc );
#if DEBUG
		showstate(vc); getch();
#endif
		columns = vc.numxpixels;
		rows = vc.numypixels;
		numgrey = vc.numcolors; 
		numbits = vc.bitsperpixel;
		xnw = 0;	ynw = 0;	xse = columns;	yse = rows;
#if CENTER != 0
		xo = (columns - imgcolumns)/2; yo = (rows - imgrows)/2;
#endif
		_setviewport(xnw, ynw, xse, yse);
		greypalette(numbits, numgrey);
		showhead(imgrows, imgcolumns, imgbits, title);
		paintfile(fp_in);
		showhead(imgrows, imgcolumns, imgbits, title);
		getch(); 
	} else puts("Can't enter desired graphics mode.\n");
	_setvideomode(_DEFAULTMODE);	
	return 0;
}

/* speed up file input - read row at a time OK */
/* speed up computation of palette OK */
/* speed up grey-level remapping using array OK */
/* fix kludges for packed pixels ? */
/* do windowing iternally, don't write to stuff outside port */
/* do text output to screen more fancily */
/* make text output work for _MAXCOLORMODE */
/* make videomode command line flag ? */
/* make xo, yo windowing command line option */
/* make mingrey and maxgrey command line options also */
/* put optional frame around image area ? */
