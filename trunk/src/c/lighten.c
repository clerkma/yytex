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

int maptable[16] = {
	7, 7, 7, 7, 7, 7, 7, 7,
	15, 7, 7, 15, 7, 15, 15, 15
};

void remap(char *buffer) {
	char *s = buffer;
	int c;
	while ((c = *s) != '\0') {
		if (c == '\n') break;
		if (c >= '0' && c <= '9') c = c - '0';
		else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
		else fprintf(stderr, "SHIT: %d ", c);
		if (c < 0 || c > 15) fprintf(stderr, "CRAP: %d ", c);
		c = maptable[c];
		if (c < 0 || c > 15) fprintf(stderr, "MAPPED CRAP: %d ", c);
		if (c > 9) c = (c - 10) + 'A';
		else if (c > 0)  c = c + '0';
		else fprintf(stderr, "JUNK: %d ", c);
		if (c < 32 || c > 128) fprintf(stderr, "GARBAGE: %d ", c);
		*s++ = (char) c;
	}
}


int main(int argc, char *argv[]) {       /* main program */
	FILE *input, *output;
	char infilename[] = "c:\\ps\\framing.ps";
	char outfilename[] = "d:\\framingn.ps";
	char buffer[256];
	
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);
		return -1;
	}
	
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);
		return -3;
	}
	
	for(;;) {
		if (fgets(buffer, 256, input) == NULL) break;
		fputs(buffer, output);
		if (strstr(buffer, "BeginBinary") != NULL) break;
	}
	fgets(buffer, 256, input);
	fputs(buffer, output);	
	for(;;) {
		if (fgets(buffer, 256, input) == NULL) break;
		if (strstr(buffer, "EndBinary") != NULL) break;
		remap(buffer);
		fputs(buffer, output);
	}	
	fputs(buffer, output);
	for(;;) {
		if (fgets(buffer, 256, input) == NULL) break;
		fputs(buffer, output);
		if (strstr(buffer, "EOF") != NULL) break;
	}	
	putc(4, output);
	fclose(input);
	fclose(output);
	return 0;
}
