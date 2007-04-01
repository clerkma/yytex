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

/************************************************************************
* Smooth once using binomial filter										*
*                                                                       *
* If file extensions omitted, assumes ".img" is meant.                  *
* If only one file name given, output will overwrite input.             *
* If no command line arguments given, uses standard input and output;   *
* this allows piping, using the "|"  operator                           * 
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

/* #define FNAMELEN 80 */
#define HEADLEN 0         	/* header length */
#define ROWS 201			/* rows in image */
#define COLUMNS 201			/* columns in image */

/* This code does not conform totally to proposed ANSI C standard...  *
 * I was too lazy to stick in all the function prototypes and such... */

inextension(fname)        /* supply extension "img" if none present */
char *fname;
{
    if (strchr(fname, '.') == NULL) strcat(fname, ".img");
}

outextension(fname)        /* supply extension "smo" if none present */
char *fname;
{
    if (strchr(fname, '.') == NULL) strcat(fname, ".smo");
}

makename(fname)     /* makeup unique temporary filename */
char *fname;
{
    char *s;
    s = strrchr(fname,'\\');  /* search back to directory part */
    if (s == NULL) s = strrchr(fname,':');   /* search back to drive part */
    strcpy(s+1, "XXXXXX");   /* stick in template of six characters */
    mktemp(fname);           /* replace template by unique string */
    outextension(fname);        /* add extension (bad idea?) */
}

main(argc, argv)            /* main program */
char *argv[];               /* argument vector */
int argc;                   /* argument count */
{
    FILE *fopen(), *fp_in, *fp_out;
    char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];	 /* allow room for file names */
	int rowa[COLUMNS], rowb[COLUMNS], rowout[COLUMNS];
    int c, i, j, n, k = 0;  
    
    if (argc >= 2) {
        strcpy(fn_in, argv[1]);     /* grab it */
        inextension(fn_in);           /* add extension if needed */
		if (argc == 3) strcpy(fn_out, argv[2]);
		else strcpy(fn_out, argv[1]);
		outextension(fn_out);      	/* add extension if needed */
	}
	else {                  /* incorrect number of arguments */
		fprintf (stderr, "Correct usage is:\n");
		fprintf (stderr, "%s <input-a> <input-b> <output-file>\n",
			argv[0]);
		exit(1);
	}
	if ((fp_in = fopen(fn_in, "rb")) == NULL) {
		perror(fn_in);          /* some error - like file not found? */
		exit(2);
	}
	if ((fp_out = fopen(fn_out, "wb")) == NULL) {
		perror(fn_out);         /* some error - like out of disk space? */
		exit(3);
	}

    /* At this point we have the input and output files all set */

    /* Now in safe position to go read and write up to EOF */
    
	for (j = 0; j < HEADLEN; j++) getc(fp_in); /* flush header */

	for (j = 0; j < COLUMNS; j++) rowa[j] = getc(fp_in); 

	for (i = 0; i < (ROWS - 1); i++) {
	
		for (j = 0; j < COLUMNS; j++) rowb[j] = getc(fp_in);

		for (j = 0; j < (COLUMNS-1); j++) {
			rowout[j] = (rowa[j+1] + rowa[j] + rowb[j+1] + rowb[j])/4;
		}

		rowout[(COLUMNS-1)] = (rowa[(COLUMNS-1)] + rowb[(COLUMNS-1)])/2;

		for (j = 0; j < COLUMNS; j++) putc(rowout[j], fp_out);
		
		for (j = 0; j < COLUMNS; j++) rowa[j] = rowb[j];
		
	}		

	for (j = 0; j < COLUMNS; j++) putc(rowb[j], fp_out); /* keep same size */

    fclose(fp_in);
    fclose(fp_out);
    if (k != 0)  {	/* k non-zero; encountered some bug in the above */
            remove(fn_out);         /* delete bad output file */
	}
        exit(k);
}


