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
#include <string.h>
#include <stdlib.h>

typedef struct {
	long zeroL;			/* always 0L */
/*	int	rsvptrs;	*/	/* always 5 */
/*	int *pLocalHeap; */	/* => local heap */
/*	int *pAtomTable; */	/* => atom table */
/*	int *pStackTop; */	/* => stack top (low end) */
/*	int *pStackMin;	*/	/* => stack minimum used */
/*	int *pStackBot;	*/	/* => stack bottom (high end) */
} *NPTASKHEADER;

static NPTASKHEADER npTaskHeader;	/* is NULL */

int  main(int argc, char *argv[]) {	
	long zero;
	long *foo=NULL;

	npTaskHeader = (NPTASKHEADER) NULL;	/* debugging */

	zero = npTaskHeader->zeroL;
	printf("zero %ld\n", zero);
	if (zero != 0L) {
		fprintf(stderr, "WARNING: NULL pointer used! %ld\n", zero);
	}

	npTaskHeader->zeroL = 12345;

	zero = npTaskHeader->zeroL;

	printf("zero %ld\n", zero);
	if (zero != 0L) {
		fprintf(stderr, "WARNING: NULL pointer used! %ld\n", zero);
	}

	npTaskHeader->zeroL = 0;
	
	*foo = 54321;

	zero = npTaskHeader->zeroL;

	printf("zero %ld\n", zero);
	if (zero != 0L) {
		fprintf(stderr, "WARNING: NULL pointer used! %ld\n", zero);
	}

	return 0;
}
