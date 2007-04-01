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
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <time.h>

#define PI    3.141592653

#define TWOPI 6.283195306

int nsize=32;		// speckle is nsize * nsize

int nsource=359;	// number of sources

double eps=0.1;		// pixel size in microns

double lambda=0.5;	// wavelength / cos(theta) in microns

int usecolorflag=1;

double **speckler=NULL;		// real part
double **specklei=NULL;		// imaginary part

double **image=NULL;

double **red=NULL;
double **green=NULL;
double **blue=NULL;

///////////////////////////////////////////////////////////////////////

//	general utility functions

double deg_from_rad (double theta) {
	return theta * 180.0 / PI;
}

double rad_from_deg (double angle) {
	return angle * PI / 180.0;
}

// Generate uniformly distributed pseudo-random numbers

double random (double rmin, double rmax) {	// uniform between rmin & rmax
	return ((double) rand() / (double) RAND_MAX) * (rmax - rmin) + rmin;
}

#define NGAUSSIAN 27

// Generate pseudo-random numbers with Gaussian dsitribution

double gaussian (double stdev) {	// mean zero Gaussian random var
	double sum=0.0;
	int i;
	if (stdev == 0.0) return 0.0;
	for (i = 0; i < NGAUSSIAN; i++) sum += random(-1.0, 1.0);
	return sum * stdev / sqrt(NGAUSSIAN/3);
}

///////////////////////////////////////////////////////////////////////

//	create vector, quaternion & matrix structures, and free them again
//	NOTE: for vectors can also just use double foo[3]
//	NOTE: for quaternions can also just use double foo[4]
//	NOTE: matrices should be arrays of arrays, hence NOT double foo[3][3]

double *make_vector_n (int n) {			// vector of length n
	return (double *) malloc(n * sizeof(double));
}

double *make_vector (void) {
	return make_vector_n(3);
}

double *make_quaternion (void) {
	return make_vector_n(4);
}

//	make n x m matrix --- i.e. m column vectors of n elements each
//	NOTE: matrix organized as array of row vectors 
//	NOTE: m[i] is the i-th row ... *not* column
//	NOTE: double m[...][...] creates a *different* structure
//	(which is accessed using multiplication rather than indirection)

double **make_matrix_nm (int n, int m) { // n rows m columns
	int i;
	double **mat;
	mat	= (double **) malloc(n * sizeof(double *));
	for (i = 0; i < n; i++) mat[i] = make_vector_n(m);
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
	for (i = 0; i < n; i++) show_vector_n(mat[i], m);
}

void show_matrix (double **mat) {
	show_matrix_nm(mat, 3, 3);
}

void setup_vector(double v0, double v1, double v2, double *vec) {
	vec[0] = v0; vec[1] = v1; vec[2] = v2;
}

////////////////////////////////////////////////////


/* Stuff for dealing with BMP file format */

#define FAR
#define BYTE unsigned char
#define SHORT short
#define WORD unsigned short
#define LONG long
#define DWORD unsigned long

/* _CRTIMP size_t __cdecl fread(void *, size_t, size_t, FILE *); */

#ifdef _WIN32
#include <pshpack2.h>
#endif
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#ifdef _WIN32
#include <poppack.h>
#endif

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

BITMAPFILEHEADER bmfh;	/* for BMP files */

BITMAPINFOHEADER bmih;	/* for BMP files */

RGBQUAD colortable[256];	/* used both in BMP input and BMP output */

void setupcolortable (RGBQUAD colortable[], int n) {
	int k;
	for (k = 0; k < n; k++) 
		colortable[k].rgbRed = colortable[k].rgbGreen = colortable[k].rgbBlue = (unsigned char) k;
}

void showcolortable (RGBQUAD colortable[], int n) {
	int k;
	for (k = 0; k < n; k++) {
		printf("%d\t%d\t%d\t%d\n",
			   k, colortable[k].rgbRed, colortable[k].rgbGreen, colortable[k].rgbBlue);
	}
}


////////////////////////////////////////////////////

int roundup (int n) {	/* nearest multiple of four equal to or larger */
	return (((n-1) / 4) + 1) * 4;
}

void writebmphead (FILE *output, int usecolorflag, int h, int w) {
	unsigned long current;
	int rem;

	if (h == 0 || w == 0) printf("Zero size image %d x %d \n", h, w);
	bmfh.bfType = 'B' | ('M' << 8);
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
/*	following doesn't take into account padding */ /*	bmih.biClrUsed = 256; */
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if (usecolorflag) bmfh.bfSize += 3 * w * h;		/* no color map if 24 bit color */
	else bmfh.bfSize += w * h + 256 * sizeof(RGBQUAD);
/*	bmfh.bfOffBits = sizeof(BITMAPINFOHEADER); */
/*	bmfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER); */
	bmfh.bfOffBits = roundup(sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER));
	if (usecolorflag) ;
	else bmfh.bfOffBits += 256 * sizeof(RGBQUAD);	/* adjust for space taken by color map */
	fwrite(&bmfh, sizeof(bmfh), 1, output);
	bmih.biSize = 40;							/* unchanged */
	bmih.biWidth = w;
	bmih.biHeight = h;
	bmih.biPlanes = 1;
	if (usecolorflag) bmih.biBitCount = 24;		/* go for 24 bit RGB */
	else bmih.biBitCount = 8;					/* go for 8 bit B/W */
	bmih.biCompression = 0;						/* no compression */
	if (usecolorflag) bmih.biSizeImage = 3 * w * h;
	else bmih.biSizeImage = w * h;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;			/* no color mape for 24  bit RGB */
								/* otherwise map is assumed to be of size 2^n ? */
	if (! usecolorflag) bmih.biClrUsed = 256;	
	bmih.biClrImportant = 0;
	fwrite(&bmih, sizeof(bmih), 1, output);
	if (usecolorflag) ;			/*	no color map for 24 bit RGB */
	else fwrite(&colortable, sizeof(RGBQUAD), 256, output);	/* assume set up */
	current = ftell(output);
	rem = (int) (current % 4);
	while (rem != 0) {			/* pad to multiple of four bytes */
		putc(0, output);
		rem = (rem + 1) % 4;
	}			
	current = ftell(output);
	if (current != bmfh.bfOffBits) {
		printf("ERROR: Header not correct size %ld %ld\n",
			   current, bmfh.bfOffBits);
	}
}

void writebmpimage(FILE *output, int usecolorflag, int width, int height) {
	int i, j, c;
	if (usecolorflag == 0) setupcolortable (colortable, 256);
	writebmphead (output, usecolorflag, width,  height);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			if (usecolorflag == 0) {
				c = (int) image[i][j];
				if (c < 0) c = 0;
				else if (c > 255) c = 255;
				putc(c, output);
			}
			else {
//				c = (int) red[i][j];
				c = (int) blue[i][j];
				if (c < 0) c = 0;
				else if (c > 255) c = 255;
				putc(c, output);
				c = (int) green[i][j];
				if (c < 0) c = 0;
				else if (c > 255) c = 255;
				putc(c, output);
//				c = (int) blue[i][j];
				c = (int) red[i][j];
				if (c < 0) c = 0;
				else if (c > 255) c = 255;
				putc(c, output);
			}
		}
	}
}

/******************************************************************************/


char *outfilename="speckle.bmp";

void write_speckle (void) {
	FILE *output;

	output = fopen(outfilename, "wb");
	if (output == NULL) {
		perror(outfilename);
		exit(1);
	}
	printf("Writing %s\n", outfilename);
	writebmpimage(output, usecolorflag, nsize, nsize);
	fclose(output);
}

///////////////////////////////////////////////////


/* mapping from real and imaginary components to color components */
/* attempt to show amplitude and phase graphically */

/* red   along x-axis - 60 degree */
/* green along x-axis + 60 degrees */
/* blue  along x-axis - 180 degrees */
/* Hence 0 degree (real value) is yellow ... */
/* atan2 returns angle in radians between -PI and +PI */

double rcomponent (double dtheta) {
	dtheta += 60.0;
	if (dtheta > 180.0) dtheta -= 360.0;
	if (dtheta >= 0 && dtheta < 120.0) {
		return (120.0 - dtheta) / 120.0;
	}
	else if (dtheta <= 0 && dtheta > -120.0) {
		return (120.0 + dtheta) / 120.0;
	}
	else return 0.0;
}

double gcomponent (double dtheta) {
	dtheta -= 60.0;
	if (dtheta < -180.0) dtheta += 360.0;
	if (dtheta >= 0 && dtheta < 120.0) {
		return (120.0 - dtheta) / 120.0;
	}
	else if (dtheta <= 0 && dtheta > -120.0) {
		return (120.0 + dtheta) / 120.0;
	}
	else return 0.0;
}

double bcomponent (double dtheta) {
	dtheta += 180.0;
	if (dtheta > 180.0) dtheta -= 360.0;
	if (dtheta >= 0 && dtheta < 120.0) {
		return (120.0 - dtheta) / 120.0;
	}
	else if (dtheta <= 0 && dtheta > -120.0) {
		return (120.0 + dtheta) / 120.0;
	}
	else return 0.0;	
}


double logoffset=3.0;
double grayscale=255.0;

void convert_speckle (void) {
	double theta, c, s, rcs, rcl, r, g, b;
	int i, j;
	for (i = 0; i < nsize; i++) {
		for (j = 0; j < nsize; j++) {
			c = speckler[i][j];
			s = specklei[i][j];

			rcs = sqrt(c * c + s * s);

//			rcl = log(rcs) + logoffset; 
			rcl = rcs; 

			if (rcl < 0.0) rcl = 0.0;
/*	since c and s are only used in atan2, this normalization is bogus */
			if (rcs > 0.0) {
				c = (c / rcs) * rcl;
				s = (s / rcs) * rcl;
			}

			theta = atan2(s, c) * 360.0 / TWOPI;

/*	this sqrt business attempts to stop everything from being pure colors */
			r = sqrt(rcomponent (theta)) * rcl;
			g = sqrt(gcomponent (theta)) * rcl;
			b = sqrt(bcomponent (theta)) * rcl;

			red[i][j] = grayscale * r;
			green[i][j] = grayscale * g;
			blue[i][j] = grayscale * b;
//			green[i][j] = 0.0;
//			blue[i][j] = 0.0;

		}
	}
}

////////////////////////////////////////////////////

void make_speckle (void) {
	double theta, cs, sn, rls, igs;
	double x, y, delta, cc, ss, rl, ig;
	double phi, xo=0.5, yo=0.0;
	double scale;
	int i, j, k;
	for (k = 0; k < nsource; k++) {
		theta = k * 2.0 * PI / nsource;
		cs = cos(theta);
		sn = sin(theta);
//		printf("theta %lg cs %lg sn %lg\n", theta, cs, sn);
//		rls = 1.0;			// beam amplitude * cos(phase)
//		igs = 0.0;			// beam amplitude * sin(phase)
//		rls = cs;
//		igs = sn;
//		phi = - (cs * xo + sn * yo) / lambda * TWOPI;
//		rls = cos(phi);
//		igs = sin(phi);
//		rls = cs;
//		igs = 0.0;
		rls = cos(2 * theta);
		igs = 0.0;
		for (i = 0; i < nsize; i++) {
			y = i * eps - nsize * eps / 2;
			for (j = 0; j < nsize; j++) {
				x = j * eps - nsize * eps / 2;
				delta = (cs * x + sn * y) / lambda * TWOPI;
				cc = cos(delta);
				ss = sin(delta);
				rl = rls * cc - igs * ss;
				ig = rls * ss + igs * cc;
				speckler[i][j] += rl;
				specklei[i][j] += ig;
			}
		}
	}
	scale = 1.0 / nsource;
	for (i = 0; i < nsize; i++) {
		for (j = 0; j < nsize; j++) {
			speckler[i][j] *= scale;
			specklei[i][j] *= scale;
		}
	}
}

////////////////////////////////////////////////////

void show_speckle (void) {
	double rl, ig, mag;
	int i, j;
	printf("MAGNITUDE:\n");
	for (i = 0; i < nsize; i++) {
		printf("%3d ", i);
		for (j = 0; j < nsize; j++) {
			rl = speckler[i][j];
			ig = specklei[i][j];
			mag = sqrt(rl * rl + ig * ig);
			printf("%6.3lf ", mag);
		}
		printf("\n");
	}
	printf("\n");
}

void show_speckler (void) {
	int i, j;
	printf("REAL PART:\n");
	for (i = 0; i < nsize; i++) {
		printf("%3d ", i);
		for (j = 0; j < nsize; j++) {
			printf("%6.3lf ", speckler[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void show_specklei (void) {
	int i, j;
	printf("IMAGINARY PART:\n");
	for (i = 0; i < nsize; i++) {
		printf("%3d ", i);
		for (j = 0; j < nsize; j++) {
			printf("%6.3lf ", specklei[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[]) {
	speckler = make_matrix_nm(nsize, nsize);
	specklei = make_matrix_nm(nsize, nsize);
	image = make_matrix_nm(nsize, nsize);
	red = make_matrix_nm(nsize, nsize);
	green = make_matrix_nm(nsize, nsize);
	blue = make_matrix_nm(nsize, nsize);

	make_speckle();

	show_speckle();
	show_speckler();
	show_specklei();

	convert_speckle();

	write_speckle();

	free(blue);
	free(green);
	free(red);
	free(image);
	free(specklei);
	free(speckler);
	return 0;
}
