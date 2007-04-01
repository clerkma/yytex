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

/* Adjust dy in seac calls in PLN file (used for CSC fonts) */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int offset=0;

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

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

#define MAXLINE 256
#define MAXCHARNAME 256

int adjustseac(FILE *output, FILE *input) {
	char line[MAXLINE];
	char charname[MAXCHARNAME]="";
	int asb, dx, dy, bchr, achr;
	int length;
	
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '/') sscanf(line, "/%s %d", charname, length);
		if (strstr(line, "seac") != NULL) {
			if (sscanf(line, "%d %d %d %d %d seac",
					   &asb, &dx, &dy, &bchr, &achr) == 6) {
				dy = dy + offset;
				sprintf(line, "%d %d %d %d %d seac\n",
						asb, dx, dy, bchr, achr);
			}
			else fprintf(stderr, "BAD: %s", line);
		}
		fputs(line, output);		
	}
}

int main(int argc, char *argv[]) {
	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	FILE *input, *output;
	int firstarg=1;
	char *s;

	if (argc < firstarg+1) exit(1);
	while (*argv[firstarg] == '-') {
		s = argv[firstarg];
		if (strchr(s), 'o') != NULL) {
			if (sscanf(s, "-o=%d", &offset) == 0) {
			}
			else fprintf(stderr, "BAD: %s\n", s);
		}
		else fprintf(stderr, "BAD: %s\n", s);
		firstarg++;
	}
	if (argc < firstarg+1) exit(1);
	strcpy(infile, argv[firstarg]);
	extension(infile, "pln");
	strcpy(outfile, removepath(infile));
	forceexten(outfile, "new");
	if ((input = fopen(infile, "r")) == NULL) {
		perror(infile);
		exit(1);
	}
	if ((output = fopen(outfile, "w")) == NULL) {
		perror(outfile);
		exit(1);
	}
	printf("%s => %s\n", infile, outfile);
	adjustseac(output, input);
	fclose(output);
	fclose(input);
	return 0;
}
