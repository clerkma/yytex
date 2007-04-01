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

struct ratio {			/* structure for rational numbers */
	long numer;
	long denom;
};

/* Compute good rational approximation to floating point number. */
/* Generates successive approximants using continued fractions --- */
/* until either get exact answer, or numerator or denominator get too large */
/* (i.e. larger than nlimit or dlimit respectively) */

struct ratio rational(double x, long nlimit, long dlimit) {
	long p0, q0, p1, q1, p2, q2;	/* three successive rationals */
	double s, ds;					/* left over piece */
	struct ratio res;				/* place for result */
	
	if (x == 0.0) {		/* zero - deal with here to avoid trouble later */
		res.numer = 0; res.denom = 1;
		return res;		/* the answer is 0/1 */
	}
	if (x < 0.0) {		/* do negative as positive, and then negate numer */
		res = rational(-x, nlimit, dlimit);
		res.numer = - res.numer; 
		return res;
	}
/*	when we get here we can assume x is positive */
	s = x;
/*	set up first two rational approximations 0/1 and 1/0 */
/*	(just a convenient artifice to get the iteration going) */
	p0=0; q0=1;
	p1=1; q1=0;
	for(;;) {								/* iterate until done */
/*  compute next rational approximation using continued fraction formula */
		p2 = p0 + (long) s * p1;
		q2 = q0 + (long) s * q1;
/*	stop if numer or denom exceeds specified limit --- also stop if */
/* --- due to error in arithmetic they become negative (should not happen) */
/*  in each case back up to *previous* rational approximation */
		if (p2 > nlimit || q2 > dlimit || p2 < 0 || q2 <= 0) {
			p2 = p1; q2 = q1; break;
		}
/*	stop if we have the exact answer (not very likely) */
		if ((double) p2 / (double) q2 == x) break;
/*  compute `left over' part */
		ds = s - (double) ((long) s);
/*	we are done if s is integer - in any case we don't want to divide by zero */
		if (ds == 0.0) break;
/*	shift old values down by one p0/q0 <- p1/q1 <- p2/q2 */
		p0 = p1; q0 = q1; p1 = p2; q1 = q2;
		s = 1/ds;
/*	catch large s that would overflow when converted to long in next step */
/*	(these are arbitrary limits good enough for task at hand) */
/*  nlimit and dlimit should really be chosen to exit before this happens */
		if (s > 1000000000.0 || s < -1000000000.0) break;
	}
/*	at this point we assert that q2 > 0 and exit */
	res.numer = p2; res.denom = q2;
	return res;		/* the answer is p2/q2 */
}

int main (int argc, char *argv[]) {
	double x;
	struct ratio r;
	if (argc < 2) exit(1);
	printf("Scanning %s\n", argv[1]);
	if (sscanf(argv[1], "%lg", &x) < 1) exit(1);
	printf("Obtained %lg\n", x);
	r = rational(x, 32000, 4000);
	printf("%lg ===> %ld / %ld\n", x, r.numer, r. denom);
	return 0;
}
