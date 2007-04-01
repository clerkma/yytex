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

/* remove duplicate lines in file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 1024

char line[MAXLINE];
char oldline[MAXLINE];

int verboseflag=1;

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); 
		strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); 
		strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
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


char *extractfilename(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int removedups (FILE *output, FILE *input) {
	int duplicate=0;
	strcpy(oldline, "");
	while (fgets(line, sizeof(line), input) != NULL) {
		if (_stricmp(line, oldline) == 0) {
			duplicate++;
			if (verboseflag) printf("%s", line);
			continue;
		}
		fputs(line, output);
		strcpy(oldline, line);
	}
	return duplicate;
}

int main(int argc, char *argv[]) {
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	FILE *input, *output;
	int m, n;

	for (m = 1; m < argc; m++) {
		strcpy(fn_in, argv[m]);
		input = fopen(fn_in, "r");
		if (input == NULL) {
			perror(fn_in);
			continue;
		}
		strcpy(fn_out, extractfilename(argv[m]));
		forceexten(fn_out, "nod");		/* for no duplicates */
		output = fopen(fn_out, "w");
		if (output == NULL) {
			perror(fn_out);
			continue;
		}
		printf("Removing duplicate lines %s => %s\n", fn_in, fn_out);
		n = removedups(output, input);
		fclose(output);
		fclose(input);
		if (n > 0) printf("Removed %d repeated line%s\n", n, (n == 1) ? "s" : "");
		else printf("Found no repeated lines to remove\n");
	}

	return 0;
}
