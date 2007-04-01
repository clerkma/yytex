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
#include <malloc.h>

int nprimes=20;			/* number of primes to try */
int p = 263;				/* prime of form 4k+3 */
int plusminus=0;
int flipsign=1;
int verboseflag=0;

#ifdef IGNORED
b[i] such that the sum of b[i] is large, but the sum of b[i] * b[i+j]
   is a small constant independent of j, whenever j \neq 0.

   Euler squareness mod p: if p is prime, then  a^(p-1)/2 = 1 or -1
	If the formula yields 1, then a is a square, otherwise it is not.


Algorithm: for p prime, p = 4k + 3

							b[0] = 1;		/* or -1, or anything else */
for i = 1 tp p - 1
		begin
		b[i] = 1 if i is a square mod p,
		-1 otherwise
			   end
#endif

int isprime(int a) {
	int k;
	for (k = 2; k*k <= a; k++) if ((a % k) == 0) return 0;
	return 1;
}

int nextprime(int p) {
	for (;;) {
		p += 2;
		if (isprime(p)) return p;
	}
}

int nextniceprime(int p) {
	for (;;) {
		p = nextprime(p);
		if ((p+1) % 4 == 0) return p;
		if (verboseflag) printf("Bad %d (not 4 * %d + 3)\n", p, (p-3)/4);
	}
}

/* compute a^b mod p */

int expt (int a, int b, int p) {
	int r=1;
	int i;
	for (i=0; i <b; i++) {
		r = r * a;
		r = r % p;
	}
	return r;
}

/* return 1 if a is a square mode p, -1 otherwise */

int issquare (int a, int p) {
	if (expt(a, (p-1)/2, p) == 1) return 1;
	else return -1;
}

void fill (int *b, int p) {
	int i;
	b[0] = 1;
	for (i=1; i < p; i++)
		b[i] = issquare(i, p);
}

int convolution(int *b, int j, int p) {
	int s=0;
	int i;
	if (plusminus)
		for (i=0; i<p; i++) s += b[i] * b[((i+j) % p)];
	else if (flipsign)
		for (i=0; i<p; i++) s += (1-(b[i]+1)/2) * (1-(b[((i+j) % p)]+1)/2);
	else 
		for (i=0; i<p; i++) s += (b[i]+1)/2 * (b[((i+j) % p)]+1)/2;
	return s;
}

void showpattern (int *b, int p) {
	int i;
	if (plusminus)
		for (i = 0; i < p; i++)	printf("%3d", b[i]);
	else if (flipsign)
		for (i = 0; i < p; i++)	printf("%1d", 1-(b[i]+1)/2);
	else
		for (i = 0; i < p; i++)	printf("%1d", (b[i]+1)/2);
}

int checkconvolution (int *b, int p) {
	int i;
	int side=convolution(b, 1, p);
	for (i = 2; i < p; i++) {
		if (convolution(b, i, p) != side) {
			return -1;
		}
	}
	return side;
}

void showconvolution (int *b, int p) {
	int i;
	if (p < 400)
		for (i = 0; i < p; i++)	printf("%3d", convolution(b, i, p));
	else
		for (i = 0; i < p; i++)	printf("%4d", convolution(b, i, p));		
}

int main(int argc, char *argv[]) {
	int *b;
	int k, side;

	p = 1;
	for (k = 0; k < nprimes; k++) {
/*		p += 2; */
/*		p = nextprime(p); */
		p = nextniceprime(p);
		printf("Trying p = %d\n", p);
		if (isprime(p) == 0) printf("Bad %d not a prime\n", p); 
		if ((p+1) % 4 != 0) printf("Bad %d (not 4 * %d + 3)\n", p, (p-3)/4);
		b = (int *) malloc(p * sizeof(int));
		fill(b, p);
		showpattern(b, p);
		printf("\n");
		side = checkconvolution(b, p);
		if (side < 0) showconvolution(b, p);
		else printf("Side %d", side);
		printf("\n");
		free(b);
	}
	return 0;
}

