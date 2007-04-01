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
#include <stdlib.h>

/* /uni0000 16 RD */
/* 0 1369 hsbw */
/* 100 callsubr */
/* 100 callsubr */
/* endchar */
/* ND */

void gencharstrings(FILE *output, int n) {
	int k, l, h;
	for (k=0; k < n; k++) {
		l = k & 255;
		h = (k >> 8) & 255;
		fprintf(output, "/uni%04X 0 RD\n", k);
		fprintf(output, "0 1369 hsbw\n");
		fprintf(output, "%d callsubr\n", l+100);
		fprintf(output, "%d callsubr\n", h+100);
		fprintf(output, "endchar\n");
		fprintf(output, "ND\n");
		fprintf(output, "\n");
	}
}

int main(int argc, char *argv[]) {
	gencharstrings(stdout, 65536);
	return 0;
}
