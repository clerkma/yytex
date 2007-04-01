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
#include <string.h>

#include <math.h>

int isprime(long n) {
	int k, ns;
	ns = (int) sqrt((double) n) + 1;
	if (n % 2 == 0) return 0;
	for (k = 3; k <= ns; k += 2) {
		if (n % k == 0) return 0;
	}
	return 1;
}

long findprime (long n) {
	if (n % 2 == 0) n++;
	while (isprime(n) == 0) {
		printf("%ld is NOT prime\n", n);
		n = n+2;
	}
	printf("%ld is prime\n", n);
	return n;
}

int main (int argc, char *argv[]) {
	long n;
	if (argc < 2) exit(1);
	if (sscanf(argv[1], "%ld", &n) < 1) exit(1);
	findprime(n);
	return 0;
}
