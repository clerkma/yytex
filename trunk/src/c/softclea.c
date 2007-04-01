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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* #include <conio.h> */

/* clean up softfonts entries in win.ini */

#define MAXFILENAME 128
#define MAXLINE 256

int verboseflag = 0;
int detailflag = 0;
int traceflag = 0;
int overwriteflag = 0;

char line[MAXLINE];
char buffer[MAXLINE];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int gettoken (char *buff, FILE *input) {
	int c, n=0;
	char *s=buff;

	for (;;) {
		c = getc(input);
		if (c == EOF) return EOF;
		if (n++ >= MAXLINE) return n;
		if (c <= ' ') {
			*s++ = '\0';
			if (traceflag != 0) printf("%s\n", buff);
			return n;
		}
		else *s++ = (char) c;
	}
}

void showusage(char *s) {
	printf("\n");
	printf("Usage: %s [-{v}{i}] [win.ini]\n", s);
	if (detailflag == 0) exit(0);
	printf("\n");
	printf("\tv verbose mode\n");
	printf("\ti overwrite original file\n");
	printf("\t  (output appears in current directory otherwise)\n");
	exit(1);
}

int decodeflag (int c) { 			/* decode command  line flag */
	switch(c) { 
		case 'v': if(verboseflag != 0) traceflag = 1; else verboseflag = 1; return 0; 
		case '?': detailflag = 1; return 0; 
		case 'i': overwriteflag = 1; return 0; 
/* the rest take arguments */
/*		case 'a': charaflag = 1; return -1;  */
/*		case 'b': charbflag = 1; return -1;  */
		default: {
				fprintf(stderr, "Invalid command line flag '%c'\n", c);
				exit(13);
		}
	}
/*	return 0; */	/* ??? */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 1) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command line flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
/*				if (charaflag != 0) {
					if (sscanf (s, "%d", &spacecode) < 1)
						fprintf(stderr, "Don't understand %s\n", s);
					charaflag = 0;
				} */
/*				if (charbflag != 0) {
					if (sscanf (s, "%d", &suppresscode) < 1)
						fprintf(stderr, "Don't understand %s\n", s);
					charbflag = 0;
				} */
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int serialize(FILE *output, FILE *input, int outflag) {
	int serial=1;
	char *s;
	
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == ';') {
			if (outflag != 0) fputs(line, output);
			continue;
		}
		if (strncmp(line, "softfont", 8) != 0) break;
		if ((s = strchr(line, '=')) != NULL) {
			if (*(s+1) < ' ') continue;		/* ignore `softfontxyz=' */
			strcpy(buffer, s);
			sprintf(line+8, "%d", serial++);
			strcat(line, buffer);
		}
		else fprintf(stderr, "Don't understand: %s", line);
		if (outflag != 0) fputs(line, output);
	}
	if (outflag != 0) fputs(line, output);
	return (serial - 1);
}

/* void renumber(FILE *output, FILE *input) {
	while (fgets(line, MAXLINE, input) != NULL) {
		fputs(line, output);
		if (*line == ';') continue;
		if (strncmp(line, "softfonts", 9) == 0) {
			serialize(output, input);
		}
	}

} */

void renumber(FILE *output, FILE *input) {
	long softstart;
	int softcount;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == ';') {
			fputs(line, output);
			continue;
		}
		if (strncmp(line, "softfonts", 9) == 0) {
			softstart = ftell(input);
			softcount = serialize(output, input, 0);
			fseek(input, softstart, SEEK_SET);
			fprintf(output, "softfonts=%d\n", softcount);
			softcount = serialize(output, input, 1);
		}
		else fputs(line, output);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void removeexten(char *fname) {  /* remove extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) != NULL) {
		if ((t = strrchr(fname, '\\')) == NULL || s > t) *s = '\0';
	}
} 

char *stripname(char *name) {
	char *s;
	if ((s = strrchr(name, '\\')) != NULL) return (s+1);
	if ((s = strchr(name, ':')) != NULL) return (s+1);
	else return name;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[MAXFILENAME];
/*	char bakfilename[MAXFILENAME]; */
	char outfilename[MAXFILENAME];	
	int firstarg = 1;
/*	char *s; */

	if (argc < 1) showusage(argv[0]);

	firstarg = commandline(argc, argv, 1);
	
	printf("Program for cleaning up softfonts in WIN.INI. Version 0.9\n");

	if (firstarg > 1) strcpy(infilename, argv[firstarg]);
	else strcpy(infilename, "c:\\windows\\win.ini");

	extension(infilename, "ini");

	if (overwriteflag != 0) strcpy(outfilename, infilename);
	else strcpy(outfilename, stripname(infilename));

	if (strcmp(infilename, outfilename) == 0) {
		forceexten (infilename, "bak");
		remove(infilename); 		/* in case backup already exists */
		printf("Renaming %s to %s\n", outfilename, infilename);
		rename(outfilename, infilename); /* ignore error in renaming */
	}
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);
		exit(1); 
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);
		exit(1); 
	}

	renumber(output, input);

	(void) fclose(input);
	if (ferror(output) != 0) {
		perror(outfilename);
	}
	else (void) fclose(output);
	return 0;
}
