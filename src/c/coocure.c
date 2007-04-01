/* Copyright 1990, 1991, 1992 Y&Y
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

/* Program to search for particular patterns in binary files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* #define FNAMELEN 80 */

#define RINGSIZE 512

unsigned int ring[RINGSIZE];

int atm=0; atm16=0, atm32=0;

/* catch positions where `chara' is followed by `chara' with offset `shift' */

void cooccurence(FILE *input, int chara, int charb, int shift) {
	int old, new;
	long current;
	int k, n, index;

/* start by filling the ring buffer */
	for (k = 0; k < shift; k++) ring[k] = getc(input);
/*		ring[k] = (char) getc(input); */

	index = 0;
	while ((new = getc(input)) != EOF) {
		old = ring[index];							/* look back by shift */
/*		ring[index++] = (char) new; */				/* insert new in ring */
		ring[index++] = new;						/* insert new in ring */
		if (index >= shift) index = 0;				/* wrap around */
		if (old == chara && new == charb) {			/* coincidence ? */
			current = ftell(input) - shift - 1;
			printf("%6ld: ", current);
			printf("%3d ", old);
			for (k = 0; k < shift; k++) {
				n = k + index;
				if (n >= shift) n -= shift;
				printf("%3d ", ring[n]);
			}
			putc('\n', stdout);
		}
	}
	return;
}

void shownames(FILE *input, long table, long string) {
	int k, c, d, e, f;
	int chr;
	long pos;

	if (atm32 != 0) {

	for (k = 0; k < 256; k++) {
		fseek(input, table + k * 8, SEEK_SET);
/*		chr = getc(input); */
/*		printf("%3d: ", chr); */
/*		(void) getc(input); (void) getc(input); (void) getc(input); */
		c = getc(input); d = getc(input);
		pos = ((long) d << 8) | c;
		(void) getc(input); (void) getc(input); 
		pos = pos + string;
		chr = getc(input); 
		printf("%3d: ", chr); 
		if (fseek(input, pos, SEEK_SET) != 0) {
			fprintf(stderr, "Seek error %ld\n", pos);
			return;
		}
		while ((c = getc(input)) != 0) putc(c, stdout);
		putc('\n', stdout);
	}
	return; }
	else if (atm16 != 0 || atm != 0) {
		pos = string;
		for (k = 0; k < 256; k++) {
			fseek(input, table + k * 6, SEEK_SET);
			c = getc(input); d = getc(input); 
			e = getc(input); f = getc(input);
			chr = getc(input); f = getc(input);
			if (c != 255 || d != 255 || e != 0 || f != 0) break;
			printf("%3d: ", chr);
			fseek(input, pos, SEEK_SET);
			while ((c = getc(input)) != '\0') putc(c, stdout);
			putc('\n', stdout);
			pos = ftell(input);
		}
	}
}

void showmapping(FILE *input, long start, int inc) {
	int k, c, d, e, f, g;
	fseek(input, start, SEEK_SET);
	for (k = 0; k < 256; k++) {
		c = getc(input); 
		e = getc(input); 
		if (inc > 2) {
			f = getc(input); g = getc(input);
			if (e != 0 || f != 0 || g != 0) break;		
		}
		else if (e != 0) break;
		d = getc(input); 
		e = getc(input); 
		if (inc > 2) {
			f = getc(input); g = getc(input);		
			if (e != 0 || f != 0 || g != 0) break;
		}
		else if (e != 0) break;
		printf("%3d => %3d\n", c, d);
	}
}

/* coocure <file> <chara> <charb> <shift> */

int main(int argc, char *argv[]) {
!	char filename[FILENAME_MAX];
	FILE *input;
	int chara, charb, shift=1, increment = 4;
	long tablestart=0, nameoffset = 0, maptable = 0;

	if (argc < 4 + 1) exit(1);

	if (sscanf(argv[2], "%d", &chara) < 1) {
		fprintf(stderr, "Don't understand %s", argv[2]);
		exit(1);
	}
	
	if (sscanf(argv[3], "%d", &charb) < 1) {
		fprintf(stderr, "Don't understand %s", argv[3]);
		exit(1);
	}
	
	if (sscanf(argv[4], "%d", &shift) < 1) {
		fprintf(stderr, "Don't understand %s", argv[4]);
		exit(1);
	}

	strncpy(filename, argv[1], FILENAME_MAX);
	if ((input = fopen(filename, "rb")) == NULL) {
		perror(filename);
		exit(2);
	}
	if (strstr(filename, "32") != NULL) atm32++;
	else if (strstr(filename, "16") != NULL) atm16++;
	else atm++;

	if (atm32 != 0) {
		tablestart = 65108;
		nameoffset = 69932 - 15884;
		maptable = 64812;
		increment = 4;
	}
	else if (atm16 != 0) {
		tablestart = 152350;
		nameoffset = 150634 - 0;
		maptable = 152202;
		increment = 2;
	}
	else if (atm != 0) {
		tablestart = 156020;
		nameoffset = 154410 - 0;
		maptable = 155944;
		increment = 2;
	}

	if (shift < 0) shownames(input, tablestart, nameoffset);
	else if (shift == 0) showmapping(input, maptable, increment);
	else cooccurence(input, chara, charb, shift);

	fclose(input);
	return 0;
}

/* Explicitly mapped to zero in ATM 2.0: */

/* Lslash, breve, dotaccent, fi, fl, fraction,  */
/* hungarumlaut, lslash, minus, ogonek */
