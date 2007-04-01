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

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<share.h>

#define MAXLINE 256

/* #define FNAMELEN 80 */

int dotsflag=1;

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	char line[MAXLINE];
	int shflag=0;

	if (argc < 2) exit(1);
	strcpy(infile, argv[1]);
/*	if (argc < 3) shflag = _SH_COMPAT; */
	if (argc < 3) shflag = _SH_DENYNO;
	else shflag = atoi(argv[2]) << 4;
	printf("shflag = %x\n", shflag);
	if ((input = _fsopen(infile, "rb", shflag)) == NULL) {
		perror(infile);
		exit(1);
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (dotsflag) putc('.', stdout);
	}
	fclose(input);
	return 0;
}

/* #define SH_COMPAT	0x00 */	/* compatibility mode */
/* #define SH_DENYRW	0x10 */	/* deny read/write mode */
/* #define SH_DENYWR	0x20 */	/* deny write mode */
/* #define SH_DENYRD	0x30 */	/* deny read mode */
/* #define SH_DENYNO	0x40 */	/* deny none mode */
