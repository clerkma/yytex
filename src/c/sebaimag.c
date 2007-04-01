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

int main (int argc, char *argv[]) {
	FILE *input, *output;
	int c, k, n, i, byte;
	
	if ((input = fopen("foo.bar", "r")) == NULL) {
		perror("foo.bar");
		exit(1);
	}
	if ((output = fopen("foo.img", "wb")) == NULL) {
		perror("foo.bar");
		exit(1);
	}
	putc('I', output); 	putc('M', output); putc('G', output);
	putc(64, output);
	putc(16, output);
	putc(247, output); 	putc(0, output);			/* rows */
	putc(0, output); putc(4, output);				/* columns */
	putc(1, output);								/* bits per sample */
	putc(0, output); putc(1, output);				/* min, max */
	putc(0, output);								/* flags */
	for (k = 13; k < 16; k++) putc(0, output);		/* reserved */
	fputs("ArcheoInformatica", output);
	for (k = 16 + 17; k < 64; k++) putc(0, output);	/* padding */
	for (k = 0; k < 247; k++) {
		for (n = 0; n < 128; n++) {			/* 1024 / 8 */
			byte = 0;
			for (i = 0; i < 8; i++) {
				c = getc(input);
				if (c == '1') byte = (byte << 1);
				else if (c == '0') byte = (byte << 1) | 1;
				else fprintf(stderr, "c = %d ", c);
			}
			putc(byte, output);
		}
		c = getc(input);
		if (c > ' ') fprintf(stderr, "c = %d ", c);
		while (c <= ' ' && c != EOF) c = getc(input);
		ungetc(c, input);
		putc('.', stdout);
	}
	fclose(output);
	fclose(input);
	return 0;
}
