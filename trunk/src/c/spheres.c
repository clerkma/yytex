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

#include <graph.h>	/* graphics functions */
#include <stdio.h>	/* puts */
#include <conio.h>	/* getch */
#include <stdlib.h> /* atexit */
#include <math.h>   /* sqrt */

#define MAXGREY 256		/* maximum number of grey-levels in any mode */

/* int videomode=_MAXRESMODE; */
int videomode=_MAXCOLORMODE;
int columns, rows, numgrey, numbits;
float fps = 1.0f, fqs = 1.0f; 	/* position of light source */

void showstate(struct videoconfig vc) { /* show state of video adapter */
	printf( "Adapter code %x\n", vc.adapter);
	printf( "Mode code %x\n", vc.mode);
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

int log2(int n) { 	/* compute log base 2 */
	if (n <= 0) {
		fprintf(stderr, "Error in log2, n = %d", n);
		exit(3);
	}
	if (n == 1) return 0;
	else return log2(n >> 1) + 1;
}	

/* have the table precompiled ? */

void greypalette(int numbits, int numgrey) { /* set up straight grey scale */
	int i, gry, shft;
	long palette[MAXGREY];

	shft = 6 - numbits;
	printf("Shift is %d ", shft); /* ? */
	getch();
	for (i=0; i < numgrey; i++) {
		if (shft >= 0)	gry = i << shft;
		else gry = i >> -shft;
		palette[i] = 0x3F3F3FL & ((long) (gry) << 16 | (gry) << 8 |	(gry)); 
/*		_remappalette(i, col); */
	}
	if (_remapallpalette(palette) == 0) {
		_setvideomode(_DEFAULTMODE);
		fprintf(stderr, "Couldn't remap palette\n");
		exit(7);
	}
}

void drawsphere(int xo, int yo, int radius) {
	int dx, dy;
	long rs, rads;
	int grey, ogrey=-1;
	float fmaxg, fpsqs, fz, fnumer, fdenom;

	fmaxg = (float) numgrey;
	rads = (long) radius * (long) radius;
	fpsqs = (float) sqrt(1.0 + fps * fps + fqs * fqs);
	fdenom = (float) radius * fpsqs;
	for (dx = -radius; dx <= radius; dx++) {
		for (dy = -radius; dy <= radius; dy++) {
			rs = (long) dx * (long) dx + (long) dy * (long) dy;
			if (rs < rads) {
				fz = (float) sqrt((float) radius * (float) radius -
					(float) dx * (float) dx - (float) dy * (float) dy); 
				fnumer = fz - fps * (float) dx - fqs * (float) dy;
				if (fnumer > 0.0f) grey = (int) ((fnumer/fdenom) * fmaxg); 
				else grey = 0;
				if (grey != ogrey) _setcolor(grey);
				ogrey = grey;
				_setpixel(xo + dx, yo + dy);
			}
		}
	}
}

void cleanup(void) { /* atexit */
	_setvideomode(_DEFAULTMODE);
}

int main(void) {
	struct videoconfig vc;
	short xnw, ynw, xse, yse;
	
	if(atexit(&cleanup) != 0) {
		printf("Couldn't register atexit function\n");
		getch();
	}
		
	if(_setvideomode(videomode) != 0) {
		_getvideoconfig( &vc );
		columns = vc.numxpixels;
		rows = vc.numypixels;
		numgrey = vc.numcolors; 
		numbits = vc.bitsperpixel;
		printf("%d columns %d rows %d grey %d bits ", 
			columns, rows, numgrey, numbits); 

		xnw = 0; ynw = 0;
		xse = columns; 		/* depends on mode */
		yse = rows;			/* depends on mode */

		_setviewport(xnw, ynw, xse, yse);
		greypalette(numbits, numgrey);
		drawsphere(columns/2, rows/2, 100);
		drawsphere(columns/4, rows/4, 50);
		drawsphere(columns/2, rows/4, 75);
		getch(); 
	} else puts("Can't enter desired graphics mode.\n");
	_setvideomode(_DEFAULTMODE);	
	return 0;
}

/* look into reading and writing the whole image */
