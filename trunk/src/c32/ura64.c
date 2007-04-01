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
#include <string.h>

int verboseflag=0;
int traceflag=0;
int debugflag=0;

#define MAXPATTERN 1024

__int64 patterns[MAXPATTERN];

int patterninx;

int bits[65536];						/* lookup table for number of bits */

int countbits(__int64 res) {			/* count number of bits */
	int nbits=0;
	if (traceflag) printf("CountBits %X\n", (unsigned long) res);
	while (res != 0) {
		res = res & ~(res & (~res + 1));	/* knock out rightmost bit */
		nbits++;							/* count the bits */
	}
	return nbits;
}

int countbitsfast(__int64 res) {	/* uses table set up by countbits */
	unsigned int k1, k2, k3, k4;
	k1 = (unsigned int) (res & 0XFFFF);
	res = res >> 16;
	k2 = (unsigned int) (res & 0XFFFF);
	res = res >> 16;
	k3 = (unsigned int) (res & 0XFFFF);
	res = res >> 16;
	k4 = (unsigned int) (res & 0XFFFF);
	return bits[k4] + bits[k3] + bits[k2] + bits[k1];
}

__int64 lowest (__int64 pattern, int nlen) {	/* find lowest equiv pattern */
	__int64 low;
	__int64 carry = 1 << nlen;
	int k;
/*	int nbits1, nbits2; */

	if (pattern == 0) return 0;		/* should never happen */
/*	Pick the one with fewer bits: pattern and its complement */
/*	nbits1 = countbitsfast(pattern);
	low = ~pattern & ((1 << nlen) -1);
	nbits2 = countbitsfast(low);
	if (nbits2 < nbits1) pattern = low; */
/*	nbits = countbitsfast(pattern & (pattern << 1)); */
/*	if (nbits > nlen / 2) {
		if (traceflag) printf ("pattern %0X ", pattern);
		pattern = ~pattern & ((1 << nlen) - 1);
		if (traceflag) printf ("flipped to %0X \n", pattern);
	} */
/*	while ((pattern & 1) == 0) {
		pattern = pattern >> 1;
		if (pattern == 0) {
			printf ("Pattern 0!\n");
			return 0;
		}
	} */
/*	Then pick the one which represents the lowest number */
	low = pattern;
	for (k = 0; k < nlen; k++) {
		if (debugflag) printf("Shift %d %0X\n", k, (unsigned long) pattern);
		if (pattern < low) low = pattern;
		pattern = pattern << 1;			/* rotate left */
		if (pattern & carry) pattern = (pattern & ~carry) | 1;		
	}
	if (debugflag)
	printf("Pattern %0X Low %0X\n", (unsigned long) pattern, (unsigned long) low);
	return low;
}

int already (__int64 pattern) {	/* see if already in table */
	int k;
	if (traceflag) printf("Already %X?\n", (unsigned long) pattern);
	for (k = 0; k < patterninx; k++) {
		if (patterns[k] == pattern) return k;
	}
	return -1;
}

int checkit (__int64 pattern, int nlen) {
	__int64 master, res;
	int nshift, nbits, nbitsone, npattern;

	if (nlen > 32) {
		printf("Pattern too long (%d > 32)\n", nlen);
		return -1;
	}
	if (nlen < 2) {
		printf("Pattern too short (%d < 2)\n", nlen);
		return -1;
	}
	master = pattern | (pattern << nlen);	/* duplicate pattern */

	npattern = countbitsfast (pattern);
/*	not interestedin single bit on or single bit off patterns */
	if (npattern == 1 || npattern == nlen-1) return -1;
	
	pattern = pattern << 1;
	nshift = 1;
	res = master & pattern;				/* and them together */
	nbitsone = nbits = countbitsfast (res);
	if (traceflag)
		printf("%d\t%0X %0X %d\n", nshift,
			   (unsigned long) master, (unsigned long) pattern, nbits);
	pattern = pattern << 1;

	for (nshift = 2; nshift < nlen; nshift++) {
		res = master & pattern;				/* and them together */
		nbits = countbitsfast (res);
		if (traceflag)
			printf("%d\t%0X %0X %d\n", nshift,
				   (unsigned long) master, (unsigned long) pattern, nbits);
		if (nbits != nbitsone) return -1;	/* sidelobes not flat */
		pattern = pattern << 1;
	}
	return nbitsone;
}


/* generate the patterns we want to test */

void process(int nlen) {
	__int64 pattern, low, high;
	int nbits, nbitszero;
	patterninx=0;
	high = (1U << nlen)-1;
	if (verboseflag) printf("nlen %d: 0 - 0%X (%ld)\n", nlen,
							(unsigned long) high, (unsigned long) high);
	for (pattern = 1; pattern < high; pattern = pattern+2) {
/*		if (pattern & 1) continue; */
		nbits = countbitsfast(pattern);
		if (nbits > nlen/2) continue;	/* ignore will meet complement */
		if (traceflag) printf("Trying %0X (%ld)\n",
								(unsigned long) pattern, (unsigned long) pattern);
		else putc('.', stdout);
		nbitszero = countbitsfast (pattern);
		nbits = checkit (pattern, nlen);
		if (nbits > 0) {
			low = lowest (pattern, nlen);
			if (already(low) >= 0) {	/* already in table ? */
				printf("%0X (%0X) already in table\n",
					   (unsigned long) low, (unsigned long) pattern);
			}
			else {
				if (patterninx >= MAXPATTERN) {
					printf ("Too many patterns to store\n");
					exit(1);
				}
				else patterns[patterninx++] = low;
				printf("HIT: %d\t%04X %d / %d (nlen %d)\n",
				   patterninx, (unsigned long) pattern, nbits, nbitszero, nlen);
			}
		}
	}
	printf("Found %d distinct patterns\n", patterninx);
}

int checkold (__int64 pattern, int nlen) {
	__int64 master, res;
	int nshift, nbits;

	if (nlen > 32) {
		printf("Pattern too long (%d > 32)\n", nlen);
		return 0;
	}
	master = pattern | (pattern << nlen);	/* replicate pattern */
	for (nshift = 0; nshift < nlen; nshift++) {
		nbits = 0;
		res = master & pattern;				/* and them together */
		while (res != 0) {					/* count number of bits */
			res = res & ~(res & ((~res) + 1));	/* knock out rightmost */
			nbits++;						/* count the bits */
		}
		printf("%d\t%0X %0X %d\n", nshift,
			   (unsigned long) master, (unsigned long) pattern, nbits);
		pattern = pattern << 1;
	}
	return 0;
}

int main (int argc, char *argv[]) {
	__int64 pattern;
	int k, nlen;
	int firstarg=1;

	while (firstarg < argc && *(argv[firstarg]) == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag=1;
		if (strcmp(argv[firstarg], "-t") == 0) traceflag=1;
		if (strcmp(argv[firstarg], "-d") == 0) debugflag=1;
		firstarg++;
	}

	if (traceflag) printf("Setting up table\n");

	for (k = 0; k < 65536; k++)
		bits[k] = countbits(k);

	if (traceflag) printf("Finished setting up table\n");

	if (argc < firstarg+1) {
		for (k = 1; k < 32; k = k+2) {
			printf("nlen %d\n", k);
			process(k);
		}
		return 0;
	}

	if (argc < firstarg+2) {		/* one arg - use for nlen */
		if (sscanf(argv[firstarg], "%d", &nlen) < 1) {
			printf("Don't understand %s\n", argv[firstarg]);
		}
		if (traceflag) printf("nlen %d\n", nlen);
		process (nlen);
		return 0;
	}

	if (sscanf(argv[firstarg], "%X", &pattern) < 1) {
		printf("Don't understand %s\n", argv[firstarg]);
	}
	if (sscanf(argv[firstarg+1], "%d", &nlen) < 1) {
		printf("Don't understand %s\n", argv[firstarg+1]);
	}
	checkold(pattern, nlen);
	return 0;
}
