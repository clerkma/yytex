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

int traceflag=0;

int sreadtwoold (FILE *input) {
	short int result;
/*	{
		int c, d;
		c = getc(input);	d = getc(input);
		if (traceflag) printf("%d %d\n", c, d);
		result = ((short int) c << 8) | (short int) d;
	} */
	result = ((short int) getc(input) << 8) | (short int) getc(input); 
	return result;
}

int sreadtwo (FILE *input) {			/* fixed 98/Feb/7 */
	int c, d;
	c = getc(input);	d = getc(input);
	if (traceflag) printf("%d %d\n", c, d);
	if (c > 127) c = c - 256; 
	return  (c << 8) | d;
}


int main(int argc, char *argv[]) {
	int c, nold, n;
	FILE *input;
	char *infilename="nfm.tfm";
	long present;

	if ((input = fopen(infilename, "rb")) == NULL) {
		perror(infilename);
		exit(1);
	}
	for(;;) {
		c = getc(input);
		ungetc(c, input);
		if (c == EOF) break;
		present = ftell(input);
		nold = sreadtwoold(input);
/*		fseek(input, -2, SEEK_CUR); */
		fseek(input, present, SEEK_SET);
		n = sreadtwo(input);
		if (nold != n) {
			printf("%04X\t%04X ERROR!!!!!!!!!!!!!!!!!!\n", nold, n);
		}
		else printf("%04X\t%04X\n", nold, n);
	}
	fclose(input);
	return 0;
}
