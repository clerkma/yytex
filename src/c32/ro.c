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
#include <math.h>
#include "useful.h"

#include "useful.c"					// for convenience on my machine (?)

#define b_WGS84 6356752.3142		// Semi-minor axis (Polar radius)
#define a_WGS84 6378137				// Semi-major Axis (Equatorial radius)
#define eSquared 0.00669437999013	// First eccentricity squared

#define PI 3.1415926535
#define MAXPOINTS 32
#define HEIGHT 1030
#define WIDTH 1300
#define ANGLE 28
#define FOCAL_LENG 0.4
#define EXAGGERATION_FACTOR 1
// shouldn't need too many iterations (?)
#define MAXITER 10
// possibly take less than full step in adjustment (?)
#define ALPHA 1.0

int testflag=0;			// debugging only (?)
int verboseflag=0;		// debugging only (?)

///////////////////////////////////////////////////////////////////

//
//   Compute the leng of a one-Pi arc parallel at a certain latitude
//
double ComputeRadiusLat(double Latitude)
{
    double x, y ;
    Latitude = Latitude * PI / 180.0 ;
    x = a_WGS84 * (1.0 - eSquared) ;
    y = pow(1.0 - eSquared * pow(sin(Latitude),2), 1.5) ;
	return (x/y) ;
}
//
//   Compute the leng of a one-Pi arc meridian at a certain latitude
//
double ComputeRadiusLong(double Latitude)
{
    double x, y ;
    Latitude = Latitude * PI / 180.0 ;
    x = a_WGS84 * cos(Latitude) ;
    y = pow(1.0 - eSquared * pow(sin(Latitude), 2), 0.5) ;
    return(x/y) ;
}
//
//	Distance between two points x(long1, lat, altitude) and y(long2, lat, altitude)
//  Use WGS84 reference ellipsoid
//	Altitude is ellipsoidal height
//  For small distances (< 10 km), difference between arc length and straight line is negligible 
//
double ComputeDistanceX(double Altitude, double lat, double long1, double long2)
{
    double x ;
	double Radius ;
	Radius = ComputeRadiusLong(lat) ;
	x = ((long2-long1)*PI)/180.0 ;
	x = x * Radius ;
	// Adjust for altitude
	x = x * ((Radius+Altitude)/Radius) ;
	return x ;
}
//
//	Distance between two points x(long1, lat, Altitude) and y(long2, lat, Altitude)
//
double ComputeDistanceY(double Altitude, double lat1, double lat2)
{
    double x ;
	double Radius ;
	Radius = ComputeRadiusLat(lat1) ; 
	x = ((lat2-lat1)*PI)/180.0 ;
	x = x * Radius ;
	// Adjust for altitude
	x = x * ((Radius + Altitude) / Radius) ;
	return x ;
}

//
// Compute vector from point 1 to point 2 in meters
//
void vector_between_two_points(double long1, 
							   double lat1, 
							   double altitude1, 
							   double long2, 
							   double lat2, 
							   double altitude2,
							   double *v)
{
	v[0] = ComputeDistanceX(altitude1, lat1, long1, long2) ;
	v[1] = ComputeDistanceY(altitude1, lat1, lat2) ;
	v[2] = altitude2 - altitude1 ;
}

///////////////////////////////////////////////////////////////////

// split off for convenience - maybe put in useful.c (?)

void rotate_vector (double *r, double *q, double *rd)
{
	double qrl[4], qc[4], qtemp[4];
	quaternion_conjugate(q, qc) ;
	compose_quaternion(0, r, qrl) ;
	quaternion_product(q, qrl, qtemp) ;
	quaternion_product(qtemp, qc, qrl) ;
	(void) decompose_quaternion(qrl, rd) ;
}

#ifdef IGNORED
// alternate written out in terms of vector operations
void rotate_vector (double *r, double *q, double *rd)
{
	double vr[3], qv[3], qr[3], qqr[3];
	double qo, qmag;	// qmag should be 1
	qo = decompose_quaternion(q, qv);
	qmag = dotproduct(qv, qv) + qo * qo;
	crossproduct(qv, r, qr);
	crossproduct(qv, qr, qqr);
	vector_scale(qr, 2 * qo, qr);
	vector_scale(qqr, 2, qqr);
	vector_scale(r, qmag, rd);
	vector_sum(rd, qr, rd);
	vector_sum(rd, qqr, rd);
}
#endif

///////////////////////////////////////////////////////////////////

int exit_cond(double *db, double *dw)
{
	return 0 ;	// For now
}
//
//	Notes:
//  1. The aircraft moves in the positive y direction, therefore the "left" image
//	in this case is the lower image and the "right" image the upper image.
//
int main(int argc, char *argv[])
{
    // First construct the left and right rays in their own coordinate systems
    double *rl [MAXPOINTS] ;    // Left rays 
    double *rr [MAXPOINTS] ;     // Right rays
    double *rl2[MAXPOINTS] ;    // Left rays in right camera coordinates
    double angle_x = 0 ; // Relative orientation angle (x , y and z rotation)
	double angle_y = 0 ;
	double angle_z = 0 ;
	double *axis_x, *axis_y, *axis_z ; // Unit axis
	double *q, *qx, *qy, *qz, *qtemp ;
//	double *qc ;		// conjugate quaternion (?)
	double *c[MAXPOINTS] ;
	double *d[MAXPOINTS] ;
	double t[MAXPOINTS] ;
	double w[MAXPOINTS] ;
	double *b ;
	double variance_o = 1.0E6 ;
	double variance_l[MAXPOINTS] ;
	double variance_r[MAXPOINTS] ;
	double **C, **D, **F ;
	double *c_bar, *d_bar, lambda ;
	double *vtemp;		// new ?
	double **mtemp ;
	double **A ;
	double *v_zero ;	// zero vector
	double *x_zero ;	// zero vector (length 7) (?)
	double *db, *dw ;	// delta w vector 
	double *qdw ;		// delta w quaternion
	double *x ;			// Vector in equation A x = y
	double hr, hl ;		// Elevation (ellipsoidal height) of right and left projection points
	double alpha=ALPHA;	// dampening factor in iterative adjustment (?)
	double theta;		// angle tor orientation (?)
	double *axis;		// axis of orientation (?)
    int i, iter ;

    // Image (pixel) coordinates of corresponding points
    
    int n = 6 ;        // Number of rays
    double xl[MAXPOINTS] = {190, 217, 916, 887, 1257, 1160} ;
    double yl[MAXPOINTS] = {5, 213, 21, 188, 51, 179} ;	
    double xr[MAXPOINTS] = {180, 207, 925, 890, 1272, 1168} ;
    double yr[MAXPOINTS] = {799, 1019, 840, 1013, 875, 1007} ;	
    double half_height = HEIGHT / 2.0 ;        // Half image height
    double half_width  = WIDTH  / 2.0 ;        // Width
	//
	// Multiply camera focal length with an exaggeration factor to make
	// the numbers close to unity for ease of debugging
	// This should be OK since distances are proportional
	//
    double f = FOCAL_LENG * EXAGGERATION_FACTOR ;  // Camera focal length
    double lense_width_angle = ANGLE ;         // lense angle in the x dimension
    
    double plate_half_height, plate_half_width ;
    double tan_half_height, tan_half_width ;
    double pixel_size ; // x and y dimension of each pixel in plate (world) coordinate
    
	double total, oldtotal=0.0;

	char buf[128] ;
    
	printf("Initializations\n");

	// Initializations

	axis = make_vector() ;	// (?)
	v_zero = make_vector() ;
	x_zero = make_vector_n(7) ;	// (?)
	for (i = 0 ; i < 3; ++i)
		v_zero[i] = 0.0 ;

	for (i = 0 ; i < 7; ++i)
		x_zero[i] = 0.0 ;

	axis_x = make_vector() ;
	axis_y = make_vector() ;
	axis_z = make_vector() ;

	setup_vector (1.0, 0.0, 0.0, axis_x) ;
	setup_vector (0.0, 1.0, 0.0, axis_y) ;
	setup_vector (0.0, 0.0, 1.0, axis_z) ;
	vtemp = make_vector();
	q     = make_quaternion() ;
	qx    = make_quaternion() ;
	qy    = make_quaternion() ;
	qz    = make_quaternion() ;
	qtemp = make_quaternion() ;
//	qc    = make_quaternion() ;	// (?)
	mtemp = make_matrix() ;

	printf("Convert image coordinates to plate coordinate\n");
    //
    // Convert image coordinates to plate coordinate
    //
    
	for (i = 0 ; i < n; ++i)
	{
		rl [i] = make_vector() ;
		rl2[i] = make_vector() ;
        rr [i] = make_vector() ;
		c  [i] = make_vector() ;
		d  [i] = make_vector() ;
		w  [i] = 1.0 ;
	}
	
	b = make_vector() ;
	
	printf("Compute initial baseline vector using coordinates of projection points\n");
	// 
	//	Compute initial baseline vector using coordinates of projection points
	//
	hr = 409.5 ;		// From INI file
	hl = 408.2 ;
	vector_between_two_points(-95.674422, 
							  29.820931, 
							  hr,
							  -095.674319,
							  29.819818,
							  hl,
							  b) ;
	vector_scale(b, FOCAL_LENG/409.5, b) ;	// Scale to plate coordinates

//	setup_vector(1, 1, 1, b) ;	// override above for now (?)
//	setup_vector(0, 1, 0, b) ;	// override above for now (?)
	vector_normalize(b, b) ;	// (?)

	C = make_matrix() ;
	D = make_matrix() ;
	F = make_matrix() ;
	A = make_matrix_nm(7, 7) ;
	c_bar = make_vector() ;
	d_bar = make_vector() ;
	db = make_vector() ;
	dw = make_vector() ;
	qdw = make_quaternion() ;
	x = make_vector_n(7) ;
	
	printf("Transform to plate (world) coordinates\n");	// (?)
	// Transform to plate (world) coordinates

    tan_half_width = tan(rad_from_deg(lense_width_angle/2.0)) ;
    tan_half_height = tan_half_width * (half_height/half_width) ;
    plate_half_height = f * tan_half_height ;
    plate_half_width  = f * tan_half_width ;
    pixel_size = plate_half_width / half_width ;

	printf("Principle distance %lg m\n", f);
	printf("Plate half height %lg m\n", plate_half_height);
	printf("Plate half width %lg m\n", plate_half_width);
	printf("Pixel size %lg m\n", pixel_size);

	printf("Assume standard deviation is half a pixel\n");	// (?)
	// Assume standard deviation is half a pixel
	
	for (i = 0 ; i < n ; ++i)
	{
		variance_l[i] = (pixel_size/2.0) * (pixel_size/2.0) ;
		variance_r[i] = (pixel_size/2.0) * (pixel_size/2.0) ;
	}
    
	printf("Left and right rays in their original coordinates\n") ;
    for (i = 0 ; i < n ; ++i)
    {
        xl[i] = (xl[i] - half_width) * pixel_size ;
        yl[i] = (half_height - yl[i]) * pixel_size ;
        setup_vector(xl[i], yl[i], -f, rl[i]) ; 
		printf("rl[%d]= ", i) ;
		show_vector(rl[i]) ;
        xr[i] = (xr[i] - half_width) * pixel_size ;
        yr[i] = (half_height - yr[i]) * pixel_size ;
        setup_vector(xr[i], yr[i], -f, rr[i]) ; 
		printf("rr[%d]= ", i) ;
//		show_vector(rl[i]) ; (?)
		show_vector(rr[i]) ; 
    }

	// Initial value for rotation (?)
	// Rotation quaternions // (?)
	printf("Euler angles %lg %lg %lg\n", angle_x, angle_y, angle_z);
	quaternion_from_axis_and_angle(axis_x, angle_x, qx) ;
	quaternion_from_axis_and_angle(axis_y, angle_y, qy) ;
	quaternion_from_axis_and_angle(axis_z, angle_z, qz) ;
	// Combined quaternion
	quaternion_product(qz, qy, qtemp) ;
	quaternion_product(qtemp, qx, q) ;
	//	quaternion_conjugate(q, qc) ;

	printf("b= ");
	show_vector(b);
	printf("q = ") ;
	show_quaternion(q) ;
	//		printf("q'= ") ;	// (?)
	//		show_quaternion(qc) ;	// (?)

	printf("Starting iteration\n");	// (?)
	fflush(stdout);				// in case of infinite loop

    for (iter = 0 ; iter < MAXITER ; ++iter) 
    {
		printf("Iteration %d\n", iter);

//		reset matrices to zero so can accumulate in them (?)
		for (i = 0 ; i < 3 ; ++i)
		{
			insert_row(v_zero, i, C) ;
			insert_row(v_zero, i, D) ;
			insert_row(v_zero, i, F) ;
		}
		for (i = 0 ; i < 7; ++i)
			insert_row_nm (x_zero, 7, 7, i, A) ;
//		reset vectors to zero so can accumulate in them (?)
		vector_copy(v_zero, c_bar) ;
		vector_copy(v_zero, d_bar) ;

        // Transform left rays into right camera coordinate systems
		// Represent a ray by a quaternion with scalar = 0
		
		if (verboseflag)
			printf("Left rays in right image coordinates\n") ;// (?)

        for (i = 0 ; i < n ; ++i)
        {
			rotate_vector(rl[i], q, rl2[i]);
			if (verboseflag) {
				printf("rl'[%d]=", i) ;
				show_vector(rl2[i]) ;
			}
		}
		if (verboseflag) printf("Compute c[i], d[i] and t[i]\n");// (?)
		//
		// Compute c[i], d[i] and t[i]
		//
		total = 0.0;		// accumulate error terms (?)
		for (i = 0 ; i < n ; ++i)
		{
			crossproduct(rl2[i], rr[i], c[i]) ;
			if (verboseflag) {
				printf("c[%d]=", i) ;
				show_vector(c[i]) ;
			}
//			crossproduct(c[i], b, d[i]) ; // (?)
			crossproduct(rr[i], b, vtemp);
			crossproduct(rl2[i], vtemp, d[i]);
			if (verboseflag) {
				printf("d[%d]=", i) ;
				show_vector(d[i]) ;
			}
			t[i] = dotproduct(b, c[i]) ;	// sign OK
			if (verboseflag)
				printf("t[%d]=%lg\n", i, t[i]) ;
			total += t[i] * t[i];	// (?)
		}
		printf("Error %lg\n", sqrt(total/n)); // (?)
//		crude terminating condition (?)
		if (iter > 4 && total * 1.1 > oldtotal) break;	// (?)
		oldtotal = total;				// (?)

//		with such shallow angles, expect the weights to all be more
//		or less the same (?)
		if (verboseflag)
			printf("Compute weights\n");
		// Compute weights
		for (i = 0 ; i < n; ++i)
		{
			double c_mag, rl2_mag, rr_mag  ;
			double *vtemp ;
			double x1, x2 ;
			vtemp = make_vector() ;
			c_mag = vector_magnitude(c[i]) ;
			rl2_mag = vector_magnitude(rl2[i]) ;
			rr_mag = vector_magnitude(rr[i]) ;
			crossproduct(b, rr[i], vtemp) ;
			x1 = dotproduct(qtemp, c[i]) ;
			x1 = x1 * x1 * rl2_mag * rl2_mag * variance_l[i] ;
			crossproduct(b, rl2[i], vtemp) ;
			x2 = dotproduct(qtemp, c[i]) ;
			x2 = x2 * x2 * rr_mag * rr_mag * variance_r[i] ;
			w[i] = (c_mag*c_mag*variance_o) / (x1 + x2) ;
			free_vector(vtemp) ;

			// Assume unity weight for now
			w[i] = 1.0 ;
		}
		if (verboseflag) printf("Compute C\n");

		// Compute C (assumed set to zero)
		for (i =0 ; i < n; ++i)
		{
			matrix_dyadic_product(c[i], c[i], mtemp) ;
			matrix_plus_scaled_matrix(C, mtemp, w[i], C) ;
		}
		if (verboseflag) printf("Compute D\n");
		// Compute D (assumed set to zero)
		for (i =0 ; i < n; ++i)
		{
//			matrix_dyadic_product(c[i], d[i], mtemp) ;
			matrix_dyadic_product(d[i], d[i], mtemp) ; // (?)
			matrix_plus_scaled_matrix(D, mtemp, w[i], D) ;
		}
		if (verboseflag) printf("Compute F\n");
		// Compute F (assumed set to zero)
		for (i =0 ; i < n; ++i)
		{
//			matrix_dyadic_product(d[i], d[i], mtemp) ;
			matrix_dyadic_product(c[i], d[i], mtemp) ; // (?)
			matrix_plus_scaled_matrix(F, mtemp, w[i], F) ;
		}
		if (verboseflag) {
			printf("C=\n") ;
			show_matrix(C) ;
			printf("D=\n") ;
			show_matrix(D) ;
			printf("F=\n") ;
			show_matrix(F) ;
		}
		if (verboseflag)
			printf("Compute c bar\n");
		// Compute c bar (assumed set to zero)
		for (i =0 ; i < n; ++i)
		{
//			vector_plus_scaled_vector(c_bar, c[i], w[i], c_bar) ; // (?)
			vector_plus_scaled_vector(c_bar, c[i], w[i] * t[i], c_bar) ;
		}
		if (verboseflag) {
			printf("c_bar= ") ;
			show_vector(c_bar) ;
		}
		if (verboseflag)
			printf("Compute d bar\n");

		// Compute d bar (assumed set to zero)
		for (i =0 ; i < n; ++i)
		{
//			vector_plus_scaled_vector(d_bar, d[i], w[i], d_bar) ;
			vector_plus_scaled_vector(d_bar, d[i], w[i] * t[i], d_bar) ;
		}
		if (verboseflag) {
			printf("d_bar= ") ;
			show_vector(d_bar) ;
		}

		// Set up equation Ax = y
		// Compute final matrix A
		matrix_insert_at_ij_from_matrix(C,  0, 0, A) ;
		matrix_insert_at_ij_from_matrix(F,  0, 3, A) ;
		matrix_insert_at_ij_from_vector(b,  0, 6, A) ;
		matrix_insert_at_ij_from_matrix_transpose(F, 3, 0, A) ;
		matrix_insert_at_ij_from_matrix(D,  3, 3, A) ;
		matrix_insert_at_ij_from_vector_transpose(b, 6, 0, A) ;
		A[6][6] = 0.0 ;
		if (verboseflag) {
			printf("A=\n") ;
			show_matrix_nm(A, 7, 7) ;
		}
		if (testflag) {
			gauss_invert(A, 7);
			printf("A^(-1) =\n");
			show_matrix_nm(A, 7, 7);
			break;
		}
		if (verboseflag) 
			printf("Compute x\n");
		// Compute x (rhs)
		vector_scale(c_bar, -1, c_bar);	// flip sign (?)
		vector_scale(d_bar, -1, d_bar);	// flip sign (?)
		vector_insert_at_i_from_vector(c_bar, 0, x) ;
		vector_insert_at_i_from_vector(d_bar, 3, x) ;
		x[6] = 0.0 ;
		if (verboseflag) {
			printf("rhs= ") ;
			show_vector_n(x, 7) ;
		}
		gauss_solve(A, 7, x) ;
		if (verboseflag) {
			printf("sol= ") ;
			show_vector_n(x, 7) ;
		}
		vector_extract_at_i_from_vector(x, 0, db) ;
		vector_extract_at_i_from_vector(x, 3, dw) ;
		lambda = x[6];
		if (verboseflag) {
			printf("db= ") ;
			show_vector(db) ;
			printf("dw= ") ;
			show_vector(dw) ;
		}
		if (verboseflag)
			printf("lambda = %lg\n", lambda);

//		sanity check on result (should be zero)
		if (verboseflag)
			printf("b . db = %lg\n", dotproduct(b, db));

		// Adjust b and w
		if (verboseflag) 
			printf("Adjust b and w\n");
		vector_scale(db, alpha, db); // (?)
		vector_sum(b, db, b) ;
//		printf("b= ");
//		show_vector(b);
		vector_normalize(b, b) ;
		printf("b= ");
		show_vector(b);
		vector_scale(dw, alpha, dw); // (?)
		vector_scale(dw, 0.5, dw) ;
		compose_quaternion(1, dw, qdw) ;
		quaternion_product(qdw, q, q) ;
//		printf("q= ");
//		show_quaternion(q);
		quaternion_normalize(q, q);
		printf("q= ");
		show_quaternion(q);
		if (verboseflag) printf("End of iteration %d\n", iter);
		if (exit_cond(db, dw)) break ;
		printf("Press enter to continue, Control/C to abort\b") ;
		fflush(stdout);
		gets(buf) ;
	}

	printf("Translation direction ");			// (?)
	show_vector(b);					// (?)
	theta = axis_and_angle_from_quaternion(q, axis);	// (?)
	printf("Rotation %lg degrees about ", deg_from_rad(theta));
	show_vector(axis);

	free_vector(axis) ;
	free_vector_n(x_zero, 7) ;
	free_vector(v_zero) ;
	free_vector(axis_x) ;
	free_vector(axis_y) ;
	free_vector(axis_z) ;
	free_vector(vtemp);	// new (?)
	free_quaternion(q) ;
	free_quaternion(qx) ;
	free_quaternion(qy) ;
	free_quaternion(qz) ;
	free_quaternion(qtemp) ;
//	free_quaternion(qc) ;
	free_matrix(mtemp) ;
	for (i = 0 ; i < n; ++i)
	{
		free_vector(rl[i]) ;
		free_vector(rr[i]) ;
		free_vector(c [i]) ;
//		free_vector(c [i]) ;  // repeat (?)
		free_vector(d [i]) ;
	}
	free_vector(b) ;
	free_matrix(C) ;
	free_matrix(D) ;
	free_matrix(F) ;
	free_matrix_nm(A, 7, 7) ;
	free_vector(c_bar) ;
	free_vector(d_bar) ;
	free_vector(db) ;
	free_vector(dw) ;
	free_quaternion(qdw) ;
	free_vector_n(x, 7) ;
	return 0 ;
}

// NOTE: if initial conditions were not known, one would
// have to watch out for alternate minima:
// If (b, q) is a solution, then there are four solutions
// (b, q), (-b, q), (b, -q), (-b -q)
// and four more, which are most easily described as follows:
// If we let d = b q then the above corresponds to
// (d, q), (-d, q), (d, -q), (-d -q)
// but we can also interchange q and d to get
// (q, d), (-q, d), (q, -d), (-q, -d)
