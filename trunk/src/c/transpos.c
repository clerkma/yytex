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

/* code to take an image in IMG form and transpose it */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

unsigned int iwidth, iheight, bitsperpixel;

int leastin = 1, leastout = 1;

int mingrey, maxgrey;

unsigned int ureadtwo(FILE *input) {
	unsigned int c, d;
	c = (unsigned int) getc(input);
	d = (unsigned int) getc(input);	
	if (leastin != 0) return ((d << 8) | c);
	else return ((c << 8) | d);
}

/*
unsigned long ureadfour(FILE *input) {
	unsigned int c, d, e, f;
	c = (unsigned int) getc(input);
	d = (unsigned int) getc(input);	
	e = (unsigned int) getc(input);	
	f = (unsigned int) getc(input);		
	if (leastin != 0) return ((((((f << 8) | e) << 8) | d) << 8) | c);
	else return ((((((c << 8) | d) << 8) | e) << 8) | f);
} */

void uwritetwo(unsigned int offset, FILE *output) {
	if (leastout != 0) {
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;
	}
	else {
		putc((int) ((offset >> 8) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 8) & 255), output); offset = offset << 8;
	}
}

/*
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
} */

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

#define MAXFILENAME 128

int main(int argc, char *argv[]) {       /* main program */
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	FILE *input, *output;
	unsigned int i, j, inrowlength;
	int c, d;
	long k;
	char _huge *image;
	char *s; 
	
	if (argc < 2) exit(1);

	mingrey=255; maxgrey=0;
	strcpy(infilename, argv[1]);
	extension(infilename, "img");
/*	strcpy(outfilename, argv[1]);
	forceexten(outfilename, "out"); */

	if ((s=strrchr(infilename, '\\')) != NULL) s++;
	else if ((s=strrchr(infilename, ':')) != NULL) s++;
	else s = infilename;
	strcpy(outfilename, s);  	/* copy input file name minus path */
	forceexten(outfilename, "out");

	if((input = fopen(infilename, "rb")) == NULL) {
		perror(infilename);
		exit(3);
	}
	if((output = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename);
		exit(3);
	}
	
	printf("Input file: %s output file: %s\n", infilename, outfilename);
	
/*	assumes header length is 64 */
	for (i = 0; i < 5; i++) putc(getc(input), output); 
	iheight = ureadtwo(input); 
	iwidth = ureadtwo(input);
	uwritetwo(iwidth, output); 
	uwritetwo(iheight, output);
	bitsperpixel = getc(input);
	putc(bitsperpixel, output);
	for (i = 5 + 5; i < 64; i++) putc(getc(input), output); 
	
	inrowlength = (iwidth * bitsperpixel + 7) / 8;

	printf("Image is %d by %d\n", iheight, iwidth);

	printf("Allocate memory\n");
	if ((image = (char _huge *) 
		halloc((long) iheight * iwidth, sizeof(char))) == NULL)
		exit(7);

	printf("Read image %s\n", infilename);
	for (i = 0; i < iheight; i ++) {
		k = (long) i * iwidth;
		for (j = 0; j < iwidth; j ++) {	/* k = i * iwidth + j */
			c = getc(input);
			if (bitsperpixel == 8) {
				if (c > maxgrey) maxgrey = c;
				else if (c < mingrey) mingrey = c;
				image[k++] = (char) c; 
			}
			else {	/* assume four bits per pixel */
				d = c & 15; c = c >> 4;
				image[k++] = (char) c; image[k++] = (char) d;
			}
		}
		printf(".");
	}
	printf("\n");
	if (bitsperpixel == 8)
		printf("Min Grey %d Max Grey %d\n", mingrey, maxgrey);
	printf("Write image %s\n", outfilename);
	for (j = 0; j < iwidth; j ++) {
		k = j;
		for (i = 0; i < iheight; i ++) { 
			if (bitsperpixel == 8) {
				putc(image[k], output);
				k += iwidth;
			}
			else {	/* assumes four bits per pixel */
				c = image[k]; k += iwidth;
				d = image[k]; k += iwidth;
				c = (c << 4) | d;
				putc(c, output);
			}
		}
		printf("*");
	} 
	printf("\n");
	fclose(input);
	fclose(output);
	hfree(image);
	printf("Finished\n");
	(void) getch();
/*	exit(0); */
	return 0; 
}
