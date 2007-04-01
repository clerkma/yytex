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

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>		/* for sqrt */

#define MAXLINE 256
/* #define FNAMELEN 80 */
/* #define MAXPATH 512 */
#define MAXPATH 300

#define NOHINT 0

#define PI 3.141592653

#define MAXCHRS 256

#define MAXCHARNAME 64

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

typedef struct {
	double x;
	double y;
} VECTOR;

typedef struct {
	VECTOR start;
	VECTOR cst;
	VECTOR cen;
	VECTOR end;
	double ast;
	double aen;	
	int hint;
	char code;
} STEP;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double eps = 6.0;					/* 10.0 offset for swelling */

/* double vernier = 2.0; */			/* eps magnification in beak */

/* double lineareps = 0.1; */		/* colinear threshold */
double lineareps = 0.5;				/* colinear threshold */

double sigma = 6.0;					/* parameter on offset curve was 4.0 */
									/* disabled if sigma == 0.0 */
									/* prevents large offset of new corner */

double maxdist = 6.0;				/* maximum offset from actual segment */

double mindeter = 0.000001;			/* minimum acceptable determinant */

double epsilon = 0.0001;			/* used in path comparison */

/* double beakthres = (PI/8); */	/* PI/8 radian difference 22.5 degrees */
/* double beakthres = (PI/10); */	/* PI/10 radian difference 18 degrees */
double beakthres = 0.1;				/* accept almost all for new method */
/* double parthres = (PI/3); */		/* PI/6 radian difference 30 degrees */
double parthres = (PI/2);			/* PI/2 radian difference 90 degrees */
/*  To get intersection	on the curveto's themselves need at least PI/2 */

int verboseflag=0;
int traceflag=0;
int debugflag=0;
int detailflag=0;
int dotsflag=1;						/* show dot for each character */
int showalphabugs=0;				/* show problems with alpha calcs */
int writeclosing=1;					/* write closing lineto */
int checkbad=1;						/* check for bad outline */
int trimlost=1;						/* check for lost segments */
int swelling=1;						/* perform the swelling operation */
int showold=1;						/* show old font level hints */
int showeps=1;						/* show epsilon used */
int adjustcontrol=0;				/* adjust control points NOT ANYMORE */
int tunecontrol=1;					/* adjust control points (alpha/beta) */
int tweakcontrol=1;					/* adjust control points (ends slide) */
int ignorelarge=0;					/* don't attempt to adjust large offset */
int distantflag=1;					/* watch out for distant intersections */
int avoidshort=1;					/* avoid short ends right off */
									/* for italic fonts use -e arg */
int shorttangent=1;					/* use tangent if knot on end point */
int averageflag=1;					/* bad intersections, use average */
int wraparound = 1;					/* wrap around path */
int beakflag=0;						/* show potential beaks */
int avoidbeak=1;					/* try and avoid beaks right off */
int otherwayflag=1;					/* eliminate more short segments */
int endirection=1;					/* use tangents at end in flush decision */

char *afmfilename=NULL;				/* AFM file name if given */

int fontographer=0;					/* adjustments for swelling Fontographer */

double adjustby=1.0;				/* modulate the adjustment */

int hinting=0;

int alphaflag=0;					/* computing alphas - be gentle */

char line[MAXLINE];

char oldline[MAXLINE];

char buffer[MAXLINE];

char *fontname=NULL;

int pathindex;						/* elements in path */

int chr;							/* current character code */

char charname[MAXCHARNAME];			/* current character name */

int currindex;						/* current index into path */

int xll[MAXCHRS], yll[MAXCHRS], xur[MAXCHRS], yur[MAXCHRS]; /* char bbox */

STEP path[MAXPATH];

VECTOR saved_start[MAXPATH];	/* save old corner positions 95/June/10 */
VECTOR saved_end[MAXPATH];		/* save old corner positions 95/June/10 */

int proper[MAXPATH];			/* intersection of k-1 and k done properly */
								/* don't adjust controls if set 95/June/10 */
								

VECTOR intersection;

double alpha, beta;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int intersect_various(STEP, STEP);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showvector(VECTOR a) {
	printf(" (%lg %lg) ", a.x, a.y);
}

void showsegment (STEP segment) {
	switch (segment.code) {
		case 'C':
/*			printf("\nCURVETO (%lg %lg) (%lg %lg) (%lg %lg) (%lg %lg) ",
				segment.start.x, segment.start.y,
					segment.cst.x, segment.cst.y,
						segment.cen.x, segment.cen.y,
							segment.end.x, segment.end.y); */
			printf("\nCURVETO");
			showvector(segment.start);
			showvector(segment.cst);
			showvector(segment.cen);
			showvector(segment.end);
			break;
		case 'L':
/*			printf("\nLINETO (%lg %lg) --- (%lg %lg) ",
				segment.start.x, segment.start.y,
					segment.end.x, segment.end.y); */
			printf("\nLINETO");
			showvector(segment.start);
			showvector(segment.end);
			break;
		default:
			printf("UNKNOWN CODE!\n");
			break;
	}
}

void showcurrent (int k) {
	showsegment(path[k]);
}

/* void make_vector (double x, double y, VECTOR result) {
	result.x = x; result.y = y;
} */

int vector_zero (VECTOR a) {
	return (a.x == 0.0 && a.y == 0.0);
}

int vector_eq (VECTOR a, VECTOR b) {
	return (a.x == b.x && a.y == b.y);
}

/* These are safe when result is the same as one of the argument */

void vector_scale (VECTOR a, double k, VECTOR *result) {
	result->x = a.x * k;
	result->y = a.y * k;
}

void vector_dif (VECTOR a, VECTOR b, VECTOR *result) {
	result->x = a.x - b.x;
	result->y = a.y - b.y;
}

void vector_sum (VECTOR a, VECTOR b, VECTOR *result) {
	result->x = a.x + b.x;
	result->y = a.y + b.y;
}

void turn_90 (VECTOR a, VECTOR *result) {
	double temp;
	temp = a.x;			/* just in case result == a */
	result->x = a.y;
/*	result->y = -a.x; */
	result->y = -temp;
}

/* result = a + (b - c) * k; */

void sum_scale_dif (VECTOR a, VECTOR b, VECTOR c, double scale,
					VECTOR *result) {
	VECTOR b_c;
	VECTOR scaled_b_c;

	vector_dif (b, c, &b_c);
	vector_scale (b_c, scale, &scaled_b_c);
	vector_sum (a, scaled_b_c, result);
}

double dot_product (VECTOR a, VECTOR b) {
	return a.x * b.x + a.y * b.y ;
}

double triple_cross (VECTOR a, VECTOR b) {	/* positive if turn a->b is CCW */
	return a.x * b.y - a.y * b.x;
}

double mag (VECTOR a) {
	return sqrt(dot_product (a,a));
}

double mag_dif (VECTOR a, VECTOR b) {
	VECTOR difference;
	vector_dif(a, b, &difference);
	return sqrt(dot_product (difference, difference));
}

double cos_direct (VECTOR a, VECTOR b) { /* cosine angle between directions */
	if (mag(a) == 0.0 || mag(b) == 0.0) {
		printf(" ZERO VECTOR COS ");
		return 0.0;	/* error */
	}
	return dot_product (a, b) / (mag(a) * mag(b));
}

/* positive means turning a -> b is anti-clockwise */

double sin_direct (VECTOR a, VECTOR b) { /* sine angle between directions */
	if (mag(a) == 0.0 || mag(b) == 0.0) {
		printf(" ZERO VECTOR SIN ");
		return 0.0;	/* error */
	}
	return triple_cross (a, b) / (mag(a) * mag(b));
}

/* true if x lies within the band perpendicular to the line from a to b */

int within (VECTOR a, VECTOR x, VECTOR b) {
	VECTOR b_a, x_a, b_x;
	vector_dif (b, a, &b_a);
	vector_dif (x, a, &x_a);
	vector_dif (b, x, &b_x);
	return (dot_product (x_a, b_a) > 0 && dot_product (b_x, b_a) > 0);
}

void vector_aver (VECTOR a, VECTOR b, VECTOR *result) {
	vector_sum(a, b, result);
	vector_scale(*result, 0.5, result);	
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* compute sideway offset for points on vector from a to b */
/* the distance to offset is eps, and the direction is to the left */
/* simply go at right angles to the connecting line distance eps */
/* returns non-zero if it fails (zero length) */

int offset (VECTOR a, VECTOR b, VECTOR *result) {
	VECTOR b_a;
	VECTOR turn_b_a;
	double mag_b_a;

	vector_dif (b, a, &b_a);
	mag_b_a = mag(b_a);
	if (mag_b_a == 0.0) {
		if (alphaflag == 0 || showalphabugs) {
			fprintf(stderr,
	"\nZero Magnitude (in offset) - char `%s' (%d) k %d ",
					charname, chr, currindex);
			showvector(a); showvector(b);
			showcurrent(currindex);
		}
		result->x = 0; result->y = 0;
		return -1;					/* bad */
	}
	else {
		turn_90 (b_a, &turn_b_a);
		vector_scale(turn_b_a, eps / mag_b_a, result);
		return 0;					/* OK */
	}
}

/* compute the offset from the corner b to the intersection */
/* of the lines offset from a -> b by eps and offset from c -> b by eps */

int corner (VECTOR a, VECTOR b, VECTOR c, VECTOR *result) {
	double denom;
	VECTOR b_a, c_b;
	VECTOR turn_b_a, turn_c_b;
	VECTOR scaled_b_a, scaled_c_b;
	double mag_b_a, mag_c_b;

	vector_dif (b, a, &b_a);
	vector_dif (c, b, &c_b);
	mag_b_a = mag(b_a);
	mag_c_b = mag(c_b);
	if (mag_c_b == 0 && mag_b_a == 0) {
fprintf(stderr, "\nBoth Segments Zero Length (corner) in `%s' (%d) k %d ",
			charname, chr, currindex);
		showcurrent(currindex);
		result->x = 0; result->y = 0;
		return -1;			/* BAD */
	}
	else if (mag_c_b == 0.0) {		/* second segment zero length, ignore it */
		offset (a, b, result);
		return 0;
	}
	else if (mag_b_a == 0.0) {		/* first segment zero length, ignore it */
		offset (b, c, result);
		return 0;
	}
	
	denom = mag_b_a * mag_c_b + dot_product (b_a, c_b);
/*	if (denom == 0.0) { */
	if (fabs(denom) < mindeter) {
		fprintf(stderr, "\nParallel Segments in `%s' (%d) k %d ",
				charname, chr, currindex);
		showcurrent(currindex);
/*		result->x = 0; result->y = 0; */
/*		return -1; */		/* BAD ??? */
		offset (a, b, result); /* NEW */
/*		offset (b, c, result); */ /* SAME */
		return 0;			/* corner is offset this much anyway ??? NEW */
	}
	else {
		turn_90 (b_a, &turn_b_a);
		turn_90 (c_b, &turn_c_b);
		vector_scale (turn_b_a, eps * mag(c_b) / denom, &scaled_b_a),
		vector_scale (turn_c_b, eps * mag(b_a) / denom, &scaled_c_b);
		vector_sum (scaled_b_a, scaled_c_b, result);
		return 0;			/* OK */
	}
}

int intersect_lines (VECTOR, VECTOR, VECTOR, VECTOR, VECTOR *);

/* alternate corner finder */ /* returns corner, not offset */

int corner_alt (VECTOR a, VECTOR b, VECTOR c, VECTOR d, VECTOR *result) {
	VECTOR at, bt, ct, dt;
	VECTOR b_a, d_c;
	VECTOR delta_d_c, delta_b_a;
	double mag_b_a, mag_d_c;
	int flag=0;
	double denom;

	vector_dif (b, a, &b_a);
	vector_dif (d, c, &d_c);
	mag_b_a = mag(b_a);
	mag_d_c = mag(d_c);
/*	if (debugflag) 
		printf(" CORNER ALT |b-a| %lg |d-c| %lg ", mag_b_a, mag_d_c); */
	if (mag_d_c == 0 && mag_b_a == 0) {
fprintf(stderr, "\nBoth Segments Zero Length (corner_alt) in `%s' (%d) k %d ",
			charname, chr, currindex);
		result->x = (b.x + c.x) / 2.0;
		result->y = (b.y + c.y) / 2.0;
		return -1;
	}
	else if (mag_d_c == 0.0) {		/* second segment zero length, ignore it */
		offset (a, b, &delta_b_a);
		vector_sum (b, delta_b_a, result);
		return 0;
	}
	else if (mag_b_a == 0.0) {		/* first segment zero length, ignore it */
		offset (c, d, &delta_d_c);
		vector_sum (c, delta_d_c, result);
		return 0;
	}

	denom = mag_b_a * mag_d_c + dot_product (b_a, d_c); /* NEW */ 
/*	if (denom == 0.0) { */
/*	if (debugflag) printf(" DENOM %lg ", denom); */
	if (fabs(denom) < mindeter) {
		fprintf(stderr, "\nParallel Segments in `%s' (%d) k %d ",
				charname, chr, currindex);
		showcurrent(currindex);
		if (b.x == c.x && b.y == c.y) {
			offset (a, b, &delta_b_a);
			vector_sum (b, delta_b_a, result);	/* NEW */
/*			offset (c, d, &delta_d_c);
			vector_sum (c, delta_d_c, result); */ /* SAME */
			return 0;	/* corner is at anyway ??? */
		}
		else {
			result->x = (b.x + c.x) / 2.0;
			result->y = (b.y + c.y) / 2.0;
			return -1; 		/* BAD ??? */
		}
	}
	offset (a, b, &delta_b_a);
	offset (c, d, &delta_d_c);
	vector_sum (a, delta_b_a, &at);
	vector_sum (b, delta_b_a, &bt);
	vector_sum (c, delta_d_c, &ct);
	vector_sum (d, delta_d_c, &dt);
/*	if (debugflag) printf(" CALLING INTERSECT LINES "); */
/*	intersect_lines (a, b, c, d, result); */
	if (intersect_lines (at, bt, ct, dt, result) != 0) {
/*	shouldn't happen anymore ? */
		printf("\nFAILED TO INTERSECT ALT ");
/*		printf("(%lg %lg) (%lg %lg) (%lg %lg) (%lg %lg) ",
			   at.x, at.y, bt.x, bt.y, ct.x, ct.y, dt.x, dt.y); */
		showvector(at); showvector(bt); showvector(ct); showvector(dt);
		printf("RESULT (%lg %lg) ", result->x, result->y);
/*		printf("\nFROM (%lg %lg) (%lg %lg) (%lg %lg) (%lg %lg) ",
			   a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y); */
		printf("\nFROM");
		showvector(a); showvector(b); showvector(c); showvector(d);
		result->x = (bt.x + ct.x) / 2.0;
		result->y = (bt.y + ct.y) / 2.0;
		flag = 1;
	}
	return flag;
}

void show_bad (VECTOR a, VECTOR b, VECTOR c, VECTOR d) { /* Bad Intersection */
	fprintf(stderr,
			"\n(%lg %lg) (%lg %lg) (%lg %lg) (%lg %lg)",
			a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y);
}

/* intersect line from a to b with line from d to c */

int intersect_lines (VECTOR a, VECTOR b, VECTOR c, VECTOR d, VECTOR *result) {
	double axbymaybx, dycxmdxcy;
	double det;
	VECTOR inter;

/*	if (debugflag)
	printf(" INTERSECT `%s' (%d) k %d  ", charname, chr, currindex); */
	if (mag_dif(a,b) == 0 || mag_dif(c,d) == 0) {
		if (alphaflag == 0 || showalphabugs) {
fprintf(stderr, "\nZero length line (intersect_lines) char `%s' (%d) k %d : ",
					charname, chr, currindex);
			show_bad (a, b, c, d);
		}
		result->x = 0; result->y = 0;
		return -1;							/* BAD */
	}
	det = (d.y - c.y) * (b.x - a.x) - (b.y - a.y) * (d.x - c.x);
/*	if (det == 0.0) { */
	if (fabs(det) < mindeter) {
		if (alphaflag == 0 || showalphabugs) {
			fprintf(stderr, "\nParallel lines char `%s' (%d) k %d : ",
					charname, chr, currindex);
			show_bad (a, b, c, d);
		}
		result->x = 0; result->y = 0;
		return -1;							/* BAD */
	}
	axbymaybx = a.x * b.y - a.y * b.x;
	dycxmdxcy = d.y * c.x - d.x * c.y;
	inter.x = ((b.x - a.x) * dycxmdxcy - (d.x - c.x) * axbymaybx) / det;
	inter.y = ((b.y - a.y) * dycxmdxcy - (d.y - c.y) * axbymaybx) / det;
	result->x = inter.x;
	result->y = inter.y;
	if (distantflag) {
		if (mag_dif (inter, a) > (mag_dif (d, a) + maxdist) ||
			mag_dif (inter, d) > (mag_dif (a, d) + maxdist)) {
			if (alphaflag == 0 || showalphabugs) {
		fprintf(stderr, "\nDistant Intersection `%s' (%d) k %d at (%lg %lg)",
				   charname, chr, currindex, inter.x, inter.y);
		if (verboseflag)
			fprintf(stderr, "\ninter - a: %lg inter - d: %lg, d - a: %lg : ",
					mag_dif(inter, a), mag_dif(inter, d), mag_dif(d, a));
		if (verboseflag)
			show_bad (a, b, c, d);
			}
			result->x = 0; result->y = 0;
			return -1;		/* BAD */
		}
	}
	return 0;				/* OK */
}

/* check whether intersection of line from a to b and line from c to d */
/* lies within lines (rather than their extensions) */

int real_intersect (VECTOR a, VECTOR b, VECTOR c, VECTOR d, VECTOR *inter) {
/*	VECTOR x; */
/*	fail right away if lines parallel or intersection too far away */
	if (intersect_lines (a, b, c, d, inter) != 0) return 0;
	return (within (a, *inter, b) && within (c, *inter, d));
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double square (double x) {
	return x*x;
}

/* int isitbeak (int, int); */
int isitbeak (STEP, STEP, int);

int offset_step (STEP *a) {
	VECTOR deltas, deltae;
	int flag;

	flag = offset (a->start, a->cst, &deltas);
	if (flag != 0) {
		if (shorttangent) {	/* appropriate if a->start == a->cst */
			flag = offset (a->start, a->cen, &deltas);
			if (flag == 0) printf(" RESCUED SHORT TANGENT ");
		}
	}
	if (flag) {
		printf(" OFFSET FAILED ");
		showsegment(*a);
		return -1;
	}
	if (traceflag) printf ("\nOFFSET START (%lg %lg) ", deltas.x, deltas.y);
	flag = offset (a->cen, a->end, &deltae);
	if (flag != 0) {
		if (shorttangent) { /* appropriate if a->cen == a->end */
			flag = offset (a->cst, a->end, &deltae);
			if (flag == 0) printf(" RESCUED SHORT TANGENT ");
		}
	}
	if (flag) {
		printf(" OFFSET FAILED ");
		showsegment(*a);
		return -1;
	}
	if (traceflag) printf ("\nOFFSET END (%lg %lg) ", deltae.x, deltae.y);
	vector_sum (a->start, deltas, &a->start);
	vector_sum (a->cst, deltas, &a->cst);
	vector_sum (a->cen, deltae, &a->cen);
	vector_sum (a->end, deltae, &a->end);
	return 0;
}

/* shift a curveto outwards if possible */

int shift_knots (STEP step, STEP *res) {			/* 1995/June/8 */
	STEP new;
	VECTOR intersection;
	int flag;
	
/*	if (debugflag) printf(" shift_knots "); */
	if (vector_eq(step.start, step.cst)) {
		if (shorttangent) { /* appropriate if step.start == step.cst */
			if (vector_eq(step.start, step.cen)) return -1;
		}
		else return -1;
	}
	if (vector_eq(step.cen, step.end)) {
		if (shorttangent) { /* appropriate if step.cen == step.end */
			if (vector_eq(step.cst, step.end)) return -1;
		}
		else return -1;
	}
	new = step;						/* copy across all unchanged data */
/*	parallel offset the line from start to cst and the line from cen to end */
	if (offset_step (&new)) return -1;
/*	Now adjust two inner knots to new size - only for curveto's */
/*	if (debugflag) printf(" CALL INTERSECT LINES "); */
	if (step.code == 'C') {								/* 95/June/10 */
		intersect_lines (new.start, new.cst, new.cen, new.end, &intersection);
/*		if (debugflag) printf(" RETURN INTERSECT LINES "); */
		flag = vector_zero(intersection);
		if (flag) {
			if (shorttangent) {
				flag = 0;
				if (vector_eq(new.start, new.cst)) intersection = new.cen;
				else if (vector_eq(new.cen, new.end)) intersection = new.cst;
				else flag = 1;
				if (flag == 0) printf ("\nRESCUED SHORT TANGENT ");
			}
			if (flag) {
				printf("\nBAD INTERIOR SHIFTKNOTS ");
				return -1;
			}
		}
		if (averageflag == 0) {
			if (step.ast == 0.0 || step.aen == 0.0) return -1;
		}
		sum_scale_dif (new.start, intersection, new.start,
				   step.ast,  &new.cst);
		sum_scale_dif (new.end, intersection, new.end,
				   step.aen, &new.cen);
	} 
	*res = new;
	return 0;
}

/* answer in global var intersection - also alpha & beta */
/* -1 => no intersection 0 => don't know +1 => intersection */

int intersect_curves (STEP, STEP);

int doreal_intersect (STEP before, STEP after) {		/* 95/June/8 */
	STEP before_new, after_new;
	int flag;

/*	shift all knots of `before' and `after' outward */
	if (shift_knots (before, &before_new)) {
		printf ("\nFAILED shift_knots BEFORE ");
		return -1;
	}
	if (shift_knots (after, &after_new)) {
		printf ("\nFAILED shift_knots AFTER ");
		return -1;
	}
	flag = intersect_curves (before_new, after_new);
	if (flag <= 0)
		printf("\nFAILED INTERSECT_CURVES in `%s' (%d) REAL", charname, chr);
	if (flag < 0) return -1;
	if (flag == 0) return -1;
	return 0;
}

void split_curve (STEP, double, int, STEP *);

/* shifts all control points outward perpendicular to line */
/* for corners, find intersection of lines shifted outward parallel */

void doswelling () {
	int k, flag, beakflag, realflag;
	VECTOR corneroffset, beforeoffset, afteroffset;
	VECTOR cornerpoint;
	VECTOR a, b, c, d;
	double doffset, len, scaling;
	STEP segment;		/* remember for next move */
	STEP before, after;
	STEP before_new, after_new;
	VECTOR middle;

	segment = path[pathindex-1];			/* set first time to `previous' */
	if (traceflag) printf("Entering swelling operation\n");
	for (k = 0; k < pathindex-1; k++) {
		currindex = k;				/* set global for error message */
/* first deal with case of a segment that got discarded */
		if (! vector_eq (path[k].end, path[k+1].start)) {
			if (trimlost == 0)
			fprintf(stderr,
				"\nERROR: End of %d does not match start of %d", k, k+1);
			else if (verboseflag) fprintf(stderr,
				"\nWARNING: End of %d does not match start of %d", k, k+1);

/* 95/June/10 - try dorealintersect! */
			if (verboseflag) printf(" DOREALINTERSECT ");

/*			realflag = doreal_intersect (segment, path[k+1]); */
/*	shift all knots of `before' and `after'  curveto's outward */
			flag = 0;
/*			if (shift_knots (before, &before_new)) { */ /* before = segment */
			if (shift_knots (segment, &before_new)) {
				printf ("\nFAILED shift_knots BEFORE ");
				flag = -1;
			}
/*			if (shift_knots (after, &after_new)) { */ /* after = path[k+1] */
			if (shift_knots (path[k+1], &after_new)) {
				printf ("\nFAILED shift_knots AFTER ");
				flag = -1;
			}
/*			if (debugflag) printf(" CALLING INTERSECT CURVES "); */
			if (flag == 0) {
				flag = intersect_curves (before_new, after_new);
				if (flag <= 0) {
					printf("\nFAILED INTERSECT_CURVES in `%s' (%d) GAP ",
						  charname, chr);
					if (traceflag) {
						showsegment(before_new);
						showsegment(after_new);
					}
				}
			}
			if (flag > 0) {
/*			if (realflag == 0)  */
/* following no longer used */
				vector_dif (intersection, segment.end, &corneroffset);
				beakflag=1;
				if (verboseflag) {
					printf (" INTERSECT (%lg %lg) ",
						intersection.x, intersection.y); 
					printf (" OFFSET (%lg %lg) ",
							corneroffset.x, corneroffset.y);
				}
				segment = path[k+1]; /* save now - it will be modified */
				if (traceflag) printf(" ALPHA %lg BETA %lg ", alpha, beta);
				proper[k+1] = 1;/* mark corner between k and k+1 as done */
/* need to split OFFSET curves ! */
/*				split_curve (segment, alpha, 1, &before);  */
				split_curve (before_new, alpha, 0, &before); 
				if (traceflag) {
					showsegment (before_new); /* ??? */
					showsegment (before); /* ??? */
				}
/*				split_curve (segment, beta, 0, &before); */
/*				don't overwrite path[k].start and path[k].cst */
				path[k].cen = before.cen;
				path[k].end = before.end;
/* need to split OFFSET curves ! */
/*				split_curve (path[k+1], beta, 0, &after);  */
				split_curve (after_new, beta, 1, &after); 
				if (traceflag) {
					showsegment (after_new); /* ??? */
					showsegment (after); /* ??? */
				}
/*				split_curve (path[k+1], 1 - alpha, 1, &after); */
/*				don't overwrite path[k+1].cen and path[k+1].end */
				path[k+1].start = after.start;
				path[k+1].cst = after.cst;
				continue;	/* we have done it all ! */
			} /* end of flag > 0 (intersect curves worked) */

			a = path[k].cen;
			b = path[k].end;		/* end of previous segment */
			c = path[k+1].start;	/* start of following segment */
			d = path[k+1].cst;
/*	hack to avoid crazy short control distance influence */
			if (avoidshort) {
			if ((len = mag_dif(a, b)) < eps) {
				if (verboseflag) {
printf("\nA too close to B (%lg) in `%s' (%d) k %d - GAP - using whole curve ", 
					   len, charname, chr, k);
/*					showcurrent(k); */
					showsegment(segment);	/* show save version */
				}
/*				a = path[k].start; */
				a = segment.start;	/* use saved version instead ! */
			}
			if ((len = mag_dif(c, d)) < eps) {
				if (verboseflag) {
printf("\nC too close to D (%lg) in `%s' (%d) k %d - GAP - using whole curve ",
					   len, charname, chr, k);
					showcurrent(k+1);
				}
				d = path[k+1].end;
			}  
			} /* end of avoidshort */
/*			printf(" USING CORNER ALT "); */
/*			segment = path[k+1]; */	/* save now since it will be modified */
			if (corner_alt (a, b, c, d, &cornerpoint) != 0) {
/* first try failed - so use whole curve */
				printf(" CORNER FAILED ");
				a = segment.start;
				d = path[k+1].end;
				if (corner_alt (a, b, c, d, &cornerpoint) == 0)
					printf(" RESCUED ");
				else printf(" FAILED AGAIN ");
			}
			else if (traceflag) printf(" CORNER ALT OK ");

			scaling = 1.0;		/* check scaling ??? *//* scaling not used ? */
/*			if (sigma != 0.0) */
			if (before.code == 'C' && after.code == 'C' && sigma != 0.0) {
				vector_aver(b, c, &middle);
				doffset = mag_dif(cornerpoint, middle);
				scaling = 1 / sqrt(1 + square(doffset / (eps * sigma)));
			}
			segment = path[k+1];	/* save now since it will be modified */
			vector_dif (cornerpoint, path[k].end, &beforeoffset);
			vector_dif (cornerpoint, path[k+1].start, &afteroffset);
/* could now do clever scaling on these as done below ... */
			vector_sum(path[k].cen, beforeoffset, &path[k].cen);
			vector_sum(path[k].end, beforeoffset, &path[k].end);
			vector_sum(path[k+1].start, afteroffset, &path[k+1].start);
			vector_sum(path[k+1].cst, afteroffset, &path[k+1].cst);
/*			if (debugflag) printf(" CORNER ALT FINISH "); */
		}

		else {  /* this is the normal branch - where there are no gaps */
/*		an experiment: lets do these early */
/*		offset(path[k].cen, path[k].end, &beforeoffset);  */
/*		offset(path[k+1].start, path[k+1].cst, &afteroffset); */
		a = path[k].cen;
		b = path[k].end;   /* =  path[k+1].start; */
		c = path[k+1].cst;
/*		corner (path[k].cen, path[k].end, path[k+1].cst, &corneroffset); */
		if (avoidshort) {
		if ((len = mag_dif(a, b)) < eps) {
			if (verboseflag) {
	printf("\nA too close to B (%lg) in %s (%d) k %d - using whole curve ", 
				   len, charname, chr, k);
/*				showcurrent(k); */
				showsegment(segment);
			}
/*			a = path[k].start; */
			a = segment.start;
		}
		if ((len = mag_dif(b, c)) < eps) {
			if (verboseflag) {
	printf("\nB too close to C (%lg) in %s (%d) k %d - using whole curve ",
				   len, charname, chr, k+1);
				showcurrent(k+1);
			}
			c = path[k+1].end;
		}  
		}  /* end of avoidshort */
/*		segment = path[k+1]; */	/* save now since it will be modified */
		if (corner (a, b, c, &corneroffset) != 0) {
			printf (" CORNER FAILED ");
			a = segment.start;
			c = path[k+1].end;
			if (corner (a, b, c, &corneroffset) == 0)
				printf(" RESCUED ");
			else printf(" FAILED AGAIN ");
		}
		beakflag = 0;
		realflag = 0;
/*		if (avoidbeak && isitbeak(k, 0)) 	*/	/* 95/June/6 */
		if (avoidbeak && isitbeak(segment, path[k+1], 0)) {	/* 95/June/7 */
			if (verboseflag) {
				printf("\nBEAK avoidance in `%s' (%d) k %d ",
				   charname, chr, k);
				if (segment.code != 'C' || path[k+1].code != 'C')
					printf(" %c & %c ", segment.code, path[k+1].code);
			}

/*			if (segment.code == 'C' && path[k+1].code == 'C')  { */
				if (verboseflag) {
					showsegment(segment);
					showsegment(path[k+1]);
				}				/* show before 95/June/8 */

				if (verboseflag) printf(" DOREALINTERSECT ");

/*				realflag = doreal_intersect (segment, path[k+1]); */
/*	shift all knots of `before' and `after'  curveto's outward */
				flag = 0;
/*			if (shift_knots (before, &before_new))  */ /* before = segment */
				if (shift_knots (segment, &before_new)) {
					printf ("\nFAILED shift_knots BEFORE ");
					flag = -1;
				}
/*			if (shift_knots (after, &after_new))  */ /* after = path[k+1] */
				if (shift_knots (path[k+1], &after_new)) {
					printf ("\nFAILED shift_knots AFTER ");
					flag = -1;
				}
/*				if (debugflag) printf(" CALLING INTERSECT CURVES "); */
				if (flag == 0) {
					flag = intersect_curves (before_new, after_new);
					if (flag <= 0) {
						printf("\nFAILED INTERSECT_CURVES in `%s' (%d) ",
							  charname, chr);
						if (traceflag) {
							showsegment(before_new);
							showsegment(after_new);
						}
					}
				}
				if (flag > 0) {
/*				if (realflag == 0)  */
/* following no longer used */
					vector_dif (intersection, segment.end, &corneroffset);
					beakflag=1;
					if (verboseflag) {
						printf (" INTERSECT (%lg %lg) ",
							intersection.x, intersection.y); 
						printf (" OFFSET (%lg %lg) ",
								corneroffset.x, corneroffset.y);
					}
					segment = path[k+1]; /* save now - it will be modified */
					if (traceflag) printf(" ALPHA %lg BETA %lg ", alpha, beta);
					proper[k+1] = 1;/* mark corner between k and k+1 as done */
/* need to split OFFSET curves ! */
/*					split_curve (segment, alpha, 1, &before);  */
					split_curve (before_new, alpha, 0, &before); 
					if (traceflag) {
						showsegment (before_new); /* ??? */
						showsegment (before); /* ??? */
					}
/*					split_curve (segment, beta, 0, &before); */
/*					don't overwrite path[k].start and path[k].cst */
					path[k].cen = before.cen;
					path[k].end = before.end;
/* need to split OFFSET curves ! */
/*					split_curve (path[k+1], beta, 0, &after);  */
					split_curve (after_new, beta, 1, &after); 
					if (traceflag) {
						showsegment (after_new); /* ??? */
						showsegment (after); /* ??? */
					}
/*					split_curve (path[k+1], 1 - alpha, 1, &after); */
/*					don't overwrite path[k+1].cen and path[k+1].end */
					path[k+1].start = after.start;
					path[k+1].cst = after.cst;
					continue;	/* we have done it all ! */
				} /* end of flag > 0 (intersect curves worked */
/*				else printf ("\nFAILED dorealintersect "); */
/*			} *//* end of two curveto's intersecting */
		} /* end of avoidbeak && isitbeak */

		scaling = 1.0;
/*		if (sigma != 0.0) { */ /* do only with curveto's */
		if (path[k].code == 'C' && path[k+1].code == 'C' && sigma != 0.0) {
			doffset = mag(corneroffset);
			scaling = 1 / sqrt(1 + square(doffset / (eps * sigma)));
			vector_scale (corneroffset, scaling, &corneroffset);
		}			
		offset(path[k].cen, path[k].end, &beforeoffset);  
/*		if (sigma != 0.0)  */
		if (scaling != 1.0)vector_scale (beforeoffset, scaling, &beforeoffset);
		if (vector_zero (beforeoffset)) beforeoffset = corneroffset;

		offset(path[k+1].start, path[k+1].cst, &afteroffset); 
/*		if (sigma != 0.0)  */
		if (scaling != 1.0)	vector_scale (afteroffset, scaling, &afteroffset);
		if (vector_zero (afteroffset)) afteroffset = corneroffset;

/*		if (beakflag) {
			printf("\nDOFFSET %lg SCALING %lg ", doffset, scaling); 
			printf("\nBEFOREOFFSET: (%lg %lg) ",
				beforeoffset.x, beforeoffset.y);
			printf("\nCORNEROFFSET: (%lg %lg) ",
				corneroffset.x, corneroffset.y);
			printf("\nAFTEROFFSET: (%lg %lg) ",
				afteroffset.x, afteroffset.y);
		}  */

		segment = path[k+1];	/* save now since it will be modified */
		vector_sum(path[k].cen, beforeoffset, &path[k].cen);
		vector_sum(path[k].end, corneroffset, &path[k].end);
		vector_sum(path[k+1].start, corneroffset, &path[k+1].start);
		vector_sum(path[k+1].cst, afteroffset, &path[k+1].cst);
/*		if (beakflag) {
			printf("\nBEFORE (%lg %lg) (%lg %lg) ",
	path[k].cen.x,  path[k].cen.y, path[k].end.x,  path[k].end.y);
			printf("\nAFTER (%lg %lg) (%lg %lg) ",
	path[k+1].start.x,  path[k+1].start.y, path[k+1].cst.x,  path[k+1].cst.y);
		} */
/*		path[k+1].start = path[k].end; */
		}
	}
}

void wrapends(void) { /*	Patch up first and last (which is a repeat of first) */
/*	path[0] = path[pathindex-1];	 */		/* wrong ! */
	path[0].start = path[pathindex-1].start;
	path[0].cst = path[pathindex-1].cst;
	path[pathindex-1].cen = path[0].cen;
	path[pathindex-1].end = path[0].end;
}

int readoutline (FILE *input) {
	int nocurrent=1, closed=0;
	int hint;
	double xa, ya, xb, yb, xc, yc;
	double xstart=0.0, ystart=0.0;	/* init to quiten compiler ... */
	double xlast=0.0, ylast=0.0;	/* init to quiten compiler ... */
	int hintwaiting=0;
	
	pathindex = 0;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (*line == ']') {				/* should not happen */
			if (! closed) fprintf(stderr, "\nPath not closed ");
			if (! nocurrent) fprintf(stderr, "\nPath not closed ");
			return 0;
		}
		if (strstr(line, " c") != NULL &&
			sscanf (line, "%lg %lg %lg %lg %lg %lg c",
				&xa, &ya, &xb, &yb, &xc, &yc) == 6) {
				if (nocurrent) fprintf(stderr, "\nNo current point ");
				path[pathindex].start.x = xlast;
				path[pathindex].start.y = ylast;
				path[pathindex].cst.x = xa;
				path[pathindex].cst.y = ya;
				path[pathindex].cen.x = xb;
				path[pathindex].cen.y = yb;
				path[pathindex].end.x = xc;
				path[pathindex].end.y = yc;
				path[pathindex].code = 'C';
				if (hintwaiting) {
					path[pathindex].hint = hint;
					hintwaiting=0;
				}
				else path[pathindex].hint = NOHINT;
				pathindex++;
				xlast = xc; ylast = yc;
		}
		else if (strstr(line, " l") != NULL &&
			sscanf (line, "%lg %lg l",
				&xa, &ya) == 2) {
				if (nocurrent) fprintf(stderr, "\nNo current point ");
				path[pathindex].start.x = xlast;
				path[pathindex].start.y = ylast;
				path[pathindex].cen.x = xlast;	/* redundant */
				path[pathindex].cen.y = ylast;	/* redundant */
				path[pathindex].cst.x = xa;		/* redundant */
				path[pathindex].cst.y = ya;		/* redundant */
				path[pathindex].end.x = xa;
				path[pathindex].end.y = ya;
				path[pathindex].code = 'L';
				if (hintwaiting) {
					path[pathindex].hint = hint;
					hintwaiting=0;
				}
				else path[pathindex].hint = NOHINT;
				pathindex++;
				xlast = xa; ylast = ya;
		}
		else if (strstr(line, " m") != NULL &&
			sscanf (line, "%lg %lg m",
				&xa, &ya) == 2) {
			if (! nocurrent) fprintf(stderr, "\nRepeated moveto ");
			xstart = xa; ystart = ya;
			xlast = xa; ylast = ya;
			nocurrent = 0;
			hintwaiting = 0;
		}
		else if (*line ==  'h') {
			if (xlast != xstart || ylast != ystart) {
				if (verboseflag) {
					printf("\nClosing path in `%s' (%d) ", charname, chr);
					printf("(%lg %lg) k %d ", xlast, ylast, pathindex);
				}
				path[pathindex].start.x = xlast;
				path[pathindex].start.y = ylast;
				path[pathindex].cen.x = xlast;	/* redundant */
				path[pathindex].cen.y = ylast;	/* redundant */
				path[pathindex].cst.x = xstart;	/* redundant */
				path[pathindex].cst.y = ystart;	/* redundant */
				path[pathindex].end.x = xstart;
				path[pathindex].end.y = ystart;
				path[pathindex].code = 'L';
				path[pathindex].hint = NOHINT;
				pathindex++;
			}
			if (wraparound) {
				path[pathindex] = path[0];
				pathindex++;
			}
			closed = 1;
			return 0;
		}
		else if (strstr(line, " s") != NULL &&
			sscanf(line, "%d s", &hint) == 1) {
			if (pathindex > 0) path[pathindex-1].hint = hint;
			else {
				path[pathindex].hint = hint;	/* initial hint switch */
				hintwaiting=1;
			}
/*			pathindex++; */
		}
		else {
			fprintf(stderr, "\nExpecting m, l, c, or h not %s", line);
		}
	}
	fprintf(stderr, "\nEOF ");
	return -1;
}

int colinear (VECTOR a, VECTOR b, VECTOR c) {
	double delx, dely, dell, rho;
	delx = c.x - a.x;
	dely = c.y - a.y;	
	dell = sqrt (delx * delx + dely * dely); 
	if (dell == 0.0) {
		printf("\nZERO LENGTH CURVETO (colinear) in `%s' (%d)", charname, chr);
		return 1;
	}
	rho = ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) / dell;
	if (rho > (- lineareps) && rho < lineareps) {
		if (traceflag) 
			printf("\nCOLINEAR rho %lg - %lg %lg %lg %lg %lg %lg",
				rho, a.x, a.y, b.x, b.y, c.x, c.y);
		return 1;	/* near colinear */
	}
	else return 0;							/* not near colinear */
}

/* check for bad path components such as zero length lines */

int checkoutline () {
	int k, m, flag=0;
	for (k = 0; k < pathindex; k++) {
		if (vector_eq (path[k].start, path[k].end)) {
			printf("\nBAD PATH COMPONENT char `%s' (%d) k %d\n",
			   charname, chr, k);
			showsegment(path[k]);
/*			fprintf(stderr, "%lg %lg m\n", path[k].start.x, path[k].start.y);
			if (path[k].code == 'L') {
				fprintf(stderr, "%lg %lg l\n", path[k].end.x, path[k].end.y);
			}
			else if (path[k].code == 'C') {
				fprintf(stderr, "%lg %lg %lg %lg %lg %lg c\n",
					path[k].cst.x, path[k].cst.y,
						path[k].cen.x, path[k].cen.y,
							path[k].end.x, path[k].end.y);
			} */
			flag++; 
			for (m = k; m < pathindex; m++) path[m] = path[m+1];
			pathindex--;
		}
		if (path[k].code == 'C') {
			if (vector_eq(path[k].start, path[k].cst) &&
				vector_eq(path[k].cen, path[k].end)) {
			printf("\nCURVETO IN `%s' (%d) k %d changed to LINETO",
				   charname, chr, k);
			showsegment(path[k]);
/*	printf("\ncurveto %d `%s' %d changed to lineto %lg %lg %lg %lg",
		chr, charname, k, path[k].start.x, path[k].start.y,
			path[k].end.x, path[k].end.y); */
				path[k].cst = path[k].end;
				path[k].cen = path[k].start;
				path[k].code = 'L';
			}
		}
		if (path[k].code == 'C') {
			if (colinear(path[k].start, path[k].cst, path[k].end) &&
				colinear(path[k].start, path[k].cen, path[k].end)) {
			printf("\nCURVETO IN `%s' (%d) k %d changed to LINETO",
				   charname, chr, k);
			showsegment(path[k]);
/*	printf("\ncurveto %d `%s' %d changed to lineto %lg %lg %lg %lg",
		chr, charname, k, path[k].start.x, path[k].start.y,
			path[k].end.x, path[k].end.y); */
				path[k].cst = path[k].end;
				path[k].cen = path[k].start;
				path[k].code = 'L';
			}
		}
	}
	return flag;
}

/* check for short segments that should get lost */
/* note path[pathindex-1] = path[0] */

/* instead: look for segments shorter than eps !!! */
/* actually: look for segments which when projected on edge are shorter */

void checklost () {	
	int k, m, flag;
	double len, sinb, sina, cosb, cosa;
	VECTOR zero, before, current, after;
/*	double len_before, len_after; */

	zero.x = 0; zero.y = 0;

	if (traceflag) printf("\nChecking for lost segments - ");
	if (pathindex < 5) {
		if (verboseflag)
		printf ("\nTRIANGLE in `%s' (%d) k %d ", charname, chr, pathindex);
		return; 			/* avoid cutting triangles */
	}
/*	possible problem if we cut segment from triangle ! */	

	alphaflag = 1;
	for (k = 0; k < pathindex-1; k++) {
		flag = 0;
		len = mag_dif (path[k].start, path[k].end);
/* do this better vernier depends on angles of intersection */
/* one of intersection further from corner than other end of line */
		if (len > eps * 10.0) continue;		/* unlikely to be a problem */
		if (len == 0.0) flag = 1;			/* flush this for sure ! */
/*		if (len < eps) flag = 1; */			/* flush this for sure */
/*		don't flush if turning through appreciable angle ? */

		vector_dif(path[k].end, path[k].start, &current);

/*		now do one way - direction before & after is direction of whole */
		if (endirection) {
			if (k > 0) vector_dif(path[k-1].end, path[k-1].cen, &before); 
			else vector_dif(path[pathindex-2].end, path[pathindex-2].cen,
						&before); 
		}
		if (!endirection || mag(before) < eps) {
			if (k > 0) vector_dif(path[k-1].end, path[k-1].start, &before);
/*		else before = zero; */ 		/* wrap around ? */
			else vector_dif(path[pathindex-2].end, path[pathindex-2].start,
						&before); 
		}
		if (endirection) {
			if (k < pathindex) vector_dif(path[k+1].cst, path[k+1].start, &after);
			else vector_dif(path[1].cst, path[1].start, &after);
		}
		if (!endirection || mag(after) < eps) {
			if (k < pathindex) vector_dif(path[k+1].end, path[k+1].start, &after);
/*		else after = zero; */ 		/* wrap around ? */
			else vector_dif(path[1].end, path[1].start, &after); 
		}

		if (mag(before) > 0.0 && mag(current) > 0.0) {
			sinb = sin_direct (before, current);
			cosb = cos_direct (before, current);
			if (sinb < 0 && cosb < 0 && len * (-sinb) < eps) {
				if (traceflag) {
				printf("\nSINB %lg COSB %lg LEN %lg ", sinb, cosb, len);
				showvector(before); showvector(current);
				showsegment(path[k-1]);
				showsegment(path[k]);
				}
				flag = 1;
			}
		}
		if (mag(after) > 0.0 && mag(current) > 0.0) {
			sina = sin_direct (current, after);
			cosa = cos_direct (current, after);
			if (sina < 0 && cosa < 0 && len * (-sina) < eps) {
				if (traceflag) {
				printf("\nSINA %lg COSA %lg LEN %lg ", sina, cosa, len);
				showsegment(path[k]);
				showsegment(path[k+1]);
				}
				flag = 1;
			}
		}


		if (otherwayflag) {
/*		now do the other way also - direction before & after is just tangent */
		if (k > 0) vector_dif(path[k-1].end, path[k-1].cen, &before); 
/*		if (k > 0) vector_dif(path[k-1].end, path[k-1].start, &before); */
/*		else before = zero; */ 		/* wrap around ? */
		else vector_dif(path[pathindex-2].end, path[pathindex-2].start,
						&before); 
		if (k < pathindex) vector_dif(path[k+1].cst, path[k+1].start, &after);
/*		if (k < pathindex) vector_dif(path[k+1].end, path[k+1].start, &after); */
/*		else after = zero; */ 		/* wrap around ? */
		else vector_dif(path[1].end, path[1].start, &before); 
		if (mag(before) > 0.0 && mag(current) > 0.0) {
			sinb = sin_direct (before, current);
			cosb = cos_direct (before, current);
			if (sinb < 0 && cosb < 0 && len * (-sinb) < eps) {
				if (traceflag) {
				printf("\nSINB %lg COSB %lg LEN %lg ", sinb, cosb, len);
				showvector(before); showvector(current);
				showsegment(path[k-1]);
				showsegment(path[k]);
				}
				flag = 1;
			}
		}
		if (mag(after) > 0.0 && mag(current) > 0.0) {
			sina = sin_direct (current, after);
			cosa = cos_direct (current, after);
			if (sina < 0 && cosa < 0 && len * (-sina) < eps) {
				if (traceflag) {
				printf("\nSINA %lg COSA %lg LEN %lg ", sina, cosa, len);
				showsegment(path[k]);
				showsegment(path[k+1]);
				}
				flag = 1;
			}
		}
		}

/*		if (flag == 0 && len < eps) {	
			printf("\nNO SKIP !!! in `%s' (%d) k %d length %lg --- ",
				charname, chr, k, len);
			printf("\nSINB %lg COSB %lg LEN %lg ", sinb, cosb, len);
			showsegment(path[k-1]);
			showsegment(path[k]);
			printf("\nSINA %lg COSA %lg LEN %lg ", sina, cosa, len);
			showsegment(path[k]);
			showsegment(path[k+1]);
		} */ /* possible error warning - short segment not flushed */
/*		if (len < eps * vernier) { */
		if (flag) {
		printf("\nSKIP segment in `%s' (%d) k %d length %lg --- eliminating ",
				charname, chr, k, len);
			if (verboseflag)
				showcurrent(k);			/* this one will be flushed */
			for (m = k; m < pathindex-1; m++) {
				path[m] = path[m+1];
			}
			pathindex--;
		}
	}
	if (traceflag) printf(" finished\n");
	alphaflag = 0;
}

double angle_sum (double alpha, double beta) {
	double cost, sint, result;
	
/*	result = alpha + beta; */
	cost = cos (alpha) * cos (beta) - sin (alpha)  * sin (beta);
	sint = sin (alpha) * cos (beta) + cos (alpha)  * sin (beta);
	result = atan2 (sint, cost);
	return result;
}

double angle_aver (double alpha, double beta) {
	double cost, sint, result;
	
/*	result = (alpha + beta) / 2.0; */
	cost = cos (alpha) + cos (beta);
	sint = sin (alpha) + sin (beta);
	result = atan2 (sint, cost);
	return result;
}

/* Difference in direction - deals with wrap-around at 2 PI */

double angle_dif (double alpha, double beta) {
	double cost, sint, result;
/*	result = alpha - beta; */
	cost = cos (alpha) * cos (beta) + sin (alpha)  * sin (beta);
	sint = sin (alpha) * cos (beta) - cos (alpha)  * sin (beta);
	result = atan2 (sint, cost);
	return result;
}

double direction (VECTOR a, VECTOR b) {
	double x, y;
	VECTOR dif;

	vector_dif (b, a, &dif);
	x = dif.x; y = dif.y;
	if (x == 0.0 && y == 0.0) {
		printf("\nERROR: direction of zero length vector ");
		return 0.0;
	}
	else return atan2(y, x);
}

int isitbeak (STEP a, STEP b, int flag) {
	double theta1, theta2, theta3, theta4, thetaclose, thetafar, delta;

/*	if (a.code == 'L' || b.code == 'L') return 0; */	/* for now */
	if (traceflag) printf(" CHECK BEAK `%s' (%d)", charname, chr);
	if (shorttangent) {
		if (vector_eq(a.cen, a.end)) theta1 = direction(a.cst, a.end);
		else theta1 = direction(a.cen, a.end);
		if (vector_eq(b.start, b.cst)) theta3 = direction(b.start, b.cen);
		else theta3 = direction(b.start, b.cst);
	}
	else {
		if (vector_eq(a.cen, a.end) || vector_eq(b.start, b.cst)) {
	printf("\nZERO LENGTH in BEAK TEST `%s' (%d) k %d (%lg %lg) (%lg %lg) ",
			charname, chr, currindex, a.cen, a.end, b.start, b.cst);
			return 0;
		}
		theta1 = direction(a.cen, a.end);
		theta3 = direction(b.start, b.cst);
	}
	delta = angle_dif (theta1, theta3);
	if (fabs (delta) < parthres) return 0;	/* not enough turning */
	if (delta < 0.0) return 0;				/* turning CCW ??? */

	theta2 = direction(a.start, a.end);
	theta4 = direction(b.start, b.end);
	thetaclose = angle_aver (theta1, theta3);
	thetafar = angle_aver (theta2, theta4);
	delta = angle_dif (thetaclose, thetafar);
	if (beakthres == 0.0 || fabs(delta) > beakthres) {
		if (flag) {
	printf("\nBEAK in `%s' (%d)  close: %lg far: %lg (%lg %lg)",
		   charname, chr, thetaclose, thetafar,
			   a.end.x, a.end.y);
	printf("\nclose: (%lg %lg) far: (%lg %lg) ",
		   theta1, theta3, theta2, theta4);
		}
		if (verboseflag) {
		if (a.code == 'L' || b.code == 'L') {
			printf("\nBEAK in `%s' (%d) k %d %c & %c ",
				   charname, chr, currindex, a.code, b.code);
		}
		}
		return -1;
	}
	return 0;
}

/* compute interior `corner' points of curveto segments */
/* and then compute fractional offsets of control points */

int computealphas () {
	int k, flag=0;
	double dstart, dend, astart, aend;
	double alpha_start, alpha_end;
	VECTOR intersection;

	alphaflag = 1;

	for (k = 0; k < pathindex; k++) {
		if (path[k].code != 'C') continue;
		if (vector_eq (path[k].start, path[k].end)) {
		printf("\nBAD PATH COMPONENT char `%s' (%d) k %d ", charname, chr, k);
			showsegment(path[k]);
/*		fprintf(stderr, "%lg %lg m\n", path[k].start.x, path[k].start.y); */
		}
/*		intersection = intersect (path[k].start, path[k].cst,
			path[k].cen, path[k].end);*/
		if (shorttangent) {
			if (vector_eq(path[k].start, path[k].cst)) {
				intersection = path[k].cen;
			}
			if (vector_eq(path[k].cen, path[k].end)) {
				intersection = path[k].cst;
			}
			else intersect_lines (path[k].start, path[k].cst, path[k].cen,
				   path[k].end, &intersection);
		}
		else {
			intersect_lines (path[k].start, path[k].cst, path[k].cen,
				   path[k].end, &intersection);
		}
		if (averageflag) {
			if (vector_zero(intersection)) {
				printf("\nUsing average of knots IN `%s' (%d) k %d ",
					   charname, chr, k);
				vector_aver(path[k].cst, path[k].cen, &intersection);
			}
		}
		if (vector_zero(intersection)) {	/* not good intersection */
			path[k].ast = 0.0;
			path[k].aen = 0.0;
			flag++;
		}
		else {	
			dstart = mag_dif (intersection, path[k].start);
			astart = mag_dif (path[k].cst, path[k].start);
			dend = mag_dif (intersection, path[k].end);
			aend = mag_dif (path[k].cen, path[k].end);
			if (dstart != 0.0) alpha_start = astart / dstart;
			else {
				printf(" INTERSECT at START in `%s' (%d) k %d ",
					  charname, chr, k);
				showsegment(path[k]);
				alpha_start = 0.0;
			}
			path[k].ast = alpha_start;
			if (dend != 0.0) alpha_end = aend / dend;
			else {
				printf(" INTERSECT at END in `%s' (%d) k %d ",
					  charname, chr, k);
				showsegment(path[k]);
				alpha_end = 0.0;
			}
			path[k].aen = alpha_end;
			if (alpha_start < 0.0 || alpha_end < 0.0) {
				printf ("\nNegative alphas %k %k %lg %lg",
					chr, k, alpha_start, alpha_end);
				if (alpha_start < 0.0) alpha_start = 0.0;
				if (alpha_end < 0.0) alpha_end = 0.0;
			}
		}
	}
	alphaflag = 0;
	return flag;
}

void savecorners(void) {
	int k;
	for (k = 0; k < pathindex; k++) {
		saved_start[k] = path[k].start;
		saved_end[k] = path[k].end;
		proper[k] = 0;
	}
	proper[pathindex]=0;
}

void docontrols () {
	int k, flag=0;
	int outward;
	double xtotal, ytotal, xdelta, ydelta, xadjust, yadjust;

	for (k = 0; k < pathindex; k++) {
		if (path[k].code != 'C') continue;
		xtotal = path[k].end.x - path[k].start.x;
		ytotal = path[k].end.y - path[k].start.y;
		if (xtotal == 0.0 || ytotal == 0.0) continue;/* not v to h or h to v */
/*		vhcurveto case */	/* v to h */
		if ((path[k].cst.x == path[k].start.x) &&
			(path[k].cen.y == path[k].end.y)) {
			if (xtotal * ytotal < 0) outward = 1;
			else outward = 0;
			xdelta = path[k].cen.x - path[k].end.x;
			ydelta = path[k].cst.y - path[k].start.y;
			if (ignorelarge) {
				if (fabs(xdelta) > fabs(xtotal) ||
					fabs(ydelta) > fabs(ytotal)) continue;
			}
			yadjust = eps * ydelta / fabs(ytotal) * adjustby;
			xadjust = eps * xdelta / fabs(xtotal) * adjustby;
			if (! outward) {
				xadjust = - xadjust;
				yadjust = - yadjust;
			}
			if (fabs (ydelta + yadjust) > fabs (ytotal) ||
				fabs (xdelta + xadjust) > fabs (xtotal)) continue;
			if (fabs (yadjust) > fabs (ydelta) ||
				fabs (xadjust) > fabs (xdelta)) continue;
/*			if ((ydelta + yadjust) * ydelta < 0 ||
				(xdelta + xadjust) * xdelta < 0) continue; */
			path[k].cst.y += yadjust;
			path[k].cen.x += xadjust;
		}
/*		hvcurveto case */	/* h to v */
		if ((path[k].cst.y == path[k].start.y) &&
			(path[k].cen.x == path[k].end.x)) {
			if (xtotal * ytotal > 0) outward = 1;
			else outward = 0;
			xdelta = path[k].cst.x - path[k].start.x;
			ydelta = path[k].cen.y - path[k].end.y;
			if (ignorelarge) {
				if (fabs(xdelta) > fabs(xtotal) ||
					fabs(ydelta) > fabs(ytotal)) continue;
			}
			yadjust = eps * ydelta / fabs(ytotal) * adjustby;
			xadjust = eps * xdelta / fabs(xtotal) * adjustby;
			if (! outward) { 
				xadjust = - xadjust;
				yadjust = - yadjust;
			}
			if (fabs (ydelta + yadjust) > fabs (ytotal) ||
				fabs (xdelta + xadjust) > fabs (xtotal)) continue;
			if (fabs (yadjust) > fabs (ydelta) ||
				fabs (xadjust) > fabs (xdelta)) continue;
/*			if ((ydelta + yadjust) * ydelta < 0 ||
				(xdelta + xadjust) * xdelta < 0) continue; */
			path[k].cst.x += xadjust;
			path[k].cen.y += yadjust;
		}
/*	we dont' do anything with general curveto's right now */
	}
/*	if (flag) fprintf(stderr, "\nCan't adjust control in char %d ", chr); */
}

void newcontrols () {
	int k, flag=0;
	int badalphabeta=0;
/*	int outward; */
/*	double xtotal, ytotal, xdelta, ydelta, xadjust, yadjust; */
	VECTOR intersection;
	VECTOR new_cst, new_cen;
	VECTOR mod_cst, mod_cen;

	for (k = 0; k < pathindex; k++) {
/*		process only curveto's */
		if (path[k].code != 'C') continue;
		if (traceflag) printf("\n%d %d alpha start %lg alpha end %lg",
			chr, k, path[k].ast, path[k].aen);
		if (path[k].ast == 0.0 || path[k].aen == 0.0) {
			printf("\nZero alpha (%lg) or beta (%lg) in `%s' (%d) k %d ",
				   path[k].ast,  path[k].aen, charname, chr, k);
			if (averageflag == 0) continue; 
			badalphabeta=1;
		}
		else badalphabeta=0;
/*		intersection = intersect (path[k].start, path[k].cst,
			path[k].cen, path[k].end); */
		if (vector_eq(path[k].start, path[k].cst)) {
			intersection = path[k].cen;			/* 1995/June/18 */
		}
		if (vector_eq(path[k].cen, path[k].end)) {
			intersection = path[k].cst;			/* 1995/June/18 */
		}
		else intersect_lines (path[k].start, path[k].cst,
			path[k].cen, path[k].end, &intersection);
		if (vector_zero(intersection)) {
			if (averageflag == 0)	continue;
			if (badalphabeta != 0) continue;		/* safety valve ??? */
/* following may be good idea for short, nearly straight segments */
/* following may a bad idea for longer highly curved segments */
			printf("\nUsing average of knots OUT `%s' (%d) k %d ",
				   charname, chr, k);
/*			vector_aver(path[k].cst, path[k].cen, &intersection); */
			vector_aver(path[k].start, path[k].end, &intersection);
		}
/*		new_cst = vector_sum (path[k].start,
			vector_scale (vector_dif(intersection, path[k].start),
				path[k].ast)); */
/*		if (proper[k] == 0) */						/* 95/June/10 */
		sum_scale_dif (path[k].start, intersection, path[k].start,
					   path[k].ast,  &new_cst);
/*		new_cen = vector_sum(path[k].end,
			vector_scale (vector_dif(intersection, path[k].end),
				path[k].aen)); */
/*		if (proper[k+1] == 0) */						/* 95/June/10 */
		sum_scale_dif (path[k].end, intersection, path[k].end,
					   path[k].aen, &new_cen);
		if (traceflag)
			printf("\n%d %d Adjust %lg %lg -> %lg %lg and %lg %lg -> %lg %lg",
			chr, k, path[k].cst.x, path[k].cst.y, new_cst.x, new_cst.y,
				path[k].cen.x, path[k].cen.y, new_cen.x, new_cen.y);
		if (adjustby != 1.0) {
/*			path[k].cst = vector_sum(path[k].cst,
				vector_scale (vector_dif(new_cst, path[k].cst), adjust)); */
			sum_scale_dif (path[k].cst, new_cst, path[k].cst,
						   adjustby, &mod_cst);
			path[k].cst = mod_cst;
/*			path[k].cen = vector_sum(path[k].cen,
				vector_scale (vector_dif(new_cen, path[k].cen), adjust)); */
			sum_scale_dif (path[k].cen, new_cen, path[k].cen,
						   adjustby, &mod_cen);
			path[k].cen = mod_cen;
		}
		else {
			path[k].cst = new_cst;
			path[k].cen = new_cen;
		}
	}
/*	if (flag) fprintf(stderr, "\nCan't adjust control in char %d ", chr); */
}

void slidecontrols (void) {						/* 95/June/10 */
	int k;
	VECTOR tangent;	/* a */
	VECTOR offset;	/* v */
	double len;
	double vacost;	/* (a . v) / (a . a) */

/*	adjust knot for movement in end point */
	for (k = 0; k < pathindex; k++) {
		if (path[k].code != 'C') continue;		/* only for curveto */
		if (proper[k] == 0) {
		vector_dif(path[k].start, path[k].cst, &tangent);
		vector_dif(path[k].start, saved_start[k], &offset);
		if (! vector_zero(tangent))	{	/* need tangent to do this */
			vacost = dot_product (offset, tangent) /
				   dot_product (tangent, tangent);
			if (vacost != 0.0) {
				vector_scale (tangent, vacost, &tangent);
				len = mag(tangent);
				if (len > eps) {	/* 1995/June/18 */
					vector_scale (tangent, eps / len, &tangent);
					len = eps;
				}
				if (len < mag_dif(path[k].cst, path[k].start))
					vector_dif (path[k].cst, tangent, &path[k].cst);
			}
		}
		}
		if (proper[k+1] == 0) {
		vector_dif(path[k].end, path[k].cen, &tangent);
		vector_dif(path[k].end, saved_end[k], &offset);
		if (! vector_zero(tangent))	{	/* need tangent to do this */
			vacost = dot_product (offset, tangent) /
				   dot_product (tangent, tangent);
			if (vacost != 0.0) {
				vector_scale (tangent, vacost, &tangent);
				len = mag(tangent);
				if (len > eps) {	/* 1995/June/18 */
					vector_scale (tangent, eps / len, &tangent);
					len = eps;
				}
				if (len < mag_dif(path[k].cen, path[k].end))
					vector_dif (path[k].cen, tangent, &path[k].cen);
			}
		}		
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int writeoutline (FILE *output) {
	int nocurrent=1, hintdone=0;
	int k;
	
/*	if (pathindex < 5) {
		printf("\nTRIANGLE ");
		for (k = 0; k < pathindex; k++)	showsegment(path[k]);
	} */
/*	pathindex-1 instead of pathindex because of wraparound */
	for (k = 0; k < pathindex-1; k++) {
		if (nocurrent) {
			fprintf(output, "%lg %lg m\n", path[k].start.x, path[k].start.y);
			nocurrent = 0;
			if (path[k].hint != NOHINT) {
				fprintf(output, "%d s %%\n", path[k].hint);
				hintdone=1;
			}
		}
/*	don't show closing lineto if requested */
		if (path[k].code == 'L') {
			if (k < pathindex-2 || writeclosing)
				fprintf(output, "%lg %lg l\n", path[k].end.x, path[k].end.y);
			else if (path[k].end.x != path[0].start.x ||
					path[k].end.y != path[0].start.y)
				fprintf(output, "Ends do not match beginning\n");
		}
		else if (path[k].code == 'C') {
			fprintf(output, "%lg %lg %lg %lg %lg %lg c\n",
				path[k].cst.x, path[k].cst.y,
					path[k].cen.x, path[k].cen.y,
						path[k].end.x, path[k].end.y);
		}
/*		else if (path[k].code == 'H') {
			fprintf(output, "%d s %%\n", path[k].hint);
		} */
		if (path[k].hint != NOHINT) {
			if (! hintdone)	fprintf(output, "%d s %%\n", path[k].hint);
		}
	}
	fprintf(output, "h\n");
	if (ferror(output) != 0) {
		fprintf(stderr, "Error in output file ");
		perror(""); return -1;
	}
	else return 0;
}

int processoutline (FILE *output, FILE *input) {
	double psize, fscale;
	double xll, yll, xur, yur;
	int width, flag;
	int c=0;			/* init to quiten compiler ... */

	if (verboseflag) printf("Entering processoutline\n");

	if (showeps)
		fprintf(output, "%% Font %s with %lg swelling\n", fontname, eps);

	while (fgets (line, MAXLINE, input) != NULL) {
		fputs (line, output);
		if (verboseflag) fputs (line, stdout);
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf (line, "%lg %lg", &psize, &fscale) == 2) break;
		fprintf(stderr, "Don't understand %s", line);
	}
	while (fgets (line, MAXLINE, input) != NULL) {
		fputs (line, output);
		if (verboseflag) fputs (line, stdout);
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf (line, "%lg %lg %lg %lg", &xll, &yll, &xur, &yur) == 4)
			break;
		fprintf(stderr, "Don't understand %s", line);
	}
	if (verboseflag) printf("Entering main outline process loop\n");
	for (;;) {
		while (fgets (line, MAXLINE, input) != NULL) {
			fputs (line, output);
			if (traceflag) fputs (line, stdout); 
			if (*line == '%' || *line == ';' || *line == '\n') continue;
			if (*line == ']') break;
			fprintf(stderr, "Looking for ] not %s", line);
		}
		while (fgets (line, MAXLINE, input) != NULL) {	
			fputs (line, output);
			if (*line == '%' || *line == ';' || *line == '\n') continue;
			*charname = '\0';
			if (sscanf (line, "%d %d %% %s", &chr, &width, charname) > 1)
				break; 
			fprintf(stderr, "Looking for chr and width not %s", line);
		}
		for (;;) {
			flag = readoutline (input);
			if (pathindex == 0) break;
			if (checkbad) checkoutline();
			if (trimlost) checklost();		/* 95/May/30 */
/*			if (beakflag) checkbeak(); */	/* 95/June/3 */
			computealphas();
			savecorners();					/* 95/June/10 */
			if (swelling) doswelling();
			wrapends();
			if (adjustcontrol) docontrols();		/* NO */
			else if (tunecontrol) newcontrols();	/* NOW DEFAULT */
			wrapends();
			if (tweakcontrol) slidecontrols();		/* 95/June/10 */
			wrapends();
			writeoutline (output);
			if (flag < 0) return 0;
			c = getc(input);
			ungetc(c, input);
			if (c == EOF) return 0;
			if (c == ']') break;		/* done with this character */
		}
		if (traceflag)
			printf("Done with char %d flag %d c %d\n", chr, flag, c);
		if (dotsflag) putc('.', stdout);
		if (flag < 0) return 0;			/* ??? */
		if (c == EOF) return 0;			/* ??? */
	}
/*	return 0; */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int adjusthintline (char *buffer, char *line) {
	int chr, subr;
	int start, end, newstart, newend, oldend, new, n;
	char *s, *t;
	int flag=0;
	int cxll, cyll, cxur, cyur;

	s = line; t = buffer;
/*	if (dotsflag && *s == 'C') putc('.', stdout); */
	new = 1; subr = 0;
	if (*s == 'S') {
		sscanf(s, "S %d", &subr);
		while (*s != ';' && *s != '\0') *t++ = *s++;
		if (*s != '\0') *t++ = *s++;
		while (*s == ' ') *t++ = *s++;
		new = 0;
	}
	if (*s == 'C') {
		sscanf(s, "C %d", &chr);
		if (traceflag) printf("\nchar %d subr %d ", chr, subr);
		else if (dotsflag && new) putc('.', stdout); 
		while (*s != ';' && *s != '\0') *t++ = *s++;
		if (*s != '\0') *t++ = *s++;
		while (*s == ' ') *t++ = *s++;
	}
	oldend = -4096;
	if (*s == 'H' || *s == 'E') {
		while (*s != ' ' && *s != '\0') *t++ = *s++;
		while (*s == ' ') *t++ = *s++;
		for(;;) {
			if (sscanf(s, "%d %d%n", &start, &end, &n) < 2) break;
			s += n;
			if (fontographer) {
				cxll = xll[chr]; cyll = yll[chr];
				cxur = xur[chr]; cyur = yur[chr];
			}
			if (fontographer) {
				if (start < cyll)
					printf("V %d < %d in char %d\n", start, cyll, chr);
				newstart = (int) (cyll + (long)
				  (start - cyll) * (cyur - cyll) / (cyur - cyll + eps + eps));
				if (end > cyur)
					printf("V %d > %d in char %d\n", end, cyur, chr);
				newstart = (int) (cyll + (long)
				  (start - cyll) * (cyur - cyll) / (cyur - cyll + eps + eps));
				newend = (int) (cyur - (long)
				  (cyur - end) * (cyur - cyll) / (cyur - cyll + eps + eps));
			}
			else {
				newstart = (int) (start - eps);
				newend = (int) (end + eps);
			}
			if (newend > newstart && newstart > oldend) {	/* avoid overlap */
				sprintf(t, "%d %d ", newstart, newend);
				oldend = newend;
				t += strlen(t);
			}
			else {
				flag++;
				printf("\nDropping horizontal stem %d %d in char %d subr %d ",
				start, end, chr, subr);
			}
		}
		while (*s != ';' && *s != '\0') *t++ = *s++;
		if (*s != '\0') *t++ = *s++;
		while (*s == ' ') *t++ = *s++;
	}
	oldend = -4096;
	if (*s == 'V' || *s == 'M') {
		while (*s != ' ' && *s != '\0') *t++ = *s++;
		while (*s == ' ') *t++ = *s++;
		for(;;) {
			if (sscanf(s, "%d %d%n", &start, &end, &n) < 2) break;
			s += n;
			if (fontographer) {
				if (start < cxll)
					printf("H %d < %d in char %d\n", start, cxll, chr);
				newstart = (int) (cxll + (long)
				  (start - cxll) * (cxur - cxll) / (cxur - cxll + eps + eps));
				if (end > cxur)
					printf("H %d > %d in char %d\n", end, cxur, chr);
				newend = (int) (cxur - (long)
				  (cxur - end) * (cxur - cxll) / (cxur - cxll + eps + eps));
			}
			else {
				newstart = (int) (start - eps);
				newend = (int) (end + eps);
			}
			if (newend > newstart && newstart > oldend) {		/* avoid overlap */
				sprintf(t, "%d %d ", newstart, newend);
				oldend = newend;
				t += strlen(t);
			}
			else {
				flag++;
				printf("\nDropping vertical stem %d %d in char %d subr %d ",
				start, end, chr, subr);
			}
		}
	}
	while (*s != ';' && *s != '\0') *t++ = *s++;
/*	if (*s != '\0') *t++ = *s++; */
	while (*s != '\0') *t++ = *s++;
	*t = '\0';
	return flag;
}

void processfontlevel (FILE *output, FILE *input) {
	int c, n;
	char *s;
	int stem, newstem;
	int start, end, newstart, newend;
	int others, bottom;
	int horizontal, vertical;

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '/') {
/* check for /StdHW, /StdVW, /StemSnapH, /StemSnapV */
			if (strncmp(line, "/Std", 4) == 0 ||
				strncmp(line, "/StemSnap", 9) == 0) {
				if (strchr(line, 'H') != NULL) horizontal = 1;
				else horizontal = 0;
				if (strchr(line, 'V') != NULL) vertical = 1;
				else vertical = 0;
				if (showold) fprintf(output, "%% %s", line);
				if ((s = strchr(line, '[')) != NULL) {
					s++;
					while (sscanf(s, "%d%n", &stem, &n) == 1) {
						newstem = (int) (stem + eps + eps);
						if (fontographer) {
/* may want to adjust based on `average' char bounding box */
/* make use of vertical &  horizontal flags */
						}
						if (newstem > 0) stem = newstem;
						strcpy(buffer, s+n);
						sprintf(s, "%d", stem);
						s += strlen(s);
						strcpy(s, buffer);
						if (*s == ' ') s++;
						if (*s == ']') break;
					}						
				}
			}
			else if (strstr(line, "OtherBlues") != NULL ||
				     strstr(line, "BlueValues") != NULL) {
				if (showold) fprintf(output, "%% %s", line);
				if (strstr(line, "OtherBlues") != NULL) others = 1;
				else others = 0;
				bottom = 1;
				if ((s = strchr(line, '[')) != NULL) {
					s++;
					while (sscanf(s, "%d %d%n", &start, &end, &n) == 2) {
						if (bottom) {
							newstart = (int) (start - eps);
							newend = (int) (end - eps);
							if (fontographer) {
/* may want to adjust based on `average' char bounding box */
							}
						}
						else {
							newstart = (int) (start + eps);
							newend = (int) (end + eps);
							if (fontographer) {
/* may want to adjust based on `average' char bounding box */
							}
						}
						strcpy(buffer, s+n);
						sprintf(s, "%d %d", newstart, newend);
						s += strlen(s);
						strcpy(s, buffer);
						if (*s == ' ') s++;
						if (! others) bottom = 0;
					}
				}
			}
		}
		if (*line == '%' || *line == ';' || *line == '\n') {
		}
		else fputs(line, output); 
		c = getc(input);
		ungetc(c, input);
		if (c == 'C') break;
		if (c == 'S') break;
	}
}

void processhinting (FILE *output, FILE *input) {
/*	int c; */
/*	int chr, subr; */
/*	int start, end, new, oldend, n; */
/*	char *s, *t; */
	
	if (traceflag) printf("Processing hint file\n");
	if (showeps)
		fprintf(output, "%% Font %s with %lg swelling\n", fontname, eps);
	processfontlevel(output, input);
	if (traceflag) printf("Found start of character level hints\n");
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		strcpy(oldline, line);			/* remember original version */
		if (adjusthintline (buffer, line) != 0 && showold)
			fprintf(output, "%% %s", oldline);
		fputs(buffer, output);
	}
	if (dotsflag) putc('\n', stdout);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void intersect_test (void) {
	VECTOR a, b, c, d, corner;
	double ax, ay, bx, by, cx, cy, dx, dy;
	for (;;) {
		fputs("Enter coordinates of end points of two lines\n", stdout);
		fgets(line, MAXLINE, stdin);
		if (sscanf (line, "%lg %lg %lg %lg %lg %lg %lg %lg",
			&ax, &ay, &bx, &by, &cx, &cy, &dx, &dy) < 8) break;
		a.x = ax; a.y = ay;
		b.x = bx; b.y = by;
		c.x = cx; c.y = cy;
		d.x = dx; d.y = dy;
/*		corner = intersect (a, b, c, d); */
		intersect_lines (a, b, c, d, &corner);
		printf("Intersection (%lg %lg) --- ", corner.x, corner.y);
		if (real_intersect (a, b, c, d, &corner))
			printf("on both line segments (%lg %lg)\n",
				corner.x, corner.y);
		else printf("NOT on both line segments(%lg %lg)\n",
			corner.x, corner.y);
	}
} 

void usage (int argc, char *argv[]) {
	printf("Usage: %s [-v][-t][-l][-g][-e] [-s=<eps>] [-a=<adj>] <file1> ...\n",
		argv[0]);
	printf("\tv  verbose mode\n");
	printf("\tt  trace mode\n");
	printf("\tl  do not flush tiny path segments\n");
	printf("\tg  show problems when computing control point alphas\n");
	printf("\te  do not avoid short ends in corner finding\n");
	printf("\ts  swelling amount in Adobe units (default %lg)\n",
		eps);
	printf("\ta  adjustment of control points (default %ld%%)\n",
		(long) (adjustby * 100));
	printf("\to  roll over on corner offset (default %ld%%)\n",
		(long) (sigma * 100));
	printf("\tF  use char bounding boxes from specified AFM file\n");
	printf("\n");
	printf("\t   outline files are in `OUT' format (=> SWL)\n");
	printf("\t   hint    files are in `HNT' format (=> STM)\n");
	exit(1);
}

int commandline (int argc, char *argv[], int firstarg) {
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strncmp(argv[firstarg], "-s=", 3) == 0)
			eps = atoi(argv[firstarg]+3);
		if (strncmp(argv[firstarg], "-a=", 3) == 0) {
			adjustby = (double) atoi(argv[firstarg]+3) / 100.0;
			printf("Adjust %lg\n", adjustby);
		}
		if (strncmp(argv[firstarg], "-l=", 3) == 0) {
			lineareps = (double) atoi(argv[firstarg]+3) / 100.0;
			printf("Linear %lg\n", lineareps);
		}
		if (strncmp(argv[firstarg], "-o=", 3) == 0) {
			sigma = (double) atoi(argv[firstarg]+3) / 100.0;
			printf("Sigma %lg\n", sigma);
		}
		if (strncmp(argv[firstarg], "-F=", 3) == 0) {
			afmfilename = argv[firstarg]+3;
			fontographer=1;
		}
		if (strncmp(argv[firstarg], "-t", 2) == 0)
			traceflag = ~traceflag;
		if (strncmp(argv[firstarg], "-v", 2) == 0)
			verboseflag = ~verboseflag;
		if (strncmp(argv[firstarg], "-g", 2) == 0)
			 showalphabugs = ~showalphabugs;
		if (strncmp(argv[firstarg], "-b", 2) == 0)
			checkbad = ~checkbad;
		if (strncmp(argv[firstarg], "-l", 2) == 0)
			trimlost = ~trimlost;
		if (strncmp(argv[firstarg], "-d", 2) == 0)
			 distantflag = ~distantflag;
		if (strncmp(argv[firstarg], "-e", 2) == 0)
			 avoidshort = ~avoidshort;
		if (strncmp(argv[firstarg], "-s", 2) == 0)
			  shorttangent = ~shorttangent;
		if (strncmp(argv[firstarg], "-p", 2) == 0)
			 avoidbeak = ~avoidbeak;
		if (strncmp(argv[firstarg], "-o", 2) == 0)
			 otherwayflag = ~otherwayflag;
		if (strncmp(argv[firstarg], "-?", 2) == 0)
			detailflag = ~detailflag;
		firstarg++;
	}
	return firstarg;
}

void vector_test (void);

void vector_hack (void);

int readbboxes(char *afmfilename) {
	FILE *input;
	int k, chr, width;
	int count = 0;
	char charname[MAXCHARNAME];
	int cxll, cyll, cxur, cyur;
	
	for (k = 0; k < MAXCHRS; k++) xll[k] = yll[k] = xur[k] = yur[k] = 0;
	if ((input = fopen(afmfilename, "r")) == NULL) {
		perror(afmfilename);
		exit(1);
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "StartCharMetrics", 16) ==0) break;
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "EndCharMetrics", 14) ==0) break;
		if (sscanf(line, "C %d ; WX %d ; N %s ; B %d %d %d %d ;",
				   &chr, &width, charname, &cxll, &cyll, &cxur, &cyur) == 7)
		{
			if (chr >= 0 && chr < MAXCHRS) {
				xll[chr] = cxll;	yll[chr] = cyll;
				xur[chr] = cxur;	yur[chr] = cyur;
				count++;
			}
		}
	}
	fclose (input);
	printf("Noted bounding boxes for %d characters\n", count);
	return count;
}

void vector_test (void) {
	VECTOR a, b, result;
	a.x = 1.0; a.y = 1.0;
	b.x = 2.0; b.y = 0.0;
	vector_sum(a, b, &result);
	printf("SUM %lg %lg\n", result.x, result.y);
	vector_dif(a, b, &result);
	printf("DIF %lg %lg\n", result.x, result.y);
	vector_scale(a, 12.0, &result);
	printf("SCL %lg %lg\n", result.x, result.y);
	turn_90 (b, &result);
	printf("TRN %lg %lg\n", result.x, result.y);
	printf("MAG_DIF %lg\n", mag_dif(a, b));
	exit(1);
}

void vector_hack (void) {
	VECTOR u;
	VECTOR a, b, c;

	u.x = 12; u.y = 34;
	a.x = 1; a.y = 1;
	b.x = 2; b.y = 0;
	c = u;
	vector_sum(a, b, &c);
	printf("a (%lg %lg) b (%lg %lg) c (%lg %lg) u (%lg %lg) \n",
		   a.x, a.y, b.x, b.y, c.x, c.y, u.x, u.y);
}

int main (int argc, char *argv[]) {
/*	VECTOR a, b, c, new; */
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	int firstarg=1;
	int m;
	
/*	intersect_test();  */

/*	vector_test(); */

/*	vector_hack (); */

	if (argc <= firstarg) usage (argc, argv);

	firstarg = commandline (argc, argv, firstarg);

	if (argc <= firstarg) usage (argc, argv);

	if (verboseflag) printf("Swelling by %lg\n", eps);

	if (adjustby == 0.0) adjustcontrol=0;
	else if (verboseflag)
		printf("Adjusting control points by %ld%%\n", (long) (adjustby * 100));

	if (fontographer) readbboxes(afmfilename);

	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		fontname = _strdup(infilename);	/* for message output */
		extension (infilename, "out");
		strcpy(outfilename, removepath(infilename));
		if (strstr(infilename, ".hnt") != NULL) {
			forceexten(outfilename, "stm");
			hinting=1;
		}
		else {
			forceexten(outfilename, "swl");
			hinting=0;
		}
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename); exit(1);
		}
		if ((output = fopen(outfilename, "w")) == NULL) {
			perror(outfilename); exit(1);
		}
		if (hinting) processhinting (output, input);
		else processoutline (output, input);

		fclose(input);
		fclose(output);
		if (fontname != NULL) free(fontname);
	}
	return 0;
}

/* The new corner is a distance (eps / sin theta/2) from the old corner */
/* Sum of perpendicular offsets has length (2 * eps * sin theta/2) */
/* so need to divide this sum  by (2 sin^2 theta/2) = (1 + cos theta) */
/* and (cos theta) = dot(a, b) / (mag(a) * mag(b)) */
/* 1 / (1 + cos theta) = (mag(a) * mag(b) / (mag(a) * mag(b) + dot(a,b))) */
/* offset = eps * (a* / mag(a) + b* / mag(b)) / (1 + cos theta) */
/* where a* and b* are a and b rotated 90 degrees clockwise */

/* fatal error C1001: internal compiler error */
/* (compiler file `msc2.cpp' line 1011) */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* code imported from unionize.c */

double minfour(double a, double b, double c, double d)  {
	double low;
	low = a;
	if (b < low) low = b;
	if (c < low) low = c;
	if (d < low) low = d;
	return low;
}

double maxfour(double a, double b, double c, double d)  {
	double high;
	high = a;
	if (high < b) high = b;
	if (high < c) high = c;
	if (high < d) high = d;
	return high;
}

/* int round(double z) {
	if (z < 0.0) return (- (int) (-z + 0.5));
	else return ((int) (z + 0.5));
} */

/* double xintersect, yintersect; */

/* double alpha, beta; */

/* find intersection between lines from (asx, asy) to (axf, ayf)  and */
/* from (bxs, bys) to (bxf, byf)  - return zero if segments don't intersect */

int intersectlines(double axs, double ays, double axf, double ayf,
		double bxs, double bys, double bxf, double byf, int flag) {
	double det, numa, numb;
	double xinta, yinta, xintb, yintb, dx, dy;
	
/*  cross-product (af - as) x (bf - bs) */
	det = (axf - axs) * (byf - bys) - (ayf - ays) * (bxf - bxs); 
	if (det == 0.0)  return 0;  /* parallel lines don't intersect */

/*	cross-product (bs - as) x (bf - bs) */
	numa = (bxs - axs) * (byf - bys) - (bys - ays) * (bxf - bxs);
	alpha = numa / det;  
	if (flag == 0) {
		if (alpha <= 0.0 || alpha >= 1.0) return 0;
	}

/*	cross-product (bs - as) x (af - as) */
	numb = (bxs - axs) * (ayf - ays) - (bys - ays) * (axf - axs);
	beta = numb / det;
	if (flag == 0) {
		if (beta <= 0.0 || beta >= 1.0) return 0;
	}

	xinta = (1.0 - alpha) * axs + alpha * axf;
	xintb = (1.0 - beta) * bxs + beta * bxf;		
	yinta = (1.0 - alpha) * ays + alpha * ayf;
	yintb = (1.0 - beta) * bys + beta * byf;		

	dx = xintb - xinta; dy = yintb - yinta;
	
	if (dx < -0.1 || dx > 0.1 || dy < -0.1 || dy > 0.1)
		fprintf(stderr, "(%lg %lg) <> (%lg %lg)\n", 
			xinta, yinta, xintb, yintb);

/*	xintersect = (xinta + xintb) * 0.5; */
	intersection.x = (xinta + xintb) * 0.5;
/*	yintersect = (yinta + yintb) * 0.5;	 */
	intersection.y = (yinta + yintb) * 0.5;	

	if (alpha <= 0.0 || alpha >= 1.0) return 0;
	if (beta <= 0.0 || beta >= 1.0) return 0;	

	return 1;
}

double bezier(double t, double zs, double za, double zb, double zf) {
	double s;
	s = 1.0 - t;
	return (s * s * s * zs + 3.0 * t * s * s * za + 
		3.0 * t * t * s * zb + t * t * t * zf);
}

double bezierderiv(double t, double zs, double za, double zb, double zf) {
	double s;
	s = 1.0 - t;
/*	return 3.0 * (- s * s * zs + (1.0 - 3.0 * t) * s * za + 
		t * (2.0 - 3.0 * s) * zb + t * t * zf); */
	return 3.0 * (- s * s * zs + s * (s  - 2.0 * t) * za + 
		t * (2.0 * s - t) * zb + t * t * zf);
}

double epsbezier = 0.0001;
int maxloop = 1024;

/* intersect lineto and curveto - return zero if don't intersect */
/* heuristic - numerical - assumes curveto shallow */

int intersectlinecurvesub(double axs, double ays, double axf, double ayf,
		double bxs, double bys, double bxa, double bya, 
			double bxb, double byb, double bxf, double byf) {
	double xt, yt, xl, yl, dx, dy;
	double dlx, dly, dcx, dcy;
	double det, numa, numb;
	int n=0;

/*	first intersect chord with line - get approximate beta */
	(void) intersectlines(axs, ays, axf, ayf, bxs, bys, bxf, byf, 1);
	if (alpha < -0.5 || alpha > 1.5) return 0;
	if (beta < -0.5 || beta > 1.5) return 0;

/* then iteratively try to improve it */
	for (;;) {
		if (alpha < -0.5 || alpha > 1.5) return 0;
		if (beta < -0.5 || beta > 1.5) return 0;	
/*	compute point on curve */
		xt = bezier(beta, bxs, bxa, bxb, bxf);
		yt = bezier(beta, bys, bya, byb, byf);
/*  compute point on line */
		xl = (1.0 - alpha) * axs + alpha * axf;
		yl = (1.0 - alpha) * ays + alpha * ayf;		
/*	compute error */
		dx = xl - xt; dy = yl - yt;
		if (dx > - epsbezier && dx < epsbezier && dy > -epsbezier && dy < epsbezier) { /* converge */
			if (alpha < 0.0 || alpha > 1.0) return 0;
			if (beta < 0.0 || beta > 1.0) return 0;		
/*			xintersect = xl; yintersect = yl; */
			intersection.x = xl;
			intersection.y = yl;
			return 1;	/* yes, they intersect right here ! */
		}
		if (n++ > maxloop) return 0;
/*	compute tangents along lineto and curveto */
		dlx = axf - axs; dly = ayf - ays;
		dcx = bezierderiv(beta, bxs, bxa, bxb, bxf);
		dcy = bezierderiv(beta, bys, bya, byb, byf);
		det = dlx * dcy - dly * dcx;
		if (det == 0.0) return 0;
		numa = dx * dcy - dy * dcx;
		numb = dx * dly - dy * dlx;
		alpha -= numa / det;
		beta -= numb /det;
	}
}

/* -1 => no intersection 0 => don't know +1 => intersection */

int intersectlinecurve(double axs, double ays, double axf, double ayf,
		double bxs, double bys, double bxa, double bya, 
			double bxb, double byb, double bxf, double byf) {
	double xminc, yminc, xmaxc, ymaxc;
	double xminl, yminl, xmaxl, ymaxl;	
	double bxsa, bysa, bxab, byab, bxbf, bybf;
	double bxsab, bysab, bxabf, byabf, bxsabf, bysabf;

/*	first see whether enclosing rectangles intersect at all */
	xminc = minfour(bxs, bxa, bxb, bxf);
	yminc = minfour(bys, bya, byb, byf);
	xmaxc = maxfour(bxs, bxa, bxb, bxf);
	ymaxc = maxfour(bys, bya, byb, byf);
	xminl = axs; if (axf < xminl) xminl = axf;
	xmaxl = axs; if (axf > xmaxl) xmaxl = axf;	
	yminl = ays; if (ayf < yminl) yminl = ayf;
	ymaxl = ays; if (ayf > ymaxl) ymaxl = ayf;		
	if (xminl > xmaxc || xminc > xmaxl ||
		yminl > ymaxc || yminc > ymaxl) {
		printf (" BBox non-overlap (L & C) ");
		return -1;
	}
	if (intersectlinecurvesub(axs, ays, axf, ayf, 
		bxs, bys, bxa, bya, bxb, byb, bxf, byf) != 0) return 1;
	else {
/*	attempt to split curveto into two parts */
		bxsa = (bxs + bxa) * 0.5; bysa = (bys + bya) * 0.5;
		bxab = (bxa + bxb) * 0.5; byab = (bya + byb) * 0.5;	
		bxbf = (bxb + bxf) * 0.5; bybf = (byb + byf) * 0.5;	
		bxsab = (bxsa + bxab) * 0.5;  bysab = (bysa + byab) * 0.5;
		bxabf = (bxab + bxbf) * 0.5;  byabf = (byab + bybf) * 0.5;
		bxsabf = (bxsab + bxabf) * 0.5; bysabf = (bysab + byabf) * 0.5;
/*		return 0; */

/*	now try each of the two parts */
		if (intersectlinecurvesub(axs, ays, axf, ayf, 
			bxs, bys, bxsa, bysa, bxsab, bysab, bxsabf, bysabf) != 0) {
			beta = beta * 0.5;	/* ??? */
			return 1;
		}
		if (intersectlinecurvesub(axs, ays, axf, ayf, 
			bxsabf, bysabf, bxabf, byabf, bxbf, bybf, bxf, byf) != 0) {
			beta = 1.0 - 0.5 * (1.0 - beta);
			return 1;
		}
	}
/*	try splitting even finer ? */
	return 0;
}


int intersectcurvessub(double axs, double ays, double axa, double aya,
		double axb, double ayb, double axf, double ayf,
			double bxs, double bys, double bxa, double bya, 
				double bxb, double byb, double bxf, double byf) {
	double axt, ayt, bxt, byt, dx, dy;
	double dax, day, dbx, dby;
	double det, numa, numb;
	int n=0;
	double temp;

/*	first intersect the two chords - get approximate alpha and beta */
	(void) intersectlines(axs, ays, axf, ayf, bxs, bys, bxf, byf, 1);
	if (alpha < -0.5 || alpha > 1.5 ||
		beta < -0.5 || beta > 1.5) {
/*		return 0;		 */
/*  maybe intersect one line with one curveto using the above ? */
/*		if (trycurves == 0) return 0; */
		if (intersectlinecurve (axs, ays, axf, ayf,
			bxs, bys, bxa, bya, bxb, byb, bxf, byf) < 1) {
			if (intersectlinecurve (bxs, bys, bxf, byf,
				axs, ays, axa, aya, axa, aya, axf, ayf) < 1)
					return 0;
			temp = alpha; alpha = beta; beta = temp;
		}
	}

/*	then iteratively try to improve it */
	for (;;) {
		if (alpha < -0.5 || alpha > 1.5) return 0;
		if (beta < -0.5 || beta > 1.5) return 0;
/*	compute point on curve */
		axt = bezier(alpha, axs, axa, axb, axf);
		ayt = bezier(alpha, ays, aya, ayb, ayf);
/*  compute point on other curve */
		bxt = bezier(beta, bxs, bxa, bxb, bxf);
		byt = bezier(beta, bys, bya, byb, byf);
/*	compute error */
		dx = axt - bxt; dy = ayt - byt;
		if (dx > - epsbezier && dx < epsbezier && dy > -epsbezier && dy < epsbezier) {
/*			xintersect = (axt + bxt) * 0.5;  */
			intersection.x = (axt + bxt) * 0.5; 
/*			yintersect = (ayt + byt) * 0.5; */
			intersection.y = (ayt + byt) * 0.5;
			if (alpha < 0.0 || alpha > 1.0) return 0;
			if (beta < 0.0 || beta > 1.0) return 0;		
			return 1;	/* yes, they intersect right here ! */
		}
		if (n++ > maxloop) return 0;	/* give up ... */
/*	compute tangents along lineto and curveto */
		dax = bezierderiv(alpha, axs, axa, axb, axf);
		day = bezierderiv(alpha, ays, aya, ayb, ayf);
		dbx = bezierderiv(beta, bxs, bxa, bxb, bxf);
		dby = bezierderiv(beta, bys, bya, byb, byf);
		det = dax * dby - day * dbx;
		if (det == 0.0) return 0;		/* tangents parallel */
		numa = dx * dby - dy * dbx;
		numb = dx * day - dy * dax;
		alpha -= numa / det;
		beta -= numb /det;
	}
}

/* answer in global var intersection */
/* -1 => no intersection 0 => don't know +1 => intersection */

int intersectcurves(double axs, double ays, double axa, double aya,
		double axb, double ayb, double axf, double ayf,
			double bxs, double bys, double bxa, double bya, 
				double bxb, double byb, double bxf, double byf) {
	double xmina, ymina, xmaxa, ymaxa;
	double xminb, yminb, xmaxb, ymaxb;	

	xmina = minfour(axs, axa, axb, axf);
	ymina = minfour(ays, aya, ayb, ayf);
	xmaxa = maxfour(axs, axa, axb, axf);
	ymaxa = maxfour(ays, aya, ayb, ayf);
	xminb = minfour(bxs, bxa, bxb, bxf);
	yminb = minfour(bys, bya, byb, byf);
	xmaxb = maxfour(bxs, bxa, bxb, bxf);
	ymaxb = maxfour(bys, bya, byb, byf);		

	if (xmina > xmaxb || xminb > xmaxa ||
		ymina > ymaxb || yminb > ymaxa) {
		printf (" BBox non-overlap (C & C) ");
		return -1;
	}
	if (intersectcurvessub(axs, ays, axa, aya, axb, ayb, axf, ayf, 
		bxs, bys, bxa, bya, bxb, byb, bxf, byf) != 0) return 1;
	else return 0;
}

/* answer in global var intersection */
/* -1 => no intersection 0 => don't know +1 => intersection */

int intersect_curves (STEP a, STEP b) {		/* 95/June/8 */
	int flag = 0;
	double temp;

	if (a.code == 'C' && b.code == 'C') {
		flag = intersectcurves (a.start.x, a.start.y, a.cst.x, a.cst.y,
					a.cen.x, a.cen.y, a.end.x, a.end.y,
					b.start.x, b.start.y, b.cst.x, b.cst.y,
					b.cen.x, b.cen.y, b.end.x, b.end.y);
		if (verboseflag && flag <= 0) printf(" C & C ");
	}
	else if (a.code == 'L' && b.code == 'L') {
		flag = intersectlines (a.start.x, a.start.y, a.end.x, a.end.y,
						b.start.x, b.start.y, b.end.x, b.end.y, 0);
		if (verboseflag && flag <= 0) printf(" L & L ");
	}
	else if (a.code == 'L' && b.code == 'C') {
		flag = intersectlinecurve (a.start.x, a.start.y, a.end.x, a.end.y,
						b.start.x, b.start.y, b.cst.x, b.cst.y,
						b.cen.x, b.cen.y, b.end.x, b.end.y);
		if (verboseflag && flag <= 0) printf(" L & C "); 
	}
	else if (a.code == 'C' && b.code == 'L') {
		flag = intersectlinecurve (b.start.x, b.start.y, b.end.x, b.end.y,
							a.start.x, a.start.y, a.cst.x, a.cst.y,
							a.cen.x, a.cen.y, a.end.x, a.end.y);
		temp = alpha; alpha = beta; beta = temp;
		if (verboseflag && flag <= 0) printf(" C & L "); 
	}
	return flag;
}

/* reverse == 0 => split at alpha and keep initial part */
/* reverse != 0 => split at alpha and keep rest part */

void split_curveto (STEP step, double alp, int reverse, STEP *res) {
	double f0, f1, f2, f3;
	double a, b, c, d, mal;
	double an, bn, cn, dn;
	STEP new;
	
	new = step;					/* copy across constant parts */
/*  now for the hard part */ /* the curveto is split at t = alp */
	if (reverse == 0) {
		if (vector_eq(step.end, intersection)) printf(" BOGUS SPLIT ");
	}
	else {
		if (vector_eq(step.start, intersection)) printf(" BOGUS SPLIT ");
	}

/*	f0 = xs[k]; f1 = xa[k]; f2 = xb[k]; f3 = xf[k]; */
	f0 = step.start.x;
	f1 = step.cst.x;
	f2 = step.cen.x;
	f3 = step.end.x;
	
	a = (f3 - f0) - 3.0 * (f2 - f1);
	b = 3.0 * (f2 - 2.0 * f1 + f0);
	c = 3.0 * (f1 - f0);	
	d = f0;
	
	if (reverse == 0) {
	an = a * alp * alp * alp;
	bn = b * alp * alp;
	cn = c * alp;
	dn = d;

/*	if (new.start.x != dn) */  /*	xs[k] should equal round(dn) */
	if (fabs(new.start.x - dn) > epsilon)
		printf("\nnew.start.x (%lg) != dn (%lg) A", new.start.x, dn);
	new.start.x = dn;
/*	xa[k] = round(dn + cn / 3.0); */
	new.cst.x = (dn + cn / 3.0);
/*	xb[k] = round(dn + (2.0 * cn + bn) / 3.0); */
	new.cen.x = (dn + (2.0 * cn + bn) / 3.0);
/*	xf[k] = round(x); */
	new.end.x = intersection.x;
/*	if (new.end.x != (dn+cn+bn+an)) */ /* xf[k] = round(dn + cn + bn + an); */
	if (fabs(new.end.x - (dn+cn+bn+an)) > epsilon) 
		printf("\nnew.end.x (%lg) != (dn+cn+bn+an) (%lg) A", 
			new.end.x, (dn+cn+bn+an));
	}
	else {	/* reverse != 0 */

	mal = 1.0 - alp;
	an =  a * mal * mal * mal;
	bn =  (3.0 * a * alp + b) * mal * mal;
	cn =  (alp * (3.0 * a * alp + 2.0 * b) + c) * mal;
	dn =  alp * ( alp * (alp * a + b) + c) + d; 
	
/*	xs[k+1] = round(x); */
	new.start.x = intersection.x;
/*	if (new.start.x != dn) */  /*	xs[k+1] should equal round(dn) */
	if (fabs(new.start.x - dn) > epsilon)
		printf("\nnew.start.x (%lg) != dn (%lg) B ", new.start.x, dn);
/*	xa[k+1] = round(dn + cn / 3.0); */
	new.cst.x = (dn + cn / 3.0);
/*	xb[k+1] = round(dn + (2.0 * cn + bn) / 3.0); */
	new.cen.x = (dn + (2.0 * cn + bn) / 3.0);
/*	if (new.end.x != (dn+cn+bn+an)) */  /* xf[k+1] = round(dn+cn+bn+an); */
	if (fabs(new.end.x - (dn+cn+bn+an)) > epsilon)
		printf("\nnew.end.x (%lg) != (dn+cn+bn+an) (%lg) B ",
			   new.end.x, (dn+cn+bn+an));
	}
/*	new.end.x = (dn+cn+bn+an);  */ /* ??? */

/*	f0 = ys[k]; f1 = ya[k]; f2 = yb[k]; f3 = yf[k]; */
	f0 = step.start.y;
	f1 = step.cst.y;
	f2 = step.cen.y;
	f3 = step.end.y;
	
	a = (f3 - f0) - 3.0 * (f2 - f1);
	b = 3.0 * (f2 - 2.0 * f1 + f0);
	c = 3.0 * (f1 - f0);	
	d = f0;
	
	if (reverse == 0) {
	an = a * alp * alp * alp;
	bn = b * alp * alp;
	cn = c * alp;
	dn = d;

/*	if (new.start.y != dn) */  /*	ys[k] should equal round(dn) */
	if (fabs(new.start.y - dn) > epsilon) 
		printf("\nnew.start.y (%lg) != dn (%lg) A ", new.start.y, dn);
	new.start.y = dn;
/*	ya[k] = round(dn + cn / 3.0); */
	new.cst.y = (dn + cn / 3.0);
/*	yb[k] = round(dn + (2.0 * cn + bn) / 3.0); */
	new.cen.y = (dn + (2.0 * cn + bn) / 3.0);
/*	yf[k] = round(y); */
	new.end.y = intersection.y;
/*	if (new.end.y != (dn+cn+bn+an)) */  /* yf[k] = round(dn + cn + bn + an); */
	if (fabs(new.end.y - (dn+cn+bn+an)) > epsilon)  
		printf("\nnew.end.y (%lg) != (dn+cn+bn+an) (%lg) A ", 
			new.end.y, (dn+cn+bn+an));

	}
	else { /* reverse != 0 */

	mal = 1.0 - alp;
	an =  a * mal * mal * mal;
	bn =  (3.0 * a * alp + b) * mal * mal;
	cn =  (alp * (3.0 * a * alp + 2.0 * b) + c) * mal;
	dn =  alp * ( alp * (alp * a + b) + c) + d; 
	
/*	ys[k+1] = round(y); */
	new.start.y = intersection.y;
/*	if (new.start.y != dn) */  /*	ys[k+1] should equal round(dn) */
	if (fabs(new.start.y - dn) > epsilon)  
		printf("\nnew.start.y (%lg) != dn (%lg) B ", new.start.y, dn);
/*	ya[k+1] = round(dn + cn / 3.0); */
	new.cst.y = (dn + cn / 3.0);
/*	yb[k+1] = round(dn + (2.0 * cn + bn) / 3.0); */
	new.cen.y = (dn + (2.0 * cn + bn) / 3.0);
/*	if (new.end.y != (dn+cn+bn+an)) */  /* yf[k+1] = round(dn+cn+bn+an); */
	if (fabs(new.end.y - (dn+cn+bn+an)) > epsilon)
		printf("\nnew.end.y (%lg) != (dn+cn+bn+an) (%lg) B ",
			   new.end.y, (dn+cn+bn+an));
	}
/*	new.end.y = (dn+cn+bn+an); */
	*res = new;
}

void split_lineto (STEP step, double alp, int reverse, STEP *res) {
	VECTOR offset;
	STEP new;
			
	new = step;
	vector_dif(step.end, step.start, &offset);
	vector_scale(offset, alp, &offset);

	if (reverse == 0) vector_sum(new.start, offset, &new.end);
/*	else vector_dif(new.end, offset, &new.start); */
	else vector_sum(new.start, offset, &new.start);		/* 95/June/11 */

	new.cst = new.end; new.cen = new.start;
	*res = new;
}

void split_curve (STEP step, double alp, int reverse, STEP *res) {
	if (step.code == 'L') split_lineto (step, alp, reverse, res);
	if (step.code == 'C') split_curveto (step, alp, reverse, res);
}


/*	We should adjust knots also when the end-point of a curveto is extended
	or reduced (that is, when it is not just shifted outward or inward.
	To do this we'd need to calculate the shift parallel to the tangent
	at the ends, which means we need the old corner positions OK, done. */

/*	The new method for intersecting two curveto's should really be used
	in general, not just for `beak' problems */

/*	The new method should also be used for intersecting lineto's with
	curveto's, not just for `beak' problems OK, working on it */	

/*	Should also use the new method when a segment is dropped for being short*/

/*	Stack overflow may occur when recursion in curveto intersection */
/*	particularly when a solution may not exist ... */

/*	May need to compile with extra stack allocation (3rd arg in compile.bat) */
/*	compile4 swell 6000 */

/* maybe do not use average for intersection if things are not near colinear */

/* maybe prepass end tangents less than eps are extended to equal eps */

/* maybe prepass zero tangents are replaced by eps long in tangent dir */ 

