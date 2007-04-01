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

int nmax=128;

double pi=3.141592653;

double kexp=1.0;

int nabs=32;	    /* position of absorber */

double rabs=0.9;    /* radius of absorber (1 - depth) */

double sabs=0.25;    /* fraction absorbed */

double kabs=2.0;

void outinhex (FILE *output, int k) {
    int c, d;
    if (k < 0 || k > 255) fprintf(stderr, "ERROR: %d ", k);
    c = (k >> 4) & 15; d = k & 15;
    if (c < 10) putc(c + '0', output);
    else putc((c-10) + 'A', output);
    if (d < 10) putc(d + '0', output);
    else putc((d-10) + 'A', output);
}

int main (int argc, char *argv[]) {
    int i, j, grey;
    double xa, ya, xb, yb;
    double xabs, yabs;
    double theta, thetaa, thetab, r, ra, rb, f;

    theta = (double) nabs * 2.0 * pi / (double) nmax;
    xabs = rabs * cos (theta); yabs = rabs * sin (theta);

    for (i = 0; i < nmax; i++) {
	thetaa = (double) i * 2.0 * pi / (double) nmax;
	xa = cos(thetaa); ya = sin(thetaa);
	ra = sqrt ((xabs - xa) * (xabs - xa)  + (yabs - ya) * (yabs - ya));
	for (j = 0; j < nmax; j++) {
	    thetab = (double) j * 2.0 * pi / (double) nmax;
	    xb = cos(thetab); yb = sin(thetab);
	    r = sqrt ((xb - xa) * (xb - xa)  + (yb - ya) * (yb - ya));
	    rb = sqrt ((xabs - xb) * (xabs - xb)  + (yabs - yb) * (yabs - yb));
	    f = exp (- kexp * r);
	    f = f - sabs * (exp (- kabs * ra) + exp (- kabs * rb));
	    if (f < 0.0) f = 0.0;
	    grey = (int) (f * 255.999999);
	    outinhex(stdout, grey);
	}
	putc('\n', stdout);
    }
    return 0;
}
