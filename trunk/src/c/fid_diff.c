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

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define MAXFILENAME 80

#define MAXLINE 256

int main(int argc , char *argv[]) {
	int m;
	FILE *input, FILE *output;
	char infilename[MAXFILENAME];
	char bakfilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	if (argc < 2) exit(1);
	for (m = 1; m < argc; m++) {
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;
		}
		if ((output = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename);
			fclose(input);
			continue;
		}
		processpdf(output, input);
		fclose(output);
		fclose(input);
	}
	return 0;
}

int findstring (FILE *output, FILE *input, char *str) {
	int c;
	char *s;

	s = str;
	c = getc(input);
	if (c == EOF) return EOF;
	for (;;) {
		while (c == *s) {
			putc(c, output); 
			c = getc(input);
			if (c == EOF) return EOF; 
			if (c == EOF) break;
			s++;
			if (*s == '\0') return 0;	/* match found */
		}
/*		t = str; */
/*		while (t < s) putc(*t++, output); *//* put out partial match */
/*		if (c == EOF) return EOF; */
		putc(c, output);				/* first mismatched character */
		c = getc(input);
		if (c == EOF) return EOF;
		s = str;
	}
}

int processpdf (FILE *output, FILE *input) {
	while (findstring(output, input, "/Differences [") != EOF) {
/*		fputs("/Differences [", output); */


	}
	return EOF;
}

/* pattern to deal with */

n 0 obj
<<
/Type /Encoding
/Differences [
/AE/OE/Oslash 94/circumflex 126/tilde/dieresis/Lslash/quotesingle 
...
]
>>
endobj

change to:

/AE/OE/Oslash 39/quoteright 94/circumflex 96/quoteleft 126/tilde/dieresis/Lslash/quotesingle 
