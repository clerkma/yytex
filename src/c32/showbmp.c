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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************************/

int verboseflag=0;
int colorflag=0;

#define DEFAULTDPI 72

unsigned char *image=NULL;

#define FAR
#define BYTE unsigned char
#define SHORT short
#define WORD unsigned short
#define LONG long
#define DWORD unsigned long

/* _CRTIMP size_t __cdecl fread(void *, size_t, size_t, FILE *); */

#include <pshpack2.h>
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#include <poppack.h>

typedef struct tagBITMAPINFOHEADER{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD FAR* LPRGBQUAD;

typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} BITMAPINFO, FAR *LPBITMAPINFO, *PBITMAPINFO;

BITMAPFILEHEADER bmfh;

BITMAPINFOHEADER bmih;

RGBQUAD colortable[256];	/* set up when reading BMP */

/******************************************************************************/

long nlen;

int w, h;

int nColor;

int Xdpi, Ydpi;

void showcolortable (RGBQUAD colortable[], int nColor) {
	int k;
	if (nColor > 256) {
		printf("ERROR: too many colors: %d\n", nColor);
		nColor = 256;
	}
	for (k = 0; k < nColor; k++) {
		printf("%d\t%d\t%d\t%d\n",
			   k, colortable[k].rgbRed, colortable[k].rgbGreen, colortable[k].rgbBlue);
	}
}

int dpifrom(int res) {
	double dres = (double) res * 25.4 / 1000.0;
	dres = (double) ((int) (dres * 10.0 + 0.5)) / 10.0;
	return (int) (dres + 0.5);
}

int readhead (FILE *input) {
	long current, OffSet;
	double dw, dh;
	
	fseek(input, 0, SEEK_END);
	nlen = ftell(input);
/*	if (verboseflag) printf("File length %ld bytes\n", nlen); */
	fseek(input, 0, SEEK_SET);
/*	if (verboseflag) putc('\n', stdout); */
/*	read file header */
	fread(&bmfh, sizeof(bmfh), 1, input);
	if (bmfh.bfType != (77 * 256 + 66)) {			/*  "BM" */
		printf("Not BMP file %X\n", bmfh.bfType);
/*		return -1; */
	}
	else if (verboseflag) printf("BMP file %X\n", bmfh.bfType);
/*	file size in words ? NO */
	if (verboseflag) printf("File size %ld %lu\n", nlen, bmfh.bfSize);
/*	offset from end of header ? NO */
	if (verboseflag) printf("Offset to image %lu\n", bmfh.bfOffBits);
/*	if (verboseflag) putc('\n', stdout); */
/*	read bitmap info header */	
	fread(&bmih, sizeof(bmih), 1, input);
	if (bmih.biClrUsed > 0) nColor = bmih.biClrUsed;
	else if (bmih.biBitCount < 24) nColor = 1 << bmih.biBitCount;
	else nColor = 0;
	printf("Width in pixels %ld\n", bmih.biWidth);
	w = bmih.biWidth;
	printf("Height in pixels %ld\n", bmih.biHeight);
	h = bmih.biHeight;
	printf("Bits per pixels %u\n", bmih.biBitCount);
	if (verboseflag) {
		printf("Size of header %lu\n", bmih.biSize);	
		printf("Number of image planes %u\n", bmih.biPlanes);
		printf("Compression %lu\n", bmih.biCompression);
		printf("Size of compressed image %lu\n", bmih.biSizeImage);
		printf("Number of colors used %lu %d\n", bmih.biClrUsed,
			   nColor);
		printf("Number of `important' colors %lu\n", bmih.biClrImportant);
	}

	if (verboseflag) printf("Horizontal pixel per meter %ld\n", bmih.biXPelsPerMeter);
	Xdpi = dpifrom(bmih.biXPelsPerMeter);
	if (Xdpi > 0) dw = (double) w / Xdpi;
	else dw = (double) w / DEFAULTDPI;
	printf("%d dpi (horizontal)\twidth %lg inch\n", Xdpi, dw);

	if (verboseflag) printf("Vertical pixel per meter %ld\n", bmih.biYPelsPerMeter);
	Ydpi = dpifrom(bmih.biYPelsPerMeter);
	dh = (double) h / Ydpi;
	if (Ydpi > 0) dh = (double) h / Ydpi;
	else dh = (double) h / DEFAULTDPI;
	printf("%d dpi (vertical)\theight %lg inch\n", Ydpi, dh);

	if (nColor > 0) {		/*	read color table */
		if (nColor > 256) {
			printf("ERROR: too many colors: %d\n", nColor);
			nColor = 256;
		}
		fread(&colortable, sizeof(RGBQUAD), nColor, input);
	}
	current = ftell(input);
	OffSet= sizeof(bmfh) + sizeof(bmih) + nColor * sizeof(RGBQUAD);
	if (verboseflag)
		printf("Image starts at %ld %ld\n", OffSet, current);
	if (colorflag) showcolortable(colortable, nColor);
	if (verboseflag) putc('\n', stdout);
	fflush(stdout);
	return 0;
}

int histogram[256];

void makehistogram (int h, int w) {
	int i, j, k;

	for (k = 0; k < 256; k++) histogram[k] = 0;
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
/*			k = image [i * w + j]; */
			k = (int) image [i * w + j];
			if (k >= 0 && k < 256)
				histogram[k]++;
		}
	}
}

void showhistogram (void) {
	int k;
	long counteven=0;
	long countodd=0;
	for (k = 0; k < 256; k++) {
		if (histogram[k] > 0) printf("%d\t%d\n", k, histogram[k]);
		if ((k % 2) == 0) counteven += histogram[k];
		else countodd += histogram[k];
	}
	printf("EVEN %ld ODD %ld\n", counteven, countodd);
}

int readimage(FILE *input) {
	int i, j, k;
	int c;
/*	unsigned char color; */
	long n;
	int nbytes=1;
	long current;

	w = (int) bmih.biWidth;
	h = (int) bmih.biHeight;
	n = w * h;
	if (n == 0) return -1;
	image = (unsigned char *) malloc(sizeof(unsigned char) * n); 
	if (image == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d pixel image\n", n);
		exit(1);
	}
/*	current = sizeof(BITMAPFILEHEADER) + bmfh.bfOffBits;  */
	current = bmfh.bfOffBits;
	printf("SEEK to %ld\n", current);
	fseek(input, current, SEEK_SET); 
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
/*			fread(&color, nbytes, 1, input); */
			c = getc(input);
			if (c < 0) {
				printf("ERROR: EOF i %d j %d\n", i, j);
				break;
			}
			image [k] = ((unsigned char) c);
			k++;
		}
		if (c < 0) {
			printf("ERROR: EOF i %d j %d\n", i, j);
			break;
		}
	}
	if (c < 0) printf("ERROR: Premature EOF\n");
	current = ftell(input);
	printf("end at %ld last c %d\n", current, c);
	if (verboseflag) putc('\n', stdout);
	fflush(stdout);
	return 0;
}

/**********************************************************************************/

int roundup (int n) {
	return (((n-1) / 4) + 1) * 4;
}

void writehead (FILE *output, int colorflag) {
	long current;
	int rem;

	bmfh.bfType = 'B' | ('M' << 8);
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
/*	following doesn't take into account padding */ /*	bmih.biClrUsed = 256; */
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if (colorflag) bmfh.bfSize += 3 * w * h;
	else bmfh.bfSize += w * h + 256 * sizeof(RGBQUAD);
/*	bmfh.bfOffBits = sizeof(BITMAPINFOHEADER); */
/*	bmfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER); */
	bmfh.bfOffBits = roundup(sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER));
	fwrite(&bmfh, sizeof(bmfh), 1, output);
	bmih.biSize = 40;  /* unchanged */
	bmih.biWidth = w;
	bmih.biHeight = h;
	bmih.biPlanes = 1;
	if (colorflag) bmih.biBitCount = 24;		/* go for 24 bit RGB */
	else bmih.biBitCount = 8;					/* go for 8 bit B/W */
	bmih.biCompression = 0;
	if (colorflag) bmih.biSizeImage = 3 * w * h;
	else bmih.biSizeImage = w * h;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;			/* if non-zero specifies size of color map */
/*	bmih.biClrUsed = 256; */
	bmih.biClrImportant = 0;
	fwrite(&bmih, sizeof(bmih), 1, output);
	if (colorflag) ; /*	no color map for 24 bit data */
	else fwrite(&colortable, sizeof(RGBQUAD), 256, output);	/* assume set up */
	current = ftell(output);
	if (current != sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
		printf("ERROR: Header not correct size %ld\n", current);
	}
	rem = current % 4;
	while (rem != 0) {
		putc(0, output);
		rem = (rem + 1) % 4;
	}			
}


void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

int main (int argc, char *argv[]) {
	FILE *input;
	char infile[FILENAME_MAX];
	int firstarg=1;
	int m;

	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag = ! verboseflag;
		if (strcmp(argv[firstarg], "-c") == 0) colorflag = ! colorflag;
		firstarg++;
	}

	for (m = firstarg; m < argc; m++) {
		strcpy(infile, argv[m]);
		extension(infile, "bmp");
		if ((input = fopen(infile, "rb")) == NULL) {
			perror(infile);
			exit(1);
		}
		printf("Processing %s\n", infile);

		readhead(input);
/*		readimage(input); */
		if (ferror(input)) {
			printf("ERROR: input file\n");
			perror(infile);
			exit(1);
		}
		fclose (input);
		printf("\n");
		if (verboseflag) fflush(stdout);
	}
	return 0;
}

