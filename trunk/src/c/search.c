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

#define MAXARGS 64
#define MAXLINE 8192
#define OPENMODE "rb"

int verboseflag=0;
int traceflag=0;
int casesensitive=1;

int present[MAXARGS];

char line[MAXLINE];

void lowercase(char *s) {
	while (*s != '\0') {
		if (*s >= 'A' && *s <= 'Z') *s = (char) (*s + 'a' - 'A');
		s++;
	}
}

int main(int argc, char *argv[]) {
	int k, m, n;
	int firstarg=1, firstfile=1;
	int count=0, matches=0;
	int missing, lineno, satisfied;
	FILE *input;
	char *s;
	
	if (argc < 2) {
		printf("Usage: SEARCH [-v] [-c] -string1 -string2 ... file1 file2 ... filen\n");
		printf("\tEnclose string in \"...\" if it contains white space\n");
		printf("\t-v verbose mode\n");
/*		printf("\t-t trace mode\n"); */
		printf("\t-c case insensitive\n");
		exit(1);

	}

/*	for (;;) { */
	while (firstarg < argc && *argv[firstarg][0] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag=1;
		else if (strcmp(argv[firstarg], "-t") == 0) traceflag=1;
		else if (strcmp(argv[firstarg], "-c") == 0) casesensitive=0;
		else break;
		firstarg++;
	}

	if (argc < firstarg+1) exit(1);

	while (*argv[firstfile] == '-') {
		s = argv[firstfile]+2;
		if (*s == '"') {
			strcpy(s, s+1);			/* strip off first " */
			n = strlen(s);
			*(s + n - 1) = '\0';	/* strip off second " */
		}
		if (casesensitive == 0) lowercase(argv[firstfile]);
		firstfile++;
	}

	if (firstfile < firstarg+1) exit(1);

	for (m = firstfile; m < argc; m++) {
		for (k = firstarg; k < firstfile; k++) present[k] = 0;
		missing = firstfile - firstarg;
		if ((input = fopen(argv[m], OPENMODE)) == NULL) {
			perror(argv[m]);
			exit(1);
		}
		lineno = 0;
		while (fgets(line, MAXLINE, input) != NULL) {
			if (casesensitive == 0) lowercase(line);
			for (k = firstarg; k < firstfile; k++) {
				if (present[k]) continue;
				if (strstr(line, argv[k]+1) != NULL) {
					if (traceflag) printf("%d\t%s", lineno, line);
					present[k] = 1;
					missing--;
					if (missing == 0) {
						if (verboseflag) printf("%d\t%s", lineno, line);
						break;
					}
				}
			}
			lineno++;
		}
		fclose (input);
		count++;
		satisfied = 1;
		for (k = firstarg; k < firstfile; k++) {
			if (present[k] == 0) {
				satisfied = 0;
				break;
			}
		}
		if (satisfied) {
			printf("File `%s' contains listed strings\n", argv[m]);
			matches++;
		}
		if (satisfied && missing) fprintf (stderr, "Inconsistent state!!!\n");
	}
	printf("Searched %d files, found %d matches\n", count, matches);
	return 0;
}
