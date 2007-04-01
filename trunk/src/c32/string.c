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

int verboseflag=0;		/* debugging */

int traceflag=0;		/* debugging */

int threshold1=4;		/* if vowel found */

int threshold2=5;		/* if only consonants */

int unicodeflag=0;		/* search for Unicode text strings only */

int ischar(int c) {
	if (unicodeflag) {
		if ((c >= 32 && c < 127) || (c >= 160 && c < 255) ||
			c == '\n' || c == '\r' || c == '\t') return 1;
		else if (c == 31) return 1;				/* ? */
		else return 0;
	}
	else {
		if ((c >= 32 && c < 127) ||
			c == '\n' || c == '\r' || c == '\t') return 1;
		else return 0;
	}
}

int isletter(int c) {
	if ((c >= 65 && c <= 90) ||
		(c >= 97 && c <= 122)) return 1;
	else return 0;
}

int isvowel(int c) {
	if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
		c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') return 1;
	else return 0;
}

void stringize (FILE *input) {
	int c, k;
	int count=0, vowel=0;
	long start;

	if (verboseflag) printf("Entering stringize\n");

	if (unicodeflag) {
		while ((c = getc(input)) >= 0) {
			if (c != 0 && c != 31)
				continue;	/* skip to start of two-byte ASCII char */
			c = getc(input);		/* secont byte of first char */
			for (;;) {
				if (ischar(c)) {
					if (count == 0) {
						start = ftell(input);
						count++;
					}
					else {
						if (isletter(c)) count++;
						if (isvowel(c)) vowel++;
					}
					
					if (count >= threshold2 ||
						(count >= threshold1 && vowel != 0)) {
						if (start == 0) start++;
						if (traceflag)
							printf("Seeking back to %ld\n", start-2);
						fseek(input, start-2, SEEK_SET);
						/*				putc('#', stdout); */
						for (k = 0; k < count; k++) {
							c = getc(input);			/* should be zero */
							putc(getc(input), stdout);
						}
						/*				putc('@', stdout); */
						c = getc(input);		/* should be zero */
						for(;;) {
							if (c != 0 && c != 31) break;
							c = getc(input);
							if (ischar(c)) putc(c, stdout);
							else break;
							c = getc(input);
						}
						/*				putc('*', stdout); */
						putc('\n', stdout);
						count = 0;					
						vowel = 0;
						ungetc(c, input);	/* ? */
						break;	/* ? */
					}
				}				/* if (ischar(c)) ... */
				else {			/* not a character */
					count = 0;
					vowel = 0;			
					ungetc(c, input);	/* ? */
					break;
				}
				c = getc(input);
				if (c != 0 && c != 31) {
					count = 0;
					vowel = 0;			
					ungetc(c, input);	/* ? */
					break;
				}
				c = getc(input);
			}
		}
	}		
	else {
		while ((c = getc(input)) >= 0) {
			c = getc(input);
			if (ischar(c)) {
				if (count == 0) {
					start = ftell(input);
					count++;
				}
				else {
					if (isletter(c)) count++;
					if (isvowel(c)) vowel++;
				}
				
				if (count >= threshold2 ||
					(count >= threshold1 && vowel != 0)) {
					if (start == 0) start++;
					if (traceflag) printf("Seeking back to %ld\n", start-1);
					fseek(input, start-1, SEEK_SET);
					/*				putc('#', stdout); */
					for (k = 0; k < count; k++) putc(getc(input), stdout);
					/*				putc('@', stdout); */
						while (ischar(c = getc(input))) putc(c, stdout);
						/*				putc('*', stdout); */
						putc('\n', stdout);
						count = 0;					
						vowel = 0;
				}
			}				/* if (ischar(c)) ... */
			else {			/* not a character */
				count = 0;
				vowel = 0;			
				break;
			}
		}
	}
}

void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

int main(int argc, char *argv[]) {
	FILE *input;
	int m=1;

	if (argc < m+1) exit(1);
	while (m < argc && argv[m][0] == '-') { 
		if (strcmp(argv[m], "-v") == 0) {
			verboseflag = ! verboseflag;
		}
		else if (strcmp(argv[m], "-t") == 0) {
			traceflag = ! traceflag;
		}
		else if (strcmp(argv[m], "-u") == 0) {
			unicodeflag = ! unicodeflag;
		}
		m++;
	}
	if (argc < m+1) {
		printf("No argument given\n");
		exit(1);
	}
	for (; m < argc; m++) {
		if ((input = fopen(argv[m], "rb")) == NULL) {
			extension(argv[m], "exe");
			if ((input = fopen(argv[m], "rb")) == NULL) {
				perror(argv[m]);
				exit(1);
			}
		}
		if (verboseflag) printf("Opened %s\n", argv[m]);
		stringize (input);
		if (verboseflag) printf("Closed %s\n", argv[m]);
		fclose(input);
	}
	return 0;
}

