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

// Code for fitting radial and tangential distortion
// Code for locating BB images accurately

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int verboseflag=0;
int traceflag=0;
int showheader=0;
int refineflag=1;				// refine position of BB image centers
int showcolorflag=0;			// show color table
int writesmoothflag=1;			// write smooth image
int colorflag=0;				// write image in RGB
int fuzzyflag=1;				// check more carefully for local min/max
int sparseflag=0;				// set to 1 for image001.bmp

unsigned char *image=NULL;

unsigned char **imagerow=NULL;

double *blurr=NULL;

double **blurrrow=NULL;

int histogram[256];

char *infile1="image001.bmp";

char *infile3="image003.bmp";

#define PI 3.141592653

/************************************************************************/

// code for least squares fitting image intensifier distortion
// one term in radial distortion and one in tangential distortion

// Image Intensifier CCD is 980 x 980

double xo=490.0;	// image center in x
double yo=490.0;	// image center in y

/*****************************************************************************/

// DATA: estimated centers of BB images in 980 x 980 image in image1.bmp
// Along with coordinates in target coordinate system.
// Items starting with {0, 0, ...} are ignored
// This data is used by setupdata and setupdatafine

// PaintShop measures second coordinate from the top row of the image,
// while data starts at the bottom row...

//	image_x, (height-1) - image_y, target_x, target_y

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

int bbmax = sizeof(bidata) / sizeof(bidata[0]);

#define MAXBBS 10000

// int imax=9, jmax=12;		// for image001.bmp

// double iicoords[9][12][2];	// BB image center coordinates

int imax=65, jmax=85;		// for image003.bmp

double iicoords[MAXBBS][2];	// BB image center coordinates

int ioff=33, joff=40;			// to get non-negative subscripts

double bbcoords[MAXBBS][2]	// BB target coordinates

/***************************************************************************************/

/* Stuff for dealing with BMP image files */

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

int width, height, bitsperpixel;

long offset;

BITMAPFILEHEADER bmfh;		// kept global for easy access

BITMAPINFOHEADER bmih;		// kept global for easy access

RGBQUAD colortable[256];	/* set up when reading BMP */

/******************************************************************************/

void showcolortable (RGBQUAD colortable[], int n) {
	int k;
	for (k = 0; k < n; k++) {
		printf("%d\t%d\t%d\t%d\n",
			   k, colortable[k].rgbRed, colortable[k].rgbGreen, colortable[k].rgbBlue);
	}
}

int readBMPheader (FILE *input) {
/*	BITMAPFILEHEADER bmfh; */
/*	BITMAPINFOHEADER bmih; */
	long nlen;

	fseek(input, 0, SEEK_END);
	nlen = ftell(input);
	fseek(input, 0, SEEK_SET);
	if (showheader) printf("File length %ld\n", nlen);
/*	read file header */
	fread(&bmfh, sizeof(bmfh), 1, input);
	if (bmfh.bfType != (77 * 256 + 66)) {			/*  "BM" */
		printf("Not BMP file %X\n", bmfh.bfType);
/*		return -1; */
	}
	else if (showheader) printf("BMP file %X\n", bmfh.bfType);
	offset = bmfh.bfOffBits;
	fread(&bmih, sizeof(bmih), 1, input);
	width = bmih.biWidth;
	height = bmih.biHeight;
	bitsperpixel = bmih.biBitCount;
/*	read color table */
	fread(&colortable, sizeof(RGBQUAD), bmih.biClrUsed, input);
/*	file size in words ? */
	if (showheader) {
		printf("File size %lu\n", bmfh.bfSize);
/*	offset from end of header ? */
		printf("Offset to image %lu\n", bmfh.bfOffBits);
		putc('\n', stdout);
/*	read bitmap info header */	
		printf("Size of header %lu\n", bmih.biSize);	
		printf("Width in pixels %ld\n", bmih.biWidth);	
		printf("Height in pixels %ld\n", bmih.biHeight);
		printf("Number of image planes %u\n", bmih.biPlanes);
		printf("Bits per pixels %u\n", bmih.biBitCount);
		printf("Compression %lu\n", bmih.biCompression);
		printf("Size of compressed image %lu\n", bmih.biSizeImage);
		printf("Horizontal pixel per meter %ld\n", bmih.biXPelsPerMeter);
		printf("Vertical pixel per meter %ld\n", bmih.biYPelsPerMeter);
		printf("Number of colors used %lu\n", bmih.biClrUsed);
		printf("Number of `important' colors %lu\n", bmih.biClrImportant);
	}
	if (showcolorflag) showcolortable(colortable, bmih.biClrUsed);
	if (showheader) putc('\n', stdout);
	fflush(stdout);
	return 0;
}

int setupimagearrays (void) {
	int k, nlen;
	long npixels = width * height;

	if (npixels == 0) return -1;
	if (image != NULL) free(image);
	nlen = sizeof(char) * npixels;
	image = (unsigned char *) malloc(nlen);
	if (image == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d pixel image\n", npixels);
		exit(1);
	}
	if (imagerow != NULL) free(imagerow);
	nlen = sizeof(char *) * height;
	imagerow = (unsigned char **) malloc(nlen);
	if (imagerow == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d margin array\n", height);
		exit(1);
	}
//	set up convenient margin array
	for (k = 0; k < height; k++) imagerow[k] = image + k * width;
	return 0;
}

int readBMPimage(FILE *input) {
	int i, j;
	int c=0;
/*	unsigned char color; */
//	int nbytes=1;
	long current;

	width = (int) bmih.biWidth;
	height = (int) bmih.biHeight;
	if (setupimagearrays()) return -1;	// failure

/*	current = sizeof(BITMAPFILEHEADER) + bmfh.bfOffBits;  */
	current = bmfh.bfOffBits;
	if (showheader) printf("SEEK to %ld\n", current);
	fseek(input, current, SEEK_SET); 
	for (i = 0; i < height; i++) {
//		k = i * width;
		for (j = 0; j < width; j++) {
/*			fread(&color, nbytes, 1, input); */
			c = getc(input);
			if (c < 0) {
				printf("ERROR: EOF i %d j %d\n", i, j);
				break;
			}
/*			image [k] = (double) color; */
//			image [k] = (double) ((unsigned char) c);
//			image [k] = (char) c;
//			k++;
			imagerow[i][j] = (unsigned char) c;	// ignore color table
		}
		if (c < 0) {
			printf("ERROR: EOF i %d j %d\n", i, j);
			break;
		}
	}
	if (c < 0) printf("ERROR: Premature EOF\n");
	current = ftell(input);
	if (traceflag) printf("end at %ld last c %d\n", current, c);
	if (verboseflag) putc('\n', stdout);
	fflush(stdout);
	return 0;
}

/******************************************************************************/

int roundup (int n) {
	return (((n-1) / 4) + 1) * 4;
}

void writeBMPheader (FILE *output) {
	unsigned long current;
	int rem;

	bmfh.bfType = 'B' | ('M' << 8);
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
/*	following doesn't take into account padding */ /*	bmih.biClrUsed = 256; */
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if (colorflag) bmfh.bfSize += 3 * width * height;
	else bmfh.bfSize += width * height + 256 * sizeof(RGBQUAD);
/*	bmfh.bfOffBits = sizeof(BITMAPINFOHEADER); */
/*	bmfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER); */
	bmfh.bfOffBits = roundup(sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER));
	if (colorflag) ;
	else bmfh.bfOffBits += 256 * sizeof(RGBQUAD);
	fwrite(&bmfh, sizeof(bmfh), 1, output);
	bmih.biSize = 40;  /* unchanged */
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biPlanes = 1;
	if (colorflag) bmih.biBitCount = 24;		/* go for 24 bit RGB */
	else bmih.biBitCount = 8;					/* go for 8 bit B/W */
	bmih.biCompression = 0;
	if (colorflag) bmih.biSizeImage = 3 * width * height;
	else bmih.biSizeImage = width * height;
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

void writeBMPimage (FILE *output) {
	int i, j;
//	int k;
	int ir, ig, ib;
	int rem;

	for (i = 0; i < height; i++) {
//		k = i * width;
		for (j = 0; j < width; j++) {
//			ir = ig = ir = image[k];
			ib = ig = ir = imagerow[i][j];
			if (colorflag) {
				putc(ib, output); 
				putc(ig, output); 
				putc(ir, output);
			}
			else putc(ig, output);
//			k++;
		}
		if (colorflag) rem = (width * 3) % 4;
		else rem = width % 4;
		while (rem != 0) {
			putc(0, output);
			rem = (rem + 1) % 4;
		}			
	}
}

void makehistogram (void) {
	int i, j, k;
	for (k = 0; k < 256; k++) histogram[k] = 0;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			k = imagerow[i][j];
			histogram[k]++;
		}
	}
}

void showhistogram (void) {
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

// Just to test linear equation solver above

void testsolve33 (void) {
	double m[3][3];
	double a[3], b[3], c[3];
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			m[i][j] = gaussian(1.0);
		}
		b[i] = gaussian(1.0);
	}
	printf("MATRIX:\n");
	show_matrix33(m);
	printf("VECTOR B: ");
	show_vector3(b);
	matrix_times_vector33(m, b, a);
	printf("VECTOR A: ");
	show_vector3(a);
	solve33(m, a, c);
	printf("VECTOR C: ");
	show_vector3(c);
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
	int i, icol, irow, j, k, l, ll;
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

// create matrix of image and target coordinates from list of data --
// for image001.bmp

int setupdata (int bbmax) {
	int k, count=0;
//	int bbmax = 9 * 12;

	height = width = 980;
	for (k = 0; k < bbmax; k++) {
		if (bidata[k][0] == 0 && bidata[k][1] == 0) continue;
//		coordinates in image
		iicoords[k][0] = bidata[k][0];				// image_x
		iicoords[k][1] = (height-1) - bidata[k][1];	// image_y
//		coordinates in target
		bbcoords[k][0] = bidata[k][2];		// target_x
		bbcoords[k][1] = bidata[k][3];		// target_y
		count++;
	}
	return 0;
}

/*****************************************************************************/

// set up matrix for least squares solution of affine transformation
// gets list of  target coordinates in first argument
// writes result back into last argument

int make_affine_matrix (double bbcoords[][2], double m[3][3]) {
	int i, j, k, count = 0;
	double xb, yb;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) m[i][j] = 0.0;
	}
	for (k = 0; k < bbmax; k++) {
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
		count++;
	}
	if (count < 6) {
		printf("TOO LITTLE DATA %d x %d\n", count);
		return -1;
	}
	return 0;
}

// solve for first row of affine transformation (x part)
// gets list of target coordinates in first argument
// gets list of image coordinates in second argument
// writes result back into last argument

int affinefitx (double bbcoords[][2], double iicoords[][2], double c[3]) {
	double m[3][3];
	double b[3];
	double xb, yb, xi, yi;
	int i, k, count = 0;

	if (make_affine_matrix(bbcoords, m)) return -1;
	for (i = 0; i < 3; i++) b[i] = 0.0;
	for (k = 0; k < bbmax; k++) {
		xi = iicoords[k][0];		// image_x
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		b[0] += xi * xb;
		b[1] += xi * yb;
		b[2] += xi;
		count++;
	}

	solve33(m, b, c);

	printf("MATRIX:\n");
	show_matrix33(m);
	printf("VECTOR B: ");
	show_vector3(b);
	printf("VECTOR C: ");
	show_vector3(c);
	return 0;
}

// solve for second row of affine transformation (y part)
// gets list of target coordinates in first argument
// gets list of image coordinates in second argument
// writes result back into last argument

int affinefity (double bbcoords[][2], double iicoords[][2], double c[3]) {
	double m[3][3];
	double b[3];
	double xb, yb, xi, yi;
	int i, k, count = 0;

	if (make_affine_matrix(bbcoords, m)) return -1;
	for (i = 0; i < 3; i++) b[i] = 0.0;
	for (k = 0; k < bbmax; k++) {
		xi = iicoords[k][0];		// image_x
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		b[0] += yi * xb;
		b[1] += yi * yb;
		b[2] += yi;
		count++;
	}

	solve33(m, b, c);

	printf("MATRIX:\n");
	show_matrix33(m);
	printf("VECTOR B: ");
	show_vector3(b);
	printf("VECTOR C: ");
	show_vector3(c);
	return 0;
}

// Compute error in affine fit - decompose into radial and tangential

void checkaffinefit (double cx[3], double cy[3]) {
	int i, j, k;
	double x, y, xd, yd, dx, dy, dv, r, dr, dt;

	for (k = 0; k < bbmax, k++) {
		xi = iicoords[k][0];		// image_x
		yi = iicoords[k][1];		// image_y
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
//		measured image coordinates (relative to center)
		xi = xi - xo;
		yi = yi - yo
		r = sqrt(xi * xi + yi * yi);
//		compute coordinates expected from affine transformation
		xd = cx[0] * xb + cx[1] * yb + cx[2];
		yd = cy[0] * xb + cy[1] * yb + cy[2];
//		computed image coordinates (relative to center)
		xd = xd - xo;
		yd = yd - yo;
//		error
		dx = xi - xd;
		dy = yi - yd;
		dv = sqrt(dx * dx + dy * dy);
//		radial and tangential components of error
		dr = (dx * xi + dy * yi) / r;	
		dt = (dy * xi - dx * yi) / r;
//		printf("%2d %2d\t x %4lg y %4lg\t xd %7lg yd %7lg\t dx %8lg dy %8lg\t dr %8lg\n",
//			   i, j, x, y, xd, yd, dx, dy, dr);
//		printf("%2d %2d x %4lg y %4lg r %8lg xd %8lg yd %8lg dx %9lg dy %9lg dv %8lg\n",
//		i, j, x, y, r, xd, yd, dx, dy, dv);
		printf("%2d %2d x %4lg y %4lg r %8lg xd %8lg yd %8lg dr %9lg dt %9lg dv %8lg\n",
			   i, j, x, y, r, xd, yd, dr, dt, dv);
	}
}

// set up matrix for least squares solution of radial and tangential distortion

int make_distort_matrix (double m[3][3]) {
	int i, k, count = 0;
	double xi, y, r;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) m[i][j] = 0.0;
	}
	for (k = 0; k < bbmax; k++) {
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
	if (count < 3) {
		printf("TOO LITTLE DATA %d x %d\n", count);
		return -1;
	}
	return 0;
}

// Get best second order fit in radial direction

int radialfit (double cx[3], double cy[3], double c[3]) {
	int i, j, k;
	double xi, yi, xb, yb;
	double xd, yd, dx, dy, dv, r, dr, dt;
	double m[3][3], b[3];

	if (make_distort_matrix(m) < 0) return -1;
	for (k = 0; k < 3; k++) b[k] = 0.0;
	for (k = 0; k < bbmax; k++) {
		xi = iicoords[k][0] - xo;
		yi = iicoords[k][1] - yo;
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		r = sqrt(xi * xi + yi * yi);
		xd = cx[0] * xb + cx[1] * yb + cx[2] - xo;
		yd = cy[0] * xb + cy[1] * yb + cy[2] - yo;
		dx = xi - xd;
		dy = yi - yd;
		dv = sqrt(dx * dx + dy * dy);
		dr = (dx * xi + dy * yi) / r;
		dt = (dy * xi - dx * yi) / r;
		b[0] += dr * r * r;
		b[1] += dr * r;
		b[2] += dr;
	}
	solve33(m, b, c);
	printf("%lg * r * r + %lg * r + %lg\n", c[0], c[1], c[2]);
	{
		double rcrs, rzer1, rzer2, rmax, det, maxerr1, maxerr2, maxerr3, maxerr4, maxerr5, temp;
		rcrs = -c[1] / c[0];
		rmax = width/2;
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
}

// Get best second order fit in tangential direction

void tangentialfit (double cx[3], double cy[3], double c[3]) {
	int i, j, k;
	double xi, yi, xb, yb;
	double xd, yd, dx, dy, dv, r, dr, dt;
	double m[3][3], b[3];

	if (make_distort_matrix(m) < 0) return -1;
	for (k = 0; k < 3; k++) b[k] = 0.0;
	for (k = 0; k < bbmax; k++) {
		xi = iicoords[k][0] - xo;
		yi = iicoords[k][1] - yo;
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		r = sqrt(xi * xi + yi * yi);
		xd = cx[0] * xb + cx[1] * yb + cx[2] - xo;
		yd = cy[0] * xb + cy[1] * yb + cy[2] - yo;
		dx = xi - xd;
		dy = yi - yd;
		dv = sqrt(dx * dx + dy * dy);
		dr = (dx * xi + dy * yi) / r;
		dt = (dy * xi - dx * yi) / r;
		b[0] += dt * r * r;
		b[1] += dt * r;
		b[2] += dt;
	}
	solve33(m, b, c);
	printf("%lg * r * r + %lg * r + %lg\n", c[0], c[1], c[2]);
	{
		double rcrs, rzer1, rzer2, rmax, det, maxerr1, maxerr2, maxerr3, maxerr4, maxerr5, temp;
		rcrs = -c[1] / c[0];
		rmax = width/2;
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
}

// check error remaining after radial and tangential fit

double checkerror (double cx[3], double cy[3], double cr[3], double ct[3]) {
	int i,j, count=0;
	double x, y, xd, yd, dx, dy, dv, r, dr, dt;
	double drd, dtd;
	double sum=0.0;
	for (k = 0; k < bbmax; k++) {
		xi = iicoords[k][0] - xo;
		yi = iicoords[k][1] - yo;
		xb = bbcoords[k][0];		// target_x
		yb = bbcoords[k][1];		// target_y
		r = sqrt(xi * xi + yi * yi);
		xd = cx[0] * xb + cx[1] * yb + cx[2] - xo;
		yd = cy[0] * xb + cy[1] * yb + cy[2] - yo;
		dx = xi - xd;
		dy = yi - yd;
		dv = sqrt(dx * dx + dy * dy);
		dr = (dx * x + dy * y) / r;
		dt = (dy * x - dx * y) / r;
		drd = cr[0] * r * r + cr[1] * r + cr[2];
		dtd = ct[0] * r * r + ct[1] * r + ct[2];
		sum += (drd - dr) * (drd -dr) + (dtd - dt) * (dtd - dt);
		count++;
	}
	printf("st. dev. %lg for %d BBs\n", sqrt(sum/count), count);
	return sqrt(sum/count);
}

/***********************************************************************************/

int testread (char *infile) {
	FILE *input;
	input = fopen(infile, "rb");
	if (input == NULL) {
		perror(infile);
		return -1;
	}
	readBMPheader(input);
	readBMPimage(input);
	fclose(input);
	return 0;
}

int testwrite (char *outfile) {
	FILE *output;
	output = fopen(outfile, "wb");
	if (output == NULL) {
		perror(outfile);
		return -1;
	}
	writeBMPheader(output);
	writeBMPimage(output);
	fclose(output);
	return 0;
}

int setupblurrarray (void) {
	int npixels = width * height;
	int k,nlen;
	
	if (npixels == 0) return -1;
	if (blurr != NULL) free(blurr);
//	nlen = sizeof(int) * npixels
	nlen = sizeof(double) * npixels;
//	printf("pixels: %d array: %d\n", npixels, nlen);
//	blurr = (int *) malloc(nlen);
	blurr = (double *) malloc(nlen);
	if (blurr == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d pixel blurr\n", npixels);
		exit(1);
	}
//	nlen = sizeof(int *) * height;
	nlen = sizeof(double *) * height;
//	printf("pixels: %d array: %d\n", npixels, nlen);
	if (blurrrow != NULL) free(blurrrow);
//	blurrrow = (int **) malloc(nlen);
	blurrrow = (double **) malloc(nlen);
	if (blurrrow == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d margin array\n", height);
		exit(1);
	}
//	set up convenient margin array
	for (k = 0; k < height; k++) blurrrow[k] = blurr + k * width;
	return 0;
}

void copyimagein (void) {
	int i, j;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++)	{
			blurrrow[i][j] = imagerow[i][j];
		}
	}
}

void copyimageout (void) {
	int i, j;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++)	{
			imagerow[i][j] = (unsigned char) (blurrrow[i][j] + 0.5);
		}
	}
}

// Convolve image n times with second order binomial smoothing filter
//
// 1 2 1
// 2 4 2
// 1 2 1
//
// Does not rescale, so each time average level grows by factor of 16.
// Hence need more bits to represent result, but no loss by rounding.
// So for 16-bit (SHORT) result can do this twice only before overflow.
// And for 32-bit (INT) result can do this up to six times.
// Floating point version does rescale...
//
// At edges need to pretend that pixels one row/column outside the
// "frame" have the same grey values as those on the frame.
// (Otherwise the frame becomes darker relative to the rest).

int blurrimage (int n) {
	int i, j, iter;

//	if (n > 6) printf("Danger of overflow in result (n > 6)\n");

	for (iter = 0; iter < n; iter++) {
//		Apply horizontal filter (1 2 1)
		for (i = 0; i < height; i++) {
			for (j = 0; j < width-1; j++) 
				blurrrow[i][j] = (blurrrow[i][j] + blurrrow[i][j+1]) * 0.5;
//			blurrrow[i][width-1] = blurrrow[i][width-1] + blurrrow[i][width-1];
			for (j = width-1; j > 0; j--) 
				blurrrow[i][j] = (blurrrow[i][j] + blurrrow[i][j-1]) * 0.5;
//			blurrrow[i][0] = blurrrow[i][0] + blurrrow[i][0];
		}
//		Apply vertical filter (1 2 1)
		for (j = 0; j < width; j++) {
			for (i = 0; i < height-1; i++) 
				blurrrow[i][j] = (blurrrow[i][j] + blurrrow[i+1][j]) * 0.5;
//			blurrrow[height-1][j] = blurrrow[height-1][j] + blurrrow[height-1][j];			
			for (i = height-1; i > 0; i--) 
				blurrrow[i][j] = (blurrrow[i][j] + blurrrow[i-1][j]) * 0.5;
//			blurrrow[0][j] = blurrrow[0][j] + blurrrow[0][j];
		}
	}
	return 0;
}

/******************************************************************************/

// Is (i, j) a point of local minimum ?

int checklocalmin (int i, int j) {
//	int center = blurrrow[i][j];
	double center; 
	if (i <= 0 || i >= height-1 ||
		  j <= 0 || j >= width-1) {
		printf("checklocalmin error: %d %d\n", i, j);
		return 0;
	}
	center = blurrrow[i][j];
	if (blurrrow[i][j+1] < center ||
		  blurrrow[i][j-1] < center ||
		  blurrrow[i+1][j] < center ||
		  blurrrow[i-1][j] < center) return 0;
	if (fuzzyflag) {
		if (blurrrow[i+1][j+1] < center ||
			  blurrrow[i+1][j-1] < center ||
			  blurrrow[i-1][j+1] < center ||
			  blurrrow[i-1][j-1] < center) return 0;
	}
	return -1;			// yes, it is a local minimum
}

// Is (i, j) a point of local maximum ?

int checklocalmax (int i, int j) {
//	int center = blurrrow[i][j];
	double center; 
	if (i <= 0 || i >= height-1 ||
		  j <= 0 || j >= width-1) {
		printf("checklocalmax error: %d %d\n", i, j);
		return 0;
	}
	center = blurrrow[i][j];
	if (blurrrow[i][j+1] > center ||
		  blurrrow[i][j-1] > center ||
		  blurrrow[i+1][j] > center ||
		  blurrrow[i-1][j] > center) return 0;
	if (fuzzyflag) {
		if (blurrrow[i+1][j+1] > center ||
			  blurrrow[i+1][j-1] > center ||
			  blurrrow[i-1][j+1] > center ||
			  blurrrow[i-1][j-1] > center) return 0;
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
		printf("Zero determinant\n");
		return -1;
	}
	if (traceflag)
		printf("detx %lg dety %lg det %lg\n", detx, dety, det);
	*x = detx / det;
	*y = dety / det;
	return 0;						// success
}

// Fit second order surface
// z = a + b * x + c * y + d * x * x + 2 * e * x * y + f * y * y
// to 3 x 3 neighborhood (es=1)
// or 5 x 5 neighborhood (es=2)

int fitsecondorder (int i, int j, double *a, double *b, double *c, double *d, double *e, double *f) {
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
	if (i <= 0 || i >= height-1 ||
		  j <= 0 || j >= width-1) return -1;	// failure
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
			z = blurrrow[ik][jk];
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
//	ret = solve66(m, r, s);
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

void showneighbourhood (int i, int j) {
	int ik, jk, es=3;

	printf("  %4d\t", 0);
	for (jk = j-es; jk <= j+es; jk++) {
		printf("%9d ", jk);
	}
	printf("\n");
	for (ik = i-es; ik <= i+es; ik++) {
		printf("i %4d\t", ik);
		for (jk = j-es; jk <= j+es; jk++) {
			printf("%9lg ", blurrrow[ik][jk]);
		}
		printf("\n");
	}
}

int checkbackground (int i, int j) {
	int ik, jk, es=1;
	for (ik = i-es; ik <= i+es; ik++) {
		for (jk = j-es; jk <= j+es; jk++) {
			if (blurrrow[ik][jk] > 0) return 0;
		}
	}
	printf("BACKGROUND\n");
	return -1;
}

// Find local minimum in blurred array near (i, j)
// returns result in last two arguments

int findlocalmin (int i, int j, double *x, double *y) {
	int ik, jk, ret, found=0;
	double a, b, c, d, e, f;
	double xd, yd;
	double es=1.0;
	if (i <= 0 || i >= height-1 ||
		  j <= 0 || j >= width-1) {
		printf("findlocalmin error: %d %d\n", i, j);
		return -1;
	}
	ik = i;
	jk= j;
	if (checkbackground(i, j)) return -1;
	if (checklocalmin(ik, jk)) found = 1;
//	showneighbourhood(i, j);	// debugging
	if (found == 0) {
		for (ik = i-1; ik <= i+1; ik++) {
			for (jk = j-1; jk <= j+1; jk++) {
				if (ik == i && jk == j) continue;
				if (checklocalmin(ik, jk)) {
					found = 1;
					break;
				}
			}
			if (found) break;
		}
	}
//	showneighbourhood(i, j);	// debugging
	if (found == 0) {
		for (ik = i-2; ik <= i+2; ik++) {
			for (jk = j-2; jk <= j+2; jk++) {
				if ((i-1 <= ik) && (ik <= i+1) &&
					  (j-1 <= jk) && (jk <= j+1)) continue;
				if (checklocalmin(ik, jk)) {
					found = 1;
					break;
				}
			}
			if (found) break;
		}
	}
	if (found == 0) {
		printf("No local min at or near x %d y %d (PSP %d %d)\n", j, i, j, (height-1)-i);
		if (verboseflag) showneighbourhood(i, j);
		*x = j;
		*y = i;
		return -1;
	}
	ret = fitsecondorder(ik, jk, &a, &b, &c, &d, &e, &f);
	if (ret < 0) return ret;	// failure
	ret = secondorderextremum(b, c, d, e, f, &xd, &yd);
	if (ret < 0) return ret;	// failure
	if (xd < -es || xd > es || yd < -es || yd > es) {
		printf("Solution too far away dx %lg dy %lg\n", xd, yd);
		if (verboseflag) showneighbourhood(ik, jk);
		return -1;
	}
	*x = jk + xd;
	*y = ik + yd;
	return 0;					// success
}

// Find local maximum in blurred array near (i, j)
// returns result in last two arguments

int findlocalmax (int i, int j, double *x, double *y) {
	int ik, jk, ret, found=0;
	double a, b, c, d, e, f;
	double xd, yd;
	double es=1.0;
	if (i <= 0 || i >= height-1 ||
		  j <= 0 || j >= width-1) {
		printf("findlocalmax error: %d %d\n", i, j);
		return -1;
	}
	ik = i;
	jk= j;
	if (checkbackground(i, j)) return -1;
	if (checklocalmax(ik, jk)) {
//		printf("FOUND %d %d\n", ik, jk);
		found = 1;
	}
	if (found == 0) {
//		printf("Trying in 3 x 3 %d %d\n", i, j);
//		showneighbourhood(i, j);	// debugging
		for (ik = i-1; ik <= i+1; ik++) {
			for (jk = j-1; jk <= j+1; jk++) {
				if (ik == i && jk == j) continue;
				if (checklocalmax(ik, jk)) {
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
//		showneighbourhood(i, j);	// debugging
		for (ik = i-2; ik <= i+2; ik++) {
			for (jk = j-2; jk <= j+2; jk++) {
				if ((i-1 <= ik) && (ik <= i+1) &&
					  (j-1 <= jk) && (jk <= j+1)) continue;
				if (checklocalmax(ik, jk)) {
//					printf("FOUND %d %d\n", ik, jk);
					found = 1;
					break;
				}
			}
			if (found) break;
		}
	}
	if (found == 0) {
		printf("No local max at or near x %d y %d (PSP %d %d)\n", j, i, j, (height-1)-i);
		if (verboseflag) showneighbourhood(i, j);
		*x = j;
		*y = i;
		return -1;
	}
//	printf("LOCALMAX ANSWER %d %d\n", ik, jk);
//	showneighbourhood(ik, jk);
	ret = fitsecondorder(ik, jk, &a, &b, &c, &d, &e, &f);
	if (ret < 0) return ret;	// failure
	ret = secondorderextremum(b, c, d, e, f, &xd, &yd);
	if (ret < 0) return ret;	// failure
	if (xd < -es || xd > es || yd < -es || yd > es) {
		if (verboseflag)
			printf("Solution too far away dx %lg dy %lg\n", xd, yd);
		if (verboseflag) showneighbourhood(ik, jk);
		return -1;
	}
	*x = jk + xd;
	*y = ik + yd;
//	printf("FINAL ANSWER %d %d\n", (int) (*y+0.5), (int) (*x+0.5));
//	showneighbourhood((int) (*y+0.5), (int) (*x+0.5));
	return 0;					// success
}

int testminimumfind (double xo, double yo) {
	int i, j;
	int ik, jk;
	double alpha=2, beta=1, gamma=4;
	double a, b, c, d, e, f;
	double x, y;

	height = width = 980;
	if (setupblurrarray ()) return -1;
	ik = (int) (yo + 0.5);
	jk = (int) (xo + 0.5);
	a = alpha * xo * xo + beta * xo * yo + gamma * yo * yo;
	b = - 2 * alpha * xo - beta * yo;
	c = - 2 * gamma * yo - beta * xo;
	d = alpha;
	e = beta/2;
	f = gamma;
//	a = 1000.0;
	
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			x = j;
			y = i;
			blurrrow[i][j] =
					a + b * x + c * y + d * x * x + 2 * e * x * y + f * y * y;
		}
	}
	if (findlocalmin(ik, jk, &x, &y)) return -1;	// failure
	printf("xo %lg yo %lg x %lg y %lg\n", xo, yo, x, y);
	return 0;
}

// m[i][j] => j corresponds to x, positive to the right
// m[i][j] => i corresponds to y, positive down

// create matrix of coordinates from list of measured data 
// for image001.bmp

int setupdatafine (int bbmax) {
	int ik, jk, k, count=0, bad=0;
	double x, y;
//	int bbmax = 9 * 12;

	if (blurrrow == NULL) {
		printf("Need to set up blurred data array first\n");
		return -1;
	}
	height = width = 980;
	for (k = 0; k < bbmax; k++) {
		if (bidata[k][0] == 0 && bidata[k][1] == 0) continue;
		jk = bidata[k][0];				// image_x
		ik = (height-1) - bidata[k][1];	// image_y
		if (findlocalmin (ik, jk, &x, &y)) {
			x = jk; y = ik;
			bad++;
		}
		else {
			if (verboseflag)
				printf("%4d %4d\t%lg %lg\t%lg %lg\n",
					   ik, jk, y, x, y-ik, x-jk);
		}
		count++;
//		coordinates in image
		iicoords[k][0] = x;		// refined image_x
		iicoords[k][1] = y;		// refined image_y
//		coordinates in target
		bbcoords[k][0] = bidata[k][2];	// target_x
		bbcoords[k][1] = bidata[k][3];	// target_y
		count++;
	}
	if (bad > 0) printf("%d out of %d BBs not located\n", bad, count);
	return 0;
}

/***********************************************************************************/

// Computer power at given spatial frequency (in cycles per pixel)

// for image003.bmp get max at (.01319 .08842)
// this means spots are spaced (1.650 11.063) pixels in one direction
// for image003.bmp get max at (.08805 .01315)
// this means spots are spaced (11.109 -1.659) pixels in other direction

double powerat (double u, double v) {
	int i, j;
	double x, y, w, sumc=0.0, sums= 0.0;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			x = j - xo;
			y = i - yo;
			w = 2.0 * PI * (u * x + v * y);
			if (imagerow[i][j] > 0) {
				sumc += imagerow[i][j] * cos(w);
				sums += imagerow[i][j] * sin(w);
			}
		}
	}
	return (sqrt(sumc * sumc + sums * sums)) / (width * height);
}


/***********************************************************************************/

// Attempt at fitting radial and tangential distortion

void testfit (void) {
	double dxj, dyj, dxi, dyi, dot, dj, di;
	double cx[3], cy[3];
	double cr[3], ct[3];
	double zero[3] = {0.0, 0.0, 0.0};

//	dxj = averagexfromj();
//	dyj = averageyfromj();
//	dxi = averagexfromi();
//	dyi = averageyfromi();
	printf("dx per dj %lg\n", dxj);
	printf("dy per dj %lg\n", dyj);
	printf("dx per di %lg\n", dxi);
	printf("dy per di %lg\n", dyi);
	dot = dxj * dxi + dyj * dyi;
	dj = sqrt(dxj * dxj + dyj * dyj);
	di = sqrt(dxi * dxi + dyi * dyi);
	printf("dot-product %lg\n", dot);
	printf("dj %lg di %lg\n", dj, di);
	printf("dot / (di * dj) %lg\n", dot / (di * dj));
	printf("ratio of lengths %lg\n", di / dj);
	testsolve33();
	printf("Fit for x:\n");
	affinefitx(cx);
	printf("Fit for y:\n");
	affinefity(cy);
	checkaffinefit(cx, cy);
	checkerror(cx, cy, zero, zero);
	printf("\n");
	printf("Radial     Fit: ");
	radialfit(cx, cy, cr);
	printf("\n");
	printf("Tangential Fit: ");
	tangentialfit(cx, cy, ct);
	printf("\n");
	printf("radial and tangential correction:\n");
	checkerror(cx, cy, cr, ct);
	printf("radial correction only:\n");
	checkerror(cx, cy, cr, zero);
	printf("tangential correction only:\n");
	checkerror(cx, cy, zero, ct);
	printf("no correction:\n");
	checkerror(cx, cy, zero, zero);
}

void showBBs (void) {
	int i, j, l, k, flag;
	for (l = 0; l < imax; l++) {
		flag = 0;
		for (k = 0; k < jmax; k++) {
			if (iicoords[l][k][0] != 0.0 || iicoords[l][k][1] != 0.0) {
				j = (int) (iicoords[l][k][0]+0.5);
				i = (height-1) - (int) (iicoords[l][k][1] + 0.5);
				printf("%3d %3d\t%lg\t%lg (PSP %d %d)\n",
					   l-ioff, k-joff,
					   iicoords[l][k][0], iicoords[l][k][1],
					   j, i);
				flag++;
			}
		}
		if (flag) printf("\n");
	}
}

int scanformax (int io, int jo, int *in, int *jn) {
	int i, j, es;
	if (checklocalmax(io, jo)) {
		*in = io;
		*jn = jo;
		return 0;
	}
	for (es = 1; es < 16; es++) {
		for (i = io - es; i <= io + es; i++) {
			for (j = jo - es; j <= jo + es; j++) {
				if (checklocalmax(i, j)) {
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

// grid spacing in two orthogonal directions in image003.bmp

double dx1=1.650, dy1=11.063;
double dx2=11.109, dy2=-1.659;

// this means spots are spaced (1.650 11.063) pixels in one direction
// this means spots are spaced (11.109 -1.659) pixels in other direction

int makegrid (char *filename) {
	int i, j, k, l, count=0;
	double xc, yc, x,  y, xo, yo, xn, yn;

	imax=65;
	jmax=85;			// limits for image003.bmp
	ioff=33;
	joff=40;			// to get non-negative subscripts
	for (l = 0; l < imax; l++) {
		for (k = 0; k < jmax; k++) {
			iicoords[l][k][0] = 0;
			iicoords[l][k][1] = 0;
		}
	}
//	485, 489 in PSP is x, (height-y) -- arbitrary "image center" BB
	if (strstr(filename, "003") != NULL) {
		yc = (height-1)-489;
		xc =  485;
	}
	else if (strstr(filename, "004") != NULL) {
		yc = (height-1)-491;
		xc =  483;
	}
	else if (strstr(filename, "005") != NULL) {
		yc = (height-1)-484;
		xc =  486;
	}
	else if (strstr(filename, "006") != NULL) {
		yc = (height-1)-484;
		xc =  481;
	}
	else {
		printf("Unknown input file\n");
		exit(1);
	}
	
//	yo = (height-1)-489;
//	xo =  485;
	xo = xc;
	yo = yc;
	for (l = 0; l < 256; l++) {
		x = xo;
		y = yo;
		if (findlocalmax((int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
			break;
		}
		x = xo = xn;
		y = yo = yn;
		for (k = 0; k < 256; k++) {
			if (findlocalmax((int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
				printf("FOUND %d spots starting from %d (+)\n", k, l);
				break;
			}
			if (verboseflag)
				printf("Local max at x %lg y %lg (PSP %lg %lg)\n",
				   xn, yn, xn, (height-1) - yn);
			i = ioff+l;
			j = joff+k;
			if (i < 0 || i >= imax || j < 0 || j >= jmax) 
				printf("ERROR: %d %d\n", i, j);
			else {
				iicoords[i][j][0] = xn;
				iicoords[i][j][1] = yn;
				count++;
			}
			x = xn + dx1;
			y = yn + dy1;
			if (verboseflag)
				printf("NEW PLACE at x %lg y %lg (PSP %lg %lg)\n", x, y, x, (height-1) - y);
		}
		x = xo;
		y = yo;
		for (k = 0; k < 256; k++) {
			if (findlocalmax((int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
				printf("FOUND %d spots starting from %d (-)\n", k, l);
				break;
			}
			if (verboseflag)
				printf("Local max at x %lg y %lg (PSP %lg %lg)\n",
				   xn, yn, xn, (height-1) - yn);
			i = ioff+l;
			j = joff-k;
			if (i < 0 || i >= imax || j < 0 || j >= jmax) 
				printf("ERROR: %d %d\n", i, j);
			else {
				iicoords[i][j][0] = xn;
				iicoords[i][j][1] = yn;
				count++;
			}
			x = xn - dx1;
			y = yn - dy1;
			if (verboseflag)
				printf("NEW PLACE at x %lg y %lg (PSP %lg %lg)\n", x, y, x, (height-1) - y);
		}
		xo = xo + dx2;
		yo = yo + dy2;
	}
	printf("DONE %d columns (+)\n", l);
//	yo = (height-1)-489;
//	xo =  485;
	xo = xc;
	yo = yc;
	for (l = 0; l < 256; l++) {
		x = xo;
		y = yo;
		if (findlocalmax((int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
			break;
		}
		x = xo = xn;
		y = yo = yn;
		for (k = 0; k < 256; k++) {
			if (findlocalmax((int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
				printf("FOUND %d spots starting from %d (+)\n", k, l);
				break;
			}
			if (verboseflag)
				printf("Local max at x %lg y %lg (PSP %lg %lg)\n",
					   xn, yn, xn, (height-1) - yn);
			i = ioff-l;
			j = joff+k;
			if (i < 0 || i >= imax || j < 0 || j >= jmax) 
				printf("ERROR: %d %d\n", i, j);
			else {
				iicoords[i][j][0] = xn;
				iicoords[i][j][1] = yn;
				count++;
			}
			x = xn + dx1;
			y = yn + dy1;
			if (verboseflag)
				printf("NEW PLACE at x %lg y %lg (PSP %lg %lg)\n", x, y, x, (height-1) - y);
		}
		x = xo;
		y = yo;
		for (k = 0; k < 256; k++) {
			if (findlocalmax((int) (y+0.5), (int) (x+0.5), &xn, &yn)) {
				printf("FOUND %d spots starting from %d (-)\n", k, l);
				break;
			}
			if (verboseflag)
				printf("Local max at x %lg y %lg (PSP %lg %lg)\n",
					   xn, yn, xn, (height-1) - yn);
			i = ioff-l;
			j = joff-k;
			if (i < 0 || i >= imax || j < 0 || j >= jmax) 
				printf("ERROR: %d %d\n", i, j);
			else {
				iicoords[i][j][0] = xn;
				iicoords[i][j][1] = yn;
				count++;
			}
			x = xn - dx1;
			y = yn - dy1;
			if (verboseflag)
				printf("NEW PLACE at x %lg y %lg (PSP %lg %lg)\n", x, y, x, (height-1) - y);
		}
		xo = xo - dx2;
		yo = yo - dy2;
	}
	printf("DONE %d columns (-)\n", l);
	printf("\n");
	showBBs();
	if (count < 6) {
		printf("TOO FEW BBS %d\n", count);
		return -1;
	}
	return 0;
}

int extendwiths (char *filename) {
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

//	double u, v, p;
//	double x, y;
//	if (argc > 2) {
//	sscanf(argv[1], "%lg", &u);
//	sscanf(argv[2], "%lg", &v);
//	}
//	for (u = 0.0131; u < 0.0133; u += 0.00001) {
//		for (v = 0.0884; v < 0.0885; v += 0.00001) {
//			p = powerat (u, v);
//			printf("Power at %lg %lg\t%lg\n", u, v, p);
//			fflush(stdout);
//		}
//		printf("\n");
//	}
//	for (u = 0.0878; u < 0.0885; u += 0.00005) {
//		for (v = -0.0134; v < -0.0130; v += 0.00005) {
//			p = powerat (u, v);
//			printf("Power at %lg %lg\t%lg\n", u, v, p);
//			fflush(stdout);
//		}
//		printf("\n");
//	}

int main(int argc, char *argv[]) {
	char infile[FILENAME_MAX];
	char outfile[FILENAME_MAX];

	if (argc > 1) strcpy(infile, argv[1]);
	else {
		strcpy(infile, "d:\\bmp\\");
		if (sparseflag) strcat(infile, infile1);
		else strcat(infile, infile3);
	}
	extension(infile, "bmp");
	if (testread(infile)) exit(1);
 //	makehistogram();
 //	showhistogram();
 //	height = width = 980;
	if (setupblurrarray()) exit(1);
	copyimagein();
	if (sparseflag) blurrimage(20);
	else blurrimage(4);
	if (writesmoothflag) {
		copyimageout();
		strcpy(outfile, infile);
		if (extendwiths(outfile)) exit(1);
		testwrite(outfile);
	}
	if (sparseflag) {
 //		testminimumfind (485, 489);
		bbmax = sizeof(bidata) / sizeof(bidata[0]);
		if (refineflag) setupdatafine(bbmax);
		else setupdata(bbmax);
	}
	else {
 //		testminimumfind (320.12, 230.45);
		if (makegrid(infile)) exit(1);
	}
	testfit();
}

/***********************************************************************************/
