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

/* convert from hex format back to raw format */

/* #define FNAMELEN 80 */
#define MAXLINE 8192

#include <stdio.h>
#include <conio.h>					/* for user interrupt */
#include <string.h>
#include <stdlib.h>
#include <errno.h>

char line[MAXLINE];

int xll, yll, xur, yur;

int charcount;

int bboxflag;

double designsize=10.0;

double scale=1.0;

int verboseflag = 0;

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

void scantochars(FILE *input) {
	bboxflag = 0;
	charcount = 0;
	if (fgets(line, MAXLINE, input) == NULL) {
		fprintf(stderr, "Unexpected EOF\n"); exit(1);
	}
	while(strstr(line, "/CharDefs get") == NULL) {
		
		if (strstr(line, "/FontBBox ") != NULL) {
			if((sscanf(line, "/FontBBox [%d %d %d %d]", 
				&xll, &yll, &xur, &yur)) == 4) bboxflag++;
		}
		if (fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Unexpected EOF\n"); exit(1);
		}
	}
}

void writeheader(FILE *output) {
	fprintf(output, "%lg %lg\n", designsize, scale);
	fprintf(output, "%d %d %d %d\n", xll, yll, xur, yur);
}

void getdesign(char *fn_in) {
	char *s;
	int isize;
	
	designsize = 10.0;		/* safe default */
	if((s = strpbrk(fn_in, "0123456789")) == NULL) {
		if (strcmp(fn_in, "cminch") == 0) designsize = 104.069;
		else fprintf(stderr, "WARNING: Can't figure out design size\n");
	}
	else {
		if(sscanf(s, "%d", &isize) < 1) {
			fprintf(stderr, "WARNING: Can't figure out design size\n");
		}
		else {
			if (isize == 17) designsize = 17.28;
			else designsize = (double) isize;
		}
	}
}

void spacify(char *s) {
	int c;
	while ((c = *s) != '\0') {
		if (c == 'A') *s = ' ';
		else if (c == 'B') *s = '-';
		else if (c == 'F') *s = 'm';
		else if (c == 'E') *s = 'l';
		else if (c == 'C') *s = 'c';
		else if (c == 'D') *s = 'h';
		s++;
	}
}

void returnify(char *s) {
	int c;
	while ((c = *s++) != '\0') {
		if ((c == 'm') || 
			(c == 'l') ||
			(c == 'c') ||
			(c == 'h')) {
			if (*s == ' ') *s = '\n';
			else if (*s == '>') *s = '\n';
			else if (*s == '\0') {
				*s++ = '\n'; *s = '\0';
			}
			else fprintf(stderr, "I'm confused %s", line);
		}
	}
	if ((s = strchr(line, '>')) != NULL) *s = '\0';
	else if ((s = strstr(line, "put")) != NULL) *s = '\0';
}

void dochars(FILE *input, FILE *output) {
	char *s;
	int chrs, width, n;
	if (fgets(line, MAXLINE, input) == NULL) {
		fprintf(stderr, "Unexpected EOF\n"); exit(1);
	}
	while(strstr(line, "dup") == NULL) {
		if (fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Unexpected EOF\n"); exit(1);
		}
	}

	while(strstr(line, "dup") != NULL) {
		s = line;
		charcount++;
		while(strstr(s, "put") == NULL) {
			s = s + strlen(s) - 1;
			if (fgets(s, MAXLINE, input) == NULL) {
				fprintf(stderr, "Unexpected EOF\n"); exit(1);
			}			
		}
		spacify(line);
		returnify(line);
		if (sscanf (line, "dup %d<%d%n", &chrs, &width, &n) < 2) {
			fprintf(stderr, "Don't understand line %s", line);
		}
		s = line + n + 1;
		if (verboseflag != 0) printf("char %d width %d\n", chrs, width);
		fprintf(output, "]\n");
		fprintf(output, "%d %d\n", chrs, width);
		fprintf(output, "%s", s);
		if (fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Unexpected EOF\n"); exit(1);
		}
	}
		

}

int main(int argc, char *argv[]) {
	FILE *input, *output;
/*	int n, x, y; */
	int m;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	char *s;
/*	char field[FNAMELEN]; */

	if (argc < 2) exit(1);
	for(m = 1; m < argc; m++) {
		strcpy(fn_in, argv[m]);
		extension(fn_in, "hex");
		if((input = fopen(fn_in, "r")) == NULL) {
			perror(fn_in), exit(2);
		}
		s = stripname(fn_in);
		strcpy(fn_out, s);
		forceexten(fn_out, "raw");
		if((output = fopen(fn_out, "w")) == NULL) {
			perror(fn_out), exit(3);
		}

		printf("Processing %s ", fn_in);

		getdesign(fn_in);
		scantochars(input);
		writeheader(output);
		if (verboseflag != 0) printf("Finished with header\n");
		dochars(input, output);

		printf(" - %d characters\n", charcount);

		fclose(input);
		if (ferror(output) != 0) {
			perror(fn_out), exit(4);
		}
		else fclose(output);
	}
	if (argc > 2) printf("Processed %d files\n", argc -1);
	return 0;
}
