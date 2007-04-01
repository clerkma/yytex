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

// Code for fitting affine transformation
// Code for fitting radial and tangential distortion
// Code for locating BB images accurately

// code for least squares fitting image intensifier distortion
// one second order term in radial distortion and
// one second order term in tangential distortion

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define PI 3.141592653

#define MAXBBS 10000

/************************************************************************/

// global parameters controlling behaviour

int verboseflag=1;
int traceflag=0;
// int sparseflag=1;			// set to 1 for image001.bmp
int sparseflag=0;				// set to 0 for image003.bmp
int refineflag=1;				// refine position of BB image centers
int fuzzyflag=1;				// check more carefully for local min/max

int showheaderflag=0;			// show BMP file header info (input)
int showcolorflag=0;			// show BMP file color table (input)
int writesmoothflag=0;			// write smooth image (output)
int colorflag=0;				// write image in RGB (rather than B/W)

int suppressradial=0;			// suppress radial distortion compensation
int suppresstangential=0;		// suppress tangential distortion compensation

// int forceaffine=1;				// force identity affine transformation

/************************************************************************/

// Image size --- kept global to avoid passing so many args

int imageheight, imagewidth;

/************************************************************************/

// Image Intensifier CCD is 980 x 980

// We (arbitrarily) pick the center of the image as the center of
// radial and tangential distortion.
// (Could make these two more unknowns to be determined by optimization).

double xo=490.0;	// image center in x
double yo=490.0;	// image center in y

/*****************************************************************************/

// DATA: estimated centers of BB images in 980 x 980 image in IMAGE1.BMP
// Along with coordinates in (integer) target coordinate system.
// Items starting with {0, 0, ...} are ignored
// This data is used by setupdata and setupdatafine

// PaintShop measures second coordinate from the top row of the image,
// while data starts at the bottom row...
// Target rows are also numbered from top downwards...

//	image_x, (imageheight-1) - image_y, target_x, (12 - target_y)

int bidata[][4]=
{
{  0,   0, 0, 0},
{  0,   0, 1, 0},
{  0,   0, 2, 0},
{410,  41, 3, 0},
{492,  66, 4, 0},
{572,  88, 5, 0},
{651, 108, 6, 0},
{730, 126, 7, 0},
{  0,   0, 8, 0}, // {809, 140},
{  0,   0, 9, 0},
{  0,   0, 10, 0},
{  0,   0, 11, 0},

{  0,   0, 0, 1},
{221, 104, 1, 1},
{307, 130, 2, 1},
{389, 154, 3, 1},
{468, 176, 4, 1},
{546, 199, 5, 1},
{622, 220, 6, 1},
{699, 239, 7, 1},
{777, 258, 8, 1},
{856, 274, 9, 1},
{  0,   0, 10, 1},
{  0,   0, 11, 1},


{  0,   0, 0, 2},
{201, 213, 1, 2},
{283, 236, 2, 2},
{363, 259, 3, 2},
{440, 281, 4, 2},
{518, 305, 5, 2},
{593, 325, 6, 2},
{668, 346, 7, 2},
{746, 366, 8, 2},
{823, 384, 9, 2},
{905, 400, 10, 2},
{  0,   0, 11, 2},

{ 90, 299, 0, 3},
{176, 319, 1, 3},
{257, 340, 2, 3},
{335, 361, 3, 3},
{412, 383, 4, 3},
{487, 405, 5, 3},
{562, 428, 6, 3},
{638, 449, 7, 3},
{715, 470, 8, 3},
{793, 488, 9, 3},
{874, 507, 10, 3},
{  0,   0, 11, 3}, // {961, 523},

{ 63, 405, 0, 4},
{148, 423, 1, 4},
{228, 441, 2, 4},
{306, 463, 3, 4},
{382, 484, 4, 4},
{458, 506, 5, 4},
{533, 528, 6, 4},
{608, 550, 7, 4},
{685, 570, 8, 4},
{763, 591, 9, 4},
{844, 609, 10, 4},
{932, 627, 11, 4},

{ 34, 512, 0, 5},
{120, 527, 1, 5},
{200, 545, 2, 5},
{276, 564, 3, 5},
{352, 585, 4, 5},
{428, 606, 5, 5},
{504, 628, 6, 5},
{580, 650, 7, 5},
{657, 671, 8, 5},
{736, 691, 9, 5},
{818, 712, 10, 5},
{  0,   0, 11, 5},

{  0,   0, 0, 6},
{ 90, 635, 1, 6},
{171, 651, 2, 6},
{248, 669, 3, 6},
{325, 688, 4, 6},
{401, 709, 5, 6},
{476, 729, 6, 6},
{553, 750, 7, 6},
{631, 772, 8, 6},
{712, 793, 9, 6},
{796, 815, 10, 6},
{  0,   0, 11, 6},

{  0,   0, 0, 7},
{  0,   0, 1, 7},
{140, 761, 2, 7},
{220, 776, 3, 7},
{298, 794, 4, 7},
{375, 813, 5, 7},
{451, 834, 6, 7},
{529, 855, 7, 7},
{609, 877, 8, 7},
{691, 901, 9, 7},
{  0,   0, 10, 7},
{  0,   0, 11, 7},

{  0,   0, 0, 8},
{  0,   0, 1, 8},
{  0,   0, 2, 8},
{  0,   0, 3, 8},
{  0,   0, 4, 8}, // {271, 910},
{350, 927, 5, 8},
{429, 947, 6, 8},
{  0,   0, 7, 8},	// {509, 969},
{  0,   0, 8, 8},
{  0,   0, 9, 8},
{  0,   0, 10, 8},
{  0,   0, 11, 8}
};

/************************************************************/
/* Code for dealing with BMP image files                    */
/************************************************************/

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

/******************************************************************************/

// code for reading BMP image files 

void showcolortable (RGBQUAD colortable[], int n) {
	int k;
	printf("\tR\tG\tB\n");
	for (k = 0; k < n; k++) {
		printf("%d\t%d\t%d\t%d\n",
			   k, colortable[k].rgbRed, colortable[k].rgbGreen, colortable[k].rgbBlue);
	}
}

int readBMPheader (FILE *input, BITMAPFILEHEADER *bmfh, BITMAPINFOHEADER *bmih) {
	RGBQUAD colortable[256];
	long nlen;

//	printf("bmfh %X bmih %X\n", bmfh, bmih);
	fflush(stdout);
	fseek(input, 0, SEEK_END);
	nlen = ftell(input);
	fseek(input, 0, SEEK_SET);
	if (showheaderflag) printf("File length %ld\n", nlen);
//	read file header 
	fread(bmfh, sizeof(BITMAPFILEHEADER), 1, input);
	if (bmfh->bfType != (77 * 256 + 66)) {			//  "BM" 
		printf("Not BMP file %X (%X)\n", bmfh->bfType, (77 * 256 + 66));
//		return -1; 
	}
	else if (showheaderflag) printf("BMP file %X (%X)\n", bmfh->bfType, (77 * 256 + 66));
//	offset = bmfh->bfOffBits;
	fread(bmih, sizeof(BITMAPINFOHEADER), 1, input);
	imagewidth = bmih->biWidth;
	imageheight = bmih->biHeight;
//	bitsperpixel = bmih->biBitCount;
//	read color table 
	fread(&colortable, sizeof(RGBQUAD), bmih->biClrUsed, input);
//	file size in words ? 
	if (showheaderflag) {
		printf("File size %lu\n", bmfh->bfSize);
//		offset from end of header ? 
		printf("Offset to image %lu\n", bmfh->bfOffBits);
		putc('\n', stdout);
//		read bitmap info header 
		printf("Size of header %lu\n", bmih->biSize);	
		printf("Width in pixels %ld\n", bmih->biWidth);	
		printf("Height in pixels %ld\n", bmih->biHeight);
		printf("Number of image planes %u\n", bmih->biPlanes);
		printf("Bits per pixels %u\n", bmih->biBitCount);
		printf("Compression %lu\n", bmih->biCompression);
		printf("Size of compressed image %lu\n", bmih->biSizeImage);
		printf("Horizontal pixel per meter %ld\n", bmih->biXPelsPerMeter);
		printf("Vertical pixel per meter %ld\n", bmih->biYPelsPerMeter);
		printf("Number of colors used %lu\n", bmih->biClrUsed);
		printf("Number of `important' colors %lu\n", bmih->biClrImportant);
	}
	if (showcolorflag) showcolortable(colortable, bmih->biClrUsed);
	if (showheaderflag) putc('\n', stdout);
	fflush(stdout);
	return 0;
}

unsigned char **setupimagearray(int imageheight, int imagewidth);

unsigned char **readBMPimage (FILE *input, BITMAPFILEHEADER *bmfh, BITMAPINFOHEADER *bmih) {
	int i, j;
	int c=0;
//	int nbytes=1;
	long current;
	unsigned char **image=NULL;

	imagewidth = (int) bmih->biWidth;
	imageheight = (int) bmih->biHeight;
	printf("%d x %d\n", imageheight,imagewidth);
	image = setupimagearray(imageheight, imagewidth);
	if (image == NULL) {
		printf("ERROR: unable to create image array %d x %d\n", imageheight, imagewidth);
		return NULL;		// failure
	}

//	current = sizeof(BITMAPFILEHEADER) + bmfh->bfOffBits; 
	current = bmfh->bfOffBits;
	if (showheaderflag) printf("SEEK to %ld\n", current);
	fseek(input, current, SEEK_SET); 
	for (i = 0; i < imageheight; i++) {
//		k = i * imagewidth;
		for (j = 0; j < imagewidth; j++) {
//			fread(&colortable, nbytes, 1, input);
			c = getc(input);
			if (c < 0) {
				printf("ERROR: EOF i %d j %d\n", i, j);
				break;
			}
//			image [k++] = (char) c;
			image[i][j] = (unsigned char) c;	// ignore color table
		}
		if (c < 0) {
			printf("ERROR: EOF i %d j %d\n", i, j);
			break;
		}
	}
	if (c < 0) printf("ERROR: Premature EOF\n");
	current = ftell(input);
	if (traceflag) printf("end at %ld last c %d\n", current, c);
//	if (verboseflag) putc('\n', stdout);
//	fflush(stdout);
	return image;
}

unsigned char **readBMP (char *infile) {
	FILE *input;
	unsigned char **image=NULL;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	input = fopen(infile, "rb");
	if (input == NULL) {
		perror(infile);
		return NULL;
	}
//	printf("bmfh %X bmih %X\n", &bmfh, &bmih);
	readBMPheader(input, &bmfh, &bmih);
	if (bmfh.bfType != (77 * 256 + 66)) {
		printf("Not BMP file %X (%X)\n", bmfh.bfType, (77 * 256 + 66));
		return NULL;
	}
	image = readBMPimage(input, &bmfh, &bmih);
	fclose(input);
	return image;
}

/******************************************************************************/

// code for writing BMP image files 

// setup up gray color table for 8 bit data (not used for 24 bit)

void setupcolortable (RGBQUAD colortable[], int n) {
	int k;
	for (k = 0; k < n; k++) {
		colortable[k].rgbRed = (unsigned char) k;
		colortable[k].rgbGreen = (unsigned char) k;
		colortable[k].rgbBlue = (unsigned char) k;
	}
}

int roundup (int n) {
	return (((n-1) / 4) + 1) * 4;
}

void writeBMPheader (FILE *output, BITMAPFILEHEADER *bmfh, BITMAPINFOHEADER *bmih,
					 int imageheight, int imagewidth) {
	RGBQUAD colortable[256];
	unsigned long current;
	int rem;
//	BITMAPFILEHEADER bmfh;
//	BITMAPINFOHEADER bmih;

	bmfh->bfType = 'B' | ('M' << 8);
	bmfh->bfReserved1 = 0;
	bmfh->bfReserved2 = 0;
//	following doesn't take into account padding  ---	bmih->biClrUsed = 256; 
	bmfh->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if (colorflag) bmfh->bfSize += 3 * imagewidth * imageheight;
	else bmfh->bfSize += imagewidth * imageheight + 256 * sizeof(RGBQUAD);
//	bmfh->bfOffBits = sizeof(BITMAPINFOHEADER); 
//	bmfh->bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER); 
	bmfh->bfOffBits = roundup(sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER));
	if (colorflag) ;
	else bmfh->bfOffBits += 256 * sizeof(RGBQUAD);
//	fwrite(&bmfh, sizeof(bmfh), 1, output);
	fwrite(bmfh, sizeof(BITMAPFILEHEADER), 1, output);
	bmih->biSize = 40;  // unchanged 
	bmih->biWidth = imagewidth;
	bmih->biHeight = imageheight;
	bmih->biPlanes = 1;
	if (colorflag) bmih->biBitCount = 24;		// go for 24 bit RGB 
	else bmih->biBitCount = 8;					// go for 8 bit B/W
	bmih->biCompression = 0;
	if (colorflag) bmih->biSizeImage = 3 * imagewidth * imageheight;
	else bmih->biSizeImage = imagewidth * imageheight;
	bmih->biXPelsPerMeter = 0;
	bmih->biYPelsPerMeter = 0;
	bmih->biClrUsed = 0;			// if non-zero specifies size of color map 
//	bmih->biClrUsed = 256; 
	bmih->biClrImportant = 0;
//	fwrite(&bmih, sizeof(bmih), 1, output);
	fwrite(bmih, sizeof(BITMAPINFOHEADER), 1, output);
	if (colorflag) ; //	no color map for 24 bit data 
	else {
		setupcolortable(colortable, 256);
		fwrite(&colortable, sizeof(RGBQUAD), 256, output);	// assume set up 
	}
	current = ftell(output);
	rem = current % 4;
	while (rem != 0) {
		putc(0, output);
		rem = (rem + 1) % 4;
	}			
	current = ftell(output);
	if (current != bmfh->bfOffBits) {
		printf("ERROR: Header not correct size %ld %ld\n",
			   current, bmfh->bfOffBits);
	}
}

void writeBMPimage (FILE *output, unsigned char **image,
					BITMAPFILEHEADER *bmfh, BITMAPINFOHEADER *bmih) {
	int i, j, rem;
	int ir, ig, ib;
	int imageheight, imagewidth;

	imagewidth = (int) bmih->biWidth;
	imageheight = (int) bmih->biHeight;
	printf("%d x %d\n", imageheight,imagewidth);

	for (i = 0; i < imageheight; i++) {
//		k = i * imagewidth;
		for (j = 0; j < imagewidth; j++) {
//			ir = ig = ir = image[k];
			ib = ig = ir = image[i][j];
			if (colorflag) {
				putc(ib, output); 
				putc(ig, output); 
				putc(ir, output);
			}
			else putc(ig, output);
//			k++;
		}
		if (colorflag) rem = (imagewidth * 3) % 4;
		else rem = imagewidth % 4;
		while (rem != 0) {
			putc(0, output);
			rem = (rem + 1) % 4;
		}			
	}
}

int writeBMP (char *outfile, unsigned char **image, int imageheight, int imagewidth) {
	FILE *output;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	output = fopen(outfile, "wb");
	if (output == NULL) {
		perror(outfile);
		return -1;
	}
	writeBMPheader(output, &bmfh, &bmih, imageheight, imagewidth);
	writeBMPimage(output, image, &bmfh, &bmih);
	fclose(output);
	return 0;
}

/********************************************************************/

void makehistogram (unsigned char **image, int *histogram) {
	int i, j, k;
	for (k = 0; k < 256; k++) histogram[k] = 0;
	for (i = 0; i < imageheight; i++) {
		for (j = 0; j < imagewidth; j++) {
			k = image[i][j];
			histogram[k]++;
		}
	}
}

void showhistogram (int *histogram) {
	int k, kstart=0, kend=255;
	for (k = 0; k < 256; k++) {
		if (histogram[k] > 0) {
			kstart = k;
			break;
		}
	}
	for (k = 255; k >= 0; k--) {
		if (histogram[k] > 0) {
			kend = k+1;
			break;
		}
	}
	for (k = kstart; k < kend; k++)
		printf("%3d\t%ld\n", k, histogram[k]);
}

/****************************************************************************/

// Uniform and Gaussian random numbers

double random (double rmin, double rmax) {	// uniform between rmin & rmax
	return ((double) rand() / (double) RAND_MAX) * (rmax - rmin) + rmin;
}

#define NGAUSSIAN 6		// enough out to 6 st devs
// #define NGAUSSIAN 3			// smaller for speed out to 3 st devs

// Generate pseudo-random numbers with Gaussian dsitribution

double gaussian (double stdev) {	// mean zero Gaussian random var
	double sum=0.0;
	int i;
	if (stdev == 0.0) return 0.0;
	for (i = 0; i < NGAUSSIAN; i++) sum += random(-1.0, 1.0);
	return sum * stdev / sqrt(NGAUSSIAN/3);
}

/****************************************************************************/

// Simple stuff for 3-vectors and 3 x 3 matrices
// Here matrices are just of the form:	double mat[3][3]

void show_vector3 (double a[3]) {
	printf("%lg %lg %lg\n", a[0], a[1], a[2]);
}

void show_matrix33 (double m[3][3]) {
	printf("%lg %lg %lg\n", m[0][0], m[0][1], m[0][2]);
	printf("%lg %lg %lg\n", m[1][0], m[1][1], m[1][2]);
	printf("%lg %lg %lg\n", m[2][0], m[2][1], m[2][2]);
}

// Product written back into last argument

void matrix_times_vector33 (double c[3][3], double b[3], double a[3]) {
	a[0] = c[0][0] * b[0] + c[0][1] * b[1] + c[0][2] * b[2];
	a[1] = c[1][0] * b[0] + c[1][1] * b[1] + c[1][2] * b[2];
	a[2] = c[2][0] * b[0] + c[2][1] * b[1] + c[2][2] * b[2];
}

// Solution written back into last argument

double solve33 (double m[3][3], double b[3], double a[3]) {
	double c[3][3];
	double det;
//	first find transpose of cofactors of M
	c[0][0] = m[1][1] * m[2][2] - m[1][2] * m[2][1];
	c[0][1] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
	c[0][2] = m[1][0] * m[2][1] - m[1][1] * m[2][0];	
	c[1][0] = m[0][2] * m[2][1] - m[0][1] * m[2][2];
	c[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
	c[1][2] = m[0][1] * m[2][0] - m[0][0] * m[2][1];
	c[2][0] = m[0][1] * m[1][2] - m[0][2] * m[1][1];
	c[2][1] = m[0][2] * m[1][0] - m[0][0] * m[1][2];
	c[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];
	det = m[0][0] * c[0][0] + m[0][1] * c[0][1] + m[0][2] * c[0][2];
	if (det == 0.0) {
		printf("Zero determinant!\n");
		return 0.0;
	}
//	solution is C b / det
	a[0] = (c[0][0] * b[0] + c[1][0] * b[1] + c[2][0] * b[2])/det;
	a[1] = (c[0][1] * b[0] + c[1][1] * b[1] + c[2][1] * b[2])/det;
	a[2] = (c[0][2] * b[0] + c[1][2] * b[1] + c[2][2] * b[2])/det;
	return det;
}

// Code to test linear equation solver above

void testsolve33 (void) {
	double m[3][3];
	double a[3], b[3], c[3];
	double det;
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			m[i][j] = gaussian(1.0);
		}
		b[i] = gaussian(1.0);
	}
	det = solve33(m, a, c);
	if (traceflag) {
		printf("MATRIX:\n");
		show_matrix33(m);
		printf("VECTOR B: ");
		show_vector3(b);
		matrix_times_vector33(m, b, a);
		printf("VECTOR A: ");
		show_vector3(a);
		printf("VECTOR C: ");
		show_vector3(c);
	}
}

/****************************************************************************/

//	create vector, quaternion & matrix structures, and free them again

double *make_vector_n (int n) {			// vector of length n
	return (double *) malloc(n * sizeof(double));
}

double *make_vector (void) {
	return make_vector_n(3);
}

double *make_quaternion (void) {
	return make_vector_n(4);
}

//	make n x m matrix --- i.e. n rows of m elements
//	NOTE: matrix organized as array of row vectors 
//	NOTE: m[i] is the i-th row ... *not* column
//	NOTE: uses margin array for fast indexing and convenience
//	NOTE: double m[...][...] creates a *different* structure

double **make_matrix_nm (int n, int m) { // n rows m columns
	int i;
	double **mat;
	mat	= (double **) malloc(n * sizeof(double *));	// margin array
	for (i = 0; i < n; i++)
		mat[i] = make_vector_n(m);	// row of matrix
	return mat;
}

double **make_matrix (void) {
	return make_matrix_nm(3, 3);
}

void free_vector_n (double *v, int n) {
	free(v);
}

void free_vector (double *v) {
	free_vector_n(v, 3);
}

void free_quaternion (double *q) {
	free_vector_n(q, 4);
}

void free_matrix_nm (double **mat, int n, int m) {
	int i;
	for (i = n-1; i > 0; i--) free(mat[i]);
	free(mat);
}

void free_matrix (double **mat) {
	free_matrix_nm(mat, 3, 3);
}

void show_vector_n (double *v, int n) {
	int i;
	if (v == NULL) return;
	for (i = 0; i < n; i++) printf("%lg ", v[i]);
	printf("\n");
}

void show_vector (double *v) {
	show_vector_n(v, 3);
}

void show_quaternion (double *q) {
	show_vector_n(q, 4);
}

void show_matrix_nm (double **mat, int n, int m) {
	int i;
	if (mat == NULL) return;
	for (i = 0; i < n; i++) show_vector_n(mat[i], m);
}

void show_matrix (double **mat) {
	show_matrix_nm(mat, 3, 3);
}

void setup_vector(double v0, double v1, double v2, double *vec) {
	vec[0] = v0; vec[1] = v1; vec[2] = v2;
}

/*******************************************************************/

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

//	Gauss Jordan from Numerical Recipes
//	Here matrices are built using margin arrays

int *make_ivector(int n) {
	return (int *) malloc (n * sizeof(int));
}

void free_ivector(int *v, int n) {
	free(v);
}

//	Matrix to invert in a, rhs to solve in b
//	NOTE: inverse overwrites original vector
//	NOTE: solutions overwrite original rhs

int gaussj(double **a, int n, double **b, int m) {
	int *indxc, *indxr, *ipiv;				// for pivoting bookkeeping
	int i, j, k, l, ll;
	int icol=0, irow=0;			// preset to avoid benign compiler warning
	double big, dum, pivinv, temp;

	indxc = make_ivector(n);
	indxr = make_ivector(n);
	ipiv  = make_ivector(n);				// whether has been used as pivot

	for (j = 0; j < n; j++) ipiv[j] = 0;	// set to unused
	for (i = 0; i < n; i++) {				// main loop over columns
		big = 0.0;
		for (j = 0; j < n; j++)				// outer loop search for pivot
			if (ipiv[j] !=  1)
				for (k = 0; k < n; k++) {
					if (ipiv[k] == 0) {
						if (fabs(a[j][k]) >=  big) {
							big = fabs(a[j][k]);
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1) {
						printf("ERROR: gaussj: Singular Matrix-1\n");
						return -1;
					}
				}
		++(ipiv[icol]);						// mark used
//		now have pivot element, so interchange rows if needed
		if (irow !=  icol) {
			for (l = 0; l < n; l++) SWAP(a[irow][l], a[icol][l])
					for (l = 0; l < m; l++) SWAP(b[irow][l], b[icol][l])
		}
		indxr[i] = irow;				// remember which ones we swapped
		indxc[i] = icol;
//		now divide pivot row by pivot element
		if (a[icol][icol] == 0.0) {
			printf("ERROR: gaussj: Singular Matrix-2\n");
			return -1;
		}
		pivinv = 1.0 / a[icol][icol];
		a[icol][icol] = 1.0;
		for (l = 0; l < n; l++) a[icol][l] *=  pivinv;
		for (l = 0; l < m; l++) b[icol][l] *=  pivinv;
//		reduce the rows --- except for the pivot row
		for (ll = 0; ll < n; ll++)
			if (ll !=  icol) {
				dum = a[ll][icol];
				a[ll][icol] = 0.0;
				for (l = 0; l < n; l++) a[ll][l] -=  a[icol][l] * dum;
				for (l = 0; l < m; l++) b[ll][l] -=  b[icol][l] * dum;
			}
	}
//	now need to unscramble column interchanges - in reverse order
	for (l = n-1; l >= 0; l--) {
		if (indxr[l] !=  indxc[l])
			for (k = 0; k < n; k++)
				SWAP(a[k][indxr[l]], a[k][indxc[l]]);
	}

	free_ivector(ipiv,  n);
	free_ivector(indxr, n);
	free_ivector(indxc, n);
	return 0;
}

//	just invert the matrix, no rhs to solve equations for

int gauss_invert (double **a, int n) {
	return gaussj (a, n, NULL, 0);
}

//	solve for just a single rhs vector

int gauss_solve (double **a, int n, double *rhs, double *sol) {
	double **b;
	int k, flag;
	b = make_matrix_nm(n, 1);	// skinny right hand side
	for (k = 0; k < n; k++) b[k][0] = rhs[k];
	flag = gaussj (a, n, b, 1);
	for (k = 0; k < n; k++) sol[k] = b[k][0];
	free_matrix_nm(b, n, 1);
	return flag;
}

/*****************************************************************************/

// Dynamically set up array for image using margin array

unsigned char **setupimagearray (int imageheight, int imagewidth) {
	int k, nlen;
	long npixels = imagewidth * imageheight;
	unsigned char *imagearray=NULL;
	unsigned char **image=NULL;

	if (npixels == 0) return NULL;
	if (imagearray != NULL) free(imagearray);
	nlen = sizeof(char) * npixels;
	imagearray = (unsigned char *) malloc(nlen);
	if (imagearray == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d pixel imagearray\n", npixels);
		return NULL;
	}
	if (image != NULL) free(image);
	nlen = sizeof(char *) * imageheight;
	image = (unsigned char **) malloc(nlen);
	if (image == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d margin array\n", imageheight);
		return NULL;
	}
//	set up convenient margin array
	for (k = 0; k < imageheight; k++) image[k] = imagearray + k * imagewidth;
	return image;
}

// Dynamically set up array for smoothed image using margin array

double **setupblurrarray (int imageheight, int imagewidth) {
	int npixels = imagewidth * imageheight;
	int k,nlen;
	double *blurrarray=NULL;
	double **blurr=NULL;

	if (npixels == 0) return NULL;
	if (blurrarray != NULL) free(blurrarray);
	nlen = sizeof(double) * npixels;
	blurrarray = (double *) malloc(nlen);
	if (blurrarray == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d pixel blurrarray\n", npixels);
		return NULL;
	}
	nlen = sizeof(double *) * imageheight;
	if (blurr != NULL) free(blurr);
	blurr = (double **) malloc(nlen);
	if (blurr == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d margin array\n", imageheight);
		return NULL;
	}
//	set up convenient margin array
	for (k = 0; k < imageheight; k++) blurr[k] = blurrarray + k * imagewidth;
	return blurr;
}

/****************************************************************************************/

// create matrix of image and target coordinates from list of data --
// for image001.bmp

int setupdata (int bidata[][4], double **bbcoords, double **iicoords, int bbmax) {
	int k, kk=0;
//	int targetwidth=12;
	int targetheight=9;

	if (verboseflag) printf("Setupdata %d\n", bbmax);
	imageheight = imagewidth = 980;
	for (k = 0; k < bbmax; k++) {
		if (bidata[k][0] == 0 && bidata[k][1] == 0) continue;
//		coordinates in image
		iicoords[kk][0] = bidata[k][0];						// image_x
		iicoords[kk][1] = (imageheight-1) - bidata[k][1];	// image_y
//		coordinates in target
		bbcoords[kk][0] = bidata[k][2]				;		// target_x
		bbcoords[kk][1] = (targetheight-1) - bidata[k][3];	// target_y
		kk++;
	}
	return kk;
}

/*****************************************************************************/

// set up matrix for least squares solution of affine transformation
// gets list of  target coordinates in first argument
// writes result back into last argument

int make_affine_matrix (double **bbcoords, int bbtotal,
						double m[3][3]) {
	int i, j, k;
	double xb, yb;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) m[i][j] = 0.0;
	}
	for (k = 0; k < bbtotal; k++) {
		xb = bbcoords[k][0];
		yb = bbcoords[k][1];
		m[0][0] += xb * xb;
		m[0][1] += xb * yb;
		m[0][2] += xb;
		m[1][0] += yb * xb;
		m[1][1] += yb * yb;
		m[1][2] += yb;
		m[2][0] += xb;
		m[2][1] += yb;
		m[2][2] += 1;
	}
	if (bbtotal < 3) {
		printf("ERROR: too little data %d\n", bbtotal);
		return -1;
	}
	return 0;
}

// solve for first row of affine transformation (x part)
// gets list of target coordinates in first argument
// gets list of image coordinates in second argument
// writes result back into last argument
// uses m[3][3] obtained from make_affine_matrix above

int affinefitx (double **bbcoords, double **iicoords, int bbtotal, double m[3][3],
				double c[3]) {
//	double m[3][3];
	double b[3];
	double xb, yb, xi, det;
	int i, k;

//	if (make_affine_matrix(bbcoords, bbtotal, m) < 0) return -1;
	for (i = 0; i < 3; i++) b[i] = 0.0;
	for (k = 0; k < bbtotal; k++) {
		xi = iicoords[k][0];		// image_x
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		b[0] += xi * xb;
		b[1] += xi * yb;
		b[2] += xi;
	}

	det = solve33(m, b, c);

	if (traceflag) {
		printf("MATRIX:\n");
		show_matrix33(m);
		printf("VECTOR B: ");
		show_vector3(b);
		printf("VECTOR C: ");
		show_vector3(c);
	}
	printf("xi = xb * %lg + yb * %lg + %lg\n", c[0], c[1], c[2]);
	return 0;
}

// solve for second row of affine transformation (y part)
// gets list of target coordinates in first argument
// gets list of image coordinates in second argument
// writes result back into last argument

int affinefity (double **bbcoords, double **iicoords, int bbtotal, double m[3][3],
				double c[3]) {
//	double m[3][3];
	double b[3];
	double xb, yb, yi, det;
	int i, k;

//	if (make_affine_matrix(bbcoords, bbtotal, m) < 0) return -1;
	for (i = 0; i < 3; i++) b[i] = 0.0;
	for (k = 0; k < bbtotal; k++) {
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		b[0] += yi * xb;
		b[1] += yi * yb;
		b[2] += yi;
	}

	det = solve33(m, b, c);

	if (traceflag) {
		printf("MATRIX:\n");
		show_matrix33(m);
		printf("VECTOR B: ");
		show_vector3(b);
		printf("VECTOR C: ");
		show_vector3(c);
	}
	printf("yi = xb * %lg + yb * %lg + %lg\n", c[0], c[1], c[2]);
	return 0;
}

// Compute error in affine fit - decompose into radial and tangential
// Show details if traceflag is on

double checkaffinefit (double **bbcoords, double **iicoords, int bbtotal, double cx[3], double cy[3]) {
	int k;
	double xi, yi, xb, yb, xid, yid;
	double xd, yd, dx, dy, dv, r, dr, dt;
	double sum=0.0;

	if (traceflag) printf("Checking affine fit:\n"); 

	for (k = 0; k < bbtotal; k++) {
		xi = iicoords[k][0];		// image_x
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
//		compute coordinates expected from affine transformation
		xd = cx[0] * xb + cx[1] * yb + cx[2];
		yd = cy[0] * xb + cy[1] * yb + cy[2];
//		error - difference observed - predicted from affine
		dx = xi - xd;
		dy = yi - yd;
		dv = sqrt(dx * dx + dy * dy);	// error magnitude
		sum += dx * dx + dy * dy;
//		measured image coordinates (relative to center)
		xid = xi - xo;
		yid = yi - yo;
		r = sqrt(xid * xid + yid * yid);
//		computed image coordinates (relative to center)
//		xdd = xd - xo;
//		ydd = yd - yo;
//		radial and tangential components of error
		if (r != 0.0) {
			dr = (dx * xid + dy * yid) / r;	
			dt = (dy * xid - dx * yid) / r;
//		printf("%2d %2d\t x %4lg y %4lg\t xd %7lg yd %7lg\t dx %8lg dy %8lg\t dr %8lg\n",
//			   i, j, x, y, xd, yd, dx, dy, dr);
//		printf("%2d %2d x %4lg y %4lg r %8lg xd %8lg yd %8lg dx %9lg dy %9lg dv %8lg\n",
//		i, j, x, y, r, xd, yd, dx, dy, dv);
			if (traceflag)
				printf("%2d x %4lg y %4lg r %8lg xd %8lg yd %8lg dr %9lg dt %9lg dv %8lg\n",
					   k, xi, yi, r, xd, yd, dr, dt, dv);
		}
	}
	if (traceflag) printf("Average st. dev. error %lg\n", sqrt(sum / bbtotal));
	return sqrt(sum / bbtotal);
}

/*******************************************************************************/

//	Set up matrix for least squares solution of radial and tangential distortion
//	Writes result back into last argument

int make_distort_matrix (double **iicoords, int bbtotal,
						 double m[3][3]) {
	int i, j, k;
	double xi, yi, r;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) m[i][j] = 0.0;
	}
	for (k = 0; k < bbtotal; k++) {
		xi = iicoords[k][0] - xo;
		yi = iicoords[k][1] - yo;
		r = sqrt(xi * xi + yi * yi);
		m[0][0] += r * r * r * r;
		m[0][1] += r * r * r;
		m[1][0] += r * r * r;
		m[0][2] += r * r;
		m[1][1] += r * r;
		m[2][0] += r * r;
		m[1][2] += r;
		m[2][1] += r;
		m[2][2] += 1;
	}
	if (bbtotal < 3) {
		printf("TOO LITTLE DATA %d x %d\n", bbtotal);
		return -1;
	}
	return 0;
}

//	Get best second order fit in radial direction
//	Uses m[3][3] made bu make_distort_matrix
//	Writes result back into last argument

int radialfit (double **bbcoords, double **iicoords, int bbtotal, 
			   double ax[3], double ay[3], double m[3][3], double c[3]) {
	int k;
	double xi, yi, xb, yb, det, xid, yid;
	double xd, yd, dx, dy, r, dr;
//	double m[3][3];
	double b[3];

//	if (make_distort_matrix(iicoords, bbtotal, m) < 0) return -1;
	for (k = 0; k < 3; k++) b[k] = 0.0;
	for (k = 0; k < bbtotal; k++) {
		xi = iicoords[k][0];		// image_x
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		xd = ax[0] * xb + ax[1] * yb + ax[2];
		yd = ay[0] * xb + ay[1] * yb + ay[2];
		dx = xi - xd;
		dy = yi - yd;
		xid = xi - xo;
		yid = yi - yo;
		r = sqrt(xid * xid + yid * yid);
		if (r != 0.0) {
			dr = (dx * xid + dy * yid) / r;
//			dt = (dy * xid - dx * yid) / r;
			b[0] += dr * r * r;
			b[1] += dr * r;
			b[2] += dr;
		}
	}
	det = solve33(m, b, c);
	printf("%lg * r * r + %lg * r + %lg\n", c[0], c[1], c[2]);
//	Following is detail output on the curve
	if (verboseflag) {
		double rcrs, rzer1, rzer2, rmax, det, maxerr1, maxerr2, maxerr3, maxerr4, maxerr5, temp;
		rcrs = -c[1] / c[0];
		rmax = imagewidth/2;
		maxerr1 = c[0] * (rcrs/2) * (rcrs/2) + c[1] * (rcrs/2) + c[2];
		maxerr2 = c[0] * rmax * rmax + c[1] * rmax + c[2];
		det = c[1] * c[1] - 4 * c[0] * c[2];
		if (det > 0) rzer1 = (- c[1] + sqrt(det)) / (2 * c[0]);
		else rzer1 = 0.0;
		if (det > 0) rzer2 = (- c[1] - sqrt(det)) / (2 * c[0]);
		else rzer2 = 0.0;
		if (rzer2 < rzer1) {
			temp = rzer2;
			rzer2 = rzer1;
			rzer1 = temp;
		}
		maxerr3 = c[0] * rzer1 * rzer1 + c[1] * rzer1 + c[2];
		maxerr4 = c[0] * rcrs * rcrs + c[1] * rcrs + c[2];
		maxerr5 = c[0] * rzer2 * rzer2 + c[1] * rzer2 + c[2];
		printf("org (%lg) zero %lg max pos %lg (%lg) zero %lg edge %lg (%lg)\n",
			   c[2],
			   rzer1, /*  maxerr3, */
			   rcrs/2, maxerr1,
			   rzer2, /* maxerr5, */
/*			   rcrs, maxerr4, */
			   rmax, maxerr2);
	}
	return 0;
}

//	Get best second order fit in tangential direction
//	Uses m[3][3] made bu make_distort_matrix
//	Writes result back into last argument

int tangentialfit (double **bbcoords, double **iicoords, int bbtotal, 
				   double ax[3], double ay[3], double m[3][3], double c[3]) {
	int k;
	double xi, yi, xb, yb, det, xid, yid;
	double xd, yd, dx, dy, r, dt;
//	double m[3][3];
	double b[3];

//	if (make_distort_matrix(iicoords, bbtotal, m) < 0) return -1;
	for (k = 0; k < 3; k++) b[k] = 0.0;
	for (k = 0; k < bbtotal; k++) {
		xi = iicoords[k][0];		// image_x
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		xd = ax[0] * xb + ax[1] * yb + ax[2];
		yd = ay[0] * xb + ay[1] * yb + ay[2];
		dx = xi - xd;
		dy = yi - yd;
		xid = xi - xo;
		yid = yi - yo;
		r = sqrt(xid * xid + yid * yid);
		if (r != 0.0) {
//			dr = (dx * xid + dy * yid) / r;
			dt = (dy * xid - dx * yid) / r;
			b[0] += dt * r * r;
			b[1] += dt * r;
			b[2] += dt;
		}
	}
	det = solve33(m, b, c);
	printf("%lg * r * r + %lg * r + %lg\n", c[0], c[1], c[2]);
//	Following is detail output on the curve
	if (verboseflag) {
		double rcrs, rzer1, rzer2, rmax, det, maxerr1, maxerr2, maxerr3, maxerr4, maxerr5, temp;
		rcrs = -c[1] / c[0];
		rmax = imagewidth/2;
		maxerr1 = c[0] * (rcrs/2) * (rcrs/2) + c[1] * (rcrs/2) + c[2];
		maxerr2 = c[0] * rmax * rmax + c[1] * rmax + c[2];
		det = c[1] * c[1] - 4 * c[0] * c[2];
		if (det > 0) rzer1 = (- c[1] + sqrt(det)) / (2 * c[0]);
		else rzer1 = 0.0;
		if (det > 0) rzer2 = (- c[1] - sqrt(det)) / (2 * c[0]);
		else rzer2 = 0.0;
		if (rzer2 < rzer1) {
			temp = rzer2;
			rzer2 = rzer1;
			rzer1 = temp;
		}
		maxerr3 = c[0] * rzer1 * rzer1 + c[1] * rzer1 + c[2];
		maxerr4 = c[0] * rcrs * rcrs + c[1] * rcrs + c[2];
		maxerr5 = c[0] * rzer2 * rzer2 + c[1] * rzer2 + c[2];
		printf("org (%lg) zero %lg  max pos %lg (%lg) zero %lg edge %lg (%lg)\n",
			   c[2],
			   rzer1, /* maxerr3, */
			   rcrs/2, maxerr1,
			   rzer2, /* maxerr5, */
/*			   rcrs, maxerr4, */
			   rmax, maxerr2);
	}
	return 0;
}

//	Predict image coordinates (xi, yi)
//	corresponding to given target coordinates (xb, yb)
//	given affine transformation ax[], ay[],
//	radial cr[] and tangential distortion ct[]
//	writes result back into third and fourth argument

void predict (double xb, double yb, double *xi, double *yi,
			  double ax[3], double ay[3], double cr[3], double ct[3]
			 ) {
	double xd, yd, xid, yid, xdp, ydp, r, drd, dtd;
	xd = ax[0] * xb + ax[1] * yb + ax[2];			// affine x part
	yd = ay[0] * xb + ay[1] * yb + ay[2];			// affine y part
	xid = xd - xo;				// w.r.t center of image
	yid = yd - yo;				// w.r.t center of image
	r = sqrt(xid * xid + yid * yid);
	if (r != 0.0) {
		drd = cr[0] * r * r + cr[1] * r + cr[2];	// radial correction
		dtd = ct[0] * r * r + ct[1] * r + ct[2];	// tangential correction
		xdp = xd + (drd * xid - dtd * yid) / r;
		ydp = yd + (drd * yid + dtd * xid) / r;
	}
	else {
		xdp = xd;
		ydp = yd;
	}
	*xi = xdp;
	*yi = ydp;
}

// check error remaining after radial and tangential fit
// applies correction using observed image coordinates

double checkerror (double **bbcoords, double **iicoords, int bbtotal,
				   double ax[3], double ay[3], double cr[3], double ct[3]) {
	int k;
	double xi, yi, xb, yb, xid, yid, xdp, ydp;
	double xd, yd, dx, dy, r;
//	double dr, dt;
	double drd, dtd;
	double sum=0.0;
	for (k = 0; k < bbtotal; k++) {
		xi = iicoords[k][0];		// image_x
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		xd = ax[0] * xb + ax[1] * yb + ax[2];
		yd = ay[0] * xb + ay[1] * yb + ay[2];
		dx = xi - xd;
		dy = yi - yd;
		xid = xi - xo;
		yid = yi - yo;
		r = sqrt(xid * xid + yid * yid);
		if (r != 0.0) {
//			dr = (dx * xid + dy * yid) / r;
//			dt = (dy * xid - dx * yid) / r;
			drd = cr[0] * r * r + cr[1] * r + cr[2];
			dtd = ct[0] * r * r + ct[1] * r + ct[2];
//			one way to compute the error:
//			sum += (drd - dr) * (drd -dr) + (dtd - dt) * (dtd - dt);
//			another way to compute the error:
//			image coordinates predicted using affine + radial +	tangential
			xdp = xd + (drd * xid - dtd * yid) / r;
			ydp = yd + (drd * yid + dtd * xid) / r;
			dx = xi - xdp;
			dy = yi - ydp;
			sum += dx * dx + dy * dy;
		}
	}
	printf("st. dev. %lg for %d BBs\n", sqrt(sum / bbtotal), bbtotal);
	return sqrt(sum / bbtotal);
}

#ifdef IGNORED

// check error remaining after radial and tangential fit
// applies correction using predicted image coordinates

double checkerror2 (double **bbcoords, double **iicoords, int bbtotal,
				   double ax[3], double ay[3], double cr[3], double ct[3]) {
	int k;
	double xi, yi, xb, yb, xid, yid;
	double dx, dy;
	double sum=0.0;
	for (k = 0; k < bbtotal; k++) {
		xi = iicoords[k][0];		// image_x
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		predict (xb, yb, &xid, &yid, ax, ay, cr, ct);
		dx = xid - xi;
		dy = yid - yi;
		sum += dx * dx + dy * dy;
	}
	printf("st. dev. %lg for %d BBs\n", sqrt(sum / bbtotal), bbtotal);
	return sqrt(sum / bbtotal);
}

#endif

/***********************************************************************************/

void copyimagein (unsigned char **image, int imageheight, int imagewidth,
				  double **blurr) {
	int i, j;
	for (i = 0; i < imageheight; i++) {
		for (j = 0; j < imagewidth; j++)	{
			blurr[i][j] = image[i][j];
		}
	}
}

void copyimageout (double **blurr, int imageheight, int imagewidth,
				   unsigned char **image) {
	int i, j;
	for (i = 0; i < imageheight; i++) {
		for (j = 0; j < imagewidth; j++)	{
			image[i][j] = (unsigned char) (blurr[i][j] + 0.5);
		}
	}
}

/********************************************************************/

// Convolve image n times with second order binomial smoothing filter
//
// 1 2 1
// 2 4 2
// 1 2 1
//
// If you do not rescale, average level grows by factor of 16 each time.
// Would need more bits to represent result, but no loss by rounding.
// So for 16-bit (SHORT) result can do this twice only before overflow.
// And for 32-bit (INT) result can do this up to six times.
// Floating point version does rescale...
//
// At edges need to pretend that pixels one row/column outside the
// "frame" have the same grey values as those on the frame.
// (Otherwise the frame becomes darker relative to the rest).

#ifdef IGNORED
int blurrimage1 (int n) {
	int i, j, iter;

	for (iter = 0; iter < n; iter++) {
//		Apply horizontal filter (1 2 1)
		for (i = 0; i < imageheight; i++) {
			for (j = 0; j < imagewidth-1; j++) 
				blurr[i][j] = (blurr[i][j] + blurr[i][j+1]) * 0.5;
//			blurr[i][imagewidth-1] = blurr[i][imagewidth-1] + blurr[i][imagewidth-1];
			for (j = imagewidth-1; j > 0; j--) 
				blurr[i][j] = (blurr[i][j] + blurr[i][j-1]) * 0.5;
//			blurr[i][0] = blurr[i][0] + blurr[i][0];
		}
//		Apply vertical filter (1 2 1)
		for (j = 0; j < imagewidth; j++) {
			for (i = 0; i < imageheight-1; i++) 
				blurr[i][j] = (blurr[i][j] + blurr[i+1][j]) * 0.5;
//			blurr[imageheight-1][j] = blurr[imageheight-1][j] + blurr[imageheight-1][j];			
			for (i = imageheight-1; i > 0; i--) 
				blurr[i][j] = (blurr[i][j] + blurr[i-1][j]) * 0.5;
//			blurr[0][j] = blurr[0][j] + blurr[0][j];
		}
	}
	return 0;
}
#endif

#ifdef IGNORED
int blurrimage2 (int n) {
	int i, j, iter;
	double old, new;

	for (iter = 0; iter < n; iter++) {
//		Apply horizontal filter (1 2 1)
		for (i = 0; i < imageheight; i++) {
			old = blurr[i][0];
			for (j = 0; j < imagewidth-1; j++) {
				new = blurr[i][j+1];
				blurr[i][j] = (old + new) * 0.5;
				old = new;
			}
			old = blurr[i][imagewidth-1];
			for (j = imagewidth-1; j > 0; j--) {
				new = blurr[i][j-1];
				blurr[i][j] = (old + new) * 0.5;
				old = new;
			}
		}
//		Apply vertical filter (1 2 1)
		for (j = 0; j < imagewidth; j++) {
			old = blurr[0][j];
			for (i = 0; i < imageheight-1; i++) {
				new = blurr[i+1][j];
				blurr[i][j] = (old + new) * 0.5;
				old = new;
			}
			old = blurr[imageheight-1][j];
			for (i = imageheight-1; i > 0; i--) {
				new = blurr[i-1][j];
				blurr[i][j] = (old + new) * 0.5;
				old = new;
			}
		}
	}
	return 0;
}
#endif

// Slighlty faster version passes over image 1/2 number of times

// int blurrimage3 (int n) 
int blurrimage (double **blurr, int imageheight, int imagewidth, int n) {
	int i, j, iter;
	double old, current, new;

	for (iter = 0; iter < n; iter++) {
//		Apply horizontal filter (1 2 1) / 4
		for (i = 0; i < imageheight; i++) {
			old = blurr[i][0];
			current = blurr[i][1];
			for (j = 1; j < imagewidth-1; j++) {
				new = blurr[i][j+1];
				blurr[i][j] = (old + current + current + new) * 0.25;
				old = current;
				current = new;
			}
		}
//		Apply vertical filter (1 2 1) / 4
		for (j = 0; j < imagewidth; j++) {
			old = blurr[0][j];
			current = blurr[1][j];
			for (i = 1; i < imageheight-1; i++) {
				new = blurr[i+1][j];
				blurr[i][j] = (old + current + current + new) * 0.25;
				old = current;
				current = new;
			}
		}
	}
	return 0;
}

#ifdef IGNORED
int blurrimage (int n) {
	int ret;
	clock_t sclock, eclock;
	sclock = clock();
//	ret = blurrimage1(n);
//	ret = blurrimage2(n);
	ret = blurrimage3(n);
	eclock = clock();
	printf("Smoothing took %lg sec\n", (double) (eclock - sclock) / CLOCKS_PER_SEC);
	return ret;
}
#endif

/******************************************************************************/

// code used to locate the minima in the circuit board pictures

// Is (i, j) a point of local minimum ?

int checklocalmin (double **blurr, int i, int j) {
//	int center = blurr[i][j];
	double center; 
	if (i <= 0 || i >= imageheight-1 ||
		  j <= 0 || j >= imagewidth-1) {
		printf("checklocalmin error: %d %d\n", i, j);
		return 0;
	}
	center = blurr[i][j];
	if (blurr[i][j+1] < center ||
		  blurr[i][j-1] < center ||
		  blurr[i+1][j] < center ||
		  blurr[i-1][j] < center) return 0;
	if (fuzzyflag) {
		if (blurr[i+1][j+1] < center ||
			  blurr[i+1][j-1] < center ||
			  blurr[i-1][j+1] < center ||
			  blurr[i-1][j-1] < center) return 0;
	}
	return -1;			// yes, it is a local minimum
}

// Is (i, j) a point of local maximum ?

int checklocalmax (double **blurr, int i, int j) {
//	int center = blurr[i][j];
	double center; 
	if (i <= 0 || i >= imageheight-1 ||
		  j <= 0 || j >= imagewidth-1) {
		printf("checklocalmax error: %d %d\n", i, j);
		return 0;
	}
	center = blurr[i][j];
	if (blurr[i][j+1] > center ||
		  blurr[i][j-1] > center ||
		  blurr[i+1][j] > center ||
		  blurr[i-1][j] > center) return 0;
	if (fuzzyflag) {
		if (blurr[i+1][j+1] > center ||
			  blurr[i+1][j-1] > center ||
			  blurr[i-1][j+1] > center ||
			  blurr[i-1][j-1] > center) return 0;
	}
	return -1;			// yes, it is a local minimum
}

// find extremum of second order surface given by
// z = a + b * x + c * y + d * x * x + 2 * e * x * y + f * y * y

int secondorderextremum (double b, double c, double d, double e, double f,
					   double *x, double *y) {
	double det = d * f - e * e;
	double detx = (c * e - b * f) / 2;
	double dety = (b * e - c * d) / 2;
	if (det == 0.0) {
		if (traceflag)
			printf("Zero determinant in second order extremum\n");
		return -1;
	}
	if (traceflag)
		printf("detx %lg dety %lg det %lg\n", detx, dety, det);
	*x = detx / det;
	*y = dety / det;
	return 0;						// success
}

// Fit second order surface	 (for sub-pixel BB image location)
// z = a + b * x + c * y + d * x * x + 2 * e * x * y + f * y * y
// to 3 x 3 neighborhood (es=1)
// or 5 x 5 neighborhood (es=2)

int fitsecondorder (double **blurr, int i, int j, double *a, double *b, double *c, double *d, double *e, double *f) {
	double sum=0, sumx=0, sumy=0, sumxx=0, sumxy=0, sumyy=0;
	double sumxxx=0, sumxxy=0, sumxyy=0, sumyyy=0;
	double sumxxxx=0, sumxxxy=0, sumxxyy=0, sumxyyy=0, sumyyyy=0;
	double sumz=0, sumzx=0, sumzy=0, sumzxx=0, sumzxy=0, sumzyy=0;
	double x, y, z;
	int k, ik, jk, ret;
	int es=1;
//	int es=2;
	double *m[6];
	double mat[6 * 6];
	double r[6], s[6];
	for (k = 0; k < 6; k++) m[k] = mat + 6 * k;	// margin array
	if (i <= 0 || i >= imageheight-1 ||
		  j <= 0 || j >= imagewidth-1) return -1;	// failure
	for (ik = i-es; ik <= i+es; ik++) {
		for (jk = j-es; jk <= j+es; jk++) {
			x = jk - j;
			y = ik - i;
			sum += 1;
			sumx += x;
			sumy += y;
			sumxx += x * x;
			sumxy += x * y;
			sumyy += y * y;
			sumxxx += x * x * x;
			sumxxy += x * x * y;
			sumxyy += x * y * y;
			sumyyy += y * y * y;
			sumxxxx += x * x *  x * x;
			sumxxxy += x * x *  x * y;
			sumxxyy += x * x *  y * y;
			sumxyyy += x * y *  y * y;
			sumyyyy += y * y *  y * y;
			z = blurr[ik][jk];
			sumz += z;
			sumzx += z * x;
			sumzy += z * y;
			sumzxx += z * x * x;
			sumzxy += z * x * y;
			sumzyy += z * y * y;
		}
	}
	m[0][0] = sum;
	m[0][1] = m[1][0] = sumx;
	m[0][2] = m[2][0] = sumy;
	m[0][3] = m[3][0] = sumxx;
	m[0][4] = m[4][0] = sumxy;
	m[0][5] = m[5][0] = sumyy;
	m[1][1] = sumxx;
	m[1][2] = m[2][1] = sumxy;
	m[1][3] = m[3][1] = sumxxx;
	m[1][4] = m[4][1] = sumxxy;
	m[1][5] = m[5][1] = sumxyy;
	m[2][2] = sumyy;
	m[2][3] = m[3][2] = sumxxy;
	m[2][4] = m[4][2] = sumxyy;
	m[2][5] = m[5][2] = sumyyy;
	m[3][3] = sumxxxx;
	m[3][4] = m[4][3] = sumxxxy;
	m[3][5] = m[5][3] = sumxxyy;
	m[4][4] = sumxxyy;
	m[4][5] = m[5][4] = sumxyyy;
	m[5][5] = sumyyyy;
	r[0] = sumz;
	r[1] = sumzx;
	r[2] = sumzy;
	r[3] = sumzxx;
	r[4] = sumzxy;
	r[5] = sumzyy;
	ret = gauss_solve (m, 6, r, s);
	if (ret < 0) return ret;	// failure
	*a = s[0];
	*b = s[1];
	*c = s[2];
	*d = s[3];
	*e = s[4]/2;
	*f = s[5];
	return 0;			// success
}

void showneighbourhood (double **blurr, int i, int j) {
	int ik, jk, es=3;

	printf("  %4d\t", 0);
	for (jk = j-es; jk <= j+es; jk++) {
		printf("%9d ", jk);
	}
	printf("\n");
	for (ik = i-es; ik <= i+es; ik++) {
		printf("i %4d\t", ik);
		for (jk = j-es; jk <= j+es; jk++) {
			printf("%9lg ", blurr[ik][jk]);
		}
		printf("\n");
	}
}

int checkbackground (double **blurr, int i, int j) {
	int ik, jk, es=1;
	for (ik = i-es; ik <= i+es; ik++) {
		for (jk = j-es; jk <= j+es; jk++) {
			if (blurr[ik][jk] > 0) return 0;
		}
	}
	if (traceflag) printf("BACKGROUND\n");
	return -1;
}

// Find local minimum in blurred array near (i, j)
// returns result in last two arguments

int findlocalmin (double **blurr, int i, int j, double *x, double *y) {
	int ik, jk, ret, found=0;
	double a, b, c, d, e, f;
	double xd, yd;
	double es=1.0;
	if (i <= 0 || i >= imageheight-1 ||
		  j <= 0 || j >= imagewidth-1) {
		printf("findlocalmin error: %d %d\n", i, j);
		return -1;
	}
	ik = i;
	jk= j;
	if (checkbackground(blurr, i, j)) return -1;
	if (checklocalmin(blurr, ik, jk)) found = 1;
//	showneighbourhood(blurr, i, j);	// debugging
	if (found == 0) {
		for (ik = i-1; ik <= i+1; ik++) {
			for (jk = j-1; jk <= j+1; jk++) {
				if (ik == i && jk == j) continue;
				if (checklocalmin(blurr, ik, jk)) {
					found = 1;
					break;
				}
			}
			if (found) break;
		}
	}
//	showneighbourhood(blurr, i, j);	// debugging
	if (found == 0) {
		for (ik = i-2; ik <= i+2; ik++) {
			for (jk = j-2; jk <= j+2; jk++) {
				if ((i-1 <= ik) && (ik <= i+1) &&
					  (j-1 <= jk) && (jk <= j+1)) continue;
				if (checklocalmin(blurr, ik, jk)) {
					found = 1;
					break;
				}
			}
			if (found) break;
		}
	}
	if (found == 0) {
		if (traceflag)
			printf("No local min at or near x %d y %d (PSP %d %d)\n", j, i, j, (imageheight-1)-i);
//		if (verboseflag) showneighbourhood(blurr, i, j);
		*x = j;
		*y = i;
		return -1;
	}
	ret = fitsecondorder(blurr, ik, jk, &a, &b, &c, &d, &e, &f);
	if (ret < 0) return ret;	// failure
	ret = secondorderextremum(b, c, d, e, f, &xd, &yd);
	if (ret < 0) return ret;	// failure
	if (xd < -es || xd > es || yd < -es || yd > es) {
		printf("Solution too far away dx %lg dy %lg\n", xd, yd);
//		if (verboseflag) showneighbourhood(blurr, ik, jk);
		return -1;
	}
	*x = jk + xd;
	*y = ik + yd;
	return 0;					// success
}

// Find local maximum in blurred array near (i, j)
// returns result in last two arguments

int findlocalmax (double **blurr, int i, int j, double *x, double *y) {
	int ik, jk, ret, found=0;
	double a, b, c, d, e, f;
	double xd, yd;
	double es=1.0;
	if (i <= 0 || i >= imageheight-1 ||
		  j <= 0 || j >= imagewidth-1) {
		printf("findlocalmax error: %d %d\n", i, j);
		return -1;
	}
	ik = i;
	jk= j;
	if (checkbackground(blurr, i, j)) return -1;
	if (checklocalmax(blurr, ik, jk)) {
//		printf("FOUND %d %d\n", ik, jk);
		found = 1;
	}
	if (found == 0) {
//		printf("Trying in 3 x 3 %d %d\n", i, j);
//		showneighbourhood(blurr, i, j);	// debugging
		for (ik = i-1; ik <= i+1; ik++) {
			for (jk = j-1; jk <= j+1; jk++) {
				if (ik == i && jk == j) continue;
				if (checklocalmax(blurr, ik, jk)) {
//					printf("FOUND %d %d\n", ik, jk);
					found = 1;
					break;
				}
			}
			if (found) break;
		}
	}
	if (found == 0) {
//		printf("Trying in 5 x 5 %d %d\n", i, j);
//		showneighbourhood(blurr, i, j);	// debugging
		for (ik = i-2; ik <= i+2; ik++) {
			for (jk = j-2; jk <= j+2; jk++) {
				if ((i-1 <= ik) && (ik <= i+1) &&
					  (j-1 <= jk) && (jk <= j+1)) continue;
				if (checklocalmax(blurr, ik, jk)) {
//					printf("FOUND %d %d\n", ik, jk);
					found = 1;
					break;
				}
			}
			if (found) break;
		}
	}
	if (found == 0) {
		if (traceflag)
			printf("No local max at or near x %d y %d (PSP %d %d)\n", j, i, j, (imageheight-1)-i);
//		if (verboseflag) showneighbourhood(blurr, i, j);
		*x = j;
		*y = i;
		return -1;
	}
//	printf("LOCALMAX ANSWER %d %d\n", ik, jk);
//	showneighbourhood(blurr, ik, jk);
	ret = fitsecondorder(blurr, ik, jk, &a, &b, &c, &d, &e, &f);
	if (ret < 0) return ret;	// failure
	ret = secondorderextremum(b, c, d, e, f, &xd, &yd);
	if (ret < 0) return ret;	// failure
	if (xd < -es || xd > es || yd < -es || yd > es) {
//		if (verboseflag) printf("Solution too far away dx %lg dy %lg\n", xd, yd);
//		if (verboseflag) showneighbourhood(blurr, ik, jk);
		return -1;
	}
	*x = jk + xd;
	*y = ik + yd;
//	printf("FINAL ANSWER %d %d\n", (int) (*y+0.5), (int) (*x+0.5));
//	showneighbourhood(blurr, (int) (*y+0.5), (int) (*x+0.5));
	return 0;					// success
}

// test function not used

int testminimumfind (double xo, double yo) {
	int i, j;
	int ik, jk;
	double alpha=2, beta=1, gamma=4;
	double a, b, c, d, e, f;
	double x, y;
	double **blurr=NULL;

	imageheight = imagewidth = 980;
	blurr = setupblurrarray(imageheight, imagewidth);
	if (blurr == NULL) return -1;
	ik = (int) (yo + 0.5);
	jk = (int) (xo + 0.5);
	a = alpha * xo * xo + beta * xo * yo + gamma * yo * yo;
	b = - 2 * alpha * xo - beta * yo;
	c = - 2 * gamma * yo - beta * xo;
	d = alpha;
	e = beta/2;
	f = gamma;
//	a = 1000.0;
	
	for (i = 0; i < imageheight; i++) {
		for (j = 0; j < imagewidth; j++) {
			x = j;
			y = i;
			blurr[i][j] =
					a + b * x + c * y + d * x * x + 2 * e * x * y + f * y * y;
		}
	}
	if (findlocalmin(blurr, ik, jk, &x, &y)) return -1;	// failure
	printf("xo %lg yo %lg x %lg y %lg\n", xo, yo, x, y);
	return 0;
}

// m[i][j] => j corresponds to x, positive to the right
// m[i][j] => i corresponds to y, positive down

// create matrix of coordinates from list of measured data 
// for image001.bmp

int setupdatafine (double **bbcoords, double **iicoords, int bbmax, double **blurr) {
	int ik, jk, k, kk=0, bad=0;
	double x, y;
//	int targetwidth=12;
	int targetheight=9;

	if (verboseflag) printf("Setupdatafine %d\n", bbmax);
	if (blurr == NULL) {
		printf("Need to set up blurred data array first\n");
		return -1;
	}
	imageheight = imagewidth = 980;
	for (k = 0; k < bbmax; k++) {
		if (bidata[k][0] == 0 && bidata[k][1] == 0) continue;
		jk = bidata[k][0];				// image_x
		ik = (imageheight-1) - bidata[k][1];	// image_y
		if (findlocalmin(blurr, ik, jk, &x, &y)) {
			x = jk; y = ik;
			bad++;
		}
		else {
			if (traceflag) printf("%4d %4d\t%lg %lg\t%lg %lg\n", ik, jk, y, x, y-ik, x-jk);
		}
//		coordinates in image
		iicoords[kk][0] = x;		// refined image_x
		iicoords[kk][1] = y;		// refined image_y
//		coordinates in target
		bbcoords[kk][0] = bidata[k][2];	// target_x
		bbcoords[kk][1] = (targetheight-1) - bidata[k][3];	// target_y
		kk++;
	}
	if (bad > 0) printf("%d out of %d BBs not located\n", bad, kk);
	return kk;
}

/***********************************************************************************/

void showBBdata (double **bbcoords, double **iicoords, int bbtotal) {
	int k;
	for (k = 0; k < bbtotal; k++) {
		printf("%lg %lg target => %lg %lg image\n",
			   bbcoords[k][0], bbcoords[k][1],
			   iicoords[k][0], iicoords[k][1]);
	}
}

/***********************************************************************************/
/***********************************************************************************/

//  Fitting radial and tangential distortion

int SecondOrderFit (double **bbcoords, double **iicoords, int bbtotal,
					double ax[3], double ay[3], double cr[3], double ct[3]) {
	double m[3][3];

	if (verboseflag) printf("Doing the affine fitting\n");
	if (make_affine_matrix(bbcoords, bbtotal, m) < 0) return -1;

	if (verboseflag) printf("Fit for x:\n");
	affinefitx(bbcoords, iicoords, bbtotal, m, ax);		// first row of transformation
	if (verboseflag) printf("Fit for y:\n");
	affinefity(bbcoords, iicoords, bbtotal, m, ay);		// second row of transformation
	checkaffinefit(bbcoords, iicoords, bbtotal, ax, ay);
	if (verboseflag) printf("\n");

//	checkerror(bbcoords, iicoords, bbtotal, ax, ay, zero, zero);

	if (verboseflag) printf("Doing the radial and tangential fitting\n");
	if (make_distort_matrix(iicoords, bbtotal, m) < 0) return -1;

	if (verboseflag) printf("Radial     Fit: ");
	radialfit(bbcoords, iicoords, bbtotal, ax, ay, m, cr);
	if (verboseflag) printf("\n");
	if (verboseflag) printf("Tangential Fit: ");
	tangentialfit(bbcoords, iicoords, bbtotal, ax, ay, m, ct);
	if (verboseflag) printf("\n");
	return 0;
}

// Check the second order fit

double CheckSecondOrderFit (double **bbcoords, double **iicoords, int bbtotal,
							double ax[3], double ay[3], double cr[3], double ct[3]) {	
	double zero[3] = {0.0, 0.0, 0.0};
	double ret;
	if (verboseflag) printf("no correction:\n");
	checkerror(bbcoords, iicoords, bbtotal, ax, ay, zero, zero);
	if (verboseflag) printf("radial correction only:\n");
	checkerror(bbcoords, iicoords, bbtotal, ax, ay, cr, zero);
	if (verboseflag) printf("tangential correction only:\n");
	checkerror(bbcoords, iicoords, bbtotal, ax, ay, zero, ct);
	if (verboseflag) printf("radial and tangential correction:\n");
	ret = checkerror(bbcoords, iicoords, bbtotal, ax, ay, cr, ct);
	return ret;
}

// Construct inverse of affine transform ax[3], ay[3] in iax[3], iay[3]

int InvertAffine (double ax[3], double ay[3], double iax[3], double iay[3]) {
	double det;
	det = ax[0] * ay[1] - ax[1] * ay[0];
	if (det == 0.0) {
		printf("ERROR: determinant is zero (InvertAffine)\n");
		return -1;
	}
	iax[0] =   ay[1] / det;
	iax[1] = - ax[1] / det;
	iay[0] = - ay[0] / det;
	iay[1] =   ax[0] / det;
	iax[2] = - (iax[0] * ax[2] + iax[1] * ay[2]);
	iay[2] = - (iay[0] * ax[2] + iay[1] * ay[2]);
	return 0;
}

//  Correct for radial and tangential distortion
//	bbcoords	input image coordinates
//	ax, ay		affine transformation coefficients
//	cr			radial distortion coefficients
//	ct			tangential distortion coefficients
//	xxcoords	output (corrected) image coordinates

int CorrectForSecondOrder (double **bbcoords, int bbtotal,
							double ax[3], double ay[3], double cr[3], double ct[3],
							double **xxcoords) {
	int k;
//	double xi, yi;
	double xid, yid, xip, yip, drd, dtd, r;
	double xb, yb, xd, yd;
	double iax[3], iay[3];

	if (InvertAffine(ax, ay, iax, iay) < 0) return -1;
	
	for (k = 0; k < bbtotal; k++) {
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
//		xd = ax[0] * xb + ax[1] * yb + ax[2];
		xd = iax[0] * xb + iax[1] * yb + iax[2];
//		yd = ay[0] * xb + ay[1] * yb + ay[2];
		yd = iay[0] * xb + iay[1] * yb + iay[2];
		xid = xd - xo;				// relative to center
		yid = yd - yo;				// relative to center
		r = sqrt(xid * xid + yid * yid);
		if (r != 0.0) {
			if (suppressradial) drd = 0.0;
			else drd = cr[0] * r * r + cr[1] * r + cr[2];	// radial correction
			if (suppresstangential) dtd = 0.0;
			else dtd = ct[0] * r * r + ct[1] * r + ct[2];	// tangential correction
//			xip = xd + (drd * xid - dtd * yid) / r;
			xip = xd - (drd * xid - dtd * yid) / r;
//			yip = yd + (drd * yid + dtd * xid) / r;
			yip = yd - (drd * yid + dtd * xid) / r;
		}
		else {
			xip = xd;
			yip = yd;
		}
		xxcoords[k][0] = xip;	// x corrected for second order distortion
		xxcoords[k][1] = yip;	// y corrected for second order distortion
	}	
	return 0;
}

// compenstateForTangentialDistortion(
//									double **realBBlocn, 
// the "real" 2d image locations of bbs that were input to Tsai
//									double **tsaiBBlocn, 
// the "estimated" 2d image locns that are projection of 3d bb locations using Tsai 
//									int BBtotal
// the number of BBs
//									double **correctedBBlocn,
// the corrected 2d locations that are output by the method
//									double cr[3], ct[3]);
// the coefficients of the correction that were used to achieve above result.)

// NOTE: global variable suppressradial can be set to suppress radial correction.
// NOTE: global variable suppresstangential can be set to suppress tangential correction.

int compensateForTangentialDistortion (double **realBBlocn, double **tsaiBBlocn, int nBB,
						double **correctedBBlocn, double cr[3], double ct[3]) {
	double ax[3], ay[3];	// place for affine transformation coefficients
	if (SecondOrderFit(tsaiBBlocn, realBBlocn, nBB, ax, ay, cr, ct) < 0) {
		printf("ERROR: SecondOrderFit failed\n");
		return -1;
	}
	CorrectForSecondOrder(realBBlocn, nBB, ax, ay, cr, ct, correctedBBlocn);
	return 0;
}


/********************************************************************************/
/***********************************************************************************/

void FlipSignVector (double foo[3]) {
	foo[0] = - foo[0];	foo[1] = - foo[1];	foo[2] = - foo[2];
}

// Create test data for above method
// Add second order distortions to tsaiBBlocn to create realBBlocn
// using radial distortion c[3] and tangential distortion ct[3]

int CreateDistortionTest (double **tsaiBBlocn, int nBB,
						  double **realBBlocn,
						  double ax[3], double ay[3], double cr[3], double ct[3]) {
	FlipSignVector(cr);	FlipSignVector(ct);
	CorrectForSecondOrder(tsaiBBlocn, nBB, ax, ay, cr, ct, realBBlocn);
	FlipSignVector(cr);	FlipSignVector(ct);
	return 0;
}

void showcompensation(double **tsaiBBlcon, double **realBBlcon, double **correctedBBlcon,
					  int nBB) {
	int k;
	for (k =- 0; k < nBB; k++) {
		printf("ORG %9lg %9lg DIS %9lg %9lg COR %9lg %9lg\n",
			   tsaiBBlcon[k][0], tsaiBBlcon[k][1],
			   realBBlcon[k][0], realBBlcon[k][1],
			   correctedBBlcon[k][0], correctedBBlcon[k][1]);
	}
}

void TestDistortionCompensation (double **tsaiBBlocn, int nBB,
								 double ax[3], double ay[3], double cr[3], double ct[3]) {
	double crd[3], ctd[3];
	double **realBBlocn=NULL;
	double **correctedBBlocn=NULL;

	realBBlocn = make_matrix_nm(nBB, 2);
	correctedBBlocn = make_matrix_nm(nBB, 2);
	CreateDistortionTest(tsaiBBlocn, nBB, realBBlocn, ax, ay, cr, ct);
	compensateForTangentialDistortion(realBBlocn, tsaiBBlocn, nBB,
									   correctedBBlocn, crd, ctd);
	printf(" cr[0] %9lg  cr[1] %9lg  cr[2] %9lg\n", cr[0], cr[1], cr[2]);
	printf(" ct[0] %9lg  ct[1] %9lg  ct[2] %9lg\n", ct[0], ct[1], ct[2]);
	printf("crd[0] %9lg crd[1] %9lg crd[2] %9lg\n", crd[0], crd[1], crd[2]);
	printf("ctd[0] %9lg ctd[1] %9lg ctd[2] %9lg\n", ctd[0], ctd[1], ctd[2]);
	showcompensation(tsaiBBlocn, realBBlocn, correctedBBlocn, nBB);
    free_matrix_nm(correctedBBlocn, nBB, 2);
	free_matrix_nm(realBBlocn, nBB, 2);
}


/********************************************************************************/

// Computer power at given spatial frequency (in cycles per pixel)

// for image003.bmp get max at (.01319 .08842)
// this means spots are spaced (1.650 11.063) pixels in one direction
// for image003.bmp get max at (.08805 .01315)
// this means spots are spaced (11.109 -1.659) pixels in other direction

double powerat (double **image, double u, double v) {
	int i, j;
	double x, y, w, sumc=0.0, sums= 0.0;
	for (i = 0; i < imageheight; i++) {
		for (j = 0; j < imagewidth; j++) {
			x = j - xo;
			y = i - yo;
			w = 2.0 * PI * (u * x + v * y);
			if (image[i][j] > 0) {
				sumc += image[i][j] * cos(w);
				sums += image[i][j] * sin(w);
			}
		}
	}
	return (sqrt(sumc * sumc + sums * sums)) / (imagewidth * imageheight);
}


// Search for a local maximum near given point --- not used 

int scanformax (double **blurr, int io, int jo, int *in, int *jn) {
	int i, j, es;
	if (checklocalmax(blurr, io, jo)) {
		*in = io;
		*jn = jo;
		return 0;
	}
	for (es = 1; es < 16; es++) {
		for (i = io - es; i <= io + es; i++) {
			for (j = jo - es; j <= jo + es; j++) {
				if (checklocalmax(blurr, i, j)) {
					*in = i;
					*jn = j;
					printf("FOUND MAX AT %d %d\n", i, j);
					return 0;
				}
			}
		}
	}
	return -1;
}

/***********************************************************************************/

// Code to find all the local minima in circuit board images

// grid spacing in two orthogonal directions in image003.bmp
// estimated by hand from PaintShopPro image displace

double dx1=1.650, dy1=11.063;
double dx2=11.109, dy2=-1.659;

// this means spots are spaced (1.650 11.063) pixels in one direction
// this means spots are spaced (11.109 -1.659) pixels in other direction

int makegrid (char *filename,
			  double **bbcoords, double **iicoords, int bbmax, double **blurr) {
	int l, m, kk=0;
	double xc, yc, x,  y, xo, yo, xn, yn;

//	imax=65;
//	jmax=85;			// limits for image003.bmp
//	ioff=33;
//	joff=40;			// to get non-negative subscripts

//	485, 489 in PSP is x, (imageheight-y) -- arbitrary "image center" BB
//	Give manually determined coords of some point near center of image
	if (strstr(filename, "003") != NULL) {
		yc = (imageheight-1)-489;
		xc =  485;
	}
	else if (strstr(filename, "004") != NULL) {
		yc = (imageheight-1)-491;
		xc =  483;
	}
	else if (strstr(filename, "005") != NULL) {
		yc = (imageheight-1)-484;
		xc =  486;
	}
	else if (strstr(filename, "006") != NULL) {
		yc = (imageheight-1)-484;
		xc =  481;
	}
	else {
		printf("Unknown input file\n");
		exit(1);
	}
	
	xo = xc;
	yo = yc;
	for (l = 0; l < 256; l++) {
		x = xo;
		y = yo;
		if (findlocalmax(blurr, (int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
			break;
		}
		x = xo = xn;
		y = yo = yn;
		for (m = 0; m < 256; m++) {
			if (findlocalmax(blurr, (int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
				if (traceflag)
					printf("FOUND %d spots starting from %d (+)\n", m, l);
				break;
			}
//			if (verboseflag) printf("Local max at x %lg y %lg (PSP %lg %lg)\n",  xn, yn, xn, (imageheight-1) - yn);
//			i = ioff+l;
//			j = joff+k;
//			if (i < 0 || i >= imax || j < 0 || j >= jmax) 
//				printf("ERROR: %d %d\n", i, j);
//			else {
				iicoords[kk][0] = xn;
				iicoords[kk][1] = yn;
				bbcoords[kk][0] = l;
				bbcoords[kk][1] = m;
				if (kk++ >= bbmax-1) {
					printf("TOO MANY BBs %d\n", kk);
					exit(1);
				}
//			}
			x = xn + dx1;
			y = yn + dy1;
//			if (verboseflag) printf("NEW PLACE at x %lg y %lg (PSP %lg %lg)\n", x, y, x, (imageheight-1) - y);
		}
		x = xo;
		y = yo;
		for (m = 0; m < 256; m++) {
			if (findlocalmax(blurr, (int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
				if (traceflag)
					printf("FOUND %d spots starting from %d (-)\n", m, l);
				break;
			}
//			if (verboseflag) printf("Local max at x %lg y %lg (PSP %lg %lg)\n", xn, yn, xn, (imageheight-1) - yn);
//			i = ioff+l;
//			j = joff-k;
//			if (i < 0 || i >= imax || j < 0 || j >= jmax) 
//				printf("ERROR: %d %d\n", i, j);
//			else {
				iicoords[kk][0] = xn;
				iicoords[kk][1] = yn;
				bbcoords[kk][0] = l;
				bbcoords[kk][1] = -m;
				if (kk++ >= bbmax-1) {
					printf("TOO MANY BBs %d\n", kk);
					exit(1);
				}
//			}
			x = xn - dx1;
			y = yn - dy1;
//			if (verboseflag) printf("NEW PLACE at x %lg y %lg (PSP %lg %lg)\n", x, y, x, (imageheight-1) - y);
		}
		xo = xo + dx2;
		yo = yo + dy2;
	}
	printf("DONE %d columns (+)\n", l);
//	yo = (imageheight-1)-489;
//	xo =  485;
	xo = xc;
	yo = yc;
	for (l = 0; l < 256; l++) {
		x = xo;
		y = yo;
		if (findlocalmax(blurr, (int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
			break;
		}
		x = xo = xn;
		y = yo = yn;
		for (m = 0; m < 256; m++) {
			if (findlocalmax(blurr, (int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
				if (traceflag)
					printf("FOUND %d spots starting from %d (+)\n", m, l);
				break;
			}
//			if (verboseflag) printf("Local max at x %lg y %lg (PSP %lg %lg)\n", xn, yn, xn, (imageheight-1) - yn);
//			i = ioff-l;
//			j = joff+k;
//			if (i < 0 || i >= imax || j < 0 || j >= jmax) 
//				printf("ERROR: %d %d\n", i, j);
//			else {
				iicoords[kk][0] = xn;
				iicoords[kk][1] = yn;
				bbcoords[kk][0] = -l;
				bbcoords[kk][1] = m;
				if (kk++ >= bbmax-1) {
					printf("TOO MANY BBs %d\n", kk);
					exit(1);
				}
//			}
			x = xn + dx1;
			y = yn + dy1;
//			if (verboseflag) printf("NEW PLACE at x %lg y %lg (PSP %lg %lg)\n", x, y, x, (imageheight-1) - y);
		}
		x = xo;
		y = yo;
		for (m = 0; m < 256; m++) {
			if (findlocalmax(blurr, (int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
				if (traceflag)
					printf("FOUND %d spots starting from %d (-)\n", m, l);
				break;
			}
//			if (verboseflag) printf("Local max at x %lg y %lg (PSP %lg %lg)\n", xn, yn, xn, (imageheight-1) - yn);
//			i = ioff-l;
//			j = joff-k;
//			if (i < 0 || i >= imax || j < 0 || j >= jmax) 
//				printf("ERROR: %d %d\n", i, j);
//			else {
				iicoords[kk][0] = xn;
				iicoords[kk][1] = yn;
				bbcoords[kk][0] = -l;
				bbcoords[kk][1] = -m;
				if (kk++ >= bbmax-1) {
					printf("TOO MANY BBs %d\n", kk);
					exit(1);
				}
//			}
			x = xn - dx1;
			y = yn - dy1;
//			if (verboseflag) printf("NEW PLACE at x %lg y %lg (PSP %lg %lg)\n", x, y, x, (imageheight-1) - y);
		}
		xo = xo - dx2;
		yo = yo - dy2;
	}
	printf("DONE %d columns (-)\n", l);
	printf("\n");
//	showBBdata(bbcoords, iicoords, bbtotal);
	if (kk < 3) {
		printf("TOO FEW BBS %d\n", kk);
		return -1;
	}
	return kk;
}


/*******************************************************************************/

int extend_with_s (char *filename) {
	char *s;
	int n;
	if ((s = strrchr(filename, '.')) != NULL) {
		n = strlen(s);
		memmove(s+1, s, n+1);
		*s = 's';
		return 0;
	}
	else {
		printf("ERROR: %s\n", filename);
		return -1;
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

/******************************************************************************/

char *infile1="image001.bmp";	// default for sparseflag=1

char *infile3="image003.bmp";	// default for sparseflag=0

int main(int argc, char *argv[]) {
	char infile[FILENAME_MAX];
	char outfile[FILENAME_MAX];
	int bbmax, bbtotal, ngauss;
	double **iicoords=NULL;
	double **bbcoords=NULL;
	unsigned char **image=NULL;
	double **blurr=NULL;
//	int histogram[256];

	if (1) {
		double ax[3], ay[3];
		double cr[3], ct[3];
		ax[0] = 1.0; ax[1] = 0.0; ax[2] = 0.0;
		ay[0] = 0.0; ay[1] = 1.0; ax[2] = 0.0;
		cr[0] = 0.00001; cr[1] = 0.0; cr[2] = 0.0;
		ct[0] = 0.00002; ct[1] = 0.0; ct[2] = 0.0;
		bbmax = sizeof(bidata) / sizeof(bidata[0]);
		iicoords = make_matrix_nm (bbmax, 2);
		bbcoords = make_matrix_nm (bbmax, 2);
		bbtotal = setupdata(bidata, bbcoords, iicoords, bbmax);
		TestDistortionCompensation(iicoords, bbtotal, ax, ay, cr, ct);
		free_matrix_nm (bbcoords, bbmax, 2);
		free_matrix_nm (iicoords, bbmax, 2);
		return 0;
	}

//	set up input BMP file name
	if (argc > 1) strcpy(infile, argv[1]);
	else {
		strcpy(infile, "d:\\bmp\\");
		if (sparseflag) strcat(infile, infile1);
		else strcat(infile, infile3);
	}
	extension(infile, "bmp");
	if (verboseflag) {
		printf("Reading %s\n", infile);
		fflush(stdout);
	}

	image = readBMP(infile);
	if (image == NULL) {
		printf("ERROR: image is NULL\n");
		exit(1);
	}
 //	makehistogram(image, histogram);
 //	showhistogram(histogram);
 //	imageheight = imagewidth = 980;
	if (verboseflag) {
		printf("Smoothing image %s\n", infile);
		fflush(stdout);
	}
	blurr = setupblurrarray (imageheight, imagewidth);
	if (blurr == NULL) {
		printf("ERROR: blurr is NULL\n");
		exit(1);
	}
	copyimagein(image, imageheight, imagewidth, blurr);

	if (sparseflag) ngauss = 20;
	else ngauss = 4;
	blurrimage(blurr, imageheight, imagewidth, ngauss);

	if (writesmoothflag) {
		copyimageout(blurr, imageheight, imagewidth, image);
		strcpy(outfile, infile);
		if (extend_with_s(outfile)) {
			printf("ERROR: Missing extension %s\n", outfile);
			exit(1);
		}
		if (verboseflag) {
			printf("Writing %s\n", outfile);
			fflush(stdout);
		}
		writeBMP(outfile, image, imageheight, imagewidth);
	}

	if (sparseflag) bbmax = sizeof(bidata) / sizeof(bidata[0]);
	else bbmax = MAXBBS;
	iicoords = make_matrix_nm (bbmax, 2);
	bbcoords = make_matrix_nm (bbmax, 2);
	if (sparseflag) {
		if (verboseflag) {
			printf("%d potential BB data items\n", bbmax);
			fflush(stdout);
		}
		if (refineflag)
			bbtotal = setupdatafine(bbcoords, iicoords, bbmax, blurr);
		else 
			bbtotal = setupdata(bidata, bbcoords, iicoords, bbmax);
	}
	else bbtotal = makegrid(infile, bbcoords, iicoords, bbmax, blurr);
	if (bbtotal < 0) {
		printf("ERROR: bbtotal %d\n", bbtotal);
		exit(1);
	}
	if (verboseflag) {
		printf("%d actual BB data items\n", bbtotal);
		fflush(stdout);
	}
	if (traceflag) showBBdata(bbcoords, iicoords, bbtotal);
	{
		double ax[3], ay[3];	// affine fit rows
		double cr[3], ct[3];	// radial and tangential fit coefficients
		SecondOrderFit (bbcoords, iicoords, bbtotal, ax, ay, cr, ct);
		CheckSecondOrderFit (bbcoords, iicoords, bbtotal, ax, ay, cr, ct);
	}
	free_matrix_nm (bbcoords, bbmax, 2);
	free_matrix_nm (iicoords, bbmax, 2);
	return 0;
}

/***********************************************************************************/

// Interface specification:

// Array data formats:

// Two-d arrays are set up using margin arrays (somewhat as in
// Numerical Recipes) for fast access and to make dynamic allocation
// possible (important with variable size arrays).

// (The alternative would be the built in language support for
// multi-dimensional arrays, which requires dimension information to be
// known at compile time).

// The function make_matrix_nm can be used to create 2-d arrays set up
// this way (use free_matrix_nm to free the array storage and the
// margin array if needed).

// Lists of (2-D) coordinates:

// Lists of N image coordinates are given as 2-d array with N rows and 
// two columns.  e.g. realBBlocn[34][0] and realBBlocn[34][1] are x and y
// of BB[34] in the image.  (For structure of 2-d array see above).

// The six coefficients of an affine transformation are given as two rows
// ax[3] and ay[3], where
//		xi = ax[0] * xb + ax[1] * yb + ax[2]
//		yi = ay[0] * xb + ay[1] * yb + ay[2]
// The transformation is from (xb, yb) in the model to (xi, yi) in the image.

// The radial distortion correction is given by cr[3], where
//		dr = cr[0] * r * r + cr[1] * r + cr[2]
//	here r is the distance from the center of the image

// The tangential distortion correction is given by ct[3], where
//		dt = ct[0] * r * r + ct[1] * r + ct[2]
//	here r is the distance from the center of the image

// SecondOrderFit(...):

// The function for fitting the transformation is "SecondOrderFit"
//	int SecondOrderFit (double **bbcoords, double **iicoords, int bbtotal,
//					double ax[3], double ay[3], double cr[3], double ct[3]); 
//	INPUTS:
//		bbcoords	array of "model" (target) coordinates
//		iicoords	array of corresponding "image" coordinates
//		bbtotal		number of BBs in the above two arrays
//	OUTPUTS:
//		ax[3], ay[3]	the two rows of the best fit affine transformation
//		cr[3]			the coefficients of the second order radial fit
//		ct[3]			the coefficients of the second order tangential	fit

// CheckSecondOrderFit(...):

// The function for testing the second order fit is
//	double CheckSecondOrderFit (double **bbcoords, double **iicoords, int bbtotal,
//						double ax[3], double ay[3], double cr[3], double ct[3])  
//	INPUTS:
//		bbcoords	array of "model" coordinates
//		iicoords	array of corresponding "image" coordinates
//		bbtotal		number of BBs in the above two arrays
//		ax[3], ay[3]	the two rows of the best fit affine transformation
//		cr[3]			the coefficients of the second order radial fit
//		ct[3]			the coefficients of the second order tangential	fit
//	OUTPUT:
//		on screen error analysis for:
//		(i) no non-linear correction (affine fit only)
//		(ii) compensation for radial error only
//		(iii) compensation for tangential error only
//		(iv) compensation for radial and tangential error

// CorrectForSecondOrder(...):

//	The function for correcting image measurements for second order distortions is
//	double CorrectForSecondOrder (double **iicoords, int bbtotal,
//					double ax[3], double ay[3], double cr[3], double ct[3],
//					double **xxcoords)  
//	INPUTS:
//		iicoords	array of "image" coordinates
//		bbtotal		number of BBs
//		ax[3], ay[3]	the two rows of the best fit affine transformation
//		cr[3]			the coefficients of the second order radial fit
//		ct[3]			the coefficients of the second order tangential	fit
//	OUTPUT:
//		xxcoords	array of corrected image coordinates


// compensateForTangentialDistortion(
//									double **realBBlocn, 
// the "real" 2d image locations of bbs that were input to Tsai
//									double **tsaiBBlocn, 
// the "estimated" 2d image locns that are projection of 3d bb locations using Tsai 
//									int BBtotal,
// the number of BBs
//									double **correctedBBlocn,
// the corrected 2d locations that are output by the method
//									double cr[3], ct[3]);
// the coefficients of the correction that were used to achieve above result.)


/***********************************************************************************/
