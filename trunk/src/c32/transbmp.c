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
#include <math.h>

/****************************************************************************/

/* first is code for estimating the psatial frequencies */

#define PI  3.141592653589793

#define TWOPI 6.28318530717959

#define NLASERS 41

int verboseflag=1;
int traceflag=0;
int debugflag=0;
int showcolorflag=0;	/* dump out color table */
int showhistoflag=0;	/* show histogram */
int markedgeflag=0;		/* mark i = 0 and j = 0 in image */
int picflag=0;			/* show progress in hough transform */
int centeroutflag=1;
int apodizeflag=1;
int edgefixflag=1;
int circularizeflag=0;
int doubletransform=0;
/* int dumpflag=0; */
int guessfidflag=0;
int adjustfiducials=1;
int experimentalfid=1;
int dohoughtransform=0;		/* do Hough transform */
int colorflag=1;			/* do output in color */
int superimposefiducials=0;	/* superimpose estimated fiducial positions */
int phaseflag=0;			/* estimate phases of beams */

double baseoff=0.0;			/* apodizing offset */

/* double scale = 100.0; */
double scale = 200.0; 
/* double logoffset = 5.0; */
/* double logoffset = 3.0; */
/* double logoffset = 4.0; */
double logoffset = 3.5; 

/* double radiusd=122.0; */		/* max radius of pattern in transform domain */
/* double radiusd=121.0; */		/* max radius of pattern in transform domain */
/* double radiusd=123.0; */		/* max radius of pattern in transform domain */
/* double radiusd=124.0; */  	/* max radius of pattern in transform domain */
/* double radiusd=119.0; */		/* max radius of pattern in transform domain */
/* double radiusd=120.0; */ 	/* max radius of pattern in transform domain */
double radiusd=122.0;  	/* max radius of pattern in transform domain */

/* The radius of the laser spatial frequencies lie at radiusd/2 */

int w=256;
int h=256;						/* width and height of image */

int nlasers = NLASERS;

double ku[NLASERS+NLASERS], kv[NLASERS+NLASERS];	/* spatial frequencies */
/* units are cycles across the image width/height */

long nlen;

/* int *image=NULL; */
/* unsigned char *image=NULL; */
double *image=NULL;				/* image read in from file */

double *cc=NULL;				/* real part of transform */
double *ss=NULL;				/* imaginary part of transform */

double *mag=NULL;				/* magnitude of transform */

double *fftdata=NULL;			/* space for transform of image */

double *hough=NULL;				/* space for hough transform */

unsigned int histogram[256];	/* image gray level histogram */

FILE *errout = stdout;		/* or stderr */

/*****************************************************************************************/

void fixedges (void) {
	int i, j, k;
	for (i = 0; i < h; i++) {
		k = i * w + 255;
		cc[k] = cc[k-1];
		ss[k] = ss[k-1];
	}
	for (j = 0; j < h; j++) {
		k = 255 * w + j;
		cc[k] = cc[k-w];
		ss[k] = ss[k-w];
	}
}

void apodize (void) {
	int i, j, k;
	double weight;
	for (i = 0; i < h; i++) {
		k = i * w;
		weight = (1.0 - cos (TWOPI * i / h)) / 2.0;
		weight = baseoff + (1.0 - baseoff) * weight; 
		for (j = 0; j < w; j++) {
			cc[k] = cc[k] * weight;
			ss[k] = ss[k] * weight;
			k++;
		}
	}
	for (j = 0; j < w; j++) {
		k = j;
		weight = (1.0 - cos (TWOPI * j / w)) / 2.0;
		weight = baseoff + (1.0 - baseoff) * weight; 
		for (i = 0; i < h; i++) {
			cc[k] = cc[k] * weight;
			ss[k] = ss[k] * weight;
			k += w;
		}
	}
}

void circularize (void) {
	int i, j, k;
	double r;
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
			r = sqrt ((double) (i - h/2) * (i - h/2) + (j - w/2) * (j - w/2));
			if (r > h/2 || r > w/2) {
/*				printf("(%d %d) ", i, j); */
				printf(".");
				cc[k] = 0.0;
				ss[k] = 0.0;
			}
			else printf("@");
			k++;
		}
		printf("\n");
	}
}

/***************************************************************************************/

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

void showtrans (void) {
	int i, j, k;
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < h; j++) {
			printf("%d\t%d\t%lg\t%lg\n", i, j, cc[k], ss[k]);
			k++;
		}
	}
}

/* From Numerical Recipes */
/* replaces data[1..2*nn] by its DFT if isign is +1 */
/* replaces data[1..2*nn] by its inverse DFT times nn if isign is -1 */
/* data[1], data[2] are real and imaginary parts of first element */
/* nn must be a power of two */ /* data is 1 based not 0 based */

void dfour1(double data[], unsigned long nn, int isign) {
	unsigned long n, mmax, m, j, istep, i;
/*	double wtempr, wtempi; */
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi;
	long nnx = nn;

	if ((nnx & (- nnx)) != nnx) {
		printf("%ld not a power of two \n", nn);
/*		return; */
	}
/*	first do the bitreversed reordering */
	n = nn << 1;
	j = 1;
	for (i = 1; i < n; i += 2){
		if (j > i) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m = n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
/*	Now for Danielson-Lanczos routine */
	mmax = 2;
	while (n > mmax) {
		istep = mmax << 1;
		theta = isign * (TWOPI / mmax);
		wtemp = sin (0.5 * theta);
		wpr = -2.0 * wtemp * wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 1; m < mmax; m += 2) {
/*			wtemp = sqrt(wr * wr + wi * wi); */
/*			printf("wr %lg\twi %lg\trad %lg\tm %d\t istep %d\n",
				   wr, wi, wtemp, m, istep);  */
			for (i = m; i <= n; i += istep) {
				j = i + mmax;
				tempr = wr * data[j] - wi * data[j+1];
				tempi = wr * data[j+1] + wi * data[j];
				data[j] = data[i] - tempr;
				data[j+1] = data[i+1] - tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr = (wtemp=wr) * wpr - wi * wpi + wr; 
			wi = wi * wpr + wtemp * wpi + wi; 
/*			wtempr = wr * wpr - wi * wpi + wr; 
			wtempi = wi * wpr + wr * wpi + wi;
			wr = wtempr;
			wi = wtempi; */
		}
		mmax = istep;
	}
}

void fasttransform (void) {
	int i, j, k;
/*	copy image into working arrays */
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
			cc[k] = image[k];
			ss[k] = 0.0;
			k++;
		}
	}
	if (edgefixflag) fixedges();
	if (apodizeflag) apodize(); 
	if (circularizeflag) circularize();
	printf("IMAGE:\n");
/*	showtrans(); */

/*	now do horizontal transforms */
	if (fftdata != NULL) free (fftdata);
	fftdata = (double *) malloc ((w * 2 + 1) * sizeof(double));
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
			fftdata[j * 2 + 1] = cc[k];
			fftdata[j * 2 + 2] = ss[k];
			k++;
		}
		dfour1(fftdata, w, 1);
		k = i * w;
		for (j = 0; j < w; j++) {
			cc[k] = fftdata[j * 2 + 1];
			ss[k] = fftdata[j * 2 + 2];
			k++;
		}
	}
	printf("AFTER HORIZONTAL TRANSFORM:\n");
/*	showtrans(); */
/*	now do vertical transforms */
	if (fftdata != NULL) free (fftdata);
	fftdata = (double *) malloc ((h * 2 + 1)  * sizeof(double));
	for (j = 0; j < w; j++) {
		k = j;
		for (i = 0; i < h; i++) {
			fftdata[i * 2 + 1] = cc[k];
			fftdata[i * 2 + 2] = ss[k];
			k += w;
		}
		dfour1(fftdata, w, 1);
		k = j;
		for (i = 0; i < h; i++) {
			cc[k] = fftdata[i * 2 + 1];
			ss[k] = fftdata[i * 2 + 2];
			k += w;
		}
	}

	printf("AFTER VERTICAL TRANSFORM:\n");
/*	showtrans(); */
	if (fftdata != NULL) free (fftdata);
}

/***************************************************************************************/

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

/* stuff to deal with BMP image files */

void showcolortable (RGBQUAD colortable[], int n) {
	int k;
	for (k = 0; k < n; k++) {
		printf("%d\t%d\t%d\t%d\n",
			   k, colortable[k].rgbRed, colortable[k].rgbGreen, colortable[k].rgbBlue);
	}
}

int readhead (FILE *input) {
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
	if (showcolorflag) showcolortable(colortable, bmih.biClrUsed);
	if (verboseflag) putc('\n', stdout);
	fflush(stdout);
	return 0;
}

void makehistogram (void) {
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

void testimage(double A, double u, double v) {
	int i, j, k;
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
			image[k] = A * (1 + cos  (TWOPI * (u * j / w + v * i / h)));
			k++;
		}
	}
}

void transformtoimage (void) {
	int i, j, k;
	for (i= 0; i < h; i++) {
		k = i * w;
		for (j= 0; j < w; j++) {
			image[k] = sqrt(cc[k] * cc[k] + ss[k] * ss[k]);
			image[k] = image[k] * 0.01; 
/*			image[k] = image[k] * 0.05; */
			k++;
		}
	}
}

/* put hough transform data into spatial frequency arrays so can share output routines */

void houghtotransform (void) {
	int i, j, k;
	for (i= 0; i < h; i++) {
		k = i * w;
		for (j= 0; j < w; j++) {
/*			cc[k] = hough[k] * 0.1; */
/*			cc[k] = hough[k] * 0.02; */
			cc[k] = hough[k] * 0.01; 
/*			cc[k] = hough[k] * 0.005;  */ /* *** */
/*			cc[k] = hough[k] * 0.0025; */
/*			cc[k] = hough[k] * 0.001; */
/*			cc[k] = hough[k] * 0.0001; */
			ss[k] = 0.0;
			k++;
		}
	}
}

void guessfiducials (double radius, double dphi, int n) {
	int i;
	double itheta, phi;
	phi  = dphi * TWOPI / 360.0;
	for (i = 0; i < n; i++) {
		itheta = i * TWOPI / n + phi;
		ku[i] = radius * cos (itheta);
		kv[i] = radius * sin (itheta);
	}
}

/* first estimates of spatial frequencies */
/* these can be estimated from Hough transforms of image obtained by */
/* blocking all but one third of the beams at a time */

#ifdef IGNORED
double hotspots[][2]={
	{190,127}, {188,118}, {187,110}, {183,101},
	{179,93}, {173,86}, {167,79}, {159,74}, {150,70},
	{142,67}, {132,66}, {123,66}, {114,67}, {106,70}, {98,74}, {90,78},
	{84,84}, {77,91}, {71,101}, {69,109}, {67,119},
	{73,96}, 
	{66,127}, {67,136},	{69,145}, {73,154}, {77,161}, {83,169}, {89,175}, {97,180},
	{105,184}, {114,187}, {124,188}, {133,188}, {142,186}, {150,184},
	{158,180}, {166,176}, {172,170}, {178,163}, {184,154}, {187,146}, {189,136},
	{183,158},
	{0,0}
};
#endif

double hotspots3[][2]={
	{175, 90},
	{170, 82},
	{163, 76},
	{154, 73},
	{146, 68},
	{138, 67},
	{128, 65},
	{120, 67},
	{110, 69},
	{102, 71},
	{93, 77},
	{86, 83},
	{80, 89},
	{76, 97},
	{0, 0}
};

double hotspots4[][2]={
	{133, 188},
	{142, 186},
	{150, 183},
	{160, 181},
	{165, 175},
	{174, 170},
	{179, 162},
	{184, 153},
	{186, 144},
	{189, 135},
	{189, 126},
	{188, 116},
	{186, 106},
	{182, 99},
	{0, 0},

};

double hotspots5[][2]={
	{75, 98},
	{71, 106},
	{68, 115},
	{67, 124},
	{66, 133},
	{69, 143},
	{71, 151},
	{75, 160},
	{81, 168},
	{87, 173},
	{94, 179},
	{103, 183}, 
	{111, 186},
	{120, 188},
	{0, 0},
};

double hotspots[][2]={
	{175, 90.5},
	{169.6, 82.5},
	{163, 77.5},
	{154, 73.5},
	{146, 69.39},
	{137.6, 67.5},
	{128, 66.5},
	{120, 67.5},
	{110, 69.78},
	{102, 72.5},
	{93, 78},
	{86, 84.5},
	{80, 90.4},
	{76, 97.5},

/*	{75, 98}, */
	{72, 105.5},
	{69, 114.4},
	{68, 123.5},
	{67, 132.5},
	{69, 142.5},
	{72, 150.5},
	{76, 159},
	{81.5, 167.5},
	{88, 172.5},
	{95, 178.4},
	{104, 182.5},
	{112, 185.5},
	{121, 187},

	{131, 187.5},
	{140, 185.5},
	{149, 183.4},
	{158, 180.5},
	{164, 174.4},
	{173, 169.4},
/*	{180, 161.5}, */
	{177, 161.5},
	{182, 152.5},
	{184, 143.5},
	{187, 134.5},
	{187, 125.5},
	{186, 115.5},
	{184, 106},
	{181, 98.5},

	{0, 0},			/* end marker */
};


#ifdef IGNORED
double hotspots[][2]={
	{175, 90},
	{170, 82},
	{163, 76},
	{154, 73},
	{146, 68},
	{138, 67},
	{128, 65},
	{120, 67},
	{110, 69},
	{102, 71},
	{93, 77},
	{86, 83},
	{80, 89},
	{76, 97},

	{133, 188},
	{142, 186},
	{150, 183},
	{160, 181},
	{165, 175},
	{174, 170},
	{179, 162},
	{184, 153},
	{186, 144},
	{189, 135},
	{189, 126},
	{188, 116},
	{186, 106},
	{182, 99},

	{75, 98},
	{71, 106},
	{68, 115},
	{67, 124},
	{66, 133},
	{69, 143},
	{71, 151},
	{75, 160},
	{81, 168},
	{87, 173},
	{94, 179},
	{103, 183}, 
	{111, 186},
	{120, 188},
	{0, 0},
};
#endif

/* Centroid of hot-spots -0.739017 -0.00289508 */

double io=127.0;		/* image center */
double jo=128.0;		/* image center */

/***************************************************************************/

double magnitude(int i, int j) {
	int k = i * w + j;
	double c = cc[k];
	double s = ss[k];
/* 	return (c * c + s * s); */
	return sqrt(c * c + s * s);			/* ??? */
}

void computemag (void) {		/* precompute magnitude array to speed processing */
	int k, n = w * h;	
	for (k = 0; k < n; k++) mag[k] = sqrt(cc[k] * cc[k] + ss[k] * ss[k]);
}

/*         1 2 1 */
/*  (1/16) 2 4 2 */
/*         1 2 1 */

double average (int i, int j) {		/* two-d binomially weighted local average */
	double total = 0.0;
	int k = i * w + j;

	if (i < 1 || i >= h-1 || j < 1 || j >= w-1) return 0.0;
/*	total += magnitude(i-1, j-1) + magnitude(i-1, j+1) +
			 magnitude(i+1, j-1) + magnitude(i+1, j+1);
	total += 2.0 * (magnitude(i-1, j) + magnitude(i, j+1) +
					magnitude(i+1, j) + magnitude(i, j-1));
	total += 4.0 * magnitude(i, j); */
	total += mag[k-w-1] + mag[k-w+1] + mag[k+w-1] + mag[k+w+1];
	total += 2.0 * (mag[k-w] + mag[k+1] + mag[k-w] + mag[k-1]);
	total += 4.0 * mag[k];	
	return total/16.0;
}

double localsum (int i, int j) {	/* sum of power in 3 x 3 area */
	double total = 0.0;
	int k = i * w + j;

	if (i < 1 || i >= h-1 || j < 1 || j >= w-1) return 0.0;
/*	total += magnitude(i-1, j-1) + magnitude(i-1, j+1) +
			 magnitude(i+1, j-1) + magnitude(i+1, j+1);
	total += magnitude(i-1, j) + magnitude(i, j+1) +
			 magnitude(i+1, j) + magnitude(i, j-1);
	total += magnitude(i, j); */
	total += mag[k-w-1] + mag[k-w+1] + mag[k+w-1] + mag[k+w+1];
	total += mag[k-w] + mag[k+1] + mag[k-w] + mag[k-1];
	total += mag[k];
	return total;
}

/* simple bi-linear interpolation */

double interpolatemag (double di, double dj) {
	int i, j, i1, j1;
	double ai, aj;
	double a00, a01, a10, a11;
	double inter;

	i = (int) di;
	j = (int) dj;
	ai = di - i;
	aj = dj - j;
	j1 = (j+1) % w;		/* implement wrap-around */
	i1 = (i+1) % h;		/* implement wrap-around */
/*	a00 = magnitude(i, j);
	a01 = magnitude(i, j+1);
	a10 = magnitude(i+1, j);
	a11 = magnitude(i+1, j+1); */
	a00 = mag[i * w + j];
	a01 = mag[i * w + j1];
	a10 = mag[i1 * w + j];
	a11 = mag[i1 * w + j1];
	inter = aj * (ai * a11 + (1-ai) * a01) + (1-aj) * (ai * a10 + (1-ai) * a00);
	if (debugflag)
	printf("di %lg dj %lg i %d j %d ai %lg aj %lg a00 %lg a01 %lg a10 %lg a11 %lg inter %lg\n",
		   di, dj, i, j, ai, aj, a00, a01, a10, a11, inter);
	return inter; 
}

double interpolatecc (double di, double dj) {
	int i, j, i1, j1;
	double ai, aj;
	double a00, a01, a10, a11;
	double inter;

	i = (int) di;
	j = (int) dj;
	ai = di - i;
	aj = dj - j;
	j1 = (j+1) % w;		/* implement wrap-around */
	i1 = (i+1) % h;		/* implement wrap-around */
	a00 = cc[i * w + j];
	a01 = cc[i * w + j1];
	a10 = cc[i1 * w + j];
	a11 = cc[i1 * w + j1];
	inter = aj * (ai * a11 + (1-ai) * a01) + (1-aj) * (ai * a10 + (1-ai) * a00);
	return inter; 
}

double interpolatess (double di, double dj) {
	int i, j, i1, j1;
	double ai, aj;
	double a00, a01, a10, a11;
	double inter;

	i = (int) di;
	j = (int) dj;
	ai = di - i;
	aj = dj - j;
	j1 = (j+1) % w;		/* implement wrap-around */
	i1 = (i+1) % h;		/* implement wrap-around */
	a00 = ss[i * w + j];
	a01 = ss[i * w + j1];
	a10 = ss[i1 * w + j];
	a11 = ss[i1 * w + j1];
	inter = aj * (ai * a11 + (1-ai) * a01) + (1-aj) * (ai * a10 + (1-ai) * a00);
	return inter; 
}

double uderivative (double di, double dj) {
	int i, j, i1, j1;
	double ai, aj, dfdx;
	double a00, a01, a10, a11;

	i = (int) di;
	j = (int) dj;
	ai = di - i;
	aj = dj - j;
	j1 = (j+1) % w;		/* implement wrap-around */
	i1 = (i+1) % h;		/* implement wrap-around */
/*	a00 = magnitude(i, j);
	a01 = magnitude(i, j+1);
	a10 = magnitude(i+1, j);
	a11 = magnitude(i+1, j+1); */
	a00 = mag[i * w + j];
	a01 = mag[i * w + j1];
	a10 = mag[i1 * w + j];
	a11 = mag[i1 * w + j1];
	dfdx =  ai * (a11 - a10) + (1 - ai) * (a01 - a00);
	if (debugflag)
	printf("di %lg dj %lg i %d j %d ai %lg aj %lg a00 %lg a01 %lg a10 %lg a11 %lg dfdx %lg\n",
		   di, dj, i, j, ai, aj, a00, a01, a10, a11, dfdx);
	return dfdx;
}

double vderivative (double di, double dj) {
	int i, j, i1, j1;
	double ai, aj, dfdy;
	double a00, a01, a10, a11;

	i = (int) di;
	j = (int) dj;
	ai = di - i;
	aj = dj - j;
	j1 = (j+1) % w;		/* implement wrap-around */
	i1 = (i+1) % h;		/* implement wrap-around */
/*	a00 = magnitude(i, j);
	a01 = magnitude(i, j+1);
	a10 = magnitude(i+1, j);
	a11 = magnitude(i+1, j+1); */
	a00 = mag[i * w + j];
	a01 = mag[i * w + j1];
	a10 = mag[i1 * w + j];
	a11 = mag[i1 * w + j1];
	dfdy = aj * (a11 - a01) + (1 - aj) * (a10 - a00);
	if (debugflag)
	printf("di %lg dj %lg i %d j %d ai %lg aj %lg a00 %lg a01 %lg a10 %lg a11 %lg dfdy %lg\n",
		   di, dj, i, j, ai, aj, a00, a01, a10, a11, dfdy);
	return dfdy;
}

double sumfiducials (double xo, double yo, int n) {
	int i, j, ii, jj;
	double x, y, r, weight;
	double total=0.0;

	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			if (i == j) continue;
			x = xo + (ku[i] - ku[j]);
			y = yo + (kv[i] - kv[j]);
			r = sqrt(x*x + y*y);
			if (n < 20)	/* subset of lasers */
				weight  = 1.0 - cos (r/2 * TWOPI / radiusd);
			else	/* full set of lasers */
				weight  = 1.0 - cos (r * TWOPI / radiusd);
/*			if (x < 0.0) x = w + x; */
			if (x < -0.5) x = w + x;
/*			if (y < 0.0) y = h + y; */
			if (y < -0.5) y = h + y;
			jj = (int) (x + 0.5);
			ii = (int) (y + 0.5);
/*			if (ii > 0 && ii < h-1 && jj > 0 && jj < w-1) {
				total += average(ii, jj) * weight;
			} */
/*			if (ii > 0 && ii < h-1 && jj > 0 && jj < w-1) { */
			if (ii >= 0 && ii < h && jj >= 0 && jj < w) {
				total += interpolatemag(y, x) * weight;
			}
		}
	}
/*	return sqrt (total / (n * n)); */
	return total / (n * n);
}

void powerinbeams (double xo, double yo, int n) {
	int i, j, ii, jj;
	double x, y, r, weight;
	double total, sumw;

	for (i = 0; i < n; i++) {
		total = 0.0;
		sumw = 0.0;
		for (j = 0; j < n; j++) {
			if (i == j) continue;			/* ignore shared DC component */
			x = xo + (ku[i] - ku[j]);
			y = yo + (kv[i] - kv[j]);
			r = sqrt(x*x + y*y);
			if (n < 20)	/* subset of lasers */
				weight  = 1.0 - cos (r/2 * TWOPI / radiusd);
			else	/* full set of lasers */
				weight  = 1.0 - cos (r * TWOPI / radiusd);
/*			if (x < 0.0) x = w + x; */
			if (x < -0.5) x = w + x;
/*			if (y < 0.0) y = h + y; */
			if (y < -0.5) y = h + y;
			jj = (int) (x + 0.5);
			ii = (int) (y + 0.5);
/*			if (ii > 0 && ii < h-1 && jj > 0 && jj < w-1) { */
			if (ii >= 0 && ii < h && jj >= 0 && jj < w) {
				total += localsum(ii, jj) * weight;
				sumw += weight;
			}
		}
/*		printf("Amplitude beam %2d: %d\n", i, (int) (total / (double) n)); */
		printf("Amplitude beam %2d: %d\n", i, (int) (total / sumw));
	}
}

void showfiducials(int n) {
	int k;
	double u, v, r, theta, dk, di, dj;
	printf("\n");
	for (k = 0; k < n; k++) {
		u = ku[k];
		v = kv[k];
		r = sqrt(u * u + v * v);
		theta = atan2(v, u);
		if (theta < 0.0) theta += TWOPI;
		dk = theta / (TWOPI/NLASERS);
		dj = u + jo;
		di = io - v;
		printf("%d\t%lg\t%lg\t(%lg %lg)\t%lg %lg\n",
			   k, ku[k], kv[k], r, dk, dj, di);
	}
	printf("\n");
/*	for (k = 0; k < n; k++) {
		u = ku[k] + 0.678;
		v = kv[k] + 0.007;
		r = sqrt(u * u + v * v);
		theta = atan2(v, u);
		if (theta < 0.0) theta += TWOPI;
		dk = theta / (TWOPI/NLASERS);
		dj = u + jo;
		di = io - v;
		printf("%d\t%lg\t%lg\t(%lg %lg)\t%lg %lg\n",
			   k, ku[k], kv[k], r, dk, dj, di);
	}
	printf("\n"); */
}

void meanfiducials(int n) {
	int k;
	double u0=0.0, v0=0.0;
	for (k = 0; k < n; k++) {
		u0 += ku[k];
		v0 += kv[k];
	}
	printf("Centroid of hot-spots %lg %lg\n",
		   u0 / n, v0 / n);
}

double ugradient[NLASERS];
double vgradient[NLASERS];

double setupgradient (double xo, double yo, int n) {
	int i, j, ii, jj;
	double totalu=0.0, totalv=0.0;
	double x, y, r, weight;
	double total=0.0;

	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			if (i == j) continue;
			x = xo + (ku[i] - ku[j]);
			y = yo + (kv[i] - kv[j]);
			r = sqrt(x*x + y*y);
			if (n < 20)	/* subset of lasers */
				weight  = 1.0 - cos (r/2 * TWOPI / radiusd);
			else	/* full set of lasers */
				weight  = 1.0 - cos (r * TWOPI / radiusd);
/*			if (x < 0.0) x = w + x; */
			if (x < -0.5) x = w + x;
/*			if (y < 0.0) y = h + y; */
			if (y < -0.5) y = h + y;
			jj = (int) (x + 0.5);
			ii = (int) (y + 0.5);
/*			if (ii > 0 && ii < h-1 && jj > 0 && jj < w-1) { */
			if (ii >= 0 && ii < h && jj >= 0 && jj < w) {
				totalu += uderivative (y, x);
				totalv += vderivative (y, x);
			}
		}
		ugradient[i] = totalu;
		vgradient[i] = totalv;
		total += totalu * totalu + totalv * totalv;
	}
	return sqrt(total);
}

void showgradients(int nlasers) {
	int k;
	printf("GRADIENTS:\n");
	for (k = 0; k < nlasers; k++) {
		printf("%d\t(%lg %lg)\t(%lg %lg)\n",
			   k, ku[k], kv[k], ugradient[k], vgradient[k]);
	}
}

void movealonggradient (double delta, int nlasers) {
	int k;
	for (k = 0; k < nlasers; k++) {
		ku[k] += delta * ugradient[k];
		kv[k] += delta * vgradient[k];
	}
};

int iteratefiducials(double xo, double yo, double eps, int nlasers) {
	double total, new, old, gradmag, e, test;
	int k;

	computemag();				/* precompute magnitude of transform for convenience */
/*	eps = -eps; */
	total = sumfiducials(xo, yo, nlasers);
	printf("OLD TOTAL %lg eps %lg\n", total, eps);
	gradmag = setupgradient(xo, yo, nlasers);
	if (traceflag) showgradients(nlasers);
	e = eps / gradmag;
	movealonggradient(e, nlasers);
	new = sumfiducials(xo, yo, nlasers);
	printf("NEW TOTAL %lg eps %lg\n", new, eps);
	movealonggradient(-e, nlasers);		/* undo previous motion */
	old = new;
	if (new < total) {		/* too far try smaller steps */
		printf("TRY SMALLER STEP\n");
		eps = eps / sqrt(2.0);				/* try smaller step */
		for (k = 0; k < 16; k++) {
			e = eps / gradmag;
			movealonggradient(e, nlasers);
			new = sumfiducials(xo, yo, nlasers);
			printf("NEW TOTAL %lg eps %lg\n", new, eps);
			movealonggradient(-e, nlasers);		/* undo motion */
			if (new < old) {					/* smaller step is worse */
				if (old < total) {
					printf("Giving up: old %lg < total %lg\n", old, total);
					return 0;		/* no improvement - don't move at all */
				}
				eps = eps * sqrt(2.0);			
				e = eps / gradmag;
				movealonggradient(e, nlasers);		/* reset to last one that worked */
				test = sumfiducials(xo, yo, nlasers);
				if (test != old) printf("ERROR! test %lg old %lg\n", test, old);
				printf("Improved: old %lg > total %lg\n", old, total);
				return 1;						/* succeeded to improve */
			}
			eps = eps / sqrt(2.0);				/* try smaller step */
			old = new;
		}
		printf("Giving up: 16 iterations\n");
		if (old < total) return 0;
		eps = eps * sqrt(2.0);			
		e = eps / gradmag;
		movealonggradient(e, nlasers);		/* reset to last one that worked */
		test = sumfiducials(xo, yo, nlasers);
		if (test != old) printf("ERROR! test %lg old %lg\n", test, old);
		printf("Improved: old %lg > total %lg\n", old, total);
		return 1;						/* succeeded to improve */
	}
	else {
		printf("TRY LARGER STEP\n");
		eps = eps * sqrt(2.0);				/* try larger step */
		for (k = 0; k < 16; k++) {
			e = eps / gradmag;
			movealonggradient(e, nlasers);
			new = sumfiducials(xo, yo, nlasers);
			printf("NEW TOTAL %lg eps %lg\n", new, eps);
			movealonggradient(-e, nlasers);		/* undo motion */
			if (new < old) {					/* larger step is worse */
				if (old < total) {
					printf("Giving up: old %lg < total %lg\n", old, total);
					return 0;		/* no improvement - don't move at all */
				}
				eps = eps / sqrt(2.0);
				e = eps / gradmag;
				movealonggradient(e, nlasers);		/* reset to last one that worked */
				test = sumfiducials(xo, yo, nlasers);
				if (test != old) printf("ERROR! test %lg old %lg\n", test, old);
				printf("Improved: old %lg > total %lg\n", old, total);
				return 1;						/* succeeded to improve */
			}
			eps = eps * sqrt(2.0);				/* try larger step */
			old = new;
		}
		printf("Giving up: 16 iterations\n");
		if (old < total) return 0;
		eps = eps / sqrt(2.0);
		e = eps / gradmag;
		movealonggradient(e, nlasers);		/* reset to last one that worked */
		test = sumfiducials(xo, yo, nlasers);
		if (test != old) printf("ERROR! test %lg old %lg\n", test, old);
		printf("Improved: old %lg > total %lg\n", old, total);
		return 1;						/* succeeded to improve */
	}
	return 0;
}

int setexpfiducials (double hotspots[][2]) {	/* experimental data */
	double di, dj;
	int k, kend;
	for (k = 0; k < 256; k++) {
		dj = hotspots[k][0];
		di = hotspots[k][1];
		if (di == 0.0 && dj == 0.0) {
			kend = k;
			break;
		}
		ku[k] = dj - jo;
		kv[k] = io - di;
	}
	printf("Set up %d hot spots\n", kend);
	return kend;
}

/* mark spots in the spatial transform image */

void addfiducials (double xo, double yo, int n) {
	int i, j, ii, jj, k;
	double x, y;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
/*			if (i == j) continue; */
			x = xo + (ku[i] - ku[j]);
			y = yo + (kv[i] - kv[j]);
/*			if (x < 0.0) x = w + x; */
			if (x < -0.5) x = w + x;
/*			if (y < 0.0) y = h + y; */
			if (y < -0.5) y = h + y;
			jj = (int) (x + 0.5);
			ii = (int) (y + 0.5);
			if (ii >= 0 && ii < h && jj >= 0 && jj < w) {
				k = w * ii + jj;
				cc[k] = 100000.0;
				ss[k] = 0.0;
			}
		}
	}
}


/*************************************************************************/

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

/**********************************************************************************/

int roundup (int n) {
	return (((n-1) / 4) + 1) * 4;
}

void writehead (FILE *output, int colorflag) {
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

void spaceshift() {
	int i, j, k;
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
			if (((i + j) % 2) != 0) {
				cc[k] = -cc[k];
				ss[k] = -ss[k];
			}
			k++;
		}
	}
}

#ifdef IGNORED
g = c;
r = s * 0.866 - c * 0.5;
b = - s * 0.866 - c * 0.5;

if (s > 0) g = c + 0.5 * s;
else g = c - 0.5 * s;

if (0.866 * c + 0.5 * s > 0) r = s;
else r = - 0.5 * c + 0.866 * s;

if (0.866 * c - 0.5 * s > 0) b = - s;
else b = - 0.5 * c - 0.866 * s;
#endif

/* red   along x-axis - 60 degree */
/* green along x-axis + 60 degrees */
/* blue  along x-axis - 180 degrees */
/* atan2 returns angle in radians between -PI and +PI */

double rcomponent (double dtheta) {
/*	if (theta > PI || theta < -PI) printf("%lg ", theta); */
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
/*	dtheta -= 120.0; */
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
/*	dtheta += 120.0; */
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

double square(double x) {
	return x * x;
}

void writebmp (FILE *output, int colorflag) {
	int i, j, k;
	double c, s, rcs, rcl;
	double r, g, b;
	int ir, ig, ib;
	int rem;
	double theta;
	
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
			if (centeroutflag) {
/*				k = i * w + j; */
				k = ((i + h/2) % h) * w + ((j + w/2) % w);
			}
			c = cc[k];
			s = ss[k];

/*			if (c == 0 && s == 0) printf("(%d %d) ", i, j); */
			c = c / (h * w);
			s = s / (h * w);
			if (apodizeflag) {
				c = c * 4.0;
				s = s * 4.0;
			} 
			rcs = sqrt(c * c + s * s);
			rcl = log(rcs)+logoffset; 
			if (rcl < 0.0) rcl = 0.0;
			if (rcs > 0.0) {
/*				c = c / sqrt(rcs); */
/*				s = s / sqrt(rcs); */
				c = (c / rcs) * rcl;
				s = (s / rcs) * rcl;
			}
			
			theta = atan2(s, c) * 360.0 / TWOPI;

			r = sqrt(rcomponent (theta)) * rcl;
			g = sqrt(gcomponent (theta)) * rcl;
			b = sqrt(bcomponent (theta)) * rcl;
#ifdef IGNORED
			r = square(rcomponent (theta)) * rcl;
			g = square(gcomponent (theta)) * rcl;
			b = square(bcomponent (theta)) * rcl;
#endif			
			if (r > 0.0) ir = (int) (scale * r);
			else ir = 0;
			if (ir > 255) ir = 255;
			if (g > 0.0) ig = (int) (scale * g);
			else ig = 0;
			if (ig > 255) ig = 255;			
			if (b > 0.0) ib = (int) (scale * b);
			else ib = 0;
			if (ib > 255) ib = 255;

			if (markedgeflag) {
/*				ir = ig = ib = 0;	ib = 64; */
				if (i == 0) {		/* bottom line in image */
					ir = 255;
				}
				if (j == 0) {		/* left edge of image */
					ig = 255;
				}
				if (i == 255) {		/* top line of image */
					ig = 255;
					ib = 255;
				}
				if (j == 255) {		/* right edge of image */
					ir = 255;
					ib = 255;
				} 
				if (i == 0 && j == 0) ir = ig = ib = 255;
				if (i == 255 && j == 255) ir = ig = ib = 255;
				if (i == 0 && j == 255) ir = ig = ib = 0;
				if (i == 255 && j == 0) ir = ig = ib = 0;
				if (i == h/2 && j == w/2) ir = ig = ib = 0;
			}

			if (colorflag) {
				putc(ib, output); 
				putc(ig, output); 
				putc(ir, output);
			}
			else putc(ig, output);
			k++;
		}
		if (colorflag) rem = (w * 3) % 4;
		else rem = w % 4;
		while (rem != 0) {
			putc(0, output);
			rem = (rem + 1) % 4;
		}			
	}
}

#ifdef IGNORED
void dumptrans (FILE *output) {
	long nc, ns;
	nc = fwrite(cc, w * h, sizeof(double), output);
	ns = fwrite(ss, w * h, sizeof(double), output);
	printf("nc %ld ns %ld\n", nc, ns);
}
#endif

#ifdef IGNORED
void undumptrans (FILE *input) {
	long nc, ns;
	nc = fread(cc, w * h, sizeof(double), input);
	ns = fread(ss, w * h, sizeof(double), input);
	printf("nc %ld ns %ld\n", nc, ns);
}
#endif

/* free various allocations - mostly for fun and debugging only */

void freestuff(void) {
	if (cc != NULL) free(cc);
	if (ss != NULL) free(ss);
	if (mag != NULL) free(mag);
	if (image != NULL) free (image);
	if (fftdata != NULL) free (fftdata);
	if (hough != NULL) free (hough);
#ifdef IGNORED
	if (cosih != NULL) free (cosih);
	if (sineh != NULL) free (sineh);
	if (cosiv != NULL) free (cosiv);
	if (sinev != NULL) free (sinev);
#endif
}

#define NSIZE 16

void testit (void) {
	int i;
	double data[NSIZE+NSIZE+1];
	int n = NSIZE;
	for (i = 0; i < n; i++) {
/*		data[i+i+1] = cos (TWOPI * i / n); */
/*		data[i+i+2] = sin (TWOPI * i / n); */
		data[i+i+1] = 0.0;
		data[i+i+2] = 0.0;		
	}
/*	data[1] = 1.0; */
	data[3] = 1.0;
	dfour1(data, n, 1);
	for (i = 0; i < n; i++)
		printf("%3d %lg %lg\n", i, data[i+i+1], data[i+i+2]);
}

void allocatecands(void) {
	if (cc != NULL || ss != NULL) {
		fprintf(stderr, "ERROR: arrays already allocated\n");
		exit(1);
	}
	cc = (double *) malloc (sizeof(double) * w * h);
	ss = (double *) malloc (sizeof(double) * w * h);
	if (cc == NULL || ss == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate memory\n");
		exit(1);
	}
	mag = (double *) malloc (sizeof(double) * w * h);
	if (mag == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate memory\n");
		exit(1);
	}
}

void allochough(void) {
	if (hough != NULL) {
		fprintf(stderr, "ERROR: arrays already allocated\n");
		exit(1);
	}
	hough = (double *) malloc (sizeof(double) * w * h);
	if (hough == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate memory\n");
		exit(1);
	}
}

double dabs(double x) {
	if (x < 0.0) return -x;
	else return x;
}

int round (double x) {
	if (x < 0.0) return - ((int) (-x + 0.5));
	else return (int) (x + 0.5);
}

void resethough (void) {
	int i, j, k;
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) hough[k++] = 0.0;
	} 
}

void spread (double xo, double yo, double amp, double radius) {
	int i, j, iold, jold, k, kk;
	double theta;
	double x, y;
	int ndel = w + h;

	iold = jold = -1;
	for (k = 0; k < ndel; k++) {
		theta = k * TWOPI / ndel;
		x = xo + radius * cos(theta);
		y = yo + radius * sin(theta);
		if (x >= w/2 || x <= -w/2) continue;
		if (y >= h/2 || y <= -h/2) continue;
		if (x < -0.5) j = (int) (x + w + 0.5);
		else j = (int) (x + 0.5);
		if (y < -0.5) i = (int) (y + h + 0.5);
		else i = (int) (y + 0.5);
		if (i < 0 || i >= h || j < 0 || j >= w) {
			printf("ERROR %lg %lg %d %d ", x, y, i, j);
			continue;
		}
		if (i != iold || j != jold) {
			kk = i * w + j;
			hough[kk] += amp;
		}
		iold = i;
		jold = j;
	}
}

void houghtransform(double radius) {
	int i, j, k;
	double amp;
	double epsl = 1.0e-9;
	double xo, yo, ro, weight;

	resethough();
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
			amp = sqrt (cc[k] * cc[k] + ss[k] * ss[k]);
			if (j >= w/2) xo = j - w;
			else xo = j;
			if (i >= h/2) yo = i - h;
			else yo = i;
			ro = sqrt(xo*xo + yo*yo);
			if (ro >= radiusd) {
				if (picflag) putc(':', stdout);
				k++;
				continue;
			}
			weight  = 1.0 - cos (ro * TWOPI / radiusd);
			amp = amp * weight;
/*			if (amp > 1000.0) {*/
/*			if (amp > 250.0) { */
			if (amp > 500.0) {
				amp = sqrt (amp * 5000.0);
				spread(xo, yo, amp, radius);
				if (picflag) putc('@', stdout);
			}
			else if (picflag) putc('.', stdout);
			k++;
		}
		if (picflag) putc('\n', stdout);
		else putc('.', stdout);
	}
}

void checktransform(void) {
	int i, j, k, io, jo, ko;
	double am, amo, err;
	int imax=0, jmax=0;
	double errmax = 0.0;
	for (i = 1; i < h; i++) {
		io = h - i;
		for (j = 1; j < w; j++) {
			jo = w - j;
			k = i * w + j;
			ko = io * w + jo;
			am = sqrt(cc[k] * cc[k] + ss[k] * ss[k]);
			amo = sqrt(cc[ko] * cc[ko] + ss[ko] * ss[ko]);
			err = dabs(am - amo);
			if (err > errmax) {
				errmax = err;
				imax = i;
				jmax = j;
			}
			if (err > .1)
				printf("(%d %d) %lg  (%d %d) %lg\n", i, j, am, io, jo, amo);
		}
	}
	printf("Transform max symmetry error %lg at (%d %d)\n", errmax, imax, jmax);
}

void checkhough(void) {
	int i, j, k, io, jo, ko;
	double err;
	int imax=0, jmax=0;
	double errmax = 0.0;
	for (i = 1; i < h; i++) {
		io = h - i;
		for (j = 1; j < w; j++) {
			jo = w - j;
			k = i * w + j;
			ko = io * w + jo;
			err = dabs(hough[k] - hough[ko]);
			if (err > errmax) {
				errmax = err;
				imax = i;
				jmax = j;
			}
			if (err > .1)
				printf("(%d %d) %lg  (%d %d) %lg\n", i, j, hough[k],
					   io, jo, hough[ko]);
		}
	}
	printf("Hough max symmetry error %lg at (%d %d)\n", errmax, imax, jmax);
}

/**********************************************************************************************/

/* second comes code for phase estimation code */

/* N vectors of unknown magnitude and phase */

#define NLASERS 41

double c[NLASERS][NLASERS];			/* measured dot-products x_i * x_j + y_i * y_j */

double s[NLASERS][NLASERS];			/* measured cross-products y_i * x_j - x_i * y_j */

double wt[NLASERS][NLASERS];		/* weighting factor - reliability of this estimate */

double xo[NLASERS], yo[NLASERS];	/* test values of vectors - used only to compute c & s */

double x[NLASERS], y[NLASERS];		/* current guesses at vectors */

double xu[NLASERS], yu[NLASERS];	/* new guesses at vectors constructed in iteration */

int nlaser=NLASERS;					/* number of sources */

double minamplitude=0.8;

double maxamplitude=1.2;

/* double noise=0.01; */	/* noise in pairwise phase estimates */
double noise=0.0;

double initerr=1.0; 	/* error in initial guesses */
/* double initerr=0.0; */

int randominit=1;		/* use random initial values */

/* int niter=100; */		/* number of iterations */
int niter=100;

 double alpha=1.0;			/* damping underrelaxation */
/* double alpha=0.1; */		/* damping underrelaxation */

double beta = 0.5;		/* optimal value 0.5 */

int gaussjordan=0;		/* update as you go if set */

int testflag=0;			/* 0 => use data derived from image */

double total;

/******************************************************************************/

/* next part is for setting up random test data for phase estimation */

double random (double rmin, double rmax) {
	return ((double) rand() / (double) RAND_MAX) * (rmax - rmin) + rmin;
}

void setup_test(void) {
	nlaser = 3;
	xo[0] = 1.0; yo[0] = 0.0;
	xo[1] = 0.0; yo[1] = 1.0;
	xo[2] = -1.0; yo[2] = -1.0;
}

/* make up n random vectors in range minamplitude to maxamplitude */

void init_vec (int n) {
	int i;
	double xu, yu, rn;
	for (i = 0; i < n; i++) {
		for (;;) {
			xu = random(-maxamplitude, maxamplitude);
			yu = random(-maxamplitude, maxamplitude);
			rn = sqrt(xu * xu + yu * yu);
			if (rn > minamplitude && rn < maxamplitude) break;
		}
		xo[i] = xu; yo[i] = yu;
	}
}

/* compute real and imaginary parts of interference from spatial frequencies */

void init_measurements (int n) {
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = i; j < n; j++) {
			c[i][j] = xo[i] * xo[j] + yo[i] * yo[j] + random(-noise, noise);
			s[i][j] = xo[i] * yo[j] - yo[i] * xo[j] + random(-noise, noise);
			wt[i][j] = 1.0;
			c[j][i] = c[i][j];
			s[j][i] = -s[i][j];
			wt[j][i] = wt[i][j];
		}
		s[i][i] = 0.0;
	}
}

double dc=0.0;

void extractphaseinfo (double xo, double yo, int n) {
	int i, j, ii, jj;
	double ccomp, scomp;
	double x, y, r, weight;

	if (verboseflag) printf("Extracting phase info from transform n %d\n", n);
	ccomp = interpolatecc (yo, xo);
	scomp = interpolatess (yo, xo);
	dc = ccomp; 						/* DC component */
/*	dc = ccomp * ccomp; */				/* DC component */
	if (verboseflag) printf("DC %lg %lg => %lg\n", ccomp, scomp, dc);

	for (i = 0; i < n; i++) {
		for (j = i; j < n; j++) {
			if (i == j) continue;		/* ignore DC */
			x = xo + (ku[i] - ku[j]);
			y = yo + (kv[i] - kv[j]);
			r = sqrt(x*x + y*y);
			if (n < 20)	/* subset of lasers */
				weight  = 1.0 - cos (r/2 * TWOPI / radiusd);
			else	/* full set of lasers */
				weight  = 1.0 - cos (r * TWOPI / radiusd);
/*			if (x < 0.0) x = w + x; */
			if (x < -0.5) x = w + x;
/*			if (y < 0.0) y = h + y; */
			if (y < -0.5) y = h + y;
			jj = (int) (x + 0.5);
			ii = (int) (y + 0.5);
/*			if (ii > 0 && ii < h-1 && jj > 0 && jj < w-1) { */
			if (ii >= 0 && ii < h && jj >= 0 && jj < w) {
/*				interpolate in spatial frequency domain */
/*				use local average instead ? */
				ccomp = interpolatecc(y, x);
				scomp = interpolatess(y, x);
				if (debugflag) printf("i %d j %d cc %lg ss %lg\n", i, j, ccomp, scomp);
			}
			else {
				ccomp = scomp = weight = 0.0;
				fprintf(errout, "`Impossible' ii %d jj %d\n", ii, jj);
				fprintf(errout, "i %d j %d x %lg y %lg r %lg weight %lg\n",
						i, j, x, y, r, weight);
				fprintf(errout, "ku[i] %lg kv[i] %lg ku[j] %lg kv[j] %lg\n",
						ku[i], kv[i], ku[j], kv[j]);
			}
			wt[i][j] = weight;
			c[i][j] = ccomp;		/* real part */
			s[i][j] = scomp;		/* imaginary part */
			c[j][i] = c[i][j];
			s[j][i] = -s[i][j];
			wt[j][i] = wt[i][j];
		}
		c[i][i] = 0.0;
		s[i][i] = 0.0;
		wt[i][i] = 0.0;
	}
}

double total_error (int n, int showflag) {
	int i, j;
	double cx, sx, dx, dy, weight;
	double sum=0.0;
	double sumw=0.0;

	for (i = 0; i < n; i++) {
		for (j = i; j < n; j++) {
			cx = x[i] * x[j] + y[i] * y[j];
			dx = cx - c[i][j];
			sx = x[i] * y[j] - y[i] * x[j];
			dy = sx - s[i][j]; 
			weight = wt[i][j];
			if (debugflag || showflag)
				printf("%d\t%d\t(%lg %lg)\t(%lg %lg)\n",
								   i, j, c[i][j], s[i][j], cx, sx);
			sum += (dx * dx + dy * dy) * weight;
			sumw += weight;
		}
	}
/*	return sqrt (sum / (double) n); */
	return sqrt (sum / sumw);
}

void init_guess (int n) {
	int i;
	if (randominit) {
		for (i = 0; i < n; i++) {
			x[i] = random(-maxamplitude, maxamplitude);
			y[i] = random(-maxamplitude, maxamplitude);
		}
	}
	else {
		for (i = 0; i < n; i++) {
			x[i] = xo[i] + random(-initerr, initerr);
			y[i] = yo[i] + random(-initerr, initerr);
		}
	}
}

double total_power (int n) {	/* sum of squares of length of vectors */
	int i;
	double sum=0.0;
	for (i = 0; i < n; i++)	sum += x[i] * x[i] + y[i] * y[i];
	return sum;
}

double total_power_u (int n) {	/* sum of squares of length of vectors */
	int i;
	double sum=0.0;
	for (i = 0; i < n; i++)	sum += xu[i] * xu[i] + yu[i] * yu[i];
	return sum;
}

/* only useful on synthetic data ! in real data DC component is shared */

double dc_power (int n) {
	int k;
	double weight;
	double sum=0.0, sumw = 0.0;

	if (dc != 0.0) return dc;		/* if precomputed */

	for (k = 0; k < n; k++)	{
		weight = wt[k][k];
		sum += c[k][k] * weight;
		sumw += weight;
		printf("k %d c[k][k] %lg wt[k][k] %lg\n", k, c[k][k], wt[k][k]);
	}
	if (sumw == 0.0) {
		printf("Sum of weights is zero\n");
		return 0.0;
	}
	return (sum / sumw);
}

#ifdef IGNORED
void normalize_guess(int n) {
	double dc, total, scale;
	int i;
	dc = dc_power(n);
	total = total_power(n);
	scale = sqrt(dc / total);
	for (i = 0; i < n; i++) {
		x[i] *= scale;
		y[i] *= scale;
	}
}
#endif

void normalize_amplitude(int n) {
	int i, j;
	double totalcs=0.0;
	double totalxy=0.0;
	double scale;
	
/*	compute this sum only once ? use upper triangle only and double ? */
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			totalcs += ( c[i][j] * c[i][j] + s[i][j] * s[i][j] ) *  wt[i][j];
		}
	}
	for (i = 0; i < n; i++) {
		totalxy += (x[i] * x[i] + y[i] * y[i]);
	}
/*	totalcs should equal totalxy ^ 2 */
	scale = sqrt(sqrt(totalcs) / totalxy);
	if (verboseflag) printf("sqrt(totalcs) %lg totalxy %lg scale %lg\n", sqrt(totalcs), totalxy, scale);
	for (i = 0; i < n; i++) {
		x[i] *= scale;
		y[i] *= scale;
	}
}

double new_x (int k, int n, double scale) {
	int i;
	double weight;
	double sum=0.0, sumw = 0.0;

	for (i = 0; i < n; i++) {
		weight = wt[k][i];
		sum += (c[k][i] * x[i] + s[k][i] * y[i]) * weight;
		sumw += weight;
	}
	if (sumw == 0.0) {
		printf("Sum of weights is zero\n");
		return 0.0;
	}
	return sum / (scale * sumw);
}

double new_y (int k, int n, double scale) {
	int i;
	double weight;
	double sum=0.0, sumw = 0.0;

	for (i = 0; i < n; i++)	{
		weight = wt[k][i];
		sum += (c[k][i] * y[i] - s[k][i] * x[i]) * weight;
		sumw += weight;
	}
	if (sumw == 0.0) {
		printf("Sum of weights is zero\n");
		return 0.0;
	}
	return sum / (scale * sumw);
}

void iterate_phase (int m, int n) {
	int i;
	double terror, oldtotal, dc, scale;

	oldtotal = total;
	total = total_power(n);
/*	total = (oldtotal + total) / 2.0; */
	terror = total_error(n, 0);
	dc = dc_power(n);
	if (verboseflag) printf("%d\ttotal %lg\tdc %lg\terror %lg\n", m, total, dc, terror);
/*	scale = sqrt(dc * total); */
/*	scale = total; */
/*	scale = (dc + total)/2; */
	scale = beta * total + (1 - beta) * dc;
	for (i = 0; i < n; i++) {
		xu[i] = new_x(i, n, scale);
		yu[i] = new_y(i, n, scale);
		if (gaussjordan) {						/* replace right away */
			x[i] = alpha * xu[i] + (1 - alpha) * x[i];
			y[i] = alpha * yu[i] + (1 - alpha) * y[i];
		}
	}
	if (! gaussjordan) {						/* replace at end of iteration */ 
		for (i = 0; i < n; i++) {
			x[i] = alpha * xu[i] + (1 - alpha) * x[i];
			y[i] = alpha * yu[i] + (1 - alpha) * y[i];
		}
	}
/*	normalize_guess(nlaser); */
	normalize_amplitude(nlaser);
/*	normalize_amplitude(nlaser); */
}

void iterate_phase_x (int m, int n) {
	int i;
	double terror, oldtotal;

	oldtotal = total;
	total = total_power(n);
	terror = total_error(n, 0);
	if (verboseflag) printf("%d\ttotal %lg\terror %lg\n", m, total, terror);

	for (i = 0; i < n; i++) {
		xu[i] = new_x(i, n, 1.0);
		yu[i] = new_y(i, n, 1.0);
		if (gaussjordan) {						/* replace right away */
			x[i] = alpha * xu[i] + (1 - alpha) * x[i];
			y[i] = alpha * yu[i] + (1 - alpha) * y[i];
		}
	}
	if (! gaussjordan) {						/* replace at end of iteration */ 
		for (i = 0; i < n; i++) {
			x[i] = alpha * xu[i] + (1 - alpha) * x[i];
			y[i] = alpha * yu[i] + (1 - alpha) * y[i];
		}
	}
	normalize_amplitude(nlaser);
/*	normalize_amplitude(nlaser); */
	return;
}

#ifdef IGNORED
	double total_u, lambda, scale;

	total_u = total_power_u(n);
	lambda = sqrt(total_u / total);
	printf("lambda %lg\n", lambda);
/*	scale = 1.0 / sqrt(total_u); */
	scale = sqrt(lambda) / sqrt(total_u);
	for (i = 0; i < n; i++) {
		x[i] = scale * x[i];
		y[i] = scale * y[i];
	}
#endif

void estimatephases(void) {
	int k;
	double error;
	if (verboseflag) 
	switch (testflag) {
		case 0: extractphaseinfo(0.0, 0.0, nlaser);
				break;
		case 1: setup_test();
				init_measurements(nlaser);
				break;
		case 2: init_vec(nlaser);
				init_measurements(nlaser);
				break;
		default: fprintf(errout, "ERROR: invalid testflag choice\n");
				 return;
	}
	if (verboseflag) printf("Making random guess at phases\n");
	init_guess(nlaser);				/* random initial guess at phases */
	if (verboseflag) printf("Normalize amplitude of guesses\n");
/*	normalize_guess(nlaser); */
	normalize_amplitude(nlaser);
	normalize_amplitude(nlaser);	
	total = total_power(nlaser);
/*	for (k = 0; k < niter; k++)	iterphase(k, nlaser); */
	for (k = 0; k < niter; k++)	iterate_phase_x(k, nlaser); 
	error = total_error(nlaser, 1);
}

void resetimage(void) {
	int i, j, k;
	for (i = 0; i < h; i++) {
		k = i * w;
		for (j = 0; j < w; j++) {
			cc[k] = ss[k] = 0;
			k++;
		}
	}
}

void place(int i, int j, double co, double so) {
	int k;
	k = i * w + j;
	cc[k] = co; ss[k] = so;
	if (j > 0) {
		cc[k-1] = co; ss[k-1] = so;
	}
	if (j < w-1) {
		cc[k+1] = co; ss[k+1] = so;
	}
	if (i > 0) {
		cc[k-w] = co; ss[k-w] = so;
	}
	if (i < h-1) {
		cc[k+w] = co; ss[k+w] = so;
	}
}

void phase_image (double xo, double yo, int nlaser) {
	int i, j, ii, jj, k;
	double xx, yy, co, so;
	resetimage();
	for (i = 0; i < nlaser; i++) {
		for (j = 0; j < nlaser; j++) {
			xx = xo + (ku[i] - ku[j]);
			yy = yo + (kv[i] - kv[j]);
			if (xx < -0.5) xx += w;
			if (yy < -0.5) yy += h;
			jj = (int) (xx + 0.5);
			ii = (int) (yy + 0.5);
			k = w * ii + jj;
			co = x[i] * x[j] + y[i] * y[j];
			so = x[i] * y[j] - y[i] * x[j];
			place(ii, jj, co, so);			
		}
	}
}

/**********************************************************************************************/

int commandline(int argc, char *argv[], int k) {
	while (k < argc && argv[k][0] == '-') {
		if (strcmp(argv[k], "-v") == 0) verboseflag = ! verboseflag;
		if (strcmp(argv[k], "-t") == 0) traceflag = ! traceflag;
		if (strcmp(argv[k], "-d") == 0) debugflag = ! debugflag;
		if (strcmp(argv[k], "-p") == 0) phaseflag = ! phaseflag;
		k++;
	}
	return k;
}

/* char *imagefile="reference_bkph.bmp"; */
/* char *imagefile="25april98one6ref.bmp"; */
/* char *imagefile="2may98default1.bmp"; */
/* char *imagefile="25april98one6ref.bmp"; */
char *imagefile="18may98one6_r0000.bmp";	/* where to get input (image) */
char *transfile="trans.bmp";				/* where to put output (transform) */
/* char *undumpfile="dump.bin"; */
/* char *dumpfile="dump.bin"; */

int main (int argc, char *argv[]) {
	FILE *input, *output;
	int firstarg=1;
	int k; 

	firstarg = commandline(argc, argv, firstarg);
	if (argc >= firstarg + 1) {	/* specified image file on command line ? */
		imagefile = argv[firstarg];
		firstarg++;
	}

	if ((input = fopen(imagefile, "rb")) == NULL) {
		perror(imagefile);
		exit(1);
	}
	else {
		readhead(input);
		readimage(input);
		fclose (input);
/*		testimage(127.0, 1.0, 0.0); */
/*		testimage(127.0, 0.0, 1.0); */
		if (showhistoflag) {
			makehistogram();
			showhistogram();
		}

		allocatecands();
/*		dotransform(); */
		fasttransform();
		spaceshift(); 
		if (verboseflag) printf("Finished computing transform\n");
	}
	
/*	do transform of magnitude of transform as an experiment */
	if (doubletransform) {
		transformtoimage();
		fasttransform();
		spaceshift();
	}

	checktransform();	/* debugging */
	if (verboseflag) printf("Finished checking transform\n");

/*	add fiducial marks and iterate to improve them */
	if (guessfidflag) guessfiducials(radiusd/2.0, 1.0, NLASERS); 
	if (experimentalfid) {
		nlasers = setexpfiducials(hotspots);
		meanfiducials(nlasers);
	}
	if (verboseflag) printf("Ready to adjust spatial frequency guesses\n");
	fflush(stdout);

	if (adjustfiducials) {
/*		double eps = 1.0; */
/*		double eps = 0.1; */
		double eps = 0.01; 
/*		double eps = 2.0; */
/*		double eps = 0.0001; */
/*		for (k = 0; k < 16; k++) {  */
		for (k = 0; k < 32; k++) { 
/*		for (k = 0; k < 4; k++) { */
			if (verboseflag) printf("Iteration %d\n", k);
			fflush(stdout);
/*			iteratefiducials(radiusd/2.0, 1.0, nlasers); */
			if (iteratefiducials(0.0, 0.0, eps, nlasers) == 0) break;
			eps = eps / sqrt(sqrt(2.0)); 
/*			eps = eps / sqrt(2.0); */
/*			eps = eps / 2.0; */
		}
		showfiducials(nlasers);
	}
	if (verboseflag) printf("Finished adjusting spatial frequencies\n");
	fflush(stdout);
	meanfiducials(nlasers);
	powerinbeams(0.0, 0.0, nlasers);
	if (verboseflag) printf("Finished computing power in beams\n");
	if (guessfidflag || experimentalfid) {
		if (phaseflag == 0 && superimposefiducials) addfiducials(0.0, 0.0, nlasers);
	}

	if (phaseflag == 0 && dohoughtransform) {
		allochough();
		houghtransform(radiusd/2.0);
		checkhough();
		houghtotransform();
	}

	if (phaseflag) {
		if (verboseflag) printf("Ready to try and estimate phases\n");
		estimatephases();
		phase_image(0.0, 0.0, nlaser);			/* create suitable image */
	}

		printf("NOTE: Now generating output image\n");
		if (dohoughtransform) colorflag=0;
		if ((output = fopen(transfile, "wb")) == NULL) {
			perror(transfile);
		}
		else {
			writehead(output, colorflag);
			writebmp(output, colorflag);
			fclose(output);
		}


	freestuff();		/* free memory again ... */
	return 0;
}

/***********************************************************************************/

/* rest is stuff no longer used ... */

#ifdef IGNORED
double *cosih=NULL;
double *sineh=NULL;

double *cosiv=NULL;
double *sinev=NULL;

void trigtables (void) {
	int i, j;

	cosih = (double *) malloc (sizeof(double) * w);
	sineh = (double *) malloc (sizeof(double) * w);
	cosiv = (double *) malloc (sizeof(double) * h);
	sinev = (double *) malloc (sizeof(double) * h);
	for (j = 0; j < w; j++) {
		cosih[j] = cos (TWOPI * j / w);
		sineh[j] = sin (TWOPI * j / w);
	}
	for (i = 0; i < h; i++) {
		cosiv[i] = cos (TWOPI * i / h);
		sinev[i] = sin (TWOPI * i / h);
	}
}

/* double coscomp (double u, double v) { */
double coscomp (int id, int jd) {
	int i, j, k;
	int ii, jj;
/*	double theta; */
	double c;
	double cii, sii;
	double sum = 0.0;

/*	u = TWOPI * jd / w; */
/*	v = TWOPI * id / w; */
	ii = 0;
	for (i = 0; i < h; i++) {
		k = i * w;
/*		ii = (i * id) % h; */
/*		theta = u * i; */
		cii = cosih [ii];
		sii = sineh [ii];
		jj = 0;
		for (j = 0; j < w; j++) {
/*			k = i * w + j; */
/*			theta = u * i + v * j; */
/*			c = cos (theta); */
/*			jj = (j * jd) % w; */
/*			c = cosih [ii] * cosiv [jj] - sineh [ii] * sinev [jj]; */
			c = cii * cosiv [jj] - sii * sinev [jj];
			sum += c * image [k];
			k++;
			jj = (jj + jd) % w;
/*			theta += v; */
		}
		ii = (ii + id) % h;
	}
	return sum / (w * h);
}

/* double sincomp (double u, double v) { */
double sincomp (int id, int jd) {
	int i, j, k;
	int ii, jj;
/*	double theta; */
	double s;
	double cii, sii;
	double sum = 0.0;

/*	u = TWOPI * jd / w; */
/*	v = TWOPI * id / w; */
	ii = 0;
	for (i = 0; i < h; i++) {
		k = i * w;
/*		ii = (i * id) % h; */
/*		theta = u * i; */
		cii = cosih [ii];
		sii = sineh [ii];
		jj = 0; 
		for (j = 0; j < w; j++) {
/*			k = i * w + j; */
/*			theta = u * i + v * j; */
/*			s = sin (theta); */
/*			jj = (j * jd) % w; */
/*			s = sineh [ii] * cosiv [jj] + cosih [ii] * sinev [jj]; */
			s = sii * cosiv [jj] + cii * sinev [jj];
			sum += s * image [k];
			k++;
/*			theta += v; */
			jj = (jj + jd) % w;
		}
		ii = (ii + id) % h;
	}
	return sum / (w * h);
}

void dotransform (void) {
	int i, j, k;
	double u, v;
	double c, s;

	for (i = 0; i < h; i++) {
		k = i * w;
		v = TWOPI * i / h;
		for (j = 0; j < w; j++) {
/*			k = i * w + j; */
			u = TWOPI * j / w;
/*			c = coscomp (u, v);*/
			c = coscomp (i, j);
/*			s = sincomp (u, v); */
			s = sincomp (i, j);
			cc [k] = c;
			ss [k] = s;
			printf("%d\t%d\t%lg\t%lg\t%lg\t%lg\n",  i, j, u, v, c, s);
			k++;
		}
		fflush(stdout);
	}
}
#endif

#ifdef IGNORED
double iteratefiducials (double xo, double yo, double eps, int n, int iter) {
	int i;
	double total, totald;
	for (i = 0; i < n; i++) {
		total = sumfiducials(xo, yo, n);
		ku[i] += eps;
		totald = sumfiducials(xo, yo, n);
		if (total >= totald) {
			ku[i] -= 2.0 * eps;
			totald = sumfiducials(xo, yo, n);
			if (total >= totald) ku[i] += eps;
			else {
				printf("%d %d\t%lg\n", iter, i, total);
				total = totald;
			}
		}
		else {
			printf("%d %d\t%lg\n", iter, i, total);
			total = totald;
		}

		total = sumfiducials(xo, yo, n);
		kv[i] += eps;
		totald = sumfiducials(xo, yo, n);
		if (total >= totald) {
			kv[i] -= 2.0 * eps;
			totald = sumfiducials(xo, yo, n);
			if (total >= totald) kv[i] += eps;
			else {
				printf("%d %d\t%lg\n", iter, i, total);
				total = totald;
			}
		}
		else {
			printf("%d %d\t%lg\n", iter, i, total);
			total = totald;
		}

	}
	total = sumfiducials(xo, yo, n);
	fflush(stdout);
	return total;
}
#endif

#ifdef IGNORED
int setexpfiducials (void) {	/* experimental data */
	int i1, j1, k1;
	int i2, j2, k2;
	double x1, y1;
	double x2, y2;
	for (k1 = 0; k1 < NLASERS+NLASERS; k1++) {
		j1 = hotspots[k1][0];
		i1 = hotspots[k1][1];
		if (k1 > 21) k2 = k1 - 22;
		else k2 = k1 + 22;
		j2 = hotspots[k2][0];
		i2 = hotspots[k2][1];
		if (i1 == 0 && j1 == 0) break;
		i1 = (h-1) - i1; /* ??? */
/*		j = w - j; */ /* ??? */
		if (j1 > w/2) x1 = j1 - w;
		else x1 = j1;
		if (i1 > h/2) y1 = i1 - h;
		else y1 = i1;
		if (j2 > w/2) x2 = j2 - w;
		else x2 = j2;
		if (i2 > h/2) y2 = i2 - h;
		else y2 = i2;
		ku[k1] = (x1-x2)/2.0;	/* average */
		kv[k1] = (y1+y2)/2.0;
	}
	printf("Set up %d hot spots\n", k1);
	return k1;
}
#endif

#ifdef IGNORED	
/*	skip this reading in of transform data now - fast enough to recompute */
if ((input = fopen(undumpfile, "rb")) == NULL) {
	perror(undumpfile);
}
else {
	printf("NOTE: Undumping data from disk\n");
	w = h = 256;
	allocatecands();
	undumptrans(input);
	fclose(input);
	printf("NOTE: Using dumped data from disk\n");
	goto reprocess;
}
#endif

#ifdef IGNORED
/*	skip dumping of transform data now - fast enough to recompute */
if (dumpflag) {
	if ((output = fopen(dumpfile, "wb")) == NULL) {
		perror(dumpfile);
	}
	else {
		dumptrans(output);
		fclose(output);
	}
}
#endif

#ifdef IGNORED
trigtables();
#endif
