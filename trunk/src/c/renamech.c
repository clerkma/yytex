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

/* usage: renamech <afm-file> */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <conio.h>

/* #define FNAMELEN 80 */
#define MAXLINE 256
#define MAXCHARNAME 32
#define MAXCHRS 512

int verboseflag=1;
int traceflag=0;

char line[MAXLINE], buffer[MAXLINE];

int nchrs;

char charold[MAXCHRS][MAXCHARNAME];
char charnew[MAXCHRS][MAXCHARNAME];

#ifdef IGNORE
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

int readmapping(FILE *input) {
	int k=0;
	char charnamea[MAXLINE], charnameb[MAXLINE];

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf(line, "%s %s", charnamea, charnameb) < 2) {
			fprintf(stderr, "Don't understand: %s", line);
		}
		else if (strlen(charnamea) > MAXCHARNAME ||
				strlen(charnameb) > MAXCHARNAME) {
			fprintf(stderr, "Char names too long: %s", line);
		}
		else if (k >= MAXCHRS) {
			fprintf(stderr, "Too many characters\n");
			return k;
		}
		else {
			if (strchr(charnamea, '(') != NULL ||
				strchr(charnamea, ')') != NULL ||
				strchr(charnamea, '/') != NULL ||
				strchr(charnamea, '\\') != NULL)
					fprintf(stderr, "ERROR: bad character name: %s\n",
						charnamea);
			if (strchr(charnameb, '(') != NULL ||
				strchr(charnameb, ')') != NULL ||
				strchr(charnameb, '/') != NULL ||
				strchr(charnameb, '\\') != NULL)
					fprintf(stderr, "ERROR: bad character name: %s\n",
						charnameb);
			strcpy(charold[k], charnamea);
			strcpy(charnew[k], charnameb);			
			k++;
		}		
	}
	return k;
}

int remap (char *name) {
	int k;
	for (k = 0; k < nchrs; k++) {
		if (strcmp(charold[k], name) == 0) {
			strcpy(name, charnew[k]);
			return -1;
		}
	}
	return 0;
}

void renamekernpair(void) {
	char charnamea[MAXLINE], charnameb[MAXLINE];
	double kern;

	if (sscanf (line, "KPX %s %s %lg", charnamea, charnameb, &kern) < 3) {
		fprintf(stderr, "Don't understand: %s", line);
	}
	else {
		remap(charnamea); remap(charnameb);
		sprintf(line, "KPX %s %s %lg ;\n", charnamea, charnameb, kern);
	}
}

void renamecomposite(void) {
	char charnamea[MAXLINE], charnameb[MAXLINE], charnamec[MAXLINE];
	int z, xa, ya, xb, yb;

	if (sscanf (line, "CC %s %d ; PCC %s %d %d ; PCC %s %d %d",
		charnamea, &z, charnameb, &xa, &ya, charnamec, &xb, &yb) < 8) {
		fprintf(stderr, "Don't understand: %s", line);
	}
	else {
		remap(charnamea); remap(charnameb); remap(charnamec);
		sprintf(line, "CC %s %d ; PCC %s %d %d ; PCC %s %d %d ;\n", 
			charnamea, z, charnameb, xa, ya, charnamec, xb, yb);
	}
}

void renamecharmetric(void) {
	char charnamea[MAXLINE], charnameb[MAXLINE], charnamec[MAXLINE];
	int z, n;
	double wx, xll, yll, xur, yur;
	char *s, *t;
	
	if (sscanf (line, "C %d ; WX %lg ; N %s ; B %lg %lg %lg %lg%n",
		&z, &wx, charnamea, &xll, &yll, &xur, &yur, &n) < 7) {
		fprintf(stderr, "Don't understand: %s", line);
	}
	else {
		strcpy(buffer, line+n);	/* save the tail - ligatures */
		remap(charnamea); 
		sprintf(line, "C %d ; WX %lg ; N %s ; B %lg %lg %lg %lg ;", 
			z, wx, charnamea, xll, yll, xur, yur);
		if ((s = strchr(buffer, ';')) != NULL) {
			while ((s = strchr(s, 'L')) != NULL) {
				if (sscanf(s, "L %s %s%n", charnameb, charnamec, &n) < 2){
					fprintf(stderr, "Don't understand: %s", s);
				}
				else {
					remap(charnameb);  remap(charnamec); 
					t = line + strlen(line);
					sprintf(t, " L %s %s ;", charnameb, charnamec);
					s = s + n;
/*					printf("n = %d rest = %s", n, s); *//* debugging */
				}
			}
		}
		strcat(line, "\n");
	}
}

void renamechar (FILE *output, FILE *input) {
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') {
			fputs (line, output); continue;
		}
		else if (strncmp(line,"C ", 2) == 0) renamecharmetric();
		else if (strncmp(line,"CC ", 3) == 0) renamecomposite();
		else if (strncmp(line,"KPX ", 4) == 0) renamekernpair();
		fputs (line, output);
	}	
}

int main(int argc, char *argv[]) { 
    FILE *fp_in, *fp_out;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_bak[FILENAME_MAX];
	int firstarg=1, m;
	char *s;

	if (argc < 2) exit(1);
	
	if (verboseflag != 0) printf("Reading name mapping file\n");

	strcpy(fn_in, argv[firstarg++]);
	if((fp_in = fopen(fn_in, "r")) == NULL) {
		perror(fn_in); exit(1);
	}
	nchrs = readmapping(fp_in);
	fclose(fp_in);

	for (m = firstarg; m < argc; m++) {

	strcpy(fn_in, argv[m]);
	extension(fn_in, "afm");
	if (verboseflag != 0) printf("File:  %s\n", fn_in);
	if((fp_in = fopen(fn_in, "r")) == NULL) {
		perror(fn_in); exit(1);
	}
	if ((s = strrchr(fn_in, '\\')) != NULL) s++;
	else if ((s = strrchr(fn_in, '/')) != NULL) s++;
	else if ((s = strrchr(fn_in, ':')) != NULL) s++;	
	else s = fn_in;
	strcpy (fn_out, s);
	forceexten(fn_out, "afm");
	if (strcmp(fn_out, fn_in) == 0) {
		strcpy(fn_bak, fn_in);
		forceexten(fn_bak, "bak");
		remove(fn_bak);
		printf("Renaming %s to %s\n", fn_in, fn_bak);
		rename(fn_in, fn_bak);
	}
	if ((fp_out = fopen(fn_out, "w")) == NULL) {
		perror(fn_out); exit(1);
	}

	renamechar(fp_out, fp_in);

	if (ferror(fp_out) != 0) {
		perror(fn_out); exit(3);
	}
	else fclose(fp_out);

	fclose(fp_in);
	}
	return 0;
}
