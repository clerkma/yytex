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
#include <math.h>

int verboseflag=1;
int traceflag=0;
int refineflag=1;                               // refine position of BB image centers
int showcolorflag=0;                    // show color table
int writesmoothflag=1;                  // write smooth image
int colorflag=0;                                // write image in RGB

unsigned char *image=NULL;

unsigned char **imagerow=NULL;

// int *blurr=NULL;
double *blurr=NULL;

// int **blurrrow=NULL;
double **blurrrow=NULL;

int histogram[256];

char *infile="d:\\bmp\\image001.bmp";
char *outfile="d:\\bmp\\image001s.bmp";


/************************************************************************/


// code for least squares fitting image intensifier distortion
// one term in radial distortion and one in tangential distortion


// Image Intensifier CCD is 980 x 980


double xo=490.0;        // image center in x
double yo=490.0;        // image center in y


/*****************************************************************************/


// DATA: estimated centers of BB images in 980 x 980 image in image1.bmp

// PaintShop measures second coordinate from the top row of the image,
// while data starts at the bottom row.

// * setupdata converts this data from integer to double and flips the y

// * setupdatafine tries to fine tune coordinates first
// * --- by smoothing and finding local minima.

int iidata[][2]=
{
{0, 0},
{0, 0},
{0, 0},
{410, 41},
{492, 66},
{572, 88},
{651, 108},
{730, 126},
{0, 0}, // {809, 140},
{0, 0},
{0, 0},
{0, 0},


{0, 0},
{221, 104},
{307, 130},
{389, 154},
{468, 176},
{546, 199},
{622, 220},
{699, 239},
{777, 258},
{856, 274},
{0, 0},
{0, 0},


{0, 0},
{201, 213},
{283, 236},
{363, 259},
{440, 281},
{518, 305},
{593, 325},
{668, 346},
{746, 366},
{823, 384},
{905, 400},
{0, 0},


{90, 299},
{176, 319},
{257, 340},
{335, 361},
{412, 383},
{487, 405},
{562, 428},
{638, 449},
{715, 470},
{793, 488},
{874, 507},
{0, 0}, // {961, 523},


{63, 405},
{148, 423},
{228, 441},
{306, 463},
{382, 484},
{458, 506},
{533, 528},
{608, 550},
{685, 570},
{763, 591},
{844, 609},
{932, 627},


{34, 512},
{120, 527},
{200, 545},
{276, 564},
{352, 585},
{428, 606},
{504, 628},
{580, 650},
{657, 671},
{736, 691},
{818, 712},
{0, 0},


{0, 0},
{90, 635},
{171, 651},
{248, 669},
{325, 688},
{401, 709},
{476, 729},
{553, 750},
{631, 772},
{712, 793},
{796, 815},
{0, 0},


{0, 0},
{0, 0},
{140, 761},
{220, 776},
{298, 794},
{375, 813},
{451, 834},
{529, 855},
{609, 877},
{691, 901       },
{0, 0},
{0, 0},


{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0}, // {271, 910},
{350, 927},
{429, 947},
{0, 0}, // {509, 969},
{0, 0},
{0, 0},
{0, 0},
{0, 0}
};


// int iicoords[9][12][2];
double iicoords[9][12][2];      // BB image center coordinates


/***************************************************************************************/
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


BITMAPFILEHEADER bmfh;          // kept global for easy access


BITMAPINFOHEADER bmih;          // kept global for easy access


RGBQUAD colortable[256];        /* set up when reading BMP */


/******************************************************************************/


void showcolortable (RGBQUAD colortable[], int n) {
        int k;
        for (k = 0; k < n; k++) {
                printf("%d\t%d\t%d\t%d\n",
                           k, colortable[k].rgbRed, colortable[k].rgbGreen, colortable[k].rgbBlue);
        }
}


int readBMPheader (FILE *input) {
/*      BITMAPFILEHEADER bmfh; */
/*      BITMAPINFOHEADER bmih; */
        long nlen;


        fseek(input, 0, SEEK_END);
        nlen = ftell(input);
        if (verboseflag) printf("File length %ld\n", nlen);
        fseek(input, 0, SEEK_SET);
        if (verboseflag) putc('\n', stdout);
/*      read file header */
        fread(&bmfh, sizeof(bmfh), 1, input);
        if (bmfh.bfType != (77 * 256 + 66)) {                   /*  "BM" */
                printf("Not BMP file %X\n", bmfh.bfType);
/*              return -1; */
        }
        else if (verboseflag) printf("BMP file %X\n", bmfh.bfType);
/*      file size in words ? */
        if (verboseflag) printf("File size %lu\n", bmfh.bfSize);
/*      offset from end of header ? */
        if (verboseflag) printf("Offset to image %lu\n", bmfh.bfOffBits);
        offset = bmfh.bfOffBits;
        if (verboseflag) putc('\n', stdout);
/*      read bitmap info header */      
        fread(&bmih, sizeof(bmih), 1, input);
        if (verboseflag) printf("Size of header %lu\n", bmih.biSize);   
        if (verboseflag) printf("Width in pixels %ld\n", bmih.biWidth); 
        width = bmih.biWidth;
        if (verboseflag) printf("Height in pixels %ld\n", bmih.biHeight);
        height = bmih.biHeight;
        if (verboseflag) printf("Number of image planes %u\n", bmih.biPlanes);
        if (verboseflag) printf("Bits per pixels %u\n", bmih.biBitCount);
        bitsperpixel = bmih.biBitCount;
        if (verboseflag) printf("Compression %lu\n", bmih.biCompression);
        if (verboseflag) printf("Size of compressed image %lu\n", bmih.biSizeImage);
        if (verboseflag) printf("Horizontal pixel per meter %ld\n", bmih.biXPelsPerMeter);
        if (verboseflag) printf("Vertical pixel per meter %ld\n", bmih.biYPelsPerMeter);
        if (verboseflag) printf("Number of colors used %lu\n", bmih.biClrUsed);
        if (verboseflag) printf("Number of `important' colors %lu\n", bmih.biClrImportant);
/*      read color table */
        fread(&colortable, sizeof(RGBQUAD), bmih.biClrUsed, input);
        if (showcolorflag) showcolortable(colortable, bmih.biClrUsed);
        if (verboseflag) putc('\n', stdout);
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
//      set up convenient margin array
        for (k = 0; k < height; k++) imagerow[k] = image + k * width;
        return 0;
}


int readBMPimage(FILE *input) {
        int i, j;
        int c=0;
/*      unsigned char color; */
//      int nbytes=1;
        long current;


        width = (int) bmih.biWidth;
        height = (int) bmih.biHeight;
        if (setupimagearrays()) return -1;      // failure


/*      current = sizeof(BITMAPFILEHEADER) + bmfh.bfOffBits;  */
        current = bmfh.bfOffBits;
        printf("SEEK to %ld\n", current);
        fseek(input, current, SEEK_SET); 
        for (i = 0; i < height; i++) {
//              k = i * width;
                for (j = 0; j < width; j++) {
/*                      fread(&color, nbytes, 1, input); */
                        c = getc(input);
                        if (c < 0) {
                                printf("ERROR: EOF i %d j %d\n", i, j);
                                break;
                        }
/*                      image [k] = (double) color; */
//                      image [k] = (double) ((unsigned char) c);
//                      image [k] = (char) c;
//                      k++;
                        imagerow[i][j] = (unsigned char) c;     // ignore color table
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
/*      following doesn't take into account padding */ /*       bmih.biClrUsed = 256; */
        bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        if (colorflag) bmfh.bfSize += 3 * width * height;
        else bmfh.bfSize += width * height + 256 * sizeof(RGBQUAD);
/*      bmfh.bfOffBits = sizeof(BITMAPINFOHEADER); */
/*      bmfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER); */
        bmfh.bfOffBits = roundup(sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER));
        if (colorflag) ;
        else bmfh.bfOffBits += 256 * sizeof(RGBQUAD);
        fwrite(&bmfh, sizeof(bmfh), 1, output);
        bmih.biSize = 40;  /* unchanged */
        bmih.biWidth = width;
        bmih.biHeight = height;
        bmih.biPlanes = 1;
        if (colorflag) bmih.biBitCount = 24;            /* go for 24 bit RGB */
        else bmih.biBitCount = 8;                                       /* go for 8 bit B/W */
        bmih.biCompression = 0;
        if (colorflag) bmih.biSizeImage = 3 * width * height;
        else bmih.biSizeImage = width * height;
        bmih.biXPelsPerMeter = 0;
        bmih.biYPelsPerMeter = 0;
        bmih.biClrUsed = 0;                     /* if non-zero specifies size of color map */
/*      bmih.biClrUsed = 256; */
        bmih.biClrImportant = 0;
        fwrite(&bmih, sizeof(bmih), 1, output);
        if (colorflag) ; /*     no color map for 24 bit data */
        else fwrite(&colortable, sizeof(RGBQUAD), 256, output); /* assume set up */
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
//      int k;
        int ir, ig, ib;
        int rem;


        for (i = 0; i < height; i++) {
//              k = i * width;
                for (j = 0; j < width; j++) {
//                      ir = ig = ir = image[k];
                        ib = ig = ir = imagerow[i][j];
                        if (colorflag) {
                                putc(ib, output); 
                                putc(ig, output); 
                                putc(ir, output);
                        }
                        else putc(ig, output);
//                      k++;
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
/****************************************************************************/

// Uniform and Gaussian random numbers


double random (double rmin, double rmax) {      // uniform between rmin & rmax
        return ((double) rand() / (double) RAND_MAX) * (rmax - rmin) + rmin;
}


#define NGAUSSIAN 6             // enough out to 6 st devs
// #define NGAUSSIAN 3                  // smaller for speed out to 3 st devs


// Generate pseudo-random numbers with Gaussian dsitribution


double gaussian (double stdev) {        // mean zero Gaussian random var
        double sum=0.0;
        int i;
        if (stdev == 0.0) return 0.0;
        for (i = 0; i < NGAUSSIAN; i++) sum += random(-1.0, 1.0);
        return sum * stdev / sqrt(NGAUSSIAN/3);
}


/****************************************************************************/
/*****************************************************************************/


// Simple stuff for 3-vectors and 3 x 3 matrices
// Here matrices are just of the form double mat[3][3]


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
//      first find transpose of cofactors of M
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
//      solution is C b / det
        a[0] = (c[0][0] * b[0] + c[1][0] * b[1] + c[2][0] * b[2])/det;
        a[1] = (c[0][1] * b[0] + c[1][1] * b[1] + c[2][1] * b[2])/det;
        a[2] = (c[0][2] * b[0] + c[1][2] * b[1] + c[2][2] * b[2])/det;
        return det;
}


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

//      create vector, quaternion & matrix structures, and free them again

double *make_vector_n (int n) {                 // vector of length n
        return (double *) malloc(n * sizeof(double));
}

double *make_vector (void) {
        return make_vector_n(3);
}

double *make_quaternion (void) {
        return make_vector_n(4);
}

//      make n x m matrix --- i.e. m column vectors of n elements each
//      NOTE: matrix organized as array of row vectors 
//      NOTE: m[i] is the i-th row ... *not* column
//      NOTE: double m[...][...] creates a *different* structure

double **make_matrix_nm (int n, int m) { // n rows m columns
        int i;
        double **mat;
        mat     = (double **) malloc(n * sizeof(double *));
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


//      Gauss Jordan from Numerical Recipes
//      Here matrices are built using margin arrays


int *make_ivector(int n) {
        return (int *) malloc (n * sizeof(int));
}


void free_ivector(int *v, int n) {
        free(v);
}


//      Matrix to invert in a, rhs to solve in b
//      NOTE: inverse overwrites original vector
//      NOTE: solutions overwrite original rhs


int gaussj(double **a, int n, double **b, int m) {
        int *indxc, *indxr, *ipiv;                              // for pivoting bookkeeping
        int i, icol, irow, j, k, l, ll;
        double big, dum, pivinv, temp;


        indxc = make_ivector(n);
        indxr = make_ivector(n);
        ipiv  = make_ivector(n);                                // whether has been used as pivot


        for (j = 0; j < n; j++) ipiv[j] = 0;    // set to unused
        for (i = 0; i < n; i++) {                               // main loop over columns
                big = 0.0;
                for (j = 0; j < n; j++)                         // outer loop search for pivot
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
                ++(ipiv[icol]);                                         // mark used
//              now have pivot element, so interchange rows if needed
                if (irow !=  icol) {
                        for (l = 0; l < n; l++) SWAP(a[irow][l], a[icol][l])
                                        for (l = 0; l < m; l++) SWAP(b[irow][l], b[icol][l])
                }
                indxr[i] = irow;                                // remember which ones we swapped
                indxc[i] = icol;
//              now divide pivot row by pivot element
                if (a[icol][icol] == 0.0) {
                        printf("ERROR: gaussj: Singular Matrix-2\n");
                        return -1;
                }
                pivinv = 1.0 / a[icol][icol];
                a[icol][icol] = 1.0;
                for (l = 0; l < n; l++) a[icol][l] *=  pivinv;
                for (l = 0; l < m; l++) b[icol][l] *=  pivinv;
//              reduce the rows --- except for the pivot row
                for (ll = 0; ll < n; ll++)
                        if (ll !=  icol) {
                                dum = a[ll][icol];
                                a[ll][icol] = 0.0;
                                for (l = 0; l < n; l++) a[ll][l] -=  a[icol][l] * dum;
                                for (l = 0; l < m; l++) b[ll][l] -=  b[icol][l] * dum;
                        }
        }
//      now need to unscramble column interchanges - in reverse order
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


//      just invert the matrix, no rhs to solve equations for


int gauss_invert (double **a, int n) {
        return gaussj (a, n, NULL, 0);
}


//      solve for just a single rhs vector


int gauss_solve (double **a, int n, double *rhs, double *sol) {
        double **b;
        int k, flag;
        b = make_matrix_nm(n, 1);       // skinny right hand side
        for (k = 0; k < n; k++) b[k][0] = rhs[k];
        flag = gaussj (a, n, b, 1);
        for (k = 0; k < n; k++) sol[k] = b[k][0];
        free_matrix_nm(b, n, 1);
        return flag;
}


/*****************************************************************************/


// * The measured data is organized in rows of BBs
// * because of the circular II outline, different rows
// * have different numbers of entries.
// * If we think of the BBs as having integer coordinates (i,j)
// * then the array entry corresponding to BB (i,j) is iicoords[i][j]

// * Note that j corresponds to the x direction
// * Note that i corresponds to the y direction (inverted in PaintShop Pro)

// Create matrix of coordinates from list of measured data 

// * setupdata converts from integer to double and flips the y
// * (PaintShop Pro counts rows from the top down
// * and the values in iidata are manually obtained from PantShopPro

int setupdata (void) {
        int i, j, k=0;
        height = width = 980;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        iicoords[i][j][0] = iidata[k][0];               // x (j)
                        if (iidata[k][1] == 0) iicoords[i][j][1] = 0;
                        else iicoords[i][j][1] = height -1 - iidata[k][1];              // y (i)
                        k++;
                }
        }
        return 0;
}

/*****************************************************************************************/

// * Note that j corresponds to the x direction
// * Note that i corresponds to the y direction (inverted in PaintShop Pro)

// * Find the average increment in x when j is increment
// * i.e. linear scale factor relating BB integer coordinate j and
// * image coordinate x.

// * Next for function are subsumed by affine fitting below

double averagexfromj (void) {
        int i, j;
        double sum=0.0;
        int count=0;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12-1; j++) {
                        if (iicoords[i][j+1][0] > 0 && iicoords[i][j][0] > 0) {
                                sum += (iicoords[i][j+1][0] - iicoords[i][j][0]);
                                count++;
                        }
                }
        }
        return (sum / count);
}

// * Find the average increment in y when j is increment
// * i.e. linear scale factor relating BB integer coordinate j and
// * image coordinate y.

double averageyfromj (void) {
        int i, j;
        double sum=0.0;
        int count=0;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12-1; j++) {
                        if (iicoords[i][j+1][1] > 0 && iicoords[i][j][1] > 0) {
                                sum += (iicoords[i][j+1][1] - iicoords[i][j][1]);
                                count++;
                        }
                }
        }
        return (sum / count);
}

// * Find the average increment in x when i is increment
// * i.e. linear scale factor relating BB integer coordinate i and
// * image coordinate x.

double averagexfromi (void) {
        int i, j;
        double sum=0.0;
        int count=0;
        for (i = 0; i < 9-1; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i+1][j][0] > 0 && iicoords[i][j][0] > 0) {
                                sum += (iicoords[i+1][j][0] - iicoords[i][j][0]);
                                count++;
                        }
                }
        }
        return (sum / count);
}

// * Find the average increment in y when i is increment
// * i.e. linear scale factor relating BB integer coordinate i and
// * image coordinate y.

double averageyfromi (void) {
        int i, j;
        double sum=0.0;
        int count=0;
        for (i = 0; i < 9-1; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i+1][j][1] > 0 && iicoords[i][j][1] > 0) {
                                sum += (iicoords[i+1][j][1] - iicoords[i][j][1]);
                                count++;
                        }
                }
        }
        return (sum / count);
}

// * The four scale factors estimated above form the 2 x 2 core of the affine
// * transform:
// *    x = a11 * i + a12 * j + a13
// *	y = a21 * i + a22 * j + a23

/**********************************************************************************/

// Set up matrix for least squares solution of affine transformation

// * This assumes the BB coordinates are integers based on the row and
// * column index of the entry in iicoords[][].  "Missing" BB images
// * (because of circular II cut off) are denoted by zeros.

// * If BB locations are not on a regular grid, need to replace i and j
// * in the accumulation of m[][] with the actual x and y coordinates.

void make_affine_matrix(double m[3][3]) {
        int i, j;
        for (i = 0; i < 3; i++) {
                for (j = 0; j < 3; j++) {
                        m[i][j] = 0.0;
                }
        }
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i][j][0] > 0) {
                                m[0][0] += i * i;
                                m[0][1] += i * j;
                                m[0][2] += i;
                                m[1][0] += j * i;
                                m[1][1] += j * j;
                                m[1][2] += j;
                                m[2][0] += i;
                                m[2][1] += j;
                                m[2][2] += 1;
                        }
                }
        }
}

// solve for first row of affine transformation (x part)

// * This sets up the matrix M and the right hand side vector
// * for the least squares problem of solving for the coefficients in
// * x = a11 * i + a12 * j + a13
// * the vector b is the right hand side
// * the vector c is the solution

// * If BB locations are not on a regular grid, need to replace i and j
// * in the accumulation of b[] with the actual x and y coordinates.

void affinefitx (double c[3]) {
        double m[3][3];
        double b[3];
        double x;
        int i, j, count;

        count = 0;
        make_affine_matrix(m);
        for (i = 0; i < 3; i++) b[i] = 0.0;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i][j][0] > 0) {
                                x = iicoords[i][j][0];
                                b[0] += x * i;
                                b[1] += x * j;
                                b[2] += x;
                                count++;
                        }
                }
        }


        solve33(m, b, c);


        printf("MATRIX:\n");
        show_matrix33(m);
        printf("VECTOR B: ");
        show_vector3(b);
        printf("VECTOR C: ");
        show_vector3(c);
}

// solve for second row of affine transformation (y part)

// * This sets up the matrix M and the right hand side vector
// * for the least squares problem of solving for the coefficients in
// * y = a21 * i + a22 * j + a23
// * the vector b is the right hand side
// * the vector c is the solution

// * If BB locations are not on a regular grid, need to replace i and j
// * in the accumulation of b[] with the actual x and y coordinates.

void affinefity (double c[3]) {
        double m[3][3];
        double b[3];
        double y;
        int i, j, count;


        count = 0;
        make_affine_matrix(m);
        for (i = 0; i < 3; i++) b[i] = 0.0;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i][j][0] > 0) {
                                y = iicoords[i][j][1];
                                b[0] += y * i;
                                b[1] += y * j;
                                b[2] += y;
                                count++;
                        }
                }
        }


        solve33(m, b, c);


        printf("MATRIX:\n");
        show_matrix33(m);
        printf("VECTOR B: ");
        show_vector3(b);
        printf("VECTOR C: ");
        show_vector3(c);
}

/*************************************************************************************/

// Compute error in affine fit - decompose into radial and tangential

// * Now that we have a least squares linear affine fit, compute the
// * residues.  That is, the part not predicted by the linear formula
// * Decompose the residues into radial and tangential components.

void checkaffinefit (double cx[3], double cy[3]) {
        int i,j;
        double x, y, xd, yd, dx, dy, dv, r, dr, dt;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i][j][0] > 0) {
//                              observed coordinates
                                x = iicoords[i][j][0] - xo;
                                y = iicoords[i][j][1] - yo;
                                r = sqrt(x * x + y * y);
//                              computed coordinates using affine transform
                                xd = cx[0] * i + cx[1] * j + cx[2] - xo;
                                yd = cy[0] * i + cy[1] * j + cy[2] - yo;
//                              error
                                dx = x - xd;
                                dy = y - yd;
                                dv = sqrt(dx * dx + dy * dy);
//                              radial and tangential components of error
                                dr = (dx * x + dy * y) / r;     
                                dt = (dy * x - dx * y) / r;
//                              printf("%2d %2d\t x %4lg y %4lg\t xd %7lg yd %7lg\t dx %8lg dy %8lg\t dr %8lg\n",
//                                         i, j, x, y, xd, yd, dx, dy, dr);
//                              printf("%2d %2d x %4lg y %4lg r %8lg xd %8lg yd %8lg dx %9lg dy %9lg dv %8lg\n",
//                              i, j, x, y, r, xd, yd, dx, dy, dv);
                                printf("%2d %2d x %4lg y %4lg r %8lg xd %8lg yd %8lg dr %9lg dt %9lg dv %8lg\n",
                                           i, j, x, y, r, xd, yd, dr, dt, dv);
                        }
                }
                printf("\n");
        }
}

/*************************************************************************************/

// set up matrix for least squares solution of radial and tangential distortion

// * This assumes the BB coordinates are integers based on the row and
// * column index of the entry in iicoords[][].  "Missing" BB images
// * (because of circular II cut off) are denoted by zeros.

void make_distort_matrix (double m[3][3]) {
        int i, j;
        double x, y, r;
        for (i = 0; i < 3; i++) {
                for (j = 0; j < 3; j++) m[i][j] = 0.0;
        }
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i][j][0] > 0) {
                                x = iicoords[i][j][0] - xo;
                                y = iicoords[i][j][1] - yo;
                                r = sqrt(x * x + y * y);
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
                }
        }
}


// Get best second order fit in radial direction

// * Sets up the matrix M and the right hand side vector b
// * Solution in vector c.

void radialfit (double cx[3], double cy[3], double c[3]) {
        int i,j;
        double x, y, xd, yd, dx, dy, dv, r, dr, dt;
        double m[3][3], b[3];


        make_distort_matrix(m);
        for (i = 0; i < 3; i++) b[i] = 0.0;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i][j][0] > 0) {
                                x = iicoords[i][j][0] - xo;
                                y = iicoords[i][j][1] - yo;
                                r = sqrt(x * x + y * y);
                                xd = cx[0] * i + cx[1] * j + cx[2] - xo;
                                yd = cy[0] * i + cy[1] * j + cy[2] - yo;
                                dx = x-xd;
                                dy = y-yd;
                                dv = sqrt(dx * dx + dy * dy);
//	radial component of error
                                dr = (dx * x + dy * y) / r;
//	tangential component of error (not used here)
                                dt = (dy * x - dx * y) / r;
                                b[0] += dr * r * r;
                                b[1] += dr * r;
                                b[2] += dr;
                        }
                }
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
/*                         rcrs, maxerr4, */
                           rmax, maxerr2);
        }
}


// Get best second order fit in tangential direction

// * Sets up the matrix M and the right hand side vector b
// * Solution in vector c.

void tangentialfit (double cx[3], double cy[3], double c[3]) {
        int i,j;
        double x, y, xd, yd, dx, dy, dv, r, dr, dt;
        double m[3][3], b[3];


        make_distort_matrix(m);
        for (i = 0; i < 3; i++) b[i] = 0.0;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i][j][0] > 0) {
                                x = iicoords[i][j][0] - xo;
                                y = iicoords[i][j][1] - yo;
                                r = sqrt(x * x + y * y);
                                xd = cx[0] * i + cx[1] * j + cx[2] - xo;
                                yd = cy[0] * i + cy[1] * j + cy[2] - yo;
                                dx = x-xd;
                                dy = y-yd;
                                dv = sqrt(dx * dx + dy * dy);
//	radial component of error (not used here)
                                dr = (dx * x + dy * y) / r;
//	tangential component of error
                                dt = (dy * x - dx * y) / r;
                                b[0] += dt * r * r;
                                b[1] += dt * r;
                                b[2] += dt;
                        }
                }
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
/*                         rcrs, maxerr4, */
                           rmax, maxerr2);
        }
}


// check error remaining after radial and tangential fit

// * Uses vector of three coefficients for radial direction second order fit.
// * Uses vector of three coefficients for tangential direction second order fit.

void checkerror(double cx[3], double cy[3], double cr[3], double ct[3]) {
        int i,j, count=0;
        double x, y, xd, yd, dx, dy, dv, r, dr, dt;
        double drd, dtd;
        double sum=0.0;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        if (iicoords[i][j][0] > 0) {
                                x = iicoords[i][j][0] - xo;
                                y = iicoords[i][j][1] - yo;
                                r = sqrt(x * x + y * y);
                                xd = cx[0] * i + cx[1] * j + cx[2] - xo;
                                yd = cy[0] * i + cy[1] * j + cy[2] - yo;
                                dx = x-xd;
                                dy = y-yd;
                                dv = sqrt(dx * dx + dy * dy);
                                dr = (dx * x + dy * y) / r;
                                dt = (dy * x - dx * y) / r;
                                drd = cr[0] * r * r + cr[1] * r + cr[2];
                                dtd = ct[0] * r * r + ct[1] * r + ct[2];
                                sum += (drd - dr) * (drd -dr) + (dtd - dt) * (dtd - dt);
                                count++;
                        }
                }
        }
        printf("st. dev. %lg for %d BBs\n", sqrt(sum/count), count);
}


/***********************************************************************************/


// * read image in BMP format

void testread (char *infile) {
        FILE *input;
        input = fopen(infile, "rb");
        readBMPheader(input);
        readBMPimage(input);
        fclose(input);
}

// * write image in BMP format

void testwrite (char *outfile) {
        FILE *output;
        output = fopen(outfile, "wb");
        writeBMPheader(output);
        writeBMPimage(output);
        fclose(output);
}

/*********************************************************************************/

// * Set up array for smoothed image

int setupblurrarray (void) {
        int npixels = width * height;
        int k,nlen;
        
        if (npixels == 0) return -1;
        if (blurr != NULL) free(blurr);
//      nlen = sizeof(int) * npixels
        nlen = sizeof(double) * npixels;
//      printf("pixels: %d array: %d\n", npixels, nlen);
//      blurr = (int *) malloc(nlen);
        blurr = (double *) malloc(nlen);
        if (blurr == NULL) {
                fprintf(stderr, "ERROR: Unable to allocate %d pixel blurr\n", npixels);
                exit(1);
        }
//      nlen = sizeof(int *) * height;
        nlen = sizeof(double *) * height;
//      printf("pixels: %d array: %d\n", npixels, nlen);
        if (blurrrow != NULL) free(blurrrow);
//      blurrrow = (int **) malloc(nlen);
        blurrrow = (double **) malloc(nlen);
        if (blurrrow == NULL) {
                fprintf(stderr, "ERROR: Unable to allocate %d margin array\n", height);
                exit(1);
        }
//      set up convenient margin array
        for (k = 0; k < height; k++) blurrrow[k] = blurr + k * width;
        return 0;
}

// * Copy original image into place where smoothing will happen

void copyimagein (void) {
        int i, j;
        for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++)     {
                        blurrrow[i][j] = imagerow[i][j];
                }
        }
}

// * Copy blurred image bacj out after smoothing

void copyimageout (void) {
        int i, j;
        for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++)     {
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


//      if (n > 6) printf("Danger of overflow in result (n > 6)\n");


        for (iter = 0; iter < n; iter++) {
//              Apply horizontal filter (1 2 1)
                for (i = 0; i < height; i++) {
                        for (j = 0; j < width-1; j++) 
                                blurrrow[i][j] = (blurrrow[i][j] + blurrrow[i][j+1]) * 0.5;
//                      blurrrow[i][width-1] = blurrrow[i][width-1] + blurrrow[i][width-1];
                        for (j = width-1; j > 0; j--) 
                                blurrrow[i][j] = (blurrrow[i][j] + blurrrow[i][j-1]) * 0.5;
//                      blurrrow[i][0] = blurrrow[i][0] + blurrrow[i][0];
                }
//              Apply vertical filter (1 2 1)
                for (j = 0; j < width; j++) {
                        for (i = 0; i < height-1; i++) 
                                blurrrow[i][j] = (blurrrow[i][j] + blurrrow[i+1][j]) * 0.5;
//                      blurrrow[height-1][j] = blurrrow[height-1][j] + blurrrow[height-1][j];                  
                        for (i = height-1; i > 0; i--) 
                                blurrrow[i][j] = (blurrrow[i][j] + blurrrow[i-1][j]) * 0.5;
//                      blurrrow[0][j] = blurrrow[0][j] + blurrrow[0][j];
                }
        }
        return 0;
}


// Is (i, j) a point of local minimum ?

int checklocalmin (int i, int j) {
//      int center = blurrrow[i][j];
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
        if (blurrrow[i+1][j+1] < center ||
                  blurrrow[i+1][j-1] < center ||
                  blurrrow[i-1][j+1] < center ||
                  blurrrow[i-1][j-1] < center) return 0;
        return -1;                      // yes, it is a local minimum
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
        *x = detx / det;
        *y = dety / det;
        return 0;                                               // success
}


// Fit second order surface
// z = a + b * x + c * y + d * x * x + 2 * e * x * y + f * y * y
// to 3 x 3 neighborhood


int fitsecondorder (int i, int j, double *a, double *b, double *c, double *d, double *e, double *f) {
        double sum=0, sumx=0, sumy=0, sumxx=0, sumxy=0, sumyy=0;
        double sumxxx=0, sumxxy=0, sumxyy=0, sumyyy=0;
        double sumxxxx=0, sumxxxy=0, sumxxyy=0, sumxyyy=0, sumyyyy=0;
        double sumz=0, sumzx=0, sumzy=0, sumzxx=0, sumzxy=0, sumzyy=0;
        double x, y, z;
        int k, ik, jk, ret, es=1;
        double *m[6];
//      double m[6][6];
        double mat[6 * 6];
        double r[6], s[6];
        for (k = 0; k < 6; k++) m[k] = mat + 6 * k;     // margin array
        if (i <= 0 || i >= height-1 ||
                  j <= 0 || j >= width-1) return -1;    // failure
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
//      ret = solve66(m, r, s);
        ret = gauss_solve (m, 6, r, s);
        if (ret < 0) return ret;        // failure
        *a = s[0];
        *b = s[1];
        *c = s[2];
        *d = s[3];
        *e = s[4]/2;
        *f = s[5];
        return 0;                       // success
}


void showneighbourhood(int i, int j) {
        int ik,jk, es=3;

        for (ik = i-es; ik <= i+es; ik++) {
                for (jk = j-es; jk <= j+es; jk++) {
                        printf("%9lg ", blurrrow[ik][jk]);
                }
                printf("\n");
        }
}


// Find local minimum in blurred array near (i, j)
// returns result in last two arguments


int findlocalmin (int i, int j, double *x, double *y) {
        int ik, jk, ret, found=0;
        double a, b, c, d, e, f;
        double xd, yd;
        if (i <= 0 || i >= height-1 ||
                  j <= 0 || j >= width-1) {
                printf("findlocalmin error: %d %d\n", i, j);
                return 0;
        }
        ik = i;
        jk= j;
        if (checklocalmin(ik, jk)) found = 1;
        if (found == 0) {
                for (ik = i-1; ik <= i+1; ik++) {
                        for (jk = j-1; jk <= j+1; jk++) {
                                if (ik == i && jk == j) continue;
                                if (checklocalmin(ik, jk)) {
                                        found = 1;
                                        break;
                                }
                        }
                }
        }
        if (found == 0) {
                for (ik = i-2; ik <= i+2; ik++) {
                        for (jk = j-2; jk <= j+2; jk++) {
                                if ((-1 < ik-i) && (ik-i < 1) && (-1 < jk-j) && (jk-j < 1)) continue;
                                if (checklocalmin(ik, jk)) {
                                        found = 1;
                                        break;
                                }
                        }
                }
        }
        if (found == 0) {
//              printf("No local minimum at or near (%d %d)\n", i, j);
                printf("No local minimum at or near (%d %d)\n", j, (height-i));
                showneighbourhood(i,j);
                *x = j;
                *y = i;
                return -1;
        }
        ret = fitsecondorder(ik, jk, &a, &b, &c, &d, &e, &f);
        if (ret < 0) return ret;        // failure
        ret = secondorderextremum(b, c, d, e, f, &xd, &yd);
        if (ret < 0) return ret;        // failure
        *x = jk + xd;
        *y = ik + yd;
        return 0;                                       // success
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
//      a = 1000.0;
        
        for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++) {
                        x = j;
                        y = i;
                        blurrrow[i][j] =
                                        a + b * x + c * y + d * x * x + 2 * e * x * y + f * y * y;
                }
        }
        if (findlocalmin(ik, jk, &x, &y)) return -1;    // failure
        printf("xo %lg yo %lg x %lg y %lg\n", xo, yo, x, y);
        return 0;
}


// m[i][j] => j corresponds to x, positive to the right
// m[i][j] => i corresponds to y, positive down


// create matrix of coordinates from list of measured data 

// * This differs from setup(void) above in that it refines the
// * manually obtained coordinates using PaintShop Pro.

int setupdatafine (void) {
        int i, j, ik, jk, k=0, count=0, bad=0;
        double x, y;


        if (blurrrow == NULL) {
                printf("Need to set up blurred data array first\n");
                return -1;
        }
        height = width = 980;
        for (i = 0; i < 9; i++) {
                for (j = 0; j < 12; j++) {
                        jk = iidata[k][0];              // x (j)
                        if (iidata[k][1] == 0) ik = 0;
                        else ik = height - 1 - iidata[k][1];            // y (i)
                        if (ik != 0 || jk != 0) {
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
                        }
                        else x = y = 0;
                        iicoords[i][j][0] = y;  // ik;
                        iicoords[i][j][1] = x;  // jk;
                        k++;
                }
        }
        if (bad > 0) printf("%d out of %d BBs not located\n", bad, count);
        return 0;
}

/***********************************************************************************/


// Attempt at fitting radial and tangential distortion


void testfit (void) {
        double dxj, dyj, dxi, dyi, dot, dj, di;
        double cx[3], cy[3];
        double cr[3], ct[3];
        double zero[3] = {0.0, 0.0, 0.0};


        dxj = averagexfromj();
        dyj = averageyfromj();
        dxi = averagexfromi();
        dyi = averageyfromi();
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


int main(int argc, char *argv[]) {
        testread(infile);
//      makehistogram();
//      showhistogram();
//      height = width = 980;
//      testminimumfind (320.12, 230.45);
        if (setupblurrarray()) exit(1);
        copyimagein();
//      blurrimage(6);
//      blurrimage(10);
        blurrimage(20);
        if (writesmoothflag) {
                copyimageout();
                testwrite(outfile);
        }
        if (refineflag) setupdatafine();
        else setupdata();
        testfit();
}


/***********************************************************************************/
