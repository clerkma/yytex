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

/* convert icons from Mac to Windows format and vice versa */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* #define FNAMELEN 80 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

void removeexten(char *fname) {  /* remove extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) != NULL) {
		if ((t = strrchr(fname, '\\')) == NULL || s > t) *s = '\0';
	}
} 

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void removeunder(char *filename) { /* remove Adobe style underscores */
	char *s;
	s = filename + strlen(filename) - 1;
	while (*s == '_') s--;
	*(s + 1) = '\0';
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned char icon[128], mask[128];

int main (int argc, char *argv[]) {
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	FILE *input, *output;
	int i, j, k;

	if (argc < 2) exit(7);
	
	strcpy(infilename, argv[1]);
	extension(infilename, "ico");
	if ((input = fopen(infilename, "rb")) == NULL) {
		perror(infilename); exit(3);
	}
	
	strcpy(outfilename, "newicon.ico");
	if ((output = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename); exit(3);
	}	
	
	for (k = 0; k < 70; k++) putc(getc(input), output);
	for (k = 0; k < 128; k++) icon[k] = (unsigned char) getc(input);
	for (k = 0; k < 128; k++) mask[k] = (unsigned char) getc(input);
	
	fclose(input);
	
	for (i = 0; i < 32; i++) {
		k = (31 - i) * 4;
		for (j = 0; j < 4; j++) {
			putc(icon[k] ^ mask[k], output); k++;
		}
	}
	
	for (i = 0; i < 32; i++) {
		k = (31 - i) * 4;
		for (j = 0; j < 4; j++) {
			putc(~mask[k], output); k++;
		}
	}

	fclose(output);

	return 0;
} 
