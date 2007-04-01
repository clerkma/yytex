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

/* make simple grey level image for testeing purposes */
/* run through imgtotif afterwards to make TIFF file from it */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void writeheader (FILE *output, int width, int height) {
	char *title="LZW compression test image";
	int i, n;
	putc('I', output);	/* IMG */
	putc('M', output);
	putc('G', output);
	putc(64, output);	/* length of header */
	putc(16, output);	/* position of title string */
	putc(height & 255, output);		/* number of rows, low order first */
	putc((height >> 8) , output);
	putc(width & 255, output);		/* number of columns, low order first */
	putc((width >> 8) , output);
	putc(8, output);	/* bits per pixel */
	putc(0, output);	/* min grey level */
	putc(255, output);	/* max grey level */
	putc(0, output);	/* reserved flags */
	n = 13;
	for (i=n; i < 16; i++) putc(0, output);		/* pad out */
	fputs(title, output);
	n = 16 + strlen(title);
	for (i=n; i < 64; i++) putc(0, output);		/* pad out */
}

void writeimage(FILE *output, int width, int height) {
	int i, j, grey, scale;
	scale = 256 / (width + height);
	if (scale == 0) scale = 1;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			grey = ((i + j) * scale) & 255;
			putc(grey, output);
		}
	}
}

int main(int argc, char *argv[]) {
	char *filename="test.img";
	FILE *output;
/*	int width=8; */
/*	int width=16; */
/*	int width=32; */
/*	int width=64; */
/*	int width=128; */
	int width=256;
/*	int height=8; */
/*	int height=16; */
/*	int height=32; */
/*	int height=64; */
/*	int height=128; */
	int height=256;

	if ((output = fopen (filename, "wb")) == NULL) {
		perror(filename);
		exit(1);
	}
	writeheader(output, width, height);
	writeimage(output, width, height);
	fclose(output);
	return 0;
}
