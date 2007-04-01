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

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define TRACEFLAG 0
#define DEBUGFLUSH 0
#define MAXLINE 256
/* #define FNAMELEN 80 */

static char line[MAXLINE];

static char *mess = "\
/Helvetica findfont 20 scalefont setfont\n\
100 100 moveto\n\
(This is a Message) show\n\
% crash\n\
showpage\n";

void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

int main(int argc, char *argv[]) {
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	FILE *fp_in, *fp_out, *com_in, *com_out;
	char *com="COM1"; 
/*	char *com="LPT1";  */  /* only works in outward direction - if rerouted */
	char *s;
	int n, m, firstarg=1;
	
	printf("Trying %s\n", com);

	if ((com_in = fopen(com, "rb")) == NULL) {
		fprintf(stderr, "Can't open %s input\n", com);
		perror(com);
		exit(0);
	}
	
	if ((com_out = fopen(com, "wb")) == NULL) {
		fprintf(stderr, "Can't open %s output\n", com);
		perror(com);
		exit(0);
	} 
	
/*	com_out = com_in; */

#if DEBUGFLUSH != 0
	setbuf(com_out, NULL); /* serious stuff */
#endif

	if (argc <= 1) {
		printf("Will now send message out\n"); /* getchar(); */
		n = fprintf(com_out, "%s", mess); 
		if (n < 0) fprintf(stderr, "n = %d ", n);
		printf("Will now send control D out\n"); /* getchar(); */
		n = putc(4, com_out); /* control D */
		if (n < 0) fprintf(stderr, "n = %d ", n);
		n = fflush(com_out);
		if (n < 0) fprintf(stderr, "n = %d ", n);
		printf("Will now send out control T's\n"); /* getchar(); */
	
		for(;;) {

			printf("+"); /* getchar(); */
			n = putc(20, com_out); /* control T */
			if (n < 0) fprintf(stderr, "n = %d ", n);
			n = fflush(com_out);
			if (n < 0) fprintf(stderr, "n = %d ", n);
			printf("-"); /* getchar(); */
			
			*line = '\0';
			s = fgets(line, MAXLINE, com_in);
			if (s != NULL) printf("Line is: %s", line);
			if (kbhit() != 0) break;
		}
		if (*line != '\0') 	printf("Line is: %s", line);
	}
	else {
		for (m = firstarg; m < argc; m++) {
			strcpy(fn_in, argv[m]);
			extension(fn_in, "ps");
			printf("Will now send file %s\n", fn_in); /* getchar(); */
			if ((fp_in = fopen(fn_in, "rb")) == NULL) {
				fprintf(stderr, "Can't open %s input file\n", fn_in);
				perror(fn_in);
				exit(0);
			}
			while (fgets(line, MAXLINE, fp_in) != NULL) {
#if TRACEFLAG != 0
				printf("%s", line); 
#endif
				n = fprintf(com_out, "%s", line); 
				if (n < 0) fprintf(stderr, "n = %d ", n);
			}	
		}
		n = fflush(com_out);
		if (n < 0) fprintf(stderr, "n = %d ", n);

		for(;;) {
			*line = '\0';
			s = fgets(line, MAXLINE, com_in);
			if (s != NULL) printf("Line is: %s", line);
			if (kbhit() != 0) break;
		}
		if (*line != '\0') 	printf("Line is: %s", line);

	}

	printf("Closing the connection\n"); /* getchar(); */

	n = fclose(com_out);
	if (n < 0) fprintf(stderr, "n = %d ", n);
	n = fclose(com_in);
	if (n < 0) fprintf(stderr, "n = %d ", n);

	printf("Reached the end\n"); /* getchar(); */
	return 0;
}
