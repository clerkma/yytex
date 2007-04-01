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
#include <math.h>

int verboseflag=0;
int traceflag=0;
int debugflag=0;

typedef unsigned __int64 UINT64;

#define PI 3.141592653

/**********************************************************************/

void print64 (UINT64 res) {
	unsigned long k1, k2;
	k2 = (unsigned long) (res & 0XFFFFFFFF);
	k1 = (unsigned long) ((res >> 32) & 0XFFFFFFFF);
	if (k1 == 0) printf("%8X", k2);
	else printf("%8X%08X", k1, k2);
}

void print64dec (UINT64 res) {
	int c;
	if (res == 0) return;
	c = (int) (res % 10);
	print64dec(res / 10);
	putc('0'+c, stdout);
}

void printbits (UINT64 res, int nlen) {
	unsigned long k1, k2;
	unsigned long topbit;
	int n;
	k2 = (unsigned long) (res & 0XFFFFFFFF);
	k1 = (unsigned long) ((res >> 32) & 0XFFFFFFFF);
	if (nlen > 32) {
		topbit = 1 << (nlen-32-1);
		for (n = nlen; n > 32; n--) {
			if (topbit & k1) putc('1', stdout);
			else putc('0', stdout);
			k1 = k1 << 1;
		}
		nlen = 32;
	}
	topbit = 1 << (nlen-1);
	for (n = nlen; n > 0; n--) {
		if (topbit & k2) putc('1', stdout);
		else putc('0', stdout);
		k2 = k2 << 1;
	}
}

void print64CR (UINT64 res) {
	print64 (res);
	putc('\n', stdout);
}

/**********************************************************************/

/* The slow way, used to set up the lookup table */

int countbits(UINT64 res) {			/* count number of bits */
	int nbits=0;
	if (traceflag) {
		printf("CountBits ");
		print64CR(res);
	}
	while (res != 0) {
		res = res & ~(res & (~res + 1));	/* knock out rightmost */
		nbits++;							/* count the bits */
	}
	return nbits;
}

int reverseslow (int pattern) {	/* reverse pattern 16 bit */
	int rever=0;
	int k;

	for (k = 0; k < 16; k++) {
		if (pattern & 1) rever = (rever << 1) | 1;
		else rever = rever << 1;
		pattern = pattern >> 1;
	}
	return rever;
}

void showfreq (UINT64 pattern, int nlen, int sidelobe, int nbits) {
	int k, i;
	UINT64 res;
	double csum, ssum, theta, mag, mag2, phase;
	
	print64(pattern);
	printf(" %d bits %d sidelobe %d nbits\n", nlen, sidelobe, nbits);
	fflush(stdout);
/*	for (k = 0; k < nlen-1; k++) { */
	for (k = 0; k < nlen; k++) {
		res = pattern;
		csum = ssum = 0.0;
/*		for (i = 0; i < nlen-1; i++) { */
		for (i = 0; i < nlen; i++) {
			if (res & 1) {
				theta = (double) (k * i) / (double) nlen * 2.0 * PI;
				csum += cos(theta);
				ssum += sin(theta);
			}
			res = res >> 1;
		}
		mag2 = (csum*csum + ssum*ssum);
		mag = sqrt(mag2);
		phase = atan2(ssum, csum) * 180.0 / PI;
		printf("%3d\tc %lg\ts %lg\tp %lg\tm %lg\n",
			   k, csum, ssum, phase, mag2);
		fflush(stdout);
	}
}

int main (int argc, char *argv[]) {
	int firstarg=1;

	showfreq ((UINT64) 0xB, 7, 1, 3);
	showfreq ((UINT64) 0x97, 11, 2, 5);
	showfreq ((UINT64) 0x53, 13, 1, 4);
	showfreq ((UINT64) 0x8D, 13, 1, 4);
	showfreq ((UINT64) 0x537, 15, 3, 7);
	showfreq ((UINT64) 0x5793, 19, 4, 9);
	showfreq ((UINT64) 0x985, 21, 1, 5);
	showfreq ((UINT64) 0x299AF, 23, 5, 11);
	showfreq ((UINT64) 0x21413, 31, 1, 6);
	showfreq ((UINT64) 0x22903, 31, 1, 6);
	showfreq ((UINT64) 0x4110B, 31, 1, 6);
	showfreq ((UINT64) 0x86811, 31, 1, 6);
	showfreq ((UINT64) 0x202053, 31, 1, 6);
	showfreq ((UINT64) 0x23A979B, 31, 7, 15);
	showfreq ((UINT64) 0x263CADD, 31, 7, 15);
	showfreq ((UINT64) 0x32DEA27, 31, 7, 15);
	showfreq ((UINT64) 0x48DBC57, 31, 7, 15);
	showfreq ((UINT64) 0x237A9B47, 35, 8, 17);
	showfreq ((UINT64) 0x2068453, 37, 2, 9);
	showfreq ((UINT64) 0x81882D1, 37, 2, 9);
	showfreq ((UINT64) 0x235929DF17, 43, 10, 21);

	return 0;
}
