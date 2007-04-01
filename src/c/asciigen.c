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

/* ASCII85 encoding (b1 b2 b3 b4) => (c1 c2 c3 c4 c5) */

/* b1 * 256^3 + b2 * 256^2 + b3 * 256  + b4 = 
   c1 * 85^4  + c2 * 85^3  + c3 * 85^2 + c4 * 85 + c5 */

/* Add `!' =  33 to make ASCII code. ! represents 0, u represent 84. */

/* If all five digits are zero, use z instead of !!!!! - special case. */

/* At end, given n = 1, 2, 3 bytes of binary data, append 4 - n zeros.
   Encode this in usual way (but *without* z special case). 
   Write first (n+1) bytes.  Follow by -> (EOD) */

unsigned long asciinum=0;	/* accumulated 32 bit number */
int asciicount=0;			/* how many bytes seen so far 0, 1, 2, 3, 4 */
int asciicolumn=0;			/* what column we are in output keep <= 80 */

/* Output 32 bit unsigned int as 5 ASCII 85 bytes */

void asciilong (FILE *output, unsigned long n, int nbytes) {
	unsigned int c[5];
	int k;

	if (nbytes == 0) nbytes--;			/* nothing to do */
	if (n == 0 && nbytes == 4) {
		putc('z', output);
		if (asciicolumn++ >= 64) {
			putc('\n', output);
			asciicolumn = 0;
		}
		return;
	}
	for (k = 4; k >= 0; k--) {
		c[k] = (unsigned int) (n % 85);
		n = n / 85;
	}
	for (k = 0; k < 5; k++) {
		if (nbytes-- < 0) {			
			asciicolumn++;	asciicolumn++;
			if (asciicolumn >= 64) {
				putc('\n', output);
				asciicolumn = 0;
			}
			putc('~', output);			/* EOD */
			putc('>', output);
			putc('\n', output);
			asciicolumn = 0;
			return;
		}
		putc(c[k] + '!', output);
		asciicolumn++;
		if (asciicolumn >= 64) {
			putc('\n', output);
			asciicolumn = 0;
		}
	}
}

/* Reset ASCII85 output buffering */

void asciiinit (void) {
	asciinum = 0;
	asciicount = 0;
	asciicolumn = 0;
}

/* Add one byte to ASCII85 output stream */

void asciiout(FILE *output, unsigned int x) {
	if (x > 255) x = x & 255;
	asciinum = (asciinum << 8) | x;
	asciicount++;
	if (asciicount >= 4) {
		asciilong(output, asciinum, asciicount);
		asciinum = 0;
		asciicount = 0;
	}
}

void hexout(FILE *output, unsigned int x) {
	int c, d;
	if (x > 255) x = x & 255;
	d = x & 15;
	c = (x >> 4); 
	if (c >= 10) putc(c + 'A' - 10, output);
	else putc(c + '0', output);
	if (d >= 10) putc(d + 'A' - 10, output);
	else putc(d + '0', output);
}

/* Called at end in ASCII85 compress to flush out buffered data */

void asciiflush (FILE *output) {		/* asciicount < 4 */
	int k;
/*	if (asciicount == 0) return; */		/* nothing to do ! */
	for (k = asciicount; k < 4; k++) {
		asciinum = (asciinum << 8);		/* append n-4 zeros */
	}
	asciilong(output, asciinum, asciicount);
}

/********************************* test program ***********************/

void testout (FILE *output) {
	int i, j, k, n;
	int nrows=32, ncolumns = 64;
/*	for (n = 1; n < 16; n++) {
		asciiinit();
		for (k = 0; k < n; k++) asciiout (output, k+17);
		asciiflush(output);
	} */
	asciiinit();
	for (i = 0; i < nrows; i++) {
		for (j = 0; j < ncolumns; j++) {
			asciiout (output, (i+4*j) & 255);
/*			asciiout (output, (j*4) & 255);  */
/*			hexout (output, (i+4*j) & 255);  */
		}
/*		putc('\n', output); */
	}
	asciiflush(output);
}

/************************* test code for reading ASCII85 ********************/

void asciireset (void) {
	asciinum = 0;
	asciicount = 0;
}

void shownum(unsigned long n, int nbytes) {
	int k;
	unsigned int c[4];
	for (k = 3; k >= 0; k--) {
		c[k] = (unsigned int) (n & 255);
		n = (n >> 8);
	}
	putc('(', stdout);
	for (k = 0; k < nbytes-1; k++)
		printf("%u, ", c[k]);
	putc(')', stdout);
	putc(' ', stdout);
}

int testin(FILE *input) {
	int c, d, k;
	asciireset();
	for (;;) {
		c = getc(input);
		if (c == EOF) {
			printf("EOF\n");
			return -1;
		}
		if (c <= 32) continue;	/* ignore white space */
		if (c == '~') {
			d = getc(input);
			if (d == '>') {
				for (k = asciicount; k < 5; k++)
/*					asciinum = asciinum * 85; */
					asciinum = asciinum * 85 + 84; /* ??? */
				shownum(asciinum, asciicount);
				return -1;		/* EOD */
			}
			else {
				fprintf(stderr, "Incorrect EOD\n");
				ungetc(d, input);
			}
		}
		if (c == 'z') {
			if (asciicount != 0) fprintf(stderr, "z in wrong place\n");
			asciinum = 0;
			shownum(asciinum, 5);
			asciireset();
		}
		else {
			asciinum = asciinum * 85 + (c - '!');
			asciicount++;
			if (asciicount >= 5) {
				shownum(asciinum, asciicount);
				asciireset();
			}
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc > 1) testin(stdin);
	else testout(stdout);
	return 0;
}
