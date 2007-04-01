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

/*********************************************************************/

#define MAXLINE 512

char line[MAXLINE];

int verboseflag=1;
int traceflag=0;
int debugflag=0;
int colorflag=1;

int w, h;						/* width and height */

FILE *errout=stdout;

/*********************************************************************/

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

/*********************************************************************/

/* stuff to deal with BMP image files */

void showcolortable (RGBQUAD colortable[], int n) {
	int k;
	for (k = 0; k < n; k++) {
		printf("%d\t%d\t%d\t%d\n",
			   k, colortable[k].rgbRed, colortable[k].rgbGreen, colortable[k].rgbBlue);
	}
}

int readhead (FILE *input) {
	long nlen;
/*	BITMAPFILEHEADER bmfh; */
/*	BITMAPINFOHEADER bmih; */
	
	fseek(input, 0, SEEK_END);
	nlen = ftell(input);
	if (verboseflag) printf("File length %ld\n", nlen);
	fseek(input, 0, SEEK_SET);
	if (verboseflag) putc('\n', stdout);
/*	read file header */
	fread(&bmfh, sizeof(bmfh), 1, input);
	if (bmfh.bfType != (77 * 256 + 66)) {			/*  "BM" */
		printf("Not BMP file %X\n", bmfh.bfType);
/*		return -1; */
	}
	else if (verboseflag) printf("BMP file %X\n", bmfh.bfType);
/*	file size in words ? */
	if (verboseflag) printf("File size %lu\n", bmfh.bfSize);
/*	offset from end of header ? */
	if (verboseflag) printf("Offset to image %lu\n", bmfh.bfOffBits);
	if (verboseflag) putc('\n', stdout);
/*	read bitmap info header */	
	fread(&bmih, sizeof(bmih), 1, input);
	if (verboseflag) printf("Size of header %lu\n", bmih.biSize);	
	if (verboseflag) printf("Width in pixels %ld\n", bmih.biWidth);	
	if (verboseflag) printf("Height in pixels %ld\n", bmih.biHeight);	
	if (verboseflag) printf("Number of image planes %u\n", bmih.biPlanes);
	if (verboseflag) printf("Bits per pixels %u\n", bmih.biBitCount);
	if (verboseflag) printf("Compression %lu\n", bmih.biCompression);
	if (verboseflag) printf("Size of compressed image %lu\n", bmih.biSizeImage);
	if (verboseflag) printf("Horizontal pixel per meter %ld\n", bmih.biXPelsPerMeter);
	if (verboseflag) printf("Vertical pixel per meter %ld\n", bmih.biYPelsPerMeter);
	if (verboseflag) printf("Number of colors used %lu\n", bmih.biClrUsed);
	if (verboseflag) printf("Number of `important' colors %lu\n", bmih.biClrImportant);
/*	read color table */
	fread(&colortable, sizeof(RGBQUAD), bmih.biClrUsed, input);
/*	if (showcolorflag) showcolortable(colortable, bmih.biClrUsed); */
	if (verboseflag) putc('\n', stdout);
	fflush(stdout);
	return 0;
}

/*********************************************************************/

double *image=NULL;

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
/*	image = (unsigned char *) malloc(sizeof(unsigned char) * n); */
	image = (double *) malloc(sizeof(double) * n);
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
/*			image [k] = (double) color; */
			image [k] = (double) ((unsigned char) c);
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

/*********************************************************************/

int roundup (int n) {
	return (((n-1) / 4) + 1) * 4;
}

void writebmpheader (FILE *output, int colorflag) {
	unsigned long current;
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
	if (colorflag) ;
	else bmfh.bfOffBits += 256 * sizeof(RGBQUAD);
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
	rem = current % 4;
	while (rem != 0) {
		putc(0, output);
		rem = (rem + 1) % 4;
	}			
	current = ftell(output);
	if (current != bmfh.bfOffBits) {
		printf("ERROR: Header not correct size %ld %ld\n",
			   current, bmfh.bfOffBits);
	}
}

/*********************************************************************/

int getabyte(FILE *input) {
	int c, d;
	while ((c = getc(input)) <= ' ' && c != EOF) ;
	if (c == EOF) {
		fprintf(errout, "Premature EOF\n");
		return -1;
	}
	if (c >= '0' && c <= '9') c = c - '0';
	else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
	else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
	else {
		fprintf(errout, "Bad character %c\n", c);
		return -1;
	}
	while ((d = getc(input)) <= ' ' && d != EOF) ;
	if (d == EOF) {
		fprintf(errout, "Premature EOF\n");
		return -1;
	}
	if (d >= '0' && d <= '9') d = d - '0';
	else if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
	else if (d >= 'a' && d <= 'f') d = d - 'a' + 10;
	else {
		fprintf(errout, "Bad character %c\n", d);
		return -1;
	}
	c = (c << 4) | d;
	return c;
}

/* read row of hex data from image in PS file and convert to binary */

int hextobinrow (FILE *output, FILE *input, int nWidth) {
	int c, r, g, b, k;

	for (k = 0; k < nWidth; k++) {
		if (colorflag) {
			r = getabyte(input);
			if (r < 0) return -1;
			g = getabyte(input);
			if (g < 0) return -1;
			b = getabyte(input);
			if (b < 0) return -1;
			putc(b, output);
			putc(g, output);
			putc(r, output);
		}
		else {
			c = getabyte(input);
			if (c < 0) return -1;
			putc(c, output);
		}
	}
	return 0;
}

int nWidth, nHeight, nBitsPerComponent;

int readpsheader (FILE *input) {
	int c;
	char *s;

	nWidth = nHeight = nBitsPerComponent = -1;
	while (fgets(line, sizeof(line), input) != NULL) {
/*		printf("%s", line); */
		if (strstr(line, "currentfile") != NULL) {
			printf("Found: %s", line);
			break;
		}
		if ((s = strstr(line, "/Width")) != NULL) {
			if (sscanf(s+7, "%d", &nWidth) < 1) 
				fprintf(errout, "Don't understand %s", line);
			printf("%s", line);
		}
		else if ((s = strstr(line, "/Height")) != NULL) {
			if (sscanf(s+8, "%d", &nHeight) < 1) 
				fprintf(errout, "Don't understand %s", line);
			printf("%s", line);
		}
		else if ((s = strstr(line, "/BitsPerComponent")) != NULL) {
			if (sscanf(s+18, "%d", &nBitsPerComponent) < 1) 
				fprintf(errout, "Don't understand %s", line);
			printf("%s", line);
		}
	}
	fgets(line, sizeof(line), input);
	printf("%s", line);
	if (strcmp(line, "ID\n") != 0) {
		fprintf(errout, "End of header not found %s\n", line);
		return -1;
	}
	w = nWidth;
	h = nHeight;
	if (nWidth < 0 || nHeight < 0 || nBitsPerComponent < 0) {
		fprintf(errout, "Header information incomplete\n");
		return -1;
	}
	printf("Found PS image %d x %d (%d bits per component)\n",
		   nWidth, nHeight, nBitsPerComponent);
	return 0;
}

int scantoheader (FILE *input, int nimage) {
	while ((fgets (line, sizeof(line), input)) != NULL) {
		if(strcmp(line, "BI\n") == 0) {
			if (--nimage <= 0) return 0;
		}
	}
	return -1;
}

int getpsheader (FILE *input, int nimage) {
	if (scantoheader(input, nimage)) {
		fprintf(errout, "Failed to find image header\n");
		return -1;
	}
	if (verboseflag) printf("Scanned to PS image header\n");
	if (readpsheader(input) != 0) {
		fprintf(errout, "Failed to get header information\n");
		return -1;
	}
	if (verboseflag) printf("Read PS image header\n");
	fflush(stdout);
}

/* read nrows of image data and write it to BMP file */
/* pad out each row to multiple of four bytes */
/* nbytes is image width if B/W or image width * 3 for RGB */

int hextobinimage (FILE *output, FILE *input, int ncolumns, int nrows) {
	int k, rem;
	int nbytes;

	if (ncolumns <= 0 || nrows <= 0) return -1;
	writebmpheader(output, colorflag);
	if (verboseflag) printf("Wrote BMP file header\n");
	if (verboseflag)
		printf("Processing %d rows of %d pixels\n", nrows, ncolumns);
	for (k = 0; k < nrows; k++) {
		if (hextobinrow(output, input, ncolumns) != 0) {
			fprintf(errout, "Transfer failed\n");
			return -1;
		}
		if (colorflag) nbytes = ncolumns * 3;
		else nbytes = ncolumns;
		rem = nbytes % 4;
		while (rem != 0) {	/* pad out to multiple of four bytes */
			putc(0, output);
			rem = (rem + 1) % 4;
		}			
		if (verboseflag) putc('.', stdout);
	}
	return 0;
}


/*****************************************************************/

int commandline(int argc, char *argv[], int k) {
	while (k < argc && argv[k][0] == '-') {
		if (strcmp(argv[k], "-v") == 0) verboseflag = ! verboseflag;
		if (strcmp(argv[k], "-t") == 0) traceflag = ! traceflag;
		if (strcmp(argv[k], "-c") == 0) colorflag = ! colorflag;
		if (strcmp(argv[k], "-d") == 0) debugflag = ! debugflag;
		k++;
	}
	return k;
}

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *extractfilename(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
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

char psfile[FILENAME_MAX]="page4.ps1";
char bmpfile[FILENAME_MAX];

int main (int argc, char *argv[]) {
	FILE *input, *output;
	int firstarg=1;
	int k; 

	firstarg = commandline(argc, argv, firstarg);
	if (argc >= firstarg + 1) {
		strcpy(psfile, argv[firstarg]);
		firstarg++;
	}
	else {
		fprintf(errout, "Missing file name\n");
/*		exit(1); */	/* using default */
	}

	if ((input = fopen(psfile, "r")) == NULL) {
		perror(psfile);
		exit(1);
	}

	strcpy(bmpfile, extractfilename(psfile));
	forceexten(bmpfile, "bmp");

	if ((output = fopen(bmpfile, "wb")) == NULL) {
		perror(bmpfile);
		exit(1);
	}

	if (verboseflag) printf("%s => %s\n", psfile, bmpfile);
	getpsheader(input, 1);
	hextobinimage (output, input, nWidth, nHeight);

	fclose(output);
	fclose(input);
	return 0;
}

/*******************************************************************/

