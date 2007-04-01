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

/* Remap AFM to use new encodig vector assignments */
/* usage: changenc <file-vec> <file-afm> */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <conio.h>

/* #define FNAMELEN 80 */
#define MAXCHRS 256
#define MAXCHARNAME 32
#define MAXLINE 256

int verboseflag=1;
int traceflag=0;

static char encoding[MAXCHRS][MAXCHARNAME];
static char line[MAXLINE];

void readvector(FILE *input) {
	int k;
	char charname[MAXCHARNAME];
	for (k=0; k < MAXCHRS; k++) strcpy(encoding[k], "");
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == '\n') continue;
		if (sscanf(line, "%d %s", &k, &charname) < 2) {
			fprintf(stderr, "Don't understand %s", line);
			getch();
		}
		else strcpy(encoding[k], charname);
	}
}

int charcode(char *charname) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(charname, encoding[k]) == 0) return k;
	}
	return -1;
}

void uppercase(char *s, char *t) {
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s = (char) c;
}

void scanuptochar(FILE *input, FILE *output) {
	fgets(line, MAXLINE, input);
	while (strstr(line, "StartCharMetrics") == NULL) {
		fputs(line, output);
		fgets(line, MAXLINE, input);		
	}
	fputs(line, output);
}

void copyrest(FILE *input, FILE *output) {
	while (fgets(line, MAXLINE, input) != NULL) fputs(line, output);
}

void remapchars(FILE *input, FILE *output) {
	int n, chr, new;
	double width;
	char name[MAXCHARNAME], uname[MAXCHARNAME];
	char *s;

	fgets(line, MAXLINE, input);
	while (strstr(line, "EndCharMetrics") == NULL) {
		if (*line == '%' || *line == '\r') {
			fputs(line, output);
			fgets(line, MAXLINE, input);
			continue;
		}
		if (traceflag != 0) printf("%s", line);
		if (sscanf(line, "C %d ; WX %lg ; N %s ;%n", 
			&chr, &width, &name, &n) < 3) {
			fprintf(stderr, "Don't understand %s", line);
			getch();
		}
		if((new = charcode(name)) < 0) {
			fprintf(stderr, "Name %s not found\n", name);
/*			getch(); */
			uppercase(uname, name);
			if ((new = charcode(uname)) < 0) {
				fprintf(stderr, "Name %s not found\n", uname);
				getch();
			}
		}
		fprintf(output, "C %d ; WX %lg ; N %s ;", new, width, name);
		s = line + n;
		fputs(s, output);
		fgets(line, MAXLINE, input);
	}
	fputs(line, output);
}

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

int main(int argc, char *argv[]) { 
    FILE *fp_in, *fp_out, *fp_vec;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_vec[FILENAME_MAX];
	int m;
	char *s;

	if (verboseflag != 0) printf("Reading new encoding vector ");
	strcpy(fn_vec, argv[1]);
	extension(fn_vec, "vec");
	if (verboseflag != 0) printf("%s\n", fn_vec);
	if((fp_vec = fopen(fn_vec, "r")) == NULL) {
		perror(fn_vec); exit(1);
	}
	readvector(fp_vec);
	fclose(fp_vec);
	if (verboseflag != 0) printf("Closing new encoding vector\n");

	for (m = 2; m < argc; m++) {
		if (verboseflag != 0) printf("\nOpening old afm file ");
		strcpy(fn_in, argv[m]);
		extension(fn_in, "afm");
		if (verboseflag != 0) printf("%s\n", fn_in);
		if((fp_in = fopen(fn_in, "r")) == NULL) {
			perror(fn_in); exit(1);
		}
		if (verboseflag != 0) printf("Opening new afm file ");
		if ((s = strrchr(fn_in, '\\')) == NULL) {
			if ((s = strrchr(fn_in, ':')) == NULL) s = fn_in;
			else s++;
		} else s++;
		strcpy(fn_out, s);
		forceexten(fn_out, "new");
		if (verboseflag != 0) printf("%s\n", fn_out);
		if((fp_out = fopen(fn_out, "w")) == NULL) {
			perror(fn_out); exit(1);
		}
		if (verboseflag != 0) printf("Scanning up to charmetrics\n");
		scanuptochar(fp_in, fp_out);
		if (verboseflag != 0) printf("Remapping characters\n");
		remapchars(fp_in, fp_out);
		if (verboseflag != 0) printf("Copying rest of file\n");
		copyrest(fp_in, fp_out);
		if (verboseflag != 0) printf("Closing files\n");
		fclose(fp_in);
		fclose(fp_out);
	}
	return 0;
}
