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

#define NSOURCES 3

#define NITER 12

double xs[NSOURCES], ys[NSOURCES];

double dots[NSOURCES][NSOURCES];

double veca[NSOURCES], vecb[NSOURCES];

double xt[NSOURCES], yt[NSOURCES];

int verboseflag=1;
int normalize=1;

void showvecs (double xs[], double ys[], int n) {
	int k;
	for (k=0 ; k < n; k++) printf("%d\t%lg\t%lg\n", k, xs[k], ys[k]);
}

void showdots (int n) {
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) printf("%d\t%d\t%lg\n", i, j, dots[i][j]);
	}
}

void matrix_times_vector (double vecb[], double veca[], int n) {
	int i, j;
	double sum;
	for (i = 0; i < n; i++) {
		sum = 0.0;
		for (j = 0; j < n; j++) {
			sum += dots[i][j] * veca[j];
		}
		vecb[i] = sum;
	}
}

double dot_product(double a[], double b[], int n) {
	double sum = 0.0;
	int k;
	for (k = 0; k < n; k++) sum += a[k] * b[k];	
	return sum;
}

double size_vec (double vec[], int n) {
	return sqrt(dot_product(vec, vec, n));
}

void random_vec (double vec[], int n) {
	int k;
	for (k = 0; k < n; k++) vec[k] = (double) (rand() - RAND_MAX / 2) / 32000.0;
}

void add_vec (double c[], double a[], double b[], int n) {
	int k;
	for (k = 0; k < n; k++) c[k] = a[k] + b[k];
}

void sub_vec (double c[], double a[], double b[], int n) {
	int k;
	for (k = 0; k < n; k++) c[k] = a[k] - b[k];
}

void sub_scale_vec (double c[], double a[], double b[], double scale, int n) {
	int k;
	for (k = 0; k < n; k++) c[k] = a[k] - b[k] * scale;
}

void scale_vec (double vec[], double scale, int n) {
	int k;
	for (k = 0; k < n; k++) vec[k] *= scale;
}

void show_vec (double vec[], int n) {
	int k;
	for (k = 0; k < n; k++) printf("%lg ", vec[k]);
	printf("\n");
}

double normalize_vec (double vec[], int n) {
	double size;
	size = size_vec(vec, n);
	scale_vec(vec, 1.0 / size, n);
	return size;
}

void copy_vec (double vecb[], double veca[], int n) {
	int k;
	for (k = 0; k < n ; k++) vecb[k] = veca[k];
}

double itermul (int m, int n) {
	int i;
	double scale;
	random_vec (veca, n);
	for (i = 0; i < m; i++) {
		normalize_vec(veca, n);
		matrix_times_vector (vecb, veca, n);
		scale = size_vec(vecb, n);
		copy_vec(veca, vecb, n);
		if (verboseflag) printf("%d\t%lg\n", i, scale);
	}
	return scale;
}

void remove_component(double vec[], double eigen[], int n) {
	double dot, dote, scale;
	dot = dot_product(vec, eigen, n);
	dote = dot_product(eigen, eigen, n);
	scale = dot / dote;
	sub_scale_vec (vec, vec, eigen, scale, n);	
}

double itermulx (int m, double eigen[], int n) {
	int i;
	double scale;
	random_vec (veca, n);
	for (i = 0; i < m; i++) {
		normalize_vec (veca, n);
		remove_component (veca, eigen, n);
		matrix_times_vector (vecb, veca, n);
		scale = size_vec(vecb, n);
		copy_vec(veca, vecb, n);
		if (verboseflag) printf("%d\t%lg\n", i, scale);
	}
	return scale;
}

void removeeigen (double lambda, int n) {
	int k;
	for (k = 0; k < n; k++) {
		dots[k][k] -= lambda;
	}
}

void random_directions(double xs[], double ys[], int n) {
	int k;
	double rs;
	for (k=0 ; k < n; k++) {
		xs[k] = (double) (rand() - RAND_MAX / 2);
		ys[k] = (double) (rand() - RAND_MAX / 2);
		if (normalize) {
			rs = sqrt (xs[k] * xs[k] + ys[k] * ys[k]);
			xs[k] = xs[k] / rs;
			ys[k] = ys[k] / rs;
		}
		else {
			xs[k] = xs[k] / 32000.0;
			ys[k] = ys[k] / 32000.0;
		}
	}
}

void make_dots (int n) {
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			dots[i][j] = xs[i] * xs[j] + ys[i] * ys[j];
		}
	}
}

int main(int argc, char *argv[]) {
	int n;
	double lambda1, lambda2;

	n = NSOURCES;
	
	random_directions(xs, ys, n);
	if (verboseflag) showvecs(xs, ys, n);
	
	make_dots(n);
	if (verboseflag) showdots(n);

	lambda1 = itermul (NITER, n);
	copy_vec(xt, veca, n);
	show_vec(veca, n);

/*	lambda2 = itermulx (NITER, lambda1, n); */
	lambda2 = itermulx (NITER, xt, n);
	copy_vec(yt, veca, n);
	show_vec(veca, n);

	copy_vec(xs, xt, n);
	copy_vec(ys, yt, n);
	make_dots(n);
	if (verboseflag) showdots(n);	

	return 0;

}
