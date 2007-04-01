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

/* copy InternalLeading and PixHeight from new form to old form PFM file */

/* old PFM file has extension PFM, new PFM file has extension NEW */
/* on command line list all OLD PFM files (to be updated) */

/* #define FNAMELEN 80 */

int InternalLeading;
int PixHeight;

int verbosemode=1;

void copyheight (FILE *old, FILE *new) {
	int c, d;
	fseek(old, 76, SEEK_SET);
	fseek(new, 76, SEEK_SET);	
	c = getc(new);
	putc(c, old);
	d = getc(new);
	putc(d, old);
	InternalLeading = (d << 8) | c;
	fseek(old, 88, SEEK_SET);
	fseek(new, 88, SEEK_SET);	
	c = getc(new);
	putc(c, old);
	d = getc(new);
	putc(d, old);
	PixHeight = (d << 8) | c;
}

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

int main (int argc, char *argv[]) {
	FILE *new, *old;
	char newname[FILENAME_MAX], oldname[FILENAME_MAX];
	int m;
	for (m = 1; m < argc; m++) {
		strcpy(oldname, argv[m]);
		extension (oldname, "pfm");
		if ((old = fopen(oldname, "rb+")) == NULL) {
			perror(oldname);
			exit(1);
		}
		strcpy(newname, argv[m]);
		forceexten (newname, "new");
		if ((new = fopen(newname, "rb")) == NULL) {
			perror(newname);
			exit(1);
		}
		copyheight (old, new);
		if (verbosemode)
			printf("InternalLeading %d\tHeight %d\tFont %s\n",
				InternalLeading, PixHeight, oldname);
		fclose(old);
		fclose(new);
	}
	return 0;
}
