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

/* print out PostScript FontName from PFA or PFB file or AFM file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* #define FNAMELEN 80 */
#define MAXLINE 512

int verboseflag = 1;
int traceflag = 0;
int defineflag = 0;
int wrapflag = 0;
int flushcr = 0;

char *outfilename="c:\\ps\\params.ps";	/* standard place to write output */

char *familyname="";					/* if given */

char line[MAXLINE];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

/***************************************************************************/

/* returns zero when reached EOF and line empty - char count otherwise */

int getline(FILE *input, char *buff, int nlen) { /* get line from normal ASCII file */
	int c, k=0;
	char *s=buff;

	c = getc(input);
/*	if (flushcr != 0) while (c == '\r') c = getc(input); */
	if (c == EOF) {
		*s = '\0';	return 0;
	}
	while ((c != '\n' && c != '\r' && c != EOF && k < nlen)) {
		if (c < ' ' && c != '\t') c = getc(input);
		else {
			*s++ = (char) c; k++;
			c = getc(input);
		}
	}
	if (c == '\r' && flushcr != 0) {
		c = getc(input); 
		if (c != '\n') {
			(void) ungetc(c, input);
			c = '\r';		/* put back the return at least 92/Oct/30 */
/*			c = '\n'; */ /* ??? */
		}
	}
	else if (c == EOF) {
		if (k > 0) c = '\n'; 
		else {
			*s = '\0'; return 0;
		}
	}
	*s++ = (char) c; k++;
	*s = '\0';
	return k;
}

int getrealline(FILE *input, char *buff, int nlen) { /* get non-comment, non-blank */
	int k;
	k = getline(input, buff, nlen);
	while ((*buff == '%' || *buff == '\n') && k > 0)
		k = getline(input, buff, nlen);
	return k;
}

void copyfile(FILE *output, FILE *input) {
	while (fgets(line, MAXLINE, input) != NULL) {
		fputs(line, output);
	}
}

void writenames(FILE *output, char *name) {
	if (defineflag) fputs("/fontname ", output);
	if (wrapflag || defineflag) putc('(', output);
	fputs(name, output);
	if (wrapflag || defineflag) putc(')', output);
	if (defineflag) fputs(" def", output);
	putc('\n', output);
	if (strcmp(familyname, "") != 0) {
		fprintf(output, "/familyname (%s) def\n", familyname);
	}
}

int scanfile(FILE *output, FILE *input) {
	int c, d;
	char fontname[MAXLINE];
	char *s;

/*	try and figure out what kind of a file it is */
	c = getc(input); d = getc(input);
	if (c == 0 && d == 1) {			/* take care of PFM case */
		if (traceflag) printf("Apparently a PFM file\n");
		fseek(input, 139, SEEK_SET);
		c = getc(input); d = getc(input);
		c = (d << 8) | c;			/* ptr to Driver Info */
		fseek (input, c, SEEK_SET);
		s = fontname;
		while ((c = getc(input)) >= ' ') *s++ = (char) c;
		*s = '\0';
		writenames(output, fontname);
		if (traceflag) writenames(stdout, fontname);
		return 0;
	}
	
/*	take care of PFA, PFB, and AFM */
	while (getrealline(input, line, sizeof(line)) > 0) {
/*		if ((s = strstr(line, "/FontName ")) != NULL) {
			if (sscanf(s+strlen("/FontName "), "/%s", fontname) < 1) {
				fprintf(stderr, "Don't understand %s", line);
			}
			writename(output, fontname);
			return 0;
		} */ /* 1993/Aug/14 */
		if ((s = strstr(line, "/FontName")) != NULL) {
			s += 9;						/* step over `/FontName' */
/*			while (*s != '/' && *s != '\0') s++; */	/* 1993/Aug/15 */
			while (*s != '/' && *s != '\0' && *s != '(') s++; /* 1993/Oct/22 */
			if (sscanf(s, "/%s", fontname) < 1) {
				if (sscanf(s, "(%s)", fontname) < 1) /* 1993/Oct/22 */
					fprintf(stderr, "Don't understand %s", line);
				else if ((s = strchr(fontname, ')')) != NULL) *s = '\0';
			}
			if (traceflag) printf("Apparently a PFA file\n");
			writenames(output, fontname);
			if (traceflag) writenames(stdout, fontname);
			return 0;
		} 
/*		if (strncmp(line, "FontName ", strlen("FontName ")) == 0) {
			if (sscanf(line + strlen("FontName "), "%s", fontname) < 1) {
				fprintf(stderr, "Don't understand %s", line);
			}
			writename(output, fontname);
			return 0;	
		} */ /* 1993/Aug/14 */
		if (strncmp(line, "FontName", strlen("FontName")) == 0) {
			if (sscanf(line + strlen("FontName"), "%s", fontname) < 1) {
				fprintf(stderr, "Don't understand %s", line);
			}
			if (traceflag) printf("Apparently an AFM file\n");
			writenames(output, fontname);
			if (traceflag) writenames(stdout, fontname);
			return 0;	
		} 		
		if (*line == '/' && strstr(line, " 16 dict dup begin") != NULL &&
			strstr(line, "/FontInfo") == NULL) {		/* 1993/Dec/23 */
			if (sscanf(line, "/%s", fontname) < 1) {
				fprintf(stderr, "Don't understand %s", line);
			}
			writenames(output, fontname);
			if (traceflag) writenames(stdout, fontname);
			return 0;	/* success */
		}
	}
	fprintf(stderr, "Did not find FontName\n", line);
	return -1;			/* failed */
}

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = stripname(filename);
/*	if ((s = strrchr(filename, '\\')) != NULL) s++;
	else if ((s = strrchr(filename, '/')) != NULL) s++;
	else if ((s = strrchr(filename, ':')) != NULL) s++;
	else s = filename; */
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
/*	if (traceflag != 0) printf("n = %d m = %d ", n, m); */
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
/*	if (traceflag != 0) printf("Now trying file name: %s\n", s); */
/*	return 0; */
}

FILE *tryopen(char *filename, char *ext, char *openmode){
	FILE *input;

	extension(filename, ext);
	input = fopen(filename, openmode);
	if (input == NULL) {
		underscore(filename);
		input = fopen(filename, openmode);
	}
	return input;
}

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	int k, firstarg=1;
	char *s;

/*	-n on command line means use /fontname (<FontName>) def output form */
/*	-f gives FamilyName */

	if (traceflag) {
		for (k = 0; k < argc; k++) printf("%d\t%s\n", k, argv[k]);
	}

	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (strcmp(argv[firstarg], "-n") == 0) {
			defineflag = 1;
		}
		else if (strncmp(argv[firstarg], "-f", 2) == 0) {
			if ((s = strchr(argv[firstarg], '=')) != NULL) familyname = s+1;
			else {
				firstarg++;
				familyname = argv[firstarg];
			}
			printf("FamilyName %s\n", familyname);
		}
		firstarg++;
	}
	if (argc <= firstarg) {		/*  file name specified ? */
		exit(1);
	}

	strcpy(fn_in, argv[firstarg]);
	input = fopen(fn_in, "rb");
	if (input == NULL) {
		strcpy(fn_in, argv[firstarg]);
		input = tryopen(fn_in, "pfb", "rb");
	}
	if (input == NULL) {
		strcpy(fn_in, argv[firstarg]);
		input = tryopen(fn_in, "pfa", "r");
	}
	if (input == NULL) {
		strcpy(fn_in, argv[firstarg]);
		input = tryopen(fn_in, "afm", "r");
	}	
	if (input == NULL) {
		strcpy(fn_in, argv[firstarg]);
		input = tryopen(fn_in, "pfm", "rb");
	}	
	if (input == NULL) {
/*		perror(fn_in); exit(2); */
		perror(argv[firstarg]); exit(2); 
	}
	if (traceflag) printf("Opened %s for input\n", fn_in); 

	if (verboseflag != 0) printf("FILE: %s\n", fn_in);
	firstarg++;

	if (argc > firstarg) {
		outfilename = argv[firstarg];
		strcpy(fn_out, outfilename);
		if ((output = fopen(fn_out, "w")) == NULL) {
			perror(fn_out); exit(2);
		}
		if (traceflag) printf("Opened %s for output\n", fn_out);
		wrapflag++;
	}
	else {
		output = stdout;
		if (traceflag) printf("Using stdout for output\n");
	}

	if (scanfile(output, input) != 0) {
		fclose(output);
		fclose(input);
		exit(5);
	}

	if (ferror(output) != 0) {
		perror(fn_out); exit(3);
	}
	else fclose(output);
	fclose(input);
	return 0;
}
