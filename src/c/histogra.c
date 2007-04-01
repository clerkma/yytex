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
* Make histogram of grey-levels in file									*
*                                                                       *
* If only one file name given, output will have extension .his          *
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
#define HEADLEN 64				/* default header length */

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

int main(int argc, char *argv[])       /* main program */
{
    FILE *fopen(), *fp_in, *fp_out;
    char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX]; /* allow room for file names */
    int c, d, e, i, k = 0, headerlength=HEADLEN;  
	long histogram[256], sum = 0L;
    
    if (argc == 1) {        /* no command line arguments? */
		fp_in = stdin;      /* use standard input and output */
		fp_out = stdout;
        fn_in[0] = '\0';        /* note lack of file name */
		fn_out[0] = '\0';        
	}
	else {                      /* at least one command line argument */
		strcpy(fn_in, argv[1]);     /* grab it */
		extension(fn_in, "img"); 	/* add extension if needed */
		if (argc == 2) {            /* one command line argument only? */
			strcpy(fn_out, argv[1]);  /* use same file name */
            forceexten(fn_out, "his");      /* add extension  */
		}	
        else if (argc == 3) {   /* input and output arguments given? */
            strcpy(fn_out, argv[2]);
			extension(fn_out, "his");      /* add extension if needed */
		}
        else {                  /* incorrect number of arguments */
            fprintf (stderr, "Correct usage is:\n");
            fprintf (stderr, "%s <input-file> <output-file>\n",
                argv[0]);
            exit(1);
		}
	}

	if ((fp_in = fopen(fn_in, "rb")) == NULL) {
		perror(fn_in);          /* some error - like file not found? */
		exit(2);
	}
	if ((fp_out = fopen(fn_out, "wb")) == NULL) {
		perror(fn_out);         /* some error - like out of disk space? */
		exit(3);
	}

	c = getc(fp_in); d = getc(fp_in); e = getc(fp_in);
	if (c == 'I' && d == 'M' && e == 'G') headerlength = getc(fp_in);
	else getc(fp_in);

    /* At this point we have the input file all set */

    /* Now in safe position to go read up to EOF */
    
	for (i = 4; i < headerlength; i++) getc(fp_in); /* flush header */

	for (c = 0; c < 256; c++)	histogram[c] = 0L;

	c = getc(fp_in);
    while (c != EOF)
		{histogram[c]++;
		c = getc(fp_in);
		}

    fclose(fp_in);

/*	sum = 0L; */
	for (c = 0; c < 256; c++) {
		if (histogram[c] > 0) {
			sum = sum + histogram[c];
 			fprintf(fp_out, "i =%4d, n =%8ld, sum =%8ld\n", c, histogram[c], sum); 
		}
	}

    fclose(fp_out);
	return 0;
}
