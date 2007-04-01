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

/************************************************************************
* Do 2 x 2 block averaging												*
*                                                                       *
* If in file extensions omitted, assumes ".img" is meant.               *
*																		*
* subsample file-in file-out 											*
*			&optional (rows 454) (columns 576) 							*
*                                                                       *
* User can interrupt processing by typing control-]                     *
************************************************************************/

/* The following are mostly just for function prototypes */

#include <io.h>
#include <stdio.h>
#include <conio.h>
#include <string.h> 
#include <stdlib.h>
#include <process.h>
#include <errno.h>  
/* #include <math.h> */

#define MAXCOLUMNS 1000 	/* maximum number of columns */
/* #define FNAMELEN 80 */

void inextension(fname)        /* supply extension "img" if none present */
char *fname;
{
    if (strchr(fname, '.') == NULL) strcat(fname, ".img");
}

void outextension(fname)        /* supply extension "sub" if none present */
char *fname;
{
    if (strchr(fname, '.') == NULL) strcat(fname, ".sub");
}

void main(argc, argv)	       /* main program */
char *argv[];	               /* argument vector */
int argc;               	   /* argument count */
{
    FILE *fopen(), *fp_in, *fp_out;
    char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
    int i, j, x, y, k = 0, headlen = 0;
	int rows = 454, columns = 576;
	int ra[MAXCOLUMNS], rb[MAXCOLUMNS], rowout[MAXCOLUMNS/2];
    
    if (argc >= 2) {
        strcpy(fn_in, argv[1]);     /* grab it */
		if (argc >= 3) strcpy(fn_out, argv[2]);     /* grab it */
		else strcpy(fn_out, fn_in);
		inextension(fn_in);           /* add extension if needed */
		outextension(fn_out);           /* add extension if needed */
	}
	else {                  /* incorrect number of arguments */
		fprintf (stderr, "Correct usage is:\n");
		fprintf (stderr, "%s <input-file> <output-file>\n",
			argv[0]);
		exit(1);
	}
	if (argc >= 4) rows = (int) strtol(argv[3], NULL, 10);
	if (argc >= 5) columns = (int) strtol(argv[4], NULL, 10);
	
	fprintf(stdout, "Rows = %4d Columns = %4d\n",	rows, columns);

	if ((fp_in = fopen(fn_in, "rb")) == NULL) {
		perror(fn_in);          /* some error - like file not found? */
		exit(2);
	}
	if ((fp_out = fopen(fn_out, "wb")) == NULL) {
		perror(fn_out);          /* some error - like no disk space? */
		exit(2);
	}

    /* At this point we have the input and output file all set */

    /* Now in safe position to go read up to EOF */
    
	for (j = 0; j < headlen; j++) getc(fp_in); /* flush header */

	for (i = 0; i < rows/2; i++) {
			
		for (j = 0; j < columns; j++) ra[j] = getc(fp_in);
		if (ra[columns-1]==EOF) fprintf(stderr, 
			"EOF reached on input file in row = %4d\n", i+i);

		for (j = 0; j < columns; j++) rb[j] = getc(fp_in);
		if (rb[columns-1]==EOF) fprintf(stderr, 
			"EOF reached on input file in row = %4d\n", i+i+1);
		
		for (j = 0; j < columns/2; j++) {
		
			rowout[j] = (ra[j+j] + rb[j+j] + ra[j+j+1] + rb[j+j+1] + 2)/4;

/*			fprintf (stdout, "%4d %4d : %4d %4d %4d %4d -> %4d\n",
				i+i, j+j, ra[j+j], ra[j+j+1], rb[j+j], rb[j+j+1],
				rowout[j]); */

		}
		for (j = 0; j < columns/2; j++) putc(rowout[j], fp_out);
	}

    fclose(fp_in);
	fclose(fp_out);
    if (k != 0)  {	/* k non-zero; encountered some bug in the above */
		remove(fn_out);         /* delete bad output file */
	}
	exit(k);
}



