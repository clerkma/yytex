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

/* Remap RAW file to use new encodig vector assignments */
/* changraw.c <file-vec> <file-raw> */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <conio.h>

/* #define FNAMELEN 80 */
#define CHARNAME_MAX 64
#define MAXCHRS 256
#define MAXCHARNAME 32
#define MAXLINE 256

int verboseflag=1;
int traceflag=0;

static char encoding[MAXCHRS][MAXCHARNAME];
static char line[MAXLINE];
static char comment[MAXLINE];

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

int lookup(char *charname) {
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

void flushnewline(char *s) {
	while (*s++ != '\n') ;
	*(s-1) = ' ';
}

void remapchars(FILE *input, FILE *output) {
	int chr, chrd, xo, yo;
	double width;
	char charname[CHARNAME_MAX], filename[FILENAME_MAX];
	char *s, *t;

	t = fgets(line, MAXLINE, input);
	while (t != NULL) {
		if (*line != ']') {
			if (traceflag != 0) printf("%s", line);
			fputs(line, output);
			t = fgets(line, MAXLINE, input);
			continue;
		}
		fputs(line, output);
		t = fgets(line, MAXLINE, input);		
		if (sscanf(line, "%d %lg", &chr, &width) < 2) {
			fprintf(stderr, "Don't understand %s", line);
		}
		if ((s = strchr(line, '%')) != NULL) strcpy(comment, s);
		else strcpy(comment, "");	/* remember comment on this line */
		t = fgets(line, MAXLINE, input);
		if (sscanf(line, "%% %s %s %d", &charname, &filename, &chrd) < 3) {
			fprintf(stderr, "Don't understand %s", line);
		}
		if (chrd != chr) {
			fprintf(stderr, "Inconsistent char number %d <> %d\n", chr, chrd);
		}
		if (strcmp(charname, "AT&T") == 0) strcpy(charname, "ATT");
		if (strcmp(charname, "asciicircum") == 0) 
			strcpy(charname, "circumflex");
		if (strcmp(charname, "asciitilde") == 0) 
			strcpy(charname, "tilde");		
		if((chr = lookup(charname)) < 0) {
			fprintf(stderr, "Char %s not found\n", charname);
		}
		if (chr != chrd && verboseflag != 0) printf("%d => %d; ", chrd, chr);
		if (strstr(comment, "offset") != NULL ||
			(strcmp(comment, "") == 0))
			fprintf(output, "%d %lg\n", chr, width);
		else fprintf(output, "%d %lg %s", chr, width, comment);
		fprintf(output, "%% %s %s %d\n", charname, filename, chr);
		t = fgets(line, MAXLINE, input);
		while (*line == '%') {	 /* copy over comment lines */
			fputs(line, output);
			t = fgets(line, MAXLINE, input);
		}
		if(sscanf(line, "%d %d m", &xo, &yo) < 2) {
			fprintf(stderr, "Expecting moveto line: %s", line);
		}
		if (strstr(comment, "offset") != NULL) {
			flushnewline(line);
			strcat(line, comment); /* insert offset comment here */
		}
		fputs(line, output);
		t = fgets(line, MAXLINE, input);
	}
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
		if (verboseflag != 0) printf("\nOpening old raw file ");
		strcpy(fn_in, argv[m]);
		extension(fn_in, "afm");
		if (verboseflag != 0) printf("%s\n", fn_in);
		if((fp_in = fopen(fn_in, "r")) == NULL) {
			perror(fn_in); exit(1);
		}
		if (verboseflag != 0) printf("Opening new raw file ");
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
		if (verboseflag != 0) printf("Remapping characters\n");
		remapchars(fp_in, fp_out);
		if (verboseflag != 0) printf("Closing files\n");
		fclose(fp_in);
		if (ferror(fp_out) != 0) {
			perror(fn_out); exit(13);
		}
		else fclose(fp_out);
	}
	return 0;
}
