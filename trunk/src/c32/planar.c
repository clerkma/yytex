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

/************** planar.c  ***************************************/

/************** 2-d to 2-d photogrammetric problem **************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PI 3.141592653

#define PHI 1.618033989

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

////////////////////////////////////////////////////////////////////////

int verboseflag=1;
int traceflag=0;
int debugflag=0;

int forceoverdeter=0;	// use LSQ / overdetermined even in minimal case
int forceequilateral=0;	// make equilateral triangle of first 3 points
int asymmetryflag=1;	// make array of points non-square

////////////////////////////////////////////////////////////////////////

// True transformation model:

// x'_i =  c' x_i + s' y_i + xo
// y'_i = -s' x_i + c' y_i + yo

// Hence when taking differences:

// dx'_i =  c' dx_i + s' dy_i
// dy'_i = -s' dx_i + c' dy_i

double param[4]={		// actual parameters
	1.0, 1.0, 1.0, 1.0	// cd, sd, xo, yo
};

double theta, scl, xo, yo;

double paramd[4];		// recovered 4-parameters

double paramh[6];		// recovered 6-parameters

int npoints=9;

double points[][2]={
	{1.0, 0.0},
	{0.0, 1.0},
	{0.0, 0.0},		// modified if forceequilateral true
	{1.0, 1.0},
	{0.5, 0.5},
	{0.5, 0.0},
	{1.0, 0.5},
	{0.5, 1.0},
	{0.0, 0.5}	
};

double pointsd[9][2];	// transformed points

// x_k = points[k][0], y_k = points[k][1]

double noise=0.01;		// image measurement noise st dev

int nrepeat=1000;		// how many random tests to run

////////////////////////////////////////////////////////////////////////

//	general utility functions

double deg_from_rad (double theta) {
	return theta * 180.0 / PI;
}

double rad_from_deg (double angle) {
	return angle * PI / 180.0;
}

double random (double rmin, double rmax) {	// uniform between rmin & rmax
	return ((double) rand() / (double) RAND_MAX) * (rmax - rmin) + rmin;
}

#define NGAUSSIAN 27

double gaussian (double stdev) {	// mean zero Gaussian random var
	double sum=0.0;
	int i;
	if (stdev == 0.0) return 0.0;
	for (i = 0; i < NGAUSSIAN; i++) sum += random(-1.0, 1.0);
	return sum * stdev / sqrt(NGAUSSIAN/3);
}

/*******************************************************************/

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

//	make n x m matrix --- i.e. m column vectors of n elements each
//	NOTE: matrix organized as array of row vectors 
//	NOTE: m[i] is the i-th row ... *not* column
//	NOTE: double m[...][...] creates a *different* structure

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

//	common vector operations

double dotproduct_n (double *a, double *b, int n) {
	double sum=0.0;
	int i;
	if (a == NULL || b == NULL) return 0.0;
	for (i = 0; i < n; i++) sum += a[i] * b[i];
	return sum;
}

double dotproduct (double *a, double *b) {
	return dotproduct_n(a, b, 3);
}

double vector_magnitude_n (double *v, int n) {
	if (v == NULL) return 0.0;
	return sqrt(dotproduct_n(v, v, n));
}

double vector_magnitude (double *v) {
	return sqrt(dotproduct(v, v));
}

//	cosine of angle between two vectors

double cosine_between(double *v1, double *v2) {
	double mv1, mv2;
	mv1 = dotproduct(v1, v1);
	mv2 = dotproduct(v2, v2);
	if (mv1 == 0.0 || mv2 == 0.0) {
		printf("ERROR: cosine between\n");
		return 0.0;
	}
	return dotproduct(v1, v2) / sqrt(mv1 * mv2);
}

void vector_scale_n (double *v, int n, double scl, double *vd) {
	int i;
	if (v == NULL || vd == NULL) return;
	for (i = 0; i < n; i++) vd[i] = v[i] * scl;
}

void vector_scale (double *v, double scl, double *vd) {
	vector_scale_n(v, 3, scl, vd);
}

int vector_normalize_n (double *v, int n, double *vn) {	
	double mag = vector_magnitude(v);
	if (v == NULL || vn == NULL) return -1;
	if (mag == 0.0) {
		printf("ERROR: zero size vector\n");
		return -1;
	}
	vector_scale_n(v, n, 1/mag, vn);
	return 0;
}

int vector_normalize (double *v, double *vn) {
	return vector_normalize_n(v, 3, vn);
}

void vector_sum_n (double *a, double *b, int n, double *c) {
	int i;
	if (a == NULL || b == NULL || c == NULL) return;
	for (i = 0; i < n; i++) c[i] = a[i] + b[i];
}

void vector_sum (double *a, double *b, double *c) {
	vector_sum_n(a, b, 3, c);
}

void vector_difference_n (double *a, double *b, int n, double *c) {
	int i;
	if (a == NULL || b == NULL || c == NULL) return;
	for (i = 0; i < n; i++) c[i] = a[i] - b[i];
}

void vector_difference (double *a, double *b, double *c) {
	vector_difference_n(a, b, 3, c);
}

void vector_plus_scaled_vector_n (double *a, double *b, int n, double scl, double *c) {
	int i;
	if (a == NULL || b == NULL || c == NULL) return;
	for (i = 0; i < n; i++) c[i] = a[i] + scl * b[i];
}

void vector_plus_scaled_vector (double *a, double *b, double scl, double *c) {
	vector_plus_scaled_vector_n(a, b, 3, scl, c);
}

double vector_distance_sq_n (double *a, double *b, int n) {
	double sum = 0.0;
	int i;
	if (a == NULL || b == NULL) return 0.0;
	if (a == b) return 0.0;
	for (i = 0; i < n; i++) sum += (a[i] - b[i]) * (a[i] - b[i]);
	return sum;
}

double vector_distance_sq (double *a, double *b) {
	return vector_distance_sq_n(a, b, 3);
}

double vector_distance_n (double *a, double *b, int n) {
	return sqrt(vector_distance_sq_n(a, b, n));
}

double vector_distance (double *a, double *b) {
	return sqrt(vector_distance_sq(a, b));
}

void vector_copy_n (double *a, int n, double *b) {
	int i;
	for (i = 0; i < n; i++) b[i] = a[i];
}

void vector_copy (double *a, double *b) {
	vector_copy_n(a, 3, b);
}

//	remove component of a that is parallel to b. result orthogonal to b

void remove_component_n (double *a, double *b, int n, double *ad) {
	double bdb, comp;
	bdb = dotproduct_n(b, b, n);
	if (bdb == 0.0) {
		printf("WARNING: zero length vector\n");
		vector_copy_n(a, n, ad);
		return;
	}
	comp = dotproduct_n(a, b, n);
	vector_plus_scaled_vector_n(a, b, n, -comp / bdb, ad);
}

void remove_component (double *a, double *b, double *ad) {
	remove_component_n(a, b, 3, ad);
}

/*******************************************************************/

//	functions restricted to 3-vectors - related to cross-product

void crossproduct (double * a, double * b, double * c) {
	if (a == NULL || b == NULL || c == NULL) return;
	if (c == a || c == b) printf("ERROR: crossproduct\n");
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

double tripleproduct (double *a, double *b, double *c) {
	double bxc[3];
	if (a == NULL || b == NULL || c == NULL) return 0.0;
	crossproduct(b, c, bxc);
	return dotproduct(a, bxc);
}

double angle_between_vectors (double *a, double *b) {
	double dot, cross;
	double axb[3];			// a x b
	dot = dotproduct(a, b);
	crossproduct(a, b, axb);
	cross = vector_magnitude(axb);
	return atan2 (cross, dot);		// range 0 -- PI
}

//	force vectors u and v to be orthogonal, that is,
//	find nearest vectors that are orthogonal, that is,
//	find alphs s.t. (u + alpha * v) . (v + alpha * u) = 0

void vector_orthogonalize (double *u, double *v) {
	double alpha, alphad, a, b;
	double ud[3], vd[3];
	int i;

	a = dotproduct(u, v);
	if (a == 0.0) return;			// already orthogonal
	b = dotproduct(u, u) + dotproduct(v,v);
//	we expect a to be very small, so be careful solving the quadratic:
//		a * alpha^2 + b * alpha + a = 0
	if (debugflag) printf("a %lg b %lg\n", a, b);
	alpha = - a / b;				// first approximation
	if (debugflag) printf("alpha %lg\n", alpha);
//	now find fixed point of alpha = - (a/b) (alpha * alpha + 1)
	for (i = 0; i < 8; i++) {
		alphad = - (a/b) * (alpha * alpha + 1);
		if (alphad == alpha) break;
		else alpha = alphad;
		if (debugflag) printf("alpha %lg (%d)\n", alpha, i);
	}

	vector_plus_scaled_vector(u, v, alpha, ud);
	vector_plus_scaled_vector(v, u, alpha, vd);
	vector_copy(ud, u);
	vector_copy(vd, v);
}

/*************************************************************************************/

//	matrix and matrix/vector operations

void extract_row_nm (double **mat, int n, int m, int k, double *res) {
	if (mat == NULL || res == NULL) return;
	if (k >= n) printf("ERROR: extract_row_nm %d >= %d (%d x %d)\n",
					   k, n, n, m);
	vector_copy_n(mat[k], m, res);
}

void extract_row (double **mat, int k, double *res) {
	if (k >= 3) printf("ERROR: extract_row %d >= 3\n", k);
	extract_row_nm(mat, 3, 3, k, res);
}

void extract_column_nm (double **mat, int n, int m, int k, double *res) {
	int i;
	if (mat == NULL || res == NULL) return;
	if (k >= m) printf("ERROR: extract_column_nm %d >= %d (%d x %d)\n",
					   k, m, n, m);
	for (i=0; i < n; i++) res[i] = mat[i][k];
}

void extract_column (double **mat, int k, double *res) {
	if (k >= 3) printf("ERROR: extract_column %d >= 3\n", k);
	extract_column_nm(mat, 3, 3, k, res);
}

void insert_row_nm (double *res, int n, int m, int k, double **mat) {
	if (mat == NULL || res == NULL) return;
	if (k >= n) printf("ERROR: insert_row_nm %d >= %d (%d x %d)\n",
					   k, n, n, m);
	vector_copy_n(res, m, mat[k]);
}

void insert_row (double *res, int k, double **mat) {
	if (k >= 3) printf("ERROR: insert_row %d >= 3\n", k);
	insert_row_nm(res, 3, 3, k, mat);
}

void insert_column_nm (double *res, int n, int m, int k, double **mat) {
	int i;
	if (mat == NULL || res == NULL) return;
	if (k >= m) printf("ERROR: insert_column_nm %d >= %d (%d x %d)\n",
					   k, m, n, m);
	for (i=0; i < n; i++) mat[i][k] = res[i];
}

void insert_column (double *res, int k, double **mat) {
	if (k >= 3) printf("ERROR: insert_column %d >= 3\n", k);
	insert_column_nm(res, 3, 3, k, mat);
}

void transpose_nm (double **mat, int n, int m, double **mt) {
	double temp;				// so can do inplace transpose
	int i, j;
	if (mat == NULL || mt == NULL) return;
	if (mt == mat) {	// in place
		if (n == m) {	// square matrix
			for (i = 0; i < n; i++) {
				for (j = i+1; j < n; j++) {
					SWAP(mat[i][j], mat[j][i]);
				}
			}
		}
		else printf("ERROR: transpose_nm %d %d\n", n, m);
	}
	else {
		for (i = 0; i < n; i++) {
			for (j = 0; j < m; j++) {
				mt[j][i] = mat[i][j];
			}
		}
	}
}

void transpose (double **mat, double **mt) {
	transpose_nm(mat, 3, 3, mt);
}

//	n x m matrix times m-vector yields n-vector
//	exploits the fact that the matrix is stored as array of rows

void matrix_times_vector_nm (double ** mat, double * v, int n, int m, double * res) {
	int i;
	if (mat == NULL || v == NULL || res == NULL) return;
	if (res == v) printf("ERROR: Matrix times vector\n");
	for (i = 0; i < n; i++)	res[i] = dotproduct_n (mat[i], v, m);
}

void matrix_times_vector (double ** mat, double * v, double * res) {
	matrix_times_vector_nm (mat, v, 3, 3, res);
}

//	n x m matrix transposed times n-vector yields m-vector

void matrix_transpose_times_vector_nm (double ** mat, double * v, int n, int m, double * res) {
	double *column;
	int i;
	if (mat == NULL || v == NULL || res == NULL) return;
	if (res == v) printf("ERROR: Matrix tranpose times vector\n");
	column = make_vector_n(n);
	for (i=0; i < m; i++) {
		extract_column_nm(mat, n, m, i, column);
		res[i] = dotproduct_n(column, v, n);
	}
	free_vector_n(column, n);
}

void matrix_transpose_times_vector (double ** mat, double * v, double * res) {
	matrix_transpose_times_vector_nm(mat, v, 3, 3, res);	
}

double matrix_magnitude_nm (double **mat, int n, int m) {	// Frobenius norm
	double sum=0.0;
	int i;
	if (mat == NULL) return 0.0;
	for (i = 0; i < n; i++) sum += dotproduct_n(mat[i], mat[i], m);
	return sum;
}

double matrix_magnitude (double **mat) {	// Frobenius norm
	return matrix_magnitude_nm(mat, 3, 3);
}

void matrix_scale_nm (double **mat, int n, int m, double scl, double **md) {
	int i;
	if (mat == NULL || md == NULL) return;
	for (i = 0; i < n; i++)	vector_scale_n(mat[i], m, scl, md[i]);
}

void matrix_scale (double **mat, double scl, double **md) {
	matrix_scale_nm(mat, 3, 3, scl, md);
}

//	matrix A (n x m) times matrix B (m x p) yields matrix C (n x p)
//	use dot-products of rows in A and rows in transpose(B)
//	these matrices must be distinct

void matrix_times_matrix_nm (double **mat, double **nat, int n, int m, int p, double **res) {
	double **natd;
	int i, j;
	if (mat == NULL || nat == NULL || res == NULL) return;
	if (res == mat) printf("ERROR: Matrix product\n");
	natd = make_matrix_nm(p, m);
	transpose_nm(nat, m, p, natd);
	for (i = 0; i < n; i++) {
		for (j = 0; j < p; j++) res[i][j] = dotproduct_n(mat[i], natd[j], m);
	}
	free_matrix_nm(natd, p, m);
}

void matrix_times_matrix (double **mat, double **nat, double **res) {
	matrix_times_matrix_nm(mat, nat, 3, 3, 3, res);
}

//	matrix A (n x m) times matrix B (p x m) transposed yields matrix C (n x p)

void matrix_times_matrix_transpose_nm (double **mat, double **nat, int n, int m, int p, double **res) {
	int i, j;
	if (mat == NULL || nat == NULL || res == NULL) return;
	if (res == mat || res == nat) printf("ERROR: Matrix product\n");
	for (i = 0; i < n; i++) {
		for (j = 0; j < p; j++) res[i][j] = dotproduct_n(mat[i], nat[j], m);
	}
}

void matrix_times_matrix_transpose (double **mat, double **nat, double **res) {
	matrix_times_matrix_transpose_nm(mat, nat, 3, 3, 3, res);
}

//	matrix A (n x m) transposed times matrix B (n x p) yields matrix C (m x p)

void matrix_transpose_times_matrix_nm (double **mat, double **nat, int n, int m, int p, double **res) {
	double **matd;
	double **natd;
	int i, j;
	if (mat == NULL || nat == NULL || res == NULL) return;
	matd = make_matrix_nm(m, n);
	natd = make_matrix_nm(p, n);
	transpose_nm(mat, n, m, matd);
	transpose_nm(nat, n, p, natd);
	for (i = 0; i < m; i++) {
		for (j = 0; j < p; j++) res[i][j] = dotproduct_n(matd[i], natd[j], n);
	}
	free_matrix_nm(natd, p, n);
	free_matrix_nm(matd, m, n);
}

void matrix_transpose_times_matrix (double **mat, double **nat, double **res) {
	matrix_transpose_times_matrix_nm(mat, nat, 3, 3, 3, res);
}

void matrix_copy_nm (double **a, int n, int m, double **b) {
	int i;
	for (i = 0; i < n; i++) vector_copy_n(a[i], m, b[i]);
}

void matrix_copy (double **a, double **b) {
	matrix_copy_nm(a, 3, 3, b);
}

//	matrix times diagonal matrix (given as vector of diagonal elements)

void matrix_times_diagonal_nm (double **mat, int n, int m, double *d, double **md) {
	int i, j;
	if (mat == NULL || md == NULL || d == NULL) return;
	for (i = 0; i < n; i++) {
		for (j = 0; j < m; j++) md[i][j] = mat[i][j] * d[j];
	}
}

void matrix_times_diagonal (double **mat, double *d, double **md) {
	matrix_times_diagonal_nm(mat, 3, 3, d, md);
}

void matrix_from_rows(double *r0, double *r1, double *r2, double **mat) {
	vector_copy(r0, mat[0]);
	vector_copy(r1, mat[1]);
	vector_copy(r2, mat[2]);
}

void matrix_from_columns(double *c0, double *c1, double *c2, double **mat) {
	insert_column (c0, 0, mat);
	insert_column (c1, 1, mat);
	insert_column (c2, 2, mat);
}

/******************************************************************************/

//	inverse, equation solver, determinant for 3 x 3 matrices

double determinant (double **mat) {
	return tripleproduct (mat[0], mat[1], mat[2]);
}

int matrix_inverse (double **mat, double **inv) {
	double det;
	if (mat == NULL || inv == NULL) return -1;
	if (mat == inv) printf("ERROR: Matrix inverse\n");
	det = determinant(mat);
	if (det == 0.0) {
		printf("ERROR: Determinant is zero\n");
		return -1;
	}
	if (debugflag) printf("Determinant %lg\n", det);
	crossproduct(mat[1], mat[2], inv[0]);
	crossproduct(mat[2], mat[0], inv[1]);
	crossproduct(mat[0], mat[1], inv[2]);
	transpose(inv, inv);
	matrix_scale(inv, 1/det, inv);
	return 0;
}

int solve_equations (double **mat, double *rhs, double *sol) {
	double **tran;
	double det;
	if (mat == NULL || rhs == NULL || sol == NULL) return -1;
	tran = make_matrix();
	transpose(mat, tran);
	det = tripleproduct(tran[0], tran[1], tran[2]);
	if (traceflag) printf("Determinant %lg\n", det);
	if (det == 0.0) {
		printf("ERROR: zero determinant\n");
		return -1;
	}
	sol[0] = tripleproduct(rhs, tran[1], tran[2]) / det;
	sol[1] = tripleproduct(rhs, tran[2], tran[0]) / det;
	sol[2] = tripleproduct(rhs, tran[0], tran[1]) / det;
	free_matrix(tran);
	return 0;
}

/******************************************************************************/

//	Gauss Jordan from Numerical Recipes

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

int gauss_solve (double **a, int n, double *rhs) {
	double **b;
	int k, flag;
	b = make_matrix_nm(n, 1);	// skinny right hand side
	for (k = 0; k < n; k++) b[k][0] = rhs[k];
	flag = gaussj (a, n, b, 1);
	for (k = 0; k < n; k++) rhs[k] = b[k][0];
	free_matrix_nm(b, n, 1);
	return flag;
}

/*****************************************************************************/

//	use pseudo-inverse to solve over-determined set of linear equations A x = b 
//	minimum error solution is x = (A^T T)^{-1} A^T b

int pseudo_solve (double **dm, int n, int m, double *rhs) {
	double **dmd;
	double *rhsd;
	int flag=0;
	
	dmd = make_matrix_nm (m, m);
	rhsd = make_vector_n(m);

	if (traceflag) {
		printf("M: (%d x %d)\n", n, m);
		show_matrix_nm(dm, n, m);
		printf("b: (%d)", n);
		show_vector_n(rhs, n);
	}

	matrix_transpose_times_matrix_nm(dm, dm, n, m, m, dmd);	// A^T A
	matrix_transpose_times_vector_nm(dm, rhs, n, m, rhsd); // A^T b

	if (traceflag) {
		printf("M^T M: (%d x %d)\n", m, m);
		show_matrix_nm(dmd, m, m);
		printf("M^T b (%d): ", m);
		show_vector_n(rhsd, m);
	}

	if (gauss_solve(dmd, m, rhsd) != 0) flag = -1;
	vector_copy_n(rhsd, m, rhs);

	free_vector_n(rhsd, m);
	free_matrix_nm(dmd, m, m);
	return flag;
}

/*****************************************************************************/

//	unit quaternions provide convenient representation for rotation

//	quaternion operations

double quaternion_dotproduct (double *p, double *q) {
	return dotproduct_n(p, q, 4);
}

double quaternion_magnitude (double *q) {
	return (sqrt(quaternion_dotproduct(q, q)));
}

void quaternion_scale (double *q, double scl, double *qs) {
	vector_scale_n(q, 4, scl, qs);
}

void quaternion_copy (double *a, double *b) {
	vector_copy_n(a, 4, b);
}

int quaternion_normalize (double *q, double *qn) {
	double mag;
	mag = quaternion_magnitude(q);
	if (mag == 0.0) {
		printf("Zero size quaternion\n");
		return -1;
	}
	quaternion_scale(q, 1/mag, qn);
	return 0;
}

//	split quaternion into scalar and vector part

double decompose_quaternion (double *q, double *qv) {
	qv[0] = q[1]; qv[1] = q[2]; qv[2] = q[3];
	return q[0];
}

//	construct quaternion from scalar and vector part

void compose_quaternion (double q0, double *qv, double *q) {
	q[0] = q0; q[1] = qv[0]; q[2] = qv[1]; q[3] = qv[2];
}

//	compose two rotations by multiplying the unit quaternions

void quaternion_product (double *p, double *q, double *r) {
	double p0, q0, r0;
	double qv[3], pv[3], rv[3];
	p0 = decompose_quaternion(p, pv);
	q0 = decompose_quaternion(q, qv);
	r0 = p0 * q0 - dotproduct(pv, qv);
	crossproduct(pv, qv, rv);
	vector_plus_scaled_vector(rv, pv, q0, rv);
	vector_plus_scaled_vector(rv, qv, p0, rv);
	compose_quaternion(r0, rv, r);
}

//	make unit quaternion from unit axis vector and angle 
//	we normalize axis first just to make sure...

int quaternion_from_axis_and_angle (double *axis, double angle, double *q) {
	double axisd[3];
	double c2, s2;
	if (axis == NULL || q == NULL) return -1;
	c2 = cos (angle/2);
	s2 = sin (angle/2);
	if (vector_normalize(axis, axisd) != 0) {
		printf("ERROR: Axis has zero length\n");
		return -1;
	}
	vector_scale(axisd, s2, axisd);
	compose_quaternion(c2, axisd, q);
	return 0;
}

//	unit axis vector and angle from unit quaternion

double axis_and_angle_from_quaternion (double *q, double *axis) {
	double c2, s2;
	c2 = decompose_quaternion(q, axis);
	s2 = vector_magnitude(axis);
	if (vector_normalize(axis, axis) != 0) {
		if (c2 == 0.0) {
			printf("ERROR: Zero quaternion\n");
			return 0.0;
		}
	}
	return 2 * atan2(s2, c2);		// angle --- range 0 - 2 * PI
}

/************************************************************************/

//	Need to be able to convert from and to orthonormal matrix also

//	make orthonormal rotation matrix from unit quaternion
//	we normalize quaternion first just to make sure...

int matrix_from_quaternion (double *q, double **r) {
	double qd[4];
	if (q == NULL || r == NULL) return -1;
	if (quaternion_normalize(q, qd) != 0) {
		printf("ERROR: quaternion is zero\n");
		return -1;
	}
	r[0][0] = qd[0] * qd[0] + qd[1] * qd[1] - qd[2] * qd[2] - qd[3] * qd[3];
	r[1][1] = qd[0] * qd[0] - qd[1] * qd[1] + qd[2] * qd[2] - qd[3] * qd[3];
	r[2][2] = qd[0] * qd[0] - qd[1] * qd[1] - qd[2] * qd[2] + qd[3] * qd[3];
	r[0][1] = 2 * (-qd[0] * qd[3] + qd[1] * qd[2]);
	r[1][0] = 2 * ( qd[0] * qd[3] + qd[1] * qd[2]);
	r[0][2] = 2 * ( qd[0] * qd[2] + qd[1] * qd[3]);
	r[2][0] = 2 * (-qd[0] * qd[2] + qd[1] * qd[3]);
	r[1][2] = 2 * (-qd[0] * qd[1] + qd[2] * qd[3]);
	r[2][1] = 2 * ( qd[0] * qd[1] + qd[2] * qd[3]);
	return 0;
}

//	Recover quaternion from what is assumed to be an orthonormal matrix ...
//	This attempts to have good numerical properties ...
//	Normalizes result for good measure

int quaternion_from_matrix(double **r, double *q) {
	double f0m, f1m, f2m, f3m, fmx;
	double f01, f02, f12, f23, f30, f31;
	
	f0m = +r[0][0] + r[1][1] + r[2][2];
	f1m = +r[0][0] - r[1][1] - r[2][2];
	f2m = -r[0][0] + r[1][1] - r[2][2];
	f3m = -r[0][0] - r[1][1] + r[2][2];

	f12 =  r[1][0] + r[0][1];
	f23 =  r[2][1] + r[1][2];
	f31 =  r[0][2] + r[2][0];

	f30 =  r[1][0] - r[0][1];
	f01 =  r[2][1] - r[1][2];
	f02 =  r[0][2] - r[2][0] ;

//	f0m, f1m, f2m, f3m should equal q0*q0+1, q1*q1+1, q2*q2+1, q3*q3+1
//	hence all should be >= -1.0
//	but due to numerical noise, some may be slightly less
//	but at least the *largest* ought to be > -1.0
	fmx = f0m;				// find the largest 
	if (f1m > fmx) fmx = f1m;
	if (f2m > fmx) fmx = f2m;
	if (f3m > fmx) fmx = f3m;

	if (fmx < -1) {
		printf("ERROR: in quaternion_from_matrix %lg (%lg) %lg (%lg) %lg (%lg) %lg (%lg) \n",
			   f0m, f0m+1, f1m, f1m+1, f2m, f2m+1, f3m, f3m+1);
		show_matrix(r);		// argh, very bad "rotation matrix"!
		return -1;
	}

	if (f0m == fmx) {
		q[0] = sqrt(f0m + 1)/2;
		q[1] = f01 / (q[0] * 4);
		q[2] = f02 / (q[0] * 4);
		q[3] = f30 / (q[0] * 4);		
	}
	else if (f1m == fmx) {
		q[1] = sqrt(f1m + 1)/2;
		q[2] = f12 / (q[1] * 4);
		q[3] = f31 / (q[1] * 4);
		q[0] = f01 / (q[1] * 4);
	}
	else if (f2m == fmx) {
		q[2] = sqrt(f2m + 1)/2;
		q[3] = f23 / (q[2] * 4);
		q[0] = f02 / (q[2] * 4);
		q[1] = f12 / (q[2] * 4);
	}
	else if (f3m == fmx) {
		q[3] = sqrt(f3m + 1)/2;
		q[0] = f30 / (q[3] * 4);
		q[1] = f31 / (q[3] * 4);
		q[2] = f23 / (q[3] * 4);
	}
	else {
		printf("ERROR: in quaternion_from_matrix\n");
		return -1;				// this can't happen
	}
	if (quaternion_normalize(q, q) != 0) return -1;
	return 0;
}

//	one way to describe magnitude of difference in coordinate system orientations
//	-- recover axis and angle of rotation required to rotate one into the other --
//	that is R1 R2^T if one attitude is pecified by R1 and the other by R2.
//	Direction of axis of rotation is discarded, returns the angle to turn through.

double attitude_difference (double **R1, double **R2) {
	double **R1R2T;
	double angle;
	double axis[3];
	double q[4];

	R1R2T = make_matrix();

	matrix_times_matrix_transpose(R1, R2, R1R2T);
	if (quaternion_from_matrix(R1R2T, q) != 0) {
		printf("ERROR: in attitude_difference\n");
		return 0.0;
	}
	angle = axis_and_angle_from_quaternion(q, axis);

	free_matrix(R1R2T);
	return angle;			
}

////////////////////////////////////////////////////////////////////////


// True transformation model	// four parameter transform

// x'_i =  c' x_i + s' y_i + xo
// y'_i = -s' x_i + c' y_i + yo

void transform (double *point, double *param, double *pointd) {
	double cd, sd, xo, yo;
	double x, y, xd, yd;
	cd = param[0]; sd = param[1]; xo = param[2]; yo = param[3];
	x = point[0]; y = point[1];
	xd =  cd * x + sd * y + xo;
	yd = -sd * x + cd * y + yo;	
	pointd[0] = xd;	pointd[1] = yd;
}

// Alternate transformation model // six parameter transform

// x'_i =  a x_i + b y_i + e
// y'_i =  c x_i + d y_i + f

void transformh (double *point, double *paramh, double *pointd) {
	double a, b, c, d, e, f;
	double x, y, xd, yd;
	a = paramh[0]; b = paramh[1]; c = paramh[2]; d = paramh[3];
	e = paramh[4]; f = paramh[5];
	x = point[0]; y = point[1];
	xd =  a * x + b * y + e;
	yd =  c * x + d * y + f;	
	pointd[0] = xd;	pointd[1] = yd;
}

// recover four parameter transform from two correspondences

int recover (double points[][2], double pointsd[][2], double *paramd) {
	double x1, y1, x2, y2, xd1, yd1, xd2, yd2;
	double dx, dy, dxd, dyd, xa, ya, xad, yad;
	double dsq, dot, crs;
	double cd, sd, xo, yo;

	x1 = points[0][0];	y1 = points[0][1];
	x2 = points[1][0];	y2 = points[1][1];
	xd1 = pointsd[0][0];	yd1 = pointsd[0][1];
	xd2 = pointsd[1][0];	yd2 = pointsd[1][1];

	dx = x2 - x1;	dy = y2 - y1;
	dxd = xd2 - xd1;	dyd = yd2 - yd1;

	dsq = dx * dx + dy * dy;
	if (dsq == 0.0) return -1;
	dot = dxd * dx + dyd * dy;
	crs = dxd * dy - dyd * dx;
	cd = dot / dsq;	sd = crs / dsq;
	paramd[0] = cd;	paramd[1] = sd;

//	use average (for numerical accuracy only)
	xa = (x1 + x2) / 2.0;	ya = (y1 + y2) / 2.0;
	xad = (xd1 + xd2) / 2.0;	yad = (yd1 + yd2) / 2.0;

	xo = xad - ( cd * xa + sd * ya);
	yo = yad - (-sd * xa + cd * ya);
	paramd[2] = xo;	paramd[3] = yo;
	return 0;
}

// recover four parameter transform from N >= 2 correspondences LSQ

int recovern (double points[][2], double pointsd[][2], int n, double *paramd) {
	double sumx, sumy, sumdx, sumdy;
	double xoff, yoff, xdoff, ydoff;
	double sumden, sumc, sums;
	double xa, ya, xad, yad;
	int i;
	double cd, sd, xo, yo;

//	get centroid first
	sumx = sumy = sumdx = sumdy = 0.0;
	for (i = 0; i < n; i++) {
		sumx += points[i][0];
		sumy += points[i][1];
		sumdx += pointsd[i][0];
		sumdy += pointsd[i][1];
	}
	xa = sumx / n;
	ya = sumy / n;	
	xad = sumdx / n;
	yad = sumdy / n;	

//	all measurements relative to centroid now
	sumden = sumc = sums = 0.0;
	for (i = 0; i < n; i++) {
		xoff = points[i][0] - xa;
		yoff = points[i][1] - ya;
		sumden += xoff * xoff + yoff * yoff;
		xdoff = pointsd[i][0] - xad;
		ydoff = pointsd[i][1] - yad;
		sums += xdoff * yoff - ydoff * xoff;
		sumc += xdoff * xoff + ydoff * yoff;		
	}
	cd = sumc / sumden;
	sd = sums / sumden;
	paramd[0] = cd;	paramd[1] = sd;
	xo = xad - ( cd * xa + sd * ya);
	yo = yad - (-sd * xa + cd * ya);
	paramd[2] = xo;	paramd[3] = yo;
	return 0;
}

// recover six parameter transform from three correspondences

int recoverh (double points[][2], double pointsd[][2], double *paramh) {
	double x1, y1, x2, y2, x3, y3, xd1, yd1, xd2, yd2, xd3, yd3;
	double det;
	double a, b, c, d, e, f;
	double **mat;
	double vec[3], rhs[3];

	mat = make_matrix();
	x1 = points[0][0];	y1 = points[0][1];
	x2 = points[1][0];	y2 = points[1][1];
	x3 = points[2][0];	y3 = points[2][1];
	det = (x2 * y3 - x3 * y2) + (x3 * y1 - x1 * y3) + (x1 * y2 - x2 * y1);
	if (traceflag) printf("det %lg\n", det);
	if (det == 0.0) return -1;
	xd1 = pointsd[0][0];	yd1 = pointsd[0][1];
	xd2 = pointsd[1][0];	yd2 = pointsd[1][1];
	xd3 = pointsd[2][0];	yd3 = pointsd[2][1];

	mat[0][0] = x1; mat[0][1] = y1; mat[0][2] = 1.0;
	mat[1][0] = x2; mat[1][1] = y2; mat[1][2] = 1.0;	
	mat[2][0] = x3; mat[2][1] = y3; mat[2][2] = 1.0;
	det = determinant(mat);
	if (traceflag) printf("det %lg\n", det);
	if (det == 0.0) return -1;

	rhs[0] = xd1; rhs[1] = xd2; rhs[2] = xd3;
	if (solve_equations(mat, rhs, vec) != 0) return -1;
	a = vec[0]; b = vec[1]; e = vec[2];

	rhs[0] = yd1; rhs[1] = yd2; rhs[2] = yd3;
	if (solve_equations(mat, rhs, vec) != 0) return -1;
	c = vec[0]; d = vec[1]; f = vec[2];
	
	paramh[0] = a; paramh[1] = b; paramh[2] = e;
	paramh[3] = c; paramh[4] = d; paramh[5] = f;
	if (traceflag) printf("a %lg b %lg e %lg c %lg d %lg f %lg\n",
						  paramh[0], paramh[1], paramh[2], paramh[3],
						  paramh[4], paramh[5]);
	free_matrix(mat);
	return 0;
}

// recover six parameter transform from N correspondences	LSQ

int recovernh (double points[][2], double pointsd[][2], int n, double *paramh) {
	double x,y, xd, yd;
	double a, b, c, d, e, f;
	double **mat;
	double *rhs;
	int i;

	mat = make_matrix_nm(n, 3);
	rhs = make_vector_n(n);
	for (i = 0; i < n; i++) {
		x = points[i][0];	y = points[i][1];
		mat[i][0] = x;
		mat[i][1] = y;
		mat[i][2] = 1;
		xd = pointsd[i][0];	yd = pointsd[i][1];
		rhs[i] = xd;
	}

	if (pseudo_solve(mat, n, 3, rhs) != 0) return -1;

	a = rhs[0]; b = rhs[1]; e = rhs[2];

	for (i = 0; i < n; i++) {
		x = points[i][0];	y = points[i][1];
		mat[i][0] = x;
		mat[i][1] = y;
		mat[i][2] = 1;
		xd = pointsd[i][0];	yd = pointsd[i][1];
		rhs[i] = yd;
	}

	if (pseudo_solve(mat, n, 3, rhs) != 0) return -1;

	c = rhs[0]; d = rhs[1]; f = rhs[2];

	paramh[0] = a; paramh[1] = b; paramh[2] = e;
	paramh[3] = c; paramh[4] = d; paramh[5] = f;
	if (traceflag) printf("a %lg b %lg e %lg c %lg d %lg f %lg\n",
						  paramh[0], paramh[1], paramh[2], paramh[3],
						  paramh[4], paramh[5]);
	free_vector(rhs);
	free_matrix(mat);
	return 0;
}

////////////////////////////////////////////////////////////////////////////

void setup (int npoints, double noise) {
	int k;
	if (npoints == 2) npoints = 3;	// need three min for other method
	if (traceflag)
		printf("Setting up %d points with %lg noise\n", npoints, noise);
	for (k = 0; k < npoints; k++) {
		transform(points[k], param, pointsd[k]);
		pointsd[k][0] += gaussian(noise);
		pointsd[k][1] += gaussian(noise);
		if (traceflag)
			printf("%d\t%lg %lg =>\t %lg %lg\n",
				   k, points[k][0], points[k][1], pointsd[k][0], pointsd[k][1]);
	}
}

// totals accumulated Monte Carlo simulation

double sumtheta, sumthetas, sumhtheta, sumhthetas;
double sumscl, sumscls, sumhscl, sumhscls;
double sumxy, sumxys, sumhxy, sumhxys;

void test_once (int npoints) {
	double cd, sd, xod, yod;
	double thetad, scld;
	double dtheta, dscl;
	double dxy;

	setup(npoints, noise);

	if (npoints > 2 || forceoverdeter) recovern(points, pointsd, npoints, paramd);
	else recover(points, pointsd, paramd);	// noints == 2

	cd = paramd[0]; sd = paramd[1]; xod = paramd[2]; yod = paramd[3];
	if (traceflag) printf("cd %lg sd %lg xo %lg yo %lg\n",
						  cd, sd, xod, yod);
	thetad = atan2(sd, cd);
	scld = sqrt(cd * cd + sd * sd);
	if (traceflag) printf("Recovered theta %lg scl %lg xo %lg yo %lg\n",
		   deg_from_rad(thetad), scld, xod, yod);
	dtheta = fabs(thetad - theta);
	sumtheta += dtheta;
	sumthetas += dtheta * dtheta;
	dscl = fabs(scld - scl);
	sumscl += dscl;
	sumscls += dscl * dscl;
	dxy = sqrt((xod-xo)*(xod-xo)+(yod-yo)*(yod-yo));
	sumxy += dxy;
	sumxys += dxy * dxy;

	if (npoints == 2) npoints = 3; // need three points for alternate method

	if (npoints > 3 || forceoverdeter) recovernh(points, pointsd, npoints, paramh);
	else recoverh(points, pointsd, paramh);	// npoints == 3

	cd = (paramh[0] + paramh[4]) / 2.0;	// average to approximate
	sd = (paramh[1] - paramh[3]) / 2.0;	// average to approximate
	xod = paramh[4]; yod = paramh[5];
	if (traceflag) printf("cd %lg sd %lg xo %lg yo %lg\n",
						  cd, sd, xod, yod);
	thetad = atan2(sd, cd);
	scld = sqrt(cd * cd + sd * sd);
	if (traceflag) printf("Recovered theta %lg scl %lg xo %lg yo %lg\n",
		   deg_from_rad(thetad), scld, xod, yod);	
	dtheta = fabs(thetad - theta);
	sumhtheta += dtheta;
	sumhthetas += dtheta * dtheta;
	dscl = fabs(scld - scl);
	sumhscl += dscl;
	sumhscls += dscl * dscl;
	dxy = sqrt((xod-xo)*(xod-xo)+(yod-yo)*(yod-yo));
	sumhxy += dxy;
	sumhxys += dxy * dxy;	
}

void test (int npoints, int n) {
	int i;

	if (verboseflag) {
		if (npoints == 2)
			printf("\nTesting with 2/3 points:\n");
		else printf("\nTesting with %d points:\n", npoints);
	}
	sumtheta = sumthetas = sumhtheta = sumhthetas = 0.0;
	sumscl = sumscls = sumhscl = sumhscls = 0.0;	// reset totals
	sumxy = sumxys = sumhxy = sumhxys = 0.0;

	for (i = 0; i < n; i++) test_once(npoints);
//	results for model A angle
	printf("theta err 4 param %lg (%lg) degrees\n",
		   deg_from_rad(sumtheta/n), deg_from_rad(sqrt(sumthetas/n)));
//	results for model B angle
	printf("theta err 6 param %lg (%lg) degrees\n",
		   deg_from_rad(sumhtheta/n), deg_from_rad(sqrt(sumhthetas/n)));

//	results for model A scale
	printf("scale err 4 param %lg (%lg)\n", sumscl/n, sqrt(sumscls/n));
//	results for model B scale
	printf("scale err 6 param %lg (%lg)\n", sumhscl/n, sqrt(sumhscls/n));

//	results for model A translation
	printf("trans err 4 param %lg (%lg)\n", sumxy/n, sqrt(sumxys/n));
//	results for model B translation
	printf("trans err 6 param %lg (%lg)\n", sumhxy/n, sqrt(sumhxys/n));
}

int main(int argc, char *argv[]) {
	int i;
	double cd, sd;
	int npoints = sizeof(points)/sizeof(points[0]);

	if (forceequilateral) {
//		points[2][0] = points[2][1] = 1.0 + sqrt(2.0);
		points[2][0] = points[2][1] = (1.0 + sqrt(3.0)) / 2.0;
		for (i = 0; i < 3; i++) 
			printf("%d\t%lg %lg\n", i, points[i][0], points[i][1]);
	}
	if (asymmetryflag) {
		for (i = 0; i < npoints; i++) points[i][1] = points[i][1] / 4.0;
	}
	printf("Maximum of %d points in target\n", npoints);
	printf("Testing with %d random trials\n", nrepeat);
	cd = param[0]; sd = param[1]; xo = param[2], yo = param[3];
	theta = atan2(sd, cd);
	scl = sqrt(sd * sd + cd * cd);
	printf("theta %lg scl %lg xo %lg yo %lg\n",
		   deg_from_rad(theta), scl, xo, yo);

	fflush(stdout);
	test(2, nrepeat);
	fflush(stdout);
	test(3, nrepeat);
	fflush(stdout);
	test(npoints, nrepeat);
	return 0;
}
