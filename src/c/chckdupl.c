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
#include <string.h>
#include <stdlib.h>
#include <conio.h>

/* check for duplicates in encoding vector */

#define MAXFILENAME 128
#define MAXLINE 512

int maxwidth=800;
int maxpos=1000;

int verboseflag = 0;
int traceflag = 0;

int hitlist[256];

char charnames[256][32];

int gettoken (char *buff, FILE *input) {
	int c, n=0;
	char *s=buff;

	for (;;) {
		c = getc(input);
		if (c == EOF) {
			fprintf(stderr, "ERROR: Unexpected EOF (token %s)\n", buff);
			return EOF;
		}
		if (n++ >= MAXLINE) {
			fprintf(stderr, "ERROR: Token too long (%s)\n", buff);
			getch();
			return n;
		}
		if (c <= ' ') {
			*s++ = '\0';
			if (traceflag != 0) printf("%s\n", buff);
			return n;
		}
		else *s++ = (char) c;
	}
}

int main(int argc, char *argv[]) {
	FILE *input;
	char filename[MAXFILENAME];
	char line[MAXLINE];
	int k, m, chrs, count, flag;
	int firstarg = 1;

/*	if (*argv[firstarg] == '-')  */
	if (firstarg < argc && *argv[firstarg] == '-') {
		if (strchr(argv[firstarg], 'v') != NULL) verboseflag++;
		if (strchr(argv[firstarg], 't') != NULL) traceflag++;		
		firstarg++;
	}

	if (argc < firstarg + 1) exit(1);

	for (m = firstarg; m < argc; m++) {

		strcpy(filename, argv[m]);
		if (strchr(filename, '.') == NULL) strcat(filename, ".pfb");
		if ((input = fopen(filename, "rb")) == NULL) {
			perror(filename);
			exit(1);
		}

		for (k = 0; k < 256; k++) hitlist[k] = 0;

		for (;;) {
			if (gettoken(line, input) == EOF) {
				fprintf(stderr, "ERROR: Unexpected EOF A in %s\n", filename);
				getch();
				break;
/*				exit(1); */
			}
			if (strcmp(line, "/Encoding") == 0) break;
		}

		flag = 0;
		for (;;) {
			if (gettoken(line, input) == EOF) {
				fprintf(stderr, "Unexpected EOF B in %s\n", filename);
				getch();
				break;
/*				exit(1); */
			}
			if (strcmp(line, "StandardEncoding") == 0) {
				flag = -1;
				break;
			}
			if (strcmp(line, "readonly") == 0) break;
			if (strcmp(line, "dup") == 0) {
				(void) gettoken(line, input);
				if(sscanf(line, "%d", &chrs) < 1) {
					fprintf(stderr, "ERROR: Don't understand %s (%s)", 
						line, filename);
					continue;
				}
				if (chrs < 0 || chrs > 255) {
					fprintf(stderr, "ERROR: Bad char number %d (%s)\n", 
						chrs, filename);
					continue;
				}
				(void) gettoken(line, input);
				if (hitlist[chrs]++ != 0) {
					putc('\n', stderr);
					fprintf(stderr, "ERROR: Duplicate encoding %d %s (%s)\n", 
						chrs, charnames[chrs], filename);
					getch();
				}
				strncpy(charnames[chrs], line, 32);
/*				if (verboseflag != 0) putc('.', stdout); */
				if (traceflag != 0) putc('.', stdout);
			}
		}
		fclose(input);
		count = 0;
		for (k = 0; k < 256; k++) if (hitlist[k] > 0) count++;
		if (flag != 0) 	printf("%s uses Adobe StandardEncoding\n", argv[m]);
		else printf("%s has %d characters in encoding\n", argv[m], count);
	}
	printf("\nChecked %d files\n", argc - firstarg);
	return 0;
}
