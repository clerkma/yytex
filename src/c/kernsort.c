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

/* sort kern table */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <search.h>

/* #define FNAMELEN 80 */
#define MAXKERNS 512
#define MAXLINE 256
#define BUFFERLEN 32000

static char *lines[MAXKERNS * 2];

static char buffer[BUFFERLEN];

#ifdef IGNORE
void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) /* change extension if one present */
{
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}
#endif

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

int compare(char **linea, char **lineb) {
	return strcmp(*linea, *lineb);
}

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	int m, k, nlines;
	char *s;

	for (m = 1; m < argc; m++) {
		strcpy(fn_in, argv[m]);
		extension(fn_in, "krn");
		strcpy(fn_out, argv[m]);	
		forceexten(fn_out, "srt");
		if((input = fopen(fn_in, "r")) == NULL) {
			perror(fn_in); exit(1);
		}
		if((output = fopen(fn_out, "w")) == NULL) {
			perror(fn_out); exit(1);
		}
		printf("reading kern pairs ");
		s = buffer; k = 0;
		while(fgets(s, MAXLINE, input) != NULL) {
			lines[k++] = s;
			s = s + strlen(s) + 1;
		}
		nlines = k;
		fclose(input);
		printf("read %d lines ", nlines);
		printf("sorting ");
		qsort(lines, nlines, sizeof(lines[0]), compare);
		printf("writing sorted kern pairs ");
		for (k = 0; k < nlines; k++) {
			fputs(lines[k], output);
		}
		fclose(output);
	}
	return 0;
}
