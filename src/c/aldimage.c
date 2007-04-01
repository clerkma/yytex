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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* modify PS produced by Quark so image works better */
/* adds lprep header also */

/* #define FNAMELEN 80 */
#define MAXLINE 512
#define MAXCOLUMNS 78

int clm=0;

int bintohexflag = 0;		/* not for new version of Quark ? */

char *newreadstring=
	"/xpreadstring{currentfile xppixstr readhexstring pop}bdf";

/* char preamblefile[FILENAME_MAX]= "c:/ps/lprep68.pre";  */

char preamblefile[FILENAME_MAX]= "c:/ps/lprep71.pre"; 

char line[MAXLINE];

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

int readline (FILE *input, char *buffer, int maxlen) {
	int c, n=0;
	char *s=buffer;

	if((c = getc(input)) == EOF) return EOF;
	while (c != '\r' && c != '\n') {
		if (c == EOF) {
			c = '\n'; break;
		}
		if (n++ >= maxlen) break;
		*s++ = (char) c;
		c = getc(input);
	}
	if (c == '\r') {
		c = getc(input);
		if (c != '\n') {
			(void) ungetc(c, input);
			c = '\n';
		}		
	}
	*s++ = (char) c; *s = '\0';
	return 0;
}

void insertpreamble(FILE *output) {
	FILE *preamble;
	int c;
	forceexten(preamblefile, "pre");
	if ((preamble = fopen(preamblefile, "r")) == NULL) {
		forceexten(preamblefile, "pro");
		if ((preamble = fopen(preamblefile, "r")) == NULL) {
			perror(preamblefile); return;
		}
	}
	while ((c = getc(preamble)) != EOF) putc(c, output);
	fclose(preamble);
}

/* return pointer to file name - minus path - returns pointer to filename */
char *removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX], oldfilename[FILENAME_MAX];
	long i, k, bytes;
	int c, d;
	
	if (argc < 2) exit(3);
	strcpy(infilename, argv[1]);
/*	extension(infilename, "ps"); */
/*	strcpy(outfilename, argv[1]); */
	strcpy(outfilename, removepath(argv[1]));

	forceexten(outfilename, "ps");	
	if (strcmp(infilename, outfilename) == 0) {
		strcpy(oldfilename, infilename);
		forceexten(infilename, "bak");
		(void) remove(infilename);
		(void) rename(oldfilename, infilename);
	}
	if ((input = fopen(infilename, "rb")) == NULL) {
		perror(infilename); exit(7);
	}
	if ((output = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename); exit(9);
	}

/*	(void) readline(input, line, MAXLINE); */
	for (;;) {
		if (readline(input, line, MAXLINE) == EOF) {
			fprintf(stderr, "Premature EOF\n"); exit(7);
		}
		if (strstr(line, "%%IncludeProcSet") != NULL) break;
		if (strstr(line, "%%BeginProcSet") != NULL) {
			while (strstr(line, "%%EndProcSet") == NULL) {
				if (readline(input, line, MAXLINE) == EOF) {
					fprintf(stderr, "Premature EOF\n"); exit(7);
				}
			}
			break;
		}
		fputs(line, output);
	}

	insertpreamble(output);

	if (ferror(output) != 0) {
		perror(outfilename); exit(3);
	}

	for (;;) {
		if (readline(input, line, MAXLINE) == EOF) break;
		if (strncmp(line, "/xpreadstring", 13) == 0) {
			fprintf(output, "%s\n", newreadstring);
			bintohexflag = 1;			/* 92/March/16 */
		}
		else if (strstr(line, "%%BeginBinary:") != NULL) {
			if(sscanf(line + 14, "%ld", &bytes) < 1) {
				fprintf(stderr, "Don't understand: %s", line);
			}
			else if (bintohexflag != 0) { /* convert binary to hex ? */
				fputs(line, output);
				clm = 0;
				i = 1;			/* i = 0; */ /* ??? */
				while ((c = getc(input)) >= ' ') {
					putc(c, output); i++;
				}
				if (c == '\r') c = '\n';
				putc(c, output); i++;
				while ((c = getc(input)) < ' ') {
					if (c == '\r') c = '\n';
					putc(c, output); i++;
				} 
				(void) ungetc (c, input); 
/*				printf("at %d ", i); */		/* debugging */
				for (k = i; k < bytes; k++) {
					c = getc(input);
					d = c & 15; c = (c >> 4) & 15;
					if (c < 10) c = c + '0'; else c = c + 'A' - 10; 
					if (d < 10) d = d + '0'; else d = d + 'A' - 10; 
					putc(c, output);
					putc(d, output);
					if ((clm += 2) >= MAXCOLUMNS) {
						putc('\n', output);
						clm = 0;
					}
				}
			}
			else fputs(line, output);
			putc('\n', output);
		}
		else fputs(line, output);
		if (ferror(output) != 0) {	/* 1992/Nov/19 */
			perror(outfilename); exit(3);
		}
	}
	fclose(input);
	if (ferror(output) != 0) perror(outfilename);
	else fclose(output);
	return 0;

}
