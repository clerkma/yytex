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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* N vectors of unknown magnitude and phase */

/* #define N 7 */

#define N 41

double c[N][N];			/* measured dot-products x_i * x_j + y_i * y_j */

double s[N][N];			/* measured cross-products y_i * x_j - x_i * y_j */

double xo[N], yo[N];	/* true values of vectors - used only to compute c & s */

double x[N], y[N];		/* current guesses at vectors */

double xu[N], yu[N];	/* new guesses at vectors constructed in iteration */

int nlaser=N;			/* number of sources */

double minamplitude=0.8;

double maxamplitude=1.2;

/* double noise=0.01; */	/* noise in pairwise phase estimates */
double noise=0.0;

double initerr=1.0; 	/* error in initial guesses */
/* double initerr=0.0; */

int randominit=1;		/* use random initial values */

/* int niter=100; */		/* number of iterations */
int niter=100;

double alpha=1.0;		/* damping underrelaxation */

double beta = 0.5;		/* optimal value 0.5 */

int gaussjordan=0;		/* update as you go if set */

int verboseflag=1;
int traceflag=0;
int testflag=0;

double total;

/******************************************************************************/

double random (double rmin, double rmax) {
	return ((double) rand() / (double) RAND_MAX) * (rmax - rmin) + rmin;
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

void init_measurements (int n) {
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = i; j < n; j++) {
			c[i][j] = xo[i] * xo[j] + yo[i] * yo[j] + random(-noise, noise);
			s[i][j] = xo[i] * yo[j] - yo[i] * xo[j] + random(-noise, noise);
			c[j][i] = c[i][j];
			s[j][i] = -s[i][j];
		}
		s[i][i] = 0.0;
	}
}

double total_error (int n) {
	int i, j;
	double cc, ss, dx, dy;
	double sum=0.0;
	for (i = 0; i < n; i++) {
		for (j = i; j < n; j++) {
			cc = x[i] * x[j] + y[i] * y[j];
			dx = cc - c[i][j];
			ss = x[i] * y[j] - y[i] * x[j];
			dy = ss - s[i][j]; 
			if (traceflag) 	printf("%d\t%d\t(%lg %lg)\t(%lg %lg)\n",
								   i, j, c[i][j], s[i][j], cc, ss);
			sum += dx * dx + dy * dy;
		}
	}
	return sqrt (sum / (double) n);
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
	for (i = 0; i < n; i++) sum += x[i] * x[i] + y[i] * y[i];
	return sum;
}

double dc_power (int n) {
	int k;
	double sum=0.0;
	for (k = 0; k < n; k++)	sum += c[k][k];
	return sum;
}

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

double new_x (int k, int n, double scale) {
	int i;
	double sum=0.0;
	for (i = 0; i < n; i++)	sum += c[k][i] * x[i] + s[k][i] * y[i];
	return sum / scale;
}

double new_y (int k, int n, double scale) {
	int i;
	double sum=0.0;
	for (i = 0; i < n; i++)	sum += c[k][i] * y[i] - s[k][i] * x[i];
	return sum / scale;
}

void iter (int m, int n) {
	int i;
	double terror, oldtotal, dc, scale;

	oldtotal = total;
	total = total_power(n);
/*	total = (oldtotal + total) / 2.0; */
	terror = total_error(n);
	dc = dc_power(n);
	if (verboseflag) printf("%d\ttotal %lg\tdc %lg\terror %lg\n", m, total, dc, terror);
/*	scale = sqrt(dc * total); */
/*	scale = total; */
/*	scale = (dc + total)/2; */
	scale = beta * total + (1 - beta) * dc;
	for (i = 0; i < n; i++) {
		xu[i] = new_x(i, n, scale);
		yu[i] = new_y(i, n, scale);
		if (gaussjordan) {
			x[i] = alpha * xu[i] + (1 - alpha) * x[i];
			y[i] = alpha * yu[i] + (1 - alpha) * y[i];
		}
	}
	if (! gaussjordan) {
		for (i = 0; i < n; i++) {
			x[i] = alpha * xu[i] + (1 - alpha) * x[i];
			y[i] = alpha * yu[i] + (1 - alpha) * y[i];
		}
	}
}

void setup_test(void) {
	nlaser = 3;
	xo[0] = 1.0; yo[0] = 0.0;
	xo[1] = 0.0; yo[1] = 1.0;
	xo[2] = -1.0; yo[2] = -1.0;
}

int main (int argc, char *argv[]) {
	int k;
	if (testflag) setup_test();
	else init_vec(nlaser);
	init_measurements(nlaser);
	init_guess(nlaser);
	normalize_guess(nlaser);
	total = total_power(nlaser);
	for (k = 0; k < niter; k++) iter(k, nlaser);
	return 0;
}
