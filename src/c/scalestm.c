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

/* rescale a stem file - multiply coordinates by numer/denom */

/* #define FNAMELEN 80 */
#define MAXLINE 256

#include <stdio.h>
#include <conio.h>					/* for user interrupt */
#include <string.h>
#include <stdlib.h>
#include <errno.h>

double numer=1.0; /* double numer=72.27; */
/* double denom=2.0; */ /* double denom=72.0; */

double denom=2.04082;

int verboseflag=1;

int traceflag=0;

int wantoutput=1;

char line[MAXLINE];

FILE *errout=stdout;

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int round(double x) {
	if (x < 0) return (- (round (- x)));
	else return (int) (x + 0.5);
}

#ifdef IGNORE
void extension(char *fname, char *ext) { /* supply extension if none */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}
#endif

void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

int main(int argc, char *argv[]) {
	FILE *input, *output;
	int chrs, m, n, x, y;
	long totalstems=0, totalghosts=0, fontstems=0, ghoststems=0;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	char *s;
	char field[FILENAME_MAX];

	if (argc < 2) exit(1);

	totalstems=0; totalghosts=0;

	for (m = 1; m < argc; m++) {

		fontstems=0;  ghoststems=0;
		strcpy(fn_in, argv[m]);
		extension(fn_in, "hnt");
		if((input = fopen(fn_in, "r")) == NULL) {
			extension(fn_in, "stm");
			if((input = fopen(fn_in, "r")) == NULL) {
				perror(fn_in), exit(2);
			}
		}
		if (wantoutput != 0) {
			s = stripname(fn_in);
			strcpy(fn_out, s);
			forceexten(fn_out, "scl");
			if((output = fopen(fn_out, "w")) == NULL) {
				perror(fn_out), exit(3);
			}
		}
		
		while ((fgets(line, MAXLINE, input)) != NULL) {
			s = line;
			if (*line == '/' || *line == '%' || *line == '\n') {
				if (wantoutput != 0) fprintf(output, "%s", line);
				continue;
			}
			if (strncmp(line, "S ", 2) == 0) {
				if ((s = strchr(line, ';')) != NULL) {
					*s = '\0';
					if (wantoutput != 0) fprintf(output, "%s; ", line);
					if (traceflag) printf("|%s; |", line);
					s++;
					while (*s != '\0' && *s != 'C') s++;
					strcpy(line, s);
					if (traceflag) printf("|%s", line);
					s = line;
				}
			}
			if(sscanf(line, "C %d ; %n", &chrs, &n) < 1) {
				if (verboseflag != 0)
					fprintf(errout, "Don't understand line %s", line);
				if (wantoutput != 0) fprintf(output, "%s", line);
				continue;
			}
			s = s + n;
			if (wantoutput != 0) fprintf(output, "C %d ; ", chrs);
			while((sscanf(s, "%s %n", &field, &n)) > 0) {
				s = s + n;
				if (wantoutput != 0) fprintf(output, "%s ", field);
				while((sscanf(s, "%d %d %n", &x, &y, &n) == 2)) {
					s = s + n;
					fontstems++;
					if (x + 20 == y || x + 21 == y) {
						ghoststems++;
						/* potential ghost stem ! */
					}
					x = round((((double) x) * numer) / denom);
					y = round((((double) y) * numer) / denom);
					if (wantoutput != 0) fprintf(output, "%d %d ", x, y);
				}
				sscanf(s, "; %n", &n);
				s = s + n;
				if (wantoutput != 0) fprintf(output, "; ");
			}
			if (wantoutput != 0) fprintf(output, "\n");
		}
		fclose(input);
		if (wantoutput != 0) {
			if (ferror(output) != 0) {
				perror(fn_out), exit(4);
			}
			else fclose(output);
		}
		printf("%s: \tProcessed %ld stems \t(including %ld ghost stems)\n",
			fn_in, fontstems, ghoststems);
		totalstems = totalstems + fontstems;
		totalghosts = totalghosts + ghoststems;
	}
	printf("\nProcessed %d hint files containing %ld stems (including %ld ghost stems)\n", 
		argc - 1, totalstems, totalghosts);
	return 0;
}

/* avoid rescaling stems that are 20 or 21 (ghost stems) ? */

/* can be used to rescale stems - set wantoutput to 1 */
/* can be used to count stems - set wantoutput to 0 */
