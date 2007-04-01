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

#define BUFSIZE 65536

char buffer1[BUFSIZE];
char buffer2[BUFSIZE];

long comparelength(FILE *input1, FILE *input2) {
	long nlen1, nlen2;
	fseek(input1, 0, SEEK_END);
	nlen1 = ftell(input1);
	fseek(input2, 0, SEEK_END);
	nlen2 = ftell(input2);
	return (nlen1 - nlen2);
}

int comparefiles(FILE *input1, FILE *input2) {
	int nlen1, nlen2;
	int k;
	unsigned char *s1, *s2;

	for (;;) {
		nlen1 = fread(buffer1, 1, BUFSIZE, input1);
		nlen2 = fread(buffer2, 1, BUFSIZE, input2);
		if (nlen1 != nlen2) return 1;
		if (nlen1 == 0 || nlen2 == 0) break;
		s1 = buffer1;
		s2 = buffer2;
		for (k = 0; k < nlen1; k++) {
			if (*s1++ != *s2++) return 1;
		}
	}
	return 0;
}


int main(int argc, char *argv[]) {
	FILE *input1, *input2;
	char filename1[FILENAME_MAX];
	char filename2[FILENAME_MAX];
	long nlendif;
	int flag;
	
	if (argc < 3) exit(1);

	strcpy(filename1, argv[1]);
	strcpy(filename2, argv[2]);
	input1 = fopen(filename1, "rb");
	if (input1 == NULL) {
		perror(filename1);
		exit(1);
	}
	input2 = fopen(filename2, "rb");
	if (input2 == NULL) {
		perror(filename2);
		exit(1);
	}

	nlendif = comparelength(input1, input2);
	if (nlendif != 0) printf("Files %s %s have diffent length\n", filename1, filename2);
	else {
		flag = comparefiles(input1, input2);
		if (flag != 0) printf("Files %s %s are different\n", filename1, filename2);
		else printf("File %s same as %s\n", filename1, filename2);
	}
	fclose(input2);
	fclose(input1);

	return 0;
}
