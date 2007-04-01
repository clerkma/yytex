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

/* #define FNAMELEN 80 */

int traceflag = 0;

int analyzeflag = 0;

int ignorecount = 1;

int suppresszero=0;
int suppresscontrol=0;
int suppresssplit=0;
int showascii=0;

int total=0;

int column=0;

void analyze(FILE *output, FILE *input) {
	int c, cold=0, canc=0, count=0, field=0;
	
	while ((c = getc(input)) != EOF) {
		if (c == 12) {
			count = canc;
			field=1;
		}
		if (c == 11) {
			count = canc;
			field=0;
		}
		if (suppresszero && c == 0) ;
		else if (suppresssplit && c == 1) {
			putc('\n', output);
			c = getc(input);
			putc('\t', output);
			c = getc(input);
		}
		else if (c < 32) {
			if (suppresscontrol == 0) fprintf(output, " %d ", c);
		}
		else if (c < 128) {
			if (showascii == 0) fprintf(output, " %d ", c);
			else putc(c, output);
		}
		else if (c >= 128 && c <= 128 + 12)
			fprintf(output, "\n(%2d - %2d  - %2d)\t", c - 128, count-6, field);
		else if (c >= 160) putc(c-128, output);
/*		else if (c >= 128 + 'A' && c <= 128 + 'Z') putc(c-128, output); */
/*		else if (c >= 128 + 'a' && c <= 128 + 'z') putc(c-128, output); */
/*		else if (c >= 128 + '0' && c <= 128 + '9') putc(c-128, output); */
		else printf(" %d ", c);
		canc = cold; cold = c;
	}
}

void process(FILE *output, FILE *input) {
	int c, n, cold=0, canc=0, count=0, field=0, type=0;
	
	while ((c = getc(input)) != EOF) {
		if (c == 0) ;
		else if (c == 12) {
			count = canc;
			field=1;
		}
		else if (c == 11) {
			count = canc;
			field=0;
		}
		else if (c >= 128 && c <= 128 + 12) {
			type = c - 128;
			if (type == 0) {
				total++;
				putc('\n', stdout);
			}
			n = count;
			if (type > 4) {
				while (n >= 6 || ignorecount != 0) {
					c = getc(input);
					if (n < 6 && ignorecount != 0 && c < 128 + 32) break;
					n--;
				}
			}
			while (n >= 6 || ignorecount != 0) {
				c = getc(input);
				n--;
				if (n < 6 && ignorecount != 0 && c < 128 + 32) break;
				if (field) {
					if (c == 0) ;
					else if (c == 1) {
						if (traceflag != 0) printf(" END OF LINE");
						putc('\n', output);
						column = 0;
/*						c= getc(input);		n--; */
/*						c= getc(input);		n--; */
					}
					else {
						if (c >= 128 + 32) {
							if (c-128 > 32 || column > 0) {
								putc(c-128, output);
								column++;
							}
						}
					}
				}
			}
			if (type == 0) putc(' ', output);
			else if (type <= 4) {
				if (count > 6) {
					if (traceflag != 0) printf(" END OF FIELD");
					putc('\n', output);
				}
			}
/* 			else putc(' ', output); */
			count = 0;
		}
		canc = cold; cold = c;
	}
}

int main(int argc, char *argv[]) {
!	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	FILE *input, *output;

	if (argc < 2) exit(1);
	strcpy(infile, argv[1]);
	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile); exit(3);
	}
	output = stdout;

	if (analyzeflag != 0) 	analyze (output, input);
	else process (output, input);

	printf("Total of %d addresses\n", total);

	fclose(input);
	fclose(output);
	return 0;
}
