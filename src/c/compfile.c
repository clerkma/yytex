/* Copyright 1990, 1991, 1992 Y&Y
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

/* Quick hack for comparing two files */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAXFILENAME 128

int main(int argc, char *argv[]) {
	char filex[MAXFILENAME], filey[MAXFILENAME];
	FILE *fx, *fy;
	int c, d;
	long fxlen, fylen, filepos;

	if (argc != 3) {
		fprintf(stderr, "Wrong number of arguments\n");
		exit(9);
	}
	strcpy(filex, argv[1]);
	strcpy(filey, argv[2]);
	if(strcmp(filex, filey) == 0) {
		fprintf(stderr, "Comparing file with itself?\n");
	}
	if ((fx = fopen(filex, "rb")) == NULL) {
		perror(filex); exit(8);
	}
	if ((fy = fopen(filey, "rb")) == NULL) {
		perror(filey); exit(8);
	}
	if(fseek(fx, 0, SEEK_END) != 0) {
		fprintf(stderr, "Seek error on first file\n");
	}
	if(fseek(fy, 0, SEEK_END) != 0) {
		fprintf(stderr, "Seek error on second file\n");
	}
	fxlen = ftell(fx);
	fylen = ftell(fy);
	if (fxlen != fylen) {
		fprintf(stderr, "First file length %ld - Second file length %ld\n",
			fxlen, fylen);
		exit(7);
	}
	if(fseek(fx, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Seek error on first file\n");
	}
	if(fseek(fy, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Seek error on second file\n");
	}
	for(;;) {
		c = getc(fx); d = getc(fy);
		if (c != d) {
			filepos = ftell(fx);
			fprintf(stderr, "First file %X Second file %X at byte %ld\n",
				c, d, filepos - 1);
			exit(5);
		}
		else if (c == EOF || d == EOF) break;
	}
	if (ferror(fx) != 0) {
		perror(filex);
	}
	else fclose(fx);
	if (ferror(fy) != 0) {
		perror(filey);
	}
	else fclose(fy);
	return 0;
}
