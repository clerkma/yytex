/* Copyright 1990, 1991, 1992 Y&Y, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* extracts icon from Mac font file and shows it */

/* #define FNAMELEN 80 */

#define ICONSIZE 128

int hexashow = 1;
int doubleshow = 1;

int main (int argc, char *argv[]) {
	FILE *input;
	char filename[FILENAME_MAX];
	long iconoffset;
	unsigned char iconbuffer[ICONSIZE];
	unsigned c, d;
	int i, j, k;

	if (argc < 3) exit(1);
	strcpy(filename, argv[1]);
	if ((input = fopen(filename, "rb")) == NULL) {
		perror(filename); exit(3);
	}
	if(sscanf(argv[2], "%ld", &iconoffset) < 0) {
		fprintf(stderr, "Don't understand: %s\n", argv[2]);
		exit(5);
	}
	if (iconoffset < 0) {
		iconoffset = - iconoffset;
		hexashow = 0;				/* show in binary instead */
	}
	if (fseek(input, iconoffset, SEEK_SET) != 0) {
		fprintf(stderr, "Seek error: %ld\n", iconoffset);
	}
	for (k = 0; k < ICONSIZE; k++) 	
		iconbuffer[k] = (unsigned char) getc(input);
	fclose(input);

	for (i = 0; i < 32; i++) {
		for (j = 0; j < 4; j++) {
			c = iconbuffer[i * 4 + j];
			if (hexashow != 0) {
				d = c >> 4;
				if (d > 9) d = d + 'A' - 10;
				else d = d + '0';
				putc(d, stdout);
				c = c & 15;
				if (c > 9) c = c + 'A' - 10;
				else c = c + '0';				
				putc(c, stdout);
			}
			else {
				for (k = 0; k < 8; k++) {
					if ((c & 128) != 0) putc('X', stdout);
					else putc('.', stdout);
					if (doubleshow != 0) {
						if ((c & 128) != 0) putc('X', stdout);
						else putc('.', stdout);
					}
					c = c << 1;
				}
			}
		}
		putc('\n', stdout);
	}
	return 0;
}
