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

/* Program to generate image of Y&Y logo */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define MAXCOLUMNS 1024

int diffused[MAXCOLUMNS];

int nrows=0, ncolumns=0;		/* set in main routine */

/* double background = 1.0; */

/* double background = 0.0; */			/* black background experiment */

/* double background = -1.0; */			/* white background */

double background = 192.0 / 256.0;			/* grey background */

int backgroundflush=0;				/* to set background to white */

/* double mingrey=0.25, maxgrey=1.0; */

/* double mingrey=0.15, maxgrey=0.9; */	/* used for business cards */

/* double mingrey=0.25, maxgrey=0.95; */	/* used for license agreements */

double mingrey=0.05, maxgrey=0.95;		/* used for install logo */

/* for PostScript version */

int	psfrequency=53;		/* 71 for QMS PS 815 MR */
int psangle=45;
double pseta=0.08;

/* double sx = 1.0, sy = -1.0, sz = 1.0; */ 

double sx = 0.5, sy = -0.5, sz = 1.0;				/* source position */

/* /po -0.5 def /qo -0.5 def */ /* changed to match y&ylogo.eps 93/Sep/30 */

int tophalf;

int verboseflag = 1;

int originalflag = 0;		/* non-zero => use original Floyd/Steinberg */

int addhalftone = 0;		/* straight image if zero */
							/* add half-tone screen if positive */
							/* add random dither if negative */

int errordiffuse = 0;		/* use error diffusion */

int showinhex = 0;			/* show output in hexadecimal */

/* double wavelength = 5.0; */	/* wavelength in pixels of halftone screen */

double wavelength = 5.65685425;	/* wavelength in pixels of halftone screen */

double eta = 0.08;			/* `ellipticity' of halftone dot */

double pi = 3.141592653;

double angle = 45.0;			/* angle of screen */

double theta, costheta, sintheta;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *copyright="Copyright (C) 1991, Y&Y.  All rights reserved.";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double halftone(double x, double y) { /* halftone screen in -1 to +1 range */
	double xr, yr, wx, wy, half;
	if (addhalftone > 0) {
		xr = x * costheta - y * sintheta;
		yr = x * sintheta + y * costheta;
		wx = 2.0 * pi * xr / wavelength;
		wy = 2.0 * pi * yr / wavelength;
		half = 0.5 * ((1.0 + eta) * cos(wx) + (1 - eta) * cos(wy));
		half = 0.5 * half;			/* reduce to -1/2 to +1/2 range */
/*		half = 0.5 * (1.0 + half); */	/* reduce to 0.0 - 1.0 range */
/*		if (half < 0.0 || half > 1.0) {
			fprintf(stderr, "half %lg for x %lg y %lg wx %lg wy %lg\n", 
				half, x, y, wx, wy);
		} */
	}
	else {		/* pseudo-random in -1/2 to +1/2 range */
		half = (double) rand() / (double) RAND_MAX - 0.5;
	}
	return half;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double greylamb(double x, double y, double rs) {
	double z, e;
	z = sqrt(1.0 - rs);
	e = x * sx + y * sy + z * sz;
	if (e < 0.0) return 0.0;
	else return e;
}

double greyinner(double x, double y, double rs) {
	double z, e;
	z = - sqrt(1.0 - rs);
	e = -(x * sx + y * sy + z * sz);
	if (e < 0.0) return 0.0;
	else return e;
}

double bottomadjust(double grey, double x, double y){ 
	double e;
	if (tophalf != 0) {						/* ya > 0  && x > 0 */
		if (x < 0) 
			fprintf(stderr, "tophalf %d x %lg y %lg grey %lg\n",
				tophalf, x, y, grey);
		e = grey * ((x + y)/(-1.41) * (-0.15) + 1.15);  
	}
	else {									/* ya < 0 && x < 0 */
		if (x > 0)
			fprintf(stderr, "tophalf %d x %lg y %lg grey %lg\n",
				tophalf, x, y, grey);
		e = grey * (- (x + y)/1.41);  
	}
	if (e < 0.0 || e > 1.0) {
/*		fprintf(stderr, "tophalf %d x %lg y %lg grey %lg adjusted %lg\n",
			tophalf, x, y, grey, e); */
	}
	if (e < 0.0) e = 0.0;
	return e;
}

double greytop (double x, double y) {
	double rs, xd, rd;

	rs = x * x + y * y;
	if (rs >= 1.0) return background;
	if (y > 0 && x < 0) return greylamb(x, y, rs);
	if (x < 0) {		/* x < 0 */
		xd = x + 0.5;
		rd = xd * xd + y * y;
		if (rd >= 0.25) return background;
		else return greylamb(x, y, rs);	
	}
	else {				/* x > 0 */
		xd = x - 0.5;
		rd = xd * xd + y * y;
		if (rd >= 0.25) {
			if (y < 0) return background;
			else return greylamb(x, y, rs);
		}
		else return bottomadjust(greyinner(x, y, rs), x, y);
	}
}

double greybottom (double x, double y) {
	double rs, xd, rd;

	rs = x * x + y * y;
	if (rs >= 1.0) return background;
	if (y < 0 && x > 0) return greylamb(x, y, rs);
	if (x > 0) {			/* x > 0 */
		xd = x - 0.5;
		rd = xd * xd + y * y;
		if (rd >= 0.25) return background;
		else return greylamb(x, y, rs);	
	}
	else {					/* x < 0 */
		xd = x + 0.5;
		rd = xd * xd + y * y;
		if (rd >= 0.25) {
			if (y > 0) return background;
			else return greylamb(x, y, rs);
		}
		else return bottomadjust(greyinner(x, y, rs), x, y);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void uwriteone(unsigned int n, FILE *output) {
	int c, d;
	if (showinhex == 0) putc(n, output);
	else {
		c = (n >> 4) & 15;
		d = n & 15;
		if (c > 9) c = c + 'A' - 10;	else c = c + '0';
		if (d > 9) d = d + 'A' - 10;	else d = d + '0';
		putc(c, output); putc(d, output);
	}
}

void uwritetwo(unsigned int offset, FILE *output) {
	if (showinhex == 0) {
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;
	}
	else {
		uwriteone((int) (offset & 255), output); offset = offset >> 8;
		uwriteone((int) (offset & 255), output); offset = offset >> 8;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int column=0;			/* current column index */
int leftover=0;			/* `error' in next picture cell in this row */

/*		X	A */
/*	B	C	D */

void floydsteinberg(int grey, FILE *output) {
	int err, erra, errb, errc, errd;

	grey = grey + leftover;
	if (grey > 127) {
		uwriteone(255, output); err = grey - 255; 
	}
	else {
		uwriteone(0, output); err = grey - 0; 
	}
	if (originalflag != 0) {	/* use original floyd-steinberg weights */
		erra = (err * 7) / 16;
		errb = (err * 3) / 16;
		errc = (err * 5) / 16;
/*		errd = err / 16; */
	}
	else {
		erra = (err * 3) / 8;
		errb = err / 8;
		errc = (err * 3) / 8;
/*		errd = err / 8 */
	}
	errd = err - erra - errb - errc;
	leftover = diffused[column+1] + erra;
	if (column > 0) diffused[column-1] = diffused[column-1] + errb;
	diffused[column] = diffused[column] + errc;
	diffused[column+1] = errd;
	
	column++;
	if (column == ncolumns) {
		column = 0;
		leftover = diffused[0];
		diffused[0] = 0;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void writegrey(double x, FILE *output) {
	double xd;
	int c;

	if (x == background && backgroundflush != 0) {
		if (errordiffuse != 0) floydsteinberg(255, output);
		else uwriteone(255, output);			/* background code */
	}
	else {
/* 		if (x < 0.0) x = 0.0; */
/*		else if (x > 1.0) x = 1.0; */
/*		c = mingrey + (int) (x * (maxgrey - mingrey)); */
		if (x == background) xd = x;			/* 93/Dec/31 */
		else xd = mingrey + x * (maxgrey - mingrey);
		c = (int) (xd * 256.0);
		if (c < 0) c = 0;
		else if (c > 255) c = 255;
		if (errordiffuse != 0) floydsteinberg(c, output);
		else uwriteone(c, output);
	}
}

char *positioning = "\
8.5 2 div 72 mul 161 2 div 0.39 mul sub 9.45 72 mul translate\n\
0.39 dup scale\n\
";

char *screenfun = "\
freq angle {1 add 180 mul cos 1 eta add mul exch 2 add 180 mul cos\n\
	 1 eta sub mul add 2 div} bind setscreen % (c) bkph 1989\n\
";

char *imagefun="\
/picstr width string def\n\
width height scale\n\
width height 8\n\
[width 0 0 height neg 0 height]\n\
{currentfile picstr readhexstring pop}\n\
image\n\
";

void writeheader (FILE *output) {
	int k;
	if (showinhex == 0) {
		fprintf(output, "IMG");
		uwriteone(64, output);			/* offset of data */
		uwriteone(16, output);			/* offset of comment */
		uwritetwo(nrows, output);
		uwritetwo(ncolumns, output);
	}
	else {
		fprintf(output, "%% Y&Y LOGO IMAGE\n");
		fprintf(output, "%% %d rows %d columns\n", nrows, ncolumns);
	}
	if (showinhex == 0) {
		uwriteone(8, output);			/* bits per pixel */
		uwriteone(0, output);			/* fake mingrey */
		uwriteone(255, output);			/* fake maxgrey */
		for (k = 12; k < 16; k++) uwriteone(0, output);
	}
	else fprintf(output, "%% ");
	if (strlen(copyright) > (64 - 16) - 1) {
		fprintf(stderr, "Copyright too long\n");
		exit(7);
	}
	fprintf(output, copyright);
	if (showinhex == 0)
		for (k = 16 + strlen(copyright); k < 64; k++) uwriteone(0, output);
	else putc('\n', output);
	if (showinhex == 0) return;
	fprintf(output, "\n%s", positioning);
	fprintf(output, "\n/freq %d def /angle %d def /eta %lg def\n",
		psfrequency, psangle, pseta);
	fprintf(output, "\n%s", screenfun);
	fprintf(output, "\n/width %d def /height %d def\n", ncolumns, nrows);
	fprintf(output, "\n%s", imagefun);
}

void showusage(char *s) {
printf("%s [-{v}{r}{s}{f}{x}] [-n columns] [-l mingrey] [-h maxgrey] [-b background]\n", s);
printf("\
\tv	verbose mode\n\
\tr	add random dither\n\
\ts	add halftone screen\n\
\tf	use Floyd Steinberg\n\
\tx	output in hex, not binary\n\
\tn	number of columns\n\
\tl	low - minimum grey\n\
\th	high -maximum grey\n\
\tb	background\n\
", s);

/* exit(0); */

}

int main (int argc, char *argv[]) {
	FILE *output;
	char outfilename[80]="y&ylogo.img";
	double x, y, rs, ys;
	double grey;
	int c, i, j, k, firstarg = 1;
	char *s;
	
	if (argc > 1) {
/*		while (*argv[firstarg] == '-')  */
		while (firstarg < argc && *argv[firstarg] == '-') {
			s = argv[firstarg];
/*			if (s == NULL) break; */
			while ((c = *s++) != '\0') {
				if (c == 'v') verboseflag = 1;
				else if (c == 'r') addhalftone=-1;	/* random dither */
				else if (c == 's') addhalftone=1;	/* halftone screen */
				else if (c == 'f') errordiffuse=1;	/* Floyd Steinberg */
				else if (c == 'x') showinhex = 1;	/* output in hex */
				else if (c == 'n') {				/* number of columns */
					firstarg++;
					s = argv[firstarg];
					if (sscanf(s, "%d", &ncolumns) < 1) {
						fprintf(stderr, "Don't understand %s\n", s);
					}
					break;
				}
				else if (c == 'l') {				/* low - mingrey */
					firstarg++;
					s = argv[firstarg];
					if (sscanf(s, "%lg", &mingrey) < 1) {
						fprintf(stderr, "Don't understand %s\n", s);
					}
					break;
				}
				else if (c == 'b') {				/* low - mingrey */
					firstarg++;
					s = argv[firstarg];
					if (sscanf(s, "%lg", &background) < 1) {
						fprintf(stderr, "Don't understand %s\n", s);
					}
					break;
				}
				else if (c == 'h') {				/* high - maxgrey */
					firstarg++;
					s = argv[firstarg];
					if (sscanf(s, "%lg", &maxgrey) < 1) {
						fprintf(stderr, "Don't understand %s\n", s);
					}
					break;
				}
			}
			firstarg++;
		}
	}
	else showusage(argv[0]);

	if (addhalftone > 0) {
		if (verboseflag != 0) 
			printf ("Adding halftone screen\n");
		strcpy (outfilename, "y&yhalf.img");
	}
	if (addhalftone < 0) {
		if (verboseflag != 0) 
			printf ("Adding random dither\n");
		strcpy (outfilename, "y&yrand.img");
	}
	if (errordiffuse != 0) {
		if (verboseflag != 0) 
			printf ("Using Floyd Steinberg\n");
		strcpy (outfilename, "y&yfloyd.img");
	}
	if (showinhex != 0) {
		if (verboseflag != 0) 
			printf ("Using hex representation in output\n");
		strcpy (outfilename, "y&ylogo.ps");
	}

/*	sx = 1.0; sy = -1.0; sz = 1.0; */
/*	sx = 0.5; sy = -0.5; sz = 1.0; */
	rs = sqrt(sx * sx + sy * sy + sz * sz);
	sx = sx / rs; sy = sy / rs; sz = sz / rs;

	theta = angle * pi / 180.0;
	costheta = cos(theta); sintheta = sin(theta);

	if (ncolumns == 0) {	/* if number of columns not specified */
/*		ncolumns = 100; */ 
/*		ncolumns = 150; */
		ncolumns = 162;
	}
	nrows = (ncolumns * 3) / 2;
	nrows = (nrows / 2) * 2; 
	ncolumns = (nrows * 2) / 3;

	if (errordiffuse != 0) {
		assert (ncolumns < MAXCOLUMNS);
		for(k = 0; k <= ncolumns; k++) diffused[k] = 0;
		column = 0; leftover = 0;
	}

	if((output = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename);
		exit(3);
	}

	if (verboseflag != 0)
printf("%d rows of %d columns - %lg min grey %lg max grey %lg background\n",
			nrows, ncolumns, mingrey, maxgrey, background);

	writeheader (output);

/*	if (showinhex != 0) putc('\n', output); */
	for (i = 0; i < nrows; i++) {
/*		y = ((double) i - nrows/2) / (ncolumns/2); */
		y = ((double) i - nrows/2.0) / (ncolumns/2.0);
		if (y >= 0.0) {		/* `top' half */
			tophalf = 1;			/* ya > 0 */
			ys = y - 0.5; 
			for (j = 0; j < ncolumns; j++) {
/*				printf("(%d %d) ", j, column); */
				if (errordiffuse != 0) assert(j == column); 
/*				x = ((double) j - ncolumns/2) / (ncolumns/2); */
				x = ((double) j - ncolumns/2.0) / (ncolumns/2.0);
				grey = greytop(-x, ys);
				if (addhalftone != 0) {
					if (grey != background)
						grey = (grey + halftone((double) i, (double) j));
				}
				writegrey(grey, output);
/*				writegrey(greytop(-x, ys), output); */
			}
		}
		else {				/* `bottom' half */
			tophalf = 0;			/* ya < 0 */
			ys = y + 0.5; 
			for (j = 0; j < ncolumns; j++) {
/*				printf("(%d %d) ", j, column); */
				if (errordiffuse != 0) assert(j == column); 
/*				x = ((double) j - ncolumns/2) / (ncolumns/2); */
				x = ((double) j - ncolumns/2.0) / (ncolumns/2.0);
				grey = greybottom(-x, ys);
				if (addhalftone != 0) {
					if (grey != background)
						grey = (grey + halftone((double) i, (double) j));
				}
				writegrey(grey, output);
/*				writegrey(greybottom(-x, ys), output); */
			}
		}
		if (verboseflag != 0) {
			if (i % 64 == 0 && i != 0) putc('\n', stdout);
			putc('.', stdout);
		}
		if (showinhex != 0) putc('\n', output);
	}
	if (showinhex != 0) fprintf(output, "showpage\n");
	if (showinhex != 0) putc(4, output);
	if (ferror(output) != 0) {
		perror(outfilename); exit(1);
	}
	else fclose(output);
	return 0;
}
			
/* straight image if zero           default */
/* add half-tone screen if positive +       */
/* add random dither if negative    -       */
/* use error diffusion if equal		=		*/

/* straight image if zero           default */
/* add half-tone screen if positive +       */	/* s */
/* add random dither if negative    -       */	/* r */
/* use error diffusion if equal		=		*/	/* e */
/* show output in hex instead of binary		*/	/* h */

/* 8.5 2 div 72 mul 161 2 div 0.39 mul sub 9.45 72 mul translate */
/* 0.39 dup scale */
