/* Copyright 1990 Y&Y, Inc.
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

/* Make outline from MetaFont for lasy */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <conio.h> 
#include <math.h> 
	
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

struct coord {				/* structure for a coordinate pair */
	double x; double y;
};

/* struct line {			
	struct coord a; struct coord b;
};  */

struct spline {			/* structure for a Bezier curve */
	struct coord zs; struct coord za;
	struct coord zb; struct coord ze;	
};

/* #define FNAMELEN 80 */

static char fn_out[FILENAME_MAX];

FILE *output;

int verboseflag = 1;

int traceflag = 0;

int warnflag = 0;

int afmtrace = 1;				/* output accurate character widths */

int splitsuppress = 0;			/* suppress splitting at extrema */

double bezierq = 0.5541;		/* magic approximation for circle quadrant */

double epsilon = 0.0000001;		/* to check path continuity and convergence */

/* double inipar = 1.0;	*/		/* initial guess for alpha & beta */

double maxstep= 0.1;		/* limit change in alpha and beta per iteration */

int code;					/* character number */

static struct coord zold, zstart, zbad;

/* parameters used for lasy fonts - set by lasy5, lasy6 ...  */

double font_size=10.0; 
double w, h, d;

double letter_fit=0.0;	/* amount to add to sidebearing on each side */

/* double px = 1.0, py = 1.0; */
/* double slant=0.0, aspect_ratio=1.0; */

/* double superness = 0.707; */
/* superpull = 1.0/6.0; */
/* fudge = 1.0 */

double u=20.0/36.0;
double o = 6.0/36.0;      
double tiny = 8.0/36.0;
double crisp = 8.0/36.0; 
double hair = 9.0/36.0;
double vair = 8.0/36.0;
double bar = 11.0/36.0; 
double x_height=155.0/36.0;
double asc_height = 250.0/36.0; 
double rule_thickness = 0.4;
double math_axis = 90.0/30.0;
double cap_height = 247.0/36.0;    
double cap_curve = 35.0/36.0;
double cap_stem = 30.0/36.0;   
double slab = 11.0/36.0;
double beak = 70.0/36.0;
double beak_jut = 10.0/36.0;
double beak_darkness = 11.0/30.0;
double math_spread = 0.0;

double udash;	 /* modified u so arrow heads have same aspect ratio */
double asc_dash; /* modified asc_height so arrow heads same aspect ratio */

double ro, bo, co, to;		/* pen radii rule, bar, crisp, tiny */

double oo;					/* half of o */

double cap_vstem;			/* 0.8 * cap_stem + (1.0 - 0.8) * vair */

double o_correction=1.0;	/* over-shoot correction for device */

double pi = 3.141592653;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showcode(void) {
	fprintf(stderr, " in character %d\n", code);
	getch();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double sind(double theta) {
	return sin(theta * pi / 180.0);
}

double cosd(double theta) {
	return cos(theta * pi / 180.0);
}

double atan2d(double y, double x) {
	return (atan2(y, x) * 180.0 / pi);
}

/* direction of line connection za to zb - relative to x axis */

double direction(struct coord za, struct coord zb) {
	return atan2d(zb.y - za.y, zb.x - za.x);
}

/* direction between vector a and vector b */

double angle(struct coord va, struct coord vb) { /* angle between two vectors */
	return atan2d(va.y * vb.x - va.x * vb.y, va.x * vb.x + va.y * vb.y);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* vector sum, difference, magnitude and scaling */

struct coord diff(struct coord z1, struct coord z2) {
	struct coord zs;
	zs.x = z1.x - z2.x; 	zs.y = z1.y - z2.y;
	return zs;
}

struct coord sum(struct coord z1, struct coord z2) {
	struct coord zs;
	zs.x = z1.x + z2.x; 	zs.y = z1.y + z2.y;
	return zs;
}

struct coord scale(struct coord z, double factor) {
	struct coord zs;
	zs.x = z.x * factor;	zs.y = z.y * factor;
	return zs;
}

struct coord neg(struct coord z) {
	struct coord zs;
	zs.x = -z.x;	zs.y = -z.y;
	return zs;
}

double magnitude(struct coord z) {
	return sqrt(z.x * z.x + z.y * z.y);
}

/* return vector parallel to za - zb of length ro */

struct coord parallel(struct coord za, struct coord zb, double ro) {
	struct coord dz;
	double rad;

	dz = diff(za, zb);
	rad = magnitude(dz);
	if (rad == 0.0) {
		fprintf(stderr, "Zero length vector (parallel)");
		showcode();
		rad = 1.0; /* kludge, just to go on ? */
	}
	return scale(dz, ro / rad);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* solve pair of linear equations - given columns and right hand side */

struct coord solvepair(struct coord col1, struct coord col2, 
			struct coord rhs) {
	double a11, a12, a13, a21, a22, a23, det;
	struct coord z;

	a11 = col1.x; a12 = col2.x; a13 = rhs.x;
	a21 = col1.y; a22 = col2.y; a23 = rhs.y;
	det = a11 * a22 - a12 * a21;
	if (det == 0.0) {
/*		fprintf(stderr, "Zero Determinant"); */
		fprintf(stderr, 
			"Zero Determinant (solvpair) - %lg %lg %lg - %lg %lg %lg \n",
				a11, a12, a13, a21, a22, a23); 
		showcode();
		exit(1);
	}
	z.x = (a13 * a22 - a23 * a12) / det;
	z.y = (a23 * a11 - a13 * a21) / det;
	return z;
}

/* intersect two line segments */

struct coord intersectsub(struct coord z1, struct coord z2, 
			struct coord z3, struct coord z4) {
	struct coord col1, col2, rhs, par;
	double alpha, beta;

	col1 = diff(z2, z1); col2 = diff(z3, z4); rhs = diff(z3, z1);
	par = solvepair(col1, col2, rhs);
	alpha = par.x; beta = par.y;

/*  check whether intersection on line segments  */
	if (warnflag != 0) {
		if (alpha < 0 || alpha > 1 || beta < 0 || beta > 1) {
		fprintf(stderr, "Don't intersect alpha (intersectsub) %lg, beta %lg", 
			alpha, beta);
		showcode();
		}
	}
	return (sum(z1, scale(diff(z2, z1), alpha)));
/*	return (sum(z4, scale(diff(z3, z4), beta))); */
}

/* give weighted sum z = (1 - t) * za + t * zb  */

struct coord weighted(struct coord za, struct coord zb, double t) {
	struct coord z;
	z.x = zb.x * t + za.x * (1.0 - t);
	z.y = zb.y * t + za.y * (1.0 - t);
	return z;
}

/* compute point on Bezier's curve */

struct coord beziersub(struct coord zs, struct coord za, 
	struct coord zb, struct coord ze, double t) {
		struct coord zt;

	zt.x = zs.x * (1.0 - t) * (1.0 - t) * (1.0 - t)
		+ 3.0 * za.x * t * (1.0 - t) * (1.0 - t)
			+ 3.0 * zb.x * t * t * (1.0 - t)
				+ ze.x * t * t * t;
	zt.y = zs.y * (1.0 - t) * (1.0 - t) * (1.0 - t)
		+ 3.0 * za.y * t * (1.0 - t) * (1.0 - t)
			+ 3.0 * zb.y * t * t * (1.0 - t)
				+ ze.y * t * t * t;
	return zt;
}

struct coord bezier(struct spline bz, double t) {
	return beziersub(bz.zs, bz.za, bz.zb, bz.ze, t);
}

/* compute velocity of point on Bezier's curve  given control points */

struct coord bezierdsub(struct coord zs, struct coord za, 
	struct coord zb, struct coord ze, double t) {
		struct coord zt;

	zt.x = -3.0 * zs.x * (1.0 - t) * (1.0 - t)
		+ 3.0 * za.x * (1.0 - t) * (1.0 - 3.0 * t)
			+ 3.0 * zb.x * t * (2.0 - 3.0 * t)
				+ 3.0 * ze.x * t * t;
	zt.y = -3.0 * zs.y * (1.0 - t) * (1.0 - t)
		+ 3.0 * za.y * (1.0 - t) * (1.0 - 3.0 * t)
			+ 3.0 * zb.y * t * (2.0 - 3.0 * t)
				+ 3.0 * ze.y * t * t;
	return zt;
}

struct coord bezierd(struct spline bz, double t) {
	return bezierdsub(bz.zs, bz.za, bz.zb, bz.ze, t);
}

/* construct default 1/3 1/3 spline given ends and directions */

struct spline tensespline(struct coord zs, struct coord zsd,
			struct coord ze, struct coord zed, double alpha, double beta) {
	struct coord za, zb;
	struct spline bz;
	double rad;

	rad = magnitude(diff(ze, zs));
	za = sum(zs, scale(zsd, alpha * rad / magnitude(zsd)));
	zb = diff(ze, scale(zed, beta * rad / magnitude(zed)));
	bz.zs = zs;  bz.za = za;  bz.zb = zb;  bz.ze = ze;
	return bz;
}

/* make canonical smooth spline with 1/3 and 1/3 velocities */

struct spline smoothspline(struct coord zs, struct coord zsd,
			struct coord ze, struct coord zed) {
	return(tensespline(zs, zsd, ze, zed, 1.0/3.0, 1.0/3.0));
}

/* make elliptical quadrant spline - assumes zed perpendicular zsd */

struct spline quadrantspline(struct coord zs, struct coord zsd,
			struct coord ze, struct coord zed) {
	return(tensespline(zs, zsd, ze, zed, bezierq, bezierq));
}

/* generate bezier curve approximating small circular segment */
/* initial point zs, final point is ze, starting tangent zd */

struct spline circleapprox(struct coord zs, struct coord ze, 
				struct coord zd) {
	struct coord zi;
	struct spline bz;
	double alpha, talpha;

	bz.zs = zs; bz.ze = ze;
	alpha = angle(zd, diff(ze, zs));
	talpha = sind(alpha) / cosd(alpha);
	zi.x = 0.5 * (zs.x + ze.x) - 0.5 * (ze.y - zs.y) * talpha; 
	zi.y = 0.5 * (zs.y + ze.y) + 0.5 * (ze.x - zs.x) * talpha;
	bz.za = weighted(zs, zi, 2.0/3.0);
	bz.zb = weighted(ze, zi, 2.0/3.0);	
	return bz;
}

/* reverse Bezier spline curve */

struct spline reversebezier(struct spline bz) {
	struct spline bzr;
	bzr.zs = bz.ze; bzr.za = bz.zb; bzr.zb = bz.za; bzr.ze = bz.zs;
	return bzr;
}

/* vector between points on two Bezier curves */

struct coord separation(struct spline bz1, struct spline bz2, 
		double t1, double t2) {
	return diff(bezier(bz1, t1), bezier(bz2, t2));
}

/* find intersection of two Bezier curves - returns t1, t2 */

struct coord crossover(struct spline bz1, struct spline bz2, 
		double t1, double t2) {
/*	double dt1, dt2; */
	struct coord t12, dz;
	struct coord mdz1, mdz2, dt12;
	int m;

	for(m = 0; m < 10000; m++) {
		dz = separation(bz1, bz2, t1, t2);
/*		if (m % 100 == 0 && m != 0) 
			printf("T1 %lg T2 %lg error (%lg %lg) ", t1, t2, dz.x, dz.y); */
		if (magnitude(dz) < epsilon) {
			t12.x = t1; t12.y = t2;
			return t12;
		}
		mdz1 = diff(separation(bz1, bz2, t1 + epsilon, t2),
					separation(bz1, bz2, t1 - epsilon, t2));
		mdz2 = diff(separation(bz1, bz2, t1, t2 + epsilon),
					separation(bz1, bz2, t1, t2 - epsilon));
		mdz1 = scale(mdz1, 1.0/(2.0 * epsilon));
		mdz2 = scale(mdz2, 1.0/(2.0 * epsilon));
		dt12 = solvepair(mdz1, mdz2, dz); 
/*		if (m % 100 == 0 && m != 0) 
			printf("DT1 %lg DT2 %lg\n", dt12.x, dt12.y); */
		t1 = t1 - dt12.x; t2 = t2 - dt12.y;

	}
	fprintf(stderr, "Too slow (crossover) %lg %lg", t1, t2);
	showcode();

	t12.x = t1; t12.y = t2;
	return t12;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* To turn elliptical pen into circular, y coordinate is stretched */
/* forward transformation also includes slanting operation */

/*
struct coord mapit(struct coord z) {	
	struct coord zd;

	zd.x = z.x + slant * z.y;
	zd.y = z.y * px / py;
	return zd;
} */

/* To turn pen nib back into elliptical shape, y coordinate is shrunk */
/* inverse transformation does not undo slanting operation */

/*
struct coord unmapit(struct coord zd) { 
	struct coord z;

	z.x = zd.x; 
	z.y = zd.y * py / px;
	return z;
} */

/* Help check whether path is continuous */

int samesub(double xa, double ya, double xb, double yb) {
	if ((fabs(xb - xa) < epsilon) && (fabs(yb - ya) < epsilon)) 
		return -1;
	else return 0;
}

/* Check whether two points are the same within tolerance */

int same(struct coord za, struct coord zb) { /* return 0 if not close */
	return samesub(za.x, za.y, zb.x, zb.y);
}

int round(double x) {
	if (x < 0) -round(-x);
	return (int) (x + 0.5);
}

void spitadobe (struct coord z) { /* unstretch and map to Adobe coord */
	double x, y;
	int xi, yi;
	
	zold = z;					/* remember where we were */
/*	z = unmapit(zd); */
	x = z.x;	y = z.y;
	if (letter_fit != 0.0) xi += letter_fit;	/* 1992/July/4 */
	xi = round((x / font_size) * 1000.0); 
	yi = round((y / font_size) * 1000.0);
	fprintf(output, "%d %d ", xi, yi);
}

void spitadobesub(double x, double y) {
	struct coord zd;
	zd.x = x; zd.y = y;
	spitadobe(zd);
}

/* make offset of length ro perpendicular to line conecting za to zb */

/*
struct coord perpen (struct coord za, struct coord zb, double ro) {
	struct coord zs;
	double dx, dy, rad;

	dx = zb.x - za.x;	dy = zb.y - za.y;
	rad = sqrt(dx * dx + dy * dy);
	if (rad == 0.0) {
		fprintf(stderr, "Zero length segment (perpen)");
		showcode();
		rad = 1.0;
	}
	zs.x = (dy / rad) * ro; zs.y = - (dx / rad) * ro;
	return zs;
} */

/* draw a circular arc - center and radius and start and end angle */
/* angles assumed to be (or made to be) from 0 to 360 */
/* assumes convex shape */
/* automatically split at extrema */

void arc(struct coord center, double ro, double alpha, double beta) {
	double theta, zeta, ca, sa, cb, sb;
	double xo, yo, xs, ys, xe, ye;

/*	printf("(%lg %lg) %lg - %lg\n", center.x, center.y, alpha, beta); 
	getch();  */

	if (alpha < 0.0) alpha = alpha + 360.0;
	if (beta < 0.0) beta = beta + 360.0;
	if (beta < alpha) beta = beta + 360.0; 
	if (alpha >= 360.0 && beta >= 360.0) {
		alpha = alpha - 360.0; beta = beta - 360.0;
	}
	if (alpha < - epsilon && beta > epsilon) {
		arc(center, ro, alpha, 0.0);
		arc(center, ro, 0.0, beta);
		return;
	}	
	if (alpha < 90.0 - epsilon && beta > 90.0 + epsilon) {
		arc(center, ro, alpha, 90.0);
		arc(center, ro, 90.0, beta);
		return;
	}
	if (alpha < 180.0 - epsilon && beta > 180.0 + epsilon) {
		arc(center, ro, alpha, 180.0);
		arc(center, ro, 180.0, beta);
		return;
	}
	if (alpha < 270.0 - epsilon && beta > 270.0 + epsilon) {
		arc(center, ro, alpha, 270.0);
		arc(center, ro, 270.0, beta);
		return;
	}	
	if (alpha < 360.0 - epsilon && beta > 360.0 + epsilon) {
		arc(center, ro, alpha, 360.0);
		arc(center, ro, 360.0, beta);
		return;
	}	

	xo = center.x; yo = center.y;
	theta = (beta - alpha) / 2.0; 
/*	zeta = (4.0/3.0) * (1.0 - cosd(theta)) / sind(theta); */
	zeta = (4.0/3.0) * sind(theta) / (1.0 + cosd(theta));	
	if (fabs((beta - alpha) - 90.0) < epsilon) zeta = bezierq;

	ca = cosd(alpha); sa = sind(alpha);
	cb = cosd(beta);  sb = sind(beta);	

	xs = xo + ro * ca; ys = yo + ro * sa;
	if (samesub(xs, ys, zold.x, zold.y) == 0) {
		fprintf(stderr, "Discontinuous (ARC) (%lg %lg) <> (%lg %lg)",
			zold.x, zold.y, xs, ys);
		showcode();
	}
	xe= xo + ro * cb; ye = yo + ro * sb;
	spitadobesub(xs - zeta * ro * sa, ys  + zeta * ro * ca); /* (xa, ya) */
	spitadobesub(xe + zeta * ro * sb, ye  - zeta * ro * cb); /* (xb, yb) */
	spitadobesub(xe, ye);
	fprintf(output, "c\n");
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* code for fitting Bezier curve when end points and direction given */
/* as well as some point on curve with known direction */

/* initial point is (xo, zo) - final point is (x3, y3) */
/* initial direction is (a,b) - final direction is (-c, -d) */
/* curve has to pass through (xm, ym) with direction (e,f) */

/* first some general purpose stuff */

/* compute point on bezier curve given ends and directions */

struct coord bezier1(struct coord zs, struct coord zsd, 
	struct coord ze, struct coord zed, double alpha, double beta, double t) {
		struct coord zt;
		double xs, xa, xb, xe, ys, ya, yb, ye;

		xs = zs.x;  ys = zs.y;  xe = ze.x;  ye = ze.y; 
		xa = xs + alpha * zsd.x; ya = ys + alpha * zsd.y;
		xb = xe - beta * zed.x;  yb = ye - beta * zed.y;
		
		zt.x = xs * (1.0 - t) * (1.0 - t) * (1.0 - t) 
			+ 3.0 * xa * t * (1.0 - t) * (1.0 - t)
				+ 3.0 * xb * t * t * (1.0 - t) + xe * t * t * t; 
		zt.y = ys * (1.0 - t) * (1.0 - t) * (1.0 - t) 
			+ 3.0 * ya * t * (1.0 - t) * (1.0 - t)
				+ 3.0 * yb * t * t * (1.0 - t) + ye * t * t * t; 
		return zt;
}

/* compute point on bezier curve given ends and directions */

/*
struct coord bezier2(struct coord zs, struct coord zsd, 
	struct coord ze, struct coord zed, double alpha, double beta, double t) {
		struct coord zt;
		double xs, ys, xe, ye;
		double xsd, ysd, xed, yed;
		xs = zs.x;   ys = zs.y;   xe = ze.x;   ye = ze.y; 
		xsd = zsd.x; ysd = zsd.y; xed = zed.x; yed = zed.y;
		zt.x = xs * (1.0 - t) * (1.0 -t) * (1 + 2.0 * t)
			+ 3 * alpha * xsd * t * (1.0 -t ) * (1.0 - t)
				- 3 * beta * xed * t * t * (1.0 - t)
					+ xe * t * t * (3.0 - 2.0 * t);
		zt.y = ys * (1.0 - t) * (1.0 -t) * (1 + 2.0 * t)
			+ 3 * alpha * ysd * t * (1.0 -t ) * (1.0 - t)
				- 3 * beta * yed * t * t * (1.0 - t)
					+ ye * t * t * (3.0 - 2.0 * t);
		return zt;
}  */

/* compute velocity of point on Bezier's curve  given ends and directions */

struct coord bezierd1(struct coord zs, struct coord zsd, 
	struct coord ze, struct coord zed, double alpha, double beta, double t) {
		struct coord zt;
		double xs, ys, xa, ya, xb, yb, xe, ye;
/*		double xsd, ysd, xed, yed; */

		xs = zs.x;	 ys = zs.y;  xe = ze.x; ye = ze.y; 
		xa = xs + alpha * zsd.x; ya = ys + alpha * zsd.y;
		xb = xe - beta * zed.x;  yb = ye - beta * zed.y;
		zt.x = 3.0 * ( - xs * (1.0 - t) * (1.0 -t)
			+ xa * (1.0 - t) * (1.0 - 3.0 * t)
				+ xb * t * (2.0 - 3.0 * t) + xe * t * t);
		zt.y = 3.0 * ( - ys * (1.0 - t) * (1.0 -t)
			+ ya * (1.0 - t) * (1.0 - 3.0 * t)
				+ yb * t * (2.0 - 3.0 * t) + ye * t * t);
		return zt;
}

/*
struct coord bezierd2(struct coord zs, struct coord zsd, 
	struct coord ze, struct coord zed, double alpha, double beta, double t) {
		struct coord zt;
		double xs, ys, xe, ye, xt, yt;
		double xsd, ysd, xed, yed;
		xs = zs.x;	 ys = zs.y; xe = ze.x; ye = ze.y; 
		xsd = zsd.x; ysd = zsd.y; xed = zed.x; yed = zed.y;
		xt = (xe - xs) * 6.0 * t * (1.0 - t)
			+ 3.0 * alpha * xsd * (1.0 - 4.0 * t + 3.0 * t * t)
				- 3.0 * beta * xed * t * (2.0 - 3.0 * t);
		yt = (ye - ys) * 6.0 * t * (1.0 - t)
			+ 3.0 * alpha * ysd * (1.0 - 4.0 * t + 3.0 * t * t)
				- 3.0 * beta * yed * t * (2.0 - 3.0 * t);
		zt.x = xt; zt.y = yt;
		return zt;
} */

/*
struct coord bezierd3(struct coord zs, struct coord zsd, 
	struct coord ze, struct coord zed, double alpha, double beta, double t) {
	struct coord z1, z2;

	z1 = bezier1(zs, zsd, ze, zed, alpha, beta, t - epsilon);
	z2 = bezier1(zs, zsd, ze, zed, alpha, beta, t + epsilon);
	return scale(diff(z2, z1), (1 / (2.0 * epsilon)));
} */

/* find where curve is tangent to given direction */
/* returns parameter t of spline - or -1.0 */

/*
double tangent(struct coord zs, struct coord zsd, 
	struct coord ze, struct coord zed, double alpha, double beta,
		struct coord zmd) {
		double t1, t2, c1, c2, c3, a, b, c, dis, dsc;
		double xs, ys, xe, ye, xmd, ymd;
		double xsd, ysd, xed, yed;
		int flag1, flag2;

		xs = zs.x;   ys = zs.y;   xe = ze.x;   ye = ze.y;
		xsd = zsd.x; ysd = zsd.y; xed = zed.x; yed = zed.y;
		xmd = zmd.x; ymd = zmd.y;
		c1 = 6.0 * (ymd * (xe - xs) - xmd * (ye - ys));
		c2 = 3.0 * alpha * (xsd * ymd - ysd * xmd);
		c3 = -3.0 * beta * (xed * ymd - yed * xmd);
		
		a = - c1 + 3.0 * (c2 - c3);
		b = c1 - 4.0 * c2 + 2.0 * c3;
		c = c2;
		
		if (a == 0.0) return (- c / b);
		if (c == 0.0) return (- b / a);
		dis = b * b - 4.0 * a * c;
		if (dis < 0) {
			fprintf(stderr, 
				"Negative discriminant (tangent) a %lg b %lg c %lg",
					a, b, c);
			showcode();
			return(-b / (2.0 * a));
		}
		dsc = sqrt(dis);
		t1 =  (- b + dsc) / (2.0 * a);
		t2 =  (- b - dsc) / (2.0 * a);
		if (t1 >= 0.0 && t1 <= 1.0) flag1 = -1; else flag1 = 0;
		if (t2 >= 0.0 && t2 <= 1.0) flag2 = -1; else flag2 = 0;
		if (flag1 != 0 && flag2 != 0) {
			fprintf(stderr, "Two tangents T1 %lg T2 %lg (tangent)", t1, t2);
			showcode();
			if (fabs(t1 - 0.5) < fabs(t2 - 0.5)) return t1;
			else return t2;
		}
		if (flag1 == 0 && flag2 == 0) {
			fprintf(stderr, "Not tangents T1 %lg T2 %lg (tangent)", t1, t2);
			return(-b / (2.0 * a));
		}
		if (flag1 != 0) return t1;	else return t2;
}  */

double tangent(struct coord zs, struct coord zsd, 
	struct coord ze, struct coord zed, double alpha, double beta,
		struct coord zmd) {
		double ax, ay, bx, by, cx, cy, a, b, c;
		double t1, t2, dis, dsc;
		double xs, ys, xa, ya, xb, yb, xe, ye, xmd, ymd;
		int flag1, flag2;

		xs = zs.x;   ys = zs.y;   xe = ze.x;   ye = ze.y;
		xa = xs + alpha * zsd.x; ya = ys + alpha * zsd.y;
		xb = xe - beta * zed.x;  yb = ye - beta * zed.y;		
		xmd = zmd.x;  ymd = zmd.y;

		ax = (xe - xs) + 3.0 * (xa - xb);
		ay = (ye - ys) + 3.0 * (ya - yb);
		
		bx = 2.0 * (xs - 2.0 * xa + xb);
		by = 2.0 * (ys - 2.0 * ya + yb);
		
		cx = xa - xs; cy = ya - ys;
		
		a = ax * ymd - ay * xmd;
		b = bx * ymd - by * xmd;		
		c = cx * ymd - cy * xmd;

		if (a < 0.0) {
			a = - a; b = -b; c = - c;
		}

/*		printf("a %lg b %lg c %lg\n", a, b, c);	getch(); */

		if (a == 0.0) return (- c / b);
		if (c == 0.0) return (- b / a);
		dis = b * b - 4.0 * a * c;
		if (dis < 0) {
			fprintf(stderr, 
				"Negative discriminant (tangent) a %lg b %lg c %lg\n",
					a, b, c);
				showcode();
			return(-b / (2.0 * a));
		}
		dsc = sqrt(dis);
		if (b < 0.0) {
			t1 =  (dsc - b) / (2.0 * a);
			t2 =  (2.0 * c) / (dsc - b);
		}
		else {
			t1 = - (2.0 * c) / (b + dsc);
			t2 = - (b + dsc) / (2.0 * a);
		}
/*		printf("b %lg dsc %lg t1 %lg t2 %lg\n", b, dsc, t1, t2); */

		if (t1 >= 0.0 && t1 <= 1.0) flag1 = -1; else flag1 = 0;
		if (t2 >= 0.0 && t2 <= 1.0) flag2 = -1; else flag2 = 0;
		if (flag1 != 0 && flag2 != 0) {
			fprintf(stderr, "Two tangents (tangent) T1 %lg T2 %lg", t1, t2);
			showcode();
			if (fabs(t1 - 0.5) < fabs(t2 - 0.5)) return t1;
			else return t2;
/*			return(-b / (2.0 * a)); */
		}
		if (flag1 == 0 && flag2 == 0) {
			fprintf(stderr, "No tangents (tangent) T1 %lg T2 %lg\n", t1, t2);
			showcode();
			return(-b / (2.0 * a));
		}
		if (flag1 != 0) return t1;	else return t2;
} 

/* return coordinates of point on Bezier curve where it is tangent to zmd */

struct coord tangentp(struct coord zs, struct coord zsd,
		struct coord ze, struct coord zed, double alpha, double beta, 
			struct coord zmd) {
	return bezier1(zs, zsd, ze, zed, alpha, beta, 
		tangent(zs, zsd, ze, zed, alpha, beta, zmd));
} 

/* come in with current estimates of alpha and beta */
/* estimate new parameters given guess */

struct coord newpar(struct coord zs, struct coord zsd,
		struct coord zm, struct coord zmd,
	struct coord ze, struct coord zed, double alpha, double beta) {

	struct coord zo, dz, dzmda, dzmdb, dpar, par;
	zo = tangentp(zs, zsd, ze, zed, alpha, beta, zmd);
/*	if (traceflag != 0) printf("ZO (%lg %lg) ", zo.x, zo.y); */
	dz = diff(zm, zo);
/*	if (traceflag != 0) printf("DZ (%lg %lg) ", dz.x, dz.y); */
	dzmda = diff(tangentp(zs, zsd, ze, zed, alpha + epsilon, beta, zmd),
		tangentp(zs, zsd, ze, zed, alpha - epsilon, beta, zmd));
	dzmdb = diff(tangentp(zs, zsd, ze, zed, alpha, beta + epsilon, zmd),
		tangentp(zs, zsd, ze, zed, alpha, beta - epsilon, zmd));
	dzmda = scale(dzmda, 1 / (2.0 * epsilon));
	dzmdb = scale(dzmdb, 1 / (2.0 * epsilon));	
/*	if (traceflag != 0) printf("DZMDA (%lg %lg) ", dzmda.x, dzmda.y); */
/*	if (traceflag != 0) printf("DZMDB (%lg %lg) ", dzmdb.x, dzmdb.y); */
	dpar = solvepair(dzmda, dzmdb, dz);
/*	if (traceflag != 0) printf("DPAR (%lg %lg) ", dpar.x, dpar.y); */
	par.x = alpha; par.y = beta;
	return sum(par, dpar);
}

/* find parameters that make Bezier curve fit ends, point on curve */
/* and direction at ends and direction at point on curve */

struct coord findpar(struct coord zs, struct coord zsd,
		struct coord zm, struct coord zmd,
	struct coord ze, struct coord zed, double alpha, double beta) {

	struct coord par, zo, dz, znd;
	double alphan, betan, adz, adzn, t;
	int m;

	t = tangent(zs, zsd, ze, zed, alpha, beta, zmd);
	znd = bezierd1(zs, zsd, ze, zed, alpha, beta, t);
/*	printf("ZMD slope (%lg) T %lg ZND slope (%lg)\n", 
		zmd.y / zmd.x, t, znd.y / znd.x);
	getch(); */

	zo = bezier1(zs, zsd, ze, zed, alpha, beta, t);
	dz = diff(zm, zo);
	adz = magnitude(dz);

	for(m = 0; m < 100; m++) {
/*		if (traceflag != 0) printf("\nALPHA %lg BETA %lg ", alpha, beta); */
		par = newpar(zs, zsd, zm, zmd, ze, zed, alpha, beta);
		alphan = par.x; betan = par.y;
/*		limit step size */
		if (alphan - alpha > maxstep) alphan = alpha + maxstep;
		if (alpha - alphan > maxstep) alphan = alpha - maxstep;
		if (betan - beta > maxstep) betan = beta + maxstep;
		if (beta - betan > maxstep) betan = beta - maxstep;		

		t = tangent(zs, zsd, ze, zed, alphan, betan, zmd);
		znd = bezierd1(zs, zsd, ze, zed, alpha, beta, t);
/*		if (traceflag != 0) printf("ZMD slope (%lg) T %lg ZND slope (%lg)\n", 
			zmd.y / zmd.x, t, znd.y / znd.x);
		getch(); */
		zo = bezier1(zs, zsd, ze, zed, alphan, betan, t);
		dz = diff(zm, zo);
		adzn = magnitude(dz);
		if (adzn > adz) {
			fprintf(stderr, 
				"Large step (findpar) alpha beta (%lg %lg) => (%lg %lg) ",
				alpha, beta, alphan, betan);
			showcode();
			alphan = alpha + 0.1 * (alphan - alpha);
			betan = beta + 0.1 * (betan - beta);
			t = tangent(zs, zsd, ze, zed, alphan, betan, zmd);
			zo = bezier1(zs, zsd, ze, zed, alphan, betan, t);
			dz = diff(zm, zo);
			adzn = magnitude(dz);
		}
		alpha = alphan; beta = betan; adz = adzn;

		if (adzn < epsilon) {
/*		if (traceflag != 0)	printf("ALPHA %lg BETA %lg\n", alpha, beta); */
			par.x = alpha; par.y = beta;
			return(par);
		}
	}
	fprintf(stderr, "Too slow (findpar) alpha %lg beta %lg", alpha, beta);
	showcode();
	par.x = alpha; par.y = beta;
	return(par);
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* see if Bezier curve contains an extremum */
/* return parameter t where it occurs - or -1 */
/* used for splitting curvetos */

/* presently not used 

double extreme(double zs, double za, double zb, double ze) {
	double a, b, c, dis, dsc, t1, t2, zmin, zmax;

	if (splitsuppress != 0) return -1;
	if (ze >= zs) {
		zmin = zs; zmax = ze;
	}
	else {
		zmin = ze; zmax = zs;
	}
	if (za >= zmin && za <= zmax && zb >= zmin && zb <= zmax) {
		return -1;
	}

	a = (zs - ze) + 3.0 * (zb - za);
	b = -2.0 * (zs - 2.0 * za + zb);
	c = (zs - za);

	if (a == 0.0) {
		t1 = - c / b;
		if (t1 > 0.0 && t1 < 1.0) return t1;
		else return -1;
	}
	if (c == 0.0) {
		t1 = - b / a;
		if (t1 > 0.0 && t1 < 1.0) return t1;
		else return -1;
	}
	if (traceflag != 0) printf("a %lg b %lg c %lg ", a, b, c); 
	dis = b * b - 4.0 * a * c;
	if (dis < 0.0) return -1.0;	
	dsc = sqrt(dis);
	t1 = (- b + dsc) / (2.0 * a);
	t2 = (- b - dsc) / (2.0 * a);
	if (traceflag != 0) printf("t1 %lg t2 %lg\n", t1, t2); 
	if ((t1 > epsilon) && (t1 < (1.0 - epsilon))) return t1;
	if ((t2 > epsilon) && (t2 < (1.0 - epsilon))) return t2;
	return -1;
} 

double vertical(struct coord zs, struct coord za, 
		struct coord zb, struct coord ze) {
	return extreme(zs.x, za.x, zb.x, ze.x);
}

double horizontal(struct coord zs, struct coord za, 
		struct coord zb, struct coord ze) {
	return extreme(zs.y, za.y, zb.y, ze.y);
} 

void curveto(struct coord z0, struct coord z1, 
		struct coord z2, struct coord z3);

void splitsplinesub(struct coord zs, struct coord za, 
				struct coord zb, struct coord ze, double to) {
		struct coord zs1, za1, zb1, ze1, zs2, za2, zb2, ze2;
		double a, b, c, d, an, bn, cn, dn, tn;
		zs1 = zs; 
		ze1 = beziersub(zs, za, zb, ze, to);
		zs2 = ze1; 
		ze2 = ze;
		tn = 1.0 - to;
		
		a = (ze.x - zs.x) - 3.0 * (zb.x - za.x);
		b = 3.0 * (zb.x - 2.0 * za.x + zs.x);
		c = 3.0 * (za.x - zs.x);
		d = zs.x;
		an = a * to * to * to;
		bn = b * to * to;
		cn = c * to;
		dn = d;
		zs1.x = dn;
		za1.x = dn + cn / 3.0;
		zb1.x = dn + (2.0 * cn  + bn) / 3.0;
		ze1.x = dn + cn + bn + an;

		an = a * tn * tn * tn;
		bn = (3.0 * a * to + b) * tn * tn;
		cn = (to * (3.0 * a * to + 2.0 * b) + c) * tn;
		dn = to * ( to * (to * a + b) + c) + d;
		zs2.x = dn;
		za2.x = dn + cn / 3.0;
		zb2.x = dn + (2.0 * cn + bn) / 3.0;
		ze2.x = dn + cn + bn + an;
		
		a = (ze.y - zs.y) - 3.0 * (zb.y - za.y);
		b = 3.0 * (zb.y - 2.0 * za.y + zs.y);
		c = 3.0 * (za.y - zs.y);
		d = zs.y;
		an = a * to * to * to;
		bn = b * to * to;
		cn = c * to;
		dn = d;
		zs1.y = dn;
		za1.y = dn + cn / 3.0;
		zb1.y = dn + (2.0 * cn  + bn) / 3.0;
		ze1.y = dn + cn + bn + an;

		an = a * tn * tn * tn;
		bn = (3.0 * a * to + b) * tn * tn;
		cn = (to * (3.0 * a * to + 2.0 * b) + c) * tn;
		dn = to * ( to * (to * a + b) + c) + d;
		zs2.y = dn;
		za2.y = dn + cn / 3.0;
		zb2.y = dn + (2.0 * cn + bn) / 3.0;
		ze2.y = dn + cn + bn + an;

		if (traceflag != 0) {
		printf("zs (%lg %lg) za (%lg %lg) zb (%lg %lg) ze (%lg %lg)\n",
			zs.x, zs.y, za.x, za.y, zb.x, zb.y, ze.x, ze.y);
		printf("zs1 (%lg %lg) za1 (%lg %lg) zb1 (%lg %lg) ze1 (%lg %lg)\n",
			zs1.x, zs1.y, za1.x, za1.y, zb1.x, zb1.y, ze1.x, ze1.y);
		printf("zs2 (%lg %lg) za2 (%lg %lg) zb2 (%lg %lg) ze2 (%lg %lg)\n",
			zs2.x, zs2.y, za2.x, za2.y, zb2.x, zb2.y, ze2.x, ze2.y);
		getch();
		}
		curveto(zs1, za1, zb1, ze1);
		curveto(zs2, za2, zb2, ze2);
} */

/* compute sub-spline from 0 to t */

struct spline splitfirst(struct spline bz, double to) {
	struct coord zs, za, zb, ze;
	struct coord zs1, za1, zb1, ze1, zs2, ze2;
	double a, b, c, d, an, bn, cn, dn, tn;
	struct spline bzd;
	zs = bz.zs; za = bz.za, zb = bz.zb, ze = bz.ze;
	zs1 = zs; 
	ze1 = beziersub(zs, za, zb, ze, to);
	zs2 = ze1; 
	ze2 = ze;
	tn = 1.0 - to;
		
	a = (ze.x - zs.x) - 3.0 * (zb.x - za.x);
	b = 3.0 * (zb.x - 2.0 * za.x + zs.x);
	c = 3.0 * (za.x - zs.x);
	d = zs.x;
	an = a * to * to * to;
	bn = b * to * to;
	cn = c * to;
	dn = d;
	zs1.x = dn;
	za1.x = dn + cn / 3.0;
	zb1.x = dn + (2.0 * cn  + bn) / 3.0;
	ze1.x = dn + cn + bn + an;
	
	a = (ze.y - zs.y) - 3.0 * (zb.y - za.y);
	b = 3.0 * (zb.y - 2.0 * za.y + zs.y);
	c = 3.0 * (za.y - zs.y);
	d = zs.y;
	an = a * to * to * to;
	bn = b * to * to;
	cn = c * to;
	dn = d;
	zs1.y = dn;
	za1.y = dn + cn / 3.0;
	zb1.y = dn + (2.0 * cn  + bn) / 3.0;
	ze1.y = dn + cn + bn + an;

/*	if (traceflag != 0) {
	printf("zs (%lg %lg) za (%lg %lg) zb (%lg %lg) ze (%lg %lg)\n",
		zs.x, zs.y, za.x, za.y, zb.x, zb.y, ze.x, ze.y);
	printf("zs1 (%lg %lg) za1 (%lg %lg) zb1 (%lg %lg) ze1 (%lg %lg)\n",
		zs1.x, zs1.y, za1.x, za1.y, zb1.x, zb1.y, ze1.x, ze1.y);
	getch();
	} */

	bzd.zs = zs1;
	bzd.za = za1;
	bzd.zb = zb1;
	bzd.ze = ze1;
	return bzd;
}

/* compute sub-spline from t to 1 */

struct spline splitlast(struct spline bz, double to) {
	struct coord zs, za, zb, ze;
	struct coord zs1, ze1, zs2, za2, zb2, ze2;
	double a, b, c, d, an, bn, cn, dn, tn;
	struct spline bzd;
	zs = bz.zs; za = bz.za, zb = bz.zb, ze = bz.ze;
	zs1 = zs; 
	ze1 = beziersub(zs, za, zb, ze, to);
	zs2 = ze1; 
	ze2 = ze;
	tn = 1.0 - to;
		
	a = (ze.x - zs.x) - 3.0 * (zb.x - za.x);
	b = 3.0 * (zb.x - 2.0 * za.x + zs.x);
	c = 3.0 * (za.x - zs.x);
	d = zs.x;
	
	an = a * tn * tn * tn;
	bn = (3.0 * a * to + b) * tn * tn;
	cn = (to * (3.0 * a * to + 2.0 * b) + c) * tn;
	dn = to * ( to * (to * a + b) + c) + d;
	zs2.x = dn;
	za2.x = dn + cn / 3.0;
	zb2.x = dn + (2.0 * cn + bn) / 3.0;
	ze2.x = dn + cn + bn + an; 
		
	a = (ze.y - zs.y) - 3.0 * (zb.y - za.y);
	b = 3.0 * (zb.y - 2.0 * za.y + zs.y);
	c = 3.0 * (za.y - zs.y);
	d = zs.y;

	an = a * tn * tn * tn;
	bn = (3.0 * a * to + b) * tn * tn;
	cn = (to * (3.0 * a * to + 2.0 * b) + c) * tn;
	dn = to * ( to * (to * a + b) + c) + d;
	zs2.y = dn;
	za2.y = dn + cn / 3.0;
	zb2.y = dn + (2.0 * cn + bn) / 3.0;
	ze2.y = dn + cn + bn + an; 

/*	if (traceflag != 0) {
	printf("zs (%lg %lg) za (%lg %lg) zb (%lg %lg) ze (%lg %lg)\n",
		zs.x, zs.y, za.x, za.y, zb.x, zb.y, ze.x, ze.y);
	printf("zs2 (%lg %lg) za2 (%lg %lg) zb2 (%lg %lg) ze2 (%lg %lg)\n",
		zs2.x, zs2.y, za2.x, za2.y, zb2.x, zb2.y, ze2.x, ze2.y); 
	getch();
	} */

	bzd.zs = zs2;
	bzd.za = za2;
	bzd.zb = zb2;
	bzd.ze = ze2;
	return bzd;
} 

/*
void fitcurve(struct coord zs, struct coord zsd,
	struct coord ze, struct coord zed, double alpha, double beta,
		struct coord zm, struct coord zmd) {
	struct coord par, z0, z1, z2, z3;
	z0 = zs; z3 = ze;
	par = findpar(zs, zsd, zm, zmd, ze, zed, alpha, beta);
	alpha = par.x; beta = par.y;

	z1 = sum(zs, scale(zsd, alpha));
	z2 = diff(ze, scale(zed, beta));
	curveto(z0, z1, z2, z3);

} */

/* make bezier curve given end points and direction as well as */
/* point on the curve and direction there */
/* alpha and beta are just initial guesses */

struct spline makecurve(struct coord zs, struct coord zsd,
		struct coord zm, struct coord zmd,
	struct coord ze, struct coord zed, double alpha, double beta) {

	struct coord par;
	struct spline bz;

	par = findpar(zs, zsd, zm, zmd, ze, zed, alpha, beta);
	alpha = par.x; beta = par.y;
/*	printf("ALPHA %lg BETA %lg\n", alpha, beta);	getch(); */

	bz.zs = zs; 
	bz.za = sum(zs, scale(zsd, alpha));
	bz.zb = diff(ze, scale(zed, beta));
	bz.ze = ze;
	return bz;
} 
	
/* spline from zs to ze through zm, parallel to (ze - zs) at zm */

struct spline flexspine(struct coord zs, struct coord zsd,
			struct coord ze, struct coord zed, struct coord zm) {
	return makecurve(zs, zsd, zm, diff(ze, zs), ze, zed, 1.0, 1.0);
}

/* compute offset from a curve perpendicular to given direction */

struct coord offsetdir(struct coord za, struct coord zd, double ro) {
	double dx, dy, rad;

	dx = zd.x;	dy = zd.y;
	rad = sqrt(dx * dx + dy * dy);
	if (rad == 0.0) {
		fprintf(stderr, "Zero length direction (offsetdir)");
		showcode();
		rad = 1.0;	/* kludge, just to go on ? */
	}
	zd.x = (dy / rad) * ro; zd.y = - (dx / rad) * ro;	
	return sum(za, zd);
}

/* try to offset bezier curve by offset ro perpendicular to curve */

struct spline offsetbezier(struct spline bz, double ro) {
	struct coord zs, za, zb, ze, zm;
	struct coord zmd, zsd, zed;
	struct coord zns, znm, zne;

	zs = bz.zs, za = bz.za, zb = bz.zb, ze = bz.ze;
/* printf("ZS (%lg %lg) ZA (%lg %lg) ZB (%lg %lg) ZE (%lg %lg)\n",
		zs.x, zs.y, za.x, za.y, zb.x, zb.y, ze.x, ze.y); */
/*	zsd = bezierd(bz, 0.0); */
	zsd = diff(za, zs);
	zns = offsetdir(zs, zsd, ro);
	zm = bezier(bz, 0.5);
	zmd = bezierd(bz, 0.5);
	znm = offsetdir(zm, zmd, ro);
/*	zed = bezierd(bz, 1.0); */
	zed = diff(ze, zb);
	zne = offsetdir(ze, zed, ro);
/* printf("ZS (%lg %lg) ZSD (%lg %lg) ZM (%lg %lg) ZMD (%lg %lg) ZE (%lg %lg) ZED (%lg %lg)\n",
	zns.x, zns.y, zsd.x, zsd.y, zm.x, zm.y, zmd.x, zmd.y, zne.x, zne.y,
		zed.x, zed.y); */
/*	bz.za = sum(zns, diff(bz.za, bz.zs));
	bz.zs = zns;
	bz.zb = sum(zne, diff(bz.zb, bz.ze));
	bz.ze = zne;
	return bz; */
	return makecurve(zns, zsd, znm, zmd, zne, zed, 1.0, 1.0);
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* crude iterative intersection of Bezier curve and line */
/* Bezier curve given by four control points */
/* line given by two points on it */
/* conoverted to parameters a * x + b * y + c = 0 */
/* initial guess of parameter is t - final guess is returned */
/* returns -1 if does not cross with 0 < t < 1 */

/*
double intersectcurve(struct spline bz, struct coord zn, struct coord zm, 
						double t){
	struct coord zt, ztd;
	double a, b, c, ft, ftd, dt, fto;
	int m;

	a = zm.y - zn.y; 
	b = zn.x - zm.x;
	c = zm.x * zn.y - zn.x * zm.y;
	
	zt = bezier(bz, t);
	ztd = bezierd(bz, t);	
	ft = a * zt.x + b * zt.y + c;
	ftd = a * ztd.x + b * ztd.y + c;
		
	for(m = 0; m < 1000; m++) {
		if (fabs(ft) < epsilon) return t;
		fto = ft;					
		dt = - ft/ftd;				
		if (t + dt > 1.0) dt = (1.0 - t) / 2.0;
		if (t + dt < 0) dt = - t / 2.0;
		zt = bezier(bz, t + dt);
 		ztd = bezierd(bz, t+ dt);
		ft = a * zt.x + b * zt.y + c;
		ftd = a * ztd.x + b * ztd.y + c;
		while (fabs(ft) > fabs(fto)) {
			dt = dt/2.0;
			zt = bezier(bz, t + dt);
			ztd = bezierd(bz, t + dt);	
			ft = a * zt.x + b * zt.y + c;
			ftd = a * ztd.x + b * ztd.y + c;
			if (m++ > 1000) break;
		}
		t = t + dt;
	}
	fprintf(stderr, "Too slow (intersectcurve) t %lg", t);
	showcode();
	return t;
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* implement moveto, lineto, curveto and closepath */

void moveto(struct coord z) {
	if (same(zold, zbad) == 0) {
		fprintf(stderr, "Moveto before closepath");
		showcode();
	}
	zstart = z;			/* remember start of closed outline */
	spitadobe(z);
	fprintf(output, "m\n");
}

void linetosub(struct coord ze) { /* no checking */
	spitadobe(ze);	
	fprintf(output, "l\n");
}

void lineto(struct coord zs, struct coord ze) {
	
	if (same(zs, zold) == 0) {
		fprintf(stderr, "Discontinuous (L) (%lg %lg) <> (%lg %lg)",
			zold.x, zold.y, zs.x, zs.y);
		showcode();
	}
	linetosub(ze);
}

void curveto(struct coord z0, struct coord z1, 
		struct coord z2, struct coord z3) {
/*	struct coord zt; */
/*	double t; */
		
	if (same(zold, z0) == 0) {
		fprintf(stderr, "Discontinuous (C) (%lg %lg) <> (%lg %lg)",
			zold.x, zold.y, z0.x, z0.y);
		showcode();
	}
/*	if (((t = horizontal(z0, z1, z2, z3)) > 0.0) ||
		((t = vertical(z0, z1, z2, z3)) > 0.0)) {
				if (traceflag != 0) { 
					zt = bezierdsub(z0, z1, z2, z3, t);
					printf("At t = %lg tangent (%lg %lg)\n", t, zt.x, zt.y);
				} 
				splitsplinesub(z0, z1, z2, z3, t);
	}
	else { */
		spitadobe(z1);	spitadobe(z2); spitadobe(z3);
		fprintf(output, "c\n");
/*	} */
} 

/* spit out bezier curveto */

void showbezier(struct spline bz) {
	curveto(bz.zs, bz.za, bz.zb, bz.ze);
} 

void printbezier(struct spline bz) {
	printf("BEZIER (%lg %lg)  (%lg %lg)  (%lg %lg)  (%lg %lg) \n",
		bz.zs.x, bz.zs.y, bz.za.x, bz.za.y, 
			bz.zb.x, bz.zb.y, bz.ze.x, bz.ze.y); 
} 

void closepath(void) {
	if (same(zold, zstart) == 0) {	/* check if closed */
		fprintf(stderr, "Path not closed (%lg %lg) (%lg %lg)",
			zold.x, zold.y, zstart.x, zstart.y);
		showcode();
		linetosub(zstart);
	}
	fprintf(output, "h\n");	 /*  fprintf(output, "cp\n");	 */
	zold = zbad;			 /*  currentpoint unknown	*/ 
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* lineto parallel to za to zb, but offset by ro perpendicular to right */

/* near end of offset lineto */

struct coord offsetnear(struct coord za, struct coord zb, double ro) {
	struct coord zd;
	double dx, dy, rad;

	dx = zb.x - za.x;	dy = zb.y - za.y;
	rad = sqrt(dx * dx + dy * dy);
	if (rad == 0.0) {
		fprintf(stderr, "Zero length segment (offsetnear)");
		showcode();
		rad = 1.0; /* kludge, just to go on ? */
	}
	zd.x = (dy / rad) * ro; zd.y = - (dx / rad) * ro;	
	return sum(za, zd);
}

/* far end of offset lineto */

struct coord offsetfar(struct coord za, struct coord zb, double ro) {
	struct coord zd;
	double dx, dy, rad;

	dx = zb.x - za.x;	dy = zb.y - za.y;
	rad = sqrt(dx * dx + dy * dy);
	if (rad == 0.0) {
		fprintf(stderr, "Zero length segment (offsetfar)");
		showcode();
		rad = 1.0; /* kludge, just to go on ? */
	}
	zd.x = (dy / rad) * ro; zd.y = - (dx / rad) * ro;	
	return sum(zb, zd);
}

/*
void offsetlineto(struct coord za, struct coord zb, double ro) {
	struct coord zd;
	double dx, dy, rad;

	dx = zb.x - za.x;	dy = zb.y - za.y;
	rad = sqrt(dx * dx + dy * dy);
	if (rad == 0.0) {
		fprintf(stderr, "Zero length segment (offsetlineto");
		showcode();
		rad = 1.0;
	}
	zd.x = (dy / rad) * ro; zd.y = - (dx / rad) * ro;	
	lineto(sum(za, zd), sum(zb, zd));
} */

void offsetlineto(struct coord za, struct coord zb, double ro) {
	lineto(offsetnear(za, zb, ro), offsetfar(za, zb, ro));
}

/* intersect two lines offset by given amount from given lines */

/*
struct coord offsetintersect(struct coord za, struct coord zb, 
	struct coord zc, struct coord zd, double ro) {
	struct coord zd1, zd2;
	double dx1, dy1, rad1, dx2, dy2, rad2;

	dx1 = zb.x - za.x;	dy1 = zb.y - za.y;
	rad1 = sqrt(dx1 * dx1 + dy1 * dy1);
	if (rad1 == 0.0) {
		fprintf(stderr, "Zero length segment (offsetintersect)");
		showcode();
		rad1 = 1.0;
	}
	zd1.x = (dy1 / rad1) * ro; zd1.y = - (dx1 / rad1) * ro;	

	dx2 = zd.x - zc.x;	dy2 = zd.y - zc.y;
	rad2 = sqrt(dx2 * dx2 + dy2 * dy2);
	if (rad2 == 0.0) {
		fprintf(stderr, "Zero length segment (offsetintersect)");
		showcode();
		rad2 = 1.0;
	}
	zd2.x = (dy2 / rad2) * ro; zd2.y = - (dx2 / rad2) * ro;	

	return intersectsub(sum(za, zd1), sum(zb, zd1), 
		sum(zc, zd2), sum(zd, zd2));
} */

struct coord offsetintersect(struct coord za, struct coord zb, 
	struct coord zc, struct coord zd, double ro) {
	
	return intersectsub(offsetnear(za, zb, ro), offsetfar(za, zb, ro),
		offsetnear(zc, zd, ro), offsetfar(zc, zd, ro));
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* draw an arc around zb to lineup with offset line from za to zb */
/* and offset line from zb to zc */

void arcbetween(struct coord za, struct coord zb, struct coord zc,
		double ro) {
		arc(zb, ro, direction(za, zb) - 90.0, direction(zb, zc) - 90.0);
}

/* draw an arc about zo connection za to zb */

void arcconnect(struct coord za, struct coord zo, struct coord zb) {
	double ra, rb;
	ra = magnitude(diff(za, zo)); 	rb = magnitude(diff(zb, zo));
	if (fabs(ra - rb) > epsilon) {
		fprintf(stderr, "Radii unequal (arcconnect) %lg <> %lg", ra, rb);
		showcode();
	}
	arc(zo, sqrt(ra * rb), direction(zo, za), direction(zo, zb));
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* start up code called for each character in font */

void beginchar(int k, double width, double height, double depth) {
	double fwidth;
	width += letter_fit * 2;					/* 1992/July/4 */ 
	code = k;
	w = width;
	h = height;
	d = depth;
	fwidth = (w / font_size) * 1000.0;
	fprintf(output, "] \n");
	fprintf(output, "%d %d\n", code, round(fwidth));
	fprintf(output, "%% a%d\n", code);
	if (afmtrace != 0)  {
		fwidth = ((double) ((long)(fwidth * 1000.0 + 0.5))) / 1000.0;
		printf("C %d ; WX %lg ; N a%d ;\n", code, fwidth, code);
	}
/*	if (traceflag != 0) printf("%c ", code); */
/*  could use bounding box here ? */
}

/* def v_center(expr h_sharp) =
 .5h_sharp+math_axis#, .5h_sharp-math_axis# enddef; */

/* begin character vertically centered about math_axis */

void begincenter(int k, double width, double h_sharp) {
	beginchar(k, width, 0.5 * h_sharp + math_axis, 0.5 * h_sharp - math_axis);
}

/* reflect about vertical center line */

struct coord reflect(struct coord z) {
	struct coord zr;
	zr.x = w - z.x; zr.y = z.y;
	return zr;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* finally: actual character definitions for lasy */

void lasy_square(void) { /* 062 "Square" */
	struct coord z1, z2, z3, z4;
	double a;

/*	beginchar(50, 2.1 * math_axis + 4.0 * u, 
		0.5 * (7.0 * u) + math_axis, 0.5 * (7.0 * u) - math_axis); */
	
	a = 2.1 * math_axis;

	begincenter(50, a + 4.0 * u, 7.0 * u);

/*	printf("font_size %lg, width %lg, height %lg, ro %lg\n",
		font_size, h, w, ro);
	printf("math_axis %lg, unit %lg, a %lg\n", math_axis, u, a); */

	z1.x = 2.0 * u; z4.x = z1.x;
	z2.x = z1.x + a; z3.x = z2.x;
	z3.y = 0.5 * (cap_height - a); z4.y = z3.y;
	z1.y = z3.y + a; z2.y = z1.y;

	moveto(offsetnear(z4, z3, ro));
	offsetlineto(z4, z3, ro);
	arcbetween(z4, z3, z2, ro);
	offsetlineto(z3, z2, ro);
	arcbetween(z3, z2, z1, ro);
	offsetlineto(z2, z1, ro);
	arcbetween(z2, z1, z4, ro);
	offsetlineto(z1, z4, ro);
	arcbetween(z1, z4, z3, ro);
	closepath();
	moveto(offsetintersect(z3, z4, z4, z1, ro));
	linetosub(offsetintersect(z4, z1, z1, z2, ro));
	linetosub(offsetintersect(z1, z2, z2, z3, ro));
	linetosub(offsetintersect(z2, z3, z3, z4, ro));
	linetosub(offsetintersect(z3, z4, z4, z1, ro));
	closepath();
}

void lasy_diamond(void) { /* 063 "Diamond" */
	struct coord z2, z4, z6, z8;
	double a;

/*	beginchar(51, 2.0 * (0.85 * asc_height - math_axis) + 2.0 * u,
		0.5 * (7.0 * u) + math_axis, 0.5 * (7.0 * u) - math_axis); */

	a = 0.85 * asc_height - math_axis;

	begincenter(51, 2.0 * a + 2.0 * u, 7.0 * u);

/*	printf("font_size %lg, width %lg, height %lg, ro %lg\n",
		font_size, h, w, ro);
	printf("math_axis %lg, unit %lg, a %lg\n", math_axis, u, a); */

	z4.x = 0.5 * w; z8.x = z4.x;
	z2.x = z4.x + (a - ro); z6.x = w - z4.x - (a - ro);
	z2.y = 0.5 * cap_height; z6.y = z2.y;
	z8.y = z2.y + (a - ro); z4.y =  cap_height - z8.y;

	moveto(offsetnear(z4, z2, ro));
	offsetlineto(z4, z2, ro);
	arcbetween(z4, z2, z8, ro);
	offsetlineto(z2, z8, ro);
	arcbetween(z2, z8, z6, ro);
	offsetlineto(z8, z6, ro);
	arcbetween(z8, z6, z4, ro);
	offsetlineto(z6, z4, ro);
	arcbetween(z6, z4, z2, ro); 
	closepath();
	moveto(offsetintersect(z2, z4, z4, z6, ro));
	linetosub(offsetintersect(z4, z6, z6, z8, ro));
	linetosub(offsetintersect(z6, z8, z8, z2, ro));	
	linetosub(offsetintersect(z8, z2, z2, z4, ro));	
	linetosub(offsetintersect(z2, z4, z4, z6, ro));	
	closepath(); 
}

void lasy_bowtie(void) { /* 061 "Bowtie" */
	struct coord z1, z2, z3, z4;
	double a;

/*	beginchar(49, 13.0 * u,
		0.5 * (7.0 * u) + math_axis, 0.5 * (7.0 * u) - math_axis); */
	
	begincenter(49, 13.0 * u, 7.0 * u);

	a = 1.1 * math_axis;

/*	printf("font_size %lg, width %lg, height %lg, ro %lg\n",
		font_size, h, w, ro);
	printf("math_axis %lg, unit %lg, a %lg\n", math_axis, u, a); */

	z1.x = 1.5 * u; z4.x = z1.x;
	z2.x = w - z1.x; z3.x = z2.x;
	z1.y = 0.5 * (cap_height - 2.0 * a); z2.y = z1.y;
	z3.y = z1.y + 2.0 * a; z4.y = z3.y;
	
/*	ro = 4.0 * ro; */  /* debugging */

	moveto(offsetnear(z2, z3, ro));
	offsetlineto(z2, z3, ro);
	arcbetween(z2, z3, z1, ro);
	linetosub(offsetintersect(z3, z1, z2, z4, ro));
	linetosub(offsetfar(z2, z4, ro));
	arcbetween(z2, z4, z1, ro);
	offsetlineto(z4, z1, ro);
	arcbetween(z4, z1, z3, ro);
	linetosub(offsetintersect(z1, z3, z4, z2, ro));
	linetosub(offsetfar(z4, z2, ro));
	arcbetween(z4, z2, z3, ro);
	closepath();
	moveto(offsetintersect(z3, z1, z1, z4, ro));
	linetosub(offsetintersect(z1, z4, z4, z2, ro));
	linetosub(offsetintersect(z4, z2, z3, z1, ro));
	linetosub(offsetintersect(z3, z1, z1, z4, ro));
	closepath();
	moveto(offsetintersect(z1, z3, z3, z2, ro));
	linetosub(offsetintersect(z3, z2, z2, z4, ro));
	linetosub(offsetintersect(z2, z4, z1, z3, ro));
	linetosub(offsetintersect(z1, z3, z3, z2, ro));
	closepath();
}

/* def compute_spread(expr normal_spread,big_spread)=
 spread#:=math_spread[normal_spread,big_spread];
 spread:=ceiling(spread#*hppp)+eps; enddef; */

double compute_spread(double normal_spread, double big_spread) {
	return (math_spread * big_spread + (1.0 - math_spread) * normal_spread);
}

void lasy_subset(void) { /* 074 "square subset" */
	struct coord z1, z2, z3, z4;
	double spread;

	spread = compute_spread(5.0/4.0 * x_height, 3.0/2.0 * x_height);
	
/*	beginchar(60, 14.0 * u,
		0.5 * (spread+rule_thickness) + math_axis, 
			0.5 * (spread+rule_thickness) - math_axis); */

	begincenter(60, 14.0 * u, spread + rule_thickness);

	z2.x = 1.5 * u + oo + ro; z3.x = z2.x;
	z1.x = w - 1.5 * u; z4.x = z1.x;
	z1.y = math_axis + 0.5 * spread; z2.y = z1.y;
	z4.y = z1.y - spread; z3.y = z4.y;

	moveto(offsetnear(z3, z4, ro));
	offsetlineto(z3, z4, ro);
	arcbetween(z3, z4, z3, ro);
	linetosub(offsetintersect(z4, z3, z3, z2, ro));
	linetosub(offsetintersect(z3, z2, z2, z1, ro));
	linetosub(offsetfar(z2, z1, ro));
	arcbetween(z2, z1, z2, ro);
	offsetlineto(z1, z2, ro);
	arcbetween(z1, z2, z3, ro);
	offsetlineto(z2, z3, ro);
	arcbetween(z2, z3, z4, ro);
	closepath();
}

void lasy_superset(void) { /* 075 "square superset" */
	struct coord z1, z2, z3, z4;
	double spread;

	spread = compute_spread(5.0/4.0 * x_height, 3.0/2.0 * x_height);
	
/*	beginchar(61, 14.0 * u,
		0.5 * (spread+rule_thickness) + math_axis, 
			0.5 * (spread+rule_thickness) - math_axis); */
	
	begincenter(61, 14.0 * u, spread + rule_thickness);

	z1.x = 1.5 * u; z4.x = z1.x;
	z2.x = w - 1.5 * u - oo - ro; z3.x = z2.x;
	z1.y = math_axis + 0.5 * spread; z2.y = z1.y;
	z4.y = z1.y - spread; z3.y = z4.y;

	moveto(offsetnear(z2, z1, ro));
	offsetlineto(z2, z1, ro);
	arcbetween(z2, z1, z2, ro);
	linetosub(offsetintersect(z1, z2, z2, z3, ro));
	linetosub(offsetintersect(z2, z3, z3, z4, ro));
	linetosub(offsetfar(z3, z4, ro));
	arcbetween(z3, z4, z3, ro);
	offsetlineto(z4, z3, ro);
	arcbetween(z4, z3, z2, ro);
	offsetlineto(z3, z2, ro);
	arcbetween(z3, z2, z1, ro);
	closepath(); 
}

void lasy_lhdtria(void) { /* 001 "lhd triangle" */
	struct coord z1, z2, z3;
	double spread;

	spread = compute_spread(5.0/4.0 * x_height, 3.0/2.0 * x_height);
	
/*	beginchar(1, 14.0 * u,
		0.5 * (spread+rule_thickness) + math_axis, 
			0.5 * (spread+rule_thickness) - math_axis); */

	begincenter(1, 14.0 * u, spread + rule_thickness); 

	z2.x = 1.5 * u + ro; 
	z1.x = w - z2.x; z3.x = z1.x;
	z2.y = math_axis;
	z1.y = z2.y + 0.5 * spread; 
	z3.y = z2.y - 0.5 * spread; 

	moveto(offsetnear(z3, z1, ro));
	offsetlineto(z3, z1, ro);
	arcbetween(z3, z1, z2, ro);
	offsetlineto(z1, z2, ro);
	arcbetween(z1, z2, z3, ro);
	offsetlineto(z2, z3, ro);
	arcbetween(z2, z3, z1, ro);
	closepath(); 
	moveto(offsetintersect(z3, z2, z2, z1, ro));
	linetosub(offsetintersect(z2, z1, z1, z3, ro));	
	linetosub(offsetintersect(z1, z3, z3, z2, ro));	
	linetosub(offsetintersect(z3, z2, z2, z1, ro));
	closepath(); 
}

void lasy_rhdtria(void) { /* 003 "rhd triangle" */
	struct coord z1, z2, z3;
	double spread;

	spread = compute_spread(5.0/4.0 * x_height, 3.0/2.0 * x_height);
	
/*	beginchar(3, 14.0 * u,
		0.5 * (spread+rule_thickness) + math_axis, 
			0.5 * (spread+rule_thickness) - math_axis); */

	begincenter(3, 14.0 * u, spread + rule_thickness);

	z2.x = w - 1.5 * u - ro; 
	z1.x = w - z2.x; z3.x = z1.x;
	z2.y = math_axis;
	z1.y = z2.y + 0.5 * spread; 
	z3.y = z2.y - 0.5 * spread; 

	moveto(offsetnear(z1, z3, ro));
	offsetlineto(z1, z3, ro);
	arcbetween(z1, z3, z2, ro);
	offsetlineto(z3, z2, ro);
	arcbetween(z3, z2, z1, ro);
	offsetlineto(z2, z1, ro);
	arcbetween(z2, z1, z3, ro);
	closepath(); 
	moveto(offsetintersect(z1, z2, z2, z3, ro));
	linetosub(offsetintersect(z2, z3, z3, z1, ro));	
	linetosub(offsetintersect(z3, z1, z1, z2, ro));	
	linetosub(offsetintersect(z1, z2, z2, z3, ro));
	closepath(); 
}

void lasy_unlhdtria(void) { /* 002 "unlhd triangle" */
	struct coord z1, z2, z3, z8, z9;
	double spreadh, spread;

	spreadh = compute_spread(0.45 * x_height, 0.55 * x_height);
	spread = compute_spread(5.0/4.0 * x_height, 3.0/2.0 * x_height);
	
/*	beginchar(2, 14.0 * u,
		0.5 * (spreadh+spread+rule_thickness) + math_axis, 
			0.5 * (spreadh+spread+rule_thickness) - math_axis); */

	begincenter(2, 14.0 * u, spreadh + spread + rule_thickness);

	z2.x = 1.5 * u + ro; 
	z1.x = w - z2.x; z3.x = z1.x;
	z1.y = h - ro;
	z3.y = z1.y - spread;
	z2.y = 0.5 * (z1.y + z3.y); 

	z8.x = z1.x; z9.x = z2.x;
	z9.y = z3.y - spreadh; z8.y = z9.y;

	moveto(offsetnear(z3, z1, ro));
	offsetlineto(z3, z1, ro);
	arcbetween(z3, z1, z2, ro);
	offsetlineto(z1, z2, ro);
	arcbetween(z1, z2, z3, ro);
	offsetlineto(z2, z3, ro);
	arcbetween(z2, z3, z1, ro);
	closepath(); 
	moveto(offsetintersect(z3, z2, z2, z1, ro));
	linetosub(offsetintersect(z2, z1, z1, z3, ro));	
	linetosub(offsetintersect(z1, z3, z3, z2, ro));	
	linetosub(offsetintersect(z3, z2, z2, z1, ro));
	closepath();

	moveto(offsetnear(z9, z8, ro));
	offsetlineto(z9, z8, ro);
	arcbetween(z9, z8, z9, ro);
	offsetlineto(z8, z9, ro);	
	arcbetween(z8, z9, z8, ro);
	closepath();
}

void lasy_unrhdtria(void) { /* 004 "unrhd triangle" */
	struct coord z1, z2, z3, z8, z9;
	double spreadh, spread;

	spreadh = compute_spread(0.45 * x_height, 0.55 * x_height);
	spread = compute_spread(5.0/4.0 * x_height, 3.0/2.0 * x_height);
	
/*	beginchar(4, 14.0 * u,
		0.5 * (spread+spreadh+rule_thickness) + math_axis, 
			0.5 * (spread+spreadh+rule_thickness) - math_axis); */

	begincenter(4, 14.0 * u, spreadh + spread + rule_thickness);

	z1.x = 1.5 * u + ro; 
	z2.x = w - z1.x; z3.x = z1.x;
	z1.y = h - ro;
	z3.y = z1.y - spread;
	z2.y = 0.5 * (z1.y + z3.y);
	
	z8.x = z1.x; z9.x = z2.x;
	z9.y = z3.y - spreadh; z8.y = z9.y;

	moveto(offsetnear(z1, z3, ro));
	offsetlineto(z1, z3, ro);
	arcbetween(z1, z3, z2, ro);
	offsetlineto(z3, z2, ro);
	arcbetween(z3, z2, z1, ro);
	offsetlineto(z2, z1, ro);
	arcbetween(z2, z1, z3, ro);
	closepath(); 
	moveto(offsetintersect(z1, z2, z2, z3, ro));
	linetosub(offsetintersect(z2, z3, z3, z1, ro));	
	linetosub(offsetintersect(z3, z1, z1, z2, ro));	
	linetosub(offsetintersect(z1, z2, z2, z3, ro));
	closepath(); 

	moveto(offsetnear(z9, z8, ro));
	offsetlineto(z9, z8, ro);
	arcbetween(z9, z8, z9, ro);
	offsetlineto(z8, z9, ro);	
	arcbetween(z8, z9, z8, ro);
	closepath();
}

void lasy_leftward(void) { /* 050 "Leftward arrowhead" */
	double spread, t1, t2;
	struct coord z0, z3, z4, z3l, z3r, z4l, z4r, z5r, z6r;
	struct coord z9; /* z10, z10o */
	struct coord t12;
	struct spline sp1, sp2, sp3, sp4;

	spread = compute_spread(0.45 * x_height, 0.55 * x_height);
	
/*	beginchar(40, 6.0 * u,
		0.5 * (spread+rule_thickness) + math_axis, 
			0.5 * (spread+rule_thickness) - math_axis); */
	
	begincenter(40, 6.0 * u, spread + rule_thickness);

/*	printf("u %lg, co %lg, bo %lg\n", u, co, bo); */

	z0.x = u + co;
	z0.y = math_axis;
/*	z3.y = z0.y + 0.24 * asc_height; */
	z3.y = z0.y + 0.24 * asc_dash; 
/*	z4.y = z0.y - 0.24 * asc_height; */
	z4.y = z0.y - 0.24 * asc_dash;
/*	z3.x = z0.x + 3.0 * u;  */
	z3.x = z0.x + 3.0 * udash;
	z4.x = z3.x;
	z3l.x = z3.x - (bo - co); z3r.x = z3.x + (bo - co);
	z3l.y = z3.y; z3r.y = z3.y;
	z4l.x = z4.x - (bo - co); z4r.x = z4.x + (bo - co);
	z4l.y = z4.y; z4r.y = z4.y;
	z5r = sum(z0, parallel(z4, z0, 2.0 * (bo - co)));
	z6r = sum(z0, parallel(z3, z0, 2.0 * (bo - co)));
	z9.x = 0.381966 * z0.x + (1.0 - 0.381966) * 0.5 * (z3.x + z4.x);
	z9.y = 0.381966 * z0.y + (1.0 - 0.381966) * 0.5 * (z3.y + z4.y);
/*	z9.y = z0.y; */

	sp1 = circleapprox(z4r, z6r, diff(z9, z4));
	sp2 = circleapprox(z3r, z5r, diff(z9, z3));
	sp3 = circleapprox(z4l, z0, diff(z9, z4));
	sp4 = circleapprox(z3l, z0, diff(z9, z3));

	sp4 = offsetbezier(sp4, co);
	sp3 = reversebezier(sp3);
	sp3 = offsetbezier(sp3, co);
	sp3 = reversebezier(sp3);
	sp1 = offsetbezier(sp1, co);
	sp2 = reversebezier(sp2);
	sp2 = offsetbezier(sp2, co);
	sp2 = reversebezier(sp2);	

	t12 = crossover(sp1, sp2, 0.9, 0.9);
	t1 = t12.x; t2 = t12.y;
	sp1 = splitfirst(sp1, t1);
	sp2 = splitfirst(sp2, t2);
	sp1.ze = sp2.ze;			/* to avoid numerical problems */
	sp2 = reversebezier(sp2);
	sp3 = reversebezier(sp3);

	moveto(sp2.zs);
	showbezier(sp2);
	arcconnect(sp2.ze, z3r, offsetnear(z3r, z3l, co));
	offsetlineto(z3r, z3l, co);
	arcconnect(offsetfar(z3r, z3l, co), z3l, sp4.zs);
	showbezier(sp4);
	arcconnect(sp4.ze, z0, sp3.zs);
	showbezier(sp3);
	arcconnect(sp3.ze, z4l, offsetnear(z4l, z4r, co));
	offsetlineto(z4l, z4r, co);
	arcconnect(offsetfar(z4l, z4r, co), z4r, sp1.zs);
	showbezier(sp1);
	closepath(); 

}

void lasy_rightward(void) { /* 051 "Rightward arrowhead" */
	double spread, t1, t2;
	struct coord z0, z3, z4, z3l, z3r, z4l, z4r, z5r, z6r;
	struct coord z9; /* z10, z10o */
	struct coord t12;
	struct spline sp1, sp2, sp3, sp4;

	spread = compute_spread(0.45 * x_height, 0.55 * x_height);
	
/*	beginchar(41, 6.0 * u,
		0.5 * (spread+rule_thickness) + math_axis, 
			0.5 * (spread+rule_thickness) - math_axis); */

	begincenter(41, 6.0 * u, spread + rule_thickness);

/*	printf("u %lg, co %lg, bo %lg\n", u, co, bo); */

/*  note: `l' and `r' are right and left respectively */

	z0.x = w - u - co;
	z0.y = math_axis;
/*	z3.y = z0.y + 0.24 * asc_height; */
	z3.y = z0.y + 0.24 * asc_dash;
/*	z4.y = z0.y - 0.24 * asc_height; */
	z4.y = z0.y - 0.24 * asc_dash;
/*	z3.x = z0.x - 3.0 * u; */
	z3.x = z0.x - 3.0 * udash;
	z4.x = z3.x;
	z3l.x = z3.x + (bo - co); z3r.x = z3.x - (bo - co);
	z3l.y = z3.y; z3r.y = z3.y;
	z4l.x = z4.x + (bo - co); z4r.x = z4.x - (bo - co);
	z4l.y = z4.y; z4r.y = z4.y;
	z5r = sum(z0, parallel(z4, z0, 2.0 * (bo - co)));
	z6r = sum(z0, parallel(z3, z0, 2.0 * (bo - co)));
	z9.x = 0.381966 * z0.x + (1.0 - 0.381966) * 0.5 * (z3.x + z4.x);
	z9.y = 0.381966 * z0.y + (1.0 - 0.381966) * 0.5 * (z3.y + z4.y);
/*	z9.y = z0.y; */

	sp1 = circleapprox(z4r, z6r, diff(z9, z4));
	sp2 = circleapprox(z3r, z5r, diff(z9, z3));
	sp3 = circleapprox(z4l, z0, diff(z9, z4));
	sp4 = circleapprox(z3l, z0, diff(z9, z3));

	sp4 = reversebezier(sp4);
	sp4 = offsetbezier(sp4, co);
	sp4 = reversebezier(sp4);
	sp3 = offsetbezier(sp3, co);
	sp1 = reversebezier(sp1);
	sp1 = offsetbezier(sp1, co);
	sp1 = reversebezier(sp1);
	sp2 = offsetbezier(sp2, co);

	t12 = crossover(sp1, sp2, 0.9, 0.9);
	t1 = t12.x; t2 = t12.y;
	sp1 = splitfirst(sp1, t1);
	sp2 = splitfirst(sp2, t2);
	sp1.ze = sp2.ze;			/* to avoid numerical problems */
	sp1 = reversebezier(sp1);
	sp4 = reversebezier(sp4);

	moveto(sp1.zs);
	showbezier(sp1);
	arcconnect(sp1.ze, z4r, offsetnear(z4r, z4l, co));
	offsetlineto(z4r, z4l, co);
	arcconnect(offsetfar(z4r, z4l, co), z4l, sp3.zs);
	showbezier(sp3);
	arcconnect(sp3.ze, z0, sp4.zs);
	showbezier(sp4);
	arcconnect(sp4.ze, z3l, offsetnear(z3l, z3r, co));
	offsetlineto(z3l, z3r, co);
	arcconnect(offsetfar(z3l, z3r, co), z3r, sp2.zs);
	showbezier(sp2);
	closepath(); 

}

/* numeric asc_depth#; .5[asc_height#,-asc_depth#]=math_axis#; */

void lasy_upward(void) { /* 052 "Upward arrowhead" */
	double asc_depth, t1, t2;
	struct coord z0, z3, z4, z3l, z3r, z4l, z4r, z5r, z6r;
	struct coord z9; /* z10, z10o */
	struct coord t12;
	struct spline sp1, sp2, sp3, sp4;

	asc_depth = asc_height - 2.0 * math_axis;

	beginchar(42, 9.0 * u, asc_height, asc_depth);

/*	printf("u %lg, co %lg, bo %lg\n", u, co, bo); */

	z0.x = w / 2.0; z0.y = - co;
/*	z3.x = z0.x - 3.0 * u;  z4.x = z0.x + 3.0 * u; */
	z3.x = z0.x - 3.0 * udash;	z4.x = z0.x + 3.0 * udash;	
/*	z3.y = z0.y - 0.24 * asc_height; */
	z3.y = z0.y - 0.24 * asc_dash;	
	z4.y = z3.y;

	z3l.y = z3.y - (bo - co); z3r.y = z3.y + (bo - co);
	z3l.x = z3.x; z3r.x = z3.x;
	z4l.y = z4.y - (bo - co); z4r.y = z4.y + (bo - co);
	z4l.x = z4.x; z4r.x = z4.x;
	z5r = sum(z0, parallel(z4, z0, 2.0 * (bo - co)));
	z6r = sum(z0, parallel(z3, z0, 2.0 * (bo - co)));
	z9.y = 0.381966 * z0.y + (1.0 - 0.381966) * 0.5 * (z3.y + z4.y);
	z9.x = 0.381966 * z0.x + (1.0 - 0.381966) * 0.5 * (z3.x + z4.x);
/*	z9.x = z0.x; */

	sp1 = circleapprox(z4l, z6r, diff(z9, z4));
	sp2 = circleapprox(z3l, z5r, diff(z9, z3));
	sp3 = circleapprox(z4r, z0, diff(z9, z4));
	sp4 = circleapprox(z3r, z0, diff(z9, z3));

	sp4 = reversebezier(sp4);
	sp4 = offsetbezier(sp4, co);
	sp4 = reversebezier(sp4);
	sp3 = offsetbezier(sp3, co);
	sp1 = reversebezier(sp1);
	sp1 = offsetbezier(sp1, co);
	sp1 = reversebezier(sp1);
	sp2 = offsetbezier(sp2, co);

	t12 = crossover(sp1, sp2, 0.9, 0.9);
	t1 = t12.x; t2 = t12.y;
	sp1 = splitfirst(sp1, t1);
	sp2 = splitfirst(sp2, t2);
	sp1.ze = sp2.ze;			/* to avoid numerical problems */
	sp1 = reversebezier(sp1);
	sp4 = reversebezier(sp4);

	moveto(sp1.zs);
	showbezier(sp1);
	arcconnect(sp1.ze, z4l, offsetnear(z4l, z4r, co));
	offsetlineto(z4l, z4r, co);
	arcconnect(offsetfar(z4l, z4r, co), z4r, sp3.zs);
	showbezier(sp3);
	arcconnect(sp3.ze, z0, sp4.zs);
	showbezier(sp4);
	arcconnect(sp4.ze, z3r, offsetnear(z3r, z3l, co));
	offsetlineto(z3r, z3l, co);
	arcconnect(offsetfar(z3r, z3l, co), z3l, sp2.zs);
	showbezier(sp2);
	closepath(); 
}

/* numeric asc_depth#; .5[asc_height#,-asc_depth#]=math_axis#; */

void lasy_downward(void) { /* 053 "Downward arrowhead" */
	double asc_depth, t1, t2;
	struct coord z0, z3, z4, z3l, z3r, z4l, z4r, z5r, z6r;
	struct coord z9; /* z10, z10o */
	struct coord t12;
	struct spline sp1, sp2, sp3, sp4;

	asc_depth = asc_height - 2.0 * math_axis;

	beginchar(43, 9.0 * u, asc_height, asc_depth);

/*	printf("u %lg, co %lg, bo %lg\n", u, co, bo); */

	z0.x = w / 2.0; z0.y = co;
/*	z3.x = z0.x - 3.0 * u; z4.x = z0.x + 3.0 * u; */
	z3.x = z0.x - 3.0 * udash; z4.x = z0.x + 3.0 * udash;
/*	z3.y = z0.y + 0.24 * asc_height; */
	z3.y = z0.y + 0.24 * asc_dash;	
	z4.y = z3.y;

/* note `l' is up and `r' is down in z3 and z4 */

	z3l.y = z3.y + (bo - co); z3r.y = z3.y - (bo - co);
	z3l.x = z3.x; z3r.x = z3.x;
	z4l.y = z4.y + (bo - co); z4r.y = z4.y - (bo - co);
	z4l.x = z4.x; z4r.x = z4.x;
	z5r = sum(z0, parallel(z4, z0, 2.0 * (bo - co)));
	z6r = sum(z0, parallel(z3, z0, 2.0 * (bo - co)));
	z9.y = 0.381966 * z0.y + (1.0 - 0.381966) * 0.5 * (z3.y + z4.y);
	z9.x = 0.381966 * z0.x + (1.0 - 0.381966) * 0.5 * (z3.x + z4.x);
/*	z9.x = z0.x; */

	sp1 = circleapprox(z4l, z6r, diff(z9, z4));
	sp2 = circleapprox(z3l, z5r, diff(z9, z3));
	sp3 = circleapprox(z4r, z0, diff(z9, z4));
	sp4 = circleapprox(z3r, z0, diff(z9, z3));

	sp4 = offsetbezier(sp4, co);
	sp3 = reversebezier(sp3);
	sp3 = offsetbezier(sp3, co);
	sp3 = reversebezier(sp3);
	sp1 = offsetbezier(sp1, co);
	sp2 = reversebezier(sp2);
	sp2 = offsetbezier(sp2, co);
	sp2 = reversebezier(sp2);

	t12 = crossover(sp1, sp2, 0.9, 0.9);
	t1 = t12.x; t2 = t12.y;
	sp1 = splitfirst(sp1, t1);
	sp2 = splitfirst(sp2, t2);
	sp1.ze = sp2.ze;			/* to avoid numerical problems */
	sp2 = reversebezier(sp2);
	sp3 = reversebezier(sp3);

	moveto(sp2.zs);
	showbezier(sp2);
	arcconnect(sp2.ze, z3l, offsetnear(z3l, z3r, co));
	offsetlineto(z3l, z3r, co);
	arcconnect(offsetfar(z3l, z3r, co), z3r, sp4.zs);
	showbezier(sp4);
	arcconnect(sp4.ze, z0, sp3.zs);
	showbezier(sp3);
	arcconnect(sp3.ze, z4r, offsetnear(z4r, z4l, co));
	offsetlineto(z4r, z4l, co);
	arcconnect(offsetfar(z4r, z4l, co), z4l, sp1.zs);
	showbezier(sp1);
	closepath(); 
}

void lasy_leadsto(void) { /* 072 "Leads to character extension" */
	struct coord z10, z11, z12, z13, z14, z15, z16;
	struct spline sp1, sp2, sp3, sp4;
	struct spline sp1u, sp1d, sp2u, sp2d, sp3u, sp3d, sp4u, sp4d;
	double a, spread;
	
	spread = compute_spread(0.45 * x_height, 0.55 * x_height);	

/*	beginchar(58, 12.0 * u,
		0.5 * spread + math_axis, 0.5 * spread - math_axis); */
	
	begincenter(58, 12.0 * u, spread + rule_thickness);

	a = 0.45 * math_axis;

/*	printf("font_size %lg, width %lg, height %lg, ro %lg\n",
		font_size, h, w, ro);
	printf("math_axis %lg, unit %lg, a %lg\n", math_axis, u, a); */

	z11.y = math_axis, z13.y = z11.y, z15.y = z11.y;
	z12.y = math_axis + a; z16.y = z12.y;
	z14.y = math_axis - a; z10.y = z14.y;
	z11.x = 0.0; z15.x = w; z13.x = 0.5 * (z11.x + z15.x);
	z12.x = 0.5 * (z11.x + z13.x); z14.x = 0.5 *(z13.x + z15.x);
	z10.x = z11.x - (z12.x - z11.x);
	z16.x = z15.x - (z14.x - z15.x);

	sp1 = smoothspline(z12, diff(z13, z11), z14, diff(z13, z11));
	sp2 = smoothspline(z10, diff(z13, z11), z12, diff(z13, z11));	
	sp3 = smoothspline(z14, diff(z13, z11), z16, diff(z13, z11));
	
	sp2 = splitlast(sp2, 0.5);
	sp3 = splitfirst(sp3, 0.5);
	sp4 = splitlast(sp1, 0.5);
	sp1 = splitfirst(sp1, 0.5);
	
	sp2d = offsetbezier(sp2, ro);
	sp1d = offsetbezier(sp1, ro);
	sp4d = offsetbezier(sp4, ro);
	sp3d = offsetbezier(sp3, ro);
	
	sp3u = offsetbezier(reversebezier(sp3), ro);
	sp4u = offsetbezier(reversebezier(sp4), ro);
	sp1u = offsetbezier(reversebezier(sp1), ro);
	sp2u = offsetbezier(reversebezier(sp2), ro);

	moveto(sp3d.zs);
	showbezier(sp3d);
	arcconnect(sp3d.ze, z15, sp3u.zs);
	showbezier(sp3u);
	showbezier(sp4u);
	showbezier(sp1u);	
	showbezier(sp2u);
	arcconnect(sp2u.ze, z11, sp2d.zs);
	showbezier(sp2d);
	showbezier(sp1d);
	showbezier(sp4d);
	closepath();
}

void lasy_leadsarrow(void) { /* 073 "Leads to character with arrow head" */
	struct coord z0, z3, z4, z9, voff, tcrs;
	struct coord z3l, z3r, z4l, z4r, z5r, z6r;
	struct coord z10, z11, z12, z13, z14, z15, z16;
	struct spline line1, line2;
	struct spline sp1, sp2, sp3, sp4, sp5;
	struct spline sp11, sp12, sp13, sp14;
	struct spline sp1u, sp1d, sp2u, sp2d, sp3u, sp3d, sp4u, sp4d, sp5u, sp5d;
	double a, spread;
	
	spread = compute_spread(0.45 * x_height, 0.55 * x_height);	

/*	beginchar(59, 18.0 * u,
		0.5 * (spread + rule_thickness) + math_axis, 
			0.5 * (spread + rule_thickness)  - math_axis); */

	begincenter(59, 18.0 * u, spread + rule_thickness); 

	a = 0.45 * math_axis;

/*	printf("font_size %lg, width %lg, height %lg, ro %lg\n",
		font_size, h, w, ro);
	printf("math_axis %lg, unit %lg, a %lg\n", math_axis, u, a); */

	z0.y = math_axis; z0.x = w -u - co;

	z11.y = math_axis, z13.y = z11.y, z15.y = z11.y; z16.y = z11.y;
	z12.y = math_axis + a; 
	z14.y = math_axis - a; z10.y = z14.y;

	z11.x = 0.0; z15.x = 2.0 * w / 3.0; z13.x = 0.5 * (z11.x + z15.x);
	z12.x = 0.5 * (z11.x + z13.x); z14.x = 0.5 *(z13.x + z15.x);
	z10.x = z11.x - (z12.x - z11.x);
	z16.x = 0.3 * z0.x + (1.0 - 0.3) * z15.x;

/*	z17.y = z16.y; z17.x = z16.x + (z16.x - z15.x);  */

	sp1 = smoothspline(z12, diff(z13, z11), z14, diff(z13, z11));
	sp2 = smoothspline(z10, diff(z13, z11), z12, diff(z13, z11));	
	sp3 = smoothspline(z14, diff(z13, z11), z16, diff(z13, z11));
	
	sp2 = splitlast(sp2, 0.5);
	sp5 = splitlast(sp3, 0.5);
	sp3 = splitfirst(sp3, 0.5);
	sp4 = splitlast(sp1, 0.5);
	sp1 = splitfirst(sp1, 0.5);
	
	sp2d = offsetbezier(sp2, ro);
	sp1d = offsetbezier(sp1, ro);
	sp4d = offsetbezier(sp4, ro);
	sp3d = offsetbezier(sp3, ro);
	sp5d = offsetbezier(sp5, ro);
	
	sp3u = offsetbezier(reversebezier(sp3), ro);
	sp4u = offsetbezier(reversebezier(sp4), ro);
	sp1u = offsetbezier(reversebezier(sp1), ro);
	sp2u = offsetbezier(reversebezier(sp2), ro);
	sp5u = offsetbezier(reversebezier(sp5), ro);

	z0.x = w - u - co;
	z0.y = math_axis;
	z3.y = z0.y + 0.24 * asc_height;
	z4.y = z0.y - 0.24 * asc_height;
	z3.x = z0.x - 3.0 * u; z4.x = z3.x;
	z3l.x = z3.x + (bo - co); z3r.x = z3.x - (bo - co);
	z3l.y = z3.y; z3r.y = z3.y;
	z4l.x = z4.x + (bo - co); z4r.x = z4.x - (bo - co);
	z4l.y = z4.y; z4r.y = z4.y;
	z5r = sum(z0, parallel(z4, z0, 2.0 * (bo - co)));
	z6r = sum(z0, parallel(z3, z0, 2.0 * (bo - co)));
	z9.x = 0.381966 * z0.x + (1.0 - 0.381966) * 0.5 * (z3.x + z4.x);
	z9.y = z0.y; 

	sp11 = circleapprox(z4r, z6r, diff(z9, z4));
	sp12 = circleapprox(z3r, z5r, diff(z9, z3));
	sp13 = circleapprox(z4l, z0, diff(z9, z4));
	sp14 = circleapprox(z3l, z0, diff(z9, z3));

	sp14 = reversebezier(sp14);
	sp14 = offsetbezier(sp14, co);
	sp14 = reversebezier(sp14);
	sp13 = offsetbezier(sp13, co);
	sp11 = reversebezier(sp11);
	sp11 = offsetbezier(sp11, co);
	sp11 = reversebezier(sp11);
	sp12 = offsetbezier(sp12, co);

	voff.y = ro; voff.x = 0.0;
	line2 = smoothspline(diff(z16, voff), diff(z0, z16), 
		diff(z0, voff), diff(z0, z16));
	line1 = smoothspline(sum(z16, voff), diff(z0, z16), 
		sum(z0, voff), diff(z0, z16));	

	tcrs = crossover(sp12, line1, 0.9, 0.9);
	sp12 = splitfirst(sp12, tcrs.x);

	tcrs = crossover(sp11, line2, 0.9, 0.9);
	sp11 = splitfirst(sp11, tcrs.x);

	sp11 = reversebezier(sp11);
	sp14 = reversebezier(sp14);

	moveto(sp11.zs);
	showbezier(sp11);
	arcconnect(sp11.ze, z4r, offsetnear(z4r, z4l, co));
	offsetlineto(z4r, z4l, co);
	arcconnect(offsetfar(z4r, z4l, co), z4l, sp13.zs);
	showbezier(sp13);
	arcconnect(sp13.ze, z0, sp14.zs);
	showbezier(sp14);
	arcconnect(sp14.ze, z3l, offsetnear(z3l, z3r, co));
	offsetlineto(z3l, z3r, co);
	arcconnect(offsetfar(z3l, z3r, co), z3r, sp12.zs);
	showbezier(sp12);

	linetosub(sum(z16, voff));
	showbezier(sp5u);
	showbezier(sp3u);
	showbezier(sp4u);
	showbezier(sp1u);	
	showbezier(sp2u);
	arcconnect(sp2u.ze, z11, sp2d.zs);
	showbezier(sp2d);
	showbezier(sp1d);
	showbezier(sp4d);
	showbezier(sp3d);
	showbezier(sp5d);
	linetosub(sp11.zs);
	closepath();
}

/* There are at least two bugs in the lasy fonts:  */

/* note: in cmr10, where this comes from, crisp = 0 */
/* hence filldraw (called in arm) creates SHARP corners */
/* unlike in cmsy10 where crisp in non-zero */
/* so there is a BUG in lasy10.mf, since it uses parameters of cmsy10 */
/* and does not set crisp = 0 - thus the serifs in omega are rounded */
/* (fixed here) */

/* also, the ratio of width to height in arrow head is the same for */
/* vertical and horizontal arrow heads ONLY for lasy 10 */
/* the reason is that this depends on the ratio of asc_height / u */


void lasy_unomega(void) { /* 060 "Upside down Omega" */
	struct coord z1, z2, z3, z4, z5;
/*	struct coord z6, z7, z8, z9; */
	struct coord z6l, z6r, z7l, z7r, z8l, z8r, z9l, z9r;
	struct coord z10, z11, z12, z13, z14, z15, z16, z17, z18, z19;
	struct coord hoff, voff, t12;
	struct coord z1l, z1r, z2l, z2r, z3l, z3r, z4l, z4r, z5l, z5r;
	struct spline sp1, sp2, sp3, sp4, sp5, sp6, sp7, sp8;
/*	struct spline sp9, sp10, sp11, sp12; */
	struct spline sp13, sp14;
	struct spline line1;
/*	struct spline line2; */
	double arm_thickness;

/* only for cmsy6 is vair <> tiny - need to round corner differently */

/* for omega (from cmr10), crisp = 0, hence co = 0 */	
	double coserif=0.0; /* coserif = co */

/* for bowl we take a shortcut: Instead of drawing outline narrower by to */
/* and then growing it by to - just draw full width right away */
	double tobowl=0.0; /* tobowl = to */
	
	beginchar(48, 13.0 * u, cap_height, 0.0);

	voff.x = 0.0; voff.y = 1.0;
	hoff.x = 1.0; hoff.y = 0.0;	

	z1.x = 0.5 * w;  z1l.x = z1.x; z1r.x = z1.x;
	z1r.y = tobowl - o; z1l.y = z1r.y + vair - tobowl;
	z4r.x = u + tobowl; z4l.x = z4r.x + cap_curve - tobowl;
	z2l.x = w - z4l.x; 	z2r.x = w - z4r.x;
	z4.y = h / 3.0; z2.y = z4.y;
	z4l.y = z4.y; 	z4r.y = z4.y;
	z2l.y = z2.y; 	z2r.y = z2.y;	
	z3.y = h - to;		/* z3.y = h - tobowl */
	z3l.y = z3.y; z3r.y = z3.y;
	z5.y = z3.y; 
	z5l.y = z5.y; z5r.y = z5.y;	
	z5l.x = (w + 0.5 * u) / 3.0 + 0.5 * hair;
	z5r.x = z5l.x - (vair - tobowl);
	z3l.x = w - z5l.x; z3r.x = w - z5r.x;
	z2.x = 0.5 * (z2l.x + z2r.x);
	z3.x = 0.5 * (z3l.x + z3r.x);
	z4.x = 0.5 * (z4l.x + z4r.x);	
	z5.x = 0.5 * (z5l.x + z5r.x);	

/* following is for bowl */

	sp1 = smoothspline(z5r, neg(voff), z4r, neg(voff));
	sp2 = tensespline(z4r, neg(voff), z1r, hoff, 0.5, 0.33);
	sp3 = tensespline(z1r, hoff, z2r, voff, 0.33, 0.5);
	sp4 = smoothspline(z2r, voff, z3r, voff);
	sp5 = smoothspline(z3l, neg(voff), z2l, neg(voff));
	sp6 = tensespline(z2l, neg(voff), z1l, neg(hoff), 0.5, 0.33);
	sp7 = tensespline(z1l, neg(hoff), z4l, voff, 0.33, 0.5);
	sp8 = smoothspline(z4l, voff, z5l, voff);

/* following is for serifs */

/*	since hefty = 0 for all these fonts: */
	arm_thickness = cap_vstem * 0.75 + slab * (1.0 - 0.75);

/*	printf("Z3 (%lg %lg) Z2 (%lg %lg)\n", z3.x, z3.y, z2.x, z2.y);
	getch(); */

/* note: actual slab = 0.75 * cap_vstem + 0.25 * slab */
/* note: actual beak_jut = 1.2 * beak_jut */
/* note: actual beak = 0.5 * beak */
/* note: actual beak_darkness = 0.5 * beak_darkness */
	
	z6r.y = h - coserif; z6l.y = z3.y - arm_thickness;
	z6r.x = z3.x; z6l.x = z6r.x;
	
	z7r.x = w - 0.8 * u - coserif; z7l.x = z7r.x - hair;
	z7r.y = z6l.y - 0.5 * beak; z7l.y = z7r.y;

/*	printf("Z6L (%lg %lg) Z6R (%lg %lg) Z7L (%lg %lg) Z7R (%lg %lg)\n", 
		z6l.x, z6l.y, z6r.x, z6r.y, z7l.x, z7l.y, z7r.x, z7r.y);
	getch();	 */

	z10.x = z7r.x - 1.2 * beak_jut;
	z10.y = z6r.y; /*	z10.y = z3r.y; */
	z12 = sum(z7l, scale(diff(z10, z7r), (z6l.y - z7r.y) / (z10.y - z7r.y)));

/*	printf("Z10 (%lg %lg) Z12 (%lg %lg)\n", z10.x, z10.y, z12.x, z12.y);
	getch();  */

	z14 = scale(sum(z6l, z12), 0.5);

/*	printf("Z14 (%lg %lg)\n", z14.x, z14.y);
	getch();  */
	
	z18 = weighted(z12, weighted(z7l, z14, 0.5), 0.5 * beak_darkness);

	line1 = smoothspline(z14, neg(hoff), z6l, neg(hoff));
	if (traceflag != 0) {
		printbezier(line1);	printbezier(sp4);
	}
	t12 = crossover(sp4, line1, 0.8, 0.8);
	z16 = bezier(sp4, t12.x);

/*	printf("Z16 (%lg %lg)\n", z16.x, z16.y);
	getch(); */

/* now reflect to make left serif */
	
	z11 = reflect(z10); z13 = reflect(z12); 
	z15 = reflect(z14); z17 = reflect(z16); z19 = reflect(z18);
	z8l = reflect(z6l); z8r = reflect(z6r);
	z9l = reflect(z7l); z9r = reflect(z7r);

	sp4 = splitfirst(sp4, t12.x);	/* truncate this sucker */

	sp1 = splitlast(sp1, 1.0 - t12.x); /* truncate this sucker also */

	sp13 = flexspine(z14, hoff, z7l, diff(z7l, z12), z18);
	sp14 = flexspine(z9l, diff(z13, z9l), z15, hoff, z19);

/*	sp1 = offsetbezier(sp1, to);
	sp2 = offsetbezier(sp2, to);	
	sp3 = offsetbezier(sp3, to);
	sp4 = offsetbezier(sp4, to);
	sp5 = offsetbezier(sp5, to);
	sp6 = offsetbezier(sp6, to);
	sp7 = offsetbezier(sp7, to);
	sp8 = offsetbezier(sp8, to); 
	sp9 = offsetbezier(sp9, to); 	
	sp10 = offsetbezier(sp10, to); 		
	sp11 = offsetbezier(sp11, to); 		
	sp12 = offsetbezier(sp12, to); 	*/

/*	moveto(offsetfar(z5l, z5r, ro)); 	
	arcconnect(offsetfar(z5l, z5r, ro), z5r, sp1.zs);
	showbezier(sp1); showbezier(sp2); showbezier(sp3); showbezier(sp4);
	arcconnect(sp4.ze, offsetnear(z3r, z3l, ro), z3r);
	offsetlineto(z3r, z3l, ro);
	arcconnect(offsetfar(z3r, z3l, ro), z3r, sp5.zs);
	showbezier(sp5); showbezier(sp6); showbezier(sp7); showbezier(sp8);	
	arcconnect(sp8.ze, z5l, offsetnear(z5l, z5r, ro));
	offsetlineto(z5l, z5r, ro); */
	
/*	sp9 = splitlast(sp1, 0.5);
	sp1 = splitfirst(sp1, 0.5);
	sp10 = splitlast(sp4, 0.5);
	sp4 = splitfirst(sp4, 0.5);
	sp11 = splitlast(sp5, 0.5);
	sp5 = splitfirst(sp5, 0.5);
	sp12 = splitlast(sp8, 0.5);
	sp8 = splitfirst(sp8, 0.5); */

/*	printf("READY\n");
	getch(); */
	
	moveto(z5l); 	
/*	lineto(z5l, z8r); */
	if (vair == tiny) arcconnect(z5l, z5, z8r);
	else showbezier(quadrantspline(z5l, voff, z8r, neg(hoff)));
	lineto(z8r, z11); 
	lineto(z11, z9r);
	lineto(z9r, z9l);
/*	lineto(z9l, z13); lineto(z13, z15); */
/*	lineto(z9l, z19); lineto(z19, z15);	*/
	showbezier(sp14);
	lineto(z15, z17);
	showbezier(sp1); 
/*	showbezier(sp9); */
	showbezier(sp2); showbezier(sp3); 
	showbezier(sp4); 
/*	showbezier(sp10); */
	lineto(z16, z14);
/*	lineto(z14, z12); 	lineto(z12, z7l); */
/*	lineto(z14, z18); 	lineto(z18, z7l); */
	showbezier(sp13);
	lineto(z7l, z7r);
	lineto(z7r, z10);
	lineto(z10, z6r); 
/*	lineto(z6r, z3l);  */
	if (vair == tiny) arcconnect(z6r, z3, z3l);
	else showbezier(quadrantspline(z6r, neg(hoff), z3l, neg(voff)));

/*	lineto(z3r, z3l); */
	showbezier(sp5); 
/*	showbezier(sp11); */
	showbezier(sp6); showbezier(sp7); 
	showbezier(sp8); 
/*	showbezier(sp12); */
/*	lineto(z5l, z5r); */
	closepath();

}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* now for code shared by all the lasy fonts */

void lasy(void) { /* derived quantities */
	double stretch;
/*	px = 1.0; py = 1.0; */
	ro = rule_thickness/2.0;
	co = crisp/2.0;
	bo = bar/2.0;
	to = tiny/2.0;
	oo = o * 0.5 * o_correction;
	cap_vstem = 0.8 * cap_stem + (1.0 - 0.8) * vair;
	stretch = sqrt((2.0 / 25.0) * (asc_height / u));
	printf("STRETCH %lg\n", stretch);
	udash = u * stretch;
	asc_dash = asc_height / stretch;
}

/*  oo:=vround(.5o#*hppp*o_correction)+eps; */

/* now for code for specific sizes and weights */

/* order: cmsy5, cmsy6, cmsy7, cmsy8, cmsy9, cmsy10, cmbsy10 */

void lasy5(void) {
	font_size = 5.0;
	u = 12.5/36.0;      /* unit width */
	letter_fit = 10.0/36.0;	/* extra sidebar space */
	o = 4.0/36.0;      /* amount of overshoot for curves */
	crisp = 6.0/36.0;    /* diameter of serif corners */
	bar = 8.0/36.0;      /* lowercase bar thickness */
	x_height = 77.5/36.0;    /* height of lowercase without ascenders */
	asc_height = 125.0/36.0;    /* height of lowercase ascenders */
	cap_height = 123.0/36.0;    /* height of caps */
	math_axis = 45.0/36.0;    /* axis of symmetry for math symbols */
	rule_thickness = .28;  /* thickness of lines in math symbols */
	cap_curve = 20.5/36.0;    /* uppercase curve breadth */
	hair = 7.5/36.0;    /* lowercase hairline breadth */
	vair = 6.0/36.0;      /* vertical diameter of hairlines */
	tiny = 6.0/36.0;      /* diameter of rounded corners */
	slab =  8.0/36.0;      /* serif and arm thickness */
	beak_jut =  6.0/36.0;    /* horizontal protrusion of beak serifs */
	beak =  35.0/36.0;    /* vertical protrusion of beak serifs */
 	beak_darkness = 11.0/30.0;   /* fraction of triangle inside beak serifs */
	cap_stem = 18.5/36.0;    /* uppercase stem breadth */
	math_spread = 1.0;     /* extra openness of math symbols */
	lasy();
}

void lasy6(void) {
	font_size = 6.0;
	u = 14.0/36.0;      /* unit width */
	letter_fit = 6.0/36.0;	/* extra sidebar space */
	o = 4.5/36.0;      /* amount of overshoot for curves */
	crisp = 6.5/36.0;    /* diameter of serif corners */
	bar = 8.5/36.0;    /* lowercase bar thickness */
	x_height = 93.0/36.0;    /* height of lowercase without ascenders */
	asc_height = 150.0/36.0;    /* height of lowercase ascenders */
	cap_height = 147.6/36.0;  /* height of caps */
	math_axis = 54.0/36.0;    /* axis of symmetry for math symbols */
	rule_thickness = .31;  /* thickness of lines in math symbols */
	cap_curve = 23.5/36.0;    /* uppercase curve breadth */
	hair = 8.0/36.0;      /* lowercase hairline breadth */
	vair = 7.0/36.0;      /* vertical diameter of hairlines */
	tiny = 6.5/36.0;    /* diameter of rounded corners */
	slab =  8.5/36.0;    /* serif and arm thickness */
	beak_jut =  6.8/36.0;    /* horizontal protrusion of beak serifs */
	beak =  42.0/36.0;    /* vertical protrusion of beak serifs */
	beak_darkness = 11.0/30.0;   /* fraction of triangle inside beak serifs */
	cap_stem = 21.0/36.0;    /* uppercase stem breadth */
	math_spread = 0.8;     /* extra openness of math symbols */
	lasy();
}

void lasy7(void) {
	font_size = 7.0;
	u = 15.5/36.0;      /* unit width */
	letter_fit = 4.0/36.0;	/* extra sidebar space */
	o = 5.0/36.0;      /* amount of overshoot for curves */
	crisp = 7.0/36.0;    /* diameter of serif corners */
	bar = 9.0/36.0;      /* lowercase bar thickness */
	x_height = 108.5/36.0;    /* height of lowercase without ascenders */
	asc_height = 175.0/36.0;    /* height of lowercase ascenders */
	cap_height = 172.2/36.0;  /* height of caps */
	math_axis = 63.0/36.0;    /* axis of symmetry for math symbols */
	rule_thickness = .34;  /* thickness of lines in math symbols */
	cap_curve = 26.5/36.0;    /* uppercase curve breadth */
	hair = 8.5/36.0;    /* lowercase hairline breadth */
	vair = 7.0/36.0;      /* vertical diameter of hairlines */
	tiny = 7.0/36.0;      /* diameter of rounded corners */
	slab =  9.0/36.0;      /* serif and arm thickness */
	beak_jut =  7.6/36.0;    /* horizontal protrusion of beak serifs */
	beak =  49.0/36.0;    /* vertical protrusion of beak serifs */
	beak_darkness = 11.0/30.0;   /* fraction of triangle inside beak serifs */
	cap_stem = 23.5/36.0;    /* uppercase stem breadth */
	math_spread = 0.6;     /* extra openness of math symbols */
	lasy();
}

void lasy8(void) {
	font_size = 8.0;
	u = 17.0/36.0;      /* unit width */
	letter_fit = 0.0;	/* extra sidebar space */
	o = 6.0/36.0;      /* amount of overshoot for curves */
	crisp = 8.0/36.0;    /* diameter of serif corners */
	bar = 9.5/36.0;    /* lowercase bar thickness */
	x_height = 124.0/36.0;    /* height of lowercase without ascenders */
	asc_height = 200.0/36.0;    /* height of lowercase ascenders */
	cap_height = 196.8/36.0;  /* height of caps */
	math_axis = 72.0/36.0;    /* axis of symmetry for math symbols */
	rule_thickness = .36;  /* thickness of lines in math symbols */
	cap_curve = 29.0/36.0;    /* uppercase curve breadth */
	hair = 9.0/36.0;      /* lowercase hairline breadth */
	vair = 8.0/36.0;      /* vertical diameter of hairlines */
	tiny = 8.0/36.0;      /* diameter of rounded corners */
	slab =  9.5/36.0;    /* serif and arm thickness */
	beak_jut =  8.4/36.0;    /* horizontal protrusion of beak serifs */
	beak =  56.0/36.0;    /* vertical protrusion of beak serifs */
	beak_darkness = 11.0/30.0;   /* fraction of triangle inside beak serifs */
	cap_stem = 25.5/36.0;    /* uppercase stem breadth */
	math_spread = 0.4;     /* extra openness of math symbols */
	lasy();
}

void lasy9(void) {
	font_size = 9.0;
	u = 18.5/36.0;      /* unit width */
	letter_fit = 0.0;	/* extra sidebar space */
	o = 7.0/36.0;      /* amount of overshoot for curves */
	crisp = 8.0/36.0;    /* diameter of serif corners */
	bar = 10.0/36.0;      /* lowercase bar thickness */
	x_height = 139.5/36.0;    /* height of lowercase without ascenders */
	asc_height = 225.0/36.0;    /* height of lowercase ascenders */
	cap_height = 221.4/36.0;  /* height of caps */
	math_axis = 81.0/36.0;    /* axis of symmetry for math symbols */
	rule_thickness = .38;  /* thickness of lines in math symbols */
	cap_curve = 32.0/36.0;    /* uppercase curve breadth */
	hair = 9.0/36.0;      /* lowercase hairline breadth */
	vair = 8.0/36.0;      /* vertical diameter of hairlines */
	tiny = 8.0/36.0;      /* diameter of rounded corners */
	slab =  10.0/36.0;    /* serif and arm thickness */
	beak_jut =  9.2/36.0;    /* horizontal protrusion of beak serifs */
	beak =  63.0/36.0;    /* vertical protrusion of beak serifs */
	beak_darkness = 11.0/30.0;   /* fraction of triangle inside beak serifs */
	cap_stem = 28.0/36.0;    /* uppercase stem breadth */
	math_spread = 0.2;     /* extra openness of math symbols */
	lasy();
}

void lasy10(void) {
	font_size = 10.0;
	u = 20.0/36.0;      /* unit width */
	letter_fit = 0.0;	/* extra sidebar space */
	o = 8.0/36.0;      /* amount of overshoot for curves */
	crisp = 8.0/36.0;    /* diameter of serif corners */
	bar = 11.0/36.0;      /* lowercase bar thickness */ 
	x_height = 155.0/36.0;    /* height of lowercase without ascenders */
	asc_height = 250.0/36.0;    /* height of lowercase ascenders */
	cap_height = 246.0/36.0;    /* height of caps */
	math_axis = 90.0/36.0;    /* axis of symmetry for math symbols */
	rule_thickness = 0.4;    /* thickness of lines in math symbols */
	cap_curve = 35.0/36.0; /* uppercase curve breadth */
	hair = 9.0/36.0;		/* lowercase hairline breadth */
	vair = 8.0/36.0;		/* verticaldiameter of hairlines */
	tiny = 8.0/36.0;		/* diameter of rounded corners */
	slab =  11.0/36.0;    /* serif and arm thickness */
	beak_jut =  10.0/36.0;    /* horizontal protrusion of beak serifs */
	beak =  70.0/36.0;    /* vertical protrusion of beak serifs */
	beak_darkness = 11.0/30.0;   /* fraction of triangle inside beak serifs */
	cap_stem = 30.0/36.0;    /* uppercase stem breadth */
	math_spread = 0.0;     /* extra openness of math symbols */
	lasy();
}

void lasyb10(void) {
	font_size = 10.0;
	u = 23.0/36.0;      /* unit width */
	letter_fit = 0.0;	/* extra sidebar space */
	o = 6.0/36.0;      /* amount of overshoot for curves */
	crisp = 13.0/36.0;    /* diameter of serif corners */
	bar = 17.0/36.0;      /* lowercase bar thickness */
	x_height = 160.0/36.0;    /* height of lowercase without ascenders */
	asc_height = 250.0/36.0;    /* height of lowercase ascenders */
	cap_height = 247.0/36.0;    /* height of caps */
	math_axis = 90.0/36.0;    /* axis of symmetry for math symbols */
	rule_thickness = .6;    /* thickness of lines in math symbols */
	cap_curve = 53.0/36.0;    /* uppercase curve breadth */
	hair = 17.0/36.0;    /* lowercase hairline breadth */
	vair = 13.0/36.0;    /* vertical diameter of hairlines */
	tiny = 13.0/36.0;    /* diameter of rounded corners */
	slab =  17.0/36.0;    /* serif and arm thickness */
	beak_jut =  11.0/36.0;    /* horizontal protrusion of beak serifs */
	beak =  70.0/36.0;    /* vertical protrusion of beak serifs */
	beak_darkness = .4;    /* fraction of triangle inside beak serifs */
	cap_stem = 50.0/36.0;    /* uppercase stem breadth */
	math_spread = 0.5;     /* extra openness of math symbols */
	lasy();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void dolasy(void) {
	lasy_unomega();
	lasy_leadsarrow();
	lasy_leadsto();
	lasy_leftward();
	lasy_rightward();
	lasy_upward();
	lasy_downward();
	lasy_lhdtria();
	lasy_unlhdtria();
	lasy_rhdtria();
	lasy_unrhdtria();
	lasy_square(); 
	lasy_diamond();
	lasy_bowtie(); 
	lasy_subset();
	lasy_superset(); 
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *ext) { /* supply extension if none */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void makelasy(char *fname, void (*fontfunc)(void)) {

	strcpy(fn_out, fname);
	if (verboseflag != 0) printf("Making font %s\n", fname); 
	extension(fn_out, "raw");
	if ((output = fopen(fn_out, "w")) == NULL) {
		perror(fn_out); exit(3);
	}

/*	slant=0.0;	*/
	fontfunc();	/* set up parameters for this font */

	fprintf(output, "%lg %lg\n", font_size, 10.0);	/* design size & scale */
	fprintf(output, "%d %d %d %d\n", 0, -200, 800, 700); /* BBox */
/*		(int) (1000.0 * (18.0 * u + 2.0 * s) / font_size), 
			(int) (1000.0 * ht / font_size)); */
	fprintf(output, "%%%% FontName: %s\n", fname);
	
	dolasy();	/* do all characters */

	if (ferror(output) != 0) {
		perror(fn_out); exit(4);
	}
	else fclose(output);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	
	makelasy("lasy10", lasy10);
	makelasy("lasy9", lasy9);
	makelasy("lasy8", lasy8);
	makelasy("lasy7", lasy7);
	makelasy("lasy6", lasy6);
	makelasy("lasy5", lasy5);
	makelasy("lasyb10", lasyb10); 
	return 0;
}

/* we assume that slant will always be zero here */
/* we assume that the pen is always circular here */
/* otherwise need to use mapit and unmapit */

/* angles are in degrees, NOT radians */

/* watch out for `add' and `sub' - instead of `sum' and `diff' */

/* needs to be compiled in 'MEDIUM' model - otherwise too big */

/* needs enlarge so stack is big enough */
